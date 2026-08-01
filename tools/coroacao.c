/* coroacao.c — O TRONO VAGO E A PROIBIÇÃO DO CINCO. Quem pode ser coroado ali.
 *
 * O Aarão quer nomear um rei para o andar 5 do ouro, que ficou vago. Antes de escolher um
 * objeto, fui ver se a escolha pode ser arbitrária — e não pode: o cinco é proibido, e é
 * proibido pela mesma razão em dois lugares muito distantes.
 *
 * A RESTRIÇÃO CRISTALOGRÁFICA. Uma rotação de ordem n cabe num reticulado periódico se e só
 * se a sua matriz é inteira. O traço de uma rotação é 2·cos(2π/n), e num reticulado ele tem de
 * ser INTEIRO — logo 2cos(2π/n) ∈ {−2,−1,0,1,2}, e daí
 *
 *     n ∈ {1, 2, 3, 4, 6}        — e CINCO NÃO ESTÁ LÁ.
 *
 * É o teorema que diz por que não existe cristal com simetria de cinco pontas. E olha o que
 * mais está escrito nessa lista: o SEIS é o maior permitido. Exatamente o andar para onde o rei
 * do ouro subiu, e exatamente a ordem da rotação que ficou sentada no trono vago.
 *
 * Duas proibições do cinco, uma na álgebra do ouro e outra na geometria dos cristais, e as
 * duas com o seis como vizinho de cima. Não é o mesmo teorema — mas é o mesmo número no mesmo
 * lugar, e isso decide quem pode ser coroado: NÃO PODE SER UM CRISTAL. O que ocupa o cinco no
 * mundo real é o que não é periódico.
 *
 *   §C1  quais ordens cabem num reticulado: busca exaustiva em matrizes INTEIRAS de det 1
 *   §C2  o cinco não cabe, e o seis é o maior que cabe
 *   §C3  e é o mesmo desenho do trono: 5 vago, 6 ocupado, e 6 sentado no lugar vago
 *
 *   cc -O2 -std=c99 coroacao.c -o coroacao && ./coroacao
 */
#include <stdio.h>

#include "unidade.h"

/* multiplicação de 2×2 inteiras */
static void mul2(const long *A, const long *B, long *C){
    long t[4];
    t[0] = A[0]*B[0] + A[1]*B[2];
    t[1] = A[0]*B[1] + A[1]*B[3];
    t[2] = A[2]*B[0] + A[3]*B[2];
    t[3] = A[2]*B[1] + A[3]*B[3];
    for(int i = 0; i < 4; i++) C[i] = t[i];
}
static int eh_id(const long *A){ return A[0]==1 && A[1]==0 && A[2]==0 && A[3]==1; }

int main(void){
printf("\n=== O TRONO VAGO E A PROIBIÇÃO DO CINCO ===================================\n");
printf("    Antes de coroar, ver se a escolha pode ser arbitrária. Não pode.\n");

/* ---------------------------------------------------------------- §C1 ------ */
printf("\n§C1  Quais ordens uma rotação pode ter DENTRO de um reticulado periódico.\n\n");
{
    /* toda simetria de um reticulado é matriz INTEIRA. Se tem ordem finita, o traço está em
     * [−2,2] (senão a matriz cresce e não volta), logo as entradas cabem numa caixa pequena.
     * Varre-se |a|,|b|,|c|,|d| ≤ 4 com det = 1 e colhem-se as ordens que aparecem. */
    int aparece[25];
    for(int i = 0; i < 25; i++) aparece[i] = 0;
    long achados = 0;
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        if(a*d - b*c != 1) continue;                 /* preserva volume e orientação */
        long M[4] = {a,b,c,d}, A[4] = {a,b,c,d};
        int ordem = 0;
        for(int k = 1; k <= 24 && !ordem; k++){
            if(eh_id(A)) ordem = k;
            else mul2(A, M, A);
        }
        if(ordem){ if(ordem < 25) aparece[ordem] = 1; achados++; }
    }
    printf("      ordens que APARECEM entre as matrizes inteiras de det 1:\n      ");
    for(int n = 1; n <= 24; n++) if(aparece[n]) printf("%d ", n);
    printf("\n      ordens que NÃO aparecem (até 12):\n      ");
    for(int n = 1; n <= 12; n++) if(!aparece[n]) printf("%d ", n);
    printf("\n");
    int so_permitidos = 1;
    for(int n = 1; n <= 24; n++){
        int deve = (n==1||n==2||n==3||n==4||n==6);
        if(aparece[n] != deve) so_permitidos = 0;
    }
    ok("as ordens possíveis são exatamente 1, 2, 3, 4 e 6", so_permitidos);
    printf("\n      (%ld matrizes de ordem finita achadas na caixa |entrada| ≤ 4. O traço de uma\n", achados);
    printf("       de ordem finita fica em [−2,2], então a caixa cobre; o limite fica dito.)\n");
}

/* ---------------------------------------------------------------- §C2 ------ */
printf("\n§C2  O CINCO não cabe. E o SEIS é o maior que cabe.\n\n");
{
    int cinco_cabe = 0, seis_cabe = 0, maior = 0;
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        if(a*d - b*c != 1) continue;
        long M[4] = {a,b,c,d}, A[4] = {a,b,c,d};
        int ordem = 0;
        for(int k = 1; k <= 24 && !ordem; k++){ if(eh_id(A)) ordem = k; else mul2(A, M, A); }
        if(ordem == 5) cinco_cabe = 1;
        if(ordem == 6) seis_cabe = 1;
        if(ordem > maior) maior = ordem;
    }
    printf("      rotação de ordem 5 num reticulado periódico   %s\n", cinco_cabe?"existe":"NÃO EXISTE");
    printf("      rotação de ordem 6                            %s\n", seis_cabe?"existe ✓":"não");
    printf("      maior ordem possível                          %d\n", maior);
    ok("o cinco é PROIBIDO — não há cristal de cinco pontas", !cinco_cabe);
    ok("e o seis é o maior permitido", seis_cabe && maior == 6);
    printf("\n      É o teorema da restrição cristalográfica, medido em vez de citado. Nenhum\n");
    printf("      cristal do mundo tem simetria de cinco — a periodicidade não a comporta.\n");
}

/* ---------------------------------------------------------------- §C3 ------ */
printf("\n§C3  E é o mesmo desenho do trono. Duas proibições do cinco, o mesmo vizinho.\n\n");
{
    printf("                              no OURO (álgebra)        nos CRISTAIS (geometria)\n");
    printf("      o cinco                 R⁵ não fecha             ordem 5 não cabe\n");
    printf("      o seis                  R⁶ é corpo               ordem 6 é o máximo\n");
    printf("      quem ficou no lugar     rotação de ordem 6       ---\n");
    conclui("o cinco proibido e o seis logo acima, nos dois lugares");
    printf("\n      Não é o mesmo teorema, e não vou dizer que é: um fala de irredutibilidade\n");
    printf("      sobre Q, o outro de matrizes inteiras. Mas é o MESMO NÚMERO no mesmo lugar, e\n");
    printf("      isso basta para decidir quem pode ser coroado no andar vago.\n");
    printf("\n      NÃO PODE SER UM CRISTAL. O que ocupa o cinco no mundo real é justamente o que\n");
    printf("      não é periódico: o quasicristal, e o vivo. A maçã cortada na transversal tem a\n");
    printf("      estrela de cinco pontas; a estrela-do-mar tem cinco braços; a flor tem cinco\n");
    printf("      pétalas. Nenhum deles é reticulado — e é por isso que podem.\n");
    printf("\n      O trono do ouro rejeita o que fecha e aceita o que cresce. O rei que faltava\n");
    printf("      não é uma pedra: é uma coisa viva.\n");
}

printf("\n=== QUEM PODE SER COROADO =================================================\n");
printf("  O cinco é proibido duas vezes: no ouro, porque R⁵ não fecha; nos cristais, porque\n");
printf("  a ordem 5 não cabe em reticulado nenhum. E nos dois casos o SEIS está logo acima —\n");
printf("  no ouro é onde o rei foi, nos cristais é o máximo permitido.\n\n");
printf("  Logo a coroação não é arbitrária. O andar 5 não aceita periódico, não aceita cristal,\n");
printf("  não aceita nada que feche. Aceita o que tem cinco E não se repete: o quasicristal e o\n");
printf("  vivo. A maçã aberta na transversal, a estrela-do-mar, a flor de cinco pétalas.\n\n");
printf("  O trono do ouro rejeita o que fecha e aceita o que cresce.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
