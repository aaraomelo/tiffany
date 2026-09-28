CONTRATO X4 — SALTO DIMENSIONAL DE BYTES: X₃ → X₄
═══════════════════════════════════════════════════════════════════════════════

    Documento:  CONTRATO_X4.md
    Versão:    2026-09
    Âmbito:    definir a passagem de X₃ para X₄ na família de bytes, o oráculo
               que a avalia, os controlos negativos que a distinguem de
               substituições, os casos-limite, e o critério de fecho. NÃO define
               a aritmética de X₄, NÃO implementa, NÃO mede.
    Estado:    ABERTO. A seta está aberta por aqui. Nada foi implementado e nada
               foi medido. O critério de fecho está em §10 e ainda não foi
               cumprido por ninguém.


0. NOTAÇÃO E ÂMBITO — LER ANTES DE TUDO

Neste documento, X_d refere-se exclusivamente à família de espaços de bytes
definida em redes/campos.tex, secção sec:byte-carry. A notação X₃ e X₄ aqui não
se refere à cadeia X₁→X₂→X₃→X₄ da construção da escada existente em outros
documentos do projecto.

Três leituras de "X₁..X₄" coexistem neste repositório. A desambiguação é por
ESCOPO DOCUMENTAL, e não por renomeação.

  • A família de BYTES, que é a deste contrato. Em campos.tex, sec:byte-carry,
    linha 386, X_d := (Z/256Z)^d. X₄ aqui são 256⁴ = 2³² = 4294967296
    elementos, quatro coordenadas byte.

  • A família GENÉRICA de base b. Em campos.tex, sec:nivel-peso-escala, os
    mesmos símbolos designam X_d := {0,…,b−1}^d, da qual a anterior é o caso
    particular b=256. O commit a44b3c5e limitou-se a declarar essa ligação
    numa frase, sem introduzir símbolo novo. Renomear só uma das duas criaria
    uma incoerência onde antes havia uma sobrecarreg contextual legítima.

  • A ESCADA, onde X₄ não é um espaço de bytes. Em papers/aranha.tex:4461-4462
    e lib/escada.h:7-10 a cadeia é X₁ = ℤ, X₂ = X₁²/~, X₃ = (X₂×X₂*)/~ e
    X₄ = "os cortes de X₃". Um conjunto de cortes não tem quatro coordenadas
    byte nem 2³² elementos. A colisão é real e não é resolvida por renomeação.

Uma quarta colisão, puramente TEXTUAL e sem qualquer relação semântica com as
três anteriores: em cerca de 26 ficheiros de tests/ e banco/, "§X1" a "§X4" é
marcador de secção de um paper (por exemplo tests/cosmico.c:27-30,
banco/agentes.c:183). Uma busca por X1..X4 devolve esses falsos positivos. Fica
registado para que ninguém investigue a escada por essa via.

Símbolos proibidos neste documento, e por quê: X₄^bytes, BX₄, Y₄, X₄′. Nenhum é
necessário. Introduzi-los seria criar uma terceira notação para resolver uma
colisão que o âmbito já resolve.

A definição canónica da seta NÃO é copiada para aqui. É a de campos.tex,
sec:byte-carry, def:iota-d (linhas 397-427). Este documento aponta para lá e
mede-se contra lá. Duas fontes para a mesma definição seriam uma fonte de
divergência.

A instância d=2 da MESMA definição já foi fechada em 0e3df918: ι₂(b₀,b₁) =
(b₀,b₁,0). A ι₃ não é uma lei nova, não estende a ι₂, e não a copia. É a
substituição de d por 3 numa frase já publicada.


1. OS DOIS ESPAÇOS

  X₃ := (Z/256Z)³          |X₃| = 256³      = 16777216 = 2²⁴
  X₄ := (Z/256Z)⁴          |X₄| = 256⁴      = 4294967296 = 2³²

Ambos são instâncias de X_d := (Z/256Z)^d, campos.tex:386, para d = 3 e d = 4.
Um elemento de X₃ escreve-se (b₀, b₁, b₂) e um de X₄ escreve-se
(b₀, b₁, b₂, b₃), com b_k ∈ {0, …, 255}, as coordenadas independentes entre si.

A ordem é little-endian, a do paper: b₀ tem peso 256⁰ e é o menos significativo;
b₃ tem peso 256³ e é o mais significativo. A convenção não é uma escolha deste
documento — é a de campos.tex:392, onde C_d(b₀,…,b_{d−1}) = Σ b_k 256^k.


2. A SETA SOB TESTE

  ι₃ : X₃ ↪ X₄,   ι₃(b₀, b₁, b₂) = (b₀, b₁, b₂, 0)

b₀, b₁ e b₂ passam intactos. A quarta coordenada é o resíduo 0 na posição nova,
a de peso 256³, exactamente como manda a definição canónica para a posição de
peso 256^d. Determinístico, sem estado, sem alocação, sem dependência de nada
que não sejam os três argumentos.

Propriedades exigidas, todas consequência da definição e nenhuma delas uma
escolha:

  • INJECTIVA. A projecção para as primeiras três coordenadas compõe com ι₃ na
    identidade de X₃ (campos.tex:410-411).
  • IMAGEM. A imagem é exactamente {(b₀,b₁,b₂,0)}, de cardinalidade 2²⁴, e é o
    subconjunto de X₄ com a quarta coordenada nula.
  • IDENTITY UNDER C. C₄(ι₃(x)) = C₃(x), porque a coordenada acrescentada tem
    peso 256³ e vale 0 (campos.tex:424), por ser o caso r=1 da concatenação de
    blocos de campos.tex:671.

A ι₃ é a terceira seta da frente. ι₁ está FECHADO em 25a19dfa, ι₂ FECHADO em
0e3df918.


3. REPRESENTAÇÃO E A IDENTIDADE DE COLAPSO

3.1  Codificação e amplitude

  C₄(b₀,b₁,b₂,b₃) = b₀ + 256·b₁ + 65536·b₂ + 16777216·b₃
  0 ≤ C₄ ≤ 2³²−1 = 4294967295

  C₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂
  0 ≤ C₃ ≤ 2²⁴−1 = 16777215

O tipo de C₄ no instrumento é uint64_t, por uma razão que não é de
"segurança": o máximo de C₄ é 4294967295, que é o valor MÁXIMO de uint32_t e
NÃO cabe em int com sinal, cujo máximo é 2147483647. Em particular
16777216 · 255 = 4278190080 transborda int com sinal.

Aviso que este documento não pode dar e que fica para a auditoria de código:
uint32_t TAMBÉM caberia em C₄, exactamente, sem folga. A escolha de uint64_t é
margem e auditabilidade, não correcção forçada. Consequência: um veredicto
verde NÃO prova o tipo usado. O que prova é a ausência de promoção perigosa —
ver §6.

O instrumento nunca calcula desvios por subtracção. As mutações de §9 produzem
desvios negativos (b₀: 255→0 e b₁: 255→0), e subtrair em aritmética não-signed
transbordaria silenciosamente. Compara-se.

3.2  A identidade de colapso

Sob a codificação, ι₃ é a identidade:

  C₄(ι₃(x)) = C₄(b₀,b₁,b₂,0) = b₀ + 256·b₁ + 65536·b₂ = C₃(x)

É esta identidade que o oráculo de §6 mede, e é ela que distingue uma inclusão
de uma substituição. Um alvo que devolvesse (b₀,b₁,b₂,b₂) — o controlo N de §7.4
— satisfaz injectividade e falha aqui.

3.3  Projeções e obrigação de canal

  π₀(y) = b₀    π₁(y) = b₁    π₂(y) = b₂    π₃(y) = b₃

Indexação 0-based, como em CONTRATO_X2.md §3. A indexação 1-based que pode
surgir na notação do paper NÃO é adoptada aqui, e a divergência fica declarada e
não harmonizada.

Obrigação de canal: as quatro posições têm de ser legíveis SEPARADAMENTE e os
quatro canais entram todos no veredicto. Nenhuma falha pode ser atribuída a "o
alvo" sem nomear o canal. Contadores por canal são obrigatórios, e a
atribuição tem de ser real: um instrumento que contasse "pelo menos um canal
falhou" não distinguiria as categorias 5 e 6 de §9, que falham dois e quatro
canais respectivamente.

Representação de memória em X₄: quatro bytes separados num struct, e não um word
de 32 bits. Ver §12.1.


4. MEDIÇÃO POSITIVA

  COBERTURA := o número de elementos de X₃ submetidos ao alvo, cada um uma vez.

  |X₃| = 256³ = 16777216

A cobertura exigida é integral:

  COBERTURA = 16777216/16777216

Para cada (b₀,b₁,b₂) ∈ X₃, o instrumento:

  1. chama ι₃ uma vez;
  2. lê as QUATRO coordenadas em bytes separados, e把它们 copia para um tipo
     próprio do instrumento antes de qualquer verificação;
  3. compara π₀ com b₀, π₁ com b₁, π₂ com b₂, π₃ com 0, em quatro verificações
     independentes, com contadores separados;
  4. compara C₄ da saída observada com o oráculo O₃(b₀,b₁,b₂).

O valor esperado em todos os quatro casos vem dos argumentos de entrada. Nunca
vem de iota3.

AMOSTRAGEM É PROIBIDA. Não se mede uma amostra, não se mede um subconjunto, não
se reduz a cobertura por desempenho, e não se substitui o全域 por um conjunto de
casos representativos. O motivo não é estético: as categorias 7, 8 e 9 de §9
são detectadas em exactamente 65536 dos 16777216 elementos, ou seja, em 0,390625 %
do domínio. Qualquer esquema que não garanta visitar cada valor de cada
coordenada é cego a elas por construção. Ver §9.3.


5. INJECTIVIDADE

Verificada no domínio testado: se ι₃(x) = ι₃(y), então b₃ = 0 = b₃ e as três
primeiras coordenadas coincidem, logo x = y. A injectividade é uma consequência
de C₄ ser bijecção (campos.tex:394) e de C₄(ι₃(x)) = C₃(x): injectividade de
C₄ seguida da de C₃ em X₃.

Isto é consequência de §3.2, não uma verificação independente. Registado como
tal, para que não seja contado como um teste adicional.


6. ORÁCULO INDEPENDENTE

  O₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂

O₃ é calculado APENAS a partir dos argumentos de entrada. Não chama iota3, não
lê a saída de iota3, não inclui lógica do alvo, e sobrevive a qualquer alteração
do alvo. É a identidade de §3.2 escrita do lado do instrumento.

O par C₄ / O₃ é o par que tem de ser independente: C₄ lê a saída OBSERVADA, O₃ lê
os ARGUMENTOS. Um é a medida, o outro é a régua. Um oráculo que lesse a saída
para derivar o esperado não mediria nada.

A forma de O₃ é deliberadamente diferente de C₄: três termos contra quatro, e o
peso 16777216 não aparece em O₃ pelo próprio facto de a coordenada nova valer
zero. Não há forma partilhada a duplicar.

Requisito de implementação, e é auditoria de código e não medição: cada produto
de C₄ e de O₃ leva o cast para uint64_t no PRIMEIRO operando, e os
multiplicadores levam UINT64_C. A razão está em §3.1 e é a promoção inteira:
com q3 um uint8_t, `q3 * 16777216` é int × int e transborda com sinal, sem
qualquer aviso, mesmo com tipo de retorno uint64_t.


7. CONTROLOS NEGATIVOS

Os controlos são CASOS NEGATIVOS: saídas ERRADAS que o instrumento tem de
REJEITAR. Não são elementos de X₃ submetidos ao alvo. Nenhum é construído
chamando iota3, e nenhum parte de uma saída de iota3 corrompida. Cada um é
montado com coordenadas explícitas e depois entregue às mesmas quatro projeções
e ao mesmo oráculo.

Cada família viola EXACTAMENTE UM canal. Os outros três ficam correctos, para
que a falha seja atribuível a um canal nomeado.

7.1  C₀ — primeira coordenada errada
     (S₁(b₀), b₁, b₂, 0), com b₀ ∈ {0,…,254}
     domínio: 255 · 256 · 256 = 16711680
     esperado: π₀ FALHA, π₁ π₂ π₃ PASSAM

7.2  C₁ — segunda coordenada errada
     (b₀, S₁(b₁), b₂, 0), com b₁ ∈ {0,…,254}
     domínio: 256 · 255 · 256 = 16711680
     esperado: π₁ FALHA, π₀ π₂ π₃ PASSAM

7.3  C₂ — terceira coordenada errada
     (b₀, b₁, S₁(b₂), 0), com b₂ ∈ {0,…,254}
     domínio: 256 · 256 · 255 = 16711680
     esperado: π₂ FALHA, π₀ π₁ π₃ PASSAM

7.4  N — quarta coordenada errada, a coordenada NOVA
     (b₀, b₁, b₂, b₂), com b₂ ∈ {1,…,255}
     domínio: 256 · 256 · 255 = 16711680
     esperado: π₃ FALHA, π₀ π₁ π₂ PASSAM

7.5  As exclusões, e porque não são falha de cobertura

As três famílias C excluem o valor 255 da coordenada corrompida, para que a
corrupção seja um incremento limpo S₁(b) = b+1, sem wrap modular. A família N
exclui b₂ = 0, para que a coordenada nova errada seja de facto não nula.

Nenhum dos valores excluídos fica por medir. b_k = 255 e b₂ = 0 estão ambos no
domínio de §4, que é integral, e são verificados ali. As exclusões são uma
escolha de desenho de cada família, não um buraco. 16711680 ≠ 16777216 neste
documento é esperado e não deve ser lido como cobertura incompleta.

7.6  Por que nenhuma família depende de iota3

Porque cada uma é uma propriedade do INSTRUMENTO. As quatro são montadas a
partir de literais no medidor e julgadas pelas mesmas projeções que julgam o
alvo. Se o instrumento aceitasse uma destas saídas erradas, isso seria uma
falha do instrumento e não do alvo, e é exactamente por isso que elas são
medidas ANTES do alvo correr.

7.7  Total dos controlos

  4 · 16711680 = 66846720


8. COBERTURA NÃO É CONTROLOS

  16777216  é a COBERTURA POSITIVA do domínio de X₃. Uma vez, cada elemento.
  66846720  são CASOS NEGATIVOS, quatro conjuntos separados, fora do domínio.

Não somar. 16777216 + 66846720 = 83623936 não é uma cobertura, não é um total
de testes de ι₃, e não deve aparecer em nenhum relatório como tal. Os controlos
são 3,984375 vezes a cobertura, e esse factor é um artefacto de desenho — quatro
famílias de 255·256² —, não uma cardinalidade. A soma confundiria o que o
instrumento tem de mostrar com aquilo que ele tem de conseguir enganar.


9. MUTABILIDADE E AUDITORIA

O instrumento tem de ser auditável contra mutações da implementação. A estrutura
tem de ser capaz de detectar, no mínimo, estas dez categorias, CUJO ALVO CONCRETO
é fixado por esta secção:

   1. primeira coordenada preservada corrompida
   2. segunda coordenada preservada corrompida
   3. terceira coordenada preservada corrompida
   4. quarta coordenada, a nova, corrompida
   5. troca de posições entre coordenadas
   6. deslocamento / off-by-one de posição
   7. erro de canto 255 na primeira coordenada
   8. erro de canto 255 na segunda coordenada
   9. erro de canto 0 na terceira coordenada
  10. erro específico na coordenada nova, com valor constante

Estas categorias NÃO são uma cópia das de CONTRATO_X3.md §9. A lista da ι₂ tem
três coordenadas e dez alvos para as cobrir; a ι₃ tem quatro e as proporções
mudam. Copiar mecanicamente daria uma lista que não cobre a coordenada nova e
que gasta dois alvos onde a ι₃ gasta três. Desenhadas à parte, e escritas aqui
antes de qualquer execução.

Para cada mutação, o que tem de estar escrito antes de a correr: a alteração
concreta no alvo, que observável deve falhar, por que a cobertura de §4 a
alcança, e se existe ponto degenerado que permanece coincidente. É esse
conjunto de quatro dados que §9.1 a §9.10 fixam.

9.1  primeira preservada: y.b0 = (Word8)(b0 + 1u)
     observável: π₀, e o oráculo
     alcançada por: o conjunto de detecção é X₃ inteiro
     degenerado: NENHUM. b₀+1 ≠ b₀ para todo b₀, com ou sem wrap

9.2  segunda preservada: y.b1 = (Word8)(b1 + 1u)
     observável: π₁, e o oráculo
     alcançada por: o conjunto de detecção é X₃ inteiro
     degenerado: NENHUM

9.3  terceira preservada: y.b2 = (Word8)(b2 + 1u)
     observável: π₂, e o oráculo
     alcançada por: o conjunto de detecção é X₃ inteiro
     degenerado: NENHUM

9.4  coordenada nova por identidade: y.b3 = b2
     observável: π₃, e o oráculo
     alcançada por: b₂ ≠ 0, ou seja 16711680 elementos
     degenerado: b₂ = 0, ou seja 65536 elementos, onde a saída coincide com a
     correcta. Taxa de detecção 99,609375 %

9.5  troca de posições: y.b0 = b1; y.b1 = b0
     observável: π₀ E π₁ em conjunto, e o oráculo
     alcançada por: b₀ ≠ b₁, ou seja 16711680 elementos
     degenerado: b₀ = b₁, ou seja 65536 elementos. Esta mutação falha dois
     canais em simultâneo, e é a prova de que a atribuição por canal de §3.3 é
     real

9.6  deslocamento: y.b0 = 0; y.b1 = b0; y.b2 = b1; y.b3 = b2
     observável: as quatro projeções, e o oráculo
     alcançada por: todos os elementos excepto a origem
     degenerado: a origem (0,0,0), UM elemento, onde a saída coincide com a
     correcta. Taxa de detecção 99,999994 %. Esta é a única mutação cuja
    degenerescência é um ponto, e é também um dos casos-limite de §10

9.7  canto 255 em b₀: if (b0 == 255) y.b0 = 0
     observável: π₀, e o oráculo
     alcançada por: b₀ = 255, ou seja 65536 elementos
     degenerado: fora desse plano a saída é a correcta, por desenho. Conjunto de
     detecção 0,390625 % do domínio

9.8  canto 255 em b₁: if (b1 == 255) y.b1 = 0
     observável: π₁, e o oráculo
     alcançada por: b₁ = 255, ou seja 65536 elementos
     degenerado: fora desse plano a saída é a correcta, por desenho. Conjunto de
     detecção 0,390625 % do domínio

9.9  canto 0 em b₂: if (b2 == 0) y.b2 = 255
     observável: π₂, e o oráculo
     alcançada por: b₂ = 0, ou seja 65536 elementos
     degenerado: fora desse plano a saída é a correcta, por desenho. Conjunto de
     detecção 0,390625 % do domínio

9.10 erro específico na coordenada nova: y.b3 = 1
     observável: π₃, e o oráculo
     alcançada por: o conjunto de detecção é X₃ inteiro, porque 1 ≠ 0 em todas
     as entradas
     degenerado: NENHUM

9.11 As três de canto obrigam a cobertura integral de §4

As categorias 7, 8 e 9 são detectadas em 65536 dos 16777216 elementos. Um
instrumento que amostrasse o domínio e falhasse em detectá-las estaria a ser
aprovado com um PASS que não significa nada. A ligação entre §4 e esta lista não
é decorativa: é a razão de a amostragem ser proibida.

9.12 As degenerescências não são defeito do instrumento

Uma degenerescência é um ponto onde a saída ERRADA coincide com a correcta. Nela,
a detecção é impossível por construção, e não por fraqueza: a mutação 9.4 em
b₂ = 0 produz (b₀,b₁,0,0), que é a saída especificada. Um instrumento que
"detectasse" ali estaria errado. As percentagens de detecção de §9.4, §9.5 e
§9.6 são consequência da definição, e registá-las é honestidade, não defeito.

9.13 NÃO SE AFIRMA NENHUMA TAXA DE DETECÇÃO

Nenhuma. Este contrato não diz que estas dez são detectadas, nem em que
proporção. Qualquer número exige uma bateria efectivamente executada, com as
mutações aplicadas uma a uma e registadas uma a uma, e essa bateria ainda não
existe. Declarar "10/10" aqui seria inventar evidência.

O resultado que a bateria poderá declarar, e só ele, é este: "10/10 das mutações
especificadas em §9.1 a §9.10 foram detectadas". Não "10/10 da classe de erros",
nem qualquer percentagem de cobertura de falhas.


10. CASOS-LIMITE, RESÍDUO E VEREDITO

10.1  Os dois casos-limite

  (0,0,0)       →  (0,0,0,0)          C₄ = 0
  (255,255,255) →  (255,255,255,0)    C₄ = 16777215

Ambos já estão no domínio de §4, que é integral. São exigidos ADEMAS como
verificações explícitas, porque têm propriedades que o resto do ciclo não
exercita de forma igualmente explícita:

  (0,0,0) é o mínimo global, e é o ÚNICO ponto onde as mutações 9.4, 9.5 e 9.6
  produzem a saída correcta. É onde o factor nulo de X₄ se manifesta.

  (255,255,255) é o máximo de X₃, e portanto o maior valor que C₃ pode assumir,
  16777215. Verificá-lo é verificar o limite superior do oráculo.

As duas verificações comparam a saída do alvo contra LITERAIS escritos no
instrumento, e não contra O₃. A razão: se o esperado viesse de O₃, e se o
oráculo estivesse errado, o erro anularia-se a si próprio.

10.2  RESÍDUO

  RESÍDUO := o número de elementos de X₃ sem veredicto determinado.

Com a cobertura integral de §4, o resíduo tem de ser

  RESÍDUO = 0

isto é, cada um dos 16777216 elementos foi efectivamente decidido como PASS ou
como falha atribuída a um canal nomeado. Resíduo diferente de zero é buraco de
cobertura, e é falha do instrumento, não do alvo.

10.3  Critério de fecho — TODOS estes, e nenhum basta sozinho

  IMPLEMENTADO:            SIM
  MEDIDO:                  SIM
  ORÁCULO INDEPENDENTE:    SIM
  CONTROLES NEGATIVOS:     SIM
  CASOS-LIMITE:            SIM
  COBERTURA:               16777216/16777216
  C₀, C₁, C₂, N:           16711680/16711680 cada
  RESÍDUO:                 0
  FALHAS:                  0
  VEREDITO:                FECHADO

Uma falha em qualquer canal obrigatório impede o fechamento. Não há degradação,
nem "PASS com ressalva", nem veredicto majoritário.

Este é o estado ESPERADO de sucesso. NÃO É UM RESULTADO OBTIDO. Nada disto foi
observado. ι₃ não está implementado, não foi medido, e nenhum oráculo
independente existe em código. A lista acima é o que teria de ser verdadeiro
para se poder escrever FECHADO ao lado de ι₂.

Uma nota sobre o critério e a auditoria de tipo. A garantia de que C₄ é
acumulado sem overflow de sinal com sinal NÃO é uma condição deste critério, e
não pode ser: uint32_t também caberia em C₄, de modo que o veredicto não a
poderia confirmar nem infirmar. A garantia é por leitura do código, e é a
auditoria do header e do medidor que a fornece.


11. REFERÊNCIAS

Definição canónica da seta:
  redes/campos.tex, sec:byte-carry, def:iota-d — linhas 397-427

Espaço, codificação e bijecção:
  redes/campos.tex, sec:byte-carry, def:Xd — linhas 382-395

A identidade sob a codificação:
  redes/campos.tex, sec:byte-carry — linha 424, e o caso r=1 da concatenação
  de blocos, sec:nivel-peso-escala, linha 671

Sucessor com carry, de onde vem S₁:
  redes/campos.tex, sec:byte-carry, def:Sd — linhas 431-449

Precedente imediato, ι₂:
  CONTRATO_X3.md, lib/iota2.h, tests/medidor_iota2.c — commit 0e3df918
  Em particular §3.1 (amplitude e tipo), §3.3 (obrigação de canal), §7
  (controlos), §8 (cobertura ≠ controlos), §9 (mutabilidade), §10 (fecho)

Precedente distante, ι₁:
  CONTRATO_X2.md, lib/iota1.h, tests/medidor_iota1.c — commit 25a19dfa

Âmbito da família genérica de base b, e a ligação com b = 256:
  redes/campos.tex, sec:nivel-peso-escala — commit a44b3c5e

A escada, para contraste de âmbito:
  lib/escada.h, linhas 7-10; papers/aranha.tex, linhas 4461-4470


12. DECISÕES ABERTAS

  1. Representação de memória de X₄ no instrumento: quatro bytes separados, ou
     um word de 32 bits. A segunda opção tem o mesmo poder de medida e obrigaria
     a extrair as posições à mão, e então a separação dos canais de §3.3 passaria
     a depender de quem lê, não da forma do dado. É a mesma decisão que
     CONTRATO_X3.md §12.1 deixou em aberto, e a prática de ι₂ resolveu por
     bytes separados. O contrato não é alterado por isso.

  2. Ordem de leitura do oráculo. O₃(b₀,b₁,b₂) = b₀ + 256·b₁ + 65536·b₂ está
     escrita na forma posicional directa, e a acumulado por Horner big-endian é
     equivalente e igualmente independente. Nenhuma das duas é partilhada com o
     alvo, que não calcula código nenhum. Decisão de implementação, e é o
     medidor que a fecha. O contrato não é alterado por isso.

  3. Onde vive a bateria de mutações. As dez de §9 têm de ser aplicadas uma a
     uma, sobre cópias temporárias do alvo, com o instrumento inalterado e o
     alvo restaurado byte a byte depois de cada uma. Isso é trabalho de sessão,
     não de produto, e não cabe neste ficheiro nem no medidor. Decisão de
     implementação.

  4. Colisão nominal com a escada. Resolvida por âmbito, em §0, e não reabierta.
     Se algum dia a escada precisar de bytes, é a escada que se move, não esta
     frente.

  5. Drift de medidor/ESPECIFICACAO.md. Esse ficheiro menciona ι₁ e não menciona
     nem ι₂ nem ι₃. É drift de outra frente, e por decisão explícita não é
     tocado no checkpoint da ι₃, para não misturar dois assuntos num só
     fecho.


13. ONDE CADA COISA FECHA

    o que é X₃ e X₄           §1, instâncias de X_d, campos.tex:386
    a seta sob teste           §2, instância d=3 de def:iota-d
    por que não é lei nova     §2, substituição de índice
    codificação e amplitude    §3.1, C₄, uint64_t, e por que uint32_t serviria
    a identidade de colapso    §3.2, C₄(ι₃(x)) = C₃(x)
    as quatro projeções       §3.3, π₀..π₃, 0-based
    obrigação de canal         §3.3, quatro canais, todos no veredicto
    a divergência de índice    §3.3, declarada, não harmonizada
    cobertura positiva         §4, 16777216/16777216, integral
    proibição de amostragem    §4, e as três de canto de §9 obrigam
    injectividade             §5, consequência de §3.2, não teste à parte
    o oráculo                  §6, O₃, sem iota3, sem lib/iota3.h
    a auditoria de tipo        §6, casts no primeiro operando, por leitura
    controlo da 1ª coordenada  §7.1, (S₁(b₀),b₁,b₂,0), b₀≠255
    controlo da 2ª coordenada  §7.2, (b₀,S₁(b₁),b₂,0), b₁≠255
    controlo da 3ª coordenada  §7.3, (b₀,b₁,S₁(b₂),0), b₂≠255
    controlo da coord. nova    §7.4, (b₀,b₁,b₂,b₂), b₂≠0
    as exclusões               §7.5, e porque não são buraco
    total dos controlos        §7.7, 66846720
    cobertura ≠ controlos      §8, e a soma é proibida
    mutabilidade               §9, dez alvos concretos, nenhuma taxa afirmada
    as três de canto           §9.11, 0,390625 % cada, e a razão da exaustividade
    as degenerescências        §9.12, impossíveis por construção, não fraqueza
    casos-limite               §10.1, (0,0,0) e (255,255,255), contra literais
    resíduo e fecho            §10.2, RESÍDUO=0, e nada foi medido
    âmbito e desambiguação      §0, três leituras de X₁..X₄, sem renomeação

    ι₃ está ABERTO. ι₂ está FECHADO em 0e3df918, ι₁ em 25a19dfa. Este documento
    não fecha nada: dá a régua com que a frente seguinte se mede.

═══════════════════════════════════════════════════════════════════════════════
