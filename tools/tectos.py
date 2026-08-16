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


def texto_do_veredicto(src, i):
    """devolve o texto do ok(...) que começa no índice i, seguindo parênteses"""
    n, j, dentro = 0, i, False
    while j < len(src):
        c = src[j]
        if c == '"':                      # salta a string, contando-a
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
    p1, p2, vistos = [], [], 0
    for f in fich:
        try:
            src = open(f, encoding='utf-8').read()
        except Exception:
            continue
        # o comentário do próprio detector, e as menções ao defeito já corrigido, não
        # contam: só o CÓDIGO. Tira-se comentário de bloco e de linha.
        codigo = re.sub(r'/\*.*?\*/', ' ', src, flags=re.S)
        codigo = re.sub(r'//[^\n]*', ' ', codigo)
        for m in JANELA.finditer(codigo):
            linha = codigo[:m.start()].count('\n') + 1
            p1.append((f, linha, m.group(0)))
        for ini, bl in blocos(codigo):
            vistos += 1
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
    return 1 if p1 else 0


if __name__ == '__main__':
    sys.exit(main())
