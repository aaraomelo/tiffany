---
name: project-arquitetura-nao-demonstra
description: A arquitetura.tex EXECUTA e não demonstra — se precisar de prova, desce para papers/; e as citações cruzadas fazem-se pelo NOME do teorema, nunca pelo número.
metadata:
  type: project
---

Ordem do Aarão (21/08/2026): **«arquitetura evita demonstração, se precisar
desce para os papers de papers/»**.

O `arquitetura.tex` é a SÍNTESE no polo τ=−1: onde os outros constroem o
*objecto*, ele constrói a *máquina* que o executa. Por isso não redemonstra
nada — cada enunciado remete para o paper onde a prova vive. Verificação:
`0 provas locais, 0 enunciados sem remissão`.

**E a citação cruzada faz-se pelo NOME do teorema, nunca pelo número.** Duas
estavam erradas e o LaTeX não as apanha, porque são texto corrido:

    «papers/aranha.tex, Teor. 1»  →  a multiplicidade é o Teorema 11
    «papers/aranha.tex, Teor. 5»  →  o levantamento é o Teorema 28

Uma renumeração parte-as em silêncio. O número é uma coordenada; o nome é o
objecto. Ver [[project-tres-documentos]], [[feedback-duas-reguas]].
