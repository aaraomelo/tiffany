# Os corpos na ISA — o mapa da tríade (⊕ ⊗ ∏)

> Cada corpo do catálogo, com a sua régua, a tríade escrita nas operações da **ISA ERG-64**, os teoremas
> certificados (resíduo 0), o ponto do contrato, a categoria  e a **jogada** (como vira combate).

> **20 corpos mapeados.** Faltam: técnico, rotor, cósmico, universal, nervoso, exterior, sensitivo, deflexivo.

---

## fractal

**Personagem:** O REI σ_m (o ponto fixo / o gap metálico σ_m=m+1/σ_m, σ₁=φ): o corpo fractal é a RAIZ onde "os metais são os pontos de contato" da cascata até ℝ; o rei ϖ=π/AGM(1,√2) nasce da AGM (PA×PG, a lemniscata). O operador ∏ é PONTRYAGIN (o Príncipe/operador, exp∘Σ∘log) — quem dá os inversos e leva o anel ao corpo. O par criativo aparece nos Duques: ⊕ o Duque (Joaquim) / ⊗ a Duquesa (Yasmin).

**A régua.** A régua μ (T_ω) é z·z̄ — a NORMA pela conjugação (o produto de La Hire de z pelo seu flip dual z̄, que dá |z|²). É a régua do contrato na tabela das 11 realizações (catalogo.tex linha 228: fractal ℚ(ζ₈), Σ=ℤ/8, régua z z̄ conjugação). Mede o "tamanho" invariante do elemento; nas esferas de divisão ímpares (S¹,S³,S⁷ de ℂ,ℍ,𝕆) a norma é MULTIPLICATIVA |xy|=|x||y| (Hurwitz para em 𝕆). O ponteiro conforme mede −0,5 = −½·d² em todas as 6 certificadas (fractal_corpo.py).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | ⊕ CLIFFORD (a soma, a lemniscata, a PA) — componente a componente: (a,b,c,d)⊕(a',b',c',d')=(a+a',b+b',c+c',d+d'). Grupo abeliano: neutro (0,0,0,0), inverso −u. É ℤ[i] aditivo empilhado. Na ISA ERG-64: ADD em i64, exato (resíduo 0 real, sem float), quatro somas independentes nas 4 coordenadas. | |
| **⊗ La Hire (o produto)** | ⊗ LA HIRE (o produto, o relógio, ℤ/8) — a convolução ciclotômica com ζ⁴=−1: c₀=a₀b₀−(a₁b₃+a₂b₂+a₃b₁), c₁=a₀b₁+a₁b₀−(a₂b₃+a₃b₂), c₂=a₀b₂+a₁b₁+a₂b₀−a₃b₃, c₃=a₀b₃+a₁b₂+a₂b₁+a₃b₀. Comutativo, neutro (1,0,0,0); ζ⊗ζ³=ζ⁴=−1 (a rotação de 45° dá a volta). A fórmula fechada bate com a convolução crua (certificado). Na ISA: a MULT por double-and-add (ADD deslocado) para os produtos aᵢbⱼ, e o SUB (o sinal ζ⁴=−1) para os termos negativos — inteiro, resíduo 0. | |
| **∏ Pontryagin (o operador)** | ∏ PONTRYAGIN (o operador, exp∘Σ∘log): ∏(z,w)=exp(log z+log w)=z·w — leva a SOMA dos ângulos ao PRODUTO das raízes (ζ₈ʲ·ζ₈ᵏ=ζ₈^{(j+k)mod8}). O INVERSO z⁻¹=exp(−log z) é o degrau que dá os inversos e LEVA O ANEL AO CORPO (sem ∏, ℤ[ζ₈] é só anel, 2 sem inverso; com ∏, 2↦½ e ℚ(ζ₈) é corpo). Na ISA: NOT/dispatch/composição — a NEGAÇÃO do log (−log = o "NOT" do expoente) e a composição exp∘Σ∘log; para as unidades o inverso é exato (ζ⁻¹=−ζ³, um SUB/negate). | |

**Os teoremas (certificados, resíduo 0).**

- fractal_corpo.py 6/6 (resíduo 0): as 8 unidades geram ℤ[ζ₈] (ℤ/8) — ℤ[i] dá 4 (±1,±i), o relógio dá 4 (±ζ₈,±ζ₈³) via Gentil; ⊕/⊗/∏ explícitas; ℤ[ζ₈] é ANEL (2 sem inverso, ½∉ℤ[ζ₈]); com ∏ vira CORPO ℚ(ζ₈)=ℚ[x]/(x⁴+1), Φ₈=x⁴+1 irredutível, 2↦½
- fractal_newton.py 5/5 (resíduo 0): o fractal de Newton no rotor z∈ℂ — inteiros↔irracionais (as bacias que fecham = ℤ/ℚ, a fronteira fractal = irracionais); as n raízes formam o grupo ℤ/n (o relógio); as iterações formam o monoide (ℕ,+), Nᵃ∘Nᵇ=N^{a+b}
- realizacao_corpos.py 5/5 (resíduo 0): o contrato do corpo fractal — Σ=ℤ/8, régua μ=z·z̄ (conjugação), flip dual ν=conjugação z↦z̄
- torre_relativistica.py 6/6 e reta_metais.py 6/6 (resíduo 0): a cascata das torres ℕ→ℤ→ℚ→ℝ fecha nos REAIS onde os metais são os pontos de contato; período 8 (Brauer-Wall/Bott real): rei +7 metais =8=dim 𝕆
- torre_rotores.py 4/4, torres_metalicas.py 4/4, torres_verifica.py 4/4, esferas_metalicas.py 4/4: a torre de Cayley-Dickson ℝ→ℂ→ℍ→𝕆 (dim 1,2,4,8), Hurwitz para em 𝕆, as esferas paralelizáveis S⁰,S¹,S³,S⁷ (Adams)

**O dual (flip ν).** ν = conjugação z↦z̄ (o flip dual da tabela de realizações). Primal ⊕ dual: z⊕z̄ = (2a,0,0,0) = 2·Re(z) (a projeção real, o par de Clifford), e z⊗z̄ = |z|² = a régua (a norma). É a complementaridade da lemniscata ℤ[i]: a reflexão sobre o eixo real que fecha o produto na norma. No jogo, o lance-espelho.

**O contrato.** Σ=ℤ/8 (as 8ª raízes ζ₈, o relógio, a rotação de 45°). Régua T_ω = z·z̄ (a norma pela conjugação). Flip dual ν = conjugação z↦z̄. Mate o: o corpo fecha em ℚ(ζ₈) (todo x≠0 invertível pelo operador ∏). O casamento Γ=0 / FP=1: a norma unitária |z|=1 — nas esferas de divisão a norma é MULTIPLICATIVA (|xy|=|x||y|), o rotor |z|=1 é o cone nulo do corpo (nenhum elemento vaza; mover fora de |z|=1 seria FP≠1, proibido). Certificado do contrato em realizacao_corpos.py (5/5, resíduo 0).

**o signo.** A qualidade própria do corpo é PRIMEIRIDADE / ÍCONE (a semelhança): o fractal é auto-similar — o tabuleiro que se parece consigo em toda escala (a tesselação nativa, as bacias de Newton auto-similares). Dentro dele a tríade se distribui: ⊕ o par (secundidade/índice, a conexão factual da soma), ⊗ a mediação, e ∏ Pontryagin a LEI/o hábito (terceiridade/símbolo) — o operador-convenção que costura ⊕ e ⊗ e, dando os inversos, medeia a subida ANEL→CORPO.

**A JOGADA.** O tabuleiro fractal É a raiz: a tesselação auto-similar nativa (zoom = mesmo tabuleiro). Cada peça carrega um RUMO = uma das 8 unidades ζ₈ᵏ (o relógio ℤ/8, 8 direções de 45°). LANCE DE GIRO = ⊗ La Hire: multiplicar o rumo por ζ₈ gira a peça 45° (tique do relógio); ζ⊗ζ³=−1 é o giro completo à meia-volta. LANCE DE AVANÇO = ⊕ Clifford: somar deslocamento (empilhar coordenada a coordenada, a PA da lemniscata). A PROMOÇÃO = ∏ Pontryagin: concede o INVERSO — transforma um peão do ANEL (unidirecional, sem desfazer, 2 sem inverso) em peça do CORPO (bidirecional, pode reverter/undo via −log). O ESPELHO = o flip dual ν (conjugação z↦z̄): reflete o rumo do inimigo; e z⊗z̄=|z|² é a RÉGUA DE DANO/ALCANCE (a norma, quanto o golpe pontua). REGRA DE CONSERVAÇÃO: só rumos de norma unitária |z|=1 (na esfera S⁷, na circunferência do anel) são lances legais — mover com |z|≠1 VAZA (FP≠1 proibido). O ATRATOR DE NEWTON: uma peça lançada ao tabuleiro converge a uma das 8 raízes (lance de homing à âncora mais próxima); se cai na FRONTEIRA fractal (irracional) nunca assenta — casa instável/contestada.

---

## criativo

**Personagem:** As PEÇAS são os operadores (não um único personagem): Torre=⊕ (Clifford), Bispo=⊗/∧ (La Hire), Dama=C⊕Dafny, Cavalo=2C⊕D, Rei=C⊕Dafny dual das peças, Peão=bit⊕carry∧. O corpo criativo é o GERADOR (o lado gerar do par gerar↔atestar); o seu dual são as Princesas (o corpo técnico que ATESTA — Dafny/Isabelle, o kernel LCF). Os operadores compostos-filhos são o Duque ⊕ (Joaquim) e a Duquesa ⊗ (Yasmin).

**A régua.** Mede a estrutura algébrica que as oito linguagens realizam, sobre conjuntos de palavras (linguagens L ⊆ Σ*, alfabeto {a,b} até comprimento 3 no certificado). A régua T_ω é a invariância sob os oito substratos: os oito substratos dão o MESMO estado, e o resíduo nulo L△L=∅ é invariante sob a régua. O monoide de base é a concatenação (·) com neutro {ε} — o monoide sintático de –. Na ISA, a régua vira exata: as três operações escritas em naked Chess (ERG-64), i64, resíduo 0 real, sem float. Uma segunda régua acoplada (a programação dinâmica, criativo_dinamica.py) é o semianel (⊕,⊗): uma régua por problema — troca o semianel e o MESMO algoritmo de Bellman resolve outro problema (alcança?/menor caminho/quantos caminhos/paridade F_2).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | ⊕ Clifford (a SOMA, ⊥) = a DIFERENÇA SIMÉTRICA △: L△M=(L∪M)\(L∩M). Comutativa, neutro ∅, e cada linguagem é o seu próprio inverso: L△L=∅ (o resíduo nulo, o x⊕x=0 do corpo binário nas linguagens). Os dentes do refutador: trocar △ por ∪ falha, pois L∪L=L≠∅ — a união não tem inverso, △ tem. Na ISA: XOR (a corrente no nó, a diferença simétrica). No xadrez é a Torre (retas, C, PA, emit_c). | |
| **⊗ La Hire (o produto)** | ⊗ La Hire (o PRODUTO, ∘) = a INTERSEÇÃO ∩: L∩M, associativa, idempotente (L∩L=L), e distribui sobre △: L∩(M△N)=(L∩M)△(L∩N) — o que faz de (P(Σ*),△,∩) um ANEL BOOLEANO. Os dentes: pôr ∪ no lugar de ∩ (∪ não distribui sobre △) faz a lei do anel falhar. Na ISA: AND (a interseção, o ganho); a MULT double-and-add colapsa aqui na idempotência booleana (x⊗x=x). No xadrez é o Bispo (diagonais, ∧, Dafny, PG, dafny run). | |
| **∏ Pontryagin (o operador)** | ∏ Pontryagin (o OPERADOR que costura) = NOT = XOR com todos-1 (0xFFF…FF): a involução, o complemento — o DUAL. Na teoria o operador que eleva o monoide/anel é composto, Turing∘Pontryagin: TURING = o fecho universal (o μ / while / estrela de Kleene, o resíduo sem cota — f≡1 nunca chega a 0, a paragem); PONTRYAGIN = o adjunto de – (índice à direita/hereditariedade ↔ índice dos dois lados/seleção; a diferença é exatamente o contexto esquerdo u, o adjunto S†). No dispatch da ISA (corpo_criativo_isa.py): o opcode no slot 9 (0=⊕/XOR, 1=⊗/AND, 2=∏/NOT) — o próprio dispatch por opcode É Pontryagin, a composição que costura as três. ∏=exp∘Σ∘log (o twist). | |

**Os teoremas (certificados, resíduo 0).**

- criativo.py 12/12, resíduo 0, ponteiro −½ em todas (VERIFICADO rodando agora): · associativa e neutro {ε}; △ comutativa com neutro ∅; L△L=∅ (o resíduo nulo, x⊕x=0); ∩ distribui sobre △ (anel booleano); L∩L=L idempotente; μ (while) sem cota vs for com cota (Turing); nerode=sint=2 no 'par de a's', nerode=3<sint=5 no 'termina em ab' (Pontryagin, S†); △≅Clifford ∧ ∩≅La Hire; L△L=∅ ≡ x⊕x=0 que o Dafny prova nos oito substratos
- corpo_criativo_isa.py — a tríade na ISA pelo chessc: ⊕=XOR, ⊗=AND, ∏=NOT exatas no interpretador (erro 0,0,0 em 300 casos de 64 bits); interp≡WASM (emit_wasm, roda no wasmtime/metal, erro 0); interp≡WASM≡Dafny (emit_dafny/Z3 prova x⊕x=0, x⊕0=x, assoc, comuta, x⊗x=x para TODO x); artefato assets/figuras/wasm/corpo_criativo.wasm — o GABARITO do catálogo
- criativo_dinamica.py 10/10, resíduo 0: a régua é o semianel (⊕,⊗); troca o semianel, o mesmo algoritmo de Bellman resolve outro problema; o produto de semianéis resolve o problema composto num passo — (min,+)×(+,×) devolve (5,1) (menor custo E quantos caminhos); a régua errada é o problema errado
- colapso_colaborativo.py — o corpo criativo reproduz o Capítulo do Grafo byte-a-byte (462 nós, 2535 arestas, 210.647 bytes), cada linguagem no seu Φ
- partida_completa.py — mate em 23 lances (Evans Gambit → 12.Qxf7#); partida_conservacao.py 5/5 — a Ópera de Morphy, material conserva 108, expansão colapsa a 0 no mate
- mate_kr.py: KR/KQ vs K fecham em 17 e 29 lances; a tablebase (33 classes de –) mate KR ótimo em 13 lances

**O dual (flip ν).** O flip dual ν é gerar↔atestar: o corpo criativo (gerar, o Φ generativo) é o dual do corpo técnico (atestar, o kernel LCF/Dafny/Isabelle) — primal+dual = o neutro. Dentro do corpo, ∏=NOT é a própria involução/complemento (o dual: L e o seu complemento). No plano da régua, o flip é a antissimetria indução(S)↔meta-indução(π) — o que se preserva (o Rei no xadrez). Nas peças, ν=e↦−e (a reflexão do Rei-deflexão que as peças refletem, mantendo o gap ±1); nos peões ν=−1 (os dois mostradores, a seta do tempo).

**O contrato.** Realização completa: Σ=Z/n é a órbita/relógio (nas linguagens, as classes de –, o índice sintático; nα ISA a roda de opcodes, slot 9). Régua μ=T_ω: a invariância dos oito substratos, o resíduo nulo L△L=∅. Flip dual ν: gerar↔atestar / complemento NOT / indução↔meta-indução. Mate ∘: o mate do xadrez, a cristalização (resíduo→0). O casamento Γ=0: o resíduo nulo x⊕x=0 (o cristal); e no idioma de potência (obs:fp-criativo) toda cena gerada fecha com FP=1 — o inversor casado à bateria (Γ=(Z−Z0)/(Z+Z0)=0, o cone nulo σ=1, E²−B²=0). Nenhum universo pode ter FP≠1: seria energia fora do Sol, e o corpo criativo só gera o que o contrato assina. FP=1 É a conservação (a conservação: ∏_v|x|_v=1).

**o signo.** Terceiridade / o SÍMBOLO (a convenção). A linguagem é convenção pura — o corpo das oito linguagens é a lei/mediação/hábito, o símbolo . É também o corpo onde a tríade inteira se lê: ⊕/△ o par (Secundidade/índice, a diferença factual), ⊗/∩ a mediação, e ∏ (o operador Turing∘Pontryagin, o dispatch/NOT) a lei/terceiridade/símbolo. Como corpo gerador que costura os oito substratos numa convenção comum (o ERG-64), a sua qualidade dominante é a Terceiridade — a mediação simbólica que faz todas as linguagens dizerem o mesmo estado.

**A JOGADA.** O corpo criativo JOGA xadrez — as peças que ele move SÃO os seus operadores (obs:pecas). O lance = a operação: mover a TORRE = executar ⊕ (Clifford/XOR, C calcula as retas); mover o BISPO = executar ⊗ (La Hire/AND, Dafny calcula as diagonais); a DAMA = C⊕Dafny grau (1,1) (retas ⊕ diagonais da mesma peça, Q=R+B); o CAVALO = 2·C⊕1·Dafny grau (2,1) (Pontryagin graduado); o REI = C⊕Dafny de um passo, mas DUAL das peças (elas refletem o seu passo, ν=e↦−e, mantendo o gap ±1); o PEÃO = o bit ⊕ com o carry ∧ (soma com transporte x+y=(x⊕y)+2(x∧y)). O colapso Ψ é o minimax/argmax (a seleção de Pontryagin/Bellman); a abertura segue as órbitas clássicas (indução no grafo). Resultado concreto e certificado: partida completa da inicial ao mate, Evans Gambit 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 4.b4 … 12.Qxf7# — xeque-mate em 23 lances, a Dama (C⊕Dafny) cristalizando; a Ópera de Morphy reproduzida em 0,6s. O MATE é a cristalização: a expansão entrópica (a mobilidade) colapsa a 0, resíduo 0 — e NÃO é dissipação: é a transferência ao dual (o fechamento: a soma sobre todos os lugares é 0). O mate não mata. Cada célula-jogador tem a sua réplica do grafo (o barramento distribuído, P2P).

---

## eletromagnetico

**Personagem:** Não nomeia Rei/Rainha diretamente, mas o ELENCO da tríade está presente: o Duque ⊕ (Clifford, a soma dos perfis) e a Duquesa ⊗ (La Hire, as impedâncias). O corpo é o campo que PROPAGA — o torque do motor é este mesmo Poynting E×B dando trabalho (obs:em-cosmico). O cone nulo é o ponto fixo do flip (o traço de mate/morte do corpo).

**A régua.** Mede o par de campos (E,B) por Gram-Schmidt: o perfil h_i = log||b~_i||² (entropias das colunas ortogonalizadas). A régua/telescópio é a SOMA do perfil: Σh = log det G = log|E×B|² — o vetor de Poynting (a energia que propaga, via identidade de Lagrange det G = |E|²|B|² − (E·B)²). O passo/curvatura é a impedância σ = e^{−Δh/2} = |E|/|B| para E⟂B (o metal do campo). A norma/assinatura da forma quadrática E²−B² sobre (E,B) é (p,q,r)=(3,3,0), grau 6 — três redondas, três hiperbólicas (medido em elementares/assinaturas.py); a onda plana é o vetor isotrópico.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD, a soma: os perfis h são ADITIVOS — o telescópio Σh = log/E×B/² (o Poynting) é a superposição dos campos, aditiva. Empilhar meios SOMA os perfis Δh₁+Δh₂. Na ISA ERG-64: ADD (a soma exata i64 dos log-perfis / a corrente de Kirchhoff que superpõe). | |
| **⊗ La Hire (o produto)** | (x) LA HIRE, o produto: as IMPEDÂNCIAS multiplicam ao empilhar meios — σ(Δh₁+Δh₂) = σ(Δh₁)·σ(Δh₂). A tensão/ganho da régua é multiplicativa (o metal). Na ISA: a MULT por double-and-add (o produto das impedâncias = ADD deslocado dos perfis via o log). | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN, o operador que costura: ∏ = exp∘Σ∘log. A impedância σ = e^{−Δh/2} e log σ = −Δh/2 — o log LINEARIZA a impedância (multiplicativa, La Hire) no perfil (aditivo, Clifford); o exp sobe de volta. É o MESMO operador dos juros, da expansão e do cósmico. Na ISA: o dispatch/composição exp∘Σ∘log (o ponteiro/poinsétia que percorre o perfil e costura soma↔produto). | |

**Os teoremas (certificados, resíduo 0).**

- Poynting é o telescópio: Σh = log det G = log|E×B|² — refutado o uso de só |E|² (eletromagnetico.py, ok)
- identidade de Lagrange: det G = |E|²|B|² − (E·B)² (ok)
- o passo é a impedância: para E⟂B, σ = e^{−Δh/2} = exatamente |E|/|B| (ok; σ(φ)=φ, o áureo dá λ=−2·log φ; o prateado 1+√2 dá λ=−2·log(1+√2))
- o vácuo é a régua reta E o cone nulo: onda plana |E|=|B| → σ=1, λ=0, e os dois invariantes de Lorentz anulam-se E²−B²=0 e E·B=0 (ok)
- autodual ⟺ Poynting unitário: Σh=0 ⟺ |E×B|=1 (ok)
- Hodge (E,B)↦(B,−E) é a REV do corpo telescópico: inverte a ordem do perfil e PRESERVA o Poynting (ok)
- a dualidade da RÉGUA é ν∘rev: INVERTE o Poynting (Σh↦−Σh) e troca E por 1/B — a impedância invertida (|E*|²=1/|B|², |B*|²=1/|E|²) (ok)
- a tríade/contrato: SOMA Clifford (perfis aditivos), MULT La Hire (impedâncias multiplicam), OPERADOR Pontryagin (log lineariza) — as três ok
- o fechamento energético: FP=1 É o cone nulo σ=1; o inversor casado Γ=(Z−Z0)/(Z+Z0)=0 (Z=Z0) o realiza, 1−|Γ|²=1 — nenhum universo conserva com FP≠1 (ok)
- PLACAR REAL da execução: 17/17 certificadas, resíduo 0, ponteiro −0.5 em todas (o .tex diz 13/13 no resumo e 16/16 no rodapé, mas eletromagnetico.py corre 17/17)

**O dual (flip ν).** Duas dualidades distintas, e a distinção é o teorema central. HODGE (E,B)↦(B,−E) é apenas a REV: permuta o perfil, PRESERVA o Poynting — falta-lhe a reflexão. O FLIP DUAL ν da régua é ν∘rev (ν=−1): INVERTE o Poynting (Σh↦−Σh) e troca E por 1/B (a impedância invertida). Primal+dual encontram-se no ponto fixo comum |E||B|=1 com σ=1 — o vácuo de Poynting unitário, o cone nulo.

**O contrato.** Cumpre o CONTRATO (teo:triade-em): a tríade Clifford⊕/La Hire⊗/Pontryagin op. O CASAMENTO de impedância Γ=(Z−Z0)/(Z+Z0)=0 (quando Z=Z0) é REALIZADO exatamente aqui: é o CONE NULO σ=1 (|E|=|B|), o FATOR DE POTÊNCIA UNITÁRIO. Este corpo é a instância física do critério Γ=0=FP=1 de todo o reino: nenhum universo pode conservar (resíduo 0) com FP≠1 — seria energia surgindo fora do Sol, o que o Contrato proíbe. Σ=Z/n é a órbita do flip; a régua μ é T_ω=σ; o mate ō é o cone nulo.

**o signo.** SECUNDIDADE / índice como qualidade dominante — o corpo é o par (E,B), o embate/reação de dois campos (E×B, o produto que gera o Poynting = conexão factual). A tríade interna mapeia: (+) Clifford = o par/superposição (secundidade/índice); (x) La Hire = a mediação das impedâncias; (op) Pontryagin exp∘Σ∘log = a LEI/hábito que costura (terceiridade/símbolo). A qualidade pura do campo (a onda plana, o vácuo, o cone nulo σ=1) é a primeiridade/ícone — o estado sem reação, a régua reta.

**A JOGADA.** Cada peça/onda no tabuleiro carrega um par (E,B) e uma impedância σ=|E|/|B| (o seu metal). O LANCE de propagar = a superposição (ADD dos perfis, Poynting Σh); o LANCE de atravessar meios empilhados = multiplicar as impedâncias (La Hire, o casamento de terreno). O ATAQUE máximo é o lance CASADO: levar o alvo ao CONE NULO σ=1 (|E|=|B|), onde FP=1 e toda a potência do Poynting PROPAGA (transferência 1−|Γ|²=1) — o golpe que não vaza. Um lance DESCASADO (σ=φ≠1) deixa potência REATIVA presa (E²−B²≠0): energia que volta, o vazamento — dano perdido. O MATE é o cone nulo: o ponto fixo do flip onde o corpo morre (curvatura zero, a régua reta). O contra-lance é o flip de Hodge (E,B)↦(B,−E): gira o par preservando o Poynting (troca o eixo do ataque sem perder energia).

---

## O corpo MOTOR

**Personagem:** O REI-GERADOR (Pontryagin, o operador exp/log que costura o gerador G à evolução exp(tG); o DTC é o seu comando de controle ótimo). A RAINHA aparece como o produto La Hire (⊗, o torque/o ganho). Os DOIS FILHOS/duques na forma do dipolo: há DUAS de cada peça em cores opostas (o par branco/negro, Joaquim ⊕ / Yasmin ⊗). O motor é ainda o PRÍNCIPE (a potência/a exp ρ=e^{at}) e, no eixo do ecossistema, o corpo CRIATIVO (quem executa/gera), dual das Princesas técnicas (quem atesta).

**A régua.** Mede a MAGNITUDE / a dissipação: a parte real do gerador (ρ=e^{at}, log|z|), a seta do tempo. Não é forma quadrática — é o semi-eixo dissipativo (autovalores complexos de parte real negativa). A régua nativa distingue conservativo↔dissipativo pela imagem dos autovalores (Im=0 vs Im≠0). No chessc a régua concreta são as DUAS operações inteiras exatas (i64, resíduo 0 real): soma a+b e produto a·b, uma só IR ERG-64 com dispatch por opcode (slot 9: 0=soma, 1=mult). Do torque τ=ψ×i=|ψ||i|sin(θ), máximo quando ortogonais (θ=90°).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | CLIFFORD (⊕) = a SOMA das dinâmicas/geradores: a soma direta G1⊕G2 (tabuleiros distribuídos), as taxas de dissipação/crescimento acrescentam-se; é a corrente de Kirchhoff no nó. Na ISA ERG-64: ADD. No chessc é o ramo SOMA (LOAD 1, LOAD 2, ADD, STORE 3) — a+b exato, dispatch op=0. Em polares, o produto de C vira a SOMA das magnitudes (log/zw/=log/z/+log/w/), que é o motor. | |
| **⊗ La Hire (o produto)** | LA HIRE (⊗) = o PRODUTO / a composição das evoluções exp(tG1)exp(sG2); e sobretudo o TORQUE τ=ψ×i (o produto vetorial fluxo×corrente, o Poynting E×B agora dando trabalho), máximo ortogonal. Na ISA: a MULT por double-and-add (ADD deslocado, com máscara AND por bit). No chessc é o ramo MULT: laço de 64 bits (LOAD 2/LOAD 4 AND para testar o bit; se ≠0, LOAD 3/LOAD 5 ADD acumula; duplica mask e aa) — a·b exato, dispatch op=1. A Rainha, o ganho/a tensão. | |
| **∏ Pontryagin (o operador)** | PONTRYAGIN = o operador exp/log que liga o gerador G (aditivo) à evolução exp(tG) (multiplicativo) — o rei-gerador. O DTC (Direct Torque Control) É Pontryagin: modula o torque por controle ótimo, o estado evolui por G e o co-estado (o valor) por Gᵀ (o princípio do máximo), certificado por exp(tG)ᵀ=exp(tGᵀ) com G≠Gᵀ (há rotação). O flip de WICK (a↦ia) É o de Pontryagin (gerar→atestar). Na ISA: o DISPATCH por opcode (slot 9, CMP+JZ que escolhe soma/mult) — a composição/o costurar. A tríade compõe: torque e avanço da fase saem de ⊗ e ⊕. | |

**Os teoremas (certificados, resíduo 0).**

- motor.py 4/4 (resíduo 0): (1) o dual é o DISSIPATIVO — conservativo tem autovalores reais (ponto fixo), dissipativo complexos de parte real <0 (a espiral); (2) o torque τ=|ψ||i|sin(θ) é o produto vetorial máximo ortogonal (Poynting E×B); (3) o DTC é Pontryagin — exp(tG)ᵀ=exp(tGᵀ) com G≠Gᵀ (co-estado por Gᵀ); (4) cada peça tem lado branco/negro — os dois bispos em cores opostas (dipolo dissipativo)
- motor_rotor.py 5/5 (resíduo 0): motor=magnitude ⊕ rotor=fase JUNTOS completam o corpo C (dim 2, Hurwitz); duais por WICK (a↦ia, cosh a=cos(ia)); a involução ×i preserva a SOMA mas NÃO o produto (i(zw)≠(iz)(iw)) — dualidade de complementação, não isomorfismo; em polares o produto de C = soma das coordenadas (log|zw|, arg(zw))
- dualidade_criativo_motor.py 5/5 e iso_motor_tecnico.py 5/5 (resíduo 0): motor⋈rotor ≅ criativo⋈técnico — o motor (magnitude dissipativa) ≅ o corpo criativo (executa/gera); o flip de Wick É o de Pontryagin (gerar→atestar); log z = motor + i·rotor = criativo + i·técnico (Curry-Howard: programa↔prova)
- axiomas_corpo.py e reta_geral.py 4/4 (resíduo 0): os NOVE axiomas de corpo (A1-A4 soma, M1-M4 produto, D distributiva) valem para TODA a reta metálica Q(√(m²+4)), não só C — a reta universal φ_ν(x)=νx (soma pontual, compõe multiplicativo)
- circuito_rlc.py 6/6 (resíduo 0): o RLC — a capacitância dual, o gap 4 como bateria, o inversor que corrige o fator de potência; a ressonância Z_L=Z_C ⇔ tr M=0 (autovalores no eixo imaginário, o ciclo puro) dá FP=1
- regua_motor_chessc.py: a SOMA (⊕) e a MULT (⊗) numa só IR — interp≡WASM≡Dafny (soma 17+25=42, mult 6·7=42, torque ψ×i=7, avanço 142), erro 0; emite assets/figuras/wasm/regua_motor.wasm (Dafny sujeito à presença do binário)
- painel_motor_chessc.py: o avanço da fase nova_fase=fase+ω·dt (⊗∘⊕) — interp≡WASM≡Dafny (100+6·7=142), erro 0 em 4 casos; emite assets/figuras/wasm/painel_motor.wasm

**O dual (flip ν).** O ROTOR (rotor.tex) — a FASE, o círculo U(1), CONSERVATIVO/unitário, ≅ o corpo TÉCNICO (atesta). O flip nu é o de WICK (a↦ia: o boost dissipativo vira rotação; cosh a=cos(ia)) = o flip de Pontryagin (gerar→atestar). É dualidade de COMPLEMENTAÇÃO, não de transporte estrutural (a involução ×i preserva a soma Clifford mas não o produto La Hire). Primal ⊕ dual: magnitude ⊕ fase = C. No plano do ecossistema: criativo(motor) ⋈ técnico(rotor); no elétrico: indutivo ⋈ capacitivo (Z_L ⋈ Z_C).

**O contrato.** Σ=Z/n: a órbita cíclica da rotação (o motor cai em CICLOS, não pontos fixos — o dual do conservativo). Régua μ=T_ω: a magnitude/dissipação (semi-eixo, o avanço da fase por ω·dt no painel). Flip dual ν: Wick a↦ia (motor↔rotor, magnitude↔fase). Mate ⊙: a ressonância que fecha casa↔peça. O CASAMENTO Γ=0 é quase literal aqui: a grandeza conservada é a energia reativa, e na RESSONÂNCIA Z_L=Z_C as reatâncias cancelam, a impedância fica resistiva, Γ=(Z-Z0)/(Z+Z0)=0 → FP=1 (toda a potência transfere, resíduo 0). No operador 2×2 é tr M=0 (autovalores no eixo imaginário, o ciclo puro). FP<1 é a reatância descasada (vazamento); o inversor multinível (Pontryagin, degraus de Fibonacci φ^{-j} da bateria áurea) casa cada harmônico. Nenhum universo pode ter FP≠1.

**o signo.** SECUNDIDADE / o ÍNDICE — o motor é o embate, a REAÇÃO factual: dissipação (elétrica→mecânica dá trabalho), o torque como par ortogonal fluxo×corrente, o dipolo branco/negro (o par). É a força bruta que resiste e empurra (a Secundidade  = ação-reação, o esforço). Contrasta com o rotor (Terceiridade/lei, a fase que conserva a norma = o veredito que conserva a validade). O operador Pontryagin que o costura (exp/log, o DTC, a mediação G↔Gᵀ) traz a Terceiridade/símbolo; a qualidade dissipativa pura (a espiral, e^{at}) é a Primeiridade/ícone.

**A JOGADA.** O motor é o MOVER da peça — a mecânica de rotação/torque que empurra o combate. No jogo: o motor gira o campo (a gaiola de esquilo) e dá TRABALHO — o torque τ=ψ×i é MÁXIMO quando fluxo e corrente estão ORTOGONAIS (o golpe de flanco a 90°, não frontal). O PAINEL do motor é o controle de velocidade: o SLIDER é ω, e cada tick avança a fase nova_fase = fase + ω·dt (⊗ compõe com ⊕: a MULT faz ω·dt, a SOMA acrescenta à fase). O DTC (Pontryagin) permite MODULAR o torque em jogo — o co-estado (o valor futuro Gᵀ) guia quanto empurrar agora. E cada peça carrega o seu LADO BRANCO/NEGRO: há DUAS de cada (os dois bispos em casas de cores OPOSTAS, _cor(7,2)≠_cor(7,5)), o dipolo dissipativo — o par ressoa entre a casa e a peça (o atrator cósmico local). No metal: o app roda painel_motor.wasm/regua_motor.wasm (a ponte da CPU) e entrega a fase ao canvas GLSL (u_time) — o motor gira o pixel.

---

## O RELÓGIO UNIVERSAL (elementares/relogio.tex + relogio.py)

**Personagem:** O RELÓGIO UNIVERSAL em si (obs:omnitrix) — não Rei/Rainha, mas o instrumento-mãe do tempo: a roda Σ=ℤ/n da espinha, que CLASSIFICA cada corpo pela aridade (a ordem da sua rotação característica). Encarna o operador PONTRYAGIN (exp.Σ.log, o ponteiro/poinsétia que percorre a órbita). A rapidez λ é a coordenada do recobrimento; a família ⊕_k, ao varrer k, entrega a régua de todos os personagens anteriores.

**A régua.** Mede o TEMPO — quão depressa o relógio corre. A régua é o LAPSO N(ψ)=cos ψ = √a = sech λ, o único compatível com a métrica ds²=-N²dθ²+dψ² de curvatura constante (R=-2N''/N; impor R const + N par + N(±π/2)=0 força N=cos ψ, R=2). O tempo próprio de um passo é dτ=N dθ; a taxa vale 1 no ponto comutativo (s=0, relógio máximo) e 0 no quatérnio (s=±1, o relógio para). N²=a=|1-s²| é a magnitude do associador — a régua É a raiz da não-associatividade. No plano geral T_ω é a rapidez λ=artanh s (avança a taxa constante λ̇=1); o comprimento total da régua ℓ_k varre os três ramos: redonda (ℓ=π, k<0), reta (∞, k=0), hiperbólica com horizonte |s|=1 (k>0). Por isso é o ESPECTRO: ao varrer k, ⊕_k entrega a régua de todos os outros corpos.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | CLIFFORD (a soma) = a ADIÇÃO RELATIVÍSTICA DE VELOCIDADES: s₁⊕s₂ = (s₁+s₂)/(1+s₁s₂), grupo abeliano na ilha (-1,1), neutro 0 (a álgebra comutativa), inverso -s, fechado. É a lei que o fluxo ṡ=1-s² impõe. Na ISA: sob o flip Λ=artanh vira ADD puro — Λ(a⊕b)=Λa+Λb, a soma na reta da rapidez. Na roda ℤ/n (relogio_cruzamento.py) ⊕ é GIRAR: os índices SOMAM mod n, (k₁+k₂) mod n, as rotações compõem e^{i(θ₁+θ₂)} → ADD mod n. | |
| **⊗ La Hire (o produto)** | LA HIRE (o produto) = a COMPOSIÇÃO DOS ENDOMORFISMOS da reta de rapidez: s₁⊙s₂ = tanh(artanh s₁ · artanh s₂), associativa, comutativa, distribui sobre ⊕, neutro tanh(1)=0,761594… (o "um" não é 1: é tanh da unidade da régua), inverso tanh(ℓ²/artanh s). Na ISA: sob Λ é a MULT double-and-add (Λ(a⊙b)=Λa·Λb, o produto na rapidez). Na roda ℤ/n ⊗ é ESCALAR: os índices MULTIPLICAM mod n, (k₁·k₂) mod n, a rotação eleva-se (ω)^k → MULT mod n (se n primo fecha corpo F_p). O cruzamento de corpos de aridade m,n é a La Hire m·n (relogio_cruzamento.py). | |
| **∏ Pontryagin (o operador)** | PONTRYAGIN (o operador que costura) = Λ = artanh, o FLIP/exp-log que endireita: leva (I,⊕,⊙) em (ℝ,+,·), transportando ⊕→+ e ⊙→·. É o flip de Wick na forma hiperbólica (tanh no lugar de tan), e a função de Gudermann (sin ψ = tanh λ) é o dicionário entre as duas réguas. Também é a reflexão ν=-1 (s↦-s, ponto fixo s=0). Na ISA: composição/dispatch — em relogio_aridade.py o Pontryagin do relógio calcula a MULTIPLICIDADE TELESCÓPICA Π_{k} e^{2πik/n}=(-1)^{n-1} e o log CONVERTE o produto das rotações (multiplicidade) na soma dos ângulos (aridade): exp.Σ.log no círculo. | |

**Os teoremas (certificados, resíduo 0).**

- relogio.py: 68/68 certificadas, resíduo 0 (ponteiro -0,5 em todas)
- R = -2N''/N; impor R const + N par + N(±π/2)=0 + N>0 força N=cos ψ e R=2 (o lapso é o associador a=|1-s²|=cos²ψ) [relogio.py]
- o imposto da conservação da norma é N²·‖a×b‖²; resíduo nulo ⟺ N=0 ⟺ |s|=1 ⟺ a álgebra é associativa (ℍ): onde a medida fecha, o tempo para [relogio.py]
- ⊕=(s₁+s₂)/(1+s₁s₂) é grupo abeliano; Λ=artanh é bijeção (-1,1)→ℝ que leva ⊕ em + e ⊙ em ·: (I,⊕,⊙)≅(ℝ,+,·), É CORPO [relogio.py]
- o espectro ⊕_k=(s+t)/(1+k·st) em três ramos: k=-1 tan/régua redonda ℓ=π, k=0 id/régua reta, k=+1 tanh/horizonte |s|=1 [relogio.py]
- o fluxo ṡ=1-s²=N², s(t)=tanh(t+c), fonte s=0 e atratores s=±1: a velocidade do fluxo é o QUADRADO da taxa do relógio [relogio.py]
- cor:maquina — a máquina cria um bordo interior falso s*≈2/(p ln2), a bandeira cai no lance ⌈p·ln2/2⌉ (cristalização precoce) [relogio.py]
- partida_relogio.py: 7/7 resíduo 0 — s_k=ω^k·s_0 (órbita ℤ/8), a partida reversível por potências, só o MATE (norma→0) é irreversível
- relogio_aridade.py: 4/4 resíduo 0 — a aridade é a ordem n (universal=1, dual=2/-1, complexo=4/i, relógio=8, Pontryagin=∞); Π raízes=(-1)^{n-1}, log converte multiplicidade→aridade
- relogio_cruzamento.py: 9/9 resíduo 0 — ⊕ soma mod n / ⊗ mult mod n; Cayley-Dickson 1→2→4→8, Hurwitz para em 8; CRT ℤ/m×ℤ/n≅ℤ/mn

**O dual (flip ν).** O flip ν = Λ = artanh (o flip de Wick hiperbólico) é a própria estrutura do corpo: o métrico é o UNIVERSAL FLIPADO. A reflexão ν=-1 é s↦-s (ponto fixo s=0). A dualidade é o MODO COMO Λ_k FALHA: em k<0 é sobrejetivo não-injetivo (núcleo πℤ = o resíduo, quociente ℝ/πℤ, o ℤ-módulo do deflexivo); em k>0 é injetivo não-sobrejetivo (falta o fechamento ±1, o horizonte); k=0 é o único bijetivo/auto-dual (o degenerado). O bordo de uma régua é a 2-torção da outra: horizonte hiperbólico ↔ metade ℓ/2 da redonda, por tanh(iθ)=i·tan θ.

**O contrato.** Este corpo É o relógio do contrato: fornece o Σ=ℤ/n (a órbita, as raízes da unidade) e a taxa temporal para todo o reino. Régua μ = o lapso N=√a. Flip ν = artanh (o universal flipado). MATE ∘ = a cristalização N=0. O CASAMENTO Γ=0 é exatamente a condição de resíduo nulo: o imposto N²‖a×b‖² anula-se ⟺ N=0 ⟺ |s|=1 ⟺ álgebra associativa (ℍ) — o horizonte da ilha é a associatividade, o cristal (temperatura nula, lapso nulo, nada a medir). É o mesmo enunciado do cone nulo σ=1 / FP unitário: onde casa, o tempo não corre.

**o signo.** TERCEIRIDADE / o SÍMBOLO (a lei, a mediação, o hábito, a convenção). O relógio é o operador que MEDIA (Pontryagin exp.Σ.log); a órbita ℤ/n que fecha em ω^n=1 e conserva a norma para TODO k é a meta-indução — a classe de toda a partida, a lei que rege (não um ícone nem um par isolado). É a régua-de-réguas, o hábito que ordena os corpos por aridade.

**A JOGADA.** O relógio é o CONTADOR DE TURNOS / a roda ℤ/n do tabuleiro (partida_relogio.py: 2^128 estados = 8×8 casas × 2 bitboards, o hipercubo (ℤ/2)^128). O LANCE = um TICK = multiplicar o estado por ω: s_k = ω^k·s_0, a evolução é a ÓRBITA {ω^k·s_0}. No ℤ/8 o tick é a rotação de 45° que leva a reta da TORRE na diagonal do BISPO (relogio_cruzamento). A partida é a sucessão de estados-curva, e é REVERSÍVEL: ω^{-k} destica (o log a torna uma PA, a reta reversível; a órbita FECHA em ω^n=1 e a norma/o gap conserva |s_k|=|s_0|). No COMBATE as velocidades combinam por ⊕ relativística: nenhum combo cruza o HORIZONTE |s|=1 — o dano/velocidade satura mas nunca quebra a régua (o bordo não é elemento, só se alcança no limite). O único tick SEM inverso é o MATE: a cristalização, norma→0, o relógio PARA (N=0, resíduo 0) — o xeque-mate é o tempo que congela.

---

## telescópico

**Personagem:** O Rei — o gap metálico σ_m (σ_m=m+1/σ_m; σ_1=φ). A régua metálica vive em E₊: o perfil centrado com passo λ=−2 log σ_m é o único de passo constante que fica autodual (uma única órbita da deflexão D_λ). O índice metálico é o invariante da régua de passo constante; o invariante geral é o multiconjunto dos passos. As Princesas (o corpo técnico, o kernel LCF) atestam: 21/21 no formalizador, cada afirmação exibe um refutador.

**A régua.** Mede o PERFIL da régua reticular: as entropias h_i = log‖b~_i‖² dos eixos de Gram–Schmidt. Duas leituras acopladas, o telescópio: a NORMA N(h)=Π e^{h_i}=e^{Σh}=det G=covol² (o produto que telescopa) e o TRAÇO T(h)=Σh_i=log covol² (a soma dos logs). O telescópio é a norma; a soma dos logs é o traço. Num reticulado ‖b~_i‖²=det G_i/det G_{i-1} e o produto telescopa — decompor sem perder nem acrescentar. O resíduo da régua é T(h): o desequilíbrio, o desvio do covolume 1.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford = a SOMA PONTUAL h+g (componente a componente), com inverso aditivo −h (é o que dá SINAL ao corpo: as amplitudes cancelam). ISA ERG-64: ADD por eixo (i64, resíduo 0). Certificado: h+(−h)=0 (afirma sec.5, o refutador troca por max e falha). | |
| **⊗ La Hire (o produto)** | (x) La Hire = a MULTIPLICAÇÃO PONTUAL de R^n (produto de Hadamard eixo a eixo, a álgebra cindida). No domínio das entropias ela vira o telescópio: N(h⊙g)=N(h)·N(g), o produto de normas colapsa na soma dos perfis (verbo 7, a norma é multiplicativa). ISA: MULT (double-and-add) por eixo. É AQUI que a cisão degenera: e_i·e_j=0 são divisores de zero — o produto pode aniquilar. | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin = a DEFLEXÃO D_λ, o passo constante da régua (λ=−2 log σ_m), estilo exp·Σ·log — o passo dá D_λ e D_λ gera a órbita centrada (a régua metálica). Costura ao operador de dualidade ν=(−1)∘rev (a reflexão do contrato composta com a inversão de ordem, involução ν²=id). ISA: a composição/dispatch para D_λ (o passo aplicado por eixo) e NOT (a negação −1) composto com rev para ν. Certificado: diff(D_λ)=λ, ν∘ν=id, T(νh)=−T(h) (o telescópio é ímpar, a dualidade inverte o covolume). | |

**Os teoremas (certificados, resíduo 0).**

- o signo (sec.1): os eixos de Gram–Schmidt são idempotentes e_i²=e_i, ortogonais e_i·e_j=0, e somam 1 (Σe_i=1) — a decomposição por idempotentes; certificado, refutador troca produto por soma e falha
- Não é corpo (sec.1): para n≥2, e_1·e_2=0 com e_1,e_2≠0 são divisores de zero; n=1→R, n=2→R⊕R (W_{+1})
- Verbo 7 (sec.2): N(h)=Πe^{h_i}=e^{Σh} e é multiplicativa N(h⊙g)=N(h)N(g) — o produto telescopa
- Verbo 8 (sec.2): num reticulado N=det G=covol² por ‖b~_i‖²=det G_i/det G_{i-1} — decompor sem perder nem acrescentar
- Verbo 9 (sec.3): ν=(−1)∘rev é involução (ν²=id), aditiva, rev é automorfismo do anel, NÃO multiplicativa; T(νh)=−T(h), o telescópio é ímpar
- o signo da involução (sec.4): R^n=E₊⊕E₋ (±1-autoespaços de ν), dim E₊+dim E₋=n; E₊=antipalíndromos=réguas autoduais, e T anula-se em E₊ (Σh=0, covolume 1, unimodular)
- Sombra entrópica (sec.5): trocar + por max mata de uma vez o inverso aditivo E a reflexão (a idempotência é o preço, pago nas duas moedas)
- Verbo 10 / régua metálica (sec.6): o perfil centrado h_i=(i−(n−1)/2)λ, λ=−2 log σ_m, é antipalíndromo (∈E₊, autodual), Σh=0, covol 1; centrar na metade é o que torna a régua metálica autodual; não centrada Σh=−9,62
- Fechamento energético (sec.7): FP=1 ⟺ T(h)=0 ⟺ N=det G=1, o covol conservado / régua reta / cone nulo; nenhum perfil conservado tem FP≠1
- Assinatura de Sylvester de ν (obs.): (p,q,r)=(2,2,0), grau 4 (para n=4), medida em elementares/assinaturas.py
- Placar: 21/21 certificadas no formalizador, ponteiro −0,5 em todas, resíduo 0 (o .tex diz 20/20; a execução ao vivo tem 21 com o verbo 7 do fechamento energético)

**O dual (flip ν).** ν=(−1)∘rev — a dualidade da régua: ν(h)_i=−h_{n+1-i}. Involução aditiva (não multiplicativa), inverte o covolume T(νh)=−T(h). O primal + dual fecha no neutro em E₊: Fix(ν)=antipalíndromos=réguas AUTODUAIS, onde T=0 (covolume 1, unimodular) — o ponto fixo do telescópio é a régua autodual. Anti(ν)=palíndromos (E₋). Complementaridade R^n=E₊⊕E₋. A sombra que perde este flip é o corpo entrópico (soma trocada por max: reflexão colapsa na identidade).

**O contrato.** O casamento Γ=0 aparece como o TRAÇO ZERO (sec.7, o fechamento energético): FP=1 ⟺ T(h)=Σh_i=0 ⟺ N=det G=1 — o covolume conservado, a régua reta. O que um eixo estica (+h_i, indutor) outro contrai (−h_i, capacitor): soma 0, sem reatância líquida — o cone nulo σ=1 do corpo EM traduzido como o traço nulo. É Fix(ν) (a régua metálica centrada, autodual). Um perfil com T≠0 estica/contrai o covol (FP≠1): o contrato proíbe. Σ=Z/n é a órbita da deflexão D_λ (o passo constante); a régua μ é o par (N,T); o flip ν=(−1)∘rev; o mate é a régua unimodular T=0.

**o signo.** Terceiridade (símbolo/lei) como categoria dominante: o corpo É a decomposição por idempotentes feita norma — o produto que telescopa é a mediação/o hábito, decompor sem perder nem acrescentar (a lei multiplicativa N(h⊙g)=N(h)N(g)). Mas carrega no coração a Secundidade (índice/embate): os idempotentes ortogonais que se ANIQUILAM (e_i·e_j=0), os divisores de zero — a reação factual, o par que se cancela. A soma pontual com inverso aditivo (as amplitudes que cancelam) é o índice/secundidade; a norma-traço que telescopa é o símbolo/terceiridade. O corpo invoca o signo literalmente (a decomposição por idempotentes e o o signo da involução).

**A JOGADA.** A régua telescópica é o PERFIL de um alinhamento — uma coluna de eixos de Gram–Schmidt (a torre, uma fileira de peças ortogonais no tabuleiro). O LANCE de ataque = o produto que telescopa: encadeia-se dano por N eixos independentes e o dano total é a NORMA N=covol² que colapsa na SOMA das entropias Σh_i — atacar por muitos eixos é somar perfis, não multiplicar custo. O FLIP/mate = a reflexão ν=(−1)∘rev: espelha a régua invertendo o sinal, troca ataque por defesa e INVERTE o covolume (T→−T). A régua BLINDADA (invulnerável) é a autodual, T=0, covolume 1: o que um eixo ESTICA (+h_i, o indutor) outro CONTRAI (−h_i, o capacitor) — a soma zero, régua reta, sem reatância líquida (FP=1, o cone nulo). E o golpe-fantasma: dois eixos ORTOGONAIS têm e_i·e_j=0 — bater num eixo ortogonal ao do inimigo dá dano ZERO (a jogada some, não troca), o preço da cisão. A régua metálica do Rei (σ_m, passo λ) centrada na metade é o único perfil de passo constante que fica autodual — o lance-âncora que fecha o covol.

---

## O corpo cristalino: o quadratico IMAGINARIO C_D = Q(sqrt D), D<0 livre de quadrados. E o corpo de fracoes dos dois reticulados do plano: o quadrado de Gauss Q(i)=Z[i] e o hexagonal de Eisenstein Q(omega)=Z[omega]. E o corpo de REGULADOR NULO, o corpo em que a medida ja fechou. O seu traco proprio (Dirichlet) e o POSTO 0 do grupo de unidades: as unidades sao finitas (so raizes da unidade), nao ha regua a percorrer, o residuo e 0 em toda parte. E paga um preco exato: a restricao cristalografica so deixa passar n em {1,2,3,4,6}, e o PRIMEIRO traco proibido e o aureo (2cos(2pi/5)=phi-1, 2cos(2pi/10)=phi). O cristal proibe exatamente o preenchedor otimo. A tese: fechar exclui preencher.

**Personagem:** Nao ha personagem-heroi: o cristal e o ANTI-aureo. O Rei sigma_m (o gap metalico sigma_m=m+1/sigma_m, sigma_1=phi) e a Rainha La Hire (phi^2=phi+1) sao justamente quem o cristal PROIBE - phi entra so como traco de rotacao proibida (dobra 5 e 10). O traco proprio e o CRISTAL: regulador zero, orbita fechada, relogio parado - a armadura universal em sua forma mais rigida (o escudo que nao vaza). As unidades finitas (ordens 4 e 6) sao os unicos giros permitidos. Encontra o entropico (entropia zero) e o metrico/relogio (lapso zero) no MESMO ponto: tres nomes do residuo que se anula.

**A régua.** A norma DEFINIDA positiva N(z)=z.zbar=a^2+|D|b^2=|z|^2. Como D<0 anula-se so em z=0 (anisotropica), logo e corpo e z^-1=zbar/N (inversos construidos para D=-1 e D=-3). Assinatura de Sylvester (p,q,r)=(2,0,0), grau 2 - "duas redondas", a mesma regua do reflexivo e do celeste (medido em elementares/assinaturas.py). Mas a regua LOGARITMICA de Dirichlet (log|z|, o regulador) colapsa: a imagem do mergulho logaritmico e o reticulado de posto 0, isto e {0}. O relogio Sigma=Z/n esta PARADO: regulador = log 1 = 0. A difracao INVERTE a regua: covol(L).covol(L*)=1, Gram(L*)=Gram(L)^-1.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford = soma PONTUAL: (a+b.sqrtD)+(c+d.sqrtD)=(a+c)+(b+d).sqrtD. Na ISA ERG-64: ADD componente a componente (a parte real e a imaginaria somadas em paralelo). E a corrente de Kirchhoff lida no plano do reticulado - o par (secundidade). | |
| **⊗ La Hire (o produto)** | (x) La Hire = MULTIPLICACAO por composicao: (a+b.sqrtD)(c+d.sqrtD)=(ac+Dbd)+(ad+bc).sqrtD. Na ISA: a MULT por double-and-add (ADD deslocado); como D<0 o termo real ac+Dbd usa SUB (D e negativo). Geometricamente e rotacao-e-escala; restrita as UNIDADES vira rotacao PURA de ordem finita (4 em Z[i], 6 em Z[omega]) - a orbita fecha, nao expande. | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin = a CONJUGACAO / reflexao de Galois zbar=a-b.sqrtD, a involucao nu=-1 (a "torcao" do dicionario). Ela costura o inverso z^-1=zbar/N e a lei de Friedel F(-q)=conj F(q). Na ISA: NOT / negar a componente imaginaria (b -> 0-b, um SUB), o dispatch que troca o sinal. E tambem o inversor multinivel (exp.Sigma.log) que recoloca todo vazamento - o casamento FP=1. O contrato admite UMA involucao nao trivial, e e esta. | |

**Os teoremas (certificados, resíduo 0).**

- Restricao cristalografica: 2cos(2pi/n) em Z sse n em {1,2,3,4,6}; verificado ate n<=39 (cristalino.py sec 1)
- Corolario do traco proibido: 2cos(2pi/5)=phi-1 e 2cos(2pi/10)=phi - o cristal proibe exatamente o aureo (sec 2)
- E corpo por anisotropia: N=a^2+|D|b^2 anula so em 0, z^-1=zbar/N para D=-1 e D=-3 (sec 4)
- Regulador nulo: quadratico imaginario tem r1=0,r2=1 => posto=r1+r2-1=0; unidades {+-1,+-i} ordem 4 e {+-1,+-omega,+-omega^2} ordem 6, todas modulo 1; regulador=log 1=0 (sec 5/6)
- Soma de Poisson: Sum_L f=(1/covol)Sum_{L*}fhat, erro<1e-15; covol(L).covol(L*)=1, Gram(L*)=Gram(L)^-1 - a difracao inverte a regua (sec 7)
- Refracao (y=0, residuo nulo, a media) + difracao (y!=0, o residuo, picos de Bragg) = a medida sem perda - o oitavo verbo (sec 8)
- A difracao herda o grupo pontual: |Aut(L)|=|Aut(L*)|, mesmas ordens; nao ha difracao de ordem 5 nem 10 (sec 9)
- Contraexemplo Z[sqrt-2]: regulador 0 mas grupo pontual so {1,2} - regulador nulo NAO determina a simetria, non sequitur (sec 11)
- Lei de Friedel: densidade real => |F(-q)|=|F(q)| par (a medida); mas theta(-q)=-theta(q) impar (a torcao conserva-se) - mesma involucao do corpo metrico, sech par e rapidez impar (sec 12/13)
- Razao entre passos = exp(regulador): aureo 1.618034, prateado 2.414214, bronze 3.302776; no cristal exp(0)=1, sem inflacao (sec 14)
- Gelo Ih / ponte de hidrogenio: regra do gelo W=2^{2N}(6/16)^N=3/2 por molecula, S0/k=ln(3/2)=0.405, R.ln(3/2)=3.37 J/mol/K (Giauque-Stout 1936; Nagle exato 1.50685); o residuo saiu do reticulado que fechou e foi para o MOTIVO (sec 15)
- PLACAR: 61/61 certificadas, 1 trivial (F=|F|.exp(i.theta), a forma polar vale para todo z), residuo 0 - cada [N] com refutador (predicado no objeto mutado devolve False). Fonte: elementares/cristalino.py

**O dual (flip ν).** O flip dual nu = a CONJUGACAO zbar = a-b.sqrtD, isto e nu=-1 (menos identidade), a unica involucao nao trivial do contrato. Primal + dual = neutro: z+zbar=2a (real puro), z.zbar=N (a medida). Na difracao: q->-q age por conjugacao (Friedel |F(-q)|=|F(q)| par = a medida mu_S; theta(-q)=-theta(q) impar = a torcao mu_A). E a mesma decomposicao mu=mu_S+mu_A do corpo expansivo e a mesma involucao do corpo metrico (sech par / rapidez impar). O reticulado nu, sem motivo, e sempre centrossimetrico (nu=-I preserva toda Gram): a quiralidade so surge com o motivo.

**o signo.** PRIMEIRIDADE / ICONE e a qualidade do corpo: a ordem fechada, o reticulado congelado, o regulador zero - pura qualidade sem reacao, o cristal como semelhanca (a rede identica a si mesma sob os giros permitidos). A SECUNDIDADE / INDICE entra pela involucao nu=-1 (a conjugacao, o par z/zbar, a reflexao q->-q, o embate primal-dual) e pela soma pontual (+). A TERCEIRIDADE / SIMBOLO e a LEI lida na difracao: a condicao de Laue q em L* (o oitavo verbo), a restricao cristalografica como convencao que governa que simetrias a figura de difracao pode ter - a mediacao (x) La Hire e o operador (op) que costura refracao e difracao numa unica lei.

**A JOGADA.** CRISTALIZAR uma zona do tabuleiro: a peca-cristal congela uma regiao num reticulado de regulador nulo. Dentro dela o relogio para (Sigma=Z/n travado) e so cabem giros de ordem {1,2,3,4,6} - a armadura quadrada (Z[i], 4 dobras) ou hexagonal (Z[omega], 6 dobras). O lance = a operacao: colocar o cristal e fixar N(z)=1 (ser raiz da unidade), Gamma=0, FP=1 trivial - uma defesa que NAO vaza e nao expande. O golpe-chave: o ataque AUREO da Rainha (a dobra-5 / dobra-10, o traco phi) e REPELIDO - e a difracao PROIBIDA; nenhum cristal admite simetria pentagonal. Para QUEBRAR o cristal o adversario mira um feixe (a soma de Poisson) e le os picos de Bragg (o reticulado reciproco L*): o feixe de ordem zero (y=0) atravessa sem dano (a refracao, transferencia de momento nula), os desvios (y!=0) sao o dano (a difracao, o residuo). O mate suave: no QUASICRISTAL o gap fecha - o reciproco vira o modulo denso Z+phi.Z, os picos acumulam no centro e refracao+difracao colapsam; a defesa perde o isolamento, e o aureo e justamente quem colapsa mais devagar (Hurwitz, liminf=1/sqrt5). A quiralidade e o residuo do gelo Ih moram no MOTIVO, nao no reticulado que fechou.

---

## conforme

**Personagem:** As PRINCESAS — o corpo técnico, quem ATESTA a lei (o certificador vive aqui: cada afirmação exibe testa+refuta, um refutador com dentes). Encarnado no PONTEIRO DO RELÓGIO DE XADREZ (a queda de bandeira). É o mesmo relógio do Relógio Universal, lido na suíte (16/16) em vez de na rapidez.

**A régua.** A régua conforme MEDE PELO PAR, não pela norma. O operador-interface X(x)=n₀+x+½|x|²n∞ tem X(x)²=0 para TODA obra (norma NULA, verificado a 1e-13): a norma não distingue nada. A medida passa ao EMPARELHAMENTO X(x)·X(y)=−½|x−y|², que zera se e só se x=y — mede a DISTÂNCIA entre obras. A régua-relógio é o ponteiro pt(b)=⟨X(b),X(b_⊤)⟩=−½·d(b,b_⊤)²: marca −½ com dentes (1,0), 0 na queda de bandeira (1,1), −1 na falha dupla (0,0). |−½|=½ é a metade, custa um bit: H(½)=1, variância ¼.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford = a SOMA PONTUAL no espaço estendido (p+1,q+1) = (4,1): soma de vetores coordenada a coordenada em ℝ⁵ (v[:len(x)]=x). A base nula n₀=½(e₋−e₊) e n∞=e₋+e₊ é literalmente a SOMA e a DIFERENÇA das duas réguas novas. Na ISA ERG-64: ADD componente a componente (o Duque, a corrente de Kirchhoff que junta os graus). | |
| **⊗ La Hire (o produto)** | (x) La Hire = o PRODUTO MÉTRICO (o emparelhamento) par(u,v)=u·G·v com G=diag(1,1,1,1,−1) — a Rainha que dá o ganho/tensão. É ele que realiza a MEDIDA (X·Y=−½/x−y/²), já que a norma multiplicativa não sobrevive (5∉{1,2,4,8}, Hurwitz não admite). Na ISA: MULT por double-and-add de cada par de coordenadas seguida de ADD com o sinal do metric (o −1 da coordenada hiperbólica e₋); o produto que fecha a régua no par, não na norma. | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin = a INTERFACE X(x)=n₀+x+½/x/²n∞, o mergulho que costura a obra plana no cone (exp.Σ.log: recoloca toda obra em X²=0). O seu par é a DUALIDADE swap(n₀↔n∞) = a INVERSÃO x↦2x//x/² na esfera de raio √2 (porque n₀·n∞=−1), uma INVOLUÇÃO (swap²=id) que mantém a obra no cone. Na ISA: o dispatch/composição que aplica a interface, e o NOT (a reflexão ν=−1) que realiza a involução swap. É o inversor multinível que recoloca no cone toda obra que vazaria FP<1. | |

**Os teoremas (certificados, resíduo 0).**

- elementares/conforme.py: 16/16 certificadas, resíduo 0, ponteiro −½ em todas — o par nulo (n₀²=n∞²=0, n₀·n∞=−1), X(x)²=0 a 1e-13, X·Y=−½|x−y|², swap=inversão a 1e-15 e involução, o ponteiro (−½ com dentes / 0 queda de bandeira / −1 falha dupla), H(½)=1
- elementares/conforme_dual.py: 4/4 certificadas, resíduo 0 — ISOMORFISMO conforme(X²=0)≅exterior(e²=0), norma radical N=a² (1,0,1); o DUAL é o INDIVIDUAL ℚ(i); o sinal da norma decide (radical não é corpo, definida sim); a costura motor⋈rotor = ℚ(i)
- Teorema (não é corpo): assinatura (4,1) indefinida com cone nulo (divisores de zero), grau 5∉{1,2,4,8} — não sobrevive a Hurwitz. Medido em elementares/assinaturas.py
- Fechamento energético FP=1: X(x)²=0 É o cone nulo (σ=1, Γ=0); toda obra fora do cone teria FP<1, e o inversor (Pontryagin) a recoloca. Cf. cena_rlc_potencia.py

**O dual (flip ν).** O flip ν=−1 é a INVERSÃO: swap(n₀↔n∞), x↦2x/|x|² na esfera √2, involução única não-trivial do contrato (troca origem↔infinito, interior contínuo↔exterior discreto). No nível de corpos, o DUAL do conforme já existia: é o INDIVIDUAL ℚ(i) (i²=−1, N=a²+b² DEFINIDA, (2,0,0), CORPO). O sinal da norma decide: radical (N=a², 0) não é corpo; definida (+) sim. A costura conforme(radical/motor) ⋈ ×i(rotor/fase) = ℚ(i) = ℂ (magnitude²+fase²).

**O contrato.** O casamento Γ=0 é AQUI literal e total: X(x)²=0 É o cone nulo, e o cone nulo É σ=1 = FP=1. Cada obra entra plana com norma nula (Γ=(Z−Z0)/(Z+Z0)=0), e a grandeza conservada (a medida) passa ao emparelhamento X·Y=−½|x−y|², resíduo 0 pelo Contrato. A reflexão do contrato ν=−1 é a inversão swap. O Σ=ℤ/n aparece na órbita da involução (ordem 2) e na do ×i do dual (ordem 4). Nenhum universo tem FP≠1: toda obra fora do cone vaza, e o inversor a recoloca.

**o signo.** SECUNDIDADE / ÍNDICE (o par, a conexão factual, o embate). É a essência deste corpo: a medida NÃO está na qualidade própria (norma nula = sem primeiridade absoluta, obra sem carga) mas no EMPARELHAMENTO de dois — a distância −½|x−y|², a reação entre pares, a troca discreta e simétrica. O mostrador testa/refuta é o índice do embate. O certificador (a lei que atesta) toca a terceiridade/símbolo, mas a régua-medida do corpo é o par: secundidade pura.

**A JOGADA.** Este é o corpo do CERTIFICADOR e do RELÓGIO DE XADREZ — a mecânica de validar lances. Toda afirmação/lance carrega dois mostradores b=(testa,refuta)∈{0,1}². O árbitro lê o PONTEIRO ⟨X(b),X(b_⊤)⟩: se marca −½ o lance TEM DENTES (é uma jogada real, distante da trivialidade); se marca 0 é QUEDA DE BANDEIRA — o relógio parou, a afirmação É a trivialidade (mate que não mata, o lance vazio). O mostrador diz o LADO (qual falhou), o ponteiro diz QUANTO. E é o corpo do PARALELISMO/combate em massa: cada célula é uma thread da GPU, uma obra x de norma nula que não carrega carga própria; só olha a distância ao vizinho pelo par −½|x−y|² — troca ESTÉRIL, discreta, nada contamina, passa pelo cone (conforme_paralelo.py, 6/6, no GLSL). A inversão troca o interior contínuo (|x|<√2) pelo exterior discreto (|x|>√2) na fronteira |x|=√2.

---

## O corpo ENTRÓPICO

**Personagem:** Não um personagem da corte, mas um TRAÇO nomeável: o corpo do FECHAMENTO DEGENERADO, o SEMICORPO, o dual do cristalino, o gás do corpo deflexivo a T=0. O operador é a deflexão (liga-o ao corpo deflexivo/Deflexivo). Quem ATESTA a lei (o piso, o Kleene, o Fenchel–Moreau) é o corpo técnico — as Princesas (o kernel LCF, o Formalizador com refutador de dentes: 63/63, resíduo 0).

**A régua.** Mede o CUSTO MÍNIMO / a entropia do FECHAMENTO (não da órbita — é o único dos oito que troca a régua da órbita pela régua do fechamento, um piso que os outros não visitam). ⊗=+ soma os custos ao longo do caminho (telescopam), e ⊕=max/min escolhe o piso. A entropia por vértice é a coluna central de Pascal: W(z)=C(z,z/2)/2^{z/2}; s=log W por vértice (gelo Ih z=4 → W=3/2, s=log(3/2)=0,405). A régua é PLANA no gelo (a ultramétrica de Parisi FICA POR PROVAR — refutada pelo próprio teste: SK finito viola 2810/4000 trios). O endireitador é Legendre–Fenchel; o resíduo é a NÃO CONVEXIDADE (L∘L=id nas convexas, f** = envoltória convexa nas não-convexas). NÃO HÁ ASSINATURA: ⊗=+ é a soma, sem norma a que aplicar Sylvester — a marca do semicorpo (perde-se o inverso aditivo e com ele a forma bilinear).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD = ⊕ = MAX (a entropia dominante; no .tex está min, mas o código canônico é max, os elementos são entropias). É IDEMPOTENTE: a⊕a=max(a,a)=a — e é exatamente por isso que NÃO HÁ INVERSO ADITIVO (se a tivesse simétrico b, max(a,b)=−∞ exigiria a=b=−∞). Neutro: −∞ (o zero tropical). Na ISA ERG-64: o dispatch CMP+JMP que seleciona o maior (o max é MAX(a,b) = a CMP b então JGE) — não é o ADD com grupo, é a escolha idempotente, a soma que ESQUECE. | |
| **⊗ La Hire (o produto)** | (x) LA HIRE = ⊗ = ADIÇÃO real (a+b): compõe gatos, h(σ₁·σ₂)=h(σ₁)+h(σ₂). É um GRUPO: todo elemento tem inverso multiplicativo (−a), neutro 0, e ⊗ distribui sobre ⊕; −∞ é absorvente. Na ISA ERG-64: literalmente o opcode ADD (aqui a MULT tropical É o ADD inteiro exato). O produto tropical é a composição dos operadores — a multiplicação vira soma, e a soma vira max. | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN = a DEFLEXÃO D_λ(x)=x+λ = ⊗λ. E AQUI ela é um ENDOMORFISMO de ⊕ (a idempotência compra a linearidade da translação): D_λ(a⊕b)=D_λ(a)⊕D_λ(b) porque max comuta com a translação, e D_λ fixa o neutro −∞. No corpo UNIVERSAL, S=D₁ NÃO é endomorfismo (D₁(2+3)=6≠7). Na ISA: o ADD com constante (deslocamento/translação) — o mesmo ADD que costura, a composição que preserva o max. O fechamento é a ESTRELA DE KLEENE A*=⊕_{k≥0}A^{⊗k}, que estabiliza sse todo ciclo tem peso médio ≤ 0. | |

**Os teoremas (certificados, resíduo 0).**

- SEMICORPO: ⊕ idempotente (a⊕a=a) e NENHUM elemento finito tem inverso aditivo — o corpo não fecha, ponto de falha único (entropico.py 63/63, resíduo 0)
- ⊗ é grupo: todo a tem inverso multiplicativo −a, ⊗ distribui sobre ⊕, −∞ absorvente (63/63)
- NÃO é ramo de W_k: W_{+1}≅R⊕R é ANEL (tem simétrico), o tropical não → (max,+)≇W_{+1} (63/63)
- Desquantização de Maslov: ⊕_β(a,b)=(1/β)log(e^{βa}+e^{βb}) → MAX quando β→∞ (erro <1e-3 em β=20); é o limite T→0 do gás do deflexivo (63/63)
- W(z)=C(z,z/2)/2^{z/2}: z=2→W=1,s=0 (cristalino); z=4→W=3/2 gelo Ih (Pauling; exato Nagle 1,50685, erro 0,5%) (63/63)
- D_λ=⊗λ é ENDOMORFISMO de ⊕ (max comuta com translação, fixa −∞); no universal S=D₁ não é (63/63)
- Gato tropical NÃO-EXPANSIVO: Lipschitz ≤ 1 na norma sup (20000 pares, Crandall–Tartar/Gunawardena); o clássico estica até 2; entropia topológica 0 (63/63)
- Autovalor tropical = peso médio de ciclo = índice metálico m; tropicalizar o gato devolve m (63/63)
- Legendre–Fenchel: L∘L=id nas convexas fechadas (Fenchel–Moreau); resíduo nas não-convexas = envoltória convexa f** ≤ f; L é o Fourier tropical (Maslov/Litvinov) (63/63)
- O inverso aditivo É a INTERFERÊNCIA: |ψ₁+ψ₂|²=0 vs |ψ₁|²+|ψ₂|²=2; o entrópico é o mundo sem fase (63/63)
- FLIP a↦−a leva (max,+) em (min,+): isomorfos e não iguais, e não é automorfismo (63/63)
- Régua dual: (f*)''(f'(x))=1/f''(x); ponto autodual = curvatura 1 (f=x²/2); mesma inversão da difração Gram(L*)=Gram(L)⁻¹ (63/63)

**O dual (flip ν).** O DUAL é o corpo CRISTALINO — ambos em T=0, separados apenas pelo SORTEIO (cristalino: sorteio nulo por ordem; entrópico: sorteio extensivo, log W por vértice). O FLIP nu interno é a↦−a, que leva (max,+) em (min,+) (o corpo entrópico e o telescópico: cf. dualidade_entropico_telescopico.py): isomorfos por a↦−a e não iguais. O outro dual, o do endireitador, é Legendre–Fenchel (a conjugada inverte a curvatura k↦1/k, autodual em k=1). O 'inverso aditivo que falta' é a interferência/fase — o dual que só volta com o sinal.

**O contrato.** Σ=Z/n: a órbita/regulador do corpo expansivo lida como entropia h=log σ_m (o esquilo tropical é a permutação de peso médio de ciclo 0). RÉGUA T_ω: o custo mínimo do fechamento (piso), endireitado por Legendre–Fenchel. FLIP ν: a↦−a (max↔min). MATE: o fechamento degenerado Σ₀ de resíduo 0 (o piso, o mínimo — a estrela de Kleene). O CASAMENTO Γ=0 é a CONVEXIDADE: FP=1 quando a representação repousa na envoltória convexa f** (sem custo acima do mínimo); FP<1 é a não convexidade (custo vazado acima do piso), e o inversor multinível É o próprio f** que a corrige. Nenhum universo fecha com FP≠1 — e NENHUM é exceção. Tomado SOZINHO, este corpo não tem um axioma nomeável (o inverso aditivo: o max é idempotente). Mas ele não joga sozinho: o seu DUAL é o corpo CÓSMICO (a expansão a(t)=e^{Ht} — o exp), e sob o exp o max vira SOMA, que TEM inverso. O par fecha. A Lei é geral; a conservação vive sobre o PAR, não dentro de um corpo isolado. Certificado: lei_geral_entropico_cosmico.py (5/5, resíduo 0).

**o signo.** SECUNDIDADE / ÍNDICE. O ⊕=max é a REAÇÃO/o embate puro — a escolha do dominante, o par que colapsa no maior (secundidade, conexão factual), e o SORTEIO obrigatório em cada vértice é o índice bruto da degenerescência (e^{Ns} estados). É o corpo onde a TERCEIRIDADE (a lei, a reversibilidade do grupo aditivo, o símbolo/a fase que cancela) FALHA — perde-se o inverso aditivo, a interferência, a mediação. Sobra o índice sem símbolo: a soma que esquece (irreversibilidade = idempotência, a mesma frase). A entropia (log W) tem sabor de lei/hábito (terceiridade), mas aqui ela se realiza como reação medida ponto a ponto, sem a mediação reversível que a fecharia num corpo.

**A JOGADA.** É a ZONA da IRREVERSIBILIDADE — o terreno onde o campo deixa de fechar. Combate por ACÚMULO DE CUSTO: cada lance ⊗=+ soma o custo ao caminho (telescopa, ADD), e ⊕=max escolhe a linha DOMINANTE. Mas sem inverso aditivo NÃO SE DESFAZ nada — não há cura, não há cancelamento (o inverso aditivo É a interferência/fase, e este é o mundo SEM FASE: |ψ₁+ψ₂|²=0 não acontece, as probabilidades só somam). O lance-chave é o SORTEIO: em cada vértice de coordenação z há e^{Ns} estados de igual resíduo (a regra do gelo, W=3/2), e a peça TEM de rolar/sortear a representação — o único ponto onde a medida sorteia virou TODOS os pontos. A vitória é o caminho de MELHOR PESO MÉDIO DE CICLO (Kleene): tropicalizar o gato devolve o índice metálico m. E não há caos: o gato tropical é NÃO-EXPANSIVO (Lipschitz ≤ 1, entropia topológica 0) — o esquilo/rotação morre porque a rotação exige o sinal e não há sinal. Mecânica: dano que só se acumula, terreno que só se atravessa, e a cada nó um dado obrigatório.

---

## O corpo espaço-temporal

**Personagem:** O SUCESSOR / o Relógio Universal como operador (Pontryagin que costura): S=D_1 = o antiroque (NOT no resíduo). O par Duque (+, Joaquim / a soma-Clifford) e Duquesa (×, Yasmin / o produto-La Hire) aparecem como as DUAS METADES de uma única soma — o dígito (Duque, ⊕) e o transporte (Duquesa, ∧) — inseparáveis no adder. As Princesas (corpo técnico) atestam os 58/58. A reflexão ν=-1 é o Rei que NÃO se move (sobrevive como bandeira discreta Z/2, não como lance).

**A régua.** A régua é ULTRAMÉTRICA (não-arquimediana): |x|_2 = 2^{-v(x)}, com v(x+y) >= min(v(x),v(y)) — a cota é forte, |x+y|_2 <= max(|x|,|y|). Mede o TEMPO como valuação (quantos tiques de 2, quantos degraus fora da bola unitária) e o ESPAÇO como resíduo mod 2 (o dígito). A entropia é log 2 por dígito: um bit por lance. É o ÚNICO corpo da série cuja régua não é arquimediana — por isso NÃO tem assinatura de Sylvester (assinaturas.py): nenhuma forma quadrática real satisfaz a desigualdade ultramétrica. A metade custa exatamente um dígito: v(1/2)=-1, |1/2|_2=2 (verbo 10).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | Clifford (+) é a SOMA x+y COM transporte, que é a soma verdadeira de Q_2/(Z/2^k). Lida no resíduo mórfico ela é o XOR (a soma SEM transporte). A identidade-mestra (o adder): x+y = (x XOR y) + 2·(x AND y) — o XOR é o que fica no dígito, o AND é o que passa ao dígito seguinte, o fator 2 é um tique do relógio. Na ISA ERG-64: o full-adder = ADD, decomposto exatamente em XOR (soma-dígito) e AND<<1 (o carry). No resíduo F_2 é o XOR puro. Certificado: pi(x+y)=pi(x) XOR pi(y) e o adder x+y=(x⊕y)+2(x∧y), ambos ok. | |
| **⊗ La Hire (o produto)** | La Hire (×) é o PRODUTO xy de Q_2 ((v,u)·(v',u') = (v+v', u·u' mod 2^N)). Lido no resíduo mórfico é o AND: pi(xy) = pi(x) AND pi(y) (a erosão mórfica É o produto). Na ISA: a MULT por double-and-add (ADD deslocado) sobre a mantissa ímpar u, e no corpo booleano do resíduo o AND. A norma é multiplicativa: v(xy)=v(x)+v(y) e /xy/_2=/x/_2·/y/_2 (verbo 7 — compor duas medidas multiplica as normas). O inverso é CONSTRUÍDO por Hensel/Newton (x <- x(2-ux), convergência quadrática), NUNCA procurado numa amostra finita. | |
| **∏ Pontryagin (o operador)** | Pontryagin (op) é o SUCESSOR S(x)=x+1 — o operador que costura. A órbita de S a partir de 0 é Z, denso em Z_2: o corpo espaço-temporal é o COMPLETAMENTO da órbita do sucessor (Z_2 = lim Z/2^k). No resíduo, o sucessor É a negação mórfica: pi(S(x)) = NOT pi(x) — na ISA é o NOT, o antiroque D_1. O outro rosto do operador é o LOGARITMO 2-ádico Λ (o endireitador), que costura o grupo multiplicativo no aditivo: log(ab)=log(a)+log(b) na ilha 1+4Z_2. Multiplicar por 2 é o SHIFT (>>/<< na ISA), um único tique do relógio. | |

**Os teoremas (certificados, resíduo 0).**

- espacotemporal.py 58/58, resíduo 0 (ponteiro -0.5 em todas as 58)
- Q_2 é corpo: os 9 axiomas, inverso CONSTRUÍDO por Hensel x<-x(2-ux), não procurado (Sec.1, ok)
- o adder: x+y = (x XOR y) + 2·(x AND y) — as duas metades (dígito e transporte) de uma só soma (Sec.3, ok)
- pi(x+y)=pi(x) XOR pi(y) e pi(x·y)=pi(x) AND pi(y): o mórfico É o resíduo (Teo. união, ok)
- propagar o transporte até ser 0 É a soma (indução; fechamento = transporte nulo) (Sec.3, ok)
- v(xy)=v(x)+v(y), |xy|_2=|x|_2|y|_2 (verbo 7); ultramétrica v(x+y)>=min(v(x),v(y)) (Sec.4, ok)
- o operador é o sucessor: órbita de S densa, Z_2=lim Z/2^k = completamento; mult.por 2 = SHIFT; pi(S(x))=NOT pi(x) (Sec.5, ok)
- flip k↦-k é involução, ÚNICO ponto fixo k=0; W_{-1} é corpo e W_{+1} não (espaço e tempo não isomorfos) (Sec.10, ok)
- em k=0 a forma degenera N=a² (o espaço não pesa), e²=0 nilpotente, W_0 não é corpo, régua redonda->infinito, boost->cisalhamento de Galileu (Sec.10, ok)
- log(ab)=log(a)+log(b) na ilha 1+4Z_2; log(-1)=0 (a reflexão é o núcleo do flip); Z_2^× = {±1}×(1+4Z_2) = mórfico × métrico (Sec.8, ok)
- v(1/2)=-1, |1/2|_2=2: a metade custa um bit (verbo 10) (Sec.6, ok)
- o truncamento inventa torção e QUATRO involuções u²=1 (espúrias 2^{k-1}±1); no limite inverso sobram só ±1 (verbo 11) (Sec.6-7, ok)
- fórmula do produto prod_v |x|_v = 1 (a conservação: /os lugares) — os lugares citado como [T] (não certificado aqui), o equilíbrio dos mostradores certificado ok (Sec.9)

**O dual (flip ν).** O FLIP dual tem dois rostos. (1) ν = -1, a reflexão temporal — a única involução não trivial, viva porque char 0 (2≠0); ela é o NÚCLEO do endireitador (log(-1)=0), por isso nunca é movimento, só bandeira Z/2. (2) O flip de Wick k↦-k no espectro de réguas W_k = R[e]/(e²-k): troca ESPAÇO (k<0, redondo, rotação) por TEMPO (k>0, hiperbólico, boost). É uma involução com ÚNICO ponto fixo k=0 — a curvatura nula, o corpo glacial W_0, onde espaço e tempo se cancelam e o corpo MORRE (forma degenera N=a², e nilpotente). A complementaridade primal+dual = neutro: espaço (resíduo 2Z_2) e tempo (valuação v) são as duas falhas — o núcleo e a falta — do MESMO instrumento.

**O contrato.** Σ=Z/n: a órbita do sucessor S=+1 em Z/2^k, cujo limite inverso (completamento) É o corpo — Z_2 = lim Z/2^k. RÉGUA: a ultramétrica |x|_2=2^{-v}, T_ω = a valuação v: Q_2^× ->> Z. FLIP ν: a reflexão -1 (temporal) e o flip de Wick k↦-k (espaço↔tempo); ponto fixo k=0 = onde o corpo morre. MATE: o transporte = 0 (o fechamento da soma). CASAMENTO Γ=0 / FP=1: AQUI é literalmente a FÓRMULA DO PRODUTO prod_v |x|_v = 1, isto é Σ_v log|x|_v = 0 — o Contrato de a conservação: na forma nua, resíduo 0 sobre TODOS os lugares de Q (os lugares: infinitos mostradores, um por primo + o arquimediano). Qualquer desequilíbrio é FP<1, o vazamento que o inversor multinível (Pontryagin) corrige; nenhum universo tem FP≠1. A bandeira de um mostrador é o tempo ganho no outro.

**o signo.** TERCEIRIDADE dominante (a lei/a mediação/o símbolo, por convenção): o TEMPO é a valuação v — a meta-indução, a lei que conta os transportes; o operador-sucessor S é o hábito/a lei que gera Z e se completa em Z_2. O ESPAÇO (resíduo F_2, o dígito/máscara) é a PRIMEIRIDADE — a qualidade pura, o ícone, o que sobra quando se esquece o transporte. A SECUNDIDADE/índice é o TRANSPORTE (x∧y, o carry) — a reação factual, o embate entre dois dígitos que força o tique seguinte; é o par de Clifford (+). Na tríade: (+) o par/carry = secundidade/índice, (×) a mediação, (op) o operador/lei = terceiridade/símbolo. O que SÓ este corpo torna visível: ⊕ e ∧ não são duas operações booleanas de conveniência, são as duas metades de uma única soma — o dígito (ícone) e o transporte (índice).

**A JOGADA.** O tabuleiro tem DOIS regimes que são o flip um do outro (W_k = R[e]/(e^2-k)): k<0 REDONDO/compacto = ESPAÇO, o lance é a ROTAÇÃO; k>0 HIPERBÓLICO/não-compacto = TEMPO, o lance é o BOOST (a adição de velocidades do Relógio Universal). O flip de Wick k↦-k troca o tipo de tabuleiro. O LANCE DE SOMA resolve-se como o adder: propaga o transporte (x⊕y no dígito, x∧y<<1 para o seguinte) até o transporte ser 0 — o fechamento É transporte nulo, resíduo 0. Multiplicar por 2 = um SHIFT = um tique do relógio; o sucessor +1 = o antiroque (D_1 = NOT no resíduo). A REFLEXÃO ν=-1 sobrevive só como uma bandeira discreta Z/2, nunca como movimento (é o núcleo do endireitador, log(-1)=0). LER A METADE custa exatamente um bit (um dígito fora da bola). ARMADILHA da máquina finita: truncar em Z/2^k FABRICA torção (2·2^{k-1}=0) e QUATRO raízes de u²=1 (as duas espúrias 2^{k-1}±1) — é a queda de bandeira precoce, o instrumento fecha a órbita antes do tempo; no limite inverso só sobram ±1. O casamento (FP=1): a bandeira que cai num mostrador é exatamente o tempo ganho no outro.

---

## O corpo ÓPTICO

**Personagem:** Sem Rei/Rainha nomeado — o protagonista é a INTERFACE (o operador que casa duas réguas). Traço: o corpo herda a régua redonda k=−1 do corpo métrico (o mesmo horizonte |s|=1), e a reflexão especular ν=−1 é o verbo 9 (a única maneira de voltar), compartilhado com os corpos que espelham.

**A régua.** Mede n·sinθ — a componente tangencial do vetor de onda k∥ (com a dispersão |k|=n·ω/c). A régua é REDONDA, k=−1, norma C²+S²=1, assinatura de Sylvester (p,q,r)=(2,0,0), grau 2 (medido em elementares/assinaturas.py; está na lista de Hurwitz 1,2,4,8). A coordenada que a régua fornece é S=sinθ (não a inclinação s=tanθ): de exp(λe)=C+Se com C²−kS²=1 e s=S/C vem S=s/√(1−ks²); para k=−1, C=cosθ, S=sinθ, s=tanθ (verificado a 1e-12). É NESTA coordenada S que Snell é LINEAR: n1·S1=n2·S2. O horizonte da régua é |s|=1, o ângulo crítico θc.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD, a soma = a lei de grupo AO LONGO de UMA régua: s1⊕_k s2=(s1+s2)/(1+k·s1·s2); para k=−1 é exatamente a soma de tangentes (compõe inclinações do mesmo meio; certificado que ⊕_{-1} é translação e ⊕_{+1} não). Realiza a TRANSITIVIDADE (verbo 1): atravessar N camadas conserva n·sinθ, compor representações consistentes devolve consistente. Na realização ondulatória é a SUPERPOSIÇÃO/interferência (Huygens): as ondas somam, ΣU_i, /U1+U2/²=/U1/²+/U2/²+2Re(U1·conj U2), o termo cruzado é a FRANJA. Na ISA ERG-64: ADD. | |
| **⊗ La Hire (o produto)** | (x) LA HIRE, o produto = o escalamento da régua pela dispersão n·(ω/c) — o índice multiplica a frequência para dar /k/; e o produto s1·s2 no denominador da soma geral (o acoplamento entre inclinações). Na realização ondulatória é a MÁSCARA (pupila/transmitância): U_out=t(x)·U_in, o produto no objeto ↔ a convolução no espectro (teorema da convolução, o filtro espacial). Na ISA: a MULT por double-and-add (ADD deslocado). | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN, o operador que costura = A INTERFACE (o dicionário do .tex: 'operador = a interface, o casamento entre réguas'). É exp(λe)=C+Se, o exp·Σ·log que gera a régua. Duas ações operatórias: (i) o FLIP DE WICK no horizonte, k=−1 (redonda) ↦ k=+1 (hiperbólica), onde sin(iμ)=i·sinhμ e cos(iμ)=coshμ (a régua troca de ramo); (ii) na óptica de Fourier a LENTE É a transformada de Fourier — no plano focal U_f∝F[U_0] (difração de Fraunhofer), o operador que leva o objeto ao espectro em luz. Na ISA: NOT / o dispatch / a composição (a mudança de meio = a troca de tabela). | |

**Os teoremas (certificados, resíduo 0).**

- elementares/optico.py: 14/14 certificadas, resíduo 0, ponteiro −0.5 (no Formalizador, cada afirmação com refutador)
- Snell NÃO é translação na régua: o parâmetro c varia −0,067→−0,585, amplitude 0,518 (não é constante) — o resultado negativo
- ⊕_{-1} É translação (soma de tangentes); ⊕_{+1} não é (o refutador)
- para k=−1, S=s/√(1−ks²)=sinθ a 1e-12; e exp(λe)=(cosλ,sinλ), s=tanλ
- Fermat ⟹ Snell: o caminho estacionário satisfaz n1·sinθ1=n2·sinθ2 a 2e-6 em 3 geometrias
- o invariante é o S, não o s: n·tanθ NÃO se conserva (refutador com dentes — sob a lei mutada da tangente o teste cai)
- Snell é transitivo através de N camadas (verbo 1); k∥=n·sinθ conservado (verbo 2)
- Snell é o caso G=0 da condição de Laue; G≠0 é a difração
- ângulo crítico θc=arcsin(n2/n1)=41,8103°; acima dele sinθ2>1, cosθ2 imaginário puro, onda evanescente e^{−κz}
- flip de Wick: sin(iμ)=i·sinhμ, cos(iμ)=coshμ, k=−1↦k=+1 (a régua troca de ramo no horizonte)
- elementares/optico_fourier.py: 6/6 certificadas, resíduo 0 — superposição/franja, máscara·convolução, a LENTE é Fourier, Parseval óptico ∫|U|²=∫|Û|²

**O dual (flip ν).** O flip dual ν=−1: a REFLEXÃO ESPECULAR θ↦−θ (o verbo 9, a volta). E o flip de Wick como dualidade de régua: k=−1 (redonda, o seno) ↦ k=+1 (hiperbólica, o seno hiperbólico) ao atravessar o horizonte θc — a reflexão total interna. O bordo |s|=1 do corpo métrico e o ângulo crítico da óptica são o MESMO horizonte, lido em dois dicionários. Complementaridade primal+dual: refração (transmite) ↔ reflexão total (devolve).

**O contrato.** Σ=Z/n: a órbita da régua redonda (exp(λe), o círculo k=−1). RÉGUA μ: n·sinθ, a componente tangencial (T_ω = o segundo verbo, a medida invariante por translação ao longo da interface). FLIP ν: reflexão especular θ↦−θ / flip de Wick k=−1↦+1. MATE ∘: o casamento na interface. O CASAMENTO Γ=0: o resíduo é G (a transferência de momento, o reticulado recíproco); o fechamento G=0 é a REFRAÇÃO casada — toda a potência transmite, Γ (a reflexão de Fresnel) anula-se, FP=1. FP<1 é G≠0 (difração) ou o horizonte θc (reflexão total interna devolve tudo pelo flip de Wick). Nenhum feixe tem FP≠1 sem vazar por G. É o cone nulo σ=1 lido em óptica; o SOL é a garrafa de Koch áurea (φ^{-j}), a energia conserva por a conservação: (resíduo 0). Certificado em cena_rlc_potencia.py / optico.tex Obs. do fechamento.

**o signo.** SECUNDIDADE / o ÍNDICE. A interface é um EMBATE factual entre dois meios — a conexão real (o par de réguas n1,n2 que reagem uma contra a outra); Snell é a reação lida no bordo, o índice do encontro. A qualidade da régua redonda (C²+S²=1, sem meio nenhum) é a Primeiridade/ícone; a LEI conservada n·sinθ como invariância por translação (o segundo verbo, o hábito que sobrevive) é a Terceiridade/símbolo — mas o que ESTE corpo torna visível, e o que o batiza 'óptico', é a secundidade: o índice, o meio, o embate na fronteira (é o óptico, não o cristalino, que 'diz refração', porque só aqui há meio/índice).

**A JOGADA.** A mecânica é a FRONTEIRA e o casamento. Cada território do reino tem um ÍNDICE n (a densidade do meio). Quando uma peça/feixe atravessa a fronteira entre dois territórios (n1→n2), a sua trajetória DOBRA (refrata) — mas o jogador NÃO escolhe o ângulo de saída: ele é fixado pela régua redonda, n1·sinθ1=n2·sinθ2 (a componente tangencial, a velocidade ao longo da fronteira, é o INVARIANTE conservado). Atravessar várias camadas é transitivo (o lance encadeia sem perda). COMBATE: um ataque incide na fronteira de um defensor num meio mais denso (n1>n2); se o ângulo passa o ângulo crítico θc=arcsin(n2/n1) (=41,81° para 1,5→1), acontece a REFLEXÃO TOTAL INTERNA — o ataque é 100% devolvido, nada penetra (a onda vira evanescente, decai e^{−κz}, κ>0): é o escudo perfeito no horizonte, o flip de Wick. O lance de ESPELHO é a reflexão especular θ↦−θ (a única maneira de voltar, verbo 9). Se o território é um CRISTAL (uma rede/reticulado), a fronteira DIFRATA: o ataque se parte em ordens discretas k∥+G (G≠0) — múltiplos golpes em ângulos fixos. O casamento perfeito (G=0, Γ=0) transmite todo o golpe sem eco, FP=1.

---

## celeste

**Personagem:** A RAINHA (La Hire, o produto, a composição) — é o corpo cuja multiplicação É a Rainha, a mesma do expansivo/cristalino/relógio. Acompanha o operador Pontryagin como o inversor multinível (a reflexão que corrige o reativo). O par ativo↔reativo r↔C ecoa o Duque(+)⊕/Duquesa(×) na repartição da energia.

**A régua.** Mede o CAMPO MÉDIO de um estado puro de dois qubits M (com ‖M‖_F=1). A norma-lei é r²+C²=1, onde r = raio de Bloch de ρ₁=MM† (a parte ATIVA, o campo médio) e C = 2|det M| = concorrência (a parte REATIVA, o emaranhamento). É uma norma redonda (o círculo). Ponto de fundo: a norma é ESPECÍFICA da régua — a forma geral é N_k(a,c)=a²−k·c², e o k É a régua (muda a norma, nunca a multiplicação: Teorema de Unicidade). Assinatura de Sylvester (p,q,r)=(2,0,0), grau 2 — a mesma régua de ℂ, do cristalino e do óptico (medido em elementares/assinaturas.py).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford = a soma dos ângulos: compor rotações soma, a⊕b = a+b, R(a)R(b)=R(a⊕b). No corpo dual (ℝ,+,⋆) a soma é a mesma soma de ℝ (só o produto vira ⋆). ISA: ADD (i64 exato). O refutador que troca por a·b (multiplicar os ângulos) falha. | |
| **⊗ La Hire (o produto)** | (x) La Hire = a COMPOSIÇÃO, e é UMA só (a mesma do expansivo, do cristalino e do relógio): R(a)R(b)=R(a⊕b), e a matriz companheira multiplica-se, comp(zw)=comp(z)comp(w). Generaliza a operadores pelo produto estrela: ⋆_{1,1,1,0} É a multiplicação matricial A@B — a mecânica quântica padrão (erro 4e-16). ISA: MULT por double-and-add (ADD deslocado); o companheiro é justamente o produto realizado como matriz. O refutador que troca o produto companheiro por soma (comp(a)+comp(b)) falha. | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin = a REFLEXÃO ν(x)=−x, o flip que costura o primal ao dual. É isomorfismo: φ(x)=−x leva (ℝ,+,·) em (ℝ,+,⋆) com x⋆y=−x·y, neutro −1, φ(φ(x))=x. Daí a unidade imaginária deriva-se: 1⋆1=−1 e â⋆â=−⟨â,â⟩=−1. É também o inversor multinível que corrige o fator reativo e conserva r²+C²=1 pelo Contrato. ISA: NEG / NOT (a troca de sinal, x↦−x = o complemento) — a involução ν²=id. | |

**Os teoremas (certificados, resíduo 0).**

- teo:lahire — R(a)R(b)=R(a⊕b), a⊕b=a+b, e comp(zw)=comp(z)comp(w): La Hire É a composição [celeste.py §1, certificado]
- teo:normaregua — N_k(zw)=N_k(z)N_k(w) para todo k, mas N_(−1)(2,3)=13 ≠ N_(+1)(2,3)=−5: a norma muda com a régua, a multiplicação não [§2]
- teo:campomedio — r²+C²=1 sobre 600 estados puros de dois qubits, erro <1e-9 (e 2e-15 em reflexivo.tex sobre 3000); a hipótese ‖M‖_F=1 é indispensável [§3]
- fechamento: C=0 ⟺ estado produto ⟺ r=1; metade: r=0 ⟺ Bell ⟺ C=1 [§3]
- teo:gentil — (ℝ,+,⋆) com x⋆y=−x·y é CORPO (neutro −1, inverso 1/x), φ(x)=−x isomorfismo; logo i²=−1 deriva-se (1⋆1=−1, â⋆â=−1) [§4]
- cor:star — ⋆_{1,1,1,0}=A@B (MQ padrão, 4e-16) e ½[A_im,B_im]=−i(a×b)·σ: o comutador É o produto vetorial [§5]
- obs:assinatura — assinatura de Sylvester (2,0,0), grau 2 (o círculo); FP=r, FP=1 ⟺ C=0 (assinaturas.py) [§ observação]
- PLACAR: elementares/celeste.py — 14/14 certificadas, resíduo 0, ponteiro −0.5 em todas

**O dual (flip ν).** ν = −1, a reflexão: o produto dual x⋆y=−x·y é o universal LIDO pela reflexão, não um corpo novo (φ(x)=−x é isomorfismo, involução ν²=id). O par físico é r (ativo) ↔ C (reativo): primal+dual reparte a unidade em r²+C²=1. E ℂ_â = ℝ ⊕ ℝ_dual: dois eixos com regras de sinal opostas (1·1=+1 vs 1⋆1=−1).

**O contrato.** Σ=ℤ/n: a órbita das rotações (compor = somar ângulos mod 2π). Régua μ=T_ω: o campo médio r²+C²=1. Flip ν: a reflexão −1 (que gera i). Mate ∘: a medida que sorteia no Bell. O CASAMENTO Γ=0 é o FECHAMENTO C=0 (estado produto) ⟺ FP=r=1: nenhum universo com FP≠1; o emaranhamento C>0 é o vazamento reativo, e o inversor Pontryagin o corrige conservando r²+C²=1 (cf. cena_rlc_potencia.py).

**o signo.** Secundidade / ÍNDICE — o corpo é o PAR: dois qubits, e a concorrência C mede o emaranhamento, a conexão factual (o embate/reação) entre eles. A parte reativa é a secundidade pura. Sobreposto: a lei de conservação r²+C²=1 e a reflexão ν que gera i (a convenção) são a Terceiridade/símbolo mediadora; o campo médio r (a qualidade do estado) toca a Primeiridade/ícone.

**A JOGADA.** Cada peça carrega uma unidade de energia repartida entre r (potência ATIVA, o campo médio, o dano direto) e C (a REATIVA, o emaranhamento) — e r²+C²=1 conserva sempre. LANCE DE EMARANHAR (La Hire, compor): enlaçar duas peças acopla-as (C>0) e por isso BAIXA a potência individual FP=r de cada uma — ganhas correlação, perdes dano solo. FECHAMENTO (C=0, estado produto, Γ=0): a peça isolada em FP=1, potência máxima, o casamento perfeito. METADE / BELL (r=0, C=1): puro emaranhamento, um único bit compartilhado — a MOEDA que o MATE (a medida) sorteia: o par de peças colapsa em cadeia. Compor dois lances vira um só somando os ângulos (a⊕b). E o FLIP ν=−1 vira a peça para o seu eixo imaginário (i²=−1), trocando as regras de sinal do combate.

---

## O corpo econômico

**Personagem:** O PRÍNCIPE (a potência / a exp): o corpo econômico é o reino da exponencial — a capitalização é exp(δt), o crescimento composto que sobe o índice. A força de juros δ=log(1+r) é o gerador infinitesimal que o Príncipe (a exp) integra. Aparece também o operador de Pontryagin (o ponteiro/poinsétia, ∏=exp∘Σ∘log) como a lente que reetiqueta as escalas; e o casamento Γ=0 ecoa o corpo eletromagnético (o cone nulo σ=1, FP=1).

**A régua.** Mede o CRESCIMENTO DO VALOR no tempo — quanto o dinheiro rende por período. A régua T_ω é a FORÇA DE JUROS δ=log(1+r) (a rapidez, o gerador infinitesimal): a taxa lida na escala aditiva de Pontryagin. Com r=5%, δ=log(1,05)≈0,048790. A norma é o fator acumulado A/P=Π(1+r_i)=N(h) (a norma telescópica), e o telescópio T=Σδ_i é a força de juros acumulada. Duas leituras da mesma régua: a DIFERENÇA constante P·r (o simples, PA) e a RAZÃO constante 1+r (o composto, PG).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | Clifford ⊕ = o JURO SIMPLES: A(t)=P(1+r·t). O aditivo — acrescenta sempre a MESMA parcela P·r (o juro só sobre o principal), a diferença A(t+1)−A(t)=P·r é constante: uma PROGRESSÃO ARITMÉTICA. Na escala da força, ⊕ é a soma de δ (δ+δ+...). Na ISA ERG-64: ADD (a soma inteira exata, i64, resíduo 0). Certificado na seção 1 de economico.py. | |
| **⊗ La Hire (o produto)** | La Hire ⊗ = o JURO COMPOSTO: A(t)=P(1+r)^t. O multiplicativo — compõe sempre o MESMO fator 1+r (o juro rende juro), a razão A(t+1)/A(t)=1+r é constante: uma PROGRESSÃO GEOMÉTRICA. Compor = somar os expoentes/forças: (1+r)^{a+b}=(1+r)^a·(1+r)^b. Na ISA: a MULT por double-and-add — a potenciação (1+r)^t é a exponenciação-por-quadratura, o produto realizado como ADD deslocado. Certificado na seção 2 de economico.py. | |
| **∏ Pontryagin (o operador)** | Pontryagin ∏=exp∘Σ∘log — a LENTE que liga o simples ao composto. BAIXAR O ÍNDICE (log, o adjunto): log A(t)=log P + t·δ leva a PG (composto, La Hire) à PA (simples na força δ) — a multiplicação por 1+r vira a soma de δ. SUBIR O ÍNDICE (exp): exp(δt)=(1+r)^t sobe a PA de volta à PG (no contínuo dA=δA dt, A=P·e^{δt}). Na ISA: o dispatch/composição — a composição log/exp que reetiqueta ⊗ como ⊕ (o mesmo papel de NOT/composição da tríade, aqui a mudança de escala multiplicativo↔aditivo). Certificado nas seções 3, 4 e 5 de economico.py. | |

**Os teoremas (certificados, resíduo 0).**

- Teo. o par do contrato: o simples é PA (diferença P·r constante = Clifford ⊕) e o composto é PG (razão 1+r constante = La Hire ⊗). economico.py seções 1-2, 16/16 resíduo 0.
- Teo. o log baixa o índice (PG→PA): log A(t)=log P + t·δ com δ=log(1+r) a força de juros (o adjunto de Pontryagin). economico.py seção 3.
- Teo. o exp sobe o índice (PA→PG): exp(δt)=(1+r)^t; no contínuo A=P·e^{δt}. economico.py seção 4.
- Obs. o composto domina o simples a partir de t=2 (convexidade de exp); Pontryagin mede o descolamento reta↔exponencial. economico.py seção 5.
- Teo. o dual pelo dual do operador (ν=−1): VP(t)=A(1+r)^{−t}=A·e^{−δt}, o desconto; e^{−δt}·e^{δt}=1, VP(A(t),t)=P (descontar desfaz capitalizar). economico.py seção 6.
- Teo. NÃO isomorfo ao entrópico: o econômico tem inverso aditivo (o desconto, δ+(−δ)=0) → é CORPO; o entrópico (max) é idempotente, sem inverso → SEMICORPO. economico.py seção 7.
- Teo. a sombra (desquantização de Maslov): a soma-quente log-sum-exp T·log(e^{a/T}+e^{b/T}) → max(a,b) quando T→0; o entrópico é o cristal a T=0 do econômico. economico.py seção 7.
- Teo. DUAIS ao entrópico (mesma ν=−1): a reflexão leva max↦min (min-plus) como leva capitalizar↦descontar (δ↦−δ). economico.py seção 7.
- Teo. NÃO isomorfo ao celeste (hiperbólico e^{δt} injetor, grupo (ℝ₊,×) não compacto vs elíptico rot periódico 2π, S¹ compacto); mas coincidem no produto La Hire (compor=somar). economico.py seção 8.
- Teo. rotação de Wick (δ↦iδ) liga celeste e econômico: cos(iθ)=cosh(θ). economico.py seção 8.
- Teo. ISOMORFO ao telescópico: o composto A/P=Π(1+r_i)=Πe^{δ_i}=N(h) é a norma telescópica; φ=log leva o produto La Hire à soma Clifford, T=Σδ_i o telescópio; ambos guardam o inverso N(h)·N(−h)=1 (o desconto). economico.py seção 9.
- Placar global: 16/16 certificadas, ponteiro −0.5 em todas, resíduo 0 (medida consistente). Refutador com dentes: cada afirma() traz a sua contra-afirmação falsa.

**O dual (flip ν).** O flip ν=−1: a reflexão δ↦−δ (a régua do avesso, dual.tex). Leva a CAPITALIZAÇÃO (o futuro, e^{δt}, autoespaço E_+) ao DESCONTO / VALOR PRESENTE (e^{−δt}, autoespaço E_−). São inversos: e^{−δt}·e^{δt}=1, VP(A(t),t)=P — descontar desfaz capitalizar. A mesma ν=−1 que dualiza o entrópico (max↦min, min-plus). Assinatura (herdada do telescópico): (p,q,r)=(2,2,0), grau 4, ν=(−1)∘rev.

**O contrato.** O casamento Γ=0 é a AUSÊNCIA DE ARBITRAGEM: FP=1 quando o desconto desfaz exatamente a capitalização, sem vazar valor (e^{−δt}·e^{δt}=1). FP<1 é o SPREAD, o atrito que vaza valor; o inversor multinível casa cada força δ_i do perfil (o telescópio) até nada vazar — nenhum universo fecha com FP≠1. Σ=ℤ/n: a órbita dos períodos (o relógio dos turnos t). Régua μ=T_ω: a força de juros δ=log(1+r). Flip ν=−1: capitalizar↔descontar. Mate ∘: a composição à identidade VP(A(t),t)=P. A grandeza conservada é o VALOR (Contrato, a conservação: resíduo 0). SOL = garrafa de Koch áurea (harmônicos φ^{−j}).

**o signo.** TERCEIRIDADE / o SÍMBOLO (a lei, a mediação, o hábito, a convenção). O corpo econômico é a mediação por excelência: o dinheiro é a convenção pura, o juro é a LEI de crescimento que medeia o valor entre presente e futuro, e o operador de Pontryagin (∏=exp∘Σ∘log) é a mediação/lei que costura ⊗ (composto) a ⊕ (simples). A força δ é o hábito (a taxa) que rege a evolução. Dentro da tríade: ⊕ o par (secundidade/índice, a diferença factual constante), ⊗ a mediação, (op) o operador/lei (terceiridade/símbolo).

**A JOGADA.** O tempo é o tabuleiro; o principal P é a peça posta em campo. CAPITALIZAR é o lance de acumular poder: a cada turno (período) o jogador escolhe a operação. Lance ⊕ (juro simples / ADD): ganho FIXO e previsível por turno — P·r a mais, uma reta segura; a peça engrossa devagar mas nunca vaza. Lance ⊗ (juro composto / MULT double-and-add): ganho que RENDE SOBRE SI MESMO — a partir do 3º turno (t≥2) o composto DOMINA o simples por convexidade da exp, uma bola de neve que descola da reta (Pontryagin mede o descolamento). O contra-lance é o DESCONTO (o flip ν=−1): trazer valor do FUTURO ao PRESENTE — descontar DESFAZ capitalizar exatamente (VP(A(t),t)=P), o retorno perfeito à origem. A vitória é o casamento Γ=0 / FP=1: SEM ARBITRAGEM, nenhum valor vaza no ciclo capitalizar⋈descontar. Quem deixa FP<1 abre um SPREAD — atrito que sangra valor a cada turno; o adversário explora o vazamento. As taxas variáveis δ_i são o perfil da partida (o telescópio N(h)=Πe^{δ_i}): casar cada força do perfil é afinar o inversor multinível até nada vazar.

---

## O corpo evolutivo

**Personagem:** O operador central é PONTRYAGIN (a seleção, sobe/desce índices — o Príncipe/a potência via exp·Σ·log). A Rainha (La Hire ×) na ponderação pela fitness g=diag(w). O Duque (+) / a Duquesa (×) — Joaquim/Yasmin — na reprodução (o par ⋆) e no produto pela aptidão. Traço distintivo: é o corpo da SELEÇÃO, o operador que maximiza ⟨w⟩ (Fisher).

**A régua.** Mede a FREQUÊNCIA gênica p∈(0,1) numa régua HIPERBÓLICA. Coordenada s=2p−1∈(−1,1), norma 1−s² (assinatura (1,1,0), medida em assinaturas.py). Os horizontes s=±1 são a FIXAÇÃO (p=1) e a EXTINÇÃO (p=0) — a ilha da régua hiperbólica. É a régua W_+1, idêntica à do sensitivo/expansivo/relógio. Composição de seleções: s1⊕s2=(s1+s2)/(1+s1s2), a adição relativística de velocidades. Sobre ℚ fecha em corpo via o metal σ_m no meio: N(σ_m)=σ_m·σ̄_m=−1, e ℚ(√(m²+4)) é corpo (√∉ℚ) — anisótropo, corpo de fato.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford = a REPRODUÇÃO, o monóide idempotente de Hardy–Weinberg: p⋆q=(p+q)/2. Comutativa e idempotente (p⋆p=p — o equilíbrio de Hardy–Weinberg é o ponto fixo). É monóide, não anel (falta-lhe o flip para fechar). Na régua hiperbólica a soma das seleções é ⊕_{+1}: s1⊕s2=(s1+s2)/(1+s1s2). Na ISA ERG-64: ADD inteiro (a média/soma exata sobre ℚ, num/den, resíduo 0), a corrente de Kirchhoff que conserva ∑p_i=1. | |
| **⊗ La Hire (o produto)** | (×) La Hire = a PONDERAÇÃO pela fitness, o produto p_i·w_i (a métrica g=diag(w) atuando sobre a frequência) — o ganho/tensão da Rainha que multiplica cada tipo pela sua aptidão antes de normalizar. Na ISA ERG-64: a MULT por double-and-add (ADD deslocado), exata sobre ℚ. É o passo "sobe com w" do replicador (o produto que a normalização depois desce). | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin = a SELEÇÃO NATURAL, o replicador p_i↦p_i·w_i/⟨w⟩. É subir/descer índices: SOBE com a fitness-métrica g=diag(w) (o produto La Hire), DESCE por normalização (divide por ⟨w⟩=∑p_i·w_i). ⟨w⟩ é monótona crescente (o teorema fundamental de Fisher) — a norma que o operador maximiza. Na ISA: o dispatch/composição exp·Σ·log (o ponteiro/poinsétia que costura sobe→desce); o SUB/CMP que normaliza e compara gerações. Certificado ⟨w⟩ crescente por 30 gerações. | |

**Os teoremas (certificados, resíduo 0).**

- Teo. monóide (evolutivo.py §1): ⋆ comutativa e idempotente (p⋆p=p), Hardy–Weinberg é o ponto fixo — monóide, não anel — CERTIFICADO
- Teo. régua hiperbólica (§2): s=2p−1 tem horizontes s=±1 (fixação p=1 / extinção p=0); s1⊕s2=(s1+s2)/(1+s1s2) = régua W_+1 — CERTIFICADO
- Assinatura (1,1,0), norma 1−s², grau 2, k=+1 — a mesma do relógio/expansivo/sensitivo/frações contínuas (assinaturas.py) — CERTIFICADO
- Teo. flip/ancoragem (§3): p↦1−p (s↦−s) troca os tipos, ponto fixo p=1/2 (frequência neutra) — CERTIFICADO
- Teo. régua dual = inversão conforme (§4): w↦1/w involução (1/(1/w)=w), a mesma do conforme (n₀↔n∞, x↦2x/|x|²); cone nulo |x|²=0 ↔ fitness neutra s=0 — CERTIFICADO
- Teo. metal fecha o corpo (§5): σ_m·σ̄_m=−1, ℚ(√(m²+4)) é corpo (√∉ℚ), anisótropo sobre ℚ — CERTIFICADO
- Teo. seleção = Pontryagin (§6): replicador p_i↦p_i·w_i/⟨w⟩ sobe com g=diag(w), desce por normalização, ∑=1 conservado — CERTIFICADO
- Teo. Fisher (§6): ⟨w⟩ monótona crescente por 30 gerações — a norma que o operador maximiza — CERTIFICADO
- Fechamento energético FP=1: ∑p_i=1 é a grandeza conservada, o casamento Γ=0 é a normalização; nenhum universo fecha com FP≠1 (a conservação: resíduo 0)
- PLACAR verificado rodando evolutivo.py: 10/10 certificadas, resíduo 0, ponteiro −0.5 em todas as 10 (⟨X,X_⊤⟩=−½·d²)

**O dual (flip ν).** O FLIP dual ν: p↦1−p (equivalente a s↦−s), a ancoragem — troca os tipos, ν=−1, ponto fixo p=1/2 (a frequência neutra, a deriva). A RÉGUA DUAL é a inversão w↦1/w (s↦−s), uma INVOLUÇÃO (1/(1/w)=w) — a MESMA involução do corpo CONFORME (a inversão n₀↔n∞, x↦2x/|x|²). Casamento notável: o cone nulo do conforme (|x|²=0) corresponde exatamente à fitness neutra (s=0), o mesmo horizonte. A régua dual de Darwin É a inversão conforme.

**O contrato.** Σ=Z/n: a órbita das frequências, ∑p_i=1 (a medida conservada). RÉGUA μ=T_ω: a norma hiperbólica 1−s², s=2p−1, assinatura (1,1,0), fechada em corpo pelo metal σ_m (N=−1) sobre ℚ(√(m²+4)). FLIP ν=−1: p↦1−p, ponto fixo p=1/2; régua dual = inversão conforme. MATE o: os horizontes fixação (p=1)/extinção (p=0). CASAMENTO Γ=0 / FP=1: a NORMALIZAÇÃO — ∑p_i=1 se mantém enquanto ⟨w⟩ sobe (o replicador divide por ⟨w⟩ e conserva o total). FP<1 seria perda de massa que o inversor (renormalização) corrige; nenhum universo fecha com FP≠1 (a conservação: resíduo 0). Cf. cena_rlc_potencia.py.

**o signo.** TERCEIRIDADE / SÍMBOLO dominante: o corpo é regido pela LEI/mediação (o operador Pontryagin, a seleção como hábito/convenção estatística — o replicador que governa gerações; Fisher como teorema-lei). A SECUNDIDADE/ÍNDICE é a reprodução ⋆ (o par Hardy–Weinberg, a conexão factual entre dois genótipos) e o embate da fitness (a captura). A PRIMEIRIDADE/ÍCONE é a qualidade pura da frequência p∈(0,1), a régua hiperbólica em si (semelhança com sensitivo/expansivo/relógio, mesma assinatura (1,1,0)).

**A JOGADA.** O corpo evolutivo é a mecânica de POPULAÇÃO / meta do reino: cada tipo de peça tem uma frequência p no exército, num arco entre EXTINÇÃO (p=0, a peça some do tabuleiro) e FIXAÇÃO (p=1, domina). O LANCE = o replicador de Pontryagin: a cada turno as peças que capturaram/sobreviveram (fitness w_i alta) sobem sua frequência, p_i↦p_i·w_i/⟨w⟩, e o total ∑p_i=1 se conserva (FP=1, nada vaza — massa constante). Somar dois pushes de seleção NÃO é linear: obedece à régua hiperbólica s1⊕s2=(s1+s2)/(1+s1s2), então empilhar vantagens rende cada vez menos perto do horizonte da fixação (não há vitória grátis nos extremos). O FLIP p↦1−p é a jogada de ANCORAGEM/deriva: troca os tipos e o único ponto fixo é p=1/2, a frequência neutra — a deriva genética como reset ao equilíbrio. A cada geração ⟨w⟩ (a aptidão média do exército) só cresce (Fisher): o combate é uma escalada monótona de qualidade, e a seleção é irreversível dentro de uma linhagem.

---

## O corpo expansivo

**Personagem:** O REI: σ_m é o ponto fixo / o gap metálico σ_m = m + 1/σ_m, a unidade fundamental da ordem Z[σ_m] (σ_1 = φ, o áureo — a mesma raiz que a Rainha La Hire φ²=φ+1). O gato A_m é o operador do Rei; sua expansão é a marcha do rei que cresce de taxa constante. Também o Príncipe (a potência/exp: σ_m^n são todas as unidades) e os Duques Joaquim⊕/Yasmin⊗ na dupla expansão(+)/contração(x).

**A régua.** A régua redonda G = R/(log σ_m)Z: mede o resíduo log|x| mod log σ_m; unidade = o regulador ℓ = log σ_m.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD = a soma PONTUAL: (a+b√D)+(c+d√D) = (a+c)+(b+d)√D. Componente a componente, sem mistura. Neutro 0. Na ISA ERG-64: ADD, dois i64 (na verdade racional exata num/den via Fraction, resíduo 0 real — sem float) somados coordenada a coordenada. Realizado no código por Q2.__add__. | |
| **⊗ La Hire (o produto)** | (x) LA HIRE = o produto do corpo: (a+b√D)·(c+d√D) = (ac+Dbd)+(ad+bc)√D — a régua D dobra a parte irracional de volta ao racional (é o A²=mA+I fechando). Neutro 1. Na realização matricial é XY (o produto do gato). Certificado: associativa, distributiva, N multiplicativa (N(xy)=N(x)N(y)) em 400 triplos para D=5,8,13; refutado por _mul_naoassoc (coef 2, perde associatividade) e _mul_naodistrib (termo y_b², quebra bilinearidade). Na ISA: a MULT por double-and-add (ADD deslocado), exata. Código: Q2.__mul__. | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN = o FLIP Λ = log (exp·Σ·log), o ponteiro que costura os dois espaços: no plano da álgebra a órbita corre sobre a hipérbole N(x)=const; na coordenada logarítmica corre sobre uma reta; módulo o regulador, sobre o CÍRCULO, avançando UMA unidade por passo (resíduo nulo sempre). O operador também é a conjugação de Galois σ̄ = −1/σ (ν=−1 na parte irracional) e, no nível do produto, a CONTRAÇÃO de índices μ_lij z^i w^j (a multiplicação É um tensor de ordem 3). Na ISA: o dispatch / a composição — a contração einsum 'lij,i,j->l'; e o NOT booleano é o análogo do flip ν=−1. Código: Q2.conj, einstein(), _erro_contr. | |

**Os teoremas (certificados, resíduo 0).**

- Cayley–Hamilton: A_m² = mA_m + I ⟹ Z[A_m] ≅ Z[x]/(x²−mx−1) ≅ Z[σ_m], corpo de frações Q(√(m²+4)) — expansivo.py §2 (ok)
- Q(√D) é corpo: associativa, distributiva, N(xy)=N(x)N(y), todo x≠0 inverte (x̄/N(x)) — 400 triplos, D=5,8,13 — expansivo.py §3 (ok)
- N(σ_m)=σσ̄=−1=det A_m, σ̄=−1/σ, Fix(conj)=Q (a reflexão é a conjugação de Galois) — expansivo.py §4 (ok)
- H = (1/n)·log q_n → log σ_m: a taxa de expansão É o regulador da ordem Z[σ_m] (n=3000; 0,480944 vs log φ=0,481212 para m=1) — expansivo.py §5 (ok)
- posto 1 (Dirichlet) ⟹ UMA taxa: qualquer início x0∈{0.3,1,7} converge ao mesmo log σ_m — expansivo.py §5 (ok)
- m↦Q(√(m²+4)) é MUITOS-PARA-UM: Q(√5) gerado por m=1,4,11,29; log σ_m é regulador da ORDEM, não do corpo (log σ_4=3·log φ) — expansivo.py §5 (ok)
- régua redonda G=R/(log σ)Z: resíduo nulo ⟺ ser unidade N(x)=±1 (σ^{−2},σ^0,σ^3 → 0; 1+√5 → 0,2119) — expansivo.py §5b (ok)
- órbita é HIPÉRBOLE: autovalores σ_m, −1/σ_m reais e distintos; preserva a forma quadrática (N: −1→+1) e NÃO o supremo (3→5) — expansivo.py §5c (ok)
- D<0 (elipse) ⟹ unidades finitas; D>0 (hipérbole) ⟹ infinitas: expandir é ser hipérbole — expansivo.py §5c (ok)
- inversão da obstrução: completar cria os divisores de zero — R[e]/(e²−5) cinde, Q(√5) não — expansivo.py §6 (ok)
- H = log ρ(A_m) = h_top: o regulador É a entropia topológica do subshift do gato (m=1: golden-mean shift, proíbe '11', h=log φ) — expansivo.tex Teo. sft (N)
- o produto é tensor μ de ordem 3: z·w=Σμ_lij z^i w^j; μ_S = −⟨x,y⟩ (a medida), μ_A = x×y (a torção) — verificado em H e O — expansivo.py §6b (ok)
- o homônimo de Einstein: o TENSOR G_μν NÃO é o produto — G≡0 em 2D, G(λg)=G(g) (grau 0), Schwarzschild vácuo G=0 — expansivo.py §7 (ok)
- o gato preserva a forma SIMPLÉTICA: Aᵀ J A = det(A)·J = −J — anti-simplectomorfia — expansivo.py §8 (ok)
- PLACAR: 62/62 certificadas, 4 triviais (verdadeiras e independentes do objeto), resíduo 0 (medida consistente)

**O dual (flip ν).** O FLIP ν = −1: a conjugação de Galois x̄ = a − b√D = −1/σ (ν=−1 agindo só na parte irracional). Primal·dual = a medida: x·x̄ = N(x) = a²−Db² = det. É a TORÇÃO do dicionário (μ_A antissimétrica). Complementaridade dinâmica: σ_m (autovetor que EXPANDE, taxa +log σ) ↔ −1/σ_m (que CONTRAI, taxa −log σ) — o flip Λ=log reconcilia a hipérbole (plano da álgebra) e o círculo (coordenada log). Fix(conj)=Q, o neutro.

**O contrato.** Σ=Z/n: as unidades ±σ_m^n formam o reticulado Λ=(log σ_m)Z; o relógio é G=R/(log σ_m)Z, um círculo — posto 1 por Dirichlet, UMA órbita. Régua μ = T_ω: o resíduo log|x| mod log σ_m, unidade = o regulador ℓ=log σ_m. Flip dual ν=−1: a conjugação de Galois σ̄=−1/σ. Mate ∘: resíduo nulo ⟺ ser unidade N(x)=±1. Casamento Γ=0 = FP=1: a régua radial (logarítmica) fecha sem enrugar exatamente quando N(x)=±1 — todo N≠±1 tem resíduo>0, FP<1, o vazamento que o inversor de Pontryagin (torre de Fibonacci, harmónicos φ^{−j} da garrafa áurea de Koch) corrige. Nenhum universo tem FP≠1. (expansivo.tex Obs. assinatura; assinatura de Sylvester (p,q,r)=(1,1,0), grau 2 — indefinida sobre R, corpo sobre Q.)

**o signo.** TERCEIRIDADE / SÍMBOLO domina o corpo: a lei "três nomes, um número" — taxa de expansão H = regulador de Dirichlet log σ_m = entropia topológica h_top do subshift do gato — é a mediação/o hábito que unifica três domínios num só invariante (convenção que se cumpre por lei, não por semelhança nem embate). O operador Pontryagin (o flip Λ=log, exp·Σ·log, a contração de índices μ_lij) é a terceiridade/símbolo. A camada de secundidade/índice está na dinâmica hiperbólica: a órbita do gato é reação/embate factual (v=(3,2)→(5,3)), o par expansão↔contração (σ ↔ −1/σ). A qualidade/primeiridade/ícone é o próprio metálico σ_m, a razão pura que o corpo encarna.

**A JOGADA.** O corpo expansivo é a MECÂNICA DE CRESCIMENTO do Rei. Cada lance aplica o gato A_m ao vetor-estado (a,b): a posição EXPANDE por fator σ_m a cada turno (a razão metálica; σ_1=φ, o áureo). Por posto 1 há UMA só taxa de expansão — não dá para diversificar: a força cresce numa direção única (o autovetor σ_m), a outra (−1/σ_m) contrai. O ataque é HIPERBÓLICO: estica a peça (v=(3,2), ‖v‖∞ = 3 → 5) mas CONSERVA a norma N=a²−Db²=det a menos de sinal — N(v)=−1 vira N(Av)=+1, um FLIP entre os dois ramos da hipérbole (o lance troca de ramo, não vaza energia). O gate de PONTUAÇÃO/MATE: só marca quem aterrissa numa UNIDADE, N(x)=±1 = resíduo 0 no relógio redondo (casamento Γ=0, FP=1). Toda posição com N≠±1 tem resíduo positivo — VAZA (FP<1), e o inversor de Pontryagin (a torre de Fibonacci, o flip Λ=log) corrige o vazamento devolvendo a peça ao círculo, um passo por turno. O dual σ̄=−1/σ é o RECUO/contração (a Duquesa desfaz a expansão do Duque). A vitória não é matar: é fechar a órbita sobre o círculo, resíduo nulo — a peça vira unidade.

---

## O corpo SOMÁTICO

**Personagem:** As PRINCESAS / o corpo técnico (quem ATESTA a lei, o adjunto Gᵀ, o par estado ⋈ co-estado do princípio do máximo — a regulação). O tex explicita o par de Pontryagin do corpo técnico (G ⋈ Gᵀ). Como personagem coletivo, é o TECIDO-organismo: o exército de peões (as células que colidem/travam na homeostase, como os peões).

**A régua.** A NORMA é o DETERMINANTE da dinâmica: det(exp(tG)) = e^{t·tr G}, MULTIPLICATIVA (det(AB)=det A·det B). Não há Sylvester único — a régua é o det (o traço na álgebra de Lie). O traço tr G (a soma das taxas de proliferação/diferenciação) é o telescópio, e por HURWITZ a norma multiplicativa faz do somático um CORPO. A régua conjugada é a forma bilinear do adjunto: ⟨Gx,y⟩ = ⟨x,Gᵀy⟩ (o par estado ⋈ co-estado, Pontryagin). Certificado det(exp(tG))=exp(t·tr G) e a multiplicatividade em somatico.py.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | Clifford (+) = a FUSÃO de dois tecidos: a soma direta G⊕G₂ (bloco-diagonal), com exp(G⊕G₂)=exp(G)⊕exp(G₂) — cada dinâmica segue independente. Na ISA ERG-64: ADD (a montagem bloco-diagonal, o empacotamento das duas dinâmicas em uma; na álgebra o traço soma, tr(G⊕G₂)=tr G+tr G₂, o telescópio aditivo). No idioma entrópico da expansão, o ⊕ é o max (o tipo dominante do tecido). | |
| **⊗ La Hire (o produto)** | La Hire (x) = a COMPOSIÇÃO da dinâmica no tempo: exp((s+t)G)=exp(sG)·exp(tG) — o grupo a um parâmetro, o produto multiplicativo das trajetórias. Na ISA: a MULT por double-and-add (o ADD deslocado que a matriz exp acumula — Σ M^k/k!, o produto de matrizes construído de somas). No entrópico da expansão, o ⊗ é o + (a soma sobre os tipos, o telescópio da população). | |
| **∏ Pontryagin (o operador)** | Pontryagin (op) = exp∘Σ∘log, o mesmo operador da mitose, da hélice e dos juros. Gera a dinâmica: N(t)=exp(tG)N₀ leva o gerador G (as taxas, o infinitesimal aditivo) à trajetória (o movimento multiplicativo); o log recupera G. Na ISA: o dispatch/a composição (a costura que aplica exp — a série que soma-e-desloca). O DUAL do operador é o adjunto Gᵀ: exp(tG)ᵀ=exp(tGᵀ), o co-estado que evolui para trás (o princípio do máximo). | |

**Os teoremas (certificados, resíduo 0).**

- A INDUÇÃO constrói (proliferação): base zigoto k=0 + passo S (mitose dobra) → 2^k; injetiva não-sobrejetiva, cokernel=fechamento — certificado somatico.py
- A META-INDUÇÃO classifica (diferenciação): π projeta N células em T tipos; sobrejetiva não-injetiva, kernel=resíduo; opera em O(T) não O(N) — certificado
- Indução ⊣ meta-indução são DUAIS: S injetivo (↑, cokernel=fechamento) ↔ π sobrejetivo (↓, kernel=resíduo) — o par de Pontryagin
- ENTRÓPICO (max,+): a população SOMA sobre os tipos (⊗=+, telescópio, 128), o dominante é o max (⊕, 64) — certificado
- A EXPANSÃO colide: proliferação por S até a capacidade (512≤1000, 2·512>1000) — a homeostase, a inibição por contato
- DINÂMICA=Pontryagin: N(t)=exp(tG)N₀ é grupo a um parâmetro, exp((s+t)G)=exp(sG)·exp(tG) — certificado (é o produto, não a soma)
- CO-ESTADO=adjunto: exp(tG)ᵀ=exp(tGᵀ) com G≠Gᵀ — o princípio do máximo de Pontryagin, a homeostase ótima
- SOMA (Clifford ⊕): exp(G⊕G₂)=exp(G)⊕exp(G₂) (bloco-diagonal, fusão) — certificado
- NORMA/Hurwitz: det(exp(tG))=exp(t·tr G) MULTIPLICATIVA, det(AB)=det A·det B → é CORPO — certificado
- PLACAR: certificadas 10/10, resíduo 0 (medida consistente); ponteiro -0.5 em todas — somatico.py roda ok

**O dual (flip ν).** O flip dual é a dualidade proliferar ↔ diferenciar (indução S ⊣ meta-indução π): S injetivo não-sobrejetivo (↑ constrói, o local, cokernel=fechamento) ↔ π sobrejetivo não-injetivo (↓ classifica, o global, kernel=resíduo). Na dinâmica, o dual é o adjunto: ESTADO por G (para frente) ↔ CO-ESTADO por Gᵀ (para trás no tempo), exp(tG)ᵀ=exp(tGᵀ). Primal+dual equilibrados = a homeostase (Γ=0).

**O contrato.** Σ=ℤ/n: a órbita da diferenciação — π=x mod T classifica as células em T tipos (as classes/órbitas, o relógio dos tipos). Régua μ (T_ω): a norma det(exp(tG))=e^{t·tr G}, multiplicativa (Hurwitz→corpo). Flip ν: o dual proliferar↔diferenciar / estado↔co-estado (Gᵀ). Mate ⊙: a HOMEOSTASE. Casamento Γ=0: a homeostase é o casamento — FP=1 quando tr G=0 (det exp=1, a população conserva-se, nada vaza); FP<1 é tr G≠0 (crescimento que transborda a capacidade), corrigido até a homeostase. O SOL é a garrafa de Koch áurea (fonte única, harmônicos φ^{-j}); nenhum universo fecha com FP≠1 (a conservação: resíduo 0).

**o signo.** TERCEIRIDADE / o SÍMBOLO (a lei, a mediação, o habito). O somático é regido pela LEI da dinâmica: o operador de Pontryagin (exp∘Σ∘log) e o princípio do máximo (o adjunto Gᵀ) — a mediação/regulação que costura estado e co-estado. É o corpo da lei que rege o organismo (a homeostase como habito/norma convencionada). A diferenciação (π, a classificação em tipos/classes) é ela própria a terceiridade: a mediação por classes, o símbolo. Internamente a tríade aparece: o par de tecidos ⊕ (secundidade/índice), a composição ⊗ (mediação), o operador op (terceiridade/símbolo).

**A JOGADA.** O somático é o TABULEIRO-EXÉRCITO: o global que reúne muitos tabuleiros-célula (ℚ_p locais). A JOGADA-mãe é PROLIFERAR — o lance S (o sucessor, a mitose): o tecido dobra célula a célula, 2^k, e EXPANDE por S até COLIDIR com a capacidade do espaço (a inibição por contato) — aí para: a HOMEOSTASE, exatamente como os peões colidem e travam. O lance dual é DIFERENCIAR — π projeta as N células nos T TIPOS (as classes/órbitas): você não move célula a célula (O(N)), move por tipo (O(T)) — o atalho do grafo (o BAI, a seleção indutiva usa as classes, não os nós). FUNDIR (+): juntar dois tecidos-exército é a soma direta G⊕G₂, cada dinâmica independente, empilhada. A régua do combate é det(exp(tG)): quem tem tr G>0 cresce (transborda, FP<1) até ser corrigido à homeostase; tr G=0 conserva a população (FP=1, nada vaza). O co-estado Gᵀ é a REGULAÇÃO — o valor-sombra de cada tipo, quem decide a trajetória ótima do exército.

---

## geométrico

**Personagem:** O Príncipe (a potência / a exp∘log) e Pontryagin — o corpo é o do operador exp·Σ·log encarnado: o potencial é um logaritmo, a conexão é a composição de derivadas. Traço: o CURVADOR, quem dobra a régua. A conservação (Bianchi ∇^μG_{μν}=0) é a lei que o corpo faz respeitar; o SOL como fonte única (garrafa áurea de Koch, harmónicos de Fibonacci φ^{-j}) é o único poço que não vaza.

**A régua.** A régua T_ω é a MÉTRICA. Duas camadas: (1) a norma de Minkowski ds²=t²−x²−y²−z², de assinatura (p,q,r)=(1,3,0), grau 4 — uma redonda e três hiperbólicas, medida em elementares/assinaturas.py; ela mede o intervalo espaço-temporal e diz o que é o cone nulo. (2) o potencial-poço Φ=−½·log r com r=1+2M/|x|: mede a profundidade do vale gravitacional como um logaritmo de norma. A régua que CURVA é g_{μν}: o que ela mede é a geodésica, o caminho que uma peça de massa impõe ao tabuleiro. A régua tensorial é indispensável — a escalar mede errado a luz (fator 6).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) Clifford, a SOMA: a superposição de potenciais, Φ=Σ Φ_i — os poços gravitacionais somam-se linearmente. E o nó exato do corpo: o potencial SOMA-SE onde as normas se MULTIPLICAM (Φ=−½·log r transforma produto de normas em soma de potenciais). Além disso a própria assinatura (1,3,0) é uma assinatura de ÁLGEBRA DE CLIFFORD Cl(1,3) — o Minkowski é o Clifford do corpo. Na ISA ERG-64: ADD (acumular o potencial, o Σ Φ_i). | |
| **⊗ La Hire (o produto)** | (x) La Hire, o PRODUTO: a multiplicatividade da norma N(ab)=N(a)·N(b) — é sobre esse produto que o log age para virar soma. No tensor é a contração/produto tensorial de g_{μν} e o produto de Christoffels que constrói Ricci (Gam·Gam nos termos R=∂Γ−∂Γ+ΓΓ−ΓΓ). O ganho, a tensão que curva. Na ISA: a MULT por double-and-add (ADD deslocado) — cada produto ΓΓ do Riemann é uma MULT. | |
| **∏ Pontryagin (o operador)** | (op) Pontryagin, o OPERADOR exp·Σ·log — este corpo É o corpo de Pontryagin: o log é o costurador. Φ=−½·log r é literalmente log∘norma: o operador que leva o produto das normas à soma dos potenciais (e o exp de volta). No lado tensorial o operador é a CONEXÃO/derivada covariante: o Christoffel Γ=½g^{Ad}(∂g+∂g−∂g) e o Ricci R_{μν}=∂Γ−∂Γ+ΓΓ−ΓΓ são a composição de derivadas que curva o espaço. Na ISA: NOT/dispatch/composição — o operador que compõe (∂ e o Σ sobre índices), a poinsétia que o ponteiro percorre para fechar a curvatura. | |

**Os teoremas (certificados, resíduo 0).**

- Φ = −½·log r com r = 1+2M/|x| (o potencial escalar é meia-log da norma) — ok
- limite newtoniano de 1ª ordem: Φ ≈ −M/|x| (a única parte certa da prop. do paper) — ok
- 2ª ordem: o coeficiente é M²/|x|², NÃO M²/(2|x|²) — refuta o erro de fator 2 em sandbox/fisica.tex; o correto é Φ=−M/x+M²/x²−4M³/(3x³) — ok
- Φ é MEIA ENTROPIA: log N é a entropia (norma.py) e Φ=−½·log r, logo o potencial soma onde as normas multiplicam — ok
- Schwarzschild é VÁCUO: R_{μν}=0 para f=1−2M/r, g_rr=−1/f, verificado simbolicamente nas 16 componentes (refutador 1−2M/r² dá Ricci≠0) — ok
- g_rr=−1/f é indispensável: com g_rr=−1 a solução quebra — ok
- PLACAR: geometrico.py 6/6, resíduo 0, ponteiro −0.5 em todas (⟨X,X_⊤⟩=−½·d², distância conforme à afirmação trivial)

**O dual (flip ν).** O flip nu é log↔exp (soma↔produto): o operador que leva o produto de normas à soma de potenciais e de volta — primal (produto multiplicativo da norma) + dual (soma aditiva do potencial) casam pelo −½·log. Em pares físicos: ENTROPIA ↔ POTENCIAL (Φ=−½·entropia, com o sinal trocado — o dual literal). E a dualidade das duas teorias: a ESCALAR (incompleta, erra a luz por 6) é o dual pobre da TENSORIAL (completa, dá Einstein); o campo r escalar é promovido a tensor g_{μν} para fechar o vazamento.

**O contrato.** Σ=Z/n: a órbita do cone nulo/geodésica. Régua μ=T_ω: a métrica de Minkowski, assinatura (1,3,0), grau 4 (assinaturas.py). Flip ν: log↔exp / entropia↔potencial. Mate ∘: o VÁCUO Schwarzschild R_{μν}=0. O casamento Γ=0 é o CONE NULO ds²=0, σ=1 → FP=1: a luz corre no cone nulo e nenhum universo vaza. A grandeza conservada é o momento-energia via a identidade de Bianchi ∇^μG_{μν}=0 — a energia circula e fecha pelo Contrato (a conservação: resíduo 0). Todo FP<1 é fonte espúria que o inversor multinível (torre de Fibonacci, Pontryagin) corrige. Cf. cena_rlc_potencia.py.

**o signo.** Terceiridade / Símbolo (a lei, a mediação, o habito): o corpo é sobre o OPERADOR que costura — a conexão covariante e a curvatura de Ricci são a lei que MEDIA como toda peça se move (a geodésica é hábito, convenção do campo), e a identidade de Bianchi é a lei de conservação por convenção. É o corpo de terceiridade da tríade: o (op) Pontryagin, a mediação exp·Σ·log. Nota: há uma face de Secundidade (a gravidade como reação/embate factual entre massa e caminho, o índice), mas a ênfase certificada é a LEI que curva (R_{μν}=0, ∇^μG_{μν}=0) — símbolo.

**A JOGADA.** O corpo geométrico é a CURVATURA DO TABULEIRO. O tabuleiro tesselado deixa de ser plano: uma peça de massa M cava um poço Φ=−½·log(1+2M/|x|) e as linhas de movimento das outras peças passam a seguir GEODÉSICAS — curvam ao redor da massa. O lance = pousar massa: colocar uma peça pesada dobra a régua e desvia quem passa perto (a deflexão). O COMBATE corre no cone nulo: um ataque de luz viaja em ds²=0 (σ=1, Γ=0) e é onde FP=1 — o disparo limpo que chega. O poço bem-formado é o VÁCUO R_{μν}=0 (Schwarzschild): campo sem vazamento. O refutador com dentes é a mecânica de falha: se a forma do poço quebra (trocar 1−2M/r por 1−2M/r², ou g_rr=−1/f por g_rr=−1), Ricci≠0 — o poço mal-cavado tem fonte espúria, vaza energia (FP<1). Renomear a massa (2M→3M) não quebra: é só um poço maior. Só quebrar a FORMA quebra. Assim a jogada distingue um buraco negro legítimo (mate que não mata, curvatura conservada) de um vazamento proibido.

---

## O corpo mórfico

**Personagem:** Sem personagem nomeado do elenco. O TRAÇO é ser O DUAL DO RELÓGIO: o par oposto do corpo métrico (ℝ, a régua). São os dois degenerados opostos do mesmo círculo ℝ/ℓℤ — ℓ→∞ é o métrico (reflexão, sem metade), ℓ=2 é o mórfico (metade, sem reflexão). É parente próximo do corpo criativo (o mesmo anel booleano F₂: ⊕=XOR, ∧=AND).

**A régua.** Mede o RESÍDUO, não uma norma. O corpo é o círculo com comprimento ℓ=2; a medida conservada pelo Contrato (a conservação: resíduo 0) é o resíduo. NÃO HÁ ASSINATURA: o peso de Hamming sobre F₂ não é forma quadrática — não é homogéneo de grau 2 (q(λx)=λ²q(x) falha) e não há lei de inércia de Sylvester em característica 2 (obs:assinatura, medido em elementares/assinaturas.py). A régua T_ω aqui é a torção dada de partida com a máscara: 𝒢=ℤ/2 é finito, sem órbita infinita que preencha a medida.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD = o ANTIROQUE ⊕: o XOR bit a bit de F₂ⁿ. Neutro ⊥ (o vazio/zero). Em ℤ/2 é a deflexão pela metade D₁(x)=x⊕1. Na ISA ERG-64: XOR (a soma exata do corpo booleano, a mesma do corpo criativo). | |
| **⊗ La Hire (o produto)** | (x) LA HIRE = a EROSÃO / o ∧ da álgebra: o AND bit a bit. Neutro ⊤ (o topo/universo). Preserva a multiplicação: ε(A∧B)=ε(A)∧ε(B). Na ISA: AND (a MULT em F₂ colapsa em AND — o double-and-add degenera pois a∧a=a é idempotente). | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN = o operador morfológico que costura: a ADJUNÇÃO δ⊣ε (dilatação ⊣ erosão), com δεδ=δ e εδε=ε, donde φ=εδ e γ=δε idempotentes; a COMPLEMENTAÇÃO ¬ = D₁(x) = x⊕1 = NOT (a deflexão pela metade, a única involução involutiva de ℤ/2); e a COMPOSIÇÃO/boleado γ=δε. Na ISA: NOT (a complementação/o flip) e o dispatch/composição da adjunção. Nota: ¬ é translação pelo topo, NÃO endomorfismo (¬0=1≠0). | |

**Os teoremas (certificados, resíduo 0).**

- é corpo ⟺ n=1 (=F₂): todo a idempotente a∧a=a; n>1 é anel booleano com divisores de zero (0b001∧0b010=0) e elementos sem inverso (nenhum b faz 0b011∧b=0b111) — morfico.py, teo:socorpon1
- o antiroque D₁(x)=x⊕1=¬x é a ÚNICA deflexão involutiva de ℤ/2 (a deflexão pela metade ℓ/2=1, a 2-torção genuína) — morfico.py, teo:antiroquemetade
- MAS NÃO é a reflexão: End(ℤ/2)=F₂, a única involução é ν=1=identidade; −1≡+1 colapsa em char 2; D₁ é translação (D₁(0)=1≠0), não endomorfismo — como permutações [1,0]≠[0,1] — morfico.py, teo:naoereflexao
- mórfico ⊣ métrico são DUAIS: exatamente um de {metade, reflexão ν=−1} é não trivial em cada; ℝ/ℓℤ tem ambos, ℓ→∞ dá o métrico (só reflexão), ℓ=2 dá o mórfico (só metade) — os dois limites degenerados opostos do mesmo círculo — morfico.py, teo:dualmetrico/cor:degenerados
- adjunção δ⊣ε [3000 pares]: δ(A)⊆B ⟺ A⊆ε(B); donde δεδ=δ, εδε=ε; φ=εδ idempotente e extensiva, γ=δε idempotente e anti-extensiva — morfico.py, teo:adjuncao
- cada operador guarda uma operação: erosão preserva ∧ (ε(A∧B)=ε(A)∧ε(B)), dilatação preserva ∨; ε NÃO preserva ∨ — morfico.py, teo:guarda
- dualidade morfológica ε_B(A)=¬δ_{B̌}(¬A) com a antípoda B̌=−B [2000 conjuntos em ℤ/12]: quem conjuga δ em ε é a ANTÍPODA (a reflexão ν=−1 na máscara), não a complementação ¬ — as duas involuções são distintas (¬A≠(−1)A; ¬ não fixa ∅, ν=−1 fixa) — morfico.py, cor:conjuga
- PLACAR: 36/36 certificadas, resíduo 0, ponteiro −0.5 em todas (⟨X,X_⊤⟩=−½·d², distância conforme à afirmação trivial) — morfico.py rodado

**O dual (flip ν).** O flip ν=−1 (a reflexão do contrato) — mas aqui ele COLAPSA na identidade: em característica 2, −1≡+1, End(ℤ/2)={ν=1}. A involução verdadeira sobrevive fora dos endomorfismos, como a COMPLEMENTAÇÃO ¬ (a deflexão pela metade, translação pelo topo). E a reflexão ν=−1 migra para a MÁSCARA, onde vive como a antípoda B̌=−B. O dual GLOBAL do corpo é o corpo MÉTRICO (ℝ): mórfico e métrico trocam reflexão por deflexão-pela-metade, os dois limites opostos do círculo.

**O contrato.** Σ = ℤ/2 (a órbita, ℓ=2, a metade ℓ/2=1). Régua μ = o resíduo (não uma norma; sem assinatura em char 2). Flip ν = a reflexão −1, que colapsa na identidade — a involução efetiva é ¬=D₁ (a deflexão pela metade). Mate = o fechamento ⊤ (ponto fixo da dilatação δ, o universo cheio). O CASAMENTO Γ=0: o fator de potência unitário FP=1 é o fechamento ⊤ (universo cheio, equilíbrio nativo); FP<1 é a ABERTURA — o que a boleado γ apaga, o vazamento que o inversor multinível (Γ=0) recupera degrau a degrau. Sol = a garrafa de Koch áurea (φ^−j), fonte única; energia conserva por a conservação: resíduo 0, mas a medida conservada é o resíduo. Nenhum universo tem FP≠1 (cena_rlc_potencia.py).

**o signo.** TERCEIRIDADE / SÍMBOLO — a lei, a mediação, o hábito. O corpo mórfico é o corpo do OPERADOR posto como lei: a adjunção δ⊣ε e a complementação ¬ são a convenção/mediação (terceiridade) que costura conjuntos. Na tríade: (+)⊕ o antiroque é o par/embate (secundidade/índice, a diferença contestada), (x)∧ a erosão é a mediação, e (op)¬/adjunção é o operador/a lei (terceiridade/símbolo). A primeiridade/ícone do corpo é sua qualidade pura: a característica 2 onde tudo é 2-torção e a reflexão desaparece.

**A JOGADA.** O corpo mórfico governa TERRITÓRIO / ZONAS DE CONTROLE no tabuleiro: cada casa é um bit (dominada=1, livre=0), e cada exército é um conjunto de casas. Os lances são operações morfológicas exatas: (1) EROSÃO ∧/AND (La Hire) — interseção estrutural: só sobrevivem as casas cuja vizinhança inteira já dominas; encolhe a fronteira e consolida o núcleo (cruzar duas zonas dá exatamente as casas comuns, ε preserva o produto). (2) DILATAÇÃO ∨ (o dual de La Hire) — espalha a zona a cada casa adjacente: o avanço de território, preserva a soma reticular. (3) ANTIROQUE ⊕/XOR (Clifford) — a diferença simétrica das duas zonas: as casas contestadas por exatamente um exército, o lance da fronteira; ao invés de proteger o rei (o roque), cede-e-toma a linha de frente. (4) COMPLEMENTAÇÃO ¬/NOT (Pontryagin) — vira o tabuleiro inteiro, ocupado↔vazio (a deflexão pela metade); é involução, dois lances desfazem. O MATE é o fechamento ⊤: encher o universo (ponto fixo da dilatação) — aí FP=1; deixar uma abertura (o que γ apaga) é vazamento FP<1, recuperado degrau a degrau pelo inversor (o casamento Γ=0).

---

## O CORPO ÁUREO ℤ[φ]

**Personagem:** O REI — σ_1 = φ, o ponto fixo do gap metálico σ_m = m + 1/σ_m com m=1. O catálogo o nomeia: "o ouro σ_1=φ é o menor regulador (o fundamental) e o áureo (o quasicristal): o lastro-base" (catalogo.tex, corpo mineral). É o corpo do Rei porque φ é o ÚNICO ponto fixo que se auto-gera (φ = 1+1/φ) — o rei que se sustenta sozinho. A RAINHA aparece no ⊗: a identidade φ²=φ+1 é a lei de La Hire deste corpo, e o produto áureo é literalmente quatro MULT dela (a Rainha executa; o Rei é o valor que a lei fixa).

**A régua.** A régua é o próprio corpo: ℤ[φ] MEDE a cadeia fractal infinita — a torre de Koch/Fibonacci de amplitudes φ^-j — EXATAMENTE, em dois inteiros. Como mede: em vez de somar termo a termo (o que estoura o i64, ou vaza no float), fecha a série geométrica por identidade — Σ_{j≥0} φ^-j = 1/(1-φ^-1); em ℤ[φ]: φ^-1 = φ-1 = (-1,1), 1-φ^-1 = (2,-1) = φ^-2, e 1/φ^-2 = φ² = (1,1). A medida do infinito é o par (1,1). Toda potência φ^n dobra de volta em dois inteiros (φ^n = F_{n-1} + F_n·φ, a órbita do gato A_1); a projeção real (a+bφ ≈ 2.618034 = φ²) só CONFERE a régua, nunca a substitui. T_ω aqui é o regulador vivo log φ > 0 (catalogo.tex: os metais σ_m, de regulador vivo — o boost).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD, a soma, o par: (a+bφ) ⊕ (c+dφ) = (a+c) + (b+d)φ — componente a componente, o par de correntes de Kirchhoff somando em cada trilho (o trilho racional a, o trilho áureo b). NA ISA: DUAS ADD puras (LOAD/LOAD/ADD/STORE em cada coordenada). É a operação mais barata do corpo — o infinito soma-se com dois ADD. | |
| **⊗ La Hire (o produto)** | (x) LA HIRE, o produto, a Rainha: (a+bφ)(c+dφ) = (ac+bd) + (ad+bc+bd)φ — o termo bd·φ² é DOBRADO por φ²=φ+1 e cai metade no trilho racional (bd) e metade no áureo (bd). NA ISA: quatro MULT double-and-add (_mult_block: os quatro produtos ac, bd, ad, bc — cada um o laço AND(mask)/CMP/JZ/ADD-acumula/ADD-desloca(aa+aa, mask+mask)/SUB no contador de 64 bits/JMP), mais as somas do bloco FIN: ADD(40,41)→50 (ac+bd, a parte real) e ADD(42,43)→51 seguido de ADD(51,41)→51 (ad+bc, depois +bd — o dobramento de φ²). Certificado exato em 153 casos, 0 falhas. | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN, o operador que costura: a MULTIPLICAÇÃO POR φ = (0,1) — o shift áureo/Fibonacci (a,b) ↦ (b, a+b), que é exatamente o gato A_1 = [[1,1],[1,0]] de catalogo.tex (σ²=σ+1, autovetor [φ,1]). É o exp·Σ·log do corpo: aplicar o operador n vezes é φ^n = F_{n-1}+F_n·φ — a órbita inteira do infinito gerada por um só lance. NA ISA: é o DISPATCH de ⊗ pelo gerador (0,1) — degenera nas somas do FIN (o produto por (0,1) reduz-se a um swap por STORE e um ADD), a composição sem custo de MULT. É o operador que COMPACTA: o que seria uma expansão infinita vira composição de um shift. | |

**Os teoremas (certificados, resíduo 0).**

- O PRODUTO ÁUREO NA ISA: (a+bφ)(c+dφ) = (ac+bd)+(ad+bc+bd)φ, exato em 153 casos, 0 falhas — quatro MULT double-and-add sobre φ²=φ+1; o corpo ℤ[φ] em INTEIROS, não float, não estoura como o racional (sandbox/tecnicas/regua_corpo_aureo_chessc.py, §1)
- φ² = φ+1, a IDENTIDADE ÁUREA: φ=(0,1); (0,1)⊗(0,1) = (1,1) EXATO no naked Chess, resíduo 0 — é ela que COMPACTA o infinito, toda potência φ^n dobrando de volta em dois inteiros (§2)
- Σφ^-j = φ² = (1,1), o INFINITO CONSTRUÍDO: φ^-1 = φ-1 = (-1,1), 1-φ^-1 = (2,-1) = φ^-2, 1/φ^-2 = φ² — a cadeia fractal fechada pela série geométrica (a projeção real só confirma: →2.618034); o i64 estouraria termo a termo (§3)
- interp ≡ WASM ≡ Dafny: emit_wasm bate com o interpretador em 50 casos (falhas 0) e o Dafny PROVA as leis E CALCULA φ·φ (re=1, o 1 de 1+φ) — a mesma IR, o mesmo ℤ[φ], no metal e provado (§4)
- O ARTEFATO: assets/figuras/wasm/corpo_aureo.wasm, 1694 bytes, magic \0asm — ℤ[φ] na ISA; a régua nunca é float, o PTX/GLSL só no FIM, o pixel (§5)
- PLACAR: sandbox/tecnicas/regua_corpo_aureo_chessc.py — 5/5 certificadas, resíduo 0 (ponteiro -0.5 em todas as 5)
- Herdados do catálogo (catalogo.tex): os metais σ_m=(m+√(m²+4))/2, o gato A_m, σ_m²=mσ_m+1, com dual σ_m'=-1/σ_m (σ_mσ_m'=-1) — corpo_mineral.py 7/7 resíduo 0; e a assinatura CF σ_m=[m;m,…] (ouro φ=[1;1,…]) — assinatura_orbita.py 6/6 resíduo 0

**O dual (flip ν).** O flip ν é a CONJUGAÇÃO DE GALOIS φ ↦ φ' = -1/φ = 1-φ = (1,-1) — em catalogo.tex: "o seu DUAL é σ_m'=-1/σ_m (a contração, σ_mσ_m'=-1): o par EXPANDE ⋈ CONTRAI". φ é o regulador vivo (|φ|>1, o boost, a expansão); φ' é a contração (|φ'|=φ^-1<1, o decaimento). A complementaridade: N(a+bφ) = (a+bφ)(a+bφ') = a²+ab-b² é a NORMA (inteira, exata) — o invariante que o flip fixa; e é a contração φ' que faz a cadeia Σφ^-j convergir, ou seja, é o DUAL que torna o infinito somável e o PRIMAL que o torna gerável. Primal ⋈ dual: expandir ⋈ dobrar de volta.

**O contrato.** Σ = ℤ/n: a órbita é a do gato A_1 = [[1,1],[1,0]] — o shift (a,b)↦(b,a+b); a órbita real é densa/incomensurável (o quasicristal, catalogo.tex: "o ventricular — o pulso, o áureo/incomensurável, a órbita densa"), e o relógio ℤ/n é a redução da órbita de Fibonacci módulo n (o período de Pisano). RÉGUA μ: a norma inteira / a cadeia Σφ^-j = φ² = (1,1), medida em dois inteiros, resíduo 0. FLIP ν: a conjugação φ↦-1/φ (expansão ⋈ contração, σσ'=-1). MATE: a dobra — toda potência reduzida a (F_{n-1},F_n), o infinito fechado. O CASAMENTO Γ=0 / FP=1: aqui é O NÃO-VAZAMENTO POR CONSTRUÇÃO — a cadeia fractal fecha EXATAMENTE (Σφ^-j = φ², não uma soma truncada), logo não há resíduo de arredondamento a escapar; o float vazaria, ℤ[φ] não. É o mesmo casamento que o catálogo descreve na energia: a distorção áurea THD = φ^{-1/2} vem desta identidade, e "o inversor multinível (Pontryagin) casa a torre de Fibonacci da bateria e a distorção some" → FP=1 (catalogo.tex, linhas 214-215). O corpo áureo é o LASTRO do casamento: a única régua que mede a cadeia infinita sem perder um bit.

**o signo.** TERCEIRIDADE / SÍMBOLO — a LEI que medeia. O corpo áureo é a lei φ²=φ+1 tomada como HÁBITO: ela não descreve um caso, ela DOBRA todos os casos (toda potência, toda a cadeia infinita) de volta em dois inteiros; é convenção/mediação pura, o operador Pontryagin como lei. Sua PRIMEIRIDADE/ÍCONE subjacente é a autossemelhança da cadeia φ^-j (a parte é a imagem do todo — a torre de Koch, o quasicristal), e a Secundidade/índice aparece no embate primal⋈dual (expansão ⋈ contração, σσ'=-1). Mas o corpo, enquanto corpo, é o símbolo: a identidade que rege.

**A JOGADA.** O LANCE ÁUREO — A DOBRA. No universo: uma peça carrega uma cadeia INFINITA de golpes que decaem (φ^-1, φ^-2, φ^-3, … a torre de Koch/Fibonacci, o eco que nunca acaba). Num motor comum o eco teria de ser TRUNCADO (o dano vaza, a partida deixa de ser reversível). No Reino Dourado o Rei DOBRA a cadeia: a identidade φ²=φ+1 fecha o eco infinito no par (1,1) — o golpe infinito resolve num único tick, com dano EXATO φ² = 2.618… e zero arredondamento. Mecânica jogável: (1) o MOVIMENTO — a peça áurea anda pelo operador (a,b)↦(b,a+b): cada turno avança a soma dos dois últimos passos (a escada de Fibonacci, a espiral); é o único lance do tabuleiro que acelera sem gastar recurso, porque a órbita é gerada, não somada. (2) a PARADA/o dual — o adversário para com o flip conjugado φ'=-1/φ (a CONTRAÇÃO): o escudo (a,b)↦(a+b,-b) encolhe o golpe por φ^-1 a cada camada; o par expande ⋈ contrai, σφ'=-1, e a soma de tudo é limitada por φ² — nada vaza. (3) a MOEDA — o ouro do reino É ℤ[φ]: todo dano, todo recurso, todo eco é um par de inteiros, nunca float; logo toda partida é replayável bit a bit e o mate é auditável (resíduo 0). (4) o MATE — a posição em que toda potência do adversário foi reduzida a (F_{n-1}, F_n): não há para onde expandir, a cadeia infinita já está fechada em dois inteiros. O infinito não é derrotado — é DOBRADO.

---

## O corpo RACIONAL ℚ

**Personagem:** A RAINHA (La Hire ⊗) — o próprio certificado cita: "sandbox/tecnicas/mult_rainha_chessc.py — a MULT ⊗ (double-and-add): aqui, COMPOSTA DUAS VEZES no corpo ℚ" (uma Rainha no numerador, outra no denominador). E o corpo é, literalmente, o CASAL das duas camadas do teorema: a multiplicidade ADITIVA a = o numerador (o Duque ⊕, Joaquim) sobre a multiplicidade de ESCALA b≠0 = o denominador (a Duquesa ⊗, Yasmin) — o racional é o par ⊕/⊗. Ao fundo, o REI: a régua de Euclides é medir ao resto 0, e o resto 0 é o ponto fixo que fecha a medição.

**A régua.** Mede RAZÃO: quanto de b cabe em a (a/b = a solução de b⊗x=a). A régua T_ω é EUCLIDES — a fração contínua a/b=[a₀;a₁,…,aₙ], "medir ao resto 0", gerada pela recursão de Möbius x↦aᵢ+1/x (o gato A_m=[[m,1],[1,0]], os convergentes pₙ/qₙ). Ela é a ASSINATURA da órbita: INVARIANTE de escala — CF(a/b)=CF(ka/kb), o k some — logo colapsa a classe inteira e é única, não uma escolha (catalogo.tex, teo:assinatura). O critério da régua: o racional é exatamente aquele cuja medição TERMINA (CF finita, resto 0); o irracional nunca fecha (CF infinita = um METAL, e os metais completam ℚ→ℝ). Em medida.tex: o ponto racional exige programa FINITO (Kolmogorov K(x) finito); o irracional, informação infinita. A régua carrega-se em i64 exato — nunca em float (float = 1 ulp de vazamento).

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford (a soma)** | (+) CLIFFORD, a SOMA: (a/b) ⊕ (c/d) = (a·d + b·c)/(b·d) — a corrente de Kirchhoff sobre denominador comum, o PAR das duas camadas cruzando. Custo na ISA: três MULT + uma ADD (regua_racional_chessc.py, docstring linha 16). Na ISA ERG-64: o opcode ADD (i64, exato) soma os produtos cruzados a·d e b·c — cada produto sendo ele mesmo um laço double-and-add. Nenhum float, nenhuma truncagem. Certificada bem-definida na CLASSE (independe do representante da órbita): torre_racionais.py §3, [(a,b)]+[(c,d)]=[(ad+bc,bd)]. | |
| **⊗ La Hire (o produto)** | (x) LA HIRE, o PRODUTO — a Rainha: (a/b) ⊗ (c/d) = (a·c)/(b·d), DUAS MULT (a Rainha composta duas vezes: uma no numerador, uma no denominador). Na ISA é a MULT por DOUBLE-AND-ADD: ⊗ = Σ∘deslocamento — o programa naked chess em regua_racional_chessc.py (blocos M1 e M2) roda 64 ticks: AND (testar o bit da máscara) → CMP/JZ (se o bit é 1) → ADD (acumular o parcial) → ADD+ADD (dobrar máscara e parcial = o deslocamento) → SUB (decrementar o contador). O produto racional é INTEIRO e EXATO: resíduo 0 REAL, não 1 ulp. Certificado: num=a·c e den=b·d exatos em 202 casos, falhas 0, e Fraction(num,den) == Fraction(a,b)·Fraction(c,d). | |
| **∏ Pontryagin (o operador)** | (op) PONTRYAGIN, o operador que costura, em três encarnações neste corpo: (1) O DESLOCAMENTO que faz o produto virar soma — ⊗ = Σ∘deslocamento é exp∘Σ∘log em i64: o log é o bit (a escala 2^k), a Σ são os 64 ADDs deslocados, o exp é a máscara dobrando. Na ISA: o DISPATCH/COMPOSIÇÃO — CMP / JZ / JNZ / JMP, o laço que compõe os 64 lances num só produto (mais AND, o teste do bit). (2) A TROCA DE CAMADAS — [(a,b)]⁻¹ = [(b,a)]: trocar numerador↔denominador. É ele que FAZ VIRAR CORPO (todo x≠0 invertível); na ISA é puro LOAD/STORE (swap dos slots 10↔20), zero aritmética. (3) O QUOCIENTE DA CLASSE — a redução ao representante irredutível (gcd=1, q>0) via Euclides: o operador que apaga a redundância e colapsa a órbita ao seu gerador (a CF). | |

**Os teoremas (certificados, resíduo 0).**

- regua_racional_chessc.py — 4/4, resíduo 0 (rodado agora): [1] O PRODUTO RACIONAL NA ISA: (a/b)·(c/d)=(a·c)/(b·d) em naked chess, num=a·c e den=b·d EXATOS em 202 casos (falhas 0), e Fraction(num,den) == Fraction(a,b)·Fraction(c,d) — exato, inteiro, sem truncagem, resíduo 0 REAL (não 1 ulp).
- regua_racional_chessc.py [2] — interp ≡ WASM: emit_wasm roda o produto racional no metal (wasmtime, o mesmo do Chrome) e bate com o interpretador em 60/60 casos (falhas 0).
- regua_racional_chessc.py [3] — interp ≡ WASM ≡ Dafny: o Dafny PROVA as leis (dafny verify, Z3, 0 errors) E CALCULA a máquina (dafny run --target:py): (3/4)·(5/7) dá num=15 (=3·5), den=28. A costura fecha; o Dafny é quem calcula o EXATO (BigRational) sem teto de 1 ulp.
- regua_racional_chessc.py [4] — o ARTEFATO: assets/figuras/wasm/racional.wasm (880 bytes, magic \0asm) — o corpo ℚ na ISA, emitido pelo chessc.
- elementares/torre_racionais.py — 4/4, resíduo 0 (rodado agora), cit. catalogo.tex teo:torre-racionais: [1] a 3ª torre são as órbitas por duas multiplicidades (a,b): a razão a/b = solução de b⊗x=a; [2] a redundância classifica — (p,q)~(p',q') ⟺ pq'=p'q, (2,4)≡(1,2) é a mesma órbita, o representante é o irredutível (gcd=1); [3] o ISOMORFISMO com ℚ — [(a,b)]↦a/b é homomorfismo bijetor, soma [(ad+bc,bd)] e produto [(ac,bd)] BEM-DEFINIDOS (independem do representante); [4] o CORPO — o inverso multiplicativo [(a,b)]⁻¹=[(b,a)] é TROCAR as camadas, e ℚ completa a cascata ℕ→ℤ→ℚ.
- elementares/classes_inteiros_racionais.py — 5/5, resíduo 0 (rodado agora), cit. catalogo.tex teo:classes-zq: ℤ = classes ADITIVAS (Grothendieck, (a,b)~(c,d) ⟺ a+d=b+c, a mesma diferença) e ℚ = classes MULTIPLICATIVAS ((p,q)~(p',q') ⟺ pq'=p'q, a mesma razão), ℚ=(ℤ×ℤ*)/~; φ([(p,q)])=p/q é isomorfismo de CORPOS (soma, produto, inverso).
- catalogo.tex teo:assinatura — a fração contínua é a assinatura invariante da classe (CF(a/b)=CF(ka/kb)); os metais (CF periódica, Lagrange) completam ℚ→ℝ. Certificado citado: assinatura_orbita.py (6/6, resíduo 0).
- catalogo.tex teo:torres-nz — ℤ AINDA é anel (2 não tem inverso multiplicativo); o corpo ℚ (corpo de frações) é a terceira torre. Certificado citado: torres_operador.py (4/4, resíduo 0).

**O dual (flip ν).** O FLIP ν = a TROCA DAS DUAS CAMADAS: [(a,b)] ↦ [(b,a)] — numerador↔denominador. É o inverso MULTIPLICATIVO, e primal ⊗ dual = [(ab,ba)] = [1] = o neutro multiplicativo (torre_racionais.py §4, certificado). É a complementaridade que FAZ VIRAR CORPO: em ℤ (o anel) o 2 não tinha inverso; em ℚ todo x≠0 tem. Na ISA: puro swap de slots (10↔20), zero aritmética. Note-se a bela simetria da cascata: o MESMO gesto de trocar o par é, em ℤ (classes aditivas, Grothendieck), o inverso ADITIVO (a,b)↦(b,a) com neutro 0; e em ℚ (classes multiplicativas), o inverso MULTIPLICATIVO com neutro 1. Um só flip, os dois neutros.

**O contrato.** Σ = ℤ/n: o relógio é a PALAVRA i64 — o double-and-add percorre uma órbita de 64 ticks (.data 13 64 / .data 23 64, o contador; a máscara 1,2,4,…,2⁶³ a dobrar, wrap por 2⁶⁴); e, acima, a órbita de CLASSE: o grupo de escala ℤ* agindo em (p,q)↦(kp,kq), cujo quociente é o corpo (a redundância que o relógio apaga; o representante é a órbita primitiva, gcd=1).
RÉGUA μ: a razão, medida por Euclides/fração contínua (T_ω: x↦aᵢ+1/x), invariante de escala e terminando em resto 0.
FLIP ν: [(a,b)]⁻¹=[(b,a)] — primal ⊗ dual = 1 (o neutro).
MATE: o resto 0 de Euclides — a CF FECHA (finita) ⟺ o número é racional. O irracional (o metal) não é matável por esta régua: é aí que ℚ→ℝ.
CASAMENTO Γ=0 / FP=1 (o ponto duro): o corpo ℚ é o casamento perfeito de impedância do CÁLCULO. A ISA ERG-64 é inteira e exata: o produto racional não trunca — resíduo 0 REAL, não "1 ulp". O float32 do GLSL NÃO é a régua: seria uma impedância descasada (Γ≠0, 1 ulp de energia refletida a cada operação = vazamento, FP≠1). Por isso a lei: o EXATO corre no chessc/Dafny (ℚ = num/den, ou ℤ[φ] no corpo áureo), e o float entra SÓ NO FIM, no pixel, onde 1 ulp é invisível. Nenhum universo pode vazar. E a costura que atesta o casamento: interp ≡ WASM ≡ Dafny, o mesmo corpo ℚ da mesma IR (falhas 0).

**o signo.** SECUNDIDADE / ÍNDICE — este é o corpo do PAR. A qualidade própria de ℚ é a diádica: um racional não é uma coisa, são DUAS multiplicidades em embate (numerador contra denominador), e a razão a/b é a REAÇÃO entre elas — a solução de b⊗x=a, uma conexão factual, existencial, entre duas camadas do operador. O índice aponta para o outro: o denominador só significa contra o numerador. (A terceiridade/símbolo entra por cima, como a LEI que rege o par: a classe pq'=p'q e a fração contínua — a convenção que colapsa infinitos pares num só gerador invariante. E o mate/resto 0 de Euclides é o hábito, a lei que fecha.) Mas o corpo, em si, é o embate do par: ⊕ Clifford, a secundidade.

**A JOGADA.** A LINGUAGEM DO UNIVERSO: toda peça se move por uma RAZÃO — o lance é um par (num, den): o numerador é o AVANÇO (a camada aditiva, o passo do Duque ⊕) e o denominador é a MARCHA/TEMPO (a camada de escala, a engrenagem da Duquesa ⊗). Casas por tick = num/den. Quatro mecânicas, todas jogáveis:
(1) A CARGA DA RAINHA (⊗ = double-and-add): amplificar um lance por um ganho c/d não é "multiplicar" — a peça EXECUTA 64 micro-lances bit a bit (testa o bit → soma deslocado → dobra). O ganho é exato: o combate acumula sem vazar um único ulp. Duas cargas (uma no numerador, uma no denominador) = uma jogada de Rainha.
(2) O CONTRA-GOLPE PELA RECÍPROCA (o flip ν): um ataque de razão 3/4 é ANULADO exatamente por sua recíproca 4/3 — primal ⊗ dual = 1, o neutro. Trocar as duas camadas é o lance de defesa perfeita (Γ=0: a energia do golpe casa e nada reflete). É o inverso que faz o corpo virar corpo: em ℚ, todo ataque não-nulo TEM defesa exata (em ℤ, não tinha).
(3) O DUELO DE EUCLIDES (o mate): dois lances racionais se enfrentam medindo-se um no outro — subtrai-se o menor do maior, repetidamente (a fração contínua). Quem chega ao RESTO 0 dá o mate. Duelo entre racionais SEMPRE termina (CF finita = mate em número finito de lances). Contra um METAL (irracional, CF infinita: φ, prata, bronze) o duelo NUNCA fecha — o metal não pode ser matado pela régua racional: é o xeque perpétuo, e é exatamente por esse buraco que ℚ se completa em ℝ.
(4) A LEI DA LIMPEZA (o gcd=1): 2/4 e 1/2 são a MESMA casa — o tabuleiro quocienta a redundância. Inflar as duas camadas (×k) não gera vantagem nenhuma: não há energia de graça. E a regra de ouro do reino: nenhuma mecânica pode ARREDONDAR no meio da partida — o estado corre em pares (i64, i64); o float só aparece ao pintar o pixel. Arredondar é vazar (FP≠1) — lance ilegal.

---

## Os 8 corpos restantes — completando o mapa

> Estes ficaram de fora da primeira passagem (os agentes esbarraram no limite). Foram lidos e mapeados
> diretamente dos `.tex` e dos certificados `.py` — os placares abaixo foram **rodados**, não citados de memória.

---

## técnico — *as Princesas: quem atesta a lei*

**Personagem:** as **Princesas** (o corpo técnico, o kernel LCF — Dafny, Isabelle).

**A régua.** Não produz **estado**: produz **veredito**. É o dual do criativo (gerar ⋈ atestar) — o corpo que
**mede 0**. A régua é o próprio resíduo: `x ⊕ x = 0`.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a diferença simétrica — `x ⊕ x = 0` é o **cristal** (o veredito: nada sobrou) | `XOR` |
| **⊗ La Hire** | a conjunção das hipóteses (todas devem valer) | `AND` |
| **∏ Pontryagin** | a **refutação** — a involução que tenta derrubar a afirmação | `NOT` |

**Os teoremas.** `tecnico.py` — **9/9, resíduo 0**. A advertência que o próprio corpo grava: *"`x ⊕ x = 0` passa nos
`k` casos, mas **a amostra não é o universo**"* — por isso o Dafny (o Z3) prova para **todo** `x`, não por amostra.

**o signo.** Terceiridade / **símbolo** — a lei, a convenção que julga.

**A JOGADA.** A **atestação**. Antes de um lance valer, uma Princesa o audita: aplica `x ⊕ x` sobre o estado
declarado e o real. Se o resíduo for `0`, o lance é **legal**; se não, o lance **não aconteceu** (é rejeitado, não
punido). É a única peça que não ataca — e sem ela nenhum universo abre.

---

## rotor — *a velocidade que não passa da luz*

**A régua.** A **rapidez** `φ = artanh(v)`. O rotor mede o giro; a sua régua é hiperbólica.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a **soma relativística**: `v₁ ⊕ v₂ = (v₁+v₂)/(1 + v₁v₂)` — nunca ultrapassa 1 | `ADD` + `MULT` (a razão) |
| **⊗ La Hire** | o produto que a soma esconde (o denominador `1 + v₁v₂`) | a MULT (double-and-add) |
| **∏ Pontryagin** | `φ = artanh(v)` — **leva a soma ao produto**: `φ(v₁ ⊕ v₂) = φ₁ + φ₂` | a composição `exp∘Σ∘log` |

**Os teoremas.** `rotor.py` — **4/4, resíduo 0**. A identidade central: **a rapidez SOMA** (`φ₁ + φ₂`) enquanto a
velocidade só se compõe. O operador é exatamente o que lineariza — é a assinatura de Pontryagin.

**A JOGADA.** O **impulso**. Somar velocidades nunca te leva além da luz (`v < 1` sempre) — mas as **rapidezes**
somam sem teto. Quem joga no `v` satura; quem joga no `φ` acumula. A peça que aprende a mudar de régua ganha.

---

## cósmico — *a expansão*

**A régua.** A **lei de potência** (a escala do universo). Mede pelo expoente, não pela grandeza.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma das escalas (os logs somam) | `ADD` |
| **⊗ La Hire** | o produto `· → ⋆` (a convolução das escalas) | a MULT |
| **∏ Pontryagin** | a **expansão** `a(t) = e^{Ht}` — a mesma tríade do **econômico** (os juros são a expansão) | `exp∘Σ∘log` |

**Os teoremas.** `cosmico.py` — o resíduo da lei de potência é `‖b̃₂‖²`, o segundo vetor do GSO (Gram-Schmidt):
planetas `3.09e-05`, luas `6.01e-09`. **Resíduo nulo** = o Princípio da Medida Consistente numa lei de potência.

**A JOGADA.** A **escala**. Um lance cósmico não move a peça: move o **tamanho do tabuleiro**. Tudo se conserva em
proporção (é o mesmo jogo, maior) — mas quem mede em grandeza absoluta se perde; quem mede em expoente, não.

---

## universal — *a raiz: o sucessor*

**A régua.** A **contagem**. É o corpo mais elementar e o mais fundo: dele todos os outros descem.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma é o **sucessor iterado** (`x + n` = aplicar `S` n vezes) | `ADD` |
| **⊗ La Hire** | a multiplicação é a **soma iterada** | a MULT (double-and-add) |
| **∏ Pontryagin** | o **sucessor** `S(x) = x + 1` — o operador que gera tudo. E `x ⊕ 1 = ¬x` (no bit) | `ADD 1` ≡ `NOT` (em 𝔽₂) |

**Os teoremas.** `universal.py` — **57/57, resíduo 0** (o maior placar do catálogo). É a raiz: o sucessor gera a
soma, a soma gera o produto, o produto gera o corpo.

**A JOGADA.** O **passo**. É o lance mínimo — `+1`. Não tem poder nenhum e é o que sustenta todos os outros: toda
jogada do reino, decomposta, é uma pilha de sucessores. Quem entende isso lê a partida na sua unidade.

---

## nervoso — *a rede*

**A régua.** Não o neurônio: a **REDE**. A novidade do corpo é que os neurônios **não são atratores isolados** — o
que mede é a conexão.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma (a corrente que chega ao nó — Kirchhoff literal) | `ADD` |
| **⊗ La Hire** | o produto (o **peso** sináptico: o ganho) | a MULT |
| **∏ Pontryagin** | o operador de ativação — a rede que recorre | `NOT` / o dispatch |

**Os teoremas.** `nervoso.py` — **4/4, resíduo 0**.

**A JOGADA.** A **cadeia**. Um lance nervoso não atinge uma peça: atinge **a rede dela**. O dano se propaga pelos
pesos (⊗) e se soma nos nós (⊕). Isolar uma peça é a defesa; conectá-la é o poder — e o risco.

---

## exterior — *o produto que se parte em dois*

**A régua.** O que **falta** — o resíduo do completamento. O corpo mede pela dimensão que sobra.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma dos multivetores (grau a grau) | `ADD` |
| **⊗ La Hire** | o **produto de Clifford**, que se **PARTE EM DOIS**: `ab = a·b + a∧b` (o interior + o exterior) | a MULT + `XOR` (o grau) |
| **∏ Pontryagin** | o operador de **Volterra** (a integral que acumula) | `exp∘Σ∘log` |

**Os teoremas.** `exterior.py` — *"o (n+1)-ésimo resíduo é ZERO: a régua de ℝⁿ tem exatamente **n** passos"*. E o
refutador com dentes: *"põe 5 vetores em ℝ⁶ — sobra dimensão, o resíduo **não morre**"*. **O corpo detecta o que
falta.**

**o signo.** Secundidade / **índice** — o embate (o produto que se parte em dois é o encontro).

**A JOGADA.** O **corte**. O produto exterior `a∧b` mede a **área** que dois lances varrem juntos — e é **zero** se
forem paralelos. Atacar na mesma direção do inimigo não gera nada; atacar **transversal** gera a lâmina. É o corpo
que ensina a jogar de través.

---

## sensitivo — *a régua hiperbólica*

**A régua.** **Hiperbólica**, assinatura de Sylvester `(1,1,0)`, curvatura `k = +1`.

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma (o lado arquimediano) | `ADD` |
| **⊗ La Hire** | o produto que **casa as normas**: `\|x\|_∞ · \|x\|_2 = 1` (a dualidade das réguas) | a MULT |
| **∏ Pontryagin** | o operador que troca o arquimediano pelo `p`-ádico (o espaço-temporal) | o dispatch |

**Os teoremas.** `sensitivo.py` — **15/15, resíduo 0**. A identidade `\|x\|_∞ · \|x\|_2 = 1` é o **casamento** neste
corpo: as duas réguas são inversas uma da outra (é o Γ=0 dele).

**A JOGADA.** A **percepção**. O corpo sensitivo é quem **enxerga** o inimigo: mede pela norma dual. Um golpe que
maximiza `\|·\|_∞` (o pico) minimiza `\|·\|_2` (a energia) — e vice-versa. Não dá para ser preciso e forte no mesmo
lance. Escolher a régua **é** o lance.

---

## deflexivo — *o operador que não pertence ao sistema que o gera*

**A régua.** A **reflexão**. E a lição mais dura do reino: *"o operador `A_m` **não pertence ao sistema que o
gera** — pertence ao **completamento**."*

| a tríade | neste corpo | na ISA |
|---|---|---|
| **⊕ Clifford** | a soma | `ADD` |
| **⊗ La Hire** | o produto é a **reflexão** (`p ↦ -p`, a deflexão) | a MULT + `SUB` (o sinal) |
| **∏ Pontryagin** | o operador `A_m` — o **gato**, que vive fora do sistema (no completamento) | `exp∘Σ∘log` |

**Os teoremas.** `deflexivo.py` — e aqui está o **aviso mais importante do catálogo inteiro**:

> *"resíduo exato = `1.14e-16`; mutar `Fraction` → `float` rende exatamente `0.0`"*

Ou seja: **o float dá um ZERO FALSO.** O resíduo verdadeiro (em racional exato) **não é zero** — o float apenas não
o enxerga. É a razão pela qual a régua do reino é o exato (ℚ, ℤ[φ]) e o float só aparece no pixel. E o áureo: *"a
média temporal do resíduo → `ℓ/2` (`0.500002`); mutar `θ` para a órbita racional `1/4` dá `0.375 ≠ 1/2`"* — só a
órbita **irracional** (áurea) equidistribui.

**A JOGADA.** A **deflexão**. Não se bloqueia um golpe: **desvia-se**. A reflexão `p ↦ -p` devolve o ataque com o
sinal trocado — e o operador que a executa **não está no tabuleiro** (vive no completamento). É a peça que age de
fora: o gato. Quem confia no zero do float acha que está seguro — e não está.

---
