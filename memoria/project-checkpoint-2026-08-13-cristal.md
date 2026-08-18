---
name: project-checkpoint-2026-08-13-cristal
description: "13/08 tarde — o cristal grande recuperado do broca-so para o tiffany, LaTeX como projeção verificável, volta 9:9 resíduo 0"
metadata: 
  node_type: memory
  type: project
  originSessionId: b6c6c5cb-b5ec-45f0-ac00-480c20a1bb2d
  modified: 2026-08-14T03:49:45.955Z
---

# 13/08 — O CRISTAL RECUPERADO (arqueologia do eval)

**Onde estava**: `broca-so/conversa/dados/conhecimento.graph.jsonl` — jornal de
75.165 registos, **4286 conceitos únicos** (última versão por id), com
proveniência por registo (origem, meta.fonte, meta.dominio, epistemico,
confianca, arestas). O eval palpitava `machine/tecido_prosa` (4,7G) — mas isso
é o tecido linguístico (Tatoeba + psi_cache derivada), NÃO o cristal.
O jornal fica no broca-so intocado.

**A cadeia do gerente (eval.txt)**: localizar → inventariar → proveniência →
extrair → gerar LaTeX → medir resíduos. Regra de ouro: **não converter antes
de conseguir reconstruir** — LaTeX = projeção verificável, não fonte.

**O que entrou no tiffany**:
- `cristal/cristal.jsonl` (4,0M) — fonte canónica; `cristal/LEIA.md`.
- `tools/cristal_extrai.py` — extração + limpeza do IP privado (corpo.sh:
  «IP privado — não publicar»; `aarao@78.46.19.151`→`gex44`, 3 subst., 0 restam).
- `tools/cristal_tex.py` — 10 projeções `papers/cristal_*.tex` (63 domínios →
  10 grupos; ciencias 899, matematica 680, manual 560, computacao 516,
  engenharia 442, papers 427, fisica 332, floxina 187, xadrez 154, diversos 89).
  Cada secção precedida do registo em `%CRISTAL` — o desenho do /Type/FonteTeX.
  Transliteração unicode com fallback CONTADO (9 chars CJK); PUA F8F1-F8F4 são
  chavetas de OCR; sub/sobrescritos consecutivos fundem num só `_{...}` (dois
  seguidos é erro TeX). **Todos os 10 compilam** com pdflatex.
- `tests/cristal_volta.js` — §V0–V4: reconstrução byte a byte R=0, secções==
  registos por dois caminhos, mutação de 1 byte acusa, portão do IP. #TOTAL 9 0.
- **`tools/ingere.py` corrigido**: comentário TeX (% não escapado) já não entra
  na resposta — 153/154 respostas do xadrez estavam a engolir o %CRISTAL da
  secção seguinte; teoria.tex deu 0 linhas mudadas (só remove ruído).

**Limitação conhecida**: ingere.py trunca títulos com math aninhada no
primeiro `}` (pré-existente). PDFs não vão ao git (gitignore já cobre).

**RONDA 2 (ordem de marcha do gerente+diretor, eval): TUDO FECHOU — commit
`c0893a5`.** (1) Claim CristalVolta formalizado: `conecthus/claims/cristal.claim`,
`exec_cristal` em execute.c (claim_ir §C10, 65:0), **wasm id 15** (backends_wasm
28:0); 0∧0 não fecha, réu não fecha. (2) Mutações com REOPEN à vista no
cristal_volta.js (13:0): apagar conceito R=1, trocar resposta R=1, alterar
categoria R=1, corromper projeção R=1641 — todas REOPEN; **reordenar sobrevive
POR DESENHO** (ordem canónica derivada do id; precedente «mutação equivalente»).
(3) História em `cristal/historia.tsv` (soma 75165 = jornal; floxina 2908) e
«história N versões» na proveniência visível. (4) `tools/cristal.sh` ingere
falas + 4236/4286 conceitos no banco em 1,4s (50 caem nos filtros de tamanho do
ingere, contados); corpus.sh chama-o. A Tiffany já consulta: «reticulado»
devolve conceito+proveniência+história pela porta conversa.

**Pendente**: push (do dono); chip do cristal na UI da assistente (passo 6
pleno); rebuild de claim.wasm foi feito por tools/sobe_backends_wasm.sh.

**RONDA 3 (ordem do coordenador: Lyapunov dual, teto dimensional via TCL) —
commit `3ef9bb2`.** `tools/lyapunov_measure.js`: inteiro puro, LCG, UMA
perturbação DUAS leituras (a lição «duas réguas para o MESMO objecto» aplicada
ao desenho), torre por dobra 134→4286, e = R−k. **Medido**: endereço preservado
→ e=0 EXATO nas duas réguas em toda a torre (λ⁺+λ⁻=0); corromper na régua de
ORDEM diverge ~n² (σ² ×1200 quando n×32, n²=1024; média e≈n/2 — posição
uniforme); corromper na régua DUAL → e=1 constante (faltante+excedente), σ²=0;
lote de 8 → e NEGATIVO por colisão: flip∘flip=id, a Lei 1 medida DENTRO do
estresse, ~C(8,2)/n → 0. **Veredito: o teto EXISTE na medida dual — caso
positivo, a teoria NÃO precisa de expansão, precisa da régua certa** (a ordem é
derivada do id; a régua não transporta). Proposta parada à espera do dono:
medidor v2 com régua dual no residuo (núcleo c0893a5 congelado por ordem do
gerente/diretor). Frase do gerente adotada: «o sistema distingue mutações
admissíveis das que violam o invariante do claim».

**RONDA 4 (ordem do coordenador: formalizar e sanear) — commit `9c42020`.**
**Teorema da Absorção** entrou no corpo_peano.tex (§sec:absorcao, thm:absorcao,
após o Controle de Histerese): e=R−k=0 exato pelo ENDEREÇO em toda a torre;
endereço destruído = o PAR faltante⊕excedente (Lei 0), e≡1 σ²=0; por posição
diverge ~n² — o estresse é da RÉGUA. Prop. involucao-ruido (flip∘flip=id no
lote) e corolário no Controle (variante que preserva endereço → RETAIN sem
custo). Na arquitetura.tex: subsecção «A absorção: o endereço decide o custo»
em §reversão, ligada ao thm:assinatura. **Saneamento**: «mutação» fora da prosa
dos dois papers e instrumentos (→ perturbação/variante); ficam SÓ os literais
da IR (mutate/mutation, contrato do parser) e as citações. #MUT→#PERT.
Ambos os papers compilam (2 passagens); 13:0 e números idênticos re-medidos.
**RONDA 5 (ordem do diretor: v2 pelo endereço + matriz na tela) — commit
`4f8d184`.** `cristal_volta.js` v2: residuoV2 indexa EXCLUSIVAMENTE pela chave
de endereço (faltantes+alterados+excedentes+duplicados; duplicação conta cada
cópia a mais); v1 (posição) fica de contraste na matriz. **Matriz 16:0**:
permutação e retoque da projeção ABSORVIDOS (R=0); conteúdo/categoria/remoção/
duplicação R=1 REOPEN; endereço destruído **v1 R=1641 × v2 R=2** (o par, Lei 0)
— o teorema na tela. Triângulo teorema↔medidor↔perturbação concorda; a
quarentena dos .tex cumpriu a própria condição e a prova do thm:absorcao ganhou
só a citação da matriz. Frase do gerente: «a absorção não é tolerar erro — é
ter uma coordenada onde a perturbação se contabiliza e, dualizada, cancela».

**RONDA 6 (ordem do coordenador: indução/meta-indução + energia + Parseval
multidim do central) — commit `0c64ad8`.** corpo_peano §sec:energia:
def:inducao (indução = o passo, Lei 4; meta-indução = a leitura dual que
VALIDA A CONSERVAÇÃO DE ENERGIA), def:energia (E=Σx² = a MASSA da cruz;
Hurwitz N(xy)=N(x)N(y) + |det|=1 + λ⁺+λ⁻=0 são três nomes; Parseval = a
exigência lida na transformada), thm:parseval-multi (derivado do central:
X_k/X_{−k} é o par frente/volta; Σg^{(i−j)k}=Nδ conta; ∫|f|²=∫|F|² integra
via Lebesgue σ-aditiva; eixos por Fubini → fator ∏Nᵢ SÓ da dimensão total).
**tests/cristal_energia.js 10:0**: E=38.731.623.179 exato dos dois lados;
digest AssinaturaOito, 256=256=16·16=4⁴ → espectro 37.222 nos TRÊS caminhos;
raiz errada quebra; matriz por energia: remoção×duplicação ΔE=∓7.433.979 —
o par da Lei 0 com soma zero LIDO NA ENERGIA. Vocabulário: perturbação→
indução (quadro da absorção; #IND); banda de histerese clássica fica.

**RONDA 7 (congelar → UI → caçada adversarial) — commits `e5fa6ed`, PUSH FEITO
(c0893a5→e5fa6ed no origin; portão segredo.sh LIMPO antes).**
Chip **X** (xtal) na assistente: proveniência da RESPOSTA do banco na tela
(origem·domínio·confiança·história) — sem ficheiro novo no manifesto (corpo.json
é MEDIDO); assinatura energia+fase; rejeitaPrimeiroErro. cristal_front 16:0.
**A CAÇADA (cristal_adversarial.js 10:0)**: a maldita EXISTE — 600 encontradas:
transposição interna, reversão de trecho e troca de conteúdo entre endereços
dão ΔE=0 (200/200 cada) com R_end>0 → **conservar energia NÃO basta; o fecho é
R_total=(R_endereço,R_E)=0** (o gerente previu; confirmado). A régua do
coordenador fecha: assinatura (E_a,fase_a) POR ENDEREÇO — fase=Σi·bᵢ apanha o
que a energia não vê — 0 escaparam das 600; primeiro byte errado rejeita cedo
(pos média 351–502). Permutação (0,0) admissível. **Teoria congelada** — o
achado (par de conservações simultâneas) espera a próxima ordem para entrar
nos papers (o gerente: «a dualidade central precisa de mais de uma conservação
simultânea?»).

**RONDA 8 (bateria banal + caçada às colisões) — commit `788fcc9`, push feito.**
Condições de contorno do coordenador tratadas como hipótese medida (gerente/
diretor: fusão em QUARENTENA até haver caso real; clone sob x∼_O y).
**assinatura_banal 4:0** (20 pares orgânicos de conversa.tex: reler conserva,
190 distintos 0 colisões; cafe↔café etc. são corpos distintos — a ADMISSÃO é
do Controle/𝒱, não da régua). **assinatura_colisoes 7:0 — O OURO**: a dupla
transposição espelhada («ab…ba»→«ba…ab» à mesma distância) conserva E e
cancela Φ (ΔΦ=d(a−b)+d(b−a)=0); **4286/4286 registos do cristal admitem a
colisão orgânica** (o par «ar…ra» é ubíquo — até o campo "arestas"); a
cegueira do observador dual é GENÉRICA. **Φ₂=Σi²b separa TODAS** (só falha
com centros iguais); (E,Φ,Φ₂) sem colisão em 30k aleatórias (orçamento dito);
a escada dos momentos satura no espectro completo = o byte a byte do v2.
Clone: I(clone)=I(original), cardinalidade no endereço conta a carga.
Zero prosa .tex — teoria continua congelada; candidata à secção futura:
«duas conservações simultâneas» + a escada (E,Φ,Φ₂,…)→espectro.

**RONDA 9 (o log de Φ₂ no banal) — commit `0b82715`, push feito.** A mesa
teórica foi REABERTA pelo diretor mas atribuída AO GERENTE (escada de
observadores I₁≺I₂≺I₃ + divisão como FIBRA INVERSA F⁻¹(z) + isomorfismo-dual
que preserva I ∧ ordem); a minha tarefa era o verde do terminal.
**assinatura_banal 7:0 com a escada**: os 190 pares orgânicos distintos
separam-se TODOS no degrau I₁ (E) — a energia basta para a diferença natural;
o espelhado adversarial vive no próprio banal (11/20 respostas) e cai SÓ em
Φ₂. A fotografia: degrau grosso para a diferença natural, degrau fino para a
equivalência adversarial. Coordenador trocou clone→DIVISÃO (ver se deriva a
divisão clássica — o gerente: pela fibra inversa, sem declarar antes de
mostrar). Fusão/divisão ainda SEM objeto no banco. Próximo: o gerente traz o
texto da teoria; eu meço/realizo quando vier.

**RONDA 10 (teto pelas 8 leis + fusão→multiplicação + divisão=fibra) — commit
`660e6f4`, push feito.** Mesa aberta pelo diretor; medido ANTES da prosa.
**observador_torre.js 10:0**: Vandermonde no anel (det=Π(xi−xj) sem zero) —
**8 momentos (um por lei) fecham suporte ≤8; Δ⁸ (suporte 9) anula os 8 e
escapa ao linear, a ENERGIA apanha (ΔE=12870)** — fecha em 8, falha em 9, o
gesto do relógio de Hurwitz (analogia dita); **fusão ⊗: E(u⊗v)=E(u)·E(v)
EXATO (N(xy)=N(x)N(y)); divisão = FIBRA: x_i=z_ij/v_j inteira, 8 caminhos
concordantes, E divide — a clássica EMERGE**; fusão ⊕ soma e retração devolve.
corpo_peano §sec:observador: escada I₁≺I₂≺I₃ (estrita e orgânica),
teto-oito, iso-dual (preserva I ∧ ordem; a escada é observador DA ordem),
fusao-mult, divisao-fibra. Fusão de CONCEITOS: quarentena mantida (sem objeto
no banco). Bateria do dia toda verde: 10+7+7+16+10.

**RONDA 11 (a ponte universal↔metálica) — commit `b0f4b53`, push feito.**
O pedido de «visão unificada em todo o repo» (Dirac/Möbius/Pisot/CF/convolução/
termodinâmica…) ficou como MAPA DE DESTINO congelado; a ordem foi UM
experimento: a ponte. **ponte_universal_metalica.js 12:0** (BigInt, sem float):
a transformada universal realizada INTEIRA pela matriz companheira (avaliação
nas folhas; o √N não sobrevive — lição da memória aplicada); conservação POR
PRODUTO N(xy)=N(x)N(y); det(A_m^k)=(−1)^k; inversa inteira nas unidades
(FP=1); adj·M=det·I (a fibra: dividir custa o det); **T⁻¹∘T devolve massa E
ordem** (escada idêntica); **EMERGEM sem forçar: frações contínuas (A_m^k
carrega os convergentes), Möbius (compor matrizes=compor fracionárias), Pisot
inteiro (L²−ΔF²=4(−1)^k)**; NÃO aparecem: Dirac, convolução — registrados.
Anel: Parseval dos F_k fecha (14050). corpo_peano §sec:ponte
(thm:ponte-universal + obs:emerge, cadeia corpo→norma→ordem→observador→
transformação→dual — «hipótese testável por elos, não manifesto»).
Bateria do dia: 12+10+7+7+16+10 verde.

**RONDA 12 (Dirac + corpo_universal.tex) — commit `655de03`, push feito.**
A escolha do coordenador: raiz quadrada («metade para cada lado»).
**dirac_transicao.js 13:0**: a borda x²=mx+1 É a fatoração de Dirac no andar
(D=A_m, D²=mD+I); a raiz do PASSO não existe no 2×2 (busca exaustiva vazia;
det²=−1 — o preço do dual σσ†=−1); existe INTEIRA um andar acima
(D=[[0,I],[A,0]], D²=A⊕A, det=−1 unidade); **D aterra na diagonal ⟺ y=Ax**
(100/100 vs 100/100 — «atua exatamente nas transições» com conteúdo exato);
Inversor aceita (escada restaurada); **D⁴=mD²+I — mínimo x⁴−mx²−1 = a borda
DOBRADA (x→x², a dobra temporal); representantes no grau 4, o andar acima**;
D†D=(mA+I)⊕I. Condição do gerente cumprida → **papers/corpo_universal.tex
nasceu** (compila): cinco operações com conservação medida, Inversor,
membrana (def sobre teoremas), papéis com cidadania condicional
(Conservatório = espaço dos invariantes), tabela Peano≺Universal, extensões
SEM teorema (Pontryagin/Clifford/La Hire/Dirac contínuo — cada uma espera a
sua ponte). O 4º documento do sistema — atualizar a memória «três documentos»
se ele ficar.

**RONDA 13 — FECHO DO DIA (o vetor total no banal) — commit `7a1ba2a`, push.**
Prosa congelada em absoluto (nenhum .tex). **residuos_totais.js 9:0**:
R_total=(R_endereço,R_E,R_Φ,R_Φ₂,R_D) sobre os 20 pares reais — RETAIN só
quando TUDO zera; permutação (0,0,0,0,0) e a membrana SEM cegueira nova;
conteúdo/endereço REOPEN; **caso crítico: 11/11 espelhados em FLAGRANTE (E,Φ
dizem OK; Φ₂ apanha 11/11 E a membrana D apanha 11/11 — duas vigilâncias
independentes)**; §R6: 1º estado + transições reconstroem o corpo (metade
para cada lado no texto). R_D = transições (pares de bytes consecutivos) por
endereço — o operador que vive ENTRE estados, transportado do laboratório.
**Bateria final do dia: 9+13+12+10+7+7+16+10 = tudo verde.** 13 rondas,
14 commits `c0893a5→7a1ba2a`, tudo no ar. A mesma cadeia
corpo→observador→transformação→dual→inversão fechou no laboratório algébrico
E no corpus real.

**RONDA 14 (primitivas mecânicas + domínios) — commit `e88a7c3`, push.**
Coordenador insistiu («são primitivas operacionais; precisamos das operações
mecânicas») e o diretor autorizou COM a regra de ouro (biunívoco com medidor
verde). **Inventário revelou: TODOS os domínios pedidos já têm medidor no
repo** — re-atestados na hora: carnot.c (η=1−T_f/T_q pelo ∮ em ℚ; irreversível
CAI), topologia.c (d=|Δ₁−Δ₂|, d=0⟺isomorfos, transporte parabólico),
cosmologia.c (p.u. inteira, w órbitas de 4, w=−1 ponto fixo), sombra_cone.c
(simplex = sombra do andar acima), rede_dual.js 28:0 (perceptron=estaca Lei 1,
rotor Lei 2, Hopfield). corpo_universal.tex ganhou §primitivas (as 6
engrenagens ↔ medidores: leitura, ordem, transformação, fibra, membrana,
Inversor total) e §domínios (álgebra/análise/topologia/geometria/
termodinâmica/cosmologia/redes como realizações das 8 leis, Clifford parcial)
— com o estatuto: a tabela NÃO afirma equivalência entre domínios; a ponte
entre eles continua elo a elo. «Todo passo reverte, ou paga» é a 2ª lei
medida (carnot §C4).

**RONDA 15 (o laboratório de Clifford) — commit `1811de5`, push.** A frase do
coordenador («f'=−f⁻¹ ou f=f'; Clifford via transformada = soma dual de
multiplicação») passou pelo protocolo estrito (importar, não recriar; termos
cruzados à vista). **clifford_dual.js 14:0**: a lei Df=−f⁻¹ é A ESTACA da
borda (x†=−1/x), EXATA no setor de norma −1; no setor N=+1 dá Df=+f⁻¹ — a
«exceção» é estrutura: **Df = N(f)·f⁻¹, a graduação ℤ₂ de Clifford, medida**;
D²=id (Lei 1), f=Df ⟺ centro (b=0, grau 0); **o par de Dirac D e o gêmeo
dual G=[[0,I],[−A,0]]: {D,G}=0 EXATO, D²=L, G²=−L — Cl(1,1) sobre o corpo
metálico**; a soma dual de multiplicação: v=xD+yG → v²=(x²−y²)·L com o termo
cruzado nulo POR anticomutação — a hipérbole emerge derivada. Cayley–Hamilton
(M·M†=det·I) amarra as importadas. Estatuto no corpo_universal: Clifford
parcial→VERDE (única linha autorizada). Migração arquitetural Peano→Universal:
fica para o debate do gerente/diretor.

**RONDA 16 (a morfologia no Universal) — commit `7e43387`, push.** A ordem:
buscar a construção NO REPO antes de derivar. Inventário: erosão/dilatação =
a adjunção δ⊣ε (toolkit.c §K3 8ok; morfico_ordem.c 3ok — o raio SOMA;
Porquês=ε/5W2H=δ); torção = o esquilo (E²=−I, det=+1) E a torção w do anel
(«a base é a órbita»); universal.c VERDE 42ok. **morfologia_universal.js
14:0**: adjunção 4096/4096; **abertura/fecho EMERGEM idempotentes; a volta
existe NA FRONTEIRA ADMISSÍVEL (δ(ε(A))=A nos B-abertos, 29/64)**; δ cria/ε
remove (62/62); raio soma; J norma exata (não é dilatação escondida);
**o DUAL DO DUAL DEVOLVE O GRUPO no anel — o degrau discreto de PONTRYAGIN
medido** (órbita fecha em 256, nenhum divisor próprio). Critério decisivo:
válida ⟺ dual ∧ volta ∧ invariante. Separação: δ/ε mudam o CORPO; a torção
muda o REPRESENTANTE. corpo_universal §sec:morfologia (thm:morf-par +
thm:torcao; morfologia = δ/ε + J; costura com Cl(1,1)); Pontryagin nas
extensões anotado (discreto medido; contínuo no mapa). PENDENTE da mesa:
a migração arquitetural Peano→Universal (o coordenador disse «sim, mas
antes a morfologia» — a morfologia está feita).

**RONDA 17 (inventário da migração + saneamento da bateria) — commit
`7f3ae42`, push.** Ordem: inventariar SEM mover; adaptador; bateria antes;
AGUARDAR O CORTE. **docs/MIGRACAO_UNIVERSAL.md**: hierarquia 𝒰/realização/
fundação, tabela de donos (mover: absorção, energia, observador do Peano →
𝒰; ficam: torre/música/histerese; mesa debate: a ponte), protocolo do corte
em 8 passos com o diff da contagem por documento.
**equivalencia_universal.js 7:0**: 𝒰[σ_Peano]≅𝒫 exato (E=38.731.623.179;
espectro 37.222; R_end e D iguais no íntegro e sob indução).
**A bateria investigada até a causa (416→420, 418 verdes, 2 falhas
pré-existentes nomeadas)**: 6 js OOM eram o ulimit -v 8GB vs a reserva
VIRTUAL de ~10GB/memória wasm do V8 (js agora 200GB virtual);
corpo_disco era MAX_FICH=64 com 69 ficheiros desde 11/08 (bisseção;
MAX_FICH→128, tex.wasm re-subido); **tests/tex.c NÃO COMPILAVA desde
0aa6ace (partitura só-WASM; INTERFACE_N/LADO_N só no libc) e o «NÃO
COMPILOU» não diz FALHA — o medidor que nunca mediu em flagrante; agora
compila (defaults no guard) e mostra §X9/§X16 nativas a falhar (wasm é o
canónico, verde)**; fala.c é a antena (daemon) — decisão do dono; os 4
medidores novos citados no corpo_universal → varredura 416→420 («não
citado não é testado»). CUIDADO meu documentado: o s.replace sem assert
calou um no-op — asserção vazia minha, apanhada pela lista de não-citados.
PENDENTE: instrução final de corte.

**RONDA 18 — O CORTE (commit `10952e9`, push).** O «segue» executou o
protocolo dos 8 passos: movidas para o corpo_universal a Absorção, a
Energia/Parseval e o Observador (escada/teto-oito/iso-dual/fusão/fibra);
refs de instância viraram citações textuais (zero órfãs, varredura
\ref×\label); Peano ganhou §sec:instancia-universal (o quadro enuncia, a
instância MEDE); a ponte FICOU no Peano (decisão da mesa respeitada);
arquitetura re-apontada. **Guardas: bateria ANTES=DEPOIS 420/418/2 (diff
zero); união de citados 38=38 (ninguém saiu em silêncio); adaptador 7:0
antes E depois.** DECLARADO: **o Corpo de Peano é realização do Corpo
Universal — 𝒰[σ_Peano], propriedade medida.** 21 commits no dia,
c0893a5→10952e9, tudo no ar.

**RONDA 19 (manutenção pós-corte) — commit `0e86ca7`, push.** lyapunov
movido tools→tests/lyapunov_torre.js (a varredura só vê tests|banco) →
**bateria 421, 419 verdes**; citações atualizadas. **Veredito do tex nativo
FECHADO por bisseção: o pré-0aa6ace passava as MESMAS 62 unidades SEM
INTERFACE_N/LADO_N; a maquinaria da interface (0aa6ace) mudou o esp_gira e
§X9/§X16 quebraram — e estão POR TESTAR em qualquer lado desde então (o
nativo não compilava; o wasm não exporta a suíte §X). Expectativa velha ou
regressão do giro: DECISÃO DO DONO.** fala.c idem (daemon; modo -teste ou
args()). Ar verificado: GET 200. Pendências restantes ao dono: tex §X9/§X16,
fala.c, fusão de conceitos (sem objeto), os 8 não-citados (helpers).

**RONDA 20 — A REVERSÃO DA PARTITURA (commit `25d8c9e`, push).** O giro
TINHA regredido: desde 0aa6ace, espiral nível ≥3 → LADO_N=1 → sobe_exp_m
trocava esp_sobe (COM sinais) por esp_sobe_torre (toda-positiva) — sem
sinais a inversa morre (§X9) e o vão sup–sub colapsa (§X16). Reversão
cirúrgica: o giro soma SEMPRE com sinais; o Gentil continua no PASSO
(esp_passo_nv); torres todas-positivas coincidem nas duas somas → a indução
T+T* nada perde. **tex nativo 62:0 (igual ao pré-0aa6ace); DEZ suítes wasm
verdes (tex_wasm 9, traduz_volta 33, torre_induc 5, traduz_pi_dim 38,
autoria 8, corpo_disco 6, estrela_unifica 9, volta_compila 7, tex_ponta 4,
prod_fumo 8); bateria 421 — 420 VERDES, 1 falha (só o daemon fala.c).**
23 rondas, c0893a5→25d8c9e.

**RONDA 21 — LATEX COMO INTERFACE PADRÃO (commit `90ce0c3`, push).**
A reconciliação: o padrão é a PORTA, não o trono — as linguagens continuam
backends de porta única; por omissão o corpo entra e sai como .tex porque só
o LaTeX é (1,1,1) COM tradutor pleno e volta byte a byte (thm:composicao +
o 62:0 devolvido pela reversão da partitura). manifesto.json ganhou
interface_padrao=latex; **tests/interface_padrao.js 6:0** (declaração;
único pleno; compor†=descompor no wasm real via protocolo §W4 com
nulo_disco; atestado); arquitetura §pipeline + tabela dos papéis do
universal. **Bateria: 422 — 421 verdes, 1 falha (fala.c)**. 24 rondas.

**RONDA 22 — O CANVAS POR DEFAULT (commit `cd25ae6`, push).** O daemon
renderiza LaTeX no canvas por default, MESMO esquema do PDF: canvas_tex.js
(o pintor do dialecto — texto plano, glifos Forms m/l, ONZE operadores
q Q cm Do rg RG w m l h f S re, zero deps externas), comporTexto no
tex_tradutor (fonte virtual no Map + fich_miss + compila + MOVE(14,+1) +
selos + volta_compila), rendeCanvas na assistente (4 pontos de resposta,
fallback texto, title = Alonzo /SementeEstrela + Caelum /AssinaturaOito —
«ver o corpo de caelum e alonzo»). **canvas_front.js 11:0**: dualsort em
node pelo esquema, A4 exato, 51.744 traços, ZERO ops desconhecidos, Caelum
256 componentes, e a resposta solta embrulhada compõe E pinta. Build do app
OK. **Bateria: 423 — 422 verdes, 1 falha (fala.c)**. 25 rondas.

**RONDA 23 — FECHO: A BATERIA TODA VERDE (commit `6d3b46f`, push).**
`./fala -teste` (4:0, offline): empacota∘desempacota=id nas 4 ops;
bump∘bump=id (a Lei 1 no canal); mutação acusa (magic −2, corpo difere);
banda errada não lê (isolamento sha256). bateria args: fala→-teste.
**`total 423 : 423 verdes, 0 negativos, 0 falhas` — A PRIMEIRA BATERIA
COMPLETAMENTE VERDE DO REINO.** 26 rondas no dia, c0893a5→6d3b46f.
Pendências restantes: fusão de conceitos (sem objeto no banco); os
«não citados» helpers (tex_env/tex_core/prod_fumo — infra, não medidores).

**RONDA 24 — A FUSÃO SAI DA QUARENTENA PELOS DADOS (commit `19879e0`,
push).** O objeto foi ENCONTRADO: 7 pares singular/plural + >100 títulos
duplicados no cristal (reticulado↔reticulados, alekhine↔alexander_alekhine,
fourier análise↔transformada). **fusao_conceitos.js 8:0**: fusão = soma
direta com contorno (as partes viajam intactas — a dualidade é a memória da
divisão); **E(z)=E(x)+E(y)+E_∂ com E_∂=E(esqueleto)=410.151 EXATO, dois
caminhos — a previsão do gerente aterrou**; fibra devolve byte a byte;
no corpus é TRANSAÇÃO declarada (R_end=3: 2 saem 1 entra) e desfazer
devolve R=0 («todo passo reverte, ou paga»). Fundir DE FACTO = curadoria
do dono (cristal.jsonl intocado). corpo_universal: quarentena atualizada
com a caixa da conservação. **Bateria: 424 — 424 verdes, 0 falhas.**
27 rondas, c0893a5→19879e0.

Relacionado: [[project-checkpoint-2026-08-13-pipeline]], [[feedback-a-regua-nao-transporta]].
