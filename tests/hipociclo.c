/* hipociclo.c — O MECANISMO. La Hire, e não o lcm.
 *
 * Correção do Aarão, e ela é séria: lcm não é mecanismo nenhum, é o NÚMERO que sai. Eu tinha
 * escrito R^i ∨ R^j = R^lcm(i,j) e chamado isso de operação — é rótulo aritmético, não máquina.
 * O mecanismo é um só: LA HIRE, o rolamento sem escorregar de uma roda dentro de outra. E o lcm
 * é o FECHAMENTO do rolamento, não a sua causa.
 *
 * A tradução é exata, e é medida aqui:
 *
 *     Frobenius φ(x) = x^p   É A ROTAÇÃO      — ordem exatamente n em R^n
 *     o pai R^i              É A RODA         — os pontos fixos de φ^i, período i
 *     o filho                É O FECHAMENTO   — onde as duas rodas voltam JUNTAS
 *     o traço x + φ^i(x)+…   É O TRAÇADO      — o ponto da roda desenhando a curva
 *
 * E o hipociclo traçado por R^i dentro de R^k tem r = k/i PONTAS. Daí:
 *
 *     r = 2   →   LA HIRE: φ^i é involução, o hipociclo DEGENERA EM RETA, e o traçado
 *                 é o vai-e-vem sobre o diâmetro. É a única razão em que a rotação vira
 *                 translação — e é por isso que a multiplicação racional do projeto é
 *                 La Hire e não outra coisa: é a passagem que não perde nada.
 *
 *   §H1  Frobenius é a rotação: ordem exata n, nem antes nem depois
 *   §H2  o rolamento: duas rodas i e j voltam à configuração inicial em lcm(i,j) passos
 *   §H3  e o filho é esse fechamento — o número não é definido, é MEDIDO no mecanismo
 *   §H4  o traçado: o traço é aditivo, cai no pai, e é p^(k−i) para 1
 *   §H5  LA HIRE: r=2 é a única razão em que φ^i é involução — e aí a curva é RETA
 *
 *   cc -O2 -std=c99 hipociclo.c -o hipociclo && ./hipociclo
 */
#include <stdio.h>

#define KMAX 8
#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }

static unsigned RED[KMAX+1];
static unsigned mulk(unsigned a, unsigned b, int n){
    unsigned r = 0, red = RED[n];
    for(int i = 0; i < n; i++) if((b >> i) & 1u) r ^= a << i;
    for(int t = 2*n-2; t >= n; t--) if((r >> t) & 1u) r ^= red << (t - n);
    return r & ((1u << n) - 1u);
}
static int irred(unsigned poly, int n){
    unsigned m = 1u << n;
    for(unsigned a = 2; a < m; a++)
        for(unsigned b = 2; b < m; b++){
            unsigned p = 0;
            for(int i = 0; i < n; i++) if((b >> i) & 1u) p ^= a << i;
            if(p == (poly | (1u << n))) return 0;
        }
    return 1;
}
static void prepara(void){
    for(int n = 1; n <= KMAX; n++){
        RED[n] = 0;
        for(unsigned p = 0; p < (1u << n); p++)
            if(irred(p, n)){ RED[n] = p | (1u << n); break; }
    }
}
/* φ^t : a rotação aplicada t vezes */
static unsigned frob(unsigned x, int t, int n){
    for(int s = 0; s < t; s++) x = mulk(x, x, n);
    return x;
}

int main(void){
prepara();
printf("\n=== O MECANISMO: LA HIRE, E NÃO O lcm =====================================\n");
printf("    O rolamento sem escorregar de uma roda dentro de outra. O lcm é o\n");
printf("    FECHAMENTO do rolamento — o número que sai —, não a máquina que o produz.\n");

/* ---------------------------------------------------------------- §H1 ------ */
printf("\n§H1  Frobenius É a rotação: em R^n tem ordem exatamente n.\n\n");
{
    int mau = 0;
    printf("      n    menor t>0 com φ^t = id    n    confere\n");
    for(int n = 1; n <= 6; n++){
        long tot = 1L << n;
        int ordem = 0;
        for(int t = 1; t <= 2*n && !ordem; t++){
            int todos = 1;
            for(unsigned x = 0; x < tot; x++) if(frob(x, t, n) != x){ todos = 0; break; }
            if(todos) ordem = t;
        }
        if(ordem != n) mau++;
        printf("      %d    %22d    %d    %s\n", n, ordem, n, ordem==n?"✓":"✗");
    }
    ok("a rotação fecha a volta em n passos — nem antes, nem depois", mau == 0);
    printf("\n      Não é analogia: é uma roda de n posições, e o corpo R^n é o seu perímetro.\n");
    printf("      Cada aplicação de φ é um dente. A volta inteira devolve tudo ao lugar.\n");
}

/* ---------------------------------------------------------------- §H2 ------ */
printf("\n§H2  O ROLAMENTO: duas rodas, i e j, andando juntas sem escorregar.\n\n");
{
    int mau = 0;
    printf("      i   j   passos até (0,0) de novo   lcm(i,j)   confere\n");
    for(int i = 1; i <= 6; i++) for(int j = 1; j <= 6; j++){
        /* o mecanismo, literalmente: dois discos avançam um dente por passo,
         * cada um com o seu número de dentes. Quando os dois voltam ao 0 juntos,
         * a figura fechou. Ninguém define nada — conta-se. */
        long a = 0, b = 0, passos = 0;
        do { a = (a+1) % i; b = (b+1) % j; passos++; } while(a || b);
        if(passos != mmc(i,j)) mau++;
        if((i==2&&j==3)||(i==2&&j==4)||(i==4&&j==6)||(i==3&&j==6)||(i==5&&j==6))
            printf("      %d   %d   %23ld   %8ld   %s\n", i, j, passos, mmc(i,j),
                   passos==mmc(i,j)?"✓":"✗");
    }
    ok("a figura fecha, e fecha no lcm — MEDIDO no mecanismo, não definido", mau == 0);
    printf("\n      É aqui que o lcm nasce. Ele não é a operação: é a hora em que o desenho\n");
    printf("      se fecha. A operação é o rolamento, e ela acontece dente a dente.\n");
}

/* ---------------------------------------------------------------- §H3 ------ */
printf("\n§H3  E o filho é esse fechamento — as duas rodas dentro do mesmo corpo.\n\n");
{
    int mau = 0, fora = 0, testados = 0;
    printf("      i   j   menor k que segura as duas rodas   passos do §H2   confere\n");
    for(int i = 1; i <= 4; i++) for(int j = 1; j <= 4; j++){
        /* o lado do corpo é varrido ponto a ponto, e 2^k pontos só cabem no exaustivo
         * até k=6 aqui. Quem fecha depois disso fica DE FORA — e é contado, não calado. */
        {   long a = 0, b = 0, p = 0;
            do { a = (a+1) % i; b = (b+1) % j; p++; } while(a || b);
            if(p > 6){ fora++; continue; }
        }
        testados++;
        /* procura, sem usar lcm: o menor k em que R^i e R^j cabem os dois.
         * "caber" medido pelo mecanismo — φ^i fixa 2^i pontos e φ^j fixa 2^j. */
        int achado = 0;
        for(int k = 1; k <= 6 && !achado; k++){
            long tot = 1L << k, fi = 0, fj = 0;
            for(unsigned x = 0; x < tot; x++){
                if(frob(x, i, k) == x) fi++;
                if(frob(x, j, k) == x) fj++;
            }
            if(fi == (1L << i) && fj == (1L << j)) achado = k;
        }
        long a = 0, b = 0, passos = 0;
        do { a = (a+1) % i; b = (b+1) % j; passos++; } while(a || b);
        if(achado != passos) mau++;
        if((i<=2&&j<=3)||(i==2&&j==4)||(i==3&&j==4))
            printf("      %d   %d   %33d   %13ld   %s\n", i, j, achado, passos,
                   achado==passos?"✓":"✗");
    }
    ok("o corpo do filho É o passo em que o rolamento fecha", mau == 0);
    printf("      (%d pares conferidos; %d ficaram de fora por fecharem além de k=6,\n", testados, fora);
    printf("       que é o limite da varredura exaustiva aqui — não é resultado, é alcance.)\n");
    printf("\n      Duas contas independentes — uma em engrenagem, outra em pontos fixos —\n");
    printf("      dão o mesmo número. O filho não é escolhido: ele é onde a figura fecha.\n");
}

/* ---------------------------------------------------------------- §H4 ------ */
printf("\n§H4  O TRAÇADO: o ponto da roda desenhando a curva. r = k/i pontas.\n\n");
{
    int mau_a = 0, mau_i = 0, mau_f = 0;
    printf("      i   k   r=k/i   traço aditivo   imagem = R^i   fibra   2^(k−i)\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        int r = k / i;
        long tot = 1L << k;
        /* o traçado: soma das r posições da roda — o ponto visto de todas as pontas */
        int adit = 1, naim = 1;
        int nim = 0;
        static char vis[1 << 6];
        for(long t = 0; t < 64; t++) vis[t] = 0;
        for(unsigned x = 0; x < tot; x++){
            unsigned tr = 0;
            for(int t = 0; t < r; t++) tr ^= frob(x, i*t, k);
            if(frob(tr, i, k) != tr) naim = 0;         /* o traçado cai no pai? */
            if(!vis[tr]){ vis[tr] = 1; nim++; }
            for(unsigned y = 0; y < tot; y++){
                unsigned ty = 0, txy = 0;
                for(int t = 0; t < r; t++){ ty ^= frob(y, i*t, k); txy ^= frob(x^y, i*t, k); }
                if(txy != (tr ^ ty)) adit = 0;
            }
        }
        long fibra = tot / nim;
        if(!adit) mau_a++;
        if(!naim || nim != (1L << i)) mau_i++;
        if(fibra != (1L << (k-i))) mau_f++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==2)||(k==6&&i==3)||(k==4&&i==1))
            printf("      %d   %d   %5d   %13s   %12s   %5ld   %7ld\n", i, k, r,
                   adit?"sim ✓":"NÃO", (naim&&nim==(1L<<i))?"sim ✓":"NÃO",
                   fibra, 1L << (k-i));
    }
    ok("o traçado é ADITIVO — o desenho não distorce a soma", mau_a == 0);
    ok("e cai exatamente no pai: a curva vive no corpo de baixo", mau_i == 0);
    ok("cada ponto do pai é atingido 2^(k−i) vezes — fibra constante", mau_f == 0);
    printf("\n      A curva tem r = k/i pontas, e o pai é onde ela mora. Aditivo quer dizer\n");
    printf("      que o mecanismo não inventa: soma de traçados é traçado da soma.\n");
}

/* ---------------------------------------------------------------- §H5 ------ */
printf("\n§H5  LA HIRE: r=2 é a ÚNICA razão em que a curva é RETA.\n\n");
{
    int mau = 0;
    printf("      i   k   r=k/i   ordem de φ^i   involução?   a curva\n");
    for(int k = 1; k <= 6; k++) for(int i = 1; i <= k; i++){
        if(k % i) continue;
        int r = k / i;
        long tot = 1L << k;
        /* φ^i é a rotação da roda pequena vista de dentro da grande.
         * Ela é involução — ida e volta, sem meio-termo — se e só se r = 2. */
        int inv = 1;
        for(unsigned x = 0; x < tot; x++) if(frob(frob(x, i, k), i, k) != x){ inv = 0; break; }
        if(inv != (r == 2 || r == 1)) mau++;
        if((k==2&&i==1)||(k==4&&i==2)||(k==6&&i==3)||(k==6&&i==2)||(k==3&&i==1))
            printf("      %d   %d   %5d   %12d   %10s   %s\n", i, k, r, r,
                   inv?"sim":"não", r==2 ? "RETA (La Hire)" : "hipociclo de r pontas");
    }
    ok("φ^i é involução exatamente quando r = 2 (fora do trivial r = 1)", mau == 0);
    printf("\n      É o teorema de La Hire no corpo: a roda de raio metade, rolando por dentro,\n");
    printf("      traça uma RETA — a rotação vira translação. Com r=2 o mecanismo é ida-e-volta\n");
    printf("      sobre o diâmetro: nada gira, nada se perde, e desfaz-se aplicando de novo.\n");
    printf("\n      Com r≥3 a curva tem pontas e o percurso não é reversível ponto a ponto —\n");
    printf("      passa-se pelas pontas. Por isso a multiplicação racional do projeto é\n");
    printf("      La Hire e não outra: é a única razão em que a passagem não perde nada.\n");
}

printf("\n=== O QUE É O MECANISMO ===================================================\n");
printf("  Não há operação chamada lcm. Há UM mecanismo — o rolamento de La Hire — e o\n");
printf("  lcm é o passo em que ele fecha a figura:\n\n");
printf("    a rotação    φ(x) = x^p, de ordem exata n em R^n — a roda\n");
printf("    o pai        R^i = pontos fixos de φ^i — a roda de i dentes\n");
printf("    o filho      o passo em que as duas rodas voltam juntas ao 0\n");
printf("    o traçado    o traço, aditivo, com r = k/i pontas, morando no pai\n");
printf("    LA HIRE      r = 2: a curva degenera em RETA, e a passagem é reversível\n\n");
printf("  As operações do filho vêm daí: a soma é o traçado somando (aditivo, §H4) e a\n");
printf("  multiplicação é o par de rolamentos, um em cada roda. O número lcm é o que se\n");
printf("  LÊ no fim, quando o desenho fecha — nunca o que se faz.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
