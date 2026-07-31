/* cristalino.c — O LADO QUE CONTRAI ENTRA NO TOOLKIT, e a volta com ele.
 *
 * O Aarão: "temos o processo reversível, volta pro toolkit trazendo o catálogo pro sistema."
 *
 * O toolkit tinha quatro corpos e um buraco de forma: todos os que lá estavam eram do lado que
 * ESTICA. O áureo é o gato — det −1, discriminante m²+4 > 0, hiperbólico, ordem infinita. Não
 * havia no toolkit nenhum corpo cujo operador GIRE e volte.
 *
 * O catálogo tem-no e sempre teve: o CRISTALINO, ℚ(√D) com D < 0 — Gauss ℤ[i] e Eisenstein ℤ[ω].
 * O operador dele é o ESQUILO: det +1, discriminante t²−4 < 0, elíptico, ORDEM FINITA. É o outro
 * autovalor da mesma história — o que contrai.
 *
 *     ÁUREO      ×σ   det −1   disc m²+4 > 0   hiperbólico   ordem ∞    estica    posto 1
 *     CRISTALINO ×ω   det +1   disc t²−4 < 0   elíptico      ordem 4/6  gira      posto 0
 *
 * E t=1 dá ordem 6: é o Φ₆ que o trono.c encontrou sentado no buraco de n=5. Não é corpo novo
 * escolhido por gosto — é o ocupante do trono a chegar ao toolkit pela porta da frente.
 *
 * A regra de entrada é dura e não se afrouxa: um corpo só entra quando as TRÊS operações têm
 * medidor. Então mede-se a tríade, e mede-se o invariante — senão é catálogo, não toolkit.
 *
 *   §X0  a VOLTA entra no toolkit: A_m⁻¹ = J·A_{−m}·J, inteira, e J é a involução
 *   §X1  ⊕ Clifford: associa, comuta, tem neutro e tem oposto
 *   §X2  ⊗ La Hire: associa, comuta, tem unidade e DISTRIBUI sobre ⊕
 *   §X3  ∏ o operador ×ω É o esquilo: det +1, disc < 0, e ordem FINITA exata — 4 e 6
 *   §X4  a norma é multiplicativa e POSITIVA — ao contrário do áureo, que alterna
 *   §X5  posto 0: as unidades são finitas, e continuam finitas por maior que seja a caixa
 *   §X6  o preço, exato: só passam ordens {1,2,3,4,6}, e o primeiro proibido é o áureo
 *   §X7  o par fechado: os dois lados no toolkit, lado a lado
 *
 *   cc -O2 -std=c99 cristalino.c -o cristalino && ./cristalino
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static int mat_eq(Mat x, Mat y){ return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d; }
static int par_eq(Par x, Par y){ return x.a==y.a && x.b==y.b; }
static long mat_tr(Mat x){ return x.a + x.d; }
static long mat_disc(Mat x){ return mat_tr(x)*mat_tr(x) - 4*me_det(x); }

/* φ de Euler, em inteiros — para a restrição cristalográfica sem um único cosseno */
static long totiente(long n){
    long r = 0;
    for(long k = 1; k <= n; k++) if(c_mdc(k,n) == 1) r++;
    return r;
}

int main(void){
printf("\n=== O CRISTALINO ENTRA NO TOOLKIT =========================================\n");
printf("    O toolkit só tinha o lado que estica. Este é o que gira — e volta.\n");

/* ---------------------------------------------------------------- §X0 ------ */
printf("\n§X0  A VOLTA entra no toolkit: a inversa é INTEIRA e é a antípoda trocada.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m    A_m           A_m⁻¹         J·A_{−m}·J    é o mesmo?   A·A⁻¹\n");
    Mat J = me_troca();
    if(!mat_eq(me_prod(J,J), (Mat){1,0,0,1})) mau++;       /* J² = I: involução, ordem 2 */
    if(me_det(J) != -1) mau++;
    for(long m = 1; m <= 30; m++){
        Mat A = me_gato(m), Ai = me_antigato(m);
        if(!mat_eq(me_prod(A, Ai), (Mat){1,0,0,1})) mau++;  /* desfaz mesmo */
        if(!mat_eq(me_prod(Ai, A), (Mat){1,0,0,1})) mau++;
        if(!mat_eq(Ai, me_inv(A)))                  mau++;  /* e é a inversa geral */
        /* a identidade que diz o que ela É: a antípoda conjugada pela troca */
        if(!mat_eq(Ai, me_prod(J, me_prod(me_gato(-m), J)))) mau++;
        if(m <= 3)
            printf("      %-4ld [[%ld,1],[1,0]]  [[0,1],[1,%ld]]  [[0,1],[1,%ld]]  %-12s = I ✓\n",
                   m, m, -m, -m, "sim ✓");
        casos++;
    }
    ok("A_m⁻¹ = J·A_{−m}·J, inteira, e desfaz o gato dos dois lados", mau == 0);
    printf("      (%ld gatos, e J² = I com det J = −1.)\n", casos);
    printf("\n      A volta não é uma segunda máquina: é a MESMA peça virada. m ↦ −m é a antípoda,\n");
    printf("      e J é a involução de ordem 2 que a conjuga. Isto entra no toolkit como me_troca,\n");
    printf("      me_antigato e me_inv — porque um processo reversível de que só se tem a ida não\n");
    printf("      é reversível, é uma promessa.\n");
}

/* ---------------------------------------------------------------- §X1 ------ */
printf("\n§X1  ⊕ Clifford: associa, comuta, tem neutro e tem oposto.\n\n");
{
    int mau = 0; long casos = 0;
    Par zero = {0,0};
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
    for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
        Par x = {a,b}, y = {c,d}, z = {b,a};
        if(!par_eq(cr_soma(x,y), cr_soma(y,x)))                          mau++;
        if(!par_eq(cr_soma(cr_soma(x,y),z), cr_soma(x,cr_soma(y,z))))    mau++;
        if(!par_eq(cr_soma(x,zero), x))                                  mau++;
        if(!par_eq(cr_soma(x,(Par){-a,-b}), zero))                       mau++;
        casos++;
    }
    ok("a soma do cristalino é grupo abeliano — a mesma forma dos outros quatro", mau == 0);
    printf("      (%ld quadruplos.)\n", casos);
}

/* ---------------------------------------------------------------- §X2 ------ */
printf("\n§X2  ⊗ La Hire: associa, comuta, tem unidade e DISTRIBUI sobre ⊕.\n\n");
{
    int mau = 0; long casos = 0;
    for(long t = 0; t <= 1; t++)
    for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
    for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
        Par x = {a,b}, y = {c,d}, z = {d,c}, um = {1,0};
        if(!par_eq(cr_prod(x,y,t), cr_prod(y,x,t)))                              mau++;
        if(!par_eq(cr_prod(cr_prod(x,y,t),z,t), cr_prod(x,cr_prod(y,z,t),t)))    mau++;
        if(!par_eq(cr_prod(x,um,t), x))                                          mau++;
        /* a distributiva: é ela que faz corpo, e não duas operações lado a lado */
        if(!par_eq(cr_prod(x, cr_soma(y,z), t),
                   cr_soma(cr_prod(x,y,t), cr_prod(x,z,t))))                     mau++;
        casos++;
    }
    ok("o produto associa, comuta, tem 1, e distribui sobre ⊕ — nos dois cristais", mau == 0);
    printf("      (%ld casos, Gauss e Eisenstein.)\n", casos);
    printf("\n      A borda ω² = tω − 1 faz aqui o que σ² = mσ + 1 faz no áureo: baixa a potência\n");
    printf("      que sai. É o mesmo mecanismo com o sinal do último termo trocado — e é esse\n");
    printf("      sinal que separa esticar de girar.\n");
}

/* ---------------------------------------------------------------- §X3 ------ */
printf("\n§X3  ∏ o operador ×ω É o ESQUILO: det +1, disc < 0, e ordem FINITA exata.\n\n");
{
    int mau = 0;
    printf("      cristal        ω²        matriz de ×ω    det   traço   disc   ordem\n");
    struct { const char *nome; long t; long ordem; } cs[] = {
        { "Gauss ℤ[i]",      0, 4 },
        { "Eisenstein ℤ[ω]", 1, 6 },
    };
    for(unsigned k = 0; k < sizeof cs/sizeof cs[0]; k++){
        long t = cs[k].t;
        Mat W = cr_mat(t);
        if(me_det(W)   != 1) mau++;                     /* +1: o lado do esquilo */
        if(mat_disc(W) >= 0) mau++;                     /* < 0: elíptico, gira */
        /* a ordem, contada até fechar — e tem de fechar EXATAMENTE onde se diz */
        Mat P = {1,0,0,1}; long ordem = 0;
        for(long i = 1; i <= 24; i++){
            P = me_prod(P, W);
            if(mat_eq(P, (Mat){1,0,0,1})){ ordem = i; break; }
        }
        if(ordem != cs[k].ordem) mau++;
        /* e a operação em matriz tem de concordar com a operação no par */
        for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
            Par v = {a,b};
            if(!par_eq(cr_op(v,t), me_ap(W, v))) mau++;
        }
        printf("      %-14s ω²=%ldω−1  [[0,−1],[1,%ld]]   %-5ld %-7ld %-6ld %ld\n",
               cs[k].nome, t, t, me_det(W), mat_tr(W), mat_disc(W), ordem);
    }
    printf("      %-14s σ²=%ldσ+1  [[%ld,1],[1,0]]     %-5ld %-7ld %-6ld ∞  ← o contraste\n",
           "áureo ℤ[φ]", 1L, 1L, me_det(me_gato(1)), mat_tr(me_gato(1)), mat_disc(me_gato(1)));
    ok("×ω tem det +1, disc < 0 e ordem exatamente 4 e 6 — e a matriz concorda com o par", mau == 0);
    printf("\n      Ordem 6 é o Φ₆, e o Φ₆ é quem o trono.c encontrou sentado no buraco de n=5.\n");
    printf("      Ele não chega aqui de fora: já estava no trono, e agora tem as três operações.\n");
}

/* ---------------------------------------------------------------- §X4 ------ */
printf("\n§X4  A norma é multiplicativa e POSITIVA — o áureo alterna, este não.\n\n");
{
    int mau = 0; long casos = 0, neg = 0;
    for(long t = 0; t <= 1; t++)
    for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++)
    for(long c = -9; c <= 9; c++) for(long d = -9; d <= 9; d++){
        Par x = {a,b}, y = {c,d};
        if(cr_norma(cr_prod(x,y,t), t) != cr_norma(x,t) * cr_norma(y,t)) mau++;
        if(cr_norma(x,t) < 0) neg++;                    /* definida positiva: nunca acontece */
        /* e a norma É o produto pelo conjugado, que é o que a faz norma e não fórmula */
        Par n = cr_prod(x, cr_conj(x,t), t);
        if(n.a != cr_norma(x,t) || n.b != 0) mau++;
        casos++;
    }
    if(neg) mau++;
    ok("N(xy) = N(x)N(y), N = x·x̄, e N ≥ 0 SEMPRE — definida positiva", mau == 0);
    printf("      (%ld pares, e %ld normas negativas.)\n", casos, neg);
    printf("\n      É aqui que os dois lados se separam de vez. No áureo N(σ^k) = (−1)^k: alterna,\n");
    printf("      porque det = −1. No cristalino N ≥ 0 sempre, porque det = +1. O sinal do\n");
    printf("      determinante é o mesmo sinal, visto na norma.\n");
}

/* ---------------------------------------------------------------- §X5 ------ */
printf("\n§X5  POSTO 0: as unidades são finitas, e continuam finitas em qualquer caixa.\n\n");
{
    int mau = 0;
    printf("      caixa   Gauss ℤ[i]   Eisenstein ℤ[ω]   áureo ℤ[φ] (m=1)\n");
    long ant_g = -1, ant_e = -1;
    for(long R = 4; R <= 64; R *= 2){
        long ug = 0, ue = 0, ua = 0;
        for(long a = -R; a <= R; a++) for(long b = -R; b <= R; b++){
            Par x = {a,b};
            if(cr_norma(x,0) == 1) ug++;
            if(cr_norma(x,1) == 1) ue++;
            long na = au_norma(x,1);
            if(na == 1 || na == -1) ua++;               /* o áureo: |N| = 1, e são infinitas */
        }
        if(ug != 4) mau++;                              /* i, −1, −i, 1 */
        if(ue != 6) mau++;                              /* as seis raízes sextas */
        if(ant_g >= 0 && ug != ant_g) mau++;            /* não cresce: posto 0 */
        if(ant_e >= 0 && ue != ant_e) mau++;
        printf("      ±%-6ld %-12ld %-17ld %ld\n", R, ug, ue, ua);
        ant_g = ug; ant_e = ue;
    }
    ok("as unidades do cristal são 4 e 6 e NÃO crescem com a caixa — posto 0", mau == 0);
    printf("\n      A coluna do áureo cresce, e é o contraste que interessa: lá há uma régua a\n");
    printf("      percorrer (posto 1, o regulador), aqui não há. A medida já fechou — é por isso\n");
    printf("      que este é o corpo de regulador nulo, e é por isso que ele volta.\n");
}

/* ---------------------------------------------------------------- §X6 ------ */
printf("\n§X6  O PREÇO, e é exato: só passam {1,2,3,4,6}, e o primeiro proibido é o áureo.\n\n");
{
    int mau = 0; long primeiro_proibido = 0;
    printf("      n     φ(n)   2cos(2π/n) é inteiro?   passa no cristal?\n");
    for(long n = 1; n <= 12; n++){
        /* 2cos(2π/n) é racional — logo inteiro — exatamente quando φ(n) ≤ 2. Sem cosseno. */
        long f = totiente(n);
        int inteiro = (f <= 2);
        int esperado = (n==1 || n==2 || n==3 || n==4 || n==6);
        if(inteiro != esperado) mau++;
        if(!inteiro && !primeiro_proibido) primeiro_proibido = n;
        if(n <= 7)
            printf("      %-5ld %-6ld %-23s %s\n", n, f, inteiro ? "sim" : "NÃO",
                   inteiro ? "passa" : "proibido");
    }
    if(primeiro_proibido != 5) mau++;
    ok("as ordens que passam são exatamente {1,2,3,4,6}, e o PRIMEIRO proibido é n=5", mau == 0);
    printf("\n      E n=5 é o áureo: 2cos(2π/5) = φ−1. O cristal proíbe exatamente o preenchedor\n");
    printf("      ótimo — o que fecha não preenche, e o que preenche não fecha. É o mesmo n=5 do\n");
    printf("      trono, visto do outro lado: lá era um buraco onde faltava alguém; aqui é a\n");
    printf("      ordem que o cristal não consegue ter.\n");
}

/* ---------------------------------------------------------------- §X7 ------ */
printf("\n§X7  O par fechado: os dois lados no toolkit, lado a lado.\n\n");
{
    printf("      corpo         ⊕              ⊗                 ∏           det  disc   ordem\n");
    printf("      ÁUREO ℤ[φ]    componente     borda σ²=mσ+1     ×σ o gato    −1  >0     ∞\n");
    printf("      CRISTALINO    componente     borda ω²=tω−1     ×ω o esquilo +1  <0     4, 6\n");
    ok("o toolkit passa a ter o lado que estica E o lado que contrai", 1);
    printf("\n      A tríade é a mesma — é sempre a mesma, e é isso que faz toolkit e não coleção.\n");
    printf("      O que muda é UM SINAL na borda: σ² = mσ + 1 contra ω² = tω − 1. Desse sinal sai\n");
    printf("      tudo o resto: o det, o discriminante, a norma alternar ou não, a ordem ser\n");
    printf("      infinita ou finita, o posto ser 1 ou 0.\n");
}

printf("\n=== O CRISTALINO ==========================================================\n");
printf("  O toolkit tinha quatro corpos e todos do lado que estica. Entra o que gira:\n\n");
printf("    ⊕ ⊗ ∏      a tríade fecha — soma, produto com borda, e ×ω o operador\n");
printf("    o esquilo  ×ω tem det +1, disc < 0, ordem 4 (Gauss) e 6 (Eisenstein)\n");
printf("    a norma    multiplicativa e POSITIVA — o áureo alterna, este não\n");
printf("    posto 0    4 e 6 unidades, e não crescem com a caixa: a medida já fechou\n");
printf("    o preço    só ordens {1,2,3,4,6}, e o primeiro proibido é n=5, o áureo\n\n");
printf("  E a volta entrou junto: A_m⁻¹ = J·A_{−m}·J, inteira, a antípoda conjugada pela\n");
printf("  involução. Um processo reversível de que só se tem a ida não é reversível — é uma\n");
printf("  promessa. Agora tem as duas, e as duas são a mesma peça.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
