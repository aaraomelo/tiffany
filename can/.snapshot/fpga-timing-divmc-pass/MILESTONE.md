# MILESTONE — FPGA TIMING: DIV MULTI-CYCLE PASS @ 50 MHz

**Data:** 2026-09-06
**Alvo:** Lattice ECP5 (LFE5U-25F, CABGA256), toolchain OSS
  yosys 0.66 (synth_ecp5) + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64)

## Resultado

**P&R exit 0 — timing fechado: Fmax final = 52,49 MHz, PASS @ 50 MHz.**
O gargalo combinacional do `IOP_DIV` foi eliminado; o DIV agora é
**multi-cycle** (datapath de 65 bits, ~1 ciclo por bit, long division
idêntica à do `DIV`/exec80 de micro.c). Nenhuma alteração de ISA.

Obs: uma estimativa intermediária de 45,42 MHz aparece no log do
`nextpnr-ecp5 --freq 200` **não é o resultado final do P&R a 50 MHz**;
o resultado registrado é **52,49 MHz (PASS @ 50 MHz)** na rodada de
fechamento (exit 0).

## Decisão de microarquitetura autorizada (pelo usuário)

**DIV multi-cycle via long division** — atacar exclusivamente a operação
dominante do critical path, medido isoladamente (`alu_crit.sv`):

| OP | Fmax isolado |
|---|---|
| DIV | **2,35 MHz** (domina o núcleo inteiro) |
| MUL | 61,04 MHz |
| ADD | 209 MHz |
| SUB | 211 MHz |
| AND/OR/XOR/NOT/MOV | 564–974 MHz |

Implementação: variante TEMP `micro_alu_divmc.sv` (módulo `micro_fractal`,
mesma interface). O `S_ALU` só desvia quando `rop == IOP_DIV`:

- Datapath: `div_a`/`div_b` (latch 64-bit), `div_r` (65 bits, inv `r < 2b`),
  `div_q` (64 bits), `div_cnt[5:0]`.
- Iteração (`S_DIV_IT`, 1 bit/ciclo, 63→0): `r = {r[63:0], a[cnt]}`;
  se `r >= b`: `r -= b`, `q[cnt]=1` — equivale ao loop `for i=63..0` de
  micro.c. Estado final `S_DIV_END`: `canal <= div_q`.
- `DIV(a,0) = 0` imediato (sem loop; semântica DIVQ do contrato).
- **O divisor combinacional `a/b` não existe mais no netlist** (dead,
  otimizado pelo yosys); `mwdata` no `S_WB` usa `div_q` quando `rop==DIV`.
- 64 ciclos extras por DIV; `steps_out` continua contando **instruções**
  (inalterado para V01..V10).

## Evidências (nesta pasta)

| Artefato | Descrição |
|---|---|
| `micro_alu_divmc.sv` | Variante TEMP com DIV multi-cycle (65-bit) |
| `micro_tb_sync.sv` | TB V01..V10 sync-read: 10/10 resid 0 |
| `micro_fpga_top.sv` | Wrapper I/O (31 pinos); núcleo intacto embutido |
| `05_synth_ecp5_top.log` | synth_ecp5: `word`/`imem via $__DP16KD_` (EBR) |
| `06_nextpnr.log` | P&R fechamento @ 50 MHz: exit 0, Fmax 52,49 PASS |
| `07_nextpnr_freq200.txt` | Varredura @ 200 MHz (referência; 45,42 é intermediário) |
| `08_micro_divmc.txt` | Configuração LFE5U-25F (1,5 MB) |
| `09_micro_divmc.bit` | Bitstream (ecppack exit 0, 198.600 bytes) |
| `10_contract_regression.txt/vvp` | micro.sv original 10/10 resid 0 |

## Recursos físicos medidos (LFE5U-25F CABGA256, P&R fechamento @ 50 MHz)

```
TRELLIS_IO:   31/197    15%
DP16KD (EBR): 24/56     42%    (word 4096×64 + imem + portas — EBR, não LUT/FF)
MULT18X18D:   10/28     35%    (mul 64-bit preservado em DSPs)
TRELLIS_FF:  895/24288   3%    (FSM + datapath DIV multi-cycle)
TRELLIS_COMB: 2266/24288  9%   (carry-heavy do long-division)
```

## Timing (resultado final do P&R)

- `nextpnr-ecp5 --json micro_divmc_top.json --25k --package CABGA256
  --freq 50 --textcfg` → **exit 0**.
- **Fmax = 52,49 MHz — PASS @ 50 MHz** (era 2,38 MHz com DIV combinacional).
- Novo critical path: tipicamente rede lógica curta (estado FSM + cnt/div_q),
  longe do teto; MUL (~61 MHz isolado) alinhado ao teto do núcleo.

## Verificação de equivalência

1. `iverilog -g2012 micro.sv micro_tb.sv` → **10/10 resid 0** (contrato).
2. `iverilog -g2012 micro_alu_divmc.sv micro_tb_sync.sv` → **10/10 resid 0**
   (resultados V01..V10 = 42, 58, 144, 14, 0xDEADBEEF, 99, 0, 45, 20, 0;
   steps 2/2/2/2/2/3/1/3/3/3).
3. `synth_ecp5` → `word via $__DP16KD_` e `imem via $__DP16KD_` (EBR).

## Próximo critical path (NÃO otimizado nesta etapa)

MUL: **~61 MHz** medido isolado (datapath 64-bit em 10 MULT18X18D).
Ainda não é um bloqueio para 50 MHz — a otimização do MUL fica para etapa
posterior, se o alvo de clock subir. **Não otimizado aqui, conforme pedido.**

## Não alterado (regras respeitadas)

api.h, opcodes, formato 80-bit (`LOAD→LOAD→ALU→STORE`), semântica, programas
V01..V10, snapshots anteriores (`rtl-synthesis-pass`, `fpga-pnr-pass`)
preservados. `micro.sv` e `micro_tb.sv` do contrato intactos. Sem CAN/J1939.