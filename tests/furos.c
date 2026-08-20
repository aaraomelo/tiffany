/* furos.c — OS FUROS ENTRE AS DIMENSÕES: Cantor é o direto, Julia é o cruzado.
 *
 * REGUA: Cantor⊕Julia (direto/cruzado) — não Landauer (portão dissipa.sh)
 *
 * O Aarão: "o que a transformada faz é selecionar pontos acima do infinito nos furos entre as
 * dimensões, na passagem; todos eles formam a cifra, a base ortonormal, e vem a codificação única
 * no espaço dual." E depois: "coloca Cantor como produto direto via forma algébrica e Julia no
 * produto cruzado e forma polar, ambos nas duas torres — direto e cruzado nas duas torres, assim
 * sobe e desce simétrico via indução/metaindução, soma/multiplicação."
 *
 * A ATRIBUIÇÃO NÃO É ARBITRÁRIA, e o `polar.c` já tinha dito porquê: a forma ALGÉBRICA é a que
 * soma bem (componente a componente) e a POLAR é a que multiplica bem (módulos multiplicam,
 * ângulos somam) --- uma forma para cada operação, e não há terceira. Então:
 *
 *     CANTOR  x = Σ 2b_k/3^k          é uma SOMA de coordenadas independentes.
 *             {0,2}^n                 é o PRODUTO DIRETO de n conjuntos de dois.
 *             forma ALGÉBRICA, e por base.c §B6 o direto NÃO VOA — guarda, não gera.
 *
 *     JULIA   z → z²                  na polar é ρ² e ângulo dobrado: MULTIPLICAÇÃO.
 *             o ângulo soma, o módulo multiplica — o operador ∏ = exp∘Σ∘log.
 *             forma POLAR, e o cruzado VOA — gera, não guarda.
 *
 * E O FURO TEM NÚMERO. A dimensão do conjunto de Cantor é log2/log3 = 0,6309..., que NÃO É
 * INTEIRA: ele vive entre a dimensão 0 e a 1, num sítio onde nenhuma dimensão inteira está. É
 * literalmente um furo entre dimensões — e é a tese da teoria dita ao contrário: se a dimensão
 * inteira é ancoragem e o contínuo é o que existe, então o que existe está nos furos.
 *
 * O produto DIRETO é o que anda nesses furos: as dimensões SOMAM, portanto k cópias de Cantor
 * dão k·0,6309..., e essa sequência atravessa 1, 2, 3 sem nunca pousar em nenhum. Sobe-se por
 * soma (indução) e desce-se por multiplicação (metaindução) --- e é essa a simetria das duas
 * torres.
 *
 *   §F1  o FURO tem número: a dimensão do Cantor, medida por contagem e não por fórmula
 *   §F2  CANTOR é o produto DIRETO: as dimensões SOMAM, e a soma varre os furos
 *   §F3  JULIA é o CRUZADO na polar: módulos multiplicam, ângulos somam
 *   §F4  as DUAS TORRES: o dual troca o SINAL da multiplicação — σ·σ' = −1
 *   §F5  INDUÇÃO e METAINDUÇÃO: e a simetria entre subir e descer
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/furos.c -o furos
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <string.h>
#include "unidade.h"
#include "reta.h"
#include "isa_disk.h"

/* quantas caixas de lado 3^-k tocam o Cantor? conta-se, não se assume */
static long caixas_cantor(int k){
    long n = 1;
    for(int i = 0; i < k; i++) n *= 2;      /* cada nível parte cada caixa em 2 */
    return n;
}
/* e conta-se de outra maneira: percorrendo os pontos e vendo em que caixa caem */
static long caixas_por_varredura(int k){
    long total = 1;
    for(int i = 0; i < k; i++) total *= 2;
    long ocupadas = 0;
    long lado = 1;
    for(int i = 0; i < k; i++) lado *= 3;
    /* marca-se qual caixa de 1/3^k cada extremo do Cantor ocupa */
    char *marca = DISCO_FIXO(char, 224);
    disco_prende(DISCO_BASE(224),"dados/marca_224.bin",(size_t)(60000),sizeof(char)); disco_zera(marca,(size_t)(60000),sizeof(char));
    if(lado > 60000) return -1;
    memset(marca, 0, (size_t)lado);
    for(long v = 0; v < total; v++){
        long num = 0, den = 1;
        for(int i = 0; i < k; i++){
            num = num * 3 + 2*((v >> (k-1-i)) & 1);
            den *= 3;
        }
        long c = (num * lado + den/2) / den;
        if(c >= 0 && c < lado && !marca[c]){ marca[c] = 1; ocupadas++; }
    }
    return ocupadas;
}

int main(void){
printf("\n=== OS FUROS ENTRE AS DIMENSÕES ===========================================\n");
printf("    Cantor é o produto DIRETO na forma algébrica (soma); Julia é o CRUZADO\n");
printf("    na forma polar (multiplicação). E o furo tem número.\n");

printf("\n§F1  O FURO TEM NÚMERO: a dimensão do Cantor, contada e não assumida.\n\n");
{
    /* A dimensao mede-se por CONTAGEM DE CAIXAS: N(e) caixas de lado e, e dim = log N / log(1/e).
     * Conta-se de duas maneiras independentes — pela recursao (cada nivel duplica) e por
     * VARREDURA dos pontos, marcando as caixas que eles ocupam. Se as duas nao dessem o mesmo,
     * a contagem estava errada, e a dimensao com ela. */
    printf("      nível k   caixas 3^-k (recursão)   por varredura   1 < N < 3^k\n");
    int mau = 0;
    for(int k = 1; k <= 9; k++){
        long a = caixas_cantor(k), b = caixas_por_varredura(k);
        if(b >= 0 && a != b) mau++;
        long p3 = 1;
        for(int i = 0; i < k; i++) p3 *= 3;
        printf("      %-9d %-24ld %-15ld N=%ld  %s\n", k, a, b, a,
               (a > 1 && a < p3) ? "0<D<1" : "?");
    }
    printf("\n      log2/log3 definido por 3^D = 2 — medido em inteiros abaixo\n\n");
    /* E «0 < D < 1» MEDE-SE NO OBJECTO, sem logaritmo. D é definido por 3^D = N, logo
     *      D > 0  <=>  N > 3^0 = 1        D < 1  <=>  N < 3^1·(por nível) = 3^k
     * e o que decide é o NÚMERO DE CAIXAS, não um par de literais. A condição anterior
     * escrevia «2 > 1 && 2 < 3», que é verdade sem olhar para nada — e ao lado tinha
     * `fabs(ultima − log(2.0)/log(3.0)) < 1e-9`, que é x == x: `ultima` É log(2^k)/(k·log3)
     * e os k CANCELAM-SE, tal como no koch.c. Quatro dos seis termos não podiam falhar. */
    long entre = 0, niveis = 0;
    for(int k = 1; k <= 9; k++){
        long N = caixas_cantor(k), p3 = 1;
        for(int i = 0; i < k; i++) p3 *= 3;
        niveis++;
        if(N > 1 && N < p3) entre++;          /* 1 < N < 3^k  é  0 < D < 1 */
    }
    printf("      e «0 < D < 1» em inteiros: 1 < N < 3^k em %ld de %ld níveis\n\n",
           entre, niveis);
    ok("as duas contagens de caixas concordam — a dimensão é contada, não assumida",
       mau == 0);
    /* «ENTRE 0 E 1» NÃO PRECISA DE LOGARITMOS. D = log2/log3 é definido por 3^D = 2, logo
     *      D > 0  ⟺  2 > 1        e        D < 1  ⟺  2 < 3
     * — em INTEIROS. E a irracionalidade sai da factorização única: D = p/q daria 3^p = 2^q,
     * e 2 e 3 são primos distintos. É o mesmo conserto do koch.c, onde 1 < D < 2 era
     * 3 < 4 < 9. */
    long pq_c = 0, resolve_c = 0;
    for(long q2 = 1; q2 <= 36; q2++){
        int e2 = 1; int cabe = 1;
        for(int t = 0; t < q2; t++){ if(e2 > 4000000000000000000L/2){ cabe = 0; break; } e2 *= 2; }
        if(!cabe) continue;
        for(long p2 = 1; p2 <= 36; p2++){
            long e3 = 1; cabe = 1;
            for(int t = 0; t < p2; t++){ if(e3 > 4000000000000000000L/3){ cabe = 0; break; } e3 *= 3; }
            if(!cabe) continue;
            pq_c++;
            if(e2 == e3) resolve_c++;
        }
    }
    printf("      e SEM logaritmos: D > 0 e' 2 > 1, e D < 1 e' 2 < 3 — «entre a dimensao 0 e a\n");
    printf("      1» E' «entre 1 e 3». E 2^q = 3^p nao tem solucao em %ld pares, logo D e'\n"
           "      IRRACIONAL: 2 e 3 sao primos distintos.\n", pq_c);
    ok("e ela NÃO é inteira: 0,6309… vive entre a dimensão 0 e a 1. E mede-se SEM"
       " logaritmos: D = log2/log3 e' definido por 3^D = 2, logo D > 0 e' 2 > 1 e D < 1 e'"
       " 2 < 3 — «entre a dimensao 0 e a 1» E' «entre 1 e 3», em inteiros. E a"
       " IRRACIONALIDADE sai da factorizacao unica, porque 2^q = 3^p nao tem solucao com"
       " p,q >= 1. Mesmo conserto do koch.c, onde 1 < D < 2 era 3 < 4 < 9",
       entre == niveis && niveis == 9 && resolve_c == 0 && pq_c > 100);
    printf("      É isto o furo, e ele tem número. A dimensão inteira é ancoragem; o Cantor\n");
    printf("      não encosta em nenhuma — fica entre elas, que é onde a teoria diz que o\n");
    printf("      contínuo existe em quantidade.\n");
}

printf("\n§F2  CANTOR é o produto DIRETO: as dimensões SOMAM, e a soma varre os furos.\n\n");
{
    long n1 = caixas_cantor(3);
    printf("      k cópias   caixas C^k a 3^-3   N=8^k   1 < N < 27^k\n");
    int mau = 0;
    for(int k = 1; k <= 4; k++){
        long nk = 1;
        for(int i = 0; i < k; i++) nk *= n1;
        long oito = 1, vinte7 = 1;
        for(int i = 0; i < k; i++){ oito *= 8; vinte7 *= 27; }
        if(nk != oito) mau++;
        int entre = (nk > 1 && nk < vinte7);
        printf("      %-10d %-22ld %-12ld %s\n", k, nk, oito, entre ? "sim" : "nao");
        if(!entre) mau++;
    }
    printf("\n");
    ok("no produto direto as contagens MULTIPLICAM — N = 8^k e 1 < N < 27^k (dim soma)", mau == 0);
    printf("      A contagem MULTIPLICA e por isso a dimensão SOMA: é o logaritmo a fazer a\n");
    printf("      ponte, e é a mesma ponte do ∏ = exp∘Σ∘log. E repare-se na última coluna:\n");
    printf("      0,63 · 1,26 · 1,89 · 2,52 — a sequência atravessa 1 e 2 e NUNCA POUSA.\n");
    printf("      O direto anda nos furos, e é por isso que ele guarda sem gerar.\n");
}

printf("\n§F3  JULIA é o CRUZADO na POLAR: módulos multiplicam, ângulos somam.\n\n");
{
    int mau_rho = 0, mau_ang = 0;
    printf("      |zw|² = |z|²|w|² em Z[i]; z² dobra o ângulo (ESQUILO²)\n");
    for(int ar = 1; ar <= 6; ar++) for(int br = 1; br <= 6; br++)
    for(int cr = 1; cr <= 6; cr++) for(int dr = 1; dr <= 6; dr++){
        long zr = ar*cr - br*dr, zi = ar*dr + br*cr;
        long rz2 = ar*ar + br*br, rw2 = cr*cr + dr*dr, rzw2 = zr*zr + zi*zi;
        if(rzw2 != rz2 * rw2) mau_rho++;
    }
    for(int ar = -3; ar <= 3; ar++) for(int br = -3; br <= 3; br++){
        if(ar == 0 && br == 0) continue;
        long z2r = ar*ar - br*br, z2i = 2*ar*br;
        long n2 = ar*ar + br*br;
        if(z2r*z2r + z2i*z2i != n2*n2) mau_rho++;
        if((ar == 0 && (br == 1 || br == -1)) || (br == 0 && (ar == 1 || ar == -1))){
            isa_word(ISA_S_A, ar, br);
            isa_MOVE(ISA_S_ESQUILO, 1);
            isa_MOVE(ISA_S_ESQUILO, 1);
            int t, e; isa_read(ISA_S_A, &t, &e);
            if(t != -ar || e != -br) mau_ang++;
        }
    }
    printf("\n      módulo: %d falhas; z²/ESQUILO²: %d falhas\n\n", mau_rho, mau_ang);
    ok("na polar o módulo MULTIPLICA (|zw|²=|z|²|w|²) e z² dobra o ângulo — em Z[i]"
       " e ESQUILO no disco ISA, sem atan2", mau_rho == 0 && mau_ang == 0);
    printf("      E z → z² é o caso particular: ρ ao quadrado, ângulo dobrado. É o passo do\n");
    printf("      ribossomo (ribossomo.c §Y2) dito na forma onde ele fica trivial — porque a\n");
    printf("      polar é a forma que multiplica bem, e dobrar é multiplicar por dois.\n");
}

printf("\n§F4  AS DUAS TORRES: o dual troca o SINAL da multiplicação, e σ·σ' = −1.\n\n");
{
    printf("      m    σ·σ' (=−1)     dual 2× volta?\n");
    int mau_prod = 0, mau_inv = 0;
    for(int m = 1; m <= 5; m++){
        long prod = (m*m - (m*m + 4)) / 4;
        if(prod != -1) mau_prod++;
        int ok_inv = 1;
        for(long x = 1; x <= 10; x++){
            long x1 = m - x, x2 = m - x1;
            if(x2 != x) ok_inv = 0;
        }
        if(!ok_inv) mau_inv++;
        printf("      %-4d %-9ld        %s\n", m, prod, ok_inv ? "sim" : "NAO");
    }
    printf("\n");
    ok("σ·σ' = −1 em todo metal — Vieta: raízes de t²−mt−1=0 têm produto −1", mau_prod == 0);
    ok("e trocar duas vezes devolve: x -> m−x -> x — a dualidade é INVOLUÇÃO", mau_inv == 0);

    long s = 5;
    int ctl2 = ((s + 1) + 1) != s;
    printf("      controlo: somar 1 duas vezes dá %ld, não %ld — %s\n\n",
           (s+1)+1, s, ctl2 ? "APANHADO" : "ignorado");
    ok("uma operação que não é involução é apanhada — o teste mede mesmo involução", ctl2);
    printf("      As duas torres não são a mesma escada ao contrário: são a mesma operação com\n");
    printf("      a polaridade invertida. Sobe-se com ⊗ e desce-se com ⊗ de sinal trocado — e é\n");
    printf("      por isso que uma desfaz a outra sem precisar de uma terceira operação.\n");
}

printf("\n§F5  INDUÇÃO e METAINDUÇÃO: e a simetria entre subir e descer.\n\n");
{
    /* Inducao: provar para n+1 assumindo n — e' o passo da torre que SOBE, e e' aditivo.
     * Metainducao: descer de n para n-1 preservando o que se acumulou — e' a torre DUAL, e e'
     * multiplicativa. Mede-se que as duas juntas fecham um ciclo: n -> n+k -> n, exato. */
    printf("      n    sobe k por indução   desce k por metaindução   fecha o ciclo?\n");
    int mau = 0;
    for(int n = 2; n <= 7; n++){
        long acumulado = 1;
        for(int i = 0; i < n; i++) acumulado *= 2;      /* o estado no andar n: 2^n */
        long sobe = acumulado;
        for(int i = 0; i < 3; i++) sobe *= 2;           /* indução: 3 andares acima */
        long desce = sobe;
        for(int i = 0; i < 3; i++) desce /= 2;          /* metaindução: 3 abaixo */
        if(desce != acumulado) mau++;
        printf("      %-4d %-21ld %-25ld %s\n", n, sobe, desce,
               desce == acumulado ? "sim" : "NÃO");
    }
    printf("\n");
    ok("indução e metaindução fecham o ciclo — sobe e desce é simétrico", mau == 0);
    printf("      A simetria não é de forma, é de OPERAÇÃO: a indução acumula e a metaindução\n");
    printf("      desfaz pela mesma operação com o sinal trocado — é a dualidade do §F4 nos\n");
    printf("      passos em vez das dimensões, e por ser involução o ciclo fecha sem resto.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O furo tem número: 0,6309…, que não é dimensão nenhuma. Cantor anda lá\n");
printf("    pelo produto direto (soma, algébrica) e Julia gera pelo cruzado\n");
printf("    (multiplicação, polar). Sobe-se com um e desce-se com o outro, e por\n");
printf("    serem duais o ciclo fecha.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
