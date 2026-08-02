#!/bin/bash
# oraculo.sh — o forward do disco contra o ollama, no mesmo prompt e sem batota.
#
# O `gguf.c` disse que a dequantização Q4_K/Q6_K estava PLAUSÍVEL — pesos centrados em zero,
# desvio na escala certa — e disse também que isso não é o mesmo que estar certa. Este script é
# o oráculo que faltava: pergunta-se o mesmo ao ollama e ao nosso forward, e os textos ou são
# iguais ou não são.
#
# A ARMADILHA, que vale a pena estar escrita porque me apanhou: o ollama aplica
# `repeat_penalty: 1.1` POR OMISSÃO, mesmo com temperatura 0. Com "The capital of France is",
# ele dizia " Paris. The country" e o nosso forward " Paris. The capital" — e a diferença não
# era nossa: "capital" está no prompt, portanto era penalizado. Comparar contra um oráculo sem
# saber o que ele está a fazer mede a configuração dele, não o nosso trabalho.
#
#   raw:true          sem template de conversa por cima do prompt
#   temperature:0     sem amostragem
#   top_k:1           argmax puro, como o nosso
#   repeat_penalty:1  sem penalização — é o que nos torna comparáveis
set -u
CD="$(cd "$(dirname "$0")" && pwd)"
MODELO=${MODELO:-qwen2.5:1.5b}
PROMPT=${1:-"The capital of France is"}
N=${2:-4}
BIN=${BIN:-$CD/forward}

[ -x "$BIN" ] || { echo "oraculo: falta o binário — cc -O2 -std=c99 -I$CD $CD/forward.c -lm -o $BIN" >&2; exit 1; }

echo "prompt: \"$PROMPT\"   ($N tokens)"
echo

esperado=$(python3 - "$MODELO" "$PROMPT" "$N" <<'PY'
import json, urllib.request, sys
mod, pr, n = sys.argv[1], sys.argv[2], int(sys.argv[3])
d = json.dumps({"model":mod,"prompt":pr,"raw":True,"stream":False,
    "options":{"temperature":0,"num_predict":n,"top_k":1,"repeat_penalty":1.0,
               "use_mmap":True}}).encode()
q = urllib.request.Request("http://localhost:11434/api/generate", d,
                           {"Content-Type":"application/json"})
print(json.loads(urllib.request.urlopen(q, timeout=600).read())["response"], end="")
PY
)

ORACULO="$esperado" "$BIN" "$PROMPT" "$N"
