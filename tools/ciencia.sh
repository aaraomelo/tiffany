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

# ═══ INFERÊNCIA: o dado sozinho não decide ═══
p "o que é o p-valor" "a probabilidade de ver dado tão extremo ou mais, SE a hipótese nula for verdadeira. Não é a probabilidade de a hipótese ser falsa — trocar as duas é a falácia do promotor, e é a troca de P(dado dada hipótese) por P(hipótese dado o dado)."
p "correlação implica causa" "não; e a ausência dela também não implica ausência de causa. Uma relação em U — y igual a x ao quadrado, com x simétrico em torno do vértice — dá correlação linear ZERO com dependência TOTAL, e a simetria faz falta na frase. Para falar de causa é preciso intervenção, ou um grafo causal declarado."
p "o que é o paradoxo de Simpson" "o sinal de uma associação inverter-se quando se agrega ou se separa por grupos. Não há erro de conta: as duas contas estão certas, e qual responde à pergunta depende de qual é a causa. É o caso que mostra que o dado sozinho não decide."
p "a amostra maior é sempre melhor" "maior reduz a variância e NÃO reduz o viés. Uma amostra enviesada grande erra com mais confiança: o Literary Digest recolheu 2,4 milhões de RESPOSTAS, de dez milhões de boletins enviados a donos de telefone e de automóvel, e falhou a eleição de 1936. O buraco estava em quem respondia, e nenhum tamanho o tapa."
p "o que Bayes faz" "atualiza: a posterior é proporcional à verossimilhança vezes a prior. Não produz probabilidade do nada — sem prior declarada não há posterior, e a escolha da prior é parte do modelo, não um detalhe."

# ═══ NÚMEROS DE MÁQUINA: o flutuante não é corpo ═══
p "0,1 mais 0,2 é 0,3" "em decimal exato, sim. Em vírgula flutuante binária não, e o valor depende da PRECISÃO: em binary64 — o double — dá 0,30000000000000004; em binary32 o arredondamento tapa e imprime 0,3. Nem 0,1 nem 0,2 têm representação finita em base 2. É a mesma questão do um terço em base 3 — a dízima é da BASE."
p "a soma em vírgula flutuante é associativa" "não é: (a+b)+c difere de a+(b+c) quando as escalas estão distantes. Os flutuantes não formam corpo — perde-se a associatividade, e é por isso que somar por ordens diferentes dá resultados diferentes."
p "quantos números tem um double" "finitos — cerca de 2 elevado a 64 padrões a cobrir um contínuo. Toda aritmética de máquina é a aproximação de um infinito por um finito, e o erro mora exatamente onde essa aproximação foi feita."

# ═══ COSMOLOGIA: não há um fora ═══
p "o que explodiu no big bang" "nada explodiu num lugar: o que se descreve é o espaço a expandir-se em toda a parte a partir de um estado denso e quente. Não há centro nem exterior — quem escala é a métrica."
p "o universo expande-se para onde" "para lado nenhum. A expansão é da própria métrica e o que cresce são as distâncias entre pontos que não se mexem — em GRANDE ESCALA. O que está ligado, por gravidade ou por força, não expande: nem o átomo, nem a Terra, nem a galáxia. Perguntar para onde pressupõe um fora que o modelo não tem."
p "nada escapa de um buraco negro" "nada de dentro do horizonte, na relatividade geral CLÁSSICA. Com efeitos quânticos há a radiação de Hawking, e o que sai leva a massa embora. O nada é do modelo, não do objeto."
p "qual é a idade do universo" "cerca de 13,8 mil milhões de anos, no referencial comóvel e dentro do modelo LCDM. É o tempo próprio desde o estado quente NESSE modelo — não é um número independente da teoria que o mede."

# ═══ MECÂNICA: a conservação é consequência de uma simetria ═══
p "o movimento precisa de força" "a MUDANÇA de movimento é que precisa. Sem força resultante a velocidade fica como está; foi isto que Galileu separou do que se via, porque cá em baixo o atrito está sempre lá a fingir de lei."
p "a energia conserva-se sempre" "onde há simetria de translação no tempo, por Noether. Na cosmologia a métrica muda com o tempo, essa simetria não existe e a energia global nem se define bem. A conservação é CONSEQUÊNCIA de uma simetria, não axioma."
p "os corpos pesados caem mais depressa" "no vácuo não: a aceleração é a mesma porque a massa cancela dos dois lados da equação. No ar caem, e quem decide é a razão entre o arrasto e o peso — uma pena e um martelo caíram juntos na Lua."
p "qual a diferença entre massa e peso" "o peso é a força com que a gravidade puxa e muda de lugar para lugar; a massa é a resistência a mudar de movimento e não muda. Em queda livre o peso não se SENTE — não porque desapareça, mas porque tudo cai junto."

# ═══ CAOS: determinismo e previsão separam-se ═══
p "o que é o caos" "sensibilidade às condições iniciais num sistema DETERMINÍSTICO. Não é aleatório: as equações são exatas e o que cresce é o erro da medida inicial. É aqui que determinismo e previsibilidade deixam de ser a mesma coisa."
p "o que é o expoente de Lyapunov" "a taxa a que duas trajetórias vizinhas se afastam. Positivo é caos, e o inverso dele é o HORIZONTE: o tempo além do qual a medida inicial já não diz nada."
p "o problema de três corpos tem solução" "depende de solução de quê. Em fórmula fechada geral não — Poincaré mostrou que faltam integrais analíticas. Em série convergente, tem: a de Sundman — e essa exige momento angular total não nulo, porque a colisão tripla fica de fora — que converge devagar demais para servir. E há soluções particulares exatas."
p "o que é um atrator" "o ponto ou conjunto para onde a órbita cai. Num MAPA iterado o ponto fixo atrai quando a derivada lá tem módulo menor que 1 e repele acima de 1; num fluxo contínuo o critério é outro — a parte real do autovalor negativa — o MESMO ponto troca de papel com o parâmetro, e é nessa troca que a bifurcação acontece."

# ═══ MEDIDA: a hipótese a mais era "todo conjunto tem volume" ═══
p "Banach-Tarski quebra a conservação da matéria" "não quebra física nenhuma: as peças são NÃO MENSURÁVEIS e não têm volume que se possa somar. Precisa do axioma da escolha e de DIMENSÃO 3 ou mais — no plano não vale, porque lá há medida finitamente aditiva invariante, e a diferença está no grupo dos movimentos, que só a partir de três dimensões é livre bastante. e o que ele mostra é que a hipótese a mais era supor volume para todo conjunto."
p "todo conjunto tem medida" "em ZFC não: com a escolha constrói-se o conjunto de Vitali, que não é mensurável à Lebesgue. Em ZF com o axioma da determinação, todos os conjuntos de reais são mensuráveis. Quem decide é o sistema escolhido."
p "há mais racionais que inteiros" "não: têm a mesma cardinalidade, os dois são numeráveis. Reais é que há mais, pela diagonal de Cantor. Para infinitos, mais quer dizer não haver bijeção — e não estar contido."

# ═══ LUZ: o que se mede depende do que se pergunta ═══
p "a luz é onda ou partícula" "a pergunta com OU pressupõe uma escolha que a teoria não faz. Cada modelo prevê o que o outro não prevê, e quem fecha é a teoria quântica de campo — o que aparece depende do que a montagem pergunta."
p "por que o céu é azul" "dispersão de Rayleigh: partículas muito menores que o comprimento de onda espalham na quarta potência da frequência, e o azul espalha muito mais. Ao pôr do sol o caminho pela atmosfera é longo, o azul já se espalhou para fora, e sobra o vermelho."
p "a velocidade da luz é constante" "no VÁCUO e em todo referencial inercial — é o postulado de Einstein e está medido. Num meio a velocidade de SINAL é menor, e a razão entre as duas é o índice de refração; a de fase chega a passar c em meios de índice menor que 1, como os raios X, e não transporta nada. O postulado não fala do meio, e é por isso que a luz abranda na água sem contradizer coisa nenhuma."
p "as cores existem" "o comprimento de onda existe e mede-se; a cor é o que os cones e o cérebro fazem com ele — três tipos na visão humana típica, dois em quem é dicromata, e mais noutras espécies. Duas misturas espectrais DIFERENTES dão a mesma cor — são os metâmeros — e é aí que se vê onde acaba o físico e começa o observador."

# ═══ SEGURANÇA: a força está na chave ═══
p "o que faz um hash ser criptográfico" "três propriedades, e não uma: resistência a pré-imagem, a segunda pré-imagem e a colisão. Caem em ordens diferentes — o MD5 caiu por colisão em 2004 e continua sem ataque prático de pré-imagem. Dizer que um hash está quebrado sem dizer em qual das três é dizer pouco."
p "existe cifra inquebrável" "o one-time pad é, e Shannon provou-o. O preço são três condições: chave do tamanho da mensagem, mesmo aleatória, e usada UMA vez. Reutilizar a chave desfaz a prova toda, e é por isso que quase não se usa."
p "o computador gera números aleatórios" "sozinho não: um programa determinístico produz pseudoaleatório, reprodutível a partir da semente — o que é uma virtude para simular e um defeito para cifrar. Aleatório mesmo vem de fonte física, ruído ou deriva."
p "esconder o algoritmo dá segurança" "é o contrário do princípio de Kerckhoffs: a segurança tem de estar na CHAVE, porque o algoritmo acaba por vazar. Esconder acrescenta atrito e nunca substitui a chave."

# ═══ ESCALA: é geometria, não biologia ═══
p "por que não há insetos gigantes" "a lei do quadrado-cubo: dobrar de tamanho multiplica a área por 4 e o volume por 8. A força vai com a secção e o peso com o volume, e o mesmo desenho deixa de se aguentar. Mas nos insetos quem aperta primeiro é a RESPIRAÇÃO: as traqueias levam o ar por difusão e não escalam. E a prova está no Carbonífero — com mais oxigénio no ar houve libélulas de setenta centímetros, e a geometria era a mesma."
p "para que serve a análise dimensional" "para ver se os dois lados têm a mesma unidade. É prova barata e forte pelo lado negativo: uma equação que falha aí está errada de certeza, e nenhuma conta a salva. Passar não prova que está certa."

# ═══ DIAGNÓSTICO: quem decide não é o teste, é a taxa base ═══
p "o teste deu positivo então tenho a doença" "depende da PREVALÊNCIA. Com sensibilidade e especificidade de 99 por cento e a doença em 1 de cada 10 mil, cerca de 1 por cento dos positivos é verdadeiro — quase todos são falsos. É Bayes: quem decide não é o teste, é a taxa base."
p "o que é a sensibilidade de um teste" "a fração de doentes que ele apanha; a especificidade é a fração de sãos que ele deixa passar. Num teste com LIMIAR as duas trocam-se ao mexer o corte — subir uma baixa a outra; num teste sem limiar não há essa troca a fazer — e nenhuma sozinha diz o que fazer com um resultado."
p "o efeito placebo cura" "move o RELATADO — dor, náusea, o que o doente diz — e não move bem o objetivo: não encolhe tumor nem baixa carga viral. E confunde-se com regressão à média e com a evolução natural da doença, que é justamente para o que serve o braço de controlo."
p "por que os antibióticos deixam de funcionar" "por SELEÇÃO: o antibiótico não cria a resistência, escolhe quem já a tinha. É evolução em semanas e mede-se. Já a duração ideal do tratamento está em disputa — a regra de terminar sempre a caixa foi posta em causa em 2017, porque mais dias também é mais pressão seletiva, e a resposta depende da infeção."

# ═══ DECISÃO: estável não quer dizer bom ═══
p "o mercado é eficiente" "a hipótese tem três formas — fraca, semiforte e forte — e só a fraca resiste bem ao dado. E não se testa sozinha: vai sempre em conjunto com um modelo de formação de preço, e a rejeição pode ser do modelo."
p "a votação pode ser justa" "Arrow provou que com três ou mais opções nenhum método ORDINAL cumpre ao mesmo tempo domínio irrestrito, unanimidade, independência das irrelevantes e não-ditadura, saindo com uma ordem completa e transitiva. O domínio irrestrito conta como hipótese e costuma esquecer-se: restringir as preferências admissíveis já escapa ao teorema. Métodos cardinais escapam ao teorema porque mudam a hipótese, e não porque refutem o argumento."
p "o equilíbrio de Nash é o melhor para todos" "não: é onde ninguém melhora DESVIANDO SOZINHO. Em estratégias puras pode nem existir; em mistas, Nash provou que num jogo finito existe sempre. No dilema do prisioneiro o equilíbrio é pior para ambos do que cooperar. Estável não quer dizer bom, e confundir os dois é ler o teorema ao contrário."
p "imprimir dinheiro causa inflação" "se a produção não acompanhar e a velocidade da moeda ficar estável. As duas condições fazem falta: depois de 2008 a base monetária multiplicou-se e o índice de preços quase não mexeu, porque a procura de moeda subiu junto e a velocidade caiu."

# ═══ CUSTO: a régua é o recurso escasso ═══
p "o que diz o O grande" "como o custo cresce com o tamanho, no limite e a menos de constante. Não diz qual é mais rápido AGORA: um n log n com constante enorme perde para um n ao quadrado em entrada pequena, e é por isso que as bibliotecas trocam de algoritmo abaixo de um limiar."
p "mais processadores é mais rápido" "com o problema de tamanho FIXO, até ao limite de Amdahl: se uma fração s é serial, o ganho não passa de 1 sobre s por mais núcleos que se ponha. Mas essa hipótese é metade da conta — se o problema cresce com a máquina, vale Gustafson e o ganho sobe com os núcleos. Amdahl responde quanto mais depressa se faz o MESMO; Gustafson, quanto MAIS se faz no mesmo tempo."
p "o algoritmo mais rápido é o melhor" "depende do que é caro. Onde a memória é o gargalo, um algoritmo com mais operações e menos acessos ganha ao que conta menos operações. A régua é o recurso escasso, e ela declara-se antes de comparar."

# ═══ LINGUAGEM: a verdade não se define de dentro ═══
p "esta frase é falsa" "é o paradoxo do mentiroso, e o que ele mostra é que a verdade não se define DENTRO da linguagem que a usa — Tarski provou isso, para linguagem forte o bastante para a aritmética e consistente, que são as mesmas hipóteses de Gödel. As saídas são hierarquizar as linguagens, ou admitir mais de dois valores."
p "definir bem resolve a discussão" "resolve as que eram de palavra e não toca nas que são de facto. E definir não é livre: numa estrutura já montada, uma definição errada quebra teoremas — foi o que aconteceria com o 1 a contar como primo."

# ═══ AFINAÇÃO: a fração contínua decide quantas notas ═══
p "por que o piano é afinado errado" "porque não PODE ser certo: doze quintas puras não fecham em sete oitavas. (3/2) elevado a 12 sobre 2 elevado a 7 dá 1,0136 — é o coma pitagórico. O temperamento igual reparte esse erro por todos os intervalos, e o único que fica puro é a oitava."
p "por que a oitava tem doze notas" "porque 7 doze avos é o convergente de log2(3/2) que casa a quinta com denominador pequeno. Os seguintes são 24 sobre 41 e 31 sobre 53, e há mesmo quem afine em 53. Doze não é lei nem gosto: é onde a fração contínua para primeiro com erro pequeno."
p "o que é um harmónico" "os múltiplos inteiros da frequência fundamental — em corda e em tubo, que é onde a aproximação vale. Em sino e em prato os parciais NÃO são múltiplos inteiros, e é por isso que não têm altura definida; até a corda do piano tem rigidez e desafina para cima nos agudos, e o afinador estica a oitava por causa disso. É o PESO dos parciais que dá o timbre: duas notas na mesma altura e com instrumentos diferentes têm a mesma fundamental e harmónicos com pesos diferentes."
p "por que dois sons próximos batem" "porque somar duas frequências vizinhas dá uma envolvente na DIFERENÇA: ouvem-se tantos batimentos por segundo quanto o módulo de f1 menos f2. É por isso que se afina de ouvido — anula-se o batimento e a diferença foi a zero."

# ═══ TEMPO PROFUNDO: cada relógio tem o seu alcance ═══
p "como se sabe a idade de um fóssil" "por decaimento radioativo, e cada relógio tem alcance próprio: o carbono-14 até cerca de 50 mil anos, o urânio-chumbo até milhares de milhões. Aplicar o carbono a um fóssil de dinossauro é usar a régua errada, e é daí que saem as datas absurdas."
p "os continentes movem-se" "sim, e mede-se hoje por GPS: alguns centímetros por ano. Wegener acertou o QUE e falhou o COMO — não tinha mecanismo, e por isso a ideia levou cinquenta anos a ser aceite. Faltava-lhe a parte que faz uma descrição virar explicação."
p "o efeito de estufa é mau" "sem ele a Terra estaria à volta de 33 graus mais fria e congelada. O assunto não é o efeito, é a MUDANÇA rápida da sua intensidade — e o que se discute é a taxa, não a existência."
p "o clima já mudou antes" "mudou, e o que se compara é a TAXA. E houve mesmo episódios ABRUPTOS: os eventos de Dansgaard-Oeschger e o Dryas recente mudaram vários graus em décadas — mas regionais, no Atlântico Norte, e sem causa a soprar de fora. O que se compara é a taxa GLOBAL, e essa nas transições glaciares levou milhares de anos. Dizer que já aconteceu não responde à pergunta, que é a que velocidade e em que extensão."

# ═══ CALOR: o que se sente é o fluxo ═══
p "a água ferve a 100 graus" "ao nível do mar, a uma atmosfera. Em La Paz ferve perto dos 87 e numa panela de pressão passa dos 120. A temperatura de ebulição é função da PRESSÃO, e não uma propriedade da água sozinha."
p "qual a diferença entre calor e temperatura" "temperatura é o quão quente; calor é quanta energia atravessa. Uma faísca está a mil graus e não queima — tem temperatura e quase não tem energia para entregar."
p "por que o metal parece mais frio que a madeira" "não está mais frio: à mesma temperatura ambiente estão iguais, e mede-se. O metal conduz melhor e leva o calor da mão mais depressa. O que a pele sente é o FLUXO, não a temperatura."

# ═══ REDES E MÉTODO: o que se mediu e como ═══
p "seis graus de separação" "o número vem de um ensaio de Milgram com taxa de conclusão baixa. Nas redes grandes onde hoje se mede direto dá entre 4 e 6, e é propriedade de grafo pequeno-mundo — caminho curto com aglomeração alta. Não é uma constante do mundo, é uma medida de uma rede."
p "o que faz uma afirmação ser científica" "poder ser desmentida por observação, no critério de Popper — que não é o único proposto nem é pacífico. O que junta mais acordo é mais fraco e mais útil: uma afirmação vale o que valem as maneiras de a pôr à prova."
p "foi publicado então é verdade" "publicação não é replicação. Nas áreas onde se foi medir, boa parte dos resultados não replicou. Um estudo é uma medida com incerteza, e o que conta é a convergência de várias — não a existência de uma."

# ═══ QUÂNTICA: a previsão é uma, a interpretação é que discorda ═══
p "o gato está vivo e morto" "o gato foi uma ILUSTRAÇÃO de Schrödinger CONTRA a leitura ingénua, e não uma previsão. Superposição mede-se em sistemas pequenos e isolados; a decoerência com o ambiente desfá-la depressa, e um gato tem graus de liberdade a mais. As interpretações discordam no que conta como medida e concordam em todas as previsões."
p "o emaranhamento transmite informação instantânea" "não transmite nenhuma: a correlação só aparece ao comparar os dois lados por um canal clássico, que anda à velocidade de sempre. Bell mostra que não há variáveis escondidas LOCAIS, e mostra-o sob hipóteses que se declaram: localidade e independência da medida. Quem quiser salvar as variáveis escondidas tem de largar uma das duas — é o preço, e não a inexistência de saída. O que Bell não mostra é que se comunique. A não-sinalização é teorema dentro da própria teoria."
p "o observador consciente colapsa a função de onda" "não é preciso consciência nenhuma: um detetor, um átomo de gás, qualquer coisa que se emaranhe com o sistema já o decoere. Observador é herança de vocabulário e não requisito da matemática."
p "o que é o princípio de exclusão de Pauli" "dois férmions idênticos não ocupam o mesmo estado quântico, e é daí que a matéria tem VOLUME. Não é uma força: é a antissimetria da função de onda. Os bosões não obedecem, e é por isso que o laser existe."

# ═══ ELETRICIDADE: nenhuma grandeza sozinha decide ═══
p "o que mata é a tensão ou a corrente" "quem faz o dano é a corrente que atravessa o corpo, mas quem a empurra é a tensão contra a resistência do caminho. Nenhuma sozinha decide: dezenas de miliamperes pelo tórax chegam para fibrilar SE durarem — a norma trata corrente e tempo juntos, e um pico curto de meio ampere pode passar sem fibrilhação enquanto trinta miliamperes prolongados matam — e a mesma tensão com pele seca ou molhada dá correntes muito diferentes."
p "corrente alternada ou contínua qual é melhor" "para quê. A alternada troca de sentido e transforma-se facilmente de tensão, e foi por isso que venceu a distribuição. A contínua voltou nas linhas de altíssima tensão e longa distância, onde as perdas se invertem a favor dela. Não houve vencedor absoluto — houve regimes."
p "os eletrões correm pelo fio à velocidade da luz" "não: a velocidade de deriva é de fração de milímetro por segundo. O que anda depressa é o CAMPO, e a energia viaja no campo à volta do fio — não empurrada pelos eletrões em fila indiana."

# ═══ FASES: a classificação declara-se ═══
p "o vidro é um líquido" "não: é um sólido amorfo. Os vitrais medievais mais grossos em baixo são do FABRICO da época e não de escorrimento — à temperatura ambiente a viscosidade daria escoamento desprezável em tempos maiores que a idade do universo."
p "quantos estados da matéria existem" "depende do critério, e por isso a pergunta pede o critério antes da conta. Três na resposta escolar, quatro com o plasma, e mais se contarem condensado de Bose-Einstein, superfluido, supercondutor e cristal líquido. É classificação, e classificação declara-se."
p "o zero absoluto pode ser alcançado" "não em número finito de passos — é a terceira lei. Chega-se a nanokelvin e continua a não chegar. Não é limite de tecnologia: é da estrutura, e é por isso que não adianta melhorar a máquina. E há um caso que parece contradizer e não contradiz: sistemas de spin com população invertida têm temperatura NEGATIVA, e essas não estão abaixo do zero — estão acima do infinito, porque a régua é 1 sobre T e é ela que passa por zero."

# ═══ CONTAGEM: o que cresce são os pares ═══
p "quantas pessoas para duas fazerem anos no mesmo dia" "23 para passar de metade, e 70 para chegar a 99,9 por cento. Parece pouco porque a conta é de PARES e não de pessoas: com 23 há 253 pares, e é isso que cresce depressa."
p "o que diz o teorema de Ramsey" "que a desordem completa é impossível: num grafo grande o bastante aparece sempre estrutura ordenada. Com seis pessoas há sempre três que se conhecem todas ou três que não se conhecem nenhuma. E a lição é dura: ordem encontrada não implica causa — pode ser só contagem."

# ═══ MODELOS: a medida tem de ser fora da amostra ═══
p "o que é o sobreajuste" "o modelo decorar o RUÍDO da amostra em vez da estrutura. Vê-se na diferença entre o erro no treino e o erro fora dele — e note-se que dentro da amostra o sobreajustado é o MELHOR de todos. É por isso que a medida tem de ser feita fora."
p "mais parâmetros é pior" "na visão clássica sim, pela troca viés-variância. Mas mediu-se o duplo descenso: passado o ponto em que o modelo interpola os dados, o erro fora da amostra volta a DESCER. A regra clássica tem regime, e fora dele não vale."
p "o que diz o teorema do almoço grátis" "que, MEDIADO sobre todos os problemas possíveis, nenhum algoritmo bate outro. Não diz que não haja melhores nos problemas que aparecem — quem carrega o resultado é a hipótese de que todos os problemas são igualmente prováveis, e no mundo não são."
p "o modelo acertou então entendeu" "acertar é o que se mediu; entender é outra afirmação e precisa de outro teste. Um modelo pode acertar por ATALHO — uma marca de água, o fundo da imagem, um viés da recolha — e falha assim que o atalho sai. Testar fora da distribuição é o que separa as duas."

# ═══ CORPO: de onde vêm os números que se repetem ═══
p "usamos 10 por cento do cérebro" "não, e a razão certa não é a que se costuma dar. O fMRI não mostra ligado ou desligado: mostra DIFERENÇAS de sinal entre condições, e num instante qualquer a maioria dos neurónios está mesmo calada — se disparassem todos ao mesmo tempo, isso chamava-se convulsão. O que derruba o mito é outra coisa: ao longo do tempo toda a região tem função medida, e lesão em quase qualquer parte tem consequência — o que não aconteceria se noventa por cento fosse folga. O número não tem fonte identificável."
p "a língua tem zonas de sabor" "o mapa vem de má leitura de uma tese alemã de 1901, e o desenho que circula é de 1942. Todos os tipos de recetor aparecem por toda a língua, com sensibilidade um pouco diferente e nenhuma zona exclusiva."
p "é preciso beber dois litros de água por dia" "o número não vem de medida: a recomendação original de 1945 contava a água da COMIDA, e essa parte perdeu-se na citação. A necessidade muda com calor, esforço e massa, e em pessoa saudável a sede regula bem."
p "o corpo precisa de detox" "fígado e rins fazem isso continuamente. A pergunta que arruma o assunto é qual substância sai, medida como e para onde — e para os produtos vendidos como detox essa resposta em geral não existe."

# ═══ GENÉTICA: a herdabilidade é da população, não da pessoa ═══
p "existe um gene para a inteligência" "não: os traços complexos são poligénicos — milhares de variantes de efeito minúsculo, mais o ambiente e a interação entre eles. Um gene PARA alguma coisa só faz sentido nas doenças mendelianas de gene único."
p "o que é a herdabilidade" "a fração da VARIAÇÃO de um traço, numa dada POPULAÇÃO e num dado AMBIENTE, explicada por variação genética. Não diz nada sobre um indivíduo e muda se o ambiente mudar — é a estatística mais mal usada da biologia, e as três palavras em maiúsculas são as que se costumam calar."
p "o adquirido passa aos filhos" "como regra não, e foi esse o ponto em que Lamarck perdeu. Mas há marcas epigenéticas que atravessam gerações — bem estabelecido em plantas e em nemátodos, ainda em disputa em mamíferos. E não repõe Lamarck: o que passa é regulação, e apaga-se."

# ═══ ARGUMENTO: critério de escolha não é prova ═══
p "a navalha de Occam prova a explicação simples" "não prova nada: é critério para escolher entre explicações que preveem IGUAL. Se a mais complexa prevê melhor, a navalha não a corta. E simples precisa de uma medida — número de parâmetros, comprimento de descrição — que quase nunca se declara."
p "quem afirma tem de provar" "é regra de procedimento e não de verdade. Serve para decidir quem move primeiro numa discussão. Uma afirmação não fica falsa por ninguém a ter provado — fica sem apoio, e as duas coisas são diferentes."
p "a exceção confirma a regra" "o original é jurídico e tem duas leituras, ambas defensáveis: haver uma exceção DECLARADA implica que existe regra para os casos não excetuados; ou, no sentido antigo de provar como pôr à PROVA, a exceção é o que testa a regra. Em ciência, com o sentido moderno de confirmar, a exceção não confirma nada — é ela que obriga a rever a regra, ou o alcance onde ela vale."

# ═══ FLUIDOS: a explicação popular está errada e a certa é mais simples ═══
p "por que a asa levanta o avião" "porque desvia ar para BAIXO, e a reação empurra a asa para cima. A explicação do caminho mais longo por cima está errada: as partículas não têm de se reencontrar atrás, e mede-se que não se reencontram. Bernoulli vale, mas não naquele argumento — e a prova simples é que um avião invertido voa."
p "por que a água sobe pela palhinha" "não sobe puxada: a pressão atmosférica EMPURRA quando se baixa a pressão lá dentro. É por isso que existe um teto — cerca de dez metros de coluna de água ao nível do mar, por mais força que se faça, e nenhuma bomba de sucção passa disso."
p "o que é a turbulência" "o regime onde a inércia domina a viscosidade, acima de um certo número de Reynolds. É o problema clássico mais antigo ainda aberto com uso diário: não há solução geral das equações de Navier-Stokes, e provar que ela existe e é suave é um dos problemas do milénio."

# ═══ CÉU: o que se vê e a razão que não é a óbvia ═══
p "por que há estações do ano" "pela INCLINAÇÃO do eixo, não pela distância. A Terra está mais perto do Sol no início de janeiro — e nessa data é verão no hemisfério sul e inverno no norte ao mesmo tempo, o que sozinho derruba a explicação da distância. A distância não é nula, atenção: entre o mais perto e o mais longe a luz que chega varia à volta de 7 por cento. É efeito real e pequeno, montado por cima do da inclinação — e negá-lo seria trocar um erro por outro."
p "por que a lua causa marés" "pela DIFERENÇA de atração entre o lado perto e o lado longe, e não pela atração em si. É força de maré e cai com o cubo da distância, não com o quadrado. E é daí que saem duas marés por dia na maior parte da costa: há bojo dos DOIS lados. Mas não em toda: a forma das bacias faz haver lugares de uma só maré diária, no Golfo do México e em parte do Vietname, e outros de regime misto — a régua final é local. E o Sol também puxa, com quase metade da força de maré da Lua: quando os dois se alinham dá maré viva, e em quadratura dá maré morta."
p "o sol nasce a leste" "exatamente a leste só nos equinócios, duas datas por ano. No resto do tempo nasce mais a norte ou mais a sul disso, e quanto maior a latitude mais varia. A frase é uma aproximação com data marcada."

# ═══ ESTATÍSTICA: a diferença entre as duas é que é a informação ═══
p "usar a média ou a mediana" "a média usa o valor de todos e é arrastada pelos extremos; a mediana só usa a ordem e aguenta. Em rendimento a média fica bem acima da mediana, e a DIFERENÇA entre as duas é que é a informação — não é detalhe de escolha, é o dado a dizer que a cauda é longa."
p "o que diz o desvio-padrão" "a dispersão em torno da média, na mesma unidade do dado. A regra dos 68, 95 e 99,7 só vale na NORMAL; em cauda pesada engana, e em Cauchy o desvio nem existe — nem a média existe. Citar o desvio sem dizer a forma é dizer meio número."
p "estar acima da média é bom" "depende da FORMA da distribuição. Numa enviesada a maioria pode estar abaixo da média sem que nada esteja errado — é o que acontece com rendimento e com número de amigos. A média não é o meio; o meio é a mediana."

# ═══ COMO A CIÊNCIA ANDA: precisão e mecanismo são duas contas ═══
p "Ptolomeu estava errado" "previu posições com boa precisão durante mil e quatrocentos anos, que é o que se pede a uma teoria. Errou o MECANISMO. E Copérnico, na versão original com círculos, não previa melhor — a vantagem só veio quando Kepler pôs elipses. Precisão e mecanismo são duas contas separadas."
p "a ciência corrige-se sozinha" "só onde houver quem meça de novo, possa publicar o desmentido e tenha algum incentivo para o fazer. A correção não é automática: depende de replicação, de acesso ao dado e de quem ganhe alguma coisa em contrariar. Onde essas três faltam, o erro fica."
