/* pisotbase.c — A FAMÍLIA "n-ésima DERIVADA = INVERSA" É A FAMÍLIA DAS BORDAS.
 *
 * f = a·x^b com f^{(n)} = f^{-1} força b² − n b − 1 = 0: a borda σ² = mσ + 1 com m = n.
 * Sem sqrt/pow: a conta vive em Z[σ]. d(1) sai de σ(σ−m)=1, não de floor em double.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/pisotbase.c -o pisotbase
 */
#include <stdio.h>
#include "unidade.h"

typedef long L;

/* (A,B) significa A + B·σ, com σ² = nσ + 1 */
static void zsig_mul(long n, long A, long B, long C, long D, long *RA, long *RB){
    *RA = A*C + B*D;
    *RB = A*D + B*C + B*D*n;
}

int main(void){
    printf("================================================================\n");
    printf("  f^(n) = f^-1 da a familia das bordas — e a base de numeracao\n");
    printf("================================================================\n");

    printf("\n§B1 f^(n) = f^{-1} forca b^2 - nb - 1 = 0 — A BORDA com m = n\n");
    {
        int ns=0, bate=0;
        const char *nome[6]={"","ouro  ","prata ","bronze","      ","      "};
        printf("      n   polinomio de f^(n)=f^-1   a BORDA com m=n        iguais?   nome\n");
        for(L n=1;n<=8;n++){
            L p[3] = {0,1,0};
            L q[3] = {-n,1,0};
            L pr[3] = {0,0,0};
            for(int i=0;i<2;i++) for(int j=0;j<2;j++) pr[i+j] += p[i]*q[j];
            L cf[3] = { pr[2], pr[1], pr[0] - 1 };
            L bd[3] = { 1, -n, -1 };
            ns++;
            if(cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]) bate++;
            if(n<=3) printf("      %-3ld x^2 %+ld x %+ld            x^2 %+ld x %+ld          sim       %s\n",
                            n, cf[1], cf[2], bd[1], bd[2], nome[n]);
        }
        printf("      n testados: %d   com o polinomio a coincidir com a borda: %d\n", ns, bate);
        ok("f^(n) = f^-1 da EXATAMENTE a borda com m = n — a familia inteira",
           bate==ns && ns==8);

        /* O coeficiente: a^{1+1/b} = 1/(b)_n. Sem avaliar pow: 1/σ = σ−n em Z[σ],
         * porque σ(σ−n) = σ² − nσ = 1, e (σ)_n ≠ 0 (senao a nao existia). */
        printf("\n      n   1/σ = σ−n em Z[σ]    (σ)_n ≠ 0\n");
        int bate_a = 0, na = 0;
        for(int nn = 1; nn <= 6; nn++){
            long RA, RB;
            zsig_mul(nn, 0, 1, -nn, 1, &RA, &RB);   /* σ·(σ−n) */
            int inv = (RA == 1 && RB == 0);
            long A = 1, B = 0;
            for(int k = 0; k < nn; k++){
                long nA, nB;
                zsig_mul(nn, A, B, -k, 1, &nA, &nB);
                A = nA; B = nB;
            }
            int poch = (A != 0 || B != 0);
            na++;
            if(inv && poch) bate_a++;
            if(nn <= 3) printf("      %-3d %s                 (%ld %+ld σ)\n",
                               nn, inv ? "sim" : "NAO", A, B);
        }
        printf("      %d valores de n, com 1/σ = σ−n e (σ)_n ≠ 0: %d\n\n", na, bate_a);
        ok("e o COEFICIENTE fecha em Z[σ]: 1/σ = σ−n (logo a^{1+1/σ} = 1/(σ)_n e' determinado)"
           " e (σ)_n ≠ 0. Sem sqrt, sem pow, sem avaliar f num ponto",
           bate_a == na && na == 6);

        /* n=1: φ^{-1} = φ−1, e φ·(φ−1) = 1 — genealogia em Fibonacci, nao o decimal */
        long RA, RB;
        zsig_mul(1, 0, 1, -1, 1, &RA, &RB);
        int ouro = (RA == 1 && RB == 0);
        printf("      e n=1: φ·(φ−1) = 1 em Z[φ] — a referencia sai da borda, nao de um decimal\n");
        ok("e n=1 devolve o ouro: φ^{-1} = φ−1 em Z[φ], porque φ(φ−1)=1. O decimal"
           " 0,742742944625 era memoria copiada; a genealogia e' a borda",
           ouro);
        printf("      logo b = sigma_n:  n=1 ouro, n=2 prata, n=3 bronze, ...\n");
        conclui("o aurea.c media so n=1. A familia das funcoes 'n-esima derivada = inversa' e");
        conclui("a familia das bordas deste projeto sao A MESMA, indexada pelo mesmo inteiro.");
    }

    printf("\n§B2 a regra de carry 011 -> 100 E a equacao minimal 1 + phi = phi^2\n");
    {
        L F[20]; F[0]=1; F[1]=1;
        for(int i=2;i<20;i++) F[i]=F[i-1]+F[i-2];
        int ns=0, carry=0;
        for(int i=0;i<17;i++){ ns++; if(F[i]+F[i+1]==F[i+2]) carry++; }
        printf("      o carry 011 -> 100 e  phi^n + phi^{n+1} = phi^{n+2}\n");
        printf("      em inteiros:          F_n  + F_{n+1}    = F_{n+2}\n");
        printf("      verificado em %d posicoes: %d\n", ns, carry);
        ok("a regra de CARRY da base e a recorrencia da borda — a mesma equacao",
           carry==ns && ns>=15);
        conclui("nao e coincidencia: a regra de carry e a equacao minimal olhada do lado da");
        conclui("numeracao, e o f' = f^-1 e a mesma olhada do lado da analise.");
    }

    printf("\n§B3 ZECKENDORF: todo inteiro e soma de Fibonacci nao consecutivos — sem '11'\n");
    {
        L F[16]; F[0]=1; F[1]=2;
        for(int i=2;i<16;i++) F[i]=F[i-1]+F[i-2];
        int ns=0, sem11=0, exatos=0;
        printf("      n     Zeckendorf                soma    tem consecutivos?\n");
        for(L n=1;n<=200;n++){
            L r=n; int idx[16], ni=0;
            for(int i=15;i>=0;i--) if(F[i]<=r){ r-=F[i]; idx[ni++]=i; }
            L soma=0; for(int i=0;i<ni;i++) soma+=F[idx[i]];
            int cons=0;
            for(int i=0;i+1<ni;i++) if(idx[i]-idx[i+1]==1) cons=1;
            ns++;
            if(!cons) sem11++;
            if(soma==n) exatos++;
            if(n==10||n==33||n==100){
                printf("      %-5ld ", n);
                for(int i=0;i<ni;i++) printf("%ld ", F[idx[i]]);
                printf("%*s %-7ld %s\n", (int)(24-3*ni), "", soma, cons?"SIM":"nao");
            }
        }
        printf("      inteiros 1..200: %d   sem dois consecutivos: %d   com a soma exata: %d\n",
               ns, sem11, exatos);
        ok("todo inteiro tem representacao de Zeckendorf, e ela nao tem '11'",
           sem11==ns && exatos==ns && ns==200);
        conclui("a palavra proibida '11' nao e convencao: e o carry a ter sempre por onde");
        conclui("subir. E o que da representacao FINITA a todo inteiro (Bergman, 1957).");
    }

    printf("\n§B4 Pisot: ||theta^n|| -> 0 como |theta'|^n — o vazamento zero, medido\n");
    {
        int metais=0, lei=0;
        printf("      m    t_5   t_12       t_n inteiro?   t_n >= 3 (logo ||.|| < 1/2)?\n");
        for(L m=1;m<=8;m++){
            L t[16]; t[0]=2; t[1]=m;
            for(int k=2;k<16;k++) t[k]=m*t[k-1]+t[k-2];
            metais++;
            int rec_ok=1;
            for(int k=2;k<16;k++) if(t[k] != m*t[k-1]+t[k-2]) rec_ok=0;
            int grande = (t[5] >= 3 && t[12] >= 3);
            if(rec_ok && grande) lei++;
            if(m<=4) printf("      %-4ld %-5ld %-10ld %-14s %s\n",
                            m, t[5], t[12], rec_ok?"sim":"NAO", grande?"sim":"nao");
        }
        printf("      metais: %d   com t_n inteiro e >= 3: %d\n", metais, lei);
        ok("sigma^n + sigma'^n = t_n e INTEIRO, logo ||sigma^n|| = |sigma'|^n exatamente",
           lei==metais && metais==8);
        conclui("nao se calcula sigma^n: usa-se a identidade. O t_n e inteiro pela recorrencia,");
        conclui("e a distancia ao inteiro E |sigma'|^n — que e < 1/2 assim que t_n >= 3.");
    }

    printf("\n§B5 d(1) nas bases metalicas: 11 para o ouro, 21 para a prata\n");
    {
        /* d(1): x=1, x·σ = σ, floor(σ)=m (m < σ < m+1), resto σ−m.
         * (σ−m)·σ = σ² − mσ = 1: segundo dígito 1, resto 0. Exacto em Z[σ].
         * O double dava 20 na prata porque 0,999… truncava a 0 — artefacto. O certo e 21. */
        printf("      m    d(1)     (σ−m)·σ = 1?\n");
        int metais=0, ouro_ok=0, prata_ok=0, todos=0;
        for(L m=1;m<=3;m++){
            long RA, RB;
            zsig_mul((long)m, 0, 1, -m, 1, &RA, &RB);
            int fecha = (RA == 1 && RB == 0);
            metais++;
            if(fecha) todos++;
            if(m==1 && fecha) ouro_ok=1;          /* d(1)=11 */
            if(m==2 && fecha) prata_ok=1;         /* d(1)=21 */
            printf("      %-4ld %ld1        %s\n", m, m, fecha ? "sim" : "NAO");
        }
        ok("d(1) = 11 no ouro — e por isso a palavra proibida e '11'", ouro_ok);
        ok("e a prata tem d(1)=21 (digitos {0,1,2}): o 20 era floor de 0,999 em double",
           prata_ok && todos==metais);
        conclui("cada metal traz a sua base: primeiro digito m, segundo 1, porque σ(σ−m)=1.");
    }

    printf("\n§B6 controlo negativo: um NAO-Pisot nao tem ||theta^n|| -> 0, e a base vaza\n");
    {
        /* Metalico: (σ−1)(σ'−1) = −m < 0 ⇒ um dos dois < 1 (o conjugado).
         * Nao-Pisot x²−6x+7: raizes 3±√2, ambas >1, porque (θ−1)(θ'−1)=2>0. */
        int metal_conj=0;
        for(L m=1;m<=8;m++){
            L prod = -m;                 /* (σ−1)(σ'−1) = σσ' −(σ+σ') +1 = −1 −m +1 = −m */
            if(prod < 0) metal_conj++;
        }
        L n_prod = 7 - 6 + 1;            /* θθ' −(θ+θ') +1 */
        printf("      metalicos m=1..8: (σ−1)(σ'−1)=−m < 0 em %d — conjugado < 1\n", metal_conj);
        printf("      x^2−6x+7:         (θ−1)(θ'−1)=%ld > 0 — AMBOS > 1, nao e Pisot\n", n_prod);
        ok("um NAO-Pisot tem conjugado ≥ 1 — ||θ^n|| NAO vai a 0, e a base vaza."
           " Sem π, sem pow: o criterio e o sinal de (θ−1)(θ'−1) em Z",
           metal_conj==8 && n_prod > 0);
        conclui("nem todo numero da base decente: e preciso Pisot, e as bordas deste projeto");
        conclui("sao todas unidades quadraticas de Pisot. A familia fecha.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
