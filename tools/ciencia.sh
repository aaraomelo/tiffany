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
