---
name: project-checkpoint-2026-08-03-noite2
description: "A DFT saiu do universal.c, nasceu o mutagera.py, e TRÊS vezes anunciei um problema grande que a medição desfez"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T23:18:40.661Z
---

Sessão longa de 03/08. Bateria estável em **293 medidores, 288 verdes** do princípio ao fim — as
5 falhas são ambientais (ollama, corpus) e já lá estavam.

## O QUE MUDOU

**A DFT saiu do `universal.c`** — reescrito sobre a *avaliação nas raízes da borda*, sem uma
vírgula flutuante e sem uma tolerância. E a medição ficou **mais forte**, não só mais limpa:

| § | com a DFT | agora |
|---|---|---|
| U1 | 4 tamanhos amostrados, resíduo `7e-15` | o anel **inteiro**, 14 641 pares, resíduo **0** |
| U2 | varrer o expoente e achar `1/√N` | `N(ab) = N(a)N(b)` — o invariante que a borda **tem** |
| U5 | "nenhuma casa é zero" | a contagem: exatamente `(p−1)ⁿ = 100` de 121 |
| U7 | `F⁴ = id` (facto **da DFT**) | Frobenius de ordem = **grau** — o ν, não o i |

A lição está em [[feedback-a-base-ja-existe]]: era a maquinaria do círculo que **obrigava** ao
float. `|ω| = 1` só existe em ℝ, e arrastava `sqrt`, `complex.h` e um limiar por asserção.

**Nasceu o `tools/mutagera.py`** — gera mutações a partir do código em vez de as ter escritas à
mão (o `mutacao.sh` satura: 26/27 matadas e a 27.ª documentada). O que o faz valer não são os
filtros, é a **classificação**: *equivalente* (output bit a bit igual — o ramo não corre) contra
*buraco* (output muda, exit não — falta medir). Mais tarde ganhou a categoria *só-impressão*.

## O PADRÃO DO DIA, e é o mais importante

**Três vezes anunciei um problema grande e a medição desfê-lo:**

| anunciei | a medição disse |
|---|---|
| "100 medidores com `return 0` cego" | 99 tinham `if(falhas) return 1` antes |
| "51 ficheiros com π/e transcritos" | 49 são **exatos ao bit**; os 2 restantes não entram em asserção |
| "143 medidores não sabem acusar" | 99 sinalizam com outro idioma; **2** eram cegos a sério |

A raiz é sempre a mesma: **grep como substituto de medição**. O grep diz o que está *escrito*; só
a execução diz o que *acontece*. Isso corrigiu também uma guarda errada no `mutacao.sh` que existia
desde que ele foi escrito — aceitava 137 medidores onde devia aceitar 237, e o alcance do
varrimento saltou de 51 mutações para 398.

## OS BURACOS QUE ERAM REAIS

- **`fusao.c`** — o **ν** estava calculado e nunca medido (`dx`, `dy` entravam num printf e mais
  nada). Agora: ν∘ν = id e norma multiplicativa, 7203 casos.
- **`forca.c`** — o **produto cruzado** sem uma única asserção. Agora perpendicularidade e
  Lagrange, exatos em ℤ, 15 625 pares.
- **`cruzamento_geral.c`** — **tautologia lógica**, tipo novo: `eh_metal = (grau == 2)` com
  `grau = mesmo ? 2 : 4`, logo `!mesmo && eh_metal` é `!mesmo && mesmo`. Duas linhas afastadas,
  invisível à leitura.
- **`gauss.c`** — pedia `falhou > 0` (um piso); com o resto errado o resultado publicado saltava
  de 16,0% para **43,0%** com a bateria verde.
- **27 pisos viraram igualdades** — `casos >= 30` onde o valor real era 44; `casos > 3000` onde
  era **17 544**. Método reutilizável: injetar `(printf(valor), condição)` e ver se o valor sai
  único.

## A ARRUMAÇÃO

`assets/` (figuras + soltos), `tools/bin/` (25 executáveis), `tatoeba/tabelas/`. A raiz ficou só
com fontes. Duas descobertas com causa no código, não desarrumação:

- as tabelas `regua_*.bin` estavam **em três diretórios** (27 ficheiros) porque o `regua.c`
  procurava o corpus em vários sítios mas **escrevia no cwd**. Agora ancoram-se ao corpus.
- o `app/vite.config.js` era a **única** referência em código a `../figuras` — as outras sete
  eram documentação. Mover sem a corrigir partia o app em silêncio.

Ver [[feedback-a-referencia-escrita-a-mao]]: caí **seis vezes** no mesmo erro dentro das próprias
correções, e só a mutação o apanhou — reler passou nas seis.
