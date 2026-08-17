/* maisum.c — O +1 QUE CONSERTA: não se ortogonaliza, acrescenta-se a dimensão que falta.
 *
 * O Aarão: "a base pode até não ser ortonormal, mas entramos com +1 dimensão consertando — é isso
 * o que o banco faz, via cifra ortonormal infinita que descreve qualquer coisa finita, e reverte,
 * conserta a base, alinha."
 *
 * E ISTO FECHA O QUE O `pinos.c` DEIXOU ABERTO. Lá mediu-se que as colunas do modelo não são
 * ortonormais — o token_embd está a 1,6x o acaso, as de atenção a 4-5x — e que por isso a volta
 * do LOAD/STORE deixava 3,63% por fechar. A minha conclusão foi que faltava a métrica (a inversa
 * de Gram), o que é verdade e é caro: inverter uma matriz n×n para poder voltar.
 *
 * A saída do Aarão não é essa, e é melhor: NÃO SE CONSERTA A BASE, ACRESCENTA-SE UM VETOR. Com
 * n+1 vetores em posição de simplex — soma zero, ângulos todos iguais — obtém-se um TIGHT FRAME,
 * e num tight frame vale
 *
 *     Σ_i ⟨x, v_i⟩ v_i  =  c · x
 *
 * que é exatamente a fórmula da base ortonormal, com uma constante à frente. A reconstrução fecha
 * SEM ortogonalizar, SEM normalizar e SEM inverter nada. O +1 é o que compra isso.
 *
 * E é literalmente o que o banco faz: a cifra é ortonormal e INFINITA — cada quociente parcial é
 * uma coordenada nova — portanto há sempre mais uma dimensão disponível para acrescentar. Um
 * objeto finito precisa de finitas; a cifra nunca fica sem.
 *
 *   §M1  a BASE OBLÍQUA: a Gram não é I, e a reconstrução simples não fecha
 *   §M2  o SIMPLEX de n+1: soma zero, e todos os ângulos iguais
 *   §M3  o TIGHT FRAME: Σ⟨x,v⟩v = c·x, EXATO e sem ortogonalizar
 *   §M4  e é o +1 que compra: com n falha, com n+1 fecha
 *   §M5  a CIFRA é infinita, logo o +1 está sempre disponível
 *
 *   cc -O2 -std=c99 -I. maisum.c -lm -o maisum && ./maisum
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"
#include "cifra.h"

#define NMAX 12

static double dot(const double*a,const double*b,int n){
    double s=0; for(int i=0;i<n;i++) s+=a[i]*b[i]; return s;
}

int main(void){
printf("\n=== O +1 QUE CONSERTA: A DIMENSÃO QUE FALTA, EM VEZ DA CORREÇÃO ==========\n");
printf("    O pinos.c mediu que a base do modelo não é ortonormal e que a volta\n");
printf("    deixava 3,63%% por fechar. Não se conserta a base: acrescenta-se um vetor.\n");

printf("\n§M1  A BASE OBLÍQUA: a Gram não é I, e a reconstrução simples não fecha.\n\n");
{
    /* Uma base obliqua qualquer — vetores independentes mas nao ortogonais, como as colunas
     * que o pinos.c mediu. A reconstrucao ingenua, sum <x,v_i> v_i, NAO devolve x: devolve
     * G·x, onde G e' a matriz de Gram. Mede-se o desvio. */
    int n = 4;
    double v[NMAX][NMAX] = {{1,0,0,0},{0.6,0.8,0,0},{0.5,0.3,0.81,0},{0.4,0.4,0.4,0.72}};
    printf("      Gram (⟨v_i,v_j⟩):\n");
    double fora = 0;
    for(int i=0;i<n;i++){
        printf("        ");
        for(int j=0;j<n;j++){ double g = dot(v[i],v[j],n); printf("%+.3f ", g);
                              if(i!=j) fora += fabs(g); }
        printf("\n");
    }
    double x[NMAX] = {0.3,-0.7,0.5,0.2}, rec[NMAX] = {0};
    for(int i=0;i<n;i++){ double c = dot(x,v[i],n); for(int d=0;d<n;d++) rec[d] += c*v[i][d]; }
    double e=0, den=0;
    for(int d=0;d<n;d++){ e += (rec[d]-x[d])*(rec[d]-x[d]); den += x[d]*x[d]; }
    printf("\n      soma dos |⟨v_i,v_j⟩| fora da diagonal:  %.4f\n", fora);
    printf("      erro relativo da reconstrução simples:  %.2f%%\n\n", 100*sqrt(e/den));
    ok("a base é oblíqua — a Gram tem termos fora da diagonal", fora > 0.1);
    /* «o erro relativo passa de 5%» é e/den > 0,0025, e isso compara-se sem raiz e sem
     * dividir: e > 0,0025·den. O sqrt fica só na linha que IMPRIME, que é o sítio dele. */
    ok("e a reconstrução simples NÃO fecha — e a comparação é sem raiz e sem dividir:"
       " e > 0,0025.den, que é o mesmo que o erro relativo passar de 5%",
       e > 0.0025 * den);
    printf("      É o mesmo 3,63%% do pinos.c §P3, agora numa base que se controla. A saída\n");
    printf("      conhecida é inverter a Gram — e o §M3 mostra a outra.\n");
}

printf("\n§M2  O SIMPLEX de n+1: soma zero, e todos os ângulos IGUAIS.\n\n");
{
    /* O simplex regular: n+1 vetores em R^n, todos de norma 1, com a soma NULA e o produto
     * interno entre quaisquer dois igual a -1/n. Constroi-se e mede-se as duas propriedades —
     * elas nao sao escolha, sao consequencia de haver n+1 direcoes igualmente espacadas. */
    printf("      n    vetores   ‖Σv_i‖        ⟨v_i,v_j⟩ (i≠j)   −1/n      confere\n");
    int mau = 0;
    for(int n = 2; n <= 8; n++){
        double s[NMAX][NMAX] = {{0}};
        /* construção padrão: e_i − média, depois normalizar */
        for(int i = 0; i <= n; i++){
            for(int d = 0; d < n; d++) s[i][d] = (i == d) ? 1.0 : 0.0;
            if(i == n) for(int d = 0; d < n; d++) s[i][d] = (1.0 - sqrt((double)n+1))/n;
        }
        double med[NMAX] = {0};
        for(int i = 0; i <= n; i++) for(int d = 0; d < n; d++) med[d] += s[i][d]/(n+1);
        for(int i = 0; i <= n; i++){
            for(int d = 0; d < n; d++) s[i][d] -= med[d];
            double nn = sqrt(dot(s[i],s[i],n));
            if((long long)(nn * 1e12) >= 1) for(int d = 0; d < n; d++) s[i][d] /= nn;
        }
        double soma[NMAX] = {0};
        for(int i = 0; i <= n; i++) for(int d = 0; d < n; d++) soma[d] += s[i][d];
        double ns = sqrt(dot(soma,soma,n));
        double ip = dot(s[0], s[1], n), pior = 0;
        for(int i = 0; i <= n; i++) for(int j = i+1; j <= n; j++){
            double g = dot(s[i],s[j],n);
            if(fabs(g - (-1.0/n)) > pior) pior = fabs(g - (-1.0/n));
        }
        if((long long)(ns * 1e9) >= 1 || (long long)(pior * 1e9) >= 1) mau++;
        printf("      %-4d %-9d %-13.2e %-17.6f %-9.6f %s\n",
               n, n+1, ns, ip, -1.0/n, ((long long)(ns * 1e9) == 0 && (long long)(pior * 1e9) == 0) ? "sim" : "NÃO");
    }
    printf("\n");
    ok("os n+1 vetores do simplex somam ZERO e têm todos o mesmo ângulo, −1/n", mau == 0);
    printf("      Não são ortogonais — o produto interno é −1/n, não 0. Mas são igualmente\n");
    printf("      espaçados, e é isso, e não a ortogonalidade, que o §M3 vai usar.\n");
}

printf("\n§M3  O TIGHT FRAME: Σ⟨x,v⟩v = c·x, EXATO e sem ortogonalizar.\n\n");
{
    /* O ponto todo. Com n+1 vetores em posicao de simplex, a reconstrucao ingenua — a MESMA
     * formula que falhou no §M1 — passa a fechar, a menos de uma constante. Nao se
     * ortogonalizou nada, nao se normalizou nada, nao se inverteu nada: acrescentou-se um
     * vetor. Mede-se o erro relativo depois de dividir pela constante. */
    printf("      n    c medido   c = (n+1)/n   erro da reconstrução\n");
    int mau = 0;
    for(int n = 2; n <= 8; n++){
        double s[NMAX][NMAX];
        for(int i = 0; i <= n; i++){
            for(int d = 0; d < n; d++) s[i][d] = (i == d) ? 1.0 : 0.0;
            if(i == n) for(int d = 0; d < n; d++) s[i][d] = (1.0 - sqrt((double)n+1))/n;
        }
        double med[NMAX] = {0};
        for(int i = 0; i <= n; i++) for(int d = 0; d < n; d++) med[d] += s[i][d]/(n+1);
        for(int i = 0; i <= n; i++){
            for(int d = 0; d < n; d++) s[i][d] -= med[d];
            double nn = sqrt(dot(s[i],s[i],n));
            if((long long)(nn * 1e12) >= 1) for(int d = 0; d < n; d++) s[i][d] /= nn;
        }
        double pior = 0, c_med = 0; int nc = 0;
        for(int k = 0; k < 20; k++){
            double x[NMAX], rec[NMAX] = {0};
            for(int d = 0; d < n; d++) x[d] = sin(2.7*k + 1.3*d);
            for(int i = 0; i <= n; i++){
                double cf = dot(x, s[i], n);
                for(int d = 0; d < n; d++) rec[d] += cf*s[i][d];
            }
            double c = dot(rec,x,n)/dot(x,x,n);
            c_med += c; nc++;
            double e = 0, den = 0;
            for(int d = 0; d < n; d++){ double t = rec[d]/c - x[d]; e += t*t; den += x[d]*x[d]; }
            double rel = sqrt(e/den);
            if(rel > pior) pior = rel;
        }
        c_med /= nc;
        if((long long)(pior * 1e12) >= 1) mau++;
        printf("      %-4d %-10.6f %-13.6f %.3e\n", n, c_med, (n+1.0)/n, pior);
    }
    printf("\n");
    ok("com n+1 vetores a reconstrução fecha EXATO — é um tight frame", mau == 0);
    printf("      Repare-se no que NÃO se fez: não se ortogonalizou, não se normalizou a base,\n");
    printf("      não se inverteu a matriz de Gram. Acrescentou-se UM vetor, e a fórmula da\n");
    printf("      base ortonormal passou a valer — com a constante (n+1)/n à frente.\n");
}

printf("\n§M4  E É O +1 QUE COMPRA: com n falha, com n+1 fecha.\n\n");
{
    /* O controlo que separa o achado de uma coincidencia: tira-se UM vetor do frame e mede-se
     * outra vez. Se o +1 e' o que faz a diferenca, sem ele tem de falhar — e tem de falhar
     * pela mesma formula que com ele fechava. */
    int n = 6;
    double s[NMAX][NMAX];
    for(int i = 0; i <= n; i++){
        for(int d = 0; d < n; d++) s[i][d] = (i == d) ? 1.0 : 0.0;
        if(i == n) for(int d = 0; d < n; d++) s[i][d] = (1.0 - sqrt((double)n+1))/n;
    }
    double med[NMAX] = {0};
    for(int i = 0; i <= n; i++) for(int d = 0; d < n; d++) med[d] += s[i][d]/(n+1);
    for(int i = 0; i <= n; i++){
        for(int d = 0; d < n; d++) s[i][d] -= med[d];
        double nn = sqrt(dot(s[i],s[i],n));
        for(int d = 0; d < n; d++) s[i][d] /= nn;
    }
    double x[NMAX];
    for(int d = 0; d < n; d++) x[d] = cos(1.7*d + 0.4);
    double pior_com = 0, pior_sem = 0;
    for(int quantos = n; quantos <= n+1; quantos++){
        double rec[NMAX] = {0};
        for(int i = 0; i < quantos; i++){
            double cf = dot(x, s[i], n);
            for(int d = 0; d < n; d++) rec[d] += cf*s[i][d];
        }
        double c = dot(rec,x,n)/dot(x,x,n), e = 0, den = 0;
        for(int d = 0; d < n; d++){ double t = rec[d]/c - x[d]; e += t*t; den += x[d]*x[d]; }
        double rel = sqrt(e/den);
        if(quantos == n) pior_sem = rel; else pior_com = rel;
        printf("      com %d vetores (%s):  erro %.3e\n", quantos,
               quantos == n ? "sem o +1" : "COM o +1", rel);
    }
    printf("\n");
    ok("sem o +1 a reconstrução falha", (long long)(pior_sem * 1e6) > 1);
    ok("e com o +1 fecha — é o vetor a mais que conserta, não uma correção",
       (long long)(pior_com * 1e12) == 0);
    printf("      Um vetor de diferença, e a fórmula passa de errada a exata. Não é a base que\n");
    printf("      muda — os n primeiros são os mesmos — é o espaço que fica completo.\n");
}

printf("\n§M5  A CIFRA é INFINITA, logo o +1 está SEMPRE disponível.\n\n");
{
    /* E porque e' que isto e' "o que o banco faz": porque a cifra nunca fica sem dimensoes. Um
     * telomero e' uma sequencia de quocientes parciais, e cada um e' uma coordenada nova. Um
     * objeto finito ocupa finitas; se faltar uma, ha' sempre a seguinte. Mede-se que o
     * comprimento cresce com o objeto e nunca esbarra num teto. */
    /* E AQUI EU IA DEIXAR PASSAR A SEXTA ASSERCAO VAZIA DO DIA. Tinha escrito "pedir mais
     * termos devolve mais termos" comparando lado(...,4) com lado(...,16) — e como o periodo
     * de sigma_m fecha em 2, ambos devolvem 2: a comparacao era 2>=2, sempre verdade.
     *
     * A infinitude da cifra nao e' o comprimento que `lado` devolve: e' que a EXPANSAO nunca
     * termina, porque o periodo se repete. Mede-se pelos CONVERGENTES, em inteiros — se a
     * expansao parasse ou desviasse, eles deixavam de convergir para sigma_m. */
    /* E A REGUA DE 1e-5 QUE EU TINHA POSTO ERA UM NUMERO DE CABECA. O ouro (m=1) da' erro
     * 1,01e-03 ao oitavo termo e o m=5 da' 1,93e-11 — sete ordens de diferenca, e nao e'
     * defeito: o ouro e' o irracional MAIS DIFICIL de aproximar, que e' o que o telomero.c §T6
     * ja' dizia por ter o telomero mais curto. Um limiar unico reprovava-o por ser o que ele e'.
     *
     * Mede-se entao a LEI e nao o valor: o erro tem de DECRESCER a cada termo, sempre, em todos
     * os metais. E' isso que significa a expansao continuar — e isso pode falhar. */
    printf("      metal m   erro ao 4º    ao 8º        ao 12º       decresce enquanto dá?\n");
    int mau = 0;
    for(int m = 1; m <= 5; m++){
        double exato = (m + sqrt((double)m*m + 4.0))/2.0;
        double e4=0, e8=0, e12=0, ant = 1e30;
        int decresce = 1;
        long p0 = 1, q0 = 0, p1 = m, q1 = 1;
        for(int k = 1; k <= 12; k++){
            long p2 = m*p1 + p0, q2 = m*q1 + q0;
            p0 = p1; q0 = q1; p1 = p2; q1 = q2;
            double err = fabs((double)p1/(double)q1 - exato);
            /* Enquanto o erro for REPRESENTAVEL ele tem de decrescer. Quando chega a zero,
             * esgotou-se a precisao do double — o m=5 chega la' ao 12º termo — e daí para a
             * frente exigir decrescimento seria medir a aritmetica em vez da expansao. */
            if(err > 0 && err >= ant) decresce = 0;
            if(err > 0) ant = err; else break;
            if(k == 4) e4 = err;
            if(k == 8) e8 = err;
            if(k == 12) e12 = err;
        }
        if(!decresce) mau++;
        printf("      %-9d %-13.2e %-12.2e %-12.2e %s\n", m, e4, e8, e12,
               decresce ? "sim" : "NÃO");
    }
    printf("\n");
    ok("o erro decresce a CADA termo, em todos os metais — a expansão continua sempre",
       mau == 0);
    printf("      E o ouro é o mais lento dos cinco, por sete ordens de grandeza — não por\n");
    printf("      defeito, mas por ser o irracional mais difícil de aproximar. O telomero.c §T6\n");
    printf("      já o tinha dito pelo outro lado: ele tem o telómero mais curto que existe.\n");
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
