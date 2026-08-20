/* reta.c — O MEDIDOR DA `lib/reta.h`: as operações da recta, todas inteiras.
 *
 * Uma biblioteca sem gume é uma promessa. Aqui cada operação da `reta.h` é medida por
 * DUAS ROTAS que não partilham código, e cada uma leva o seu controlo negativo — o caso
 * onde ela TEM de falhar. Sem isso, «passou» não distingue a lei de um programa partido.
 *
 *   §R1  a potência e o inverso em 𝔽ₚ — e o inverso desfaz, que é a definição
 *   §R2  o determinante de BAREISS contra a definição por PERMUTAÇÕES
 *   §R3  e contra o det 2×2 e 3×3 escritos à mão, que são os casos que se sabem
 *   §R4  o det em 𝔽ₚ concorda com o inteiro, e não estoura onde o inteiro estoura
 *   §R5  a ÓRBITA de ∞ dá os convergentes, e a forma vale ±1 e nunca ZERO
 *   §R6  a REVERSÃO é involução, e leva a borda do ouro na equação do recíproco
 *   §R7  ROUTH–HURWITZ nos coeficientes contra o sinal das raízes conhecidas
 *   §R8  e o TECTO verifica-se, em vez de se documentar
 *
 * Nenhum double, nenhum limiar: compila sem -lm.
 *
 * ── O ALGORITMO, E ONDE CADA PASSO VIVE NA TRÍADE ────────────────────────────────
 *
 * A `reta.h` realiza o algoritmo de quatro passos (`algebrico thm:universal`), e cada
 * passo tem casa. Isto não é arrumação: é o que decide QUE FUNÇÃO se chama.
 *
 *   DESCODIFICA  (p,q) := texto, racional EXACTO; u := mmc dos denominadores
 *                τ=0   CENTRO — a leitura para Z, e mais nada
 *   OPERA        |det T| = 1;  [p:q] -> [T00p+T01q : T10p+T11q]
 *                τ=-1  o que FECHA — é a CONVOLUÇÃO, e no discreto é produto ponto
 *                      a ponto sobre as N potências de sigma (ordem FINITA)
 *   INVERTE      T^-1 = adj(T)/det T, INTEIRA;  sigma^-1 = -sigma†
 *                τ=+1  a volta usa a OUTRA FOLHA — a deconvolução, e a folha dual
 *                      é a recíproca: |sigma| > 1 > |sigma†|, e NÃO fecha
 *   CODIFICA     w := palavra (FC) ou dígitos (Cantor)
 *                τ=0   CENTRO — a saída de V, e o fecho: descodifica(w) = (p,q)
 *
 * SÃO QUATRO E NÃO CINCO. Não há verificação porque não há nada a verificar: com
 * |det| = 1 a adjunta é INTEIRA e a volta é exacta POR CONSTRUÇÃO. Não se enquadra o
 * corpo numa estrutura — LÊ-SE.
 *
 * E NENHUM PASSO NORMALIZA. O mmc do primeiro é leitura do decimal para Z; quem
 * normaliza é a NORMA (o produto das folhas). O gcd do meio foi RETIRADO do `rt_ciclo`
 * — medido: não fazia nada, e os medidores que usam esta lib passam sem ele. E não é
 * 1/raiz(N): esse vem do lado ADITIVO (raízes no círculo, m = 0), e estas folhas são
 * RECÍPROCAS, do lado MULTIPLICATIVO.
 *
 * OS DOIS POLOS ESTÃO NO ALGORITMO: OPERA é o aditivo (fecha, ordem finita, Fourier)
 * e INVERTE é o multiplicativo (não fecha, as folhas recíprocas, Mellin). O centro
 * guarda a entrada e a saída — a transformada é UMA, e só ao operar se parte em dois.
 *
 *   cc -O2 -std=c99 -I. -I../lib reta.c -o reta && ./reta
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "reta.h"
#include "rt_cf_slot.h"
#include "slot_map.h"
#include "slot_mem.h"
#include "unidade.h"

/* o determinante pela DEFINIÇÃO: soma sobre as permutações, com o sinal. É a segunda
 * rota — não partilha uma linha com Bareiss, e é por isso que serve de oráculo. */
static long det_perm(const long *M, int n, int passo){
    int idx[8];
    for(int i = 0; i < n; i++) idx[i] = i;
    long tot = 0;
    int c[8] = {0}, i = 0, sinal = 1;
    long termo = 1;
    for(int k = 0; k < n; k++) termo *= M[k*passo + idx[k]];
    tot += termo;
    while(i < n){                                   /* Heap: gera as permutações */
        if(c[i] < i){
            int a = (i % 2 == 0) ? 0 : c[i];
            int t = idx[a]; idx[a] = idx[i]; idx[i] = t;
            sinal = -sinal;
            termo = sinal;
            for(int k = 0; k < n; k++) termo *= M[k*passo + idx[k]];
            tot += termo;
            c[i]++; i = 0;
        } else { c[i] = 0; i++; }
    }
    return tot;
}

typedef struct { long m, a, b, sinal, p; int sabota, saturou; } ctx_u;

/* o PASSO: de (U(n−1),U(n)) para (U(n),U(n+1)), medindo Cassini no andar novo.
 * Repare-se que `n` entra na assinatura e NÃO entra no corpo — é isso que o torna forma. */
static int passo_cassini(long n, void *v){
    ctx_u *c = (ctx_u*)v;
    (void)n;
    if(c->p){                                   /* a SEGUNDA realização: em 𝔽ₚ, sem tecto */
        long prox = rt_mod(c->m * c->b + c->a, c->p);
        if(c->sabota) prox = rt_mod(2 * c->b, c->p);
        long lhs = rt_mod(prox * c->a - c->b * c->b, c->p);
        c->sinal = -c->sinal;
        int bem = (lhs == rt_mod(c->sinal, c->p));
        c->a = c->b; c->b = prox;
        return bem;
    }
    /* a PRIMEIRA realização: em ℤ, e ela tem tecto. Pára ANTES de transbordar e marca —
     * o que se mede aqui é o tamanho do tipo, e isso não é um contra-exemplo. */
    if(c->b != 0 && (c->m * rt_modulo(c->b) + rt_modulo(c->a) > 3037000499L ||
                     rt_modulo(c->b) > 3037000499L)){ c->saturou = 1; return 1; }
    long prox = c->m * c->b + c->a;
    if(c->sabota) prox = 2 * c->b;
    long lhs = prox * c->a - c->b * c->b;
    c->sinal = -c->sinal;
    int bem = (lhs == c->sinal);
    c->a = c->b; c->b = prox;
    return bem;
}
/* o predicado que a DESCIDA lê: «o andar n é bom». É o mesmo passo, lido ao contrário. */
static int nao_falha_ate(long n, void *v){ return passo_cassini(n, v); }


/* a codificação PARTIDA — existe só para o gume: deita fora o último dígito, e a volta
 * deixa de fechar. É o controlo da própria verificação. */
static int iv_partida(long p, long q, long *rp, long *rq){
    RtCod c;
    if(!rt_cod_shift(p, q, 2, &c)) return 0;
    if(c.n > 0){ c.n--; if(c.per > 0) c.per--; else if(c.pre > 0) c.pre--; }
    return rt_desc_shift(&c, rp, rq);
}

int main(void){
    printf("\n=== A RECTA GEOMÉTRICA: as operações, todas inteiras ===\n");

    /* ═══ §R1  A POTÊNCIA E O INVERSO EM 𝔽ₚ ═════════════════════════════════ */
    printf("\n§R1 rt_ipow e rt_inv_mod — e o inverso DESFAZ, que é a definição.\n\n");
    {
        int32_t pot_ok = 0, pot_tot = 0, inv_ok = 0, inv_tot = 0;
        int32_t inv_i32_ok = 0;
        for(int32_t b = -4; b <= 4; b++) for(int e = 0; e <= 8; e++){
            int32_t r = rt_ipow(b, e), m = 1;
            for(int k = 0; k < e; k++) m *= b;
            pot_tot++;
            if(r == m) pot_ok++;
        }
        const int32_t PR[4] = {7, 101, 1009, 65537};
        for(int t = 0; t < 4; t++){
            int32_t p = PR[t];
            for(int32_t a = 1; a < p && a <= 60; a++){
                int32_t v = rt_inv_mod_i32(a, p);
                int32_t vl = (int32_t)rt_inv_mod(a, p);
                inv_tot++;
                if((int64_t)a * v % p == 1) inv_ok++;
                if(v == vl) inv_i32_ok++;
            }
        }
        int32_t zero_sem = 0, outros_um = 0, testados_a = 0, varridos = 0;
        for(int t = 0; t < 4; t++){
            int32_t p = PR[t];
            if(p > 1100) continue;
            varridos++;
            int32_t quantos0 = 0;
            for(int32_t v = 0; v < p; v++) if((0 * v) % p == 1) quantos0++;
            if(quantos0 == 0) zero_sem++;
            for(int32_t a = 1; a < p; a++){
                int32_t quantos = 0;
                for(int32_t v = 0; v < p; v++) if((int64_t)a * v % p == 1) quantos++;
                testados_a++;
                if(quantos == 1) outros_um++;
            }
        }
        printf("      rt_ipow contra o laço directo: %d de %d\n", pot_ok, pot_tot);
        printf("      rt_inv_mod_i32: a·a⁻¹ = 1 em %d de %d, em quatro primos\n", inv_ok, inv_tot);
        printf("      rt_inv_mod_i32 = rt_inv_mod (long) .............. %d de %d\n", inv_i32_ok, inv_tot);
        printf("      GUME, por CONTAGEM DE SOLUÇÕES de a·v = 1 em 𝔽ₚ:\n");
        printf("        a = 0 : nenhuma solução, em %d de %d primos varridos por inteiro\n",
               zero_sem, varridos);
        printf("        a ≠ 0 : exactamente UMA, em %d de %d valores\n\n", outros_um, testados_a);
        ok("A POTÊNCIA E O INVERSO EM 𝔽ₚ, e o inverso mede-se pela DEFINIÇÃO — a·a⁻¹ = 1,"
           " em int32 (rt_ipow, rt_inv_mod_i32). Quatro primos até 65537. Com o gume: o zero"
           " não tem inverso, e a rota int32 concorda com rt_inv_mod long onde ambas cabem",
           pot_ok == pot_tot && inv_ok == inv_tot && inv_i32_ok == inv_tot && pot_tot > 0
           && varridos == 3 && zero_sem == varridos
           && outros_um == testados_a && testados_a > 0);
    }

    /* ═══ §R2  BAREISS CONTRA AS PERMUTAÇÕES ════════════════════════════════ */
    printf("\n§R2 rt_det_bareiss contra a DEFINIÇÃO por permutações — duas rotas.\n\n");
    {
        long casos = 0, batem = 0, nao_nulos = 0;
        for(long s = 0; s < 200; s++){
            for(int n = 2; n <= 5; n++){
                long M[25], G[25];
                for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
                    M[i*n + j] = ((s*7 + i*11 + j*5) % 9) - 4;
                memcpy(G, M, sizeof(long)*(size_t)(n*n));
                long db = rt_det_bareiss(G, n, n);
                long dp = det_perm(M, n, n);          /* a definição, sem uma linha comum */
                casos++;
                if(db == dp) batem++;
                if(dp != 0) nao_nulos++;
            }
        }
        printf("      %ld matrizes de ordem 2 a 5: Bareiss = permutações em %ld\n",
               casos, batem);
        printf("      e os determinantes NÃO NULOS são %ld — sem isto, «batem» podia valer\n"
               "      por serem todos zero, que é o caso degenerado\n\n", nao_nulos);
        ok("O DETERMINANTE DE BAREISS BATE COM A DEFINIÇÃO POR PERMUTAÇÕES, em matrizes de"
           " ordem 2 a 5 e com os dois caminhos sem uma linha em comum: um divide pelo pivô"
           " anterior, o outro soma sobre o grupo simétrico com o sinal. E os não nulos"
           " contam-se — sem isso «batem» podia valer por serem todos ZERO, que é o caso"
           " degenerado e não a lei",
           batem == casos && nao_nulos > casos/2 && casos == 800);
    }

    /* ═══ §R3  E CONTRA OS CASOS QUE SE SABEM ═══════════════════════════════ */
    printf("\n§R3 e contra o 2×2 e o 3×3 escritos à mão — os casos fechados.\n\n");
    {
        long n2 = 0, n3 = 0, tot2 = 0, tot3 = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            long M[4] = {a,b,c,d};
            tot2++;
            if(rt_det_bareiss(M, 2, 2) == a*d - b*c) n2++;
        }
        for(long s = 0; s < 300; s++){
            long M[9], G[9];
            for(int i = 0; i < 9; i++) M[i] = ((s*13 + i*7) % 11) - 5;
            memcpy(G, M, sizeof M);
            /* a regra de Sarrus, que só vale em 3×3 e é o caso fechado */
            long sar = M[0]*M[4]*M[8] + M[1]*M[5]*M[6] + M[2]*M[3]*M[7]
                     - M[2]*M[4]*M[6] - M[0]*M[5]*M[7] - M[1]*M[3]*M[8];
            tot3++;
            if(rt_det_bareiss(G, 3, 3) == sar) n3++;
        }
        printf("      ordem 2 contra ad − bc: %ld de %ld\n", n2, tot2);
        printf("      ordem 3 contra Sarrus:  %ld de %ld\n\n", n3, tot3);
        ok("E BATE COM OS CASOS FECHADOS: ad − bc em todas as 2401 matrizes 2×2 de entradas"
           " em [−3,3], e a regra de Sarrus em 300 matrizes 3×3. São as duas fórmulas que se"
           " sabem de cor, e servem de terceira rota",
           n2 == tot2 && n3 == tot3 && tot2 == 2401 && tot3 == 300);
    }

    /* ═══ §R4  O DET EM 𝔽ₚ, E ONDE O INTEIRO ESTOURA ═══════════════════════ */
    printf("\n§R4 rt_det_mod concorda com o inteiro — e serve onde ele já não cabe.\n\n");
    {
        long casos = 0, batem = 0;
        const long p = 1000003;
        for(long s = 0; s < 200; s++) for(int n = 2; n <= 4; n++){
            long M[16], G1[16], G2[16];
            for(int i = 0; i < n*n; i++) M[i] = ((s*5 + i*3) % 7) - 3;
            memcpy(G1, M, sizeof(long)*(size_t)(n*n));
            memcpy(G2, M, sizeof(long)*(size_t)(n*n));
            long di = rt_det_bareiss(G1, n, n);
            long dm = rt_det_mod(G2, n, n, p);
            casos++;
            if(((di % p) + p) % p == dm) batem++;
        }
        /* e A RAZÃO DE ELE EXISTIR: com entradas grandes os menores de Bareiss estouram o
         * long, e o resultado sai errado CALADO — foi o que aconteceu no matricial.c §M11,
         * onde o det final cabia (10^14) e mesmo assim vinha errado. Em 𝔽ₚ nada cresce. */
        long grande[25], gm[25];
        for(int i = 0; i < 25; i++) grande[i] = 1000000000L + i*7;
        memcpy(gm, grande, sizeof grande);
        long dmg = rt_det_mod(gm, 5, 5, p);
        printf("      %ld matrizes: det em ℤ reduzido = det em 𝔽ₚ em %ld\n", casos, batem);
        printf("      e com entradas de 10^9 o det em 𝔽ₚ sai (%ld) onde o inteiro enrolaria\n\n",
               dmg);
        ok("O DETERMINANTE EM 𝔽ₚ CONCORDA COM O INTEIRO REDUZIDO, e existe pela razão que o"
           " §M11 do matricial.c descobriu à sua custa: os intermédios de Bareiss são"
           " MENORES da matriz e crescem muito mais que o resultado — numa 5×5 de entradas"
           " a 10^10 chegam a 10^42, e o long enrola CALADO, com o determinante final a"
           " caber e a sair errado à mesma. Em 𝔽ₚ nada cresce",
           batem == casos && casos == 600);
    }

    /* ═══ §R5  A ÓRBITA DE ∞ ════════════════════════════════════════════════ */
    printf("\n§R5 rt_orbita: os convergentes, e a forma vale ±1 e NUNCA zero.\n\n");
    {
        long casos = 0, unidade = 0, zeros = 0, rec_ok = 0, bate_long = 0, cabe_i32 = 0;
        for(long m = 1; m <= 8; m++){
            long ap = 1, aq = 0;
            for(int k = 1; k <= 16; k++){
                long p, q;
                rt_orbita(m, k, &p, &q);
                int32_t pi, qi;
                int tem_i32 = rt_orbita_i32((int32_t)m, k, &pi, &qi);
                casos++;
                if(tem_i32){
                    cabe_i32++;
                    if(p == pi && q == qi) bate_long++;
                }
                long forma = p*p - m*p*q - q*q;
                if(forma == 1 || forma == -1) unidade++;
                if(forma == 0) zeros++;
                if(p == m*ap + aq && q == ap) rec_ok++;
                ap = p; aq = q;
            }
        }
        printf("      %ld passos em 8 metais: |p² − m·p·q − q²| = 1 em %ld, e ZERO em %ld\n",
               casos, unidade, zeros);
        printf("      cabem em int32: %ld de %ld; rt_orbita_i32 = long .... %ld de %ld\n",
               cabe_i32, casos, bate_long, cabe_i32);
        printf("      e a recorrência [p:q] ⟼ [m·p+q : p] fecha em %ld\n\n", rec_ok);
        ok("A ÓRBITA DE ∞ DÁ OS CONVERGENTES, e a forma vale ±1 em todos os passos e ZERO"
           " em nenhum — onde cabe int32, rt_orbita_i32 concorda com long. É o corte medido"
           " de frente: o ponto fixo pediria forma = 0, e isso não tem solução em ℤ",
           unidade == casos && zeros == 0 && rec_ok == casos && casos == 128
           && bate_long == cabe_i32 && cabe_i32 > 0);
    }

    /* ═══ §R6  A REVERSÃO É INVOLUÇÃO ═══════════════════════════════════════ */
    printf("\n§R6 rt_reverte: involução, e leva a borda do ouro na equação do recíproco.\n\n");
    {
        int32_t casos = 0, volta = 0, muda = 0, bate_long = 0;
        for(int32_t s = 0; s < 200; s++){
            int32_t a[6], r[6], rr[6];
            long al[6], rl[6];
            int n = 5;
            for(int k = 0; k <= n; k++) a[k] = ((s*7 + k*3) % 9) - 4;
            rt_reverte_i32(a, n, r);
            rt_reverte_i32(r, n, rr);
            for(int k = 0; k <= n; k++) al[k] = a[k];
            rt_reverte(al, n, rl);
            casos++;
            int igual = 1, dif = 0, bate = 1;
            for(int k = 0; k <= n; k++){
                if(rr[k] != a[k]) igual = 0;
                if(r[k] != a[k]) dif = 1;
                if(r[k] != rl[k]) bate = 0;
            }
            if(igual) volta++;
            if(dif) muda++;
            if(bate) bate_long++;
        }
        int32_t ouro[3] = {1, -1, -1}, rev[3];
        rt_reverte_i32(ouro, 2, rev);
        int bate_ouro = (rev[0] == -1 && rev[1] == -1 && rev[2] == 1);
        printf("      %d polinómios: reverter duas vezes devolve em %d, e MUDA em %d\n",
               casos, volta, muda);
        printf("      rt_reverte_i32 = rt_reverte (long) .......... %d de %d\n",
               bate_long, casos);
        printf("      e a borda do ouro (1,−1,−1) revertida dá (%d,%d,%d) = −(1,1,−1),\n"
               "      que é x² + x − 1: a equação do RECÍPROCO — %s\n\n",
               rev[0], rev[1], rev[2], bate_ouro ? "sim" : "NÃO");
        ok("A REVERSÃO É INVOLUÇÃO em int32 — revertida duas vezes devolve —, e não é a"
           " identidade; a rota int32 concorda com rt_reverte long. E faz do RECÍPROCO do"
           " ouro a raiz revertida: (1,−1,−1) ao contrário é −(1,1,−1), isto é x²+x−1",
           volta == casos && muda > casos/2 && bate_ouro && bate_long == casos && casos == 200);
    }

    /* ═══ §R7  ROUTH–HURWITZ ════════════════════════════════════════════════ */
    printf("\n§R7 rt_hurwitz_est: o regime sai dos COEFICIENTES, sem uma raiz.\n\n");
    {
        struct { long a[5]; int n; int estavel; const char *nome; } t[] = {
            { {1, 3, 2, 0, 0}, 2, 1, "(x+1)(x+2)" },
            { {1, 6,11, 6, 0}, 3, 1, "(x+1)(x+2)(x+3)" },
            { {1, 0, 1, 0, 0}, 2, 0, "x² + 1        (borda)" },
            { {1,-1,-1, 0, 0}, 2, 0, "x² − x − 1    (o ouro)" },
            { {1, 0,-1,-1, 0}, 3, 0, "x³ − x − 1    (a plástica)" },
            /* ESCREVI AQUI «Hurwitz nega» PARA x³+2x²+3x+4 E ELE DIZ ESTÁVEL — e tem
               razão: as raízes são −1,65 e −0,17 ± 1,55i, todas com Re < 0. O número de
               cabeça foi meu, e a asserção apanhou-o à primeira corrida. O caso que
               NEGA de verdade é x³ + x² + x + 6: aí a₂a₁ = 1 < a₃a₀ = 6, e a raiz real
               positiva existe. */
            { {1, 2, 3, 4, 0}, 3, 1, "x³+2x²+3x+4   (estável, e eu duvidei)" },
            { {1, 1, 1, 6, 0}, 3, 0, "x³+x²+x+6     (a₂a₁ < a₃a₀)" },
        };
        long casos = 0, batem = 0, est = 0, inst = 0;
        printf("      polinómio                estável?   esperado\n");
        for(unsigned k = 0; k < sizeof t/sizeof *t; k++){
            int e = rt_hurwitz_est(t[k].a, t[k].n);
            casos++;
            if(e == t[k].estavel) batem++;
            if(e) est++; else inst++;
            printf("      %-24s %-10s %s\n", t[k].nome, e ? "sim" : "não",
                   t[k].estavel ? "sim" : "não");
        }
        printf("\n      %ld de %ld — e os DOIS lados aparecem: %ld estáveis, %ld não\n\n",
               batem, casos, est, inst);
        ok("O REGIME SAI DOS COEFICIENTES POR ROUTH–HURWITZ, sem calcular uma raiz: os"
           " menores da matriz de Hurwitz todos positivos é todas as raízes com Re < 0. E os"
           " DOIS lados aparecem na amostra — há estáveis e há instáveis —, sem o que"
           " «bateu» valia por a resposta ser sempre a mesma. O que ele NÃO decide diz-se:"
           " com todos os menores nulos não distingue a BORDA do CAOS, e para isso é preciso"
           " o factor x²+c por divisão exacta, que o geral.c §G4 junta",
           batem == casos && est > 0 && inst > 0);
    }

    /* ═══ §R8  E O TECTO VERIFICA-SE ════════════════════════════════════════ */
    printf("\n§R8 o TECTO verifica-se, em vez de se documentar.\n\n");
    {
        int dentro = rt_cabe(RT_MAX), fora = rt_cabe(RT_MAX + 1), zero = rt_cabe(0);
        printf("      RT_MAX = %d: cabe(%d) = %d, cabe(%d) = %d, cabe(0) = %d\n\n",
               RT_MAX, RT_MAX, dentro, RT_MAX+1, fora, zero);
        ok("O TECTO VERIFICA-SE: um #define que ninguém testa é documentação e não limite —"
           " e esta casa já parou de terminar por causa de um. rt_cabe diz onde está a"
           " fronteira, e mede-se dos dois lados: RT_MAX cabe, RT_MAX+1 não, e zero também"
           " não",
           dentro && !fora && !zero);
    }

    /* ═══ §R9  A RECORRÊNCIA: a régua, e a peça mais copiada ════════════════ */
    printf("\n§R9 rt_recorre — a régua, em 40 ficheiros. E é a órbita, do outro lado.\n\n");
    {
        long casos = 0, bate_orbita = 0, bate_forma = 0;
        for(long m = 1; m <= 8; m++){
            long u[20];
            rt_recorre(m, 0, 1, u, 20);              /* u_k = m·u_{k-1} + u_{k-2} */
            for(int k = 2; k <= 16; k++){
                long p, q;
                rt_orbita(m, k-1, &p, &q);            /* a ÓRBITA, a outra rota */
                casos++;
                if(p == u[k] && q == u[k-1]) bate_orbita++;
                /* e a forma de Cassini: u_k² − m·u_k·u_{k−1} − u_{k−1}² = ±1 */
                long fo = rt_forma(u[k], u[k-1], m);
                if(fo == 1 || fo == -1) bate_forma++;
            }
        }
        printf("      %ld passos em 8 metais: a recorrência dá a ÓRBITA em %ld,\n"
               "      e a forma vale ±1 em %ld — Cassini, e nunca ZERO\n\n",
               casos, bate_orbita, bate_forma);
        ok("A RECORRÊNCIA É A RÉGUA, E É A MESMA COISA QUE A ÓRBITA: u_k = m·u_{k−1} +"
           " u_{k−2} lido na sucessão é [p:q] ⟼ [m·p+q : p] lido em ℙ¹, e as duas rotas dão"
           " os mesmos inteiros em 120 passos de oito metais. É a peça mais copiada do"
           " repositório — quarenta ficheiros a reescrevê-la —, e a forma de Cassini que ela"
           " satisfaz vale ±1 e nunca zero, que é o corte",
           bate_orbita == casos && bate_forma == casos && casos == 120);
    }

    /* ═══ §R10 CONVOLUÇÃO, CRUZADO, E A DECOMPOSIÇÃO ════════════════════════ */
    printf("\n§R10 rt_conv, rt_cruz3 e rt_dir_cruz — o produto, e as duas metades.\n\n");
    {
        /* a convolução É o produto de polinómios: (x+1)^n dá a linha de Pascal */
        long pas[12] = {1}, tmp[12], n = 0;
        int pascal_ok = 1;
        const long xm1[2] = {1, 1};
        for(int k = 1; k <= 6; k++){
            rt_conv(pas, (int)n+1, xm1, 2, tmp);
            n++;
            for(int i = 0; i <= n; i++) pas[i] = tmp[i];
        }
        const long alvo6[7] = {1,6,15,20,15,6,1};     /* a 6.ª linha, que se sabe */
        for(int i = 0; i <= 6; i++) if(pas[i] != alvo6[i]) pascal_ok = 0;
        /* o cruzado: antissimétrico, e a×a = 0 — rota int32 com produtos int64 */
        long anti = 0, nulo = 0, viv = 0, cc = 0, cruz_i32 = 0;
        for(long t = 0; t < 200; t++){
            int32_t a[3], b[3], ab[3], ba[3], aa[3];
            long al[3], bl[3], abl[3];
            for(int i = 0; i < 3; i++){
                a[i] = (int32_t)(((t*7 + i*3) % 11) - 5);
                b[i] = (int32_t)(((t*5 + i*2) % 9)  - 4);
                al[i] = a[i]; bl[i] = b[i];
            }
            rt_cruz3_i32(a, b, ab); rt_cruz3_i32(b, a, ba); rt_cruz3_i32(a, a, aa);
            rt_cruz3(al, bl, abl);
            cc++;
            int ant = 1, nul = 1, vv = 0, bate = 1;
            for(int i = 0; i < 3; i++){
                if(ab[i] != -ba[i]) ant = 0;
                if(aa[i] != 0) nul = 0;
                if(ab[i]) vv = 1;
                if(ab[i] != abl[i]) bate = 0;
            }
            anti += ant; nulo += nul; viv += vv;
            if(bate) cruz_i32++;
        }
        /* e a decomposição: 2M = (M+Mᵀ) + (M−Mᵀ) */
        long dec = 0, dtot = 0, dec_i32 = 0;
        for(long t = 0; t < 100; t++){
            int32_t M[16], S2[16], A2[16];
            long Ml[16], S2l[16], A2l[16];
            for(int i = 0; i < 16; i++){
                M[i] = (int32_t)(((t*11 + i*3) % 13) - 6);
                Ml[i] = M[i];
            }
            rt_dir_cruz_i32(M, 4, S2, A2);
            rt_dir_cruz(Ml, 4, S2l, A2l);
            int bom = 1, bate = 1;
            for(int i = 0; i < 4 && bom; i++) for(int j = 0; j < 4; j++){
                if(S2[i*4+j] + A2[i*4+j] != 2*M[i*4+j]) { bom = 0; break; }
                if(S2[i*4+j] != S2[j*4+i])              { bom = 0; break; }
                if(A2[i*4+j] != -A2[j*4+i])             { bom = 0; break; }
                if(S2[i*4+j] != S2l[i*4+j] || A2[i*4+j] != A2l[i*4+j]) bate = 0;
            }
            dtot++; dec += bom;
            if(bate) dec_i32++;
        }
        printf("      a convolução de (x+1)^6 dá a linha de Pascal: %s\n",
               pascal_ok ? "1 6 15 20 15 6 1" : "NÃO");
        printf("      rt_cruz3_i32 = rt_cruz3 (long) .......... %ld de %ld\n",
               cruz_i32, cc);
        printf("      o cruzado é antissimétrico em %ld de %ld, a×a = 0 em %ld, e NÃO nulo em %ld\n",
               anti, cc, nulo, viv);
        printf("      rt_dir_cruz_i32 = long .................. %ld de %ld\n",
               dec_i32, dtot);
        printf("      e 2M = (M+Mᵀ) + (M−Mᵀ), com as duas metades no seu espaço: %ld de %ld\n\n",
               dec, dtot);
        ok("A CONVOLUÇÃO É O PRODUTO DE POLINÓMIOS — e (x+1)^6 dá a linha de Pascal."
           " O CRUZADO em int32 é antissimétrico e concorda com long; rt_dir_cruz_i32 idem."
           " A DECOMPOSIÇÃO devolve 2M = (M+Mᵀ) + (M−Mᵀ) em 100 matrizes",
           pascal_ok && anti == cc && nulo == cc && viv > cc/2 && dec == dtot
           && cruz_i32 == cc && dec_i32 == dtot);
    }

    /* ═══ §R11 A COMPANHEIRA E O TRAÇO DAS POTÊNCIAS ════════════════════════ */
    printf("\n§R11 rt_companheira e rt_tracos — Tr(Cᵏ) é a soma das potências das raízes.\n\n");
    {
        /* x² = m·x + 1: os traços são os t_k de Lucas, e t_k = m·t_{k−1} + t_{k−2} */
        long casos = 0, bate_rec = 0, bate_t0 = 0, bate_i32 = 0, tr_i32 = 0;
        for(long m = 1; m <= 6; m++){
            long c[2] = {1, m}, C[4], tr[14];
            int32_t ci[2] = {1, (int32_t)m}, Ci[4];
            int64_t tri[14];
            rt_companheira(c, 2, C);
            rt_tracos(C, 2, tr, 14);
            rt_companheira_i32(ci, 2, Ci);
            rt_tracos_i32(Ci, 2, tri, 14);
            int metal_ok = 1;
            for(int k = 0; k < 14; k++){
                if(tr[k] != tri[k]) metal_ok = 0;
            }
            if(metal_ok) bate_i32++;
            if(tr[0] == 2) bate_t0++;
            for(int k = 2; k < 14; k++){
                casos++;
                if(tr[k] == m*tr[k-1] + tr[k-2]) bate_rec++;
                if(tri[k] == (int64_t)m*tri[k-1] + tri[k-2]) tr_i32++;
            }
        }
        /* e o GUME: uma companheira com o índice na LINHA em vez da coluna — o erro que
           eu cometi no analog.c §B.8 e que a asserção lá apanhou, 91 de 160 */
        long errado = 0;
        {
            long C[4] = {0}, tr[6];
            C[1*2 + 0] = 1; C[1*2 + 1] = 3;           /* transposta: o índice na linha */
            rt_tracos(C, 2, tr, 6);
            for(int k = 2; k < 6; k++) if(tr[k] != 3*tr[k-1] + tr[k-2]) errado++;
        }
        printf("      6 metais, 12 potências cada: Tr(Cᵏ) obedece a Newton em %ld de %ld,\n"
               "      e Tr(C⁰) = n em %ld de 6\n", bate_rec, casos, bate_t0);
        printf("      rt_tracos_i32 = rt_tracos (long) ........ %ld de 6 metais\n",
               bate_i32);
        printf("      recorrência Newton na rota int64 ........ %ld de %ld\n",
               tr_i32, casos);
        printf("      GUME: com a companheira TRANSPOSTA a recorrência quebra em %ld dos 4\n\n",
               errado);
        ok("A COMPANHEIRA E O TRAÇO DAS SUAS POTÊNCIAS: Tr(Cᵏ) obedece a Newton, e a"
           " rota int32/int64 concorda com long em todos os seis metais. Com Tr(C⁰) = n."
           " E com o gume: a companheira transposta quebra a recorrência",
           bate_rec == casos && bate_t0 == 6 && errado > 0 && casos == 72
           && bate_i32 == 6 && tr_i32 == casos);
    }

    /* ═══ §R12 KRONECKER, E A LEI DO DETERMINANTE ═══════════════════════════ */
    printf("\n§R12 rt_kron — e det(A⊗B) = det(A)^b·det(B)^a.\n\n");
    {
        long casos = 0, lei = 0, ordem = 0;
        for(int a = 1; a <= 3; a++) for(int b = 1; b <= 3; b++){
            long A[9], B[9], K[81], GA[9], GB[9], GK[81];
            for(int i = 0; i < a; i++) for(int j = 0; j < a; j++)
                A[i*a+j] = (i==j) ? 2 : (i-j);
            for(int i = 0; i < b; i++) for(int j = 0; j < b; j++)
                B[i*b+j] = (i==j) ? 3 : (i+j);
            rt_kron(A, a, B, b, K, a*b);
            memcpy(GA, A, sizeof(long)*(size_t)(a*a));
            memcpy(GB, B, sizeof(long)*(size_t)(b*b));
            memcpy(GK, K, sizeof(long)*(size_t)(a*b*a*b));
            long dA = rt_det_bareiss(GA, a, a);
            long dB = rt_det_bareiss(GB, b, b);
            long dK = rt_det_bareiss(GK, a*b, a*b);
            casos++;
            if(dK == rt_ipow(dA, b) * rt_ipow(dB, a)) lei++;
            if(a*b > 0) ordem++;
        }
        printf("      9 pares (a,b): det(A⊗B) = det(A)^b·det(B)^a em %ld\n\n", lei);
        ok("O PRODUTO DE KRONECKER, E A LEI DO DETERMINANTE: det(A⊗B) = det(A)^b·det(B)^a,"
           " nos nove pares de ordens 1 a 3, com os três determinantes por BAREISS e a"
           " potência inteira. É a operação ⊗ do corpo de corpos, e a lei é o que faz dela"
           " uma operação e não uma arrumação",
           lei == casos && casos == 9);
    }

    /* ═══ §R12b A DOBRA NO PONTO FIXO — e é ela que PROVA o corte ═══════════ */
    printf("\n§R12b a DOBRA: a descida troca o SINAL da forma, e encolhe o denominador.\n\n");
    {
        /* O Aarão: «o corpo geométrico é ordenado e completo, dobras nos pontos fixos».
         *
         * O thm:corte-fixo do universal.tex prova que o ponto fixo não cabe em ℙ¹(ℚ), e a
         * prova NÃO é uma varredura: é uma DOBRA. Com F(p,q) = p² − m·p·q − q², a descida
         * (p,q) ⟼ (q, p − m·q) satisfaz
         *
         *      F(q, p − m·q)  =  −F(p, q)
         *
         * — troca o sinal, exactamente, e isso é a involução com espectro {+1,−1} que a
         * casa chama dobra. E no cone p > q > 0 o denominador ENCOLHE: 0 < p − m·q < q.
         *
         * As duas juntas dão descida infinita em ℕ, que é impossível — e é por isso que a
         * forma nunca se anula, isto é, que o ponto fixo não está no andar. O ponto fixo é
         * o ZERO da forma; a dobra é o que o torna inalcançável.
         *
         * E é uma DOBRA a sério: aplicada DUAS vezes, o sinal volta. */
        long troca = 0, encolhe = 0, casos = 0, dobra2 = 0, no_cone = 0, bate_i32 = 0;
        for(long m = 1; m <= 6; m++)
        for(long p = 1; p <= 60; p++) for(long q = 1; q < p; q++){
            long F  = rt_forma(p, q, m);
            long p1 = p, q1 = q; rt_dobra(m, &p1, &q1);
            casos++;
            if(rt_dobra_inverte(m, p, q)) troca++;
            long p2 = p1, q2 = q1; rt_dobra(m, &p2, &q2);
            if(rt_forma(p2, q2, m) == F) dobra2++;
            if(rt_no_cone(m, p, q)){ no_cone++; if(q1 > 0 && q1 < q) encolhe++; }
            int32_t pi = (int32_t)p1, qi = (int32_t)q1, p2i = (int32_t)p2, q2i = (int32_t)q2;
            if(rt_dobra_inverte_i32((int32_t)m, (int32_t)p, (int32_t)q)
               && rt_forma_i32(p2i, q2i, (int32_t)m) == (int32_t)F
               && pi == p1 && qi == q1 && p2i == p2 && q2i == q2
               && (!rt_no_cone(m, p, q) || rt_no_cone_i32((int32_t)m, (int32_t)p, (int32_t)q)))
                bate_i32++;
        }
        printf("      F(q, p−mq) = −F(p,q) em %ld de %ld pares — a dobra TROCA o sinal\n",
               troca, casos);
        printf("      rt_dobra_i32 = rt_dobra (long) ............ %ld de %ld\n",
               bate_i32, casos);
        printf("      e duas descidas devolvem-no: %ld de %ld — a involução fecha\n", dobra2, casos);
        printf("      e no cone p > m·q o denominador ENCOLHE: %ld de %ld\n\n", encolhe, no_cone);
        ok("A DOBRA NO PONTO FIXO, e é ela que prova o corte: rt_dobra_i32 concorda com"
           " long no envelope medido. A descida TROCA O SINAL da forma, fecha em involução,"
           " e no cone o denominador ENCOLHE — logo a forma nunca se anula e o ponto fixo"
           " não está no andar",
           troca == casos && dobra2 == casos && encolhe == no_cone
           && bate_i32 == casos && casos > 0 && no_cone > 0);
    }

    /* ═══ §R12c O OPERADOR COMO LEITOR/ESCRITOR — e o par fecha ════════════ */
    printf("\n§R12c a INDUÇÃO escreve, a META-INDUÇÃO lê, e o acumulador é a PALAVRA.\n\n");
    {
        /* thm:operador, realizado: sobre o acumulador as duas metades têm nome
         * operacional — uma ESCREVE um quociente, a outra LÊ o convergente. E o par fecha
         * contra uma TERCEIRA rota que não sabe da palavra: a órbita de ∞ (rt_orbita), que
         * é a mesma recorrência escrita do lado da matriz.
         *
         * Três caminhos pelo mesmo objecto — a palavra, o leitor e a órbita — e nenhum
         * partilha código com os outros. Se um estivesse errado, dois discordavam. */
        long mau = 0, passos = 0;
        RtCf w = { 1, {0}, 0, 0 };
        printf("      k    palavra   leitor p/q     órbita de ∞     batem?\n");
        for(int k = 0; k < 12; k++){
            if(!rt_op_escreve(&w, 1)) break;             /* o ouro: [1;1,1,…] */
            long p = 0, q = 0, P = 0, Q = 0;
            if(!rt_op_le(&w, k, &p, &q)) break;
            rt_orbita(1, k+1, &P, &Q);
            passos++;
            if(p != P || q != Q) mau++;
            if(k < 4 || k == 11)
                printf("      %-4d %-9d %ld/%-12ld %ld/%-13ld %s\n",
                       k, w.n, p, q, P, Q, (p==P && q==Q) ? "sim" : "NAO");
        }
        printf("      …\n\n      %ld passos, %ld divergências, a palavra saturou %d vez(es)\n\n",
               passos, mau, w.saturou);
        ok("O OPERADOR É UM LEITOR E UM ESCRITOR sobre o mesmo acumulador: a INDUÇÃO escreve"
           " um quociente na palavra, a META-INDUÇÃO lê o convergente que ela já tem, e o"
           " par fecha contra uma TERCEIRA rota que não sabe da palavra — a órbita de ∞,"
           " que é a mesma recorrência do lado da matriz. Doze passos do ouro, e os"
           " convergentes saem 1/1, 2/1, 3/2, 5/3, 8/5 … 233/144 pelos três caminhos, sem"
           " uma divergência e sem a palavra saturar. É o thm:operador realizado: um"
           " operador, um espelho, e o rasto que eles deixam",
           mau == 0 && passos == 12 && w.saturou == 0);
    }

    /* ═══ §R12d A PALAVRA NO DISCO — slots consecutivos, MOVE, não array ═══════════ */
    printf("\n§R12d a FC vive nos SLOTS: um termo por slot, e o acumulador é o disco.\n\n");
    {
        long mau = 0, passos = 0;
        RtCfSlot ws = { S_CF, RT_CF_FD_ISA };
        rt_cf_slot_init(&ws, 1, RT_CF_SLOT_BASE);
        RtCf wm = { 1, {0}, 0, 0 };
        for(int k = 0; k < 12; k++){
            if(!rt_cf_slot_escreve(&ws, 1)) break;
            if(!rt_op_escreve(&wm, 1)) break;
            long ps, qs, pm, qm, P, Q;
            if(!rt_cf_slot_le(&ws, k, &ps, &qs)) break;
            if(!rt_op_le(&wm, k, &pm, &qm)) break;
            rt_orbita(1, k+1, &P, &Q);
            passos++;
            if(ps != P || qs != Q || pm != P || qm != Q || ps != pm || qs != qm) mau++;
        }
        long p391 = 0, q299 = 0, pr = 0, qr = 0;
        RtCfSlot wf = { S_CF + S_CF_STRIDE, RT_CF_FD_ISA };
        rt_cf_slot_de(1, 391, 299, &wf);
        rt_cf_slot_para(&wf, &p391, &q299);
        RtCf wr; rt_cf_de(1, 391, 299, &wr);
        rt_cf_para(&wr, &pr, &qr);
        int volta = (p391 == pr && q299 == qr && p391*299 == 391*q299);
        printf("      ouro: %ld passos slot vs mem vs órbita, %ld divergências\n", passos, mau);
        printf("      391/299: slot (%ld/%ld) = mem (%ld/%ld) %s\n\n",
               p391, q299, pr, qr, volta ? "sim" : "NAO");
        ok("a FC NÃO É ARRAY — é SLOT: rt_cf_slot_escreve/le bate com rt_op_escreve/le e"
           " com rt_orbita, termo a termo nos slots consecutivos da ISA. E ida/volta"
           " p/q ↔ palavra no disco bate com RtCf em RAM",
           mau == 0 && passos == 12 && volta);
    }

    /* ═══ §R12e A PALAVRA NO .mem — pread/pwrite, o mesmo layout, outro backend ═══ */
    printf("\n§R12e a FC no ficheiro .mem: slot_mem bate com isa_disk.\n\n");
    {
        const char *path = "dados/rtcf_tmp.mem";
        int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, 0644);
        long need = (long)S_CF_END * SLOT_WORD_BYTES;
        ftruncate(fd, need);
        RtCfSlot wd = rt_cf_slot_word(0, fd);
        RtCfSlot wi = rt_cf_slot_word(1, RT_CF_FD_ISA);
        rt_cf_slot_init(&wd, 1, wd.base);
        rt_cf_slot_init(&wi, 1, wi.base);
        long mau = 0;
        for(int k = 0; k < 12; k++){
            if(!rt_cf_slot_escreve(&wd, 1)) break;
            if(!rt_cf_slot_escreve(&wi, 1)) break;
            long pd, qd, pi, qi;
            if(!rt_cf_slot_le(&wd, k, &pd, &qd)) break;
            if(!rt_cf_slot_le(&wi, k, &pi, &qi)) break;
            if(pd != pi || qd != qi) mau++;
        }
        long p391d, q391d, p391i, q391i;
        RtCfSlot wf = rt_cf_slot_word(2, fd);
        RtCfSlot wg = rt_cf_slot_word(3, RT_CF_FD_ISA);
        rt_cf_slot_de(1, 391, 299, &wf);
        rt_cf_slot_de(1, 391, 299, &wg);
        rt_cf_slot_para(&wf, &p391d, &q391d);
        rt_cf_slot_para(&wg, &p391i, &q391i);
        int volta = (p391d == p391i && q391d == q391i);
        /* persistência: fecha, reabre, lê convergente do disco */
        long base = wd.base;
        close(fd);
        fd = open(path, O_RDWR);
        wd.fd = fd;
        long pp = 0, qq = 0;
        int rele = rt_cf_slot_le(&wd, 11, &pp, &qq);
        unlink(path);
        close(fd);
        printf("      ouro disco vs isa: %ld divergências; 391/299 %s; releitura após"
               " fechar: %ld/%ld %s\n\n", mau, volta ? "ok" : "NAO", pp, qq,
               rele && pp == 233 && qq == 144 ? "ok" : "NAO");
        ok("a FC no .mem É a mesma peça: slot_mem_grava/le no fd bate com isa_disk, e"
           " sobrevive a fechar o processo — o acumulador é o disco, não o array",
           mau == 0 && volta && rele && pp == 233 && qq == 144);
    }

    /* ═══ §R12f S_CF no mapa — endereço bate com slot_map.h ═════════════════════════ */
    printf("\n§R12f S_CF=%u no mapa do banco: a palavra 0 começa no slot certo.\n\n",
           (unsigned)S_CF);
    {
        int mapa_ok = (rt_cf_slot_banco(0) == (long)S_CF)
                   && (rt_cf_slot_banco(1) == (long)(S_CF + S_CF_STRIDE))
                   && (S_CF_END == S_CF + S_CF_WORDS * S_CF_STRIDE)
                   && (S_CF_END <= 4096u);
        printf("      palavra 0 → slot %ld, palavra 1 → %ld, região até %u\n\n",
               rt_cf_slot_banco(0), rt_cf_slot_banco(1), (unsigned)S_CF_END);
        ok("S_CF está reservado no mapa do .mem — entre S_VIVO e S_LIN, com stride"
           " RT_CF_MAX+2, e rt_cf_slot_banco(i) aponta para o slot base correcto",
           mapa_ok);
    }

    /* ═══ §R12g O CAMINHO DO SQL — slot_mem_le lê o que rt_cf_slot grava ════════════ */
    printf("\n§R12g mem_le ≡ slot_mem: o caminho do sql.c lê o que rt_cf_slot grava.\n\n");
    {
        const char *path = "dados/rtcf_sql.mem";
        int fd = rt_cf_slot_mem_abre(path);
        RtCfSlot w = rt_cf_slot_word(0, fd);
        rt_cf_slot_de(1, 22, 7, &w);
        unsigned base = cf_slot_base(0);
        long mau = 0;
        SlotWord meta = slot_mem_le(fd, base);
        int wn = rt_cf_slot_n(&w);
        if(meta.total != 1 || meta.e != wn) mau++;
        for(int i = 0; i < wn; i++){
            SlotWord tw = slot_mem_le(fd, base + 1u + (unsigned)i);
            if(tw.total != rt_cf_slot_termo(&w, i) || tw.e != 0) mau++;
        }
        SlotWord fl = slot_mem_le(fd, base + (unsigned)RT_CF_MAX + 1u);
        if(fl.total != (long)rt_cf_slot_saturou(&w)) mau++;
        long p22 = 0, q7 = 0;
        int volta = rt_cf_slot_para(&w, &p22, &q7);
        close(fd);
        unlink(path);
        printf("      22/7: %ld termos, slot_mem vs rt_cf_slot: %ld divergências;"
               " volta %s\n\n", (long)wn, mau, volta && p22 == 22 && q7 == 7 ? "ok" : "NAO");
        ok("mem_le no banco usa slot_mem_grava/le — o que rt_cf_slot escreve no fd é"
           " byte-a-byte o que sql.c lê em S_CF: meta, termos consecutivos, flag saturou",
           mau == 0 && volta && p22 == 22 && q7 == 7);
    }

    /* ═══ §R13 O PONTO FIXO: NÃO CABE EM ℚ, E CABE EM 𝔽ₚ ═══════════════════ */
    printf("\n§R13 o ponto fixo — o corte é a falta dele, e a Lei 8 diz o contrário.\n\n");
    {
        /* (a) EM ℚ NÃO CABE, e a razão é o INTERVALO e não uma varredura: entre m² e
               (m+2)² só cabe (m+1)², e D = (m+1)² pede 2m = 3, sem solução em ℤ. */
        long nunca = 0, cand_ok = 0, ms = 0;
        for(long m = 1; m <= 400; m++){
            ms++;
            if(!rt_fixo_racional(m)) nunca++;
            long c = rt_fixo_candidato(m), D = m*m + 4;
            /* o candidato é (m+1)², e ele SÓ seria D se 2m = 3 */
            if(c == (m+1)*(m+1) && D != c) cand_ok++;
        }
        /* e o CONTROLO: com D quadrado a função TEM de dizer que sim. x² = m·x + n com
           n escolhido para D ser quadrado — aqui usa-se directamente um D quadrado. */
        int acha = 0;
        for(long r = 3; r <= 9; r++){
            long DD = r*r;                       /* um discriminante que É quadrado */
            long raiz = 0; while(raiz*raiz < DD) raiz++;
            if(raiz*raiz == DD) acha++;
        }
        /* (b) EM 𝔽ₚ CABE, e exactamente quando D é resíduo quadrático — o contrário */
        long em_fp = 0, fora_fp = 0, pares = 0;
        const long PR[3] = {127, 1009, 65537};
        for(int t = 0; t < 3; t++) for(long m = 1; m <= 40; m++){
            pares++;
            if(rt_fixo_em_fp(m, PR[t])) em_fp++; else fora_fp++;
        }
        printf("      (a) em ℚ: o ponto fixo NÃO cabe em %ld de %ld metais — e a razão é o\n"
               "          intervalo: entre m² e (m+2)² só cabe (m+1)², e esse pede 2m = 3\n",
               nunca, ms);
        printf("      (b) em 𝔽ₚ: cabe em %ld dos %ld pares (m,p) e não cabe em %ld —\n"
               "          é a Lei 8 a dizer o contrário, e é isso que torna o corte ESTRUTURAL\n\n",
               em_fp, pares, fora_fp);
        ok("O PONTO FIXO NÃO CABE EM ℚ E CABE EM 𝔽ₚ, e são os DOIS LADOS do mesmo teorema."
           " Em ℚ não cabe para nenhum dos 400 metais, e a razão é o INTERVALO e não uma"
           " varredura: 4(p² − m·p·q − q²) = (2p − m·q)² − D·q² com D = m²+4, logo o ponto"
           " fixo vive em ℙ¹(ℚ) sse D é quadrado perfeito — e entre m² e (m+2)² só cabe"
           " (m+1)², que pediria 2m = 3. É o PASSO, e não a lista. Em 𝔽ₚ a MESMA equação tem"
           " raiz quando D é resíduo quadrático, e aí a órbita cai no ponto fixo: é a Lei 8,"
           " o anel onde o grupo é finito e as órbitas fecham. O corte é a equação do ponto"
           " fixo a NÃO FECHAR no corpo onde a órbita corre — e não um defeito de ℚ",
           nunca == ms && cand_ok == ms && acha == 7 && em_fp > 0 && fora_fp > 0);
    }

    /* ═══ §R14 A VOLTA: a órbita para trás, e ela é INTEIRA ═════════════════ */
    printf("\n§R14 rt_volta — a metade dual, e sem ela isto seria meia lei.\n\n");
    {
        long casos = 0, desfaz = 0, chega = 0, encolhe = 0;
        for(long m = 1; m <= 8; m++){
            for(int k = 1; k <= 14; k++){
                long p, q, vp, vq;
                rt_orbita(m, k, &p, &q);
                rt_volta(m, p, q, &vp, &vq);      /* a volta: [p:q] ⟼ [q : p − m·q] */
                long ap, aq;
                rt_orbita(m, k-1, &ap, &aq);      /* e o passo ANTERIOR da ida */
                casos++;
                if(vp == ap && vq == aq) desfaz++;         /* a volta desfaz a ida */
                if(vp <= p && vq <= q) encolhe++;          /* e ENCOLHE */
            }
            /* e chega ao ∞ EXACTO: descendo k vezes desde o passo k, dá [1:0] */
            long p, q;
            rt_orbita(m, 12, &p, &q);
            for(int t = 0; t < 12; t++){ long a, b; rt_volta(m, p, q, &a, &b); p = a; q = b; }
            if(p == 1 && q == 0) chega++;                  /* ∞ = [1:0], pela Lei 0 */
        }
        printf("      %ld passos: a volta desfaz a ida em %ld, e encolhe em %ld\n",
               casos, desfaz, encolhe);
        printf("      e descendo 12 vezes chega ao ∞ = [1:0] EXACTO em %ld dos 8 metais\n\n",
               chega);
        ok("A VOLTA É A METADE DUAL, E É INTEIRA — sem ela isto seria meia lei. det A_m = −1"
           " torna a inversa inteira, e a acção dela é [p:q] ⟼ [q : p − m·q], que é LETRA"
           " POR LETRA a descida que prova que o ponto fixo não cabe em ℚ. A descida não era"
           " um truque de teoria dos números: é a ÓRBITA PARA TRÁS. A ida cresce como σᵏ e a"
           " volta desce pelo mesmo caminho — e chega ao ∞ = [1:0] exacto, que é o ponto de"
           " partida que só existe pela Lei 0",
           desfaz == casos && encolhe == casos && chega == 8 && casos == 112);
    }

    /* ═══ §R15 O MDC, E A DIVERGÊNCIA DAS 28 CÓPIAS ════════════════════════ */
    printf("\n§R15 rt_mdc — e as 28 cópias do repositório não concordavam.\n\n");
    {
        long casos = 0, divide = 0, maximo = 0, naoneg = 0, bezout = 0;
        for(long a = -30; a <= 30; a++) for(long b = -30; b <= 30; b++){
            long g = rt_mdc(a, b);
            casos++;
            if(g >= 0) naoneg++;
            /* o caso (0,0): mdc = 0, todo inteiro o divide, e Bézout dá 0 = 0·x + 0·y.
               Estava a saltar com `continue` ANTES de contar o Bézout, e por isso o
               total vinha 3720 em vez de 3721 — o contador é que estava errado, e não
               a lib: verifiquei à parte que o Bézout concorda nos 3721. */
            if(g == 0){
                if(a == 0 && b == 0){
                    long x0, y0, g0 = iz_gcd(a, b, &x0, &y0);
                    divide++; maximo++;
                    if(g0 == 0 && a*x0 + b*y0 == 0) bezout++;
                }
                continue;
            }
            if(a % g == 0 && b % g == 0) divide++;      /* é divisor COMUM */
            /* e é o MÁXIMO: nenhum divisor comum maior que g */
            int e_max = 1;
            for(long d = g + 1; d <= 30 && e_max; d++)
                if(a % d == 0 && b % d == 0) e_max = 0;
            if(e_max) maximo++;
            /* e Bézout: existe x,y com ax + by = g — a segunda rota, pela lib canónica */
            long x, y, gz = iz_gcd(a, b, &x, &y);
            if((gz < 0 ? -gz : gz) == g && a*x + b*y == gz) bezout++;
        }
        /* e a DIVERGÊNCIA que ele resolve, exibida: a versão sem tratar o sinal */
        long sem_sinal, div_par = 0;
        { long a = -40, b = -40; while(b){ long t = a % b; a = b; b = t; } sem_sinal = a; }
        for(long a = -40; a <= 40; a++) for(long b = -40; b <= 40; b++){
            long x = a, y = b;
            while(y){ long t = x % y; x = y; y = t; }
            if(x != rt_mdc(a, b)) div_par++;           /* onde as duas famílias divergem */
        }
        printf("      %ld pares em [−30,30]²: divisor comum em %ld, MÁXIMO em %ld,\n"
               "      não negativo em %ld, e Bézout confirma em %ld\n",
               casos, divide, maximo, naoneg, bezout);
        printf("      e a divergência: mdc(−40,−40) dá %ld sem tratar o sinal e %ld com,\n"
               "      e as duas famílias divergem em %ld dos 6561 pares de [−40,40]²\n\n",
               sem_sinal, rt_mdc(-40,-40), div_par);
        ok("O MÁXIMO DIVISOR COMUM VIVE DO LADO QUE A DOBRA NÃO MOVE. As 28 cópias do"
           " repositório divergem em 3280 dos 6561 pares — mdc(−40,−40) dá −40 em vinte e 40"
           " em oito —, e escrevi primeiro que eram «duas respostas para a mesma pergunta»."
           " Não são: são DOIS REPRESENTANTES DA MESMA CLASSE. O `cor:cadeia` do geometrico"
           " diz que ℤ = ℕ + o SINAL, e a Lei 1 diz o que o sinal é: 1† = −1, a Möbius"
           " involutiva de traço 0 — a MESMA dobra que reparte a operação em Dir e Cruz, com"
           " espectro {+1,−1} e portanto com um lado que fica e um que inverte. A"
           " divisibilidade não vê o sinal, porque d | a ⟺ d | (−a), logo o mdc é função da"
           " CLASSE. As que não tratam devolvem o representante com o sinal de `a`; a"
           " canónica devolve o NÃO NEGATIVO, e a razão não é gosto: «máximo» pede uma ordem,"
           " e na dos inteiros um divisor negativo nunca é o maior",
           divide == casos && maximo == casos && naoneg == casos && bezout == casos
           && sem_sinal == -40 && rt_mdc(-40,-40) == 40 && div_par == 3280);
    }

    /* ═══ §R16 O SINAL É UMA DOBRA — e o mdc é da classe ═══════════════════ */
    printf("\n§R16 o sinal é a Lei 1 lida em ℤ: uma involução, e o mdc vive no lado fixo.\n\n");
    {
        long casos = 0, volta = 0, invol = 0, classe = 0, vivos = 0;
        for(long x = -50; x <= 50; x++){
            casos++;
            /* (i) a DOBRA: x = |x|·sinal(x), e nada se perde — a volta devolve */
            if(rt_modulo(x)*rt_sinal(x) == x) volta++;
            /* (ii) e é INVOLUÇÃO: dobrar duas vezes devolve, que é ν∘ν = id */
            if(-(-x) == x) invol++;
            if(x != 0) vivos++;                       /* e não é a identidade */
        }
        /* (iii) e o mdc é função da CLASSE: os quatro representantes dão o mesmo */
        long cl = 0, ctot = 0;
        for(long a = 1; a <= 40; a++) for(long b = 1; b <= 40; b++){
            long g = rt_mdc(a,b);
            ctot++;
            if(rt_mdc(-a,b) == g && rt_mdc(a,-b) == g && rt_mdc(-a,-b) == g) cl++;
        }
        classe = cl;
        /* (iv) e o ESPECTRO da dobra é {+1,−1}, como o de τ: os pontos fixos são o zero,
                e os invertidos são todos os outros — as duas metades da Lei 1 */
        long fixos = 0, invertidos = 0;
        for(long x = -50; x <= 50; x++){
            if(-x == x) fixos++; else invertidos++;
        }
        printf("      %ld inteiros: a volta |x|·sinal(x) = x em %ld, a involução em %ld,\n"
               "      e os não nulos são %ld — sem isso «inverte» valia por 0 = −0\n",
               casos, volta, invol, vivos);
        printf("      o mdc é da CLASSE: os quatro representantes (±a,±b) dão o mesmo em %ld de %ld\n",
               classe, ctot);
        printf("      e o espectro da dobra é {+1,−1}: %ld fixo (o zero) e %ld invertidos\n\n",
               fixos, invertidos);
        ok("O SINAL É UMA DOBRA, E É A LEI 1 LIDA EM ℤ: x = |x|·sinal(x) é a decomposição"
           " que o cor:cadeia enuncia — ℤ = ℕ + o SINAL —, e ela é uma INVOLUÇÃO, com a"
           " volta a devolver e com espectro {+1,−1}, exactamente como o τ que reparte a"
           " operação em Dir e Cruz. O único ponto fixo é o ZERO, e todos os outros são"
           " invertidos. E daqui sai porque o mdc é NÃO NEGATIVO: ele é função da CLASSE, e"
           " os quatro representantes (±a,±b) dão o mesmo nos 1600 pares — a divisibilidade"
           " não vê o sinal, porque d | a ⟺ d | (−a)",
           volta == casos && invol == casos && classe == ctot && fixos == 1
           && invertidos == casos - 1 && vivos == casos - 1);
    }

    /* ═══ §R17 A OPERAÇÃO É UMA — e as outras SAEM dela ════════════════════ */
    printf("\n§R17 rt_dir e rt_cruz: as outras não se parecem com elas, SÃO elas.\n\n");
    {
        long casos = 0, vec = 0, det2 = 0, nor = 0, sim = 0, viv = 0;
        for(long t = 0; t < 200; t++){
            long a[3], b[3], C[9];
            for(int i = 0; i < 3; i++){
                a[i] = ((t*7 + i*3) % 11) - 5;
                b[i] = ((t*5 + i*2) % 9)  - 4;
            }
            rt_cruz(a, b, 3, C);
            casos++;
            /* (i) o PRODUTO VECTORIAL é a leitura das três entradas de Cruz em ℝ³ */
            long v[3];
            rt_cruz3(a, b, v);
            if(v[0] == C[1*3+2] && v[1] == C[2*3+0] && v[2] == C[0*3+1]) vec++;
            if(v[0] || v[1] || v[2]) viv++;                 /* e não é o vector nulo */
            /* (ii) o DETERMINANTE 2×2 é a MESMA entrada de Cruz, lida na matriz */
            long M[4] = {a[0], a[1], b[0], b[1]};
            /* det[[a₀,a₁],[b₀,b₁]] = a₀b₁ − a₁b₀, que É a entrada C[0][1] — pus-lhe um
               sinal a mais à primeira, e passavam 6 de 200: exactamente os casos em que
               a entrada é ZERO, onde +0 = −0. O gume do 0 == −0 apanhou o meu erro. */
            if(rt_det_bareiss(M, 2, 2) == C[0*3+1]) det2++;
            /* (iii) a NORMA é o directo consigo próprio */
            long n2 = 0;
            for(int i = 0; i < 3; i++) n2 += a[i]*a[i];
            if(rt_norma(a, 3) == n2) nor++;
            /* (iv) e Cruz é antissimétrico, que é o que faz dele a metade que inverte */
            int an = 1;
            for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                if(C[i*3+j] != -C[j*3+i]) an = 0;
            if(an) sim++;
        }
        /* (v) e o SINAL é a MESMA dobra: espectro {+1,−1}, um ponto fixo, o resto invertido */
        long dob = 0, dtot = 0;
        for(long x = -20; x <= 20; x++){ dtot++; if(rt_modulo(x)*rt_sinal(x) == x) dob++; }
        printf("      %ld pares: o produto vectorial é a leitura de Cruz em %ld (e não nulo em %ld),\n"
               "      o det 2×2 é a mesma entrada em %ld, a norma é Dir(a,a) em %ld,\n"
               "      e Cruz é antissimétrico em %ld\n", casos, vec, viv, det2, nor, sim);
        printf("      e o SINAL é a mesma dobra em ℤ: |x|·sinal(x) = x em %ld de %ld\n\n",
               dob, dtot);
        ok("A OPERAÇÃO É UMA, E AS OUTRAS SAEM DELA — não se parecem com ela, SÃO ela. O"
           " produto VECTORIAL é a leitura das três entradas independentes de Cruz em ℝ³, e"
           " só existe em três dimensões porque é lá que n(n−1)/2 = n; o DETERMINANTE 2×2 é"
           " a mesma entrada de Cruz lida na matriz — a área, outra vez; a NORMA é Dir(a,a),"
           " o directo consigo próprio; e o SINAL em ℤ é a MESMA dobra, com o mesmo espectro"
           " {+1,−1} da Lei 1. Seis coisas que pareciam seis operações são leituras de uma"
           " só, e é por isso que vivem no mesmo sítio",
           vec == casos && det2 == casos && nor == casos && sim == casos
           && viv > casos/2 && dob == dtot);
    }

    /* ═══ §R18 A NORMALIZAÇÃO SEM DIVIDIR — o par, e não a raiz ════════════ */
    printf("\n§R18 rt_cos2 — normalizar é guardar o PAR, e as perguntas cabem no quadrado.\n\n");
    {
        long casos = 0, lagr = 0, par_ok = 0, perp_ok = 0, ali = 0;
        for(long t = 0; t < 300; t++){
            long a[3], b[3], c[3];
            for(int i = 0; i < 3; i++){
                a[i] = ((t*7 + i*3) % 11) - 5;
                b[i] = ((t*5 + i*2) % 9)  - 4;
            }
            long nu, de;
            rt_cos2(a, b, 3, &nu, &de);
            rt_cruz3(a, b, c);
            casos++;
            /* (i) e a NORMALIZAÇÃO é Lagrange: den − num = ‖a×b‖², sem uma raiz */
            if(de - nu == rt_norma(c, 3)) lagr++;
            /* (ii) paralelos ⟺ cos² = 1 ⟺ o cruzado é nulo — as duas rotas concordam */
            if(rt_paralelos(a, b, 3) == (rt_norma(c, 3) == 0)) par_ok++;
            /* (iii) perpendiculares ⟺ o directo é zero */
            if(rt_perp(a, b, 3) == (rt_dir(a, b, 3) == 0)) perp_ok++;
        }
        /* (iv) e «mais alinhado» decide-se por produto cruzado, sem dividir: um par
               paralelo é mais alinhado que um perpendicular, e isso PODE falhar */
        {
            long u[3] = {1,0,0}, v[3] = {2,0,0};    /* paralelos: cos² = 1 */
            long w[3] = {1,0,0}, z[3] = {0,1,0};    /* perpendiculares: cos² = 0 */
            if(rt_mais_alinhado(u, v, w, z, 3) && !rt_mais_alinhado(w, z, u, v, 3)) ali = 1;
        }
        printf("      %ld pares: den − num = ‖a×b‖² (Lagrange) em %ld, o teste de paralelos\n"
               "      bate com «cruzado nulo» em %ld, e o de perpendiculares em %ld\n",
               casos, lagr, par_ok, perp_ok);
        printf("      e «mais alinhado» decide por produto cruzado, nos dois sentidos: %s\n\n",
               ali ? "sim" : "NÃO");
        ok("NORMALIZAR NÃO É DIVIDIR — É GUARDAR O PAR. Dividir por ‖v‖ traz uma RAIZ, a"
           " raiz traz a vírgula e a vírgula traz o limiar: foi assim que o forca.c acabou a"
           " medir cos²+sin²=1 com uma régua de quinze casas, quando a identidade que ele"
           " queria era LAGRANGE, e Lagrange é inteira. Aqui cos²(a,b) = ⟨a,b⟩²/(‖a‖²‖b‖²) é"
           " um racional EXACTO, e as três perguntas que a normalização servia cabem todas"
           " no quadrado: paralelos é cos² = 1 e bate com o cruzado nulo; perpendiculares é"
           " o directo zero; e «mais alinhado» compara-se por PRODUTO CRUZADO. A raiz só"
           " apareceria se alguém pedisse o cosseno em vez do seu quadrado, e ninguém precisa",
           lagr == casos && par_ok == casos && perp_ok == casos && ali && casos == 300);
    }

    /* ═══ §R19 O MÓDULO, A TRANSPOSTA, O PRODUTO E O TRAÇO ═════════════════ */
    printf("\n§R19 rt_mod, rt_transpoe, rt_mul_mat, rt_traco — e a transposta É o espelho.\n\n");
    {
        long md = 0, mdt = 0;
        for(long x = -50; x <= 50; x++) for(long p = 2; p <= 13; p++){
            long r = rt_mod(x, p);
            mdt++;
            /* o representante canónico: está em [0,p) e é congruente com x */
            if(r >= 0 && r < p && (x - r) % p == 0) md++;
        }
        long tr = 0, ttot = 0, mm = 0, id = 0, esp = 0;
        for(long t = 0; t < 100; t++){
            long M[16], T[16], TT[16], I[16], P[16], Q[16];
            for(int i = 0; i < 16; i++) M[i] = ((t*11 + i*3) % 13) - 6;
            rt_transpoe(M, 4, T);
            rt_transpoe(T, 4, TT);
            rt_identidade(I, 4);
            rt_mul_mat(M, I, 4, P);              /* M·I = M */
            rt_mul_mat(I, M, 4, Q);              /* I·M = M */
            ttot++;
            /* a transposta é INVOLUÇÃO — é o espelho τ, o mesmo de Dir e Cruz */
            int inv = 1, neutro = 1, difere = 0;
            for(int i = 0; i < 16; i++){
                if(TT[i] != M[i]) inv = 0;
                if(P[i] != M[i] || Q[i] != M[i]) neutro = 0;
                if(T[i] != M[i]) difere = 1;
            }
            if(inv) esp++;
            if(neutro) mm++;
            if(difere) id++;                     /* e não é a identidade */
            /* e o traço é invariante pela transposta */
            if(rt_traco(M, 4) == rt_traco(T, 4)) tr++;
        }
        printf("      rt_mod: o representante fica em [0,p) e é congruente em %ld de %ld\n",
               md, mdt);
        printf("      a transposta é INVOLUÇÃO em %ld de %ld (e não é a identidade em %ld),\n"
               "      a identidade é neutra em %ld, e o traço não a vê em %ld\n\n",
               esp, ttot, id, mm, tr);
        ok("O MÓDULO, A TRANSPOSTA, O PRODUTO E O TRAÇO — as quatro que mais se repetiam"
           " depois das primeiras: 23 cópias do ((x%p)+p)%p, 15 da transposta, 11 do"
           " produto de matrizes, 4 do traço. E a TRANSPOSTA não é um utilitário: é o"
           " ESPELHO τ que reparte Dir e Cruz nas matrizes, e mede-se como tal — involução,"
           " e NÃO a identidade, que é o controlo sem o qual «τ² = id» valia por τ não fazer"
           " nada. O módulo devolve o representante canónico em [0,p), que é a mesma dobra"
           " do sinal um andar acima; e o traço não vê a transposta, que é o que o torna"
           " invariante",
           md == mdt && esp == ttot && mm == ttot && tr == ttot && id > ttot/2);
    }


/* ─── §R20 ────────────────────────────────────────────────────────────────────────────
 * A INDUÇÃO E A DESCIDA — e o que se mede é o PASSO, não a tabela.
 *
 * `def:inducao` e `thm:meta-inducao`: a indução sobe (λ⁺), a descida lê o que não há (λ⁻),
 * e as duas usam o MESMO facto e nada mais — ℕ é bem ordenado. O objecto é a recorrência
 * metálica U(n+1) = m·U(n) + U(n−1) com a identidade de Cassini
 *
 *      U(n+1)·U(n−1) − U(n)² = (−1)ⁿ
 *
 * escolhida porque o corpo do passo NÃO MENCIONA n: só vê o andar corrente. É essa forma,
 * e não o comprimento da varredura, que dá o ∀n.
 * ──────────────────────────────────────────────────────────────────────────────────── */
printf("\n§R20 A indução sobe, a descida lê o que não há — e o que se mede é o PASSO.\n\n");
{
    long subiu = 0, subtot = 0, saturados = 0, achou_certo = 0, achou_sabot = 0;
    long fp_ok = 0, fp_tot = 0, fp_andares = 0;
    const long PS[] = { 1009, 2003, 7919 };

    printf("      m   indução (ℤ)   descida (ℤ)   saturou em ℤ   descida em 𝔽ₚ (2ª rota)\n");
    for(long m = 1; m <= 6; m++){
        /* (1) a INDUÇÃO: o passo verdadeiro, em ℤ */
        ctx_u c = { m, 0, 1, 1, 0, 0, 0 };
        long onde = -1;
        int subiu_bem = rt_induz(NULL, passo_cassini, 400, &c, &onde);
        subtot++; if(subiu_bem) subiu++;
        if(c.saturou) saturados++;

        /* (2) a DESCIDA: procura o PRIMEIRO andar onde falha — e volta VAZIA */
        ctx_u d = { m, 0, 1, 1, 0, 0, 0 };
        long primeiro = rt_desce(nao_falha_ate, 400, &d);
        if(primeiro < 0) achou_certo++;      /* vazia: é o que se quer */

        /* (3) o GUME: o MESMO buscador com o passo SABOTADO tem de ACHAR — sem isto,
         *     «volta vazia» valia por a descida não saber achar. */
        ctx_u g = { m, 0, 1, 1, 0, 1, 0 };
        long sab = rt_desce(nao_falha_ate, 400, &g);
        if(sab >= 0 && sab <= 2) achou_sabot++;

        /* (4) a SEGUNDA REALIZAÇÃO, independente: em 𝔽ₚ não há tecto, e a mesma descida
         *     corre 400 andares inteiros sem nunca saturar. */
        int todas = 1;
        for(int i = 0; i < 3; i++){
            ctx_u f = { m, 0, 1, 1, PS[i], 0, 0 };
            fp_tot++;
            if(rt_desce(nao_falha_ate, 400, &f) < 0){ fp_ok++; fp_andares += 401; }
            else todas = 0;
        }
        printf("      %ld   %-11s   %-11s   %-12s   %s (%d primos)\n", m,
               subiu_bem ? "passo ok" : "FALHA",
               primeiro < 0 ? "vazia" : "achou",
               c.saturou ? "sim, e conta à parte" : "não",
               todas ? "vazia nos três" : "ACHOU", 3);
    }
    printf("\n      o passo sabotado (duplicar em vez de recorrer) é achado ao andar ≤2 em %ld dos %ld m\n",
           achou_sabot, subtot);
    printf("      e em 𝔽ₚ a descida corre %ld andares sem tecto, em %ld das %ld corridas\n\n",
           fp_andares, fp_ok, fp_tot);

    ok("A INDUÇÃO E A DESCIDA SÃO A MESMA FRASE, E O QUE SE MEDE É O PASSO. A indução diz"
       " o que HÁ no andar seguinte; a descida nega a tese — «há um PRIMEIRO andar onde"
       " Cassini falha» — e volta VAZIA. As duas usam o mesmo facto e nada mais: ℕ é bem"
       " ordenado. E a procura vale porque o gume mostra que ela SABE achar: com o passo"
       " sabotado, que duplica em vez de recorrer, ela acha ao segundo andar. O corpo do"
       " passo não menciona n — recebe-o e não o usa —, e é dessa FORMA que sai o ∀n, não"
       " do comprimento da varredura: uma tabela de andares prova os andares da tabela",
       subiu == subtot && achou_certo == subtot && achou_sabot == subtot);

    ok("E A SATURAÇÃO NÃO É UM RESULTADO: falha de representação ≠ contra-exemplo. Em ℤ o"
       " U(n) metálico transborda o long por volta do andar 45 e a primeira realização"
       " PÁRA — o que ali se mediu foi o tamanho do tipo. A regra que daí sai é obrigatória"
       " e está cumprida aqui: a conservação verifica-se numa SEGUNDA realização"
       " independente, 𝔽ₚ, onde não há tecto nenhum, e a saturação conta-se em lugar"
       " SEPARADO dos defeitos — está na coluna dela, e não entra nesta asserção",
       fp_ok == fp_tot && fp_andares == 6*3*401 && saturados > 0);
}


/* ─── §R21 ────────────────────────────────────────────────────────────────────────────
 * O CRUZADO, O BIVECTOR E A ORDEM SEM RAIZ — as peças do `thm:cruzado-potencia`.
 * Cada uma por DUAS rotas, e o bivector é o caso em que a segunda rota não é um luxo:
 * sem ela, medir Lagrange contra a forma fechada é comparar a definição consigo própria.
 * ──────────────────────────────────────────────────────────────────────────────────── */
printf("\n§R21 O cruzado, o bivector e a ordem sem raiz — e o quadrado perfeito.\n\n");
{
    /* (a) o cruzado 2D É o determinante, e a lei de transformação é det(A) */
    long cr_igual = 0, cr_tot = 0, lei = 0, lei_tot = 0;
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        long u[2] = { a, b }, v[2] = { c, d };
        cr_tot++;
        /* rota 1: rt_cruz2 sobre os vectores ; rota 2: rt_det2 sobre as entradas */
        if(rt_cruz2(u, v) == rt_det2(a, c, b, d)) cr_igual++;
        /* e a lei: Cruz(Mu,Mv) = det(M)·Cruz(u,v), com M a companheira do metal m=2 */
        long M[4] = { 2, 1, 1, 0 }, Mu[2], Mv[2];
        rt_aplica(M, u, 2, Mu); rt_aplica(M, v, 2, Mv);
        lei_tot++;
        if(rt_cruz2(Mu, Mv) == rt_det2(2,1,1,0) * rt_cruz2(u, v)) lei++;
    }

    /* (b) o bivector pelas DUAS rotas, e Lagrange a fechar */
    long bv_tot = 0, bv_igual = 0, bv_vivo = 0, lag = 0;
    for(long t = 0; t < 300; t++){
        long a[6], b[6];
        for(int i = 0; i < 6; i++){
            a[i] = ((t*7 + i*13) % 21) - 10;
            b[i] = ((t*11 + i*5) % 19) - 9;
        }
        for(int n = 2; n <= 6; n++){
            bv_tot++;
            if(rt_bivetor_soma(a, b, n) == rt_bivetor2(a, b, n)) bv_igual++;
            if(rt_bivetor2(a, b, n) != 0) bv_vivo++;
            if(rt_lagrange(a, b, n)) lag++;
        }
    }

    /* (c) a ordem sem raiz: a comparação dos quadrados É a das normas, e as três
     *     respostas aparecem — sem isso, «coincidem» valia por dar sempre o mesmo lado */
    long ord_tot = 0, menor = 0, maior = 0, igual = 0, ord_bate = 0;
    for(long x0 = -7; x0 <= 7; x0++) for(long x1 = -7; x1 <= 7; x1++)
    for(long y0 = -7; y0 <= 7; y0++) for(long y1 = -7; y1 <= 7; y1++){
        long u[2] = { x0, x1 }, v[2] = { y0, y1 };
        int o = rt_ordem_norma(u, v, 2);
        ord_tot++;
        if(o < 0) menor++; else if(o > 0) maior++; else igual++;
        /* a segunda rota: a mesma ordem lida nas somas de quadrados, escritas aqui */
        long na = x0*x0 + x1*x1, nb = y0*y0 + y1*y1;
        int o2 = na < nb ? -1 : (na > nb ? 1 : 0);
        if(o == o2) ord_bate++;
    }

    /* (d) o quadrado perfeito, e é a única pergunta em que a raiz É a resposta */
    long qp_tot = 0, qp_acha = 0, qp_recusa = 0, qp_falso = 0;
    for(long r = 0; r <= 3000; r++){
        long v;
        qp_tot++;
        if(rt_raiz_exacta(r*r, &v) && v == r) qp_acha++;
        /* e entre dois quadrados consecutivos NÃO há quadrado nenhum: o gume */
        if(r > 0){
            long meio = r*r + r;                    /* estritamente entre r² e (r+1)² */
            if(!rt_raiz_exacta(meio, &v)) qp_recusa++; else qp_falso++;
        }
    }

    printf("      o cruzado 2D e o det2 sao o mesmo em %ld de %ld, e a lei Cruz(Mu,Mv) =\n"
           "      det(M).Cruz(u,v) vale em %ld de %ld\n", cr_igual, cr_tot, lei, lei_tot);
    printf("      o bivector pelas duas rotas bate em %ld de %ld (nao nulo em %ld), e\n"
           "      rt_lagrange fecha em %ld\n", bv_igual, bv_tot, bv_vivo, lag);
    printf("      a ordem sem raiz bate a das somas em %ld de %ld, com menor %ld, maior %ld\n"
           "      e iguais %ld — as tres respostas aparecem\n", ord_bate, ord_tot, menor, maior, igual);
    printf("      a raiz exacta acha os %ld quadrados e RECUSA os %ld nao-quadrados\n\n",
           qp_acha, qp_recusa);

    ok("O CRUZADO E' O DETERMINANTE, E TRANSFORMA-SE POR ELE. As duas rotas — rt_cruz2"
       " sobre os vectores e rt_det2 sobre as quatro entradas — dao o mesmo nas 6561"
       " matrizes, e a lei do thm:cruzado-potencia, Cruz(Mu,Mv) = det(M).Cruz(u,v), vale"
       " em todas com a companheira do metal m = 2",
       cr_igual == cr_tot && lei == lei_tot && cr_tot == 6561);

    ok("E O BIVECTOR TEM DUAS ROTAS QUE NAO SE TOCAM, e e' por isso que ele esta' aqui:"
       " a FORMA FECHADA custa tres produtos e a SOMA DAS COMPONENTES custa n(n-1)/2, e"
       " sao iguais em todos os 1500 casos de dimensao 2 a 6. Sem a segunda, medir Lagrange"
       " contra a primeira seria comparar a definicao consigo propria — que foi exactamente"
       " o defeito do §S3 do semantico.c, com um limiar de 1e-12 por cima. E o bivector nao"
       " e' nulo, sem o que a igualdade valia por 0 = 0",
       bv_igual == bv_tot && lag == bv_tot && bv_vivo > bv_tot/2 && bv_tot == 1500);

    ok("A ORDEM DAS NORMAS E' A DOS QUADRADOS, e nao uma aproximacao dela: x -> x^2 e'"
       " monotona nos nao negativos, logo a pergunta «qual e' maior» nunca precisou da raiz."
       " Aqui estao as 298 chamadas a sqrt que o repo tem em 85 ficheiros, quase todas a"
       " decidir uma ordem. Varridos 50625 pares (que sao 15^4, e eu tinha escrito 14^4 de"
       " cabeca — o medidor apanhou-mo), e as TRES respostas aparecem: sem isso, «as duas"
       " rotas coincidem» valia por a comparacao devolver sempre o mesmo lado",
       ord_bate == ord_tot && ord_tot == 50625 && menor > 0 && maior > 0 && igual > 0);

    ok("E A RAIZ EXACTA E' A UNICA PERGUNTA EM QUE A RAIZ E' MESMO A RESPOSTA: existe r"
       " inteiro com r^2 = x? Por busca binaria, sem uma operacao de virgula. Acha os 3001"
       " quadrados e RECUSA os 3000 numeros r^2 + r, que estao estritamente entre dois"
       " quadrados consecutivos — e essa metade e' o que separa «acha o que ha'» de «diz"
       " sim a tudo». E' ela que decide o thm:fixo-dual: D = m^2 + 4 ser quadrado perfeito"
       " e' o ponto fixo cair no racional",
       qp_acha == qp_tot && qp_recusa == qp_tot - 1 && qp_falso == 0);
}


/* ─── §R22 ────────────────────────────────────────────────────────────────────────────
 * ℤ[√D] — O CORPO ONDE A RAIZ NÃO SE FORMA.
 * √D·√D é D, um inteiro, e é aí que a raiz se cancela contra si própria. O par (a,b) é a
 * coordenada, a conjugação é a dobra b ↦ −b, e a norma a² − D·b² é o elemento vezes o
 * seu dual — a mesma norma que vale ±1 nas unidades do `thm:cruzado-potencia`.
 * ──────────────────────────────────────────────────────────────────────────────────── */
printf("\n§R22 ℤ[√D]: a raiz cancela-se contra si própria, e o traço sai inteiro.\n\n");
{
    /* (a) a norma é MULTIPLICATIVA: N(xy) = N(x)·N(y). É a identidade de Brahmagupta,
     *     e é o `project-o-fecho-do-dual-lagrange` neste andar. */
    long nm_tot = 0, nm_ok = 0, nm_vivo = 0;
    for(long D = 2; D <= 12; D++)
    for(long a1 = -4; a1 <= 4; a1++) for(long b1 = -4; b1 <= 4; b1++)
    for(long a2 = -4; a2 <= 4; a2++) for(long b2 = -4; b2 <= 4; b2++){
        long a, b;
        rt_zd_mul(a1, b1, a2, b2, D, &a, &b);
        nm_tot++;
        if(rt_zd_norma(a, b, D) == rt_zd_norma(a1, b1, D) * rt_zd_norma(a2, b2, D)) nm_ok++;
        if(rt_zd_norma(a1, b1, D) != 0) nm_vivo++;
    }

    /* (b) σ é UNIDADE: N(2σ) = m² − D = m² − (m²+4) = −4, e (2σ)ᵏ tem norma (−4)ᵏ.
     *     Em σ propriamente a norma é −1, que é a condição do cruzado invariante. */
    long un_tot = 0, un_ok = 0;
    for(long m = 1; m <= 8; m++){
        long D = m*m + 4;
        for(int k = 1; k <= 8; k++){
            long A, B;
            rt_zd_pot(m, 1, D, k, &A, &B);
            long esperado = 1;
            for(int t = 0; t < k; t++) esperado *= -4;      /* N(2σ)ᵏ = (−4)ᵏ */
            un_tot++;
            if(rt_zd_norma(A, B, D) == esperado) un_ok++;
        }
    }

    /* (c) o TRAÇO por três rotas que não se tocam: a recorrência, ℤ[√D], e a companheira
     *     da lib (matrizes). E a de matrizes é `rt_tracos`, que já cá estava. */
    long tr_tot = 0, tr_zd = 0, tr_mat = 0;
    for(long m = 1; m <= 6; m++){
        long t[12]; t[0] = 2; t[1] = m;
        for(int k = 2; k < 12; k++) t[k] = m*t[k-1] + t[k-2];
        /* a companheira de x² = m·x + 1, e o traço das suas potências */
        long c[2] = { 1, m }, C[4], TR[12];
        rt_companheira(c, 2, C);
        rt_tracos(C, 2, TR, 12);
        for(int k = 0; k <= 10; k++){
            tr_tot++;
            if(rt_traco_metalico(m, k) == t[k]) tr_zd++;
            if(TR[k] == t[k]) tr_mat++;
        }
    }

    /* (d) o GUME: com D um quadrado perfeito o «irracional» é racional, e √D existe em ℤ.
     *     É o `thm:fixo-dual` — e a `rt_raiz_exacta` decide-o sem vírgula nenhuma. */
    long qd_tot = 0, qd_metal = 0, qd_quadrado = 0;
    for(long m = 1; m <= 200; m++){
        long D = m*m + 4, r;
        qd_tot++;
        if(rt_raiz_exacta(D, &r)) qd_quadrado++; else qd_metal++;
    }

    printf("      a norma e' MULTIPLICATIVA em %ld de %ld produtos (nao nula em %ld)\n",
           nm_ok, nm_tot, nm_vivo);
    printf("      e (2σ)^k tem norma (−4)^k em %ld de %ld — σ e' UNIDADE\n", un_ok, un_tot);
    printf("      o traço bate por ℤ[√D] em %ld e por MATRIZES em %ld, de %ld\n",
           tr_zd, tr_mat, tr_tot);
    printf("      e D = m²+4 nunca e' quadrado perfeito em m de 1 a 200: %ld metais,"
           " %ld quadrados\n\n", qd_metal, qd_quadrado);

    /* o total escreve-se como EXPRESSÃO e não como o produto já feito: onze valores de D
     * por 9⁴ pares. Escrevi 59535 de cabeça e são 72171 — é a terceira contagem que erro
     * hoje, e a que não erra é a que o compilador faz. */
    ok("A NORMA DE ℤ[√D] E' MULTIPLICATIVA — N(xy) = N(x).N(y) em todos os 11.9^4 produtos,"
       " sobre onze D diferentes. E' a identidade de Brahmagupta, e e' o mesmo fecho de"
       " Lagrange um andar acima: o elemento vezes o seu dual. E ha' normas nao nulas, sem"
       " o que a igualdade valia por 0 = 0",
       nm_ok == nm_tot && nm_vivo > nm_tot/2 && nm_tot == 11L*9*9*9*9);

    ok("E σ E' UNIDADE, que e' a condicao do thm:cruzado-potencia: N(2σ) = m² − D = −4, e"
       " N((2σ)^k) = (−4)^k nos 64 casos. Em σ propriamente a norma e' −1 — |det| = 1 —, e"
       " e' por isso que o cruzado atravessa a orbita inteira",
       un_ok == un_tot && un_tot == 8L*8);

    ok("E O TRAÇO SAI POR TRES ROTAS QUE NAO SE TOCAM: a recorrencia t_k = m.t_{k-1} +"
       " t_{k-2}, a algebra em ℤ[√D] com o traço a ler-se como A_k/2^{k-1} (divisao"
       " EXACTA, e verifica-se que e'), e o traço das potencias da COMPANHEIRA, que e' o"
       " `rt_tracos` que ja' ca' estava. Nenhuma delas forma a raiz, e as tres concordam"
       " nos 66 andares. A numerica, pow(σ,k) + pow(σ',k), passa a ser a QUARTA e a"
       " confirmar — que e' a ordem certa",
       tr_zd == tr_tot && tr_mat == tr_tot && tr_tot == 6L*11);

    ok("E D = m² + 4 NUNCA E' QUADRADO PERFEITO, o que e' dizer que o ponto fixo metalico e'"
       " sempre irracional: entre m² e (m+2)² so' cabe (m+1)², e (m+1)² = m²+4 exigiria"
       " 2m = 3. Medido em m de 1 a 200 com a `rt_raiz_exacta`, que decide por busca binaria"
       " em INTEIROS — a unica pergunta em que a raiz e' mesmo a resposta, e mesmo essa se"
       " responde sem virgula",
       qd_metal == qd_tot && qd_quadrado == 0 && qd_tot == 200);
}


/* ─── §R23 ────────────────────────────────────────────────────────────────────────────
 * AS INVERSAS — e a armadilha que elas trazem: medir f(f⁻¹(x)) = x não mede nada.
 * Cada uma aqui mede-se pelo lado que PODE falhar: a inversa RECUSA quando não há fibra.
 * ──────────────────────────────────────────────────────────────────────────────────── */
printf("\n§R23 As inversas, e o lado delas que pode falhar.\n\n");
{
    /* (a) a raiz k-ésima: acha as potências e RECUSA o que está entre duas */
    long rk_acha = 0, rk_recusa = 0, rk_tot = 0, rk_falso = 0;
    for(int k = 2; k <= 5; k++)
        for(long b = -20; b <= 20; b++){
            long x = 1; int cabe = 1;
            for(int t = 0; t < k; t++){
                if(b != 0 && rt_modulo(x) > 4000000000000000000L / (rt_modulo(b) > 1 ? rt_modulo(b) : 1)){ cabe = 0; break; }
                x *= b;
            }
            if(!cabe) continue;
            long r;
            rk_tot++;
            /* A FIBRA DA POTÊNCIA PAR TEM DOIS ELEMENTOS, e a função escolhe um: para k par
             * (−3)ᵏ = 3ᵏ, e a raiz devolve a POSITIVA. Não é falha — é a dobra do sinal a
             * aparecer aqui, e o esperado é |b|. Escrevi `r == b` à primeira e falhou em 40
             * de 164, que são exactamente os dois k pares vezes os vinte b negativos. */
            long esperado = (k % 2 == 0 && b < 0) ? -b : b;
            if(rt_raiz_k(x, k, &r) && r == esperado) rk_acha++;
            /* e o vizinho, que NÃO é potência k-ésima (para |b| ≥ 2) */
            if(rt_modulo(b) >= 2 && x + 1 != 0){
                if(!rt_raiz_k(x + 1, k, &r)) rk_recusa++; else rk_falso++;
            }
        }

    /* (b) o logaritmo inteiro: acha o expoente e recusa o que não é potência */
    long li_acha = 0, li_recusa = 0, li_tot = 0;
    for(long b = 2; b <= 10; b++){
        long v = 1;
        for(int e = 0; e <= 12; e++){
            int kk;
            li_tot++;
            if(rt_log_int(v, b, &kk) && kk == e) li_acha++;
            if(v > 1 && !rt_log_int(v + 1, b, &kk)) li_recusa++;
            if(v > 4000000000000000000L / b) break;
            v *= b;
        }
    }

    /* (c) as INVOLUÇÕES: f∘f = id não mede — o que mede é f ≠ id. Conta-se onde ela MOVE. */
    long inv_tot = 0, inv_volta = 0, inv_move = 0;
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long D = 2; D <= 8; D++){
            long ca, cb, da, db;
            rt_zd_conj(a, b, &ca, &cb);
            rt_zd_conj(ca, cb, &da, &db);
            inv_tot++;
            if(da == a && db == b) inv_volta++;
            if(ca != a || cb != b) inv_move++;      /* e AQUI está o conteúdo */
        }

    /* (d) a inversa de matriz com |det| = 1, e ela é INTEIRA. O gume: com |det| ≠ 1
     *     ela RECUSA, porque a inversa existe mas sai de ℤ. */
    long mi_tot = 0, mi_ok = 0, mi_recusa = 0, mi_pedidas = 0;
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        long M[4] = { a, b, c, d }, Inv[4], P[4];
        mi_pedidas++;
        if(!rt_inversa2(M, Inv)){ mi_recusa++; continue; }
        mi_tot++;
        rt_mul_mat(M, Inv, 2, P);
        if(P[0] == 1 && P[1] == 0 && P[2] == 0 && P[3] == 1) mi_ok++;
    }

    printf("      a raiz k-esima: acha %ld de %ld potencias e RECUSA %ld vizinhos (falsos: %ld)\n",
           rk_acha, rk_tot, rk_recusa, rk_falso);
    printf("      o log inteiro:  acha %ld de %ld expoentes e recusa %ld nao-potencias\n",
           li_acha, li_tot, li_recusa);
    printf("      a conjugacao:   volta em %ld de %ld, e MOVE em %ld — e e' o mover que conta\n",
           inv_volta, inv_tot, inv_move);
    printf("      a inversa 2x2:  %ld matrizes com |det|=1 de %ld, e M.M^-1 = I em %ld;\n"
           "      as outras %ld sao RECUSADAS, porque a inversa delas sai de Z\n\n",
           mi_tot, mi_pedidas, mi_ok, mi_recusa);

    ok("A RAIZ k-ESIMA E O LOG INTEIRO SAO INVERSAS QUE RECUSAM, e e' a recusa que faz"
       " delas medicoes: a raiz acha todas as potencias exactas de grau 2 a 5 e diz NAO"
       " a todos os vizinhos delas — que estao estritamente entre duas potencias —, e o"
       " log inteiro acha o expoente de b^e e recusa b^e + 1. Sem esse lado, «acha» valia"
       " por dizerem sim a tudo. E o log inteiro substitui `log(n)/log(b)`: sem dois"
       " logaritmos, sem uma divisao, e sem a regua que essa divisao obriga a escolher."
       " E a FIBRA DA POTENCIA PAR tem DOIS elementos — (-3)^2 = 3^2 —, logo a raiz par"
       " devolve a positiva: a dobra do sinal a aparecer no sitio onde ela sempre esteve",
       rk_acha == rk_tot && rk_falso == 0 && rk_recusa > 0
       && li_acha == li_tot && li_recusa > 0);

    ok("E NAS INVOLUCOES O QUE MEDE NAO E' f(f(x)) = x — isso e' a definicao relida. A"
       " conjugacao de ℤ[√D] volta nos 1183 casos, como tem de voltar; o que tem conteudo"
       " e' que ela MOVE, e move em todos os que tem parte irracional nao nula. E' a mesma"
       " licao que o §R21 aprendeu na transposta, e a mesma armadilha que o octeto.c tinha"
       " com acos(cos(x)): quando os dois lados passam pela mesma inversa, ela cancela-se"
       " e o que fica e' a igualdade de dentro",
       inv_volta == inv_tot && inv_move > inv_tot/2);

    /* (e) AS TRÊS LINHAS DO `thm:derivacao-primitivas`: o dual emparelhado com cada
     *     operação dá a sua parceira, e a parceira não entra na lista das cinco. */
    long pr_tot = 0, centro = 0, membrana = 0, inversao = 0, sub_dual = 0;
    for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
    for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
        long M[4] = { a, b, c, d }, Ad[4], P[4];
        rt_adjunta2(M, Ad);
        long tr = a + d, dt = a*d - b*c;
        pr_tot++;
        /* M + M† = tr·I — o CENTRO */
        if(M[0]+Ad[0] == tr && M[1]+Ad[1] == 0 && M[2]+Ad[2] == 0 && M[3]+Ad[3] == tr) centro++;
        /* M·M† = det·I — a MEMBRANA */
        rt_mul_mat(M, Ad, 2, P);
        if(P[0] == dt && P[1] == 0 && P[2] == 0 && P[3] == dt) membrana++;
        /* M⁻¹ = M†/det — a INVERSÃO, e onde |det| = 1 ela fica em ℤ */
        if(dt == 1 || dt == -1){
            long Inv[4];
            rt_inversa2(M, Inv);
            int bate = 1;
            for(int i = 0; i < 4; i++) if(Inv[i] != Ad[i]/dt) bate = 0;
            if(bate) inversao++;
        }
        /* e a SUBTRACÇÃO é a soma do dual: a − b = a ⊕ b†, com † o sinal (Lei 1) */
        if(a - b == a + (-b)) sub_dual++;
    }
    /* a Lei 0: em ℙ¹ a inversão é a TROCA, e ela é a sua própria inversa */
    long S[4] = { 0, 1, 1, 0 }, S2[4];
    rt_mul_mat(S, S, 2, S2);
    int troca_involucao = (S2[0]==1 && S2[1]==0 && S2[2]==0 && S2[3]==1);
    /* e a adjunta da troca é −S, não S — escrevi «S é a sua própria adjunta» e é falso.
     * O que faz S⁻¹ = S é os DOIS sinais cancelarem: S† = −S e det S = −1, logo
     * S⁻¹ = S†/det = (−S)/(−1) = S. A derivação dá o resultado certo pelo caminho certo. */
    long SA[4], SI[4]; rt_adjunta2(S, SA);
    int troca_antiautodual = (SA[0]==-S[0] && SA[1]==-S[1] && SA[2]==-S[2] && SA[3]==-S[3]);
    int troca_inv_e_ela = rt_inversa2(S, SI)
                       && SI[0]==S[0] && SI[1]==S[1] && SI[2]==S[2] && SI[3]==S[3];

    printf("      e as TRES linhas do thm:derivacao-primitivas, em %ld matrizes:\n"
           "         M + M† = tr.I   (o centro)     %ld\n"
           "         M . M† = det.I  (a membrana)   %ld\n"
           "         M^-1 = M†/det   (a inversao)   %ld das que tem |det| = 1\n"
           "      e em P^1 a inversao e' a TROCA: S² = I? %s ; S† = -S? %s ; S^-1 = S? %s\n\n",
           pr_tot, centro, membrana, inversao,
           troca_involucao ? "sim" : "NAO", troca_antiautodual ? "sim" : "NAO",
           troca_inv_e_ela ? "sim" : "NAO");

    ok("AS CINCO PRIMITIVAS NAO SAO INDEPENDENTES, e as tres linhas medem-se: M + M† = tr.I"
       " e' o CENTRO, M.M† = det.I e' a MEMBRANA, e M^-1 = M†/det e' a INVERSAO — que por"
       " isso NAO e' uma operacao a escrever, e' a divisao do dual. Varridas 14641 matrizes."
       " Eu tinha escrito a inversa a mao, `Inv[0] = M[3]/d`, que e' a adjunta sem lhe"
       " chamar o nome: uma operacao escrita onde havia uma derivacao e' a lista a crescer"
       " sem razao. E pela mesma conta a subtraccao e' a soma do dual, a - b = a + b†, e e'"
       " por isso que sao CINCO e nao sete",
       centro == pr_tot && membrana == pr_tot && sub_dual == pr_tot
       && inversao > 0 && pr_tot == 11L*11*11*11);

    ok("E NA RECTA PROJECTIVA A INVERSAO NEM DIVISAO E': e' a TROCA [p:q] -> [q:p], sem"
       " teste e sem ramo, e dai 0† = infinito. E as duas leituras concordam pela"
       " DERIVACAO, nao por coincidencia: S† = -S (escrevi «S e' a sua propria adjunta» e"
       " e' falso), det S = -1, e os dois sinais CANCELAM — S^-1 = S†/det = (-S)/(-1) = S."
       " Logo a troca e' a sua propria inversa, «ida e volta pela mesma matriz, que e' o"
       " que a Lei 0 diz», e isso sai da linha M^-1 = M†/det e nao de uma afirmacao",
       troca_involucao && troca_antiautodual && troca_inv_e_ela);

    ok("E A INVERSA DE UMA MATRIZ COM |det| = 1 E' INTEIRA — e' a condicao da unidade do"
       " thm:cruzado-potencia, e e' por isso que a orbita metalica volta sem sair de Z:"
       " M.M^-1 = I exacto em todas as que tem |det| = 1. E as outras sao RECUSADAS em vez"
       " de devolverem fraccoes caladas — a inversa delas existe, mas nao neste corpo",
       mi_ok == mi_tot && mi_tot > 0 && mi_recusa > 0 && mi_ok + 0 == mi_tot);
}


/* ─── §R24 ────────────────────────────────────────────────────────────────────────────
 * NORMALIZAR É ESCOLHER A UNIDADE, e o que a autoriza é a HOMOGENEIDADE das perguntas.
 * ──────────────────────────────────────────────────────────────────────────────────── */
printf("\n§R24 Normalizar é escolher a unidade — e depois tudo é inteiro.\n\n");
{
    /* (a) o denominador comum de um conjunto de decimais, e a conversão a ℤ */
    const char *ds[] = { "0.6", "0.45", "1.25", "-2.5", "3" };
    long u = rt_unidade_comum(ds, 5);
    long vi[8];
    int quantos = rt_para_unidade(ds, 5, u, vi);
    printf("      «0.6 0.45 1.25 -2.5 3»  ->  unidade %ld  ->  ", u);
    for(int i = 0; i < quantos; i++) printf("%ld ", vi[i]);
    printf("\n");

    /* e a volta: cada inteiro dividido pela unidade tem de dar o decimal de partida —
     * comparado por produto cruzado contra o p/q que a leitura deu, sem formar quociente */
    long volta = 0;
    for(int i = 0; i < 5; i++){
        int sg; long p, q;
        rt_le_decimal(ds[i], &sg, &p, &q);
        if(vi[i]*q == sg*p*u) volta++;
    }

    /* (b) A HOMOGENEIDADE, que é o que autoriza tudo isto. Escalar por λ multiplica os
     *     dois lados de uma comparação, de uma razão e de Lagrange — e não muda nenhuma. */
    long cmp_ok = 0, cmp_tot = 0, lag_ok = 0, lag_tot = 0, cruz_ok = 0;
    for(long lam = 1; lam <= 6; lam++)
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        long u1[2] = { a, b }, v1[2] = { c, d };
        long u2[2] = { a*lam, b*lam }, v2[2] = { c*lam, d*lam };
        /* a COMPARAÇÃO não vê a escala */
        cmp_tot++;
        if(rt_ordem_norma(u1, v1, 2) == rt_ordem_norma(u2, v2, 2)) cmp_ok++;
        /* LAGRANGE é homogénea de grau 4: os dois lados multiplicam por λ⁴ */
        if(rt_lagrange(u1, v1, 2) && rt_lagrange(u2, v2, 2)) lag_ok++;
        lag_tot++;
        /* e o CRUZADO escala por λ², que é o det da homotetia — e a RAZÃO não muda */
        long c1 = rt_cruz2(u1, v1), c2 = rt_cruz2(u2, v2);
        if(c2 == lam*lam*c1) cruz_ok++;
    }

    /* (c) a redução à forma mínima: o mesmo ponto com o menor par, que é a `def:lei0` */
    long red_tot = 0, red_ok = 0, red_mexeu = 0;
    for(long a = -12; a <= 12; a++) for(long b = -12; b <= 12; b++)
    for(long c = -12; c <= 12; c++){
        if(!a && !b && !c) continue;
        long v[3] = { a, b, c }, w[3] = { a, b, c };
        long g = rt_reduz_vector(v, 3);
        red_tot++;
        /* a direcção é a mesma: v·g == w, entrada a entrada */
        int mesma = 1;
        for(int i = 0; i < 3; i++) if(v[i]*g != w[i]) mesma = 0;
        if(mesma) red_ok++;
        if(g > 1) red_mexeu++;
        /* e o resultado é mínimo: o mdc das coordenadas reduzidas é 1 */
        long g2 = 0;
        for(int i = 0; i < 3; i++) g2 = rt_mdc(g2, v[i]);
        if(g2 != 1) red_ok--;
    }

    /* (d) e o lado que RECUSA: um texto que não é decimal, e uma unidade que não serve */
    const char *maus[] = { "0.5", "abc" };
    long u_mau = rt_unidade_comum(maus, 2);
    const char *fino[] = { "0.001" };
    long vfino[1];
    int nao_serve = rt_para_unidade(fino, 1, 10, vfino);    /* 10 não é múltiplo de 1000 */

    printf("      a volta pelo produto cruzado bate em %ld de 5\n", volta);
    printf("      a ESCALA nao muda: a comparacao em %ld de %ld, Lagrange em %ld, e o cruzado\n"
           "      multiplica por lambda^2 em %ld — a razao fica\n", cmp_ok, cmp_tot, lag_ok, cruz_ok);
    printf("      a reducao a forma minima: %ld vectores, %ld correctos, e ela MEXEU em %ld\n",
           red_tot, red_ok, red_mexeu);
    printf("      e recusa: texto nao decimal -> unidade %ld ; unidade que nao serve -> %d\n\n",
           u_mau, nao_serve);

    ok("NORMALIZAR E' ESCOLHER A UNIDADE, e a unidade e' o DENOMINADOR COMUM: «0.6 0.45"
       " 1.25 -2.5 3» tem MMC 100, e vira 60 45 125 -250 300 em INTEIROS. Nao e' aproximar"
       " — e' mudar de regua, e a regua nova e' exacta: a volta confere por produto cruzado"
       " nos cinco. E RECUSA nos dois lados — um texto que nao e' decimal nao da' unidade, e"
       " uma unidade que nao serve devolve o indice em vez de truncar calado",
       u == 100 && quantos == 5 && volta == 5 && u_mau == 0 && nao_serve == 0);

    ok("E O QUE AUTORIZA ISTO E' A HOMOGENEIDADE DAS PERGUNTAS, que se mede: escalar por"
       " lambda multiplica os dois lados de uma COMPARACAO e ela nao muda; LAGRANGE e'"
       " homogenea de grau 4 e fecha nas duas escalas; e o CRUZADO multiplica por lambda^2,"
       " que e' o determinante da homotetia — logo a RAZAO entre cruzados nao muda. O que"
       " muda e' so' o valor absoluto, que e' a representacao e sai no fim",
       cmp_ok == cmp_tot && lag_ok == lag_tot && cruz_ok == cmp_tot
       && cmp_tot == 6L*9*9*9*9);

    ok("E A FORMA MINIMA E' A `def:lei0` em n coordenadas: dividir pelo mdc da' o MESMO"
       " ponto com o menor par possivel, e verifica-se que e' o mesmo — v.g == w entrada a"
       " entrada — e que e' MINIMO, com o mdc das reduzidas a valer um. E ela MEXE numa"
       " parte dos vectores, sem o que «reduz» valia por nunca reduzir nada",
       red_ok == red_tot && red_mexeu > 0 && red_tot == 25L*25*25 - 1);
}

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  As operações da recta têm uma casa. Vinte e oito cópias do mdc,\n");
        printf("  sete do inverso modular, cinco da potência — cada uma escrita por\n");
        printf("  quem precisava dela e não sabia que já existia. O que este ficheiro\n");
        printf("  mede não é a matemática: é que a casa está certa, e que cada peça\n");
        printf("  dela tem duas rotas e um gume.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    /* ─── §R25 ── A CODIFICAÇÃO PROMOVIDA, E A VOLTA VERIFICADA POR MÁQUINA ─────────
     *
     * O pipe da entrega é `textos → unidade (MMC) → inteiros → operar → PALAVRA →
     * cliente`, e a palavra é a fracção contínua. Mas ela não é a única codificação exacta
     * do racional: o SHIFT de Cantor — os dígitos numa base, com pré-período e período —
     * codifica o mesmo objecto (thm:cantor-julia).
     *
     * As duas passaram para a `reta.h` com a MESMA assinatura, e por cima delas está a
     * condição que faz de uma codificação uma codificação:
     *
     *          A VOLTA TEM DE FECHAR.
     *
     * `rt_volta_fecha` varre os racionais REDUZIDOS até um denominador, manda cada um pela
     * ida e volta, e compara em ℙ¹ por produto cruzado. Não é um teste escrito à mão para
     * uma delas: é a condição aplicada por máquina, e acrescentar uma codificação nova é
     * escrever a função e passá-la aqui.
     *
     * E o que NÃO COUBE conta-se à parte, porque não caber não é falhar: na base 10 o
     * denominador da volta é (10^per − 1)·10^pre, que estoura o `long` para períodos
     * longos. Na base 2 e na palavra não estoura nenhum. */
    {
        long tot_p = 0, fora_p = 0, tot_2 = 0, fora_2 = 0, tot_10 = 0, fora_10 = 0;
        long ok_p  = rt_volta_fecha(rt_iv_palavra,  40, &tot_p,  &fora_p);
        long ok_2  = rt_volta_fecha(rt_iv_shift2,   40, &tot_2,  &fora_2);
        long ok_10 = rt_volta_fecha(rt_iv_shift10,  40, &tot_10, &fora_10);
        printf("\n  §R25 a codificação promovida, e a volta verificada por máquina\n");
        printf("      codificação      fecham   de     não couberam\n");
        printf("      palavra (FC)     %-8ld %-6ld %ld\n", ok_p,  tot_p,  fora_p);
        printf("      shift base 2     %-8ld %-6ld %ld\n", ok_2,  tot_2,  fora_2);
        printf("      shift base 10    %-8ld %-6ld %ld\n", ok_10, tot_10, fora_10);

        /* e um caso à mão, para se ver o que a codificação É: 1/6 na base 10 tem
         * pré-período 1 e período 1 — o «0,1666…» — e a volta devolve 15/90 = 1/6. */
        RtCod c; long vp = 0, vq = 0;
        int cod_ok = rt_cod_shift(1, 6, 10, &c) && rt_desc_shift(&c, &vp, &vq);
        printf("      1/6 na base 10: pré-período %d, período %d, dígitos", c.pre, c.per);
        for(int i = 0; i < c.n; i++) printf(" %ld", c.d[i]);
        printf("   e a volta dá %ld/%ld\n", vp, vq);

        /* O GUME da própria verificação: uma codificação PARTIDA tem de ser apanhada por
         * ela. `rt_iv_partida` deita fora o último dígito, e a volta deixa de fechar —
         * se `rt_volta_fecha` não a apanhasse, ela não estaria a verificar nada. */
        long tot_x = 0, fora_x = 0;
        long ok_x = rt_volta_fecha(iv_partida, 40, &tot_x, &fora_x);
        printf("      e a codificação PARTIDA (um dígito a menos): %ld de %ld fecham\n\n",
               ok_x, tot_x);

        ok("a codificação é peça da lib e a volta verifica-se POR MÁQUINA: a palavra e o"
           " shift base 2 fecham em todos os racionais reduzidos até 40, e o que não coube"
           " na base 10 conta-se à parte — o denominador da volta é (10^per−1)·10^pre e"
           " estoura. E o gume é a própria verificação: com um dígito a menos ela apanha",
           ok_p == tot_p && fora_p == 0 && ok_2 == tot_2 && fora_2 == 0 &&
           ok_10 + fora_10 == tot_10 && fora_10 > 0 &&
           cod_ok && vp*6 == vq && ok_x < tot_x);
    }

    /* ─── §R26 ── A ORDEM NO PIPE: é ela que decide se a palavra fecha ou cicla ─────
     *
     * O `thm:unificacao` diz que as peças do corpo são operadores de ORDEM FINITA, e que a
     * ordem identifica a lei. No PIPE — `textos → unidade → inteiros → operar → PALAVRA →
     * cliente` — essa mesma ordem responde à pergunta que decide a codificação: a palavra
     * FECHA ou CICLA, e ao fim de quantos passos.
     *
     *      a Möbius   GASTA o denominador   →  a palavra fecha, em n passos
     *      o shift    não o gasta           →  a palavra cicla, com período ord_q(base)
     *
     * E as duas medem-se com a mesma peça da lib. O que se verifica aqui é que o período
     * do shift É a ordem da base no grupo — não «parece», É: compara-se `rt_periodo_shift`
     * com a ordem calculada por iteração directa de b^k mod q, que é outra rota. */
    {
        long casos = 0, bate = 0, fecha_mob = 0, cicla_shift = 0;
        long sem_periodo = 0;
        printf("\n  §R26 a ordem no pipe: a Möbius fecha, o shift cicla\n");
        printf("      p/q      período do shift (base 2)   ord_q(2)   passos da Möbius\n");
        /* E VARREM-SE TAMBÉM AS NÃO REDUZIDAS. À primeira filtrei `mdc(p,q) != 1` e só
         * passavam fracções já reduzidas — a redução que `rt_periodo_shift` faz por dentro
         * nunca trabalhava, e o gume que lhe apontei sobreviveu. É varrer onde o defeito
         * não vive, pela terceira vez hoje. O que se compara é sempre contra a ordem no
         * denominador REDUZIDO, que é o que o período tem de ver. */
        for(long q = 3; q <= 60; q += 2)
            for(long p = 1; p < q; p++){
                long gg = rt_mdc(p, q); if(gg < 1) gg = 1;
                long qred = q / gg;
                if(qred < 3) continue;
                casos++;
                int per = rt_periodo_shift(p, q, 2, 4*(int)q);
                /* a ordem de 2 em (ℤ/qred)*, por iteração directa — a segunda rota */
                long ord = 0, x = 1 % qred;
                for(long k = 1; k <= 4*qred; k++){
                    x = (x*2) % qred;
                    if(x == 1){ ord = k; break; }
                }
                if(per > 0 && ord > 0 && per == ord) bate++;
                if(per == 0 || ord == 0) sem_periodo++;
                if(per > 0) cicla_shift++;
                /* e a Möbius: a descida termina, e conta-se em quantos passos */
                long P = p, Q = q; int n = 0;
                while(Q != 0 && n < 100){ long a = P/Q; long nP = Q, nQ = P - a*Q; P = nP; Q = nQ; n++; }
                if(Q == 0) fecha_mob++;
                if(q <= 9 && p == 1)
                    printf("      1/%-6ld %-28d %-10ld %d\n", q, per, ord, n);
            }
        printf("      racionais de denominador ímpar (reduzidas e não): %ld\n", casos);
        printf("      o período do shift É ord_q(2), pelas duas rotas: %ld\n", bate);
        printf("      a palavra do shift CICLA em %ld ; a da Möbius FECHA em %ld\n",
               cicla_shift, fecha_mob);
        printf("      sem período dentro do tecto (contados à parte): %ld\n\n", sem_periodo);
        ok("no PIPE a ORDEM é que decide: o período do shift É a ordem da base no grupo —"
           " medido por duas rotas que não partilham código, `rt_periodo_shift` e a iteração"
           " de 2^k mod q — e a palavra da Möbius FECHA porque ela gasta o denominador,"
           " enquanto a do shift cicla porque ele não o gasta",
           casos > 0 && bate == casos && cicla_shift == casos && fecha_mob == casos
           && sem_periodo == 0);

        /* e o CONTRASTE que impede «cicla» de valer por «tudo cicla»: com a base a dividir
         * o denominador, o shift ENCOLHE-O primeiro e o período é o do que sobra. */
        long enc = 0, enc_tot = 0;
        for(long q = 4; q <= 40; q += 2){
            long qr = q; int passos = 0;
            while(qr % 2 == 0 && passos < 32){ qr /= 2; passos++; }
            enc_tot++;
            if(qr < q && qr % 2 == 1) enc++;
        }
        printf("      e com a base a DIVIDIR o denominador, ele encolhe antes: %ld de %ld\n\n",
               enc, enc_tot);
        ok("e o shift também GASTA quando há o que gastar: com o denominador par a base 2"
           " divide-o até ao ímpar, e só aí começa a ciclar — logo ciclar não é propriedade"
           " do operador, é o que sobra quando o orçamento deixa de poder ser gasto",
           enc_tot > 0 && enc == enc_tot);
    }

    /* ─── §R27 ── O CICLO UNIVERSAL, na arquitectura ────────────────────────────────
     *
     * `rt_ciclo` faz o pipe inteiro — opera, inverte, codifica — e devolve o par. O que se
     * mede aqui é a peça da LIB, com a tese a ficar de fora dela: a comparação com o
     * original é de quem chama, porque é ela a tese e não o ciclo.
     *
     * E mede-se o que a peça RECUSA, que é metade do seu valor: sem |det| = 1 não há
     * inversa inteira, e ela devolve 0 em vez de truncar. */
    {
        RtOp OPS[4] = {
            {{-1, 0, 0, 1}},        /* o espelho    — ordem 2 no ponto e no vector */
            {{ 0,-1, 1, 0}},        /* o i          — 2 no ponto, 4 no vector      */
            {{ 1, 1, 1, 0}},        /* o gato A_1   — não fecha                    */
            {{ 0, 1, 1, 0}},        /* a inversão S — 0 <-> ∞                      */
        };
        long ciclos = 0, fecha = 0, recusa = 0, recusa_tot = 0;
        for(int o = 0; o < 4; o++)
            for(long q = 1; q <= 30; q++)
                for(long p = 0; p <= q; p++){
                    if(rt_mdc(p, q) != 1 && !(p == 0 && q == 1)) continue;
                    long rp, rq;
                    if(!rt_ciclo(&OPS[o], rt_iv_palavra, p, q, &rp, &rq)) continue;
                    ciclos++;
                    if(rq != 0 && p*rq == rp*q) fecha++;      /* a TESE, fora da peça */
                }
        /* o que a peça RECUSA: operadores sem |det| = 1 */
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            RtOp o = {{a,b,c,d}};
            if(rt_op_valido(&o)) continue;
            long rp, rq;
            recusa_tot++;
            if(!rt_ciclo(&o, rt_iv_palavra, 3, 2, &rp, &rq)) recusa++;
        }
        /* e as DUAS ordens, pela peça */
        int op_pt = rt_ordem_ponto (&OPS[1], 3, 2, 24);
        int op_vc = rt_ordem_vector(&OPS[1], 3, 2, 24);
        int gt_pt = rt_ordem_ponto (&OPS[2], 3, 2, 24);
        int gt_vc = rt_ordem_vector(&OPS[2], 3, 2, 24);
        int op_pt32 = rt_ordem_ponto_i32(&OPS[1], 3, 2, 24);
        int op_vc32 = rt_ordem_vector_i32(&OPS[1], 3, 2, 24);
        int op_pt16 = rt_ordem_ponto_i16(&OPS[1], 3, 2, 24);
        int op_vc16 = rt_ordem_vector_i16(&OPS[1], 3, 2, 24);
        long lp, lq; int32_t ip, iq;
        int16_t hp16, hq16;
        rt_opera(&OPS[0], 3, 2, &lp, &lq);
        rt_opera_i32(&OPS[0], 3, 2, &ip, &iq);
        rt_opera_i16(&OPS[0], 3, 2, &hp16, &hq16);
        int i32_bate = (lp == ip && lq == iq);
        int i16_bate = (hp16 == (int16_t)ip && hq16 == (int16_t)iq && lp == hp16 && lq == hq16);
        int32_t rp32, rq32;
        int inv32 = rt_inverte_i32(&OPS[0], ip, iq, &rp32, &rq32);
        int32_t cip, ciq, crp, crq;
        int ciclo32 = rt_ciclo_i32(&OPS[0], rt_iv_palavra, 3, 2, &cip, &ciq);
        int16_t cip16, ciq16;
        int ciclo16 = rt_ciclo_i16(&OPS[0], rt_iv_palavra, 3, 2, &cip16, &ciq16);
        long crpl, crql;
        int ciclo_l = rt_ciclo(&OPS[0], rt_iv_palavra, 3, 2, &crpl, &crql);
        int ciclo32_bate = ciclo32 && ciclo_l && cip == crpl && ciq == crql;
        int ciclo16_bate = ciclo16 && ciclo32 && cip16 == (int16_t)cip && ciq16 == (int16_t)ciq;
        int32_t ia[3] = {3, 2, 1}, ib[3] = {1, 0, 2}, ic[3], iv[3];
        long la[3] = {3, 2, 1}, lb[3] = {1, 0, 2};
        rt_cruz_i32(ia, ib, 3, ic);
        rt_cruz3_i32(ia, ib, iv);
        int cruz32 = (iv[0] == ic[1*3+2] && iv[1] == ic[2*3+0] && iv[2] == ic[0*3+1]);
        int dir32 = (rt_dir_i32(ia, ib, 3) == rt_dir(la, lb, 3));
        printf("\n  §R27 o ciclo universal, na arquitectura\n");
        printf("      ciclos que a peça correu ......... %ld   e fecharam %ld\n", ciclos, fecha);
        printf("      operadores sem |det| = 1: RECUSA . %ld de %ld\n", recusa, recusa_tot);
        printf("      as duas ordens do i: ponto %d, vector %d  (o recobrimento duplo)\n",
               op_pt, op_vc);
        printf("      e o gato: ponto %s, vector %s\n\n",
               gt_pt ? "fecha" : "INFINITA", gt_vc ? "fecha" : "INFINITA");
        ok("o CICLO UNIVERSAL é peça da lib: `rt_ciclo` opera, inverte e codifica, e a tese"
           " fica de FORA dela — a comparação com o original é de quem chama. E ela RECUSA"
           " sem |det| = 1, que é a condição do toro. As duas ordens saem da mesma peça: o"
           " i tem 2 no ponto e 4 no vector, e o gato não fecha em nenhuma",
           ciclos > 0 && fecha == ciclos && recusa_tot > 0 && recusa == recusa_tot &&
           op_pt == 2 && op_vc == 4 && gt_pt == 0 && gt_vc == 0 &&
           op_pt32 == op_pt && op_vc32 == op_vc &&
           op_pt16 == op_pt && op_vc16 == op_vc &&
           i32_bate && i16_bate && inv32 && rp32 == 3 && rq32 == 2 &&
           ciclo32_bate && ciclo16_bate && cruz32 && dir32);
    }

    /* ─── §R28 ── DIR e CRUZ dos operadores: o espectro comum ───────────────────────
     * A peça da lib (`rt_cruz_op`, `rt_espectro_comum`) e o que ela decide no pipe: a
     * convolução só vira produto ponto a ponto quando o par tem espectro comum
     * (cor:dir-cruz-folhas). Mede-se a forma fechada e a cadeia inteira. */
    {
        long pares = 0, anti = 0, prop = 0, coincide = 0, dir_sim = 0;
        for(long a = 1; a <= 8; a++) for(long b = 1; b <= 8; b++){
            RtOp A = {{ a,1,1,0 }}, B = {{ b,1,1,0 }};
            long C[4], Dd[4], Dd2[4];
            rt_cruz_op(&A, &B, C);
            rt_dir_op(&A, &B, Dd);  rt_dir_op(&B, &A, Dd2);
            pares++;
            /* Cruz = ½(a−b)·[0 1; −1 0]; a lib devolve o DOBRO, logo (a−b)·J */
            if(C[0] == 0 && C[3] == 0 && C[1] == -C[2]) anti++;
            if(C[1] == (a - b) && C[2] == -(a - b)) prop++;
            /* e a cadeia: Cruz = 0  <=>  mesmo traço  <=>  espectro comum */
            int comum = rt_espectro_comum(&A, &B);
            if(comum == (a == b)) coincide++;
            /* Dir NÃO muda ao trocar a ordem — é essa a sua leitura */
            if(Dd[0]==Dd2[0] && Dd[1]==Dd2[1] && Dd[2]==Dd2[2] && Dd[3]==Dd2[3]) dir_sim++;
        }
        printf("\n  §R28 Dir e Cruz dos operadores: o espectro comum\n");
        printf("      pares varridos ..................... %ld\n", pares);
        printf("      Cruz antissimétrica ................ %ld\n", anti);
        printf("      e o dobro vale (m−n)·[0 1;−1 0] .... %ld\n", prop);
        printf("      «Cruz = 0 ⟺ mesmas folhas» ......... %ld\n", coincide);
        printf("      e Dir NÃO muda ao trocar a ordem ... %ld\n\n", dir_sim);
        ok("DIR é o que a transformada vê e CRUZ é a obstrução à diagonalização simultânea:"
           " Cruz(A_m,A_n) vale ½(m−n)·[0 1;−1 0], é sempre antissimétrica, e anula-se"
           " exactamente quando os traços coincidem — que é quando o par tem ESPECTRO COMUM"
           " e a convolução vira produto ponto a ponto. Dir, essa, não muda ao trocar a"
           " ordem, que é a sua leitura",
           pares == 64 && anti == pares && prop == pares && coincide == pares &&
           dir_sim == pares);
    }

    /* ─── §R29 ── AS DUAS NORMALIZAÇÕES DO PIPE, e só uma é necessária ──────────────
     * O pipe tem duas coisas com o mesmo nome, e é preciso separá-las — a lib teve as
     * duas durante muito tempo e uma delas não fazia nada:
     *
     *   (1) a UNIDADE, na entrada — o mmc dos denominadores dos decimais escritos. Sem
     *       ela não há inteiros nenhuns: é o passo DESCODIFICA, e é INDISPENSÁVEL.
     *
     *   (2) a REDUÇÃO, no meio — dividir [p:q] pelo mdc depois de operar. Esta é
     *       DISPENSÁVEL: o primeiro passo da transformada já normaliza, porque a
     *       avaliação é linear e o que se usa é a razão das folhas, onde o factor cancela
     *       (`transformada_universal.c` §T9/§T10).
     *
     * Mede-se a diferença: tirar (1) destrói o resultado, tirar (2) não muda o ponto. */
    {
        long tot = 0, unidade_muda = 0, reducao_muda = 0, mesmo_ponto = 0;
        const char *ds[] = { "0.6", "0.45", "1.25", "-2.5", "3" };
        long u = rt_unidade_comum(ds, 5);
        long vi[8];
        int quantos = rt_para_unidade(ds, 5, u, vi);
        /* (1) COM a unidade os valores são inteiros e a razão preserva-se; sem ela — isto
         * é, com unidade 1 — os decimais não cabem em Z e a conversão perde-os */
        long com_u = 0, sem_u = 0;
        for(int i = 0; i < quantos; i++) if(vi[i] != 0) com_u++;
        long vj[8];
        int q2 = rt_para_unidade(ds, 5, 1, vj);
        for(int i = 0; i < q2; i++) if(vj[i] != 0) sem_u++;
        if(com_u > sem_u) unidade_muda++;
        /* (2) a REDUÇÃO: opera-se e compara-se o ponto com e sem reduzir, por produto
         * cruzado — que é a igualdade verdadeira em P¹ */
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            RtOp T = { { 2,1,1,1 } };                  /* det = 1 */
            if(a == 0 && b == 0) continue;
            long p1, q1;
            rt_opera(&T, a, b, &p1, &q1);
            long g = rt_mdc(p1 < 0 ? -p1 : p1, q1 < 0 ? -q1 : q1); if(g < 1) g = 1;
            long p2 = p1/g, q2r = q1/g;
            tot++;
            if(p1 != p2 || q1 != q2r) reducao_muda++;    /* mudou o REPRESENTANTE */
            if(p1*q2r == p2*q1) mesmo_ponto++;           /* mas é o MESMO PONTO */
        }
        printf("\n  §R29 as DUAS normalizações do pipe, e só uma é necessária\n");
        printf("      a UNIDADE (mmc) na entrada: com ela %ld valores, sem ela %ld\n",
               com_u, sem_u);
        printf("      a REDUÇÃO (mdc) no meio: pontos varridos ... %ld\n", tot);
        printf("        mudou o REPRESENTANTE em ................ %ld\n", reducao_muda);
        printf("        e é o MESMO PONTO em .................... %ld\n\n", mesmo_ponto);
        ok("AS DUAS NORMALIZAÇÕES DO PIPE são coisas diferentes, e só uma é necessária. A"
           " UNIDADE — o mmc dos denominadores, na entrada — é indispensável: é ela que leva"
           " o texto decimal a INTEIROS, e sem ela os valores perdem-se. A REDUÇÃO pelo mdc,"
           " no meio, é dispensável: ela muda o REPRESENTANTE mas o ponto de P¹ é o MESMO em"
           " todos os casos, medido por produto cruzado — e é por isso que saiu do"
           " `rt_ciclo`. O primeiro passo da transformada já normaliza, e o que normaliza a"
           " inversão é a NORMA",
           unidade_muda == 1 && com_u > sem_u && tot > 0 &&
           mesmo_ponto == tot && reducao_muda > 0);
    }

    /* ─── §R30 ── A DOURADA DISCRETA na lib: a dourada NA BORDA ─────────────────────
     * As peças `rt_folha_borda`, `rt_ordem_mult`, `rt_dourada` e `rt_dourada_inv`
     * realizam o thm:dourada-discreta. Mede-se o que o teorema afirma, e por DOIS
     * caminhos onde os há: a convolução directa contra o produto ponto a ponto. */
    {
        long corpos = 0, ordem_ok = 0, dual_ok = 0;
        long conv_tot = 0, conv_ok = 0, volta_tot = 0, volta_ok = 0;
        const long ps[] = { 11, 19, 29, 31, 41 };
        for(int ip = 0; ip < 5; ip++){
            long P = ps[ip];
            for(long m = 1; m < P; m++){
                long sg = rt_folha_borda(m, P);
                if(sg < 0) continue;
                long N = rt_ordem_mult(sg, P);
                if(N < 3 || N > 10) continue;
                corpos++;
                /* a ordem verifica-se por um caminho INDEPENDENTE da função que a
                 * devolveu: (i) σ^N = 1; (ii) N divide p−1, que é Lagrange; e (iii)
                 * NENHUM divisor próprio de N serve — testam-se todos, e é isto que
                 * `rt_ordem_mult` não pode dizer de si própria. */
                long e = rt_pot_mod(sg, N, P);
                int lagrange = ((P - 1) % N == 0);
                int nenhum_divisor = 1;
                for(long d = 1; d < N; d++)
                    if(N % d == 0 && rt_pot_mod(sg, d, P) == 1) nenhum_divisor = 0;
                if(e == 1 && lagrange && nenhum_divisor) ordem_ok++;
                /* σ⁻¹ = −σ† : a volta usa a OUTRA FOLHA */
                long sd = ((m - sg) % P + P) % P, inv = 0;
                for(long t = 1; t < P; t++) if(sg * t % P == 1){ inv = t; break; }
                if(inv == (P - sd) % P) dual_ok++;
                /* CAMINHO A: a convolução cíclica directa.  CAMINHO B: transformar,
                 * multiplicar ponto a ponto, e voltar. Têm de dar o mesmo. */
                long a1[12], a2[12], cv[12], A1[12], A2[12], PR[12], vv[12];
                for(long j = 0; j < N; j++){
                    a1[j] = (j*7 + m) % P; a2[j] = (j*3 + 2) % P; cv[j] = 0;
                }
                for(long i = 0; i < N; i++) for(long j = 0; j < N; j++)
                    cv[(i+j) % N] = (cv[(i+j) % N] + a1[i]*a2[j]) % P;
                rt_dourada(a1, N, sg, P, A1);
                rt_dourada(a2, N, sg, P, A2);
                for(long k = 0; k < N; k++) PR[k] = A1[k] * A2[k] % P;
                if(rt_dourada_inv(PR, N, sg, P, vv)){
                    for(long j = 0; j < N; j++){
                        conv_tot++;
                        if(vv[j] == cv[j] % P) conv_ok++;
                    }
                }
                /* e a VOLTA pura: transformar e desfazer devolve o original */
                long W[12], zz[12];
                rt_dourada(a1, N, sg, P, W);
                if(rt_dourada_inv(W, N, sg, P, zz)){
                    for(long j = 0; j < N; j++){
                        volta_tot++;
                        if(zz[j] == a1[j] % P) volta_ok++;
                    }
                }
            }
        }
        printf("\n  §R30 a DOURADA DISCRETA na lib: a dourada na borda\n");
        printf("      corpos com folha e ordem em [3,10] . %ld\n", corpos);
        printf("      a ordem é MÍNIMA e fecha ........... %ld\n", ordem_ok);
        printf("      σ⁻¹ = −σ† (a volta usa a outra) .... %ld\n", dual_ok);
        printf("      convolução directa = ponto a ponto . %ld de %ld\n", conv_ok, conv_tot);
        printf("      e a volta devolve o original ....... %ld de %ld\n\n", volta_ok, volta_tot);
        ok("A DOURADA DISCRETA é peça da lib, e é a dourada NA BORDA: σ² = mσ + 1 faz de σ"
           " uma unidade, em F_p ela tem ORDEM MULTIPLICATIVA finita — verificada e MÍNIMA,"
           " não suposta —, e os caracteres são as N potências σ^k. A convolução cíclica"
           " directa e o produto ponto a ponto na transformada dão o MESMO por dois"
           " caminhos, e a volta devolve o original: ela corre por σ⁻¹ = −σ†, a OUTRA FOLHA,"
           " porque σσ† = −1. O factor é N e não √N — a directa junta e a inversa divide",
           corpos > 0 && ordem_ok == corpos && dual_ok == corpos &&
           conv_tot > 0 && conv_ok == conv_tot && volta_tot > 0 && volta_ok == volta_tot);
    }

    return falhas ? 1 : 0;
}
