---
name: feedback-o-double-que-so-transportava
description: "O tests/hurwitz.c tinha 20 doubles e um limiar 1e-9, e o gerador SEMPRE produziu inteiros — o double não carregava nada de fracionário, só trazia uma régua de borla."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T17:59:08.392Z
---

Reparei que `tests/hurwitz.c` media o cristal N(xy)=N(x)N(y) com **20 doubles e uma
tolerância `1e-9`**. Disse-o ao Aarão em vez de o corrigir; ele: **«rira os doubles tudo
dicreto enteiro»**.

**Por que era pior do que parecia:** o gerador `gera()` sempre produziu **inteiros** (um
hash `% 19 - 9`), e Cayley–Dickson só usa `+`, `−`, `×`, que não saem de ℤ. O double não
transportava nada de fracionário. Ele só trazia, de borla, um **limiar escolhido por mim**:

    antes:  «a norma é multiplicativa» = «o resíduo é menor que 1e-9»   ← régua minha
    depois: «a norma é multiplicativa» = «o resíduo é ZERO»             ← sem régua nenhuma

São afirmações diferentes, e a segunda é mais forte. Em dim 16 o resíduo passou de
`4.842e-01` (um número que só quer dizer algo relativo a outro) para **182204**, um
inteiro que é o que é.

## O gatilho

**Um tipo com vírgula onde os dados nunca têm vírgula.** Perguntar: *de onde vêm as
entradas?* Se vêm de contagem, de hash, de índice ou de tabela, o float é transporte, não
necessidade — e todo float traz um `ε` que passa a decidir por mim o que conta como zero.

## E as duas regras que a conversão obrigou

1. **Não mudar o dado ao mesmo tempo que o tipo.** Deixei `gera()` letra por letra igual.
   Se mudasse os dois, não podia comparar as versões — e a comparação é a única prova de
   que não perdi cobertura. Medido: os **7 veredictos originais idênticos**, tabelas iguais.
   ([[feedback-a-chave-faz-parte-da-medida]], [[feedback-dois-caminhos]])

2. **O inteiro tem teto, e o teto verifica-se.** Pus `#define TETO` e um contador
   `estouros` em cada conta, porque um limite que ninguém testa é documentação
   ([[feedback-o-teto-nao-verificado]]). Deu 0.

E na comparação dos dois caminhos apanhei-me a contar a **minha própria prosa** como dado:
a frase nova «e não «zero abaixo de um limiar meu»» tinha um «não» que entrou no diff —
[[feedback-a-chave-faz-parte-da-medida]] outra vez, na mesma hora.

7 → 12 asserções, 0 falhas. Ver [[feedback-inteiro-primeiro]],
[[project-o-fecho-do-dual-lagrange]].
