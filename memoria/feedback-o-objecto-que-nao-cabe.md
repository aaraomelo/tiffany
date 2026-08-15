---
name: feedback-o-objecto-que-nao-cabe
description: "A soma parcial exata de Σ1/n³ saiu NEGATIVA — e o defeito não era o guarda de estouro, era eu ter construído um objeto que não precisava de existir."
metadata: 
  node_type: memory
  type: feedback
  originSessionId: 1b414fab-4a31-4b15-bef4-49020ec22a30
  modified: 2026-08-15T20:30:00.743Z
---

Acumulei a soma parcial exata de Σ1/n^p como uma fração. Em p = 3, N = 40 o resultado
saiu **negativo**: o denominador é da ordem de lcm(1..40)³ e estourou o `long`. E o meu
contador de estouros **não viu** — eu guardava a potência `n^p` e não a **soma**.

**O defeito não era o guarda. Era o OBJECTO.** A convergência de uma série-p não se
decide olhando para o valor da soma. Decide-se por **comparação que telescopa**, e aí os
números ficam minúsculos:

    1/n^p ≤ 1/(n−1) − 1/n     →     Σ_{n=2}^{N} ≤ 1 − 1/N < 1

E a divergência da harmónica também é exata, por blocos de ≥ 1/2 — **sem somar bloco
nenhum**: verifica-se a desigualdade termo a termo (`n ≤ 2^{k+1}`) e a contagem (`2^k`
termos). Dois números pequenos em vez de uma soma que não cabe.

## E foi DUAS VEZES na mesma hora

A primeira versão dos blocos também somava, e deu **«9 de 12 blocos ≥ 1/2»** — o mesmo
defeito, e o «9 de 12» é que o denunciou (devia ser 12 de 12, porque a desigualdade é
sempre verdadeira). Um número que devia ser total e não é: [[feedback-dois-caminhos]].

## O gatilho

**Antes de acumular exatamente, perguntar: este objeto precisa de existir?** Se o que se
quer é uma *desigualdade* ou uma *decisão*, construir o valor exato é trabalho a mais que
estoura. A regra da casa já o dizia para outra coisa
([[feedback-representacao-inteligente]]): a série custa e dá lixo, a forma fechada custa
nada e dá o exato. Aqui é a versão para provas: **prove-se a desigualdade, não se calcule
a soma**.

E o corolário para o guarda: se o guarda vive na *sub-expressão* e o estouro acontece na
*acumulação*, ele nunca dispara. O guarda tem de estar onde os números crescem —
[[feedback-o-teto-nao-verificado]], [[feedback-o-numero-que-nao-cabe]].

## E quase fiz uma tautologia ao lado

No mesmo ficheiro, o Fubini tinha um parâmetro `ordem` que a função **ignorava** — o que
tornaria «as duas ordens concordam» uma comparação vazia
([[feedback-assercoes-vazias]]). Reescrito para que os dois integrais iterados construam
objetos intermédios **diferentes**: um colapsa y e deixa um polinómio em x, o outro
colapsa x. Só o número final coincide.

Ver [[project-calculo-exacto]].
