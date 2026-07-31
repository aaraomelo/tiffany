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
p "existe o maior número primo" "não. Dado qualquer conjunto finito de primos, o produto deles mais 1 tem um fator primo que não está lá."

# --- reversibilidade: o critério do sistema ---
p "o que torna uma operação segura" "ter dual: poder desfazer-se. O que não reverte perde informação, e o que perde informação não se audita."
p "a soma é reversível" "sim, pela subtração. E é por isso que ela serve para construir: o que ela faz pode ser desfeito."
p "o hash é reversível" "não, e é de propósito: a irreversibilidade está num passo só, a soma final de Davies-Meyer. As 64 rondas revertem."
p "por que a prova de trabalho funciona" "porque a contração não tem dual. Se tivesse, invertia-se em vez de se procurar — a ausência do dual É o trabalho."

# --- as constantes e o que elas são ---
p "o que é o número de ouro" "a raiz de σ = 1 + 1/σ. A cifra dele é [1;1,1,1,...] — só uns, e por isso é o mais mal aproximado por racionais."
p "por que o ouro é o mais irracional" "porque a cifra não tem termo grande. Um termo grande dá um convergente que quase acerta; só uns não dão nenhum."
p "o que é uma fração contínua" "escrever um número como a₀ + 1/(a₁ + 1/(a₂ + ...)). Finita se o número é racional; periódica se é quadrático."
p "o que Lagrange provou sobre frações contínuas" "que a cifra é periódica se e só se o número é irracional quadrático — e o período é invariante completo."

# --- medida e infinito ---
p "o conjunto de Cantor tem medida" "zero. Tira-se o terço do meio a cada nível, e a medida encolhe por 2/3 — vai a zero, mas o conjunto não é vazio."
p "um conjunto de medida zero pode encher" "a soma dele consigo pode: C + C = [0,2]. O conjunto é pó; a operação enche."
p "quantos racionais há" "tantos quantos inteiros — são contáveis. Os reais não são, e a diagonal de Cantor mostra-o."

# --- física, e o que nela é invariante ---
p "o que se conserva num choque" "o momento sempre; a energia cinética só se for elástico. O que se conserva é o que a simetria protege — é Noether."
p "o que o teorema de Noether diz" "a cada simetria contínua corresponde uma quantidade conservada: tempo dá energia, espaço dá momento, rotação dá momento angular."
p "por que a entropia cresce" "porque os estados macroscópicos prováveis são muitos mais que os improváveis. Não é lei de força: é contagem."
p "o que é o cone de luz" "onde a distância própria é nula. Separa o que pode influenciar do que não pode, e é fronteira, não lugar."
