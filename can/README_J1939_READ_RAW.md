# j1939_read_raw — Análise Pré-Implementação

## A) LOAD Indireto na ERG-64

### Existem dois modos de acesso indireto:

**LOADS ptr** (3 bytes: opcode + ptr_u16 LE):
- ptr é imediato no programa
- slot_indice(ptr) = mem[ptr].total | (mem[ptr].e << 8)
- A = mem[slot_indice(ptr)]
- B = A anterior
- R = A

**STORE_IND ptr** (3 bytes):
- grava R em mem[slot_indice(ptr)]

### PROBLEMA: Nenhum dos dois usa A como ponteiro

LOADS ptr: indireto via slot cujo ENDEREÇO É o imediato ptr.
Se slot 16 contém o valor 3, LOADS 16 lê mem[3].
Mas o endereço 3 é fixo no programa, não em registo.

Não existe LOAD_IND que faça: A = mem[A] (endereço em registo).

### SOLUÇÃO: slots de ponteiros com LOADS

Se slot K contém um endereço A (como Word: A.total=A, A.e=0),
então LOADS K → mem[slot_indice(K)] = mem[A].

Para payload[i]:
  slot P+i = i  (ponteiro fixo para slot i)
  LOADS P+i   → A = mem[i] = payload[i]

Isto funciona! Endereços são pré-carregados em slots de ponteiros.

## B) Representação 64-bit raw

Word = { Word8 total, Word8 e } = 16 bits = 2 slots.
64 bits = 4 Words = 8 slots.

Slots necessários para raw de 8 bytes:
  raw[0..7] = slots 16..23

Total slots da máquina: 16 (slots 0..15).
CONFLITO: precisamos de slots 16..23 para raw.

Solução: usar slots 0..7 para payload (8 bytes CAN fixo).
slots 8..15 para resultado/temporários.

ABI revisada:
  slot 0 = payload[0]
  slot 1 = payload[1]
  ...
  slot 7 = payload[7]
  slot 8 = start_byte
  slot 9 = length
  slot 10 = raw[0] (LSB)
  slot 11 = raw[1]
  slot 12 = raw[2]
  slot 13 = raw[3]
  slot 14 = raw[4]
  slot 15 = raw[5] (faltam raw[6], raw[7] — sem slots!)

PROBLEMA: 16 slots não cabem payload(8) + raw(8) + controle.

### Compromisso: raw parcial em slots disponíveis

Opção 1: raw em 4 slots (16 bits, length≤2) — PROTÓTIPO
Opção 2: reutilizar slots de payload conforme necessário
Opção 3: slots 0..7 = payload, slots 8..11 = raw_lo (4 bytes = 32 bits)
         length≤4 cobre 75% dos casos J1939 (SPNs usam 1..4 bytes)

## C) SHL/SHR sem opcode

SHL x, 8 = x * 256:
  MUL por constante 256 em slot separado
  Mas MUL é Word8 componente-a-componente → só multiplica bytes

SHR x, 8 = x / 256:
  Sem DIV de byte. DIV opera em Word64 (u64).
  MAS DIVQ está em micro.h como função C, não é opcode ERG-64.
  ERG-64 tem OP_DIV (opcode 4) que opera em u64.

CONFIRMAÇÃO: erg.c tem OP_DIV que faz DIVQ(a,b) em u64.
Divisão de Word64 por constante = SHR se potência de 2.

## D) Carry multi-word

ADD16 faz ADD de 16 bits (2 Word8) com carry entre componentes.
Para 64 bits: cadeia de ADD16 + propagate carry manual.

OP_DIV opera em u64 (64 bits). Divisão u64 / u64 existe.
Multiplicação MUL opera em u64 (64 bits).

CONFIRMAÇÃO: erg.c exec80 usa ADD/SUB/MUL/DIV em u64 (Word64).
A máquina manipula u64 internamente!

Então: SHL x, 8 = MUL x, 256 (u64)
       SHR x, 8 = DIV x, 256 (u64)

MAS: MUL e DIV operam em u64, não em Word8.
Na ABI do teste vinco_test.c, Word = 2x Word8.
A operação MUL de u64 multiplica os 8 bytes como u64.

## E) ABI final proposta (revista)

Usando u64 por slot (máquina trabalha em u64):
  slot 0 = payload[0] (u64, byte 0..7 little-endian)
  slot 1 = payload[1]
  ...
  slot 7 = payload[7]
  slot 8 = start_byte
  slot 9 = length
  slot 10 = raw (u64 resultado)
  slot 11 = i (contador)
  slot 12 = temp
  slot 13 = err_flag
  slot 14 = mul_const (256^i)
  slot 15 = reserva

Ou simplificado (payload em slots 0..7, cada slot = 1 byte):
  raw é acumulado em slot 10 via:
    raw = raw + payload[i] * (256^i)
  usando MUL/DIV u64 para shift.

## F) Quantidade estimada de instruções

Para length 1..8 com MUL/DIV u64:
  Loop de length iterações × ~6 instruções = ~48 instruções

## G) Decisão necessária

1. Usar MUL/DIV u64 para SHL/SHR (divisão por 256^i)?
   - Sim se OP_DIV funcionar em u64 e aceite divisor 256^i.
   - Verificar se erg_new.exe suporta MUL/DIV u64.

2. Ou usar LOADS com slots de ponteiros (mais fiel à ISA)?
   - Precisa de slots 16+ para ponteiros. Máquina só tem 16 slots.

3. Comprimir payload em menos slots?
   - CAN payload é sempre 8 bytes, mas podemos usar u64 por slot.

4. Usar Word8 componente-a-componente + cadeia de carries?
   - Mais instruções, mas fiel ao Word8 da ISA.

PRÓXIMO PASSO: verificar se OP_DIV e OP_MUL aceitam
divisores/multiplicadores constantes e como fazem shift.
