#!/bin/sh
# indices.sh — DOIS OBJECTOS NO MESMO ENDERECO E' UM SEGFAULT SILENCIOSO.
#
# O endereco do disco e' DISCO_LAJE + i*DISCO_PASSO, e o `i` escolhe-se a mao. Escolher
# duas vezes o mesmo `i` DENTRO DO MESMO FICHEIRO poe dois objectos na mesma pagina — e o
# programa nao acusa nada: mapeia, escreve por cima, e rebenta longe dali. Foi o que
# aconteceu no dualcifra.c, onde o 200 era um `double*` e passou a ser `double complex*`.
#
# NENHUMA ASSERCAO APANHA ISTO, porque o medidor morre antes de imprimir — e com stdout em
# buffer, morre SEM DEIXAR UMA LINHA: a bateria ve exit 139 e um ficheiro vazio.
#
# O CRITERIO E' O NOME DO FICHEIRO, e nao o tipo. A primeira versao deste verificador
# comparava tipos, e deixou passar duas variaveis distintas do MESMO tipo no mesmo indice —
# que colidem na mesma. Dois `disco_prende` com nomes de ficheiro diferentes no mesmo `i`
# sao dois corpos a disputar uma pagina, e isso decide-se sem ambiguidade.
#
#   sh tools/indices.sh
RAIZ=$(cd "$(dirname "$0")/.." && pwd)
cd "$RAIZ" || exit 1
exec python3 - "$RAIZ" <<'PY'
import re, sys, glob, collections
raiz = sys.argv[1]
# TECTO: o espaco de enderecos do utilizador em x86-64 acaba em 0x7FFF_FFFF_FFFF (47 bits).
# Com DISCO_LAJE = 0x2000_0000_0000 e DISCO_PASSO = 256 GiB cabem exactamente
#     (0x8000_0000_0000 - 0x2000_0000_0000) / 0x40_0000_0000  =  384  lugares, 0..383.
# Um indice acima disso NAO da' erro de compilacao: da' ENOMEM no mmap, e o medidor morre.
TECTO = 383
fich = sorted(glob.glob(raiz+'/tests/*.c') + glob.glob(raiz+'/banco/*.c')
              + glob.glob(raiz+'/lib/*.h') + glob.glob(raiz+'/tools/*.c'))
re_prende = re.compile(r'disco_prende\s*\(\s*DISCO_BASE\s*\(\s*([0-9]+)\s*\)\s*,\s*"([^"]+)"')
re_idx    = re.compile(r'DISCO_(?:FIXO2?|BASE)\s*\([^()]*?\b([0-9]+)\s*\)')
mau, usados, opacos = 0, set(), []
for f in fich:
    s = open(f, encoding='utf-8', errors='replace').read()
    porind = collections.defaultdict(set)
    for i, nome in re_prende.findall(s):
        porind[int(i)].add(nome)
    for i in sorted(porind):
        if len(porind[i]) > 1:
            print("  COLIDE  %s  indice %d  <- %s" %
                  (f[len(raiz)+1:], i, ", ".join(sorted(porind[i]))))
            mau += 1
    usados |= {int(m) for m in re_idx.findall(s)}
    # ── OS INDICES QUE NAO SAO LITERAIS ──────────────────────────────────────────
    # DISCO_BASE(ix[i_]) esconde o indice numa tabela, e o teste de cima nao o ve. Duas
    # coisas se fazem, e nenhuma delas e' calar: verifica-se a TABELA (dois iguais na
    # mesma lista sao a mesma colisao), e CONTA-SE o que fica por verificar.
    for nome, corpo in re.findall(r'static\s+const\s+int\s+(\w+)\s*\[[^\]]*\]\s*=\s*\{([^}]*)\}', s):
        if not re.search(r'DISCO_BASE\s*\(\s*' + nome + r'\[', s): continue
        vs = [int(x) for x in re.findall(r'-?\d+', corpo)]
        rep = sorted({v for v in vs if vs.count(v) > 1})
        if rep:
            print("  COLIDE  %s  tabela %s repete %s"
                  % (f[len(raiz)+1:], nome, ", ".join(str(v) for v in rep)))
            mau += 1
        usados |= set(vs)
    n_op = len(re.findall(r'DISCO_BASE\s*\(\s*[A-Za-z_]', s))
    if n_op: opacos.append((f[len(raiz)+1:], n_op))
alto = max(usados) if usados else -1
print("  ── %d ficheiros, %d colisoes de indice" % (len(fich), mau))
if opacos:
    print("  ── %d indices por tabela ou variavel, verificados so' pela tabela: %s"
          % (sum(n for _, n in opacos), ", ".join("%s(%d)" % t for t in opacos[:6])))
print("  ── indices: %d distintos, maior %d, tecto %d (%d por usar)"
      % (len(usados), alto, TECTO, TECTO + 1 - len(usados)))
if alto > TECTO:
    print("  ACIMA DO TECTO: o indice %d nao e' mapeavel — mmap devolve ENOMEM" % alto)
    mau += 1
sys.exit(1 if mau else 0)
PY
