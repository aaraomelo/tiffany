#!/bin/bash
# hostinger-dns.sh — o TXT do _acme-challenge, escrito e apagado pela API da Hostinger.
#
# É o auth-hook e o cleanup-hook do certbot para o cert WILDCARD da Patria, que só se pode
# validar por DNS-01. Sem isto o certbot corre duas vezes por dia e falha sempre, que foi o
# que aconteceu entre 06/07/2026 (a expiração) e 21/08/2026.
#
#   ./hostinger-dns.sh auth       usa CERTBOT_DOMAIN e CERTBOT_VALIDATION do certbot
#   ./hostinger-dns.sh cleanup    apaga os TXT que esta corrida pôs
#   ./hostinger-dns.sh backup     guarda a zona inteira, e não toca em nada
#   ./hostinger-dns.sh teste      põe um TXT inócuo, confirma-o por dig, apaga-o
#
# O TOKEN vive em /root/.hostinger_token (chmod 600) e NUNCA aparece em argumentos nem em log.
set -u
TOKEN_FILE=/root/.hostinger_token
ZONA=patriatechnology.com
API=https://developers.hostinger.com/api/dns/v1
GUARDA=/root/dns-backups

[ -r "$TOKEN_FILE" ] || { echo "sem $TOKEN_FILE — não há como escrever no DNS"; exit 1; }
TOKEN=$(tr -d ' \n\r' < "$TOKEN_FILE")
[ -n "$TOKEN" ] || { echo "$TOKEN_FILE está vazio"; exit 1; }

api() {  # api <método> <caminho> [corpo]
  local m=$1 p=$2 corpo=${3:-}
  if [ -n "$corpo" ]; then
    curl -sS -X "$m" "$API$p" -H "Authorization: Bearer $TOKEN" \
         -H 'Content-Type: application/json' -H 'Accept: application/json' -d "$corpo"
  else
    curl -sS -X "$m" "$API$p" -H "Authorization: Bearer $TOKEN" -H 'Accept: application/json'
  fi
}

backup() {
  mkdir -p "$GUARDA"; chmod 700 "$GUARDA"
  local f="$GUARDA/zona-$(date -u +%Y%m%dT%H%M%SZ).json"
  api GET "/zones/$ZONA" > "$f" || return 1
  # uma zona válida traz registos; se veio erro, o backup não vale e o resto não corre
  grep -q '"type"' "$f" || { echo "a API não devolveu uma zona: $(head -c 300 "$f")"; return 1; }
  echo "$f"
}

# o nome do registo: _acme-challenge para o apex e para *.dominio; para *.dev.dominio
# é _acme-challenge.dev — o subdomínio entra no meio, e é isso que o certbot espera.
nome_do_registo() {
  local d=${1#\*.}                       # tira o "*." se vier
  if [ "$d" = "$ZONA" ]; then echo "_acme-challenge"
  else echo "_acme-challenge.${d%.$ZONA}"; fi
}

case "${1:-}" in
  backup) backup ;;

  auth)
    : "${CERTBOT_DOMAIN:?falta CERTBOT_DOMAIN}" "${CERTBOT_VALIDATION:?falta CERTBOT_VALIDATION}"
    NOME=$(nome_do_registo "$CERTBOT_DOMAIN")
    B=$(backup) || exit 1
    echo "  zona guardada em $B"
    # overwrite:false ACRESCENTA. É a diferença que interessa: o apex e o wildcard validam no
    # MESMO nome, o certbot chama este hook duas vezes, e os dois TXT têm de coexistir. Com
    # overwrite:true o segundo apagava o primeiro e a emissão falhava a meio.
    R=$(api PUT "/zones/$ZONA" "{\"overwrite\":false,\"zone\":[{\"name\":\"$NOME\",\"type\":\"TXT\",\"ttl\":60,\"records\":[{\"content\":\"$CERTBOT_VALIDATION\"}]}]}")
    echo "  API: $(echo "$R" | head -c 200)"
    echo "  esperando que $NOME.$ZONA publique o valor…"
    for i in $(seq 1 40); do
      if dig +short TXT "$NOME.$ZONA" @1.1.1.1 2>/dev/null | tr -d '"' | grep -qF "$CERTBOT_VALIDATION"; then
        echo "  visto ao fim de $((i*15))s"; sleep 10; exit 0
      fi
      sleep 15
    done
    echo "  o TXT não apareceu em 10 minutos"; exit 1
    ;;

  cleanup)
    NOME=$(nome_do_registo "${CERTBOT_DOMAIN:-$ZONA}")
    api DELETE "/zones/$ZONA" "{\"filters\":[{\"name\":\"$NOME\",\"type\":\"TXT\"}]}" >/dev/null 2>&1
    echo "  $NOME limpo (ou deixado; um TXT a mais não faz mal a ninguém)"
    ;;

  teste)
    B=$(backup) || exit 1
    echo "zona guardada em $B ($(wc -c < "$B") bytes)"
    V="teste-$(date -u +%s)"
    api PUT "/zones/$ZONA" "{\"overwrite\":false,\"zone\":[{\"name\":\"_acme-teste\",\"type\":\"TXT\",\"ttl\":60,\"records\":[{\"content\":\"$V\"}]}]}" | head -c 200; echo
    for i in $(seq 1 20); do
      dig +short TXT "_acme-teste.$ZONA" @1.1.1.1 2>/dev/null | tr -d '"' | grep -qF "$V" && { echo "TXT publicado em $((i*10))s — a escrita funciona"; break; }
      sleep 10
    done
    api DELETE "/zones/$ZONA" "{\"filters\":[{\"name\":\"_acme-teste\",\"type\":\"TXT\"}]}" | head -c 200; echo
    sleep 5
    dig +short TXT "_acme-teste.$ZONA" @1.1.1.1 2>/dev/null | grep -q . \
      && echo "ATENÇÃO: o TXT de teste ainda lá está — a limpeza não funcionou" \
      || echo "TXT de teste apagado — os dois lados funcionam"
    ;;

  *) echo "uso: $0 auth|cleanup|backup|teste"; exit 2 ;;
esac
