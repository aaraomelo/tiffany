# A cifra — o circuito autossimilar, e o que dele se colhe

Cada arquivo é **uma peça só, medida**: mede uma afirmação contra um oráculo externo e
devolve **resíduo 0** ou falha. Não se implementa a matemática — o circuito, sendo o corpo,
já faz; aqui apenas se **colhe** e se confere. C puro (libc + libm):

```
cc -O2 <arquivo>.c -lm -o <arquivo> && ./<arquivo>
```

Para rodar **tudo de uma vez** — a lista sai dos próprios papers, e cada medidor roda sob teto de
memória e `timeout`:

```
./tools/bateria.sh
```

Os três papers colhem daqui: `teoria.tex` (o corpo), `microprocessador.tex` (o gabarito
analógico), `tiffany.tex` (a fala) — todos na raiz do projeto.

---

## A peça, uma vez — os headers compartilhados

O gato é o mesmo em toda parte; para não copiá-lo, ele vive **uma vez** em cada header e é
reusado (`#include`). A auto-similaridade posta no próprio código — a peça, não a cópia.

| header | a peça | reusada por |
|---|---|---|
| `gp2.h`  | o gato em `GF(p²)=ℤ_p[σ]` (o plano ℂ finito) | `duais`, `navega`, `complexo`, `ordem` |
| `gf2n.h` | o corpo binário `GF(2ⁿ)` (`gfmul`, o inverso pelo dual) | `recursao`, `dimensoes` |
| `quat.h` | os quaternions `ℍ=M₂(ℤ_p)` (`mmul`, o salto) | `esquilo`, `saltos` |
| `pgm.h`  | o leitor de imagem PGM binária (`le_pgm`) | `completo`, `linear`, `ordem`, `venom` |

## O circuito fractal — digital e analógico (o neurônio)

O gato nu, subindo a torre; a **mesma peça em dois meios**.

| arquivo | o que é |
|---|---|
| `neuronio.c` | o **digital** (bits): o gato `A_m` sobe as torres (temporal `Aᵏ→σ_m`, dimensional `Aₙ` em `Rⁿ`); o esquilo `Aₙ⁻¹` desce; os metais `σ_m` (ouro/prata/bronze) |
| `neuronio_analog.c` | o **analógico** (correntes): as mesmas etapas pelo translinear + Kirchhoff; alinha com o digital, resíduo 0 |
| `fractal.c` | valida os dois (§1–6): o gato, as torres, os metais, a dualidade `gato∘esquilo=id` |

## A peça — o gato e o esquilo

O gato e o seu esquilo, uma peça em toda escala. É o mesmo laço, `σ = m + 1/σ`.

| arquivo | o que colhe |
|---|---|
| `checkup.c`  | o gato `A_m` e o esquilo — o corpo fechado, reversível, contínuo, ordenado, completo, multidimensional |
| `esquilo.c`  | o esquilo (`det=+1`, `G⁴=I`) — o dual que traz de volta |
| `duais.c`    | os dois pontos fixos `σ` (negro/sorvedouro) e `σ'=−1/σ` (branco/fonte) — a dualidade |
| `estelar.c`  | **a base certa é `q=e^{−2π}`: π gera cada metal** (fonte: `broca-so/papers/corpo_estelar.tex`). Na fc **regular** `φ=[1;1,1,…]` é o pior aproximável (Hurwitz) — certo naquela base e contra o fluxo; na **base π** o ouro é **algébrico**: `R(e^{−2π}) = √(φ√5) − φ`, raiz de `x⁴+2x³−6x²−2x+1`, por **multiplicação complexa**. E há lei: `Q_m = x⁴+2mx³−6x²−2mx+1 = (x²+2σ_m x−1)(x²+2σ_m′x−1)`, com `v_m = √(σ_m²+1) − σ_m` (m=1..8). E o **dupolinômio é a COLISÃO** `P_g=x²` com `P_a=mx+1` |
| `pi.c`       | **a reta é a órbita do 1, e até onde π comanda**: com "somar 1" + `×σ` alcança-se **todo** `GF(pⁿ)` (6 corpos); os metais são de π exatamente (`1/φ=2cos(2π/5)`, somas de Gauss `√5,√13,√17`); **π é o 0** — ciclotomia é *corte do círculo*, e `Σζ_n^k = 0` devolve o centro, com `ζⁿ=1` fechando. E o **corte**: raízes da unidade geram o **abeliano máximo** (Kronecker–Weber) — todo metal é quadrático logo está dentro, mas o **plástico** `x³−x−1` (`S₃`, disc −23) fica fora, e é o fator do furo do ouro em `n=5` |
| `rotulos.c`  | **inteiro, racional e irracional são RÓTULOS** — trocam com a base: `1/φ` é irracional em `ℚ` e é `(−1,1)`, **inteiro**, em `ℤ[σ]`. Cada **dimensão é um irracional** que **colapsa em inteiro** na passagem (`σ_n` = `(0,1,0,…)`, grau `n` sobre `ℚ`), e um só gerador varre a base de toda reta. Logo **sem lista de primos**: o primo sai do inteiro por regra. E `√n` de todo não-quadrado tem fração contínua periódica — o período é a dobra; os quadrados não dobram (as partes **estéreis**) |
| `progressoes.c`| **PA e PG de ordem `k`, e o polinômio generalizado (o par)**: a PA de ordem `k` **é** o polinômio de grau `k` (base binomial, Newton); a PG de ordem `k` é a sua imagem por `exp`, na base das potências; e a ponte `∏` (exp/log, exata em `ℤ_p`) atravessa **sem mudar a ordem**. `Δ` e `Σ` são gato e esquilo do lado `⊕` (`Δ∘Σ=id`, `Σ` sobe a ordem), Stirling é o dicionário das duas bases (`S·s=I`), e a peça `U_n(m)` **não é PA nem PG** de ordem finita — é o que fica entre as duas |
| `rotaciona.c`| **rotaciona um polinômio pelo gato, desrotaciona pelo esquilo** — `σ⁻¹ = σⁿ⁻¹ − m·σⁿ⁻²` colhido da borda (sem Fermat); ida e volta exata em `n=2..8`, dado qualquer e prosa crua. E os **dois esquilos**: `σ⁻¹ = −σ'` — pelo inverso volta `A`, pelo conjugado volta `−A` (a folha) |
| `converte.c` | **converter dois polinômios quaisquer**: `C = B ⊛ A⁻¹` com `A⁻¹` do **dual** (os `n−1` conjugados de Frobenius sobre a norma, sem Fermat; em `n=4`, três batidas). Ida e volta, dado arbitrário e prosa |
| `gerador_analog.c`| **o mesmo gerador nos dois meios**: a torção discreta (`w=36043`, ordem 256) e a **rotação da malha LC** (`2π/n` colhida de `ω₀=1/√(LC)`, na borda `|λ|=1`) — `n` passos voltam à identidade (`6e-15`), a torre **é** o zoom do §B.7 (mesmo `dt`, dobrar `ω₀` eleva a projeção ao quadrado, `Z₀` fixo), e a **convolução circular sai igual dos dois** (exata em `ℤ_p`, `8e-12` no LC, ambas batendo o oráculo `O(n²)`). Dente: errar o ângulo por 1/257 não fecha. E as ordens **ímpares** (`k=3,5,7`) passam igual nos dois meios (`p=31,61,71`; convolução exata no inteiro e igual no circuito) |
| `gerador.c`  | **o gerador do CORPO UNIVERSAL (enredo §150.1), e ele já estava dado**: a única propriedade usada é `χ_k(u+v)=χ_k(u)χ_k(v)` (o caractere leva `⊕` a `⊗`, 7680/7680), e a torção BASTA porque o dual do dual devolve o grupo — as inversas (transformada, convolução, flip) são consequências de **uma** razão, não três teoremas. Ao compor, os expoentes **somam** (a PA); ao iterar, **multiplicam** (a PG). Também: o gerador global, e por que vai escrito**: `p=40961, n=256, g=3` (o MENOR), `w=36043`, `r=16` — reconferidos; `ord(w)=n` exata, ida e volta em **inteiros**, três obras fundem e cada uma volta. É **FRACTAL**: 8 níveis, cada torção o quadrado da de cima, e por isso **sem tabelas** (PA nos expoentes, PG nas potências, 2 escalares de estado). E o aviso do enredo sai corrigido: fundir/abrir é **invariante** ao gerador (permuta do dual); onde ele é areia é ao **ordenar** o dual (truncar: 256/256 discordam) |
| `lemniscata.c`| **π se dobra, e o AGM é o fator da costura**: `ϖ = π/M(1,√2) = 2∫₀¹dt/√(1−t⁴)` pelas duas vias (erro **0**); e a costura é geral — `K(k) = π/(2M(1,k′))` para todo `k`. O círculo é `k=0` (π puro), a lemniscata é `k=1/√2` = a **primeira âncora τ=1**: a escada das curvas deformadas e a dos singular values são a mesma. Dobrar custa exatamente `M(1,√2)` |
| `agm_analog.c`| **o AGM colhido no circuito**: `⊕` é Kirchhoff + espelho 2:1 e `⊗` é o translinear com **somador em ganho ½** — que **dispensa a corrente de referência** (`I_S` ×10⁴ e `T` de 250–400 K não mudam nada). O laço de correntes dá o AGM dobrando os dígitos, o invariante `I(a,b)` fica fixo, e o dente (somador cheio = produto) **estoura** em 8 batidas |
| `agm.c`      | **o AGM procura a âncora; o invariante é a integral**: `a←(a+b)/2` (⊕) e `b←√(ab)` (⊗) alternados, convergência quadrática (razão fixa `0,0858`), e `I(a,b)=I((a+b)/2,√(ab))` exata. `1/AGM = (2/π)I` — o invariante **é** o ponto de ancoragem. E a família é contínua: `τ=K′/K` varre `(0,∞)`, e as estruturas inteiras são os **singular values** `τ=√N` (CM), algébricos exatos (`1/√2`, `√2−1`, `(√3−1)/(2√2)`, `3−2√2`) |
| `deforma_d.c`| **a deformação em dimensões maiores (KAM)**: ressonâncias crescem como `Nᵈ` (contagem = fórmula de Delannoy), fração ressonante cresce com `d`, e no mapa **simplético** de Froeschlé o mesmo `K` destrói mais em dimensão maior (em `d=1` a virada cai sozinha no `K≈0,9716` de Greene). Entre irracionais, a curva **áurea** é a última a romper; racional não dá toro, dá **ilha** travada |
| `agm_deforma.c`| **o invariante do AGM sobrevive à deformação?** Dissociação: a **velocidade** resiste (quadrática em todo `p≠1`, razão fechada `(1−p)/(8M)`, conferida em 5 valores) mas o **invariante** morre à primeira ordem (`|ΔI|/I ~ p`, expoente `0,9994`) — nenhum `p` crítico, o oposto de KAM. Sob a **isogenia** (Landen, `τ→2τ`) ele é indestrutível e leva âncora `τ=1` em âncora `τ=2` (`3−2√2`, resíduo 0): sobrevive como **classe**, não como ponto |
| `deforma.c`  | **a deformação é a dinâmica; a simetria só ancora**: racionais densos e de **medida nula**; órbita racional fecha em `q` (cristalização sem dinâmica) vs `1/φ` que nunca fecha e tem só **3 comprimentos** de gap. No mapa do círculo a fração travada vai de **0,0000** (`K=0`) a `0,70` (`K=0,99`) — a simetria dá o centro da língua, a deformação dá a largura; e o ouro é o último a travar |
| `entre.c`    | **há dimensão intermediária?** a variável contínua é a **inclinação** `α`: racional `p/q` → cristal de período exatamente `q`; irracional → quasicristal (`p(k)=k+1`). A transição é a escada dos convergentes (período `3→610`, concordância `6→1595`, razão `→φ²` e `→1`). E o intermediário é a **escala**: em toda janela finita há período local `≈L/φ` (sempre Fibonacci) — o rank só salta no limite |
| `quasi.c`     | **por que a dimensão 5 não tem ouro**: `2cos(2π/5)=φ−1 ∉ ℤ` e `φ(5)=4` proíbem o cinco em rede (as duas contas dão `{1,2,3,4,6}`); o furo fatora em `Φ₆` (giro) × `x³−x−1` (o menor Pisot, plástico) — a dim. 5 separa giro de crescimento; e a palavra do gato do ouro é aperiódica de complexidade `k+1` (sturmiana): o **quasicristal** |
| `transforma.c`| **escala · cisalhamento · rotação, e a composição**: `≤2n` cisalhamentos levam qualquer polinômio em qualquer outro (SL_n é transitivo — a escala só ajusta o volume), tudo reversível operação por operação. E a diferença das rotas: em `GL_n` há `\|GL_n\|/(pⁿ−1)` transformações que chegam (156 em `n=2,p=13`), no corpo há **uma** — a única que comuta com a convolução. Acha também o furo do ouro em `n=5` |
| `venom.c`    | o `0` (Venom): a imagem inteira que se reparte, deixando o vértice |
| `recursao.c` | **a multiplicação recursiva** — dim `n` pela `n−1`, a auto-similaridade posta como o produto |
| `dimensoes.c`| cada dimensão, par e ímpar, é um corpo — recursivo `=` direto |

## O corpo — as propriedades colhidas

| arquivo | o que colhe |
|---|---|
| `ordem.c`    | o corpo é **ordenado** |
| `completo.c` | o corpo é **completo** |
| `complexo.c` | os atratores formam `ℂ` — `σ` e `σ'=σ̄`, um em cada eixo |
| `norma.c`    | os conjugados de Frobenius — o inverso pelo **dual** (`σ⁻¹` sem Fermat) |
| `sombras.c`  | a forma simplética, `k` gatos — o **lucro** do eixo ímpar |
| `saltos.c`   | os saltos entre órbitas quaisquer — os quaternions `ℍ = M₂(ℤ_p)` |
| `fermat.c`   | **Fermat cai no gato** — Wiles `ρ(Frob)` e Kummer `σ_n` são o gato `2×2` |
| `isomorfo.c` | **elétron e bóson são a mesma função de onda** — ponto a ponto (mesmo `C(n,N)`, mesma norma); a estatística está no *produto ordenado* (a string), não no estado. As **folhas**: `σσ'=−1` é a ida e volta que descola, holonomia `−1` em toda face, e a duplicidade é só das dimensões **pares** (`det A_n=(−1)^{n+1}`) |

## Os terminais — a colheita (o analógico é o gabarito)

| arquivo | o que colhe |
|---|---|
| `analog.c` | as **10 colheitas** no circuito (§B.1–B.10): o translinear `ANTILOG(log a + s·log b − s·log ref)`, a convolução (`s=+1`, gato/×), a deconvolução (`s=−1`, esquilo/÷), a mult em `Rⁿ`, a interpolação — coordenadas contínuas |
| `tres_reconstroi.c` | as **três batidas** `ℱ³=ℱ⁻¹` — a prosa volta byte a byte, `16384` vetores semânticos, `0` erros |
| `interp.c` | a interpolação (a matemática: `n` amostras **são** o polinômio de `Rⁿ`) — depois colhida no circuito em `analog.c` §B.9 |

## A aplicação — a fala (Tiffany)

| arquivo | o que colhe |
|---|---|
| `linear.c`   | lineariza o sinal — um texto é um sinal como outro qualquer |
| `orbitas.c`  | o corpus vira um **grafo de órbitas** — frases de mesma estrutura, mesmo atrator |
| `navega.c`   | o navegante percorre os caminhos que passam pela fala — cobertura `100%`, reversível |
| `navegante.c`| a busca **sem Metrópolis** — a recursão (a fração contínua desdobrada), backtrack, resíduo 0 |
| `../tatoeba/`| o pipeline real sobre o Tatoeba: `ingestor`, `convtexto`, `caminha`, `dual`, `tiffany` |

## Base histórica

| arquivo | nota |
|---|---|
| `micro.c` | o corpo no **discreto** (10 seções). O gabarito é hoje o analógico (`analog.c`); "discretização é inútil" — útil só como oráculo. |
