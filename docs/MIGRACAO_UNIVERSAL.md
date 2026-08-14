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

## 10. A AUDITORIA DA REGRESSÃO TOTAL (14/08/2026, madrugada)

Ordem da mesa: «regressão completa caso a caso; o critério não é escore
global». Executado: `bateria.sh --refaz` (todas as 433 sementes reabertas,
tabela nunca truncada) + diff contra a fotografia dos atestados.

**O ACHADO: 28 medidores caíram com assinatura de fonte INALTERADA** — o
«medidor que nunca mediu» em escala: a atestação assina a fonte do medidor,
e o MUNDO por baixo (libc, fixtures, papers, memória wasm) mudou sem os
re-correr. A regressão total existe exatamente para isto.

### Classe A — fixtures externas ausentes (10): «NÃO MEDIU», honestos
antissim, campomedio, cifrando, dualcifra, entrega, folhas, liquida_doador,
protocolo, recupera, transfusao_real — todos pedem o DOADOR (Ollama acordado
+ colheitas: interroga.sh, colhe_transfusao.sh, antissimetrica.sh…). O
sistema é AUTO-CONTIDO desde 05/08 (o Ollama saiu da órbita): estes
medidores só medem com o doador presente. Estado: vermelhos com causa.
**Proposta ao dono**: colher uma vez com o doador e VERSIONAR os vetores
como fixtures do repo (medir contra fixture, não contra o daemon) — ou
reclassificar como bancada-com-doador fora da bateria de sempre.

### Classe B — pipeline compositor/fontes/app (14): regressões reais acumuladas
spline (8 falhas: leitura de contornos TTF), dual_spline_ttf (4: glyf×hmtx),
biblioteca (2: cobertura do compositor), volta_estrela (1: resíduo por
glifo), fonte_banco (1: assinatura do LATEX no banco), avalia_macros (3:
\gkcapa na página 1), design_no_pdf (1), escala (1), escala_dourada (1:
salto do título), dois_streams (4), app_arranca (2), sem_chute (5),
compoe_ao_clicar, fator (2: dobras da assistente). Causas prováveis por
família: fontes/TTF (mudança na fonte ou no leitor), compositor tex.c
(papers cresceram; capa mudou), app (bundle). **Pendente: uma sessão de
reparo por família, um medidor de cada vez.**

### Classe C — causa imediata (4): CONSERTADOS nesta auditoria
- **refs** ✓ 0: 2 órfãs + 7 \ref CRUZADAS no catalogo.tex (labels vivem na
  teoria.tex — o próprio §R3 do medidor as proíbe) → todas textuais.
- **morfico** ✓ 0: `formalizador` mudou para tools/ numa reorganização;
  sys.path aponta lá.
- **tres_reconstroi** ✓ 0: a fixture sandbox/tiffany.tex sumiu; a prosa
  agora é ../teoria.tex (cwd dos medidores = tests/).
- **libc_wasm** ▲ de CRASH para 2 falhas NOMEADAS: (a) o Node exige o
  objeto de imports (env.__fich_miss, do trabalho MAX_FICH) — stubs
  nomeados; (b) o offset mágico end_saida()+7M caía fora das 2 páginas da
  dieta «sem memória» → vfs_reserva. O que RESTA é real e novo:
  §L4b o write devolve vazio (candidato: caminho de escrita pós-dieta);
  §L5 o traduz acusa «pilha vazia na descida» ao redescer a libc inteira
  (candidato: construções novas do MAX_FICH que a descida não cobre).

**Totais**: antes da auditoria 433:433 (com 28 atestados envelhecidos);
depois do refaz 405 verdes/28; depois dos consertos **408 verdes / 25
vermelhos com causa nomeada** (Classe A 10 + Classe B 14 + libc_wasm). A bateria agora diz a VERDADE.

REGRA REFORÇADA: a regressão total (`--refaz`) entra no protocolo de fecho
de qualquer fase — a atestação por assinatura não vê o mundo a mudar.

## 11. O SANEAMENTO DO DOADOR E O REPARO DAS FONTES (14/08, continuação)

Decisão do coordenador («podemos tirar o conceito de doador e sanear o
sistema»), protocolo do gerente (o crivo derivável/fixture/integração/
remover), selo do diretor (a Regra do Oráculo; teoria CONGELADA até verde).

- **Classe A resolvida — integração declarada**: os 10 da crónica do doador
  + compoe_ao_clicar (servidor vite) moveram para `integration/` com
  LEIA.md (estatuto, achados históricos, o que cada um precisa). Nenhum
  removido; nenhum finge medir. Citações no catálogo atualizadas (saem da
  bateria DECLARADAMENTE — ninguém sai em silêncio). Gémeos internos das
  operações já na bateria: transfusao.c, plugue, fala.c -teste.
- **Família das fontes CURADA na raiz**: a fonte do repo
  (lib/fontes/documento-*.otf) é CFF — SEM glyf/loca — e `ttf_contorno`
  lia offsets inexistentes como lixo silencioso (loca negativa, 0
  contornos). O conserto no sítio: DESPACHO em ttf_contorno para o
  cff_contorno que a lib já tinha («TTF e OTF são a mesma spline»,
  honrado no código). E os medidores-oráculo voltaram ao seu oráculo:
  spline.c e dual_spline_ttf.c abrem a LIBERATION (o §P2 compara contra a
  tabela Helvetica do tex.c; o xMin do glyf só existe em TrueType) — a
  lista SPLINE_REG da lib serve o COMPOSITOR, não estes medidores.
  spline 0 ✓ · dual_spline_ttf 0 ✓.
- **O que resta (12)**: a família compositor-texto — o catálogo COMPÕE
  (1622 réguas, cores do estilo confirmadas, controlo por mutação passa)
  mas a extração de TEXTO dá 0 palavras/1 página (design_no_pdf §G3,
  dois_streams, sem_chute, escala, escala_dourada, avalia_macros,
  volta_estrela, fonte_banco, biblioteca, fator, app_arranca). Candidato
  único: a forma do TEXTO no PDF mudou (glifos como desenho/XObjects com
  as fontes do repo) e os leitores dos medidores não a reconhecem — OU o
  compositor perde o texto de facto. SESSÃO PRÓPRIA. E libc_wasm: o write
  devolve vazio; o traduz «pilha vazia na descida» na libc inteira.

**Estado: bateria honesta 422 medidores — 410 verdes, 12 com causa nomeada.**
Teoria (Viviani → Lei trial → intervalos encaixantes → Clifford) congelada
até o verde pleno, por ordem da mesa.

## 12. O REPARO DA CLASSE B (14/08, madrugada 2 — a sessão do compositor)

Ordem da mesa: «ataquem exclusivamente os 12 vermelhos; proibido usar teoria
como curativo». Executado — 10 de 12 verdes, 2 vermelhos HONESTOS a apontar
defeitos reais:

**A causa-mãe da família texto**: o dialecto da casa desenha glifos
(`q s 0 0 s x y cm /Gf_c Do Q` — o código do carácter no nome; sem Tj/Tf/BT)
e o pdftotext lê ZERO. Nasceu o leitor partilhado `tests/pdf_casa_texto.js`
(objetos, páginas, glifos com x/y/s/f/c, texto por linhas, letras longas —
uma leitura, muitos usos). Consertados com ele:
- design_no_pdf ✓ (40.611 sequências longas no catálogo)
- dois_streams ✓ (glifos por Do; fixture própria com tcolorbox — o
  corpo-estelar perdeu as caixas DE PROPÓSITO: «só o tcolorbox tem caixa»)
- avalia_macros ✓ (página 1 sem espaços; capítulos 148=148 contra o
  pdflatex com oráculo AUTO-CONSTRUÍDO; degraus pela escala do cm;
  mutação filtrada às palavras EXCLUSIVAS do estilo)
- escala ✓ (corpos exatos da escala do cm; degraus verdadeiros + normalsize
  da CLASSE lido de size11.clo/latex.ltx + derivados por produto cruzado)
- volta_estrela ✓ (mutação de UM glifo /G0_82→88; âncora do §V4 no fim
  REAL da função)
- escala_dourada ✓ (a escada ficou contígua por decisão — o salto conta-se,
  não se exige)
- fonte_banco ✓ (o /tmp/cards_banco é partilhado por 21 medidores e
  abrir(...,1) TRUNCA — a âncora passou à FONTE do cards.c + volta própria)
- fator ✓ (o conversa mudou para banco/bin numa reorganização)
- app_arranca ✓ (TextEncoder/TextDecoder do canvas na sandbox)
- biblioteca ✓ (a cisão tex.c+tex_core.c — lê o PAR; §Y3 atualizado: o
  buraco de 2026-08 nomeou a operação, a operação chegou)
- sem_chute: N1–N6 ✓ (Symbol identificada PELA FORMA do σ com sonda;
  justificação pela moda das bordas; avanços pela moda do documento;
  mutação WinAnsi no tex_core: 59.024 chutes sem a conversão)

**OS DOIS VERMELHOS HONESTOS (defeitos reais, nomeados)**:
1. **sem_chute §N7 — DUPLO-DESENHO no compositor**: nas páginas 422/423/425/
   434 do catálogo (cluster nas molduras multi-linha), fragmentos pós-hífen
   («as-sim» → «sim») desenhados DUAS vezes no mesmo sítio (93 glifos;
   «ssiimm» à vista). O medidor está certo; o compositor tem o bug.
   ALVO DA PRÓXIMA SESSÃO: quebra_e_desenrola × pintor BX_.
2. **libc_wasm** — write devolve vazio; traduz «pilha vazia na descida» na
   libc inteira (herança MAX_FICH). Já nomeados no §10.

**Bateria: 422 medidores — 420 verdes, 2 vermelhos com defeito real
apontado.** (Os 11 de integração fora da bateria, por estatuto.)

## 13. O DUPLO-DESENHO RESOLVIDO NA RAIZ (14/08, madrugada 3 — martelo)

Ordem da mesa: «não se tapa duplo-desenho com regra de supressão;
corrige-se a fonte da emissão». Executado por caso mínimo + bissecção:

- **Reprodução mínima**: `\[ \boxed{\;\begin{aligned}…\end{aligned}\;} \]`
  seguido de QUALQUER tabela ⇒ células umas sobre as outras. Bissecção no
  catálogo apontou as linhas 20970–20985 (o boxed+aligned do «par n,m»).
- **A raiz**: o despacho de ambientes casava `aligned` pelo PREFIXO
  «align» e tratava-o como porta de display — o `\begin{aligned}` fazia
  `centra_mat = CENTRA` (guardava 1 por cima do 0 do pai `\[`) e o
  `\end{aligned}` DESLIGAVA o modo do pai. Dali em diante o CENTRA ficava
  preso: TUDO centrava — prosa no meio da página e as células das tabelas
  no mesmo x (o «sim»+«sim» = 93 glifos duplos, págs 422/423/425/434).
- **O conserto (tex_core.c)**: `aligned`/`gathered` são SUB-ambientes
  dentro da matemática — só consomem a chave e ficam no modo do pai.
  Caso mínimo 0 duplos; **catálogo inteiro 0 duplos**; tex.wasm
  reconstruído; sem_chute **VERDE** (§N7 resíduo 0 nos três documentos).

## 14. A LIBC: UM LADO CURADO, O OUTRO NOMEADO ATÉ AO OSSO

- **§L4b CURADO em dois passos**: (a) `tam_saida()` devolvia só PDF_N — o
  par PDF_N/SAIDA_N nasceu na dieta e o write direto enchia a SAIDA com o
  contador a devolver 0; agora `PDF_N>0 ? PDF_N : SAIDA_N`. (b) o medidor
  lia com o ponteiro VELHO da SAIDA (o realloc move-a) — lê-se fresco.
  O fwrite/fputc/fseek/rewind fecham; falta só o fprintf (abaixo).
- **§L5 e o fprintf — A MESMA RAIZ, nomeada**: a descida do traduz morre
  «pilha vazia» DENTRO do sscanf (fn=29, op=0x10 call, MP=3899; anel de
  16 opcodes gravado no erro — instrumentação que FICA no traduz).
  Causa: o `va_arg` do subida IÇA bytes de código para o princípio da
  frase (memmove no COD) e a leitura LINEAR da descida fica com a pilha
  de expressões desfasada. O fprintf mudo do §L4b é o mesmo território
  (varargs pela fita). **Sessão própria de compilador: ensinar a descida
  a desfazer o içamento (ou marcar a fita na descida).**

**Bateria: 422 — 421 verdes, 1 vermelho (libc_wasm) com a causa reduzida a
UM mecanismo do traduz.** O portão de ouro (422:422 + --refaz) espera essa
sessão; o compositor já fala a verdade pura.

## 15. O PORTÃO DE OURO: 422:422 — E A REDE CONTRA O VERDE FALSO

A sessão de compilador previu «desarmar o içamento do va_arg» — e o
diagnóstico anterior estava ERRADO nas duas frentes. Nenhuma das causas
era a lógica do va_arg:

- **§L5 (round-trip 14551 bytes)**: DUAS raízes no traduz. (a) A descida
  IGNORAVA a secção de imports do wasm — `call` e exports indexam
  contando os imports, mas ASS[]/NOMES[] só tinham as definidas: o `f`
  errado tirava o npar errado e a pilha esvaziava. Agora a secção 2 é
  lida (DESCE_NIMP/DESCE_IMP_ASS/DESCE_IMP_NOME) e os índices descontam.
  (b) A captura de directivas apanhava `#define MAX_FICH 128 /* o
  manifesto…` — um `/*` SEM fecho na mesma linha — e no replay da config
  esse abridor PENDURADO engolia as directivas seguintes na recaptura
  (as caudas divergiam: A=0, B=1 em `#define MAX_AGULHA`). A directiva
  corta-se no abridor; o comentário viaja INTEIRO pela passagem 2.
  **sobe(desce(M)) = M, resíduo 0 nos 29717 bytes da libc.**
- **§L4b (fprintf «mudo»)**: a libc NUNCA esteve muda. A vista
  `Uint8Array` do medidor ficava DESTACADA quando o `memory.grow` do
  realloc da SAIDA corria — o `poe` do formato escrevia numa vista morta,
  SEM erro, e o vsnprintf via string vazia. A vista pede-se FRESCA a cada
  uso, para ler E para escrever. Sai `"BT 42 Tf 13.600 Td\n"`, byte a byte.

E o refaz do portão (422:422 nos exits) denunciou na linha das unidades
(«2 falharam») um FALSO VERDE ESTRUTURAL:

- **cards.c saía 0 com duas #UNIT falha**: um `long falhas = 0` LOCAL no
  main sombreava o contador de lib/unidade.h — o ok() somava no do
  header, o return devolvia o local. E as unidades falhavam DE VERDADE:
  o pino do manifesto em 105 com o mundo em 111 (docs 7→12: papers com
  capa, Partitura, Corpo de Peano; o autor com nome), e o §B10 a assumir
  um /tmp/render_bin que NINGUÉM construía. Pino atualizado com a
  história, o bloco compila o renderizador ele próprio (8 assinaturas
  lidas, 0 divergem — o circuito fecha de verdade), shadow removido.
  Mutação de controlo: pino errado → exit 1.
- **A classe varrida**: mais 16 medidores com o MESMO shadow, 3 deles com
  `return 0` fixo (einstein, vestir, renormaliza — o exit nunca poderia
  falhar). Todos consertados.
- **A REDE (bateria.sh, nos 3 ramos)**: exit e unidades TÊM DE CONCORDAR
  — exit 0 com `#UNIT falha` vira «FALHA: VERDE FALSO», e a atestação é
  corrigida para 9 NA HORA (senão o reuso ressuscitava o verde — a lição
  do medidor que nunca mediu). Nenhum medidor futuro precisa de ser
  confiado: os dois caminhos comparam-se no runner.

**Bateria: 422:422, 0 falhas, unidades 118:0 nos 17 reabertos. O portão
de ouro está fechado; a fila teórica reabre (Viviani → Lei trial →
intervalos encaixantes → teorema dos resíduos → Clifford).**

## 16. FASE 4 — O NÚCLEO UNIFICADO (ordem da mesa, 14/08 noite)

O coordenador: «migrar para sistema unificado». O gerente autorizou com
o contrato: **o Universal é dono da ESTRUTURA, cada domínio é dono da
realização** — «unifica sem apagar as diferenças».

- **lib/universal.js exporta `nucleo`** = (X, S, H, J) com as cinco
  relações do Teorema Universal e `verifica()` operacional. X e H são
  DERIVADOS de espelho/J no próprio módulo (a referência não é cópia).
  A mudança na lib foi puramente aditiva (diff: só o export cresceu).
- **A árvore dual (nucleo_unificado.js 11:0)**: cada objeto pelos DOIS
  caminhos — legado local vs núcleo — igualdade entrada a entrada:
  §N0 origem (X,H == construção local), §N1 álgebra (⊗I₄ reproduz os
  blocos de clifford_pleno byte a byte, relações com as mesmas
  constantes; ⊗espelho é o gume), §N2 geometria/espectro (H nas folhas
  = soma/diferença; Cooley–Tukey através do núcleo), §N3 Peano
  (ν∘ν=id, fibra verbatim, RETAIN/REOPEN pelo vetor completo),
  §N4 leis 8/8 + contrato juntos.
- **Legados NÃO apagados** (regra do gerente): a limpeza dos duplicados
  é fase posterior, depois do portão --refaz validar a torre inteira.

## 17. O PROTOCOLO NORMATIVO DA MEDIÇÃO — regra permanente (14/08, mesa)

**O contrato** (lib/universal.js, `medicao` — meta-medido por
tests/medicao_normativa.js):

    𝓜(O) = (R, G, V)      fecha(𝓜) ⟺ R = 0  ∧  G  ∧  V = 0

- **R** — o invariante: resíduo da afirmação;
- **G** — o GUME: o contra-caso construído para falhar, e que FALHOU
  como previsto (`true` estrito; truthy desleixado é recusado);
- **V** — a volta: resíduo da reconstrução exata.

Critério mais forte que «deu zero»: *um teste que só sabe passar não
atesta* — a forma do falso verde de 14/08 (R=0 sem gume) é recusada
pelo próprio contrato. Todo medidor novo importa o núcleo e declara o
gume.

**A lei arquitetural do --refaz** (elevada de procedimento a portão):

    mudança estrutural OU teorema novo  ⟹  --refaz total obrigatório

Todo teorema entra pelo mesmo portão pelo qual a arquitetura entra.

**A simetria de desenho**: o produto dual exige o espelho no índice
(k↔−k) para não perder informação; a medição exige o gume
(afirmação↔contra-caso) para não perder falsificabilidade.

## 18. O SELO — o fim das migrações (14/08 noite, decreto do diretor)

A ordem «formalizar o corpo e migrar o sistema com base nas operações»
resolveu-se SEM fase 7: a formalização é o Teorema de Estrutura
(thm:estrutura, estrutura_corpo.js 7:0) e a migração já estava feita —
as seis fases eram a implementação da cadeia
Lei → Operação → Corpo → Ordem → Refinamento → Caminho → Limite.
O manifesto completo: docs/ESTRUTURA_CORPO.md. Congelamento
arquitetural absoluto: nova migração só com inventário de ownership e
ordem explícita do gerente. «Completo» = completude por refinamento
operacional (selo de honestidade protegido por medidor); um real =
um caminho raiz→folha na árvore da torre (thm:real-caminho).
