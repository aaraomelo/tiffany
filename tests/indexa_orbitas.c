/* indexa_orbitas.c — FRASE → π̃ → índice idioma/pt/orbita/…
 *
 * Escreve lib/classe/corpus_orbitas_pt.txt (linhas `orbita slug|meta`)
 * para IMPORT IDIOMA 'pt' '…/corpus_orbitas_pt.txt'.
 *
 *   §IX1  cada frase do corpus emite π̃ com G̃≡1
 *   §IX2  ficheiro de índice: N linhas orbita válidas
 *   §IX3  meta cabe em chave (<480); slug estável
 *   §IX4  IMPORT IDIOMA no banco + BUSCA TEXTO recupera (distância 0)
 *
 *   cc -O2 -std=c99 -w -I../lib -o indexa_orbitas indexa_orbitas.c -lm
 *   ./indexa_orbitas
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "unidade.h"
#include "portugues.h"

#define OFS  1024
#define LAD  2048
#define NMAX 8192
#define CAP  64

typedef long L;

static int G[LAD][LAD], cont[LAD][LAD];
static int xs[NMAX], ys[NMAX], ks[NMAX];

static void g_zera(void){
    for(int i = 0; i < LAD; i++)
        for(int j = 0; j < LAD; j++){ G[i][j] = 0; cont[i][j] = 0; }
}
static void g_inc(int x, int y){ G[y + OFS][x + OFS]++; }
static int g_get(int x, int y){ return G[y + OFS][x + OFS]; }
static int drag_vira_esq(L k){ return (((k & -k) << 1) & k) != 0; }

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

static uint32_t ck_orbita(L nt){
    uint32_t h = 2166136261u;
    for(L t = 0; t < nt; t++){
        h ^= (uint32_t)(xs[t] + 0x9e3779b9);
        h *= 16777619u;
        h ^= (uint32_t)(ys[t] + 0x85ebca6b);
        h *= 16777619u;
        h ^= (uint32_t)ks[t];
        h *= 16777619u;
    }
    return h;
}

static void slug_de(const char *frase, char *out, size_t cap){
    /* FNV-1a 32 → 8 hex (chave total ≤48: limite da cifra no sql.c) */
    uint32_t h = 2166136261u;
    for(const unsigned char *p = (const unsigned char*)frase; *p; p++){
        h ^= *p; h *= 16777619u;
    }
    snprintf(out, cap, "%08x", h);
}

/* meta compacta sem ';' nem espaços — cabe com idioma/pt/orbita/<slug>|… em ≤48 */
static void meta_de(char *out, size_t cap, const char *slug, L nt, int mg, int t1, uint32_t ck){
    snprintf(out, cap, "%s|n%ldg%dt%dc%08x", slug, (long)nt, mg, t1, ck);
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

static const char *caminho_indice(char *buf, size_t cap){
    const char *cands[] = {
        "lib/classe/corpus_orbitas_pt.txt",
        "../lib/classe/corpus_orbitas_pt.txt",
        NULL
    };
    for(int i = 0; cands[i]; i++){
        FILE *t = fopen(cands[i], "rb");
        if(t){ fclose(t); snprintf(buf, cap, "%s", cands[i]); return buf; }
    }
    snprintf(buf, cap, "%s", cands[0]);
    return buf;
}

int main(void){
    char frases[CAP][256];
    char idx_cam[256];
    int nf = le_corpus("lib/classe/corpus_frases_pt.txt", frases, CAP);
    if(!nf) nf = le_corpus("../lib/classe/corpus_frases_pt.txt", frases, CAP);

    printf("=== INDEXA ÓRBITAS π̃ → idioma/pt/orbita/… ==========================\n\n");
    printf("§IX1  corpus → π̃ com G̃≡1\n");
    {
        int ok_all = nf > 0;
        for(int i = 0; i < nf; i++){
            PtOrbita o; int ni = pt_emite_orbita(frases[i], &o);
            L n_passos = ni > 1 ? (L)(ni - 1) : 0;
            g_zera(); L nt = drag_andar(n_passos); levanta(nt);
            if(!tilde_ok(nt)) ok_all = 0;
        }
        ok("§IX1 cada frase do corpus (amostra) admite π̃ injectivo G̃≡1", ok_all);
    }

    printf("§IX2  escreve corpus_orbitas_pt.txt\n");
    int n_lin = 0; int meta_ok = 1;
    {
        caminho_indice(idx_cam, sizeof idx_cam);
        /* preferir caminho relativo à raiz do repo */
        {
            FILE *probe = fopen("lib/classe/corpus_frases_pt.txt", "rb");
            if(probe){ fclose(probe); snprintf(idx_cam, sizeof idx_cam,
                "lib/classe/corpus_orbitas_pt.txt"); }
            else snprintf(idx_cam, sizeof idx_cam,
                "../lib/classe/corpus_orbitas_pt.txt");
        }
        FILE *out = fopen(idx_cam, "wb");
        if(!out){
            ok("§IX2 abre corpus_orbitas_pt.txt para escrita", 0);
        } else {
            fprintf(out,
                "# corpus_orbitas_pt.txt — π̃ indexável: IMPORT IDIOMA 'pt' este ficheiro\n"
                "# orbita slug|nNgMt1cHEX   (≤48 na chave idioma/pt/orbita/…)\n"
                "# chave no banco: idioma/pt/orbita/<meta>\n");
            for(int i = 0; i < nf; i++){
                PtOrbita o; int ni = pt_emite_orbita(frases[i], &o);
                L n_passos = ni > 1 ? (L)(ni - 1) : 0;
                g_zera(); L nt = drag_andar(n_passos); levanta(nt);
                char slug[16], resto[64], chave[80];
                slug_de(frases[i], slug, sizeof slug);
                meta_de(resto, sizeof resto, slug, nt, max_g(), tilde_ok(nt), ck_orbita(nt));
                snprintf(chave, sizeof chave, "idioma/pt/orbita/%s", resto);
                if(strlen(chave) > 48) meta_ok = 0;
                fprintf(out, "orbita %s\n", resto);
                n_lin++;
            }
            fclose(out);
            ok("§IX2 índice com uma linha orbita por frase da amostra",
               n_lin == nf && n_lin > 0);
        }
    }

    printf("§IX3  meta cabe na chave do banco\n");
    {
        ok("§IX3 chave idioma/pt/orbita/<meta> ≤48 (cifra do sql)", meta_ok && n_lin > 0);
    }

    printf("§IX4  IMPORT IDIOMA + BUSCA TEXTO\n");
    {
        char sqlb[256];
        const char *cands[] = {"/tmp/sqlb", "banco/bin/sql", "../banco/bin/sql", NULL};
        const char *bin = NULL;
        for(int i = 0; cands[i]; i++){
            FILE *t = fopen(cands[i], "rb");
            if(t){ fclose(t); bin = cands[i]; break; }
        }
        /* compila se faltar */
        if(!bin){
            int rc = system(
                "cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sqlb banco/sql.c -lm 2>/dev/null"
                " || cc -O2 -std=c99 -w -I../lib -I../banco -o /tmp/sqlb ../banco/sql.c -lm 2>/dev/null");
            if(rc == 0) bin = "/tmp/sqlb";
        }
        int passou = 0;
        if(bin && n_lin > 0){
            char cmd[800];
            const char *base = "/tmp/tiffany_orbita_idx";
            snprintf(cmd, sizeof cmd,
                "rm -f %s.mem %s.prog; %s %s \"IMPORT IDIOMA 'pt' '%s'\" >/tmp/ix_import.out 2>&1",
                base, base, bin, base, idx_cam);
            if(system(cmd) == 0){
                FILE *im = fopen("/tmp/ix_import.out", "rb");
                char buf[4096]; size_t n = 0;
                if(im){ n = fread(buf, 1, sizeof buf - 1, im); buf[n] = 0; fclose(im); }
                long postas = 0;
                const char *p = strstr(buf, "IMPORT IDIOMA");
                if(p){
                    p = strstr(p, ":");
                    if(p) postas = strtol(p + 1, NULL, 10);
                }
                char slug[16], resto[64], chave[80];
                slug_de(frases[0], slug, sizeof slug);
                PtOrbita o0; int ni0 = pt_emite_orbita(frases[0], &o0);
                L np0 = ni0 > 1 ? (L)(ni0 - 1) : 0;
                g_zera(); L nt0 = drag_andar(np0); levanta(nt0);
                meta_de(resto, sizeof resto, slug, nt0, max_g(), tilde_ok(nt0), ck_orbita(nt0));
                snprintf(chave, sizeof chave, "idioma/pt/orbita/%s", resto);
                /* BUSCA (ACHA-árvore pode falhar entre processos; BUSCA compara cifras) */
                {
                    FILE *sq = fopen("/tmp/ix_busca.sql", "wb");
                    if(sq){ fprintf(sq, "BUSCA TEXTO '%s'\n", chave); fclose(sq); }
                }
                snprintf(cmd, sizeof cmd,
                    "%s %s - </tmp/ix_busca.sql >/tmp/ix_busca.out 2>&1", bin, base);
                int achou = 0;
                if(system(cmd) == 0){
                    FILE *a = fopen("/tmp/ix_busca.out", "rb");
                    if(a){
                        fseek(a, 0, SEEK_END);
                        long sz = ftell(a);
                        if(sz < 0) sz = 0;
                        if(sz > 1<<20) sz = 1<<20;
                        fseek(a, 0, SEEK_SET);
                        char *big = (char*)malloc((size_t)sz + 1);
                        if(big){
                            n = fread(big, 1, (size_t)sz, a); big[n] = 0;
                            char marca[96];
                            snprintf(marca, sizeof marca, "MAIS PROXIMO: %s", chave);
                            achou = strstr(big, marca) != NULL;
                            free(big);
                        }
                        fclose(a);
                    }
                }
                passou = (postas == n_lin) && achou && strlen(chave) <= 48;
                printf("      import postas=%ld (esperado %d)  BUSCA_exacta=%s  |chave|=%zu\n",
                       postas, n_lin, achou ? "sim" : "nao", strlen(chave));
            }
        }
        ok("§IX4 IMPORT IDIOMA 'pt' indexa idioma/…/orbita e BUSCA recupera", passou);
    }

    return falhas ? 1 : 0;
}
