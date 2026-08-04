SO NO CRISTAL — placa multiplicadora/somadora analogica (log-domain, Pontryagin)
Autor: Aarao Melo Lopes

CONTEUDO:
  so_cristal_gerbers.zip  — Gerbers RS-274X + Excellon (mandar para a fab)
  BOM.csv                 — lista de materiais
  so_cristal.kicad_pcb    — projeto KiCad (fonte)

FABRICACAO DA PCB (JLCPCB / PCBWay):
  - 2 camadas, FR-4, 80 x 64 mm, 1.6 mm, acabamento HASL ou ENIG.
  - Contorno em Edge_Cuts (.gm1); furos em so_cristal.drl (Excellon).
  - DRC do KiCad: 0 violacoes, 0 itens desconectados.

MONTAGEM:
  - Alimentacao +-12V em J3. Entradas a,b em J1. Saidas: MULT (a*b) e ADD (a+b) em J2.
  - Q1-Q3: usar o PAR/QUAD CASADO (SSM2212, ou matriz LM3046) — RECOMENDADO, nao opcional.
    O log/antilog so' fecha se VT e Is rastreiam (mesma pastilha): e' o casamento que
    cancela VT (o translinear). 3 MMBT3904 discretos so' como ultimo recurso, e ainda
    assim COLADOS termicamente (o descasamento vira erro no produto).

TESTE DE ACEITACAO:
  - O sinal analogico deve reproduzir a ISA discreta (ADD/MUL). Referencia validada:
    universe/tools/so_cristal.c (analogico == ISA, 40000/40000, residuo 0).

BRING-UP (bancada, antes de qualquer tiragem — checklist):
  A aritmetica e a topologia JA' estao validadas (so_cristal.c == ISA, residuo 0); o que
  segue e' CALIBRACAO fisica do eval board, nao verificacao do conceito.
  [ ] 1. Par casado no lugar (SSM2212/LM3046). Sem isso o produto nao casa.
  [ ] 2. Escala do multiplicador: por uma corrente de referencia Iref que ponha a*b na
         faixa util (0..10 V). Sem Iref a saida do log-domain sai em escala minuscula.
  [ ] 3. Estabilidade do log-amp: capacitor de compensacao (Cc) na malha do op-amp com
         transistor — o transdiode tende a oscilar sem ele.
  [ ] 4. Faixa/clamps: o log exige I>0 (entradas positivas); garantir MULT e ADD dentro
         de +-12 V sem saturar o op-amp.

NOTA: eval board / prototipo. Fabricavel como esta' (Gerbers/DRC 0). O bring-up acima e'
a etapa de bancada padrao de todo analogico log-domain — prevista, nao um conserto.
