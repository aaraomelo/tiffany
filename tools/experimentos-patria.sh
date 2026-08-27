#!/usr/bin/env bash
# experimentos-patria.sh — MEDIR O BANCO NO SERVIDOR, onde o canal/UDP existe.
#
# POR QUE EXISTE. O `banco/sql.c` é POSIX puro: pread/pwrite, sockets BSD, UDP
# multicast. Num portátil Windows não compila nativo, e o que se arranja com
# stubs mede o stub — as duas asserções do CANAL falham lá porque o multicast de
# loopback não passa pelo winsock emulado. Medir o banco onde o canal não
# funciona é dizer que não se mediu. A `srv1559444` é Linux: ali o `sql.c`
# compila com um `cc` só, e o canal é UDP a sério.
#
# ISTO É O PROCEDIMENTO, e não uma explicação a repetir. Um comando embarca a
# árvore, compila nativo, corre os medidores e traz os números. A chave é a de
# `tools/chave-patria.sh` (segredo/id_ed25519_patria); o host e o utilizador vêm
# das variáveis do repo (SSH_HOST/SSH_USER), como em todo o resto desta casa.
#
#   bash tools/experimentos-patria.sh            # embarca, compila, mede tudo
#   bash tools/experimentos-patria.sh canal      # só o banco via UDP (canal)
#   bash tools/experimentos-patria.sh limpa      # apaga o diretório remoto
#
# O QUE NÃO TOCA. Trabalha num diretório próprio no servidor (/root/tiffany-exp),
# em /tmp para os binários e bases de teste. Não mexe na publicação, em container
# nenhum, nem em base de cliente. `sql teste` cria e apaga o seu /tmp/sql_teste.*.
#
# O CANAL SAI DO LOOPBACK aqui: corre-se o `sql teste` uma segunda vez com
# TIFFANY_CANAL_IF=any, para o backend do canal falar pela placa real e não só
# consigo próprio — é «o banco via UDP» medido fora do loopback.
set -euo pipefail

REPO="${REPO:-aaraomelo/tiffany}"
KEY="${PATRIA_KEY:-$(cd "$(dirname "$0")/.." && pwd)/segredo/id_ed25519_patria}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REMOTO="${REMOTO:-/root/tiffany-exp}"

[ -f "$KEY" ] || { echo "falta a chave $KEY — corre 'bash tools/chave-patria.sh puxar' primeiro"; exit 1; }

host(){ gh variable get SSH_HOST -R "$REPO" 2>/dev/null || echo ""; }
user(){ gh variable get SSH_USER -R "$REPO" 2>/dev/null || echo "root"; }
H="$(user)@$(host)"
SSHOPT=(-i "$KEY" -o StrictHostKeyChecking=accept-new -o ConnectTimeout=25)
sshx(){ ssh "${SSHOPT[@]}" "$H" "$@"; }

# ── EMBARCAR: tar por cima do ssh ────────────────────────────────────────────
# Só o que os medidores tocam: lib/, banco/, tests/, tools/. Nada de .git, nada
# de PDFs, nada de dados, e NUNCA o segredo/. Os `--exclude` vêm ANTES dos
# caminhos --- o GNU tar ignora-os em silêncio se vierem depois, e foi o que
# partiu a primeira corrida. tar-só (sem rsync) porque o git-bash do Windows não
# traz rsync, e um caminho só é um caminho a menos que falha.
embarca(){
  echo "→ embarcar lib/ banco/ tests/ tools/ para $H:$REMOTO"
  sshx "mkdir -p '$REMOTO'"
  tar -C "$ROOT" \
      --exclude='.git' --exclude='*.pdf' --exclude='dados' --exclude='segredo' \
      -czf - lib banco tests tools \
    | sshx "tar -C '$REMOTO' -xzf - && echo '  ✓ embarcado' && du -sh '$REMOTO'"
}

# ── O MEDIDOR NO SERVIDOR ────────────────────────────────────────────────────
# Corre remoto e devolve exit != 0 se algo ficar vermelho. As unidades #UNIT são
# contadas; as três que falham no Windows (merkle, banda, slot) NÃO devem falhar
# aqui — é Linux nativo, e é esse o ponto.
mede(){
  sshx "bash -s" <<REMOTO_SCRIPT
set -euo pipefail
cd '$REMOTO'
echo '==================== TOOLCHAIN ===================='
cc --version | head -1; uname -srm
echo
echo '==================== COMPILAR O MOTOR (nativo, sem stub) ===================='
cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sqlb banco/sql.c -lm
echo "OK  sql.c -> /tmp/sqlb (\$(stat -c%s /tmp/sqlb) bytes)"
echo
echo '==================== sql teste (98 unidades; ZERO falhas aqui) ===================='
rm -f /tmp/sql_teste.*
/tmp/sqlb teste > /tmp/sql_teste.out 2>&1 || true
ok=\$(grep -ac '^#UNIT ok' /tmp/sql_teste.out || true)
ma=\$(grep -ac '^#UNIT falha' /tmp/sql_teste.out || true)
echo "unidades: \$ok ok, \$ma falha(s)"
[ "\${ma:-0}" = 0 ] || { echo '-- as que falharam --'; grep -a '^#UNIT falha' /tmp/sql_teste.out; }
echo '-- o CANAL como backend (é isto que só se mede aqui) --'
grep -a -E '#UNIT (ok|falha) +(o Word atravessou a banda|e o slot e o endereco)' /tmp/sql_teste.out || echo '  (asserções do canal não apareceram)'
echo
echo '==================== O BANCO VIA UDP, FORA DO LOOPBACK ===================='
echo '   (segunda corrida do canal com TIFFANY_CANAL_IF=any: a placa real, não só o loopback)'
rm -f /tmp/sql_teste.*
TIFFANY_CANAL_IF=any /tmp/sqlb teste > /tmp/sql_udp.out 2>&1 || true
grep -a -E '#UNIT (ok|falha) +(o Word atravessou a banda|e o slot e o endereco)' /tmp/sql_udp.out || echo '  (canal off-loopback: sem saída — ver /tmp/sql_udp.out no servidor)'
echo
echo '==================== A BANDA E A ANTENA (tests/canal.c §N1-N4) ===================='
cc -O2 -std=c99 -w -Ilib -Itests -o /tmp/canal tests/canal.c -lm && /tmp/canal | tail -8
echo
echo '==================== A BANDA VIVA: DOIS BANCOS, DOIS PROCESSOS, POR UDP ===================='
cc -O2 -std=c99 -w -Ilib -o /tmp/bv tests/banda_viva.c
echo '-- no loopback --';                /tmp/bv prova 12 | grep -a '#UNIT'
echo '-- fora do loopback (placa real, banda propria) --'
TIFFANY_CANAL_IF=any TIFFANY_TECIDO="a minha banda propria" /tmp/bv prova 12 | grep -a '#UNIT'
bv_ma=\$( { /tmp/bv prova 12; TIFFANY_CANAL_IF=any /tmp/bv prova 12; } 2>&1 | grep -ac '#UNIT falha' || true)
echo
echo '==================== AS OITO RELAÇÕES (tests/simbolos.c) ===================='
cc -O2 -std=c99 -w -Ilib -Itests -o /tmp/simbolos tests/simbolos.c -lm && /tmp/simbolos | tail -3
echo
echo '==================== ζ ACUMULA, μ DESACUMULA (tests/zetamu.c) ===================='
cc -O2 -std=c99 -w -Ilib -Itests -o /tmp/zetamu tests/zetamu.c -lm && /tmp/zetamu | grep -a -E '#UNIT|unidades,'
echo
echo '==================== OS SEIS SINAIS CONTRA O ORÁCULO ===================='
B=\$(mktemp -d); cd "\$B"
/tmp/sqlb t "CREATE TABLE t (a,c)" >/dev/null
for v in 1 5 10 20 33 50; do /tmp/sqlb t "INSERT INTO t VALUES (\$v,\$((v*2)))" >/dev/null; done
tot=0; mau=0
for op in "=" "<>" "<" ">" "<=" ">="; do for k in 0 1 20 33 50 100; do for neg in 0 1; do
  if [ "\$neg" = 1 ]; then w="NOT (a \$op \$k)"; else w="a \$op \$k"; fi
  esp=""
  for v in 1 5 10 20 33 50; do
    case "\$op" in "=") t=\$([ "\$v" -eq "\$k" ]&&echo 1||echo 0);; "<>") t=\$([ "\$v" -ne "\$k" ]&&echo 1||echo 0);;
      "<") t=\$([ "\$v" -lt "\$k" ]&&echo 1||echo 0);; ">") t=\$([ "\$v" -gt "\$k" ]&&echo 1||echo 0);;
      "<=") t=\$([ "\$v" -le "\$k" ]&&echo 1||echo 0);; ">=") t=\$([ "\$v" -ge "\$k" ]&&echo 1||echo 0);; esac
    [ "\$neg" = 1 ] && t=\$((1-t)); [ "\$t" = 1 ] && esp="\$esp\$v|\$((v*2)) "
  done
  got=\$(/tmp/sqlb t "SELECT * FROM t WHERE \$w" 2>&1 | grep -E '^ +[0-9-]+ \|' | tr -d ' ' | tr '\n' ' ' || true)
  tot=\$((tot+1)); [ "\$(echo \$got)" != "\$(echo \$esp)" ] && { mau=\$((mau+1)); echo "ERRADO \$w esp=[\$esp] got=[\$got]"; }
done; done; done
cd /; rm -rf "\$B"; rm -f /tmp/sql_teste.* /tmp/sql_udp.out
echo "\$tot consultas contra o oráculo, \$mau erradas"
[ "\${ma:-0}" = 0 ] && [ "\${mau:-0}" = 0 ] && [ "\${bv_ma:-0}" = 0 ] \
  || { echo; echo "❌ HÁ VERMELHO: \$ma unidade(s), \$mau consulta(s), \$bv_ma banda(s)"; exit 1; }
echo; echo "✅ tudo verde no metal — o banco mede-se onde o canal existe, e a banda está viva por UDP"
REMOTO_SCRIPT
}

case "${1:-tudo}" in
  tudo)   embarca; mede ;;
  canal)  embarca
          echo "→ só o canal, fora do loopback:"
          sshx "cd '$REMOTO' && cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sqlb banco/sql.c -lm && \
                cc -O2 -std=c99 -w -Ilib -Itests -o /tmp/canal tests/canal.c -lm && /tmp/canal | tail -10 && \
                echo '--- backend do canal, off-loopback ---' && \
                rm -f /tmp/sql_teste.* && TIFFANY_CANAL_IF=any /tmp/sqlb teste 2>&1 | \
                grep -aE 'atravessou a banda|o slot e o endereco'" ;;
  limpa)  sshx "rm -rf '$REMOTO'"; echo "✓ $REMOTO apagado no servidor" ;;
  *)      sed -n '2,20p' "$0"; echo "subcomandos: tudo | canal | limpa"; exit 1 ;;
esac
