/* relogio_curva.c — O RELÓGIO DESENHA A CURVA PLANA, E ELE DECIDE O GRAU.
 *
 * O Aarão: «o relógio trabalha infinito, não limite o grau do polinómio. Vê como o
 * relógio desenha uma curva plana — ele que decide o grau. É alto, grau lá para 12.»
 *
 * O relógio geométrico: q = 4096 = 2^12 marcas. cos/sen de cada marca NÃO pede π: a
 * semente da dobra vem do MEIO-ÂNGULO desde cos π = −1, com a raiz mais próxima em
 * escala inteira, e as marcas seguintes saem do rotor (Lei 2). A transformada são
 * somas sobre a órbita. §R8 é a NTT em Z_65537, N=256=2^8 — volta exacta, sem um double.
 *
 *   cc -O2 -std=c99 -I../lib relogio_curva.c -o relogio_curva && ./relogio_curva
 */
#include "spline.h"

#define NAM   512
#define NCLK  256
#define QREL  4096                 /* dobra 12: o mesmo relógio da carta, em Z */
#define SC    1000000000L          /* escala do rotor: cos,sen ∈ [−SC, SC] */
#define SUB   64                   /* a unidade da fonte, em tiques — a Bézier não arredonda cedo */
#define GMAX  39
#define NCOEF (2*GMAX + 2)
#define P8    65537LL

typedef long long LL;
typedef __int128 I128;

static long raiz_piso(long x){
    if(x < 0) return -1;
    if(x < 2) return x;
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return lo;
}
static long raiz_perto(long x){
    if(x <= 0) return 0;
    long r = raiz_piso(x);
    I128 dlo = (I128)x - (I128)r * r;
    I128 dhi = (I128)(r + 1) * (r + 1) - x;
    return (dhi < dlo) ? r + 1 : r;
}

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

static long TC[QREL], TS[QREL];
static long MARCA_S;                 /* sen de uma marca: o π discreto sai daqui */
static int relogio_ok = 0;

static long mulsc(long a, long b){
    I128 p = (I128)a * b;
    if(p >= 0) return (long)((p + SC/2) / SC);
    return (long)((p - SC/2) / SC);
}
static long div_arred(I128 n, I128 d){
    if(d < 0){ n = -n; d = -d; }
    if(n >= 0) return (long)((n + d/2) / d);
    return (long)((n - d/2) / d);
}

static void gera_relogio(void){
    if(relogio_ok) return;
    long c = -SC, sn = 0;                       /* dimensão 2: cos π = −1, sem avaliar π */
    for(int q = 2; q < QREL; q *= 2){
        long c0 = c;
        I128 ac = (I128)SC * (SC + c0) / 2;
        I128 as = (I128)SC * (SC - c0) / 2;
        if(ac < 0) ac = 0; if(as < 0) as = 0;
        c  = raiz_perto((long)ac);
        sn = raiz_perto((long)as);
    }
    MARCA_S = sn;
    TC[0] = SC; TS[0] = 0;
    for(int k = 1; k < QREL; k++){
        long t0 = TC[k-1], s0 = TS[k-1];
        TC[k] = mulsc(t0, c) - mulsc(s0, sn);
        TS[k] = mulsc(s0, c) + mulsc(t0, sn);
    }
    relogio_ok = 1;
}

static long cosq(int h, int k, int n){
    long i = ((LL)k * h) % n; if(i < 0) i += n;
    return TC[i * QREL / n];
}
static long senq(int h, int k, int n){
    long i = ((LL)k * h) % n; if(i < 0) i += n;
    return TS[i * QREL / n];
}

static long AX[NAM], AY[NAM], UX[NAM], UY[NAM];

static long bez(long P0, long P1, long P2, long P3, long tn, long d){
    long un = d - tn;
    LL x = (LL)un*un*un*P0 + 3LL*un*un*tn*P1 + 3LL*un*tn*tn*P2 + (LL)tn*tn*tn*P3;
    LL den = (LL)d*d*d;
    if(x >= 0) return (long)((x + den/2) / den);
    return (long)((x - den/2) / den);
}

static int amostra(const Contorno *c, int a, int z, int n, long *xs, long *ys){
    int nt = 0; int idx[160][4]; int np = z - a + 1;
    int s0 = a; while(s0 <= z && !c->p[s0].onda) s0++;
    if(s0 > z) return 0;
    int i = 0;
    while(i < np && nt < 160){
        int p0 = a + (s0 - a + i) % np;
        int q1 = a + (s0 - a + i + 1) % np;
        int q2 = a + (s0 - a + i + 2) % np;
        int q3 = a + (s0 - a + i + 3) % np;
        if(!c->p[q1].onda){ idx[nt][0]=p0; idx[nt][1]=q1; idx[nt][2]=q2; idx[nt][3]=q3; nt++; i += 3; }
        else { idx[nt][0]=p0; idx[nt][1]=p0; idx[nt][2]=q1; idx[nt][3]=q1; nt++; i += 1; }
    }
    if(!nt) return 0;
    for(int k = 0; k < n; k++){
        long tg_n = (long)k * nt, d = n;
        int tr = (int)(tg_n / d); if(tr >= nt) tr = nt - 1;
        long tn = tg_n - (long)tr * d;
        const Pt *P0=&c->p[idx[tr][0]], *P1=&c->p[idx[tr][1]],
                 *P2=&c->p[idx[tr][2]], *P3=&c->p[idx[tr][3]];
        xs[k] = bez((long)P0->x * SUB, (long)P1->x * SUB,
                    (long)P2->x * SUB, (long)P3->x * SUB, tn, d);
        ys[k] = bez((long)P0->y * SUB, (long)P1->y * SUB,
                    (long)P2->y * SUB, (long)P3->y * SUB, tn, d);
    }
    return 1;
}

static void relogio_tempo(int nd, const long *xs, const long *ys,
                          int n, long *ux, long *uy){
    long acc[NAM + 1], L = 0;
    for(int k = 0; k < nd; k++){
        int k2 = (k + 1) % nd;
        long dx = xs[k2]-xs[k], dy = ys[k2]-ys[k];
        long q = dx*dx + dy*dy;
        acc[k] = L; L += raiz_piso(q < 0 ? 0 : q);
    }
    acc[nd] = L;
    if(L <= 0){ for(int k = 0; k < n; k++){ ux[k]=xs[0]; uy[k]=ys[0]; } return; }
    int j = 0;
    for(int k = 0; k < n; k++){
        long alvo = L * k / n;
        while(j + 1 < nd && acc[j+1] < alvo) j++;
        long seg = acc[j+1] - acc[j];
        int j2 = (j + 1) % nd;
        if(seg <= 0){ ux[k]=xs[j]; uy[k]=ys[j]; continue; }
        long t = alvo - acc[j];
        ux[k] = xs[j] + (long)((LL)t * (xs[j2] - xs[j]) / seg);
        uy[k] = ys[j] + (long)((LL)t * (ys[j2] - ys[j]) / seg);
    }
}

/* co[0] em unidades da fonte; co[ímpar/par] = amplitude · SC (o rotor ainda escala) */
static void transforma(int n, const long *v, int g, long *co){
    gera_relogio();
    I128 s0 = 0;
    for(int k = 0; k < n; k++) s0 += v[k];
    co[0] = div_arred(s0, n);
    for(int h = 1; h <= g; h++){
        I128 ca = 0, sa = 0;
        for(int k = 0; k < n; k++){
            ca += (I128)v[k] * cosq(h, k, n);
            sa += (I128)v[k] * senq(h, k, n);
        }
        co[2*h-1] = div_arred(2 * ca, n);
        co[2*h]   = div_arred(2 * sa, n);
    }
}

static I128 orbita_sc(int g, const long *co, int k, int n){
    I128 s = (I128)co[0] * SC;
    for(int h = 1; h <= g; h++)
        s += ((I128)co[2*h-1]*cosq(h, k, n) + (I128)co[2*h]*senq(h, k, n)) / SC;
    return s;
}
static long orbita(int g, const long *co, int k, int n){
    return div_arred(orbita_sc(g, co, k, n), SC);
}
/* derivada d/dt, t = k/n — o 2π é o perímetro do polígono: QREL·MARCA_S / SC */
static long orbita_d(int g, const long *co, int k, int n){
    I128 acc = 0;
    for(int h = 1; h <= g; h++)
        acc += ((I128)h) * (-(I128)co[2*h-1]*senq(h, k, n)
                            + (I128)co[2*h]*cosq(h, k, n));
    I128 PI2 = (I128)QREL * MARCA_S;
    return div_arred(acc * PI2, (I128)SC * SC * SC);
}

static long veste_e_mede(int g, const long *cox, const long *coy, int M, int n,
                         const long *ux, const long *uy){
    long pior = 0;
    if(M < 1) return 1L << 30;
    for(int seg = 0; seg < M; seg++){
        int k0 = (int)((LL)seg * n / M);
        int k1 = (int)((LL)(seg + 1) * n / M);
        if(k1 <= k0) k1 = k0 + 1;
        int k1m = (k1 >= n) ? 0 : k1;
        long P0x = orbita(g, cox, k0, n), P0y = orbita(g, coy, k0, n);
        long P3x = orbita(g, cox, k1m, n), P3y = orbita(g, coy, k1m, n);
        long C1x = P0x + orbita_d(g, cox, k0, n) / (3 * M);
        long C1y = P0y + orbita_d(g, coy, k0, n) / (3 * M);
        long C2x = P3x - orbita_d(g, cox, k1m, n) / (3 * M);
        long C2y = P3y - orbita_d(g, coy, k1m, n) / (3 * M);
        int k_hi = (k1 > n) ? n : k1;
        long d = k_hi - k0; if(d < 1) d = 1;
        for(int k = k0; k < k_hi; k++){
            int kk = k % n;
            long tn = k - k0;
            long bx = bez(P0x, C1x, C2x, P3x, tn, d);
            long by = bez(P0y, C1y, C2y, P3y, tn, d);
            long dx = bx - ux[kk]; if(dx < 0) dx = -dx;
            long dy = by - uy[kk]; if(dy < 0) dy = -dy;
            if(dx > pior) pior = dx;
            if(dy > pior) pior = dy;
        }
    }
    return pior;
}

static long residuo(int n, const long *ux, const long *uy, int g){
    long cox[NCOEF], coy[NCOEF], pr = 0;
    if(g < 1 || g > GMAX) return 1L << 30;
    transforma(n, ux, g, cox);
    transforma(n, uy, g, coy);
    for(int k = 0; k < n; k++){
        long dx = orbita(g, cox, k, n) - ux[k]; if(dx < 0) dx = -dx;
        long dy = orbita(g, coy, k, n) - uy[k]; if(dy < 0) dy = -dy;
        if(dx > pr) pr = dx;
        if(dy > pr) pr = dy;
    }
    return pr;
}

static int grau_fecha(int n, const long *ux, const long *uy, long regua, int max){
    int teto = max; if(teto > GMAX) teto = GMAX;
    for(int g = 1; g <= teto; g++)
        if(residuo(n, ux, uy, g) < regua) return g;
    return -1;
}

static int prepara(const Ttf *t, int ch, int arco){
    static Contorno c;
    int gi = ttf_glifo(t, ch);
    if(!gi || !cff_contorno(t, gi, &c) || !c.nc) return 0;
    int nd = 480;
    if(!amostra(&c, 0, c.fim[0], nd, AX, AY)) return 0;
    if(arco) relogio_tempo(nd, AX, AY, NCLK, UX, UY);
    else {
        for(int k = 0; k < NCLK; k++){ UX[k] = AX[k * nd / NCLK]; UY[k] = AY[k * nd / NCLK]; }
    }
    return 1;
}

static LL pot_mod(LL b, LL e){
    LL r = 1; b %= P8;
    while(e){ if(e & 1) r = r * b % P8; b = b * b % P8; e >>= 1; }
    return r;
}
static void ntt(int n, const LL *x, LL *X, int inv){
    LL w = pot_mod(3, 65536 / n);
    if(inv) w = pot_mod(w, P8 - 2);
    for(int h = 0; h < n; h++){
        LL s2 = 0, wk = 1, wh = pot_mod(w, h);
        for(int k = 0; k < n; k++){ s2 = (s2 + x[k] * wk) % P8; wk = wk * wh % P8; }
        X[h] = inv ? s2 * pot_mod(n, P8 - 2) % P8 : s2;
    }
}

int main(void){
    Ttf t;
    if(!ttf_abre(&t, "lib/fontes/documento-regular.otf")
       && !ttf_abre(&t, "../lib/fontes/documento-regular.otf")){
        puts("a carta nao esta — e diz-se, em vez de passar em silencio");
        return 0;
    }
    long PAPEL = (long)t.upem / 100 * SUB, FONTE = SUB;
    if(PAPEL < 1) PAPEL = 1;
    gera_relogio();
    printf("  a régua do papel deriva do upem da fonte: upem %d  ->  PAPEL = %ld em  (×%d tiques)\n",
           t.upem, PAPEL / SUB, SUB);
    printf("O RELOGIO DESENHA A CURVA PLANA — rotor em Z, q=%d, escala %ld, sem pi\n\n",
           QREL, SC);

    int go = prepara(&t, 'o', 1) ? grau_fecha(NCLK, UX, UY, PAPEL, GMAX) : -1;
    long ro_fonte = prepara(&t, 'o', 1) ? residuo(NCLK, UX, UY, 8) : (1L<<30);
    int ge = prepara(&t, 'e', 1) ? grau_fecha(NCLK, UX, UY, PAPEL, GMAX) : -1;
    int gR = prepara(&t, 'R', 1) ? grau_fecha(NCLK, UX, UY, PAPEL, GMAX) : -1;
    printf("  grau na regua do papel: o=%d  e=%d  R=%d\n", go, ge, gR);

    ok("§R1 a orbita lisa fecha BARATO: o «o» em grau <= 4 na regua do papel",
       go > 0 && go <= 4);
    ok("§R2 o «e» fecha a volta do grau 12, como previsto (8 <= g <= 14)",
       ge >= 8 && ge <= 14);
    ok("§R3 a estrutura manda no grau: grau(o) < grau(e) <= grau(R) — a quina cobra",
       go > 0 && ge > go && gR >= ge);

    int go_troco = prepara(&t, 'o', 0) ? grau_fecha(NCLK, UX, UY, PAPEL, GMAX) : -1;
    ok("§R4 CONTROLO: com o tempo POR TROCO (nao o arco) o «o» nao fecha em <= 4 — "
       "o tempo do relogio E o arco",
       go_troco < 0 || go_troco > 4);
    printf("     -> o «o» por troco: fecha em %d (pelo arco: %d)\n", go_troco, go);

    int r5 = 0;
    if(prepara(&t, 'o', 1)){
        long cox[NCOEF], coy[NCOEF];
        transforma(NCLK, UX, 8, cox);
        transforma(NCLK, UY, 8, coy);
        r5 = 1;
        for(int k = 0; k < NCLK; k++){
            I128 dx = orbita_sc(8, cox, k, NCLK) - (I128)UX[k] * SC; if(dx < 0) dx = -dx;
            I128 dy = orbita_sc(8, coy, k, NCLK) - (I128)UY[k] * SC; if(dy < 0) dy = -dy;
            if(dx >= (I128)FONTE * SC || dy >= (I128)FONTE * SC) r5 = 0;
        }
        /* o pior em tiques da fonte, para o relato */
        ro_fonte = residuo(NCLK, UX, UY, 8);
    }
    ok("§R5 a volta na regua da FONTE: a orbita do «o» a grau 8 passa a menos de 1 "
       "unidade de TODOS os 256 pontos",
       r5);
    printf("     -> pior residuo da orbita do «o» a grau 8: %ld/%d da unidade da fonte\n",
           ro_fonte, SUB);

    if(prepara(&t, 'e', 1)){
        long cox[NCOEF], coy[NCOEF];
        int g = ge > 0 ? ge : 13, M = 0; long rv = 1L << 30;
        for(; g <= GMAX; g++){
            transforma(NCLK, UX, g, cox);
            transforma(NCLK, UY, g, coy);
            M = 2 * g;
            rv = veste_e_mede(g, cox, coy, M, NCLK, UX, UY);
            if(rv < PAPEL) break;
        }
        ok("§R6 a ROUPA: a orbita do «e» vestida em cubicas (nos no relogio, tangentes "
           "da orbita) fica na regua do papel contra a curva ORIGINAL",
           rv < PAPEL);
        printf("     -> o relogio subiu ao grau %d: %d cubicas, pior residuo %ld/%d em "
               "(regua %ld em)\n", g, M, rv, SUB, PAPEL / SUB);
        int n_ass = 2 * (2*g + 1), n_cad = 2 * 39;
        ok("§R7 a economia: a assinatura do relogio cabe na ordem da cadeia de controlos",
           n_ass < 3 * n_cad);
        printf("     -> assinatura: %d numeros (grau %d); a cadeia da carta: %d\n",
               n_ass, g, n_cad);
    } else { ok("§R6 a roupa", 0); ok("§R7 a economia", 0); }

    if(prepara(&t, 'e', 1)){
        int g = ge > 0 ? ge : 13;
        long cox[NCOEF], coy[NCOEF];
        transforma(NCLK, UX, g, cox);
        transforma(NCLK, UY, g, coy);
        long rr = 1L<<30; int M2 = 0;
        for(M2 = 4 * g; M2 <= 512; M2 *= 2){
            rr = 0;
            for(int k = 0; k < NCLK; k++){
                int s0 = k * M2 / NCLK;
                int k0 = s0 * NCLK / M2, k1 = (s0 + 1) * NCLK / M2;
                if(k1 >= NCLK) k1 = 0;
                long d = k1 - k0; if(d <= 0) d = NCLK - k0;
                long t = k - k0;
                long x0 = orbita(g, cox, k0, NCLK), y0 = orbita(g, coy, k0, NCLK);
                long x1 = orbita(g, cox, k1, NCLK), y1 = orbita(g, coy, k1, NCLK);
                long px = x0 + t * (x1 - x0) / d;
                long py = y0 + t * (y1 - y0) / d;
                long dx = px - UX[k]; if(dx < 0) dx = -dx;
                long dy = py - UY[k]; if(dy < 0) dy = -dy;
                if(dx > rr) rr = dx;
                if(dy > rr) rr = dy;
            }
            if(rr < PAPEL) break;
        }
        ok("§R9 a ROUPA DO RELOGIO: o rasto de grau 1 fecha na regua do papel contra a "
           "referencia — e o PASSO e' do relogio, sobe ate fechar",
           rr < PAPEL);
        printf("     -> o relogio marcou %d passos (grau %d): pior residuo %ld/%d em "
               "(regua %ld em)\n", M2, g, rr, SUB, PAPEL / SUB);
    } else ok("§R9 a roupa do relogio", 0);

    if(prepara(&t, 'e', 1)){
        static LL vx[NCLK], VX[NCLK], vv[NCLK];
        for(int k = 0; k < NCLK; k++){
            LL u = (LL)UX[k] % P8; if(u < 0) u += P8;
            vx[k] = u;
        }
        ntt(NCLK, vx, VX, 0);
        ntt(NCLK, VX, vv, 1);
        int iguais = 0;
        for(int k = 0; k < NCLK; k++) if(vv[k] == vx[k]) iguais++;
        ok("§R8 a assinatura INTEIRA (a universal no anel da lei 8, Z_65537, N=2^8): "
           "a volta e' EXACTA nos 256 pontos — residuo 0, sem um double",
           iguais == NCLK);
        printf("     -> %d de %d exactos; espectro de %d inteiros mod 65537\n",
               iguais, NCLK, NCLK);
        VX[7] ^= 1;
        ntt(NCLK, VX, vv, 1);
        int difere = 0;
        for(int k = 0; k < NCLK; k++) if(vv[k] != vx[k]) difere++;
        ok("§R8b o CONTROLO: um bit trocado no espectro muda a volta em >= 250 dos 256 "
           "pontos — a transformada e' global, e por isso e' um selo",
           difere >= 250);
        printf("     -> o bit espalhou-se por %d de %d pontos\n", difere, NCLK);
    } else { ok("§R8 a assinatura inteira", 0); ok("§R8b o controlo", 0); }

    printf("\nunidades: %d   falhas: %d\nRESIDUO %d\n", feitas, falhas, falhas);
    return falhas ? 1 : 0;
}
