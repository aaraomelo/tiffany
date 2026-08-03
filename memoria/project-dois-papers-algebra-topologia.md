---
name: project-dois-papers-algebra-topologia
description: "O paper das frações partiu-se em dois pelo eixo álgebra/topologia, que é Pontryagin — e o que cada lado pode e não pode"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-03T00:42:26.575Z
---

02/08/2026. O material organizou-se em **três partes num só `teoria.tex`** (41 pp.), pelo eixo que o
Aarão deu: **álgebra, topologia e análise, duais entre si**. Ficam **três documentos** no repo:
`teoria.tex`, `catalogo.tex` (145 pp.) e `enredo.tex`.

- **Parte I --- Álgebra:** o que dá `+`, `×` e inverso. Dicionário
  matricial, `|det|=1`, companheira e `K(n,m)`, Pisot por Rouché, a norma, Fermat/Pisot, os traços,
  a zeta dinâmica `ζ = 1/β*`, as operações em base σ, o corpo métrico, e a secção de Pontryagin.
- **Parte II --- Topologia:** o que dá ordem, limite e vizinhança.
  Ordem alternada, encaixados, construção de ℝ, completude, Baire, geodésicas e Selberg, Hilbert e
  as áreas, as três codificações, a espiral e o cone.

**Os dois estavam trocados na primeira tentativa** — a construção de ℝ (topologia) estava no I e as
operações (álgebra) no II. Se voltar a mexer, é este o critério.

## O eixo é Pontryagin, e isso não é analogia

`Ĝ = Hom(G,S¹)`, regra **compacto ↔ discreto**, `G ≅ Ĝ̂` (que é `ν∘ν = id`), e **R̂ = R** — a reta é
autodual, o ponto fixo. A transformada é **uma**: a DFT em ℤ/8 sai da definição com resíduo
8,6e−16 contra a FFT. Pontryagin apresenta-se pela topologia; aqui começa-se pela álgebra, e dá no
mesmo porque a dualidade é simétrica.

**Subtileza que quase perdi:** é falso que "a álgebra é o lado discreto". **ℤ[φ] é denso em ℝ**
(menor intervalo 0,090 com coeficientes até 6, e tende a zero). Quem é discreto é a **imagem por
Minkowski** em ℝ² (distância mínima 1,414). *A dualidade não se vê na reta — é preciso subir a ℝⁿ.*
O mergulho de Minkowski não é ilustração: é o que torna o par observável.

## A tensão, medida (e é o resultado que a divisão serve)

- **A álgebra opera e não alcança a completude.** A base σ dá algoritmos **exatos**, sem vírgula
  flutuante, em `ℤ[σ]∩[0,∞)` — adição **linear** (transdutor finito), multiplicação **Θ(N²)** com o
  gargalo no *carry* e não na convolução — e estende-se a `ℚ(σ)` por expansões eventualmente
  periódicas, onde o **inverso é efetivo** (`1/x = x̄/N(x)`; períodos 3, 8, 16, 20 para 1/2, 1/3,
  1/7, 1/5). *A passagem a ℝ é completação, não algoritmo.*
- **A topologia alcança ℝ e não opera.** As letras da fração contínua dão ordem e completude e não
  dão fórmula para a soma.
- **O obstáculo é o mesmo com o sinal trocado:** o *carry* é não-local, a vizinhança exige
  localidade.

## Factos técnicos que custaram caro e não podem perder-se

- **O carry depende de m:** `σ^−(k−1) = m·σ^−k + σ^−(k+1)`. A forma sem o `m` é o caso φ e **só
  ele** — para m≥2 não conserva o valor (erro 0,414 em m=2). E a forma canónica "sem consecutivos
  nem repetições" é Parry **só para m=1**; para m≥2 é `d_k ≤ m` com `d_k=m ⟹ d_{k+1}=0`.
- **A terminação depende da estratégia:** *carry* no menor índice 1200/0; no maior, **1095 ciclos
  em 1200**. Faltava a regra dual (*unfold*).
- **`ℤ[σ,σ⁻¹] = ℤ[σ]`** porque σ é unidade (`σ⁻¹ = σ−m`) — é a condição de Pisot que faz o anel
  fechar.
- **Não é corpo:** coeficientes em ℕ, sem simétricos. É **semianel**.

## Pipeline

`publica.yml` atualizado em sete pontos para os dois papers (o novo era invisível e não sairia).
Pisos recalibrados ao estado real: `fracoes-continuas` de 6 → **15**, `realizacoes` **8**. Ver
[[project-publicacao-patria]] e [[feedback-o-disco-limpo]] — o portão simula-se localmente antes de
commitar.

Os erros deste dia estão em [[feedback-ceder-contra-a-medicao]],
[[feedback-normalizar-nao-e-medir]] e [[feedback-insinuacao-arquitetonica]].

## Parte III --- Análise (acrescentada depois)

Medida de Gauss e ergodicidade, Gauss–Kuzmin, Khinchin, Lévy, entropia `π²/(6log2)` e o câmbio de
Lochs (**uma letra vale 0,97 algarismos** — é isso que torna justa a comparação entre codificações),
a extensão natural, e o cálculo no toro (grad/div adjuntos, laplaciano diagonal no dual, Poisson
como divisão, soma de Poisson).

**Os pontos fixos de T(x) = {1/x} são os 1/σ_m — os metais — com multiplicador σ², logo
REPULSORES.** É isso que explica por que a análise genérica não os vê: são de medida nula porque a
dinâmica os repele.

E a equidistribuição testa-se **no dual**: o vetor `h` que a mata **é o polinómio mínimo**
(`|S_N| = 1` exatamente em `h = (1,−1)` para `(φ,φ²)`, e os `h` que matam formam um reticulado).
