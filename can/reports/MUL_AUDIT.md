# MUL AUDIT — auditoria técnica do MUL 64-bit (baseline `fpga-postpnr-functional-baseline`)

**Data:** 2026-09-07
**Modo:** auditoria somente — nenhum arquivo do baseline foi alterado.
**Baseline:** commit `dce95a66`, tag `fpga-postpnr-functional-baseline`.
**Toolchain:** yosys 0.66 + nextpnr-ecp5 0.7 + prjtrellis 1.4 (MSYS2/mingw64).

## Resumo executivo

```
MUL AUDIT

Current Fmax (isolado, FF->MUL->FF)..... 61,04 MHz   (alu_crit.sv OP=MUL)
Current Fmax (core completo pos-P&R).... 52,49 MHz   (PASS @ 50 MHz)
Current slack @ 50 MHz (core)........... +0,95 ns    (slack minimo ~948-1854 ps)
DSP usage................................ 10/28 (35,7%)
  - 8 DSPs 18x18 cheios
  - 2 DSPs de borda truncados (10 / 18 bits) - produto parcial parcialmente
    descartado (nao afeta os 64 bits do resultado)
Area (core completo).................... LUT4 2184/24288 (logic 1670 + carry 514),
                                          FF 895, DP16KD 24/56, IO 31/197
Critical path (nextpnr, full core)...... imem.3.0.DOB7 -> decode/FSM ->
                                          mwdata (PFUMZ/CCU2C) -> mwdata[63] FF
                                          = 19,1 ns (8,2 logic + 10,8 routing)
                                        NENHUM hop em MULT18X18D.

Verdict: KEEP

Reason:
1. O MUL NAO esta no critical path do core completo. O caminho pior
   (19,1 ns) sai da imem (EBR DOB) e passa pela FSM + selecao/soma do
   mwdata[63]; nao existe nenhum MULT18X18D nele.
2. MUL isolado cai a ~61 MHz > 52,49 MHz do core -> otimizar o MUL
   hoje NAO aumenta o Fmax do core (o limite 52,49 e outro caminho).
3. 50 MHz ja tem margem (+0,95 ns) e esta PASS.
4. DSPs ja sao usados de forma eficiente (10/28, Karatsuba do yosys vs
   16 do grid 4x4 naive).
```

---

## 1. Critical path do MUL no relatorio nextpnr

O relatorio final do P&R (`06_nextpnr.log`, Fmax 52,49 MHz) lista o
caminho critico (posedge -> posedge):

```
Source imem.3.0.DOB7            (EBR imem, leitura sincrona)   5,8 ns
 -> LUT4 (readout/decodificacao)
 -> state (FSM, DI dos FFs)
 -> core.mwdata_PFUMZ ... CCU2C ... (mux/soma do mwdata, bits 17..63)
Sink  core.mwdata_TRELLIS_FF_DI_63.M
Total 19,1 ns  (8,2 ns logica + 10,8 ns roteamento)  -> Fmax 52,49 MHz
```

- **Nenhum hop passa por instancia `MULT18X18D`.**
- A nuvem `mwdata` (onde os produtos parciais do MUL somam via carry
  chains CCU2C e onde o mux de write-back seleciona `d_din/div_q/alu_r`)
  compoe a metade final do caminho, mas a parte dominante e a leitura da
  imem + decodificacao da FSM.
- Slack minimo @ 50 MHz: +948..1854 ps (histograma do mesmo log) -> ~+0,95 ns.

## 2. Decomposicao do multiplicador 64-bit nos MULT18X18D

Inspecionado `03_pre_net.v` (netlist pos-P&R):

- **10 MULT18X18D** (utilizacao 10/28 = 35,7%).
- Fatias usadas (A = a_tmp, B = b_tmp):
  - 8 DSPs com 18x18 cheios;
  - 1 DSP com A 18x10 (borda baixa do produto);
  - 1 DSP com A 10x18 (borda alta do produto).
- Sinais internos `P9/P8/P27/P35/P5/P23` = produtos parciais; a soma
  final e feita em cadeias CCU2C (carry LUTs) dentro da nuvem `mwdata`.
- Isso e a decomposicao **Karatsuba-estilo do yosys** (menos DSPs,
  mais reducao em LUT) contra o **grid 4x4 classico de 16 DSPs**. Ou
  seja: 10 DSPs e uma escolha de area/numero de DSP eficiente.

**Eficiencia DSP:** adequada. Nenhum DSP ocioso/com recursos desperdicados.

## 3. MUL e realmente o proximo gargalo do core completo?

**Nao.** Evidencia:

| Dado | Fonte |
|---|---|
| MUL isolado (FF->MUL->FF, mux removido) | 61,04 MHz (alu_crit.sv OP=3) |
| Core completo pos-P&R | 52,49 MHz (nextpnr, exit 0 @ 50) |
| Path critico do core | imem -> FSM -> mwdata (sem DSP) |

- O MUL isolado (61,04) fica **acima** do Fmax do core (52,49). O MUL
  nao e o fator limitante real.
- O Fmax do core e ditado pela imem (saida de EBR) + decodificacao de
  instrucao/FSM + selecao/soma de mwdata (bits altos).
- O MUL "61 MHz" e um teto futuro: so importa quando o nucleo passar de
  ~52,5 MHz. Para o requisito atual (50 MHz) ha margem de +0,95 ns.

## 4. Alternativas (avaliacao conceitual — NENHUMA implementada)

| Alternativa | Ganho Fmax esperado (cenario atual) | Custo de area | Latencia | Impacto controle | Risco regressao | Muda microarquitetura |
|---|---|---|---|---|---|---|
| **KEEP (fazer nada)** | 0 (ja PASS) | 0 | 0 | nenhum | nenhum | nao |
| Multi-cycle MUL (espelho do DIV) | ~0 hoje (MUL nao limita); so depois de subir o 52,49 | ~0 (DSPs reusados) | +N ciclos por MUL | FSM nova (estados de iteracao) | alto | sim |
| Pipeline via registradores do DSP (INPUT_REG/OUTPUT_REG) | ~0 hoje; +~15-20% quando o DSP for o limite | 0 (regs nativos) | +1 ciclo | latencia de canal +1 | medio | sim (latencia) |
| Grid 4x4 = 16 DSPs | ~0 a +5% (arvore de soma mais rasa, roteamento maior) | +6 DSPs (35% -> 57%) | 0 | nenhum | baixo | nao |
| Parcial em LUT (menos DSP) | negativo (LUT tree ja e o que pesa) | +LUTs | 0 | nenhum | medio | nao |
| DSP+LUT hibrido (cascata/ADDN) | igual ao "pipeline via regs DSP" | 0 | +1 | idem | medio | sim |

**Observacao honesta:** a arvore de soma/reducao do MUL participa da
nuvem `mwdata` que aparece no path critico. Reduzir a profundidade dessa
arvore (ex. pipeline) tem potencial marginal de aliviar o 19,1 ns
(~-0,5 a -1,0 ns) mesmo hoje. Porém o dominante do path e a imem
(5,8 ns) + roteamento global (10,8 ns); o ganho esperado do MUL e
pequeno e o custo de regressao nao compensa enquanto o requisito e
50 MHz.

## 5. Conclusao

**Verdict: KEEP**

Razoes:
1. Requisito atendido: 52,49 MHz, PASS @ 50 MHz, slack ~+0,95 ns.
2. MUL NAO e o critical path do core (imem/FSM/mwdata e o pior); MUL
   isolado (61,04) e superior ao Fmax do core.
3. Otimizar o MUL agora nao aumenta Fmax (o 52,49 continuaria
   limitando) e adiciona risco de regressao funcional numa ISA recém-
   validada.
4. DSPs ja eficientes (10 Karatsuba vs 16 naive; 8x 18x18 cheios).
5. Se um dia o alvo de clock subir (ex. >55 MHz), a proposta mais
   promissora e **pipeline com registradores nativos do DSP**
   (INPUT_REG/OUTPUT_REG, sem DSP extra) ou multi-cycle; filho desta
   auditoria, aguardando autorizacao.

## Nada foi alterado

- `micro.sv`, `micro_tb.sv`, ISA/opcodes/formato 80-bit intactos.
- Baseline `fpga-postpnr-functional-baseline` (commit `dce95a66`)
  preservado.
- Nenhum arquivo de snapshot modificado. Este relatorio e o unico novo.