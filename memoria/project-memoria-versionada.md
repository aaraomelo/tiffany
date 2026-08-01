---
name: project-memoria-versionada
description: "A memória passou a ser versionada em tiffany/memoria/ — onde está, como sincronizar, e porque o checkpoint mudou de dois passos para três"
metadata:
  type: project
---

# A memória é versionada — `tiffany/memoria/`

Desde 01/08/2026 (commit `a7c1d4b`). O Aarão: *"versiona os memories."*

A memória viva continua em `~/.claude/projects/-home-aaraolopes-Documentos-tiffany/memory/`, que
**não é repositório** — persiste no disco e desaparece com ele. A cópia versionada faz duas coisas:
sobrevive à máquina, e **fica legível por quem lê o repo**, que é o ponto de um projeto público.

## O CHECKPOINT PASSOU A TER TRÊS PASSOS

Isto é o que importa lembrar, porque é fácil esquecer o terceiro e a cópia fica velha em silêncio:

    1. escrever/atualizar os .md na memória viva
    2. tools/memoria/sincroniza.sh guarda        <- o passo NOVO
    3. git add memoria && commit && push

O `sincroniza.sh` vive em `memoria/` e vai nas **duas direções**, e **nunca adivinha qual**:
`guarda` (viva → repo, antes de commitar) e `restaura` (repo → viva, numa máquina nova). *Uma cópia
que adivinha a direção apaga o lado errado, e a memória é a única coisa no repo que não se refaz
medindo* — um medidor perdido corre-se outra vez; um memory perdido leva consigo a razão pela qual
algo foi decidido.

## O que foi verificado antes de subir, e tem de ser sempre

**O repo é público e o histórico do git é permanente.** Antes do primeiro commit varri o conjunto
por e-mail, caminho local, hostname de servidor e credencial — nada. Isso **repete-se a cada
memory novo**, e não é formalidade: um segredo que entre aqui não sai do histórico.

Ver [[project-publicacao-patria]] (o repo público e o fork), [[feedback-assercoes-vazias]] (o
memory que mais vale a quem chega de fora).


## A DIREÇÃO, e ela custou-me as edições de um checkpoint (01/08/2026)

`sincroniza.sh guarda` copia de `~/.claude/.../memory/` **para** `tiffany/memoria/`. Editei
`memoria/MEMORY.md` diretamente, corri o `guarda`, e ele **sobrescreveu tudo com a cópia velha**.
O `git status` mostrou só o ficheiro novo (que sobreviveu por não existir na fonte), e as duas
edições tinham desaparecido em silêncio.

```
a FONTE   ~/.claude/projects/-home-aaraolopes-Documentos-tiffany/memory/   ← escrever AQUI
a CÓPIA   tiffany/memoria/                                                  ← o guarda enche
```

**Escrever na fonte, depois `guarda`.** Escrever na cópia é escrever no destino de um `rsync` —
o trabalho existe até ao próximo sincronismo, e depois não existe.
