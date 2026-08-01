#!/bin/bash
# cicla_llm.sh — A TRANSFUSÃO: a LLM ACORDADA no circuito, com o corpo a fechá-lo.
#
# O Aarão: "se enxertou metade é porque não fez o procedimento todo. Ela precisa estar acordada,
# interagindo, pra funcionar os dois lados da torre e fechar o circuito. Aí vira transfusão."
#
# Sozinha, realimentada, a LLM DERIVA — não fecha. Com o corpo no meio ela fecha, e o corpo é
# quem fecha, porque é FINITO: em Z_q o circuito tem de repetir em <= q passos, por gaiola.
#
#   ./cicla_llm.sh sozinha    a LLM realimentada (a torre branca só)
#   ./cicla_llm.sh transfusao a LLM + o corpo (as duas torres)
set -e
MODO="${1:-transfusao}"
python3 - "$MODO" <<'PY'
import json, urllib.request, hashlib, sys
def fala(p, n=50):
    d = json.dumps({"model":"llama3.2:1b","prompt":p,"stream":False,
        "options":{"temperature":0,"seed":7,"num_predict":n}}).encode()
    r = urllib.request.Request("http://localhost:11434/api/generate", d, {"Content-Type":"application/json"})
    with urllib.request.urlopen(r, timeout=240) as f: return json.load(f)["response"].strip()
Q = 23
if sys.argv[1] == "sozinha":
    cur, vistos = "descreva uma torre", {}
    for t in range(14):
        h = hashlib.sha256(cur.encode()).hexdigest()[:12]
        if h in vistos: print("FECHOU periodo", t - vistos[h]); break
        vistos[h] = t; print("t=%2d %s" % (t, cur[:60].replace("\n"," "))); cur = fala(cur)
    else: print("NAO FECHOU em 14 — a torre branca sozinha deriva")
else:
    est, vistos, t = 7, {}, 0
    while t < 30:
        if est in vistos:
            print("FECHOU periodo %d, em %d passos, ponto fixo %d" % (t-vistos[est], t, est)); break
        vistos[est] = t
        est = int(hashlib.sha256(fala("responda em uma frase curta sobre o numero %d" % est).encode()).hexdigest(),16) % Q
        t += 1
PY
