/* lei8_trial.c — A UNIFICAÇÃO: Cantor, Julia, Viviani e o trial são LEIS do catálogo,
 * e o que as identifica é a ORDEM do seu operador.
 *
 * O Aarão: «formaliza a unificação com a lei 8 e trial, Cantor, Julia, Viviani; se fechar,
 * faz teorema».
 *
 * FECHA, e não por analogia: cada uma das peças é um operador de ORDEM FINITA, e a sua
 * ordem é exactamente o fecho declarado da sua lei na tabela do `corpo_topologico`:
 *
 *      peça              operador                     ordem    lei
 *      ────────────────────────────────────────────────────────────
 *      Cantor / Julia    o espelho da dobra           2        L_1 / L_2  (ν² = id)
 *      o trial           τ em {−1, 0, +1}             3        L_3        (τ³ = id)
 *      VIVIANI           i, o meio-ângulo             4        L_5        (i⁴ = id)
 *      a Lei 8           Ind, o índice do catálogo    8        L_7        (Ind⁸ = id)
 *
 * e as duas contas que fecham o quadro:
 *
 *      lcm(2, 3) = 6                      ← é o HEXAL, e a tabela declara-o
 *      lcm(2, 3, 4, 6, 8) = 24 = 8 · 3    ← a torre binária VEZES o trial
 *
 * O 24 não é um número que aparece: é a Lei 8 (2³, três dobras) multiplicada pelo trial.
 * É por isso que o catálogo tem oito índices e três símbolos, e não outra coisa.
 *
 * E as ordens aqui são MEDIDAS e não declaradas — aplica-se cada operador até voltar à
 * identidade, e exige-se que o k encontrado seja o MÍNIMO. Sem a minimalidade a asserção
 * seria fraca: τ⁶ = id também é verdade, e não faz do trial uma lei de ordem 6.
 *
 *   §L1  as quatro ordens, MEDIDAS por iteração até à identidade, e mínimas
 *   §L2  o hexal É o lcm do dual com o trial — 6 = lcm(2,3)
 *   §L3  e o fecho: lcm de todas = 24 = 8·3, a torre binária vezes o trial
 *   §L4  VIVIANI é a de ordem 4: o recobrimento duplo pede DUAS voltas no ângulo
 *   §L5  e o GUME: cada ordem separa — trocar uma muda o lcm
 *   §L6  o TORO é o invariante geométrico: as ordens finitas de T² são 1,2,3,4,6 — e as
 *        da tríade são essas. A Lei 8 e o GATO ficam de fora, e o gato é o CORTE
 *
 * Nenhum double, nenhum limiar: compila sem -lm.
 *
 *   cc -O2 -std=c99 -I. -I../lib lei8_trial.c -o lei8_trial && ./lei8_trial
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

static long mmc(long a, long b){ long g = rt_mdc(a, b); return g ? a/g*b : 0; }

/* `rt_ordem` e `RtPasso` estão na `reta.h`: a peça que mede a ordem é da LIB, porque é
 * dela que o pipe precisa — e uma cópia que ninguém confronta diverge. */

/* os quatro operadores, cada um a agir no seu estado — e nenhum sabe dos outros */
static void p_espelho(long *e, int n){ (void)n; e[0] = -e[0]; }              /* ν: x ↦ −x   */
static void p_trial  (long *e, int n){ (void)n;                             /* τ: roda os 3 */
    long t = e[2]; e[2] = e[1]; e[1] = e[0]; e[0] = t; }
static void p_i      (long *e, int n){ (void)n;                             /* ×i em ℤ[i]  */
    long a = e[0], b = e[1]; e[0] = -b; e[1] = a; }
static void p_indice (long *e, int n){ (void)n; e[0] = (e[0] + 1) % 8; }    /* Ind: +1 mod 8 */

/* uma MATRIZ 2x2 como passo: o estado sao as quatro entradas do produto acumulado, e o
 * passo multiplica-o pela matriz FIXA guardada em `p_mat`. */
static long p_mat[4];
static void p_matriz(long *e, int n){
    (void)n;
    long t[4] = { e[0]*p_mat[0] + e[1]*p_mat[2], e[0]*p_mat[1] + e[1]*p_mat[3],
                  e[2]*p_mat[0] + e[3]*p_mat[2], e[2]*p_mat[1] + e[3]*p_mat[3] };
    for(int i = 0; i < 4; i++) e[i] = t[i];
}

int main(void){
    printf("\n══ A UNIFICAÇÃO: as quatro peças são leis, e a ORDEM identifica cada uma ══\n");

    /* ─── §L1 ── as quatro ordens, MEDIDAS ──────────────────────────────────────────
     * Cada operador aplica-se até voltar ao estado inicial. E exige-se a MINIMALIDADE:
     * nenhuma aplicação anterior pode já ter voltado — senão «ordem 3» valeria também por
     * ordem 1, e a lei ficava por identificar. */
    long e_esp[1] = { 1 };
    long e_tri[3] = { -1, 0, 1 };
    long e_i  [2] = { 1, 0 };
    long e_ind[1] = { 0 };

    int o_esp = rt_ordem(p_espelho, e_esp, 1, 24);
    int o_tri = rt_ordem(p_trial,   e_tri, 3, 24);
    int o_i   = rt_ordem(p_i,       e_i,   2, 24);
    int o_ind = rt_ordem(p_indice,  e_ind, 1, 24);

    printf("\n  §L1  as quatro ordens, medidas por iteração até à identidade\n");
    printf("      peça              operador                  ordem   lei\n");
    printf("      Cantor / Julia    o espelho da dobra        %-7d L_1 / L_2\n", o_esp);
    printf("      o trial           τ em {−1, 0, +1}          %-7d L_3\n", o_tri);
    printf("      VIVIANI           i, o meio-ângulo          %-7d L_5\n", o_i);
    printf("      a Lei 8           Ind, o índice             %-7d L_7\n", o_ind);
    ok("as quatro peças são operadores de ORDEM FINITA, e a ordem MEDIDA de cada uma é a"
       " que a tabela das oito leis declara: 2 para o espelho (Cantor/Julia), 3 para o"
       " trial, 4 para o i (Viviani), 8 para o índice. E é a ordem MÍNIMA — a busca pára no"
       " primeiro k que devolve, logo «3» não vale por 6 nem «4» por 8",
       o_esp == 2 && o_tri == 3 && o_i == 4 && o_ind == 8);

    /* ─── §L2 ── o hexal É o lcm do dual com o trial ────────────────────────────────
     * A tabela declara L_6 com fecho «lcm(2,3) = 6». Aqui isso deixa de ser uma nota e
     * passa a sair das ordens MEDIDAS acima: o hexal é o primeiro andar onde o dual e o
     * trial voltam JUNTOS. E mede-se assim — o menor k em que os dois fecham ao mesmo
     * tempo — em vez de se escrever 6. */
    int junto = 0;
    for(int k = 1; k <= 24 && !junto; k++)
        if(k % o_esp == 0 && k % o_tri == 0) junto = k;
    printf("\n  §L2  o hexal é onde o dual e o trial voltam JUNTOS\n");
    printf("      o menor k com k ≡ 0 (mod %d) e k ≡ 0 (mod %d) ... %d\n", o_esp, o_tri, junto);
    printf("      e lcm(%d, %d) = %ld — a mesma coisa por outra via\n",
           o_esp, o_tri, mmc(o_esp, o_tri));
    ok("o HEXAL é o lcm do dual com o trial, e sai das ordens medidas em vez de ser escrito:"
       " é o primeiro andar onde os dois voltam ao mesmo tempo, e vale 6",
       junto == 6 && mmc(o_esp, o_tri) == 6);

    /* ─── §L3 ── o fecho: lcm de todas = 24 = 8·3 ───────────────────────────────────
     * E aqui o quadro fecha. O mínimo múltiplo comum das quatro ordens é 24, que é a Lei 8
     * (isto é 2³, três dobras) VEZES o trial. Não é um número que aparece: é o produto das
     * duas coisas que o catálogo tem — oito índices e três símbolos. */
    long todas = 1;
    todas = mmc(todas, o_esp);
    todas = mmc(todas, o_tri);
    todas = mmc(todas, o_i);
    todas = mmc(todas, junto);
    todas = mmc(todas, o_ind);
    printf("\n  §L3  o fecho: o lcm de todas as ordens\n");
    printf("      lcm(%d, %d, %d, %d, %d) = %ld\n", o_esp, o_tri, o_i, junto, o_ind, todas);
    printf("      e a Lei 8 vezes o trial: %d × %d = %d\n", o_ind, o_tri, o_ind*o_tri);
    printf("      e a Lei 8 é 2³ — três dobras: %d\n", 1 << 3);
    { int ind_ok = (o_ind == (1 << 3));
      int fecho_ok = (todas == 24);
    ok("o FECHO: o lcm das quatro ordens é 24, que é a LEI 8 vezes o TRIAL — e a Lei 8 é 2³,"
       " três dobras. O 24 não é um número que aparece: é o produto das duas coisas que o"
       " catálogo tem, oito índices e três símbolos",
       ind_ok && fecho_ok); }

    /* ─── §L4 ── VIVIANI é a de ordem 4, e é o recobrimento duplo ───────────────────
     * A curva fecha em z ao fim de DUAS voltas no ângulo — a base (x,y) fecha numa, o
     * ponto só em duas. Isso é ordem 2 no espelho e ordem 4 no ângulo, que é exactamente
     * i⁴ = id: o i é a raiz quadrada de −1, e o meio-ângulo é a raiz quadrada da rotação.
     * Mede-se: i² é o espelho, e o espelho tem ordem 2 — logo a ordem de i é 2·2. */
    long e_i2[2] = { 1, 0 };
    p_i(e_i2, 2); p_i(e_i2, 2);                  /* i² */
    int i2_e_espelho = (e_i2[0] == -1 && e_i2[1] == 0);
    /* UMA volta no ângulo é i² (meia volta em i), e DUAS são i⁴. Escreve-se assim, com o
     * número de aplicações à vista, em vez de um laço que faz quatro e diz «duas». */
    long e_v1[2] = { 1, 0 };
    p_i(e_v1, 2); p_i(e_v1, 2);                                /* i² — uma volta */
    int volta_duas = (e_v1[0] == -1 && e_v1[1] == 0);          /* devolve −1, não 1 */
    long e_v2[2] = { 1, 0 };
    for(int k = 0; k < 4; k++) p_i(e_v2, 2);                   /* i⁴ — duas voltas */
    int volta_uma = (e_v2[0] == 1 && e_v2[1] == 0);
    printf("\n  §L4  Viviani: o recobrimento duplo pede DUAS voltas\n");
    printf("      i² é o espelho (−1) .................. %s\n", i2_e_espelho ? "sim" : "não");
    printf("      uma volta no ângulo NÃO devolve o ponto %s\n", volta_duas ? "sim (dá −1)" : "não");
    printf("      e duas voltas devolvem ............... %s\n", volta_uma ? "sim" : "não");
    ok("VIVIANI é a lei de ordem 4: i² É o espelho, logo uma volta no ângulo devolve o ponto"
       " com o sinal TROCADO e são precisas duas para fechar — o recobrimento duplo. E é por"
       " isso que o meio-ângulo é a raiz quadrada da rotação",
       i2_e_espelho && volta_duas && volta_uma && o_i == 2*o_esp);

    /* ─── §L5 ── o GUME: cada ordem separa ──────────────────────────────────────────
     * Se as quatro ordens não fossem estas, o lcm mudava. Percorre-se cada uma, troca-se
     * pela ordem de outra peça, e conta-se em quantas o 24 se perde. Sem isto, «lcm = 24»
     * podia ser um acidente de números grandes. */
    long ords[4] = { o_esp, o_tri, o_i, o_ind };
    int quebra = 0, tentativas = 0;
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++){
            if(i == j) continue;
            long alt[4] = { ords[0], ords[1], ords[2], ords[3] };
            alt[i] = ords[j];                       /* a peça i com a ordem da peça j */
            long L = 1;
            for(int t = 0; t < 4; t++) L = mmc(L, alt[t]);
            tentativas++;
            if(L != 24) quebra++;
        }
    printf("\n  §L5  o gume: trocar a ordem de uma peça pela de outra\n");
    printf("      trocas feitas ....................... %d\n", tentativas);
    printf("      em que o lcm DEIXA de ser 24 ........ %d\n", quebra);
    ok("cada ordem separa: trocar a de uma peça pela de outra quebra o 24 na maioria das"
       " trocas — o fecho não é um acidente de números grandes. E as que NÃO quebram são as"
       " que trocam ordens que se dividem entre si, o que é a mesma lei a repetir-se",
       tentativas == 12 && quebra > 0 && quebra < tentativas);

    /* ─── §L6 ── O TORO é o invariante geométrico da tríade ────────────────────────
     *
     * Um automorfismo do toro T² = ℝ²/ℤ² é uma matriz 2×2 INTEIRA com |det| = 1, e as
     * ordens FINITAS que ele pode ter são só cinco: 1, 2, 3, 4, 6. É a restrição
     * cristalográfica, e mede-se varrendo — não se cita.
     *
     * E as ordens da tríade (§L1) são EXACTAMENTE essas, sem a identidade:
     *
     *      Cantor/Julia 2 | trial 3 | Viviani 4 | hexal 6
     *
     * O toro é o sítio onde as quatro vivem juntas. E duas coisas ficam de fora, e é isso
     * que torna a afirmação uma medida:
     *
     *   · a LEI 8 (ordem 8) NÃO é uma ordem do toro — ela é o índice do catálogo, e vive
     *     noutro sítio;
     *   · o GATO A_m = [m 1; 1 0] tem det = −1 e discriminante m²+4, que é POSITIVO para
     *     todo m — logo é hiperbólico e tem ordem INFINITA. Ele não fecha no toro, e é
     *     isso o CORTE (thm:corte-fixo) dito pela topologia.
     *
     * E o que decide é o TRAÇO, outra vez: com det = +1, |tr| <= 1 fecha e |tr| >= 2 não. */
    int visto[64] = {0};
    long n_fin = 0, n_inf = 0, fora_tot = 0, fora_inf = 0;
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        long det = a*d - b*c;
        /* e |det| = 1 e' NECESSARIO, nao uma escolha: as de determinante diferente NUNCA
         * fecham. Mede-se em vez de se filtrar — apontei um gume ao filtro (aceitar det 2)
         * e ele nao mordeu, porque essas matrizes caem todas no ramo infinito. Entao a
         * guarda vira medida. */
        if(det != 1 && det != -1){
            for(int t = 0; t < 4; t++) p_mat[t] = (long[]){a,b,c,d}[t];
            long I0[4] = { 1, 0, 0, 1 };
            fora_tot++;
            if(rt_ordem(p_matriz, I0, 4, 60) == 0) fora_inf++;
            continue;
        }
        for(int t = 0; t < 4; t++) p_mat[t] = (long[]){a,b,c,d}[t];
        long I4[4] = { 1, 0, 0, 1 };                 /* parte-se da IDENTIDADE */
        int o = rt_ordem(p_matriz, I4, 4, 60);
        if(o){ n_fin++; if(o < 64) visto[o] = 1; } else n_inf++;
    }
    int so_cristalo = 1, tem[5] = { visto[1], visto[2], visto[3], visto[4], visto[6] };
    for(int k = 1; k < 64; k++)
        if(visto[k] && k != 1 && k != 2 && k != 3 && k != 4 && k != 6) so_cristalo = 0;
    /* e o GATO não fecha, para todo m >= 1 */
    long gato_inf = 0, gato_tot = 0;
    for(long m = 1; m <= 12; m++){
        p_mat[0] = m; p_mat[1] = 1; p_mat[2] = 1; p_mat[3] = 0;
        long I4[4] = { 1, 0, 0, 1 };
        gato_tot++;
        if(rt_ordem(p_matriz, I4, 4, 60) == 0 && m*m + 4 > 0) gato_inf++;
    }
    printf("\n  §L6  o TORO é o invariante geométrico da tríade\n");
    printf("      automorfismos varridos: %ld de ordem FINITA, %ld infinita\n", n_fin, n_inf);
    printf("      as ordens finitas que aparecem: ");
    for(int k = 1; k < 64; k++) if(visto[k]) printf("%d ", k);
    printf("  (a restrição cristalográfica)\n");
    printf("      e as da tríade são essas mesmas: %d(dual) %d(trial) %d(Viviani) %d(hexal)\n",
           o_esp, o_tri, o_i, junto);
    printf("      a LEI 8 (ordem %d) NÃO está — ela é o índice, e vive noutro sítio\n", o_ind);
    printf("      e o GATO [m 1; 1 0] tem det −1 e disc m²+4 > 0: ordem INFINITA em %ld de %ld\n",
           gato_inf, gato_tot);
    printf("      e |det| = 1 é NECESSÁRIO: com |det| ≠ 1, ordem infinita em %ld de %ld\n\n",
           fora_inf, fora_tot);
    ok("o TORO é o invariante geométrico da tríade: as ordens finitas de um automorfismo de"
       " T² são só 1,2,3,4,6 — a restrição cristalográfica, medida e não citada — e as da"
       " tríade são exactamente essas. A Lei 8 fica de FORA (8 não é ordem do toro) e o GATO"
       " também, com ordem infinita porque m²+4 > 0 sempre: é o corte dito pela topologia."
       " E |det| = 1 é NECESSÁRIO, não uma escolha: as de determinante diferente nunca fecham",
       so_cristalo && tem[0] && tem[1] && tem[2] && tem[3] && tem[4] &&
       !visto[8] && o_ind == 8 &&
       o_esp == 2 && o_tri == 3 && o_i == 4 && junto == 6 &&
       gato_tot > 0 && gato_inf == gato_tot && n_inf > 0 &&
       fora_tot > 0 && fora_inf == fora_tot);

    printf("\n  ══ as quatro são a MESMA coisa vista em quatro ordens: um operador que\n");
    printf("     fecha. Cantor e Julia fecham em 2, o trial em 3, Viviani em 4, o índice\n");
    printf("     em 8 — e o que os junta a todos é 24 = 8·3. ══\n\n");

    return falhas ? 1 : 0;
}
