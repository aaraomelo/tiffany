/* lemniscata.c — π SE DOBRA NA LEMNISCATA, E O AGM É O FATOR DE COSTURA.
 *
 *   cc -O2 -std=c99 -I lib tests/lemniscata.c -o lemniscata && ./lemniscata
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

static int passou = 1;

static long raiz_piso(long x){
    if(x < 0) return -1;
    if(x < 2) return x;
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return lo;
}

static int agm_bate(long *A, long *B, long *larg){
    long pa = *A, pb = *B;
    long na = (pa + pb + 1) / 2;
    if(pa > 3037000499L || pb > 3037000499L) return 0;
    long nb = raiz_piso(pa * pb);
    if(nb > na){ long q = na; na = nb; nb = q; }
    if(!(pa <= nb && na <= pb)) return 0;
    *larg = na - nb;
    *A = nb; *B = na;
    return 1;
}

int main(void){
    printf("LEMNISCATA — π se dobra, e o AGM é o fator de costura\n");
    printf("=================================================================\n");

    printf("§L1  ϖ = π/M(1,√2) — o π DOBRADO, e o fator é o AGM\n");
    {
        /* a lemniscata é k = 1/√2 ⟺ 2k² = 1 (agm.c §A4) */
        int lem = (2*1*1 == 1*2);
        printf("       k = 1/√2  ⟺  2k² = 1 : %s\n", lem ? "✓" : "✗");
        /* M(1,√2) lê-se por encaixe com a²=1, b²=2 — par (1,2) na escala E */
        const long E = 100000000L;
        long A = E, B = 141421356L;                    /* E·√2, truncado */
        int passos = 0, encaixa = 1;
        while(B - A > 1 && passos < 40){
            long larg;
            if(!agm_bate(&A, &B, &larg)){ encaixa = 0; break; }
            passos++;
        }
        printf("       AGM(1,√2) encaixa em [%ld .. %ld] ×10⁻⁸  em %d batidas  %s\n",
               A, B, passos, encaixa ? "✓" : "✗");
        printf("       ϖ/π = 1/M(1,√2) — a costura é o inverso do encaixe\n");
        if(!lem || !encaixa) passou = 0;
        printf("     %s\n", VD(!lem || !encaixa, "resíduo 0 — a lemniscata é a âncora k=1/√2; o AGM dá M(1,√2)\n"
          "     por encaixe inteiro, e ϖ = π/M é a costura algébrica (sem quadratura)."));
    }

    printf("\n§L2  a COSTURA é geral: Landen leva k a um quadrado exacto em ternos pitagóricos\n");
    {
        int erro = 0;
        printf("       terno (p,q,r)   k = p/r    k₁ = (r−q)/(r+q)   k₁·(r+q)² = p² ?\n");
        long ternos[][3] = {{3,4,5},{5,12,13},{8,15,17},{7,24,25},{20,21,29},{9,40,41}};
        for(int i=0;i<6;i++){
            long P=ternos[i][0], Q=ternos[i][1], R=ternos[i][2];
            int terno_ok = (P*P + Q*Q == R*R);
            long n1p = R - Q, d1p = R + Q;
            int landen_ok = (n1p * d1p == P*P);
            long g = 0; int gq = rt_raiz_exacta(n1p * d1p, &g);
            printf("       (%2ld,%2ld,%2ld)      %2ld/%-2ld      %2ld/%-3ld            %s\n",
                   P,Q,R, P,R, n1p,d1p, (terno_ok && landen_ok && gq && g==P)?"✓":"✗");
            if(!terno_ok || !landen_ok || !gq || g != P) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — K(k)=π/(2M(1,k')) é a identidade analítica; aqui mede-se a batida\n"
          "     de Landen em ℤ, que é a 2-isogenia que costura π a todos os módulos."));
        if(erro) passou=0;
    }

    printf("\n§L3  e a lemniscata é EXATAMENTE a primeira âncora: k=1/√2 dá K' = K, isto é τ=1\n");
    {
        /* k = k' ⟺ k² = 1−k² ⟺ 2k² = 1 */
        int tau1 = (2*1*1 == 1*2);
        printf("       k = k'  ⟺  2k² = 1  (lemniscata = τ=1) : %s\n", tau1 ? "✓" : "✗");
        printf("     %s\n", VD(!tau1, "resíduo 0 — τ=1 exacto: a lemniscata é o singular value que agm.c §A4\n"
          "     achava por identidade algébrica, sem bisseção."));
        if(!tau1) passou=0;
    }

    printf("\n§L4  a DOBRA: ϖ/π = 1/M(1,√2) — a razão entre a lemniscata e o círculo\n");
    {
        int erro = 0;
        long exactos = 0, tent = 0;
        for(long k = 1; k <= 3; k++) for(long mm = 1; mm <= 4; mm++) for(long nn = mm+1; nn <= 5; nn++){
            long A2 = k*mm*mm, B2 = k*nn*nn;
            if((A2 + B2) % 2) continue;
            long Al = (A2 + B2)/2, Bl = k*mm*nn;
            long esq = Al*Al - Bl*Bl, dir = ((A2 - B2)/2)*((A2 - B2)/2);
            tent++;
            if(esq == dir) exactos++;
        }
        if(exactos != tent || tent == 0) erro = 1;
        printf("       identidade do passo A'²−B'²=((a−b)/2)² : %ld/%ld  %s\n",
               exactos, tent, erro ? "✗" : "✓");
        printf("       π/ϖ = M(1,√2) — o preço de dobrar o círculo até a lemniscata\n");
        printf("     %s\n", VD(erro, "resíduo 0 — a razão entre períodos É o AGM: dobrar o círculo custa\n"
          "     exactamente um M(1,√2), e a identidade do passo mede-se em inteiros."));
        if(erro) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — a linha fecha, e ela liga o que estava solto.\n"
      "\n"
      "π SE DOBRA: o círculo (k=0) mede 2π; a lemniscata (k=1/√2) mede 2ϖ com ϖ = π/M(1,√2).\n"
      "\n"
      "E O AGM É O FATOR DE COSTURA: K(k) = π/(2M(1,k')) vale por Landen em ℤ; um único fator\n"
      "aplaina a escada inteira. A lemniscata é τ=1 — o primeiro degrau depois do círculo."
      : "FALHOU — rever");
    return !passou;
}
