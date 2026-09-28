CONTRATO X3 — SALTO DIMENSIONAL DE BYTES: X₂ → X₃
═══════════════════════════════════════════════════════════════════════════════

    Documento:  CONTRATO_X3.md
    Versão:    2026-09
    Âmbito:    definir a passagem de X₂ para X₃ na família de bytes, o oráculo
               que a avalia, os controlos negativos que a distinguem de
               substituições, e o critério de fecho. NÃO define a aritmética de
               X₃, NÃO implementa, NÃO mede.
    Estado:    ABERTO. A seta está aberta por aqui. Nada foi implementado e nada
               foi medido. O critério de fecho está em §10 e ainda não foi
               cumprido por ninguém.


0. NOTAÇÃO E ÂMBITO — LER ANTES DE TUDO

Neste documento, X_d refere-se exclusivamente à família de espaços de bytes
definida em redes/campos.tex, secção sec:byte-carry. A notação X₃ aqui não se
refere à cadeia X₁→X₂→X₃→X₄ da construção da escada existente em outros
documentos do projecto.

Esta declaração não é cosmética. A desambiguação é por ESCOPO DOCUMENTAL, e
não por renomeação: a família de bytes continua a chamar-se X_d em campos.tex,
dentro de sec:byte-carry, e assim continuará. Uma renomeação global para
X_d^bytes foi considerada e rejeitada, por duas razões que valem registar:

  • campos.tex usa X_d com dois sentidos legitimados. Em sec:byte-carry,
    linha 386, X_d := (Z/256Z)^d é a família de bytes. Em
    sec:nivel-peso-escala, linha 595, X_d := {0,…,b−1}^d é a família genérica
    de base b, da qual a anterior é o caso particular b=256. São duas secções
    matematicamente ligadas, e o commit a44b3c5e limitou-se a declarar essa
    ligação numa frase, sem introduzir símbolo novo. Renomear só uma delas
    criaria uma incoerência onde antes havia uma sobrecarreg contextual
    legítima.

  • A colisão nominal com a escada não existe ainda em campos.tex. Uma auditoria
    de impacto devolveu ZERO ocorrências de X_1, X_2, X_3, X_4 e zero
    ocorrências de "escada", "degrau" ou "quociente" no ficheiro. O que colide é
    o X₃ que ESTE salto ia introduzir, e é aqui que se resolve: por âmbito.

Símbolos proibidos neste documento, e por quê: X₃^bytes, BX₃, Y₃, X₃′.
Nenhum é necessário. Introduzi-los seria criar uma terceira notação para
resolver uma colisão que o âmbito já resolve.

A definição canónica da seta NÃO é copiada para aqui. É a de campos.tex,
sec:byte-carry, def:iota-d (linhas 397–427). Este documento aponta para lá e
mede-se contra lá. Duas fontes para a mesma definição seriam uma fonte de
divergência.


1. OS DOIS ESPAÇOS

  X₂ := (Z/256Z)²          |X₂| = 256 · 256 = 65536   = 2¹⁶
  X₃ := (Z/256Z)³          |X₃| = 256³      = 16777216 = 2²⁴

Ambos são instâncias de X_d := (Z/256Z)^d, campos.tex:386, para d = 2 e
d = 3. Um elemento de X₂ escreve-se (b₀, b₁) e um de X₃ escreve-se
(b₀, b₁, b₂), com b_k ∈ {0, …, 255}, as coordenadas independentes entre si.

A ordem é little-endian, a do paper: b₀ tem peso 256⁰ e é o menos significativo;
b₂ tem peso 256² e é o mais significativo. A convenção não é uma escolha deste
documento — é a de campos.tex:392, onde C_d(b₀,…,b_{d−1}) = Σ b_k 256^k.


2. A SETA SOB TESTE

  ι₂ : X₂ → X₃,      ι₂(b₀, b₁) = (b₀, b₁, 0)

Isto é a instância d = 2 da definição recursiva única de def:iota-d
(campos.tex:397–427), que para d geral dá

  ι_d : X_d ↪ X_{d+1},    ι_d(b₀,…,b_{d−1}) = (b₀,…,b_{d−1}, 0).

Registado com força, porque é o ponto onde um documento como este costuma
exagerar: ι₂ NÃO É UMA NOVA LEI. NÃO É UM RESULTADO NOVO. NÃO É UMA
DESCOBERTA. É a substituição de d por 2 numa definição que já existia, já
estava escrita e já tinha sido publicada no commit 25a19dfa. O que este
documento acrescenta é a instrumentação, o oráculo e os controlos — não a
matemática.

O que a definição canónica garante, e que este contrato não redefine mas
invoca:

  • injectividade — a projecção para as primeiras d coordenadas compõe com
    ι_d na identidade (campos.tex:410–411);
  • componente acrescentado nulo, na posição de peso 256^d;
  • encadeamento — ι_{d+1} ∘ ι_d acrescenta um segundo zero;
  • invariância da codificação — C_{d+1}(ι_d(x)) = C_d(x), campos.tex:424,
    provada pelo caso r=1 da concatenação de blocos, campos.tex:671.


3. REPRESENTAÇÃO E A IDENTIDADE DE COLAPSO

3.1 Codificação

  C₃(b₀, b₁, b₂) = b₀ + 256·b₁ + 65536·b₂
  0 ≤ C₃ ≤ 2²⁴ − 1 = 16777215

C₃ é a instância d = 3 de C_d, campos.tex:392. uint32_t é suficiente: 2²⁴
valores cabem em 32 bits sem sinal com folga de 2⁸, e o valor máximo
representado, 16777215, é o topo exacto do intervalo. Não há overflow a
evitar neste contrato. Registado porque o salto anterior, com C₂ em uint16_t,
tinha o topo exacto do intervalo e esse facto deve continuar explícito.

3.2 A identidade que este salto introduz

  C₃(ι₂(b₀, b₁)) = C₃(b₀, b₁, 0) = b₀ + 256·b₁ = C₂(b₀, b₁)

A última igualdade é a C₂ de CONTRATO_X2.md, com a mesma convenção de
coordenadas. É a instância d = 2 de C_{d+1}(ι_d(x)) = C_d(x).

Esta identidade é a característica específica DESTE salto, e tem uma
consequência quantitativa que convém não esconder: o valor codificado da
imagem de ι₂ vive todo no fundo da escala.

  imagem de ι₂, em espaço de códigos   =   {0, …, 65535}
  espaço de códigos total             =   {0, …, 16777215}
  códigos fora da imagem              =   16711680

Ou seja: a imagem de ι₂ ocupa 65536 dos 16777216 códigos possíveis, uma fracção
de 1/256. A invariância sob C é real e o paper garante-a, mas é uma
identidade estreita, e o instrumento tem de a tratar como tal.

3.3 As três projeções, e a obrigação de canal

Este contrato usa INDEXAÇÃO 0-BASED, a do paper, em que a coordenada b_k tem
peso 256^k e ocupa a posição k. As três projeções de X₃ são, por essa
convenção,

  π₀(y) = b₀        π₁(y) = b₁        π₂(y) = b₂

isto é, π₀ lê a primeira posição, π₁ a segunda e π₂ a terceira. Cada uma é
uma função total de X₃ para Z/256Z, e cada uma lê uma posição e nada mais.

Sobre a imagem de ι₂, e por consequência da definição em §2, vale

  π₀(ι₂(b₀,b₁)) = b₀        π₁(ι₂(b₀,b₁)) = b₁        π₂(ι₂(b₀,b₁)) = 0

Aviso de convenção, porque dois documentos deste projecto usam π₁ com sentidos
diferentes. CONTRATO_X2.md §3 escreve π₁(x) = b₀ e π₂(x) = b₁, numerando as
posições a partir de 1. ESSA NOTAÇÃO PERTENCE AO CONTRATO ANTERIOR E NÃO É
REINTERPRETADA NEM ALTERADA AQUI: π₁ significa b₀ em CONTRATO_X2.md e
significa b₁ neste documento, e as duas leituras estão cada uma correcta no
seu contrato. CONTRATO_X2.md permanece congelado no checkpoint 25a19dfa e não
é tocado por esta frente. A divergência fica declarada, não harmonizada.

OBRIGAÇÃO DE CANAL. Este contrato obriga a que a imagem de ι₂ seja observada
por TRÊS canais lidos separadamente, e a que os três participem do veredicto.
Nenhum canal pode falhar enquanto o instrumento continua a reportar PASS. Isto
é uma condição do contrato: não é uma optimização, não é preferência de
estilo, e não é negociável pelo instrumento.

Âmbito desta obrigação, dito com precisão. C₃ é uma bijecção de X₃ para
{0, …, 2²⁴−1}, logo comparar apenas o valor codificado resolveria a questão
abstracta: v = C₃(b₀,b₁,0) se e só se a saída é (b₀,b₁,0). A exigência dos
três canais NÃO é uma afirmação de que sejam necessários para provar essa
igualdade abstracta. Não são. É uma exigência INSTRUMENTAL e de AUDITORIA, e
vale mesmo que a igualdade se possa resolver por via mais curta.

A obrigação das três projeções existe por duas razões que não são de
correctude abstracta e que valem tanto:

  (a) ATRIBUIÇÃO DE FORÇA AOS CONTROLOS. Os três controlos negativos de §7
      violam exactamente um canal cada: A viola π₂, B1 viola π₀, B2 viola π₁.
      Se o instrumento observasse um único escalar, os três seriam
      indiscerníveis entre si e não se provaria que cada um tem força. Com três
      canais, a falha é atribuída a um canal nomeado. A exigência de canal é o
      que torna a negativa de §7 informativa em vez de decorativa.

  (b) INDEPENDÊNCIA DO INSTRUMENTO. A disciplina de oráculo de CONTRATO_X2.md
      §5 e §6 exige que o valor esperado seja construído a partir da definição
      e não obtido chamando a função auditada. A mesma disciplina, aplicada à
      OBSERVAÇÃO: se o instrumento lê uma única palavra de 32 bits e deriva
      dela tudo o mais, uma falha na leitura não é separável de uma falha na
      escrita. Três bytes lidos por três caminhos preservam essa separação.

Em suma: a condição é arquitectural e metodológica, não uma afirmação de que o
código não bastaria. A obrigação mantém-se na mesma.


4. MEDIÇÃO POSITIVA

Cobertura integral, sem amostragem:

  256 × 256 = 65536 entradas, uma por elemento de X₂.

Para cada (b₀, b₁):

  y = ι₂(b₀, b₁)
  π₀(y) =? b₀
  π₁(y) =? b₁
  π₂(y) =? 0

As três verificações são independentes e as três entram no veredicto. Um caso é
PASS apenas se as três o são. A convenção de índice que as rege é a declarada
em §3.3: π₀ = b₀, π₁ = b₁, π₂ = b₂.

A cobertura é integral por uma razão que é fácil de perder e que §9 lista de
novo: as mutações de canto — b₀ = 0, b₀ = 255, b₁ = 0, b₁ = 255 — só são
detectadas se esses quatro pontos estiverem no domínio coberto. Uma cobertura
amostrada que os deixasse de fora daria PASS a exactamente as mutações que o
contrato mais precisa de pegar. Logo: 65536/65536, e o número é uma
obrigação, não um resumo do que aconteceu.


5. INJECTIVIDADE

Propriedade matemática, da definição canónica:

  ι₂(x) = ι₂(y)  ⇒  x = y

A medição pode verificar, sobre as 65536 entradas, que os pares
(π₀(y), π₁(y)) saem todos distintos — o que é o mesmo que dizer que a
composição com C₂ é injectiva, pela bijeção C₂.

Limite explícito, e é um limite de honestidade e não de força: uma verificação
sobre 65536 pontos distintos estabelece injectividade SOBRE O DOMÍNIO TESTADO, e
nada mais. Não é uma prova de injectividade em geral, e o instrumento não
deverá reportar injectividade como resultado abstracto. O domínio de X₂ tem
exactamente 65536 elementos, e por isso é exaustivo e não amostral — mas a
frase a usar continua a ser "verificado no domínio", e não "provado".


6. ORÁCULO INDEPENDENTE

  O₂(b₀, b₁) = C₃(b₀, b₁, 0)

O instrumento futuro tem de implementar este cálculo de forma independente da
função sob teste. Proibições explícitas:

  • PROIBIDO chamar iota2(…) dentro do oráculo.
  • PROIBIDO incluir lib/iota2.h no código do oráculo.
  • O oráculo tem de permanecer correcto se iota2 for alterada. Esta é a
    condição que dá sentido às duas proibições: não se pede que o oráculo seja
    diferente do alvo, pede-se que ele sobreviva à mudança do alvo.

O oráculo é construído a partir de: o valor de b₀, o valor de b₁, o zero da
terceira posição, e a fórmula posicional de §3.1. Nenhum passo requer
conhecer ι₂ como função. Note-se que O₂(b₀,b₁) = b₀ + 256·b₁, que é
C₂. O oráculo deste salto coincide numericamente com o do salto anterior, e é
construído a partir da definição de X₃, não de implementação nenhuma de ι₂.


7. CONTROLOS NEGATIVOS

Os três controlos são construídos directamente, a partir de coordenadas
explícitas. Nenhum chama ι₂. Nenhum chama iota2. Todos são contados antes de
qualquer medição.

7.1 Controle A — terceiro componente

Construir (b₀, b₁, b₂) com

  b₂ = b₁        b₁ ∈ {1, …, 255}        b₀ ∈ {0, …, 255}

  Total: 256 × 255 = 65280

Esperado:  π₀ passa, π₁ passa, π₂ falha.

Exclusão de b₁ = 0: se b₁ = 0 então b₂ = 0, e a perturbação devolve
(b₀, 0, 0), que é a resposta correcta. Seria o oráculo, não uma perturbação.
Fora.

7.2 Controle B1 — alteração do primeiro componente

Construir (S₁(b₀), b₁, 0) com

  b₀ ∈ {0, …, 254}        b₁ ∈ {0, …, 255}

  Total: 255 × 256 = 65280

Esperado:  π₀ falha, π₁ passa, π₂ passa.

Exclusão de b₀ = 255: S₁(255) = 0 por overflow global (campos.tex:448), e
(S₁(255), b₁, 0) = (0, b₁, 0) coincide exactamente com a resposta correcta
para a entrada (0, b₁). Fora.

7.3 Controle B2 — alteração do segundo componente

Construir (b₀, S₁(b₁), 0) com

  b₀ ∈ {0, …, 255}        b₁ ∈ {0, …, 254}

  Total: 256 × 255 = 65280

Esperado:  π₀ passa, π₁ falha, π₂ passa.

Exclusão de b₁ = 255: mesma razão. S₁(255) = 0, e (b₀, 0, 0) coincide com a
resposta correcta para (b₀, 0). Fora.

7.4 Sobre a natureza das três exclusões

As três exclusões são o mesmo fenómeno, e é o mesmo que CONTRATO_X2.md §9
regista: coincidência entre CASOS DE ENTRADA, não ponto fixo da aplicação e
não falha do mapa. S₁(b) = b implicaria 1 ≡ 0 (mod 256), impossível — o
sucessor numérico não tem ponto fixo. O que tem é a perturbação: em 255, a
perturbação deixa de ser perturbação. É periodicidade do sucessor.

S₁ é o sucessor nomeado pelo paper para d = 1 (campos.tex:433, com
S_d(255,…,255) = (0,…,0) em campos.tex:448). Os controlos usam S₁ e não
"b + 1": "b+1" não é operação que o paper defina, S₁ é. Mesma disciplina de
CONTRATO_X2.md §9.

7.5 Total dos controlos

  65280
+ 65280
+ 65280
────────
 195840


8. COBERTURA NÃO É CONTROLES

  65536   é a COBERTURA POSITIVA do domínio de X₂. Uma vez, cada elemento.
  195840  são CASOS NEGATIVOS, três conjuntos separados, fora do domínio.

Não somar. 65536 + 195840 = 261376 não é uma cobertura, não é um total de
testes de ι₂, e não deve aparecer em nenhum relatório como tal. A soma
confundiria o que o instrumento tem de mostrar com aquilo que ele tem de
conseguir enganar.


9. MUTABILIDADE E AUDITORIA

O instrumento tem de ser auditável contra mutações da implementação. A
estrutura tem de ser capaz de detectar, no mínimo:

   1. terceiro componente incorreto
   2. primeiro componente alterado
   3. segundo componente alterado
   4. identidade em vez de inclusão
   5. deslocamento / off-by-one
   6. leitura ou projecção errada
   7. erro específico em b₀ = 0
   8. erro específico em b₀ = 255
   9. erro específico em b₁ = 0
  10. erro específico em b₁ = 255

As quatro últimas são mutações de CANTO, e são elas que obrigam a cobertura
integral de §4: um instrumento que amostrasse o domínio e falhasse em detectar
7 a 10 estaria a ser aprovado com um PASS que não significa nada. A ligação
entre a cobertura de §4 e esta lista não é decorativa.

NÃO SE AFIRMA NENHUMA TAXA DE DETECÇÃO. Nenhuma. Este contrato não diz que
estas dez são detectadas, nem em que proporção. Qualquer número de detecção
exige uma bateria efectivamente executada, com as mutações aplicadas uma a
uma e registadas uma a uma, e essa bateria ainda não existe. Declarar
"10/10" aqui seria inventar evidência.

A mesma disciplina que a frente de ι₁ encontrou na prática: a contagem
histórica "10/10 mutações" é evidência de sessão, não está versionada, e não
constitui precedente para este contrato.


10. RESÍDUO E VEREDITO

  RESÍDUO := o número de elementos de X₂ sem veredicto determinado.

Com a cobertura integral de §4, o resíduo tem de ser

  RESÍDUO = 0

isto é, cada um dos 65536 elementos foi efectivamente decidido como PASS ou
como falha atribuída a um canal nomeado. Resíduo diferente de zero é buraco
de cobertura, e é falha do instrumento, não do alvo.

Uma falha em qualquer canal obrigatório impede o fechamento. Não há
degradação, nem "PASS com ressalva", nem veredicto majoritário.

Critério de fecho — TODOS estes, e nenhum basta sozinho:

  IMPLEMENTADO:          SIM
  MEDIDO:                SIM
  ORÁCULO INDEPENDENTE:  SIM
  CONTROLES NEGATIVOS:   SIM
  COBERTURA:             65536/65536
  RESÍDUO:               0
  FALHAS:                0
  VEREDITO:              FECHADO

Este é o estado ESPERADO de sucesso. NÃO É UM RESULTADO OBTIDO. Nada disto
foi observado. ι₂ não está implementado, não foi medido, e nenhum oráculo
independente existe em código. A lista acima é o que teria de ser verdadeiro
para se poder escrever FECHADO ao lado de ι₁.


11. REFERÊNCIAS

Definição canónica da seta:
  redes/campos.tex, sec:byte-carry, def:iota-d — linhas 397–427

Espaço e codificação da família:
  redes/campos.tex, sec:byte-carry, def:Xd — linha 386
  redes/campos.tex — C_d, linha 392
  redes/campos.tex — S_d, linha 433
  redes/campos.tex — C_{d+1}(ι_d(x)) = C_d(x), linha 424
  redes/campos.tex — concatenação de blocos, linha 671

Declarações de âmbito que sustentam §0:
  redes/campos.tex:589  — X_d na secção genérica de base b
  redes/campos.tex:595  — X_d := {0,…,b−1}^d, família genérica
  a44b3c5e             — frase de âmbito e correcção da referência cruzada

Contraste registado, não é objecto deste contrato:
  campos.tex:1400 — extensão dimensional ≠ Transformada 𝒯
  campos.tex:1398 — 2⁸ = 256 é reformulação de cardinalidade, não teorema

Documento correlacionado, congelado e não alterado:
  CONTRATO_X2.md — X₂ e C₂; define o objecto de partida deste salto


12. DECISÕES ABERTAS

Nenhuma bloqueia a abertura. Todas são para a frente do medidor.

  1. Representação de memória de X₃ no instrumento: três bytes, ou código de
     24 bits. §3.1 mostra que a escolha não altera o poder de medida; altera o
     que o medidor toca com as mãos. E — importante — se escolher o código de
     24 bits, a leitura dos três canais de §3.3 tem de continuar a ser feita
     sobre bytes independentes, sob pena de a exigência de canal ser
     cumprida só no papel.

  2. Convenção de índice das projeções. RESOLVIDA, e não é decisão em aberto.
     A convenção local deste contrato está declarada em §3.3: indexação
     0-based, π₀ = b₀, π₁ = b₁, π₂ = b₂. A divergência com a notação
     de CONTRATO_X2.md §3 (π₁ = b₀) está declarada em §3.3 e não é
     harmonizada, porque CONTRATO_X2.md está congelado em 25a19dfa. Nada aqui
     reabre esse checkpoint, e nada aqui autoriza reinterpretar π₁ no
     contrato anterior. Não fica trabalho por fazer sobre este ponto; fica
     apenas o aviso, para quem ler os dois documentos em sequência.

  3. Ordem de leitura do oráculo. O oráculo O₂(b₀,b₁) = b₀ + 256·b₁ tem
     três formas possíveis no C: por bytes, por soma deslocada, ou por
     composição com C₂. A escolha é do medidor e tem de ser tomada antes de
     correr. Se a implementação auditada usar uma delas, o oráculo não deve
     usar a mesma, ou a independência da §6 é nominal.

  4. Bateria de mutações. As dez de §9 têm de ser aplicadas uma a uma e
     registadas uma a uma. Falta decidir onde vive essa bateria e se as
     mutações de canto (7 a 10) são mutações distinctas ou quatro instâncias
     de um mesmo template.

  5. Colisão nominal com a escada. Resolvida por âmbito, em §0, e não
     reabierta. Se algum dia a escada precisar de bytes, é a escada que se
     move, não esta frente.


13. ONDE CADA COISA FECHA

    o que é X₂ e X₃           §1, instâncias de X_d, campos.tex:386
    a seta sob teste           §2, instância d=2 de def:iota-d
    por que não é lei nova     §2, substituição de índice
    codificação e amplitude    §3.1, C₃, uint32_t suficiente
    a identidade de colapso    §3.2, C₃(ι₂(x)) = C₂(x)
    as três projeções         §3.3, π₀=b₀, π₁=b₁, π₂=b₂, 0-based
    obrigação de canal         §3.3, três canais, todos no veredicto
    a divergência com X2       §3.3, declarada, não harmonizada
    cobertura positiva         §4, 65536/65536, integral
    injectividade             §5, verificada no domínio testado
    o oráculo                  §6, O₂, sem iota2, sem lib/iota2.h
    controlo do 3º componente  §7.1, (b₀,b₁,b₁), b₁≠0
    controlo do 1º componente  §7.2, (S₁(b₀),b₁,0), b₀≠255
    controlo do 2º componente  §7.3, (b₀,S₁(b₁),0), b₁≠255
    total dos controlos        §7.5, 195840
    cobertura ≠ controlos      §8, e a soma é proibida
    mutabilidade               §9, dez alvos, nenhuma taxa afirmada
    resíduo e fecho            §10, RESÍDUO=0, e nada foi medido
    âmbito e desambiguação      §0, escopo documental, sem renomeação

    ι₂ está ABERTO. ι₁ está FECHADO em 25a19dfa. Este documento não fecha nada:
    dá a régua com que a frente seguinte se mede.

═══════════════════════════════════════════════════════════════════════════════
