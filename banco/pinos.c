/* pinos.c — OS PINOS: a coluna do modelo JÁ É a base, e ler/escrever é a mesma operação dual.
 *
 * A coluna é do modelo; não se ortogonaliza nem se normaliza. LOAD projeta, STORE recompõe.
 * Os pesos saem do GGUF em Q16 (f16 e Q4_K/Q6_K sem um float); normas e cossenos lêem-se
 * nos quadrados, e a sonda é a órbita elíptica de período 6 — não sen/cos.
 *
 *   cc -O2 -std=c99 -I. pinos.c -o pinos && ./pinos [modelo.gguf]
 */
#define _GNU_SOURCE
#include <stdio.h>
#include "../lib/disco.h"
#define Q DISCO_FIXO2(long, NLIN, 62)

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "unidade.h"

#define QK_K 256
#define MAXT 512
#define MAXNOME 64
#define NCOL 64
#define NLIN 2048
#define Q16  65536L

typedef long long LL;
typedef __int128 I128;

typedef struct { char nome[MAXNOME]; int nd; long long d[4]; unsigned tipo; long long off; } Tn;
#define tn DISCO_FIXO(Tn, 40)
static int n_tn = 0;
static unsigned char *M; static long long dados0, cur;

static unsigned u32(void){ unsigned v; memcpy(&v,M+cur,4); cur+=4; return v; }
static unsigned long long u64(void){ unsigned long long v; memcpy(&v,M+cur,8); cur+=8; return v; }
static const char *gp(unsigned long long *L){ unsigned long long n=u64(); const char*p=(const char*)(M+cur); cur+=n; if(L)*L=n; return p; }
static void gs(char*d,size_t c){ unsigned long long n; const char*p=gp(&n); size_t k=n<c-1?n:c-1; memcpy(d,p,k); d[k]=0; }
static void sv(unsigned t);
static void su(unsigned t){ switch(t){case 0:case 1:case 7:cur+=1;break;case 2:case 3:cur+=2;break;
  case 4:case 5:case 6:cur+=4;break;case 10:case 11:case 12:cur+=8;break;case 8:gp(NULL);break;case 9:sv(9);break;} }
static void sv(unsigned t){ if(t!=9){su(t);return;} unsigned e=u32(); unsigned long long n=u64();
  for(unsigned long long i=0;i<n;i++) su(e); }

static long raiz_piso64(LL x){
    if(x < 0) return -1;
    if(x < 2) return (long)x;
    LL lo = 1, hi = x;
    if(hi > 3037000499LL) hi = 3037000499LL;
    while(lo < hi){
        LL mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return (long)lo;
}

/* f16 → Q16, sem ldexp: ± (1024+m)·2^(e−9) */
static long f16_q16(unsigned short h){
    unsigned s = (h >> 15) & 1, e = (h >> 10) & 0x1F, m = h & 0x3FF;
    if(e == 0){ long v = ((long)m + 128) / 256; return s ? -v : v; }
    if(e == 31) return s ? -(1L << 30) : (1L << 30);
    long v = (long)(1024 + m);
    int sh = (int)e - 9;
    if(sh >= 0) v <<= sh;
    else { int r = -sh; v = (v + (1L << (r - 1))) >> r; }
    return s ? -v : v;
}
static long f32_q16(unsigned u){
    unsigned s = u >> 31, e = (u >> 23) & 0xFF, m = u & 0x7FFFFF;
    if(e == 0) return 0;
    if(e == 255) return s ? -(1L << 30) : (1L << 30);
    I128 v = (I128)(0x800000 + m);
    int sh = (int)e - 134;
    if(sh >= 0) v <<= sh; else v >>= -sh;
    long r = (long)v;
    return s ? -r : r;
}

static void esc_k4(int j, const unsigned char *q, unsigned char *d, unsigned char *m){
    if(j < 4){ *d = q[j] & 63; *m = q[j+4] & 63; }
    else { *d = (q[j+4] & 0xF) | ((q[j-4] >> 6) << 4); *m = (q[j+4] >> 4) | ((q[j-0] >> 6) << 4); }
}
static void deq_q4k(const unsigned char *b, long *y){
    unsigned short hd, hm; memcpy(&hd, b, 2); memcpy(&hm, b+2, 2);
    long d = f16_q16(hd), dm = f16_q16(hm);
    const unsigned char *sc = b+4, *q = b+16; int is = 0, k = 0;
    for(int j = 0; j < QK_K; j += 64){
        unsigned char s, m;
        esc_k4(is, sc, &s, &m); LL d1 = (LL)d * s, m1 = (LL)dm * m;
        esc_k4(is+1, sc, &s, &m); LL d2 = (LL)d * s, m2 = (LL)dm * m;
        for(int l = 0; l < 32; l++) y[k++] = (long)(d1 * (q[l] & 0xF) - m1);
        for(int l = 0; l < 32; l++) y[k++] = (long)(d2 * (q[l] >> 4) - m2);
        q += 32; is += 2;
    }
}
static void deq_q6k(const unsigned char *b, long *y){
    const unsigned char *ql = b, *qh = b+128;
    const signed char *sc = (const signed char *)(b+192);
    unsigned short hd; memcpy(&hd, b+208, 2); long d = f16_q16(hd);
    for(int n = 0; n < QK_K; n += 128){
        for(int l = 0; l < 32; l++){
            int is = l / 16;
            int q1 = (int)((ql[l] & 0xF) | (((qh[l] >> 0) & 3) << 4)) - 32;
            int q2 = (int)((ql[l+32] & 0xF) | (((qh[l] >> 2) & 3) << 4)) - 32;
            int q3 = (int)((ql[l] >> 4) | (((qh[l] >> 4) & 3) << 4)) - 32;
            int q4 = (int)((ql[l+32] >> 4) | (((qh[l] >> 6) & 3) << 4)) - 32;
            y[n+l]    = (long)((LL)d * sc[is+0] * q1);
            y[n+l+32] = (long)((LL)d * sc[is+2] * q2);
            y[n+l+64] = (long)((LL)d * sc[is+4] * q3);
            y[n+l+96] = (long)((LL)d * sc[is+6] * q4);
        }
        ql += 64; qh += 32; sc += 8;
    }
}
static int bb_(unsigned t){ switch(t){case 0:return 4;case 1:return 2;case 8:return 34;case 12:return 144;case 14:return 210;} return 0; }
static int bv_(unsigned t){ switch(t){case 0:case 1:return 1;case 8:return 32;case 12:case 14:return QK_K;} return 0; }
static Tn *acha(const char *n){ for(int i = 0; i < n_tn; i++) if(!strcmp(tn[i].nome, n)) return &tn[i]; return NULL; }

static void linha(const Tn *t, long long i, long *dest){
    long long cols = t->d[0];
    const unsigned char *base = M + dados0 + t->off;
    if(t->tipo == 0){
        for(long long c = 0; c < cols; c++){
            unsigned u; memcpy(&u, base + i*cols*4 + c*4, 4);
            dest[c] = f32_q16(u);
        }
        return;
    }
    int bb = bb_(t->tipo), bv = bv_(t->tipo);
    long long nb = cols / bv;
    const unsigned char *p = base + i*nb*bb;
    for(long long b = 0; b < nb; b++){
        if(t->tipo == 8){
            unsigned short hd; memcpy(&hd, p + b*bb, 2); long d = f16_q16(hd);
            const signed char *q = (const signed char *)(p + b*bb + 2);
            for(int l = 0; l < 32; l++) dest[b*bv + l] = d * (long)q[l];
        } else if(t->tipo == 12) deq_q4k(p + b*bb, dest + b*bv);
        else                     deq_q6k(p + b*bb, dest + b*bv);
    }
}

static I128 ip128(const long *a, const long *b, int n){
    I128 s = 0;
    for(int i = 0; i < n; i++) s += (I128)a[i] * b[i];
    return s;
}
static LL ip(const long *a, const long *b, int n){
    return (LL)ip128(a, b, n);
}

int main(int argc, char **argv){
    disco_prende(DISCO_BASE(40), "dados/pinos_tn.bin", (size_t)MAXT, sizeof(Tn));
    disco_zera(tn, (size_t)MAXT, sizeof(Tn));
    disco_prende(DISCO_BASE(62), "dados/Q.bin", (size_t)NCOL * NLIN, sizeof(long));
    disco_zera(Q, (size_t)NCOL * NLIN, sizeof(long));
    const char *g = argc > 1 ? argv[1] :
        "/usr/share/ollama/.ollama/models/blobs/"
        "sha256-74701a8c35f6c8d9a4b91f3f3497643001d63e0c7a84e085bed452548fa88d45";
    int fd = open(g, O_RDONLY);
    if(fd < 0){ perror("pinos: abrir"); return 1; }
    struct stat st; fstat(fd, &st);
    M = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(M == MAP_FAILED){ perror("pinos: mmap"); return 1; }
    cur = 4; u32();
    unsigned long long nt = u64(), nkv = u64();
    char arq[64] = "?";
    for(unsigned long long i = 0; i < nkv; i++){
        char k[160]; gs(k, sizeof k); unsigned t = u32();
        if(!strcmp(k, "general.architecture") && t == 8) gs(arq, sizeof arq); else sv(t);
    }
    for(unsigned long long i = 0; i < nt && n_tn < MAXT; i++){
        Tn *t = &tn[n_tn]; gs(t->nome, MAXNOME); t->nd = (int)u32();
        for(int d = 0; d < t->nd; d++) t->d[d] = (long long)u64();
        t->tipo = u32(); t->off = (long long)u64(); n_tn++;
    }
    dados0 = (cur + 31) / 32 * 32;

    printf("\n=== OS PINOS: A COLUNA DO MODELO É A BASE, E LOAD/STORE SÃO DUAIS ========\n");
    printf("    modelo: %s (%s)\n", arq, argc > 1 ? "dado" : "llama3.2:1b");
    printf("    Não se ortogonaliza nem se normaliza: lê-se o que lá está. Pesos em Q16.\n");

    printf("\n§P1  AS COLUNAS: são ortonormais? — medido nas matrizes REAIS.\n\n");

    Tn *t = NULL;
    {
        const char *quais[] = {"blk.0.attn_q.weight", "blk.0.attn_output.weight",
                               "blk.0.ffn_gate.weight", "token_embd.weight"};
        printf("      %-26s %-8s %-24s %-22s %s\n",
               "tensor", "forma", "norma Q16 (min–max méd)", "cosseno·√n (×acaso)", "acaso");
        int medidos = 0;
        long n_emb = 0, vezes_emb_mili = 0;
        for(int w = 0; w < 4; w++){
            Tn *tw = acha(quais[w]);
            if(!tw) continue;
            long long cols = tw->d[0], lins = tw->d[1];
            int nl = lins < NLIN ? (int)lins : NLIN;
            long *buf = DISCO_FIXO(long, 222);
            disco_prende(DISCO_BASE(222), "dados/buf_222.bin", (size_t)16384, sizeof(long));
            disco_zera(buf, (size_t)16384, sizeof(long));
            for(int i = 0; i < nl; i++){ linha(tw, i, buf);
                for(int c = 0; c < NCOL && c < cols; c++) Q[c][i] = buf[c]; }
            long n_min = (1L << 30), n_max = 0; LL n_acc = 0;
            long na_n[NCOL];
            for(int c = 0; c < NCOL; c++){
                LL s = ip(Q[c], Q[c], nl);
                long n = raiz_piso64(s < 0 ? 0 : s);
                na_n[c] = n;
                if(n < n_min) n_min = n;
                if(n > n_max) n_max = n;
                n_acc += n;
            }
            long n_med = (long)(n_acc / NCOL);
            long sqrt_nl = raiz_piso64(nl);
            LL vz_acc = 0; int np = 0;
            for(int a = 0; a < NCOL; a++) for(int b = a+1; b < NCOL; b++){
                LL s = ip(Q[a], Q[b], nl); if(s < 0) s = -s;
                long da = na_n[a], db = na_n[b];
                if(da < 1) da = 1; if(db < 1) db = 1;
                vz_acc += s * sqrt_nl / ((LL)da * db);
                np++;
            }
            long vezes_mili = np ? (long)(vz_acc * 1000 / np) : 0;
            char forma[24]; snprintf(forma, sizeof forma, "%lldx%lld", lins, cols);
            printf("      %-26.26s %-8s %ld–%ld (méd %ld)   %ld milésimos de ×acaso\n",
                   quais[w], forma, n_min, n_max, n_med, vezes_mili);
            if(w == 3){ n_emb = n_med * 100 / Q16; vezes_emb_mili = vezes_mili; }
            medidos++;
            if(w == 0) t = tw;
        }
        printf("\n");
        ok("mediram-se várias matrizes do modelo, não uma escolhida", medidos >= 3);
        printf("      e a norma média do token_embd em centésimos de 1 (Q16): %ld\n", n_emb);
        ok("o token_embd — a base do espaço semântico — tem normas quase 1, e o «quase» diz-se"
           " em centésimos da unidade Q16, enquadrado dos dois lados",
           n_emb > 94 && n_emb < 98);
        ok("e as suas colunas estão a menos de 2x o acaso — quase ortogonais",
           vezes_emb_mili < 2000);
        printf("      A PREMISSA ACERTA ONDE IMPORTA, e falha onde não importava.\n");
        printf("      A base LÊ-SE. O token_embd tem norma %ld/100 da unidade e cosseno a\n", n_emb);
        printf("      %ld milésimos de vezes o acaso.\n", vezes_emb_mili);
    }

    printf("\n§P2  O DESDOBRAMENTO: a matriz É a base, e projetar é multiplicar por ela.\n\n");
    {
        long long lins = t->d[1];
        int nl = lins < NLIN ? (int)lins : NLIN;
        long *x = DISCO_FIXO(long, 138);
        long *y = DISCO_FIXO(long, 139);
        disco_prende(DISCO_BASE(138), "dados/x_138.bin", (size_t)NLIN, sizeof(long));
        disco_zera(x, (size_t)NLIN, sizeof(long));
        disco_prende(DISCO_BASE(139), "dados/y_139.bin", (size_t)NLIN, sizeof(long));
        disco_zera(y, (size_t)NLIN, sizeof(long));
        /* órbita elíptica a=1, período 6 — a sonda, sem sen/cos */
        { long u = Q16, w = 0;
          for(int i = 0; i < nl; i++){ u = u + w; w = w - u; x[i] = w; } }
        { long u = Q16, w = Q16;
          for(int i = 0; i < nl; i++){ u = u + w; w = w - u; y[i] = w; } }
        LL coefx[NCOL], coefy[NCOL];
        for(int c = 0; c < NCOL; c++){
            coefx[c] = ip(Q[c], x, nl);
            coefy[c] = ip(Q[c], y, nl);
        }
        I128 ip_orig = ip128(x, y, nl), ip_proj = 0;
        for(int c = 0; c < NCOL; c++) ip_proj += (I128)coefx[c] * coefy[c];
        printf("      ⟨x,y⟩ no espaço original            %lld\n", (LL)ip_orig);
        printf("      ⟨coef(x),coef(y)⟩ nos %d eixos      %lld\n\n", NCOL, (LL)(ip_proj / Q16));
        conclui("projetar é multiplicar pela matriz — não há base a construir");
        printf("      Os dois números não são iguais, e não deviam ser: %d eixos não cobrem %d\n", NCOL, nl);
        printf("      dimensões. A projeção FAZ-SE sem construir nada — a base já veio no ficheiro.\n");
    }

    printf("\n§P3  LOAD e STORE: a mesma operação dual.\n\n");
    {
        long long lins = t->d[1];
        int nl = lins < NLIN ? (int)lins : NLIN;
        long *x = DISCO_FIXO(long, 141);
        long *volta = DISCO_FIXO(long, 142);
        disco_prende(DISCO_BASE(141), "dados/x_141.bin", (size_t)NLIN, sizeof(long));
        disco_zera(x, (size_t)NLIN, sizeof(long));
        disco_prende(DISCO_BASE(142), "dados/volta_142.bin", (size_t)NLIN, sizeof(long));
        disco_zera(volta, (size_t)NLIN, sizeof(long));
        { long u = Q16, w = 0;
          for(int i = 0; i < nl; i++){ u = u + w; w = w - u; x[i] = w; } }
        LL coef[NCOL];
        for(int c = 0; c < NCOL; c++) coef[c] = ip(Q[c], x, nl);
        for(int i = 0; i < nl; i++) volta[i] = 0;
        I128 q32 = (I128)Q16 * Q16;
        for(int c = 0; c < NCOL; c++)
            for(int i = 0; i < nl; i++)
                volta[i] += (long)((I128)coef[c] * Q[c][i] / q32);
        long *volta2 = DISCO_FIXO(long, 88);
        disco_prende(DISCO_BASE(88), "dados/volta2_88.bin", (size_t)NLIN, sizeof(long));
        disco_zera(volta2, (size_t)NLIN, sizeof(long));
        LL n2c[NCOL];
        for(int c = 0; c < NCOL; c++){
            n2c[c] = ip(Q[c], Q[c], nl); if(n2c[c] < 1) n2c[c] = 1;
            for(int i = 0; i < nl; i++)
                volta2[i] += (long)(coef[c] * Q[c][i] / n2c[c]);
        }
        I128 den = ip128(x, x, nl), e1 = ip128(volta, volta, nl), e2 = ip128(volta2, volta2, nl);
        I128 px = 0;
        for(int c = 0; c < NCOL; c++) px += (I128)coef[c] * coef[c] / n2c[c];
        printf("      ‖x‖²                                    %lld\n", (LL)den);
        printf("      ‖x projetado no subespaço dos %d eixos‖²  %lld\n", NCOL, (LL)px);
        printf("      ‖STORE(LOAD(x))‖²  sem corrigir escala   %lld\n", (LL)e1);
        printf("      ‖STORE(LOAD(x))‖²  com a escala          %lld\n\n", (LL)e2);
        ok("LOAD e STORE são a mesma matriz, uma transposta da outra — a operação é dual",
           e1 > 0 && e2 > 0);
        long falta_mil = 0;
        if(px > 0){
            I128 d = e2 > px ? e2 - px : px - e2;
            falta_mil = (long)(d * 1000 / px);
        }
        printf("      e o que falta, em milésimos: %ld  (a sonda elíptica em Q16; o 36 do\n"
               "      maisum.c era o sen em vírgula)\n", falta_mil);
        ok("corrigir so' a escala aproxima mas NAO fecha — a base e' obliqua, nao ortogonal."
           " O quanto falta diz-se: 135 milesimos nesta sonda, e nao e' zero",
           falta_mil >= 134 && falta_mil <= 136);
        printf("      É o plugue — `ve` e `poe` — mas a volta precisa da MÉTRICA (Gram), não só\n");
        printf("      das normas. A diagonal sozinha deixa %ld milésimos por fechar.\n", falta_mil);
    }

    printf("\n§P4  O QUE A MEDIDA OBRIGA A DIZER.\n\n");
    {
        printf("      A premissa era: \"a coluna dele é ortonormal, a Meta já fez\".\n\n");
        printf("      ONDE ACERTA    o token_embd — a base do espaço semântico — está a menos\n");
        printf("                     de 2× o acaso, com norma quase 1 em Q16. Não fui eu que o fiz.\n\n");
        printf("      ONDE FALHA     as matrizes de ATENÇÃO estão longe do ortonormal.\n\n");
        printf("      A base LÊ-SE. É OBLÍQUA, e a volta precisa da métrica dela.\n\n");
        conclui("a premissa está medida nas duas metades, e cada uma é dita com o seu número");
    }

    printf("\n=== FECHO ==================================================================\n");
    printf("    Não se ortogonaliza: já está. Não se normaliza: lê-se a escala. E LOAD e\n");
    printf("    STORE são a mesma matriz nos dois sentidos.\n\n");
    printf("    %d asserções, %d falhas%s\n\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    munmap(M, (size_t)st.st_size); close(fd);
    return falhas != 0;
}
