#!/bin/sh
# corpo.sh — O QUE O FRONT PRECISA, MEDIDO E NÃO ADIVINHADO.
#
# O Aarão: «põe os arquivos que vc precisa no front, pode ser?»
#
# Pode — mas quais são «os que preciso» não é opinião minha: é o que o tradutor ABRE. Então
# pergunta-se ao próprio libc. Um `fopen` interceptado diz, ficheiro a ficheiro, o que ele foi
# buscar para compor cada documento, e a lista sai daí. Uma lista escrita à mão envelhece em
# silêncio — muda-se um `\fontsize` no estilo e entra um corpo novo que ninguém pôs no
# manifesto; o pedido cai, e cai com um 404 que não explica nada.
#
# E NÃO SE COPIA NADA. O `vite.config.js` já o diz sobre o PDF — «não se move nenhum ficheiro
# de lugar, e não fica cópia nenhuma no dist» — e vale igual aqui: o manifesto guarda CAMINHOS,
# e o front serve o ficheiro de onde ele está. Copiar seria gravar, e gravar envelhece.
#
# E O MANIFESTO É TAMBÉM O PORTÃO. O repositório é público mas nem tudo nele é para servir: o
# `curriculo/` tem CPF e conta bancária, o `broca-so/cristalchain` diz «IP privado — não
# publicar». Uma rota estática sobre a raiz serviria isso tudo. Aqui só sai o que está na
# lista, e a lista é o que o tradutor provou precisar.
#
#   ./tools/corpo.sh          mede e escreve app/src/corpo.json
#
cd "$(dirname "$0")/.." || exit 1
RAIZ=$(pwd)
ESPIA=/tmp/corpo_espia
mkdir -p "$ESPIA"

# ── o espião: o `fopen` diz o que abre, e só o que abre para LER ──────────────────────
cat > "$ESPIA/espia.c" <<'FIM'
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
static FILE *(*real)(const char*, const char*);
FILE *fopen(const char *c, const char *m){
    if(!real) real = dlsym(RTLD_NEXT, "fopen");
    if(m[0] == 'r') fprintf(stderr, "ABRE %s\n", c);
    return real(c, m);
}
FIM
cc -O2 -fPIC -shared "$ESPIA/espia.c" -o "$ESPIA/espia.so" -ldl || {
  echo "corpo.sh: o espião não construiu — e sem ele a lista seria adivinhada."; exit 1; }

# o tradutor tem de estar de pé: é ele quem responde à pergunta
[ -x tests/tex ] || (cd tests && cc -O2 -std=c99 -I../lib tex.c -lm -o tex) || {
  echo "corpo.sh: o tradutor tests/tex não construiu."; exit 1; }

DOCS="teoria.tex catalogo.tex enredo.tex papers/corpo-estelar.tex papers/dualsort.tex"

: > "$ESPIA/tudo.txt"
for d in $DOCS; do
  [ -f "$d" ] || { echo "  falta $d"; continue; }
  printf '  a compor %-28s ' "$d"
  ( cd tests && LD_PRELOAD="$ESPIA/espia.so" ./tex "$RAIZ/$d" "$ESPIA/saida.pdf" ) \
      2>>"$ESPIA/tudo.txt" >/dev/null
  echo "ok"
done

# ── os caminhos, resolvidos e sem repetir. O tradutor tenta duas raízes; conta a que existe ──
LISTA="$ESPIA/lista.txt"
grep '^ABRE ' "$ESPIA/tudo.txt" | sed 's/^ABRE //' | sed 's#^\.\./##' | sed "s#^$RAIZ/##" \
  | sort -u | while read -r f; do [ -f "$RAIZ/$f" ] && echo "$f"; done | sort -u > "$LISTA"

N=$(wc -l < "$LISTA")
BYTES=$(while read -r f; do stat -c%s "$RAIZ/$f"; done < "$LISTA" | paste -sd+ | bc)

# ── o manifesto ──────────────────────────────────────────────────────────────────────
SAIDA=app/src/corpo.json
{
  echo '{'
  echo '  "medido_por": "tools/corpo.sh — fopen interceptado, não lista à mão",'
  echo '  "regra": "o front serve estes de onde eles estão; nada é copiado para o dist",'
  printf '  "ficheiros": [\n'
  i=0
  while read -r f; do
    i=$((i+1))
    [ "$i" -eq "$N" ] && v='' || v=','
    printf '    "%s"%s\n' "$f" "$v"
  done < "$LISTA"
  printf '  ]\n'
  echo '}'
} > "$SAIDA"

echo
echo "  $N ficheiros, $BYTES bytes  ->  $SAIDA"
echo "  (medido a compor: $DOCS)"
