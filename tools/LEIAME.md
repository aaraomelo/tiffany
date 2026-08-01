# A cifra — o circuito autossimilar, e o que dele se colhe

Cada arquivo é **uma peça só, medida**: mede uma afirmação contra um oráculo externo e devolve
**resíduo 0** ou falha. Não se implementa a matemática — o circuito, sendo o corpo, já faz; aqui
apenas se **colhe** e se confere. C puro (libc + libm):

```
cc -O2 <arquivo>.c -lm -o <arquivo> && ./<arquivo>
```

Para rodar **tudo de uma vez** — a lista sai dos próprios papers, cada medidor roda sob teto de
memória e `timeout`, e a saída distingue três casos (verde, negativo-por-projeto, falha):

```
./tools/bateria.sh
```

Estado atual: **59 medidores — 57 verdes, 2 negativos por projeto, 0 falhas.** Os dois negativos são
`../tatoeba/ancora.c` e `../tatoeba/homogeneo.c`, que devolvem `1` porque **provam** que não existe
solução para o sistema da tradução — é o resultado documentado em `tiffany.tex` §6, não uma quebra.

As seções abaixo seguem a ordem de `teoria.tex`. Os três papers colhem daqui: `teoria.tex` (o corpo),
`microprocessador.tex` (o gabarito analógico), `tiffany.tex` (a fala) — todos na raiz.

---

## A peça, uma vez — os headers compartilhados

O gato é o mesmo em toda parte; para não copiá-lo, ele vive **uma vez** em cada header e é reusado
(`#include`). A auto-similaridade posta no próprio código — a peça, não a cópia.

| header | a peça | reusada por |
|---|---|---|
| `gp2.h`  | o gato em `GF(p²)=ℤ_p[σ]` (o plano ℂ finito) | `duais`, `navega`, `complexo`, `ordem` |
| `gf2n.h` | o corpo binário `GF(2ⁿ)` (`gfmul`, o inverso pelo dual) | `recursao`, `dimensoes` |
| `quat.h` | os quaternions `ℍ=M₂(ℤ_p)` (`mmul`, o salto) | `esquilo`, `saltos` |
| `pgm.h`  | o leitor de imagem PGM binária (`le_pgm`) | `completo`, `linear`, `ordem`, `venom` |

## §1 — a peça: o gato e o esquilo

O mesmo laço, `σ = m + 1/σ`, em toda escala.

| arquivo | o que colhe |
|---|---|
| `checkup.c`  | o gato `A_m` e o esquilo — o corpo fechado, reversível, contínuo, ordenado, completo, multidimensional |
| `esquilo.c`  | o esquilo (`det=+1`, `G⁴=I`) — o dual que traz de volta |
| `duais.c`    | os dois pontos fixos `σ` (negro/sorvedouro) e `σ'=−1/σ` (branco/fonte) — a dualidade |
| `espelho.c`| **quebrar o espelho não cria nem some espelho** — toda cisão é **partição**, e por isso conserva. **Sem um único float**: nos 256 bytes, `pop(b&0xAA) + pop(b&0x55) = pop(b)`, os cacos recompõem (`|`) e não se sobrepõem (`&` = 0), logo somar é unir. O gato é **permutação** mod `2^L` (todas as `2^2L` imagens distintas, `L=1..7`) — atravessar não perde. A mesma lei em 4, 8, 16 e 32 bits (**poucos bytes bastam**: o resto é cópia) e nos dois meios — no analógico a cisão é o **nó** de Kirchhoff, e é a mesma conta, sem casa decimal para concordar |
| `semente.c`| **o que não cabe na descrição é a SEMENTE** — e com ela a obra volta inteira. A assinatura diz *quantos*, a semente diz *quais*; o inverso reconstrói **256 de 256, bit a bit**, sem uma diferença. O espaço de sementes de cada classe é `C(4,p)·C(4,i)` e soma 256 — logo **a semente é maior onde a classe é maior**: o genérico não é o esvaziado, é onde cabe mais gente diferente. E o necrotério tem número: descartar a semente colapsa 256 em 25, **matando 231** — o limite (toda semente zero) é uma assinatura só para todos, que é o que a caixa exige para funcionar |
| `assinatura.c`| **a contagem não perde: ela ASSINA** — e o defeito era meu, de leitura. Inteiro, sem float, sem amostra, os 256 bytes varridos. Os dois papéis, que eu misturava, ficam separados: o **gato recupera** (bijeção, nada some) e a **contagem nomeia** (assinatura, agrupa por projeto). As fibras `C(4,p)·C(4,i)` somam 256 e dão **25 assinaturas**; **4 são próprias** (`0x00`, `0xAA`, `0x55`, `0xFF` — os puros, que por serem singulares não formam classe); e a **resolução** é exata: 2322 pares de 32640 assinam igual. Mesma assinatura no analógico — o que é força: uma que dependesse do meio não identificaria nada |
| `selo.c`| **o selo: uma chave emaranhada (o sistema) e uma dividida (os clientes)**. Critério exato de separabilidade — remodelando `|Ψ⟩` por uma bipartição, separável ⟺ **posto 1**: a dividida dá 1 em todo corte (um fator por cliente), a do sistema dá **2** (não se escreve como produto — por isso sela). A soma fecha em **zero**, e as fases do código (`φ = π·o/8`, raízes 16-ésimas) somam o centro — a ciclotomia. E **recupera-se por diferenças**: elas são invariantes sob deslocamento global, dando tudo menos a origem, que só a chave do sistema fixa. O selo mede-se por contagem: varrendo `Z_97`, a parte que falta tem **97** valores possíveis sem a chave do sistema e **1** com ela |
| `isserlis.c`| **o defeito `E_k` em forma fechada, ou não entra**: a esperança gaussiana é soma finita de emparelhamentos (Wick), e a maquinaria valida-se **de fora** — Isserlis contra Gauss–Hermite (exata para polinômios) em 9 momentos de uma gaussiana correlacionada, 13-14 casas. Achado que a tabela do capítulo não declara: **`N_k` é ele próprio um número de Wick** — `3,15,105,945` são os emparelhamentos de `k+1` pontos, isto é `k!!`, por força bruta. Logo `C_k² = 2·N_k·6^((k−1)/2)`, e as duas formas fechadas são **uma** identidade (`k!! = k(k−2)!!`, `a_k/b_k = 2·6^…`) |
| `prensor.c`| **o prensor é o cone que o chicote conserva** — sem ele a dinâmica não fecha (transporte sem invariante é agitação). `Q(p)=pᵗMp` com `M=[[−2,m],[m,2]]`, e `Q(Ap) = −Q(p)` **exato em inteiros** (2500 pontos, `m=1..4`): uma batida troca o sinal, duas devolvem. O cone `Q=0` **é** o par de atratores `(σ,1)` e `(σ′,1)`. E as **marcas são espirais**: quem começa fora nunca toca (`|Q|` constante em 40 batidas) e nunca escapa (`|c₂/c₁|` cai por `1/σ²` a **cada** batida) — nem chega, nem sai, enrola. Logarítmica: `Δ(log r) = log σ` a `1e-19`; com o esquilo, a áurea `r = φ^(2θ/π)` exata |
| `prensor.c`| **o prensor é o cone que o chicote conserva** — sem ele a dinâmica não fecha. `Q(p)=pᵗMp` com `M=[[−2,m],[m,2]]`, e `Q(Ap) = −Q(p)` **exato em inteiros** (2500 pontos). O cone `Q=0` **é** o par de atratores. As **marcas são espirais**: nunca toca (`|Q|` constante) e nunca escapa (`|c₂/c₁|` cai `1/σ²` por batida) — `Δ(log r) = log σ` a `1e-19`, e com o esquilo a áurea `r = φ^(2θ/π)` exata. E a **ponta é o 0**: único ponto fixo, a passagem reversível — o sinal alterna a cada batida e a órbita **nunca atravessa** (salta); as batidas duais fecham em `G³ = G⁻¹`, `G⁴ = I` |
| `chicote.c`| **o chicote é o tensor**: compor gatos É a contração de Einstein `(A·B)^i_k = Σ_j A^i_j B^j_k` (exato em inteiros; para `m=1` as entradas são Fibonacci). A forma **k-ária não dá poder** — as parentizações são `Cat(k−1)` (`1,2,5,14,42,132,429`) e **todas concordam**, uma a uma: o ganho é não ter de inventar a ordem, que entre chaves simultâneas não existe. E o **mesmo flip nas duas formas bilineares**: `AᵗΩA = det(A)Ω` (o volume) e `AᵗMA = −M` (o cone, com `M = [[−2,m],[m,2]]` saindo **exato** porque `σσ'=−1` e `σ+σ'=m`) — uma só peça, dois retratos. Curvatura `N−1` conferida para `N=2..8` |
| `antissimetrico.c`| **a assimetria inicial é uma ANTIssimetria, e é a única que atravessa a dimensão**: `ω(u,u)=0` (nada, sobre um ponto) e não-degenerada (tudo, sobre pares), com `Sp` **transitiva** nos pontos (`6400/6400` transvecções construídas) — ela só fala de **diferença**. As alternantes não-degeneradas são `0` em toda dimensão ímpar e **uma só classe** em toda par (`28`, `468`, `12400` batendo `|GL_n|/|Sp_n|`; base simplética construída `468/468`), contra `≥2` classes da simétrica em `F_p` e `n+1` em `R`. O gato mora nela (`ω(Au,Av)=det(A)ω(u,v)`, `det=−1`: **anti**-conserva — a holonomia das folhas). Em `n=1` ela é **nula** e em `n=2` é `C` (`J²=−I`, `ω(u,v)=⟨u,Jv⟩`): os complexos são o primeiro lugar onde há algo. E a lei de potência sai daí: `det = Pf²` e o módulo `|x|^d` com `d=1,2,4` em `R,C,H` |
| `instrumento.c`| **o instrumento cria a assimetria**: sem lei, toda permutação preserva a estrutura (`S_N`, simetria máxima, nada distingue); com lei, só sobrevive quem **comuta** — o grupo cai de `N!` para `d^c·c!` (força bruta = fórmula; `40320→32`). **Observar é o que sobra quando a permutabilidade acaba.** E a realidade observada é produto do instrumento (`6,12,24,84,168,1` classes no mesmo corpo) — mas **coerente**: as partições formam um **retículo** de refinamentos, logo a medida escolhe a resolução e não fabrica o substrato |
| `significado.c`| **quando algo significa**: no repouso **tudo** é invariante, logo nada significa; uma lei particiona mas precisa **conservar** (`×σ` tem `N=−1` e a norma alterna — não nomeia; `×σ²` está na **borda** e nomeia); e se a lei alcança tudo, volta a não haver significado (uma classe só). O significado é uma **janela**: `1 < classes < pontos`, com lei conservativa |
| `venom.c`    | o `0` (Venom): a imagem inteira que se reparte, deixando o vértice |
| `rotaciona.c`| **rotaciona um polinômio pelo gato, desrotaciona pelo esquilo** — `σ⁻¹ = σⁿ⁻¹ − m·σⁿ⁻²` colhido da borda (sem Fermat); ida e volta exata em `n=2..8`, dado qualquer e prosa crua. E os **dois esquilos**: `σ⁻¹ = −σ'` — pelo inverso volta `A`, pelo conjugado volta `−A` (a folha) |
| `converte.c` | **converter dois polinômios quaisquer**: `C = B ⊛ A⁻¹` com `A⁻¹` do **dual** (os `n−1` conjugados de Frobenius sobre a norma, sem Fermat; em `n=4`, três batidas). Ida e volta, dado arbitrário e prosa |
| `transforma.c`| **escala · cisalhamento · rotação, e a composição**: `≤2n` cisalhamentos levam qualquer polinômio em qualquer outro (`SL_n` é transitivo — a escala só ajusta o volume), tudo reversível. E a diferença das rotas: em `GL_n` há `\|GL_n\|/(pⁿ−1)` que chegam (156 em `n=2,p=13`), no corpo há **uma** — a única que comuta com a convolução |

## §2 — os rótulos, e o gerador que já estava dado

| arquivo | o que colhe |
|---|---|
| `rotulos.c`  | **inteiro, racional e irracional são RÓTULOS** — trocam com a base: `1/φ` é irracional em `ℚ` e é `(−1,1)`, **inteiro**, em `ℤ[σ]`. Cada **dimensão é um irracional** que **colapsa em inteiro** na passagem (`σ_n = (0,1,0,…)`, grau `n` sobre `ℚ`), e um só gerador varre a base de toda reta. Logo **sem lista de primos**: o primo sai do inteiro por regra. E `√n` de todo não-quadrado tem fração contínua periódica — o período é a dobra; os quadrados não dobram (as partes **estéreis**) |
| `pi.c`       | **a reta é a órbita do 1, e até onde π comanda**: com "somar 1" + `×σ` alcança-se **todo** `GF(pⁿ)` (6 corpos); os metais são de π exatamente (`1/φ=2cos(2π/5)`, somas de Gauss `√5,√13,√17`); **π é o 0** — ciclotomia é *corte do círculo*, e `Σζ_n^k = 0` devolve o centro, com `ζⁿ=1` fechando. E o **corte**: raízes da unidade geram o **abeliano máximo** (Kronecker–Weber) — todo metal é quadrático logo está dentro, mas o **plástico** `x³−x−1` (`S₃`, disc −23) fica fora, e é o fator do furo do ouro em `n=5` |
| `estelar.c`  | **a base certa é `q=e^{−2π}`: π gera cada metal** (fonte: `broca-so/papers/corpo_estelar.tex`). Na fc **regular** `φ=[1;1,1,…]` é o pior aproximável (Hurwitz) — certo naquela base e contra o fluxo; na **base π** o ouro é **algébrico**: `R(e^{−2π}) = √(φ√5) − φ`, raiz de `x⁴+2x³−6x²−2x+1`, por **multiplicação complexa**. E há lei: `Q_m = x⁴+2mx³−6x²−2mx+1 = (x²+2σ_m x−1)(x²+2σ_m′x−1)`, com `v_m = √(σ_m²+1) − σ_m` (`m=1..8`). E o **dupolinômio é a COLISÃO** de `P_g=x²` com `P_a=mx+1` |
| `gerador.c`  | **o gerador do CORPO UNIVERSAL (enredo §150.1) — já estava dado**: a única propriedade usada é `χ_k(u+v)=χ_k(u)χ_k(v)` (o caractere leva `⊕` a `⊗`, 7680/7680), e a torção BASTA porque o dual do dual devolve o grupo — as inversas (transformada, convolução, flip) são consequências de **uma** razão, não três teoremas. O global: `p=40961, n=256, g=3` (o MENOR), `w=36043`, `r=16`, reconferidos; `ord(w)=n` exata, ida e volta em **inteiros**, três obras fundem e cada uma volta. **Um gerador e projeções**: `w_d = g^((p−1)/d)`, e os 16384 "outros geradores" são `g^j`. A cadeia **abre de baixo** nas ordens que a necessidade pede (`k=1..12`, o primo vem da ordem). E o aviso do enredo sai corrigido: fundir/abrir é **invariante** ao gerador; onde ele é areia é ao **ordenar** o dual (truncar: 256/256 discordam) |
| `gerador_analog.c`| **o mesmo gerador nos dois meios**: a torção discreta (`w=36043`, ordem 256) e a **rotação da malha LC** (`2π/n` de `ω₀=1/√(LC)`, na borda `\|λ\|=1`) — `n` passos voltam à identidade (`6e-15`), a torre **é** o zoom do §B.7, e a **convolução circular sai igual dos dois** (exata em `ℤ_p`, `8e-12` no LC, ambas batendo o oráculo `O(n²)`). Dente: errar o ângulo por 1/257 não fecha. As ordens **ímpares** (`k=3,5,7`) passam igual nos dois meios |

## §3 — as duas progressões, e o dupolinômio

| arquivo | o que colhe |
|---|---|
| `progressoes.c`| **PA e PG de ordem `k`**: a PA de ordem `k` **é** o polinômio de grau `k` (base binomial, Newton); a PG de ordem `k` é a sua imagem por `exp`, na base das potências; e a ponte `∏` (exp/log, exata em `ℤ_p`) atravessa **sem mudar a ordem**. `Δ` e `Σ` são gato e esquilo do lado `⊕` (`Δ∘Σ=id`, `Σ` sobe a ordem), Stirling é o dicionário das duas bases (`S·s=I`), e a peça `U_n(m)` **não é PA nem PG** de ordem finita — é o que fica entre as duas |
| `recursao.c` | **a multiplicação recursiva** — dim `n` pela `n−1`, a auto-similaridade posta como o produto |
| `dimensoes.c`| cada dimensão, par e ímpar, é um corpo — recursivo `=` direto |

## §4 e §5 — o corpo, e a dualidade

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

## §6 — a dimensão é ancoragem; o contínuo é o corpo

A deformação como dinâmica, e o AGM como o invariante que a atravessa.

| arquivo | o que colhe |
|---|---|
| `quasi.c`     | **por que a dimensão 5 não tem ouro**: `2cos(2π/5)=φ−1 ∉ ℤ` e `φ(5)=4` proíbem o cinco em rede (as duas contas dão `{1,2,3,4,6}`); o furo fatora em `Φ₆` (giro) × `x³−x−1` (o menor Pisot, plástico) — a dim. 5 separa giro de crescimento; e a palavra do gato do ouro é aperiódica de complexidade `k+1` (sturmiana): o **quasicristal** |
| `entre.c`    | **há dimensão intermediária?** a variável contínua é a **inclinação** `α`: racional `p/q` → cristal de período exatamente `q`; irracional → quasicristal (`p(k)=k+1`). A transição é a escada dos convergentes (período `3→610`, concordância `6→1595`). E o intermediário é a **escala**: em toda janela finita há período local `≈L/φ` (sempre Fibonacci) — o rank só salta no limite |
| `deforma.c`  | **a deformação é a dinâmica; a simetria só ancora**: racionais densos e de **medida nula**; órbita racional fecha em `q` (cristalização sem dinâmica) vs `1/φ` que nunca fecha e tem só **3 comprimentos** de gap. A fração travada vai de **0,0000** (`K=0`) a `0,70` (`K=0,99`). E a robustez é da **classe modular**, não do número: os **nobres** (equivalentes a `φ` sob `SL(2,ℤ)`, sendo `φ` o ponto fixo do gato) ficam todos acima dos não-nobres |
| `deforma_d.c`| **a deformação em dimensões maiores (KAM)**: ressonâncias crescem como `Nᵈ` (contagem = fórmula de Delannoy), fração ressonante cresce com `d`, e no mapa **simplético** de Froeschlé o mesmo `K` destrói mais em dimensão maior (em `d=1` a virada cai sozinha no `K≈0,9716` de Greene). Entre irracionais, a curva **áurea** é a última a romper; racional não dá toro, dá **ilha** travada |
| `agm.c`      | **o AGM procura a âncora; o invariante é a integral**: `a←(a+b)/2` (⊕) e `b←√(ab)` (⊗) alternados, convergência quadrática (razão fixa `0,0858`), e `I(a,b)=I((a+b)/2,√(ab))` exata. `1/AGM = (2/π)I` — o invariante **é** o ponto de ancoragem. E a família é contínua: `τ=K′/K` varre `(0,∞)`, e as estruturas inteiras são os **singular values** `τ=√N` (CM), algébricos exatos |
| `agm_deforma.c`| **o invariante sobrevive à deformação?** Dissociação: a **velocidade** resiste (quadrática em todo `p≠1`, razão fechada `(1−p)/(8M)`) mas o **invariante** morre à primeira ordem (`\|ΔI\|/I ~ p`, expoente `0,9994`) — nenhum `p` crítico, o oposto de KAM. Sob a **isogenia** (Landen, `τ→2τ`) é indestrutível e leva âncora `τ=1` em âncora `τ=2`: sobrevive como **classe**, não como ponto |
| `agm_gerador.c`| **a dinâmica do AGM É a do gerador.** A **prova** são duas linhas: de `K(k₁)=((1+k′)/2)K(k)` e `K′(k₁)=(1+k′)K′(k)` sai `τ₁=2τ` por divisão (medi as duas separadamente, `≤5e-17`); e `(w_{2d})² = g^{2(p−1)/(2d)} = w_d`. A **forma forte**: nos dois lados a operação é um **homomorfismo de grau 2** — `x↦x²` leva `μ_{2d}` sobre `μ_d` com núcleo `{±1}` (todo degrau), e a 2-isogenia do toro tem grau 2 e núcleo 2. É a **multiplicação por 2 no grupo**, canônica. No expoente: `2·log√(ab) ≡ log a + log b` (261/261) — `⊕` é a média no **valor**, `⊗` a mesma média no **expoente** |
| `lemniscata.c`| **π se dobra, e o AGM é o fator da costura**: `ϖ = π/M(1,√2) = 2∫₀¹dt/√(1−t⁴)` pelas duas vias (erro **0**); e a costura é geral — `K(k) = π/(2M(1,k′))` para todo `k`. O círculo é `k=0` (π puro), a lemniscata é `k=1/√2` = a **primeira âncora τ=1**: a escada das curvas deformadas e a dos singular values são a mesma. Dobrar custa exatamente `M(1,√2)` |

## §7 — a colheita: os terminais, e o circuito nos dois meios

| arquivo | o que colhe |
|---|---|
| `analog.c` | as **10 colheitas** no circuito (§B.1–B.10): o translinear `ANTILOG(log a + s·log b − s·log ref)`, a convolução (`s=+1`, gato/×), a deconvolução (`s=−1`, esquilo/÷), a mult em `Rⁿ`, a interpolação — coordenadas contínuas |
| `agm_analog.c`| **o AGM colhido no circuito**: `⊕` é Kirchhoff + espelho 2:1 e `⊗` é o translinear com **somador em ganho ½** — que **dispensa a corrente de referência** (`I_S` ×10⁴ e `T` de 250–400 K não mudam nada). O laço de correntes dá o AGM dobrando os dígitos, o invariante `I(a,b)` fica fixo, e o dente (somador cheio = produto) **estoura** em 8 batidas |
| `tres_reconstroi.c` | as **três batidas** `ℱ³=ℱ⁻¹` — a prosa volta byte a byte, `16384` vetores semânticos, `0` erros |
| `interp.c` | a interpolação (`n` amostras **são** o polinômio de `Rⁿ`) — depois colhida no circuito em `analog.c` §B.9 |
| `neuronio.c` | o **digital** (bits): o gato `A_m` sobe as torres (temporal `Aᵏ→σ_m`, dimensional `Aₙ` em `Rⁿ`); o esquilo desce; os metais. *Pede um caminho de arquivo como argumento.* |
| `neuronio_analog.c` | o **analógico** (correntes): as mesmas etapas pelo translinear + Kirchhoff; alinha com o digital, resíduo 0. *Idem.* |
| `fractal.c` | valida os dois: o gato, as torres, os metais, a dualidade `gato∘esquilo=id` |
| `base.c` | **a base ortonormal, as duas torres, e a geração** (14 seções, 35 unidades). A base é `n` pontos da esfera dois a dois perpendiculares; as duas recursões repartem a **mesma** partição — o direto **soma** as projeções (`⟨x,y⟩₂ₙ = ⟨π₁x,π₁y⟩ + ⟨π₂x,π₂y⟩`), o cruzado **cruza-as** com o conjugado, e a metade antissimétrica do Cayley–Dickson **é** o `a×b`. Correção do Aarão: a dimensão não vem sozinha, vem no par `(n,2n)` — o `J` está medido em cada um **inclusive em `R¹⁰`**, então a dim 5 não é buraco, é projeção do 10. E **a reversibilidade é do par**: sozinha toda `R^n` (`n≥2`) tem divisor de zero; com a dual, `x⁻¹ = conj(x)/N(x)`. **O corpo é a torre inteira**, não o andar: um andar perde (`E∘C ≠ id`), a torre com os saldos não perde; e há a **torre dual que desce** (o traço, sobrejetivo — a inclusão falha onde ele fecha). As duas são **adjuntas** (`Sᵀ = D`): `S−D` antissimétrica, `S+D` simétrica, e a conservação **é** a antissimetria (`⟨Ax,x⟩=0`, `exp(tA)` ortogonal — contra 100/100 de quebra no simétrico). O comutador `[S,D]` cancela em todo andar interior e só sobra nas **pontas** (traço 0); `D⁸=0` — a dual esvazia, e resta **um** elemento, `ker D = coker S`: a cifra, fora do jogo. E o **origami**: a dobra guarda a memória da simetria porque tem **ordem finita** (`conj²=id`, `J⁴=I`, `F⁴=id`) — escalar por `1,3` volta ao início em `0` de `100`. A geração, contada em `GF(2^6)`: fechar o par por `+` e `×` dá `2^lcm` exato; **só com o `+`** para em `16 = 2⁴` com `4∤6`, e nem chega a ser corpo |

## A aplicação — a fala (Tiffany)

| arquivo | o que colhe |
|---|---|
| `linear.c`   | lineariza o sinal — um texto é um sinal como outro qualquer. *Pede um `.pgm`.* |
| `orbitas.c`  | o corpus vira um **grafo de órbitas** — frases de mesma estrutura, mesmo atrator |
| `navega.c`   | o navegante percorre os caminhos que passam pela fala — cobertura `100%`, reversível |
| `navegante.c`| a busca **sem Metrópolis** — a recursão (a fração contínua desdobrada), backtrack, resíduo 0 |
| `traducao.c` · `rotacao.c` | a tradução como **rotação em classes**: um par fixa o multiplicador, e o dente (um `k` errado) quebra |
| `../tatoeba/regua.c`| **as duas réguas**: PT e EN não se contêm (`|V_pt|/|V_en| = 1,710` sobre o mesmo conteúdo) e o corpus **não é função** em direção nenhuma (21,7% das frases EN têm 2+ PT, até 52; 6,9% das PT têm 2+ EN). A **amputação** tem preço: 41 palavras EN cobrem 50% dos tokens, e 30% do léxico aparece uma vez e paga 0,32%. A **âncora é órbita**: 758 verbos validados pela família de 1287 candidatos, em 77,7% dos pares. *Pede `pares.tsv`.* |
| `../tatoeba/centro.c`| **o centro é o interlocutor**: PT→centro→EN, e no centro o verbo é invariante — 55,4% das formas chegam ao mesmo parceiro da órbita contra 34,6% do acaso. Duas hipóteses minhas caíram medidas (massa ≠ ambiguidade; Dice não compara granularidades). O significado é da palavra **com a vizinhança**: pureza `0,629 → 0,701`, e em 54,5% das ocorrências a vizinhança muda a resposta (`vendo`→sell/see, `saia`→skirt/sair, `que`→that/why). E o **teste cego** (10% fora do aprendizado): 65,8% cascata > 62,7% forma > 54,9% órbita > 37,4% base. §C7 instancia a **BAI** (`broca-so/papers/casl-propagation.tex`): a entrada de dicionário é `κ=(σ,c,δ,ω,o)`, `δ` é o raio da amputação, `Ψ=Collapse` com **fail-closed** — e o dado justifica recusar (onde o estrito aceita, acerta 68,5%; onde recusa, 58,0%) com **0 escaladas** (`thm:noesc` exato). *Pede `pares.tsv`.* |
| `../tatoeba/bairro.c`| **a vizinhança É a órbita**: o bairro inteiro entra e quem escolhe é a contração `s(e)=a(e)·(m+Σ w(f)c(e,f))` — `σ=m+1/σ` iterado ao ponto fixo. Converge em **todas** as frases do cego; iteração 0 É a marginal, e o bairro paga `+0,4` ponto (66,5→66,9%) com a **mesma população** (115.871 decisões). Onde mexeu: troca 4,10% das escolhas e **ganha 1219 contra 772** (61,2%) — fiação **difusa**, não inerte. E o mesmo ponto fixo pelas duas vias: digital × translinear, `4000/4000` mesma escolha, `3,3e-16` de diferença. *Pede `pares.tsv`.* |
| `../tatoeba/`| o pipeline real sobre o Tatoeba: `ingestor`, `convtexto`, `caminha`, `dual`, `ciclo`, `ciclo_analog`, `tiffany` — e a série da tradução (`embedding`, `ancora`, `dente`, `homogeneo`, `operador`), cujo resultado é o §6 de `tiffany.tex` |

## Base histórica

| arquivo | nota |
|---|---|
| `micro.c` | o corpo no **discreto** (10 seções). O gabarito é hoje o analógico (`analog.c`); "discretização é inútil" — útil só como oráculo. |
