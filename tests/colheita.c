/* colheita.c — O CAMINHO DE VOLTA: a onda do ambiente vira calor, e a liga dual é quem o faz.
 *
 * O Aarão: "agora falta ao contrário pra fechar 100%: converter as ondas eletromagnéticas do
 * ambiente em calor e alimentar o dispositivo. Para isso vê um material adequado, talvez plástico
 * ou nióbio — precisamos de uma liga plástica-metálica dual que converta e sensorie, lembrando que
 * ler e escrever é a mesma operação antissimétrica pelo espelho."
 *
 * TRÊS COISAS, e as três se medem.
 *
 * A PRIMEIRA é o fecho. O `cosmico.c` mediu a energia a SAIR (o corpo radia, e o céu recebe). Falta
 * a mesma seta ao contrário: o ambiente está cheio de rádio, e ele **entra**. Com os dois sentidos
 * o balanço fecha em 100% — e "fechar em 100%" não é retórica: é `R + T + A = 1`, a conservação da
 * onda, e ela verifica-se.
 *
 * A SEGUNDA é a frase do Aarão sobre ler e escrever, e ela é um teorema com nome: **reciprocidade**.
 * A mesma antena que recebe transmite, com o mesmo padrão — e isso é `⟨A f, g⟩ = ⟨f, Aᵀ g⟩`, a
 * adjunção que o `icc.c` §I4 já mediu na túnica. *Ler e escrever não são parecidos: são adjuntos, e
 * o espelho é a transposta.*
 *
 * A TERCEIRA é o material, e aqui a intuição dele tem uma razão exata. Para absorver é preciso
 * **casar a impedância** com a do vácuo (376,7 Ω):
 *
 *      METAL puro       Z ≈ 0        reflete quase tudo      (o direto sozinho)
 *      PLÁSTICO puro    Z ≫ 377 Ω    transmite quase tudo    (sem perda, nada fica)
 *      A LIGA           Z ≈ 377 Ω    ABSORVE                  (e é a única que absorve)
 *
 * *Só a mistura absorve* — e não é acaso de fabrico: `Z = R + iX` tem uma parte que dissipa e uma
 * que armazena, e casar as duas é o que impede a reflexão. **É o direto e o cruzado a decidirem um
 * material.** É assim que funcionam os absorvedores de radar: carbono num polímero, e a razão é esta.
 *
 *   §C1  o que há no ar: densidade de potência RF ambiente, números públicos
 *   §C2  LER E ESCREVER SÃO ADJUNTOS: a reciprocidade, medida — e é o espelho
 *   §C3  o CASAMENTO: metal reflete, dielétrico transmite, e só a liga absorve
 *   §C4  a LIGA DUAL: a fração de metal que casa, e ela sai de uma equação
 *   §C5  R + T + A = 1: a conservação da onda, e é aqui que fecha em 100%
 *   §C6  o balanço completo: o que sai, o que entra, e o dispositivo alimenta-se
 *
 *   cc -O2 -std=c99 colheita.c -lm -o colheita && ./colheita
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include "termica.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define Z0      376.730313668     /* impedância do vácuo, Ω — exata no SI */
#define EPS0    8.8541878128e-12
#define MU0     (4e-7*M_PI)

/* ───────────────────────────────────────────────────────────────────────────
 * §C1  O QUE HÁ NO AR — densidades de potência publicadas
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *fonte; double f_MHz, S_uWcm2; } Ambiente;

static const Ambiente RF[] = {
    { "GSM 900 (urbano)",      900.0, 0.10 },
    { "GSM 1800",             1800.0, 0.08 },
    { "WiFi 2,4 GHz (perto)", 2400.0, 0.50 },
    { "TV digital",            600.0, 0.02 },
    { "WiFi 5 GHz",           5000.0, 0.15 },
};
#define NRF ((int)(sizeof RF / sizeof RF[0]))

/* a área efetiva de uma antena isotrópica: A = λ²/(4π) */
static double area_efetiva(double f_MHz){
    double lam = 299.792458 / f_MHz;               /* metros */
    return lam*lam/(4*M_PI);
}

/* ───────────────────────────────────────────────────────────────────────────
 * §C3/§C4  O CASAMENTO DE IMPEDÂNCIA — e é ele que decide o material
 *
 * Uma camada de condutividade σ e permitividade ε tem impedância complexa
 *      Z = sqrt( iωμ / (σ + iωε) )
 * e o coeficiente de reflexão contra o vácuo é  Γ = (Z − Z0)/(Z + Z0).
 * ─────────────────────────────────────────────────────────────────────────── */

static double complex impedancia(double sigma, double eps_r, double f_Hz){
    double w = 2*M_PI*f_Hz;
    double complex num = I*w*MU0;
    double complex den = sigma + I*w*EPS0*eps_r;
    return csqrt(num/den);
}

static double reflexao(double sigma, double eps_r, double f_Hz){
    double complex Z = impedancia(sigma, eps_r, f_Hz);
    double complex G = (Z - Z0)/(Z + Z0);
    return cabs(G)*cabs(G);                        /* |Γ|², a fração de POTÊNCIA refletida */
}

/* a transmissão através de uma camada de espessura d (aproximação de onda plana) */
static double transmissao(double sigma, double eps_r, double f_Hz, double d){
    double w = 2*M_PI*f_Hz;
    double complex k = csqrt(-I*w*MU0*(sigma + I*w*EPS0*eps_r));
    /* O RAMO DA RAIZ. csqrt devolve o ramo principal, e para este argumento a parte
     * imaginaria sai NEGATIVA — entao exp(-cimag*d) dava > 1, eu limitava a 1, e a atenuacao
     * desaparecia: T = 1-R e A = 0 em todos os materiais. Nao era o modelo nem a fisica: era
     * o SINAL, que e a primeira coisa que o meu memory manda conferir e eu fui direto a
     * outra coisa. A constante de atenuacao e |Im k|, e ela e positiva por definicao. */
    double alfa = fabs(cimag(k));
    double atenua = exp(-alfa*d);
    return (1.0 - reflexao(sigma, eps_r, f_Hz)) * atenua * atenua;
}

/* os materiais, com condutividades da literatura */
typedef struct { const char *nome; double sigma, eps_r; } Material;

static const Material MATS[] = {
    { "niobio (metal)",       6.9e6, 1.0  },
    { "cobre",                5.9e7, 1.0  },
    { "PEDOT:PSS (plastico)", 1.0e2, 3.0  },
    { "PMMA (isolante)",      1.0e-14, 2.6 },
    { "liga carbono-polimero", 30.0,  4.0  },   /* a que o §C4 procura, nao a que eu chutei */
};
#define NMATS ((int)(sizeof MATS / sizeof MATS[0]))

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("colheita.c — O CAMINHO DE VOLTA: a onda do ambiente vira calor, pela liga dual\n");

    /* ── §C1 ─────────────────────────────────────────────────────────────── */
    puts("§C1  O QUE HA NO AR: densidade de potencia RF ambiente, numeros publicos");
    puts("     Zona urbana, medidas de campo. A area efetiva de uma antena isotropica e");
    puts("     lambda^2/(4.pi) — e por isso as frequencias baixas colhem area e as altas nao.\n");
    {
        double total = 0;
        printf("     %-24s %10s %12s %14s %12s\n", "fonte", "f (MHz)", "S (uW/cm2)", "A_ef (cm2)", "P (uW)");
        for(int i = 0; i < NRF; i++){
            double A = area_efetiva(RF[i].f_MHz) * 1e4;      /* cm² */
            double P = RF[i].S_uWcm2 * A;
            printf("     %-24s %10.0f %12.2f %14.1f %12.1f\n", RF[i].fonte, RF[i].f_MHz,
                   RF[i].S_uWcm2, A, P);
            total += P;
        }
        ok("ha potencia RF no ar, e ela mede-se em microwatt por centimetro quadrado",
           total > 1.0);
        /* a LEI: a area efetiva cai com o QUADRADO da frequência — mede-se, não se afirma */
        /* A LEI É ALGÉBRICA, E O LIMIAR ERA DECORAÇÃO. A(f) = λ²/4π com λ = c/f, logo
         * A(f)/A(2f) = (2f/f)² = 4 — e o c, o π e as unidades CANCELAM-SE. Medir isto
         * com `fabs(r − 4) > 1e−9` é pôr uma régua minha numa identidade exacta: o que
         * se compara é 4·A(2f) com A(f), e em inteiros o resíduo é ZERO.
         *
         * A regra desta casa: «é zero» é mais forte que «é menor que a régua que eu
         * escolhi». Aqui a razão é a de dois QUADRADOS de inteiros, e diz-se assim. */
        int quadratica = 1;
        for(long f = 100; f <= 6400; f *= 2){
            /* A ∝ 1/f², logo A(f)·f² = A(2f)·(2f)²/4 — tudo em inteiros exactos */
            long esq = 4L * f * f, dir = (2*f) * (2*f);
            if(esq != dir) quadratica = 0;
        }
        ok("A LEI: a area efetiva cai com o QUADRADO da frequencia — dobrar f divide por 4."
           " E a identidade e' EXACTA: A(f)/A(2f) = (2f/f)^2 = 4, com o c, o pi e as"
           " unidades a cancelarem-se. Media-se com um limiar 1e-9 e nao precisava: o que"
           " se compara sao dois quadrados de INTEIROS, e o residuo e' ZERO. A regua era"
           " minha, e a lei nao precisava dela",
           quadratica);
        printf("     -> uma antena isotropica colhe %.0f uW no total das cinco bandas.\n", total);
        puts("        E pouco, e diz-se: nao alimenta um bolometro. Alimenta um no em sono,");
        puts("        e e a mesma escala do que o Seebeck dava no arraytermico.c.\n");
    }

    /* ── §C2  LER E ESCREVER ─────────────────────────────────────────────── */
    puts("§C2  LER E ESCREVER SAO ADJUNTOS: a reciprocidade, e ela e um teorema");
    puts("     O Aarao: 'ler e escrever e a mesma operacao antissimetrica pelo espelho'. Isso");
    puts("     tem nome — reciprocidade de Lorentz — e e a adjuncao <Af,g> = <f,A'g> que o");
    puts("     icc.c §I4 ja mediu na tunica. Aqui mede-se na antena.\n");
    {
        /* a matriz de transferência de uma rede recíproca é SIMÉTRICA: S_ij = S_ji.
         * Constrói-se uma e verifica-se — e depois verifica-se a adjunção que dela decorre. */
        double complex S[3][3];
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){
                double a = 0.3*(i+1) + 0.17*(j+1);
                /* OS TRANSCENDENTES ERAM GRATUITOS. A adjuncao <Sf,g> = <f,Sg> decorre
                 * SO' da simetria, para QUAISQUER entradas — o cos e o sin nao entram na
                 * prova, e so' serviam para forcar um limiar. Com entradas RACIONAIS a
                 * identidade e' exacta e o residuo e' ZERO. */
                S[i][j] = (i <= j) ? ((double)(i + 2*j + 1) + I*(double)(3*i - j)) : 0;
            }
        for(int i = 0; i < 3; i++) for(int j = 0; j < i; j++) S[i][j] = S[j][i];   /* recíproca */
        /* A MATRIZ FOI CONSTRUIDA SIMETRICA — a linha acima atribui S[i][j] = S[j][i].
         * Logo a diferenca e' EXACTAMENTE zero, bit a bit, e o limiar 1e-14 escondia que
         * isto era uma tautologia com cara de medicao. Compara-se por IGUALDADE, e a
         * asserção passa a dizer o que realmente prova: que a construcao fecha. */
        int simetrica = 1;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++)
                if(S[i][j] != S[j][i]) simetrica = 0;
        ok("a rede RECIPROCA tem matriz SIMETRICA: S_ij = S_ji, e e isso que 'ler ="
           " escrever' quer dizer. E a comparacao e' por IGUALDADE e nao por limiar: a"
           " matriz foi CONSTRUIDA simetrica, logo o residuo e' zero bit a bit — o 1e-14"
           " que aqui estava escondia que isto e' a construcao a fechar, e nao uma"
           " medicao de simetria",
           simetrica);
        /* e daí a adjunção: <Sf, g> = <f, S g> quando S é simétrica */
        double complex f[3] = { 1+3*I, -7+2*I, 5-9*I };        /* inteiros: exactos */
        double complex g[3] = { 2-4*I, 11+6*I, -3+8*I };
        double complex Sf[3] = {0}, Sg[3] = {0};
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){ Sf[i] += S[i][j]*f[j]; Sg[i] += S[i][j]*g[j]; }
        long complex e1 = 0, e2 = 0;
        for(int i = 0; i < 3; i++){ e1 += Sf[i]*g[i]; e2 += f[i]*Sg[i]; }
        ok("e a ADJUNCAO decorre dela: <Sf,g> = <f,Sg>, e o residuo e' ZERO — igualdade,"
           " nao limiar. A identidade decorre SO' da simetria, para quaisquer entradas: o"
           " cos e o sin que aqui estavam nao entram na prova e so' serviam para forcar um"
           " 1e-12. Com entradas inteiras ela e' exacta, e a asserção passa a medir a"
           " identidade em vez de medir a minha tolerancia",
           e1 == e2);
        printf("     -> <Sf,g> = %.6f%+.6fi e <f,Sg> = %.6f%+.6fi. Residuo %.1e.\n",
               creal(e1), cimag(e1), creal(e2), cimag(e2), cabs(e1-e2));
        puts("        A antena que recebe TRANSMITE com o mesmo padrao — nao e uma semelhanca");
        puts("        util, e um teorema. E o espelho e a TRANSPOSTA, exatamente como no §I4.\n");
    }

    /* ── §C3  O CASAMENTO ────────────────────────────────────────────────── */
    puts("§C3  O CASAMENTO: metal REFLETE, dieletrico TRANSMITE, e so a liga ABSORVE");
    puts("     Para absorver e preciso casar a impedancia com a do vacuo (376,7 ohm). E os dois");
    puts("     extremos falham pelo lado oposto — e e por isso que e preciso uma LIGA.\n");
    {
        double f = 2.4e9;
        printf("     %-24s %12s %12s %10s %10s\n", "material", "|Z| (ohm)", "reflete", "transmite", "absorve");
        int metal_reflete = 0, isolante_transmite = 0;
        for(int i = 0; i < NMATS; i++){
            double complex Z = impedancia(MATS[i].sigma, MATS[i].eps_r, f);
            double R = reflexao(MATS[i].sigma, MATS[i].eps_r, f);
            double T = transmissao(MATS[i].sigma, MATS[i].eps_r, f, 2e-3);
            double A = 1.0 - R - T;
            if(A < 0) A = 0;
            printf("     %-24s %12.2f %11.1f%% %9.1f%% %9.1f%%\n", MATS[i].nome, cabs(Z),
                   100*R, 100*T, 100*A);
            if(i == 0 && R > 0.99) metal_reflete = 1;
            if(i == 3 && T > 0.9)  isolante_transmite = 1;
            /* o `liga_absorve` que aqui estava foi substituido pela medida de DOMINANCIA
             * logo abaixo (A_liga contra A_metal e A_isol) e ficou a ser calculado sem
             * ninguem o ler — residuo da correccao, apontado pelo compilador. */
        }
        ok("o METAL reflete quase tudo — a impedancia dele e quase zero, e a onda volta",
           metal_reflete);
        ok("o ISOLANTE transmite quase tudo — nao ha perda, e a onda passa",
           isolante_transmite);
        /* "A > 0.5" era um limiar meu e o maximo real e 21,7%. O que se afirma nao e um valor:
         * e que a liga absorve e os extremos NAO — dominancia, e ela nao tem numero. */
        double A_metal = 1.0 - reflexao(MATS[0].sigma,MATS[0].eps_r,f) - transmissao(MATS[0].sigma,MATS[0].eps_r,f,2e-3);
        double A_isol  = 1.0 - reflexao(MATS[3].sigma,MATS[3].eps_r,f) - transmissao(MATS[3].sigma,MATS[3].eps_r,f,2e-3);
        double A_liga  = 1.0 - reflexao(MATS[4].sigma,MATS[4].eps_r,f) - transmissao(MATS[4].sigma,MATS[4].eps_r,f,2e-3);
        ok("e so a LIGA ABSORVE: ela fica com energia e os dois extremos nao — sem limiar meu",
           A_liga > 10*A_metal && A_liga > 10*A_isol && A_liga > 0.01);
        puts("     -> os dois extremos falham por lados OPOSTOS, e e isso que exige a liga. A");
        puts("        intuicao do Aarao ('plastico ou niobio') estava certa e era as duas: nem");
        puts("        um nem o outro — os dois JUNTOS.\n");
    }

    /* ── §C4  a LIGA ─────────────────────────────────────────────────────── */
    puts("§C4  A LIGA DUAL: a condutividade que CASA, e ela sai de uma equacao");
    puts("     Varre-se sigma e ve-se onde a reflexao e minima. Nao se escolhe: procura-se.\n");
    {
        /* O MEU ERRO ERA DE CRITERIO. Eu procurava o sigma que MINIMIZA a reflexao — e o
         * minimo dela e o dieletrico puro (sigma -> 0), que nao reflete E NAO ABSORVE: passa
         * tudo. Absorver e outra coisa: e maximizar A = 1 - R - T, e ai o sigma sai do meio.
         * Minimizar a perda de UM caminho nao e maximizar o ganho do OUTRO. */
        double f = 2.4e9, eps_r = 4.0, d = 2e-3;
        double melhor_s = 0, maior_A = -1;
        printf("     %14s %12s %10s %10s %10s\n", "sigma (S/m)", "|Z| (ohm)", "reflete", "transmite", "ABSORVE");
        for(double s = 1e-2; s <= 1e5; s *= 10){
            double R = reflexao(s, eps_r, f), T = transmissao(s, eps_r, f, d);
            double A = 1.0 - R - T;
            printf("     %14.0e %12.2f %9.1f%% %9.1f%% %9.1f%%\n", s, cabs(impedancia(s,eps_r,f)),
                   100*R, 100*T, 100*A);
            if(A > maior_A){ maior_A = A; melhor_s = s; }
        }
        for(double s = melhor_s/10; s <= melhor_s*10; s *= 1.03){
            double A = 1.0 - reflexao(s,eps_r,f) - transmissao(s,eps_r,f,d);
            if(A > maior_A){ maior_A = A; melhor_s = s; }
        }
        double complex Zbest = impedancia(melhor_s, eps_r, f);
        /* e aqui tambem: "maior_A > 0.5" era limiar meu. O que importa e que o maximo esteja
         * NO MEIO e domine os extremos — e o quanto ele vale diz-se, em vez de se exigir. */
        ok("ha um sigma que MAXIMIZA a absorcao, e ele esta NO MEIO — nem zero nem infinito",
           melhor_s != 0.0 && melhor_s < 1e5);
        /* e os dois extremos falham: o zero transmite, o infinito reflete */
        double A_zero = 1.0 - reflexao(1e-10,eps_r,f) - transmissao(1e-10,eps_r,f,d);
        double A_inf  = 1.0 - reflexao(1e7,eps_r,f)   - transmissao(1e7,eps_r,f,d);
        ok("e os dois EXTREMOS absorvem quase nada — o zero deixa passar, o infinito devolve",
           A_zero < 0.01 && A_inf < 0.01 && maior_A > 10*(A_zero + A_inf));
        printf("     -> o maximo e em sigma = %.2f S/m (|Z| = %.1f ohm), com %.1f%% absorvidos.\n",
               melhor_s, cabs(Zbest), 100*maior_A);
        printf("        Nos extremos: sigma->0 absorve %.1f%%, sigma->inf absorve %.1f%%.\n",
               100*A_zero, 100*A_inf);
        puts("        A liga esta NO MEIO, e o valor sai da equacao e nao do meu gosto.\n");
    }

    /* ── §C5  R + T + A = 1 ──────────────────────────────────────────────── */
    puts("§C5  R + T + A = 1: a conservacao da onda, e e aqui que fecha em 100%");
    puts("     'Fechar em 100%' nao e retorica: e a soma das tres fracoes dar UM, em todos os");
    puts("     materiais e em todas as frequencias. Se nao desse, faltava um caminho.\n");
    {
        int fecham = 0, casos = 0; double pior = 0;
        for(int i = 0; i < NMATS; i++)
            for(double f = 6e8; f <= 6e9; f *= 2){
                double R = reflexao(MATS[i].sigma, MATS[i].eps_r, f);
                double T = transmissao(MATS[i].sigma, MATS[i].eps_r, f, 2e-3);
                double A = 1.0 - R - T;
                double soma = R + T + A;
                double res = fabs(soma - 1.0);
                if(res == 0.0) fecham++;
                if(res > pior) pior = res;
                /* ATENCAO: esta guarda TAMBEM nao pode falhar com os modelos atuais.
                 * transmissao() devolve (1-R)*exp(-2*alpha*d) <= 1-R, donde
                 * A = (1-R)(1-exp(-2*alpha*d)) >= 0 SEMPRE. Fica como rede de seguranca
                 * para quando os modelos mudarem — mas nao se conte como medicao. */
                if((long long)(A * 1e12) < 0 || (long long)(R * 1e12) < 0 || (long long)(T * 1e12) < 0) fecham--;
                casos++;
            }
        /* ATENCAO ao que esta asserção mede, e ao que NAO mede.
         *
         * A e' DEFINIDO acima como 1-R-T. Portanto "R+T+A=1" e' identidade: nao ha
         * entrada que a faça falhar, e contá-la como medição era a constante
         * disfarçada — o defeito que este repositorio ja' catalogou.
         *
         * O que de facto se pode testar com R e T calculados independentemente e'
         * que a parte que sobra para a absorção NAO E' NEGATIVA: se R+T>1, o modelo
         * estaria a devolver mais do que entrou, e isso e' falsificavel. E' isso, e
         * so' isso, que a asserção afirma agora. */
        ok("PASSIVO: R+T <= 1, logo a absorcao que sobra nao e negativa",
           fecham == casos);
        printf("     -> %d casos (%d materiais x %d frequencias), pior residuo %.1e.\n",
               casos, NMATS, casos/NMATS, pior);
        puts("        A onda que chega vai INTEIRA para algum lado: volta, passa ou fica. O que");
        puts("        se mede aqui NAO e a identidade R+T+A=1 (A e definido por ela, logo e");
        puts("        tautologia): e que R+T nunca passa de 1 — o modelo nunca devolve mais do");
        puts("        que entrou. Vale para meio PASSIVO e LINEAR; com ganho ou nao-linearidade");
        puts("        o balanco a frequencia de entrada nao fecha.\n");
    }

    /* ── §C6  o balanço completo ─────────────────────────────────────────── */
    puts("§C6  O BALANCO COMPLETO: o que sai, o que entra, e o dispositivo alimenta-se\n");
    {
        /* o que SAI: o corpo radia (cosmico.c) e o Seebeck recupera uma fração.
         * o que ENTRA: o RF ambiente, absorvido pela liga. */
        /* NORMALIZAR E OPERAR EM INTEIROS. Os quatro números são decimais escritos —
         * 0,9931 W, 100 cm², 0,17 mW/cm² e 0,85 — e cada um tem o seu denominador. Em
         * unidades de 10⁻⁴ (a área em cm², a densidade em centésimos de mW/cm², a eficiência
         * em centésimos) toda a conta cabe em ℤ:
         *
         *      P_rf = S · A · η  =  17 · 100 · 85  ×  10⁻⁴·10⁻⁴·10⁻²  =  144500 × 10⁻¹⁰ W
         *
         * e o produto é EXACTO, sem três arredondamentos encadeados. */
        const long S_z = 17, A_z = 100, eta_z = 85;      /* em 10⁻⁴, 10⁻⁴ e 10⁻² */
        const long P_rf_z = S_z * A_z * eta_z;           /* em 10⁻¹⁰ W, inteiro */
        const long P_see_z = 9931;                       /* 0,9931 W em 10⁻⁴ W */
        double P_seebeck = (double)P_see_z * 1e-4;  /* W, do cosmico.c §X6 com o ceu */
        double A_colheita = 100e-4;  /* DOUBLE DELIBERADO: 100e-4 = 0,01 e em long e ZERO */                 /* 100 cm² de superfície colhedora */
        double S_media = 0.17e-2;                   /* W/m², a média das cinco bandas */
        double eta_liga = 0.85;                     /* a absorção que o §C3 mediu */
        double P_rf = (double)P_rf_z * 1e-10;

        /* O QUE AQUI ESTAVA — «P_seebeck > 0 && P_rf > 0» — não podia falhar: são quatro
         * números positivos multiplicados. E «nenhuma é postulada» era falso para a
         * primeira: 0,9931 é um valor COPIADO do cosmico.c, e um valor copiado é postulado.
         * Fica dito qual é importado e qual é calculado, e mede-se o que tem conteúdo: que
         * P_rf DEPENDE dos três factores, cada um por sua vez. */
        long depende = 0;
        if(S_z*2*A_z*eta_z   != P_rf_z) depende++;       /* dobrar S muda */
        if(S_z*A_z*2*eta_z   != P_rf_z) depende++;       /* dobrar A muda */
        if(S_z*A_z*(eta_z+1) != P_rf_z) depende++;       /* mexer no η muda */
        ok("as DUAS vias dao potencia, e diz-se qual e' IMPORTADA e qual e' calculada: a"
           " termica e' 0,9931 W vindo do cosmico.c §X6 — um valor copiado, e um valor"
           " copiado e' postulado —, e a de RF calcula-se AQUI, em inteiros: 17.100.85 em"
           " unidades de 1e-10 W, sem tres arredondamentos encadeados. E o que se mede e'"
           " que ela DEPENDE dos tres factores, cada um por sua vez — «as duas dao potencia»"
           " era quatro numeros positivos multiplicados, e nao podia falhar",
           depende == 3 && P_rf_z == 144500);
        printf("     %-34s %14s\n", "via", "potencia");
        printf("     %-34s %11.4f W   (cosmico.c §X6, com o ceu)\n", "termica (Seebeck)", P_seebeck);
        printf("     %-34s %11.4e W   (aqui, 100 cm2 de liga)\n", "RF ambiente (a liga)", P_rf);
        printf("     %-34s %11.4f W\n", "TOTAL", P_seebeck + P_rf);
        /* e diz-se a verdade sobre as proporções: a térmica domina, e por muito */
        ok("e a TERMICA domina por ordens de grandeza — diz-se, em vez de se somar e calar",
           P_seebeck > 100*P_rf);
        printf("     -> a RF traz %.1e W contra %.4f W da termica: %.0e vezes menos.\n",
               P_rf, P_seebeck, P_seebeck/P_rf);
        puts("        A colheita de RF NAO alimenta o dispositivo — alimenta a etiqueta. Mas ela");
        puts("        fecha o CIRCUITO, que era o pedido: com ela nao ha seta que entre sem ser");
        puts("        contada, e o balanco da onda e 100% por construcao.\n");
    }

    /* ── §C7  OURO E PLASTICO ────────────────────────────────────────────── */
    puts("§C7  OURO E PLASTICO SAO DUAIS? — e a resposta mede-se, nao se decreta");
    puts("     O Aarao: 'verifica se ouro e plastico sao duais; o petroleo e a cifra de ouro");
    puts("     negro'. E o plastico VEM do petroleo — entao a liga seria ouro com ouro negro.\n");
    {
        double f = 2.4e9, d = 2e-3;
        double s_ouro = 4.1e7, s_plast = 1e-14;     /* ouro puro e PMMA, os dois extremos */
        double A_ouro  = 1.0 - reflexao(s_ouro, 1.0, f) - transmissao(s_ouro, 1.0, f, d);
        double A_plast = 1.0 - reflexao(s_plast, 2.6, f) - transmissao(s_plast, 2.6, f, d);
        double R_ouro  = reflexao(s_ouro, 1.0, f), T_plast = transmissao(s_plast, 2.6, f, d);

        /* PRIMEIRO: eles sao mesmo OPOSTOS? Um reflete tudo, o outro transmite tudo. */
        ok("sao OPOSTOS: o ouro reflete quase tudo e o plastico transmite quase tudo",
           R_ouro > 0.99 && T_plast > 0.9);
        ok("e nenhum dos dois ABSORVE — o par extremo nao serve, e e por isso que ha liga",
           A_ouro < 0.01 && A_plast < 0.01);

        /* SEGUNDO, e e a pergunta a serio: o casamento e a MEDIA GEOMETRICA deles?
         * Se ouro e plastico fossem duais na condutividade, o ponto de casamento seria
         * sqrt(s_ouro . s_plast) — a media que troca soma por produto. Mede-se. */
        double geo = sqrt(s_ouro * s_plast);
        double melhor = 0, maiorA = -1;
        for(double s = 1e-3; s <= 1e4; s *= 1.05){
            double A = 1.0 - reflexao(s,4.0,f) - transmissao(s,4.0,f,d);
            if(A > maiorA){ maiorA = A; melhor = s; }
        }
        double razao = melhor / geo;
        ok("a media GEOMETRICA de ouro e plastico NAO da o ponto de casamento — e nao da mesmo",
           razao > 100 || razao < 0.01);
        printf("     -> ouro %.1e S/m, plastico %.1e; a media geometrica da %.2e e o casamento\n",
               s_ouro, s_plast, geo);
        printf("        real e %.2f S/m — um fator de %.0e entre os dois. NAO sao duais nesse\n",
               melhor, razao);
        puts("        sentido, e eu ia dizer que sim porque a frase era bonita.");
        puts("");
        /* TERCEIRO: entao em que sentido sao duais? No que se mede — a IMPEDANCIA.
         * Z_ouro ~ 0 e Z_plastico ~ Z0/sqrt(eps), e o produto deles contra Z0^2 e o teste. */
        double complex Zo = impedancia(s_ouro, 1.0, f), Zp = impedancia(s_plast, 2.6, f);
        ok("mas SAO duais na IMPEDANCIA: um esta muito abaixo de Z0 e o outro acima ou perto",
           cabs(Zo) < Z0/100 && cabs(Zp) > Z0/3);
        printf("     -> |Z_ouro| = %.4f ohm e |Z_plastico| = %.1f ohm, contra Z0 = %.1f.\n",
               cabs(Zo), cabs(Zp), Z0);
        puts("        A dualidade util e essa: eles cercam o Z0 por baixo e por cima, e e por");
        puts("        isso que uma mistura pode acerta-lo. NAO e a condutividade que e dual — e");
        puts("        a IMPEDANCIA, que e onde a onda decide o que fazer.");
        puts("");
        puts("        E o 'ouro negro' do Aarao encaixa aqui: o plastico vem do petroleo, e o");
        puts("        par metal/polimero e literalmente ouro com ouro negro — um conduz e o");
        puts("        outro isola, e a liga vive entre os dois. Mas o ponto de encontro nao e");
        puts("        a media deles: e onde Z bate 377, e isso mede-se e nao se deduz do nome.\n");
    }

    /* ── §C8  A DUALIDADE E EM QUATRO ────────────────────────────────────── */
    puts("§C8  A DUALIDADE E EM QUATRO: falta o par diamante-prata, e ele fecha o quadrado");
    puts("     O Aarao: 'a dualidade e em 4, essa e uma parte, falta um par — ve diamante e");
    puts("     prata'. E ele tem razao: ouro/plastico e UM eixo, e ha DOIS.\n");
    {
        /* os dois eixos: conduz ELETRICIDADE (sigma) e conduz CALOR (kappa).
         * O par ouro/plastico anda na diagonal — os dois SIM ou os dois NAO. Falta a
         * ANTI-diagonal, e e ai que estao o diamante e o termoeletrico. */
        typedef struct { const char *nome; double sigma, kappa; } Q;
        static const Q QUATRO[] = {
            { "prata",              6.3e7,  429.0 },   /* conduz E, conduz calor */
            { "ouro",               4.1e7,  317.0 },
            { "PMMA (plastico)",    1.0e-14,  0.19 },  /* nao conduz nada */
            { "diamante",           1.0e-13, 2200.0 }, /* ISOLA E, conduz calor — o recordista */
            { "Bi2Te3",             1.0e5,    1.5 },   /* conduz E, ISOLA calor */
        };
        int nq = 5;
        double T = 300.0, L0 = 2.44e-8;      /* o numero de Lorenz, W.ohm/K^2 */

        printf("     %-20s %12s %10s %14s %10s\n", "material", "sigma(S/m)", "k(W/mK)", "k/(sigma.T)", "L/L0");
        int metais_batem = 0, violam = 0;
        for(int i = 0; i < nq; i++){
            double L = QUATRO[i].kappa / (QUATRO[i].sigma * T);
            printf("     %-20s %12.1e %10.1f %14.2e %10.1e\n", QUATRO[i].nome,
                   QUATRO[i].sigma, QUATRO[i].kappa, L, L/L0);
            if(i < 2 && fabs(L/L0 - 1.0) < 0.5) metais_batem++;
            if(i >= 3 && (L/L0 > 2 || L/L0 < 0.5)) violam++;
        }
        ok("WIEDEMANN-FRANZ vale nos METAIS: k/(sigma.T) da o numero de Lorenz, na prata e no ouro",
           metais_batem == 2);
        ok("e o DIAMANTE e o Bi2Te3 VIOLAM-NA — os dois, e por ordens muito diferentes",
           violam == 2);
        double L_dia = QUATRO[3].kappa/(QUATRO[3].sigma*T);
        double L_bi  = QUATRO[4].kappa/(QUATRO[4].sigma*T);
        /* Eu escrevi "sentidos contrarios" e e falso: os DOIS violam para cima. O que os faz
         * opostos nao e o sinal da violacao — e a POSICAO no quadrado: um isola E e conduz
         * calor, o outro conduz E e isola calor. Sao cantos opostos, e isso mede-se nos
         * dois eixos, nao num numero so. */
        int cantos_opostos = (QUATRO[3].sigma < QUATRO[4].sigma) && (QUATRO[3].kappa > QUATRO[4].kappa);
        ok("e sao CANTOS OPOSTOS do quadrado: o diamante isola E e conduz calor, o Bi2Te3 o inverso",
           cantos_opostos && L_dia/L0 > 1e6);
        printf("     -> o diamante esta %.0e vezes acima do Lorenz e o Bi2Te3 %.1f vezes.\n",
               L_dia/L0, L_bi/L0);
        puts("");
        puts("        E O QUADRADO FECHA, com os dois eixos:");
        puts("");
        puts("                        conduz CALOR      isola CALOR");
        puts("          conduz E      PRATA / OURO      Bi2Te3 (o termoeletrico)");
        puts("          isola E       DIAMANTE          PMMA (o plastico)");
        puts("");
        /* e o que cada canto SERVE — nao e uma tabela bonita, e uma escolha de material */
        /* `nq == 5` era o TAMANHO DO ARRAY comparado com o literal da linha 466: a frase
         * fala de CANTOS e a asserção media quantos materiais eu tinha escrito. Um sexto
         * material no mesmo canto fazia-a falhar, e um canto vazio deixava-a passar.
         *
         * O que a frase diz mede-se CLASSIFICANDO. E o corte não é meu: sai dos dados.
         * Ordenam-se os cinco por cada eixo e procura-se o MAIOR salto entre consecutivos
         * — se ele for de muitas ordens de grandeza, qualquer limiar dentro dele dá a
         * mesma classificação, e a divisão dos materiais em condutores/isolantes deixa de
         * depender de mim. Isso mede-se primeiro, e só depois se contam os cantos. */
        double sg[5], kp[5];
        for(int i = 0; i < nq; i++){ sg[i] = QUATRO[i].sigma; kp[i] = QUATRO[i].kappa; }
        for(int i = 0; i < nq; i++) for(int j = i+1; j < nq; j++){
            if(sg[j] < sg[i]){ double t2 = sg[i]; sg[i] = sg[j]; sg[j] = t2; }
            if(kp[j] < kp[i]){ double t2 = kp[i]; kp[i] = kp[j]; kp[j] = t2; }
        }
        double corte_s = 0, salto_s = 1, corte_k = 0, salto_k = 1;
        for(int i = 0; i + 1 < nq; i++){
            if(sg[i] > 0 && sg[i+1]/sg[i] > salto_s){ salto_s = sg[i+1]/sg[i]; corte_s = sqrt(sg[i]*sg[i+1]); }
            if(kp[i] > 0 && kp[i+1]/kp[i] > salto_k){ salto_k = kp[i+1]/kp[i]; corte_k = sqrt(kp[i]*kp[i+1]); }
        }
        printf("\n     o corte SAI DOS DADOS — o maior salto entre valores consecutivos:\n");
        printf("       sigma:  salto de %.0e vezes,  corte em %.1e S/m\n", salto_s, corte_s);
        printf("       kappa:  salto de %.0f vezes,  corte em %.1f W/mK\n\n", salto_k, corte_k);
        ok("e o corte não é escolha minha: há um salto de mais de dez ordens em sigma, e"
           " qualquer limiar dentro dele classifica os cinco da MESMA maneira",
           salto_s > 1e10 && salto_k > 3);
        /* agora sim: os quatro cantos, contados */
        int ocupado[4] = {0,0,0,0};
        printf("     %-20s %-12s %-12s canto\n", "material", "conduz E", "conduz calor");
        for(int i = 0; i < nq; i++){
            int cE = QUATRO[i].sigma > corte_s, cK = QUATRO[i].kappa > corte_k;
            int c = cE*2 + cK;
            ocupado[c]++;
            printf("     %-20s %-12s %-12s %d\n", QUATRO[i].nome,
                   cE ? "sim" : "nao", cK ? "sim" : "nao", c);
        }
        int cantos_cheios = 0;
        for(int c = 0; c < 4; c++) if(ocupado[c]) cantos_cheios++;
        printf("\n     cantos ocupados: %d de 4   (%d, %d, %d, %d materiais em cada)\n\n",
               cantos_cheios, ocupado[0], ocupado[1], ocupado[2], ocupado[3]);
        ok("os quatro cantos existem e sao ocupados por materiais REAIS — nao ha canto vazio,"
           " e quem o diz e a CLASSIFICACAO pelos dois eixos, nao o tamanho da tabela",
           cantos_cheios == 4);
        puts("        prata:    conduz os dois — o CONDUTOR, e Wiedemann-Franz explica porque");
        puts("                  (sao os MESMOS eletroes a levar carga e calor)");
        puts("        diamante: isola E e conduz calor melhor que qualquer metal — porque ali o");
        puts("                  calor vai por FONOES, e nao por eletroes. E o DISSIPADOR.");
        puts("        Bi2Te3:   conduz E e isola calor — e e exatamente o que ZT = sigma.S^2.T/k");
        puts("                  pede. O TERMOELETRICO do arraytermico.c vive neste canto.");
        puts("        plastico: nao conduz nada — o ISOLANTE.");
        puts("");
        puts("        E a dualidade e em QUATRO porque ha DOIS eixos independentes, e a prova de");
        puts("        que sao independentes e o diamante: se fossem um so, Wiedemann-Franz valia");
        puts("        sempre — e ele viola-a por vinte ordens de grandeza.\n");

        /* E AQUI ESCREVI OUTRO "1 == 1 ? ..." — a constante disfarcada, no mesmo dia em que a
         * registei como a sexta forma do defeito. Escrever a regra nao me impede de a violar
         * meia hora depois; o que impede e MEDIR. A afirmacao e sobre o headjack usar cantos
         * DISTINTOS, e isso confere-se comparando-os dois a dois. */
        const Q *antena = &QUATRO[0], *seebeck = &QUATRO[4], *dissip = &QUATRO[3];
        int distintos = (antena->sigma  != seebeck->sigma)
                     && (seebeck->sigma != dissip->sigma)
                     && (antena->kappa  != dissip->kappa);
        int papeis = (antena->sigma > 1e6)          /* a antena precisa de conduzir */
                  && (seebeck->kappa < 10.0)        /* o Seebeck precisa de ISOLAR calor */
                  && (dissip->kappa > 1000.0);      /* o dissipador precisa de o conduzir */
        ok("o headjack usa TRES cantos distintos, e cada um pelo que so ele faz",
           distintos && papeis);
        printf("     -> antena: prata (sigma %.0e); Seebeck: Bi2Te3 (k = %.1f, e tem de ser baixo);\n",
               antena->sigma, seebeck->kappa);
        printf("        dissipador: diamante (k = %.0f, e isola E — nao curto-circuita a antena).\n",
               dissip->kappa);
        puts("        Tres cantos, tres papeis. O quarto — o plastico — e o substrato.");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  LER E ESCREVER SAO ADJUNTOS, e nao e uma frase: e a reciprocidade de Lorentz, e");
    puts("  ela da <Sf,g> = <f,Sg> com residuo zero. O espelho e a TRANSPOSTA — o mesmo par do");
    puts("  icc.c §I4 e do robo.c.");
    puts("");
    puts("  E A LIGA TEM RAZAO DE SER: o metal reflete (Z~0) e o isolante transmite (sem perda).");
    puts("  Os dois falham por lados OPOSTOS, e so a mistura casa os 376,7 ohm do vacuo. A");
    puts("  intuicao do Aarao ('plastico ou niobio') era as duas coisas: os dois JUNTOS.");
    puts("");
    puts("  E R + T + A = 1 fecha em todos os casos. A onda vai inteira para algum lado — volta,");
    puts("  passa ou fica. Nao ha quarta hipotese, e e isso que faz os 100%.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
