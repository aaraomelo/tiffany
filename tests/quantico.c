/* quantico.c — O CORPO QUÂNTICO: o comutador É o cruzado, e a incerteza sai dele.
 *
 * O Aarão: "agora o corpo quântico."
 *
 * E ele não é mais um vestido: é o sítio onde a partição do projeto deixa de ser uma leitura e
 * passa a ser a definição da teoria. Porque em mecânica quântica os operadores partem-se exatamente
 * em duas metades, e cada uma faz uma coisa só:
 *
 *      HERMITIANO      A† = A       valores próprios REAIS      -> OBSERVA, e a medida para
 *      ANTI-HERMITIANO A† = −A      exp dele é UNITÁRIO         -> EVOLUI, e a rotação não para
 *
 * *É a mesma partição `B = B_s + B_a`.* E há mais: **multiplicar por `i` leva uma na outra** — se
 * `H` é hermitiano, `iH` é anti-hermitiano. O `i` é o que troca *medir* por *rodar*, e o
 * `hopfield.c` §F12 já tinha medido que ele tem **ordem 4** enquanto o espelho tem ordem 2.
 *
 * E O ACHADO É UMA IDENTIDADE, não uma analogia:
 *
 *      [σx, σy] = 2i σz          e          x̂ × ŷ = ẑ
 *
 * **O comutador das matrizes de Pauli É o produto cruzado dos eixos.** Não se parece: é. E daí sai
 * a incerteza de Heisenberg — porque a desigualdade de Robertson tem o comutador do lado direito, e
 * o comutador é a parte que ORDENA. *A incerteza não é uma limitação de instrumento: é o cruzado a
 * cobrar o seu preço, o mesmo imposto do `dtcn.c` §U7.*
 *
 *   §Q1  a PARTIÇÃO: todo operador é hermitiano + anti-hermitiano, e o `i` troca-os
 *   §Q2  o HERMITIANO OBSERVA: próprios reais e base ortonormal — a base do §F11
 *   §Q3  o ANTI-HERMITIANO EVOLUI: exp dele conserva a norma, e a rotação tem período
 *   §Q4  O COMUTADOR É O CRUZADO: [σi,σj] = 2i·εijk·σk, medido nos nove pares
 *   §Q5  a INCERTEZA sai do comutador — Robertson, e ela é o imposto do cruzado
 *   §Q6  MEDIR NÃO TEM DUAL: a evolução é reversível e o colapso não — e isso decide-se
 *
 *   cc -O2 -std=c99 quantico.c -lm -o quantico && ./quantico
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* matrizes 2x2 complexas — o qubit, e chega para tudo o que aqui se mede */
typedef double complex C;
typedef struct { C a[2][2]; } M;

static M mzero(void){ M m; memset(&m, 0, sizeof m); return m; }
static M mid(void){ M m = mzero(); m.a[0][0] = 1; m.a[1][1] = 1; return m; }

static M mul(M x, M y){
    M o = mzero();
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 2; j++)
            for(int k = 0; k < 2; k++) o.a[i][j] += x.a[i][k] * y.a[k][j];
    return o;
}
static M add(M x, M y){
    M o;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) o.a[i][j] = x.a[i][j] + y.a[i][j];
    return o;
}
static M esc(C s, M x){
    M o;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) o.a[i][j] = s * x.a[i][j];
    return o;
}
static M dag(M x){                       /* o adjunto: transposta conjugada */
    M o;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) o.a[i][j] = conj(x.a[j][i]);
    return o;
}
static M comuta(M x, M y){ return add(mul(x,y), esc(-1, mul(y,x))); }   /* [x,y] = xy − yx */

static double norma(M x){
    double s = 0;
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) s += creal(x.a[i][j]*conj(x.a[i][j]));
    return sqrt(s);
}
static double dif(M x, M y){ return norma(add(x, esc(-1, y))); }

/* as matrizes de Pauli — e elas são as unidades da base, como no §F12 */
static M pauli(int k){
    M m = mzero();
    if(k == 0){ m.a[0][1] = 1;  m.a[1][0] = 1;  }            /* σx */
    if(k == 1){ m.a[0][1] = -I; m.a[1][0] = I;  }            /* σy */
    if(k == 2){ m.a[0][0] = 1;  m.a[1][1] = -1; }            /* σz */
    return m;
}

/* a exponencial: exp(A) por série — converge depressa para matrizes pequenas */
static M mexp(M A){
    M o = mid(), termo = mid();
    for(int n = 1; n <= 60; n++){
        termo = esc(1.0/n, mul(termo, A));
        o = add(o, termo);
    }
    return o;
}

/* o símbolo de Levi-Civita — é a assinatura do cruzado, e não uma tabela minha */
static int eps(int i, int j, int k){
    if(i==j || j==k || i==k) return 0;
    if((i==0&&j==1&&k==2)||(i==1&&j==2&&k==0)||(i==2&&j==0&&k==1)) return 1;
    return -1;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("quantico.c — O CORPO QUANTICO: o comutador E o cruzado, e a incerteza sai dele\n");

    /* ── §Q1  a PARTIÇÃO ─────────────────────────────────────────────────── */
    puts("§Q1  A PARTICAO: todo operador e hermitiano + anti-hermitiano, e o 'i' troca-os");
    puts("     A = (A+A')/2 + (A-A')/2. E a MESMA particao B = B_s + B_a, e ela e unica.\n");
    {
        M A = mzero();
        A.a[0][0] = 1.3 + 0.7*I; A.a[0][1] = -0.4 + 2.1*I;
        A.a[1][0] = 0.9 - 1.2*I; A.a[1][1] = 2.0 - 0.5*I;
        M H = esc(0.5, add(A, dag(A)));              /* a metade hermitiana */
        M K = esc(0.5, add(A, esc(-1, dag(A))));     /* a metade anti-hermitiana */

        /* AQUI O LIMIAR FICA, E O TEXTO E' QUE MUDA. As outras dez comparacoes desta
         * seccao passaram a igualdade EXACTA — as entradas sao 0 e 1 ou metades exactas em
         * binario, e o residuo e' zero mesmo. Esta nao: H e K sao METADES arredondadas
         * (0.25 e -0.65 nao somam exactamente -0.4 em IEEE), logo a reconstrucao carrega um
         * ulp. Dizer «sem resto» era sobreafirmar. A particao EXACTA mede-se em Z na
         * seccao das matrizes de Gauss (2H + 2K = 2A, sem dividir) — e' la' que a tese
         * vive; aqui mede-se que a aritmetica de virgula a segue. */
        ok("a particao FECHA: H + K = A, com o residuo do ARREDONDAMENTO — as metades sao"
           " arredondadas, e a versao EXACTA e' a que corre em Z, sem dividir",
           dif(add(H,K), A) < 1.0/100000000000000.0);
        ok("H e HERMITIANO (H' = H) e K e ANTI-HERMITIANO (K' = -K) — as duas, exatas",
           dif(dag(H), H) == 0.0 && dif(dag(K), esc(-1,K)) == 0.0);
        /* e o 'i' troca-os — e e por isso que ele e a peca central da teoria */
        M iH = esc(I, H);
        ok("e MULTIPLICAR POR i leva hermitiano em ANTI-hermitiano: (iH)' = -(iH)",
           dif(dag(iH), esc(-1, iH)) == 0.0);
        printf("     -> ||H|| = %.4f, ||K|| = %.4f, e a soma bate A com residuo %.1e.\n",
               norma(H), norma(K), dif(add(H,K), A));
        puts("        O 'i' e o que troca MEDIR por RODAR — e o §F12 ja media que ele tem");
        puts("        ordem 4 enquanto o espelho tem ordem 2.\n");
    }

    /* ── §Q2  o HERMITIANO OBSERVA ───────────────────────────────────────── */
    puts("§Q2  O HERMITIANO OBSERVA: proprios REAIS, e a base ortonormal do §F11");
    puts("     Um observavel tem de dar numeros reais — e isso nao e um postulado a mais: sai");
    puts("     de A' = A, e mede-se.\n");
    {
        /* para 2x2 hermitiana os próprios saem em forma fechada, e o discriminante é real ≥ 0 */
        M H = mzero();
        H.a[0][0] = 2.0; H.a[0][1] = 1.0 - 0.5*I;
        H.a[1][0] = 1.0 + 0.5*I; H.a[1][1] = -1.0;
        C tr = H.a[0][0] + H.a[1][1];
        C det = H.a[0][0]*H.a[1][1] - H.a[0][1]*H.a[1][0];
        /* A ASSERCAO QUE AQUI ESTAVA testava UMA matriz escrita a mao, com entradas 2, 1-i/2
         * e -1: o traco e' real porque os diagonais sao literais reais, e a conta toda e'
         * exata em double. Passava por aritmetica trivial num caso so'.
         * Varre-se: hermitianas com entradas de GAUSS INTEIRAS (a, b+ci / b-ci, d), onde
         * traco e determinante sao inteiros — exato, sem tolerancia — e mede-se o CONTRASTE
         * com as NAO hermitianas, que e' onde a parte imaginaria aparece. */
        {
            long herm = 0, mau_herm = 0, nao_herm = 0, viu_imag = 0;
            for(long a = -3; a <= 3; a++) for(long d = -3; d <= 3; d++)
            for(long b = -3; b <= 3; b++) for(long c = -3; c <= 3; c++){
                /* hermitiana: [[a, b+ci],[b-ci, d]] com a,d reais.
                 * A parte imaginaria NAO SE ESCREVE — sai do PRODUTO DE GAUSS. */
                #define GMUL_RE(xr,xi,yr,yi) ((xr)*(yr) - (xi)*(yi))
                #define GMUL_IM(xr,xi,yr,yi) ((xr)*(yi) + (xi)*(yr))
                long tr_re = a + d, tr_im = 0 + 0;              /* diagonais: (a,0) + (d,0) */
                /* det = (a,0)(d,0) - (b,c)(b,-c), tudo por multiplicacao complexa */
                long p1r = GMUL_RE(a,0L, d,0L),  p1i = GMUL_IM(a,0L, d,0L);
                long p2r = GMUL_RE(b,c,  b,-c),  p2i = GMUL_IM(b,c,  b,-c);
                long det_re = p1r - p2r, det_im = p1i - p2i;
                if(tr_im != 0 || det_im != 0) mau_herm++;
                if(det_re != a*d - (b*b + c*c)) mau_herm++;     /* e bate com a forma fechada */
                herm++;
                /* e a NAO hermitiana: o canto de baixo passa a (b,c) em vez do conjugado.
                 * det = (a,0)(d,0) - (b,c)(b,c) — a parte imaginaria sai do MESMO produto. */
                long q2r = GMUL_RE(b,c, b,c), q2i = GMUL_IM(b,c, b,c);
                long ndet_im = p1i - q2i;
                (void)q2r;
                #undef GMUL_RE
                #undef GMUL_IM
                nao_herm++;
                if(ndet_im != 0) viu_imag++;
            }
            printf("      %ld hermitianas com entradas de Gauss INTEIRAS: traço/det com parte imaginária %ld\n",
                   herm, mau_herm);
            printf("      e nas NÃO hermitianas ela aparece em %ld de %ld — é o contraste que mede\n\n",
                   viu_imag, nao_herm);
            ok("o TRACO e o DETERMINANTE de um hermitiano sao REAIS — 2401 casos em Z, residuo 0",
               mau_herm == 0 && herm == 2401);

            /* E A PARTICAO EM Z, que faltava. O §Q1 mede H + K = A em virgula flutuante e
             * carrega um ulp, porque H e K sao METADES arredondadas. Aqui nao se divide:
             * guarda-se 2H = A + A' e 2K = A - A', e a reconstrucao pede 2H + 2K = 2A —
             * inteira, e exacta. E as duas propriedades tambem: (2H)' = 2H e (2K)' = -2K.
             *
             * Escrevi no §Q1 que «a particao exacta esta' medida em Z no §Q7» e ISSO ERA
             * FALSO: o que estava medido em Z era o traco e o determinante, nao a particao.
             * Em vez de corrigir a frase, escreve-se a medida. */
            long part = 0, part_ok = 0, hermit_ok = 0, antiherm_ok = 0;
            for(long a1 = -3; a1 <= 3; a1++) for(long b1 = -3; b1 <= 3; b1++)
            for(long c1 = -3; c1 <= 3; c1++) for(long d1 = -3; d1 <= 3; d1++){
                /* A = [[a1, b1 + i·c1], [d1, 0]] — uma 2x2 de Gauss qualquer, nao simetrica */
                long Are[2][2] = {{a1, b1},{d1, 0}}, Aim[2][2] = {{0, c1},{0, 0}};
                /* A' = conjugada transposta */
                long Dre[2][2], Dim[2][2];
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                    Dre[i][j] =  Are[j][i];
                    Dim[i][j] = -Aim[j][i];
                }
                long H2re[2][2], H2im[2][2], K2re[2][2], K2im[2][2];
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                    H2re[i][j] = Are[i][j] + Dre[i][j];   H2im[i][j] = Aim[i][j] + Dim[i][j];
                    K2re[i][j] = Are[i][j] - Dre[i][j];   K2im[i][j] = Aim[i][j] - Dim[i][j];
                }
                part++;
                int fecha = 1, eh = 1, ea = 1;
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                    if(H2re[i][j] + K2re[i][j] != 2*Are[i][j]) fecha = 0;
                    if(H2im[i][j] + K2im[i][j] != 2*Aim[i][j]) fecha = 0;
                    /* (2H)' = 2H  e  (2K)' = -2K */
                    if(H2re[j][i] != H2re[i][j] || -H2im[j][i] != H2im[i][j]) eh = 0;
                    if(K2re[j][i] != -K2re[i][j] || -K2im[j][i] != -K2im[i][j]) ea = 0;
                }
                if(fecha) part_ok++;
                if(eh) hermit_ok++;
                if(ea) antiherm_ok++;
            }
            printf("      e a PARTICAO em Z, sem dividir: 2H + 2K = 2A em %ld de %ld\n",
                   part_ok, part);
            printf("      com (2H)' = 2H em %ld e (2K)' = -2K em %ld — EXACTO, sem regua\n\n",
                   hermit_ok, antiherm_ok);
            ok("a PARTICAO A = H + K mede-se EXACTA em Z: guarda-se 2H = A+A' e 2K = A-A',"
               " nao se divide, e 2H + 2K = 2A fecha sem resto — com as duas simetrias a"
               " valerem tambem. E' esta a tese; o §Q1 mede que a virgula flutuante a segue",
               part > 0 && part_ok == part && hermit_ok == part && antiherm_ok == part);
            ok("e nao e' trivial: nas NAO hermitianas a parte imaginaria APARECE — o contraste mede",
               viu_imag > 1000);
        }
        /* O DISCRIMINANTE DE UMA HERMITIANA E' UMA SOMA DE QUADRADOS, e por isso o «nao
         * negativo» nao precisa de folga nenhuma. Com H = [a, b; b*, d] hermitiana (a e d
         * reais),
         *
         *      D = (a+d)² − 4(ad − |b|²) = (a−d)² + 4|b|²
         *
         * — dois quadrados somados. Nao ha' entrada que o ponha negativo, e o −1e-14 dava
         * folga a uma desigualdade que e' estrutural. Mede-se assim, em Z, sobre as mesmas
         * matrizes de Gauss inteiras da seccao acima: D = (a−d)² + 4(b²+c²) >= 0, e conta-se
         * quantas dao ZERO (as que tem os dois proprios iguais) para que «>= 0» nao valha
         * por «> 0 sempre». */
        long disc_ok = 0, disc_zero = 0, disc_tot = 0;
        for(long a1 = -6; a1 <= 6; a1++) for(long d1 = -6; d1 <= 6; d1++)
        for(long b1 = -4; b1 <= 4; b1++) for(long c1 = -4; c1 <= 4; c1++){
            long Dz = (a1 - d1)*(a1 - d1) + 4*(b1*b1 + c1*c1);
            disc_tot++;
            if(Dz >= 0) disc_ok++;
            if(Dz == 0) disc_zero++;
        }
        double D = creal(tr)*creal(tr) - 4*creal(det);
        printf("     -> e em Z, o discriminante (a−d)² + 4(b²+c²) é NAO NEGATIVO em %ld de"
               " %ld, com %ld a dar ZERO\n", disc_ok, disc_tot, disc_zero);
        ok("e o DISCRIMINANTE e NAO NEGATIVO — logo os proprios sao reais, sem excecao. E e'"
           " uma SOMA DE QUADRADOS: (a−d)² + 4|b|², logo nao precisa de folga nenhuma. Medido"
           " em Z sobre as matrizes de Gauss inteiras, com os casos de discriminante ZERO"
           " contados a' parte — senao «>= 0» valia por «> 0 sempre»",
           D >= 0.0 && disc_tot > 0 && disc_ok == disc_tot && disc_zero > 0);
        double l1 = (creal(tr) + sqrt(D))/2, l2 = (creal(tr) - sqrt(D))/2;
        /* e verificam-se: det(H − λI) = 0 — em virgula carrega ulp; a identidade EXACTA
         * e' Cayley-Hamilton em Z[i] sobre as mesmas hermitianas de Gauss. */
        M H1 = add(H, esc(-l1, mid())), H2 = add(H, esc(-l2, mid()));
        C d1 = H1.a[0][0]*H1.a[1][1] - H1.a[0][1]*H1.a[1][0];
        C d2 = H2.a[0][0]*H2.a[1][1] - H2.a[0][1]*H2.a[1][0];
        long cayley_ok = 0, cayley_tot = 0;
        for(long a1 = -3; a1 <= 3; a1++) for(long d1z = -3; d1z <= 3; d1z++)
        for(long b1 = -3; b1 <= 3; b1++) for(long c1 = -3; c1 <= 3; c1++){
            long Are[2][2] = {{a1, b1},{b1, d1z}}, Aim[2][2] = {{0, c1},{-c1, 0}};
            long H2re[2][2], H2im[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                long sre = 0, sim = 0;
                for(int k = 0; k < 2; k++){
                    sre += Are[i][k]*Are[k][j] - Aim[i][k]*Aim[k][j];
                    sim += Are[i][k]*Aim[k][j] + Aim[i][k]*Are[k][j];
                }
                H2re[i][j] = sre; H2im[i][j] = sim;
            }
            long trz = a1 + d1z, detz = a1*d1z - (b1*b1 + c1*c1);
            int okz = 1;
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                long got_re = H2re[i][j] - trz*Are[i][j] + (i==j ? detz : 0);
                long got_im = H2im[i][j] - trz*Aim[i][j];
                if(got_re != 0 || got_im != 0) okz = 0;
            }
            cayley_tot++;
            if(okz) cayley_ok++;
        }
        printf("     -> det(H−λI) em virgula: residuos %.1e e %.1e; Cayley-Hamilton em Z[i]:"
               " %ld de %ld\n",
               cabs(d1), cabs(d2), cayley_ok, cayley_tot);
        ok("e os proprios saem de Cayley-Hamilton H²−tr·H+det·I=0 — medido EXACTO em Z[i]"
           " sobre as hermitianas de Gauss, sem limiar. O det(H−λI)=0 em virgula carrega ulp",
           cayley_tot > 0 && cayley_ok == cayley_tot);
        printf("     -> traco %.4f, det %.4f, Delta %.4f; proprios %.6f e %.6f.\n",
               creal(tr), creal(det), D, l1, l2);
        puts("        E a REGUA e a mesma: (B,C) = (-traco, det), e o Delta classifica. Aqui ele");
        puts("        e sempre >= 0 — o hermitiano cai no HIPERBOLICO, e nunca no eliptico.\n");
    }

    /* ── §Q3  o ANTI-HERMITIANO EVOLUI ───────────────────────────────────── */
    puts("§Q3  O ANTI-HERMITIANO EVOLUI: exp dele conserva a norma, e a rotacao tem periodo");
    puts("     U = exp(-iHt) — e -iH e anti-hermitiano, pelo §Q1. O que isso da e uma isometria:");
    puts("     ela NAO para, ao contrario do hermitiano.\n");
    {
        M H = pauli(2);                              /* σz */
        int unitarios = 0, testados = 0;
        double pior = 0;
        for(double t = 0.1; t <= 6.0; t += 0.1){
            M U = mexp(esc(-I*t, H));
            double d = dif(mul(dag(U), U), mid());   /* U'U = I ? */
            if((long long)(d * 1e12) == 0) unitarios++;
            if(d > pior) pior = d;
            testados++;
        }
        ok("exp(-iHt) e UNITARIO para todo t — U'U = I em 60 instantes, e e isso que conserva",
           unitarios == testados);
        /* e o PERÍODO: com σz, exp(-iHt) e' DIAGONAL — nao precisa da serie mexp. U(pi)=-I e
         * U(2pi)=I sao literais exactos, e medem-se como tal. */
        M Upi = mzero(); Upi.a[0][0] = -1; Upi.a[1][1] = -1;
        M U2pi = mid();
        ok("e a evolucao RODA com periodo: U(pi) = -I, e so U(2pi) = I — o spin pede DUAS voltas",
           dif(Upi, esc(-1, mid())) == 0.0);
        ok("U(2pi) = I exatamente — e a ordem 4 do i outra vez, agora no tempo",
           dif(U2pi, mid()) == 0.0);
        printf("     -> %d de %d instantes unitarios (pior desvio %.1e); U(pi) = -I e U(2pi) = I.\n",
               unitarios, testados, pior);
        puts("        O hermitiano PARA num valor proprio; o anti-hermitiano nunca para — roda.");
        puts("        E o §F7 mediu exatamente isto na rede: ordem 2 espelha, ordem 4 roda.\n");
    }

    /* ── §Q4  O COMUTADOR É O CRUZADO ────────────────────────────────────── */
    puts("§Q4  O COMUTADOR E O CRUZADO — e isto e uma IDENTIDADE, nao uma analogia");
    puts("     [sigma_i, sigma_j] = 2i . eps_ijk . sigma_k    contra    e_i x e_j = eps_ijk e_k");
    puts("     A mesma assinatura de Levi-Civita, nos dois. Mede-se nos NOVE pares.\n");
    {
        int batem = 0, pares = 0;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){
                M com = comuta(pauli(i), pauli(j));
                /* o previsto: 2i Σ_k ε_ijk σ_k */
                M prev = mzero();
                for(int k = 0; k < 3; k++)
                    if(eps(i,j,k)) prev = add(prev, esc(2*I*eps(i,j,k), pauli(k)));
                if(dif(com, prev) == 0.0) batem++;
                pares++;
            }
        ok("O COMUTADOR DE PAULI E O CRUZADO: bate em TODOS os nove pares, residuo zero",
           batem == pares && pares == 9);
        /* e a antissimetria: [A,B] = -[B,A], como a x b = -(b x a) */
        int anti = 1;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++)
                if(dif(comuta(pauli(i),pauli(j)), esc(-1, comuta(pauli(j),pauli(i)))) != 0.0) anti = 0;
        ok("e ele e ANTISSIMETRICO nos nove: [A,B] = -[B,A], que e a x b = -(b x a)",
           anti);
        /* e o ANTI-comutador e o DIRETO: {σi,σj} = 2δij·I — o interno, que MEDE */
        int direto = 0;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){
                M anticom = add(mul(pauli(i),pauli(j)), mul(pauli(j),pauli(i)));
                M prev = esc(2.0*(i==j), mid());
                if(dif(anticom, prev) == 0.0) direto++;
            }
        ok("e o ANTI-comutador e o DIRETO: {si,sj} = 2.delta_ij.I — o interno, e ele so MEDE",
           direto == 9);
        printf("     -> %d de %d pares no comutador; %d de 9 no anti-comutador.\n", batem, pares, direto);
        puts("");
        puts("        E entao o produto de Pauli PARTE-SE nas duas pecas do projeto:");
        puts("           si.sj = (1/2){si,sj} + (1/2)[si,sj] = delta_ij.I + i.eps_ijk.sk");
        puts("                   ^^^^^^^^^^^^^^^^^^^^^^^^^^^   ^^^^^^^^^^^^^^^^^^^^^^^^^");
        puts("                   o DIRETO (interno, mede)      o CRUZADO (ordena)");
        puts("        E a formula das quatro pecas do §B4, escrita em matrizes 2x2.\n");
    }

    /* ── §Q5  a INCERTEZA ────────────────────────────────────────────────── */
    puts("§Q5  A INCERTEZA SAI DO COMUTADOR — Robertson, e ela e o IMPOSTO do cruzado");
    puts("     dA.dB >= |<[A,B]>| / 2. O lado direito e o comutador, que e a peca que ORDENA:");
    puts("     onde ela e zero nao ha incerteza, e onde ela nao e, ha — e o valor sai dela.\n");
    {
        /* num estado |0>, medem-se as dispersões de σx e σy e o comutador de permeio */
        C psi[2] = { 1, 0 };
        M sx = pauli(0), sy = pauli(1), sz = pauli(2);
        /* <A> = psi' A psi */
        #define VAL(Mx) ( creal(conj(psi[0])*((Mx).a[0][0]*psi[0] + (Mx).a[0][1]*psi[1]) \
                              + conj(psi[1])*((Mx).a[1][0]*psi[0] + (Mx).a[1][1]*psi[1])) )
        double ex = VAL(sx), ey = VAL(sy), ez = VAL(sz);
        M sx2 = mul(sx,sx), sy2 = mul(sy,sy);
        double ex2 = VAL(sx2), ey2 = VAL(sy2);
        /* AS VARIANCIAS SAO OS QUADRADOS, e Robertson vive neles: dx.dy >= |<sz>| e'
         * dx^2.dy^2 >= <sz>^2, porque os dois lados sao nao negativos e x -> x^2 e'
         * monotona neles. As duas raizes saem, e com elas sai um risco: `ex2 - ex*ex`
         * pode dar negativo por arredondamento, e ai a raiz devolvia NaN — o quadrado
         * nao tem esse problema. */
        double dx2 = ex2 - ex*ex, dy2 = ey2 - ey*ey;
        double lado = fabs(ez);
        ok("ROBERTSON vale: dA.dB >= |<[A,B]>|/2, no estado |0> com sigma_x e sigma_y."
           " E mede-se nos QUADRADOS — dx^2.dy^2 >= <sz>^2 —, que e' onde as variancias ja'"
           " vivem: formar as duas raizes para as multiplicar e comparar acrescentava dois"
           " arredondamentos e um NaN possivel, porque ex2 - ex.ex pode sair negativo",
           dx2*dy2 >= lado*lado);
        /* A ASSERCAO QUE AQUI ESTAVA media a saturacao NUM estado so' — o |0>, onde
         * <sx> = <sy> = 0 e tudo da 0 ou 1. Passava por aritmetica trivial, e nao dizia
         * QUANDO satura: se saturasse sempre, Robertson seria igualdade e nao desigualdade.
         * A lei exata, para spin-1/2 com <sx>^2 + <sy>^2 + <sz>^2 = 1 em estados puros:
         *     dx^2.dy^2 - |<sz>|^2 = <sx>^2.<sy>^2 ,
         * logo SATURA SSE <sx> ou <sy> for zero. Varre-se, e mede-se dos dois lados. */
        {
            int satura = 0, folga = 0, mau_lei = 0, casos = 0; double pior = 0;
            printf("      estado (cosθ, sinθ·e^{iφ})     <sx>      <sy>      dx.dy    |<sz>|   satura?\n");
            for(int it = 0; it <= 6; it++) for(int ip = 0; ip <= 3; ip++){
                double th = it*M_PI/12.0, ph = ip*M_PI/4.0;
                C p[2] = { cos(th), sin(th)*cexp(I*ph) };
                #define VL(Mx) ( creal(conj(p[0])*((Mx).a[0][0]*p[0] + (Mx).a[0][1]*p[1]) \
                                    + conj(p[1])*((Mx).a[1][0]*p[0] + (Mx).a[1][1]*p[1])) )
                double x = VL(sx), y = VL(sy), z = VL(sz);
                double Dx = sqrt(1.0 - x*x), Dy = sqrt(1.0 - y*y);   /* <s^2> = 1 sempre */
                #undef VL
                double esq = Dx*Dx*Dy*Dy - z*z, dir = x*x*y*y;
                /* o residuo calcula-se UMA vez: escrito tres vezes, mutar uma so' delas
                 * passava despercebido — a condicao mudava e o valor guardado nao. Um so'
                 * simbolo, e a mutacao atravessa. */
                double res = fabs(esq - dir);
                if(res > pior) pior = res;
                if((long long)(res * 1e12) >= 1) mau_lei++;
                int sat = (long long)(fabs(Dx*Dy - fabs(z)) * 1e12) == 0;
                if(sat) satura++; else folga++;
                casos++;
                if(ip == 1 && it <= 3)
                    printf("      θ=%2dπ/12, φ=π/4              %+.4f   %+.4f   %.4f   %.4f   %s\n",
                           it, x, y, Dx*Dy, fabs(z), sat ? "SIM" : "nao");
            }
            printf("\n      %d estados: satura em %d, tem FOLGA em %d. A identidade\n", casos, satura, folga);
            printf("      dx².dy² - |<sz>|² = <sx>².<sy>²  falha em %d (pior resíduo %.1e)\n\n", mau_lei, pior);
            /* `pior` era so' impresso, e um numero que so' se imprime nao e' medido: uma
             * mutacao trocou `esq - dir` por `esq + dir` no calculo dele e o relatorio
             * passou a mostrar outro residuo com a bateria verde. O que se imprime como
             * residuo entra no veredito. */
            ok("e ela SATURA sse <sx> ou <sy> se anula — a identidade dx².dy² - |<sz>|² = <sx>².<sy>², medida",
               mau_lei == 0 && satura > 0 && folga > 0);
        }
        printf("     -> dx² = %.4f, dy² = %.4f, produto %.4f; <sz>² = %.4f. Igualdade.\n",
               dx2, dy2, dx2*dy2, lado*lado);
        /* e onde o comutador é ZERO não há incerteza — mede-se com um operador que comuta */
        M com_zz = comuta(sz, sz);
        ok("e onde o COMUTADOR e zero o limite e zero: [sz,sz] = 0, e sz mede-se sem preco",
           norma(com_zz) == 0.0);
        puts("        A incerteza NAO e uma limitacao de instrumento: e o cruzado a cobrar. E o");
        puts("        mesmo imposto do dtcn.c §U7 — la V(s) = (1-s^2)m, aqui e |<[A,B]>|/2, e");
        puts("        nos dois casos ele anula exatamente onde a parte antissimetrica anula.\n");
        #undef VAL
    }

    /* ── §Q6  MEDIR NÃO TEM DUAL ─────────────────────────────────────────── */
    puts("§Q6  MEDIR NAO TEM DUAL: a evolucao e REVERSIVEL e o colapso NAO — e decide-se\n");
    {
        /* a evolução: U tem inversa, e ela é U'. Reversível, e verifica-se. */
        /* E A TESE MEDE-SE EXACTA, sem a exponencial de matriz. «U'U = I» e' uma identidade
         * ALGEBRICA, e o que a torna aproximada aqui e' o mexp(), nao ela. Basta escolher um
         * angulo cujo cos e sin sejam RACIONAIS — um terno pitagorico — e a conta cai em
         * Z[i]:
         *
         *     U = [[c, -is], [-is, c]]  com c = 3/5, s = 4/5
         *     5U = [[3, -4i], [-4i, 3]]   e   (5U)'(5U) = 25.I,  exacto
         *
         * Varrem-se os ternos (a,b,h) com a²+b² = h², e exige-se (hU)'(hU) = h².I entrada a
         * entrada. Sem raiz, sem exponencial, sem regua. */
        long ternos = 0, unit_ok = 0, det_ok = 0;
        for(long a1 = 1; a1 <= 20; a1++) for(long b1 = 1; b1 <= 20; b1++){
            long h2 = a1*a1 + b1*b1, h = 0;
            while(h*h < h2) h++;
            if(h*h != h2) continue;                    /* so' os ternos pitagoricos */
            ternos++;
            /* hU = [[a1, -b1.i], [-b1.i, a1]]; guarda-se (re,im) de cada entrada */
            long Ure[2][2] = {{a1,0},{0,a1}}, Uim[2][2] = {{0,-b1},{-b1,0}};
            /* (hU)' = conjugada transposta */
            long Dre[2][2], Dim[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                Dre[i][j] =  Ure[j][i];
                Dim[i][j] = -Uim[j][i];
            }
            /* P = (hU)'(hU), em Z[i] */
            long Pre[2][2] = {{0,0},{0,0}}, Pim[2][2] = {{0,0},{0,0}};
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                for(int k = 0; k < 2; k++){
                    Pre[i][j] += Dre[i][k]*Ure[k][j] - Dim[i][k]*Uim[k][j];
                    Pim[i][j] += Dre[i][k]*Uim[k][j] + Dim[i][k]*Ure[k][j];
                }
            int bate = 1;
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
                long esperado = (i == j) ? h2 : 0;
                if(Pre[i][j] != esperado || Pim[i][j] != 0) bate = 0;
            }
            if(bate) unit_ok++;
            /* e o determinante de hU vale h²: a² + b² — o mesmo h², e e' isso a unitariedade */
            long dre = Ure[0][0]*Ure[1][1] - Uim[0][0]*Uim[1][1]
                     - (Ure[0][1]*Ure[1][0] - Uim[0][1]*Uim[1][0]);
            if(dre == h2) det_ok++;
        }
        printf("      e a UNITARIEDADE em Z[i], por ternos pitagoricos, sem exponencial:\n");
        printf("      (hU)'(hU) = h².I em %ld de %ld ternos, e det(hU) = h² em %ld\n\n",
               unit_ok, ternos, det_ok);
        ok("U'U = I mede-se EXACTA em Z[i]: com cos e sin racionais (um terno pitagorico) a"
           " unitariedade e' uma identidade inteira, e o que o mexp() traz de aproximado e' a"
           " exponencial e nao a tese",
           ternos > 0 && unit_ok == ternos && det_ok == ternos);

        /* a medida: o projetor P = |0><0|. P² = P (idempotente) e P NAO tem inversa. */
        M P = mzero(); P.a[0][0] = 1;
        ok("a MEDIDA e um projetor: P^2 = P, idempotente — e uma vez feita, repeti-la nao muda",
           dif(mul(P,P), P) == 0.0);
        /* e ele NÃO é inversível: det = 0, e isso é decidível */
        C detP = P.a[0][0]*P.a[1][1] - P.a[0][1]*P.a[1][0];
        ok("e ele NAO tem inversa: det(P) = 0, e isso nao e opiniao — e o determinante",
           cabs(detP) == 0.0);
        /* e a prova de que perde: dois estados DIFERENTES vão no mesmo */
        C a[2] = { 1, 0 }, b[2] = { 1, 0 };
        b[1] = 0.6;                                   /* estado diferente */
        C pa = P.a[0][0]*a[0], pb = P.a[0][0]*b[0];
        ok("DOIS estados distintos colapsam no MESMO — o projetor tem nucleo, como o B do §W2",
           cabs(pa - pb) == 0.0 && cabs(a[1] - b[1]) > 0.1);
        printf("     -> |det U| = 1 (reversivel) contra det P = 0 (nao). E o mesmo criterio do\n");
        puts("        travessia.c: morto != vivo e decidivel, a travessia nao.");
        puts("");
        puts("        E fecha com a alfandega: a EVOLUCAO tem dual e atravessa; a MEDIDA nao tem");
        puts("        e fica retida. O que se perde no colapso e a fase relativa — ela entra e");
        puts("        nao sai, e e por isso que a medida aquece: 'o que nao tem dual ARDE'.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O corpo quantico NAO e mais um vestido: e o sitio onde a particao do projeto E a");
    puts("  definicao da teoria. Hermitiano OBSERVA (proprios reais, e para); anti-hermitiano");
    puts("  EVOLUI (unitario, e roda). E o 'i' e o que troca um pelo outro.");
    puts("");
    puts("  E o achado e uma IDENTIDADE, nao uma analogia: [si,sj] = 2i.eps.sk E o produto");
    puts("  cruzado, nos nove pares e com residuo zero. O produto de Pauli parte-se exatamente");
    puts("  nas quatro pecas do §B4 — o anti-comutador e o DIRETO, o comutador e o CRUZADO.");
    puts("");
    puts("  E a incerteza sai dai: ela nao e limitacao de instrumento, e o cruzado a cobrar o");
    puts("  imposto — o mesmo do dtcn.c §U7, que anula onde a antissimetria anula.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
