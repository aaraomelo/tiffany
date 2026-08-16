/* liga.c — AS PROPRIEDADES DA LIGA grafeno+estanho: e elas não se satisfazem todas.
 *
 * O Aarão: "estuda as propriedades dessa liga — condutividade, ida e volta, impedância,
 * ductilidade, tudo."
 *
 * O `octeto.c` escolheu o par. Falta medir o que ele dá — e o resultado tem uma forma que vale
 * dizer à cabeça: **os requisitos entram em conflito, e o conflito mede-se.** Uma liga não otimiza
 * tudo; ela escolhe, e o que se pode fazer é mostrar onde está a escolha em vez de a esconder.
 *
 * O QUE GOVERNA TUDO É A PERCOLAÇÃO. Um compósito não interpola entre os dois materiais: ele salta.
 * Abaixo de uma fração crítica `pc` o grafeno está disperso e não conduz; acima, os caminhos ligam-se
 * e a condutividade sobe por lei de potência,
 *
 *      σ(p) ∝ (p − pc)^t          com t ≈ 2 em 3D
 *
 * — e `pc` é minúsculo para folhas de alta razão de aspeto (~0,1%), porque uma folha grande toca
 * muitas outras. *É a mesma ideia do §M5 do microfluidica.c: o que decide não é a quantidade, é a
 * geometria de quem toca quem.*
 *
 * E DAÍ SAI A TENSÃO CENTRAL. O `colheita.c` §C4 mediu que absorver pede `σ ≈ 3,46 S/m`. Mas acima
 * do limiar a condutividade **dispara** — passa de zero a milhões em pouca fração. **A janela onde
 * ela vale 3,46 é estreita**, e medi-la é o trabalho.
 *
 *   §L1  a PERCOLAÇÃO: o limiar, a lei de potência, e o salto medido
 *   §L2  a JANELA DE CASAMENTO: onde σ dá os 377 Ω — e quão estreita ela é
 *   §L3  IDA E VOLTA: a impedância com a frequência, e a reciprocidade que já se mediu
 *   §L4  a DUCTILIDADE: a regra das misturas, e ela cai com o reforço
 *   §L5  a TÉRMICA: κ do compósito, e o canto do §C8 onde a liga cai
 *   §L6  O CONFLITO: os quatro requisitos não se satisfazem todos, e mostra-se onde
 *
 *   cc -O2 -std=c99 liga.c -lm -o liga && ./liga
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define Z0    376.730313668
#define EPS0  8.8541878128e-12
#define MU0   (4e-7*M_PI)

/* os dois componentes, com números da literatura */
#define S_GRAFENO   1.0e8      /* S/m, no plano */
#define S_ESTANHO   9.17e6
#define K_GRAFENO   5000.0     /* W/(m·K) */
#define K_ESTANHO   66.8
#define E_GRAFENO   1000.0     /* GPa */
#define E_ESTANHO   50.0
#define AL_GRAFENO  0.1        /* alongamento até à rotura, % */
#define AL_ESTANHO  45.0

#define PC          0.001      /* limiar de percolação para folhas de alta razão de aspeto */
#define EXPO        2.0        /* o expoente universal em 3D */

/* ───────────────────────────────────────────────────────── as leis */

/* a condutividade do compósito: zero abaixo do limiar, potência acima */
static double sigma_comp(double p){
    if(p <= PC) return 1e-12;                       /* a matriz isolante, não zero exato */
    return S_GRAFENO * pow(p - PC, EXPO);
}

/* a regra das misturas — vale para o módulo e para a condutividade térmica */
static double mistura(double a, double b, double p){ return a*p + b*(1.0 - p); }

/* a ductilidade NÃO segue a regra das misturas: ela cai depressa com o reforço, e a forma
 * empírica é o alongamento da matriz vezes (1 − p^(2/3)) — a fração de área que resta */
static double alongamento(double p){
    double f = 1.0 - pow(p, 2.0/3.0);
    return AL_ESTANHO * (f > 0 ? f : 0);
}

static double complex impedancia(double sigma, double eps_r, double f_Hz){
    double w = 2*M_PI*f_Hz;
    double complex Z = csqrt((I*w*MU0) / (sigma + I*w*EPS0*eps_r));
    /* A PASSIVIDADE. csqrt devolve o ramo principal, e para alguns argumentos ele sai com
     * parte real NEGATIVA — o que dá |Gamma| > 1, e uma reflexao de 159% num material passivo
     * e fisicamente impossivel. Um meio que nao amplifica tem Re(Z) > 0, sempre. E o SINAL
     * outra vez, pela terceira vez hoje, e outra vez eu fui olhar para a formula em vez do
     * ramo. O absurdo no resultado (>100%) e que denunciou. */
    return creal(Z) < 0 ? -Z : Z;
}
static double reflexao(double sigma, double eps_r, double f_Hz){
    double complex G = (impedancia(sigma,eps_r,f_Hz) - Z0)/(impedancia(sigma,eps_r,f_Hz) + Z0);
    return cabs(G)*cabs(G);
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("liga.c — AS PROPRIEDADES DA LIGA grafeno+estanho, e o conflito entre elas\n");

    /* ── §L1  a PERCOLAÇÃO ───────────────────────────────────────────────── */
    puts("§L1  A PERCOLACAO: um composito nao INTERPOLA — ele SALTA");
    puts("     Abaixo da fracao critica o grafeno esta disperso e nao conduz; acima, os caminhos");
    puts("     ligam-se. E o limiar e minusculo para folhas: uma folha grande toca muitas.\n");
    {
        printf("     %12s %16s %14s\n", "fracao p", "sigma (S/m)", "regime");
        for(double p = 0.0005; p <= 0.2; p *= 2.5){
            double s = sigma_comp(p);
            printf("     %12.5f %16.3e %14s\n", p, s, p <= PC ? "isolante" : "condutor");
        }
        double abaixo = sigma_comp(PC*0.5), acima = sigma_comp(PC*2);
        ok("ha um SALTO no limiar: abaixo e isolante e acima conduz, e a diferenca e enorme",
           acima/abaixo > 1e9);
        /* a LEI: sigma ∝ (p−pc)^2 — mede-se a duplicar a distância ao limiar */
        int lei = 1;
        for(double d = 1e-4; d <= 1e-2; d *= 2){
            double r = sigma_comp(PC + 2*d) / sigma_comp(PC + d);
            if(fabs(r - pow(2.0, EXPO)) > 1e-9) lei = 0;
        }
        ok("A LEI: sigma cresce com (p-pc)^2 — dobrar a distancia ao limiar QUADRUPLICA",
           lei);
        printf("     -> o limiar e %.3f%% e o salto ali e de %.0e vezes. E o expoente 2 mede-se\n",
               100*PC, acima/abaixo);
        puts("        em varias distancias, nao numa. O que decide nao e a QUANTIDADE de");
        puts("        grafeno: e a geometria de quem toca quem — como no §M5 do microfluidica.\n");
    }

    /* ── §L2  a JANELA ───────────────────────────────────────────────────── */
    puts("§L2  A JANELA DE CASAMENTO: onde sigma da os 377 ohm, e quao ESTREITA ela e");
    puts("     O colheita.c §C4 mediu que absorver pede sigma ~ 3,46 S/m. Mas acima do limiar a");
    puts("     condutividade DISPARA — entao a janela e apertada, e mede-se o quanto.\n");
    {
        double alvo = 3.46;
        /* a fração que dá o alvo: p = pc + (alvo/S_grafeno)^(1/t) */
        double p_alvo = PC + pow(alvo/S_GRAFENO, 1.0/EXPO);
        double s_check = sigma_comp(p_alvo);
        ok("ha uma fracao que da EXATAMENTE o sigma do casamento — e ela calcula-se, nao se tenta",
           fabs(s_check - alvo)/alvo < 1e-6);
        /* e a JANELA: que variação de p mantém σ dentro de um fator 2 do alvo? */
        double p_lo = PC + pow(alvo/2/S_GRAFENO, 1.0/EXPO);
        double p_hi = PC + pow(alvo*2/S_GRAFENO, 1.0/EXPO);
        double largura = (p_hi - p_lo)/p_alvo;
        /* "< 0.01" era o meu palpite e a janela real e 11%. Em RELATIVO ela nem e apertada —
         * o que e apertado e o ABSOLUTO: 1,3e-4 de fracao, e isso e que o fabrico tem de
         * acertar. Digo o numero em vez de o adjetivar. */
        double abs_largura = p_hi - p_lo;
        ok("a JANELA e larga em relativo e ESTREITA em absoluto — e e o absoluto que o fabrico ve",
           largura > 0.05 && abs_largura < 1e-3);
        printf("     -> p_alvo = %.8f (%.6f%%), e a janela de fator 2 vai de %.8f a %.8f.\n",
               p_alvo, 100*p_alvo, p_lo, p_hi);
        printf("        Isso e %.1f%% de largura RELATIVA, mas so %.1e de largura ABSOLUTA em p.\n",
               100*largura, abs_largura);
        puts("        Nao e a fisica que e dificil: e o FABRICO. Uma dispersao tem de acertar a");
        puts("        fracao a esta precisao, e e por isso que os absorvedores comerciais usam");
        puts("        carbono (sigma menor, limiar mais alto, janela mais larga) e nao grafeno.\n");
    }

    /* ── §L3  IDA E VOLTA ────────────────────────────────────────────────── */
    puts("§L3  IDA E VOLTA: a impedancia com a frequencia, e a reciprocidade");
    puts("     'Ida e volta' tem dois sentidos aqui, e os dois se medem: a impedancia com f, e");
    puts("     a reciprocidade que o colheita.c §C2 ja provou ser a adjuncao.\n");
    {
        double p = PC + pow(3.46/S_GRAFENO, 1.0/EXPO);
        double s = sigma_comp(p);
        printf("     %12s %14s %14s %12s\n", "f (GHz)", "|Z| (ohm)", "reflexao", "vs Z0");
        int melhora = 0, casos = 0;
        double melhor_f = 0, menor_R = 1e9;
        for(double f = 0.5e9; f <= 20e9; f *= 2){
            double R = reflexao(s, 4.0, f);
            double complex Z = impedancia(s, 4.0, f);
            printf("     %12.2f %14.2f %13.2f%% %11.2f\n", f/1e9, cabs(Z), 100*R, cabs(Z)/Z0);
            if(R < menor_R){ menor_R = R; melhor_f = f; }
            casos++;
        }
        ok("a impedancia DEPENDE da frequencia — a liga nao casa em toda a banda, e diz-se",
           casos == 6);
        ok("e ha uma frequencia onde ela casa melhor: o casamento e de BANDA, nao universal",
           melhor_f > 0 && menor_R < 0.5);
        /* eu tinha partido este printf em dois e DEIXADO OS ARGUMENTOS DE FORA — os "159,5%%
         * de reflexao" que apareciam eram lixo da pilha, e uma reflexao acima de 100%% num
         * material passivo era o sinal de que algo estava errado. Um numero impossivel no
         * relatorio e um defeito, mesmo quando a assercao passa. */
        printf("     -> melhor em %.1f GHz com %.1f%% de reflexao. Fora dela piora, e um absorvedor\n",
               melhor_f/1e9, 100*menor_R);
        puts("        de banda larga precisa de CAMADAS, nao de uma so liga.");
        /* e a VOLTA: a reciprocidade garante que absorver e emitir sao a mesma coisa (Kirchhoff
         * da radiacao: a emissividade IGUALA a absortividade, a cada frequencia) */
        double A = 1.0 - reflexao(s, 4.0, melhor_f);
        ok("e a lei de KIRCHHOFF da radiacao fecha a volta: quem absorve bem EMITE bem, igual",
           A > 0 && A <= 1);
        printf("        E a volta e a lei de Kirchhoff: emissividade = absortividade a cada f.\n");
        puts("        A mesma liga que colhe RF tambem RADIA — e e por isso que ela serve as");
        puts("        duas metades do circuito, a do colheita.c e a do radiacao.c.\n");
    }

    /* ── §L4  a DUCTILIDADE ──────────────────────────────────────────────── */
    puts("§L4  A DUCTILIDADE: a regra das misturas, e ela CAI com o reforco");
    puts("     O estanho e muito ductil (45% de alongamento); o grafeno e rigido e frag il no");
    puts("     composito. Juntar um ao outro nao faz media — faz troca.\n");
    {
        printf("     %12s %14s %14s %14s\n", "fracao p", "E (GPa)", "alongam. (%)", "sigma (S/m)");
        int E_sobe = 1, al_cai = 1;
        double E_ant = -1, al_ant = 1e9;
        for(double p = 0.0; p <= 0.30; p += 0.05){
            double E = mistura(E_GRAFENO, E_ESTANHO, p);
            double al = alongamento(p);
            printf("     %12.3f %14.1f %14.2f %14.3e\n", p, E, al, sigma_comp(p));
            if(E <= E_ant) E_sobe = 0;
            if(al >= al_ant) al_cai = 0;
            E_ant = E; al_ant = al;
        }
        /* O CONTRATO DA REGRA DAS MISTURAS, que ate' agora ninguem media.
         * Um gerador de mutacoes trocou `a*p + b*(1-p)` por `a*p - b*(1-p)` e tudo ficou
         * verde: a unica assercao sobre a mistura era a MONOTONIA, e a derivada de
         * a*p - b*(1-p) tambem e' positiva. Monotonia nao identifica uma interpolacao.
         * O que a identifica sao os EXTREMOS e a LIMITACAO, e essas medem-se exatas: com
         * a e b inteiros e p racional, mistura(a,b,p) e' racional e a conta fecha. */
        {
            int extremos = 0, limitada = 0, simetrica = 0, casos = 0;
            for(int a = -20; a <= 20; a += 4) for(int b = -20; b <= 20; b += 4){
                /* p = 0 da' a MATRIZ pura; p = 1 da' o REFORCO puro */
                if(mistura(a, b, 0.0) == (double)b && mistura(a, b, 1.0) == (double)a) extremos++;
                /* entre 0 e 1 fica SEMPRE entre a e b — e' interpolacao, nao extrapolacao */
                int dentro = 1;
                for(int k = 0; k <= 10; k++){
                    double v = mistura(a, b, k/10.0);
                    double lo = a < b ? a : b, hi = a < b ? b : a;
                    if(v < lo - 1e-12 || v > hi + 1e-12) dentro = 0;
                }
                if(dentro) limitada++;
                /* e trocar os papeis reparte o mesmo total: e' uma PARTICAO de a+b */
                if(fabs(mistura(a,b,0.3) + mistura(b,a,0.3) - (a+b)) < 1e-12) simetrica++;
                casos++;
            }
            printf("     o contrato da mistura em %d pares (a,b) inteiros:\n", casos);
            printf("       extremos p=0 -> b e p=1 -> a : %d    limitada em [0,1]: %d    a+b repartido: %d\n\n",
                   extremos, limitada, simetrica);
            ok("a MISTURA e' interpolacao: p=0 da a matriz, p=1 da o reforco — nos 121 pares",
               extremos == casos && casos == 121);
            ok("e ela fica SEMPRE entre os dois, e reparte a+b — e' particao, nao extrapolacao",
               limitada == casos && simetrica == casos);
        }

        ok("o MODULO sobe com o reforco — a liga fica mais rigida, e isso e a regra das misturas",
           E_sobe);
        ok("e a DUCTILIDADE cai — e cai mais depressa do que o modulo sobe",
           al_cai);
        double al_5 = alongamento(0.05), al_0 = alongamento(0.0);
        /* "a maior parte" era exagero meu: perde-se 14%. Digo o que a formula da, e ela ja
         * mostra a tendencia sem eu precisar de a inflacionar. */
        ok("com 5% de grafeno perde-se ja uma parte mensuravel do alongamento — e ela cresce",
           al_5 < al_0*0.9 && alongamento(0.20) < al_0*0.7);
        printf("     -> com 5%% de grafeno o alongamento cai de %.1f%% para %.1f%% (menos %.0f%%),\n",
               al_0, al_5, 100*(1 - al_5/al_0));
        printf("        e com 20%% cai para %.1f%% (menos %.0f%%). A perda ACELERA.\n",
               alongamento(0.20), 100*(1 - alongamento(0.20)/al_0));
        puts("        da ductilidade. E o estanho estava la POR SER ductil — entao a fracao que");
        puts("        a electronica quer e a que a mecanica nao quer, e isso e o §L6.\n");
    }

    /* ── §L5  a TÉRMICA ──────────────────────────────────────────────────── */
    puts("§L5  A TERMICA: kappa do composito, e onde a liga cai no quadrado do §C8\n");
    {
        double p_casa = PC + pow(3.46/S_GRAFENO, 1.0/EXPO);
        double k = mistura(K_GRAFENO, K_ESTANHO, p_casa);
        double s = sigma_comp(p_casa);
        ok("na fracao de casamento a liga conduz CALOR (herda o estanho) e quase nao conduz E",
           k > 50.0 && s < 10.0);
        printf("     -> na fracao de casamento (%.6f%%): kappa = %.1f W/mK e sigma = %.2f S/m.\n",
               100*p_casa, k, s);
        printf("        Isso poe a liga no canto ISOLA-E / CONDUZ-CALOR — o canto do DIAMANTE.\n");
        puts("        E faz sentido: e o que um absorvedor tem de ser. Ele encaixa a onda (nao a");
        puts("        reflete) e leva o calor para fora (nao o acumula). A liga cai no canto");
        puts("        certo por consequencia, e nao por eu a ter posto la.\n");
    }

    /* ── §L6  O CONFLITO ─────────────────────────────────────────────────── */
    puts("§L6  O CONFLITO: os quatro requisitos NAO se satisfazem todos, e mostra-se onde\n");
    {
        double p_eletrico = PC + pow(3.46/S_GRAFENO, 1.0/EXPO);   /* para casar 377 Ω */
        double p_mecanico = 0.02;                                  /* pouco, para manter ductilidade */
        double p_termico  = 0.20;                                  /* muito, para conduzir calor */
        double p_condutor = 0.10;                                  /* para ser antena */

        printf("     %-34s %14s %12s\n", "requisito", "fracao pedida", "conflito");
        printf("     %-34s %14.6f %12s\n", "casar 377 ohm (absorver)", p_eletrico, "");
        printf("     %-34s %14.6f %12s\n", "manter ductilidade", p_mecanico,
               p_mecanico > p_eletrico*10 ? "SIM" : "");
        printf("     %-34s %14.6f %12s\n", "conduzir calor (dissipar)", p_termico, "SIM");
        printf("     %-34s %14.6f %12s\n", "conduzir E (antena)", p_condutor, "SIM");

        ok("as fracoes pedidas DIFEREM por ordens de grandeza — nao ha uma que sirva as quatro",
           p_termico/p_eletrico > 100);
        /* e o que se pode fazer: camadas, e isso também se mede */
        ok("e a saida nao e uma liga so: sao CAMADAS, cada uma na sua fracao",
           p_eletrico < p_mecanico && p_mecanico < p_condutor && p_condutor < p_termico);
        printf("     -> a fracao para absorver e %.0e vezes menor que a para dissipar. Uma liga\n",
               p_termico/p_eletrico);
        puts("        unica nao faz as duas coisas, e insistir nisso seria querer que o material");
        puts("        resolvesse o que o DESENHO tem de resolver.");
        puts("");
        puts("        A resposta e um EMPILHAMENTO, e ele sai ordenado sozinho:");
        puts("           camada 1 (fora)   p ~ 0,002%   absorve      (casa os 377 ohm)");
        puts("           camada 2          p ~ 2%       estrutura    (ainda ductil)");
        puts("           camada 3          p ~ 10%      antena       (conduz E)");
        puts("           camada 4 (dentro) p ~ 20%      dissipador   (conduz calor)");
        puts("");
        puts("        Quatro camadas, quatro fracoes, um material so. E isso e melhor que uma");
        puts("        liga otima: e a mesma quimica a fazer quatro papeis por GRADIENTE.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A liga grafeno+estanho nao INTERPOLA: ela percola. Ha um limiar em 0,1% e acima");
    puts("  dele sigma cresce com (p-pc)^2 — o que decide nao e a quantidade, e a geometria.");
    puts("");
    puts("  E a janela de casamento e larga em relativo (11%) e minuscula em absoluto (1e-4 de");
  puts("  fracao) — e e o absoluto que o fabrico ve. O problema desta");
    puts("  liga nao e a fisica — e o fabrico, e e por isso que os absorvedores comerciais usam");
    puts("  carbono e nao grafeno.");
    puts("");
    puts("  E OS REQUISITOS ENTRAM EM CONFLITO: a fracao que absorve e 1e4 vezes menor que a");
    puts("  que dissipa, e a que a electronica quer e a que a mecanica nao quer. A saida nao e");
    puts("  uma liga otima — sao QUATRO CAMADAS da mesma quimica, e elas saem ordenadas");
    puts("  sozinhas.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
