#!/bin/bash
# veste_doador.sh — A TÚNICA NO DOADOR: ele LÊ o que disse, VERIFICA, e ESCREVE a correção.
#
# O Aarão: "põe a túnica nele e vê se ele consegue verificar as afirmações e corrigir, entregar
# pronta."
#
# A TÚNICA É O PAR ADJUNTO (ler, escrever), e vestir alguém com ela é dar-lhe os DOIS lados. Até
# aqui ele só escreveu — respondeu e nós medimos. Vestido, ele passa a LER o que escreveu e a
# escrever POR CIMA. O ciclo fecha nele, e não em nós.
#
#   ler       a afirmação anterior  →  o prompt      (a torre branca desce)
#   verificar o critério            →  a decisão
#   escrever  a correção            →  a afirmação   (a torre negra sobe)
#
# E A SUPERVISÃO É O CRITÉRIO, que é nosso: dizemos-lhe o DOMÍNIO da pergunta (matemática e
# computação, não biologia) e pedimos-lhe que corrija só se estiver errada. Sem isso ele não tem
# como saber que "involução" era a algébrica — e a supervisão é exatamente isso: o lado que ele
# não pode fornecer a si próprio.
set -e
GERADOR=${GERADOR:-qwen2.5:1.5b}
curl -s -m 5 localhost:11434/api/tags >/dev/null || { echo "  o doador não responde."; exit 2; }
[ -s /tmp/saber.txt ] || { echo "  sem /tmp/saber.txt — corra ./interroga.sh"; exit 2; }
echo "  a vestir $GERADOR com a túnica: ele lê o que disse, verifica, e escreve por cima"
python3 - "$GERADOR" <<'PY'
import json, urllib.request, sys, time
GER = sys.argv[1]
def fala(p, n=110):
    d = json.dumps({"model":GER,"prompt":p,"stream":False,
                    "options":{"temperature":0,"seed":7,"num_predict":n}}).encode()
    r = urllib.request.Request("http://localhost:11434/api/generate", d, {"Content-Type":"application/json"})
    return json.loads(urllib.request.urlopen(r, timeout=300).read())["response"].strip()

pares = [l.rstrip("\n").split("\t") for l in open("/tmp/saber.txt") if "\t" in l]
t0 = time.time()
with open("/tmp/corrigido.txt","w") as fo:
    for i,(q,a) in enumerate(pares):
        # A TÚNICA: ele LÊ a sua afirmação, e o critério é a supervisão que nós pomos.
        p = (f"Contexto: MATEMÁTICA e CIÊNCIA DA COMPUTAÇÃO (não biologia, não medicina).\n"
             f"Pergunta: {q}\n"
             f"Resposta dada anteriormente: {a}\n\n"
             f"Verifique se essa resposta está correta NESTE contexto. "
             f"Se estiver correta, escreva exatamente: CORRETA\n"
             f"Se estiver errada, escreva: ERRADA seguido da correção em uma frase.\n"
             f"Responda:")
        v = fala(p).replace("\n"," ").strip()
        fo.write(q + "\t" + a + "\t" + v + "\n")
        marca = "CORRETA" if v.upper().startswith("CORRETA") else "ERRADA "
        print(f"  {i+1:2}/{len(pares)}  [{marca}] {v[:66]}", flush=True)
print(f"  {len(pares)} verificações em {time.time()-t0:.1f}s -> /tmp/corrigido.txt")
PY
