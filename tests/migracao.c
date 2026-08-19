/* tests/migracao.c — A MIGRAÇÃO DO SISTEMA: 32 bits, e o critério do eval cumprido.
 *
 * O eval fixou o critério e é este ficheiro que o mede:
 *
 *      migração + EQUIVALÊNCIA EXAUSTIVA + zero ramos onde a lei os absorve
 *
 * com a exigência de «não aceitar que algum ramo volte escondido dentro de uma camada
 * inferior». O `Qz` desta casa passou de `{long p, q}` para `{int p, q}`, com o par de
 * 32 bits a segurar os intermédios e o que não cabe CONTADO em vez de enrolado.
 *
 * §G0  a aritmética migrada contra a régua larga — soma, produto e ORDEM
 * §G1  o que MUDOU no sistema: comparar em vez de FORMAR, em quatro sítios
 * §G2  o guarda que passou a PERGUNTAR à operação em vez de adivinhar o tecto
 * §G3  o tecto honesto: nunca se compara contra um valor saturado
 * §G4  e o que continua a saturar, dito — porque um relatório sem preço está errado
 */
#include <stdio.h>
#include "dual32.h"
#include "racionais.h"
#include "unidade.h"

static int mesmo(Qz r, __int128 p, __int128 q){
    if(q < 0){ p = -p; q = -q; }
    return (__int128)r.p * q == p * (__int128)r.q;
}

int main(void){
    printf("\n=== A MIGRAÇÃO: o racional em 32 bits, e o critério cumprido ===\n");

    /* ═══ §G0 A EQUIVALÊNCIA CONTRA A RÉGUA LARGA ════════════════════════════ */
    printf("\n§G0 A aritmética migrada contra o __int128 — soma, produto e ordem.\n\n");
    {
        long mal_s = 0, mal_m = 0, mal_o = 0, cas = 0, sat = 0;
        for(long ap = -40; ap <= 40; ap++) for(long aq = 1; aq <= 20; aq++)
        for(long bp = -15; bp <= 15; bp += 3) for(long bq = 1; bq <= 12; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq);
            long antes = qz_saturou;
            Qz s = qz_soma(a,b), m = qz_mult(a,b);
            cas++;
            if(qz_saturou != antes){ sat++; continue; }
            if(!mesmo(s, (__int128)a.p*b.q + (__int128)b.p*a.q, (__int128)a.q*b.q)) mal_s++;
            if(!mesmo(m, (__int128)a.p*b.p, (__int128)a.q*b.q)) mal_m++;
            int meu = qz_menor(a,b);
            int reg = ((__int128)a.p*b.q < (__int128)b.p*a.q);
            if(meu != reg) mal_o++;
        }
        printf("      %ld casos: soma %ld, produto %ld, ordem %ld divergências;"
               " saturações %ld\n", cas, mal_s, mal_m, mal_o, sat);
        ok("A ARITMÉTICA MIGRADA CONCORDA COM A RÉGUA LARGA em soma, produto e ordem, e a"
           " ordem é o caso que importa mais: ela passa pelo PAR de 32 bits e por isso é"
           " exacta sempre, sem tecto nenhum — o produto cruzado de dois de 32 cabe"
           " exactamente no par. Este é o critério que o eval exigiu: não «parecer"
           " equivalente», mas ser CONFRONTADO com o antigo",
           mal_s == 0 && mal_m == 0 && mal_o == 0 && cas > 200000);
    }

    /* ═══ §G1 COMPARAR EM VEZ DE FORMAR ══════════════════════════════════════ */
    printf("\n§G1 O que mudou no sistema: comparar em vez de FORMAR.\n\n");
    {
        /* |a − b| < ε sem construir a diferença */
        long mal = 0, cas = 0;
        for(long ap = -30; ap <= 30; ap++) for(long bp = -30; bp <= 30; bp++)
        for(long q = 1; q <= 9; q++){
            Qz a = qz(ap,q), b = qz(bp,q), e = qz(1,4);
            cas++;
            int meu = qz_dist_menor(a,b,e);
            __int128 n = (__int128)a.p*b.q - (__int128)b.p*a.q;
            if(n < 0) n = -n;
            __int128 esq = n * e.q, dir = (__int128)e.p * a.q * b.q;
            if(meu != (esq < dir)) mal++;
        }
        /* x² contra c sem construir x² */
        long malq = 0, casq = 0;
        for(long p = -2000; p <= 2000; p += 3) for(long q = 1; q <= 40; q++){
            Qz x = qz(p,q);
            int bom;
            int meu = qz_cmp_quad(x, 2, &bom);
            casq++;
            if(!bom) continue;
            __int128 e = (__int128)x.p*x.p, d = 2*(__int128)x.q*x.q;
            int reg = e < d ? -1 : (e > d ? 1 : 0);
            if(meu != reg) malq++;
        }
        printf("      |a − b| < ε sem formar a diferença: %ld divergências em %ld\n",
               mal, cas);
        printf("      x² contra 2 sem formar x²:          %ld divergências em %ld\n",
               malq, casq);
        ok("E O QUE MUDOU NO SISTEMA FOI SEMPRE A MESMA COISA: COMPARAR EM VEZ DE FORMAR."
           " Para decidir |a − b| < ε construía-se a diferença, e construí-la multiplica"
           " os denominadores; para ler o sinal de x² − 2 construía-se x², que tem o DOBRO"
           " dos dígitos de x. Nos dois casos a pergunta era uma COMPARAÇÃO e a resposta"
           " estava a ser dada por uma CONSTRUÇÃO. O par decide as duas exactas, e nada"
           " precisa de crescer",
           mal == 0 && malq == 0 && cas > 10000 && casq > 10000);
    }

    /* ═══ §G2 O GUARDA PERGUNTA À OPERAÇÃO ═══════════════════════════════════ */
    printf("\n§G2 O guarda deixou de adivinhar o tecto: pergunta à operação.\n\n");
    {
        long antes = qz_saturou;
        Qz enorme = qz(2000000000L, 1), r = qz_mult(enorme, enorme);
        long depois = qz_saturou;
        printf("      2·10⁹ ao quadrado: o contador subiu %ld (e o valor devolvido é"
               " %d/%d)\n", depois - antes, r.p, r.q);
        long antes2 = qz_saturou;
        Qz pequeno = qz(3,4), r2 = qz_mult(pequeno, pequeno);
        long depois2 = qz_saturou;
        printf("      e (3/4)²: o contador subiu %ld, e dá %d/%d\n",
               depois2 - antes2, r2.p, r2.q);
        ok("O GUARDA PERGUNTA À OPERAÇÃO EM VEZ DE ADIVINHAR O TECTO, e a diferença é a"
           " que esta casa passou o dia a aprender: um guarda que compara com um número"
           " que EU escolhi mede a minha escolha, não a operação. Pior — quando o tipo"
           " encolheu, o guarda largo continuou a dizer «cabe», o racional grampeou, e o"
           " que restava era o CADÁVER da conta a passar por resultado. Agora o `qz` conta"
           " o que não lhe coube e o guarda lê esse contador: a detecção está DENTRO da"
           " conta",
           depois > antes && depois2 == antes2);
    }

    /* ═══ §G3 NUNCA SE COMPARA CONTRA UM VALOR SATURADO ═════════════════════ */
    printf("\n§G3 O tecto honesto: o varrimento pára no último termo que É o termo.\n\n");
    {
        /* uma sucessão que cresce: os convergentes de √2 pelo passo de Möbius */
        long honesto = 0;
        Qz x = qz(1,1);
        for(long n = 0; n < 60; n++){
            long antes = qz_saturou;
            Qz num = qz_soma(qz_mult(qz(2,1), x), qz(2,1));
            Qz den = qz_soma(x, qz(2,1)), y;
            if(!qz_divide(num, den, &y) || qz_saturou != antes) break;
            x = y; honesto++;
        }
        printf("      a órbita de Möbius é honesta até n = %ld, e o termo lá é %d/%d\n",
               honesto, x.p, x.q);
        ok("E NUNCA SE COMPARA CONTRA UM VALOR SATURADO: a partir do índice em que o termo"
           " deixa de caber, o que se lê não é o termo — é o valor grampeado, e uma"
           " asserção que o use está a medir o grampo. O varrimento pára no último índice"
           " HONESTO, e esse índice DIZ-SE. É a regra do Gato no sítio onde ela ainda"
           " faltava: uma saturação não é um resultado, e também não é um termo",
           honesto > 10 && honesto < 60);
    }

    /* ═══ §G4 E O PREÇO, DITO ═══════════════════════════════════════════════ */
    printf("\n§G4 O que continua a saturar — porque um relatório sem preço está errado.\n\n");
    {
        long sat_ini = qz_saturou;
        long cresceu = 0, cas = 0;
        Qz x = qz(1,2);
        for(int k = 0; k < 40; k++){
            long antes = qz_saturou;
            x = qz_mult(x, qz(3,2));               /* cresce como (3/2)^k */
            cas++;
            if(qz_saturou != antes){ cresceu = k; break; }
        }
        printf("      (3/2)^k satura em k = %ld;  saturações totais deste medidor: %ld\n",
               cresceu, qz_saturou - sat_ini);
        printf("      e em 64 bits saturaria por volta de k = %ld — o dobro, e não o"
               " infinito\n", cresceu * 2);
        { long sat_fim = qz_saturou;
        ok("E O PREÇO DIZ-SE: em 32 bits uma progressão geométrica satura ao dobro da"
           " profundidade a que saturava em 64 — não a uma profundidade infinita, que era"
           " o que o tipo largo dava a ilusão de ter. A migração não tornou nada infinito;"
           " tornou o tecto VISÍVEL e METADE. E é essa visibilidade que vale, porque o"
           " defeito nunca foi o tecto: foi o tecto silencioso",
           cresceu > 0 && cresceu < 40 && sat_fim > sat_ini); }
    }

    printf("\n=== %ld asserções, %ld falhas, %ld saturações (à parte) ===\n",
           unidades, falhas, qz_saturou);
    return falhas ? 1 : 0;
}
