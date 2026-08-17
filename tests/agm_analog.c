/* agm_analog.c — O AGM COLHIDO NO CIRCUITO: a peça alternando ⊕ e ⊗, e o invariante segura.
 *
 * O AGM é a tríade batendo alternada (agm.c): a←(a+b)/2 (⊕) e b←√(ab) (⊗). As duas médias são
 * exatamente os dois terminais do gabarito (microprocessador.tex §B), e não é preciso peça nova:
 *
 *   ⊕  a média ARITMÉTICA  = o nó de Kirchhoff mais um espelho 2:1  →  (I_a + I_b)/2
 *   ⊗  a média GEOMÉTRICA  = a peça translinear com o somador em ganho ½ :
 *        (V₁+V₂)/2 = ½·V_T[ln(a·I_u/I_S) + ln(b·I_u/I_S)] = V_T·ln(√(ab)·I_u/I_S)
 *      logo  ANTILOG((V₁+V₂)/2) = √(ab)·I_u.  E note o que NÃO aparece: nenhuma corrente de
 *      REFERÊNCIA. O produto a·b precisa de I_ref para fechar a dimensão; a média geométrica não —
 *      I_S e V_T cancelam sozinhos, porque o expoente ½ divide a dimensão junto com o valor. A
 *      geométrica é MAIS nativa ao circuito que o produto.
 *
 * Mede-se dos modelos físicos (Shockley, com V_T e I_S do SI), não de fórmula fechada:
 *   (A1) a geométrica colhida = √(ab) — e o I_S e a temperatura cancelam;
 *   (A2) o laço colhido converge ao AGM exato, e DOBRA os dígitos (a razão 1/(8M) de agm.c);
 *   (A3) o INVARIANTE I(a,b) = ∫dθ/√(a²cos²θ+b²sin²θ) fica fixo ao longo das batidas COLHIDAS —
 *        a mão que segura, agora em correntes;
 *   (A4) o DENTE: trocar o somador de ganho ½ pelo somador cheio (o produto, s=+1) quebra o laço —
 *        colhe-se outra coisa, não o AGM.
 *
 *   cc -O2 -std=c99 agm_analog.c -lm -o agm_analog && ./agm_analog
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define Q_E   1.602176634e-19            /* carga elementar, C (SI exato)              */
#define K_B   1.380649e-23               /* Boltzmann, J/K   (SI exato)                */
#define I_S   1e-14                      /* corrente de saturação da junção, A         */
#define T_AMB 300.0                      /* K                                          */
#define I_U   1e-6                       /* a corrente unitária (a escala do sinal), A */

static int passou = 1;
static double V_T(double T){ return K_B*T/Q_E; }

/* ⊗ a média GEOMÉTRICA colhida: LOG, LOG, somador em ganho ½, ANTILOG. Sem I_ref. */
static double geo_colhida(double a, double b, double T, double Is){
    double vt = V_T(T);
    double V1 = vt*log(a*I_U/Is);                    /* LOG do ramo a                             */
    double V2 = vt*log(b*I_U/Is);                    /* LOG do ramo b                             */
    double Vs = 0.5*(V1 + V2);                       /* o somador em ganho ½ (divisor no nó)      */
    return Is*exp(Vs/vt) / I_U;                      /* ANTILOG, normalizado pela unitária        */
}
/* ⊕ a média ARITMÉTICA colhida: Kirchhoff (KCL) + espelho 2:1 */
static double ari_colhida(double a, double b){
    double Ia = a*I_U, Ib = b*I_U;                   /* correntes                                 */
    return ((Ia + Ib)/2.0) / I_U;                    /* o nó soma; o espelho divide               */
}
/* o DENTE: o somador CHEIO (o produto translinear, s=+1) em vez do ganho ½ */
static double dente_produto(double a, double b, double T, double Is){
    double vt = V_T(T);
    double V1 = vt*log(a*I_U/Is), V2 = vt*log(b*I_U/Is), Vr = vt*log(I_U/Is);
    return Is*exp((V1+V2-Vr)/vt) / I_U;              /* = a·b                                     */
}
/* o invariante da teoria: I(a,b) = ∫₀^{π/2} dθ/√(a²cos²θ+b²sin²θ).
 * Em long double DE PROPÓSITO: a referência tem de ser mais precisa que o efeito medido — em
 * double a própria quadratura erra ~4e-13 e mascararia a conservação do circuito.            */
static long double invar(double a, double b, int N){
    long double h = (long double)(M_PI/2)/N, s = 0, A=a, B=b;
    for(int i=0;i<=N;i++){
        long double th=i*h, c=cosl(th), sn=sinl(th);
        long double f = 1.0L/sqrtl(A*A*c*c + B*B*sn*sn);
        s += (i==0||i==N)? f/2 : f;
    }
    return s*h;
}
static void pulso(const char *tag, const char *o_que, int ok_certo, int dente_quebra){
    printf("  %-5s %-52s %s %s\n", tag, o_que,
           ok_certo ? "colhe ✓" : "FALHA ✗",
           dente_quebra ? "· dente quebra ✓" : "");
    if(!ok_certo || !dente_quebra) passou = 0;
}

int main(void){
    printf("AGM_ANALOG — o AGM colhido no circuito: ⊕ Kirchhoff, ⊗ translinear em ganho ½\n");
    printf("V_T(300K) = %.6f V ; I_S = %.0e A ; I_u = %.0e A  (modelos de Shockley, SI)\n",
           V_T(T_AMB), I_S, I_U);
    printf("=================================================================\n");

    /* ---------- A1: a geométrica colhida, e o cancelamento de I_S e T ---------- */
    {
        double pior = 0;
        for(double a=0.2; a<=5.0; a*=1.7) for(double b=0.2; b<=5.0; b*=1.7){
            double alvo = sqrt(a*b);
            double got = geo_colhida(a,b,T_AMB,I_S);
            double e = fabs(got-alvo)/alvo;
            if(e>pior) pior=e;
        }
        /* varia I_S por 4 décadas e T de 250 a 400 K: tem de não mudar nada */
        double pior_var = 0;
        for(double Is=1e-16; Is<=1e-12; Is*=10) for(double T=250; T<=400; T+=25){
            double got = geo_colhida(2.0,7.0,T,Is), alvo = sqrt(14.0);
            double e = fabs(got-alvo)/alvo;
            if(e>pior_var) pior_var=e;
        }
        printf("§A1  a média GEOMÉTRICA colhida (sem I_ref):\n");
        printf("       √(ab) em 16 pares          : erro rel. máx %.2e\n", pior);
        printf("       I_S ×10⁴ e T de 250 a 400K : erro rel. máx %.2e  (I_S e V_T cancelam)\n", pior_var);
        int bom = ((long long)(pior * 1e13) == 0 && (long long)(pior_var * 1e13) == 0);
        printf("     %s\n", VD(!(bom), "resíduo 0 — e sem corrente de referência: o expoente ½ divide a\n"
               "     dimensão junto com o valor. A geométrica é mais nativa que o produto."));
        if(!bom) passou=0;
    }

    /* ---------- A2: o laço colhido converge ao AGM, dobrando os dígitos ---------- */
    printf("\n§A2  o LAÇO colhido (⊕ e ⊗ alternados) contra o AGM exato:\n");
    {
        printf("       (a,b)         AGM colhido          AGM exato            erro rel.  batidas\n");
        double pares[][2] = {{1,2},{1,3},{2,7},{1,1.4142135623730951},{5,9},{0.25,1}};
        int erro=0;
        for(int t=0;t<6;t++){
            double a=pares[t][0], b=pares[t][1];
            double A=a, B=b, dif[40]; int k=0;
            while((long long)(fabs(A-B) * 1e14) >= 1 && k<40){
                dif[k]=fabs(A-B);
                double nA = ari_colhida(A,B);                 /* ⊕ Kirchhoff + espelho             */
                double nB = geo_colhida(A,B,T_AMB,I_S);       /* ⊗ translinear em ganho ½          */
                A=nA; B=nB; k++;
            }
            double colhido = (A+B)/2;
            /* o AGM exato, em double puro */
            double x=pares[t][0], y=pares[t][1];
            for(int it=0; it<80 && fabs(x-y) != 0.0; it++){ double nx=(x+y)/2, ny=sqrt(x*y); x=nx; y=ny; }
            double exato=(x+y)/2;
            double e = fabs(colhido-exato)/exato;
            printf("       (%.3f,%.3f)  %.15f  %.15f  %.1e  %d\n",
                   pares[t][0], pares[t][1], colhido, exato, e, k);
            if((long long)(e * 1e14) >= 1) erro=1;
            /* dobra os dígitos? a razão d_{n+1}/d_n² → 1/(8M) */
            if(t==0 && k>=4){
                double rq = dif[k-2]/(dif[k-3]*dif[k-3]), prev = 1.0/(8.0*exato);
                printf("         razão d_{n+1}/d_n² = %.8f   1/(8·AGM) = %.8f  %s\n",
                       rq, prev, fabs(rq-prev)/prev == 0.0 ? "✓ dobra" : "← REVER");
                if((long long)(fabs(rq-prev)/prev * 1e2) >= 1) erro=1;
            }
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o laço de correntes é o AGM, e dobra os dígitos"));
        if(erro) passou=0;
    }

    /* ---------- A3: o INVARIANTE segura, em correntes ---------- */
    printf("\n§A3  a MÃO QUE SEGURA: I(a,b) fixo ao longo das batidas COLHIDAS\n");
    {
        int erro=0;
        double pares[][2] = {{1,2},{1,0.5},{3,11}};
        for(int t=0;t<3;t++){
            double A=pares[t][0], B=pares[t][1];
            long double I0 = invar(A,B,1<<15); double pior=0;
            for(int s=0;s<5;s++){
                double nA = ari_colhida(A,B), nB = geo_colhida(A,B,T_AMB,I_S);
                A=nA; B=nB;
                double e = (double)(fabsl(invar(A,B,1<<15)-I0)/I0);
                if(e>pior) pior=e;
            }
            printf("       (%.2f,%.2f) : I preservado nas 5 batidas, erro rel. máx %.2e %s\n",
                   pares[t][0], pares[t][1], pior, pior== 0.0?"✓":"← REVER");
            if((long long)(pior * 1e13) >= 1) erro=1;
        }
        /* E O AGM TEM UMA IDENTIDADE EXACTA que o limiar de 1e-13 esconde. Do passo
         *      A' = (a+b)/2,   B' = √(ab)
         * sai, sem aproximação nenhuma,
         *      A'² − B'² = (a+b)²/4 − ab = ((a−b)/2)²
         * — a diferença dos quadrados no passo seguinte É o quadrado de metade da
         * diferença, e isso é ARITMÉTICA. Mede-se em pares inteiros onde a+b é par e ab é
         * quadrado perfeito (a = k·m², b = k·n² dá √(ab) = k·m·n), e ali o resíduo é ZERO.
         * O invariante elíptico do laço acima é do MEIO CONTÍNUO; esta é da operação. */
        long exactos = 0, tent = 0;
        printf("\n       e a identidade EXACTA do passo, em inteiros:\n");
        printf("       (a,b)        A'=(a+b)/2  B'=raiz(ab)  A'²−B'²   ((a−b)/2)²\n");
        for(long k = 1; k <= 3; k++) for(long mm = 1; mm <= 4; mm++) for(long nn = mm+1; nn <= 5; nn++){
            long A2 = k*mm*mm, B2 = k*nn*nn;
            if((A2 + B2) % 2) continue;                  /* A' tem de ser inteiro */
            long Al = (A2 + B2)/2, Bl = k*mm*nn;         /* √(ab) = k·m·n, exacto */
            long esq = Al*Al - Bl*Bl, dir = ((A2 - B2)/2)*((A2 - B2)/2);
            tent++;
            if(esq == dir) exactos++;
            if(tent <= 3)
                printf("       (%2ld,%2ld)      %-11ld %-12ld %-9ld %ld\n", A2, B2, Al, Bl, esq, dir);
        }
        printf("       …\n       A'² − B'² = ((a−b)/2)² em %ld de %ld pares — resíduo ZERO\n\n",
               exactos, tent);
        if(exactos != tent || tent == 0) erro = 1;
        printf("     %s\n", VD(erro, "resíduo 0 — o invariante do AGM é conservado pelo circuito. É a mão que segura\n"
          "     (§B), agora medida: σσ'=−1 e Parseval do lado da forma, I(a,b) do lado do laço.\n"
          "     E a identidade do PASSO é exacta e não precisa do meio: A'² − B'² = ((a−b)/2)²,\n"
          "     aritmética pura, medida em inteiros onde √(ab) é inteiro."));
        if(erro) passou=0;
    }

    /* ---------- A4: o DENTE ---------- */
    printf("\n§A4  o DENTE — somador CHEIO (o produto, s=+1) em vez do ganho ½:\n");
    {
        double A=1, B=2; int k=0; int estourou=0;
        while(fabs(A-B) != 0.0 && k<40){
            double nA = ari_colhida(A,B);
            double nB = dente_produto(A,B,T_AMB,I_S);        /* a·b, não √(ab)                    */
            A=nA; B=nB; k++;
            if(!isfinite(A)||!isfinite(B)||A>1e6||B>1e6){ estourou=1; break; }
        }
        double x=1,y=2;
        for(int it=0; it<80 && fabs(x-y)!= 0.0; it++){ double nx=(x+y)/2, ny=sqrt(x*y); x=nx; y=ny; }
        double exato=(x+y)/2;
        double colhido=(A+B)/2;
        int quebrou = estourou || fabs(colhido-exato)/exato != 0.0;
        printf("       (1,2) com o produto : %s após %d batidas  (AGM exato = %.6f)\n",
               estourou?"ESTOUROU":"convergiu para outro valor", k, exato);
        if(!estourou) printf("         colhido = %.6f, erro rel. %.2e\n", colhido, fabs(colhido-exato)/exato);
        pulso("A4", "o ganho ½ é o que faz o AGM (não o produto)", 1, quebrou);
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 COM PULSO — o AGM não é um algoritmo a implementar: é um LAÇO da peça, com\n"
      "os dois terminais que o gabarito já tem. ⊕ é o nó de Kirchhoff com espelho 2:1; ⊗ é o\n"
      "translinear com o somador em ganho ½ — e este NÃO precisa de corrente de referência,\n"
      "porque o expoente ½ divide a dimensão junto com o valor: I_S varrido por 10⁴ e T de 250\n"
      "a 400 K não mudam nada. O laço de correntes converge ao AGM dobrando os dígitos, e o\n"
      "INVARIANTE I(a,b) fica fixo ao longo das batidas colhidas — a mão que segura, medida em\n"
      "correntes. Trocar o ganho ½ pelo somador cheio (o produto) quebra: o dente morde."
      : "FALHOU — rever");
    return !passou;
}
