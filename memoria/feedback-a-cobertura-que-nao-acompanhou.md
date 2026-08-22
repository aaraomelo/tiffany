---
name: feedback-a-cobertura-que-nao-acompanhou
description: "O tests/refs.c vigiava 3 documentos e o repo tinha 12. Ao estender apareceram 36 órfãs e um \\label duplicado que desviava 8 citações — nenhum era novo."
metadata:
  node_type: memory
  type: feedback
---

O `tests/refs.c` persegue a referência órfã — «não falha o pdflatex: emite
Warning, imprime `??` e o PDF sai». Lia **`teoria.tex`, `catalogo.tex`,
`enredo.tex`**, que eram o repositório em 03/08. Os `papers/` nasceram depois:
**nove documentos com `\documentclass` próprio, 590 labels, 978 referências**.

Ao estender a lista, de uma vez:

- **36 referências órfãs** no `corpo_analitico.tex`, todas a apontar para o
  `corpo_algebrico.tex` — travessia entre documentos que compilam sozinhos;
- **`\label{thm:central}` DUPLICADO** no mesmo ficheiro, em dois teoremas
  *diferentes*. Em LaTeX o segundo ganha, logo as **oito** citações ao teorema
  central apontavam para o teorema errado, em silêncio.

Nenhum destes defeitos era novo. Estavam lá, e a lei que os proíbe também.

**Why:** eu verifico se a lei existe, não se ela **alcança**. Um medidor com a
lista de alvos codificada mede o repositório do dia em que foi escrito, e o
verde continua a sair enquanto o repositório cresce à volta dele. É a mesma
família de [[feedback-o-que-esta-no-disco-e-nao-no-git]] — ali o git não
reproduzia a árvore, aqui a lista não a cobre.

**How to apply:**
1. Quando um medidor tem uma **lista fixa de alvos**, perguntar quantos alvos
   existem hoje. `ls papers/*.tex | wc -l` contra o `DOCS[]` custa um comando.
2. Preferir a lista **derivada** (glob + filtro por `\documentclass`) à
   codificada, quando o critério é dizível.
3. E a extensão tem o seu próprio gume: **`onde()` devolvia o primeiro label
   com aquele nome em QUALQUER documento**. Com três raramente colidia; com
   doze passou a acusar dezenas de «cruzadas» que são referências locais
   perfeitas (`sec:geo` está no `aranha` E no `teoria`). *Um extractor correto
   numa escala mente noutra* — ver [[feedback-a-definicao-do-extractor]]. Não
   publiquei o número antes de o verificar, e era falso.

E a excepção declarada precisa de controlo próprio: o `xr` resolve travessias
legítimas, e uma porta que aceita tudo é um buraco. Mede-se **desligando** —
as referências que ela cobre têm de voltar a ser órfãs — e com um nome de
prefixo certo mas ausente do alvo, que tem de continuar órfão.

**Aplicado na hora, e valeu logo:** trocar a lista escrita à mão pela derivada
(`.tex` com `\documentclass`, isto é, que compila sozinho) fez o medidor achar
**13** documentos e não os 12 que eu tinha contado — o `livro.tex` estava-me
invisível. A derivação vê o que quem escreve a lista não vê. E ela traz um
risco novo que precisa de gume próprio: se os alvos passarem do tecto do array,
os últimos saem *em silêncio* — o mesmo defeito com outra causa. Conta-se por
FORA do mecanismo e exige-se que todos tenham entrado.
