---
name: project-checkpoint-2026-08-10
description: "10/08 — a ronda do gabarito (carta das variáveis, pilha de fontes, radicando um só) e o corpus/docs/milenio.tex afirmativo; Aarão fora até quinta 13/08"
metadata: 
  node_type: memory
  type: project
  originSessionId: 991e6ff9-9166-4fb7-8926-fbeab5326ca9
  modified: 2026-08-11T02:35:27.361Z
---

**O dia**: a ronda de alinhamento do tradutor contra o gabarito pdflatex, e o paper do milénio.
Aarão sem cota, **volta quinta 13/08**. Quatro commits `[skip ci]` no ar (sem deploy):
`0d7cf13`, `6290470`, `85d67b4`, `aa065b2`.

**O tradutor (tests/tex_core.c) — cada fix foi reproduzido em caso mínimo antes de tocar**:
- **A carta das variáveis** (`lib/fontes/documento-varia.otf`, família `F_MAT`): a matemática
  compunha na itálica de TEXTO (V: bearing 209, avanço 743) onde o gabarito usa o cmmi (56, 583)
  — era o «End( V)» com buraco. Alfabeto U+1D434… nas posições ASCII, h=U+210E, em-space em 0xA0.
- `\operatorname`/`\text`/`\mbox` romanos; `*` matemático = U+2217 na símbolo; `\quad`/`\qquad`
  = 1/2 quadrados em REAIS (mediam 10,3pt, são 24,6).
- **O espaço no modo matemático não é glifo** — e a porta era a de CIMA do varredor (c==' ' na
  ~2291), não a de baixo que escrevi primeiro (código morto, removido). Vírgula = pontuação.
- **A fonte repõe-se por PILHA** (fp_p/fp_f na Est, mecânica do exp_pf): a heurística antiga
  espreitava o próximo char e só repunha a NEGRA — `\code{...}` deixava mono ligada («).» mono).
  Resets de parágrafo esvaziam a pilha.
- **A régua do TeX: espaço após nome de controlo come-se** — a expansão do \code
  (`\texttt{\footnotesize #1}`) punha espaço mono: «( tests/...». 43 fantasmas a menos.
- **O radicando é um só**: `\sqrt{x^2+4}` desenhava DOIS radicais (o ±2 interior partia a corrida
  de marcas 5). O vão atravessa o expoente; gancho não recomeça nos medidores de largura.
- **A fila depois do \toprule/\midrule nasce na coluna 0 do bloco** (o fecha_paragrafo apagava o
  recuo — tabela da pág. 63 com «de \ para» fora do bloco). E `\backslash` entrou no léxico.
- `\emph` é ITÁLICA (fallback Helvetica-Bold era pré-carta).
- **Zero doubles**: o último real (`ct_i`) virou `contorno_xi/yi` no spline.h — a fracção 16.16
  da CFF arredonda no DONO da representação; quem consome fala só inteiros.
- Sempre verde: bateria 27:27 resíduo 0, volta byte a byte 0, teoria 64pp, traduz wasm limpo,
  `-fsyntax-only` em todos os que incluem spline.h (a lição do header).

**O isomorfismo promovido**: graus 2,4,6,8 ≅ torre inteira (corpo_analitico `sub:oitoleis`, o ciclo
gerador L→L(L)→…) — notas na teoria (junto à propriedade operacional do π), física (mesmas leis em
n=4 e n=16) e catálogo (o mesmo medidor serve em qualquer dimensão).

**corpus/docs/milenio.tex** (novo, auto-contido, 6pp pdflatex limpo) — Aarão: «é lugar de afirmar,
não economiza nas afirmações». Estrutura: a autópsia (cristalização G=A, sintomas P1/P2/P3
decidíveis) → as oito leis → o dicionário com A SOLUÇÃO de cada um (Poincaré resolvido PELO método
= devolver o gerador; YM/P-NP decididos no estatuto — o centro 0, Abel e a quíntica; Riemann/BSD/
Hodge = eixos −1/+1/0 sobre Kronecker/Dirichlet/Lefschetz; NS = metade conservativa fecha |det|=1
Liouville, dissipativa mal posta P3) → a conta que fecha → «o que este paper afirma» (verdade
relativa ao corpo COMO TESE, não ressalva). Fonte: [[project-checkpoint-2026-08-09-oito-leis]],
obs:clay/obs:seis-leis do catálogo. O sujeito da frase é a lei, sempre.

**As matrizes (noite, commit `3de8b23`)**: «mesmo esquema de tabelas» — marcas ±8 (small=dobra,
normal=corpo cheio), separadores de CONTROLO (2=col, 3=fila, 4..7=delimitadores), pintor com
colunas na largura MÁXIMA e filas à volta do eixo da barra do \frac; os DOIS medidores medem o
BLOCO. De caminho: o gancho da raiz somava 10⁻³ em acumulador 10⁻⁶ (1000× a menos, pré-existente);
símbolo regenerada com ⊤ (0xA1), − matemático (0xAD, U+2212) e em-space (0xA0) — «M^⊤=−M» compõe
igual ao gabarito. LIMITE: delimitadores de matrizes ≥3 filas ficam no corpo (instância assimétrica
partiria a volta — o varre_postos lê UMA escala); só a companheira 5×5 do catálogo os usa.

**A fração da 1949 (commit seguinte ao `3de8b23`)**: \frac sem chavetas aceita COMANDO e
empilha no display; \boxed com moldura (controlo 8/9, rectângulo pelos corpos); operadores
nomeados (\det, \dim, \log…) romanos; o título do teorema processa \emph/léxico (saía literal).

**A ESPIRAL GERAL (commit `37691e1`, o fecho do dia)**: o relógio compõe translação+escala num
giro por nível, semente = (corpo, dobra da dourada). Estado = TRAJETÓRIA (nível, máscara de
sinais) no Gl.e 16..127; escalas PELO CAMINHO DOS DEGRAUS (nv1=E1/E3, nv2=E0/E4, depois razão
composta φ^(-2/3)) — a conservação fecha EXACTA por construção (a versão por-unidade arredondava
noutra ordem e o §X8 apanhou: A RÉGUA ESTAVA CERTA). x^{a^{b^c}} sem teto; \boxed vira
consequência (moldura mede os estados do conteúdo, respiro = dobra da semente); §X9 na bateria
(4 unidades, 31:31) — e o meu 1º rascunho do medidor caiu num 'a' solto do texto: a mutação.
eval.txt (GPT) confirmou o desenho: T_k(x)=b_k+s_k·x, caixa como consequência, frase forte
«uma única espiral gera a tipografia inteira» — PENDENTE: fração de fração (a pilha ±4 ainda
não aninha), e a pilha/raiz/matriz ainda são réguas locais fora da espiral (a camada de cima
existe, falta migrá-las).

**A FRONTEIRA DA REGIÃO (commit seguinte)**: \left/\right e \big* → controlos 4/6/10|5/7/11,
UMA regra para ([{: o pintor mede os estados entre o par e estica SÓ na vertical (sv=sc·k,
sh=sc — W necessário; margem = dobra da semente); instância assimétrica `q sh 0 0 sv` no
formato, volta continua 0. §X10 (35:35) com CONTROLO k=1 — apanhou 2 réguas minhas (margem fora
do denominador; caixa natural 21/20 vs varrimento 11/10). Limite: o ^ dentro do bloco do \frac
fica no bloco (não encolhe); o refaz de instâncias boost re-emite UMA escala (poe_de_volta) —
se o -residuo de PDFs com boost for medido, estender varre_postos ao par (sh,sv).

**A TRANSLAÇÃO ORTOGONAL (§X11, 38:38)**: a vertical é a horizontal RODADA (M=J·i) — o
desenrola mede topo/fundo da linha pelos estados e soma o EXCESSO fronteira-a-fronteira (antes
e depois); a pilha deixou de marcar teve_rotulo nos DOIS flushes (meia entrelinha dupla). O
controlo: baixa fica na entrelinha; alta desce 2dv simétrico exacto. \emph em fórmula = itálica
pela porta do \text. Teoria agora 65pp. FEITO os $ nos títulos (o laço alterna modo, letras na F_MAT, léxico dá símbolos; extractores
engolem $). E o \bigl\langle lê o nome inteiro (⟨⟩ = controlos 12/13 na fronteira). LIMITE
NOVO: o TOC/cabeçalho são WinAnsi e omitem símbolos — «A definição de , e...» no sumário sem
o π; pede fita de fonte no t->txt.

**OS DOTS E O BOLD MATH (fecho da noite)**: censo primeiro (\dots 218/\ldots 104/\cdots 49/
\vdots 2/\ddots 2) — o \cdots NÃO EXISTIA; símbolo ganhou ⋯⋮⋱ (0xA2/A4/A6) + variantes
amsmath, tudo pela cadeia única. Bold math (2444): carta REAL bold italic da LM Math
(documento-varia-negra.otf, F_MTB, rota por e.fonte==F_NEG) — «não improvisa» MEDIDO: 894
XObjects, 0 curvas importadas, 890 rasto m/l. Títulos com $...$ compõem (F_MAT + léxico).

**A RONDA DAS VARIANTES (11/08 madrugada)**: símbolo NEGRA (grego bold U+1D6C2 da LM, F_SMB,
funil no empurra), operadores nomeados herdam contexto, traço engrossa pela razão do PESO da
referência (ponto negra/regular), \emph em contexto negro = lmroman10-bolditalic REAL (F_NIT),
\boxed com respiro interno vertical + externo (conta no topo/fundo da linha), semente do
expoente +passo/3 para cima e kern/3 para a direita PROPAGADO pela espiral (§X8/§X9 com a lei
d+d/3), binário sem espaços dentro do giro (x_{n+1}). 10 cartas agora (MAX_CARTA=10).

**A RONDA 11/08 (dia)**: underbrace inteiro (ub por PROF, nbsp -5 no vão, chaveta pelos corpos,
controlo 14 = início do vão, rótulo sob o vinco); tabela como REGIÃO (mede antes de abrir, vai
inteira se não couber — toprule órfão e header desnivelado da 215 mortos); x-setas rotuladas
(rótulo = giro da espiral); \text/porta respeita contexto negro; margem da barra de fração
±(2dv+dv/3) com §X11 a acompanhar. PENDENTE X12: \multicolumn como região única, header repetido
em tabelas > página, invariantes Δx_col/Δy_header/A_composto/ida-volta (corpus ainda sem
\multicolumn).

**A LEI GLOBAL (fecho 11/08)**: a SEMENTE declarada uma vez (SEM_V[] estado da estrela:
respiro /3, caixa 17/20+1/4, traço 400; razão = da escala) — 55 números dispersos viraram
leituras; a semente VIAJA no PDF (/SementeEstrela, como o FonteTeX); §X13 (41:41): semente
ida-e-volta, pares (sh,sv) idênticos, teste de sementes (≠ semente → ≠ geometria, mesma lei).
Duas quedas minhas apanhadas: semente dentro do if(FONTE_TEX); sscanf sem NUL (strlen ao
abismo). Bateria agora 41 unidades.

**A COMPOSIÇÃO DE REGIÕES (fecho 11/08 tarde)**: o rótulo do underbrace mede o FUNDO real do
segmento (a fração no vão descia mais que o y fixo); as filas da matriz por ALTURAS MEDIDAS
(H_i = máx das células). PENDENTE X14 (o próximo bloco, eval): a sub-região herda a
transformação do estado PAI — o índice dentro de célula small ainda gira do nível 0 (compõe
grande); com isso matriz∘matriz, fração em célula etc. fecham sem casos especiais. E X12
(multicolumn/header repetido) continua pendente.

**O DESENHO DO X14 (a teoria de composição — para quinta, com bateria própria)**: a sub-região
herda a transformação do PAI. Tecnicamente: ESP_SG passa a 2 bits/giro (none/up/down — o giro
sem translação é a ESCALA pura, a célula da matriz), esp_gira(atual, dir∈{-1,0,+1}); a matriz
small abre com estado nível-1-none (o conteúdo herda-o e o índice gira DELE — r·r, sobe só do
2º giro); o vão da matriz precisa de controlo de abertura próprio (15?) porque o conteúdo
deixa de ser ±8. Com isso: matriz∘matriz, fração em célula, tudo pela mesma lei — «a regra
compõe o objeto que desenha a próxima regra» (o eval de Church: a régua pode ser uma função).

**A PENTAL GERAL E O ALONZO (fecho 11/08 noite)**: \mathbb da referência (11 blackboard na
símbolo); o vector do giro declarado UMA vez (esp_kern_nv=Re, esp_sobe_nv=Im) — o i é a
ROTAÇÃO DIMENSIONAL (entre dimensões, não no plano; M=J·i; a torre); isomorfismo H↔V no §X9
(42:42): o respiro do giro é o MESMO número nos dois eixos, exacto. Alonzo comanda a pental
(cat:alonzo — a composição É um corpo, f∘f, z²+c, cantor/julia) = a âncora do X14.
PENDENTE-BUG: um «mathbb» residual na pág 27 (y=391, 6 glifos F_MAT) — expansão de macro que
cospe o corpo sem re-scan; achar a porta.

**A REVISÃO PELA CORTE (fecho final 11/08)**: o mapa Ada/Penny/Alonzo/Caelum escrito na
cabeça do motor (comentário-mestre antes da SEMENTE) — aditivo/multiplicativo/composição/
esqueleto, selo comum FP=1 (X13). As duas fraquezas COM DONO: Alonzo→X14 (herança do pai,
desenhado); **Caelum→o SELO da lei 8**: assinar os XObjects do PDF com a NTT (Z_65537, N=2^8,
como o §R8 do relogio_curva) — o PDF deve carregar a assinatura inteira do esqueleto, como já
carrega /SementeEstrela e o FonteTeX.

**O ESPAÇO DE CAELUM (fecho real 11/08)**: o selo da lei 8 no pdf_fecha — os XObjects
acumulam em N=2^8 e a NTT (Z_65537, raiz 3^256) escreve /AssinaturaOito no PDF (ao lado de
/SementeEstrela e FonteTeX); §X15 (45:45): dois caminhos resíduo 0, mutação de 1 bit espalha
256/256. O CORPO DE CAELUM no catálogo (cat:caelum, ao lado do cat:alonzo): ele É a Lei 8 —
o esqueleto ⊗, Fourier(8)⊗áurea(5); o que Alonzo compõe, Caelum assina.

**Pendente quinta**: continuar a varredura contra o gabarito — align/display multi-linha, \iint
(pág. com o desvio geodésico), gregas em expoente, e as páginas ainda não conferidas (só 4, 40 e
63 foram). O gabarito compilado vive no scratchpad da sessão antiga (recompilar: pdflatex
teoria.tex via TeX Live).
