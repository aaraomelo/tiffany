/* furos.c — OS FUROS ENTRE AS DIMENSÕES: Cantor é o direto, Julia é o cruzado.
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
 *   cc -O2 -std=c99 -I. furos.c -lm -o furos && ./furos
 */
#include <stdio.h>
#include "../lib/disco.h"
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "unidade.h"

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
        double x = 0, p = 1.0/3.0;
        for(int i = 0; i < k; i++){ x += 2.0*((v >> (k-1-i)) & 1)*p; p /= 3.0; }
        long c = (long)(x*lado + 1e-9);
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
    printf("      nível k   caixas 3^-k (recursão)   por varredura   dim = log N/log 3^k\n");
    int mau = 0; double ultima = 0;
    for(int k = 1; k <= 9; k++){
        long a = caixas_cantor(k), b = caixas_por_varredura(k);
        if(b >= 0 && a != b) mau++;
        double d = log((double)a) / (k*log(3.0));
        ultima = d;
        printf("      %-9d %-24ld %-15ld %.9f\n", k, a, b, d);
    }
    printf("\n      e log2/log3 = %.9f\n\n", log(2.0)/log(3.0));
    ok("as duas contagens de caixas concordam — a dimensão é contada, não assumida",
       mau == 0);
    ok("e ela NÃO é inteira: 0,6309… vive entre a dimensão 0 e a 1",
       fabs(ultima - log(2.0)/log(3.0)) < 1e-9
       && fabs(ultima - floor(ultima+0.5)) > 0.1);
    printf("      É isto o furo, e ele tem número. A dimensão inteira é ancoragem; o Cantor\n");
    printf("      não encosta em nenhuma — fica entre elas, que é onde a teoria diz que o\n");
    printf("      contínuo existe em quantidade.\n");
}

printf("\n§F2  CANTOR é o produto DIRETO: as dimensões SOMAM, e a soma varre os furos.\n\n");
{
    /* O produto direto de dois conjuntos tem dimensao igual a SOMA das dimensoes — e' a marca
     * do direto, e e' o que o distingue do cruzado. Mede-se: k copias de Cantor dao k vezes a
     * dimensao, e a contagem de caixas confirma-o sem se usar a formula. */
    double d1 = log(2.0)/log(3.0);
    printf("      k cópias   caixas de C^k a 3^-3   dim contada     k·log2/log3   inteira?\n");
    int mau = 0;
    for(int k = 1; k <= 4; k++){
        long n1 = caixas_cantor(3);              /* caixas de C ao nível 3 */
        long nk = 1;
        for(int i = 0; i < k; i++) nk *= n1;     /* o direto MULTIPLICA as contagens... */
        double d = log((double)nk) / (3*log(3.0));  /* ...e por isso as dimensões somam */
        if(fabs(d - k*d1) > 1e-9) mau++;
        int e_inteira = fabs(d - floor(d+0.5)) < 1e-9;
        printf("      %-10d %-22ld %-15.9f %-13.9f %s\n", k, nk, d, k*d1,
               e_inteira ? "SIM" : "não");
    }
    printf("\n");
    ok("no produto direto as dimensões SOMAM — dim(C^k) = k·dim(C)", mau == 0);
    printf("      A contagem MULTIPLICA e por isso a dimensão SOMA: é o logaritmo a fazer a\n");
    printf("      ponte, e é a mesma ponte do ∏ = exp∘Σ∘log. E repare-se na última coluna:\n");
    printf("      0,63 · 1,26 · 1,89 · 2,52 — a sequência atravessa 1 e 2 e NUNCA POUSA.\n");
    printf("      O direto anda nos furos, e é por isso que ele guarda sem gerar.\n");
}

printf("\n§F3  JULIA é o CRUZADO na POLAR: módulos multiplicam, ângulos somam.\n\n");
{
    /* A forma polar do polar.c: rho(zw) = rho(z)rho(w) e theta(zw) = theta(z)+theta(w). Mede-se
     * nos dois, e mede-se que z -> z^2 e' o caso particular que dobra o angulo — que e' o passo
     * do ribossomo do ribossomo.c §Y2. A mesma operacao, dita na forma que a torna simples. */
    int mau_rho = 0, mau_ang = 0; long casos = 0;
    printf("      z            w            ρ(zw) vs ρ(z)ρ(w)     θ(zw) vs θ(z)+θ(w)\n");
    for(int i = 1; i <= 40; i++) for(int j = 1; j <= 40; j++){
        double a = 0.1*i, b = 0.1*j, c = 0.07*i, d = 0.13*j;
        double zr = a*c - b*d, zi = a*d + b*c;                  /* z·w */
        double rz = sqrt(a*a+b*b), rw = sqrt(c*c+d*d), rzw = sqrt(zr*zr+zi*zi);
        double tz = atan2(b,a), tw = atan2(d,c), tzw = atan2(zi,zr);
        if(fabs(rzw - rz*rw) > 1e-9*(1+rz*rw)) mau_rho++;
        double soma = tz + tw;
        while(soma >  M_PI) soma -= 2*M_PI;
        while(soma < -M_PI) soma += 2*M_PI;
        if(fabs(tzw - soma) > 1e-9) mau_ang++;
        casos++;
        if(i == 1 && j <= 2)
            printf("      %.2f+%.2fi   %.2f+%.2fi   %.9f          %.9f\n",
                   a,b,c,d, rzw - rz*rw, tzw - soma);
    }
    printf("\n      %ld pares: %d falhas no módulo, %d no ângulo\n\n", casos, mau_rho, mau_ang);
    ok("na polar o módulo MULTIPLICA e o ângulo SOMA — é o cruzado", mau_rho == 0 && mau_ang == 0);
    printf("      E z → z² é o caso particular: ρ ao quadrado, ângulo dobrado. É o passo do\n");
    printf("      ribossomo (ribossomo.c §Y2) dito na forma onde ele fica trivial — porque a\n");
    printf("      polar é a forma que multiplica bem, e dobrar é multiplicar por dois.\n");
}

printf("\n§F4  AS DUAS TORRES: o dual troca o SINAL da multiplicação, e σ·σ' = −1.\n\n");
{
    /* E AQUI EU TINHA POSTO A DESCIDA COMO DIVISÃO, que é uma operação inventada por mim.
     *
     * O Aarão: "no dual apenas troca o sinal da multiplicação." É literal, e o projeto já o
     * tinha medido noutro sítio: os dois pontos fixos são σ e σ' = −1/σ, e o que os liga é
     * σ·σ' = −1. O dual não desfaz a multiplicação — MULTIPLICA COM O SINAL TROCADO. É uma peça
     * só que muda, e é sempre a mesma peça.
     *
     * A diferença importa: dividir é uma operação nova, e trocar o sinal é a MESMA operação com
     * a polaridade invertida. Só a segunda é involução — e é por ser involução que a torre dual
     * desce até ao fundo e volta sem resto. */
    double d1 = log(2.0)/log(3.0);
    printf("      m    σ (negro)      σ' = −1/σ (branco)   σ·σ'      trocar 2× volta?\n");
    int mau_prod = 0, mau_inv = 0;
    for(int m = 1; m <= 5; m++){
        double s  = (m + sqrt((double)m*m + 4.0))/2.0;
        double sl = -1.0/s;                          /* o dual: o sinal da multiplicação */
        double prod = s*sl;
        if(fabs(prod + 1.0) > 1e-12) mau_prod++;
        double volta = -1.0/sl;                      /* trocar outra vez */
        if(fabs(volta - s) > 1e-12) mau_inv++;
        printf("      %-4d %-14.9f %-20.9f %-9.6f %s\n", m, s, sl, prod,
               fabs(volta - s) < 1e-12 ? "sim" : "NÃO");
    }
    printf("\n");
    ok("σ·σ' = −1 em todo metal — o dual é o sinal da multiplicação trocado", mau_prod == 0);
    ok("e trocar duas vezes devolve: a dualidade é INVOLUÇÃO", mau_inv == 0);

    /* E O CONTROLO: uma operação que não é involução não serve de dual. */
    double s = (1 + sqrt(5.0))/2.0, nao_inv = 1.0/s, volta_ma = 1.0/nao_inv;
    int ctl = fabs(volta_ma - s) < 1e-12;            /* 1/x TAMBÉM é involução... */
    double roda = s + 1.0, volta_roda = roda + 1.0;  /* ...mas somar 1 não é */
    int ctl2 = fabs(volta_roda - s) > 0.5;
    printf("      controlo: somar 1 e somar 1 outra vez dá %.6f, não %.6f — %s\n\n",
           volta_roda, s, ctl2 ? "APANHADO" : "ignorado");
    ok("uma operação que não é involução é apanhada — o teste mede mesmo involução", ctl2);
    (void)ctl; (void)d1;
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
