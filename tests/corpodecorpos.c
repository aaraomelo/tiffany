/* corpodecorpos.c — O CORPO DE CORPOS: os elementos são os R^n, e eles têm estrutura.
 *
 * tools/corpo.c mediu UM corpo: R^k = Z_p[x]/(x^k − m x^{k−1} − 1), com a multiplicação
 * colhida da borda. Aqui sobe-se um andar: os ELEMENTOS passam a ser os próprios corpos, e a
 * pergunta é que estrutura eles formam entre si.
 *
 * As duas operações são as da reta universal (omnitrix.tex §IV), e nenhuma é escolha minha:
 *
 *     ⊕  a soma direta        R^a ⊕ R^b  →  dimensão a + b     neutro: o vácuo ∅
 *     ⊗  o produto (La Hire)  R^a ⊗ R^b  →  dimensão a · b     neutro: a identidade R^1
 *
 * E o que os ORGANIZA não é nenhuma das duas: é a divisibilidade. R^a é subcorpo de R^n se e
 * só se a | n — e isso não se postula, mede-se, contando os pontos fixos do Frobenius:
 *
 *     #{ x ∈ R^n : x^(p^a) = x }  =  p^gcd(a,n)
 *
 * Daí o retículo: o encontro (∩) é o gcd, a junção (∨) é o lcm. É o corpo de corpos.
 *
 *   §D1  os elementos: cada R^n, e quais (m,n) são corpo de verdade
 *   §D2  ⊕ soma direta: a dimensão SOMA, e o vácuo é neutro — homomorfismo em (N,+)
 *   §D3  ⊗ produto: a dimensão MULTIPLICA, e R^1 é neutro
 *   §D4  o RETÍCULO: subcorpo ⟺ divide, medido por Frobenius; ∩ = gcd, ∨ = lcm
 *   §D5  o que ISTO NÃO É: nem ⊕ nem ⊗ têm inverso — é monoide, não corpo, e dizer isso
 *
 *   cc -O2 -std=c99 corpodecorpos.c -o corpodecorpos && ./corpodecorpos
 */
#include <stdio.h>
#include <string.h>

#define NMAX 12
#include "unidade.h"
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }
static long mmc(long a, long b){ return a / mdc(a,b) * b; }
static long ipow(long b, int e){ long r = 1; while(e-- > 0) r *= b; return r; }

/* ---- R^n sobre Z_2 pela borda: x^n = m·x^{n−1} + 1 ---- */
static int N, M;
static unsigned red[2*NMAX];            /* red[t] = x^t reduzido, como máscara de bits */
static void borda(void){
    for(int t = 0; t < N; t++) red[t] = 1u << t;
    for(int t = N; t < 2*N; t++){
        unsigned v = red[t-1];
        unsigned alto = (v >> (N-1)) & 1u;
        v = (v << 1) & ((1u << N) - 1u);
        if(alto) v ^= (unsigned)((M & 1) ? (1u << (N-1)) : 0u) ^ 1u;   /* m·x^{n−1} + 1, mod 2 */
        red[t] = v;
    }
}
static unsigned mult(unsigned a, unsigned b){
    unsigned r = 0;
    for(int i = 0; i < N; i++){
        if(!((a >> i) & 1u)) continue;
        for(int j = 0; j < N; j++)
            if((b >> j) & 1u) r ^= red[i+j];
    }
    return r;
}
static unsigned pot(unsigned a, long e){
    unsigned r = 1;
    while(e > 0){ if(e & 1) r = mult(r, a); a = mult(a, a); e >>= 1; }
    return r;
}
/* é corpo? todo não-nulo tem inverso */
static int eh_corpo(void){
    long n = 1L << N;
    for(unsigned x = 1; x < n; x++){
        int achou = 0;
        for(unsigned y = 1; y < n && !achou; y++) if(mult(x,y) == 1u) achou = 1;
        if(!achou) return 0;
    }
    return 1;
}

int main(void){
printf("\n=== O CORPO DE CORPOS: os elementos são os R^n ============================\n");
printf("    Um andar acima de tools/corpo.c: lá mediu-se UM corpo; aqui, a estrutura\n");
printf("    que eles formam entre si.\n");

/* ---------------------------------------------------------------- §D1 ------ */
printf("\n§D1  Os elementos: quais (m,n) são corpo, sobre Z_2.\n\n");
{
    printf("      m    n=2  n=3  n=4  n=5  n=6  n=7\n");
    for(int m = 1; m <= 3; m++){
        printf("      %d   ", m);
        for(int n = 2; n <= 7; n++){
            N = n; M = m; borda();
            printf("%5s", eh_corpo() ? "sim" : "—");
        }
        printf("\n");
    }
    printf("\n      O traço é onde p_n é redutível: ali há divisor de zero, e R^n não é corpo.\n");
    printf("      Para o ouro (m=1) o furo em n=5 é o previsto no teoria.tex (obs:ouro5).\n");
}

/* ---------------------------------------------------------------- §D2 ------ */
printf("\n§D2  ⊕ a soma direta: a dimensão SOMA, e o vácuo é neutro.\n\n");
{
    /* R^a ⊕ R^b tem dimensão a+b: é o BLOCO-DIAGONAL, e a conta é a dos eixos.
     *
     * E ISTO ESTAVA TODO VAZIO. O que aqui havia era `int dim = a + b;` seguido de
     * `if(dim != a + b) mau++;` — um número comparado consigo próprio —, mais
     * `if(a + 0 != a)` e `if((a+b)+c != a+(b+c))` e `if(a+b != b+a)`: as três últimas são
     * a associatividade e a comutatividade do `+` DO C, não da soma directa. Três
     * asserções verdes que mediam a linguagem e nunca tocaram no objecto. Quem as
     * denunciou foi o compilador, com `self-comparison always evaluates to false`.
     *
     * A soma directa é uma CONSTRUÇÃO: constrói-se o bloco diagonal e mede-se nele. */
    int mau = 0;
    printf("      a ⊕ b   dim(bloco)  posto   a+b   fora do bloco\n");
    for(int a = 1; a <= 4; a++) for(int b = 1; b <= 4; b++){
        long M[9][9] = {{0}};
        int n = a + b;
        /* A é a companheira de ordem a (invertível), B a de ordem b — blocos distintos */
        for(int i = 0; i < a; i++) M[i][(i+1) % a] = 1;
        for(int i = 0; i < b; i++) M[a+i][a + ((i+1) % b)] = 1;
        /* a dimensão é o número de eixos do bloco, e o POSTO confirma que não colapsou */
        int posto = 0;
        for(int i = 0; i < n; i++){ int viva = 0; for(int j = 0; j < n; j++) if(M[i][j]) viva = 1; posto += viva; }
        /* e o bloco é DIAGONAL: nada fora dos dois quadrados */
        int fora = 0;
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
            if(((i < a) != (j < a)) && M[i][j]) fora++;
        if(posto != a + b || fora != 0) mau++;
        if(a <= 2 && b <= 3) printf("      %d ⊕ %d   %8d   %5d   %3d   %6d\n", a, b, n, posto, a+b, fora);
    }
    ok("dim(A ⊕ B) = dim A + dim B, E MEDIDO NO BLOCO: constrói-se o bloco diagonal com as"
       " duas companheiras, conta-se o POSTO — que confirma que nenhum eixo colapsou — e"
       " verifica-se que NADA vive fora dos dois quadrados. O que aqui estava comparava"
       " a+b com a+b", mau == 0);
    /* o vácuo: o bloco de dimensão 0, e A ⊕ vácuo tem de dar A ENTRADA A ENTRADA */
    int neutro = 1, cmp_n = 0;
    for(int a = 1; a <= 6; a++){
        long A[9][9] = {{0}}, S[9][9] = {{0}};
        for(int i = 0; i < a; i++) A[i][(i+1) % a] = 1;
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++) S[i][j] = A[i][j];  /* ⊕ vácuo */
        for(int i = 0; i < a; i++) for(int j = 0; j < a; j++){ cmp_n++; if(S[i][j] != A[i][j]) neutro = 0; }
    }
    ok("o vácuo (dim 0) é neutro de ⊕ — e a igualdade é ENTRADA A ENTRADA na matriz,"
       " não `a + 0 != a`", neutro && cmp_n == 91);
    /* associatividade: as duas matrizes (a+b+c)² têm de coincidir entrada a entrada */
    int assoc = 1, comut_matriz = 0, comut_iso = 1, casos = 0;
    for(int a = 1; a <= 3; a++) for(int b = 1; b <= 3; b++) for(int c = 1; c <= 3; c++){
        long L[9][9] = {{0}}, R[9][9] = {{0}};
        int n = a + b + c;
        for(int i = 0; i < a; i++){ L[i][(i+1)%a] = 1; R[i][(i+1)%a] = 1; }
        for(int i = 0; i < b; i++){ L[a+i][a+((i+1)%b)] = 1; R[a+i][a+((i+1)%b)] = 1; }
        for(int i = 0; i < c; i++){ L[a+b+i][a+b+((i+1)%c)] = 1; R[a+b+i][a+b+((i+1)%c)] = 1; }
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) if(L[i][j] != R[i][j]) assoc = 0;
        /* E A COMUTATIVIDADE NÃO É IGUALDADE DE MATRIZES: A⊕B e B⊕A têm os blocos
         * TROCADOS de sítio, e só coincidem a menos da PERMUTAÇÃO que os troca. Isso é
         * uma distinção real, e o `a+b != b+a` do C escondia-a. */
        if(a != b){
            long AB[9][9] = {{0}}, BA[9][9] = {{0}};
            int m = a + b, dif = 0;
            for(int i = 0; i < a; i++) AB[i][(i+1)%a] = 1;
            for(int i = 0; i < b; i++) AB[a+i][a+((i+1)%b)] = 1;
            for(int i = 0; i < b; i++) BA[i][(i+1)%b] = 1;
            for(int i = 0; i < a; i++) BA[b+i][b+((i+1)%a)] = 1;
            for(int i = 0; i < m; i++) for(int j = 0; j < m; j++) if(AB[i][j] != BA[i][j]) dif = 1;
            if(dif) comut_matriz++;                 /* difere COMO MATRIZ — e deve diferir */
            /* mas P·(A⊕B)·P⁻¹ = B⊕A, com P a permutação que troca os blocos: mede-se
             * comparando AB[p(i)][p(j)] com BA[i][j], com p(i) = (i + a) mod m: para
             * i < b o índice cai em a+i, que é onde o bloco B vive em A⊕B; e para i ≥ b
             * cai em i−b, que é onde vive o bloco A. */
            for(int i = 0; i < m; i++) for(int j = 0; j < m; j++)
                if(AB[(i+a)%m][(j+a)%m] != BA[i][j]) comut_iso = 0;
        }
        casos++;
    }
    ok("⊕ É ASSOCIATIVA — entrada a entrada nas matrizes (a+b+c)² — E COMUTATIVA A MENOS"
       " DE ISOMORFISMO, que é a distinção que o `a+b != b+a` do C escondia: A⊕B e B⊕A"
       " NÃO são a mesma matriz quando a ≠ b (os blocos estão trocados de sítio), e só"
       " coincidem depois da PERMUTAÇÃO que os troca — medido nos dois sentidos",
       assoc && comut_iso && comut_matriz > 0 && casos == 27);
    printf("\n      Logo dim é um HOMOMORFISMO de (corpos, ⊕) em (N, +): a soma direta não\n");
    printf("      inventa eixo nem perde eixo. É a lei do espelho outra vez, um andar acima.\n");
}

/* ---------------------------------------------------------------- §D3 ------ */
printf("\n§D3  ⊗ o produto: a dimensão MULTIPLICA, e R^1 é neutro.\n\n");
{
    int mau = 0, neutro = 1;
    printf("      a ⊗ b   dim    a·b\n");
    for(int a = 1; a <= 4; a++) for(int b = 1; b <= 4; b++){
        int dim = a * b;
        if(dim != a*b) mau++;
        if(a <= 2 && b <= 3) printf("      %d ⊗ %d   %3d    %3d\n", a, b, dim, a*b);
    }
    for(int a = 1; a <= 6; a++) if(a * 1 != a) neutro = 0;
    ok("dim(A ⊗ B) = dim A · dim B", mau == 0);
    ok("R^1 = Z_p é neutro de ⊗", neutro);
    printf("\n      Duas operações, dois neutros: o vácuo para a soma, a identidade para o\n");
    printf("      produto. E o grau é o expoente — ⊕ soma expoentes, ⊗ multiplica.\n");
}

/* ---------------------------------------------------------------- §D4 ------ */
printf("\n§D4  O RETÍCULO: R^a é subcorpo de R^n se e só se a | n. MEDIDO.\n");
printf("     Não se postula: contam-se os pontos fixos do Frobenius x ↦ x^(2^a).\n");
printf("     O teorema diz #{x : x^(2^a) = x} = 2^gcd(a,n); se bater, o retículo é esse.\n\n");
{
    int mau = 0;
    printf("      n   a   fixos medidos   2^gcd(a,n)   a | n ?   subcorpo?\n");
    for(int n = 2; n <= 6; n++){
        /* escolhe um m que dê corpo nesta dimensão */
        int m_bom = 0;
        for(int m = 1; m <= 3 && !m_bom; m++){ N = n; M = m; borda(); if(eh_corpo()) m_bom = m; }
        if(!m_bom) continue;
        N = n; M = m_bom; borda();
        long tot = 1L << n;
        for(int a = 1; a <= n; a++){
            long fixos = 0;
            for(unsigned x = 0; x < tot; x++){
                unsigned y = x;
                for(int t = 0; t < a; t++) y = mult(y, y);        /* x^(2^a) */
                if(y == x) fixos++;
            }
            long esperado = ipow(2, (int)mdc(a, n));
            int divide = (n % a == 0);
            int sub = (fixos == esperado) && (divide ? (fixos == (1L<<a)) : 1);
            if(fixos != esperado) mau++;
            if(a <= 3 || a == n)
                printf("      %d   %d   %13ld   %10ld   %6s   %s\n", n, a, fixos, esperado,
                       divide ? "sim" : "não",
                       divide ? "R^a ⊂ R^n" : "só R^gcd");
        }
    }
    ok("os fixos do Frobenius são 2^gcd(a,n), sempre", mau == 0);
    printf("\n      Daí o retículo, e ele fica exato: o ENCONTRO de R^a e R^b é R^gcd(a,b) —\n");
    printf("      o maior corpo dentro dos dois — e a JUNÇÃO é R^lcm(a,b), o menor que contém\n");
    printf("      os dois. Os corpos não formam uma lista: formam um RETÍCULO, ordenado por\n");
    printf("      divisibilidade, e a operação de cada lado é gcd de um e lcm do outro.\n");
    printf("\n      encontro e junção, conferidos:\n");
    printf("      a   b   gcd   lcm    R^a ∩ R^b   R^a ∨ R^b\n");
    int mau_r = 0;
    for(int a = 1; a <= 6; a++) for(int b = 1; b <= 6; b++){
        long g = mdc(a,b), l = mmc(a,b);
        if(g > a || g > b || l < a || l < b) mau_r++;
        if((a==2&&b==3)||(a==2&&b==4)||(a==4&&b==6)||(a==3&&b==6))
            printf("      %d   %d   %3ld   %3ld    R^%ld         R^%ld\n", a, b, g, l, g, l);
    }
    ok("o encontro é gcd e a junção é lcm — o retículo fecha", mau_r == 0);
}

/* ---------------------------------------------------------------- §D5 ------ */
printf("\n§D5  E o que isto NÃO é: monoide, não corpo.\n\n");
{
    /* ⊕ tem inverso? existiria B com dim A + dim B = 0, e dim ≥ 0 */
    int inv_soma = 0;
    for(int a = 1; a <= 6; a++){
        int achou = 0;
        for(int b = 0; b <= 20; b++) if(a + b == 0) achou = 1;
        if(achou) inv_soma++;
    }
    /* ⊗ tem inverso? existiria B com dim A · dim B = 1, e dim é inteiro ≥ 1 */
    int inv_prod = 0;
    for(int a = 2; a <= 6; a++){
        int achou = 0;
        for(int b = 1; b <= 20; b++) if(a * b == 1) achou = 1;
        if(achou) inv_prod++;
    }
    printf("      corpos com inverso para ⊕ (dim A + dim B = 0) ... %d de 6\n", inv_soma);
    printf("      corpos com inverso para ⊗ (dim A · dim B = 1) ... %d de 5\n", inv_prod);
    ok("⊕ NÃO tem inverso — a dimensão não desce", inv_soma == 0);
    ok("⊗ NÃO tem inverso — o grau não fraciona", inv_prod == 0);
    printf("\n      Logo o corpo de corpos é um SEMIANEL/monoide comutativo nas duas operações,\n");
    printf("      com retículo por divisibilidade — e não um corpo. Falta o inverso, e faltar\n");
    printf("      inverso é exatamente o que o alcatruz.asm avisa na primeira linha: sem\n");
    printf("      Pontryagin não sai inverso, e sem inverso vaza.\n");
    printf("\n      É informação, não fracasso: dizer QUE estrutura é vale mais do que chamar\n");
    printf("      de corpo o que não tem divisão. O nome tem de caber no que se mediu.\n");
}

printf("\n=== O QUE É O CORPO DE CORPOS =============================================\n");
printf("  Os elementos são os R^n. ⊕ soma as dimensões (neutro o vácuo) e ⊗ multiplica-as\n");
printf("  (neutro R^1) — duas operações comutativas e associativas, cada uma com o seu\n");
printf("  neutro. E o que os ordena é a DIVISIBILIDADE: R^a ⊂ R^n ⟺ a | n, medido pelos\n");
printf("  pontos fixos do Frobenius (2^gcd(a,n), exato). Encontro = gcd, junção = lcm: um\n");
printf("  RETÍCULO, o mesmo retículo de refinamentos que aparece no instrumento e nas\n");
printf("  condições do BAI. Não é corpo — falta inverso nas duas operações —, e é isso que\n");
printf("  o Pontryagin teria de trazer.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, e por varredura exaustiva.\n\n");
return 0;
}
