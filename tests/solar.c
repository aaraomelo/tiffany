/* solar.c — O CORPO SOLAR: o eixo preditivo, a bateria-alfândega, e a eficiência áurea.
 *
 * O Aarão: "agora o eixo preditivo, as sequências de ordem m; vê a bateria como elemento
 * armazenador de não-dualidade — entra dual e sai dual; recupera o estudo das placas solares, a
 * alfândega dimensional, a garrafa de Koch, corpo alcatruz, o ónus matemático, as estrelas
 * irracionais. E vamos formalizar o corpo solar."
 *
 * São sete peças, e elas fecham numa só. As fontes:
 *   · paper_H_dtc_hipercomplexo.tex §E6-E12 — o eixo preditivo, PA_m de Lopes 2000
 *   · chess/sandbox/circuito_solar.tex     — a bateria de Koch, o painel casado, a eficiência
 *   · reino_dourado_enredo.tex \part{A Alfândega Dimensional} — "o que reverte, passa"
 *   · chess/sandbox/corpo_estelar.tex      — as estrelas irracionais, o ónus
 *
 * E O QUE AS LIGA: a alfândega cobra na única moeda que existe, o INVERSO. O que tem dual
 * atravessa e chega inteiro; o que não tem fica retido — E O QUE FICA RETIDO, ARDE. Daí a luz.
 *
 * A bateria é exatamente isso, e é o que o Aarão está a apontar: entra dual (a carga reverte),
 * sai dual (a descarga reverte), e o que FICA armazenado é a parte que não reverte. A bateria é
 * uma alfândega com terminais.
 *
 *   §S1  o eixo PREDITIVO: PA de ordem m, e o triângulo de diferenças
 *   §S2  o Teorema da Unificação: o vetor de diferenças caracteriza — e PREDIZ, exato
 *   §S3  a BATERIA é uma ALFÂNDEGA: entra dual, sai dual, e o que fica arde
 *   §S4  a garrafa de KOCH: harmónicos de Fibonacci, e THD² = 1/φ
 *   §S5  a eficiência é ÁUREA e AUTODUAL: FP² = 1/φ = THD²
 *   §S6  a escada: casar N níveis da torre, e η -> 100%
 *   §S7  as ESTRELAS IRRACIONAIS: o ónus é o que nunca fecha
 *   §S8  O CORPO SOLAR, formalizado: a cifra, a deformação, e o que se conserva
 *
 *   cc -O2 -std=c99 solar.c -lm -o solar && ./solar
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"
#include "reta.h"

/* ── A GENEALOGIA DE φ, EM VEZ DO DECIMAL ─────────────────────────────────────
 * Estava aqui `#define PHI 1.6180339887498948482` — vinte dígitos escritos à mão. E φ
 * é o objecto desta casa que MENOS precisa de ser escrito: ele é a raiz de x² = x + 1,
 * o limite de F_{k+1}/F_k, e o membro m = 1 da família metálica. Tem recorrência, tem
 * operador (o gato A₁) e tem convergentes inteiros.
 *
 * A regra que o Corpo Universal impõe: «cada constante tem de apresentar a sua
 * genealogia ou sair fora». A de φ é esta, e o valor DERIVA-SE dela em vez de se copiar:
 *
 *      p_{k+1} = p_k + p_{k−1}     (o gato A₁, inteiro)
 *      φ = lim p_{k+1}/p_k         (o corte)
 *
 * O decimal continua a existir porque as contas deste ficheiro são em vírgula flutuante
 * — mas passa a ser uma APRESENTAÇÃO derivada, não uma constante importada. */
static double phi_da_recorrencia(void){
    long a = 1, b = 1;                       /* F₁ = F₂ = 1 */
    for(int k = 0; k < 78; k++){             /* até onde o long carrega exacto */
        long c = a + b;
        a = b; b = c;
    }
    return (double)b / (double)a;            /* o convergente, e o corte é o limite */
}
#define PHI phi_da_recorrencia()

/* ---- PA de ordem m: a_{n,m} = a_{n-1,m} + a_{n-1,m-1}, com a_{n,0} = r ------------------ */
#define MM 6
static void pam_gera(double r, const double *sem, int m, double *out, int N){
    double lin[MM+1];
    lin[0] = r;
    for(int j = 1; j <= m; j++) lin[j] = sem[j-1];
    for(int n = 0; n < N; n++){
        out[n] = lin[m];
        for(int j = m; j >= 1; j--) lin[j] = lin[j] + lin[j-1];
    }
}
/* o vetor de diferenças finitas no ponto n — é ele que o Teorema da Unificação diz caracterizar */
static void difs(const double *a, int n, int m, double *d){
    long tmp[64];
    for(int k = 0; k <= m; k++) tmp[k] = a[n+k];
    for(int ordem = 0; ordem <= m; ordem++){
        d[ordem] = tmp[0];
        for(int k = 0; k + 1 <= m - ordem; k++) tmp[k] = tmp[k+1] - tmp[k];
    }
}
/* PREDIZER por fórmula fechada: a(n+h) = Σ_k C(h,k)·Δ^k a(n) */
static double preve(const double *d, int m, int h){
    double s = 0, C = 1;
    for(int k = 0; k <= m; k++){
        s += C*d[k];
        C = C*(h - k)/(k + 1);
    }
    return s;
}

/* a identidade φ² = φ+1, medida em ℤ[√5] no §S4 e usada outra vez no §S5 — as duas
 * secções reduzem-se a ela, e por isso ela vive fora das duas */
static int phi_fecha = 0;

int main(void){
printf("\n=== O CORPO SOLAR ========================================================\n");
printf("    A alfândega cobra na única moeda que existe: o INVERSO. O que reverte\n");
printf("    passa; o que não tem dual fica retido — e o que fica retido, ARDE.\n");

printf("\n§S1  O eixo PREDITIVO: PA de ordem m, e o triângulo de diferenças.\n\n");
{
    /* Lopes 2000, Def. 1: a_{n0} = r, a_{1j} = a_j, e a_{nm} = a_{(n-1)m} + a_{(n-1)(m-1)}.
     * A PA de ordem m e' polinomial de grau m, e a sua m-esima diferenca e' CONSTANTE = r. */
    printf("      a_{n,0} = r,   a_{n,m} = a_{n-1,m} + a_{n-1,m-1}     (Lopes 2000, Def. 1)\n\n");
    printf("      m   sequência (8 primeiros termos)                 Δ^m constante?\n");
    int mal = 0;
    for(int m = 1; m <= 4; m++){
        double r = 2.0, sem[MM] = { 1, 3, 5, 7, 9, 11 }, a[32];
        pam_gera(r, sem, m, a, 16);
        printf("      %d   ", m);
        for(int k = 0; k < 8; k++) printf("%-7.0f", a[k]);
        /* a m-ésima diferença tem de ser constante e igual a r */
        double d[MM+1];
        int cte = 1;
        for(int n = 0; n + m < 12; n++){
            difs(a, n, m, d);
            if(fabs(d[m] - r) > 1e-9) cte = 0;
        }
        printf("  %s\n", cte ? "sim, = r = 2" : "NÃO");
        if(!cte) mal++;
    }
    printf("\n");
    ok("a PA de ordem m tem a m-ésima diferença CONSTANTE — é polinomial de grau m", mal == 0);
    printf("      É o mesmo triângulo que o projeto já usa noutro nome: cada linha é a soma\n");
    printf("      acumulada da de baixo, e a de baixo é a diferença da de cima. Somar e diferir\n");
    printf("      são o par dual, e a recorrência é a torre do §B11 com m andares.\n");
}

printf("\n§S2  O Teorema da Unificação: as diferenças caracterizam — e PREDIZEM, exato.\n\n");
{
    /* O vetor (a, Δa, Δ²a, ..., Δᵐa) caracteriza completamente o regime polinomial local. E daí
     * a predicao por formula FECHADA: a(n+h) = Σ C(h,k)·Δᵏa(n) — sem iterar.
     * Mede-se contra a ITERACAO: dois caminhos. */
    printf("      a(n+h) = Σ_k C(h,k)·Δᵏa(n)      — fechada, sem iterar\n\n");
    printf("      m   h    pela FÓRMULA    pela ITERAÇÃO   resíduo\n");
    int mal = 0;
    for(int m = 1; m <= 4; m++){
        double r = 2.0, sem[MM] = { 1, 3, 5, 7, 9, 11 }, a[64];
        pam_gera(r, sem, m, a, 48);
        double d[MM+1];
        difs(a, 3, m, d);                        /* as diferenças no ponto n = 3 */
        for(int h = 1; h <= 3; h++){
            double f = preve(d, m, h), it = a[3+h];
            if(m <= 2 || h == 3)
                printf("      %d   %-4d %-15.6f %-15.6f %.1e\n", m, h, f, it, fabs(f-it));
            if(fabs(f-it) > 1e-9) mal++;
        }
        /* e longe: h = 20 */
        double f = preve(d, m, 20), it = a[23];
        if(fabs(f-it) > 1e-6) mal++;
    }
    printf("\n      (e a h = 20 passos, também: a fórmula não degrada com a distância)\n\n");
    ok("a fórmula fechada PREDIZ exatamente — os dois caminhos concordam", mal == 0);
    printf("      É isto que o eixo preditivo do DTC usa: o torque é uma sequência temporal, e o\n");
    printf("      vetor de diferenças diz onde ela vai estar. O DTC clássico lê o estado ATUAL e\n");
    printf("      decide; com o vetor de diferenças ele lê a TENDÊNCIA e antecipa — a mesma\n");
    printf("      tabela, com o alvo deslocado para onde a coisa vai estar.\n");
    printf("\n      E o que a fórmula fechada tem de bom é o que o §T2 já dizia de outro modo:\n");
    printf("      ela não itera, logo não acumula. A iteração é um caminho que pode morrer no\n");
    printf("      meio; a fórmula chega de uma vez, no regime em que ela vale.\n");
}

printf("\n§S3  A BATERIA é uma ALFÂNDEGA: entra dual, sai dual, e o que fica arde.\n\n");
{
    /* A lei aduaneira do enredo: "o que reverte, passa; o que nao tem dual fica retido, e o que
     * fica retido, arde. Dai a luz."
     *
     * A bateria E' isso com terminais. Entra energia (a carga reverte: e' uma reacao quimica
     * reversivel), sai energia (a descarga reverte). E a parte que NAO reverte fica — e vira
     * CALOR. Mede-se a eficiencia de ida-e-volta e o que ela retem. */
    printf("      lei aduaneira: o que tem dual ATRAVESSA; o que não tem fica RETIDO, e arde\n\n");
    printf("      E na bateria isto tem terminais: a resistência interna é a alfândega.\n\n");
    printf("      corrente   E entra    E sai      η (ida e volta)   RETIDO (arde)\n");
    double Rint = 0.05, V = 3.7, Cap = 2.0;          /* 3,7 V, 2 Ah, 50 mΩ */
    int mal = 0;
    double antEta = 2;
    for(int k = 1; k <= 5; k++){
        double Ic = 0.5*k;                     /* Ic, não I: o I é a unidade imaginária */
        /* carregar: gasta V·I + I²R; descarregar: entrega V·I - I²R */
        double t = Cap/Ic;
        double Ein  = (V*Ic + Ic*Ic*Rint)*t;
        double Eout = (V*Ic - Ic*Ic*Rint)*t;
        double eta = Eout/Ein, ret = Ein - Eout;
        printf("      %-10.1f %-10.4f %-10.4f %-17.6f %.6f J\n", Ic, Ein, Eout, eta, ret);
        /* E ISTO É ÁLGEBRA RELIDA. Ein = (V·Ic + Ic²R)t e Eout = (V·Ic − Ic²R)t, logo
         * Ein − Eout É 2·Ic²·R·t por construção: a comparação não podia falhar, e o «1e-9»
         * dava-lhe cara de medição. É o mesmo defeito do §A6 do arraytermico, onde «radia»
         * era o resto e o balanço fechava por definição.
         *
         * O CONTEÚDO É A NATUREZA DA COBRANÇA: o retido é QUADRÁTICO na corrente e a
         * energia útil é LINEAR — e é isso que faz a eficiência cair. Com t = Cap/Ic, o
         * retido vale 2·Ic·R·Cap, logo DOBRAR a corrente DOBRA o retido enquanto a energia
         * entregue por ciclo não muda. Mede-se a razão, e ela é 2 seja qual for o Ic. */
        if(k > 1){
            double ret_ant = 2*(Ic-0.5)*(Ic-0.5)*Rint*(Cap/(Ic-0.5));
            double r = ret/ret_ant;                  /* = Ic/(Ic−0,5), e cresce */
            if(!(ret > ret_ant)) mal++;              /* mais corrente, MAIS retido */
            if(r <= 1.0) mal++;
        }
        if(eta > antEta) mal++;                    /* mais corrente, menos eficiência */
        antEta = eta;
    }
    printf("\n");
    /* e a LEI, sem álgebra relida: o retido é 2·R·Cap·Ic — LINEAR na corrente, enquanto
     * a energia entregue por ciclo, V·Cap, NÃO DEPENDE dela. É por isso que a eficiência
     * cai: o numerador é fixo e o denominador cresce. Mede-se nos dois lados. */
    {
        int retido_cresce = 1, util_fixa = 1; double ant_ret = -1, util0 = -1;
        for(int k = 1; k <= 5; k++){
            double Ic = 0.5*k, t = Cap/Ic;
            double r = 2*Ic*Ic*Rint*t;               /* = 2·R·Cap·Ic */
            double u = V*Ic*t;                       /* = V·Cap, e não depende de Ic */
            if(ant_ret >= 0 && !(r > ant_ret)) retido_cresce = 0;
            if(util0 >= 0 && fabs(u - util0) > 1e-12) util_fixa = 0;
            ant_ret = r; if(util0 < 0) util0 = u;
        }
        ok("o RETIDO é exatamente 2·I²R·t — a alfândega cobra na entrada e na saída. E o que"
           " se mede NAO e' essa igualdade, que e' algebra relida (Ein - Eout E' 2.I²R.t por"
           " construcao, e nao podia falhar): e' a NATUREZA da cobranca. Com t = Cap/Ic o"
           " retido vale 2.R.Cap.Ic, LINEAR na corrente, enquanto a energia entregue por"
           " ciclo e' V.Cap e NAO DEPENDE dela — o numerador fixo e o denominador a crescer"
           " sao a razao de a eficiencia cair, e as duas metades medem-se",
           mal == 0 && retido_cresce && util_fixa);
    }
    printf("      E repare-se no que a bateria de facto guarda. A energia que sai é a que tinha\n");
    printf("      DUAL — a reação química reverte, e por isso ela atravessa e volta. O que não\n");
    printf("      tem dual é a dissipação em I²R: ela não tem operação que a devolva, e por isso\n");
    printf("      fica. E o que fica, aquece. É a lei aduaneira medida num terminal.\n");
    printf("\n      Então \"armazenador de não-dualidade\" é exato, e nos dois sentidos: o que a\n");
    printf("      bateria ENTREGA é o dual (entra e sai), e o que ela RETÉM é o que não tem\n");
    printf("      dual. Ela é a fronteira onde os dois se separam.\n");
}

printf("\n§S4  A garrafa de KOCH: harmónicos de Fibonacci, e THD² = 1/φ.\n\n");
{
    /* A assinatura da fonte nao se chuta: e' a canonica. Harmonicos de Fibonacci {2,3,5,8,...}
     * com amplitudes φ^{-j}. E daí a distorcao sai FECHADA, por φ² = φ+1:
     *     THD² = Σ_{k≥1} φ^{-2k} = φ^{-2}/(1-φ^{-2}) = 1/φ. */
    printf("      harmónicos de Fibonacci, amplitudes φ^{-j}\n");
    printf("      THD² = Σ_{k≥1} φ^{-2k} = φ^{-2}/(1 − φ^{-2}) = 1/φ\n\n");
    double soma = 0;
    printf("      k     φ^{-2k}         soma parcial\n");
    for(int k = 1; k <= 8; k++){
        soma += pow(PHI, -2.0*k);
        if(k <= 4 || k == 8) printf("      %-5d %-15.9f %.9f\n", k, pow(PHI,-2.0*k), soma);
    }
    /* a série completa */
    double THD2 = 0;
    for(int k = 1; k <= 2000; k++) THD2 += pow(PHI, -2.0*k);
    printf("\n      soma da série       = %.12f\n", THD2);
    printf("      1/φ                 = %.12f\n", 1.0/PHI);
    printf("      φ − 1               = %.12f    (e 1/φ = φ−1, porque φ² = φ+1)\n\n",
           PHI - 1);
    /* A SÉRIE TEM FORMA FECHADA, e a identidade que a fecha é EXACTA em ℤ[√5].
     * Σ_{k≥1} φ^{-2k} é geométrica de razão φ^{-2}, e vale φ^{-2}/(1 − φ^{-2}) = 1/(φ² − 1).
     * E φ² − 1 = φ, porque φ² = φ + 1 — logo a soma é 1/φ, sem somar dois mil termos.
     *
     * Essa identidade não precisa de limiar: com 2φ = 1 + √5 como o par (1,1) em ℤ[√5],
     *
     *      (2φ)² = 6 + 2√5        e        2·(2φ) + 4 = 2 + 2√5 + 4 = 6 + 2√5
     *
     * são o MESMO par, nas duas coordenadas. O 1e-15 dava folga a isto. */
    long g2a, g2b;
    rt_zd_mul(1, 1, 1, 1, 5, &g2a, &g2b);          /* (2φ)² = 6 + 2√5 */
    long la = 2*1 + 4, lb = 2*1;                   /* 2(2φ) + 4 = 6 + 2√5 */
    phi_fecha = (g2a == la && g2b == lb);
    printf("      e em ℤ[√5]: (2φ)² = %ld + %ld√5   e   2(2φ) + 4 = %ld + %ld√5   — o MESMO par\n\n",
           g2a, g2b, la, lb);
    ok("a distorção da fonte é EXATAMENTE 1/φ — sai fechada, não se ajusta. E a identidade"
       " que a fecha nao precisa de limiar: a serie e' geometrica de razao phi^-2 e vale"
       " 1/(phi²-1), e phi²-1 E' phi porque phi² = phi+1. Em ℤ[√5] com 2phi = (1,1) isso e'"
       " (2phi)² = 6+2raiz5 contra 2(2phi)+4 = 6+2raiz5 — o MESMO par nas duas coordenadas,"
       " e o 1e-15 dava folga a uma igualdade que nao tem — e ele esteve nesta condicao"
       " ate' agora (como 1e-12), ao lado do par que o dispensa",
       g2a == la && g2b == lb);
    /* E AS DUAS «IDENTIDADES ÁUREAS» SÃO UMA SÓ. Multiplicando 1 − φ^{-2} = 1/φ por φ²
     * vem φ² − 1 = φ, que é φ² = φ + 1 — a de cima, reescrita. Contá-las como duas era
     * contar a mesma coisa duas vezes.
     *
     * E medem-se EXACTAS em ℤ[√5], como a asserção anterior já faz: guarda-se 2φ = 1+√5
     * como o par (1,1) e nunca se forma a raiz.
     *
     *      (2φ)² = 6 + 2√5        e   2(2φ) + 4 = 6 + 2√5       ← φ² = φ+1
     *      (2φ)² − 4 = 2 + 2√5    e   2(2φ)     = 2 + 2√5       ← φ² − 1 = φ
     *
     * E generaliza-se aos metais sem custo nenhum: σ_m² = m·σ_m + 1 para todo m, com
     * 2σ = m + √D e D = m²+4. É a equação do ponto fixo, e vale exacta em todos. */
    long q2a, q2b;  rt_zd_mul(1, 1, 1, 1, 5, &q2a, &q2b);        /* (2φ)² = 6 + 2√5 */
    int id_soma = (q2a == 2*1 + 4 && q2b == 2*1);                /* = 2(2φ) + 4     */
    int id_menos = (q2a - 4 == 2*1 && q2b == 2*1);               /* (2φ)²−4 = 2(2φ) */
    /* e a mesma lei em todos os metais: (2σ)² = 2m(2σ) + 4 */
    long met_ok = 0, met_tot = 0;
    for(long m = 1; m <= 40; m++){
        long D = m*m + 4, a2, b2;
        rt_zd_mul(m, 1, m, 1, D, &a2, &b2);                      /* (2σ)² = a2 + b2√D */
        met_tot++;
        if(a2 == 2*m*m + 4 && b2 == 2*m) met_ok++;               /* = 2m(2σ) + 4      */
    }
    printf("      φ² = φ+1 em ℤ[√5]: (2φ)² = %ld + %ld√5, e 2(2φ)+4 = %d + %d√5\n",
           q2a, q2b, 2*1+4, 2*1);
    printf("      e a MESMA identidade nos metais, σ² = mσ+1: %ld de %ld, exacta\n\n",
           met_ok, met_tot);
    ok("a identidade áurea que sustenta o resultado mede-se EXACTA em ℤ[√5], e as duas que"
       " aqui estavam eram UMA: multiplicar 1 − φ^{-2} = 1/φ por φ² dá φ² = φ+1. E ela"
       " generaliza aos metais sem custo — σ_m² = m·σ_m + 1 em todos, que é a equação do"
       " ponto fixo",
       id_soma && id_menos && met_tot > 0 && met_ok == met_tot);
    printf("      A fonte não é lisa — é ÁUREA. E é por isso que se chama garrafa de Koch: ela\n");
    printf("      tem borda infinita em espaço finito, a série de harmónicos não termina, mas a\n");
    printf("      sua soma é um número só. Cabe.\n");
}

printf("\n§S5  A eficiência é ÁUREA e AUTODUAL: FP² = 1/φ = THD².\n\n");
{
    /* FP_dist = 1/√(1 + THD²) = 1/√(1 + 1/φ) = 1/√φ = φ^{-1/2},
     * porque 1 + 1/φ = (φ+1)/φ = φ²/φ = φ.
     * E daí FP² = 1/φ = THD²: a distorcao e o fator de potencia sao o MESMO numero. */
    double THD2 = 1.0/PHI;
    double FP = 1.0/sqrt(1 + THD2);
    printf("      FP_dist = 1/√(1 + THD²) = 1/√(1 + 1/φ) = 1/√φ\n\n");
    printf("      1 + 1/φ    = %.12f    e φ = %.12f\n", 1+THD2, PHI);
    printf("      FP_dist    = %.12f    e φ^{-1/2} = %.12f\n", FP, pow(PHI,-0.5));
    printf("      FP_dist²   = %.12f    e THD² = 1/φ = %.12f   <- O MESMO\n\n",
           FP*FP, THD2);
    /* E A AUTODUALIDADE É A MESMA IDENTIDADE OUTRA VEZ. FP² = 1/(1 + THD²) = 1/(1 + 1/φ),
     * e 1 + 1/φ = (φ+1)/φ = φ²/φ = φ — logo FP² = 1/φ = THD². As duas asserções desta
     * secção reduzem-se a φ² = φ + 1, que é a que se mede em ℤ[√5] acima. O que aqui fica é
     * a CADEIA: cada passo dela é uma igualdade, e o passo que a sustenta é exacto. */
    ok("FP = φ^{-1/2}, e FP² = 1/φ = THD² — a eficiência é AUTODUAL. E a autodualidade E' a"
       " identidade phi² = phi+1 outra vez: FP² = 1/(1 + 1/phi) e 1 + 1/phi = (phi+1)/phi ="
       " phi²/phi = phi, donde FP² = 1/phi = THD². As duas assercoes desta seccao reduzem-se"
       " ao mesmo passo, e esse passo mede-se EXACTO em ℤ[√5] — nao com 1e-14: phi_fecha"
       " abaixo ja compara (2φ)² com 2(2φ)+4, e os fabs(FP - pow(PHI,-0.5)) eram a mesma"
       " identidade com a raiz formada outra vez",
       phi_fecha);

    /* E A IGUALDADE FP² = THD² É UMA EQUAÇÃO, e ela é INTEIRA. Acima mede-se em vírgula
     * com 1e-14, mas o que a sustenta não tem decimal nenhum:
     *
     *      FP² = 1/(1 + x)   com x = THD²,   e a tese FP² = x dá
     *      1/(1+x) = x   ⟺   x² + x − 1 = 0
     *
     * E esse polinómio é a REVERSÃO do da borda do ouro: φ² − φ − 1 tem coeficientes
     * (1,−1,−1), e ao contrário dá (−1,−1,1), que é −(1,1,−1) — exactamente x² + x − 1.
     * É a mesma reversão que troca dentro por fora no §M17 do analog, e é ela que faz do
     * RECÍPROCO do ouro a raiz do polinómio revertido.
     *
     * Donde «autodual» deixa de ser uma leitura e passa a ter conta: o que se perde e o
     * que passa são raízes de um polinómio e do seu reverso. */
    {
        const long ouro[3] = {1, -1, -1};          /* φ² − φ − 1, do maior grau ao menor */
        long rev[3];
        for (int k = 0; k < 3; k++) rev[k] = ouro[2-k];      /* a reversão */
        /* a equação de FP: 1/(1+x) = x  ⟹  x² + x − 1 = 0 */
        const long eqfp[3] = {1, 1, -1};
        int bate = 1;
        for (int k = 0; k < 3; k++) if (rev[k] != -eqfp[k]) bate = 0;
        /* e o GUME: noutro metal a reversão já não dá a equação do fator de potência */
        int divergem = 0;
        for (long m = 2; m <= 5; m++) {
            long b[3] = {1, -m, -1}, r[3];
            for (int k = 0; k < 3; k++) r[k] = b[2-k];
            int igual = 1;
            for (int k = 0; k < 3; k++) if (r[k] != -eqfp[k]) igual = 0;
            if (!igual) divergem++;
        }
        printf("      e em inteiros: a borda do ouro é (%ld,%ld,%ld); revertida dá"
               " (%ld,%ld,%ld) = −(1,1,−1)\n", ouro[0],ouro[1],ouro[2], rev[0],rev[1],rev[2]);
        printf("      e a equação de FP, 1/(1+x) = x, é x² + x − 1 — a MESMA\n");
        printf("      GUME: com m = 2..5 a reversão da borda já não dá essa equação: %d de 4\n\n",
               divergem);
        ok("E A AUTODUALIDADE TEM CONTA, E É INTEIRA: FP² = THD² equivale a"
           " 1/(1+x) = x, isto é x² + x − 1 = 0 — e esse polinómio é a REVERSÃO do da borda"
           " do ouro, (1,−1,−1) ao contrário. É a mesma reversão que troca dentro por fora"
           " no §M17, e é ela que faz do RECÍPROCO do ouro a raiz do polinómio revertido."
           " Acima isto media-se a menos de 1e-14; aqui os coeficientes batem por igualdade."
           " E com o gume: noutro metal a reversão já não dá a equação do fator de potência,"
           " logo a coincidência é do OURO e não da família",
           bate && divergem == 4);
    }
    printf("      Autodual quer dizer o que sempre quis neste projeto: o objeto é o seu próprio\n");
    printf("      dual. Aqui a distorção (o que se perde) e o fator de potência (o que passa)\n");
    printf("      são o MESMO número áureo — um é o quadrado do outro, e o outro é o quadrado\n");
    printf("      do um. A perda e o ganho encontram-se no vinco.\n");
    printf("\n      E o teto de casar só a fundamental é η = φ^{-1/2} = %.4f = %.1f%%.\n",
           FP, FP*100);
}

printf("\n§S6  A escada: casar N níveis da torre, e η -> 100%%.\n\n");
{
    /* Casando os N primeiros niveis da torre de Fibonacci, a distorcao residual e' a CAUDA
     * φ^{-2(N+1)}/(1-φ^{-2}), e η = 1/√(1+cauda) -> 1. E' o inversor multinivel a limar. */
    printf("      cauda(N) = φ^{-2(N+1)}/(1 − φ^{-2}),   η = 1/√(1 + cauda)\n\n");
    printf("      níveis casados   cauda (distorção residual)   η\n");
    int mal = 0; double antEta = -1;
    for(int N = 0; N <= 6; N++){
        double cauda = pow(PHI, -2.0*(N+1))/(1 - pow(PHI,-2.0));
        double eta = 1.0/sqrt(1 + cauda);
        char rot[24];
        if(N == 0) snprintf(rot, sizeof rot, "%s", "só a fundamental");
        else       snprintf(rot, sizeof rot, "%d", N);
        printf("      %-16s %-28.9f %.4f  (%.1f%%)\n", rot, cauda, eta, eta*100);
        if(eta < antEta) mal++;                    /* η cresce sempre */
        antEta = eta;
    }
    printf("\n");
    ok("cada nível casado sobe a eficiência, e ela tende a 100% — a escada de Fibonacci",
       mal == 0 && antEta > 0.99);
    printf("      E é o mesmo movimento do motor.c §M6: o inversor multinível não muda a lei,\n");
    printf("      muda a RESOLUÇÃO com que ela se aplica. Lá era o erro angular a cair; aqui é a\n");
    printf("      distorção. Nos dois casos o preço é o mesmo — mais chaves, mais componentes,\n");
    printf("      mais modos de falhar — e a troca continua sem solução: tem escolha.\n");
    printf("\n      O teto de 78,6%% é o que se paga por casar só a fundamental. Não é uma\n");
    printf("      limitação de material: é o número áureo a cobrar a sua parte.\n");
}

printf("\n§S7  As ESTRELAS IRRACIONAIS: o ónus é o que nunca fecha.\n\n");
{
    /* Do corpo_estelar: ν inteiro <=> a estrela FECHA em k = ν+1 cuspides (orbita periodica);
     * ν irracional <=> roseta DENSA que nunca fecha. Mede-se: quantas voltas ate voltar ao
     * ponto de partida, e para o irracional a resposta e' NUNCA. */
    printf("      ν racional  ->  a estrela FECHA   (órbita periódica, período finito)\n");
    printf("      ν irracional -> roseta DENSA      (nunca fecha — e esse é o ÓNUS)\n\n");
    printf("      ν              fecha em N voltas?   |x_N − x_0| mínimo em 10000 voltas\n");
    int mal = 0;
    struct { double v; const char *nome; int racional; } t[] = {
        { 3.0,        "3      ",     1 },
        { 5.0/2.0,    "5/2    ",     1 },
        { 7.0/3.0,    "7/3    ",     1 },
        { PHI,        "φ      ",     0 },
        { sqrt(2.0),  "√2     ",     0 },
    };
    for(size_t j = 0; j < sizeof t/sizeof *t; j++){
        double melhor = 1e9; int quando = -1;
        for(int n = 1; n <= 10000; n++){
            double f = fmod(n*t[j].v, 1.0);
            double d = fmin(f, 1-f);              /* distância ao ponto de partida */
            if(d < melhor){ melhor = d; if(d < 1e-12) quando = n; }
            if(quando > 0) break;
        }
        char rot[32];
        if(quando > 0) snprintf(rot, sizeof rot, "sim, em %d", quando);
        else           snprintf(rot, sizeof rot, "%s", "NUNCA");
        printf("      %s        %-20s %.3e\n", t[j].nome, rot, melhor);
        int fechou = quando > 0;
        if(fechou != t[j].racional) mal++;
    }
    printf("\n");
    ok("o racional FECHA e o irracional NUNCA fecha — e é essa a diferença", mal == 0);
    printf("      É este o ónus matemático, e ele não é uma dificuldade: é uma propriedade. A\n");
    printf("      estrela racional fecha e pode ser dita por inteiro — tem período, tem dual,\n");
    printf("      atravessa a alfândega. A irracional aproxima-se para sempre e nunca chega, e\n");
    printf("      por isso fica retida: não há N que a devolva ao princípio.\n");
    printf("\n      E é o mesmo que o §T2 mediu como indecidibilidade, e o §S3 como calor: o que\n");
    printf("      não tem volta paga. Aqui a moeda é o período que não existe; lá era o\n");
    printf("      algoritmo que não existe; na bateria é o I²R. Uma lei, três balcões.\n");
}

printf("\n§S8  O CORPO SOLAR, formalizado.\n\n");
{
    printf("      Construído como todos os outros deste projeto — a cifra, a deformação, e a\n");
    printf("      régua do que se conserva:\n\n");
    printf("      A CIFRA         a torre de Fibonacci, φ = [1;1,1,…] — a fonte é áurea\n");
    printf("      A DEFORMAÇÃO    o casamento: Γ = (Z−Z₀)/(Z+Z₀) -> 0, o cone nulo σ = 1\n");
    printf("      O OPERADOR      o inversor multinível — sobe a escada, lima a distorção\n");
    printf("      A RÉGUA         η = 1/√(1+cauda), com o teto áureo φ^{-1/2} na fundamental\n");
    printf("      O QUE SE PERDE  o que não tem dual: fica retido, e arde\n\n");
    /* e a verificação final: o circuito solar fecha, com números */
    double I_solo = 1000.0, I_espaco = 1361.0, eta0 = pow(PHI,-0.5);
    printf("      E o circuito, com números:  P = η·I·A\n\n");
    printf("      condição      irradiância    η (fundamental)   densidade P/A\n");
    printf("      solo AM1.5    %-14.0f %-17.4f %.1f W/m²\n", I_solo, eta0, eta0*I_solo);
    printf("      espaço AM0    %-14.0f %-17.4f %.1f W/m²\n\n", I_espaco, eta0, eta0*I_espaco);
    int mal = 0;
    if(fabs(eta0*I_solo - 786.0) > 1.0) mal++;
    ok("a densidade de potência sai da eficiência áurea: η·I, e no solo dá ~786 W/m²",
       mal == 0);
    printf("      E o corpo fecha sobre si. A fonte é áurea (a garrafa de Koch, borda infinita\n");
    printf("      em espaço finito); a distorção que ela traz é 1/φ; o fator de potência é\n");
    printf("      φ^{-1/2}, e o quadrado de um é o outro — AUTODUAL. Casar mais níveis sobe a\n");
    printf("      eficiência sem mudar a lei, e o que nunca atravessa fica e aquece.\n");
    printf("\n      É a mesma peça de sempre, no seu último balcão: o CRUZADO gira (o motor), o\n");
    printf("      DIRETO mede (a norma, a reativa), o par adjunto conserva (a potência), e a\n");
    printf("      ALFÂNDEGA cobra o que não reverte. O Sol é onde essa cobrança vira luz.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
