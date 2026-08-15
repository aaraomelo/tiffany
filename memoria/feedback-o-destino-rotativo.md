---
name: feedback-o-destino-rotativo
description: "O buffer estático que roda: mais chamadas num printf do que fatias e a linha imprime o número errado — e nenhuma asserção o vê, porque as asserções leem os valores."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T04:48:12.639Z
---

A FAMÍLIA: o valor está certo e o **texto** sai errado. Nenhuma asserção o apanha, porque
as asserções leem os valores. Já apareceu em quatro formas:

`frac2` (e `fc_da_borda`) devolvem um ponteiro para um buffer estático **que roda** — quatro
fatias. Escrever mais de quatro chamadas num único `printf` faz a última sobrescrever a
primeira, e a linha sai com o número errado:

- `∫₀³x²` — sete chamadas: «F(3) − F(0) = 0 − 3 = 3».
- a sucessão de Cauchy de √2 — cinco chamadas: «1, 4/3, 7/5, 24/17, **1**», quando o
  quinto termo é 41/29.
- a testemunha de um contra-exemplo **devolvida por estrear**: a função saía num teste
  anterior sem preencher o par de saída, e a linha imprimia memória —
  «f(0·0) = 140720728132432». Regra: **quem devolve testemunha escreve-a em TODOS os
  caminhos de saída**, incluindo os de recusa antecipada.
- `%ld` escrito dentro de `tique()` ou de `ok()`, que recebem texto PRONTO e **não
  formatam**: o literal sai para o ecrã. Sete sítios ao todo, alguns a durar sessões.
- o `%-Ns` a encher por **bytes** sobre texto acentuado («bisseção» tem 8 caracteres em 10
  bytes): a coluna sai torta. Cura: `esc_col`, que conta cabeças de UTF-8.

**Why:** é uma família de defeitos que **não tem asserção possível do lado do valor**, porque o valor
está certo — o defeito é do TEXTO. E o texto é o que o Aarão lê. Toda a bateria pode estar
verde e a linha impressa mentir. É o irmão de [[feedback-assercoes-vazias]] pelo avesso: não
é a asserção que passa sem poder falhar, é o erro que passa sem asserção que o cubra.

**How to apply:** a medida tem de ser do lado da **FONTE**, e é `tools/bench_destino.sh`:
conta as chamadas rotativas por `printf` e recusa acima do `FRAC2_N` que a **própria fonte
declara** (ler o teto em vez de o escrever — senão era um número meu, ver
[[feedback-a-referencia-escrita-a-mao]]). Regra de escrita: **uma chamada por `printf`**, ou
contar.

E o gatilho geral: quando uma função devolve `const char *` para memória estática, perguntar
**quantas ao mesmo tempo**. Se a resposta é «uma rotação», então há um teto, e o teto merece
um medidor — não um comentário.
