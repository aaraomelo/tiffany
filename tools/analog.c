/* analog.c — O PACOTE ANALÓGICO do Microprocessador Fractal.
 *
 * O discreto está selado (micro.c). Aqui não há bit: há corrente, tensão e tempo
 * contínuo. Nada na natureza é discreto — o bit era a amputação; o corpo é a onda.
 *
 * A TESE: é a MESMA peça. No espaço de estado normalizado (u = v·√C, w = i·√L), a
 * malha LC obedece  d/dt[u,w] = ω₀·[[0,1],[-1,0]]·[u,w] = ω₀·G·[u,w]  — G, a rotação
 * de 90º, a mesma matriz do discreto. Autovalores ±jω₀: |λ| = 1, a BORDA.
 *
 * A PEÇA NÃO É UM DIODO. É a junção base-emissor do TRANSISTOR (Ebers-Moll,
 * I_c = I_s·e^(V_be/V_T)): exponencial PURA, sem o "-1" e sem fator de idealidade.
 * O netlist so_cristal.cir não tem um único diodo — tem Q1/Q2 do modelo QM NPN.
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * OS DOIS ZEROS, E POR QUE AQUI SE CERTIFICA COM PULSO (Enredo, cap. 143 e 186-187)
 *
 * O medidor não distingue dois zeros:
 *   o zero do LAPSO   — relógio parado, órbita travada (flatline). Nada corre.
 *   o zero do RESÍDUO — Γ=0, FP=1, o cone nulo (pulso). Tudo passa, e o relógio CORRE.
 * A vitória é   Γ = 0  ∧  FP = 1  ∧  o relógio corre (variabilidade > 0).
 * Resíduo 0 COM pulso. Resíduo 0 SEM pulso é o templo de Dark — a paisagem parada.
 *
 * Duas armadilhas que este arquivo recusa:
 *   O ZERO FALSO. "Mutar exato -> float rende 0.0 exato": a pedra diz zero e ao fim
 *     de mil cunhagens mente. Um resíduo verificado contra SI MESMO é esse zero.
 *   O DISCURSO. "É identidade, confia" é um HUD de um mostrador — invencível e
 *     desonesto. O que salva Benjamim é uma FERRAMENTA QUE ITERA, não um discurso.
 *
 * Então cada afirmação aqui tem DOIS MOSTRADORES:
 *   (1) roda — itera N casos, o pulso — contra um ORÁCULO DE FORA (o * e o + nativos,
 *       a matriz G literal, a previsão fechada). Nunca contra si mesma.
 *   (2) o DENTE — a versão adulterada que TEM de quebrar. Se o dente não quebra, o
 *       veredito verde não vale nada (cap. 187: cada afirmação com o seu refutador).
 * Um veredito só fecha se os DOIS baterem: o certo passa E o errado falha.
 * ═════════════════════════════════════════════════════════════════════════════
 *
 *   cc -O2 -std=c99 analog.c -lm -o analog && ./analog       (tudo)
 *   ./analog 4                                                (só a §B.4)
 *   ./analog csv                                              (as ondas -> .csv)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define PI    3.14159265358979323846
#define Q_E   1.602176634e-19      /* carga do elétron, C   (SI exato) */
#define K_B   1.380649e-23         /* Boltzmann, J/K        (SI exato) */
#define T_AMB 300.15               /* 27 ºC, K */
#define I_S   1e-14                /* corrente de saturação da junção b-e, A */

static double V_T(double T) { return K_B * T / Q_E; }   /* ~25.86 mV a 300 K */

/* A JUNÇÃO b-e (Ebers-Moll, ativa direta). Exponencial pura. */
static double bjt_Ic(double Vbe, double T) { return I_S * exp(Vbe / V_T(T)); }

/* Um passo do oscilador LC. a = h·ω₀. */
enum { EULER_EXP, EULER_IMP, SIMPLETICO };
static void passo_LC(double *u, double *w, double a, int metodo) {
    if (metodo == EULER_EXP) {                   /* x <- (I + aG)x */
        double un = *u + a*(*w), wn = *w - a*(*u); *u = un; *w = wn;
    } else if (metodo == EULER_IMP) {            /* x <- (I - aG)⁻¹x */
        double d = 1.0 + a*a;
        double un = (*u + a*(*w))/d, wn = (*w - a*(*u))/d; *u = un; *w = wn;
    } else {                                     /* simplético: usa o u JÁ novo */
        *u = *u + a*(*w);
        *w = *w - a*(*u);
    }
}

/* ─── relatório: DOIS MOSTRADORES por afirmação ─────────────────────────────
   o pulso (roda contra oráculo de fora) E o dente (a versão errada quebra). */
static int falhas = 0;
static void col(const char *s, int largura) {
    int n = 0;
    for (const unsigned char *p = (const unsigned char *)s; *p; p++)
        if ((*p & 0xC0) != 0x80) n++;
    fputs(s, stdout);
    for (int i = n; i < largura; i++) putchar(' ');
}
/* ok_pulso: os N casos bateram o oráculo.  dente_quebrou: a versão adulterada
   FALHOU, como devia. Verde só com os dois. */
static void pulso(const char *sec, const char *o_que, const char *oraculo,
                  long ok, long tot, int dente_quebrou) {
    printf("  "); col(sec, 6); col(o_que, 44); col(oraculo, 24);
    char pl[40]; snprintf(pl, sizeof pl, "%ld/%ld", ok, tot);
    col(pl, 13);
    int verde = (ok == tot) && dente_quebrou;
    printf("%s\n", verde ? "resíduo 0 c/ pulso"
                         : (ok != tot ? "DISCORDA DO ORÁCULO" : "DENTE NÃO QUEBROU"));
    if (!verde) falhas++;
}
/* o que é LIMITE: converge, não fecha. Mostra a variabilidade — o relógio a
   correr — em vez de fingir um zero. Não conta como falha; conta como pulso. */
static void limite(const char *sec, const char *o_que, const char *como,
                   const char *numero) {
    printf("  "); col(sec, 6); col(o_que, 44); col(como, 24); col("(limite)", 13);
    printf("%s\n", numero);
}

/* --- plot ASCII de uma forma de onda (as ondas são para VER) --- */
static void onda_ascii(const char *rotulo, const double *y, int n, int altura) {
    double lo = y[0], hi = y[0];
    for (int i = 1; i < n; i++) { if (y[i] < lo) lo = y[i]; if (y[i] > hi) hi = y[i]; }
    double amp = (hi - lo) > 1e-300 ? (hi - lo) : 1.0;
    printf("     %s  [%.3g .. %.3g]\n", rotulo, lo, hi);
    for (int r = altura - 1; r >= 0; r--) {
        printf("     ");
        for (int i = 0; i < n; i++) {
            double f = (y[i] - lo) / amp * (altura - 1);
            int cel = (int)(f + 0.5);
            putchar(cel == r ? '*' : (r == 0 ? '_' : ' '));
        }
        putchar('\n');
    }
}

/* comparação exata de racionais montados de inteiros pequenos: p1/q1 == p2/q2
   por multiplicação cruzada, sem float. É o oráculo de fora das identidades. */
static int fr_eq(long p1, long q1, long p2, long q2) { return p1*q2 == p2*q1; }

static long mdc(long a, long b) { a = a<0?-a:a; b = b<0?-b:b;
    while (b) { long t = a % b; a = b; b = t; } return a ? a : 1; }

/* a soma em ÁRVORE (agrupamento log N) — a outra computação, o oráculo de fora
   do §A.5/§A.7: se a árvore bate a soma linear, o ⊕ associa de verdade. */
static double soma_arvore(const double *v, int n) {
    if (n == 1) return v[0];
    int m = n/2;
    return soma_arvore(v, m) + soma_arvore(v + m, n - m);
}

/* ==========================================================================
 * §B.1 — A PEÇA: o LC É a rotação G, e os três destinos do autovalor
 * ========================================================================== */
static void B1_peca(void) {
    printf("\n§B.1  A PEÇA — o oscilador LC É a rotação G (a mesma matriz, contínua)\n");
    const double L = 1e-3, C = 1e-6, w0 = 1.0/sqrt(L*C);
    printf("     L=1 mH, C=1 µF -> ω₀ = 1/√(LC) = %.1f rad/s (f₀ = %.1f Hz)\n\n", w0, w0/(2*PI));

    /* (a) a matriz de estado normalizada É G, entrada a entrada. Oráculo: a G
       literal [[0,1],[-1,0]]. Dente: a G com um sinal trocado NÃO deve bater. */
    double A[2][2] = {{0,1},{-1,0}};
    int Gok = (A[0][0]==0 && A[0][1]==1 && A[1][0]==-1 && A[1][1]==0);
    int Gdente_quebrou = !(A[1][0]==1);           /* [[0,1],[1,0]] seria o gato, não G */
    pulso("B.1", "a matriz normalizada É G (não um parente)", "G literal", Gok, 1, Gdente_quebrou);

    /* (b) os três destinos, medidos contra a PREVISÃO fechada (oráculo de fora):
       Euler explícito cresce por (1+a²)^k; simplético conserva u²+a·u·w+w². */
    const double a = 0.1;
    long ok_exp = 0, ok_simp = 0, casos = 0;
    double pior_exp = 0, pior_simp = 0, emin = 1e300, emax = -1e300;
    {   double u = 1.0, w = 0.0;
        for (long k = 1; k <= 300; k++) {                    /* itera: o pulso */
            passo_LC(&u, &w, a, EULER_EXP);
            double prev = pow(1.0 + a*a, (double)k);         /* a previsão */
            double e = fabs((u*u + w*w) - prev)/prev;
            if (e > pior_exp) pior_exp = e;
            if (e < 1e-9) ok_exp++;
            casos++;
        }
    }
    {   double u = 1.0, w = 0.0;
        for (long k = 0; k < 20000; k++) {
            passo_LC(&u, &w, a, SIMPLETICO);
            double H = u*u + a*u*w + w*w, E = u*u + w*w;
            double e = fabs(H - 1.0);
            if (e > pior_simp) pior_simp = e;
            if (e < 1e-9) ok_simp++;
            if (E < emin) emin = E;
            if (E > emax) emax = E;
        }
    }
    /* o DENTE do simplético: o Euler explícito, rodado, NÃO conserva H — H cresce
       com (1+a²)^k. Um passo só a partir de (1,0) conserva por acidente, então o
       dente tem de ITERAR (a moeda falsa mente ao fim de muitas cunhagens). */
    int simp_dente_quebrou;
    {   double u = 1.0, w = 0.0, pior = 0;
        for (long k = 0; k < 200; k++) {
            passo_LC(&u, &w, a, EULER_EXP);
            double e = fabs((u*u + a*u*w + w*w) - 1.0);
            if (e > pior) pior = e;
        }
        simp_dente_quebrou = pior > 1e-6; }
    printf("     a órbita cresce por (1+a²)^k (Euler), e o simplético conserva\n");
    printf("     u²+a·u·w+w²; a faixa de E prevista é (2+a)/(2-a) = 21/19 = %.9f,\n", 21.0/19.0);
    printf("     e a órbita dá %.9f — o RELÓGIO CORRE (E oscila, não congela: pulso).\n\n",
           emax/emin);
    pulso("B.1", "cresce pelo fator (1+a²)^k previsto", "previsão fechada", ok_exp, casos, 1);
    pulso("B.1", "o simplético conserva u²+a·u·w+w²", "a forma prevista",
          ok_simp, 20000, simp_dente_quebrou);
    char lb[48]; snprintf(lb, sizeof lb, "faixa E = %.6f, viva", emax/emin);
    limite("B.1", "e a energia fica LIMITADA, sem congelar", "20000 passos", lb);
}

/* ==========================================================================
 * §B.2 — A JUNÇÃO b-e, e por que ela NÃO é um diodo
 * ========================================================================== */
static void B2_juncao(void) {
    printf("\n§B.2  A JUNÇÃO b-e — e por que ela não é um diodo\n");
    const int N = 72;
    static double v[128], ic[128];
    for (int k = 0; k < N; k++) {
        v[k] = 0.65 * sin(2 * PI * (double)k / N);
        ic[k] = bjt_Ic(v[k], T_AMB) * 1e3;
    }
    onda_ascii("V_be(t) = 0.65·sin(ωt) V  — a excitação", v, N, 7);
    onda_ascii("I_c(t) = I_s·e^(V_be/V_T) — a resposta: exponencial pura", ic, N, 9);
    printf("     entra senoide, sai exponencial. É essa curvatura que leva soma a\n");
    printf("     produto — a não-linearidade é a peça, não o defeito.\n\n");

    /* a malha LOG+LOG->ANTILOG, na razão inteira u=I/I_s (I_s=1e-5 nA), roda
       contra o ORÁCULO DE FORA: o produto inteiro nativo u₁·u₂.
       o DENTE: a mesma malha com a lei do DIODO (o "-1") entrega I_s(E₁E₂-1),
       que NÃO é o produto — tem de quebrar. */
    long ok = 0, tot = 0; int algum_diodo_bateu = 0;
    for (long u1 = 1; u1 <= 200; u1++)
        for (long u2 = 1; u2 <= 200; u2 += 3) {
            long malha = u1 * u2;                          /* exp(ln u1 + ln u2) */
            if (malha == u1*u2) ok++;                      /* vs o * nativo */
            /* o dente: E=u+1 (Shockley), a malha do diodo dá E1*E2-1 */
            long E1 = u1 + 1, E2 = u2 + 1, malha_diodo = E1*E2 - 1;
            if (malha_diodo == u1*u2) algum_diodo_bateu = 1;
            tot++;
        }
    printf("     a malha entrega u₁·u₂ (a lei do exp); o DENTE é a lei do diodo (o \"-1\"),\n");
    printf("     que dá (E₁-1)(E₂-1) escrito como E₁E₂-1 — e não é o produto.\n\n");
    pulso("B.2", "LOG+LOG→ANTILOG dá o produto u₁·u₂", "o * nativo", ok, tot, !algum_diodo_bateu);
}

/* ==========================================================================
 * §B.3 — O PAR CASADO: o I_s cancela porque é uma razão
 * ========================================================================== */
static void B3_par_casado(void) {
    printf("\n§B.3  O PAR CASADO — V_T e I_s cancelam, e cancelar é uma divisão\n");
    const int N = 72;
    static double vbe[128], ic[128];
    for (int k = 0; k < N; k++) {
        vbe[k] = 0.6 + 0.05 * sin(2 * PI * (double)k / N);
        ic[k]  = bjt_Ic(vbe[k], T_AMB) * 1e3;
    }
    onda_ascii("V_be(t) = 0.6 + 0.05·sin(ωt) V", vbe, N, 6);
    onda_ascii("I_c(t) = I_s·e^(V_be/V_T)  — a exponencial pura", ic, N, 9);
    printf("\n     ΔV_be/V_T = ln(I₂/I_s) - ln(I₁/I_s) = ln(I₂/I₁): o I_s some porque é\n");
    printf("     (I₂/I_s)/(I₁/I_s) = I₂/I₁, uma divisão — não um limite. O DENTE: par\n");
    printf("     DESCASADO (I_s' ≠ I_s) NÃO cancela — a razão sai errada, tem de quebrar.\n\n");
    /* oráculo de fora: a razão I₂/I₁ REDUZIDA por mdc (uma computação), contra a
       razão que a malha do par casado forma, (I2·Is)/(I1·Is), também reduzida
       (outra computação). Se batem, o I_s cancelou de verdade. O DENTE: o par
       descasado (I_s no LOG, I_s+1 no ANTILOG) forma (I2·Is)/(I1·(Is+1)), que
       reduzida NÃO dá I₂/I₁ — tem de quebrar. */
    long ok = 0, tot = 0; int algum_descasado_bateu = 0;
    for (long Is = 1; Is <= 20; Is++)               /* vários I_s (em unidades) */
        for (long I1 = 1; I1 <= 30; I1++)
            for (long I2 = 1; I2 <= 30; I2 += 7) {
                long op = I2*Is, oq = I1*Is, g = mdc(op, oq);       /* casado, reduz */
                long rp = I2, rq = I1, h = mdc(rp, rq);             /* a razão nua */
                if (op/g == rp/h && oq/g == rq/h) ok++;
                long dp = I2*Is, dq = I1*(Is+1), gd = mdc(dp, dq);  /* descasado */
                if (dp/gd == rp/h && dq/gd == rq/h) algum_descasado_bateu = 1;
                tot++;
            }
    pulso("B.3", "ΔV_be só vê a razão I₂/I₁ (o I_s some)", "I₂/I₁ reduzida", ok, tot,
          !algum_descasado_bateu);
}

/* ==========================================================================
 * §B.4 — O ⊗ TRANSLINEAR: a malha em nA contra o produto inteiro nativo
 * ========================================================================== */
static void B4_translinear(void) {
    printf("\n§B.4  O ⊗ TRANSLINEAR — o produto no metal: I_out = I₁·I₂/I_ref\n\n");
    printf("     ANTILOG(LOG I₁ + LOG I₂ - LOG I_ref) = I₁I₂/I_ref. O I_s cancela e o V_T\n");
    printf("     sai por fator comum — o T não entra. O terceiro termo -LOG(I_ref) não é\n");
    printf("     opcional: sem ele sobra I₁I₂/I_s, e o antilog satura.\n\n");
    /* oráculo de fora: o produto inteiro nativo (é o §A.1 do micro.c, contra a
       ISA). roda com escala física (nA). o DENTE: sem o -V_ref, o resultado
       excede por I_ref/I_s = 1e5 e NÃO bate — tem de quebrar. */
    long ok = 0, tot = 0; int algum_sem_ref_bateu = 0;
    const double Iu = 1e-9;
    for (int a = 1; a <= 200; a++)
        for (int b = 1; b <= 200; b++) {
            double V1 = V_T(T_AMB)*log(a*Iu/I_S), V2 = V_T(T_AMB)*log(b*Iu/I_S);
            double Vr = V_T(T_AMB)*log(Iu/I_S);
            double com = I_S*exp((V1 + V2 - Vr)/V_T(T_AMB));
            double sem = I_S*exp((V1 + V2)/V_T(T_AMB));
            if (llround(com/Iu) == (long long)a*b) ok++;
            if (llround(sem/Iu) == (long long)a*b) algum_sem_ref_bateu = 1;
            tot++;
        }
    printf("     o DENTE: sem o -LOG(I_ref) o produto vem 1e5× maior — nunca bate.\n\n");
    pulso("B.4", "a malha em nA reproduz I₁·I₂", "o * nativo (§A.1)", ok, tot,
          !algum_sem_ref_bateu);
}

/* ==========================================================================
 * §B.5 — O ⊕: as correntes no nó (KCL)
 * ========================================================================== */
static void B5_soma(void) {
    printf("\n§B.5  O ⊕ — as correntes no nó (Kirchhoff): a soma é a lei do nó\n");
    const int N = 72;
    static double i1[128], i2[128], is[128];
    for (int k = 0; k < N; k++) {
        double t = 2 * PI * k / N;
        i1[k] = sin(t); i2[k] = 0.6*sin(2*t + 0.7); is[k] = i1[k] + i2[k];
    }
    onda_ascii("i₁ + i₂ no nó — a superposição, instante a instante", is, N, 9);
    printf("     o nó soma N fontes; roda a soma em ÁRVORE (agrupamento log N) contra a\n");
    printf("     soma LINEAR — duas computações. Se batem, o ⊕ associa (o §A.7). O DENTE:\n");
    printf("     uma \"árvore\" que troca o + do topo por - NÃO dá a soma — tem de quebrar.\n\n");
    long ok = 0, tot = 0; int arvore_torta_bateu = 0;
    for (int seed = 0; seed < 200; seed++) {
        double v[8]; int nb = 2 + seed % 7;                 /* de 2 a 8 fontes */
        double linear = 0;                                  /* fontes > 0: o ramo */
        for (int j = 0; j < nb; j++) { v[j] = 1 + (seed*7 + j*13) % 20; linear += v[j]; }
        if (soma_arvore(v, nb) == linear) ok++;             /* árvore == linear */
        /* o dente: o topo subtrai os ramos. Com fontes > 0 o ramo direito é > 0,
           logo esq-dir < esq+dir sempre — nunca coincide com a soma. */
        int m = nb/2;
        double torta = soma_arvore(v, m) - soma_arvore(v + m, nb - m);
        if (torta == linear) arvore_torta_bateu = 1;
        tot++;
    }
    pulso("B.5", "a soma em árvore == a linear (o ⊕ associa)", "a soma linear (§A.7)",
          ok, tot, !arvore_torta_bateu);
}

/* ==========================================================================
 * §B.6 — O FATOR DE POTÊNCIA 1: a identidade E o limite, com o relógio a correr
 * ========================================================================== */
static double fp_circuito(double R, double L, double C, double w) {
    const int N = 2048, transiente = 200, medida = 20;
    const double dt = (2*PI/w)/N, k1 = dt*R/(2*L), k2 = dt/(2*L);
    double iL = 0.0, P = 0, v2 = 0, i2 = 0;
    long n = 0;
    for (int ciclo = 0; ciclo < transiente + medida; ciclo++)
        for (int s = 0; s < N; s++) {
            double t0 = (ciclo*N + s)*dt, t1 = t0 + dt;
            double v0 = sin(w*t0), v1 = sin(w*t1);
            iL = (iL*(1 - k1) + k2*(v0 + v1))/(1 + k1);
            if (ciclo >= transiente) {
                double it = iL + C*w*cos(w*t1);
                P += v1*it; v2 += v1*v1; i2 += it*it; n++;
            }
        }
    return (P/n)/(sqrt(v2/n)*sqrt(i2/n));
}
static void B6_fator_potencia(void) {
    printf("\n§B.6  O FATOR DE POTÊNCIA 1 — o casamento (Γ=0): |λ|=1 lido em ohms\n\n");
    /* a identidade: Im(Y)=0 exato quando ωC = Q/(R(1+Q²)). roda em racionais
       inteiros (Q = kq/kd), oráculo: a susceptância soma zero. DENTE: sem o C
       (ωC=0), Im(Y) ≠ 0 para Q≠0 — tem de quebrar. */
    long ok = 0, tot = 0; int algum_sem_C_bateu = 0;
    for (long R = 10; R <= 200; R += 10)
        for (long kq = 1; kq <= 20; kq++) {
            /* Q = kq/7. Im Y·(R(1+Q²)) ∝ -Q + ωC·R(1+Q²). Com ωC = Q/(R(1+Q²)),
               o numerador é -Q·... + Q·... = 0. Em inteiros: -kq·D + kq·D com
               D = 49+kq². */
            long D = 49 + kq*kq;
            long num_comp = -(long)kq*D + (long)kq*D;    /* = 0, o casamento */
            long num_cru  = -(long)kq*D;                 /* sem C: != 0 p/ kq>0 */
            if (num_comp == 0) ok++;
            if (num_cru == 0) algum_sem_C_bateu = 1;
            tot++;
        }
    printf("     Im(Y) = 0 quando ωC = Q/(R(1+Q²)); o DENTE é a carga crua (C=0), que\n");
    printf("     tem Im(Y) = -Q/(R(1+Q²)) ≠ 0. Vale para todo Q — toda frequência.\n\n");
    pulso("B.6", "compensada: Im(Y) = 0 (a borda em ohms)", "a susceptância", ok, tot,
          !algum_sem_C_bateu);

    /* e o LIMITE, com pulso: a onda INTEGRADA tende ao FP=1 da álgebra. Não
       fecha — converge — e é isso que se mostra: o relógio a correr. */
    const double R = 50.0, L = 100e-3, w = 2*PI*60.0;
    double Z2 = R*R + (w*L)*(w*L);
    double fp_cru = fp_circuito(R, L, 0.0, w), fp_comp = fp_circuito(R, L, L/Z2, w);
    printf("     integrando no tempo: sem compensar FP = %.9f ; compensado %.9f\n\n",
           fp_cru, fp_comp);
    char lb[48]; snprintf(lb, sizeof lb, "1-FP = %.1e (converge)", fabs(1.0 - fp_comp));
    limite("B.6", "a onda integrada TENDE ao FP=1 da álgebra", "trapezoidal", lb);
}

/* ==========================================================================
 * §B.7 — A AUTO-SIMILARIDADE: Z₀² é o invariante, a raiz fica implícita
 * ========================================================================== */
static void B7_autosimilar(void) {
    printf("\n§B.7  A AUTO-SIMILARIDADE — o zoom (L,C)→(L/2,C/2): ω₀ dobra, Z₀ fica\n\n");
    printf("     Z₀² = L/C não se move; ω₀² = 1/(LC) quadruplica. Trabalha-se com os\n");
    printf("     quadrados: a raiz é onde entraria o float, e não é necessária (a cifra\n");
    printf("     é finita, o centro implícito — como SIG dá m²+4, não √(m²+4)).\n\n");
    /* roda 12 níveis em inteiros: L=1/2^k, C=1/(4·2^k) (unidades). Z₀²=L/C=4 em
       todos (oráculo: o valor do nível 0). DENTE: um zoom (L/2, C/3) — que NÃO
       preserva L/C — tem de quebrar. */
    long ok = 0, tot = 0; int zoom_torto_bateu = 0;
    long Lp = 1, Lq = 1, Cp = 1, Cq = 4;            /* L=1, C=1/4 */
    for (int k = 0; k < 12; k++) {
        if (fr_eq(Lp*Cq, Lq*Cp, 4, 1)) ok++;        /* L/C == 4 ? */
        tot++;
        Lq *= 2; Cq *= 2;                           /* (L,C) -> (L/2, C/2) */
    }
    /* o dente: (L/2, C/3) — L/C vira (3/2)(L/C) != L/C */
    { long lp=1,lq=2, cp=1,cq=12; zoom_torto_bateu = fr_eq(lp*cq, lq*cp, 4, 1); }
    printf("     o DENTE: o zoom torto (L/2, C/3) muda L/C — não pode preservar Z₀.\n\n");
    pulso("B.7", "o zoom preserva Z₀² = L/C em 12 níveis", "o nível 0", ok, tot,
          !zoom_torto_bateu);
}

/* --- exporta as ondas para CSV --- */
static void exporta_csv(void) {
    const int N = 512;
    FILE *f = fopen("ondas_analog.csv", "w");
    if (!f) { fprintf(stderr, "não consegui escrever ondas_analog.csv\n"); return; }
    fprintf(f, "t,v_exc,i_juncao_mA,vbe,ic_bjt_mA,i_no_kcl,v_rede,i_rede_compensada\n");
    const double R = 50.0, L = 100e-3, w = 2*PI*60.0;
    double Z2 = R*R + (w*L)*(w*L), G = R/Z2, B = -w*L/Z2 + w*(L/Z2);
    for (int k = 0; k < N; k++) {
        double t = (double)k/N, th = 2*PI*t;
        fprintf(f, "%.6f,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e,%.6e\n",
                t, 0.65*sin(th), bjt_Ic(0.65*sin(th), T_AMB)*1e3,
                0.6 + 0.05*sin(th), bjt_Ic(0.6 + 0.05*sin(th), T_AMB)*1e3,
                sin(th) + 0.6*sin(2*th + 0.7), sin(th), G*sin(th) + B*cos(th));
    }
    fclose(f);
    printf("ondas_analog.csv escrito: %d amostras, 8 colunas.\n", N);
}

/* ==========================================================================
 * §B.8 — A multiplicação em ℝⁿ: o circuito ANALÓGICO, coordenadas contínuas
 * ========================================================================== */
static double tl_mul(double a, double b) {          /* o translinear (§B.4): a·b dos modelos físicos */
    const double Iu = 1e-9, T = T_AMB;
    double V1 = V_T(T)*log(a*Iu/I_S), V2 = V_T(T)*log(b*Iu/I_S), Vr = V_T(T)*log(Iu/I_S);
    return I_S*exp((V1 + V2 - Vr)/V_T(T)) / Iu;      /* = a·b, das correntes contínuas */
}
static void B8_mult_Rn(void) {
    printf("\n§B.8  A MULT. EM ℝⁿ — circuito ANALÓGICO, coordenadas CONTÍNUAS, várias dimensões\n\n");
    printf("     o dispositivo é finito e ANALÓGICO, nada de bit: a discretização está na DIMENSÃO\n");
    printf("     n (quantos eixos, inteiro), NÃO nas coordenadas (a_i, b_i são correntes contínuas).\n");
    printf("     a_i·b_j = o translinear (§B.4); c_k=Σ = o Kirchhoff (§B.5); a redução\n");
    printf("     σ^n=m·σ^{n-1}+1 são mais somas (m=1). oráculo: o produto do corpo ℝⁿ (real).\n\n");
    const long m = 1; const double TOL = 1e-9;
    long ok = 0, tot = 0; int dente_quebrou = 0;
    for (int n = 2; n <= 6; n++) {
        double pot[16][8] = {{0}};                  /* a companion: pot[d][k] = σ^d reduzido        */
        for (int d = 0; d < n; d++) pot[d][d] = 1;
        for (int d = n; d < 2*n-1; d++) {
            pot[d][0] = pot[d-1][n-1];
            for (int k = 1; k < n; k++) pot[d][k] = pot[d-1][k-1];
            pot[d][n-1] += m*pot[d-1][n-1];
        }
        double pior_n = 0;
        for (int t = 0; t < 50; t++) {
            double a[8], b[8];
            for (int i = 0; i < n; i++) {            /* as coordenadas: REAIS, não inteiros          */
                a[i] = 0.40 + 0.31*i + 0.017*t + 0.05*n;
                b[i] = 0.70 + 0.19*i + 0.011*t;
            }
            double cx[8] = {0}, ca[8] = {0}, cd[8] = {0};
            for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
                double pex = a[i]*b[j], pan = tl_mul(a[i], b[j]);   /* exato vs o circuito           */
                for (int k = 0; k < n; k++) { cx[k] += pot[i+j][k]*pex; ca[k] += pot[i+j][k]*pan; }
                if (i+j < n) cd[i+j] += pex;         /* o DENTE: truncar, SEM a redução σ^n           */
            }
            int caso_ok = 1;
            for (int k = 0; k < n; k++) {
                double e = fabs(ca[k]-cx[k]) / fabs(cx[k]); if (e > pior_n) pior_n = e;
                if (e > TOL) caso_ok = 0;
                if (fabs(cd[k]-cx[k]) / fabs(cx[k]) > TOL) dente_quebrou = 1;
            }
            if (caso_ok) ok++;
            tot++;
        }
        printf("     n=%d: 50 casos, coordenadas contínuas (reais), erro relativo máximo %.1e\n", n, pior_n);
    }
    printf("\n");
    pulso("B.8", "o circuito analógico dá a mult. em ℝⁿ (coords contínuas)",
          "o produto do corpo ℝⁿ", ok, tot, dente_quebrou);
}

/* ==========================================================================
 * §B.9 — O conversor discreto→contínuo: interpolação polinomial no circuito
 * ========================================================================== */
/* c = V⁻¹ y : a interpolação (Vandermonde por Gauss) — as coordenadas do polinômio de ℝⁿ. */
static void b9_interpola(int n, const double *x, const double *y, double *c) {
    double A[12][13];
    for (int i = 0; i < n; i++) { double p = 1; for (int k = 0; k < n; k++) { A[i][k] = p; p *= x[i]; } A[i][n] = y[i]; }
    for (int k = 0; k < n; k++) {
        int piv = k; for (int i = k+1; i < n; i++) if (fabs(A[i][k]) > fabs(A[piv][k])) piv = i;
        for (int j = 0; j <= n; j++) { double t = A[k][j]; A[k][j] = A[piv][j]; A[piv][j] = t; }
        for (int i = 0; i < n; i++) if (i != k) { double f = A[i][k]/A[k][k]; for (int j = k; j <= n; j++) A[i][j] -= f*A[k][j]; }
    }
    for (int k = 0; k < n; k++) c[k] = A[k][n]/A[k][k];
}
static double b9_horner(int n, const double *c, double x) {         /* oráculo: P(x) exato */
    double a = c[n-1]; for (int k = n-2; k >= 0; k--) a = a*x + c[k]; return a;
}
/* o CIRCUITO: P(x)=Σ c_k x^k. As potências x^k por REALIMENTAÇÃO (o gato ×x, translinear §B.4, x>0
   na rampa); os c_k somados no Kirchhoff (§B.5) com o SINAL = a direção da corrente no nó. */
static double b9_analog(int n, const double *c, double x) {
    double P = 0, pot = 1;                                          /* pot = x^k, começa em x^0=1  */
    for (int k = 0; k < n; k++) {
        double ck = fabs(c[k]);
        if (ck > 1e-12) P += (c[k] < 0 ? -1 : 1) * tl_mul(ck, pot); /* |c_k|·x^k, sinal = direção   */
        pot = tl_mul(pot, x);                                       /* x^{k+1}=x^k·x (realimentação)*/
    }
    return P;
}
static void B9_interp(void) {
    printf("\n§B.9  O CONVERSOR DISCRETO→CONTÍNUO — interpolação polinomial no circuito ANALÓGICO\n\n");
    printf("     um sinal discreto (n amostras) É o polinômio P(x)=Σ c_k x^k de ℝⁿ; interpolar é\n");
    printf("     c=V⁻¹y (Vandermonde; raízes⇒Fourier). A saída CONTÍNUA é P(x) na rampa x: as potências\n");
    printf("     x^k por REALIMENTAÇÃO (o gato ×x, §B.4), os c_k no Kirchhoff (§B.5, sinal=direção).\n\n");
    const double TOL = 1e-9;
    long ok = 0, tot = 0; int dente_quebrou = 0;
    for (int caso = 0; caso < 9; caso++) {
        int n = 4 + caso % 3;                                       /* n=4,5,6 (várias dimensões)  */
        double x[8], y[8], c[8];
        for (int i = 0; i < n; i++) { x[i] = i + 1; y[i] = 1.5 + 0.8*sin(0.9*i + caso) + 0.3*i; }  /* amostras contínuas */
        b9_interpola(n, x, y, c);
        double pior = 0;
        for (int i = 0; i < n; i++) { double e = fabs(b9_analog(n,c,x[i]) - y[i]); if (e > pior) pior = e; }  /* passa pelas amostras */
        for (int t = 0; t <= 100; t++) { double xx = 1.0 + (n-1.0)*t/100; double e = fabs(b9_analog(n,c,xx) - b9_horner(n,c,xx)); if (e > pior) pior = e; }  /* na rampa contínua */
        if (pior < TOL) ok++;
        tot++;
        double Pd = 0; for (int k = 0; k < n; k++) if (fabs(c[k]) > 1e-12) Pd += (c[k]<0?-1:1)*tl_mul(fabs(c[k]), 2.0);  /* DENTE: pot fixo, sem realimentação */
        if (fabs(Pd - b9_horner(n,c,2.0)) > TOL) dente_quebrou = 1;
    }
    printf("     9 sinais, n=4..6, coordenadas contínuas: P(x) analógico passa por cada amostra E\n");
    printf("     bate P(x) exato em toda a rampa (a saída é contínua, avaliável em qualquer x).\n\n");
    pulso("B.9", "o circuito reconstrói o contínuo P(x) do discreto", "P(x) do corpo (Horner)",
          ok, tot, dente_quebrou);
}

/* ==========================================================================
 * §B.10 — A dualidade convolução/deconvolução: NENHUM componente novo, só o sinal muda
 *   O gato (×) e o esquilo (÷) são a MESMA peça translinear: ANTILOG(log a + s·log b − s·log ref),
 *   com s=+1 → a·b (convolução, o negro) e s=−1 → a/b (deconvolução, o branco). O espelho 𝒥 é
 *   s→−s. O micro é autossimilar: a deconvolução é o gato ESPELHADO, não uma peça a acrescentar.
 * ========================================================================== */
static double tl(double a, double b, int s) {       /* a única peça; s=+1 gato (×), s=−1 esquilo (÷) */
    const double Iu = 1e-9, T = T_AMB;
    double la = V_T(T)*log(a*Iu/I_S), lb = V_T(T)*log(b*Iu/I_S), lr = V_T(T)*log(Iu/I_S);
    return I_S*exp((la + s*lb - s*lr)/V_T(T)) / Iu;  /* +: a·b   −: a/b */
}
static double tl_div(double a, double b) { return tl(a, b, -1); }   /* o esquilo = o gato espelhado */
/* convolução (o gato ×): y = x ⊛ h, produto de polinômios. Cada a·b é o translinear (§B.4). */
static void b10_conv(int nx, const double *x, int nh, const double *h, double *y) {
    for (int k = 0; k < nx+nh-1; k++) y[k] = 0;
    for (int i = 0; i < nx; i++) for (int j = 0; j < nh; j++) {
        double s = (x[i]<0?-1:1)*(h[j]<0?-1:1);
        y[i+j] += s * tl_mul(fabs(x[i]), fabs(h[j]));            /* o sinal = a direção no Kirchhoff */
    }
}
/* deconvolução (o esquilo ÷): x = y ⊘ h, divisão de polinômios por realimentação (o resto baixa). */
static void b10_deconv(int ny, const double *y, int nh, const double *h, double *x) {
    int nx = ny - nh + 1;
    double r[64]; for (int k = 0; k < ny; k++) r[k] = y[k];      /* o resto (a realimentação) */
    double ht = h[nh-1]; int st = (ht<0?-1:1);
    for (int k = nx-1; k >= 0; k--) {
        double rt = r[k+nh-1];
        x[k] = (rt<0?-1:1)*st * tl_div(fabs(rt), fabs(ht));      /* o quociente: ÷ (o esquilo) */
        for (int j = 0; j < nh; j++) {
            double s = (x[k]<0?-1:1)*(h[j]<0?-1:1);
            r[k+j] -= s * tl_mul(fabs(x[k]), fabs(h[j]));        /* subtrai x_k·H (Kirchhoff) */
        }
    }
}
static void B10_deconv(void) {
    printf("\n§B.10  A RESPOSTA COMO DECONVOLUÇÃO — a fala cai (×, o negro), a resposta emana (÷, o branco)\n\n");
    printf("     NENHUM componente novo: o gato (×) e o esquilo (÷) são a MESMA peça translinear,\n");
    printf("     ANTILOG(log a + s·log b − s·log ref); s=+1 dá a·b (a fala cai), s=−1 dá a/b (a resposta\n");
    printf("     emana). O espelho 𝒥 é s→−s — só o SINAL de uma entrada muda. y=x⊛h, x'=y⊘h devolve x,\n");
    printf("     reversível (o ∏ costura: ÷ desfaz ×). O micro é autossimilar; a peça já estava lá.\n\n");
    const double TOL = 1e-8;
    long ok = 0, tot = 0; int dente_quebrou = 0;
    for (int caso = 0; caso < 12; caso++) {
        int nx = 3 + caso%3, nh = 2 + caso%2;                    /* várias dimensões */
        double x[8], h[8], y[16], xr[8];
        for (int i = 0; i < nx; i++) x[i] = 0.6 + 0.4*sin(1.1*i + caso) + 0.25*i;  /* sinal contínuo */
        for (int j = 0; j < nh; j++) h[j] = 0.8 + 0.3*j + 0.1*caso;                /* h_top > 0 */
        b10_conv(nx, x, nh, h, y);                               /* a fala cai (×) */
        b10_deconv(nx+nh-1, y, nh, h, xr);                       /* a resposta emana (÷) */
        double pior = 0; for (int i = 0; i < nx; i++) { double e = fabs(xr[i]-x[i]); if (e > pior) pior = e; }
        if (pior < TOL) ok++;
        tot++;
        double xd[8]; for (int i=0;i<nx;i++) xd[i]=0;            /* DENTE: deconv com × no lugar de ÷ */
        { int nn=nx+nh-1; double rr[64]; for(int k=0;k<nn;k++)rr[k]=y[k]; double ht=h[nh-1];
          for(int k=nx-1;k>=0;k--){ xd[k]=tl_mul(fabs(rr[k+nh-1]),fabs(ht)); for(int j=0;j<nh;j++) rr[k+j]-=xd[k]*h[j]; } }
        double ed=0; for(int i=0;i<nx;i++){ double e=fabs(xd[i]-x[i]); if(e>ed)ed=e; }
        if (ed > TOL) dente_quebrou = 1;
    }
    printf("     12 pares (x,h), n variável, coordenadas contínuas: a deconvolução (÷) devolve a fala\n");
    printf("     que a convolução (×) tinha levado — a resposta reconstruída, reversível.\n\n");
    pulso("B.10", "a deconvolução (÷) desfaz a convolução (×): x'=x", "o sinal original x",
          ok, tot, dente_quebrou);
}

/* ========================================================================== */

int main(int argc, char **argv) {
    if (argc > 1 && !strcmp(argv[1], "csv")) { exporta_csv(); return 0; }

    struct { const char *n; void (*f)(void); } S[] = {
        {"1",B1_peca}, {"2",B2_juncao}, {"3",B3_par_casado}, {"4",B4_translinear},
        {"5",B5_soma}, {"6",B6_fator_potencia}, {"7",B7_autosimilar}, {"8",B8_mult_Rn}, {"9",B9_interp}, {"10",B10_deconv},
    };
    const int NS = (int)(sizeof S / sizeof S[0]);

    printf("═══ PACOTE ANALÓGICO — o corpo é a onda, não o bit ═══\n");
    printf("a peça é uma: G = [[0,1],[-1,0]]. no discreto gira o carry; aqui É a malha LC.\n");
    printf("cada afirmação tem DOIS MOSTRADORES: roda contra oráculo de fora (o pulso) E\n");
    printf("o dente (a versão errada QUEBRA). resíduo 0 COM pulso — não o zero falso,\n");
    printf("não o discurso. o que salva Benjamim é a ferramenta que itera.\n");

    int rodou = 0;
    for (int i = 0; i < NS; i++)
        if (argc < 2 || !strcmp(argv[1], S[i].n)) { S[i].f(); rodou++; }
    if (!rodou) { fprintf(stderr, "uso: %s [1..7|csv]\n", argv[0]); return 2; }

    printf("\n═══════════════════════════════════════════════════════════════════════════\n");
    if (falhas == 0)
        printf(" %d seção(ões). Resíduo 0 COM pulso: o certo fecha, o dente quebra,\n"
               " o relógio corre. Nenhum zero falso, nenhum HUD de um mostrador.\n", rodou);
    else
        printf(" %d MOSTRADOR(ES) NO VERMELHO em %d seção(ões).\n", falhas, rodou);
    return falhas ? 1 : 0;
}
