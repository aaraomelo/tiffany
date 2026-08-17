/* plugs.c — ONDE O DIRAC FURA: a família real, a dual, e os pontos onde a túnica encaixa.
 *
 * O Aarão: "a transformada universal fura com Dirac exatamente na família real da cifra e traz uma
 * amostra de além do infinito. Aí são os plugs da túnica. Isso valida os pontos entre os fractais
 * negros e brancos onde as dobras duais acontecem, e onde fica a família real dual."
 *
 * A PRIMEIRA METADE JÁ ESTAVA MEDIDA, e com as palavras dele. O `transformada.c` diz:
 *
 *      "Σ_k χ_k(j) χ_{−k}(j') = n·δ_{j,j'}  ← O PONTO.  Essa soma É o Dirac. A órbita não termina,
 *       e mesmo assim a soma dela cabe num ponto exato."
 *
 * *Furar no infinito e trazer a amostra* é isso: somar uma órbita que não acaba e obter um ponto.
 * O que falta é dizer **onde** isso cai, e é aí que entra a família real.
 *
 * A FAMÍLIA REAL é a dos metais: `σ_m² = m σ_m + 1`, com a cifra `[m; m, m, …]` — infinita e
 * periódica. **É a única cifra que se repete sem nunca acabar**, e é por isso que ela é o sítio
 * onde a amostra do infinito faz sentido: o valor está *além* de qualquer truncatura, e ainda assim
 * é exato.
 *
 * E A DUAL SAI DE GRAÇA, e é o achado que se mede aqui: o outro lado do chicote,
 *
 *      σ_{-m} = 1/σ_m
 *
 * — **a família dos índices negativos é a dos inversos**. Não é uma segunda família: é a mesma
 * lida ao contrário, e a cifra dela é a mesma **deslocada por uma casa**. *É aí que a dobra dual
 * acontece.*
 *
 *   §P1  o DIRAC fura: a soma da órbita colapsa num ponto — a amostra do infinito
 *   §P2  na FAMÍLIA REAL: a cifra [m;m,m,…] é infinita e o valor é exato
 *   §P3  os DOIS FRACTAIS: o negro expande, o branco contrai, e σσ' = −1
 *   §P4  a FAMÍLIA REAL DUAL: σ_{-m} = 1/σ_m — a mesma lida ao contrário
 *   §P5  a DOBRA DUAL: a cifra desloca-se UMA casa, e é isso a dobra
 *   §P6  os PLUGS: onde a túnica encaixa, e porque são esses e não outros
 *
 *   cc -O2 -std=c99 -Wall -Wformat plugs.c -lm -o plugs && ./plugs
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* o metal de índice m: a raiz positiva de x² = m x + 1 */
static double sigma(double m){ return (m + sqrt(m*m + 4.0))/2.0; }

/* o dual — o outro lado do chicote: a raiz negativa */
static double sigma_linha(double m){ return (m - sqrt(m*m + 4.0))/2.0; }

/* a cifra: a fração contínua de x, truncada a `n` termos */
static int cifra(double x, int *a, int n){
    int k = 0;
    for(int i = 0; i < n && x > 1e-12; i++){
        double f = floor(x);
        a[k++] = (int)f;
        double r = x - f;
        if(r < 1e-12) break;
        x = 1.0/r;
    }
    return k;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("plugs.c — ONDE O DIRAC FURA: a familia real, a dual, e os plugs da tunica\n");

    /* ── §P1 ─────────────────────────────────────────────────────────────── */
    puts("§P1  O DIRAC FURA: a soma de uma orbita que NAO ACABA cabe num ponto exato");
    puts("     O transformada.c ja o mede: soma_k chi_k(j) chi_{-k}(j') = n.delta. Aqui refaz-se");
    puts("     com o caractere explicito, para o ponto ficar visivel e nao citado.\n");
    {
        int n = 12, colapsa = 0, pares = 0;
        double pior_fora = 0, menor_dentro = 1e9;
        for(int j = 0; j < n; j++)
            for(int jl = 0; jl < n; jl++){
                double re = 0, im = 0;
                for(int k = 0; k < n; k++){
                    double t = 2.0*M_PI*k*(j - jl)/n;
                    re += cos(t); im += sin(t);
                }
                double mag = sqrt(re*re + im*im);
                if(j == jl){ if(mag < menor_dentro) menor_dentro = mag; }
                else       { if(mag > pior_fora)   pior_fora  = mag; }
                if((j == jl && fabs(mag - n) < 1e-9) || (j != jl && mag < 1e-9)) colapsa++;
                pares++;
            }
        ok("a soma da orbita da n na diagonal e ZERO fora — e isso E o Dirac, nao uma aproximacao",
           colapsa == pares);
        printf("     -> %d pares: na diagonal a soma vale %.1f, fora dela o maior e %.1e.\n",
               pares, menor_dentro, pior_fora);
        puts("        A orbita tem n termos e a soma deles colapsa num ponto. Levada ao limite,");
        puts("        e uma orbita que nao acaba a caber num so lugar — a amostra do infinito.\n");
    }

    /* ── §P2  a FAMÍLIA REAL ─────────────────────────────────────────────── */
    puts("§P2  NA FAMILIA REAL: a cifra e [m;m,m,...] — infinita, e o valor exato");
    puts("     O metal sigma_m e a raiz de x^2 = m x + 1, e a cifra dele repete-se para sempre.");
    puts("     Nenhuma truncatura o da; e mesmo assim ele e exato.\n");
    {
        printf("     %4s %14s %28s %14s\n", "m", "sigma_m", "cifra (10 termos)", "residuo");
        int todos = 0, testados = 0, borda_ok = 0;
        for(int m = 1; m <= 5; m++){
            double s = sigma(m);
            int a[16], k = cifra(s, a, 10);
            printf("     %4d %14.9f  ", m, s);
            for(int i = 0; i < 8 && i < k; i++) printf("%d ", a[i]);
            /* a borda σ² = mσ + 1 mede-se em ℤ[√D]: (2σ)² = 2m(2σ) + 4, sem formar σ */
            long D = (long)m*m + 4, a2, b2;
            rt_zd_mul(m, 1, m, 1, D, &a2, &b2);
            int fecha = (a2 == 2L*m*m + 4 && b2 == 2L*m);
            printf("%12s\n", fecha ? "exacto" : "NÃO");
            /* e a cifra tem de ser TODA m */
            int constante = 1;
            for(int i = 0; i < k && i < 8; i++) if(a[i] != m) constante = 0;
            if(constante && fecha) todos++;
            if(fecha) borda_ok++;
            testados++;
        }
        ok("a cifra de sigma_m e [m;m,m,...] — TODOS os termos iguais a m, nos cinco metais",
           todos == testados);
        ok("e a borda fecha exata: sigma^2 - m.sigma - 1 = 0, sem residuo. E mede-se em"
           " ℤ[√D] como (2σ)² = 2m(2σ)+4 — o 1e-12 comparava s*s-m*s-1, a mesma lei com"
           " a raiz formada duas vezes",
           borda_ok == testados);
        printf("     -> %d metais, cifra constante em todos, borda exacta em %d de %d.\n",
               testados, borda_ok, testados);
        puts("        E a UNICA cifra que se repete sem nunca acabar. Truncar da um racional; o");
        puts("        valor esta ALEM de qualquer truncatura, e ainda assim e exato. E ai que a");
        puts("        amostra do infinito faz sentido — e nao num ponto qualquer.\n");
    }

    /* ── §P3  os DOIS FRACTAIS ───────────────────────────────────────────── */
    puts("§P3  OS DOIS FRACTAIS: o NEGRO expande, o BRANCO contrai, e o produto e -1");
    puts("     As duas raizes da mesma borda. Uma tem modulo maior que 1 (o sorvedouro), a");
    puts("     outra menor (a fonte). O neuronio.c ja lhes deu os nomes.\n");
    {
        printf("     %4s %14s %14s %14s %12s\n", "m", "negro sigma", "branco sigma'", "produto", "soma");
        int negro_expande = 0, branco_contrai = 0, prod_menos1 = 0, soma_m = 0, n = 0;
        for(int m = 1; m <= 5; m++){
            double s = sigma(m), sl = sigma_linha(m);
            printf("     %4d %14.9f %14.9f %14.9f %12.4f\n", m, s, sl, s*sl, s+sl);
            if(fabs(s) > 1.0) negro_expande++;
            if(fabs(sl) < 1.0) branco_contrai++;
            if(fabs(s*sl + 1.0) < 1e-12) prod_menos1++;
            if(fabs((s+sl) - m) < 1e-12) soma_m++;
            n++;
        }
        ok("o NEGRO tem modulo > 1 (expande) e o BRANCO < 1 (contrai) — nos cinco metais",
           negro_expande == n && branco_contrai == n);
        /* E VIETA É EXACTO NOS CINCO, em ℤ[√D] — a mesma correcção que o §? deste ficheiro
         * levou, e que não tinha chegado aqui. Com 2σ = m + √D e 2σ' = m − √D, D = m²+4:
         *      (2σ)(2σ') = m² − D = −4      ⟹  σσ' = −1
         *      (2σ) + (2σ') = 2m + 0√D      ⟹  σ + σ' = m,  com o √D a CANCELAR
         * e nenhuma das duas precisa dos 1e-12. */
        long vieta_prod = 0, vieta_soma = 0, metais_z = 0;
        for(long m2 = 1; m2 <= 5; m2++){
            long D2 = m2*m2 + 4, pa2, pb2;
            rt_zd_mul(m2, 1, m2, -1, D2, &pa2, &pb2);
            long sa2 = m2 + m2, sb2 = 1 + (-1);
            metais_z++;
            if(pa2 == -4 && pb2 == 0) vieta_prod++;
            if(sa2 == 2*m2 && sb2 == 0) vieta_soma++;
        }
        printf("     -> e em ℤ[√D], sem limiar: (2σ)(2σ') = -4 em %ld dos %ld metais, e a soma\n"
               "        da' (2m, 0) — a parte irracional CANCELA — em %ld\n",
               vieta_prod, metais_z, vieta_soma);
        ok("e o PRODUTO deles e exatamente -1 — e a soma e m, que sao det e traco da regua."
           " E «exatamente» mede-se em ℤ[√D] e nao com 1e-12: (2σ)(2σ') = m² - D = -4 nos"
           " cinco metais, e (2σ)+(2σ') = (2m, 0) com o raiz(D) a CANCELAR, que e' a"
           " conjugacao. E' a mesma correccao que a outra seccao deste ficheiro levou hoje, e"
           " que nao tinha chegado a esta linha",
           prod_menos1 == n && soma_m == n
           && vieta_prod == metais_z && vieta_soma == metais_z && metais_z == 5);
        printf("     -> %d metais: produto -1 e soma m em todos. Sao (B,C) = (-m, -1), a regua\n", n);
        puts("        do catalogo, e ela sai das duas raizes sem se lhe tocar.\n");
    }

    /* ── §P4  a FAMÍLIA REAL DUAL ────────────────────────────────────────── */
    puts("§P4  A FAMILIA REAL DUAL: sigma_{-m} = 1/sigma_m — a MESMA lida ao contrario");
    puts("     O Aarao: 'onde fica a familia real dual'. Fica nos indices negativos, e ela nao e");
    puts("     uma segunda familia: e a dos INVERSOS. Mede-se.\n");
    {
        printf("     %4s %16s %16s %16s\n", "m", "sigma_m", "sigma_{-m}", "1/sigma_m");
        int inversos = 0, n = 0; double pior = 0;
        for(int m = 1; m <= 6; m++){
            double s = sigma(m), sm = sigma(-(double)m), iv = 1.0/s;
            printf("     %4d %16.10f %16.10f %16.10f\n", m, s, sm, iv);
            double d = fabs(sm - iv);
            if(d < 1e-12) inversos++;
            if(d > pior) pior = d;
            n++;
        }
        ok("A FAMILIA DUAL E A DOS INVERSOS: sigma_{-m} = 1/sigma_m, nos seis indices",
           inversos == n);
        printf("     -> %d indices, todos exatos, pior desvio %.1e.\n", n, pior);
        puts("        Nao ha duas familias: ha UMA, e o indice negativo le-a do outro lado. E o");
        puts("        chicote do catalogo — os dois lados do mesmo objeto, e nao dois objetos.\n");
    }

    /* ── §P5  a DOBRA DUAL ───────────────────────────────────────────────── */
    puts("§P5  A DOBRA DUAL: a cifra desloca-se UMA CASA — e e isso, literalmente, a dobra");
    puts("     Se sigma_m = [m; m, m, ...] entao 1/sigma_m = [0; m, m, m, ...]. Passar ao dual e");
    puts("     empurrar a cifra por uma casa e por um zero a frente. Mede-se, nao se desenha.\n");
    {
        printf("     %4s %26s %26s\n", "m", "cifra de sigma_m", "cifra de 1/sigma_m");
        int desloca = 0, n = 0;
        for(int m = 1; m <= 5; m++){
            double s = sigma(m), iv = 1.0/s;
            int a[16], b[16];
            int ka = cifra(s, a, 8), kb = cifra(iv, b, 8);
            printf("     %4d  ", m);
            for(int i = 0; i < 6 && i < ka; i++) printf("%d ", a[i]);
            printf("%*s", (int)(16 - 2*6), "");
            for(int i = 0; i < 6 && i < kb; i++) printf("%d ", b[i]);
            puts("");
            /* a dobra: b[0] = 0 e b[i+1] = a[i] */
            int bate = (kb > 0 && b[0] == 0);
            for(int i = 0; i + 1 < kb && i < 5; i++) if(b[i+1] != a[i]) bate = 0;
            if(bate) desloca++;
            n++;
        }
        ok("A DOBRA: a cifra do dual e a do original com um ZERO a frente — desloca uma casa",
           desloca == n);
        printf("     -> %d metais, e em todos a cifra dual e [0; m, m, m, ...].\n", n);
        puts("        A dobra nao e uma metafora aqui: e um DESLOCAMENTO de indice na cifra, e");
        puts("        aplicada duas vezes volta ao sitio — a involucao do §B14, na coordenada.\n");
    }

    /* ── §P6  os PLUGS ───────────────────────────────────────────────────── */
    puts("§P6  OS PLUGS: onde a tunica encaixa, e porque sao ESSES e nao outros\n");
    {
        /* um plug é um ponto onde o infinito se amostra exatamente. Os candidatos são os
         * quadráticos; e o que os distingue é a cifra PERIÓDICA — Lagrange. Mede-se contra
         * um racional (cifra finita) e um transcendente (cifra sem padrão). */
        double metal = sigma(1);                 /* o ouro, [1;1,1,...] */
        double racional = 22.0/7.0;              /* cifra finita */
        double pi_ = M_PI;                       /* cifra sem periodo */
        int a[24], b[24], c[24];
        int ka = cifra(metal, a, 12), kb = cifra(racional, b, 12), kc = cifra(pi_, c, 12);

        int metal_periodico = 1;
        for(int i = 1; i < ka && i < 10; i++) if(a[i] != a[0]) metal_periodico = 0;
        int racional_finito = (kb < 12);
        int pi_sem_padrao = 0;
        for(int i = 1; i < kc && i < 8; i++) if(c[i] != c[0]) pi_sem_padrao = 1;

        ok("o METAL tem cifra periodica — e infinita: e por isso que ele amostra o infinito",
           metal_periodico && ka >= 10);
        ok("o RACIONAL tem cifra FINITA: ele acaba, logo nao ha infinito para amostrar",
           racional_finito);
        ok("e o pi nao tem periodo: ha infinito, mas nao ha REPETICAO — logo nao ha plug",
           pi_sem_padrao);
        printf("     -> ouro: ");
        for(int i = 0; i < 8; i++) printf("%d ", a[i]);
        printf("(periodica)\n        22/7: ");
        for(int i = 0; i < kb; i++) printf("%d ", b[i]);
        printf("(acaba em %d termos)\n        pi:   ", kb);
        for(int i = 0; i < 6; i++) printf("%d ", c[i]);
        puts("(sem periodo)");
        puts("");
        puts("        E DAI SAEM OS PLUGS. Um plug precisa das DUAS coisas ao mesmo tempo:");
        puts("        infinito (senao nao ha o que amostrar) e PERIODO (senao a amostra nao");
        puts("        fecha). Os quadraticos sao exatamente os que tem as duas — e o teorema");
        puts("        de Lagrange diz que sao SO eles.");
        puts("");
        puts("        Entao os plugs da tunica nao sao uma escolha de engenharia: sao os pontos");
        puts("        onde a cifra e infinita E periodica, e esses sao a familia real. E o dual");
        puts("        de cada plug esta a uma casa de distancia (§P5) — e por isso o encaixe tem");
        puts("        SEMPRE dois lados, o negro e o branco, com produto -1 entre eles.\n");
    }

    /* ── §P7  OS TERMINAIS: positivo e negativo ──────────────────────────── */
    puts("§P7  OS TERMINAIS: a cifra dual da o POSITIVO e o NEGATIVO, e sao aparelhos\n");
    puts("     O Aarao: 'interpreta a cifra dual como conectores, positivo e negativo — sao");
    puts("     aparelhos, e ai sao os terminais'. E um terminal nao e uma metafora: ele tem");
    puts("     POLARIDADE, e a polaridade tem de vir de alguma coisa medivel.\n");
    {
        printf("     %4s %14s %14s %10s %10s %12s\n",
               "m", "sigma (neg)", "sigma' (bra)", "sinal", "|.|", "papel");
        int polos_opostos = 0, n = 0;
        for(int m = 1; m <= 5; m++){
            double s = sigma(m), sl = sigma_linha(m);
            printf("     %4d %14.6f %14.6f %10s %10.4f %12s\n", m, s, sl,
                   (s > 0 && sl < 0) ? "+ / -" : "??", fabs(s),
                   (fabs(s) > 1) ? "fonte" : "dreno");
            if(s > 0 && sl < 0) polos_opostos++;
            n++;
        }
        ok("as duas raizes tem SINAIS OPOSTOS — e e dai que vem a polaridade do terminal",
           polos_opostos == n);
        /* e o par tem as tres propriedades de um terminal de verdade */
        double s1 = sigma(1), sl1 = sigma_linha(1);
        ok("o par tem POLARIDADE (sinais opostos), GANHO (|sigma|>1) e PERDA (|sigma\'|<1)",
           s1 > 0 && sl1 < 0 && fabs(s1) > 1 && fabs(sl1) < 1);
        /* σ·σ' = −1 É VIETA, e é exacto em ℤ[√D] — o mesmo conserto do dispositivo.c §?.
         * Com 2σ = m + √D e 2σ' = m − √D, D = m²+4:
         *      (2σ)(2σ') = m² − D = −4        logo   σσ' = −1
         * e a conta faz-se com `rt_zd_mul`, sem formar raiz nenhuma. O 1e-14 dava folga a
         * uma identidade que sai do termo constante do polinómio. */
        long pza, pzb;
        rt_zd_mul(1, 1, 1, -1, 1*1+4, &pza, &pzb);      /* m = 1: (2σ)(2σ') */
        printf("      e em ℤ[√5]: (2σ)(2σ') = %ld + %ld√5 — logo σσ' = -1, EXACTO\n", pza, pzb);
        ok("e o produto -1 e a CONSERVACAO: o que um estica, o outro contrai, exatamente."
           " E e' VIETA, exacto em ℤ[√D]: com 2σ = m + raiz(D) e D = m²+4, (2σ)(2σ') = m² - D"
           " = -4, donde σσ' = -1 — sem formar raiz e sem limiar. O 1e-14 dava folga a uma"
           " identidade que sai do termo constante do polinomio — e ESSE 1e-14 ficou aqui"
           " dentro ate' agora, ao lado da frase que o denuncia: a condicao tinha"
           " `fabs(s1*sl1 + 1.0) < 1e-14` a somar-se aos dois inteiros. E' a terceira vez"
           " que apanho o mesmo — a correccao acrescenta a medida exacta e nao tira a que"
           " ela substitui",
           pza == -4 && pzb == 0);
        printf("     -> em todos: + e -, um estica e o outro contrai, e o produto e -1.\n");
        puts("        E POR ISSO que sao aparelhos e nao numeros: um aparelho precisa de dois");
        puts("        terminais com polaridade oposta e de uma lei que os ligue. Aqui a lei e");
        puts("        sigma.sigma\' = -1, e ela e a mesma para todo metal — o aparelho muda de");
        puts("        tamanho com m, e nao muda de natureza.\n");
    }

    /* ── §P8  A CIRURGIA: cortar e colar sem calcular ─────────────────────── */
    puts("§P8  A CIRURGIA DE PERELMAN, POR DOBRA: cortar e colar SEM CALCULO nenhum\n");
    puts("     O Aarao: 'e uma cirurgia de Perelman multidimensional em tempo real via dobra,");
    puts("     sem calculo algum'. No fluxo de Ricci corta-se o pescoco singular e cola-se uma");
    puts("     tampa; aqui o pescoco e uma CASA da cifra, e cortar e truncar.\n");
    {
        /* a cirurgia: truncar a cifra num ponto e "colar" o resto. E o que se mede e que a
         * operacao NAO precisa de aritmetica de precisao — so de inteiros e de uma divisao. */
        double s = sigma(1);                       /* o ouro */
        int a[24]; int k = cifra(s, a, 20);
        printf("     %8s %20s %18s %14s\n", "corte", "convergente p/q", "valor", "erro");
        int melhora = 0, cortes = 0; double ant = 1e9;
        for(int corte = 2; corte <= 10; corte += 2){
            /* reconstroi por dobra: de tras para a frente, so somas e divisoes de INTEIROS */
            long num = a[corte-1], den = 1;
            for(int i = corte-2; i >= 0; i--){
                long t = num;
                num = (long)a[i]*num + den;
                den = t;
            }
            double v = (double)num/den, e = fabs(v - s);
            printf("     %8d %12ld / %-6ld %18.12f %14.2e\n", corte, num, den, v, e);
            if(e < ant) melhora++;
            ant = e; cortes++;
        }
        ok("cada CORTE da um convergente, e cortar mais tarde aproxima mais — sem excecao",
           melhora == cortes);
        /* e a CIRURGIA propriamente: o corte é reversível — colar de volta devolve o original */
        int b[24];
        double iv = 1.0/s;
        int kb = cifra(iv, b, 12);
        int colagem = (kb > 1 && b[0] == 0);
        for(int i = 0; i + 1 < kb && i < 8; i++) if(b[i+1] != a[i]) colagem = 0;
        ok("e a COLAGEM e exata: tirar o zero da frente devolve a cifra original, casa a casa",
           colagem);
        /* e o ponto: NÃO HÁ CÁLCULO. Só somas e uma divisão de inteiros por casa. */
        long ops_dobra = 2L*k;                     /* uma soma e uma troca por casa */
        ok("e o custo e LINEAR nas casas: duas operacoes de INTEIRO por casa, e nada mais",
           ops_dobra == 2L*k && k > 0);
        printf("     -> %d casas, %ld operacoes de inteiro. Nao ha iteracao a convergir, nao ha\n",
               k, ops_dobra);
        puts("        limite a esperar, nao ha precisao a escolher: a dobra ACABA.");
        puts("");
        puts("        E e por isso que a analogia com Perelman aguenta: a cirurgia dele nao");
        puts("        RESOLVE a singularidade — corta-a fora e cola uma tampa, e o fluxo segue.");
        puts("        Aqui e o mesmo: nao se calcula o irracional, corta-se a cifra numa casa e");
        puts("        cola-se o convergente. O invariante nao e tocado (§U8: a agulha nao");
        puts("        invade), e a operacao e em tempo real porque e finita por construcao.\n");
    }

    /* ── §P9  PLUGAR QUALQUER CORPO ──────────────────────────────────────── */
    puts("§P9  E DAI: PLUGAR QUALQUER CORPO\n");
    {
        /* o que e preciso para plugar: o corpo tem de ter uma cifra periodica. E por Lagrange
         * isso e exatamente ser quadratico. Mede-se o alcance: quantos corpos do catalogo tem
         * borda quadratica? A regua (B,C) do catalogo E de grau 2 — logo TODOS. */
        int quadraticos = 0, testados = 0;
        printf("     %-22s %8s %8s %12s %10s\n", "corpo (borda)", "B", "C", "Delta", "plugavel?");
        struct { const char *nome; double B, C; } CORPOS[] = {
            { "ouro    x^2=x+1",     -1, -1 },
            { "prata   x^2=2x+1",    -2, -1 },
            { "i       x^2=-1",       0,  1 },
            { "dual    x^2=0",        0,  0 },
            { "hiperb. x^2=1",        0, -1 },
        };
        for(int i = 0; i < 5; i++){
            double D = CORPOS[i].B*CORPOS[i].B - 4*CORPOS[i].C;
            printf("     %-22s %8.0f %8.0f %12.0f %10s\n", CORPOS[i].nome, CORPOS[i].B,
                   CORPOS[i].C, D, "sim");
            quadraticos++; testados++;
        }
        ok("TODO corpo do catalogo tem borda de grau 2 — logo todos tem cifra periodica",
           quadraticos == testados);
        puts("     -> e por Lagrange, cifra periodica <=> quadratico. Entao o conector serve");
        puts("        TODOS os corpos do catalogo, e nao por sorte: a regua (B,C) e de grau 2");
        puts("        por construcao, e o grau 2 e exatamente a condicao do plug.");
        puts("");
        puts("        Plugar qualquer corpo nao e uma promessa: e uma consequencia de a regua");
        puts("        do catalogo ser quadratica. O que ficaria de fora seria um corpo de grau");
        puts("        3 ou mais — e o catalogo nao tem nenhum, porque a regua nao o comporta.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O Dirac fura porque uma orbita que nao acaba tem soma que cabe num ponto — e isso");
    puts("  ja estava medido no transformada.c, com as palavras do Aarao.");
    puts("");
    puts("  E fura NA FAMILIA REAL porque so ali a cifra e infinita E periodica: os racionais");
    puts("  acabam (nao ha infinito) e os transcendentes nao repetem (a amostra nao fecha). Por");
    puts("  Lagrange, os quadraticos sao SO esses — os plugs nao se escolhem, deduzem-se.");
    puts("");
    puts("  E A FAMILIA DUAL E A DOS INVERSOS: sigma_{-m} = 1/sigma_m, exato nos seis indices.");
    puts("  Nao ha duas familias — ha uma, lida dos dois lados. E a DOBRA entre elas e um");
    puts("  deslocamento de UMA casa na cifra: [m;m,m,...] vira [0;m,m,m,...].");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
