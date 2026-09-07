# FORMAL AUDIT — auditoria conceitual do Microprocessador Fractal contra a teoria (fisica.tex / Corpo Universal / Catálogo / papers)

**Data:** 2026-09-07
**Modo:** auditoria conceitual/formal — **MODO AUDITORIA/PESQUISA INTERNA**. Nenhum arquivo de baseline foi alterado; nenhum opcode, nenhuma instrução, nenhum RTL, nenhuma mudança em `micro.sv`, `ISA_CONTRACT.md`, `api.h`, `micro.c` ou nas camadas CAN/J1939. Só registo.
**Baseline (intocável):** commit `dce95a66`, tag `fpga-postpnr-functional-baseline`; ISA = `ISA_CONTRACT.md` v1 (11 opcodes, `HALT 0..JMP 10`).
**Fontes da teoria:** `tiffany/fisica.tex` (16.699 linhas; labels `fis:*`), `tiffany/corpo_universal.tex` (`univ:*`), `tiffany/catalogo.tex` (`cat:*`), `tiffany/microprocessador.tex`, `tiffany/papers/` (`arquitetura.tex`, `redes.tex`, `aranha.tex`, `inteiros.tex` em especial).
**Fontes da realização:** `can/{api.h, micro.c, micro.h, micro.sv, ISA_CONTRACT.md, can.c, can_bus.c, j1939.c, j1939_tp.c, j1939_catalog.c, j1939_signal.c, can_micro.c, so_cristal.c}`.
**Objetivo:** responder (A) se a microarquitetura atual já realiza em hardware estruturas que a teoria formal descreve matematicamente, e (B) se existe otimização arquitetural que seja consequência da teoria (não ad hoc). Regra de ouro: **não "fractalizar" artificialmente o processador**; descobrir se ele já é uma realização computacional natural do que a teoria encontrou de forma independente, documentar a ponte ou onde ela quebra.

---

## Parte 1 — Mapear a física (fisica.tex)

O Corpo Universal fecha o documento:

> `fis:def:U` (fisica.tex L347–382): **U = (X, X\*, Mor, Aut)**. X é o espaço vectorial sobre B, **dimensão 8**, base `e_k=2^k`. X\* é o dual pelo emparelhamento `<a,b> = paridade(a∧b)`. **As oito leis são essa base** (§fis:neuronio no byte; não há Lei 8). Mor relaciona; Aut processa o vector G∈V. Física ⟵ U ⟶ Catálogo. **X\* ≠ V\* ≠ Duo.**

Cinco factos operativos da física que importam à máquina:

1. **fis:thm:B** (L1999–2022): sobre dois símbolos, as duas composições ficam *forçadas*: a multiplicativa é AND (0 absorvente), a aditiva é XOR com **1⊕1=0** — *o oposto de 1 é 1*, o −1 nunca é escrito. A ISA reproduz isto literalmente nas portas `AND`, `XOR`, `NOT` (api.h).
2. **fis:thm:base** (L3899–3917): potências de 2 são base ortonormal; **`<b,e_k> = (b>>k)&1` — medir e ler o bit são a mesma operação**; uma régua `g^(k)(b)=b∧2^k` é cega às outras.
3. **fis:def:doisandares** (L3949): em baixo, X por onde a trajetória anda; em cima, `V = Z^X` módulo livre de rank `|X|=2^m`, base `δ_x`; o campo G é um vector de V. Convolução `δ_a*δ_b = δ_{a⊕b}` (L3967) — anel de grupo.
4. **fis:def:dual** (L3994): `V* = Hom(V, Z)`, caractere `χ_k(j)=(-1)^{<k,j>}`.
5. **fis:neuronio** (L9203–9290) — o algoritmo final tem **quatro operações**, nem uma a mais:
   ```
   e = pop(b ∧ 0x55),   o = pop(b ∧ 0xAA),   sai [e+o, e]
   ```
   - `⊕` = **cisão** (agrupar em fases; realização π da Def. objeto);
   - `Σ` = **soma = popcount** = Kirchhoff (conservação, `fis:lem:conserva`);
   - `⊗` = **o gato** (sobe; convolução/ζ, `fis:thm:shift`);
   - `⊘` = **o esquilo** (desce; deconvolução, `μ=ζ⁻¹`, `fis:thm:mu`);
   - o par que sai é o **levantamento**: guardado `(e+o, e)`, a fase ausente recupera-se `o=(e+o)-e`. As máscaras `0xAA/0x55` são complementares e *particionam o byte em duas fases: índices pares e ímpares*.
   - `fis:def:gato` (L9251): sobre `Z^m`, `(⊗c)_0 = m·c_0 + c_{n-1}`, `(⊗c)_i = c_{i-1}`; o esquilo é a volta.
   - `fis:thm:gato` (L9266): **`⊘∘⊗=id` exato, `|det⊗|=1`** → inversa inteira, sem nunca dividir. Em n=2: traço m, determinante −1.

Aula de escada (`fis:thm:largura`, L2226): a largura é a ordem, `2^w` lugares, e **cada degrau é uma oitava do anterior** (`ρ_{w+1}⊕ρ_{w+1}=ρ_w`). O "8" é grau/cardinalidade; **Lei 8 não existe** (L195, L243, L5409, `fis:obs:quantica-oito-leis`).

---

## Parte 2 — Corpo Universal e Catálogo

- `univ:def:U` (corpo_universal.tex L109–143): U=(X,X\*,Mor,Aut); Mor={Hom,Iso,Diff,Isom_μ,Duo} "é preservação"; **"A quádrupla é a máquina."** (L142). Abstract: *"A máquina operacional realiza U; o browser realiza a máquina"*.
- dual exact (L921–937): `<a,b>=paridade(a&b)`, `X*≅X` via `{e_k}`; `X*≠V*≠Duo` (não promover evidência a estrutura).
- `cat:ingestao` (catalogo.tex L1614–1647): um corpo novo entra por **INGEST → U** (a raiz), não por legislação. `U ≠ catálogo ≠ INGEST` (L127). **O catálogo não inventa estrutura — regista** (fis:def:U).
- papers/arquitetura.tex: **o átomo é o BYTE; a máquina é a ISA nos slots; a interface comercial é SQL** (L7–9). `sec:isa` (L2475–2481): *"O Catálogo reduz a máquina a UMA operação: MOVE(destino, sentido) — leitura (+1), escrita (−1), salto (pc como destino). O irredutível booleano vive no slot, não no processador (NAND retém operação, não valor). memória = estado geométrico; o slot é onde a palavra algébrica se materializa."* e *"a memória existe onde a trajetória passou"* (L2539).

---

## Parte 3 — BYTE = ESPAÇO DE DIMENSÃO 8

A hipótese: *o byte é X, dim 8, base `e_k=2^k`, e as oito leis são essa base.*

Perguntas 1–5 (da especificação) contra a realização:

| # | Pergunta | Resposta na teoria | Na realização (micro.c / api.h / micro.sv) | Status |
|---|---|---|---|---|
| 1 | A palavra operada tem dimensão 8? | X é dim 8, base `2^k` (fis:def:U; fis:thm:base) | A datapath da ISA é **64 bits** (ADD/MUL sobre u64); o **byte** é a unidade de instrução (10 bytes) e o átomo do paper; `word[]` é u64 | **HIPÓTESE**: o byte é a *unidade*, mas o espaço de máquina é Z₂⁶⁴ |
| 2 | As oito leis = base e₀..e₇? | Sim, L0..L7, sem Lei 8 | Não há registo/flags "leis"; as direções surgem implicitamente nos bits | **ANALOGIA → HIPÓTESE** (ver v2) |
| 3 | Medir e ler = mesma operação? | `<b,e_k>=(b>>k)&1` | Verdadeiro na prática: extração por `AND` máscara + shift (can.tex L296 `R=(D&M)>>r`; j1939_signal extract) | **PROVADA** |
| 4 | Régua cega às outras? | `g⁽ᵏ⁾(b)=b∧2^k` | `AND` com máscara de 1 bit é exatamente isto | **PROVADA** |
| 5 | A largura é a ordem / oitava? | `fis:thm:largura`: `2^w`, duplicação | O contador/divisor é a torção `Z/n` fractal (`microprocessador.tex` §relógio; `w↦w²` == dividir por 2). Medido resíduo 0 em n=2..8192 | **PROVADA** (realização de https relógio/divisor, não da ALU) |

**Leitura da ponte:** a teoria diz "o átomo é o byte"; a máquina v1 processa em palavras de 64 bits mas *materializa* a palavra algébrica no **slot** (8 fatias de 8 bits). A dimensão 8 é a do espaço teórico, não a da datapath. A quebra honesta: **a ISA não expõe X como um espaço de dimensão 8 operado por 8 réguas**; expõe palavras de 64 bits. A ponte que *fecha* é a extração de bits (`AND`+shift = régua = medida), que a própria can.tex já documenta.

---

## Parte 4 — DUAL E PARIDADE

**Teoria:** `X*` pelo `<a,b> = paridade(a∧b)`, `X*≅X`, `X*≠V*≠Duo`. O caractere `χ_k(j)=(-1)^{<k,j>}`. No byte: `pop(b∧0x55)`/`pop(b∧0xAA)` selecionam fases e o `popcount` é a soma.

**Realização:**
- `AND` (op 5) existe; o **emparelhamento `<a,b>` = AND + popcount**.
- **`popcount` não é opcode da ISA v1** (11 ops: HALT..JMP; nenhuma POPCNT). A TA de aceleração (`fis:neuronio`) *exige* Σ=pop como uma das quatro operações.
- A paridade está *implícita* no XOR/parseval da api.h (`universal_dot`, Parseval por igualdade de inteiros) e no j1939 (máscaras 0x55/0xAA não existem em j1939; são do neurônio).

**Classificação:** 
- Leitura dual por emparelhamento — **realizável naturalmente** (AND+pop, já presentes como primitivas; falta só a composição).
- `X*≅X` por `{e_k}` auto-dual — **já realizado como facto** da base binária (autodualidade das potências de 2), sem instrução dedicada.
- Paridade como instrução — **apenas interpretação** hoje (é software/programa).
- `X*≠V*≠Duo` como restrição de arquitetura — **não compatível tal qual**: o modelo C funde dual e módulo no mesmo datapath; a distinção é conceitual, não tem flag.

---

## Parte 5 — TRÍADE ⊕ ⊗ ∏

A tríade da física — Richard ⊕ (soma/adição, Clifford), ⊗ (produto, La Hire), e ∏ (Pontryagin: subir/descer e o ∫=exp∘Σ∘log do `so_cristal.c`) — contra a ISA e o hardware:

| Conceito | Teoria | ISA atual | Hardware (micro.sv/FPGA) | Evidência |
|---|---|---|---|---|
| ⊕ soma | `x⊕y`, Clifford `g_ij`; no byte: XOR | `ADD` (op1), `XOR` (op7), `SUB` (op2) | ALU 64-bit; ripple `ADD = gato∘esquilo iterado` | api.h L55–57; verificado 65536 pares resíduo 0 (`add_ripple.c`) |
| ⊗ produto | `x⊗y`, La Hire; no byte: o gato (sobe) | `MUL` (op3), `AND` (op5) | ALU + **10 MULT18X18D** (síntese ECP5) | `rpgen 10_MULT18X18D`; 40000 pares resíduo 0 |
| ∏ Pontryagin | subir/descer, dual pela métrica; `∫ = exp∘Σ∘log` | `NOT` (op8), shift `<<1`; Mellin/Fourier exatos em `api.h` (D) | só **analógico**: `so_cristal.c` (diodo log/exp); não é instrução digital | so_cristal.c: ⊕≡ADD e ⊗≡MUL em 40000/40000 cada |
| inversão/divisão | o −1 nunca é escrito; DIV euclidiana | `DIV` (op4) `a=q·b+r, 0≤r<b; DIV(a,0)=(0,a)` | DIV multi-cycle (FSM) | ISA_CONTRACT §2; micro.sv |
| composição | `⊘∘⊗=id`, sequências | fluxo linear, FSM/PC; JMP = MOVE(d,0) | FSM de instrução (FETCH→DECODE→LOAD a→LOAD b→ALU→STORE) | micro.c L299–342 |
| transporte | memória = estado geométrico, slot | **`MOVE(destino, ±1, 0)` = LOAD/STORE/JMP** — *a máquina inteira* | tradução literal em micro.sv | arquitetura.tex §sec:isa; micro.c L6–33 |

**Leitura da tríade:** v1 realiza ⊕, ⊗, o oposto e a composição **em digital exato**; o **∏ analógico (∫/diodo) não é instrução digital** — é a realização no cristal (so_cristal.c, Gerbers), validada contra a ISA, não a ISA. Isto é coerente com a teoria: o ∫ é o diodo (o meio do anel→corpo), e a ISA digital não precisa dele para fechar a aritmética (o fecho é por DIV/SUB, não por log).

---

## Parte 6 — MOVIMENTO / TRANSPORTE

A teoria: *"a memória existe onde a trajetória passou"*; o slot é onde a palavra se materializa; MOVE(destino, sentido) é a **única** operação do Catálogo.

A realização: **micro.c implementa exatamente esta tese**:

- `MOVE(destino, +1) = LOAD`, `MOVE(destino, −1) = STORE`, `MOVE(destino, 0) = JMP`.
- "Computação = MOVE\* + ALU": `MOVE(a,+1) → MOVE(b,+1) → ALU(OP) → MOVE(r,−1)`.
- `Maquina = { word[4096] u64, canal, pc }` — o **canal** é o valor em trânsito (o transporte em si), o `word[]` o estado geométrico.
- `micro.sv` é a tradução literal (contrato C ↔ RTL, FPGA validado a 52,49 MHz, 10/10).

Comparação com o ciclo da teoria *corpo → representação → dual → inversor → corpo* (e δ/ε, F/F⁻¹, Duo, πU): a ISA roda o ciclo por *programa* (instruções), não por operações dedicadas; cada transformação pontual (ALU) é pontual (ADD/MUL), e a reversibilidade `⊘∘⊗=id`, `F⁻¹∘F=id`, `|det|=1` está **implementada na api.h** (`universal_F/Finv`, `Mat` com det ±1), não na ISA. **Conclusão: o transporte da teoria É o MOVE da v1 — a correspondência mais forte e mais exata de toda a ponte.**

---

## Parte 7 — CONSERVAÇÃO / INVARIANTES

Lei 4 (conservação): no byte, `e+o = pop(b)`; `fis:lem:conserva`: `Σ G(x) = |I|`; `fis:thm:kirchhoff`. A métrica canónica é o `|det|=1`, conservação de volume (`fis:thm:gato`(2)). Resíduo 0 é o fecho exato.

Candidatos de invariante com ponte **formal** (registo, sem inventar):

| Candidato | Origem teórica | Realização | Status |
|---|---|---|---|
| Volume `|det|=1` | fis:thm:gato(2); api.h Mat | `mat_det(GATO)=-1`, `mat_det(ESQUILO)=+1` — as transformações da api.h são unimodulares | **RELACIONA-DIRETO** (api.h); a ISA aritmética não conserva volume (MUL não é unimodular) → quebra documentada |
| `Σ pop = |I|` | fis:neuronio, fis:lem:conserva | **não há popcount na ISA**; soma é programada | HIPÓTESE (ver v2) |
| Ciclo reversível `⊘∘⊗=id` | fis:thm:gato | api.h: o túnel `esquilo` é a volta do `gato` | **exato**, resíduo 0 nas medições |
| Resíduo 0 (divisão exacta) | cultura do corpo | DIV devolve `(q,r)` com `r<b`; equivalência C↔RTL 10/10 = resíduo 0 | **PROVADA** |
| F⁻¹∘F = id | fis:transf/Parseval | api.h `universal_F/universal_Finv` — igualdade de inteiros | **PROVADA** (medida em dourada_exata) |

**Regra respeitada:** nada inventado; cada candidato tem a sua ponte formal (teorema ↔ código) e o seu estatuto honesto.

---

## Parte 8 — CORPO UNIVERSAL COMO CAMADA SEMÂNTICA

`U = (X, X*, Mor, Aut)` = "a quádrupla é a máquina". Mapa à v1:

| Componente de U | Na máquina v1 | Evidência |
|---|---|---|
| X (estado geométrico) | `word[4096]`, `canal` | ISA_CONTRACT §3; arquitetura.tex |
| X* (dual/leitura) | `LOAD = MOVE(+1)`; emparelhamento por AND+pop (software) | micro.c L88 |
| Mor (relacionar) | a própria composição de instruções; MOVE como morfismo de estado | micro.c L17, L259–265 |
| Aut (processar G) | a ALU (api.h: motor/gato/esquilo; Clifford/La Hire/Pontryagin) | api.h (A)(B)(C) |

**INGEST / realização:** `catalogo.tex cat:ingestao` — um corpo novo entra por ingesta; `can.c/README` diz isso *à letra*: **"o protocolo é externo à ISA"**, o catálogo de sinais é dado (j1939_catalog.c = tabela apenas), a grandeza entra já interpretada. O micro e o browser realize U: `can_micro.c` é o INGEST (compõe, mas o micro não conhece CAN/J1939/TP). 

**Resposta à pergunta da Parte 8:** a CPU v1 é uma **realização operacional de uma língua de U** — não a linguagem inteira (V é o espaço de campos, não exposto), mas a quádrupla (estado/dual/morfismos/automorfismo) *é* o modelo de estado da máquina. **FORTE CANDIDATA.**

---

## Parte 9 — CAN/J1939 COMO EXPERIMENTO

Exemplo medido (can_micro.c / j1939_catalog.c / README):

```
raw = 1920  (SPN110, PGN 65262, bytes 0–1)
num = 1, den = 32, offset = −40
num_inteiro = raw*num + offset*den = 1920 − 1280 = 640   ← o "SUB" (−1280)
phys = 640 / 32 = 20 °C                                  ← o "DIV" (/32)
```

(README narrativo: SPN110→20 °C, SPN190→1000 rpm, SPN84→10 km/h; NA bloqueado.)

A cadeia teórica do can.tex: **meio → assinatura → decodificação → transformação → estado**, com `x = S·R + O` e "⊕,⊗ suficientes para realizar a transformação linear". Isto é uma *representação → transformação → realização física* do corpo:

- `J1939Signal*` = descrição (dado, INGEST);
- `raw` = representação extraída (máscara+leitura — Parte 3);
- `physical = raw*num/den + offset` = ⊕ e ⊗ aplicados (Parte 5);
- `MICRO` = consumidor da grandeza, sem conhecer CAN (Parte 8).

**Veredito da Parte 9:** é o **primeiro caso experimental** do princípio "mesmo corpo + assinatura = ofício". O can.tex é explícito em NÃO ser "CAN=⊕": o barramento é o meio compartilhado; ⊕ é a operação do núcleo. A experimentação confirma a tese do Corpo Universal (a mesma máquina, nova régua; o protocolo externo à ISA) e *não* exige nenhuma extensão da ISA. **PROVADA como realização; não é uma afirmação de que a física do CAN seja o corpo.**

---

## Parte 10 — REDES NEURAIS E HOPFIELD (papers/redes.tex)

O paper `papers/redes.tex` (Redes Neurais Multifocais) é uma **instanciação**, não um postulado: "Hebb/Hopfield na rede **não** são a fonte da lei. São declarações: `W_ij` é o Cor. `fis:cor:hebb`; a descida energética é o Teor. `fis:thm:desce`. A lei da combinação é o Teor. `fis:thm:combina`" (`redes:thm:p6`, L72, "P6 é a instanciação discreta do `fis:thm:combina`"). Descreve-se, portanto, o que a máquina v1 realiza *deste* mecanismo, sem que o paper reclame nenhuma extensão da ISA.

### 10.1 O neurónio é a cisão + soma (JÁ na teoria e realizável na ISA)

O paper não cria objecto novo (tese `redes:tese`): um neurónio McCulloch–Pitts é "soma pesada com limiar"; uma rede Hopfield é "matriz de pesos + regra local de gravação + descida a mínimo". A primeira é **cisão seguida de soma**, a segunda é uma **realização** cuja multiplicidade tem nome próprio: *interferência*. Concretamente (redes.tex §redes:neuronio):

```
e = pop(b ∧ 0x55),  o = pop(b ∧ 0xAA),  sai [ e+o, e ]
```

exactamente o `fis:neuronio` da Parte 1 (`redes:neuronio`, L337–346) — **uma** aplicação do gato (`m=1`), o **levantamento** `(e+o, e)` — "o par que sai é o gato aplicado uma vez" (`redes:thm-mp` L352–357). Realização: o gato é `gato(x)=(x^rotl(x,1))^(x&rotl(x,7))` em `api.h` (gato_soma = XOR, gato_vaium = AND) e o `rotl`/shift é a rotação `esquilo`. **A ISA v1 tem XOR, AND, OR, NOT, shift; falta apenas o `pop` (Σ) — a mesma lacuna documentada nas Partes 7 e 12.**

### 10.2 A matriz de Hopfield e a ISA

A matriz de Hopfield (redes.tex `redes:mp-hopfield`, L373–383 + L963 `redes:duastorres`):

```
W_ij = Σ_p ξ_i^p ξ_j^p      (soma Hebb; Cor. fis:cor:hebb)
E = −½ Σ W_ij s_i s_j       (descida em energia; Teor. fis:thm:desce)
```

Três consequências directas à máquina:

| Peça de Hopfield | Na ISA v1 | Evidência | Status |
|---|---|---|---|
| Regra Hebb `ξ_i·ξ_j` | AND (op 5) e MUL (op 3); o produto exterior é programa sobre a ALU | api.h AND/MUL; ISA_CONTRACT §2 | **REALIZÁVEL** (composição) |
| Soma `Σ_p` | ADD (op 1); acumulação em `word[]` | micro.c exec80 | **REALIZÁVEL** (composição) |
| Descida energética `E` | não é opcode; exigiria uma régua Lyapunov | can.tex: "⊕,⊗ suficientes"; o hopfield.c mede **fora** da ISA | HIPÓTESE (programa) |
| Recuperar = descer (`fis:thm:desce`) | análogo ao MOVE(−1)/STORE: descer é a escrita | motora: MOVE sentidos ±1,0 | **FORTE CANDIDATA** (o descer físico é o MOVE−1) |
| Hopfield dual `λ⁺+λ⁻=0`, `F_H∘F_P=id` | api.h `universal_F/Finv` (F⁻¹∘F=id, exato); eixos só simétrico/antissimétrico `W_s/W_a` são do paper, **não** da ISA | api.h (D); redes:tabela L907 | PROVADA (à API); eixos são interpretação |
| Multiplicidade interferência `G>1` | não há contador de visitas/G na ISA (word[] não é arena) | ISA_CONTRACT §3; redes:def-gduplo L476 | SEM RELAÇÃO na v1 (pertence à arena/pipe, não ao micro) |

### 10.3 Árvore vs Hopfield — o dual que a v1 já escolhe

O corolário mais útil para a arquitetura (redes.tex `redes:cor:dual-gh` L115–123): **árvore e Hopfield são o mesmo dual** — "árvore paga espaço (`G=1`, prefixo injectivo); Hopfield paga dobra (`G>1`, soma)". A máquina v1 realiza a **árvore**: o disco como grafo, `word[]` como estado, MOVE como transporte, `G=1` ideal numa passagem única (o paper di-lo do pipe: "A fita continua G=1"). Hopfield é a *outra face* (soma Hebb numa matriz), que a v1 **não** realiza e **não** precisa de realizar para obedecer à Lei~7 — o fecho fica no duomorfismo, não numa matriz dedicada. **Isto responde à pergunta A: a v1 realiza a face-árvore (G=1) do dual e documenta a face-Hopfield (G>1) como programa possível — sem quebra de contrato.**

### 10.4 Dois resultados de Hopfield que a ISA não usa (mas o contrato permite)

1. **Overwrite é Hebb de capacidade 1** (`redes:cor:ow` L126–132): no slot único, a última escrita prevalece. A v1 faz exactamente isto no `word[]`/canal: `STORE` sobrescreve — *Hebb degenerado, não outra lei*. Coerência sem custo.
2. **Só a parte simétrica mede** (`redes:thm-duastorres` L970): `W_s` mede (energia não sobe), `W_a` ordena (`s^T W_a s = 0`), e este eixo **não é** o duomorfismo ⊕/⊗ — colar os eixos apagaria o segundo bit (redes.tex L985 "W_s/W_a e duomorfismo — eixos distintos"). A v1 não tem `W_s/W_a`; se um dia entrar por programa, o contrato não impede e não obriga.

**Conclusão da Parte 10:** redes.tex *não* exige nada da ISA v1. O neurónio-cisão+Σ topa a mesma lacuna do popcount; a matriz Hopfield é realizável por composição (AND+MUL+ADD) mas não é primitiva; a v1 só precisa que a *árvore* (G=1) continue a ser o seu modo de passagem única. Hopfield entra na taxonomia como **motivo de software/dados sobre a v1**, não como alteração.

---

## Parte 11 — OTIMIZAÇÃO (classificação A–D)

| Classe | Definição | Achados | Decisão |
|---|---|---|---|
| **A — consequência formal** | deriva de teorema, não de gosto | (1) **popcount (Σ)** como operação de núcleo: o algoritmo final da física tem 4 operações (cisão, Σ, gato, esquilo) e a *conservação* exige Σ; não está na ISA v1. (2) **emparelhamento `<a,b>=paridade(a∧b)`** = AND+pop exacto — base para Parseval/transformada. Ambas consequências da Parte 3/4/7 | Registar para **v2**; NÃO tocar v1 |
| **B — alinhamento natural** | já realizado sem esforço | MOVE como única operação de transporte; DIV euclidiano; `word[]` como disco; máscaras AND; formato 80-bit com 1 byte de op | Já feito — nada a mudar |
| **C — engenharia convencional** | otimização clássica | pipelining, dual-port, hierarquia, DMA — **fora do contrato v1** (§4) | Não entra (contrato diz não-v1) |
| **D — analogia estética** | apenas bonito | "oito slots por nó", "Lei 8 não existe", nomes de réguas | Não entra na arquitetura |

A única otimização **consequência da teoria** é expor **Σ (popcount)** e o **par de fases (0x55/0xAA → cisão par/ímpar)** como primitivas do núcleo — porque a física provou que o algoritmo cabe em 4 operações (não 11) e que a soma que se conserva é o popcount. Isto é uma *candidata* a v2, nunca uma alteração ao baseline v1.

---

## Parte 12 — POSSÍVEL NOVA MICROARQUITETURA (hipótese Fractal Core v2)

Existe consequência formal forte? **Sim, uma, e só uma**, e ela é *redução*, não extensão:

> Hipótese **Fractal Core v2**: o núcleo reduz-se às **4 operações do algoritmo final** (fis:neuronio) sobre o slot byte (dim 8): **cisão ⊕ (par/ímpar, 0x55/0xAA), Σ (popcount), gato ⊗ (sobe), esquilo ⊘ (desce)** — montado sobre MOVE(destino,±1,0) — e **nada mais**. A datapath expõe as 8 colunas (leis L0..L7) e o levantamento `(e+o,e)`, com conservação `e+o=|I|` verificável em registo.

| Aspecto | Fractal Core v1 (baseline) | Hipótese v2 |
|---|---|---|
| Estado | `word[4096] u64`, canal, pc | mesmo; + registo de invariante `|I|` exposto |
| Operações | 11 opcodes (ALU 64-bit completa) | 4 operações do algoritmo final (dim 8) |
| Memória | disco `word[]` | mesma ("o arquivo é o grafo", 1 palavra na mão) |
| Dual | implícito (AND+pop em software) | emparelhamento `<a,b>` primitivo |
| Transporte | MOVE ±1/0 | mesmo MOVE ±1/0 |
| Composição | FSM linear | mesma FSM |
| Invariantes | resíduo 0 por contrato | resíduo 0 **e** conservação `Σ=pop` registada |
| ISA | 80-bit, op em 1 byte | reduzida (op de 3 bits? decisão de projeto, NÃO feita aqui) |

**Posição:** v2 é uma *hipótese de arquitetura*, registada para existir como base formal, **expressamente sem alterar v1** (não se muda contrato, RTL, FPGA, CAN). Se não se quiser v2, a v1 permanece correta e completa; a teoria é satisfeita por *programa* (pop como subroutine), só que sem a primitiva de conservação.

---

## Parte 13 — MATRIZ FINAL

| Conceito teórico | Definição / Teorema | Correspondência ISA | Correspondência FPGA | Status |
|---|---|---|---|---|
| Binário | B={0,1}; ⊕ XOR, ⊗ AND forçados (fis:thm:B) | AND, XOR, NOT ops 5,7,8 | ALU bitwise | PROVADA |
| X dim 8 | X v.esp. sobre B, dim 8, base `2^k` (fis:def:U) | datapath 64-bit; byte = slot/instrução | ADD/MUL 64-bit | HIPÓTESE (byte≠datapath) |
| Dual | `<a,b>=paridade(a∧b)`, X*≅X (fis:thm:base) | AND+pop (software); sem opcode | — | REALIZÁVEL NATURALMENTE |
| Paridade | `<b,e_k>=(b>>k)&1` (medir=ler) | AND máscara + shift (j1939 extract) | ALU AND/SHIFT | PROVADA |
| ⊕ | soma; Clifford; 1⊕1=0 | ADD, XOR, SUB | ALU; ripple gato∘esquilo | PROVADA |
| ⊗ | produto; La Hire; o gato sobe | MUL, AND | MULT18X18D ×10 | PROVADA |
| composição | `⊘∘⊗=id`, sequências (fis:thm:gato) | FSM; `MOVE(a)*→ALU→MOVE(r)` | FSM FETCH→…→STORE | FORTE CANDIDATA |
| transporte | memória=estado geom.; slot (arq. §sec:isa) | `MOVE ±1/0 = LOAD/STORE/JMP` | tradução literal | **PROVADA** |
| morfismo | Mor={Hom,Iso,Diff,Isom_μ,Duo} | MOVE/composição lembram morfismos | FSM | INTERPRETAÇÃO |
| conservação | `Σ G=|I|`; `|det|=1` | **sem popcount**; api.h Mat det ±1 | — | HIPÓTESE (v2) |
| resíduo 0 | volta exata, inteiros | DIV `(q,r)`; equivalência C↔RTL 10/10 | P&R 52,49 MHz, 10/10 | PROVADA |
| face/fibra | dois andares X / V=Z^X (fis:def:doisandares) | palavra vs programa/memória | — | HIPÓTESE |
| U | U=(X,X*,Mor,Aut); "a quádrupla é a máquina" | estado/LOAD/ALU/MOVE = os 4 papéis | datapath | FORTE CANDIDATA |
| Ufin | anel de grupo; Fourier/Mellin exactos | api.h (D) Univ, exclusivo da API | — (software) | PROVADA (api) |
| INGEST | catálogo: ingerir, não legislar (cat:ingestao) | can_micro.c; catálogo = tabela-j1939 | — | **PROVADA** |
| Catálogo | regista, não inventa estrutura (fis:def:U) | j1939_catalog.c (só dados) | — | **PROVADA** |
| Hopfield | `W_ij=Σ ξ_i ξ_j` (fis:cor:hebb), descida `E` (fis:thm:desce); árvore G=1 / Hopfield G>1 (cor:dual-gh); overwrite = Hebb cap.1 (cor:ow); W_s mede / W_a ordena (thm-duastorres) | AND+MUL+ADD por composição; `MOVE−1/STORE` = descer/sobrescrever; **não é primitiva da ISA** | — (software) | HIPÓTESE (realizável por programa; face-árvore = v1, face-Hopfield = dual documentado) |

---

## Parte 14 — VERDICT

```
VERDICT: A MÁQUINA ALGUMA JÁ REALIZA A TEORIA; E A TEORIA REDUZ A MÁQUINA.

(A) fisica.tex ↔ ISA:  PROVADA em ⊕, ⊗, o oposto, transporte (MOVE), resíduo 0
     e extração por régua; HIPÓTESE quanto a "byte = espaço de dim 8"
     (a datapath é 64-bit, o byte é a unidade).

(B) Corpo Universal ↔ arquitetura: FORTE CANDIDATA — U=(X,X*,Mor,Aut) é o
     modelo de estado da v1; INGEST ↔ can_micro; catálogo ↔ tabela j1939.
     X*≠V*≠Duo não tem flag na máquina (distinção conceitual).

(C) Otimização consequente da teoria: SIM, uma — expor Σ (popcount) e o
     par de fases/levamento do algoritmo final como primitivas do núcleo
     (fis:neuronio tem 4 operações, não 11). É REDUÇÃO derivada de teorema,
     nunca ad hoc. CANDIDATA a v2; NÃO aplicada a v1.

(D) Próxima hipótese experimental: Fractal Core v2 como 4 operações
     (cisão ⊕, Σ, gato ⊗, esquilo ⊘) + MOVE ±1/0, com conservação |I|
     registada — levando a física ao silício sem inventar estrutura nova.

(E) papers/redes.tex: a rede NÃO é a fonte da lei — P6 é instanciação
     discreta de fis:thm:combina; Hebb é fis:cor:hebb; a descida é
     fis:thm:desce. O neurónio da casa (cisão+pop) reencontra a lacuna
     de Σ já registada; a matriz Hopfield é realizável por composição
     (AND+MUL+ADD/STORE) e não é primitiva; a v1 realiza a face-árvore
     (G=1) do dual árvore×Hopfield. Nenhuma extensão da ISA é necessária.

CLASSIFICAÇÕES:  PROVADA ... 8  (⊕, ⊗, transporte, régua, resíduo0,
                                Ufin-api, INGEST, Catálogo)
                  FORTE CANDIDATA 2 (composição, U)
                  HIPÓTESE ... 6 (X-dim8, dual, conservação, face/fibra, v2,
                                Hopfield)
                  INTERPRETAÇÃO ... 3 (morfismo, W_s/W_a eixos, oito-leis)
                  SEM RELAÇÃO ... 0
```

**Nota final (regra de ouro respeitada):** nenhum conceito foi "fractalizado" à força. As pontes PROVADAS (⊕, ⊗, MOVE, resíduo 0) já estavam na v1 por construção; as pontes HIPÓTESE (dim 8, popcount, duplo andar) são documentadas com o seu teorema de origem e deixadas abertas para v2, sem tocar o baseline `dce95a66` nem `micro.sv`.