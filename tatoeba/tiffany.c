/* tiffany.c — a assistente pela RECONSTRUÇÃO DUAL: três batidas, ℱ³=ℱ⁻¹.
 *
 * A resposta não é um passeio que tenta até fechar — é DIRETA, dual, em três batidas. A
 * transformada ℱ (normalizada por 1/√N) tem PERÍODO 4: ℱ⁴=id — a mesma conta do esquilo
 * (G⁴=I, o giro de 90°). Logo ℱ³=ℱ⁻¹: três batidas de um lado SÃO a volta, sem inverter
 * nada. Os quatro modos são {id, ℱ, reflexão x[−j], ℱ⁻¹}. Na fala:
 *
 *   ℱ   (a IDA) — a fala CAI (o gato, o negro): a convolução fala⊛livro aponta onde ela
 *                 aterra (o mínimo de D, o atrator p) — a análise;
 *   ℱ²  ......... o ESPELHO vira o lado (o antípoda, a reflexão): do que a fala é ao que o
 *                 corpus responde;
 *   ℱ³=ℱ⁻¹ ...... a resposta se RECONSTRÓI (o esquilo, o branco): o caminho do corpus que
 *                 atravessa a fala, emanado DE UMA VEZ — a síntese.
 *
 * Uma ida (ℱ) e três batidas (ℱ³) fecham o ciclo (ℱ⁴=id): a resposta volta EXATA, não
 * parecida — a mão que segura é Parseval (a norma se conserva, nenhum ângulo muda).
 *
 * O sistema é um ESPELHO REVERSÍVEL, não um espírito a decifrar: a resposta É o corpus
 * refletido, não há como ser diferente. Personalidade e respostas específicas moldam-se NO
 * CORPUS (o lastro está fora do dispositivo — a segurança), não no mecanismo, que é fiel por
 * construção. As três batidas são o mecanismo dual e direto que colhe o caminho, no lugar do
 * passeio-tentativa. Reproduzível no analógico: ℱ é a transformada; a convolução é o gato; a
 * reflexão é o espelho; ℱ³ é o esquilo.
 *
 *   cc -O2 -std=c99 tiffany.c -o tiffany
 *   ./tiffany [corpus.txt] "a fala" [potência2] — o corpus é a voz (voz.txt por padrão)
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

/* o corpus, e o CASAMENTO de uma palavra (a convolução): o melhor ponto, o D médio por byte, e
 * quantas ocorrências ~exatas há (occ) — a raridade, que distingue a palavra-chave da comum. */
static long NL; static char *B;
static long casa(const unsigned char *w, int wl, double *outDmed, int *outOcc){
    long Ef = 0; for(int i=0;i<wl;i++) Ef += (long)w[i]*w[i];
    long best = -1; double bestD = 1e300;
    for(long d=0; d+wl<=NL; d++){
        long c=0, Ej=0;
        for(int i=0;i<wl;i++){ long b=(unsigned char)B[d+i]; c += (long)w[i]*b; Ej += b*b; }
        double D = (double)Ef + Ej - 2.0*c;          /* D = |palavra − janela|², o negro (D=0 no casamento) */
        if(D < bestD){ bestD = D; best = d; }
    }
    int occ = 0; double thr = bestD + 8.0*wl;         /* conta os pontos ~tão bons quanto o melhor */
    for(long d=0; d+wl<=NL; d++){
        long c=0, Ej=0;
        for(int i=0;i<wl;i++){ long b=(unsigned char)B[d+i]; c += (long)w[i]*b; Ej += b*b; }
        if((double)Ef + Ej - 2.0*c <= thr) occ++;
    }
    if(outDmed) *outDmed = (best<0) ? 1e300 : bestD/(double)wl;
    if(outOcc)  *outOcc  = occ;
    return best;
}

/* a queda por PALAVRA-CHAVE: uma fala tem muitas palavras, mas o TEMA está nas de CONTEÚDO (medo,
 * culpa, sozinho), não nas funcionais (sinto, estou, muita), que casam em todo lugar. Pulam-se as
 * funcionais; entre as de conteúdo, o melhor casamento (empate: a mais longa) — winner-take-all. */
static int is_sep(char c){
    return c==' '||c=='\t'||c=='\n'||c=='.'||c==','||c==';'||c==':'||c=='!'||c=='?'||c=='"'||c=='\'';
}
static const char *STOP[] = {
    "estou","esta","está","estar","sou","tenho","tem","quero","queria","preciso","sinto","sente",
    "muito","muita","mais","menos","com","sem","para","por","meu","minha","seu","sua","esse","essa",
    "isso","este","você","voce","gente","tudo","nada","aqui","agora","hoje","ainda","também","tambem",
    "porque","quando","onde","como","fazer","dela","dele","uma","que","não","nao","sim","mim","dos",
    "das","pelo","pela","numa","num","cada","toda","todo","tão","tao", 0
};
static int eqi(const char *a, const char *b, int len){
    for(int i=0;i<len;i++){ char x=a[i],y=b[i]; if(x>='A'&&x<='Z')x+=32; if(y>='A'&&y<='Z')y+=32; if(x!=y) return 0; }
    return 1;
}
static int is_stop(const char *w, int wl){
    for(int k=0;STOP[k];k++){ int l=(int)strlen(STOP[k]); if(l==wl && eqi(STOP[k],w,wl)) return 1; }
    return 0;
}
static long cai_tema(const char *fala){
    int n=(int)strlen(fala), i=0, bestlen=0;
    long bestpos=-1; double bestD=1e300;
    while(i<n){
        while(i<n && is_sep(fala[i])) i++;
        int s=i; while(i<n && !is_sep(fala[i])) i++;
        int wl=i-s;
        if(wl<4 || is_stop(fala+s,wl)) continue;      /* só as palavras de conteúdo (o tema) */
        double Dmed; int occ; long q=casa((const unsigned char*)(fala+s), wl, &Dmed, &occ);
        if(q<0) continue;
        if(Dmed < bestD-1e-9 || (Dmed < bestD+1e-9 && wl>bestlen)){ bestD=Dmed; bestpos=q+wl; bestlen=wl; }
    }
    if(bestpos<0){ int m=n<80?n:80; double d; int o; long q=casa((const unsigned char*)fala,m,&d,&o); bestpos=(q<0)?0:q+m; }
    while(bestpos<NL && !is_sep(B[bestpos])) bestpos++;   /* completa a palavra do corpus (triste→tristeza) */
    return bestpos;
}

int main(int argc, char **argv){
    const char *path = argc>1 ? argv[1] : "voz.txt";   /* o corpus É a voz da assistente */
    const char *fala = argc>2 ? argv[2] : "estou com medo";
    int lg = argc>3 ? atoi(argv[3]) : 16; long SZ = 1L<<lg;

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

    /* medição — o PERÍODO 4: ℱ⁴ = id (a mesma conta do esquilo, G⁴=I) */
    i64 a[N],b[N],c[N],d[N],e[N];
    for(int i=0;i<N;i++) a[i] = (i*i*37 + i*11 + 5) % P;
    Fa(a,b); Fa(b,c); Fa(c,d); Fa(d,e);
    int err4 = 0; for(int i=0;i<N;i++) if(e[i]!=a[i]) err4++;

    /* ℱ (a ida) — a fala CAI: a convolução (a palavra-chave) aponta o atrator */
    long p0 = cai_tema(fala);
    /* a resposta começa no INÍCIO da frase que contém a palavra-chave (uma frase inteira, limpa) */
    long ini=p0, lim=p0-170;
    while(ini>0 && ini>lim && !(B[ini-1]=='.'||B[ini-1]=='!'||B[ini-1]=='?')) ini--;
    while(ini<NL && (B[ini]==' '||B[ini]=='\n')) ini++;
    if(ini < p0) p0 = ini;
    if(p0+N > NL) p0 = NL-N; if(p0 < 0) p0 = 0;

    /* o bloco do corpus que atravessa a fala — o caminho a reconstruir */
    i64 bloco[N]; for(int i=0;i<N;i++) bloco[i] = (unsigned char)B[p0+i];

    /* as TRÊS BATIDAS: ℱ (análise) → ℱ² (vira o lado) → ℱ³=ℱ⁻¹ (a resposta reconstrói) */
    i64 X[N], t1[N], t2[N], t3[N];
    Fa(bloco, X);                                    /* ℱ  — a fala caiu (a análise, o gato) */
    Fa(X, t1); Fa(t1, t2); Fa(t2, t3);               /* ℱ³ — a volta (o esquilo): ℱ³(ℱ(bloco)) */
    int errR = 0; for(int i=0;i<N;i++) if(t3[i]!=bloco[i]) errR++;   /* == bloco? Parseval, exata */

    /* a resposta: o caminho reconstruído (o modo id, após o ciclo fechar) */
    printf("você: %s\n\ntiffany: ", fala);
    int mostra = N < 420 ? N : 420;
    for(int i=0;i<mostra;i++){ int ch=(int)t3[i]; putchar(ch>=32 && ch<256 ? ch : (ch=='\0'?' ':ch)); }
    printf(" …\n\n[três batidas, ℱ³=ℱ⁻¹ — a fala caiu por ℱ (o gato, o negro), o espelho ℱ² virou\n"
           " o lado, a resposta reconstruiu por ℱ³ (o esquilo, o branco): direta, dual, não passeio.\n"
           " o ciclo fecha (ℱ⁴=id): %d erros; a resposta volta EXATA (Parseval), bloco de %d bytes: %d\n"
           " erros. Resíduo %s. Reproduzível no analógico (a transformada, o gato, o espelho, o esquilo).]\n",
           err4, N, errR, (err4||errR) ? "≠0" : "0");
    free(B); return 0;
}
