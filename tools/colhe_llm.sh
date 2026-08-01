#!/bin/bash
# colhe_llm.sh — A COLHEITA: tira do doador a sequência que o transplante.c vai medir.
#
# O doador é uma LLM local via ollama, com temperatura 0 e semente fixa — determinista, porque
# um doador que muda a cada colheita não se mede. Nada é feito à saída: os bytes dela SÃO a
# sequência, e transformá-los seria colher outra coisa.
#
#   ./colhe_llm.sh [modelo]        (por omissão llama3.2:1b)
set -e
MODELO="${1:-llama3.2:1b}"
SAIDA=/tmp/llm_medula.txt
: > "$SAIDA"
for P in "explique o que e uma matriz e para que serve" \
         "a sequencia de fibonacci ate duzentos" \
         "descreva o funcionamento de um transistor" \
         "conte uma historia curta sobre um relojoeiro"; do
  curl -s http://localhost:11434/api/generate \
    -d "{\"model\":\"$MODELO\",\"prompt\":\"$P\",\"stream\":false,\"options\":{\"temperature\":0,\"seed\":7,\"num_predict\":220}}" \
  | python3 -c 'import sys,json; sys.stdout.write(json.load(sys.stdin)["response"])' >> "$SAIDA"
done
echo "colhidos $(wc -c < "$SAIDA") bytes de $MODELO em $SAIDA"
