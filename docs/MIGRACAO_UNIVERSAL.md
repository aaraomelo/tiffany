# Inventário técnico da migração Peano → Universal

Ordem do gerente/diretor (eval 13/08): **inventariar sem mover**; preparar o
adaptador de equivalência; rodar a bateria ANTES; **aguardar a instrução de
corte**. A regra de ouro da invariância: a migração só ocorre se a bateria
completa mantiver resíduo zero antes e depois — *se o Peano alterar o
comportamento em um único bit, a migração é abortada*.

E a regra da casa que vigia reorganizações (memória do projeto): **o diff da
contagem de medidores por documento é obrigatório — quem sai, sai em
silêncio.**

## 1. A hierarquia a oficializar

- **Camada 𝒰 (Universal)** — infraestrutura abstrata: Corpo, Soma ⊕,
  Multiplicação ⊗, Divisão/fibra, Dual, Inversão, adjunção δ⊣ε, torção J,
  escada de observadores, conservação (E, Parseval), membrana de Dirac,
  Inversor total (R_total).
- **Camada de realização** — Peano (torre, π_k, Maestro/Metrónomo, música,
  histerese, rede dual) e os corpos do Catálogo (estelar, metálico, banal,
  cristal…): cada um = 𝒰 especializado por uma **assinatura** σ.
- **Fundação** — as 8 leis ficam no corpo-estelar (`sub:oitoleis`); os dois
  citam, nenhum re-funda.

## 2. Quem é dono de quê hoje (e o destino proposto)

| estrutura | dono atual | destino no corte | ação |
|---|---|---|---|
| cinco operações + Inversor | corpo_universal §cinco | 𝒰 (já lá) | nada |
| membrana de Dirac (4 teoremas) | corpo_universal §dirac | 𝒰 (já lá) | nada |
| morfologia δ⊣ε + torção | corpo_universal §morfologia | 𝒰 (já lá) | nada |
| primitivas mecânicas + domínios | corpo_universal | 𝒰 (já lá) | nada |
| **Teorema da Absorção** (thm:absorcao) | corpo_peano §absorcao | **𝒰** (lei da régua/observador) | mover; Peano cita e instancia |
| **Energia + Parseval multidim** (def:energia, thm:parseval-multi) | corpo_peano §energia | **𝒰** | mover; Peano cita |
| **Observador/escada/teto-oito/fusão/fibra** (thm:escada…thm:divisao-fibra) | corpo_peano §observador | **𝒰** (hoje o Universal cita a instância; após o corte, enuncia) | mover; Peano instancia |
| ponte universal↔metálica (thm:ponte-universal) | corpo_peano §ponte | **decisão da mesa** (é a ponte 𝒰↔realização; pode ficar como instância exemplar) | debater |
| torre, borda, π_k, Maestro, Metrónomo, Morfológico (W,H) | corpo_peano | Peano (realização) | ficam |
| música/partitura/naipes/histerese/controle/rede dual | corpo_peano | Peano (realização) | ficam |
| 8 leis, teorema central, tecidos, viveiro, cosmologia | corpo-estelar | fundação | ficam |

**Símbolos partilhados** (labels citados entre papers — reescrever as
referências no corte): `thm:absorcao`, `thm:parseval-multi`, `thm:escada`,
`thm:teto-oito`, `thm:fusao-mult`, `thm:divisao-fibra`, `prop:massa`
(def:energia cita), `thm:metronomo` (a absorção cita), `def:inducao`.

**Medidores**: nenhum move de sítio (tests/ é neutro — mede os dois lados).
O que muda é *quem cita* cada medidor; o diff da contagem por documento tem
de fechar (regra da casa).

## 3. O adaptador de equivalência — 𝒰[σ_Peano] ≅ 𝒫

A assinatura Peano σ = (bytes UTF-8; endereço = id; anel ℤ_65537; raiz 3^256;
transições = pares consecutivos). O adaptador atesta **operacionalmente**
(não por alegação): sobre os MESMOS objetos reais,

    E_U = E_P · Φ_U = Φ_P · Φ2_U = Φ2_P · R_end,U = R_end,P · D_U = D_P

Medidor: `tests/equivalencia_universal.js` — o lado 𝒰 implementado genérico
(parametrizado pela assinatura), o lado 𝒫 na forma embutida dos medidores da
instância (cristal_volta/cristal_energia/residuos_totais), fixtures reais
(o cristal e o corpus banal). Critério: igualdade EXATA, componente a
componente. Se divergir um bit, o adaptador está errado — não o Peano.

## 4. O protocolo do corte (quando a mesa mandar)

1. `tools/bateria.sh` completa → totals ANTES (atestados guardados).
2. Contagem de medidores citados POR DOCUMENTO → gravar.
3. Mover as secções da tabela §2 (só as marcadas «mover»); reescrever as
   referências cruzadas; Peano ganha as citações.
4. `pdflatex` dos dois papers (2 passagens cada).
5. `tools/bateria.sh` completa → totals DEPOIS; **diff = 0 exigido**.
6. Contagem por documento DEPOIS; **nenhum medidor pode sair em silêncio**.
7. `tests/equivalencia_universal.js` verde antes E depois.
8. Só então declarar: *Peano é realização do Universal* — como propriedade
   verificável, não intenção.

## 5. Baseline (ANTES) — registado em 13/08/2026

Suite do Universal/cristal (js): cristal_volta 16:0 · cristal_energia 10:0 ·
assinatura_banal 7:0 · assinatura_colisoes 7:0 · observador_torre 10:0 ·
ponte_universal_metalica 12:0 · dirac_transicao 13:0 · clifford_dual 14:0 ·
morfologia_universal 14:0 · residuos_totais 9:0 · rede_dual 28:0 ·
backends_wasm 28:0 · lyapunov_measure exit 0.

Âncoras C dos domínios: carnot 5 ok · topologia 6 ok · cosmologia 6 ok ·
sombra_cone 5 ok · toolkit 8 ok · morfico_ordem 3 ok · universal 42 ok.

Bateria oficial (`tools/bateria.sh`, os três documentos): primeira corrida
416 medidores, 408 verdes, 8 falhas — **todas investigadas até a causa**:

- **6 js com «Wasm Out of memory» — ambiental do harness**: o V8 reserva
  ~10 GB de espaço VIRTUAL por memória wasm (guard regions, sem tocar na
  residente) e o `ulimit -v` de 8 GB matava qualquer medidor wasm.
  Correção de manutenção: limite virtual dos .js subiu (a proteção que se
  quer é sobre a residente; o timeout continua). Re-atestados.
- **`tests/corpo_disco.js` — regressão PRÉ-EXISTENTE (não desta sessão)**:
  `MAX_FICH=64` em `tools/libc.c` e o manifesto tem 69 ficheiros desde que
  «publica todos os papers» (11/08, 35→69) o encheu; `poe_ficheiro`
  devolvia 0 no 65º (partitura.tex). Bisseção provou: falha também com o
  corpo.json anterior ao pipeline. Correção: `MAX_FICH 64→128` (o gesto do
  «alarga o slot»), `tex.wasm` re-subido, corpo_disco 6:0 verde; todos os
  medidores que carregam tex.wasm re-atestados por honestidade (a
  assinatura do atestado é da fonte, não do asset).
- **`banco/fala.c` exit 124 — pré-existente, de classificação**: é a
  ANTENA (daemon TFAL que escuta em 47314); sem argumentos bloqueia até o
  timeout da bateria. Um daemon não é medidor auto-terminante — precisa de
  um modo de auto-teste ou de uma entrada em `args()`. **Decisão do dono.**

**Total final após as correções: 420 medidores — 418 verdes, 2 falhas,
ambas pré-existentes e com causa nomeada** (o daemon fala.c; o tex.c nativo
com §X9/§X16 a falhar). Sobre o tex: a bisseção fechou o quadro — o
pré-0aa6ace passava as MESMAS 62 unidades sem sequer ter INTERFACE_N/LADO_N;
a maquinaria da interface criada em 0aa6ace mudou o comportamento do
esp_gira e §X9 (ida-e-volta do giro) e §X16 (dilatação da fronteira)
quebraram; ninguém viu porque o nativo deixou de compilar E o wasm não
exporta a suíte §X (só compila/volta). Não é «o wasm é canónico e está
verde» — o wasm está verde em OUTRAS suítes; §X9/§X16 estão por testar em
qualquer lado desde 0aa6ace. Se a expectativa envelheceu com o desenho novo
ou se o giro regrediu: **decisão do dono** (é a maquinaria da partitura).

**RESOLVIDO (ordem do coordenador: «avança com a reversão da partitura»)**:
o giro tinha regredido — o desvio `if(LADO_N) return esp_sobe_torre(...)`
no `sobe_exp_m` trocava a soma COM SINAIS pela torre toda-positiva e a
inversa morria. A reversão foi cirúrgica: o giro soma sempre com sinais
(`esp_sobe`); o lado Gentil continua a entrar pelo PASSO (`esp_passo_nv`);
para torres todas-positivas as duas somas coincidem, logo a indução T+T*
nada perdeu. tex nativo 62:0; as dez suítes wasm todas verdes; bateria
**421 — 420 verdes, 1 falha** (só o daemon fala.c). Os 4 medidores novos da sessão (residuos_totais,
cristal_adversarial, cristal_front, equivalencia_universal) entraram na
varredura ao serem citados no corpo_universal (416→420). Nota do harness:
tools/*.js nunca entram na varredura (só tests|banco) — o
tools/lyapunov_measure.js fica fora por desenho do scan; decidir no corte
se muda de pasta ou se o scan alarga.

Adaptador de equivalência: `tests/equivalencia_universal.js` **7:0** —
E, (Φ,Φ₂), R_endereço, D e o espectro idênticos por 𝒰[σ_Peano] e 𝒫 sobre
os objetos reais (E=38.731.623.179; espectro 37.222; R induzido igual).

## 6. O CORTE — executado em 13/08/2026 («segue» do coordenador)

Protocolo cumprido passo a passo:

1. Bateria ANTES: 420 — 418 verdes, 2 falhas nomeadas. ✓
2. Contagem por documento ANTES: Peano 26 · Universal 20 · união 38. ✓
3. Movidas as três secções marcadas «mover» (Absorção; Energia/Parseval;
   Observador/escada/teto-oito/iso-dual/fusão/fibra) do corpo_peano para o
   corpo_universal; referências cruzadas reescritas (labels de instância →
   citações textuais); o Peano ganhou a nota de instância
   (§sec:instancia-universal); a ponte universal↔metálica FICOU no Peano
   como instância exemplar (a mesa não mandou movê-la); a menção da
   Arquitetura mudou de casa junto. ✓
4. pdflatex: corpo_peano, corpo_universal e arquitetura — 2 passagens cada,
   zero referências órfãs (varredura \ref×\label nos dois). ✓
5. Bateria DEPOIS: 420 — 418 verdes, 2 falhas — **idêntica**. ✓
6. Contagem DEPOIS: Peano 21 · Universal 20 · **união 38 = 38, diff vazio —
   ninguém saiu em silêncio**. ✓
7. `tests/equivalencia_universal.js`: **7:0 antes E depois**. ✓
8. Logo, declara-se — como propriedade verificável, não intenção:

   **O CORPO DE PEANO É REALIZAÇÃO DO CORPO UNIVERSAL.**

   𝒰 é a infraestrutura (operações, observador, morfologia, membrana,
   Inversor total); o Peano é 𝒰[σ_Peano] — e a bateria não mudou um bit.

**Estado: CORTE EXECUTADO. Peano ≺ Universal como propriedade medida.**

## 7. FASE 2 — a infraestrutura (executada em 14/08/2026)

Ordem da mesa (eval 14/08): gerente **autoriza** «em modo cirúrgico:
inventário → adaptador → equivalência → regressão → remoção de duplicatas.
Nenhuma lei nova»; diretor sela a **Regra de Ouro da Equivalência**: não
basta a bateria passar — os estados internos (E, Φ, Φ₂, R_D) e as decisões
têm de ser idênticos POR CASO (`R_antigo == R_universal_peano`).

Princípio: **«Universal não é mais um paper. É a infraestrutura.»**

- **Freeze**: base S₀ = `1f5c38b` (bateria de referência 426:426).
- **A única implementação**: `lib/universal.js` — 𝒰 agnóstico,
  parametrizado por σ = (anel, leitura, endereço); operadores: escada
  (E,Φ,Φ₂), energia, R_endereço, transições da membrana, residuoTotal +
  retain, funde/esqueleto/fibra, monodromia, contorno, e `mat2` (J,
  espelho, A_m, W, estaca, cartas). Proveniência linha a linha dos
  medidores atestados (semântica byte a byte, zero teoria nova).
  **Agnosticismo MEDIDO**: a fonte não contém a palavra da instância
  (§M0 falha se alguém a escrever).
- **A realização**: `lib/peano.js` — σ_Peano (ℤ_65537, bytes UTF-8,
  endereço = id JSON com sentinela por posição). 𝒫 = 𝒰[σ_Peano].
- **Dupla árvore**: nenhum medidor antigo foi apagado; as formas embutidas
  continuam nos medidores da instância. `tests/equivalencia_universal.js`
  foi RELIGADO à lib (o lado 𝒰 embutido subiu; 7:0 igual antes e depois).
- **O teste decisivo**: `tests/migracao_universal.js` **10:0** — por caso:
  escada idêntica nos 4274 textos reais (4234 cristal + 40 banal); energia
  total exata; R_endereço igual sob as 7 induções de cristal_volta
  (0,0,1,1,1,1,2 dos dois lados); vetor total E DECISÕES iguais nas 5
  classes de residuos_totais — espelhado orgânico (1,0,0,62937,1) com E e
  Φ cegos e Φ₂/R_D a acusar DOS DOIS LADOS; fibra/volta/monodromia byte a
  byte nas 52 fusões; geometria entrada a entrada (m=0..8, grelha −3..3).

**Estado: EQUIVALÊNCIA PROVADA POR CASO. A limpeza (remoção das formas
embutidas nos medidores antigos) aguarda ordem da mesa — dupla árvore de
pé, como o protocolo manda.**

## 8. A LIMPEZA — executada em 14/08/2026 (autorizada pelo diretor)

Formas embutidas removidas (agora importam a lib): cristal_volta (idDe,
residuoV2), cristal_energia (energia), residuos_totais (assinatura,
transicoes, residuoTotal, retain), fusao_conceitos (E, I3, funde, esqueleto,
fibra, rEnd), cristal_curadoria (E, esqueleto, fibra), assinatura_banal (I),
contorno_riemann (matrizes 2×2, E, corteCego, partes, funde, ν, contorno —
a lib ganhou `corte` para preservar os dois caminhos do §C1).

Deliberadamente NÃO tocados (não são duplicatas):
- assinatura_colisoes: lê UTF-16 (`charCodeAt & 0xFFFF`) — régua própria da
  caçada, semântica DIFERENTE da leitura por bytes;
- lyapunov_torre: idDe por regex e rDual sem contagem de duplicados — réguas
  variantes do estudo de estresse (o v1/posição é contraste POR DESENHO);
- observador_torre: energia sobre VETORES inteiros (outro domínio);
- equivalencia_universal e migracao_universal: guardam as cópias antigas POR
  DESENHO — são as testemunhas da equivalência.

Verificação: cada medidor re-corrido após a cirurgia; bateria completa
**427:427, zero falhas** (mesmos totais de antes da limpeza).

REGRA NOVA DA CASA: quando lib/universal.js ou lib/peano.js mudam, os
medidores que os importam têm de ser re-atestados (`--reatesta`) mesmo sem
mudança na própria fonte — a atestação assina o medidor, não a lib.

## 9. FASE 3 — as leis promovidas (executada em 14/08/2026, noite)

Ordem da mesa: gerente **AUTORIZA** («promover as 8 leis e as primitivas já
verificadas para o Universal; Peano passa a ser instância; zero teoria nova»);
diretor sela. A REGRA DE OURO entra no protocolo:

**O Universal é dono da lei; as instâncias são donas apenas da realização.**

- **As 8 leis em `lib/universal.js`** como interface normativa: catálogo
  `leis` com verificação OPERACIONAL por lei (identidades já atestadas pelos
  medidores — nada de teoria nova): Lei 0 pela enumeração da curva (x=±1);
  Lei 1 pela estaca A·(mI−A)=−I; Lei 2 pelo par rotor/espelho (RJ=−JR);
  Lei 3 pelo trial x³=x; Lei 4 por |det|=1 e tr W=0; Lei 5 pelo bit como
  ponto fixo de ν; Lei 6 por lcm(2,3)=6; Lei 7 pelo det multiplicativo nas
  duas cartas. `verificaLeis()` → 8/8 (migracao_universal §M7).
- **Primitivas dinâmicas promovidas**: `anel(q)`, `dft`/`idft`,
  `renormaliza` (t↦t²−2d — o OPERADOR DE LEITURA das leis, não lei nova),
  `morfo` (δ/ε em ℤ, sem truncar — a lição da borda codificada).
- **Substituição (não reescrita)**: toro_histerese (morfologia),
  metronomo_fourier (anel + dft/idft), metronomo_autossimilar
  (renormaliza), lebesgue_toro (anel + dft) religados à lib; totais
  idênticos (16/15/12/7; zeta_universal 10 intacto — a sua fábrica é de
  MATRIZES mod q, régua própria, fica).
- **Testemunha ampliada**: migracao_universal §M8 compara cada primitiva
  nova com a forma embutida antiga, caso a caso (DFT coeficiente a
  coeficiente; renormaliza vs quadraturas BigInt; morfologia elemento a
  elemento).
- **Inventário Hausdorff (ordem do coordenador)**: a régua JÁ EXISTE na
  casa — tests/dourada.c (dimensão complexa do Cantor do ouro: «Hausdorff é
  a parte REAL, a oscilação de período ln φ é a parte IMAGINÁRIA»);
  cristal: dimensao_de_hausdorff (ciencias), Cantor-sem-11 dim
  ln2/lnφ²≈0,694 (papers), energia_fractal_cantor com Bowen-Ruelle P(d)=0
  (fractal); tools/ciencia.sh («a medida é escolhida»); deforma.c (0,87 da
  literatura, NÃO medido). A ponte espectro↔dimensão fica NOMEADA como
  próxima pergunta no thm:renormalizacao — não afirmada.

**Estado: FASE 3 EXECUTADA. Universal dono das leis; Peano e demais corpos,
donos da realização. Pendente de decisão da mesa: nada.**
