---
name: project-checkpoint-2026-08-07
description: "07-08/08 — o interpretador LaTeX ganha a fonte do documento, e a capa passa de 0 a 4/5 contra o gabarito"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-08T06:22:14.054Z
---

**O dia do interpretador.** Bateria **357 : 357**. O `enredo.pdf` da raiz passou a ser
**gabarito** — o PDF do pdflatex contra o qual o tradutor se mede.

## O que o tradutor deixou de inventar

Tudo o que eu escrevia à mão passou a sair do `estilo.tex`, e **cada número meu era um
defeito visível**: a margem 64 (o estilo diz `2.6cm` = 73,7), o espaçamento 8, o corpo 10, o
degrau do capítulo (`D_CAP`=16,99 onde o `\titleformat` manda `\gktit`=23,42). Entraram: as
cores por nível, a régua dourada de `1.2pt`, os nomes do babel (lidos do `.ldf` instalado), a
numeração com o `\setcounter{chapter}{0}` que está escrito na linha 5625, a lista
`\hyphenation` — e o `\sloppy`, que é porque o pdflatex faz **zero** cortes e as minhas 62
sílabas estavam contra o documento.

**A avaliação nas raízes** substituiu o reconhecimento por nome: 74 macros definidas, 1
nomeada, 4273 usos. `\gkcapa{A}{B}{C}` é o polinómio no ponto, e traduzir é **avaliar**.

## A fonte

O gabarito embute 39 Type 1 **cm-super** (`SFRM/SFBX/SFTI/SFCC`). Extraí-as com `mutool`,
mas são subconjuntos; as completas vieram do sistema, convertidas para sfnt e postas em
`lib/fontes/` — 15 desenhos, **um por corpo**, porque a Computer Modern tem traço próprio em
cada tamanho e escalar um não dá o outro.

- `lib/spline.h` recusava a fonte sem `glyf`/`loca` → **abrir não é ler a curva**, a `hmtx`
  existe nos dois formatos;
- **TrueType e OpenType não são dois formatos**: `1,2,1` e `1,3,3,1` são linhas de Pascal, e
  a recorrência que as gera **é o passo da torre** (`tests/pascal.c`). `tests/grau.c`, 11
  unidades: elevação 2→3 com **resíduo 0 INTEIRO** por produto cruzado, e o contorno como
  **órbita** — 12 pontos → 12 harmónicos → volta, desvio 0, em `Z_13`, sem um double;
- o PDF precisa de **uma fonte por (variante, corpo)** — 11 neste documento, 32 no gabarito.

## A capa, medida bit a bit (`tools/capa.js`)

| | início | fim |
|---|---|---|
| posição | 10% | **30%** = gabarito |
| peso (tinta) | 1,83× | **0,93×** |
| cor | — | **resíduo 0** vs `\definecolor` |
| réguas | 4 | **2** = gabarito |
| unidades | 0/5 | **4/5** |

E o enredo inteiro: **142 palavras em falta** (eram 379), **2 invasões** (eram 3, e a maior
era falso positivo do medidor — uma célula que quebra não é invasão).

## Por fazer

A monoespaçada (`\texttt`, a quarta estaca — `d-tt1000.otf` já convertida), os 22,9 pt
distribuídos nos blocos da capa, a entrelinha (15,00 minha contra 13,50 do gabarito, que é o
que faz 291 páginas contra 365), e o resíduo da órbita em 53.

**Não publicado — o Aarão pediu para não fazer deploy.** ~100 commits por subir.

Ver [[feedback-duas-reguas]] e [[feedback-a-regua-nao-transporta]].
