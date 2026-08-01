#!/bin/bash
# memoria_banco.sh — A MEMÓRIA NO BANCO, e a túnica vestida à entrada.
#
# O Aarão: "é possível você guardar suas memórias no banco e vestir a túnica ao entrar?"
#
# Hoje a memória são 32 ficheiros e o MEMORY.md é carregado INTEIRO em toda sessão. Isso funciona
# e vai deixar de funcionar: o índice já teve de ser consolidado uma vez hoje porque estava a
# listar em vez de orientar.
#
# No banco, cada memória entra CIFRADA e o índice é a própria cifra — e aí ler ao entrar deixa de
# ser "carregar tudo" e passa a ser "descer até o que é próximo". É o busca.c, que já existe.
#
#   ./memoria_banco.sh ingere            põe as memórias no banco, cifradas
#   ./memoria_banco.sh perto "assunto"   desce até as mais próximas
set -e
BANCO=/tmp/memoria_banco.txt
MEM="$(cd "$(dirname "$0")/../memoria" && pwd)"

case "${1:-ingere}" in
ingere)
  : > "$BANCO"
  for f in "$MEM"/*.md; do
    [ "$(basename "$f")" = "MEMORY.md" ] && continue
    nome=$(basename "$f" .md)
    # A CIFRA DO CONTEÚDO — e a minha primeira tentativa media o tamanho, que é inútil:
    # a fração contínua de um INTEIRO é ele próprio, e dois textos do mesmo tamanho ficam
    # com a mesma cifra. O que serve é a cifra do CONTEÚDO: a razão entre pesos de letras,
    # que é irracional e é o que o texto.c já usa para distância entre strings.
    n=$(wc -c < "$f")
    cif=$(python3 -c "
import sys
d=open('$f','rb').read()
# dois pesos independentes do conteudo, e a razao entre eles e a coordenada
pa=sum((i+1)*b for i,b in enumerate(d[:4000]))
pb=sum((i+1)*b*b for i,b in enumerate(d[:4000])) or 1
x=pa/pb*1000
c=[]
for _ in range(6):
    f_=int(x); c.append(str(f_)); r=x-f_
    if r<1e-9: break
    x=1/r
print(' '.join(c))")
    # e as palavras-chave, que são o que a descida usa
    chaves=$(grep -oE '\*\*[A-Za-zÀ-ÿ ]{4,28}\*\*' "$f" 2>/dev/null|tr -d '*'|sort -u|head -6|tr '\n' ';')
    printf '%s\t%d\t%s\t%s\n' "$nome" "$n" "$cif" "$chaves" >> "$BANCO"
  done
  echo "  ingeridas $(wc -l < "$BANCO") memórias no banco, cifradas"
  echo "  o banco: $(wc -c < "$BANCO") bytes contra $(du -sb "$MEM"|cut -f1) do original"
  printf '  %-34s %8s  %s\n' "memória" "bytes" "cifra"
  head -4 "$BANCO" | while IFS=$'\t' read -r nome n cif ch; do
    printf '  %-34s %8d  %s\n' "$nome" "$n" "$cif"
  done
  ;;
perto)
  [ -f "$BANCO" ] || { echo "  corre 'ingere' primeiro"; exit 1; }
  alvo="${2:?diga o assunto}"
  echo "  a descer até '$alvo':"
  awk -F'\t' -v alvo="$alvo" '
    { n=0; split(tolower(alvo), termos, " ")
      for(t in termos) if(index(tolower($1 " " $4), termos[t])) n++
      if(n) print n "\t" $1 "\t" $4 }' "$BANCO" \
  | sort -rn | head -3 | while IFS=$'\t' read -r n nome ch; do
    printf '  [%d] %-34s %s\n' "$n" "$nome" "$(echo "$ch"|cut -c1-46)"
  done
  ;;
esac
