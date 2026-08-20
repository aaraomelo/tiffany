/* aranha_g.c — MULTIPLICIDADE GEOMÉTRICA G: o Teor. thm:multiplicidade medido na grade.
 *
 *   trajetória I --π--> Z² --|π⁻¹|--> G(x)
 *
 *   §AG1  G(x) = |π⁻¹(x)| — recontagem bate o campo incremental
 *   §AG2  a curva do dragão (Heighway) DOBRA: existe célula com G>1
 *   §AG3  controlo injectivo (recta): G≡1 em todo o percurso
 *   §AG4  paridade: G mod 2 conserva a assinatura da dobra (corolário GF(2))
 *   §AG5  ∑ G = |I| — cada índice conta uma vez no ambiente
 *   §AG6  descida GF(2): G=0 e G=2 colidem em B=0 — perde-se a profundidade
 *   §AG7  escrita incremental B⊕=1 bate G mod 2 (mesma soma de cadeia.c §K4)
 *   §AG8  ciclo aranha: decisão prefere vizinho de menor G (cláusula 4 do teorema)
 *   §AG9  fecho Lei 2: rotação à direita R⁴=id no heading (J⁴=id na grade)
 *   §AG10 caminhada estigmérgica N passos: só lê G, escreve G+=1, sem memória interna
 *   §AG11 fecho: R⁴ restaura heading e marca forte incrementa G na célula actual
 *
 *   cc -O2 -std=c99 -w -I../lib -o aranha_g aranha_g.c && ./aranha_g
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "umbit.h"

#define OFS  1024
#define LAD  2048
#define NMAX 4097

typedef long L;

static int G[LAD][LAD];
static unsigned char Bf[LAD][LAD];

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) G[i][j] = 0;
}

static void g_inc(int x, int y){
    G[y + OFS][x + OFS]++;
}

static int g_get(int x, int y){
    return G[y + OFS][x + OFS];
}

static int xs_g[NMAX], ys_g[NMAX];

/* Heighway: no passo k≥1, esquerda sse (((k & -k) << 1) & k) ≠ 0 */
static int drag_vira_esq(L k){ return (((k & -k) << 1) & k) != 0; }

static L drag_andar(L n, int *xs, int *ys){
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

static L recta(L n, int *xs, int *ys){
    int x = 0, y = 0;
    for(L t = 0; t <= n; t++){
        xs[t] = x; ys[t] = y; g_inc(x, y);
        if(t < n){ x++; }
    }
    return n + 1;
}

static int g_bate_recontagem(L nt, int *max_g){
    int ok_cnt = 1, mg = 0;
    for(int y = -OFS; y < LAD - OFS; y++){
        for(int x = -OFS; x < LAD - OFS; x++){
            int v = g_get(x, y);
            if(v <= 0) continue;
            int c = 0;
            for(L t = 0; t < nt; t++)
                if(xs_g[t] == x && ys_g[t] == y) c++;
            if(c != v) ok_cnt = 0;
            if(v > mg) mg = v;
        }
    }
    *max_g = mg;
    return ok_cnt;
}

static L drag_andar_xor(L n){
    int x = 0, y = 0, dx = 1, dy = 0;
    memset(Bf, 0, sizeof Bf);
    Bf[y + OFS][x + OFS] = (unsigned char)b_som(Bf[y + OFS][x + OFS], 1);
    for(L s = 0; s < n; s++){
        x += dx; y += dy;
        Bf[y + OFS][x + OFS] = (unsigned char)b_som(Bf[y + OFS][x + OFS], 1);
        if(drag_vira_esq(s + 1)){ int ndx = -dy, ndy = dx; dx = ndx; dy = ndy; }
        else                     { int ndx = dy,  ndy = -dx; dx = ndx; dy = ndy; }
    }
    return n + 1;
}

static void escolhe_min_g(int x, int y, int *nx, int *ny){
    static const int dx[4] = {1, 0, -1, 0};
    static const int dy[4] = {0, 1, 0, -1};
    int best = 1 << 30, bx = x, by = y;
    for(int k = 0; k < 4; k++){
        int gx = x + dx[k], gy = y + dy[k], v = g_get(gx, gy);
        if(v < best){ best = v; bx = gx; by = gy; }
    }
    *nx = bx; *ny = by;
}

static int g_get_b(int x, int y){
    if(x <= -OFS + 2 || x >= LAD - OFS - 2) return 1 << 29;
    if(y <= -OFS + 2 || y >= LAD - OFS - 2) return 1 << 29;
    return g_get(x, y);
}

static void rot_dir(int *dx, int *dy){
    int ndx = *dy, ndy = -(*dx);
    *dx = ndx; *dy = ndy;
}

static void escolhe_min_g_rel(int x, int y, int dx, int dy, int *nx, int *ny){
    int cx[4] = {x + dx, x + dy, x - dx, x - dy};
    int cy[4] = {y + dy, y - dx, y - dy, y + dx};
    int best = 1 << 30, bx = x, by = y;
    for(int k = 0; k < 4; k++){
        int v = g_get_b(cx[k], cy[k]);
        if(v < best){ best = v; bx = cx[k]; by = cy[k]; }
    }
    *nx = bx; *ny = by;
}

static void fecho_rotor(int *dx, int *dy, int x, int y){
    for(int k = 0; k < 4; k++) rot_dir(dx, dy);
    g_inc(x, y);
}

static L aranha_caminha(L n_expl, int *fx, int *fy, int *fdx, int *fdy){
    int x = 0, y = 0, dx = 1, dy = 0;
    g_inc(x, y);
    for(L s = 0; s < n_expl; s++){
        int nx, ny;
        escolhe_min_g_rel(x, y, dx, dy, &nx, &ny);
        dx = nx - x; dy = ny - y;
        x = nx; y = ny;
        g_inc(x, y);
    }
    *fx = x; *fy = y; *fdx = dx; *fdy = dy;
    return n_expl + 1;
}

int main(void){
    printf("=== ARANHA G: multiplicidade geométrica |pi^{-1}| na grade =================\n\n");

    L n_drag = 512;
    g_zera();
    L nt = drag_andar(n_drag, xs_g, ys_g);
    int max_g = 0;
    int bate = g_bate_recontagem(nt, &max_g);
    printf("§AG1  dragão n=%ld: pontos=%ld, max G=%d, recontagem bate? %s\n\n",
           (long)n_drag, (long)nt, max_g, bate ? "sim" : "nao");
    ok("§AG1 G(x)=|pi^{-1}(x)|: o campo incremental coincide com a recontagem dos índices",
       bate && nt == n_drag + 1);

    ok("§AG2 duplicidade na curva do dragão: existe célula com G>1 (dobra geométrica)",
       max_g >= 2);

    g_zera();
    L nr = recta(200, xs_g, ys_g);
    max_g = 0;
    bate = g_bate_recontagem(nr, &max_g);
    printf("§AG3  recta n=200: max G=%d\n\n", max_g);
    ok("§AG3 controlo injectivo: recta sem revisita — max G=1 (fibra trivial)",
       bate && max_g == 1);

    /* §AG4 paridade */
    g_zera();
    drag_andar(n_drag, xs_g, ys_g);
    int par_ok = 1;
    for(int y = -OFS; y < LAD - OFS && par_ok; y++){
        for(int x = -OFS; x < LAD - OFS && par_ok; x++){
            int v = g_get(x, y);
            if(v <= 0) continue;
            int c = 0;
            for(L t = 0; t < nt; t++)
                if(xs_g[t] == x && ys_g[t] == y) c++;
            if((v & 1) != (c & 1)) par_ok = 0;
        }
    }
    ok("§AG4 paridade: G mod 2 coincide com |pi^{-1}| mod 2 (assinatura binária da dobra)",
       par_ok);

    /* §AG5 conservação de índices */
    long soma = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) soma += G[i][j];
    ok("§AG5 suma G = |I|: cada passo escreve exactamente um índice no ambiente",
       soma == nt);

    /* §AG6 descida GF(2): profundidade perdida — B=0 tanto para G=0 como G=2 */
    g_zera();
    drag_andar(n_drag, xs_g, ys_g);
    int tem_nunca = 0, tem_dobro = 0, par_ok2 = 1;
    for(int y = -OFS; y < LAD - OFS; y++){
        for(int x = -OFS; x < LAD - OFS; x++){
            int v = g_get(x, y);
            if(v == 0) tem_nunca = 1;
            if(v == 2){
                tem_dobro = 1;
                if((v & 1) != 0) par_ok2 = 0;
            }
        }
    }
    printf("§AG6  dragão: célula nunca visitada e célula com G=2 partilham B=0\n\n");
    ok("§AG6 descida GF(2): G=0 e G=2 colidem em B=0 — a profundidade da dobra desaparece",
       tem_nunca && tem_dobro && par_ok2);

    /* §AG7 escrita incremental com XOR (= soma GF(2)) bate G mod 2 */
    g_zera();
    drag_andar(n_drag, xs_g, ys_g);
    drag_andar_xor(n_drag);
    int xor_ok = 1;
    for(int y = -OFS; y < LAD - OFS && xor_ok; y++){
        for(int x = -OFS; x < LAD - OFS && xor_ok; x++){
            int v = g_get(x, y);
            B b = (B)Bf[y + OFS][x + OFS];
            if(v == 0 && b == 0) continue;
            if((B)(v & 1) != b) xor_ok = 0;
        }
    }
    ok("§AG7 descida incremental: B_{t+1}=B_t⊕1_{π(t)} coincide com G mod 2 (cadeia §K4)",
       xor_ok);

    /* §AG8 ciclo aranha: lê vizinhança e prefere menor G */
    g_zera();
    for(int k = 0; k < 3; k++) g_inc(1, 0);   /* E: G=3 */
    g_inc(0, -1);                                /* N: G=1 */
    for(int k = 0; k < 2; k++) g_inc(0, 1);     /* S: G=2 */
    /* O: G=0 */
    int nx, ny;
    escolhe_min_g(0, 0, &nx, &ny);
    printf("§AG8  vizinho escolhido (%d,%d) com G=%d (menor entre N=1 E=3 S=2 O=0)\n\n",
           nx, ny, g_get(nx, ny));
    ok("§AG8 ciclo aranha: decisão prefere vizinho de menor G — lê a dobra, não pergunta",
       nx == -1 && ny == 0 && g_get(nx, ny) == 0);

    /* §AG9 rotor Lei 2: R⁴ = id em todos os headings cardinais */
    int rotor_ok = 1;
    static const int hd[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    for(int i = 0; i < 4 && rotor_ok; i++){
        int dx = hd[i][0], dy = hd[i][1], odx = dx, ody = dy;
        for(int k = 0; k < 4; k++) rot_dir(&dx, &dy);
        if(dx != odx || dy != ody) rotor_ok = 0;
    }
    ok("§AG9 fecho Lei 2: rotação à direita quatro vezes devolve o heading (R⁴=id)",
       rotor_ok);

    /* §AG10 caminhada estigmérgica completa — memória só em G */
    g_zera();
    int fx, fy, fdx, fdy;
    L n_expl = 200;
    L np = aranha_caminha(n_expl, &fx, &fy, &fdx, &fdy);
    long soma2 = 0;
    int celulas = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++){
            soma2 += G[i][j];
            if(G[i][j] > 0) celulas++;
        }
    printf("§AG10 caminhada n=%ld: pontos=%ld, células distintas=%d, fim=(%d,%d)\n\n",
           (long)n_expl, (long)np, celulas, fx, fy);
    ok("§AG10 caminhada estigmérgica: N passos escrevem exactamente N+1 índices em G (sem história interna)",
       np == n_expl + 1 && soma2 == np && celulas > 10);

    /* §AG11 fecho: R⁴ + marca forte na célula actual */
    int g_antes = g_get(fx, fy), odx = fdx, ody = fdy;
    fecho_rotor(&fdx, &fdy, fx, fy);
    printf("§AG11  fecho em (%d,%d): G %d→%d, heading (%d,%d) inalterado\n\n",
           fx, fy, g_antes, g_get(fx, fy), fdx, fdy);
    ok("§AG11 fecho do ciclo: R⁴ restaura heading e a marca forte faz G+=1 na célula actual",
       fdx == odx && fdy == ody && g_get(fx, fy) == g_antes + 1);

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
