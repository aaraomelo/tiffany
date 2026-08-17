#!/usr/bin/env python3
# tools/tectos.py — A ASSERÇÃO UNIVERSAL SERVIDA POR UM LAÇO QUE NÃO COBRE A GAMA.
#
# O defeito irmão do `orfas.py`. Lá o número mede-se e não conta; aqui o número CONTA e
# não mede o que a frase diz. O caso que deu origem a isto estava no `supremo.c` §S2, na
# cláusula que distingue o SUPREMO de um majorante qualquer:
#
#     long todos = 1, base = (m > 40) ? m - 40 : 0;
#     for(long mm = base; mm < m; mm++) ...
#     ok("... e é ela que se varreu em TODOS os m' abaixo", ...)
#
# Com m a chegar aos 92681, viam-se 40. A asserção dizia «todos». E a cláusula nem
# precisava de varredura: saía da monotonia por indução.
#
# Dois padrões, e o primeiro é o que morde:
#
#   P1  A JANELA DESLIZANTE. O laço não começa no princípio da gama: o início é
#       calculado para trás a partir do fim, com um literal. É o defeito exacto, e é
#       raro — quando aparece, quase sempre está errado.
#
#   P2  O LITERAL NU NA CONDIÇÃO. A regra que o revisor pede é
#
#           previsão  ←  fórmula  ←  objecto
#
#       e quem a viola escreve-se `dim == 36` em vez de `dim == N*(N+1)/2`. Os dois
#       passam hoje; só o segundo continua a passar quando N mudar, e só o segundo diz
#       DE ONDE vem o número. O detector procura literais comparados directamente na
#       condição do veredicto — que é a versão automática do defeito da «referência
#       escrita à mão».
#
#       Ficam de fora 0, 1 e 2: «zero falhas» e «duas metades» são estruturais, não
#       previsões. O que interessa são os números que alguém teve de calcular.
#
#   P3  A QUANTIDADE DIVIDIDA POR SI PRÓPRIA. Encontrei QUATRO destas no mesmo ficheiro,
#       o `matricial.c`, e nenhuma podia falhar:
#
#           vold = 1.0/vol      →  fabs(vol*vold − 1.0) < 1e-9      «volumes recíprocos»
#           sgl  = −1.0/sg      →  fabs(sg)*fabs(sgl)  ≈ 1          «σ·σ' = −1»
#           meio = tr/2         →  razao = meio/tr     ≈ 0.5        «a coordenada é 1/2»
#           zin  = 1/sig        →  prod = zin*sig      ≈ 1          «ν troca dentro/fora»
#
#       Quatro frases com conteúdo, e por baixo de cada uma um x·(1/x). O padrão é
#       sempre o mesmo: uma variável DEFINIDA a partir de outra, e depois as duas
#       combinadas numa expressão que se compara com uma constante. O que se mede é a
#       aritmética, não a tese — e a tese fica por medir sem que nada acuse.
#
#       O conserto nunca foi apertar o limiar: foi construir o segundo objecto por um
#       caminho que não passe pelo primeiro.
#
#   P4  O COMENTÁRIO QUE DESACONSELHA A RÉGUA, COM A RÉGUA POR BAIXO. Esta casa escreve
#       nos comentários o que aprendeu — «a LEI, e não um limiar», «medir d < 1e-12 é
#       medir a minha paciência» — e às vezes a lição fica no comentário e não chega ao
#       código. No `vizinha.c` a frase dizia com todas as letras que medir o limiar era
#       medir a paciência, e a asserção logo abaixo media `d < 1e-12`.
#
#       Rende pouco e vale: quatro candidatos no repositório, três já corrigidos e um a
#       corrigir (o `koch.c`, onde o comentário dizia «medir a taxa não pede nenhum
#       limiar» e o código comparava a razão com 1e-9 — sendo que ela é 4/9 EXACTA).
#
# O controlo é correr nos ficheiros onde a resposta já se sabe: o `supremo.c` de hoje não
# pode dar P1, e a versão anterior tinha de dar.
#
#   python3 tools/tectos.py
import re, glob, sys

# «todo», «todos», «nenhum», «sempre», «qualquer», «sem excepção» — o que promete a gama
UNIV = re.compile(r'\b(TODO|TODOS|TODAS|TODA|NENHUM|NENHUMA|SEMPRE|qualquer|'
                  r'sem excep\w+|sem excec\w+|em todos|em todas)\b')

# a janela deslizante: início calculado para trás a partir do fim, com um literal
JANELA = re.compile(r'\(\s*(\w+)\s*[><]=?\s*(\d+)\s*\)\s*\?\s*\1\s*[-+]\s*\2\s*:')

# o literal nu comparado na condição: `casos == 300`. Fora 0,1,2 — estruturais.
NU = re.compile(r'[=!]=\s*(\d{1,})\b')

# o veredicto: a chamada e o seu texto
VERED = re.compile(r'\b(ok|VD|pulso|tique)\s*\(')

# P3: uma atribuição `v = expr`, para saber de QUEM v depende
ATRIB = re.compile(r'\b(?:double|float|long|int)?\s*(\w+)\s*=\s*([^;=][^;]*);')
NOMES = re.compile(r'\b([A-Za-z_]\w*)\b')
IGNORA = {'fabs','sqrt','pow','hypot','double','long','int','float','if','return',
          'sizeof','for','while','else','const','static','unsigned','printf'}
# e a combinação das duas na mesma expressão, comparada com um número
COMBINA = re.compile(r'[*/]')


def recíprocas(bloco):
    """duas formas do mesmo defeito, e cada uma com o seu critério.

    P3a  v é DEFINIDA a partir de w, e depois as duas TOCAM-SE num produto ou
         quociente comparado com um número:   vold = 1.0/vol ; fabs(vol*vold - 1.0) < 1e-9

    P3b  v é definida por uma expressão E, e depois COMPARA-SE com a mesma expressão:
         long malha = u1 * u2 ;  if (malha == u1*u2)     — x == x, o mais nu de todos

    O critério de P3b é a IGUALDADE das expressões normalizadas, e não «v aparece perto
    de w»: com o critério largo o repositório dava 2338 candidatos em 237 ficheiros, que
    é ruído puro — `if (v == a && w > 0)` entrava. Normalizado, dá o defeito e mais nada.
    """
    def norm(e):
        return re.sub(r'\s+', '', e)

    dep, defs, ordem, achados = {}, {}, [], []
    for m in ATRIB.finditer(bloco):
        v, expr = m.group(1), m.group(2)
        if len(v) < 2:
            continue
        prof, corte = 0, len(expr)
        for k, ch in enumerate(expr):
            if ch in '([':   prof += 1
            elif ch in ')]': prof -= 1
            elif ch == ',' and prof == 0: corte = k; break
        expr = expr[:corte]
        usados = {n for n in NOMES.findall(expr)
                  if n != v and len(n) > 1 and n not in IGNORA}
        if usados:
            dep[v] = usados
            defs[v] = norm(expr)
            ordem.append((m.end(), v))

    for fim, v in ordem:
        ve = re.escape(v)
        alvo = defs[v]
        achou = False
        # P3b: v comparado com a MESMA expressão que o definiu
        for m2 in re.finditer(r'\b' + ve + r'\b\s*([=!<>]=)\s*([^;)]*)', bloco[fim:]):
            if norm(m2.group(2)) == alvo and alvo:
                ln = bloco[fim:][max(0, m2.start()-30):m2.end()+10]
                achados.append((v, sorted(dep[v])[0], ln.strip().replace(chr(10), ' ')[:70]))
                achou = True
                break
        if achou:
            continue
        # P3a: v e w tocam-se num produto/quociente comparado com um número
        for w in dep.get(v, ()):
            we = re.escape(w)
            junto = re.compile(
                r'(?:fabs\s*\(\s*)?\b(?:' + ve + r'|' + we + r')\b\s*\)?'
                r'\s*[*/]\s*'
                r'(?:fabs\s*\(\s*)?\b(?:' + ve + r'|' + we + r')\b')
            for linha in bloco[fim:].split(chr(10)):
                if not junto.search(linha):
                    continue
                if re.search(r'[<>]\s*[\d.]|[=!]=\s*[\d.]|-\s*[\d.]+\s*\)', linha):
                    achados.append((v, w, linha.strip()[:70]))
                    achou = True
                    break
            if achou:
                break
    return achados


def texto_do_veredicto(src, i):
    """devolve o texto do ok(...) que começa no índice i, seguindo parênteses"""
    n, j, dentro = 0, i, False
    while j < len(src):
        c = src[j]
        if c == '"':
            j += 1
            while j < len(src) and not (src[j] == '"' and src[j-1] != '\\'):
                j += 1
        elif c == '(':
            n += 1; dentro = True
        elif c == ')':
            n -= 1
            if dentro and n == 0:
                return src[i:j+1]
        j += 1
    return src[i:i+2000]


def despe(s):
    """apaga comentários E literais de texto, preservando as quebras de linha — um
    scanner de um percurso, porque dois regex em fila erram quando uma string contém
    «/*» (o `triagem_limiares.sh` deu 1 onde havia 4 antes de levar isto)"""
    out, i, n = [], 0, len(s)
    while i < n:
        c = s[i]
        if c == '/' and i+1 < n and s[i+1] == '*':
            j = s.find('*/', i+2); j = n if j < 0 else j+2
        elif c == '/' and i+1 < n and s[i+1] == '/':
            j = s.find(chr(10), i); j = n if j < 0 else j
        elif c in '"\'':
            j, q = i+1, c
            while j < n and s[j] != q:
                j += 2 if s[j] == '\\' else 1
            j = min(j+1, n)
        else:
            out.append(c); i += 1; continue
        out.append(''.join(ch if ch == chr(10) else ' ' for ch in s[i:j]))
        i = j
    return ''.join(out)


def blocos(src):
    """parte o ficheiro nos blocos { ... } de primeiro nível dentro da main"""
    saida, ini, n = [], None, 0
    for j, c in enumerate(src):
        if c == '{':
            if n == 0:
                ini = j
            n += 1
        elif c == '}':
            n -= 1
            if n == 0 and ini is not None:
                saida.append((ini, src[ini:j+1]))
                ini = None
    return saida


def main():
    fich = sorted(glob.glob('tests/*.c') + glob.glob('tests/*.js') +
                  glob.glob('tools/*.c'))
    p1, p2, p3, vistos = [], [], [], 0
    for f in fich:
        try:
            src = open(f, encoding='utf-8').read()
        except Exception:
            continue
        # o comentário do próprio detector, e as menções ao defeito já corrigido, não
        # contam: só o CÓDIGO. Tira-se comentário de bloco e de linha.
        codigo = despe(src)
        for m in JANELA.finditer(codigo):
            linha = codigo[:m.start()].count('\n') + 1
            p1.append((f, linha, m.group(0)))
        for ini, bl in blocos(codigo):
            vistos += 1
            for v, w, ln in recíprocas(bl):
                p3.append((f, codigo[:ini].count('\n') + 1, v, w, ln))
            for v in VERED.finditer(bl):
                txt = texto_do_veredicto(bl, v.start())
                # só a CONDIÇÃO: o que vem depois da última string do veredicto
                fim = txt.rfind('"')
                cond = txt[fim+1:] if fim >= 0 else txt
                nus = [n for n in NU.findall(cond) if int(n) >= 3]
                if nus:
                    linha = codigo[:ini + v.start()].count('\n') + 1
                    p2.append((f, linha, sorted(set(nus), key=int)))
    print(f"  {len(fich)} ficheiros, {vistos} blocos\n")
    print(f"  P1 — JANELA DESLIZANTE (o defeito exacto): {len(p1)}")
    for f, l, t in p1:
        print(f"      {f}:{l}   {t}")
    if not p1:
        print("      nenhuma — nenhum laço começa a contar para trás a partir do fim")
    fich_p2 = len({f for f, _, _ in p2})
    print(f"\n  P2 — literal NU na condição do veredicto: {len(p2)}"
          f" em {fich_p2} ficheiros")
    for f, l, t in p2[:30]:
        print(f"      {f}:{l}   {', '.join(t)}")
    if len(p2) > 30:
        print(f"      ... e mais {len(p2)-30}")
    print("\n  Para cada um a pergunta é a mesma: esse número vem de uma FÓRMULA sobre o"
          "\n  objecto, ou foi calculado à mão e escrito? Se o objecto mudar, ele muda"
          " sozinho?")
    fich_p3 = len({f for f, *_ in p3})
    print(f"\n  P3 — a quantidade dividida por si própria: {len(p3)}"
          f" em {fich_p3} ficheiros")
    for f, l, v, w, ln in p3[:200]:
        print(f"      {f}:{l}   {v} vem de {w}   →   {ln}")
    if len(p3) > 25:
        print(f"      ... e mais {len(p3)-25}")
    print("\n  Em cada um: o segundo objecto foi CONSTRUÍDO por um caminho que não passa"
          "\n  pelo primeiro, ou foi escrito a partir dele? Se foi escrito, a asserção mede"
          "\n  aritmética e a tese fica por medir.")
    return 1 if p1 else 0


if __name__ == '__main__':
    sys.exit(main())
