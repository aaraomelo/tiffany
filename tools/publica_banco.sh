#!/bin/bash
# publica_banco.sh — leva /banco/ + /src/ + manifesto ao goldenkingdom (estático).
# Não é o pipeline Vite/TeX. Não toca em /wasm/ (já vem do SPA).
# scp ficheiro a ficheiro: tar Windows→Linux deixa o src a meio.
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
KEY="${PATRIA_KEY:-$ROOT/segredo/id_ed25519_patria}"
DEST="${DEST:-/var/www/goldenkingdom}"
H="${SSH_USER:-root}@${SSH_HOST:-srv1559444.hstgr.cloud}"
[ -f "$KEY" ] || { echo "falta $KEY"; exit 1; }
SSHOPT=(-i "$KEY" -o StrictHostKeyChecking=accept-new -o ConnectTimeout=25)

ssh "${SSHOPT[@]}" "$H" "mkdir -p '$DEST/src' '$DEST/banco' '$DEST/conecthus/backends'"

for f in "$ROOT/app/src/"*; do
  [ -f "$f" ] || continue
  scp "${SSHOPT[@]}" "$f" "$H:$DEST/src/$(basename "$f")"
done
for f in "$ROOT/app/banco/"*; do
  [ -f "$f" ] || continue
  scp "${SSHOPT[@]}" "$f" "$H:$DEST/banco/$(basename "$f")"
done
scp "${SSHOPT[@]}" "$ROOT/conecthus/backends/manifesto.json" "$H:$DEST/conecthus/backends/manifesto.json"

echo "--- /banco/ ---"
ssh "${SSHOPT[@]}" "$H" "curl -sf -o /dev/null -w 'banco %{http_code}\n' https://goldenkingdom.patriatechnology.com/banco/; curl -sf -o /dev/null -w 'sec %{http_code} %{size_download}\n' https://goldenkingdom.patriatechnology.com/src/wasm_sec_browser.js"
