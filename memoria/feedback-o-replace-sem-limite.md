---
name: feedback-o-replace-sem-limite
description: "`str.replace` sem `count` atinge sítios que eu não vi — TRÊS vezes num dia, e sempre em linhas idiomáticas que a casa repete."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-16T01:46:21.145Z
---

Num só dia, três vezes: usei `s.replace(a, b)` em Python sem o terceiro argumento, e ele
mudou TODAS as ocorrências. Os estragos:

1. `for(int k = 1; k <= 14; k++){` — mudou o laço da **homotopia** de 14 para 16, e o do
   **dual** também. Duas asserções vermelhas.
2. `printf("      os dez: %d por número…")` — renomeou o rótulo do andar da **ponte** para
   «os dezasseis», num bloco que nada tinha a ver.
3. `vmal == 0 && por_n == 16 && por_nome == 16` — mudou para 17 as asserções do **tensor**
   e das **formas**. Mais duas vermelhas.

**Why:** o código desta casa é IDIOMÁTICO — os andares repetem a mesma linha letra por
letra, porque é a mesma frase dita para objectos diferentes. É exactamente essa virtude
que faz do `replace` global uma arma: quanto melhor a casa está escrita, mais sítios a
mesma linha tem.

**How to apply:**
1. **`count=1` por omissão.** `s.replace(a, b, 1)` sempre, e só sem limite quando a
   intenção é mesmo global e eu o disse.
2. Quando o alvo é uma linha genérica, **ancorar no contexto**: substituir o bloco que a
   contém, ou percorrer as linhas e trocar só as que têm o vizinho certo (`resolve_X` a
   quatro linhas de distância).
3. E o sintoma é sempre o mesmo e é barato: correr o interno logo a seguir. As três vezes
   apareceram como **vermelhas noutro andar**, nunca no que eu estava a editar.

Ver [[feedback-duas-reguas]] (letras coladas), [[feedback-o-write-que-diz-updated]].

**E a variante que ACUMULA:** `sed -i 's/(\$21{:}0\$)/(\$24{:}0\$)/g'` para
actualizar a contagem do MEU medidor — e o paper citava outro com a mesma contagem. O
`arquimedes_area.js`, que tem **21**, foi arrastado ronda após ronda:
`21 → 24 → 27 → 36 → 38 → 43 → 45 → 48 → 51 → 57 → 58`. Onze substituições, nenhuma vista.

**Why:** a contagem de um medidor é um número de dois dígitos, e há dezenas de medidores. A
colisão não é improvável — é provável.

**How to apply:** actualizar contagens é o caso onde o `count=1` não basta, porque a
ocorrência certa não é a primeira: tem de ser **ancorada no nome do ficheiro**, e nunca no
número sozinho. E a defesa barata: conferir cada `(N:0)` do paper contra a bateria — foi
assim que apareceu, e o excesso de margem foi o que me fez olhar. Ver
[[feedback-o-numero-que-nao-cabe]].
