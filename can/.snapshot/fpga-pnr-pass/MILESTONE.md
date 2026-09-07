# MILESTONE — P&R ECP5 PASS: DISCO em EBR (serializado)

**Data:** 2026-09-06
**Alvo:** Lattice ECP5 (LFE5U-25F, CABGA256), toolchain OSS
  yosys 0.66 (synth_ecp5) + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64)

## Resultado

**P&R COMPLETO, bitstream gerado (ecppack exit 0).** O DISCO `word[4096×64]`
passou a mapear em **EBR (DP16KD)**, resolvendo o bloqueio físico de portas
(3R/2W) documentado em `reports/pnr_findings.md`. O núcleo cabe no 25F e é
realizável fisicamente.

## Decisão de microarquitetura autorizada (pelo usuário)

**Serializar acessos ao DISCO** — a única mudança feita, exclusivamente
temporal (microarquitetura), **SEM tocar ISA**:

- `micro.sv` (contrato) INTACTO — regressão V01..V10 10/10 resid 0 (arquivo
  `10_contract_regression.txt`).
- Variante TEMP `micro_disco_ser.sv` (módulo `micro_fractal`, mesma interface)
  com 3 deltas físicos de harness:
  1. **1 porta de leitura + 1 porta de escrita** em `word[]` (1R/1W), com
     `rdata_q` registrado (sync read = obrigação do EBR). Estágios FSM:
     `S_RD_A` (arma ra) → `S_LD_A` (captura a) → `S_RD_B` (arma rb) →
     `S_LD_B` (captura b) → `S_ALU` → `S_WB` (escreve via porta única).
     Ciclos extras: +2/instr binária, +1/unária. `steps_out` conta
     **instruções** (semântica); `pc`/`canal`/`done`/`halted`/`timeout`
     inalterados.
  2. **Reset não zera `word[]`** (EBR não tem reset de conteúdo). Em
     V01..V10 todo word lido é pré-carregado via `d_wr` (maq_clear+word_init),
     equivalência preservada; documentado como responsabilidade do host.
  3. **`d_dout` síncrono** (valida 1 ciclo após `d_rd`/`d_addr`); o TB TEMP
     (`micro_tb_sync.sv`) amostra no posedge — programas e resultados
     V01..V10 idênticos ao oráculo micro.c.

## Evidências (nesta pasta)

| Artefato | Descrição |
|---|---|
| `micro_disco_ser.sv` | Variante serializada (EBR-ready), TEMP |
| `micro_tb_sync.sv` | TB V01..V10 sync-read: 10/10 resid 0 |
| `micro_fpga_top.sv` | Wrapper I/O (31 pinos); núcleo intacto embutido |
| `05_synth_ecp5_top.log` | synth_ecp5: `word via $__DP16KD_` |
| `06_nextpnr.log` | P&R completo |
| `07_nextpnr_console.txt` | + retorno da regressão do contrato |
| `08_micro_ser.txt` | Configuração (7 MB) |
| `09_micro_ser.bit` | Bitstream LFE5U-25F (400 KB) |
| `10_contract_regression.txt/vvp` | micro.sv original 10/10 resid 0 |

## Recursos físicos medidos (LFE5U-25F CABGA256)

```
TRELLIS_IO:   31/197   15%    (wrapper 8-bit: clk,rst,wr,addr[9],din[8],dout[8],done,halted,timeout)
DP16KD (EBR): 24/56    42%    (word 4096×64 = 16 blocos 18K + imem 256×8 + duplicação de portas)
MULT18X18D:   10/28    35%    (mul 64-bit em 10 DSPs)
TRELLIS_FF:  631/24288  2%    (FSM + pipeline; era 262.695 com word em registradores)
TRELLIS_COMB:19750/24288 81%  (LUT4: 12.908 logica + 6.516 carry)
```

**Contraste com o as-is** (medido previamente, `reports/pnr_findings.md`):
262.695 FFs + 1.314.885 muxes (word em registradores) → **1.082% do 25F**.
Depois da serialização: **24 EBRs, 631 FFs, ~19,7k LUTs** → **cabe com folga**.

## Timing (medição honesta — NÃO é um PASS de 50 MHz)

- `nextpnr-ecp5 --freq 50`: **Fmax = 2,38 MHz (FAIL no alvo de 50 MHz)**.
- Caminho crítico: **ALU DIV/MUL 64-bit** em cadeia combinacional profunda
  (slack negativo ~−548 ns); sem pipelining da ALU, sem `$dsp` para DIV.
- O `88 mux` de 64 bits (DIV shift-subtract em LUT) domina o atraso.
- Observação: Fmax de 2,38 MHz é suficiente como prova de realização e mede
  corretamente a física atual; aumentar o Fmax exigiria pipeline da ALU
  (fora do escopo "não considerar otimizações de arquitetura").

## Verificação de equivalência

1. `iverilog -g2012 micro.sv micro_tb.sv` → **10/10 resid 0** (contrato).
2. `iverilog -g2012 micro_disco_ser.sv micro_tb_sync.sv` → **10/10 resid 0**
   (mesmos números V01..V10 do oráculo micro.c: 42, 58, 144, 14, 0xDEADBEEF,
   99, 0, 45, 20, 0).
3. `synth_ecp5` → `mapping memory micro_fractal.word via $__DP16KD_` (EBR),
   e `imem` também; **nenhuma RAM LUT / FF de memória**.

## Não alterado (regras respeitadas)

api.h, opcodes, formato 80-bit (`LOAD→LOAD→ALU→STORE`), semântica, programas
V01..V10, snapshots de contrato, sem CAN/J1939. `micro.sv` e `micro_tb.sv`
do contrato intactos. `.snapshot/rtl-synthesis-pass/` inalterado.