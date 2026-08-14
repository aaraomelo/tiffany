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
