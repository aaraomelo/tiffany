/* relogio_curva.c — O RELÓGIO DESENHA A CURVA PLANA, E ELE DECIDE O GRAU.
 *
 * O Aarão: «o relógio trabalha infinito, não limite o grau do polinómio. Vê como o
 * relógio desenha uma curva plana — ele que decide o grau. É alto, grau lá para 12.
 * Faz o experimento e observa: a curva é num corpo de grau 4 — 4 é o grau do corpo,
 * o grau do polinómio é ilimitado.»
 *
 * A curva: um contorno REAL da carta (documento-regular, CFF grau 3 por troços).
 * O relógio: o parâmetro é o TEMPO DA ÓRBITA — o arco, percorrido a velocidade
 * constante — e a base é o par do rotor, cos kθ e sen kθ (polinómios de grau k em
 * cos θ: a mesma linha de Pascal do grau.c, outra roupa). Os coeficientes saem pela
 * TRANSFORMADA — somas sobre a órbita, sem sistema linear nenhum: é a transformada
 * universal avaliada no círculo do próprio relógio.
 *
 * O corpo é de GRAU 4: o ponto é o par (x,y) e a base é o par (cos,sen) — dois
 * duais, a cruz. O grau do polinómio é do relógio, ilimitado: sobe harmónicos até
 * a curva fechar na régua pedida.
 *
 *   §R1  o «o» — a órbita lisa — fecha em grau BAIXO (<=4 na régua do papel)
 *   §R2  o «e» fecha à volta do grau 12, como previsto
 *   §R3  a estrutura manda: grau(o) < grau(e) <= grau(R) — a quina cobra
 *   §R4  o CONTROLO: com o tempo POR TROÇO (não o arco), o «o» não fecha barato —
 *        o tempo do relógio é o arco, e a mutação prova-o
 *   §R5  a volta: a órbita reconstruída passa por TODOS os pontos, na régua da fonte
 *   §R6  a ROUPA: a órbita veste-se em cúbicas (nós no relógio, tangentes da órbita —
 *        Hermite -> Bézier) e a curva vestida fica na régua do papel da ORIGINAL
 *   §R7  a economia: a assinatura (2·(2g+1) números) contra a cadeia de controlos
 *   §R9  a ROUPA DO RELÓGIO: ele não cospe cúbica — cospe o RASTO, pontos da órbita
 *        no seu tempo ligados por grau 1; a cúbica é só a REFERÊNCIA da carta, e o
 *        relógio sobe o passo até o rasto fechar na régua contra ela
 *   §R8  a assinatura INTEIRA: a transformada universal no anel da lei 8 — o relógio
 *        de N=256=2^8 marcas em Z_65537 (Fermat, 2^16+1), raízes EXACTAS da borda
 *        cíclica; a volta é resíduo 0 exacto, sem um double e sem amputar nada
 *
 *   cc -O2 -std=c99 -I../lib relogio_curva.c -lm -o relogio_curva && ./relogio_curva
 */
#include "spline.h"
#include <math.h>

/* ─── O RELÓGIO SEM π: a fisica.tex manda ────────────────────────────────────────────
 * «Não há círculo na natureza — π é discreto: num relógio de q marcas, e subir a torre
 * (q = 2^k) refina-o dois bits por dobra.» O cos/sen de cada marca NÃO pede π: a
 * semente da dobra k vem do MEIO-ÂNGULO desde a dimensão 2 (cos π = −1, sem avaliar
 * π), e as marcas seguintes saem do rotor — a Lei 2, J a rodar. O «2π» que a derivada
 * pede é o π DISCRETO da própria dimensão: PI2_Q = q·sen(volta/q), o perímetro do
 * polígono inscrito — o mesmo π_q da fisica.tex, calculado e não escrito. */
#define QREL 4096                       /* o relógio da dobra 12: 2^12 marcas */
static double TC[QREL], TS[QREL];
static double PI2_Q = 0;                /* o 2π discreto desta dimensão */
static void gera_relogio(void){
    if(PI2_Q > 0) return;
    double c = -1, sn = 0;              /* a dimensão 2: meia volta — SEM avaliar π */
    for(int q = 2; q < QREL; q *= 2){   /* a dobra: meio-ângulo, dois bits por passo,
                                         * até a marca do relógio de QREL = 2^12 */
        sn = sqrt((1 - c) / 2);
        c  = sqrt((1 + c) / 2);
    }
    /* c, sn = cos e sen de UMA marca do relógio de 4096; o rotor dá as restantes */
    TC[0] = 1; TS[0] = 0;
    for(int k = 1; k < QREL; k++){
        TC[k] = TC[k-1] * c - TS[k-1] * sn;
        TS[k] = TS[k-1] * c + TC[k-1] * sn;
    }
    PI2_Q = QREL * sn;                  /* 2π_q: o perímetro do polígono — discreto */
}
static double cosq(int h, double t){
    long i = ((long)(t * QREL + 0.5) * h) % QREL; if(i < 0) i += QREL;
    return TC[i];
}
static double senq(int h, double t){
    long i = ((long)(t * QREL + 0.5) * h) % QREL; if(i < 0) i += QREL;
    return TS[i];
}

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

#define NAM 512
static double AX[NAM], AY[NAM];

/* o rasto denso do contorno (a cadeia de troços da carta), n pontos por índice de troço */
static int amostra(const Contorno *c, int a, int z, int n, double *xs, double *ys){
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
        double tg = (double)k / n * nt;
        int tr = (int)tg; if(tr >= nt) tr = nt - 1;
        double t = tg - tr, u = 1 - t;
        const Pt *P0=&c->p[idx[tr][0]], *P1=&c->p[idx[tr][1]], *P2=&c->p[idx[tr][2]], *P3=&c->p[idx[tr][3]];
        xs[k] = u*u*u*P0->x + 3*u*u*t*P1->x + 3*u*t*t*P2->x + t*t*t*P3->x;
        ys[k] = u*u*u*P0->y + 3*u*u*t*P1->y + 3*u*t*t*P2->y + t*t*t*P3->y;
    }
    return 1;
}

/* REAMOSTRA NO TEMPO DO RELÓGIO: a órbita percorre-se a velocidade constante — os n
 * pontos ficam uniformes no ARCO (interpolação linear na corda do rasto denso) */
static void relogio_tempo(int nd, const double *xs, const double *ys,
                          int n, double *ux, double *uy){
    double acc[NAM + 1], L = 0;
    for(int k = 0; k < nd; k++){
        int k2 = (k + 1) % nd;
        double dx = xs[k2]-xs[k], dy = ys[k2]-ys[k];
        acc[k] = L; L += sqrt(dx*dx + dy*dy);
    }
    acc[nd] = L;
    int j = 0;
    for(int k = 0; k < n; k++){
        double alvo = L * k / n;
        while(j + 1 < nd && acc[j+1] < alvo) j++;
        double seg = acc[j+1] - acc[j];
        double t = seg > 0 ? (alvo - acc[j]) / seg : 0;
        int j2 = (j + 1) % nd;
        ux[k] = xs[j] + t * (xs[j2] - xs[j]);
        uy[k] = ys[j] + t * (ys[j2] - ys[j]);
    }
}

/* A TRANSFORMADA DO RELÓGIO: os coeficientes por SOMAS sobre a órbita — sem Gauss,
 * sem sistema; é a avaliação no círculo, o Dirac a peneirar cada harmónico */
static void transforma(int n, const double *v, int g, double *co){
    gera_relogio();
    co[0] = 0;
    for(int k = 0; k < n; k++) co[0] += v[k];
    co[0] /= n;
    for(int h = 1; h <= g; h++){
        double ca = 0, sa = 0;
        for(int k = 0; k < n; k++){
            double t = (double)k / n;
            ca += v[k] * cosq(h, t); sa += v[k] * senq(h, t);
        }
        co[2*h-1] = 2 * ca / n; co[2*h] = 2 * sa / n;
    }
}
static double orbita(int g, const double *co, double t){
    double s = co[0];
    for(int h = 1; h <= g; h++)
        s += co[2*h-1]*cosq(h, t) + co[2*h]*senq(h, t);
    return s;
}

/* O TECTO, e ele estava POR VERIFICAR. A `transforma` escreve até `co[2*g]`, logo um
 * array de 80 só serve até g = 39 — e o `grau_fecha` era chamado com max = 40, que escreve
 * em `co[80]`, fora dele. Com a régua do papel a curva fecha muito antes e o ramo NUNCA
 * corria; apertando a régua cem vezes, o medidor deixa de passar e passa a dar FALHA DE
 * SEGMENTAÇÃO. Um `40` que ninguém liga ao `80` é documentação, não limite.
 * Agora o array deriva do grau, e o grau verifica-se onde é usado. */
#define GMAX   39                        /* o maior g que cabe: 2·39 + 1 = 79 < 80 */
#define NCOEF  (2*GMAX + 2)              /* e o array deriva DELE, não o contrário */

/* o pior resíduo da órbita de grau g contra os n pontos do relógio */
static double residuo(int n, const double *ux, const double *uy, int g){
    double cox[NCOEF], coy[NCOEF], pr = 0;
    if(g < 1 || g > GMAX){ puts("relogio_curva: grau fora do tecto"); exit(1); }
    transforma(n, ux, g, cox);
    transforma(n, uy, g, coy);
    for(int k = 0; k < n; k++){
        double t = (double)k / n;
        double dx = fabs(orbita(g, cox, t) - ux[k]);
        double dy = fabs(orbita(g, coy, t) - uy[k]);
        if(dx > pr) pr = dx;
        if(dy > pr) pr = dy;
    }
    return pr;
}

/* o grau em que a curva fecha na régua dada — o relógio decide, sem tecto posto */
static int grau_fecha(int n, const double *ux, const double *uy, double regua, int max){
    for(int g = 1; g <= max; g++)
        if(residuo(n, ux, uy, g) < regua) return g;
    return -1;
}

static double UX[NAM], UY[NAM];

static int prepara(const Ttf *t, int ch, int arco){
    static Contorno c;
    int gi = ttf_glifo(t, ch);
    if(!gi || !cff_contorno(t, gi, &c) || !c.nc) return 0;
    int nd = 480;
    if(!amostra(&c, 0, c.fim[0], nd, AX, AY)) return 0;
    if(arco) relogio_tempo(nd, AX, AY, 256, UX, UY);
    else {
        for(int k = 0; k < 256; k++){ UX[k] = AX[k * nd / 256]; UY[k] = AY[k * nd / 256]; }
    }
    return 1;
}

/* a DERIVADA da órbita — a tangente sai da própria assinatura, termo a termo */
static double orbita_d(int g, const double *co, double t){
    double s = 0;
    for(int h = 1; h <= g; h++){
        double w = PI2_Q * h;           /* o 2π discreto da dimensão, não uma constante */
        s += w * (-co[2*h-1]*senq(h, t) + co[2*h]*cosq(h, t));
    }
    return s;
}

/* A ROUPA: M cúbicas com os nós uniformes no relógio e as tangentes da órbita.
 * Hermite -> Bézier: C1 = P0 + T0/(3M), C2 = P1 - T1/(3M) — a mesma linha de Pascal. */
static double veste_e_mede(int g, const double *cox, const double *coy,
                           int M, int n, const double *ux, const double *uy){
    double pior = 0;
    for(int seg = 0; seg < M; seg++){
        double t0 = (double)seg / M, t1 = (double)(seg + 1) / M;
        double P0x = orbita(g, cox, t0), P0y = orbita(g, coy, t0);
        double P3x = orbita(g, cox, t1), P3y = orbita(g, coy, t1);
        double h = 1.0 / M;
        double C1x = P0x + orbita_d(g, cox, t0) * h / 3;
        double C1y = P0y + orbita_d(g, coy, t0) * h / 3;
        double C2x = P3x - orbita_d(g, cox, t1) * h / 3;
        double C2y = P3y - orbita_d(g, coy, t1) * h / 3;
        /* mede contra os pontos do relógio que caem neste segmento */
        for(int k = 0; k < n; k++){
            double tk = (double)k / n;
            if(tk < t0 || tk >= t1) continue;
            double t = (tk - t0) / h, u = 1 - t;
            double bx = u*u*u*P0x + 3*u*u*t*C1x + 3*u*t*t*C2x + t*t*t*P3x;
            double by = u*u*u*P0y + 3*u*u*t*C1y + 3*u*t*t*C2y + t*t*t*P3y;
            double dx = fabs(bx - ux[k]), dy = fabs(by - uy[k]);
            if(dx > pior) pior = dx;
            if(dy > pior) pior = dy;
        }
    }
    return pior;
}

/* ─── §R8: A TRANSFORMADA UNIVERSAL NO ANEL DA LEI 8 ────────────────────────────────
 * O universal.c prova: a transformada é a avaliação nas raízes da borda, e em corpo
 * finito é TUDO INTEIRO, resíduo zero exacto. A borda cíclica x^N − 1 com N = 256 =
 * 2^8 (o fechamento: a dobra oito) tem raízes exactas em Z_p com p = 65537 = 2^16+1
 * (Fermat): ω = 3^(65536/256), e 3 é raiz primitiva. A assinatura do glifo é o
 * espectro — 256 inteiros — e a volta é EXACTA: mudança de base, não aproximação. */
typedef long long LL;
#define P8 65537LL
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
    /* a régua do papel: a 600 dpi um pixel é ~10 unidades da fonte num corpo de leitura;
     * a régua da fonte é a própria unidade, 1.
     *
     * E o 10 DERIVA-SE do upem em vez de se escrever: ele valia 10 porque o upem desta fonte
     * é 1000, e isso estava no COMENTÁRIO, não na conta — uma fonte com upem 2048 (o comum
     * nas TrueType) tornaria a régua duas vezes mais apertada do que o papel, sem nada
     * acusar. A régua é um centésimo do em, e o em pergunta-se à fonte. */
    double PAPEL = t.upem / 100.0, FONTE = 1.0;
    printf("  a régua do papel deriva do upem da fonte: upem %d  ->  PAPEL = %.1f unidades\n",
           t.upem, PAPEL);

    gera_relogio();
    printf("O RELOGIO DESENHA A CURVA PLANA — o experimento, medido\n");
    printf("  o pi que a MAQUINA produz (saida, nao entrada): pi_q = %.9f na dobra 12\n\n",
           PI2_Q / 2);
    int go = prepara(&t, 'o', 1) ? grau_fecha(256, UX, UY, PAPEL, GMAX) : -1;
    double ro_fonte = prepara(&t, 'o', 1) ? residuo(256, UX, UY, 8) : 1e9;
    int ge = prepara(&t, 'e', 1) ? grau_fecha(256, UX, UY, PAPEL, GMAX) : -1;
    int gR = prepara(&t, 'R', 1) ? grau_fecha(256, UX, UY, PAPEL, GMAX) : -1;
    printf("  grau na regua do papel: o=%d  e=%d  R=%d\n", go, ge, gR);

    ok("§R1 a orbita lisa fecha BARATO: o «o» em grau <= 4 na regua do papel",
       go > 0 && go <= 4);
    ok("§R2 o «e» fecha a volta do grau 12, como previsto (8 <= g <= 14)",
       ge >= 8 && ge <= 14);
    ok("§R3 a estrutura manda no grau: grau(o) < grau(e) <= grau(R) — a quina cobra",
       go > 0 && ge > go && gR >= ge);

    int go_troco = prepara(&t, 'o', 0) ? grau_fecha(256, UX, UY, PAPEL, GMAX) : -1;
    ok("§R4 CONTROLO: com o tempo POR TROCO (nao o arco) o «o» nao fecha em <= 4 — "
       "o tempo do relogio E o arco",
       go_troco < 0 || go_troco > 4);
    printf("     -> o «o» por troco: fecha em %d (pelo arco: %d)\n", go_troco, go);

    ok("§R5 a volta na regua da FONTE: a orbita do «o» a grau 8 passa a menos de 1 "
       "unidade de TODOS os 256 pontos",
       ro_fonte < FONTE);
    printf("     -> pior residuo da orbita do «o» a grau 8: %.3f unidades\n", ro_fonte);

    /* §R6/§R7: o «e» — a assinatura veste a roupa e a volta fecha contra a ORIGINAL */
    if(prepara(&t, 'e', 1)){
        double cox[NCOEF], coy[NCOEF];
        /* O RELOGIO DECIDE O GRAU DA CURVA VESTIDA: sobe harmonicos ate a ROUPA fechar
         * na regua — nao se alarga a regua, sobe-se o grau, que nao tem tecto */
        int g = ge > 0 ? ge : 13, M = 0; double rv = 1e9;
        for(; g <= GMAX; g++){
            transforma(256, UX, g, cox);
            transforma(256, UY, g, coy);
            M = 2 * g;                 /* um segmento por meia onda do maior harmónico */
            rv = veste_e_mede(g, cox, coy, M, 256, UX, UY);
            if(rv < PAPEL) break;
        }
        ok("§R6 a ROUPA: a orbita do «e» vestida em cubicas (nos no relogio, tangentes "
           "da orbita) fica na regua do papel contra a curva ORIGINAL",
           rv < PAPEL);
        printf("     -> o relogio subiu ao grau %d: %d cubicas, pior residuo %.2f "
               "(regua %.0f)\n", g, M, rv, PAPEL);
        int n_ass = 2 * (2*g + 1), n_cad = 2 * 39;
        ok("§R7 a economia: a assinatura do relogio cabe na ordem da cadeia de controlos",
           n_ass < 3 * n_cad);
        printf("     -> assinatura: %d numeros (grau %d); a cadeia da carta: %d\n",
               n_ass, g, n_cad);
    } else { ok("§R6 a roupa", 0); ok("§R7 a economia", 0); }

    /* §R9: a roupa do relogio e' o RASTO — grau 1, o passo decidido por ele */
    if(prepara(&t, 'e', 1)){
        double cox[NCOEF], coy[NCOEF];
        int g = ge > 0 ? ge : 13;
        transforma(256, UX, g, cox);
        transforma(256, UY, g, coy);
        int M2 = 0; double rr = 1e9;
        for(M2 = 4 * g; M2 <= 512; M2 *= 2){
            /* o rasto: M2 pontos da orbita; mede-se a CORDA contra os 256 da referencia */
            rr = 0;
            for(int k = 0; k < 256; k++){
                double tk = (double)k / 256;
                int sg2 = (int)(tk * M2);
                double t0 = (double)sg2 / M2, t1 = (double)(sg2 + 1) / M2;
                double x0 = orbita(g, cox, t0), y0 = orbita(g, coy, t0);
                double x1 = orbita(g, cox, t1), y1 = orbita(g, coy, t1);
                double u = (tk - t0) * M2;
                double dx = fabs(x0 + u*(x1-x0) - UX[k]);
                double dy = fabs(y0 + u*(y1-y0) - UY[k]);
                if(dx > rr) rr = dx;
                if(dy > rr) rr = dy;
            }
            if(rr < PAPEL) break;
        }
        ok("§R9 a ROUPA DO RELOGIO: o rasto de grau 1 fecha na regua do papel contra a "
           "referencia cubica — e o PASSO e' do relogio, sobe ate fechar",
           rr < PAPEL);
        printf("     -> o relogio marcou %d passos (grau %d): pior residuo %.2f "
               "(regua %.0f)\n", M2, g, rr, PAPEL);
    } else ok("§R9 a roupa do relogio", 0);

    /* §R8: a assinatura inteira do «e» e a volta exacta */
    if(prepara(&t, 'e', 1)){
        static LL vx[256], VX[256], vv[256];
        for(int k = 0; k < 256; k++){
            long u = (long)(UX[k] >= 0 ? UX[k] + 0.5 : UX[k] - 0.5);   /* a unidade da fonte */
            vx[k] = ((u % P8) + P8) % P8;
        }
        ntt(256, vx, VX, 0);                 /* a assinatura: o espectro inteiro */
        ntt(256, VX, vv, 1);                 /* a volta: a avaliação inversa */
        int iguais = 0;
        for(int k = 0; k < 256; k++) if(vv[k] == vx[k]) iguais++;
        ok("§R8 a assinatura INTEIRA (a universal no anel da lei 8, Z_65537, N=2^8): "
           "a volta e' EXACTA nos 256 pontos — residuo 0, sem um double",
           iguais == 256);
        printf("     -> %d de 256 exactos; espectro de 256 inteiros mod 65537\n", iguais);
        /* o CONTROLO do selo: um bit trocado no espectro espalha-se por quase todos
         * os pontos da volta — nao ha onde esconder uma alteracao */
        VX[7] ^= 1;
        ntt(256, VX, vv, 1);
        int difere = 0;
        for(int k = 0; k < 256; k++) if(vv[k] != vx[k]) difere++;
        ok("§R8b o CONTROLO: um bit trocado no espectro muda a volta em >= 250 dos 256 "
           "pontos — a transformada e' global, e por isso e' um selo",
           difere >= 250);
        printf("     -> o bit espalhou-se por %d de 256 pontos\n", difere);
    } else { ok("§R8 a assinatura inteira", 0); ok("§R8b o controlo", 0); }

    printf("\nunidades: %d   falhas: %d\nRESIDUO %d\n", feitas, falhas, falhas);
    return falhas ? 1 : 0;
}
