---
name: project-tres-documentos
description: "O repo passou a ter três documentos — teoria, catálogo e enredo — e cada um tem um público e um critério"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T02:14:48.552Z
---

02–03/08/2026. O repositório reorganizou-se em **três documentos e mais nada**:

| | páginas | é | público |
|---|---|---|---|
| **`teoria.tex`** | 60 | álgebra, topologia, análise (três `\part`) | técnico |
| **`catalogo.tex`** | 303 | medidores, resultados próprios, realizações, GDD e manual | técnico |
| **`enredo.tex`** | 304 | o romance | **leigo** |

O eixo dos dois primeiros é a **dualidade de Pontryagin** (compacto↔discreto, `R̂ = R`); o terceiro
é o dual dos outros dois no sentido do público — conta o que eles medem.

## Como se chegou aqui

- Os papers soltos (`fracoes-continuas`, `realizacoes`, `analise`) **fundiram-se** no `teoria.tex`
  como três partes.
- Os `.tex` da raiz (`tiffany`, `microprocessador`, `viveiro`, `multiplicacao`, `morfico`,
  `dicionario`) **desceram ao catálogo** como *As realizações*.
- O `teoria.tex` antigo (52 pp.) **dissolveu-se**, com 30 páginas de resultados próprios
  recuperadas para o catálogo — ver [[feedback-destruir-antes-do-inventario]], porque quase se
  perderam.
- O enredo **partiu-se na costura que ele próprio tinha escrita** (linha 12421: *"daqui para trás
  o reino foi contado; daqui para a frente é o mesmo reino medido"*). O apêndice técnico —
  91,6% da matemática do ficheiro — foi para os dois irmãos.

## O que vigiar sempre que se mexer

**A `bateria.sh` monta a lista por `grep` nos três `.tex`** (linha ~76). Se um documento deixar de
citar um medidor, ele sai da corrida **em silêncio** — o total desce e continua a dizer "verdes".
Contagem atual: **232 medidores, bateria 276**. O teste obrigatório antes e depois de qualquer
reorganização:

```
grep -ohE '(tools|tatoeba)/[a-z_0-9]+\.(c|py)' teoria.tex catalogo.tex enredo.tex | sort -u | wc -l
```

## Pisos do portão (`publica.yml`)

`teoria 45 · catalogo 270 · enredo 250`. **Recalibrar sempre que o tamanho mudar** — um piso
deixado no valor antigo deixa passar um truncamento de dois terços, e já aconteceu (o de
`fracoes-continuas` estava em 6 quando o paper tinha 23 páginas).

## O container de teste está incompleto

`localhost/tex-ci` **não tem `cm-super`**, que o workflow instala. O enredo falha lá com
*"auto expansion is only possible with scalable fonts"* e compila no runner. Instalar o pacote no
container antes de concluir que um documento está partido — ver [[feedback-o-disco-limpo]], mas
com o complemento: *o disco limpo também pode estar incompleto em relação ao runner.*

## Pacotes que o catálogo passou a exigir

`longtable`, `ragged2e`, `array`, `fvextra`, `tcolorbox`, `hyphenat`, `truncate`, `xspace` — vieram
com o GDD e o manual. Todos em `texlive-latex-extra`, que o runner instala. **Carregar o pacote,
nunca definir a macro como vazia**: `\providecommand{\RaggedRight}{}` quebra o `>{...}` das tabelas.
