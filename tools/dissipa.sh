#!/bin/sh
# dissipa.sh — O PORTAO DA DISSIPACAO. A RAM E' UM TECTO; ISTO E' UM CAUDAL.
#
# O Aarao: "elimina essa dissipacao de memoria — isso tende a crescer, e muito."
#
# E cresce de outra maneira. A RAM mede-se uma vez e fica: e' um TECTO. A dissipacao
# paga-se em CADA CORRIDA e soma para sempre: e' um CAUDAL. Um vector de 4 MB que nunca e'
# escrito nao dissipa nada; um inteiro reescrito mil milhoes de vezes paga mil milhoes de
# vezes — e o portao da RAM da' vermelho ao primeiro e verde ao segundo.
#
# A REGUA E' LANDAUER: apagar um bit custa kT ln2 (~2,9e-21 J a 300 K, medido em
# laboratorio). O que aqui se conta sao as escritas DESTRUTIVAS com tamanho legivel — as
# que se pode contar sem correr nada. As outras nao se estimam: dizem-se.
#
#   sh tools/dissipa.sh
RAIZ=$(cd "$(dirname "$0")/.." && pwd); cd "$RAIZ" || exit 1
exec python3 - "$RAIZ" <<'PY'
import re, sys, glob
raiz = sys.argv[1]
kT = 1.380649e-23 * 300 * 0.6931471805599453
bits = 0; legiveis = 0; opacos = 0
por_f = {}
for f in sorted(glob.glob(raiz+'/tests/*.c') + glob.glob(raiz+'/banco/*.c')):
    s = open(f, encoding='utf-8', errors='replace').read()
    n_op = len(re.findall(r'disco_zera\(|memset\(', s))
    b = 0
    for m in re.finditer(r'disco_zera\([^,]+,\s*\(size_t\)\(*\(*([0-9]+)', s):
        b += int(m.group(1)) * 8; legiveis += 1
    for m in re.finditer(r'memset\([^,]+,\s*0\s*,\s*([0-9]+)\s*\)', s):
        b += int(m.group(1)) * 8; legiveis += 1
    opacos += n_op
    if b: por_f[f[len(raiz)+1:]] = b
    bits += b
print("  ─── O PORTAO DA DISSIPACAO ───────────────────────────────────────────")
print("  %d escritas destrutivas, %d com tamanho legivel" % (opacos, legiveis))
print("  bits apagados por corrida (os legiveis): %d" % bits)
print("  em joules por corrida: %.3e J   (Landauer, kT ln2 a 300 K)" % (bits*kT))
print("  e a 1000 corridas por dia, um ano:      %.3e J" % (bits*kT*1000*365))
print("  ── os que mais apagam:")
for f, b in sorted(por_f.items(), key=lambda t: -t[1])[:6]:
    print("     %10d bits  %s" % (b, f))
print("  ── e %d escritas cujo tamanho NAO se le' daqui: nao se estimam, dizem-se." %
      (opacos - legiveis))
PY
