/* spline.h — A CARTA DA FONTE: ler a TTF e tirar o glifo como spline. Uma leitura, dois usos.
 *
 * Isto estava dentro do spline.c, que provou (95 de 95, nas duas variantes) que a curva concorda
 * com a tabela base-14. Provado isso, o tex.c deixa de precisar da tabela: a largura VEM DA CURVA.
 * Extraído para cabeçalho para que os dois leiam a mesma leitura — porque duas cópias da mesma
 * leitura são dois sítios onde a correção pode chegar só a um, e já perdi um dia com isso.
 *
 * O contorno TrueType é Bézier quadrática: B(t) = (1−t)²P₀ + 2t(1−t)P₁ + t²P₂, o polinômio de
 * grau 2 — e no projeto todo dado já é polinômio na base {1,σ,…}. Não há conversão: há leitura.
 */
#ifndef SPLINE_H
#define SPLINE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ───────────────────────────────────────────── a TTF, lida à mão e sem biblioteca nenhuma */

typedef struct { unsigned char *d; long n; } Buf;

static unsigned u8 (const Buf *b, long p){ return (p >= 0 && p < b->n) ? b->d[p] : 0; }
static unsigned u16(const Buf *b, long p){ return (u8(b,p) << 8) | u8(b,p+1); }
static int      s16(const Buf *b, long p){ int v = (int)u16(b,p); return v >= 32768 ? v - 65536 : v; }
static unsigned long u32(const Buf *b, long p){ return ((unsigned long)u16(b,p) << 16) | u16(b,p+2); }

typedef struct {
    Buf b;
    long head, hhea, hmtx, cmap, loca, glyf, maxp, cff;
    int  upem, longloca, nhmetrics, nglifos;
} Ttf;

static long tabela(const Buf *b, const char *tag){
    int nt = (int)u16(b, 4);
    for(int i = 0; i < nt; i++){
        long r = 12 + 16L*i;
        if(!memcmp(b->d + r, tag, 4)) return (long)u32(b, r + 8);
    }
    return 0;
}

static int ttf_abre(Ttf *t, const char *nome){
    FILE *f = fopen(nome, "rb");
    if(!f) return 0;
    fseek(f, 0, SEEK_END); t->b.n = ftell(f); fseek(f, 0, SEEK_SET);
    t->b.d = malloc((size_t)t->b.n);
    if(!t->b.d || fread(t->b.d, 1, (size_t)t->b.n, f) != (size_t)t->b.n){ fclose(f); return 0; }
    fclose(f);
    t->head = tabela(&t->b,"head"); t->hhea = tabela(&t->b,"hhea");
    t->hmtx = tabela(&t->b,"hmtx"); t->cmap = tabela(&t->b,"cmap");
    t->loca = tabela(&t->b,"loca"); t->glyf = tabela(&t->b,"glyf");
    t->maxp = tabela(&t->b,"maxp");
    t->cff = tabela(&t->b,"CFF ");
    /* ABRIR NÃO É LER A CURVA. A largura vem da `hmtx`, e a `hmtx` existe nos DOIS
     * formatos --- o que muda é onde moram os contornos: `glyf`/`loca` na TrueType, `CFF `
     * na OpenType. Exigir `glyf` para ABRIR recusava a Latin Modern inteira, que é a fonte
     * que o documento usa por omissão, e obrigava o compositor a cair na Helvetica: MEDIDO,
     * 1,83× a tinta do gabarito na mesma página.
     *
     * E os dois não são dois formatos: são A MESMA SPLINE EM GRAUS DIFERENTES. O
     * `tests/pascal.c` di-lo --- «C(n,k) = C(n-1,k-1) + C(n-1,k) é o passo da torre
     * A_{n+1} = A_n ⊕ A_n†, SÃO A MESMA COISA» --- e os coeficientes de Bézier são as
     * linhas de Pascal: 1,2,1 no grau 2 e 1,3,3,1 no grau 3. Uma linha acima, e mais nada. */
    if(!t->head || !t->hhea || !t->hmtx || !t->cmap) return 0;
    if(!t->glyf && !t->cff) return 0;              /* sem contornos em lado nenhum */
    t->upem      = (int)u16(&t->b, t->head + 18);
    t->longloca  = s16(&t->b, t->head + 50);
    t->nhmetrics = (int)u16(&t->b, t->hhea + 34);
    t->nglifos   = (int)u16(&t->b, t->maxp + 4);
    return t->upem > 0;
}

/* o cmap formato 4: do código Unicode ao índice de glifo */
static int ttf_glifo(const Ttf *t, int cp){
    int nt = (int)u16(&t->b, t->cmap + 2);
    long sub = 0;
    for(int i = 0; i < nt; i++){
        long r = t->cmap + 4 + 8L*i;
        int pid = (int)u16(&t->b, r), eid = (int)u16(&t->b, r + 2);
        if((pid == 3 && (eid == 1 || eid == 0)) || pid == 0) sub = t->cmap + (long)u32(&t->b, r + 4);
    }
    if(!sub || u16(&t->b, sub) != 4) return 0;
    int segX2 = (int)u16(&t->b, sub + 6), seg = segX2 / 2;
    long fim = sub + 14, ini = fim + segX2 + 2, del = ini + segX2, ran = del + segX2;
    for(int i = 0; i < seg; i++){
        int e = (int)u16(&t->b, fim + 2L*i);
        if(cp > e) continue;
        int s = (int)u16(&t->b, ini + 2L*i);
        if(cp < s) return 0;
        int ro = (int)u16(&t->b, ran + 2L*i);
        if(!ro) return (cp + s16(&t->b, del + 2L*i)) & 0xFFFF;
        long p = ran + 2L*i + ro + 2L*(cp - s);
        int g = (int)u16(&t->b, p);
        return g ? (g + s16(&t->b, del + 2L*i)) & 0xFFFF : 0;
    }
    return 0;
}

/* a largura de avanço, em unidades da fonte */
static int ttf_avanco(const Ttf *t, int g){
    if(g >= t->nhmetrics) g = t->nhmetrics - 1;
    return (int)u16(&t->b, t->hmtx + 4L*g);
}

/* ───────────────────────────────────────────── §P1  A CARTA: o contorno como splines */

typedef struct { double x, y; int onda; } Pt;      /* onda=1 -> ponto da curva; 0 -> de controlo */

#define MAXPT 4096
typedef struct { Pt p[MAXPT]; int n; int fim[64]; int nc; } Contorno;

static int ttf_contorno(const Ttf *t, int g, Contorno *c){
    long off, off2;
    if(t->longloca){ off = (long)u32(&t->b, t->loca + 4L*g); off2 = (long)u32(&t->b, t->loca + 4L*g + 4); }
    else           { off = 2L*u16(&t->b, t->loca + 2L*g);   off2 = 2L*u16(&t->b, t->loca + 2L*g + 2); }
    c->n = c->nc = 0;
    if(off >= off2) return 1;                       /* glifo vazio (o espaço) — e é legítimo */
    long p = t->glyf + off;
    int nc = s16(&t->b, p);
    if(nc < 0 || nc > 63) return 0;                 /* composto: fora do alcance desta medida */
    c->nc = nc;
    p += 10;
    int npt = 0;
    for(int i = 0; i < nc; i++){ c->fim[i] = (int)u16(&t->b, p + 2L*i); npt = c->fim[i] + 1; }
    p += 2L*nc;
    p += 2 + u16(&t->b, p);                          /* as instruções do hinting, saltadas */
    if(npt > MAXPT) return 0;
    unsigned char fl[MAXPT];
    for(int i = 0; i < npt; ){
        unsigned char f = u8(&t->b, p++);
        fl[i++] = f;
        if(f & 8){ int r = u8(&t->b, p++); while(r-- && i < npt) fl[i++] = f; }
    }
    double x = 0;
    for(int i = 0; i < npt; i++){
        if(fl[i] & 2){ int d = u8(&t->b, p++); x += (fl[i] & 16) ? d : -d; }
        else if(!(fl[i] & 16)){ x += s16(&t->b, p); p += 2; }
        c->p[i].x = x; c->p[i].onda = (fl[i] & 1) ? 1 : 0;
    }
    double y = 0;
    for(int i = 0; i < npt; i++){
        if(fl[i] & 4){ int d = u8(&t->b, p++); y += (fl[i] & 32) ? d : -d; }
        else if(!(fl[i] & 32)){ y += s16(&t->b, p); p += 2; }
        c->p[i].y = y;
    }
    c->n = npt;
    return 1;
}
/* onde a Liberation Sans costuma estar. É metricamente compatível com a Helvetica, que é a fonte
 * que o PDF nomeia — por isso medir por uma e desenhar com a outra é exato, e não uma aproximação:
 * o §P2 do spline.c mediu os 95 glifos das duas variantes e não houve uma divergência. */
/* A FONTE DO DOCUMENTO VEM PRIMEIRO. O `estilo.tex` não declara nenhuma, e por omissão o
 * LaTeX usa a Computer Modern --- é ela que está no gabarito (SFBX/SFTI/SFCC). A Latin
 * Modern é a sua versão em contorno, e vive em OpenType: os contornos em `CFF `, grau 3.
 *
 * Não é outro leitor: os coeficientes de Bézier são as linhas de Pascal, `1 2 1` no grau 2
 * e `1 3 3 1` no grau 3, e a recorrência que as gera é o passo da torre (tests/pascal.c).
 * O mesmo polinómio uma linha acima. A Liberation fica atrás, para quando a primeira não
 * estiver no sistema. */
static const char *SPLINE_REG[] = {
    "/usr/share/fonts/lm/lmroman10-regular.otf",
    "/usr/share/texmf-dist/fonts/opentype/public/lm/lmroman10-regular.otf",
    "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
};
static const char *SPLINE_NEG[] = {
    "/usr/share/fonts/lm/lmroman10-bold.otf",
    "/usr/share/texmf-dist/fonts/opentype/public/lm/lmroman10-bold.otf",
    "/usr/share/fonts/liberation-sans/LiberationSans-Bold.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Bold.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
};
#define SPLINE_NCAND 3

static int spline_abre_alguma(Ttf *t, const char **v, int n, const char **usada){
    for(int i = 0; i < n; i++) if(ttf_abre(t, v[i])){ if(usada) *usada = v[i]; return 1; }
    return 0;
}
#endif
