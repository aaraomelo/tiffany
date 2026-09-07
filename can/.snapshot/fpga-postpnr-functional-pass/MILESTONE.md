# MILESTONE — FPGA POST-P&R FUNCTIONAL — 10/10 PASS

**Data:** 2026-09-07
**Alvo:** Lattice ECP5 (LFE5U-25F, CABGA256), toolchain OSS
  yosys 0.66 (synth_ecp5) + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64)

## Registro

```
RTL oracle             -> PASS
pre-P&R netlist        -> PASS
post-P&R netlist       -> PASS
V01..V10               -> 10/10
```

## Resultado funcional pós-P&R

A netlist pos-P&R do core `micro_divmc` foi funcionalmente validada
contra o oraculo da ISA atraves de **simulacao estrutural ECP5**
(iverilog + `ecp5_sim_ext.v`, células `common_sim.vh`/`ccu2c_sim.vh`),
V01..V10 = **10/10**, resultados identicos ao modelo de referencia.

| Vetor | Resultado (hex) | Oracle (dec) | PC | Steps |
|---|---|---|---|---|
| V01 | 0x2a | 42 | 10 | 2 |
| V02 | 0x3a | 58 | 10 | 2 |
| V03 | 0x90 | 144 | 10 | 2 |
| V04 | 0x0e | 14 | 10 | 2 |
| V05 | 0xdeadbeef | 0xDEADBEEF | 10 | 2 |
| V06 | **0x63** | **99** | 30 | 3 |
| V07 | 0x00 | 0 | 0 | 1 |
| V08 | 0x2d | 45 | 20 | 3 |
| V09 | 0x14 | 20 | 20 | 3 |
| V10 | 0x00 | 0 | 20 | 3 |

> Nota de conferencia: V06 = **99 decimal = 0x63** (não 0x3f). O valor
> `0x3f` (63) não corresponde ao oraculo `rexp[6] = 64'd99` nem ao log
> `10_contract_regression`/`09_v01_v10_postpnr.log` (`V06 OK result=63`).

## Causa raiz documentada

A falha anterior **não estava na netlist nem no P&R**. O erro estava no
**modelo de simulacao das primitivas ECP5** (`ecp5_sim_ext.v`, módulo
`DP16KD`).

Para `DP16KD` com `DATA_WIDTH=4` (w4):

```
endereco EBR = ADA[13:2]
lane         = ADA[4:2]
ADA[1:0]     = ignorados
```

O modelo anterior utilizava incorretamente `ada[2:0]`.

Mapeamentos corrigidos (write A, write B e read):

```
w4  -> lane = a[4:2]
w2  -> lane = a[4:1]
w1  -> lane = a[4:0]
w9  -> lane = a[4:3]  (ja estava correto)
w18 -> posicao unica: mem[a[13:5]] (a[4] = seletor de 2 lanes)
```

O erro produzia **corrupcao aparente das palavras EBR** (todas as
palavras com mesmo `ADA[2:0]` colidiam na mesma lane) e gerava **falsos
failures V08-V10** (ex.: V08 0x18 e V09 0x20 observados antes da
correcao; corretos = 0x2d e 0x14).

## Roundtrip host (probe EBR) — demonstra o bug e a correcao

`06_probe9.v` + `07_probe9_roundtrip_set.v` gravam word[k] via host
(`ADA = k<<2`) e releem pelo porto B (`ADB = k<<2`).

Resultado final (log `08_probe9_roundtrip.log`):

```
word[k] escrito      10  5  3  -  45  -  77  -  99
read back (byte)    0a 05 03 00 2d 00 4d 00 63
```

Correspondencia de lanes: `ADA=ADB=k<<2` -> row=a[13:5], lane=a[4:2]:

```
k=0 -> 0x00 -> row0 lane0  -> 0x0a
k=1 -> 0x04 -> row0 lane1  -> 0x05
k=2 -> 0x08 -> row0 lane2  -> 0x03
k=3 -> 0x0c -> row0 lane3  -> 0x00 (nunca escrito)
k=4 -> 0x10 -> row0 lane4  -> 0x2d
k=5 -> 0x14 -> row0 lane5  -> 0x00 (nunca escrito)
k=6 -> 0x18 -> row0 lane6  -> 0x4d
k=7 -> 0x1c -> row0 lane7  -> 0x00 (nunca escrito)
k=8 -> 0x20 -> row1 lane0  -> 0x63
```

## Evidencias (nesta pasta)

| Artefato | Descricao |
|---|---|
| `01_ecp5_sim_ext.v` | **Modelo ECP5 CORRIGIDO** (w1/w2/w4 adicionados, w9 ok) |
| `02_ecp5_sim_ext_dbg.v` | Variante de debug usada na investigacao (eeA/eeB) |
| `03_pre_net.v` | Netlist pos-P&R testada (`module micro_fpga_top`) |
| `04_micro_divmc_post_struct.v` | Netlist pos-P&R standalone (`module top`) |
| `05_wrap_pre.v` | Wrapper usado na simulacao estrutural |
| `06_probe9.v` | Probe/roundtrip EBR (demonstra o bug da lane w4) |
| `07_probe9_roundtrip_set.v` | Build combinado do roundtrip (pre_net + cells + probe) |
| `08_probe9_roundtrip.log` | Log final do roundtrip host |
| `09_v01_v10_postpnr.log` | **Log final V01..V10 = 10/10** |
| `10_micro.c` | Oráculo da ISA (micro.c) |
| `11_synth_ecp5_top.log` | synth_ecp5 (word/imem via `$__DP16KD_`) |
| `12_nextpnr.log` | P&R de fechamento @ 50 MHz (exit 0, Fmax 52,49 MHz) |
| `13_micro_tb_postpnr.sv` | Testbench estrutural V01..V10 (sync-read, importedo de combO2) |

### Rebuild do teste funcional (para reproducao)

```
iverilog -g2012 -I <yosys>/share/yosys/lattice -s main \
  -o combO2.vvp <pre_net> <common_sim.vh> <ccu2c_sim.vh> \
               <ecp5_sim_ext.v> <wrap_pre.v> <micro_tb_postpnr.sv>
vvp combO2.vvp   -> V01..V10 10/10
```

## Distincao oficial

```
Timing:
Fmax = 52,49 MHz  ->  PASS @ 50 MHz

Functional:
post-P&R V01..V10  ->  10/10 PASS
```

## Conclusao formal

> A netlist pos-P&R do core `micro_divmc` foi funcionalmente validada
> contra o oraculo da ISA atraves de simulacao estrutural ECP5, com
> V01..V10 = 10/10 e resultados identicos ao modelo de referencia.

## Nao alterado (regras respeitadas)

api.h, escolha de ISA, opcodes, formato 80-bit (`LOAD->LOAD->ALU->STORE`),
`micro.sv`, programas V01..V10, milestone `fpga-timing-divmc-pass`
preservado. A correcao e **exclusivamente** no modelo de simulacao das
primitivas ECP5. MUL nao foi otimizado (proximo critical path ~61 MHz,
agendado para futura etapa, conforme pedido).