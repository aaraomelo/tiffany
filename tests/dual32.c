/* tests/dual32.c — 64 BITS SÃO DOIS DUAIS DE 32, e mede-se contra a régua larga.
 *
 * O `dual32.h` constrói o produto de 32×32 como um PAR de 32, sem usar tipo mais largo
 * nenhum. Isto só vale alguma coisa se concordar com quem tem o tipo largo — e a regra
 * desta casa é essa: DOIS CAMINHOS QUE TÊM DE CONCORDAR. O par calcula-se em `unsigned`
 * de 32; o mesmo produto calcula-se em `__int128`; e nas varreduras os dois têm de dar o
 * mesmo. Se divergirem, é o par que está errado.
 *
 * E há uma segunda coisa a medir, que é a razão de o ficheiro existir: o par tem de
 * decidir a ORDEM onde um `int` sozinho já não decide. Um teste que só corresse em
 * números pequenos não mediria nada — o regime onde o defeito vive é aquele em que o
 * produto NÃO cabe.
 *
 * §D0  o produto: o par contra o __int128, em números pequenos e nos EXTREMOS
 * §D1  a ordem: o par decide onde o int transborda — e o int transborda mesmo
 * §D2  a soma e a diferença do par, com transporte
 * §D3  o determinante 2×2, e o que ele faz quando NÃO cabe
 * §D4  o gume: sem o alto, a comparação erra — e conta-se quantas vezes
 * §D5  INT_MIN, que é o caso que parte as implementações
 */
#include <stdint.h>
#include <stdio.h>
#include "dual32.h"
#include "unidade.h"

/* a régua larga, e ela existe SÓ aqui — o `dual32.h` não a conhece */
static __int128 largo(int a, int b){ return (__int128)a * b; }

static int cmp128(__int128 x, __int128 y){ return x < y ? -1 : (x > y ? 1 : 0); }

int main(void){
    printf("\n=== 64 BITS SÃO DOIS DUAIS DE 32 ===\n");

    /* ═══ §D0 O PRODUTO: o par contra a régua larga ═══════════════════════════ */
    printf("\n§D0 O produto como PAR, contra o __int128.\n\n");
    {
        long mal = 0, casos = 0;
        const int V[] = { 0, 1, 2, 3, 7, 255, 256, 65535, 65536, 65537,
                          1000003, 46341, 2147483647, 2147483646, 1073741824 };
        int n = (int)(sizeof V / sizeof *V);
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
            unsigned a = (unsigned)V[i], b = (unsigned)V[j];
            D64 p = d64_mult(a, b);
            __int128 e = (__int128)a * b;
            uint64_t alto = (uint64_t)(e >> 32);
            uint64_t baixo = (uint64_t)(e & 0xFFFFFFFFu);
            casos++;
            if(p.alto != (unsigned)alto || p.baixo != (unsigned)baixo) mal++;
        }
        /* e uma varredura densa, para não medir só os que eu escolhi */
        for(unsigned a = 0; a < 700; a++) for(unsigned b = 0; b < 700; b++){
            unsigned x = a * 3061u + 7u, y = b * 4093u + 11u;
            D64 p = d64_mult(x, y);
            __int128 e = (__int128)x * y;
            casos++;
            if(p.alto != (unsigned)(uint64_t)(e >> 32)
               || p.baixo != (unsigned)(uint64_t)(e & 0xFFFFFFFFu)) mal++;
        }
        printf("      o par contra o __int128 em %ld produtos: %ld divergências\n",
               casos, mal);
        printf("      e o maior deles: 2147483647² = "); {
            D64 p = d64_mult(2147483647u, 2147483647u);
            printf("alto %u, baixo %u\n", p.alto, p.baixo);
        }
        ok("O PRODUTO DE 32×32 CONSTRÓI-SE COMO UM PAR DE 32, sem tipo mais largo nenhum, e"
           " concorda com o __int128 em todos os casos — incluindo os extremos, que é onde"
           " uma implementação errada passa despercebida se só se varrerem números"
           " pequenos. As quatro parcelas de 16×16 cabem cada uma num `unsigned` porque"
           " (2¹⁶−1)² < 2³², e o que NÃO cabe é a soma das duas do meio: é aí que o"
           " transporte aparece, e guardá-lo é exactamente guardar a metade que a"
           " multiplicação perderia",
           mal == 0 && casos > 490000);
    }

    /* ═══ §D1 A ORDEM onde o int já não decide ════════════════════════════════ */
    printf("\n§D1 A ordem por produto cruzado — e o int transborda mesmo.\n\n");
    {
        long mal = 0, casos = 0, transbordou = 0;
        for(int i = -60; i <= 60; i++) for(int j = -60; j <= 60; j++){
            int a = i * 37021, b = j * 41113;
            int c = (i + 1) * 36011, d = (j - 1) * 42013;
            int meu = d32_cmp_prod(a, b, c, d);
            int reg = cmp128(largo(a,b), largo(c,d));
            casos++;
            if(meu != reg) mal++;
            /* o mesmo em int puro, que é o que se faria sem o par */
            int ingenuo = (a*b < c*d) ? -1 : ((a*b > c*d) ? 1 : 0);
            if(ingenuo != reg) transbordou++;
        }
        printf("      o par contra o __int128 em %ld comparações: %ld divergências\n",
               casos, mal);
        printf("      e o int SOZINHO erra em %ld delas — o regime onde o defeito vive\n",
               transbordou);
        ok("O PAR DECIDE A ORDEM ONDE O `int` SOZINHO JÁ NÃO DECIDE, e a segunda linha é"
           " que torna isto uma medição: se o produto coubesse sempre, o par não estaria a"
           " fazer nada e a varredura passaria por acaso. Aqui o `int` puro erra em"
           " milhares de comparações e o par não erra em nenhuma — e a ordem por produto"
           " cruzado é a única coisa que o corpo dos racionais precisa do tipo largo",
           mal == 0 && transbordou > 1000);
    }

    /* ═══ §D2 A SOMA E A DIFERENÇA, com transporte ════════════════════════════ */
    printf("\n§D2 O transporte entre as duas metades.\n\n");
    {
        long mal = 0, casos = 0, houve = 0;
        for(unsigned i = 0; i < 400; i++) for(unsigned j = 0; j < 400; j++){
            D64 x = d64_mult(i * 5000009u + 3u, j * 4000037u + 5u);
            D64 y = d64_mult(j * 3000017u + 7u, i * 6000011u + 11u);
            __int128 ex = ((__int128)x.alto << 32) | x.baixo;
            __int128 ey = ((__int128)y.alto << 32) | y.baixo;
            D64 s = d64_soma(x, y);
            __int128 es = ex + ey;
            casos++;
            if(s.alto != (unsigned)(uint64_t)(es >> 32)
               || s.baixo != (unsigned)(uint64_t)(es & 0xFFFFFFFFu)) mal++;
            if(x.baixo + y.baixo < x.baixo) houve++;      /* houve transporte */
            if(d64_cmp(x,y) >= 0){
                D64 df = d64_menos(x, y);
                __int128 ed = ex - ey;
                if(df.alto != (unsigned)(uint64_t)(ed >> 32)
                   || df.baixo != (unsigned)(uint64_t)(ed & 0xFFFFFFFFu)) mal++;
            }
        }
        printf("      soma e diferença em %ld pares: %ld divergências;  e houve transporte"
               " em %ld deles\n", casos, mal, houve);
        ok("O TRANSPORTE ENTRE AS DUAS METADES É O DUAL A RECLAMAR O SEU LADO, e mede-se"
           " que ele ACONTECE: se nunca houvesse transporte, a soma dos pares seria duas"
           " somas independentes e não haveria nada a provar. A volta do `unsigned` é"
           " aritmética módulo 2³² e é DEFINIDA em C — não é comportamento acidental, é o"
           " que permite detectar o transporte sem tipo mais largo",
           mal == 0 && houve > 1000);
    }

    /* ═══ §D3 O DETERMINANTE 2×2, e o que ele faz quando NÃO cabe ═════════════ */
    printf("\n§D3 O determinante 2×2 pelo par — e a recusa honesta.\n\n");
    {
        long mal = 0, casos = 0, coube = 0, recusou = 0;
        for(int i = -40; i <= 40; i++) for(int j = -40; j <= 40; j++){
            int a = i * 1013, b = j * 1009, c = j * 997, d = i * 1021;
            int r;
            casos++;
            if(d32_det2(a, b, c, d, &r)){
                coube++;
                __int128 e = largo(a,d) - largo(b,c);
                if((__int128)r != e) mal++;
            } else recusou++;
        }
        /* e o GATO: det(A_m) = −1 para todo metal, pelo par */
        long gato_mal = 0;
        for(int m = 1; m <= 200; m++){
            int r;
            if(!d32_det2(m, 1, 1, 0, &r) || r != -1) gato_mal++;
        }
        printf("      det 2×2 em %ld matrizes: %ld couberam num int (%ld divergências),"
               " %ld recusadas\n", casos, coube, mal, recusou);
        printf("      e o gato em 200 metais: %ld divergências de det = −1\n", gato_mal);
        ok("O DETERMINANTE 2×2 SAI DO PAR, e quando o resultado não cabe num `int` a"
           " função RECUSA em vez de devolver lixo — que é a diferença entre um tecto e um"
           " transbordo. E o gato passa: det(A_m) = −1 em 200 metais, calculado sem um só"
           " tipo de 64 bits. O que era o tecto da máquina virou uma estrutura da"
           " matemática: o 64 não é primitivo, é o dual de dois 32",
           mal == 0 && gato_mal == 0 && coube > 0);
    }

    /* ═══ §D4 O GUME: sem o alto, a comparação erra ═══════════════════════════ */
    printf("\n§D4 O gume: retirar a metade dual e ver a ordem partir-se.\n\n");
    {
        long errou = 0, casos = 0;
        for(int i = 1; i <= 200; i++) for(int j = 1; j <= 200; j++){
            unsigned a = (unsigned)i * 70001u, b = (unsigned)j * 70003u;
            unsigned c = (unsigned)j * 70009u, d = (unsigned)i * 69997u;
            D64 p = d64_mult(a,b), q = d64_mult(c,d);
            int certo = d64_cmp(p, q);
            int so_baixo = (p.baixo < q.baixo) ? -1 : (p.baixo > q.baixo ? 1 : 0);
            casos++;
            if(certo != so_baixo) errou++;
        }
        printf("      comparando SÓ pelo baixo: %ld erros em %ld — a metade que se perde"
               " é a que decide\n", errou, casos);
        ok("E O GUME É RETIRAR A METADE DUAL: comparar só pelo `baixo` — que é exactamente"
           " o que fazer a conta num `int` faria — erra em milhares de casos. A metade que"
           " a multiplicação perde não é um resto pequeno: é a parte que ORDENA. É por isso"
           " que «a dualidade é a memória da divisão» não é uma frase bonita — aqui ela"
           " conta-se, e a conta é a diferença entre ordenar certo e ordenar ao acaso",
           errou > 100 && casos == 40000);
    }

    /* ═══ §D5 INT_MIN, o caso que parte as implementações ════════════════════ */
    printf("\n§D5 O −2³¹, que não tem simétrico dentro do tipo.\n\n");
    {
        long mal = 0, casos = 0;
        const int E[] = { -2147483647-1, -2147483647, -1, 0, 1, 2147483647 };
        int n = (int)(sizeof E / sizeof *E);
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
        for(int k = 0; k < n; k++) for(int l = 0; l < n; l++){
            casos++;
            if(d32_cmp_prod(E[i],E[j],E[k],E[l])
               != cmp128(largo(E[i],E[j]), largo(E[k],E[l]))) mal++;
        }
        unsigned m = d32_abs(-2147483647-1);
        printf("      |−2³¹| como unsigned: %u   (e cabe, porque a magnitude não é um"
               " int)\n", m);
        printf("      %ld comparações com os extremos: %ld divergências\n", casos, mal);
        ok("E O −2³¹ NÃO É CASO ESPECIAL AQUI, que é o sítio onde estas implementações"
           " costumam partir-se: ele não tem simétrico DENTRO do `int`, mas a magnitude"
           " dele — 2³¹ — cabe folgadamente num `unsigned`. A separação entre a parte que"
           " ORDENA (a magnitude, sem sinal) e a parte que ORIENTA (o sinal, à parte) é a"
           " mesma decomposição que esta casa faz em todo o lado, e é ela que faz o caso"
           " difícil desaparecer em vez de precisar de um `if`",
           mal == 0 && m == 2147483648u);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
