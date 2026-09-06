# CAN + J1939 + TP + Catálogo + Sinal → Microprocessador Fractal

```text
CAN + J1939 + TP  OVER  MICROPROCESSOR FRACTAL
ARCHITECTURAL BASELINE  v1.0
```

> **O protocolo é externo à ISA.**
>
> CAN, J1939 e J1939-TP realizam transporte e interpretação de dados.
> A camada de composição converte esse resultado em grandezas semânticas.
> O Microprocessador Fractal recebe apenas essas grandezas e executa suas
> operações, **sem conhecer CAN, J1939 ou J1939-TP**.

$$
\boxed{\text{CAN/J1939/TP} \rightarrow \text{semântica} \rightarrow \text{MICRO}}
$$

**Baseline intocável:** `.snapshot/arch-closed/`

---

Modelo digital de referência: o **Microprocessador Fractal** consome **grandezas
já interpretadas**, sem conhecer CAN, J1939, TP ou catálogo.

---

## Princípio arquitetural

> **Transport independence:** a camada semântica recebe uma mensagem J1939 completa
> independentemente de ela ter sido transportada por um frame CAN curto ou por
> J1939-TP. A camada de execução recebe apenas a grandeza interpretada.

```text
CAN/J1939 curto ──┐
                  ├──→ interpretação genérica ─→ grandeza ─→ MICRO
J1939-TP ─────────┘
```

O Micro **não** precisa saber PGN, SPN, origem (curto/TP), reassembly ou escala.

---

## Cadeia de responsabilidades

| Módulo | Responsabilidade | Conhece |
|--------|------------------|---------|
| `can.c` | COMO o frame CAN funciona | — |
| `can_bus.c` | COMO o barramento arbitra | can |
| `j1939.c` | COMO decodificar o ID/payload | can frame |
| `j1939_tp.c` | COMO transportar >8 bytes | can, j1939 |
| `j1939_catalog.c` | O QUE o sinal significa (só dados) | j1939 tipos |
| `j1939_signal.c` | COMO extrair/converter números | catalog, j1939 |
| `can_micro.c` | COMO compor tudo | todas as interfaces |
| `micro.c` | COMO executar transformação | api.h |

**Camadas congeladas:** micro, can, can_bus, j1939, j1939_tp.  
**Validado:** catalog, signal.  
**Integrador:** can_micro.

---

## Fluxo end-to-end

```text
CAN frame
   ↓
J1939 decode (ID → PGN)
   ↓
short  ──ou──  TP → reassembly → mensagem completa
   ↓
catalog_find(PGN, SPN) → J1939Signal*
   ↓
signal_extract → raw (+ NA/signed/endian)
   ↓
signal_physical → grandeza
   ↓
MICRO (MOVE / ALU / DIV)
```

Demonstrado:

```text
SPN 110 → 20 °C
SPN 190 → 1000 rpm
SPN 84  → 10 km/h
NA      → bloqueado (não entra no MICRO)
short   ≡ TP
```

---

## Contratos de API

### `j1939_catalog_find(pgn, spn)`

| | |
|--|--|
| **Entrada** | PGN, SPN |
| **Pré** | — |
| **Saída** | `const J1939Signal*` ou `NULL` |
| **Erros** | `NULL` se ausente |
| **Chama** | ninguém (só tabela) |
| **Não conhece** | CAN, TP, MICRO, payload |

`J1939Signal*` = **descrição** do sinal (metadados).

---

### `j1939_signal_extract(payload, len, interp, &raw, &status)`

| | |
|--|--|
| **Entrada** | bytes da mensagem + `J1939Interp` (aponta ao signal + signed/LE/NA) |
| **Pré** | `payload` cobre `start_byte..length`; `interp->sig` válido |
| **Saída** | `raw` (int64), `status` ∈ {OK, NA, ERROR_RANGE} |
| **Erros** | `J1939_SIG_ERR` (oob, args nulos) |
| **Não conhece** | CAN bus, TP, MICRO, catálogo (só o ponteiro `sig`) |

`raw` = **representação extraída** do campo.

---

### `j1939_signal_physical(raw, interp, &phys)`

| | |
|--|--|
| **Entrada** | `raw`, metadados de escala/offset via `interp->sig` |
| **Pré** | `scale_den ≠ 0`; status anterior era OK (não NA) |
| **Saída** | `phys` (int32) ≈ `raw * num/den + offset` |
| **Erros** | overflow / args inválidos |
| **Não conhece** | CAN, TP, MICRO |

`physical` = **grandeza interpretada**.

---

### `j1939_tp_*` (BAM / RTS-CTS / DT)

| | |
|--|--|
| **Entrada** | frames CAN CM/DT ou payload a fragmentar |
| **Saída** | frames ou mensagem reassembled + PGN |
| **Não conhece** | MICRO, catalog, signal |

---

### Micro (`LOAD` / `STORE` / `roda80` / ALU)

| | |
|--|--|
| **Entrada** | estado em memória (valores numéricos) |
| **Saída** | estado transformado |
| **Não conhece** | CAN, J1939, TP, catalog, signal |

MICRO = **consumidor da grandeza** (ou de raw já preparado pelo integrador).

---

### `can_micro.c` (composição)

Único módulo autorizado a orquestrar:

```text
catalog_find → signal_extract → signal_physical → micro
```

e a provar `short ≡ TP`.

---

## Demonstração narrativa

```bash
cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
   demo_e2e.c can.c j1939.c j1939_tp.c j1939_catalog.c j1939_signal.c micro.c -lm \
   -o demo_e2e
./demo_e2e
```

Conta a história completa: CAN short → TP/BAM → convergência → isolamento → `END-TO-END: PASS`.

## Compilação

```bash
# suite completa
cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
  can_micro.c can.c can_bus.c j1939.c j1939_tp.c \
  j1939_catalog.c j1939_signal.c micro.c -lm -o can_micro
./can_micro

# isolamento por camada
cc -c -std=c11 -I. micro.c
cc -c -std=c11 -I. can.c
cc -c -std=c11 -I. can_bus.c
cc -c -std=c11 -I. j1939.c
cc -c -std=c11 -I. j1939_tp.c
cc -c -std=c11 -I. j1939_catalog.c
cc -c -std=c11 -I. j1939_signal.c

# demos isolados
cc -O2 -std=c11 -I. j1939_tp_demo.c j1939_tp.c j1939.c can.c -o j1939_tp_demo
cc -O2 -std=c11 -I. j1939_catalog_demo.c j1939_catalog.c j1939.c can.c -o j1939_catalog_demo
cc -O2 -std=c11 -I. j1939_signal_demo.c j1939_signal.c j1939_catalog.c j1939.c can.c -o j1939_signal_demo
```

---

## Status

```text
arch-closed  =  baseline de referência (não reabrir)
demo_e2e.c   =  demonstração narrativa
catálogo     =  crescimento de dados (não de arquitetura)
```

**Fase de engenharia do núcleo: encerrada.**  
Próximo valor: publicação / demonstração / catálogo como dado.

## Snapshots

```text
.snapshot/can-j1939-tp-micro/   FECHADO DEFINITIVO
.snapshot/j1939-catalog/        VALIDADO
.snapshot/j1939-signal/         VALIDADO
.snapshot/arch-closed/          ARQUITETURA FUNCIONAL FECHADA
```

### GAPs (não implementados de propósito)

```text
timeouts T1–T4 TP
Address Claim / Request-Response / Diagnostics
CTS multi-janela avançada
catálogo massivo (crescimento de dados, não de arquitetura)
```

---

## Separação de conceitos

```text
J1939Signal*   = descrição do sinal
raw            = representação extraída
physical       = grandeza interpretada
MICRO          = consumidor da grandeza
```
