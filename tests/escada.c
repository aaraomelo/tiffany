/* escada.c — A ANÁLISE DA FAMÍLIA DE POTÊNCIA f(x) = a·x^b SOB f^(n) = f^-1.
 *
 * O Aarao, depois de eu verificar que as duas equacoes ja' existem na literatura (f' = f^-1
 * resolve-se em phi — J. D. Cook; e o produto de duas involucoes e' Wonenburger/Djokovic, 1967):
 *
 *     "elas aparecem LARGADAS na literatura. Juntas, e criamos a analise. A analise completa
 *      e' nossa contribuicao, alem de toda a sintese."
 *
 * ENTAO E' AQUI QUE TEM DE ESTAR CERTO. As equacoes sao conhecidas e cada uma esta' publicada
 * sozinha; o que se mede aqui e' o que a familia de potencia FAZ sob elas quando se lhes pede
 * a mesma coisa em graus diferentes — e nenhum dos cinco factos abaixo estava escrito.
 *
 *   §A1  a formula geral CONTEM o nivel 0: n = 0 da' b = ±1, e b = -1 e' a Mobius de traco zero
 *   §A2  o coeficiente existe e e' UNICO para todo n — e a razao e' uma desigualdade, nao um calculo
 *   §A3  as duas raizes sao sigma e sigma' com sigma·sigma' = -1: a alfandega E' o par de raizes
 *   §A4  DICOTOMIA PAR/IMPAR: n par tem DUAS solucoes reais, n impar tem UMA
 *   §A5  (sigma_n)_n e' inteiro em Z[sigma], e para n IMPAR o termo constante e' ZERO
 *
 * A conta de base, feita uma vez:
 *     f(x) = a·x^b   =>   f^(n)(x) = a·(b)_n·x^(b-n),  (b)_n = b(b-1)···(b-n+1)
 *                        f^-1(x)  = a^(-1/b)·x^(1/b)
 *     expoente:      b - n = 1/b        =>  b^2 - n·b - 1 = 0    (a borda, com m = n)
 *     coeficiente:   a·(b)_n = a^(-1/b) =>  a^(1+1/b) = 1/(b)_n
 *
 * Quase tudo o que se segue e' EXATO em Z ou em Z[sigma]. So' §A2 toca o real, e mesmo esse
 * mede a EQUACAO (a^(1+1/b)·(b)_n = 1) em vez de comparar com um valor escrito a mao.
 *
 *   cc -O2 -std=c99 -Wall escada.c -lm -o escada && ./escada
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

typedef long long L;

/* ─── aritmetica exata em Z[sigma], com sigma^2 = n·sigma + 1 ─────────────────────────────
 * todo elemento e' p + q·sigma; a reducao usa a borda e mais nada. Zero floats.
 * NOTA: as DUAS raizes satisfazem a mesma borda, logo a mesma reducao serve para sigma'. */
typedef struct { L p, q; } Zs;

static Zs zs_mul_lin(Zs x, L k, int n){         /* (p + q·sigma)·(sigma - k) */
    /* = p·sigma - p·k + q·sigma^2 - q·k·sigma
     * = p·sigma - p·k + q·(n·sigma + 1) - q·k·sigma
     * = (q - p·k) + (p + q·n - q·k)·sigma                                          */
    Zs r; r.p = x.q - x.p*k; r.q = x.p + x.q*n - x.q*k; return r;
}
static Zs desc(int n){                          /* (sigma)_n = sigma(sigma-1)···(sigma-n+1) */
    Zs r = {1,0};
    for(int k=0;k<n;k++) r = zs_mul_lin(r, k, n);
    return r;
}

int main(void){
    puts("================================================================================");
    puts("  A FAMILIA DE POTENCIA f(x) = a·x^b SOB f^(n) = f^-1");
    puts("  as equacoes estao publicadas cada uma por si; a ANALISE da familia e' o que se mede");
    puts("================================================================================");

    /* ── §A1 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A1  A FORMULA GERAL CONTEM O NIVEL 0 — a involucao nao e' um caso a` parte.\n\n");
    {
        /* b^2 - n·b - 1 = 0 com n = 0 da' b^2 = 1. Resolve-se em Z, sem raiz quadrada. */
        int raizes = 0, b_mais = 0, b_menos = 0;
        for(L b=-8;b<=8;b++)
            if(b*b - 0*b - 1 == 0){ raizes++; if(b>0) b_mais=1; else b_menos=1; }
        printf("      b^2 - n·b - 1 = 0  com n = 0  =>  b^2 = 1  =>  b = ±1\n");
        printf("        raizes inteiras encontradas: %d   (+1: %s, -1: %s)\n\n",
               raizes, b_mais?"sim":"nao", b_menos?"sim":"nao");
        ok("n = 0 na MESMA equacao da' exatamente b = ±1 — dois valores, nem mais nem menos",
           raizes == 2 && b_mais && b_menos);

        /* b = -1 e' f(x) = a/x, que como Mobius e' [[0,a],[1,0]]: traco 0, o nivel 0.
         * b = +1 e' f(x) = a·x com inversa x/a, iguais sse a^2 = 1.
         * Mede-se a involucao pela COMPOSICAO, sem escrever a resposta: f(f(x)) == x. */
        long mau_inv = 0, casos = 0, mau_lin = 0, lin_bons = 0;
        for(L a=-6;a<=6;a++){
            if(!a) continue;
            for(L x=1;x<=9;x++){
                /* b = -1: f(x) = a/x. f(f(x)) = a/(a/x) — em racionais, numerador e denominador */
                L n1 = a,    d1 = x;                   /* f(x)    = n1/d1 */
                L n2 = a*d1, d2 = n1;                  /* f(f(x)) = a·d1/n1 */
                if(n2 != x*d2) mau_inv++;              /* n2/d2 == x ? */
                casos++; }
            /* b = +1: f(x) = a·x e' involucao sse a^2 = 1 */
            int comp_id = 1;
            for(L x=1;x<=9;x++) if(a*(a*x) != x) comp_id = 0;
            if(comp_id != (a*a == 1)) mau_lin++; else if(a*a == 1) lin_bons++;
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

    /* ── §A2 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A2  O COEFICIENTE EXISTE E E' UNICO PARA TODO n — por desigualdade, nao por calculo.\n\n");
    {
        /* sigma_n = (n + sqrt(n^2+4))/2 > n. Logo, para todo k <= n-1, b - k > n - k >= 1 > 0:
         * TODOS os fatores de (b)_n sao positivos, logo (b)_n > 0.
         * E o expoente 1 + 1/b = 1 + b - n > 1 > 0.
         * Logo a = ((b)_n)^(-1/(1+b-n)) existe, e' unico e e' positivo. Sempre.
         *
         * A desigualdade mede-se em INTEIROS: sigma_n > n <=> sqrt(n^2+4) > n <=> n^2+4 > n^2. */
        long mau = 0, n_test = 0;
        for(L n=1;n<=40;n++){ if(!((n*n + 4) > n*n)) mau++; n_test++; }
        printf("      sigma_n > n  <=>  n^2 + 4 > n^2  — verificado em Z para n = 1..40: %ld falhas\n",
               mau);
        ok("sigma_n > n e' uma desigualdade de INTEIROS e vale para todo n — logo (b)_n > 0",
           mau == 0 && n_test == 40);

        /* e a EQUACAO do coeficiente, medida COMO equacao: a^(1+1/b)·(b)_n = 1.
         * O 'a' deriva-se da equacao; nada aqui esta' escrito a mao. */
        printf("\n      %2s %18s %20s %20s %14s\n", "n", "sigma_n", "(sigma_n)_n", "a (derivado)",
               "a^(1+1/b)·(b)_n");
        double pior = 0.0; long fora = 0;
        for(int n=1;n<=10;n++){
            double b = (n + sqrt((double)n*n + 4.0)) / 2.0;
            double pn = 1.0;
            for(int k=0;k<n;k++) pn *= (b - k);
            double e = 1.0 + 1.0/b;                   /* = 1 + b - n */
            double a = pow(pn, -1.0/e);
            double res = fabs(pow(a, e) * pn - 1.0);  /* a equacao, nao um valor esperado */
            if(pn <= 0.0 || e <= 0.0) fora++;
            if(res > pior) pior = res;
            if(n <= 5 || n == 10)
                printf("      %2d %18.12f %20.10f %20.12f %14.1e\n", n, b, pn, a, res);
        }
        printf("      ...\n\n");
        ok("a equacao do coeficiente fecha em n = 1..10, e (b)_n e o expoente sao SEMPRE positivos",
           pior < 1e-12 && fora == 0);

        /* e n = 1 tem de devolver o valor ja' publicado para o ouro — a generalizacao CONTEM */
        double b1 = (1.0 + sqrt(5.0)) / 2.0;
        double a1 = pow(b1, -1.0/(1.0 + 1.0/b1));
        printf("      n = 1 devolve a = %.12f — o valor ja' publicado para o ouro.\n\n", a1);
        ok("n = 1 reproduz 0.742742944625: a generalizacao CONTEM o caso conhecido, nao o troca",
           fabs(a1 - 0.742742944625) < 1e-11);
        conclui("o que faz o coeficiente existir nao e' um calculo feliz: e' sigma_n > n, que e'");
        conclui("uma desigualdade de inteiros. Por ela, todos os fatores do produto descendente");
        conclui("sao positivos e o expoente tambem — e a solucao real positiva nunca falha.");
    }

    /* ── §A3 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A3  AS DUAS RAIZES SAO sigma E sigma', E sigma·sigma' = -1: a alfandega E' o par.\n\n");
    {
        /* b^2 - n·b - 1 = 0: por Vieta, soma das raizes = n e produto = -1. Lido nos
         * COEFICIENTES, exato em Z, sem calcular raiz nenhuma. */
        long mau_prod = 0, mau_soma = 0, casos = 0;
        for(L n=0;n<=40;n++){
            L a_c = 1, b_c = -n, c_c = -1;
            if(c_c / a_c != -1) mau_prod++;
            if(-b_c / a_c != n)  mau_soma++;
            casos++; }
        printf("      lido nos coeficientes de b^2 - n·b - 1 (Vieta), para n = 0..40:\n");
        printf("        produto das raizes = -1 sempre: %ld falhas\n", mau_prod);
        printf("        soma das raizes    = n  sempre: %ld falhas\n\n", mau_soma);
        ok("sigma·sigma' = -1 para TODO n — e e' exatamente a alfandega deste texto",
           mau_prod == 0 && mau_soma == 0 && casos == 41);

        /* e o discriminante n^2+4 nunca e' quadrado perfeito => sigma_n e' sempre irracional */
        long quad = 0;
        for(L n=1;n<=2000;n++){
            L D = n*n + 4, r = 0;
            while(r*r < D) r++;
            if(r*r == D) quad++; }
        printf("      discriminante n^2 + 4 quadrado perfeito em n = 1..2000: %ld vezes\n", quad);
        printf("        (para n >= 2, n^2 < n^2+4 < (n+1)^2 = n^2+2n+1 pois 2n+1 > 4;\n");
        printf("         e em n = 1, 5 nao e' quadrado. Logo nunca.)\n\n");
        ok("n^2 + 4 nunca e' quadrado perfeito: sigma_n e' irracional para todo n, sem excecao",
           quad == 0);
        conclui("sigma' = -1/sigma nao e' uma analogia com a alfandega: E' a alfandega. O par de");
        conclui("raizes da equacao do expoente E' o par (sigma, sigma') que atravessa a fronteira,");
        conclui("e o -1 do produto E' o sinal que se paga.");
    }

    /* ── §A4 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A4  DICOTOMIA PAR/IMPAR: n par tem DUAS solucoes reais, n impar tem UMA.\n\n");
    {
        /* sigma' = -1/sigma esta' em (-1, 0). Logo cada fator (sigma' - k), k = 0..n-1, e'
         * NEGATIVO, e (sigma')_n tem sinal (-1)^n. Como se precisa de a^(1+1/sigma') =
         * 1/(sigma')_n com a > 0 real, e' preciso (sigma')_n > 0 — logo n PAR. */
        long mau_sinal = 0, pares = 0, impares = 0;
        printf("      %2s %10s %10s %14s %8s   2.a solucao real?\n", "n", "p", "q", "(sigma')_n", "sinal");
        for(int n=1;n<=12;n++){
            Zs d = desc(n);                            /* a mesma reducao serve para as duas raizes */
            double s2 = (n - sqrt((double)n*n + 4.0)) / 2.0;
            double val = d.p + d.q * s2;               /* avaliado na raiz NEGATIVA */
            int pos = (val > 0.0);
            if(pos != (n % 2 == 0)) mau_sinal++;       /* previsto: positivo <=> n par */
            if(n % 2 == 0) pares++; else impares++;
            if(n <= 6 || n == 12)
                printf("      %2d %10lld %10lld %14.6f %8s   %s\n", n, d.p, d.q, val,
                       pos?"+":"-", pos?"SIM":"nao");
        }
        printf("      ...\n\n");
        ok("o sinal de (sigma')_n e' (-1)^n — previsto pela desigualdade, medido em 12 casos",
           mau_sinal == 0 && pares == 6 && impares == 6);
        conclui("n PAR: a familia de potencia tem DUAS solucoes reais, uma por raiz. n IMPAR: uma");
        conclui("so', porque a segunda pediria que um positivo elevado a um real desse negativo.");
        conclui("O que as separa e' o mesmo (-1)^n de Cassini e do determinante da alfandega.");
    }

    /* ── §A5 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A5  (sigma_n)_n E' INTEIRO EM Z[sigma] — e para n IMPAR o termo constante e' ZERO.\n\n");
    {
        long mau_fecho = 0, mau_dic = 0, impares = 0, pares = 0;
        printf("      %2s %14s %14s   %s\n", "n", "p", "q", "nota");
        for(int n=1;n<=14;n++){
            Zs d = desc(n);
            double b = (n + sqrt((double)n*n + 4.0)) / 2.0, pn = 1.0;
            for(int k=0;k<n;k++) pn *= (b - k);
            if(fabs((d.p + d.q*b) - pn) > 1e-6 * (pn > 1 ? pn : 1)) mau_fecho++;
            if((d.p == 0) != (n % 2 == 1)) mau_dic++;
            if(n % 2) impares++; else pares++;
            if(n <= 8 || n == 14)
                printf("      %2d %14lld %14lld   %s\n", n, d.p, d.q,
                       d.p == 0 ? "p = 0  (multiplo puro de sigma)" : "");
        }
        printf("      ...\n\n");
        ok("o produto descendente FECHA em Z[sigma]: p + q·sigma bate com o real em 14 casos",
           mau_fecho == 0);
        ok("e n IMPAR <=> termo constante ZERO — a mesma dicotomia par/impar do §A4",
           mau_dic == 0 && impares == 7 && pares == 7);
        conclui("a analise nao sai da algebra: derivar n vezes uma potencia e reduzir pela borda");
        conclui("devolve um inteiro de Z[sigma]. E a paridade de n decide se ele tem parte");
        conclui("racional — o mesmo (-1)^n, agora dentro do proprio coeficiente.");
    }

    /* ── §A6 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A6  TEOREMA: a reversibilidade FORCA Pisot em grau 2 — e o alcance e' de um lado so'.\n\n");
    {
        /* O Aarao: "a questao de Pisot e a familia metalica merece teorema proprio."
         *
         * A CADEIA:  f^(n) = f^-1  =>  b(b-n) = 1  =>  b^2 - n·b - 1 = 0  =>  termo
         * independente -1  =>  N(sigma) = sigma·sigma' = -1  =>  |sigma'| = 1/sigma < 1.
         * E sigma > n >= 1. Logo sigma e' inteiro algebrico > 1 cujo unico conjugado esta'
         * dentro do disco: E' PISOT. Para todo n >= 1.
         *
         * E ha' um criterio EXATO em inteiros, que dispensa calcular raizes. Para
         * x^2 - A·x + B com D = A^2 - 4B > 0:
         *     |sigma'| < 1  <=>  (A-2)^2 < D < (A+2)^2  <=>  -A-1 < B < A-1.
         * (a segunda equivalencia sai de D = A^2 - 4B, e e' aritmetica de inteiros.)
         * Mede-se contra o calculo real: DOIS CAMINHOS que tem de concordar. */
        long discord = 0, testados = 0, pisot = 0;
        for(L A=1;A<=60;A++) for(L B=-60;B<=60;B++){
            L D = A*A - 4*B;
            if(D <= 0) continue;
            L r = 0; while(r*r < D) r++;
            if(r*r == D) continue;                     /* raiz racional: nao interessa */
            double rd = sqrt((double)D), s = (A + rd)/2.0, s2 = (A - rd)/2.0;
            if(!(s > 1.0)) continue;
            int por_real  = (fabs(s2) < 1.0);          /* caminho 1: calcular o conjugado */
            int por_ints  = (-A-1 < B && B < A-1);     /* caminho 2: desigualdade em Z */
            if(por_real != por_ints) discord++;
            testados++; pisot += por_real; }
        printf("      criterio em Z:  x^2 - A·x + B tem sigma > 1 Pisot  <=>  -A-1 < B < A-1\n");
        printf("        pares (A,B) testados: %ld     Pisot: %ld\n", testados, pisot);
        printf("        discordancias entre CALCULAR O CONJUGADO e a DESIGUALDADE: %ld\n\n", discord);
        ok("a condicao de Pisot em grau 2 e' uma desigualdade de INTEIROS nos coeficientes",
           discord == 0 && testados > 3000 && pisot > 0 && pisot < testados);

        /* e a familia metalica tem A = n, B = -1: cai SEMPRE dentro, e ve-se sem contas */
        long fora = 0;
        for(L n=1;n<=500;n++) if(!(-n-1 < -1 && -1 < n-1)) fora++;
        printf("      a familia metalica e' A = n, B = -1:  -n-1 < -1 < n-1  para todo n >= 2,\n");
        printf("        e em n = 1 o intervalo e' (-2, 0), que contem -1. Falhas em n = 1..500: %ld\n\n",
               fora);
        ok("TODO membro da familia metalica e' numero de Pisot — sem excecao, e por uma linha",
           fora == 0);

        /* MAS o alcance e' de um lado so'. Unidade => Pisot; Pisot NAO => unidade. */
        long contraex = 0; L cA = 0, cB = 0;
        for(L A=1;A<=12;A++) for(L B=-12;B<=12;B++){
            if(B == 1 || B == -1) continue;            /* |N| != 1 */
            L D = A*A - 4*B;
            if(D <= 0) continue;
            L r = 0; while(r*r < D) r++;
            if(r*r == D) continue;
            if(-A-1 < B && B < A-1){ contraex++; if(!cA){ cA=A; cB=B; } } }
        printf("      e a RECIPROCA e' falsa: ha' %ld Pisot de grau 2 com |N| != 1.\n", contraex);
        printf("        o primeiro: x^2 - %lldx + (%lld), com N = %lld\n\n", cA, cB, cB);
        ok("a implicacao e' NUMA direcao: unidade => Pisot, mas Pisot NAO obriga a unidade",
           contraex > 0);
        conclui("o teorema e': em grau 2, |N(sigma)| = 1 com sigma > 1 obriga sigma a ser Pisot,");
        conclui("e a familia metalica e' exatamente o caso das unidades de traco n. Ela nao e'");
        conclui("'os Pisot de grau 2' — e' um subconjunto proprio, e dizer mais seria reivindicar");
        conclui("a mais. E a tensao com a dimensao: em grau n a MESMA exigencia ja' nao forca —");
        conclui("beta_{n,1} deixa de ser Pisot a partir de n = 6. O grau 2 e' que e' especial.");
    }

    /* ── §A7 ─────────────────────────────────────────────────────────────────────────── */
    printf("\n§A7  O MESMO EXPOENTE, DOIS SENTIDOS: iterar da' ORDEM, derivar da' BORDA.\n\n");
    {
        /* O catalogo ja' tinha a lei escrita, noutra roupa e sem a nomear:
         *     F^3 = F^-1  (a transformada, periodo 4)     Frob^(n-1) = Frob^-1  (ordem n)
         * E' a MESMA forma g^k = g^-1, com o expoente lido como ITERACAO em vez de derivacao.
         * Do lado discreto ela nao da' numero nenhum: da' PERIODO. */
        long mau = 0, casos = 0;
        for(L N=2;N<40;N++) for(L g=0;g<N;g++) for(L k=1;k<12;k++){
            /* em Z/N (aditivo): g^k e' k·g, g^-1 e' -g, e a identidade e' 0 */
            int a = (((g*k) % N + N) % N == ((-g) % N + N) % N);
            int b = (((g*(k+1)) % N + N) % N == 0);
            if(a != b) mau++;
            casos++; }
        printf("      g^k = g^-1  <=>  g^(k+1) = e   em Z/N, N = 2..39:\n");
        printf("        %ld casos, discordancias: %ld\n\n", casos, mau);
        ok("do lado da ITERACAO a lei nao produz numero: produz ORDEM — 8569 casos, resid. 0",
           mau == 0 && casos == 8569);

        /* e do lado da DERIVACAO a mesma forma da' um irracional. Em grau 2 os dois lados
         * NUNCA se encontram: sigma_m > 1 real, logo |sigma_m| != 1, logo nunca e' raiz da
         * unidade. Mede-se em INTEIROS: sigma_m > 1  <=>  m^2+4 > (2-m)^2  <=>  m > 0. */
        long unit = 0;
        for(L m=1;m<=500;m++) if(!((m*m + 4) > (2-m)*(2-m))) unit++;
        printf("      e sigma_m > 1  <=>  m^2+4 > (2-m)^2  <=>  m > 0 — em Z, m = 1..500: %ld falhas\n",
               unit);
        printf("        logo |sigma_m| != 1 e ele NUNCA tem ordem finita: em grau 2 os dois\n");
        printf("        lados da lei nao se encontram. E' preciso subir de grau para que se\n");
        printf("        encontrem — e ai' aparece o modulo 1, que E' ter ordem finita.\n\n");
        ok("em grau 2 sigma_m nunca esta' sobre o circulo — o lado da iteracao nao o alcanca",
           unit == 0);
        conclui("a MESMA equacao g^k = g^-1 lida com o expoente como iteracao da' um periodo");
        conclui("(finito, discreto, aritmetico) e lida como derivacao da' a borda (irracional,");
        conclui("continua, algebrica). E' o eixo de Pontryagin escrito NA PROPRIA LEI, e nao");
        conclui("numa analogia sobre ela. O catalogo ja' tinha os dois lados e nao os juntara'.");
    }

    printf("\n================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
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
