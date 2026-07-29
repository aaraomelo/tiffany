/* micro.c — MICROPROCESSADOR FRACTAL, o discreto: as afirmações num arquivo só.
 *
 * BASE HISTÓRICA. O gabarito (microprocessador.tex, refatorado) é agora o ANALÓGICO —
 * a colheita vive em analog.c (o translinear, §B.8–B.10). Este mede o corpo no DISCRETO
 * (10 seções): útil como oráculo, mas "discretização é inútil" — o corpo vive no contínuo.
 * Cada seção mede uma afirmação contra um oráculo externo e devolve resíduo 0 ou falha.
 *
 *   cc -O2 micro.c -lm -o micro && ./micro          (tudo)
 *   ./micro 5                                        (só a §A.5)
 *
 * AS PEÇAS (Parte I) são quatro, e delas sai tudo:
 *   meio-somador  A_m = [[m,1],[1,0]]   (Fibonacci)  — o ⊕ e a fração contínua
 *   rotação       G   = [[0,1],[-1,0]]  (simplética) — o deslocamento e o oscilador
 *   diodo         log/exp                            — o ∏ = exp∘Σ∘log
 *   disco         o arquivo é o grafo                — o estado
 *
 * O QUE ESTE ARQUIVO SUBSTITUI. Antes eram 10 programas + api.h (54 KB), com
 * redundância medida: G^4=I calculado duas vezes idêntico; ADD-ripple implementado três
 * vezes (api.h, confere_micro, add_ripple); w^n=1 varrido em três; os convergentes de
 * Fibonacci empilhados em dois; e 15 dos 30 símbolos do api.h mortos (Fourier, Mellin,
 * Parseval, Dirac, Clifford/La Hire/Pontryagin em double, o motor u64, NOT/OR/NAND/NOR/
 * XNOR/SUB). Aqui cada peça existe UMA vez e cada seção a chama.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

typedef uint64_t u64;
typedef int64_t  i64;

/* ==========================================================================
 * PARTE I — AS PEÇAS (só o que o micro usa; uma implementação de cada)
 * ========================================================================== */

/* --- a matriz 2x2: o meio-somador A_m, a rotação G, e o produto (La Hire) --- */
typedef struct { i64 a, b, c, d; } Mat;

static Mat MEIO(i64 m)   { Mat r = {m, 1, 1, 0}; return r; }   /* A_m: o meio-somador */
static Mat ROT(void)     { Mat r = {0, 1, -1, 0}; return r; }  /* G: a rotação 90°    */
static Mat mul_mat(Mat X, Mat Y) {
    Mat r = { X.a*Y.a + X.b*Y.c, X.a*Y.b + X.b*Y.d,
              X.c*Y.a + X.d*Y.c, X.c*Y.b + X.d*Y.d }; return r;
}
static i64 det_mat(Mat X) { return X.a*X.d - X.b*X.c; }

/* --- o meio-somador em bits: XOR = soma sem carry, AND = carry gerado --- */
static u64 XOR(u64 x, u64 y) { return x ^ y; }
static u64 AND(u64 x, u64 y) { return x & y; }
static u64 SHL(u64 x)        { return x << 1; }   /* a rotação: move o carry */

/* --- ADD: meio-somador ∘ rotação, iterado. UMA implementação; o contador de
       tiques é opcional (t=NULL quando não interessa). É a única soma do arquivo. --- */
static u64 ADD_t(u64 x, u64 y, int *t) {
    int n = 0;
    while (y) { u64 s = XOR(x, y); u64 c = SHL(AND(x, y)); x = s; y = c; n++; }
    if (t) *t = n;
    return x;
}
static u64 ADD(u64 x, u64 y) { return ADD_t(x, y, NULL); }

/* --- MUL: shift-and-add, sobre a mesma ADD --- */
static u64 MUL(u64 x, u64 y) {
    u64 r = 0;
    while (y) { if (y & 1) r = ADD(r, x); x = SHL(x); y >>= 1; }
    return r;
}

/* --- o diodo: log/exp. NOTA HONESTA: I_s = 1 e V_T = 1 implícitos, logo
       exp(log a + log b) = a·b exato. Isto mede a aritmética log-domain, NÃO o
       circuito físico — num transistor real sobra um 1/I_s (~1e14) e o somador Σ
       precisa de um terceiro termo -V_ref. Ver so_cristal.cir e §9 do paper. --- */
static double d_log(double I) { return log(I); }
static double d_exp(double V) { return exp(V); }
static double an_mul(double a, double b) { return d_exp(d_log(a) + d_log(b)); }
static double an_som(double a, double b) { return a + b; }   /* Kirchhoff: correntes no nó */

/* --- o corpo finito: a torção w de ordem n (o relógio, o controle) --- */
typedef struct { u64 p, g, n, w; } Univ;

/* precondição: p < 2^32, logo a·b < 2^64 e não é preciso __int128 (p=40961 aqui). */
static u64 u_mul(const Univ *U, u64 a, u64 b) { return (a * b) % U->p; }
static u64 u_pot(const Univ *U, u64 b, u64 e) {
    u64 r = 1; b %= U->p;
    while (e) { if (e & 1) r = u_mul(U, r, b); b = u_mul(U, b, b); e >>= 1; }
    return r;
}
static u64 u_inv(const Univ *U, u64 a) { return u_pot(U, a, U->p - 2); }
static Univ corpo(u64 p, u64 g, u64 n) {
    Univ U; U.p = p; U.g = g; U.n = n; U.w = u_pot(&U, g, (p - 1) / n); return U;
}
#define P_CORPO 40961ULL   /* primo com 2^13 | p-1: a torre n=2..8192 cabe inteira */
#define G_CORPO 3ULL

/* --- a fração contínua: ADC (subtrai-e-flipa) e DAC (produto de meio-somadores) --- */
static int adc(i64 p, i64 q, i64 *a) {          /* p/q -> [a0;a1,...] sem '/' nem '%' */
    int n = 0;
    while (q) {
        i64 ak = 0, r = p;
        while (r >= q) { r -= q; ak++; }        /* o quociente por subtração */
        a[n++] = ak; p = q; q = r;              /* o flip: a escala 1/x */
    }
    return n;
}
static void dac(const i64 *a, int n, i64 *p, i64 *q) {   /* [a_k] -> p/q */
    Mat M = {1,0,0,1};
    for (int k = 0; k < n; k++) M = mul_mat(M, MEIO(a[k]));
    *p = M.a; *q = M.c;
}

/* --- utilitário de relatório. Alinha por CARACTERE, não por byte: o texto tem
       UTF-8 (⊕ ⊗ ∏ → ² ⁴ ⁻¹ φ) e "%-46s" contaria os bytes, torcendo a coluna. --- */
static int falhas = 0;
static void col(const char *s, int largura) {
    int n = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++)
        if ((*p & 0xC0) != 0x80) n++;            /* conta só os inícios de code point */
    fputs(s, stdout);
    for (int i = n; i < largura; i++) putchar(' ');
}
static void veredito(const char *sec, const char *o_que, int ok, const char *medida) {
    printf("  "); col(sec, 6); col(o_que, 55); col(medida, 26);
    printf("%s\n", ok ? "resid 0" : "FALHOU");
    if (!ok) falhas++;
}

/* ==========================================================================
 * PARTE II — AS SEÇÕES (uma por afirmação do paper)
 * ========================================================================== */

/* §A.1 — O NÚCLEO: a tríade nos diodos. ⊕ e ⊗ analógicos == a ISA. */
static void A1_nucleo(void) {
    printf("\n§A.1  O NÚCLEO — a tríade nos diodos (⊕ Kirchhoff, ⊗ log-domain)\n");
    long som_ok = 0, mul_ok = 0, tot = 0;
    for (i64 a = 1; a <= 200; a++) for (i64 b = 1; b <= 200; b++) {
        if ((u64)llround(an_som((double)a,(double)b)) == ADD((u64)a,(u64)b)) som_ok++;
        if ((u64)llround(an_mul((double)a,(double)b)) == MUL((u64)a,(u64)b)) mul_ok++;
        tot++;
    }
    printf("     log(12)+log(7) = %.4f ; exp = %.0f = 84 (o ∏ = exp∘Σ∘log, e o log desfaz)\n",
           log(12.0)+log(7.0), an_mul(12,7));
    char m1[64], m2[64];
    snprintf(m1, sizeof m1, "%ld/%ld pares", som_ok, tot);
    snprintf(m2, sizeof m2, "%ld/%ld pares", mul_ok, tot);
    veredito("A.1", "⊕ analógico (correntes no nó) == ISA ADD", som_ok == tot, m1);
    veredito("A.1", "⊗ analógico (log-domain) == ISA MUL",      mul_ok == tot, m2);
}

/* §A.2 — O RELÓGIO: a torção para TODO n. Fecha (w^n=1) e reverte (w·w^-1=1). */
static void A2_relogio(void) {
    printf("\n§A.2  O RELÓGIO — a torção para todo n (a torre inteira, sem n fixo)\n");
    int viol = 0, escalas = 0;
    for (u64 n = 2; n <= 8192; n *= 2) {
        Univ U = corpo(P_CORPO, G_CORPO, n);
        u64 acc = 1;
        for (u64 t = 0; t < n; t++) acc = u_mul(&U, acc, U.w);   /* w^n */
        if (acc != 1 || u_mul(&U, U.w, u_inv(&U, U.w)) != 1) viol++;
        escalas++;
    }
    char m[64]; snprintf(m, sizeof m, "n=2..8192 (%d escalas)", escalas);
    veredito("A.2", "fecha (w^n=1) e reverte (w·w⁻¹=1) em toda escala", !viol, m);
}

/* §A.3 — O OSCILADOR É O CÁLCULO: a rotação G^4=I oscila, e o mesmo <<1 ripplea o
   carry. ADD/MUL contra o oráculo NATIVO (o + e o * do C) — oráculo distinto do §A.1,
   que compara analógico contra ISA. */
static void A3_oscilador(void) {
    printf("\n§A.3  O OSCILADOR É O CÁLCULO — G⁴=I oscila; o mesmo <<1 ripplea o carry\n");
    Mat G = ROT(), P = {1,0,0,1};
    for (int k = 0; k < 4; k++) P = mul_mat(P, G);               /* G^4 — calculado UMA vez */
    int g4 = (P.a==1 && P.b==0 && P.c==0 && P.d==1);
    printf("     G⁴ = [[%lld,%lld],[%lld,%lld]] = I  (gap-4, |λ|=1: oscila sem decair; det G = %lld)\n",
           (long long)P.a,(long long)P.b,(long long)P.c,(long long)P.d,(long long)det_mat(G));
    printf("     meio-somador em bits: [0,0]->(0,0) [0,1]->(1,0) [1,0]->(1,0) [1,1]->(0,1)  (XOR,AND)\n");
    long viol = 0, tot = 0; int tmax = 0;
    for (u64 a = 0; a < 200; a++) for (u64 b = 0; b < 200; b++) {
        int t; if (ADD_t(a,b,&t) != a + b) viol++;
        if (t > tmax) tmax = t;
        if (MUL(a,b) != a * b) viol++;
        tot++;
    }
    char m[64]; snprintf(m, sizeof m, "%ld pares, tiques máx %d", tot, tmax);
    veredito("A.3", "G⁴=I (a rotação fecha em 4)", g4, "gap-4");
    veredito("A.3", "ADD e MUL == nativo (o tique ripplea um estágio)", !viol, m);
}

/* §A.4 — O CORAÇÃO DUAL: a defasagem áurea minimiza o maior vão, em TODA escala.
   Inteiros no núcleo (delta = k/M exato); float só no relatório. */
static i64 gap_max(i64 k, i64 M, i64 N) {
    static i64 fase[4096];
    if (N > 4096) N = 4096;
    i64 acc = 0;
    for (i64 j = 0; j < N; j++) { fase[j] = acc; acc += k; while (acc >= M) acc -= M; }
    for (i64 a = 1; a < N; a++) {                    /* insertion sort no círculo */
        i64 v = fase[a], b = a - 1;
        while (b >= 0 && fase[b] > v) { fase[b+1] = fase[b]; b--; }
        fase[b+1] = v;
    }
    i64 g = 0;
    for (i64 a = 1; a < N; a++) { i64 d = fase[a] - fase[a-1]; if (d > g) g = d; }
    i64 wrap = (fase[0] + M) - fase[N-1];
    return wrap > g ? wrap : g;
}
static void A4_coracao(void) {
    printf("\n§A.4  O CORAÇÃO DUAL — por que a defasagem é áurea, não π\n");
    const i64 M = 2584;                              /* F_18: a grade fina */
    const i64 esc[] = {8,13,21,34,55,89,144,233};    /* as escalas de Fibonacci */
    const int ne = 8;
    printf("     π (δ=1/2, antifase): vão máx %.0f/1000 do ciclo  <- meio ciclo morto\n",
           1000.0*gap_max(1292,M,300)/M);
    printf("     áureo (δ=1/φ)      : vão máx %.2f/1000 do ciclo\n",
           1000.0*gap_max(1597,M,300)/M);
    /* varre TODA defasagem k/M, minimizando o pior vão NORMALIZADO (g·N/M) sobre as
       escalas — é a normalização que faz a áurea vencer; sem ela o argmin é M/8. */
    double melhor = 1e18; i64 kbest = -1; int empates = 0;
    for (i64 k = 1; k < M; k++) {
        double pior = 0;
        for (int e = 0; e < ne; e++) {
            double disc = (double)gap_max(k,M,esc[e]) * (double)esc[e] / (double)M;
            if (disc > pior) pior = disc;
        }
        if (pior < melhor - 1e-12) { melhor = pior; kbest = k; empates = 1; }
        else if (pior < melhor + 1e-12) empates++;
    }
    double frac = (double)kbest / (double)M, iphi = 0.6180339887498949;
    double d1 = fabs(frac - iphi), d2 = fabs((1.0 - frac) - iphi);
    int aureo = (d1 < 0.01 || d2 < 0.01);
    /* o argmin é degenerado por reflexão: gap(k) = gap(M-k) sempre. Os DOIS vencedores
       são 987/2584 = 1/φ² e 1597/2584 = 1/φ — o mesmo ponto espelhado, e mais nenhum. */
    int reflexo = (gap_max(987,M,233) == gap_max(1597,M,233)) && (987 + 1597 == M);
    printf("     vencedor: %lld/%lld = %.5f  (= 1/φ² ; o espelho %lld/%lld = 1/φ dá idêntico)\n",
           (long long)kbest, (long long)M, frac, (long long)(M-kbest), (long long)M);
    char m[64]; snprintf(m, sizeof m, "%ld defasagens", (long)(M-1));
    veredito("A.4", "o argmin da varredura é a defasagem áurea", aureo, m);
    veredito("A.4", "e é o par refletido {1/φ, 1/φ²}, e nada mais", reflexo && empates == 2, "gap(k)=gap(M−k)");
}

/* §A.5 — A SOMA NASCE DO PAR: o traço do ripple, e o certificado em 8 bits. */
static void bits4(u64 x) { for (int i = 3; i >= 0; i--) putchar((x>>i)&1 ? '1':'0'); }
static void A5_ripple(void) {
    printf("\n§A.5  A SOMA NASCE DO PAR — ADD = meio-somador ∘ rotação, iterado\n");
    printf("     7+1 (4 bits):  tique |  a (soma parc.)  b (carry<<1)\n");
    u64 a = 7, b = 1; int t = 0;
    while (b) { printf("                     %4d  |  ", t); bits4(a); printf("            "); bits4(b);
                printf("\n"); u64 s = XOR(a,b), c = SHL(AND(a,b)); a = s; b = c; t++; }
    printf("                     %4d  |  ", t); bits4(a); printf("            "); bits4(b);
    printf("   <- carry 0: para, soma = %llu\n", (unsigned long long)a);
    printf("     tiques = propagação do vai-um: 2+1→1, 7+1→4, 15+1→5, 255+1→9 (não um n fixo)\n");
    long viol = 0, tot = 0; int tmax = 0;
    for (u64 x = 0; x < 256; x++) for (u64 y = 0; y < 256; y++) {
        int k; if (ADD_t(x,y,&k) != x + y) viol++;
        if (k > tmax) tmax = k;
        tot++;
    }
    char m[64]; snprintf(m, sizeof m, "%ld pares, tiques máx %d", tot, tmax);
    veredito("A.5", "a iteração bate a soma nativa em todo par de 8 bits", !viol, m);
}

/* §A.6 — A MEMÓRIA É O DISCO: streaming reversível, RAM constante. */
static u64 rl7(u64 x) { return (x << 7) | (x >> 57); }
static void fluxo(const char *in, const char *out, int inverso, u64 s) {
    FILE *fi = fopen(in,"rb"), *fo = fopen(out,"wb"); int b;
    if (!fi || !fo) { if(fi)fclose(fi); if(fo)fclose(fo); return; }
    while ((b = fgetc(fi)) != EOF) {                 /* 1 palavra na mão, 1 byte por vez */
        int r = (b ^ (int)(s & 0xFF)) & 0xFF;
        s = rl7(s) ^ (u64)(inverso ? b : r);         /* a contração reversível */
        fputc(r, fo);
    }
    fclose(fi); fclose(fo);
}
static int mesmo(const char *x, const char *y) {
    FILE *fa = fopen(x,"rb"), *fb = fopen(y,"rb"); int p, q, ok = 1;
    if (!fa || !fb) { if(fa)fclose(fa); if(fb)fclose(fb); return 0; }
    do { p = fgetc(fa); q = fgetc(fb); if (p != q) { ok = 0; break; } } while (p != EOF);
    fclose(fa); fclose(fb); return ok;
}
static void A6_memoria(void) {
    printf("\n§A.6  A MEMÓRIA É O DISCO — sem registradores, 1 palavra na mão\n");
    const char *O = "micro_tmp.bin", *C = "micro_tmp.cif", *R = "micro_tmp.rec";
    size_t tam[] = {1024, 65536, 1048576};
    int viol = 0;
    for (int i = 0; i < 3; i++) {
        FILE *f = fopen(O,"wb"); if (!f) { viol++; continue; }
        u64 x = 0x2545F4914F6CDD1DULL;
        for (size_t j = 0; j < tam[i]; j++) { x = x*6364136223846793005ULL + 1; fputc((int)(x>>56), f); }
        fclose(f);
        fluxo(O,C,0,0x9E3779B97F4A7C15ULL);          /* contrai  */
        fluxo(C,R,1,0x9E3779B97F4A7C15ULL);          /* reconstrói */
        if (!mesmo(O,R)) viol++;
        printf("     %7zu B no disco -> RAM de trabalho 1 palavra (8 B), reverte\n", tam[i]);
    }
    remove(O); remove(C); remove(R);
    veredito("A.6", "RAM constante e a volta reconstrói o arquivo", !viol, "1 KB / 64 KB / 1 MB");
}

/* §A.7 — O BARRAMENTO É O ⊕: abeliano, e a árvore P2P == a soma plana. */
static u64 soma_arvore(const u64 *v, int n) {
    if (n == 1) return v[0];
    int m = n / 2;
    return ADD(soma_arvore(v,m), soma_arvore(v+m,n-m));   /* cada nó é um ⊕ */
}
static void A7_barramento(void) {
    printf("\n§A.7  O BARRAMENTO É O ⊕ — o nó de Kirchhoff, em árvore P2P\n");
    u64 f[8] = {11,22,33,44,55,66,77,88};
    u64 plana = 0, nativo = 0, rev = 0;
    for (int i = 0; i < 8; i++) { plana = ADD(plana,f[i]); nativo += f[i]; }
    for (int i = 7; i >= 0; i--) rev = ADD(rev,f[i]);                       /* comuta  */
    u64 g1 = ADD(ADD(f[0],f[1]),ADD(f[2],f[3]));                           /* associa */
    u64 g2 = ADD(f[0],ADD(f[1],ADD(f[2],f[3])));
    int viol = 0;
    for (int n = 1; n <= 8; n++) {
        u64 v[8], p = 0;
        for (int i = 0; i < n; i++) v[i] = (u64)((i*37+13) % 100);
        for (int i = 0; i < n; i++) p = ADD(p,v[i]);
        if (soma_arvore(v,n) != p) viol++;
    }
    printf("     8 fontes no nó: %llu (nativo %llu) ; árvore log N níveis: %llu\n",
           (unsigned long long)plana,(unsigned long long)nativo,
           (unsigned long long)soma_arvore(f,8));
    veredito("A.7", "⊕ é abeliano (reordenar e reagrupar não muda)", rev==plana && g1==g2, "comuta e associa");
    veredito("A.7", "a árvore P2P == a soma plana (sem barramento central)", !viol && plana==nativo, "N=1..8");
}

/* §A.8 — O I/O: a fração contínua, sem 2^k. DAC(ADC(p/q)) = p/q em todo ramo. */
static void A8_io(void) {
    printf("\n§A.8  O I/O — a fração contínua (o peso é a escala 1/x, não 2^k)\n");
    i64 pi4[] = {3,7,15,1}, p, q;
    dac(pi4,4,&p,&q);
    printf("     [3;7,15,1] -> %lld/%lld (o convergente de π) ; det = ±1 em todo ramo\n",
           (long long)p,(long long)q);
    long viol = 0, tot = 0;
    for (i64 x = 1; x <= 200; x++) for (i64 y = 1; y <= 200; y++) {
        i64 a[64]; int n = adc(x,y,a); i64 pr, qr; dac(a,n,&pr,&qr);
        if (pr*y != qr*x) viol++;                 /* oráculo: pr/qr == x/y */
        tot++;
    }
    i64 uns[16]; for (int k = 0; k < 16; k++) uns[k] = 1;
    int vfib = 0;
    for (int n = 3; n <= 12; n++) {               /* [1;1,1,...] -> Fibonacci -> φ */
        i64 pn,qn,pa,qa,pb,qb;
        dac(uns,n,&pn,&qn); dac(uns,n-1,&pa,&qa); dac(uns,n-2,&pb,&qb);
        if (pn != pa + pb) vfib++;
    }
    dac(uns,12,&p,&q);
    printf("     [1;1,1,...] -> Fibonacci: convergente 12 = %lld/%lld, a razão -> φ (nenhum 2^k)\n",
           (long long)p,(long long)q);
    char m[64]; snprintf(m, sizeof m, "%ld frações", tot);
    veredito("A.8", "DAC(ADC(p/q)) = p/q reduzido (a ponte é exata)", !viol, m);
    veredito("A.8", "a recorrência áurea p_n = p_{n-1} + p_{n-2}", !vfib, "convergentes 3..12");
}

/* §A.9 — O CONTROLE: a roda de opcodes, o PC, o despacho ao ALU.
   NOTA: o PC é o índice na roda (pc++); a torção Z/n de ordem n=|programa| é a mesma
   medida em §A.2 e não se repete aqui. O paper (§8) afirma que o PC avança POR w —
   isso não está implementado: o despacho é medido, a torção do PC não. */
enum { LDI, ADDI, MULI, HALT };
typedef struct { int op; u64 arg; } Instr;
static u64 roda(const Instr *prog, int n, int *passos) {
    u64 acc = 0; int pc = 0, t = 0;
    while (pc < n && prog[pc].op != HALT) {
        switch (prog[pc].op) {
            case LDI:  acc = prog[pc].arg;        break;
            case ADDI: acc = ADD(acc, prog[pc].arg); break;   /* ⊕ */
            case MULI: acc = MUL(acc, prog[pc].arg); break;   /* ⊗ */
        }
        pc++; t++;
    }
    if (passos) *passos = t;
    return acc;
}
static void A9_controle(void) {
    printf("\n§A.9  O CONTROLE — a roda de opcodes (o PC), despachando ao ALU\n");
    Instr p1[] = {{LDI,3},{ADDI,4},{MULI,5},{HALT,0}};
    Instr p2[] = {{LDI,10},{MULI,10},{ADDI,1},{MULI,2},{HALT,0}};
    int s1, s2;
    u64 r1 = roda(p1,4,&s1), r2 = roda(p2,5,&s2);
    printf("     [LDI 3; ADD 4; MUL 5]      -> %llu (esperado 35)  ; %d passos\n",
           (unsigned long long)r1, s1);
    printf("     [LDI 10; MUL 10; ADD 1; MUL 2] -> %llu (esperado 202) ; %d passos\n",
           (unsigned long long)r2, s2);
    long viol = 0, tot = 0;
    for (u64 a = 0; a < 40; a++) for (u64 b = 0; b < 40; b++) for (u64 c = 0; c < 40; c++) {
        Instr pr[] = {{LDI,a},{ADDI,b},{MULI,c},{HALT,0}};
        if (roda(pr,4,NULL) != (a+b)*c) viol++;
        tot++;
    }
    char m[64]; snprintf(m, sizeof m, "%ld programas", tot);
    veredito("A.9", "os programas de exemplo dão 35 e 202", r1==35 && r2==202, "2 programas");
    veredito("A.9", "(a+b)·c bate o nativo, varrendo o cubo", !viol, m);
}

/* §A.10 — A FRACTALIDADE MEDIDA: o mapa de escala explícito entre níveis. */
static void A10_fractal(void) {
    printf("\n§A.10 A FRACTALIDADE — o mapa de escala, medido (não afirmado)\n");
    int viol = 0, trans = 0;
    for (u64 n = 4; n <= 8192; n *= 2) {
        Univ Un = corpo(P_CORPO,G_CORPO,n), Uh = corpo(P_CORPO,G_CORPO,n/2);
        int bad = (u_mul(&Un,Un.w,Un.w) != Uh.w);        /* w(n)² == w(n/2) */
        u64 an = 1, ah = 1;
        for (u64 k = 0; k < n/2; k++) {                  /* e a órbita conjuga */
            an = u_mul(&Un, u_mul(&Un,an,Un.w), Un.w);   /* +2 tiques no nível n   */
            ah = u_mul(&Uh, ah, Uh.w);                   /* +1 tique  no nível n/2 */
            if (an != ah) bad = 1;
        }
        if (bad) viol++;
        trans++;
    }
    Mat M = {1,0,0,1}, Mp; int vz = 0;
    for (int k = 1; k <= 12; k++) {                      /* o I/O: M_k carrega M_{k-1} */
        Mp = M; M = mul_mat(M, MEIO(1));
        if (M.b != Mp.a || M.d != Mp.c) vz++;            /* a 2ª coluna É o nível de baixo */
    }
    char m[64]; snprintf(m, sizeof m, "%d transições", trans);
    veredito("A.10", "o zoom w→w² conjuga o nível n com o n/2 (relógio)", !viol, m);
    veredito("A.10", "M_k = M_{k-1}·A_1: o nível k carrega o k-1 (I/O)",  !vz, "convergentes 1..12");
}

/* ========================================================================== */

int main(int argc, char **argv) {
    struct { const char *nome; void (*f)(void); } S[] = {
        {"1",A1_nucleo}, {"2",A2_relogio}, {"3",A3_oscilador}, {"4",A4_coracao},
        {"5",A5_ripple}, {"6",A6_memoria}, {"7",A7_barramento}, {"8",A8_io},
        {"9",A9_controle}, {"10",A10_fractal},
    };
    const int NS = (int)(sizeof S / sizeof S[0]);

    printf("═══ MICROPROCESSADOR FRACTAL — o apêndice, no metal ═══\n");
    printf("as peças: meio-somador A_m, rotação G, diodo log/exp, disco. tudo sai delas.\n");

    int rodou = 0;
    for (int i = 0; i < NS; i++)
        if (argc < 2 || !strcmp(argv[1], S[i].nome)) { S[i].f(); rodou++; }

    if (!rodou) { fprintf(stderr, "uso: %s [1..10]\n", argv[0]); return 2; }

    printf("\n═══════════════════════════════════════════════════════════════════════════\n");
    if (falhas == 0)
        printf(" %d seção(ões), RESÍDUO 0 — a tríade (⊕ ⊗ ∏) e o disco, em toda escala.\n", rodou);
    else
        printf(" %d FALHA(S) em %d seção(ões).\n", falhas, rodou);
    return falhas ? 1 : 0;
}
