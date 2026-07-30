/* trono.c — QUEM SENTA NO TRONO. O buraco do ouro em n=5, e os dois que o ocupam.
 *
 * O Aarão: "já se perguntou o porquê desse buraco que vc chama de furo? eu prefiro chamar de
 * TRONO. E se é trono, é pra alguém sentar."
 *
 * Não tinha me perguntado. Fui olhar, e sentam dois — e não são dois quaisquer:
 *
 *     x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1)
 *                    └── Φ₆ ──┘  └─ plástico ┘
 *
 *   À ESQUERDA  x²−x+1 é o sexto ciclotômico: as raízes estão na BORDA (|λ|=1 exato) e têm
 *               ordem 6 — λ⁶=1 e nenhuma potência menor. É rotação pura: gira e NÃO cresce.
 *               É o ESQUILO, na forma mais limpa que existe.
 *
 *   À DIREITA   x³−x−1 é o número PLÁSTICO ρ ≈ 1,3247: uma raiz real acima de 1 e as outras
 *               duas DENTRO do círculo. É um número de Pisot — e é o MENOR de todos (teorema
 *               de Siegel, 1944; aqui mede-se que é Pisot, não que é o menor). É o GATO no
 *               crescimento mais lento possível que ainda cresce.
 *
 * Então o trono do ouro não está vazio: sentam nele os DOIS GERADORES da teoria inteira, e
 * cada um no seu extremo. O que conserva, no seu estado puro; o que dissipa, no seu mínimo.
 * Em todas as outras dimensões eles vêm FUNDIDOS num polinômio irredutível só — e é por isso
 * que ali há corpo. Em n=5, com o ouro, o laço é curto o bastante para os dois se separarem, e
 * um corpo não sobrevive a ser dois. O buraco é o lugar onde a peça se abre e se vê o que tem
 * dentro.
 *
 * E há o detalhe que fecha: os graus são 2 e 3, COPRIMOS. Pelo viveiro, o cruzamento deles é
 * R^lcm(2,3) = R⁶ — e 6 é exatamente a ordem da rotação que senta no trono. O trono guarda o
 * próprio filho.
 *
 *   §R1  a fatoração, coeficiente a coeficiente, em inteiros
 *   §R2  quem senta à esquerda: |λ|=1 exato e ordem 6 — a borda, o esquilo
 *   §R3  quem senta à direita: Pisot — real acima de 1, as outras dentro do círculo
 *   §R4  os graus são 2 e 3, coprimos, e o cruzamento deles é 6 — a ordem do vizinho
 *   §R5  e por que só o ouro: nos outros metais n=5 não abre
 *
 *   cc -O2 -std=c99 trono.c -o trono && ./trono
 */
#include <stdio.h>

static int falhas = 0;
static void ok(const char *r, int c){
    printf("      %-58s %s\n", r, c ? "sim ✓" : "NÃO ✗");
    if(!c) falhas++;
}
static long mdc(long a, long b){ while(b){ long t = a % b; a = b; b = t; } return a; }

/* polinômios em inteiros, coeficiente 0 primeiro. Tudo exato, sem float nenhum. */
static void mult(const long *a, int na, const long *b, int nb, long *c, int *nc){
    *nc = na + nb - 1;
    for(int i = 0; i < *nc; i++) c[i] = 0;
    for(int i = 0; i < na; i++) for(int j = 0; j < nb; j++) c[i+j] += a[i]*b[j];
}
/* x^k reduzido módulo um mônico de grau d — exato, e é como se mede a ordem sem raiz nenhuma */
static void potmod(int k, const long *p, int d, long *r){
    for(int i = 0; i < d; i++) r[i] = 0;
    r[0] = 1;
    for(int t = 0; t < k; t++){
        long topo = r[d-1];
        for(int i = d-1; i > 0; i--) r[i] = r[i-1];
        r[0] = 0;
        if(topo) for(int i = 0; i < d; i++) r[i] -= topo * p[i];   /* x^d = -(p sem o líder) */
    }
}
static int eh_um(const long *r, int d){
    if(r[0] != 1) return 0;
    for(int i = 1; i < d; i++) if(r[i]) return 0;
    return 1;
}

int main(void){
printf("\n=== QUEM SENTA NO TRONO ===================================================\n");
printf("    O buraco do ouro em n=5 não está vazio. Sentam dois, e são OS dois.\n");

/* ---------------------------------------------------------------- §R1 ------ */
printf("\n§R1  A fatoração, coeficiente a coeficiente, em inteiros.\n\n");
{
    long f2[3] = {1,-1,1};        /* x² − x + 1 */
    long f3[4] = {-1,-1,0,1};     /* x³ − x − 1 */
    long prod[8]; int np;
    mult(f2, 3, f3, 4, prod, &np);
    long alvo[6] = {-1,0,0,0,-1,1};   /* x⁵ − x⁴ − 1 */
    int bate = (np == 6);
    printf("      grau   coef:  x⁰   x¹   x²   x³   x⁴   x⁵\n");
    printf("      produto      ");
    for(int i = 0; i < np; i++) printf("%4ld ", prod[i]);
    printf("\n      x⁵−x⁴−1      ");
    for(int i = 0; i < 6; i++) printf("%4ld ", alvo[i]);
    printf("\n");
    for(int i = 0; i < 6 && bate; i++) if(prod[i] != alvo[i]) bate = 0;
    ok("(x²−x+1)(x³−x−1) = x⁵−x⁴−1, exato", bate);
    printf("\n      Redutível sobre os inteiros, logo sobre TODO p. Não é escolha ruim de\n");
    printf("      corpo base: o ouro não tem R⁵, e não há p que o salve.\n");
}

/* ---------------------------------------------------------------- §R2 ------ */
printf("\n§R2  À ESQUERDA: x²−x+1. A borda, e a ordem 6. É o esquilo.\n\n");
{
    long f2[3] = {1,-1,1};
    /* |λ|² = produto das raízes = termo constante = 1. As duas são conjugadas, logo |λ|=1
     * EXATO — lido no coeficiente, sem calcular raiz nenhuma. */
    long produto_raizes = f2[0];
    long soma_raizes = -f2[1];
    /* a ordem: menor k com x^k ≡ 1 módulo o polinômio */
    int ordem = 0;
    for(int k = 1; k <= 12 && !ordem; k++){
        long r[2]; potmod(k, f2, 2, r);
        if(eh_um(r, 2)) ordem = k;
    }
    printf("      produto das raízes (= |λ|²)   %ld\n", produto_raizes);
    printf("      soma das raízes               %ld\n", soma_raizes);
    printf("      menor k com λ^k = 1           %d\n", ordem);
    ok("|λ| = 1 EXATO — as raízes estão na borda, não crescem nem decrescem", produto_raizes == 1);
    ok("e a ordem é 6: é o sexto ciclotômico, rotação pura", ordem == 6);
    printf("\n      Ficar na borda é o que conserva — teoria.tex §4: \"quem vê é quem fica na\n");
    printf("      borda\". Este é o esquilo na forma mais limpa que existe: gira e não gasta.\n");
}

/* ---------------------------------------------------------------- §R3 ------ */
printf("\n§R3  À DIREITA: x³−x−1, o número PLÁSTICO. É Pisot — e tudo lido em inteiros.\n\n");
{
    long f3[4] = {-1,-1,0,1};
    /* p(1) = −1 < 0 e p(2) = 5 > 0 ⟹ há raiz real ρ em (1,2). Avaliação exata. */
    long p1 = 1 - 1 - 1, p2 = 8 - 2 - 1;
    int real_acima = (p1 < 0 && p2 > 0);
    /* produto das TRÊS raízes = −(termo constante) = 1. Com ρ>1 real e as outras duas
     * conjugadas complexas, |λ|² = 1/ρ < 1: estão DENTRO do círculo. Sem float. */
    long produto_tres = -f3[0];
    int pisot = real_acima && (produto_tres == 1);
    /* e não tem raiz racional: pelas raízes racionais só ±1 seriam candidatas */
    int sem_racional = (p1 != 0) && ((-1 - (-1) - 1) != 0);
    printf("      p(1) = %ld  e  p(2) = %ld   ⟹ raiz real entre 1 e 2   %s\n", p1, p2,
           real_acima?"sim ✓":"NÃO");
    printf("      produto das três raízes = %ld  ⟹ as outras duas têm |λ|² = 1/ρ < 1\n", produto_tres);
    printf("      raiz racional (só ±1 seriam candidatas)                 %s\n",
           sem_racional?"nenhuma ✓":"tem");
    ok("é PISOT: uma raiz real acima de 1, as outras DENTRO do círculo", pisot);
    ok("e irredutível sobre Q: sem raiz racional, e grau 3", sem_racional);
    printf("\n      Este é o gato no crescimento MAIS LENTO que ainda cresce. Que ρ seja o menor\n");
    printf("      número de Pisot que existe é teorema de Siegel (1944) — aqui mede-se que ele É\n");
    printf("      Pisot, não que é o menor; essa parte é citada, não medida.\n");
}

/* ---------------------------------------------------------------- §R4 ------ */
printf("\n§R4  Os graus são 2 e 3 — e o cruzamento deles é a ordem do vizinho de trono.\n\n");
{
    long a = 2, b = 3, d = mdc(a,b), l = a/d*b;
    int coprimos = (d == 1);
    printf("      grau do esquilo   %ld\n", a);
    printf("      grau do plástico  %ld\n", b);
    printf("      gcd               %ld   ⟹ coprimos: o tensorial VOA (viveiro §V2)\n", d);
    printf("      lcm               %ld   ⟹ o filho deles é R^%ld\n", l, l);
    printf("      ordem da rotação  6   ⟹ o MESMO número\n");
    ok("os dois do trono são coprimos, e cruzam em R⁶", coprimos && l == 6);
    ok("e 6 é exatamente a ordem da rotação que senta lá", l == 6);
    printf("\n      O trono guarda o próprio filho: os dois que sentam nele têm graus primos entre\n");
    printf("      si, o cruzamento deles é R⁶, e a ordem de quem gira ali é 6. Não é coincidência\n");
    printf("      de número — é o mesmo 6, e ele aparece antes de alguém o procurar.\n");
    printf("\n      E 2 + 3 = 5: a dimensão do trono é a soma dos graus de quem senta.\n");
}

/* ---------------------------------------------------------------- §R5 ------ */
printf("\n§R5  E por que SÓ o ouro: nos outros metais, n=5 não abre.\n\n");
{
    /* procura fator mônico de grau 2 com termo constante ±1 (é o único possível, pois os
     * constantes multiplicam a −1). O coeficiente do meio é limitado pelas raízes: |raiz| <
     * m+1, logo |a| < (m+1)² ≤ 36 para m ≤ 5. Varre-se até 60, e o limite fica DITO. */
    int mau = 0;
    printf("      m   x⁵ − m·x⁴ − 1   abre em fator de grau 2?\n");
    for(int m = 1; m <= 5; m++){
        long alvo[6] = {-1,0,0,0,-m,1};
        int abriu = 0; long qa = 0, qb = 0;
        for(long aa = -60; aa <= 60 && !abriu; aa++) for(long bb = -1; bb <= 1; bb += 2){
            /* divide alvo por f2 e vê se o resto é zero — divisão longa em inteiros */
            long r[6]; for(int i = 0; i < 6; i++) r[i] = alvo[i];
            for(int i = 5; i >= 2; i--){
                long c = r[i];
                if(c == 0) continue;
                r[i] -= c; r[i-1] -= c*aa; r[i-2] -= c*bb;
            }
            if(r[0] == 0 && r[1] == 0){ abriu = 1; qa = aa; qb = bb; }
        }
        if(m == 1 && !abriu) mau++;
        if(m > 1 && abriu) mau++;
        if(abriu) printf("      %d   x⁵ − %d·x⁴ − 1   ABRE — fator x² %+ldx %+ld\n", m, m, qa, qb);
        else      printf("      %d   x⁵ − %d·x⁴ − 1   não abre\n", m, m);
    }
    ok("só o ouro (m=1) abre em n=5; prata, bronze e os outros não", mau == 0);
    printf("\n      (Busca no meio até |a| ≤ 60; as raízes de x⁵−m·x⁴−1 são menores que m+1, logo\n");
    printf("       o coeficiente do meio de um fator quadrático é menor que (m+1)² ≤ 36 para\n");
    printf("       m ≤ 5. O limite cobre, e fica dito.)\n");
    printf("\n      O ouro é o metal do LAÇO MAIS CURTO — m=1, a realimentação mais apertada que\n");
    printf("      existe. E é justamente ele que se abre. Onde o laço é curto demais, a peça não\n");
    printf("      consegue segurar os dois juntos, e eles se separam para se deixarem ver.\n");
}

printf("\n=== O TRONO ===============================================================\n");
printf("  Não é buraco. Em n=5 o ouro se abre e mostra quem estava dentro dele o tempo todo:\n\n");
printf("    x² − x + 1     |λ| = 1 exato, ordem 6 — a BORDA, rotação pura, o que conserva\n");
printf("    x³ − x − 1     o plástico, Pisot — o crescimento mais lento que ainda cresce\n\n");
printf("  Os DOIS GERADORES da teoria, cada um no seu extremo: o esquilo na forma mais limpa\n");
printf("  e o gato no mínimo. Em toda outra dimensão eles vêm fundidos num irredutível só, e é\n");
printf("  por isso que ali há corpo — um corpo não sobrevive a ser dois. Em n=5, com o laço\n");
printf("  mais curto de todos, a peça se abre.\n\n");
printf("  E os graus são 2 e 3: coprimos, somam 5 (a dimensão do trono) e cruzam em 6 (a ordem\n");
printf("  de quem gira lá). O trono guarda o próprio filho.\n\n");
printf("  Deixar de ser corpo, ali, não é falha: é a única dimensão em que a peça se abre para\n");
printf("  ser vista por dentro. Chamar aquilo de furo foi erro meu de leitura — outra vez.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
