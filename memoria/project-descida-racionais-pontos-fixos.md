---
name: project-descida-racionais-pontos-fixos
description: A DESCIDA — ℚ são as classes de equivalência dos pontos fixos, o teorema dos resíduos dá-os directo, e a aritmética aditiva é a Möbius com um S no meio.
metadata:
  type: project
---

# A descida: reais → racionais, e ela estava toda escrita

17/08/2026, commits `2f0b354` → `fa7fb5e`. O `geometrico.tex` já subia — a órbita corre em
ℙ¹(ℚ) e o ponto fixo **não está lá** (`thm:corte-fixo`). Faltava a outra metade do par, e ela
não pedia construção nova: pedia ler o que o `thm:fixo-dual` já dizia.

## A construção (thm:descida)

Cada σ_m vive no **seu próprio corpo** ℚ(√(m²+4)) — φ em ℚ(√5), 1+√2 em ℚ(√8), um corpo por
cada m. Não há sítio onde caibam todos, **excepto o que a simetrização deixa**:

    σ + σ† = m      ← o TRAÇO: projecta, e é SOBREJECTIVO sobre ℤ
    σ · σ† = −1     ← a NORMA: constante, e não separa NADA

Logo **ℤ ≅ {pontos fixos}/∼ pelo traço**, a fibra sobre m é o par dual {σ_m, σ_m†} — *a classe
de equivalência* — e ℚ é a razão: `a/b = tr(σ_a)/tr(σ_b)`, com `ad = bc` **recuperado e não
posto**. E o racional é o que a DOBRA fixa: `x ∈ ℚ ⟺ x = x†`, o auto-espaço +1 (Dir alcança,
Cruz opera — Pontryagin literal).

**O gume está dentro da construção**, e é o que a torna medida e não definição: se a coordenada
escolhida fosse a norma, a fibra era a família inteira. Medido lado a lado — o traço separa 60
de 60, a norma separa 1; em 20 736 quádruplos o traço acerta todos e a norma acerta 432. É
[[feedback-normalizar-nao-e-medir]] a aparecer como escolha de coordenada.

## O teorema dos resíduos JÁ os dá (thm:residuo-fixo)

O denominador da zeta **factoriza nos pontos fixos**, e sai do traço e do determinante sem
formar raiz: `1 − mx − x² = (1−σx)(1−σ†x)`. Logo os polos SÃO os pontos fixos (nos recíprocos),
a decomposição dá uma parcela POR polo, e extrair o coeficiente de x^k é somar sobre eles — o
que sai é σ^k + σ†^k, **o traço**. O círculo fecha:

    extrair coeficiente = somar sobre os pontos fixos = o traço = ℚ

E **um resíduo sozinho não é racional**: σ^k tem parte √D não nula em todo k ≥ 1, e são os DOIS
que dão o inteiro. [[feedback-dual-exige-dois]] como conta.

## A aritmética aditiva (thm:aditiva)

`A_a · A_0 = T_a` — o passo da FC composto com a INVERSÃO dá a TRANSLAÇÃO — logo

    A_a · A_0 · A_b = A_{a+b}

**a soma realiza-se dentro do grupo multiplicativo, com S = A_0 no meio**: o eixo 0 ↔ ∞ a fazer
de logaritmo. Na palavra é a mesma frase: **um quociente ZERO soma os vizinhos**,
`[… a, 0, b …] = [… a+b …]`.

E o «∞ + 1 = −1» do Aarão **fecha nos índices**, com x_k := A_m^k(∞):

    x_0 = ∞ (o NEUTRO) · x_-1 = 0 (o 0 = 1/∞) · x_-2 = −1/m = norma/traço

— *o elemento que fecha é a razão dos dois invariantes do par dual*, o que conserva a dividir o
que separa. Em m=1 vale −1: **a relação dele é o caso do ouro**. Na volta ao quadrado fica um
passo só, e o anel muda: det de −1 para +1 (a volta passa a PRÓPRIA) e disc de D para m²·D,
isto é ℤ[m√D].

## E a descida via Möbius É Euclides

A casa tinha as duas metades em colunas diferentes sem nunca dizer que eram a mesma:
`rt_cf_de` escrita como mdc, `rt_dobra` escrita como Möbius — e `p − a·q` **é** `p mod q`
quando `a = ⌊p/q⌋`. O algoritmo da fracção contínua é iterar g⁻¹, e por isso a palavra sai sem
uma divisão em vírgula. Ver [[project-a-reta-construida]] e [[feedback-a-base-ja-existe]].

**Medidores:** `tests/racionais_fixos.c` (8:0), `tests/descida_mobius.c` (10:0), e o
`tests/continua.c` §C2 com a identidade `−log(1−mx−x²) = Σ t_k x^k/k` provada **coeficiente a
coeficiente em ℚ** em vez de avaliada em 15 pontos com duas réguas.

## A CONSERVAÇÃO (thm:conservacao) — e é a dobra

«Falta algo ainda, a conservação (energia), porque a condição de paragem é a energia que o nível
comporta.» Faltava, e a ponte já estava escrita em dois sítios que não se conheciam: o
`corpo_universal` (def:inducao) diz que **o que a meta-indução VALIDA é a conservação de energia**,
e o thm:operador diz que a meta-indução **é a dobra**. Logo **A DOBRA É A LEI DE CONSERVAÇÃO**.

Na descida vê-se em duas quantidades que fazem coisas opostas:

    CONSERVA-SE   mdc(p,q)   a ENERGIA      mdc(p,q) = mdc(q, p−aq), exacto
    GASTA-SE      q          o ORÇAMENTO    decresce em ℕ

A paragem é `q = 0`, e nesse instante **o que sobra em p é o mdc inicial**: o nível comporta uma
energia finita, o passo gasta-a, e quando acaba a descida devolve-a. E nada se perde porque
|det| = 1 — **conservar e poder voltar são a mesma frase**. A descida infinita é impossível pelo
PAR: o mdc não desaparece e q não decresce para sempre. 6400 descidas, tudo verde.

## O que FUNDAMENTA: o teorema central Gentil–Hurwitz–Lebesgue

Já estava no repo (`corpo_universal` thm:parseval-multi, `cor:verifica-energia`):

- **HURWITZ CONTA** — `Σ_k ω^{(i−j)k} = N·δ_ij`, os cruzados cancelam-se EXACTAMENTE. É o corte
  discreto, e é daqui que Parseval sai. Medido exacto em 𝔽ₚ (32/32 e 224/224), **com o controlo
  que o paper exige: raiz de ordem ERRADA quebra**.
- **GENTIL INTEGRA** — `∫|f|² = ∫|F|²` com Lebesgue.
- **LEBESGUE CASA-OS**, e o que preserva é a ORDEM (σ-aditividade). *Reversível* é a mesma palavra
  do thm:conservacao.

**E o tecto de 8 é da NORMA, não do objecto**: Hurwitz limita a {1,2,4,8} com norma EUCLIDIANA
**e** bilinearidade. A recta não paga porque a energia que conserva é o **determinante**,
multiplicativo em todo n — e o `nne.c` mede o contraste: determinante 6561/6561, euclidiana 289.
Ver [[project-torre-hurwitz-gentil]].

## E os sete teoremas são uma frase: o traço MEDE, o determinante CONSERVA

    tr    separa, coordena, constrói — muda de andar para andar
    det   invariante, e é por isso que há volta

É a divisão do thm:fixo-dual (traço e determinante, ambos inteiros) lida em cada um dos sete.

## Cantor/Julia: o mesmo par dual, noutra carta (thm:cantor-julia)

O `corpo_peano` (def:cantor) já tinha **cantor = φ, julia = ψ, φψ = −1** — que é
`σσ† = −1` do thm:fixo-dual com m=1. **Não é analogia: é o mesmo par.** E o shift θ↦2θ
(aditivo, Cantor) conjugado a z↦z² (multiplicativo, Julia) pela exponencial é o
[[project-descida-racionais-pontos-fixos]] thm:aditiva com **outro conjugador** — lá a
exponencial, aqui a inversão S.

**E o que as separa é a conservação, que é o resultado do dia:**

    Cantor   r ↦ b·r mod q      NÃO gasta q  →  CICLA
    Möbius   (p,q) ↦ (q,p−aq)   gasta q      →  TERMINA

Com q ímpar o 2 é invertível, o shift não tem o que gastar, e fecha em ciclo. **E o
contraste prova que não é do operador**: com q PAR o shift ENCOLHE o denominador (590/590),
e só deixa de encolher ao chegar ao ímpar. *Ciclar não é uma propriedade do operador: é o
que sobra quando o orçamento deixa de poder ser gasto.*

## A UNIFICAÇÃO (thm:unificacao): a ORDEM identifica a lei

«Formaliza a unificação com a lei 8 e trial, Cantor, Julia, Viviani; se fechar, faz teorema.»
Fechou — e a estrutura **já estava na tabela das oito leis do `corpo_peano`**, com o FECHO de cada
uma escrito ao lado. O que faltava era ver que as peças SÃO essas leis:

    Cantor / Julia   o espelho da dobra          2    L_1/L_2   ν² = id
    o trial          τ em {−1, 0, +1}            3    L_3       τ³ = id
    VIVIANI          i, o meio-ângulo            4    L_5       i⁴ = id
    a Lei 8          Ind, o índice do catálogo   8    L_7       Ind⁸ = id

**E o quadro fecha em 24 = 8 × 3** — a Lei 8 (que é 2³, três dobras) vezes o trial. *O 24 não é um
número que aparece: é o produto das duas coisas que o catálogo tem, oito índices e três símbolos.*
E o HEXAL sai das ordens medidas: o menor k onde dual e trial voltam juntos, 6 = lcm(2,3).

**Viviani é a de ordem 4 porque i² É o espelho** — uma volta no ângulo devolve o ponto com o sinal
trocado, e ord(i) = 2·ord(ν). O meio-ângulo é a raiz quadrada da rotação, e a ordem di-lo.

**As ordens MEDEM-SE e a minimalidade é parte da tese**: τ⁶ = id também é verdade e não faz do
trial uma lei de ordem 6. E o gume está dentro — trocar a ordem de uma peça pela de outra quebra o
24 em 6 das 12 trocas.

Medidor: `tests/lei8_trial.c` (5:0, zero sobreviventes em 80 mutações do gume).

## O CICLO UNIVERSAL (thm:universal) — e o toro é a CONDIÇÃO

    DESCODIFICA   (p,q) := o texto, racional EXACTO ; u := mmc dos denominadores
    OPERA         exige |det T| = 1 ;  [p:q] -> T·[p:q], sem dividir
    INVERTE       T⁻¹ = adj(T)/det T, INTEIRA porque det = ±1
    CODIFICA      w := palavra (FC) ou dígitos (Cantor)
    E FECHA       descodifica(w) == (p,q)

**O toro não é mais um operador na lista: é a CONDIÇÃO.** Sem |det| = 1 a inversa não é
inteira e `rt_inverte` RECUSA (2169/2169). É o thm:conservacao dito como algoritmo.

**Viviani tem DUAS ordens, e foi a asserção que mo mostrou**: medi só em ℙ¹ e deu 2 em vez
de 4 — em ℙ¹ os vectores v e −v são o MESMO PONTO, logo i² = −1 já é a identidade
projectiva. A ordem 4 vive no VECTOR. *É isso o recobrimento duplo.* E o gato não fecha em
nenhuma das duas.

**A codificação é LIVRE** (palavra, base 2, base 10 dão o mesmo racional): a escrita é uma
escolha, não é do objecto. É isso que «universal» quer dizer.

Promovido à lib: `RtOp`, `rt_opera`, `rt_inverte`, `rt_ordem_ponto/vector`, `rt_ciclo` —
e **a TESE fica de FORA da peça** (a comparação com o original é de quem chama, senão ela
media a própria definição). Ver [[project-a-reta-construida]].

