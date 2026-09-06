# CAN + J1939 sobre o Microprocessador Fractal

Modelo digital de referência para integração entre o **Microprocessador Fractal**, um controlador **CAN** e a camada semântica **J1939**.

O projeto separa explicitamente:

```text
ISA / máquina
      ↓
   micro.c
      ↓ I/O
    can.c
      ↓ frame
   j1939.c
      ↓
 grandeza física
```

O objetivo é demonstrar que um frame CAN/J1939 pode ser recebido, interpretado e transformado em uma grandeza física utilizando a máquina de referência do Microprocessador Fractal, **sem incorporar CAN ou J1939 à ISA**.

---

## Arquitetura

```text
┌──────────────────────────────┐
│          api.h               │
│       ISA canônica           │
│ ADD / SUB / MUL / ...        │
└──────────────┬───────────────┘
               │
┌──────────────▼───────────────┐
│          micro.c             │
│  modelo de referência        │
│                              │
│  MOVE / ALU / DIV            │
│  memória / PC / instr 80-bit │
└──────────────┬───────────────┘
               │ I/O
┌──────────────▼───────────────┐
│           can.c              │
│   controlador CAN digital    │
│                              │
│  CanFrame                    │
│  RX / TX                     │
│  mailbox                     │
│  validação                   │
└──────────────┬───────────────┘
               │ frame
┌──────────────▼───────────────┐
│          j1939.c             │
│      semântica J1939         │
│                              │
│  Priority / DP / PF / PS     │
│  SA / DA / PGN               │
│  SPN / escala                │
└──────────────┬───────────────┘
               │
               ▼
        grandeza física
```

### Regra de dependência

```text
micro.c   NÃO conhece CAN
can.c     NÃO conhece J1939
j1939.c   NÃO conhece o micro
can_micro.c conhece as três camadas
```

Essa separação permite testar e evoluir cada camada independentemente.

---

# Arquivos

| Arquivo       | Responsabilidade                                 |
| ------------- | ------------------------------------------------ |
| `api.h`       | ISA canônica                                     |
| `micro.h`     | Interface pública da máquina                     |
| `micro.c`     | Modelo digital de referência do microprocessador |
| `can.h`       | Interface do controlador CAN                     |
| `can.c`       | Modelo digital de RX/TX CAN                      |
| `j1939.h`     | Interface da semântica J1939                     |
| `j1939.c`     | Decodificação J1939 e escala de sinais           |
| `can_micro.c` | Integração e testbench                           |

---

# `micro.c`

O Microprocessador Fractal permanece independente do protocolo de comunicação.

Sua abstração fundamental é:

```text
MOVE  → transferência de estado
ALU   → transformação
HALT  → controle do modelo
```

A instrução de 80 bits utiliza:

```text
24 bits  endereço A
24 bits  endereço B
 8 bits  operação
24 bits  endereço resultado
────────────────────────────
80 bits
```

A camada CAN/J1939 utiliza a máquina através de sua interface pública em `micro.h`.

**Nenhuma instrução CAN ou J1939 foi adicionada à ISA.**

---

# `can.c`

`can.c` representa o controlador CAN como um periférico digital.

Ele trabalha somente com frames CAN e não interpreta seu significado.

## Frame

```c
typedef struct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  dlc;
    uint8_t  extended;
    uint8_t  rtr;
} CanFrame;
```

Suporta:

```text
CAN standard  → 11-bit ID
CAN extended  → 29-bit ID
DLC            → 0..8
RTR            → 0 ou 1
```

## RX

Um frame pode ser injetado no controlador:

```text
can_inject_rx()
        ↓
      RX
        ↓
     can_rx()
```

A injeção representa um frame que já chegou do barramento.

Não existe PHY real neste modelo.

## TX

A transmissão é modelada por uma mailbox:

```text
can_tx()
   ↓
TX pending
   ↓
can_tx_done()
```

O modelo não transmite eletricamente nenhum frame.

---

# `j1939.c`

A camada J1939 interpreta um CAN extended frame de 29 bits.

O identificador é dividido em:

```text
28..26  Priority       3 bits
25      Reserved       1 bit
24      Data Page      1 bit
23..16  PDU Format     8 bits
15..8   PDU Specific   8 bits
7..0    Source Addr    8 bits
```

Visualmente:

```text
┌────────┬───┬───┬────────┬────────┬────────┐
│Priority│ R │ DP│   PF   │   PS   │   SA   │
│  3 bit │1b │1b │ 8 bit  │ 8 bit  │ 8 bit  │
└────────┴───┴───┴────────┴────────┴────────┘
```

## PDU1

Quando:

```text
PF < 240
```

temos PDU1.

Nesse formato:

```text
PS = Destination Address
```

e o PGN é:

```text
PGN = DP : PF : 00
```

## PDU2

Quando:

```text
PF >= 240
```

temos PDU2.

Nesse formato:

```text
PS
```

faz parte do PGN:

```text
PGN = DP : PF : PS
```

---

# SPN

Um sinal J1939 é descrito por `J1939Signal`.

O descritor contém:

```text
PGN
SPN
posição inicial
comprimento
offset
scale_num
scale_den
nome
unidade
```

A representação permite descrever sinais de:

```text
1..8 bytes
```

com leitura little-endian.

A transformação física é:

```text
physical =
    raw * scale_num / scale_den
    + offset
```

ou, de forma racional:

```text
physical =
    (raw * scale_num + offset * scale_den)
    / scale_den
```

A aritmética da camada J1939 utiliza representação assinada.

Isso é deliberado:

```text
ISA / micro
    ↓
u64

J1939 / grandeza física
    ↓
int64_t
```

A semântica de valores negativos, portanto, não é incorporada à ISA.

---

# Exemplo: SPN 110

O exemplo integrado utiliza:

```text
PGN = 65262
SPN = 110
```

**Engine Coolant Temperature**

Descrição:

```text
resolution = 1/32 °C/bit
offset     = -40 °C
```

Portanto:

```text
T = raw / 32 - 40
```

Para:

```text
DATA = 80 07
```

a leitura little-endian produz:

```text
raw = 0x0780
    = 1920
```

Então:

```text
T = 1920 / 32 - 40
  = 60 - 40
  = 20 °C
```

O mesmo valor é obtido na integração utilizando as operações do micro.

---

# Integração

`can_micro.c` funciona como integrador e testbench.

A sequência é:

```text
frame
  ↓
can_inject_rx()
  ↓
can_rx()
  ↓
J1939 ID decode
  ↓
PGN
  ↓
SPN
  ↓
extração dos bits
  ↓
micro.c
  ├── MOVE / STORE
  ├── ALU
  └── DIV
  ↓
grandeza física
```

O teste de integração utiliza:

```text
ID       = 0x0CFEEE00
PGN      = 65262
SA       = 0
DATA     = 80 07 ...
SPN      = 110
resultado = 20 °C
```

---

# Testes

A suíte verifica:

### CAN

```text
✓ inicialização
✓ frame standard
✓ frame extended
✓ validação de ID
✓ validação de DLC
✓ validação de RTR
✓ RX
✓ TX
✓ mailbox ocupada
✓ RX overflow
```

### J1939

```text
✓ campos do identificador
✓ priority
✓ reserved
✓ data page
✓ PF
✓ PS
✓ SA
✓ PDU1
✓ PDU2
✓ destination address
✓ cálculo do PGN
✓ validação de frame J1939
✓ leitura little-endian
✓ sinais de 1..8 bytes
✓ escala racional
✓ escala assinada
✓ valores negativos
```

### Integração

```text
✓ CAN → J1939
✓ J1939 → SPN
✓ SPN → raw
✓ raw → micro
✓ micro → escala
✓ resultado = 20 °C
```

Teste explícito de escala negativa:

```text
raw = 0
```

produz:

```text
-40 °C
```

---

# Compilação

No Windows/MinGW:

```cmd
gcc -O2 -std=c11 -Wall -DMICRO_AS_LIB -I. ^
  can_micro.c can.c j1939.c micro.c -lm -o can_micro.exe
```

Executar:

```cmd
can_micro.exe
```

Resultado esperado:

```text
[OK] can + j1939 resid 0
```

seguido da integração J1939:

```text
SPN 110: Engine Coolant Temperature = 20 deg C
```

---

# Escopo do modelo

Este projeto é um **modelo digital de referência arquitetural**.

Ele não pretende representar literalmente a camada física CAN.

Não são modelados neste estágio:

```text
CANH / CANL
transceptor físico
níveis diferenciais
temporização elétrica
bit timing físico
```

O controlador trabalha na abstração:

```text
frame CAN
```

Isso permite manter a separação entre:

```text
ISA
periférico
protocolo
semântica
grandeza física
```

---

# Estado do projeto

```text
api.h / micro.h / micro.c
        → FECHADO

can.h / can.c
        → FECHADO

j1939.h / j1939.c
        → FECHADO

can_micro.c
        → INTEGRADOR + TESTBENCH
```

## Arquitetura consolidada

```text
                 ISA
                  │
                  ▼
              micro.c
                  │
                 I/O
                  │
                  ▼
                CAN
                  │
                frame
                  │
                  ▼
               J1939
                  │
              PGN / SPN
                  │
                  ▼
          grandeza física
```

A camada de comunicação permanece **externa à ISA**.

O Microprocessador Fractal não precisa conhecer CAN ou J1939 para executar a transformação de estado. CAN fornece o frame; J1939 fornece a semântica; o micro realiza a transformação.
