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

com `σσ'=−1` (a mão que segura). O **esquilo** é o dual que traz de volta: `gato ∘ esquilo = id`.

## Os três papers

| paper | o quê |
|---|---|
| `teoria.tex` | **O Alfabeto e a Operação** — o corpo autossimilar Rⁿ, a multiplicação recursiva, a dualidade, a colheita; Fermat cai no gato (uma identificação, não prova nova) |
| `microprocessador.tex` | **o gabarito** — o circuito analógico, a peça translinear `ANTILOG(log a + s·log b − s·log ref)` (`s=+1` gato, `s=−1` esquilo), os terminais onde se colhe |
| `tiffany.tex` | **a fala** — a assistente: o corpus é um grafo de órbitas, a fala cai nele, o caminho que passa por ela é a resposta |

## A cifra — o código

Em `tools/`, cada arquivo mede uma afirmação e devolve **resíduo 0**. C puro (libc + libm):

```
cc -O2 tools/checkup.c -lm -o /tmp/checkup && /tmp/checkup
```

A peça vive **uma vez** em cada header e é reusada por `#include` — a auto-similaridade posta no
próprio código: `gp2.h` (GF(p²)), `gf2n.h` (GF(2ⁿ)), `quat.h` (ℍ=M₂), `pgm.h` (imagens). O índice
completo está em [`tools/LEIAME.md`](tools/LEIAME.md).

### O neurônio — o circuito fractal, digital e analógico

`tools/neuronio.c` (bits) e `tools/neuronio_analog.c` (correntes) são a **mesma peça** em dois meios:
o sinal parte em fases (`⊕`), soma-se cada uma (`∑`, Kirchhoff) e o gato sobe a torre — **temporal**
(`Aᵏ`, a razão → `σ_m`) e **dimensional** (`Aₙ` em `R²⊂R³⊂⋯`) —, o esquilo desce. Os dois dão o mesmo
resultado, coordenada a coordenada (`tools/fractal.c`, resíduo 0). *O bit era a amputação, o corpo é a
onda.*

## Estrutura

```
teoria.tex · tiffany.tex · microprocessador.tex   os três papers
tools/       a cifra (scripts .c + headers .h), tudo resíduo 0
tatoeba/     o pipeline da assistente sobre o corpus Tatoeba
hardware/    a placa so_cristal (Gerbers, KiCad, DRC 0 violações)
```

Os PDFs dos papers e os binários compilados são regenerados localmente (ver `.gitignore`); os dados
grandes do corpus (`tatoeba/*.tsv`) não são versionados.

---

> *"Não se implementa a matemática; colhe-se o que o circuito, sendo o corpo, já faz."*
