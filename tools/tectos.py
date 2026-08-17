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
import collections
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


# ── P4: A FUNÇÃO E A SUA INVERSA, DENTRO DA MESMA COMPARAÇÃO ─────────────────────────
#
# O defeito que 16/08 deu sete vezes, sempre disfarçado por conversões que o tornavam
# ilegível. Medir f(f⁻¹(x)) = x não mede nada — é a definição do par relida:
#
#     octeto.c   |acos(ip/(n0·n1))·180/π − acos(−1/3)·180/π| < 1e-9
#                e ip/(n0·n1) É −1/3: dois acos e duas conversões a mascarar x = x
#     octeto.c   |acos(cos(2π/3))·180/π − 120| < 1e-9        acos∘cos
#     xx.c       log(xc·0,99) + 1 < 0   com xc = 1/e          É log(0,99)
#     milenio.c  L = log(exp(−λt)), e o passo é constante     log∘exp
#     koch.c     |log(4^N)/log(3^N) − log4/log3| < 1e-12      os N cancelam
#     liga.c     |sigma_comp(p_alvo) − alvo|/alvo < 1e-6      p_alvo veio de invertê-la
#     gerador    |√(L/C) − √((L/2)/(C/2))|                    (L/2)/(C/2) É L/C
#
# A regra: quando os DOIS lados de uma comparação passam pela mesma função, ela
# cancela-se e o que fica é a igualdade de dentro — que costuma ser INTEIRA. Ou se mede
# essa, ou não se mede nada.
#
# O padrão procurado é conservador: os PARES conhecidos (log/exp, acos/cos, asin/sin,
# atan/tan, sqrt/pow-2) a aparecerem compostos, ou a MESMA função transcendental a
# aparecer nos dois lados de uma subtracção dentro de um fabs.
PARES = [('log', 'exp'), ('exp', 'log'), ('acos', 'cos'), ('cos', 'acos'),
         ('asin', 'sin'), ('sin', 'asin'), ('atan', 'tan'), ('tan', 'atan'),
         ('log10', 'pow'), ('log2', 'pow')]
# a composição tem de ser DIRECTA: f(g(x)) e não f(g(x) + h(y)). O parêntesis do g tem
# de fechar imediatamente o do f — senão `log(exp(a) + exp(b))` entrava, e essa não é
# uma composição: é um logaritmo de uma soma, e nada se cancela.
COMPOSTA = [(a, b, re.compile(r'\b' + a + r'\s*\(\s*' + b + r'\s*\(')) for a, b in PARES]


def composta_directa(t, m, nome_g):
    """o parêntesis aberto por g fecha exactamente onde o de f fecha?"""
    i = m.end() - 1                      # o '(' do g
    d, j = 0, i
    while j < len(t):
        if t[j] == '(':
            d += 1
        elif t[j] == ')':
            d -= 1
            if d == 0:
                break
        j += 1
    if j >= len(t):
        return False
    resto = t[j+1:].lstrip()
    return resto.startswith(')')          # o do f fecha logo a seguir
# f(...) − f(...) dentro de um fabs: a mesma função nos dois lados de uma diferença
MESMA_FN = re.compile(
    r'fabsl?\s*\(\s*(sqrtf?|logf?|log10|log2|expf?|acos|asin|atan|cosf?|sinf?|tanf?)'
    r'\s*\([^()]*(?:\([^()]*\)[^()]*)*\)\s*[-+]\s*\1\s*\(')


def inversas(bloco):
    """devolve as ocorrências de f∘f⁻¹ ou f(x) − f(y) dentro de um fabs, no bloco."""
    achados = []
    for ln in bloco.split('\n'):
        t = ln.strip()
        if not t:
            continue
        for fa, fb, rx in COMPOSTA:
            m = rx.search(t)
            if m and composta_directa(t, m, fb):
                achados.append(('composta', fa + '∘' + fb, t[:78]))
                break
        m = MESMA_FN.search(t)
        if m:
            achados.append(('mesma nos dois lados', m.group(1), t[:78]))
    return achados


# ── P5: A QUANTIDADE DEFINIDA COMO O RESTO ───────────────────────────────────────────
#
# Uma variável é definida por subtracção — o que sobra depois de tirar as outras — e depois
# a asserção verifica que a SOMA de todas dá o total. Isso é a definição relida: o resto
# foi construído para fechar, e fecha. Quatro casos em 16/08:
#
#     colheita.c    A = 1.0 − R − T          →  «R + T + A = 1, a conservação da onda»
#     arraytermico  radia = P − volta        →  «volta + radia = P, o balanço fecha»
#     arraytermico  perdido = P − P_rec      →  «perdido/P > 0,99»   (o P cancela)
#     solar.c       ret = Ein − Eout         →  «ret = 2·I²·R·t»     (é a álgebra de cima)
#
# O que costuma ter conteúdo é a DESIGUALDADE que sobra — que o resto não é negativo, ou
# seja que as partes não excedem o total —, e essa não é tautologia: é a passividade do
# modelo, e pode falhar. O colheita.c chegou lá sozinho e escreve-o no texto.
#
# Procura-se: uma atribuição `v = A − B` (com ou sem mais termos) e, no mesmo bloco, uma
# condição que mencione `v` e pelo menos DOIS dos termos que o definem.
RESTO = re.compile(
    r'^\s*(?:const\s+)?(?:double|float|long|int|long\s+long)?\s*'
    r'([A-Za-z_]\w*)\s*=\s*([^;=]*?-[^;=]+);', re.M)


def resto_relido(bloco):
    """variáveis definidas por subtracção que reaparecem numa condição com os seus termos."""
    achados = []
    for m in RESTO.finditer(bloco):
        v, expr = m.group(1), m.group(2)
        termos = [t for t in re.findall(r'[A-Za-z_]\w*', expr)
                  if t not in ('fabs', 'fabsl', 'sqrt', 'exp', 'log', 'pow', 'double',
                               'float', 'long', 'int', 'creal', 'cimag', 'cabs')]
        if len(termos) < 2:
            continue
        # a condição de um veredicto no mesmo bloco menciona v e >= 2 dos termos?
        for cm in re.finditer(r'\bok\s*\((?:[^()]|\([^()]*\))*\)', bloco, re.S):
            cond = cm.group(0)
            fim = cond.rfind('"')
            cond = cond[fim+1:] if fim >= 0 else cond
            if not re.search(r'\b' + re.escape(v) + r'\b', cond):
                continue
            juntos = sum(1 for t in set(termos) if re.search(r'\b' + re.escape(t) + r'\b', cond))
            if juntos >= 2:
                # a distinção que importa: numa IGUALDADE o resto fecha por construção;
                # numa DESIGUALDADE o que sobra é a passividade, e essa PODE falhar.
                tem_ig = bool(re.search(r'==|fabs\s*\(', cond))
                tem_des = bool(re.search(r'[<>]', cond))
                tipo = 'IGUALDADE' if (tem_ig and not tem_des) else (
                       'desigualdade' if tem_des and not tem_ig else 'mista')
                achados.append((v, tipo + ': ' + ' '.join(sorted(set(termos))[:3]),
                                m.group(0).strip()[:64]))
                break
    return achados


# ── A ESPÉCIE DO LITERAL: contagem ou régua? ─────────────────────────────────────────
#
# Os 723 literais nus do P2 não são todos o mesmo defeito, e tratá-los como se fossem faz
# a lista inútil. Há duas espécies, e distinguem-se pelo que o literal ENFRENTA:
#
#   CONTAGEM   o literal está numa IGUALDADE com um contador incrementado num laço do
#              mesmo bloco — `casos == 6561`, `pares == 144`. É o total da varredura, e
#              é legítimo: diz quantos casos entraram, o que impede a asserção de passar
#              por a varredura estar vazia. MAS devia ser uma EXPRESSÃO e não o produto já
#              feito: escrevi 38416 onde eram 15⁴ = 50625, e 59535 onde eram 11·9⁴ = 72171
#              — duas vezes no mesmo dia. A conta que não erra é a que o compilador faz.
#
#   RÉGUA      o literal está numa DESIGUALDADE contra um valor calculado — `< 1e-9`,
#              `> 0.5`. Esse é o que a casa persegue: ou vem de uma fórmula sobre o
#              objecto, ou foi escolhido até passar.
#
# E há uma terceira, pior que as duas: o literal DECIMAL numa igualdade, que é a
# referência copiada — o `1.9248473002` do selberg.c, que era o traço 7 visto por um log.
INCR = re.compile(r'\b([A-Za-z_]\w*)\s*(?:\+\+|\+=\s*1\b)')


def especie(bloco, cond, nus):
    """contagem, EXPRESSAO, regua ou copiada — e a expressao e' a forma boa."""
    contadores = set(INCR.findall(bloco))
    tem_decimal = any('.' in n or 'e' in n.lower() for n in nus)
    for n in nus:
        # dentro de um PRODUTO ou SOMA? Entao e' uma expressao de contagem, e e' a forma
        # que se quer: `11L*9*9*9*9` em vez de `72171`, porque a conta que nao erra e' a
        # que o compilador faz. Nao e' defeito — e' o CONSERTO do defeito.
        if re.search(r'[*+]\s*' + re.escape(n) + r'\b', cond) or \
           re.search(r'\b' + re.escape(n) + r'[Ll]?\s*[*+]', cond):
            return 'expressao'
        for c in contadores:
            if re.search(r'\b' + re.escape(c) + r'\s*==\s*' + re.escape(n) + r'\b', cond) or \
               re.search(r'\b' + re.escape(n) + r'\s*==\s*' + re.escape(c) + r'\b', cond):
                return 'contagem'
    if re.search(r'==', cond) and tem_decimal:
        return 'COPIADA'
    if re.search(r'[<>]', cond):
        return 'REGUA'
    return 'outra'


# ── E AS REGUAS DECIMAIS, que o NU nao via ───────────────────────────────────────────
# O `NU` so' apanha inteiros a seguir a `==` ou `!=`. As reguas mais perigosas nao sao
# essas: sao os decimais numa DESIGUALDADE — `< 1e-9`, `> 0.5` —, porque um limiar decimal
# nunca vem de uma contagem. Procuram-se a' parte, e sao a lista que interessa ler.
REGUA_DEC = re.compile(r'[<>]=?\s*([0-9]*\.[0-9]+(?:[eE][-+]?\d+)?|[0-9]+[eE][-+]?\d+)')


def main():
    fich = sorted(glob.glob('tests/*.c') + glob.glob('tests/*.js') +
                  glob.glob('tools/*.c'))
    p1, p2, p3, p4, p5, vistos = [], [], [], [], [], 0
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
            for tipo, fn, ln in inversas(bl):
                p4.append((f, codigo[:ini].count('\n') + 1, tipo, fn, ln))
            for v, termos, ln in resto_relido(bl):
                p5.append((f, codigo[:ini].count('\n') + 1, v, termos, ln))
            for v in VERED.finditer(bl):
                txt = texto_do_veredicto(bl, v.start())
                # só a CONDIÇÃO: o que vem depois da última string do veredicto
                fim = txt.rfind('"')
                cond = txt[fim+1:] if fim >= 0 else txt
                nus = [n for n in NU.findall(cond) if int(n) >= 3]
                decs = REGUA_DEC.findall(cond)
                if decs:
                    linha = codigo[:ini + v.start()].count('\n') + 1
                    p2.append((f, linha, sorted(set(decs)), 'REGUA decimal'))
                if nus:
                    linha = codigo[:ini + v.start()].count('\n') + 1
                    p2.append((f, linha, sorted(set(nus), key=int), especie(bl, cond, nus)))
    print(f"  {len(fich)} ficheiros, {vistos} blocos\n")
    print(f"  P1 — JANELA DESLIZANTE (o defeito exacto): {len(p1)}")
    for f, l, t in p1:
        print(f"      {f}:{l}   {t}")
    if not p1:
        print("      nenhuma — nenhum laço começa a contar para trás a partir do fim")
    fich_p2 = len({f for f, *_ in p2})
    por_esp = collections.Counter(e for *_, e in p2)
    print(f"\n  P2 — literal NU na condição do veredicto: {len(p2)}"
          f" em {fich_p2} ficheiros")
    print("      por espécie: " + ", ".join(f"{k} {v}" for k, v in por_esp.most_common()))
    for esp in ('COPIADA', 'REGUA decimal', 'REGUA'):
        do_esp = [x for x in p2 if x[3] == esp]
        if not do_esp:
            continue
        print(f"\n      ── {esp} ({len(do_esp)}) " + "─"*40)
        for f, l, t, _ in do_esp[:22]:
            print(f"      {f}:{l}   {', '.join(t)}")
        if len(do_esp) > 22:
            print(f"      ... e mais {len(do_esp)-22}")
    print("\n  CONTAGEM é o total da varredura e é legítimo — diz quantos casos entraram, e"
          "\n  sem ele a asserção passaria por a varredura estar vazia. Mas devia ser uma"
          "\n  EXPRESSÃO e não o produto já feito: 15⁴ e não 50625, porque a conta que não"
          "\n  erra é a que o compilador faz."
          "\n  RÉGUA é o que esta casa persegue: vem de uma fórmula sobre o objecto, ou foi"
          "\n  escolhida até passar?"
          "\n  COPIADA é a pior: um decimal numa igualdade é uma referência escrita à mão.")
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
    fich_p4 = len({f for f, *_ in p4})
    print(f"\n  P4 — a função e a sua INVERSA na mesma comparação: {len(p4)}"
          f" em {fich_p4} ficheiros")
    for f, l, tipo, fn, ln in p4[:40]:
        print(f"      {f}:{l}   [{tipo}] {fn}   →   {ln}")
    if len(p4) > 40:
        print(f"      ... e mais {len(p4)-40}")
    if not p4:
        print("      nenhuma — nenhuma composição f∘f⁻¹ dentro de uma comparação")
    print("\n  Medir f(f⁻¹(x)) = x não mede nada: é a definição do par relida. Quando os"
          "\n  DOIS lados passam pela mesma função, ela cancela-se e o que fica é a"
          "\n  igualdade de dentro — que costuma ser INTEIRA. Ou se mede essa, ou nada.")
    fich_p5 = len({f for f, *_ in p5})
    print(f"\n  P5 — a quantidade definida como O RESTO: {len(p5)}"
          f" em {fich_p5} ficheiros")
    for f, l, v, termos, ln in p5[:40]:
        print(f"      {f}:{l}   {v} = resto de ({termos})   →   {ln}")
    if len(p5) > 40:
        print(f"      ... e mais {len(p5)-40}")
    if not p5:
        print("      nenhuma — nenhum resto reaparece na condição que o define")
    print("\n  O resto foi CONSTRUÍDO para fechar, e fecha: numa IGUALDADE a soma dele com"
          "\n  as partes dá o total por definição, e essa não pode falhar. Numa DESIGUALDADE"
          "\n  o que sobra é a passividade — as partes não excedem o total —, e essa mede."
          "\n  Ler a etiqueta antes de mexer.")
    return 1 if p1 else 0


if __name__ == '__main__':
    sys.exit(main())
