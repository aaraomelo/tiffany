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
N=int(__import__("os").environ.get("NDIM","64"))
# ── A BASE ESTAVA INCOMPLETA: FALTAVA A TERCEIRA COORDENADA ────────────────────────
#
# O Aarao: "completa a dimensao, poe mais um vetor na base — ela e' projeccao de uma
# dimensao acima."
#
# A decomposicao par/impar sob k -> n-k tem DOIS pedacos, e o trial tem TRES estados. O que
# falta e' o PONTO FIXO: os indices com k == (n-k) mod n, que sao k=0 e k=n/2. Neles a parte
# impar e' zero POR CONSTRUCAO — nao porque o vector nao tenha la' nada, mas porque a
# projeccao nao os ve. Sao a coordenada 0 do trial, e estavam a ser somados a' parte par
# como se fossem simetricos.
#
# Separa-los e' acrescentar o vector que faltava a' base: par, impar, FIXO.
def parimparfixo(x):
    n=len(x)
    fix = [k for k in range(n) if k == (n-k)%n]          # k=0 e k=n/2
    p = [0.0]*n; i = [0.0]*n; f = [0.0]*n
    for k in range(n):
        if k in fix: f[k] = x[k]                          # o ponto fixo, sozinho
        else:
            p[k] = (x[k]+x[(n-k)%n])/2
            i[k] = (x[k]-x[(n-k)%n])/2
    return p, i, f
def parimpar(x):
    p,i,f = parimparfixo(x)
    return ([p[k]+f[k] for k in range(len(p))], i)        # a projeccao ANTIGA, para comparar
def res_dual(xs,xa):
    ps,is_=parimpar(xs); pa,ia=parimpar(xa)
    num=sum((pa[k]-ps[k])**2 for k in range(len(ps)))+sum((ia[k]+is_[k])**2 for k in range(len(ps)))
    return math.sqrt(num/sum(ps[k]**2+is_[k]**2 for k in range(len(ps))))
def res_id(xs,xa):     # ν∘ν = id: a volta tem de dar o MESMO, não o oposto
    ps,is_=parimpar(xs); pa,ia=parimpar(xa)
    num=sum((pa[k]-ps[k])**2 for k in range(len(ps)))+sum((ia[k]-is_[k])**2 for k in range(len(ps)))
    return math.sqrt(num/sum(ps[k]**2+is_[k]**2 for k in range(len(ps))))
def res_id3(xs,xa):    # o MESMO, na base COMPLETA: par, impar e fixo
    ps,is_,fs=parimparfixo(xs); pa,ia,fa=parimparfixo(xa)
    n=len(ps)
    num=(sum((pa[k]-ps[k])**2 for k in range(n))
        +sum((ia[k]-is_[k])**2 for k in range(n))
        +sum((fa[k]-fs[k])**2 for k in range(n)))
    den=sum(ps[k]**2+is_[k]**2+fs[k]**2 for k in range(n))
    return math.sqrt(num/den)

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
# NAO DECLARAR: COMPARAR. O menor residuo e' o percurso que mais se aproxima de fechar, e
# so' vale alguma coisa se ficar ABAIXO do controlo — senao nao fechou nenhum.
# ── RECONSTRUIR O PONTO FIXO A PARTIR DOS ESPELHOS ─────────────────────────────────
#
# O Aarao: "tenta reconstruir o ponto fixo, pq temos os espelhos, e via involucao e 3
# numeros conseguimos reconstruir."
#
# Uma involucao afim e' uma REFLEXAO: nu(x) = 2c - x, com c o ponto fixo. Logo
#
#     c = (x + nu(x)) / 2
#
# e' a MEDIA de qualquer par de espelhos. Nao e' preciso ler os pesos: o centro le-se das
# saidas. E ha' um teste que decide se ha' mesmo um ponto fixo — se a involucao for a
# mesma, os centros de TODOS os pares consecutivos tem de COINCIDIR.
def centro(u,v): return [(u[k]+v[k])/2 for k in range(len(u))]
def dist(u,v):   return math.sqrt(sum((u[k]-v[k])**2 for k in range(len(u))))
def norma(u):    return math.sqrt(sum(u[k]*u[k] for k in range(len(u))))

c1 = centro(x1, xa)          # de S1 e do 1.o espelho
c2 = centro(xa, x_meio)      # do 1.o e do 2.o
c3 = centro(x_meio, x_tres)  # do 2.o e do 3.o
c4 = centro(x_tres, x2)      # do 3.o e do 4.o
cm = [ (c1[k]+c2[k]+c3[k]+c4[k])/4 for k in range(len(c1)) ]

print()
print("  O PONTO FIXO, RECONSTRUIDO DOS ESPELHOS (c = (x + nu(x))/2):")
print(f"    |c1-c2| = {dist(c1,c2):.6f}   |c2-c3| = {dist(c2,c3):.6f}   |c3-c4| = {dist(c3,c4):.6f}")
esp = max(dist(c1,cm),dist(c2,cm),dist(c3,cm),dist(c4,cm))
print(f"    dispersao dos 4 centros em torno da media: {esp:.6f}   (|c| = {norma(cm):.6f})")
print(f"    razao dispersao/|c| = {esp/norma(cm):.4f}   <- se for pequena, HA' ponto fixo")
print()
print("  E AS DISTANCIAS AO CENTRO — numa orbita, sao todas iguais:")
for nome, v in [("S1",x1),("p1",xa),("p2",x_meio),("p3",x_tres),("p4",x2)]:
    print(f"    |{nome} - c| = {dist(v,cm):.6f}")
raios = [dist(v,cm) for v in (x1,xa,x_meio,x_tres,x2)]
disp_r = (max(raios)-min(raios))/ (sum(raios)/len(raios))
print(f"    dispersao dos raios: {disp_r*100:.2f}%  <- se for pequena, os 5 estao na MESMA esfera")
# e SEM o S1: os quatro ESPELHOS entre si. Se eles estiverem na mesma esfera e o S1 fora,
# entao o S1 nao pertence a' orbita — a primeira aplicacao levou-o para outro sitio, e o
# ponto fixo que se reconstroi e' o dos espelhos, nao o do par (S1, p1).
ce = [ (xa[k]+x_meio[k]+x_tres[k]+x2[k])/4 for k in range(len(xa)) ]
re_ = [dist(v,ce) for v in (xa,x_meio,x_tres,x2)]
d_e = (max(re_)-min(re_))/(sum(re_)/len(re_))
print()
print("  E SO' OS QUATRO ESPELHOS, sem o S1:")
for nome, v in [("p1",xa),("p2",x_meio),("p3",x_tres),("p4",x2)]:
    print(f"    |{nome} - ce| = {dist(v,ce):.6f}")
print(f"    dispersao dos raios dos ESPELHOS: {d_e*100:.2f}%")
print(f"    e o S1 fica a {dist(x1,ce):.6f} do centro deles — "
      f"{dist(x1,ce)/(sum(re_)/len(re_)):.2f}x o raio")

# ── O CENTRO QUE DA' RESIDUO 0 ─────────────────────────────────────────────────────
#
# O Aarao: "o centro que da' residuo 0 e' o que interessa pra gente."
#
# O centro reconstruido pela media dos espelhos e' o de MINIMOS QUADRADOS da hipotese
# "nu e' reflexao pura". Mas a pergunta e' outra: existe um c que faca o residuo IR A
# ZERO? Procura-se, em vez de se supor — descida directa sobre c, com o residuo centrado
# como funcao objectivo.
def res_c(c, u, v):
    uu = [u[k]-c[k] for k in range(len(u))]
    vv = [v[k]-c[k] for k in range(len(v))]
    return res_id(uu, vv)

def procura_centro(u, v, c0, passos=400, lr=0.35):
    c = list(c0); melhor = res_c(c,u,v); h = 1e-3
    for it in range(passos):
        g = []
        for k in range(len(c)):
            c[k] += h; f1 = res_c(c,u,v); c[k] -= 2*h; f2 = res_c(c,u,v); c[k] += h
            g.append((f1-f2)/(2*h))
        gn = math.sqrt(sum(x*x for x in g)) or 1.0
        cn = [c[k] - lr*g[k]/gn for k in range(len(c))]
        r = res_c(cn,u,v)
        if r < melhor: c, melhor = cn, r
        else: lr *= 0.6
        if lr < 1e-6: break
    return c, melhor

print()
print("  O CENTRO QUE DA' RESIDUO 0 — procurado, nao suposto:")
for nome, u, v in [("tres passos", x1, x_tres), ("quatro passos", x1, x2)]:
    r0 = res_c([0.0]*len(x1), u, v)
    rm = res_c(cm, u, v)
    c_opt, r_opt = procura_centro(u, v, cm)
    print(f"    {nome:>14}:  sem centro {r0:.6f}   com a media {rm:.6f}   "
          f"OPTIMO {r_opt:.6f}")
    print(f"    {'':>14}   e |c_optimo| = {norma(c_opt):.4f}, a {dist(c_opt,cm):.4f} da media")
    # ── O CONTROLO, E E' OBRIGATORIO: um c GRANDE E QUALQUER tambem baixa o residuo?
    # O residuo e' uma razao. Com |c| enorme, x-c ~ -c para todo x, e numerador e
    # denominador ficam ambos dominados por c: a razao vai a zero SEM QUERER DIZER NADA.
    # Se o controlo baixar tanto como o optimo, o optimo nao provou coisa nenhuma.
    import random as _r; _r.seed(11)
    piores = []
    for _ in range(5):
        d = [_r.gauss(0,1) for _ in range(len(x1))]
        dn = norma(d) or 1.0
        c_r = [ d[k]/dn*norma(c_opt) for k in range(len(d)) ]   # MESMA norma, direccao ao acaso
        piores.append(res_c(c_r, u, v))
    print(f"    {'':>14}   CONTROLO: c ao acaso com a MESMA norma da' {min(piores):.6f}"
          f" a {max(piores):.6f}")
    if min(piores) <= r_opt*3:
        print(f"    {'':>14}   -> DEGENERESCENCIA: qualquer c grande baixa o residuo."
              f" O optimo NAO diz nada.")
    else:
        print(f"    {'':>14}   -> o optimo e' {min(piores)/r_opt:.1f}x melhor que o acaso"
              f" de mesma norma: a DIRECCAO importa.")

# e o mesmo na BASE COMPLETA, com a terceira coordenada
t2, t3, t4, tc = (res_id3(x1,x_meio), res_id3(x1,x_tres), res_id3(x1,x2), res_id3(x1,xa))
print()
print("  NA BASE COMPLETA (par, impar e FIXO — a coordenada que faltava):")
for nome, v in [("dois",t2),("tres",t3),("quatro",t4)]:
    print(f"  {nome:>7} passos  {v:.6f}   {'ABAIXO do controlo' if v < tc else 'acima do controlo'}")
print(f"  controlo        {tc:.6f}")

cands = [("dois", r_2p), ("tres", r_3p), ("quatro", r_id)]
melhor = min(cands, key=lambda t: t[1])
print()
for nome, v in cands:
    print(f"  {nome:>7} passos  {v:.6f}   {'ABAIXO do controlo' if v < r_ctl else 'acima do controlo'}")
print(f"  controlo        {r_ctl:.6f}")
print()
if melhor[1] < r_ctl:
    print(f"  O MELHOR E' {melhor[0].upper()} ({melhor[1]:.6f}), e e' o unico abaixo do controlo.")
else:
    print(f"  NENHUM FECHA: o melhor e' {melhor[0]} ({melhor[1]:.6f}) e ainda esta' acima do")
    print(f"  controlo ({r_ctl:.6f}). A involucao em linguagem nao fecha por este caminho.")
open("dados/colhido/tresp.txt","w").write(
  f"{r1a:.6f}\t{ra2:.6f}\t{r_id:.6f}\t{r_ctl:.6f}\n{s1}\n{a}\n{s2}\n")
PY
