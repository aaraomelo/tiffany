#!/bin/bash
# sincroniza.sh — traz a memória viva para o repositório, ou devolve-a.
#
# A memória da assistente vive em ~/.claude/projects/<projeto>/memory/ e NÃO é um repositório:
# ela persiste entre sessões no disco do Aarão e desaparece se a máquina desaparecer. Isto
# versiona-a — e a direção importa, por isso é explícita e nunca se adivinha.
#
#   ./sincroniza.sh guarda    a memória viva -> o repositório   (antes de commitar)
#   ./sincroniza.sh restaura  o repositório -> a memória viva    (numa máquina nova)
set -e
VIVA="$HOME/.claude/projects/-home-aaraolopes-Documentos-tiffany/memory"
AQUI="$(cd "$(dirname "$0")" && pwd)"

case "${1:-}" in
  guarda)
    [ -d "$VIVA" ] || { echo "não há memória viva em $VIVA"; exit 1; }
    cp "$VIVA"/*.md "$AQUI"/
    echo "guardados $(ls "$AQUI"/*.md | wc -l) ficheiros no repositório"
    ;;
  restaura)
    mkdir -p "$VIVA"
    cp "$AQUI"/*.md "$VIVA"/
    echo "restaurados $(ls "$VIVA"/*.md | wc -l) ficheiros na memória viva"
    ;;
  *)
    echo "uso: $0 guarda | restaura"
    exit 1
    ;;
esac
