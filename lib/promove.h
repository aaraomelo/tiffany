/* promove.h — A FERRAMENTA DE LEITURA. Escrever promovido ja' traz a leitura.
 *
 * O Aarao: «isso nao e' so' teste, e' ferramenta de leitura — obtem a dual de escrita e
 * promove.»
 *
 * Certo. O que eu tinha escrito media que (x + x^dag)/2 e (x - x^dag)/2 reconstroem x. Mas
 * isso nao e' uma verificacao a fazer depois: e' a OPERACAO. Ela faz duas coisas de uma vez,
 * e nenhuma delas e' testar.
 *
 *   PROMOVE   x  |->  (S, A)      sobe um andar: um valor vira um PAR, e dim dobra —
 *                                 e' o dim A_{n+1} = 2.dim A_n da torre, e sai de graca
 *                                 porque a divisao por dois e' exacta
 *
 *   DESCE     (S, A)  |->  S + A  volta ao andar de baixo, com residuo 0
 *
 * E a dual de escrita: escrever x no disco APAGA o que la' estava — dissipa. Escrever o PAR
 * nao apaga nada, porque o par contem a volta. Em MOVE:
 *
 *      MOVE(slot, -1) escreve      e sozinho e' meia operacao — so' emite
 *      MOVE(slot, +1) le'          e sozinho e' meia operacao — so' absorve
 *      PROMOVE                     escreve E le' na mesma passagem — e' a estrela
 *
 * Nao ha' aqui um passo de verificacao a acrescentar. A leitura vem junto porque S e' o que
 * nao se moveu e A e' o que se moveu, e juntos sao o objecto. Quem escreve o par ja' leu.
 *
 * A divisao por dois e' EXACTA e nao e' sorte: x + x^dag = 2c e x - x^dag = 2(x-c), sempre
 * pares, porque a involucao o garante. Num objecto que nao reverte, ela falha — e e' por isso
 * que a ferramenta so' serve onde ha' dual.
 */
#ifndef PROMOVE_H
#define PROMOVE_H

/* o andar de cima: um par (o que nao se moveu, o que se moveu) */
typedef struct { long S, A; } Par;

/* a involucao de centro c — a estaca */
static inline long pr_dag(long x, long c){ return 2*c - x; }

/* PROMOVE: sobe um andar. UMA aplicacao da involucao, e a leitura vem junto. */
static inline Par promove(long x, long c)
{
    long d = pr_dag(x, c);
    Par p = { (x + d) / 2,        /* S: o que nao se moveu — e' o centro          */
              (x - d) / 2 };      /* A: o objecto, com o sinal da reversao virado */
    return p;
}

/* DESCE: volta ao andar de baixo. */
static inline long desce(Par p){ return p.S + p.A; }

/* e a paridade que faz a promocao fechar em inteiros — falsa num objecto sem dual */
static inline int promove_fecha(long x, long c)
{
    long d = pr_dag(x, c);
    return ((x + d) % 2 == 0) && ((x - d) % 2 == 0);
}

#endif
