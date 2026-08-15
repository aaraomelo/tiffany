---
name: project-mobius-e-elipticas
description: Möbius como inversor (μ = 1⁻¹ na convolução de Dirichlet) e as curvas elípticas, onde a fibra escolhe qual das três operações corre.
metadata:
  type: project
---

Dois pacotes do `eval.txt` que se tocam no mesmo sítio — a **INVERSÃO**. Commit `3073941`,
`lib/dirichlet.h` + §C33 e `lib/eliptica.h` + §C34. Zero doubles nas duas.

## μ = 1⁻¹ (Dirichlet)

A convolução `(f*g)(n) = Σ_{d|n} f(d)g(n/d)` é a multiplicação sobre a **árvore dos
divisores**, e nela **μ é o inverso da função constante 1**. É o que torna a ideia de
inversão desta casa matemática limpa: o μ deixa de ser uma tabela de sinais e passa a ser
o que **desfaz a soma sobre os divisores**. Medido nos dois sentidos e com a UNICIDADE —
construindo o inverso pela recorrência que a definição obriga, sai exatamente o μ.

`φ = id * μ` (e o dual `φ * 1 = id`); `τ = 1*1` e `σ = id*1` caem de graça. A inversão de
Möbius **é a deconvolução**, e ele diz isso mesmo.

**A decisão que evitou os floats**: ele escreve `f: ℕ → ℂ`, mas as funções do andar são
todas inteiras. E a **série de Dirichlet mede-se FORMALMENTE** — `D_{f*g} = D_f·D_g` é uma
identidade sobre os **coeficientes**, e o `s` nunca se avalia. Avaliar pedia análise e
decimais; os coeficientes são inteiros. Ver [[feedback-inteiro-primeiro]] e
[[feedback-representacao-inteligente]]: escolher a representação em que a pergunta é exata.

## A fibra escolhe a operação (elípticas)

«A fibra determina qual operação existe» — a frase é dele e já é a língua da casa. Três
ramos: opostos (𝒪), tangente, secante. **Mediram-se um a um** (18/18/288), porque um ramo
que não corre não está medido — é [[feedback-o-ramo-que-nunca-corre]].

Sobre 𝔽ₚ a divisão é a **inversão modular** — «o andar dos racionais voltando» — e a
**redução** liga os dois mundos: a órbita de ℚ com frações exatas, reduzida mod 11, dá a
órbita de 𝔽₁₁. Duas implementações independentes obrigadas a concordar.

A **associatividade** varre os 6859 triplos do grupo inteiro (ele avisa: «não basta testar
alguns pontos e declarar»). E **Viète** mede-se em vez de se citar: `x₁+x₂+x₃ = m²` em 288
secantes, o que transforma a fórmula de receita em consequência.

## Os dois defeitos, e o que ensinam

1. **O Lagrange ia ser medido numa curva de #E = 19, primo** — «todas as ordens dividem N»
   seria quase automático, caso degenerado a passar por teorema. Troquei para #E = 24 com
   sete ordens distintas. Gatilho novo: **quando o teorema é sobre divisibilidade, escolher
   o caso onde o divisor tem divisores.**
2. **A testemunha por estrear**: `dl_multiplicativa` saía no teste `f(1)=1` sem preencher o
   par, e a linha imprimia memória (`f(0·0) = 140720728132432`) com a asserção verde. Mesma
   classe do [[feedback-o-destino-rotativo]]: valor errado no TEXTO, invisível às
   asserções. Regra: **quem devolve testemunha escreve-a em TODOS os caminhos de saída.**

E o guarda dos opostos em `ef_soma` é **redundante** (medido) — segunda ocorrência do mesmo
achado depois do Teorema Chinês, e por isso [[feedback-o-ramo-que-nunca-corre]] ganhou
confirmação em vez de exceção.
