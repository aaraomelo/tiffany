---
name: feedback-o-controlo-a-tres-linhas
description: "Quando um resultado bom aparece, o controlo custa três linhas — e desfá-lo mais vezes do que o confirma"
metadata: 
  node_type: memory
  type: feedback
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-05T22:01:55.504Z
---

Procurei o centro que zerava o resíduo de uma involução. **Achei**: caiu de `0,386` para `0,0185` — 21× melhor. Ia reportar como resultado.

Mas o optimizador tinha fugido para longe: `|c| = 140` contra `|c_média| = 6,5`. Isso pedia um controlo, e o controlo eram **três linhas**: gerar um `c` ao acaso *com a mesma norma* e medir.

```
optimo (procurado)   0.018514
acaso, mesma norma   0.018549 .. 0.018851     <- o MESMO
```

**Degenerescência.** O resíduo é uma razão; com `|c|` enorme, `x−c ≈ −c` para todo `x`, e numerador e denominador ficam ambos dominados por `c`. A razão vai a zero sem dizer nada.

**Why:** o defeito não é procurar — é aceitar o que se procurou sem perguntar *o que daria uma escolha qualquer com a mesma liberdade*. Um optimizador que foge para o infinito está sempre a dizer que a função objectivo tem uma saída degenerada, e isso lê-se na magnitude do parâmetro antes de se ler no valor.

**How to apply:** sempre que um número melhora muito, gerar o **mesmo tipo de objecto ao acaso, com a mesma magnitude**, e medir. Se o acaso empata, não há resultado. Gatilhos concretos:

- o parâmetro óptimo tem magnitude muito acima da escala natural dos dados;
- a métrica é uma **razão** (normalizada) — é onde a degenerescência se esconde;
- o ganho é grande e veio de acrescentar um grau de liberdade.

Nesta sessão o mesmo padrão apareceu do outro lado: a linha que imprimia *«QUATRO passos fecham (0.458860)»* **não comparava com nada** — e `0,459` estava acima do controlo `0,422`. Declarar em vez de comparar é o mesmo defeito sem optimizador.

Ver [[feedback-normalizar-nao-e-medir]], [[feedback-assercoes-vazias]], [[feedback-dois-caminhos]].
