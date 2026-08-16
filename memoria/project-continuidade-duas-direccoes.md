---
name: project-continuidade-duas-direccoes
description: A casa media SÓ uma direcção do supremo; a continuidade é a VOLTA, e constrói-se em inteiros
metadata:
  node_type: memory
  type: project
---

15–16/08/2026. O Aarão publicou a construção de ℝ e eu, na conversa seguinte, hesitei sobre a
completude — «é definição ou axioma». **Estava errado, e a casa já tinha a resposta.**

## A lacuna era de MEDIÇÃO, não de construção

Três sítios mediam a completude, e **os três mediam a MESMA direcção**:

> dado o habitante `x`, a classe racional dele cresce, fica abaixo, e ultrapassa todo racional
> menor — logo `x` é o supremo **da sua própria classe**

(`thm:central-continuo` §T4, `encaixotamento.js` §X3, `geometria_real.c` §L11e). Isso constrói o
**corte a partir do habitante**. A **continuidade** é a volta, e é ela que separa ℝ de ℚ como
estrutura ordenada:

> dado `S ≠ ∅` limitado superiormente, **existe** `sup S` **dentro do objecto**

E não é axioma aqui: `m_k = max{m : m/2^k < algum x ∈ S}`, e o que faz dele um habitante é o
**PASSO** — `m_{k+1} ∈ {2m_k, 2m_k+1}`, desce a árvore sem saltar. `tests/supremo.c` (4:0),
104 passos por **duas rotas concordantes** (o filho pela descida, o máximo pela definição).

## E o que separa de ℚ: a diagonal, FORA DA BORDA

`tests/cardinal.c` (4:0). ℚ enumera-se com o índice exibido; os caminhos não. **Mas a diagonal
ingénua tem buraco**, e era meu: `d[k] = 1−lista[k][k]` pode sair eventualmente constante, logo
diádica, logo com **dois caminhos** — e estar na lista pela outra representação. Apareceu em
**1 das 29 listas**. A que fecha gasta as duas paridades: `d[2k] = 1−L_k[2k]` difere na posição
`2k`, `d[2k+1]` alterna e tira-a da borda.

## O argumento dele, que é o certo

> «nós resolvemos qualquer problema aqui atravessando quantas dimensões quisermos com **erro 0,
> ida e volta**. Se não é contínuo, como explicas esse sistema?»

E fecha assim: **o habitante É o caminho**. Não há valor decimal por trás a ser aproximado, logo
não há erro a acumular — a ida e a volta são a involução, exacta por construção. A continuidade
clássica precisa do limite porque lá o ponto tem de *aparecer de fora*; aqui o ponto é a própria
sucessão de passos, e o supremo é mais um caminho que se desce.

**How to apply:** quando uma propriedade tem duas direcções (⟸ e ⟹), medir só uma e chamar-lhe
a propriedade é o defeito de [[feedback-verdadeiro-e-parcial]] com outro rosto. Perguntar: **esta
é a direcção que o teorema clássico afirma?** Ver [[project-a-reta-construida]] e
[[feedback-medir-so-metade-do-par]].
