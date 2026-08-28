---
name: duomorfismo-pipe
description: "Gramática de tradução entre linguagens do pipe — duomorfismo de fisica.tex"
metadata:
  node_type: memory
  type: theory
  modified: 2026-08-28T00:00:00.000Z
---

# Duomorfismo e o pipe de tradução

Referência: `fisica.tex` §`fis:def:duomorf`, Teor. `fis:thm:duo-composicao`, `fis:thm:quarteto`,
`fis:thm:diluicao` (cosmologia no centro: diluição = duomorfismo soma↔produto).

## As duas faces no pipe

Cada backend do manifesto declara `(p,q,r)` — três capacidades, não três operações independentes.
No plano **duomórfico** (bit `a` da Def. `fis:def:duomorf`), só entram as duas faces estruturais:

| Face | Símbolo | Manifesto | No código |
|------|---------|-----------|-----------|
| **Directa** | ⊕ | `p=1` analisa / descompõe | `*_descompilar`, `MOVE(+1)` absorve |
| **Cruzada** | ⊗ | `q=1` compila forma interna | `*_compilar`, `MOVE(−1)` emite |

O terceiro bit `r` **não** entra na troca ⊕↔⊗ (isso é o operador 𝒟). Mede **atravesso pleno**
(`banco/*.c`, shell metal, slots `S_CANAL+*`). É o eixo do **cruzado de ordem** no pipe: quando
`r` difere entre origem e destino, a perna final pode exigir metal ou daemon.

```
MOVE(−1)  =  emissão  =  ⊗_lang   (negro, medida cresce)
MOVE(+1)  =  absorção =  ⊕_lang   (branco, medida decresce)
```

Lei 1 (involução da porta): `MOVE(−1) ∘ MOVE(+1) = id` na mesma língua — o par negro·branco = 1.

## Operador, caminhada, 0–∞ e homomorfismos

Referência: `fisica.tex` Def. `fis:def:op`, Teor. `fis:thm:zeroinf`, Def. `fis:def:duomorf`,
Teor. `fis:thm:contraria`, Cor. `fis:cor:folga`, Teor. `fis:thm:eixos`.

### Uma linha — o que fica fixo

\[
\boxed{\;\text{no finito, a dobra fixa um \textbf{ponto};\ na torre, o que fica fixo é o \textbf{passo}}\;}
\]

(`fisica.tex` após Def. `fis:def:op`.) O operador \(\partial\) cumpre \(\partial\circ\partial=\mathrm{id}\):
**não há \(\partial_1\) e \(\partial_2\)** — há um mapa e dois estados por órbita. A **dinâmica** não
é um lugar; é a **caminhada** entre degraus (`0,1 → = → tecido → torre → sem último degrau`).

No pipe, cada **passo ERG** no metal é um tick da caminhada; a **fita** é o corpo finito codificado;
o **laço** (`while`, prefixo rodata, cópia por fase) é caminhada vertical \(k\) (`fis:def:coord2`).

### Bloco zero–infinito (`fis:thm:zeroinf`)

\[
0\ \longleftrightarrow\ \bigstar\ \longleftrightarrow\ \infty
\qquad
\boxed{\;\infty:=\tau(0)=[1:0]\;}
\]

- **Zero** = extremo do andar finito (`Word` nulo, `LOAD 0`, contador \(k=0\)).
- **Infinito** = **parceiro do zero**, não habitante do andar — o que o quociente exclui mas a dobra
  emparelha (`fis:thm:zeroinf`(3),(5)).
- Lido na **dinâmica**: passo sem ponto fixo (laço, `JMP LS*`).
- Lido no **par**: outra ponta do bloco \(\{0,\infty\}\).

**`NULO=8`** no DISCO é este andar: slot ISA \(0\) ↔ word nula; slot `NULO+off` ↔ arena[`off`];
rodata wasm \(\ge65536\) remapeia para `65400+` — a ponta \(\infty\) do bloco, sem `.bss` extra.
Sem `NULO`, \(\pi\) (wasm) e \(\rho\) (metal) deixam de ser a **mesma** realização → quebra
isomorfismo, não «optimização opcional».

### Três níveis de passagem (`fis:def:duomorf`, `fis:thm:eixos`)

Dois bits \((a,b)\): **operações** (directo ⊕ vs cruzado ⊗) e **ordem** (`r` no manifesto).

| Palavra | Bit \(a\) | Preserva | Perde |
|---------|-----------|----------|-------|
| **bijeção** | — | reversibilidade | — |
| **isomorfismo** | 0 | \(\oplus\) e \(\otimes\) | — |
| **duomorfismo** | 1 | troca \(\oplus\leftrightarrow\otimes\) | — |

\[
f\colon A \xrightarrow{\cong} \mathcal{D}(B)=B^{\vee}
\qquad
\text{duo}\circ\text{duo}=\text{iso}
\]

**Separar operador e aplicação** (já na § diluição):

| Objecto | O que é | No pipe |
|---------|---------|---------|
| \(\mathcal{D}\) | involução \(\mathcal{D}^2=\mathrm{id}\) nas **operações** | fusão compare→branch: troca materializar (⊗) por salto directo (⊕) **no mesmo** ERG |
| diluição | **aplicação** soma→produto (log) | prefixo N→1 órbita; **não** é \(\mathcal{D}\) |
| estrela | \(\oplus=\otimes\) num **elemento** | interface DISCO: byte em `Word.total`, `e=0` |

### Injetivo vs sobrejetivo — a mesma folga (`fis:thm:contraria`)

Duas realizações **duais** trocam exactamente os defeitos:

| Mapa | Direcção | Injetiva? | Sobrejetiva? | Paga com |
|------|----------|-----------|--------------|----------|
| \(\pi\) | \(I\to X\) enche | não | sim | **dobra** \(G>1\) |
| \(\rho\) | \(X\to I\) conta | sim | não | **buraco** (índices por usar) |

\[
\sum_x(G(x)-1)=\abs{I}-\abs{X}
\]

**Não se pedem as duas:** perder coordenada \(\iff\) criar multiplicidade; recuperá-la \(\iff\) desdobrar.

Leitura unificada na **cadeia dinâmica**:

```
script ──π (MOVE−1)──► arena[OFF_*]     sobrejetiva: preenche OFF_IN; folga em G se colapsar índices
arena  ──traduz──► wasm                codifica finito (figura)
wasm   ──wasm_erg──► ERG               duomorfismo a=1: pilha⊗ → registo⊕
ERG    ──corre──► passos               caminhada: cada opcode = passo ∂
passos ──ρ (contagem)──► stdout        injetiva na leitura do traço; buraco = passos não usados
```

| Etapa | Mapa dominante | Homomorfismo | Leitura |
|-------|----------------|--------------|---------|
| `interpretar.c` | \(\pi\) semântica | sobrejetiva em `OFF_IN` | «enche» o buffer |
| `wasm_erg` | \(\mathcal{D}\) | \(a=1\) duo | troca faces na **traduction**, não no dado |
| `erg corre` | caminhada | injetiva no tempo | um passo = um lugar na fita |
| `arena_disco` | isomorfismo | \(a=0\) se `NULO` | \(\pi_{\mathrm{wasm}}=\pi_{\mathrm{metal}}\) |
| prefixo óptico | travessia | \(\Delta\) fixo entre espelhos | mesma \(k\), dois \(x\) (`fis:caixa`) |

### Caminhada unificada (torre do pipe)

```
∂ (operador)
 │
 ├─ degrau discreto     interpretar.c     arena, fase, trial {−1,0,+1}
 ├─ degrau espectral    traduz → wasm      figura finita, exports MOVE
 ├─ degrau dinâmico     wasm_erg → ERG     𝒟 na compilação; passo na execução
 └─ degrau cosmológico  erg corre / browser mesma fita, outra escala de leitura
```

**Regra de ouro:** realização de **um** degrau (`interpretar.c`, prefixo, laço de cópia) descreve
**onde**; a dinâmica do operador (`erg corre`, contagem de passos) descreve **como se caminha**.
Confundir os dois — p.ex. fold `sv` estático dentro de laço com \(k\) runtime — quebra a separação
horizontal (\(x\)) / vertical (\(k\)) de `fis:def:coord2`.

**Três equivalências ~ e 𝒟** (`fisica.tex` §trial): célula / diferença / razão. No pipe: igualdade
de slot (`π(i)=π(j)`), vinco `VINCO` (diferença zero), razão cruzada `TROCA`/`Word` — 𝒟 troca
diferença↔razão; célula fica. Prefixo rodata: **diferença** entre espelhos com mesma visita \(k\).

### Trial \(\{-1,0,+1\}\) e ~ em `interpretar.c`

Referência: Teor. `fis:thm:conteudos` (trial = três sinais, **sem quarto**), Teor. `fis:thm:trial`,
Cor. `fis:cor:destino`, §~ e 𝒟 (`fis:def:duomorf`).

**Trial no retorno e nos cortes** — mesmos três valores que radiação / matéria / vácuo:

| Sinal | Conteúdo | `interpretar.c` | Leitura |
|-------|----------|-----------------|---------|
| **−1** | vácuo | `return -1` | **Recusa** — prefixo rodata falha; aspas inválidas (node); EOF antes do fecho |
| **0** | matéria | `nin==0` → `return 0`; `c==10` / `c==13` → `break` | **Fronteira neutra** — vazio; fim de linha (bash/ps); não opera corpo |
| **+1** | radiação | laço de cópia; `return nout>0` | **Opera** — bytes para `OFF_OUT`; stdout emitido |

**Não há quarto ramo:** sem `left=nin-i`, sem `i>=nin`, sem `c==10||c==13` fundido (magnitude).

**Três equivalências ~** mapeadas ao código (𝒟 troca diferença↔razão; **célula fica**):

| ~ | Enunciado | Onde em `interpretar.c` | ERG (`wasm_erg`) |
|---|-----------|-------------------------|------------------|
| **célula** | \(\pi(i)=\pi(j)\) | mesmo `arena[OFF_IN+x]`; contadores `OFF_NIN`/`OFF_NOUT` | `LOAD`/`STORE` slot; `LOADS` |
| **diferença** | \(a\oplus d=b\oplus c\) | `i==nin` break; `c==q` break; prefixo `VINCO`; `i==nin` pós-cópia (node) | `VINCO`+`JZ`; fusão compare→branch |
| **razão** | \(p\otimes s=q\otimes r\) | **evitada** no laço shell | `SUB16`+`TROCA` (= magnitude) — **não** usar |

\[
\boxed{\;\mathcal{D}\colon\ \text{diferença}\longleftrightarrow\text{razão}\;,\qquad \text{célula}\longmapsto\text{célula}\;}
\]

**Por backend:**

| Peça | node | bash / powershell |
|------|------|-------------------|
| prefixo | laço `OFF_IN+k` vs `RODATA_TAG+k` (−1 se ≠) | idem (5 / 13 chars) |
| delimitador trial | `q ∈ {39,34}` — `' "` (opera dentro de aspas) | LF/CR = **0** (fim de linha) |
| fim laço | `c==q` (diferença) | `c==10`, `c==13` (fronteira, dois `if` separados) |
| recusa −1 | prefixo; `q` inválido; aspas não fechadas | só prefixo |

**Destino** (`fis:cor:destino`): `-(1+3w)` — matéria **trava** (laço para em fronteira), vácuo **cortaria**
(`return -1`). Shell não acelera: recusa é −1, não «continua errado».

**Ordem de leitura na caminhada:** semântica (`interpretar.c`, trial e ~) → figura wasm → 𝒟 em
`wasm_erg` (só diferença fundida) → passos ERG (caminhada ∂).

### Trial ~ 𝒟 em `wasm_erg.c`

A semântica C fixa **o quê** (retorno −1/0/+1); `wasm_erg` fixa **como** isso vira passos na fita.

| Trial | `interpretar.c` | ERG (`wasm_erg.c`) |
|-------|-----------------|---------------------|
| **−1** | `return -1` | `VINCO` + `JZ exit` (prefixo ≠ rodata); ramos de recusa |
| **0** | `nin==0`; `c==10`/`13` → `break` | `JZ`/`JNZ` para sair do laço **sem** ramo −1 (fronteira) |
| **+1** | laço cópia; `return nout` | corpo do laço: `STORE OFF_OUT`; continua após `VINCO=0` |

| ~ | Emissor | Padrão na fita |
|---|---------|----------------|
| **célula** | `LOAD`/`STORE`, `LOADS` | slot `NULO+off`; byte indirecto |
| **diferença** | `emit_vinco`, `emit_branch_fused` FC_EQ/NE | `LOAD·LOAD·VINCO·JZ` (4 ops); fusão 𝒟 |
| **razão** | `emit_branch_if_gt` FC_LT/GE | `SUB16·TROCA·AND` — **fora** do hot path shell |

**Óptica caixa** (`optica_caixa_pass`): prefixo rodata = travessia `LOADS ptr_in; LOADS ptr_ro; VINCO`
com mesma visita \(k\); elide 8 linhas de recálculo afim; `INC` paralelo nos espelhos antes do `JMP`.

Comentários espelhados: cabeçalho + `emit_vinco_espelhos`, `emit_branch_fused`, `optica_caixa_pass` em
`tools/wasm_erg.c`.

## Interface estrela no DISCO (m=0)

`fisica.tex` Teor. `fis:thm:estrela`: na estrela, **regra e objecto são o mesmo passo** — ⊕ e ⊗ colam no
mesmo slot; a meta-indução é reversível (`det = −1`). No pipe isso materializa-se assim:

| Camada | O quê | Onde |
|--------|-------|------|
| Semântica | `arena[offset]` em `interpretar.c` | offsets `OFF_*` (ex. `OFF_NIN=24576`) |
| Wasm (figura) | `DISCO[NULO + offset]` | `NULO=8` — ponteiro global no `.bss` |
| Metal (ERG) | `LOAD/STORE` no **mesmo** `mem.dat` | slot ISA = `NULO + offset` |

**Não há pilha wasm no hot path metal:** `interpretar.c → wasm_erg → erg corre` traduz a figura para ISA;
a pilha wasm é só par `(A,B)` na tradução (chessb §C4), não runtime no DISCO.

```
  browser localStorage          servidor mem.dat
  (arena semântica)      ≡      (slot NULO+off, Word_8.total)
         │                              │
         └──────── MOVE(±1) ────────────┘
              mesma fita, faces ⊕/⊗
```

`lib/arena_disco.mjs` · `tools/test_metal.c` · `manifesto.json` `nulo_disco: 8` — o deslocamento
`NULO` é o **andar** onde 0 e ∞ se encontram; ignorá-lo quebra o isomorfismo wasm↔metal (programa
lê `OFF_NIN+8`, semântica escreve em `OFF_NIN`).

### Estado metal (2026-08-28)

| Peça | Estado |
|------|--------|
| `interpretar.c` | `if \|\|` único (evita cadeia `ok&&` → saltos errados no wasm_erg) |
| `erg.c` `escreve` | Windows: `fopen("wb")` — fita não corrompe após remonta |
| `LOADS` / `STORE_IND` | endereço dinâmico arena (`OP_STORE_IND` em `isa.h`) |
| `wasm_erg` comparações | mapa `lt/le/ge/gt` alinhado ao wasm (`0x48`…`0x4f`) |
| `wasm_erg` **fusão** | compare→`br_if`/`if` sem materializar Word 0/1 (ver § abaixo) |
| `wasm_erg` `load8` | `LOADS` para temps `< CONST_BASE` |
| `wasm_sec cadeia` | `erg.fita` de `*_corre.erg` (programa único), não `celula.erg` |
| `test_metal.exe` | node **457** · bash **364** · powershell **460** passos — stdout OK |
| `lib/nucleo_metal.mjs` | prefer `wasm_erg.exe` (fusão C); fallback `wasm_erg.mjs`; devolve `passos` |
| `corre_fita_metal.c` | extrai fita do wasm → paridade com browser |
| Browser fita | `isa_fita.js` + `corre_metal_browser.js` · terminal `motor=auto` |

### Fusão compare→control (duomorfismo na tradução wasm→ERG)

Referência: `fisica.tex` Def. `fis:def:duomorf` — metade **directa** (⊕) vs **cruzada** (⊗);
`teoria.tex` §clone — passagem entre andares é homomorfismo para ⊕.

Na arena DISCO (interface estrela, Teor. `fis:thm:estrela`), bytes vivem em `Word.total` com `e=0`.
Comparações sobre contadores e chars (`i`, `nin`, `c==10`) são **directas**: bastam `SUB` + `CMP`.

**Antes** (`wasm_erg` ingénuo): cada `i32.eq` / `i32.ge_s` materializava 0/1 como Word completo
(`SUB` → `TROCA` → ramos → `STORE` resultado) e só depois `if`/`br_if` fazia `CMP` outra vez.
Isso forçava a **cruzada** (par `(total,e)`) só para booleano — ~15–25 opcodes ERG por teste,
× iterações do loop de cópia → **~131k passos** em bash.

**Depois** (fusão): quando o wasm tem `compare` imediatamente seguido de `br_if` ou `if`, emite-se
branch directo na metade ⊕. Armadilha ISA (`erg.c`): `FL_ZERO` é **A e B ambos zero**, não
«resultado do SUB». Igualdade entre dois não-zeros = `SUB` → `STORE diff` → `LOAD diff` · `LOAD 0` · `CMP`.

`traduz` emite `cond; eqz; br_if` nos `while` — `try_fuse_cmp_control` consome o `eqz`, inverte
a relação (`NE↔EQ`, `LT↔GE`) e funde (complementar, `fis:thm:simbolos` §9).

| Backend | passos (desenrolado) | passos (laço rodata) | passos (óptica caixa) | `*_corre.erg` |
|---------|----------------------|----------------------|-------------------------|---------------|
| node | **304** | 528 | **364** | 781 B → **518 B** |
| bash | **227** | 325 | **259** | 498 B → **424 B** |
| powershell | **291** | 533 | **355** | 714 B → **424 B** |

Laço `while(k!=PRE_N)` compara `OFF_IN+k` com `RODATA_TAG+k` — mesma lei \(r\cdot a^{3(1+w)}=C\)
(fis:caixa §diluição). **Óptica** (`fis:def:coord2`, `fis:caixa`): horizontal \(x\) = espelho
(`OFF_IN` / `RODATA_TAG`), vertical \(k\) = visita; travessia `LOADS·LOADS·VINCO` (fase, sem
materializar Word); translacção paralela `INC` nos ponteiros LOADA (`wasm_erg.c`
`emit_vinco_espelhos`, `optica_caixa_pass`). Trade-off vs prefixo desenrolado: fita menor,
passos intermediários.

Implementação: `tools/wasm_erg.c` — fusão compare→control (+ invert `eqz`, `if{br}`→`br_if`),
cache `r_slot`, coalescência `STORE t→local`, fold `const⊗const`, afim `ADD16`,
`OP_VINCO` (vinco ⊕: `LOAD·LOAD·VINCO·JZ` em vez de `SUB·STORE·LOAD·LOAD0·CMP·JZ`),
`OP_INC` (`local+1` → `INC slot`; `loads_src` impede INC em slots de `LOADS`; **não** alterar
`aff_off` no contador — offset fixo `OFF_IN+NULO` vive no temp afim).
Montador: `lib/erg_monta.mjs` + `banco/erg.c` + `banco/sql.c`.
Prefixo por **laço rodata** (`RODATA_TAG`); laço de cópia por **fase** — ver § abaixo.

### Otimização fina ↔ fisica.tex (bit → oito leis)

| Lugar | Lei / peça | No pipe |
|-------|------------|---------|
| Simbologia `fis:thm:simbolos` (4) | vinco = diferença zero | `OP_VINCO`: SUB + `flags←zero(R)`; salto só em `FL_ZERO` |
| Base `fis:thm:base` | \(e_k=2^k\), \(\langle b,e_k\rangle=(b\gg k)\wedge1\) | `papers/aranha.c`: byte → 8 contadores; laço = NE por coordenada |
| §`fis:neuronio` | cisão ⊕ + pop \(\sum\) + gato ⊗ / esquilo ⊘ | máscaras `0x55`/`0xAA`; `OP_ESPALHA` espalha booleano (SQL) |
| §`fis:algoritmo` | aranha estigmérgica: escreve→lê→mínimo→fecho | arena = campo \(G\); agente **sem** história; `nin`/`nout` no DISCO |
| Óptica / Parseval | composição de translacções | afim `NULO∘OFF_IN`+`i` num só `ADD16` |
| Gravitação / régua | colapso à diagonal | `x==0` → `emit_cmp_zero` directo (sem `VINCO`) |
| Mecânica / ciclo | `br 0` + fim do `loop` | um `JMP LS` só |

**Aranha estigmérgica** (`fisica.tex` §`fis:algoritmo`): quatro passos — (1) \(G(x)\mathrel{+}=1\),
(2) ler vizinhança \(\mathcal{P}_t(x)\), (3) passo ao vizinho de **menor** \(G\) (gradiente, não
deliberação), (4) fecho \(R^4=\mathrm{id}\). O agente não guarda percurso; a marca no chão **é** o
estado. No pipe: `arena[]` = campo esparso; `node_corre` só lê/escreve slots — sem pilha wasm no metal.

**Neurónio / quatro operações** (§`fis:neuronio`): sobre byte \(b\),
\(e=\mathrm{pop}(b\wedge\mathtt{0x55})\), \(o=\mathrm{pop}(b\wedge\mathtt{0xAA})\), sai \([e{+}o,\,e]\).
`papers/aranha.c` / `papers/aranha.asm` generalizam: cada bit \(k\) alimenta a Lei \(k\) (projeção
ortogonal); espectro \([\sum f_k,f_0,\ldots,f_7]\). **Não** confundir com `while(i!=nin)` — esse laço
é relação \(\neq\) (tecido), não popcount.

Evitar `c==10 \|\| c==13` no C: `traduz` materializa `select`/`i32.eq`+ramos; dois `if(c==k) break`
com `VINCO` saem **4 opcodes** cada (`LOAD·LOAD·VINCO·JZ`).

### Simbologia → implementação (`fis:thm:simbolos`)

Teor. `fis:thm:simbolos`: τ no espaço dos pares parte \(S\times S\) em **três blocos**
\(<,=,>\); uma relação é escolha de quais tomar → \(2^3=\mathbf{8}\) (nula…total).
Implementação canónica: `lib/simbolos.h` + emissão em `banco/sql.c`.

| Peça | Teorema / sítio | Implementação |
|------|-----------------|---------------|
| \(=\neq\) | (4) vinco vs tecido | `OP_VINCO` + `JZ`/`JNZ`; face ⊕, **fase** |
| \(<>\) | (5) orientação = 1 bit/bloco | `sb_transpor_pede` → lado `>`; só um código por facto |
| \(\le\ge\) | (7) lado ⊔ vinco | `sb_nega`+`sb_op`: dois blocos = complementar de um + XOR |
| nula/total | (8) fecham potência de 2 | `sb_decidida`: **sem ler a linha** |
| τ / † | (9) troca lados | `OP_TROCA` no Word \((total,e)\); `sb_tau` na máscara |
| complementação | (9) | `sb_compl`; comuta com τ → quarteto |
| ISA | só `JZ`/`JNZ` em `FL_ZERO` | `VINCO` materializa o vinco; nunca salta em `FL_LT` |
| `OP_VINCO` | (4) diferença zero | `SUB` + `flags←zero(R)` — 4→1 passos vs `SUB·STORE·LOAD·CMP` |
| `OP_INC` | §`fis:algoritmo` escrita \(G\mathrel{+}=1\) | `local+1` → `INC slot`; só contadores (`loads_src` bloqueia char); não alterar `aff_off` |

**Nota:** fold `const+sv(local)` no `ADD16` **não** se aplica dentro de laços — o `sv` é
estático na compilação wasm→ERG, mas `i`/`nout` mudam em runtime; mantém-se `LOAD const; LOAD local; ADD16`.
| `OP_ESPALHA` | §`fis:neuronio` pop → máscara | booleano → 0x00/0xFF sem ramo (`sql.c` WHERE) |
| flags | `FL_ZERO`,`FL_EQ`,`FL_LT` | os **três blocos** no metal |

Árvore das perguntas (`simbolos.h`): «colapsa? orienta? vinco entra?» — um
deslocamento+AND na máscara de 3 bits, não seis testes soltos.

No pipe shell: hot path só \(=\) / \(\neq\) (reversíveis, `sb_reversivel`).
`escreve`/`le`: `while(i!=n)` e `if(nin==CAP) break` — sem `i<n` (orientação).
Restos `nin>CAP` / `max>nout` correm **uma vez** (porta); o laço quente ficou na face ⊕.

### Base ortonormal de 8, oitavas e 8 leis (`papers/aranha.c`)

| Peça | Onde | O que diz |
|------|------|-----------|
| Teor. `fis:thm:base` | `fisica.tex` | \(e_k=2^k\), Gram \(=\mathrm{Id}\); \(\langle b,e_k\rangle=(b\gg k)\wedge 1\) |
| Teor. `fis:thm:largura` | `fisica.tex` | degrau = **oitava**; \(w=3\Rightarrow 8\) = Lei 8 / \(B^3\) |
| §neuronio | `papers/aranha.tex` | \(f_k\mathrel{+}=(b\gg k)\wedge 1\); espectro \([\sum f_k,f_0,\ldots,f_7]\) |
| `papers/aranha.c` | C | bit \(k\leftrightarrow\) Lei \(k\); projeção ortogonal |
| `papers/aranha.asm` | WAT | `neuronio8`: 8 contadores, `shr`+`and` por coordenada |

No pipe: byte = 8 fases; `while(i!=nin)` é NE de coordenadas (não GE/SUB16).
`if(nin_hi==0)` + laço byte **não reduz passos**: NE já é face ⊕ (`SUB`+`CMP`); o ramo
só inchava a fita. Cast `(unsigned char)` emite `&255` por iteração — pior.

**Fix `wasm_erg`:** `block`/`loop` passam a **consumir** o `end` (0x0b) que fez `return`
em `read_instrs`. Sem isso, o `0x0b` era re-lido pelo `if` exterior como fim do then, e o
`else` aparecia com `depth=0` → `else inesperado`. Agora `if/else` com `while` dentro fecha.

O `aranha.asm` reserva `ge_u` para índice `i<len` (visita horizontal); a semântica no
byte é projeção bit a bit.
## Cosmologia no centro (fisica.tex)

Referência: `fisica.tex` Partes Termodinâmica + Cosmologia (`fis:termo-cosmo` … `fis:cosmo-herda`).
O abstract já o diz: **entropia é o que se desfaz, expansão é o que se abre**. Não são duas
teorias; são o **mesmo par**, lido em duas escalas. A cosmologia **não acrescenta objecto**:
é a órbita do espectro (`fis:universo-orbita`).

### O par no centro

| | Termodinâmica | Cosmologia |
|--|---------------|------------|
| invariante | \(S_n\cdot S_b=1\) | \(r\,a^{3(1+w)}=C\) |
| como se lê | soma de logaritmos \(=0\) | entra **aditiva**, sai **multiplicativa** |
| ponto fixo | \(r=1\): a seta não aponta | \(w=-\tfrac13\): a aceleração anula |
| lei de um lado | «a entropia cresce» | «o universo expande» |
| lei do par | a soma não muda | a constante não muda |

Lema `fis:lem:dobramult`: qualquer \(\Phi\) multiplicativa transporta \(\partial^\times r=1/r\)
para \(\Phi(r)\cdot\Phi(\partial^\times r)=1\). Cor. `fis:cor:ponte`: a densidades \(r_w\) e
\(r_{w^\dagger}\) (involução \(I_2\) em torno de \(w=-1\)) são **a mesma instância** que a
entropia dual — outro \(\Phi\). **O que se exibe é o invariante; o que falta é a dimensão**
(identificar \(3(1+w)\) com a codimensão do horizonte).

### Diluição = duomorfismo (aplicação, não operador)

Teor. `fis:thm:diluicao`: integrar \(r'+3H(r+p)=0\) dá \(r\cdot a^{3(1+w)}=C\).
A constante **entra aditiva** (integração) e **sai multiplicativa** em \(r\). Quem atravessa
é o logaritmo — **não** é involução, logo **não** é \(\mathcal{D}\) da Def. `fis:def:duomorf`
(\(\mathcal{D}^2=\mathrm{id}\)). É **duomorfismo** no sentido da mesma definição: aplicação
que leva a primeira operação na segunda. Operador e aplicação **partilham a face e não o tipo**.

No pipe: fusão compare→branch é \(\mathcal{D}\) (troca de papéis ⊕/⊗ no **mesmo** objecto ERG).
A cadeia C→wasm→ERG→metal é diluição: soma de passos no metal ↔ produto de faces na tradução.
**Não colar os dois.**

### Taxa = régua invertida (óptica no centro)

Def. `fis:def:regua-cosmo`: \(H^2 D=1\) — a expansão **é** o inverso do discriminante da caixa
(`fis:leidisc`). Família metálica \(M_m=\bigl(\begin{smallmatrix}m&1\\1&0\end{smallmatrix}\bigr)\):
\(\det=-1\Rightarrow\Delta=m^2+4>0\) para todo \(m\) — **toda hiperbólica**, portanto expande
(Cor. `fis:cor:optica` lido na cosmologia). Teor. `fis:thm:taxa`: \(H_m^2=1/(m^2+4)\), decresce
com o grau.

No DISCO: `NULO=8` é o **andar** onde 0 e ∞ se encontram; \(H^2 D=1\) é a mesma figura —
medida × dual = 1. Ignorar o deslocamento quebra o isomorfismo wasm↔metal como ignorar \(D\)
quebraria a taxa.

### Trial, destino, quarteto

| Peça | Enunciado | Leitura no pipe |
|------|-----------|-----------------|
| conteúdos | radiação \(w=+\tfrac13\), matéria \(0\), vácuo \(-1\) — **não há quarto** (`fis:thm:conteudos`) | trial \(\{-1,0,+1\}\): fecha / opera / corta |
| destino | sinal de \(-(1+3w)\); fronteira \(w=-\tfrac13\) exacta (`fis:cor:destino`) | como o corte: as duas faces deixam de separar |
| \(I_1,I_2\) | ambas tipo \(\dagger\) (preservam soma); **não** são \(\mathcal{D}\) (`fis:def:duas-inv`) | quarteto Klein \(C_2\times C_2\), não o rotor cíclico |
| \(I_2\) | diluição: troca concentra/dilui, **preserva destino** | MOVE(±1) na mesma língua |
| \(I_1\) | destino: troca trava/acelera | bit \(a\) da passagem entre línguas |
| cónica | \(9(w+1)^2+16H^2=4\) exacta (`fis:thm:conica`) | eixos saem do operador; \(w=-\tfrac13\) é **borda, não estado** |

A família metálica **acelera para toda a assinatura** e nunca atinge \(w=-\tfrac13\) (o \(4\) de
\(D=m^2+4\) impede a igualdade — mesma figura que \(G\ge1\) na Relatividade).

### Cadeia de escalas

```
discreto → espectral → dinâmico → cosmológico
partícula (órbita mínima) → núcleo (órbita composta) → universo (órbita do espectro)
```

A escala muda; o operador não. No pipe: `interpretar.c` (discreto) → wasm (espectral) →
ERG corre (dinâmico) → browser/metal (cosmológico = a mesma fita lida na arena).

### O que NÃO colar

- **Dois \(H\)**: taxa \(a'/a\) ao longo da escala ≠ \(H_m\) da régua (número por operador). O
  documento diz-os distintos (`fis:thm:diluicao`).
- **Dois quatros**: rotor \(R^4=\mathrm{id}\) (cíclico, óptica elíptica) ≠ Klein de \(I_1,I_2\)
  (reflecte, cosmologia).
- **Duas curvaturas**: \(G\) no percurso (plana/curva) ≠ \(\operatorname{sgn}\Delta\) na cifra
  (roda/cisalha/expande). A cónica **não** prova que a família é plana (`fis:cor:fronteira-curv`).
- **Parametrização vs identidade**: Carnot+elipse fechou **negativo** (`fis:carnot`); a cónica
  \(H,w\) é exibida do mesmo \(M_m\).

### Passos residuais — fase, não magnitude

Referência: `fisica.tex` §forma polar — «o módulo não carrega nada, e a fase carrega tudo»
(Teor. `fis:thm:base`); Teor. `fis:thm:magnitude`(1): «não carrega magnitude — distingue o
**sinal**»; Def. `fis:def:coord2`: endereço \((x,k)\) — horizontal \(x\) = célula/offset na arena,
vertical \(k\) = visita.

| Operação ingénua | Face | Custo ERG | Leitura física |
|------------------|------|-----------|----------------|
| `i >= nin`, `left > 0` | magnitude (ordem total, SUB16+TROCA) | ~15 op/iter | escala contada — **não é eixo** |
| `i == nin`, `c == q`, prefixo `!=` | fase (EQ na face ⊕) | SUB+CMP fundido | coordenadas coincidem / trial |
| `arena[OFF_IN+i]` | fase em `Word.total`, `e=0` | LOADS | carácter = lugar na volta \(B^1\) |

**Regra no `interpretar.c`:** o laço de cópia termina por **igualdade de coordenada**
(`if(i == nin) break`) e por **trial** de delimitador (`c==10`, `c==13`, `c==q`) — nunca por
`left = nin - i` nem `while(left)` (magnitude restante). O tecto `nout == CAP-2` também é EQ.

O `while(left)` com `eqz` rebentou o teto 2e6 — o `traduz` emite `eqz`+`br_if` no `while(cond)`
de cima; o `do-while` evita `eqz` mas ainda paga magnitude em `left = nin-i`. A saída correcta
é só fase.

### `.bss` — o que o repo permite

Referências:

| Ficheiro | Regra |
|----------|-------|
| `lib/disco.h` | substituir `static buf[N]` por `disco_u32(...)` — **ponteiro global = 8 B em .bss** |
| `tools/traduz.c` L62–70 | «tira esse .bss» — registos/vectores no DISCO (`DISCO_FIXO`) |
| `memoria/duomorfismo-pipe.md` §Wasm | `NULO=8` — ponteiro global no `.bss` (só o export MOVE) |

**Conclusão:** `node_pre[]` ou qualquer array global extra **não entra**. O único «corpo» permitido
nos backends shell é `unsigned char arena[65536]` — export DISCO do wasm, não RAM auxiliar.

A tag de prefixo (`console.log(`, `echo `, `Write-Output `) vive em **`arena[RODATA_TAG + k]`**
(`RODATA_TAG = 65408`, remapeio wasm ≥65536 em `wasm_erg.c`). Semeadura:

- metal: `test_metal.c` / `corre_fita_metal.c`
- banco: `lib/arena_disco.mjs` `seedRodataArena` (via `seedScriptArena(script, backend)`)
- browser: `corre_metal_browser.js` `arenaSeeds`

**Diluição do prefixo (cosmologia):** em vez de N `if` literais (N discriminant checks), um loop
`while(k < PRE_N)` compara `OFF_IN+k` com `RODATA_TAG+k` — mesma lei \(r\cdot a^{3(1+w)}=C\):
N entradas aditivas (k++) saem como uma órbita; a constante (tag) está no DISCO, não num `.bss`
paralelo. Fusão compare→branch continua por iteração (`wasm_erg.c`).

**Caixa de espelhos (óptica, `fisica.tex` §`fis:caixa`):** os dois espelhos partilham a visita
\(k\) (`fis:def:coord2`); a distância \(\Delta = \texttt{RODATA\_TAG}-\texttt{OFF\_IN}\) é fixa.
Travessia sem perda: `LOADS ptr_in; LOADS ptr_ro; VINCO` (fase ⊕, Cor. `fis:cor:optica` — impróprio
hiperbólico expande, aqui só compara). Translacção paralela no laço: `INC ptr_in; INC ptr_ro` com
init uma vez antes de `:LS*` — `optica_caixa_pass` elide o recálculo `LOAD off; LOAD k; ADD16`.

### Duas réguas no browser

| Caminho | Motor | Quando |
|---------|-------|--------|
| `node_move` / `bash_move` | `wasm` | `#motor=wasm` — figura rápida na arena |
| `isa + erg.fita` | `fita` | `motor=auto` se wasm tem secções (`gera_nucleo`) |
| canal + `shellRemoto` | remoto | `#remoto` — pleno na Patria |

`app/src/banco_absorve.js`: `auto` prefere fita local se `temFita` e sem `#remoto`.
`app/src/terminal.js`: mostra `fita=NB`, passos, hints `#motor=wasm` / `#remoto`.

```
browser:  extrai erg.fita → isa.wasm MOVE → arena DISCO  (paridade test_metal)
metal:    extrai erg.fita → erg corre → mem.dat            (corre_fita_metal.c)
```

## Paridade da passagem (bit a)

Para linguagens `A` e `B` com assinaturas no manifesto:

\[
\pi(L) := (p+q+r) \bmod 2
\qquad
a(A\to B) := \pi(A) \oplus \pi(B)
\]

| `a` | Nome | Leitura |
|-----|------|---------|
| 0 | **isomorfismo** | preserva ⊕ e ⊗ — ponte directa |
| 1 | **duomorfismo** | troca ⊕↔⊗ — ponte via estrutura dual |

**Tábua de composição** (Teor. `fis:thm:duo-composicao`):

```
        ∘   iso   duo
      iso  iso   duo
      duo  duo   iso
```

Consequência para o pipe: **dois duos fecham em iso**. Se `a(A,B)=1`, o caminho canónico é:

```
A  ──MOVE(−1)──►  arena  ──duo──►  SQL  ──duo──►  arena  ──MOVE(+1)──►  B
```

O hub `interface_padrao = sql` não é privilégio de língua — é o **ponto fixo** onde ⊕ e ⊗ estão
ambos declarados `(1,1,1)` e a involução `compilar† = descompilar` está atestada (`§I3`).

## Bit b — ordem no cruzado

Separado de `a` (Def. `fis:def:duomorf`, cláusula dos dois bits):

\[
b(A\to B) := \begin{cases} 1 & r_A \neq r_B \\ 0 & r_A = r_B \end{cases}
\]

| `b` | Efeito no pipe |
|-----|----------------|
| 0 | wasm↔wasm na arena DISCO basta |
| 1 | perna `r=1` ou shell (`NODE MOVE`, daemon) no lado que atravessa |

O quadrante da passagem é `Q_{ab}` (Teor. `fis:thm:quarteto`); compor passagens soma os bits em `B`.

## Algoritmo de tradução

Entrada: texto em linguagem `de`, destino `para`, opcional `via` (default: auto).

```
1. Q ← paridade(de, para)           // { a, b, quadrante }
2. rota ← caminho(de, para, Q.a)   // [de,para] ou [de, sql, para]
3. corpo ← texto
4. para cada L em rota[0..n-2]:
       corpo ← MOVE_L(−1, corpo)    // emite na língua actual
       se L ≠ próximo e Q.a=1 na fronteira:
           corpo ← ponte_duo(corpo)  // sql_compilar ou sql_descompilar conforme sentido
5. corpo ← MOVE_rota[-1](+1, corpo) // absorve no destino
6. se Q.b=1 e para.r=0: opcional perna metal (shell/daemon)
```

### Ponte duo na fronteira SQL

Na arena wasm, a ponte entre representações é sempre uma das duas faces SQL:

- **para dentro** (texto legível → forma interna): `sql_compilar`
- **para fora** (forma interna → texto legível): `sql_descompilar`

HTML/LaTeX `(1,1,1)` partilham a forma interna em tags; SQL compila/descompila entre query e tags.

### Exemplos de paridade

| de → para | π(de) | π(para) | a | rota |
|-----------|-------|---------|---|------|
| sql → html | 1 | 1 | 0 | directa |
| sql → css | 1 | 0 | 1 | sql hub |
| css → js | 0 | 0 | 0 | directa |
| bash → sql | 0 | 1 | 1 | sql hub |
| bash → node | 0 | 0 | 0 | directa (mesma classe shell) |

**Nota:** a rota via hub usa `sql_compilar` ∘ `sql_descompilar` como par duo quando a
representação intermédia é SQL/tags. Backends com `q=0` (css, js) ainda não têm ponte
própria para essa forma — a gramática de paridade está certa; as arestas concretas são
extensões futuras no manifesto (`arestas` por par de línguas).

## Invariantes do par (s, p)

No par `{texto, ∂texto}` da dobra, o cruzado anula-se (Lema comutapar). Sobem só:

\[
s = x \oplus \partial x \quad\text{(soma directa)}\qquad
p = x \otimes \partial x \quad\text{(produto cruzado)}
\]

No pipe: `s` = conteúdo estável sob idempotência (`bump∘bump`, `STORE` repetido); `p` = par
negro·branco do canal (`S_negro · S_branco = 1`). A tradução **preserva** `s` quando `a=0` ou
quando `duo∘duo`; perde informação só onde o backend não tem a face (`q=0` não compila).

## Cadeia C → wasm → Node (duomorfismo no cruzado)

Referência: `fisica.tex` Def. `fis:def:duomorf` — dois bits `(a,b)`: operações (⊕↔⊗) e ordem (`r`).

```
interpretar.c  ──traduz sobe──►  node.wasm  ──MOVE(±1)──►  arena DISCO
     ▲                              │                        │
     └── traduz desce ──────────────┘                        │
                                                             │ b=1 (r muda)
                                                             ▼
                                                    canal S_CANAL+9120/9121
                                                             │
                                                             ▼
                                                    banco/node.c (pleno r=1)
                                                             │
                                                             ▼
                                                       Node (metal)
```

| Passagem | Tipo | Bit `a` | Bit `b` | Leitura |
|----------|------|---------|---------|---------|
| C ↔ wasm | **figura** (traduz) | — | — | `sobe(desce(M))=M` byte a byte |
| wasm célula ↔ Node pleno | **isomorfismo** na arena | 0 | 1 | mesma π; `r` cruza → fio canal |
| canal ↔ node | aresta manifesto | 0 | 1 | `Q01` — cruzado de ordem, não duo de operações |

**Capas `(p,q,r)`:**

| Capa | p | q | r | π |
|------|---|---|---|---|
| célula (`node.wasm`) | 1 | 1 | 0 | 0 |
| pleno (`banco/node.c`) | 1 | 1 | 1 | 1 |

`a(wasm→metal) = π(wasm) ⊕ π(metal) = 0` — preserva ⊕ e ⊗ na arena; `b = 1` porque `r` difere.

**Lei 1 na célula:** `node_move(−1)` emite (⊗), `node_move(+1)` absorve (⊕); na mesma língua com stdout preenchido, o par fecha.

**Medidores:** `tests/traduz_c_wasm_node.js` (§CN1–CN7) · `app/src/c_wasm_node.js` · `tests/duomorf_pipe.js` §D6.

## Cadeia ASM ↔ WASM (a mesma ISA, duas réguas)

A ISA ERG-64 tem **duas realizações** — não adaptadores:

```
texto .erg  ──monta──►  fita (bytecode)  ──sobe──►  isa.wasm + secção erg.fita
     ▲                        │                              │
     └── desmonta ◄── desce ──┘                              │
                                                              ▼
                                                    MOVE(1050) na arena
                                                    (mesma fita que erg corre)
```

| Passagem | Tipo | Leitura |
|----------|------|---------|
| asm ↔ fita | **monta/desmonta** (erg) | `monta∘desmonta=id` na fita |
| fita ↔ wasm | **figura** (secção custom) | `erg.fita` embutida no módulo MOVE |
| erg ↔ wasm exec | **isomorfismo** | `tests/isa_dupla.js` — mesma fita, slot a slot |

**Medidores:** `tests/traduz_asm_wasm.js` (§AW1–AW6) · `tests/isa_dupla.js` · `tools/asm_wasm.mjs`.

## Cadeia C → assembly → Node

```
interpretar.c  ──traduz sobe──►  node.wasm  ──wasm_para_erg──►  celula.erg
     ▲                               │                              │
     └── traduz desce ───────────────┘         node.erg + erg.fita   │
                                        └──── node_move(±1) ────────┘
                                                     │ b=1
                                                     ▼
                                            banco/node.c → Node
```

| Passagem | Ferramenta | Leitura |
|----------|------------|---------|
| C ↔ wasm | **traduz** sobe/desce | `sobe(desce(M))=M` — figura |
| wasm ↔ asm | **wasm_para_erg** (chessb §C4) | pilha wasm = par (A,B) em ERG |
| asm no módulo | **c_asm_node** embute `node.erg` | tradução completa no artefacto |
| wasm → Node | **absorveBackend** MOVE + canal | b=1, slots 9120/9121 |

**Medidores:** `tests/traduz_c_asm_node.js` (§CAN1–CAN7) · `tools/c_asm_shell.mjs` · `tests/traduz_c_wasm_node.js`.

## Cadeia C → assembly → shell (bash / node / powershell)

Mesma ponte que Node, generalizada em `lib/c_asm_shell.mjs`:

```
interpretar.c  ──traduz──►  *.wasm  ──wasm_para_erg──►  celula.erg
                                 └──── MOVE(±1) + canal ──► banco/*.c pleno
```

| Shell | wasm | secção asm | slots |
|-------|------|------------|-------|
| bash | `interpretar.wasm` | `bash.erg` | 9100/9101 |
| node | `node.wasm` | `node.erg` | 9120/9121 |
| powershell | `powershell.wasm` | `powershell.erg` | 9110/9111 |

```bash
node tools/c_asm_shell.mjs sobe bash
node tools/c_asm_shell.mjs sobe all
node tools/c_asm_shell.mjs corre bash "echo 42"
```

**Medidores:** `tests/traduz_c_asm_shell.js` (§CAS/CB) · `tests/traduz_c_asm_node.js`.

## O que NÃO é duomorfismo no pipe

- **Bijeção** (`f` reversível) ≠ isomorfismo — MOVE pode ser bijecção sem preservar ⊕/⊗.
- **Canal bump** — transporte nilpotente; não troca operações, só polaridade de medida.
- **Traduz C→wasm** — reexprime a figura na outra régua; `sobe∘desce=iso` atesta (não é passagem duo entre línguas-backend).
- **Par `(p,q,r)` completo** — três bits de capacidade; só `(p+q+r) mod 2` classifica `a`.
- **Diluição cosmológica** (`fis:thm:diluicao`) — duomorfismo *aplicação* (log: soma→produto), **não** o operador \(\mathcal{D}\) (\(\mathcal{D}^2=\mathrm{id}\)).
- **Quarteto \(I_1,I_2\)** — duas involuções tipo \(\dagger\) (preservam soma); não são a troca ⊕↔⊗.

## Medidores

| Teste | O quê |
|-------|-------|
| `tools/test_metal.exe` | C→wasm→ERG→corre; passos node/bash/ps |
| `tests/corre_fita_metal.c` | erg.fita embutida no wasm → stdout 42 |
| `tests/corre_fita_browser.js` | mesma fita via WebAssembly API (opcional, Node) |
| `tests/celula_wasm_sec.c` | secções `*.erg` + `erg.fita` no wasm pós-`gera_nucleo` |
| `tools/test_cadeia.bat` | suite completa (~100+ asserts) |
| `tests/traduz_c_wasm_node.js` | cadeia C→wasm→node, §CN7 duomorf arena |
| `tests/duomorf_pipe.js` | paridade, rota, roundtrip wasm, §D6 cadeia |
| `tests/interface_padrao.js` | hub SQL, involução compilar† |
| `tests/canal_watcher.js` | par negro·branco = 1 |
| `app/src/c_wasm_shell.js` | `paridadeWasmMetal`, `traduzCadeia` |
| `tests/cosmologia.c` | diluição \(r a^{3(1+w)}=C\); taxa \(H^2=1/D\); órbita de quatro |
| `app/src/banco_absorve.js` | `motor=auto\|wasm\|fita`, meta passos/fitaLen |
| `app/src/banco_tradutor.js` | API: `paridade`, `caminho`, `traduzWasm` |

## Ligação ao barramento

```
┌──────────┐  MOVE(−1)   ┌─────────┐  Q_ab   ┌─────────┐  MOVE(+1)   ┌──────────┐
│ Lang A   │ ──────────► │  arena  │ ──────► │  hub?   │ ──────────► │  Lang B  │
│ wasm     │   negro     │  DISCO  │  duo?   │  SQL    │   branco    │  wasm    │
└──────────┘             └─────────┘         └─────────┘             └──────────┘
      │                                                                    │
      └──────────────────── S_CANAL (bump 6B) ────────────────────────────┘
```

A tradução entre línguas **não** passa pelo canal — o canal leva polaridade entre bancos.
A tradução passa pela arena DISCO e pelas faces ⊕/⊗ de cada backend. O canal sincroniza
**quando** cada lado faz o seu MOVE, não **o quê** o MOVE contém.

## Ver também

- [`redes-pipe.md`](redes-pipe.md) — ponte Redes (`fisica.tex` / `multifocal.tex`): árvore \(G=1\) no pipe vs Hopfield \(G>1\) (futuro); **não** mistura os dois mecanismos.
