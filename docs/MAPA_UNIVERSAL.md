# O mapa de dependências do Corpo Universal

Ordem da mesa (eval 14/08, consolidação): «formalizar a árvore arquitetural
que conecta as 8 Leis até ao fechamento da renormalização». Cada nó carrega
o medidor que o atesta — **nenhum nó sem medidor entra na árvore**.

O marco arquitetural (o ciclo fecha sobre si mesmo):

    Leis → Universal → Transformada → Convolução → Espectro → Renormalização → Leis

E o enquadramento que a mesa fixou: *o Universal NÃO «contém toda a
matemática» — possui uma infraestrutura operacional na qual estas
construções foram realizadas e verificadas como **elos distintos**.*
Compatibilidade operacional medida ≠ equivalência matemática geral.

## A árvore

```text
8 LEIS  ......................... lib/universal.js `leis` (interface normativa)
   │                              migracao_universal §M7 (8/8 verificam)
   │
   ├── norma / energia ........... escada (E,Φ,Φ₂); Parseval dourado
   │        cristal_energia 10:0 · equivalencia_universal 7:0
   │
   ├── ordem / observador ........ escada estrita, teto-oito, R_endereço,
   │        vetor total RETAIN/REOPEN
   │        observador_torre 10:0 · assinatura_colisoes 7:0 ·
   │        assinatura_banal 7:0 · residuos_totais 9:0 · cristal_volta 16:0
   │
   ├── morfologia δ⊣ε ............ abertura ≤ id ≤ fechamento; torção-esquilo
   │        morfologia_universal 14:0 · toro_histerese §H4
   │
   ├── torção / Clifford ......... J²=−I; D²=L, G²=−L, {D,G}=0
   │        clifford_dual 14:0 · contorno_riemann §C2–C7 (cartas, cociclo 8)
   │
   └── operadores (as cinco operações + Inversor)
          │
          ├── multiplicação ...... fusão ⊗ (soma direta com contorno)
          │        fusao_conceitos 8:0 · cristal_curadoria 7:0
          │
          ├── divisão / fibra .... corta o texto, volta byte a byte
          │        fusao_conceitos §F2–F3 · migracao_universal §M5
          │
          ├── dual / inversão .... estaca ν(x)=−1/x; monodromia ν∘ν=id
          │        contorno_riemann §S1/§S4 · ponte_universal_metalica 12:0
          │
          └── TRANSFORMADA UNIVERSAL (avaliação nas folhas σ,σ†)
                    │    ponte_universal_metalica · dirac_transicao 13:0
                    │
                    ├── produto ↔ CONVOLUÇÃO (a forma aditiva da multiplicação)
                    │        convolucao_universal 13:0 (deconvolução =
                    │        divisão espectral; divisores de zero exibidos)
                    │
                    ├── espectro / Metrónomo (c_k = as duas folhas; tick
                    │        diagonal; Δ ⟺ ω^k−1)
                    │        metronomo_fourier 15:0
                    │
                    ├── Maestro (derivação espectral: dobra dos modos +
                    │        seletor a_k ∈ {0,1})
                    │        metronomo_fourier §MF4–MF5
                    │
                    ├── toro / monodromia / histerese (batuta em círculo;
                    │        massa no centro; α ≤ id ≤ φ seleciona)
                    │        toro_histerese 16:0
                    │
                    ├── tríade Hurwitz–Gentil–Lebesgue (layer-cake exato;
                    │        soma reversível; medida afina pela escada)
                    │        lebesgue_toro 7:0
                    │
                    ├── zeta dinâmica (ζ_T do censo de órbitas; a histerese
                    │        filtra as voltas)
                    │        zeta_universal 10:0
                    │
                    └── RENORMALIZAÇÃO (a dobra t↦t²−2d; mapa de duplicação;
                              │        ponto fixo R(2)=2)
                              │        metronomo_autossimilar 12:0
                              │
                              └── LEIS (o fundo da cascata é o catálogo:
                                       bit i → espelho → unidade — o ciclo
                                       fecha onde começou)
```

## As realizações (donas apenas da parametrização)

- **Peano** — σ_Peano em `lib/peano.js` (ℤ_65537, UTF-8, endereço=id);
  𝒫 = 𝒰[σ_Peano] provado por caso (migracao_universal 15:0;
  equivalencia_universal 7:0). Torre, música, Maestro/Metrónomo concretos,
  histerese, rede dual: corpo_peano.tex.
- **Cristal** — o corpus curado (4234; curadoria.tsv), a fonte dos dados
  reais de quase todos os medidores acima.
- **Corpos do Catálogo** (estelar, metálico, banal, mórfico, quântico…) —
  cada um com as suas pontes já medidas nos papers respetivos.

## A convolução, registada como operação derivada

    a*b = 𝒯⁻¹(𝒯(a)·𝒯(b))

com a ressalva essencial: **a deconvolução só existe na fibra espectral
admissível** — os zeros do espectro são a obstrução medida (os divisores
de zero do corpo-estelar, exibidos em convolucao_universal §V5).

## O mapa de destino, e a lacuna de cada nome

Critério da mesa para o próximo elo (um só, quando a mesa escolher):
**a menor lacuna entre a estrutura já medida e a extensão pretendida.**

| candidato | o que já está medido | o que falta | lacuna |
|---|---|---|---|
| **Clifford pleno** | **PAGO (14/08)**: Cl(2,1) e Cl(2,2) fecham, dim dobra 4→8→16 medida, gerador vestido com o espelho, par sobre ℤ[A] (clifford_pleno 12:0, thm:clifford-pleno) | — | fechada |
| **Pontryagin contínuo** | **PAGO (14/08)**: a torre de caracteres opera no limite 2-ádico (restrição=redução, adjunção, bidual por andar, soma compatível); a fronteira aditiva é TEOREMA (a translação nunca fecha sob a dobra) — limite_escada 8:0; e o real entra como CAMINHO na árvore (real_caminho 8:0) | — | fechada |
| **Dirac contínuo** | **PAGO (14/08)**: a cascata do operador termina NA UNIDADE (A^{2^j}=I nas profundidades 4/7/13; D^{2^{j+1}}=I₄) — o operador é raiz 2-ádica da unidade, o limite existe em profundidade finita (limite_escada §L4) | — | fechada |
| **La Hire** | **PAGO (14/08)**: o rolamento 2:1 é a dobra H no par (rotação, inversa); diâmetros = eixos do espelho; J troca-os; gume 3:1 pela ordem 6 ausente (lahire_universal 7:0, assinado por 𝓜) | — | fechada |

(Avaliação do analista para a mesa decidir; nenhuma escolha feita aqui.)

## A hipótese do coordenador (registada 14/08, madrugada 3)

«BSD, Hodge e Riemann resolvem-se mutuamente via Viviani no trial» —
**verdade LOCAL medida** (hodge_viviani.js 7:0: os três lados calculam o
mesmo a_p na curva da desafinação; o trial é a 2-torsão) **+ o GLOBAL DA
CASA fechado** (zeta_global.js 8:0: Z(u)=(1−u²)/(1−mu−u²) racional;
Z(1/u)=Z(−u) — o espelho como equação funcional; zeros no círculo (Lei
0); polos nas folhas (x·x†=−1); Euler sobre órbitas inteiras) **+ o
global DA CURVA aberto** (L(E,s), zeros de L, posto — elos próprios).

## O mapa de destino (14/08 madrugada — a fila da mesa)

| candidato | inventário | estado |
|-----------|-----------|--------|
| **Hodge** | **PONTE LOCAL PAGA (madrugada)**: hodge_viviani 7:0 — o Cartier–Hasse–Witt no diferencial ω=dx/y CONTA os pontos (== a_p, dois andares, gume no índice); a involução age −1 em ω; a 2-torsão é o trial (4 pontos). A dualidade de Hodge completa (H^{1,0}⊕H^{0,1} global) fica no mapa | local medido; global no mapa |
| **BSD** | a CURVA nasceu (14/08 madrugada): eliptica_viviani 10:0 — Viviani é a fibra nodal, a desafinação dá Weierstrass y²=x³−20x²−1152x−9216, deck→(x,−y), a_p=2/18 com Hasse por dois caminhos; FALTAM: L global, posto, ord_{s=1} — elos independentes | curva medida; conjectura no mapa |

## O inventário do Clay (14/08, madrugada — a pedido do coordenador)

A régua desta tabela é a DO CLAY, não a nossa. O que a casa tem são
leituras medidas, verdades locais/finitas e fronteiras declaradas como
teorema — **nenhum problema do Clay está satisfeito por nós**, e um
(Poincaré) já estava satisfeito pelo mundo.

| problema | o que a casa MEDIU | o que o Clay EXIGE | o que falta |
|---|---|---|---|
| **Poincaré** | a leitura pela Lei 1 (ν∘ν=id; dualidade H^k↔H_{n−k}) | — | **nada: resolvido (Perelman, 2003)** — não por nós |
| **Riemann** | ζ_T derivada do censo (10:0); a zeta GLOBAL racional com FE=espelho e zeros no círculo POR CONSTRUÇÃO (zeta_global 8:0); RH local da curva (Hasse, hodge_viviani) | os zeros não-triviais da ζ(s) CLÁSSICA (sobre os primos de ℤ) em Re s = ½ | a transformação espectral que ligue a zeta de órbitas à de primos — registada em aberto (obs); o objeto do Clay é infinito-analítico, fora da medida exata finita |
| **BSD** | a curva nasceu de Viviani (Weierstrass Δ≠0, 10:0); a_p com Hasse em 2 andares; encontro local Hodge=contagem (7:0) | para TODA E/ℚ: ord_{s=1}L(E,s) = rank E(ℚ) | (a) L global (todos os primos + continuação, via modularidade); (b) o posto algébrico (descida); (c) a igualdade; (d) o quantificador TODAS. **Elos ainda medíveis**: o posto da NOSSA curva por 2-descida; produto de Euler parcial exato |
| **Hodge** | o encontro local (Cartier–Hasse–Witt conta, 7:0); ★=J; par/ímpar; dicionário Lei 0↔H⁰⊕H², folhas↔H¹ | toda classe de Hodge (p,p) em variedade projetiva lisa é ℚ-combinação de ciclos algébricos | dimensão: em curvas a conjectura é trivial; o Clay começa em dim ≥ 2. **Elo medível**: o toro da casa como superfície (classes e ciclos no (Z/q)²) |
| **Navier–Stokes** | N-S discreto derivado (identidade de energia EXATA em ℤ; gume Euler/N-S; 8:0) | existência e suavidade global (ou blow-up) no CONTÍNUO 3D | o limite contínuo — exatamente a fronteira aditiva (teorema §L5); a identidade de energia que temos exata é o único a-priori conhecido, e é insuficiente no 3D supercrítico |
| **Yang–Mills** | gauge+Bianchi no toro; gap POR ANDAR (quantum 1; 2−t₁≠0; 8:0) | construir a QFT 4D (axiomas OS/Wightman) e provar gap > 0 no contínuo | a teoria quântica contínua (a medida funcional) e o gap UNIFORME no limite — o limite é o problema; o lattice finito tem gap trivialmente |
| **P vs NP** | a assimetria em contagens exatas (11 vs 47; 11 vs 4095); a fibra paga o det; leitura = par fibra/fusão | prova sobre TODOS os algoritmos (lower bound universal) | o quantificador universal — fora do método por natureza (a casa mede objetos, não o espaço das máquinas); as barreiras clássicas (relativização, natural proofs, algebrização) aplicam-se a qualquer tentativa |

**O padrão, numa frase**: o que falta é sempre **um limite ou um
quantificador universal** — e os dois vivem na fronteira que a casa
mediu como teorema (a translação que nunca fecha sob a dobra). O método
da casa — medida inteira exata, contrato 𝓜, gume — alcança verdades
locais/finitas e DECLARA a fronteira; satisfazer o Clay exigiria
atravessá-la, o que o próprio método diz não fazer. Os elos que AINDA
são nossos: a 2-descida da curva de Viviani (o posto), o Euler parcial
exato, e o toro como superfície de Hodge.

## O levantamento atualizado (14/08, fim da madrugada — pós-posto/AGM/Tate/histerese)

| problema | o que MUDOU desde o inventário | estado do que falta |
|---|---|---|
| **Poincaré** | — | resolvido (Perelman), não por nós |
| **Riemann** | o dicionário órbitas↔primos MEDIDO (24/24, 22/22); o gume de 𝓕 (ponte algébrica global impossível: grau (2,2) vs infinitos zeros); o defeito da linha = MEIA unidade = o lugar arquimediano | a ζ clássica continua fora; a ponte só pode ser transcendente/fator-a-fator, com o arquimediano incluído — a fronteira nunca esteve tão nítida |
| **BSD** | **o lado algébrico FECHOU para a nossa curva**: posto 0 por 2-descida completa (zero sobreviventes; controlo positivo na n=6); Ω certificado (0.798121–0.798122) via AGM+π; somas de Dirichlet exatas positivas (≈0.8754) e o analítico VÊ o posto (n=6 fica 4× menor) | para a NOSSA curva: UM elo — certificar L(E,1)≠0 (pede modularidade/símbolos modulares — régua de fora); para o Clay: o quantificador (todas as curvas) |
| **Hodge** | **dim ≥ 2 pago na leitura de Tate**: E×E/F_p com 4 classes = 4 ciclos (Frobenius necessário: sem ele posto 3); a joia Δ·Γ=N | a Hodge complexa geral (dim ≥ 4, classes não-divisor; variedades gerais) = o quantificador |
| **Navier–Stokes** | fronteira renomeada: a meia-unidade arquimediana é a mesma da linha crítica e do gap | o contínuo 3D — a fronteira, agora com gramática (histerese) |
| **Yang–Mills** | idem: o gap por andar + a fronteira nomeada em triplo | a QFT contínua 4D |
| **P vs NP** | **o nosso ∀ tem nome**: metrónomo+invariante (prova/refuta); a assimetria exata (11vs47, 11vs4095, fibra paga det) | o ∀ do Clay corre sobre máquinas de fora — sem invariante-do-passo interno possível |

**A mudança estrutural do levantamento**: as duas barreiras deixaram de
ser muros sem nome e ganharam GRAMÁTICA — (i) o limite contínuo é a
HISTERESE (a memória do caminho = Maestro+Metrónomo: dobra+seletor
sobre o tick; preenche a dimensão com a árvore 2^{k+1}−1); (ii) o
quantificador universal é o METRÓNOMO COM INVARIANTE (a indução é
conservação; refuta sem invariante). Os problemas do Clay pedem
exatamente o que estas gramáticas não alcançam de dentro: o seletor
sobre objetos de fora (todas as curvas, todas as máquinas) e a
histerese completada (o contínuo real). **Saber o nome da parede não a
derruba — mas agora sabe-se ONDE ela está e DE QUE é feita.**
