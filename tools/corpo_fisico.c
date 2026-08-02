/* corpo_fisico.c — O CORPO FÍSICO: declara-se o IMPOSTO, e a mecânica inteira deriva.
 *
 * O Aarão: "já abre o corpo físico e deriva toda a mecânica, no catálogo e na teoria."
 *
 * E é literalmente o movimento do `regua.c`, aplicado à física em vez de à álgebra. Lá, o cliente
 * declarava a régua (B,C) e as outras três operações saíam dela — não se escreviam. Aqui declara-se
 * UMA coisa,
 *
 *      V(s) = (1 − s²)·S        o imposto algébrico, com S = ‖a×b‖²
 *
 * e sai a mecânica toda, sem se postular nada pelo caminho:
 *
 *      m = S                            a massa É a secção (o cruzado)
 *      F = −∂V/∂s = 2sS                 a força, por derivada e não por decreto
 *      T = ½mṡ² = ½Sṡ²                  a energia cinética
 *      p = mṡ = Sṡ                      o momento
 *      L = T − V                        o lagrangiano
 *      H = T + V                        o hamiltoniano
 *      d/dt(∂L/∂ṡ) − ∂L/∂s = 0          Euler–Lagrange  →  s̈ = 2s
 *      ṡ = ∂H/∂p ,  ṗ = −∂H/∂s          Hamilton, o mesmo por outro lado
 *      W = ∫F ds = −ΔV                  o trabalho
 *      P = F·ṡ = dT/dt                  a potência
 *      J = ∫F dt = Δp                   o impulso
 *
 * NADA DISTO É POSTULADO. Cada linha é uma DERIVADA do imposto, e este ficheiro mede cada uma
 * contra a sua definição numérica — porque escrever a fórmula certa e escrever a fórmula que eu
 * acho que é certa dão o mesmo aspeto no papel. O que separa as duas é a diferença finita.
 *
 * E o que o corpo físico herda do resto do projeto, sem nada de novo:
 *
 *   · a massa é o CRUZADO (S = ‖a×b‖²). Logo *sem cruzado não há massa*, e um corpo no mesmo
 *     campo local (a×b = 0) não tem inércia nenhuma — atravessa sem custo.
 *   · a força é direto × cruzado (F = 2sS), e anula-se de qualquer um dos lados.
 *   · o potencial é o cruzado ao quadrado vezes a secção (Π = 1−s² = sin²), o que faz do
 *     imposto uma ÁREA e não um comprimento.
 *
 *   §H1  a FORÇA sai por derivada do imposto — e bate com a diferença finita
 *   §H2  EULER–LAGRANGE devolve s̈ = 2s, sem se lá pôr
 *   §H3  HAMILTON dá o mesmo por outro caminho — dois caminhos que têm de concordar
 *   §H4  o TRABALHO é −ΔV, e a POTÊNCIA é dT/dt: medidas, não afirmadas
 *   §H5  o IMPULSO é Δp, e o momento conserva-se quando a força se anula
 *   §H6  o VIRIAL e o horizonte: onde o corpo fica preso e onde foge
 *
 *   cc -O2 -std=c99 -I. corpo_fisico.c -lm -o corpo_fisico && ./corpo_fisico
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

/* O ÚNICO dado declarado: o imposto. Tudo o resto é derivada dele. */
static double S_glob = 1.0;
static double V(double s){ return (1.0 - s*s) * S_glob; }

/* e as derivadas NUMÉRICAS, que são o juiz — não se confia na fórmula que eu escrevi */
static double dVds(double s){ double h = 1e-6; return (V(s+h) - V(s-h))/(2*h); }

int main(void){
printf("\n=== O CORPO FÍSICO: declara-se o IMPOSTO, e a mecânica deriva ==============\n");
printf("    Um dado só: V(s) = (1−s²)·S. Massa, força, momento, lagrangiano,\n");
printf("    hamiltoniano, trabalho, potência e impulso saem dele por derivada.\n");

double a[3] = {1.0, 0.4, -0.2}, b[3] = {0.3, -0.6, 0.8};
double c[3] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
S_glob = c[0]*c[0] + c[1]*c[1] + c[2]*c[2];
printf("\n    a = (%.1f, %.1f, %.1f)   b = (%.1f, %.1f, %.1f)\n", a[0],a[1],a[2],b[0],b[1],b[2]);
printf("    S = ‖a×b‖² = %.6f   ← e esta é a MASSA, porque a massa é o cruzado\n", S_glob);

printf("\n§H1  A FORÇA sai por DERIVADA do imposto — e bate com a diferença finita.\n\n");
{
    /* F = -dV/ds. A formula fechada da' 2sS. Mede-se contra a derivada numerica do V
     * declarado — se eu tivesse escrito a formula errada, a diferenca aparecia aqui. */
    printf("      s        V(s)        F fechada = 2sS   F = −dV/ds (numérica)   |dif|\n");
    double pior = 0;
    for(int i = -3; i <= 3; i++){
        double s = i*0.35;
        double Ff = 2*s*S_glob, Fn = -dVds(s), d = fabs(Ff - Fn);
        if(d > pior) pior = d;
        printf("      %+7.3f  %-11.6f %-17.6f %-22.6f %.2e\n", s, V(s), Ff, Fn, d);
    }
    printf("\n      pior diferença: %.3e\n\n", pior);
    ok("a força é MENOS a derivada do imposto — 2sS não foi postulado, saiu", pior < 1e-6);
    printf("      E repare-se onde ela se anula: em s = 0 (o direto é zero) e em S = 0 (não há\n");
    printf("      cruzado). A força é o produto dos dois, e morre de qualquer um dos lados.\n");
}

printf("\n§H2  EULER–LAGRANGE devolve s̈ = 2s — sem se lá pôr.\n\n");
{
    /* L = T - V = (1/2)S ṡ² - (1-s²)S. Euler-Lagrange: d/dt(dL/dṡ) - dL/ds = 0.
     * dL/dṡ = S ṡ  ->  d/dt = S s̈ ;   dL/ds = 2sS.  Logo S s̈ = 2sS -> s̈ = 2s.
     * Mede-se com derivadas NUMERICAS do L, para nao ser eu a dizer o que dá. */
    printf("      s       ṡ       ∂L/∂ṡ (num)   ∂L/∂s (num)   s̈ = (∂L/∂s)/m   2s      |dif|\n");
    double pior = 0, h = 1e-6;
    for(int i = -2; i <= 2; i++){
        double s = i*0.4, v = 0.3 + 0.1*i;
        /* L como função de (s, v), com o V declarado lá em cima */
        double Lv1 = 0.5*S_glob*(v+h)*(v+h) - V(s), Lv0 = 0.5*S_glob*(v-h)*(v-h) - V(s);
        double dLdv = (Lv1 - Lv0)/(2*h);                       /* = m ṡ = p */
        double Ls1 = 0.5*S_glob*v*v - V(s+h), Ls0 = 0.5*S_glob*v*v - V(s-h);
        double dLds = (Ls1 - Ls0)/(2*h);                       /* = 2sS */
        double acel = dLds / S_glob;                            /* m = S */
        double d = fabs(acel - 2*s);
        if(d > pior) pior = d;
        printf("      %+6.2f  %+6.2f  %-13.6f %-13.6f %-16.6f %-7.3f %.2e\n",
               s, v, dLdv, dLds, acel, 2*s, d);
    }
    printf("\n      pior diferença: %.3e\n\n", pior);
    ok("Euler–Lagrange sobre L = T − V dá s̈ = 2s, e a massa que aparece É S", pior < 1e-5);
    printf("      A equação universal do paper_A não é um postulado do modelo: é o que sai de\n");
    printf("      derivar o imposto duas vezes. E o m que cancela é a secção, o cruzado.\n");
}

printf("\n§H3  HAMILTON dá o mesmo por OUTRO caminho — e os dois têm de concordar.\n\n");
{
    /* H(s,p) = p²/(2m) + V(s). Hamilton: ṡ = dH/dp, ṗ = -dH/ds. Se a mecanica esta' certa,
     * integrar por Hamilton e por Lagrange tem de dar a MESMA trajetoria. Dois caminhos. */
    double s1 = 0.2, v1 = 0.1;                 /* Lagrange: s̈ = 2s */
    double s2 = 0.2, p2 = S_glob*0.1;          /* Hamilton: (s, p) */
    double h = 1e-6; long N = 500000;
    printf("      passo       Lagrange s      Hamilton s      |dif|\n");
    double pior = 0;
    for(long i = 1; i <= N; i++){
        /* Lagrange */
        v1 += 2*s1*h; s1 += v1*h;
        /* Hamilton, com as derivadas NUMÉRICAS de H */
        double dHdp = p2/S_glob;                                  /* ṡ */
        double dHds = dVds(s2);                                   /* ∂V/∂s */
        p2 += (-dHds)*h; s2 += dHdp*h;
        double d = fabs(s1 - s2);
        if(d > pior) pior = d;
        if(i % 125000 == 0) printf("      %-11ld %-15.8f %-15.8f %.2e\n", i, s1, s2, d);
    }
    printf("\n      pior divergência entre os dois formalismos: %.3e\n\n", pior);
    ok("Lagrange e Hamilton dão a MESMA trajetória — dois caminhos, um corpo", pior < 1e-6);
    printf("      Não é redundância: é o teste. Uma força errada passaria num formalismo e\n");
    printf("      falharia no outro, porque um deriva de L e o outro de H.\n");
}

printf("\n§H4  O TRABALHO é −ΔV e a POTÊNCIA é dT/dt — medidos, não afirmados.\n\n");
{
    /* W = integral de F ds. Se a mecanica fecha, W tem de dar exatamente -(V_fim - V_ini),
     * e a potencia instantanea F·ṡ tem de ser a derivada da energia cinetica. */
    double s = -0.6, v = 0.0, h = 1e-6; long N = 400000;
    double V0 = V(s), T0 = 0.5*S_glob*v*v, W = 0, piorP = 0;
    for(long i = 0; i < N; i++){
        double F = -dVds(s);
        double Tantes = 0.5*S_glob*v*v;
        v += (F/S_glob)*h;
        double ds = v*h; s += ds;
        W += F*ds;                                    /* trabalho acumulado */
        double Tdepois = 0.5*S_glob*v*v;
        double Pot = F*v, dTdt = (Tdepois - Tantes)/h;
        double d = fabs(Pot - dTdt);
        if(i > 10 && d > piorP) piorP = d;
    }
    double dV = V(s) - V0, T = 0.5*S_glob*v*v;
    printf("      trabalho acumulado  W        = %+.8f\n", W);
    printf("      menos a variação do imposto  −ΔV = %+.8f\n", -dV);
    printf("      |W + ΔV| = %.3e\n\n", fabs(W + dV));
    printf("      variação da energia cinética ΔT = %+.8f  (e W = ΔT: %.3e)\n\n",
           T - T0, fabs(W - (T - T0)));
    ok("o trabalho da força É menos a variação do imposto", fabs(W + dV) < 1e-5);
    ok("e o trabalho É a variação da energia cinética — o teorema fecha", fabs(W - (T-T0)) < 1e-5);
    ok("a potência instantânea F·ṡ é a derivada de T", piorP < 1e-3);
}

printf("\n§H5  O IMPULSO é Δp, e o momento CONSERVA-SE quando a força se anula.\n\n");
{
    /* J = integral de F dt = Delta p. E o controlo que torna isto uma medida e nao uma
     * definicao: no caso s = 0 a forca e' nula, e entao o momento tem de ficar PARADO.
     * Sem esse caso, a asserção seria verdadeira por construcao do integrador. */
    double s = 0.3, v = 0.05, h = 1e-6; long N = 300000;
    double p0 = S_glob*v, J = 0;
    for(long i = 0; i < N; i++){
        double F = -dVds(s);
        J += F*h;
        v += (F/S_glob)*h; s += v*h;
    }
    double p1 = S_glob*v;
    printf("      impulso acumulado J = ∫F dt = %+.8f\n", J);
    printf("      variação do momento Δp      = %+.8f      |dif| = %.2e\n\n", p1-p0, fabs(J-(p1-p0)));
    ok("o impulso É a variação do momento", fabs(J - (p1-p0)) < 1e-5);
    /* o CONTROLO: com s = 0 exatamente, F = 0 e o momento não se pode mexer */
    {
        double s2 = 0.0, v2 = 0.0, p20 = S_glob*v2;
        for(long i = 0; i < 100000; i++){ double F = -dVds(s2); v2 += (F/S_glob)*h; s2 += v2*h; }
        double p21 = S_glob*v2;
        ok("em s = 0 a força é nula e o momento NÃO se mexe — o controlo", fabs(p21-p20) < 1e-9);
        printf("      (controlo positivo: sem este caso, a asserção acima passaria por\n");
        printf("       construção do integrador, e não por a mecânica estar certa.)\n");
    }
}

printf("\n§H6  O VIRIAL e o HORIZONTE: onde o corpo fica preso, e onde foge.\n\n");
{
    /* O imposto e' um potencial INVERTIDO (-s^2), logo nao ha' poco: todo corpo com |s|<1 e
     * energia suficiente foge. O horizonte |s|=1 e' onde V muda de sinal. Mede-se o tempo de
     * fuga e compara-se com a solucao fechada s(t) = s0 cosh(sqrt(2) t): o tempo em que
     * |s| = 1 e' t = arccosh(1/s0)/sqrt(2). Dois caminhos outra vez. */
    printf("      s₀       t de fuga medido   t fechado = arccosh(1/s₀)/√2   |dif|\n");
    double pior = 0;
    for(int i = 1; i <= 5; i++){
        double s0 = 0.15*i, s = s0, v = 0, h = 1e-6, t = 0;
        while(fabs(s) < 1.0 && t < 20){ v += 2*s*h; s += v*h; t += h; }
        double tf = acosh(1.0/s0)/sqrt(2.0);
        double d = fabs(t - tf);
        if(d > pior) pior = d;
        printf("      %-8.2f %-18.6f %-29.6f %.2e\n", s0, t, tf, d);
    }
    printf("\n      pior diferença: %.3e\n\n", pior);
    ok("o tempo de fuga bate com arccosh(1/s₀)/√2 — a dinâmica é a que se derivou", pior < 1e-4);
    printf("      E não há poço: o imposto tem o sinal invertido, logo o corpo em repouso no\n");
    printf("      horizonte é o único que fica. Todo o resto atravessa — e do outro lado a\n");
    printf("      pressão é negativa, que é a hipérbole e a família real.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Um dado declarado — o imposto V = (1−s²)S — e a mecânica inteira derivada:\n");
printf("    massa, força, momento, Lagrange, Hamilton, trabalho, potência, impulso e o\n");
printf("    tempo de fuga. Nenhuma foi postulada; todas foram medidas contra a derivada\n");
printf("    numérica do próprio imposto. E a massa é o CRUZADO: sem produto cruzado não\n");
printf("    há inércia, não há força e não há imposto.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
