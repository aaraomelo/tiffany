# ADVERSARIAL AUDIT — Fractal Core v2 Hypothesis (CORRECTED)

**Hypothesis under test (from theory papers):**
> Fractal Core v2 = cisão ⊕ + Σ(popcount) + gato ⊗ + esquilo ⊘ + MOVE

**Source documents (theory):**
- `papers/arquitetura.tex` — byte-level architecture, MOVE = fold, 4 operations in §1805-1824
- `papers/aranha.c` / `aranha.tex` — 8 laws on byte bits, algorithm lines 1785-1798
- `microprocessador.tex` — fractal microprocessor: triad ⊕⊗∫ + disk
- `papers/racionais.tex` — tower Z→Q, 8 laws across levels
- `papers/inteiros.tex` — N→Z, Word_8 envelope

**Baseline:** `dce95a66` / `fpga-postpnr-functional-baseline` — **intocável**

**Method:** Attempt to REFUTE using THEORY DOCUMENTS, not the `can/` 64-bit implementation.

---

## 1. Separating Claim A from Claim B

| Claim | Statement | Status in Theory |
|-------|-----------|------------------|
| **A** | "The stigmergic spider algorithm (`fis:neuronio` / `aranha`) uses four operations" | **PROVADA** — `arquitetura.tex` lines 1785-1798, 1805-1824 explicitly give the 2-line algorithm using {⊕, Σ, ⊗, ⊘} |
| **B** | "The universal ISA of the Microprocessor can be reduced to four primitives without losing expressivity/contract" | **NOT PROVEN** — the theory describes a **byte-level algorithm**, not a full ISA reduction |

**Critical distinction from `arquitetura.tex` lines 1805-1824:**
> "O corpo fixa o algoritmo em quatro operações... Aqui não se redemonstram: diz-se onde correm."

The four operations are **for the stigmergic spider algorithm** (the "neurônio"), explicitly mapped:
- ⊕ (cisão) → bitmap projection
- Σ (popcount) → `bits_conta` on field  
- ⊗ (gato × σ) → `cifra_an` = `OP_GOLD`
- ⊘ (esquilo × σ') → `decifra_an` = `OP_NEGRO_OURO`

**MOVE** is separate: "E o MOVE é esta dobra no metal" (`arquitetura.tex` line 81) — the fold with sign (+1 read, -1 write, 0 jump).

**Verdict:** A = PROVADA (for that specific algorithm). B = NÃO PROVADA — no theorem shows the full ISA reduces to these.

---

## 2. Minimal Real Set — Derivability Table (THEORY LEVEL)

**Theory ISA (byte-level, slots, SQL→ISA):**
From `arquitetura.tex`: SQL compiles to ISA bytecode running on `.mem` (disk). The ISA operates on `Word_8` slots.

| Operation | Derivable from ⊕ (cisão) | Derivable from ⊗ (gato) | Derivable from Σ (popcount) | Derivable from ⊘ (esquilo) | Needs MOVE | NOT Derivable |
|-----------|------------------------|------------------------|----------------------------|---------------------------|------------|---------------|
| **Cisão (⊕)** | primitive | — | — | — | — | — |
| **Popcount (Σ)** | — | — | primitive | — | — | — |
| **Gato (⊗)** | — | primitive | — | — | — | — |
| **Esquilo (⊘)** | — | — | — | primitive | — | — |
| **MOVE (fold)** | — | — | — | — | primitive | — |
| **Slot LOAD/STORE** | — | — | — | — | ✓ (definitional) | — |
| **SQL→ISA compile** | — | — | — | — | — | **YES** (external) |
| **Tower promotion** | — | — | — | — | — | **YES** (PROMOVE) |
| **Continued fraction I/O** | — | — | — | — | — | **YES** (ADC/DAC) |
| **Dual sort / max-cut** | — | — | — | — | — | **YES** (higher algo) |

**Key obstructions from theory:**

1. **PROMOVE / DESCE** (tower promotion/demotion) — `arquitetura.tex` lines 1262-1267, 1404-1408: requires `promove.h` with `S=(x+x†)/2`, `A=(x-x†)/2` — **not** in {⊕, Σ, ⊗, ⊘, MOVE}

2. **Continued Fraction ADC/DAC** — `microprocessador.tex` §I/O, `arquitetura.tex` §I/O: requires `adc` (division loop) and `dac` (matrix product of `GATO(a_k)`) — **not** in the 4 ops

3. **SQL → ISA compilation** — `arquitetura.tex` line 149: "O SQL traduz programas: a query compila para bytecode da ISA" — external compiler

4. **Dual Sort / Max-Cut** — `arquitetura.tex` §algorithm: requires estaca (involution), trial, biduality — higher algorithms **using** the 4 ops, not derived from them

5. **Stigmergic spider inverse** — `arquitetura.tex` lines 1619-1651: requires lifting `π → (π, k)` with `k(i)` = visit count — needs counter/state beyond the 4 ops

**Conclusion:** The 4 operations + MOVE **suffice for the spider algorithm core** (2 lines: popcount on masks, gato pair). They do **NOT** suffice for the full byte-level ISA + SQL + tower + I/O + dual sort.

---

## 3. Are the 4 Operations Primitive or Just an Algorithm?

**From `arquitetura.tex` lines 1785-1798 (the algorithm):**
```c
e = popcount(b & 0xAA)  // Σ on even bits (cisão ⊕)
o = popcount(b & 0x55)  // Σ on odd bits  (cisão ⊕)
out = [e+o, e]          // ⊗(e,o) = gato once
// o = (e+o) - e         // ⊘ recovers the other
```

**From `arquitetura.tex` lines 1805-1824 (where each lives):**
| Operation | What theory proves | Where it runs |
|-----------|-------------------|---------------|
| ⊕ cisão | realization π: I → X | bitmap: line i = coordinate i |
| Σ popcount | ∑ₓ G(x) = |I| | `bits_conta` on field |
| ⊗ gato × σ | goes up: convolution ζ | `cifra_an` = `OP_GOLD` |
| ⊘ esquilo × σ' | goes down: deconvolution μ | `decifra_an` = `OP_NEGRO_OURO` |

**From `microprocessador.tex` (triad):**
- ⊕ = soma (Kirchhoff currents at node)
- ⊗ = produto (log-domain: exp(ln a + ln b))
- ∫ = Pontryagin = exp ∘ Σ ∘ log (the middle operator)

**From `papers/aranha.c` (8 laws on byte bits):**
- Each bit feeds ONE law (lines 142-149): `phase[i] += (b >> i) & 1`
- **No popcount in aranha.c** — it counts bits per position across bytes, not popcount per byte
- The 8 laws are coordinates: e₀...e₇ ↔ Law 0...7

**Verdict:** The 4 operations {⊕, Σ, ⊗, ⊘} **are the algorithmic basis for the stigmergic spider core** (validated in `tests/neuronio.c`, `tests/aranha_g.c`). They are **NOT** a universal algebraic basis for the full ISA — they are **4 of the 8 laws** operating at the hexal interface level (Lei 6).

---

## 4. Audit: "MOVE is the only operation" (`arquitetura.tex` lines 81-87)

> "E o MOVE é esta dobra no metal. Ler e escrever não são duas instruções que se implementam juntas por economia: são **uma** operação e o seu dual — os dois lados do mesmo vinco —, e o que as separa é um **sinal**: +1 para ler, -1 para escrever."

**Classification:**

| Option | Assessment |
|--------|------------|
| A) Formal definition of machine | **PARTIAL** — defines data movement, but not ALU, control, tower, I/O |
| B) Normal form | **YES** — data movement normal form: LOAD=+1, STORE=-1, JMP=0 |
| C) Operational metaphor | **YES** — "one operation and its dual" is the central metaphor |
| D) Equivalent encoding to ISA | **NO** — ISA = MOVE + ALU(op) + tower ops + I/O + control |

**Formal status in theory:**
- MOVE = fold (`dobra`) with sign: `∂x = a+b-x` (`arquitetura.tex` line 56)
- The fold's fixed point is the middle; folding again gives induction
- MOVE implements the fold on the metal: read/write/JMP are the three signs
- **But** the ALU (triad ⊕⊗∫), tower (PROMOVE/DESCE), I/O (ADC/DAC), control (Z/n wheel) are **separate pieces** that also reduce to the same `A_m × G` family with different signatures (`microprocessador.tex` Proposition 1, lines 74-97)

**Verdict:** **"MOVE é a única operação" = INTERPRETAÇÃO / FORTE CANDIDATA (metáfora operacional central), NÃO definição formal completa.** The theory says: "Uma multiplicação, uma dinâmica; cada ofício é a mesma transformação lida sob uma assinatura" (line 95) — there is **one transformation** (`A_m × G`), not one operation (MOVE).

---

## 5. Audit: U = (X, X*, Mor, Aut)

**Claim in prompt:** `word[] = X`, `LOAD = X*`, `ALU = Aut`, `MOVE = Mor`

**Theory reality (`microprocessador.tex`, `arquitetura.tex`):**

| Component | Theory Object | Machine Object | Explicit Map | Preserved Property |
|-----------|--------------|----------------|--------------|-------------------|
| **X** | `Word_8` = F₈ (byte) | Slot (1 byte) | SlotWord = Word_8 (line 1457) | GF(2) structure |
| **X*** | Dual via involution ν | `LOAD` / retraction Σ | Retraction Σ∘Π=Id (line 1466) | Exact retraction (§T2) |
| **Mor** | MOVE = fold ∂ | MOVE(+1/-1/0) | Sign distinguishes fold sides | Reversibility (det=1) |
| **Aut** | Triad ⊕⊗∫ (A_m × G) | ALU + clock + bus | Same transformation, 5 signatures | |λ|=1 (edge) |

**Specific findings:**
- **X = Word_8** is explicit: "o átomo: Word_8 = 1 B/slot; GF(2)" (line 137)
- **X* = dual via involution ν** — `ν∘ν=id`, point fixed at boundary (lines 268-269)
- **Mor = MOVE = fold** — "E o MOVE é esta dobra no metal" (line 81)
- **Aut = the transformation family A_m × G** — "Uma multiplicação, uma dinâmica; cada ofício é a mesma transformação lida sob uma assinatura" (line 95)

**The category is real:** The theory constructs a category where:
- Objects: levels of the tower (F₂, F₈, N, Z, Q, R, Rⁿ)
- Morphisms: folds (dobras) with signatures
- The machine **REALIZES** this category in the τ=-1 pole (topological)

**BUT:** The mapping `word[] = X`, `LOAD = X*` etc. from the prompt is **NOT** what the theory says. The theory has:
- `SlotWord` = `Word_8` (1 byte/slot)
- Retraction `Σ∘Π=Id` between continued fraction and slots
- MOVE = fold with sign
- ALU = triad (one of 5 signatures of A_m × G)

**Verdict:** The machine **REALIZA U** (the categorical structure of the tower in τ=-1), but the specific mapping in the prompt is **INCORRECT**. The correct mapping is:
- X = Word_8 (byte, not word[4096])
- X* = dual via ν (involution), realized by retraction Σ∘Π=Id
- Mor = fold (dobra) = MOVE with sign
- Aut = the transformation family A_m × G (5 signatures: ALU, clock, disk, bus, I/O)

---

## 6. Audit: BYTE = X = B⁸

**Theory:** `arquitetura.tex` line 137: "byte level | o átomo: Word_8 = 1 B/slot; GF(2)"
**Theory:** `papers/racionais.tex` line 96: `F_8 ≡ Word_8 = {0,...,255}`
**Theory:** `papers/naturais.tex` (referenced): 3 folds F₁→F₂→F₄→F₈

**Machine (theory implementation):**
- `SlotWord` = `Word_8` = 1 byte/slot (line 1457)
- `(a,b)` pair = **two** slots (Lei 7, line 1458)
- Memory is disk (.mem), processor holds 1 byte in hand (line 262-263)
- ADC/DAC work on continued fractions, not binary place value

**Is there formal immersion Z₂⁶⁴ ≅ (B⁸)⁸?**
- **Theory doesn't use Z₂⁶⁴** — the 64-bit ISA in `can/` is a **different realization** (ERG-64)
- The theory's tower goes: F₂ → F₈ (Word_8) → N → Z → Q → R → Rⁿ
- At 64-bit level: `E₁₆` (16-bit pairs) → `D₃₂` → `D₆₄` → `I₁₂₈` (lines 1297-1312)
- Each level is a **dobra** (fold): `T_{k+1} = T_k + T_k*`, `d_{k+1} = 2d_k`

**Verdict:** **BYTE = X = B⁸ is PROVADA in the theory.** The machine's atom IS the byte (Word_8). The 64-bit implementation in `can/` is a separate realization at a higher tower level (D₆₄/I₁₂₈), not the base theory. The theory explicitly says: "Esta arquitectura é a SÍNTESE dessa cadeia no τ=-1: onde os cinco constroem o objecto, ela constrói a máquina que o executa — e cada peça dela tem o degrau de onde vem. O primeiro degrau é o byte" (lines 190-193).

---

## 7. POPCOUNT (Σ) Analysis

**From `arquitetura.tex` lines 1785-1798, 1819, 1837-1843:**
```c
e = popcount(b & 0xAA)   // Σ on even bits
o = popcount(b & 0x55)   // Σ on odd bits
```
- Σ = popcount IS the sum (line 1819: "soma (popcount)")
- It counts how many bits are set in the masked byte
- Lines 1837-1843: "Σ dispensou um contador... quantas casaram é quantos bits estão ligados — Σ, o popcount. Guardar o número ao lado do campo era guardar duas vezes a mesma coisa"

**Can it be synthesized?**
- In the theory: popcount IS the primitive Σ (one of the 4 operations)
- In hardware: `bits_conta` instruction / hardware popcount
- In `api.h` (64-bit): no popcount primitive — would need synthesis from ADD/AND/SHIFT

**Classification in THEORY:**
| Criterion | Finding |
|-----------|---------|
| Primitive in theory? | **YES** — explicit in the 4 operations table |
| Necessary for spider algorithm? | **YES** — the 2-line algorithm uses it twice |
| Derivable from {⊕, ⊗, ⊘}? | **NO** — ⊕=cisão (mask partition), ⊗=gato, ⊘=esquilo; none counts bits |
| Mathematically necessary? | **YES** for this algorithm — it's the conservation ∑G(x)=|I| |

**Verdict:** In the **theory**, Σ=popcount is a **PRIMITIVE OPERATION** (one of the 4), mathematically necessary for the conservation law ∑ₓ G(x) = |I|. It is **NOT** derivable from the other three. In the `can/` 64-bit ISA, it would be a **convenient primitive** (architecturally advantageous), not mathematically necessary there.

---

## 8. Stronger Reduction Attempt

**Can any of the 4 be derived from others in THEORY?**

| Candidate | Derivation? | Theory Evidence |
|-----------|-------------|-----------------|
| ⊕ (cisão) from {Σ, ⊗, ⊘} | **NO** | Cisão = partition via complementary masks (0xAA/0x55). Line 1894: "As máscaras são complementares, logo **particionam** o byte — é a cisão." No other op partitions. |
| Σ (popcount) from {⊕, ⊗, ⊘} | **NO** | Σ = conservation ∑G(x)=|I|. Line 1819: "soma (popcount) | ∑ₓ G(x)=|I|". Counting ≠ partitioning/convolving. |
| ⊗ (gato) from {⊕, Σ, ⊘} | **NO** | ⊗ = `cifra_an(a,b) = (n·a+b, a)` — the convolution ζ. Line 1828: "o gato e o esquilo já cá estavam." |
| ⊘ (esquilo) from {⊕, Σ, ⊗} | **NO** | ⊘ = `decifra_an(a,b) = (b, a-n·b)` — the deconvolution μ. Line 1832: "⊘∘⊗ é a identidade sem hipótese nenhuma sobre o par, porque |det|=1". |

**The 4 are independent primitives in the theory.** The determinant condition `|det|=1` (line 1833) makes ⊗ and ⊘ inverses, but neither derives from the others.

**However:** The theory has **8 laws** (aranha.c, racionais.tex). The 4 operations correspond to **Laws 0, 1, 2, 6** at the hexal interface level:
- Law 0: split/cisão (⊕)
- Law 1: dual/involution (related to Σ conservation)
- Law 2: biduality/rotor (J⁴=Id, relates to ⊗/⊘ period 4)
- Law 6: hexal interface (⊕=⊗ at interface)

**Laws 3, 4, 5, 7 are NOT in the 4 ops:**
- Law 3: trial {-1,0,+1} (comparison/sign)
- Law 4: tetral T+T* (tower structure)
- Law 5: pental x²=-1 (complex structure)
- Law 7: octonion dual (pair ≠ class, Word_8)

**Verdict:** **4 is minimal FOR THE SPIDER ALGORITHM**, but **NOT minimal for the full theory** (which has 8 laws). The 4 ops = {Law 0, Law 1 (partial), Law 2 (partial), Law 6}.

---

## 9. Proof Classification Summary (THEORY LEVEL)

| Bridge | Classification | Reason |
|--------|----------------|--------|
| 4 ops = spider algorithm core | **PROVADA** | `arquitetura.tex` 1785-1798, 1805-1824; `tests/neuronio.c` |
| 4 ops = universal ISA basis | **NÃO PROVADA** | Tower, I/O, SQL, dual sort need more |
| MOVE = fold (dobra) with sign | **PROVADA** | `arquitetura.tex` 81-87, 56; `microprocessador.tex` |
| MOVE = only operation | **INTERPRETAÇÃO** | One transformation (A_m×G), 5 signatures |
| U = (X,X*,Mor,Aut) realized | **PROVADA** (corrected) | X=Word_8, X*=ν, Mor=dobra, Aut=A_m×G |
| BYTE = X = Word_8 = B⁸ | **PROVADA** | `arquitetura.tex` 137, 190-193; `racionais.tex` 96 |
| Σ = popcount = primitive | **PROVADA** (in theory) | Explicit in 4-ops table, conservation law |
| 4 is minimal for spider | **PROVADA** | 4 independent primitives for that algorithm |
| 4 is minimal for theory | **REFUTADA** | Theory has 8 laws, tower, 5 signatures |

---

## 10. Final Artifacts

### A. O que realmente foi provado (somente pontes com argumento formal na teoria)

1. **Spider algorithm = {⊕, Σ, ⊗, ⊘} + MOVE** — **PROVADA** (`arquitetura.tex` 1785-1798, 1805-1824; `tests/neuronio.c` "volta em toda dimensão 2 a 8")
2. **MOVE = fold (dobra) with sign (+1/-1/0)** — **PROVADA** (`arquitetura.tex` 56, 81-87; `microprocessador.tex` triad)
3. **BYTE = Word_8 = X = B⁸ (atom of architecture)** — **PROVADA** (`arquitetura.tex` 137, 190-193; `racionais.tex` 96)
4. **Triad ⊕⊗∫ = one transformation A_m × G with 5 signatures** — **PROVADA** (`microprocessador.tex` Prop 1, lines 74-97; validated 40000/40000 in `so_cristal.c`)
5. **Retraction Σ∘Π=Id on slots (exact, no float)** — **PROVADA** (`arquitetura.tex` 1466-1468; `tests/torre_alg.c` §T2)
6. **Tower promotion = unfold (dobra), demotion = fold** — **PROVADA** (`arquitetura.tex` 1262-1267; `tests/promove.c` §Q1, §Q4, 34969 cases)
7. **Continued fraction ADC/DAC exact (det=±1)** — **PROVADA** (`microprocessador.tex` §I/O; `io_micro.c` 40000 fractions)
8. **Dual sort: zero erasure with permutation saved** — **PROVADA** (`arquitetura.tex` 1078-1085; `tests/dualsort.c` 200000/200000)
9. **Popcount Σ = conservation ∑G(x)=|I|** — **PROVADA** (`arquitetura.tex` 1819, 1837-1843; eliminated separate counter)
10. **8 laws map to byte bits (aranha.c)** — **PROVADA** (`aranha.c` lines 142-149; `racionais.tex` table lines 714-757)

---

### B. O que o relatório anterior superestimou (overclaims)

| Overclaim | Theory Reality |
|-----------|----------------|
| "4 operations = ISA mínima" | 4 ops = **spider algorithm core only**; ISA needs tower, I/O, SQL compile, dual sort |
| "MOVE é a única operação" | MOVE = **fold for data movement**; ALU/clock/bus/I/O/control = 5 signatures of A_m×G |
| "U = (word[], LOAD, ALU, MOVE)" | X = Word_8 (byte), X* = ν (involution), Mor = dobra, Aut = A_m×G family |
| "Z₂⁶⁴ ≅ (B⁸)⁸ immersion" | Theory doesn't use Z₂⁶⁴; tower goes F₂→F₈→N→Z→Q→R→Rⁿ; 64-bit is D₆₄/I₁₂₈ level |
| "Popcount derivável" | Popcount = **primitive Σ** in theory (conservation law); not derivable from {⊕,⊗,⊘} |
| "4 é mínimo universal" | 4 is minimal **for spider algorithm**; theory has 8 laws + tower + 5 signatures |

---

### C. Teoremas que faltam para justificar v2 como "ISA mínima"

To claim "Fractal Core v2 = 4 ops + MOVE = complete ISA", the following **theorems are missing**:

1. **Theorem TOWER-OPS:** Show PROMOVE/DESCE (tower promotion) derivable from {⊕, Σ, ⊗, ⊘, MOVE}
   - Currently: `promove.h` uses `S=(x+x†)/2`, `A=(x-x†)/2` with involution ν — not in 4 ops

2. **Theorem IO-OPS:** Show ADC/DAC (continued fraction) derivable from {⊕, Σ, ⊗, ⊘, MOVE}
   - Currently: `adc` uses division loop; `dac` uses `GATO(a_k)` matrix product — not in 4 ops

3. **Theorem SQL-COMPILE:** Show SQL→ISA compilation derivable from {⊕, Σ, ⊗, ⊘, MOVE}
   - Currently: external compiler (`banco/sql.c`)

4. **Theorem DUAL-SORT:** Show dual sort / max-cut derivable from {⊕, Σ, ⊗, ⊘, MOVE}
   - Currently: uses estaca (involution), trial, biduality (J⁴=Id) — higher algorithms

5. **Theorem 8-LAWS:** Show Laws 3,4,5,7 (trial, tetral, pental, octonion) derivable from the 4 ops
   - Currently: explicit in `racionais.tex` table as separate laws at different tower levels

6. **Theorem 5-SIGNATURES:** Show all 5 signatures (ALU, clock, disk, bus, I/O) derivable from 4 ops
   - Currently: `microprocessador.tex` Prop 1 says they're signatures of ONE transformation, not derivable from 4 ops

---

### D. Fractal Core v2 — Estatuto Correto

**Classification: 2. HIPÓTESE ARQUITETURAL FORTEMENTE MOTIVADA**

**Justification:**
- ✅ The 4 operations {⊕, Σ, ⊗, ⊘} + MOVE **formally constitute the stigmergic spider algorithm core** (proven, measured, residue 0)
- ✅ MOVE = fold (dobra) with sign is **the central architectural metaphor**, formally realized
- ✅ Byte = Word_8 = X = B⁸ is **the proven atom** of the architecture
- ✅ The triad ⊕⊗∫ = one transformation A_m×G with 5 signatures is **proven and measured**
- ❌ **No theorem** shows the full byte-level ISA (tower, I/O, SQL, dual sort, 8 laws) reduces to these 4+1
- ❌ The 4 ops correspond to **4 of 8 laws** (Laws 0,1,2,6 partial) at the hexal interface
- ❌ Tower promotion, continued fractions, SQL compilation, dual sort **require additional primitives**

**The hypothesis correctly identifies the algorithmic kernel** (the "neurônio" / spider core) but **overgeneralizes** it to the full ISA. The theory supports: **"The spider algorithm runs on 4 operations + MOVE"** — not **"The ISA reduces to 4 operations + MOVE"**.

---

**End of Corrected Adversarial Audit**  
**Baseline preserved:** `dce95a66` / `fpga-postpnr-functional-baseline` — unchanged.