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
3. ~~**áureo**, que precisa de `⊗` pela borda — a borda depende do metal `m` da coluna~~ — **FEITO em 30/07**
4. ~~**mórfico**, que já está lá disfarçado de `AND`/`OR` — é reconhecê-lo, não construí-lo~~ — **FEITO em 30/07**
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

### Passo 3 — o áureo ℤ[φ] ✔ 30/07/2026

```sql
CREATE TABLE t (a AUREO(1), b RACIONAL, c)
INSERT INTO t VALUES (3+2s, 1/2, 7)
→   3+2σ | 1/2 | 7
    5    | 1   | 9      ← "5" sozinho é o inteiro 5, não 5+σ
    0-1σ | 2   | 1
```

**O par é o mesmo; o que muda é o que ele significa** — e quem diz é a coluna. No racional
`(a,b)` é `a/b`; no áureo é `a + bσ`. Até o *padrão* muda com o corpo: no racional o segundo
componente predefine `1` (denominador), no áureo predefine `0` (sem parte σ).

Quatro asserções, e a última é a que importa: **a norma é multiplicativa no que foi guardado**.
Não se afirma só que o armazenamento funciona — afirma-se que o **invariante do corpo** sobrevive
ao armazenamento.

O metal viaja na coluna: `AUREO(2)` leva o `m=2`, e é dele que a borda `σ² = mσ + 1` vem.

**Mesmo escopo do passo 2:** lado C, entrada e saída. A comparação de áureos no `WHERE` precisa da
norma e ainda não despacha — é o passo 5.

### Passo 4 — o mórfico ✔ 30/07/2026 (por descoberta)

```sql
CREATE TABLE t (a MORFICO(6), b MORFICO(4), c)
INSERT INTO t VALUES (13, 3, 7)
→   {0,2,3}       | {0,1}     | 7
    {}            | {0,1,2,3} | 8      ← o vazio
    {0,1,2,3,4,5} | {}        | 9      ← o topo
```

**Um elemento mórfico é um conjunto**, e a coluna passa a mostrá-lo como conjunto em vez de como
número. O `n` viaja na coluna e é ele que diz qual é o universo.

E o invariante que se afirma é o que **distingue este corpo de todos os outros**: `A ∧ A = A`,
todo elemento é idempotente. É por isso que ele só é corpo quando `n = 1` — com `n > 1` há divisor
de zero e elemento sem inverso (`morfico.py`, `teo:socorpon1`). Mais: a erosão é o produto, e
`A ∧ B ⊆ A` — ela só tira.

**Por que "por descoberta":** o `AND`/`OR` do `WHERE` já eram a erosão e a dilatação, e a absorção
já estava ligada desde a adjunção. O corpo já operava; faltava a coluna reconhecê-lo. Foi o único
passo em que não construí nada — só dei nome ao que já estava a funcionar.

### Passo 5 — o mecânico: primeira pedra assente ◐ 30/07/2026

**A máquina aplica uma matriz como opcodes, e verificado no metal.** Emite-se a palavra, a máquina
corre, e compara-se com o que a matriz daria pelo `corpos.h`:

```
m   k   entrada   pela máquina   pela matriz
1   1   (3,2)     (5,3)          (5,3)        ✓
1   2   (3,2)     (8,5)          (8,5)        ✓
3   6   (3,2)     (4287,1298)    (4287,1298)  ✓
```

**E o gerador da ISA não é o cisalhamento — é o GATO.** `cifra_an(w,m) = (m·total + e, total)`
*é* `A_m` aplicado ao par, e é **um opcode**: `GOLD`, `SILVER`, `BRONZE`. Aplicar `A_m^k` é
repetir o opcode `k` vezes, **sem multiplicação nenhuma**.

Isto é a pedra, não a parede. O que falta: trocar o `emit_atomos` para emitir palavras em vez de
aritmética — e isso é reescrever a emissão inteira, o maior pedaço dos seis.

**O que já está pronto para essa troca:**
- a matemática, medida (`mecanica.c`): toda unimodular decompõe, e aplicar a palavra = aplicar a
  matriz
- o gerador certo, identificado: o gato, e ele é um opcode
- e agora a ponte: a máquina a executar a palavra, conferida contra o toolkit

**O que falta e é o trabalho:** a expressão do `WHERE` contrair numa matriz (hoje contrai num
tensor), a matriz decompor-se na palavra, e o `emit_atomos` emitir a palavra. Ver "o que vigiar".

### Passo 5 — segunda pedra: a cadeia de minerais, e a volta pelo negro ◐ 30/07

**Correção que desmonta o `mecanica.c` num ponto:** ele decompõe em `T`, o cisalhamento — e **`T`
não é opcode**. Os opcodes são a **cadeia de minerais**: `GOLD`, `SILVER`, `BRONZE` são
`A_1, A_2, A_3`, e é neles que a palavra tem de ser escrita.

E a volta é **pelo negro**: `det(A_m) = −1`, logo a inversa `A_m⁻¹ = [[0,1],[1,−m]]` é **inteira**,
e desfazer a ida é aplicar as inversas na ordem contrária. Medido no metal, com cadeias até quatro
elos misturando ouro, prata e bronze:

```
cadeia     ida          volta     fecha?
ouro       (8,5)        (5,3)     ✓
prata⁴     (181,75)     (5,3)     ✓
```

Fecha **exato**, e fecha porque `det = −1` — a inversa é inteira, não é reconstrução.

**E o que falta, dito:** a ida é opcode; a **volta ainda passa pelo toolkit**, porque a ISA não tem
o opcode da inversa. Para o percurso ser inteiro no metal falta esse opcode — ou compor a inversa
na própria cadeia, que é o que o esquilo faria se também fosse opcode.

*Isto é a lacuna concreta entre a pedra e a parede, e ela é da ISA, não do compilador.*

---

### Passo 5 (o resto) — a emissão inteira

O que falta dos passos 2, 3 e 4: eles fizeram o **lado C** (entrada e saída). A comparação no
`WHERE` ainda não despacha por corpo — e o passo 5 é o que resolve isso de uma vez, porque no
mecânico *a operação vira matriz e a matriz vira palavra nos geradores da ISA*. Não é despachar
aritmética: é trocar aritmética por movimento.

Medido em `mecanica.c`. O que vigiar está na secção acima — não serializar o que contrai.

### Passo 6 — o cristalino: o lado que gira ✔ 30/07/2026

```sql
CREATE TABLE k (a CRISTALINO(0), b CRISTALINO(1), c AUREO(1))
INSERT INTO k VALUES (3+2s, 1+1s, 3+2s)
→ tabela k criada: 3 colunas — CRISTALINO CRISTALINO(1) AUREO(1)
```

**O toolkit tinha quatro corpos e todos do mesmo lado.** Áureo, racional, mórfico e mecânico —
nenhum deles com operador que gire e volte. O catálogo tem esse lado e sempre teve: o
**cristalino**, ℚ(√D) com D < 0. Gauss ℤ[i] e Eisenstein ℤ[ω].

| | ⊕ | ⊗ | ∏ | det | disc | ordem | posto |
|---|---|---|---|---|---|---|---|
| **ÁUREO ℤ[φ]** | componente | borda σ² = mσ + 1 | ×σ, o **gato** | −1 | m²+4 > 0 | ∞ | 1 |
| **CRISTALINO ℤ[ω]** | componente | borda ω² = tω − 1 | ×ω, o **esquilo** | +1 | t²−4 < 0 | 4, 6 | 0 |

**A tríade é a mesma — e é isso que faz toolkit e não coleção.** O que muda é **um sinal na
borda**, e dele sai tudo o resto: o determinante, o discriminante, a norma alternar ou ser sempre
positiva, a ordem ser infinita ou finita, o posto ser 1 ou 0.

`t = 1` dá ordem **6**: é o Φ₆ que o `trono.c` encontrou sentado no buraco de n=5. Não é corpo
escolhido por gosto — é o ocupante do trono a chegar ao toolkit pela porta da frente.

**O preço, medido e exato** (`cristalino.c §X6`): só passam ordens {1,2,3,4,6}, e o **primeiro
proibido é n=5, o áureo**. O cristal proíbe exatamente o preenchedor ótimo. O que fecha não
preenche.

**E a VOLTA entrou junto.** `me_troca`, `me_antigato` e `me_inv`: `A_m⁻¹ = J·A_{−m}·J`, inteira,
porque `det = −1`. Não é uma segunda máquina — é a antípoda (`m ↦ −m`) conjugada pela involução.
Um processo reversível de que só se tem a ida não é reversível, é uma promessa.

Nove asserções no `sql teste` (37 no total), e o `cristalino.c` com oito unidades: a tríade
inteira, a ordem finita contada no metal, a norma positiva contra a alternante do áureo, e o
posto 0 medido por caixas que crescem sem que a contagem cresça.

### Passo 5 — terceira pedra: o opcode da inversa ✔ 30/07/2026

**O que estava travado:** "a ida é opcode; a volta ainda passa pelo toolkit, porque a ISA não tem
o opcode da inversa." Um percurso reversível de que só se tem a ida no metal não é reversível — é
uma promessa.

```
NEGRO_OURO / NEGRO_PRATA / NEGRO_BRONZE
    A_n⁻¹ = J·A_{−n}·J = [[0,1],[1,−n]]        (a,b) ↦ (b, a − n·b)
```

**Inteira, e sem uma única divisão** — porque `det A_n = −1`. Um opcode que precisasse de dividir
não caberia nesta máquina.

```
metal    opcode          par     ida        volta      desfaz?
ouro     NEGRO_OURO      (5,3)   (8,5)      (5,3)      sim ✓
prata    NEGRO_PRATA     (5,3)   (13,5)     (5,3)      sim ✓
bronze   NEGRO_BRONZE    (5,3)   (18,5)     (5,3)      sim ✓
```

675 pares por metal, nos dois sentidos (metal→negro→metal e negro→metal→negro). A cadeia de
minerais passa a ir **e voltar no metal**, e o toolkit passou de executor a **testemunha**: ele
confere e concorda, mas quem executa é a máquina.

**O que destravou não foi engenharia.** Foi saber *o que a inversa é* — a antípoda (`n ↦ −n`)
conjugada pela involução `J` — em vez de a tratar como uma segunda máquina. A reversibilidade não
foi acrescentada à ISA: já estava no determinante, e só faltava escrevê-la.

Os três códigos entram no **fim** do enumerado, de propósito: o número de cada opcode antigo não
muda e nenhum programa já compilado passa a significar outra coisa.

**Continua aberto do passo 5:** o `emit_atomos` emitir palavra em vez de aritmética. A inversa
deixou de ser o bloqueio.

### O circuito fechado ✔ 30/07/2026

**O que faltava não era mais um opcode: era o GRUPO.** A ISA tinha o gato, e o gato só *estica* —
`det −1`, hiperbólico, ordem infinita. Uma máquina que só estica não gera o grupo unimodular:
falta quem **gire**. E quem gira entrou pelo cristalino.

```
GOLD      A_1 = [[1,1],[1,0]]    det −1   ordem ∞   estica
ESQUILO   S   = [[0,−1],[1,0]]   det +1   ordem 4   gira    ← ×ω do cristal, t=0
TROCA     J   = [[0,1],[1,0]]    det −1   ordem 2   reflete ← a involução
```

`⟨S,T⟩ = SL₂(ℤ)` com **`T = A_1·J`** — o cisalhamento não precisa de opcode, é palavra de dois. Com
`J` junto, `GL₂(ℤ)` inteiro.

**Todo metal é palavra**, medido no metal (507 casos):

```
metal    opcode     palavra em ouro+troca            igual?
ouro     (8,5)      GOLD                             sim ✓
prata    (13,5)     GOLD TROCA GOLD                  sim ✓
bronze   (18,5)     GOLD TROCA GOLD TROCA GOLD       sim ✓
```

Ouro, prata e bronze têm opcode por serem os três primeiros, não por serem especiais. O
quadragésimo metal corre na mesma máquina, com palavra mais longa e sem uma multiplicação.

**E toda inversa está dentro:** o gato pelo negro, o esquilo por `S³`, a troca por si própria. O
esquilo e a troca nem precisaram de opcode de volta — por terem ordem **finita**, a inversa é a
própria peça repetida. Só o gato precisou, por ser o único que não fecha por repetição.

**Circuito fechado quer dizer isto e só isto:** o que a máquina faz, ela desfaz, nos inteiros e sem
guardar cópia. *Desfazer não precisa de memória; restaurar precisaria.*

**O que fica de fora, dito:** decompor uma unimodular **qualquer** em palavra não está no
compilador. Mede-se que existe para os metais (`§F3`) e para as inversas (`§F4`); para matriz
arbitrária o algoritmo é o de Euclides e não está aqui.

### O chicote dos dois lados ✔ 30/07/2026

**Correção do Aarão:** *"está generalizado ou não? falei pra usar o chicote todo dos dois lados,
são duais — por qual motivo o lado negro não está sendo usado todo se o branco está?"*

Ele tinha razão, e a assimetria era **minha, não do mecanismo**: eu generalizara o branco (`A_m`
para todo `m ≥ 1`) e deixara o negro nos três opcodes.

**A régua não tem lado.** `T⁻¹ = J·A_1⁻¹` é o espelho exato de `T = A_1·J` — a mesma palavra ao
contrário, cada letra invertida — e com ela `A_m = T^{m−1}·A_1` vale para `m ≤ 0` igualmente:

```
m     palavra                                  A_m no metal
−1    GOLD NEGRO TROCA NEGRO TROCA             (−2,5)   ✓
 0    GOLD NEGRO TROCA                         (3,5)    ✓
 4    GOLD TROCA GOLD TROCA GOLD TROCA GOLD    (23,5)   ✓
```

Medido de `m = −40` a `40` na álgebra, e de `−12` a `12` no metal — 3025 casos na ida, 3025 no
ida-e-volta, todos devolvendo o que entrou.

**`m = 0` dá `A_0 = J`:** a **troca é o metal do meio**, onde os dois lados da régua se encontram.
Não é peça que eu acrescentei — é onde o chicote passa ao mudar de sinal.

**A regra da volta, inteira e sem tabela:** reverter a ordem e trocar cada letra pela sua inversa.
O gato vai a negro, o negro vai a gato, e a troca fica onde está, por ser involução.

**O que se pode apagar:** `SILVER`, `BRONZE`, `NEGRO_PRATA` e `NEGRO_BRONZE` são **palavras, não
geradores**. O repertório mínimo que fecha é `GOLD`, `NEGRO_OURO`, `TROCA`, `ESQUILO` — quatro
peças, simétricas. Os outros quatro ficam por serem **atalhos**: poupam palavra, não poder. Fica
dito qual é qual, porque um atalho tomado por gerador faz pensar que a máquina precisa dele.

### Os atalhos apagados ✔ 30/07/2026

*"apaga os atalhos então, deixa só os quatro geradores."*

`SILVER`, `BRONZE`, `NEGRO_PRATA` e `NEGRO_BRONZE` saíram do enumerado e do despacho —
**apagados, não desativados**. A ISA tem agora exatamente quatro peças de metal:

```
GOLD          A_1  = [[1,1],[1,0]]    estica     det −1   ordem ∞
NEGRO_OURO    A_1⁻¹                   desfaz     det −1
TROCA         J    = [[0,1],[1,0]]    reflete    det −1   ordem 2
ESQUILO       S    = [[0,−1],[1,0]]   gira       det +1   ordem 4
```

No lugar dos quatro atalhos ficou **um par de emissores** no compilador:

```c
emit_metal(m, slot)       /* A_m   = T^{m−1}·A_1,  T   = A_1·J   */
emit_metal_inv(m, slot)   /* A_m⁻¹ = a mesma palavra ao contrário, letra a letra invertida */
```

que servem **todo `m ∈ ℤ`** — negativo, zero e positivo. A máquina não perdeu nada: deixou de ter
três nomes para a mesma peça.

```
metal    palavra nos geradores            A_m(5,3)   confere?
ouro     GOLD                             (8,5)      sim ✓
prata    GOLD TROCA GOLD                  (13,5)     sim ✓
bronze   GOLD TROCA GOLD TROCA GOLD       (18,5)     sim ✓
```

**Sexta vez que a solução certa apaga trabalho em vez de o acrescentar** — e desta vez apagou
instrução do processador.

### O catálogo completo ✔ 30/07/2026 — e ele é menor do que parecia

*"volta a trazer o catálogo de chess/ completo para o toolkit."*

A minha primeira ideia foi implementar os 25 corpos que faltavam, um a um. **Está errada**, e quem
o diz é o próprio catálogo — `chess/elementares/catalogo_isomorfismos.py` abre com a tese:

> quase todo corpo é o mesmo CORPO-MÃE vestido por uma RÉGUA diferente. E há só QUATRO
> transformações-tipo que ligam um corpo a outro.

Trazer o catálogo completo foi trazer **menos** código:

| seta | | o que faz | estado |
|---|---|---|---|
| **W** | Wick | o **sinal** da borda: `det −1 ↦ +1`, `disc m²+4 ↦ m²−4` | exata em ℤ — `ar_wick` |
| **ν** | nu | a antípoda `m ↦ −m` | exata em ℤ — `ar_nu` |
| **P** | Pontryagin | `⊕` vira `⊗`: `χ(u+v) = χ(u)·χ(v)` | exata em ℤ/p |
| **L** | Legendre | **não é isomorfismo** — o tropical perde o inverso | é limite, medido |

**W é a seta que usei a sessão inteira sem lhe saber o nome** — "o que muda é um sinal na borda".

**E dela sai a tricotomia**, que diz que as três peças do circuito não são escolha minha:

```
m     disc = m²−4   tipo          o que faz
−2    0             parabólico    desloca — o cisalhamento
−1    −3            elíptico      gira, ordem 3
 0    −4            elíptico      gira, ordem 4
 1    −3            elíptico      gira, ordem 6   ← o Φ₆ do trono
 2    0             parabólico    desloca
 3    5             hiperbólico   estica — o gato
```

O elíptico é **só** `m ∈ {−1,0,1}`, com ordens 3, 4, 6 — e nenhum outro m fecha. Com a identidade
e `−I`: **{1,2,3,4,6}**, a restrição cristalográfica pelo outro caminho (em `cristalino.c` saiu da
totiente, aqui sai do discriminante). Dois caminhos, um número.

**As três exceções**, cada uma com a sua razão — e são razões **distintas**:

| metade | falha em | perde | o seu dual |
|---|---|---|---|
| telescópico | `e₁·e₂ = 0`, divisor de zero | a integridade | **celeste** — é o cone nulo |
| entrópico | `max` nunca desce ao neutro | o grupo aditivo | **cósmico** — max+min = a+b |
| motor | `\|det\| ≠ 1`, a inversa sai de ℤ | a norma | **rotor** — o fixo de ν, tr=0 |

**O catálogo em classes:** MULTIPLICATIVA (P) → o caractere; HIPERBÓLICA (W,ν) → o gato;
ELÍPTICA (W) → o esquilo; SOMBRAS (L) e DISSIPATIVOS (ν) → não são corpos. Cabe em três classes e
duas exceções.
### As metades ✔ 30/07/2026

*"não tem essa de não é corpo. Falta o dual do telescópico — acho que é celeste; do entrópico é
cósmico; do motor é rotor."*

Ele tem razão, e **o catálogo dizia-o**. `cosmico.py`: *"o entrópico não é isomorfo ao cósmico — é
a sua **METADE**, uma polaridade do dipolo; a dualidade negro↔branco é a reflexão ν = −1."*
`certifica_corpos.py`: *"o conservativo (tr=0, det=1) é que **seria** corpo."* Eu li a primeira
metade das duas frases.

| metade ↔ dual | perde | o par devolve |
|---|---|---|
| entrópico ↔ **cósmico** | o oposto aditivo | `ν(max) = min`, e `max + min = a + b` |
| motor ↔ **rotor** | a conservação | `ν(t) = −t`, e o rotor é o **ponto fixo** `tr = 0` |
| telescópico ↔ **celeste** | a integridade | o divisor de zero **é** o cone nulo de `a²−b²` |

**O rotor não é um terceiro objeto:** é o **único autodual**. O motor dissipa (`tr<0`), o dual
amplifica (`tr>0`), somados dão zero — que é conservar.

**Telescópico e celeste são o mesmo objeto em duas bases:** em `(1,j)` com `j²=1` gira
hiperbolicamente com norma `N = a²−b²`; na base dos idempotentes cinde, e `N = α·β` — donde o
divisor de zero ser exatamente o cone nulo. *Chamar-lhe defeito é dizer que a luz é um defeito do
espaço-tempo.*

**E o que sobra fora do par tem uma marca só, em ℤ:** a volta existe no anel exatamente onde
`|det| = 1`. **É por isso que a ISA só tem peças de det ±1** — não por escolha de projeto, mas
porque fora dali não há volta.

O erro tem forma conhecida e vale mais que o resultado: **descrever uma peça pelo que lhe falta em
vez de pelo que ela é.** "Não tem oposto" é verdade sobre o polo e falso sobre o dipolo.
