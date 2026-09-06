# CAN + J1939 + TP sobre o Microprocessador Fractal

Modelo digital de referência: o **Microprocessador Fractal** processa grandezas
extraídas de frames **CAN/J1939**, inclusive mensagens longas via **J1939-TP**,
**sem incorporar protocolo à ISA**.

```text
                    CAN / J1939 curto ──────┐
                                            ├→ mensagem semântica → MICRO
                    CAN / J1939 TP ─────────┘
```

---

## Arquitetura

```text
api.h / micro.h / micro.c     ISA + máquina (MOVE / ALU / DIV)
         │
        I/O
         │
can.h / can.c                 controlador CAN (frame, arb, CRC, stuff, ACK, TEC)
         │
can_bus.h / can_bus.c         barramento multi-nó
         │
j1939.h / j1939.c             decodificação J1939 + interpretação de sinais
         │
         ├──── mensagem curta (≤ 8 B) ─────────────┐
         │                                         │
         └→ j1939_tp (BAM / RTS-CTS / DT)          │
                 → reassembly → mensagem completa ─┤
                                                   ▼
                                            can_micro.c
                                          (única composição)
                                                   │
                                                   ▼
                                                 MICRO
```

Os dois caminhos (curto e TP) convergem para a **mesma representação semântica**
antes de entrar no Microprocessador Fractal.

### Regras de dependência (congeladas)

```text
micro.c      NÃO conhece CAN
can.c        NÃO conhece J1939
can_bus.c    NÃO conhece J1939
j1939.c      NÃO conhece micro
j1939_tp.c   NÃO conhece micro
can_micro.c  conhece as interfaces e compõe
```

---

## Status das camadas

| Camada        | Status              |
|---------------|---------------------|
| `micro.c`     | **FECHADO**         |
| `can.c`       | **FECHADO**         |
| `can_bus.c`   | **FECHADO**         |
| `j1939.c`     | **FECHADO**         |
| `j1939_tp.c`  | **AUDITADO / FECHADO** |
| `can_micro.c` | integrador          |

Snapshots:

```text
.snapshot/can-j1939-closed/       CAN + J1939 + Micro
.snapshot/can-j1939-tp-micro/     + TP + integração MICRO
```

### Camada de catálogo (nova, pós-freeze)

```text
j1939.c         = gramática do protocolo
j1939_catalog.c = significado dos sinais (crescível)
```

### GAPs registrados (não implementados)

```text
timeouts T1–T4 J1939-TP
API TX de Abort
CTS multi-janela avançada
Address Claim
Request / Response
Diagnóstico J1939
```

---

## Arquivos

| Arquivo            | Papel |
|--------------------|--------|
| `api.h`            | ISA canônica |
| `micro.h` / `micro.c` | Máquina de referência |
| `can.h` / `can.c`  | Controlador CAN |
| `can_bus.h` / `can_bus.c` | Barramento multi-nó |
| `j1939.h` / `j1939.c` | Decodificação J1939 + interpretação de sinais |
| `j1939_tp.h` / `j1939_tp.c` | Transport Protocol |
| `j1939_catalog.h` / `j1939_catalog.c` | Catálogo PGN/SPN (significado dos sinais) |
| `j1939_catalog_demo.c` | Demo do catálogo sem micro |
| `j1939_tp_demo.c`  | Auditoria isolada do TP |
| `can_micro.c`      | Composição + testes de integração |
| `so_cristal.c`     | Validação analógica da tríade (separada) |

---

## Fluxos demonstrados

### 1. Mensagem curta (≤ 8 bytes)

```text
CAN frame → ID 29-bit → PGN → SPN → raw → escala → MICRO → grandeza
```

### 2. Mensagem longa (J1939-TP)

```text
CM.BAM / RTS+CTS → DT×N → reassembly → mensagem → PGN → SPN → raw → MICRO
```

### 3. Convergência

O caminho curto e o caminho TP, para o mesmo sinal, produzem:

```text
mesmo PGN, mesmo SPN, mesmo raw, mesmo resultado no MICRO
```

---

## Isolamento estrutural

Cada módulo compila **standalone** (sem puxar o Micro nem camadas superiores):

```bash
# prova de isolamento (sem linkar can_micro)
cc -c -std=c11 -I. micro.c
cc -c -std=c11 -I. can.c
cc -c -std=c11 -I. can_bus.c
cc -c -std=c11 -I. j1939.c
cc -c -std=c11 -I. j1939_tp.c
```

Somente `can_micro.c` inclui e compõe todas as interfaces.

## Compilação

```bash
# suite completa (CAN + J1939 + TP + MICRO)
cc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. \
   can_micro.c can.c can_bus.c j1939.c j1939_tp.c j1939_catalog.c micro.c -lm -o can_micro
./can_micro

# auditoria isolada do TP
cc -O2 -std=c11 -Wall -I. \
   j1939_tp_demo.c j1939_tp.c j1939.c can.c -o j1939_tp_demo
./j1939_tp_demo

# micro isolado
cc -O2 -std=c11 -Wall -I. micro.c -lm -o micro
./micro
```

Windows (CMD):

```cmd
gcc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. ^
  can_micro.c can.c can_bus.c j1939.c j1939_tp.c j1939_catalog.c micro.c -lm -o can_micro.exe
can_micro.exe
```

---

## Testes de integração (can_micro)

```text
CAN protocol audit          resid 0
J1939 decode                resid 0
TP BAM 138 B → MICRO        resid 0
TP RTS/CTS 138 B → MICRO    resid 0
TP incompleto não vaza      resid 0
Convergência multi-PGN/SPN  resid 0
Frame curto SPN 110 → 20°C  resid 0
```

---

## Exemplos de sinais

| PGN   | SPN | Nome                         | Escala (modelo)        |
|-------|-----|------------------------------|------------------------|
| 65262 | 110 | Engine Coolant Temperature   | raw/32 − 40 °C         |
| 61444 | 190 | Engine Speed                 | raw × 0.125 rpm        |
| 65265 | 84  | Wheel-Based Vehicle Speed    | raw / 256 km/h         |

Sinais adicionais usados nos testes de convergência são definidos **apenas** no
integrador (`can_micro.c`), sem alterar `j1939.c`.

---

## Princípio

> O Microprocessador Fractal não sabe que existe CAN, J1939 ou TP.
> Ele recebe estado e transforma estado (MOVE + ALU).
> Protocolo e semântica ficam nas camadas externas; a composição é `can_micro.c`.
