---
name: project-cert-wildcard-expirado
description: "O wildcard *.patriatechnology.com esteve expirado 46 dias porque o certbot não podia renová-lo; RESOLVIDO a 21/08/2026 com hooks DNS-01 pela API da Hostinger — e o que aprender disso"
metadata:
  type: project
---

# O certificado da Patria: 46 dias expirado, e o conserto que o torna automático

**RESOLVIDO a 21/08/2026.** O que está abaixo é o diagnóstico; o conserto vem no fim.

Achado a 20/08/2026, a medir a linha de base do smoke test do ERP.

    /etc/letsencrypt/live/patriatechnology.com   expirou  06/07/2026
    /etc/letsencrypt/live/goldenkingdom          válido   12/10/2026

**Porquê:** o cert da Patria cobre `*.patriatechnology.com`, `*.dev.…`, `*.homolog.…` e o apex
— é **wildcard**, e wildcard só se valida por **DNS-01**. Foi emitido com
`authenticator = manual`, portanto o `certbot.timer` corre (de facto corre, duas vezes ao dia) e
falha **sempre**, com:

    PluginError: An authentication script must be provided with --manual-auth-hook
                 when using the manual plugin non-interactively

O `goldenkingdom` renova sozinho porque **não é wildcard**: usa `authenticator = webroot`
(HTTP-01). É por isso que o site do tiffany está no ar com SSL bom e o resto da Patria não.

**O que isto derruba:** tudo o que serve por aquele cert — o ERP em produção
(`{tenant}.patriatechnology.com`) e em homologação (`erp-homolog…`), `api`, `loja`, `social`,
`hiper`, `nco`, `dev`, `homolog`, `ouro`. **Os serviços estão de pé** — `curl -k` dá 200 nos
dois do ERP, e os containers respondem 200 em `127.0.0.1:8090` e `:8091`. O que está partido é
só o certificado, e é de fora que se vê.

**As duas saídas:**
1. **À mão, agora:** `certbot certonly --manual --preferred-challenges dns` e criar o TXT
   `_acme-challenge` no DNS. Repõe o SSL, e volta a expirar daqui a 90 dias.
2. **Automático, de vez:** um plugin DNS do certbot com token de API. O DNS está na
   **Hostinger** (`lunar/solar.dns-parking.com`) e **não há nenhum plugin DNS instalado** no
   servidor. Ou se usa a API da Hostinger, ou se passa o DNS para um provedor com plugin
   oficial (Cloudflare é o caminho batido).

Nenhuma das duas foi feita: exige credenciais de DNS que eu não tenho, e é produção.
Ver [[project-erp-em-plataforma]].


## O conserto (21/08/2026)

`/root/hostinger-dns.sh` é o `--manual-auth-hook` e o `--manual-cleanup-hook`: escreve e apaga
o TXT `_acme-challenge` pela API da Hostinger (`developers.hostinger.com/api/dns/v1`), com o
token em `/root/.hostinger_token` (chmod 600, nunca em argumentos nem em log).

**As três coisas que o script tem de fazer bem, e porquê:**

1. **`overwrite:false` no PUT.** O apex e o wildcard validam-se no MESMO nome
   `_acme-challenge.patriatechnology.com`, e o certbot chama o hook uma vez por cada: com
   `overwrite:true` o segundo apagava o primeiro e a emissão falhava a meio. Os dois TXT
   têm de coexistir.
2. **Backup da zona antes de cada escrita**, em `/root/dns-backups/`. Um PUT distraído numa
   zona de DNS derruba tudo o que a Patria serve.
3. **Esperar pela publicação** (poll com `dig @1.1.1.1`), não dormir um número fixo. Medido:
   a Hostinger publica em ~10–15s.

**E o hook que faltava:** `/etc/letsencrypt/renewal-hooks/deploy/reload-nginx.sh`. O plugin
`manual` escreve o ficheiro e mais nada — não é o plugin `nginx`, não recarrega ninguém. Sem
isto o nginx serviria o cert velho em memória até alguém reparar, que é a maneira lenta de
repetir a mesma avaria.

Cert novo: `notAfter=Nov 18 2026`. Verificado de fora **sem `-k`**, que é o teste que estava
a falhar: 200 em `erp-homolog`, `patria`, `api`, `goldenkingdom` — e num tenant inventado
(`tenant-que-nao-existe.patriatechnology.com`), que é o que prova o wildcard.

**Separado disto e NÃO consertado:** `loja.` e `social.` dão **502** — ambos fazem proxy para
`127.0.0.1:9300` e não há nada a escutar nessa porta. É backend em baixo, não é TLS, e é
anterior a esta mudança.
