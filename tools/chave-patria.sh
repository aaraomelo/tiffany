#!/usr/bin/env bash
# chave-patria.sh — A CHAVE DE ACESSO À PATRIA, GUARDADA ONDE SE LÊ DO TERMINAL.
#
# O erro que isto corrige. A chave privada foi parar a um SECRET do GitHub
# (`SSH_KEY`), e um secret não se lê de volta — só serve DENTRO de um workflow.
# Testar o acesso passou a exigir um commit, o que não é acesso nenhum. E a
# chave também não estava em disco: uma chave que vive num portátil é tão
# durável quanto o portátil, e o portátil perdeu-se a 25/08/2026.
#
# O DESENHO. UMA chave só, para tudo — terminal E workflow —, em três casas:
#   1. o disco       segredo/id_ed25519_patria — a que o .env e o ssh usam. É o
#                    ÚNICO ficheiro de chave; não há segunda cópia em ~/.ssh.
#   2. uma VARIÁVEL do repo, `PATRIA_CHAVE` — o cofre durável: vive na conta, não
#      na máquina, e LÊ-SE do terminal com `gh variable get` (o secret não se
#      lia, e era esse o buraco). Nome fixo, sem id para decorar.
#   3. o secret SSH_KEY — a MESMA chave, para os workflows (publica.yml, etc.).
# E a pública vai ao servidor pelo `acesso.yml`, que a ACRESCENTA sem apagar nada.
# Uma chave, quatro sítios, o mesmo fingerprint em todos.
#
# A passphrase é OPCIONAL e é TUA. Sem `PATRIA_PASS`, a variável leva a chave em
# claro (base64) — num repo privado e só teu, quem lhe acede és tu. Com
# `PATRIA_PASS` no ambiente, leva-a cifrada (aes-256-cbc/pbkdf2), e puxa-se com a
# mesma frase. A frase, quando existe, nunca é escrita em disco nem no comando.
#
#   bash tools/chave-patria.sh gera         # cria a chave (uma vez)
#   bash tools/chave-patria.sh guardar      # chave -> variável do repo (o cofre)
#   bash tools/chave-patria.sh secret       # a MESMA chave -> secret SSH_KEY
#   bash tools/chave-patria.sh instala      # a pública -> servidor (acesso.yml)
#   bash tools/chave-patria.sh prova        # liga-se e diz olá
#   bash tools/chave-patria.sh puxar        # noutra máquina: variável -> disco
# (junta PATRIA_PASS='...' à frente do guardar/puxar para cifrar o cofre)
set -euo pipefail

REPO="${REPO:-aaraomelo/tiffany}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
KEY="${PATRIA_KEY:-$ROOT/segredo/id_ed25519_patria}"
VAR="${PATRIA_VAR:-PATRIA_CHAVE}"

host(){ gh variable get SSH_HOST -R "$REPO" 2>/dev/null || echo ""; }
user(){ gh variable get SSH_USER -R "$REPO" 2>/dev/null || echo "root"; }

# A passphrase é OPCIONAL. Com `PATRIA_PASS` no ambiente, o blob vai cifrado
# (aes-256-cbc/pbkdf2); sem ela, vai em claro — a decisão é de quem corre, e num
# repo privado e só teu a variável já só a lês tu. O `guardar` e o `puxar` usam a
# MESMA regra, pelo que o que se guardou cifrado só se puxa com a mesma frase, e
# o que se guardou em claro puxa-se sem nenhuma.
embrulha(){                 # entrada -> base64 (cifrado se houver PATRIA_PASS)
  if [ -n "${PATRIA_PASS:-}" ]; then
    openssl enc -aes-256-cbc -pbkdf2 -iter 200000 -salt -pass env:PATRIA_PASS | base64
  else
    base64
  fi
}
desembrulha(){              # base64 -> saída (decifra se houver PATRIA_PASS)
  if [ -n "${PATRIA_PASS:-}" ]; then
    base64 -d | openssl enc -aes-256-cbc -pbkdf2 -iter 200000 -d -salt -pass env:PATRIA_PASS
  else
    base64 -d
  fi
}

case "${1:-}" in
  gera)
    [ -f "$KEY" ] && { echo "já existe $KEY — apaga-o à mão se queres uma nova"; exit 1; }
    ssh-keygen -t ed25519 -N '' -C "patria-$(date +%Y%m%d) $(whoami 2>/dev/null || echo user)" -f "$KEY"
    chmod 600 "$KEY"
    echo "✓ chave nova: $KEY  (.pub ao lado)"
    ;;

  guardar)
    [ -f "$KEY" ] || { echo "falta $KEY — corre 'gera' primeiro"; exit 1; }
    embrulha < "$KEY" | gh variable set "$VAR" -R "$REPO"
    if [ -n "${PATRIA_PASS:-}" ]; then
      echo "✓ variável $VAR gravada (CIFRADA) — puxa com a mesma PATRIA_PASS"
    else
      echo "✓ variável $VAR gravada (em claro, base64) — lê-se com 'gh variable get $VAR'"
    fi
    ;;

  puxar)
    gh variable get "$VAR" -R "$REPO" | desembrulha > "$KEY"
    chmod 600 "$KEY"
    echo "✓ chave restaurada em $KEY"
    ;;

  secret)
    [ -f "$KEY" ] || { echo "falta $KEY"; exit 1; }
    gh secret set SSH_KEY -R "$REPO" < "$KEY"
    echo "✓ secret SSH_KEY rodado para a chave nova (os workflows usam-na)"
    ;;

  instala)
    [ -f "$KEY.pub" ] || { echo "falta $KEY.pub"; exit 1; }
    gh workflow run acesso.yml -R "$REPO" -f pubkey="$(cat "$KEY.pub")"
    echo "✓ acesso.yml disparado — instala a pública no servidor (acrescenta, não apaga)"
    ;;

  prova)
    [ -f "$KEY" ] || { echo "falta $KEY"; exit 1; }
    H="$(user)@$(host)"
    echo "→ ssh $H  (com $KEY)"
    ssh -i "$KEY" -o StrictHostKeyChecking=accept-new -o ConnectTimeout=20 "$H" \
        'echo "olá de $(hostname) — $(uname -srm) — $(date -u +%FT%TZ)"'
    ;;

  *)
    sed -n '2,27p' "$0"
    echo "subcomandos: gera | guardar | puxar | secret | instala | prova"
    exit 1
    ;;
esac
