# A cifra — o circuito autossimilar, e o que dele se colhe

Cada arquivo é **uma peça só, medida**: mede uma afirmação contra um oráculo externo e
devolve **resíduo 0** ou falha. Não se implementa a matemática — o circuito, sendo o corpo,
já faz; aqui apenas se **colhe** e se confere. C puro (libc + libm):

```
cc -O2 <arquivo>.c -lm -o <arquivo> && ./<arquivo>
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
