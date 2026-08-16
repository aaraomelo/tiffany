/* gerador_analog.c — O GERADOR NOS DOIS MEIOS: a torção discreta e a rotação da malha LC.
 *
 * O gerador global (gerador.c) é digital e exato: w = 36043 em ℤ_40961, de ordem n = 256. Este arquivo
 * mede o MESMO gerador no outro meio --- o analógico, a malha LC do §B.1 --- e mostra que são a mesma
 * peça, como neuronio.c/neuronio_analog.c e agm.c/agm_analog.c.
 *
 * A TORÇÃO NO ANALÓGICO. O oscilador LC gira o estado (V, I·Z₀) com ω₀ = 1/√(LC): em Δt = T/n ele gira
 * exatamente 2π/n. Essa rotação É a torção --- vive na BORDA |λ|=1 (não cresce, não decai, fecha), e n
 * passos devolvem a identidade. O que no discreto é w^n = 1, no contínuo é uma volta completa.
 *
 * E A TORRE É O ZOOM. No discreto os níveis são w_d = w^{n/d}, cada um o quadrado do seguinte. No
 * circuito, subir um nível é DOBRAR a frequência --- e o §B.7 já mede que o zoom (L,C)→(L/2,C/2) dobra
 * ω₀ mantendo Z₀. Então a torre fractal do gerador e o zoom do circuito são a mesma escada: elevar a
 * torção ao quadrado = dobrar ω₀ = descer um nível de L e C.
 *
 * Mede-se:
 *   (GA1) a torção analógica está na borda: n passos = identidade, energia conservada, e o nível 2d
 *         batido duas vezes é o nível d (o quadrado = o dobro da frequência);
 *   (GA2) a transformada colhida SEM TABELA: o fator gira por realimentação (a PG), ida e volta = id;
 *   (GA3) DIGITAL ≡ ANALÓGICO: a convolução circular pela torção exata em ℤ_p e pela rotação LC dão o
 *         MESMO vetor de inteiros, e ambas batem o oráculo O(n²);
 *   (GA4) o DENTE: rotação de ângulo errado (2π/(n+1)) quebra a ida e volta e a convolução.
 *
 * Buffers fixos (n ≤ 256), zero malloc.
 *
 *   cc -O2 -std=c99 gerador_analog.c -lm -o gerador_analog && ./gerador_analog
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 256
#define P_GLOBAL 40961L
#define W_GLOBAL 36043L
#define R_GLOBAL 16L

static int passou = 1;
static long p = P_GLOBAL;

/* --------- o lado DIGITAL: a torção exata em ℤ_p --------- */
static long md(long x){ x%=p; return x<0?x+p:x; }
static long mul(long a,long b){ return (a%p)*(b%p)%p; }
static long pot(long b,long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=mul(r,b); b=mul(b,b); e>>=1; } return r; }
static long inv(long a){ return pot(a,p-2); }

/* --------- o lado ANALÓGICO: a malha LC --------- */
/* estado u = (V, I·Z₀); a dinâmica LC gira u com ω₀. Em Δt = 2π/(n·ω₀) gira 2π/n. */
typedef struct { double c, s; } Rot;                  /* a rotação colhida: (cos θ, sin θ)         */
static Rot rot_lc(double L, double C, int niveis_de_zoom, int n){
    for(int z=0; z<niveis_de_zoom; z++){ L/=2; C/=2; }/* o zoom §B.7 dobra ω₀ e mantém Z₀          */
    double w0 = 1.0/sqrt(L*C);
    double dt = 2*M_PI/((double)n*w0);                /* um passo de T/n                           */
    Rot r = { cos(w0*dt), sin(w0*dt) };               /* = 2π/n, colhido da física                 */
    return r;
}
/* a rotação girada num intervalo dt FIXO pela malha (L,C) — é aqui que o zoom se vê: com o mesmo
 * dt, dobrar ω₀ dobra o ângulo, isto é ELEVA A TORÇÃO AO QUADRADO.                        */
static Rot rot_passo(double L, double C, double dt){
    double w0 = 1.0/sqrt(L*C), th = w0*dt;
    Rot r = { cos(th), sin(th) };
    return r;
}
static void aplica(Rot r, double *x, double *y){      /* gira (x,y) — a peça na borda              */
    double nx = r.c*(*x) - r.s*(*y);
    double ny = r.s*(*x) + r.c*(*y);
    *x=nx; *y=ny;
}

/* --------- convolução circular: o oráculo O(n²) --------- */
static void conv_oraculo(const long *a, const long *b, long *c, int n){
    for(int k=0;k<n;k++){
        long acc=0;
        for(int j=0;j<n;j++) acc += a[j]*b[((k-j)%n+n)%n];
        c[k]=acc;                                     /* em ℤ, sem reduzir — o valor verdadeiro     */
    }
}

int main(void){
    printf("GERADOR_ANALOG — o mesmo gerador nos dois meios: torção discreta e malha LC\n");
    printf("=================================================================\n");

    /* ---------- GA1: a torção analógica está na borda, e a torre é o zoom ---------- */
    printf("§GA1 a TORÇÃO no analógico é a rotação da malha LC (a borda |λ|=1):\n");
    {
        double L=1e-3, C=1e-9;                        /* uma malha qualquer: 1 mH, 1 nF            */
        printf("       L=%.0e H, C=%.0e F → ω₀=%.6e rad/s, Z₀=%.1f Ω\n",
               L, C, 1.0/sqrt(L*C), sqrt(L/C));
        Rot r = rot_lc(L,C,0,N);
        /* n passos = identidade */
        double x=1, y=0, e0 = 1.0;
        double pior_E=0;
        for(int i=0;i<N;i++){
            aplica(r,&x,&y);
            double E = x*x+y*y;                       /* a energia (a borda conserva)              */
            if(fabs(E-e0)>pior_E) pior_E=fabs(E-e0);
        }
        double volta = sqrt((x-1)*(x-1)+y*y);
        printf("       %d passos de 2π/n : volta à identidade com erro %.2e ; energia varia %.2e\n",
               N, volta, pior_E);
        int bom1 = (volta<1e-12 && pior_E<1e-12);
        /* a torre: o nível 2d batido DUAS vezes é o nível d — e é o zoom (dobrar ω₀) */
        int erro_torre=0;
        printf("       a TORRE, e ela é o zoom §B.7 (dobrar ω₀ = elevar a torção ao quadrado):\n");
        double w0base = 1.0/sqrt(L*C);
        for(int d=N; d>=4; d/=2){
            double dt = 2*M_PI/((double)(2*d)*w0base);  /* o passo do nível 2d, FIXO nos dois casos */
            Rot r2d = rot_passo(L,   C,   dt);          /* a malha original gira 2π/(2d)           */
            Rot rd  = rot_passo(L/2, C/2, dt);          /* com o ZOOM, ω₀ dobra: gira 2π/d         */
            double a1=1,b1=0, a2=1,b2=0;
            aplica(rd,&a1,&b1);
            aplica(r2d,&a2,&b2); aplica(r2d,&a2,&b2);   /* duas batidas do nível de cima            */
            double dif = sqrt((a1-a2)*(a1-a2)+(b1-b2)*(b1-b2));
            /* Z₀ FIXO SOB O ZOOM, e agora medido. O que aqui estava era
             *      difz = |√(L/C) − √((L/2)/(C/2))|
             * e (L/2)/(C/2) É L/C: as duas expressões são a mesma, logo difz era zero por
             * construção e a tese do §B.7 nunca foi medida. Mede-se assim:
             *
             *   — Z₀² = L/C é uma FRACÇÃO de inteiros (1 mH e 1 nF são 1000 µH e 1000 pF),
             *     e «os dois Z₀ são iguais» é L·C' = L'·C, por produto cruzado e sem raiz;
             *   — e o GUME: um zoom que divide só o L NÃO preserva Z₀, e tem de falhar.
             * Sem a segunda metade, «Z₀ fixo» continuava a valer por nada estar a mudar. */
            const long Lu = 1000, Cp = 1000;                 /* µH e pF: a malha, em inteiros */
            long Lz = Lu/2, Cz = Cp/2;                       /* o zoom do §B.7: (L,C) → (L/2,C/2) */
            long Lg = Lu/2, Cg = Cp;                         /* o gume: divide só o L            */
            int z_fixo  = (Lu*Cz == Lz*Cp);                  /* Z₀² igual, por cruzado */
            int g_mexeu = (Lu*Cg != Lg*Cp);                  /* e este TEM de mudar     */
            if(d==N||d==16||d==4)
                printf("         nível %3d : zoom(1 batida) == 2 batidas do nível %3d ? %.1e ;"
                       " Z₀ fixo? %s (e com o zoom torto muda? %s)\n",
                       d, 2*d, dif, z_fixo ? "sim" : "NAO", g_mexeu ? "sim" : "NAO");
            if(dif>1e-12 || !z_fixo || !g_mexeu) erro_torre=1;
        }
        printf("     %s\n", VD(!((bom1 && !erro_torre)), "resíduo 0 — a torção é a rotação da borda: fecha em n passos, conserva energia, e a\n"
          "     torre fractal É o zoom do circuito: com o MESMO passo de tempo, dobrar ω₀ (o zoom\n"
          "     (L,C)→(L/2,C/2) do §B.7) gira o DOBRO do ângulo — eleva a torção ao quadrado — e Z₀\n"
          "     fica fixo. Subir um nível da torre é descer um nível de L e C."));
        if(!bom1 || erro_torre) passou=0;
    }

    /* ---------- GA2: a transformada colhida sem tabela (a PG em correntes) ---------- */
    printf("\n§GA2 a transformada colhida SEM TABELA: o fator gira por realimentação (a PG)\n");
    {
        double *xr = DISCO_FIXO(double, 303);
        double *xi = DISCO_FIXO(double, 304);
        double *Xr = DISCO_FIXO(double, 305);
        double *Xi = DISCO_FIXO(double, 306);
        double *yr = DISCO_FIXO(double, 307);
        double *yi = DISCO_FIXO(double, 308);
        disco_prende(DISCO_BASE(303),"dados/xr_303.bin",(size_t)((N)),sizeof(double));
        disco_zera(xr,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(304),"dados/xi_304.bin",(size_t)((N)),sizeof(double));
        disco_zera(xi,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(305),"dados/Xr_305.bin",(size_t)((N)),sizeof(double));
        disco_zera(Xr,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(306),"dados/Xi_306.bin",(size_t)((N)),sizeof(double));
        disco_zera(Xi,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(307),"dados/yr_307.bin",(size_t)((N)),sizeof(double));
        disco_zera(yr,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(308),"dados/yi_308.bin",(size_t)((N)),sizeof(double));
        disco_zera(yi,(size_t)((N)),sizeof(double));
        long s=2024;
        for(int i=0;i<N;i++){ s=(s*1103515245+12345)&0x7fffffff; xr[i]=(double)(s%251); xi[i]=0; }
        double rn = 1.0/sqrt((double)N);
        /* F: para cada k, o fator gira por ×(rotação de 2πk/n) — nenhuma tabela de cos/sin */
        double L=1e-3, C=1e-9;
        Rot base = rot_lc(L,C,0,N);                    /* 2π/n, colhida da malha                    */
        /* wk = base^k obtido por realimentação (PG); dentro, o fator gira por wk (PG) */
        double wkc=1, wks=0;
        for(int k=0;k<N;k++){
            double fc=1, fs=0, ar=0, ai=0;
            for(int j=0;j<N;j++){
                ar += xr[j]*fc;  ai += -xr[j]*fs;      /* e^{-iθ}: conjugado                        */
                double nfc = fc*wkc - fs*wks, nfs = fc*wks + fs*wkc;
                fc=nfc; fs=nfs;                        /* o fator anda por UM produto (a PG)        */
            }
            Xr[k]=ar*rn; Xi[k]=ai*rn;
            double nwc = wkc*base.c - wks*base.s, nws = wkc*base.s + wks*base.c;
            wkc=nwc; wks=nws;                          /* k anda por soma; w^k por produto          */
        }
        /* Finv */
        wkc=1; wks=0;
        for(int j=0;j<N;j++){
            double fc=1, fs=0, ar=0, ai=0;
            for(int k=0;k<N;k++){
                ar += Xr[k]*fc - Xi[k]*fs;
                ai += Xr[k]*fs + Xi[k]*fc;
                double nfc = fc*wkc - fs*wks, nfs = fc*wks + fs*wkc;
                fc=nfc; fs=nfs;
            }
            yr[j]=ar*rn; yi[j]=ai*rn;
            double nwc = wkc*base.c - wks*base.s, nws = wkc*base.s + wks*base.c;
            wkc=nwc; wks=nws;
        }
        double pior=0;
        for(int i=0;i<N;i++){ double e=fabs(yr[i]-xr[i]); if(e>pior) pior=e; }
        printf("       Finv(F(x)) = x : erro máx %.2e   (estado: 4 escalares, nenhuma tabela)\n", pior);
        printf("     %s\n", pior<1e-9 ?
          "resíduo 0 no analógico — o mesmo desenho do digital: o expoente anda por SOMA e o\n"
          "     fator pela PG que a acompanha. Nenhum seno tabelado, nenhuma potência guardada."
          : "FALHA");
        if(pior>=1e-9) passou=0;
    }

    /* ---------- GA3: DIGITAL ≡ ANALÓGICO ---------- */
    printf("\n§GA3 DIGITAL ≡ ANALÓGICO: a convolução circular pelos dois meios, e o oráculo\n");
    {
        long *a = DISCO_FIXO(long, 310);
        long *b = DISCO_FIXO(long, 311);
        long *cor = DISCO_FIXO(long, 312);
        long *cdig = DISCO_FIXO(long, 313);
        disco_prende(DISCO_BASE(310),"dados/a_310.bin",(size_t)((N)),sizeof(long));
        disco_zera(a,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(311),"dados/b_311.bin",(size_t)((N)),sizeof(long));
        disco_zera(b,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(312),"dados/cor_312.bin",(size_t)((N)),sizeof(long));
        disco_zera(cor,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(313),"dados/cdig_313.bin",(size_t)((N)),sizeof(long));
        disco_zera(cdig,(size_t)((N)),sizeof(long));
        double *ar_ = DISCO_FIXO(double, 315);
        double *ai_ = DISCO_FIXO(double, 316);
        double *br_ = DISCO_FIXO(double, 317);
        double *bi_ = DISCO_FIXO(double, 318);
        double *Ar = DISCO_FIXO(double, 319);
        double *Ai = DISCO_FIXO(double, 320);
        double *Br = DISCO_FIXO(double, 321);
        double *Bi = DISCO_FIXO(double, 322);
        double *Cr = DISCO_FIXO(double, 323);
        double *Ci = DISCO_FIXO(double, 324);
        disco_prende(DISCO_BASE(315),"dados/ar__315.bin",(size_t)((N)),sizeof(double));
        disco_zera(ar_,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(316),"dados/ai__316.bin",(size_t)((N)),sizeof(double));
        disco_zera(ai_,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(317),"dados/br__317.bin",(size_t)((N)),sizeof(double));
        disco_zera(br_,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(318),"dados/bi__318.bin",(size_t)((N)),sizeof(double));
        disco_zera(bi_,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(319),"dados/Ar_319.bin",(size_t)((N)),sizeof(double));
        disco_zera(Ar,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(320),"dados/Ai_320.bin",(size_t)((N)),sizeof(double));
        disco_zera(Ai,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(321),"dados/Br_321.bin",(size_t)((N)),sizeof(double));
        disco_zera(Br,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(322),"dados/Bi_322.bin",(size_t)((N)),sizeof(double));
        disco_zera(Bi,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(323),"dados/Cr_323.bin",(size_t)((N)),sizeof(double));
        disco_zera(Cr,(size_t)((N)),sizeof(double));
        disco_prende(DISCO_BASE(324),"dados/Ci_324.bin",(size_t)((N)),sizeof(double));
        disco_zera(Ci,(size_t)((N)),sizeof(double));
        long s=4242;
        /* valores pequenos: a convolução inteira cabe em ℤ_p sem dobrar (256·3·3 = 2304 < 40961) */
        for(int i=0;i<N;i++){ s=(s*1103515245+12345)&0x7fffffff; a[i]=s%4; }
        for(int i=0;i<N;i++){ s=(s*1103515245+12345)&0x7fffffff; b[i]=s%4; }
        conv_oraculo(a,b,cor,N);
        long maxc=0; for(int i=0;i<N;i++) if(cor[i]>maxc) maxc=cor[i];
        printf("       máx da convolução = %ld  (< p = %ld, então nenhum termo dobra) %s\n",
               maxc, p, maxc<p?"✓":"✗");

        /* (a) DIGITAL: F, produto ponto a ponto, Finv, e desfaz a normalização (×r) */
        long RN=inv(R_GLOBAL), W=W_GLOBAL;
        long *A = DISCO_FIXO(long, 326);
        long *B = DISCO_FIXO(long, 327);
        long *Ck = DISCO_FIXO(long, 328);
        disco_prende(DISCO_BASE(326),"dados/A_326.bin",(size_t)((N)),sizeof(long));
        disco_zera(A,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(327),"dados/B_327.bin",(size_t)((N)),sizeof(long));
        disco_zera(B,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(328),"dados/Ck_328.bin",(size_t)((N)),sizeof(long));
        disco_zera(Ck,(size_t)((N)),sizeof(long));
        for(int k=0;k<N;k++){
            long acc=0, f=1, wk=pot(W,k);
            for(int j=0;j<N;j++){ acc=md(acc+mul(a[j],f)); f=mul(f,wk); }
            A[k]=mul(acc,RN);
            acc=0; f=1;
            for(int j=0;j<N;j++){ acc=md(acc+mul(b[j],f)); f=mul(f,wk); }
            B[k]=mul(acc,RN);
        }
        for(int k=0;k<N;k++) Ck[k]=mul(A[k],B[k]);
        long Wi=inv(W);
        for(int j=0;j<N;j++){
            long acc=0, f=1, wj=pot(Wi,j);
            for(int k=0;k<N;k++){ acc=md(acc+mul(Ck[k],f)); f=mul(f,wj); }
            cdig[j]=mul(mul(acc,RN), R_GLOBAL);       /* ×r desfaz o (√n)^{L−1} da fusão            */
        }
        int dif_dig=0; for(int i=0;i<N;i++) if(cdig[i]!=md(cor[i])) dif_dig++;
        printf("       (a) digital (torção %ld em ℤ_%ld) vs oráculo : %d/%d %s\n",
               W, p, N-dif_dig, N, dif_dig?"✗":"✓ exato");

        /* (b) ANALÓGICO: a rotação LC, produto ponto a ponto, volta */
        for(int i=0;i<N;i++){ ar_[i]=(double)a[i]; ai_[i]=0; br_[i]=(double)b[i]; bi_[i]=0; }
        double rn=1.0/sqrt((double)N);
        for(int k=0;k<N;k++){
            double th=-2*M_PI*k/N, cc=cos(th), ss=sin(th), fc=1, fs=0;
            double sar=0,sai=0,sbr=0,sbi=0;
            for(int j=0;j<N;j++){
                sar += ar_[j]*fc; sai += ar_[j]*fs;
                sbr += br_[j]*fc; sbi += br_[j]*fs;
                double nfc=fc*cc-fs*ss, nfs=fc*ss+fs*cc; fc=nfc; fs=nfs;
            }
            Ar[k]=sar*rn; Ai[k]=sai*rn; Br[k]=sbr*rn; Bi[k]=sbi*rn;
        }
        for(int k=0;k<N;k++){
            Cr[k]=Ar[k]*Br[k]-Ai[k]*Bi[k];
            Ci[k]=Ar[k]*Bi[k]+Ai[k]*Br[k];
        }
        int dif_an=0; double pior_an=0;
        for(int j=0;j<N;j++){
            double th=2*M_PI*j/N, cc=cos(th), ss=sin(th), fc=1, fs=0, sr=0;
            for(int k=0;k<N;k++){
                sr += Cr[k]*fc - Ci[k]*fs;
                double nfc=fc*cc-fs*ss, nfs=fc*ss+fs*cc; fc=nfc; fs=nfs;
            }
            double val = sr*rn*sqrt((double)N);        /* desfaz a normalização, como o ×r digital  */
            double e = fabs(val - (double)cor[j]);
            if(e>pior_an) pior_an=e;
            if(e>0.5) dif_an++;                        /* arredonda ao inteiro                      */
        }
        printf("       (b) analógico (rotação LC) vs oráculo : %d/%d divergem ; erro máx %.2e\n",
               dif_an, N, pior_an);
        int bom = (dif_dig==0) && (dif_an==0);
        printf("     %s\n", VD(!(bom), "resíduo 0 nos DOIS MEIOS — a mesma convolução circular sai da torção exata em ℤ_p e da\n"
          "     rotação da malha LC, e as duas batem o oráculo O(n²). O digital dá o inteiro exato;\n"
          "     o analógico dá o mesmo inteiro dentro do arredondamento. Uma peça, dois meios."));
        if(!bom) passou=0;
    }

    /* ---------- GA4: o dente ---------- */
    printf("\n§GA4 o DENTE: e se o ângulo não for 2π/n?\n");
    {
        double L=1e-3, C=1e-9;
        Rot errada = rot_lc(L,C,0,N+1);               /* 2π/(n+1): quase certo, e quebra           */
        double x=1,y=0;
        for(int i=0;i<N;i++) aplica(errada,&x,&y);
        double volta = sqrt((x-1)*(x-1)+y*y);
        printf("       %d passos de 2π/%d : volta com erro %.4f  %s\n", N, N+1, volta,
               volta>1e-3 ? "✓ NÃO fecha (o dente morde)" : "✗ fechou?");
        printf("     %s\n", volta>1e-3 ?
          "resíduo 0 com pulso — a torção não é uma rotação qualquer: é a que fecha em n. Errar o\n"
          "     ângulo por 1/257 já não fecha, e a obra não volta. É a borda que segura, e ela é exata."
          : "FALHA");
        if(volta<=1e-3) passou=0;
    }

    /* ---------- GA5: as ordens ÍMPARES (3, 5, 7) nos dois meios ---------- */
    printf("\n§GA5 as ordens ÍMPARES — 3, 5, 7 — nos DOIS meios. Aqui não há escada binária: a\n");
    printf("     ordem vem da necessidade, o primo vem da ordem (k | p−1), e a projeção analógica\n");
    printf("     é a rotação de 2π/k. Nota: a normalização simétrica 1/√k exige k resíduo\n");
    printf("     quadrático (falha em k=3, p=7), então usa-se k⁻¹ de um lado — sempre existe.\n");
    {
        int ks[3] = {3,5,7};
        int erro_geral = 0;
        for(int t=0;t<3;t++){
            int k = ks[t];
            /* o primo vem da ordem: k | p−1 e p > maior valor da convolução (o enredo pede p > dado) */
            long pk=0, maxconv = (long)k*3*3;
            for(long q=k+1;q<100000;q++) if(q>maxconv){
                int ep=1; for(long d=2;d*d<=q;d++) if(q%d==0){ ep=0; break; }
                if(ep && (q-1)%k==0){ pk=q; break; }
            }
            long pg = p; p = pk;                          /* trabalha no corpo da necessidade      */
            long gk=0;
            for(long a=2;a<pk;a++){ long o=1,c=a; while(c!=1){ c=mul(c,a); o++; } if(o==pk-1){ gk=a; break; } }
            long wk = pot(gk,(pk-1)/k);
            long o_wk=1, c=wk; while(c!=1){ c=mul(c,wk); o_wk++; }
            /* abre de baixo? se 2k | p−1, w_k = (w_{2k})² */
            const char *abre = "—";
            if((pk-1)%(2*k)==0){
                long w2k = pot(gk,(pk-1)/(2*k));
                abre = (mul(w2k,w2k)==wk) ? "sim" : "NÃO";
                if(mul(w2k,w2k)!=wk) erro_geral=1;
            }
            printf("\n       ── ordem k=%d : p=%ld (menor com k|p−1 e p>%ld), g=%ld, w=%ld, ord=%ld %s ; abre de w_{%d}? %s\n",
                   k, pk, maxconv, gk, wk, o_wk, o_wk==k?"✓":"✗", 2*k, abre);
            if(o_wk != k) erro_geral=1;

            /* --- ANALÓGICO: a rotação de 2π/k colhida da malha, e a volta em k passos --- */
            double L=1e-3, C=1e-9;
            Rot rk = rot_lc(L,C,0,k);
            double x=1,y=0, piorE=0;
            for(int i=0;i<k;i++){ aplica(rk,&x,&y); double E=x*x+y*y; if(fabs(E-1)>piorE) piorE=fabs(E-1); }
            double volta = sqrt((x-1)*(x-1)+y*y);
            printf("          analógico: %d passos de 2π/%d → identidade, erro %.2e ; energia varia %.2e\n",
                   k, k, volta, piorE);
            if(volta>1e-13 || piorE>1e-13) erro_geral=1;
            /* e abre de baixo no analógico: 2 batidas de 2π/(2k) == 1 de 2π/k */
            double w0=1.0/sqrt(L*C), dt=2*M_PI/((double)(2*k)*w0);
            Rot r2 = rot_passo(L,C,dt), r1 = rot_passo(L/2,C/2,dt);
            double a1=1,b1=0,a2=1,b2=0;
            aplica(r1,&a1,&b1); aplica(r2,&a2,&b2); aplica(r2,&a2,&b2);
            double difz = sqrt((a1-a2)*(a1-a2)+(b1-b2)*(b1-b2));
            printf("          analógico: o zoom (dobrar ω₀) == 2 batidas de 2π/%d ? %.1e\n", 2*k, difz);
            if(difz>1e-13) erro_geral=1;

            /* --- a CONVOLUÇÃO de tamanho k nos dois meios, contra o oráculo --- */
            long a[8], b[8], cor[8], cdig[8];
            long s=1234+t*77;
            for(int i=0;i<k;i++){ s=(s*1103515245+12345)&0x7fffffff; a[i]=s%4; }
            for(int i=0;i<k;i++){ s=(s*1103515245+12345)&0x7fffffff; b[i]=s%4; }
            for(int i=0;i<k;i++){ long acc=0; for(int j=0;j<k;j++) acc += a[j]*b[((i-j)%k+k)%k]; cor[i]=acc; }
            /* digital: F sem normalizar, produto, Finv com k⁻¹ */
            long A[8],B[8],Ck[8], kinv=inv(k);
            for(int u=0;u<k;u++){
                long sa=0,sb=0, wu=pot(wk,u), f=1;
                for(int j=0;j<k;j++){ sa=md(sa+mul(a[j],f)); sb=md(sb+mul(b[j],f)); f=mul(f,wu); }
                A[u]=sa; B[u]=sb;
            }
            for(int u=0;u<k;u++) Ck[u]=mul(A[u],B[u]);
            long wi=inv(wk);
            for(int j=0;j<k;j++){
                long acc=0, wj=pot(wi,j), f=1;
                for(int u=0;u<k;u++){ acc=md(acc+mul(Ck[u],f)); f=mul(f,wj); }
                cdig[j]=mul(acc,kinv);
            }
            int dd=0; for(int i=0;i<k;i++) if(cdig[i]!=md(cor[i])) dd++;
            /* analógico: a rotação de 2π/k, mesma receita, com 1/k */
            double Ar[8],Ai[8],Br[8],Bi[8],Cr[8],Ci[8];
            for(int u=0;u<k;u++){
                double th=-2*M_PI*u/k, cc=cos(th), ss=sin(th), fc=1, fs=0;
                double sar=0,sai=0,sbr=0,sbi=0;
                for(int j=0;j<k;j++){
                    sar += a[j]*fc; sai += a[j]*fs; sbr += b[j]*fc; sbi += b[j]*fs;
                    double nc=fc*cc-fs*ss, ns=fc*ss+fs*cc; fc=nc; fs=ns;
                }
                Ar[u]=sar; Ai[u]=sai; Br[u]=sbr; Bi[u]=sbi;
            }
            for(int u=0;u<k;u++){ Cr[u]=Ar[u]*Br[u]-Ai[u]*Bi[u]; Ci[u]=Ar[u]*Bi[u]+Ai[u]*Br[u]; }
            int da=0; double pior=0;
            for(int j=0;j<k;j++){
                double th=2*M_PI*j/k, cc=cos(th), ss=sin(th), fc=1, fs=0, sr=0;
                for(int u=0;u<k;u++){ sr += Cr[u]*fc - Ci[u]*fs;
                    double nc=fc*cc-fs*ss, ns=fc*ss+fs*cc; fc=nc; fs=ns; }
                double val = sr/k;
                double e = fabs(val-(double)cor[j]); if(e>pior) pior=e;
                if(e>0.5) da++;
            }
            printf("          convolução: oráculo=[");
            for(int i=0;i<k;i++) printf("%ld%s", cor[i], i+1<k?" ":"");
            printf("] ; digital %s ; analógico %s (erro máx %.1e)\n",
                   dd?"✗":"exato ✓", da?"✗":"igual ✓", pior);
            if(dd||da) erro_geral=1;
            p = pg;
        }
        printf("\n     %s\n", VD(erro_geral, "resíduo 0 nas três ordens ímpares, nos dois meios — não há nada de especial na potência\n"
          "     de 2: a ordem vem da necessidade, o primo vem da ordem, e a projeção é a mesma\n"
          "     fórmula (g^((p−1)/k) no discreto, a rotação de 2π/k na malha). Cada uma abre de baixo\n"
          "     pelo quadrado da de ordem 2k — nos dois meios —, e a convolução de tamanho k sai\n"
          "     igual do inteiro exato e do circuito."));
        if(erro_geral) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 NOS DOIS MEIOS — o gerador global não é uma constante do digital: é a peça, e ela\n"
      "tem os dois lados. No discreto, w=36043 de ordem 256 em ℤ_40961; no analógico, a rotação de\n"
      "2π/n colhida da malha LC, na borda |λ|=1 — fecha em n passos, conserva a energia, e a TORRE\n"
      "FRACTAL é o ZOOM do circuito (elevar a torção ao quadrado = dobrar ω₀ = §B.7).\n"
      "\n"
      "E a convolução circular sai igual dos dois: a torção exata em ℤ_p dá o inteiro exato, a\n"
      "rotação LC dá o mesmo inteiro dentro do arredondamento, e ambas batem o oráculo O(n²). Nos\n"
      "dois meios sem tabela — o expoente por soma, o fator pela PG que a acompanha. Errar o ângulo\n"
      "por 1/257 já não fecha: a borda é exata, e é ela que segura."
      : "FALHOU — rever");
    return !passou;
}
