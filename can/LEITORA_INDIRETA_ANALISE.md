# Relatório de Análise — Leitura Indireta ERG-64

## A) LOAD Indireto: como funciona

### LOADS (opcode 14, OP_LOADS) — formato de 3 bytes
```
byte 0: OP_LOADS (14)
byte 1..2: ptr_u16 LE (endereço do slot que contém o índice)
```

Semântica (erg.c:342-349):
```c
case OP_LOADS: {
    unsigned ptr = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
    pc += 2;
    r->B = r->A;
    Word w = mem_le(slot_indice(ptr));
    r->A.total = w.total;
    r->A.e = 0;
    break;
}
```

Onde slot_indice(ptr):
```c
static unsigned slot_indice(unsigned ptr){
    Word idx = mem_le(ptr);           // lê slot 'ptr' como Word
    return (unsigned)idx.total | ((unsigned)idx.e << 8);
}
```

LOADS lê mem[slot_indice(ptr)]. O endereço efetivo é o valor
guardado no slot ptr (seu .total | .e<<8).

### STORE_IND (opcode 23, OP_STORE_IND) — formato de 3 bytes
```
byte 0: OP_STORE_IND (23)
byte 1..2: ptr_u16 LE
```
Semântica (erg.c:351-358): grava r->R.total em mem[slot_indice(ptr)].

### Conclusão A: LOADS/STORE_IND são indireto via slot

| Modo | Opcode | Endereço efetivo |
|------|--------|------------------|
| LOAD direto | OP_LOAD (1) | slot = imediato do programa |
| LOADS indireto | OP_LOADS (14) | slot = slot[ptr].total | (slot[ptr].e << 8) |
| STORE direto | OP_STORE (2) | slot = imediato do programa |
| STORE_IND indireto | OP_STORE_IND (23) | slot = slot[ptr].total | (slot[ptr].e << 8) |

Não existe LOAD_IND que use A/B/R como endereço.
Mas LOADS já faz indireto: se slot K contém o endereço desejado,
LOADS K lê mem[slot_indice(K)].

### Solução para payload_base + i

Pré-carregar endereços em slots de ponteiros:
```
slot 16 = 0    ; ponteiro para payload[0]
slot 17 = 1    ; ponteiro para payload[1]
...
slot 23 = 7    ; ponteiro para payload[7]
```

Para ler payload[i]:
```
LOADS 16+i   → A = mem[slot_indice(16+i)] = mem[i] = payload[i]
```

Isso funciona como leitura indireta via registo (slot).

## B) Representação 64-bit raw

Word = { Word8 total, Word8 e } = 16 bits = 2 slots.
64 bits = 4 Words = 8 slots.

Slots necessários para raw de 8 bytes:
  raw[0..7] = slots 16..23

Total slots da máquina: 16 (slots 0..15).
CONFLITO: precisamos de slots 16..23 para raw.

Solução: usar slots 0..7 para payload (8 bytes CAN fixo).
slots 8..15 para resultado/temporários.

### Compromisso necessário

16 slots não cabem payload(8) + raw(8) + controle.
Opções:
  A) raw parcial em slots disponíveis (length≤2 ou length≤4)
  B) reutilizar slots de payload conforme necessário
  C) Word8 componente-a-componente + cadeia de carries

## C) SHL/SHR sem opcode — verificação MUL/DIV u64

erg.c exec80 usa ADD/SUB/MUL/DIV em u64 (Word64).
MUL e DIV operam em u64.

SHL x, 8 = MUL x, 256
SHR x, 8 = DIV x, 256

## D) Carry multi-word

ADD16 faz ADD de 16 bits (2 Word8) com carry entre componentes.
Para 64 bits: cadeia de ADD16 + propagate carry manual.

## E) ABI final proposta (revista)

Usando u64 por slot (máquina trabalha em u64):
  slot 0 = payload[0] (u64, byte 0..7 little-endian)
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

Ou Word8 por slot (fiel à ISA Word8):
  slot 0..7 = payload[0..7]
  slot 8 = start_byte
  slot 9 = length
  slot 10..13 = raw[0..3] (32 bits — length≤4)
  slot 14 = i
  slot 15 = err_flag

## F) Quantidade estimada de instruções

Para length 1..8 com MUL/DIV u64:
  Loop de length iterações × ~6 instruções = ~48 instruções

## G) Decisão necessária

1. Usar MUL/DIV u64 para SHL/SHR?
   - Sim se OP_DIV funcionar com divisor constante 256^i.

2. Ou usar LOADS com slots de ponteiros (fiel à ISA)?
   - Precisa de slots 16+ para ponteiros. Máquina só tem 16 slots.

3. Comprimir payload em menos slots?
   - CAN payload é sempre 8 bytes, mas podemos usar u64 por slot.

4. Usar Word8 componente-a-componente + cadeia de carries?
   - Mais instruções, mas fiel ao Word8 da ISA.

PRÓXIMO PASSO: verificar se OP_DIV e OP_MUL aceitam
divisores/multiplicadores constantes e como fazem shift.
