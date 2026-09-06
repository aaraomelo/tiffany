# CAN + J1939 sobre o Microprocessador Fractal

Modelo digital de referência para integração entre o **Microprocessador Fractal**, um controlador **CAN**, o **barramento CAN** e a camada semântica **J1939**.

O projeto separa explicitamente:

```text
                         ISA / máquina
                              │
                              ▼
                           micro.c
                              │
                             I/O
                              │
                              ▼
                            can.c
                              │
                              ▼
                         can_bus.c
                              │
                           frame CAN
                              │
                              ▼
                           j1939.c
                              │
                              ▼
                       grandeza física


                    ┌──────────────────┐
                    │   can_micro.c    │
                    │ integração /     │
                    │ testbench        │
                    └──────────────────┘
                       camada de composição
```

O objetivo é demonstrar que um frame CAN/J1939 pode ser recebido, arbitrado, validado, interpretado e transformado em uma grandeza física utilizando a máquina de referência do Microprocessador Fractal, **sem incorporar CAN ou J1939 à ISA**.

---

# Arquitetura

```text
┌──────────────────────────────┐
│           api.h              │
│        ISA canônica          │
│   ADD / SUB / MUL / ...      │
└──────────────┬───────────────┘
               │
┌──────────────▼───────────────┐
│           micro.c            │
│     modelo de referência     │
│                              │
│     MOVE / ALU / DIV         │
│     memória / PC / instr 80b │
└──────────────┬───────────────┘
               │
              I/O
               │
┌──────────────▼───────────────┐
│            can.c             │
│      controlador CAN         │
│                              │
│ frame / arbitragem / CRC     │
│ stuffing / ACK / erros       │
│ TEC / REC / BUS_OFF          │
└──────────────┬───────────────┘
               │
┌──────────────▼───────────────┐
│          can_bus.c            │
│       barramento CAN         │
│                              │
│       múltiplos nós          │
│       arbitragem             │
│       can_bus_tick()         │
└──────────────┬───────────────┘
               │
             frame
               │
┌──────────────▼───────────────┐
│           j1939.c            │
│        semântica J1939       │
│                              │
│ Priority / DP / PF / PS      │
│ SA / DA / PGN                │
│ SPN / escala                 │
└──────────────┬───────────────┘
               │
               ▼
        grandeza física
```

A integração das camadas é realizada por `can_micro.c`, que funciona como **compositor e testbench**, sem criar uma dependência estrutural entre os módulos.

---

# Regra de dependência

```text
micro.c      NÃO conhece CAN

can.c        NÃO conhece J1939

can_bus.c    NÃO conhece J1939

j1939.c      NÃO conhece o micro

can_micro.c  conhece as interfaces
             para integração e testes
```

Essa separação permite testar e evoluir cada camada independentemente.

A arquitetura pode ser entendida como:

```text
ISA
 ↓
máquina
 ↓
I/O
 ↓
protocolo
 ↓
semântica
 ↓
grandeza física
```

---

# Arquivos

| Arquivo       | Responsabilidade                                         |
| ------------- | -------------------------------------------------------- |
| `api.h`       | ISA canônica                                             |
| `micro.h`     | Interface pública da máquina                             |
| `micro.c`     | Modelo digital de referência do microprocessador         |
| `can.h`       | Interface do controlador CAN                             |
| `can.c`       | Frame, arbitragem, CRC, stuffing, ACK e máquina de erros |
| `can_bus.h`   | Interface do barramento CAN                              |
| `can_bus.c`   | Modelo multi-nó e arbitragem do barramento               |
| `j1939.h`     | Interface da semântica J1939                             |
| `j1939.c`     | Decodificação J1939 e escala de sinais                   |
| `can_micro.c` | Integração e testbench                                   |

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

A comunicação permanece sendo uma camada externa à máquina.

---

# `can.c`

`can.c` representa o controlador CAN como um modelo digital de referência.

Diferentemente de uma simples mailbox RX/TX, o controlador modela elementos essenciais do protocolo CAN:

```text
frame
 │
 ├── validação
 ├── arbitragem
 ├── bit stuffing
 ├── CRC-15
 ├── ACK
 └── tratamento de erros
```

## Frame CAN

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
CAN standard   → 11-bit ID
CAN extended   → 29-bit ID
DLC            → 0..8
RTR            → 0 ou 1
```

---

# Arbitragem

O modelo utiliza a regra fundamental do CAN:

```text
dominante = 0
recessivo = 1

0 vence 1
```

A arbitragem ocorre bit a bit.

Como consequência, identificadores numericamente menores possuem maior prioridade.

Exemplo:

```text
ECU1 → 0x300
ECU2 → 0x100
ECU3 → 0x200
```

A ordem de vitória é:

```text
0x100
  ↓
0x200
  ↓
0x300
```

O nó que transmite um bit recessivo e observa um bit dominante perde a arbitragem e abandona a transmissão.

A arbitragem é **não destrutiva**: o frame vencedor continua sendo transmitido sem corrupção causada pela disputa.

---

# Bit stuffing

O controlador modela o mecanismo de bit stuffing utilizado pelo CAN.

Após cinco bits consecutivos com o mesmo valor, um bit complementar é inserido:

```text
11111 → 11111 0

00000 → 00000 1
```

O estado da sequência é mantido explicitamente durante o processo de stuffing.

O modelo também possui destuffing e detecção de `stuff error`.

É testado o round-trip:

```text
bits
 ↓
stuff
 ↓
destuff
 ↓
bits originais
```

---

# CRC

O frame possui verificação por **CRC-15 CAN**.

O modelo permite verificar:

```text
frame original
     ↓
    CRC
     ↓
frame válido
```

e detectar corrupção:

```text
frame
 ↓
bit alterado
 ↓
CRC mismatch
 ↓
erro
```

A suíte verifica tanto a estabilidade do CRC quanto a detecção de corrupção.

---

# ACK

O modelo também representa o comportamento lógico do ACK.

Quando existe um receptor/ouvintes no barramento:

```text
transmissor
    │
    ▼
  frame
    │
    ▼
 receptor
    │
    ▼
   ACK
```

Sem um nó capaz de reconhecer o frame:

```text
TX
 ↓
sem ACK
 ↓
ACK_ERR
 ↓
TEC += 8
```

O frame permanece pendente no transmissor em caso de `ACK_ERR`, permitindo que o estado de transmissão seja observado pelo testbench.

---

# Máquina de erros

O controlador mantém:

```text
TEC = Transmit Error Counter

REC = Receive Error Counter
```

e os estados:

```text
ERROR_ACTIVE
      │
      │ erros
      ▼
ERROR_PASSIVE
      │
      │ erros adicionais
      ▼
BUS_OFF
```

O limite de `TEC` é modelado até:

```text
TEC = 256
```

permitindo a transição para:

```text
BUS_OFF
```

A recuperação é explicitamente modelada por:

```c
can_recover()
```

que permite sair do estado `BUS_OFF` conforme o contrato do modelo.

---

# `can_bus.c`

`can_bus.c` representa o barramento lógico e permite múltiplos nós CAN.

Conceitualmente:

```text
                 CAN BUS

                  │
          ┌───────┼───────┐
          │       │       │
        ECU1    ECU2    ECU3
          │       │       │
          └───────┴───────┘
```

Cada nó possui seu controlador CAN.

O barramento recebe as transmissões pendentes e executa a arbitragem.

A evolução temporal do modelo é realizada por:

```c
can_bus_tick()
```

Exemplo:

```text
ECU1 = 0x300
ECU2 = 0x100
ECU3 = 0x200

tick 1 → 0x100
tick 2 → 0x200
tick 3 → 0x300
```

A fila é, portanto, esvaziada de acordo com a prioridade CAN.

`can_bus.c` não interpreta PGN, SPN ou qualquer semântica J1939.

---

# `j1939.c`

A camada J1939 interpreta um CAN extended frame de 29 bits.

O identificador é dividido em:

```text
28..26  Priority        3 bits
25      Reserved        1 bit
24      Data Page       1 bit
23..16  PDU Format      8 bits
15..8   PDU Specific    8 bits
7..0    Source Addr     8 bits
```

Visualmente:

```text
┌────────┬───┬───┬────────┬────────┬────────┐
│Priority│ R │ DP│   PF   │   PS   │   SA   │
│ 3 bit  │1b │1b │ 8 bit  │ 8 bit  │ 8 bit  │
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

e:

```text
PGN = DP : PF : 00
```

## PDU2

Quando:

```text
PF >= 240
```

temos PDU2.

Nesse formato, `PS` participa do PGN:

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

São suportados sinais de:

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

ou:

```text
physical =
    (raw * scale_num + offset * scale_den)
    / scale_den
```

A aritmética da camada J1939 utiliza representação assinada para a grandeza física.

Isso mantém a separação:

```text
ISA / micro
    ↓
u64

J1939 / grandeza física
    ↓
representação assinada
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

Parâmetros:

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

O mesmo resultado é obtido através da máquina do Microprocessador Fractal.

---

# Integração

`can_micro.c` funciona exclusivamente como **integrador e testbench**.

Ele compõe as interfaces das camadas sem alterar suas responsabilidades.

A sequência lógica é:

```text
                 CAN BUS
                    │
                    ▼
                 can.c
                    │
                 frame
                    │
                    ▼
                 j1939.c
                    │
                ID decode
                    │
                    ▼
                   PGN
                    │
                    ▼
                   SPN
                    │
                    ▼
              extração raw
                    │
                    ▼
                 micro.c
              ┌─────┼─────┐
              │     │     │
            MOVE   ALU   DIV
              │     │     │
              └─────┼─────┘
                    │
                    ▼
             grandeza física
```

O teste de integração utiliza:

```text
ID       = 0x0CFEEE00
extended = 1
DLC      = 8

PGN      = 65262
SA       = 0x00

DATA     = 80 07 00 00 00 00 00 00

SPN      = 110
raw      = 1920

resultado = 20 °C
```

Resultado:

```text
J1939 = 20 °C
MICRO = 20 °C
```

---

# Testes

## CAN

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

## Protocolo CAN

```text
✓ arbitragem 0x100 < 0x200 < 0x300
✓ empate com mesmo ID
✓ arbitragem standard
✓ arbitragem extended
✓ DLC 0..8
✓ rejeição de DLC 9
✓ bit stuffing 11111
✓ bit stuffing 00000
✓ destuff round-trip
✓ detecção de stuff error
✓ CRC estável
✓ detecção de corrupção
✓ ACK com receptor
✓ ACK_ERR sem receptor
✓ TEC
✓ REC
✓ ERROR_ACTIVE
✓ ERROR_PASSIVE
✓ BUS_OFF
✓ recover
✓ multi-nó
✓ can_bus_tick()
✓ fila em ordem de prioridade
```

## J1939

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

## Integração

```text
✓ CAN → J1939
✓ J1939 → PGN
✓ PGN → SPN
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
  can_micro.c can.c can_bus.c j1939.c micro.c -lm -o can_micro.exe
```

Executar:

```cmd
can_micro.exe
```

Resultado esperado:

```text
=== auditoria CAN / J1939 ===

  [can]        resid 0
  [j1939]      resid 0
  [protocol]   resid 0

[OK] can + j1939 + protocol resid 0
```

seguido da integração:

```text
CAN  ID=0x0CFEEE00 DLC=8

J1939 PGN=65262 pri=3 SA=0x00

SPN 110 raw=1920
J1939=20 C
MICRO=20 C

RESULTADO:
Engine Coolant Temperature = 20 deg C
```

---

# Escopo do modelo

Este projeto é um **modelo digital de referência arquitetural**.

Ele representa logicamente elementos do protocolo CAN, incluindo:

```text
frame
arbitragem
bit stuffing
CRC
ACK
detecção de erros
TEC / REC
ERROR ACTIVE
ERROR PASSIVE
BUS_OFF
barramento multi-nó
```

Não pretende representar literalmente a camada elétrica CAN.

Não são modelados neste estágio:

```text
CANH / CANL
transceptor físico
níveis diferenciais
temporização elétrica
características analógicas do barramento
```

O barramento é tratado como uma abstração digital de bits dominante/recessivo e frames CAN.

Isso permite manter a separação entre:

```text
ISA
 ↓
máquina
 ↓
I/O
 ↓
protocolo
 ↓
semântica
 ↓
grandeza física
```

---

# Estado do projeto

```text
api.h / micro.h / micro.c
        → FECHADO

can.h / can.c
        → AUDITADO / FECHADO

can_bus.h / can_bus.c
        → AUDITADO / FECHADO

j1939.h / j1939.c
        → FECHADO

can_micro.c
        → INTEGRADOR + TESTBENCH
```

## Resultado consolidado

A suíte termina com:

```text
[OK] can + j1939 + protocol resid 0
```

e a integração produz:

```text
Engine Coolant Temperature = 20 °C
```

tanto pela camada J1939 quanto pela realização através do Microprocessador Fractal.

---

# Arquitetura consolidada

```text
                         ISA
                          │
                          ▼
                       micro.c
                          │
                         I/O
                          │
                          ▼
                        can.c
                          │
                          ▼
                     can_bus.c
                          │
                        frame
                          │
                          ▼
                       j1939.c
                          │
                     PGN / SPN
                          │
                          ▼
                  grandeza física


              ┌──────────────────────┐
              │     can_micro.c      │
              │                      │
              │ composição /         │
              │ integração /         │
              │ testbench            │
              └──────────────────────┘
```

A camada de comunicação permanece **externa à ISA**.

O Microprocessador Fractal não precisa conhecer CAN ou J1939 para executar a transformação computacional.

**CAN fornece o frame.**

**`can.c` fornece o comportamento lógico do protocolo.**

**`can_bus.c` fornece o barramento e a arbitragem entre nós.**

**J1939 fornece a semântica da mensagem.**

**O Microprocessador Fractal realiza a transformação computacional.**

A composição dessas camadas ocorre em `can_micro.c`, preservando a independência estrutural de cada módulo.
