---
name: project-pisot-rouche-dual
description: A conjectura de Pisot caiu por Rouché no dual — e a lição foi o Aarão a impedir-me de usar um teorema clássico como carimbo de arquivamento
metadata: 
  node_type: memory
  type: project
  originSessionId: c1a01a83-ee3b-4229-965f-a0acb8476ee5
  modified: 2026-08-02T22:38:15.664Z
---

02/08/2026. A conjectura do paper `fracoes/fracoes-continuas.tex` — β(n,m) = xⁿ − m x^{n−1} − 1 é
polinómio de Pisot — deixou de ser conjectura. Prova por **Rouché**, não por indução em n.

**A prova dual é mais curta, e é o padrão a reter.** ν leva β no recíproco β* = xⁿ + mx − 1 e troca
dentro por fora, logo "β tem n−1 raízes internas" vira "β* tem UMA". Comparando β* com o termo
**linear** h = mx: sobre |x|=1, |β*−h| = |xⁿ−1| ≤ 2 contra |h| = m. Contar um zero simples em vez
de uma multiplicidade n−1 é toda a diferença. *O lado em que se faz a conta não é indiferente, e o
dual costuma ser o lado barato.* Ver [[project-transformada-universal]].

**A fronteira m≥2 aparece nos dois lados por contas distintas** — direto: min|x−m| = m−1 ≥ 1;
dual: máx|xⁿ−1| = 2 ≤ m. Duas medições diferentes excluindo a mesma linha m=1 é assinatura de
condição real, não de artefacto do método. E quando falha, falha *sobre a circunferência*: as
raízes sextas de Selmer em β(5,1) são os PONTOS FIXOS de ν.

**Graus 2 e 3 são teorema pela norma sozinha**: disc(β₃,ₘ) = −4m³−27 < 0 sempre → sobra um par
conjugado → módulos iguais → |λ| = σ^(−1/2) < 1. O número plástico é Pisot por essa razão, não por
exceção. A partir de n=4 a norma não chega: ela dá o produto, não cada fator.

## O que o Aarão me corrigiu, e é a lição

Eu ia escrever "a álgebra de Gentil não contradiz Hurwitz porque não é álgebra de divisão" — um
carimbo de arquivamento. Ele: *"hurwitz nao arrendou a matematica"*, *"classico é morto, no
cemiterio é que todo mundo é igual"*. A pergunta certa não é se contradiz; é **qual hipótese o
teorema escolheu**. Hurwitz exige três coisas: norma multiplicativa, **bilinearidade**, sem
divisores de zero. A de Gentil é homogénea de grau 1 mas **não aditiva** (medido: desvio 0,27 em
⟨u+v,w⟩ vs ⟨u,w⟩+⟨v,w⟩; a escala fecha a 2e−16). Larga só a cláusula (2) — está **fora** do
enunciado, não contra ele. Um teorema é um contrato: quem não assina não está preso.

Isto é a mesma armadilha de [[project-quantico-cosmico]] ("chamei lei à consequência de uma escolha
minha") e de [[feedback-verdadeiro-e-parcial]] ("que hipótese herdei sem escolher"). **Novo sinal
de alerta: quando me apanhar a usar um clássico para FECHAR uma pergunta em vez de a abrir,
enumerar as hipóteses do clássico e verificar quais o objeto novo assina.**

## A dualidade Fermat ↔ Pisot, e o limite honesto

N(a+b√D) = a² − Db² é UMA fórmula; o sinal de D decide tudo. D>0: indefinida, posto de Dirichlet 1,
unidade infinita → Pell, onde N vale 1, o **núcleo**. D<0: definida, unidades finitas → que valores
N toma, a **imagem**. Os casos degenerados coincidem: p=2 ramifica em Z[i] e as raízes de Selmer
estão no círculo — ponto fixo da involução, dito em duas linguagens.

**Não se estende ao Último Teorema** — testei e não fecha; são teoremas diferentes com o mesmo nome.
Recusei encenar "a prova de Wiles completa": são 109 páginas dos Annals mais Taylor–Wiles. O que é
verdade: a involução está lá com sinal, det ρ(c) = −1, e sai de χ(c) = −1 no caráter ciclotómico.

**E o dual de Wiles não é vazio — é Maass.** Lado ímpar → formas holomorfas; lado par → formas de
Maass (Δf = λf). A ponte é a fórmula do traço de Selberg, cujo lado geométrico são as geodésicas
fechadas de PSL₂(Z) = as frações contínuas periódicas deste paper. **O σ do paper É o autovalor da
classe hiperbólica**, e ℓ = 2 log σ o comprimento da geodésica. Pontryagin outra vez.

## A correção que me devo, sobre o eixo da indução

Escrevi "no FLT não satura" — errado no eixo. A indução no **expoente** não satura (Kummer: h_p
cresce, irregulares começam em 37, densidade conjeturada 1−e^(−1/2) ≈ 39%). A de **Taylor–Wiles**
satura: o posto do anel de deformação é limitado uniformemente ao longo da torre, e a compacidade
fecha — a mesma forma da meta-indução do paper. *A diferença entre Kummer e Wiles não é engenho: é
o eixo em que se induz.* Escolher o eixo onde a quantidade estabiliza é o passo que decide tudo.

## O medidor que falhou antes do teorema

§M17 do `matricial.c` reprovou na primeira corrida — e era **o medidor**, não o teorema. σ−m decai
como m^{−(n−1)} e some no epsilon do double (m=5, n=24 dá 1,8e−15), logo `σ > m` falhava por
precisão. Correção: testar o intervalo pelos **sinais** β(m) = −1 < 0 < β(m+1), exatos em inteiros
— que é o próprio passo (iii) da prova. Ver [[feedback-simulacao-nao-bate]]: antes da lógica,
verificar se as escalas fecham. `matricial.c` 38 → 41 asserções, e a (b) é a que **pode** falhar
(a linha m=1), como manda [[feedback-assercoes-vazias]].

Paper 16 → 20 páginas, 0 erros. Bateria 274/276 (as 2 falhas são as antigas). E o
`fracoes-continuas.pdf` saiu do git — era o único binário versionado, ver
[[project-publicacao-patria]].
