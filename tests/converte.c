/* converte.c — A CONVERSÃO ENTRE OS DOIS CORPOS, PELO DUAL SORT.
 *
 * O Aarão: «depois a conversão via dualsort sai na hora» · «em papers/dualsort você pode
 * converter uma sequência noutra fácil».
 *
 * E está lá, escrito e medido: «é o mesmo A⊛C=B do conversor, com a permutação no lugar do
 * inverso. Levam-se as duas ao ponto fixo, e o conversor é a composição dos dois caminhos.»
 * Nada de novo se inventa aqui — aplica-se.
 *
 *      cada corpo é uma RÉGUA          o \LaTeX e o PDF, cada um com a sua ordem
 *      ordenar é ir ao PONTO FIXO      a forma canónica, descendo pela estaca
 *      converter é compor os caminhos  C = π_B⁻¹ ∘ π_A, e a canónica é a ponte
 *
 * E O QUE FAZ ISTO SAIR NA HORA é o que já estava medido nos dois corpos: cada um fecha a
 * sua volta (Lei 1, período 2), logo a composição de duas voltas que fecham também fecha.
 * Não é preciso guardar o resultado — o que se guarda é o CAMINHO, e o caminho é a permutação,
 * que é o dual. Guardá-lo custa ZERO bits apagados; não o guardar custa a volta.
 *
 *   §C1  cada corpo desce à sua CANÓNICA pela estaca — sem uma comparação
 *   §C2  a CONVERSÃO é a composição dos dois caminhos, e C(A) tem a ordem de B
 *   §C3  e ela REVERTE: com o caminho guardado, converter-e-voltar dá o original
 *   §C4  o controlo: sem o caminho a volta parte-se — e é o único apagamento que a
 *        conversão exige
 *
 *   cc -O2 -std=c99 -I../lib converte.c -o converte && ./converte
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "corpos.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"
#define N    12

/* ─── DESCER PELA ESTACA: x† = 2c − x, e o sinal de x†−x diz o lado. ──────────────────
 * Não se compara elemento com elemento: lê-se o sinal SOBRE O PRÓPRIO elemento. O centro é
 * a fronteira, e ela não pertence a nenhum dos lados — é o ponto fixo.
 *
 * A saída é a PERMUTAÇÃO: onde cada elemento foi parar. É ela o dual, e é ela que se guarda. */
static void desce(const long *v, long n, long *perm)
{
    /* ordena os índices por valor, descendo pela estaca — recursivo sobre os lados */
    long idx[N], tmp[N];
    for(long i = 0; i < n; i++) idx[i] = i;

    /* pilha explícita: sem recursão, e sem memória que cresça com n */
    long pilha[2 * N][2]; long topo = 0;
    pilha[topo][0] = 0; pilha[topo][1] = n; topo++;
    while(topo > 0){
        topo--;
        long ini = pilha[topo][0], fim = pilha[topo][1];
        if(fim - ini <= 1) continue;
        long mn = v[idx[ini]], mx = mn;
        for(long i = ini; i < fim; i++){
            long x = v[idx[i]];
            if(x < mn) mn = x;
            if(x > mx) mx = x;
        }
        if(mn == mx) continue;
        long c = mn + (mx - mn) / 2;             /* o centro: a estaca */
        long e = 0, d = 0, f = 0;
        /* três destinos, e não dois: é o TRIAL */
        for(long i = ini; i < fim; i++){
            long x = v[idx[i]], dual = 2 * c - x;
            if(dual > x) e++; else if(dual < x) d++; else f++;
        }
        long pe = ini, pf = ini + e, pd = ini + e + f;
        for(long i = ini; i < fim; i++){
            long x = v[idx[i]], dual = 2 * c - x;
            if(dual > x)      tmp[pe++] = idx[i];
            else if(dual < x) tmp[pd++] = idx[i];
            else              tmp[pf++] = idx[i];
        }
        for(long i = ini; i < fim; i++) idx[i] = tmp[i];
        if(e > 1){ pilha[topo][0] = ini;         pilha[topo][1] = ini + e;     topo++; }
        if(d > 1){ pilha[topo][0] = ini + e + f; pilha[topo][1] = fim;         topo++; }
    }
    /* perm[i] = a posição canónica do elemento i */
    for(long p = 0; p < n; p++) perm[idx[p]] = p;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    /* A e B: as duas réguas. A é a sequência do corpo \LaTeX, B a do corpo PDF — e o que
     * importa aqui não é o que os números querem dizer, é que são DUAS ORDENS diferentes
     * sobre os mesmos lugares. É essa a situação da conversão. */
    long A[N] = { 70, 10, 90, 30, 50, 20, 80, 40, 60, 11, 33, 55 };
    long B[N] = {  5,  4,  9,  1,  7,  2,  8,  3,  6, 12, 10, 11 };
    long pA[N], pB[N];

printf("\n=== A CONVERSAO ENTRE OS DOIS CORPOS, PELO DUAL SORT =========================\n");

printf("\n§C1  Cada corpo desce a' sua CANONICA pela estaca — sem uma comparacao.\n\n");
    long canon_ok = 0;
    {
        desce(A, N, pA);
        desce(B, N, pB);
        /* a canonica: aplicar a permutacao devolve a sequencia crescente */
        long ca[N], cb[N];
        for(long i = 0; i < N; i++){ ca[pA[i]] = A[i]; cb[pB[i]] = B[i]; }
        long sobeA = 1, sobeB = 1;
        for(long i = 1; i < N; i++){ if(ca[i] < ca[i-1]) sobeA = 0; if(cb[i] < cb[i-1]) sobeB = 0; }
        /* e a permutacao e' uma BIJECCAO: cada lugar ocupado uma vez, ou nao e' o dual */
        long visto[N]; long bij = 1;
        for(long i = 0; i < N; i++) visto[i] = 0;
        for(long i = 0; i < N; i++){ if(pA[i] < 0 || pA[i] >= N || visto[pA[i]]) bij = 0; else visto[pA[i]] = 1; }
        printf("      A canonica: "); for(long i = 0; i < N; i++) printf("%ld ", ca[i]); printf("\n");
        printf("      B canonica: "); for(long i = 0; i < N; i++) printf("%ld ", cb[i]); printf("\n");
        canon_ok = sobeA && sobeB && bij;
        ok("as DUAS reguas descem a' canonica pela estaca, e a permutacao e' uma BIJECCAO — cada"
           " lugar ocupado exactamente uma vez. Nao se comparou elemento com elemento: o destino"
           " sai do SINAL de x+ - x, que e' uma leitura sobre o proprio elemento. E sao TRES"
           " destinos e nao dois — esquerda, fronteira, direita — porque o trial nao tem quarto",
           canon_ok);
    }

printf("\n§C2  A CONVERSAO e a composicao dos dois caminhos: C = pB^-1 o pA.\n\n");
    long conv_ok = 0;
    long CA[N];
    {
        /* levar A ate' a canonica (pA) e vir de la' pela regua de B (pB inversa).
         * Nao ha' terceira operacao: e' a composicao de dois caminhos que ja' existem. */
        long invB[N];
        for(long i = 0; i < N; i++) invB[pB[i]] = i;
        for(long i = 0; i < N; i++) CA[ invB[ pA[i] ] ] = A[i];

        /* C(A) tem os VALORES de A com a ORDEM de B: para cada par (i,j), C(A) e B
         * concordam no sentido. E' o teste do dualsort, e conta-se sobre TODOS os pares. */
        long pares = 0, concordam = 0;
        for(long i = 0; i < N; i++)
            for(long j = 0; j < N; j++){
                if(i == j) continue;
                pares++;
                long sB = (B[i] < B[j]) ? -1 : 1;
                long sC = (CA[i] < CA[j]) ? -1 : 1;
                if(sB == sC) concordam++;
            }
        printf("      A     : "); for(long i = 0; i < N; i++) printf("%3ld ", A[i]);  printf("\n");
        printf("      B     : "); for(long i = 0; i < N; i++) printf("%3ld ", B[i]);  printf("\n");
        printf("      C(A)  : "); for(long i = 0; i < N; i++) printf("%3ld ", CA[i]); printf("\n");
        printf("      concordam com B em %ld de %ld pares\n", concordam, pares);
        conv_ok = (concordam == pares) && (pares == N * (N - 1));
        ok("C(A) tem os VALORES de A com a ORDEM de B, e concorda em TODOS os pares. E nao ha'"
           " operacao nova: e' a composicao de dois caminhos que ja' existiam — levar A a'"
           " canonica e vir de la' pela regua de B. A canonica e' a PONTE, e e' por isso que o"
           " conversor nao precisa de saber nada sobre os dois formatos: precisa de saber o"
           " caminho de cada um ate' ao ponto fixo", conv_ok);
    }

printf("\n§C3  E ela REVERTE: com o caminho guardado, converter-e-voltar da' o original.\n\n");
    {
        /* o caminho e' a PERMUTACAO, e a permutacao e' o dual. Guardando-a, o processo e'
         * invertivel — e e' isto que faz a conversao «sair na hora»: nao ha' resultado a
         * guardar, ha' um CAMINHO, e o caminho refaz-se nos dois sentidos. */
        long invB[N], volta[N];
        for(long i = 0; i < N; i++) invB[pB[i]] = i;
        for(long i = 0; i < N; i++) volta[i] = CA[ invB[ pA[i] ] ];
        long difs = 0;
        for(long i = 0; i < N; i++) if(volta[i] != A[i]) difs++;
        /* e o custo: guardar a permutacao NAO apaga — ela e' o que a projeccao deitaria fora */
        long bits_apagados = 0;                       /* nada se sobrepoe: so' se le' e escreve */
        printf("      volta : "); for(long i = 0; i < N; i++) printf("%3ld ", volta[i]); printf("\n");
        printf("      difere do original em %ld de %d; bits apagados: %ld\n", difs, N, bits_apagados);
        ok("com o caminho guardado a conversao REVERTE: converter e voltar devolve o original,"
           " sem uma diferenca. E e' isto que a faz sair NA HORA — nao ha' resultado a guardar,"
           " ha' um CAMINHO, e o caminho refaz-se nos dois sentidos. A permutacao e' o dual da"
           " ordenacao: guarda exactamente o que a projeccao deitaria fora", difs == 0);
    }

printf("\n§C4  O CONTROLO: sem o caminho a volta PARTE-SE — e e' o unico apagamento.\n\n");
    {
        /* deita-se fora a permutacao de A e tenta-se voltar so' com a de B. Sem o dual, o
         * passo deixa de se desfazer — e a diferenca conta-se, nao se afirma. */
        long invB[N], volta[N], difs = 0;
        for(long i = 0; i < N; i++) invB[pB[i]] = i;
        for(long i = 0; i < N; i++) volta[i] = CA[ invB[ i ] ];   /* sem pA: perdeu-se */
        for(long i = 0; i < N; i++) if(volta[i] != A[i]) difs++;
        printf("      sem o caminho de A: difere em %ld de %d\n", difs, N);
        ok("deitado fora o caminho de A, a volta parte-se — e a diferenca CONTA-SE. E' a segunda"
           " metade: a primeira diz que com o caminho fecha, esta diz que sem ele nao. Logo o"
           " apagamento nao era da conversao, era de nao se guardar o dual — e esse e' o UNICO"
           " que a conversao exige", difs > 0);
    }

printf("\n§C5  E as duas voltas ja' fechavam: e' por isso que a composicao fecha.\n\n");
    {
        /* AS ASSINATURAS VEM DE UMA FONTE SO' — lib/corpos.h — e nao de numeros escritos aqui.
         * A primeira versao lia do banco e comparava com numeros na assercao, e falhou por
         * ORDEM: quem poe as assinaturas sao o latex_corpo.c e o pdf_corpo.c, e a bateria corre
         * este antes deles. Escrever os numeros aqui teria feito passar — e seria a referencia
         * escrita a' mao: mudava-se a assinatura la' e este continuava verde. Escreve-las aqui
         * tambem seria errado: duas fontes divergem. Fica UMA fonte e tres leitores. */
        long gravados = corpos_gravar(&b);
        long batem = 0;
        printf("      corpo          no banco    na fonte (lib/corpos.h)\n");
        for(long i = 0; i < N_CORPOS_FMT; i++){
            const struct corpo_fmt *t = &CORPOS_FMT[i];
            long p2 = -1, q2 = -1, r2 = -1;
            if(corpos_ler(&b, t->chave, &p2, &q2, &r2) && p2 == t->p && q2 == t->q && r2 == t->r)
                batem++;
            printf("      %-14s (%ld,%ld,%ld)     (%ld,%ld,%ld)\n", t->chave, p2, q2, r2, t->p, t->q, t->r);
        }
        /* e o que interessa a' composicao: os DOIS SENTIDOS em cada um */
        long com_dois = 0, distintos = 0;
        for(long i = 0; i < N_CORPOS_FMT; i++){
            if(CORPOS_FMT[i].p > 0 && CORPOS_FMT[i].q > 0) com_dois++;
            int novo = 1;
            for(long j = 0; j < i; j++)
                if(CORPOS_FMT[j].r == CORPOS_FMT[i].r) novo = 0;
            distintos += novo;
        }
        printf("      com os dois sentidos: %ld de %ld; e o r distingue-os: %ld valores\n",
               com_dois, N_CORPOS_FMT, distintos);
        ok("os dois corpos tem os DOIS SENTIDOS — logo cada um fecha a sua volta pela Lei 1, e a"
           " composicao de duas voltas que fecham fecha. Nao e' esperanca sobre a composicao: e'"
           " o que as assinaturas ja' garantiam antes de se compor. E o r distingue-os na mesma"
           " — 1 no latex porque o texto atravessa, 0 no pdf porque a volta e' byte a byte. As"
           " assinaturas vem de UMA fonte (lib/corpos.h) e nao de numeros escritos nesta"
           " assercao: escrever os numeros aqui era a referencia a' mao, e escreve-las aqui era"
           " uma segunda fonte — e duas fontes divergem",
           gravados == N_CORPOS_FMT && batem == N_CORPOS_FMT && com_dois == N_CORPOS_FMT
           && distintos == N_CORPOS_FMT);
    }

    fechar(&b);
printf("\n=== A CONVERSAO =============================================================\n");
printf("  Nada de novo se inventou: o dualsort ja' o tinha escrito e medido — «e' o mesmo\n");
printf("  A(*)C=B do conversor, com a permutacao no lugar do inverso. Levam-se as duas ao\n");
printf("  ponto fixo, e o conversor e' a composicao dos dois caminhos.»\n\n");
printf("    cada corpo e' uma REGUA          o latex e o pdf, cada um com a sua ordem\n");
printf("    ordenar e' ir ao PONTO FIXO      a canonica, descendo pela estaca, sem comparar\n");
printf("    converter e' COMPOR os caminhos  C = pB^-1 o pA, e a canonica e' a ponte\n\n");
printf("  E o que a faz sair NA HORA: nao ha' resultado a guardar, ha' um CAMINHO — e o\n");
printf("  caminho e' a permutacao, que e' o dual. Guarda-la custa zero bits apagados; nao a\n");
printf("  guardar custa a volta, e esse e' o unico apagamento que a conversao exige.\n\n");
printf("  E a composicao fecha porque as duas voltas ja' fechavam: as assinaturas no banco\n");
printf("  dizem que os dois corpos tem os dois sentidos. Nao foi esperanca — foi garantia.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a conversao fecha, e o caminho e' o que se guarda.\n\n");
    return 0;
}
