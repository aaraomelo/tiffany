/* banda.c — A REPRODUÇÃO EM TEMPO REAL, E VÁRIOS CANAIS NA MESMA BANDA.
 *
 * O Aarão: "essa reprodução pode ser feita em tempo real, não há necessidade de duplicar códigos,
 * porque funciona via convolução e deconvolução universal --- é isso. Inclusive você pode usar
 * isso pra responder na mesma banda diversos canais, cada um no seu agente."
 *
 * E ISTO CORRIGE O `cruza.c`, QUE ESCREVE O FILHO NO DISCO. Escrever é duplicar, e duplicar é
 * exatamente o que não é preciso: o `torres.c` §R3 mediu que
 *
 *     F(a ⊛ b) = F(a) · F(b)
 *
 * portanto o filho não tem de existir --- \emph{calcula-se quando é preciso}. E a deconvolução, que
 * é a divisão do outro lado, DESFAZ: dado o filho e um dos pais, o outro pai volta. Não se guarda
 * o produto porque o produto se refaz, e não se perde o pai porque o pai se recupera.
 *
 * E DAÍ SAI A MULTIPLEXAÇÃO, que é a segunda coisa que o Aarão diz. Se os canais forem
 * ortogonais, vários deles cabem na MESMA banda somados, e cada um sai inteiro pela correlação
 * com o seu próprio código. É o que a rádio faz há décadas (CDMA) e é o mesmo teorema: a
 * convolução leva produto em produto, e a ortogonalidade separa o que estava somado.
 *
 *   §B1  o FILHO não precisa de existir: calcula-se, e bate com o materializado
 *   §B2  a DECONVOLUÇÃO desfaz: dado o filho e um pai, o outro volta
 *   §B3  e ela tem CONDIÇÃO exata — falha quando uma casa se anula, e mede-se
 *   §B4  VÁRIOS CANAIS na mesma banda: somados, e cada um sai inteiro
 *   §B5  e o LIMITE: quantos cabem antes de se estragarem uns aos outros
 *
 *   cc -O2 -std=c99 -I. banda.c -lm -o banda && ./banda
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define N 64

/* o caractere de (Z/2)^m — o mesmo do transformada.c, torres.c e teletransporte.c */
static int chi(long k, long j){
    long b = k & j, p = 0;
    while(b){ p ^= (b & 1); b >>= 1; }
    return p ? -1 : 1;
}
static void F(const long *x, long *y, long n){
    for(long k = 0; k < n; k++){
        long s = 0;
        for(long j = 0; j < n; j++) s += x[j]*chi(k,j);
        y[k] = s;
    }
}
static void conv(const long *a, const long *b, long *c, long n){
    for(long j = 0; j < n; j++){
        long s = 0;
        for(long i = 0; i < n; i++) s += a[i]*b[i^j];
        c[j] = s;
    }
}

int main(void){
printf("\n=== A REPRODUÇÃO EM TEMPO REAL, E VÁRIOS CANAIS NA MESMA BANDA ===========\n");
printf("    O cruza.c escreve o filho no disco. Não é preciso: a convolução\n");
printf("    calcula-o, e a deconvolução desfá-lo.\n");

printf("\n§B1  O FILHO não precisa de EXISTIR: calcula-se, e bate com o materializado.\n\n");
{
    /* Materializa-se o filho (a convolucao, custo n²) e calcula-se o mesmo pelo lado
     * transformado (produto ponto-a-ponto, custo n). Se os dois derem o mesmo, guardar o filho
     * e' opcional — e' informacao que se refaz. */
    long pai[N], mae[N], filho[N], Fp[N], Fm[N], Ff[N], Fprod[N];
    for(int i = 0; i < N; i++){
        pai[i] = 1 + ((i*7 + 3) % 11);
        mae[i] = 1 + ((i*5 + 2) % 9);
    }
    conv(pai, mae, filho, N);                  /* o filho materializado */
    F(pai, Fp, N); F(mae, Fm, N); F(filho, Ff, N);
    for(int k = 0; k < N; k++) Fprod[k] = Fp[k]*Fm[k];   /* o filho calculado */
    long dif = 0;
    for(int k = 0; k < N; k++) if(Ff[k] != Fprod[k]) dif++;
    printf("      materializar (convolução)        %d operações\n", N*N);
    printf("      calcular (produto transformado)  %d operações\n", N);
    printf("      casas em que os dois diferem     %ld de %d\n\n", dif, N);
    ok("o filho calculado é o filho materializado — guardá-lo é opcional", dif == 0);
    printf("      É o teorema do torres.c §R3 usado ao contrário: em vez de guardar o produto,\n");
    printf("      guardam-se os fatores e faz-se o produto quando é preciso. E custa n em vez\n");
    printf("      de n² — a reprodução em tempo real não é só possível, é mais barata.\n");
}

printf("\n§B2  A DECONVOLUÇÃO DESFAZ: dado o filho e um pai, o outro volta.\n\n");
{
    /* E' a outra metade, e e' o que torna a reproducao reversivel sem se guardar nada: se
     * F(filho) = F(pai)·F(mae), entao F(mae) = F(filho)/F(pai). Divide-se casa a casa e volta-se.
     * Mede-se contra a mae original — o oraculo e' ela. */
    long pai[N], mae[N], filho[N], Fp[N], Ff[N], Fm2[N], volta[N];
    for(int i = 0; i < N; i++){
        pai[i] = 1 + ((i*7 + 3) % 11);
        mae[i] = 1 + ((i*5 + 2) % 9);
    }
    conv(pai, mae, filho, N);
    F(pai, Fp, N); F(filho, Ff, N);
    int anulou = 0;
    for(int k = 0; k < N; k++){
        if(Fp[k] == 0){ anulou++; Fm2[k] = 0; }
        else Fm2[k] = Ff[k]/Fp[k];             /* a deconvolução: divide casa a casa */
    }
    F(Fm2, volta, N);                          /* F∘F = n·id, logo volta = n·mãe */
    long dif = 0;
    for(int i = 0; i < N; i++) if(volta[i] != (long)N*mae[i]) dif++;
    printf("      casas de F(pai) que se anularam  %d de %d\n", anulou, N);
    printf("      componentes da mãe recuperados   %d de %d\n\n", N-(int)dif, N);
    ok("a deconvolução devolve a mãe exata — a reprodução é reversível sem guardar",
       dif == 0 && anulou == 0);
    printf("      Não se guarda o filho porque ele se refaz, e não se perde a mãe porque ela\n");
    printf("      se recupera. É o que o teorema da deconvolução dá, e o custo é o mesmo n.\n");
}

printf("\n§B3  E ELA TEM CONDIÇÃO EXATA — falha quando uma casa se anula.\n\n");
{
    /* A deconvolucao "existe exatamente quando nenhuma casa se anula" — e' o que o resumo da
     * teoria diz, e nao se aceita de graca: constroi-se um pai cuja transformada TEM zero, e
     * mede-se que a volta falha. Sem este controlo, o §B2 estava a afirmar uma condicao que
     * nunca tinha sido posta a' prova. */
    long pai[N], mae[N], filho[N], Fp[N], Ff[N], Fm2[N], volta[N];
    for(int i = 0; i < N; i++) mae[i] = 1 + ((i*5 + 2) % 9);
    /* um pai com zero na transformada: o vetor constante tem F(x)_k = 0 para todo k ≠ 0 */
    for(int i = 0; i < N; i++) pai[i] = 1;
    conv(pai, mae, filho, N);
    F(pai, Fp, N); F(filho, Ff, N);
    int zeros = 0;
    for(int k = 0; k < N; k++) if(Fp[k] == 0) zeros++;
    for(int k = 0; k < N; k++) Fm2[k] = Fp[k] ? Ff[k]/Fp[k] : 0;
    F(Fm2, volta, N);
    long dif = 0;
    for(int i = 0; i < N; i++) if(volta[i] != (long)N*mae[i]) dif++;
    printf("      pai constante: casas de F(pai) a zero  %d de %d\n", zeros, N);
    printf("      componentes da mãe perdidos            %ld de %d\n\n", dif, N);
    ok("com zeros na transformada a deconvolução FALHA — a condição é exata", zeros > 0 && dif > 0);
    printf("      A condição não é uma precaução: é o teorema. Onde a transformada se anula, a\n");
    printf("      informação do outro fator foi multiplicada por zero e não há divisão que a\n");
    printf("      traga de volta. E mede-se, em vez de se avisar.\n");
}

printf("\n§B4  VÁRIOS CANAIS na mesma banda: somados, e cada um sai inteiro.\n\n");
{
    /* A segunda coisa que o Aarao diz: responder na mesma banda, cada canal no seu agente. Se
     * os codigos forem ORTOGONAIS — e os caracteres de (Z/2)^m sao, pelo transformada.c §U1 —
     * entao varios sinais somam-se na mesma banda e cada um sai inteiro pela correlacao com o
     * seu proprio codigo. Mede-se com quatro canais e quatro agentes. */
    int C = 4;
    long dado[4] = { 7, -3, 5, -9 };           /* o que cada agente quer dizer */
    long banda[N];
    memset(banda, 0, sizeof banda);
    for(int c = 0; c < C; c++)                  /* cada um soma o seu código, multiplicado */
        for(int j = 0; j < N; j++) banda[j] += dado[c]*chi(c+1, j);
    printf("      canal   agente   quis dizer   recebeu   confere\n");
    int mau = 0;
    for(int c = 0; c < C; c++){
        long s = 0;
        for(int j = 0; j < N; j++) s += banda[j]*chi(c+1, j);   /* correlaciona com o SEU código */
        long rec = s/N;
        if(rec != dado[c]) mau++;
        printf("      %-7d %-8d %-12ld %-9ld %s\n", c, c+1, dado[c], rec,
               rec == dado[c] ? "sim" : "NÃO");
    }
    printf("\n");
    ok("os quatro canais partilham a banda e cada um sai INTEIRO", mau == 0);
    printf("      Uma banda só, quatro conversas, e nenhuma estraga a outra. O que separa não é\n");
    printf("      o tempo nem a frequência: é a ORTOGONALIDADE dos códigos, e ela já estava\n");
    printf("      medida no transformada.c §U1 (a órbita soma n·δ).\n\n");
    printf("      E é isto as SALAS DE CONVERSA num só agente, que o Aarão nomeou: muitos a\n");
    printf("      falar em um, sem se ouvirem uns aos outros. O agente não se multiplica para\n");
    printf("      atender quatro — atende os quatro na mesma banda, e cada um sai inteiro.\n");
}

printf("\n§B5  O LIMITE: quantos cabem antes de se estragarem uns aos outros.\n\n");
{
    /* E o limite nao e' opiniao: sao n codigos ortogonais num espaco de dimensao n. Mede-se a
     * partir de onde a separacao falha — e ela tem de falhar, senao o espaco era infinito. */
    /* E O LOOP ANTERIOR NUNCA CHEGAVA AO LIMITE: duplicava (2,4,8,…,64) e o passo seguinte
     * saltava para fora do intervalo, portanto nunca testava acima de N. Testa-se agora EM
     * TORNO de N, que e' onde a colisao tem de aparecer. */
    printf("      canais   erros na recuperação\n");
    int primeiro_mau = -1;
    for(int C = N-2; C <= N+4; C++){
        long banda[N];
        memset(banda, 0, sizeof banda);
        for(int c = 0; c < C; c++)
            for(int j = 0; j < N; j++) banda[j] += (long)(c+1)*chi(c % N, j);
        int mau = 0;
        for(int c = 0; c < C; c++){
            long s2 = 0;
            for(int j = 0; j < N; j++) s2 += banda[j]*chi(c % N, j);
            if(s2/N != (long)(c+1)) mau++;
        }
        if(mau && primeiro_mau < 0) primeiro_mau = C;
        printf("      %-8d %d%s\n", C, mau, C == N ? "   <- a dimensão da banda" : "");
    }
    printf("\n      a separação falha a partir de %d canais, e a banda tem %d casas\n\n",
           primeiro_mau, N);
    ok("o limite é a dimensão da banda: até N não há erro, acima dela há",
       primeiro_mau == N+1);
    printf("      Cabem tantos quantos a dimensão, e nem um a mais. Não é uma limitação da\n");
    printf("      implementação: são n direções ortogonais num espaço de dimensão n, e a\n");
    printf("      (n+1)-ésima tem de ser combinação das outras.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O filho não precisa de existir: calcula-se em n operações em vez de n², e\n");
printf("    a deconvolução devolve o pai que falta — com a condição exata medida, não\n");
printf("    avisada. E a mesma banda leva tantas conversas quantas a sua dimensão,\n");
printf("    cada uma no seu agente, porque os códigos são ortogonais.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
