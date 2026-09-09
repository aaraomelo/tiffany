# Análise Final — j1939_read_raw (Fidelidade à ISA ERG-64)

## A) Semântica exata de LOAD, LOADS, STORE, STORE_IND, ADD, ADD16, MUL, DIV

### Word e Slot

- Átomo = Word8 = 1 byte por slot (slot_mem.h).
- Word lógico = par de átomos consecutivos (slot par=total, slot ímpar=e).
- 16 slots = 16 Words8 = 8 Words16.

### LOAD (OP_LOAD, 2 bytes)
- Formato: LOAD slot_u16_LE
- Semântica: r->B = r->A; r->A = mem_le(slot); r->A.e = 0;
- mem_le(slot) lê átomos slot*2 e slot*2+1.
- Largura: Word8 por componente, Word16 lógico.

### LOADS (OP_LOADS, 3 bytes)
- Formato: LOADS ptr_u16_LE
- Semântica: ptr = prog_le(pc)|(prog_le(pc+1)<<8); pc+=2; r->B=r->A; w=mem_le(slot_indice(ptr)); r->A.total=w.total; r->A.e=0;
- slot_indice(ptr) = mem[ptr].total | (mem[ptr].e<<8).
- Endereço efetivo = valor guardado no slot ptr.
- .e = 0 sempre (convenção de leitura).

### STORE (OP_STORE, 2 bytes)
- Formato: STORE slot_u16_LE
- Semântica: mem_grava(slot, r->R)

### STORE_IND (OP_STORE_IND, 3 bytes)
- Formato: STORE_IND ptr_u16_LE
- Semântica: tgt = slot_indice(ptr); slot_mem_grava(fmem, tgt*2, r->R.total);
- Grava só r->R.total (átomo baixo). NÃO grava .e.

### ADD (OP_ADD, 1 byte)
- Semântica: r->R = ula_add(r->A, r->B);
- Soma componente a componente (Word8 + Word8). Sem carry entre total/e.

### ADD16 (OP_ADD16, 1 byte)
- Semântica: W16 a={r->A.total, r->A.e}, b={r->B.total, r->B.e}, x=ula_add16(a,b,NULL); r->R.total=x.baixo; r->R.e=x.alto;
- Soma 16-bit com carry entre baixo e alto.

### MUL16 (OP_MUL16, 1 byte)
- Semântica: W16 a={r->A.total, r->A.e}, b={r->B.total, r->B.e}, x=ula_mul16(a,b); r->R.total=x.baixo; r->R.e=x.alto;
- Multiplicação 16-bit (produto de dois Word16).

### DIV — NÃO EXISTE!
- Não há OP_DIV no enum (isa.h).
- Não há caso DIV em erg.c.
- DIV foi excluída do roadmap CAN/J1939.
- Análise que menciona DIV está incorreta.

## B) Largura real dos operandos

| Instrução | Largura |
|-----------|---------|
| LOAD/LOADS/STORE/STORE_IND | Word8 por componente, Word16 lógico |
| ADD | Word8 componente-a-componente |
| ADD16 | 16-bit com carry |
| MUL16 | 16-bit |
| CMP/CMP16 | 16-bit |
| INC | Word8+1 |
| ESPALHA | Word8→0xFF/0x00 |
| VINCO | Word8 (SUB+flags) |

## C) Representação de valores > Word8

Pares (total, e) em dois slots consecutivos:
- slot par = total (baixo)
- slot ímpar = e (alto)

64 bits = 8 Words8 = 16 slots.
32 bits = 4 Words8 = 8 slots.
16 bits = 2 Words8 = 4 slots.

ISA não tem registradores > 16 bits.

## D) ABI mínima — payload(8) + raw(8) + start + length

16 slots disponíveis.
Payload 8 bytes = 8 slots.
Raw 8 bytes = 8 slots.
Start + length = 2 slots.
Total = 18 slots > 16 disponíveis.

Não cabe em 16 slots sem compressão.

Opções:
1. Reutilizar slots de payload após uso (payload é só leitura).
2. Eliminar start/length da ABI (constantes).
3. Estender máquina para 18+ slots.

## E) Composição little-endian sem SHL/SHR

Sem shift, <<8 = *256. Sem MUL de 64-bit, sem DIV, composição 64-bit é impossível em 16 slots.

## F) Ler payload[start+i] com instruções existentes

LOADS com ponteiros pré-carregados:
- slot 0..7 = ponteiros (valores 8..15)
- slot 8..15 = payload[0..7]
- LOADS i → A = mem[slot_indice(i)] = payload[i]

## G) Contagem de instruções

Loop length iterações × ~6 instruções = ~48 para length=8.

## H) Equivalência semântica

Não é possível realizar o contrato completo dentro de 16 slots com instruções existentes, sem gambiarra.

Gambiarra (NÃO recomendada):
- Comprimir payload/raw para 6 bytes.
- Usar u64 interno no teste C (não é fiel à ISA).

Extensão necessária:
- Aumentar slots de 16 para 18+.
- Ou: reutilização dinâmica de slots de payload.

## I) Decisão necessária

1. Aumentar slots para 18?
2. Reutilizar slots de payload?
3. Aceitar raw parcial (length≤4)?

Aguardar decisão antes de implementar.