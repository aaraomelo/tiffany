#!/bin/bash
# veste.sh — VESTIR O OLLAMA COM A TÚNICA: ele emite ISA, o banco executa, e o ciclo fecha.
#
# O Aarão: "o plugue deve ser inversível, claro — ler e escrever é a mesma operação dual. Veste o
# ollama com a túnica e faz ele controlar o banco."
#
# A TÚNICA é o par adjunto (ler, escrever), e ela já está medida: colheita.c §C2 mede que ler e
# escrever são adjuntos com resíduo zero. Vestir o modelo com ela é dar-lhe os DOIS lados:
#
#   ler      o banco -> o estado -> o prompt        (a torre branca desce)
#   escrever a resposta -> a operação -> o banco    (a torre negra sobe)
#
# E o banco é FINITO — Z_q — logo o ciclo TEM de fechar, por gaiola. É o transplante.c §T7 outra
# vez, agora com o modelo a controlar em vez de só responder.
set -e
Q=${Q:-16}                      # o corpo: Z_16, e o ciclo fecha em <= 16
BANCO=/tmp/banco_llm.txt
MODELO=${1:-llama3.2:1b}
: > "$BANCO"
for i in $(seq 0 $((Q-1))); do echo "0" >> "$BANCO"; done   # os slots, todos a zero

echo "  a tunica: Z_$Q, $(wc -l < $BANCO) slots, modelo $MODELO"
echo "  ler = LOAD (o banco -> o prompt) · escrever = STORE (a resposta -> o banco)"
echo

python3 - "$MODELO" "$Q" "$BANCO" <<'PY'
import json, urllib.request, sys, hashlib
modelo, Q, banco = sys.argv[1], int(sys.argv[2]), sys.argv[3]

def LOAD(slot):                                  # ler: o banco -> o valor
    return int(open(banco).read().split('\n')[slot] or 0)
def STORE(slot, v):                              # escrever: o valor -> o banco
    L = open(banco).read().split('\n')
    L[slot] = str(v % Q)
    open(banco,'w').write('\n'.join(L))

def fala(p, n=24):
    d = json.dumps({"model":modelo,"prompt":p,"stream":False,
        "options":{"temperature":0,"seed":7,"num_predict":n}}).encode()
    r = urllib.request.Request("http://localhost:11434/api/generate", d,
                               {"Content-Type":"application/json"})
    with urllib.request.urlopen(r, timeout=240) as f: return json.load(f)["response"].strip()

# O CICLO: o estado do banco vira prompt; a resposta vira STORE; e repete.
estado, vistos, t = 3, {}, 0
print("  %4s %8s %-46s %8s" % ("t","LOAD","a resposta (a torre branca)","STORE"))
while t < Q + 4:
    if estado in vistos:
        print("\n  FECHOU: periodo %d, em %d passos, no estado %d (de %d possiveis)"
              % (t - vistos[estado], t, estado, Q))
        json.dump({"periodo": t-vistos[estado], "passos": t, "Q": Q},
                  open('/tmp/veste_ciclo.json','w'))
        break
    vistos[estado] = t
    v = LOAD(estado)                                          # LER  — a torre branca
    r = fala("responda com uma palavra sobre o numero %d" % (estado + v))
    novo = int(hashlib.sha256(r.encode()).hexdigest(), 16) % Q
    STORE(estado, novo)                                       # ESCREVER — a torre negra
    print("  %4d %8d %-46s %8d" % (t, v, r[:44].replace('\n',' '), novo))
    estado = novo
    t += 1
else:
    print("\n  NAO fechou em %d — impossivel em Z_%d, havia bug" % (Q+4, Q))
PY
echo
echo "  o banco no fim: $(tr '\n' ' ' < $BANCO)"
