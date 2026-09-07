# P&R FPGA — tentativa as-is (ECP5 open-source) — análise física

**Data:** 2026-09-06
**Alvo candidato:** Lattice ECP5 (LFE5U-12F/25F/45F/85F), toolchain OSS
  yosys 0.66 + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64)
**Procedência:** `micro.sv` intacto; regressão V01..V10 10/10 preservada (resid 0).

## Resultado: P&R NÃO passa no RTL exatamente como está — dois bloqueios físicos

### ATUALIZAÇÃO — Medição autoritativa da variante sync-read (harness de medição)

Em 2026-09-06 foi criado o harness em TEMP (`micro_sync.sv` = `micro.sv` com
a ÚNICA mudança `d_dout` síncrono, + `micro_fpga_top.sv` = wrapper de I/O de
31 pinos) e executado `synth_ecp5`. Censo AIGMAP (autoral, antes do abc9):

```
TRELLIS_FF: 262.695   (word[4096×64] = 262.144 FFs + regs do FSM/wrapper)
  $_MUX_:   1.314.885  (demux das 3 portas de leitura do word)
  DIIFF/FF: 24.288 FF no LFE5U-25F  ⇒ 1.082% do orçamento  → inviável
  DP16KD:         8    (imem mapeado em EBR)
  MULT18X18D: 10    (mul 64-bit mapeado em 10 DSPs)
  CCU2C:      5.294   (cadeia de carry)
  AND 22.538 / NOT 2.384 / OR 9.646 / XOR 2.390
```

**Causa:** mesmo com leitura síncrona, `word[]` tem 3 portas de leitura
(ra, rb, d_dout) + 2 de escrita (FSM, d_wr) = 3R/2W. O DP16KD (EBR) suporta
no máximo 2R/2W; o mapa de memória do yosys não infere EBR para 3R ⇒
converte todo o `word[]` em arquivo de registradores (262.144 FFs) + muxes
de leitura (1,3M `$_MUX_`). O abc9 não fecha (malha gigantesca). **Resultado:
a estrutura multiporta do DISCO é o gargalo real; read-síncrono sozinho não
resolve.** Para EBR seria preciso reduzir portas (ex.: juntar ra/rb num único
port de leitura com mux de endereço no FSM, e arbitrar a escrita d_wr) —
mudança de porta do DISCO, decisão de arquitetura, fora do harness.

### Bloqueio 1 — DISCO com porta de LEITURA ASSÍNCRONA é inviável em FPGA

`word[]` (4096×64, 262.144 bits) tem 3 portas de leitura combinacional:
- `d_dout` (`always_comb`, porta externa d_rd/d_dout) — READ assíncrono
- `a_tmp <= word[ra]` (LOAD a) e `b_tmp <= word[rb]` (LOAD b)

EBR/BRAM do ECP5 tem leitura **síncrona** (estado registrado no relógio).
Teste empírico (prjtrellis): memória 16×8 com write síncrono + read assíncrono
foi mapeada pelo nextpnr-ecp5 em **RAM LUTs** (lógica distribuída), NÃO em EBR:
```
LUT4s:     20/24288   0%   (logic 8, RAM 8, RAMW 4)
```
Para `word[]` as-is, o read assíncrono expande para muxes distribuídos:
~(4096 × 64 bits × 3 portas) de RAM LUT ⇒ ordem de **~500–800 mil LUT4s**.
Capacidade: ECP5-25F 24.288 LUTs; ECP5-85F 83.584 LUTs ⇒ **≫ que qualquer ECP5**.
Consequência: o pass `abc9` do `synth_ecp5` fica sem terminar (timeout) ao
mapear essa malha; place-and-route do as-is é inviável em tempo prático.

Nota: a ALU usa `$mul`/`$div` 64-bit (aceitáveis em LUTs; ECP5 não tem DSP
para mul 64×64). Não são o gargalo — a memória assíncrona é.

### Bloqueio 2 — Interface exige 373 pinos de I/O; maior ECP5 tem 365

Interface do `micro.sv` (top-level, 19 portas = 373 bits de E/S):
- Entradas (186): clk, rst, start, prog_bytes[24], canal_init[64],
  p_wr, p_addr[8], p_data[8], d_addr[12], d_din[64], d_wr, d_rd
- Saídas (187): done, halted, timeout, pc_out[24], steps_out[32],
  canal_out[64], d_dout[64]

Capacidade de user-IO dos ECP5 (medida via nextpnr-ecp5 0.7):
```
LFE5U-12F (CABGA256):  197
LFE5U-25F (CABGA256):  197
LFE5U-45F (CABGA256):  245
LFE5U-85F (CABGA381):  365   <- maior da família
```
**373 > 365** ⇒ mesmo no maior ECP5 não cabe o top as-is. Bloqueio
independente da memória.

## Interpretação (arquitetura, não otimização)

Os dois bloqueios não são questões de "quão bem o Yosys otimiza"; são
mismatches físicos do CONTRATO atual com FPGA:
1. DISCO precisa de **leitura síncrona** para mapear BRAM (semântica ISA
   preservada; apenas o instante de validade da porta de teste muda).
2. Os observáveis contínuos (pc_out/steps_out/canal_out/d_dout + portas de
   carga/teste) precisam de **mapeamento/paginação de I/O** (ex.: registros
   de status multiplexados num barramento) — fora do escopo atual.

Não foi alterado nenhum arquivo do contrato (micro.sv/micro_tb.sv intactos).

## RESOLUÇÃO (2026-09-06) — Disco serializado, EBR, P&R PASS

O usuário autorizou a serialização dos acessos ao DISCO. Variante TEMP
`micro_disco_ser.sv` (1R/1W, `rdata_q` síncrono, reset não zera word[]):
10/10 resid 0 no simulador; `synth_ecp5` mapeia `word` e `imem` em DP16KD
(EBR); P&R+bitstream no LFE5U-25F completos (24 EBR, 631 FF, ~19,7k LUT,
10 DSP). Fmax = 2,38 MHz (gargalo: ALU DIV/MUL 64-bit combinacional).
Detalhes completos + artefatos em `.snapshot/fpga-pnr-pass/` (MILESTONE.md).

## RESOLUÇÃO 2 (2026-09-06) — Timing: DIV multi-cycle → PASS @ 50 MHz

**DIV combinacional → gargalo → multi-cycle → Fmax 52,49 MHz → PASS @ 50 MHz.**

O bloqueio de timing anterior (Fmax 2,38 MHz, causado pelo caminho com
DIV 64-bit combinacional) foi eliminado. Medindo o critical path isolado
por operação (`alu_crit.sv`), o DIV era o dominante (2,35 MHz isolado).

Variante TEMP `micro_alu_divmc.sv`: o DIV agora é **multi-cycle** — long
division de 65 bits, ~1 ciclo/bit (64 ciclos), FSM `S_ALU → S_DIV_IT →
S_DIV_END → S_WB`; `DIV(a,0)=0` imediato; `mwdata` do S_WB usa `div_q`
quando `rop==DIV` (a unidade combinacional `a/b` saiu do netlist).

Resultado do P&R de fechamento (`--freq 50 --textcfg`): **exit 0**,
Fmax **52,49 MHz = PASS @ 50 MHz**; 895 FF, 2266 LUTs, 24/56 DP16KD,
10/28 MULT18X18D, 31/197 IO. Bitstream `ecppack` exit 0 (198.600 bytes).
Regressão V01..V10 do contrato e da variante: 10/10 resid 0.
Artefatos completos em `.snapshot/fpga-timing-divmc-pass/` (MILESTONE.md).

**Próximo critical path medido: MUL ≈ 61 MHz (isolado)** — ainda não é um
bloqueio para o alvo atual de 50 MHz; otimização do MUL fica para uma etapa
posterior (não executada nesta).

Ferramentas instaladas e prontas para (a): yosys 0.66 + nextpnr-ecp5 0.7
+ prjtrellis 1.4 (pacotes MSYS2/mingw64).