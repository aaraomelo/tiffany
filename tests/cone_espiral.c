/* cone_espiral.c — O CONE E A ESPIRAL: Σ∘Π = Id, mas Π∘Σ NÃO — e é aí que está a informação.
 *
 * Π e Σ em inteiros (Euclides + convergentes). Sem math.h — a régua sai inteira.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/cone_espiral.c -o cone_espiral
 */
#include <stdio.h>
#include <string.h>
#include "aritmetica.h"
#include "algebra.h"
#include "unidade.h"

static int Pi_exato(long p, long q, int *a, int n){
    int k = 0;
    while(q != 0 && k < n){
        long f = p / q;
        a[k++] = (int)f;
        long r = p - f*q;
        p = q; q = r;
    }
    return k;
}

static void Sigma_exato(const int *a, int n, long *p, long *q){
    long P = 1, Q = 0;
    for(int k = n-1; k >= 0; k--){ long np = a[k]*P + Q; Q = P; P = np; }
    *p = P; *q = Q;
}

static int canonica(const int *a, int n){
    for(int i = 1; i < n; i++) if(a[i] < 1) return 0;
    if(n > 1 && a[n-1] == 1) return 0;
    return 1;
}

int main(void){
printf("\n=== O CONE E A ESPIRAL: Σ∘Π = Id, mas Π∘Σ NÃO ============================\n");

printf("\n§E1  Σ∘Π = Id: a espiral recompõe o real — em Z, resíduo 0 INTEIRO.\n\n");
{
    printf("      p/q        Π(p/q)              Σ∘Π = p/q reduzido?\n");
    long racionais = 0, fecha_exacto = 0;
    for(unsigned long q = 1; q <= 40; q++) for(unsigned long p = 1; p <= 40; p++){
        unsigned long a_[40], pc[40], qc[40];
        int n = nt_fc(p, q, a_, 40);
        if(n <= 0) continue;
        int m = nt_convergentes(a_, n, pc, qc);
        if(m <= 0) continue;
        unsigned long g = (unsigned long)al_mdc((long)p, (long)q);
        racionais++;
        if(pc[m-1] == p/g && qc[m-1] == q/g) fecha_exacto++;
        if(p <= 3 && q <= 5){
            printf("      %lu/%lu     [", p, q);
            for(int k = 0; k < n && k < 5; k++) printf("%lu%s", a_[k], k<n-1&&k<4?";":"");
            printf("]           %s\n", (pc[m-1]==p/g && qc[m-1]==q/g) ? "sim" : "NAO");
        }
    }
    printf("\n      %ld racionais p/q ≤ 40: fecho exacto em %ld\n\n", racionais, fecha_exacto);
    ok("Σ∘Π = Id — num RACIONAL a palavra de Euclides (nt_fc) recomposta pelos"
       " convergentes (nt_convergentes) da' p/q REDUZIDO em Z, nos 1600 pares com p,q ate' 40."
       " Residuo 0 INTEIRO, sem virgula",
       racionais == 1600 && fecha_exacto == racionais);
}

printf("\n§E2  Π∘Σ ≠ Id: sequências que a espiral aceita e o cone NUNCA produz.\n\n");
{
    printf("      sequência dada          p/q         Π(Σ)         canónica?  voltou?\n");
    struct { int a[6], n; } casos[] = {
        {{3,7,15,1},        4},
        {{3,7,15,0,1},      5},
        {{3,7,16},          3},
        {{3,7,15,1,1},      5},
        {{1,1,1,1,1},       5},
        {{2,0,3},           3},
    };
    int naoVolta = 0, naoCanon = 0;
    for(int i = 0; i < 6; i++){
        long P, Q; Sigma_exato(casos[i].a, casos[i].n, &P, &Q);
        int b[24], m = Pi_exato(P, Q, b, 24);
        int igual = (m == casos[i].n);
        if(igual) for(int k = 0; k < m; k++) if(b[k] != casos[i].a[k]) igual = 0;
        int can = canonica(casos[i].a, casos[i].n);
        if(!can) naoCanon++;
        if(!igual) naoVolta++;
        printf("      [");
        for(int k = 0; k < casos[i].n; k++) printf("%d%s", casos[i].a[k], k<casos[i].n-1?";":"");
        printf("]     %ld/%-6ld [", P, Q);
        for(int k = 0; k < m && k < 5; k++) printf("%d%s", b[k], k<m-1&&k<4?";":"");
        printf("]   %-10s %s\n", can ? "sim" : "NAO", igual ? "sim" : "NAO");
    }
    printf("\n      %d de 6 não voltaram; %d não eram canónicas\n\n", naoVolta, naoCanon);
    ok("há sequências que Σ aceita e Π nunca produz — Π∘Σ NÃO é a identidade", naoVolta > 0);
    ok("e são exatamente as não-canónicas: zero no meio, ou terminar em 1", naoVolta == naoCanon);
}

printf("\n§E3  A AMBIGUIDADE dos racionais: exatamente DUAS representações, contadas.\n\n");
{
    int comDupla = 0, total = 0, mau = 0;
    for(int q = 2; q <= 40; q++)
    for(int p = 1; p < q; p++){
        if(p % 2 == 0 && q % 2 == 0) continue;
        int a[24], n = Pi_exato(p, q, a, 24);
        if(n < 2) continue;
        total++;
        int b[25]; memcpy(b, a, (size_t)n*sizeof(int));
        b[n-1] -= 1; b[n] = 1;
        int m = n + 1;
        if(b[n-1] < 1 && n > 1) continue;
        comDupla++;
        long P2, Q2; Sigma_exato(b, m, &P2, &Q2);
        if((long)p*Q2 != P2*(long)q) mau++;
    }
    printf("      %d racionais varridos, %d com forma dupla, %d discordâncias\n\n",
           total, comDupla, mau);
    ok("a forma alternativa dá EXATAMENTE o mesmo número — Σ não é injetiva",
       mau == 0 && comDupla > 100);
}

printf("\n§E4  Logo o par é uma RETRAÇÃO, e não uma involução.\n\n");
{
    int mau = 0;
    for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
        long a2 = a + 2*b, b2 = -b;
        long a3 = a2 + 2*b2, b3 = -b2;
        if(a3 != a || b3 != b) mau++;
    }
    ok("na involução ν, o MESMO operador duas vezes devolve", mau == 0);
    {
        /* convergente de √3: 71/41 — Σ∘Π fecha exacto em Z */
        long p = 71, q = 41;
        int a[20], n = Pi_exato(p, q, a, 20);
        long P, Q; Sigma_exato(a, n, &P, &Q);
        int ida = (P == p && Q == q);
        int c[4] = {1,7,0,2};
        long P2, Q2; Sigma_exato(c, 4, &P2, &Q2);
        int d[20], k2 = Pi_exato(P2, Q2, d, 20);
        int outra = (k2 == 4 && d[0]==c[0] && d[1]==c[1] && d[2]==c[2] && d[3]==c[3]);
        printf("      Σ∘Π em 71/41 (conv. √3): %s\n", ida ? "sim" : "nao");
        printf("      Π∘Σ em [1;7,0,2]:        %s\n\n", outra ? "sim" : "NAO");
        ok("Σ∘Π fecha e Π∘Σ não — é retração, e a assimetria é o cone", ida && !outra);
    }
}

printf("\n§E5  E o que o cone perde por passo é o que a régua ganha.\n\n");
{
    /* Convergentes consecutivos: |p_n/q_n - p_{n+1}/q_{n+1}| = 1/(q_n q_{n+1}) */
    int a[20]; for(int i = 0; i < 20; i++) a[i] = 1; /* φ = [1;1,1,…] */
    long p0=1,q0=0,p1=1,q1=1;
    int mau = 0, casos = 0;
    printf("      k    convergente    |p_n q_{n+1} - p_{n+1} q_n| (=1)\n");
    for(int k = 1; k <= 8; k++){
        long pn = a[k]*p1+p0, qn = a[k]*q1+q0;
        long det = p1*qn - pn*q1;
        if(det < 0) det = -det;
        if(det != 1) mau++;
        casos++;
        printf("      %-4d %ld/%-10ld %ld\n", k+1, pn, qn, det);
        p0=p1; q0=q1; p1=pn; q1=qn;
    }
    printf("\n");
    ok("cada passo do cone paga-se: convergentes consecutivos distam 1/(q_n q_{n+1})"
       " — |p_n q_{n+1} - p_{n+1} q_n| = 1 em Z, o teorema de Hurwitz escrito sem virgula",
       mau == 0 && casos == 8);
}

printf("\n=== FECHO ==================================================================\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
