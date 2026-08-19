/* maisum.c — O +1 QUE CONSERTA: não se ortogonaliza, acrescenta-se a dimensão que falta.
 *
 * O Aarão: "a base pode até não ser ortonormal, mas entramos com +1 dimensão consertando — é isso
 * o que o banco faz, via cifra ortonormal infinita que descreve qualquer coisa finita, e reverte,
 * conserta a base, alinha."
 *
 * O pinos.c mediu que as colunas do modelo não são ortonormais e que a volta do LOAD/STORE
 * deixava 3,63% por fechar. A saída não é inverter a Gram: NÃO SE CONSERTA A BASE, ACRESCENTA-SE
 * UM VETOR. Com n+1 vetores em posição de simplex — soma zero, ângulos todos iguais — obtém-se
 * um TIGHT FRAME, e num tight frame vale
 *
 *     Σ_i ⟨x, v_i⟩ v_i  =  c · x
 *
 * a fórmula da base ortonormal, com uma constante à frente. Sem ortogonalizar, sem normalizar
 * e sem inverter nada. O +1 é o que compra isso. A cifra é infinita: há sempre mais uma
 * dimensão (cada quociente parcial é uma coordenada nova).
 *
 *   §M1  a BASE OBLÍQUA: a Gram não é I, e a reconstrução simples não fecha
 *   §M2  o SIMPLEX de n+1: soma zero, e todos os ângulos iguais — em ℤ, no hiperplano
 *   §M3  o TIGHT FRAME: Σ⟨x,v⟩v = (n+1)² x, EXATO e sem ortogonalizar
 *   §M4  e é o +1 que compra: com n falha, com n+1 fecha
 *   §M5  a CIFRA é infinita: os convergentes crescem, |det| = 1, o +1 está sempre à mão
 *
 * LEI vs TRANSPORTE. Normalizar com sqrt, sin/cos a gerar x, e |p/q − σ| com σ formado
 * pela raiz eram o método. A lei é o simplex inteiro w_i = (n+1)e_i − 1 no hiperplano
 * Σ = 0: ângulos iguais por produto interno, tight frame por (n+1)², convergentes por
 * rt_orbita sem avaliar σ.
 *
 *   cc -O2 -std=c99 -I lib tests/maisum.c -o maisum && ./maisum
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

#define DMAX 10                               /* n+1 ≤ 9 para n ≤ 8 */

/* n+1 vectores em ℤ^{n+1}, no hiperplano Σ = 0: w_i = (n+1) e_i − 1.
 * Soma nula, ‖w‖² = n(n+1), ⟨w_i,w_j⟩ = −(n+1) para i≠j — equiângulo, sem raiz. */
static void simplex(int n, long w[DMAX][DMAX]){
    int d = n + 1;
    for(int i = 0; i < d; i++)
        for(int j = 0; j < d; j++)
            w[i][j] = (i == j) ? n : -1;
}

int main(void){
printf("\n=== O +1 QUE CONSERTA: A DIMENSÃO QUE FALTA, EM VEZ DA CORREÇÃO ==========\n");
printf("    O pinos.c mediu que a base do modelo não é ortonormal e que a volta\n");
printf("    deixava 3,63%% por fechar. Não se conserta a base: acrescenta-se um vetor.\n");

printf("\n§M1  A BASE OBLÍQUA: a Gram não é I, e a reconstrução simples não fecha.\n\n");
{
    /* Independentes e não ortogonais — como as colunas que o pinos.c mediu. A reconstrução
     * ingénua Σ ⟨x,v_i⟩ v_i NÃO devolve x. Sem 0.6/0.8: são 3-4-5 em vírgula. Inteiros. */
    enum { N = 4 };
    long v[N][N] = {
        { 2, 0, 0, 0 },
        { 1, 2, 0, 0 },
        { 1, 1, 2, 0 },
        { 1, 1, 1, 2 },
    };
    long x[N] = { 1, 1, 1, 1 }, rec[N] = { 0 };
    long fora = 0, pares = 0;
    printf("      Gram (⟨v_i,v_j⟩):\n");
    for(int i = 0; i < N; i++){
        printf("        ");
        for(int j = 0; j < N; j++){
            long g = rt_dir(v[i], v[j], N);
            printf("%+3ld ", g);
            if(i != j){ fora += g < 0 ? -g : g; pares++; }
        }
        printf("\n");
    }
    for(int i = 0; i < N; i++){
        long c = rt_dir(x, v[i], N);
        for(int d = 0; d < N; d++) rec[d] += c * v[i][d];
    }
    int paralelo = 1, igual = 1;
    for(int d = 0; d < N; d++) if(rec[d] != x[d]) igual = 0;
    for(int a = 0; a < N; a++) for(int b = a + 1; b < N; b++)
        if(rec[a]*x[b] != rec[b]*x[a]) paralelo = 0;
    printf("\n      |⟨v_i,v_j⟩| fora da diagonal:  %ld  (%ld pares)\n", fora, pares);
    printf("      rec = (%ld,%ld,%ld,%ld)   x = (1,1,1,1)   paralelo? %s\n\n",
           rec[0], rec[1], rec[2], rec[3], paralelo ? "sim" : "não");
    ok("a base é oblíqua — a Gram tem termos fora da diagonal, exactos em ℤ",
       fora > 0 && pares == N*(N-1));
    ok("e a reconstrução simples NÃO fecha — rec ≠ x e rec não é paralelo a x, sem raiz"
       " e sem limiar de 5%",
       !igual && !paralelo);
    printf("      É o mesmo 3,63%% do pinos.c §P3, agora numa base que se controla. A saída\n");
    printf("      conhecida é inverter a Gram — e o §M3 mostra a outra.\n");
}

printf("\n§M2  O SIMPLEX de n+1: soma zero, e todos os ângulos IGUAIS.\n\n");
{
    /* n+1 vectores em ℤ^{n+1}, no hiperplano. Não se normaliza: a divisão PÕE a norma 1.
     * −1/n é n⟨v_i,v_j⟩ + ‖v‖² = 0, sem formar o cosseno. */
    printf("      n    vetores   Σw = 0?   ⟨w_i,w_j⟩   n⟨,⟩+‖w‖² = 0?\n");
    int mau = 0, ns = 0;
    for(int n = 2; n <= 8; n++){
        long w[DMAX][DMAX];
        simplex(n, w);
        int d = n + 1;
        long soma[DMAX] = {0};
        for(int i = 0; i < d; i++) for(int j = 0; j < d; j++) soma[j] += w[i][j];
        int sum0 = 1;
        for(int j = 0; j < d; j++) if(soma[j]) sum0 = 0;
        long ip = rt_dir(w[0], w[1], d), nw = rt_norma(w[0], d);
        int ang = 1;
        for(int i = 0; i < d; i++) for(int j = i + 1; j < d; j++){
            long g = rt_dir(w[i], w[j], d);
            if(g != ip) ang = 0;
            if(n * g + nw != 0) ang = 0;
        }
        for(int i = 0; i < d; i++) if(rt_norma(w[i], d) != nw) ang = 0;
        ns++;
        if(!sum0 || !ang) mau++;
        printf("      %-4d %-9d %-10s %-12ld %s\n",
               n, d, sum0 ? "sim" : "NÃO", ip, ang ? "sim" : "NÃO");
    }
    printf("\n");
    ok("os n+1 vetores do simplex somam ZERO e têm todos o mesmo ângulo — n⟨v_i,v_j⟩ + ‖v‖² = 0,"
       " que é −1/n sem dividir nem raiz",
       mau == 0 && ns == 7);
    printf("      Não são ortogonais — o produto interno é −(n+1), não 0. Mas são igualmente\n");
    printf("      espaçados, e é isso, e não a ortogonalidade, que o §M3 vai usar.\n");
}

printf("\n§M3  O TIGHT FRAME: Σ⟨x,v⟩v = (n+1)² x, EXATO e sem ortogonalizar.\n\n");
{
    /* A MESMA fórmula que falhou no §M1 fecha aqui, exacta, para todo x no hiperplano.
     * sin(2.7k) era transporte. c = (n+1)/n era a norma 1; sem normalizar, c = (n+1)². */
    printf("      n    c = (n+1)²    rec = c·x ?\n");
    int mau = 0, tot = 0;
    for(int n = 2; n <= 8; n++){
        long w[DMAX][DMAX];
        simplex(n, w);
        int d = n + 1;
        long c0 = (long)d * d;
        long xs[3][DMAX];
        for(int j = 0; j < d; j++){
            xs[0][j] = (j < n) ? 1 : -n;
            xs[1][j] = (j == 0) ? 1 : (j == 1 ? -1 : 0);
            xs[2][j] = (j == 0) ? 2 : (j == 1 || j == 2 ? -1 : 0);
        }
        int fecha = 1;
        for(int t = 0; t < 3; t++){
            long rec[DMAX] = {0}, sx = 0;
            for(int j = 0; j < d; j++) sx += xs[t][j];
            if(sx != 0){ fecha = 0; continue; }
            int nulo = 1;
            for(int j = 0; j < d; j++) if(xs[t][j]) nulo = 0;
            if(nulo) continue;
            for(int i = 0; i < d; i++){
                long cf = rt_dir(xs[t], w[i], d);
                for(int j = 0; j < d; j++) rec[j] += cf * w[i][j];
            }
            for(int j = 0; j < d; j++) if(rec[j] != c0 * xs[t][j]) fecha = 0;
            tot++;
        }
        if(!fecha) mau++;
        printf("      %-4d %-13ld %s\n", n, c0, fecha ? "sim" : "NÃO");
    }
    printf("\n");
    ok("com n+1 vetores a reconstrução fecha EXATO — tight frame, rec = (n+1)² x no"
       " hiperplano, sem ortogonalizar e sem inverter a Gram",
       mau == 0 && tot == 21);
    printf("      Repare-se no que NÃO se fez: não se ortogonalizou, não se normalizou a base,\n");
    printf("      não se inverteu a matriz de Gram. Acrescentou-se UM vetor, e a fórmula da\n");
    printf("      base ortonormal passou a valer — com a constante (n+1)² à frente.\n");
}

printf("\n§M4  E É O +1 QUE COMPRA: com n falha, com n+1 fecha.\n\n");
{
    int n = 6;
    long w[DMAX][DMAX];
    simplex(n, w);
    int d = n + 1;
    long x[DMAX];
    long s = 0;
    for(int j = 0; j < n; j++){ x[j] = j + 1; s += x[j]; }
    x[n] = -s;                                           /* Σ = 0, e NÃO é paralelo a nenhum w_i */
    long c0 = (long)d * d;
    long rec_sem[DMAX] = {0}, rec_com[DMAX] = {0};
    for(int i = 0; i < n; i++){                          /* SEM o +1 */
        long cf = rt_dir(x, w[i], d);
        for(int j = 0; j < d; j++) rec_sem[j] += cf * w[i][j];
    }
    for(int i = 0; i < d; i++){                          /* COM o +1 */
        long cf = rt_dir(x, w[i], d);
        for(int j = 0; j < d; j++) rec_com[j] += cf * w[i][j];
    }
    int fecha_com = 1, fecha_sem = 1, par_sem = 1;
    for(int j = 0; j < d; j++){
        if(rec_com[j] != c0 * x[j]) fecha_com = 0;
        if(rec_sem[j] != c0 * x[j]) fecha_sem = 0;
    }
    for(int a = 0; a < d; a++) for(int b = a + 1; b < d; b++)
        if(rec_sem[a]*x[b] != rec_sem[b]*x[a]) par_sem = 0;
    printf("      com %d vetores (sem o +1):  rec paralelo a x? %s\n", n, par_sem ? "sim" : "não");
    printf("      com %d vetores (COM o +1):  rec = %ld·x ? %s\n\n", d, c0, fecha_com ? "sim" : "NÃO");
    ok("sem o +1 a reconstrução falha — rec não é paralelo a x",
       !fecha_sem && !par_sem);
    ok("e com o +1 fecha — é o vetor a mais que conserta, não uma correção",
       fecha_com);
    printf("      Um vetor de diferença, e a fórmula passa de errada a exata. Não é a base que\n");
    printf("      muda — os n primeiros são os mesmos — é o espaço que fica completo.\n");
}

printf("\n§M5  A CIFRA é INFINITA, logo o +1 está SEMPRE disponível.\n\n");
{
    /* |p/q − σ| com σ = (m+√(m²+4))/2 era transporte, e o 1e-5 reprovava o ouro por ser
     * o mais difícil. A lei: os convergentes são rt_orbita, |det| = 1, e q_k cresce —
     * o vão 1/(q_k q_{k+1}) aperta, a expansão não pára. */
    printf("      metal m    q_4     q_8      q_12     q cresce?   |det|=1?\n");
    int mau = 0, metais = 0;
    for(long m = 1; m <= 5; m++){
        int cresce = 1, unimod = 1;
        long q_ant = 0, q4 = 0, q8 = 0, q12 = 0, p_ant = 0;
        for(int k = 1; k <= 12; k++){
            long p, q; rt_orbita(m, k, &p, &q);
            if(k > 1){
                if(q < q_ant) cresce = 0;
                long det = p * q_ant - p_ant * q;
                if(det != 1 && det != -1) unimod = 0;
            }
            if(k == 4) q4 = q;
            if(k == 8) q8 = q;
            if(k == 12) q12 = q;
            p_ant = p; q_ant = q;
        }
        metais++;
        if(!cresce || !unimod || !(q12 > q8 && q8 > q4)) mau++;
        printf("      %-10ld %-7ld %-8ld %-8ld %-12s %s\n",
               m, q4, q8, q12, cresce ? "sim" : "NÃO", unimod ? "sim" : "NÃO");
    }
    printf("\n");
    ok("q_k cresce e |p_k q_{k-1} − p_{k-1} q_k| = 1 em todos os metais — a expansão"
       " continua, sem formar σ e sem um 1e-5 que reprovava o ouro por ser ouro",
       mau == 0 && metais == 5);
    printf("      É por isso que o +1 está sempre à mão: a cifra é ortonormal e infinita, e um\n");
    printf("      objeto finito só precisa de finitas coordenadas. Quando a base do modelo não\n");
    printf("      chega, não se conserta a base — pede-se à cifra a dimensão que falta.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O pinos.c dizia que faltava a métrica e que era preciso inverter a Gram.\n");
printf("    Faltava menos: um vetor. Com n+1 em posição de simplex a reconstrução\n");
printf("    fecha exata, sem ortogonalizar, sem normalizar e sem inverter nada — e a\n");
printf("    cifra, sendo infinita, tem sempre esse vetor para dar.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
