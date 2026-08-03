---
name: feedback-a-referencia-escrita-a-mao
description: "Ao corrigir uma asserção vazia, calculo a referência de cabeça e ESCREVO o resultado em vez de o derivar — reintroduzindo o defeito dentro da própria correção. Quatro vezes num dia"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T20:10:51.318Z
---

03/08, a varrer a bateria por asserções que passam sem poder falhar. Corrigi dezenas. E **em
quatro delas reintroduzi o mesmo defeito dentro da correção**:

| ficheiro | o que escrevi na "correção" | porque é o mesmo defeito |
|---|---|---|
| `aurea.c` | um `1 == 1` para substituir uma identidade | literal puro |
| `lambert.c` | `soma_a = 1 + 2, soma_b = 1 - 1` para φ² + φ⁻² | escrevi o resultado de ℤ[φ] de cabeça |
| `nne.c` | `g2_n = 5 - 3*3` como referência exata | não acompanha se o exemplo mudar |
| `quantico.c` | `tr_im = 0, det_im = 0` no hermitiano | **afirmei** que a parte imaginária anula |

O último é o mais claro: eu estava a corrigir uma asserção porque ela não calculava nada — e a
minha correção também não calculava, só escrevia `0`.

## O MECANISMO

Ao construir a referência exata eu **já sei a resposta** (foi por isso que a chamei referência).
Sabendo-a, escrevo-a. Mas uma referência escrita à mão não testa a função: testa se eu copiei bem.
É a mesma família do número de cabeça, com outra roupa — ali o alvo era um decimal transcrito, aqui
é o resultado de uma conta que fiz na cabeça.

## A REGRA

**A referência exata deriva-se dos dados de entrada, com as operações do objeto.** φ⁻² sai de
multiplicar `(−1,1)` por si em ℤ[φ]; a parte imaginária sai do produto de Gauss; `g` sai de `r1q` e
`C1`. Se eu apagar a linha onde escrevi o valor, o teste tem de continuar a saber a resposta.

**TESTE:** mudar o dado de entrada (o exemplo, o primo, o expoente). Se a referência não mudar
sozinha, ela não é uma referência — é uma cópia.

## A QUINTA VEZ, e foi a HORA DEPOIS de escrever isto

A cobrir o diâmetro hidráulico do `microfluidica.c` — a fórmula estava em linha, sem cobertura.
Escrevi o teste **repetindo a fórmula dentro do teste**. Passou verde, e a mutação da linha
original continuou a não acusar: eu estava a testar a minha cópia.

A correção não é escrever melhor o teste — é **extrair a fórmula para uma função** e chamar a
mesma dos dois lados. Enquanto o uso e a medida forem dois textos, medem-se um ao outro por
acaso; quando são um só símbolo, a mutação atravessa.

**REGRA:** para cobrir uma expressão em linha, primeiro dá-lhe um nome. Só depois a testes.

## E O QUE O APANHA

Não é reler. Reli, e passou nas quatro. **É a mutação** — estragar a *função* (não a referência) e
ver se a asserção acusa. Foi assim que os quatro apareceram. Ligada a [[feedback-assercoes-vazias]]
(as oito formas) e a [[feedback-dois-caminhos]]: a referência exata só vale como segundo caminho se
os dois caminhos forem mesmo **dois**.
