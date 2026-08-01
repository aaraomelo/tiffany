---
name: project-publicacao-patria
description: "Como o tiffany chega ao ar: o fork na Patria, o front do goldenkingdom, os dois workflows cruzados, e as três armadilhas do deploy"
metadata: 
  node_type: memory
  type: project
  originSessionId: 2e442d4f-0e54-4e4d-b500-96b10b6085bc
  modified: 2026-08-01T16:47:23.189Z
---

# A publicação na Patria — a infraestrutura, e as armadilhas

Montado em 01/08/2026. **`https://goldenkingdom.patriatechnology.com`** serve o front do Reino
Dourado (`chess/app`, Vite) e, agora, os papers do tiffany.

## O desenho

    push no tiffany  ->  publica.yml  ->  espelha no fork + avisa o chess
    push no chess    ->  deploy.yml   ->  build -> R2 -> servidor
                              ^
                         repository_dispatch: tiffany-atualizado

- **O fork**: `/root/tiffany-repo.git` (bare) na VPS, e uma cópia servida por *dumb HTTP* em
  `/var/www/goldenkingdom/repo.git`. O front tem um botão para ele; `git clone
  https://goldenkingdom.patriatechnology.com/repo.git` funciona.
- **Três documentos no ar**, e só três: **teoria**, **catálogo**, **enredo**. O GDD e o manual
  vivem *dentro* do enredo; o microprocessador saiu da página. A contagem está em **quatro
  sítios** (manifesto, ícones, validação pré-deploy, health check) de propósito.
- **Nenhum binário no git.** Os PDFs são compilados no deploy a partir do `.tex` — os do tiffany
  saem de `pdflatex` sobre o próprio fork.

## O workflow da Patria (o molde)

`patria/landpage/.github/workflows/deploy.yml`, e os outros seis apps seguem-no:
build → validação com `vite preview` + `curl` → `tar.gz` → `aws s3 cp` para
`s3://patria-deploys/` no **endpoint R2 da Cloudflare** → **presigned URL** de 30 min → SSH: o
servidor baixa com `curl` → health check → `aws s3 rm` (cleanup).

**A diferença do goldenkingdom:** é servido direto pelo nginx do host
(`/var/www/goldenkingdom`), **não por container** — o passo do `docker compose` dá lugar a
extrair e trocar o diretório.

## As três armadilhas, todas apanhadas por teste

1. **O SPA fallback do nginx corrompia o clone.** `try_files $uri $uri/ /index.html` devolvia
   **200 + HTML** para um objeto git inexistente, e o git tentava descomprimir HTML →
   *"inflate: data stream error"*. O clone recuperava pelo packfile mas cuspia erros. Curado com
   uma `location /repo.git/` **antes** do fallback, com `try_files $uri =404` e `gzip off`.
   Também é preciso **remover o `objects/info/commit-graph`** — o nginx gzip-a e dá o mesmo erro.
2. **`git clone --depth 1` NÃO funciona com dumb HTTP** — *"does not support shallow
   capabilities"*. Eu escrevera `--depth 1` por hábito, e teria falhado no primeiro deploy.
3. **O `rsync --delete` apagaria o fork.** O `repo.git` vive dentro do `DEPLOY_DIR` e não vem no
   artefato. O script extrai para um diretório novo e usa `--exclude 'repo.git'`.

E um erro meu: **subi 9,6 MB de PDFs no git** (o `catalogo.pdf` no commit da separação). O Aarão
apanhou — *"não sobe binário no git"*. Saíram do índice e entraram no `.gitignore`.

## OS DOIS WORKFLOWS ESTÃO VERDES E NO AR (01/08/2026, fim do dia)

Os segredos foram copiados da Patria com a conta do Aarão e os dois circuitos fecharam:

    push no chess    -> deploy.yml   -> 200, e os três PDFs servidos
    push no tiffany  -> publica.yml  -> espelha no fork, e o catálogo RECOMPILA sozinho

Verificado: `/docs/{enredo,teoria,catalogo}.pdf` a 200 com `application/pdf`, o clone público a
apontar ao commit da hora, e o `catalogo.pdf` a crescer de 664457 para 670798 bytes com as duas
entradas novas — ou seja, a compilação no deploy está mesmo a acontecer a partir do `.tex`.

**E o `publica.yml` falhou mais duas vezes no SSH, as duas no *antes-da-ligação*:** primeiro
`Network is unreachable` (o runner resolve o host em IPv6 e não tem rota — curado com
`AddressFamily inet`), depois `Connection timed out`, que passou na re-tentativa. É exatamente a
coluna que o `sshb.c` mediu: **o SSH tem um antes-de-haver-ligação onde falhar, e o bump não tem.**

## As QUATRO falhas do deploy, e nenhuma era o pdflatex

O meu diagnóstico era "faltam pacotes LaTeX no runner". O container `texlive/texlive` resolveu
isso numa linha, e depois vieram quatro que eram todas outra coisa:

1. **O `pip` recusou** — o Debian do container marca o Python como *externally-managed*; é preciso
   `--break-system-packages`. E o `2>/dev/null || true` que eu lá tinha posto **engolia a recusa**,
   três linhas abaixo do meu próprio comentário a dizer *"um passo que pode falhar tem de deixar
   ver porquê"*.
2. **Faltava o `poppler-utils`** — o `integra_docs` chama `pdfinfo`.
3. **A ordem dos geradores** — o `manual_pdf.py` conferia PDFs que o `integra_docs.py` ainda não
   escrevera.
4. **E ao inverter, falhou o outro** — porque **cada um dos dois confere OS TRÊS e só gera dois**.
   Num disco vazio o primeiro a correr falha sempre. Inverter só trocou quem falhava.

A quarta é a que ensina, e tem memory próprio: [[feedback-o-disco-limpo]]. **O runner limpo não é
mais rigoroso — ele só não tem o meu passado.** Os três PDFs estavam no meu disco desde a primeira
vez que corri aquilo à mão, e mascaravam um ponto morto do ciclo.

E também: o container corre como **root** (fora o `sudo` do install do AWS CLI) e o node vem do
`setup-node`, não do apt.

## Onde as coisas estão

- `chess/deploy/publicar.sh` — o script manual antigo (rsync direto)
- `chess/.github/workflows/deploy.yml` — o novo, pelo R2
- `tiffany/.github/workflows/publica.yml` — espelha e avisa
- `patria/secrets.txt` — **não tem** as credenciais R2 (estão só nos GitHub Secrets)
- config nginx: `/etc/nginx/sites-available/goldenkingdom.patriatechnology.com` (backup em
  `/root/gk.nginx.bak.*`)
