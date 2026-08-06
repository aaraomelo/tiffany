#!/bin/sh
# ciencia.sh — o corpus científico. Só o que se sustenta, e nada de "mais ou menos".
#
# A regra que o Aarão deu, e é o critério: não se julga a roupa. Verifica-se se está COMPLETO
# (tem dual), se é REVERSÍVEL, e se não VAZA. O que entra aqui obedece a isso — e onde uma coisa
# só vale sob uma régua, a régua vai dita, senão a afirmação fica pela metade.
#
#   ./ciencia.sh <base>
# corre de onde for chamado: os caminhos relativos daqui contam a partir de tools/
cd "$(dirname "$0")" || exit 1
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

# ═══ ÁGUA E QUÍMICA: a escala tem condição ═══
p "por que o gelo flutua" "porque a ligação de hidrogénio obriga o sólido a uma rede aberta, e assim ele fica MENOS denso que o líquido. É anomalia — quase tudo o resto afunda no próprio líquido. E a água tem densidade máxima perto dos 4 graus e não a 0, que é o que mantém o fundo do lago líquido enquanto a superfície gela."
p "o que é o pH" "menos o logaritmo da ATIVIDADE dos iões de hidrogénio, em solução AQUOSA, e cada unidade é um fator de dez. A definição por concentração é a aproximação que vale em solução diluída; em solução concentrada as duas separam-se, e quem manda é a atividade. A escala não se transporta para outros solventes. E o 7 neutro é a 25 graus: a água pura a 50 graus tem pH perto de 6,6 e continua exatamente neutra."
p "orgânico quer dizer sem química" "tudo é química, incluindo a água e o sal. Orgânico em química é composto de carbono; orgânico no rótulo é um conjunto de regras de produção. São duas palavras iguais que não falam uma com a outra — a confusão é de vocabulário e não de facto."

# ═══ ENERGIA: são duas proibições diferentes ═══
p "existe máquina de movimento perpétuo" "de primeira espécie não, pela conservação: não se cria energia. De segunda espécie também não, pela segunda lei: não se converte calor em trabalho inteiro num ciclo. As duas proibições são diferentes, e dizer só que não existe deixa por explicar qual delas se está a invocar."
p "qual o rendimento máximo de um motor" "o de Carnot: 1 menos a razão das temperaturas ABSOLUTAS. É teto e não meta — depende só das duas temperaturas e nenhuma engenharia o passa. Entre 600 e 300 kelvin, 50 por cento, por melhor que seja a máquina."
p "a energia acaba" "não se gasta, DEGRADA-SE: passa a formas menos aproveitáveis. O que se esgota é a energia livre, a parte que ainda pode fazer trabalho. É por isso que a conta útil é de exergia e não de energia — a energia total continua lá, e não serve."

# ═══ ÓPTICA: cada um vê o seu ═══
p "o espelho troca a esquerda e a direita" "não troca isso: troca a FRENTE com o trás. Quem inverte esquerda e direita é a nossa descrição, porque imaginamos que demos meia-volta em vez de nos vermos invertidos em profundidade. O espelho inverte um eixo só, o que aponta para ele."
p "por que o mar é azul" "parte é o céu refletido, e não é só isso: a própria água absorve o vermelho muito mais que o azul, e ao fim de alguns metros o que sobra é azul. Vê-se numa piscina funda de fundo branco, sem céu nenhum a refletir."
p "por que o arco-íris tem essas cores" "por dispersão: o índice de refração da água muda com o comprimento de onda e cada cor sai num ângulo próprio, à volta de 42 graus para o vermelho e 40 para o violeta. E o arco não está num sítio — é um conjunto de DIREÇÕES, e por isso cada pessoa vê o seu."

# ═══ MEDIDA: dizer o número é dizer a incerteza ═══
p "quanto é um quilograma" "desde 2019 é definido pela constante de Planck, e não pelo cilindro de Paris. Em 2019 mudaram quatro — o quilograma, o ampere, o kelvin e a mole; o segundo já assentava no césio desde 1967 e o metro na velocidade da luz desde 1983. O que 2019 fechou foi o último ARTEFACTO: o cilindro derivava com o tempo e ninguém tinha contra o que o comparar, e uma constante não deriva."
p "precisão e exatidão são o mesmo" "não: exatidão é acertar no alvo, precisão é repetir no mesmo sítio. Dá para ser muito preciso e inexato — os tiros todos juntos e longe do centro — e esse é o caso perigoso, porque a repetição parece confirmação."
p "quantos algarismos escrever num resultado" "tantos quantos a incerteza sustentar, e não os que a calculadora despeja. Pôr 3,14159265 num valor medido com dois dígitos é inventar precisão que não se mediu — o número sem a incerteza ao lado está incompleto."

# ═══ MÉTRICA: pi é propriedade da régua, não do círculo ═══
p "quanto vale pi" "3,14159… na métrica euclidiana. A razão entre perímetro e diâmetro depende da MÉTRICA: na do taxista, onde a distância é a soma dos catetos, o círculo é um losango e essa razão dá exatamente 4. Pi é propriedade da régua, e não do círculo."
p "o teorema de Pitágoras é sempre verdade" "no plano euclidiano. Na esfera e na hiperbólica há versões próprias, com cossenos e com cossenos hiperbólicos, e a euclidiana é o limite delas para triângulos pequenos. É verdade com a geometria declarada, como tudo o resto."
p "a distância mais curta é a linha reta" "na geometria euclidiana, e aí quase por definição de reta. Em superfície curva a mais curta é uma GEODÉSICA — na esfera, o arco MENOR de círculo máximo, porque o arco maior também é geodésica e não é o mais curto: geodésica é mínimo local, e mínimo local não é mínimo — e é por isso que as rotas longas parecem desviar-se no mapa. Quem engana não é o avião, é a projeção."

# ═══ CONVENÇÃO OU ESTRUTURA: quase sempre é estrutura ═══
p "por que zero fatorial é 1" "porque é o produto VAZIO, e o produto vazio é o neutro da multiplicação. E porque a recursão obriga: de n! igual a n vezes (n-1)! com n igual a 1 sai 1! = 1 vezes 0!, logo 0! = 1. Não é convenção solta — é o que a estrutura exige para fechar."
p "por que menos vezes menos dá mais" "porque a distributividade obriga. Menos um vezes menos um, mais menos um vezes um, é menos um vezes zero, que é zero — logo o primeiro termo é 1. Escolher outra coisa quebrava a distributividade, e o que sobrava já não era anel."
p "por que qualquer número elevado a zero é 1" "porque a lei a elevado a m sobre a elevado a n igual a a elevado a m menos n obriga, tomando m igual a n. Vale para a diferente de zero: em 0 elevado a 0 a lei não decide, e aí a resposta volta a depender do contexto."

# ═══ CONCORRÊNCIA: o defeito que passa nos testes ═══
p "o que é uma condição de corrida" "duas linhas de execução a mexer no mesmo estado sem ordem garantida, com o resultado a depender do escalonamento. É o defeito que PASSA nos testes: a execução que falha pode ser uma em milhões, e correr mil vezes verde não prova nada."
p "o que é um impasse" "quatro condições ao mesmo tempo: exclusão mútua, posse e espera, ausência de preempção, e espera circular. Quebrar qualquer uma delas resolve — e a mais barata de quebrar costuma ser a circular, ordenando os recursos e pedindo sempre pela mesma ordem."
p "uma operação de uma linha é atómica" "não é atómica por ser de uma linha. Incrementar um contador é ler, somar e escrever, e as três podem ser interrompidas no meio. A atomicidade é propriedade da máquina e do tipo, não do tamanho do texto que se escreveu."

# ═══ CALENDÁRIO: e o gregoriano não é o melhor ═══
p "o ano tem 365 dias" "tem 365,24219, e daí o bissexto. A regra gregoriana tem três níveis — de quatro em quatro sim, de cem em cem não, de quatrocentos em quatrocentos sim — e por isso 2000 foi bissexto e 1900 não foi. É uma fração a ser aproximada por outra de denominador pequeno."
p "o calendário gregoriano é o mais exato" "a pergunta não fecha sem dizer EXATO CONTRA QUÊ, e é aí que está tudo. Ele usa 97 dias extra em 400 anos, ou seja 0,2425. Contra o ano trópico MÉDIO (0,242190) erra 3,1 décimos de milésimo, e perde para o ciclo persa de 8 em 33 e muito para o de 31 em 128, que erra mil vezes menos. Mas o gregoriano não foi feito para o ano médio: foi feito para segurar o equinócio de março, e contra o ano VERNAL (0,242374) o erro dele cai para metade — e o 31 em 128, o melhor na outra régua, passa a ser o PIOR dos três. A mesma medida, duas réguas, e a ordem inverte-se. Melhor em ambas fica o persa, que ainda por cima tem ciclo doze vezes mais curto."
p "o dia tem 24 horas" "o dia SOLAR tem, em média. O sideral, que é uma volta da Terra sobre si, tem 23 horas e 56 minutos. E a rotação está a abrandar: o relógio atómico e a Terra já não concordam, e quem se ajustava com segundos intercalares era o relógio civil — decidiu-se em 2022 abandoná-los até 2035, porque o salto partia mais sistemas do que resolvia."
p "o que é um fuso horário" "uma convenção política sobre uma faixa geográfica, e não uma consequência da geometria. Não são 24 iguais: há fusos de meia hora e de quarenta e cinco minutos, e a China inteira usa um só apesar de atravessar cerca de cinco."

# ═══ IMUNIDADE: o limiar é função do agente ═══
p "o antibiótico serve para a gripe" "não: antibiótico age em bactéria e a gripe é vírus. Tomar sem indicação não encurta nada e ainda seleciona resistência — é o mesmo mecanismo de seleção, a correr sem nenhum benefício do outro lado da conta."
p "o que é a imunidade de grupo" "o limiar acima do qual cada infetado passa a doença a menos de uma pessoa e o surto morre sozinho. Sai de 1 menos 1 sobre R0: com R0 de 3 dá uns dois terços, e no sarampo, com R0 acima de 12, passa dos 90 por cento. Não é um número — é função do agente e do padrão de contacto. E a fórmula supõe MISTURA HOMOGÉNEA: com contacto heterogéneo, quem mais espalha infeta-se cedo e o limiar efetivo desce abaixo do que a conta simples dá."
p "a vacina dá a doença" "depende do TIPO, e o tipo declara-se. As inativadas e as de subunidade não têm nada capaz de se replicar. As atenuadas têm o agente enfraquecido e podem dar quadro leve — e é exatamente por isso que se contraindicam em imunodeprimidos."
p "por que o vírus muta" "porque copiar erra, e os de RNA erram mais por à maioria faltar revisão da cópia — com exceções que contam: os coronavírus TÊM uma exonuclease corretora e por isso mutam bem mais devagar que a gripe. Mutar não é estratégia nem intenção: a esmagadora maioria das mutações é neutra ou pior para o próprio vírus, e o que se vê é a SELEÇÃO a ficar com as poucas que ajudam."

# ═══ COR: uma soma, a outra subtrai ═══
p "por que misturar tintas dá diferente de misturar luzes" "porque uma soma e a outra subtrai. A luz é aditiva: vermelho mais verde mais azul dá branco. A tinta é subtrativa — cada pigmento TIRA uma faixa do que reflete, e juntar pigmentos tira quase tudo e aproxima do preto."
p "o preto é uma cor" "como pigmento é; como luz é a ausência dela. A pergunta troca de resposta conforme se fale do que entra no olho ou do que se põe no papel, e é a mesma distinção que separa o aditivo do subtrativo."
p "por que o ecrã só tem três cores" "porque o olho humano típico tem três tipos de cone, e o ecrã não reproduz o espectro: produz um METÂMERO — outra mistura física que o olho não distingue da original. Mas três primárias NÃO chegam para tudo: só se alcança o que cai dentro do triângulo que elas formam, e fora dele ficam cores reais que nenhum ecrã RGB produz — os verdes e os ciãs muito saturados. É por isso que se acrescentam primárias em ecrãs de gama larga."

# ═══ LINGUAGEM: a contagem depende do que conta ═══
p "há línguas mais primitivas" "não se encontrou nenhuma: toda língua falada tem gramática completa e produtividade sem limite. O que varia é o vocabulário especializado, que acompanha o que a comunidade faz — e isso muda numa geração, sem a gramática mexer."
p "a tradução perfeita existe" "entre línguas naturais quase nunca é bijetiva: uma palavra vai em várias e várias vão numa. O que se conserva é a função no contexto e não a correspondência de palavras, e é por isso que traduzir de volta não devolve o original."
p "quantas palavras tem uma língua" "a pergunta não fecha sem dizer o que conta como palavra: flexões, compostos, termos técnicos, palavras mortas. As contagens de dicionário são decisões editoriais, e comparar números entre línguas é comparar decisões diferentes, não línguas."

# ═══ POPULAÇÃO: a taxa é que é o assunto ═══
p "a população cresce exponencialmente" "só enquanto o recurso não limitar. Com limite o crescimento é logístico e assenta na capacidade de carga. E na humana a taxa está a CAIR há décadas: mais de metade dos países, com cerca de dois terços da população mundial, já tem fecundidade abaixo da reposição."
p "o que é a capacidade de carga" "o tamanho que o ambiente sustenta DADA uma tecnologia e um consumo. Não é constante da natureza: muda com o que se come, com o que se desperdiça e com o rendimento agrícola — e por isso citá-la sem essas três é citar meio número."
p "a extinção é natural então não importa" "a extinção de fundo é natural; o que se mede hoje é uma TAXA muito acima dela. É a mesma resposta do clima: quem diz que já acontecia está a responder à existência quando a pergunta era a velocidade."

# ═══ LINGUAGENS: quase nada do que se atribui à linguagem é da linguagem ═══
p "qual é a linguagem de programação mais rápida" "a pergunta não fecha: velocidade é da IMPLEMENTAÇÃO e do que se mede, não da gramática. O C costuma sair depressa porque o seu modelo de execução expõe a máquina — mas um algoritmo mau em C perde para um bom em qualquer outra, e a diferença entre algoritmos costuma ser de ORDEM enquanto a entre linguagens é de CONSTANTE. Mas nem sempre: uma linguagem que só ofereça listas ligadas, ou estruturas persistentes por partilha, impõe um fator logarítmico onde outra tem acesso direto — e aí a diferença sobe à ordem e deixa de ser constante. É a mesma imutabilidade que dá segurança sem fechadura a cobrar aqui o preço."
p "compilada ou interpretada qual é melhor" "isso não é propriedade da linguagem: é da implementação. Há C interpretado e há Python compilado à frente ou em tempo de execução. A mesma gramática admite as duas — a distinção é do ferramental, e falar dela como se fosse da linguagem já é o erro."
p "o que é tipagem estática" "verificar os tipos ANTES de correr. E há dois eixos que se confundem sempre: estática ou dinâmica é QUANDO se verifica; forte ou fraca é QUANTO se converte à socapa. São independentes — Python é dinâmica e forte, C é estática e fraca."
p "o que é um tipo" "um conjunto de valores mais as operações que o respeitam. É o CORPO declarado da variável, e verificar tipos é uma prova pequena e barata de que só se opera lá dentro. Quem escreve o tipo está a dizer em que estrutura a conta é válida."
p "o que é ser Turing completo" "poder calcular o que uma máquina de Turing calcula. Quase toda a linguagem de uso geral é, e por isso a afirmação diz pouco — e há quem faça o contrário de propósito: as linguagens totais, como as dos assistentes de prova, abrem mão da completude justamente para GARANTIR que todo programa termina. Perde-se poder e ganha-se uma prova. o que separa as linguagens não é o que PODEM computar — é o que tornam fácil de exprimir e difícil de errar."
p "o que é uma linguagem funcional" "a que trata funções como valores e desencoraja o estado mutável. Não é o oposto de imperativa: é uma disciplina sobre o EFEITO, e hoje quase toda a linguagem grande tem as duas maneiras lá dentro."
p "o coletor de lixo elimina as fugas de memória" "elimina as de esquecer de libertar, e não as lógicas: ele devolve o que já não se ALCANÇA, e um cache sem limite ou um ouvinte nunca removido continuam alcançáveis. Vaza-se com coletor e tudo — só muda a forma da fuga."
p "por que existem ponteiros" "porque a memória é endereçável e às vezes é preciso falar do LUGAR e não do valor. O que dá problema não é o ponteiro: é o ponteiro sem dono declarado. As linguagens que resolveram isto não os tiraram — declararam a POSSE e o tempo de vida, que é a informação que faltava."
p "o que é comportamento indefinido" "o padrão dizer que, se aquilo acontecer, não promete nada. E não quer dizer fazer coisa aleatória: quer dizer que o compilador pode ASSUMIR que nunca acontece e otimizar em cima disso — é daí que saem os casos em que a verificação que se escreveu simplesmente desaparece do binário."
p "recursão ou iteração" "iguais em poder. A recursão gasta pilha, a iteração gasta escrita à mão. Onde o compilador garante a otimização de chamada em cauda, a recursão em cauda VIRA um ciclo e não gasta pilha nenhuma — mas é preciso que ele a garanta, e nem todos garantem. Não é gosto: é o que está escrito no contrato do compilador."
p "o que é imutabilidade" "o valor não mudar depois de criado. Isso torna a partilha entre linhas de execução segura sem fechadura, e torna barato voltar atrás — o que se paga é a escrita no lugar, que às vezes é justamente o que custa caro."
p "o código auto-documentado dispensa comentários" "o nome diz o QUE e o comentário existe para o PORQUÊ. Nomear bem mata os comentários que repetem o código, e não mata os que registam a decisão tomada, a alternativa que se recusou e a restrição de fora — nada disso está no código, por melhor que ele seja."
p "o que é uma linguagem de baixo nível" "a que abstrai pouco da máquina — e é RELATIVO: o C era considerado alto nível em 1972 diante do assembler. O nível não está no texto, está na distância ao modelo de execução da máquina de que se fala, e essa máquina muda."
