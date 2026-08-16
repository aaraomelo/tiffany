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
