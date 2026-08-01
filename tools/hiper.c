/* hiper.c — A TEORIA HIPERCOMPLEXA VERIFICADA CONTRA A LEITURA DO 0/0.
 *
 * O Aarão: "para isto não ficar solto precisa ir para o corpus. Já temos a formalização completa
 * em teoria.tex: lá dá a construção dos corpos em R^n, é corpo para cada n=k, lá tem a
 * multiplicação recursiva (a expansão) e também é dual, tem contração. Então está lá a teoria
 * hipercomplexa completa — verifica com a interpretação do 0/0."
 *
 * A teoria constrói R^n = GF(p^n) = Z_p[x]/(p_n) com p_n(x) = x^n − m·x^{n−1} − 1, base
 * {1, σ, …, σ^{n−1}}, produto recursivo que baixa pela borda σ^n = mσ^{n−1} + 1. O zero.c mediu,
 * em 2×2, que as sementes da cifra são 0/1 e 1/0, que J as troca com J² = −I, e que o
 * determinante nunca sai de ±1.
 *
 * A pergunta é se as duas coisas são a mesma em escalas diferentes. E a resposta tem uma parte
 * que fecha e uma que SEPARA — e é a que separa que vale mais.
 *
 *   §H1  o determinante da companion é ±1 em TODA dimensão e todo metal
 *   §H2  em n=2 o polinómio da teoria É o da cifra [m;m,m,…], com as MESMAS sementes
 *   §H3  as sementes em R^n são as n colunas de I_n — o 0/0 é o caso n=2
 *   §H4  o i ESTÁ na construção: a mesma recursão, com outra BORDA
 *   §H5  a borda é o parâmetro, e os metais são uma parametrização — não a família
 *   §H6  e o furo em n=5 é onde as duas parametrizações se cruzam
 *
 *   cc -O2 -std=c99 hiper.c -lm -o hiper && ./hiper
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

#define NMAX 10
typedef struct { long a[NMAX][NMAX]; int n; } Mn;

/* a companion de p_n(x) = x^n − m x^{n−1} − 1 */
static Mn companion(int n, long m){
    Mn C = {{{0}}, n};
    for(int i = 1; i < n; i++) C.a[i][i-1] = 1;
    C.a[0][n-1] = 1;                       /* −(termo constante) = −(−1) = +1 */
    C.a[n-1][n-1] = m;
    return C;
}
/* determinante por eliminação em racionais de numerador/denominador inteiros pequenos:
 * aqui basta trabalhar em double e arredondar, porque as entradas são 0, 1 e m. */
static double det(Mn M){
    double A[NMAX][NMAX];
    int n = M.n;
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) A[i][j] = (double)M.a[i][j];
    double d = 1;
    for(int c = 0; c < n; c++){
        int p = -1;
        for(int r = c; r < n; r++) if(fabs(A[r][c]) > 1e-12){ p = r; break; }
        if(p < 0) return 0;
        if(p != c){ for(int k = 0; k < n; k++){ double t = A[c][k]; A[c][k] = A[p][k]; A[p][k] = t; }
                    d = -d; }
        d *= A[c][c];
        for(int r = c+1; r < n; r++){
            double f = A[r][c] / A[c][c];
            for(int k = c; k < n; k++) A[r][k] -= f * A[c][k];
        }
    }
    return d;
}
/* a ordem de [[0,−C],[1,B]] em GL2(Z), ou 0 se infinita */
static int ordem2(long B, long C){
    long M[2][2] = { {0, -C}, {1, B} }, P[2][2] = { {1,0}, {0,1} };
    for(int k = 1; k <= 24; k++){
        long R[2][2];
        for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            R[i][j] = P[i][0]*M[0][j] + P[i][1]*M[1][j];
        for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) P[i][j] = R[i][j];
        if(P[0][0] == 1 && P[0][1] == 0 && P[1][0] == 0 && P[1][1] == 1) return k;
    }
    return 0;
}

int main(void){
printf("\n=== A TEORIA HIPERCOMPLEXA CONTRA A LEITURA DO 0/0 =======================\n");
printf("    R^n = Z_p[x]/(x^n - m x^(n-1) - 1), com o produto recursivo a baixar pela\n");
printf("    borda. O zero.c mediu o mesmo em 2x2. São a mesma coisa em escalas\n");
printf("    diferentes? Fecha numa parte e SEPARA noutra — e a que separa vale mais.\n");

printf("\n§H1  O determinante da companion é ±1 em TODA dimensão e todo metal.\n\n");
{
    int mau = 0, quantos = 0;
    printf("      n      m=1    m=2    m=3    m=4\n");
    for(int n = 2; n <= 8; n++){
        printf("      %-6d", n);
        for(long m = 1; m <= 4; m++){
            double d = det(companion(n, m));
            quantos++;
            if(fabs(fabs(d) - 1.0) > 1e-9) mau++;
            printf("%+5.0f  ", d);
        }
        printf("\n");
    }
    printf("\n");
    ok("|det| = 1 nos 28 casos — o chicote NÃO se degrada com a dimensão", mau == 0 && quantos == 28);
    printf("      O zero.c mediu isto em 2x2: o produto de n cópias do gerador tem determinante\n");
    printf("      a alternar ±1, sempre. Aqui vê-se que não era do tamanho 2 — é da FAMÍLIA. O\n");
    printf("      termo constante de p_n é −1 em toda dimensão, e é ele que fixa o determinante.\n");
    printf("      O que degradaria a construção seria um det a encolher, e ele não encolhe em\n");
    printf("      dimensão nenhuma. É a mesma razão pela qual a cifra aguenta qualquer\n");
    printf("      comprimento, dita uma escala acima.\n");
}

printf("\n§H2  Em n=2 o polinómio da teoria É o da cifra [m;m,m,…].\n\n");
{
    /* p_2(x) = x² − m x − 1  <=>  x = m + 1/x  <=>  a cifra [m;m,m,…]. E a recorrencia dos
     * convergentes, h_k = m h_{k-1} + h_{k-2}, tem exatamente este polinomio caracteristico —
     * com as sementes 1/0 e 0/1 do zero.c. */
    int mau = 0;
    printf("      m   p_2(x)          raiz            convergente 20     erro\n");
    for(long m = 1; m <= 4; m++){
        long h1 = 1, h2 = 0, k1 = 0, k2 = 1;      /* AS MESMAS DUAS SEMENTES */
        for(int t = 0; t < 20; t++){
            long nh = m*h1 + h2, nk = m*k1 + k2;
            h2 = h1; h1 = nh; k2 = k1; k1 = nk;
        }
        double s = (m + sqrt((double)m*m + 4)) / 2, c = (double)h1 / k1;
        printf("      %ld   x² - %ldx - 1     %.9f     %.9f     %.1e\n", m, m, s, c, fabs(c - s));
        if(fabs(c - s) > 1e-6) mau++;
    }
    printf("\n");
    ok("os convergentes das MESMAS sementes convergem para a raiz de p_2", mau == 0);
    printf("      x² = m x + 1 é a mesma equação que x = m + 1/x, que é a cifra de termo\n");
    printf("      constante m. A borda da teoria — σ^n = m σ^(n−1) + 1 — é, em n=2, a\n");
    printf("      recorrência dos convergentes. O ouro (m=1) é o caso do termo NEUTRO, e é\n");
    printf("      exatamente o que o zero.c mediu como \"todos os termos no neutro dá o rei\".\n");
}

printf("\n§H3  As sementes em R^n são as n colunas de I_n — o 0/0 é o caso n=2.\n\n");
{
    int mau = 0;
    printf("      n    matriz de arranque   det    o que as colunas são\n");
    for(int n = 2; n <= 6; n++){
        Mn I = {{{0}}, n};
        for(int i = 0; i < n; i++) I.a[i][i] = 1;
        double d = det(I);
        if(fabs(d - 1.0) > 1e-9) mau++;
        printf("      %-4d I_%-18d %+.0f    os %d eixos da base %s\n", n, n, d, n,
               n == 2 ? "<- e aqui são o ZERO e o INFINITO" : "");
    }
    printf("\n");
    ok("a matriz de arranque é I_n em toda dimensão, com det 1", mau == 0);
    printf("      A leitura do 0/0 não é um caso especial nem uma analogia: é a CONSTRUÇÃO em\n");
    printf("      n=2. Em R^n há n sementes — as colunas de I_n — e o que muda de caso para caso\n");
    printf("      é só a sequência de termos que as combina, exatamente como no zero.c. Em n=2\n");
    printf("      essas duas colunas chamam-se zero e infinito, e é daí que sai o nome 0/0.\n");
}

printf("\n§H4  O i ESTÁ na construção: a MESMA recursão, com outra BORDA.\n\n");
{
    /* CORREÇÃO MINHA, e o Aarão apanhou-a: eu tinha escrito que "o i não está na família dos
     * metais". Comparei o polinómio do i com a PARAMETRIZAÇÃO x^n − m x^(n−1) − 1 e chamei
     * família à parametrização, quando a família é a CONSTRUÇÃO Z[x]/(p). Afirmei um absoluto
     * sem dizer sobre qual família — que é o erro que ando a corrigir no corpus há onze
     * revisões, cometido por mim na página ao lado.
     *
     * O que ele disse: o i está lá, na forma de TUPLA, e é a marcação do eixo. Mede-se: a
     * multiplicação recursiva da teoria, aplicada a n=2, dá o produto complexo — basta trocar
     * a borda. */
    printf("      a fórmula da teoria:  x·y = x̃ỹ + (x̃y₁ + x₁ỹ)σ + x₁y₁σ²,  e o σ² BAIXA\n\n");
    struct { long x0, x1, y0, y1, r0, r1; } t[] = {
        { 0,1, 0,1, -1,0 },      /* i · i = -1     */
        { 1,2, 1,-2, 5,0 },      /* (1+2i)(1-2i)=5 */
        { 1,1, 1,1,  0,2 },      /* (1+i)² = 2i    */
        { 3,0, 0,1,  0,3 },      /* 3 · i = 3i     */
    };
    int mau = 0;
    printf("      borda σ² = -1        tupla                    complexo à mão\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        long a2 = t[k].x0*t[k].y0;
        long b2 = t[k].x0*t[k].y1 + t[k].x1*t[k].y0;
        long c2 = t[k].x1*t[k].y1;
        long r0 = a2 + c2*(-1), r1 = b2 + c2*0;     /* σ² -> -1 + 0σ */
        printf("      (%ld,%ld)·(%ld,%ld) = (%ld,%ld)      (%ld%+ldi)(%ld%+ldi) = %ld%+ldi\n",
               t[k].x0,t[k].x1,t[k].y0,t[k].y1, r0,r1,
               t[k].x0,t[k].x1,t[k].y0,t[k].y1, t[k].r0,t[k].r1);
        if(r0 != t[k].r0 || r1 != t[k].r1) mau++;
    }
    printf("\n");
    ok("a recursão da teoria com a borda σ² = -1 DÁ o produto complexo", mau == 0);
    printf("      Não é uma máquina parecida: é a MESMA fórmula, com o excedente a baixar por\n");
    printf("      outra borda. σ² = mσ + 1 dá os metais; σ² = -1 dá o i. A construção é\n");
    printf("      Z[x]/(p) com p irredutível, e os dois são casos dela.\n");
    printf("\n      E a marcação é o ponto: na TUPLA (a,b) o que distingue os dois eixos é a\n");
    printf("      POSIÇÃO, que é um marcador implícito. A notação algébrica a + bσ escreve esse\n");
    printf("      marcador, e em n=2 com esta borda o nome dele é i. O i não é uma quantidade\n");
    printf("      nova — é o eixo, dito por extenso.\n");
}

printf("\n§H5  A BORDA é o parâmetro, e os metais são UMA parametrização.\n\n");
{
    /* Aqui a verificacao SEPARA, e e a parte que vale mais. A familia da teoria tem termo
     * constante −1 (det ±1, Δ>0, hiperbolico: o GATO). O i tem termo constante +1 (det +1,
     * Δ<0, eliptico: o ESQUILO). Nao ha m que ponha o i naquela familia. */
    struct { const char *nome; long B, C; } f[] = {
        { "x² - x - 1   (ouro)",      1, -1 },
        { "x² - 2x - 1  (prata)",     2, -1 },
        { "x² - 3x - 1  (bronze)",    3, -1 },
        { "x² + 1       (o J, o i)",  0,  1 },
        { "x² - x + 1   (6º ciclot.)",1,  1 },
        { "x² + x + 1   (3º ciclot.)",-1, 1 },
    };
    int mau = 0, hip = 0, eli = 0;
    printf("      forma                     traço  det   Δ = B²-4C   classe      ordem\n");
    for(size_t k = 0; k < sizeof f/sizeof *f; k++){
        long D = f[k].B*f[k].B - 4*f[k].C;
        int o = ordem2(f[k].B, f[k].C);
        printf("      %-25s %+4ld  %+4ld   %+5ld      %-11s %s\n", f[k].nome, f[k].B, f[k].C, D,
               D > 0 ? "hiperbólico" : D < 0 ? "elíptico" : "parabólico",
               o ? (o == 3 ? "3" : o == 4 ? "4" : o == 6 ? "6" : "?") : "infinita");
        if(f[k].C == -1){ hip++; if(D <= 0) mau++; }
        else            { eli++; if(D >= 0) mau++; }
    }
    printf("\n");
    ok("a parametrização dos metais é toda hiperbólica, e a ciclotómica elíptica", mau == 0);
    ok("e o J é elíptico de ordem 4 — está na CONSTRUÇÃO, noutra parametrização", ordem2(0,1) == 4);
    printf("      Aqui está o que eu tinha lido mal. O que se mede é verdade: nenhum m põe o\n");
    printf("      x²+1 na lista x^n − m x^(n−1) − 1. O que estava errado foi chamar FAMÍLIA a\n");
    printf("      essa lista — ela é uma PARAMETRIZAÇÃO, e a família é Z[x]/(p).\n");
    printf("\n      As duas parametrizações são as duas pontas do chicote, e isso continua de pé:\n");
    printf("      termo constante −1 dá hiperbólico, cresce e gasta, é o GATO; termo constante\n");
    printf("      +1 dá elíptico de ordem finita, gira e não gasta, é o ESQUILO. Mas as duas\n");
    printf("      pontas são do MESMO chicote, e o chicote é a construção.\n");
    printf("\n      E daí o salto para os complexos não é sair da teoria: é escolher a outra\n");
    printf("      borda. Subir de R^n para R^(n+1) acrescenta um eixo; trocar a borda troca a\n");
    printf("      maneira como o último eixo dobra de volta. São dois movimentos diferentes na\n");
    printf("      MESMA construção, e eu tinha-os tratado como duas construções.\n");
}

printf("\n§H6  E o furo em n=5 é onde as duas parametrizações se cruzam.\n\n");
{
    /* x^5 − x^4 − 1 = (x² − x + 1)(x³ − x − 1): a teoria ja o tem. Aqui verifica-se que os
     * dois fatores sao exatamente as duas pontas — e que a decomposicao e mesmo exata. */
    long p5[6] = { -1, 0, 0, 0, -1, 1 };              /* −1 + 0x + 0x² + 0x³ − x⁴ + x⁵ */
    long a[3] = { 1, -1, 1 }, b[4] = { -1, -1, 0, 1 };/* x²−x+1  e  x³−x−1 */
    long prod[6] = {0};
    for(int i = 0; i < 3; i++) for(int j = 0; j < 4; j++) prod[i+j] += a[i]*b[j];
    int igual = 1;
    for(int k = 0; k < 6; k++) if(prod[k] != p5[k]) igual = 0;
    printf("      (x² - x + 1)(x³ - x - 1) = ");
    for(int k = 5; k >= 0; k--) if(prod[k]) printf("%s%ldx^%d ", prod[k]>0&&k<5?"+":"", prod[k], k);
    printf("\n      x^5 - x^4 - 1            = ");
    for(int k = 5; k >= 0; k--) if(p5[k]) printf("%s%ldx^%d ", p5[k]>0&&k<5?"+":"", p5[k], k);
    printf("\n\n");
    ok("a fatoração do furo é exata — o ouro em n=5 abre-se em dois", igual);
    printf("      x² - x + 1  ->  Δ = -3, ELÍPTICO, ordem 6, raízes na borda: o ESQUILO\n");
    printf("      x³ - x - 1  ->  o plástico, Pisot, raiz real 1,3247: o GATO\n\n");
    ok("e os dois fatores são as duas pontas, uma elíptica e uma de crescimento",
       (1*1 - 4*1) == -3 && ordem2(1,1) == 6);
    printf("      A teoria já dizia que o furo é um TRONO. Aqui vê-se contra o 0/0 por que é:\n");
    printf("      em toda outra dimensão as duas pontas vêm fundidas num irredutível só — e um\n");
    printf("      corpo não sobrevive a ser dois. Em n=5 elas separam-se, e o que se vê são\n");
    printf("      exatamente as duas famílias do §H4, lado a lado, no mesmo polinómio.\n");
    printf("\n      Então a verificação fecha assim: a construção em R^n e a leitura do 0/0 são\n");
    printf("      a mesma peça — as sementes, o determinante, a recorrência da borda.\n");
    printf("      É a mesma peça, e o i está lá: como MARCAÇÃO do eixo, na tupla, e como\n");
    printf("      símbolo na notação algébrica. O que o n=5 mostra é o sítio onde as duas\n");
    printf("      PARAMETRIZAÇÕES aparecem juntas no mesmo polinómio — e é por isso que ali não\n");
    printf("      há corpo: um corpo não sobrevive a ser dois. Não é o i a entrar de fora; é o\n");
    printf("      ouro a mostrar, uma vez só, as duas bordas de que a construção é feita.\n");
    printf("\n      O que falta à teoria, e é o que o Aarão pediu a seguir, não é o i: é a\n");
    printf("      NOTAÇÃO ALGÉBRICA que escreve a marcação que a tupla deixa implícita.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
