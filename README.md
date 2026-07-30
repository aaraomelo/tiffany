# tiffany — o corpo universal

Tudo aqui é um **corpo**, e ele é **autossimilar**: uma peça — o *gato* e o seu *esquilo* — que se
repete em toda escala e dimensão, sem nunca mudar. Dele se derivam a aritmética, a geometria, a
dinâmica e a linguagem. E o que costuma exigir maquinário — a *transformada*, a *convolução*, a
*deconvolução*, a *inversa* — aqui não se constrói: é **propriedade do corpo**, apenas **colhida**.

Cada afirmação é **medida na cifra** (código C puro, sem física nem parâmetro inventado), contra um
oráculo externo, com **resíduo 0** — exata, ou falha.

*Autor: Aarão Melo Lopes.*

## A peça

O gato é a matriz `A_m = [[m,1],[1,0]]` (det = −1). O seu ponto fixo é a **realimentação**:

```
σ = m + 1/σ
```

— σ dos dois lados, o sinal que volta: a raiz de `x²−mx−1`, o **metal** `σ_m` (m=1 ouro φ, m=2 prata,
m=3 bronze). Iterada, a mesma peça é o corpo **Rⁿ = GF(pⁿ)** em qualquer dimensão — a multiplicação é
recursiva, a dimensão `n` vem da `n−1`, o mesmo laço uma volta abaixo.

O gato tem dois atratores **duais**, os dois lados do corpo:

| | atrator | papel | operação |
|---|---|---|---|
| **negro** | `σ`, \|σ\|>1 | sorvedouro — a fala cai | convolução (`×σ`, sobe) |
| **branco** | `σ'=−1/σ`, \|σ'\|<1 | fonte — a resposta emana | deconvolução (`×σ'`, desce) |

com `σσ'=−1` (a mão que segura). O **esquilo** é o dual que traz de volta — e ele não se constrói,
**colhe-se da borda**: de `σⁿ = m·σⁿ⁻¹ + 1` sai `σ⁻¹ = σⁿ⁻¹ − m·σⁿ⁻²`, dois termos da própria
realimentação, sem Fermat e sem eliminação.

## A tese que organiza o resto

Uma **inversão**: a dimensão inteira **não é o objeto** — é um *ponto de ancoragem*. O que existe em
quantidade é o contínuo (os racionais têm medida nula, e quase todo corte é aperiódico), e a dimensão
inteira é onde esse contínuo encosta e trava. **Simetria ancora; deformação é a dinâmica.** E a peça
está *entre* os dois lados do corpo — não é progressão aritmética nem geométrica —, sendo por não
estacionar de nenhum lado que gera corpo em toda dimensão.

Dois corolários que o projeto mede:

- **Os rótulos trocam com a base.** "Inteiro", "racional" e "irracional" não são propriedades do
  número: `1/φ` é irracional em ℚ e é `(−1,1)` — inteiro — em `ℤ[σ]`. Cada dimensão é um irracional
  que **colapsa em inteiro** na passagem.
- **A assimetria que basta é a *anti*ssimétrica.** Sem assimetria não há nada — e a forma
  `ω(u,v) = −ω(v,u)` é a única que existe do mesmo modo em **toda** dimensão: uma só classe sempre
  (contra `≥2` da simétrica em `F_p`, `n+1` em `R`), zero em dimensão ímpar, e `nula` em `n=1`. Ela é
  `0` sobre um ponto e não-degenerada sobre pares — *carrega tudo e nada*: não mede ponto, mede
  **diferença**. Em `n=2` ela **é** `ℂ` (`J²=−I`), e dela sai a lei de potência (`det = Pf²`, módulo
  `|x|^d`).
- **π é o `0`.** *Ciclotomia* significa "corte do círculo": dividir o círculo em `n` devolve o centro
  (`Σζⁿ = 0`), e o circuito fecha (`ζⁿ=1`). Os metais são de π exatamente (`1/φ = 2cos(2π/5)`), e o
  que π alcança é a extensão **abeliana máxima** — o que fica fora é o não-abeliano, e o exemplo é o
  fator plástico do furo do ouro na dimensão 5.

## Os três papers

| paper | o quê |
|---|---|
| `teoria.tex` | **O Alfabeto e a Operação** — o corpo autossimilar `Rⁿ`, os rótulos e o gerador, as progressões, a dualidade, a deformação e o AGM, a colheita |
| `microprocessador.tex` | **o gabarito** — o circuito analógico, a peça translinear `ANTILOG(log a + s·log b − s·log ref)` (`s=+1` gato, `s=−1` esquilo), o gerador global, os terminais onde se colhe |
| `tiffany.tex` | **a fala** — a assistente: o corpus é um grafo de órbitas, a fala cai nele, e o caminho que passa por ela é a resposta; e o §6, onde a tradução por *embedding* exato **não fecha**, com a causa contada |

O `teoria.tex` está em sete seções: **§1** a peça · **§2** os rótulos e o gerador · **§3** as duas
progressões e o dupolinômio · **§4** o corpo `Rⁿ` · **§5** a dualidade · **§6** a dimensão é ancoragem
· **§7** a colheita.

## A cifra — o código

Em `tools/`, cada arquivo mede uma afirmação e devolve **resíduo 0**. C puro (libc + libm):

```
cc -O2 tools/checkup.c -lm -o /tmp/checkup && /tmp/checkup
```

Para rodar **tudo**: a bateria extrai a lista dos próprios papers, compila e roda cada medidor sob teto
de memória e `timeout`:

```
./tools/bateria.sh
```

**53 medidores — 51 verdes, 2 negativos por projeto, 0 falhas.** Os dois negativos
(`tatoeba/ancora.c`, `tatoeba/homogeneo.c`) devolvem `1` porque *provam* que o sistema da tradução não
tem solução — é resultado, não quebra.

A peça vive **uma vez** em cada header e é reusada por `#include` — a auto-similaridade posta no próprio
código: `gp2.h` (GF(p²)), `gf2n.h` (GF(2ⁿ)), `quat.h` (ℍ=M₂), `pgm.h` (imagens). O índice completo, seção
por seção, está em [`tools/LEIAME.md`](tools/LEIAME.md).

### O gerador, e por que ele vai escrito

O gerador não é invenção deste projeto — é o do **corpo universal**, e basta recolhê-lo: um primo `p` com
`n | p−1`, o **menor** gerador `g`, a torção `w = g^((p−1)/n)` (cuja órbita *é* a base, que portanto não
se escolhe) e a normalização `r² = n`. Para texto em bytes: `p=40961, g=3, w=36043, r=16`.

A única propriedade que a construção usa é `χ_k(u+v) = χ_k(u)·χ_k(v)` — o caractere leva `⊕` a `⊗` — e a
torção **basta** porque o dual do dual devolve o grupo: as inversas (da transformada, da convolução, o
flip) são consequências de **uma** razão, não três teoremas. E há **um** gerador, com projeções
encadeadas `w_d = g^((p−1)/d)`, que se abrem de baixo nas ordens que a necessidade pede.

### O neurônio, e o AGM — a mesma peça em dois meios

`tools/neuronio.c` (bits) e `tools/neuronio_analog.c` (correntes) são a **mesma peça** em dois meios: o
sinal parte em fases (`⊕`), soma-se cada uma (`∑`, Kirchhoff) e o gato sobe a torre — temporal (`Aᵏ`, a
razão → `σ_m`) e dimensional (`Aₙ` em `R²⊂R³⊂⋯`) —, o esquilo desce. Os dois dão o mesmo resultado
(`tools/fractal.c`, resíduo 0). *O bit era a amputação, o corpo é a onda.*

O mesmo vale para o **AGM**, que é a tríade batendo alternada (`⊕` e `⊗`) e cujo invariante é uma
integral: `tools/agm.c` (exato) e `tools/agm_analog.c` (colhido no circuito, onde a média geométrica é o
translinear com somador em ganho ½ — e **dispensa a corrente de referência**). E a sua dinâmica **é** a
do gerador: conjugado por `h : k ↦ τ = K′/K`, o AGM é `τ ↦ 2τ`, e a torre da torção é `w_d = (w_{2d})²`
— nos dois lados, a multiplicação por 2 no grupo (`tools/agm_gerador.c`).

## Estrutura

```
teoria.tex · tiffany.tex · microprocessador.tex   os três papers
tools/       a cifra (.c + .h) e bateria.sh — tudo resíduo 0 ou falha
tatoeba/     o pipeline da assistente sobre o corpus Tatoeba
hardware/    a placa so_cristal (Gerbers, KiCad, DRC 0 violações)
```

Os PDFs dos papers e os binários compilados são regenerados localmente (ver `.gitignore`); os dados
grandes do corpus (`tatoeba/*.tsv`) não são versionados.

---

> *"Não se implementa a matemática; colhe-se o que o circuito, sendo o corpo, já faz."*
