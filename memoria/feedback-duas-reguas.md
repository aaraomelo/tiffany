---
name: feedback-duas-reguas
description: SEIS vezes num dia — medir por uma fonte e desenhar por outra. E a variante do texto: a convenção declarada LONGE do primeiro uso
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-08T06:21:30.112Z
---

**Duas réguas para o mesmo objecto.** O defeito que mais me custou em 07-08/08, e apareceu
**seis vezes no mesmo dia**, sempre com rostos diferentes e **sempre com o mesmo sintoma
visível: letras coladas ou palavras sobrepostas**.

| onde | medi por | desenhei por |
|---|---|---|
| `/Widths` vs `FontFile` | a tabela base-14 da Helvetica | a fonte embutida |
| desenho por corpo | `d-cc1728` (o do corpo) | `documento-versalete` (o de 10 pt) |
| um só `FontFile3` | a carta de cada variante | **um único ficheiro** para 16 objectos |
| `largura()` | `(fonte==F_NEG) ? &CARTA_N : &CARTA_R` | a carta do par (variante, corpo) |
| o `p->fundo` | rebobinei o stream principal | **o fundo não** → réguas a dobrar |
| `\texttt` | mapeado para a negra | monoespaçada, que não tenho |

**Why:** o Aarão diz-o assim — «uma régua e um lápis, e os dois têm de vir do mesmo corpo».
O `escala_espaco.c` já o tinha escrito: o espaçamento SOMA e a escala MULTIPLICA, e separá-los
por fontes diferentes «é pôr cada metade do par num corpo diferente, e aí eles deixam de ser
duais de nada».

**How to apply:** o sítio esquecido foi **sempre o mesmo tipo de coisa** — um `if` ou um
ternário escrito quando só havia duas fontes, que sobreviveu à generalização. Antes de dar
por resolvido: procurar `? &` e `== F_` no ficheiro, e perguntar se cada um ainda vale com N
variantes. E quando o sintoma for letra colada, **não é posição: é a régua a não separar** —
o `corpo-estelar.tex` §coexistem, «o conflito por espaço não é uma luta: é falta de
resolução».

Ver [[feedback-a-referencia-escrita-a-mao]] e [[project-checkpoint-2026-08-07]].

**E a pior variante é o NOME DE UM MEMBRO a servir de nome da FAMÍLIA.** Chamei `F_k` à sucessão
metálica `U^{(m)}_{k+2} = m·U_{k+1} + U_k`, em código e em paper. **A conta estava certa** —
`A_2² = (5,2;2,1)`, e não `(2,1;1,1)` —, mas `F` lê-se Fibonacci, e Fibonacci é só `m = 1`.
Um revisor externo leu o paper e apontou-o como erro matemático em três secções; nenhuma
asserção minha o podia apanhar, porque **as asserções comparavam a conta com ela própria**.

**How to apply:** quando uma letra serve uma família indexada, ela leva o índice
(`U^{(m)}`) — e a defesa é medir a DIVERGÊNCIA: `U^{(m)}` coincide com `F` em exactamente
um dos oito metais, e diverge já em `k = 2`. Isso transforma a confusão de nome num
número que falha se eu voltar a colá-las. Ver [[project-a-reta-construida]].


## E a variante do TEXTO: a convenção declarada longe do primeiro uso

16/08/2026, na revisão do `geometrico.tex`. O paper **declarava**, na §`sec:reta`:

> o discriminante `D = tr² − 4det` — **escrito `D`, e não `Δ`, que fica reservado à diferença
> finita**

e depois usava `Δ` para o discriminante em **oito linhas** (10 ocorrências). Duas réguas para o
mesmo objecto, em LaTeX, num paper que declara a régua no próprio texto.

**A causa é posicional:** a declaração estava na linha 741 e o **primeiro uso** na 201 — 540
linhas acima. Uma convenção que nasce a meio do documento não governa o que veio antes, e
ninguém a lê duas vezes.

**How to apply:** `grep` do símbolo antes de dar por fechado, e comparar a **primeira** ocorrência
com o sítio onde a convenção é declarada. Se a declaração não é a primeira, ela não é convenção —
é nota de rodapé. E o gume barato: se dois objectos distintos partilham símbolo, o compilador não
se queixa e nenhum medidor apanha, porque **o texto não corre**.
