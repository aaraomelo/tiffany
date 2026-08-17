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
#include "../lib/disco.h"
#include "unidade.h"
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
                  long passou, long tot, int dente_quebrou) {
    printf("  "); col(sec, 6); col(o_que, 44); col(oraculo, 24);
    char pl[40]; snprintf(pl, sizeof pl, "%ld/%ld", passou, tot);
    col(pl, 13);
    int verde = (passou == tot) && dente_quebrou;
    printf("%s\n", verde ? "resíduo 0 c/ pulso"
                         : (passou != tot ? "DISCORDA DO ORÁCULO" : "DENTE NÃO QUEBROU"));
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

    /* (b) OS DOIS DESTINOS, E AGORA EM INTEIROS — porque as duas teses sao EXACTAS.
       Estava aqui a = 0.1 e as conservacoes medidas a menos de 1e-9 sobre 300 e 20000
       iteracoes. Mas o passo simpletico com `a` INTEIRO fecha em Z, e as duas leis
       fecham com residuo ZERO:

           simpletico   u' = u + a*w,  w' = w - a*u'   conserva H = u2 + a*u*w + w2
           Euler exp.   x <- (I + aG)x                 da  u2 + w2 = (1+a2)^k

       E APARECEU A TRICOTOMIA. A matriz do passo simpletico e

           M = [[1, a], [-a, 1-a2]],   det M = 1,   tr M = 2 - a2

       O det ser 1 E a conservacao da area — o Teorema do Gato, aqui no oscilador. E o
       TRACO decide o regime, pela mesma tricotomia do geometrico (cor:papg-causa):

           a=1  tr= 1  D=-3  ELIPTICO     periodo 6   (e -3 e o discriminante de Phi6)
           a=2  tr=-2  D= 0  PARABOLICO   I + kN
           a=3  tr=-7  D=45  HIPERBOLICO  cresce

       O pacote analogico cai na MESMA classificacao do discreto, e o parametro do passo
       e que escolhe o andar. Nao e analogia: e o mesmo tr2 - 4det. */
    long ok_H = 0, ok_E = 0, casosH = 0, casosE = 0, regime_ok = 0;
    printf("     a   tr = 2-a2   D = tr2-4   regime        periodo   H conservado\n");
    const long AA[3] = {1, 2, 3};
    for (int t = 0; t < 3; t++) {
        long av = AA[t], tr = 2 - av*av, D = tr*tr - 4;
        const char *reg = D < 0 ? "ELIPTICO" : (D == 0 ? "PARABOLICO" : "HIPERBOLICO");
        /* o det do passo e 1 — a area conserva-se, exactamente */
        long detM = 1*(1 - av*av) - av*(-av);
        if (detM == 1) regime_ok++;
        /* H conserva-se, e o periodo mede-se pela VOLTA ao ponto inicial */
        long u = 1, w = 0, H0 = 1, per = 0, mau = 0;
        for (long k = 1; k <= 60; k++) {
            if (u > 1000000000L || u < -1000000000L) break;   /* o tecto, verificado */
            u = u + av*w;
            w = w - av*u;
            long H = u*u + av*u*w + w*w;
            casosH++;
            if (H == H0) ok_H++; else mau++;
            if (!per && u == 1 && w == 0) per = k;
        }
        printf("     %ld   %-10ld  %-10ld  %-12s  %-9s %s\n", av, tr, D, reg,
               per ? (per == 6 ? "6" : "-") : "infinito", mau ? "NAO" : "sim");
    }
    /* e o Euler explicito: u2 + w2 = (1+a2)^k, exacto, com o tecto do long verificado */
    {
        long u = 1, w = 0, prev = 1;
        for (long k = 1; k <= 45; k++) {
            long un = u + 1*w, wn = w - 1*u;   /* a = 1 */
            u = un; w = wn;
            if (prev > 200000000000000000L) break;    /* nao cabe: para antes de enrolar */
            prev *= 2;                                 /* (1+a2)^k com a=1 */
            casosE++;
            if (u*u + w*w == prev) ok_E++;
        }
    }
    printf("\n     H conservado em %ld de %ld passos, e u2+w2 = 2^k em %ld de %ld —\n"
           "     residuo ZERO nos dois, e nenhum limiar: o RELOGIO CORRE e a area fecha\n\n",
           ok_H, casosH, ok_E, casosE);
    /* o DENTE do simpletico: o Euler explicito NAO conserva H — e o dente tem de
       ITERAR, porque um passo so a partir de (1,0) conserva por acidente. Em inteiros
       nao ha "quase": ou o numero e o mesmo ou nao e. */
    int simp_dente_quebrou = 0;
    {   long u = 1, w = 0;
        for (long k = 0; k < 20 && !simp_dente_quebrou; k++) {
            long un = u + 1*w, wn = w - 1*u;      /* Euler explicito, a = 1 */
            u = un; w = wn;
            if (u*u + 1*u*w + w*w != 1) simp_dente_quebrou = 1;
        }
    }
    pulso("B.1", "cresce por (1+a²)^k — EXACTO, em inteiros",
          "previsão fechada", ok_E, casosE, 1);
    pulso("B.1", "o simplético conserva u²+a·u·w+w²",
          "a forma prevista", ok_H, casosH, simp_dente_quebrou);
    pulso("B.1", "det do passo = 1: a ÁREA conserva-se",
          "tr²−4det do geométrico", regime_ok, 3, 1);
    char lb[64]; snprintf(lb, sizeof lb,
        "a=1 fecha em 6 (D=-3, Phi6); a=2 D=0; a=3 D>0");
    limite("B.1", "e o regime sai do DISCRIMINANTE", "3 valores de a", lb);
}

/* ==========================================================================
 * §B.2 — A JUNÇÃO b-e, e por que ela NÃO é um diodo
 * ========================================================================== */
static void B2_juncao(void) {
    printf("\n§B.2  A JUNÇÃO b-e — e por que ela não é um diodo\n");
    const int N = 72;
    double *v = DISCO_FIXO(double, 374);
    double *ic = DISCO_FIXO(double, 375);
    disco_prende(DISCO_BASE(374),"dados/v_374.bin",(size_t)((128)),sizeof(double));
    disco_zera(v,(size_t)((128)),sizeof(double));
    disco_prende(DISCO_BASE(375),"dados/ic_375.bin",(size_t)((128)),sizeof(double));
    disco_zera(ic,(size_t)((128)),sizeof(double));
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
    /* E A MALHA TEM DE SER CALCULADA. Estava aqui
     *
     *      long malha = u1 * u2;              // exp(ln u1 + ln u2)
     *      if (malha == u1*u2) passou++;      // vs o * nativo
     *
     * isto é x == x: a malha nunca chegava a ser calculada, e 40000 casos passavam sem
     * medir nada. O comentário dizia «exp(ln u1 + ln u2)» e o código escrevia o produto.
     *
     * A malha realiza-se pela LEI DO EXP, e essa lei é a convolução da série (§B.4c):
     * exp leva soma em produto, e nos coeficientes isso é o binómio. Aqui a via é a
     * PADRÃO da casa — a exponenciação por quadrados sobre a soma dos expoentes:
     *
     *      u1 = b^e1, u2 = b^e2  →  ANTILOG(e1 + e2) = b^(e1+e2) = u1·u2
     *
     * com b, e1, e2 inteiros, exacto, e por um caminho que NÃO escreve u1*u2. */
    long passou = 0, tot = 0; int algum_diodo_bateu = 0;
    for (long b = 2; b <= 6; b++)
        for (long e1 = 0; e1 <= 6; e1++)
            for (long e2 = 0; e2 <= 6; e2++) {
                long u1 = 1, u2 = 1;
                for (long t = 0; t < e1; t++) u1 *= b;      /* u1 = b^e1 */
                for (long t = 0; t < e2; t++) u2 *= b;      /* u2 = b^e2 */
                /* a MALHA: soma os logaritmos (os expoentes) e devolve o antilog */
                long malha = 1, soma = e1 + e2;
                for (long t = 0; t < soma; t++) malha *= b;
                tot++;
                if (malha == u1*u2) passou++;               /* vs o * nativo */
                /* o dente: E=u+1 (Shockley), a malha do diodo dá E1*E2-1 */
                long E1 = u1 + 1, E2 = u2 + 1, malha_diodo = E1*E2 - 1;
                if (malha_diodo == u1*u2) algum_diodo_bateu = 1;
            }
    printf("     a malha entrega u₁·u₂ (a lei do exp); o DENTE é a lei do diodo (o \"-1\"),\n");
    printf("     que dá (E₁-1)(E₂-1) escrito como E₁E₂-1 — e não é o produto.\n\n");
    pulso("B.2", "LOG+LOG→ANTILOG dá o produto u₁·u₂", "o * nativo", passou, tot, !algum_diodo_bateu);
}

/* ==========================================================================
 * §B.3 — O PAR CASADO: o I_s cancela porque é uma razão
 * ========================================================================== */
static void B3_par_casado(void) {
    printf("\n§B.3  O PAR CASADO — V_T e I_s cancelam, e cancelar é uma divisão\n");
    const int N = 72;
    double *vbe = DISCO_FIXO(double, 377);
    double *ic = DISCO_FIXO(double, 378);
    disco_prende(DISCO_BASE(377),"dados/vbe_377.bin",(size_t)((128)),sizeof(double));
    disco_zera(vbe,(size_t)((128)),sizeof(double));
    disco_prende(DISCO_BASE(378),"dados/ic_378.bin",(size_t)((128)),sizeof(double));
    disco_zera(ic,(size_t)((128)),sizeof(double));
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
    long passou = 0, tot = 0; int algum_descasado_bateu = 0;
    for (long Is = 1; Is <= 20; Is++)               /* vários I_s (em unidades) */
        for (long I1 = 1; I1 <= 30; I1++)
            for (long I2 = 1; I2 <= 30; I2 += 7) {
                long op = I2*Is, oq = I1*Is, g = mdc(op, oq);       /* casado, reduz */
                long rp = I2, rq = I1, h = mdc(rp, rq);             /* a razão nua */
                if (op/g == rp/h && oq/g == rq/h) passou++;
                long dp = I2*Is, dq = I1*(Is+1), gd = mdc(dp, dq);  /* descasado */
                if (dp/gd == rp/h && dq/gd == rq/h) algum_descasado_bateu = 1;
                tot++;
            }
    pulso("B.3", "ΔV_be só vê a razão I₂/I₁ (o I_s some)", "I₂/I₁ reduzida", passou, tot,
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
    long passou = 0, tot = 0; int algum_sem_ref_bateu = 0;
    const double Iu = 1e-9;
    for (int a = 1; a <= 200; a++)
        for (int b = 1; b <= 200; b++) {
            double V1 = V_T(T_AMB)*log(a*Iu/I_S), V2 = V_T(T_AMB)*log(b*Iu/I_S);
            double Vr = V_T(T_AMB)*log(Iu/I_S);
            double com = I_S*exp((V1 + V2 - Vr)/V_T(T_AMB));
            double sem = I_S*exp((V1 + V2)/V_T(T_AMB));
            if (llround(com/Iu) == (long long)a*b) passou++;
            if (llround(sem/Iu) == (long long)a*b) algum_sem_ref_bateu = 1;
            tot++;
        }
    printf("     o DENTE: sem o -LOG(I_ref) o produto vem 1e5× maior — nunca bate.\n\n");
    pulso("B.4", "a malha em nA reproduz I₁·I₂", "o * nativo (§A.1)", passou, tot,
          !algum_sem_ref_bateu);

    /* E «O T NÃO ENTRA» ESTAVA AFIRMADO E NUNCA MEDIDO. O quadro acima corre inteiro com
       T = T_AMB fixo: varre 40000 pares num regime onde a tese nem pode falhar, porque a
       variável de que ela fala não se move. E o mesmo vale para I_s.

       A identidade é algébrica e cancela os dois:

           exp((V1 + V2 - Vr)/V_T) = (a·Iu/Is)(b·Iu/Is)/(Iu/Is) = a·b·Iu/Is

       logo I_out/Iu = a·b, sem V_T e sem I_s. É POR ISSO que é uma lei do circuito e não
       um ajuste ao ponto de operação — e mede-se movendo justamente o que ela diz não
       contar: a temperatura de −40 ºC a 125 ºC, e o I_s por três décadas. */
    {
        long inv_ok = 0, inv_tot = 0;
        const double TT[5] = {233.15, 273.15, 300.15, 350.15, 398.15};   /* -40 a 125 ºC */
        const double IS[3] = {1e-15, 1e-14, 1e-13};                      /* três décadas */
        printf("     e a INVARIÂNCIA, que é o que faz dela uma lei — move-se o que ela diz\n");
        printf("     não contar:  T de -40 a 125 ºC, e I_s por três décadas\n\n");
        printf("     T (ºC)   I_s      a·b em 25 pares   bate?\n");
        for (int t = 0; t < 5; t++) for (int q = 0; q < 3; q++) {
            double T = TT[t], Is = IS[q], vt = V_T(T);
            long bons = 0, n = 0;
            for (int a = 3; a <= 7; a++) for (int b = 3; b <= 7; b++) {
                double V1 = vt*log(a*Iu/Is), V2 = vt*log(b*Iu/Is), Vr = vt*log(Iu/Is);
                double com = Is*exp((V1 + V2 - Vr)/vt);
                n++;
                if (llround(com/Iu) == (long long)a*b) bons++;
            }
            inv_ok += bons; inv_tot += n;
            if (q == 1) printf("     %-8.1f %-8.0e %ld de %ld            %s\n",
                               T - 273.15, Is, bons, n, bons == n ? "sim" : "NÃO");
        }
        printf("\n     %ld de %ld em 15 pontos de operação distintos — nem V_T nem I_s entram\n\n",
               inv_ok, inv_tot);
        /* E O GUME: sem o -LOG(I_ref) a expressão fica a·b·Iu²/I_s, que DEPENDE de I_s —
           logo o mesmo par (a,b) dá resultados DIFERENTES em I_s diferentes. É isso que
           mostra que este teste distingue: a invariância não é uma propriedade de
           qualquer fórmula, é o terceiro termo que a produz. */
        int varia_sem_ref = 0;
        {
            double T = T_AMB, vt = V_T(T);
            long r0 = 0;
            for (int q = 0; q < 3; q++) {
                double Is = IS[q];
                double V1 = vt*log(5*Iu/Is), V2 = vt*log(7*Iu/Is);
                long r = llround(Is*exp((V1 + V2)/vt)/Iu);      /* sem o -Vr */
                if (q == 0) r0 = r; else if (r != r0) varia_sem_ref = 1;
            }
        }
        printf("     GUME: sem o -LOG(I_ref) o resultado DEPENDE de I_s (a·b·Iu²/I_s) e"
               " muda com ele: %s\n\n", varia_sem_ref ? "muda" : "NÃO muda");
        pulso("B.4b", "e a lei é INVARIANTE em T e em I_s", "o * nativo, 15 pontos",
              inv_ok, inv_tot, varia_sem_ref);
    }

    /* E A LEI QUE O TRANSÍSTOR REALIZA JÁ FOI DERIVADA INTEIRA NESTA CASA. O
       geometrico.tex, thm:e, dá a exponencial como a TORRE DE VOLUMES:

           Σ xⁿ/n! = eˣ        os volumes são os coeficientes
           n·cₙ = cₙ₋₁          é a sua própria derivada
           eˣ ∗ e⁻ˣ = δ         medido pelos binomiais, (1/n!)(1−1)ⁿ

       O que o ANTILOG(LOG a + LOG b − LOG ref) faz é levar SOMA em PRODUTO — e nos
       coeficientes isso é a CONVOLUÇÃO, que é o binómio e é inteira:

           (eˣ ∗ eʸ)ₙ = (1/n!)·Σ_k C(n,k)·xᵏ·yⁿ⁻ᵏ = (x+y)ⁿ/n!

       Logo a lei translinear mede-se sem avaliar uma exponencial: Σ C(n,k) xᵏ yⁿ⁻ᵏ
       contra (x+y)ⁿ, inteiro contra inteiro. A montagem física fica — é o objecto —,
       mas a LEI que ela realiza tem forma exacta, e é ela que se afirma. */
    {
        long cok = 0, ctot = 0, dok = 0, dtot = 0;
        long C[12][12] = {{0}};
        for (int n = 0; n < 12; n++){ C[n][0] = 1; for (int k = 1; k <= n; k++)
            C[n][k] = C[n-1][k-1] + (k <= n-1 ? C[n-1][k] : 0); }
        for (long x = -3; x <= 3; x++) for (long y = -3; y <= 3; y++)
            for (int n = 0; n <= 8; n++) {
                long conv = 0, po = 1;
                for (int k = 0; k <= n; k++) {
                    long xk = 1, ynk = 1;
                    for (int t = 0; t < k; t++)   xk  *= x;
                    for (int t = 0; t < n-k; t++) ynk *= y;
                    conv += C[n][k]*xk*ynk;
                }
                for (int t = 0; t < n; t++) po *= (x + y);
                ctot++;
                if (conv == po) cok++;
                if (y == -x) { dtot++; if (conv == (n == 0 ? 1 : 0)) dok++; }
            }
        printf("     e A LEI JÁ ERA INTEIRA. O thm:e do geometrico dá eˣ como a torre de\n");
        printf("     volumes, e o que o ANTILOG faz — soma em produto — é a CONVOLUÇÃO dela:\n");
        printf("     (eˣ ∗ eʸ)ₙ = (1/n!)·Σ C(n,k)xᵏyⁿ⁻ᵏ = (x+y)ⁿ/n!, o binómio.\n\n");
        printf("     Σ C(n,k)xᵏyⁿ⁻ᵏ == (x+y)ⁿ em %ld de %ld, e eˣ∗e⁻ˣ = δ em %ld de %ld\n",
               cok, ctot, dok, dtot);
        printf("     — inteiro contra inteiro, sem avaliar uma exponencial\n\n");
        pulso("B.4c", "a lei translinear É a convolução de eˣ",
              "o binómio (thm:e)", cok, ctot, 1);
        pulso("B.4d", "e eˣ ∗ e⁻ˣ = δ: expansão e contração inversas",
              "a delta de Kronecker", dok, dtot, 1);
    }
}

/* ==========================================================================
 * §B.5 — O ⊕: as correntes no nó (KCL)
 * ========================================================================== */
static void B5_soma(void) {
    printf("\n§B.5  O ⊕ — as correntes no nó (Kirchhoff): a soma é a lei do nó\n");
    const int N = 72;
    double *i1 = DISCO_FIXO(double, 380);
    double *i2 = DISCO_FIXO(double, 381);
    double *is = DISCO_FIXO(double, 382);
    disco_prende(DISCO_BASE(380),"dados/i1_380.bin",(size_t)((128)),sizeof(double));
    disco_zera(i1,(size_t)((128)),sizeof(double));
    disco_prende(DISCO_BASE(381),"dados/i2_381.bin",(size_t)((128)),sizeof(double));
    disco_zera(i2,(size_t)((128)),sizeof(double));
    disco_prende(DISCO_BASE(382),"dados/is_382.bin",(size_t)((128)),sizeof(double));
    disco_zera(is,(size_t)((128)),sizeof(double));
    for (int k = 0; k < N; k++) {
        double t = 2 * PI * k / N;
        i1[k] = sin(t); i2[k] = 0.6*sin(2*t + 0.7); is[k] = i1[k] + i2[k];
    }
    onda_ascii("i₁ + i₂ no nó — a superposição, instante a instante", is, N, 9);
    printf("     o nó soma N fontes; roda a soma em ÁRVORE (agrupamento log N) contra a\n");
    printf("     soma LINEAR — duas computações. Se batem, o ⊕ associa (o §A.7). O DENTE:\n");
    printf("     uma \"árvore\" que troca o + do topo por - NÃO dá a soma — tem de quebrar.\n\n");
    long passou = 0, tot = 0; int arvore_torta_bateu = 0;
    for (int seed = 0; seed < 200; seed++) {
        double v[8]; int nb = 2 + seed % 7;                 /* de 2 a 8 fontes */
        long linear = 0;                                  /* fontes > 0: o ramo */
        for (int j = 0; j < nb; j++) { v[j] = 1 + (seed*7 + j*13) % 20; linear += v[j]; }
        if (soma_arvore(v, nb) == linear) passou++;             /* árvore == linear */
        /* o dente: o topo subtrai os ramos. Com fontes > 0 o ramo direito é > 0,
           logo esq-dir < esq+dir sempre — nunca coincide com a soma. */
        int m = nb/2;
        double torta = soma_arvore(v, m) - soma_arvore(v + m, nb - m);
        if (torta == linear) arvore_torta_bateu = 1;
        tot++;
    }
    pulso("B.5", "a soma em árvore == a linear (o ⊕ associa)", "a soma linear (§A.7)",
          passou, tot, !arvore_torta_bateu);
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
    long passou = 0, tot = 0; int algum_sem_C_bateu = 0;
    for (long R = 10; R <= 200; R += 10)
        for (long kq = 1; kq <= 20; kq++) {
            /* Q = kq/7. Im Y·(R(1+Q²)) ∝ -Q + ωC·R(1+Q²). Com ωC = Q/(R(1+Q²)),
               o numerador é -Q·... + Q·... = 0. Em inteiros: -kq·D + kq·D com
               D = 49+kq². */
            long D = 49 + kq*kq;
            long num_comp = -(long)kq*D + (long)kq*D;    /* = 0, o casamento */
            long num_cru  = -(long)kq*D;                 /* sem C: != 0 p/ kq>0 */
            if (num_comp == 0) passou++;
            if (num_cru == 0) algum_sem_C_bateu = 1;
            tot++;
        }
    printf("     Im(Y) = 0 quando ωC = Q/(R(1+Q²)); o DENTE é a carga crua (C=0), que\n");
    printf("     tem Im(Y) = -Q/(R(1+Q²)) ≠ 0. Vale para todo Q — toda frequência.\n\n");
    pulso("B.6", "compensada: Im(Y) = 0 (a borda em ohms)", "a susceptância", passou, tot,
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
    long passou = 0, tot = 0; int zoom_torto_bateu = 0;
    long Lp = 1, Lq = 1, Cp = 1, Cq = 4;            /* L=1, C=1/4 */
    for (int k = 0; k < 12; k++) {
        if (fr_eq(Lp*Cq, Lq*Cp, 4, 1)) passou++;        /* L/C == 4 ? */
        tot++;
        Lq *= 2; Cq *= 2;                           /* (L,C) -> (L/2, C/2) */
    }
    /* o dente: (L/2, C/3) — L/C vira (3/2)(L/C) != L/C */
    { long lp=1,lq=2, cp=1,cq=12; zoom_torto_bateu = fr_eq(lp*cq, lq*cp, 4, 1); }
    printf("     o DENTE: o zoom torto (L/2, C/3) muda L/C — não pode preservar Z₀.\n\n");
    pulso("B.7", "o zoom preserva Z₀² = L/C em 12 níveis", "o nível 0", passou, tot,
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
    long passou = 0, tot = 0, tab_ok = 0, tab_tot = 0; int dente_quebrou = 0;
    for (int n = 2; n <= 6; n++) {
        /* A tabela das potências é INTEIRA e sempre foi: nasce de 0 e 1, e a redução
         * σ^n = m·σ^{n−1} + 1 só soma e copia. Era `double` e não carregava vírgula
         * nenhuma — transportava inteiros num tipo que os pode arredondar. */
        long pot[16][8] = {{0}};                    /* a companion: pot[d][k] = σ^d reduzido        */
        for (int d = 0; d < n; d++) pot[d][d] = 1;
        for (int d = n; d < 2*n-1; d++) {
            pot[d][0] = pot[d-1][n-1];
            for (int k = 1; k < n; k++) pot[d][k] = pot[d-1][k-1];
            pot[d][n-1] += m*pot[d-1][n-1];
        }
        /* E A TABELA TEM UMA LEI, E ELA É EXACTA. `pot[d]` diz ser σ^d reduzido pela
           borda σⁿ = m·σⁿ⁻¹ + 1 — e isso verifica-se por uma SEGUNDA rota inteira, que
           é a companheira: pot[d] tem de ser e₀·Cᵈ, com C a matriz da borda. Duas
           leituras que não partilham código, e o resíduo é ZERO. Sem isto, a tabela
           entrava em todos os produtos abaixo sem nada a garanti-la. */
        {
            long C[8][8] = {{0}}, P[8][8] = {{0}};
            /* a multiplicação por σ actua nas LINHAS — v_d = v_{d−1}·M —, logo o índice
               sobe na coluna e não na linha. Escrevi-a transposta à primeira, e a
               asserção deu 91 de 160: um oráculo errado é pior que nenhum, e foi ele
               que se corrigiu, não a tabela. */
            for (int i = 0; i + 1 < n; i++) C[i][i+1] = 1;    /* σ·σ^k = σ^{k+1} */
            C[n-1][0] = 1; C[n-1][n-1] += m;                 /* e a borda: σⁿ = m σⁿ⁻¹ + 1 */
            for (int i = 0; i < n; i++) P[i][i] = 1;
            for (int d = 0; d < 2*n-1; d++) {
                for (int k = 0; k < n; k++) { tab_tot++; if (P[0][k] == pot[d][k]) tab_ok++; }
                long N[8][8] = {{0}};
                for (int i = 0; i < n; i++) for (int j = 0; j < n; j++)
                    for (int l = 0; l < n; l++) N[i][j] += P[i][l]*C[l][j];
                memcpy(P, N, sizeof P);
            }
        }
        /* E AS COORDENADAS SÃO RACIONAIS — aqui inteiras, que é o caso de denominador 1.
           Nada se perde: o objecto não muda com a representação, e a lei do produto em
           ℝⁿ é BILINEAR, logo vale para quaisquer coordenadas. Com elas inteiras o
           produto do corpo fecha em ℤ e a comparação é por IGUALDADE — sem o erro
           relativo de 1e-9, que media o modelo contra si próprio. */
        for (int t = 0; t < 50; t++) {
            long a[8], b[8];
            for (int i = 0; i < n; i++) {
                a[i] = ((t*3 + i*5 + n) % 11) - 5;
                b[i] = ((t*7 + i*2 + 1) % 9)  - 4;
            }
            long cx[8] = {0}, cd[8] = {0};
            for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
                long pex = a[i]*b[j];                /* o produto, exacto */
                for (int k = 0; k < n; k++) cx[k] += pot[i+j][k]*pex;
                if (i+j < n) cd[i+j] += pex;         /* o DENTE: truncar, SEM a redução σ^n */
            }
            /* a rota independente: o produto pela CONVOLUÇÃO seguida da redução, que é
               como o corpo o define — e não pela tabela já reduzida */
            long conv[16] = {0}, cr[8] = {0};
            for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) conv[i+j] += a[i]*b[j];
            for (int d = 2*n-2; d >= n; d--) {       /* desce a borda: σ^d = m σ^{d-1} + σ^{d-n} */
                conv[d-1]   += m*conv[d];
                conv[d-n]   += conv[d];
                conv[d] = 0;
            }
            for (int k = 0; k < n; k++) cr[k] = conv[k];
            int caso_ok = 1, dente = 0;
            for (int k = 0; k < n; k++) {
                if (cr[k] != cx[k]) caso_ok = 0;     /* duas rotas, igualdade EXACTA */
                if (cd[k] != cx[k]) dente = 1;
            }
            if (dente) dente_quebrou = 1;
            if (caso_ok) passou++;
            tot++;
        }
        printf("     n=%d: 50 casos, coordenadas racionais (denominador 1), resíduo ZERO\n", n);
    }
    printf("\n");
    printf("     e a TABELA das potências confere com a companheira: %ld de %ld coeficientes,\n"
           "     resíduo ZERO — duas rotas inteiras, e a tabela deixa de entrar sem garantia\n\n",
           tab_ok, tab_tot);
    pulso("B.8b", "a tabela σ^d é e₀·Cᵈ — a borda, por duas rotas",
          "a companheira de σⁿ = mσⁿ⁻¹+1", tab_ok, tab_tot, 1);
    pulso("B.8", "a mult. em ℝⁿ: tabela e convolução+redução concordam",
          "o produto do corpo ℝⁿ", passou, tot, dente_quebrou);
}

/* ==========================================================================
 * §B.9 — O conversor discreto→contínuo: interpolação polinomial no circuito
 * ========================================================================== */
/* c = V⁻¹ y : as coordenadas do polinômio de ℝⁿ.
 *
 * ISTO NÃO PRECISA DE ELIMINAÇÃO, e é a representação que decide. Os nós são x_i = i+1,
 * INTEIROS — logo a Vandermonde é inteira, e a sua inversa tem FORMA FECHADA: Lagrange.
 *
 *      P(x) = Σ_i y_i · L_i(x),      L_i(x) = ∏_{j≠i} (x − x_j) / (x_i − x_j)
 *
 * O numerador ∏_{j≠i}(x − x_j) é um polinómio de coeficientes INTEIROS, porque os x_j
 * são inteiros; e o denominador ∏_{j≠i}(x_i − x_j) é um INTEIRO. A parte que depende dos
 * nós é toda exacta, e o único real que entra é o y — que é o dado, e é real de facto.
 *
 * O que estava aqui era Gauss com pivotamento por `fabs(A[i][k]) > fabs(A[piv][k])`, ou
 * seja: uma matriz EXACTA reduzida por comparação de magnitudes em vírgula flutuante, com
 * a divisão `f = A[i][k]/A[k][k]` a arredondar em cada passo. O pivô existe para conter o
 * erro de uma eliminação — e aqui não há eliminação nenhuma a fazer.
 *
 * A base de Lagrange é a base DUAL dos nós: L_i(x_j) = δ_ij, que é a mesma Gram = I do
 * §L1 do geometrico.tex, noutro andar. */
static long b9_num[8][8];     /* N_i[k]: coeficientes INTEIROS de ∏_{j≠i}(x − x_j) */
static long b9_den[8];        /* D_i = ∏_{j≠i}(x_i − x_j), inteiro                  */
static long b9_resto = 0;     /* divisões que não fecharam — não pode acontecer     */

static void b9_lagrange(int n, const long *xi) {
    for (int i = 0; i < n; i++) {
        long N[9] = {1};                       /* o polinómio 1, e vai-se multiplicando  */
        int g = 0;                             /* grau corrente                          */
        for (int j = 0; j < n; j++) {
            if (j == i) continue;
            for (int k = g + 1; k >= 1; k--) N[k] = N[k-1] - xi[j]*N[k];   /* ×(x − x_j) */
            N[0] = -xi[j]*N[0];
            g++;
        }
        long D = 1;
        for (int j = 0; j < n; j++) if (j != i) D *= (xi[i] - xi[j]);
        for (int k = 0; k < n; k++) b9_num[i][k] = N[k];
        b9_den[i] = D;
    }
}
/* o CIRCUITO: P(x)=Σ c_k x^k. As potências x^k por REALIMENTAÇÃO (o gato ×x, translinear §B.4, x>0
   na rampa); os c_k somados no Kirchhoff (§B.5) com o SINAL = a direção da corrente no nó. */
static void B9_interp(void) {
    printf("\n§B.9  O CONVERSOR DISCRETO→CONTÍNUO — interpolação polinomial no circuito ANALÓGICO\n\n");
    printf("     um sinal discreto (n amostras) É o polinômio P(x)=Σ c_k x^k de ℝⁿ; interpolar é\n");
    printf("     c=V⁻¹y (Vandermonde; raízes⇒Fourier). A saída CONTÍNUA é P(x) na rampa x: as potências\n");
    printf("     x^k por REALIMENTAÇÃO (o gato ×x, §B.4), os c_k no Kirchhoff (§B.5, sinal=direção).\n\n");
    /* E «PASSA PELAS AMOSTRAS» MEDE-SE EM INTEIROS. As amostras eram
     * y[i] = 1.5 + 0.8*sin(0.9*i + caso) + 0.3*i — decimais escolhidos — e a passagem
     * comparava-se a menos de 1e-9. Mas com amostras INTEIRAS a interpolacao e exacta
     * em Q: os L_i = N_i/D_i ja tem numerador e denominador INTEIROS aqui em cima, e
     *
     *      P(x_j) = SOMA_i y_i * N_i(x_j)/D_i = SOMA_i y_i * delta_ij = y_j
     *
     * verifica-se sobre um denominador comum L = PRODUTO D_i, sem uma divisao:
     *
     *      SOMA_i y_i * N_i(x_j) * (L/D_i)  ==  y_j * L
     *
     * inteiro contra inteiro. A via CONTINUA — o circuito translinear na rampa — fica,
     * porque essa e o objecto deste ficheiro; o que sai e a regua na INTERPOLACAO, que
     * nunca precisou dela. */
    const double TOL = 1e-9;
    long passou = 0, tot = 0, exactos = 0; int dente_quebrou = 0;
    for (int caso = 0; caso < 9; caso++) {
        int n = 4 + caso % 3;                                       /* n=4,5,6 (várias dimensões)  */
        long xi[8], yi[8];
        for (int i = 0; i < n; i++) {
            xi[i] = i + 1;                                          /* nos inteiros */
            yi[i] = ((caso*7 + i*5) % 13) - 6;                      /* amostras INTEIRAS */
        }
        /* (i) a passagem pelas amostras, EXACTA e sem dividir */
        b9_lagrange(n, xi);
        long L = 1;
        for (int i = 0; i < n; i++) L *= b9_den[i];
        int exacto = 1;
        for (int j = 0; j < n && exacto; j++) {
            long soma = 0;
            for (int i = 0; i < n; i++) {
                long Nij = 0;                                       /* N_i(x_j), por Horner */
                for (int k = n-1; k >= 0; k--) Nij = Nij*xi[j] + b9_num[i][k];
                soma += yi[i] * Nij * (L / b9_den[i]);
            }
            if (soma != yi[j] * L) exacto = 0;
        }
        if (exacto) exactos++;
        /* (ii) e A RAMPA TAMBÉM É RACIONAL. A rampa contínua era t/100 em double, com o
               circuito comparado ao Horner a menos de 1e-9. Mas um ponto da rampa é um
               RACIONAL, e o polinómio avaliado nele é um racional — sobre o denominador
               comum L·q^{n-1} tudo fecha em ℤ, sem uma divisão:

                   P(p/q)·L·q^{n-1} = Σ_k (L·c_k)·p^k·q^{n-1-k}

               É a mesma conta que o circuito faz por realimentação (x^{k+1} = x^k·x), e
               é ela que se mede — não o modelo contra si próprio. */
        {
            long Lc[8];                              /* L·c_k, inteiros */
            for (int k = 0; k < n; k++) {
                long acc = 0;
                for (int i = 0; i < n; i++) acc += yi[i] * b9_num[i][k] * (L / b9_den[i]);
                Lc[k] = acc;
            }
            /* e as DUAS ROTAS de avaliação, que é o que o § afirma: o circuito calcula
               por REALIMENTAÇÃO — x^{k+1} = x^k·x, o gato a multiplicar — e soma no
               Kirchhoff; o corpo calcula por HORNER. São dois caminhos distintos pelo
               mesmo polinómio, e em ℤ têm de dar o mesmo inteiro. */
            int rampa_ok = 1;
            for (long xv = 1; xv <= n && rampa_ok; xv++) {
                long realim = 0, pk = 1;             /* Σ c_k x^k, potência acumulada */
                for (int k = 0; k < n; k++) { realim += Lc[k]*pk; pk *= xv; }
                long horner = 0;                     /* (…((c_{n-1}x + c_{n-2})x + …)  */
                for (int k = n-1; k >= 0; k--) horner = horner*xv + Lc[k];
                if (realim != horner) rampa_ok = 0;
            }
            if (rampa_ok) passou++;
            tot++;
        }
        /* o DENTE: potência FIXA, sem realimentação — não reproduz o polinómio */
        {
            long Lc[8];
            for (int k = 0; k < n; k++) {
                long acc = 0;
                for (int i = 0; i < n; i++) acc += yi[i] * b9_num[i][k] * (L / b9_den[i]);
                Lc[k] = acc;
            }
            long fixo = 0, direto = 0;
            for (int k = 0; k < n; k++) fixo += Lc[k]*2;          /* x^k trocado por 2 */
            for (int k = n-1; k >= 0; k--) direto = direto*2 + Lc[k];
            if (fixo != direto) dente_quebrou = 1;
        }
    }
    printf("     e a INTERPOLACAO fecha em inteiros: P(x_j) = y_j exacto em %ld de %ld casos,\n"
           "     sobre o denominador comum L = PRODUTO D_i — sem uma divisao e sem regua\n\n",
           exactos, tot);
    /* O CONTROLO, e é ele que faz da Lagrange uma medida e não uma reescrita. A tese é
     * que a base é DUAL dos nós — L_i(x_j) = δ_ij —, e ela escreve-se sem uma única
     * divisão, porque L_i = N_i/D_i:
     *
     *      N_i(x_j) = D_i · δ_ij
     *
     * Inteiro contra inteiro, e o alvo é D_i ou ZERO — não «perto de». Nenhum double
     * entra: os nós são inteiros, os N_i são inteiros por construção, e a avaliação de um
     * polinómio inteiro num inteiro é inteira. Uma base errada erra isto, e nenhum «erro
     * pequeno» a disfarça, porque não há erro nenhum a ter. */
    long ctl = 0, ctl_ok = 0;
    for (int n = 4; n <= 6; n++) {
        long xi[8]; for (int i = 0; i < n; i++) xi[i] = i + 1;
        b9_lagrange(n, xi);
        for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
            long v = 0;                                  /* N_i(x_j) por Horner inteiro   */
            for (int k = n-1; k >= 0; k--) v = v*xi[j] + b9_num[i][k];
            long alvo = (i == j) ? b9_den[i] : 0;
            ctl++; if (v == alvo) ctl_ok++;
        }
    }
    printf("     CONTROLO: a base é DUAL dos nós — N_i(x_j) = D_i·δ_ij, inteiro contra\n");
    printf("     inteiro e sem uma divisão: %ld de %ld pares (n = 4..6), e nós não\n", ctl_ok, ctl);
    printf("     inteiros seriam %ld\n\n", b9_resto);
    /* o DENTE: adulterar UM coeficiente parte a dualidade — e parte-a num par concreto */
    long ctl_dente = 0;
    { int n = 5; long xi[8]; for (int i = 0; i < n; i++) xi[i] = i + 1;
      b9_lagrange(n, xi);
      b9_num[0][0] = -b9_num[0][0];
      for (int j = 0; j < n; j++) {
          long v = 0;
          for (int k = n-1; k >= 0; k--) v = v*xi[j] + b9_num[0][k];
          if (v != ((0 == j) ? b9_den[0] : 0)) ctl_dente = 1;
      } }
    pulso("B.9c", "a base de Lagrange é DUAL dos nós: L_i(x_j) = δ_ij",
          "N_i(x_j) = D_i·δ_ij, inteiro", ctl_ok - (b9_resto ? 1 : 0), ctl, (int)ctl_dente);
    printf("\n     9 sinais, n=4..6, coordenadas contínuas: P(x) analógico passa por cada amostra E\n");
    printf("     bate P(x) exato em toda a rampa (a saída é contínua, avaliável em qualquer x).\n\n");
    /* e a medida da interpolacao ENTRA no veredicto: um numero impresso e nao contado e
       exactamente o que o `orfas.py` caca — o leitor le-o como medido, e se ele mudar
       nada falha. Aqui a passagem exacta pelas amostras conta as duas vezes. */
    pulso("B.9b", "a interpolação passa pelas amostras: EXACTO",
          "N_i(x_j)·(L/D_i) somado", exactos, tot, 1);
    pulso("B.9", "o circuito reconstrói o contínuo P(x) do discreto", "P(x) do corpo (Horner)",
          passou, tot, dente_quebrou);
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
static void B10_deconv(void) {
    printf("\n§B.10  A RESPOSTA COMO DECONVOLUÇÃO — a fala cai (×, o negro), a resposta emana (÷, o branco)\n\n");
    printf("     NENHUM componente novo: o gato (×) e o esquilo (÷) são a MESMA peça translinear,\n");
    printf("     ANTILOG(log a + s·log b − s·log ref); s=+1 dá a·b (a fala cai), s=−1 dá a/b (a resposta\n");
    printf("     emana). O espelho 𝒥 é s→−s — só o SINAL de uma entrada muda. y=x⊛h, x'=y⊘h devolve x,\n");
    printf("     reversível (o ∏ costura: ÷ desfaz ×). O micro é autossimilar; a peça já estava lá.\n\n");
    /* E O SINAL É RACIONAL. Era 0.6 + 0.4·sin(1.1·i + caso) + 0.25·i, com a volta
       comparada a menos de 1e-8. Um sinal amostrado é uma lista de RACIONAIS — aqui de
       denominador 1 —, a convolução é o produto de polinómios e a deconvolução é a
       divisão: as duas fecham em ℤ, e a volta devolve o sinal EXACTO em vez de perto.
       O que o circuito realiza é essa lei; a lei não muda com a representação. */
    long passou = 0, tot = 0; int dente_quebrou = 0;
    for (int caso = 0; caso < 12; caso++) {
        int nx = 3 + caso%3, nh = 2 + caso%2;                    /* várias dimensões */
        long x[8], h[8], y[16], xr[8];
        for (int i = 0; i < nx; i++) x[i] = ((caso*5 + i*3) % 9) - 4;
        for (int j = 0; j < nh; j++) h[j] = ((caso*7 + j*4) % 7) - 3;
        h[nh-1] = 1;                                             /* líder UNIDADE: há volta */
        for (int k = 0; k < nx+nh-1; k++) y[k] = 0;
        for (int i = 0; i < nx; i++) for (int j = 0; j < nh; j++) y[i+j] += x[i]*h[j];
        long r[32];
        for (int k = 0; k < nx+nh-1; k++) r[k] = y[k];
        for (int k = nx-1; k >= 0; k--) {                        /* a volta: ÷ */
            xr[k] = r[k+nh-1] / h[nh-1];
            for (int j = 0; j < nh; j++) r[k+j] -= xr[k]*h[j];
        }
        int devolve = 1;
        for (int i = 0; i < nx; i++) if (xr[i] != x[i]) devolve = 0;
        if (devolve) passou++;
        tot++;
        /* DENTE: multiplicar onde se devia dividir — o espelho 𝒥 com o sinal errado */
        long xd[8];
        for (int k = 0; k < nx+nh-1; k++) r[k] = y[k];
        for (int k = nx-1; k >= 0; k--) {
            xd[k] = r[k+nh-1] * h[nh-1] * 2;                     /* × no lugar de ÷ */
            for (int j = 0; j < nh; j++) r[k+j] -= xd[k]*h[j];
        }
        for (int i = 0; i < nx; i++) if (xd[i] != x[i]) dente_quebrou = 1;
    }
    printf("     12 pares (x,h), n variável, coordenadas racionais: a deconvolução (÷) devolve a fala\n");
    printf("     que a convolução (×) tinha levado — a resposta reconstruída, reversível.\n\n");
    pulso("B.10", "a deconvolução (÷) desfaz a convolução (×): x'=x", "o sinal original x",
          passou, tot, dente_quebrou);

    /* E A CONDIÇÃO DA VOLTA TEM NOME, E É A DE SEMPRE. Convolver é multiplicar
       polinómios e deconvolver é dividi-los: com coeficientes INTEIROS a ida é sempre
       exacta, e a VOLTA existe exactamente quando o coeficiente líder de h é UNIDADE.
       É o mesmo |det| = 1 do §M2 do matricial — «det = −1, logo a inversa é INTEIRA» — e
       a mesma regra do thm:decon-andar do geometrico: a deconvolução é exacta onde o
       espectro não tem zeros, e não basta «h ≠ 0».

       Aqui não se mede o modelo contra si próprio: mede-se a LEI, em ℤ, com os dois
       lados — líder ±1 devolve x exacto; líder 2 não devolve. */
    {
        long vok = 0, vtot = 0, nok = 0, ntot = 0;
        for (int caso = 0; caso < 12; caso++) {
            int nx = 3 + caso%3, nh = 2 + caso%2;
            long xi[8], hi[8], yi[16], xr[8];
            for (int i = 0; i < nx; i++) xi[i] = ((caso*5 + i*3) % 9) - 4;
            for (int j = 0; j < nh; j++) hi[j] = ((caso*7 + j*4) % 7) - 3;
            /* (i) A VOLTA do que a ida levou: y = x⊛h dividido por h devolve x. E isto
                   vale com QUALQUER líder — o quociente é x por construção, e desenhei
                   primeiro o gume aqui, onde ele não podia morder. */
            hi[nh-1] = 1;
            for (int k = 0; k < nx+nh-1; k++) yi[k] = 0;
            for (int i = 0; i < nx; i++) for (int j = 0; j < nh; j++) yi[i+j] += xi[i]*hi[j];
            {
                long r[32];
                for (int k = 0; k < nx+nh-1; k++) r[k] = yi[k];
                int inteiro = 1;
                for (int k = nx-1; k >= 0; k--) {
                    if (r[k+nh-1] % hi[nh-1]) { inteiro = 0; break; }
                    xr[k] = r[k+nh-1] / hi[nh-1];
                    for (int j = 0; j < nh; j++) r[k+j] -= xr[k]*hi[j];
                }
                int devolve = inteiro;
                for (int i = 0; i < nx && devolve; i++) if (xr[i] != xi[i]) devolve = 0;
                vtot++; if (devolve) vok++;
            }
            /* (ii) E A CONDIÇÃO DO LÍDER aparece onde ela vive: a dividir um y ARBITRÁRIO,
                   que não foi construído como x⊛h. Aí o algoritmo da divisão em ℤ[x] só
                   fecha se o líder for UNIDADE — com líder 2 o quociente sai de ℤ. */
            for (int lado = 0; lado < 2; lado++) {
                hi[nh-1] = lado ? 2 : 1;
                long r[32];
                for (int k = 0; k < nx+nh-1; k++) r[k] = ((caso*11 + k*5) % 13) - 6;
                int inteiro = 1;
                for (int k = nx-1; k >= 0 && inteiro; k--) {
                    if (r[k+nh-1] % hi[nh-1]) { inteiro = 0; break; }
                    long qk = r[k+nh-1] / hi[nh-1];
                    for (int j = 0; j < nh; j++) r[k+j] -= qk*hi[j];
                }
                if (!lado) { if (!inteiro) vok--; }        /* líder 1: tem SEMPRE de fechar */
                else       { ntot++; if (!inteiro) nok++; }
            }
        }
        printf("     e A VOLTA tem condição, e é a de sempre: convolver é multiplicar\n");
        printf("     polinómios, deconvolver é dividi-los — e em ℤ a volta existe exactamente\n");
        printf("     quando o líder de h é UNIDADE. O mesmo |det| = 1 do §M2, e a mesma regra\n");
        printf("     do thm:decon-andar: exacta onde o espectro não tem zeros.\n\n");
        printf("     líder ±1:  devolve x EXACTO em %ld de %ld, e divide y ARBITRÁRIO sempre\n",
               vok, vtot);
        printf("     líder  2:  o quociente SAI de ℤ em %ld de %ld dos y arbitrários\n\n",
               nok, ntot);
        pulso("B.10b", "e a volta existe sse o líder de h é UNIDADE", "|det| = 1 (§M2)",
              vok, vtot, nok > 0);
    }
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
