---
name: feedback-a-macro-por-definir
description: "Escrevi `\\sen` e `\\FF` num paper que não os define. Os papers/*.tex são `article` independentes e não herdam preâmbulo de ninguém — o erro só aparece em quem compilar."
metadata:
  node_type: memory
  type: feedback
---

Ao registar um resultado no `papers/arquitetura.tex` escrevi `\sen` e
`$\FF_{127}$`. Os dois existem na casa — `\sen` no `estilo.tex` da raiz,
`\FF` em quatro papers — e **nenhum dos dois está definido no
`arquitetura.tex`**, que é um `\documentclass{article}` independente e não
herda preâmbulo de ninguém.

A varredura que se seguiu achou mais três, antigos: `\abs` usado sem definição
no `corpo_analitico` e no `corpo_topologico`, e `\colunas` no
`corpo_analitico`.

**Why:** eu leio a casa como um documento só. As macros são vocabulário
partilhado *na minha cabeça*, e no disco cada `article` é um universo fechado.
É exatamente a família da referência órfã ([[feedback-a-cobertura-que-nao-acompanhou]]):
o defeito está escrito, compila em quem tem o preâmbulo, e **daqui não se vê**
— a regra da casa é que medir contra o `pdflatex` é intransportável, logo não
havia nada a apanhá-lo.

**How to apply:**
1. Antes de usar uma macro da casa num paper, `grep -c 'newcommand{\\x}'`
   **nesse ficheiro**. Custa um comando.
2. Preferir o comando padrão quando existe: `\sin` em vez de `\sen`,
   `\mathbb{F}` em vez de `\FF`, quando é uso isolado.
3. Ficou medido em `tests/refs.c` §R6 — «toda macro da casa usada num
   documento está definida NELE» —, com controlo negativo próprio: «nenhuma
   falta» passa em qualquer extractor que não colha nada, pelo que se constrói
   o caso com material verdadeiro (uma macro exclusiva de um documento) e se
   exige que a lógica veja que não está noutro.

E duas vezes o extractor mentiu antes de eu publicar: `catalogo`/`teoria` usam
`\documentclass[livro.tex]{subfiles}` e herdam o preâmbulo (seis falsos
positivos), e `\C` era o `\\` de quebra de linha seguido de `C` num
`smallmatrix`. [[feedback-a-definicao-do-extractor]] — verificar SEMPRE antes
de afirmar o número.
