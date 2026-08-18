---
name: feedback-o-medido-sem-medidor
description: "28 blocos \\medido afirmam número e resíduo sem nomear programa nenhum — a bateria não os vê porque só conta o que é citado, e o cruzamento por números não os recupera"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-07T05:56:59.173Z
---

07/08/2026. A bateria conta **319 : 319 verdes** e essa contagem está certa — mas ela só vê o que é
**citado**. Um bloco `\medido{...}` que não nomeia ficheiro `.c` nenhum está fora do universo que ela
percorre: não é uma referência quebrada (não há referência), não é um medidor que falha (não há
medidor). É invisível por construção.

São **28**: 18 em `teoria.tex`, 10 em `catalogo.tex`. Os dois papers (`corpo_analitico`, `dualsort`)
estão limpos — zero.

## Como se acham

```python
# localizar cada \medido{...} contando chavetas, e ver se o bloco cita .c ou .py
```
Não serve `grep '\medido'` sozinho: os blocos têm várias linhas e chavetas aninhadas.

## O que NÃO funciona para os recuperar

Tentei cruzar os números afirmados com as saídas dos medidores em `/tmp/bateria/`. **Falhou**, por
duas razões que valem para a próxima vez:

1. **Falsos positivos por tamanho.** `101`, `144`, `200`, `400` aparecem em quase todas as saídas. Só
   números com 4+ dígitos e pouco redondos discriminam.
2. **Falsos positivos do meu parser.** O regex apanhava `$k=1,3,5,7,9$ dão $n=1,4,11,29,76$` e
   colava tudo em `13579` e `14112976` — números que não existiam em lado nenhum, e que eu ia
   reportar como órfãos. Ao extrair números de LaTeX, **as listas separadas por vírgula não são um
   número**.

Verifiquei os quatro candidatos mais distintivos um a um (`3240`/`1522` contra `gerador.txt`, `6348`
contra `dualcifra.txt`, `6561` contra `dna.txt`, `100001` contra `pi_rei.txt`): **todos coincidências**.
Num caso o número aparecia, mas o outro número do mesmo bloco não — e um bloco só se confirma se
**todos** os seus números aparecerem.

## O que isto quer dizer

Não são necessariamente falsos. Muitos parecem resultados reais de sessões antigas. Mas são
**inauditáveis**: afirmam número e resíduo, e não há como voltar a correr. E são exatamente do tipo
de frase que já me traiu — *"resíduo 0"*, *"sem uma única exceção"*, *"zero discordâncias"*.

A legenda do próprio catálogo (`catalogo.tex`, na definição de `\medido`) promete: *«o enunciado foi
verificado por programa (…) **e o medidor vai indicado**»*. Vinte e oito não cumprem a promessa que a
legenda faz.

**Para resolver:** por cada um, ou achar o medidor e citá-lo, ou escrever o medidor, ou retirar a
afirmação. Não há quarto caminho — e deixá-los como estão é o que os faz passar por medidos.

Relacionado: [[feedback-o-medidor-que-nunca-mediu]] (a atestação guardava o resultado e não o motivo),
[[feedback-assercoes-vazias]], [[feedback-dois-caminhos]].
