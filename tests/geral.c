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
 *   GENERALIZA:  a companion (grau n vira sistema n×n), a órbita A^k, o regime pelos menores
 *                de Hurwitz, a raiz de multiplicidade pelo gcd(p,p′), e o traço/determinante
 *                como soma/produto das raízes — sem formar raiz nenhuma.
 *
 *   E GENERALIZA TAMBÉM a classificação — mas pela ASSINATURA (r,s), e não pelo Δ: em
 *                grau 2 a assinatura cabe no SINAL de um número (e é isso o Δ, e são as três
 *                classes); em grau n precisa do par, e há n/2 + 1 casos. É o mesmo corpo — nem
 *                sequer outra roupa, só outra interpretação. Ver §G5, que eu ia escrever ao
 *                contrário e o Aarão corrigiu.
 *
 *   §G1  a companion de grau n devolve o característico — traço das potências vs Newton
 *   §G2  as somas de potências são INTEIROS conhecidos (Lucas, ciclotómico), e a raiz dupla
 *        é gcd(p,p′) ≠ 1 — sem formar z
 *   §G3  a órbita da companion: Cayley–Hamilton e A^{a+b}=A^a A^b, em ℤ
 *   §G4  o regime sai dos coeficientes: Hurwitz + factor x²+c em ℤ[x]
 *   §G5  e a classificação GENERALIZA — pela assinatura (r,s), contada por Sturm
 *
 * LEI vs TRANSPORTE. Durand–Kerner, |p(z)| 1e-9, e^{At} contra RK4 e max Re(λ) com 1e-9 eram
 * o método. A lei é Newton nos coeficientes (Lucas no ouro, P_k=6 se 6|k em x^6−1), gcd em
 * ℤ[x] para a raiz dupla, a órbita da companion, Hurwitz, e Sturm — sem uma raiz e sem
 * <math.h>.
 *
 *   cc -O2 -std=c99 -I lib tests/geral.c -o geral && ./geral
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"
#include "poli.h"

static long caminhos = 0, concordam = 0;   /* Bareiss: a divisão tem de ser exacta — a vigia
                                            * do resto vive agora na reta.h, `rt_bareiss_resto` */
#define GMAX 12                 /* a caixa desta máquina — da caixa, e não da matemática */

/* p(x) = c[0] + c[1]x + … + c[n]x^n, com c[n] = 1 (mónico). Coeficientes INTEIROS. */
typedef struct { long c[GMAX+1]; int n; } Poli;

static void newton_pk(const Poli *p, long *PN, int kmax){
    int n = p->n;
    PN[0] = n;
    for(int q = 1; q <= kmax; q++){
        long v = 0;
        if(q <= n){
            for(int j = 1; j < q; j++) v += (-p->c[n-j]) * PN[q-j];
            v += (long)q * (-p->c[n-q]);
        } else {
            for(int j = 1; j <= n; j++) v += (-p->c[n-j]) * PN[q-j];
        }
        PN[q] = v;
    }
}

/* grau do gcd(p, p′) em ℤ[x]. 0 = livre de quadrados; >0 = raiz múltipla. −1 = não coube. */
static int grau_gcd_pp(const Poli *p){
    Pz a, b, g;
    memset(&a, 0, sizeof a);
    memset(&b, 0, sizeof b);
    a.n = p->n;
    for(int k = 0; k <= p->n; k++) a.a[k] = p->c[k];
    if(p->n < 1) return 0;
    b.n = p->n - 1;
    for(int k = 0; k <= b.n; k++) b.a[k] = (long)(k + 1) * p->c[k + 1];
    int passos = 0;
    if(!pz_mdc(a, b, &g, &passos)) return -1;
    return g.n;
}

/* p(A) = 0 por Horner, em ℤ. */
static int p_de_A_zero(const Poli *p, const long *A){
    int n = p->n;
    long R[GMAX*GMAX], T[GMAX*GMAX];
    rt_identidade(R, n);
    for(int k = n - 1; k >= 0; k--){
        rt_mul_mat(A, R, n, T);
        for(int i = 0; i < n*n; i++) R[i] = T[i];
        for(int i = 0; i < n; i++) R[i*n + i] += p->c[k];
    }
    for(int i = 0; i < n*n; i++) if(R[i]) return 0;
    return 1;
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
    for(int k = 0; k < 5; k++){
        int n = ps[k].n;
        long cl[GMAX], A[GMAX*GMAX];
        for(int j = 0; j < n; j++) cl[j] = -ps[k].c[j];
        rt_companheira(cl, n, A);
        long tr = rt_traco(A, n);

        long M[GMAX*GMAX];
        for(int i = 0; i < n*n; i++) M[i] = A[i];
        long det = rt_det_bareiss(M, n, n);

        long esp_tr = -ps[k].c[n-1], esp_det = ((n%2)?-1:1)*ps[k].c[0];

        /* DOIS CAMINHOS que não se tocam:
         *   (M) tr(A^k)   — multiplicando MATRIZES, sem olhar para os coeficientes
         *   (N) P_k       — a recorrência de NEWTON sobre os COEFICIENTES, sem matriz
         * Se a companion não realizasse p, os dois separavam-se. */
        long PN[16];
        newton_pk(&ps[k], PN, 6);
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

printf("\n§G2  As somas de potências são INTEIROS conhecidos — sem formar raiz nenhuma.\n\n");
{
    Poli ouro   = { { -1, -1, 1 }, 2 };
    Poli ciclo  = { { -1, 0, 0, 0, 0, 0, 1 }, 6 };
    Poli dupla  = { { 1, 0, 2, 0, 1 }, 4 };           /* (x²+1)² */
    Poli ps[6] = {
        { { -1, -1, 1 }, 2 },
        { { -1, -1, 0, 1 }, 3 },
        { { 1, 0, 2, 0, 1 }, 4 },
        { { -1, 0, 0, 0, -1, 1 }, 5 },
        { { -1, 0, 0, 0, 0, 0, 1 }, 6 },
        { { 1, 2, 3, 4, 3, 2, 1, 0, 1 }, 8 },
    };
    const char *nm[6] = { "x²-x-1", "x³-x-1", "x⁴+2x²+1", "x⁵-x⁴-1", "x⁶-1", "grau 8 qualquer" };
    const long LUCAS[7] = { 0, 1, 3, 4, 7, 11, 18 };
    const long CICLO[7] = { 0, 0, 0, 0, 0, 0, 6 };

    long PN[16];
    newton_pk(&ouro, PN, 6);
    int lucas_ok = 1;
    printf("      ouro x²-x-1     P_1..P_6 =");
    for(int q = 1; q <= 6; q++){
        printf(" %+ld", PN[q]);
        if(PN[q] != LUCAS[q]) lucas_ok = 0;
    }
    printf("   (Lucas)\n");

    newton_pk(&ciclo, PN, 6);
    int ciclo_ok = 1;
    printf("      x^6-1           P_1..P_6 =");
    for(int q = 1; q <= 6; q++){
        printf(" %+ld", PN[q]);
        if(PN[q] != CICLO[q]) ciclo_ok = 0;
    }
    printf("   (6 se 6|k)\n\n");

    ok("no ouro os P_k sao 1, 3, 4, 7, 11, 18 — a sucessao de LUCAS, tirada dos"
       " coeficientes sem raiz nenhuma; e em x^6-1 sao 0,0,0,0,0,6 — a soma das"
       " potencias das raizes sextas da unidade, um inteiro que se conhece antes"
       " de haver raiz",
       lucas_ok && ciclo_ok);

    printf("      polinómio          grau gcd(p,p′)    raiz múltipla?\n");
    int gcd_ok = 1;
    for(int k = 0; k < 6; k++){
        int g = grau_gcd_pp(&ps[k]);
        int quer_dupla = (k == 2);                    /* só (x²+1)² */
        printf("      %-18s %-16d %s\n", nm[k], g, g > 0 ? "sim" : "nao");
        if(g < 0) gcd_ok = 0;
        else if(quer_dupla && g <= 0) gcd_ok = 0;
        else if(!quer_dupla && g != 0) gcd_ok = 0;
    }
    printf("\n");
    ok("a raiz DUPLA e gcd(p, p') de grau > 0 em Z[x]: (x^2+1)^2 tem, e os outros"
       " cinco — livres de quadrados — nao. Perto de uma raiz dupla o polinomio e'"
       " plano e |p(z)| ficava pequeno sem a raiz estar certa; o gcd nao se deixa"
       " enganar por isso, e nao forma z",
       gcd_ok);
    printf("      A partir de grau 5 não há fórmula (Abel-Ruffini), e é por isso que se lê o\n");
    printf("      corpo ℚ[x]/(p) em vez de resolver — o método muda, o critério não. As raízes\n");
    printf("      irracionais não se calculam: declara-se o corpo, e lá dentro elas têm nome.\n");
}

printf("\n§G3  A órbita da companion: Cayley–Hamilton e A^{a+b}=A^a A^b, em ℤ.\n\n");
{
    /* e^{At} contra RK4 era o transporte. A lei é a órbita discreta: p(A)=0 determina
     * a recorrência, e a soma no expoente vira produto de matrizes. */
    Poli ps[5] = {
        { { -1, -1, 1 }, 2 },
        { { -1,  0, 0, 1 }, 3 },
        { { -1, -1, 0, 1 }, 3 },
        { {  1,  0, 2, 0, 1 }, 4 },
        { { -1,  0, 0, 0, -1, 1 }, 5 },
    };
    const char *nomes[5] = { "ouro grau 2", "x³-1", "plástico grau 3", "x⁴+2x²+1", "furo grau 5" };
    int mal_ch = 0, mal_sg = 0;
    printf("      sistema             n   p(A)=0   A^5[0,0]   A^2 A^3[0,0]\n");
    for(int k = 0; k < 5; k++){
        int n = ps[k].n;
        long cl[GMAX], A[GMAX*GMAX];
        for(int j = 0; j < n; j++) cl[j] = -ps[k].c[j];
        rt_companheira(cl, n, A);
        int ch = p_de_A_zero(&ps[k], A);
        if(!ch) mal_ch++;

        long P5[GMAX*GMAX], P2[GMAX*GMAX], P3[GMAX*GMAX], Prod[GMAX*GMAX];
        rt_pot_mat(A, n, 5, P5);
        rt_pot_mat(A, n, 2, P2);
        rt_pot_mat(A, n, 3, P3);
        rt_mul_mat(P2, P3, n, Prod);
        int sg = 1;
        for(int i = 0; i < n*n; i++) if(P5[i] != Prod[i]) sg = 0;
        if(!sg) mal_sg++;
        printf("      %-19s %d   %-7s  %+8ld   %+ld\n",
               nomes[k], n, ch ? "sim" : "nao", P5[0], Prod[0]);
    }
    printf("\n");
    ok("Cayley-Hamilton: p(A)=0 na companion, em INTEIROS — a recorrencia que determina"
       " a orbita sem formar e^{At}",
       mal_ch == 0);
    ok("e A^{2+3}=A^2 A^3 em todos: a soma no expoente vira produto, que e' o morfismo"
       " (N,+) -> (matrizes,x), medido em Z e sem uma exponencial",
       mal_sg == 0);
    printf("      A solução é a órbita A^k x₀ em QUALQUER dimensão — é a única coisa desta\n");
    printf("      página que não muda nada ao subir de grau. E outra vez são dois caminhos\n");
    printf("      que têm de fechar no mesmo: Cayley–Hamilton não sabe do semigrupo, e o\n");
    printf("      semigrupo não sabe dos coeficientes.\n");
}

printf("\n§G4  O regime sai dos coeficientes — Hurwitz, sem uma raiz.\n\n");
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
    /* CRISTAL  todas as raizes com Re < 0  ⟺  os menores de HURWITZ sao todos > 0
       BORDA    ha raiz no eixo imaginario  ⟺  p tem factor x² + c com c > 0
       CAOS     o resto
       Routh-Hurwitz e divisao exacta em Z[x] — sem Durand-Kerner e sem limiar 1e-9. */
    long conc = 0, casos_rh = 0;
    printf("      polinómio            menores de Hurwitz    factor x²+c   regime (Z)\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int n = t[k].p.n;
        long a[GMAX+1];
        for(int i = 0; i <= n; i++) a[i] = t[k].p.c[n-i];   /* do maior grau ao menor */
        int todos_pos = rt_hurwitz_est(a, n);
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
    printf("\n      a rota inteira acerta %ld de %ld\n\n", conc, casos_rh);
    ok("o regime SAI DOS COEFICIENTES, SEM UMA RAIZ: cristal e' os menores de HURWITZ"
       " todos positivos, borda e' haver factor x²+c com c>0 (divisao exacta em Z[x], a"
       " mesma do §M10 do matricial), e caos e' o resto. O max Re(λ) do Durand-Kerner"
       " com um limiar de 1e-9 a decidir os tres casos era um regime julgado por uma"
       " regua de nove casas",
       conc == casos_rh && casos_rh == 7);
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

    struct { const char *nome; Poli p; int r, s; } t[] = {
        { "x² - x - 1  (ouro)",   { { -1, -1, 1 }, 2 },          2, 0 },
        { "x² + 1      (o i)",    { { 1, 0, 1 }, 2 },            0, 1 },
        { "x³ - x - 1  (plástico)",{ { -1, -1, 0, 1 }, 3 },      1, 1 },
        { "x⁴ + 1",               { { 1, 0, 0, 0, 1 }, 4 },      0, 2 },
        { "x⁴ - 1",               { { -1, 0, 0, 0, 1 }, 4 },     2, 1 },
        { "x⁵ - x⁴ - 1 (o furo)", { { -1, 0, 0, 0, -1, 1 }, 5 }, 1, 2 },
    };
    /* A ASSINATURA CONTA-SE POR STURM. O que aqui estava era |Im(z)| < 1e-9 — uma régua
     * escolhida por mim a decidir se um número é real. A pergunta responde-se em INTEIROS,
     * sem calcular raiz nenhuma. Os seis polinómios são livres de quadrados, logo
     * distintas = todas. */
    int mau2 = 0, sturm_casos = 0;
    printf("\n      polinómio                 (r,s) Sturm     o que é\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        int n2 = t[k].p.n;
        Pol pp; pol_zera(&pp); pp.n = n2;
        for(int i2 = 0; i2 <= n2; i2++){ pp.p[i2] = t[k].p.c[i2]; pp.q[i2] = 1; }
        int r_st = pol_sturm_reais(pp);
        int s_st = (r_st >= 0) ? (n2 - r_st)/2 : -1;
        printf("      %-25s (%d, %d)          %s\n", t[k].nome, r_st, s_st,
               n2 == 2 ? (s_st ? "elíptico" : "hiperbólico") : "e em grau 2 chamar-se-ia assim");
        if(r_st != t[k].r || s_st != t[k].s) mau2++;
        sturm_casos++;
    }
    printf("\n");
    ok("a assinatura sai por STURM — em INTEIROS e sem calcular raiz nenhuma — e bate a"
       " esperada nos seis, de grau 2 a 5. O que aqui estava contava as raizes reais por"
       " |Im(z)| < 1e-9, que e' uma regua minha a decidir se um numero e' real",
       mau2 == 0 && sturm_casos == 6);
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
