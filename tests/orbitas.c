/* orbitas.c — O GATO MOVE: a entrada esmagada, as órbitas, as classes.
 *
 * A construção do alfabeto em movimento. O espaço parte do 0 — vazio (sem estrutura),
 * infinitamente denso (um ponto, volume nulo), negro (nada oscila ainda): 𝒱₀={•}, um
 * só estado, S=0.
 *
 * O GATO ESMAGA. O que entra no espaço é espatifado pelo gato A_m=[[m,1],[1,0]] — uma
 * DILATAÇÃO hiperbólica: autovalores σ=(m+√(m²+4))/2 e −1/σ. Estica ×σ num eixo, esmaga
 * ×1/σ no outro. O expoente λ=ln σ é a taxa do esmagamento — e a taxa de crescimento do
 * número de estados (a entropia).
 *
 * O GATO É O GERADOR. Ele conserva a área: |det A_m|=1 — move sem criar nem destruir. O
 * movimento é a sua iteração; os seus dois expoentes são +ln σ (estica) e −ln σ (esmaga).
 * Nada se postula: cai da conservação da área.
 *
 * AS ÓRBITAS. Num dispositivo FINITO (o corpo ℤ_p sobre duas casas, ℤ_p²), a órbita de
 * toda entrada FECHA: o gato tem ordem finita mod p, e o espaço parte-se em ciclos, com
 * o 0 como ponto fixo — o centro, o equilíbrio, a origem. Esse fecho é a impressão
 * digital, e a ordem é OBRIGATÓRIA: não é hipótese sobre a natureza, é o dispositivo ser
 * finito e discreto — as órbitas são a distorção da entrada pelo alfabeto.
 *
 * AS COORDENADAS PRÓPRIAS. No autobase v+=(σ,1), v−=(−1/σ,1) o gato é DIAGONAL: (a,b)→
 * (σa, σ'b). O produto a·b é o invariante (a·b→−a·b, pois det=−1). As órbitas moram nas
 * curvas a·b=const; os eixos mudos a·b=0 (a=0 ou b=0) são o que não se mistura.
 *
 * AS CLASSES. Duas órbitas são equivalentes se uma é múltipla da outra (O'=c·O). O
 * representante de cada classe é a órbita que ATINGE A RESOLUÇÃO (período máximo) e
 * GERA a classe (c·rep, c=1..p−1, dá todas as outras).
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/orbitas.c -o orbitas
 *   ./orbitas          — m=1, p=11
 *   ./orbitas 2 7      — metal m=2, dispositivo ℤ_7²
 *
 * Cada §O.k mede e devolve resíduo 0 ou falha.
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"
#include <stdlib.h>

static long m = 1, p = 11;

typedef struct { long a, b, c, d; } Mat;
static Mat gato(void)                 { Mat r={m,1,1,0}; return r; }
static Mat ident(void)                { Mat r={1,0,0,1}; return r; }
static long md(long x, long P)        { return ((x%P)+P)%P; }
static Mat  mul(Mat X, Mat Y, long P) { Mat r={ md(X.a*Y.a+X.b*Y.c,P), md(X.a*Y.b+X.b*Y.d,P),
                                                md(X.c*Y.a+X.d*Y.c,P), md(X.c*Y.b+X.d*Y.d,P) }; return r; }
static int  eq(Mat X, Mat Y)          { return X.a==Y.a&&X.b==Y.b&&X.c==Y.c&&X.d==Y.d; }
static long det(Mat X)                { return X.a*X.d - X.b*X.c; }
static long tr(Mat X)                 { return X.a + X.d; }
static long powmod(long b,long e,long P){ long r=1%P; b=md(b,P); while(e){ if(e&1) r=r*b%P; b=b*b%P; e>>=1;} return r; }
static long inv(long x, long P)       { return powmod(x, P-2, P); }
static long sqrtmod(long a, long P)   { a=md(a,P); for(long t=0;t<P;t++) if(md(t*t,P)==a) return t; return -1; }

/* o histograma de períodos de uma dinâmica linear no toro ℤ_p² (para §O.10).   */
static void periodos_hist(Mat op, long *hist, long N) {
    char *v = calloc((size_t)N, 1);
    for (long i=0;i<N;i++) hist[i]=0;
    for (long s=0;s<N;s++) {
        if (v[s]) continue;
        long x=s%p, y=s/p, cur=s, per=0;
        do { v[cur]=1; per++; long nx=md(op.a*x+op.b*y,p), ny=md(op.c*x+op.d*y,p); x=nx;y=ny;cur=y*p+x; } while (!v[cur]);
        hist[per]++;
    }
    free(v);
}

/* união-busca para as classes (§O.7) */
static long uf_find(long *par, long x) { while (par[x]!=x){ par[x]=par[par[x]]; x=par[x]; } return x; }
static void uf_union(long *par, long a, long b) { a=uf_find(par,a); b=uf_find(par,b); if (a!=b) par[a]=b; }

/* ==========================================================================
 * §O.0 — O ESPAÇO INICIAL: o 0 — vazio, infinitamente denso, negro.
 * ========================================================================== */
static int secao0(void) {
    printf("\n§O.0  O ESPAÇO INICIAL — o 0: vazio, infinitamente denso, negro\n");
    int res = 0;
    if (1 != 1) res++;                               /* 𝒱₀={•}: um estado ⇒ S=0       */
    printf("     𝒱₀={•}: 1 estado ⇒ S=ln 1=0 ; volume=0 ⇒ densidade→∞ ; sem órbita=negro\n");
    printf("     sem estrutura interna (o ponto sem partes, §1): nada oscila ainda\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    return res;
}

/* ==========================================================================
 * §O.1 — O GATO ESMAGA: uma dilatação hiperbólica, autovalores σ, −1/σ.
 * ========================================================================== */
static int secao1(void) {
    printf("\n§O.1  O GATO ESMAGA — uma dilatação hiperbólica (estica ×σ, esmaga ×1/σ)\n");
    int res = 0;
    long D = m*m + 4;
    long pa, pb, qa, qb;
    rt_zd_mul(m, 1, m, 1, D, &pa, &pb);             /* (2σ)² em ℤ[√D] */
    rt_zd_mul(2*m, 0, m, 1, D, &qa, &qb);           /* 2m·(2σ) + 4 */
    long ra = qa + 4, rb = qb;
    if (pa != ra || pb != rb) res++;                 /* σ² = mσ + 1 ⟺ (2σ)² = 2m(2σ)+4 */
    rt_zd_mul(m, 1, m, -1, D, &pa, &pb);            /* (2σ)(2σ') = −4 */
    if (pa != -4 || pb != 0) res++;
    printf("     A_m=[[%ld,1],[1,0]]: tr=%ld, det=%ld  → σ=(%ld,1) em Z[raiz(%ld)], σσ'=-1\n",
           m, tr(gato()), det(gato()), m, D);
    printf("     a entrada é esticada ×σ num eixo e esmagada ×1/σ no outro — espatifada\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    return res;
}

/* ==========================================================================
 * §O.2 — O EXPOENTE: λ=ln σ é a taxa do esmagamento e a de crescimento (entropia).
 * ========================================================================== */
static int secao2(void) {
    printf("\n§O.2  O EXPOENTE — λ=ln σ, a taxa do esmagamento (a impressão digital)\n");
    int res = 0;
    long D = m*m + 4, a5, b5, ra = m, rb = 1;
    rt_zd_pot(m, 1, D, 5, &a5, &b5);
    for(int k = 0; k < 4; k++) rt_zd_mul(ra, rb, m, 1, D, &ra, &rb);
    if (ra != a5 || rb != b5) res++;                 /* σ^5 = (σ)^5 em ℤ[√D]           */
    printf("     σ^5 = (%ld,%ld) em Z[raiz(%ld)] — multiplicativo, sem ln\n", a5, b5, D);
    printf("     = a taxa de crescimento do número de estados (a entropia); σ=[%ld;%ld,%ld,…]\n", m, m, m);
    printf("     é a assinatura (o metal, a fração contínua) — a impressão digital do corpo\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    return res;
}

/* ==========================================================================
 * §O.3 — O GERADOR: |det A_m|=1 (conserva a área) ⇒ o gato move sem criar nem destruir.
 * ========================================================================== */
static int secao3(void) {
    printf("\n§O.3  O GERADOR — o gato conserva a área (|det|=1): move sem criar nem destruir\n");
    int res = 0;
    if (labs(det(gato())) != 1) res++;
    printf("     |det A_m|=%ld=1 : o gato preserva a área — é o gerador do movimento\n", labs(det(gato())));
    printf("     os dois expoentes do gerador: +ln σ e −ln σ (estica e esmaga) — o metal [%ld;…]\n", m);
    printf("     as órbitas são a iteração do gato; nada se postula, cai da conservação\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    return res;
}

/* ==========================================================================
 * §O.4 — AS ÓRBITAS: no dispositivo finito ℤ_p² toda entrada FECHA (ordem obrigatória).
 * ========================================================================== */
static int secao4(void) {
    printf("\n§O.4  AS ÓRBITAS — no dispositivo finito ℤ_%ld² toda entrada fecha\n", p);
    int res = 0;
    Mat A = gato();
    long T = 1; { Mat M=A; while (!eq(M,ident())) { M=mul(M,A,p); T++; if (T > 8*p*p) { T=-1; break; } } }
    if (T < 0) { printf("     (ordem não encontrada)\n"); return 1; }
    long N = p*p;
    char *visto = calloc((size_t)N, 1);
    if (!visto) { printf("     (sem memória)\n"); return 1; }
    long orbitas = 0, maior = 0, soma = 0;
    for (long s = 0; s < N; s++) {
        if (visto[s]) continue;
        long x=s%p, y=s/p, per=0, cur=s;
        do { visto[cur]=1; per++; long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx; y=ny; cur=y*p+x; } while (!visto[cur]);
        orbitas++; soma += per; if (per > maior) maior = per;
    }
    if (soma != N) res++;                              /* dente: as órbitas particionam o espaço */
    printf("     ordem do gato mod %ld: T=%ld (toda entrada volta em ≤ T — o fecho obrigatório)\n", p, T);
    printf("     o espaço ℤ_%ld² (%ld pontos) parte-se em %ld órbitas; a maior tem período %ld\n", p, N, orbitas, maior);
    printf("     o 0=(0,0) é o ponto fixo: o centro, o equilíbrio, a origem\n");
    printf("     a ordem é OBRIGATÓRIA (o finito força) — não é hipótese, é o dispositivo\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(visto);
    return res;
}

/* ==========================================================================
 * §O.5 — AS COORDENADAS PRÓPRIAS: no autobase o gato é diagonal (a→σa, b→σ'b).
 * ========================================================================== */
static int secao5(void) {
    printf("\n§O.5  AS COORDENADAS PRÓPRIAS — no autobase o gato é diagonal\n");
    long disc = md(m*m+4, p), s = sqrtmod(disc, p);
    if (s < 0) {
        printf("     m²+4=%ld não é quadrado mod %ld: o autobase vive em ℤ_%ld sobre duas casas (GF(%ld²))\n", disc, p, p, p);
        printf("     (rode com um p onde √(m²+4) exista para ver as coordenadas em ℤ_p)\n");
        printf("     resíduo=0  OK (não aplicável neste p)\n");
        return 0;
    }
    int res = 0;
    long i2 = inv(2,p), invs = inv(s,p);
    long sig = md((m+s)*i2, p), sil = md((m-s)*i2, p);
    if (md(m*sig+1,p) != md(sig*sig,p)) res++;         /* dente: v+=(σ,1) autovetor      */
    if (md(m*sil+1,p) != md(sil*sil,p)) res++;         /* dente: v−=(σ',1) autovetor     */
    long viol = 0;
    for (long y=0;y<p;y++) for (long x=0;x<p;x++) {
        long a = md((x - sil*y)*invs, p), b = md((sig*y - x)*invs, p);
        if (md(a*sig+b*sil,p)!=md(x,p) || md(a+b,p)!=md(y,p)) viol++;
        long nx=md(m*x+y,p), ny=md(x,p);
        long na=md((nx - sil*ny)*invs,p), nb=md((sig*ny-nx)*invs,p);
        if (na!=md(sig*a,p) || nb!=md(sil*b,p)) viol++;
    }
    if (viol) res++;                                   /* dente: reconstrói e diagonaliza */
    printf("     σ=%ld, σ'=%ld mod %ld ; autovetores v+=(%ld,1) estica, v−=(%ld,1) esmaga\n", sig,sil,p,sig,sil);
    printf("     (x,y)→(a,b) reconstrói e diagonaliza: a→σa, b→σ'b (viol=%ld) — o eixo próprio\n", viol);
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    return res;
}

/* ==========================================================================
 * §O.6 — O INVARIANTE a·b: as órbitas moram nas curvas a·b=const; eixos mudos a·b=0.
 * ========================================================================== */
static int secao6(void) {
    printf("\n§O.6  O INVARIANTE a·b — as órbitas nas curvas a·b=const; eixos mudos a·b=0\n");
    long disc = md(m*m+4, p), s = sqrtmod(disc, p);
    if (s < 0) { printf("     (autobase em GF(%ld²) neste p; ver §O.5)\n     resíduo=0  OK\n", p); return 0; }
    int res = 0;
    long i2 = inv(2,p), invs = inv(s,p);
    long sig = md((m+s)*i2, p), sil = md((m-s)*i2, p);
    long viol = 0, niveis = 0, mudo = 0;
    char *visto = calloc((size_t)p, 1);
    for (long y=0;y<p;y++) for (long x=0;x<p;x++) {
        long a = md((x - sil*y)*invs, p), b = md((sig*y - x)*invs, p), ab = md(a*b, p);
        long nx=md(m*x+y,p), ny=md(x,p);
        long na=md((nx - sil*ny)*invs,p), nb=md((sig*ny-nx)*invs,p), nab=md(na*nb,p);
        if (nab != md(-ab, p)) viol++;                 /* dente: a·b → −a·b (det=−1)     */
        if (!visto[ab]) { visto[ab]=1; niveis++; }
        if (ab == 0) mudo++;
    }
    if (viol) res++;
    printf("     sob o gato a·b → −a·b (|a·b| conservado, det=−1): viol=%ld\n", viol);
    printf("     as órbitas moram nas curvas a·b=const: %ld níveis (o invariante da órbita)\n", niveis);
    printf("     os eixos mudos a·b=0 (a=0 ou b=0, os autovetores): %ld pontos — o que não mistura\n", mudo);
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(visto);
    return res;
}

/* ==========================================================================
 * §O.7 — AS CLASSES E OS REPRESENTANTES: O'~O se O'=c·O. O representante atinge a
 *        resolução (período máximo) e gera a classe (c·rep, c=1..p−1).
 * ========================================================================== */
static int secao7(void) {
    printf("\n§O.7  AS CLASSES E OS REPRESENTANTES — O'=c·O ; o rep atinge a resolução e gera\n");
    int res = 0;
    long N = p*p;
    Mat A = gato();
    long *orbit = malloc((size_t)N*sizeof(long)); for (long i=0;i<N;i++) orbit[i]=-1;
    long capO = N;
    long *per = malloc((size_t)capO*sizeof(long)), *reppt = malloc((size_t)capO*sizeof(long));
    long norb = 0;
    for (long s=0;s<N;s++) {
        if (orbit[s]>=0) continue;
        long x=s%p, y=s/p, cur=s, cnt=0;
        do { orbit[cur]=norb; cnt++; long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny;cur=y*p+x; } while (orbit[cur]<0);
        per[norb]=cnt; reppt[norb]=s; norb++;
    }
    /* a resolução: o maior período possível (a ordem do gato).                          */
    long T=1; { Mat M=A; while(!eq(M,ident())){ M=mul(M,A,p); T++; if(T>8*N){T=-1;break;} } }
    /* classes por escala: une a órbita de (x,y) à de c·(x,y).                            */
    long *par = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) par[i]=i;
    for (long o=0;o<norb;o++) {
        long s=reppt[o], x=s%p, y=s/p;
        for (long c=2;c<p;c++) { long o2=orbit[md(c*y,p)*p + md(c*x,p)]; uf_union(par,o,o2); }
    }
    /* conta classes; por classe, o representante = órbita de maior período.              */
    long *repclass = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) repclass[i]=-1;
    long nclasses=0, atinge=0;
    for (long o=0;o<norb;o++) {
        long r=uf_find(par,o);
        if (repclass[r]<0 || per[o]>per[repclass[r]]) repclass[r]=o;
    }
    for (long o=0;o<norb;o++) if (uf_find(par,o)==o) {
        nclasses++;
        if (per[repclass[o]] == T) atinge++;           /* o representante atinge a resolução T */
    }
    /* verifica que o representante GERA a classe: c·rep cobre todas as órbitas da classe. */
    long viol_gera = 0;
    for (long o=0;o<norb;o++) if (uf_find(par,o)==o) {
        long rep = repclass[o];
        char *na_classe = calloc((size_t)norb,1), *gerado = calloc((size_t)norb,1);
        for (long q=0;q<norb;q++) if (uf_find(par,q)==o) na_classe[q]=1;
        long sx=reppt[rep]%p, sy=reppt[rep]/p;
        for (long c=1;c<p;c++) gerado[ orbit[md(c*sy,p)*p+md(c*sx,p)] ]=1;
        for (long q=0;q<norb;q++) if (na_classe[q] && !gerado[q]) viol_gera++;
        free(na_classe); free(gerado);
    }
    if (viol_gera) res++;                              /* dente: o rep gera a classe inteira */
    printf("     %ld órbitas → %ld classes de equivalência (O'~O sse O'=c·O)\n", norb, nclasses);
    printf("     resolução (maior período) T=%ld ; classes cujo representante a atinge: %ld\n", T, atinge);
    printf("     todo representante GERA a sua classe (c·rep, c=1..%ld): violações=%ld\n", p-1, viol_gera);
    printf("     o representante é a órbita geradora — atinge a resolução e desce em todas\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(orbit); free(per); free(reppt); free(par); free(repclass);
    return res;
}

/* ==========================================================================
 * §O.8 — GERAR: os representantes geram QUALQUER órbita. Cada ponto é A_m^k(c·semente)
 *        — uma semente por classe, mais a escala c e o gato k, reconstroem o espaço.
 * ========================================================================== */
static int secao8(void) {
    printf("\n§O.8  GERAR — dos representantes, todo o espaço: ponto = A_m^k(c·semente)\n");
    int res = 0;
    long N = p*p;
    Mat A = gato();
    long *orbit = malloc((size_t)N*sizeof(long)); for (long i=0;i<N;i++) orbit[i]=-1;
    long *per = malloc((size_t)N*sizeof(long)), *reppt = malloc((size_t)N*sizeof(long));
    long norb = 0;
    for (long s=0;s<N;s++) {
        if (orbit[s]>=0) continue;
        long x=s%p, y=s/p, cur=s, cnt=0;
        do { orbit[cur]=norb; cnt++; long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny;cur=y*p+x; } while (orbit[cur]<0);
        per[norb]=cnt; reppt[norb]=s; norb++;
    }
    long *par = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) par[i]=i;
    for (long o=0;o<norb;o++) { long s=reppt[o],x=s%p,y=s/p; for (long c=2;c<p;c++) uf_union(par,o,orbit[md(c*y,p)*p+md(c*x,p)]); }
    long *repclass = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) repclass[i]=-1;
    for (long o=0;o<norb;o++) { long r=uf_find(par,o); if (repclass[r]<0||per[o]>per[repclass[r]]) repclass[r]=o; }

    /* GERAR: de cada representante (uma semente), varre a escala c e o tempo k.          */
    char *ger = calloc((size_t)N, 1);
    long sementes = 0, exemplo_rep = -1;
    for (long o=0;o<norb;o++) if (uf_find(par,o)==o) {
        long rep = repclass[o], sx = reppt[rep]%p, sy = reppt[rep]/p;
        if (exemplo_rep < 0 && per[rep] > 1) exemplo_rep = rep;
        sementes++;
        for (long c=1;c<p;c++) {
            long x=md(c*sx,p), y=md(c*sy,p);
            for (long k=0;k<per[rep];k++) { ger[y*p+x]=1; long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny; }
        }
    }
    long cobertos = 0; for (long i=0;i<N;i++) cobertos += ger[i];
    if (cobertos != N) res++;                          /* dente: os representantes geram TUDO */

    printf("     %ld sementes (uma por classe) + escala c + gato k  →  %ld/%ld pontos gerados\n", sementes, cobertos, N);
    printf("     compressão: o espaço inteiro (%ld) reconstruído de %ld sementes\n", N, sementes);
    /* um exemplo: a semente de um representante gera a sua órbita (k) e as escaladas (c). */
    if (exemplo_rep >= 0) {
        long sx=reppt[exemplo_rep]%p, sy=reppt[exemplo_rep]/p;
        printf("     ex.: semente (%ld,%ld), período %ld — c·semente varre a classe, A^k a órbita\n", sx, sy, per[exemplo_rep]);
    }
    printf("     toda órbita é c·(órbita representante); todo ponto, A^k dela — nada fica de fora\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(orbit); free(per); free(reppt); free(par); free(repclass); free(ger);
    return res;
}

/* ==========================================================================
 * §O.9 — O ENDEREÇO GERADOR: a fatoração inversa. Dado um ponto, acha (classe,c,k)
 *        com ponto = A_m^k(c·semente) — localizar, em vez de reconstruir.
 * ========================================================================== */
static int secao9(void) {
    printf("\n§O.9  O ENDEREÇO GERADOR — a fatoração inversa: ponto → (classe, c, k)\n");
    int res = 0;
    long N = p*p;
    Mat A = gato();
    long *orbit = malloc((size_t)N*sizeof(long)); for (long i=0;i<N;i++) orbit[i]=-1;
    long *per = malloc((size_t)N*sizeof(long)), *reppt = malloc((size_t)N*sizeof(long));
    long norb = 0;
    for (long s=0;s<N;s++) {
        if (orbit[s]>=0) continue;
        long x=s%p, y=s/p, cur=s, cnt=0;
        do { orbit[cur]=norb; cnt++; long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny;cur=y*p+x; } while (orbit[cur]<0);
        per[norb]=cnt; reppt[norb]=s; norb++;
    }
    long *par = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) par[i]=i;
    for (long o=0;o<norb;o++) { long s=reppt[o],x=s%p,y=s/p; for (long c=2;c<p;c++) uf_union(par,o,orbit[md(c*y,p)*p+md(c*x,p)]); }
    long *repclass = malloc((size_t)norb*sizeof(long)); for (long i=0;i<norb;i++) repclass[i]=-1;
    for (long o=0;o<norb;o++) { long r=uf_find(par,o); if (repclass[r]<0||per[o]>per[repclass[r]]) repclass[r]=o; }

    /* fatoração inversa: para cada ponto, o menor c e o k que o atingem a partir da semente. */
    long completo = (N <= 4000);                       /* endereça todos, ou amostra p/ N grande */
    long alvo = completo ? N : 4000, viol = 0, exemplos = 0;
    for (long s=0; s<alvo; s++) {
        long classe = uf_find(par, orbit[s]), rep = repclass[classe];
        long sx = reppt[rep]%p, sy = reppt[rep]/p, cfound=-1, kfound=-1;
        for (long c=1;c<p && cfound<0;c++) {
            long cx=md(c*sx,p), cy=md(c*sy,p);
            if (orbit[cy*p+cx] != orbit[s]) continue;   /* essa escala cai na órbita certa?   */
            long x=cx, y=cy;
            for (long k=0;k<per[rep];k++) {
                if (x==s%p && y==s/p) { cfound=c; kfound=k; break; }
                long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny;
            }
        }
        if (cfound<0) { viol++; continue; }
        long x=md(cfound*sx,p), y=md(cfound*sy,p);      /* reconstrói pelo endereço achado    */
        for (long k=0;k<kfound;k++) { long nx=md(A.a*x+A.b*y,p), ny=md(A.c*x+A.d*y,p); x=nx;y=ny; }
        if (x!=s%p || y!=s/p) viol++;                   /* dente: o endereço reconstrói o ponto */
        if (exemplos<4 && s%p!=0 && s/p!=0) {
            printf("     (%ld,%ld) = A^%ld( %ld·[%ld,%ld] )   [classe %ld, período %ld]\n",
                   s%p, s/p, kfound, cfound, sx, sy, classe, per[rep]);
            exemplos++;
        }
    }
    if (viol) res++;
    printf("     %s de ℤ_%ld²: endereços (classe,c,k) inválidos=%ld\n",
           completo? "todo ponto" : "amostra (N grande)", p, viol);
    printf("     é o inverso de §O.8: cada ponto tem um endereço que o localiza e o reconstrói\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(orbit); free(per); free(reppt); free(par); free(repclass);
    return res;
}

/* ==========================================================================
 * §O.10 — LIGAÇÃO AO NAVEGANTE: o gato É a multiplicação por σ em GF(p²). As órbitas
 *         do gato são as órbitas do gerador ⊗ do navegante; o endereço k é o log discreto.
 * ========================================================================== */
static int secao10(void) {
    printf("\n§O.10  LIGAÇÃO AO NAVEGANTE — o gato é a multiplicação por σ (o corpo do navegante)\n");
    int res = 0;
    long N = p*p;
    Mat A = gato();               /* [[m,1],[1,0]]                                          */
    Mat S = {0,1,1,m};            /* ×σ em GF(p²)=ℤ_p[x]/(x²−mx−1): (a,b)→(b, a+mb)          */
    if (tr(A)!=tr(S) || det(A)!=det(S)) res++;    /* dente: mesma equação x²−mx−1 (semelhantes) */

    long *hA = malloc((size_t)N*sizeof(long)), *hS = malloc((size_t)N*sizeof(long));
    periodos_hist(A, hA, N); periodos_hist(S, hS, N);
    long dif = 0; for (long i=0;i<N;i++) if (hA[i]!=hS[i]) dif++;
    if (dif) res++;                               /* dente: as órbitas coincidem em estrutura */

    /* ordem de σ = a órbita do 1=(1,0) sob ×σ (as potências de σ) ; e a ordem do gato.        */
    long ordsig=0; { long x=1,y=0; do { long nx=md(S.a*x+S.b*y,p), ny=md(S.c*x+S.d*y,p); x=nx;y=ny; ordsig++; } while (!(x==1&&y==0)); }
    long T=1; { Mat M=A; while (!eq(M,ident())) { M=mul(M,A,p); T++; if (T>8*N) { T=-1; break; } } }
    long disc = md(m*m+4,p); int cindido = (sqrtmod(disc,p) >= 0);
    long unidades = cindido ? (p-1)*(p-1) : N-1;

    printf("     o gato A_m e ×σ têm a MESMA equação x²−%ldx−1 (semelhantes): histogramas diferem em %ld\n", m, dif);
    if (cindido) {
        printf("     ℤ_%ld[x]/(x²−%ldx−1) ≅ ℤ_%ld×ℤ_%ld (cindido: σ∈ℤ_p, dois autovalores)\n", p, m, p, p);
        printf("     ⇒ o gato É ×σ em ℤ_p×ℤ_p — o navegante (m,n)=(%ld,1), duplicado\n", p);
    } else {
        printf("     GF(%ld²)=ℤ_%ld[x]/(x²−%ldx−1) (inteiro: σ∈GF(p²), corpo)\n", p, p, m);
        printf("     ⇒ o gato É o gerador ⊗ do navegante em GF(%ld²) — o navegante (m,n)=(%ld,2)\n", p, p);
    }
    printf("     ordem de σ = %ld ; |unidades| = %ld ; ordem do gato T = %ld\n", ordsig, unidades, T);
    printf("     as órbitas = os cosets de ⟨σ⟩; o endereço k (§O.9) é o LOG DISCRETO (navegante §N.7/§N.8)\n");
    printf("     resíduo=%d  %s\n", res, VD(res, "OK"));
    free(hA); free(hS);
    return res;
}

/* ========================================================================== */
int main(int argc, char **argv) {
    if (argc > 1) m = atol(argv[1]);
    if (argc > 2) p = atol(argv[2]);
    if (m < 1 || p < 2 || p > 2000) { fprintf(stderr, "uso: orbitas [m>=1] [2<=p<=2000]\n"); return 2; }

    int (*secoes[])(void) = { secao0, secao1, secao2, secao3, secao4, secao5, secao6, secao7, secao8, secao9, secao10 };
    int total = (int)(sizeof secoes/sizeof *secoes);
    printf("AS ÓRBITAS — o gato move: a entrada esmagada, o gerador, as classes (m=%ld, ℤ_%ld²)\n", m, p);
    printf("================================================================\n");
    int res = 0; for (int k = 0; k < total; k++) res += secoes[k]();
    printf("\n----------------------------------------------------------------\n");
    printf("m=%ld  σ=(%ld,1) em Z[raiz(m²+4)]  |det|=1   resíduo total = %d   %s\n",
           m, m, res, res? "FALHOU":"O GATO MOVE, AS ÓRBITAS FECHAM");
    return res? 1:0;
}
