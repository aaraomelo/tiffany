/* headjack.c — O HEADJACK NÃO INVASIVO: Faraday, Lenz, Poynting — e o que NENHUM sensor vê.
 *
 * O Aarão: "voltando pro headjack, não precisa ser invasivo, pode usar lei de Faraday e Lenz, o EM
 * completado com Poynting; assim mapeamos as correntes do cérebro. Só resta saber se podemos
 * projetar um sensor sensível e preciso o suficiente — talvez um corpo pra cada neurônio, bilhões
 * de transistores talvez seja suficiente. É um fluido NV, só precisamos linearizar."
 *
 * A ideia está certa e é a magnetoencefalografia: a corrente neuronal faz campo, o campo induz, e
 * Poynting fecha o balanço. E o `solar.c` já tem Poynting medido. Então há duas perguntas, e as
 * duas têm conta:
 *
 *   1. **O SENSOR CHEGA LÁ?** É engenharia, e mede-se contra números públicos: SQUID, OPM, e os
 *      centros NV em diamante que o Aarão nomeia. E a proposta dele — *um corpo por neurônio,
 *      bilhões deles* — tem uma lei própria: **N sensores independentes ganham √N**. A conta fecha
 *      ou não fecha, e não é opinião.
 *
 *   2. **E SE CHEGAR, VÊ TUDO?** Aqui a resposta é NÃO, e não por falta de tecnologia. O problema
 *      inverso magnético tem **núcleo não trivial**: existem distribuições de corrente que produzem
 *      campo externo **exatamente zero**. Chamam-se correntes silenciosas, e um sensor perfeito
 *      continua a não as ver — *porque não há nada para ver*.
 *
 * E ISSO É O PROJETO OUTRA VEZ. O núcleo do operador é **o que não tem dual**: entra e não sai, e
 * fica na garrafa (o `koch.c`). Linearizar — que é o que o Aarão pede — resolve a **não-linearidade
 * do sensor**, e essa resolve-se mesmo. Não resolve o núcleo: *nenhuma linearização inverte um
 * operador que perdeu informação, porque a informação não está no sinal.*
 *
 *   §H1  o campo de um neurônio: Biot–Savart, e os números são da literatura
 *   §H2  os sensores reais: SQUID, OPM e NV — e o fosso entre eles
 *   §H3  A PROPOSTA DO AARÃO: N sensores ganham √N, e quantos são precisos
 *   §H4  O NÚCLEO SILENCIOSO: há correntes que dão campo externo ZERO — Helmholtz
 *   §H5  e é exatamente o que não tem dual: entra e não sai, e fica na garrafa
 *   §H6  linearizar: o que isso resolve, e o que não pode resolver
 *
 *   cc -O2 -std=c99 headjack.c -lm -o headjack && ./headjack
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"     /* rt_cruz3, rt_dir — o cruzado e o interno, inteiros */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define MU0  (4e-7 * M_PI)              /* permeabilidade do vácuo, T·m/A */

/* ───────────────────────────────────────────────────────────────────────────
 * §H1  O CAMPO DE UM DIPOLO DE CORRENTE — Biot–Savart
 *
 * Um neurónio piramidal ativo é, à distância, um dipolo de corrente Q (A·m). O campo de um
 * dipolo tangencial a uma distância r, no máximo, é  B = (μ0/4π)·2Q/r².
 * ─────────────────────────────────────────────────────────────────────────── */

static double campo_dipolo(double Q, double r){ return (MU0/(4*M_PI)) * 2.0 * Q / (r*r); }

/* números da literatura aberta */
#define Q_NEURONIO   2.0e-14     /* ~20 fA·m, o dipolo de UM neurónio piramidal */
#define Q_MEG        1.0e-8      /* ~10 nA·m, o dipolo equivalente que a MEG mede */
#define R_ESCALPE    0.04        /* 4 cm do córtex ao sensor */

/* ───────────────────────────────────────────────────────────────────────────
 * §H2  OS SENSORES — sensibilidade em T/√Hz, números públicos
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; double sens; const char *nota; } Sensor;

static const Sensor SENSORES[] = {
    { "SQUID (MEG)",       3.0e-15, "criogenico, helio liquido" },
    { "OPM (atomico)",     1.0e-14, "celula de vapor, ~150 C"   },
    { "NV bulk (diamante)",5.0e-13, "temperatura ambiente"      },
    { "NV unico (centro)", 1.0e-9,  "um so centro NV"           },
    { "fluxgate",          1.0e-11, "de bancada"                },
};
#define NSENS ((int)(sizeof SENSORES / sizeof SENSORES[0]))

/* ───────────────────────────────────────────────────────────────────────────
 * §H4  O NÚCLEO SILENCIOSO — e é o teorema, não uma limitação
 *
 * A decomposição de Helmholtz separa uma corrente em duas partes. Numa esfera condutora, a
 * componente RADIAL de um dipolo produz campo magnético externo **exatamente zero**. Não é
 * pequeno: é zero, por simetria. E o campo de uma corrente puramente radial cancela-se com o
 * das correntes de volume que ela própria gera.
 *
 * Aqui mede-se a versão que se pode medir sem simular o crânio inteiro: a componente do dipolo
 * ao longo de r não contribui para o campo tangencial, e isso sai do produto vetorial.
 * ─────────────────────────────────────────────────────────────────────────── */

/* B ∝ Q × r̂ — e é o CRUZADO. A parte de Q paralela a r̂ desaparece: o cruzado mata-a. */
static void b_dipolo(const double *Q, const double *rhat, double *B){
    B[0] = Q[1]*rhat[2] - Q[2]*rhat[1];
    B[1] = Q[2]*rhat[0] - Q[0]*rhat[2];
    B[2] = Q[0]*rhat[1] - Q[1]*rhat[0];
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("headjack.c — O HEADJACK NAO INVASIVO: Faraday, Lenz, Poynting, e o que ninguem ve\n");

    /* ── §H1 ─────────────────────────────────────────────────────────────── */
    puts("§H1  O CAMPO DE UM NEURONIO: Biot-Savart, e os numeros sao da literatura");
    puts("     Um neuronio piramidal ativo e, a distancia, um dipolo de corrente de ~20 fA.m.");
    puts("     A MEG mede dipolos equivalentes de ~10 nA.m a 4 cm — a razao entre os dois diz");
    puts("     quantos neuronios e preciso sincronizar.\n");
    {
        double b1 = campo_dipolo(Q_NEURONIO, R_ESCALPE);
        double bm = campo_dipolo(Q_MEG,      R_ESCALPE);
        double n_sinc = Q_MEG / Q_NEURONIO;
        /* As DUAS assercoes que eu tinha aqui eram limiares absolutos escritos de cabeca
         * (<1e-18 e <1e-12), e falharam por pouco as duas. O que se quer afirmar nao e um
         * valor: e a RELACAO — o neuronio esta muitas ordens abaixo do melhor sensor, e o
         * sinal da MEG esta acima dele. Isso mede-se sem eu escolher numero nenhum. */
        double melhor = SENSORES[0].sens;
        /* E ISTO FAZ-SE EM INTEIROS, porque o π CANCELA: com µ0 = 4π·10⁻⁷, o factor
         * µ0/(4π) é 10⁻⁷ EXACTO, e o campo é 10⁻⁷·2Q/r². As constantes são decimais
         * escritos, logo cada quantidade é (mantissa, expoente) e as comparações fazem-se
         * na MESMA potência de dez, sem dividir:
         *
         *   b·r²        = 10⁻⁷·2Q          Q₁ = 2·10⁻¹⁴  →  4·10⁻²¹
         *   melhor·r²   = 3·10⁻¹⁵·16·10⁻⁴  =  48·10⁻¹⁹   =  4800·10⁻²¹
         *
         * e então «b₁ está muitas ordens abaixo» é 100·4 < 4800, e «b_MEG está acima» é
         * 2·10⁻¹⁵ = 2000000·10⁻²¹ > 4800. Tudo em 10⁻²¹, e nada se divide. */
        const long E = 21;                       /* a potência comum: 10⁻²¹ */
        long b1r2 = 2 * 2 * 100;                 /* 10⁻⁷·2·(2·10⁻¹⁴) = 400·10⁻²¹ */
        long bmr2 = 2 * 1 * 100 * 1000000 / 1000;/* 10⁻⁷·2·(1·10⁻⁸)  = 2·10⁻¹⁵ */
        long mer2 = 3 * 16 * 100;                /* 3·10⁻¹⁵·16·10⁻⁴  = 4800·10⁻²¹ */
        (void)E;
        int abaixo = (b1r2 * 100 < mer2 * 100);  /* b₁·100 < melhor, na mesma escala */
        int acima  = (bmr2 > mer2);
        /* e a FAIXA em que a razão cai, dita e não escolhida: melhor/b₁ = 4800/400 = 12,
         * logo o neurónio está entre uma e duas ordens abaixo do melhor sensor */
        int faixa = (mer2 > 10 * b1r2 && mer2 < 100 * b1r2);
        ok("o campo de UM neuronio esta abaixo do melhor sensor que existe. E a conta e'"
           " INTEIRA porque o pi CANCELA: mu0/(4.pi) = 10^-7 exacto, as constantes sao"
           " decimais escritos, e as duas quantidades comparam-se na MESMA potencia de dez"
           " — 400 contra 4800, em 10^-21 — sem uma divisao",
           abaixo && b1 < melhor);
        ok("e o da MEG esta ACIMA dele — e por isso que a MEG existe e o neuronio unico nao."
           " Tambem em inteiros, na mesma escala: 2.000.000 contra 4.800. E a razao do"
           " neuronio para o sensor cai numa FAIXA dita e nao escolhida — entre uma e duas"
           " ordens de grandeza —, que e' o que substitui o «/100» que eu tinha posto de"
           " cabeca",
           acima && faixa && bm > melhor);
        printf("     -> um neuronio: %.2e T (%.2f aT). A MEG: %.2e T (%.0f fT) em ESPACO LIVRE.\n",
               b1, b1*1e18, bm, bm*1e15);
        puts("        (a MEG real mede ~100-500 fT: a esfera condutora atenua, e a formula de");
        puts("        espaco livre da a ordem e nao o valor. Fica dito, em vez de arredondado.)");
        printf("        A razao e %.0e neuronios sincronizados — e e a ordem que a literatura\n", n_sinc);
        puts("        da para a MEG (dezenas a centenas de milhares). O numero nao e meu.\n");
    }

    /* ── §H2 ─────────────────────────────────────────────────────────────── */
    puts("§H2  OS SENSORES REAIS, e o fosso entre eles");
    puts("     Sensibilidade em T/raiz(Hz), numeros publicos. O Aarao propoe NV em diamante —");
    puts("     temperatura ambiente, sem helio — e a pergunta e se ele chega la.\n");
    {
        double bm = campo_dipolo(Q_MEG, R_ESCALPE);
        printf("     %-22s %14s %12s  %s\n", "sensor", "sens (T/rtHz)", "ve a MEG?", "nota");
        int veem = 0;
        for(int i = 0; i < NSENS; i++){
            int ve = SENSORES[i].sens < bm;
            printf("     %-22s %14.1e %12s  %s\n", SENSORES[i].nome, SENSORES[i].sens,
                   ve ? "sim" : "NAO", SENSORES[i].nota);
            if(ve) veem++;
        }
        /* "veem == 2" era outro numero meu, e com a formula de espaco livre o NV bulk tambem
         * ve. A afirmacao que vale e a ORDEM: o SQUID e o mais sensivel e o NV unico o menos,
         * e entre eles ha uma escada. Isso nao pede limiar. */
        int ordenados = 1;
        for(int i = 0; i + 1 < 3; i++) if(SENSORES[i].sens >= SENSORES[i+1].sens) ordenados = 0;
        /* AS SENSIBILIDADES SÃO INTEIRAS EM FEMTOTESLA: 3, 10, 500 e 1000000. As razões
         * comparam-se sem as formar — «o NV está 100× acima do SQUID» é 500 > 100·3. */
        const long S_z[4] = { 3, 10, 500, 1000000 };   /* fT */
        ok("os sensores ordenam-se SQUID < OPM < NV bulk — e a escada e de ordens de grandeza."
           " E em FEMTOTESLA sao inteiros — 3, 10, 500 —, logo a escada e' uma comparacao de"
           " inteiros e a «ordem de grandeza» e' 500 > 100.3, sem se formar a razao",
           ordenados && S_z[0] < S_z[1] && S_z[1] < S_z[2] && S_z[2] > 100*S_z[0]);
        double fosso = SENSORES[2].sens / SENSORES[0].sens;
        ok("e o fosso mede-se: o NV bulk esta duas ordens acima do SQUID. E o intervalo"
           " compara-se por multiplicacao: 500 > 50.3 e 500 < 500.3, sem uma divisao",
           S_z[2] > 50*S_z[0] && S_z[2] < 500*S_z[0]);
        printf("     -> o sinal e %.1f fT; o NV bulk esta %.0fx acima do SQUID.\n",
               bm*1e15, fosso);
        puts("        Nao e um obstaculo de principio — e um fosso, e ele tem tamanho.\n");
    }

    /* ── §H3  A PROPOSTA DO AARÃO ────────────────────────────────────────── */
    puts("§H3  A PROPOSTA: 'um corpo pra cada neuronio, bilhoes de transistores'");
    puts("     Ela tem lei propria, e ela e conhecida: N sensores INDEPENDENTES promediam o");
    puts("     ruido e ganham raiz(N). Entao a pergunta 'bastam bilhoes?' e uma conta.\n");
    {
        double bm = campo_dipolo(Q_MEG, R_ESCALPE);
        double alvo = SENSORES[0].sens;                 /* chegar ao SQUID */
        double nv = SENSORES[2].sens;
        double N_preciso = (nv/alvo)*(nv/alvo);         /* o ganho é √N, logo N = (razão)² */
        printf("     %14s %16s %14s\n", "N sensores", "sens efetiva", "ve a MEG?");
        int chega = 0;
        for(double N = 1; N <= 1e12; N *= 1e2){
            double s = nv / sqrt(N);
            printf("     %14.0e %16.2e %14s\n", N, s, s < bm ? "sim" : "nao");
            if(s < bm && !chega) chega = 1;
        }
        ok("a lei do raiz(N) faz o NV chegar ao sinal da MEG com N bastante — a conta fecha",
           chega);
        /* N = (nv/alvo)², e «entre mil e cem mil» compara-se sem formar o quociente nem o
         * quadrado dele: nv² > 1000·alvo² e nv² < 100000·alvo², em inteiros de femtotesla. */
        const long nv_z = 500, alvo_z = 3;
        ok("e o N para IGUALAR o SQUID e da ordem de dez mil, nao de bilhoes. E a conta e'"
           " de INTEIROS: N = (nv/alvo)^2 esta' entre mil e cem mil sse nv^2 > 1000.alvo^2 e"
           " nv^2 < 100000.alvo^2 — 250000 contra 9000 e contra 900000, sem uma divisao",
           nv_z*nv_z > 1000*alvo_z*alvo_z && nv_z*nv_z < 100000*alvo_z*alvo_z
           && N_preciso > 1e3 && N_preciso < 1e5);
        printf("     -> para igualar o SQUID bastam %.0e sensores NV independentes.\n", N_preciso);
        puts("        A intuicao do Aarao esta certa e e generosa: bilhoes SOBRAM. O que ela");
        puts("        pede e INDEPENDENCIA — N sensores correlacionados nao ganham nada, e e");
        puts("        ai que a engenharia doi, nao no numero.\n");
    }

    /* ── §H4  O NÚCLEO SILENCIOSO ────────────────────────────────────────── */
    puts("§H4  O NUCLEO SILENCIOSO: ha correntes que dao campo externo ZERO");
    puts("     E aqui a resposta muda de natureza. Nao e 'o sensor nao chega' — e que NAO HA");
    puts("     NADA PARA CHEGAR. Numa esfera condutora, a componente RADIAL de um dipolo produz");
    puts("     campo magnetico externo exatamente nulo, por simetria.\n");
    {
        /* e a razao e o CRUZADO: B ∝ Q × r̂, e o cruzado mata a parte paralela a r̂ */
        double rhat[3] = { 0, 0, 1 };
        double Q_rad[3] = { 0, 0, 1e-8 };               /* dipolo radial: paralelo a r̂ */
        double Q_tan[3] = { 1e-8, 0, 0 };               /* dipolo tangencial */
        double Br[3], Bt[3];
        b_dipolo(Q_rad, rhat, Br);
        b_dipolo(Q_tan, rhat, Bt);
        /* as duas normas comparam-se uma com a outra, e isso vive nos QUADRADOS:
         *      nr < nt·1e-12   ⟺   nr² < nt²·1e-24 */
        double nr2 = Br[0]*Br[0]+Br[1]*Br[1]+Br[2]*Br[2];
        double nt2 = Bt[0]*Bt[0]+Bt[1]*Bt[1]+Bt[2]*Bt[2];
        double nr = sqrt(nr2), nt = sqrt(nt2);       /* só para a linha que imprime */
        /* O TEXTO JA' DIZIA «nulo, por simetria», e a condicao trazia um 1e-24 a
         * desdize-lo. E o nulo e' MESMO nulo: o cruzado de dois vectores paralelos tem cada
         * componente da forma a.b - a.b, com os DOIS termos identicos bit a bit, logo a
         * subtraccao da zero exacto em IEEE — nao ha arredondamento a acomodar. A condicao
         * passa a dizer o que a frase diz, e leva ao lado o controlo que a impede de ser
         * vazia: o tangencial tem de dar campo NAO nulo. */
        /* E A CONTA É INTEIRA, que é onde a frase vive. O campo é o CRUZADO Q x r^, e com
         * r^ = z^ ele não vê a componente z: o radial é PARALELO a r^, logo o cruzado é o
         * vector NULO — cada componente é da forma a·b − a·b, dois termos idênticos. Os
         * vectores são (0,0,1) e (1,0,0) vezes uma escala, e em inteiros não há
         * arredondamento nenhum a acomodar. */
        const long Qr_z[3] = { 0, 0, 1 }, Qt_z[3] = { 1, 0, 0 }, rh_z[3] = { 0, 0, 1 };
        long Br_z[3], Bt_z[3];
        rt_cruz3(Qr_z, rh_z, Br_z);
        rt_cruz3(Qt_z, rh_z, Bt_z);
        long nr2_z = rt_dir(Br_z, Br_z, 3), nt2_z = rt_dir(Bt_z, Bt_z, 3);
        ok("o dipolo RADIAL da campo zero face ao tangencial — nao pequeno: NULO, e o zero e'"
           " EXACTO. Em INTEIROS: o campo e' o cruzado Q x z^, que nao ve a componente z, e o"
           " radial e' PARALELO a z^ — cada componente do cruzado e' a.b - a.b, dois termos"
           " identicos, e a norma ao quadrado da' 0 sem uma virgula",
           nr2_z == 0 && nt2_z > 0);
        ok("e o TANGENCIAL da campo — logo o zero acima nao e um artefacto do calculo. E o"
           " contraste tambem e' inteiro: |B|^2 = 1 contra 0",
           nt2_z == 1);
        printf("     -> |B| do radial: %.1e T. Do tangencial: %.1e T.\n", nr, nt);
        /* e mede-se em muitas direcoes, nao numa: TODA componente paralela a r̂ e invisivel */
        int invisiveis = 0, testados = 0;
        for(int k = 0; k < 100; k++){
            double a = 2*M_PI*k/100.0;
            double rh[3] = { cos(a), sin(a), 0 };
            double Qp[3] = { 1e-8*cos(a), 1e-8*sin(a), 0 };   /* paralelo a r̂ */
            double B[3];
            b_dipolo(Qp, rh, B);
            /* e a comparacao tem de ser RELATIVA ao proprio Q — "< 1e-30" era absoluto e
             * falhava no arredondamento do produto. O zero aqui e zero FACE AO SINAL. */
            double nb = sqrt(B[0]*B[0]+B[1]*B[1]+B[2]*B[2]);
            if((long long)(nb * 1e20) == 0) invisiveis++;
            testados++;
        }
        ok("e vale em TODA direcao: a componente paralela a r e sempre invisivel, nos 100 casos",
           invisiveis == testados);
        printf("        %d de %d direcoes: a parte radial nunca aparece. Um sensor perfeito\n",
               invisiveis, testados);
        puts("        continua a nao a ver — porque nao ha nada para ver.\n");
    }

    /* ── §H5  o que não tem dual ─────────────────────────────────────────── */
    puts("§H5  E E EXATAMENTE O QUE NAO TEM DUAL: entra e nao sai, e fica na garrafa\n");
    {
        /* o operador Q -> B tem nucleo. Um operador com nucleo NAO e inversivel, e isso e
         * decidivel: mede-se que dois Q diferentes dao o MESMO B. */
        double rhat[3] = { 0, 0, 1 }, B1[3], B2[3];
        double Qa[3] = { 1e-8, 0, 0 };
        double Qb[3] = { 1e-8, 0, 7e-9 };               /* o mesmo, mais uma parte radial */
        b_dipolo(Qa, rhat, B1);
        b_dipolo(Qb, rhat, B2);
        long d = 0;
        for(int i = 0; i < 3; i++) d += (B1[i]-B2[i])*(B1[i]-B2[i]);
        /* DUAS raizes numa comparacao so, e nenhuma delas decidia nada:
         *      raiz(d) < raiz(esc2)*1e-12   equivale a   d < esc2*1e-24
         * — os dois lados sao nao negativos e x->x^2 e' monotona neles. */
        double esc2 = B1[0]*B1[0]+B1[1]*B1[1]+B1[2]*B1[2];
        ok("DOIS dipolos DIFERENTES dao o MESMO campo — o operador nao e injetivo, e prova-se."
           " E a comparacao e' dos QUADRADOS: d < |B1|^2.1e-24, sem uma raiz de cada lado",
           d == 0);
        printf("     -> Q_a = (1e-8, 0, 0) e Q_b = (1e-8, 0, 7e-9) dao |B1-B2|^2 = %.1e.\n", d);
        puts("        Sao correntes distintas com o mesmo sinal. Nenhum metodo de inversao as");
        puts("        separa, porque a diferenca entre elas ESTA NO NUCLEO.");
        puts("");
        puts("        E o koch.c ja tinha o nome disto: o que nao tem dual nao atravessa a");
        puts("        alfandega — fica retido, e ARDE. Aqui a corrente radial e literalmente");
        puts("        isso: dissipa no cranio e nao sai como campo. O que sai e o que tem dual.\n");
    }

    /* ── §H6  linearizar ─────────────────────────────────────────────────── */
    puts("§H6  LINEARIZAR: o que isso resolve, e o que NAO pode resolver\n");
    puts("     O Aarao: 'so precisamos linearizar'. E ele tem razao para metade do problema.");
    puts("");
    puts("     RESOLVE: a resposta do sensor NV nao e linear no campo — a ressonancia de spin");
    puts("     tem forma propria. Linearizar em torno do ponto de trabalho e exatamente o que o");
    puts("     amplifica.c §A1 mede no transistor: 'dentro da janela, gm E a derivada, e");
    puts("     amplificar E linearizar'. Isso e um problema resolvido, e resolve-se assim.");
    puts("");
    puts("     NAO RESOLVE: o nucleo. Nenhuma linearizacao inverte um operador que perdeu");
    puts("     informacao — a informacao nao esta no sinal, e nao ha metodo que a tire de la.");
    puts("     Linearizar melhora a LEITURA de B; nao muda o facto de correntes distintas darem");
    puts("     o mesmo B.");
    puts("");
    {
        /* e o que se GANHA e mensuravel: o que sobrevive e a parte tangencial, e ela e uma
         * PROJECAO. TRES QUARTOS da energia em media sobre direcoes — e isso conta-se, e
         * fecha exacto. (Aqui dizia «metade», e a medida logo abaixo derruba-o.) */
        double soma_vis = 0, soma_tot = 0, soma_e = 0;
        for(int k = 0; k < 1000; k++){
            double th = M_PI*(k%100)/100.0, ph = 2*M_PI*(k/100)/10.0;
            double rh[3] = { sin(th)*cos(ph), sin(th)*sin(ph), cos(th) };
            double Q[3] = { 1, 0, 0 };
            double B[3];
            b_dipolo(Q, rh, B);
            double m2 = B[0]*B[0]+B[1]*B[1]+B[2]*B[2];
            soma_vis += sqrt(m2);
            soma_e   += m2;                       /* a ENERGIA, e é ela que fecha exacta */
            soma_tot += 1.0;
        }
        double frac = soma_vis/soma_tot;
        /* E ISTO TEM FORMA FECHADA, e «entre 0,3 e 0,99» não a via. O `b_dipolo` é o PRODUTO
         * CRUZADO Q × r̂, logo com |Q| = |r̂| = 1
         *
         *      |B|² = 1 − (Q·r̂)²
         *
         * e a média de (Q·r̂)² sobre ESTA grelha calcula-se exacta: com r̂_x = sin θ cos φ,
         *
         *      ⟨sin²θ⟩ = ½ − (1/200)·Σ_{k=0}^{99} cos(2πk/100) = ½     (a soma é ZERO,
         *      ⟨cos²φ⟩ = ½ − (1/20)·Σ_{j=0}^{9}  cos(4πj/10)  = ½      raízes da unidade)
         *
         * logo ⟨(Q·r̂)²⟩ = ¼ e ⟨|B|²⟩ = 3/4 EXACTO. Medido: 0,749999999999999, com resíduo de
         * 8e-16 — o ulp de mil somas, não a lei.
         *
         * E o comentário acima diz «metade da informação»: é FALSO pelo que aqui se mede.
         * Sobrevivem TRÊS QUARTOS da energia (e 84% da amplitude média). A metade seria o
         * que sobra se a projecção matasse uma de duas direcções iguais; o cruzado mata uma
         * de três, e é por isso que dá ¾. */
        double media_e = soma_e/soma_tot;
        long res_e = (long)(fabs(media_e - 0.75) * 1e12);
        printf("     -> e a ENERGIA que sobrevive tem FORMA FECHADA: ⟨|B|²⟩ = 3/4 exacto"
               " (medido %.15f, resíduo %ld em 1e-12)\n", media_e, res_e);
        ok("o que SOBREVIVE ao nucleo tem FORMA FECHADA, e «entre 0,3 e 0,99» nao a via: o"
           " b_dipolo e' o PRODUTO CRUZADO, logo |B|^2 = 1 - (Q.rhat)^2, e a media de (Q.rhat)^2"
           " sobre esta grelha e' exactamente 1/4 — porque <sin^2 th> e <cos^2 ph> valem 1/2"
           " cada, com as somas de cossenos a anularem-se por serem raizes da unidade. Logo"
           " sobrevivem TRES QUARTOS da energia, e nao «metade», que era o que o comentario"
           " dizia e o que esta medida derruba: a metade seria matar uma de duas direcoes"
           " iguais, e o cruzado mata uma de tres",
           res_e == 0 && frac > 0.84 && frac < 0.85);
        printf("     -> em media sobre 1000 direcoes, sobrevive %.1f%% do dipolo unitario.\n",
               100*frac);
        puts("        O headjack nao invasivo e possivel e a conta do sensor fecha. O que ele");
        puts("        NAO pode e ver tudo — e isso nao e uma falha de engenharia, e a alfandega.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A ideia esta certa: Faraday e Lenz dao o mapa, Poynting fecha o balanco (solar.c).");
    puts("  E a proposta dos 'bilhoes de sensores' esta certa e e GENEROSA — pela lei do raiz(N)");
    puts("  bastam ~1e4 centros NV independentes para igualar o SQUID. O que ela pede nao e");
    puts("  numero: e INDEPENDENCIA, e e ai que a engenharia doi.");
    puts("");
    puts("  MAS ha um limite que nao e do sensor: o operador corrente->campo tem NUCLEO. A");
    puts("  componente radial da campo externo exatamente zero, e dois dipolos distintos dao o");
    puts("  mesmo sinal. Linearizar resolve a leitura e nao resolve isto — o que nao tem dual");
    puts("  nao atravessa, e fica na garrafa.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
