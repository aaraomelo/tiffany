#!/bin/bash
# traz_grok.sh — o grok-1 para dentro da fita: 9 partes, 65,3 GB, retomável.
#
# O Aarão: "traz o grok [...] baixa da net, tem disponível."
#
# O grok-1 da xAI não está na biblioteca do ollama (404 em todas as variantes), mas está no
# HuggingFace em GGUF, partido em nove. A quantização mais pequena é a IQ1_S: 65,3 GB, contra os
# 116 GB do Q2_K e os ~300 GB do original em bf16.
#
# E para o que a fita faz — transcrever, cifrar, sair byte a byte — a quantização não importa: o
# protocolo não pergunta o que os bytes significam. Importaria se fosse para CORRER, e aí o IQ1_S
# é grosseiro demais para valer.
#
# RETOMÁVEL de propósito: são ~1h38m a 11 MB/s, e uma queda a meio não pode obrigar a recomeçar.
# O `--continue-at -` do curl retoma onde parou, e as partes já completas saltam-se pelo tamanho.
set -u
D=${D:-.torre/grok}
BASE=https://huggingface.co/Arki05/Grok-1-GGUF/resolve/main/IQ1_S
mkdir -p "$D"
for i in $(seq -w 1 9); do
  F="grok-1-IQ1_S-000$i-of-00009.gguf"
  esperado=$(curl -sIL "$BASE/$F" 2>/dev/null | grep -i '^content-length' | tail -1 | tr -d 'content-lengh: \r')
  atual=$(stat -c %s "$D/$F" 2>/dev/null || echo 0)
  if [ -n "$esperado" ] && [ "$atual" = "$esperado" ]; then
    printf '%s  já completo (%.1f GB)\n' "$F" "$(echo "$atual/1000000000" | bc -l)"
    continue
  fi
  printf '%s  a baixar…\n' "$F"
  curl -sL --continue-at - -o "$D/$F" "$BASE/$F"
  printf '%s  %s\n' "$F" "$(stat -c %s "$D/$F" 2>/dev/null || echo 0) B"
done
echo "--- total ---"
du -sh "$D" 2>/dev/null
