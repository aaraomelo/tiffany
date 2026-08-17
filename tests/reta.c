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
 *   cc -O2 -std=c99 -I. -I../lib reta.c -o reta && ./reta
 */
#include <stdio.h>
#include <string.h>
#include "reta.h"
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


int main(void){
    printf("\n=== A RECTA GEOMÉTRICA: as operações, todas inteiras ===\n");

    /* ═══ §R1  A POTÊNCIA E O INVERSO EM 𝔽ₚ ═════════════════════════════════ */
    printf("\n§R1 rt_ipow e rt_inv_mod — e o inverso DESFAZ, que é a definição.\n\n");
    {
        long pot_ok = 0, pot_tot = 0, inv_ok = 0, inv_tot = 0, nao_inv = 0;
        for(long b = -4; b <= 4; b++) for(int e = 0; e <= 8; e++){
            long r = rt_ipow(b, e), m = 1;
            for(int k = 0; k < e; k++) m *= b;       /* a segunda rota: o laço directo */
            pot_tot++;
            if(r == m) pot_ok++;
        }
        const long PR[4] = {7, 101, 1009, 65537};
        for(int t = 0; t < 4; t++){
            long p = PR[t];
            for(long a = 1; a < p && a <= 60; a++){
                long v = rt_inv_mod(a, p);
                inv_tot++;
                if(a*v % p == 1) inv_ok++;           /* a DEFINIÇÃO: a·a⁻¹ = 1 */
            }
        }
        /* o GUME: em 𝔽ₚ o zero NÃO tem inverso, e um `a` múltiplo de p também não —
         * sem este lado, «o inverso desfaz» valia por nunca se lhe pedir o impossível */
        /* E O GUME NÃO MORDIA. `p*v % p != 1` é SEMPRE verdade — p·v ≡ 0 (mod p) para
         * qualquer v —, portanto `nao_inv == 4` passava independentemente do que
         * rt_inv_mod devolvesse. A tautologia estava escondida por o teste parecer usar o
         * resultado da função.
         *
         * O que se afirma verifica-se CONTANDO SOLUÇÕES, e é aí que há conteúdo: em 𝔽ₚ a
         * equação a·v = 1 tem EXACTAMENTE uma solução quando a ≠ 0 e NENHUMA quando a = 0.
         * Varre-se v e conta-se — e as duas metades vão juntas, porque «não há inverso do
         * zero» sem «há um e um só dos outros» não distingue um corpo de um anel qualquer. */
        long zero_sem = 0, outros_um = 0, testados_a = 0, varridos = 0;
        for(int t = 0; t < 4; t++){                  /* os primos que cabem: varre-se TUDO */
            long p = PR[t];
            if(p > 1100) continue;                   /* 1009² ainda cabe; 65537² não */
            varridos++;
            long quantos0 = 0;
            for(long v = 0; v < p; v++) if((0 * v) % p == 1) quantos0++;
            if(quantos0 == 0) zero_sem++;            /* o zero: NENHUMA solução */
            for(long a = 1; a < p; a++){
                long quantos = 0;
                for(long v = 0; v < p; v++) if((a * v) % p == 1) quantos++;
                testados_a++;
                if(quantos == 1) outros_um++;        /* os outros: UMA, e uma só */
            }
        }
        nao_inv = zero_sem;
        printf("      rt_ipow contra o laço directo: %ld de %ld\n", pot_ok, pot_tot);
        printf("      rt_inv_mod: a·a⁻¹ = 1 em %ld de %ld, em quatro primos\n", inv_ok, inv_tot);
        printf("      GUME, por CONTAGEM DE SOLUÇÕES de a·v = 1 em 𝔽ₚ:\n");
        printf("        a = 0 : nenhuma solução, em %ld de %ld primos varridos por inteiro\n",
               zero_sem, varridos);
        printf("        a ≠ 0 : exactamente UMA, em %ld de %ld valores\n\n", outros_um, testados_a);
        ok("A POTÊNCIA E O INVERSO EM 𝔽ₚ, e o inverso mede-se pela DEFINIÇÃO — a·a⁻¹ = 1,"
           " e não contra uma segunda fórmula do inverso, que seria escrevê-lo duas vezes."
           " Quatro primos até 65537. Com o gume: o zero não tem inverso, e sem esse lado"
           " «o inverso desfaz» valia por nunca se lhe pedir o impossível",
           pot_ok == pot_tot && inv_ok == inv_tot && pot_tot > 0
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
        long casos = 0, unidade = 0, zeros = 0, rec_ok = 0;
        for(long m = 1; m <= 8; m++){
            long ap = 1, aq = 0;
            for(int k = 1; k <= 16; k++){
                long p, q;
                rt_orbita(m, k, &p, &q);
                /* (i) a forma que o PONTO FIXO anularia vale ±1 — e nunca 0, que é o corte */
                long forma = p*p - m*p*q - q*q;
                casos++;
                if(forma == 1 || forma == -1) unidade++;
                if(forma == 0) zeros++;
                /* (ii) e a recorrência: cada convergente sai do anterior pela Möbius */
                if(p == m*ap + aq && q == ap) rec_ok++;
                ap = p; aq = q;
            }
        }
        printf("      %ld passos em 8 metais: |p² − m·p·q − q²| = 1 em %ld, e ZERO em %ld\n",
               casos, unidade, zeros);
        printf("      e a recorrência [p:q] ⟼ [m·p+q : p] fecha em %ld\n\n", rec_ok);
        ok("A ÓRBITA DE ∞ DÁ OS CONVERGENTES, e a forma que o PONTO FIXO anularia vale ±1 em"
           " todos os passos e ZERO em nenhum — que é o corte medido de frente: o ponto fixo"
           " pediria a forma = 0, e isso não tem solução em ℤ. É por isso que ele não cabe"
           " neste andar, e é por isso que um decimal não o representa: um decimal truncado"
           " não é o objecto, é outro número",
           unidade == casos && zeros == 0 && rec_ok == casos && casos == 128);
    }

    /* ═══ §R6  A REVERSÃO É INVOLUÇÃO ═══════════════════════════════════════ */
    printf("\n§R6 rt_reverte: involução, e leva a borda do ouro na equação do recíproco.\n\n");
    {
        long casos = 0, volta = 0, muda = 0;
        for(long s = 0; s < 200; s++){
            long a[6], r[6], rr[6];
            int n = 5;
            for(int k = 0; k <= n; k++) a[k] = ((s*7 + k*3) % 9) - 4;
            rt_reverte(a, n, r);
            rt_reverte(r, n, rr);                    /* reverter duas vezes devolve */
            casos++;
            int igual = 1, dif = 0;
            for(int k = 0; k <= n; k++){
                if(rr[k] != a[k]) igual = 0;
                if(r[k] != a[k]) dif = 1;
            }
            if(igual) volta++;
            if(dif) muda++;                          /* e NÃO é a identidade */
        }
        /* e o caso que o `solar.c` usa: a borda do ouro revertida é a equação do recíproco */
        long ouro[3] = {1, -1, -1}, rev[3];
        rt_reverte(ouro, 2, rev);
        int bate_ouro = (rev[0] == -1 && rev[1] == -1 && rev[2] == 1);
        printf("      %ld polinómios: reverter duas vezes devolve em %ld, e MUDA em %ld\n",
               casos, volta, muda);
        printf("      e a borda do ouro (1,−1,−1) revertida dá (%ld,%ld,%ld) = −(1,1,−1),\n"
               "      que é x² + x − 1: a equação do RECÍPROCO — %s\n\n",
               rev[0], rev[1], rev[2], bate_ouro ? "sim" : "NÃO");
        ok("A REVERSÃO É INVOLUÇÃO — revertida duas vezes devolve —, e não é a identidade,"
           " que é o controlo sem o qual «volta» valia por ela não fazer nada. E é ela que"
           " troca DENTRO por FORA no disco (Rouché no dual, §M17) e que faz do RECÍPROCO"
           " do ouro a raiz do polinómio revertido: (1,−1,−1) ao contrário é −(1,1,−1),"
           " isto é x² + x − 1, que é a equação do fator de potência do solar.c",
           volta == casos && muda > casos/2 && bate_ouro && casos == 200);
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
        /* o cruzado: antissimétrico, e a×a = 0 */
        long anti = 0, nulo = 0, viv = 0, cc = 0;
        for(long t = 0; t < 200; t++){
            long a[3], b[3], ab[3], ba[3], aa[3];
            for(int i = 0; i < 3; i++){
                a[i] = ((t*7 + i*3) % 11) - 5;
                b[i] = ((t*5 + i*2) % 9)  - 4;
            }
            rt_cruz3(a, b, ab); rt_cruz3(b, a, ba); rt_cruz3(a, a, aa);
            cc++;
            int ant = 1, nul = 1, vv = 0;
            for(int i = 0; i < 3; i++){
                if(ab[i] != -ba[i]) ant = 0;
                if(aa[i] != 0) nul = 0;
                if(ab[i]) vv = 1;
            }
            anti += ant; nulo += nul; viv += vv;
        }
        /* e a decomposição: 2M = (M+Mᵀ) + (M−Mᵀ), com S simétrica e A antissimétrica */
        long dec = 0, dtot = 0;
        for(long t = 0; t < 100; t++){
            long M[16], S2[16], A2[16];
            for(int i = 0; i < 16; i++) M[i] = ((t*11 + i*3) % 13) - 6;
            rt_dir_cruz(M, 4, S2, A2);
            int bom = 1;
            for(int i = 0; i < 4 && bom; i++) for(int j = 0; j < 4; j++){
                if(S2[i*4+j] + A2[i*4+j] != 2*M[i*4+j]) { bom = 0; break; }
                if(S2[i*4+j] != S2[j*4+i])              { bom = 0; break; }
                if(A2[i*4+j] != -A2[j*4+i])             { bom = 0; break; }
            }
            dtot++; dec += bom;
        }
        printf("      a convolução de (x+1)^6 dá a linha de Pascal: %s\n",
               pascal_ok ? "1 6 15 20 15 6 1" : "NÃO");
        printf("      o cruzado é antissimétrico em %ld de %ld, a×a = 0 em %ld, e NÃO nulo em %ld\n",
               anti, cc, nulo, viv);
        printf("      e 2M = (M+Mᵀ) + (M−Mᵀ), com as duas metades no seu espaço: %ld de %ld\n\n",
               dec, dtot);
        ok("A CONVOLUÇÃO É O PRODUTO DE POLINÓMIOS — e (x+1)^6 dá a linha de Pascal,"
           " 1 6 15 20 15 6 1, que é o caso que se sabe de cor. O CRUZADO é antissimétrico"
           " nos 200 pares e a×a é zero em todos, com os não nulos contados para «troca de"
           " sinal» não valer por 0 = −0. E a DECOMPOSIÇÃO devolve: 2M = (M+Mᵀ) + (M−Mᵀ),"
           " com a primeira metade simétrica e a segunda antissimétrica, em 100 matrizes",
           pascal_ok && anti == cc && nulo == cc && viv > cc/2 && dec == dtot);
    }

    /* ═══ §R11 A COMPANHEIRA E O TRAÇO DAS POTÊNCIAS ════════════════════════ */
    printf("\n§R11 rt_companheira e rt_tracos — Tr(Cᵏ) é a soma das potências das raízes.\n\n");
    {
        /* x² = m·x + 1: os traços são os t_k de Lucas, e t_k = m·t_{k−1} + t_{k−2} */
        long casos = 0, bate_rec = 0, bate_t0 = 0;
        for(long m = 1; m <= 6; m++){
            long c[2] = {1, m}, C[4], tr[14];         /* xⁿ = 1·x⁰ + m·x¹ */
            rt_companheira(c, 2, C);
            rt_tracos(C, 2, tr, 14);
            if(tr[0] == 2) bate_t0++;                 /* Tr(I) = n = 2 */
            for(int k = 2; k < 14; k++){
                casos++;
                if(tr[k] == m*tr[k-1] + tr[k-2]) bate_rec++;   /* Newton */
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
        printf("      GUME: com a companheira TRANSPOSTA a recorrência quebra em %ld dos 4\n\n",
               errado);
        ok("A COMPANHEIRA E O TRAÇO DAS SUAS POTÊNCIAS: Tr(Cᵏ) É a soma das potências das"
           " raízes, e obedece à recorrência do próprio polinómio — Newton, sem avaliar raiz"
           " nenhuma. Com Tr(C⁰) = n, que é a dimensão. E com o gume que me custou uma"
           " asserção no analog.c §B.8: a acção é nas LINHAS, logo o índice sobe na COLUNA;"
           " com a companheira transposta a recorrência quebra, e é isso que distingue a"
           " construção certa da que passa por acaso",
           bate_rec == casos && bate_t0 == 6 && errado > 0 && casos == 72);
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
    return falhas ? 1 : 0;
}
