/* tiffany.c — a assistente pela RECONSTRUÇÃO DUAL: três batidas, ℱ³=ℱ⁻¹.
 *
 * O mecanismo é PURO e ANALÓGICO — nada de análise de texto (sem palavras, listas ou regras);
 * tudo é convolução e transformada, reproduzível no micro que vai à fábrica:
 *
 *   a fala CAI  = a CONVOLUÇÃO fala⊛corpus: o mínimo de D=|fala−janela|² — o banco de
 *                 correlacionadores + o winner-take-all (o gato, o negro, a análise);
 *   a resposta  = as TRÊS BATIDAS ℱ³=ℱ⁻¹ do trecho onde a fala caiu (o esquilo, o branco,
 *                 a síntese). ℱ (normalizada por 1/√N) tem período 4: ℱ⁴=id — a mesma conta
 *                 do esquilo (G⁴=I). A resposta volta EXATA (Parseval), resíduo 0.
 *
 * O sistema é um ESPELHO REVERSÍVEL: a resposta É o corpus refletido. A voz, a personalidade e
 * a cobertura vêm SÓ DO CORPUS — molda-se o corpus, nunca o mecanismo.
 *
 *   cc -O2 -std=c99 tiffany.c -o tiffany
 *   ./tiffany [corpus.txt] ["a fala"]     (sem a fala: modo conversa; corpus: voz.txt por padrão)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long i64;
static const i64 P = 40961, GR = 3;                 /* primo com 4096 | P−1 (tres_reconstroi.c) */
static i64 md(i64 x){ x %= P; return x < 0 ? x + P : x; }
static i64 mul(i64 a, i64 b){ return md(a*b); }
static i64 pot(i64 b, i64 e){ i64 r = 1; b = md(b); while(e>0){ if(e&1) r = mul(r,b); b = mul(b,b); e >>= 1; } return r; }
static i64 inv(i64 a){ return pot(a, P-2); }

#define N 256                                        /* o bloco da resposta; √N=16 (período 4 exato) */
static i64 W, RN, wp[N];                             /* raiz N-ésima, 1/√N, e as potências de W */
static void Fa(const i64 *x, i64 *X){                /* ℱ: a transformada normalizada (ℱ⁴=id) */
    for(int k=0;k<N;k++){
        i64 acc = 0;
        for(int j=0;j<N;j++) acc = md(acc + mul(x[j], wp[(int)((i64)j*k % N)]));
        X[k] = mul(acc, RN);
    }
}

/* o corpus, e a QUEDA — a fala cai pela CONVOLUÇÃO (o mínimo de D, o winner-take-all). */
static long NL; static char *B;
static long cai(const unsigned char *fala, int fl){
    long Ef = 0; for(int i=0;i<fl;i++) Ef += (long)fala[i]*fala[i];
    long best = -1; double bestD = 1e300;
    for(long d=0; d+fl<=NL; d++){
        long c=0, Ej=0;
        for(int i=0;i<fl;i++){ long b=(unsigned char)B[d+i]; c += (long)fala[i]*b; Ej += b*b; }
        double D = (double)Ef + Ej - 2.0*c;          /* D = |fala − janela|² (D=0 no casamento) */
        if(D < bestD){ bestD = D; best = d; }
    }
    return best;
}

/* uma resposta: a fala cai (ℱ, a convolução), a resposta reconstrói em três batidas (ℱ³=ℱ⁻¹). */
static int responde(const char *fala){
    int fl = (int)strlen(fala);
    long q = cai((const unsigned char*)fala, fl);
    long p0 = (q<0) ? 0 : q + fl;                     /* a resposta começa após onde a fala caiu */
    if(p0+N > NL) p0 = NL-N;
    if(p0 < 0) p0 = 0;
    i64 bloco[N]; for(int i=0;i<N;i++) bloco[i] = (unsigned char)B[p0+i];
    i64 X[N], t1[N], t2[N], t3[N];
    Fa(bloco, X); Fa(X, t1); Fa(t1, t2); Fa(t2, t3); /* ℱ (análise) → ℱ³ (a volta, o esquilo) */
    int errR = 0; for(int i=0;i<N;i++) if(t3[i]!=bloco[i]) errR++;
    printf("tiffany: ");
    int mostra = N < 420 ? N : 420;
    for(int i=0;i<mostra;i++){ int ch=(int)t3[i]; putchar(ch>=32 && ch<256 ? ch : (ch=='\0'?' ':ch)); }
    printf(" …\n");
    return errR;
}

int main(int argc, char **argv){
    const char *path = argc>1 ? argv[1] : "voz.txt";   /* o corpus É a voz da assistente */
    long SZ = 1L<<20;

    /* carrega o corpus corrido (um sinal só) — aceita TEXTO PURO ou o formato Tatoeba */
    FILE *f = fopen(path,"rb"); if(!f){ fprintf(stderr,"não abri %s\n",path); return 2; }
    fseek(f,0,SEEK_END); long M=ftell(f); fseek(f,0,SEEK_SET);
    char *raw = malloc(M); if(fread(raw,1,M,f)!=(size_t)M) return 2; fclose(f);
    B = malloc(SZ); NL = 0;
    for(long i=0;i<M && NL<SZ;){
        long ls=i; while(ls<M && raw[ls]!='\n') ls++;                 /* o fim da linha            */
        long j=i; int tabs=0; for(long k=i;k<ls;k++) if(raw[k]=='\t') tabs++;
        if(tabs>=2){ int t=0; while(j<ls && t<2){ if(raw[j]=='\t') t++; j++; } }  /* Tatoeba: pula id⇥lang⇥ */
        while(j<ls && NL<SZ) B[NL++]=raw[j++];                        /* o texto da linha          */
        i = (ls<M) ? ls+1 : ls;
        if(NL<SZ) B[NL++]=' ';                                        /* frases juntas, um só sinal */
    }
    free(raw);
    if(NL < N+16){ fprintf(stderr,"corpus pequeno demais (%ld bytes; precisa > %d)\n", NL, N); return 2; }

    /* a transformada ℱ: a raiz N-ésima e a normalização 1/√N (√256 = 16) */
    W = pot(GR, (P-1)/N); RN = inv(16);
    wp[0] = 1; for(int t=1;t<N;t++) wp[t] = mul(wp[t-1], W);

    /* o PERÍODO 4: ℱ⁴ = id (a mesma conta do esquilo, G⁴=I) — a medição do mecanismo */
    i64 a[N],b[N],c[N],d[N],e[N];
    for(int i=0;i<N;i++) a[i] = (i*i*37 + i*11 + 5) % P;
    Fa(a,b); Fa(b,c); Fa(c,d); Fa(d,e);
    int err4 = 0; for(int i=0;i<N;i++) if(e[i]!=a[i]) err4++;

    if(argc>2){                                        /* one-shot: uma fala no argumento */
        printf("você: %s\n\n", argv[2]);
        int errR = responde(argv[2]);
        printf("\n[três batidas, ℱ³=ℱ⁻¹ — a fala cai por ℱ (o gato); a resposta reconstrói por ℱ³ (o\n"
               " esquilo), direta e dual. ℱ⁴=id: %d erros; a resposta volta exata (Parseval): %d erros.\n"
               " Resíduo %s. O corpus É a voz; o mecanismo é o espelho reversível — analógico, puro.]\n",
               err4, errR, (err4||errR) ? "≠0" : "0");
    } else {                                           /* o modo conversa: lê do stdin, linha a linha */
        printf("Tiffany — a voz acolhedora e reflexiva (corpus: %s). Escreva o que sente; Ctrl-D encerra.\n", path);
        printf("(o mecanismo é o espelho ℱ³=ℱ⁻¹, resíduo %s; a resposta emana do corpus, não se inventa.)\n\n",
               err4 ? "≠0" : "0");
        char linha[2048];
        printf("você: "); fflush(stdout);
        while(fgets(linha, sizeof linha, stdin)){
            linha[strcspn(linha,"\n")] = 0;
            if(linha[0]){ putchar('\n'); responde(linha); putchar('\n'); }
            printf("você: "); fflush(stdout);
        }
        printf("\ntiffany: Volta sempre que precisar. Fica bem.\n");
    }
    free(B); return 0;
}
