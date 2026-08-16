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

**A PURGA DOS METÁLICOS, E EULER É QUE A FAZ.** O Aarão: «traz a lei de Euler dos poliedros
para ver se resolve isso dos metais — **isso é realização específica**; traz a justificativa no
cone e espiral, e expurga os metálicos». Resolve, e a conta é de três linhas:

    χ(polígono preenchido) = n − n + 1 = 1        para TODO n
    χ(só a borda)          = n − n     = 0        para TODO n

**A topologia NÃO VÊ O ANDAR.** Quem vê o andar é a MÉTRICA — t_n, as áreas. Logo a família
metálica **não é ingrediente da construção: é uma realização específica na camada métrica**.
E o invariante topológico tem a MESMA FORMA do métrico: `χ = V − A + F` é soma ALTERNADA que
o dual do poliedro não move (troca V e F, guarda A — já estava em `pontofixo.c` §3), tal como
`det = (−1)^k` é sinal alternado que o passo não move em módulo.

**O MOTOR, sem metálico nenhum, já estava no repo** (`lib/medida.h`, `tests/cone_espiral.c`):
`Π` o CONE desce por Euclides escrevendo os quocientes, `Σ` a ESPIRAL sobe recompondo, e cada
passo do cone é `[[a,1],[1,0]]` com `det = −1`. **`Σ∘Π = Id` mas `Π∘Σ ≠ Id`** — e isso separa
dois duais que eu tratava como um: **INVOLUÇÃO** (ν∘ν = id, mesmo espaço) contra **RETRAÇÃO**
(espaços diferentes, só um lado fecha). O preço de Π∘Σ não fechar são as duas representações
de todo racional. **E o metálico é o PONTO FIXO do cone** — a sucessão constante: é por isso
que dá o mecanismo extremo, não por ser o caso geral.

**E O TEOREMA CENTRAL FUNDA O LIMITE** (obs:triade-central): **Hurwitz conta o domínio,
Lebesgue mede a imagem, Gentil casa os dois** pela soma reversível
`Σx_n + Σ_v #{x_n < v} = N·q` — o `∫f + ∫f⁻¹ = b·f(b) − a·f(a)` da casa. E o layer-cake fecha
**sem esperar o limite**. Donde o estatuto certo: **o limite é o PONTO FIXO onde as três medem
igual, e não um processo ε-δ**. A tricotomia distribui-se: TERMINA e RODA são o lado contável
que Hurwitz conta (e a quota deles nos 2^K intervalos DECRESCE — a borda é magra), FOGE é o
que Lebesgue mede. E o par cone/espiral tem a mesma forma da soma reversível de Gentil.

Defeito meu na ronda: medi «a borda é magra» por nós contra folhas, e essa fracção **cresce**
para ½. O que emagrece é a QUOTA DO LADO CONTÁVEL: 22 racionais fixos ocupando 22 de 64 e
depois 22 de 4096.

**A CLASSIFICAÇÃO ERA MINHA E ESTAVA CONFUSA — a certa é LAGRANGE, POR NÍVEL.** Eu classificava
a expansão BINÁRIA («diádico / primo / irracional»), misturando dois níveis. O Aarão: «racional
é sequência periódica, PA ou PG; irracional o período é 1 no nível e continua no nível acima
como racional; usa PA e PG de ordem m». A classificação lê-se na FRACÇÃO CONTÍNUA:

    CF FINITA                        → RACIONAL
    CF eventualmente PERIÓDICA (p)   → IRRACIONAL QUADRÁTICO      (Lagrange)
         · p = 1 é o METÁLICO
    CF NÃO periódica                 → irracional NÃO quadrático

E o elo entre os níveis: **o nível acima — os convergentes — continua RACIONAL**, obedecendo a
uma **PG de ordem 2 nos blocos de p**:

    u_{k+p} = T·u_k − det(M)·u_{k−p}        T = traço da matriz do bloco

**O período é a ORDEM DO SALTO; o traço do bloco é o coeficiente.** Medido em seis famílias
(p = 1, 2, 4). E **PA separa-se de PG**: quocientes em progressão aritmética de razão d ≠ 0 não
são periódicos, logo não são quadráticos; **a PA de razão 0 É a PG de período 1** — o metálico.
A periodicidade não é propriedade do número: é da sucessão de quocientes.

**E π_n GANHOU DEFINIÇÃO** — era o único ponto vermelho do revisor. Não é «fecho geométrico do
andar», que descreve: é

    π_n := [A^in_n, A^circ_n],   π_{2n} ⊂ π_n,   |π_n| → 0

um **INTERVALO**, com as duas bordas exactas no andar, formando uma cadeia encaixada. **Donde π
é o CORTE que a cadeia determina** — pelo mesmo mecanismo do resto do paper. O que é exacto por
andar são as BORDAS; π continua a ser o corte.

E dez amarelos: o metálico é ponto fixo da **Möbius associada** (não «do cone»); q_k é **não
decrescente** com estrito a partir de k=1; o teorema da completude fala de CF **infinita** e A
é **definido** por extensão; o **pombal dá UNICIDADE, não existência** — o mecanismo entrega o
corte e o Universal identifica-o; a identidade de **Gentil é herdada**, não provada aqui; a
quota decrescente declara a **discretização**; a contagem das oito leis fixa-se em (0,…,7); e as
leis são **realizadas por** vectores, não iguais a eles.

**EULER PASSA PELO CENTRO — É A PASSAGEM.** O Aarão: «sobre a contagem, é justamente Euler
que passa pelo centro, é a passagem». A operação diz-se célula a célula: acrescenta-se o
ápice no centro e liga-se a tudo — `V→V+1, A→A+V, F→F+A`, com F células novas — donde

    χ = (V+1) − (A+V) + (F+A) − F = 1,   seja qual for o poliedro

e um andar abaixo o mesmo cone leva a borda do polígono (χ=0) ao disco (χ=1). **E 1 é o χ do
PONTO: o cone é contráctil, colapsa a UM ponto.** É essa a passagem, e é ela que faz as ÁREAS
existirem — sem o cone, o polígono é só a borda e não tem área para medir.

**E DAÍ SAI e, PELOS VOLUMES.** «É a razão entre os volumes de um andar para outro; dá a
exponencial em todos os andares; é a unidade; seu próprio inverso.» O n-simplexo tem volume
`1/n!` e **é o cone sobre o de baixo**, donde

    V_n / V_{n−1} = 1/n   ← A RAZÃO DO CONE       χ(simplexo) = 1 em todo andar
    e = Σ V_n                                      a soma dos volumes da torre

E as três propriedades saem da mesma torre:
- **a exponencial em todos os andares**: `Σ xⁿ/n! = eˣ`, com os volumes por coeficiente;
- **é a unidade**: `n·c_n = c_{n−1}` — eˣ é a sua PRÓPRIA DERIVADA;
- **seu próprio inverso**: `eˣ ⊛ e^{−x} = δ`, medido pelos binomiais — e **o δ é o mesmo
  Dirac da base ortonormal**. Expansão e contração são inversas na CONVOLUÇÃO.

**E a CF de e é a PA** — `[2;1,2,1,1,4,1,1,6,1,…]`, padrão (1,2k,1), com a progressão
aritmética de razão 2 nos quocientes: não periódica, logo por Lagrange **e não é quadrático**.
É o contraponto exacto do metálico, cuja PA tem razão **zero** — a PG de período um.

**E o ln é a CONTAGEM**: de `σ·|σ†| = 1` vem `ln σ + ln|σ†| = 0`, e em inteiros o logaritmo
realiza-se contando dígitos — `#dig(t_k)` cresce linearmente com declive `log σ`.

**O ALVO SÃO OS POLINÓMIOS; OS METÁLICOS SAEM COMO SOLUÇÕES.** O Aarão: «tira o metálico do
palco, o alvo são os polinómios, metálicos saem como soluções» e «migra logo tudo, porque
**um número aqui é um polinómio, não simplesmente um escalar**». O objecto é

    p(x) = x^n − c₁x^{n−1} − … − c_n,  mónico, em ℤ[x]

e dele saem a companheira, a recorrência e as raízes — as raízes é que são as soluções.

- **`|det(companheira)| = |termo constante|`** ⟹ `|det| = 1 ⟺ termo ±1 ⟺ A RAIZ É UNIDADE`.
  Medido em dez polinómios, graus 2–4. Não é propriedade de uma família: é do polinómio.
- **o inteiro vem de NEWTON**: as somas de potências `P_k = Σ raízes^k` são inteiras e
  obedecem à recorrência **do próprio polinómio**. É daí que vem o inteiro que o operador
  «produz» — e o grau não é dois: `x³−x−1`, `x³−x²−x−1`, `x⁴−x³−1` fazem o mesmo.
- **UM NÚMERO É UM POLINÓMIO**: um elemento é a classe de `ℤ[x]/(p(x))` de grau < n, e as
  operações são as dos polinómios reduzidas por p — 171 366 produtos medidos. **O ESCALAR é
  o caso do GRAU UM**, `ℤ[x]/(x−a) ≅ ℤ`: o escalar não é o objecto, é o andar térreo.
- **os metálicos são as raízes de `x² − mx − 1`** — grau dois com termo constante −1, o mais
  simples com |det| = 1 —, e o `t_k` do resto do paper é exactamente o `P_k` desse caso. E o
  controlo `x² − 2x − 4` tem termo 4: **a raiz não é unidade**, e é aí que quebra.

E sete reparos do revisor, dos quais três eram erros: **«p nunca divide 2n» era FALSO**
(n=3,p=3 dá 3|6); **π entrava na definição de t_n** — agora `t_n` caracteriza-se pela ORDEM
(`C^n = −I`, sem `C^k = ±I` antes) e `2cos(π/n)` é identificação posterior; e **a classe A era
definida pelo §metálico** — agora define-se pelos convergentes pares. Mais: o **pombal dá
unicidade dos APROXIMANTES RACIONAIS**, não do real; o cone separa-se em **duas tabelas** por
dimensão (S¹→D² e S²→B³); a CF de e tem a PA na **subsequência central**, não na sucessão
inteira; e «o convergente par seguinte» → «ALGUM convergente par posterior».

`tests/geometria_real.c` (48:0) na ordem obrigatória do eval: 8 leis → base ortonormal →
bit a bit → dual → recorrência → Pisot → encaixe → corte → completude → ℝ → π_k/gume.
`papers/geometrico.tex` 17 páginas; `corpo_universal.tex` 54, e a compilar.
Ver [[project-o-real-e-o-corte]], [[feedback-verdadeiro-e-parcial]].
