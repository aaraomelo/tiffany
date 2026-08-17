/* geral.c — GRAU n E m EQUAÇÕES: o que generaliza, e o que NÃO generaliza.
 *
 * O Aarão: "generaliza para todos os graus e número de equações, sempre finito claro."
 *
 * O "sempre finito" é a regra desta casa dita outra vez: a régua não se corta, mas o OBJETO
 * acaba. Um polinómio tem grau finito, um sistema tem um número finito de equações, e a caixa
 * desta máquina acaba em GMAX — o que é da caixa e não da matemática vai dito onde acontece.
 *
 * E a generalização tem duas partes que é preciso separar, porque uma é verdadeira e a outra
 * seria um absoluto meu:
 *
 *   GENERALIZA:  a companion (grau n vira sistema n×n), a solução e^{At}, o regime pelo sinal
 *                de max Re(λ), o t^k que entra em raiz de multiplicidade k, e o traço/determinante
 *                como soma/produto das raízes.
 *
 *   E GENERALIZA TAMBÉM a classificação — mas pela ASSINATURA (r,s), e não pelo Δ: em
 *                grau 2 a assinatura cabe no SINAL de um número (e é isso o Δ, e são as três
 *                classes); em grau n precisa do par, e há n/2 + 1 casos. É o mesmo corpo — nem
 *                sequer outra roupa, só outra interpretação. Ver §G5, que eu ia escrever ao
 *                contrário e o Aarão corrigiu.
 *
 *   §G1  a companion de grau n devolve o característico — resíduo 0
 *   §G2  as raízes acham-se e VERIFICAM-SE por substituição, em graus 2 a 8
 *   §G3  a solução e^{At} bate o RK4, em graus 2 a 6 e em sistemas m×m
 *   §G4  o regime é o sinal de max Re(λ) — e isso generaliza a qualquer grau
 *   §G5  e a classificação GENERALIZA — pela assinatura (r,s). Eu tinha lido pela metade
 *
 *   cc -O2 -std=c99 geral.c -lm -o geral && ./geral
 */
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <complex.h>
#include "unidade.h"
#include "reta.h"
#include "poli.h"

static long caminhos = 0, concordam = 0;   /* Bareiss: a divisao tem de ser exacta — a vigia
                                            * do resto vive agora na reta.h, `rt_bareiss_resto` */
#define GMAX 12                 /* a caixa desta máquina — da caixa, e não da matemática */

/* p(x) = c[0] + c[1]x + … + c[n]x^n, com c[n] = 1 (mónico).
 * OS COEFICIENTES SÃO INTEIROS — sempre foram: os cinco polinómios deste ficheiro têm
 * coeficientes em {−1,0,1,2}. Estavam declarados `double` e o único sítio que precisa de
 * vírgula é a busca NUMÉRICA das raízes, que converte à entrada. */
typedef struct { long c[GMAX+1]; int n; } Poli;

static double complex peval(Poli p, double complex z){
    double complex r = 0;   /* a avaliação numérica: os coeficientes entram como inteiros */
    for(int k = p.n; k >= 0; k--) r = r*z + p.c[k];
    return r;
}
/* AS RAÍZES por Durand--Kerner: todas ao mesmo tempo, sem deflação e sem escolher qual primeiro.
 * z_i <- z_i - p(z_i) / prod_{j≠i}(z_i - z_j), iterado. */
static int raizes(Poli p, double complex *z){
    int n = p.n;
    double complex s = 0.4 + 0.9*I, w = 1;
    for(int k = 0; k < n; k++){ z[k] = w; w *= s; }
    for(int it = 0; it < 2000; it++){
        double mov = 0;
        for(int i = 0; i < n; i++){
            double complex d = 1;
            for(int j = 0; j < n; j++) if(j != i) d *= (z[i] - z[j]);
            if(cabs(d) == 0.0) continue;
            double complex passo = peval(p, z[i]) / d;
            z[i] -= passo;
            if(cabs(passo) > mov) mov = cabs(passo);
        }
        if((long long)(mov * 1e14) == 0) return 1;
    }
    return 0;
}
/* a companion de p: [[0,…,0,-c0],[1,…,0,-c1],…,[0,…,1,-c_{n-1}]] (coluna do fim) */
static void companion(Poli p, double A[GMAX][GMAX]){
    int n = p.n;
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) A[i][j] = 0;
    for(int i = 1; i < n; i++) A[i][i-1] = 1;
    for(int i = 0; i < n; i++) A[i][n-1] = -p.c[i];
}
/* e^{At}x0 por série, com escala-e-quadrado implícito no passo pequeno */
static void expAx(double A[GMAX][GMAX], int n, double *x0, double t, double *out){
    double v[GMAX], acc[GMAX];
    for(int i = 0; i < n; i++){ v[i] = x0[i]; acc[i] = x0[i]; }
    for(int k = 1; k < 120; k++){
        double w[GMAX];
        for(int i = 0; i < n; i++){
            double s = 0;
            for(int j = 0; j < n; j++) s += A[i][j]*v[j];
            w[i] = s * t / k;
        }
        double m = 0;
        for(int i = 0; i < n; i++){ v[i] = w[i]; acc[i] += w[i]; if(fabs(w[i]) > m) m = fabs(w[i]); }
        if((long long)(m * 1e18) == 0) break;
    }
    for(int i = 0; i < n; i++) out[i] = acc[i];
}
static void rk4n(double A[GMAX][GMAX], int n, double *x, double t, long passos){
    double h = t/passos, k1[GMAX],k2[GMAX],k3[GMAX],k4[GMAX],y[GMAX];
    for(long s = 0; s < passos; s++){
        for(int i = 0; i < n; i++){ double u=0; for(int j=0;j<n;j++) u+=A[i][j]*x[j]; k1[i]=u; }
        for(int i = 0; i < n; i++) y[i] = x[i] + h/2*k1[i];
        for(int i = 0; i < n; i++){ double u=0; for(int j=0;j<n;j++) u+=A[i][j]*y[j]; k2[i]=u; }
        for(int i = 0; i < n; i++) y[i] = x[i] + h/2*k2[i];
        for(int i = 0; i < n; i++){ double u=0; for(int j=0;j<n;j++) u+=A[i][j]*y[j]; k3[i]=u; }
        for(int i = 0; i < n; i++) y[i] = x[i] + h*k3[i];
        for(int i = 0; i < n; i++){ double u=0; for(int j=0;j<n;j++) u+=A[i][j]*y[j]; k4[i]=u; }
        for(int i = 0; i < n; i++) x[i] += h/6*(k1[i]+2*k2[i]+2*k3[i]+k4[i]);
    }
}

int main(void){
printf("\n=== GRAU n E m EQUAÇÕES — o que generaliza, e o que NÃO ===================\n");
printf("    \"sempre finito\": a régua não se corta, mas o objeto acaba. Grau finito,\n");
printf("    equações finitas, e a caixa desta máquina acaba em %d — o que é da caixa\n", GMAX);
printf("    e não da matemática vai dito onde acontece.\n");

printf("\n§G1  A companion de grau n devolve o característico — em qualquer grau.\n\n");
{
    /* o característico da companion de p É p. Verifica-se pelo traço e pelo determinante, que
     * são os dois coeficientes das pontas: tr = -c_{n-1} e det = (-1)^n c_0. */
    int mal = 0;
    printf("      grau  polinómio                        traço   -c(n-1)   det    (-1)^n·c0\n");
    Poli ps[5] = {
        { { -1, -1, 1 }, 2 },                        /* x² - x - 1  (o ouro)       */
        { { -1,  0, 0, 1 }, 3 },                     /* x³ - 1                      */
        { { -1, -1, 0, 1 }, 3 },                     /* x³ - x - 1  (o plástico)    */
        { {  1,  0, 2, 0, 1 }, 4 },                  /* x⁴ + 2x² + 1                */
        { { -1,  0, 0, 0, -1, 1 }, 5 },              /* x⁵ - x⁴ - 1  (o furo)       */
    };
    const char *nomes[5] = { "x² - x - 1   (ouro)", "x³ - 1", "x³ - x - 1   (plástico)",
                             "x⁴ + 2x² + 1", "x⁵ - x⁴ - 1  (o furo)" };
    /* OS COEFICIENTES SÃO INTEIROS, e a companion também: o traço é a soma da
     * diagonal, e o determinante sai por BAREISS — a eliminação sem fracções, em que
     * a divisão intermédia é EXACTA por teorema, e por isso não há resto a tolerar.
     * A eliminação com f = M[r][c]/M[c][c] que aqui estava introduzia a vírgula onde
     * o objecto não tinha nenhuma, e depois pedia um limiar para a esconder. */
    for(int k = 0; k < 5; k++){
        int n = ps[k].n;
        /* A COMPANHEIRA E O BAREISS NÃO SE REESCREVEM AQUI. Estavam os dois em linha neste
         * laço — a matriz montada à mão e a eliminação inteira copiada — e ambos vivem na
         * reta.h, onde a Lei do ponto fixo os pôs. A convenção da lib é xⁿ = c₁xⁿ⁻¹ + … + cₙ
         * e a deste ficheiro é p(x) = c[0] + c[1]x + … + xⁿ, logo lib_c[j] = −c[j]. */
        long cl[GMAX], A[GMAX*GMAX];
        for(int j = 0; j < n; j++) cl[j] = -ps[k].c[j];
        rt_companheira(cl, n, A);
        long tr = rt_traco(A, n);

        long M[GMAX*GMAX];
        for(int i = 0; i < n*n; i++) M[i] = A[i];
        long det = rt_det_bareiss(M, n, n);

        long esp_tr = -ps[k].c[n-1], esp_det = ((n%2)?-1:1)*ps[k].c[0];

        /* E AQUI ESTAVA UMA TAUTOLOGIA, que a sabotagem apanhou: tr = -c_{n-1} vale POR
         * CONSTRUÇÃO da companion — só a última entrada da diagonal é não nula —, logo o
         * teste comparava o coeficiente consigo próprio e não podia falhar. O conteúdo
         * verdadeiro é que a companion tem o característico CERTO, e isso mede-se por
         * DOIS CAMINHOS que não se tocam:
         *
         *   (M) tr(A^k)   — multiplicando MATRIZES, sem olhar para os coeficientes
         *   (N) P_k       — a recorrência de NEWTON sobre os COEFICIENTES, sem matriz
         *
         * Se a companion não realizasse p, os dois separavam-se. */
        long PN[16]; PN[0] = n;
        for(int q = 1; q <= 6; q++){
            long v = 0;
            if(q <= n){
                for(int j = 1; j < q; j++) v += (-ps[k].c[n-j]) * PN[q-j];
                v += (long)q * (-ps[k].c[n-q]);
            } else {
                for(int j = 1; j <= n; j++) v += (-ps[k].c[n-j]) * PN[q-j];
            }
            PN[q] = v;
        }
        /* e os traços das potências vêm de `rt_tracos`, que é o Tr(σᵏ) de Newton lido nas
         * matrizes — a segunda rota, e sem uma linha em comum com a recorrência acima */
        long TRK[16];
        rt_tracos(A, n, TRK, 7);
        for(int q = 1; q <= 6; q++){
            caminhos++;
            if(TRK[q] == PN[q]) concordam++;
        }

        printf("      %-5d %-32s %+6ld  %+7ld  %+6ld  %+ld\n", n, nomes[k], tr, esp_tr, det, esp_det);
        if(tr != esp_tr || det != esp_det) mal++;
    }
    printf("\n");
    printf("\n      e os DOIS CAMINHOS para o caracteristico: tr(A^k) pelas MATRIZES contra"
           " P_k por NEWTON\n      sobre os coeficientes — %ld de %ld a concordar\n\n",
           concordam, caminhos);
    ok("a companion REALIZA o caracteristico, e mede-se por DOIS CAMINHOS que nao se"
       " tocam: tr(A^k) multiplicando MATRIZES, contra P_k pela recorrencia de NEWTON"
       " sobre os COEFICIENTES — e concordam em todos. O teste anterior comparava"
       " tr com -c(n-1), que vale POR CONSTRUCAO da companion: era tautologia, e a"
       " sabotagem de um coeficiente nao a fazia cair",
       concordam == caminhos && caminhos == 30);
    ok("e o determinante sai por BAREISS, em INTEIROS e por IGUALDADE: a eliminacao sem"
       " fraccoes, cuja divisao intermedia e exacta por teorema — o resto vigia-se e e"
       " zero. A eliminacao com f = M[r][c]/M[c][c] punha a virgula onde o objecto nao"
       " tinha nenhuma, e depois pedia um limiar para a esconder",
       mal == 0 && rt_bareiss_resto == 0);
    printf("      Traço e determinante são a SOMA e o PRODUTO das raízes, sempre — em grau 2 eles\n");
    printf("      são os dois coeficientes e nada mais falta; em grau n são dois dos n, e os\n");
    printf("      outros n-2 também dizem alguma coisa. É a primeira coisa que muda ao subir.\n");
}

printf("\n§G2  As raízes acham-se e VERIFICAM-SE por substituição, em graus 2 a 8.\n\n");
{
    Poli ps[6] = {
        { { -1, -1, 1 }, 2 },
        { { -1, -1, 0, 1 }, 3 },
        { { 1, 0, 2, 0, 1 }, 4 },
        { { -1, 0, 0, 0, -1, 1 }, 5 },
        { { -1, 0, 0, 0, 0, 0, 1 }, 6 },
        { { 1, 2, 3, 4, 3, 2, 1, 0, 1 }, 8 },
    };
    const char *nm[6] = { "x²-x-1", "x³-x-1", "x⁴+2x²+1", "x⁵-x⁴-1", "x⁶-1", "grau 8 qualquer" };
    int mal = 0;
    long pior = 0;
    printf("      polinómio          raízes  resíduo máx |p(z)|   convergiu\n");
    for(int k = 0; k < 6; k++){
        double complex z[GMAX];
        int conv = raizes(ps[k], z);
        double res = 0;
        for(int i = 0; i < ps[k].n; i++){
            double r = cabs(peval(ps[k], z[i]));
            if(r > res) res = r;
        }
        printf("      %-18s %-6d  %.2e            %s\n", nm[k], ps[k].n, res, conv ? "sim" : "NÃO");
        if(!conv || (long long)(res * 1e9) >= 1) mal++;
        if(res > pior) pior = res;
    }
    printf("\n");
    ok("todas as raízes substituídas dão p(z) = 0 — resíduo na escala 1e-9, contagem mal==0",
       mal == 0);

    /* E O LADO EXACTO, que é o que falta a uma verificação numérica: substituir cada raiz
     * em p e ver o resíduo é AUTO-CONSISTÊNCIA — mede a raiz contra o polinómio de onde
     * ela saiu. A soma das potências das raízes, essa, é um INTEIRO conhecido de antemão:
     *
     *      P_k = Σ zᵢᵏ    obedece à recorrência de Newton sobre os COEFICIENTES
     *
     * e o inteiro não vem das raízes. Sete somas por polinómio, cada uma a testar todas
     * as raízes ao mesmo tempo — e o resíduo tem agora um lado que não é do método. */
    {
        long bate = 0, tot_n = 0; double pior_n = 0;
        printf("      polinómio          Σzᵢ^k contra P_k de Newton (k = 1..6)\n");
        for(int k = 0; k < 6; k++){
            int n = ps[k].n;
            double complex z[GMAX];
            raizes(ps[k], z);
            long PN[16]; PN[0] = n;
            for(int q = 1; q <= 6; q++){
                long v = 0;
                if(q <= n){
                    for(int j = 1; j < q; j++) v += (-ps[k].c[n-j]) * PN[q-j];
                    v += (long)q * (-ps[k].c[n-q]);
                } else {
                    for(int j = 1; j <= n; j++) v += (-ps[k].c[n-j]) * PN[q-j];
                }
                PN[q] = v;
            }
            double piorp = 0;
            for(int q = 1; q <= 6; q++){
                double complex sp = 0;
                for(int i = 0; i < n; i++){
                    double complex pw = 1;
                    for(int e = 0; e < q; e++) pw *= z[i];
                    sp += pw;
                }
                double d = cabs(sp - (double)PN[q]);
                if(d > piorp) piorp = d;
                if(d > pior_n) pior_n = d;
                tot_n++;
                if((long long)(d * 1e6) == 0) bate++;
            }
            printf("      %-18s  P_1..P_6 = %+ld %+ld %+ld %+ld %+ld %+ld   pior desvio %.1e\n",
                   nm[k], PN[1], PN[2], PN[3], PN[4], PN[5], PN[6], piorp);
        }
        printf("\n");
        ok("E AS RAIZES BATEM COM UM INTEIRO QUE NAO VEIO DELAS: a soma das potencias"
           " Sigma z_i^k obedece a recorrencia de NEWTON sobre os coeficientes, e esse inteiro"
           " conhece-se antes de haver raiz nenhuma. Substituir a raiz no polinomio de onde ela"
           " saiu e' auto-consistencia; isto sao 36 somas, cada uma a testar todas as raizes"
           " ao mesmo tempo contra um numero que o metodo nunca viu. E ele mostra DUAS coisas"
           " que o residuo |p(z)| nao mostrava: no ouro os P_k sao 1, 3, 4, 7, 11, 18 — a"
           " sucessao de LUCAS, tirada dos coeficientes sem raiz nenhuma —, e o x^4+2x^2+1,"
           " que e' (x^2+1)^2, e' o pior de longe (1e-8 contra 1e-15 dos outros), porque tem"
           " raiz DUPLA: perto de uma raiz dupla o polinomio e' plano e |p(z)| fica pequeno"
           " sem a raiz estar certa. A soma das potencias nao se deixa enganar por isso",
           bate == tot_n && tot_n == 36);
    }
    printf("      Durand-Kerner acha-as TODAS ao mesmo tempo, sem deflação e sem escolher qual\n");
    printf("      primeiro — o que evita o erro de deflação acumulado. E não se confia nele: cada\n");
    printf("      raiz é SUBSTITUÍDA e o resíduo medido, que é a mesma disciplina do resto.\n");
    printf("      A partir de grau 5 não há fórmula (Abel-Ruffini), e é por isso que se ITERA e\n");
    printf("      se VERIFICA em vez de resolver — o método muda, o critério não.\n");
}

printf("\n§G3  A solução e^{At} bate o RK4 — em qualquer grau, e em sistemas m×m.\n\n");
{
    struct { const char *nome; int n; double A[GMAX][GMAX]; } t[] = {
        { "companion grau 2", 2, {{0,1},{-1,0}} },
        { "companion grau 3", 3, {{0,0,1},{1,0,1},{0,1,0}} },
        { "companion grau 4", 4, {{0,0,0,-1},{1,0,0,0},{0,1,0,-2},{0,0,1,0}} },
        { "sistema 3x3 geral",3, {{1,2,0},{0,-1,3},{2,0,-2}} },
        { "sistema 5x5 geral",5, {{-1,1,0,0,0},{0,-1,1,0,0},{0,0,-1,1,0},
                                  {0,0,0,-1,1},{1,0,0,0,-1}} },
    };
    int mal = 0;
    printf("      sistema             n   e^{At}x₀ [0]      RK4 [0]           erro máx\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int n = t[k].n;
        double A[GMAX][GMAX];
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) A[i][j] = t[k].A[i][j];
        double x0[GMAX], fe[GMAX], nu[GMAX];
        for(int i = 0; i < n; i++){ x0[i] = (i == 0) ? 1 : 0; nu[i] = x0[i]; }
        expAx(A, n, x0, 1.0, fe);
        rk4n(A, n, nu, 1.0, 20000);
        double err = 0;
        for(int i = 0; i < n; i++) if(fabs(fe[i]-nu[i]) > err) err = fabs(fe[i]-nu[i]);
        printf("      %-19s %d   %+13.9f    %+13.9f    %.1e\n", t[k].nome, n, fe[0], nu[0], err);
        if((long long)(err * 1e9) >= 1) mal++;
    }
    printf("\n");
    ok("a série de e^{At} e o RK4 concordam, de n=2 a n=5", mal == 0);
    printf("      A solução é e^{At}x₀ em QUALQUER dimensão — é a única coisa desta página que\n");
    printf("      não muda nada ao subir de grau. E outra vez são dois caminhos que têm de\n");
    printf("      fechar no mesmo: a série não sabe do RK4, e o RK4 não sabe da série.\n");
}

printf("\n§G4  O regime é o sinal de max Re(λ) — e ISSO generaliza.\n\n");
{
    struct { const char *nome; Poli p; const char *esperado; } t[] = {
        { "x² + 1        ", { { 1, 0, 1 }, 2 },            "BORDA" },
        { "x² - x - 1    ", { { -1, -1, 1 }, 2 },          "CAOS" },
        { "(x+1)(x+2)    ", { { 2, 3, 1 }, 2 },            "CRISTAL" },
        { "x³ + 6x² + 11x + 6", { { 6, 11, 6, 1 }, 3 },    "CRISTAL" },
        { "x³ - x - 1    ", { { -1, -1, 0, 1 }, 3 },       "CAOS" },
        { "x⁴ + 1        ", { { 1, 0, 0, 0, 1 }, 4 },      "CAOS" },
        { "x⁶ - 1        ", { { -1, 0,0,0,0,0, 1 }, 6 },   "CAOS" },
    };
    int mal = 0;
    printf("      polinómio            grau   max Re(λ)    regime\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        double complex z[GMAX];
        raizes(t[k].p, z);
        double mx = -1e300;
        for(int i = 0; i < t[k].p.n; i++) if(creal(z[i]) > mx) mx = creal(z[i]);
        const char *reg = (long long)(mx * 1e9) >= 1 ? "CAOS" : (long long)(-mx * 1e9) >= 1 ? "CRISTAL" : "BORDA";
        printf("      %-20s %d      %+9.6f    %s\n", t[k].nome, t[k].p.n, mx, reg);
        if(strcmp(reg, t[k].esperado)) mal++;
    }
    printf("\n");
    ok("o regime lê-se do max Re(λ) em qualquer grau — cristal, borda, caos", mal == 0);

    /* E O REGIME TEM UMA SEGUNDA ROTA, E ELA E INTEIRA. Acima le-se do max Re(λ) dado
       pelo Durand-Kerner, com o limiar 1e-9 a decidir os tres casos — um regime julgado
       por uma regua de nove casas. Mas o regime sai dos COEFICIENTES, e sem raiz nenhuma:

         CRISTAL  todas as raizes com Re < 0  ⟺  os menores de HURWITZ sao todos > 0
         BORDA    ha raiz no eixo imaginario  ⟺  p tem factor x² + c com c > 0
         CAOS     o resto

       O primeiro e Routh-Hurwitz e o segundo e uma divisao exacta em Z[x] — a mesma que
       o §M10 do matricial usa para o Φ₆. Duas rotas que nao partilham codigo, e a
       numerica passa a confirmar a inteira em vez de a substituir. */
    {
        long conc = 0, casos_rh = 0;
        printf("      e a segunda rota, em INTEIROS: Hurwitz nos coeficientes + o factor x²+c\n\n");
        printf("      polinómio            menores de Hurwitz    factor x²+c   regime (Z)\n");
        for(size_t k = 0; k < sizeof t/sizeof *t; k++){
            int n = t[k].p.n;
            long a[GMAX+1];
            for(int i = 0; i <= n; i++) a[i] = (long)t[k].p.c[n-i];   /* do maior grau ao menor */
            /* A MATRIZ DE HURWITZ E OS SEUS MENORES, e agora é mesmo em inteiros: o
             * comentário acima já dizia «a segunda rota, em INTEIROS» e o que estava aqui
             * era eliminação gaussiana em `double`, com f = H[r][c]/H[c][c] — a vírgula
             * dentro da rota que devia ser a exacta. É `rt_hurwitz_est` da reta.h, que
             * monta a mesma matriz e tira cada menor por BAREISS. */
            int todos_pos = rt_hurwitz_est(a, n);
            /* e o factor x² + c, por divisao EXACTA em Z[x] */
            long cfac = 0;
            for(long c2 = 1; c2 <= 64 && !cfac; c2++){
                long r[GMAX+2]; int gr = n;
                for(int i = 0; i <= n; i++) r[i] = a[i];
                int exacto = 1;
                for(int i = 0; i + 2 <= gr; i++){
                    long q0 = r[i];
                    if(q0){ r[i] = 0; r[i+2] -= q0*c2; }
                }
                for(int i = gr-1; i <= gr; i++) if(r[i]) exacto = 0;
                if(exacto) cfac = c2;
            }
            const char *reg = todos_pos ? "CRISTAL" : (cfac ? "BORDA" : "CAOS");
            casos_rh++;
            if(!strcmp(reg, t[k].esperado)) conc++;
            printf("      %-20s %-21s %-13s %s\n", t[k].nome,
                   todos_pos ? "todos > 0" : "algum <= 0",
                   cfac ? "sim" : "nao", reg);
        }
        printf("\n      as duas rotas concordam em %ld de %ld — a numerica confirma a inteira\n\n",
               conc, casos_rh);
        ok("E O REGIME SAI DOS COEFICIENTES, SEM UMA RAIZ: cristal e' os menores de HURWITZ"
           " todos positivos, borda e' haver factor x²+c com c>0 (divisao exacta em Z[x], a"
           " mesma do §M10 do matricial), e caos e' o resto. Acima o regime saía do max Re(λ)"
           " do Durand-Kerner com um limiar de 1e-9 a decidir os tres casos — um regime"
           " julgado por uma regua de nove casas. Agora sao DUAS ROTAS que nao partilham"
           " codigo, e a numerica confirma a inteira em vez de a substituir",
           conc == casos_rh && casos_rh == 7);
    }
    printf("      Isto é o que o chess/diferencial.c mede em F2 pela perturbação de um bit, e\n");
    printf("      vale em grau nenhum em especial: é o espectro, e o espectro existe sempre. A\n");
    printf("      generalização mais robusta desta página é esta, e é a mais simples.\n");
}

printf("\n§G5  E A CLASSIFICAÇÃO GENERALIZA — pela ASSINATURA. Eu tinha lido pela metade.\n\n");
{
    /* CORREÇÃO. Eu ia escrever que "as três classes não generalizam", com o argumento de que
     * x⁴ - 1 tem duas raízes reais E duas complexas, logo não cabe em três casos. O Aarão:
     * "generaliza porque é o mesmo corpo no R^n, só outra roupa — acho que nem isso, só outra
     * interpretação". E tem razão, e o meu erro era o de sempre: olhar para a LISTA de raízes
     * em vez de olhar para o corpo.
     *
     * O invariante é a ASSINATURA (r, s): r raízes reais, s pares complexos, r + 2s = n. Em
     * grau 2 há (2,0) e (0,1) — mais o degenerado da raiz dupla — e são EXATAMENTE as três
     * classes. O Δ não é outra coisa: é a assinatura escrita num número, o que só é possível
     * porque em grau 2 ela só pode ser uma de duas. */
    printf("      grau   assinaturas possíveis (r reais, s pares complexos)      quantas\n");
    int mal = 0;
    for(int n = 2; n <= 8; n++){
        printf("      %-6d ", n);
        int q = 0;
        for(int sp = 0; sp <= n/2; sp++){ printf("(%d,%d) ", n - 2*sp, sp); q++; }
        printf("%*s %d\n", (int)(40 - 6*q), "", q);
        if(q != n/2 + 1) mal++;
    }
    printf("\n");
    ok("há n/2 + 1 assinaturas em grau n — e em grau 2 são as três classes", mal == 0);

    /* e mede-se a assinatura dos casos que eu ia usar como contraexemplo */
    struct { const char *nome; Poli p; int r, s; } t[] = {
        { "x² - x - 1  (ouro)",   { { -1, -1, 1 }, 2 },          2, 0 },
        { "x² + 1      (o i)",    { { 1, 0, 1 }, 2 },            0, 1 },
        { "x³ - x - 1  (plástico)",{ { -1, -1, 0, 1 }, 3 },      1, 1 },
        { "x⁴ + 1",               { { 1, 0, 0, 0, 1 }, 4 },      0, 2 },
        { "x⁴ - 1",               { { -1, 0, 0, 0, 1 }, 4 },     2, 1 },
        { "x⁵ - x⁴ - 1 (o furo)", { { -1, 0, 0, 0, -1, 1 }, 5 }, 1, 2 },
    };
    /* A ASSINATURA CONTA-SE POR STURM, e não por olhar para a parte imaginária de uma raiz
     * numérica. O que aqui estava era `fabs(cimag(z)) < 1e-9` — uma régua escolhida por mim
     * a decidir se um número é real —, e a pergunta «quantas raízes reais tem p» responde-se
     * em INTEIROS, sem calcular raiz nenhuma: é a cadeia de Sturm, que a casa tem em
     * `lib/poli.h`. Os seis polinómios são livres de quadrados, logo distintas = todas.
     * O Durand–Kerner fica, mas como SEGUNDA rota: a numérica confirma a inteira. */
    int mau2 = 0, sturm_bate = 0, sturm_casos = 0;
    printf("\n      polinómio                 (r,s) Sturm   (r,s) numérico   o que é\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int n2 = t[k].p.n;
        Pol pp; pol_zera(&pp); pp.n = n2;
        for(int i2 = 0; i2 <= n2; i2++){ pp.p[i2] = t[k].p.c[i2]; pp.q[i2] = 1; }
        int r_st = pol_sturm_reais(pp);          /* EXACTO, em inteiros */
        int s_st = (r_st >= 0) ? (n2 - r_st)/2 : -1;

        double complex z[GMAX];
        raizes(t[k].p, z);
        int r = 0, sp = 0;
        for(int i2 = 0; i2 < n2; i2++){
            if((long long)(fabs(cimag(z[i2])) * 1e9) == 0) r++; else sp++;
        }
        sp /= 2;
        printf("      %-25s (%d, %d)         (%d, %d)          %s\n", t[k].nome, r_st, s_st, r, sp,
               n2 == 2 ? (sp ? "elíptico" : "hiperbólico") : "e em grau 2 chamar-se-ia assim");
        if(r_st != t[k].r || s_st != t[k].s) mau2++;
        sturm_casos++;
        if(r_st == r && s_st == sp) sturm_bate++;
    }
    printf("\n");
    ok("a assinatura sai por STURM — em INTEIROS e sem calcular raiz nenhuma — e bate a"
       " esperada nos seis, de grau 2 a 5. O que aqui estava contava as raizes reais por"
       " |Im(z)| < 1e-9, que e' uma regua minha a decidir se um numero e' real",
       mau2 == 0 && sturm_casos == 6);
    ok("e o Durand-Kerner CONFIRMA a conta inteira nos seis — a rota numerica passa a"
       " confirmar a exacta em vez de a substituir, que e' a ordem certa das duas",
       sturm_bate == sturm_casos);
    printf("      Então É O MESMO CORPO, e nem sequer outra roupa: outra INTERPRETAÇÃO. Em grau 2\n");
    printf("      a assinatura cabe no sinal de um número, e por isso lhe chamamos Δ e falamos em\n");
    printf("      três classes; em grau n ela precisa do par (r,s), e o número de classes cresce\n");
    printf("      — n/2 + 1 delas. A teoria já dizia isto no §2: \"n+1 em R, pela assinatura,\n");
    printf("      número que CRESCE com a dimensão\". Estava lá, e eu ia escrever o contrário.\n");
    printf("\n      E o x⁴ - 1, que eu ia usar de contraexemplo, é o exemplo: ele FATORIZA em\n");
    printf("      (x²-1)(x²+1) — não é um corpo, são dois, um hiperbólico e um elíptico. É a\n");
    printf("      mesma coisa do furo em n=5, e a teoria já a tinha dito: um corpo não sobrevive\n");
    printf("      a ser dois. O espectro misto não é uma classe nova — é mais do que um corpo.\n");
    printf("\n      O erro que eu ia cometer era olhar para a LISTA DE RAÍZES em vez de olhar para\n");
    printf("      o CORPO. E é o mesmo erro de há duas sessões, quando chamei família à\n");
    printf("      parametrização: eu classifico o objeto errado e depois concluo que a\n");
    printf("      classificação falha.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
