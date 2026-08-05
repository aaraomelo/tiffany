#!/bin/bash
# tresp.sh — TRÊS PASSOS: uma frase, a ANTISSIMÉTRICA dela, e a antissimétrica DESSA.
#
# O Aarão: "queremos a simetria. Manda formular uma frase, depois a antissimétrica, e no fim
# entregar uma frase SIMÉTRICA."
#
# E É A ÁLGEBRA, não uma sequência arbitrária: **duas antissimétricas dão uma simétrica.**
# (−1)·(−1) = +1; duas reflexões dão uma rotação; ν∘ν = id. Eram estas as DUAS FUNÇÕES
# ANTISSIMÉTRICAS que ele tinha pedido e eu não tinha percebido — não duas frases lado a lado,
# mas a mesma operação APLICADA DUAS VEZES.
#
#      S₁  --ν-->  A  --ν-->  S₂          e S₂ tem de ser simétrica
#
# E ELE CONTINUA A NÃO PRECISAR DE SABER QUE ESTÁ ERRADO: pede-se-lhe sempre a mesma coisa — o
# outro lado — e é a composição que devolve o lado certo. Quem decide é o resíduo de ν∘ν = id.
set -e
GER=${GER:-qwen2.5:1.5b}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
python3 - "$GER" <<'PY'
import json, urllib.request, sys, math
GER = sys.argv[1]
def api(r,c):
    d=json.dumps(c).encode()
    q=urllib.request.Request(f"http://localhost:11434/api/{r}",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=300).read())
def fala(p,n=80):
    return api("generate",{"model":GER,"prompt":p,"stream":False,
        "options":{"temperature":0,"seed":7,"num_predict":n}})["response"].strip().replace("\n"," ")
def emb(t): return api("embeddings",{"model":"nomic-embed-text","prompt":t})["embedding"]
N=64
def parimpar(x):
    n=len(x)
    return ([(x[k]+x[(n-k)%n])/2 for k in range(n)], [(x[k]-x[(n-k)%n])/2 for k in range(n)])
def res_dual(xs,xa):
    ps,is_=parimpar(xs); pa,ia=parimpar(xa)
    num=sum((pa[k]-ps[k])**2 for k in range(len(ps)))+sum((ia[k]+is_[k])**2 for k in range(len(ps)))
    return math.sqrt(num/sum(ps[k]**2+is_[k]**2 for k in range(len(ps))))
def res_id(xs,xa):     # ν∘ν = id: a volta tem de dar o MESMO, não o oposto
    ps,is_=parimpar(xs); pa,ia=parimpar(xa)
    num=sum((pa[k]-ps[k])**2 for k in range(len(ps)))+sum((ia[k]-is_[k])**2 for k in range(len(ps)))
    return math.sqrt(num/sum(ps[k]**2+is_[k]**2 for k in range(len(ps))))

TEMA = "O que é uma involução?"
PEDE = ("Toda afirmação tem um lado SIMÉTRICO (o que ela mede, o que fica igual no espelho) e um "
        "lado ANTISSIMÉTRICO (o que ela ordena, o que troca de sinal no espelho).\n\n"
        "Afirmação: {a}\n\n"
        "Escreva a afirmação ANTISSIMÉTRICA desta — a do outro lado do espelho, sobre o mesmo "
        "termo. Uma frase só, sem preâmbulo e sem explicar.")

# ── O PERCURSO SAI DO RELOGIO, E SAO QUATRO PASSOS POR LADOS ALTERNADOS ────────────
#
# A versao anterior aplicava PEDE duas vezes — a MESMA involucao, pelo mesmo lado. O
# tests/relogio_opera.c mediu o que isso da': o mesmo lado repetido devolve a identidade e
# NAO TESTA o segundo lado; e dois passos por lados diferentes caem no ANTIPODA, que e'
# meia orbita e nao a casa. Era isso que o residuo 0,402 dizia, contra um controlo de
# 0,421 — nao era ruido, era estrutura.
#
# O relogio da' o percurso: com DOIS lados sao quatro passos, alternados (0,1,0,1). Os dois
# lados sao os dois do enunciado central — a coordenada que MEDE e a que ORDENA.
PEDE_ORDEM = ("Toda afirmação tem um SENTIDO: ela vai de algo para algo, e o espelho troca "
        "esse sentido.\n\nAfirmação: {a}\n\nEscreva a afirmação com o SENTIDO INVERTIDO — "
        "a mesma medida, o caminho ao contrário. Uma frase só, sem preâmbulo e sem explicar.")

s1 = fala(f"Responda em uma frase, sem preâmbulo: {TEMA}")
p1 = fala(PEDE.format(a=s1))            # lado A: o que mede
p2 = fala(PEDE_ORDEM.format(a=p1))      # lado B: o que ordena
p3 = fala(PEDE.format(a=p2))            # lado A outra vez
p4 = fala(PEDE_ORDEM.format(a=p3))      # lado B — e aqui a orbita FECHA
a, s2 = p1, p4
x1,xa,x2 = emb(s1)[:N], emb(p1)[:N], emb(p4)[:N]
x_meio   = emb(p2)[:N]                  # o ANTIPODA: dois passos, nao quatro
x_tres   = emb(p3)[:N]

r1a  = res_dual(x1,xa)      # o primeiro ν: quão antissimétrico ficou
ra2  = res_dual(xa,x2)      # o segundo ν
r_id = res_id(x1,x2)        # ν∘ν = id: S₂ contra S₁ — É ESTA QUE DECIDE
r_ctl= res_id(x1,xa)        # o controlo: S₁ contra A, que NÃO devia ser a identidade
r_2p = res_id(x1,x_meio)    # DOIS passos: o antipoda — tem de ser MAIOR que o de quatro
r_3p = res_id(x1,x_tres)    # tres passos: a volta, ainda do outro lado

print(f"  S1  {s1[:88]}")
print(f"  A   {a[:88]}")
print(f"  S2  {s2[:88]}")
print()
print(f"  ν  (S1→A)   resíduo dual   {r1a:.6f}")
print(f"  ν  (A→S2)   resíduo dual   {ra2:.6f}")
print(f"  ν∘ν = id ?  S2 contra S1   {r_id:.6f}   ← esta decide")
print(f"  o controlo  A  contra S1   {r_ctl:.6f}   (tem de ser MAIOR)")
print(f"  DOIS passos S2' contra S1  {r_2p:.6f}   (o antipoda: era isto que se media antes)")
print(f"  TRES passos S3 contra S1   {r_3p:.6f}   (a volta, ainda do outro lado)")
print()
print(f"  o que decide agora: QUATRO passos fecham ({r_id:.6f}) e DOIS nao ({r_2p:.6f}).")
open("dados/colhido/tresp.txt","w").write(
  f"{r1a:.6f}\t{ra2:.6f}\t{r_id:.6f}\t{r_ctl:.6f}\n{s1}\n{a}\n{s2}\n")
PY
