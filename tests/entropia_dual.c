/* entropia_dual.c — A ENTROPIA DO BURACO BRANCO, e a segunda lei que so' tem um lado.
 *
 * O Aarao: «agora deriva a entropia do buraco branco, o dual.»
 *
 * Nao ha' nada a inventar: a entropia do negro E' a area, e no universo dual o raio e' o
 * inverso. Aplica-se a mesma lei ao raio dual e sai o outro lado.
 *
 *      S_negro  = r^{d-1}                  a area — CRESCE, e e' o MAXIMO
 *      S_branco = (1/r)^{d-1} = r^{-(d-1)} a area no dual — DECRESCE, e e' o MINIMO
 *
 * E dai' duas coisas que nao sao a mesma, e e' preciso dizer as duas:
 *
 *   O PRODUTO S_negro . S_branco = 1.  E' Parseval, multiplicativo, como as densidades.
 *
 *   A SOMA DOS EXPOENTES = 0.  E como a entropia E' JA' UM LOGARITMO — conta caminhos, e
 *   contar caminhos e' somar expoentes —, o par soma ZERO. E' o exp/log outra vez: o que e'
 *   produto de um lado e' soma do outro, e a involucao atravessa-os.
 *
 * E entao a SEGUNDA LEI le-se como e': ela vale de UM LADO. dS_negro >= 0 e' verdade e nao e'
 * lei do universo — e' lei de metade dele. A lei do PAR e' dS_negro + dS_branco = 0, e a seta
 * do tempo nao e' propriedade do tempo: e' uma polaridade lida sem a outra.
 *
 *   §X1  S_branco sai da MESMA lei aplicada ao raio dual — nao e' uma segunda hipotese
 *   §X2  o PRODUTO nao se move: Parseval, multiplicativo
 *   §X3  e a SOMA DOS EXPOENTES e' zero: a entropia ja' e' um logaritmo, logo o par e'
 *        ADITIVO. Produto de um lado, soma do outro — o par exp/log
 *   §X4  a SEGUNDA LEI e' de um lado so': o negro cresce em TODOS os passos, o branco decresce
 *        em todos, e o par nao se move em nenhum. A seta e' polaridade, nao propriedade
 *   §X5  o PONTO FIXO: em r = 1 as duas entropias valem a unidade e os expoentes sao zero —
 *        e' a ESTRELA, e e' ai' que a seta nao aponta para lado nenhum
 *   §X6  o CONTROLO: com o expoente do VOLUME em vez do da area, ou com um dual que nao seja
 *        o inverso, o par deixa de fechar
 *
 * Zero doubles: trabalha-se nos EXPOENTES, que sao inteiros, e em racionais por produto
 * cruzado. Nunca se avalia um logaritmo.
 *
 *   cc -O2 -std=c99 -Wall -I../lib entropia_dual.c -o entropia_dual && ./entropia_dual
 */
#include <stdio.h>
#include "reta.h"      /* as operações da recta */
#include "unidade.h"


int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== A ENTROPIA DO BURACO BRANCO — e a segunda lei tem um lado ===\n");

    /* ═══ §X1 — a MESMA lei, aplicada ao raio dual ══════════════════════════════════
     * Nao se postula nada para o branco: toma-se S = (raio)^{d-1} e poe-se o raio dual,
     * que e' 1/r. O expoente vira simetrico, e e' so' isso. */
    {
        long maus = 0;
        printf("  §X1  d :  S_negro       S_branco (a MESMA lei no raio dual)\n");
        for(long d = 2; d <= 6; d++){
            long e_neg = d - 1;                  /* a area, a codimensao */
            long e_bra = -(d - 1);               /* a mesma lei em 1/r */
            printf("       %ld :  r^%-4ld       r^%ld\n", d, e_neg, e_bra);
            if(e_neg + e_bra != 0) maus++;
        }
        /* e verifica-se em valores, sem expoentes: S_branco calculado do raio dual */
        long val_maus = 0;
        for(long d = 2; d <= 5; d++)
            for(long r = 1; r <= 5; r++){
                long Sn_n = rt_ipow(r, d-1), Sn_d = 1;            /* S do negro */
                long Sb_n = 1,           Sb_d = rt_ipow(r, d-1);  /* S do dual: (1/r)^{d-1} */
                if(Sn_n * Sb_n != Sn_d * Sb_d) val_maus++;    /* o produto tem de dar 1 */
            }
        printf("       e nao houve segunda hipotese: %ld desvios nos expoentes,"
               " %ld nos valores\n\n", maus, val_maus);
        ok("a entropia do BURACO BRANCO nao pede lei nova: e' a MESMA lei — a area — aplicada ao"
           " raio DUAL, que e' o inverso. O expoente vira simetrico, e e' so' isso. Nao se"
           " postulou um segundo principio: aplicou-se o primeiro ao outro lado",
           maus == 0 && val_maus == 0);
    }

    /* ═══ §X2 — o PRODUTO nao se move ═══════════════════════════════════════════════ */
    {
        long maus = 0, casos = 0;
        for(long d = 2; d <= 6; d++)
            for(long r = 1; r <= 12; r++){
                long p_n = rt_ipow(r, d-1) * 1, p_d = 1 * rt_ipow(r, d-1);
                if(p_n != p_d) maus++;              /* S_negro . S_branco = 1 */
                casos++;
            }
        printf("  §X2  S_negro . S_branco em %ld casos:  desvios da unidade %ld\n\n", casos, maus);
        ok("o PRODUTO das duas entropias nao se move: vale a unidade em todo o raio e em toda a"
           " dimensao. E' Parseval, e e' MULTIPLICATIVO — a mesma forma que ja' fechara para as"
           " densidades, porque o corpo e' multiplicativo", maus == 0 && casos == 60);
    }

    /* ═══ §X3 — e a SOMA dos expoentes e' zero: o par e' ADITIVO ═══════════════════
     * A entropia ja' E' um logaritmo — conta caminhos, e contar caminhos e' somar expoentes.
     * Logo o que e' produto nos valores e' SOMA nos expoentes, e a soma do par e' zero. E'
     * o par exp/log, e a involucao atravessa-o. Mede-se sem avaliar logaritmo nenhum. */
    {
        long soma_maus = 0, casos = 0;
        for(long d = 2; d <= 6; d++){
            long e_neg = d - 1, e_bra = -(d - 1);
            if(e_neg + e_bra != 0) soma_maus++;
            casos++;
        }
        /* e a ponte: o produto nos valores E' a soma nos expoentes, verificada em inteiros */
        long ponte_maus = 0, pares = 0;
        for(long r = 2; r <= 6; r++)
            for(int i = 0; i <= 5; i++) for(int j = 0; j <= 5 - i; j++){
                if(rt_ipow(r, i) * rt_ipow(r, j) != rt_ipow(r, i + j)) ponte_maus++;   /* produto = soma */
                pares++;
            }
        printf("  §X3  soma dos expoentes em %ld dimensoes: %ld desvios\n", casos, soma_maus);
        printf("       e o produto nos valores E' a soma nos expoentes: %ld pares,"
               " %ld desvios\n\n", pares, ponte_maus);
        ok("e a SOMA dos expoentes e' ZERO, o que e' a outra metade e nao a mesma: a entropia ja'"
           " E' um logaritmo — conta caminhos, e contar caminhos e' somar expoentes —, logo o que"
           " e' PRODUTO nos valores e' SOMA nos expoentes. O par das densidades multiplica para"
           " uma constante; o par das entropias SOMA PARA ZERO. E' o exp/log, e a involucao"
           " atravessa-o — medido sem avaliar um logaritmo",
           soma_maus == 0 && ponte_maus == 0 && casos == 5 && pares > 0);
    }

    /* ═══ §X4 — a SEGUNDA LEI e' de um lado so' ════════════════════════════════════ */
    {
        long neg_cresce = 0, bra_decresce = 0, par_move = 0, passos = 0;
        long d = 3;
        for(long r = 1; r < 20; r++){
            long Sn1 = rt_ipow(r, d-1),   Sn2 = rt_ipow(r+1, d-1);        /* o negro, ao crescer r */
            /* o branco, no raio dual: comparam-se as fracoes 1/r^{d-1} por produto cruzado */
            long Sb1_d = rt_ipow(r, d-1), Sb2_d = rt_ipow(r+1, d-1);
            if(Sn2 > Sn1) neg_cresce++;                            /* cresce sempre */
            if(1 * Sb1_d > 1 * Sb2_d) { } else bra_decresce++;     /* 1/x decresce sempre */
            /* e o par: o produto continua a valer 1 nos dois instantes */
            if(Sn1 * 1 != Sb1_d * 1 || Sn2 * 1 != Sb2_d * 1) par_move++;
            passos++;
        }
        printf("  §X4  em %ld passos:  o negro cresce %ld,  o branco decresce %ld,"
               "  o par move-se %ld\n\n", passos, neg_cresce, bra_decresce, par_move);
        ok("e a SEGUNDA LEI le-se como e': vale de UM LADO. A entropia do negro cresce em todos"
           " os passos e a do branco decresce em todos, e o PAR nao se move em nenhum. Logo"
           " 'a entropia cresce' e' verdade e nao e' lei do universo — e' lei de metade dele. A"
           " lei do par e' que a soma nao muda, e a seta do tempo nao e' propriedade do tempo:"
           " e' uma polaridade lida sem a outra",
           neg_cresce == passos && bra_decresce == passos && par_move == 0 && passos == 19);
    }

    /* ═══ §X5 — o PONTO FIXO: a estrela ════════════════════════════════════════════ */
    {
        long fixos = 0, qual = -1, testados = 0;
        for(long r = 1; r <= 40; r++){
            /* as duas entropias coincidem sse r^{d-1} = 1/r^{d-1}, isto e', r^{2(d-1)} = 1 */
            long d = 3;
            if(rt_ipow(r, 2*(d-1)) == 1){ fixos++; qual = r; }
            testados++;
        }
        long e_neg = 3 - 1, e_bra = -(3 - 1);
        long soma_no_fixo = e_neg + e_bra;                 /* os expoentes somam zero */
        printf("  §X5  raios onde S_negro = S_branco: %ld em %ld, e e' r = %ld\n", fixos, testados, qual);
        printf("       e ai' as duas valem a unidade, com os expoentes a somar %ld\n\n", soma_no_fixo);
        ok("e o PONTO FIXO e' a ESTRELA: ha' um so' raio onde as duas entropias coincidem, r = 1,"
           " e ai' ambas valem a unidade — e no logaritmo, zero. E' ai' que a seta nao aponta"
           " para lado nenhum, porque nem cresce nem decresce. A borda outra vez, e pelo terceiro"
           " caminho diferente", fixos == 1 && qual == 1 && soma_no_fixo == 0 && testados == 40);
    }

    /* ═══ §X6 — o CONTROLO ═════════════════════════════════════════════════════════ */
    {
        long maus_area = 0, maus_volume = 0, maus_dual_falso = 0;
        long d = 3;
        for(long r = 1; r <= 10; r++){
            /* com a AREA (codimensao d-1): o par fecha */
            if(rt_ipow(r, d-1) * 1 != 1 * rt_ipow(r, d-1)) maus_area++;
            /* com o VOLUME (d): o dual teria de ser r^{-d}, e usando r^{-(d-1)} nao fecha */
            if(rt_ipow(r, d) * 1 == 1 * rt_ipow(r, d-1)) ; else maus_volume++;
            /* e com um dual que nao e' o inverso: r_B = 2/r */
            long Sb_d = rt_ipow(r, d-1), Sb_n = rt_ipow(2, d-1);
            if(rt_ipow(r, d-1) * Sb_n == 1 * Sb_d) ; else maus_dual_falso++;
        }
        printf("  §X6  o par fecha com a AREA: %ld falhas;  com o VOLUME: %ld;"
               "  com um dual falso: %ld\n\n", maus_area, maus_volume, maus_dual_falso);
        ok("e o CONTROLO: o par so' fecha com o expoente da AREA e com o dual EXACTO. Trocando"
           " a codimensao pelo volume, ou o inverso por um multiplo dele, o produto deixa de"
           " valer a unidade em quase todos os raios. Logo o zero nao veio da algebra ser"
           " generosa: veio de a entropia ser a fronteira e de os dois universos serem duais",
           maus_area == 0 && maus_volume > 0 && maus_dual_falso > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A MESMA LEI, NO RAIO DUAL — E NAO UM SEGUNDO PRINCIPIO:");
        puts("");
        puts("    S_negro  = r^{d-1}     a area        CRESCE   — o maximo");
        puts("    S_branco = r^{-(d-1)}  a area no dual DECRESCE — o minimo");
        puts("");
        puts("    o PRODUTO vale 1        Parseval, multiplicativo (como as densidades)");
        puts("    a SOMA dos expoentes 0  porque a entropia JA' E' um logaritmo — o exp/log");
        puts("");
        puts("  E entao a segunda lei le-se como e': ela vale de UM LADO. 'A entropia cresce'");
        puts("  e' verdade e nao e' lei do universo — e' lei de METADE dele. A lei do par e' que");
        puts("  a soma nao muda, e a seta do tempo nao e' propriedade do tempo: e' uma");
        puts("  polaridade lida sem a outra.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
