#!/usr/bin/env python3
"""doubles.py — o levantamento dos doubles, pelo que ALIMENTAM.

O número bruto de doubles nunca foi o trabalho. A pergunta que decide é outra:
a variável que recebe a vírgula aparece na condição de algum `ok(...)`?

  - se NÃO aparece, a vírgula morre num printf — é a representação a fazer-se no
    fim, que é a regra da casa e não uma excepção a ela;
  - se APARECE, ela decide se o medidor fica verde, e aí a pergunta é se a
    vírgula é do MUNDO (uma condutividade, um dBm) ou minha.

Foi esta classificação que reduziu «272 raízes» a 19 e «729 transcendentais» a 18.

RÉGUA: nomes DISTINTOS por ficheiro. Contar ocorrências dá quase o dobro (5173
contra 2875) e as duas contagens não se misturam na mesma frase.

    python3 tools/doubles.py            resumo
    python3 tools/doubles.py -v         com os ficheiros
    python3 tools/doubles.py --puros    os que nunca tocam vírgula (candidatos a inteiro)
"""
import re, sys, glob, collections

FIS = re.compile(r'\b(W/mK|S/m|ohm|volt|Volt|watt|Watt|kelvin|Kelvin|dBm?|Hz|GHz|MHz'
                 r'|joule|Joule|amp[eè]re|kappa|temperatura|condutividade|ru[ií]do|ganho'
                 r'|tens[aã]o|corrente|resist[eê]ncia|t[eé]rmic|radia[cç][aã]o|solar'
                 r'|fot[oó]n|el[eé]tr|eletr)\b')
# o que mete vírgula numa variável que começou inteira
SUJO = re.compile(r'\b(sqrt|pow|exp|log|log10|log2|sin|cos|tan|atan|atan2|acos|asin|hypot'
                  r'|cbrt|tgamma|erf|fabs|fmax|fmin|fmod|ceil|floor|round|M_PI|M_E'
                  r'|drand48|rand)\b')
FICHEIROS = sorted(glob.glob('tests/*.c')) + sorted(glob.glob('tools/*.c')) + sorted(glob.glob('lib/*.h'))


def condicoes(src):
    """o texto dentro de cada ok(...) / tique(...) — onde uma variável DECIDE."""
    out = []
    for m in re.finditer(r'\b(?:ok|tique)\s*\(', src):
        i = m.end(); d = 1; j = i
        while j < len(src) and d > 0:
            if src[j] == '(': d += 1
            elif src[j] == ')': d -= 1
            j += 1
        out.append(src[i:j])
    return '\n'.join(out)


def declaracoes(src):
    """nome -> linha, para cada double/float declarado. Nomes DISTINTOS."""
    nomes = {}
    for i, l in enumerate(src.split('\n')):
        if re.match(r'\s*(//|\*|/\*)', l): continue
        for m in re.finditer(r'\b(?:static\s+|const\s+)*(?:double|float)\s+([^;=]*(?:=[^;]*)?);', l):
            for nm in re.findall(r'\b([a-zA-Z_]\w*)\s*(?:\[[^\]]*\])?\s*(?:=|,|$)', m.group(1)):
                if nm in ('const', 'static', 'struct', 'return', 'complex'): continue
                nomes.setdefault(nm, i + 1)
    return nomes


def toca_virgula(nome, linhas):
    """a variável recebe alguma vez algo que não seja inteiro?"""
    for l in linhas:
        if re.search(r'&\s*' + re.escape(nome) + r'\b', l): return True   # sai por ponteiro
        for m in re.finditer(r'\b' + re.escape(nome) + r'\s*(?:[-+*/]?=)(?!=)\s*([^;]+)', l):
            rhs = m.group(1)
            if SUJO.search(rhs) or re.search(r'\d+\.\d*[1-9]', rhs) or '/' in rhs:
                return True
    return False


def main():
    verbose = '-v' in sys.argv
    puros_só = '--puros' in sys.argv
    res = collections.defaultdict(lambda: [0, 0])
    det = collections.defaultdict(list)
    puros = []
    for f in FICHEIROS:
        src = open(f, encoding='utf-8', errors='replace').read()
        linhas = src.split('\n')
        fam = 'MUNDO FISICO' if len(FIS.findall(src)) >= 6 else 'matematica pura'
        txt = condicoes(src)
        nomes = declaracoes(src)
        alim = [nm for nm in nomes if re.search(r'\b' + re.escape(nm) + r'\b', txt)]
        res[fam][0] += len(nomes); res[fam][1] += len(alim)
        if alim: det[fam].append((len(alim), len(nomes), f))
        if puros_só:
            for nm in alim:
                m = re.search(r'\b(?:static\s+|const\s+)*(?:double|float)\s+' + re.escape(nm)
                              + r'\s*=\s*(-?\d+(?:\.0+)?)\s*[;,]', src)
                if m and not toca_virgula(nm, linhas):
                    puros.append((f, nomes[nm], nm, m.group(1)))

    if puros_só:
        print(f"{len(puros)} doubles que alimentam asserção e NUNCA tocam vírgula.\n")
        print("Os de valor ZERO são acumuladores que recebem por uma cadeia que este")
        print("detector não segue (`if(x > pior) pior = x;`) — legítimos. Os de valor")
        print("NÃO-ZERO são os candidatos a inteiro:\n")
        for f, ln, nm, v in puros:
            if float(v) != 0: print(f"   {f}:{ln}  {nm} = {v}")
        return

    print("LEVANTAMENTO DOS DOUBLES — pelo que ALIMENTAM   (régua: nomes distintos)\n")
    print(f"{'família':<18} {'declarados':>11} {'alimentam':>10} {'%':>5}")
    for fam in ('MUNDO FISICO', 'matematica pura'):
        n, a = res[fam]
        print(f"{fam:<18} {n:>11} {a:>10} {100*a//n if n else 0:>4}%")
    tn = sum(v[0] for v in res.values()); ta = sum(v[1] for v in res.values())
    print(f"{'TOTAL':<18} {tn:>11} {ta:>10} {100*ta//tn:>4}%\n")
    print("O trabalho está na linha do meio: no MUNDO FÍSICO a vírgula é do mundo")
    print("(uma condutividade de 6,3e7 S/m não é um inteiro disfarçado); na matemática")
    print("pura, um double que decide uma asserção tem de apresentar a sua razão.\n")
    if verbose:
        for fam in ('matematica pura', 'MUNDO FISICO'):
            print(f"{fam}:")
            for a, n, f in sorted(det[fam], reverse=True)[:15]:
                print(f"   {a:4d} / {n:<4d}  {f}")
            print()


if __name__ == '__main__':
    main()
