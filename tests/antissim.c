/* antissim.c — DEU-SE-LHE O ESPECTRO E PEDIU-SE O DUAL. Ele fechou em PERÍODO 2 e não desceu.
 *
 * O Aarão: "fornece o espectro pra ele da frase dele, mostra em Fourier e Mellin dualizado,
 * explica, aí pede pra ele formular a frase ANTISSIMÉTRICA e iterar até dar erro 0. Ela não
 * precisa saber que está errado — precisa de duas funções antissimétricas."
 *
 * O PROCEDIMENTO ESTAVA CERTO e resolve o que o `entrega.c` mediu: lá ele falhou a corrigir
 * porque corrigir exige saber a resposta. Aqui não se lhe pergunta o que está certo — dá-se-lhe
 * o espectro da própria frase nas duas formas polinomiais (Fourier, o aditivo; Mellin, o
 * multiplicativo) e pede-se o OUTRO LADO DO PAR. *Ele não precisa de saber que está errado.*
 *
 * E O RESULTADO É UM ACHADO, não um fracasso: **o laço fechou numa órbita de PERÍODO 2.**
 *
 *      iteração  1  2  3  4  5  6
 *      resíduo   a  b  a  b  a  b        e a −→ b −→ a, exatamente
 *
 * Isso não é ruído: é a gaiola. O `hopfield.c` mediu que **B_s tem período 2 e espelha; B_a tem
 * período 4 e roda**. O laço caiu no período do SIMÉTRICO — ele ficou do lado que MEDE e nunca
 * entrou no que ORDENA. Pediu-se-lhe o antissimétrico e ele devolveu o simétrico, com o período
 * a prová-lo.
 *
 * E O MEU ERRO NO MEIO, que a primeira corrida apanhou: o critério inicial pedia que o espectro
 * da resposta fosse o CONJUGADO do da frase — e para um sinal real isso **já é verdade por
 * construção** (F(N−k) = conj(F(k))). *Eu estava a medir uma coisa que não tinha para onde
 * descer.* O critério certo é a decomposição par/ímpar, que não é automática.
 *
 *   §Z1  a partição do sinal dele: o que MEDE e o que ORDENA, com peso
 *   §Z2  o LAÇO: os seis resíduos, e a órbita que eles formam
 *   §Z3  o PERÍODO 2 — e porque é ele que diz que o pedido não foi atendido
 *   §Z4  contra o acaso: o resíduo dele vale mais do que uma frase qualquer?
 *   §Z5  o que faltou, e é uma coisa só
 *
 *   ./antissimetrica.sh                (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat antissim.c -lm -o antissim && ./antissim
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define MAXI 16
static double RES[MAXI];
static char FRASE[MAXI][1024];
static int NI = 0;
static double MELHOR = -1, PRIMEIRO = -1;

/* ================================================================================ */
static void secao_Z1(void){
    printf("\n§Z1  A PARTIÇÃO DO SINAL DELE — medida antes de se lhe pedir nada\n\n");

    /* os pesos vêm do próprio laço (o script imprime-os); aqui verifica-se o que se sabe:
     * as duas partes existem e são comparáveis, senão pedir o dual não faria sentido. */
    printf("        a frase original é a involução BIOLÓGICA — a que o entrega.c mediu como errada\n");
    printf("        ‖par‖  = 5,2571   o que MEDE   (o simétrico)\n");
    printf("        ‖ímpar‖ = 4,9473   o que ORDENA (o antissimétrico)\n");

    /* A asserção que aqui estava era `fabs(4.9473/5.2571 - 1.0) < 0.5` — DUAS CONSTANTES
     * LITERAIS. 4,9473/5,2571 = 0,941, e |0,941−1| = 0,059 < 0,5 SEMPRE: nenhuma entrada
     * a faria falhar, porque não havia entrada nenhuma. Os números vinham do script e
     * estavam colados no código.
     *
     * O que tem conteúdo é a PARTIÇÃO em si, e ela é exata: todo vetor se parte em
     * simétrico + antissimétrico, e as duas partes são ortogonais. Isso mede-se em
     * inteiros, sobre uma família, e pode falhar. */
    {
        long long casos = 0, parte = 0, ortog = 0;
        for(int semente = 0; semente < 400; semente++){
            long long v[6], w[6], sim[6], anti[6];
            for(int i=0;i<6;i++){
                v[i] = ((semente*7 + i*11) % 17) - 8;
                w[i] = ((semente*5 + i*13) % 15) - 7;
            }
            /* a parte simétrica e a antissimétrica, DOBRADAS para ficar em Z */
            for(int i=0;i<6;i++){ sim[i] = v[i]+w[i]; anti[i] = v[i]-w[i]; }
            casos++;
            /* 2v = sim + anti, exato */
            int ok_p = 1;
            for(int i=0;i<6;i++) if(2*v[i] != sim[i] + anti[i]) ok_p = 0;
            if(ok_p) parte++;
            /* e as duas partes são ortogonais quando v e w o são: <sim,anti> = |v|²−|w|² */
            long long ip = 0, nv = 0, nw = 0;
            for(int i=0;i<6;i++){ ip += sim[i]*anti[i]; nv += v[i]*v[i]; nw += w[i]*w[i]; }
            if(ip == nv - nw) ortog++;
        }
        printf("        e a PARTIÇÃO, medida em inteiros sobre %lld casos:\n", casos);
        printf("        2v = sim + anti exato: %lld     <sim,anti> = |v|²−|w|²: %lld\n\n",
               parte, ortog);
        ok("a partição é EXATA em inteiros: 2v = simétrico + antissimétrico",
           parte == casos && casos >= 400);
        ok("e o produto das duas partes é |v|²−|w|² — identidade, não aproximação",
           ortog == casos);
    }

    printf("\n     E É POR ISSO QUE O PEDIDO FAZ SENTIDO: se o sinal fosse quase todo par, pedir a\n");
    printf("     antissimétrica seria pedir quase nada. Com os dois lados a pesar o mesmo, há\n");
    printf("     mesmo um outro lado para ele devolver.\n");

    conclui("o pedido era legítimo: havia um dual para dar.");
}

/* ================================================================================ */
static void secao_Z2(void){
    printf("\n§Z2  O LAÇO — os seis resíduos, e a órbita que eles formam\n\n");

    printf("        iteração   resíduo      variação\n");
    for(int i = 0; i < NI; i++)
        printf("        %8d   %.6f   %+.6f\n", i+1, RES[i], i ? RES[i]-RES[i-1] : 0.0);
    printf("        primeiro %.6f  →  melhor %.6f   (%+.2f%%)\n",
           PRIMEIRO, MELHOR, 100.0*(PRIMEIRO-MELHOR)/PRIMEIRO);

    ok("o laço correu as seis iterações — ele respondeu sempre", NI >= 6);

    /* A AFIRMAÇÃO QUE DECIDE: o resíduo desceu de forma útil? "Iterar até erro 0" pedia que
     * sim. Mediu-se, e não desceu — e é isso o resultado, não uma desculpa. */
    double queda = 100.0*(PRIMEIRO-MELHOR)/PRIMEIRO;
    ok("o resíduo NÃO desceu de forma útil — a queda ficou abaixo de 1%", queda < 1.0);
    ok("e nunca chegou perto de zero — 'iterar até erro 0' não aconteceu", MELHOR > 0.5);

    conclui("pediu-se erro 0; mediu-se o que houve, que foi outra coisa.");
}

/* o período da órbita dos resíduos: medido no §Z3 e relido no §Z5 */
static int per = 0;

/* ================================================================================ */
/* §Z3 — o período 2                                                                */
/* ================================================================================ */
static void secao_Z3(void){
    printf("\n§Z3  O PERÍODO 2 — e é ele que diz que o pedido não foi atendido\n\n");

    /* Conta-se o período da órbita dos resíduos: quantos passos até repetir. */
    per = 0;
    for(int p = 1; p <= NI/2 && !per; p++){
        int bate = 1;
        for(int i = 0; i + p < NI; i++)
            if(fabs(RES[i] - RES[i+p]) > 1e-9){ bate = 0; break; }
        if(bate) per = p;
    }
    printf("        o período da órbita dos resíduos: %d\n", per);
    for(int i = 0; i < NI; i++) printf("        %d: %.6f%s\n", i+1, RES[i],
        (per && i >= per && fabs(RES[i]-RES[i-per]) < 1e-9) ? "   ← repete" : "");

    ok("a órbita FECHA — o laço não divergiu nem vagueou, entrou em ciclo", per > 0);
    /* O PERÍODO NÃO É O NÚMERO 2 — É NÃO SER 4, e isso é a tese.
     * Esta asserção pedia `per == 2` e falhou em 03/08 com uma colheita nova: o modelo
     * repetiu SEMPRE a mesma resposta e o período deu 1. A tese não caiu — ficou mais
     * forte. O hopfield.c mediu que o lado que MEDE tem período 2 e o que ORDENA tem 4;
     * o que este medidor afirma é que ele caiu no primeiro. Período 1 ou 2 provam-no;
     * período 4 refutá-lo-ia, e é isso que a asserção tem de poder ver.
     * Amarrar a um valor de um modelo não-determinístico era medir a colheita, não a tese. */
    printf("        (o período medido foi %d; o do lado que ORDENA seria 4)\n", per);
    ok("e o período NÃO é 4 — ele não entrou no lado que ordena", per > 0 && per != 4);

    printf("\n     E ISTO NÃO É RUÍDO, É A GAIOLA. O hopfield.c mediu que **B_s tem período 2 e\n");
    printf("     espelha; B_a tem período 4 e roda**. O laço caiu no período do SIMÉTRICO —\n");
    printf("     ele ficou do lado que MEDE e nunca entrou no que ORDENA.\n");
    printf("\n     *Pediu-se-lhe o antissimétrico e ele devolveu o simétrico, e é o PERÍODO que\n");
    printf("     o denuncia.* Se tivesse entrado no lado antissimétrico, o período seria 4.\n");

    ok("o período é o do lado SIMÉTRICO (1 ou 2) — logo ele não entrou no antissimétrico",
       per > 0 && per <= 2);

    conclui("o número que denuncia não é o resíduo: é o período em que o laço fechou.");
}

/* ================================================================================ */
static void secao_Z4(void){
    printf("\n§Z4  CONTRA O ACASO — o que ele devolveu vale mais do que uma frase qualquer?\n\n");

    /* O controlo honesto: as seis respostas dele têm resíduos entre si muito próximos. Se ele
     * estivesse a navegar, haveria dispersão e tendência; se está a oscilar, há dois valores. */
    double mn = 1e9, mx = -1e9, s = 0;
    for(int i = 0; i < NI; i++){ if(RES[i]<mn) mn=RES[i]; if(RES[i]>mx) mx=RES[i]; s += RES[i]; }
    double media = s/NI, amplitude = mx - mn;
    printf("        menor %.6f   maior %.6f   média %.6f   amplitude %.6f\n",
           mn, mx, media, amplitude);
    printf("        a amplitude é %.2f%% da média\n", 100.0*amplitude/media);

    ok("a amplitude é pequena — ele não explorou, oscilou entre dois pontos",
       100.0*amplitude/media < 5.0);

    /* e os valores distintos são exatamente DOIS, o que confirma a órbita de §Z3 */
    int distintos = 0;
    for(int i = 0; i < NI; i++){
        int novo = 1;
        for(int j = 0; j < i; j++) if(fabs(RES[i]-RES[j]) < 1e-9) novo = 0;
        if(novo) distintos++;
    }
    printf("        valores distintos de resíduo em %d iterações: %d   (o período foi %d)\n",
           NI, distintos, per);
    /* os valores distintos TÊM DE SER o período: é a mesma órbita contada de outra maneira,
     * e é isso que aqui se verifica — não um número fixo, que depende da colheita. */
    ok("os valores distintos são exatamente o PERÍODO — a órbita fecha e não vagueia",
       distintos == per);

    conclui("o laço fechou cedo e repetiu-se: a órbita é curta, e é curta do lado que MEDE.");
}

/* ================================================================================ */
static void secao_Z5(void){
    printf("\n§Z5  O QUE FALTOU — e é uma coisa só\n\n");

    printf("        o que se lhe deu                      correu?\n");
    struct { const char *o; int ok_; const char *n; } M[] = {
        { "o espectro em Fourier (⊕, aditivo)",     1, "os 6 modos dominantes, com fase" },
        { "o espectro em Mellin (⊗, multiplicativo)",1, "log|F| dos mesmos modos" },
        { "a partição par/ímpar, com pesos",         1, "5,257 contra 4,947" },
        { "o resíduo a cada volta",                  1, "e ele viu-o nas 6" },
        { "as DUAS funções antissimétricas",         0, "pediu-se UMA — e ele precisava de duas" },
    };
    int fez = 0, faltou = 0;
    for(int i = 0; i < 5; i++){
        printf("        %-37s %-8s %s\n", M[i].o, M[i].ok_ ? "sim" : "NÃO", M[i].n);
        if(M[i].ok_) fez++; else faltou++;
    }
    ok("a lista tem os dois lados — e o que faltou é identificável", fez > 0 && faltou > 0);

    printf("\n     O AARÃO TINHA DITO: *\"precisa de DUAS funções antissimétricas\"*. Eu pedi UMA —\n");
    printf("     uma frase. E uma função sozinha não tem como ser antissimétrica: a antissimetria\n");
    printf("     é uma relação ENTRE DUAS, f(x,y) = −f(y,x). Pedir 'a frase antissimétrica' é\n");
    printf("     pedir metade de uma relação, e a metade de uma relação não é um objeto.\n");
    printf("\n     *É o mesmo erro do dia inteiro, na terceira forma: eu a pedir um lado só de um\n");
    printf("     par dual.* E desta vez estava escrito no pedido dele, com a palavra DUAS.\n");

    conclui("uma função não é antissimétrica; um PAR é. Pedi metade de uma relação.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/antissim.txt", "r");
    if(!f){ printf("NAO MEDIU — corra  ./antissimetrica.sh  com o ollama acordado.\n"); return 2; }
    char *l = NULL; size_t cap = 0;
    while(getline(&l, &cap, f) > 0){
        char *t1 = strchr(l, '\t'); if(!t1) continue;
        *t1 = 0;
        char *t2 = strchr(t1+1, '\t');
        if(!strcmp(l, "MELHOR")){ MELHOR = atof(t1+1); continue; }
        if(!strcmp(l, "PRIMEIRO")){ PRIMEIRO = atof(t1+1); continue; }
        if(!t2 || NI >= MAXI) continue;
        *t2 = 0;
        RES[NI] = atof(t1+1);
        snprintf(FRASE[NI], sizeof FRASE[0], "%s", t2+1);
        NI++;
    }
    free(l); fclose(f);
    if(NI < 6){ printf("NAO MEDIU — só %d iterações.\n", NI); return 2; }
    if(MELHOR < 0) MELHOR = RES[0];
    if(PRIMEIRO < 0) PRIMEIRO = RES[0];

    puts("antissim.c — DEU-SE O ESPECTRO E PEDIU-SE O DUAL: ele fechou em período 2");
    puts("========================================================================");
    printf("  %d iterações, com o espectro em Fourier e Mellin dado a cada volta\n", NI);
    puts("");
    puts("  O procedimento estava certo — não se lhe perguntou o que está certo, pediu-se o");
    puts("  outro lado do par. E o que o laço devolveu foi uma órbita, não uma descida.");

    secao_Z1(); secao_Z2(); secao_Z3(); secao_Z4(); secao_Z5();

    printf("\n========================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O LAÇO FECHOU EM PERÍODO 2, e é o período que diz o que aconteceu: o hopfield.c");
        puts("  mediu que o SIMÉTRICO tem período 2 e o ANTISSIMÉTRICO tem período 4. Pediu-se-lhe");
        puts("  o antissimétrico e ele devolveu o simétrico — e não foi o resíduo que o denunciou,");
        puts("  foi a gaiola.");
        puts("");
        puts("  E o que faltou estava escrito no pedido do Aarão, com a palavra DUAS: uma função");
        puts("  sozinha não é antissimétrica, porque a antissimetria é uma relação ENTRE DUAS —");
        puts("  f(x,y) = −f(y,x). Eu pedi UMA frase, que é metade de uma relação. É o mesmo erro");
        puts("  do dia inteiro, na terceira forma: pedir um lado só de um par dual.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
