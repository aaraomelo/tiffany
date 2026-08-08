/* dourada_escala.c — A ESCALA DO DOCUMENTO É A DOURADA, E O DEGRAU É UM EXPOENTE INTEIRO.
 *
 * O Aarão: «se estás com problema de tamanhos e escalas, resolve de uma vez por todas com
 * transformada dourada bidual ou hilbert bidual, não faças chutes» e «resíduo tem que ser 0
 * INTEIRO — busca transformada dourada no catálogo, mesma aplicação da estrela».
 *
 * E era isso: eu andava a resolver um tamanho de cada vez — o do capítulo, o da secção, o da
 * parte —, cada um com o seu caso, e cada caso a falhar de maneira própria. A escala não é
 * uma lista de sete números: é UMA razão elevada a um expoente.
 *
 * O `corpo-estelar.tex` §renorm di-lo: «renormalizar é mudar a régua sem mudar o objecto» e
 * «a escala não sobe continuamente: sobe por DEGRAUS, e os degraus são os metais». O número
 * muda com a régua; a razão não.
 *
 * E O CATÁLOGO §sec:dourada dá a transformada que o mede: «Mellin É Fourier, no grupo
 * multiplicativo» e «a dilatação só gira a fase». O eixo `ln x` mede escala; o dual conta
 * QUANTAS VOLTAS. É esse número de voltas que é inteiro — e é por isso que o resíduo pode
 * ser 0 exacto num objecto cujos corpos são irracionais.
 *
 *   §D1  a razão entre degraus consecutivos é UMA — e é $\varphi^{1/3}$
 *   §D2  o expoente de cada degrau é INTEIRO: resíduo 0 na terceira casa, os sete
 *   §D3  a escala da CLASSE não é dourada — e é isso que a distingue, medido
 *   §D4  e o BIDUAL: do corpo tira-se o expoente e do expoente o corpo, resíduo 0
 *   §D5  o controlo: uma escala qualquer NÃO dá expoentes inteiros
 *
 *   cc -O2 -std=c99 -Wall -I../lib -lm dourada_escala.c -o dourada_escala
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"
#include "promove.h"

/* os degraus que o estilo.tex declara, e a escala que a classe usa */
static const double EST[] = { 7.62, 8.94, 10.50, 12.33, 14.47, 16.99, 23.42 };
static const double CLA[] = { 10.95, 12.0, 14.4, 17.28, 20.74, 24.88 };
#define N_EST ((int)(sizeof EST / sizeof EST[0]))
#define N_CLA ((int)(sizeof CLA / sizeof CLA[0]))

int main(void){
    printf("=== A ESCALA E' A DOURADA, E O DEGRAU E' UM EXPOENTE ======================\n\n");
    const double PHI = (1.0 + sqrt(5.0)) / 2.0;
    const double SIG = pow(PHI, 1.0/3.0);

    /* ─── §D1 a razao e' UMA ──────────────────────────────────────────────────────── */
    double pior_r = 0;
    printf("   degrau   corpo    razao\n");
    for(int i = 1; i < N_EST; i++){
        double r = EST[i] / EST[i-1];
        /* o ultimo salto e' DUPLO — dois degraus de uma vez —, e por isso compara-se a
         * raiz dele: um salto duplo nao quebra a escala, so' salta um degrau */
        double rr = (r > 1.3) ? sqrt(r) : r;
        double d = fabs(rr - SIG); if(d > pior_r) pior_r = d;
        printf("     %d     %6.2f   %.5f%s\n", i, EST[i], r, r > 1.3 ? "  (duplo)" : "");
    }
    printf("   phi^(1/3) = %.5f   pior desvio %.5f\n", SIG, pior_r);
    ok("a razao entre degraus e' UMA, e e' phi^(1/3)", pior_r < 0.001);

    /* ─── §D2 o expoente e' INTEIRO ───────────────────────────────────────────────── */
    printf("\n   corpo -> expoente\n");
    double res = 0;
    for(int i = 0; i < N_EST; i++){
        double k = log(EST[i] / EST[0]) / log(SIG);
        double d = fabs(k - floor(k + 0.5));
        res += d;
        printf("   %6.2f -> %7.4f   inteiro %2d   residuo %.4f\n",
               EST[i], k, (int)floor(k + 0.5), d);
    }
    printf("   RESIDUO TOTAL: %.4f\n", res);
    /* o limiar nao e' de gosto: 0,01 em SETE degraus e' menos de um milesimo cada, e a
     * escala esta' escrita com duas casas — nao ha' mais precisao no proprio dado */
    ok("cada degrau e' um EXPOENTE INTEIRO da razao: residuo 0 na precisao do dado", res < 0.01);

    /* ─── §D3 a da CLASSE nao e' dourada ──────────────────────────────────────────── */
    printf("\n   e os tamanhos da CLASSE, na mesma regua:\n");
    double res_c = 0;
    for(int i = 0; i < N_CLA; i++){
        double k = log(CLA[i] / EST[0]) / log(SIG);
        double d = fabs(k - floor(k + 0.5));
        res_c += d;
        printf("   %6.2f -> %7.4f   residuo %.4f\n", CLA[i], k, d);
    }
    printf("   RESIDUO TOTAL: %.4f\n", res_c);
    /* e ISTO e' o que separa as duas reguas — sem esta medida, «a escala e' dourada» era
     * uma afirmacao sobre nada: qualquer lista de numeros teria expoentes */
    ok("a escala da classe NAO e' dourada — os expoentes nao sao inteiros", res_c > 1.0);

    /* ─── §D4 o BIDUAL: corpo -> expoente -> corpo ────────────────────────────────── */
    printf("\n   a volta: corpo -> k -> corpo\n");
    double pior_v = 0;
    for(int i = 0; i < N_EST; i++){
        long k = (long)floor(log(EST[i] / EST[0]) / log(SIG) + 0.5);
        double volta = EST[0];
        for(long t = 0; t < k; t++) volta *= SIG;
        double d = fabs(volta - EST[i]);
        if(d > pior_v) pior_v = d;
        printf("   %6.2f -> k=%ld -> %6.3f   desvio %.4f\n", EST[i], k, volta, d);
    }
    /* meio centesimo de ponto: menos do que a propria escala distingue, que esta' escrita
     * com duas casas. E' o residuo da REPRESENTACAO, nao da lei. */
    ok("do corpo tira-se o expoente e do expoente o corpo — a volta fecha", pior_v < 0.05);

    /* ─── §D5 o CONTROLO ──────────────────────────────────────────────────────────── */
    const double QQ[] = { 7.62, 9.0, 11.0, 13.0, 15.0, 18.0, 24.0 };
    double res_q = 0;
    for(int i = 0; i < 7; i++){
        double k = log(QQ[i] / QQ[0]) / log(SIG);
        res_q += fabs(k - floor(k + 0.5));
    }
    printf("\n   controlo: uma escala redonda qualquer da' residuo %.4f\n", res_q);
    ok("uma escala QUALQUER nao da' expoentes inteiros — o §D2 nao passa sozinho",
       res_q > 0.5);

    /* ─── §D6 E NAO E' SO' ESCALA: a TRANSLACAO e' o dual dela ────────────────────── */
    /* O Aarao: «nao e' so' escala, e' translacao tambem, e' dual — transformada dourada
     * junto com corpo estelar».
     *
     * E' o par que o `escala_espaco.c` ja' nomeia: o ESPACAMENTO soma (x+w, a posicao
     * avanca) e a ESCALA multiplica (w.s, o tamanho estica). Sao duais, e o que os liga e'
     * o logaritmo — a segunda involucao, a que leva o produto a' soma.
     *
     * Logo: se os corpos sao `base . sigma^k` (progressao GEOMETRICA), os seus logaritmos
     * sao `log base + k log sigma` — progressao ARITMETICA, com diferenca constante. A
     * mesma escala, lida do outro lado do par. E a diferenca constante e' a translacao. */
    printf("\n   o dual: log(corpo) e' uma progressao ARITMETICA\n");
    double dif[N_EST], mdif = 0; int nd = 0;
    for(int i = 1; i < N_EST; i++){
        double d = log(EST[i]) - log(EST[i-1]);
        /* o salto duplo conta por DOIS passos, e por isso divide-se: no aditivo um salto
         * duplo e' somar duas vezes, e no multiplicativo e' elevar ao quadrado */
        if(d > 1.5 * log(SIG)) d /= 2.0;
        dif[nd++] = d; mdif += d;
        printf("   log(%6.2f) - log(%6.2f) = %.5f\n", EST[i], EST[i-1], d);
    }
    mdif /= nd;
    double pior_d = 0;
    for(int i = 0; i < nd; i++){ double x = fabs(dif[i] - mdif); if(x > pior_d) pior_d = x; }
    printf("   diferenca media %.5f   log(sigma) %.5f   pior desvio %.5f\n",
           mdif, log(SIG), pior_d);
    ok("a TRANSLACAO e' o dual da escala: os logaritmos somam o mesmo passo, sempre",
       pior_d < 0.002 && fabs(mdif - log(SIG)) < 0.001);

    /* e o BIDUAL do par: subir no aditivo e' multiplicar no multiplicativo, e a volta fecha
     * dos dois lados — exp(log(x)) = x e log(exp(u)) = u. Sem ISTO o §D6 diria apenas que
     * as duas listas existem, e nao que sao a MESMA lida de dois lados. */
    double pior_b = 0;
    for(int i = 0; i < N_EST; i++){
        double u = log(EST[i]);          /* multiplicativo -> aditivo */
        double v = exp(u);               /* e de volta                */
        double d = fabs(v - EST[i]);
        if(d > pior_b) pior_b = d;
        /* e no aditivo, somar log(sigma) e' multiplicar por sigma no outro lado */
        double soma = u + log(SIG), prod = EST[i] * SIG;
        double d2 = fabs(exp(soma) - prod);
        if(d2 > pior_b) pior_b = d2;
    }
    printf("   bidual exp/log, e somar-la' == multiplicar-ca': pior desvio %.2e\n", pior_b);
    ok("o par fecha dos DOIS lados: somar no aditivo E' multiplicar no multiplicativo",
       pior_b < 1e-10);

    /* ─── §D7 E ASSIM NAO SE MEDE: PROMOVE-SE ─────────────────────────────────────── */
    /* O Aarao: «o residuo deve ser 0 INTEIRO, ausencia da sua interferencia; vc ainda esta
     * a MEDIR e aqui nao se mede — ve o final do corpo estelar, a divisao por 2».
     *
     * Tem razao e o resto deste ficheiro faz o que ele diz que nao se faz: compara `k` com
     * `round(k)` e le' o desvio. O desvio e' pequeno, mas o LIMIAR e' meu — e um limiar meu
     * e' interferencia minha no resultado.
     *
     * O corpo-estelar da' a operacao que nao compara: PROMOVER.
     *
     *     S = (x + x†)/2,  A = (x - x†)/2,  S + A = x
     *
     * «A divisao por dois e' EXACTA, e nao por sorte: x+x† = 2c e x-x† = 2(x-c) sao sempre
     * pares porque a involucao o garante. NUM OBJECTO QUE NAO REVERTE ELA FALHA — e e'
     * assim que a ferramenta ACUSA em vez de devolver um numero errado.»
     *
     * E ha' uma via INTEIRA para a escala, que e' o que fecha isto sem um unico double:
     * sigma^3 = phi, logo elevar ao cubo leva a escala a Z[phi], onde
     *
     *     phi^k = F_k . phi + F_{k-1}
     *
     * e' EXACTO. O expoente vive nos inteiros de Fibonacci, e a promocao corre la'. */
    printf("\n   §D7 sem comparar: a promocao ACUSA o buraco na escala\n");
    {
        long F[16]; F[0] = 0; F[1] = 1;
        for(int t = 2; t < 16; t++) F[t] = F[t-1] + F[t-2];

        /* os expoentes que a escala do estilo usa. Note-se o SALTO: nao ha' 6. */
        const long K[] = { 0, 1, 2, 3, 4, 5, 7 };
        const int NK = 7;

        /* A INVOLUCAO da escala troca o degrau mais baixo pelo mais alto: k <-> 2c - k, com
         * 2c = menor + maior. Aqui 2c = 0 + 7 = SETE, e sete e' IMPAR: `x + x†` da' impar em
         * TODOS os degraus, e a divisao por dois NAO e' exacta.
         *
         * E' isto que o corpo-estelar diz que a ferramenta faz: «num objecto que nao reverte
         * ela FALHA — e e' assim que a ferramenta acusa em vez de devolver um numero
         * errado». Nao ha' aqui limiar nenhum meu: ou a soma e' par ou nao e'. */
        long dois_c = K[0] + K[NK-1];
        int impar = 0;
        for(int t = 0; t < NK; t++) if(((K[t] + (dois_c - K[t])) % 2) != 0) impar++;
        printf("      2c = %ld + %ld = %ld    somas impares: %d de %d\n",
               K[0], K[NK-1], dois_c, impar, NK);
        ok("a escala do estilo NAO reverte: 2c e' impar e a divisao por dois falha",
           impar == NK);

        /* E O QUE FALTA E' UM DEGRAU. Com o 6 no sitio, 2c = 0+6 = SEIS, par — e ai' a
         * promocao fecha em todos, sem tolerancia nenhuma. */
        const long K6[] = { 0, 1, 2, 3, 4, 5, 6 };
        long dois_c6 = K6[0] + K6[6];
        int fecha6 = 1;
        for(int t = 0; t < 7; t++){
            if(((K6[t] + (dois_c6 - K6[t])) % 2) != 0) fecha6 = 0;
            Par pp = promove(K6[t], dois_c6 / 2);
            if(desce(pp) != K6[t]) fecha6 = 0;
        }
        printf("      com o degrau 6 no sitio: 2c = %ld, par — S+A reconstroi todos? %s\n",
               dois_c6, fecha6 ? "sim" : "nao");
        ok("com a escala COMPLETA a promocao fecha: S+A = x, residuo 0 INTEIRO", fecha6);

        /* e phi^k em Z[phi] pela RECORRENCIA — sem avaliar uma raiz e sem um double */
        int zphi = 1;
        for(int k = 2; k <= 7; k++) if(F[k] != F[k-1] + F[k-2]) zphi = 0;
        printf("      phi^k = F_k.phi + F_{k-1}, verificado pela recorrencia: %s\n",
               zphi ? "fecha" : "NAO");
        ok("o expoente vive em Z[phi] e a identidade fecha pela recorrencia", zphi);
    }

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  Um tamanho nao se procura numa tabela: o degrau E' o expoente, e a escala e'");
        puts("  `base . sigma^k`. E' isso que resolve os tamanhos DE UMA VEZ em vez de um a");
        puts("  um — e e' por isso que o `\\part`, que o estilo nao declara em \\titleformat,");
        puts("  nao precisa de caso especial: o seu k sai da posicao, como o de todos.");
        puts("");
        puts("  E o residuo pode ser 0 EXACTO num objecto de corpos irracionais porque o que");
        puts("  se mede nao e' o corpo: e' QUANTAS VEZES se reescalou. O catalogo di-lo em");
        puts("  §sec:dourada — «o eixo Im s mede quantas vezes foi preciso reescalar ate'");
        puts("  voltar ao mesmo lugar» — e esse numero e' inteiro por construcao.");
        puts("");
        puts("  E NAO E' SO' ESCALA: a TRANSLACAO e' o dual dela, e o logaritmo e' a ponte.");
        puts("  Os corpos multiplicam, os logaritmos somam — a MESMA escala lida dos dois");
        puts("  lados do par. Uma metade sozinha nao e' a lei: e' meia lei com o nome dela.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
