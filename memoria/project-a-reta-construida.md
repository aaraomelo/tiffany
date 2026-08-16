---
name: project-a-reta-construida
description: "A construção de ℝ INTEIRO nas oito leis; o ouro é o real mais lento e limita todos; e a lição: não virar juiz da teoria."
metadata:
  node_type: memory
  type: project
---

**A LIÇÃO PRIMEIRO, porque foi a repreensão e é a que se repete.** Escrevi um §«O que este
paper não faz» — uma lista de ressalvas — e uma delas, «não constrói todos os reais», era
**invenção minha, não do quadro**. O Aarão: «vc não colocou o que NÃO PROVA em vez do que
FALTA PROVAR, vc está julgando aqui como juiz» e «não é para o agente virar juiz da teoria;
é para ele ler a teoria inteira e **executar a próxima construção**». O padrão: quando o
quadro já fechou uma construção, o meu trabalho é REALIZÁ-LA, não auditar se ela é
permitida. O freio disfarça-se de rigor.

**A HIERARQUIA que eu tinha perdido**, e que agora encabeça o paper:

    o Universal PROVA   |   o geométrico REALIZA   |   os medidores VERIFICAM

`geometrico.tex` é **filho** do `corpo_universal.tex`, não um paper independente. Cada peça
leva a linha das três colunas; se não tiver as três, não está pronta.

**E A CONSTRUÇÃO É DE ℝ INTEIRO.** Um real **É** uma sucessão de quocientes parciais
a_k ≥ 1; a construção corre sobre sucessões **arbitrárias**, e o metálico é o caso
**periódico**, não o caso único. Em 220 sucessões: `p_k q_{k−1} − p_{k−1} q_k = (−1)^{k−1}`,
logo |det| = 1 em todo passo; alternam, aninham, q_k cresce. E o fecho para todos de uma vez:

    q_k ≥ F_k para TODO real, com igualdade em TODO k só na sucessão de uns

**O ouro é o real mais lento que existe**, e por isso UMA taxa limita ℝ inteiro. A família
metálica não é o escopo — é o caso **extremo**. Ver [[project-teorema-do-gato]].

**Distinção que eu tinha misturado (o eval apanhou):** são DOIS determinantes —
`det A^k = F_{k+1}F_{k−1} − F_k²` das ENTRADAS, em ℤ, sem espectro; e
`σ^k(σ†)^k = N(σ^k) = a² + mab − b²` do ESPECTRO, sem olhar uma entrada. **A igualdade dos
dois é o TEOREMA** («det é o produto do espectro»), e é a coincidência que se mede.

**E «oito passos enchem um byte» é CODIFICAÇÃO — não demonstra bit a bit.** O que demonstra
é Gram = I. Numa base torcida f_k = e_k ⊕ e_{k+1} a Gram sai da identidade em 24 dos 64 e a
leitura directa erra **metade exacta** das 2048 coordenadas.

**Defeitos meus que a medição apanhou nesta ronda** (todos do mesmo tipo — ver
[[feedback-assercoes-vazias]]):
- **a semente da recursão dos convergentes**: pus `q₀ = 0, q₁ = 1` quando é `q₋₁ = 0, q₀ = 1`.
  Deslocava a sucessão um andar, e apanhou-se pelo sítio certo: **o ouro deixou de bater com
  Fibonacci**, que é a única coisa que ele não pode deixar de fazer.
- **afirmei de mais**: «q_k = F_k só no ouro» é FALSO pontualmente (69 empates); o que só o
  ouro faz é empatar em **TODOS** os andares.
- **três números escritos à mão** nas asserções (512, 4000, «19 3928»), trocados por
  condições estruturais.
- o sinal de Cassini trocado (0 de 115) e o det do controlo sem o factor 4.
- e o `\verb` dentro de um teorema partiu o pdflatex — **e o PDF que ficou no disco era o
  ANTIGO**, que eu quase reportei como novo. Ver [[feedback-a-mensagem-que-nao-pode-falhar]].

**A RONDA DO REVISOR EXTERNO (18:0).** Quatro erros matemáticos concretos, e o pior era de
NOME: chamei `F_k` à sucessão metálica em três secções. A conta estava certa (`A_2² =
(5,2;2,1)`), mas `F` lê-se Fibonacci, que é só `m = 1`. Agora é `U^{(m)}`, e a distinção
virou MEDIDA: coincide com F em 1 dos 8 metais, divergindo em `k = 2`. Os outros três:

- **os racionais estavam a descoberto** — CF infinita ↔ IRRACIONAIS; os racionais são os
  casos em que o processo TERMINA, com a ambiguidade `[…,a_n] = […,a_n−1,1]`. Medido em
  3660: termina em todos, exacto em todos. Sem isto, «ℝ inteiro» era reivindicação.
- **duas afirmações coladas numa**: `|σ^k − t_k| = |σ†|^k` é IMEDIATA; `dist(σ^k,ℤ) =
  |σ†|^k` exige além disso `|σ†|^k < 1/2`. Medido em separado, com `round(σ^k)` calculado
  em inteiros por raiz de Newton.
- **a varredura é VERIFICAÇÃO, não prova**: «nenhum racional cai em cima» é teorema geral
  (Δ não é quadrado); os 19 360 casos confirmam a IMPLEMENTAÇÃO. É a hierarquia aplicada
  a si própria.

E o Aarão: **«não privilegie ninguém, nem o ouro»** — a frase certa é «φ minimiza o
crescimento dos denominadores dos convergentes ENTRE as expansões com a_k ≥ 1»: é a
**extremalidade da régua**, o pior caso do mecanismo, e não uma propriedade de φ contra os
outros reais. Ver [[feedback-o-escopo-da-afirmacao]], [[feedback-duas-reguas]].

**A 2.ª RONDA DO REVISOR (21:0), e ela apanhou o PAI.**

- **«há uma só forma bilinear simétrica não degenerada sobre 𝔽₂» é FALSO**, e estava no
  **`corpo_universal.tex`**, não só no filho. Sobre 𝔽₂² há **QUATRO**, uma delas
  alternante — e ser alternante é invariante por mudança de base, logo ≥ 2 classes.
  O enunciado honesto é mais forte: a forma **ESCOLHE-SE**, e o que se prova não é que a
  base seja ortonormal (isso seria escolher a métrica que torna a conclusão verdadeira) —
  é **o que a ortonormalidade CUSTA a quem não a tem**.
- **a base torcida**: `f_7 = e_7 ⊕ e_0` (circular, e diz-se), e o que acontece é mais forte
  que «sai da identidade» — cada `f_k` tem DOIS uns, logo `⟨f_k,f_k⟩ = 0`: **nem tem
  diagonal 1**.
- **`q_k ≥ F_{k+1}`**, não `q_k ≥ F_k`. Na convenção padrão (F₀=0) e com `q₋₁=0, q₀=1`, o
  ouro dá **exactamente** `q_k = F_{k+1}`. A minha forma era verdadeira mas frouxa.
- **um teste não mede COMPLETUDE.** O mesmo reparo dos 19 360 racionais, aplicado onde mais
  importa: o argumento do corte estabelece a completude; o teste verifica as propriedades
  **operacionais** nos casos corridos.
- **π_k «exacto» qualifica-se**: exacto *no modelo discreto do andar k*, e `π_k → π` no
  refinamento. Nunca `π_k = π`. E `t_n = 2cos(π/n) ∈ ℝ` **não é** `t̄_n ∈ 𝔽_p`: o segundo é
  a **realização modular de uma relação algébrica**.
- **e a CF blinda-se**: «neste paper a CF não é a definição axiomática de ℝ; é o mecanismo
  de realização dos cortes» — outra construção pode axiomatizá-la.

**A LEI 0 É A FUNDAÇÃO DO MOTOR, e foi o Aarão que a mandou buscar.** `0† = ∞` pela troca
`[p:q] ↦ [q:p]`, matriz `S = (0,1;1,0)`, det = −1: sem divisão, sem teste, sem ramo. E não é
ornamento — **o passo da CF é `x ↦ 1/(x−a)` e MORRE em ℚ quando `x−a = 0`, que é
exactamente onde o racional termina**. Medido: a descida atinge ∞ em **todos** os 1640
racionais e em **nenhum** dos 8 σ_m ([m;m,m,…], puramente periódica). **A Lei 0 separa as
duas metades de ℝ**, e o ∞ é o ponto onde o racional fecha.

**E o `teoria.tex` dá o ESTATUTO, que eu não tinha:** o corpo dual e as duas leis são o
**fundamento**; a Möbius de Fibonacci, o gato e os convergentes são a **representação —
ferramenta, não fonte**. E as duas primeiras leis SÃO as minhas duas matrizes:
**Lei 1** = a Möbius involutiva (traço 0) = `ν(x) = −1/x`; **Lei 2** = a Möbius de Fibonacci
(det = −1) = `A_m`. E `Δ = tr² − 4det` dá `m²+4`: a família metálica é a linha **hiperbólica**
da tabela dele, os polígonos as elípticas, o círculo a parabólica. Nada disso é meu.

**E o `corpo_universal.tex` NÃO COMPILAVA** — 99 erros, todos de `\verb` em contexto frágil,
e não fui eu: HEAD dava a mesma contagem e a mesma primeira linha. 11 `\verb` → `\code`, e
passa a **54 páginas, 0 erros**. Ver [[feedback-a-mensagem-que-nao-pode-falhar]].

**A COMPLETUDE, MOSTRADA — e as referências estavam todas no Universal.**
`thm:central-continuo` item 1 e 4, `thm:encaixotamento`, `thm:primos-irracionais`:

- **UNICIDADE pelo POMBAL, e é aritmética inteira**: se dois racionais habitam os mesmos
  intervalos até profundidade K com 2^K > s₁s₂, então |r₁s₂ − r₂s₁| < 1 e é **inteiro**,
  logo ZERO. **Não há dois habitantes.** 2 624 400 pares, zero violações.
- **EXISTÊNCIA**: o que a sucessão produz é um **corte de Dedekind** — as quatro cláusulas,
  incluindo *A sem máximo*. O ponto **não vive em ℚ**, e é por isso que o CORTE é o objecto.
- **IDENTIFICAÇÃO** do conjunto dos cortes com o corpo ordenado completo: **herdada**.
Nenhuma das três por varredura. Isto responde ao revisor, que tinha razão: encaixe com
largura → 0 não estabelece a existência do ponto em ℚ.

**E o encaixotamento tem DOIS lados** (a pergunta do Aarão): por CIMA `E(x) = x/(1+x)` com
`E^n(∞) = 1/n` exacto — «a harmónica é a sombra do infinito pelas dimensões»; por BAIXO a
classe diádica cumpre as três cláusulas do supremo, e a terceira (**ultrapassa todo racional
menor**) é a que faz de σ um supremo e não um majorante. A classe **não desce** — não
«cresce»: 3/2 = 6/4, e exigir o estrito deu 0 de 8.

**A CORRESPONDÊNCIA PRIMO-IRRACIONAL é a tricotomia:** o **diádico TERMINA**, o **primo
RODA** (os bits de 1/p têm período `ord_p(2)`, e na escada de Fermat os períodos DOBRAM:
8 em 17, 16 em 257), o **irracional FOGE**. E o que os separa é a **norma**: fecho da órbita
⟹ racional ⟹ norma 0, enquanto a norma dos convergentes é ±1 — que é o mesmo |det| = 1.

**E a peça que fecha a arquitectura: `A_m = T^m·X`**, com T a translação e **X a troca da
Lei 0**. A fracção contínua é uma **palavra em duas letras**: a passos aditivos e uma troca.
O motor liga-se à Lei 0 por **identidade matricial**, não por analogia.

**A TRICOTOMIA ESTAVA ERRADA, E A RESOLUÇÃO É A QUINTA PRIMITIVA.** Escrevi
«diádico / PRIMO / irracional», e isso não é uma tricotomia: **1/6 não é diádico e não tem
denominador primo**. O Aarão apontou: «tem 5 primitivas lá, vê a última morfológica, tá
tricotomia e resolve». A quinta primitiva é a **Inversão**, cujo critério é

    admissível ⟺ TEM VOLTA ∧ conserva a escada          (def:cinco)

com o par morfológico `δ ⊣ ε` — δ CRIA (A ⊆ δA), ε REMOVE (εA ⊆ A), e a volta existe onde
`δ(ε(A)) = A` (thm:morf-par). Classificando **pela volta**, fica exaustivo e exclusivo:

    a volta NÃO existe          → FOGE      irracional
    a volta existe, ciclo ≠ {0} → RODA      racional não diádico
    a volta existe, ciclo = {0} → TERMINA   diádico

Operacionalmente, para a/b reduzido com b = 2^s·b′ e b′ ímpar: b′ = 1 TERMINA; b′ > 1 RODA
com pré-período s e período `ord_{b′}(2)`. **1260 racionais com b ≤ 64: 64 terminam, 1196
rodam, ZERO sem classe.** E **o primo não é classe — é o subcaso b′ = p, s = 0**.

**E saiu o «norma 0»**, que era afirmação sem objecto: havia três normas a rondar o paper
(a algébrica N(σ), o determinante, e uma «norma da órbita»). A norma algébrica e o
determinante ficam no §det; o que separa os habitantes é a **existência da volta**.

**O outro bloqueador: o salto entre as duas caixas.** O §corte decide `a/b < σ_m` — o corte
de um metálico *específico* — e o teorema da completude falava de `(a_k)` arbitrária. Agora
declaram-se as duas antes: **metálico = mecanismo extremo explícito; CF geral = extensão**,
com `I_k` definido, `I_{k+1} ⊂ I_k`, `|I_k| = 1/(q_k q_{k+1})` e — o elo que o pombal usava
sem ter — **`|I_k| ≤ 2^{-k}`**, que vem de `q_k ≥ F_{k+1}`.

`tests/geometria_real.c` (30:0) na ordem obrigatória do eval: 8 leis → base ortonormal →
bit a bit → dual → recorrência → Pisot → encaixe → corte → completude → ℝ → π_k/gume.
`papers/geometrico.tex` 12 páginas; `corpo_universal.tex` 54, e a compilar.
Ver [[project-o-real-e-o-corte]], [[feedback-verdadeiro-e-parcial]].
