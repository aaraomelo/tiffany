# Resposta ao utilizador — Análise DISCO → micro

## Resposta objetivo

> O micro consegue executar um programa armazenado no DISCO e ler os dados do DISCO diretamente?

**SIM.**

O programa fica em `/tmp/erg_prog.bin` (DISCO). O micro lê bytecode com `pread(fprog)`. Os dados ficam num ficheiro de dados (DISCO). O micro lê dados com `pread(fmem)` via LOADS/STORE_IND. NÃO há cópia para slots locais.

> Podemos organizar payload[8] e raw[8] no DISCO sem competir pelos 16 slots locais?

**SIM.** LOADS pode ler qualquer slot u16 (0..65535) do ficheiro de dados. STORE_IND pode escrever qualquer slot u16. Podemos colocar payload em slots 0..7 e raw em slots 8..15 do ficheiro de dados, e o micro acede via LOADS/STORE_IND com endereços u16.

## Mapa físico/lógico

```
DISCO:
  programa ERG-64 (erg_prog.bin) — bytecode
  dados (sigma_mem.dat / ficheiro de dados):
    slot 0..7  = payload[0..7]
    slot 8..15 = raw[0..7]
    slot 16    = start_byte
    slot 17    = length
    slot 18    = i
    slot 19..  = temporários

Micro (registos internos):
  A, B, R, pc, flags
```

## ABI proposta para j1939_read_raw()

Usando a arena DISCO:

```
slot 0..7  = payload[0..7]  (Word8 cada, no DISCO)
slot 8..15 = raw[0..7]      (Word8 cada, no DISCO)
slot 16    = start_byte      (Word8)
slot 17    = length          (Word8)
slot 18    = i               (Word8)
slot 19    = err_flag        (Word8)
slot 20    = mul_const       (256^i, Word8)
slot 21    = temp            (Word8)
```

Mas espera — os slots 0..7 já estão no DISCO como payload.
O micro usa slots 0..15 como endereços base para LOADS/STORE_IND.
Os dados estão no DISCO, não nos slots locais.

### Clarificação

A máquina ERG-64 tem 16 slots. Cada slot é um endereço no ficheiro de dados (DISCO).
O micro lê slots 0..15 do ficheiro de dados por defeito.
LOADS/STORE_IND podem aceder a qualquer slot da arena DISCO (0..65535).

### ABI correta

```
Ficheiro de dados (DISCO):
  slot 0..7  = payload[0..7]
  slot 8..15 = raw[0..7]
  slot 16    = start_byte
  slot 17    = length
  slot 18    = i
  slot 19..  = temporários

Micro (registos internos):
  A, B, R, pc, flags
```

O programa ERG-64 fica no DISCO (/tmp/erg_prog.bin).
Os dados ficam no DISCO (ficheiro de dados).
O micro lê do DISCO a cada instrução.

## Conclusão

O micro NÃO copia tudo para 16 slots locais.
O micro lê do DISCO via LOADS/STORE_IND.
Podemos usar toda a arena DISCO para payload e raw.
Os 16 slots da máquina são apenas o endereço base.
LOADS/STORE_IND podem aceder a qualquer slot da arena DISCO (0..65535).

## Decisão pendente

A ABI de j1939_read_raw pode usar toda a arena DISCO para payload e raw.
Os 16 slots da máquina são apenas o endereço base.
Precisamos confirmar o limite máximo do ficheiro de dados (65536 slots?).
