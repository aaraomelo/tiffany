/* aranha_inversa.c — LEVANTAMENTO DA ARANHA: curva do dragão com G̃≡1.
 *
 * Teor. thm:aranha-inversa (arquitetura §sec:aranha):
 *
 *   π: I → ℤ² (Heighway, G pode >1)
 *   k(i) = nº da visita a π(i) até i
 *   π̃(i) = (π(i), k(i)) ∈ ℤ²×ℕ  →  injectivo → G̃≡1
 *   pr₁∘π̃ = π  (projecção recupera o dragão)
 *
 * A métrica |det|=1 conserva medida e perde fibra (§M10 pata).
 * O levantamento restaura a fibra como folha.
 *
 *   §AI1  dragão dobra: max G>1
 *   §AI2  levantamento injectivo: nenhum par (x,k) repetido
 *   §AI3  G̃≡1 em todo o percurso levantado
 *   §AI4  pr₁∘π̃ = π (resíduo 0)
 *   §AI5  |im π̃| = |I| = ∑G
 *   §AI6  recta: k≡1 já na base (folha única; degenerado)
 *   §AI7  órbita específica: sequências iguais ⇒ mesmo π̃
 *
 *   cc -O2 -std=c99 -w -I../lib -o aranha_inversa aranha_inversa.c && ./aranha_inversa
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define OFS  1024
#define LAD  2048
#define NMAX 4097
#define KMAX 64

typedef long L;

static int G[LAD][LAD];
static int xs[NMAX], ys[NMAX];
static int ks[NMAX];
static int cont[LAD][LAD];

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++){ G[i][j] = 0; cont[i][j] = 0; }
}

static void g_inc(int x, int y){ G[y + OFS][x + OFS]++; }
static int g_get(int x, int y){ return G[y + OFS][x + OFS]; }

static int drag_vira_esq(L k){ return (((k & -k) << 1) & k) != 0; }

static L drag_andar(L n){
    int x = 0, y = 0, dx = 1, dy = 0;
    L t = 0;
    xs[t] = x; ys[t] = y; g_inc(x, y); t++;
    for(L s = 0; s < n; s++){
        x += dx; y += dy;
        xs[t] = x; ys[t] = y; g_inc(x, y); t++;
        if(drag_vira_esq(s + 1)){ int ndx = -dy, ndy = dx; dx = ndx; dy = ndy; }
        else                     { int ndx = dy,  ndy = -dx; dx = ndx; dy = ndy; }
    }
    return t;
}

static L recta(L n){
    int x = 0, y = 0;
    for(L t = 0; t <= n; t++){
        xs[t] = x; ys[t] = y; g_inc(x, y);
        if(t < n) x++;
    }
    return n + 1;
}

/* algoritmo inverso: k(i) = visita corrente; π̃ = (x,y,k) */
static void levanta(L nt){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) cont[i][j] = 0;
    for(L t = 0; t < nt; t++){
        int x = xs[t], y = ys[t];
        cont[y + OFS][x + OFS]++;
        ks[t] = cont[y + OFS][x + OFS];
    }
}

static int max_g(void){
    int m = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++)
            if(G[i][j] > m) m = G[i][j];
    return m;
}

static long soma_g(void){
    long s = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) s += G[i][j];
    return s;
}

/* injectividade de π̃: pares (x,y,k) todos distintos */
static int tilde_injectivo(L nt){
    for(L i = 0; i < nt; i++)
        for(L j = i + 1; j < nt; j++)
            if(xs[i] == xs[j] && ys[i] == ys[j] && ks[i] == ks[j])
                return 0;
    return 1;
}

/* G̃≡1: cada (x,y,k) aparece uma vez — equivalente a injectividade + |im|=nt */
static int tilde_g_um(L nt){
    if(!tilde_injectivo(nt)) return 0;
    /* k(i) ∈ 1..G(π(i)) e cobre exactamente */
    for(L t = 0; t < nt; t++){
        int g = g_get(xs[t], ys[t]);
        if(ks[t] < 1 || ks[t] > g) return 0;
    }
    return 1;
}

int main(void){
    printf("=== ARANHA INVERSA: levantamento em folhas, G̃≡1 ======================\n\n");

    L n_drag = 512;
    g_zera();
    L nt = drag_andar(n_drag);
    int mg = max_g();
    printf("§AI1  dragão n=%ld: |I|=%ld maxG=%d\n\n", (long)n_drag, (long)nt, mg);
    ok("§AI1 dragão dobra na base: max G>1 (há fibra a levantar)",
       mg >= 2 && nt == n_drag + 1);

    levanta(nt);
    printf("§AI2  levantamento π̃(i)=(π(i),k(i))\n\n");
    ok("§AI2 π̃ injectivo: nenhum vértice (x,y,k) repetido",
       tilde_injectivo(nt));

    ok("§AI3 G̃≡1: cada vértice levantado tem multiplicidade 1",
       tilde_g_um(nt));

    /* pr₁∘π̃ = π por construção; a fibra vertical recupera G */
    {
        int fibra_ok = 1;
        for(L t = 0; t < nt && fibra_ok; t++){
            int g = g_get(xs[t], ys[t]);
            /* visitas a esta célula: ks devem ser permutação de 1..g */
            int visto[KMAX];
            memset(visto, 0, sizeof visto);
            for(L s = 0; s < nt; s++){
                if(xs[s] != xs[t] || ys[s] != ys[t]) continue;
                if(ks[s] < 1 || ks[s] > g || ks[s] >= KMAX){ fibra_ok = 0; break; }
                visto[ks[s]]++;
            }
            for(int k = 1; k <= g && fibra_ok; k++)
                if(visto[k] != 1) fibra_ok = 0;
        }
        printf("§AI4  pr₁∘π̃=π; fibra vertical em cada x é {1..G(x)}\n\n");
        ok("§AI4 projecção recupera o dragão; folhas = fibra que a pata perdia",
           fibra_ok);
    }

    {
        long sg = soma_g();
        printf("§AI5  |im π̃|=|I|=%ld  ∑G=%ld\n\n", (long)nt, sg);
        ok("§AI5 conservação: |im π̃| = |I| = ∑G (métrica de contagem)",
           sg == nt && tilde_injectivo(nt));
    }

    /* recta: degenerado — folha única */
    {
        g_zera();
        L nr = recta(200);
        levanta(nr);
        int k1 = 1;
        for(L t = 0; t < nr && k1; t++) if(ks[t] != 1) k1 = 0;
        printf("§AI6  recta: maxG=%d  k≡1? %s\n\n", max_g(), k1 ? "sim" : "nao");
        ok("§AI6 recta = caso degenerado: G≡1 na base e k≡1 (folha única)",
           max_g() == 1 && k1 && tilde_injectivo(nr));
    }

    /* órbita específica: mesma sequência ⇒ mesmo levantamento */
    {
        g_zera();
        L n1 = drag_andar(128);
        levanta(n1);
        int k_a[NMAX];
        memcpy(k_a, ks, sizeof(int) * (size_t)n1);
        int x_a[NMAX], y_a[NMAX];
        memcpy(x_a, xs, sizeof(int) * (size_t)n1);
        memcpy(y_a, ys, sizeof(int) * (size_t)n1);

        g_zera();
        L n2 = drag_andar(128);
        levanta(n2);
        int mesmo = (n1 == n2);
        for(L t = 0; t < n1 && mesmo; t++)
            if(xs[t] != x_a[t] || ys[t] != y_a[t] || ks[t] != k_a[t]) mesmo = 0;
        printf("§AI7  órbita Heighway n=128: levantamento reprodutível? %s\n\n",
               mesmo ? "sim" : "nao");
        ok("§AI7 dada a órbita, o levantamento é determinado (mesma π̃)",
           mesmo && tilde_g_um(n2));
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
