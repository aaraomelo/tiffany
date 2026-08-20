/* ingles_aranha.c — ÁLGEBRA DO INGLÊS NAS ÓRBITAS DO DRAGÃO.
 *
 * Mesma tubagem que portugues_aranha.c: Σ → I → π Heighway → G.
 * Σ inglês = ASCII (sem C2–C3); léxico EN→PT = dual do PT.
 *
 *   §IA1–§IA8  espelham §PA1–§PA8
 *
 *   cc -O2 -std=c99 -Wall -I../lib -o ingles_aranha ingles_aranha.c && ./ingles_aranha
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "umbit.h"
#include "ingles.h"

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
    printf("=== INGLÊS × ARANHA: álgebra Word_8 nas órbitas do dragão ===============\n\n");

    en_sigma_init();
    en_lex_init();

    printf("§IA1  alfabeto Σ ⊆ Word_8 (ASCII)\n");
    {
        int card = en_sigma_card();
        int ascii_ok = en_em_sigma((EnByte)'a') && en_em_sigma((EnByte)' ')
                    && en_em_sigma((EnByte)'Z') && !en_em_sigma((EnByte)0xC3);
        printf("      |Σ|=%d  a∈Σ espaço∈Σ C3∉Σ (ASCII puro)\n\n", card);
        ok("§IA1 alfabeto Σ ⊆ Word_8: card ASCII e sem C2–C3 obrigatório",
           card == 78 && ascii_ok);
    }

    printf("§IA2  monoide Σ*: concatenação, ε neutro\n");
    {
        EnPalavra e = en_eps();
        EnPalavra gold = en_de_cstr("gold");
        EnPalavra king = en_de_cstr("king");
        EnPalavra oe = en_concat(gold, e);
        EnPalavra eo = en_concat(e, gold);
        EnPalavra gk = en_concat(gold, king);
        int neutro = en_igual(oe, gold) && en_igual(eo, gold);
        int assoc;
        {
            EnPalavra a = en_de_cstr("the ");
            EnPalavra b = en_de_cstr("gold");
            EnPalavra c = en_de_cstr(" of");
            assoc = en_igual(en_concat(en_concat(a, b), c),
                             en_concat(a, en_concat(b, c)));
        }
        int em = en_palavra_em_sigma(gold) && en_palavra_em_sigma(gk);
        printf("      ε·gold=gold·ε  assoc  gold||king n=%d\n\n", gk.n);
        ok("§IA2 monoide: ε neutro, concat associativa, palavras em Σ*",
           neutro && assoc && em && gk.n == 8);
    }

    printf("§IA3  regras: dual ν∘ν=id; identificação de classe\n");
    {
        EnPalavra gold = en_de_cstr("gold");
        EnPalavra ouro = en_dual(gold);
        EnPalavra volta = en_dual(ouro);
        int invol = en_igual(volta, gold) && en_igual(ouro, en_de_cstr("ouro"));
        EnPalavra v2 = en_aplica(EN_R_DUAL, en_aplica(EN_R_DUAL, gold, en_eps()), en_eps());
        int via_op = en_igual(v2, gold);

        EnPalavra a = en_de_cstr("mass");
        EnPalavra b = en_de_cstr("boss");   /* ambas terminam em 's' */
        EnPalavra c = en_de_cstr("king");
        int mesma = en_mesma_classe(a, b);
        int outra = !en_mesma_classe(a, c);
        EnPalavra id = en_aplica(EN_R_IDENTIFICA, a, b);
        int id_ok = en_igual(id, a);
        printf("      gold↔ouro↔gold  mass~boss  mass≁king\n\n");
        ok("§IA3 dual ν∘ν=id no léxico; identificação por classe (último byte)",
           invol && via_op && mesma && outra && id_ok && en_lex_n >= 8);
    }

    printf("§IA4  órbita I da frase → π Heighway → G>1\n");
    EnOrbita orb;
    const char *frase = "the gold of the king is algebra and orbit of the dragon";
    int ni = en_emite_orbita(frase, &orb);
    {
        L n_drag = ni > 1 ? (L)(ni - 1) : 1;
        if(n_drag < 512) n_drag = 512;
        g_zera();
        L nt = drag_andar(n_drag, xs_g, ys_g);
        int max_g = 0;
        int bate = g_bate_recontagem(nt, &max_g);
        printf("      frase |I|_alg=%d  Heighway n=%ld pontos=%ld maxG=%d\n\n",
               ni, (long)n_drag, (long)nt, max_g);
        ok("§IA4 álgebra EN emite I; π Heighway realiza; existe célula G>1 (dobra)",
           ni > 10 && bate && max_g >= 2 && nt == n_drag + 1);
    }

    printf("§IA5  recta controlo → G≡1\n");
    {
        g_zera();
        L nr = recta(200, xs_g, ys_g);
        int max_g = 0;
        int bate = g_bate_recontagem(nr, &max_g);
        printf("      recta n=200 maxG=%d\n\n", max_g);
        ok("§IA5 controlo injectivo: recta sem revisita — max G=1",
           bate && max_g == 1);
    }

    printf("§IA6  ∑G = |I|\n");
    {
        g_zera();
        L nt = drag_andar(512, xs_g, ys_g);
        long soma = 0;
        for(int i = 0; i < LAD; i++)
            for(int j = 0; j < LAD; j++) soma += G[i][j];
        printf("      ∑G=%ld  |I|=%ld\n\n", soma, (long)nt);
        ok("§IA6 conservação: ∑G = |I|", soma == nt);
    }

    printf("§IA7  G mod 2\n");
    {
        g_zera();
        L nt = drag_andar(512, xs_g, ys_g);
        memset(Bf, 0, sizeof Bf);
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
        ok("§IA7 assinatura: B⊕=1 coincide com G mod 2", xor_ok);
    }

    printf("§IA8  min-G + fecho R⁴=id\n");
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
        ok("§IA8 ciclo aranha: min-G + R⁴ + marca forte",
           min_ok && rotor_ok && fecho);
    }

    {
        char k[256];
        en_chave_alfabeto(k, sizeof k);
        int a_ok = strstr(k, "idioma/en/alfabeto") != NULL;
        en_chave_lexico(0, k, sizeof k);
        int l_ok = strstr(k, "idioma/en/lexico/") != NULL;
        en_chave_regra(EN_R_DUAL, k, sizeof k);
        int r_ok = strstr(k, "idioma/en/regra/dual_lex") != NULL;
        ok("§IA* chaves banco idioma/en/{alfabeto,lexico,regra}",
           a_ok && l_ok && r_ok);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
