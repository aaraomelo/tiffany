---
name: project-checkpoint-2026-08-03
description: "03/08: o romance sem uma fórmula, o repo auto-contido, e a teoria com enunciado único e cada secção a declarar o que é"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T07:38:08.597Z
---

03/08/2026, madrugada e manhã. **45 commits.**

## O estado

| | | |
|---|---|---|
| `teoria.tex` | 79 pp. | enunciado único + 32 secções com estatuto |
| `catalogo.tex` | 311 pp. | medidores, corpos, realizações, GDD |
| `enredo.tex` | 297 pp. | **zero fórmulas** (eram 199) |
| **`livro.tex`** | **691 pp.** | os três num volume, por `subfiles` |

**277 medidores** (277 no disco com asserção, 277 citados — nenhum órfão). Os quatro PDFs no ar.

## O que a sessão fez

1. **O romance ficou sem uma fórmula.** 199 → 0. Depois ainda saíram nove *palavras* técnicas
   (involução, impedância, isomorfo…) — limpar símbolos não limpa léxico. Ver
   [[project-tres-documentos]].
2. **O repo ficou auto-contido**: veio o `app/` e as `figuras/` do chess, um workflow só, o R2 fora.
   E nasceu o **livro completo**.
3. **A cadeia nova**: a Armadura é a túnica ([[project-armadura-e-tunica]]); o relógio de luz dá o
   tempo pelo ângulo; a escada do diabo mostra que ½ não tem dual a apresentar
   ([[project-escada-do-diabo]]); a involução justifica-se pela conservação.
4. **A arquitetura**, que foi o pedido final e o que mais rendeu:
   - **o enunciado central**, que não existia — estava espalhado por **51 frases**;
   - **cada secção com estatuto**: consequência · realização · ferramenta · navegação · design;
   - **o critério corrido**: teoria 15/15 realizações passam (com controlo negativo a falhar em 6);
     catálogo **3 secções diziam realização e não medem** (38 subsecções), e o GDD+Manual (117
     subsecções) passaram a **design** declarado.

## Defeitos que estavam publicados e nenhum medidor apanhava

`colback=` impresso no PDF (8×) · quatro `Observação ??` · um `\appendix` a meio · o design do
enredo só nos últimos 4% · **um deploy a dizer success sem publicar, dois dias** · dois "exemplos de
involução" com ordem 4 · **`R+T+A=1` era tautologia no medidor** (A definido como 1−R−T) · **seis
cabeçalhos de secção vazios** (3 na teoria, 3 no catálogo). Nasceu daí `tools/refs.c`.

## As três lições, e são regras de escrita

- [[feedback-o-sujeito-da-frase]] — o sujeito é o resultado; o clássico entra como cláusula.
  **Teste:** contar o nome do morto na secção.
- [[feedback-dual-exige-dois]] — escrever *dual* obriga a nomear o par.
- [[feedback-procurar-na-bateria-antes]] — o `colheita.c` já media o que eu escrevia pior.

E a que atravessa tudo: **os números estavam sempre certos; o que era falso era a frase que os
ligava.** Aconteceu com a escada, com a inflexão, com Hurwitz, com a complementaridade das áreas.

## O que fica por fazer

- A **revisão externa** continua a não acontecer: 15 agentes lançados, **2 entregaram**. O formato
  que funciona é **afirmações concretas com números para atacar**, enviadas depois de o agente já
  estar idle — nunca "revê o documento".
- No catálogo, as 3 secções de exposição (38 subsecções) podem ganhar medidor ou ser cortadas.
