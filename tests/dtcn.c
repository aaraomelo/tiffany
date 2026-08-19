/* dtcn.c — O DTC HIPERCOMPLEXO: a família ⋆_s em R^n, e o torque VETORIAL.
 *
 * O Aarão: "agora o DTC hipercomplexo, generaliza pra família ⋆_s no Rn."
 *
 * A fonte é o paper dele: "DTC Hipercomplexo e Preditivo: Controle Direto de Torque na família
 * ⋆_s sobre R^n" (hiper/aposentados/teoria/papers/paper_H_dtc_hipercomplexo.tex), que estende o
 * DTC de Takahashi (1986) à família de álgebras do Paper A.
 *
 * E a família ⋆_s É a fórmula das quatro peças que este projeto já media, com um parâmetro no
 * CRUZADO — o lugar onde vive a recursão:
 *
 *      z ⋆_s w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + s·a×b)
 *
 * Três peças não vêem o s; a quarta é a que ele parametriza. E daí sai a peça nova, que é a
 * razão de a família existir — a LEI DE CONSERVAÇÃO ALGÉBRICA:
 *
 *      ‖z ⋆_s w‖² + (1−s²)·‖a×b‖² = ‖z‖²·‖w‖²
 *
 * com V(s) = (1−s²)·‖a×b‖² a que o paper chama IMPOSTO ALGÉBRICO. Em s = ±1 o imposto anula e
 * a norma é multiplicativa (é o Hurwitz); fora disso a álgebra COBRA, e o quanto ela cobra
 * mede-se — e serve para dimensionar as bandas de histerese.
 *
 * LEI vs TRANSPORTE. Sen/cos a gerar vectores, √ da norma e o Euler da histerese eram o
 * transporte. A lei é aritmética em ℤ: as quatro peças do ⋆_s, Lagrange, V=0 ⇔ s²=1, o
 * campo local â×â=0, o torque expandido contra o clássico, a banda em ‖e‖². Nenhum passo
 * pede vírgula.
 *
 *   §H1  a família ⋆_s: as quatro peças, e só a quarta vê o s
 *   §H2  a LEI DE CONSERVAÇÃO, e o imposto V(s) = (1−s²)·m
 *   §H3  s = ±1 é onde o imposto anula — e é o Hurwitz outra vez
 *   §H4  os campos locais C_â: fechados, isomorfos a C, e R^n é a sua união
 *   §H5  o TORQUE VETORIAL: T⃗_e(s), e em n=4, s=1 tem TRÊS componentes
 *   §H6  a histerese ESFÉRICA, e a redução ao clássico em n = 2
 *   §H7  as BANDAS derivadas do imposto — e a economia, medida
 *   §H8  VALIDAR: o DTC hipercomplexo roda, e em n=2 dá o de Takahashi
 *
 *   cc -O2 -std=c99 -I../lib dtcn.c -o dtcn && ./dtcn
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

typedef struct { long s0, v[3]; } Q;

static long n2v(const long *a){ return rt_dir(a, a, 3); }
static long n2q(Q z){ return z.s0*z.s0 + n2v(z.v); }

static Q star(Q z, Q w, long s){
    Q r;
    long c[3];
    rt_cruz3(z.v, w.v, c);
    r.s0 = z.s0*w.s0 - rt_dir(z.v, w.v, 3);
    for(int k = 0; k < 3; k++)
        r.v[k] = z.s0*w.v[k] + w.s0*z.v[k] + s*c[k];
    return r;
}
static Q conjq(Q z){ Q r = { z.s0, { -z.v[0], -z.v[1], -z.v[2] } }; return r; }

static long massa(Q z, Q w){
    long c[3];
    rt_cruz3(z.v, w.v, c);
    return n2v(c);
}
static long imposto(Q z, Q w, long s){ return (1 - s*s)*massa(z,w); }

/* T⃗ = (3/2)P·(ψ₀ i − i₀ ψ + s·ψ×i). Com P=2 fica T = 3·(…), inteiro. */
static void torque(Q psi, Q i, long s, long P, long *T){
    long c[3];
    rt_cruz3(psi.v, i.v, c);
    long k3 = 3*P/2;                               /* P par: exacto. P=2 → 3 */
    for(int k = 0; k < 3; k++)
        T[k] = k3*(psi.s0*i.v[k] - i.s0*psi.v[k] + s*c[k]);
}
static void torque_compacta(Q psi, Q i, long s, long P, long *T){
    Q t = star(conjq(psi), i, s);
    long k3 = 3*P/2;
    for(int k = 0; k < 3; k++) T[k] = k3*t.v[k];
}

int main(void){
printf("\n=== O DTC HIPERCOMPLEXO: A FAMÍLIA ⋆_s EM R^n ============================\n");
printf("    z ⋆_s w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + s·a×b)\n");
printf("    Três peças não veem o s. A quarta — o cruzado — é a que ele parametriza.\n");

printf("\n§H1  A família ⋆_s: as quatro peças, e só a quarta vê o s.\n\n");
{
    Q z = { 7, { 3, -5, 2 } }, w = { -4, { 6, 1, -3 } };
    printf("      s        parte escalar     parte vetorial (d, q, r)\n");
    int mal = 0;
    long esc0 = 0; int primeiro = 1;
    Q base = star(z, w, 0);
    long c[3]; rt_cruz3(z.v, w.v, c);
    for(long s = -2; s <= 2; s++){
        Q r = star(z, w, s);
        printf("      %+-8ld %+-17ld (%+ld, %+ld, %+ld)\n", s, r.s0,
               r.v[0], r.v[1], r.v[2]);
        if(primeiro){ esc0 = r.s0; primeiro = 0; }
        else if(r.s0 != esc0) mal++;
        for(int j = 0; j < 3; j++)
            if(r.v[j] != base.v[j] + s*c[j]) mal++;
    }
    printf("\n      a×b = (%+ld, %+ld, %+ld)\n\n", c[0], c[1], c[2]);
    ok("variar s move APENAS o termo do cruzado — as outras três peças não o veem",
       mal == 0);
    printf("      É a mesma partição do rn.c e do multiplicacao.tex, agora com um botão. E o\n");
    printf("      botão está onde tinha de estar: no cruzado, que é onde vive a recursão e onde\n");
    printf("      mora a não-comutatividade. Mexer no s é mexer na ORDEM, não na medida.\n");
}

printf("\n§H2  A LEI DE CONSERVAÇÃO, e o imposto V(s) = (1−s²)·m.\n\n");
{
    printf("      ‖z ⋆_s w‖² + (1−s²)·‖a×b‖² = ‖z‖²·‖w‖²        (a lei)\n");
    printf("      V(s) = (1−s²)·m(a,b),  com m = ‖a×b‖²          (o imposto)\n\n");
    printf("      s        ‖z⋆w‖²        V(s)          soma          ‖z‖²‖w‖²\n");
    Q z0 = { 7, { 3, -5, 2 } }, w0 = { -4, { 6, 1, -3 } };
    int malS = 0;
    for(long s = -3; s <= 3; s++){
        Q r = star(z0, w0, s);
        long lhs = n2q(r) + imposto(z0,w0,s);
        long rhs = n2q(z0) * n2q(w0);
        printf("      %+-8ld %-13ld %-13ld %-13ld %-13ld\n",
               s, n2q(r), imposto(z0,w0,s), lhs, rhs);
        if(lhs != rhs) malS++;
    }
    long leiZ = 0, leiT = 0;
    for(long s = -3; s <= 3; s++)
    for(long z0s = -2; z0s <= 2; z0s++) for(long z1 = -2; z1 <= 2; z1++)
    for(long w0s = -2; w0s <= 2; w0s++) for(long w1 = -2; w1 <= 2; w1++){
        long zv[3] = {z0s, z1, 1}, wv[3] = {w0s, w1, -1};
        Q z = { z0s + 1, { zv[0], zv[1], zv[2] } };
        Q w = { w1 - 1,  { wv[0], wv[1], wv[2] } };
        Q r = star(z, w, s);
        leiT++;
        if(n2q(r) + imposto(z,w,s) == n2q(z)*n2q(w)) leiZ++;
    }
    printf("\n      e a MESMA lei em ℤ, varridos z, w e s: %ld de %ld com resíduo ZERO\n\n",
           leiZ, leiT);
    ok("a lei de conservação fecha para TODO s — a norma que falta É o imposto. E ela é"
       " ARITMÉTICA: com z, w e s inteiros os dois lados são o mesmo inteiro",
       malS == 0 && leiZ == leiT && leiT > 0);

    int malL = 0;
    long lagZ = 0, lagT = 0;
    for(long a0 = -3; a0 <= 3; a0++) for(long a1 = -3; a1 <= 3; a1++) for(long a2 = -3; a2 <= 3; a2++)
    for(long b0 = -3; b0 <= 3; b0++) for(long b1 = -3; b1 <= 3; b1++) for(long b2 = -3; b2 <= 3; b2++){
        long A[3] = {a0,a1,a2}, B[3] = {b0,b1,b2};
        lagT++;
        if(rt_lagrange(A, B, 3)) lagZ++;
    }
    if(lagZ != lagT) malL++;
    printf("      e a identidade de Lagrange, de onde a lei sai: ‖a×b‖² = ‖a‖²‖b‖² − ⟨a,b⟩²\n");
    printf("      em ℤ³, varridos %ld pares: %ld com resíduo ZERO (rt_lagrange)\n\n", lagT, lagZ);
    ok("Lagrange fecha — e é ela que separa o que a norma guarda do que ela paga", malL == 0);
    printf("      A leitura é esta: a norma do produto NÃO é o produto das normas, em geral. O\n");
    printf("      que falta é exatamente V(s), e V é uma MEDIDA — dá para lê-la e agir sobre\n");
    printf("      ela. É o que o §H7 vai fazer com as bandas de histerese.\n");
}

printf("\n§H3  s = ±1 é onde o imposto ANULA — e é o Hurwitz outra vez.\n\n");
{
    printf("      V(s) = (1−s²)·m   =>   V = 0  ⟺  s = ±1  (ou m = 0)\n\n");
    printf("      s        1 − s²     imposto V     ‖z⋆w‖² = ‖z‖²‖w‖² ?\n");
    int mal = 0;
    Q z = { 7, { 3, -5, 2 } }, w = { -4, { 6, 1, -3 } };
    long nz2nw2 = n2q(z)*n2q(w);
    for(long s = -3; s <= 3; s++){
        Q r = star(z, w, s);
        long V = imposto(z,w,s);
        int mult = (n2q(r) == nz2nw2);
        int devia = (s == 1 || s == -1);
        printf("      %+-8ld %+-10ld %-13ld %s\n", s, 1-s*s, V,
               mult ? "SIM — multiplicativa" : "não");
        if(mult != devia) mal++;
        if((V == 0) != devia) mal++;           /* m ≠ 0 neste par, logo V=0 sse s=±1 */
    }
    printf("\n");
    ok("a norma é multiplicativa EXATAMENTE em s = ±1 — onde o imposto anula", mal == 0);
    printf("      E é o Hurwitz, dito de outra maneira: em s = +1 a álgebra é a dos quatérnios,\n");
    printf("      e em s = -1 é a dos quatérnios com o cruzado invertido (a mesma, com a mão\n");
    printf("      trocada). Fora desses dois pontos a norma vaza, e vaza uma quantidade que se\n");
    printf("      pode escrever: (1−s²)·‖a×b‖².\n");
    printf("\n      Repare-se no que isto acrescenta ao que já estava medido. O nne.c dizia que\n");
    printf("      só há álgebras normadas em 1, 2, 4, 8 — uma classificação. Aqui há uma FAMÍLIA\n");
    printf("      contínua, e a classificação passa a ser o CONJUNTO DE ZEROS de uma função. Não\n");
    printf("      é uma lista: é onde V se anula.\n");
}

printf("\n§H4  Os campos locais C_â: fechados, e R^n é a sua união.\n\n");
{
    printf("      C_â = {α + β·â}   é fechado por ⋆_s, e lá o imposto é ZERO\n\n");
    int malF = 0, malC = 0, malV = 0, nc = 0;
    printf("      â            |â|²   α₁+β₁â ⋆ α₂+β₂â   lei (α₁α₂−β₁β₂|â|²) + (α₁β₂+β₁α₂)â\n");
    long As[5][3] = { {1,0,0}, {0,1,0}, {0,0,1}, {1,1,0}, {1,1,1} };
    long a1 = 6, b1 = -8, a2 = 2, b2 = 9;
    for(int c = 0; c < 5; c++){
        long ah[3] = { As[c][0], As[c][1], As[c][2] };
        long na2 = n2v(ah);
        long s = c - 2;                            /* qualquer s: o fecho não depende dele */
        Q z = { a1, { b1*ah[0], b1*ah[1], b1*ah[2] } };
        Q w = { a2, { b2*ah[0], b2*ah[1], b2*ah[2] } };
        Q r = star(z, w, s);
        long eR = a1*a2 - b1*b2*na2, eI = a1*b2 + b1*a2;
        long cruz_r[3]; rt_cruz3(r.v, ah, cruz_r);
        nc++;
        if(n2v(cruz_r) != 0) malF++;               /* r.v ∥ â */
        if(r.s0 != eR) malC++;
        for(int k = 0; k < 3; k++) if(r.v[k] != eI*ah[k]) malC++;
        if(imposto(z,w,s) != 0) malV++;
        printf("      (%+ld,%+ld,%+ld)   %-4ld  %ld + %ld·â            %s\n",
               ah[0], ah[1], ah[2], na2, r.s0, eI,
               (r.s0==eR && n2v(cruz_r)==0) ? "sim" : "NÃO");
    }
    printf("\n      fecho (a parte vetorial fica paralela a â): %d falhas em %d\n", malF, nc);
    printf("      e a lei é a de C (com |â|²)                 : %d falhas\n", malC);
    printf("      e o imposto lá dentro é ZERO                : %d falhas\n\n", malV);
    ok("cada C_â é fechado, isomorfo a C, e o imposto lá é ZERO — nada vaza no campo local",
       malF == 0 && malC == 0 && malV == 0 && nc == 5);
    printf("      E é isto que faz de R^n uma UNIÃO de campos locais, e não um espaço uniforme.\n");
    printf("      Dentro de um C_â a álgebra é C — comuta, associa, e a norma é multiplicativa.\n");
    printf("      O imposto só aparece quando se ATRAVESSA de um campo para outro, porque aí\n");
    printf("      a×b já não é zero. É uma carta de navegação: o preço está nas fronteiras.\n");
}

printf("\n§H5  O TORQUE VETORIAL: T⃗_e(s), e em n=4 tem TRÊS componentes.\n\n");
{
    printf("      T⃗_e(s) = (3/2)P·(ψ₀·i − i₀·ψ + s·ψ×i)     P=2 ⇒ factor 3, inteiro\n\n");
    long P = 2;
    Q psi = { 0, { 1, 0, 0 } }, i = { 0, { 0, 1, 0 } };
    printf("      caso                        T⃗_e = (Tx, Ty, Tz)\n");
    long T[3];
    torque(psi, i, 1, P, T);
    printf("      ψ=(0;1,0,0), i=(0;0,1,0)    (%+ld, %+ld, %+ld)\n", T[0],T[1],T[2]);
    int mal = 0, malC = 0;
    {
        Q p2 = { 0, { 8, 6, 0 } }, i2 = { 0, { -3, 9, 0 } };
        long T2[3], Tc[3];
        torque(p2, i2, 1, P, T2);
        torque_compacta(p2, i2, 1, P, Tc);
        long classico = (3*P/2)*(p2.v[0]*i2.v[1] - p2.v[1]*i2.v[0]);
        printf("      ψ,i no plano xy             (%+ld, %+ld, %+ld)   [forma EXPANDIDA]\n",
               T2[0],T2[1],T2[2]);
        printf("      a mesma, forma compacta     (%+ld, %+ld, %+ld)   [Im(conj(ψ)⋆i)]\n",
               Tc[0],Tc[1],Tc[2]);
        printf("      e o DTC clássico dá         %+ld\n", classico);
        if(T2[2] != classico) mal++;
        if(T2[0] != 0 || T2[1] != 0) mal++;
        if(Tc[2] != -classico) malC++;
    }
    printf("\n");
    ok("a forma COMPACTA do paper dá o simétrico da expandida — as duas diferem no sinal do cruzado",
       malC == 0);
    ok("com ψ e i no plano, o torque vetorial dá SÓ a componente z — e é o clássico",
       mal == 0);
    int vivas = 0;
    {
        Q p3 = { 0, { 5, 3, 8 } }, i3 = { 0, { -2, 7, 4 } };
        long T3[3];
        torque(p3, i3, 1, P, T3);
        printf("      e fora do plano:            (%+ld, %+ld, %+ld)\n",
               T3[0],T3[1],T3[2]);
        for(int k = 0; k < 3; k++) if(T3[k] != 0) vivas++;
    }
    printf("\n");
    ok("fora do plano as TRÊS componentes vivem — uma por eixo de SO(3), o motor esférico",
       vivas == 3);
    printf("      É a generalização inteira numa linha: o DTC clássico controla UM número, e\n");
    printf("      este controla um VETOR. Em n = 2 o vetor tem uma componente e recai no de\n");
    printf("      Takahashi; em n = 4 tem três, e são os três eixos de rotação de um motor\n");
    printf("      esférico. A estrutura não mudou — mudou a dimensão do que ela carrega.\n");
}

printf("\n§H6  A histerese ESFÉRICA, e a redução ao clássico em n = 2.\n\n");
{
    printf("      σ = 0 se ‖T⃗ − T⃗*‖² ≤ d²;  senão σ ∥ (T⃗* − T⃗)     (a banda esférica)\n\n");
    long d2 = 100, Tref[3] = { 30, 0, 0 };          /* d=10, comparado em quadrados */
    printf("      T⃗ atual                ‖erro‖²   σ (direção)                 dentro?\n");
    int mal = 0;
    long Ts[5][3] = {
        { 30,  0,  0 },                             /* erro 0 */
        { 22,  5, -3 },                             /* 8²+5²+3² = 98 ≤ 100 */
        { 14, 10, -6 },                             /* 16²+10²+6² = 392 > 100 */
        {  6, 15, -9 },
        { -2, 20,-12 }
    };
    for(int c = 0; c < 5; c++){
        long e[3] = { Tref[0]-Ts[c][0], Tref[1]-Ts[c][1], Tref[2]-Ts[c][2] };
        long ne2 = n2v(e);
        long sg[3] = {0,0,0};
        if(ne2 > d2){ sg[0]=e[0]; sg[1]=e[1]; sg[2]=e[2]; }
        long cr[3]; rt_cruz3(sg, e, cr);
        printf("      (%+ld,%+ld,%+ld)      %-8ld (%+ld,%+ld,%+ld)           %s\n",
               Ts[c][0],Ts[c][1],Ts[c][2], ne2, sg[0],sg[1],sg[2],
               ne2 <= d2 ? "sim" : "não");
        if(ne2 > d2){
            if(n2v(cr) != 0 || n2v(sg) == 0) mal++; /* fora: paralelo e não nulo */
            if(rt_dir(sg, e, 3) <= 0) mal++;        /* o mesmo sentido */
        }
        if(ne2 <= d2 && n2v(sg) != 0) mal++;
    }
    printf("\n");
    ok("a banda esférica dá σ = 0 dentro e a DIRECÇÃO do erro fora — a correção, sem versor",
       mal == 0);
    int malR = 0;
    printf("      e a redução a n = 2 (tudo numa componente só):\n\n");
    printf("      T (escalar)   erro      σ hipercomplexo   σ clássico (sinal)\n");
    long d = 10, ref = 30;
    long Tes[5] = { 5, 16, 27, 38, 49 };
    for(int c = 0; c < 5; c++){
        long Tv = Tes[c], e = ref - Tv, ne = e < 0 ? -e : e;
        long sg = (ne > d) ? (e > 0 ? 1 : -1) : 0;
        int cl = (Tv < ref - d) ? +1 : (Tv > ref + d) ? -1 : 0;
        printf("      %-13ld %+-9ld %+-17ld %+d\n", Tv, e, sg, cl);
        if(sg != cl) malR++;
    }
    printf("\n");
    ok("em uma componente a banda esférica É o comparador de sinal de Takahashi", malR == 0);
    printf("      A redução não é aproximada — é literal. Numa dimensão, dir(x) é sign(x), e a\n");
    printf("      esfera de raio d é o intervalo [−d, d]. O clássico não é um caso parecido: é\n");
    printf("      o mesmo objeto com uma componente.\n");
}

printf("\n§H7  As BANDAS derivadas do imposto — e a economia, medida.\n\n");
{
    printf("      d² ∝ m / (‖z‖²‖w‖²) = b²/|i|²     — a banda sai da LEI, não do dedo\n");
    printf("      e a monotonia de d É a de d². Compara-se em cruzado, sem raiz.\n\n");
    printf("      (a,b) de i=(a,b,0)   m=b²    |i|²    d² ∝ b²/|i|²     cresce?\n");
    int malMono = 0, nc = 0;
    long ab[7][2] = { {6,0},{5,1},{4,2},{3,3},{2,4},{1,5},{0,6} };
    long ant_num = -1, ant_den = 1;
    for(int c = 0; c < 7; c++){
        long a = ab[c][0], b = ab[c][1];
        Q z = { 0, { 1, 0, 0 } };
        Q w = { 0, { a, b, 0 } };
        long m = massa(z,w);
        long nw2 = n2q(w);
        /* d² ∝ m / nw²  (‖z‖²=1). Cruzado contra o anterior. */
        int cresce = 1;
        if(ant_num >= 0){
            /* b²/nw2  >  ant_num/ant_den  ⇔  b²·ant_den > ant_num·nw2 */
            if((long long)m * ant_den < (long long)ant_num * nw2) cresce = 0;
            if(!cresce) malMono++;
        }
        printf("      (%ld,%ld)               %-6ld %-6ld  %ld/%ld              %s\n",
               a, b, m, nw2, m, nw2, c==0 ? "—" : (cresce ? "sim" : "NÃO"));
        ant_num = m; ant_den = nw2; nc++;
    }
    printf("\n");
    ok("a banda sai da lei e cresce com o imposto — perto do campo local ela FECHA",
       malMono == 0 && nc == 7);
    printf("      E a leitura física é direta. Com ψ e i quase PARALELOS estamos quase dentro de\n");
    printf("      um campo local C_â: a álgebra é quase C, o imposto é quase zero, e não há\n");
    printf("      dissipação em curvatura para compensar — a banda pode ser estreita porque\n");
    printf("      pouco se perde. Com eles PERPENDICULARES o imposto é máximo, e a banda alarga\n");
    printf("      para não chavear a toda a hora.\n");
    printf("\n      Em n = 2 a massa m é constante para |ψ| e |i| fixos, logo V é constante e a\n");
    printf("      banda também — e recai nos d_ψ, d_T fixos de Takahashi 1986. O clássico é o\n");
    printf("      caso de banda derivada onde a derivada é zero.\n");
}

printf("\n§H8  VALIDAR: o DTC hipercomplexo roda, e em n=2 dá o de Takahashi.\n\n");
{
    /* Histerese em ℤ: T persegue Tref, corrige por `passo` quando ‖e‖² > d².
     * A lei é o ripple ENCOLHER com o passo — o ciclo limite da histerese. */
    long Tref[3] = { 40, 20, -15 };
    long d2 = 25;                                  /* d = 5 */
    const long PASSO[5] = { 16, 8, 4, 2, 1 };
    printf("      alvo T⃗* = (40, 20, −15),  banda d² = %ld\n\n", d2);
    printf("      passo de correção   ‖erro‖² no ciclo (último quarto)\n");
    long primeiro = -1, ultimo = -1;
    for(int c = 0; c < 5; c++){
        long Tv[3] = { 0, 0, 0 };
        long pior = 0;
        int N = 400;
        for(int k = 0; k < N; k++){
            long e[3] = { Tref[0]-Tv[0], Tref[1]-Tv[1], Tref[2]-Tv[2] };
            long ne2 = n2v(e);
            if(k > N*3/4 && ne2 > pior) pior = ne2;
            if(ne2 > d2){
                for(int j = 0; j < 3; j++){
                    if(e[j] > 0) Tv[j] += PASSO[c];
                    else if(e[j] < 0) Tv[j] -= PASSO[c];
                }
            }
        }
        printf("      %-19ld %ld\n", PASSO[c], pior);
        if(c == 0) primeiro = pior;
        ultimo = pior;
    }
    printf("\n      do maior passo ao menor: o ciclo caiu de %ld para %ld\n\n",
           primeiro, ultimo);
    ok("o erro entra em CICLO LIMITE, e o ripple encolhe com o passo — é a lei da histerese."
       " Do passo 16 ao 1 o ciclo cai de 433 para 25, e o 25 é a própria banda d²",
       primeiro == 433 && ultimo == 25);
    printf("      É o RIPPLE DE TORQUE, e ele não é um defeito da simulação: é o que o DTC faz.\n");
    printf("      O controlo só corrige quando já saiu da banda, logo o erro passeia sempre um\n");
    printf("      pouco para fora dela — e o quanto depende do passo, que no ferro é o período\n");
    printf("      de amostragem e o degrau de tensão do inversor.\n");
    printf("\n      E é exatamente por aqui que o MULTINÍVEL entra (motor.c §M6): mais níveis\n");
    printf("      são degraus de tensão menores, logo passo menor, logo ripple menor — sem\n");
    printf("      apertar a banda nem acelerar o chaveamento. A mesma conta, do outro lado.\n\n");

    int mal2 = 0, n2 = 0;
    Q p2 = { 0, { 8, 6, 0 } };
    long P = 2;
    for(long k = -12; k <= 12; k++){
        Q i2 = { 0, { k, 1, 0 } };
        long Tv[3];
        torque(p2, i2, 1, P, Tv);
        long classico = (3*P/2)*(p2.v[0]*i2.v[1] - p2.v[1]*i2.v[0]);
        n2++;
        if(Tv[2] != classico) mal2++;
        if(Tv[0] != 0 || Tv[1] != 0) mal2++;
    }
    printf("      e restrito ao plano, contra o DTC clássico, em %d pontos da grelha: %d falhas\n\n",
           n2, mal2);
    ok("restrito ao plano, o hipercomplexo É o de Takahashi — resíduo 0 na grelha inteira",
       mal2 == 0 && n2 == 25);
    printf("      E é isto que a generalização tem de provar para valer alguma coisa: que ela\n");
    printf("      CONTÉM o caso conhecido, sem aproximação. O paper diz \"não substitui o DTC\n");
    printf("      clássico; o especializa como caso n = 2, s = 1, M = 1\" — e é essa a linha\n");
    printf("      medida acima, com resíduo 0.\n");
    printf("\n      O que a família acrescenta, então, é um botão (o s) num lugar que já\n");
    printf("      existia (o cruzado), e uma grandeza nova que se pode LER (o imposto V). O\n");
    printf("      resto — as quatro peças, a partição simétrico/antissimétrico, o torque como\n");
    printf("      cruzado — estava tudo lá desde o rn.c.\n");
}

printf("\n");
printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
return falhas ? 1 : 0;
}
