#!/bin/bash
# traz_grok.sh — o grok-1 para dentro da fita: 9 partes, 65,3 GB, retomável.
#
# O Aarão: "traz o grok [...] baixa da net, tem disponível."
#
# O grok-1 da xAI não está na biblioteca do ollama (404 em grok, grok-1, grok1, xai/grok, grok-2),
# mas está no HuggingFace em GGUF, partido em nove. A quantização mais pequena é a IQ1_S: 65,3 GB,
# contra os 116 GB do Q2_K e os ~300 GB do original em bf16.
#
# E para o que a fita faz — transcrever, cifrar, sair byte a byte — a quantização não importa: o
# protocolo não pergunta o que os bytes significam. Importaria se fosse para CORRER, e aí o IQ1_S
# é grosseiro demais para valer.
#
# RETOMÁVEL de propósito: são ~1h38m a 11 MB/s (banda medida), e uma queda a meio não pode obrigar
# a recomeçar. As partes já completas saltam-se pelo tamanho; o resto retoma onde parou.
#
# E DOIS DEFEITOS MEUS NA PRIMEIRA VERSÃO, que deram nove ficheiros de 15 bytes — o corpo de uma
# resposta 404 — sem que nada gritasse:
#
#   1. o nome levava UM ZERO A MENOS. `seq -w 1 9` escolhe a largura pelo maior (um dígito), e
#      "000$i" dava 0001 quando o real é 00001. O printf com %05d não depende dessa escolha.
#   2. `tr -d 'content-lengh: \r'` — o "t-l" é lido como INTERVALO e está invertido, e o tr
#      recusava a linha toda. `tr -dc '0-9'` diz o que se QUER (só dígitos) em vez de listar o
#      que se não quer, e não há intervalo nenhum a interpretar.
#
# E o que os escondeu foi eu não ter verificado o TAMANHO do que chegou: 15 bytes passaram por
# download nove vezes seguidas. Agora recusa-se qualquer parte abaixo de 1 GB, que é a ordem de
# grandeza certa — um ficheiro pequeno demais é um erro disfarçado de sucesso.
set -u
D=${D:-.torre/grok}
BASE=https://huggingface.co/Arki05/Grok-1-GGUF/resolve/main/IQ1_S
MIN=${MIN:-1000000000}          # nenhuma parte legítima é menor que 1 GB
mkdir -p "$D"
total=0
for i in $(seq 1 9); do
  F="grok-1-IQ1_S-$(printf '%05d' "$i")-of-00009.gguf"
  esperado=$(curl -sIL "$BASE/$F" 2>/dev/null | grep -i '^content-length' | tail -1 | tr -dc '0-9')
  atual=$(stat -c %s "$D/$F" 2>/dev/null || echo 0)
  if [ -n "$esperado" ] && [ "$atual" = "$esperado" ]; then
    echo "$F  já completo ($((atual/1000000)) MB)"
    total=$((total+atual))
    continue
  fi
  if [ -z "$esperado" ] || [ "$esperado" -lt "$MIN" ]; then
    echo "$F  RECUSADO: o servidor diz ${esperado:-0} bytes, e nenhuma parte é tão pequena" >&2
    continue
  fi
  echo "$F  a baixar ($((esperado/1000000)) MB)…"
  curl -sL --continue-at - -o "$D/$F" "$BASE/$F"
  fim=$(stat -c %s "$D/$F" 2>/dev/null || echo 0)
  if [ "$fim" = "$esperado" ]; then echo "$F  ok ($((fim/1000000)) MB)"
  else echo "$F  INCOMPLETO: $((fim/1000000)) de $((esperado/1000000)) MB" >&2; fi
  total=$((total+fim))
done
echo "--- total: $((total/1000000000)) GB em $D ---"
