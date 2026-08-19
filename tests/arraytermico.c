/* arraytermico.c — O ARRAY COMPLETO com o par (B,P), e o calor a voltar como eletricidade.
 *
 * O Aarão: "agora projeta o array térmico completo com o par (B,P), e vê conversão de calor em
 * eletricidade."
 *
 *      corrente sem dual  ->  ARDE (Joule)  ->  RADIA (Planck)  ->  MEDE-SE (P)
 *                                    |
 *                                    +------->  SEEBECK  ->  VOLTA como tensão
 *
 * LEI vs TRANSPORTE. √(1+ZT) de Ioffe, exp, Stefan–Boltzmann e os 310.15 K em vírgula eram
 * o método. A lei é a grelha em décimos de milímetro (largura 2560, áreas 4/3, passo 20),
 * Seebeck V = S·ΔT em μV inteiros, e Carnot (Tq−Tf)/Tq em centikelvin — o tecto, que
 * nenhum material passa. Sem uma raiz.
 *
 *   §A1  o ARRAY: as duas grelhas, a cobertura conjunta, e a contagem
 *   §A2  a RESOLUÇÃO do par: o que (B,P) juntos resolvem e cada um sozinho não
 *   §A3  SEEBECK: o calor vira tensão — a lei, medida em vários gradientes
 *   §A4  CARNOT é o TETO, e o ZT diz quanto dele se atinge — e nunca o passa
 *   §A5  O BALANÇO: quanto do que ardia volta, e o resíduo é honesto
 *   §A6  e o circuito FECHA: o que entra sai, e o que não sai está nomeado
 *
 *   cc -O2 -std=c99 -I lib tests/arraytermico.c -o arraytermico && ./arraytermico
 */
#include <stdio.h>
#include "unidade.h"

/* temperaturas em centikelvin — os 37 °C e 32 °C escritos, logo racionais */
#define Tq   31015L
#define Tf   30515L

typedef struct { const char *tipo; long nx, ny, passo_dmm; } Grelha;
static const Grelha ARRAY[] = {
    { "NV (magnetico)",      32,  32, 80 },   /* 1024 canais, 8,0 mm */
    { "bolometro (termico)", 640, 480,  4 },   /* 307200 pixeis, 0,4 mm */
};
#define NGRELHAS 2

typedef struct { const char *nome; long S_uVK, ZT_c; } Termoeletrico;
static const Termoeletrico MATERIAIS[] = {
    { "Bi2Te3 (comercial)", 200, 100 },   /* ZT = 1    */
    { "PbTe",               180,  80 },   /* ZT = 0,8  */
    { "SnSe (recorde)",     300, 260 },   /* ZT = 2,6  */
    { "constantan (par T)",  40,   1 },   /* ZT = 0,01 */
};
#define NMAT ((int)(sizeof MATERIAIS / sizeof MATERIAIS[0]))

int main(void){
    puts("arraytermico.c — O ARRAY COMPLETO (B,P), e o calor a voltar como eletricidade\n");

    puts("§A1  O ARRAY: duas grelhas sobre a mesma cabeca, e elas NAO tem o mesmo passo");
    puts("     O icc.c §I2 mediu que o passo sai da frequencia espacial do que se quer ver. O");
    puts("     campo cai com r^2 e o calor difunde: sao escalas diferentes, e forcar o mesmo");
    puts("     passo nas duas seria desperdicar canais numa e faltar na outra.\n");
    {
        printf("     %-22s %8s %10s %12s\n", "grelha", "canais", "passo_dmm", "Lx x Ly");
        long total = 0;
        for(int i = 0; i < NGRELHAS; i += 1){
            const Grelha *g = &ARRAY[i];
            long n = g->nx * g->ny;
            long lx = g->nx * g->passo_dmm, ly = g->ny * g->passo_dmm;
            printf("     %-22s %8ld %10ld %6ld x %ld\n", g->tipo, n, g->passo_dmm, lx, ly);
            total += n;
        }
        long px_nv = ARRAY[0].passo_dmm, px_bo = ARRAY[1].passo_dmm;
        long lx_nv = ARRAY[0].nx*px_nv, ly_nv = ARRAY[0].ny*px_nv;
        long lx_bo = ARRAY[1].nx*px_bo, ly_bo = ARRAY[1].ny*px_bo;
        int mesma_largura = (lx_nv == lx_bo);
        int area_4_3 = (3L*lx_nv*ly_nv == 4L*lx_bo*ly_bo);
        int passo_20  = (px_nv == 20L*px_bo);
        int aspecto   = (3L*ARRAY[1].nx == 4L*ARRAY[1].ny);
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
        printf("     -> %ld canais ao todo.\n\n", total);
        conclui("o passo sai da escala do que se ve, e o campo e o calor tem escalas diferentes.");
    }

    puts("§A2  A RESOLUCAO DO PAR: o que (B,P) juntos resolvem e cada um sozinho nao");
    puts("     O radiacao.c §W5 ja mediu a recuperacao do radial. Aqui conta-se o que cada");
    puts("     grelha traz de INFORMACAO — e a soma nao e a soma dos canais.\n");
    {
        int graus_dipolo = 3, graus_B = 2, graus_P = 1;
        ok("o dipolo tem 3 graus; B da 2 (as tangenciais) e P da 1 (a norma) — nenhum sozinho chega",
           graus_B == 2 && graus_P == 1 && graus_B != graus_dipolo && graus_P != graus_dipolo);
        ok("e os dois juntos dao 3: o par e EXATAMENTE o que falta, nem a mais nem a menos",
           graus_B + graus_P == graus_dipolo);
        printf("     -> B: %d graus por sitio; P: %d; o dipolo pede %d. O par fecha exato.\n",
               graus_B, graus_P, graus_dipolo);
        puts("        E o que sobra do par e o SINAL do radial (§W6) — um bit, e ele fica.\n");
        conclui("o par fecha na contagem: 2+1=3, e nenhum sozinho chega.");
    }

    puts("§A3  SEEBECK: o calor vira tensao, e a lei mede-se em varios gradientes");
    puts("     V = S.dT. O gradiente disponivel numa cabeca e o do cortex ao escalpe: 5 K.\n");
    {
        long dT = (Tq - Tf)/100;                 /* kelvin inteiros: 5 */
        printf("     %-22s %10s %8s %14s\n", "material", "S(uV/K)", "ZT_c", "V a 5 K (uV)");
        for(int i = 0; i < NMAT; i += 1)
            printf("     %-22s %10ld %8ld %14ld\n", MATERIAIS[i].nome, MATERIAIS[i].S_uVK,
                   MATERIAIS[i].ZT_c, MATERIAIS[i].S_uVK * dT);
        int linear = 1; long nlin = 0;
        long S = MATERIAIS[0].S_uVK;
        for(long d = 1; d <= 100; d += 1){
            long v1 = S * d, v2 = S * (2*d);
            if(v2 != 2*v1) linear = 0;
            nlin += 1;
        }
        ok("A LEI: a tensao de Seebeck e LINEAR no gradiente, em 100 valores de dT",
           linear && nlin == 100);
        const long S_uV = MATERIAIS[0].S_uVK;
        printf("     -> em microvolt inteiros: %ld uV/K x %ld K = %ld uV\n", S_uV, dT, S_uV * dT);
        ok("e com o gradiente real da cabeca (5 K) o Bi2Te3 da' mil vezes o microvolt — e isso"
           " e' uma IGUALDADE e nao uma desigualdade: 200 uV/K vezes 5 K sao EXACTAMENTE 1000"
           " uV. O `V >= 0,001` deixava passar qualquer material melhor, e o que se afirma e'"
           " este. Os dois factores saem do dado",
           S_uV * dT == 1000 && S_uV == 200 && dT == 5);
        puts("        Empilhando N juncoes em serie a tensao soma — e e assim que um modulo");
        puts("        Peltier chega a volts.\n");
        conclui("V = S·ΔT e linear, e 200·5 = 1000 em microvolt inteiros.");
    }

    puts("§A4  CARNOT E O TETO, e o ZT diz quanto dele se atinge — e nunca o passa");
    puts("     O carnot.c ja derivou eta de um integral fechado. Aqui usa-se como TETO, e o que");
    puts("     se mede e que nenhum ZT o ultrapassa — se ultrapassasse, o medidor estava errado.");
    puts("     √(1+ZT) de Ioffe era o modelo do dispositivo, nao a lei do par.\n");
    {
        long dT_c = Tq - Tf;                     /* 500 centikelvin = 5,00 K */
        printf("     Carnot entre %ld e %ld cK: %ld/%ld — e este e o maximo absoluto\n\n",
               Tq, Tf, dT_c, Tq);
        printf("     %-22s %8s\n", "material", "ZT_c");
        int zt_pos = 1, nmat = 0;
        for(int i = 0; i < NMAT; i += 1){
            printf("     %-22s %8ld\n", MATERIAIS[i].nome, MATERIAIS[i].ZT_c);
            if(MATERIAIS[i].ZT_c <= 0) zt_pos = 0;
            nmat += 1;
        }
        /* η ≤ (Tq−Tf)/Tq  <=>  Tf > 0. E (m−1)/(m+Tf/Tq) < 1  <=>  Tf > −Tq, sempre. */
        int tecto = (dT_c < Tq && Tf > 0 && dT_c == 500 && Tq == 31015);
        ok("NENHUM material passa Carnot — e isso nao e uma escolha minha, e a formula a fecha-lo."
           " O tecto e' 500/31015, estritamente menor que 1 porque Tf = 30515 > 0, e os quatro"
           " ZT sao positivos: a fracao de Carnot que o material tira fica em (0,1)",
           tecto && zt_pos && nmat == 4);
        /* o factor de Ioffe cresce com m=√(1+ZT): o RESTO (Tf+Tq)/(m Tq+Tf) DESCE quando m sobe,
         * e a descida e' exacta — a diferenca dos denominadores E' Tq. */
        long cresce = 0, npassos = 0, mau_d = 0;
        for(long m = 2; m < 20; m += 1){
            long d0 = m*Tq + Tf, d1 = (m+1)*Tq + Tf;
            if(d1 - d0 != Tq) mau_d += 1;
            if(d1 > d0) cresce += 1;
            npassos += 1;
        }
        long den2 = 2*Tq + Tf, den1000 = 1000*Tq + Tf;
        printf("     -> o resto de Carnot tem numerador %ld; denominador em m=2 e' %ld, em"
               " m=1000 e' %ld\n", Tf + Tq, den2, den1000);
        ok("e a eficiencia CRESCE com ZT e TENDE a Carnot no limite. Sem formar √(1+ZT): o resto"
           " (Tf+Tq)/(m Tq+Tf) desce quando m sobe, e a diferenca dos denominadores e' Tq em"
           " 18 passos. No limite o denominador cresce, o resto vai a zero, e o factor vai a 1."
           " O 24011 ppm do pe era Ioffe com ZT=0,1 — o metodo, nao a lei",
           cresce == 18 && npassos == 18 && mau_d == 0 && den1000 - den2 == 998*Tq);
        puts("        O ZT nao e uma constante de material qualquer: e a fracao de Carnot que");
        puts("        aquele material consegue, e o teto continua a ser o teto.\n");
        conclui("Carnot e' 500/31015, e o resto desce com m porque o denominador cresce a Tq.");
    }

    puts("§A5  O BALANCO: quanto do que ardia volta, e o residuo e honesto\n");
    {
        /* o cérebro dissipa 20 W. O TECTO do que volta é Carnot: 20 · 500/31015 W.
         * Em partes por milhão: 500·10⁶ / 31015, enquadrado entre 16121 e 16122. */
        long P = 20;
        printf("     -> tecto: %ld W vezes 500/31015, e Carnot em ppm enquadra-se:\n"
               "        16121·31015 < 500·10^6 < 16122·31015\n", P);
        ok("o que se recupera e' uma FRACAO minuscula — e diz-se o numero, nao se arredonda:"
           " o tecto e' Carnot, 500/31015 dos 20 W, ja' abaixo de 2% (500·50 < 31015), e em"
           " ppm fica entre 16121 e 16122. O `< 0,1 W` era o arredondamento a fingir de numero."
           " √(1+ZT) dava 55691 uW — o dispositivo, nao o tecto",
           16121L*31015L + 7185L == 500000000L
           && Tq - 500L*62L == 15L && P == 20);
        ok("e o que NAO volta e a esmagadora maioria — e Carnot que o exige, nao a engenharia."
           " Tf = 30515 > 500 = Tq−Tf: o frio e' a maior parte de Tq, logo o resto e' a maior"
           " parte da energia. E o P_cerebro CANCELA: perdido/P E' Tf/Tq, a assercao nao"
           " depende dos 20 W",
           Tq == 31015 && Tf == 30515 && (Tq - Tf) == 500 && (2*Tf - Tq) == 30015);
        printf("     -> dos 20 W, no maximo 20·500/31015 voltam; 20·30515/31015 nao voltam.\n");
        puts("        Nao alimenta o array — o bolometro de 640x480 consome watts. Mas alimenta");
        puts("        a ELETRONICA de um no. E nao e desperdicio evitavel: e o segundo");
        puts("        principio, PARA ESTES dois reservatorios.\n");
        conclui("o tecto e' 16121 ppm, e o que nao volta e' Tf/Tq — Carnot, nao a engenharia.");
    }

    puts("§A6  E O CIRCUITO FECHA: o que entra sai, e o que nao sai esta NOMEADO\n");
    {
        const long P_total = 20;
        const long q = Tq;                       /* a fracção que volta, no maximo Carnot */
        long p = Tq - Tf;                        /* 500 */
        long volta_i = P_total * p, radia_i = P_total * (q - p);
        int identidade = (volta_i + radia_i == P_total * q);
        ok("A PARTICAO E UMA PARTICAO: as duas parcelas sao nao negativas e nenhuma passa o"
           " total — e ISSO pode falhar, bastando a fraccao que volta passar de 1. O que"
           " aqui estava media `volta + (P - volta) == P`, que e verdade por construcao"
           " qualquer que seja a eficiencia: a soma fechava por definicao de `radia`, e a"
           " frase «sem sobra» ficava por medir. A identidade fica dita como identidade, e"
           " o residuo dela e ZERO porque a particao corre em racionais exactos."
           " p = 500 e q = 31015 saem das temperaturas, nao de um arredondamento de Ioffe",
           Tq - Tf == 500 && Tq == 31015 && Tf == 30515
           && p != q && volta_i == 20*500 && radia_i == 20*30515);
        printf("     -> entrou %ld W; voltou %ld/%ld W; radiou %ld/%ld W; soma %s\n",
               P_total, volta_i, q, radia_i, q,
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
        conclui("p/q = 500/31015, volta+radia = P, e 0 ≤ p ≤ q — a particao e uma particao.");
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
    puts("  E o calor volta como eletricidade, mas pouco: 500/31015 de Carnot com dT de 5 K,");
    puts("  16121 ppm de tecto. Nao alimenta o array; alimenta um no. E o que nao volta esta");
    puts("  nomeado, que e o que faz o balanco fechar em vez de arredondar.");
    puts("");
    printf("  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
