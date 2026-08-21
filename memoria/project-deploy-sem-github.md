---
name: project-deploy-sem-github
description: "O deploy da Patria por SSH direto — quando o GitHub Actions cai, e por que o runner local NÃO resolve"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-06T21:58:13.915Z
---

**06/08: o GitHub Actions esteve em `major_outage` e CINCO corridas morreram sem começar** — *«the job was not acquired by Runner of type hosted»*. Nada a corrigir do nosso lado: o trabalho nunca arrancou.

## O runner local NÃO contorna este outage — e é o que eu supus mal

Instalei um self-hosted runner no Patria (utilizador `runner` sem privilégios, rótulo `patria`). Ele **apanhou o trabalho à primeira**, coisa que a frota não conseguiu em cinco tentativas. Mas:

1. **caiu a auto-actualizar-se** (`SelfUpdater`, 215 MB) — o job morreu com *«lost communication»* aos 10m00s exactos, e não havia `Worker_*.log` nenhum: o job nem chegou a correr;
2. na segunda tentativa ficou 10 min em fila, e o log do runner dizia
   `Attempt 1,2,3 of POST request to run-actions-2-azure-eastus` **a falhar**.

**O outage não era falta de máquinas: era o serviço de orquestração.** O runner local também precisa dele para receber trabalho. Isto é o que eu tinha assumido mal ao propor a solução.

## O que resolve: SSH directo, sem GitHub no caminho

```bash
. ./.env                       # SSH_HOST, SSH_USER, SSH_OPTS, DEPLOY_DIR
rsync -az --delete --exclude 'repo.git' -e "ssh $SSH_OPTS" \
      app/dist/ $SSH_USER@$SSH_HOST:/var/www/goldenkingdom/
```

~~**O `--exclude 'repo.git'` é obrigatório**~~ — **caducou em 20/08/2026**: o espelho público saiu da Patria (o repositório passou a privado) e já não há nada dentro do directório de publicação a preservar, portanto o `--delete` pode ser cego. Ficou registado porque a razão continua a valer para qualquer outra coisa que passe a viver no `DEPLOY_DIR` sem vir no build: era o clone público (312 MB no fim) a ser apagado por um `--delete` sem ninguém dar por isso. Ver [[project-publicacao-patria]].

**A chave é `~/.ssh/id_rsa_patria`** (root@srv1559444.hstgr.cloud). Copiada para `segredo/`, com `.env` ao lado — ambos no `.gitignore`, escrito ANTES de copiar.

## O portão: `tools/segredo.sh`

O repo é PÚBLICO. O portão **não confia no `.gitignore` — verifica**:

- ficheiros de segredo **rastreados** (`git ls-files`, não o `.gitignore`)
- o `.gitignore` cobre mesmo (`git check-ignore`)
- **chave privada no CONTEÚDO** — um `notas.txt` com uma chave dentro passa por todas as regras de nome
- e o **HISTÓRICO**, que não se apaga: se entrou uma vez, lá fica

Mutado: forcei uma chave para o índice e ele acusou. **O histórico deste repo nunca teve segredos** — verificado.

## Verificar que subiu (o 200 sozinho mente)

O SPA fallback do nginx devolve **200 com `text/html`** para um PDF que não existe. Exigir `content-type: application/pdf`, e comparar as páginas no ar com as locais.

Ver [[project-publicacao-patria]], [[feedback-o-disco-limpo]].
