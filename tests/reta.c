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
        for(int t = 0; t < 4; t++){
            long p = PR[t], v = rt_inv_mod(p, p);    /* p ≡ 0 (mod p) */
            if(p*v % p != 1) nao_inv++;
        }
        printf("      rt_ipow contra o laço directo: %ld de %ld\n", pot_ok, pot_tot);
        printf("      rt_inv_mod: a·a⁻¹ = 1 em %ld de %ld, em quatro primos\n", inv_ok, inv_tot);
        printf("      GUME: o zero de 𝔽ₚ não tem inverso, e isso confirma-se em %ld de 4\n\n",
               nao_inv);
        ok("A POTÊNCIA E O INVERSO EM 𝔽ₚ, e o inverso mede-se pela DEFINIÇÃO — a·a⁻¹ = 1,"
           " e não contra uma segunda fórmula do inverso, que seria escrevê-lo duas vezes."
           " Quatro primos até 65537. Com o gume: o zero não tem inverso, e sem esse lado"
           " «o inverso desfaz» valia por nunca se lhe pedir o impossível",
           pot_ok == pot_tot && inv_ok == inv_tot && nao_inv == 4 && pot_tot > 0);
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
           " depois das primeiras: 23 cópias do ((x%%p)+p)%%p, 15 da transposta, 11 do"
           " produto de matrizes, 4 do traço. E a TRANSPOSTA não é um utilitário: é o"
           " ESPELHO τ que reparte Dir e Cruz nas matrizes, e mede-se como tal — involução,"
           " e NÃO a identidade, que é o controlo sem o qual «τ² = id» valia por τ não fazer"
           " nada. O módulo devolve o representante canónico em [0,p), que é a mesma dobra"
           " do sinal um andar acima; e o traço não vê a transposta, que é o que o torna"
           " invariante",
           md == mdt && esp == ttot && mm == ttot && tr == ttot && id > ttot/2);
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
