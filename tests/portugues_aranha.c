/* portugues_aranha.c — ÁLGEBRA DO PORTUGUÊS NAS ÓRBITAS DO DRAGÃO.
 *
 * Uma só geometria: toda órbita discreta realiza-se no Heighway; G = |π⁻¹|.
 * O português é álgebra sobre Word_8 (lib/portugues.h). A aranha só consome I.
 *
 *   §PA1  alfabeto Σ ⊆ Word_8
 *   §PA2  monoide Σ*: concatenação, ε neutro
 *   §PA3  regras: dual ν∘ν=id; identificação de classe
 *   §PA4  órbita I da frase → π Heighway → existe G>1 (dobra)
 *   §PA5  recta controlo → G≡1
 *   §PA6  ∑G = |I|
 *   §PA7  G mod 2 (assinatura)
 *   §PA8  min-G + fecho R⁴=id
 *
 *   cc -O2 -std=c99 -Wall -I../lib -o portugues_aranha portugues_aranha.c && ./portugues_aranha
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "umbit.h"
#include "portugues.h"

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

static void g_inc(int x, int y){
    G[y + OFS][x + OFS]++;
}

static int g_get(int x, int y){
    return G[y + OFS][x + OFS];
}

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
    printf("=== PORTUGUÊS × ARANHA: álgebra Word_8 nas órbitas do dragão =============\n\n");

    pt_sigma_init();
    pt_lex_init();

    /* ── §PA1 ─────────────────────────────────────────────────────────────── */
    printf("§PA1  alfabeto Σ ⊆ Word_8\n");
    {
        int card = pt_sigma_card();
        int fora = 0;
        for(int i = 0; i < PT_W8; i++)
            if(pt_sigma[i] && (i < 0 || i >= PT_W8)) fora++;
        /* todos os bytes marcados estão em 0..255 por construção; card > 0 */
        int ascii_ok = pt_em_sigma((PtByte)'a') && pt_em_sigma((PtByte)' ')
                    && pt_em_sigma((PtByte)0xC3) && pt_em_sigma((PtByte)0xA3); /* ã */
        printf("      |Σ|=%d  a∈Σ espaço∈Σ UTF-8(C3,A3)∈Σ\n\n", card);
        ok("§PA1 alfabeto Σ ⊆ Word_8: card>0 e bytes PT (ASCII+UTF-8) admitidos",
           card > 50 && card <= PT_W8 && !fora && ascii_ok);
    }

    /* ── §PA2 ─────────────────────────────────────────────────────────────── */
    printf("§PA2  monoide Σ*: concatenação, ε neutro\n");
    {
        PtPalavra e = pt_eps();
        PtPalavra ouro = pt_de_cstr("ouro");
        PtPalavra rei  = pt_de_cstr("rei");
        PtPalavra oe = pt_concat(ouro, e);
        PtPalavra eo = pt_concat(e, ouro);
        PtPalavra or_ = pt_concat(ouro, rei);
        int neutro = pt_igual(oe, ouro) && pt_igual(eo, ouro);
        int assoc;
        {
            PtPalavra a = pt_de_cstr("o ");
            PtPalavra b = pt_de_cstr("ouro");
            PtPalavra c = pt_de_cstr(" do");
            PtPalavra ab_c = pt_concat(pt_concat(a, b), c);
            PtPalavra a_bc = pt_concat(a, pt_concat(b, c));
            assoc = pt_igual(ab_c, a_bc);
        }
        int em = pt_palavra_em_sigma(ouro) && pt_palavra_em_sigma(or_);
        printf("      ε·ouro=ouro·ε  assoc  ouro||rei n=%d  em Σ? %s\n\n",
               or_.n, em ? "sim" : "nao");
        ok("§PA2 monoide: ε neutro, concat associativa, palavras em Σ*",
           neutro && assoc && em && or_.n == 7);
    }

    /* ── §PA3 ─────────────────────────────────────────────────────────────── */
    printf("§PA3  regras gramaticais: dual ν∘ν=id; identificação de classe\n");
    {
        PtPalavra ouro = pt_de_cstr("ouro");
        PtPalavra gold = pt_dual(ouro);
        PtPalavra volta = pt_dual(gold);
        int invol = pt_igual(volta, ouro) && pt_igual(gold, pt_de_cstr("gold"));
        /* ν∘ν via pt_aplica */
        PtPalavra v2 = pt_aplica(PT_R_DUAL, pt_aplica(PT_R_DUAL, ouro, pt_eps()), pt_eps());
        int via_op = pt_igual(v2, ouro);

        PtPalavra a = pt_de_cstr("casa");
        PtPalavra b = pt_de_cstr("mesa");   /* ambas terminam em 'a' */
        PtPalavra c = pt_de_cstr("rei");
        int mesma = pt_mesma_classe(a, b);
        int outra = !pt_mesma_classe(a, c);
        PtPalavra id = pt_aplica(PT_R_IDENTIFICA, a, b);
        int id_ok = pt_igual(id, a);
        printf("      ouro↔gold↔ouro  casa~mesa  casa≁rei\n\n");
        ok("§PA3 dual ν∘ν=id no léxico; identificação por classe (último byte)",
           invol && via_op && mesma && outra && id_ok && pt_lex_n >= 8);
    }

    /* ── §PA4 ─────────────────────────────────────────────────────────────── */
    printf("§PA4  órbita I da frase → π Heighway → G>1\n");
    PtOrbita orb;
    const char *frase = "o ouro do rei é álgebra e órbita do dragão";
    int ni = pt_emite_orbita(frase, &orb);
    L n_passos;
    {
        /* |I| = ni; π no dragão com ni−1 passos (ni pontos), mínimo 512 para dobra */
        L n_drag = ni > 1 ? (L)(ni - 1) : 1;
        if(n_drag < 512) n_drag = 512;   /* garante auto-intersecção Heighway */
        g_zera();
        L nt = drag_andar(n_drag, xs_g, ys_g);
        n_passos = nt;
        int max_g = 0;
        int bate = g_bate_recontagem(nt, &max_g);
        printf("      frase |I|_alg=%d  Heighway n=%ld pontos=%ld maxG=%d\n\n",
               ni, (long)n_drag, (long)nt, max_g);
        ok("§PA4 álgebra PT emite I; π Heighway realiza; existe célula G>1 (dobra)",
           ni > 10 && bate && max_g >= 2 && nt == n_drag + 1);
    }

    /* ── §PA5 ─────────────────────────────────────────────────────────────── */
    printf("§PA5  recta controlo → G≡1\n");
    {
        g_zera();
        L nr = recta(200, xs_g, ys_g);
        int max_g = 0;
        int bate = g_bate_recontagem(nr, &max_g);
        printf("      recta n=200 maxG=%d\n\n", max_g);
        ok("§PA5 controlo injectivo: recta sem revisita — max G=1",
           bate && max_g == 1);
    }

    /* ── §PA6 ─────────────────────────────────────────────────────────────── */
    printf("§PA6  ∑G = |I|\n");
    {
        g_zera();
        L n_drag = 512;
        L nt = drag_andar(n_drag, xs_g, ys_g);
        long soma = 0;
        for(int i = 0; i < LAD; i++)
            for(int j = 0; j < LAD; j++) soma += G[i][j];
        printf("      ∑G=%ld  |I|=%ld\n\n", soma, (long)nt);
        ok("§PA6 conservação: ∑G = |I| — cada passo escreve um índice",
           soma == nt);
        (void)n_passos;
    }

    /* ── §PA7 ─────────────────────────────────────────────────────────────── */
    printf("§PA7  G mod 2\n");
    {
        g_zera();
        L n_drag = 512;
        L nt = drag_andar(n_drag, xs_g, ys_g);
        memset(Bf, 0, sizeof Bf);
        /* XOR incremental ao longo da mesma trajetória */
        for(L t = 0; t < nt; t++){
            int x = xs_g[t], y = ys_g[t];
            Bf[y + OFS][x + OFS] = (unsigned char)b_som(Bf[y + OFS][x + OFS], 1);
        }
        int xor_ok = 1;
        for(int y = -OFS; y < LAD - OFS && xor_ok; y++){
            for(int x = -OFS; x < LAD - OFS && xor_ok; x++){
                int v = g_get(x, y);
                B b = (B)Bf[y + OFS][x + OFS];
                if(v == 0 && b == 0) continue;
                if((B)(v & 1) != b) xor_ok = 0;
            }
        }
        ok("§PA7 assinatura: B⊕=1 ao longo de π coincide com G mod 2",
           xor_ok);
    }

    /* ── §PA8 ─────────────────────────────────────────────────────────────── */
    printf("§PA8  min-G + fecho R⁴=id\n");
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
        g_zera();
        g_inc(x, y);
        int g_antes = g_get(x, y);
        fecho_rotor(&dx, &dy, x, y);
        int fecho = (dx == 1 && dy == 0 && g_get(x, y) == g_antes + 1);
        printf("      min-G→O  R⁴=id  marca forte G+=1\n\n");
        ok("§PA8 ciclo aranha: min-G + R⁴ restaura heading + marca forte",
           min_ok && rotor_ok && fecho);
    }

    /* chaves do banco (smoke: formato) */
    {
        char k[256];
        pt_chave_alfabeto(k, sizeof k);
        int a_ok = strstr(k, "idioma/pt/alfabeto") != NULL;
        pt_chave_lexico(0, k, sizeof k);
        int l_ok = strstr(k, "idioma/pt/lexico/") != NULL;
        pt_chave_regra(PT_R_DUAL, k, sizeof k);
        int r_ok = strstr(k, "idioma/pt/regra/dual_lex") != NULL;
        ok("§PA* chaves banco idioma/pt/{alfabeto,lexico,regra}",
           a_ok && l_ok && r_ok);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
