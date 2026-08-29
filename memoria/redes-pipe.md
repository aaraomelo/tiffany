---
name: redes-pipe
description: "Ponte verificável Redes (fisica.tex) ↔ multifocal ↔ pipe shell — árvore G=1, não Hopfield"
metadata:
  node_type: memory
  type: theory
  modified: 2026-08-28T00:00:00.000Z
---

# Redes e o pipe — ponte verificável

Referências: `fisica.tex` Parte **Redes** (`fis:redes-tese`, §`fis:sobrepoe`–`fis:duastorres`);
`psi/multifocal.tex` (postulados **P1–P6**, §`mf:ancora`, §`mf:conscientes`, §`mf:outro`);
pipe shell: `conecthus/backends/*/interpretar.c` → `tools/wasm_erg.c` → fita ERG → arena.

Documento irmão (duomorfismo ⊕/⊗ no pipe): [`duomorfismo-pipe.md`](duomorfismo-pipe.md).

Formalização LaTeX: [`papers/redes.tex`](../papers/redes.tex) — **Redes Neurais Multifocais**.

---

## Tese central

> **O pipe da fita realiza o lado árvore/levantamento (`G=1`). As órbitas sql/latex/node declaram a matriz Hopfield dual (`tests/orbitas_hopfield.js`); isso não é a fita.**

| Lado | Mecanismo | Multiplicidade | No pipe hoje |
|------|-----------|----------------|--------------|
| **Árvore / levantamento** | prefixo distinto por memória; ramo separado | \(\tilde G=1\) (`fis:thm:levant`) | **sim** — prefixo rodata |
| **Hopfield / Hebb** | soma de padrões numa matriz \(W\) | \(G>1\) (`fis:thm:interfere`) | **órbitas** — `tests/orbitas_hopfield.js` (sql/latex/node + corpus); **fita** ainda \(G=1\) |

Não confundir **árvore com Hopfield**: a sobreposição de prefixos coincide numericamente com o produto interno Hopfield *sob hipótese da árvore* (`fis:thm:sobrepoe`), mas o **mecanismo** de gravação difere — separar vs somar (`fis:thm:interfere`). Medido em `tests/hopfield.c` §F3 vs §F5.

---

## Cadeia do pipe (o que este documento cobre)

```
fisica.tex (Redes)     multifocal.tex          código pipe
       │                      │                      │
       ├─ célula, G, árvore ────┼─ P1–P4, P5 (ζ/μ) ────┼─ arena[OFF_*]  (campo, sem contador G)
       ├─ interferência ───────┼─ P6 (Hebb) ──────────┼─ (futuro)
       └─ W_s/W_a ─────────────┼─ mf:thm:duas ─────────┼─ (fora do pipe; ver hopfield.c)
                               │
interpretar.c  ──traduz──►  wasm  ──wasm_erg.c──►  ERG/ISA  ──erg corre──►  mem.dat ≡ arena
     │                              │
     └─ semântica trial ~           └─ VINCO, óptica caixa (≠ Hopfield)
```

**`tools/test_cadeia.bat`** valida esta cadeia (C→wasm→ERG→metal/browser). **Não** inclui `hopfield.c`, `aranha_n.c` nem `zetamu.c` — esses medidores de Redes/multifocal correm à parte (comandos abaixo).

---

## Tabela principal

| Construção | `fisica.tex` | `multifocal.tex` | Pipe (estado actual) |
|------------|--------------|------------------|----------------------|
| **célula** | realização \(\pi\), slot \(x\) (`fis:def:objeto`) | P1: RPS \(x\in X\) | `arena[offset]` — `OFF_IN`, `OFF_OUT`, `OFF_NIN`… (`shell/arena.h`) |
| **campo / disponibilidade** | \(G_{\mathrm{real}}(x)=\|\pi^{-1}(x)\|\) (`fis:def:objeto`) | P2: disponibilidade histórica | **`G_{\mathrm{visit}}`** em `OFF_G` (host): contador por célula; **≠** \(G_{\mathrm{real}}\) sem protocolo |
| **sobreposição** | prefixo comum (`fis:thm:sobrepoe`) | Cor. proximidade (`mf:cor:prox`) | laço `arena[OFF_IN+k]` vs `RODATA_TAG+k` (concordância de níveis) |
| **levantamento / árvore** | ramo → \(\tilde G=1\) (`fis:thm:levant`, `fis:thm:interfere`) | P4: âncora = bola | `RODATA_TAG` + `seedRodataArena` (`lib/arena_disco.mjs`); um backend = um ramo |
| **~ diferença / trial** | três equivalências ~ (`fis:def:duomorf`) | — | `VINCO`+`JZ` na fita; trial `{−1,0,+1}` em `interpretar.c` |
| **interferência** | \(G_{\mathrm{real}}>1\) por soma Hebb (`fis:thm:interfere`) | P6, `mf:thm:outro` | **órbitas** `tests/orbitas_hopfield.js`; **não na fita** |
| **recuperação (descer)** | energia Lyapunov (`fis:thm:desce`) | âncora / território | **órbitas** — descida Hebb no medidor; fita `*_corre` continua passagem única |
| **neurónio (cisão+soma)** | `fis:redes-neuronio` | — | **não no pipe** — ver `tests/neuronio.c` (fora de `test_cadeia`) |
| **ζ / μ** | `fis:thm:zetamu`, `fis:thm:mu` | P5, `mf:thm:zetamu` | **não na fita** — `G` host acumula; μ = diferença finita em `tests/zetamu.c` |
| **\(W_s\) / \(W_a\)** | `fis:thm:duastorres` | `mf:thm:duas` | **órbitas** `tests/orbitas_hopfield.js`; medidor pleno `tests/hopfield.c` §F7–§F10 |
| **duomorfismo ⊕/⊗** | `fis:def:duomorf` (face aditiva/multiplicativa) | — | `wasm_erg`, manifesto MOVE — ver [`duomorfismo-pipe.md`](duomorfismo-pipe.md) |

---

## Mapeamentos detalhados (com ficheiro)

### Cisão ⊕ + soma Σ → neurónio

- **Teoria:** `fis:redes-neuronio` — máscaras `0x55`/`0xAA` partilham (`⊕`), `popcount` soma (`Σ`); par `[e+o, e]` é levantamento.
- **Multifocal:** não traduz o neurónio booleano; P5 fala em acumulação do campo, não em McCulloch–Pitts.
- **Pipe:** os backends `interpretar.c` **não** implementam neurónio — implementam **interpretar** script (prefixo + corpo → `OFF_OUT`). Não há `popcount` nem limiar na fita shell.
- **Teste:** `tests/neuronio.c` (cisão/soma/gato digital); `fisica.tex` cita também `tests/neuronio.c` na coluna Redes.

### Prefixo comum → sobreposição / ultramétrica

- **Teoria:** `fis:thm:sobrepoe` — \(\langle\xi,\eta\rangle=(2q-N)/N\) sob hipótese da árvore (concordam nos primeiros \(q\) níveis, discordam depois).
- **Multifocal:** `mf:cor:prox` cita o mesmo teorema; P4 liga a \(d=2^{-\operatorname{prof}}\).
- **Pipe:** concordância **byte a byte** no prefixo fixo (`PREFIX_LEN` 5 / 12 / 13 por backend), não produto interno ±1 — mas a **estrutura** é a mesma: contagem de níveis iguais antes do divergir.
- **Teste:** `tests/hopfield.c` §F3 (4096 pares); `tests/aranha_n.c` §AN6 (bolas ultramétricas). Pipe: comportamento correcto quando prefixo coincide (`test_cadeia` → stdout `42`).

### `RODATA_TAG` + prefixo → levantamento / árvore

- **Teoria:** árvore injeta por prefixo; interferência desaparece com coordenada ramo (`fis:thm:interfere`, caixa «troca interferência por espaço»).
- **Multifocal:** P4 — âncora = bola; separar memórias = prefixos disjuntos (`mf:thm:ancora`).
- **Pipe:**
  - `RODATA_TAG = 65408` (`shell/arena.h`, `arena_disco.mjs`) — rodata wasm ≥65536 remapeada (`wasm_erg.c`).
  - Tags: `console.log(` / `echo ` / `Write-Output ` (`RODATA_PREFIX` em `arena_disco.mjs`).
  - `interpretar.c`: `while(k != PREFIX_LEN) if(arena[OFF_IN+k] != arena[RODATA_TAG+k]) return -1`.
  - Host semeia: `seedRodataArena`, `seedScriptArena` (`tools/banco_metal.mjs`).
- **Estado:** um script por execução, um ramo por backend → **\(G=1\)** na leitura «cada memória no seu prefixo», sem matriz somada.

### `LOADS·LOADS·VINCO` → concordância de níveis

- **Teoria:** travessia óptica entre espelhos, mesma visita \(k\) (`fis:caixa`); concordância = diferença zero.
- **Pipe:** `tools/wasm_erg.c` — `emit_vinco_espelhos`, `optica_caixa_pass` (pós-passo elide recálculo afim, mantém `LOADS ptr_in; LOADS ptr_ro; VINCO`).
- **Medido:** passos óptica em `test_cadeia` / `corre_fita_metal` (node 364, bash 259, ps 355).

### `VINCO` → diferença / fronteira do trial

- **Teoria:** equivalência **diferença** ~ (`fis:def:duomorf`); trial `{−1,0,+1}` (`fis:thm:conteudos`).
- **Pipe:** semântica em `interpretar.c` (comentários trial); fita em `wasm_erg.c` (`VINCO`+`JZ` = prefix fail → trial −1). Ver [`duomorfismo-pipe.md`](duomorfismo-pipe.md) §Trial.

### `arena[OFF_*]` → células / campo

- **Teoria:** interface estrela DISCO — mesma célula em semântica, wasm e metal (`fis:thm:estrela` na Parte anterior; Redes usa bolas sobre \(X\)).
- **Multifocal:** P1 — \(X\) com igualdade decidível; P2 — \(G_t(x)\) sobre células.
- **Pipe:** `arena[65536]`; slots `OFF_NIN=24576`, `OFF_IN=256`, `OFF_OUT=16384`; ISA `LOAD/STORE` em `NULO+off` (`NULO_DISCO=8`).
- **Campo G (host):** `OFF_G=24608` (256 células byte, sat 255), `OFF_GSUM=24582`; células backend `G_CELL={node:1, bash:2, powershell:3}`. Após cada `execMoveMetal`, `recordCorreVisit` incrementa `G_{\mathrm{visit}}(x)` na célula do ramo. **Semântica C (`interpretar.c`) ainda não lê G** — contagem só no host/browser.
- **Duas leituras de G** (travar antes de Hebb):
  \[
  G_{\mathrm{real}}(x)=|\pi^{-1}(x)| \;\neq\; G_{\mathrm{visit}}(x)=\text{visitas registadas}
  \]
  Sob protocolo «uma entrada no diário \(\pi(i)\) por corrida, um incremento em `G_visit`», prova-se \(G_{\mathrm{visit}}=G_{\mathrm{real}}\) — ver `papers/redes.tex` (§ Duas leituras de G), `verifyGVisitEqualsReal` / `gRealFromJournal` em `arena_disco.mjs`.
- **Implicação unilateral:** Hebb ⇒ dobra ⇒ \(G_{\mathrm{real}}>1\) *possível*; mas \(G>1 \not\Rightarrow\) Hopfield (colisão/repetição basta).
- **Teste:** `tests/redes_g.js` §RG0–§RG3; `tests/redes_multifocal.js` §RG4–§RG5; `tests/redes_multifocal_io.js` §RG6 (`OFF_IN`/`OFF_OUT`, host).

### \(G=1\) vs \(G>1\)

| | Árvore (pipe) | Hopfield (futuro) |
|---|---------------|-------------------|
| Gravação | prefixo + ramo | \(W_{ij}=\sum_p \xi_i^p\xi_j^p\) |
| Injecção | sim — tags distintas | não — mesma matriz |
| Multiplicidade | 1 padrão activo | \(G>1\) se padrões colapsam |
| Onde ver | `interpretar.c`, `arena_disco.mjs` | `tests/hopfield.c` §F2, §F5 |

- **Pipe:** nenhum `W_ij`, nenhuma soma de padrões entre backends. §RG6: quatro focos em `OFF_IN` — **overwrite** (`overwrite_single_slot`); **≠** interferência / Hebb / Hopfield.

### §RG6 — OFF_IN/OFF_OUT (host, fora da fita) — **FECHADO** · P6 **TRAVADA**

- **Uma arena**, quatro diários; `focusWritePhys` / `focusReadPhys` em `OFF_IN=256`, `OFF_OUT=16384`.
- **Três multiplicidades** (`arena_multifocal.mjs`) — **não assumir iguais**:
  - `G_event(x)` — multiplicidade de **eventos** (escrita + leitura contam separado)
  - `G_focus(x)` — multiplicidade de **focos** distintos sobre x
  - `G_state(x)` — **estados efetivamente preservados** no suporte (slots físicos)
  - Medido: `G_event>1` **⇏** `G_state>1` (dual-X: event≥4, state=1)
- **Operações distintas:** `overwrite_single_slot` ≠ lei de combinação ≠ Hebb ≠ Hopfield
- **Dualidade fisica.tex:** reversível/directo = conta como G; orientada/cruzada = precisa do nome (`fis:thm:simbolos` L1255); «multiplicidade é o que o campo vê; memória é o que a construção preserva» (`fis:thm:agentes` L13079).
- **Regra observada:** `OFF_IN` é estado mutável de valor único; duas escritas → **overwrite** (`overwrite_single_slot`). **Não** é interferência/Hebb/soma.
- **Caso C mínimo:** X→X1 (F1), X→X2 (F2), F3 lê → só X2 sobrevive; X1 e X2 não coexistem num slot.
- **Dual-slot:** F1→OFF_IN, F2→OFF_OUT, mesma célula lógica X → `G_state=2`, `G_event=4` (slot único vs slots distintos).
- **Inverso:** sameRep abre fronteira de combinação; diffRep não (overwrite físico em OFF_IN único pode ocorrer sem abrir fronteira).
- **Observação:** a fronteira aparece quando vários focos incidem na **mesma** representação e o suporte não preserva simultaneamente as incidências — não apenas por haver vários focos.
- **Conclusão canónica:** *a arquitetura existente determina overwrite, mas não determina uma lei de combinação entre duas incidências concorrentes sobre a mesma representação.*
- **Teste:** `tests/redes_multifocal_io.js` (37/37). Auditoria: `tools/auditoria_rg6.bat`.

### Estatuto m=0 / m=1 vs RG6 (auditoria 2026-08-28 — **não abre P6**)

**Três símbolos — não fundir:**

| Símbolo | Onde | O que é |
|---------|------|---------|
| `m` metal | `banco/sql.c` `emit_metal` | \(A_m=T^{m-1}A_1\), \(T=A_1J\) |
| «m=0» DISCO | `duomorfismo-pipe.md` | rótulo interface estrela (`Word.total`, `e=0`) |
| `Lado` 0/1 | `arquitetura.tex` | índice espiral `k` (Hurwitz/Gentil), **não** o `m` metal |

**Demonstrado ≠ interpretativo ≠ aberto:**

| Afirmação | Estatuto | Onde mede |
|-----------|----------|-----------|
| \(m=0 \Rightarrow A_0=J\) (TROCA) | **demonstrado** | `sql.c` + teste `me_gato(m)` |
| \(m=1 \Rightarrow A_1=\) GOLD (primeiro metal) | **demonstrado** | `sql.c`; palavra = um GOLD |
| `emit_metal(0)` = GOLD·NEGRO·TROCA | **demonstrado** | convenção de palavra, não opcode único |
| \(\sum_k \omega^{(i-j)k}=N\delta_{ij}\) | **demonstrado** | `travessia.c` §T5 |
| Parseval no anel \(\mathbb{Z}_{65537}\) | **demonstrado** | `cristal_energia.js` §E1–§E3 |
| Parseval clássico contínuo | **leitura** via teorema central | `teoria.tex` `thm:parseval-multi` obs. |
| «m=0 DISCO» = «m=0 metal» | **analogia documental** | não teorema |
| Combinação de incidências RG6 | **não determinada** | §RG6; overwrite sim, lei não |

**Cruzamento com RG6 (pergunta permitida):** existe estrutura em m=0/m=1/Parseval/ν∘ν que determine o que acontece quando duas incidências chegam à mesma representação?

**Resposta:** **não.** Overwrite (`overwrite_single_slot`) está determinado; lei de combinação entre incidências concorrentes **não**. RG6 = FECHADO; P6 = TRAVADA.

Referência paper: `papers/redes.tex` §«Estatuto da interface m=0/m=1».

### Hebb / P6 → entrada futura, não presente

- **Teoria:** `fis:thm:interfere` — soma não injetiva.
- **Multifocal:** P6, `mf:thm:outro` — «traição inevitável» = interferência; desfazer = ramo (`mf:thm:outro`(3)).
- **Pipe:** nenhum `W_ij`, nenhuma soma de padrões entre backends. Canal/barramento move bytes (`MOVE`); não grava matriz associativa.

### ζ / μ — disponibilidade / resposta (fora do pipe)

- **Teoria:** `fis:thm:zetamu` — escrever com ζ, ler incremento com μ; `G_t=(a*\zeta)(t)`.
- **Multifocal:** P5, `mf:thm:zetamu` — eu = acumulação; mordomos = incremento; gerir = desacumular.
- **Testes (não em `test_cadeia`):**
  - `tests/zetamu.c` — §Z1–§Z5 (`ζ∘μ=id`, recuperação `a_x=G_t-G_{t-1}`, levantamento `k`).
  - `tests/aranha_n.c` — §AN1–§AN2 (`∑G=|I|`), §AN5 (`\tilde\pi` injectivo), §AN31 (agentes vs campo).
- **Pipe:** sem historial de visitas por célula; `OFF_SEQ` incrementa por corrida, não fibra vertical \(k(i)\).

### \(W_s\) / \(W_a\) vs duomorfismo ⊕/⊗

**Manter separado.** `fis:thm:duastorres` e `fis:duastorres` (Redes):

| Eixo | Par | Onde |
|------|-----|------|
| **Duomorfismo (face)** | ⊕ directa / ⊗ cruzada | `fis:def:duomorf`, manifesto `p,q`, `wasm_erg` fusão |
| **Redes (matriz)** | \(W_s\) simétrica / \(W_a\) antissimétrica | Hopfield, energia, Lyapunov |

Colar os dois apagaria o segundo bit (`fis:duastorres`, final da Parte Redes). O pipe documenta **duomorfismo** em [`duomorfismo-pipe.md`](duomorfismo-pipe.md); **\(W_s/W_a\)** só em `tests/hopfield.c` (§F7–§F10: simétrica espelha ordem 2; antissimétrica roda ordem 4; **§F10 falha** ao somar — antissimetria tira Lyapunov, não põe ciclo).

---

## Camadas: interpretar → wasm_erg → ERG → arena

| Camada | Ficheiro | Papel na ponte Redes |
|--------|----------|----------------------|
| Semântica | `interpretar.c` | prefixo = árvore; trial; **não** descida Hopfield |
| Figura | `traduz` → `.wasm` | arena global `DISCO[NULO+off]` |
| 𝒟 tradução | `wasm_erg.c` | `VINCO` = ~ diferença; óptica = espelhos prefixo |
| Dinâmica | fita ERG / `erg.c` | passos = caminhada ∂; laço \(k\) = vertical |
| Suporte | `arena` / `mem.dat` | células; rodata = tag de ramo |

---

## Transição futura (documentada, não implementada)

```text
árvore / levantamento          (pipe actual)
        G = 1
          ↓
     ramos separados            RODATA_TAG, PREFIX_LEN, seedRodataArena
          ↓
       arena                   OFF_IN / OFF_OUT, um script por corrida

          P6 / Hebb             (futuro — mf:thm:outro, fis:thm:interfere)
          ↓
     padrões somados            W_ij = Σ_p ξ_i^p ξ_j^p  — NÃO no interpretar.c
          ↓
       G > 1
          ↓
     interferência              saturação Hopfield; folga Φ = Σ(G−1)

          recuperação           (futuro — fis:thm:desce)
          ↓
     descida assíncrona         update s_i ← sgn(Σ_j W_ij s_j); E nunca sobe
```

Entradas naturais para uma fase Hopfield **sem misturar** com a árvore actual:

1. **Novo medidor / novo export** — não reutilizar `*_corre` como iteração energética.
2. **Campo \(G\)** na arena — seguir `aranha_n.c` / `zetamu.c` antes de Hebb.
3. **P6 isolado** — matriz separada da fita prefixo; custo espaço vs interferência (`mf:thm:outro`(4)).

---

## Medidores e comandos (verificação manual)

| Ficheiro | Blocos | O que prova (Redes / multifocal) | Em `test_cadeia`? |
|----------|--------|----------------------------------|-------------------|
| `tests/hopfield.c` | §F1–§F14 | Hopfield vs árvore; sobreposição; \(W_s/W_a\); Hebb | **não** |
| `tests/aranha_n.c` | §AN1–§AN6, §AN31… | \(G\), bolas, agentes, levantamento | **não** |
| `tests/zetamu.c` | §Z1–§Z5 | ζ/μ, deconvolução de \(G_t\) | **não** |
| `tests/neuronio.c` | — | cisão ⊕ + soma Σ (digital) | **não** |
| `tests/redes_g.js` | §RG0–§RG3 | `G_{\mathrm{visit}}`; protocolo; violações | **não** |
| `tests/redes_multifocal.js` | §RG4–§RG5 | quatro focos, uma arena; conversação | **não** |
| `tests/redes_multifocal_io.js` | §RG6 | `OFF_IN/OFF_OUT`; overwrite; três G; fronteira P6 | **não** |
| `tests/duomorf_pipe.js` | §D0–§D4 | duomorfismo manifesto (≠ Redes \(W_s/W_a\)) | **não** |
| `tools/test_cadeia.bat` | CW*, IF*, CAN*, CB*, AW* | cadeia C→wasm→ERG→metal | **sim** |

Compilação típica (fora da cadeia):

```bat
call tools\env_node.bat
node tests\redes_g.js
node tests\redes_multifocal.js
node tests\redes_multifocal_io.js
```

Ou a auditoria completa §8:

```bat
tools\auditoria_rg6.bat
```

`tools\env_node.bat` coloca `node` no PATH (instalação, `%NODE_BIN%`, ou helper do Cursor v22).
A cadeia **metal** (`corre_fita_metal.exe`, `test_metal.exe`) não depende de Node.

```bat
gcc -O2 -std=c99 -w -Ilib tests\hopfield.c -o hopfield.exe -lm
gcc -O2 -std=c99 -w -Ilib tests\aranha_n.c -o aranha_n.exe
gcc -O2 -std=c99 -w -Ilib tests\zetamu.c -o zetamu.exe
```

---

## O que o pipe **não** afirma (checklist)

- [ ] Não é rede de Hopfield nem grava Hebb.
- [ ] Não confunde \(G_{\mathrm{visit}}\) (host) com \(G_{\mathrm{real}}=|\pi^{-1}(x)|\) sem hipótese de realização.
- [ ] Não mantém \(G_{\mathrm{visit}}\) incremental **na fita** `*_corre` (só host em `OFF_G`).
- [ ] Não afirma \(G>1 \Rightarrow\) Hopfield — multiplicidade pode ser repetição/eventos, não soma Hebb.
- [ ] Não identifica \(G_{\mathrm{event}}\) com \(G_{\mathrm{state}}\) (§RG6).
- [ ] Não rotula `overwrite_single_slot` como interferência / Hebb / Hopfield (§RG6).
- [ ] Não executa recuperação por descida de energia.
- [ ] Não implementa \(W=W_s+W_a\) na fita shell.
- [ ] Não substitui `tests/hopfield.c` / `aranha_n.c` / `zetamu.c`.

---

## Ver também

- [`duomorfismo-pipe.md`](duomorfismo-pipe.md) — trial ~ 𝒟, óptica, MOVE, **faces** ⊕/⊗.
- [`project-hopfield-torres.md`](project-hopfield-torres.md) — notas de medição Hopfield vs árvore.
- [`papers/redes.tex`](../papers/redes.tex) — **Redes Neurais Multifocais** (formalização LaTeX).
- `fisica.tex` `\part{Redes}`; `psi/multifocal.tex` Parte I §`mf:ancora`, §`mf:conscientes`, §`mf:outro`.
