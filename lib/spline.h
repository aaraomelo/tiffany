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

#ifdef TEX_COM_LIBC_WASM
/* o host já pôs a carta no slot — aponta pelo nome, sem agulha (corpo-estelar §estrela) */
char *ficheiro_end_nome(char *nome);
int ficheiro_tam_nome(char *nome);
static int ttf_abre(Ttf *t, const char *nome){
    char *ja = ficheiro_end_nome((char*)nome);
    int tn = ficheiro_tam_nome((char*)nome);
    if(!ja || tn <= 0) return 0;
    t->b.d = (unsigned char*)ja; t->b.n = tn;
#else
static int ttf_abre(Ttf *t, const char *nome){
    FILE *f = fopen(nome, "rb");
    if(!f) return 0;
    fseek(f, 0, SEEK_END); t->b.n = ftell(f); fseek(f, 0, SEEK_SET);
    t->b.d = malloc((size_t)t->b.n);
    if(!t->b.d || fread(t->b.d, 1, (size_t)t->b.n, f) != (size_t)t->b.n){ fclose(f); return 0; }
    fclose(f);
#endif
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
static unsigned char TTF_FL[MAXPT];   /* flags da glyf — fora do quadro (o traduz empilha `char[MAXPT]`) */

/* o ponto do contorno lido na régua INTEIRA da fonte: o `255` da CFF traz fracção
 * 16.16 e o `div` produz razões — truncar amputava. UMA divisão arredondada, a do
 * LER, e ela mora aqui, no dono da representação: quem consome fala só inteiros. */
static long contorno_xi(const Contorno *c, int i){
    double v = c->p[i].x; return (long)(v >= 0 ? v + 0.5 : v - 0.5); }
static long contorno_yi(const Contorno *c, int i){
    double v = c->p[i].y; return (long)(v >= 0 ? v + 0.5 : v - 0.5); }

static int ttf_contorno(const Ttf *t, int g, Contorno *c){
    long off, off2;
    if(t->longloca){ off = (long)u32(&t->b, t->loca + 4L*g); off2 = (long)u32(&t->b, t->loca + 4L*g + 4); }
    else           { off = 2L*u16(&t->b, t->loca + 2L*g);   off2 = 2L*u16(&t->b, t->loca + 2L*g + 2); }
    c->n = 0; c->nc = 0;   /* duas MOVEs no par (A,B), não uma cadeia com valor na pilha */
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
    for(int i = 0; i < npt; ){
        unsigned char f = u8(&t->b, p++);
        TTF_FL[i++] = f;
        if(f & 8){ int r = u8(&t->b, p++); while(r-- && i < npt) TTF_FL[i++] = f; }
    }
    double x = 0;
    for(int i = 0; i < npt; i++){
        if(TTF_FL[i] & 2){ int d = u8(&t->b, p++); x += (TTF_FL[i] & 16) ? d : -d; }
        else if(!(TTF_FL[i] & 16)){ x += s16(&t->b, p); p += 2; }
        c->p[i].x = x; c->p[i].onda = (TTF_FL[i] & 1) ? 1 : 0;
    }
    double y = 0;
    for(int i = 0; i < npt; i++){
        if(TTF_FL[i] & 4){ int d = u8(&t->b, p++); y += (TTF_FL[i] & 32) ? d : -d; }
        else if(!(TTF_FL[i] & 32)){ y += s16(&t->b, p); p += 2; }
        c->p[i].y = y;
    }
    c->n = npt;
    return 1;
}
/* ───────────────────────────────────── §P1b  A CARTA CFF: o contorno grau TRÊS
 *
 * A OpenType do documento (a Latin Modern) guarda os contornos na `CFF `, em charstrings
 * Type2 --- a MESMA spline uma linha abaixo em Pascal: 1 3 3 1, cúbicas. Lê-se à mão,
 * como tudo aqui: os INDEX, o Top DICT (CharStrings em 17, Private em 18, Subrs em 19),
 * e a máquina de pilha dos operadores. No Contorno, o troço cúbico fica como
 * on, ctrl, ctrl, on --- DOIS pontos de controlo seguidos, que é como o leitor do
 * caminho distingue o grau. */

static long cff_index_fim(const Buf *b, long p){       /* devolve o fim de um INDEX */
    int n = (int)u16(b, p);
    if(n == 0) return p + 2;
    int osz = (int)u8(b, p + 2);
    long offs = p + 3;
    long dados = offs + (long)(n + 1) * osz - 1;
    long ult = 0;
    for(int k = 0; k < osz; k++) ult = (ult << 8) | u8(b, offs + (long)n * osz + k);
    return dados + ult;
}
static long cff_index_item(const Buf *b, long p, int i, long *fim){  /* o item i */
    int n = (int)u16(b, p);
    if(i < 0 || i >= n) return 0;
    int osz = (int)u8(b, p + 2);
    long offs = p + 3;
    long dados = offs + (long)(n + 1) * osz - 1;
    long a = 0, z = 0;
    for(int k = 0; k < osz; k++) a = (a << 8) | u8(b, offs + (long)i * osz + k);
    for(int k = 0; k < osz; k++) z = (z << 8) | u8(b, offs + (long)(i + 1) * osz + k);
    *fim = dados + z;
    return dados + a;
}
static int cff_index_n(const Buf *b, long p){ return (int)u16(b, p); }

/* o DICT: varre operandos até ao operador pedido; devolve quantos e os últimos dois */
static int cff_dict(const Buf *b, long a, long z, int op, long *v1, long *v2){
    long st[8]; int ns = 0;
    for(long p = a; p < z; ){
        unsigned c = u8(b, p);
        if(c >= 32 && c <= 246){ if(ns < 8) st[ns++] = (long)c - 139; p++; }
        else if(c >= 247 && c <= 250){ if(ns < 8) st[ns++] = ((long)c - 247) * 256 + u8(b, p+1) + 108; p += 2; }
        else if(c >= 251 && c <= 254){ if(ns < 8) st[ns++] = -((long)c - 251) * 256 - u8(b, p+1) - 108; p += 2; }
        else if(c == 28){ if(ns < 8) st[ns++] = s16(b, p+1); p += 3; }
        else if(c == 29){ if(ns < 8) st[ns++] = (long)((u32(b, p+1) ^ 0x80000000UL) - 0x80000000UL); p += 5; }
        else if(c == 30){ p++; while(p < z){ unsigned nb = u8(b, p++); if((nb & 15) == 15 || (nb >> 4) == 15) break; } }
        else { /* operador */
            int o = (int)c; p++;
            if(c == 12){ o = 1200 + (int)u8(b, p); p++; }
            if(o == op){
                if(ns >= 1) *v1 = st[ns >= 2 ? ns - 2 : 0];
                if(ns >= 2) *v2 = st[ns - 1]; else *v2 = st[0];
                return ns;
            }
            ns = 0;
        }
    }
    return 0;
}

static int cff_bias(int n){ return n < 1240 ? 107 : (n < 33900 ? 1131 : 32768); }

typedef struct { const Buf *b; Contorno *c; double x, y; int aberto;
                 long cs, gs, ls; } CffMaq;   /* charstrings, gsubrs, lsubrs (INDEX) */

static void cff_ponto(Contorno *c, double x, double y, int onda){
    if(c->n < MAXPT){ c->p[c->n].x = x; c->p[c->n].y = y; c->p[c->n].onda = onda; c->n++; }
}
static void cff_fecha(CffMaq *m){
    if(m->aberto && m->c->nc < 64){ m->c->fim[m->c->nc++] = m->c->n - 1; }
    m->aberto = 0;
}
static void cff_curva(CffMaq *m, double x1, double y1, double x2, double y2, double x3, double y3){
    cff_ponto(m->c, x1, y1, 0); cff_ponto(m->c, x2, y2, 0); cff_ponto(m->c, x3, y3, 1);
    m->x = x3; m->y = y3;
}
/* operadores Type2: cada um é uma frase. O despacho em cff_corre é if/else —
 * catalogo.tex «o switch é uma cadeia»; tools/traduz.c desce_corpo (wasm-c) reconstitui
 * if/else, nunca switch; a queda de case recusa-se (tests/traduz_volta.js §V5g). */
static void cff_rlineto(CffMaq *m, double *st, int *ns){
    int k = 0;
    while(k + 1 < *ns){ m->x += st[k]; m->y += st[k+1]; cff_ponto(m->c, m->x, m->y, 1); k = k + 2; }
    *ns = 0;
}
static void cff_hvlineto(CffMaq *m, double *st, int *ns, int h){
    int k = 0;
    while(k < *ns){ if(h) m->x += st[k]; else m->y += st[k]; cff_ponto(m->c, m->x, m->y, 1); h = !h; k = k + 1; }
    *ns = 0;
}
static void cff_rrc(CffMaq *m, double *st, int *ns){
    int k = 0;
    while(k + 5 < *ns){
        cff_curva(m, m->x + st[k], m->y + st[k+1],
                     m->x + st[k] + st[k+2], m->y + st[k+1] + st[k+3],
                     m->x + st[k] + st[k+2] + st[k+4], m->y + st[k+1] + st[k+3] + st[k+5]);
        k = k + 6;
    }
    *ns = 0;
}
static void cff_vv(CffMaq *m, double *st, int *ns){
    int k = 0; double d1 = 0;
    if(*ns & 1){ d1 = st[0]; k = 1; }
    while(k + 3 < *ns){
        double x1 = m->x + d1, y1 = m->y + st[k];
        double x2 = x1 + st[k+1], y2 = y1 + st[k+2];
        cff_curva(m, x1, y1, x2, y2, x2, y2 + st[k+3]);
        d1 = 0; k = k + 4;
    }
    *ns = 0;
}
static void cff_hh(CffMaq *m, double *st, int *ns){
    int k = 0; double d1 = 0;
    if(*ns & 1){ d1 = st[0]; k = 1; }
    while(k + 3 < *ns){
        double x1 = m->x + st[k], y1 = m->y + d1;
        double x2 = x1 + st[k+1], y2 = y1 + st[k+2];
        cff_curva(m, x1, y1, x2, y2, x2 + st[k+3], y2);
        d1 = 0; k = k + 4;
    }
    *ns = 0;
}
static void cff_vhhv(CffMaq *m, double *st, int *ns, int h){
    int k = 0;
    while(k + 3 < *ns){
        int resto = *ns - k;
        double x1, y1, x2, y2, x3, y3;
        if(h){ x1 = m->x + st[k]; y1 = m->y; }
        else { x1 = m->x; y1 = m->y + st[k]; }
        x2 = x1 + st[k+1]; y2 = y1 + st[k+2];
        if(h){ y3 = y2 + st[k+3]; x3 = (resto == 5) ? x2 + st[k+4] : x2; }
        else { x3 = x2 + st[k+3]; y3 = (resto == 5) ? y2 + st[k+4] : y2; }
        cff_curva(m, x1, y1, x2, y2, x3, y3);
        k += (resto == 5) ? 5 : 4; h = !h;
    }
    *ns = 0;
}
static void cff_rcl(CffMaq *m, double *st, int *ns){
    int k = 0;
    while(k + 5 < *ns - 2){
        cff_curva(m, m->x + st[k], m->y + st[k+1],
                     m->x + st[k] + st[k+2], m->y + st[k+1] + st[k+3],
                     m->x + st[k] + st[k+2] + st[k+4], m->y + st[k+1] + st[k+3] + st[k+5]);
        k = k + 6;
    }
    if(k + 1 < *ns){ m->x += st[k]; m->y += st[k+1]; cff_ponto(m->c, m->x, m->y, 1); }
    *ns = 0;
}
static void cff_rlc(CffMaq *m, double *st, int *ns){
    int k = 0;
    while(k + 1 < *ns - 6){ m->x += st[k]; m->y += st[k+1]; cff_ponto(m->c, m->x, m->y, 1); k = k + 2; }
    if(k + 5 < *ns)
        cff_curva(m, m->x + st[k], m->y + st[k+1],
                     m->x + st[k] + st[k+2], m->y + st[k+1] + st[k+3],
                     m->x + st[k] + st[k+2] + st[k+4], m->y + st[k+1] + st[k+3] + st[k+5]);
    *ns = 0;
}
static void cff_esc(CffMaq *m, double *st, int *ns, unsigned o2){
    double a1[16]; int n2 = *ns; int k;
    for(k = 0; k < n2 && k < 16; k++) a1[k] = st[k];
    if(o2 == 35 && n2 >= 13){
        cff_curva(m, m->x+a1[0], m->y+a1[1], m->x+a1[0]+a1[2], m->y+a1[1]+a1[3],
                     m->x+a1[0]+a1[2]+a1[4], m->y+a1[1]+a1[3]+a1[5]);
        cff_curva(m, m->x+a1[6], m->y+a1[7], m->x+a1[6]+a1[8], m->y+a1[7]+a1[9],
                     m->x+a1[6]+a1[8]+a1[10], m->y+a1[7]+a1[9]+a1[11]);
    } else if(o2 == 34 && n2 >= 7){
        double y0 = m->y;
        cff_curva(m, m->x+a1[0], m->y, m->x+a1[0]+a1[1], m->y+a1[2],
                     m->x+a1[0]+a1[1]+a1[3], m->y+a1[2]);
        cff_curva(m, m->x+a1[4], m->y, m->x+a1[4]+a1[5], y0,
                     m->x+a1[4]+a1[5]+a1[6], y0);
    } else if(o2 == 36 && n2 >= 9){
        double y0 = m->y;
        cff_curva(m, m->x+a1[0], m->y+a1[1], m->x+a1[0]+a1[2], m->y+a1[1]+a1[3],
                     m->x+a1[0]+a1[2]+a1[4], m->y+a1[1]+a1[3]);
        cff_curva(m, m->x+a1[5], m->y, m->x+a1[5]+a1[6], m->y+a1[7],
                     m->x+a1[5]+a1[6]+a1[8], y0);
    } else if(o2 == 37 && n2 >= 11){
        double x0 = m->x, y0 = m->y;
        double dx = a1[0]+a1[2]+a1[4]+a1[6]+a1[8], dy = a1[1]+a1[3]+a1[5]+a1[7]+a1[9];
        cff_curva(m, m->x+a1[0], m->y+a1[1], m->x+a1[0]+a1[2], m->y+a1[1]+a1[3],
                     m->x+a1[0]+a1[2]+a1[4], m->y+a1[1]+a1[3]+a1[5]);
        double x2 = m->x+a1[6]+a1[8], y2 = m->y+a1[7]+a1[9];
        double adx = dx < 0 ? -dx : dx, ady = dy < 0 ? -dy : dy;
        double x3 = adx > ady ? x2 + a1[10] : x2;
        double y3 = adx > ady ? y0 : y2 + a1[10];
        (void)x0;
        cff_curva(m, m->x+a1[6], m->y+a1[7], x2, y2, x3, y3);
    }
    *ns = 0;
}

/* Type2: a cadeia if/else que o wasm-c já é (catalogo.tex; papers/arquitetura.tex
 * thm:traduz — br_table não tem forma em C; desce_corpo reconstitui if). Os quatro
 * switch do tex_core acabam em return; aqui o laço continua, logo não é switch. */
static int cff_corre(CffMaq *m, long a, long z, double *st, int *ns, int *nstem, int prof){
    if(prof > 10) return 0;
    const Buf *b = m->b;
    for(long p = a; p < z; ){
        unsigned c = u8(b, p);
        if(c >= 32){
            double v;
            if(c <= 246){ v = (double)((int)c - 139); p++; }
            else if(c <= 250){ v = (double)(((int)c - 247) * 256 + (int)u8(b, p+1) + 108); p += 2; }
            else if(c <= 254){ v = (double)(-((int)c - 251) * 256 - (int)u8(b, p+1) - 108); p += 2; }
            else { v = s16(b, p+1) + u16(b, p+3) / 65536.0; p += 5; }   /* 255: fixo 16.16 */
            if(*ns < 48){ st[*ns] = v; *ns = *ns + 1; }
            continue;
        }
        if(c == 28){ if(*ns < 48){ st[*ns] = (double)s16(b, p+1); *ns = *ns + 1; } p += 3; continue; }
        p++;
        if(c == 1 || c == 3 || c == 18 || c == 23){ *nstem += *ns / 2; *ns = 0; }
        else if(c == 19 || c == 20){ *nstem += *ns / 2; *ns = 0; p += (*nstem + 7) / 8; }
        else if(c == 21){
            if(*ns > 2){ st[0] = st[*ns-2]; st[1] = st[*ns-1]; }
            cff_fecha(m); m->x += st[0]; m->y += st[1];
            cff_ponto(m->c, m->x, m->y, 1); m->aberto = 1; *ns = 0;
        }
        else if(c == 22){
            if(*ns > 1) st[0] = st[*ns-1];
            cff_fecha(m); m->x += st[0];
            cff_ponto(m->c, m->x, m->y, 1); m->aberto = 1; *ns = 0;
        }
        else if(c == 4){
            if(*ns > 1) st[0] = st[*ns-1];
            cff_fecha(m); m->y += st[0];
            cff_ponto(m->c, m->x, m->y, 1); m->aberto = 1; *ns = 0;
        }
        else if(c == 5) cff_rlineto(m, st, ns);
        else if(c == 6) cff_hvlineto(m, st, ns, 1);
        else if(c == 7) cff_hvlineto(m, st, ns, 0);
        else if(c == 8) cff_rrc(m, st, ns);
        else if(c == 24) cff_rcl(m, st, ns);
        else if(c == 25) cff_rlc(m, st, ns);
        else if(c == 26) cff_vv(m, st, ns);
        else if(c == 27) cff_hh(m, st, ns);
        else if(c == 30) cff_vhhv(m, st, ns, 0);
        else if(c == 31) cff_vhhv(m, st, ns, 1);
        else if(c == 10){
            if(m->ls && *ns >= 1){
                *ns = *ns - 1;
                { int si = (int)st[*ns] + cff_bias(cff_index_n(b, m->ls));
                  long na = 0, nz = 0;
                  na = cff_index_item(b, m->ls, si, &nz);
                  if(na && !cff_corre(m, na, nz, st, ns, nstem, prof + 1)) return 0; }
            } else *ns = 0;
        }
        else if(c == 29){
            if(m->gs && *ns >= 1){
                *ns = *ns - 1;
                { int si = (int)st[*ns] + cff_bias(cff_index_n(b, m->gs));
                  long na = 0, nz = 0;
                  na = cff_index_item(b, m->gs, si, &nz);
                  if(na && !cff_corre(m, na, nz, st, ns, nstem, prof + 1)) return 0; }
            } else *ns = 0;
        }
        else if(c == 11) return 1;
        else if(c == 14){ cff_fecha(m); return 1; }
        else if(c == 12){ unsigned o2 = u8(b, p); p++; cff_esc(m, st, ns, o2); }
        else *ns = 0;
    }
    return 1;
}

/* o contorno de um glifo CFF: a assinatura cúbica, extraída da própria fonte */
static int cff_contorno(const Ttf *t, int g, Contorno *c){
    if(!t->cff) return 0;
    const Buf *b = &t->b;
    long cff = t->cff;
    long p = cff + u8(b, cff + 2);                   /* hdrSize -> Name INDEX */
    p = cff_index_fim(b, p);                          /* -> Top DICT INDEX */
    long td_a, td_z;
    td_a = cff_index_item(b, p, 0, &td_z);
    if(!td_a) return 0;
    long strs = cff_index_fim(b, p);                  /* -> String INDEX */
    long gsub = cff_index_fim(b, strs);               /* -> Global Subr INDEX */
    long v1 = 0, v2 = 0;
    if(!cff_dict(b, td_a, td_z, 17, &v1, &v2)) return 0;
    long cs = cff + v2;                               /* CharStrings INDEX */
    long ls = 0;
    if(cff_dict(b, td_a, td_z, 18, &v1, &v2) >= 2){   /* Private: size, offset */
        long pa = cff + v2, pz = pa + v1, s1 = 0, s2 = 0;
        if(cff_dict(b, pa, pz, 19, &s1, &s2)) ls = pa + s2;
    }
    long a, z;
    a = cff_index_item(b, cs, g, &z);
    if(!a) return 0;
    c->n = 0; c->nc = 0;
    CffMaq m; m.b = b; m.c = c; m.x = 0; m.y = 0; m.aberto = 0;
    m.cs = cs; m.gs = gsub; m.ls = ls;
    double st[48]; int ns = 0, nstem = 0;
    if(!cff_corre(&m, a, z, st, &ns, &nstem, 0)) return 0;
    cff_fecha(&m);
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
/* ─── O DESENHO É DO CORPO, e não se escala um por todos ─────────────────────────────
 *
 * A Computer Modern tem um desenho POR TAMANHO — `sfrm1000` para 10 pt, `sfbx2488` para
 * 24,88 — e não são o mesmo desenho ampliado: os traços têm espessura própria em cada um.
 * MEDIDO na palavra «Dourado» da capa: a largura bate a 3,6% (o corpo está certo), mas a
 * tinta é 0,80× a do gabarito — as letras ocupam a caixa e pintam menos, porque escalar o
 * desenho de 10 pt até 23,42 afina os traços que o desenho de 24,88 tem grossos.
 *
 * Aqui estão os desenhos que o gabarito usa, um por corpo, e `spline_por_corpo` escolhe o
 * mais próximo. É a mesma leitura --- o que muda é qual ficheiro se abre. */
/* corpo em MANTISSA (10^-3 pt): o traduz não alinha `double` no const, e a estrela
 * já é discreta — um float aqui era a segunda régua. */
typedef struct { long corpo; const char *rm, *bx, *ti, *cc, *tt; } Desenho;
static const Desenho DESENHOS[] = {
    {  8000, "d-rm0800.otf", "d-bx1000.otf", "d-ti1000.otf", "d-cc1000.otf", "d-tt1000.otf" },
    { 10000, "d-rm1000.otf", "d-bx1000.otf", "d-ti1000.otf", "d-cc1000.otf", "d-tt1000.otf" },
    { 10950, "d-rm1095.otf", "d-bx1000.otf", "d-ti1000.otf", "d-cc1000.otf", "d-tt1095.otf" },
    { 12000, "d-rm1200.otf", "d-bx1200.otf", "d-ti1200.otf", "d-cc1200.otf", "d-tt1200.otf" },
    { 14400, "d-rm1200.otf", "d-bx1440.otf", "d-ti1440.otf", "d-cc1200.otf", "d-tt1200.otf" },
    { 17280, "d-rm1200.otf", "d-bx1728.otf", "d-ti1440.otf", "d-cc1728.otf", "d-tt1200.otf" },
    { 24880, "d-rm1200.otf", "d-bx2488.otf", "d-ti1440.otf", "d-cc1728.otf", "d-tt1200.otf" },
};
#define N_DESENHOS ((int)(sizeof DESENHOS / sizeof DESENHOS[0]))

/* o ficheiro do desenho para este corpo e esta variante (0 rm, 1 bx, 2 ti, 3 cc).
 * O corpo entra como MANTISSA INTEIRA na régua do Tf (10^-3 pt) — o chamador é discreto;
 * a tabela já está nessa régua. */
static const char *spline_por_corpo(long corpo_m, int variante){
    int melhor = 0; long dmin = 1L << 60;
    for(int i = 0; i < N_DESENHOS; i++){
        long d = DESENHOS[i].corpo - corpo_m; if(d < 0) d = -d;
        if(d < dmin){ dmin = d; melhor = i; }
    }
    switch(variante){
        case 1:  return DESENHOS[melhor].bx;
        case 2:  return DESENHOS[melhor].ti;
        case 3:  return DESENHOS[melhor].cc;
        case 4:  return DESENHOS[melhor].tt;   /* a monoespaçada — a estaca da largura */
        default: return DESENHOS[melhor].rm;
    }
}

static const char *SPLINE_REG[] = {
    /* A FONTE DO GABARITO, no repositório. É a cm-super — as Type 1 que o pdflatex embute
     * quando o preâmbulo pede `[T1]{fontenc}` e não declara fonte nenhuma: SFRM roman, SFBX
     * bold extended, SFTI itálica, SFCC versaletes. Convertidas para sfnt, que é o que este
     * leitor lê, e postas aqui para não dependerem de haver TeX no sistema. */
    "lib/fontes/documento-regular.otf",
    "../lib/fontes/documento-regular.otf",
    "/usr/share/fonts/lm/lmroman10-regular.otf",
    "/usr/share/texmf-dist/fonts/opentype/public/lm/lmroman10-regular.otf",
    "/usr/share/fonts/liberation-sans/LiberationSans-Regular.ttf",
    "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
};
static const char *SPLINE_NEG[] = {
    "lib/fontes/documento-negra.otf",
    "../lib/fontes/documento-negra.otf",
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
