# MILESTONE — rtl-synthesis-pass

**Data:** 2026-09-06
**Alvo:** `micro.sv` (Microprocessador Fractal, RTL literal de `micro.c`)
**Ferramenta:** Yosys 0.66 (x86_64-w64-mingw32, MSYS2/mingw64)
**Compiladores de regressão:** Icarus v13 (msys2) e v12 (Windows)

## Critério de aprovação

| Item | Status |
|---|---|
| `read_verilog -sv micro.sv` (sem erros) | PASS |
| `hierarchy -check -top micro_fractal` (sem erros, sem hierarquia órfã) | PASS |
| `proc` / `opt` / `memory -nomap` (elaboração sintetizável) | PASS |
| `techmap` + `abc -g ...` (mapeamento lógico completo) | PASS |
| `write_verilog` do netlist gate-level (pós-síntese) | PASS |
| Regressão V01..V10 continua 10/10 (resid 0) | PASS |
| Yosys exit code | 0 |

## Recursos inferidos (gate-level, memórias preservadas)

Fonte: `reports/03_synth_gate_stats.txt` (após `techmap` + `abc -g AND,OR,XOR,MUX,NAND,NOR` + `opt`)

| Recurso | Quantidade |
|---|---|
| `$mem_v2` (memórias arquiteturais, BRAM/SSRAM no FPGA) | 2 |
| `word` (DISCO) | 4096 x 64 bits = 262.144 bits |
| `imem` (program stream) | 256 x 8 bits = 2.048 bits |
| FFs `$_SDFFE_PP0P_` (registradores seqüenciais) | 240 |
| Portões lógicos (AND/NAND/OR/NOR/XOR/NOT/MUX) | 30.596 |
| Células totais (excl. memórias) | 30.838 |

Nota: `abc -lut 6` não produziu transformação nesta build (passou sem efeito visível);
o mapping de referência é o gate-level acima. FFs = estado/pc/steps/canal/datapath
após otimização de registros transitórios do pipeline de fetch.

## Warnings / Errors

- **Errors:** nenhum.
- **Warning (1, benigno):** `ABC: Warning: The network is combinational (run "fraig" or "fraig_sweep").` — esperado para um design sem hierarquia; sem impacto.

## Decisões de síntese (sem alteração de arquitetura)

- `word[]` e `imem[]` são exportadas como memórias (`$mem_v2` → `reg` arrays no netlist),
  prontas para mapeamento em BRAM/SSRAM na etapa P&R/FPGA.
- FSM e datapath sequenciais mapeados em FFs; ALU (add/sub/mul/div/and/or/xor/not/mov)
  mapeada em lógica combinacional de portões.
- Nenhuma modificação no RTL; `micro.sv`/`micro_tb.sv` congelados no snapshot.

## Regressão obrigatória (preservada)

`reports/../regression_tb_out.txt`:
```
[OK] 10 vetores - micro.sv reproduz o oraculo micro.c (resid 0)
```
Comandos:
```
iverilog -g2012 -o micro_tb.vvp micro.sv micro_tb.sv
vvp micro_tb.vvp
```

## Próximos passos (a avaliar)
- Place-and-route / alvo FPGA (vendor toolchain ou openflow) — a decidir com o usuário.
- Mapeamento das memórias em primitivas BRAM do alvo.
- Equivalência pós-síntese (opcional) via co-simulação do netlist.