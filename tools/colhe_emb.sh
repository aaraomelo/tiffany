#!/bin/bash
# colhe_emb.sh — EMBEDDINGS PUROS: os vetores, sem texto pelo meio.
#
# O doador é o nomic-embed-text local (768 dim). Os modelos generativos do ollama não expõem
# embeddings nesta versão — pede um modelo de embedding dedicado, e este é o mais leve que serve.
#
#   ./colhe_emb.sh    ->  dados/colhido/emb.txt, uma linha por termo: nome<TAB>v1 v2 v3 …
set -e
SAIDA=dados/colhido/emb.txt
: > "$SAIDA"
for T in rei rainha homem mulher torre castelo ouro prata zero um \
         cifra numero corpo dual simetria antissimetria; do
  V=$(curl -s http://localhost:11434/api/embeddings \
        -d "{\"model\":\"nomic-embed-text\",\"prompt\":\"$T\"}" \
      | python3 -c 'import sys,json; print(" ".join("%.9g"%x for x in json.load(sys.stdin)["embedding"]))')
  printf '%s\t%s\n' "$T" "$V" >> "$SAIDA"
done
echo "colhidos $(wc -l < "$SAIDA") vetores em $SAIDA"
