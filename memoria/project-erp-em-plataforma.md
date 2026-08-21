---
name: project-erp-em-plataforma
description: "O patria-erp É o tiffany/plataforma/ desde 20/08/2026 — subtree merge com os 112 commits, o GitHub arquivado, e o deploy do ERP que parou com isso"
metadata:
  type: project
---

# O ERP mudou-se para dentro do tiffany (20/08/2026)

Estava em `~/Documentos/patria/patria-erp`, repositório próprio, remote
`Patria-Labs/patria-erp` (privado). Agora **É o `tiffany/plataforma/`** — sem nível
intermédio: `plataforma/erp-api`, `plataforma/erp-app`, `plataforma/poster-artist`. Pousou
primeiro em `plataforma/patria-erp/` e subiu um andar logo a seguir (361 renomeações puras,
zero inserções e zero deleções), porque o nome estava dito duas vezes.

- **Veio por subtree merge, não por cópia.** O commit `0fe8f2d` tem DOIS pais; o segundo é a
  ponta do ERP. `git log 0fe8f2d^2` → **112 commits**. A tag **`erp-antes-da-mudanca`** aponta
  para essa ponta e é o nome estável para o passado dele.
- **`git log -- plataforma/` dá UM commit**, e `--follow` dá **zero** — medido, não
  suposto: os commits antigos guardam os caminhos sem o prefixo. Quem quiser o passado usa
  `^2` ou a tag. Isto é a natureza do subtree merge, não um defeito da importação.
- **O GitHub ficou ARQUIVADO** (read-only, privado), não apagado — guarda os 3 branches
  remotos. As 7 branches locais estavam todas fundidas no `main`, e o `main` sincronizado com
  o `origin`: nada ficou para trás.
- **O deploy voltou, e parte daqui**: `.github/workflows/erp.yml` (na RAIZ — os dois que
  vieram no subtree eram letra morta, porque o GitHub só corre os da raiz, e foram apagados).
  O servidor é o mesmo e os `vars`/`secrets` do tiffany já serviam: **SSH_HOST e SSH_USER eram
  iguais nos dois repositórios**, e ambos tinham `SSH_KEY`.
  - **push no master com `paths: plataforma/**` → HOMOLOGAÇÃO**; produção é sempre
    `workflow_dispatch` com o ambiente à mão. Mudança deliberada: lá, push no main ia para
    produção — aqui o master é o tronco diário e um commit distraído não pode chegar aos tenants.
  - **Onde chega:** produção `{tenant}.patriatechnology.com` → `:8090` (container `patria-erp`,
    o nginx passa o subdomínio em `X-Tenant`); homologação `erp-homolog.patriatechnology.com`
    → `:8091`. A fonte vai por rsync para `/root/patria-erp[-homolog]/` e o `deploy/deploy.sh`
    de lá faz `docker build` + troca o container. Os segredos de runtime são ficheiros
    `/root/.erp_*` no servidor e não passam pelo workflow.
  - **Medido antes de escrever** (checkout simulado com `git archive` + `rsync --dry-run -i`):
    **0** a apagar, **0** com conteúdo diferente, 359 só com mtime. O que está publicado É o que
    está em `plataforma/`. Testes: 9 suítes, 61 passam.
  - Nada mais liga o servidor ao repo arquivado: sem `.git` nos diretórios, sem runner, sem
    webhooks, sem deploy keys. O único cron do ERP é o `/root/erp-backup.sh` (local).
- **1,2 GB de `node_modules` (três) vieram no disco e NÃO no git**: os `.gitignore` do próprio
  ERP continuam a valer dentro do tiffany — medido com `git check-ignore` — e cobrem também o
  `erp-api/.env` e o `clients/`. Versionados: 361 ficheiros.
- **O Prisma cozinha o caminho absoluto** em `node_modules/.prisma/client/*.js` — partiu-se
  nas DUAS mudanças (a vinda e o achatamento), e o conserto é `npx prisma generate` em
  `plataforma/erp-api`. **É a única coisa que a mudança de caminho parte**: zero symlinks
  quebrados, e nenhuma outra referência.
- **E a lição do medidor:** o `grep -rl` sobre os 1,2 GB devolveu-me FALSO NEGATIVO e eu
  publiquei «zero caminhos antigos» com ele. Um grep que procura o que já não existe **não
  distingue "não há" de "não procurei"** — o par obrigatório é procurar, na mesma corrida, o
  caminho VELHO (tem de dar 0) e o NOVO (tem de dar >0). Ver [[feedback-o-controlo-a-tres-linhas]].

Em `~/Documentos/patria` ficaram `landpage`, `patria-api`, `patria-app`, `patria-bridge`,
`patria-worker` — e essa pasta **não é** um repositório git.

Ver [[project-publicacao-patria]] e [[project-tres-documentos]].
