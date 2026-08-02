#!/bin/bash
# antissimetrica.sh — DÁ-SE-LHE O ESPECTRO, PEDE-SE A ANTISSIMÉTRICA, E ITERA-SE ATÉ O RESÍDUO CAIR.
#
# O Aarão: "fornece o espectro pra ele da frase dele, mostra em Fourier e Mellin dualizado,
# explica, aí pede pra ele formular a frase ANTISSIMÉTRICA e iterar até dar erro 0. Ela não
# precisa saber que está errado — precisa de duas funções antissimétricas. Você pode fornecer as
# duas formas polinomiais, porque você tem o corpo diferencial."
#
# E ISTO RESOLVE O QUE O `entrega.c` MEDIU. Lá ele falhou a corrigir porque corrigir exige saber a
# resposta — e ele não sabe. Aqui não se lhe pergunta o que está certo: pede-se-lhe o DUAL, e quem
# decide é o resíduo. *Ele não precisa de saber que está errado.*
#
#   o que se lhe dá     o ESPECTRO da própria frase, nas duas formas polinomiais
#                       ⊕ Fourier  (o aditivo: os modos)      — a forma algébrica
#                       ⊗ Mellin   (o multiplicativo: a escala) — a forma polar
#   o que se lhe pede   a frase ANTISSIMÉTRICA — o outro lado do par
#   quem decide         o resíduo, e ele ITERA até cair
#
# As duas formas polinomiais somos nós que as damos, porque é o corpo diferencial que as tem: o
# embedding É um polinómio (dualcifra.c §W-polinómios), e um polinómio tem as duas leituras.
set -e
GER=${GER:-qwen2.5:1.5b}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
python3 - "$GER" <<'PY'
import json, urllib.request, sys, cmath, math
GER = sys.argv[1]
def api(rota, corpo):
    d = json.dumps(corpo).encode()
    r = urllib.request.Request(f"http://localhost:11434/api/{rota}", d, {"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(r, timeout=300).read())
def fala(p, n=90):
    return api("generate", {"model":GER,"prompt":p,"stream":False,
        "options":{"temperature":0.35,"seed":7,"num_predict":n}})["response"].strip()
def emb(t): return api("embeddings", {"model":"nomic-embed-text","prompt":t})["embedding"]

N = 64                       # os modos que mostramos: os primeiros do espectro
def dft(x):
    n = len(x)
    return [sum(x[j]*cmath.exp(-2j*math.pi*k*j/n) for j in range(n))/math.sqrt(n) for k in range(n)]
def espectro(v):
    x = v[:N]
    F = dft(x)
    # FOURIER: o aditivo — módulo e fase dos modos dominantes
    # MELLIN: o multiplicativo — a escala, log|F| (exp∘Σ∘log é a ponte)
    mods = [abs(c) for c in F]
    fases = [cmath.phase(c) for c in F]
    mel = [math.log(m) if m > 1e-12 else -30.0 for m in mods]
    return F, mods, fases, mel

def parimpar(x):
    """A DECOMPOSIÇÃO QUE INTERESSA: x = par + ímpar, em torno do índice 0, circularmente.
    O PAR é o que fica igual no espelho (o simétrico, o que MEDE);
    o ÍMPAR é o que TROCA DE SINAL (o antissimétrico, o que ORDENA).
    É a partição B = B_s + B_a do projeto, aplicada ao sinal."""
    n = len(x)
    par = [(x[k] + x[(n-k) % n])/2 for k in range(n)]
    imp = [(x[k] - x[(n-k) % n])/2 for k in range(n)]
    return par, imp

def residuo(x_s, x_a):
    """O PRIMEIRO CRITÉRIO QUE ESCREVI ERA DEGENERADO e não desceu: pedia que o espectro de A
    fosse o CONJUGADO do de S — mas para um sinal REAL isso já é verdade por construção
    (F(N−k) = conj(F(k))). Eu estava a pedir o que já era verdade, e o número não tinha para
    onde descer. O critério certo é a decomposição par/ímpar:

        A antissimétrica de S tem a MESMA parte par e a parte ímpar TROCADA DE SINAL.

    Isso não é automático, e por isso pode descer."""
    ps, is_ = parimpar(x_s)
    pa, ia = parimpar(x_a)
    num = sum((pa[k]-ps[k])**2 for k in range(len(ps))) + sum((ia[k]+is_[k])**2 for k in range(len(ps)))
    den = sum(ps[k]**2 + is_[k]**2 for k in range(len(ps)))
    return math.sqrt(num/den)

# a frase errada dele, do interroga.sh
FRASE = ("Involução é um processo de diminuição ou enfraquecimento gradual de um órgão, "
         "tecido muscular ou estrutura anatômica.")
PERGUNTA = "O que é uma involução?"

v = emb(FRASE); F_s, mods, fases, mel = espectro(v)
x_s = v[:N]
ps, is_ = parimpar(x_s)
peso_par = math.sqrt(sum(t*t for t in ps)); peso_imp = math.sqrt(sum(t*t for t in is_))
top = sorted(range(N), key=lambda k: -mods[k])[:6]
lin_f = "  ".join(f"k={k}: |F|={mods[k]:.3f} ∠{fases[k]:+.2f}" for k in top)
lin_m = "  ".join(f"k={k}: log|F|={mel[k]:+.2f}" for k in top)

print(f"  a frase: {FRASE[:76]}")
print(f"  ⊕ FOURIER (aditivo):      {lin_f}")
print(f"  ⊗ MELLIN  (multiplicativo): {lin_m}")
print(f"  a partição: ‖par‖={peso_par:.4f} (o que MEDE)   ‖ímpar‖={peso_imp:.4f} (o que ORDENA)")
print()

# A EXPLICAÇÃO — e ela não diz que a frase está errada. Diz o que se quer.
BASE = (f"Uma afirmação tem duas partes: a SIMÉTRICA (o que ela mede, o que fica igual no espelho) "
        f"e a ANTISSIMÉTRICA (o que ela ordena, o que TROCA DE SINAL no espelho).\n\n"
        f"Pergunta original: {PERGUNTA}\n"
        f"Afirmação dada: {FRASE}\n\n"
        f"O espectro desta afirmação, nas duas formas polinomiais:\n"
        f"  Fourier (a soma, os modos):        {lin_f}\n"
        f"  Mellin  (o produto, as escalas):   {lin_m}\n"
        f"  a parte que MEDE (par) pesa {peso_par:.3f}; a que ORDENA (ímpar) pesa {peso_imp:.3f}\n\n"
        f"Escreva a afirmação ANTISSIMÉTRICA desta: a que ocupa o outro lado do par, "
        f"sobre o MESMO termo mas no domínio oposto (o formal em vez do orgânico). "
        f"Uma frase só, sem preâmbulo.")

melhor, melhor_frase, res0 = None, None, None
p = BASE
with open("/tmp/antissim.txt","w") as fo:
    for it in range(1, 7):
        a = fala(p).replace("\n"," ").strip()
        va = emb(a)
        r = residuo(x_s, va[:N])
        if it == 1: res0 = r
        if melhor is None or r < melhor: melhor, melhor_frase = r, a
        fo.write(f"{it}\t{r:.6f}\t{a}\n")
        print(f"  iteração {it}:  resíduo {r:.6f}   {a[:70]}")
        # ITERAR: dá-se-lhe o resíduo e o sentido. Ele não sabe o que está errado — sabe o número.
        p = (BASE + f"\n\nA tentativa anterior foi: {a}\n"
             f"O resíduo dela contra o dual foi {r:.4f} (quanto menor, melhor; 0 é exato).\n"
             f"Reescreva a afirmação antissimétrica para BAIXAR esse resíduo. Uma frase só.")
    fo.write(f"MELHOR\t{melhor:.6f}\t{melhor_frase}\n")
    fo.write(f"PRIMEIRO\t{res0:.6f}\n")
print(f"\n  primeiro resíduo {res0:.6f}  ->  melhor {melhor:.6f}  ({100*(res0-melhor)/res0:+.1f}%)")
print(f"  a melhor: {melhor_frase}")
PY
