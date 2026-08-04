---
name: project-dualidade-memoria-da-divisao
description: "A definição do Aarão que generaliza as outras sete: a dualidade é a MEMÓRIA DA DIVISÃO — e o texto já a aplicava sem a enunciar"
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-04T02:53:47.763Z
---

03/08, fim do dia. O Aarão a ler a parte de análise: *"o texto ainda parece que busca explicações,
não está passando segurança pro leitor"*. Daí saiu uma definição, e ela é dele:

> **A dualidade é a memória da divisão.**

## PORQUE É EXATA, e não uma imagem

Dividir **perde**. Medido num universo de 169 elementos (ℤ[r], r²=r−1):

| guarda-se | imagens | distinções perdidas |
|---|---|---|
| só a simétrica | 37 | **132** |
| só a antissimétrica | 13 | 156 |
| **as duas** | **169** | **0** |

A dualidade é o que guarda a segunda — a memória de que as duas metades eram uma. É isso que
torna a divisão **reversível**, e por isso ela é uma tradução biunívoca: `ν∘ν = id`, *a volta é a
própria ida*.

## E O CRITÉRIO, que o texto JÁ APLICAVA sem o ter dito

**Se não há involução, não há memória — e sem memória não é dualidade, é degeneração.** Um agente
encontrou o texto a usar este critério duas vezes, por instinto:

- `catalogo.tex:6506` **recusa** o par `(D, ∫)`: `∫∘D = id − ker D`, e o núcleo é o que se perdeu.
- `catalogo.tex:2198` **exclui** a transformada de Legendre: *"não é isomorfismo… é limite"*.

Ou seja: a definição não foi imposta ao texto — foi **lida** dele.

## AS SETE FACES, todas medidas (`tests/pontofixo.c`, 28 asserções, resíduo 0)

algébrica (guarda a norma) · geométrica — cone dual, `K**=K` (a forma) · dimensional — Euler,
`V↔F` (a aresta) · lógica — `¬¬p=p`, De Morgan (o valor) · projetiva — ponto↔reta em PG(2,q) (a
incidência) · bidualidade — `R = Q×Q* = (N×N*)×(N×N*)*` (dois níveis, quatro componentes) ·
Gelfand — álgebra↔espectro (o produto).

**O cone é autodual só na métrica DELE**: com o produto euclidiano emprestado o dual sai maior
(111/91/71 contra 203/223/245). É o Δ como preço da régua errada, agora em geometria.

## O n=5 DEIXOU DE SER EXCEÇÃO

O texto dizia *"o fator ciclotómico ESTRAGA a matriz"* e *"é em n=5 que a propriedade CAI"*. O
Aarão: *"esse ponto é o começo, então como pode ser o fim?"* Tinha razão, e a conta é de duas
linhas: `r²=r−1` e `r⁶=1` dão `r^{n+1}=1`, logo **n ≡ 5 (mod 6)** — 5, 11, 17, 23, 29, 35. **O
passo da família é o mesmo 6 da ordem da raiz.** E |r|=1 quer dizer que a magnitude não se move:
**em p.u. ele já está**, e isso é ser ponto fixo. Pelo enunciado, *a fronteira é onde ν tem ponto
fixo* — logo é a borda a tocar a sua fronteira, e volta a tocá-la de seis em seis.

## O MAPA (os "pontos fixos do texto")

Três agentes varreram os 3 documentos. Cada dualidade clássica **já vivia lá**:
Pontryagin em §sec:pontryagin (e é *o eixo*), Poincaré em §sec:euler (a escada χ=2→1→0), Gelfand
na transformada, convolução/deconvolução, Fourier/Mellin, série/traços. A tabela do mapa entrou na
teoria.

**E o que falta, dito:** *Riesz* faz o trabalho todo (a forma-traço) e **o nome nunca aparece**;
*polos↔zeros* tem os dois lados e nunca é escrito como par involutivo.

**Defeito factual corrigido:** o catálogo dizia *"a teoria mencionava-o uma única vez"* sobre
Pontryagin — hoje são 12 menções e uma secção. A frase envelheceu com o texto.

Ligado a [[project-checkpoint-2026-08-03-fecho]].
