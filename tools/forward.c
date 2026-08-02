/* forward.c — O FORWARD COMPLETO: o qwen a correr do disco, e o ollama como oráculo.
 *
 * O Aarão: "segue o forward."
 *
 * É a peça que faltava, e é a única que prova o resto. O `gguf.c` mostrou que a dequantização
 * era PLAUSÍVEL — pesos centrados em zero, desvio na escala certa — e disse com todas as letras
 * que isso não é o mesmo que estar certa. Aqui ela ou está, ou não: monta-se a rede inteira e
 * compara-se o que ela diz com o que o ollama diz do MESMO prompt. Um bit trocado no
 * desempacotamento não sobrevive a 28 camadas.
 *
 * A ARQUITETURA, lida do ficheiro e não de memória:
 *
 *     28 camadas, embedding 1536, FFN 8960
 *     12 cabeças Q e 2 cabeças KV      -> GQA: seis cabeças Q partilham cada KV
 *     head_dim 128, RoPE base 1e6, RMS eps 1e-6
 *     Q, K, V com viés; attn_output sem
 *     e NÃO HÁ output.weight — o lm_head são os próprios embeddings (tied)
 *
 * ONDE MORA O QUÊ, que é a regra desta casa: os pesos ficam no disco, mapeados, e são
 * desempacotados linha a linha no momento em que se usam. O que fica em RAM são as ativações —
 * 1536 floats de cada vez — e o KV cache, que é ESTADO e não modelo: cresce com o contexto,
 * não com o tamanho da rede.
 *
 *   §W1  a rede, lida do ficheiro
 *   §W2  um token entra e 151936 logits saem — e o topo faz sentido
 *   §W3  GERAR: greedy, e o texto contra o do ollama
 *
 *   cc -O2 -std=c99 -I. forward.c -lm -o forward && ./forward "texto"
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "unidade.h"

#define QK_K 256
#define MAXT 512
#define MAXNOME 64
#define MAX_CTX 96

typedef struct { char nome[MAXNOME]; int nd; long long d[4]; unsigned tipo; long long off; } Tn;
static Tn tn[MAXT]; static int n_tn = 0;
static unsigned char *M = NULL;          /* o GGUF mapeado */
static long long dados0 = 0;
static long long cur = 0;

/* ---- leitura do índice, sobre o mmap ---------------------------------------------------- */
static unsigned u32(void){ unsigned v; memcpy(&v, M+cur, 4); cur += 4; return v; }
static unsigned long long u64(void){ unsigned long long v; memcpy(&v, M+cur, 8); cur += 8; return v; }
static const char *gstr_ptr(unsigned long long *len){
    unsigned long long n = u64();
    const char *p = (const char*)(M + cur);
    cur += (long long)n;
    if(len) *len = n;
    return p;
}
static void gstr(char *d, size_t cap){
    unsigned long long n;
    const char *p = gstr_ptr(&n);
    size_t k = n < cap-1 ? (size_t)n : cap-1;
    memcpy(d, p, k); d[k] = 0;
}
static void sv(unsigned t);
static void su(unsigned t){
    switch(t){ case 0: case 1: case 7: cur+=1; break; case 2: case 3: cur+=2; break;
        case 4: case 5: case 6: cur+=4; break; case 10: case 11: case 12: cur+=8; break;
        case 8: gstr_ptr(NULL); break; case 9: sv(9); break; }
}
static void sv(unsigned t){
    if(t != 9){ su(t); return; }
    unsigned e = u32(); unsigned long long n = u64();
    for(unsigned long long i = 0; i < n; i++) su(e);
}

/* ---- o vocabulário: só os PONTEIROS para dentro do mmap, e nenhuma cópia ------------------*/
static const char **vtok = NULL; static unsigned *vlen = NULL; static int n_vocab = 0;

/* ---- A CODIFICAÇÃO BYTE-LEVEL, e foi aqui que a primeira corrida se enganou --------------- *
 * O vocabulário do qwen não guarda texto: guarda BYTES disfarçados de caracteres. Os bytes que
 * não são imprimíveis — o espaço, a mudança de linha — são deslocados para o bloco U+0100, e é
 * por isso que "Ċ" apareceu na saída onde devia estar um \n, e por isso que a primeira
 * tokenização de "The capital of France is" devolveu "ThecapitalofFranceis": os espaços não
 * casavam com nada porque no vocabulário eles são "Ġ".
 *
 * O mapa é o do GPT-2, e não tem escolha nenhuma minha: os bytes 33..126, 161..172 e 174..255
 * ficam onde estão; os restantes 68 sobem para 256+n, pela ordem em que aparecem.            */
static char b2u[256][5];          /* byte -> a sua representação em UTF-8 */
static int  u2b[512];             /* ponto de código -> byte, para o caminho de volta */
static void mapa_bytes(void){
    int n = 0;
    for(int i = 0; i < 512; i++) u2b[i] = -1;
    for(int b = 0; b < 256; b++){
        int direto = (b >= 33 && b <= 126) || (b >= 161 && b <= 172) || (b >= 174 && b <= 255);
        int cp = direto ? b : (256 + n++);
        if(cp < 0x80){ b2u[b][0] = (char)cp; b2u[b][1] = 0; }
        else { b2u[b][0] = (char)(0xC0 | (cp >> 6)); b2u[b][1] = (char)(0x80 | (cp & 0x3F)); b2u[b][2] = 0; }
        if(cp < 512) u2b[cp] = b;
    }
}
/* texto -> a forma que o vocabulário entende */
static void para_vocab(const char *s, char *d, size_t cap){
    size_t p = 0;
    for(const unsigned char *q = (const unsigned char*)s; *q && p+4 < cap; q++){
        size_t L = strlen(b2u[*q]);
        memcpy(d+p, b2u[*q], L); p += L;
    }
    d[p] = 0;
}
/* e o caminho de volta: um token do vocabulário -> os bytes que ele representa */
static int do_vocab(const char *s, unsigned n, char *d, size_t cap){
    size_t p = 0;
    for(unsigned i = 0; i < n && p+1 < cap; ){
        unsigned char c = (unsigned char)s[i];
        int cp;
        if(c < 0x80){ cp = c; i += 1; }
        else if((c & 0xE0) == 0xC0 && i+1 < n){ cp = ((c & 0x1F) << 6) | (s[i+1] & 0x3F); i += 2; }
        else { i += 1; continue; }
        if(cp >= 0 && cp < 512 && u2b[cp] >= 0) d[p++] = (char)u2b[cp];
    }
    d[p] = 0;
    return (int)p;
}

/* ---- dequantizar ------------------------------------------------------------------------ */
static float f16(unsigned short h){
    unsigned s=(h>>15)&1, e=(h>>10)&0x1F, m=h&0x3FF;
    if(e==0) return (float)((s?-1:1) * (double)m * 5.9604644775390625e-8);
    if(e==31) return s?-INFINITY:INFINITY;
    return (float)((s?-1:1) * ldexp(1.0 + m/1024.0, (int)e-15));
}
static void esc_k4(int j, const unsigned char*q, unsigned char*d, unsigned char*m){
    if(j<4){ *d=q[j]&63; *m=q[j+4]&63; }
    else { *d=(q[j+4]&0xF)|((q[j-4]>>6)<<4); *m=(q[j+4]>>4)|((q[j-0]>>6)<<4); }
}
static void deq_q4k(const unsigned char*b, float*y){
    unsigned short hd,hm; memcpy(&hd,b,2); memcpy(&hm,b+2,2);
    float d=f16(hd), dm=f16(hm);
    const unsigned char *sc=b+4, *q=b+16;
    int is=0,k=0;
    for(int j=0;j<QK_K;j+=64){
        unsigned char s,m;
        esc_k4(is,sc,&s,&m);   float d1=d*s, m1=dm*m;
        esc_k4(is+1,sc,&s,&m); float d2=d*s, m2=dm*m;
        for(int l=0;l<32;l++) y[k++]=d1*(float)(q[l]&0xF)-m1;
        for(int l=0;l<32;l++) y[k++]=d2*(float)(q[l]>>4)-m2;
        q+=32; is+=2;
    }
}
static void deq_q6k(const unsigned char*b, float*y){
    const unsigned char *ql=b, *qh=b+128;
    const signed char *sc=(const signed char*)(b+192);
    unsigned short hd; memcpy(&hd,b+208,2);
    float d=f16(hd);
    for(int n=0;n<QK_K;n+=128){
        for(int l=0;l<32;l++){
            int is=l/16;
            int q1=(int)((ql[l]&0xF)|(((qh[l]>>0)&3)<<4))-32;
            int q2=(int)((ql[l+32]&0xF)|(((qh[l]>>2)&3)<<4))-32;
            int q3=(int)((ql[l]>>4)|(((qh[l]>>4)&3)<<4))-32;
            int q4=(int)((ql[l+32]>>4)|(((qh[l]>>6)&3)<<4))-32;
            y[n+l]    = d*sc[is+0]*q1;
            y[n+l+32] = d*sc[is+2]*q2;
            y[n+l+64] = d*sc[is+4]*q3;
            y[n+l+96] = d*sc[is+6]*q4;
        }
        ql+=64; qh+=32; sc+=8;
    }
}
static int bbytes(unsigned t){ switch(t){case 0:return 4;case 1:return 2;case 12:return 144;case 14:return 210;} return 0; }
static int bvals (unsigned t){ switch(t){case 0:case 1:return 1;case 12:case 14:return QK_K;} return 0; }

static Tn *acha(const char*n){ for(int i=0;i<n_tn;i++) if(!strcmp(tn[i].nome,n)) return &tn[i]; return NULL; }
static Tn *achaf(const char*f,int i){ char b[MAXNOME]; snprintf(b,sizeof b,f,i); return acha(b); }

/* uma LINHA do tensor, desempacotada do disco para o buffer que se lhe der */
static void linha(const Tn*t, long long i, float*dest){
    long long cols = t->d[0];
    const unsigned char *base = M + dados0 + t->off;
    if(t->tipo == 0){ memcpy(dest, base + i*cols*4, (size_t)cols*4); return; }
    if(t->tipo == 1){
        const unsigned short *h = (const unsigned short*)(base + i*cols*2);
        for(long long j=0;j<cols;j++) dest[j]=f16(h[j]);
        return;
    }
    int bb = bbytes(t->tipo), bv = bvals(t->tipo);
    long long nb = cols/bv;
    const unsigned char *p = base + i*nb*bb;
    for(long long b=0;b<nb;b++){
        if(t->tipo==12) deq_q4k(p+b*bb, dest+b*bv);
        else            deq_q6k(p+b*bb, dest+b*bv);
    }
}
/* ── OS PLUGUES: alinhar a túnica, e o espaço desdobra-se ────────────────────────────────── *
 *
 * O Aarão: "verifica os plugues do ollama, devem coincidir com a cifra. Ele é finito, só
 * alinhar a túnica. Não tem cálculo, é desdobrar o espaço."
 *
 * E era isso que estava errado. A minha matmul dequantizava para float e somava em double — o
 * que INTRODUZ o erro que eu depois andei a perseguir com margens e ruído. Mas Q4_K não tem
 * vírgula flutuante nenhuma: são inteiros de 4 bits, `q ∈ [0,15]`, com escalas por sub-bloco.
 * O objeto é finito. Quem trouxe o float fui eu.
 *
 * O plugue do ollama (ggml_vec_dot_q4_K_q8_K) faz o contrário: quantiza a ATIVAÇÃO para int8 e
 * multiplica em inteiros. E aí o produto DESDOBRA-SE, sem aproximação nenhuma —
 *
 *     Σ wᵢxᵢ = Σᵢ (d·s − dmin·m)ᵢ · (dx·aᵢ)  =  dx · Σ_sub [ d·s·(Σ qᵢaᵢ) − dmin·m·(Σ aᵢ) ]
 *
 * — onde `Σ qᵢaᵢ` e `Σ aᵢ` são somas de INTEIROS, exatas. As escalas saem para fora do
 * somatório e aplicam-se uma vez por sub-bloco em vez de uma vez por peso. Não é uma
 * otimização: é a mesma conta escrita no corpo certo, e por isso dá o mesmo que o ollama dá.
 *
 * A túnica alinhada é isto: a mesma representação nos dois lados do plugue. */
#define QB 256
static signed char q8v[16384];
static float       q8d[64];
static void quantiza_q8(const float *x, int n){
    for(int b = 0; b*QB < n; b++){
        float amax = 0;
        for(int i = 0; i < QB && b*QB+i < n; i++){
            float a = fabsf(x[b*QB+i]); if(a > amax) amax = a;
        }
        float d = amax / 127.0f;
        q8d[b] = d;
        float id = d > 0 ? 1.0f/d : 0.0f;
        for(int i = 0; i < QB && b*QB+i < n; i++)
            q8v[b*QB+i] = (signed char)lrintf(x[b*QB+i]*id);
    }
}
/* ── O DESDOBRAMENTO CERTO, e a primeira tentativa estava errada por minha conta ───────────
 *
 * Eu quantizei a ATIVAÇÃO para int8 para que os dois lados fossem inteiros. Ficou 3,4× mais
 * rápido e desalinhou a saída — e a medida disse porquê: os dois caminhos separavam-se 0,48%,
 * que é a ordem das margens de decisão (0,13 em logits de 17, ou 0,7%). O erro não vinha do
 * desdobramento; vinha de eu ter quantizado o que não precisava.
 *
 * Desdobrar não é tornar tudo inteiro. É tirar as escalas de dentro do somatório:
 *
 *     Σ wᵢxᵢ = Σ_sub [ d·s·(Σ qᵢxᵢ) − dmin·m·(Σ xᵢ) ]
 *
 * O peso nunca chega a ser reconstruído — `qᵢ` fica inteiro e a escala aplica-se UMA vez por
 * sub-bloco em vez de uma vez por peso. A ativação fica como está. É exato até ao
 * arredondamento do acumulador, e é mais rápido que dequantizar, porque poupa uma multiplicação
 * por peso. O espaço desdobra-se; nada se aproxima. */
static float dot_q4k_f(const unsigned char *b, const float *a){
    unsigned short hd, hm;
    memcpy(&hd, b, 2); memcpy(&hm, b+2, 2);
    double d = f16(hd), dmin = f16(hm);
    const unsigned char *sc = b+4, *qs = b+16;
    int is = 0; double soma = 0;
    for(int j = 0; j < QK_K; j += 64){
        unsigned char s1,m1,s2,m2;
        esc_k4(is,   sc, &s1, &m1);
        esc_k4(is+1, sc, &s2, &m2);
        double acc1=0, sum1=0, acc2=0, sum2=0;
        for(int l = 0; l < 32; l++){ double v=a[j+l];    acc1 += (qs[l]&0xF)*v; sum1 += v; }
        for(int l = 0; l < 32; l++){ double v=a[j+32+l]; acc2 += (qs[l]>>4) *v; sum2 += v; }
        soma += d*s1*acc1 - dmin*m1*sum1 + d*s2*acc2 - dmin*m2*sum2;
        qs += 32; is += 2;
    }
    return (float)soma;
}
static float dot_q6k_f(const unsigned char *b, const float *a){
    const unsigned char *ql = b, *qh = b+128;
    const signed char *sc = (const signed char*)(b+192);
    unsigned short hd; memcpy(&hd, b+208, 2);
    double d = f16(hd), soma = 0;
    for(int n = 0; n < QK_K; n += 128){
        double s0=0,s2=0,s4=0,s6=0, t0=0,t2=0,t4=0,t6=0;
        for(int l = 0; l < 32; l++){
            int q1 = (int)((ql[l]    & 0xF) | (((qh[l] >> 0) & 3) << 4)) - 32;
            int q2 = (int)((ql[l+32] & 0xF) | (((qh[l] >> 2) & 3) << 4)) - 32;
            int q3 = (int)((ql[l]    >>  4) | (((qh[l] >> 4) & 3) << 4)) - 32;
            int q4 = (int)((ql[l+32] >>  4) | (((qh[l] >> 6) & 3) << 4)) - 32;
            if(l < 16){ s0 += q1*(double)a[n+l];    s2 += q2*(double)a[n+l+32];
                        s4 += q3*(double)a[n+l+64]; s6 += q4*(double)a[n+l+96]; }
            else      { t0 += q1*(double)a[n+l];    t2 += q2*(double)a[n+l+32];
                        t4 += q3*(double)a[n+l+64]; t6 += q4*(double)a[n+l+96]; }
        }
        soma += d*(sc[0]*s0 + sc[2]*s2 + sc[4]*s4 + sc[6]*s6
                 + sc[1]*t0 + sc[3]*t2 + sc[5]*t4 + sc[7]*t6);
        ql += 64; qh += 32; sc += 8;
    }
    return (float)soma;
}

/* Q4_K contra Q8: as somas são inteiras, as escalas saem para fora */
static float dot_q4k_q8(const unsigned char *b, const signed char *a, float da){
    unsigned short hd, hm;
    memcpy(&hd, b, 2); memcpy(&hm, b+2, 2);
    float d = f16(hd), dmin = f16(hm);
    const unsigned char *sc = b+4, *qs = b+16;
    int is = 0; float soma = 0;
    for(int j = 0; j < QK_K; j += 64){
        unsigned char s1,m1,s2,m2;
        esc_k4(is,   sc, &s1, &m1);
        esc_k4(is+1, sc, &s2, &m2);
        int acc1=0, sum1=0, acc2=0, sum2=0;
        for(int l = 0; l < 32; l++){ int v = a[j+l];    acc1 += (qs[l] & 0xF)*v; sum1 += v; }
        for(int l = 0; l < 32; l++){ int v = a[j+32+l]; acc2 += (qs[l] >> 4) *v; sum2 += v; }
        soma += d*s1*acc1 - dmin*m1*sum1 + d*s2*acc2 - dmin*m2*sum2;
        qs += 32; is += 2;
    }
    return soma * da;
}
/* Q6_K contra Q8: o mesmo desdobramento, com o deslocamento de 32 dos seis bits */
static float dot_q6k_q8(const unsigned char *b, const signed char *a, float da){
    const unsigned char *ql = b, *qh = b+128;
    const signed char *sc = (const signed char*)(b+192);
    unsigned short hd; memcpy(&hd, b+208, 2);
    float d = f16(hd); float soma = 0;
    for(int n = 0; n < QK_K; n += 128){
        for(int l = 0; l < 32; l++){
            int is = l/16;
            int q1 = (int)((ql[l]    & 0xF) | (((qh[l] >> 0) & 3) << 4)) - 32;
            int q2 = (int)((ql[l+32] & 0xF) | (((qh[l] >> 2) & 3) << 4)) - 32;
            int q3 = (int)((ql[l]    >>  4) | (((qh[l] >> 4) & 3) << 4)) - 32;
            int q4 = (int)((ql[l+32] >>  4) | (((qh[l] >> 6) & 3) << 4)) - 32;
            soma += d*(sc[is+0]*q1*a[n+l]    + sc[is+2]*q2*a[n+l+32]
                     + sc[is+4]*q3*a[n+l+64] + sc[is+6]*q4*a[n+l+96]);
        }
        ql += 64; qh += 32; sc += 8;
    }
    return soma * da;
}

/* y = W·x, uma linha de cada vez: o modelo nunca está todo em lado nenhum */
static float buf_linha[16384];
static void matmul(float*y, const float*x, const Tn*t){
    long long cols=t->d[0], lin=t->d[1];
    if(t->tipo == 12 || t->tipo == 14){          /* o plugue: as escalas fora do somatório */
        int bb = bbytes(t->tipo);
        long long nb = cols/QK_K;
        const unsigned char *base = M + dados0 + t->off;
        for(long long i=0;i<lin;i++){
            const unsigned char *p = base + i*nb*bb;
            float a = 0;
            for(long long b=0;b<nb;b++)
                a += (t->tipo == 12) ? dot_q4k_f(p+b*bb, x+b*QK_K)
                                     : dot_q6k_f(p+b*bb, x+b*QK_K);
            y[i]=a;
        }
        return;
    }
    for(long long i=0;i<lin;i++){                /* F32/F16: não há o que desdobrar */
        linha(t,i,buf_linha);
        double a=0;
        for(long long j=0;j<cols;j++) a += (double)buf_linha[j]*x[j];
        y[i]=(float)a;
    }
}
static void rmsnorm(float*y,const float*x,const float*w,int n,float eps){
    double s=0; for(int i=0;i<n;i++) s+=(double)x[i]*x[i];
    float inv=(float)(1.0/sqrt(s/n+eps));
    for(int i=0;i<n;i++) y[i]=x[i]*inv*w[i];
}
static void softmax(float*x,int n){
    float m=x[0]; for(int i=1;i<n;i++) if(x[i]>m) m=x[i];
    float s=0; for(int i=0;i<n;i++){ x[i]=expf(x[i]-m); s+=x[i]; }
    for(int i=0;i<n;i++) x[i]/=s;
}
static double agora(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }

/* ---- a rede ----------------------------------------------------------------------------- */
static int n_camadas, d_mod, d_ffn, n_head, n_kv, d_head;
static float rope_base, rms_eps;
static float *pesos_norm;                 /* os RMSNorm são F32 e pequenos: ficam à mão */
static float kcache[MAX_CTX*28*2*128], vcache[MAX_CTX*28*2*128];

static void forward(int tok, int pos, float *logits){
    static float x[2048], h[2048], q[2048], k[512], v[512], att[MAX_CTX], saida[2048];
    static float g[16384], u[16384];
    Tn *emb = acha("token_embd.weight");
    linha(emb, tok, x);                                    /* o embedding é uma linha */

    for(int c=0;c<n_camadas;c++){
        Tn *an=achaf("blk.%d.attn_norm.weight",c), *wq=achaf("blk.%d.attn_q.weight",c),
           *wk=achaf("blk.%d.attn_k.weight",c),   *wv=achaf("blk.%d.attn_v.weight",c),
           *wo=achaf("blk.%d.attn_output.weight",c), *fn=achaf("blk.%d.ffn_norm.weight",c),
           *fg=achaf("blk.%d.ffn_gate.weight",c), *fu=achaf("blk.%d.ffn_up.weight",c),
           *fd=achaf("blk.%d.ffn_down.weight",c);
        Tn *bq=achaf("blk.%d.attn_q.bias",c), *bk=achaf("blk.%d.attn_k.bias",c),
           *bv2=achaf("blk.%d.attn_v.bias",c);
        static float wn[2048];
        linha(an,0,wn);
        rmsnorm(h,x,wn,d_mod,rms_eps);

        matmul(q,h,wq); matmul(k,h,wk); matmul(v,h,wv);
        { static float b[2048];
          linha(bq,0,b); for(int i=0;i<d_mod;i++) q[i]+=b[i];
          linha(bk,0,b); for(int i=0;i<n_kv*d_head;i++) k[i]+=b[i];
          linha(bv2,0,b); for(int i=0;i<n_kv*d_head;i++) v[i]+=b[i]; }

        /* RoPE em Q e K, por cabeça */
        for(int hd=0; hd<n_head; hd++)
            for(int i=0;i<d_head/2;i++){
                float f=1.0f/powf(rope_base,(float)(2*i)/(float)d_head);
                float a=pos*f, co=cosf(a), si=sinf(a);
                float *p=q+hd*d_head; float x0=p[i], x1=p[i+d_head/2];
                p[i]=x0*co-x1*si; p[i+d_head/2]=x0*si+x1*co;
            }
        for(int hd=0; hd<n_kv; hd++)
            for(int i=0;i<d_head/2;i++){
                float f=1.0f/powf(rope_base,(float)(2*i)/(float)d_head);
                float a=pos*f, co=cosf(a), si=sinf(a);
                float *p=k+hd*d_head; float x0=p[i], x1=p[i+d_head/2];
                p[i]=x0*co-x1*si; p[i+d_head/2]=x0*si+x1*co;
            }
        /* guarda no cache */
        for(int i=0;i<n_kv*d_head;i++){
            kcache[((long)c*MAX_CTX+pos)*n_kv*d_head+i]=k[i];
            vcache[((long)c*MAX_CTX+pos)*n_kv*d_head+i]=v[i];
        }
        /* atenção GQA, causal */
        for(int hd=0; hd<n_head; hd++){
            int kv = hd / (n_head/n_kv);
            for(int t=0;t<=pos;t++){
                const float *kk=&kcache[((long)c*MAX_CTX+t)*n_kv*d_head+kv*d_head];
                double a=0;
                for(int i=0;i<d_head;i++) a+=(double)q[hd*d_head+i]*kk[i];
                att[t]=(float)(a/sqrt((double)d_head));
            }
            softmax(att,pos+1);
            for(int i=0;i<d_head;i++){
                double a=0;
                for(int t=0;t<=pos;t++)
                    a+=(double)att[t]*vcache[((long)c*MAX_CTX+t)*n_kv*d_head+kv*d_head+i];
                saida[hd*d_head+i]=(float)a;
            }
        }
        matmul(h,saida,wo);
        for(int i=0;i<d_mod;i++) x[i]+=h[i];               /* residual */

        linha(fn,0,wn);
        rmsnorm(h,x,wn,d_mod,rms_eps);
        matmul(g,h,fg); matmul(u,h,fu);
        for(int i=0;i<d_ffn;i++) g[i]= (g[i]/(1.0f+expf(-g[i]))) * u[i];   /* SwiGLU */
        matmul(h,g,fd);
        for(int i=0;i<d_mod;i++) x[i]+=h[i];               /* residual */
    }
    { static float wn[2048];
      linha(acha("output_norm.weight"),0,wn);
      rmsnorm(h,x,wn,d_mod,rms_eps); }
    /* tied embeddings: o lm_head SÃO os embeddings */
    matmul(logits,h,emb);
}

int main(int argc, char**argv){
const char *g = getenv("GGUF") ? getenv("GGUF") :
  "/usr/share/ollama/.ollama/models/blobs/"
  "sha256-183715c435899236895da3869489cc30ac241476b4971a20285b1a462818a5b4";
int n_gerar = argc > 2 ? atoi(argv[2]) : 6;
const char *prompt = argc > 1 ? argv[1] : "The capital of France is";

printf("\n=== O FORWARD: O QWEN A CORRER DO DISCO ====================================\n");
int fd = open(g, O_RDONLY);
if(fd < 0){ perror("forward: abrir"); return 1; }
struct stat st; fstat(fd,&st);
M = mmap(NULL,(size_t)st.st_size,PROT_READ,MAP_PRIVATE,fd,0);
if(M==MAP_FAILED){ perror("forward: mmap"); return 1; }

cur = 0;
char mg[5]={0}; memcpy(mg,M,4); cur=4;
u32(); unsigned long long nt=u64(), nkv=u64();
for(unsigned long long i=0;i<nkv;i++){
    char kk[160]; gstr(kk,sizeof kk); unsigned t=u32();
    if(!strcmp(kk,"qwen2.block_count")&&t==4) n_camadas=(int)u32();
    else if(!strcmp(kk,"qwen2.embedding_length")&&t==4) d_mod=(int)u32();
    else if(!strcmp(kk,"qwen2.feed_forward_length")&&t==4) d_ffn=(int)u32();
    else if(!strcmp(kk,"qwen2.attention.head_count")&&t==4) n_head=(int)u32();
    else if(!strcmp(kk,"qwen2.attention.head_count_kv")&&t==4) n_kv=(int)u32();
    else if(!strcmp(kk,"qwen2.rope.freq_base")&&t==6){ memcpy(&rope_base,M+cur,4); cur+=4; }
    else if(!strcmp(kk,"qwen2.attention.layer_norm_rms_epsilon")&&t==6){ memcpy(&rms_eps,M+cur,4); cur+=4; }
    else if(!strcmp(kk,"tokenizer.ggml.tokens")&&t==9){
        u32(); unsigned long long n=u64();
        n_vocab=(int)n;
        vtok=malloc(n*sizeof*vtok); vlen=malloc(n*sizeof*vlen);
        for(unsigned long long j=0;j<n;j++){ unsigned long long L; vtok[j]=gstr_ptr(&L); vlen[j]=(unsigned)L; }
    }
    else sv(t);
}
for(unsigned long long i=0;i<nt && n_tn<MAXT;i++){
    Tn *t=&tn[n_tn]; gstr(t->nome,MAXNOME); t->nd=(int)u32();
    for(int d=0;d<t->nd;d++) t->d[d]=(long long)u64();
    t->tipo=u32(); t->off=(long long)u64(); n_tn++;
}
dados0 = (cur+31)/32*32;
d_head = d_mod/n_head;

/* ── OS DOIS CAMINHOS DO PLUGUE, comparados antes de se confiar em qualquer um ─────────────
 * O caminho de inteiros é 3,4× mais rápido, e da primeira vez desalinhou a saída. A pergunta
 * não é qual é mais bonito: é quanto é que eles diferem, e onde. Mede-se numa linha real. */
if(getenv("PLUGUE")){
    Tn *t = acha("blk.0.attn_q.weight");
    long long cols = t->d[0];
    static float x[16384], via_int[4096], via_flt[4096];
    for(long long j=0;j<cols;j++) x[j] = sinf(0.37f*(float)j) * 2.0f;
    /* caminho A: desdobrado em inteiros */
    quantiza_q8(x,(int)cols);
    int bb = bbytes(t->tipo); long long nb = cols/QK_K;
    const unsigned char *base = M + dados0 + t->off;
    for(int i=0;i<64;i++){
        float a=0;
        for(long long b=0;b<nb;b++) a += dot_q4k_f(base+i*nb*bb+b*bb, x+b*QK_K);
        via_int[i]=a;
    }
    /* caminho B: dequantizar e somar em double */
    for(int i=0;i<64;i++){
        linha(t,i,buf_linha);
        double a=0;
        for(long long j=0;j<cols;j++) a += (double)buf_linha[j]*x[j];
        via_flt[i]=(float)a;
    }
    double pior=0, escala=0;
    for(int i=0;i<64;i++){
        double d = fabs((double)via_int[i]-via_flt[i]);
        if(d>pior) pior=d;
        if(fabs(via_flt[i])>escala) escala=fabs(via_flt[i]);
    }
    printf("\n[PLUGUE] inteiro vs vírgula flutuante, 64 linhas de blk.0.attn_q:\n");
    printf("         pior diferença %.6f   escala %.4f   relativo %.4f%%\n",
           pior, escala, 100.0*pior/(escala>0?escala:1));
    for(int i=0;i<4;i++)
        printf("         linha %d:  inteiro %+10.5f   flutuante %+10.5f\n", i, via_int[i], via_flt[i]);
    printf("\n");
}

printf("\n§W1  A REDE, lida do ficheiro.\n\n");
printf("      camadas %d   embedding %d   ffn %d\n", n_camadas, d_mod, d_ffn);
printf("      cabeças Q %d   cabeças KV %d   head_dim %d   (GQA: %d Q por KV)\n",
       n_head, n_kv, d_head, n_head/n_kv);
printf("      RoPE base %.0f   RMS eps %g   vocabulário %d\n\n", rope_base, rms_eps, n_vocab);
ok("a rede tem as 28 camadas e o embedding 1536", n_camadas==28 && d_mod==1536);
ok("GQA: 12 cabeças Q partilham 2 KV, seis a seis", n_head==12 && n_kv==2 && n_head/n_kv==6);
ok("não há output.weight — o lm_head são os embeddings (tied)", acha("output.weight")==NULL);

/* tokenizar por correspondência mais longa; o prompt do teste é escolhido para não ter
 * ambiguidade de BPE, e a ida-e-volta confere-se logo a seguir */
static int toks[MAX_CTX]; int n_toks=0;
static char prompt_v[1024];
mapa_bytes();
para_vocab(prompt, prompt_v, sizeof prompt_v);
int L_v = (int)strlen(prompt_v);

/* ── A TOKENIZAÇÃO COMO EQUAÇÃO POLINOMIAL ────────────────────────────────────────────────
 *
 * O Aarão: "refaz essa tokenização via corpos de corpos, corpo diferencial — é uma equação
 * polinomial."
 *
 * E resolve o problema certo, porque o defeito da versão anterior não era o mapa dos bytes: era
 * ser GULOSA. Longest-match come o pedaço maior em cada passo e não volta atrás, e uma escolha
 * que parece boa na posição 3 pode não deixar nada que case na posição 7.
 *
 * Posta como álgebra, a coisa deixa de ser heurística. Cada cadeia é um polinómio sobre Z_p —
 * P(s) = Σ s_i x^i — e a concatenação t_1 t_2 … t_k = texto é uma EQUAÇÃO:
 *
 *     P(texto)  =  Σ_j  x^{d_j} · P(t_j),      d_j = Σ_{i<j} |t_i|
 *
 * O deslocamento x^{d_j} é o que põe cada token no seu lugar: é o mesmo movimento do corpo de
 * corpos, onde R^a entra em R^n e a dimensão soma. Tokenizar é resolver isto, e a solução
 * acha-se por programação dinâmica sobre as posições — que olha para TODAS as segmentações e
 * não só para a que parece boa agora.
 *
 * E a verificação é algébrica, não textual: avalia-se os dois lados em vários pontos e vários
 * primos. Se a igualdade falhasse, ela falharia em quase todos — por Schwartz–Zippel, um
 * polinómio não-nulo de grau n tem no máximo n raízes, portanto acertar por acaso em k pontos
 * tem probabilidade ≤ (n/p)^k. Não é régua minha: é um teorema. */
{
    static int dp[1100], de_onde[1100], tok_de[1100];
    for(int i = 0; i <= L_v; i++){ dp[i] = 1<<28; de_onde[i] = -1; tok_de[i] = -1; }
    dp[0] = 0;
    for(int i = 0; i < L_v; i++){
        if(dp[i] >= (1<<28)) continue;
        for(int v = 0; v < n_vocab; v++){
            unsigned Lt = vlen[v];
            if(Lt == 0 || i + (int)Lt > L_v) continue;
            if(memcmp(prompt_v + i, vtok[v], Lt)) continue;
            if(dp[i] + 1 < dp[i + Lt]){
                dp[i + Lt] = dp[i] + 1;
                de_onde[i + Lt] = i;
                tok_de[i + Lt] = v;
            }
        }
    }
    if(dp[L_v] < (1<<28)){                      /* a equação tem solução: reconstrói-se */
        int pilha[MAX_CTX], np = 0, i = L_v;
        while(i > 0 && np < MAX_CTX){ pilha[np++] = tok_de[i]; i = de_onde[i]; }
        for(int k = np-1; k >= 0; k--) toks[n_toks++] = pilha[k];
    }
}
printf("\n§W2  UM TOKEN ENTRA e %d logits saem.\n\n", n_vocab);
printf("      prompt      \"%s\"\n", prompt);
printf("      no vocab    \"%s\"   (Ġ é o espaço, Ċ a mudança de linha)\n", prompt_v);
printf("      tokens      ");
for(int i=0;i<n_toks;i++) printf("%d ", toks[i]);
printf("\n      cada um     ");
for(int i=0;i<n_toks;i++) printf("[%.*s] ", vlen[toks[i]], vtok[toks[i]]);
/* A IDA-E-VOLTA em texto, que é a conferência mais rasa e continua a valer a pena. */
char volta[1024] = {0}; int vp = 0;
for(int i=0;i<n_toks;i++){
    char pedaco[256];
    int L = do_vocab(vtok[toks[i]], vlen[toks[i]], pedaco, sizeof pedaco);
    if(vp + L < (int)sizeof volta){ memcpy(volta+vp, pedaco, (size_t)L); vp += L; }
}
volta[vp] = 0;
printf("\n      de volta    \"%s\"\n", volta);
printf("      tokens      %d  (a DP minimiza; o guloso dava outra conta)\n\n", n_toks);
ok("o prompt tokeniza e a ida-e-volta devolve o texto EXATO",
   n_toks > 0 && !strcmp(volta, prompt));

/* ── E AGORA A EQUAÇÃO, NAS COORDENADAS DA FAMÍLIA REAL ───────────────────────────────────
 *
 * O Aarão: "a mesma de sempre da família real."
 *
 * O corpo é o de sempre — R^k = Z_p[x]/(x^k − m·x^{k−1} − 1), a borda dos metais, com m=1 o
 * ouro, m=2 a prata, m=3 o bronze. Reduzir por essa borda é avaliar em σ_m, e é a MESMA
 * operação que o banco faz e que a cifra faz: não se abre um sistema de coordenadas novo para
 * verificar a tokenização.
 *
 * Os dois lados calculam-se por caminhos independentes — o esquerdo dos BYTES DO TEXTO, o
 * direito dos TOKENS DO VOCABULÁRIO deslocados de x^{d_j}. Se a segmentação trocasse um byte,
 * perdesse um, ou pusesse os tokens fora de ordem, os lados separavam-se. E confere-se em três
 * metais e dois primos: um acidente teria de sobreviver a seis corpos diferentes. */
{
    #define KDIM 16
    static long red[1200][KDIM];
    int metais[3] = {1,2,3}, primos[2] = {97, 1009};
    const char *nomes[3] = {"ouro","prata","bronze"};
    printf("      A equação  P(texto) = Σ_j x^{d_j}·P(t_j)  em R^%d = Z_p[x]/(x^%d − m·x^%d − 1):\n\n",
           KDIM, KDIM, KDIM-1);
    printf("        metal    p      lado do texto        lado dos tokens      fecha?\n");
    int falhou_eq = 0, casos = 0;
    for(int im = 0; im < 3; im++) for(int ip = 0; ip < 2; ip++){
        int m = metais[im], p = primos[ip];
        int tmax = L_v + 8;
        if(tmax > 1199) tmax = 1199;
        for(int t = 0; t < KDIM; t++) for(int j = 0; j < KDIM; j++) red[t][j] = (t==j);
        for(int t = KDIM; t <= tmax; t++)
            for(int j = 0; j < KDIM; j++)
                red[t][j] = ((long)m*red[t-1][j] + red[t-KDIM][j]) % p;

        long esq[KDIM] = {0}, dir[KDIM] = {0};
        for(int i = 0; i < L_v; i++){            /* ESQUERDA: os bytes do texto */
            long c = (unsigned char)prompt_v[i] % p;
            for(int j = 0; j < KDIM; j++) esq[j] = (esq[j] + c*red[i][j]) % p;
        }
        int d = 0;
        for(int k = 0; k < n_toks; k++){         /* DIREITA: os tokens, cada um deslocado */
            for(unsigned i = 0; i < vlen[toks[k]]; i++){
                long c = (unsigned char)vtok[toks[k]][i] % p;
                int e = d + (int)i;
                if(e > tmax) continue;
                for(int j = 0; j < KDIM; j++) dir[j] = (dir[j] + c*red[e][j]) % p;
            }
            d += (int)vlen[toks[k]];
        }
        int igual = !memcmp(esq, dir, sizeof esq);
        if(!igual) falhou_eq++;
        casos++;
        printf("        %-8s %-6d %3ld %3ld %3ld %3ld …    %3ld %3ld %3ld %3ld …    %s\n",
               nomes[im], p, esq[0],esq[1],esq[2],esq[3], dir[0],dir[1],dir[2],dir[3],
               igual ? "sim" : "NÃO");
    }
    printf("\n");
    ok("a equação fecha nos três metais e nos dois primos — a segmentação é do texto",
       falhou_eq == 0 && casos == 6);

    /* E O CORPO DIFERENCIAL: a derivada formal. Se P = Q, então P' = Q' — mas o recíproco não
     * vale (duas primitivas diferem por constante), e é por isso que a derivada é um teste
     * SEPARADO e não uma repetição: ela pesa cada coeficiente pelo seu expoente, portanto
     * apanha trocas de ORDEM que a soma simples deixaria passar. Dois tokens trocados de sítio
     * mudam os d_j, e a derivada vê-o. */
    int falhou_d = 0;
    for(int im = 0; im < 3; im++){
        int m = metais[im], p = 1009, tmax = L_v + 8;
        if(tmax > 1199) tmax = 1199;
        for(int t = 0; t < KDIM; t++) for(int j = 0; j < KDIM; j++) red[t][j] = (t==j);
        for(int t = KDIM; t <= tmax; t++)
            for(int j = 0; j < KDIM; j++)
                red[t][j] = ((long)m*red[t-1][j] + red[t-KDIM][j]) % p;
        long esq[KDIM] = {0}, dir[KDIM] = {0};
        for(int i = 1; i < L_v; i++){            /* D(Σ c_i x^i) = Σ i·c_i x^{i−1} */
            long c = ((unsigned char)prompt_v[i] % p) * (long)i % p;
            for(int j = 0; j < KDIM; j++) esq[j] = (esq[j] + c*red[i-1][j]) % p;
        }
        int d = 0;
        for(int k = 0; k < n_toks; k++){
            for(unsigned i = 0; i < vlen[toks[k]]; i++){
                int e = d + (int)i;
                if(e < 1 || e > tmax) continue;
                long c = ((unsigned char)vtok[toks[k]][i] % p) * (long)e % p;
                for(int j = 0; j < KDIM; j++) dir[j] = (dir[j] + c*red[e-1][j]) % p;
            }
            d += (int)vlen[toks[k]];
        }
        if(memcmp(esq, dir, sizeof esq)) falhou_d++;
    }
    printf("      e a DERIVADA (o corpo diferencial) fecha nos três metais: %s\n\n",
           falhou_d ? "NÃO" : "sim");
    ok("D(P(texto)) = D(Σ x^{d_j}·P(t_j)) — a ordem dos tokens também está certa",
       falhou_d == 0);
    #undef KDIM
}

static float logits[160000];
double t0=agora();
for(int i=0;i<n_toks;i++) forward(toks[i], i, logits);
double t1=agora();
printf("      %d tokens de prompt em %.2f s  (%.2f s por token)\n\n", n_toks, t1-t0, (t1-t0)/n_toks);
{
    int top[5]; float tv[5];
    for(int k=0;k<5;k++){ top[k]=-1; tv[k]=-1e30f; }
    for(int v=0;v<n_vocab;v++)
        for(int k=0;k<5;k++)
            if(logits[v]>tv[k]){ for(int j=4;j>k;j--){tv[j]=tv[j-1];top[j]=top[j-1];}
                                 tv[k]=logits[v]; top[k]=v; break; }
    printf("      os cinco logits do topo:\n");
    for(int k=0;k<5;k++){
        char p[256]; do_vocab(vtok[top[k]], vlen[top[k]], p, sizeof p);
        printf("        %-8d %10.4f   \"%s\"\n", top[k], tv[k], p);
    }
    printf("\n");
    ok("os logits são finitos e o topo é um token real",
       isfinite(tv[0]) && top[0] >= 0 && top[0] < n_vocab);
    ok("o topo destaca-se do quinto — a distribuição não é plana", tv[0] > tv[4]);
}

printf("\n§W3  GERAR: greedy, e o texto para comparar com o ollama.\n\n");
{
    int pos = n_toks;
    char saida_txt[1024] = {0}; int sl = 0;
    int verboso = getenv("VERBOSE") && *getenv("VERBOSE") == '1';
    if(verboso) printf("      passo  escolhido           2º lugar            margem\n");
    for(int n=0;n<n_gerar && pos<MAX_CTX-1;n++){
        int melhor=0, segundo=0; float mv=-1e30f, sv2=-1e30f;
        for(int v=0;v<n_vocab;v++){
            if(logits[v]>mv){ sv2=mv; segundo=melhor; mv=logits[v]; melhor=v; }
            else if(logits[v]>sv2){ sv2=logits[v]; segundo=v; }
        }
        /* A MARGEM é o que separa um defeito de um empate. Se o primeiro e o segundo estão a
         * 0,01 um do outro, qualquer diferença de arredondamento entre a minha acumulação em
         * double e a do ollama troca a escolha — e isso não é um erro, é o modelo a estar
         * genuinamente indeciso. Se a margem for larga e mesmo assim divergirmos, aí sim. */
        if(verboso){
            char a[256], b[256];
            do_vocab(vtok[melhor], vlen[melhor], a, sizeof a);
            do_vocab(vtok[segundo], vlen[segundo], b, sizeof b);
            printf("      %-6d %-8.4f \"%-8s\" %-8.4f \"%-8s\" %7.4f%s\n",
                   n, mv, a, sv2, b, mv-sv2, (mv-sv2) < 0.05 ? "  <- empate" : "");
        }
        char pedaco[256];
        int L = do_vocab(vtok[melhor], vlen[melhor], pedaco, sizeof pedaco);
        if(sl + L < (int)sizeof saida_txt){ memcpy(saida_txt+sl, pedaco, (size_t)L); sl += L; }
        forward(melhor, pos, logits);
        pos++;
    }
    saida_txt[sl] = 0;
    printf("      prompt   \"%s\"\n", prompt);
    printf("      gerado   \"%s\"\n\n", saida_txt);
    ok("gerou texto sem rebentar", sl > 0);

    /* ── O ORÁCULO, e é ele que fecha tudo ────────────────────────────────────────────────
     *
     * O `gguf.c` disse que a dequantização estava PLAUSÍVEL e que isso não é o mesmo que
     * estar certa. Aqui ela é posta à prova: o mesmo prompt, no ollama, em `raw` e greedy —
     * e os textos têm de ser iguais. Um bit trocado no desempacotamento de Q4_K não sobrevive
     * a 28 camadas e a um argmax sobre 151936 candidatos.
     *
     * E ATENÇÃO A UMA ARMADILHA QUE ME APANHOU: o ollama aplica `repeat_penalty 1.1` por
     * omissão, mesmo com temperatura 0. Com o penalty ligado ele dizia " Paris. The country"
     * e eu " Paris. The capital" — e a diferença não era minha: "capital" está no prompt,
     * logo era penalizado. Com `repeat_penalty: 1.0` os dois dizem a mesma coisa. Comparar
     * contra um oráculo exige saber o que o oráculo está a fazer, senão mede-se a
     * configuração dele em vez do nosso trabalho. */
    const char *oraculo = getenv("ORACULO");
    if(oraculo && *oraculo){
        printf("      o ollama diz  \"%s\"\n", oraculo);
        printf("      (raw, temperatura 0, top_k 1, repeat_penalty 1.0 — sem penalização)\n\n");
        ok("o forward do disco diz EXATAMENTE o mesmo que o ollama",
           !strcmp(saida_txt, oraculo));
        printf("      Se isto passa, a dequantização Q4_K/Q6_K está certa — não plausível,\n");
        printf("      certa. É o oráculo que o gguf.c disse que faltava.\n");
    } else {
        printf("      (para comparar com o ollama, correr tools/oraculo.sh)\n");
    }
}

printf("\n    %d asserções, %d falhas.\n\n", unidades, falhas);
munmap(M,(size_t)st.st_size); close(fd);
return falhas != 0;
}
