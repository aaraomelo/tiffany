#!/bin/bash
# colhe_transfusao.sh — ACORDA O DOADOR e colhe os vetores dele.
#
# O Aarão: "acorda o ollama e faz a transfusão real."
#
# O doador tem de estar ACORDADO — foi a correção dele há dias: "ela precisa estar acordada,
# iteragindo pra funcionar os dois lados da torre e fechar o circuito, aí vira transfusão". Aqui
# isso é literal: pede-se-lhe o embedding de cada frase e ele RESPONDE. Não se lê ficheiro nenhum
# de pesos, e não se copia nada dele.
#
#   ./colhe_transfusao.sh              colhe as frases do projeto
#   ./colhe_transfusao.sh frases.txt   colhe as suas, uma por linha
set -e
MODELO=${MODELO:-nomic-embed-text}
SAIDA=${SAIDA:-dados/colhido/vetores.txt}
curl -s -m 5 localhost:11434/api/tags >/dev/null || {
  echo "  o ollama não responde em localhost:11434 — o doador tem de estar acordado."; exit 2; }

python3 - "$MODELO" "$SAIDA" "${1:-}" <<'PY'
import struct
import json, urllib.request, sys, time
modelo, saida, arq = sys.argv[1], sys.argv[2], sys.argv[3]
# as frases do projeto: cada uma é uma afirmação medida em algum medidor daqui
PADRAO = [
 "o corpo é finito e por isso o ciclo fecha","a cifra é o endereço e não se atribui",
 "ler e escrever são a mesma operação ao contrário","a régua dá o dual, e o dual é forçado",
 "o gato estica e a volta é inteira","o esquilo gira quatro vezes e volta",
 "crescer não é cair","a soma mede e o cruzado ordena",
 "o mínimo é n mais dois","a medula regenera a partir de pouco",
 "quem diz o tamanho à cabeça desce por soma","o trie é o índice",
 "não há contrato: o corpo liquida-se","a paragem sai da álgebra e não do orçamento",
 "o mesmo conteúdo cai sempre no mesmo sítio","guardar é quase grátis, calcular não é",
 "o doador fica acordado e responde","a outra metade não atravessa: deriva-se",
 "o espelho troca só o que ordena","o período de Pisano tem nome desde mil e setecentos",
]
frases = [l.strip() for l in open(arq) if l.strip()] if arq else PADRAO
def emb(t):
    d=json.dumps({"model":modelo,"prompt":t}).encode()
    r=urllib.request.Request("http://localhost:11434/api/embeddings",d,
                             {"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(r,timeout=120).read())["embedding"]
t0=time.time()
with open(saida,"w") as f:
    for i,fr in enumerate(frases):
        v=emb(fr)
        f.write(" ".join("0x%08X"%struct.unpack("<I",struct.pack("<f",x))[0] for x in v)+"\n")
        print(f"  {i+1:3}/{len(frases)}  {len(v)} dim  \"{fr[:46]}\"",flush=True)
print(f"  colhidos {len(frases)} vetores em {time.time()-t0:.1f}s -> {saida}")
print(f"  agora:  ./transfusao_real  (ou  ../tools/painel.sh transfusao-real)")
PY
