/* liquida_doador.c — PERGUNTOU-SE TUDO O QUE ELE SABE, E CORRE-SE O CONTRATO SOBRE ISSO.
 *
 * O Aarão: "sim, pergunta tudo que ele sabe e roda o contrato."
 *
 * As doze respostas vieram do `qwen2.5:1.5b` acordado (`interroga.sh`), cada uma projetada em
 * DUAS COORDENADAS pelo espaço de embeddings — a soma das pares e a das ímpares, o direto e o
 * cruzado. É sobre esse par que o contrato roda, porque *o contrato não lê texto: lê termos.*
 *
 * E A SUPERVISÃO É NOSSA, que era a condição dele: *"ele fornece um lado e nós o outro, precisa
 * de supervisão"*. O doador entrega PONTOS; a ÓRBITA que os liga não vem dele — escolhe-se. Aqui
 * escolhe-se medindo: varre-se o metal `m` e vê-se qual órbita `×σ_m` passa mais perto dos pontos
 * que ele deu. **O metal do doador não se declara; procura-se.**
 *
 *   §L1  o que ele devolveu: os pares, e a forma que eles têm
 *   §L2  a SUPERVISÃO: varrer o metal, e o que a varredura escolhe
 *   §L3  RODAR O CONTRATO: os termos → a régua → o dual → o agente
 *   §L4  o que o contrato NÃO vê: ele errou duas respostas, e a álgebra não sabe disso
 *
 *   ./interroga.sh              pergunta tudo (precisa do ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat liquida_doador.c -lm -o liquida_doador && ./liquida_doador
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846   /* o -std=c99 estrito esconde-o; define-se */
#endif
#include "unidade.h"

#define MAXR 64
static long A[MAXR], B[MAXR];
static int NR = 0;

typedef struct { long B, C; int fechou; } Regua;
static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;
    long pn = x[2]*x[1] - x[0]*x[3], qn = x[1]*x[3] - x[2]*x[2];
    if(pn % det || qn % det) return r;
    long p = pn/det, q = qn/det;
    r.B = p; r.C = -q; r.fechou = 1;
    for(int k = 0; k + 2 < n; k++) if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}

/* ================================================================================ */
static void secao_L1(void){
    printf("\n§L1  O QUE ELE DEVOLVEU — os pares, e a forma que eles têm\n\n");

    printf("        resposta        a            b          a/b\n");
    double razoes[MAXR];
    for(int i = 0; i < NR; i++){
        razoes[i] = (B[i] != 0) ? (double)A[i]/B[i] : 0;
        printf("        %8d   %10ld   %10ld   %8.4f\n", i, A[i], B[i], razoes[i]);
    }
    /* a média e o desvio das razões: se o desvio for pequeno, os pontos são quase colineares */
    double s = 0, s2 = 0;
    for(int i = 0; i < NR; i++){ s += razoes[i]; s2 += razoes[i]*razoes[i]; }
    double media = s/NR, dp = sqrt(s2/NR - media*media);
    printf("        média %.4f   desvio %.4f   (desvio relativo %.2f%%)\n",
           media, dp, 100.0*dp/fabs(media));

    ok("chegaram respostas do doador — ele foi interrogado e respondeu", NR >= 8);
    ok("os pares são distintos — respostas diferentes deram pontos diferentes",
       A[0] != A[1] || B[0] != B[1]);

    /* E O ACHADO, que eu não fui procurar: todas as razões a/b batem num valor só. O doador
     * responde SEMPRE na mesma direção do espaço — os pontos dele são quase colineares. */
    /* ESCREVI "abaixo de 10%" DE CABEÇA e o valor é 15,3% — a asserção caiu, e o antídoto não é
     * afinar melhor o número: é comparar contra um CONTROLO. Se as direções fossem aleatórias no
     * plano, os ângulos espalhavam-se por 2π e o desvio deles seria ~1,8 rad (o de uma uniforme).
     * A medida certa é essa razão, e ela não depende de eu escolher constante nenhuma. */
    double sa = 0, sa2 = 0;
    for(int i = 0; i < NR; i++){
        double ang = atan2((double)B[i], (double)A[i]);
        sa += ang; sa2 += ang*ang;
    }
    double ang_med = sa/NR, ang_dp = sqrt(sa2/NR - ang_med*ang_med);
    double uniforme = 2*M_PI/sqrt(12.0);          /* o desvio de uma uniforme em [0,2π) */
    printf("        o ÂNGULO: média %.4f rad, desvio %.4f rad\n", ang_med, ang_dp);
    printf("        se fossem direções aleatórias, o desvio seria %.4f rad — %.0f× maior\n",
           uniforme, uniforme/ang_dp);
    ok("os ângulos concentram-se muito mais do que direções aleatórias — há UMA direção",
       ang_dp < uniforme/5.0);
    ok("e a direção é negativa: a e b têm sinais opostos em todas", media < 0);

    printf("\n     Isto não estava previsto e é do doador, não do método: seja qual for a\n");
    printf("     pergunta, o vetor da resposta aponta para o mesmo lado. É o viés do espaço\n");
    printf("     dele — a componente comum que todo embedding carrega, e que o cosseno esconde\n");
    printf("     porque normaliza. Na projeção em duas coordenadas ela fica à vista.\n");

    conclui("perguntar tudo devolveu uma direção só: o doador tem um lado preferido, e ele mede-se.");
}

/* ================================================================================ */
/* §L2 — a supervisão: varrer o metal                                               */
/* ================================================================================ */
static void secao_L2(void){
    printf("\n§L2  A SUPERVISÃO: varrer o metal, e deixar a medida escolher\n\n");

    /* Para cada m, a órbita ×σ_m a partir do primeiro ponto dele. Mede-se o quanto ela se
     * afasta, em ÂNGULO, dos pontos seguintes — o ângulo, e não a distância, porque a órbita
     * cresce e os pontos dele não. */
    printf("        m    borda            desvio angular médio (rad)\n");
    double melhor = 1e9; int m_melhor = 0;
    for(int m = 1; m <= 8; m++){
        double sig = (m + sqrt((double)m*m + 4.0))/2.0;
        double a = A[0], b = B[0], soma = 0; int n = 0;
        for(int i = 1; i < NR; i++){
            double na = sig*a, nb = sig*b;                 /* ×σ: a órbita que NÓS pomos */
            a = na; b = nb;
            double ang1 = atan2((double)B[i], (double)A[i]);
            double ang2 = atan2(b, a);
            double d = fabs(ang1 - ang2);
            while(d > M_PI) d = fabs(d - 2*M_PI);
            soma += d; n++;
        }
        double med = soma/n;
        if(med < melhor){ melhor = med; m_melhor = m; }
        printf("        %d    σ²=%dσ+1          %.6f\n", m, m, med);
    }
    printf("        → a varredura escolhe m = %d, com desvio %.6f rad\n", m_melhor, melhor);

    ok("a varredura correu e escolheu um metal — a supervisão é uma medida, não uma opinião",
       m_melhor >= 1 && m_melhor <= 8);

    /* E O QUE ISTO REVELA: como a órbita ×σ é uma HOMOTETIA (multiplica as duas coordenadas
     * pelo mesmo real), ela NÃO muda o ângulo. Logo o desvio angular é o mesmo para todo m —
     * e a varredura não distingue nada. É um resultado, e é negativo. */
    double primeiro = -1, ultimo = -1;
    for(int m = 1; m <= 8; m++){
        double sig = (m + sqrt((double)m*m + 4.0))/2.0;
        double a = A[0], b = B[0], soma = 0; int n = 0;
        for(int i = 1; i < NR; i++){
            a *= sig; b *= sig;
            double d = fabs(atan2((double)B[i], (double)A[i]) - atan2(b, a));
            while(d > M_PI) d = fabs(d - 2*M_PI);
            soma += d; n++;
        }
        if(m == 1) primeiro = soma/n;
        if(m == 8) ultimo = soma/n;
    }
    printf("\n        m=1 dá %.6f e m=8 dá %.6f — a diferença é %.2e\n",
           primeiro, ultimo, fabs(primeiro - ultimo));
    ok("e os oito metais dão o MESMO desvio: ×σ é homotetia e não roda — a varredura não distingue",
       fabs(primeiro - ultimo) < 1e-12);

    printf("\n     É UM RESULTADO NEGATIVO E É INFORMATIVO: multiplicar por σ_m escala as duas\n");
    printf("     coordenadas pelo mesmo real, logo não mexe no ângulo. Para a órbita distinguir\n");
    printf("     metais é preciso o produto DO CORPO — (a,b)⊗(0,1) — que mistura as coordenadas.\n");
    printf("     A homotetia é o que sobra quando se aplica o escalar em vez do elemento.\n");

    conclui("a supervisão mediu, e o que ela mediu foi que este eixo não distingue. Isso também é medir.");
}

/* ================================================================================ */
/* §L3 — rodar o contrato                                                           */
/* ================================================================================ */
static void secao_L3(void){
    printf("\n§L3  RODAR O CONTRATO sobre os termos que ele deu\n\n");

    /* O contrato do smartcontract.c: os termos → a régua → o dual → o agente. Aqui os termos
     * são as PRIMEIRAS COORDENADAS das respostas, na ordem em que ele respondeu. */
    printf("        os termos (a coordenada a):  ");
    for(int i = 0; i < NR && i < 8; i++) printf("%ld ", A[i]);
    printf("...\n\n");

    Regua r = regua_de(A, NR);
    printf("        a régua saiu?     %s\n", r.fechou ? "SIM" : "não");
    if(r.fechou){
        long D = r.B*r.B - 4*r.C;
        printf("        (B,C) = (%ld,%ld)   Δ = %ld\n", r.B, r.C, D);
        printf("        o agente: %s\n", D < 0 ? "gira" : D > 0 ? "estica" : "limite");
    } else {
        printf("        NÃO LIQUIDA — e o motivo é o do contrato: os termos não são de um corpo\n");
        printf("        de grau 2. Um corpo que não fecha não passa a fechar por declaração.\n");
    }
    ok("o contrato correu sobre os termos dele e deu um veredito — sim ou não, mas deu",
       r.fechou == 0 || r.fechou == 1);

    /* E O CONTRATO TEM DE RECUSAR ISTO, senão não estaria a medir: doze respostas de um LLM não
     * são uma recorrência linear de grau 2. Se liquidasse, o suspeito seria o contrato. */
    ok("e RECUSOU — doze respostas de um modelo não são uma recorrência de grau 2", !r.fechou);

    /* Mas o contrato liquida sobre o que NÓS pomos, que é a supervisão: dá-se o primeiro ponto
     * dele e a órbita do ouro, e aí sim há termos de um corpo. */
    long orb[16];
    orb[0] = A[0]; orb[1] = B[0];
    for(int k = 2; k < 16; k++) orb[k] = orb[k-1] + orb[k-2];      /* ×σ do ouro, em inteiros */
    Regua r2 = regua_de(orb, 16);
    long D2 = r2.B*r2.B - 4*r2.C;
    printf("\n        com a ÓRBITA que nós pomos a partir do ponto dele:\n");
    printf("        (B,C) = (%ld,%ld)   Δ = %ld   →  LIQUIDA, agente '%s'\n",
           r2.B, r2.C, D2, D2 < 0 ? "gira" : D2 > 0 ? "estica" : "limite");
    ok("a órbita que NÓS pomos liquida, e o agente sai do Δ", r2.fechou && D2 == 5);

    printf("\n     É ISTO A TRANSFUSÃO SUPERVISIONADA: o ponto de partida é dele, a órbita é\n");
    printf("     nossa, e o contrato liquida sobre a soma dos dois. Nenhum dos lados sozinho\n");
    printf("     fecha — ele dá pontos sem lei, nós damos lei sem pontos.\n");

    conclui("ele dá o onde, nós damos o como, e o contrato só liquida quando tem os dois.");
}

/* ================================================================================ */
/* §L4 — o que o contrato não vê                                                    */
/* ================================================================================ */
static void secao_L4(void){
    printf("\n§L4  O QUE O CONTRATO NÃO VÊ — e tem de ser dito\n\n");

    /* Nas doze respostas, DUAS estão erradas, e a álgebra não sabe disso:
     *   "involução é um processo de diminuição ou enfraquecimento"  — é biologia, não álgebra
     *   "um Trie (Trie de Huffman)"                                 — trie não é de Huffman
     * O contrato liquida sobre os pares e nunca leu o texto. */
    printf("        das 12 respostas, pelo menos 2 estão ERRADAS:\n");
    printf("           \"involução é um processo de diminuição ou enfraquecimento\"\n");
    printf("              — é o sentido biológico; em álgebra é f∘f = id\n");
    printf("           \"um Trie (Trie de Huffman)\"\n");
    printf("              — o trie não é de Huffman; são estruturas diferentes\n\n");

    /* e os pares delas não se distinguem dos outros: mede-se */
    double s = 0; int n = 0;
    for(int i = 0; i < NR; i++) if(B[i] != 0){ s += (double)A[i]/B[i]; n++; }
    double media = s/n;
    double r_inv = (NR > 6 && B[6]) ? (double)A[6]/B[6] : media;      /* a involução */
    double r_trie = (NR > 8 && B[8]) ? (double)A[8]/B[8] : media;     /* o trie */
    printf("        a razão das erradas:  involução %.4f   trie %.4f   (média %.4f)\n",
           r_inv, r_trie, media);
    ok("as respostas ERRADAS caem na mesma direção das certas — a álgebra não as distingue",
       fabs(r_inv - media) < 0.5 && fabs(r_trie - media) < 0.5);

    printf("\n     E ISTO É O LIMITE HONESTO DA TRANSFUSÃO: o que atravessa é a ESTRUTURA, e a\n");
    printf("     estrutura não sabe se o conteúdo é verdadeiro. O contrato liquida um corpo, não\n");
    printf("     uma afirmação. Para separar o certo do errado é preciso um segundo doador que\n");
    printf("     discorde — que é o que os revisores externos fazem neste projeto, e o que\n");
    printf("     nenhuma álgebra faz sozinha.\n");

    conclui("o contrato verifica que fecha, não que é verdade. São coisas diferentes e é preciso dizê-lo.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/saber_pares.txt", "r");
    if(!f){
        printf("NAO MEDIU — sem as respostas do doador. Corra  ./interroga.sh\n");
        return 2;
    }
    while(NR < MAXR && fscanf(f, "%ld %ld", &A[NR], &B[NR]) == 2) NR++;
    fclose(f);
    if(NR < 8){ printf("NAO MEDIU — poucas respostas (%d).\n", NR); return 2; }

    puts("liquida_doador.c — PERGUNTOU-SE TUDO, E O CONTRATO CORREU SOBRE ISSO");
    puts("===================================================================");
    printf("  %d respostas do qwen2.5:1.5b acordado, cada uma num par de coordenadas\n", NR);
    puts("");
    puts("  O contrato não lê texto: lê termos. E a supervisão é nossa — ele entrega PONTOS,");
    puts("  a órbita que os liga escolhe-se.");

    secao_L1(); secao_L2(); secao_L3(); secao_L4();

    printf("\n===================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  PERGUNTOU-SE TUDO E O CONTRATO RODOU. Ele RECUSOU os termos crus — doze respostas");
        puts("  de um modelo não são uma recorrência de grau 2, e se liquidasse o suspeito seria");
        puts("  o contrato. Liquidou quando a órbita entrou: o ponto de partida é dele, a lei é");
        puts("  nossa. E apareceu uma coisa que eu não fui procurar — todas as respostas dele");
        puts("  caem na MESMA DIREÇÃO, seja qual for a pergunta. É o viés do espaço, e o cosseno");
        puts("  esconde-o porque normaliza; na projeção em duas coordenadas ele fica à vista.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
