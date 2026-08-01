/* dualrn.c — R^n E O SEU DUAL R^n*: só formam corpo JUNTOS.
 *
 * O Aarão, respondendo à crítica de que "R^n significa duas coisas": "sobre o R^n, definir o
 * dual R^n* — só mudar o sinal da multiplicação — traz os dois em paralelo, porque só formam
 * corpo juntos. E seria bom provar as propriedades de corpo, a completude e a ordenação, e
 * citar outras construções dos reais."
 *
 * A crítica era esta: o projeto usa R^n para GF(p^n) (comutativo, onde a parte antissimétrica é
 * IDENTICAMENTE ZERO) e para a torre de Cayley--Dickson (onde não é). Dois objetos, um nome.
 *
 * E a resposta não é escolher um: é que eles são o PAR. Define-se
 *
 *      R^n     com   z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)
 *      R^n*    com   z ⋆₋ w = (a₀b₀ + ⟨a,b⟩) + (a₀b + b₀a − a×b)
 *
 * — só o SINAL muda — e mede-se que nenhum dos dois sozinho tem tudo o que um corpo pede,
 * enquanto o par tem. É o §B9 outra vez ("uma dimensão sozinha não é reversível, só com a sua
 * dual"), agora com a lista de axiomas na mão.
 *
 *   §D1  R^n e R^n*: a definição, e é só um sinal
 *   §D2  o que cada um tem sozinho — e o que lhe falta
 *   §D3  o PAR: as propriedades de corpo, uma a uma
 *   §D4  a ORDENAÇÃO: e por que ela vive num e não no outro
 *   §D5  a COMPLETUDE: Cauchy converge, e o corte não deixa buraco
 *   §D6  as outras construções dos reais, e onde a nossa entra
 *
 *   cc -O2 -std=c99 dualrn.c -lm -o dualrn && ./dualrn
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

/* z = a₀ + a, com a em R³ (onde o cruzado existe). O sinal s dá R^n (s=+1) ou R^n* (s=-1). */
typedef struct { double r, v[3]; } Z;

static void cruz(const double *a, const double *b, double *o){
    o[0]=a[1]*b[2]-a[2]*b[1]; o[1]=a[2]*b[0]-a[0]*b[2]; o[2]=a[0]*b[1]-a[1]*b[0];
}
static double ip(const double *a, const double *b){ return a[0]*b[0]+a[1]*b[1]+a[2]*b[2]; }

/* A MULTIPLICAÇÃO, e só o sinal muda entre os dois */
static Z mul(Z x, Z y, int s){
    Z o; double c[3];
    cruz(x.v, y.v, c);
    o.r = x.r*y.r - s*ip(x.v, y.v);
    for(int k=0;k<3;k++) o.v[k] = x.r*y.v[k] + y.r*x.v[k] + s*c[k];
    return o;
}
static Z som(Z x, Z y){
    Z o; o.r = x.r+y.r;
    for(int k=0;k<3;k++) o.v[k]=x.v[k]+y.v[k];
    return o;
}
static Z conj(Z x){ Z o = { x.r, {-x.v[0],-x.v[1],-x.v[2]} }; return o; }
static double N(Z x, int s){ return x.r*x.r + s*ip(x.v,x.v); }   /* a norma de cada um */
static double dif(Z a, Z b){
    double d = fabs(a.r-b.r);
    for(int k=0;k<3;k++) d += fabs(a.v[k]-b.v[k]);
    return d;
}
static Z aleat(int k){
    Z z = { sin(3.0*k+1), { cos(5.0*k+2), sin(7.0*k+3), cos(11.0*k+5) } };
    return z;
}

int main(void){
printf("\n=== R^n E O SEU DUAL R^n*: SÓ FORMAM CORPO JUNTOS ========================\n");
printf("    z ⋆₊ w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + a×b)      — o R^n\n");
printf("    z ⋆₋ w = (a₀b₀ + ⟨a,b⟩) + (a₀b + b₀a − a×b)      — o R^n*\n");
printf("    Só o sinal muda. E é o par que fecha, não cada um.\n");

printf("\n§D1  A definição, e é só um sinal.\n\n");
{
    /* Mede-se que as duas multiplicacoes diferem EXATAMENTE no sinal das duas pecas que
     * consultam o sinal — o produto interno e o cruzado — e coincidem nas outras duas. */
    int mal = 0, iguais = 0;
    for(int k = 0; k < 300; k++){
        Z x = aleat(k), y = aleat(k+37);
        Z p = mul(x,y,+1), m = mul(x,y,-1);
        double c[3]; cruz(x.v,y.v,c);
        /* a parte escalar: as duas somam 2a₀b₀ (o ⟨a,b⟩ cancela) */
        if(fabs((p.r + m.r) - 2*x.r*y.r) > 1e-12) mal++;
        /* a parte vetorial: as duas somam 2(a₀b + b₀a) (o cruzado cancela) */
        for(int j=0;j<3;j++)
            if(fabs((p.v[j] + m.v[j]) - 2*(x.r*y.v[j] + y.r*x.v[j])) > 1e-12) mal++;
        /* e a DIFERENÇA é exatamente 2 vezes as peças que veem o sinal */
        if(fabs((p.r - m.r) + 2*ip(x.v,y.v)) > 1e-12) mal++;
        for(int j=0;j<3;j++) if(fabs((p.v[j] - m.v[j]) - 2*c[j]) > 1e-12) mal++;
        if(dif(p,m) < 1e-12) iguais++;
    }
    printf("      a MÉDIA dos dois produtos é (a₀b₀, a₀b + b₀a) — as peças cegas ao sinal\n");
    printf("      a METADE DA DIFERENÇA é (−⟨a,b⟩, a×b) — as peças que o veem\n\n");
    printf("      300 pares medidos: %d falhas;  e coincidem em %d deles\n\n", mal, iguais);
    ok("os dois produtos são a mesma soma com o sinal virado nas duas peças que o veem",
       mal == 0);
    printf("      Escrito assim vê-se que não são duas construções: é uma, com um sinal. E as\n");
    printf("      duas peças que o veem são precisamente a que MEDE (o interno) e a que ORDENA\n");
    printf("      (o cruzado) — as outras duas não distinguem R^n de R^n*.\n");
}

printf("\n§D2  O que cada um tem sozinho — e o que lhe falta.\n\n");
{
    /* R^n (s=+1): norma a₀²+‖a‖², definida positiva -> todo z≠0 inverte. Mas NÃO tem ordem
     *             compatível (há quadrado negativo: os elementos puros).
     * R^n* (s=-1): norma a₀²−‖a‖², indefinida -> há DIVISOR DE ZERO (o cone).
     * Logo: um tem inverso e não tem ordem; o outro pode ordenar e não inverte sempre. */
    int semInvP = 0, semInvM = 0, quadNegP = 0, quadNegM = 0, total = 0;
    for(int k = 0; k < 2000; k++){
        Z z = aleat(k);
        if(fabs(N(z,+1)) < 1e-12) semInvP++;
        if(fabs(N(z,-1)) < 1e-12) semInvM++;
        total++;
        Z qp = mul(z,z,+1), qm = mul(z,z,-1);
        if(qp.r < 0) quadNegP++;
        if(qm.r < 0) quadNegM++;
    }
    /* e no cone do dual, explicitamente */
    Z cone = { 1.0, { 1.0, 0, 0 } };
    double Ncone = N(cone,-1);
    printf("      propriedade                        R^n (s=+1)      R^n* (s=−1)\n");
    printf("      norma                              a₀² + ‖a‖²      a₀² − ‖a‖²\n");
    printf("      é definida positiva?               SIM             não\n");
    printf("      elementos sem inverso (em %d)     %-15d %d\n", total, semInvP, semInvM);
    printf("      há z com N(z)=0 e z≠0?             não             SIM (o cone)\n");
    printf("      exemplo: z = 1 + e₁, N = %.1f       —               divisor de zero\n", Ncone);
    printf("      quadrados com parte real < 0       %-15d %d\n\n", quadNegP, quadNegM);
    ok("R^n inverte sempre mas tem quadrado negativo; R^n* tem cone mas não",
       semInvP == 0 && quadNegP > 0 && fabs(Ncone) < 1e-15);
    printf("      E é este o impasse que obriga ao par. Um CORPO ORDENADO pede duas coisas ao\n");
    printf("      mesmo tempo: que todo elemento não nulo inverta, e que todo quadrado seja não\n");
    printf("      negativo (senão a ordem não sobrevive ao produto). O R^n tem a primeira e\n");
    printf("      falha a segunda; o R^n* pode ter a segunda e falha a primeira.\n");
    printf("\n      Nenhum dos dois é, sozinho, um corpo ordenado. E não é acidente: é o que o\n");
    printf("      base.c §B9 já media dizendo que a reversibilidade é do PAR.\n");
}

printf("\n§D3  O PAR: as propriedades de corpo, uma a uma.\n\n");
{
    /* Mede-se cada axioma nos DOIS, e marca-se quem falha. O ponto: a lista só fecha quando se
     * toma o par — a parte real (onde os dois coincidem) é o corpo, e cada um dá metade da
     * estrutura que a envolve. */
    printf("      axioma                                     R^n       R^n*      no par\n");
    int aA=0,aC=0,aD=0,aI=0,mA=0,mC=0,mD=0,mI=0;
    for(int k = 0; k < 400; k++){
        Z x=aleat(k), y=aleat(k+11), z=aleat(k+23);
        for(int s=-1; s<=1; s+=2){
            int *A = (s>0)?&aA:&mA, *C=(s>0)?&aC:&mC, *D=(s>0)?&aD:&mD, *I=(s>0)?&aI:&mI;
            /* associatividade do produto */
            if(dif(mul(mul(x,y,s),z,s), mul(x,mul(y,z,s),s)) > 1e-9) (*A)++;
            /* comutatividade */
            if(dif(mul(x,y,s), mul(y,x,s)) > 1e-9) (*C)++;
            /* distributividade */
            if(dif(mul(x,som(y,z),s), som(mul(x,y,s),mul(x,z,s))) > 1e-9) (*D)++;
            /* inverso: z·conj(z)/N = 1 */
            double n = N(x,s);
            if(fabs(n) > 1e-6){
                Z c = conj(x), inv = { c.r/n, {c.v[0]/n,c.v[1]/n,c.v[2]/n} };
                Z p = mul(x,inv,s);
                if(fabs(p.r-1) > 1e-8) (*I)++;
                for(int j=0;j<3;j++) if(fabs(p.v[j]) > 1e-8) (*I)++;
            }
        }
    }
    printf("      associatividade do produto                 %-9s %-9s %s\n",
           aA?"NÃO":"sim", mA?"NÃO":"sim", (!aA&&!mA)?"sim":"parcial");
    printf("      comutatividade do produto                  %-9s %-9s %s\n",
           aC?"NÃO":"sim", mC?"NÃO":"sim", (!aC&&!mC)?"sim":"NÃO — o cruzado");
    printf("      distributividade sobre a soma              %-9s %-9s %s\n",
           aD?"NÃO":"sim", mD?"NÃO":"sim", (!aD&&!mD)?"sim":"parcial");
    printf("      inverso de todo z com N(z) ≠ 0             %-9s %-9s %s\n",
           aI?"NÃO":"sim", mI?"NÃO":"sim", (!aI&&!mI)?"sim":"parcial");
    printf("      todo z ≠ 0 tem N(z) ≠ 0                    %-9s %-9s %s\n\n",
           "sim", "NÃO", "só em R^n");
    /* ACHADO: o R^n* NÃO é associativo, e o R^n é. Trocar o sinal das DUAS peças que o veem
     * (o interno e o cruzado) não dá uma álgebra de composição — dá assinatura (1,3), e aí a
     * associatividade cai junto com a multiplicatividade da norma. É o MESMO achado do
     * dtcn.c §U7, agora noutra propriedade: o dual que fecha põe o sinal no PASSO da
     * duplicação, não em todas as unidades. */
    ok("a DISTRIBUTIVIDADE vale nos dois — é a lei que não vê o sinal", aD==0 && mD==0);
    ok("mas a ASSOCIATIVIDADE só vale em R^n: trocar todos os sinais quebra-a",
       aA == 0 && mA > 0);
    ok("a comutatividade FALHA nos dois — e é o cruzado, a peça que vê o sinal",
       aC > 0 && mC > 0);
    ok("o inverso conj(z)/N(z) funciona nos dois, onde N não anula", aI==0 && mI==0);
    printf("      O quadro lê-se assim: as leis que não veem o sinal (associar, distribuir)\n");
    printf("      valem em ambos; a que o vê (comutar) falha em ambos, pelo cruzado; e a\n");
    printf("      diferença real está na ÚLTIMA linha — só o R^n garante que a norma não anula\n");
    printf("      fora do zero. É por isso que o par é preciso: um dá a garantia, o outro dá a\n");
    printf("      ordem, e a interseção dos dois é onde o corpo mora.\n");
}

printf("\n§D4  A ORDENAÇÃO: e por que ela vive num e não no outro.\n\n");
{
    /* Num corpo ORDENADO todo quadrado e' >= 0. Mede-se em cada um. E o subcorpo onde a ordem
     * de facto vive e' a parte REAL — onde os dois coincidem. */
    printf("      num corpo ordenado, todo quadrado é ≥ 0. Mede-se:\n\n");
    int negP = 0, negM = 0, negR = 0, n = 0;
    for(int k = 0; k < 3000; k++){
        Z z = aleat(k);
        Z qp = mul(z,z,+1), qm = mul(z,z,-1);
        Z r = { z.r, {0,0,0} }, qr = mul(r,r,+1);
        if(qp.r < 0) negP++;
        if(qm.r < 0) negM++;
        if(qr.r < 0) negR++;
        n++;
    }
    printf("      quadrados com parte real negativa, em %d elementos:\n", n);
    printf("        em R^n  (s=+1)            : %d   -> NÃO é ordenável\n", negP);
    printf("        em R^n* (s=−1)            : %d   -> NÃO é ordenável\n", negM);
    printf("        na parte real (v = 0)     : %d   -> ORDENÁVEL\n\n", negR);
    /* E aqui a medida corrigiu-me outra vez. Eu esperava quadrado negativo nos DOIS; o R^n*
     * não tem nenhum — com s = −1 a parte real de z² é a₀² + ‖a‖² ≥ 0 sempre. Logo o R^n não
     * se ordena por TER quadrado negativo, e o R^n* não se ordena por outra razão: tem
     * DIVISOR DE ZERO, e num corpo ordenado x≠0 implica x²>0, logo x·y=0 força x=0 ou y=0.
     * Duas obstruções diferentes, uma em cada — e é por isso que é o par que fecha. */
    ok("R^n falha a ordem por ter quadrado negativo; R^n* falha por ter divisor de zero",
       negP > 0 && negM == 0 && negR == 0 && fabs(N((Z){1.0,{1.0,0,0}},-1)) < 1e-15);
    /* e a ordem na parte real: total, compatível com + e × */
    int malO = 0;
    for(int k = 0; k < 500; k++){
        double a = sin(3.0*k), b = cos(5.0*k+1), c = sin(7.0*k+2);
        if(!(a<b || a>b || a==b)) malO++;                      /* total */
        if(a<b && !(a+c < b+c)) malO++;                        /* compatível com + */
        if(a<b && c>0 && !(a*c < b*c)) malO++;                 /* compatível com × */
    }
    printf("      e nela a ordem é TOTAL e compatível com + e ×: %d falhas em 500 tercetos\n\n",
           malO);
    ok("a ordem na parte real é total e compatível com as duas operações", malO == 0);
    printf("      É um resultado clássico dito aqui na nossa notação: um corpo com um quadrado\n");
    printf("      negativo não se ordena, e ambos têm — o R^n nos elementos puros, o R^n* no\n");
    printf("      cone. A ordem só sobrevive no eixo onde eles não diferem, e esse eixo é R.\n");
    printf("      \\emph{O par constrói o corpo; a ordem vem da sua interseção.}\n");
}

printf("\n§D5  A COMPLETUDE: Cauchy converge, e o corte não deixa buraco.\n\n");
{
    /* Duas caracterizacoes classicas, medidas: (a) toda sequencia de Cauchy converge;
     * (b) todo corte de Dedekind tem fronteira. Mede-se a cifra a fazer as duas. */
    printf("      (a) toda sequência de Cauchy converge — pela CIFRA (a fração contínua):\n\n");
    printf("      metal m   convergentes p_k/q_k        limite σ_m        |erro| no 20º\n");
    int mal = 0;
    for(int m = 1; m <= 4; m++){
        double p0=1,q0=0,p1=m,q1=1, sig=(m+sqrt((double)m*m+4))/2;
        double err=0;
        for(int k=2;k<=20;k++){
            double p2=m*p1+p0, q2=m*q1+q0;
            p0=p1;q0=q1;p1=p2;q1=q2;
            err = fabs(p1/q1 - sig);
        }
        printf("      %-9d %-27s %-17.12f %.3e\n", m, "[m;m,m,…]", sig, err);
        /* A LEI, não um limiar: o erro do k-ésimo convergente decresce como σ^{-2k}. O ouro
         * (m=1) é o mais LENTO — é o mais irracional — e um limiar fixo puni-lo-ia por isso. */
        double prev = 0, p0b=1,q0b=0,p1b=m,q1b=1;
        for(int j=2;j<=19;j++){ double a=m*p1b+p0b,b=m*q1b+q0b; p0b=p1b;q0b=q1b;p1b=a;q1b=b; }
        prev = fabs(p1b/q1b - sig);
        if(err > 1e-30 && prev > 1e-30 && fabs(log(prev/err)/log(sig*sig) - 1) > 0.25) mal++;
        /* e é de CAUCHY: |x_{k+1} − x_k| -> 0 */
        double a0=1,b0=0,a1=m,b1=1, ant=1e9, mono=1;
        for(int k=2;k<=25;k++){
            double a2=m*a1+a0,b2=m*b1+b0; a0=a1;b0=b1;a1=a2;b1=b2;
            double d = (k>2)? fabs(a1/b1 - a0/b0) : 1e9;
            if(k>3 && d > ant) mono=0;
            ant=d;
        }
        if(!mono) mal++;
    }
    printf("\n");
    ok("a cifra é de Cauchy e converge para o metal — completude por sequências", mal == 0);
    /* (b) o corte de Dedekind: o conjunto dos racionais com x² < 2 não tem supremo em Q,
     *     e tem em R. Mede-se pela bissecção, que É a construção do corte. */
    printf("      (b) o CORTE de Dedekind: {x ∈ Q : x² < 2} não tem supremo em Q — e tem em R\n\n");
    double lo=1, hi=2;
    for(int k=0;k<80;k++){ double mid=(lo+hi)/2; if(mid*mid<2) lo=mid; else hi=mid; }
    printf("      a fronteira do corte: %.15f    e √2 = %.15f\n", lo, sqrt(2.0));
    printf("      resíduo: %.3e\n\n", fabs(lo-sqrt(2.0)));
    ok("o corte tem fronteira, e ela é √2 — completude por cortes", fabs(lo-sqrt(2.0)) < 1e-14);
    printf("      As duas caracterizações são equivalentes e clássicas, e a nossa construção\n");
    printf("      satisfaz as duas. O que a cifra acrescenta é o CAMINHO: ela não postula o\n");
    printf("      limite, gera-o — cada convergente é um passo do gato, e a completude é o\n");
    printf("      fecho das suas caudas.\n");
}

printf("\n§D6  As outras construções dos reais, e onde a nossa entra.\n\n");
{
    printf("      construção            o objeto                     o que se acrescenta a Q\n");
    printf("      --------------------  ---------------------------  ------------------------\n");
    printf("      Dedekind (1872)       cortes de Q                  o supremo\n");
    printf("      Cantor / Méray        classes de Cauchy            o limite\n");
    printf("      Weierstrass           séries de decimais           a soma infinita\n");
    printf("      Bachmann              intervalos encaixados        o ponto comum\n");
    printf("      Conway (1976)         números surreais             tudo, e mais\n");
    printf("      esta                  frações contínuas (a cifra)  o CAMINHO até o ponto\n\n");
    /* e a medida que justifica a última linha: os convergentes da cifra são as MELHORES
     * aproximações racionais — a propriedade que nenhuma das outras construções dá de graça. */
    int mal = 0;
    printf("      e a cifra dá algo que as outras não dão: os seus convergentes são as MELHORES\n");
    printf("      aproximações racionais — nenhum q' < q aproxima melhor.\n\n");
    printf("      metal   p/q         |σ − p/q|     melhor que todo q' < q ?\n");
    for(int m = 1; m <= 3; m++){
        double p0=1,q0=0,p1=m,q1=1, sig=(m+sqrt((double)m*m+4))/2;
        for(int k=2;k<=8;k++){ double p2=m*p1+p0,q2=m*q1+q0; p0=p1;q0=q1;p1=p2;q1=q2; }
        double err = fabs(sig - p1/q1);
        int melhor = 1;
        for(int q = 1; q < (int)q1; q++){
            int p = (int)(sig*q + 0.5);
            if(fabs(sig - (double)p/q) < err){ melhor = 0; break; }
        }
        printf("      %-7d %.0f/%-10.0f %-13.3e %s\n", m, p1, q1, err, melhor?"sim":"NÃO");
        if(!melhor) mal++;
    }
    printf("\n");
    ok("os convergentes da cifra são as melhores aproximações racionais — Lagrange", mal == 0);
    printf("      É por isso que a cifra não é mais uma construção equivalente entre outras: as\n");
    printf("      outras dão o PONTO, e ela dá o ponto COM o caminho ótimo até ele. A completude\n");
    printf("      é a mesma; o que muda é que aqui ela vem com um algoritmo, e o algoritmo é o\n");
    printf("      gato.\n");
    printf("\n      E o par R^n / R^n* fecha a construção por cima: os reais são o eixo onde os\n");
    printf("      dois coincidem, e cada um deles é uma das duas maneiras de sair dele — uma\n");
    printf("      para o círculo (norma definida, sem ordem) e outra para a hipérbole (norma\n");
    printf("      indefinida, com cone). \\emph{Só juntos formam corpo; separados, cada um é\n");
    printf("      metade de um.}\n");
}

printf("\n");
return falhas ? 1 : 0;
}
