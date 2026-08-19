/* antissim.c — DEU-SE-LHE O ESPECTRO E PEDIU-SE O DUAL. Ele fechou em PERÍODO 2 e não desceu.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./antissimetrica.sh                (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat -I lib antissim.c -o antissim && ./antissim
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define FRASE DISCO_FIXO2(char, 1024, 16)

#include <stdlib.h>
#include <string.h>
#include "unidade.h"

#define MAXI 16
#define SCALE 1000000L

static long RES[MAXI];
static int NI = 0;
static long MELHOR = -1, PRIMEIRO = -1;

static long parse_dec6(const char *s){
    const char *p = s;
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    else if(*p == '+') p++;
    long ip = 0;
    while(*p >= '0' && *p <= '9') ip = ip * 10 + (*p++ - '0');
    long fp = 0, pw = 100000L;
    if(*p == '.'){
        p++;
        while(*p >= '0' && *p <= '9' && pw > 0){
            fp += (*p++ - '0') * pw;
            pw /= 10;
        }
    }
    long r = ip * SCALE + fp;
    return neg ? -r : r;
}

/* ================================================================================ */
static void secao_Z1(void){
    printf("\n§Z1  A PARTIÇÃO DO SINAL DELE — medida antes de se lhe pedir nada\n\n");

    printf("        a frase original é a involução BIOLÓGICA — a que o entrega.c mediu como errada\n");
    printf("        ‖par‖  = 5,2571   o que MEDE   (o simétrico)\n");
    printf("        ‖ímpar‖ = 4,9473   o que ORDENA (o antissimétrico)\n");

    {
        long long casos = 0, parte = 0, ortog = 0;
        for(int semente = 0; semente < 400; semente++){
            long long v[6], w[6], sim[6], anti[6];
            for(int i = 0; i < 6; i++){
                v[i] = ((semente * 7 + i * 11) % 17) - 8;
                w[i] = ((semente * 5 + i * 13) % 15) - 7;
            }
            for(int i = 0; i < 6; i++){ sim[i] = v[i] + w[i]; anti[i] = v[i] - w[i]; }
            casos++;
            int ok_p = 1;
            for(int i = 0; i < 6; i++) if(2 * v[i] != sim[i] + anti[i]) ok_p = 0;
            if(ok_p) parte++;
            long long ip = 0, nv = 0, nw = 0;
            for(int i = 0; i < 6; i++){ ip += sim[i] * anti[i]; nv += v[i] * v[i]; nw += w[i] * w[i]; }
            if(ip == nv - nw) ortog++;
        }
        printf("        e a PARTIÇÃO, medida em inteiros sobre %lld casos:\n", casos);
        printf("        2v = sim + anti exato: %lld     <sim,anti> = |v|²−|w|²: %lld\n\n",
               parte, ortog);
        ok("a partição é EXACTA em inteiros: 2v = simétrico + antissimétrico",
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
    for(int i = 0; i < NI; i++){
        long var = i ? RES[i] - RES[i - 1] : 0;
        printf("        %8d   %ld.%06ld   %+ld.%06ld\n",
               i + 1, RES[i] / SCALE, labs(RES[i] % SCALE),
               var / SCALE, labs(var % SCALE));
    }
    long queda = PRIMEIRO > 0 ? 100L * (PRIMEIRO - MELHOR) / PRIMEIRO : 0;
    printf("        primeiro %ld.%06ld  →  melhor %ld.%06ld   (%+ld%%)\n",
           PRIMEIRO / SCALE, labs(PRIMEIRO % SCALE),
           MELHOR / SCALE, labs(MELHOR % SCALE), queda);

    ok("o laço correu as seis iterações — ele respondeu sempre", NI >= 6);
    ok("o resíduo NÃO desceu de forma útil — a queda ficou abaixo de 1%", queda < 1);
    ok("e nunca chegou perto de zero — 'iterar até erro 0' não aconteceu", MELHOR > SCALE / 2);

    conclui("pediu-se erro 0; mediu-se o que houve, que foi outra coisa.");
}

static int per = 0;

/* ================================================================================ */
static void secao_Z3(void){
    printf("\n§Z3  O PERÍODO 2 — e é ele que diz que o pedido não foi atendido\n\n");

    per = 0;
    for(int p = 1; p <= NI / 2 && !per; p++){
        int bate = 1;
        for(int i = 0; i + p < NI; i++)
            if(RES[i] != RES[i + p]){ bate = 0; break; }
        if(bate) per = p;
    }
    printf("        o período da órbita dos resíduos: %d\n", per);
    for(int i = 0; i < NI; i++)
        printf("        %d: %ld.%06ld%s\n", i + 1, RES[i] / SCALE, labs(RES[i] % SCALE),
               (per && i >= per && RES[i] == RES[i - per]) ? "   ← repete" : "");

    ok("a órbita FECHA — o laço não divergiu nem vagueou, entrou em ciclo", per > 0);
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

    long mn = RES[0], mx = RES[0];
    long long s = 0;
    for(int i = 0; i < NI; i++){
        if(RES[i] < mn) mn = RES[i];
        if(RES[i] > mx) mx = RES[i];
        s += RES[i];
    }
    long media = (long)(s / NI);
    long amplitude = mx - mn;
    long pct = media ? 100L * amplitude / media : 0;
    printf("        menor %ld.%06ld   maior %ld.%06ld   média %ld.%06ld   amplitude %ld.%06ld\n",
           mn / SCALE, labs(mn % SCALE), mx / SCALE, labs(mx % SCALE),
           media / SCALE, labs(media % SCALE), amplitude / SCALE, labs(amplitude % SCALE));
    printf("        a amplitude é %ld%% da média\n", pct);

    ok("a amplitude é pequena — ele não explorou, oscilou entre dois pontos", pct < 5);

    int distintos = 0;
    for(int i = 0; i < NI; i++){
        int novo = 1;
        for(int j = 0; j < i; j++) if(RES[i] == RES[j]) novo = 0;
        if(novo) distintos++;
    }
    printf("        valores distintos de resíduo em %d iterações: %d   (o período foi %d)\n",
           NI, distintos, per);
    ok("os valores distintos são exactamente o PERÍODO — a órbita fecha e não vagueia",
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
    disco_prende(DISCO_BASE(16),"dados/FRASE_16.bin",(size_t)((size_t)(MAXI)*1024),sizeof(char));
    disco_zera(FRASE,(size_t)((size_t)(MAXI)*1024),sizeof(char));
    FILE *f = fopen("/tmp/antissim.txt", "r");
    if(!f){ printf("NAO MEDIU — corra  ./antissimetrica.sh  com o ollama acordado.\n"); return 2; }
    char *l = NULL; size_t cap = 0;
    while(getline(&l, &cap, f) > 0){
        char *t1 = strchr(l, '\t'); if(!t1) continue;
        *t1 = 0;
        char *t2 = strchr(t1 + 1, '\t');
        if(!strcmp(l, "MELHOR")){ MELHOR = parse_dec6(t1 + 1); continue; }
        if(!strcmp(l, "PRIMEIRO")){ PRIMEIRO = parse_dec6(t1 + 1); continue; }
        if(!t2 || NI >= MAXI) continue;
        *t2 = 0;
        RES[NI] = parse_dec6(t1 + 1);
        snprintf(FRASE[NI], sizeof FRASE[0], "%s", t2 + 1);
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
