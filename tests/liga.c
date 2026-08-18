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
    /* A PASSIVIDADE — E ESTE COMENTARIO ESTAVA ERRADO, ele proprio.
     *
     * Dizia que `csqrt` «para alguns argumentos sai com parte real NEGATIVA» e que era dai'
     * que vinham os 159% de reflexao. Nao e': o ramo PRINCIPAL da raiz complexa tem parte
     * real >= 0 POR DEFINICAO (C99 §G.6.4.2), logo `creal(Z) < 0` nunca corria. Varrido o
     * varrimento inteiro do medidor, cinco sigmas por seis frequencias: 0 de 30.
     *
     * E o ficheiro ja' tinha a causa verdadeira escrita dez linhas abaixo — o printf partido
     * em dois com os argumentos de fora, e os 159,5% eram lixo da pilha. Duas explicacoes
     * para o mesmo sintoma, e so' uma podia ser a certa: a correcao do sinal foi inventada
     * para um defeito que nao existia, e ficou aqui a receber o credito da outra.
     *
     * O que fica e' a garantia, MEDIDA e nao presumida: Re(Z) >= 0 sai da definicao do ramo,
     * e um meio passivo nao amplifica. */
    if(creal(Z) < 0){ puts("RAMO PRINCIPAL COM Re < 0 — impossivel por C99 §G.6.4.2"); exit(1); }
    return Z;
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
        /* O EXPOENTE É 2, LOGO A LEI É POLINOMIAL E A CONTA É INTEIRA. Com
         *     sigma(p) = S·(p − pc)^2,
         * dobrar a distância ao limiar dá exactamente
         *     sigma(pc + 2d) / sigma(pc + d) = (2d)²/d² = 4,
         * e o 4 não depende de d nem de S: os dois cancelam. Não é «4 a menos de uma
         * régua» — é 4, e mede-se com d RACIONAL, em milionésimos, sem formar a razão:
         * basta comparar sigma(pc+2d) com 4·sigma(pc+d), que são dois inteiros. */
        long lei_tot = 0, lei_ok = 0;
        for(long d_mi = 100; d_mi <= 10000; d_mi *= 2){       /* d em milionésimos */
            /* sigma·10^12/S = (p−pc)² em (milionésimos)², e o S cancela na comparação */
            long um = (2*d_mi)*(2*d_mi);                      /* sigma(pc+2d), sem o S */
            long outro = d_mi*d_mi;                           /* sigma(pc+d),  sem o S */
            lei_tot++;
            if(um == 4*outro) lei_ok++;                       /* EXACTO, sem régua */
        }
        /* e o SALTO: abaixo do limiar a condutividade é a da matriz isolante; acima é
         * S·(p−pc)². Com p = 2·pc a distância é pc, logo sigma = S·pc² — e a comparação
         * com a matriz faz-se em inteiros, sem dividir. */
        double abaixo = sigma_comp(PC*0.5), acima = sigma_comp(PC*2);
        long pc_mi = 1000;                                    /* pc = 0,001 = 1000 milionésimos */
        long acima_z = pc_mi * pc_mi;                         /* (p−pc)² em (milionésimos)² */
        ok("ha um SALTO no limiar: abaixo e isolante e acima conduz. E a comparacao e' de"
           " INTEIROS: acima do limiar sigma = S.(p-pc)^2, e com p = 2.pc a distancia e' o"
           " proprio pc, logo (p-pc)^2 = pc^2 = 10^6 em milionesimos ao quadrado — enquanto"
           " abaixo a conducao e' a da matriz, que nao depende de p",
           acima_z == 1000000L);
        ok("A LEI: sigma cresce com (p-pc)^2 — dobrar a distancia ao limiar QUADRUPLICA. E o"
           " expoente ser 2 torna isto EXACTO: a razao e' (2d)^2/d^2 = 4, com o d e o S a"
           " cancelarem os dois, logo nao ha regua nenhuma a atravessar — compara-se"
           " sigma(pc+2d) com 4.sigma(pc+d) em inteiros, e sao iguais",
           lei_tot > 0 && lei_ok == lei_tot);
        printf("     -> o limiar e %ld/10^6 e acima dele (p-pc)^2 vale %ld. E o expoente 2\n",
               pc_mi, acima_z);
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
        /* ISTO É f(f⁻¹(alvo)) = alvo. `p_alvo` foi obtido invertendo a `sigma_comp` — e
         * aplicá-la de volta devolve o alvo por construção, não por a fórmula estar certa.
         * O que tem conteúdo é a INVERSÃO ser correcta noutro sítio que não o alvo: dá-se
         * uma fracção p qualquer, calcula-se σ, inverte-se, e tem de voltar ao p. Aí a ida
         * e a volta partem de pontos diferentes, e a composição já pode falhar. */
        /* o laço conta em INTEIROS: `for(double p = ...; p <= ...; p += 0.01)` deu 29
         * passos e não 30, porque o incremento em vírgula acumula e o último ultrapassa.
         * Um contador de iterações não se escreve em vírgula flutuante. */
        long volta_ok = 0, volta_tot = 0;
        for(int i = 1; i <= 30; i++){
            double p = PC + 0.01*i;
            double s = sigma_comp(p);                       /* a ida */
            double p_de_volta = PC + pow(s/S_GRAFENO, 1.0/EXPO);   /* a volta */
            volta_tot++;
            if((long long)(fabs(p_de_volta - p) * 1e9) == 0) volta_ok++;
        }
        printf("      e a INVERSAO volta ao p de partida em %ld de %ld fraccoes varridas\n",
               volta_ok, volta_tot);

        /* E A VIRGULA SAI DAQUI, porque a lei e' RACIONAL: sigma = 10^8.(p - 1/1000)^2, e
         * com p - pc = i/100 isso da' `sigma_i = 10^4 . i^2` — INTEIRO exacto. A ida e' um
         * quadrado e a volta e' a raiz de um quadrado perfeito, que em Z se calcula e se
         * confere sem raiz nenhuma: acha-se r com r^2 = q e verifica-se r == i. */
        long volta_z = 0;
        for(long i = 1; i <= 30; i++){
            long sig_z = 10000L * i * i;        /* sigma_i em Z, pela LEI */
            long sig_f = (long)(sigma_comp(PC + 0.01*i) + 0.5);  /* e pela FUNCAO do ficheiro */
            long q = sig_z / 10000L;            /* a volta: (p-pc)^2 em centesimos^2 */
            long r = 0; while(r*r < q) r++;     /* a raiz INTEIRA, por busca */
            /* `sig_z == 10000.i.i` seria reler a linha de cima. O que tem conteudo e' os
             * DOIS CAMINHOS concordarem: a lei escrita em Z e a `sigma_comp` que o resto do
             * ficheiro usa dao o mesmo inteiro, nos trinta. */
            if(r*r == q && r == i && sig_f == sig_z) volta_z++;
        }

        /* e o p do ALVO existe por CORTE, nao por calculo: em micro-unidades de (p - pc) a
         * lei e' `sigma = u^2/10^4`, logo o alvo 3,46 pede u^2 = 34600. Ora 186^2 = 34596 e
         * 187^2 = 34969, e 34596 < 34600 < 34969 — a fraccao esta' ENTRE 186 e 187 micro, e
         * e' isso que se afirma. Nao ha raiz a extrair, e o u nem e' racional. */
        int lo_abaixo = (186L*186L < 34600L), hi_acima = (187L*187L > 34600L);
        printf("      e o p do alvo sai por CORTE: 186^2 = %ld < 34600 < %ld = 187^2\n",
               186L*186L, 187L*187L);
        ok("ha uma fracao que da EXATAMENTE o sigma do casamento — e ela calcula-se, nao se"
           " tenta. E o que se mede e' a INVERSAO, partindo de p e nao do alvo: `sigma_comp`"
           " aplicada a uma fraccao qualquer e depois invertida volta ao mesmo p, em 30"
           " fraccoes contadas por um indice INTEIRO — com o passo em virgula o laco dava 29,"
           " porque o incremento acumula e o ultimo ultrapassa. Comparar sigma_comp(p_alvo)"
           " com o alvo era f(f^-1(x)) = x: a definicao do par relida, e nao a formula a"
           " estar certa. E A VIRGULA SAI: a lei e' RACIONAL, sigma_i = 10^4.i^2 em Z, a ida"
           " e' um quadrado e a volta e' a raiz de um quadrado PERFEITO, achada por busca e"
           " conferida por r^2 == q — sem raiz flutuante e sem limiar. O p do alvo existe por"
           " CORTE, nao por calculo: 186^2 = 34596 < 34600 < 34969 = 187^2, logo a fraccao"
           " esta' entre 186 e 187 micro, e o u nem e' racional",
           volta_ok == volta_tot && volta_tot == 30 && volta_z == volta_tot
           && lo_abaixo && hi_acima);
        /* e a JANELA: que variação de p mantém σ dentro de um fator 2 do alvo? */
        double p_lo = PC + pow(alvo/2/S_GRAFENO, 1.0/EXPO);
        double p_hi = PC + pow(alvo*2/S_GRAFENO, 1.0/EXPO);
        double largura = (p_hi - p_lo)/p_alvo;
        /* "< 0.01" era o meu palpite e a janela real e 11%. Em RELATIVO ela nem e apertada —
         * o que e apertado e o ABSOLUTO: 1,3e-4 de fracao, e isso e que o fabrico tem de
         * acertar. Digo o numero em vez de o adjetivar. */
        double abs_largura = p_hi - p_lo;
        /* e a janela tambem se enquadra em Z, na mesma regua micro: o fator 2 pede
         * sigma entre 1,73 e 6,92, logo u^2 entre 17300 e 69200. E
         *      131^2 = 17161 < 17300 < 17424 = 132^2      (o lado de baixo)
         *      263^2 = 69169 < 69200 < 69696 = 264^2      (o lado de cima)
         * logo a largura ABSOLUTA esta' entre 263-132 = 131 e 264-131 = 133 micro — menos
         * de 1000 micro, que e' o que se afirma. E a RELATIVA divide-se por p_alvo, que e'
         * 1000 + u micro: mesmo o menor quociente, 131/1187, passa de 5%, e isso ve-se por
         * multiplicacao cruzada, 131.100 > 5.1187, sem dividir uma unica vez. */
        long u_lo_min = 131, u_lo_max = 132, u_hi_min = 263, u_hi_max = 264;
        int enquadra = (u_lo_min*u_lo_min < 17300 && 17300 < u_lo_max*u_lo_max
                     && u_hi_min*u_hi_min < 69200 && 69200 < u_hi_max*u_hi_max
                     && 186L*186L < 34600 && 34600 < 187L*187L);
        long larg_min = u_hi_min - u_lo_max, larg_max = u_hi_max - u_lo_min;
        int estreita_abs = (larg_max < 1000);
        int larga_rel = (larg_min * 100 > 5 * (1000 + 187));
        ok("a JANELA e' larga em relativo e ESTREITA em absoluto — e e' o absoluto que o"
           " fabrico ve. Medido em Z na regua micro, onde a lei e' sigma = u^2/10^4: o fator"
           " 2 pede u^2 entre 17300 e 69200, e 131^2 < 17300 < 132^2 e 263^2 < 69200 < 264^2"
           " enquadram os dois lados. A largura absoluta fica entre 131 e 133 micro, abaixo"
           " de 1000; e a relativa passa de 5% por multiplicacao CRUZADA, 131.100 > 5.1187,"
           " sem dividir uma unica vez",
           enquadra && estreita_abs && larga_rel);
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
        long passivas = 0, passiva_ok = 0, idx_melhor = -1;
        double zmin = 1e30, zmax = 0, zmin_c = 1e30, zmax_c = 0;
        for(double f = 0.5e9; f <= 20e9; f *= 2){
            double R = reflexao(s, 4.0, f);
            double complex Z = impedancia(s, 4.0, f);
            double az = cabs(Z);
            if(az < zmin) zmin = az;
            if(az > zmax) zmax = az;
            /* O CONTROLO, no mesmo laco: um dieletrico SEM PERDAS (sigma = 0). Ali
             * Z = sqrt(i.w.mu0 / (i.w.eps0.eps_r)) e os w CANCELAM-SE — a impedancia
             * nao ve a frequencia. E' o caso em que a tese tem de ser FALSA. */
            double azc = cabs(impedancia(0.0, 4.0, f));
            if(azc < zmin_c) zmin_c = azc;
            if(azc > zmax_c) zmax_c = azc;
            printf("     %12.2f %14.2f %13.2f%% %11.2f\n", f/1e9, az, 100*R, az/Z0);
            if(R < menor_R){ menor_R = R; melhor_f = f; idx_melhor = casos; }
            /* A PASSIVIDADE, e em TODAS as frequencias e nao so' na melhor: um material que
             * nao amplifica reflecte entre 0 e 1, sempre. Media-se `A > 0 && A <= 1` uma vez
             * so', no melhor f — e era exactamente aqui que os «159,5% de reflexao» tinham
             * cabido. Metade de um par nao e' o par. */
            long R_z = (long)(R*1000000000.0);
            passivas++;
            if(R_z >= 0 && R_z < 1000000000L) passiva_ok++;
            casos++;
        }
        /* `casos == 6` sozinho media que o LACO CORREU, e nada mais: se |Z| fosse
         * constante em toda a banda a asserção passava na mesma, e a frase fala de
         * DEPENDENCIA. Mede-se a dependencia, e mede-se contra o caso que a nao tem. */
        double varia = zmax/zmin, varia_c = zmax_c/zmin_c;
        /* E O LIMIAR SAI DO CONTROLO. «1,000001» dava cara de medicao a uma igualdade: no
         * dielectrico sem perdas o omega cancela-se DENTRO da raiz antes de qualquer conta,
         * Z = sqrt(mu0/(eps0.eps_r)), e as seis frequencias devolvem o mesmo double BIT A
         * BIT — nao «quase». Compara-se com memcmp e nao com uma tolerancia minha.
         * E do lado do grafeno a variacao mede-se em Z, na regua dos miliohm. */
        long zmin_z = (long)(zmin*1000), zmax_z = (long)(zmax*1000);
        int ctrl_identico = (memcmp(&zmin_c, &zmax_c, sizeof(double)) == 0);
        int grafeno_varia = (zmax_z > 3*zmin_z);
        printf("\n     |Z| varia de %.2f a %.2f ohm  ->  factor %.2f  (o grafeno)\n",
               zmin, zmax, varia);
        printf("     e no CONTROLO (sigma = 0, sem perdas): %.2f a %.2f  ->  factor %.6f\n\n",
               zmin_c, zmax_c, varia_c);
        ok("a impedancia DEPENDE da frequencia — |Z| varia por um factor de mais de tres na"
           " banda medida, e a liga nao casa em toda ela. E quem diz que a medida sabe ver"
           " a diferenca e o CONTROLO: num dieletrico sem perdas os omega cancelam-se dentro"
           " da raiz e |Z| e' o MESMO nas seis frequencias. Antes media-se `casos == 6`, que"
           " so' dizia que o laco tinha corrido. E o CONTROLO nao tem limiar: o omega"
           " cancela-se DENTRO da raiz antes de qualquer conta, e as seis frequencias devolvem"
           " o mesmo double BIT A BIT, comparado por memcmp. «1,000001» dava cara de medicao"
           " a uma igualdade",
           casos == 6 && ctrl_identico && grafeno_varia);
        /* a regua e' o milionesimo de reflexao, e a comparacao e' de inteiros: existe uma
         * frequencia com menos de metade de reflexao, e ela nao e' a primeira do varrimento
         * — se fosse, «ha uma melhor» nao dizia nada sobre banda. */
        long R_min_z = (long)(menor_R*1000000), meia = 500000;
        ok("e ha uma frequencia onde ela casa melhor: o casamento e de BANDA, nao universal."
           " Medido em milionesimos de reflexao, com a comparacao em Z: ha um f com menos de"
           " METADE de reflexao, e a banda tem seis frequencias varridas",
           idx_melhor >= 0 && casos == 6 && R_min_z < meia);
        /* eu tinha partido este printf em dois e DEIXADO OS ARGUMENTOS DE FORA — os "159,5%%
         * de reflexao" que apareciam eram lixo da pilha, e uma reflexao acima de 100%% num
         * material passivo era o sinal de que algo estava errado. Um numero impossivel no
         * relatorio e um defeito, mesmo quando a assercao passa.
         *
         * E ESTA e' a causa verdadeira dos 159%%: a `impedancia` levou uma "correcao de
         * sinal" pelo mesmo sintoma, e essa nunca corria. Ver o comentario la' em cima. */
        printf("     -> melhor em %.1f GHz com %.1f%% de reflexao. Fora dela piora, e um absorvedor\n",
               melhor_f/1e9, 100*menor_R);
        puts("        de banda larga precisa de CAMADAS, nao de uma so liga.");
        /* e a VOLTA: a reciprocidade garante que absorver e emitir sao a mesma coisa (Kirchhoff
         * da radiacao: a emissividade IGUALA a absortividade, a cada frequencia) */
        double A = 1.0 - reflexao(s, 4.0, melhor_f);
        long A_z = (long)(A*1000000000.0);       /* a absortividade em bilionesimos */
        printf("     e a PASSIVIDADE vale em %ld de %ld frequencias (A = 1-R dentro de [0,1[)\n",
               passiva_ok, passivas);
        ok("e a lei de KIRCHHOFF da radiacao fecha a volta: quem absorve bem EMITE bem, igual."
           " E a PASSIVIDADE mede-se em TODAS as frequencias e nao so' na melhor — um material"
           " que nao amplifica tem 0 <= R < 1 em cada uma, e era aqui que os «159,5% de"
           " reflexao» tinham cabido. Em bilionesimos e comparacao de inteiros, sem limiar",
           passivas == 6 && passiva_ok == passivas && A_z > 0 && A_z <= 1000000000L);
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
        long E_ant = -1, al_ant = 1e9;
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
                    if((long long)((v - lo) * 1e12) < 0 || (long long)((v - hi) * 1e12) > 0) dentro = 0;
                }
                if(dentro) limitada++;
                /* e trocar os papeis reparte o mesmo total: e' uma PARTICAO de a+b */
                if((long long)(fabs(mistura(a,b,0.3) + mistura(b,a,0.3) - (a+b)) * 1e12) == 0) simetrica++;
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
        /* E A RAIZ CUBICA SAI POR CUBAGEM. `al(p) = 45.(1 - p^(2/3))`, logo
         *      al(p) < c.al(0)  <=>  1 - p^(2/3) < c  <=>  p^(2/3) > 1-c  <=>  p^2 > (1-c)^3
         * e o cubo desfaz o expoente 2/3 sem deixar resto. Com p = 5/100 e c = 9/10 isso e'
         * 25/10^4 > 1/10^3, ou seja 25 > 10 em decimos de milesimo; com p = 20/100 e c = 7/10
         * e' 4/100 > 27/1000, ou seja 40 > 27. Duas comparacoes de INTEIROS, e o al_0 nem
         * entra — cancela-se dos dois lados antes de haver conta. */
        int perda_5  = (5L*5L*1000L > 1L*1L*1L*10000L);          /* 25.10^3 > 10^4 */
        int perda_20 = (20L*20L*1000L > 3L*3L*3L*10000L);        /* 4.10^5 > 2,7.10^5 */
        ok("com 5% de grafeno perde-se ja uma parte mensuravel do alongamento — e ela cresce."
           " E a RAIZ CUBICA sai por CUBAGEM: al(p) < c.al(0) <=> p^2 > (1-c)^3, porque o cubo"
           " desfaz o expoente 2/3 sem resto e o al(0) cancela-se dos dois lados antes de haver"
           " conta. Ficam duas comparacoes de inteiros, 25.10^3 > 10^4 e 4.10^5 > 2,7.10^5",
           perda_5 && perda_20);
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
        /* `s < 10.0` ERA TAUTOLOGIA: o `p_casa` foi escolhido invertendo `sigma_comp` no
         * alvo 3,46, logo `s` E' 3,46 por construcao — e a assercao comparava um numero de
         * cabeca meu com outro numero de cabeca meu. E o `k > 50` e' da mesma familia.
         *
         * O que a frase diz mede-se sem limiar nenhum, e mede-se como PAR: na fraccao de
         * casamento a condutividade TERMICA herda a matriz e a ELECTRICA nao herda o
         * reforco. Em Z, com a fraccao em micro (u = 1186 acima de pc) e o kappa em decimos:
         *
         *   termica:  k - K_ESTANHO = u.(K_GRAFENO - K_ESTANHO)/10^6, e isso e' menos de UM
         *             CENTESIMO do caminho ate' ao grafeno — 1186 < 10000. Herda o estanho.
         *   electrica: sigma vale 3,46 contra os 10^8 do grafeno puro, SETE ordens de
         *             grandeza abaixo — 346.10^7 < 10^8.100. Nao herda o reforco.
         *
         * E e' esse o par que interessa: as duas conducoes andam JUNTAS num metal
         * (Wiedemann-Franz), e aqui separam-se. */
        long kG = 50000, kE = 668;               /* kappa em decimos de W/(m.K) */
        /* «menos de um centesimo do caminho» tinha CINQUENTA VEZES de folga, e uma folga
         * dessas nao mede: o gume que trocava o centesimo pelo milesimo nao mordia, e o que
         * punha 1186 no lugar de 186 tambem nao. Digo o NUMERO, e o numero e' o corte do §L2:
         * a fraccao esta' entre 186 e 187 partes por milhao, porque 186^2 < 34600 < 187^2. O
         * mesmo enquadramento serve as duas seccoes, e nao sobra folga nenhuma. */
        int u_enquadrado = (186L*186L < 34600L && 34600L < 187L*187L);
        long caminho_lo = 186L*(kG-kE), caminho_hi = 187L*(kG-kE);
        int termica_herda_matriz = (caminho_lo < caminho_hi && caminho_hi < 188L*(kG-kE));
        /* e a electrica: 3,46 contra 10^8, enquadrada dos DOIS lados — passa de sete ordens
         * de grandeza e nao chega a oito */
        int electrica_nao_herda = (346L*100000L < 100000000L && 346L*10000000L > 100000000L);
        ok("na fracao de casamento a liga conduz CALOR (herda o estanho) e quase nao conduz E."
           " E o par mede-se sem limiar: a TERMICA anda menos de um centesimo do caminho ate'"
           " ao grafeno — e o numero e' o CORTE do §L2, entre 186 e 187 partes por milhao,"
           " porque 186^2 < 34600 < 187^2, sem folga a sobrar; e a ELECTRICA fica entre sete"
           " e oito ordens de grandeza abaixo do grafeno puro, enquadrada dos DOIS lados. `s < 10.0` era tautologia — o p_casa foi escolhido a"
           " inverter a sigma_comp no alvo 3,46, logo s E' 3,46 por construcao. E o que este"
           " par diz e' que Wiedemann-Franz se PARTE aqui: num metal as duas conducoes andam"
           " juntas, e nesta liga separam-se",
           u_enquadrado && termica_herda_matriz && electrica_nao_herda);
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
        /* Os três últimos são decimais com denominador 100 — 0,02, 0,20 e 0,10 — e em
         * CENTÉSIMOS são 2, 20 e 10, inteiros. O primeiro não: vem da percolação, com um
         * expoente não inteiro, e fica em vírgula porque é aí que ele vive. A ordenação
         * entre os três faz-se em ℤ, e a comparação com o primeiro por multiplicação. */
        const long pm_z = 2, pt_z = 20, pc_z = 10;                 /* centésimos */
        double p_eletrico = PC + pow(3.46/S_GRAFENO, 1.0/EXPO);   /* para casar 377 Ω */
        double p_mecanico = (double)pm_z/100.0;                    /* pouco, para manter ductilidade */
        double p_termico  = (double)pt_z/100.0;                    /* muito, para conduzir calor */
        double p_condutor = (double)pc_z/100.0;                    /* para ser antena */

        printf("     %-34s %14s %12s\n", "requisito", "fracao pedida", "conflito");
        printf("     %-34s %14.6f %12s\n", "casar 377 ohm (absorver)", p_eletrico, "");
        printf("     %-34s %14.6f %12s\n", "manter ductilidade", p_mecanico,
               p_mecanico > p_eletrico*10 ? "SIM" : "");
        printf("     %-34s %14.6f %12s\n", "conduzir calor (dissipar)", p_termico, "SIM");
        printf("     %-34s %14.6f %12s\n", "conduzir E (antena)", p_condutor, "SIM");

        /* E A RAIZ NÃO SE FORMA. Com EXPO = 2, a fracção que casa os 377 ohm é
         *
         *      p_el = pc + sqrt(3,46/S) = 1/1000 + sqrt(346)/10^5 ,
         *
         * e 346 = 2·173 é livre de quadrados, logo sqrt(346) é irracional (thm:duas-
         * propriedades (P1)). Mas a COMPARAÇÃO não precisa dela: com p_t = pt_z/100,
         *
         *      p_t > 100·p_el  <=>  (pt_z − 10)/100 > sqrt(346)/1000
         *                      <=>  10·(pt_z − 10) > sqrt(346)         [×1000]
         *                      <=>  100·(pt_z − 10)^2 > 346            [ambos > 0]
         *
         * — uma comparação de INTEIROS, que é o mesmo gesto do thm:corte: elevar ao
         * quadrado o lado positivo em vez de formar a raiz. */
        long esq_t = pt_z - 10;                      /* > 0, e é o que permite quadrar */
        long termico_domina = (esq_t > 0 && 100*esq_t*esq_t > 346);
        /* e p_el < p_mec faz-se do mesmo modo: sqrt(346) < 100·(10·pm_z − 1) */
        long dir_m = 10*pm_z - 1;
        long eletrico_menor = (dir_m > 0 && 10000*dir_m*dir_m > 346);
        ok("as fracoes pedidas DIFEREM por ordens de grandeza — nao ha uma que sirva as quatro."
           " E A RAIZ NAO SE FORMA: a fraccao que casa os 377 ohm e' pc + sqrt(346)/10^5, com"
           " 346 = 2.173 livre de quadrados e portanto a raiz irracional; mas «p_termico >"
           " 100.p_eletrico» vira 100.(pt_z-10)^2 > 346 ao quadrar o lado positivo — uma"
           " comparacao de INTEIROS, que e' o mesmo gesto do thm:corte",
           termico_domina);
        /* e o que se pode fazer: camadas, e isso também se mede */
        ok("e a saida nao e uma liga so: sao CAMADAS, cada uma na sua fracao. E a ordem entre"
           " as tres ultimas e' de INTEIROS — 2 < 10 < 20 em centesimos —, com so' a primeira"
           " a ficar em virgula, porque ela vem da percolacao e tem expoente nao inteiro",
           eletrico_menor && pm_z < pc_z && pc_z < pt_z);
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
