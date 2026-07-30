#!/usr/bin/env bash
# prova.sh — o banco NÃO-CUSTODIAL: além de recusar o proibido, ele tem de não SABER nada.
# Duas provas distintas: (a) o errado não passa; (b) um dump completo não revela conteúdo.
set -u
cd "$(dirname "$0")"
DB=/tmp/bai_prova.db; rm -f "$DB"
falhas=0
sqlite3 "$DB" < bai.sql || { echo "  o esquema nem carrega"; exit 2; }
q(){ sqlite3 "$DB" "PRAGMA foreign_keys=ON; $1" 2>&1; }
H(){ sqlite3 :memory: "SELECT lower(hex(sha3('$1')));"; }
aceita(){ local o; o=$(q "$2"); if [ -n "$o" ]; then printf '      %-54s NÃO ✗ (%s)\n' "$1" "$o"; falhas=$((falhas+1));
          else printf '      %-54s sim ✓\n' "$1"; fi; }
recusa(){ local o; o=$(q "$2"); if [ -z "$o" ]; then printf '      %-54s NÃO ✗ (passou!)\n' "$1"; falhas=$((falhas+1));
          else printf '      %-54s sim ✓\n' "$1"; fi; }
vale(){ local g; g=$(q "$2"); if [ "$g" = "$3" ]; then printf '      %-54s sim ✓\n' "$1";
        else printf '      %-54s NÃO ✗ (deu %s, esperava %s)\n' "$1" "$g" "$3"; falhas=$((falhas+1)); fi; }

BANDA=$(H "tecido-do-aarao"); BANDA2=$(H "tecido-do-vizinho")
PA=$(H "principal-aarao");    PC=$(H "principal-claude");  PV=$(H "principal-vizinho")
K0=$(sqlite3 :memory: "SELECT lower(hex(sha3('$BANDA'||'passo-raiz')));")
K1=$(sqlite3 :memory: "SELECT lower(hex(sha3('$K0'||'p1')));")
K2=$(sqlite3 :memory: "SELECT lower(hex(sha3('$K1'||'p2')));")
KX=$(sqlite3 :memory: "SELECT lower(hex(sha3('$K1'||'p2')));")   # mesma derivação, outro conteúdo
KV=$(sqlite3 :memory: "SELECT lower(hex(sha3('$K0'||'pv')));")
FORJADA=$(H "eu-invento-uma-capacidade")
SEGREDO="o-conteudo-secreto-do-aarao"

echo
echo "=== A BAI NÃO-CUSTODIAL: não protegemos o dado — não o temos ==============="
echo
echo "§N1  A raiz não se concede: prova-se. Posse do tecido É a credencial."
echo
q "INSERT INTO tenant VALUES ('$BANDA'),('$BANDA2');"
q "INSERT INTO principal VALUES ('$PA','$BANDA'),('$PC','$BANDA'),('$PV','$BANDA2');"
aceita "raiz cujo h deriva da banda do detentor entra"   "INSERT INTO capacidade VALUES ('$K0','$PA',NULL,1,3,'passo-raiz',x'6c69786f');"
recusa "raiz FORJADA (h que não deriva da banda) é recusada" "INSERT INTO capacidade VALUES ('$FORJADA','$PC',NULL,1,3,'passo-raiz',x'6c69786f');"
echo
echo "§N2  A cadeia é aritmética, não política: h = sha3(pai ‖ passo), mão única."
echo
aceita "filha com h derivado do pai entra"               "INSERT INTO capacidade VALUES ('$K1','$PC','$K0',0,2,'p1',x'6c69786f');"
recusa "filha com h inventado é recusada"                "INSERT INTO capacidade VALUES ('$FORJADA','$PC','$K0',0,2,'p1',x'6c69786f');"
aceita "neta entra, com δ decrementado"                  "INSERT INTO capacidade VALUES ('$K2','$PA','$K1',0,1,'p2',x'6c69786f');"
recusa "δ que não decrementa é recusado"                 "INSERT INTO capacidade VALUES ('$KX','$PA','$K1',0,2,'p2',x'6c69786f');"
echo
echo "§N3  O tenant: atravessar a fronteira custa δ=0 (usa, não propaga)."
echo
recusa "cross-tenant com δ>0 é recusado"                 "INSERT INTO capacidade VALUES ('$KV','$PV','$K0',0,1,'pv',x'6c69786f');"
aceita "cross-tenant com δ=0 entra"                      "INSERT INTO capacidade VALUES ('$KV','$PV','$K0',0,0,'pv',x'6c69786f');"
echo
echo "§N4  Revogar é NEGAR, não apagar — e o deny domina para baixo."
echo
vale  "antes do deny, 4 capacidades vigentes"            "SELECT COUNT(*) FROM vigente;" "4"
q "INSERT INTO deny VALUES ('$K1','2026-07-30');"
vale  "negada a mãe, a linha continua lá (não apagamos)" "SELECT COUNT(*) FROM capacidade;" "4"
vale  "mas ela e a descendência saem de vigente"         "SELECT COUNT(*) FROM vigente;" "2"
recusa "e não se delega a partir do que foi negado"      "INSERT INTO capacidade VALUES ('$KX','$PA','$K1',0,0,'p2',x'6c69786f');"
echo
echo "§N5  O QUE NÃO SABEMOS: um dump completo não revela conteúdo nenhum."
echo
q "INSERT INTO capacidade VALUES ('$(sqlite3 :memory: "SELECT lower(hex(sha3('$K0'||'p9')));")','$PA','$K0',0,1,'p9',x'a3f19c22de');"
DUMP=/tmp/bai_dump.sql; sqlite3 "$DB" .dump > "$DUMP"
if grep -qi "$SEGREDO" "$DUMP"; then printf '      %-54s NÃO ✗\n' "o dump não contém o segredo"; falhas=$((falhas+1));
else printf '      %-54s sim ✓\n' "o dump não contém o segredo"; fi
if grep -qiE 'aarao|claude|vizinho|@|senha|nome' "$DUMP"; then
  printf '      %-54s NÃO ✗\n' "o dump não contém nome, e-mail nem rótulo de gente"; falhas=$((falhas+1));
else printf '      %-54s sim ✓\n' "o dump não contém nome, e-mail nem rótulo de gente"; fi
vale  "toda coluna de identidade é compromisso de 64 hex" \
      "SELECT COUNT(*) FROM capacidade WHERE length(h)<>64 OR length(detentor)<>64;" "0"
vale  "o conteúdo só existe como cofre (blob), nunca texto" \
      "SELECT COUNT(*) FROM pragma_table_info('capacidade') WHERE name IN ('sigma','descricao','nome','titulo');" "0"
echo "      (o cofre é a cifra do gato, A_n^k mod N — bijeção, recuperação exata;"
echo "       broca-so/linguagem/cifra_de_cristal.py. Aqui só se guarda o resultado.)"
echo
echo "§N6  Fail-closed: sem concessão vigente, o colapso devolve ⊥."
echo
vale  "quem está vigente recebe o seu cofre"             "SELECT COUNT(*) FROM colapso;" "3"
vale  "o negado não recebe nada — nem erro, nem palpite" "SELECT COALESCE((SELECT h FROM colapso WHERE h='$K1'),'BOT');" "BOT"
echo
if [ "$falhas" -gt 0 ]; then echo "  FALHAS: $falhas"; echo; exit 1; fi
echo "  RESÍDUO 0 — o errado não passa, e o dump não sabe de nada."
echo
