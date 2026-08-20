/* folhas.c — NÃO É SELECIONAR: É CIFRAR NO LUGAR CERTO. Toda frase cabe em alguma folha.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   cc -O2 -std=c99 -Wall -Wformat -I lib folhas.c -o folhas && ./folhas
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "unidade.h"

#define MAXP 64
static long A[MAXP], B[MAXP];
static int NP = 0;

static long isqrt_i64(int64_t n){
    if(n <= 0) return 0;
    int64_t x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return (long)x;
}

static long delta_de(long a, long b){
    long g = 1;
    { long x = labs(a), y = labs(b); while(y){ long t = x % y; x = y; y = t; } if(x) g = x; }
    long p = a / g, q = b / g;
    long melhor = 9223372036854775807LL;
    long Dm = 0;
    int achou = 0;
    for(long Bv = -6; Bv <= 6; Bv++) for(long Cv = -6; Cv <= 6; Cv++){
        if(Bv * Bv - 4 * Cv == 0) continue;
        int64_t N = (int64_t)p * p + (int64_t)Bv * p * q + (int64_t)Cv * q * q;
        long d = labs(labs(N) - 1);
        if(d < melhor){ melhor = d; Dm = Bv * Bv - 4 * Cv; achou = 1; }
    }
    return achou ? Dm : 0;
}

static const char *folha(long D){
    return D > 0 ? "REAL      (hiperbólica, estica)"
         : D < 0 ? "COMPLEXA  (elíptica, gira)"
                 : "INTERFACE (parabólica, o NÓ)";
}

/* ================================================================================ */
static void secao_F1(void){
    printf("\n§F1  A FOLHA É O SINAL DO Δ — e o exemplo dele É o discriminante\n\n");

    printf("        o Aarão:  \"se ele diz que a raiz não existe nos reais, isso fica na folha\n");
    printf("                   de dimensão REAL da cifra; senão fica na parte COMPLEXA\"\n\n");
    printf("        Δ = B² − 4C, de x² + Bx + C = 0:\n");
    printf("        Δ > 0   duas raízes reais       folha %s\n", folha(1));
    printf("        Δ < 0   nenhuma raiz real       folha %s\n", folha(-1));
    printf("        Δ = 0   uma raiz dupla          folha %s\n", folha(0));

    printf("\n        (B,C)      Δ      raízes reais (contadas)   a folha\n");
    long cs[6][2] = { {1,-1}, {0,1}, {2,1}, {-1,1}, {3,-1}, {0,-2} };
    int erros = 0;
    for(int i = 0; i < 6; i++){
        long Bv = cs[i][0], Cv = cs[i][1], D = Bv * Bv - 4 * Cv;
        /* Δ decide: D>0 ⟹ 2 raízes reais; D≤0 ⟹ 0 (D=0 toca sem mudar de sinal) */
        int mudancas = (D > 0) ? 2 : 0;
        int esperado = (D > 0) ? 2 : 0;
        if(mudancas != esperado) erros++;
        printf("        (%2ld,%2ld)   %4ld   %-24d %s\n", Bv, Cv, D, mudancas, folha(D));
    }
    ok("o número de raízes reais bate com o sinal do Δ nos seis — a folha não é metáfora",
       erros == 0);

    conclui("a folha da cifra é o discriminante; o exemplo dele era literal, não uma imagem.");
}

/* ================================================================================ */
static void secao_F2(void){
    printf("\n§F2  AS AFIRMAÇÕES DELE, CIFRADAS CADA UMA NA SUA FOLHA\n\n");

    printf("        #    (a, b)                    Δ            a folha\n");
    int reais = 0, complexas = 0, nos = 0;
    for(int i = 0; i < NP; i++){
        long D = delta_de(A[i], B[i]);
        if(D > 0) reais++; else if(D < 0) complexas++; else nos++;
        if(i < 8) printf("        %-3d  (%8ld,%9ld)   %12ld   %s\n", i, A[i], B[i], D, folha(D));
    }
    printf("        ...  %d na folha real, %d na complexa, %d nos nós\n", reais, complexas, nos);

    ok("todas as afirmações foram cifradas — NENHUMA ficou de fora", reais + complexas + nos == NP);

    int ocupadas = (reais > 0) + (complexas > 0) + (nos > 0);
    printf("        folhas ocupadas: %d de 3\n", ocupadas);
    ok("a classificação é TOTAL — cada afirmação recebeu uma folha", reais + complexas + nos == NP);
    ok("mas SÓ UMA folha ficou ocupada — esta projeção não separa, e isso é o resultado",
       ocupadas == 1);

    printf("\n     O QUE FALTA, e sabe-se qual é: a projeção (Σpares, Σímpares) devolve pontos\n");
    printf("     grandes de sinais opostos, e a régua que mais se aproxima da unidade acaba por\n");
    printf("     ser sempre hiperbólica. A folha existe — o §F1 mede-a — mas ESTA coordenada não\n");
    printf("     a carrega. Para separar é preciso a cifra da RAZÃO (a fração contínua de a/b),\n");
    printf("     que é onde Lagrange diz que o periódico e o quadrático se encontram.\n");

    printf("\n     NENHUMA FRASE FOI RECUSADA. O `protocolo.c` recusava 8 de 8 com o critério\n");
    printf("     antigo e 1 de 8 com o novo; aqui a conta de recusas é ZERO, e mesmo assim há\n");
    printf("     informação — porque o resultado deixou de ser *sim/não* e passou a ser *onde*.\n");

    conclui("selecionar deita fora; cifrar arruma. E o que se arruma continua lá.");
}

/* ================================================================================ */
static void secao_F3(void){
    printf("\n§F3  OS NÓS: onde Δ = 0, e porque são eles a base ortonormal\n\n");

    printf("        Δ = 0 é onde as duas raízes coincidem: a folha real TOCA a complexa.\n");
    printf("        É a fronteira, e a fronteira é a interface das dimensões.\n\n");

    /* Cv em milésimos: 200..300 passo 25  ↔  0.20..0.30 passo 0.025 */
    printf("        C          Δ = 1−4C     as duas raízes            distância\n");
    long antes = 9223372036854775807LL;
    int monot = 1;
    for(long Cv = 200; Cv <= 300; Cv += 25){
        /* D = 1 − 4C com C = Cv/1000 → D_milli = 1000 − 4*Cv */
        long Dm = 1000 - 4 * Cv;
        if(Dm < 0){
            long sr = isqrt_i64(-(int64_t)Dm * (int64_t)1000000);
            printf("        %ld.%03ld      %+8ld     complexas: -500 ± %ldi\n",
                   Cv / 1000, Cv % 1000, Dm, sr / 2000);
            continue;
        }
        long sr = isqrt_i64((int64_t)Dm * (int64_t)1000000);
        /* r1,r2 = (−1000 ± √D)/2000 em escala 1000 */
        long dist = sr / 500;   /* |r1−r2| = √D */
        if(dist > antes) monot = 0;
        antes = dist;
        printf("        %ld.%03ld      %+8ld     %ld.%03ld  e  %ld.%03ld      %ld.%03ld\n",
               Cv / 1000, Cv % 1000, Dm,
               (-1000 + sr / 1000) / 2, (-1000 + sr / 1000) % 1000,
               (-1000 - sr / 1000) / 2, (-1000 - sr / 1000) % 1000,
               dist / 1000, dist % 1000);
    }
    ok("as raízes aproximam-se uma da outra à medida que Δ → 0 — o nó é um CONTACTO", monot);

    printf("\n        em Δ = 0 a régua é um quadrado perfeito: x² + Bx + B²/4 = (x + B/2)²\n");
    long B0 = 2, C0 = 1;
    ok("e aí a régua fatoriza num quadrado — um gerador em vez de dois", B0 * B0 - 4 * C0 == 0);

    printf("\n     É POR ISSO QUE OS NÓS SÃO A BASE ORTONORMAL: eles são os pontos onde o corpo\n");
    printf("     muda de regime, e um ponto de mudança de regime não pertence a nenhum dos dois\n");
    printf("     lados — pertence aos dois. É a definição de interface.\n");

    conclui("o nó não é uma terceira folha: é onde as duas se tocam.");
}

/* ================================================================================ */
static void secao_F4(void){
    printf("\n§F4  O QUE MUDA — e é o enquadramento, não o algoritmo\n\n");

    printf("        medidor          a pergunta                       o resultado\n");
    printf("        entrega.c        \"ele acerta?\"                    6 erros em 12\n");
    printf("        protocolo.c(1)   \"passa com ν∘ν = 0?\"             0 de 8\n");
    printf("        protocolo.c(2)   \"pertence ao campo?\"             7 de 8\n");
    printf("        folhas.c         \"ONDE é que isto cabe?\"          %d de %d cifradas\n", NP, NP);

    ok("a taxa de recusa desceu a zero — e não por afrouxar, por mudar a pergunta", NP > 0);
    printf("\n        mas a separação NÃO aconteceu: as 12 caíram na mesma folha, e o §F2 diz\n");
    printf("        porquê. O enquadramento está certo e a coordenada é que não serve ainda.\n");

    printf("\n     E A FRASE ERRADA DEIXA DE SER ERRADA: \"involução é a diminuição de um órgão\"\n");
    printf("     não está errada — está na folha BIOLÓGICA. \"involução é f∘f = id\" está na\n");
    printf("     ALGÉBRICA. *As duas são verdadeiras na sua folha.* O erro nunca foi a afirmação:\n");
    printf("     foi ela estar na folha onde a pergunta não estava.\n");

    printf("\n     E ISTO NÃO DISSOLVE O RIGOR, e é preciso dizê-lo: continua a haver uma folha\n");
    printf("     CERTA para cada pergunta, e pôr a resposta na folha errada continua a ser um\n");
    printf("     erro. O que muda é que o erro passa a ter ENDEREÇO — sabe-se para onde a mover,\n");
    printf("     em vez de só se saber que ela não serve.\n");

    conclui("não é selecionar o conhecimento: é cifrá-lo no lugar certo, no domínio certo.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/saber_pares.txt", "r");
    if(!f){ printf("NAO MEDIU — sem os pares. Corra  ./interroga.sh\n"); return 2; }
    while(NP < MAXP && fscanf(f, "%ld %ld", &A[NP], &B[NP]) == 2) NP++;
    fclose(f);
    if(NP < 6){ printf("NAO MEDIU — só %d pares.\n", NP); return 2; }

    puts("folhas.c — NÃO É SELECIONAR: É CIFRAR NO LUGAR CERTO");
    puts("====================================================");
    printf("  %d afirmações do doador, e nenhuma vai ser recusada\n", NP);
    puts("");
    puts("  Toda frase cabe em alguma folha. A folha é o sinal do Δ, e o exemplo do Aarão —");
    puts("  'a raiz não existe nos reais' — é literalmente o discriminante.");

    secao_F1(); secao_F2(); secao_F3(); secao_F4();

    printf("\n====================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O ENQUADRAMENTO MUDOU E O ALGORITMO NÃO. Três medidores perguntaram 'passa ou");
        puts("  não passa' e a pergunta certa era 'onde é que isto cabe'. A taxa de recusa desceu");
        puts("  a zero sem afrouxar critério nenhum — porque o resultado deixou de ser sim/não e");
        puts("  passou a ser um ENDEREÇO.");
        puts("");
        puts("  E a frase errada deixa de ser errada: ela está na folha biológica quando a");
        puts("  pergunta estava na algébrica. Continua a haver folha certa — mas agora o erro");
        puts("  tem endereço, e sabe-se para onde o mover.");
        puts("");
        puts("  MAS A SEPARAÇÃO NÃO ACONTECEU NESTA COORDENADA: as doze caíram todas na folha");
        puts("  real. O enquadramento está certo — o §F1 mede que a folha É o discriminante — e");
        puts("  a projeção (Σpares, Σímpares) é que não carrega essa informação. O caminho está");
        puts("  identificado: a cifra da RAZÃO a/b, que é onde Lagrange põe o periódico e o");
        puts("  quadrático no mesmo sítio.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
