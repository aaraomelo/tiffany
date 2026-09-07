# CRITICAL PATH AUDIT — auditoria do critical path real do core (baseline `fpga-postpnr-functional-baseline`)

**Data:** 2026-09-07
**Modo:** auditoria somente — nenhum arquivo do baseline foi alterado.
**Baseline:** commit `dce95a66`, tag `fpga-postpnr-functional-baseline`.
**Toolchain:** yosys 0.66 + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64).
**Fonte:** `.snapshot/fpga-timing-divmc-pass/07_nextpnr_freq200.txt`; netlist
`.snapshot/fpga-postpnr-functional-pass/03_pre_net.v`.

## Resumo executivo

```
CRITICAL PATH AUDIT

Fmax (core completo pós-P&R)............ 52,49 MHz   (PASS @ 50 MHz)
Slack @ 50 MHz........................... ~+0,90 ns   (periodo 20 - 19,1)
Total / Logic / Routing.................. 19,1 / 8,2 / 10,8 ns
Source................................... core.imem.3.0.DOB7  (EBR imem, leitura sincrona)
Sink..................................... core.mwdata_TRELLIS_FF_DI_63.M
                                       (registrador de dados de escrita do EBR word,
                                        Q alimenta .DIA3 do DP16KD do DISCO)

Path:
  imem EBR DOB (5,8 ns clk->Q) -> 2 LUTs de readout -> byte imem[3] ->
  ~5 LUTs de decode/FSM (state.one-hot DI) -> hop 1,8 ns (15,7)->(23,10) ->
  arvore mwdata: cadeia CCU2C (do bit 17) + PFUMX bit 60 -> BIT 63 -> FF

Logic 8,2 = EBR 5,8 + ~2,4 de LUT/CCU2C  (EBR = 70% da logica)
Routing 10,8 = 56% do path (hops de 1,1 a 1,8 ns + rede 0,6 ns final)

Verdict: KEEP

Reason:
1. 52,49 MHz atende o requisito 50 MHz com ~+0,9 ns de margem (PASS).
2. O path nao e um erro de profundidade logica: a logica e rasa
   (~5 niveis de LUT + cadeia carry); o roteamento (10,8 ns) domina
   sobre a logica (8,2 ns) — cola de posicionamento/roteamento.
3. O 5,8 ns de clk->Q do EBR imem e custo estrutural de buscar
   instrucao em block RAM sincrona (70% da parte logica) — nao um bug.
4. Otimizacoes RTL (retiming do write-back) nao sao necessarias para o
   alvo atual e so seriam avaliadas se o clock alvo subir (>52,5 MHz).
```

---

## 1. Caminho critico oficial (posedge -> posedge, 19,1 ns)

Relatorio `07_nextpnr_freq200.txt` (linhas 181-239):

```
curr total
 5.8  5.8  Source core.imem.3.0.DOB7                     <- EBR imem (leitura sincrona)
 1.1  6.9    Net imem.3.0_DOB7[0]              (15,25)->(15,24)
 0.2  7.2  Source imem.3.0_DOB3_LUT4_Z_D_LUT4_Z_1.F      <- readout do byte
 1.6  8.8    Net ... (15,24)->(14,14)
 0.2  9.0  Source imem.3.0_DOB3_LUT4_Z.F
 0.4  9.4    Net core.imem.3.0_DOB3[3]         (14,14)->(14,14)
 0.2  9.6  Source state_TRELLIS_FF_Q_8_DI_LUT4_Z_D_LUT4_Z_D_LUT4_Z_D_LUT4_Z.F
                                       <- decode/FSM, INIT 16'h0053 (mixa imem + estado)
 0.6 10.2    Net ... [3]
 0.2 10.5  Source ... .F
 1.3 11.8    Net ... [1]          (14,14)->(15,9)
 0.2 12.1  Source ... .F
 0.8 12.8    Net ... [1]          (15,9)->(15,7)
 0.2 13.1  Source state_TRELLIS_FF_Q_8_DI_LUT4_Z_D_LUT4_D_1.F   <- buffer C|D
 1.8 14.9    Net ..._S0[3]        (15,7)->(23,10)      <- HOP CRITICO (roteamento)
 0.3 15.1  Source ...PFUMX_Z_BLUT_LUT4_Z.OFX            <- entrada da arvore mwdata
 1.1 16.2    Net ... [3]          (23,10)->(24,11)
 0.2 16.5  Source ...S1_LUT4_Z.F  <- cadeia CCU2C (bits 17..60)
 0.6 17.1    Net ... [2]          (24,11)->(24,11)
 0.2 17.3  Source ...S1_LUT4_D.F
 0.8 18.2    Net ... [4]          (24,11)->(25,14)
 0.3 18.4  Source core.mwdata_PFUMX_Z_60_BLUT_LUT4_Z.OFX  <- PFUMX bit 60
 0.6 19.1    Net core.mwdata[63]  (25,14)->(25,15)
 0.0 19.1  Setup core.mwdata_TRELLIS_FF_DI_63.M
8.2 ns logic, 10.8 ns routing
```

### Interpretacao funcional do sink

`core.mwdata_TRELLIS_FF_DI_63` nao vai para um simples registrador interno:
em `03_pre_net.v`, o `Q` dele alimenta `.DIA3` de um DP16KD do DISCO
(linha 35875) — ou seja, **e o registrador de dados do porto de escrita do
EBR `word`**. O mux `mwdata` RTL corresponde a
`mwdata = d_wr ? d_din : ((rop == IOP_DIV) ? div_q : alu_r)`
(`micro_alu_divmc.sv:144`, confirmado no log de origem do sink em 144.22-144.28).

Ou seja: o byte buscado na imem decodifica instrucao; o mesmo cone de
decode (que alimenta o DI dos FFs do FSM one-hot) converge para a arvore
`mwdata` que seleciona/soma o que sera escrito no DISCO — e o bit 63 fica
no fim da cadeia carry, fechando o path no FF de escrita.

## 2. Decomposicao do path

| Segmento | Ate (ns) | Delta (ns) | % do path |
|---|---|---|---|
| imem EBR clk->Q + rede local + 2 LUTs readout | 7,2 | 7,2 | 38% |
| decode/FSM (4 niveis de LUT ate (15,7)) | 13,1 | 5,9 | 31% |
| Hop de roteamento (15,7)->(23,10) | 14,9 | **1,8** | 9% |
| Arvore mwdata (CCU2C do bit 17 + PFUMX bit 60 + rede) | 19,1 | 4,2 | 22% |

- **Logic 8,2 ns** = 5,8 (clk->Q EBR imem, ~70%) + ~2,4 (LUTs/CCU2C).
- **Routing 10,8 ns** = 56% do path (hops 1,1 / 1,6 / 1,3 / 0,8 / 1,8 / 1,1 /
  0,6 / 0,8 / 0,6). O 1,8 ns na transicao decode->mwdata e um salto
  fisico de 8 colunas (15,7)->(23,10).

## 3. Evidencias de netlist (03_pre_net.v)

- **FSM one-hot (artefato de implementacao):** yosys detectou a FSM de
  `core.state` e recodificou para **one-hot** (`05_synth_ecp5_top.log`:
  `using auto encoding -> one-hot`). Na netlist, `\core.state` e
  `wire [12:0]` (13 bits, 13 estados S_IDLE..S_END) e os FFs sao
  `state_TRELLIS_FF_Q_*` com Q em `state[2]..state[10]`. O LUT do path
  (`state_TRELLIS_FF_Q_8_DI_..._D_LUT4_Z_D_LUT4_Z_D_LUT4_Z`, INIT `16'h0053`)
  consome bits de imem (`imem.7.0_DOB2[1]`, `imem.3.0_DOB9[2]`,
  `imem.3.0_DOB3[3]`) e decode de estado — e o proprio cone que alimenta o
  DI do FF `state[9]` e tambem entra na arvore `mwdata`.
- **Cadeia de carry de 47 bits em LUT/CCU2C:** a nuvem `mwdata` a partir
  do bit 17 ate o bit 63 combinou selecao do write-back + soma do ALU +
  reducao de MUL (mul/div/alu) numa unica cadeia CCU2C (nomes com ate 20+
  sufixos `CIN/COUT`). Bit 63 = fim da cadeia.
- **`mwdata_TRELLIS_FF_DI_Q [63]`** alimenta `.DIA3` do DP16KD (linha
  35875) — escrita do DISCO espera `mwdata[63]` no bordo, fechando o path.

## 4. Root cause

**Classificacao: MISTA — estrutural (leitura de imem em EBR) + implementacao
(roteamento > logica) + artefato de mapeamento (arvore mwdata fundida com a
FSM one-hot).**

1. **Estrutural:** instrucao vem de block RAM sincrona. O clk->Q do EBR
   (5,8 ns) e custo inevitavel de fetch - RAM e nao evita (LUTRAM seria
   troca arquitetural grande). O mux de write-back `mwdata` e combinational
   no ciclo de escrita, somando ALU/MUL/DIV/d_din em cadeia carry 64-bit.
2. **Implementacao (dominante):** routing 10,8 ns > logic 8,2 ns. O hop
   1,8 ns (15,7)->(23,10) separa o bloqueio de decode/FSM da arvore mwdata;
   a cadeia carry inteira fica em 2-3 linhas de SLICE, concentrando a rede.
   Nenhum `MULT18X18D` participa do path.
3. **Artefato de mapeamento:** a codificacao one-hot + fundir o decode da
   FSM com o mux `mwdata` colocou 4 LUTs extras no caminho entre o byte
   lido e a selecao de escrita. Profundidade logica real e rasa (~5 LUTs
   + cadeia carry); nao e um problema de "muita logica".

## 5. Caminho critico secundario (nao limita o clock)

`posedge -> <async>` (leitura de resultado, 18,1 ns) e `async -> async`
(17,2 ns) estao **abaixo** do path interno 19,1 ns. O path
`word.0.3.DOB0 -> dout[4]$TRELLIS_IO_OUT` (linhas 335-370) soma 7,3 ns de
logica + 10,8 de roteamento e nao impacta o Fmax interno (e saida assincrona
para IO). O max delay assincrono medido: in->out 17,17 / in->clk 13,31 /
clk->out 18,07 ns.

## 6. Alternativas (avaliacao conceitual — NENHUMA implementada)

| Alternativa | Ganho Fmax (cenario atual) | Custo | Latencia | Impacto controle | Risco regressao | Muda microarquitetura |
|---|---|---|---|---|---|---|
| **KEEP (fazer nada)** | 0 (ja PASS @ 50) | 0 | 0 | nenhum | nenhum | nao |
| **Retiming do write-back** (registrar `alu_r`/selecao em S_ALU; escrever em S_WB com um ciclo) | +~15-20% quando alvo >52,5 MHz (quebra o cone comb. entre fetch e escrita) | 1 FF x 64 + FSM/ciclos | +1 ciclo por escrita | FSM/contador de estados | **medio-alto** (ciclo-exato de programa/demo validados) | sim |
| Separar mux de escrita da cadeia ALU (nao fundir `mwdata` com carry do ALU/MUL) | marginal hoje | pouca | 0 | nenhum | baixo | nao |
| Seed / esforco de P&R (reposicionar decode junto ao mwdata) | ~0 a -1 ns (so o par 1,8+1,1) | 0 | 0 | nenhum | baixo | nao |
| imem em LUTRAM (tempo de leitura ~0,7 vs 5,8 ns) | potencial grande (+~4-5 ns) | +RAM distribuida, DISCO e imem | 0 | reset/carga da imem | **alto** (arquitetural) | sim |
| FSM nao-one-hot (4 bits binarios) | ~0 (profun- didade logica ja e rasa) | pequena | 0 | basico | medio | nao |

**Observacao honesta:** a unica alavanca RTL que "ataca" o cerne deste path
e quebrar o caminho combinational entre o byte de instrucao e o mux de
escrita (retiming do write-back) — custo de ciclos e alto risco de regressao
ciclo-exata. As demais sao posicionamento (seed) ou arquiteturais (LUTRAM).
Nenhuma e necessaria enquanto o requisito for 50 MHz.

## 7. Conclusao

**Verdict: KEEP**

Razoes:
1. Requisito atendido: 52,49 MHz, PASS @ 50 MHz, slack ~+0,90 ns.
2. O path nao indica bug logico: logica rasa (~5 LUTs + carry), EBR clk->Q
   estrutural (5,8 ns) e roteamento dominante (10,8 > 8,2).
3. MUL isolado 61,04 MHz e DIV aproveitam o mesmo cone `mwdata`; o limite
   e imem/fetch + roteamento, nao o ALU/DSP (confirmado pela MUL_AUDIT).
4. Otimizacoes so fariam sentido se o clock alvo subir (>52,5 MHz); a mais
   promissora seria **retiming do write-back** (aguardando autorizacao),
   com risco medio-alto de regressao ciclo-exata — filhos desta auditoria.

## Nada foi alterado

- `micro.sv`, `micro_tb.sv`, ISA/opcodes/formato 80-bit intactos.
- Baseline `fpga-postpnr-functional-baseline` (commit `dce95a66`)
  preservado.
- Nenhum arquivo de snapshot modificado. Este relatorio e o unico novo.