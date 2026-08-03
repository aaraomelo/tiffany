#!/bin/bash
# colhe_dualcifra.sh — colhe do doador ACORDADO as DUAS metades: as frases e as palavras delas.
#
# O §W1 precisa das duas: o embedding da frase (o que ele entrega) e os das palavras (a soma que
# lá está dentro). Sem as duas não se mede que "ele entra somando" — afirma-se.
set -e
curl -s -m 5 localhost:11434/api/tags >/dev/null || {
  echo "  o ollama não responde — o doador tem de estar acordado."; exit 2; }
python3 - <<'PY'
import json, urllib.request
def emb(t):
    d=json.dumps({"model":"nomic-embed-text","prompt":t}).encode()
    r=urllib.request.Request("http://localhost:11434/api/embeddings",d,{"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(r,timeout=120).read())["embedding"]
FR = ["o corpo é finito","a cifra é o endereço","ler e escrever","o gato estica",
      "crescer não é cair","a soma mede","o trie é o índice","guardar é grátis"]
with open("/tmp/frases.txt","w") as ff, open("/tmp/palavras.txt","w") as fp, open("/tmp/mapa.txt","w") as fm:
    for fr in FR:
        ff.write(" ".join("%.17g"%x for x in emb(fr))+"\n")
        ws = fr.split(); fm.write(str(len(ws))+"\n")
        for w in ws: fp.write(" ".join("%.17g"%x for x in emb(w))+"\n")
print(f"  {len(FR)} frases e {sum(len(f.split()) for f in FR)} palavras -> /tmp/frases.txt, /tmp/palavras.txt")
PY
