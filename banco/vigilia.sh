#!/bin/bash
# vigilia.sh — O MONITOR DE VIGÍLIA: de 5 em 5 minutos, guarda a torre e alimenta o fluxo.
#
# Três coisas a cada disparo, e nenhuma delas avalia conteúdo:
#
#   1. GUARDA    se a torre morreu, relança-a — o fluxo não pode parar por um erro de rede
#   2. ALIMENTA  o nível 2 lê as revisões do nível 1 e SINTETIZA (não revê: consolida)
#   3. DIZ       uma linha só, com os números
#
# O QUE MATOU A CORRIDA DE 01/08, e que não se conserta aqui dentro. A vigília guardava a
# torre; ninguém guardava a vigília. Ela corria como Monitor da sessão do Claude, e às 22:19 a
# sessão acabou: a torre sobreviveu órfã mais seis minutos, morreu às 22:25:38, e não houve
# quem a relançasse até de manhã. O corpus vivia em /tmp — tmpfs — e o reboot das 07:31 apagou
# ~400 conceitos e ~395 revisões.
#
# Por isso o agendamento passou para um TIMER DO SYSTEMD (ver torre-vigilia.timer, ao lado):
# não tem pai que possa morrer, sobrevive ao logout com linger, e volta sozinho no arranque.
# Um guarda que depende de quem ele guarda não é um guarda.
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
N2=${N2:-gemma2:2b}
BANCO=${BANCO:-$CD/../.torre}
MMU=${MMU:-$CD/mmu}
NUM_CTX=${NUM_CTX:-1024}

[ -x "$MMU" ] || { echo "vigilia: falta o banco — compile com 'cc -O2 -std=c99 $CD/mmu.c -lm -o $MMU'" >&2; exit 1; }
mkdir -p "$BANCO"

topo(){ MMU_RAIZ="$BANCO/$1" "$MMU" topo 2>/dev/null || echo 0; }

# ── VIVA OU MORTA, pelo LOCK e não pelo nome ───────────────────────────────────────────────
# Se o subshell consegue segurar o lock, é porque ninguém o segurava: a torre está morta. O
# fd fecha-se à saída do subshell, portanto testar não é tomar. `pgrep -f torre.sh` fazia isto
# mal e em silêncio — casava com qualquer linha de comando que contivesse o nome (um tail, um
# editor, o shell que chamou a vigília), e o guarda dava a torre por viva sem ela existir.
LOCK="$BANCO/torre.lock"
torre_viva(){ ! ( flock -n 9 ) 9>"$LOCK"; }

# 1. GUARDA — setsid solta-a de qualquer sessão, para ela não voltar a morrer com um pai
# GUARDA=0 desliga o relançamento: serve para inspecionar os números sem pôr o modelo a
# trabalhar. Um medidor que muda o que mede não serve para conferir nada.
if [ "${GUARDA:-1}" = "1" ] && ! torre_viva; then
  setsid nohup env VOLTAS=400 REVISA_A_CADA=5 "$CD/torre.sh" >> "$BANCO/torre_saida.txt" 2>&1 &
  echo "vigilia: a torre tinha parado — RELANÇADA"
fi

n0=$(topo n0); n1=$(topo n1); n2=$(topo n2)

# 2. ALIMENTA — o nível 2 sintetiza as revisões do nível 1 que ainda não entraram
if [ "$n1" -ge $(( n2 * 4 + 4 )) ]; then
  MMU_RAIZ_N1="$BANCO/n1" MMU_RAIZ_N2="$BANCO/n2" \
  python3 - "$N2" "$MMU" "$BANCO" "$NUM_CTX" <<'PY' 2>/dev/null
import json, urllib.request, sys, os, subprocess
N2, MMU, BANCO, NCTX = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
def env(n):
    e = dict(os.environ); e["MMU_RAIZ"] = os.path.join(BANCO, n); return e
def topo(n):
    p = subprocess.run([MMU,"topo"], capture_output=True, env=env(n))
    return int(p.stdout.strip() or 0)
def le(n, a):
    p = subprocess.run([MMU,"le",str(a)], capture_output=True, env=env(n))
    return p.stdout.decode("utf-8","replace") if p.returncode == 0 else ""
def poe(n, linha):
    subprocess.run([MMU,"poe"], input=linha.encode()[:1024], capture_output=True, env=env(n))
def fala(p, n=220):
    d=json.dumps({"model":N2,"prompt":p,"stream":False,
        "options":{"temperature":0.3,"num_predict":n,"use_mmap":True,"num_ctx":NCTX}}).encode()
    q=urllib.request.Request("http://localhost:11434/api/generate",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=600).read())["response"].strip().replace("\n"," ")

feitos = topo("n2")
inicio = feitos*4 + 1
lote = [le("n1", a) for a in range(inicio, inicio+4)]
lote = [l for l in lote if l and len(l.split("\t")) >= 4]
if len(lote) >= 3:
    partes = [l.split("\t") for l in lote]
    corpo = "\n\n".join(f"[{p[0]}] {p[3][:300]}" for p in partes)
    # O PEDIDO MUDOU, e por uma medida e não por gosto. Em 01/08 sete das nove sínteses
    # começaram com a MESMA frase — "Todas as revisões apontam para a necessidade de maior
    # profundidade/detalhamento/especificidade". Perguntar "o que têm em comum" a um modelo
    # de 2 B convida-o ao molde: a resposta genérica serve sempre. Perguntar o que as
    # SEPARA obriga-o a olhar para o lote que tem à frente, porque a diferença não se pode
    # responder de cor.
    s = fala(f"Quatro revisões independentes, sobre conceitos diferentes:\n\n{corpo}\n\n"
             f"1. Que ERRO CONCRETO cada revisão apontou? Cite o conceito pelo nome.\n"
             f"2. Em que é que estas quatro DIFEREM entre si?\n"
             f"Nao escreva generalidades sobre 'necessidade de aprofundar'. Tres frases.")
    bs=s.encode(); a=sum(b*(i+1) for i,b in enumerate(bs)) or 1; b=sum(x*x for x in bs) or 1
    h=[]
    while b and len(h)<6:
        q=a//b; a,b=b,a-q*b; h.append(q)
    temas = ",".join(p[0][:18] for p in partes)
    poe("n2", f"{temas}\t{' '.join(map(str,h))}\t{len(s)}\t{s}")
    print(f"n2 sintetizou {len(lote)}: {s[:88]}")
PY
fi

# 3. DIZ — uma linha
viva=$(torre_viva && echo viva || echo MORTA)
echo "vigilia $(date +%H:%M) · n0=$(topo n0) escritos · n1=$(topo n1) revistos · n2=$(topo n2) sínteses · torre $viva"
