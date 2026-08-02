#!/bin/bash
# solto.sh — SEM CRITÉRIO. Ele lê o que quiser, escreve no hash dele, e nós observamos.
#
# O Aarão: "tira o critério, deixa correr solto, põe os logs aqui — e só deixa ele ler o que
# quiser e escrever no hash dele. Depois podemos revisar."
#
# Não há fase de validação. Não há passa/pula. Ele diz, lê a base se lhe apetecer, e o que
# escreve fica no endereço que o próprio conteúdo determina — a cifra. Nós só olhamos.
set -e
GER=${GER:-qwen2.5:1.5b}
BASE=${BASE:-/tmp/solto.tsv}
LOG=${LOG:-/tmp/solto.log}
N=${N:-14}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
: > "$LOG"; [ -f "$BASE" ] || : > "$BASE"
python3 - "$GER" "$BASE" "$LOG" "$N" <<'PY'
import json, urllib.request, sys, time
GER, BASE, LOG, N = sys.argv[1], sys.argv[2], sys.argv[3], int(sys.argv[4])
def api(r,c):
    d=json.dumps(c).encode()
    q=urllib.request.Request(f"http://localhost:11434/api/{r}",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(q,timeout=300).read())
def fala(p,n=110,t=0.4,cru=False):
    r=api("generate",{"model":GER,"prompt":p,"stream":False,
        "options":{"temperature":t,"seed":7,"num_predict":n}})["response"].strip()
    return r if cru else r.replace("\n"," ")
def hash_de(txt, m=6):
    bs=txt.encode("utf-8")
    a=sum(b*(i+1) for i,b in enumerate(bs)) or 1
    b=sum(x*x for x in bs) or 1
    out=[]
    while b and len(out)<m:
        q=a//b; a,b=b,a-q*b; out.append(q)
    return " ".join(map(str,out))
L=open(LOG,"a")
def log(m):
    print("  "+m[:118],flush=True); L.write(m+"\n"); L.flush()

t0=time.time()
log(f"=== SOLTO — {GER}, sem critério, {N} voltas ===")
bruto=fala(f"Liste {N} conceitos que você conhece bem, de qualquer área. Um por linha, só os nomes.",
           n=150,t=0.5,cru=True)
temas=[l.strip(" -•.0123456789\t") for l in bruto.split("\n") if 3<len(l.strip())<46][:N]
log(f"ELE ESCOLHEU {len(temas)}: {', '.join(temas)}")
log("")
for i,tema in enumerate(temas,1):
    # LER: ele vê a base e decide sozinho se quer usar
    ja=[l.split("\t")[0] for l in open(BASE) if l.strip()]
    ctx=("\n".join(f"- {t}" for t in ja[-6:]) if ja else "(a base está vazia)")
    leu=fala(f"A base já tem estes registos:\n{ctx}\n\nVocê vai escrever sobre: {tema}\n"
             f"Quer usar algum registo da base? Responda só SIM ou NAO.", n=6)
    usa = leu.upper().startswith("S")
    log(f"[{i:2}] {tema}")
    log(f"     LEU a base ({len(ja)} registos) e quer usar? {leu[:12]}")
    p=f"Escreva o que você sabe sobre {tema}. Duas frases, sem preâmbulo."
    if usa and ja: p=f"Contexto da base:\n{ctx}\n\n"+p
    txt=fala(p, n=130)
    h=hash_de(txt)
    with open(BASE,"a") as fb: fb.write(f"{tema}\t{h}\t{len(txt)}\t{txt}\n")
    log(f"     ESCREVEU {len(txt)}B no hash [{h}]")
    log(f"     > {txt[:104]}")
    log("")
log(f"=== {len(temas)} registos, base com {sum(1 for _ in open(BASE))} linhas, {time.time()-t0:.1f}s ===")
L.close()
PY
