# A memória

O que a assistente sabe deste projeto entre sessões — e o que ela aprendeu a não repetir.

Não é documentação do código: isso está em `tools/LEIAME.md`, no `teoria.tex` e no `catalogo.tex`.
Aqui está o que **não se deduz do repositório**: decisões e o porquê delas, correções do Aarão que
mudaram uma peça, e os modos de falhar que se repetem.

## Como está organizada

- `MEMORY.md` — o índice, e é a única coisa carregada em toda sessão. Uma linha por memória.
- `feedback-*.md` — como trabalhar, e sobretudo **como eu falho**. São os mais valiosos: cada um
  nasceu de um erro concreto que voltou.
- `project-*.md` — o estado do trabalho, os checkpoints e as decisões de arquitetura.

## Porque está versionada

A memória viva fica em `~/.claude/projects/<projeto>/memory/`, que não é repositório: persiste no
disco e desaparece com ele. Versioná-la faz duas coisas — sobrevive à máquina, e **fica legível por
quem lê o repo**, que é o ponto de um projeto público.

Usa o `sincroniza.sh` nas duas direções (`guarda` antes de commitar, `restaura` numa máquina nova).
Ele nunca adivinha a direção: ela é sempre dita.

## O que ler primeiro, se leres só um

`feedback-assercoes-vazias.md` — as cinco formas de uma asserção passar sem poder falhar. Este
projeto mede tudo, e uma medida que não pode falhar é pior que nenhuma: **a que falha avisa; a vazia
conta-se como prova.**
