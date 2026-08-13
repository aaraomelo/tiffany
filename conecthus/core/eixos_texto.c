/* conecthus/core/eixos_texto.c — distâncias reais em texto (caixa/teclado/φ/forma).
 * Layout de teclas (ABNT2 aproximado, linhas inferiores QWERTY+ç):
 *   q w e r t y u i o p
 *   a s d f g h j k l ç
 *   z x c v b n m
 * Fonemas PT: regras de digrafos + c/g diante de e,i. */
#include "eixos_texto.h"
#include <string.h>
#include <ctype.h>

#define EX_MAX 96
#define EX_INF 9999

static int utf8_cp(const unsigned char *s, unsigned *cp){
    if(!s || !*s){ *cp = 0; return 0; }
    unsigned c0 = s[0];
    if(c0 < 0x80){ *cp = c0; return 1; }
    if((c0 & 0xE0) == 0xC0 && s[1]){ *cp = ((c0 & 0x1F) << 6) | (s[1] & 0x3F); return 2; }
    if((c0 & 0xF0) == 0xE0 && s[1] && s[2]){
        *cp = ((c0 & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F); return 3;
    }
    *cp = c0; return 1;
}

/* base latina + flag de acento (1 se tinha diacrítico / ç) */
static unsigned base_lat(unsigned cp, int *acento){
    *acento = 0;
    if(cp >= 'A' && cp <= 'Z') cp = cp - 'A' + 'a';
    if(cp >= 'a' && cp <= 'z') return cp;
    switch(cp){
        case 0xE1: case 0xE0: case 0xE2: case 0xE3: case 0xE4: /* áàâãä */
        case 0xC1: case 0xC0: case 0xC2: case 0xC3: case 0xC4:
            *acento = 1; return 'a';
        case 0xE9: case 0xE8: case 0xEA: case 0xEB:
        case 0xC9: case 0xC8: case 0xCA: case 0xCB:
            *acento = 1; return 'e';
        case 0xED: case 0xEC: case 0xEE: case 0xEF:
        case 0xCD: case 0xCC: case 0xCE: case 0xCF:
            *acento = 1; return 'i';
        case 0xF3: case 0xF2: case 0xF4: case 0xF5: case 0xF6:
        case 0xD3: case 0xD2: case 0xD4: case 0xD5: case 0xD6:
            *acento = 1; return 'o';
        case 0xFA: case 0xF9: case 0xFB: case 0xFC:
        case 0xDA: case 0xD9: case 0xDB: case 0xDC:
            *acento = 1; return 'u';
        case 0xE7: case 0xC7: /* ç */
            *acento = 1; return 'c';
        case 0xF1: case 0xD1:
            *acento = 1; return 'n';
        default: return cp;
    }
}

/* normaliza para ASCII minúsculo (sem acentos) */
static int norm_ascii(const char *s, char *out, int cap, int *nacentos){
    int n = 0; *nacentos = 0;
    const unsigned char *p = (const unsigned char *)s;
    while(*p && n + 1 < cap){
        unsigned cp; int k = utf8_cp(p, &cp); if(k <= 0) break;
        int ac = 0; unsigned b = base_lat(cp, &ac);
        if(ac) (*nacentos)++;
        if(b < 128) out[n++] = (char)b;
        p += k;
    }
    out[n] = 0;
    return n;
}

/* Levenshtein clássico em strings ASCII curtas */
static int lev(const char *a, const char *b){
    int na = (int)strlen(a), nb = (int)strlen(b);
    if(na > EX_MAX) na = EX_MAX;
    if(nb > EX_MAX) nb = EX_MAX;
    if(na == 0) return nb;
    if(nb == 0) return na;
    int prev[EX_MAX + 1], cur[EX_MAX + 1];
    for(int j = 0; j <= nb; j++) prev[j] = j;
    for(int i = 1; i <= na; i++){
        cur[0] = i;
        for(int j = 1; j <= nb; j++){
            int c = (a[i - 1] == b[j - 1]) ? 0 : 1;
            int x = prev[j] + 1, y = cur[j - 1] + 1, z = prev[j - 1] + c;
            cur[j] = x < y ? (x < z ? x : z) : (y < z ? y : z);
        }
        for(int j = 0; j <= nb; j++) prev[j] = cur[j];
    }
    return prev[nb];
}

long eixos_caixa(const char *a, const char *b){
    if(!a || !b) return 0;
    char aa[EX_MAX], bb[EX_MAX];
    int na_acc = 0, nb_acc = 0;
    norm_ascii(a, aa, EX_MAX, &na_acc);
    norm_ascii(b, bb, EX_MAX, &nb_acc);
    int d = lev(aa, bb);
    int da = na_acc - nb_acc; if(da < 0) da = -da;
    /* cafe↔café: lev=0, da=1 → caixa=1 (acento); segmentação soma lev */
    return (long)(d + da);
}

/* coordenadas de tecla ABNT2/QWERTY: row 0..2, col 0..9 */
static int tecla_xy(unsigned ch, int *row, int *col){
    static const char *L0 = "qwertyuiop";
    static const char *L1 = "asdfghjkl";   /* ç = col 9 */
    static const char *L2 = "zxcvbnm";
    if(ch == 0xE7 || ch == 0xC7){ *row = 1; *col = 9; return 1; }
    if(ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
    for(int c = 0; L0[c]; c++) if((unsigned)L0[c] == ch){ *row = 0; *col = c; return 1; }
    for(int c = 0; L1[c]; c++) if((unsigned)L1[c] == ch){ *row = 1; *col = c; return 1; }
    for(int c = 0; L2[c]; c++) if((unsigned)L2[c] == ch){ *row = 2; *col = c; return 1; }
    return 0;
}

static int dist_tecla(unsigned ca, unsigned cb){
    int ac = 0; unsigned ba = base_lat(ca, &ac);
    int bc = 0; unsigned bb = base_lat(cb, &bc);
    /* ç permanece distintivo no teclado */
    if(ca == 0xE7 || ca == 0xC7) ba = 0xE7;
    if(cb == 0xE7 || cb == 0xC7) bb = 0xE7;
    int ra, ca_, rb, cb_;
    if(!tecla_xy(ba, &ra, &ca_) || !tecla_xy(bb, &rb, &cb_)) return 4;
    int dr = ra - rb; if(dr < 0) dr = -dr;
    int dc = ca_ - cb_; if(dc < 0) dc = -dc;
    return dr + dc;
}

long eixos_teclado(const char *a, const char *b){
    if(!a || !b) return 0;
    /* extrai codepoints (máx EX_MAX) */
    unsigned A[EX_MAX], B[EX_MAX];
    int na = 0, nb = 0;
    const unsigned char *p = (const unsigned char *)a;
    while(*p && na < EX_MAX){
        unsigned cp; int k = utf8_cp(p, &cp); if(k <= 0) break;
        if(cp > 32) A[na++] = cp;
        p += k;
    }
    p = (const unsigned char *)b;
    while(*p && nb < EX_MAX){
        unsigned cp; int k = utf8_cp(p, &cp); if(k <= 0) break;
        if(cp > 32) B[nb++] = cp;
        p += k;
    }
    if(na == 0) return nb ? nb * 4 : 0;
    if(nb == 0) return na * 4;
    /* Levenshtein com custo de substituição = dist_tecla */
    int prev[EX_MAX + 1], cur[EX_MAX + 1];
    for(int j = 0; j <= nb; j++) prev[j] = j * 4;
    for(int i = 1; i <= na; i++){
        cur[0] = i * 4;
        for(int j = 1; j <= nb; j++){
            int sub = prev[j - 1] + dist_tecla(A[i - 1], B[j - 1]);
            int del = prev[j] + 4;
            int ins = cur[j - 1] + 4;
            cur[j] = sub < del ? (sub < ins ? sub : ins) : (del < ins ? del : ins);
        }
        for(int j = 0; j <= nb; j++) prev[j] = cur[j];
    }
    return (long)prev[nb];
}

/* emite fonemas ASCII aproximados PT em out */
static int para_fonemas(const char *s, char *out, int cap){
    char tmp[EX_MAX];
    int acc = 0;
    norm_ascii(s, tmp, EX_MAX, &acc);
    /* reintroduzir ç: percorrer original */
    /* simplificação: trabalhar sobre norm + digrafos no ascii */
    int n = 0, i = 0, L = (int)strlen(tmp);
    while(i < L && n + 1 < cap){
        char c = tmp[i], n1 = (i + 1 < L) ? tmp[i + 1] : 0;
        /* digrafos */
        if(c == 'l' && n1 == 'h'){ out[n++] = 'L'; i += 2; continue; }
        if(c == 'n' && n1 == 'h'){ out[n++] = 'N'; i += 2; continue; }
        if(c == 'c' && n1 == 'h'){ out[n++] = 'S'; i += 2; continue; }
        if(c == 'r' && n1 == 'r'){ out[n++] = 'R'; i += 2; continue; }
        if(c == 's' && n1 == 's'){ out[n++] = 's'; i += 2; continue; }
        if(c == 'q' && n1 == 'u'){ out[n++] = 'k'; i += 2; continue; }
        if(c == 'g' && n1 == 'u' && (i + 2 < L) && (tmp[i + 2] == 'e' || tmp[i + 2] == 'i')){
            out[n++] = 'g'; i += 2; continue; /* gu+e/i → g */
        }
        if(c == 'c' && (n1 == 'e' || n1 == 'i')){ out[n++] = 's'; i++; continue; }
        if(c == 'g' && (n1 == 'e' || n1 == 'i')){ out[n++] = 'Z'; i++; continue; }
        if(c == 'c'){ out[n++] = 'k'; i++; continue; }
        if(c == 'x'){ out[n++] = 'S'; i++; continue; } /* approx */
        if(c == 'w'){ out[n++] = 'v'; i++; continue; }
        if(c == 'y'){ out[n++] = 'i'; i++; continue; }
        out[n++] = c; i++;
    }
    /* ç no original: se strip mapeou ç→c, 'c'+vogal e/i já é s;
     * cafe/café → k a f e — iguais */
    out[n] = 0;
    return n;
}

long eixos_fonetico(const char *a, const char *b){
    if(!a || !b) return 0;
    char fa[EX_MAX], fb[EX_MAX];
    para_fonemas(a, fa, EX_MAX);
    para_fonemas(b, fb, EX_MAX);
    return (long)lev(fa, fb);
}

long eixos_forma(const char *a, const char *b){
    if(!a || !b) return 0;
    /* forma = Levenshtein em bases (sem peso de acento extra) */
    char aa[EX_MAX], bb[EX_MAX];
    int xa, xb;
    norm_ascii(a, aa, EX_MAX, &xa);
    norm_ascii(b, bb, EX_MAX, &xb);
    return (long)lev(aa, bb);
}

void eixos_dist_texto(const char *a, const char *b, long D8[8]){
    if(!D8) return;
    D8[4] = eixos_caixa(a, b);
    D8[5] = eixos_teclado(a, b);
    D8[6] = eixos_fonetico(a, b);
    D8[7] = eixos_forma(a, b);
}
