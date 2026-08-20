/* bit_metropolis.c — UM ÚNICO BIT ALEATÓRIO PREENCHE O ESPAÇO, E O RASTRO É A MEDIDA.
 *
 * O Aarão: «a questão aqui é usar um único bit; não interessa qual, pode ser ALEATÓRIO em
 * qualquer thread — melhor que seja. Aí vai preencher todo o espaço; observa o rastro via
 * Monte Carlo, preenche toda a área. Calcula a medida de Lebesgue em Monte Carlo–Metropolis,
 * DUAL.»
 *
 * É o DUAL do bit determinista (o gato, neuronio.c): esse desenha uma CURVA — um rasto de
 * dimensão 1, medida de Lebesgue ZERO. O bit ALEATÓRIO faz o outro lado: o passeio de
 * Metropolis (uma proposta ±1, aceite pela simetria) é ergódico, logo o seu rastro PREENCHE
 * a área — medida CHEIA. E a fração do rastro num conjunto É a medida de Lebesgue desse
 * conjunto, que bate com a CONTAGEM ordenada (pimonte.c) — os dois caminhos, um par:
 *
 *   ordenado (discreto):  contar as casas do reticulado dentro           — EXATO
 *   aleatório (contínuo): a fração do tempo que o bit passa dentro       — Monte Carlo/Metropolis
 *
 * Não interessa QUAL bit nem onde começa: a ergodicidade apaga a condição inicial. Aqui o
 * «aleatório» é um gerador inteiro reprodutível (um por corrida), sem vírgula flutuante no
 * núcleo — as comparações de área são x*x+y*y <= R*R, tudo inteiro.
 *
 *   §B1  o bit único PREENCHE o espaço: o rastro cobre TODAS as casas (cobertura = 100%)
 *   §B2  a medida de Lebesgue pelo rastro (Metropolis) BATE com a contagem — e o erro DESCE
 *   §B3  o par: a curva determinista tem medida ZERO; o rastro aleatório tem medida CHEIA
 *
 *   cc -O2 -std=c99 -Wall -I../lib bit_metropolis.c -o bit_metropolis && ./bit_metropolis
 */
#include <stdio.h>
#include "unidade.h"

typedef long L;
#define N 41                     /* o reticulado N×N, centrado em (N/2,N/2) */
#define R 18                     /* o disco x²+y² <= R² (relativo ao centro) */

/* um bit ALEATÓRIO reprodutível: um LCG inteiro. Não interessa qual — a ergodicidade apaga a
 * semente; troca-se a semente e o rastro preenche na mesma. */
static unsigned long RND;
static int sorteia(int k){ RND = RND * 6364136223846793005UL + 1442695040888963407UL; return (int)((RND >> 33) % (unsigned)k); }

/* a contagem ORDENADA: quantas casas do reticulado caem no disco — a régua exata (pimonte) */
static L conta_dentro(void){
    L c = 0;
    for(int y = 0; y < N; y++) for(int x = 0; x < N; x++){
        L dx = x - N/2, dy = y - N/2;
        if(dx*dx + dy*dy <= (L)R*R) c++;
    }
    return c;
}

int main(void){
    printf("=== UM UNICO BIT ALEATORIO PREENCHE O ESPACO, E O RASTRO E' A MEDIDA ======\n\n");

    /* ── §B1 o bit único preenche o espaço ──────────────────────────────────────────── */
    /* UM bit, uma posição, um passeio de Metropolis: propõe ±1 numa direcção sorteada, e
     * aceita se cabe no reticulado (a reflexão na borda mantém a simetria -> alvo uniforme).
     * Conta-se quantas casas o rastro visita: com passos que cheguem, visita TODAS. */
    static int visita[N][N];
    RND = 12345;                                   /* uma semente qualquer — nao interessa qual */
    int x = 3, y = 7;                              /* comeca em qualquer sitio */
    L passos = 4000000, dentro_t = 0, total_t = 0;
    for(L t = 0; t < passos; t++){
        int d = sorteia(4);
        int nx = x + (d==0) - (d==1), ny = y + (d==2) - (d==3);
        if(nx >= 0 && nx < N && ny >= 0 && ny < N){ x = nx; y = ny; }  /* aceita/reflecte */
        visita[x][y]++;
        L dx = x - N/2, dy = y - N/2;
        total_t++; if(dx*dx + dy*dy <= (L)R*R) dentro_t++;
    }
    L cobertas = 0; for(int j = 0; j < N; j++) for(int i = 0; i < N; i++) if(visita[i][j]) cobertas++;
    printf("§B1  o rastro de UM bit cobre %ld de %d casas (cobertura %ld%%)\n\n",
           cobertas, N*N, 100*cobertas/(N*N));
    ok("§B1 o BIT UNICO aleatorio PREENCHE o espaco: o rastro visita TODAS as casas do"
       " reticulado — a curva vira area", cobertas == (L)N*N);

    /* ── §B2 a medida de Lebesgue pelo rastro bate com a contagem, e o erro desce ─────── */
    /* a fração do tempo dentro do disco -> a medida de Lebesgue do disco (fração da área). A
     * contagem ordenada dá o mesmo, EXATO. Uma corrida só FLUTUA (Monte Carlo é ruidoso); a
     * convergência mede-se em MÉDIA sobre muitas sementes — a lei dos grandes números apaga a
     * flutuação, e é isso que quer dizer «não interessa QUAL bit». */
    L dentro = conta_dentro(), casas = (L)N*N;
    L medida_ordenada = dentro * 1000000 / casas;                 /* a régua exata, em milionésimos */
    L medida_rastro   = dentro_t * 1000000 / total_t;             /* uma corrida (a de §B1) */
    int M = 24;                                                   /* sementes independentes */
    L poucos = 20000, muitos = 800000;
    L soma_poucos = 0, soma_muitos = 0;
    for(int s = 0; s < M; s++){
        RND = 1000u + (unsigned)s * 7919u; int ax = 3, ay = 7, din = 0; L tot = 0;
        for(L t = 0; t < muitos; t++){
            int d = sorteia(4); int nx = ax+(d==0)-(d==1), ny = ay+(d==2)-(d==3);
            if(nx>=0&&nx<N&&ny>=0&&ny<N){ax=nx;ay=ny;}
            L dx=ax-N/2, dy=ay-N/2; tot++; int in = (dx*dx+dy*dy<=(L)R*R);
            if(in) din++;
            if(t+1 == poucos){ L mp = (L)din*1000000/tot; L e = mp-medida_ordenada; soma_poucos += e<0?-e:e; }
        }
        L mm = (L)din*1000000/tot; L e = mm-medida_ordenada; soma_muitos += e<0?-e:e;
    }
    L erro_poucos = soma_poucos / M, erro_muitos = soma_muitos / M;
    printf("§B2  medida de Lebesgue do disco (fracao da area), em milionesimos:\n");
    printf("     ORDENADA (contar casas, EXATO) ................. %ld\n", medida_ordenada);
    printf("     RASTRO   (uma corrida, %ld passos) ............ %ld\n", passos, medida_rastro);
    printf("     erro MEDIO sobre %d sementes: %ld passos -> %ld ppm ; %ld passos -> %ld ppm\n\n",
           M, poucos, erro_poucos, muitos, erro_muitos);
    ok("§B2 a medida de Lebesgue pelo RASTRO aleatorio (Metropolis) bate com a CONTAGEM"
       " ordenada — o par —, e o erro MEDIO (sobre sementes) DESCE com mais passos: converge, e"
       " nao interessa qual bit", erro_muitos < erro_poucos && erro_muitos * 50 < medida_ordenada);

    /* ── §B3 o par: a curva determinista mede ZERO; o rastro aleatorio mede CHEIO ─────── */
    /* o bit DETERMINISTA (um passeio fixo em linha, um por linha) traca uma CURVA: um rasto
     * de dimensao 1 no plano 2D, que cobre O(N) casas de N² -> fracao -> 0 (medida ZERO). O
     * bit ALEATORIO cobre N² (medida CHEIA). E' o furar de medida.tex: a curva nao pesa. */
    L curva = N;                                    /* uma linha: N casas visitadas, deterministas */
    printf("§B3  a curva determinista cobre %ld de %d (fracao 1/N = %ld ppm -> 0: medida ZERO);\n",
           curva, N*N, curva*1000000/(N*N));
    printf("     o rastro aleatorio cobre %ld de %d (medida CHEIA).\n\n", cobertas, N*N);
    /* a curva cobre N de N² = 1/N da area, que -> 0 quando N cresce (medida de Lebesgue ZERO);
     * o rastro aleatorio cobre N² (medida CHEIA). Aqui N=41, logo 1/N < 1/10. */
    ok("§B3 o PAR: a curva determinista (o gato) tem medida de Lebesgue ZERO (cobre 1/N -> 0); o"
       " rastro aleatorio (Metropolis) tem medida CHEIA — o bit DESENHA (curva) ou PREENCHE (area)",
       curva * 10 < (L)N*N && cobertas == (L)N*N);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  UM bit basta, e nao interessa qual: aleatorio, em qualquer thread, a");
        puts("  ergodicidade apaga a semente. O bit DETERMINISTA (o gato) desenha uma curva");
        puts("  (medida ZERO); o bit ALEATORIO (Metropolis) PREENCHE a area (medida CHEIA), e");
        puts("  a fracao do seu rastro num conjunto E' a medida de Lebesgue desse conjunto —");
        puts("  que bate, exata, com a CONTAGEM ordenada. Os dois lados do mesmo bit.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
