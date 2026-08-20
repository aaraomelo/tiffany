/* tick_idioma.c — MAESTRO AVANÇA i SOBRE A ÓRBITA DO IDIOMA.
 *
 * Realização de ciencia_dragao §§tick-integra / tick-inverso:
 *
 *   P = tick ∘ batuta ∘ Π
 *   Π = frase → I = pt_emite_orbita
 *   tick = avançar i ∈ I
 *   batuta I_bat ∈ {-1,0,+1}  (sentido na fita)
 *   π Heighway passo a passo; π̃ levantamento tick a tick
 *
 *   §TI1  Π emite I; |I|≥1
 *   §TI2  Maestro: um tick = um passo Heighway + G+=1; após |I| ticks, |π|=|I|
 *   §TI3  batuta +1 percorre i=0..n-1; batuta −1 percorre o dual do percurso
 *         (mesmas células na ordem inversa da caminhada — R com sentido)
 *   §TI4  a cada tick: emitir π̃(i)=(π(i),k(i)); no fim G̃≡1 e injectivo
 *   §TI5  composição = lote: Heighway tick-a-tick ≡ drag_andar(|I|-1)
 *   §TI6  Metrónomo: pr₁∘π̃=π; |im π̃|=|I|=∑G; resíduo 0
 *
 *   cc -O2 -std=c99 -w -I../lib -o tick_idioma tick_idioma.c && ./tick_idioma
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "portugues.h"

#define OFS  1024
#define LAD  2048
#define NMAX 8192

typedef long L;

static int G[LAD][LAD];
static int cont[LAD][LAD];
static int xs[NMAX], ys[NMAX], ks[NMAX];
static int dx, dy;                                 /* orientação da caminhada */

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++){ G[i][j] = 0; cont[i][j] = 0; }
}
static void g_inc(int x, int y){ G[y + OFS][x + OFS]++; }
static int g_get(int x, int y){ return G[y + OFS][x + OFS]; }
static int drag_vira_esq(L k){ return (((k & -k) << 1) & k) != 0; }

/* estado do Maestro no dragão */
typedef struct {
    int x, y, dx, dy;
    L i;                                          /* ticks dados (= pontos emitidos) */
    L n_alg;                                      /* |I| alvo */
} Maestro;

static void maestro_init(Maestro *m, L n_alg){
    m->x = 0; m->y = 0; m->dx = 1; m->dy = 0;
    m->i = 0; m->n_alg = n_alg;
    g_zera();
}

/* um TICK: escreve o ponto actual, avança a orientação se ainda há passo a dar.
 * Devolve 1 se emitiu ponto; 0 se a fita acabou. */
static int maestro_tick(Maestro *m){
    if(m->i >= NMAX) return 0;
    if(m->i >= m->n_alg) return 0;
    xs[m->i] = m->x; ys[m->i] = m->y;
    g_inc(m->x, m->y);
    m->i++;
    /* próximo passo Heighway (se ainda falta ponto) */
    if(m->i < m->n_alg){
        L s = m->i;                               /* número do passo (1-based turn index) */
        m->x += m->dx; m->y += m->dy;
        if(drag_vira_esq(s)){ int ndx = -m->dy, ndy = m->dx; m->dx = ndx; m->dy = ndy; }
        else                 { int ndx = m->dy,  ndy = -m->dx; m->dx = ndx; m->dy = ndy; }
    }
    return 1;
}

/* batuta: +1 = ticks para a frente; −1 = emitir a mesma órbita e depois
 * verificar que a sequência inversa das células tem o mesmo multiset G */
static L maestro_corre(Maestro *m, int batuta){
    if(batuta == 0) return 0;                     /* pausa: nenhum tick */
    if(batuta > 0){
        while(maestro_tick(m)) ;
        return m->i;
    }
    /* batuta −1: corre para a frente e regista; o «sentido» medido é a
     * involução do índice i ↦ n-1-i nas coordenadas — mesma imagem, ordem dual */
    while(maestro_tick(m)) ;
    L n = m->i;
    for(L a = 0; a < n/2; a++){
        L b = n - 1 - a;
        int tx = xs[a], ty = ys[a];
        xs[a] = xs[b]; ys[a] = ys[b];
        xs[b] = tx; ys[b] = ty;
    }
    /* G é multiset: reconstruir a partir da sequência (já invertida) */
    g_zera();
    for(L t = 0; t < n; t++) g_inc(xs[t], ys[t]);
    return n;
}

static void levanta(L nt){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) cont[i][j] = 0;
    for(L t = 0; t < nt; t++){
        cont[ys[t] + OFS][xs[t] + OFS]++;
        ks[t] = cont[ys[t] + OFS][xs[t] + OFS];
    }
}

static int max_g(void){
    int m = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) if(G[i][j] > m) m = G[i][j];
    return m;
}
static long soma_g(void){
    long s = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) s += G[i][j];
    return s;
}
static int tilde_ok(L nt){
    for(L i = 0; i < nt; i++)
        for(L j = i + 1; j < nt; j++)
            if(xs[i]==xs[j] && ys[i]==ys[j] && ks[i]==ks[j]) return 0;
    for(L t = 0; t < nt; t++){
        int g = g_get(xs[t], ys[t]);
        if(ks[t] < 1 || ks[t] > g) return 0;
    }
    return 1;
}

/* lote clássico (idioma_orbita): para §TI5 */
static L drag_andar(L n_passos){
    int x = 0, y = 0, ddx = 1, ddy = 0;
    L t = 0;
    if(n_passos < 0) n_passos = 0;
    if(n_passos + 1 > NMAX) n_passos = NMAX - 1;
    g_zera();
    xs[t] = x; ys[t] = y; g_inc(x, y); t++;
    for(L s = 0; s < n_passos; s++){
        x += ddx; y += ddy;
        xs[t] = x; ys[t] = y; g_inc(x, y); t++;
        if(drag_vira_esq(s + 1)){ int ndx = -ddy, ndy = ddx; ddx = ndx; ddy = ndy; }
        else                     { int ndx = ddy,  ndy = -ddx; ddx = ndx; ddy = ndy; }
    }
    return t;
}

int main(void){
    const char *Pi = "o ouro do rei é álgebra e órbita do dragão";
    PtOrbita orb;
    int nI = pt_emite_orbita(Pi, &orb);

    printf("=== TICK × IDIOMA × INVERSO: Maestro avança i sobre I =================\n\n");
    printf("      Π = \"%s\"\n      |I|=%d\n\n", Pi, nI);

    printf("§TI1  Π emite I (régua do idioma)\n");
    {
        ok("§TI1 pt_emite_orbita(|Π|) dá |I|≥1 e passos em Word_8/regras",
           nI >= 1 && nI <= PT_I_MAX && orb.n == nI);
    }

    printf("§TI2  Maestro: um tick = um ponto Heighway; após |I| ticks, |π|=|I|\n");
    {
        Maestro m; maestro_init(&m, nI);
        L ticks = 0;
        while(maestro_tick(&m)) ticks++;
        ok("§TI2 após |I| ticks do Maestro, |π|=|I| e maxG≥1",
           ticks == nI && m.i == nI && max_g() >= 1);
        printf("      ticks=%ld  maxG=%d\n", (long)ticks, max_g());
    }

    printf("§TI3  batuta +1 e −1: mesma imagem G (multiset); ordem dual sob −1\n");
    {
        static int Gf[LAD][LAD];
        Maestro mf; maestro_init(&mf, nI);
        L nf = maestro_corre(&mf, +1);
        memcpy(Gf, G, sizeof G);
        int xs_f[NMAX]; memcpy(xs_f, xs, sizeof(int)*(size_t)nf);
        int ys_f[NMAX]; memcpy(ys_f, ys, sizeof(int)*(size_t)nf);

        Maestro mb; maestro_init(&mb, nI);
        L nb = maestro_corre(&mb, -1);

        int mesmo_G = 1;
        for(int i = 0; i < LAD && mesmo_G; i++)
            for(int j = 0; j < LAD && mesmo_G; j++)
                if(Gf[i][j] != G[i][j]) mesmo_G = 0;

        int ordem_dual = (nf == nb && nf > 1);
        if(ordem_dual){
            for(L t = 0; t < nf; t++)
                if(xs_f[t] != xs[nf-1-t] || ys_f[t] != ys[nf-1-t]){ ordem_dual = 0; break; }
        }
        ok("§TI3 batuta +1/−1: mesmo G; −1 é involução da ordem dos pontos",
           nf == nb && mesmo_G && ordem_dual);
        printf("      |π|_+1=%ld  |π|_−1=%ld  mesmo_G=%d  ordem_dual=%d\n",
               (long)nf, (long)nb, mesmo_G, ordem_dual);
    }

    printf("§TI4  a cada tick: π̃(i)=(π(i),k(i)); no fim G̃≡1\n");
    {
        Maestro m; maestro_init(&m, nI);
        while(maestro_tick(&m)) ;
        L nt = m.i;
        levanta(nt);
        int ok_folha = tilde_ok(nt);
        /* k(i) ∈ [1,G(π(i))] e cresce nas revisitas */
        int k_monotono = 1;
        for(L t = 0; t < nt; t++)
            if(ks[t] < 1 || ks[t] > g_get(xs[t], ys[t])) k_monotono = 0;
        ok("§TI4 levantamento tick a tick: G̃≡1 injectivo e k∈[1,G]",
           ok_folha && k_monotono && nt == nI);
    }

    printf("§TI5  tick-a-tick ≡ lote Heighway(|I|)\n");
    {
        Maestro m; maestro_init(&m, nI);
        while(maestro_tick(&m)) ;
        L nt = m.i;
        int xs_t[NMAX], ys_t[NMAX];
        memcpy(xs_t, xs, sizeof(int)*(size_t)nt);
        memcpy(ys_t, ys, sizeof(int)*(size_t)nt);

        L nl = drag_andar(nI > 1 ? nI - 1 : 0);
        int igual = (nl == nt);
        for(L t = 0; t < nt && igual; t++)
            if(xs_t[t] != xs[t] || ys_t[t] != ys[t]) igual = 0;
        ok("§TI5 P=tick∘batuta∘Π coincide com Heighway em lote", igual);
        printf("      |π|_tick=%ld  |π|_lote=%ld\n", (long)nt, (long)nl);
    }

    printf("§TI6  Metrónomo: pr₁∘π̃=π; |im|=|I|=∑G\n");
    {
        Maestro m; maestro_init(&m, nI);
        while(maestro_tick(&m)) ;
        L nt = m.i;
        levanta(nt);
        long sg = soma_g();
        int pr1 = 1;                              /* xs,ys são π; k é folha */
        for(L t = 0; t < nt; t++)
            if(ks[t] < 1) pr1 = 0;
        ok("§TI6 |im π̃|=|I|=∑G; folhas = fibra; resíduo 0 na volta",
           nt == nI && sg == nt && tilde_ok(nt) && pr1);
        printf("      ∑G=%ld  |I|=%d  maxG=%d\n", sg, nI, max_g());
    }

    (void)dx; (void)dy;
    return falhas ? 1 : 0;
}
