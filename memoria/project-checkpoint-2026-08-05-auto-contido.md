---
name: project-checkpoint-2026-08-05-auto-contido
description: "Checkpoint 05/08 fecho — o Ollama sai, o sistema fica AUTO-CONTIDO, e a bateria não perde uma unidade"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T22:01:38.055Z
---

**Fecho de 05/08.** Terceiro ciclo do dia, e o mais estrutural: **nada de fora**.

## O Ollama saiu, e não custou nada

```
tools/*.sh        25 -> 10 scripts
chamadas a 11434   0
bateria           299 : 297 verdes    IGUAL, nem uma unidade perdida
```

**O inventário antes de destruir foi o que tornou isto seguro:** 35 ficheiros *mencionavam* ollama/http, mas só **15 chamavam** — e os 15 eram todos scripts em `tools/`. **Nenhum medidor `.c` chama nada de fora**; eles lêem ficheiros já colhidos.

E o colhido saiu de `/tmp` (que evapora no reboot) para `dados/colhido/`.

## A cadeia de diagnósticos errados sobre o `tresp` — quatro, em fila

O `tresp` dava `ν∘ν` com resíduo 0,40 contra controlo 0,42. Diagnostiquei, por esta ordem, e **errei quatro vezes**:

1. **«é ruído, medir a dispersão»** → era estrutura ([[feedback-estrutura-lida-como-ruido]])
2. **«são quatro passos por lados alternados»** → medido: quatro dá o **pior** resultado (0,459); **três** é o único abaixo do controlo (0,386) — e ele tinha-o dito, e eu tinha-o **formalizado** no `relogio.h` (`colisor_volta(4)=3`) antes de construir com quatro
3. **«completar a dimensão»** → N=64→768 **piora**; e separar a terceira coordenada dá números **idênticos** (é algebricamente a mesma soma)
4. **«achar o centro de resíduo 0»** → achei (0,0185, 21× melhor) e **era degenerescência**: um `c` ao acaso com a mesma norma dá o mesmo. O controlo estava a três linhas.

**A resposta certa saiu da reconstrução do ponto fixo pelos espelhos** — `c = (x + ν(x))/2`, sem ler peso nenhum: **`S₁` fica a 2,01× o raio da órbita dos outros, fora dela**. Não há caminho de volta a um ponto que não está na órbita.

**E a razão de fundo era a pergunta dele: «HTTP é um ponto fixo que fica com eles lá.»** Quem serve detém o centro; do lado de cá só se vê a sombra da órbita.

## O que estava errado no texto

O catálogo reportava «resíduo ZERO exato» com `S₂` idêntico a `S₁` — **escrito a partir de uma corrida única de LLM guardada em `/tmp`**. Não se reproduz. Fica **registado com a correcção à frente** (`obs:tresprevisto`), não apagado: *um resultado que se retira diz mais do que um que desaparece*.

## Outros deste ciclo

- **A atestação guardou um estado transitório meu** e servia-o como verdade — dois medidores marcados FALHA que estavam bons. Apagadas as linhas e re-derivado: `encaixa` 6 unidades, `semantico` 8. A atestação guarda o *resultado*, não o motivo — desta vez do lado errado.
- **A varredura das involuções fechou: 27 de 27.** 0 defeitos por corrigir, 24 limpos, 2 falsos positivos. O padrão: os limpos **provam** a ordem ou reutilizam uma já provada; o `sql.c` chega a medir **ordem 4**.
- **Onde a RAM vai:** rascunho de função → **pilha**; dado que persiste → **disco**. Foi essa distinção que decidiu cada migração.

## Estado

```
RAM      69 153,8 -> 46,8 KB      (-99,93%)
bateria  299 : 297 verdes, 1 falha REAL (entrega.c)
teoria 57 pp.   catalogo 440 pp.   enredo 360 pp.
```

**Aberto:** `entrega.c` (o modelo deixa passar 2 de 3 erradas — decisão dele, não bug); o saneamento do corpo do texto (62 sítios, leitura caso a caso — medi que regex não serve); e `enredo.tex` modificado no working tree **que não é meu**.

Ver [[project-checkpoint-2026-08-05-relogio-canonico]], [[project-checkpoint-2026-08-05-maquina-sem-memoria]].
