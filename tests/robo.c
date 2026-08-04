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
 *   §R1  N motores, cada um um corpo — e correm independentes
 *   §R2  o JACOBIANO é o corpo diferencial: J é a derivada da cinemática
 *   §R3  J e Jᵀ são ADJUNTOS, e a adjunção É a conservação de potência
 *   §R4  a SINGULARIDADE é a degenerescência: det J = 0, e ali é ε² = 0
 *   §R5  a rede como corpo de corpos: o que ela gera é maior que a soma
 *   §R6  CONTROLAR: a ponta segue o alvo, e valida-se pelos dois caminhos
 *
 *   cc -O2 -std=c99 robo.c -lm -o robo && ./robo
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"

#define NJ 3                                   /* juntas do braço planar */

/* ---- a cinemática: um braço planar de NJ juntas, elos de comprimento L ------------------ */
static const double L[NJ] = { 1.0, 0.8, 0.6 };

static void direta(const double *q, double *x, double *y){
    double a = 0; *x = 0; *y = 0;
    for(int k = 0; k < NJ; k++){
        a += q[k];
        *x += L[k]*cos(a);
        *y += L[k]*sin(a);
    }
}
/* O JACOBIANO — e é a DERIVADA: J[i][j] = ∂x_i/∂q_j. Forma fechada. */
static void jacobiano(const double *q, double J[2][NJ]){
    for(int j = 0; j < NJ; j++){
        double dx = 0, dy = 0, a = 0;
        for(int k = 0; k < NJ; k++){
            a += q[k];
            if(k >= j){ dx += -L[k]*sin(a); dy += L[k]*cos(a); }
        }
        J[0][j] = dx; J[1][j] = dy;
    }
}
/* o mesmo jacobiano por DIFERENÇA FINITA — o segundo caminho */
static void jacobiano_num(const double *q, double J[2][NJ], double h){
    for(int j = 0; j < NJ; j++){
        double qp[NJ], qm[NJ], xp, yp, xm, ym;
        memcpy(qp, q, sizeof qp); memcpy(qm, q, sizeof qm);
        qp[j] += h; qm[j] -= h;
        direta(qp, &xp, &yp); direta(qm, &xm, &ym);
        J[0][j] = (xp-xm)/(2*h); J[1][j] = (yp-ym)/(2*h);
    }
}
static double det2(const double J[2][NJ], int a, int b){
    return J[0][a]*J[1][b] - J[0][b]*J[1][a];
}
/* a "manipulabilidade": √det(J·Jᵀ) — zero exatamente na singularidade */
static double manip(const double J[2][NJ]){
    double A = 0, B = 0, C = 0;
    for(int j = 0; j < NJ; j++){ A += J[0][j]*J[0][j]; B += J[0][j]*J[1][j]; C += J[1][j]*J[1][j]; }
    double d = A*C - B*B;
    return d > 0 ? sqrt(d) : 0;
}

int main(void){
printf("\n=== A REDE DIFERENCIAL DE MOTORES ========================================\n");
printf("    Cada junta é um motor (um corpo); o jacobiano é o corpo diferencial que\n");
printf("    controla todos. E J com Jᵀ são o par adjunto — a potência é a mesma.\n");

printf("\n§R1  N motores, cada um um corpo, e correm independentes.\n\n");
{
    /* Cada junta tem o seu motor com o seu DTC: a sua referencia de fluxo, o seu torque, a
     * sua banda. Sao N corpos, e o que os torna uma REDE nao e' partilharem o controlo — e'
     * partilharem a ESTRUTURA. Mede-se que cada um fecha sozinho. */
    printf("      junta   L (m)   ref. de torque   banda    fecha sozinho?\n");
    int mal = 0;
    for(int j = 0; j < NJ; j++){
        double TeRef = 0.30 - 0.05*j, banda = 0.07;
        /* um DTC mínimo por junta: o erro cai dentro da banda e lá fica */
        double Te = 0.0, dt = 1e-3;
        int dentro = 0;
        for(int k = 0; k < 4000; k++){
            int d = (Te < TeRef - banda) ? +1 : (Te > TeRef + banda) ? -1 : 0;
            if(d == 0) d = (Te < TeRef) ? +1 : -1;
            Te += d*dt*20.0;
            if(k > 2000 && fabs(Te - TeRef) < 2*banda) dentro++;
        }
        printf("      %-7d %-7.1f %-16.2f %-8.2f %s\n", j+1, L[j], TeRef, banda,
               dentro > 1900 ? "sim" : "NÃO");
        if(dentro <= 1900) mal++;
    }
    printf("\n");
    ok("os %d motores fecham cada um no seu corpo, com a sua régua", mal == 0);
    printf("      São N corpos, e o que faz deles uma REDE não é partilharem o controlador — é\n");
    printf("      partilharem a estrutura. Cada um tem a sua borda, a sua régua e o seu resíduo;\n");
    printf("      o que os liga vem a seguir, e é uma derivada.\n");
}

printf("\n§R2  O JACOBIANO É o corpo diferencial: J é a derivada da cinemática.\n\n");
{
    /* J[i][j] = ∂x_i/∂q_j. Mede-se pelos DOIS caminhos: a forma fechada e a diferenca finita.
     * E o ponto e' que J nao e' "uma matriz util" — e' a DERIVADA, e por isso o corpo que
     * controla a rede e' o DIFERENCIAL. */
    printf("      ẋ = J·q̇,   com J[i][j] = ∂x_i/∂q_j — a derivada da cinemática direta\n\n");
    printf("      configuração q (rad)      pior resíduo (fechada × diferença finita)\n");
    int mal = 0;
    for(int c = 0; c < 6; c++){
        double q[NJ] = { 0.3 + 0.4*c, -0.5 + 0.3*c, 0.2 + 0.25*c };
        double Jf[2][NJ], Jn[2][NJ];
        jacobiano(q, Jf); jacobiano_num(q, Jn, 1e-6);
        double pior = 0;
        for(int i = 0; i < 2; i++) for(int j = 0; j < NJ; j++){
            double e = fabs(Jf[i][j]-Jn[i][j]);
            if(e > pior) pior = e;
        }
        printf("      (%+.2f, %+.2f, %+.2f)      %.3e\n", q[0], q[1], q[2], pior);
        if(pior > 1e-8) mal++;
    }
    printf("\n");
    ok("a forma fechada e a diferença finita dão o MESMO J — é mesmo a derivada", mal == 0);
    printf("      É por isto que o corpo que controla a rede é o DIFERENCIAL, e não outro: a\n");
    printf("      relação entre o que as juntas fazem e o que a ponta faz É uma derivada. O\n");
    printf("      milenio.c §M6 mediu que o diferencial é a instância máxima — aqui vê-se para\n");
    printf("      que serve ser máxima: ele contém os N motores e a lei que os compõe.\n");
}

printf("\n§R3  J e Jᵀ são ADJUNTOS — e a adjunção É a conservação de potência.\n\n");
{
    /* ⟨F, J·q̇⟩ = ⟨Jᵀ·F, q̇⟩. Do lado esquerdo: a potencia na PONTA (forca × velocidade).
     * Do direito: a potencia nas JUNTAS (torque × velocidade angular). Sao iguais — e isso
     * e' a adjuncao do §B12, agora em watts. */
    printf("      ẋ = J·q̇      (a velocidade SOBE: juntas -> ponta)    <- a torre branca\n");
    printf("      τ = Jᵀ·F     (a força DESCE: ponta -> juntas)        <- a torre negra\n\n");
    printf("      ⟨F, ẋ⟩ = ⟨F, J·q̇⟩ = ⟨Jᵀ·F, q̇⟩ = ⟨τ, q̇⟩\n\n");
    int mal = 0;
    printf("      caso   potência na PONTA    potência nas JUNTAS   resíduo\n");
    for(int c = 0; c < 200; c++){
        double q[NJ]  = { 0.2+0.03*c, -0.4+0.02*c, 0.1+0.017*c };
        double qd[NJ] = { sin(3.0*c), cos(5.0*c), sin(7.0*c+1) };
        double F[2]   = { cos(11.0*c), sin(13.0*c+2) };
        double J[2][NJ];
        jacobiano(q, J);
        /* ẋ = J·q̇ */
        double xd[2] = {0,0};
        for(int i = 0; i < 2; i++) for(int j = 0; j < NJ; j++) xd[i] += J[i][j]*qd[j];
        /* τ = Jᵀ·F */
        double tau[NJ] = {0};
        for(int j = 0; j < NJ; j++) for(int i = 0; i < 2; i++) tau[j] += J[i][j]*F[i];
        double Pponta = F[0]*xd[0] + F[1]*xd[1];
        double Pjunta = 0;
        for(int j = 0; j < NJ; j++) Pjunta += tau[j]*qd[j];
        double res = fabs(Pponta - Pjunta);
        if(c < 4) printf("      %-6d %+-19.9f %+-21.9f %.1e\n", c, Pponta, Pjunta, res);
        if(res > 1e-12) mal++;
    }
    printf("\n      (200 casos medidos)\n\n");
    ok("⟨F, J·q̇⟩ = ⟨Jᵀ·F, q̇⟩ — a potência é a MESMA dos dois lados, resíduo 0", mal == 0);
    printf("      É a adjunção do base.c §B12, e aqui ela tem unidade de watt. Lá dizia-se \"as\n");
    printf("      duas torres são adjuntas, e é esse o equilíbrio em todos os andares\"; aqui o\n");
    printf("      equilíbrio chama-se CONSERVAÇÃO DE ENERGIA, e o que uma torre sobe a outra\n");
    printf("      desce com a mesma conta.\n");
    printf("\n      E a assimetria certa continua lá: J leva N juntas em 2 coordenadas (perde),\n");
    printf("      Jᵀ leva 2 forças em N torques (não é sobrejetiva). Cada uma falha onde a\n");
    printf("      outra fecha — que é exatamente o que o §B11 mediu da inclusão e do traço.\n");
}

printf("\n§R4  A SINGULARIDADE é a degenerescência: det J = 0, e ali é ε² = 0.\n\n");
{
    /* Onde o braco estica (ou dobra sobre si), as colunas de J alinham-se e a manipulabilidade
     * anula. Perde-se um grau de liberdade — e' a RAIZ DUPLA, o ε²=0 do fisica.c §P1: duas
     * direcoes que colapsaram numa. */
    printf("      manipulabilidade w = √det(J·Jᵀ) — zero exatamente na singularidade\n\n");
    printf("      q₂ (rad)   q₃ (rad)   w = √det(JJᵀ)   direções úteis   estado\n");
    int mal = 0;
    for(int c = 0; c <= 6; c++){
        double q[NJ] = { 0.4, c*0.0 + (c==0?0.0:0.0), 0.0 };
        q[1] = (c*M_PI/6.0) - 0*M_PI;              /* de 0 a π */
        q[2] = 0.0;
        double J[2][NJ];
        jacobiano(q, J);
        double w = manip(J);
        /* O LIMIAR tem de ser relativo à escala do braço, e não 1e-9 absoluto. Com 1e-9 a
         * linha q₂ = π (o braço DOBRADO sobre si — a outra singularidade) dava w = 1,1e-8 e
         * saía rotulada "regular": a tabela a mentir, com a asserção a passar porque eu só
         * verificava o caso esticado. w é da ordem de L² ≈ 1, logo o limiar é relativo. */
        double escala = (L[0]+L[1]+L[2])*(L[0]+L[1]+L[2]);
        int sing = w < 1e-6*escala;
        int esperado = (fabs(q[1]) < 1e-9) || (fabs(fabs(q[1]) - M_PI) < 1e-9);
        printf("      %-10.4f %-10.4f %-15.9f %-16s %s\n", q[1], q[2], w,
               sing ? "1 (perdeu uma)" : "2",
               sing ? (fabs(q[1]) < 1e-9 ? "SINGULAR — o braço esticado"
                                         : "SINGULAR — o braço dobrado")
                    : "regular");
        if(sing != esperado) mal++;              /* as DUAS singularidades, não só uma */
    }
    printf("\n");
    /* varrer para achar TODAS as singularidades no intervalo */
    int nsing = 0;
    double antw = -1;
    for(int c = 0; c <= 2000; c++){
        double q[NJ] = { 0.4, -M_PI + 2*M_PI*c/2000.0, 0.0 };
        double J[2][NJ];
        jacobiano(q, J);
        double w = manip(J);
        double esc = (L[0]+L[1]+L[2])*(L[0]+L[1]+L[2]);
        if(antw >= 0 && ((antw > 1e-6*esc) != (w > 1e-6*esc))) nsing++;
        antw = w;
    }
    printf("      varrendo q₂ de -π a π: %d travessias da singularidade\n\n", nsing);
    ok("a manipulabilidade anula nas DUAS singularidades — esticado E dobrado",
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
    /* Cada junta sozinha traca um CIRCULO (1 grau de liberdade). Duas juntas tracam uma
     * COROA (2 graus). Tres tracam uma coroa mais espessa E com redundancia. Mede-se a
     * dimensao do que cada conjunto alcanca — e o ganho NAO e' aditivo. */
    printf("      juntas ativas   graus de liberdade   o que a ponta alcança\n");
    int mal = 0;
    for(int n = 1; n <= NJ; n++){
        /* amostrar o alcançável e medir o posto do jacobiano das n primeiras */
        int postoMax = 0;
        for(int c = 0; c < 400; c++){
            double q[NJ] = { 0.3+0.02*c, 0.7+0.013*c, -0.4+0.011*c };
            for(int j = n; j < NJ; j++) q[j] = 0;         /* as outras congeladas */
            double J[2][NJ];
            jacobiano(q, J);
            /* posto: quantas colunas (das n ativas) são independentes em R² */
            int posto = 0;
            for(int a = 0; a < n; a++){
                if(fabs(J[0][a]) + fabs(J[1][a]) < 1e-12) continue;
                if(posto == 0){ posto = 1; continue; }
                for(int b = 0; b < a; b++)
                    if(fabs(det2(J,b,a)) > 1e-9){ posto = 2; break; }
                if(posto == 2) break;
            }
            if(posto > postoMax) postoMax = posto;
        }
        printf("      %-15d %-20d %s\n", n, postoMax,
               postoMax == 1 ? "um círculo (só o raio fixo)"
             : n == 2        ? "uma coroa — o plano, localmente"
                             : "a coroa, e com REDUNDÂNCIA (3 juntas, 2 saídas)");
        if(postoMax != (n == 1 ? 1 : 2)) mal++;
    }
    printf("\n");
    ok("uma junta dá 1 grau, duas dão 2 — e a terceira não dá 3: dá REDUNDÂNCIA", mal == 0);
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
    /* O controlo diferencial: dado o erro na ponta, distribuir pelas juntas com a
     * pseudo-inversa. q̇ = Jᵀ(JJᵀ)⁻¹ ẋ. Cada junta recebe o SEU torque e o seu DTC executa.
     * E valida-se por DOIS caminhos: (a) a ponta chega ao alvo (cinemática direta) e
     * (b) a potência fecha (a adjunção do §R3) em todo o percurso. */
    printf("      q̇ = Jᵀ(J·Jᵀ)⁻¹·ẋ    — o erro na ponta distribuído pelas juntas\n\n");
    double q[NJ] = { 0.6, 0.8, -0.5 };
    double ax = 1.4, ay = 0.9;                     /* o alvo */
    double x, y;
    direta(q, &x, &y);
    printf("      partida: ponta em (%.4f, %.4f),  alvo em (%.4f, %.4f)\n\n", x, y, ax, ay);
    printf("      passo    ponta (x, y)          erro       potência: ponta × juntas\n");
    int malP = 0, chegou = 0;
    double erro0 = hypot(ax-x, ay-y);
    for(int k = 0; k <= 400; k++){
        direta(q, &x, &y);
        double ex = ax-x, ey = ay-y, err = hypot(ex,ey);
        double J[2][NJ];
        jacobiano(q, J);
        /* J·Jᵀ (2x2) e a sua inversa */
        double A = 0, B = 0, C = 0;
        for(int j = 0; j < NJ; j++){ A += J[0][j]*J[0][j]; B += J[0][j]*J[1][j]; C += J[1][j]*J[1][j]; }
        double det = A*C - B*B;
        if(fabs(det) < 1e-9) break;                /* na singularidade, pára — e é honesto */
        double iv[2][2] = { { C/det, -B/det }, { -B/det, A/det } };
        double w[2] = { iv[0][0]*ex + iv[0][1]*ey, iv[1][0]*ex + iv[1][1]*ey };
        double qd[NJ] = {0};
        for(int j = 0; j < NJ; j++) qd[j] = J[0][j]*w[0] + J[1][j]*w[1];
        /* o SEGUNDO caminho: a potência tem de fechar, agora com F = o erro */
        double xd[2] = {0,0};
        for(int i = 0; i < 2; i++) for(int j = 0; j < NJ; j++) xd[i] += J[i][j]*qd[j];
        double tau[NJ] = {0};
        for(int j = 0; j < NJ; j++) tau[j] = J[0][j]*ex + J[1][j]*ey;
        double Pp = ex*xd[0] + ey*xd[1], Pj = 0;
        for(int j = 0; j < NJ; j++) Pj += tau[j]*qd[j];
        if(fabs(Pp-Pj) > 1e-10) malP++;
        if(k % 80 == 0)
            printf("      %-8d (%+.5f, %+.5f)   %-10.6f %+.6f × %+.6f\n", k, x, y, err, Pp, Pj);
        if(err < 1e-6){ chegou = 1; break; }
        double passo = 0.06;
        for(int j = 0; j < NJ; j++) q[j] += passo*qd[j];
    }
    direta(q, &x, &y);
    double errF = hypot(ax-x, ay-y);
    printf("\n      erro inicial: %.6f    erro final: %.3e    chegou: %s\n",
           erro0, errF, chegou ? "sim" : "não");
    printf("      discordâncias de potência ao longo do percurso: %d\n\n", malP);
    ok("a ponta chega ao alvo distribuindo o erro pelas juntas", chegou && errF < 1e-6);
    ok("e a potência fecha em TODO o percurso — a adjunção não se quebra a mexer", malP == 0);
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

printf("\n");
return falhas ? 1 : 0;
}
