---
name: project-checkpoint-2026-08-18-ferramentas
description: "18/08 — três conferências novas na bateria (refcruz, caminhos, tautologia), o deploy quebrado há três dias sem ninguém dar por isso, e a lição de que um detector mal calibrado treina quem o lê a ignorá-lo"
metadata:
  type: project
---

# 18/08/2026 — As três conferências, e o deploy que ninguém viu cair

## As três famílias de referências, e só uma tinha dono

| família | quem confere |
|---|---|
| medidor `.c` citado nos papers | a bateria, desde sempre |
| **documento `.tex` citado** | **novo** — parte da bateria |
| **caminho em workflow/script/config** | **`tools/caminhos.py`** |
| **label de outro documento** | **`tools/refcruz.py`** |

`\code{topologico thm:x}` **não é um `\ref`** — é texto, e o pdflatex não o vê. Apodrece
calado quando o teorema muda de nome ou de casa.

## O deploy estava quebrado há três dias

O `.github/workflows/publica.yml` verificava, em dois sítios, `dist/corpo/papers/dualsort.tex`.
A reorganização de 15/08 moveu o ficheiro para `corpus/docs/`, o manifesto acompanhou, e o
workflow não. **O build deixou de produzir aquele caminho e ninguém deu por isso** — um
`test -s` num workflow só falha em produção.

Achado por acaso, ao renomear o ficheiro. É a razão de o `caminhos.py` existir.

## A lição das ferramentas

**Todas as três nasceram a acusar mais do que existia:**

- `caminhos.py` lia `.claim` como `.c` — 12 falsos de 20
- `refcruz.py` lia comentários como citações
- `doubles_cond.py` lia campos de struct como variáveis — 25 falsos ao todo

**Um detector mal calibrado não é neutro: enche a lista de coisas que não são, e treina quem
o lê a não olhar.** Antes de ligar um à bateria, investigar TODOS os achados da primeira
corrida.

E o corolário que me apanhou: `\b` sozinho não fecha um nome. `\bt\b` casa com o `t` de
`t.instrucoes` e `\ba\b` com o `a` de `e2.a`. **Fechar só um lado deixa passar metade** — a
contagem de doubles que eu reportava há dias estava inflacionada em 25.

## Quando a ferramenta muda, remedir o PONTO DE PARTIDA

|  | início | hoje |
|---|---|---|
| régua ingénua | 257 | 135 |
| **régua fechada** | **237** | **110** |

O «197» que eu citava como baseline não saía de medição nenhuma. **Senão o progresso
reportado é a diferença entre duas réguas, e não entre dois estados.**

Relacionado: [[feedback-o-ramo-que-nunca-corre]], [[feedback-o-medido-sem-medidor]],
[[feedback-destruir-antes-do-inventario]].
