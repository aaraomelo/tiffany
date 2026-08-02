#!/bin/bash
# interroga.sh — PERGUNTA TUDO O QUE ELE SABE, e guarda o par de cada resposta.
#
# O Aarão: "pergunta tudo que ele sabe e roda o contrato."
#
# O doador tem de estar ACORDADO — e aqui isso é literal duas vezes: ele RESPONDE em texto (o
# modelo generativo) e o que ele responde é medido no espaço vetorial (o de embeddings). Não se
# lê ficheiro de pesos, não se copia nada: pergunta-se, e ele diz.
#
# E a saída é DUAS COORDENADAS por resposta — o par — porque é sobre o par que o contrato roda.
set -e
GERADOR=${GERADOR:-qwen2.5:1.5b}
EMB=${EMB:-nomic-embed-text}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
echo "  o doador: $GERADOR  ·  o espaço: $EMB"
python3 - "$GERADOR" "$EMB" <<'PY'
import json, urllib.request, sys, time
GER, EMB = sys.argv[1], sys.argv[2]
def api(rota, corpo):
    d = json.dumps(corpo).encode()
    r = urllib.request.Request(f"http://localhost:11434/api/{rota}", d, {"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(r, timeout=300).read())
def fala(p):
    return api("generate", {"model":GER,"prompt":p,"stream":False,
                            "options":{"temperature":0,"seed":7,"num_predict":60}})["response"].strip()
def emb(t):
    return api("embeddings", {"model":EMB,"prompt":t})["embedding"]

# PERGUNTA TUDO O QUE ELE SABE — sobre os assuntos do projeto, um a um.
PERGUNTAS = [
 "O que é uma fração contínua? Responde em uma frase.",
 "O que é a razão áurea? Uma frase.",
 "O que a transformada de Fourier faz com uma convolução? Uma frase.",
 "O que é o determinante de uma matriz 2 por 2? Uma frase.",
 "O que faz a sequência de Fibonacci crescer? Uma frase.",
 "O que é um corpo em álgebra? Uma frase.",
 "O que é uma involução? Uma frase.",
 "O que é o período de Pisano? Uma frase.",
 "O que é um trie? Uma frase.",
 "O que é entropia em termodinâmica? Uma frase.",
 "O que é um smart contract? Uma frase.",
 "O que é o efeito Seebeck? Uma frase.",
]
t0 = time.time()
with open("/tmp/saber.txt","w") as ft, open("/tmp/saber_pares.txt","w") as fp:
    for i,q in enumerate(PERGUNTAS):
        r = fala(q)
        v = emb(r)
        # AS DUAS COORDENADAS: a soma das pares e a das ímpares — o direto e o cruzado
        a = sum(v[0::2]); b = sum(v[1::2])
        ft.write(q + "\t" + r.replace("\n"," ") + "\n")
        fp.write(f"{round(a*10000)} {round(b*10000)}\n")
        print(f"  {i+1:2}/{len(PERGUNTAS)}  ({round(a*10000):7},{round(b*10000):7})  {r[:56]}", flush=True)
print(f"  {len(PERGUNTAS)} respostas em {time.time()-t0:.1f}s -> /tmp/saber.txt e /tmp/saber_pares.txt")
PY
