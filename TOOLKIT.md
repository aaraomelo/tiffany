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
