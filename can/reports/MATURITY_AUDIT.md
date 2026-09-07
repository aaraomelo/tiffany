# MATURITY AUDIT — auditoria geral de maturidade do Microprocessador Fractal no FPGA

**Data:** 2026-09-07
**Modo:** auditoria somente — nenhum arquivo de baseline foi alterado.
**Baseline:** commit `dce95a66`, tag `fpga-postpnr-functional-baseline` (única âncora versionada).
**Toolchain:** yosys 0.66 (sha `7f8fdfd8d7bc08c749a2a969388d3425d4f369d5`) + nextpnr-ecp5 0.7 + prjtrellis ecppack 1.4 (`srcinfo-cache-31091-g511025d6fb`) + iverilog v13 (msys2/mingw64; logs também citam v12 Windows) — `C:/msys64/mingw64/bin`.
**Alvo:** Lattice ECP5 LFE5U-25F-6CABGA256.
**Evidências (seção por seção):** `.snapshot/{rtl-synthesis-pass,fpga-pnr-pass,fpga-timing-divmc-pass,fpga-postpnr-functional-pass}/` + `can/micro.sv`, `can/micro.c`, `can/ISA_CONTRACT.md`, `can/reports/{pnr_findings,MUL_AUDIT,CRITICAL_PATH_AUDIT}.md`.

## Verdict (resumo)

```
VERDICT: REPRODUCIBLE TOOLCHAIN REALIZATION (condicional)

Nivel FPGA honesto:........... L5 (netlist pos-P&R funcional, 10/10 vs oracle)
                               + bitstream gerado (L6), NUNCA carregado em silicio
Resultado funcional........... 10/10 vetores V01..V10 (RTL e netlist pos-P&R)
Timing p/ 50 MHz.............. PASS (Fmax 52,49 MHz, path 19,1 ns, slack ~+0,9 ns)
ISA vs microarquitetura....... preservada (serializacao 1R/1W + DIV multi-cycle)
Reprodutibilidade.............
```

- **Bloqueadores para subir de `REPRODUCIBLE TOOLCHAIN REALIZATION`:**
  1. Fonte RTL (`can/micro.sv`, `micro_tb.sv`, `synth_micro.ys`) **não versionada** no git (untracked); snapshots `rtl-synthesis-pass`, `fpga-pnr-pass`, `fpga-timing-divmc-pass` e `reports/` **untracked**. Um clone limpo não reconstrói a cadeia completa.
  2. **Sem `.lpf`/`.sdc`/constraints**: P&R rodou com constraints derivadas ("Placed 0 cells based on constraints", `--freq 50`), **sem seed registrado** → bitstream não é board-ready nem bit-reproducible.
  3. **Zero validação em silício**: sem placa, sem programador (openocd/fujprog/dfu-util ausentes), sem medição de relógio/IO real.

Somente a camada funcional pós-P&R (16 arquivos do snapshot commitado) é reconstruível por outro operador com o mesmo toolchain.

---

## 1. PROVEN (comprovado)

### 1.1 Contrato ISA (L0/L1)
- `micro.c` / `micro.h` / `api.h`/`ISA_CONTRACT.md` — instrução de 80 bits (addr_a 24 + op 8 + addr_b 24 + addr_r 24, little-endian, `addr & 0xFFFFFF`, `addr % 4096`); opcodes `HALT 0, ADD 1, SUB 2, MUL 3, DIV 4`; contrato DIV `a = q*b + r, 0 <= r < b; DIV(a,0) = (0,a)`; wrappers DIVQ.
- Oracle C executável (compilável), `vectors_isa.c`, `demo_isa_disco.c`, módulos CAN/J1939 (`can.c`, `j1939*.c`).
- **Hash:** `can/micro.c` == `10_micro.c` (snapshot) = `41B2AD6F...`; `api.h` = `DBF8EE75...` → oráculo é o que foi validado.

### 1.2 RTL (L2 — micro.sv)
- `micro.sv` (hash `D3F5DB4B...`, idêntico à cópia em `.snapshot/rtl-synthesis-pass/`) — RTL síncrona do core.
- **Regressão de contrato RTL vs oráculo:** `10_contract_regression` e `regression_tb_out.txt` → **V01..V10, 10/10, resid 0** (processa ADD/SUB/MUL/DIV/HALT e fluxo pc/steps/timeout).

### 1.3 Síntese RTL→netlist (L2/L3)
- `synth_micro.ys` (yosys 0.66): `rtl-synthesis-pass` → gate stats **30.838 células, 240 FFs, 2 memórias** (`word` 4096×64 = 262.144 bits; `imem` 256×8), 30.596 portões. PASS.
- `11_synth_ecp5_top.log`: inferência EBR `$__DP16KD_` para `imem` e `word`; **24 DP16KD, 10 MULT18X18D** no mapa ECP5.

### 1.4 Netlist pré-P&R (L3/L4)
- `03_pre_net.v` (2,5 MB) e `04_micro_divmc_post_struct.v` (2,58 MB) — estrutural pós-síntese ECP5, compila e **simula 10/10** contra oráculo.

### 1.5 P&R (L4/L5)
- `nextpnr-ecp5` (LFE5U-25F-6CABGA256): `12_nextpnr.log` exit 0; **utilização:** LUT4 2184/24288 (logic 1670 + carry 514), DFF 895/24288, TRELLIS_IO 31/197, DCCA 1/56, **DP16KD 24/56, MULT18X18D 10/28**, ALU54B 0, EHXPLLL 0.
- Timing: estimativa intermediária **45,42 MHz (FAIL@50)** na 1ª iteração → **final 52,49 MHz (PASS@50)** (log linha 372). Ver `MUL_AUDIT.md` e `CRITICAL_PATH_AUDIT.md` (KEEP; path 19,1 ns).

### 1.6 Funcional pós-P&R (L5)
- `13_micro_tb_postpnr.sv` + `05_wrap_pre.v` + `01_ecp5_sim_ext.v` (modelos whitebox DP16KD/MULT18X18D/DCCA) + `10_micro.c` oráculo.
- `09_v01_v10_postpnr.log`: **V01..V10, 10/10 OK** (result/pc/steps/halted/timeout idênticos ao oráculo).
- `14_reproducibility_check.log`: **re-execução 10/10 OK** (mesmo comando, reprodutivo).
- `08_probe9_roundtrip.log`: sonda de endereço/pino do DP16KD `word` confirma store/read roundtrip (`word[0]=10`, `word[1]=5`).

### 1.7 Bitstream (L6 — gerado, não validado)
- `fpga-timing-divmc-pass/09_micro_divmc.bit` (**198.600 bytes**, header `Part: LFE5U-25F-6CABGA256`) + `08_micro_divmc.txt` (config text, 1,54 MB) — gerado por ecppack.
- `fpga-pnr-pass/09_micro_ser.bit` (409.613 bytes) — variante serializada mais antiga.
- **Nunca carregado em hardware.**

### 1.8 Auditorias antigas (registro)
- `MUL_AUDIT.md` → KEEP; `CRITICAL_PATH_AUDIT.md` → KEEP (Fmax 52,49; slack ~+0,9 ns).

---

## 2. MODELOS / ASSUNÇÕES (modelos usados e suas premissas fortemente explícitas)

| Modelo | O que é | Assunção crítica | Status |
|---|---|---|---|
| `01_ecp5_sim_ext.v` DP16KD | Whitebox **funcional** (mapa EBR 512×36, slicing por DATA_WIDTH) | Cabeçalho do arquivo e MILESTONE dizem: "**functional verification aid, NOT a timing model**". | VALIDADO funcionalmente (probe9 + 10/10); **TIMING NÃO representado** |
| `MULT18X18D` | Multiplicador combinacional `aa*bb`, portas C/muxes "configuradas para 0" pelo toolchain | A/B/P somente; registros/OCE descartados no modelo | Compatível com netlist; sem modelo de timing |
| `DCCA` | Buffer de clock global, combinacional transparente | Sem atraso de clock | Entrega do clock global tratado pelo P&R real |
| Reset do EBR | DP16KD retém conteúdo (sem reset de dados) | Delta documentado: **reset NÃO zera `word[]`** (responsabilidade do host) | Documentado em MILESTONE/fpga-pnr-pass |
| `d_dout` | Leitura **síncrona** (1 ciclo após request) | Delta da serialização | Documentado |
| FSM | Codificação `one-hot` (auto → one-hot, confirmado no log de síntese) | Sem impacto funcional | Confirmado |
| Timing nextpnr | STA com constraints **derivadas** (`--freq 50`), sem `.lpf`/`.sdc` | "Placed 0 cells based on constraints" | Modelo, **não silício** |
| Relógio | TB usa clock virtual 100 MHz (10 ns); P&R constrangido a 50 MHz sintético | Sem oscilador real, sem PLL (EHXPLLL 0) | Somente em simulação |

> **Inconsistência documental encontrada (baixa gravidade, registrar):** o bloco de comentários do cabeçalho de `01_ecp5_sim_ext.v` (linhas 20–23) ainda descreve o mapeamento **antigo** (`w4: lane = ADA[2:0]`), enquanto o código usa o mapa **corrigido** (`mem[ada[13:5]][ada[4:2]*4 +: 4]`, linhas 415–419), conforme MILESTONE. O **código** é o que foi validado (probe9+V01..V10); o **comentário está desatualizado** → risco documental, não funcional.

---

## 3. GAPS / NÃO PROVADO (não confundir com os modelos acima)

1. **Silício (L7):** nenhuma placa ECP5; nenhum programador (openocd/fujprog/dfu-util ausentes). Não há medição de timing, IO, clock global, reset ou consumo em hardware.
2. **Constraints reais:** não existe `.lpf`/`.pcf`/`.sdc` em todo `can/`. Pins do wrapper (31 IOs: clk, rst_pin, wr, addr[8:0], din[7:0], dout[7:0], done, halted, timeout) **não têm posição física/standard atribuída** → `09_micro_divmc.bit` não é carregável numa placa específica sem um `.lpf`.
3. **Reprodutibilidade de repositório:** só `fpga-postpnr-functional-pass/` (16 arquivos) está no git. RTL, scripts, snapshots de síntese/P&R/timing e reports estão **untracked** → clone limpo não reconstrói a cadeia (ver §6).
4. **Cobertura:** apenas 10 vetores V01..V10; não há matriz exaustiva de opcodes (e.g., todos os modos de `addr mod 4096`, DIV por 0/1 finos, bordas de `canal`, estouro de `steps/timeout`).
5. **Timing ≠ silício:** EBR clk→Q 5,8 ns e path 19,1 ns são estimativas do modelo; jitter/derating/temperatura não avaliados (margem ~+0,9 ns é fina para 50 MHz).
6. **Física de IO/CAN:** camada física CAN/CANL/CANH, transceiver, isolamento e sinais elétricos — só implementados em C, nunca em hardware.

---

## 4. ISA vs MICROARQUITETURA (separação de contrato e implementação)

**ISA (arquitetura, congelada):** `ISA_CONTRACT.md` — 80 bits, opcode, endereços 24 bits mod 4096, semântica `ADD/SUB/MUL/DIV`, `DIV(a,0)=(0,a)`, HALT, pc/canal/steps/timeout.

**Microarquitetura FPGA (implementação, variáveis):**
- **Serialização DISCO 1R/1W** (decisão `pnr_findings.md` — 3R/2W era inviável: 262.144 FFs + 1,3M muxes = 1082% do orçamento): estágios `S_RD_A→S_LD_A→S_RD_B→S_LD_B→S_ALU→S_WB`; **+2 ciclos** por instrução binária, **+1** unária; `steps_out` conta **instruções**, não ciclos.
- **DIV multi-cycle**: datapath 65 bits, 1 bit/ciclo (63→0), `r<2b`, `DIV(a,0)=(0,a)` — semântica idêntica ao `DIV`/`DIVQ` do oráculo C.
- **MUL** combinacional via `MULT18X18D` (10 blocos).
- **EBR**: `word` em DP16KD (síncrono, sem reset de conteúdo), `imem` em DP16KD; leitura pós-P&R igual a RTL por construção do modelo.

**Comprovação de preservação de ISA:** V01..V10 idênticos entre micro.sv (L2), netlist pré-P&R (L3) e netlist pós-P&R (L5) — nenhuma divergência introduzida pela microarquitetura. Latência muda (ciclos), **semântica não muda** (steps/pc/canal/done/halted/timeout).

---

## 5. FPGA REALIZATION LEVEL (escala honesta)

| Nível | Critério | Estado | Evidência |
|---|---|---|---|
| L0 | Ideia/contrato ISA documentado | OK | `ISA_CONTRACT.md`, `microprocessador.tex` (referenciado) |
| L1 | Simulação de alto nível (oráculo C) | OK | `micro.c`, `vectors_isa.c`, `demo_isa_disco.c` |
| L2 | RTL sintetizável + sim RTL | OK | `micro.sv` 10/10 resid 0 |
| L3 | Síntese RTL→netlist | OK | `rtl-synthesis-pass`, `11_synth_ecp5_top.log` |
| L4 | Netlist pré-P&R funcional | OK | `03_pre_net.v`, `04_micro_divmc_post_struct.v` 10/10 |
| L5 | **Netlist pós-P&R funcional + timing estimado** | **OK** | `09_v01_v10_postpnr.log`, `14_reproducibility_check.log`, Fmax 52,49 @50 PASS |
| L6 | Bitstream place/route gerado | **Gerado, não validado** | `09_micro_divmc.bit` (ecppack) |
| L7 | Validação em silício (placa) | **NÃO** | sem placa/programador |

**Classificação: L5 atingido e demonstrado; L6 artefato produzido mas não qualificado; L7 pendente.** O gap L5→L7 não é de design (topologia e timing estimado estão OK) e sim de **hardware físico e de board-ready (constraints/pinos/seed)**.

---

## 6. REPRODUTIBILIDADE

**Versionado no git (commit `dce95a66`, tag `fpga-postpnr-functional-baseline`):** apenas 16 arquivos de `fpga-postpnr-functional-pass/` (modelos EBR+MULT+DCCA, netlists, TB, log P&R, log repro, `10_micro.c`, MILESTONE).

**NÃO versionado (untracked — verificado):**
- `can/micro.sv`, `can/micro_tb.sv`, `can/synth_micro.ys` (fonte RTL e script!);
- `.snapshot/rtl-synthesis-pass/`, `.snapshot/fpga-pnr-pass/`, `.snapshot/fpga-timing-divmc-pass/` (incl. bitstreams);
- `can/reports/` (incl. este relatório).

**Consequência:** um operador com o mesmo toolchain consegue reproduzir a **validação funcional pós-P&R** (arquivos commitados) e re-gerar o bitstream **somente se** recuperar os arquivos untracked. Seed do nextpnr e linha de comando completa do P&R **não registrados** → o bitstream não é determinístico/re-gerável do git. Toolchain pinado (hashes acima) mitiga deriva de versão, mas não a ausência de versionamento.

---

## 7. RISCOS TÉCNICOS

| # | Risco | Prob. | Impacto | Mitigação atual / recomendada |
|---|---|---|---|---|
| R1 | Bitstream não carregável sem `.lpf`/pinos | **Certa (até criar LPF)** | Bloqueia bring-up | Criar `.lpf` dos 31 pinos + clk; P&R com seed registrado |
| R2 | Repro perdida (RTL/scripts/snapshots untracked) | Alta (OneDrive/limpeza) | Perda da cadeia | `git add` RTL + 3 snapshots + reports |
| R3 | Margem de timing fina (~+0,9 ns) p/ silício | Média | Falha @50 em silício | Medir em placa; derate nos modelos |
| R4 | Modelo EBR funcional ≠ silício (timing, OCE, reset content) | Média | Divergência rara em HW | Probe9 + testes adicionais; validar na placa |
| R5 | Cobertura de 10 vetores insuficiente | Média | Bugs de borda não vistos | Expandir vetores (matriz ISA) no próximo passo |
| R6 | Física CAN/IO não validada | Alta p/ HW | Integração CAN falha | Testes em bancada com transceiver |
| R7 | Ferramentas OSS (yosys/nextpnr) mudam de comportamento | Baixa | Bitstream diferente | Pinagem de versão (documentada) |

---

## 8. NEXT EXPERIMENT (um único, maior valor informacional)

**Recomendação: fechamento de “board-readiness + reprodutibilidade” — re-P&R com `.lpf` de pinos (31 IOs + relógio) e seed registrado, commit de TODAS as fontes/snapshots, re-verificação 10/10 pós-P&R e re-geração do bitstream.** 
- Não requer hardware (executável imediatamente com o toolchain instalado).
- Resolve R1+R2 (pinos e repro), qualifica o bitstream para uma placa ECP5 e deixa o design em estado de carregar assim que uma placa existir.
- Inclui etapa de cobertura: estender a regressão pós-P&R além de V01..V10 (matriz ISA/DIV/CANAL).

> **Experimento físico NOTA EXPLÍCITA:** a validação decisiva de silício (carregar `09_micro_divmc.bit` em uma placa ECP5 LFE5U-25F-6CABGA256, medir timing, IO, clock global, reset e CAN) **depende de obter uma placa ECP5 compatível** e de um programador (JTAG/SPI — openocd/fujprog/dfu-util atualmente ausentes). Não é executável neste ambiente.

**Por que este é o de maior valor vs outros:** a progressão L5→L7 é bloqueada por hardware físico, mas o caminho L5→L6-qualificado→pronto-para-carregar é obtido neste experimento intermediário, que também transforma o repositório em fonte da verdade (hoje parcialmente órfã).

---

## 9. VERDICT FINAL

### Tabela por área

| Área | Status | Classificação |
|---|---|---|
| ISA / contrato | Congelado, oráculo C validado | **COMPLETO** (L0/L1) |
| RTL (micro.sv) | 10/10 vs oráculo | **VALIDADO** (L2) |
| Síntese RTL→netlist | yosys 0.66 PASS | **VALIDADO** (L3) |
| Netlist pré/Pós-P&R funcional | 10/10 (repetido em repro) | **VALIDADO** (L4/L5) |
| Timing @50 MHz | Fmax 52,49 (PASS), path 19,1 ns | **OK no modelo, MEDIR em silício** |
| Bitstream (ecppack) | Gerado, não carregado | **GERADO — NÃO QUALIFICADO** (L6) |
| Placa física | — | **NÃO DISPONÍVEL** (L7) |
| Constraints/pinos/seed | Ausentes | **FALTANTE** |
| Repro de repositório | RTL/scripts/snapshots untracked | **INCOMPLETA (crítica)** |
| Física CAN/IO | C-only, sem HW | **NÃO PROVADO** |

### Escala de veredito (escolha única)

- ~~EXPERIMENTAL~~ — descartado: há cadeia de toolchain fechada em 50 MHz com 10/10 funcional pós-P&R.
- **REPRODUCIBLE TOOLCHAIN REALIZATION (escolha honesta)** — a realização em toolchain (síntese+P&R+funcional+timing) está provada e a camada funcional é reprodutível; **mas** a repro completa depende de versionar RTL/scripts/snapshots e registrar constraints/seed.
- ~~FPGA-READY PENDING HARDWARE~~ — não alcançado ainda: faltam `.lpf` de pinos, seed registrado e fontes commitadas (R1+R2); o bitstream atual não é board-ready.
- ~~HARDWARE VALIDATED~~ — não alcançado: nenhum dado de silício (R3–R6).

### Veredito final

```
VERDICT: REPRODUCIBLE TOOLCHAIN REALIZATION

Nivel atingido:......... L5 (netlist pos-P&R funcional 10/10) + bitstream L6 gerado
Nivel NAO atingido:..... L7 (silicio) — sem placa/programador
Condicoes para subir um nivel:
  1) versionar RTL + 3 snapshots + reports (fecha repro); 
  2) criar .lpf (31 pinos + clk) e re-rodar P&R com seed registrado;
  3) re-validar 10/10 pós-P&R e re-gerar bitstream.
Apos isso: FPGA-READY PENDING HARDWARE. Silicio exige placa ECP5 LFE5U-25F-6CABGA256.

Nenhum arquivo de baseline foi alterado. Apenas este relatório (audit-only).
```