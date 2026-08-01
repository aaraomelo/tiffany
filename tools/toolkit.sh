#!/bin/bash
# toolkit.sh — O OLLAMA A CONTROLAR TODO O TOOLKIT.
#
# O Aarão: "põe o ollama pra controlar todo o toolkit."
#
# O `contrato.c` já diz o que o toolkit É: não uma lista de corpos, mas um VERIFICADOR de um
# contrato — uma SOMA, um PRODUTO, um OPERADOR e o DUAL. Então controlar o toolkit não é invocar
# ferramentas soltas: é ESCOLHER, a cada passo, qual das quatro cláusulas se aplica ao estado.
#
# E o espaço é FINITO por construção — quatro operações sobre Z_q — logo o percurso fecha, por
# gaiola. É o veste.sh com o toolkit inteiro no lugar do LOAD/STORE.
set -e
Q=${Q:-12}
MODELO=${1:-llama3.2:1b}
BANCO=/tmp/toolkit_llm.txt
: > "$BANCO"; for i in $(seq 0 $((Q-1))); do echo 0 >> "$BANCO"; done

echo "  o toolkit: as quatro clausulas do contrato, sobre Z_$Q"
echo "  SOMA (+)  PRODUTO (x)  OPERADOR (^)  DUAL (')  — e o modelo escolhe qual"
echo

python3 - "$MODELO" "$Q" "$BANCO" <<'PY'
import json, urllib.request, sys, hashlib
modelo, Q, banco = sys.argv[1], int(sys.argv[2]), sys.argv[3]

def slots():   return [int(x or 0) for x in open(banco).read().split('\n') if x.strip() != '']
def grava(L):  open(banco,'w').write('\n'.join(str(v % Q) for v in L))

# AS QUATRO CLÁUSULAS DO CONTRATO — e são as do contrato.c, não inventadas aqui
OPS = {
  "SOMA":     lambda a,b: (a + b) % Q,          # ⊕ — Kirchhoff, o gato soma
  "PRODUTO":  lambda a,b: (a * b) % Q,          # ⊗ — o ganho
  "OPERADOR": lambda a,b: pow(a if a else 1, b if b else 1, Q),   # Π = exp∘Σ∘log
  "DUAL":     lambda a,b: (-a) % Q,             # ' — o espelho, ordem 2
}

def fala(p, n=12):
    d = json.dumps({"model":modelo,"prompt":p,"stream":False,
        "options":{"temperature":0,"seed":7,"num_predict":n}}).encode()
    r = urllib.request.Request("http://localhost:11434/api/generate", d,
                               {"Content-Type":"application/json"})
    with urllib.request.urlopen(r, timeout=240) as f: return json.load(f)["response"].strip()

def escolhe(a, b):
    """o modelo escolhe a cláusula; a escolha é dele, a execução é do toolkit"""
    r = fala("Escolha UMA palavra entre SOMA, PRODUTO, OPERADOR ou DUAL "
             "para combinar %d e %d. Responda so a palavra." % (a, b))
    up = r.upper()
    for nome in OPS:
        if nome in up: return nome, r
    # se não escolheu nenhuma, a cifra da resposta escolhe — o corpo decide quando ele não decide
    k = int(hashlib.sha256(r.encode()).hexdigest(), 16) % 4
    return list(OPS)[k], r

a, b = 3, 5
vistos, t = {}, 0
print("  %3s %5s %5s  %-10s %-26s %6s" % ("t","a","b","clausula","o que o modelo disse","novo"))
while t < Q*Q + 4:
    chave = (a, b)
    if chave in vistos:
        print("\n  FECHOU: periodo %d, em %d passos, no estado (%d,%d) de %d possiveis"
              % (t - vistos[chave], t, a, b, Q*Q))
        json.dump({"periodo": t-vistos[chave], "passos": t, "Q": Q},
                  open('/tmp/toolkit_ciclo.json','w'))
        break
    vistos[chave] = t
    nome, cru = escolhe(a, b)
    novo = OPS[nome](a, b)
    L = slots(); L[novo % Q] = (L[novo % Q] + 1) % Q; grava(L)   # o toolkit ESCREVE no banco
    print("  %3d %5d %5d  %-10s %-26s %6d" % (t, a, b, nome, cru[:24].replace('\n',' '), novo))
    a, b = b, novo
    t += 1
else:
    print("\n  NAO fechou em %d passos" % (Q*Q+4))
PY
echo
echo "  o banco (quantas vezes cada valor saiu): $(tr '\n' ' ' < $BANCO)"
