/* rn.c — A MULTIPLICAÇÃO EM R^n, reescrita em quatro peças. E a recursão está no CRUZADO.
 *
 * O Aarão: "aí você reescreve a multiplicação no R^n em termos de soma, multiplicação e produto
 * cruzado, apresentando o produto cruzado no R^n; acredito que aqui fica a recursão, o resto é
 * coordenada real, parte imaginária e produto interno."
 *
 * E está exatamente certo. O produto de R^n, com um escalar e um vetor, é
 *
 *     (a₀, a)·(b₀, b)  =  ( a₀b₀ − ⟨a,b⟩ ,  a₀b + b₀a + a×b )
 *                            \___/  \___/    \_______/  \___/
 *                             mult   interno  imaginária cruzado
 *
 * e as quatro peças não têm o mesmo estatuto:
 *
 *   COORDENADA REAL   a₀b₀        existe em toda dimensão, e é a multiplicação de sempre
 *   PRODUTO INTERNO   ⟨a,b⟩       existe em toda dimensão, é simétrico, e dá a norma
 *   PARTE IMAGINÁRIA  a₀b + b₀a   existe em toda dimensão, e é o escalar a agir no vetor
 *   PRODUTO CRUZADO   a×b         SÓ EXISTE EM DIMENSÃO 1, 3 e 7 — e é aqui que está tudo
 *
 * As três primeiras não mudam com n. A quarta muda, e é ela que decide se o corpo comuta, se
 * associa, e onde a torre para. É o que o Aarão chama "aqui fica a recursão".
 *
 *   §R1  o produto decompõe-se nas quatro peças, e bate C e H
 *   §R2  o cruzado é o ÚNICO termo antissimétrico — e a não-comutatividade É ele
 *   §R3  em C o cruzado é zero (vetor de dim 1), e por isso C comuta
 *   §R4  e o cruzado só existe em dim 1, 3, 7 — é isso que limita a torre
 *   §R5  e os dois são as duas METADES de qualquer produto bilinear
 *   §R6  e são a SOMA e a MULTIPLICAÇÃO do corpo de corpos — o viveiro já o tinha
 *
 *   cc -O2 -std=c99 rn.c -lm -o rn && ./rn
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

static double interno(const double *a, const double *b, int n){
    double s = 0;
    for(int k = 0; k < n; k++) s += a[k]*b[k];
    return s;
}
static void cruz3(const double *a, const double *b, double *c){
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}
/* o produto de R^(1+m): escalar + vetor de dimensão m. cruz = 0 quando não há cruzado. */
static void prod(const double *A, const double *B, int m, void (*cruz)(const double*,const double*,double*),
                 double *R){
    double a0 = A[0], b0 = B[0];
    const double *a = A+1, *b = B+1;
    R[0] = a0*b0 - interno(a, b, m);
    double c[8] = {0};
    if(cruz) cruz(a, b, c);
    for(int k = 0; k < m; k++) R[1+k] = a0*b[k] + b0*a[k] + c[k];
}

int main(void){
printf("\n=== A MULTIPLICAÇÃO EM R^n, EM QUATRO PEÇAS ==============================\n");
printf("    (a0,a)·(b0,b) = ( a0b0 - <a,b> ,  a0b + b0a + a×b )\n");
printf("                       mult  interno   imaginária  CRUZADO\n");
printf("    As três primeiras existem em toda dimensão. A quarta é que muda.\n");

printf("\n§R1  A decomposição bate C e H, sem tabela de multiplicação nenhuma.\n\n");
{
    int mal = 0;
    /* C: escalar + vetor de dim 1, sem cruzado */
    printf("      em C (vetor de dim 1, cruzado inexistente):\n");
    struct { double A[2], B[2], R[2]; } tc[] = {
        { {1,2}, {3,4}, {-5,10} },
        { {0,1}, {0,1}, {-1,0} },
        { {2,0}, {0,3}, {0,6} },
    };
    for(size_t k = 0; k < sizeof tc/sizeof *tc; k++){
        double R[2];
        prod(tc[k].A, tc[k].B, 1, 0, R);
        printf("        (%g,%g)·(%g,%g) = (%g,%g)   e o produto complexo dá (%g,%g)\n",
               tc[k].A[0],tc[k].A[1], tc[k].B[0],tc[k].B[1], R[0],R[1], tc[k].R[0],tc[k].R[1]);
        if(fabs(R[0]-tc[k].R[0]) > 1e-12 || fabs(R[1]-tc[k].R[1]) > 1e-12) mal++;
    }
    /* H: escalar + vetor de dim 3, com cruzado */
    printf("\n      em H (vetor de dim 3, com cruzado):\n");
    double A[4] = {1,2,3,4}, B[4] = {5,6,7,8}, R[4];
    prod(A, B, 3, cruz3, R);
    printf("        (1,2,3,4)·(5,6,7,8) = (%g,%g,%g,%g)\n", R[0],R[1],R[2],R[3]);
    /* conferido contra a tabela de Hamilton: i²=j²=k²=ijk=-1 */
    double esperado[4] = {-60, 12, 30, 24};
    for(int k = 0; k < 4; k++) if(fabs(R[k]-esperado[k]) > 1e-12) mal++;
    printf("        e a tabela de Hamilton dá   (-60,12,30,24)\n\n");
    ok("a fórmula das quatro peças dá C e H, sem tabela de multiplicação", mal == 0);
    printf("      Não há tabela em lado nenhum: há a fórmula, e a única coisa que muda de C\n");
    printf("      para H é a DIMENSÃO do vetor e a existência do cruzado.\n");
}

printf("\n§R2  O cruzado é o ÚNICO termo antissimétrico — e a não-comutatividade É ele.\n\n");
{
    double a[3] = {1,2,3}, b[3] = {4,5,6}, ab[3], ba[3];
    cruz3(a,b,ab); cruz3(b,a,ba);
    printf("      <a,b> = %g   <b,a> = %g          simétrico\n", interno(a,b,3), interno(b,a,3));
    printf("      a×b = (%g,%g,%g)   b×a = (%g,%g,%g)   ANTIssimétrico\n\n",
           ab[0],ab[1],ab[2], ba[0],ba[1],ba[2]);
    int sim = fabs(interno(a,b,3) - interno(b,a,3)) < 1e-12;
    int anti = 1;
    for(int k = 0; k < 3; k++) if(fabs(ab[k] + ba[k]) > 1e-12) anti = 0;
    ok("o interno é simétrico e o cruzado é antissimétrico", sim && anti);

    /* e daí: a diferenca ab - ba e EXATAMENTE 2(a×b) */
    double A[4] = {1,2,3,4}, B[4] = {5,6,7,8}, P[4], Q[4];
    prod(A,B,3,cruz3,P); prod(B,A,3,cruz3,Q);
    double vc[3]; cruz3(A+1, B+1, vc);
    printf("      a·b = (%g,%g,%g,%g)\n", P[0],P[1],P[2],P[3]);
    printf("      b·a = (%g,%g,%g,%g)\n", Q[0],Q[1],Q[2],Q[3]);
    printf("      a·b - b·a = (%g,%g,%g,%g)\n", P[0]-Q[0],P[1]-Q[1],P[2]-Q[2],P[3]-Q[3]);
    printf("      e 2(a×b)  = (0,%g,%g,%g)\n\n", 2*vc[0], 2*vc[1], 2*vc[2]);
    int bate = fabs(P[0]-Q[0]) < 1e-12;
    for(int k = 0; k < 3; k++) if(fabs((P[1+k]-Q[1+k]) - 2*vc[k]) > 1e-12) bate = 0;
    ok("a·b - b·a é EXATAMENTE 2(a×b) — a não-comutatividade é o cruzado", bate);
    printf("      Das quatro peças, três são simétricas em a e b: a0b0, <a,b> e a0b + b0a\n");
    printf("      não mudam se se trocarem os dois. Só o cruzado muda, e muda de SINAL. Logo\n");
    printf("      tudo o que falta à comutatividade está nele, e nada mais.\n");
    printf("\n      E isto amarra ao §F13: lá a não-comutatividade era a ação do produto cruzado\n");
    printf("      no grupo afim; aqui é o termo a×b no produto de R^n. É a MESMA coisa com o\n");
    printf("      mesmo nome, e o nome não é coincidência — é o mesmo objeto a aparecer nos dois\n");
    printf("      sítios, e onde a ordem importa sobra sempre este termo.\n");
}

printf("\n§R3  Em C o cruzado é zero, e é por isso que C comuta.\n\n");
{
    /* o vetor de C tem dimensao 1, e o cruzado de dois vetores de dim 1 e 0 — nao ha
     * antissimetrico nao nulo em dimensao 1. */
    double A[2] = {1,2}, B[2] = {3,4}, P[2], Q[2];
    prod(A,B,1,0,P); prod(B,A,1,0,Q);
    printf("      (1,2)·(3,4) = (%g,%g)     (3,4)·(1,2) = (%g,%g)\n", P[0],P[1],Q[0],Q[1]);
    printf("      o vetor tem dim 1, e o único antissimétrico em dim 1 é o ZERO\n\n");
    ok("C comuta — e a razão é a dimensão do vetor, não uma escolha",
       fabs(P[0]-Q[0]) < 1e-12 && fabs(P[1]-Q[1]) < 1e-12);
    printf("      Não é que C tenha sido feito comutativo: é que em dimensão 1 não há para onde\n");
    printf("      o cruzado apontar. A comutatividade de C é uma CONSEQUÊNCIA da dimensão, e o\n");
    printf("      i comuta consigo por não ter com quem cruzar.\n");
}

printf("\n§R4  E o cruzado só existe em dim 1, 3 e 7 — é isso que limita a torre.\n\n");
{
    /* Um produto vetorial bilinear e antissimetrico com |a×b|² = |a|²|b|² - <a,b>² so existe
     * em dimensao 0, 1, 3 e 7 (teorema classico). Aqui NAO se demonstra: cita-se, e mede-se a
     * consequencia — a torre e onde ela para. */
    printf("      dimensão do vetor   cruzado?   corpo   comuta?  associa?\n");
    printf("      0  (R^1)            n/a        R       sim      sim\n");
    printf("      1  (R^2)            é ZERO     C       sim      sim\n");
    printf("      3  (R^4)            existe     H       NÃO      sim\n");
    printf("      7  (R^8)            existe     O       não      NÃO\n");
    printf("      15 (R^16)           NÃO HÁ     S       não      não, e há divisores de zero\n\n");
    /* mede-se o que se pode medir aqui: que em dim 3 a identidade de Lagrange vale */
    double a[3] = {1,2,3}, b[3] = {4,5,6}, c[3];
    cruz3(a,b,c);
    double lhs = interno(c,c,3);
    double rhs = interno(a,a,3)*interno(b,b,3) - interno(a,b,3)*interno(a,b,3);
    printf("      |a×b|² = %g   e  |a|²|b|² - <a,b>² = %g\n\n", lhs, rhs);
    ok("a identidade de Lagrange vale em dim 3 — é ela que a norma exige", fabs(lhs-rhs) < 1e-9);
    printf("      É esta identidade que o produto tem de cumprir para a norma ser multiplicativa,\n");
    printf("      e é ela que só se satisfaz em dimensão 0, 1, 3 e 7 (teorema clássico, citado e\n");
    printf("      não demonstrado aqui). Logo a torre R -> C -> H -> O e PARA: em R^16 não há\n");
    printf("      cruzado que sirva, a norma deixa de multiplicar, e aparecem divisores de zero.\n");
    printf("\n      E É ISTO O QUE O AARÃO DISSE: a recursão fica no CRUZADO. As outras três peças\n");
    printf("      — a coordenada real, o produto interno e a parte imaginária — existem em toda\n");
    printf("      dimensão e não mudam de forma; escrevem-se uma vez e servem sempre. O cruzado é\n");
    printf("      a peça que carrega a dimensão, e por isso é ele que decide tudo: se comuta, se\n");
    printf("      associa, e onde a construção acaba.\n");
}

printf("\n§R5  E OS DOIS SÃO AS DUAS METADES DE QUALQUER PRODUTO BILINEAR.\n\n");
{
    /* O Aarao: "exato, define bem os dois: produto direto e produto cruzado". A definicao mais
     * limpa nao e uma lista de propriedades: e que eles sao as duas METADES da decomposicao de
     * um bilinear qualquer. */
    printf("      Todo produto bilinear B(a,b) parte-se em duas, e a partição é única:\n\n");
    printf("        B(a,b) = ½[B(a,b) + B(b,a)]  +  ½[B(a,b) - B(b,a)]\n");
    printf("                  \\_______________/      \\_______________/\n");
    printf("                   SIMÉTRICA               ANTISSIMÉTRICA\n");
    printf("                   o DIRETO                o CRUZADO\n\n");
    /* mede-se num bilinear qualquer, em dim 3 */
    double M[3][3] = { {1,2,3}, {4,5,6}, {7,8,10} };     /* B(a,b) = aᵀMb, sem simetria nenhuma */
    int mal = 0;
    double pior_s = 0, pior_a = 0, pior_soma = 0;
    for(int t = 0; t < 6; t++){
        double a2[3] = { 1.0+t, 2.0-t, 0.5*t }, b2[3] = { 3.0-t, 1.0+0.5*t, 2.0 };
        double Bab = 0, Bba = 0;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            Bab += a2[i]*M[i][j]*b2[j];
            Bba += b2[i]*M[i][j]*a2[j];
        }
        double S = (Bab + Bba)/2, A2 = (Bab - Bba)/2;
        /* a simétrica é mesmo simétrica, a antissimétrica é mesmo antissimétrica, e somam */
        double S2 = (Bba + Bab)/2, A3 = (Bba - Bab)/2;
        if(fabs(S - S2) > pior_s) pior_s = fabs(S - S2);
        if(fabs(A2 + A3) > pior_a) pior_a = fabs(A2 + A3);
        if(fabs((S + A2) - Bab) > pior_soma) pior_soma = fabs((S + A2) - Bab);
    }
    printf("      num bilinear qualquer de dim 3, em 6 pares:\n");
    printf("        a parte simétrica não muda ao trocar a e b:   erro %.1e\n", pior_s);
    printf("        a antissimétrica troca de sinal:              erro %.1e\n", pior_a);
    printf("        e as duas somam o original:                   erro %.1e\n\n", pior_soma);
    ok("todo bilinear é a soma de uma parte simétrica e uma antissimétrica",
       pior_s < 1e-12 && pior_a < 1e-12 && pior_soma < 1e-12);
    if(mal) return 1;

    printf("      E DAÍ SAEM AS DUAS DEFINIÇÕES, e elas não são listas de propriedades:\n\n");
    printf("      PRODUTO DIRETO   a metade SIMÉTRICA.\n");
    printf("        ⟨a,b⟩ = ⟨b,a⟩             não vê a ordem\n");
    printf("        ⟨a,a⟩ ≥ 0                 dá a NORMA, e é dela que vem a medida\n");
    printf("        existe em TODA dimensão   não tem obstrução nenhuma\n");
    printf("        devolve um ESCALAR        baixa o grau: sai do espaço\n");
    printf("        nos grupos: G × H         (g₁,h₁)(g₂,h₂) = (g₁g₂, h₁h₂), lados ignoram-se\n\n");
    printf("      PRODUTO CRUZADO  a metade ANTISSIMÉTRICA.\n");
    printf("        a×b = -b×a                troca de sinal com a ordem, e a×a = 0\n");
    printf("        ⟨a×b, a⟩ = 0              sai perpendicular aos dois\n");
    printf("        |a×b|² = |a|²|b|² - ⟨a,b⟩²  a identidade de Lagrange, que a norma exige\n");
    printf("        só em dim 1, 3 e 7        TEM obstrução, e é ela que para a torre\n");
    printf("        devolve um VETOR          fica no espaço: não baixa o grau\n");
    printf("        nos grupos: G ⋉ H         (g₁,h₁)(g₂,h₂) = (g₁g₂, g₁·h₂ + h₁), um AGE\n\n");
    {
        double a2[3] = {1,2,3}, b2[3] = {4,5,6}, c[3];
        cruz3(a2,b2,c);
        double perp1 = interno(c,a2,3), perp2 = interno(c,b2,3);
        double aa[3]; cruz3(a2,a2,aa);
        printf("      medido: ⟨a×b, a⟩ = %g,  ⟨a×b, b⟩ = %g,  a×a = (%g,%g,%g)\n\n",
               perp1, perp2, aa[0], aa[1], aa[2]);
        ok("o cruzado sai perpendicular aos dois, e a×a = 0",
           fabs(perp1) < 1e-12 && fabs(perp2) < 1e-12
           && fabs(aa[0])+fabs(aa[1])+fabs(aa[2]) < 1e-12);
    }
    printf("      A DIFERENÇA DE FUNDO, e é uma só: o direto NÃO VÊ A ORDEM e o cruzado É a\n");
    printf("      ordem. Tudo o resto sai daí — o direto ser escalar (perdeu a direção, que era\n");
    printf("      o que a ordem guardava), o cruzado ser vetor (guardou-a), o direto existir\n");
    printf("      sempre (não pede nada) e o cruzado ter obstrução (pede um espaço onde caiba\n");
    printf("      uma direção nova a cada par).\n");
    printf("\n      E é por isso que a recursão fica no cruzado: subir de dimensão é perguntar se\n");
    printf("      ainda há para onde apontar. O direto responde sempre que sim, porque não\n");
    printf("      aponta; o cruzado responde sim em 1, 3 e 7, e depois não.\n");
}

printf("\n§R6  E ESTAS SÃO A SOMA E A MULTIPLICAÇÃO DO CORPO DE CORPOS.\n\n");
{
    /* O Aarao: "essas sao nossas soma e multiplicacao generalizadas, no corpo de corpos —
     * le de novo viveiro.tex".
     *
     * E o viveiro.tex JA TINHA a peca, escrita de outra maneira. La mediu-se que a soma direta
     * NAO VOA (ha divisor de zero sempre) e que o tensorial so voa quando gcd = 1; e que a lei
     * que voa sempre e a JUNCAO, R^a v R^b = R^lcm — que o texto escreve como o tensorial
     * BALANCEADO sobre o comum, R^i ⊗_{R^d} R^j com d = gcd.
     *
     * Balancear sobre o comum É a acao. E entao o desenho fecha: */
    printf("      SOMA generalizada        = o produto DIRETO\n");
    printf("        os dois lados ignoram-se, e nada age em nada\n");
    printf("        no viveiro: a soma direta, R^a ⊕ R^b, dimensão a+b\n");
    printf("        e ela NÃO VOA — (1,0)·(0,1) = (0,0), divisor de zero SEMPRE\n\n");
    printf("      MULTIPLICAÇÃO gen.       = o produto CRUZADO\n");
    printf("        um age no outro, e a ação passa pelo que os dois partilham\n");
    printf("        no viveiro: R^i ⊗_{R^d} R^j com d = gcd — o tensorial BALANCEADO\n");
    printf("        e ela VOA: dá R^lcm, que é corpo e contém os dois pais\n\n");

    /* e a conta que separa as duas: a·b = lcm·gcd. Sem balancear, sobram gcd copias. */
    printf("      e a conta que separa as duas leituras:  a·b = lcm · gcd\n\n");
    printf("        a  b   a·b (tensorial cru)   lcm (balanceado)   gcd = cópias a mais\n");
    int mal = 0;
    struct { int a, b; } pares[] = { {2,3}, {2,2}, {3,4}, {2,4}, {3,3}, {2,6} };
    for(size_t k = 0; k < sizeof pares/sizeof *pares; k++){
        int a2 = pares[k].a, b2 = pares[k].b, g = a2, h = b2;
        while(h){ int t = g % h; g = h; h = t; }
        int l = a2*b2/g;
        printf("        %d  %d   %-20d %-18d %d   %s\n", a2, b2, a2*b2, l, g,
               g == 1 ? "voa" : "não voa: reparte-se");
        if(a2*b2 != l*g) mal++;
    }
    printf("\n");
    ok("a·b = lcm·gcd — o tensorial cru é gcd cópias do balanceado", mal == 0);
    printf("      É por isso que o tensorial PARECE ser a lei quando se testam espécies primas\n");
    printf("      entre si: aí gcd = 1, há uma cópia só, e o cru coincide com o balanceado. Com\n");
    printf("      divisor comum ele reparte-se em gcd cópias, e o divisor de zero aparece — o\n");
    printf("      viveiro.tex já o dizia, e é o mesmo erro de quem generaliza do caso fácil.\n");

    printf("\n      E O QUE O BALANCEAMENTO É, DITO NA LINGUAGEM DE HOJE: é a AÇÃO.\n\n");
    printf("      Sem balancear, os dois lados contam o comum cada um por si — e contar duas\n");
    printf("      vezes o mesmo é justamente não haver ação: cada um faz de conta que o outro\n");
    printf("      não está lá. Balancear é fazer a multiplicação de um passar PELO que o outro\n");
    printf("      já tem, e é isso que o produto cruzado faz com o (a1·b2 + b1) do §F13.\n");
    printf("\n      Então as duas operações do corpo de corpos são as mesmas duas de sempre, um\n");
    printf("      nível acima:\n\n");
    printf("        ⊕  o direto      simétrico, sem ação, soma as dimensões, NÃO voa\n");
    printf("        ⊗  o cruzado     com ação pelo comum, dá o lcm, VOA\n\n");
    printf("      E o par (simétrico, antissimétrico) do §R5 é o mesmo par uma escala abaixo:\n");
    printf("      no vetor, o interno não vê a ordem e o cruzado é a ordem; nos corpos, o direto\n");
    printf("      não vê o outro e o cruzado age nele. \"Não ver\" e \"agir\" são a mesma\n");
    printf("      distinção nas duas escalas, e é ela que separa somar de multiplicar.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
