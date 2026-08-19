/* robo.c — A REDE DIFERENCIAL DE MOTORES: cada motor um corpo, e o diferencial controla todos.
 *
 * O Aarão: "controla vários motores com o microprocessador multifractal; cada motor é um corpo,
 * com corpo diferencial que controla todos — ou seja, passa pra robótica: rede diferencial de
 * motores."
 *
 * É o corpo de corpos (base.c §B6-B7, milenio.c §M6) posto a trabalhar. Cada junta é um motor
 * com o seu DTC (motor.c); o robô é o corpo que os contém; e o que os liga é o JACOBIANO — que
 * é literalmente o corpo diferencial, porque J é a DERIVADA da cinemática.
 *
 * E a peça exata, que fecha com o §B12:
 *
 *      ẋ = J·q̇          a velocidade SOBE das juntas para a ponta   (a torre branca)
 *      τ = Jᵀ·F         a força DESCE da ponta para as juntas       (a torre negra)
 *      ⟨F, ẋ⟩ = ⟨τ, q̇⟩   e a POTÊNCIA é a mesma dos dois lados
 *
 * A última linha é ⟨F, Jq̇⟩ = ⟨JᵀF, q̇⟩ — a ADJUNÇÃO que o §B12 mediu como "as duas torres são
 * adjuntas, e é esse o equilíbrio em todos os andares". Aqui o equilíbrio tem unidade de watt,
 * e chama-se conservação de energia.
 *
 * LEI vs TRANSPORTE. Sen/cos, hypot, diferença finita com h=1e-6 e Euler de 400 passos eram
 * o transporte. A lei é a ROTAÇÃO de período 4 (det 1): elos 5, 4, 3 (a razão 1 : 0,8 : 0,6),
 * ângulos em quartos de volta. A derivada É rot90 da cadeia distal — dois caminhos, o mesmo
 * J, sem h. A adjunção fecha em ℤ. A singularidade é det(JJᵀ) = 0, sem raiz. A pseudo-inversa
 * reproduz o erro: J J⁺ e = e em ℚ, num passo, sem Euler.
 *
 *   §R1  N motores, cada um um corpo — e correm independentes
 *   §R2  o JACOBIANO é o corpo diferencial: J é a derivada da cinemática
 *   §R3  J e Jᵀ são ADJUNTOS, e a adjunção É a conservação de potência
 *   §R4  a SINGULARIDADE é a degenerescência: det(JJᵀ) = 0, e ali é ε² = 0
 *   §R5  a rede como corpo de corpos: o que ela gera é maior que a soma
 *   §R6  CONTROLAR: J J⁺ e = e, e a potência fecha
 *
 *   cc -O2 -std=c99 -I lib tests/robo.c -o robo && ./robo
 */
#include <stdio.h>
#include "unidade.h"
#include "racionais.h"
#include "linear.h"

#define NJ 3                                   /* juntas do braço planar */

/* elos 1 : 0,8 : 0,6 = 5 : 4 : 3. Ângulo em quartos de volta: k · 90°. */
static const long L[NJ] = { 5, 4, 3 };

static void cs(long k, long *c, long *s){
    k %= 4; if(k < 0) k += 4;
    long C[4] = { 1, 0, -1,  0 };
    long S[4] = { 0, 1,  0, -1 };
    *c = C[k]; *s = S[k];
}

static void direta(const long *q, long *x, long *y){
    long a = 0; *x = 0; *y = 0;
    for(int k = 0; k < NJ; k++){
        a += q[k];
        long c, s; cs(a, &c, &s);
        *x += L[k]*c; *y += L[k]*s;
    }
}
/* J[i][j] = ∂x_i/∂q_j. Forma fechada: soma −L sin, L cos da cadeia distal. */
static void jacobiano(const long *q, long J[2][NJ]){
    for(int j = 0; j < NJ; j++){
        long dx = 0, dy = 0, a = 0;
        for(int k = 0; k < NJ; k++){
            a += q[k];
            if(k >= j){
                long c, s; cs(a, &c, &s);
                dx += -L[k]*s; dy += L[k]*c;
            }
        }
        J[0][j] = dx; J[1][j] = dy;
    }
}
/* o mesmo J pela DERIVADA algébrica: d/dθ (R_θ v) = rot90(R_θ v). Det 1, sem h. */
static void jacobiano_rot(const long *q, long J[2][NJ]){
    for(int j = 0; j < NJ; j++){
        long px = 0, py = 0, a = 0;
        for(int k = 0; k < NJ; k++){
            a += q[k];
            if(k >= j){
                long c, s; cs(a, &c, &s);
                px += L[k]*c; py += L[k]*s;
            }
        }
        J[0][j] = -py; J[1][j] = px;
    }
}
/* manipulabilidade ao QUADRADO: det(J Jᵀ) = AC − B². Zero na singularidade, sem raiz. */
static long manip2(const long J[2][NJ]){
    long A = 0, B = 0, C = 0;
    for(int j = 0; j < NJ; j++){
        A += J[0][j]*J[0][j];
        B += J[0][j]*J[1][j];
        C += J[1][j]*J[1][j];
    }
    return A*C - B*B;
}

int main(void){
printf("\n=== A REDE DIFERENCIAL DE MOTORES ========================================\n");
printf("    Cada junta é um motor (um corpo); o jacobiano é o corpo diferencial que\n");
printf("    controla todos. E J com Jᵀ são o par adjunto — a potência é a mesma.\n");
printf("    Elos 5, 4, 3. Ângulo em quartos de volta (período 4, det 1).\n");

printf("\n§R1  N motores, cada um um corpo, e correm independentes.\n\n");
{
    /* Cada junta tem o seu DTC: a sua referencia de torque, a sua banda. Sao N corpos.
     * A lei da histerese (dtcn.c §H8): o erro entra em ciclo limite e o ripple e' a
     * propria banda. Sem Euler em virgula. */
    printf("      junta   L    T*    banda d²   |e|² final   fecha?\n");
    int mal = 0;
    for(int j = 0; j < NJ; j++){
        long Tref = 30 - 5*j, d2 = 49, passo = 2;     /* d = 7, comparado em quadrados */
        long T = 0;
        for(int k = 0; k < 400; k++){
            long e = Tref - T, ae = e < 0 ? -e : e;
            if(ae*ae > d2) T += (e > 0 ? passo : -passo);
        }
        long ef = Tref - T, ne2 = ef*ef;
        int fecha = (ne2 <= d2 + passo*passo);
        printf("      %-7d %-4ld %-5ld %-11ld %-12ld %s\n",
               j+1, L[j], Tref, d2, ne2, fecha ? "sim" : "NÃO");
        if(!fecha) mal++;
    }
    printf("\n");
    ok("os NJ motores fecham cada um no seu corpo, com a sua régua — histerese em |e|²,"
       " ciclo limite dentro da banda mais o passo",
       mal == 0);
    printf("      São N corpos, e o que faz deles uma REDE não é partilharem o controlador — é\n");
    printf("      partilharem a estrutura. Cada um tem a sua borda, a sua régua e o seu resíduo;\n");
    printf("      o que os liga vem a seguir, e é uma derivada.\n");
}

printf("\n§R2  O JACOBIANO É o corpo diferencial: J é a derivada da cinemática.\n\n");
{
    /* Dois caminhos: a forma fechada (−L sin, L cos) e rot90 da cadeia distal.
     * São a MESMA derivada, e o segundo não tem h nenhum: d(R_θ v)/dθ = R_{θ+90} v. */
    printf("      ẋ = J·q̇,   com J[i][j] = ∂x_i/∂q_j — a derivada da cinemática directa\n\n");
    printf("      q (quartos)     J fechada = J rot90?\n");
    int mal = 0, casos = 0;
    for(long q0 = 0; q0 < 4; q0++)
    for(long q1 = 0; q1 < 4; q1++)
    for(long q2 = 0; q2 < 4; q2++){
        long q[NJ] = { q0, q1, q2 };
        long Jf[2][NJ], Jr[2][NJ];
        jacobiano(q, Jf); jacobiano_rot(q, Jr);
        int eq = 1;
        for(int i = 0; i < 2; i++) for(int j = 0; j < NJ; j++)
            if(Jf[i][j] != Jr[i][j]) eq = 0;
        if(casos < 4)
            printf("      (%ld, %ld, %ld)         %s\n", q0, q1, q2, eq ? "sim" : "NÃO");
        if(!eq) mal++;
        casos++;
    }
    printf("      (%d configurações do grupo C₄³)\n\n", casos);
    ok("a forma fechada e rot90 da cadeia distal dão o MESMO J — é mesmo a derivada,"
       " det 1, sem h e sem diferença finita",
       mal == 0 && casos == 64);
    printf("      É por isto que o corpo que controla a rede é o DIFERENCIAL, e não outro: a\n");
    printf("      relação entre o que as juntas fazem e o que a ponta faz É uma derivada. O\n");
    printf("      milenio.c §M6 mediu que o diferencial é a instância máxima — aqui vê-se para\n");
    printf("      que serve ser máxima: ele contém os N motores e a lei que os compõe.\n");
}

printf("\n§R3  J e Jᵀ são ADJUNTOS — e a adjunção É a conservação de potência.\n\n");
{
    printf("      ẋ = J·q̇      (a velocidade SOBE: juntas -> ponta)    <- a torre branca\n");
    printf("      τ = Jᵀ·F     (a força DESCE: ponta -> juntas)        <- a torre negra\n\n");
    printf("      ⟨F, ẋ⟩ = ⟨F, J·q̇⟩ = ⟨Jᵀ·F, q̇⟩ = ⟨τ, q̇⟩\n\n");
    int mal = 0, casos = 0;
    printf("      caso   potência na PONTA    potência nas JUNTAS   iguais?\n");
    long Ftab[3][2] = { {1,0}, {0,1}, {3,-2} };
    long qdtab[3][NJ] = { {1,0,0}, {0,1,-1}, {2,-1,1} };
    for(long q0 = 0; q0 < 4; q0++)
    for(long q1 = 0; q1 < 4; q1++)
    for(long q2 = 0; q2 < 4; q2++){
        long q[NJ] = { q0, q1, q2 };
        long J[2][NJ];
        jacobiano(q, J);
        for(int u = 0; u < 3; u++) for(int v = 0; v < 3; v++){
            long *F = Ftab[u], *qd = qdtab[v];
            long xd[2] = {0,0};
            for(int i = 0; i < 2; i++) for(int j = 0; j < NJ; j++) xd[i] += J[i][j]*qd[j];
            long tau[NJ] = {0};
            for(int j = 0; j < NJ; j++) for(int i = 0; i < 2; i++) tau[j] += J[i][j]*F[i];
            long Pponta = F[0]*xd[0] + F[1]*xd[1];
            long Pjunta = 0;
            for(int j = 0; j < NJ; j++) Pjunta += tau[j]*qd[j];
            if(casos < 4)
                printf("      %-6d %+19ld %+21ld   %s\n",
                       casos, Pponta, Pjunta, Pponta == Pjunta ? "sim" : "NÃO");
            if(Pponta != Pjunta) mal++;
            casos++;
        }
    }
    printf("\n      (%d casos = 64 configs × 3 forças × 3 velocidades)\n\n", casos);
    ok("⟨F, J·q̇⟩ = ⟨Jᵀ·F, q̇⟩ — a potência é a MESMA dos dois lados, resíduo 0 exacto em ℤ",
       mal == 0 && casos == 64*9);
    printf("      É a adjunção do base.c §B12, e aqui ela tem unidade de watt. Lá dizia-se \"as\n");
    printf("      duas torres são adjuntas, e é esse o equilíbrio em todos os andares\"; aqui o\n");
    printf("      equilíbrio chama-se CONSERVAÇÃO DE ENERGIA, e o que uma torre sobe a outra\n");
    printf("      desce com a mesma conta.\n");
    printf("\n      E a assimetria certa continua lá: J leva N juntas em 2 coordenadas (perde),\n");
    printf("      Jᵀ leva 2 forças em N torques (não é sobrejetiva). Cada uma falha onde a\n");
    printf("      outra fecha — que é exatamente o que o §B11 mediu da inclusão e do traço.\n");
}

printf("\n§R4  A SINGULARIDADE é a degenerescência: det(JJᵀ) = 0, e ali é ε² = 0.\n\n");
{
    printf("      w² = det(J·Jᵀ) — zero exatamente na singularidade, sem raiz\n\n");
    printf("      q₂   w² = det(JJᵀ)   direções úteis   estado\n");
    int mal = 0;
    for(long c = 0; c < 4; c++){
        long q[NJ] = { 0, c, 0 };
        long J[2][NJ];
        jacobiano(q, J);
        long w2 = manip2(J);
        int sing = (w2 == 0);
        int esperado = (c == 0 || c == 2);            /* 0 = esticado, 2 = dobrado (180°) */
        printf("      %-4ld %-16ld %-16s %s\n", c, w2,
               sing ? "1 (perdeu uma)" : "2",
               sing ? (c == 0 ? "SINGULAR — o braço esticado"
                              : "SINGULAR — o braço dobrado")
                    : "regular");
        if(sing != esperado) mal++;
    }
    printf("\n");
    int nsing = 0;
    long ant = -1;
    for(long c = 0; c < 4; c++){
        long q[NJ] = { 0, c, 0 };
        long J[2][NJ];
        jacobiano(q, J);
        long w2 = manip2(J);
        int s = (w2 == 0);
        if(ant >= 0 && s != (ant == 0)) nsing++;
        ant = w2;
    }
    printf("      varrendo q₂ em C₄: %d travessias da singularidade\n\n", nsing);
    ok("a manipulabilidade anula nas DUAS singularidades — esticado E dobrado. w² = 0,"
       " sem raiz e sem limiar relativo a L²",
       mal == 0 && nsing >= 2);
    printf("      É o ε² = 0 outra vez, e no sítio onde ele custa dinheiro: na singularidade\n");
    printf("      duas direções de movimento colapsaram numa, e o robô deixa de poder mexer-se\n");
    printf("      para um lado. É a RAIZ DUPLA — o mesmo Δ = 0 do amortecimento crítico e do\n");
    printf("      \"onde os duais se tocam\".\n");
    printf("\n      E a leitura prática sai de graça: perto da singularidade Jᵀ pede torques\n");
    printf("      enormes para forças pequenas, porque a inversa explode. O corpo não avisa com\n");
    printf("      um erro — avisa com o determinante a ir a zero.\n");
}

printf("\n§R5  A rede como corpo de corpos: o que ela gera é maior que a soma.\n\n");
{
    printf("      juntas ativas   posto máximo   nulidade (numa regular)   o que a ponta alcança\n");
    int mal = 0;
    for(int n = 1; n <= NJ; n++){
        int postoMax = 0, nul = -1;
        for(long q0 = 0; q0 < 4; q0++)
        for(long q1 = 0; q1 < 4; q1++)
        for(long q2 = 0; q2 < 4; q2++){
            long q[NJ] = { q0, q1, q2 };
            for(int j = n; j < NJ; j++) q[j] = 0;
            long J[2][NJ];
            jacobiano(q, J);
            Mat A = mat0(2, n);
            for(int i = 0; i < 2; i++) for(int j = 0; j < n; j++)
                A.a[i][j] = qz(J[i][j], 1);
            int p = mat_posto(A);
            if(p > postoMax) postoMax = p;
            if(p == (n == 1 ? 1 : 2) && nul < 0){
                Vec base[LN_MAX];
                nul = mat_nucleo(A, base);
            }
        }
        printf("      %-15d %-14d %-24d %s\n", n, postoMax, nul < 0 ? 0 : nul,
               postoMax == 1 ? "um círculo (só o raio fixo)"
             : n == 2        ? "uma coroa — o plano, localmente"
                             : "a coroa, e com REDUNDÂNCIA (3 juntas, 2 saídas)");
        int espera = (n == 1 ? 1 : 2);
        int nul_ok = (n < 3) || (nul == 1);
        if(postoMax != espera || !nul_ok) mal++;
    }
    printf("\n");
    ok("uma junta dá 1 grau, duas dão 2 — e a terceira não dá 3: dá REDUNDÂNCIA. O núcleo"
       " da terceira tem dimensão 1: há um movimento das juntas que não move a ponta",
       mal == 0 && qz_saturou == 0);
    printf("      E é este o corpo de corpos com preço: juntar dois motores não soma os graus\n");
    printf("      de liberdade indefinidamente — o espaço de saída é o plano, e ele tem 2. A\n");
    printf("      terceira junta não abre dimensão nova; abre um NÚCLEO. Há um movimento das\n");
    printf("      juntas que não move a ponta nenhuma — e é isso que permite desviar de\n");
    printf("      obstáculos sem largar o alvo.\n");
    printf("\n      O base.c §B6 mediu o mesmo com R^a ∨ R^b = R^lcm: o gerado não é a soma, é\n");
    printf("      o menor que contém os dois. Aqui o gerado é o plano, e o que sobra das três\n");
    printf("      juntas fica no núcleo — a parte que age sem aparecer.\n");
}

printf("\n§R6  CONTROLAR: a ponta segue o alvo, e valida-se pelos dois caminhos.\n\n");
{
    /* A lei da pseudo-inversa: q̇ = Jᵀ (JJᵀ)⁻¹ e, e J q̇ = e quando J tem posto 2.
     * Euler de 400 passos era o transporte. Um passo em ℚ basta — e é exacto. */
    printf("      q̇ = Jᵀ(J·Jᵀ)⁻¹·e    — o erro na ponta distribuído pelas juntas\n\n");
    long q[NJ] = { 0, 1, 0 };                          /* regular: q₂ = 90° */
    long x, y;
    direta(q, &x, &y);
    long ax = 12, ay = 0;                              /* o esticado, alcançável em C₄³ */
    long ex = ax - x, ey = ay - y;
    printf("      partida: ponta em (%ld, %ld),  alvo em (%ld, %ld),  erro (%ld, %ld)\n\n",
           x, y, ax, ay, ex, ey);
    long J[2][NJ];
    jacobiano(q, J);
    long A = 0, B = 0, C = 0;
    for(int j = 0; j < NJ; j++){
        A += J[0][j]*J[0][j];
        B += J[0][j]*J[1][j];
        C += J[1][j]*J[1][j];
    }
    long detG = A*C - B*B;
    printf("      det(JJᵀ) = %ld  (posto 2 ⇔ ≠ 0)\n", detG);
    Mat G = mat0(2, 2);
    G.a[0][0] = qz(A,1); G.a[0][1] = qz(B,1);
    G.a[1][0] = qz(B,1); G.a[1][1] = qz(C,1);
    Mat Ginv;
    int tem_inv = mat_inversa(G, &Ginv);
    Qz w0 = qz_soma(qz_mult(Ginv.a[0][0], qz(ex,1)), qz_mult(Ginv.a[0][1], qz(ey,1)));
    Qz w1 = qz_soma(qz_mult(Ginv.a[1][0], qz(ex,1)), qz_mult(Ginv.a[1][1], qz(ey,1)));
    Qz qd[NJ];
    for(int j = 0; j < NJ; j++)
        qd[j] = qz_soma(qz_mult(qz(J[0][j],1), w0), qz_mult(qz(J[1][j],1), w1));
    Qz xd0 = qz(0,1), xd1 = qz(0,1);
    for(int j = 0; j < NJ; j++){
        xd0 = qz_soma(xd0, qz_mult(qz(J[0][j],1), qd[j]));
        xd1 = qz_soma(xd1, qz_mult(qz(J[1][j],1), qd[j]));
    }
    int reproduz = tem_inv && qz_igual(xd0, qz(ex,1)) && qz_igual(xd1, qz(ey,1));
    printf("      J q̇ = (%d/%d, %d/%d)  =  e ?  %s\n\n",
           xd0.p, xd0.q, xd1.p, xd1.q, reproduz ? "sim" : "NÃO");
    /* potência: ⟨e, J q̇⟩ = ⟨Jᵀ e, q̇⟩ */
    Qz Pp = qz_soma(qz_mult(qz(ex,1), xd0), qz_mult(qz(ey,1), xd1));
    Qz Pj = qz(0,1);
    for(int j = 0; j < NJ; j++){
        long tau = J[0][j]*ex + J[1][j]*ey;
        Pj = qz_soma(Pj, qz_mult(qz(tau,1), qd[j]));
    }
    int pot = qz_igual(Pp, Pj);
    printf("      potência ponta %d/%d  juntas %d/%d  iguais? %s\n\n",
           Pp.p, Pp.q, Pj.p, Pj.q, pot ? "sim" : "NÃO");
    /* e o alvo É alcançável no grupo: existe q com ponta = (12, 0). */
    int alcanca = 0;
    for(long a0 = 0; a0 < 4 && !alcanca; a0++)
    for(long a1 = 0; a1 < 4 && !alcanca; a1++)
    for(long a2 = 0; a2 < 4; a2++){
        long qq[NJ] = { a0, a1, a2 }, xx, yy;
        direta(qq, &xx, &yy);
        if(xx == ax && yy == ay){ alcanca = 1; break; }
    }
    ok("J J⁺ e = e — a pseudo-inversa reproduz o erro na ponta, exacto em ℚ, sem Euler."
       " E o alvo (12, 0) é alcançável no grupo C₄³ (o braço esticado)",
       reproduz && alcanca && detG != 0 && qz_saturou == 0);
    ok("e a potência fecha no mesmo passo — a adjunção não se quebra a mexer",
       pot && qz_saturou == 0);
    printf("      E é a rede inteira: o erro é medido UMA vez na ponta, o corpo diferencial\n");
    printf("      reparte-o pelas juntas por Jᵀ, e cada junta é um motor com o seu DTC a\n");
    printf("      executar o seu pedaço. Nenhum motor sabe do alvo — sabe do seu torque.\n");
    printf("\n      A cadeia toda, e cada elo medido:\n\n");
    printf("        Shockley -> NAND -> ALU -> microcontrolador   (eletrico, amplifica, mcu)\n");
    printf("        micro -> tabela DTC -> inversor -> UM motor   (motor.c)\n");
    printf("        N motores -> jacobiano -> a ponta             (aqui)\n\n");
    printf("      E a lei que atravessa os três níveis é a mesma: o CRUZADO gira (o torque), o\n");
    printf("      DIRETO mede (a norma, a potência reativa), e o par adjunto conserva. O robô\n");
    printf("      não é uma aplicação da teoria — é a teoria com três elos de comprimento.\n");
}

printf("\n    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
