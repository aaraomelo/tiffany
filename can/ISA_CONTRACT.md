# Contrato de hardware da ISA Fractal

**Baseline:** ARCHITECTURAL BASELINE v1.0 (`.snapshot/arch-closed/`)  
**Referência comportamental:** `micro.c` / `micro.h` / `api.h`  
**Documento conceitual:** `microprocessador.tex`

Este contrato define a máquina que qualquer realização (C, RTL, FPGA) deve
implementar **identicamente**. Objetivo de equivalência:

```text
micro.c(programa, estado_inicial)  ==  micro.sv(programa, estado_inicial)
```

RTL **não** começa antes deste contrato estar estável.

---

## 1. Formato da instrução de 80 bits

Uma instrução ocupa **10 bytes** (80 bits), little-endian por campo:

```text
 byte:  0  1  2 | 3  4  5 | 6 | 7  8  9
        --------+---------+---+--------
 campo:   addr_a    addr_b   op  addr_r
 bits:     24        24       8    24
```

| Campo   | Largura | Encoding                          |
|---------|---------|-----------------------------------|
| `addr_a`| 24      | bytes 0..2, LE                    |
| `addr_b`| 24      | bytes 3..5, LE                    |
| `op`    | 8       | byte 6                            |
| `addr_r`| 24      | bytes 7..9, LE                    |

Máscara de endereço lógico: `addr & 0xFFFFFF`.  
Mapeamento físico (modelo de referência): `addr % MICRO_MEM_WORDS` com `MICRO_MEM_WORDS = 4096`.

API de referência:

```c
void encode80(const Instr80 *I, uint8_t out[10]);
void decode80(const uint8_t in[10], Instr80 *I);
```

---

## 2. Opcodes e semântica

| Código | Nome  | Operação                         | Operandos        |
|--------|-------|----------------------------------|------------------|
| 0      | HALT  | encerra `roda80`                 | —                |
| 1      | ADD   | `r = ADD(a,b)`                   | a, b → r         |
| 2      | SUB   | `r = SUB(a,b)`                   | a, b → r         |
| 3      | MUL   | `r = MUL(a,b)`                   | a, b → r         |
| 4      | DIV   | `r = DIVQ(a,b)` (quociente)      | a, b → r         |
| 5      | AND   | `r = AND(a,b)`                   | a, b → r         |
| 6      | OR    | `r = OR(a,b)`                    | a, b → r         |
| 7      | XOR   | `r = XOR(a,b)`                   | a, b → r         |
| 8      | NOT   | `r = NOT(a)`                     | a → r (b ignore) |
| 9      | MOV   | `r = a`                          | a → r (b ignore) |
| 10     | JMP   | `PC ← addr_a`                    | só `addr_a`      |

- Valores `op > 10` (exceto uso de teste): no modelo atual produzem `r = 0` e STORE — **contrato RTL deve tratar como NOP ou trap; a definir antes do tape-out simulado**.
- `ADD`/`SUB`/`MUL`/`DIV`/`AND`/`OR`/`XOR` operam em **palavras u64** (`api.h`).
- `DIV(a,0)`: quociente 0, resto `a` (contrato de `DIV` em `micro.c`).
- Largura de dados da ALU: **64 bits**. Endereços de memória de instrução: **24 bits**.

### Primitiva MOVE

Toda transferência de estado reduz-se a:

```text
MOVE(destino, +1)  = LOAD   (mem[addr] → valor)
MOVE(destino, -1)  = STORE  (canal → mem[addr])
MOVE(destino,  0)  = JMP    (PC ← destino)
```

Ciclo de instrução (não-JMP, não-HALT):

```text
a ← LOAD(addr_a)
b ← LOAD(addr_b)     // se op ∉ {NOT, MOV}
r ← ALU(op, a, b)
STORE(addr_r, r)
PC ← PC + 10
```

JMP:

```text
PC ← addr_a
```

HALT: para o fetch loop.

---

## 3. Modelo de estado da máquina

```c
typedef struct {
    u64  word[4096];   /* DISCO / memória de dados */
    u64  canal;        /* registrador de caminho STORE */
    u64  pc;           /* contador de programa (byte offset no programa) */
} Maquina;
```

| Campo   | Papel |
|---------|--------|
| `word[]`| memória de dados (disco); 4096 palavras × 64 bit |
| `canal` | valor em trânsito para STORE (modelo C) |
| `pc`    | offset em **bytes** no stream de programa |

Estado inicial após `maq_clear`: todos zeros.

**Programa** não precisa residir em `word[]` no modelo de referência atual: `roda80` recebe um buffer externo `prog[]`.  
Contrato para hardware futuro: o programa **pode** residir no DISCO (região mapeada) ou em memória de instrução separada — desde que o **efeito observável** de FETCH/DECODE/EXECUTE coincida com `roda80`.

Observáveis para equivalência C ↔ RTL:

1. conteúdo de `word[0..4095]` após N passos;
2. valor final de `pc`;
3. ocorrência de HALT.

---

## 4. DISCO / LOAD / STORE

### Endereçamento

```text
addr_logico  ∈ [0, 2^24)
addr_fisico  = addr_logico % 4096     // modelo de referência atual
```

### LOAD

```text
LOAD(addr) = word[addr_fisico(addr)]
```

### STORE_VAL

```text
word[addr_fisico(addr)] ← val
```

### STORE (via canal)

No modelo C, `STORE(M, addr)` grava `M->canal` em `word[addr]`.  
`STORE_VAL` é a forma usada por `exec80`.

### Disco como ofício (texto)

Em `microprocessador.tex`, memória = disco = contração bit a bit / arquivo-grafo.  
No contrato digital mínimo para RTL:

- uma porta de leitura e uma de escrita por ciclo de instrução (ou dual-port equivalente);
- latência 1 ciclo combinacional no modelo de referência (sem pipeline obrigatório na v1).

**Não** faz parte do contrato v1.0: hierarquia L1/L2, DMA, memória virtual.

---

## 5. Modelo de I/O

### No modelo de referência atual (`micro.c`)

I/O **não** é um opcode dedicado. A fronteira com o mundo exterior é:

1. **memória mapeada** — regiões de `word[]` tratadas pelo software como registradores de periférico;
2. **programa no DISCO** — o “driver” CAN/J1939 é código ISA, não um bloco na ALU;
3. composição externa (`can_micro.c`) — só para testes; **não** existe no silício.

### Contrato para hardware

```text
CPU Fractal
  │
  ├── word[]  (DISCO / RAM)
  ├── PC / FETCH
  ├── ALU 64-bit
  └── porta I/O física (ex.: FIFO/registradores mapeados no espaço de endereço)
           │
           ▼
        CAN PHY / outros
```

- A ISA **não** contém opcodes `CAN_TX` / `J1939_*`.
- Protocolos são **programas** que fazem LOAD/STORE nos registradores mapeados e computam com a ALU.
- Equivalência: o mesmo binário de programa J1939 (stream de instruções 80-bit) deve produzir o mesmo estado de memória/I/O em `micro.c` e em `micro.sv`.

---

## 6. Ciclo de execução (`roda80`)

```text
pc ← 0
enquanto não HALT e pc+10 ≤ tamanho_programa:
    I ← decode80(prog[pc .. pc+9])
    se I.op = HALT: fim
    se I.op = JMP:  pc ← I.addr_a
    senão:
        executar LOAD/ALU/STORE conforme §2
        pc ← pc + 10
```

---

## 7. O que a ISA **não** conhece

```text
CAN frame layout
J1939 PGN/SPN
J1939-TP BAM/RTS
catálogo de sinais
escala / offset / NA
```

Isso permanece software (ou dados no DISCO), alinhado à tese:

> **O protocolo é externo à ISA.**  
> A implementação do protocolo também é programa.

---

## 8. Caminho até o RTL (ordem)

```text
1. Este contrato estável          ← você está aqui
2. Suite de vetores (programa, estado_in → estado_out)
3. micro.sv implementando §1–§6
4. cocotb / formal: micro.c == micro.sv nos vetores
5. FPGA: só a máquina (+ I/O físico)
6. Programas CAN/J1939 como imagens no DISCO
```

**Não** gerar `j1939.sv` como motor dedicado.

---

## 9. Resumo em uma caixa

```text
Instrução 80-bit = (addr_a:24, addr_b:24, op:8, addr_r:24)
Estado           = (word[4096], canal, pc)
Ciclo            = FETCH → DECODE → MOVE/ALU → PC+10|JMP
I/O              = memória mapeada + programas no DISCO
Protocolo        = software externo à ISA
```
