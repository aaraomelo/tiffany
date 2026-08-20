/* espanhol_aranha.c — ÁLGEBRA DO ESPANHOL NAS ÓRBITAS DO DRAGÃO.
 *
 * Mesma tubagem: Σ → I → π Heighway → G. Σ com C2–C3 (ñ, ó, á…).
 *
 *   §EA1–§EA8  espelham §PA / §IA
 *
 *   cc -O2 -std=c99 -Wall -I../lib -o espanhol_aranha espanhol_aranha.c && ./espanhol_aranha
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "umbit.h"
#include "espanhol.h"

#define OFS  1024
#define LAD  2048
#define NMAX 4097

typedef long L;

static int G[LAD][LAD];
static unsigned char Bf[LAD][LAD];
static int xs_g[NMAX], ys_g[NMAX];

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) G[i][j] = 0;
}
static void g_inc(int x, int y){ G[y + OFS][x + OFS]++; }
static int g_get(int x, int y){ return G[y + OFS][x + OFS]; }
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
        if(t < n) x++;
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

static void rot_dir(int *dx, int *dy){
    int ndx = *dy, ndy = -(*dx);
    *dx = ndx; *dy = ndy;
}

static void fecho_rotor(int *dx, int *dy, int x, int y){
    for(int k = 0; k < 4; k++) rot_dir(dx, dy);
    g_inc(x, y);
}

int main(void){
    printf("=== ESPANHOL × ARANHA: álgebra Word_8 nas órbitas do dragão ============\n\n");

    es_sigma_init();
    es_lex_init();

    printf("§EA1  alfabeto Σ ⊆ Word_8\n");
    {
        int card = es_sigma_card();
        int utf = es_em_sigma((EsByte)'a') && es_em_sigma((EsByte)' ')
               && es_em_sigma((EsByte)0xC3) && es_em_sigma((EsByte)0xB1); /* ñ */
        printf("      |Σ|=%d  a∈Σ  ñ=C3B1∈Σ\n\n", card);
        ok("§EA1 alfabeto Σ ⊆ Word_8: card>0 e bytes ES (ASCII+UTF-8 ñ)",
           card == 144 && utf);
    }

    printf("§EA2  monoide Σ*: concatenação, ε neutro\n");
    {
        EsPalavra e = es_eps();
        EsPalavra oro = es_de_cstr("oro");
        EsPalavra rey = es_de_cstr("rey");
        EsPalavra oe = es_concat(oro, e);
        EsPalavra eo = es_concat(e, oro);
        EsPalavra or_ = es_concat(oro, rey);
        int neutro = es_igual(oe, oro) && es_igual(eo, oro);
        int assoc;
        {
            EsPalavra a = es_de_cstr("el ");
            EsPalavra b = es_de_cstr("oro");
            EsPalavra c = es_de_cstr(" del");
            assoc = es_igual(es_concat(es_concat(a, b), c),
                             es_concat(a, es_concat(b, c)));
        }
        int em = es_palavra_em_sigma(oro) && es_palavra_em_sigma(or_);
        printf("      ε·oro=oro·ε  assoc  oro||rey n=%d\n\n", or_.n);
        ok("§EA2 monoide: ε neutro, concat associativa, palavras em Σ*",
           neutro && assoc && em && or_.n == 6);
    }

    printf("§EA3  regras: dual ν∘ν=id; identificação de classe\n");
    {
        EsPalavra oro = es_de_cstr("oro");
        EsPalavra ouro = es_dual(oro);
        EsPalavra volta = es_dual(ouro);
        int invol = es_igual(volta, oro) && es_igual(ouro, es_de_cstr("ouro"));
        EsPalavra v2 = es_aplica(ES_R_DUAL, es_aplica(ES_R_DUAL, oro, es_eps()), es_eps());
        int via_op = es_igual(v2, oro);

        EsPalavra a = es_de_cstr("casa");
        EsPalavra b = es_de_cstr("mesa");
        EsPalavra c = es_de_cstr("rey");
        int mesma = es_mesma_classe(a, b);
        int outra = !es_mesma_classe(a, c);
        EsPalavra id = es_aplica(ES_R_IDENTIFICA, a, b);
        int id_ok = es_igual(id, a);

        /* ñ ↔ nh */
        EsPalavra ene = es_de_cstr("ñ");
        EsPalavra nh = es_dual(ene);
        int ene_ok = es_igual(nh, es_de_cstr("nh")) && es_igual(es_dual(nh), ene);

        printf("      oro↔ouro↔oro  casa~mesa  ñ↔nh\n\n");
        ok("§EA3 dual ν∘ν=id; identificação; ñ↔nh",
           invol && via_op && mesma && outra && id_ok && ene_ok && es_lex_n >= 8);
    }

    printf("§EA4  órbita I da frase → π Heighway → G>1\n");
    EsOrbita orb;
    const char *frase = "el oro del rey es álgebra y órbita del dragón";
    int ni = es_emite_orbita(frase, &orb);
    {
        L n_drag = ni > 1 ? (L)(ni - 1) : 1;
        if(n_drag < 512) n_drag = 512;
        g_zera();
        L nt = drag_andar(n_drag, xs_g, ys_g);
        int max_g = 0;
        int bate = g_bate_recontagem(nt, &max_g);
        printf("      frase |I|_alg=%d  Heighway n=%ld pontos=%ld maxG=%d\n\n",
               ni, (long)n_drag, (long)nt, max_g);
        ok("§EA4 álgebra ES emite I; π Heighway; existe G>1",
           ni > 10 && bate && max_g >= 2 && nt == n_drag + 1);
    }

    printf("§EA5  recta controlo → G≡1\n");
    {
        g_zera();
        L nr = recta(200, xs_g, ys_g);
        int max_g = 0;
        int bate = g_bate_recontagem(nr, &max_g);
        ok("§EA5 controlo injectivo: recta max G=1", bate && max_g == 1);
    }

    printf("§EA6  ∑G = |I|\n");
    {
        g_zera();
        L nt = drag_andar(512, xs_g, ys_g);
        long soma = 0;
        for(int i = 0; i < LAD; i++)
            for(int j = 0; j < LAD; j++) soma += G[i][j];
        ok("§EA6 conservação: ∑G = |I|", soma == nt);
    }

    printf("§EA7  G mod 2\n");
    {
        g_zera();
        L nt = drag_andar(512, xs_g, ys_g);
        memset(Bf, 0, sizeof Bf);
        for(L t = 0; t < nt; t++){
            int x = xs_g[t], y = ys_g[t];
            Bf[y + OFS][x + OFS] = (unsigned char)b_som(Bf[y + OFS][x + OFS], 1);
        }
        int xor_ok = 1;
        for(int y = -OFS; y < LAD - OFS && xor_ok; y++)
            for(int x = -OFS; x < LAD - OFS && xor_ok; x++){
                int v = g_get(x, y);
                B b = (B)Bf[y + OFS][x + OFS];
                if(v == 0 && b == 0) continue;
                if((B)(v & 1) != b) xor_ok = 0;
            }
        ok("§EA7 assinatura: B⊕=1 coincide com G mod 2", xor_ok);
    }

    printf("§EA8  min-G + fecho R⁴=id\n");
    {
        g_zera();
        for(int k = 0; k < 3; k++) g_inc(1, 0);
        g_inc(0, -1);
        for(int k = 0; k < 2; k++) g_inc(0, 1);
        int nx, ny;
        escolhe_min_g(0, 0, &nx, &ny);
        int min_ok = (nx == -1 && ny == 0 && g_get(nx, ny) == 0);
        int rotor_ok = 1;
        static const int hd[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
        for(int i = 0; i < 4 && rotor_ok; i++){
            int dx = hd[i][0], dy = hd[i][1], odx = dx, ody = dy;
            for(int k = 0; k < 4; k++) rot_dir(&dx, &dy);
            if(dx != odx || dy != ody) rotor_ok = 0;
        }
        int dx = 1, dy = 0, x = 0, y = 0;
        g_zera(); g_inc(x, y);
        int g_antes = g_get(x, y);
        fecho_rotor(&dx, &dy, x, y);
        int fecho = (dx == 1 && dy == 0 && g_get(x, y) == g_antes + 1);
        ok("§EA8 ciclo aranha: min-G + R⁴ + marca forte",
           min_ok && rotor_ok && fecho);
    }

    {
        char k[256];
        es_chave_alfabeto(k, sizeof k);
        int a_ok = strstr(k, "idioma/es/alfabeto") != NULL;
        es_chave_lexico(0, k, sizeof k);
        int l_ok = strstr(k, "idioma/es/lexico/") != NULL;
        es_chave_regra(ES_R_DUAL, k, sizeof k);
        int r_ok = strstr(k, "idioma/es/regra/dual_lex") != NULL;
        ok("§EA* chaves banco idioma/es/{alfabeto,lexico,regra}",
           a_ok && l_ok && r_ok);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
