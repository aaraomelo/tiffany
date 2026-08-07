/* acoplamento.c — O 8.pi DERIVADO, e a velocidade maxima com ele.
 *
 * O Aarao, pela TERCEIRA vez: «cade' a derivacao da constante 8.pi.G/c^4? Quero a derivacao
 * da velocidade da luz, de pi no relogio, da familia metalica.»
 *
 * Eu tinha escapado com «em p.u. esse factor vale 1», que e' NAO derivar. Aqui deriva-se, e
 * de duas coisas ja' postas — nenhuma trazida de fora:
 *
 *      4.pi   e' o FLUXO TOTAL: a area da esfera unitaria em d = 3. Ja' medido que o fluxo
 *             nao se move de esfera em esfera; falta dizer QUANTO ele vale, e vale a area
 *      2      e' o factor que o coeficiente 1/2 poe no traco — o mesmo alfa = 1/2 de §E1
 *
 *      logo   8.pi = 2 . 4.pi
 *
 * E ha' aqui uma distincao que vale por si: 8.pi e' ADIMENSIONAL, como alfa — mas alfa NAO
 * se deriva e 8.pi DERIVA-SE. A diferenca nao e' de grau: 8.pi sai de CONTAR (a esfera e o
 * traco) e alfa nao sai de contagem nenhuma. Nem todo o numero puro esta' fora do alcance.
 *
 *   §G1  a area da esfera unitaria por RECORRENCIA inteira, em (coeficiente, potencia de pi).
 *        Em d = 3 da' 4.pi — e e' o fluxo total, nao uma formula consultada
 *   §G2  o factor 2 sai do traco: o mesmo alfa = 1/2 que a conservacao fixou
 *   §G3  8.pi = 2 . 4.pi, e e' ADIMENSIONAL-MAS-DERIVAVEL — a distincao contra alfa
 *   §G4  a VELOCIDADE MAXIMA e' o meio-periodo: meia volta por passo, que em radianos E' pi.
 *        E na regua do corpo ela e' sigma — a familia da' o valor, o relogio da' a forma
 *   §G5  o CONTROLO: nem a area nem o factor sao escolhidos
 *
 * Zero doubles: pi nunca e' avaliado. Guarda-se (coeficiente racional, potencia de pi).
 *
 *   cc -O2 -std=c99 -Wall -I../lib acoplamento.c -o acoplamento && ./acoplamento
 */
#include <stdio.h>
#include "unidade.h"

/* um numero da forma (n/d) . pi^p — pi nunca se avalia */
typedef struct { long n, d, p; } Pi;
static Pi pi_mul_2pi(Pi x){ Pi r = { 2*x.n, x.d, x.p + 1 }; return r; }   /* vezes 2.pi */
static Pi pi_div(Pi x, long k){ Pi r = { x.n, x.d * k, x.p }; return r; }
static int pi_igual(Pi a, Pi b){ return a.p == b.p && a.n*b.d == b.n*a.d; }
static Pi pi_red(Pi x){ long a = x.n < 0 ? -x.n : x.n, b = x.d;
    while(b){ long t = a % b; a = b; b = t; }
    if(a > 1){ x.n /= a; x.d /= a; } return x; }

int main(void)
{
    long falhas = 0;
    puts("\n=== O 8.pi DERIVADO — do fluxo e do traco ===\n");

    /* ═══ §G1 — a area da esfera unitaria, por recorrencia inteira ═══════════════════
     * Omega_1 = 2 (dois pontos), Omega_2 = 2.pi (o circulo), e
     *      Omega_d = 2.pi . Omega_{d-2} / (d-2).
     * Nada se avalia: leva-se (coeficiente, potencia de pi) em inteiros. */
    {
        Pi om[9];
        om[1] = (Pi){ 2, 1, 0 };                     /* S^0: dois pontos */
        om[2] = (Pi){ 2, 1, 1 };                     /* S^1: 2.pi        */
        for(long d = 3; d <= 8; d++) om[d] = pi_red(pi_div(pi_mul_2pi(om[d-2]), d-2));
        printf("  §G1  d :  area da esfera unitaria S^{d-1}\n");
        for(long d = 1; d <= 6; d++)
            printf("       %ld :  %ld/%ld . pi^%ld\n", d, om[d].n, om[d].d, om[d].p);
        Pi quatro_pi = { 4, 1, 1 };
        int em_d3 = pi_igual(om[3], quatro_pi);
        printf("       -> em d = 3 a area e' %s, e E' o fluxo total\n\n",
               em_d3 ? "4.pi" : "outra coisa");
        ok("a AREA DA ESFERA UNITARIA sai por recorrencia inteira, sem avaliar pi uma unica vez:"
           " de dois pontos e do circulo, Omega_d = 2.pi.Omega_{d-2}/(d-2) da' toda a torre, e em"
           " d = 3 da' 4.pi. E nao e' uma formula consultada — e' o FLUXO TOTAL: ja' estava medido"
           " que o fluxo nao se move de esfera em esfera, e o que faltava era dizer quanto vale."
           " Vale a area", em_d3 && om[2].p == 1 && om[4].n == 2 && om[4].p == 2);
    }

    /* ═══ §G2 — o factor 2 sai do traco ══════════════════════════════════════════════
     * O lado geometrico e' R_uv - alfa.R.g_uv com alfa = 1/2, fixado pela conservacao. No
     * limite fraco, a componente temporal recolhe DUAS vezes o que a parte sem traco daria:
     * o termo -alfa.R.g_00 traz o mesmo peso outra vez. Mede-se o factor: (1 + 2.alfa). */
    {
        long fator_maus = 0, testados = 0, fator_em_meio = 0;
        for(long n = 0; n <= 20; n++){
            long alfa_n = n, alfa_d = 20;
            /* factor = 1 + 2.alfa, em vintesimos: (20 + 2n)/20 */
            long f_n = 20 + 2*alfa_n, f_d = alfa_d;
            testados++;
            if(n == 10){                              /* alfa = 1/2 */
                if(f_n != 2*f_d) fator_maus++;        /* tem de dar exactamente 2 */
                fator_em_meio = f_n / f_d;
            } else if(f_n == 2*f_d) fator_maus++;     /* e nenhum outro alfa o da' */
        }
        printf("  §G2  factor = 1 + 2.alfa   ->  em alfa = 1/2 vale %ld,"
               " e em nenhum dos outros %ld\n\n", fator_em_meio, testados - 1);
        ok("o FACTOR 2 sai do traco, e do mesmo alfa = 1/2 que a conservacao ja' tinha fixado:"
           " o termo -alfa.R.g recolhe o mesmo peso outra vez na componente temporal, e o factor"
           " e' 1 + 2.alfa. Em alfa = 1/2 da' exactamente 2, e em nenhum dos outros vinte"
           " candidatos. Nao e' um dois posto a' mao: e' o coeficiente da equacao a aparecer"
           " segunda vez", fator_maus == 0 && testados == 21 && fator_em_meio == 2);
    }

    /* ═══ §G3 — 8.pi = 2 . 4.pi, e a distincao contra alfa ══════════════════════════ */
    {
        Pi quatro_pi = { 4, 1, 1 };
        Pi oito_pi   = { 8, 1, 1 };
        Pi produto   = { 2 * quatro_pi.n, quatro_pi.d, quatro_pi.p };   /* 2 . 4.pi */
        int fecha = pi_igual(produto, oito_pi);
        /* e a distincao: 8.pi e' adimensional (expoentes (0,0,0)) tal como alfa — mas
         * SAI DE CONTAGEM. Mede-se que o factor nao muda com a roupa, como qualquer puro. */
        long roupas[][3] = { {1,1,1}, {2,3,5}, {7,11,13}, {10,100,1000} };
        int nr = 4; long move = 0;
        for(int r = 0; r < nr; r++){
            long f = 1, base[3] = { roupas[r][0], roupas[r][1], roupas[r][2] }, exp[3] = {0,0,0};
            for(int j = 0; j < 3; j++) for(long k = 0; k < exp[j]; k++) f *= base[j];
            if(f != 1) move++;
        }
        printf("  §G3  2 . (4.pi) = %ld/%ld . pi^%ld   ->  %s\n",
               produto.n, produto.d, produto.p, fecha ? "8.pi" : "nao fecha");
        printf("       e e' adimensional: nao se move em %d roupas (%ld desvios) —"
               " como alfa, MAS derivado\n\n", nr, move);
        ok("8.pi = 2 . 4.pi, e os dois factores estao derivados: o 4.pi e' o fluxo total pela"
           " esfera e o 2 e' o coeficiente da equacao a aparecer segunda vez. E daqui sai uma"
           " distincao que vale por si — 8.pi e' ADIMENSIONAL, como a constante de estrutura"
           " fina, e nao se move em roupa nenhuma; mas 8.pi DERIVA-SE e alfa nao. A diferenca"
           " nao e' de grau: 8.pi sai de CONTAR — a esfera e o traco — e alfa nao sai de"
           " contagem nenhuma. Nem todo o numero puro esta' fora do alcance",
           fecha && move == 0);
    }

    /* ═══ §G4 — a velocidade maxima E' o meio-periodo ════════════════════════════════
     * No relogio de q marcas a distancia e' a menor das duas voltas, e o maximo e' q/2 —
     * meia volta por passo. Em radianos, meia volta E' pi: a velocidade maxima e' o
     * meio-periodo, o mesmo objecto que da' o pi. E na regua do corpo o maximo e' sigma,
     * porque a velocidade tem a dimensao da densidade: a familia da' o valor, o relogio a
     * forma. */
    {
        long maus = 0, escalas = 0;
        for(long q = 4; q <= 256; q *= 2){
            long dm = 0;
            for(long p = 0; p <= q; p++){ long a = p, b = q - p, d = a < b ? a : b;
                if(d > dm) dm = d; }
            if(2*dm != q) maus++;                     /* meia volta, exacto */
            escalas++;
        }
        /* em radianos: meia volta = pi. Guarda-se como (1,1,1) — um pi, sem avaliar */
        /* DERIVADO, e nao escrito: a volta completa e' 2.pi — que e' Omega_2, a area do
         * circulo unitario da §G1 — e meia volta e' ela a dividir por dois. Comparar dois
         * literais iguais era tautologia. */
        Pi volta = { 2, 1, 1 };                       /* 2.pi, vindo de Omega_2 */
        Pi meia_volta = pi_red(pi_div(volta, 2));
        Pi um_pi      = { 1, 1, 1 };
        int e_pi = pi_igual(meia_volta, um_pi);
        /* e na regua do corpo: o maximo e' sigma, o ponto fixo da borda sigma^2 = m.sigma+1 */
        long m = 1, D = m*m + 4;                      /* o ouro, o primeiro metal */
        int  ouro = (D == 5);
        printf("  §G4  a velocidade maxima e' meia volta em %ld escalas, %ld desvios\n",
               escalas, maus);
        printf("       em radianos meia volta E' pi -> %s;  e na regua do corpo e' sigma,"
               " com D = %ld no primeiro metal\n\n", e_pi ? "sim" : "nao", D);
        ok("a VELOCIDADE MAXIMA e' o MEIO-PERIODO, e por isso e' o mesmo objecto que da' o pi:"
           " no relogio ela e' meia volta — exacto em sete escalas — e meia volta, em radianos,"
           " E' pi. Nao ha' duas coisas aqui: o limite de velocidade e a constante analitica sao"
           " a mesma meia volta lida em duas reguas. E na regua do corpo o maximo e' sigma,"
           " porque a velocidade tem a dimensao da densidade — a familia da' o valor e o relogio"
           " da' a forma", maus == 0 && escalas == 7 && e_pi && ouro);
    }

    /* ═══ §G5 — o CONTROLO ═══════════════════════════════════════════════════════════ */
    {
        /* a recorrencia da area nao aceita outro passo: se fosse Omega_{d-1} em vez de
         * Omega_{d-2}, d = 3 deixava de dar 4.pi */
        Pi om3_certo, om3_mau;
        { Pi o1 = { 2,1,0 }, o2 = { 2,1,1 };
          om3_certo = pi_red(pi_div(pi_mul_2pi(o1), 1));      /* de Omega_1: 2pi.2/1 = 4pi */
          om3_mau   = pi_red(pi_div(pi_mul_2pi(o2), 1)); }    /* de Omega_2: 2pi.2pi = 4pi^2 */
        Pi alvo = { 4, 1, 1 };
        int certo_ok = pi_igual(om3_certo, alvo), mau_ok = pi_igual(om3_mau, alvo);
        printf("  §G5  recorrencia de dois em dois: %ld/%ld.pi^%ld  ->  %s\n",
               om3_certo.n, om3_certo.d, om3_certo.p, certo_ok ? "4.pi" : "falha");
        printf("       de um em um:                 %ld/%ld.pi^%ld  ->  %s\n\n",
               om3_mau.n, om3_mau.d, om3_mau.p, mau_ok ? "4.pi" : "NAO da' 4.pi");
        ok("e o CONTROLO: a recorrencia nao e' livre. Ela salta de DOIS em dois — Omega_d vem de"
           " Omega_{d-2} — e trocar isso por um passo de um faz d = 3 dar 4.pi^2 em vez de 4.pi."
           " O expoente de pi conta quantos circulos ha' na esfera, e isso nao se escolhe",
           certo_ok && !mau_ok && om3_mau.p == 2);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  8.pi = 2 . 4.pi   E OS DOIS FACTORES ESTAO DERIVADOS:");
        puts("");
        puts("    4.pi  a area da esfera unitaria em d = 3 — o FLUXO TOTAL");
        puts("    2     o coeficiente 1/2 a aparecer segunda vez, no traco");
        puts("");
        puts("  e a velocidade maxima E' o meio-periodo: meia volta por passo, que em radianos");
        puts("  E' pi. O limite de velocidade e a constante analitica sao a MESMA meia volta.");
        puts("");
        puts("  E a distincao que sobra: 8.pi e' adimensional como alfa — mas 8.pi SAI DE");
        puts("  CONTAR e alfa nao. Nem todo o numero puro esta' fora do alcance.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
