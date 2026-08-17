#!/usr/bin/env python3
# residuo_zero.py — ONDE O RESÍDUO É ZERO EXACTO E MESMO ASSIM HÁ UM LIMIAR.
#
# Em 17/08 apanhei três vezes o mesmo defeito, em três ficheiros diferentes: uma asserção
# que compara DUAS EXPRESSÕES QUE SÃO A MESMA, com um limiar por cima a dar-lhe cara de
# medida.
#
#     continua.c §C4   fabs(r1 − (−sl)) < 1e-12, com −sl a ser r1 letra por letra
#     xx.c §X1         dois laços idênticos, e depois a==b  (e `if(1==1 && 1==1)`)
#     moe.c §M3        Σ W[i][j]x[i]  contra  Σ 1.0*Wm[0][i][j]*x[i], bit a bit
#
# O `gume.py` não os apanha: mutar `==` para `!=` derruba a asserção na mesma, logo ela
# não aparece como sobrevivente. Precisa-se de outro sinal, e há um barato:
#
#     DOIS CAMINHOS GENUINAMENTE DIFERENTES EM VÍRGULA FLUTUANTE QUASE NUNCA DÃO
#     RESÍDUO EXACTAMENTE ZERO.
#
# Somas na mesma ordem com os mesmos valores dão zero exacto; somas por rotas diferentes
# dão 1e-16, 3e-15, alguma coisa. Então: um medidor que imprime «pior desvio 0.000e+00»
# e tem uma asserção com limiar é candidato — ou a comparação é de uma expressão consigo
# própria, ou o limiar não tinha razão de existir e a igualdade era exacta desde início.
#
# NÃO É VEREDICTO, como o gume.py não é. As duas saídas legítimas existem:
#   · a conta é inteira e cabe no double — aí o zero é real, e o limiar é que sobra;
#   · o objecto é simétrico e o resíduo anula-se por estrutura.
# Nos dois casos a acção é a mesma: TIRAR O LIMIAR, e afirmar a igualdade exacta.
#
#   uso:  python3 tools/residuo_zero.py [medidor.c ...]

import re, subprocess, sys, os, glob, tempfile, shutil

RAIZ = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TMP  = tempfile.mkdtemp(prefix='rz_')

# as palavras com que a casa nomeia um resíduo, e o valor a seguir
PALAVRA = re.compile(
    r'(resíduo|residuo|erro|desvio|diferença|diferenca|pior|dif|delta|gap)[^0-9\-+]{0,40}'
    r'([-+]?\d+\.?\d*(?:[eE][-+]?\d+)?)', re.I)

def e_zero(txt):
    try:
        return float(txt) == 0.0
    except ValueError:
        return False

def limiares_do_fonte(caminho):
    """os limiares que aparecem DENTRO da condição de um ok(...) — não em comentários."""
    s = open(caminho, encoding='utf-8', errors='replace').read()
    out = []
    for mo in re.finditer(r'\bok\s*\(', s):
        i = mo.end(); d = 1; j = i
        while j < len(s) and d > 0:
            if s[j] == '(': d += 1
            elif s[j] == ')': d -= 1
            j += 1
        arg = s[i:j]
        k = arg.rfind('"')
        txt, cond = (arg[:k+1], arg[k+1:]) if k > 0 else ('', arg)
        for m2 in re.finditer(r'\b\d*\.?\d+[eE]-\d+\b', cond):
            out.append((s[:mo.start()].count('\n') + 1, m2.group(0), txt[1:90]))
    return out

def main():
    alvos = sys.argv[1:] or sorted(glob.glob(os.path.join(RAIZ, 'tests', '*.c')))
    print("  RESÍDUO ZERO EXACTO com limiar na asserção — onde olhar")
    print("  (não é veredicto: a conta pode ser inteira. Mas então o limiar é que sobra.)\n")
    achados = 0
    for alvo in alvos:
        base = os.path.basename(alvo)[:-2]
        lim = limiares_do_fonte(alvo)
        if not lim:
            continue
        bina = os.path.join(TMP, base)
        cc = subprocess.run(['cc', '-O2', '-std=c99', '-I', os.path.join(RAIZ, 'lib'),
                             '-I', os.path.join(RAIZ, 'tests'), '-o', bina, alvo, '-lm'],
                            capture_output=True)
        if cc.returncode != 0 or not os.path.exists(bina):
            continue
        try:
            r = subprocess.run([bina], capture_output=True, timeout=120,
                               cwd=os.path.join(RAIZ, 'tests'))
        except subprocess.TimeoutExpired:
            continue
        saida = r.stdout.decode('utf-8', 'replace')
        # o rodapé da unidade ESCREVE «RESIDUO 0» e «0 falha(s)» — apanhá-los era ler o
        # veredicto como se fosse uma medida, e enchia a lista de ruído.
        RUIDO = re.compile(r'unidade\(s\)|#UNIT|falha\(s\)|RESIDUO 0|RESÍDUO 0|^§|sim ✓|NÃO ✗')
        zeros = [(L.strip()[:88], m.group(2))
                 for L in saida.split('\n')
                 if not RUIDO.search(L.strip())
                 for m in [PALAVRA.search(L)] if m and e_zero(m.group(2))]
        if not zeros:
            continue
        achados += 1
        print(f"  ── {base}: {len(zeros)} linha(s) com resíduo ZERO, "
              f"e {len(lim)} limiar(es) a decidir")
        for L, v in zeros[:4]:
            print(f"       {L}")
        for ln, v, t in lim[:3]:
            print(f"       limiar {v} na linha {ln}: {t}")
        print()
    print(f"  candidatos: {achados}")
    return 0

if __name__ == '__main__':
    try:
        sys.exit(main())
    finally:
        shutil.rmtree(TMP, ignore_errors=True)
