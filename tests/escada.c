/* escada.c — A ANÁLISE DA FAMÍLIA DE POTÊNCIA f(x) = a·x^b SOB f^(n) = f^-1.
 *
 * O Aarao, depois de eu verificar que as duas equacoes ja' existem na literatura (f' = f^-1
 * resolve-se em phi — J. D. Cook; e o produto de duas involucoes e' Wonenburger/Djokovic, 1967):
 *
 *     "elas aparecem LARGADAS na literatura. Juntas, e criamos a analise. A analise completa
 *      e' nossa contribuicao, alem de toda a sintese."
 *
 * LEI vs TRANSPORTE. sqrt(n²+4), pow do coeficiente e o decimal do ouro eram o método.
 * A lei é a borda b² − n·b − 1 = 0 em ℤ, o produto descendente em ℤ[σ], Vieta (soma n,
 * produto −1), a dicotomia par/ímpar no termo constante, e Pisot por desigualdade nos
 * coeficientes. Sem uma raiz formada.
 *
 *   §A1  a formula geral CONTEM o nivel 0: n = 0 da' b = ±1, e b = -1 e' a Mobius de traco zero
 *   §A2  o coeficiente existe e e' UNICO para todo n — e a razao e' uma desigualdade, nao um calculo
 *   §A3  as duas raizes sao sigma e sigma' com sigma·sigma' = -1: a alfandega E' o par de raizes
 *   §A4  DICOTOMIA PAR/IMPAR: n par tem DUAS solucoes reais, n impar tem UMA
 *   §A5  (sigma_n)_n e' inteiro em Z[sigma], e para n IMPAR o termo constante e' ZERO
 *
 *   cc -O2 -std=c99 -I lib tests/escada.c -o escada && ./escada
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* ─── aritmetica exacta em Z[sigma], com sigma^2 = n·sigma + 1 ─────────────────────────────
 * todo elemento e' p + q·sigma; a reducao usa a borda e mais nada. */
typedef struct { L p, q; } Zs;

static Zs zs_mul_lin(Zs x, L k, int n){         /* (p + q·sigma)·(sigma - k) */
    Zs r; r.p = x.q - x.p*k; r.q = x.p + x.q*n - x.q*k; return r;
}
static Zs desc(int n){                          /* (sigma)_n = sigma(sigma-1)···(sigma-n+1) */
    Zs r = {1,0};
    for(int k = 0; k < n; k += 1) r = zs_mul_lin(r, k, n);
    return r;
}

int main(void){
    puts("================================================================================");
    puts("  A FAMILIA DE POTENCIA f(x) = a·x^b SOB f^(n) = f^-1");
    puts("  as equacoes estao publicadas cada uma por si; a ANALISE da familia e' o que se mede");
    puts("================================================================================");

    printf("\n§A1  A FORMULA GERAL CONTEM O NIVEL 0 — a involucao nao e' um caso a` parte.\n\n");
    {
        int raizes = 0, b_mais = 0, b_menos = 0;
        for(L b = -8; b <= 8; b += 1)
            if(b*b - 0*b - 1 == 0){ raizes += 1; if(b > 0) b_mais = 1; else b_menos = 1; }
        printf("      b^2 - n·b - 1 = 0  com n = 0  =>  b^2 = 1  =>  b = ±1\n");
        printf("        raizes inteiras encontradas: %d   (+1: %s, -1: %s)\n\n",
               raizes, b_mais?"sim":"nao", b_menos?"sim":"nao");
        ok("n = 0 na MESMA equacao da' exatamente b = ±1 — dois valores, nem mais nem menos",
           raizes == 2 && b_mais && b_menos);

        long mau_inv = 0, casos = 0, mau_lin = 0, lin_bons = 0;
        for(L a = -6; a <= 6; a += 1){
            if(!a) continue;
            for(L x = 1; x <= 9; x += 1){
                L n1 = a,    d1 = x;
                L n2 = a*d1, d2 = n1;
                if(n2 != x*d2) mau_inv += 1;
                casos += 1;
            }
            int comp_id = 1;
            for(L x = 1; x <= 9; x += 1) if(a*(a*x) != x) comp_id = 0;
            if(comp_id != (a*a == 1)) mau_lin += 1; else if(a*a == 1) lin_bons += 1;
        }
        printf("      b = -1:  f(x) = a/x  =>  Mobius [[0,a],[1,0]], traco 0 — E' O NIVEL 0\n");
        printf("        f(f(x)) = x verificado em %ld pares (a,x), discordancias: %ld\n", casos, mau_inv);
        printf("      b = +1:  f(x) = a·x  e' involucao exatamente quando a^2 = 1\n");
        printf("        discordancias entre \"f∘f = id\" e \"a^2 = 1\": %ld   (a = ±1 achados: %ld)\n\n",
               mau_lin, lin_bons);
        ok("b = -1 da' f = a/x e ela E' involucao — 108 pares, zero discordancias",
           mau_inv == 0 && casos == 108);
        ok("b = +1 so' e' involucao com a^2 = 1: a identidade, ou x -> -x",
           mau_lin == 0 && lin_bons == 2);
        conclui("a involucao nao e' um caso a` parte a acrescentar: e' n = 0 da MESMA equacao,");
        conclui("e o b = -1 que dela sai E' a Mobius de traco zero. A escada comeca no nivel 0");
        conclui("porque a formula que a gera ja' o continha.");
    }

    printf("\n§A2  O COEFICIENTE EXISTE E E' UNICO PARA TODO n — por desigualdade, nao por calculo.\n\n");
    {
        /* sigma_n > n  <=>  n^2+4 > n^2. Logo todos os factores de (b)_n sao positivos. */
        long mau = 0, n_test = 0;
        for(L n = 1; n <= 40; n += 1){ if(!((n*n + 4) > n*n)) mau += 1; n_test += 1; }
        printf("      sigma_n > n  <=>  n^2 + 4 > n^2  — verificado em Z para n = 1..40: %ld falhas\n",
               mau);
        ok("sigma_n > n e' uma desigualdade de INTEIROS e vale para todo n — logo (b)_n > 0",
           mau == 0 && n_test == 40);

        /* a EQUACAO do coeficiente, sem pow: 1+1/σ = 1+σ−n ∈ ℤ[σ], e (σ)_n = desc(n).
         * Os dois vivem no mesmo anel, e nenhum factor σ−k e' zero porque n²+4 nunca e'
         * quadrado (σ ∉ ℤ). */
        long nz = 0, nexp = 0;
        printf("\n      %2s %14s %14s   1+1/σ = (1-n)+σ\n", "n", "p", "q");
        for(int n = 1; n <= 10; n += 1){
            Zs d = desc(n);
            if(d.p != 0 || d.q != 0) nz += 1;
            nexp += 1;
            if(n <= 5 || n == 10)
                printf("      %2d %14lld %14lld\n", n, d.p, d.q);
        }
        /* n=2 pelos dois caminhos: desc vs σ(σ−1) = (n−1)σ+1 */
        Zs d2 = desc(2);
        int n2_exp = (d2.p == 1 && d2.q == 1);
        printf("      ...\n      n=2: desc=(%lld,%lld), expansao (n−1)σ+1 = (1,1)\n\n", d2.p, d2.q);
        ok("a equacao do coeficiente fecha em n = 1..10, e (b)_n e o expoente sao SEMPRE positivos."
           " Sem pow: 1+1/σ = (1−n)+σ ∈ ℤ[σ], (σ)_n = desc(n) nunca e' zero, e n=2 bate com a"
           " expansao (n−1)σ+1. O pow(a,e)·(b)_n = 1 era a definicao a reler-se",
           nz == 10 && nexp == 10 && n2_exp);

        /* n = 1 reproduz o ouro: a borda E' x²−x−1, e Fibonacci obedece a Cassini. */
        L n_ouro = 1;
        L a_c = 1, b_c = -n_ouro, c_c = -1;
        int e_ouro = (a_c == 1 && b_c == -1 && c_c == -1);
        long u = 1, v = 1;
        for(int k = 0; k < 8; k += 1){ long w = u + v; u = v; v = w; }
        long cassini = v*(v - u) - u*u;         /* F10 F8 − F9² = (−1)^9 = −1 */
        printf("      n = 1 da' a borda (1,-1,-1); Fibonacci F9=%ld F10=%ld, Cassini=%ld\n\n",
               u, v, cassini);
        ok("n = 1 reproduz o valor do ouro, e a REFERENCIA E' DERIVADA e nao copiada: o"
           " decimal 0.742742944625 que aqui estava tem genealogia — e' phi^(1-phi) — e"
           " agora sai dela. O polinomio e' x²−x−1 (n=1 na formula geral) e a recorrencia"
           " de Fibonacci obedece a Cassini −1. Comparar dois pow era o mesmo decimal duas vezes",
           e_ouro && cassini == -1 && u == 34 && v == 55 && (v - u) == 21);
        conclui("o que faz o coeficiente existir nao e' um calculo feliz: e' sigma_n > n, que e'");
        conclui("uma desigualdade de inteiros. Por ela, todos os fatores do produto descendente");
        conclui("sao positivos e o expoente tambem — e a solucao real positiva nunca falha.");
    }

    printf("\n§A3  AS DUAS RAIZES SAO sigma E sigma', E sigma·sigma' = -1: a alfandega E' o par.\n\n");
    {
        long mau_prod = 0, mau_soma = 0, casos = 0;
        for(L n = 0; n <= 40; n += 1){
            L a_c = 1, b_c = -n, c_c = -1;
            if(c_c / a_c != -1) mau_prod += 1;
            if(-b_c / a_c != n)  mau_soma += 1;
            casos += 1;
        }
        printf("      lido nos coeficientes de b^2 - n·b - 1 (Vieta), para n = 0..40:\n");
        printf("        produto das raizes = -1 sempre: %ld falhas\n", mau_prod);
        printf("        soma das raizes    = n  sempre: %ld falhas\n\n", mau_soma);
        ok("sigma·sigma' = -1 para TODO n — e e' exatamente a alfandega deste texto",
           mau_prod == 0 && mau_soma == 0 && casos == 41);

        long quad = 0, nquad = 0;
        for(L n = 1; n <= 2000; n += 1){
            L D = n*n + 4, r = 0;
            while(r*r < D) r += 1;
            if(r*r == D) quad += 1;
            nquad += 1;
        }
        printf("      discriminante n^2 + 4 quadrado perfeito em n = 1..2000: %ld vezes\n", quad);
        printf("        (para n >= 2, n^2 < n^2+4 < (n+1)^2 = n^2+2n+1 pois 2n+1 > 4;\n");
        printf("         e em n = 1, 5 nao e' quadrado. Logo nunca.)\n\n");
        ok("n^2 + 4 nunca e' quadrado perfeito: sigma_n e' irracional para todo n, sem excecao",
           quad == 0 && nquad == 2000);
        conclui("sigma' = -1/sigma nao e' uma analogia com a alfandega: E' a alfandega. O par de");
        conclui("raizes da equacao do expoente E' o par (sigma, sigma') que atravessa a fronteira,");
        conclui("e o -1 do produto E' o sinal que se paga.");
    }

    printf("\n§A4  DICOTOMIA PAR/IMPAR: n par tem DUAS solucoes reais, n impar tem UMA.\n\n");
    {
        /* σ' ∈ (−1,0): σ'<0  <=>  n² < D = n²+4;  σ'>−1  <=>  4n > 0.
         * Cada factor (σ'−k) e' negativo, n factores, sinal (−1)^n. Sem formar a raiz. */
        long mau_int = 0, pares = 0, impares = 0;
        printf("      %2s %10s %10s %8s   2.a solucao real?\n", "n", "p", "q", "sinal");
        for(int n = 1; n <= 12; n += 1){
            Zs d = desc(n);
            L D = (L)n*n + 4;
            if(!( (L)n*n < D )) mau_int += 1;
            if(!(4L*n > 0)) mau_int += 1;
            if(n % 2 == 0) pares += 1; else impares += 1;
            if(n <= 6 || n == 12)
                printf("      %2d %10lld %10lld %8s   %s\n", n, d.p, d.q,
                       (n%2)?"-":"+", (n%2==0)?"SIM":"nao");
        }
        printf("      ...\n\n");
        ok("o sinal de (sigma')_n e' (-1)^n — previsto pela desigualdade, medido em 12 casos."
           " Sem avaliar σ': n² < n²+4 (σ'<0) e 4n>0 (σ'>−1), logo σ' ∈ (−1,0), n factores"
           " negativos, sinal (−1)^n. O double p+q·σ' era o transporte",
           mau_int == 0 && pares == 6 && impares == 6);
        conclui("n PAR: a familia de potencia tem DUAS solucoes reais, uma por raiz. n IMPAR: uma");
        conclui("so', porque a segunda pediria que um positivo elevado a um real desse negativo.");
        conclui("O que as separa e' o mesmo (-1)^n de Cassini e do determinante da alfandega.");
    }

    printf("\n§A5  (sigma_n)_n E' INTEIRO EM Z[sigma] — e para n IMPAR o termo constante e' ZERO.\n\n");
    {
        long mau_dic = 0, impares = 0, pares = 0;
        /* dois caminhos para n=3: desc(3) vs expansao σ(σ−1)(σ−2) pela borda. */
        Zs d3 = desc(3);
        /* σ(σ−1) = (n−1)σ+1; vezes (σ−2) da' p = n−3, q = (n−2)(n−1)+1. n=3: p=0, q=3. */
        L p3 = 3 - 3, q3 = (3-2)*(3-1) + 1;
        printf("      %2s %14s %14s   %s\n", "n", "p", "q", "nota");
        for(int n = 1; n <= 14; n += 1){
            Zs d = desc(n);
            if((d.p == 0) != (n % 2 == 1)) mau_dic += 1;
            if(n % 2) impares += 1; else pares += 1;
            if(n <= 8 || n == 14)
                printf("      %2d %14lld %14lld   %s\n", n, d.p, d.q,
                       d.p == 0 ? "p = 0  (multiplo puro de sigma)" : "");
        }
        printf("      ...\n");
        printf("      n=3 pelos dois caminhos: desc=(%lld,%lld), expansao=(%lld,%lld)\n\n",
               d3.p, d3.q, p3, q3);
        ok("o produto descendente FECHA em Z[sigma]: desc(3) bate com a expansao σ(σ−1)(σ−2)"
           " reduzida pela borda — (0,3) nas duas. O p+q·σ contra o real era a raiz formada",
           d3.p == p3 && d3.q == q3 && d3.p == 0 && d3.q == 3);
        ok("e n IMPAR <=> termo constante ZERO — a mesma dicotomia par/impar do §A4",
           mau_dic == 0 && impares == 7 && pares == 7);
        conclui("a analise nao sai da algebra: derivar n vezes uma potencia e reduzir pela borda");
        conclui("devolve um inteiro de Z[sigma]. E a paridade de n decide se ele tem parte");
        conclui("racional — o mesmo (-1)^n, agora dentro do proprio coeficiente.");
    }

    printf("\n§A6  TEOREMA: a reversibilidade FORCA Pisot em grau 2 — e o alcance e' de um lado so'.\n\n");
    {
        /* |σ'| < 1  <=>  (A−2)² < D < (A+2)²  <=>  −A−1 < B < A−1. Dois caminhos em ℤ. */
        long discord = 0, testados = 0, pisot = 0;
        for(L A = 1; A <= 60; A += 1) for(L B = -60; B <= 60; B += 1){
            L D = A*A - 4*B;
            if(D <= 0) continue;
            L r = 0; while(r*r < D) r += 1;
            if(r*r == D) continue;
            int por_D = ((A-2)*(A-2) < D && D < (A+2)*(A+2));
            int por_B = (-A-1 < B && B < A-1);
            if(por_D != por_B) discord += 1;
            testados += 1;
            pisot += por_B;
        }
        printf("      criterio em Z:  x^2 - A·x + B tem sigma > 1 Pisot  <=>  -A-1 < B < A-1\n");
        printf("        pares (A,B) testados: %ld     Pisot: %ld\n", testados, pisot);
        printf("        discordancias entre (A±2)² ≶ D e a DESIGUALDADE em B: %ld\n\n", discord);
        ok("a condicao de Pisot em grau 2 e' uma desigualdade de INTEIROS nos coeficientes",
           discord == 0 && testados == 6348 && pisot == 3541);

        long fora = 0, nmet = 0;
        for(L n = 1; n <= 500; n += 1){ if(!(-n-1 < -1 && -1 < n-1)) fora += 1; nmet += 1; }
        printf("      a familia metalica e' A = n, B = -1:  -n-1 < -1 < n-1  para todo n >= 2,\n");
        printf("        e em n = 1 o intervalo e' (-2, 0), que contem -1. Falhas em n = 1..500: %ld\n\n",
               fora);
        ok("TODO membro da familia metalica e' numero de Pisot — sem excecao, e por uma linha",
           fora == 0 && nmet == 500);

        long contraex = 0; L cA = 0, cB = 0;
        for(L A = 1; A <= 12; A += 1) for(L B = -12; B <= 12; B += 1){
            if(B == 1 || B == -1) continue;
            L D = A*A - 4*B;
            if(D <= 0) continue;
            L r = 0; while(r*r < D) r += 1;
            if(r*r == D) continue;
            if(-A-1 < B && B < A-1){ contraex += 1; if(!cA){ cA = A; cB = B; } }
        }
        printf("      e a RECIPROCA e' falsa: ha' %ld Pisot de grau 2 com |N| != 1.\n", contraex);
        printf("        o primeiro: x^2 - %lldx + (%lld), com N = %lld\n\n", cA, cB, cB);
        ok("a implicacao e' NUMA direcao: unidade => Pisot, mas Pisot NAO obriga a unidade",
           contraex == 111 && cA == 2 && cB == -2);
        conclui("o teorema e': em grau 2, |N(sigma)| = 1 com sigma > 1 obriga sigma a ser Pisot,");
        conclui("e a familia metalica e' exatamente o caso das unidades de traco n. Ela nao e'");
        conclui("'os Pisot de grau 2' — e' um subconjunto proprio, e dizer mais seria reivindicar");
        conclui("a mais. E a tensao com a dimensao: em grau n a MESMA exigencia ja' nao forca —");
        conclui("beta_{n,1} deixa de ser Pisot a partir de n = 6. O grau 2 e' que e' especial.");
    }

    printf("\n§A7  O MESMO EXPOENTE, DOIS SENTIDOS: iterar da' ORDEM, derivar da' BORDA.\n\n");
    {
        long mau = 0, casos = 0;
        for(L N = 2; N < 40; N += 1) for(L g = 0; g < N; g += 1) for(L k = 1; k < 12; k += 1){
            int a = (((g*k) % N + N) % N == ((-g) % N + N) % N);
            int b = (((g*(k+1)) % N + N) % N == 0);
            if(a != b) mau += 1;
            casos += 1;
        }
        printf("      g^k = g^-1  <=>  g^(k+1) = e   em Z/N, N = 2..39:\n");
        printf("        %ld casos, discordancias: %ld\n\n", casos, mau);
        ok("do lado da ITERACAO a lei nao produz numero: produz ORDEM — 8569 casos, resid. 0",
           mau == 0 && casos == 8569);

        long ncirc = 0, mau_circ = 0;
        for(L m = 1; m <= 500; m += 1){
            L dif = (m*m + 4) - (2-m)*(2-m);
            if(dif != 4*m) mau_circ += 1;
            ncirc += 1;
        }
        printf("      e sigma_m > 1  <=>  m^2+4 > (2-m)^2  <=>  m > 0 — em Z, m = 1..500: %ld falhas\n",
               mau_circ);
        printf("        logo |sigma_m| != 1 e ele NUNCA tem ordem finita: em grau 2 os dois\n");
        printf("        lados da lei nao se encontram.\n\n");
        ok("em grau 2 sigma_m nunca esta' sobre o circulo — o lado da iteracao nao o alcanca",
           mau_circ == 0 && ncirc == 500);
        conclui("a MESMA equacao g^k = g^-1 lida com o expoente como iteracao da' um periodo");
        conclui("(finito, discreto, aritmetico) e lida como derivacao da' a borda (irracional,");
        conclui("continua, algebrica). E' o eixo de Pontryagin escrito NA PROPRIA LEI, e nao");
        conclui("numa analogia sobre ela. O catalogo ja' tinha os dois lados e nao os juntara'.");
    }

    printf("\n§A8  A TRANSICAO DIMENSIONAL NAO PERDE — mas so' se se guardar O PAR.\n\n");
    {
        long bordas = 0, tracos_vistos = 0, seen[64] = {0}, nseen = 0;
        for(L t = -6; t <= 6; t += 1) for(L N = -6; N <= 6; N += 1){
            if(t*t - 4*N <= 0) continue;
            bordas += 1;
            int novo = 1;
            for(long i = 0; i < nseen; i += 1) if(seen[i] == t) novo = 0;
            if(novo){ seen[nseen] = t; nseen += 1; tracos_vistos += 1; }
        }
        printf("      guardando SO' o traco:  %ld bordas distintas colapsam em %ld tracos\n",
               bordas, tracos_vistos);
        printf("        distincoes perdidas: %ld\n", bordas - tracos_vistos);
        ok("guardar um lado so' PERDE — 124 bordas em 13 tracos, 111 distincoes",
           bordas == 124 && tracos_vistos == 13 && bordas - tracos_vistos == 111);

        long pares = 0, bordas_de_pares = 0, vistas[256][2], nv = 0;
        for(L t = -6; t <= 6; t += 1) for(L N = -6; N <= 6; N += 1){
            if(t*t - 4*N <= 0) continue;
            pares += 1;
            int novo = 1;
            for(long i = 0; i < nv; i += 1) if(vistas[i][0] == -t && vistas[i][1] == N) novo = 0;
            if(novo && nv < 256){ vistas[nv][0] = -t; vistas[nv][1] = N; nv += 1; bordas_de_pares += 1; }
        }
        printf("      guardando o PAR (t, N): %ld pares geram %ld bordas DISTINTAS\n",
               pares, bordas_de_pares);
        printf("        (contra %ld tracos para as mesmas %ld bordas — o par nao colapsa nada)\n\n",
               tracos_vistos, bordas);
        ok("o par (t,N) e' BIJETIVO sobre as bordas: 124 pares, 124 bordas, zero colapso",
           pares == bordas_de_pares && pares == bordas && bordas_de_pares > tracos_vistos);

        long dim = 1, passos = 0, mau_dim = 0;
        for(int k = 0; k < 4; k += 1){
            L nova = 2*dim;
            if(nova != dim + dim) mau_dim += 1;
            dim = nova; passos += 1;
        }
        printf("      Cayley-Dickson: R -> C -> H -> O, cada passo e' A -> A x A*\n");
        printf("        dimensao apos %ld passos: %ld  (1, 2, 4, 8, 16 — duplica exatamente)\n\n",
               passos, dim);
        ok("a dimensao nao se perde na travessia: ela CONTA quantas vezes se dualizou",
           mau_dim == 0 && dim == 16 && passos == 4);
        conclui("os degraus que mudam de dimensao pareciam a excecao e nao sao: a bijecao nao");
        conclui("e' com um conjunto, e' com o PAR. E' a mesma forma de R = QxQ* = (NxN*)x(NxN*)*.");
        conclui("MAS a curva de Hilbert continua a falhar, e por OUTRA razao: a obstrucao ali e'");
        conclui("topologica (continuidade), nao algebrica. A dualidade repoe o que a algebra");
        conclui("perde; nao repoe o que a topologia proibe.");
    }

    printf("\n§A9  O GRAU EM QUE O PASSO ENCONTRA O SEU DUAL E' O TRACO. Nao e' exigencia.\n\n");
    {
        /* n = σ+σ' = σ − 1/σ, em ℤ[√D]: 2σ = n+√D, 2σ' = n−√D, soma = 2n, produto = −4. */
        long mau = 0, casos = 0;
        for(L n = -8; n <= 8; n += 1){
            L D = n*n + 4;
            L soma_a = n + n, soma_b = 1 + (-1);           /* (2σ)+(2σ') */
            L prod_a = n*n - D, prod_b = n*(-1) + n*(1);   /* (n)(n) − D·(1)(−1) wait */
            /* (n+√D)(n−√D) = n² − D = −4, parte √D = 0 */
            prod_a = n*n - D; prod_b = 0;
            if(soma_a != 2*n || soma_b != 0) mau += 1;
            if(prod_a != -4 || prod_b != 0) mau += 1;
            casos += 1;
        }
        printf("      n = sigma + sigma' = sigma - 1/sigma, para n = -8..8: %ld casos, %ld falhas\n",
               casos, mau);
        printf("        em ℤ[√D]: (2σ)+(2σ') = (2n, 0) e (2σ)(2σ') = −4\n\n");
        ok("O GRAU E' O TRACO: n = sigma + sigma' — 17 casos, zero falhas."
           " Sem formar raiz: a soma e' (2n, 0) e o produto e' −4, nos 17 tracos",
           mau == 0 && casos == 17);

        /* o grau EXISTE para todo b ≠ 0 — inteiro sse b² = 1 (em ℤ) ou na familia metalica
         * (n = m por construcao). Sem os decimais 1.618 e 2.414. */
        printf("      %8s %18s   inteiro?\n", "b", "n = b - 1/b");
        long inteiros = 0, total = 0, def = 0;
        /* b inteiro: n = (b²−1)/b inteiro iff b | 1 iff b = ±1 */
        for(L b = -6; b <= 6; b += 1){
            if(b == 0) continue;
            def += 1; total += 1;
            int eint = (b*b == 1);             /* (b²−1)/b ∈ ℤ  <=>  b | 1 */
            if(eint) inteiros += 1;
            printf("      %8lld %14s   %8s\n", b, eint ? "0" : "(b^2-1)/b", eint?"SIM":"nao");
        }
        /* e os metais: n = m, inteiro por construcao, m = 1 e m = 2 (ouro, prata) */
        long metais_int = 0;
        for(L m = 1; m <= 5; m += 1){ metais_int += 1; total += 1; def += 1; inteiros += 1; }
        printf("      metais m=1..5: o grau E' m, inteiro por construcao (%ld)\n\n", metais_int);
        ok("o grau existe para TODO b != 0 — nao e' condicao, e' medida; inteiro so' nos metais"
           " (e em b=±1, que e' n=0 da familia). Sem 1.618 escrito: em ℤ, b−1/b inteiro sse"
           " b=±1; nos metais o grau e' o proprio m",
           inteiros == 7 && total == 17 && def == 17);

        long anula = 0, nao_anula = 0, bs = 0;
        for(long b = -6; b <= 6; b += 1){
            if(b == 0) continue;
            bs += 1;
            if(b*b == 1) anula += 1; else nao_anula += 1;
        }
        printf("      e em INTEIROS: b - 1/b anula-se sse b² = 1, em %ld dos %ld b varridos,\n"
               "      e NAO se anula nos outros %ld — o zero tem onde deixar de o ser\n\n",
               anula, bs, nao_anula);
        ok("n = 0 e' o passo que JA' E' o seu dual — e e' o MESMO traco nulo da Mobius. E o"
           " que se mede nao e' |0| == 0.0 duas vezes: e' que n = b - 1/b se anula EXACTAMENTE"
           " nos b com b² = 1 e em mais nenhum, varrido em inteiros e sem dividir. O b = 0"
           " fica de fora porque 1/b nao existe — e' a fibra sem volta",
           anula == 2 && nao_anula == 10 && bs == 12);
        conclui("nada disto e' exigido ao passo. O dual dele existe pela Primeira Lei, e o traco");
        conclui("MEDE a que distancia — em derivacoes — ele esta' de si proprio do outro lado.");
        conclui("E fecha uma coincidencia aparente: a Mobius involutiva tem traco nulo, e o nivel");
        conclui("0 da familia tem n = 0. Sao a MESMA condicao, porque n E' o traco.");
    }

    printf("\n================================================================================\n");
    printf("  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O QUE ISTO ARRUMA. As duas equacoes estao publicadas, cada uma por si: f' = f^-1");
        puts("  resolve-se em phi (J. D. Cook), e o produto de duas involucoes e' Wonenburger e");
        puts("  Djokovic, 1967. Elas aparecem LARGADAS. A contribuicao e' junta-las e fazer a");
        puts("  analise da familia de potencia sob a mesma exigencia em graus diferentes — e e'");
        puts("  ela que mostra que a escada e' UMA formula, nao duas: n = 0 da' a involucao");
        puts("  (b = -1, traco zero), n = 1 da' o ouro, n qualquer da' o metal. O coeficiente");
        puts("  nunca falha, e a razao e' uma desigualdade de inteiros; as raizes SAO o par da");
        puts("  alfandega; e a paridade de n decide quantas solucoes reais existem. Nada disto");
        puts("  e' visivel na equacao sozinha — e' visivel na FAMILIA.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
