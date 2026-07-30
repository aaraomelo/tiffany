/* operador.c — a palavra é OPERADOR, a frase é o PRODUTO ORDENADO, o significado é o POLINÔMIO.
 *
 * A contagem (homogeneo.c) fechou o caso aditivo: E/V=5,76 e a dimensão não compra liberdade — num
 * embedding aditivo em K^d cada par dá d equações e cada palavra d incógnitas, a razão é invariante.
 * A saída é a palavra deixar de ser número e virar operador: matriz d×d, d² graus de liberdade, e a
 * frase o produto ordenado. Então E·d equações (os invariantes) contra V·d² incógnitas, e a condição
 * necessária é d ≥ E/V ≈ 5,76, isto é d ≥ 6.
 *
 * A teoria já tinha cada peça (teoria.tex):
 *
 *  · o SIGNIFICADO é o polinômio. "Todo dado é um polinômio δ(x)=Σdᵢσⁱ; o único discreto é a
 *    dimensão n" (§2). De uma frase-operador X colhe-se o seu polinômio característico — os d
 *    coeficientes, por Newton a partir dos traços tr(X),…,tr(X^d). Ele é invariante por conjugação,
 *    e a conjugação é a rotação: traduzir preserva o polinômio, não a matriz. A norma do caso
 *    escalar era o caso d=1 disso.
 *  · a INVERSA são TRÊS BATIDAS. ℱ⁴=id é a mesma conta de G⁴=I, logo ℱ³=ℱ⁻¹ (§3,
 *    tools/tres_reconstroi.c). O peeling precisa inverter vizinhos; na órbita do esquilo essa
 *    inversa não se constrói — colhe-se, batendo três vezes. Aferido aqui (§O0), resíduo 0.
 *  · o modo 2 é a REFLEXÃO (o ν, virar o lado). É ela que dá a não-comutatividade: potências do gato
 *    comutam entre si, e um operador que comuta colapsa a frase de volta ao saco de palavras
 *    (A^k₁A^k₂=A^{k₁+k₂} ⟹ Σk_en=Σk_pt, o aditivo em ℤ). Semente = C^a·R·C^b: gato e reflexão.
 *
 * A colheita é a mesma de ancora.c, agora por inversão em vez de divisão: se numa equação falta um
 * único operador, ele cai exato —  P·G·S = RHS  ⟹  G = P⁻¹·RHS·S⁻¹.  Sementes quando trava.
 * Λ é inobservável (g_pt ↦ Λg_ptΛ⁻¹ absorve), como no caso escalar; medido em §O3.
 *
 * SEM MEMÓRIA: zero malloc; matrizes em buffers de tamanho fixo (d≤8); o léxico de operadores no
 * disco, slot a slot (a palavra é um nó, e nó é fiação). Estado em RAM O(1).
 *
 *   cc -O2 -std=c99 operador.c -lm -o operador
 *   ./operador pares.tsv [d] [lim] [exp_lambda]
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define MAXD    8
#define NSLOT   (1L<<17)
#define LEXFILE "lexico_op.bin"
#define MAXROD  8
#define MAXCAS  6

static int p = 40013, m = 1, d = 6;                  /* o corpo, o metal, a dimensão do operador   */
static int MODO_FRACO = 0;                           /* colher pelo POLINÔMIO (1) ou pela matriz(0)*/
typedef int32_t M[MAXD*MAXD];                        /* a matriz, linearizada — buffer fixo        */

typedef struct { uint64_t fp; int32_t v[MAXD*MAXD]; } Slot;
static int fdlex;

static int md(long x){ x%=p; return (int)(x<0?x+p:x); }
static void mzero(M A){ for(int i=0;i<d*d;i++) A[i]=0; }
static void mid(M A){ mzero(A); for(int i=0;i<d;i++) A[i*d+i]=1; }
static void mcp(M A, const M B){ for(int i=0;i<d*d;i++) A[i]=B[i]; }
static int  meq(const M A, const M B){ for(int i=0;i<d*d;i++) if(A[i]!=B[i]) return 0; return 1; }
static void mmul(M R, const M A, const M B){         /* R = A·B (ordem importa)                    */
    M T; mzero(T);
    for(int i=0;i<d;i++) for(int k=0;k<d;k++){
        long a = A[i*d+k]; if(!a) continue;
        for(int j=0;j<d;j++) T[i*d+j] = md(T[i*d+j] + a*B[k*d+j]);
    }
    mcp(R,T);
}
static long invp(long k){ long r=1,b=md(k),e=p-2; while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; } return r; }
/* a inversa geral, por eliminação — usada nos operadores colhidos (que não têm órbita curta) */
static int minv(M R, const M A){
    int32_t W[MAXD][2*MAXD];
    for(int i=0;i<d;i++){ for(int j=0;j<d;j++) W[i][j]=A[i*d+j];
                          for(int j=0;j<d;j++) W[i][d+j]=(i==j); }
    for(int c=0;c<d;c++){
        int piv=-1; for(int r2=c;r2<d;r2++) if(W[r2][c]){ piv=r2; break; }
        if(piv<0) return 0;                            /* singular — não invertível                 */
        if(piv!=c) for(int j=0;j<2*d;j++){ int32_t t=W[c][j]; W[c][j]=W[piv][j]; W[piv][j]=t; }
        long ip = invp(W[c][c]);
        for(int j=0;j<2*d;j++) W[c][j] = md((long)W[c][j]*ip);
        for(int r2=0;r2<d;r2++){
            if(r2==c || !W[r2][c]) continue;
            long f = W[r2][c];
            for(int j=0;j<2*d;j++) W[r2][j] = md(W[r2][j] - f*W[c][j]);
        }
    }
    for(int i=0;i<d;i++) for(int j=0;j<d;j++) R[i*d+j]=W[i][d+j];
    return 1;
}
/* --- o SIGNIFICADO: o polinômio característico, colhido dos traços (Newton) --- */
static int mtr(const M A){ long t=0; for(int i=0;i<d;i++) t+=A[i*d+i]; return md(t); }
static void charpoly(const M A, int32_t *e){         /* e[1..d]: os coeficientes                   */
    int32_t pk[MAXD+1];
    M P; mcp(P,A);
    pk[1] = mtr(P);
    for(int k=2;k<=d;k++){ mmul(P,P,A); pk[k] = mtr(P); }
    e[0] = 1;
    for(int k=1;k<=d;k++){
        long acc = 0;
        for(int i=1;i<=k;i++){
            long term = (long)e[k-i]*pk[i] % p;
            acc += (i&1) ? term : -term;
        }
        e[k] = md(acc % p * invp(k));
    }
}
static int poly_eq(const int32_t *a, const int32_t *b){ for(int k=1;k<=d;k++) if(a[k]!=b[k]) return 0; return 1; }

/* --- o gato de dimensão d (a companion de x^d − m x^{d−1} − 1) e a reflexão (o ν) --- */
static void gato_d(M A){
    mzero(A);
    for(int i=0;i<d-1;i++) A[(i+1)*d+i] = 1;         /* o deslocamento                             */
    A[(d-1)*d+(d-1)] = md(m); A[0*d+(d-1)] = 1;      /* a borda: σ^d = mσ^{d−1} + 1                */
}
static void refl_d(M A){                             /* o modo 2: x[−j], virar o lado (R²=I)       */
    mzero(A);
    for(int i=0;i<d;i++) A[i*d+(d-1-i)] = 1;
}
static void giro_d(M A){                             /* o esquilo: G⁴=I, blocos de 90°             */
    mzero(A);
    for(int i=0;i+1<d;i+=2){ A[i*d+(i+1)] = md(-1); A[(i+1)*d+i] = 1; }
    if(d&1) A[(d-1)*d+(d-1)] = 1;
}
/* a COMPANION de um polinômio — a forma canônica: o gato daquele polinômio.
 * "Todo dado é um polinômio" (teoria.tex §2): dado o significado e[1..d], a matriz que o realiza é
 * esta, e é a mesma peça (o gato de dimensão d é a companion de x^d − m x^{d−1} − 1).             */
static void companion(const int32_t *e, M A){
    mzero(A);
    for(int i=0;i<d-1;i++) A[(i+1)*d+i] = 1;
    /* char(A) = x^d − e₁x^{d−1} + e₂x^{d−2} − … : a última coluna leva os coeficientes com sinal */
    for(int k=1;k<=d;k++){
        long c = ((k&1) ? e[k] : -e[k]);
        A[(d-k)*d+(d-1)] = md(c);
    }
}
static void mpow(M R, const M A, long e){
    M B, T; mcp(B,A); mid(R);
    while(e>0){ if(e&1){ mmul(T,R,B); mcp(R,T); } mmul(T,B,B); mcp(B,T); e>>=1; }
}
/* a semente: C^a·R·C^b — gato e reflexão, um grau de liberdade queimado, sempre invertível */
static void semente(uint64_t fp, M S){
    M C, R, T1, T2, Ca, Cb;
    gato_d(C); refl_d(R);
    mpow(Ca, C, (long)(fp % 97) + 1);
    mpow(Cb, C, (long)((fp>>17) % 89) + 1);
    mmul(T1, Ca, R); mmul(T2, T1, Cb);
    mcp(S, T2);
}
static uint64_t marca(const char *s, size_t n, uint64_t sal){
    uint64_t h = 1469598103934665603UL ^ sal;
    for(size_t i=0;i<n;i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
#define SAL_EN 0x1111111111111111UL
#define SAL_PT 0x2222222222222222UL

static int busca(uint64_t fp, M G, long *slot){
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t=0;t<NSLOT;t++){
        Slot s;
        if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot)) != (long)sizeof(Slot)) return -1;
        if(s.fp==0){ *slot=i; return 0; }
        if(s.fp==fp){ for(int k=0;k<d*d;k++) G[k]=s.v[k]; *slot=i; return 1; }
        i = (i+1)%NSLOT;
    }
    return -1;
}
static void grava(long slot, uint64_t fp, const M G){
    Slot s; memset(&s,0,sizeof s); s.fp=fp;
    for(int k=0;k<d*d;k++) s.v[k]=G[k];
    pwrite(fdlex,&s,sizeof s,slot*(long)sizeof(Slot));
}

/* varre um lado: produto ordenado das conhecidas, e as incógnitas (até 3).
 * Devolve em X o produto SÓ se nada faltar; senão X fica sem sentido e nu>0.       */
typedef struct { uint64_t fp; long slot; int lado, mult, pos; } Inc;
static void varre_lado(char *fr, int lado, M X, Inc *u, int *nu){
    mid(X);
    int pos = 0;
    for(char *w=fr; *w; ){
        while(*w==' ') w++;
        char *e=w; while(*e && *e!=' ') e++;
        if(e>w){
            uint64_t fp = marca(w, e-w, lado?SAL_PT:SAL_EN);
            M G; long sl;
            if(busca(fp,G,&sl)==1){ M T; mmul(T,X,G); mcp(X,T); }
            else {
                int achou=0;
                for(int i=0;i<*nu && i<3;i++) if(u[i].fp==fp){ u[i].mult++; achou=1; break; }
                if(!achou){ if(*nu<3){ u[*nu].fp=fp; u[*nu].slot=sl; u[*nu].lado=lado;
                                       u[*nu].mult=1; u[*nu].pos=pos; } (*nu)++; }
            }
            pos++;
        }
        w = e;
    }
}
/* prefixo e sufixo em torno da posição pos (todas as outras palavras já conhecidas) */
static int pre_suf(char *fr, int lado, int pos, M P, M S){
    mid(P); mid(S);
    int i = 0;
    for(char *w=fr; *w; ){
        while(*w==' ') w++;
        char *e=w; while(*e && *e!=' ') e++;
        if(e>w){
            if(i != pos){
                uint64_t fp = marca(w, e-w, lado?SAL_PT:SAL_EN);
                M G; long sl;
                if(busca(fp,G,&sl)!=1) return 0;
                M T;
                if(i<pos){ mmul(T,P,G); mcp(P,T); } else { mmul(T,S,G); mcp(S,T); }
            }
            i++;
        }
        w = e;
    }
    return 1;
}
static char line[8192];

int main(int argc, char **argv){
    const char *path = argc>1 ? argv[1] : "pares.tsv";
    if(argc>2) d = (int)atol(argv[2]);
    const long LIM  = argc>3 ? atol(argv[3]) : 0;
    const long EXPL = argc>4 ? atol(argv[4]) : 0;      /* Λ = C^EXPL : inobservável (§O3)          */
    if(argc>5) MODO_FRACO = (int)atol(argv[5]);
    if(d<2 || d>MAXD){ printf("d entre 2 e %d\n", MAXD); return 2; }

    printf("OPERADOR — d=%d sobre ℤ_%d ; palavra=matriz, frase=produto ORDENADO,\n", d, p);
    printf("significado=polinômio característico, tradução=conjugação por Λ=C^%ld\n", EXPL);
    printf("colheita: %s\n", MODO_FRACO ?
        "pelo POLINÔMIO (companion do alvo — gasta só os d invariantes)" :
        "pela MATRIZ (as d² entradas — queima d² graus por palavra)");
    printf("=================================================================\n");

    /* --- §O0: a inversa COLHIDA — três batidas (ℱ³=ℱ⁻¹, G⁴=I), não construída --- */
    {
        M G, G2, G3, G4, I, Gi;
        giro_d(G); mid(I);
        mmul(G2,G,G); mmul(G3,G2,G); mmul(G4,G3,G);
        int ok4 = meq(G4, I);
        int okinv = minv(Gi,G) && meq(G3, Gi);
        printf("§O0  a inversa colhida: G⁴=I %s ; G³=G⁻¹ %s  (três batidas são a volta)\n",
               VD(!(ok4), "resíduo 0"), VD(!(okinv), "resíduo 0"));
        M C, R, CR, RC;
        gato_d(C); refl_d(R); mmul(CR,C,R); mmul(RC,R,C);
        printf("     não-comutatividade (o ν): C·R ≠ R·C %s  — sem ela o operador\n",
               meq(CR,RC)?"FALHA (comutam)":"resíduo 0");
        printf("     colapsaria no saco de palavras (A^k₁A^k₂=A^{k₁+k₂})\n");
    }

    M LAM, LAMI;
    { M C; gato_d(C); if(EXPL) mpow(LAM,C,EXPL); else mid(LAM);
      if(!minv(LAMI,LAM)){ printf("Λ singular\n"); return 2; } }

    unlink(LEXFILE);
    fdlex = open(LEXFILE, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fdlex<0 || ftruncate(fdlex, NSLOT*(long)sizeof(Slot))!=0){ printf("léxico falhou\n"); return 2; }
    FILE *f = fopen(path,"r");
    if(!f){ printf("sem %s\n", path); return 2; }

    long sementes=0, deduzidas=0;

    for(int rodada=1; rodada<=MAXROD; rodada++){
        long caiu_total=0;
        for(int c=0;c<MAXCAS;c++){
            rewind(f);
            long linha=0, caiu=0;
            while(fgets(line,sizeof line,f)){
                linha++; if(LIM && linha>LIM) break;
                char *tab=strchr(line,'\t'); if(!tab) continue;
                *tab=0; char *en=line, *pt=tab+1;
                char *nl=strchr(pt,'\n'); if(nl)*nl=0;
                if(!*en||!*pt) continue;

                M Xen, Xpt; Inc u[3]; int nu=0;
                varre_lado(en,0,Xen,u,&nu);
                varre_lado(pt,1,Xpt,u,&nu);
                if(nu!=1 || u[0].mult!=1) continue;

                /* RHS = Λ·Xpt·Λ⁻¹ (falta no EN) ou LHS levado ao lado PT */
                M P,S,alvo,T,Pi,Si,G;
                if(u[0].lado==0){
                    if(!pre_suf(en,0,u[0].pos,P,S)) continue;
                    mmul(T,LAM,Xpt); mmul(alvo,T,LAMI);
                } else {
                    if(!pre_suf(pt,1,u[0].pos,P,S)) continue;
                    mmul(T,LAMI,Xen); mmul(alvo,T,LAM);
                }
                if(!minv(Pi,P) || !minv(Si,S)) continue;
                if(MODO_FRACO){
                    /* a exigência real são os d INVARIANTES, não as d² entradas: basta que o produto
                     * tenha o polinômio alvo. Toma-se a forma canônica — a companion do polinômio —
                     * e gasta-se só o que o significado pede:  G = P⁻¹·companion(e)·S⁻¹.           */
                    int32_t e[MAXD+1];
                    charpoly(alvo, e);
                    M Y; companion(e, Y);
                    mmul(T,Pi,Y); mmul(G,T,Si);
                } else {
                    mmul(T,Pi,alvo); mmul(G,T,Si);    /* G = P⁻¹·alvo·S⁻¹ — a matriz inteira        */
                }
                grava(u[0].slot, u[0].fp, G);
                caiu++;
            }
            caiu_total += caiu;
            if(!caiu) break;
        }
        deduzidas += caiu_total;

        rewind(f);
        long linha=0, sem=0;
        while(fgets(line,sizeof line,f)){
            linha++; if(LIM && linha>LIM) break;
            char *tab=strchr(line,'\t'); if(!tab) continue;
            *tab=0; char *en=line, *pt=tab+1;
            char *nl=strchr(pt,'\n'); if(nl)*nl=0;
            if(!*en||!*pt) continue;
            M Xen,Xpt; Inc u[3]; int nu=0;
            varre_lado(en,0,Xen,u,&nu);
            varre_lado(pt,1,Xpt,u,&nu);
            if(nu<1 || nu>2) continue;
            M S1; semente(u[0].fp, S1);
            grava(u[0].slot, u[0].fp, S1);
            sem++;
            if(sem >= 20000) break;
        }
        sementes += sem;
        printf("  rodada %d: caem %7ld · sementes %6ld\n", rodada, caiu_total, sem);
        if(!sem && !caiu_total) break;
    }

    /* --- a verificação: o forte (a matriz) e o FRACO (o polinômio, o significado) --- */
    rewind(f);
    long tot=0, cob=0, forte=0, fraco=0;
    while(fgets(line,sizeof line,f)){
        static long linha=0; linha++;
        if(LIM && linha>LIM) break;
        char *tab=strchr(line,'\t'); if(!tab) continue;
        *tab=0; char *en=line, *pt=tab+1;
        char *nl=strchr(pt,'\n'); if(nl)*nl=0;
        if(!*en||!*pt) continue;
        tot++;
        M Xen,Xpt; Inc u[3]; int nu=0;
        varre_lado(en,0,Xen,u,&nu);
        varre_lado(pt,1,Xpt,u,&nu);
        if(nu) continue;
        cob++;
        M T, RHS; mmul(T,LAM,Xpt); mmul(RHS,T,LAMI);
        if(meq(Xen,RHS)) forte++;
        int32_t ea[MAXD+1], eb[MAXD+1];
        charpoly(Xen,ea); charpoly(Xpt,eb);            /* conjugação preserva o polinômio           */
        if(poly_eq(ea,eb)) fraco++;
    }
    fclose(f);

    long ocup=0;
    for(long i=0;i<NSLOT;i++){
        Slot s;
        if(pread(fdlex,&s,sizeof s,i*(long)sizeof(Slot))!=(long)sizeof(Slot)) break;
        if(s.fp) ocup++;
    }
    close(fdlex);

    printf("\npalavras (operadores) no léxico     : %ld\n", ocup);
    printf("  sementes (graus queimados)        : %ld\n", sementes);
    printf("  colhidas por inversão             : %ld\n", deduzidas);
    printf("\nequações                            : %ld  (cobertas %ld)\n", tot, cob);
    printf("  FORTE  Xen = Λ·Xpt·Λ⁻¹            : %ld/%ld  (%.2f%%)\n", forte, cob,
           cob?100.0*forte/cob:0);
    printf("  FRACO  charpoly(Xen)=charpoly(Xpt): %ld/%ld  (%.2f%%)   ← o significado\n", fraco, cob,
           cob?100.0*fraco/cob:0);
    printf("\ncontagem: E=%ld equações de %d invariantes ; V=%ld palavras de %d incógnitas\n",
           tot, d, ocup, d*d);
    printf("          E·d=%ld  vs  V·d²=%ld   →  %s (d ≥ E/V exige d ≥ 6)\n",
           tot*d, ocup*(long)d*d, (ocup*(long)d*d >= tot*(long)d) ? "há liberdade" : "FALTA liberdade");
    return 0;
}
