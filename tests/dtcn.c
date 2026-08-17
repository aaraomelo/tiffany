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
 *   §H1  a família ⋆_s: as quatro peças, e só a quarta vê o s
 *   §H2  a LEI DE CONSERVAÇÃO, e o imposto V(s) = (1−s²)·m
 *   §H3  s = ±1 é onde o imposto anula — e é o Hurwitz outra vez
 *   §H4  os campos locais C_â: fechados, isomorfos a C, e R^n é a sua união
 *   §H5  o TORQUE VETORIAL: T⃗_e(s), e em n=4, s=1 tem TRÊS componentes
 *   §H6  a histerese ESFÉRICA, e a redução ao clássico em n = 2
 *   §H7  as BANDAS derivadas do imposto — e a economia, medida
 *   §H8  VALIDAR: o DTC hipercomplexo roda, e em n=2 dá o de Takahashi
 *
 *   cc -O2 -std=c99 dtcn.c -lm -o dtcn && ./dtcn
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "reta.h"      /* rt_cruz3, rt_dir: Lagrange em ℤ */
#include "unidade.h"

/* R^4 = escalar + vetor de R³ — onde o cruzado existe (n ∈ {1,3,7}) */
typedef struct { double s0, v[3]; } Q;

static void cruz(const double *a, const double *b, double *r){
    r[0] = a[1]*b[2] - a[2]*b[1];
    r[1] = a[2]*b[0] - a[0]*b[2];
    r[2] = a[0]*b[1] - a[1]*b[0];
}
static double ip3(const double *a, const double *b){
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}
static double n3(const double *a){ return sqrt(ip3(a,a)); }
static double nq(Q z){ return sqrt(z.s0*z.s0 + ip3(z.v,z.v)); }

/* A FAMÍLIA ⋆_s — e só a quarta peça vê o s */
static Q star(Q z, Q w, double s){
    Q r;
    double c[3];
    cruz(z.v, w.v, c);
    r.s0 = z.s0*w.s0 - ip3(z.v, w.v);                 /* peça 1 e 2: não veem o s */
    for(int k = 0; k < 3; k++)
        r.v[k] = z.s0*w.v[k] + w.s0*z.v[k]            /* peça 3: também não */
               + s*c[k];                              /* peça 4: É o cruzado, e vê o s */
    return r;
}
static Q conjq(Q z){ Q r = { z.s0, { -z.v[0], -z.v[1], -z.v[2] } }; return r; }

/* a massa algébrica e o imposto */
static double massa(Q z, Q w){
    double c[3];
    cruz(z.v, w.v, c);
    return ip3(c,c);
}
static double imposto(Q z, Q w, double s){ return (1 - s*s)*massa(z,w); }

/* O TORQUE VETORIAL — e aqui há uma DISCREPÂNCIA NO PAPER que vale reportar.
 *
 * O paper escreve a forma compacta e a expandida como iguais:
 *     T⃗_e(s) := (3/2)P·Im(conj(ψ) ⋆_s i) = (3/2)P·(ψ₀·i − i₀·ψ + s·ψ×i)
 *
 * Mas expandindo Im(conj(ψ) ⋆_s i) com a definição de ⋆_s do próprio paper:
 *     conj(ψ) ⋆_s i = (ψ₀i₀ + ⟨ψ,i⟩) + (ψ₀·i − i₀·ψ − s·ψ×i)
 * o termo do cruzado sai com MENOS, porque conj troca o sinal de ψ e o cruzado é
 * antissimétrico. As duas formas diferem no sinal da quarta peça.
 *
 * E é a EXPANDIDA que reduz ao DTC clássico (T_e = (3/2)P(ψ_d i_q − ψ_q i_d), positivo) —
 * que é o requisito que o próprio paper impõe: "não substitui o DTC clássico; o especializa
 * como caso n = 2". Logo é a expandida que se implementa, e o §H5 mede as duas lado a lado. */
static void torque(Q psi, Q i, double s, double P, double *T){
    double c[3];
    cruz(psi.v, i.v, c);
    for(int k = 0; k < 3; k++)
        T[k] = 1.5*P*(psi.s0*i.v[k] - i.s0*psi.v[k] + s*c[k]);
}
/* e a forma compacta, para as medir lado a lado */
static void torque_compacta(Q psi, Q i, double s, double P, double *T){
    Q t = star(conjq(psi), i, s);
    for(int k = 0; k < 3; k++) T[k] = 1.5*P*t.v[k];
}

int main(void){
printf("\n=== O DTC HIPERCOMPLEXO: A FAMÍLIA ⋆_s EM R^n ============================\n");
printf("    z ⋆_s w = (a₀b₀ − ⟨a,b⟩) + (a₀b + b₀a + s·a×b)\n");
printf("    Três peças não veem o s. A quarta — o cruzado — é a que ele parametriza.\n");

printf("\n§H1  A família ⋆_s: as quatro peças, e só a quarta vê o s.\n\n");
{
    /* Mede-se que variar s NAO mexe nas tres primeiras pecas: a parte escalar e a parte
     * (a₀b + b₀a) sao as mesmas para todo s, e so' o termo do cruzado se move. */
    Q z = { 0.7, { 0.3, -0.5, 0.2 } }, w = { -0.4, { 0.6, 0.1, -0.3 } };
    printf("      s        parte escalar     parte vetorial (d, q, r)\n");
    int mal = 0;
    double esc0 = 0; int primeiro = 1;
    Q base = star(z, w, 0.0);                          /* s = 0: sem cruzado nenhum */
    double c[3]; cruz(z.v, w.v, c);
    for(int k = 0; k <= 4; k++){
        double s = -1.0 + 0.5*k;
        Q r = star(z, w, s);
        printf("      %+-8.1f %+-17.9f (%+.6f, %+.6f, %+.6f)\n", s, r.s0,
               r.v[0], r.v[1], r.v[2]);
        if(primeiro){ esc0 = r.s0; primeiro = 0; }
        else if(fabs(r.s0 - esc0) != 0.0) mal++;      /* a escalar NÃO se move */
        for(int j = 0; j < 3; j++)                     /* e a vetorial move-se exatamente s·c */
            if(fabs(r.v[j] - (base.v[j] + s*c[j])) != 0.0) mal++;
    }
    printf("\n      a×b = (%+.6f, %+.6f, %+.6f)\n\n", c[0], c[1], c[2]);
    ok("variar s move APENAS o termo do cruzado — as outras três peças não o veem",
       mal == 0);
    printf("      É a mesma partição do rn.c e do multiplicacao.tex, agora com um botão. E o\n");
    printf("      botão está onde tinha de estar: no cruzado, que é onde vive a recursão e onde\n");
    printf("      mora a não-comutatividade. Mexer no s é mexer na ORDEM, não na medida.\n");
}

printf("\n§H2  A LEI DE CONSERVAÇÃO, e o imposto V(s) = (1−s²)·m.\n\n");
{
    /* ‖z⋆_s w‖² + (1−s²)‖a×b‖² = ‖z‖²‖w‖².  Sai de Lagrange:
     * ‖a×b‖² = ‖a‖²‖b‖² − ⟨a,b⟩², e o cruzado é perpendicular aos dois. */
    printf("      ‖z ⋆_s w‖² + (1−s²)·‖a×b‖² = ‖z‖²·‖w‖²        (a lei)\n");
    printf("      V(s) = (1−s²)·m(a,b),  com m = ‖a×b‖²          (o imposto)\n\n");
    int mal = 0, casos = 0;
    printf("      s        ‖z⋆w‖²        V(s)          soma          ‖z‖²‖w‖²      resíduo\n");
    for(int k = 0; k <= 6; k++){
        double s = -1.5 + 0.5*k;
        Q z = { 0.7, { 0.3, -0.5, 0.2 } }, w = { -0.4, { 0.6, 0.1, -0.3 } };
        Q r = star(z, w, s);
        double lhs = nq(r)*nq(r) + imposto(z,w,s);
        double rhs = nq(z)*nq(z) * nq(w)*nq(w);
        printf("      %+-8.1f %-13.9f %-13.9f %-13.9f %-13.9f %.1e\n",
               s, nq(r)*nq(r), imposto(z,w,s), lhs, rhs, fabs(lhs-rhs));
        if((long long)(fabs(lhs-rhs) * 1e12) >= 1) mal++;
    }
    /* e em massa, com z e w quaisquer */
    for(int k = 0; k < 500; k++){
        double s = -2.0 + 0.008*k;
        Q z = { sin(3.0*k+1), { cos(5.0*k), sin(7.0*k+2), cos(11.0*k+1) } };
        Q w = { cos(13.0*k), { sin(17.0*k+1), cos(19.0*k), sin(23.0*k+3) } };
        Q r = star(z, w, s);
        double lhs = nq(r)*nq(r) + imposto(z,w,s), rhs = nq(z)*nq(z)*nq(w)*nq(w);
        casos++;
        if((long long)(fabs(lhs-rhs) / (fabs(rhs)+1) * 1e11) >= 1) mal++;
    }
    printf("\n      (mais %d casos com z, w e s quaisquer)\n\n", casos);
    /* E A LEI TAMBÉM É EXACTA. ‖z⋆w‖² + (1−s²)‖a×b‖² = ‖z‖²‖w‖² é aritmética: com z, w
     * INTEIROS e s inteiro, as quatro peças do star são somas e produtos, e os dois lados
     * são o MESMO inteiro. O 1e-12 e o 1e-11 relativo acima medem os sin/cos que geram os
     * vectores, não a lei. Aqui não há o que tolerar. */
    long leiZ = 0, leiT = 0;
    for(long s = -3; s <= 3; s++)
    for(long z0 = -2; z0 <= 2; z0++) for(long z1 = -2; z1 <= 2; z1++)
    for(long w0 = -2; w0 <= 2; w0++) for(long w1 = -2; w1 <= 2; w1++){
        long zv[3] = {z0, z1, 1}, wv[3] = {w0, w1, -1}, c[3];
        long zs = z0 + 1, ws = w1 - 1;                    /* as escalares */
        rt_cruz3(zv, wv, c);
        long rs = zs*ws - rt_dir(zv, wv, 3);              /* a parte escalar de z⋆w */
        long rv[3];
        for(int k = 0; k < 3; k++) rv[k] = zs*wv[k] + ws*zv[k] + s*c[k];
        long nr = rs*rs + rt_dir(rv, rv, 3);              /* ‖z⋆w‖² */
        long imp = (1 - s*s) * rt_dir(c, c, 3);           /* o imposto */
        long nz = zs*zs + rt_dir(zv, zv, 3), nw = ws*ws + rt_dir(wv, wv, 3);
        leiT++;
        if(nr + imp == nz*nw) leiZ++;
    }
    printf("      e a MESMA lei em ℤ, com z, w e s inteiros: %ld de %ld com resíduo ZERO\n\n",
           leiZ, leiT);
    ok("a lei de conservação fecha para TODO s — a norma que falta É o imposto. E ela é"
       " ARITMÉTICA: com z, w e s inteiros os dois lados são o mesmo inteiro, sem um"
       " limiar — o 1e-12 media os sin/cos que geram os vectores, não a lei",
       mal == 0 && leiZ == leiT && leiT > 0);
    /* e a identidade de Lagrange, que é de onde a lei sai.
     *
     * ELA É EXACTA EM ℤ, e media-se com vectores gerados por sin/cos contra um 1e-12 —
     * o limiar do arredondamento, não a régua da identidade. ‖a×b‖² = ‖a‖²‖b‖² − ⟨a,b⟩²
     * é aritmética: em ℤ³ os dois lados são inteiros e a diferença é ZERO, não pequena.
     * A peça é rt_lagrange, de lib/reta.h, que já faz esta conta com rt_cruz3 e rt_dir. */
    int malL = 0;
    long lagZ = 0, lagT = 0;
    for(long a0 = -3; a0 <= 3; a0++) for(long a1 = -3; a1 <= 3; a1++) for(long a2 = -3; a2 <= 3; a2++)
    for(long b0 = -3; b0 <= 3; b0++) for(long b1 = -3; b1 <= 3; b1++) for(long b2 = -3; b2 <= 3; b2++){
        long A[3] = {a0,a1,a2}, B[3] = {b0,b1,b2}, C[3];
        rt_cruz3(A, B, C);
        long esq = rt_dir(C, C, 3);
        long dir = rt_dir(A,A,3)*rt_dir(B,B,3) - rt_dir(A,B,3)*rt_dir(A,B,3);
        lagT++;
        if(esq == dir) lagZ++;                     /* resíduo ZERO, não «== 0.0» */
    }
    if(lagZ != lagT) malL++;
    printf("      e a identidade de Lagrange, de onde a lei sai: ‖a×b‖² = ‖a‖²‖b‖² − ⟨a,b⟩²\n");
    printf("      em ℤ³, varridos %ld pares de vectores inteiros: %ld com resíduo ZERO"
           " (nem um limiar)\n\n", lagT, lagZ);
    ok("Lagrange fecha — e é ela que separa o que a norma guarda do que ela paga", malL == 0);
    printf("      A leitura é esta: a norma do produto NÃO é o produto das normas, em geral. O\n");
    printf("      que falta é exatamente V(s), e V é uma MEDIDA — dá para lê-la e agir sobre\n");
    printf("      ela. É o que o §H7 vai fazer com as bandas de histerese.\n");
}

printf("\n§H3  s = ±1 é onde o imposto ANULA — e é o Hurwitz outra vez.\n\n");
{
    printf("      V(s) = (1−s²)·m   =>   V = 0  ⟺  s = ±1  (ou m = 0)\n\n");
    printf("      s        1 − s²     imposto V     ‖z⋆w‖ = ‖z‖‖w‖ ?\n");
    int mal = 0;
    Q z = { 0.7, { 0.3, -0.5, 0.2 } }, w = { -0.4, { 0.6, 0.1, -0.3 } };
    double nzw = nq(z)*nq(w);
    for(int k = 0; k <= 8; k++){
        double s = -1.5 + 0.375*k;
        Q r = star(z, w, s);
        double V = imposto(z,w,s), dif = fabs(nq(r) - nzw);
        printf("      %+-8.3f %+-10.6f %-13.9f %s\n", s, 1-s*s, V,
               dif == 0.0 ? "SIM — multiplicativa" : "não");
        int devia = fabs(fabs(s) - 1.0) == 0.0;
        if((dif == 0.0) != devia) mal++;
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
    /* Para cada versor â, C_â = {α + β·â} e' fechado por ⋆_s e isomorfo a C. Mede-se as duas
     * coisas: o fecho e a lei complexa. E note-se que ali a×a = 0, logo o imposto e' ZERO —
     * o campo local e' onde a algebra e' associativa e nada vaza. */
    printf("      C_â = {α + β·â}   é fechado por ⋆_s, e lá o imposto é ZERO\n\n");
    int malF = 0, malC = 0, malV = 0;
    printf("      â                     α₁+β₁â × α₂+β₂â   dá (α₁α₂−β₁β₂) + (α₁β₂+β₁α₂)â ?\n");
    for(int c = 0; c < 5; c++){
        double th = c*0.7, ph = c*0.4;
        double ah[3] = { sin(th)*cos(ph), sin(th)*sin(ph), cos(th) };
        double na = n3(ah);
        for(int k = 0; k < 3; k++) ah[k] /= na;
        double s = 0.3 + 0.2*c;                        /* qualquer s: o fecho não depende dele */
        double a1 = 0.6, b1 = -0.8, a2 = 0.2, b2 = 0.9;
        Q z = { a1, { b1*ah[0], b1*ah[1], b1*ah[2] } };
        Q w = { a2, { b2*ah[0], b2*ah[1], b2*ah[2] } };
        Q r = star(z, w, s);
        /* fechado? a parte vetorial tem de ser paralela a â */
        double proj = ip3(r.v, ah), perp = 0;
        for(int k = 0; k < 3; k++){
            double e = r.v[k] - proj*ah[k];
            perp += e*e;
        }
        if((long long)(perp * 1e28) >= 1) malF++;
        /* e é a lei de C? */
        double eR = a1*a2 - b1*b2, eI = a1*b2 + b1*a2;
        if((long long)(fabs(r.s0 - eR) * 1e14) >= 1 || (long long)(fabs(proj - eI) * 1e14) >= 1) malC++;
        if((long long)(fabs(imposto(z,w,s)) * 1e28) >= 1) malV++;       /* e o imposto é ZERO lá */
        if(c < 3)
            printf("      (%+.3f,%+.3f,%+.3f)   %+.6f %+.6f    %s\n",
                   ah[0], ah[1], ah[2], r.s0, proj,
                   (fabs(r.s0-eR) == 0.0 && fabs(proj-eI) == 0.0) ? "sim" : "NÃO");
    }
    printf("\n      fecho (a parte vetorial fica paralela a â): %d falhas\n", malF);
    printf("      e a lei é a de C, exatamente                : %d falhas\n", malC);
    printf("      e o imposto lá dentro é ZERO                : %d falhas\n\n", malV);
    ok("cada C_â é fechado, isomorfo a C, e o imposto lá é ZERO — nada vaza no campo local",
       malF == 0 && malC == 0 && malV == 0);
    printf("      E é isto que faz de R^n uma UNIÃO de campos locais, e não um espaço uniforme.\n");
    printf("      Dentro de um C_â a álgebra é C — comuta, associa, e a norma é multiplicativa.\n");
    printf("      O imposto só aparece quando se ATRAVESSA de um campo para outro, porque aí\n");
    printf("      a×b já não é zero. É uma carta de navegação: o preço está nas fronteiras.\n");
}

printf("\n§H5  O TORQUE VETORIAL: T⃗_e(s), e em n=4 tem TRÊS componentes.\n\n");
{
    /* T⃗_e(s) = (3/2)P·Im(conj(ψ) ⋆_s i) = (3/2)P·(ψ₀·i − i₀·ψ + s·ψ×i).
     * O DTC classico e' a componente z disto, com n=2 (onde ψ e i tem so' uma componente
     * vetorial e o cruzado e' o escalar). Aqui sao TRES — uma por eixo de SO(3). */
    printf("      T⃗_e(s) = (3/2)P·Im(conj(ψ) ⋆_s i) = (3/2)P·(ψ₀·i − i₀·ψ + s·ψ×i)\n\n");
    double P = 2;
    Q psi = { 0.0, { 1.0, 0.0, 0.0 } }, i = { 0.0, { 0.0, 1.0, 0.0 } };
    printf("      caso                        T⃗_e = (Tx, Ty, Tz)          ‖T⃗‖\n");
    double T[3];
    torque(psi, i, 1.0, P, T);
    printf("      ψ=(0;1,0,0), i=(0;0,1,0)    (%+.4f, %+.4f, %+.4f)   %.6f\n",
           T[0],T[1],T[2], n3(T));
    int mal = 0, malC = 0;
    {   /* o caso clássico embutido: tudo no plano xy, e o torque sai em z. E medem-se AS DUAS
         * formas do paper, porque elas diferem — é o achado desta secção. */
        Q p2 = { 0, { 0.8, 0.6, 0 } }, i2 = { 0, { -0.3, 0.9, 0 } };
        double T2[3], Tc[3];
        torque(p2, i2, 1.0, P, T2);
        torque_compacta(p2, i2, 1.0, P, Tc);
        double classico = 1.5*P*(p2.v[0]*i2.v[1] - p2.v[1]*i2.v[0]);
        printf("      ψ,i no plano xy             (%+.4f, %+.4f, %+.4f)   [forma EXPANDIDA]\n",
               T2[0],T2[1],T2[2]);
        printf("      a mesma, forma compacta     (%+.4f, %+.4f, %+.4f)   [Im(conj(ψ)⋆i)]\n",
               Tc[0],Tc[1],Tc[2]);
        printf("      e o DTC clássico dá         %+.6f\n", classico);
        if(fabs(T2[2] - classico) != 0.0) mal++;
        if(fabs(T2[0]) != 0.0 || fabs(T2[1]) != 0.0) mal++;
        if(fabs(Tc[2] + classico) != 0.0) malC++;   /* a compacta dá o SIMÉTRICO */
    }
    printf("\n");
    ok("a forma COMPACTA do paper dá o simétrico da expandida — as duas diferem no sinal do cruzado",
       malC == 0);
    ok("com ψ e i no plano, o torque vetorial dá SÓ a componente z — e é o clássico",
       mal == 0);
    /* e fora do plano: as três componentes vivem */
    int vivas = 0;
    {
        Q p3 = { 0, { 0.5, 0.3, 0.8 } }, i3 = { 0, { -0.2, 0.7, 0.4 } };
        double T3[3];
        torque(p3, i3, 1.0, P, T3);
        printf("      e fora do plano:            (%+.4f, %+.4f, %+.4f)   %.6f\n",
               T3[0],T3[1],T3[2], n3(T3));
        for(int k = 0; k < 3; k++) if(fabs(T3[k]) != 0.0) vivas++;
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
    /* O comparador escalar de Takahashi (acima/abaixo da banda) vira uma banda ESFERICA:
     *   σ = 0            se ‖T⃗ − T⃗*‖ ≤ d
     *   σ = dir(T⃗* − T⃗)  caso contrario
     * Em n=2 (uma componente) isto DEGENERA no comparador de sinal do classico. */
    printf("      σ = 0 se ‖T⃗ − T⃗*‖ ≤ d;  senão σ = dir(T⃗* − T⃗)     (a banda esférica)\n\n");
    double d = 0.1, Tref[3] = { 0.3, 0.0, 0.0 };
    printf("      T⃗ atual                ‖erro‖    σ (direção da correção)      dentro?\n");
    int mal = 0;
    for(int c = 0; c < 5; c++){
        double T[3] = { 0.3 - 0.08*c, 0.05*c, -0.03*c };
        double e[3] = { Tref[0]-T[0], Tref[1]-T[1], Tref[2]-T[2] };
        double ne = n3(e);
        double sg[3] = {0,0,0};
        if(ne > d) for(int k = 0; k < 3; k++) sg[k] = e[k]/ne;
        printf("      (%+.3f,%+.3f,%+.3f)   %-9.6f (%+.3f,%+.3f,%+.3f)      %s\n",
               T[0],T[1],T[2], ne, sg[0],sg[1],sg[2], ne <= d ? "sim" : "não");
        if(ne > d && (long long)(fabs(n3(sg) - 1.0) * 1e12) >= 1) mal++;       /* σ é versor */
        if(ne <= d && n3(sg) != 0.0) mal++;                  /* dentro, σ = 0 */
    }
    printf("\n");
    ok("a banda esférica dá σ = 0 dentro e um VERSOR fora — a direção da correção", mal == 0);
    /* e a REDUCAO: com tudo numa componente, σ vira o ±1 do classico */
    int malR = 0;
    printf("      e a redução a n = 2 (tudo numa componente só):\n\n");
    printf("      T (escalar)   erro      σ hipercomplexo   σ clássico (sinal)\n");
    for(int c = 0; c < 5; c++){
        double T = 0.05 + 0.11*c, ref = 0.3;
        double e = ref - T, ne = fabs(e);
        double sg = (ne > d) ? e/ne : 0;
        int cl = (T < ref - d) ? +1 : (T > ref + d) ? -1 : 0;
        printf("      %-13.3f %+-9.3f %+-17.0f %+d\n", T, e, sg, cl);
        if(fabs(sg - cl) != 0.0) malR++;
    }
    printf("\n");
    ok("em uma componente a banda esférica É o comparador de sinal de Takahashi", malR == 0);
    printf("      A redução não é aproximada — é literal. Numa dimensão, dir(x) é sign(x), e a\n");
    printf("      esfera de raio d é o intervalo [−d, d]. O clássico não é um caso parecido: é\n");
    printf("      o mesmo objeto com uma componente.\n");
}

printf("\n§H7  As BANDAS derivadas do imposto — e a economia, medida.\n\n");
{
    /* No classico d_ψ e d_T sao parametros LIVRES, ajustados a mao. Aqui derivam-se da lei:
     *     d(t) = λ·√( V(t) / (‖z‖²‖w‖²) )
     * Imposto alto -> muita dissipacao -> bandas estreitas -> mais chaveamento.
     * Imposto baixo (perto de um campo local) -> bandas largas -> economia.
     * Mede-se a CORRELACAO: quanto mais perto do campo local, mais larga a banda. */
    printf("      d(t) = λ·√( V(t) / (‖z‖²‖w‖²) )       — a banda sai da LEI, não do dedo\n\n");
    double lam = 1.0, s = 0.6;
    printf("      ângulo ψ∠i (°)   m = ‖ψ×i‖²   imposto V    banda d      chaveia a cada\n");
    int malMono = 0; double antD = -1, antAng = -1;
    int nc = 0;
    for(int c = 0; c <= 6; c++){
        double ang = c*M_PI/12;                        /* de 0° (paralelos) a 90° */
        Q z = { 0.0, { 1.0, 0.0, 0.0 } };
        Q w = { 0.0, { cos(ang), sin(ang), 0.0 } };
        double m = massa(z,w), V = imposto(z,w,s);
        /* d² = lam²·V/(nq(z)²·nq(w)²) — e a monotonia de d É a de d², porque x ↦ √x é
         * crescente nos não negativos. A raiz fica para o valor que se imprime e para os
         * passos, que são uma contagem; a comparação que a asserção usa é em d². */
        double d2 = lam*lam*V/(nq(z)*nq(z)*nq(w)*nq(w));
        double d = sqrt(d2);
        /* quantos passos até sair da banda, com uma deriva fixa */
        int passos = (d2 != 0.0) ? (int)(d/0.002) : 999999;
        if(nc < 7) nc++;
        printf("      %-16.0f %-13.6f %-12.6f %-12.6f %d passos\n",
               ang*180/M_PI, m, V, d, passos);
        if(antD >= 0 && ang > antAng && d2 < antD) malMono++;  /* d cresce com o ângulo */
        antD = d2; antAng = ang;
    }
    printf("\n");
    ok("a banda sai da lei e cresce com o imposto — perto do campo local ela FECHA", malMono == 0);
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
    /* Roda-se o controlo VETORIAL: o torque T⃗ persegue T⃗*, com a banda esferica e a correcao
     * na direcao σ. Valida-se por DOIS caminhos: (a) o erro cai e fica dentro da banda; e
     * (b) restringindo tudo a um plano, o percurso é o mesmo que o DTC clássico daria. */
    /* O erro NÃO cai a zero: fica em CICLO LIMITE, e é assim que uma histerese funciona. A
     * amplitude do ciclo é da ordem de (banda + ganho×passo) — e isso É o RIPPLE DE TORQUE do
     * DTC, o problema que o inversor multinível existe para atacar (motor.c §M6).
     *
     * A primeira versão desta asserção exigia "erro final ≤ banda", com um passo de correção
     * grande de mais para a banda que eu tinha pedido: o ciclo limite ficava em 0,0645 contra
     * uma banda de 0,05, e a asserção caía. Outra vez parâmetros que não fecham ENTRE SI — o
     * defeito que acabei de registar na memória. A correção não é escolher melhor os números:
     * é medir a LEI, que é o ripple encolher com o passo. */
    double P = 2, s = 1.0, d = 0.05;
    double Tref[3] = { 0.40, 0.20, -0.15 };
    printf("      alvo T⃗* = (0.40, 0.20, -0.15),  banda d = %.2f\n\n", d);
    printf("      passo de correção   erro final (o ciclo limite)   ripple = erro - banda\n");
    double primeiro = -1, ultimo = -1; int foraDepois = 0;
    for(int c = 0; c < 5; c++){
        double dt = 0.02/pow(2.0, c);
        Q psi = { 0.2, { 0.9, 0.1, 0.0 } }, i = { 0.1, { 0.0, 0.5, 0.2 } };
        double errF = 0, pior = 0;
        int passos = 4000*(1<<c);
        for(int k = 0; k < passos; k++){
            double T[3];
            torque(psi, i, s, P, T);
            double e[3] = { Tref[0]-T[0], Tref[1]-T[1], Tref[2]-T[2] };
            double ne = n3(e);
            if(k > passos*3/4 && ne > pior) pior = ne;
            if(ne > d) for(int j = 0; j < 3; j++) i.v[j] += dt*e[j]/ne*0.5;
            errF = ne;
        }
        printf("      %-19.5f %-29.6f %.6f\n", dt, pior, pior - d);
        /* A lei é o ripple ENCOLHER com o passo, e ela mede-se do primeiro ao último — não
         * ponto a ponto: depois de convergir o valor oscila no ruído numérico, e exigir
         * monotonia estrita seria medir o ruído em vez da lei. */
        if(c == 0) primeiro = pior;
        ultimo = pior;
        if(c > 0 && pior > d*1.02) foraDepois++;       /* e a partir do 2º, cabe na banda */
        (void)errF;
    }
    printf("\n");
    printf("      do maior passo ao menor: o ripple caiu de %.6f para %.6f (%.0f×)\n\n",
           primeiro - d, fabs(ultimo - d), (primeiro-d)/fabs(ultimo-d+1e-12));
    ok("o erro entra em CICLO LIMITE, e o ripple encolhe com o passo — é a lei da histerese",
       primeiro - d > 0.01 && fabs(ultimo - d) < 0.002 && foraDepois == 0);
    printf("      É o RIPPLE DE TORQUE, e ele não é um defeito da simulação: é o que o DTC faz.\n");
    printf("      O controlo só corrige quando já saiu da banda, logo o erro passeia sempre um\n");
    printf("      pouco para fora dela — e o quanto depende do passo, que no ferro é o período\n");
    printf("      de amostragem e o degrau de tensão do inversor.\n");
    printf("\n      E é exatamente por aqui que o MULTINÍVEL entra (motor.c §M6): mais níveis\n");
    printf("      são degraus de tensão menores, logo passo menor, logo ripple menor — sem\n");
    printf("      apertar a banda nem acelerar o chaveamento. A mesma conta, do outro lado.\n\n");

    /* o SEGUNDO caminho: restrito ao plano, tem de dar o clássico */
    int mal2 = 0;
    Q p2 = { 0, { 0.8, 0.6, 0 } };
    for(int k = 0; k < 300; k++){
        Q i2 = { 0, { cos(0.02*k), sin(0.02*k), 0 } };
        double T[3];
        torque(p2, i2, 1.0, P, T);
        double classico = 1.5*P*(p2.v[0]*i2.v[1] - p2.v[1]*i2.v[0]);
        if(fabs(T[2] - classico) != 0.0) mal2++;
        if(fabs(T[0]) != 0.0 || fabs(T[1]) != 0.0) mal2++;
    }
    printf("      e restrito ao plano, contra o DTC clássico, em 300 pontos: %d falhas\n\n", mal2);
    ok("restrito ao plano, o hipercomplexo É o de Takahashi — resíduo 0 em 300 pontos",
       mal2 == 0);
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
return falhas ? 1 : 0;
}
