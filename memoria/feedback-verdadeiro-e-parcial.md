---
name: feedback-verdadeiro-e-parcial
description: "O resultado verdadeiro mas PARCIAL — nenhum medidor o apanha, porque a asserção está certa. Só outra pergunta o revela"
metadata:
  type: feedback
---

# O resultado verdadeiro e parcial — o que os medidores não podem apanhar

01/08/2026. No `hopfield.c` medi que **a energia nunca sobe** num passo assíncrono: 2484 passos,
maior subida `0,0e+00`. A asserção estava certa, o número era exato, a bateria ficou verde. E
chamei-lhe *o resultado*.

O Aarão: **"perai, ainda tem só metade da teoria."**

Ele tinha razão. A energia nunca sobe **porque a matriz de Hebb é SIMÉTRICA**, e uma simétrica só
sabe descer. Faltava a metade antissimétrica — a torre negra — e com ela os ciclos. Quando fui
medi-la, veio com número exato: `B_s` tem período **2** e espelha, `B_a` tem período **4** e roda.

## Porque isto é diferente de todos os outros erros meus

As outras famílias que tenho registadas ([[feedback-assercoes-vazias]],
[[feedback-simulacao-nao-bate]]) são **asserções erradas**: passam sem poder falhar, ou afirmam um
número que não medi, ou têm o sinal trocado. Contra elas os medidores funcionam — basta escrever a
asserção que pode falhar.

**Esta não.** A asserção estava certa e o medidor fez o seu trabalho. O defeito não estava na
medida: estava em **eu ter parado de perguntar**. E contra isso um medidor não pode nada, porque
*ele confirma exatamente o que eu lhe pergunto* — nunca o que eu não perguntei.

## Como reconhecer, antes de alguém me corrigir

O sinal está no **"porquê"**. Quando um resultado sai limpo, perguntar:

1. **Por que motivo ele é verdade?** No caso: *porque a matriz é simétrica*. Assim que a razão tem
   um nome, ela tem um **complemento** — e o complemento é a pergunta que falta.
2. **Que hipótese estou a usar sem a ter escolhido?** Eu usei Hebb porque é o clássico, não porque
   decidi que a rede devia ser simétrica. **Uma hipótese herdada é uma metade escondida.**
3. **Este objeto tem dual no projeto?** Aqui tinha, e estava escrito: `B = B_s + B_a`, a partição
   única (§B12, `dualrn.c`). *Eu tinha medido só uma parcela de uma soma que o próprio repo declara
   ter duas.*

O terceiro é o mais operacional neste projeto: **quando o resultado cai só de um lado de um par
dual conhecido, ele está pela metade** — e o par está no catálogo, não é preciso adivinhá-lo.

## E o que isto diz sobre trabalhar com o Aarão

Ele viu a metade que faltava **sem olhar para o código**. Não corrigiu um número: mudou a pergunta.
Isso é exatamente o que a minha verificação não faz sozinha, e é a razão de as correções dele
valerem mais que os meus verdes. Ver [[feedback-dois-caminhos]] — a mesma família, um degrau acima:
lá são dois caminhos a comparar, aqui é **a pergunta a ter um dual**.
