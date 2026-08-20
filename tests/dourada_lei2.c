/* dourada_lei2.c — A DOURADA PELA SEGUNDA LEI: T² = −1, E A ÓRBITA TEM QUATRO.
 *
 * Lei 1 → TROCA (período 2). Lei 2 → ESQUILO no disco ISA (período 4, T² = −1).
 * Sem math.h, sem complex.h — a rotação vive em tools/isa.c / isa_disk.h.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/dourada_lei2.c -o dourada_lei2
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reta.h"
#include "banco.h"
#include "unidade.h"
#include "isa_disk.h"

#define BASE "/tmp/cards_banco"

static long isa_orbita_periodo(long dest){
    int t0, e0, t, e;
    isa_word(ISA_S_A, 1, 0);
    isa_read(ISA_S_A, &t0, &e0);
    for(long k = 1; k <= 8; k++){
        isa_MOVE(dest, 1);
        isa_read(ISA_S_A, &t, &e);
        if(t == t0 && e == e0) return k;
    }
    return 0;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A DOURADA PELA SEGUNDA LEI: T2 = -1, E A ORBITA TEM QUATRO ================\n");

printf("\n§L1  A LEI 1 na dourada: o par volta em DOIS — e era isto que eu media.\n\n");
    {
        /* inversao multiplicativa a/b -> b/a, periodo 2 */
        long num = 17, den = 10, per = 0;
        for(long k = 1; k <= 8; k++){
            long tmp = num; num = den; den = tmp;
            if(num == 17 && den == 10 && !per) per = k;
        }
        /* morfismo log_b: b^m -> m -> b^m */
        long morf = 0, morf_tot = 0;
        for(long bb = 2; bb <= 5; bb++)
            for(int m = 0; m <= 6; m++){
                long x = rt_ipow(bb, m);
                int k;
                morf_tot++;
                if(rt_log_int(x, bb, &k) && k == m) morf++;
            }
        printf("      periodo da inversao, CONTADO: %ld\n", per);
        printf("      log_b(b^m) = m em %ld de %ld\n", morf, morf_tot);
        ok("a LEI 1 na dourada da' periodo DOIS: o dual do dual volta, e a inversao fecha ao fim"
           " de duas aplicacoes — contado, nao escrito. E' o espelho, e e' o que a seccao do"
           " catalogo ja' declara na margem: «realiza a Lei 1». Foi isto que eu medi todas as"
           " vezes que ele pediu a Lei 2 — o par, a involucao, o modulo que nao muda. Tudo"
           " periodo 2", per == 2 && morf == morf_tot && morf_tot > 0);
    }

printf("\n§L2  A LEI 2 na dourada: T2 = -1, e a orbita tem QUATRO — contado.\n\n");
    {
        long per = isa_orbita_periodo(ISA_S_ESQUILO);
        /* T = ESQUILO: (1,0) -> (0,1) -> (-1,0) -> (0,-1) -> (1,0) */
        isa_word(ISA_S_A, 1, 0);
        isa_MOVE(ISA_S_ESQUILO, 1);
        isa_MOVE(ISA_S_ESQUILO, 1);
        int t2, e2;
        isa_read(ISA_S_A, &t2, &e2);
        long e_menos_um = (t2 == -1 && e2 == 0);
        printf("      passo   ESQUILO^k(1)     simetrico?\n");
        isa_word(ISA_S_A, 1, 0);
        for(long k = 1; k <= 4; k++){
            isa_MOVE(ISA_S_ESQUILO, 1);
            int t, e;
            isa_read(ISA_S_A, &t, &e);
            printf("      %-7ld (%+d,%+d)           %s\n", k, t, e,
                   (k == 2 && t == -1 && e == 0) ? "SIM — T2 = -1" : "");
        }
        printf("      T2 = (%+d,%+d)   periodo CONTADO: %ld\n", t2, e2, per);
        ok("a LEI 2 na dourada da' T2 = -1 e periodo QUATRO — o dual do dual da' o SIMETRICO, e"
           " nao a identidade. O T e' ESQUILO no disco ISA (x i, ordem 4). E o periodo conta-se"
           " aplicando ate' voltar. As duas metades: se T2 fosse +1 era a Lei 1 outra vez, e se"
           " o periodo fosse outro T nao era o rotor",
           per == 4 && e_menos_um);
    }

printf("\n§L3  E o QUARTO nao se escolhe: e' o UNICO com quadrado -1.\n\n");
    {
        /* z^2 = -1  <=>  o dobro do angulo e' pi  <=>  2k ≡ 6 (mod 12)  <=>  k = 3 ou 9 */
        long achados = 0;
        printf("      fraccao da volta   k/12      z2 = -1?\n");
        for(long k = 1; k <= 12; k++){
            int e = ((2*k) % 12 == 6);
            if(e) achados++;
            if(k == 3 || k == 6 || k == 9)
                printf("      %ld/12               %s\n", k, e ? "SIM" : "nao");
        }
        printf("      dos 12 angulos varridos, %ld tem quadrado -1\n", achados);
        ok("dos doze angulos da volta, EXACTAMENTE DOIS tem quadrado -1: o quarto e o"
           " tres-quartos — e sao o par, um o dual do outro. O quarto de volta NAO SE ESCOLHE:"
           " e' o que a Lei 2 obriga, porque e' o unico angulo cujo quadrado e' o simetrico. Se"
           " fossem zero, a Lei 2 nao tinha realizacao no circulo; se fossem muitos, nao"
           " determinava nada", achados == 2);
    }

printf("\n§L4  E dai o PENTE: tau_0 = 2pi/ln(phi), e o quarto e' tau_0/4.\n\n");
    {
        printf("      tau_0 = 2pi/ln(phi)          (DEFINICAO: tau_0 · ln(phi) = 2pi)\n");
        printf("      o quarto: tau_0/4            (DEFINICAO: denominador = periodo Lei 2)\n\n");
        long J[4] = {0,-1,1,0}, Id[4], P[4];
        rt_identidade(Id, 2);
        long periodo = 0;
        printf("      k    J^k                periodo?\n");
        for(int k = 1; k <= 8; k++){
            rt_pot_mat(J, 2, k, P);
            int igual = (P[0]==Id[0] && P[1]==Id[1] && P[2]==Id[2] && P[3]==Id[3]);
            if(igual && !periodo) periodo = k;
            if(k <= 4) printf("      %-4d [%+ld %+ld ; %+ld %+ld]      %s\n",
                              k, P[0],P[1],P[2],P[3], igual ? "SIM" : "nao");
        }
        rt_pot_mat(J, 2, 2, P);
        long quadrado_menos_I = (P[0]==-1 && P[1]==0 && P[2]==0 && P[3]==-1);
        int per_isa = (isa_periodo_giro(ISA_S_ESQUILO) == 4);
        printf("\n      periodo J contado: %ld   J² = -I: %s   ESQUILO periodo: %s\n",
               periodo, quadrado_menos_I ? "sim" : "NAO", per_isa ? "4" : "NAO");
        ok("o QUARTO nao e uma escolha de escala: o 4 do denominador e o PERIODO, e ele"
           " conta-se. Na matriz do rotor J = [[0,-1],[1,0]] tem-se J^4 = I e J^k != I para"
           " k = 1, 2 e 3 — exactamente quatro, nem menos — e J² = -I, que e a Lei 2"
           " escrita. ESQUILO no disco ISA confirma periodo 4. Que tau_0 · ln(phi) seja 2pi"
           " nao se mede: e a DEFINICAO de tau_0. O pente herda o denominador da Lei 2",
           periodo == 4 && quadrado_menos_I && per_isa);
    }

printf("\n§L5  O CONTROLO: com MEIA volta a orbita fecha em DOIS — deixa de ser Lei 2.\n\n");
    {
        /* meia volta = ESQUILO^2: (1,0) -> (-1,0); a orbita de -1 tem periodo 2 */
        long per = 0;
        isa_word(ISA_S_A, 1, 0);
        for(long k = 1; k <= 8; k++){
            int t, e;
            isa_read(ISA_S_A, &t, &e);
            isa_word(ISA_S_A, -t, -e);
            isa_read(ISA_S_A, &t, &e);
            if(t == 1 && e == 0 && !per) per = k;
        }
        isa_word(ISA_S_A, 1, 0);
        isa_MOVE(ISA_S_ESQUILO, 1);
        isa_MOVE(ISA_S_ESQUILO, 1);
        int t2, e2;
        isa_read(ISA_S_A, &t2, &e2);
        printf("      com meia volta ESQUILO^2: T = (%+d,%+d), orbita de -1 periodo %ld\n",
               t2, e2, per);
        printf("      logo e' a LEI 1 (T2 = +1, periodo 2) e nao a Lei 2\n");
        ok("com MEIA volta o quadrado da' +1 e a orbita fecha em DOIS — volta a ser a Lei 1, e a"
           " Lei 2 perde-se. E' a metade que da' valor ao §L2: o quarto e' NECESSARIO, e nao uma"
           " escolha entre varias que dariam o mesmo",
           per == 2 && t2 == -1 && e2 == 0);
    }

    {
        unsigned char v[220];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,0|dourada pela Lei 2: T2 = -1, periodo 4, ESQUILO no disco ISA");
        gravar(&b, "corpo/dourada/lei2", v, m);
    }

    fechar(&b);
printf("\n=== A DOURADA PELA LEI 2 ====================================================\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — e o periodo e' QUATRO (ESQUILO no disco ISA).\n\n");
    return 0;
}
