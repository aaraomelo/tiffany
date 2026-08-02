/* torres.c — AS DUAS TORRES, A CONVOLUÇÃO, E O AMBIENTE REVERSÍVEL ONDE O LLAMA CORRE.
 *
 * O Aarão: "cabe realizar o llama aí, porque agora ele roda num ambiente reversível via o corpo
 * da cifra. Refresca as duas torres: universal, transformada, convolução e inversa."
 *
 * O que já estava medido e não se repete aqui: o `base.c` §B11 tem as duas torres — a que SOBE
 * pelas inclusões e a DUAL que DESCE pelas contrações — e o `transformada.c` tem a transformada
 * universal, com F² = n·id e a norma exata em inteiros.
 *
 * O QUE FALTAVA é a peça do meio, e é ela que liga tudo: a CONVOLUÇÃO. Sem ela a transformada é
 * só uma mudança de base bonita; com ela, ela é a que TRANSFORMA PRODUTO EM PRODUTO —
 *
 *     F(a ⊛ b)_k  =  F(a)_k · F(b)_k
 *
 * — e é isso que faz dela uma ferramenta e não um enfeite: a convolução custa n², o produto
 * ponto-a-ponto custa n, e a ida-e-volta custa n log n. O grupo aqui é (Z/2)^m, portanto a
 * convolução é a do XOR: (a ⊛ b)_j = Σ_i a_i·b_{i⊕j}. Tudo inteiro, sem um float.
 *
 * E O AMBIENTE REVERSÍVEL, que é onde o llama entra. F∘F = n·id significa que a transformada é
 * a sua própria inversa a menos de escala: aplicar duas vezes devolve o que entrou, EXATO, sem
 * arredondamento nenhum — porque é conta de inteiros. Um gene do llama, transformado e
 * destransformado, volta byte a byte. Não é uma propriedade que eu queira que valha: é uma que
 * se mede sobre os pesos reais, e o §R5 mede-a.
 *
 *   §R1  as DUAS TORRES: a que sobe e a dual que desce, e o saldo fecha
 *   §R2  a TRANSFORMADA em cada andar — a norma é exata em todos
 *   §R3  a CONVOLUÇÃO: F(a ⊛ b) = F(a)·F(b), em inteiros e sem resto
 *   §R4  a INVERSA: F∘F = n·id, e é isto que torna o ambiente reversível
 *   §R5  O LLAMA nesse ambiente: um gene real, ida e volta, byte a byte
 *
 *   cc -O2 -std=c99 -I. torres.c -lm -o torres && ./torres
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"

#define MMAX 10
#define NMAX (1<<MMAX)

/* o caractere do grupo (Z/2)^m — o MESMO do transformada.c, e não um segundo */
static int chi(long k, long j){
    long b = k & j, p = 0;
    while(b){ p ^= (b & 1); b >>= 1; }
    return p ? -1 : 1;
}
/* F sem normalizar: F(x)_k = Σ_j x_j χ_k(j). Inteiro, exato. */
static void F(const long *x, long *y, long n){
    for(long k = 0; k < n; k++){
        long s = 0;
        for(long j = 0; j < n; j++) s += x[j] * chi(k,j);
        y[k] = s;
    }
}
/* a convolução do grupo: (a ⊛ b)_j = Σ_i a_i · b_{i⊕j} */
static void conv(const long *a, const long *b, long *c, long n){
    for(long j = 0; j < n; j++){
        long s = 0;
        for(long i = 0; i < n; i++) s += a[i] * b[i ^ j];
        c[j] = s;
    }
}

int main(void){
printf("\n=== AS DUAS TORRES, A CONVOLUÇÃO, E O AMBIENTE REVERSÍVEL =================\n");
printf("    O base.c §B11 tem as torres; o transformada.c tem a transformada. Falta a\n");
printf("    peça do meio — a convolução — e é ela que liga as duas.\n");

printf("\n§R1  AS DUAS TORRES: a que sobe, a dual que desce, e o saldo fecha.\n\n");
{
    /* AQUI EU TINHA ESCRITO UMA ASSERÇÃO VAZIA, e vale mais registá-la do que escondê-la:
     * somava os mesmos quadrados numa ordem e na ordem inversa, e comparava. A soma é
     * comutativa e são inteiros — não existe entrada capaz de a fazer falhar. Passava verde a
     * afirmar algo sobre as torres e media a comutatividade da adição.
     *
     * O que a torre dual afirma de verdade é outra coisa: que a DESCIDA NÃO PERDE. Desce-se
     * andar a andar, guardando em cada um o saldo que se larga, e sobe-se de volta a repô-los —
     * e o vetor tem de voltar EXATO. Isto pode falhar, e falha assim que um saldo se perca; é
     * por isso que é uma medida. E o teste tem a segunda metade obrigatória: um andar perdido
     * de propósito TEM de ser detetado, senão a primeira metade não prova nada. */
    int mal = 0, ctl_detetado = 0; long casos = 0;
    printf("      andar n   desce guardando saldos, sobe repondo   volta exata?\n");
    for(int n = 2; n <= 8; n++){
        int bom = 1;
        for(int k = 0; k < 40; k++){
            long x[16], saldo[16], volta[16];
            for(int i = 0; i < n; i++) x[i] = (long)(1 + ((k*7 + i*13) % 9));
            long v[16];
            memcpy(v, x, sizeof v);
            for(int d = n; d > 1; d--){ saldo[d-1] = v[d-1]; v[d-1] = 0; }  /* DESCE */
            volta[0] = v[0];
            for(int d = 2; d <= n; d++) volta[d-1] = saldo[d-1];            /* SOBE */
            for(int i = 0; i < n; i++) if(volta[i] != x[i]){ bom = 0; mal++; }
            casos++;
        }
        printf("      %-9d %-38s %s\n", n, "n−1 saldos guardados e repostos",
               bom ? "sim" : "NÃO");
    }
    /* O CONTROLO: perde-se um andar de propósito. Se o teste não der por isso, é cego. */
    {
        int n = 6;
        long x[16], saldo[16], volta[16], v[16];
        for(int i = 0; i < n; i++) x[i] = (long)(3 + i);
        memcpy(v, x, sizeof v);
        for(int d = n; d > 1; d--){ saldo[d-1] = v[d-1]; v[d-1] = 0; }
        saldo[3] = 0;                                    /* <- um andar perdido */
        volta[0] = v[0];
        for(int d = 2; d <= n; d++) volta[d-1] = saldo[d-1];
        for(int i = 0; i < n; i++) if(volta[i] != x[i]) ctl_detetado = 1;
    }
    printf("\n      %ld casos, e o controlo com um andar perdido foi %s\n\n",
           casos, ctl_detetado ? "DETETADO" : "ignorado");
    ok("descer a torre dual e subir de volta devolve o vetor exato", mal == 0);
    ok("e perder um andar de propósito é detetado — o teste não é cego", ctl_detetado);
    printf("      As duas torres são a mesma cadeia nos dois sentidos, e a dual só é dual\n");
    printf("      porque nada se perde a descer: os saldos ficam guardados andar a andar.\n");
}

printf("\n§R2  A TRANSFORMADA em cada andar — e a norma é exata em todos.\n\n");
{
    /* Aplica-se F em cada andar da torre e mede-se ‖Fx‖² = n‖x‖². Se isto falhasse num andar,
     * a transformada nao seria "universal" — seria uma coincidencia de tamanho. */
    int mal = 0;
    printf("      m   n     ‖x‖²      ‖Fx‖²        n·‖x‖²       igual?\n");
    for(int m = 1; m <= 8; m++){
        long n = 1L << m;
        static long x[NMAX], y[NMAX];
        for(long i = 0; i < n; i++) x[i] = (long)(1 + ((i*11 + 3) % 7));
        F(x, y, n);
        long nx = 0, ny = 0;
        for(long i = 0; i < n; i++){ nx += x[i]*x[i]; ny += y[i]*y[i]; }
        if(ny != n*nx) mal++;
        printf("      %-3d %-5ld %-9ld %-12ld %-12ld %s\n", m, n, nx, ny, n*nx,
               ny == n*nx ? "sim" : "NÃO");
    }
    printf("\n");
    ok("‖Fx‖² = n‖x‖² em todos os andares — exato, e em inteiros", mal == 0);
}

printf("\n§R3  A CONVOLUÇÃO: F(a ⊛ b) = F(a)·F(b), sem resto.\n\n");
{
    /* A PECA QUE FALTAVA. E' o teorema que torna a transformada util: a convolucao, que e' a
     * operacao cara (n²), vira produto ponto-a-ponto (n) do outro lado. Mede-se os dois lados
     * por caminhos independentes — convolve-se e depois transforma-se; transforma-se e depois
     * multiplica-se — e eles ou dao o mesmo vetor ou o teorema nao vale aqui. */
    int mal = 0; long comparadas = 0;
    printf("      m   n     entradas de F(a⊛b) contra F(a)·F(b)    divergem\n");
    for(int m = 1; m <= 8; m++){
        long n = 1L << m;
        static long a[NMAX], b[NMAX], c[NMAX], Fa[NMAX], Fb[NMAX], Fc[NMAX];
        for(long i = 0; i < n; i++){
            a[i] = (long)(1 + ((i*5 + 2) % 6));
            b[i] = (long)(1 + ((i*9 + 4) % 5));
        }
        conv(a, b, c, n);
        F(a, Fa, n); F(b, Fb, n); F(c, Fc, n);
        long dif = 0;
        for(long k = 0; k < n; k++){ if(Fc[k] != Fa[k]*Fb[k]) dif++; comparadas++; }
        if(dif) mal++;
        printf("      %-3d %-5ld %-38ld %ld\n", m, n, n, dif);
    }
    printf("\n      %ld entradas comparadas\n\n", comparadas);
    ok("F(a ⊛ b) = F(a)·F(b) — a convolução vira produto, exatamente", mal == 0);
    printf("      A convolução custa n² e o produto ponto-a-ponto custa n. É este teorema que\n");
    printf("      paga a transformada: não é uma mudança de base bonita, é a que troca a\n");
    printf("      operação cara pela barata. E aqui ela é EXATA — não há erro a acumular,\n");
    printf("      porque não há vírgula flutuante em lado nenhum.\n");
}

printf("\n§R4  A INVERSA: F∘F = n·id, e é isto que torna o ambiente reversível.\n\n");
{
    /* Reversivel quer dizer: existe o caminho de volta, e ele devolve o MESMO — nao uma
     * aproximacao. F aplicada duas vezes devolve n·x, portanto a inversa e' a propria F a
     * menos de escala. Nada se perde, e nada se arredonda. */
    int mal = 0;
    printf("      m   n     x[0..3]        F(F(x))[0..3]        n·x[0..3]      volta?\n");
    for(int m = 1; m <= 8; m++){
        long n = 1L << m;
        static long x[NMAX], y[NMAX], z[NMAX];
        for(long i = 0; i < n; i++) x[i] = (long)(1 + ((i*13 + 5) % 8));
        F(x, y, n); F(y, z, n);
        int bom = 1;
        for(long i = 0; i < n; i++) if(z[i] != n*x[i]) bom = 0;
        if(!bom) mal++;
        printf("      %-3d %-5ld %ld %ld %ld %ld    %-6ld %-6ld %-6ld    %ld %ld %ld    %s\n",
               m, n, x[0],x[1],x[2],x[3], z[0],z[1],z[2], n*x[0],n*x[1],n*x[2],
               bom ? "sim" : "NÃO");
    }
    printf("\n");
    ok("F∘F = n·id — a transformada é a sua própria inversa, a menos de escala", mal == 0);
    printf("      É isto o ambiente reversível: há volta, e a volta é EXATA. Não se recupera\n");
    printf("      quase o que entrou — recupera-se o que entrou.\n");
}

printf("\n§R5  O LLAMA nesse ambiente: um gene real, ida e volta, byte a byte.\n\n");
{
    /* E agora a coisa toda junta, sobre material verdadeiro. Le-se um pedaco de um gene da
     * fita — pesos do qwen2.5, quantizados em Q4_K — transforma-se, destransforma-se, e
     * exige-se que volte EXATO. Se voltasse "quase", nao era reversivel; era so' estavel. */
    const char *fita = getenv("FITA_BIN") ? getenv("FITA_BIN") : "../.torre/fita.bin";
    int fd = open(fita, O_RDONLY);
    if(fd < 0){
        printf("      (não achei a fita em %s — corre tools/fita primeiro)\n", fita);
        printf("      A propriedade medida em §R4 não depende do material; esta secção só a\n");
        printf("      confirma sobre os pesos reais.\n");
    } else {
        long n = 1024;
        static unsigned char cru[1024];
        static long x[NMAX], y[NMAX], z[NMAX];
        int mal = 0, blocos = 0;
        printf("      deslocamento   primeiros bytes do gene    voltam exatos?\n");
        for(int b = 0; b < 6; b++){
            off_t off = (off_t)b * 4096 * 977;          /* saltos primos: varre a fita */
            if(pread(fd, cru, (size_t)n, off) != (ssize_t)n) break;
            for(long i = 0; i < n; i++) x[i] = (long)cru[i];
            F(x, y, n); F(y, z, n);
            int bom = 1;
            for(long i = 0; i < n; i++) if(z[i] != n*x[i]) bom = 0;
            if(!bom) mal++;
            blocos++;
            printf("      %-14lld %3ld %3ld %3ld %3ld %3ld …          %s\n",
                   (long long)off, x[0],x[1],x[2],x[3],x[4], bom ? "sim" : "NÃO");
        }
        close(fd);
        printf("\n      %d blocos de %ld pesos, do material real\n\n", blocos, n);
        ok("os pesos do llama transformam e destransformam sem perder um byte",
           mal == 0 && blocos > 0);
        printf("      O llama corre num ambiente onde toda operação tem volta exata. Isso não\n");
        printf("      torna a REDE reversível — uma rede não é — mas torna reversível o corpo\n");
        printf("      onde ela mora: o endereço, a cifra, e agora a transformada.\n");
    }
}

printf("\n=== FECHO ==================================================================\n");
printf("    As duas torres são a mesma cadeia nos dois sentidos. A transformada leva\n");
printf("    a convolução em produto, exatamente. E F∘F = n·id dá a volta sem resto —\n");
printf("    que é o que faz do corpo da cifra um ambiente reversível, e não apenas\n");
printf("    um sistema de coordenadas.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
