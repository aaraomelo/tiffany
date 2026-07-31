# O toolkit — sempre foram três operações

Todo corpo deste projeto tem a **mesma estrutura**, e ela cabe numa linha:

```
⊕  Clifford     a soma        associa, comuta, tem neutro
⊗  La Hire      o produto     distribui sobre ⊕
∏  Pontryagin   o operador    costura — é ele que liga um corpo ao outro
```

Muda **o que são**, não **quantos são**. É por isso que a mesma peça serve o corpo, a cifra, a
transformada e a máquina — e é por isso que isto é um toolkit e não uma coleção de bibliotecas.

O mapa completo dos 29 corpos está em `CORPOS_NA_ISA.md`, trazido do catálogo. Este documento é
o que está **implementado e medido aqui**.

---

## Como usar

```c
#include "corpos.h"      /* em tools/ */
```

O tipo partilhado é `Par { long a, b; }` — que é a `Word` da ISA, `{total, e}`. Todo corpo opera
sobre ele, e é essa partilha que faz as operações comporem.

---

## Os quatro que fecham

### Áureo ℤ[φ] — o corpo do rei

O elemento é `a + bσ`, com a borda `σ² = mσ + 1`. O `m` é o metal: 1 é ouro, 2 prata, 3 bronze.

| | assinatura | o que é |
|---|---|---|
| ⊕ | `au_soma(x, y)` | componente a componente — a mesma em toda dimensão |
| ⊗ | `au_prod(x, y, m)` | o produto pela borda: o `σ²` desce sempre a grau 1 |
| ∏ | `au_op(x, m)` | `×σ`: o gato `(a,b) ↦ (ma+b, a)` — **é `cifra_an` da ISA** |
| — | `au_norma(x, m)` | `a² + mab − b²`, e ela é **multiplicativa** |

**O invariante:** a norma é `(−1)^k` em toda potência de `σ`, exatamente. O ponto cresce sem parar
e nunca sai da hipérbole — *crescer não é cair*.

Medido em: `coroa.c`, `familia_real.c`, `normal_circulo.c`, `densidade.c`, `disco.c`.

### Racional ℚ — o corpo das classes

O elemento é o par `(num, den)`, e o objeto **é a classe**: `(a,b)` e `(ka,kb)` são o mesmo ponto.

| | assinatura | o que é |
|---|---|---|
| ⊕ | `ra_soma(x, y)` | Clifford cruzado: `(ad+bc, bd)` |
| ⊗ | `ra_prod(x, y)` | La Hire componente a componente: `(ac, bd)` |
| ∏ | `ra_classe(x)` | reduzir pelo mdc — o representante único |
| — | `ra_cmp(x, y)` | a ordem por multiplicação cruzada, **sem uma única divisão** |

**O invariante:** nada se arredonda porque nada se divide. E há um **teto**: com `p` e `q` primos
entre si a redução não cancela, e o denominador cresce como `q^k` — exato até onde a palavra chega.

Medido em: `racional_pg.c`, `rastro.c`, `tudo_ouro.c`.

### Mórfico — o corpo dos conjuntos

O elemento é uma máscara de bits. Vem do catálogo já certificado (36/36).

| | assinatura | o que é |
|---|---|---|
| ⊕ | `mo_soma(A, B)` | XOR — a **deflexão pela metade** `D₁(x) = x⊕1` |
| ⊗ | `mo_prod(A, B)` | AND — **a erosão mórfica É o produto** |
| ∏ | `mo_dil` / `mo_ero` | a **adjunção** `δ⊣ε`, com `δεδ=δ` e `εδε=ε` |

**O invariante:** `γ = δε` e `φ = εδ` são idempotentes — e é isso que faz a **absorção**
`(x∧y)∨x = x`, que o `WHERE` do SQL usa para simplificar.

**Cuidado que custa caro:** em característica 2 a reflexão `ν = −1` **colapsa na identidade**. A
involução verdadeira é a complementação `¬ = D₁`, que é *translação* e não endomorfismo. E quem
conjuga `δ` em `ε` é a **antípoda na máscara** `B̌ = −B`, não a complementação. São duas involuções
distintas.

Medido em: `morfico.py` (36/36, resíduo 0).

### Mecânico — o corpo dos movimentos

O elemento é uma matriz `2×2` sobre o par. **Toda operação num vetor de dois é uma matriz.**

| | assinatura | o que é |
|---|---|---|
| ⊕ | soma de matrizes | componente a componente |
| ⊗ | `me_prod(X, Y)` | o produto — **compor operações é multiplicar matrizes** |
| ∏ | `me_ap(M, v)` | aplicar ao par |
| — | `me_rot()`, `me_cis(k)`, `me_gato(m)` | rotação, cisalhamento, a cifra |

**O invariante:** `det` é multiplicativo e vale `±1` — logo tudo é **reversível em inteiros**.

**E a peça que isto abre:** toda matriz de `det ±1` é uma **palavra** nos geradores `S` (rotação) e
`T` (cisalhamento) — que a ISA já tem. Então a emissão pode ser uma sequência de opcodes **sem
multiplicação nenhuma em tempo de execução**: o compilador compõe, a máquina aplica.

Medido em: `mecanica.c`.

---

## A regra de entrada

Um corpo **só entra aqui quando as três operações estão implementadas e há medidor a fechá-las**.

Assinatura sem conta é catálogo, não ferramenta. Os 25 restantes do `CORPOS_NA_ISA.md` estão
**descritos e não implementados** — fractal, criativo, eletromagnético, motor, telescópico,
cristalino, conforme, entrópico, espaço-temporal, óptico, celeste, econômico, evolutivo, expansivo,
somático, geométrico, técnico, rotor, cósmico, e o resto. A tríade de cada um está no mapa.

Para acrescentar um:

1. ler a tríade dele no `CORPOS_NA_ISA.md`
2. implementar `⊕`, `⊗`, `∏` em `tools/corpos.h`
3. acrescentar uma secção ao `tools/toolkit.c` que meça: `⊕` associa, `⊗` distribui sobre `⊕`, e o
   invariante próprio do corpo
4. citar no paper — senão a bateria não o roda
5. e escrever aqui, nesta tabela

---

## O que isto NÃO promete

- **Sigilo.** A cifra do áureo é linear: a segurança é a *dimensão da chave*, não uma suposição de
  dificuldade. É o regime da pastilha, e é um modelo limpo — mas não é dificuldade computacional.
- **Exatidão sem teto.** O racional é exato até ao teto da palavra. Com `101/100` são quatro passos
  garantidos pela guarda conservadora.
- **Cobertura dos 29.** São quatro. Os outros estão no mapa e ficam ditos como não implementados.

---

# O catálogo em SQL — o desenho

**Pedido em 30/07/2026, não implementado.** Fica escrito para começar da forma e não do zero.

A ideia: o SQL é a **interface final**, e cada coluna declara em que **corpo** vive. As operações
do `WHERE` deixam de ser aritmética e passam a despachar para a tríade daquele corpo.

```sql
CREATE TABLE t (a RACIONAL, b AUREO(1), c MORFICO(6))
```

E aí:

| no SQL | despacha para | no corpo |
|---|---|---|
| `a + b` | `⊕` do corpo da coluna | Clifford |
| `a * b` | `⊗` | La Hire |
| `a = b` | `∏` para a forma canónica, depois compara | Pontryagin |
| `a AND b` | `⊗` do mórfico | a erosão |
| `a OR b` | a dilatação | `δ` |

## O que já está pronto para isso

- **o tipo partilhado.** Todo corpo opera sobre `Par {a,b}`, que é a `Word` da ISA. A coluna já é
  um par no disco — não é preciso mudar o formato, só saber *qual corpo* interpreta aquele par.
- **o despacho tem onde morar.** O catálogo (`S_CAT`) já guarda `ncols` e `nrows`; ganha um campo
  por coluna dizendo o corpo, como ganhou o `S_Q`.
- **a árvore do WHERE já é morfologia.** `AND`/`OR`/`XOR` já são erosão/dilatação/deflexão, e a
  absorção já está ligada. O corpo mórfico é o único que **já está implementado no SQL** — sem eu
  saber que era ele.
- **e a emissão tem o caminho.** `mecanica.c` mostra que toda operação é matriz de `det ±1` e toda
  matriz é palavra nos geradores da ISA. O despacho por corpo produz a matriz; a matriz vira
  palavra; a palavra vira opcodes. Sem multiplicação em tempo de execução.

## A ordem de fazer, do que fecha primeiro

1. ~~**o campo do corpo no catálogo** e o `CREATE TABLE` a aceitá-lo~~ — **FEITO em 30/07**
2. ~~**racional**, que já opera no SQL: só passar a despachar em vez de assumir~~ — **FEITO em 30/07**
3. **áureo**, que precisa de `⊗` pela borda — e a borda depende do metal `m` da coluna
4. **mórfico**, que já está lá disfarçado de `AND`/`OR` — é reconhecê-lo, não construí-lo
5. **mecânico**, que é o que substitui a emissão inteira
6. e só então os 25 do mapa, um a um, cada um com medidor antes de entrar

## O que vigiar, porque já mordeu

- **não serializar o que contrai.** Três tentativas hoje a emitir termo a termo, e a solução foi
  sempre uma contração só.
- **os dois lados na mesma régua.** Comparar coordenada com magnitude nunca fecha, e mascarar a
  diferença esconde em vez de resolver.
- **medir antes de levar ao `sql.c`.** As peças que mediram primeiro (`tudo_ouro`, `mecanica`)
  fecharam; as que foram direto ao compilador foram revertidas.

---

## Progresso

### Passo 1 — o corpo da coluna no catálogo ✔ 30/07/2026

```sql
CREATE TABLE t (a RACIONAL, b AUREO(2), c MORFICO(8), d)
→ tabela t criada: 4 colunas — RACIONAL AUREO(2) MORFICO(8) INTEIRO
```

O slot `S_CORPO + j` guarda `{total = código do corpo, e = parâmetro}` — o metal `m` no áureo, o
`n` no mórfico. **O tipo é opcional**: sem ele a coluna é `INTEIRO`, e nenhuma base antiga muda.

Quatro asserções no `sql teste`, e a bateria cobre-as: a coluna racional guardada como tal, o
`AUREO(2)` a guardar corpo *e* metal, o `MORFICO(8)` idem, e o sem-tipo a continuar inteiro.

*Ainda não despacha nada* — é só o campo. O despacho vem no passo 2.

### Passo 2 — o racional pelo toolkit ✔ 30/07/2026

O `sql.c` passa a `#include "corpos.h"`, e:

- **a entrada** usa `ra_classe` — a redução deixa de estar escrita à mão no INSERT. Uma
  implementação, não duas: é a mesma que o `racional_pg.c` mediu.
- **a saída despacha pelo corpo declarado** da coluna. Hoje só o racional tem forma própria; os
  outros caem no inteiro — e é para isso que o campo do passo 1 passa a servir.

Quatro asserções: `6/8` entra reduzido a `3/4`, `-2/6` vira `-1/3` com o sinal no numerador, o
inteiro fica com denominador 1, e a saída consulta o corpo.

**Escopo dito com precisão:** a aritmética do `WHERE` é *código emitido*, não chamada C — despachar
`ra_soma`/`ra_prod` para o toolkit ali não se aplica, porque ali não há chamada. O que passou pelo
toolkit foi o **lado C** (entrada e saída). O lado emitido é o passo 5, o mecânico, onde a operação
vira matriz e a matriz vira palavra.

### Passo 3 — áureo, com a borda dependente do metal da coluna

O `⊗` do áureo precisa de `σ² = mσ + 1`, e o `m` é o parâmetro que a coluna já declara
(`AUREO(2)`). A entrada e a saída passam por `au_*`; a comparação precisa da norma.
