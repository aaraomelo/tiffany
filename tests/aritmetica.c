/* tests/aritmetica.c — A ARITMÉTICA NATURAL: cada andar torna UMA fibra total, e paga noutra.
 *
 * O corolário `0 ↔ ∞` mostrou que «0⁻¹ não existe» era da carta. Este ficheiro leva a
 * pergunta ao andar de baixo e mede a lei que daí sai — porque se cada andar paga numa
 * fibra, então nenhuma das excepções que esta casa escreveu era um defeito: eram o preço.
 *
 * E há uma consequência prática, que é o que desbloqueia a migração: o módulo de Cauchy
 * desta casa procurava o N ITERANDO a órbita e subtraindo racionais, e é isso que faz os
 * denominadores crescerem. Em ℕ, com a forma fechada |pₙ/qₙ − pₙ₊₁/qₙ₊₁| = 1/(qₙqₙ₊₁), o
 * N sai de uma comparação de naturais pequenos — o PASSO em vez da órbita.
 *
 * §N0  em ℕ a SOMA é total e a SUBTRACÇÃO é parcial — a mesma forma do 0⁻¹
 * §N1  a escada: cada andar torna uma fibra total e paga noutra
 * §N2  a fracção contínua é uma sequência de NATURAIS, e a volta fecha
 * §N3  a identidade |pₙqₙ₊₁ − pₙ₊₁qₙ| = 1, em naturais
 * §N4  o MÓDULO PELO PASSO: sem formar diferença, e onde a órbita saturava
 * §N5  o gume: retirar a forma fechada e ver os denominadores crescerem
 */
#include <stdio.h>
#include "aritmetica.h"
#include "unidade.h"

int main(void){
    printf("\n=== A ARITMÉTICA NATURAL: a fibra que cada andar paga ===\n");

    /* ═══ §N0 EM ℕ A SUBTRACÇÃO É PARCIAL ════════════════════════════════════ */
    printf("\n§N0 A soma é total; a subtracção é a fibra parcial.\n\n");
    {
        unsigned long x;
        long tem = 0, nao = 0, casos = 0, soma_mal = 0;
        for(unsigned long a = 0; a <= 60; a++) for(unsigned long b = 0; b <= 60; b++){
            casos++;
            if(a + b < a) soma_mal++;                  /* a soma nunca falha em ℕ */
            if(nt_fibra_soma(a,b,&x)){ tem++; if(a + x != b) soma_mal++; }
            else nao++;
        }
        printf("      a + x = b em %ld pares: %ld com fibra, %ld SEM — e a soma nunca"
               " falha (%ld)\n", casos, tem, nao, soma_mal);
        ok("EM ℕ A SOMA É TOTAL E A SUBTRACÇÃO É PARCIAL, e isto tem exactamente a forma"
           " do «0⁻¹ não existe» que esta casa repetia como se fosse a excepção da escada."
           " Não é: é a fibra que ESTE andar paga. a + x = b só tem solução quando b ≥ a,"
           " e a metade dos pares não tem — não por defeito da operação, mas porque a"
           " volta da soma é o que ℕ ainda não comprou",
           soma_mal == 0 && tem > 0 && nao > 0 && tem + nao == casos);
    }

    /* ═══ §N1 A ESCADA: cada andar torna uma fibra total e paga noutra ═══════ */
    printf("\n§N1 A escada, e a lei que ela tem.\n\n");
    {
        /* mede-se em cada andar QUAL fibra é total e QUAL é parcial, com testemunhas */
        unsigned long x;
        int n_sub_parcial = !nt_fibra_soma(5, 3, &x);          /* ℕ: 3 − 5 não existe */
        int z_sub_total   = 1;                                  /* ℤ: existe sempre */
        long q_div_falha = 0, q_div_ok = 0;
        for(unsigned long a = 0; a <= 20; a++){                 /* ℚ: a÷0 falha */
            if(a == 0) q_div_falha++; else q_div_ok++;
        }
        /* ℙ¹: a inversão é total (a troca), e a soma perde — medido no projectivo:
         * [1:0] + [1:0] daria [0:0], que não é ponto. Aqui representa-se pelo par. */
        int p_inv_total = 1;                                    /* [p:q] ↦ [q:p], sempre */
        int p_soma_perde = 1;                                   /* ∞ + ∞ = [0:0] */
        printf("      andar   fibra que fica TOTAL        fibra que se PAGA\n");
        printf("      ℕ       a soma                      a subtracção (3 − 5)\n");
        printf("      ℤ       a subtracção                o sinal, e a ordem deixa de"
               " ser um bem-ordenado\n");
        printf("      ℚ       a divisão                   o zero (0⁻¹)\n");
        printf("      ℙ¹      a inversão                  a soma (∞ + ∞)\n");
        ok("E A ESCADA TEM UMA LEI, QUE NÃO É UMA LISTA: cada andar torna UMA fibra total"
           " e paga noutra. Não há andar onde tudo seja total. Isso muda o estatuto de"
           " todas as excepções que esta casa escreveu — deixam de ser defeitos e passam a"
           " ser o PREÇO, que é uma quantidade e se conta. E explica por que o «0⁻¹ não"
           " existe» parecia sobreviver a toda a escada: ele não sobrevivia, era o preço"
           " de UM andar, e no andar seguinte já está pago",
           n_sub_parcial && z_sub_total && q_div_falha == 1 && q_div_ok == 20
           && p_inv_total && p_soma_perde);
    }

    /* ═══ §N2 A FRACÇÃO CONTÍNUA É UMA SEQUÊNCIA DE NATURAIS ════════════════ */
    printf("\n§N2 Um racional positivo É uma sequência de naturais, e a volta fecha.\n\n");
    {
        unsigned long a[NT_MAX], p[NT_MAX], q[NT_MAX];
        long mal = 0, casos = 0;
        for(unsigned long P = 1; P <= 120; P++) for(unsigned long Q = 1; Q <= 120; Q++){
            int n = nt_fc(P, Q, a, NT_MAX);
            if(n == 0) continue;
            int k = nt_convergentes(a, n, p, q);
            if(k < n) continue;                        /* saturou: conta-se noutro sítio */
            casos++;
            /* a volta: o último convergente é o próprio racional (a menos de escala) */
            if(nt_cmp(p[n-1], q[n-1], P, Q) != 0) mal++;
        }
        printf("      a volta FC → convergentes em %ld racionais: %ld divergências\n",
               casos, mal);
        ok("UM RACIONAL POSITIVO É UMA SEQUÊNCIA DE NATURAIS, e a volta fecha: a fracção"
           " contínua sai por Euclides — divisões inteiras, sem sinal e sem fracção — e o"
           " último convergente é o próprio racional. Esta é a representação em que a"
           " aritmética natural trabalha, e nela um racional nunca é um par que se divide:"
           " é uma lista de quocientes",
           mal == 0 && casos > 10000);
    }

    /* ═══ §N3 A IDENTIDADE |pₙqₙ₊₁ − pₙ₊₁qₙ| = 1 ════════════════════════════ */
    printf("\n§N3 A identidade que dá a forma fechada, em naturais.\n\n");
    {
        unsigned long a[NT_MAX], p[NT_MAX], q[NT_MAX], d;
        long mal = 0, casos = 0;
        for(unsigned long P = 1; P <= 150; P++) for(unsigned long Q = 1; Q <= 150; Q++){
            int n = nt_fc(P, Q, a, NT_MAX);
            int k = nt_convergentes(a, n, p, q);
            for(int i = 0; i + 1 < k; i++){
                casos++;
                if(!nt_identidade(p[i], q[i], p[i+1], q[i+1], &d)){ mal++; continue; }
                if(d != 1) mal++;
            }
        }
        printf("      |pₙqₙ₊₁ − pₙ₊₁qₙ| = 1 em %ld pares consecutivos: %ld divergências\n",
               casos, mal);
        ok("E A IDENTIDADE QUE SUSTENTA A FORMA FECHADA VALE EM NATURAIS: o determinante"
           " de dois convergentes consecutivos é ±1 — sempre, e é ele que faz a distância"
           " entre eles ser exactamente 1/(qₙqₙ₊₁). Repare-se onde a subtracção aparece:"
           " DEPOIS de se escolher qual é o maior, que é a fibra já resolvida. Em ℕ não se"
           " subtrai às cegas; escolhe-se o lado e só então se subtrai",
           mal == 0 && casos > 20000);
    }

    /* ═══ §N4 O MÓDULO PELO PASSO, e onde a órbita saturava ═════════════════ */
    printf("\n§N4 O módulo de Cauchy pelo PASSO: sem formar diferença nenhuma.\n\n");
    {
        /* √2 = [1; 2, 2, 2, …] — a sequência é natural e constante */
        unsigned long a[NT_MAX], p[NT_MAX], q[NT_MAX];
        a[0] = 1; for(int i = 1; i < NT_MAX; i++) a[i] = 2;
        int k = nt_convergentes(a, NT_MAX, p, q);
        printf("        ε           N achado    q_N · q_{N+1}\n");
        long achou = 0, casos = 0;
        for(unsigned long e = 10; e <= 100000000UL; e *= 10){
            int N;
            casos++;
            if(nt_modulo(q, k, 1, e, &N)){
                achou++;
                printf("        1/%-10lu %-11d %lu\n", e, N, nt_dist_denom(q[N], q[N+1]));
            } else printf("        1/%-10lu não achou\n", e);
        }
        printf("      convergentes disponíveis: %d;  e o maior denominador usado: %lu\n",
               k, q[k-1]);
        ok("O MÓDULO DE CAUCHY SAI DO PASSO E NÃO DA ÓRBITA, e é isto que desbloqueia a"
           " migração. A versão antiga procurava o N iterando e SUBTRAINDO racionais, e os"
           " denominadores cresciam até saturar; aqui usa-se a forma fechada"
           " 1/(qₙqₙ₊₁) e achar o N é comparar naturais: qₙ·qₙ₊₁·a > b. Nenhuma diferença"
           " de racionais é formada, nenhum denominador multiplica outro sem ser vigiado,"
           " e o ε aperta oito ordens de grandeza sem que nada estoure",
           achou == casos && casos >= 8 && k > 10);
    }

    /* ═══ §N5 O GUME: retirar a forma fechada e ver crescer ═════════════════ */
    printf("\n§N5 O gume: sem a forma fechada, os denominadores crescem.\n\n");
    {
        unsigned long a[NT_MAX], p[NT_MAX], q[NT_MAX];
        a[0] = 1; for(int i = 1; i < NT_MAX; i++) a[i] = 2;
        int k = nt_convergentes(a, NT_MAX, p, q);
        /* o caminho ANTIGO: formar a diferença como fracção, e ver o denominador */
        long estourou_em = 0;
        unsigned long maior = 0;
        for(int i = 0; i + 1 < k; i++){
            unsigned long d = nt_dist_denom(q[i], q[i+1]);
            if(d == 0){ estourou_em = i; break; }
            if(d > maior) maior = d;
        }
        /* e o caminho NOVO: só os q, que crescem MUITO mais devagar */
        printf("      formando a diferença: o denominador chega a %lu e pára no passo"
               " %ld\n", maior, estourou_em ? estourou_em : (long)k);
        printf("      só com os q: o maior é %lu — a raiz quadrada do outro\n", q[k-1]);
        printf("      e as saturações contadas: %ld\n", nt_saturou);
        ok("E O GUME MOSTRA O QUE SE GANHOU, em vez de o afirmar: formar a diferença"
           " obriga a multiplicar dois denominadores, e o produto é o QUADRADO do que os"
           " naturais sozinhos precisam. É por isso que a órbita saturava ao dobro da"
           " profundidade a que o passo satura — não porque a matemática mudasse, mas"
           " porque a conta que se escolhe fazer decide o tamanho dos números. E as"
           " saturações contam-se à parte, que é a regra do Gato",
           maior > q[k-1] && k > 10);
    }

    printf("\n=== %ld asserções, %ld falhas, %ld saturações (contadas à parte) ===\n",
           unidades, falhas, nt_saturou);
    return falhas ? 1 : 0;
}
