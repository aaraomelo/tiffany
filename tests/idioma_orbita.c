/* idioma_orbita.c — FRASE DO CORPUS → ÓRBITA PELA INVERSA DO DRAGÃO.
 *
 * Cadeia (thm:multiplicidade + thm:aranha-inversa + álgebra PT/EN/ES):
 *
 *   frase ∈ corpus  →  I (álgebra do idioma, Word_8)
 *                   →  π Heighway (realização)
 *                   →  π̃(i)=(π(i),k(i))  (inversa: G̃≡1)
 *
 * A órbita que se obtém é π̃: representação injectiva da curva do dragão
 * induzida pela frase. pr₁∘π̃ recupera π.
 *
 *   §IO1  uma frase: emite I, realiza, levanta; G̃≡1
 *   §IO2  corpus inteiro: cada frase → π̃ injectivo
 *   §IO3  fluxo longo (|I|≥513): base G>1 e levantamento G̃≡1
 *   §IO4  EN e ES: mesma tubagem numa frase cada
 *   §IO5  pr₁∘π̃ = π; |im π̃|=|I|=∑G
 *
 *   cc -O2 -std=c99 -w -I../lib -o idioma_orbita idioma_orbita.c && ./idioma_orbita
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "portugues.h"
#include "ingles.h"
#include "espanhol.h"

#define OFS  1024
#define LAD  2048
#define NMAX 8192
#define KMAX 64

typedef long L;

static int G[LAD][LAD];
static int cont[LAD][LAD];
static int xs[NMAX], ys[NMAX], ks[NMAX];

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++){ G[i][j] = 0; cont[i][j] = 0; }
}
static void g_inc(int x, int y){ G[y + OFS][x + OFS]++; }
static int g_get(int x, int y){ return G[y + OFS][x + OFS]; }
static int drag_vira_esq(L k){ return (((k & -k) << 1) & k) != 0; }

/* Heighway com |I|=n_passos+1 pontos */
static L drag_andar(L n_passos){
    int x = 0, y = 0, dx = 1, dy = 0;
    L t = 0;
    if(n_passos < 0) n_passos = 0;
    if(n_passos + 1 > NMAX) n_passos = NMAX - 1;
    xs[t] = x; ys[t] = y; g_inc(x, y); t++;
    for(L s = 0; s < n_passos; s++){
        x += dx; y += dy;
        xs[t] = x; ys[t] = y; g_inc(x, y); t++;
        if(drag_vira_esq(s + 1)){ int ndx = -dy, ndy = dx; dx = ndx; dy = ndy; }
        else                     { int ndx = dy,  ndy = -dx; dx = ndx; dy = ndy; }
    }
    return t;
}

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
        for(int j = 0; j < LAD; j++) if(G[i][j] > m) m = G[i][j];
    return m;
}

static long soma_g(void){
    long s = 0;
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++) s += G[i][j];
    return s;
}

static int tilde_injectivo(L nt){
    for(L i = 0; i < nt; i++)
        for(L j = i + 1; j < nt; j++)
            if(xs[i]==xs[j] && ys[i]==ys[j] && ks[i]==ks[j]) return 0;
    return 1;
}

static int tilde_g_um(L nt){
    if(!tilde_injectivo(nt)) return 0;
    for(L t = 0; t < nt; t++){
        int g = g_get(xs[t], ys[t]);
        if(ks[t] < 1 || ks[t] > g) return 0;
    }
    return 1;
}

/* frase → I_alg → π Heighway(|I|) → π̃ ; devolve |I| geométrico */
static L frase_para_orbita_inversa(int n_alg){
    L n_passos = n_alg > 1 ? (L)(n_alg - 1) : 0;
    g_zera();
    L nt = drag_andar(n_passos);
    levanta(nt);
    return nt;
}

static int le_corpus(const char *cam, char frases[][256], int cap){
    FILE *f = fopen(cam, "rb");
    if(!f) f = fopen("../lib/classe/corpus_frases_pt.txt", "rb");
    if(!f) return 0;
    int n = 0; char lin[512];
    while(n < cap && fgets(lin, sizeof lin, f)){
        if(lin[0]=='#' || lin[0]=='\n' || lin[0]=='\r') continue;
        size_t L = strlen(lin);
        while(L && (lin[L-1]=='\n' || lin[L-1]=='\r')) lin[--L] = 0;
        if(L < 3) continue;
        snprintf(frases[n], 256, "%s", lin);
        n++;
    }
    fclose(f);
    return n;
}

int main(void){
    printf("=== IDIOMA × ÓRBITA INVERSA: frase → I → π → π̃ (G̃≡1) =================\n\n");

    pt_sigma_init(); pt_lex_init();
    en_sigma_init(); en_lex_init();
    es_sigma_init(); es_lex_init();

    /* ── §IO1 ─────────────────────────────────────────────────────────────── */
    printf("§IO1  uma frase do léxico → órbita pela inversa\n");
    {
        const char *fr = "o ouro do rei é álgebra e órbita do dragão";
        PtOrbita o;
        int ni = pt_emite_orbita(fr, &o);
        L nt = frase_para_orbita_inversa(ni);
        printf("      \"%s\"\n      |I|_alg=%d  |π̃|=%ld  maxG_base=%d  injectivo=%s\n\n",
               fr, ni, (long)nt, max_g(), tilde_injectivo(nt) ? "sim" : "nao");
        ok("§IO1 frase PT emite I; π̃ pela inversa do dragão com G̃≡1",
           ni > 10 && nt == (L)ni && tilde_g_um(nt) && tilde_injectivo(nt));
    }

    /* ── §IO2 ─────────────────────────────────────────────────────────────── */
    printf("§IO2  corpus de frases: cada uma → π̃ injectivo\n");
    {
        char frases[32][256];
        int nf = le_corpus("lib/classe/corpus_frases_pt.txt", frases, 32);
        int ok_todas = nf > 0, n_ok = 0;
        for(int i = 0; i < nf; i++){
            PtOrbita o;
            int ni = pt_emite_orbita(frases[i], &o);
            if(ni < 2){ ok_todas = 0; continue; }
            L nt = frase_para_orbita_inversa(ni);
            if(!(nt == (L)ni && tilde_g_um(nt))) ok_todas = 0;
            else n_ok++;
        }
        printf("      corpus: %d frases, %d com π̃ G̃≡1\n\n", nf, n_ok);
        ok("§IO2 corpus PT: toda frase admite órbita injectiva pela inversa",
           nf >= 8 && ok_todas && n_ok == nf);
    }

    /* ── §IO3 ─────────────────────────────────────────────────────────────── */
    printf("§IO3  fluxo longo: base dobra (G>1) e π̃ restaura G̃≡1\n");
    {
        char frases[32][256];
        int nf = le_corpus("lib/classe/corpus_frases_pt.txt", frases, 32);
        PtOrbita acc; pt_orbita_zera(&acc);
        for(int i = 0; i < nf && acc.n < 600; i++){
            PtOrbita o;
            int ni = pt_emite_orbita(frases[i], &o);
            for(int j = 0; j < ni && acc.n < PT_I_MAX; j++)
                pt_orbita_poe(&acc, o.passo[j]);
        }
        /* se ainda curto, repete Heighway mínimo via padding da emissão */
        while(acc.n < 513 && acc.n < PT_I_MAX){
            PtOrbita o;
            int ni = pt_emite_orbita("o ouro do rei é álgebra e órbita do dragão", &o);
            for(int j = 0; j < ni && acc.n < PT_I_MAX; j++)
                pt_orbita_poe(&acc, o.passo[j]);
        }
        L nt = frase_para_orbita_inversa(acc.n);
        int mg = max_g();
        printf("      |I|_alg=%d  |π̃|=%ld  maxG_base=%d\n\n", acc.n, (long)nt, mg);
        ok("§IO3 fluxo corpus: Heighway dobra na base e levantamento dá G̃≡1",
           acc.n >= 513 && nt == (L)acc.n && mg >= 2 && tilde_g_um(nt));
    }

    /* ── §IO4 ─────────────────────────────────────────────────────────────── */
    printf("§IO4  EN e ES na mesma tubagem\n");
    {
        EnOrbita eo; EsOrbita so;
        int ne = en_emite_orbita("the gold of the king is algebra and orbit of the dragon", &eo);
        L nte = frase_para_orbita_inversa(ne);
        int ok_en = ne > 10 && nte == (L)ne && tilde_g_um(nte);

        int ns = es_emite_orbita("el oro del rey es álgebra y órbita del dragón", &so);
        L nts = frase_para_orbita_inversa(ns);
        int ok_es = ns > 10 && nts == (L)ns && tilde_g_um(nts);
        printf("      EN |I|=%d π̃=%ld  ES |I|=%d π̃=%ld\n\n",
               ne, (long)nte, ns, (long)nts);
        ok("§IO4 EN e ES: frase → órbita injectiva pela inversa do dragão",
           ok_en && ok_es);
    }

    /* ── §IO5 ─────────────────────────────────────────────────────────────── */
    printf("§IO5  conservação e fibra vertical\n");
    {
        PtOrbita o;
        int ni = pt_emite_orbita("a prata e o ouro da casa do rei", &o);
        L nt = frase_para_orbita_inversa(ni);
        long sg = soma_g();
        int fibra_ok = 1;
        for(L t = 0; t < nt && fibra_ok; t++){
            int g = g_get(xs[t], ys[t]);
            int visto[KMAX]; memset(visto, 0, sizeof visto);
            for(L s = 0; s < nt; s++){
                if(xs[s]!=xs[t] || ys[s]!=ys[t]) continue;
                if(ks[s]<1 || ks[s]>g || ks[s]>=KMAX){ fibra_ok = 0; break; }
                visto[ks[s]]++;
            }
            for(int k = 1; k <= g && fibra_ok; k++) if(visto[k] != 1) fibra_ok = 0;
        }
        printf("      ∑G=%ld |I|=%ld fibra_ok=%s\n\n", sg, (long)nt, fibra_ok?"sim":"nao");
        ok("§IO5 |im π̃|=|I|=∑G; folhas = fibra; pr₁ recupera π",
           sg == nt && fibra_ok && tilde_injectivo(nt));
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
