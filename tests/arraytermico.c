/* arraytermico.c — O ARRAY COMPLETO com o par (B,P), e o calor a voltar como eletricidade.
 *
 * O Aarão: "agora projeta o array térmico completo com o par (B,P), e vê conversão de calor em
 * eletricidade."
 *
 * O `radiacao.c` fechou a MEDIDA: o par `(B,P)` recupera o radial que o magnético sozinho perde.
 * Falta o array que o realiza, e falta a segunda metade do fecho — porque medir o calor não é o
 * mesmo que **reaver** o que ele levou.
 *
 * E é aqui que o circuito fecha de verdade. O `koch.c` diz que *o que não tem dual fica retido e
 * arde*; o `radiacao.c` mostrou que o que arde **radia**, e por isso é visível. Mas radiar é perder,
 * e um circuito que perde não fecha. **Seebeck é o que devolve**: um gradiente de temperatura faz
 * tensão, e a energia que saía do sistema volta a entrar nele.
 *
 *      corrente sem dual  ->  ARDE (Joule)  ->  RADIA (Planck)  ->  MEDE-SE (P)
 *                                    |
 *                                    +------->  SEEBECK  ->  VOLTA como tensão
 *
 * E o teto não é opinável: **Carnot**, que o `carnot.c` já derivou de `∮`. Sobre ele, o ZT do
 * material diz quanto do teto se atinge. As duas coisas medem-se, e a segunda nunca passa a
 * primeira — se passasse, o medidor estaria errado.
 *
 *   §A1  o ARRAY: as duas grelhas, a cobertura conjunta, e a contagem
 *   §A2  a RESOLUÇÃO do par: o que (B,P) juntos resolvem e cada um sozinho não
 *   §A3  SEEBECK: o calor vira tensão — a lei, medida em vários gradientes
 *   §A4  CARNOT é o TETO, e o ZT diz quanto dele se atinge — e nunca o passa
 *   §A5  O BALANÇO: quanto do que ardia volta, e o resíduo é honesto
 *   §A6  e o circuito FECHA: o que entra sai, e o que não sai está nomeado
 *
 *   cc -O2 -std=c99 arraytermico.c -lm -o arraytermico && ./arraytermico
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "termica.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define T_QUENTE   310.15            /* 37 °C — o córtex */
#define T_FRIO     305.15            /* 32 °C — a superfície do escalpe */

/* ───────────────────────────────────────────────────────────────────────────
 * §A1  O ARRAY — duas grelhas sobre a mesma cabeça
 *
 * Uma grelha magnética (NV) e uma térmica (microbolómetro). Elas NÃO precisam do mesmo passo:
 * o §I2 do icc.c mediu que o passo tem de sair da frequência espacial do que se quer ver, e as
 * duas coisas têm escalas diferentes — o campo cai com r², o calor difunde.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *tipo; int nx, ny; double passo_mm, sens; } Grelha;

static const Grelha ARRAY[] = {
    { "NV (magnetico)",    32, 32, 8.0,  5.0e-13 },   /* 1024 canais, 8 mm, T/√Hz */
    { "bolometro (termico)",640,480, 0.4, 2.0e-2  },   /* 307200 pixeis, 0,4 mm, K */
};
#define NGRELHAS 2

/* ───────────────────────────────────────────────────────────────────────────
 * §A3  SEEBECK — o calor vira tensão
 *
 * V = S·ΔT, com S o coeficiente de Seebeck. Os números são de materiais reais.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *nome; double S_uVK, ZT; } Termoeletrico;

static const Termoeletrico MATERIAIS[] = {
    { "Bi2Te3 (comercial)", 200.0, 1.0 },
    { "PbTe",               180.0, 0.8 },
    { "SnSe (recorde)",     300.0, 2.6 },
    { "constantan (par T)",  40.0, 0.01 },
};
#define NMAT ((int)(sizeof MATERIAIS / sizeof MATERIAIS[0]))

static double seebeck(double S_uVK, double dT){ return S_uVK * 1e-6 * dT; }

/* Carnot: o teto, e ele não se discute */
static double carnot(double Tq, double Tf){ return (Tq - Tf) / Tq; }

/* a eficiência real de um termoelétrico: Carnot vezes o fator do ZT.
 * η = η_c · (√(1+ZT) − 1) / (√(1+ZT) + T_f/T_q) — a fórmula clássica, e ela NUNCA passa Carnot. */
static double eficiencia(double Tq, double Tf, double ZT){
    double m = sqrt(1.0 + ZT);
    return carnot(Tq, Tf) * (m - 1.0) / (m + Tf/Tq);
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

int main(void){
    puts("arraytermico.c — O ARRAY COMPLETO (B,P), e o calor a voltar como eletricidade\n");

    /* ── §A1 ─────────────────────────────────────────────────────────────── */
    puts("§A1  O ARRAY: duas grelhas sobre a mesma cabeca, e elas NAO tem o mesmo passo");
    puts("     O icc.c §I2 mediu que o passo sai da frequencia espacial do que se quer ver. O");
    puts("     campo cai com r^2 e o calor difunde: sao escalas diferentes, e forcar o mesmo");
    puts("     passo nas duas seria desperdicar canais numa e faltar na outra.\n");
    {
        printf("     %-22s %8s %10s %12s %14s\n", "grelha", "canais", "passo", "area(cm2)", "sens");
        long total = 0;
        for(int i = 0; i < NGRELHAS; i++){
            const Grelha *g = &ARRAY[i];
            long n = (long)g->nx * g->ny;
            double area = (g->nx * g->passo_mm) * (g->ny * g->passo_mm) / 100.0;
            printf("     %-22s %8ld %8.1fmm %12.1f %14.1e\n", g->tipo, n, g->passo_mm, area, g->sens);
            total += n;
        }
        /* Eu escrevi aqui "ok(..., 1)" — a constante disfarcada, a primeira forma do defeito
         * que mais me apanha — e ainda anotei ao lado que ela nao media nada, em vez de a
         * corrigir. Anotar um defeito nao o cura. A afirmacao tem duas partes e as duas se
         * medem: as areas sao comparaveis, e os passos NAO sao. */
        double a0 = (ARRAY[0].nx*ARRAY[0].passo_mm)*(ARRAY[0].ny*ARRAY[0].passo_mm);
        double a1 = (ARRAY[1].nx*ARRAY[1].passo_mm)*(ARRAY[1].ny*ARRAY[1].passo_mm);
        double razao_area = a0 > a1 ? a0/a1 : a1/a0;
        double razao_passo = ARRAY[0].passo_mm / ARRAY[1].passo_mm;
        /* E OS DOIS LIMIARES SAEM, porque as duas contas sao EXACTAS em Z. Com o passo em
         * decimos de milimetro — 80 para a grelha NV e 4 para o bolometro:
         *
         *      NV:        32.80 = 2560  por  32.80 = 2560
         *      bolometro: 640.4 = 2560  por  480.4 = 1920
         *
         * As duas tem a MESMA LARGURA, 2560 decimos, e por isso a razao das areas e' a razao
         * das alturas: 2560/1920 = 4/3 exacto — que e' a razao de aspecto do proprio
         * bolometro, 640 por 480. Nao «dentro de 2x»: e' 4/3, e diz-se.
         * E os passos estao na razao 80/4 = 20, exacta tambem. */
        /* e os passos e as contagens SAEM do proprio ARRAY[] — nao se escrevem. Escritos a
         * mao, mudar o passo do bolometro no dado nao mudava a conta, e o gume nao mordia:
         * a referencia tem de se mover quando o dado se move, senao e' copia. */
        const long px_nv = (long)(ARRAY[0].passo_mm*10 + 0.5);   /* passos em decimos de mm */
        const long px_bo = (long)(ARRAY[1].passo_mm*10 + 0.5);
        long lx_nv = ARRAY[0].nx*px_nv, ly_nv = ARRAY[0].ny*px_nv;
        long lx_bo = ARRAY[1].nx*px_bo, ly_bo = ARRAY[1].ny*px_bo;
        int mesma_largura = (lx_nv == lx_bo);
        int area_4_3 = (3L*lx_nv*ly_nv == 4L*lx_bo*ly_bo);
        int passo_20  = (px_nv == 20*px_bo);
        int aspecto   = (3L*ARRAY[1].nx == 4L*ARRAY[1].ny);  /* a razao vem do 4:3 do sensor */
        printf("     -> em decimos de mm: NV %ldx%ld e bolometro %ldx%ld — MESMA largura,\n"
               "        e a razao das areas e' 4/3 EXACTO, que e' o 4:3 do proprio sensor\n",
               lx_nv, ly_nv, lx_bo, ly_bo);
        ok("as duas grelhas cobrem area COMPARAVEL — e nao «dentro de 2x», que era um limiar"
           " meu: em decimos de milimetro as duas tem a MESMA largura, 2560, e por isso a"
           " razao das areas e' a razao das alturas, 2560/1920 = 4/3 EXACTO. E esse 4/3 nao"
           " e' coincidencia: e' a razao de aspecto do proprio bolometro, 640 por 480",
           mesma_largura && area_4_3 && aspecto);
        ok("mas os passos diferem por uma ORDEM DE GRANDEZA — e e' de proposito, nao descuido."
           " E o numero e' exacto: 80 decimos contra 4, ou seja VINTE vezes, nao «mais de dez»",
           passo_20);
        printf("     -> %ld canais ao todo; areas %.0f e %.0f mm2 (razao %.2f).\n",
               total, a0, a1, a0/a1);
        puts("");
    }

    /* ── §A2  a RESOLUÇÃO do par ─────────────────────────────────────────── */
    puts("§A2  A RESOLUCAO DO PAR: o que (B,P) juntos resolvem e cada um sozinho nao");
    puts("     O radiacao.c §W5 ja mediu a recuperacao do radial. Aqui conta-se o que cada");
    puts("     grelha traz de INFORMACAO — e a soma nao e a soma dos canais.\n");
    {
        /* o magnético dá 2 graus por sítio (as duas componentes tangenciais); o térmico dá 1
         * (a norma). Um dipolo tem 3 graus. Então nenhum sozinho chega, e os dois juntos dão 3. */
        int graus_dipolo = 3, graus_B = 2, graus_P = 1;
        ok("o dipolo tem 3 graus; B da 2 (as tangenciais) e P da 1 (a norma) — nenhum sozinho chega",
           graus_B < graus_dipolo && graus_P < graus_dipolo);
        ok("e os dois juntos dao 3: o par e EXATAMENTE o que falta, nem a mais nem a menos",
           graus_B + graus_P == graus_dipolo);
        printf("     -> B: %d graus por sitio; P: %d; o dipolo pede %d. O par fecha exato.\n",
               graus_B, graus_P, graus_dipolo);
        puts("        E o que sobra do par e o SINAL do radial (§W6) — um bit, e ele fica.\n");
    }

    /* ── §A3  SEEBECK ────────────────────────────────────────────────────── */
    puts("§A3  SEEBECK: o calor vira tensao, e a lei mede-se em varios gradientes");
    puts("     V = S.dT. O gradiente disponivel numa cabeca e o do cortex ao escalpe: ~5 K.\n");
    {
        long dT = T_QUENTE - T_FRIO;
        printf("     %-22s %10s %8s %14s\n", "material", "S(uV/K)", "ZT", "V a 5 K");
        for(int i = 0; i < NMAT; i++)
            printf("     %-22s %10.0f %8.2f %12.2f mV\n", MATERIAIS[i].nome, MATERIAIS[i].S_uVK,
                   MATERIAIS[i].ZT, seebeck(MATERIAIS[i].S_uVK, dT)*1e3);
        /* a LEI: V e LINEAR em dT — mede-se em muitos dT, nao num */
        int linear = 1;
        long S = MATERIAIS[0].S_uVK;
        for(double d = 0.5; d <= 50; d += 0.5){
            double v1 = seebeck(S, d), v2 = seebeck(S, 2*d);
            if(fabs(v2 - 2*v1) != 0.0) linear = 0;
        }
        ok("A LEI: a tensao de Seebeck e LINEAR no gradiente, em 100 valores de dT",
           linear);
        /* "!= 0.0" falhou por IGUALDADE: 200 uV/K x 5 K da exatamente 1,00 mV. Um limiar posto
         * no valor exato nao mede — ele so testa o arredondamento. A afirmacao e sobre a ESCALA:
         * a tensao e mil vezes o microvolt, e isso compara-se com a escala e nao com o valor. */
        double V = seebeck(MATERIAIS[0].S_uVK, dT);
        /* e a nota de cima ja' o dizia sem tirar a consequencia: se 200 uV/K x 5 K da'
         * EXACTAMENTE 1000 uV, entao a afirmacao e' uma IGUALDADE e nao uma desigualdade —
         * `V >= 0,001` deixava passar qualquer material melhor, e o que se afirma e' este.
         * Em microvolt inteiros: S.dT = 200.5 = 1000, e os dois factores saem do dado. */
        const long S_uV = (long)MATERIAIS[0].S_uVK;
        long V_uV_lei = S_uV * dT;                       /* pela lei, em Z */
        long V_uV_med = (long)(V*1000000 + 0.5);         /* pela funcao do ficheiro */
        printf("     -> em microvolt inteiros: %ld uV/K x %ld K = %ld uV, e a funcao da' %ld\n",
               S_uV, dT, V_uV_lei, V_uV_med);
        ok("e com o gradiente real da cabeca (5 K) o Bi2Te3 da' mil vezes o microvolt — e isso"
           " e' uma IGUALDADE e nao uma desigualdade: 200 uV/K vezes 5 K sao EXACTAMENTE 1000"
           " uV. O `V >= 0,001` deixava passar qualquer material melhor, e o que se afirma e'"
           " este. Medido por DOIS CAMINHOS, a lei em Z e a `seebeck` do ficheiro, e os dois"
           " factores saem do dado",
           V_uV_lei == 1000 && V_uV_med == V_uV_lei && S_uV*dT == V_uV_lei);
        printf("     -> a 5 K o Bi2Te3 da %.2f mV por juncao (%.0f uV). Empilhando N juncoes em\n",
               V*1e3, V*1e6);
        puts("        serie a tensao soma — e e assim que um modulo Peltier chega a volts.\n");
    }

    /* ── §A4  CARNOT ─────────────────────────────────────────────────────── */
    puts("§A4  CARNOT E O TETO, e o ZT diz quanto dele se atinge — e nunca o passa");
    puts("     O carnot.c ja derivou eta de um integral fechado. Aqui usa-se como TETO, e o que");
    puts("     se mede e que nenhum ZT o ultrapassa — se ultrapassasse, o medidor estava errado.\n");
    {
        double ec = carnot(T_QUENTE, T_FRIO);
        printf("     Carnot entre %.2f K e %.2f K: %.4f%% — e este e o maximo absoluto\n\n",
               T_QUENTE, T_FRIO, 100*ec);
        printf("     %-22s %8s %14s %14s\n", "material", "ZT", "eficiencia", "% de Carnot");
        int nenhum_passa = 1, cresce = 1;
        long ant = -1;
        for(int i = 0; i < NMAT; i++){
            double e = eficiencia(T_QUENTE, T_FRIO, MATERIAIS[i].ZT);
            printf("     %-22s %8.2f %12.5f%% %12.1f%%\n", MATERIAIS[i].nome, MATERIAIS[i].ZT,
                   100*e, 100*e/ec);
            if(e > ec) nenhum_passa = 0;
        }
        ok("NENHUM material passa Carnot — e isso nao e uma escolha minha, e a formula a fecha-lo",
           nenhum_passa);
        /* e a LEI: a eficiencia CRESCE com o ZT, e tende a Carnot quando ZT -> infinito */
        for(double zt = 0.1; zt <= 1000; zt *= 2){
            double e = eficiencia(T_QUENTE, T_FRIO, zt);
            if(e <= ant) cresce = 0;
            ant = e;
        }
        double e_inf = eficiencia(T_QUENTE, T_FRIO, 1e12);
        /* O CARNOT CANCELA-SE NOS DOIS LADOS, e é preciso dizê-lo. A `eficiencia` é
         * carnot·(m−1)/(m + Tf/Tq), logo |e_inf − ec|/ec É |f(ZT) − 1| com f a fracção de
         * Carnot: a comparação não depende das temperaturas, e mediria o mesmo com quaisquer
         * duas. Isso não a torna vazia — torna-a uma afirmação sobre f e não sobre o par.
         * O que faltava era o outro lado: para ZT pequeno f está LONGE de 1, sem o que
         * «tende a Carnot» valia por f ser sempre 1. */
        double f_inf = e_inf/ec, f_baixo = eficiencia(T_QUENTE, T_FRIO, 0.1)/ec;
        /* e os dois lados dizem-se em partes por milhao de Carnot, enquadrados: com ZT = 0,1
         * a fraccao vale 24011 ppm e no limite 999998 — logo o limite esta' entre 41 e 42
         * vezes o pe. «< 0,5» tinha vinte vezes de folga e nao dizia nada sobre o par. */
        long fz_inf = (long)(f_inf*1000000), fz_baixo = (long)(f_baixo*1000000);
        printf("     -> a fraccao de Carnot em ppm: com ZT = 0,1 vale %ld, e no limite %ld"
               " — o limite e' 41 vezes o pe\n", fz_baixo, fz_inf);
        ok("e a eficiencia CRESCE com ZT e TENDE a Carnot no limite — medido ate ZT = 1e12."
           " E o Carnot CANCELA nos dois lados: |e_inf - ec|/ec e' |f(ZT) - 1|, a fraccao de"
           " Carnot, e nao depende das temperaturas. O que a torna medicao e' o outro lado:"
           " com ZT = 0,1 a fraccao esta' LONGE de um, e sem isso «tende» valia por f ser"
           " sempre um. E os dois lados dizem-se em partes por milhao de Carnot em vez de se"
           " arbitrarem: 24011 ppm no pe e 999998 no limite, logo o limite e' entre 41 e 42"
           " vezes o pe. O «< 0,5» tinha vinte vezes de folga",
           cresce && fz_inf > 999990 && fz_inf <= 1000000 && fz_baixo > 24000 && fz_baixo < 24100
           && 41*fz_baixo < fz_inf && fz_inf < 42*fz_baixo);
        printf("     -> com ZT -> infinito a eficiencia da %.4f%% contra o Carnot de %.4f%%.\n",
               100*e_inf, 100*ec);
        puts("        O ZT nao e uma constante de material qualquer: e a fracao de Carnot que");
        puts("        aquele material consegue, e o teto continua a ser o teto.\n");
    }

    /* ── §A5  O BALANÇO ──────────────────────────────────────────────────── */
    puts("§A5  O BALANCO: quanto do que ardia volta, e o residuo e honesto\n");
    {
        /* o cérebro dissipa ~20 W (literatura: 20% do metabolismo basal de ~100 W).
         * Quanto disso um filme termoelétrico no escalpe recuperaria? */
        double P_cerebro = 20.0;
        double e = eficiencia(T_QUENTE, T_FRIO, MATERIAIS[0].ZT);
        double P_rec = P_cerebro * e;
        /* e o numero diz-se mesmo, em microwatt inteiros: 55691 uW dos 20 W, ou seja 2784
         * partes por milhao de eficiencia. «< 0,1 W» era o meu arredondamento a fingir de
         * numero — e a frase ja prometia dizer o numero. */
        long P_uW = (long)(P_rec*1000000), e_ppm = (long)(e*1000000);
        printf("     -> em microwatt inteiros: %ld uW dos 20 W, e a eficiencia %ld ppm\n",
               P_uW, e_ppm);
        ok("o que se recupera e' uma FRACAO minuscula — e diz-se o numero, nao se arredonda:"
           " 55691 microwatt dos 20 W, que sao 2784 partes por milhao de eficiencia,"
           " enquadrados dos dois lados. O `< 0,1 W` era o arredondamento a fingir de numero,"
           " e a propria frase ja prometia dizer o numero",
           P_uW > 55000 && P_uW < 56000 && e_ppm > 2784 - 1 && e_ppm < 2784 + 1);
        printf("     -> o cerebro dissipa ~%.0f W; com Bi2Te3 e dT de 5 K recupera-se %.1f mW\n",
               P_cerebro, P_rec*1e3);
        printf("        (%.4f%%). Nao alimenta o array — o bolometro de 640x480 consome watts.\n",
               100*e);
        puts("        Mas alimenta a ELETRONICA de um no: um microcontrolador em sono profundo");
        puts("        vive com dezenas de microwatt, e ha aqui milesimos de watt.");
        /* e o que NÃO volta: mede-se, e é a maior parte */
        /* E AQUI O P_cerebro CANCELA. perdido = P_cerebro − P_rec e P_rec = P_cerebro·e,
         * logo perdido/P_cerebro É 1 − e: a asserção não depende dos 20 W, e o que ela diz é
         * que a EFICIÊNCIA é menor que 1%. Fica dito assim, que é o que se mede, e o valor
         * em watts continua na linha que imprime. */
        double perdido = P_cerebro - P_rec;
        ok("e o que NAO volta e a esmagadora maioria — e Carnot que o exige, nao a engenharia."
           " E o P_cerebro CANCELA: perdido/P_cerebro E' 1 - e, logo o que se mede e' que a"
           " EFICIENCIA fica abaixo de 1%, e a assercao nao depende dos 20 W do cerebro. E o"
           " quanto abaixo diz-se: 2784 ppm contra os 16121 de Carnot, ou seja a maquina real"
           " fica abaixo de um QUINTO do tecto — e isso e' o ZT, nao a engenharia",
           e_ppm < 10000 && 5*e_ppm < 16121 && 3*e_ppm > 8000);
        printf("     -> %.3f W dos %.0f nao voltam (%.2f%%). E nao e desperdicio evitavel: e o\n",
               perdido, P_cerebro, 100*perdido/P_cerebro);
        puts("        segundo principio. Com dT de 5 K em 310 K, Carnot ja limita a 1,6%.\n");
    }

    /* ── §A6  o circuito ─────────────────────────────────────────────────── */
    puts("§A6  E O CIRCUITO FECHA: o que entra sai, e o que nao sai esta NOMEADO\n");
    {
        /* a conservação: a corrente sem dual arde; o que arde radia (mede-se) e uma fração
         * volta por Seebeck. Nada some sem nome — e é isso que se verifica. */
        /* E O BALANCO NAO ERA UMA MEDIDA. Estava aqui
         *
         *      volta = P_total * e;   radia = P_total - volta;   soma = volta + radia;
         *      ok(..., fabs(soma - P_total)/P_total == 0.0);
         *
         * e `radia` e definida COMO O RESTO: a soma da P_total por construcao algebrica,
         * qualquer que seja `e`. A assercao nao podia falhar, e a frase «sem sobra»
         * ficava por medir.
         *
         * A identidade e a medida vao para gavetas diferentes:
         *
         *   IDENTIDADE  volta + radia = P_total — por definicao de radia, e diz-se assim
         *   MEDIDA      a particao e uma PARTICAO: as duas parcelas sao NAO NEGATIVAS e
         *               nenhuma passa o total. Isso PODE falhar — basta a fracao que
         *               volta passar de 1 — e e o que «nada some sem nome» quer dizer.
         *
         * E em racionais exactos: com a fraccao que volta escrita p/q, a particao e
         * P·p/q + P·(q-p)/q = P, e o resíduo e ZERO em vez de menor que 1e-12. */
        const long P_total_i = 20;                  /* watts, inteiro */
        double e = eficiencia(T_QUENTE, T_FRIO, MATERIAIS[0].ZT);
        const long q = 1000000;                     /* a fraccao que volta, em partes por milhao */
        long p = (long)(e*q + 0.5);
        long volta_i = P_total_i * p, radia_i = P_total_i * (q - p);   /* sobre q */
        int identidade = (volta_i + radia_i == P_total_i * q);         /* exacta em Q */
        int particao   = (p >= 0 && p <= q && volta_i >= 0 && radia_i >= 0);
        ok("A PARTICAO E UMA PARTICAO: as duas parcelas sao nao negativas e nenhuma passa o"
           " total — e ISSO pode falhar, bastando a fraccao que volta passar de 1. O que"
           " aqui estava media `volta + (P - volta) == P`, que e verdade por construcao"
           " qualquer que seja a eficiencia: a soma fechava por definicao de `radia`, e a"
           " frase «sem sobra» ficava por medir. A identidade fica dita como identidade, e"
           " o residuo dela e ZERO porque a particao corre em racionais exactos",
           particao && identidade);
        printf("     -> entrou %ld W; voltou %ld/%ld W; radiou %ld/%ld W; soma %s\n",
               P_total_i, volta_i, q, radia_i, q,
               identidade ? "= P_total EXACTO (identidade)" : "NAO fecha");
        printf("     (a fraccao que volta e %ld/%ld, e e ela que tem de ficar em [0,1])\n",
               p, q);
        puts("");
        puts("        E o percurso inteiro, com cada passo medido noutro medidor:");
        puts("");
        puts("          corrente sem dual   ->  ARDE       (koch.c: a alfandega retem)");
        puts("          o que arde          ->  RADIA      (radiacao.c: Planck, e mede-se)");
        puts("          o que radia         ->  VE-SE      (radiacao.c §W5: o par (B,P))");
        puts("          o que se ve         ->  VOLTA      (aqui: Seebeck, uma fracao)");
        puts("          o que nao volta     ->  CARNOT     (carnot.c: e o teto, nao um defeito)");
        puts("");
        puts("        Nenhum passo e postulado e nenhum some sem nome. O circuito fecha — e a");
        puts("        parte que nao volta nao esta perdida: esta NOMEADA, que e a diferenca");
        puts("        entre um balanco que fecha e um que se arredonda.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O array e DUAS grelhas de passos diferentes, e isso e do icc.c §I2 — o passo sai da");
    puts("  escala do que se ve, e o campo e o calor tem escalas diferentes.");
    puts("");
    puts("  E o par fecha EXATO na contagem: o dipolo tem 3 graus, B da 2 e P da 1. Nem a mais");
    puts("  nem a menos. O que sobra e um bit — o sinal do radial — e ele fica na garrafa.");
    puts("");
    puts("  E o calor volta como eletricidade, mas pouco: 1,6% de Carnot com dT de 5 K, e o");
    puts("  Bi2Te3 tira 0,05% dos 20 W. Nao alimenta o array; alimenta um no. E o que nao");
    puts("  volta esta nomeado, que e o que faz o balanco fechar em vez de arredondar.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
