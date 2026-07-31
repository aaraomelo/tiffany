#!/bin/sh
# ciencia.sh — o corpus científico. Só o que se sustenta, e nada de "mais ou menos".
#
# A regra que o Aarão deu, e é o critério: não se julga a roupa. Verifica-se se está COMPLETO
# (tem dual), se é REVERSÍVEL, e se não VAZA. O que entra aqui obedece a isso — e onde uma coisa
# só vale sob uma régua, a régua vai dita, senão a afirmação fica pela metade.
#
#   ./ciencia.sh <base>
B="${1:-/tmp/barr/ciencia}"
A="./conversa"
p(){ "$A" "$B" aprende "$1" "$2" >/dev/null; }

# --- o que depende do corpo, e por isso vai com a régua dita ---
p "3 vezes 3 é igual a 3 mais 3" "em Z/3, sim: 3≡0, logo 9≡6≡0. Em Z, não: 9≠6. A afirmação sem o corpo declarado está incompleta."
p "quanto é 1 mais 1" "em Z, 2. Em Z/2, 0. A pergunta não fecha sem dizer o corpo."
p "a raiz de 2 é racional" "depende do corpo. Em Q não: p² = 2q² faria p e q ambos pares. Mas em Z/7 sim — 3² = 9 ≡ 2, e o 3 está lá. Também em Z/17 e Z/23."
p "onde a raiz de 2 existe" "nos corpos onde 2 é resíduo quadrático: Z/7 (x=3), Z/17 (x=6), Z/23 (x=5). Em Q não existe, e é isso que a torna irracional — irracional é RELATIVO a Q."
p "existe o maior número primo" "em N não: o produto de um conjunto finito de primos mais 1 tem um fator primo que não está lá. Em Z/p o corpo é finito e a pergunta muda de sentido — primo é do anel, não do universo."

# --- reversibilidade: o critério do sistema ---
p "o que torna uma operação segura" "ter dual: poder desfazer-se. O que não reverte perde informação, e o que perde informação não se audita."
p "a soma é reversível" "num GRUPO sim, pela subtração. Em N não: 3 menos 5 não está lá. N é monoide, e a diferença entre monoide e grupo É exatamente ter dual."
p "o hash é reversível" "não, e é de propósito: a irreversibilidade está num passo só, a soma final de Davies-Meyer. As 64 rondas revertem."
p "por que a prova de trabalho funciona" "porque a contração não tem dual. Se tivesse, invertia-se em vez de se procurar — a ausência do dual É o trabalho."

# --- as constantes e o que elas são ---
p "o que é o número de ouro" "uma das DUAS raízes de σ = 1 + 1/σ — a outra é menos 1 sobre sigma, e o par é o chicote: uma estica, a outra contrai. Em R a cifra é [1;1,1,...] e é o mais mal aproximado por racionais."
p "por que o ouro é o mais irracional" "porque a cifra não tem termo grande. Um termo grande dá um convergente que quase acerta; só uns não dão nenhum."
p "o que é uma fração contínua" "escrever um número como a₀ + 1/(a₁ + 1/(a₂ + ...)). Finita se o número é racional; periódica se é quadrático."
p "o que Lagrange provou sobre frações contínuas" "que a cifra é periódica se e só se o número é irracional quadrático — e o período é invariante completo."

# --- medida e infinito ---
p "o conjunto de Cantor tem medida" "de LEBESGUE, zero — encolhe por 2/3 a cada nível. Mas de Hausdorff tem dimensão log2/log3 e medida positiva nessa: a medida é escolhida, e mudá-la muda a resposta."
p "um conjunto de medida zero pode encher" "a soma dele consigo pode: C + C = [0,2]. O conjunto é pó; a operação enche."
p "quantos racionais há" "tantos quantos inteiros — são contáveis. Os reais não são, e a diagonal de Cantor mostra-o."

# --- física, e o que nela é invariante ---
p "o que se conserva num choque" "num sistema FECHADO, o momento; a energia cinética só se for elástico. Com força externa nem o momento se conserva — o que protege é a simetria, e força externa quebra-a."
p "o que o teorema de Noether diz" "a cada simetria contínua corresponde uma quantidade conservada: tempo dá energia, espaço dá momento, rotação dá momento angular."
p "por que a entropia cresce" "porque os estados prováveis são muitos mais que os improváveis — é CONTAGEM, não lei de força. E por isso pode decrescer: em sistemas pequenos decresce, e o teorema de flutuação diz com que probabilidade."
p "o que é o cone de luz" "onde a distância própria é nula. Separa o que pode influenciar do que não pode, e é fronteira, não lugar."

# ═══ ÁLGEBRA: o que é estrutura e o que é roupa ═══
p "o que é um grupo" "conjunto com uma operação associativa, um neutro, e CADA elemento com inverso. O inverso é o dual — sem ele é monoide."
p "qual a diferença entre monoide e grupo" "o dual. O monoide tem a operação e o neutro; o grupo exige que tudo reverta. N com a soma é monoide; Z é grupo."
p "o que é um corpo" "um anel COMUTATIVO onde todo elemento não nulo tem inverso. Sem a comutatividade é anel de divisão — os quaterniões são isso e não são corpo. Q, R, C e Z/p com p primo são corpos; Z não é, porque 2 não tem inverso lá."
p "os quaterniões são um corpo" "não: tudo tem inverso, mas o produto não comuta — ij = k e ji = menos k. É anel de divisão, e a diferença é uma palavra na definição."
p "Z/4 é corpo" "não: 2 vezes 2 é 0, logo há divisor de zero e o 2 não tem inverso. Z/p só é corpo quando p é primo."
p "o que é característica de um corpo" "quantas vezes o 1 somado consigo dá 0. Em Q é 0 — nunca dá; em Z/3 é 3, e é por isso que lá 3 vezes 3 é 3 mais 3."
p "todo corpo finito tem quantos elementos" "uma potência de primo: p^n, e para cada p^n há um só a menos de isomorfismo. Não existe corpo com 6 elementos."

# ═══ GEOMETRIA E CÔNICAS: a mesma família, aberturas diferentes ═══
p "o que distingue elipse parábola e hipérbole" "o discriminante da forma: negativo fecha em elipse, zero é a parábola, positivo abre em hipérbole. É o mesmo mecanismo com abertura diferente."
p "o que é o discriminante" "B² menos 4C da forma a² + Bab + Cb², e é traço² menos 4·det do operador. Ele diz a classe e não muda por mudança de coordenadas."
p "por que a parábola é o caso limite" "porque o discriminante é zero: o absorvente. É o único sem dual, e é por isso que é fronteira e não região."
p "o que é uma cónica degenerada" "quando a forma fatoriza: duas retas, uma reta dupla, ou um ponto. O discriminante da matriz completa anula-se — não é falha, é o caso de fronteira."

# ═══ NÚMEROS: o que é exato e o que é aproximado ═══
p "quanto é um terço em decimal" "0,333... e não fecha em base 10. Em base 3 é 0,1 exato — a dízima infinita é da BASE, não do número."
p "o que é um número transcendente" "o que não é raiz de polinómio de coeficientes racionais. Pi e e são; a raiz de 2 não é — ela é irracional mas algébrica."
p "0,999... é igual a 1" "em R, sim: a diferença seria um real positivo menor que todo real positivo, e não há. Nos hiper-reais há infinitesimais e a resposta muda."
p "quanto é 0 elevado a 0" "1 na combinatória e nas séries de potências, porque conta a função vazia. Indefinido na análise, porque o limite depende do caminho. Depende do que se está a fazer."
p "dividir por zero" "não está definido em corpo nenhum: se 0·x = 1, então 0 = 0·1 = 0·(0·x) = 0·x·0 = 0, e o corpo colapsa. Não é proibição — é o que a estrutura permite."

# ═══ COMPUTAÇÃO: o que se pode e o que não se pode ═══
p "o que Turing provou sobre a paragem" "que não há programa que decida, para todo par programa-entrada, se ele para. A prova é a diagonal: constrói-se um que faz o contrário do que o decisor diz."
p "o que é P versus NP" "se tudo o que se VERIFICA depressa se RESOLVE depressa. Está em aberto — e note-se que verificar e resolver são o par de sempre: um contrai, o outro estica."
p "o que é a complexidade de Kolmogorov" "o tamanho do menor programa que produz um objeto. É incalculável, e é a razão: se fosse calculável, produzia-se o menor objeto incompressível — contradição."
p "por que o hash tem colisões" "porque leva um conjunto infinito num finito. Não é defeito da função: é o princípio dos pombos, e vale para toda função desse feitio."

# ═══ TOPOLOGIA E GEOMETRIA: o postulado é escolha, não teorema ═══
p "as paralelas nunca se encontram" "no plano euclidiano, por POSTULADO. Na esférica não há paralelas — todas as geodésicas se cruzam; na hiperbólica há infinitas. O postulado é uma escolha, e é ela que nomeia a geometria."
p "quanto somam os ângulos de um triângulo" "180 graus no plano. Na esfera é mais, na hiperbólica é menos, e o excesso é a ÁREA vezes a curvatura. Não é um número — é uma medida do corpo onde se desenha."
p "quanto é vértices menos arestas mais faces" "2 na esfera, 0 no toro. É a característica de Euler e conta os buracos: é invariante topológico, não conta de forma."
p "a garrafa de Klein cabe em três dimensões" "não sem se atravessar; em R4 mergulha sem cruzar. A auto-interseção é do espaço ambiente e não do objeto."
p "o que é um nó" "um mergulho do círculo em R3. Em R4 todo nó se desata — a nodosidade é da dimensão três, e só dela."

# ═══ ANÁLISE: o infinito não herda o que é do finito ═══
p "quanto é 1 mais 2 mais 3 até o infinito" "como série, diverge. O menos um doze avos é o valor da continuação analítica de zeta em menos 1, que é outro objeto. Duas perguntas, duas respostas — trocá-las é que é o erro."
p "pode-se reordenar uma série" "se ela converge ABSOLUTAMENTE, sim, e a soma não muda. Se converge só condicionalmente, Riemann mostrou que se reordena para dar qualquer valor. A comutatividade é do finito e não sobrevive sozinha ao infinito."
p "toda função contínua é derivável" "em R não: Weierstrass construiu uma contínua em toda a parte e derivável em nenhuma. E na medida de Wiener quase TODA é assim — a derivável é que é a exceção."
p "o que é um limite" "o valor de que a sucessão se aproxima ao ponto de qualquer vizinhança conter a cauda. Depende da TOPOLOGIA: mudar a métrica muda o limite. Nos p-ádicos, p elevado a n tende a zero."

# ═══ PROBABILIDADE: não há força que compense ═══
p "saiu cara dez vezes agora sai coroa" "se as jogadas são independentes, não: continua metade. O que dez caras atualizam é a suspeita sobre a MOEDA, não o próximo lançamento."
p "o que garante a lei dos grandes números" "que a média converge, quase certamente, SE a esperança for finita. Sem essa hipótese falha: na distribuição de Cauchy a média de n amostras tem a mesma lei de uma só e nunca assenta. E mesmo onde vale, não garante o próximo nem puxa de volta — é convergência de médias, não compensação."
p "eventos independentes e exclusivos são o mesmo" "não, são quase opostos: independentes é P(A e B) = P(A)P(B); exclusivos com probabilidade positiva são DEPENDENTES, porque um exclui o outro."

# ═══ LÓGICA: onde a pergunta é indecidível e não em aberto ═══
p "o que Gödel provou" "que todo sistema consistente, recursivamente axiomatizável e forte para a aritmética tem sentença verdadeira que ele não prova. As três hipóteses são precisas — sem qualquer delas não vale."
p "a aritmética é incompleta" "a de Peano sim. A aritmética só com a soma — Presburger — é COMPLETA e decidível. Quem quebra é a multiplicação junto com a adição."
p "o axioma da escolha é verdadeiro" "é independente de ZF, SE ZF for consistente: Gödel mostrou que não se refuta, Cohen que não se prova. Sem essa hipótese não há independência nenhuma — de um sistema inconsistente prova-se tudo. Aceitá-lo é escolher um sistema, e há matemática dos dois lados."
p "existe conjunto entre os naturais e os reais" "é a hipótese do contínuo, independente de ZFC pelas mesmas duas metades e sob a mesma condição: ZFC consistente. Não está em aberto por falta de esforço — é indecidível ali."

# ═══ FÍSICA: o referencial vai dito ═══
p "dois eventos podem ser simultâneos" "depende do referencial: se estão separados por intervalo tipo-espaço, há referencial onde um precede o outro e outro onde a ordem inverte. A ordem só é absoluta dentro do cone de luz."
p "nada anda mais rápido que a luz" "nenhuma informação nem massa, no vácuo. A velocidade de fase passa disso e a expansão afasta galáxias mais depressa — e nenhum dos dois transporta sinal."
p "o princípio da incerteza é limitação do aparelho" "não: é da estrutura. Posição e momento são par de Fourier, e o produto das larguras tem piso para QUALQUER par assim — vale para som e para sinal, sem quântica nenhuma."
p "a massa conserva-se" "na química clássica sim, com boa aproximação. Em geral não: o que se conserva é o quadrivetor energia-momento, e a massa converte-se em energia."

# ═══ TEORIA DE NÚMEROS: a unicidade é do anel ═══
p "a fatoração em primos é única" "em Z sim, é o teorema fundamental. Em Z[raiz de menos 5] não: 6 = 2 vezes 3 e também (1+r)(1-r) com r a raiz de menos 5. Foi para recuperar a unicidade que se inventaram os ideais — ela volta lá, mas noutro objeto."
p "o que é um número primo" "o que, NÃO SENDO nulo nem unidade, só divide um produto se dividir um dos fatores. As duas exclusões fazem falta: o 1 divide tudo e o 0 gera ideal primo, e nenhum dos dois é o que se quer. Em Z isso coincide com irredutível; em anéis gerais separam-se, e o 2 em Z[raiz de menos 5] é irredutível e NÃO é primo."
p "por que 1 não é primo" "porque é unidade — tem inverso. Se fosse primo a fatoração deixava de ser única: 6 = 2·3 = 1·2·3 = 1·1·2·3. Não é convenção arbitrária, é o que preserva o teorema."
p "o pequeno teorema de Fermat serve para testar primos" "a ida vale: se p é primo, a elevado a p menos a é divisível por p. A VOLTA falha — os números de Carmichael passam sem serem primos, e o primeiro é 561, que é 3 vezes 11 vezes 17."
p "o RSA é seguro" "contra a fatoração clássica conhecida, com chave grande e implementação sem canal lateral. Contra o algoritmo de Shor num computador quântico com qubits bastantes, não — o algoritmo já existe, falta a máquina."

# ═══ ÁLGEBRA LINEAR: o critério é do anel, não da matriz ═══
p "quando uma matriz é invertível" "sobre um CORPO, quando o determinante não é zero. Sobre Z, só quando ele é mais ou menos 1 — porque só as unidades do anel invertem. O critério é do anel onde vivem as entradas."
p "o que diz o determinante" "como o volume escala e se a orientação vira. Zero quer dizer que o espaço colapsou de dimensão, e é por isso que não há volta: perdeu-se informação."
p "toda matriz tem autovalores" "sobre C sim, pelo teorema fundamental da álgebra. Sobre R não: a rotação de 90 graus não tem nenhum, e é exatamente por isso que ela é elíptica. Quem decide é o corpo."
p "o que é o traço" "a soma da diagonal. É a soma dos autovalores NO FECHO ALGÉBRICO, contados com multiplicidade — sobre R a rotação de 90 graus tem traço 0 e autovalor real nenhum, e a soma faz-se em C. Não muda com a base, e traço e determinante são os dois invariantes que dão a régua B e C."

# ═══ INFORMAÇÃO: a contagem é que proíbe ═══
p "o que é a entropia de Shannon" "o número médio de bits para dizer qual símbolo saiu, dada a distribuição. É um piso de codificação; partilha o nome com a entropia física mas a pergunta é outra."
p "tudo se pode comprimir" "não, e quem proíbe é a contagem: não há função injetiva das cadeias de n bits nas de menos de n. Todo compressor que encolhe uma entrada cresce outra — o que ele comprime é a ESTRUTURA que a fonte tem."
p "o ruído impede a comunicação" "abaixo da capacidade do canal não: Shannon mostrou que o erro vai a zero com código bastante longo. Acima dela não vai. A capacidade é fronteira, não sugestão."
p "quanto corrige um código" "até à PARTE INTEIRA de (d menos 1) a dividir por 2 erros, e DETETA até d menos 1, onde d é a distância mínima. Os dois números saem da mesma medida — corrigir custa o dobro de detetar."

# ═══ MATÉRIA: espontâneo não quer dizer rápido ═══
p "o que faz um catalisador" "baixa a barreira do caminho, e baixa-a nos dois sentidos igualmente. Muda a velocidade e NÃO muda o equilíbrio — se mudasse, fabricava energia do nada."
p "a reação vai até ao fim" "em geral não: assenta no equilíbrio, onde as duas direções correm à mesma taxa. Ir até ao fim é o caso em que a constante é enorme, e é caso, não regra."
p "o que decide se a reação acontece" "a energia livre diz SE — a de Gibbs a temperatura e pressão constantes, a de Helmholtz a temperatura e volume — e a barreira diz QUANDO. Espontâneo não quer dizer rápido: o diamante é metaestável e não vira grafite à nossa vista."

# ═══ VIDA: o sistema é aberto ═══
p "a evolução é só uma teoria" "no sentido científico, teoria é corpo explicativo com previsão testável, e não palpite. São seleção natural MAIS deriva — e a deriva é a parte que não tem direção nenhuma."
p "a vida contraria a entropia" "não, porque o sistema é ABERTO: a entropia local desce à custa de subir mais no resto, e a conta fecha. A segunda lei só obriga em sistema fechado."
p "o código genético é universal" "quase: é o mesmo na esmagadora maioria, mas há variantes — nas mitocôndrias e em alguns ciliados o mesmo codão lê outro aminoácido. Universal é o esqueleto, não a tabela toda."
