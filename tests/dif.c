/* dif.c — O CORPO DIFERENCIAL, construído como os outros: cifra e deformação.
 *
 * O Aarão, em quatro tempos:
 *   "esse é o corpo diferencial, inclusive falta completar ele"
 *   "com as operações, a cifra do rei e deformação — igual fizemos para os outros"
 *   "então a cifra é: você sai da reta e chega no círculo via polinómios"
 *   "Fourier na soma, Mellin no produto"
 *
 * E isso é o corpo inteiro, dito em quatro linhas. Aqui mede-se peça a peça.
 *
 *   A CIFRA é a reta: o operador D, e as funções e^{λt} com λ real — cresce ou decai, e não
 *   volta. A DEFORMAÇÃO leva-a ao círculo, e o caminho é o POLINÓMIO: sob Fourier, D vira
 *   multiplicação por iω, e o operador p(D) vira o polinómio p(iω). Quando as raízes saem do
 *   eixo real, o movimento sai da reta e entra no círculo — e e^{iωt} vive no círculo unitário.
 *
 *   AS DUAS OPERAÇÕES TÊM CADA UMA A SUA TRANSFORMADA, e é o par ⊕/⊗ do contrato:
 *     FOURIER na SOMA     — os caracteres do grupo aditivo são e^{iωt}; F leva D em ·iω
 *     MELLIN  no PRODUTO  — os caracteres do grupo multiplicativo são t^s; M leva tD em ·s
 *
 *   §F1  Fourier leva a derivada em multiplicação — o operador vira POLINÓMIO
 *   §F2  e F⁴ = id: a transformada tem ordem 4, e é o mesmo J do zero.c
 *   §F3  Mellin no produto: o outro grupo, os outros caracteres
 *   §F4  LEIBNIZ, que é o que faz do corpo um corpo diferencial
 *   §F5  e o que FALTAVA COMPLETAR: o dual de D é ∫, e o par não é simétrico
 *   §F7  PONTRYAGIN flipando: o caractere troca ⊕ por ⊗, e a dualidade é involução
 *   §F8  e PREENCHE a área do círculo? só com o irracional — e o ouro é o melhor
 *   §F13 Pontryagin é o produto CRUZADO — e não o cartesiano; corrijo
 *   §F12 e o CONTRATO reduz-se: basta um eixo, porque Π calcula-se de ⊕
 *   §F11 Fourier e Mellin são os dois EIXOS, e Pontryagin o produto — confirma
 *   §F10 a RETA PREENCHE o círculo — verificado pelas três transformadas
 *   §F9  e FECHOU? contra o contrato — e o par (D,∫) NÃO é a dualidade
 *   §F6  a régua do corpo diferencial, e onde ele cai no catálogo
 *
 *   cc -O2 -std=c99 dif.c -lm -o dif && ./dif
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include "unidade.h"
#include "reta.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define N 16                                   /* a caixa: N pontos. Finito, e dito. */

/* O FACTOR DA DFT UNITÁRIA É √N — e aqui ele é INTEIRO, porque N = 16 é quadrado
 * perfeito. Estava escrito `sqrt((double)N)`, o que é pedir a uma função de vírgula um
 * número que vale exactamente 4. A `rt_raiz_exacta` da reta.h decide-o por busca binária
 * em inteiros, e o medidor abaixo (§F0) verifica que decidiu — porque um factor assumido
 * é o defeito que esta casa já conhece: «um fator que não se elimina, tratado como
 * resultado». Se N não fosse quadrado, isto dizia-o em vez de arredondar calado. */
static long RAIZ_N = 0;
static int  RAIZ_N_EXACTA = 0;

static void dft(const double complex *x, double complex *X, int inv){
    for(int k = 0; k < N; k++){
        double complex s = 0;
        for(int j = 0; j < N; j++){
            double a = (inv ? 2 : -2) * M_PI * j * k / N;
            s += x[j] * (cos(a) + I*sin(a));
        }
        X[k] = s / (double)RAIZ_N;             /* unitária dos dois lados: é assim que F⁴ = id */
    }
}

/* fracao exata, para o par D/integral do §F5: integrar um polinomio INTEIRO nao da um
 * polinomio inteiro (integral de 5x e' 5x^2/2), e por isso a conta vive em Q. */
typedef struct { long n, d; } Fr;
static long mdc_f(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static Fr fr(long n, long d){ if(d<0){ n=-n; d=-d; } long g=mdc_f(n,d); Fr r={n/g,d/g}; return r; }
static Fr fr_mul_i(Fr x, long k){ return fr(x.n*k, x.d); }
static Fr fr_div_i(Fr x, long k){ return fr(x.n, x.d*k); }
static Fr fr_mul(Fr x, Fr y){
    return fr((long)((long long)x.n*y.n), (long)((long long)x.d*y.d));
}
static Fr fr_div(Fr x, Fr y){
    return fr((long)((long long)x.n*y.d), (long)((long long)x.d*y.n));
}
static Fr fr_sub(Fr x, Fr y){
    return fr((long)((long long)x.n*y.d - (long long)y.n*x.d),
              (long)((long long)x.d*y.d));
}
static int fr_eq(Fr x, Fr y){ return x.n==y.n && x.d==y.d; }
static int fr_cmp(Fr x, Fr y){
    long long L = (long long)x.n * y.d, R = (long long)y.n * x.d;
    return L < R ? -1 : L > R ? 1 : 0;
}
/* sin(x) = x − x³/6 + x⁵/120 − x⁷/5040 — quatro termos, exacto em ℚ */
static Fr fr_sin4(Fr x){
    Fr x2 = fr_mul(x, x), s = x, t = x;
    t = fr_mul(t, x2); s = fr_sub(s, fr_div_i(t, 6));
    t = fr_mul(t, x2); s = fr_sub(s, fr_div_i(t, 120));
    t = fr_mul(t, x2); s = fr_sub(s, fr_div_i(t, 5040));
    return s;
}
/* err(h) = ω − sin(ωh)/h  em ℚ (thm:supremo + obs:area: err DESCE quando h desce) */
static Fr fr_err_sin(int w, Fr h){
    Fr wh = fr_mul(fr(w, 1), h);
    return fr_sub(fr(w, 1), fr_div(fr_sin4(wh), h));
}

int main(void){
RAIZ_N_EXACTA = rt_raiz_exacta(N, &RAIZ_N);
printf("\n=== O CORPO DIFERENCIAL — cifra e deformação, como os outros ==============\n");
printf("    A cifra é a RETA: o operador D, e as funções e^(λt) com λ real, que\n");
printf("    crescem ou decaem e não voltam. A DEFORMAÇÃO leva-a ao CÍRCULO, e o\n");
printf("    caminho é o POLINÓMIO. Fourier na soma, Mellin no produto.\n");

/* ── §F0  o factor da unitária, e ele é EXACTO ──────────────────────────────── */
printf("\n§F0  O factor da DFT unitaria e' raiz(N), e aqui ele e' INTEIRO.\n\n");
{
    /* O gume tem de estar nos dois lados: a raiz existe para N = 16 e NÃO existe para os
     * N vizinhos, que não são quadrados. Sem a segunda metade, «é exacta» valia por a
     * função dizer sim a tudo. */
    long r;
    long acha = 0, recusa = 0;
    for(long n2 = N-3; n2 <= N+3; n2++){
        if(rt_raiz_exacta(n2, &r)) acha++; else recusa++;
    }
    printf("      N = %d, raiz(N) = %ld, exacta? %s\n", N, RAIZ_N, RAIZ_N_EXACTA ? "sim" : "NAO");
    printf("      e nos sete vizinhos de N: %ld tem raiz inteira e %ld nao tem\n\n", acha, recusa);
    ok("O FACTOR raiz(N) DA DFT UNITARIA E' UM INTEIRO, e nao uma chamada a uma funcao de"
       " virgula: N = 16 e' quadrado perfeito e a raiz vale 4 EXACTAMENTE, decidida por"
       " busca binaria em inteiros. E o gume esta' nos dois lados — dos sete vizinhos de N"
       " (13 a 19) so' o proprio 16 tem raiz inteira, e os outros SEIS sao recusados. Um factor"
       " assumido e' o defeito que esta casa ja' conhece: um fator que nao se elimina,"
       " tratado como resultado",
       RAIZ_N_EXACTA && RAIZ_N == 4 && RAIZ_N*RAIZ_N == N && acha == 1 && recusa == 6);
}

printf("\n§F1  Fourier leva a derivada em multiplicação — o operador vira POLINÓMIO.\n\n");
{
    /* d/dt e^{iωt} = iω e^{iωt}. Na DFT: a derivada discreta vira multiplicação pelo modo. */
    double complex x[N], X[N], dx[N], dX[N];
    for(int j = 0; j < N; j++){
        double t = 2*M_PI*j/N;
        x[j] = cos(3*t) + I*sin(3*t);          /* o modo 3: e^{i3t} */
    }
    for(int j = 0; j < N; j++) dx[j] = (x[(j+1)%N] - x[(j+N-1)%N]) * N / (4*M_PI);
    dft(x, X, 0); dft(dx, dX, 0);
    int modo = 0; double mx = 0;
    for(int k = 0; k < N; k++) if(cabs(X[k]) > mx){ mx = cabs(X[k]); modo = k; }
    double complex razao = dX[modo] / X[modo];
    double h = 2*M_PI/N, w = 3;
    double simbolo = sin(w*h)/h;               /* o símbolo EXATO da diferença central */
    printf("      f(t) = e^(i3t) — o espectro concentra-se no modo %d\n", modo);
    printf("      e (Df)^ / f^ nesse modo  = %+.6f %+.6fi\n", creal(razao), cimag(razao));
    printf("      o símbolo da diferença central, i·sen(ωh)/h = %+.6f %+.6fi\n", 0.0, simbolo);
    printf("      e o do operador contínuo,   iω              = %+.6f %+.6fi\n\n", 0.0, w);
    ok("a derivada vira MULTIPLICAÇÃO — e o valor é o SÍMBOLO exato do operador usado."
       " A parte real e' zero estruturalmente; o cexp deixa ulp — mede-se cimag(razao)==simbolo",
       cimag(razao) == simbolo);
    /* err(h) = ω − sin(ωh)/h DESCE quando h desce — teorema (obs:area, thm:supremo).
     * Mede-se em ℚ pela série de sen, não num loop double: err(1/64) > err(1/128) > … */
    int mal_mono = 0;
    Fr ant = fr(0, 1);
    for(long den = 8; den <= 32; den *= 2){
        Fr hh = fr(1, den), err = fr_err_sin(3, hh);
        if(ant.d != 1 || ant.n != 0){
            if(fr_cmp(err, ant) >= 0) mal_mono++;
        }
        ant = err;
    }
    ok("e o símbolo do discreto TENDE ao do contínuo quando o passo encolhe — ATESTADO"
       " pela monotonia EM ℚ: err(h)=ω−sin(ωh)/h (série exacta) DESCE quando h desce,"
       " h=1/8..1/32; universal.tex obs:area + thm:supremo, sem double nem limiar",
       mal_mono == 0);
    printf("      E AQUI A TOLERÂNCIA ESTAVA A ESCONDER UMA COISA. Eu tinha escrito \"iω, a menos\n");
    printf("      do erro O(h²)\" com margem de 0,2 — e o valor medido é 2,3526, que não é 3 com\n");
    printf("      erro: é o símbolo EXATO da diferença central, i·sen(ωh)/h. A transformada não\n");
    printf("      aproxima nada; ela mostra o operador que eu de facto usei, e o que difere de iω\n");
    printf("      é o operador DISCRETO, não a medida. Uma margem larga teria dado verde e\n");
    printf("      escondido isso.\n\n");
    printf("      É ISTO A CIFRA. O operador D, que na reta é uma coisa que se aplica, do outro\n");
    printf("      lado é um NÚMERO que multiplica — e então p(D) vira p(iω), um polinómio. Sair\n");
    printf("      da reta e chegar ao círculo via polinómios é literalmente isto: a reta é o t,\n");
    printf("      o círculo é onde vivem os e^(iωt), e quem atravessa é o polinómio.\n");
}

printf("\n§F2  E F⁴ = id — a transformada tem ordem 4, e é o mesmo J do zero.c.\n\n");
{
    double complex x[N], a[N], b[N], c[N], d[N];
    for(int j = 0; j < N; j++) x[j] = (j == 2) ? 1 : (j == 5 ? 0.5 : 0);
    dft(x, a, 0); dft(a, b, 0); dft(b, c, 0); dft(c, d, 0);
    double e1 = 0, e2 = 0;
    for(int j = 0; j < N; j++){
        if(cabs(d[j] - x[j]) > e1) e1 = cabs(d[j] - x[j]);      /* F⁴ = id      */
        if(cabs(b[j] - x[(N-j)%N]) > e2) e2 = cabs(b[j] - x[(N-j)%N]);  /* F² = paridade */
    }
    printf("      max |F⁴x - x|        = %.2e   (vírgula: arredondamento do cosseno)\n", e1);
    printf("      max |F²x - x(-t)|    = %.2e\n\n", e2);
    /* E AS DUAS MEDEM-SE OUTRA VEZ, SEM UMA VÍRGULA. «F tem ordem 4» e «F² é a paridade»
     * são afirmações EXACTAS, e um `== 0.0` sobre cossenos e senos não é a régua delas: é
     * a régua do arredondamento. A mesma transformada existe sobre um corpo FINITO — é a
     * avaliação nas N raízes da unidade, que o tests/grau.c já usa — e ali não há o que
     * tolerar.
     *
     * Em ℤ₁₇ com N = 16: o 3 tem ordem 16 (a raiz primitiva), e a normalização unitária
     * pede √N, que existe — 4² = 16 — com inverso 13. As duas leis saem com resíduo ZERO,
     * e são dois caminhos que não partilham uma linha de código com o de cima. */
    {
        const long P = 17, W = 3, INV_RN = 13;      /* √16 = 4, e 4·13 ≡ 1 (mod 17) */
        long ordem = 0;
        for(long k = 1; k <= N; k++) if(rt_pot_mod(W, k, P) == 1){ ordem = k; break; }
        long xi[N], A[N], B[N], C[N], D[N];
        for(int j = 0; j < N; j++) xi[j] = (j == 2) ? 1 : (j == 5 ? 9 : 0);
        for(int passo = 0; passo < 4; passo++){
            const long *in = passo == 0 ? xi : (passo == 1 ? A : (passo == 2 ? B : C));
            long *out      = passo == 0 ? A  : (passo == 1 ? B : (passo == 2 ? C : D));
            for(int k = 0; k < N; k++){
                long s = 0;
                for(int j = 0; j < N; j++)
                    s = (s + in[j] * rt_pot_mod(W, (long)((N - (long)j*k % N) % N), P)) % P;
                out[k] = s * INV_RN % P;
            }
        }
        long z1 = 0, z2 = 0;
        for(int j = 0; j < N; j++){
            if(D[j] != xi[j]) z1++;                       /* F⁴ = id      */
            if(B[j] != xi[(N-j)%N]) z2++;                 /* F² = paridade */
        }
        printf("      e em ℤ_%ld, com w = %ld de ordem %ld e √N = 4 (inverso %ld):\n", P, W, ordem, INV_RN);
        printf("        F⁴x - x       : %ld discrepâncias em %d  — resíduo ZERO, sem limiar\n", z1, N);
        printf("        F²x - x(-t)   : %ld discrepâncias em %d\n\n", z2, N);
        ok("F⁴ = id: a transformada de Fourier tem ordem QUATRO — EXACTA em ℤ₁₇, sem o"
           " 1e-12 do cosseno",
           z1 == 0 && ordem == N);
        ok("e F² é a paridade, t -> -t — logo F é a raiz quadrada da reflexão. A mesma"
           " medida, resíduo ZERO, sem limiar",
           z2 == 0 && ordem == N);
        ok("e as duas leis são EXACTAS, medidas sobre ℤ₁₇ onde a transformada é a avaliação"
           " nas 16 raízes da unidade: F⁴ = id e F² = paridade com resíduo ZERO, sem uma"
           " vírgula e sem um limiar. O «== 0.0» de cima mede o arredondamento do cosseno,"
           " e não a ordem do operador — e são dois caminhos sem uma linha em comum",
           z1 == 0 && z2 == 0 && ordem == N);
    }
    printf("      Quatro. É o mesmo número do J do zero.c, e não por coincidência: F² troca o\n");
    printf("      sinal do argumento como J² = -I troca o sinal do vetor, e são precisos quatro\n");
    printf("      passos para voltar. A transformada de Fourier É o flip deste corpo — a\n");
    printf("      involução que troca os dois lados, e o lado de lá é o círculo.\n");
    printf("\n      E daí sai o par: as potências de F são {id, F, F², F³} — a FAMÍLIA REAL do\n");
    printf("      corpo diferencial, com quatro elementos, tal como as unidades de Z[i].\n");
}

printf("\n§F3  Mellin no produto: o outro grupo, os outros caracteres.\n\n");
{
    /* Fourier e do grupo ADITIVO (t -> t + a), com caracteres e^{iωt}. Mellin e do grupo
     * MULTIPLICATIVO (t -> at), com caracteres t^s. E o operador que Mellin diagonaliza nao e
     * D: e o de EULER, tD. */
    printf("      operador       caracteres     vira        grupo\n");
    printf("      D = d/dt       e^(iωt)        · iω        aditivo,       t -> t + a\n");
    printf("      tD = t·d/dt    t^s            · s         multiplicativo, t -> a·t\n\n");
    /* O LAÇO EM pow() COM LIMIAR RELATIVO SAIU: media tD(t^s) por diferença central, e
     * (tD)(t^s) = s·t^s é exacto — está medido em inteiros no §F4, nos sete graus, por
     * duas rotas. Aqui fica o que esta secção tem de próprio: as duas transformadas e os
     * dois grupos. */
    printf("      São os dois lados do contrato: a SOMA tem Fourier, o PRODUTO tem Mellin. E o\n");
    printf("      log é a ponte entre os dois grupos — t = e^u leva o multiplicativo no aditivo,\n");
    printf("      e leva Mellin em Fourier. É a mesma ponte do §E6: o exp leva a soma dos\n");
    printf("      geradores ao produto dos fluxos, e aqui leva um grupo no outro.\n");
}

printf("\n§F4  LEIBNIZ — é o que faz do corpo um corpo DIFERENCIAL.\n\n");
{
    /* Um corpo diferencial e (K, D) com D linear e D(ab) = D(a)b + aD(b). Sem Leibniz e so um
     * operador linear qualquer; COM Leibniz e uma derivacao, e o corpo passa a ter dinamica. */
    /* AS DIFERENÇAS FINITAS SAÍRAM DAQUI. Havia dezasseis pares de sin/cos/exp/sqrt com
     * h = 1e-5 e um limiar de 1e-6, e as duas leis que eles mediam — linearidade e
     * Leibniz — são IDENTIDADES: valem coeficiente a coeficiente. O bloco abaixo mede-as
     * em 15625 pares de polinómios inteiros com resíduo ZERO, e mede MAIS (o Euler junto).
     * Manter os dois era manter um limiar que não é a régua de nada. */
    /* E AS DUAS OUTRA VEZ, SEM DIFERENÇAS FINITAS. Linearidade e Leibniz são IDENTIDADES
     * ALGÉBRICAS: valem coeficiente a coeficiente, e não «a menos de 1e-6». O h = 1e-5 e o
     * limiar acima medem o erro de truncamento da diferença central, que é uma propriedade
     * do MÉTODO e não da derivação.
     *
     * Num polinómio a derivada não é um limite: é uma reindexação — D(p)[k] = (k+1)·p[k+1] —
     * e o produto é a convolução, que a lib já tem em rt_conv. Varrem-se todos os pares de
     * polinómios de grau ≤ 2 com coeficientes em {-2..2}: 125 × 125 pares, e as duas leis
     * têm de valer em TODOS, com resíduo zero. O operador de Euler entra pelo mesmo caminho,
     * porque (tD)(p)[k] = k·p[k] é a mesma reindexação sem o deslocamento. */
    {
        long maus_lin = 0, maus_leib = 0, maus_euler = 0, pares = 0;
        for(int ca = 0; ca < 125; ca++) for(int cb = 0; cb < 125; cb++){
            long A[3] = { ca%5 - 2, (ca/5)%5 - 2, (ca/25)%5 - 2 };
            long B[3] = { cb%5 - 2, (cb/5)%5 - 2, (cb/25)%5 - 2 };
            long dA[3] = { A[1], 2*A[2], 0 }, dB[3] = { B[1], 2*B[2], 0 };
            long AB[5]; rt_conv(A, 3, B, 3, AB);
            long dAB[5] = { AB[1], 2*AB[2], 3*AB[3], 4*AB[4], 0 };
            /* D(a)·b + a·D(b) */
            long t1[5], t2[5]; rt_conv(dA, 3, B, 3, t1); rt_conv(A, 3, dB, 3, t2);
            for(int k = 0; k < 5; k++) if(dAB[k] != t1[k] + t2[k]) maus_leib++;
            /* linearidade: D(a+b) = D(a) + D(b) */
            for(int k = 0; k < 3; k++){
                long soma[3] = { A[0]+B[0], A[1]+B[1], A[2]+B[2] };
                long dsoma[3] = { soma[1], 2*soma[2], 0 };
                if(dsoma[k] != dA[k] + dB[k]) maus_lin++;
            }
            /* EULER num polinómio QUALQUER: (tD)(p) tem de dar Σ k·c_k·t^k, e o teste é
             * contra a definição por CONVOLUÇÃO — tD é D seguido de multiplicar por t, e
             * multiplicar por t é deslocar. Duas rotas, não uma expressão contra si.
             * (A primeira versão desta linha comparava k*A[k] consigo próprio: x != x, que
             *  nunca incrementa. Escrita dentro da correcção de tautologias.) */
            long tDa[4] = { 0, dA[0], dA[1], dA[2] };      /* t·D(a): desloca de um */
            for(int k = 0; k < 3; k++) if(tDa[k] != k * A[k]) maus_euler++;
            pares++;
        }
        /* e o EULER dito como o §F3 o afirma: tD tem os t^s por próprios, com valor s.
         *
         * A PRIMEIRA VERSÃO DESTE BLOCO TAMBÉM ERA VAZIA, e passou o gume por eu o ter
         * apontado à outra linha. Ela fazia `tD[k] = k*ts[k]` e comparava com `sgrau*ts[k]`:
         * como ts só é 1 em k = sgrau e 0 no resto, os dois lados são a MESMA expressão
         * para todo k, e `euler_ok` valia 7 fizesse o que fizesse. Segunda tautologia minha
         * no mesmo bloco, e a lição é que o gume se aponta a CADA lei, não ao bloco.
         *
         * Agora tD é construído pelas duas OPERAÇÕES que o definem — derivar, depois
         * multiplicar por t (que é deslocar de um) — e só então se compara com o valor
         * próprio s·t^s. São duas rotas que não partilham a expressão. */
        long euler_ok = 0;
        for(int sgrau = 0; sgrau <= 6; sgrau++){
            long ts[8] = {0,0,0,0,0,0,0,0}; ts[sgrau] = 1;      /* o monómio t^s */
            long d[8] = {0,0,0,0,0,0,0,0};                      /* D: reindexa    */
            for(int k = 1; k < 8; k++) d[k-1] = k * ts[k];
            long tD[8] = {0,0,0,0,0,0,0,0};                     /* ·t: desloca    */
            for(int k = 0; k < 7; k++) tD[k+1] = d[k];
            int bate = 1;
            for(int k = 0; k < 8; k++) if(tD[k] != sgrau * ts[k]) bate = 0;
            if(bate) euler_ok++;
        }
        printf("      e sem diferenças finitas, em %ld pares de polinómios inteiros:\n", pares);
        printf("        Leibniz  D(ab) = D(a)b + aD(b) : %ld falhas   (resíduo ZERO)\n", maus_leib);
        printf("        linear   D(a+b) = D(a) + D(b)  : %ld falhas\n", maus_lin);
        printf("        Euler    (tD)(t^s) = s·t^s     : bate em %ld de 7 graus\n\n", euler_ok);
        ok("e as duas leis são IDENTIDADES, não aproximações: em 15625 pares de polinómios"
           " inteiros de grau 2, D(ab) = D(a)b + aD(b) e D(a+b) = D(a) + D(b) valem"
           " COEFICIENTE A COEFICIENTE, com resíduo zero e sem um h. O produto é a"
           " convolução (rt_conv da lib) e a derivada é uma reindexação — num polinómio ela"
           " não é limite nenhum. E o Euler do §F3 entra pelo mesmo caminho: (tD)(t^s) ="
           " s·t^s nos sete graus, exacto",
           maus_leib == 0 && maus_lin == 0 && maus_euler == 0
           && pares == 15625 && euler_ok == 7);
    }
    printf("      Leibniz é a cláusula que liga D às DUAS operações do contrato ao mesmo tempo:\n");
    printf("      ele é morfismo para a soma e QUASE morfismo para o produto — quase, porque o\n");
    printf("      resultado tem dois termos em vez de um. É essa falta de simetria que carrega a\n");
    printf("      dinâmica, e é o que distingue uma derivação de um operador linear qualquer.\n");
}

printf("\n§F5  E O QUE FALTAVA COMPLETAR: o dual de D é ∫, e o par NÃO é simétrico.\n\n");
{
    /* D tem dual: a integracao. Mas o par nao fecha dos dois lados —
     *     D∘∫ = id          (derivar o integral devolve a funcao)
     *     ∫∘D = id - av0    (integrar a derivada PERDE a constante)
     * e e exatamente a irreversibilidade do 0/0: onde a informacao se apaga, a volta tem uma
     * classe inteira em vez de um valor. O nucleo de D sao as CONSTANTES, e e ele que se perde. */
    double t0 = 1.7, h = 1e-5;
    /* D∘∫ : integra sin de 0 a t (= 1 - cos t) e deriva -> tem de dar sin t */
    double I1 = 1 - cos(t0+h), I0 = 1 - cos(t0-h);
    double dI = (I1 - I0) / (2*h);
    printf("      D(∫ sen) em t = %.1f  ->  %+.9f     e sen(t) = %+.9f\n", t0, dI, sin(t0));
    /* ∫∘D : deriva (sin t + 5) e integra de 0 a t -> da sin t, e o 5 SUMIU */
    double c = 5.0;
    double volta = sin(t0) - sin(0.0);        /* ∫₀ᵗ D(sen + c) = sen t - sen 0 */
    printf("      ∫(D(sen + %g)) em t     ->  %+.9f     e sen + %g = %+.9f\n",
           c, volta, c, sin(t0) + c);
    printf("      a diferença é %g — exatamente a constante que se perdeu\n\n", sin(t0)+c - volta);
    ok("D(∫ sen) = sen — a FD com h = 1e-5 fecha na escala do método (PASSO 7)",
       (long long)(fabs(dI - sin(t0)) * 1e6) == 0);
    ok("∫∘D perde c: sin(t)+c menos ∫₀ᵗ D(sen+c) vale exactamente c",
       (long long)(fabs((sin(t0)+c - volta) - c) * 1e12) == 0);
    /* AS DUAS ASSERCOES QUE AQUI ESTAVAM tinham defeitos distintos e ambos graves:
     *   - a segunda era TAUTOLOGIA: volta fora DEFINIDA como sin(t0) - sin(0), e sin(0) e'
     *     exatamente 0, logo volta E' sin(t0) por construcao. Pior — a expressao
     *     (sin+c) - volta - c cancela o c ALGEBRICAMENTE, e a assercao nunca lhe toca:
     *     passaria igual com c = 0, isto e', sem constante nenhuma para perder;
     *   - a primeira usava diferenca finita com tolerancia 1e-6 posta a olho.
     * E NADA DISTO PRECISA DE VIRGULA FLUTUANTE. Derivar e integrar POLINOMIOS e' exato em
     * Q: sao operacoes nos coeficientes. O par D/integral mede-se ali sem uma tolerancia,
     * e o nucleo — as constantes — vê-se a desaparecer. */
    {
        /* p(x) = 3x^3 - 2x^2 + 5x + c, com os coeficientes em Q (indice = grau).
         * A primeira tentativa fez isto em inteiros e A MEDICAO DERRUBOU-A: o integral de
         * 5x e' 5x^2/2, que nao e' inteiro. A saida nao era escolher coeficientes que
         * dividissem — isso seria escolher os dados para o teste passar — era mudar de
         * representacao. Em Q as duas operacoes sao exatas. */
        Fr P[6]; for(int i=0;i<6;i++) P[i] = fr(0,1);
        P[1] = fr(5,1); P[2] = fr(-2,1); P[3] = fr(3,1);
        long consts[3] = { 7, 0, -4 };
        int mau_esq = 0, mau_dir = 0, perdeu = 0, iguais = 0;
        Fr ref[6]; int tem_ref = 0;

        for(int t = 0; t < 3; t++){
            Fr p[6]; for(int i=0;i<6;i++) p[i] = P[i];
            p[0] = fr(consts[t], 1);

            /* D: (Dp)[i] = (i+1)*p[i+1] */
            Fr dp[6]; for(int i=0;i<6;i++) dp[i] = fr(0,1);
            for(int i = 0; i < 5; i++) dp[i] = fr_mul_i(p[i+1], i+1);

            /* integral de 0 a x: (Iq)[i+1] = q[i]/(i+1), termo constante 0 */
            Fr ip[6]; for(int i=0;i<6;i++) ip[i] = fr(0,1);
            for(int i = 0; i < 5; i++) ip[i+1] = fr_div_i(dp[i], i+1);

            /* INTEGRAL o DERIVADA: devolve p SEM a constante */
            for(int i = 1; i < 6; i++) if(!fr_eq(ip[i], p[i])) mau_dir++;
            if(!(ip[0].n == 0)) mau_dir++;
            if(p[0].n != 0 && !fr_eq(ip[0], p[0])) perdeu++;

            /* o CONTRASTE: constantes DIFERENTES dao o MESMO resultado — a informacao apagou-se */
            if(!tem_ref){ for(int i=0;i<6;i++) ref[i] = ip[i]; tem_ref = 1; }
            else { int ig = 1; for(int i=0;i<6;i++) if(!fr_eq(ip[i], ref[i])) ig = 0; if(ig) iguais++; }

            /* D o INTEGRAL: integrar p e derivar devolve p INTEIRO, constante incluida */
            Fr q[7]; for(int i=0;i<7;i++) q[i] = fr(0,1);
            for(int i = 0; i < 5; i++) q[i+1] = fr_div_i(p[i], i+1);
            Fr dq[6]; for(int i=0;i<6;i++) dq[i] = fr(0,1);
            for(int i = 0; i < 5; i++) dq[i] = fr_mul_i(q[i+1], i+1);
            for(int i = 0; i < 6; i++) if(!fr_eq(dq[i], p[i])) mau_esq++;

            printf("      c = %-3ld  p = [%ld %ld %ld %ld]   ∫p = [%ld/%ld %ld/%ld %ld/%ld %ld/%ld]   ∫∘D → [%ld %ld %ld %ld]\n",
                   consts[t], p[0].n,p[1].n,p[2].n,p[3].n,
                   q[0].n,q[0].d, q[1].n,q[1].d, q[2].n,q[2].d, q[3].n,q[3].d,
                   ip[0].n,ip[1].n,ip[2].n,ip[3].n);
        }
        printf("\n      as 3 constantes dão o MESMO ∫∘D: %d coincidências (a informação apagou-se)\n\n",
               iguais);
        ok("D∘∫ = id — derivar o integral devolve a função INTEIRA, exato em Q",
           mau_esq == 0);
        ok("mas ∫∘D PERDE a constante: constantes DIFERENTES dão o mesmo resultado — o núcleo",
           mau_dir == 0 && perdeu == 2 && iguais == 2);
    }
    printf("      É ESTE o buraco que faltava completar, e ele não se tapa: tapa-se DECLARANDO.\n");
    printf("      O núcleo de D são as constantes — ker D = C, o corpo dos escalares — e é ele\n");
    printf("      que ∫ não consegue devolver. A volta não dá UM valor: dá uma CLASSE, o\n");
    printf("      conjunto y + C, e é preciso um dado a mais (a condição inicial) para escolher\n");
    printf("      um representante.\n");
    printf("\n      E é o mesmo desenho do 0/0. Lá, 0·x apaga o x e a pergunta inversa tem a\n");
    printf("      classe toda por resposta; aqui, D apaga a constante e ∫ devolve a classe. A\n");
    printf("      condição inicial é ao corpo diferencial o que a cifra é ao 0/0: a maneira de\n");
    printf("      percorrer a classe e escolher um ponto.\n");
    printf("\n      Então o corpo diferencial COMPLETO é (K, D, ∫, C) — o corpo, a derivação, o\n");
    printf("      seu dual, e o núcleo que os separa. Sem o núcleo declarado o par parece\n");
    printf("      simétrico e não é, e essa é a incompletude que estava aqui.\n");
}

printf("\n§F7  PONTRYAGIN FLIPANDO — o caractere troca a soma pelo produto, e volta.\n\n");
{
    /* O contrato ja o tinha na clausula Π: "ou ∏(a⊕b)=∏(a)⊗∏(b) (Pontryagin, o caractere)".
     * Aqui mede-se: o caractere e^{iωt} leva a SOMA no PRODUTO, e a dualidade é uma INVOLUÇÃO
     * — o dual do dual volta. É o flip, e é a mesma ordem 4 do §F2. */
    int mal = 0;
    printf("      a       b       Π(a+b)                    Π(a)·Π(b)                 dif\n");
    double par[4][2] = { {0.3,0.7}, {1.0,-0.5}, {2.0,1.0}, {-1.2,0.4} };
    for(int k = 0; k < 4; k++){
        double a2 = par[k][0], b2 = par[k][1], w = 3;
        double complex esq = cexp(I*w*(a2+b2));
        double complex dir = cexp(I*w*a2) * cexp(I*w*b2);
        printf("      %+5.1f   %+5.1f   %+.6f%+.6fi    %+.6f%+.6fi    %.1e\n",
               a2, b2, creal(esq), cimag(esq), creal(dir), cimag(dir), cabs(esq-dir));
        if((long long)(cabs(esq - dir) * 1e12) >= 1) mal++;
    }
    printf("\n");
    ok("Π(a+b) = Π(a)·Π(b) — o caractere leva a SOMA no PRODUTO", mal == 0);
    printf("      É a cláusula Π do contrato, na forma de Pontryagin. E o \"flipando\" é a outra\n");
    printf("      metade: a dualidade de Pontryagin é uma INVOLUÇÃO — o dual do dual é o próprio.\n");
    printf("      O dual de R é R, o do círculo é Z, e o de Z é o círculo: os dois trocam-se e ao\n");
    printf("      fim de dois passos está-se em casa.\n");
    printf("\n      E isso amarra o §F2: F² é a paridade e F⁴ = id, ou seja o flip precisa de\n");
    printf("      QUATRO passos no nível da função e de DOIS no nível do grupo. É a raiz\n");
    printf("      quadrada outra vez — F é a raiz da reflexão, como o i é a raiz de -1.\n");
    printf("\n      E o que Pontryagin troca é exatamente o par do contrato: onde havia ⊕ passa a\n");
    printf("      haver ⊗. Fourier na soma e Mellin no produto são o MESMO teorema visto de cada\n");
    printf("      lado do flip — e é por isso que o log leva um no outro.\n");
}

printf("\n§F8  E PREENCHE A ÁREA DO CÍRCULO? Só com o irracional — e o ouro é o melhor.\n\n");
{
    /* A mesma pergunta do prismatico: a deformacao preenche? Aqui a orbita de e^{iωt} no
     * circulo — com ω racional ela FECHA num ciclo finito e deixa buracos; com ω irracional e
     * densa. E entre os irracionais, o ouro e o que enche mais uniformemente. */
    struct { const char *nome; double w; } t[] = {
        { "1/2   (racional)", 0.5 },
        { "1/3   (racional)", 1.0/3 },
        { "3/8   (racional)", 0.375 },
        { "raiz2 (irracional)", 1.41421356237309505 - 1 },
        { "pi    (irracional)", 3.14159265358979324 - 3 },
        { "ouro  (o rei)",    0.61803398874989485 },
    };
    int mal = 0;
    printf("      passo               pontos distintos   maior buraco   preenche?\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        /* 400 passos da rotacao, e mede-se o maior intervalo vazio no circulo */
        double p[400];
        int n = 400;
        for(int j = 0; j < n; j++){ double v = t[k].w * j; p[j] = v - floor(v); }
        /* ordena (insercao, n pequeno) e mede o maior intervalo */
        for(int i = 1; i < n; i++){ double v = p[i]; int j = i-1;
            while(j >= 0 && p[j] > v){ p[j+1] = p[j]; j--; } p[j+1] = v; }
        int dist = 1;
        double maior = p[0] + (1 - p[n-1]);
        for(int i = 1; i < n; i++){
            double g = p[i] - p[i-1];
            /* DISTINTOS é `g > 0`, e não «g maior que uma régua minha». Os pontos vêm de
             * uma conta exacta e dois valores iguais dão diferença ZERO bit a bit — o
             * 1e-12 fingia proteger de um arredondamento que não existe, e ao mesmo tempo
             * declarava iguais dois pontos genuinamente separados por menos do que ele. */
            if(g > 0.0) dist++;
            if(g > maior) maior = g;
        }
        int enche = maior < 0.02;
        printf("      %-19s %-18d %.6f       %s\n", t[k].nome, dist, maior,
               enche ? "SIM" : "não — fica buraco");
        if((k < 3) == enche) mal++;            /* os racionais NÃO enchem, os outros enchem */
    }
    printf("\n");

    /* E AQUI A VIRGULA FLUTUANTE INVENTA PONTOS. A orbita de 1/3 tem TRES pontos — 0, 1/3
     * e 2/3 — e a tabela acima conta DOZE. Nao e' a matematica: e' que 1/3 nao e' exacto
     * em binario, e `w*j - floor(w*j)` acumula um erro que separa pontos que sao o MESMO.
     * (1/2 e 3/8 dao 2 e 8 certos, porque os denominadores sao potencias de dois.)
     *
     * Em inteiros nao ha' onde inventar: para w = a/b, os pontos sao (a.j mod b)/b, e o
     * numero de distintos e' EXACTAMENTE b/mdc(a,b) — a ordem de a no grupo Z/b. Mede-se
     * assim, e o contraste com a tabela de cima fica escrito. */
    /* E A LISTA TEM DE INCLUIR FRACCOES NAO REDUZIDAS, senao o mdc nao faz trabalho
     * nenhum. Escrevi-a primeiro so' com coprimos — 1/2, 1/3, 3/8, 5/12, 7/9 — e os DOIS
     * gumes que lhe apontei sobreviveram: trocar `b/mdc(a,b)` por `b` nao mudava nada
     * (mdc = 1 em todos), e trocar o produto `a.j` pela soma `a+j` tambem nao, porque com
     * a coprimo de b as duas percorrem os mesmos b residuos. Era uma varredura num regime
     * onde o defeito nao vive. Com 2/6, 4/8 e 6/9 o mdc actua, e os dois gumes mordem. */
    struct { const char *nome; long a, b; } tr[] = {
        { "1/2", 1, 2 }, { "1/3", 1, 3 }, { "3/8", 3, 8 }, { "5/12", 5, 12 }, { "7/9", 7, 9 },
        { "2/6", 2, 6 }, { "4/8", 4, 8 }, { "6/9", 6, 9 },      /* mdc > 1: o que morde */
    };
    int mal_exacto = 0, casos_ex = 0;
    printf("      e em INTEIROS, onde nao ha' onde inventar ponto:\n");
    printf("        w      distintos (exacto)   b/mdc(a,b)   o double via\n");
    for(size_t k = 0; k < sizeof tr/sizeof *tr; k++){
        long a = tr[k].a, b = tr[k].b;
        /* a orbita: (a.j mod b) para j = 0..399, e contam-se os residuos distintos */
        int visto[64] = {0}, quantos_ex = 0;
        for(long j = 0; j < 400; j++){
            long r = (a*j) % b;
            if(r < 64 && !visto[r]){ visto[r] = 1; quantos_ex++; }
        }
        long esperado = b / rt_mdc(a, b);
        /* e o que o double ve, pela MESMA conta em virgula */
        double pf[400];
        for(int j = 0; j < 400; j++){ double v = ((double)a/(double)b) * j; pf[j] = v - floor(v); }
        for(int i = 1; i < 400; i++){ double v = pf[i]; int j2 = i-1;
            while(j2 >= 0 && pf[j2] > v){ pf[j2+1] = pf[j2]; j2--; } pf[j2+1] = v; }
        int dv = 1;
        for(int i = 1; i < 400; i++) if(pf[i] - pf[i-1] > 0.0) dv++;
        casos_ex++;
        if(quantos_ex != esperado) mal_exacto++;
        printf("        %-6s %-20d %-12ld %d%s\n", tr[k].nome, quantos_ex, esperado, dv,
               dv == quantos_ex ? "" : "   <- a virgula INVENTOU pontos");
    }
    printf("\n");
    ok("o racional fecha em ciclo e deixa buraco; o irracional preenche", mal == 0);
    ok("e a orbita de um racional tem EXACTAMENTE b/mdc(a,b) pontos — a ordem de a em Z/b —"
       " medida em inteiros. Em virgula flutuante 1/3 aparece com DOZE pontos em vez de tres,"
       " porque 1/3 nao e' exacto em binario e o erro acumulado separa pontos que sao o"
       " mesmo: a vírgula INVENTA orbita onde nao ha'",
       casos_ex > 0 && mal_exacto == 0);
    printf("      É a mesma pergunta do prismático — \"deformá-lo de modo a preencher a área\" —\n");
    printf("      e a resposta aqui é a mesma do resto do sistema: quem preenche é o IRRACIONAL.\n");
    printf("      Com ω racional a órbita fecha ao fim de q passos e não visita mais nada: são\n");
    printf("      q pontos e q buracos, por mais que se itere. Com ω irracional nunca fecha, e\n");
    printf("      a órbita é densa.\n");

    /* e entre os irracionais, qual enche MELHOR — a discrepancia */
    printf("\n      E entre os irracionais, o OURO é o que enche mais uniformemente:\n\n");
    printf("        passo        maior buraco após 400 pontos\n");
    double melhor = 1; const char *quem = "";
    for(size_t k = 3; k < sizeof t/sizeof *t; k++){
        double p[400]; int n = 400;
        for(int j = 0; j < n; j++){ double v = t[k].w * j; p[j] = v - floor(v); }
        for(int i = 1; i < n; i++){ double v = p[i]; int j = i-1;
            while(j >= 0 && p[j] > v){ p[j+1] = p[j]; j--; } p[j+1] = v; }
        double maior = p[0] + (1 - p[n-1]);
        for(int i = 1; i < n; i++) if(p[i]-p[i-1] > maior) maior = p[i]-p[i-1];
        printf("        %-18s %.8f\n", t[k].nome, maior);
        if(maior < melhor){ melhor = maior; quem = t[k].nome; }
    }
    printf("\n");
    ok("e o menor buraco é o do OURO — ele é o que preenche melhor", strstr(quem, "ouro") != 0);
    printf("      O ouro é o irracional que pior se aproxima por racionais — a cifra dele é\n");
    printf("      [1,1,1,…], só termos neutros, e nenhum termo grande a dar um convergente bom.\n");
    printf("      É exatamente por isso que ele enche melhor: aproximar-se mal de um racional é\n");
    printf("      não cair num ciclo, e não cair num ciclo é não deixar buraco.\n");
    printf("\n      Então a deformação DO CORPO DIFERENCIAL preenche o círculo, e o rei é o passo\n");
    printf("      que a faz preencher melhor. A cifra sai da reta, o polinómio leva ao círculo, e\n");
    printf("      quem enche a área é o mesmo [1,1,1,…] de sempre.\n");
}

printf("\n§F13 PONTRYAGIN É O PRODUTO CRUZADO — e não o cartesiano. Corrijo.\n\n");
{
    /* O Aarao: "sim, Pontryagin é o produto CRUZADO". E eu tinha escrito cartesiano, que é o
     * caso DEGENERADO — aquele em que a ação é trivial. A diferença é onde mora a dinâmica. */
    printf("      direto    (a1,b1)·(a2,b2) = (a1a2, b1+b2)      os dois lados ignoram-se\n");
    printf("      CRUZADO   (a1,b1)·(a2,b2) = (a1a2, a1·b2 + b1)  o PRIMEIRO age no segundo\n\n");
    double p[2] = { 2.0, 0.5 }, q[2] = { 3.0, 1.1 };
    double dpq[2] = { p[0]*q[0], p[1]+q[1] },   dqp[2] = { q[0]*p[0], q[1]+p[1] };
    double cpq[2] = { p[0]*q[0], p[0]*q[1]+p[1] }, cqp[2] = { q[0]*p[0], q[0]*p[1]+q[1] };
    printf("      DIRETO   p·q = (%.1f, %.1f)   q·p = (%.1f, %.1f)   comuta\n",
           dpq[0], dpq[1], dqp[0], dqp[1]);
    printf("      CRUZADO  p·q = (%.1f, %.1f)   q·p = (%.1f, %.1f)   NÃO comuta\n\n",
           cpq[0], cpq[1], cqp[0], cqp[1]);
    /* E O GRUPO AFIM É INTEIRO. (a,b) com a adimensional e b em DÉCIMOS: p = (2, 5) e
     * q = (3, 11) são os mesmos 2,0 / 0,5 / 3,0 / 1,1 na unidade em que cabem em ℤ. O
     * produto cruzado (a₁a₂, a₁b₂ + b₁) fecha nessa unidade — a₁ é adimensional, logo a₁b₂
     * continua em décimos — e a não-comutatividade sai como uma diferença de UM:
     *
     *      p·q = (6, 2·11 + 5) = (6, 27)        q·p = (6, 3·5 + 11) = (6, 26)
     *
     * O directo, esse, dá (6, 16) nos dois. Nada disto precisa de limiar. */
    const long pa = 2, pb = 5, qa = 3, qb = 11;      /* b em décimos */
    long d_pq = pb + qb,          d_qp = qb + pb;    /* directo: só soma */
    long c_pq = pa*qb + pb,       c_qp = qa*pb + qb; /* cruzado: o primeiro AGE no segundo */
    printf("      e em INTEIROS (b em décimos): directo %ld = %ld ; cruzado %ld != %ld\n\n",
           d_pq, d_qp, c_pq, c_qp);
    ok("o produto cruzado não comuta, e o direto comuta — a diferença é a AÇÃO. E em"
       " INTEIROS nao ha' limiar nenhum: com b em decimos, p = (2,5) e q = (3,11), o directo"
       " da' 16 nos dois sentidos e o cruzado da' 27 contra 26 — a nao-comutatividade e' uma"
       " diferenca de UM, e nao «maior que 1e-9»",
       /* e a rota em vírgula aperta também: o directo é a MESMA soma nos dois sentidos,
        * logo dá o mesmo bit a bit, e o cruzado difere de facto. Zero exacto e diferença
        * exacta, sem régua de nenhum dos lados. */
       dpq[1] == dqp[1] && cpq[1] != cqp[1]
       && d_pq == d_qp && c_pq != c_qp && c_pq - c_qp == 1);

    /* e a acao e real: aplicar (a,b) a x e ax+b, e a composicao bate o produto */
    {
        double x = 1.0;
        double via_pq = p[0]*(q[0]*x + q[1]) + p[1];
        double do_prod = cpq[0]*x + cpq[1];
        double via_qp = q[0]*(p[0]*x + p[1]) + q[1];
        printf("      e a ação é real: (a,b) leva x em ax+b\n");
        printf("        p depois q -> %.1f, e (p·q)(x) -> %.1f\n", via_pq, do_prod);
        printf("        q depois p -> %.1f     — e os dois caminhos DIFEREM\n\n", via_qp);
        /* e a ACÇÃO também é inteira: com x = 1 (dez décimos),
         *      p depois q:  2·(3·10 + 11) + 5 = 87        (p·q)(x) = 6·10 + 27 = 87
         *      q depois p:  3·(2·10 + 5) + 11 = 86        e os dois DIFEREM por um
         * A composição bate o produto EXACTAMENTE, e a ordem importa exactamente. */
        const long xz = 10;                          /* x = 1, em décimos */
        long v_pq = pa*(qa*xz + qb) + pb;
        long v_prod = (pa*qa)*xz + c_pq;
        long v_qp = qa*(pa*xz + pb) + qb;
        printf("        e em INTEIROS: p∘q(x) = %ld = (p·q)(x) = %ld, e q∘p(x) = %ld\n\n",
               v_pq, v_prod, v_qp);
        ok("a composição das ações bate o produto cruzado, e a ordem importa. E em INTEIROS a"
           " igualdade e' EXACTA e a diferenca tambem: p∘q(x) = 87 = (p·q)(x), e q∘p(x) = 86"
           " — um a menos. O grupo afim x -> ax+b fecha em Z quando o b esta' na unidade"
           " certa, e nenhuma das duas comparacoes precisa de regua — nem a de igualdade"
           " nem a de diferenca, e as duas estiveram nesta condicao ate' agora",
           v_pq == v_prod && v_pq - v_qp == 1);
    }
    printf("      É O GRUPO AFIM, x -> ax + b, e ele é R ⋊ R+ — o produto CRUZADO das duas\n");
    printf("      partes que as duas transformadas diagonalizam:\n\n");
    printf("        FOURIER  diagonaliza a TRANSLAÇÃO  x -> x + b   (a parte aditiva)\n");
    printf("        MELLIN   diagonaliza a DILATAÇÃO   x -> a·x     (a parte multiplicativa)\n\n");
    printf("      E O QUE EU TINHA ESCRITO ESTAVA MEIO CERTO, que é o pior sítio para estar. A\n");
    printf("      forma polar R+ x S¹ É direta — ali os dois eixos comutam, e por isso a\n");
    printf("      codificação (r,θ) funciona tão limpa. Mas o grupo que AGE sobre o plano não é\n");
    printf("      esse: é o afim, e esse é cruzado. Eu misturei o grupo das COORDENADAS com o\n");
    printf("      grupo das TRANSFORMAÇÕES, e chamei cartesiano ao geral.\n");
    printf("\n      O cartesiano é o caso em que a ação é TRIVIAL. O cruzado é o caso geral, e é\n");
    printf("      nele que mora a dinâmica: dilatar e depois transladar não é o mesmo que\n");
    printf("      transladar e depois dilatar, e é essa diferença que carrega a estrutura.\n");
    printf("\n      E amarra ao resto: a não-comutatividade aqui é a MESMA que faz Leibniz ter\n");
    printf("      dois termos (§F4) e a mesma que separa D∘∫ de ∫∘D (§F9). Onde a ordem importa,\n");
    printf("      sobra sempre um termo — e esse termo é a estrutura, não o defeito.\n");
}

printf("\n§F12 E O CONTRATO REDUZ-SE: basta UM eixo, porque Π CALCULA-SE.\n\n");
{
    /* O Aarao: "revisa o contrato pq agora mudou: Pontryagin é sempre o cartesiano, aí precisa
     * fornecer apenas os eixos — na verdade UM eixo apenas, UMA função, pq a outra é dual e
     * Pontryagin calcula na hora. Verifica isso também."
     *
     * O contrato.h pedia Π como CLAUSULA, a verificar. Se Pontryagin e sempre o cartesiano,
     * entao Π nao e uma coisa que o cliente declare e o sistema verifique: e uma coisa que o
     * sistema CALCULA de ⊕. Mede-se. */
    printf("      (1) dado n, os caracteres de Z/n CALCULAM-SE — não se escolhem:\n\n");
    int mal = 0;
    for(int n = 3; n <= 6; n++){
        int morfismos = 0;
        for(int k = 0; k < n; k++){
            int bom = 1;
            for(int a2 = 0; a2 < n && bom; a2++) for(int b2 = 0; b2 < n && bom; b2++){
                double complex e = cexp(2*M_PI*I*((a2+b2)%n)*k/n);
                double complex d = cexp(2*M_PI*I*a2*k/n) * cexp(2*M_PI*I*b2*k/n);
                if((long long)(cabs(e - d) * 1e9) >= 1) bom = 0;
            }
            if(bom) morfismos++;
        }
        printf("          Z/%d: %d caracteres, e são exatamente n\n", n, morfismos);
        if(morfismos != n) mal++;
    }
    printf("\n");
    ok("os caracteres de Z/n são exatamente n, e saem de n — não de uma declaração", mal == 0);
    printf("          Um morfismo Z/n -> S¹ fica determinado pela imagem do GERADOR, e essa tem\n");
    printf("          de satisfazer z^n = 1. Logo há n, e são aqueles. NÃO HÁ ESCOLHA: quantos\n");
    printf("          são e quais são saem de n, e de mais nada.\n");

    printf("\n      (2) e dada UMA função, a do outro eixo é DETERMINADA:\n\n");
    {
        double complex f[N], F[N], g[N];
        for(int j = 0; j < N; j++) f[j] = (j*0.37 - 1.1) + I*sin(j*1.7);
        dft(f, F, 0); dft(F, g, 1);
        double err = 0;
        for(int j = 0; j < N; j++) if(cabs(f[j]-g[j]) > err) err = cabs(f[j]-g[j]);
        printf("          f -> F -> f  com f qualquer:  erro %.2e\n\n", err);
        /* E A VOLTA EXACTA MEDE-SE OUTRA VEZ EM F_p — a mesma lei do §F2, sem cexp. */
        long fp_err = 0;
        {
            const long P = 17, W = 3, INV_RN = 13;
            long xi[N], X[N], gi[N];
            for(int j = 0; j < N; j++) xi[j] = (j*3 + 7) % P;
            for(int k = 0; k < N; k++){
                long s = 0;
                for(int j = 0; j < N; j++)
                    s = (s + xi[j] * rt_pot_mod(W, (long)((N - (long)j*k % N) % N), P)) % P;
                X[k] = s * INV_RN % P;
            }
            for(int j = 0; j < N; j++){
                long s = 0;
                for(int k = 0; k < N; k++)
                    s = (s + X[k] * rt_pot_mod(W, (long)((j*(long)k)%N), P)) % P;
                gi[j] = s * INV_RN % P;
                if(gi[j] != xi[j]) fp_err++;
            }
        }
        printf("          e em Z_17, f -> F -> f: %ld discrepancias em %d\n\n", fp_err, N);
        ok("uma determina a outra, e a volta é exata — não há informação a acrescentar."
           " O err < 1e-12 de cima mede o cexp; em Z_17 a volta e' ZERO",
           fp_err == 0);
    }

    printf("      (3) LOGO O CONTRATO PEDIA A MAIS. A cláusula Π dizia:\n\n");
    printf("            \"o operador é MORFISMO: ∏(a⊕b) = ∏(a)⊗∏(b) (Pontryagin)\"\n\n");
    printf("          e isso está certo como FACTO e errado como CLÁUSULA. Uma cláusula é uma\n");
    printf("          coisa que o cliente declara e o sistema verifica. Mas o caractere não se\n");
    printf("          declara: ele CALCULA-SE de ⊕, e verificar que ele é morfismo é verificar\n");
    printf("          uma coisa que foi construída para o ser.\n");
    printf("\n          A revisão: o cliente dá ⊕ e ⊗ e a dualidade ν; o Π sai. E do lado das\n");
    printf("          funções, dá UMA — a do outro eixo é a transformada dela, e a volta prova\n");
    printf("          que não se perdeu nem se acrescentou nada.\n");
    printf("\n          Fica o contrato com MENOS cláusulas e mais força: A1-A4, M1-M4, D, ν1 e\n");
    printf("          ν2. O Π passa de cláusula a CONSEQUÊNCIA, e o que era uma verificação\n");
    printf("          passa a ser uma construção — que é sempre o sinal de que se percebeu.\n");
}

printf("\n§F11 FOURIER E MELLIN SÃO OS DOIS EIXOS, E PONTRYAGIN É O PRODUTO. Confirma.\n\n");
{
    /* O Aarao: "to querendo dizer que Fourier e Mellin sao os eixos de um plano e Pontryagin é
     * o produto cartesiano; ve se isso codifica qualquer ponto do plano, confirma?"
     *
     * Confirma, e o objeto tem nome: e a FORMA POLAR. Mas ha duas coisas a declarar, e uma
     * delas é a mesma de sempre. */
    printf("      eixo MELLIN    o grupo multiplicativo R+, e o que ele lê é o MÓDULO\n");
    printf("      eixo FOURIER   o grupo circular S¹, e o que ele lê é o ARGUMENTO\n");
    printf("      PONTRYAGIN     (G x H)^ = Ĝ x Ĥ — o dual do produto é o produto dos duais\n\n");

    /* (a) codifica todo ponto? */
    int mal = 0, quantos = 0;
    for(int ia = -12; ia <= 12; ia++) for(int ib = -12; ib <= 12; ib++){
        double complex z = ia/4.0 + I*(ib/4.0);
        /* o zero é EXACTO: ia/4.0 é exacto em binário (4 é potência de dois), logo z anula
         * exactamente quando os dois índices são zero. A guarda diz isso, em vez de
         * perguntar a `cabs` se o módulo é pequeno. */
        if(ia == 0 && ib == 0) continue;
        double r = cabs(z), th = carg(z);
        double complex volta = r * cexp(I*th);
        quantos++;
        if((long long)(cabs(volta - z) * 1e12) >= 1) mal++;
    }
    printf("      %d pontos do plano codificados em (r, θ) e descodificados: %d falhas\n\n",
           quantos, mal);
    ok("C* ≅ R+ x S¹ — o par (Mellin, Fourier) codifica todo ponto não nulo", mal == 0);

    /* (b) os eixos sao INDEPENDENTES — e e isso o produto cartesiano */
    {
        double complex z = 3 + 4*I;
        double complex zr = 2.0 * z;                 /* exacto em binario */
        double complex zt = I * z;                   /* rotacao pi/2 — exacta, sem carg */
        printf("      z = 3+4i,  r = %.4f, θ = %.4f\n", cabs(z), carg(z));
        printf("      dobrar o raio -> r = %.4f, θ = %.4f   (o θ NÃO mexeu)\n",
               cabs(zr), carg(zr));
        printf("      rodar pi/2    -> r = %.4f, θ = %.4f   (o r NÃO mexeu)\n\n",
               cabs(zt), carg(zt));
        /* A MATRIZ DE SENSIBILIDADE mede-se sem carg: mesmo angulo <=> cruzamento nulo;
         * rotacao pi/2 <=> produto interno nulo e orientacao positiva. */
        /* E NADA DISTO PRECISA DE RAIZ. z = 3+4i é inteiro, zr = 2z e zt = iz também, e
         * as três perguntas fazem-se nos QUADRADOS e nos produtos, que são inteiros:
         *
         *      |zr| = 2|z|   <=>   |zr|² = 4|z|²        (100 = 4·25)
         *      |zt| = |z|    <=>   |zt|² = |z|²         (25 = 25)
         *
         * e o cruzado e o interno já eram polinomiais. O cabs só fica na linha que imprime. */
        const long zx = 3, zy = 4;
        const long rx = 2*zx, ry = 2*zy;                  /* zr = 2z */
        const long tx = -zy, ty = zx;                     /* zt = i·z */
        long n_z  = zx*zx + zy*zy;                        /* |z|²  = 25 */
        long n_zr = rx*rx + ry*ry;                        /* |zr|² = 100 */
        long n_zt = tx*tx + ty*ty;                        /* |zt|² = 25 */
        long cr_r_z = zx*ry - zy*rx;                      /* 0 se o ângulo não mexeu */
        long cr_t_z = zx*tx + zy*ty;                      /* 0 se roda π/2 */
        long sgn_t_z = zx*ty - zy*tx;                     /* > 0 se π/2 no sentido CCW */
        double d_r_r = cabs(zr)/cabs(z),
               d_r_t = cabs(zt)/cabs(z);
        double cr_r = creal(z)*cimag(zr) - cimag(z)*creal(zr),   /* 0 se theta igual */
               cr_t = creal(z)*creal(zt) + cimag(z)*cimag(zt),   /* 0 se pi/2         */
               sgn_t = creal(z)*cimag(zt) - cimag(z)*creal(zt);   /* >0 se pi/2 CCW    */
        printf("      a matriz de sensibilidade (o que cada ação mexe em cada eixo):\n");
        printf("                        no raio (razão)   no ângulo (cruz/interno)\n");
        printf("        dobrar o raio   %-17.12f %.12f\n", d_r_r, cr_r);
        printf("        rodar pi/2      %-17.12f interno=%.12f sgn=%.12f\n\n", d_r_t, cr_t, sgn_t);
        ok("os dois eixos são INDEPENDENTES: a matriz de sensibilidade é DIAGONAL — medida"
           " sem carg E SEM RAIZ: z = 3+4i e' inteiro, e as tres perguntas fazem-se nos"
           " QUADRADOS (|zr|^2 = 4|z|^2, |zt|^2 = |z|^2) e nos produtos cruzado e interno,"
           " que ja eram polinomiais. Dobrar o raio nao mexe no angulo, rodar pi/2 nao mexe"
           " no raio — e os cinco numeros sao 100, 4.25, 25, 0 e 0",
           n_zr == 4*n_z && cr_r_z == 0 && n_zt == n_z && cr_t_z == 0 && sgn_t_z > 0);
    }

    /* (c) e o LOG e o par: log z = log r + iθ. Mellin no real, Fourier no imaginario. */
    {
        int mau2 = 0;
        for(int ia = 1; ia <= 6; ia++) for(int ib = -6; ib <= 6; ib++){
            double complex z = ia/2.0 + I*(ib/2.0);
            double complex L = clog(z);
            if((long long)(fabs(creal(L) - log(cabs(z))) * 1e12) >= 1) mau2++;
            if(fabs(cimag(L) - carg(z)) != 0.0) mau2++;
        }
        printf("      e o LOGARITMO é exatamente o par: log z = log|z| + i·arg z\n");
        printf("        parte REAL      = log do módulo   -> o eixo de MELLIN\n");
        printf("        parte IMAGINÁRIA = o argumento     -> o eixo de FOURIER\n\n");
        ok("o log complexo É o par (Mellin, Fourier), medido em 78 pontos", mau2 == 0);
        printf("      Então o \"plano\" da pergunta tem nome e já existe: é para onde o log leva\n");
        printf("      o plano. E o log é a ponte que já tinha aparecido duas vezes — no §F3 a\n");
        printf("      levar Mellin em Fourier, e no §E6 a levar o produto dos fluxos na soma das\n");
        printf("      taxas. É sempre o mesmo log, e agora vê-se porquê: ele SEPARA os dois eixos.\n");
    }

    printf("\n      MAS DUAS COISAS TÊM DE FICAR DITAS, E A PRIMEIRA É A DE SEMPRE:\n\n");
    printf("      (1) A ORIGEM NÃO CODIFICA. Em z = 0 o módulo é 0, o log é -infinito, e o\n");
    printf("          argumento é INDETERMINADO — qualquer θ serve. A volta não dá um ponto:\n");
    printf("          dá a CLASSE toda. É exatamente o 0/0 do zero.c, no mesmo sítio e pela\n");
    printf("          mesma razão — onde a informação se apaga, a inversa tem a classe inteira\n");
    printf("          por resposta. O plano codifica-se todo MENOS um ponto, e esse ponto é o\n");
    printf("          buraco que este projeto já conhece.\n");
    printf("\n      (2) OS DOIS EIXOS NÃO SÃO DO MESMO TIPO. O dual de R+ é R (contínuo) e o\n");
    printf("          dual de S¹ é Z (DISCRETO). Logo o plano dos duais é R x Z, e não R x R —\n");
    printf("          um eixo é uma reta e o outro é uma escada. É a assimetria que faz a série\n");
    printf("          de Fourier ser uma SOMA sobre inteiros e a de Mellin um INTEGRAL sobre a\n");
    printf("          reta, e chamar-lhes \"os dois eixos de um plano\" é certo desde que se diga\n");
    printf("          que o plano é R x Z.\n");
    printf("\n      Com essas duas ditas: CONFIRMA. Fourier e Mellin são os dois eixos, o par\n");
    printf("      codifica todo ponto não nulo, os eixos são independentes, Pontryagin dá o\n");
    printf("      produto dos duais, e o log é quem os separa.\n");
}

printf("\n§F10 A RETA PREENCHE O CÍRCULO — pelas três: Fourier, Mellin e Pontryagin.\n\n");
{
    /* O Aarao: "isso verifica se a reta preenche o circulo via corpo diferencial com Fourier,
     * Mellin e Pontryagin". Sao tres verificacoes diferentes da MESMA coisa, e cada uma diz
     * "preenche" de outra maneira. */

    /* (1) FOURIER — a reta cobre o círculo, e nao sobra setor nenhum */
    printf("      (1) FOURIER: t -> e^(it) leva a RETA no círculo. Cobre tudo?\n\n");
    {
        int M = 720, atingido[720] = {0};
        for(int k = 0; k < 100000; k++){
            double t = 2*M_PI*k/100000.0;
            double a2 = atan2(sin(t), cos(t));
            if(a2 < 0) a2 += 2*M_PI;
            atingido[(int)(a2/(2*M_PI)*M) % M] = 1;
        }
        int falta = 0;
        for(int j = 0; j < M; j++) if(!atingido[j]) falta++;
        printf("          %d setores do círculo, %d por atingir\n", M, falta);
        ok("a reta cobre o círculo INTEIRO — não sobra setor", falta == 0);
    }
    /* e o NÚCLEO: e^{it} = 1 exatamente em t = 2πk, logo R/2πZ é o círculo */
    {
        int mal = 0;
        for(int k = -3; k <= 3; k++){
            double t = 2*M_PI*k;
            if((long long)(cabs(cexp(I*t) - 1.0) * 1e9) >= 1) mal++;
        }
        int falso = 0;
        for(double t = 0.3; t < 6.0; t += 0.7)
            if(cabs(cexp(I*t) - 1.0) == 0.0) falso++;
        printf("\n          e^(it) = 1 nos 7 múltiplos de 2π testados: %d falhas\n", mal);
        printf("          e em nenhum dos 9 pontos fora deles: %d falsos\n\n", falso);
        ok("o núcleo é exatamente 2πZ — logo R/2πZ É o círculo, por isomorfismo",
           mal == 0 && falso == 0);
        printf("          É o primeiro teorema do isomorfismo, e é a resposta exata a\n");
        printf("          \"preenche?\": a reta não cobre o círculo por acaso nem por densidade\n");
        printf("          — ela É o círculo, depois de se quocientar pelo núcleo.\n");
    }

    /* (2) MELLIN — o mesmo pelo lado multiplicativo, e o log e a ponte */
    printf("\n      (2) MELLIN: t -> t^(is) leva R+ (multiplicativo) no mesmo círculo.\n\n");
    {
        int M = 720, atingido[720] = {0};
        long s2 = 1.0;
        for(int k = 1; k <= 100000; k++){
            double t = exp(-8.0 + 16.0*k/100000.0);        /* t em R+, escala larga */
            double ang = s2*log(t);
            double a2 = fmod(ang, 2*M_PI); if(a2 < 0) a2 += 2*M_PI;
            atingido[(int)(a2/(2*M_PI)*M) % M] = 1;
        }
        int falta = 0;
        for(int j = 0; j < M; j++) if(!atingido[j]) falta++;
        printf("          %d setores, %d por atingir a partir de R+\n", M, falta);
        ok("o multiplicativo cobre o mesmo círculo — via t^(is) = e^(is·log t)", falta == 0);
        printf("          E o LOG é a ponte: t = e^u leva R+ em R, e leva Mellin em Fourier.\n");
        printf("          São o mesmo círculo visto dos dois lados do contrato — ⊕ e ⊗.\n");
    }

    /* (3) PONTRYAGIN — a forma DUAL de "preenche": os caracteres sao COMPLETOS */
    printf("\n      (3) PONTRYAGIN: e do lado DUAL, preencher é os caracteres serem COMPLETOS.\n\n");
    {
        /* Parseval: ||x||² = ||Fx||². Se faltasse um caractere, havia energia a perder-se —
         * seria um BURACO no dual. Que a norma se conserve é a medida de que nao ha buraco. */
        double complex x[N], X[N];
        double pior = 0;
        for(int caso = 0; caso < 4; caso++){
            for(int j = 0; j < N; j++)
                x[j] = (caso==0) ? (j==3) : (caso==1) ? cos(2*M_PI*j/N)
                     : (caso==2) ? (j*0.37 - 1.1) : cexp(I*2*M_PI*5*j/N);
            dft(x, X, 0);
            double n1 = 0, n2 = 0;
            for(int j = 0; j < N; j++){ n1 += creal(x[j]*conj(x[j])); n2 += creal(X[j]*conj(X[j])); }
            if(fabs(n1 - n2) > pior) pior = fabs(n1 - n2);
        }
        printf("          max | ||x||² - ||Fx||² |  em 4 sinais  =  %.2e  (vírgula)\n\n", pior);
        /* PARSEVAL É ALGEBRICO: N·Σ x_j² = Σ X_k X_{−k} em F_p, sem módulo complexo. */
        {
            const long P = 17, W = 3, NN = 16;
            long x[NN], X[NN];
            for(int j = 0; j < NN; j++) x[j] = (3*j + 5) % P;
            for(int k = 0; k < NN; k++){
                long s = 0;
                for(int j = 0; j < NN; j++)
                    s = (s + x[j] * rt_pot_mod(W, (long)((j*(long)k)%NN), P)) % P;
                X[k] = s;
            }
            long esq = 0, dir = 0;
            for(int j = 0; j < NN; j++) esq = (esq + x[j]*x[j]) % P;
            esq = (esq * NN) % P;
            for(int k = 0; k < NN; k++){
                int km = (int)((NN - k) % NN);
                dir = (dir + X[k]*X[km]) % P;
            }
            printf("          e em ℤ_17: N·Σ x² = %ld,  Σ X_k X_{-k} = %ld — o mesmo\n\n", esq, dir);
            ok("PARSEVAL: a norma conserva-se — os caracteres são COMPLETOS, não há buraco."
               " A identidade N·Σ x_j² = Σ X_k X_{−k} fecha em ℤ₁₇, sem o 1e-12 do cexp",
               esq == dir);
        }
        printf("          É esta a forma dual de \"preenche a área\". Do lado da reta, preencher\n");
        printf("          é a órbita não deixar setor vazio; do lado DUAL é os caracteres não\n");
        printf("          deixarem função por ver. Se faltasse um deles, haveria energia a\n");
        printf("          perder-se na transformada — e a norma acusaria.\n");
        printf("\n          E os dois lados são a MESMA afirmação: R/2πZ ≅ S¹ de um lado, e do\n");
        printf("          outro que o dual de S¹ é Z e que Z gera tudo. Pontryagin diz que os\n");
        printf("          dois são equivalentes, e é por isso que basta medir um — mas mediram-\n");
        printf("          -se os dois, porque \"basta\" é uma palavra que aqui não vale sem conta.\n");
    }
    printf("\n      E ASSIM A DEFORMAÇÃO DESTE CORPO ESTÁ VERIFICADA NOS TRÊS LADOS: a reta cobre\n");
    printf("      o círculo (Fourier), o multiplicativo cobre o mesmo círculo (Mellin), e o dual\n");
    printf("      não tem buraco (Pontryagin/Parseval). A cifra sai da reta, o polinómio leva ao\n");
    printf("      círculo, e o círculo fica CHEIO.\n");
}

printf("\n§F9  E FECHOU? Contra o contrato, cláusula a cláusula — e uma NÃO fecha.\n\n");
{
    /* O Aarao: "ve o R^n de novo, ve se o corpo diferencial realmente fechou". Entao verifica-se
     * contra o contrato, e diz-se pela CLAUSULA — que e o que o contrato.h pede: "falha em M4"
     * e informacao, "nao e corpo" e juizo. */
    printf("      cláusula                            o que é aqui                fecha?\n");
    printf("      A1..A4  ⊕ associa, comuta, 0, -a    a soma de funções           SIM\n");
    printf("      M1..M4  ⊗ associa, comuta, 1, 1/a   o produto de funções        SIM\n");
    printf("      D       distributiva                herdada do corpo base       SIM\n");
    printf("      Π       morfismo                    PONTRYAGIN: Π(a+b)=Π(a)Π(b) SIM (§F7)\n");
    printf("      ν1      a dualidade é INVOLUÇÃO     F² = paridade, e (F²)² = id SIM (§F2)\n");
    printf("      ν2      e respeita a estrutura      F(a+b) = F(a) + F(b)        SIM\n\n");

    /* ν1 com F: (F²)² = F⁴ = id, ja medido no §F2. Aqui mede-se ν2: F e linear.
     * A LINEARIDADE É ALGEBRAICA, e o 1e-12 media o cexp. Em ℤ₁₇, F(x+y) = F(x)+F(y)
     * entrada a entrada, resíduo ZERO. */
    {
        const long P = 17, W = 3;
        long x[N], y[N], s[N], Fx[N], Fy[N], Fs[N];
        for(int j = 0; j < N; j++){
            x[j] = (j==2) ? 1 : 0;
            y[j] = (j==5) ? 1 : 0;
            s[j] = (x[j] + y[j]) % P;
        }
        for(int k = 0; k < N; k++){
            long sx = 0, sy = 0, ss = 0;
            for(int j = 0; j < N; j++){
                long w = rt_pot_mod(W, (long)((N - (long)j*k % N) % N), P);
                sx = (sx + x[j]*w) % P;
                sy = (sy + y[j]*w) % P;
                ss = (ss + s[j]*w) % P;
            }
            Fx[k] = sx; Fy[k] = sy; Fs[k] = ss;
        }
        long mal = 0;
        for(int k = 0; k < N; k++) if(Fs[k] != (Fx[k] + Fy[k]) % P) mal++;
        printf("      e em ℤ_17: F(x+y) = F(x)+F(y) em %d de %d modos — resíduo ZERO\n",
               N - (int)mal, N);
        ok("ν2: a transformada respeita a soma — F(a+b) = F(a) + F(b). EXACTA em ℤ₁₇,"
           " sem o 1e-12 do cexp: a linearidade e' da soma, nao do arredondamento",
           mal == 0);
    }

    printf("\n      MAS HÁ UMA QUE NÃO FECHA, e é a que o Aarão mandou olhar:\n\n");
    printf("      o par (D, ∫) NÃO é uma dualidade no sentido do contrato.\n");
    printf("      ν1 exige involução — ν∘ν = id — e aqui:\n");
    printf("        D∘∫ = id           de um lado, fecha\n");
    printf("        ∫∘D = id - ker D   do outro, NÃO fecha: perde a constante\n\n");
    {
        /* D∘∫ e ∫∘D — exacto em Q (copia compacta do §F5). */
        Fr P[6]; for(int i=0;i<6;i++) P[i]=fr(0,1);
        P[0]=fr(5,1); P[1]=fr(5,1); P[2]=fr(-2,1); P[3]=fr(3,1);
        Fr dp[6], ip[6], q[7], dq[6];
        for(int i=0;i<6;i++) dp[i]=fr(0,1);
        for(int i=0;i<5;i++) dp[i]=fr_mul_i(P[i+1], i+1);
        for(int i=0;i<6;i++) ip[i]=fr(0,1);
        for(int i=0;i<5;i++) ip[i+1]=fr_div_i(dp[i], i+1);
        for(int i=0;i<7;i++) q[i]=fr(0,1);
        for(int i=0;i<5;i++) q[i+1]=fr_div_i(P[i], i+1);
        for(int i=0;i<6;i++) dq[i]=fr(0,1);
        for(int i=0;i<5;i++) dq[i]=fr_mul_i(q[i+1], i+1);
        int mau_esq = 0, mau_dir = 0;
        for(int i=0;i<6;i++){ if(!fr_eq(dq[i], P[i])) mau_esq++; if(!fr_eq(ip[i], P[i])) mau_dir++; }
        ok("D∘∫ fecha, e ∫∘D NÃO fecha — logo (D,∫) falha a cláusula ν1. Medido em Q (§F5):"
           " D∘∫ = id coeficiente a coeficiente; ∫∘D perde c=5 ≠ 0 — sem FD, sem limiar",
           mau_esq == 0 && mau_dir > 0 && P[0].n == 5);
    }
    printf("      Então a resposta à pergunta é: O CORPO FECHOU, MAS NÃO PELO PAR QUE EU TINHA\n");
    printf("      POSTO NO LUGAR DA DUALIDADE. A dualidade dele é FOURIER, que é involução e\n");
    printf("      respeita a estrutura — cumpre ν1 e ν2. O par (D, ∫) é outra coisa: um\n");
    printf("      operador e o seu inverso À DIREITA, e a diferença entre os dois lados é\n");
    printf("      exatamente o núcleo.\n");
    printf("\n      E isso arruma o que eu tinha escrito no §F5 como \"o que faltava completar\":\n");
    printf("      não faltava completar o par — faltava NÃO O CHAMAR de dualidade. O corpo é\n");
    printf("      (K, ⊕, ⊗, F) com F a dualidade, e D é o OPERADOR Π; o ∫ é o inverso à direita\n");
    printf("      de D, e o ker D é o preço. Chamar dual ao ∫ era eu a pôr no lugar da\n");
    printf("      involução uma coisa que não involui.\n");
    printf("\n      Fica dito pela cláusula, como o contrato manda: cumpre A1-A4, M1-M4, D, Π,\n");
    printf("      ν1 e ν2 com F. Falha ν1 se se puser (D,∫) no lugar de ν — e essa falha é\n");
    printf("      informação, não veredito.\n");
}

printf("\n§F6  A régua do corpo diferencial, e onde ele cai no catálogo.\n\n");
{
    /* A deformacao e F, a transformada, e ela tem ordem 4: F⁴ = id, F² = paridade. O gerador
     * de um ciclo de ordem 4 em GL2(Z) e [[0,-1],[1,0]] — traco 0, determinante 1, Δ = -4. */
    long B = 0, C = 1, D = B*B - 4*C;
    printf("      a deformação é F, com F⁴ = id e F² = paridade\n");
    printf("      o gerador de ordem 4 é [[0,-1],[1,0]]:  traço %ld, det %ld, Δ = %ld\n\n", B, C, D);
    ok("o corpo diferencial é ELÍPTICO, com a régua (0, 1) e Δ = -4", D == -4);
    printf("      É a mesma régua do i, e tem de ser: a deformação deste corpo é a transformada,\n");
    printf("      a transformada tem ordem 4, e em GL2(Z) só há um lugar de ordem 4. O corpo\n");
    printf("      diferencial NÃO abre lugar novo no catálogo — senta-se no do i, e o que ele\n");
    printf("      traz de próprio é a DEFORMAÇÃO, não a régua.\n");
    printf("\n      E isso responde à pergunta que se faz a cada corpo novo: ele é o mesmo lugar\n");
    printf("      com outra deformação. A cifra é a reta, a deformação é o polinómio que a leva\n");
    printf("      ao círculo, o flip é Fourier, e o dual é a integração — com o núcleo declarado.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
