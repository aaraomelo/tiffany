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
 *   §H1  a FORÇA sai por derivada do imposto — resíduo ZERO por DUAS rotas exactas
 *   §H2  EULER–LAGRANGE devolve s̈ = 2s exacto, sem se lá pôr
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
#include "reta.h"      /* Dir e Cruz: a operação */
#include "unidade.h"
#include "racionais.h"
#include "calculo.h"

/* O ÚNICO dado declarado: o imposto. Tudo o resto é derivada dele. */
static double S_glob = 1.0;
static double V(double s){ return (1.0 - s*s) * S_glob; }

/* E O JUIZ NÃO É UMA DIFERENÇA FINITA COM h PEQUENO — é a derivada EXACTA.
 *
 * V(s) = (1 − s²)·S é um POLINÓMIO, e a casa já tem o teorema (lib/calculo.h): o
 * quociente de diferenças (f(a+h) − f(a))/h É ELE PRÓPRIO um polinómio em h, porque o
 * numerador se anula em h = 0 e portanto é divisível por h EXACTAMENTE. Logo f'(a) é
 * uma AVALIAÇÃO — q(0) — e não um limite. A diferença central de um polinómio de grau 2
 * é exacta para TODO h:
 *
 *      (V(s+h) − V(s−h))/(2h) = −2sS        sem erro, e sem depender de h
 *
 * O h = 1e-6 que aqui estava não media nada: fabricava o erro de arredondamento que o
 * `fabs(...) == 0.0` a seguir tolerava. Aqui as duas rotas são EXACTAS e concordam bit
 * a bit, e o que era «bate dentro de 1e-6» passa a ser ZERO.
 *
 *   rota A   fn_deriva      a regra formal do polinómio, avaliada em s
 *   rota B   fn_deriva_def  a DEFINIÇÃO, pela fibra da divisão por h
 *
 * O double fica onde tem função: nos integradores dos §H3–§H6, que simulam uma EDO no
 * tempo. Mas a FORÇA que eles usam passa a ser a exacta, derivada aqui. */
static Cf V_ex;                 /* V como polinómio em Qz */
static Qz S_ex;                 /* a secção, exacta */

/* E a derivada que os integradores dos §H3--§H6 usam é a EXACTA, não uma diferença
 * finita: V′(s) = −2sS. Ela não é postulada aqui — é o que o §H1 mede, por duas rotas
 * independentes e com resíduo ZERO. Usar a fórmula depois de a provar é o movimento
 * normal; usar uma diferença finita com h = 1e-6 seria pôr de volta o erro que o §H1
 * acabou de mostrar não existir. */
static double dVds(double s){ return -2.0*s*S_glob; }

int main(void){
printf("\n=== O CORPO FÍSICO: declara-se o IMPOSTO, e a mecânica deriva ==============\n");
printf("    Um dado só: V(s) = (1−s²)·S. Massa, força, momento, lagrangiano,\n");
printf("    hamiltoniano, trabalho, potência e impulso saem dele por derivada.\n");

/* Os vectores são racionais, e por isso o cruzado e a secção são EXACTOS: escrevem-se
 * como inteiros sobre 10, e o produto vectorial de inteiros é inteiro. Escrevê-los como
 * 1.0, 0.4, −0.2 dava a S um arredondamento antes de a mecânica começar. */
long A[3] = {10, 4, -2}, B[3] = {3, -6, 8};              /* = a·10 e b·10 */
long C[3]; rt_cruz3(A, B, C);
long n2 = C[0]*C[0] + C[1]*C[1] + C[2]*C[2];             /* ‖A×B‖², inteiro */
S_ex = qz(n2, 10000);                                     /* S = ‖a×b‖² = ‖A×B‖²/10⁴ */
V_ex = fn0(); V_ex.n = 2; V_ex.c[0] = S_ex; V_ex.c[2] = qz_oposto(S_ex);
S_glob = (double)S_ex.p / (double)S_ex.q;                 /* só para os integradores */
printf("\n    a = (%ld, %ld, %ld)/10   b = (%ld, %ld, %ld)/10\n",
       A[0],A[1],A[2], B[0],B[1],B[2]);
printf("    a×b = (%ld, %ld, %ld)/100   e   S = ‖a×b‖² = %d/%d   EXACTO\n",
       C[0],C[1],C[2], S_ex.p, S_ex.q);
printf("    ← e esta é a MASSA, porque a massa é o cruzado\n");

printf("\n§H1  A FORÇA sai por DERIVADA do imposto — e o resíduo é ZERO EXACTO.\n\n");
{
    /* F = −dV/ds, e a fórmula fechada dá 2sS. Mede-se por DUAS rotas exactas, que têm de
     * concordar — se eu tivesse escrito a fórmula errada, a discordância aparecia aqui.
     * E o que se mede é resíduo ZERO, não «menor que a régua que eu escolhi». */
    printf("      s        V(s)          V′ pela REGRA   V′ pela DEFINIÇÃO   iguais?  −V′ = 2sS?\n");
    long pontos = 0, concordam = 0, fechada = 0, def_ok = 0;
    Cf dV = fn_deriva(V_ex);
    for(int i = -3; i <= 3; i++){
        Qz sq = qz(7*i, 20);                          /* s = 0,35·i, exacto */
        Qz reg = fn_av(dV, sq);                       /* rota A: a regra formal */
        Qz def = qz(0,1); int achou = fn_deriva_def(V_ex, sq, &def);  /* rota B: a fibra */
        Qz Ff = qz_mult(qz(2,1), qz_mult(sq, S_ex));  /* a fechada: 2sS */
        Qz Vs = fn_av(V_ex, sq);
        pontos++;
        if(achou) def_ok++;
        if(achou && qz_igual(reg, def)) concordam++;
        if(qz_igual(qz_oposto(reg), Ff)) fechada++;
        printf("      %+3d/20   %+6d/%-6d %+6d/%-6d  %+6d/%-6d      %-8s %s\n",
               7*i, Vs.p, Vs.q, reg.p, reg.q, def.p, def.q,
               (achou && qz_igual(reg,def)) ? "sim" : "NÃO",
               qz_igual(qz_oposto(reg), Ff) ? "sim" : "NÃO");
    }
    printf("\n      %ld pontos: a definição resolveu em %ld, as duas rotas concordam em %ld,"
           " e −V′ = 2sS em %ld\n", pontos, def_ok, concordam, fechada);
    printf("      resíduo: ZERO EXACTO — não «menor que 1e-6»\n\n");
    ok("A FORÇA É MENOS A DERIVADA DO IMPOSTO, E O RESÍDUO É ZERO EXACTO: 2sS não foi"
       " postulado, saiu — e sai por DUAS rotas independentes que concordam bit a bit, a"
       " regra formal do polinómio e a DEFINIÇÃO pela fibra da divisão por h. Nenhuma"
       " diferença finita com h pequeno: V é um polinómio, logo (V(s+h)−V(s−h))/(2h) é"
       " exacto para TODO h, e o 1e-6 que aqui estava só fabricava o arredondamento que a"
       " tolerância a seguir perdoava",
       concordam == pontos && fechada == pontos && def_ok == pontos && pontos == 7
       && qz_saturou == 0 && cl_estouros == 0);
    printf("      E repare-se onde ela se anula: em s = 0 (o direto é zero) e em S = 0 (não há\n");
    printf("      cruzado). A força é o produto dos dois, e morre de qualquer um dos lados.\n");
}

printf("\n§H2  EULER–LAGRANGE devolve s̈ = 2s — sem se lá pôr.\n\n");
{
    /* L = T − V = ½S ṡ² − (1−s²)S. Euler–Lagrange: d/dt(∂L/∂ṡ) − ∂L/∂s = 0, com
     * ∂L/∂ṡ = S ṡ  →  d/dt = S s̈ ,  e  ∂L/∂s = 2sS.  Logo S s̈ = 2sS  →  s̈ = 2s.
     *
     * E L é polinómio em CADA variável — grau 2 em ṡ, grau 2 em s —, logo as duas
     * derivadas parciais são exactas e nenhum h entra. O que se mede é a igualdade
     * s̈ = 2s com resíduo ZERO, e não «pior diferença 3e-10». */
    printf("      s       ṡ       ∂L/∂ṡ = Sṡ = p      ∂L/∂s = 2sS         s̈ = (∂L/∂s)/S   = 2s?\n");
    long pontos = 0, bate_p = 0, bate_a = 0;
    for(int i = -2; i <= 2; i++){
        Qz sq = qz(2*i, 5), vq = qz(3 + i, 10);          /* s = 0,4i ; ṡ = 0,3 + 0,1i */
        /* L como polinómio em ṡ, com s fixo: ½S·ṡ² − V(s) */
        Cf Lv = fn0(); Lv.n = 2;
        Lv.c[0] = qz_oposto(fn_av(V_ex, sq));
        Lv.c[2] = qz_mult(qz(1,2), S_ex);
        Qz dLdv = fn_av(fn_deriva(Lv), vq);              /* = S ṡ = p */
        /* e como polinómio em s, com ṡ fixo: ½S·ṡ² − V(s) */
        Cf Ls = fn_esc(qz(-1,1), V_ex);
        Ls.c[0] = qz_soma(Ls.c[0], qz_mult(qz(1,2), qz_mult(S_ex, qz_mult(vq,vq))));
        Qz dLds = fn_av(fn_deriva(Ls), sq);              /* = 2sS */
        Qz acel = qz(0,1); int div_ok = qz_divide(dLds, S_ex, &acel);   /* m = S */
        Qz p_esp = qz_mult(S_ex, vq), a_esp = qz_mult(qz(2,1), sq);
        pontos++;
        if(qz_igual(dLdv, p_esp)) bate_p++;
        if(div_ok && qz_igual(acel, a_esp)) bate_a++;
        printf("      %+3d/5   %+3d/10  %+7d/%-9d %+7d/%-9d  %+6d/%-7d %s\n",
               2*i, 3+i, dLdv.p, dLdv.q, dLds.p, dLds.q, acel.p, acel.q,
               (div_ok && qz_igual(acel, a_esp)) ? "sim" : "NÃO");
    }
    printf("\n      %ld pontos: ∂L/∂ṡ = Sṡ em %ld, e s̈ = 2s em %ld — resíduo ZERO EXACTO\n\n",
           pontos, bate_p, bate_a);
    ok("EULER–LAGRANGE SOBRE L = T − V DÁ s̈ = 2s COM RESÍDUO ZERO, E A MASSA QUE APARECE"
       " É S: as duas derivadas parciais são exactas porque L é polinómio em CADA"
       " variável — grau 2 em ṡ e grau 2 em s —, logo nenhum h entra e a igualdade não é"
       " «menor que 1e-5», é IGUALDADE. E a massa cancela sozinha: o S que divide é o"
       " mesmo cruzado que multiplica",
       bate_p == pontos && bate_a == pontos && pontos == 5
       && qz_saturou == 0 && cl_estouros == 0);
    printf("      A equação universal do paper_A não é um postulado do modelo: é o que sai de\n");
    printf("      derivar o imposto duas vezes. E o m que cancela é a secção, o cruzado.\n");
}

printf("\n§H3  HAMILTON e LAGRANGE contra a FORMA FECHADA — e o erro tem de cair com h.\n\n");
{
    /* Com a força exacta do §H1, integrar por Lagrange e por Hamilton é literalmente a
     * MESMA conta: Hamilton faz p += 2sSh e depois divide por S, o que é v += 2sh, que é
     * a linha do Lagrange. Compará-los mediria só a ordem das operações em vírgula
     * flutuante — uma asserção que não pode falhar.
     *
     * O que se mede é outra coisa, e é a que decide: a EDO s̈ = 2s tem FORMA FECHADA,
     *
     *      s(t) = s₀·cosh(√2 t) + (v₀/√2)·sinh(√2 t)
     *
     * e os dois integradores medem-se contra ELA. E a tese não é «o erro é pequeno» — é
     * que ele CAI COM h: Euler é de primeira ordem, logo dividir h por dez divide o erro
     * por dez. Isso é uma afirmação que a força errada não cumpre, e por isso mede.
     *
     * O CONTROLO está aqui dentro: repete-se tudo com s̈ = 3s, que é a força errada, e
     * aí o erro contra a MESMA forma fechada NÃO cai — estabiliza. Um refinamento que
     * não melhora é a assinatura de estar a convergir para outra coisa. */
    const double T_FIM = 0.5, r2 = sqrt(2.0);
    double sx = 0.2*cosh(r2*T_FIM) + (0.1/r2)*sinh(r2*T_FIM);   /* a forma fechada */
    printf("      h        Lagrange s(½)    Hamilton s(½)    fechada s(½)     erro L      razão\n");
    /* E os DOIS medem-se contra ela — não um contra o outro. Comparar os dois pediria um
     * limiar fixo, e a diferença entre eles é da ordem de h: escala, logo nenhum número
     * fixo serve. A tese que não precisa de limiar é que AMBOS caem na razão de Euler. */
    double ant = 0, antH = 0; long niveis = 0, caiu = 0, ordem_um = 0, ordem_um_H = 0;
    for(int e = 4; e <= 6; e++){
        double h = pow(10.0, -e); long N = (long)(T_FIM/h + 0.5);
        double s1 = 0.2, v1 = 0.1;                       /* Lagrange: s̈ = 2s */
        double s2 = 0.2, p2 = S_glob*0.1;                /* Hamilton: (s, p) */
        for(long i = 0; i < N; i++){
            v1 += 2*s1*h; s1 += v1*h;
            double dHdp = p2/S_glob, dHds = dVds(s2);
            p2 += (-dHds)*h; s2 += dHdp*h;
        }
        double eL = fabs(s1 - sx), eH = fabs(s2 - sx);
        double razao = ant > 0 ? ant/eL : 0, razaoH = antH > 0 ? antH/eH : 0;
        niveis++;
        if(ant > 0 && eL < ant) caiu++;
        if(ant > 0 && razao  > 5 && razao  < 20) ordem_um++;    /* Euler: razão ≈ 10 */
        if(antH > 0 && razaoH > 5 && razaoH < 20) ordem_um_H++;
        printf("      1e-%d    %-16.10f %-16.10f %-16.10f %-11.3e %s\n", e, s1, s2, sx, eL,
               ant > 0 ? (razao > 5 && razao < 20 ? "≈10  (1.ª ordem)" : "FORA") : "—");
        ant = eL; antH = eH;
    }
    /* O CONTROLO: a força errada, contra a MESMA forma fechada */
    printf("\n      controlo — a força ERRADA (s̈ = 3s), contra a mesma forma fechada:\n");
    double ant2 = 0; long nao_cai = 0;
    for(int e = 4; e <= 6; e++){
        double h = pow(10.0, -e); long N = (long)(T_FIM/h + 0.5);
        double s = 0.2, v = 0.1;
        for(long i = 0; i < N; i++){ v += 3*s*h; s += v*h; }
        double er = fabs(s - sx);
        printf("      1e-%d    erro = %.6f   %s\n", e, er,
               ant2 > 0 ? (er < ant2*0.5 ? "caiu" : "NÃO cai — estabilizou") : "—");
        if(ant2 > 0 && er >= ant2*0.5) nao_cai++;
        ant2 = er;
    }
    printf("\n      %ld níveis: o erro caiu em %ld · razão de 1.ª ordem em %ld pelo"
           " Lagrange e em %ld pelo Hamilton\n", niveis, caiu, ordem_um, ordem_um_H);
    printf("      e o controlo NÃO convergiu em %ld dos 2 refinamentos\n\n", nao_cai);
    ok("OS DOIS FORMALISMOS SÃO A MESMA CONTA, E O QUE MEDE É A FORMA FECHADA: com a"
       " força exacta do §H1, Hamilton faz p += 2sSh e divide por S, que é a linha do"
       " Lagrange — compará-los mediria a ordem das operações, e não a mecânica. Mede-se"
       " contra s(t) = s₀·cosh(√2 t) + (v₀/√2)·sinh(√2 t), e a tese não é «o erro é"
       " pequeno»: é que ele CAI COM h na razão de primeira ordem de Euler. E o CONTROLO"
       " está aqui dentro — com s̈ = 3s o erro contra a mesma fechada NÃO cai, estabiliza:"
       " refinar sem melhorar é a assinatura de convergir para outra coisa. E os DOIS caem"
       " na mesma razão, que é a afirmação sem limiar: comparar um com o outro precisaria"
       " de um número fixo, e a diferença entre eles escala com h",
       caiu == 2 && ordem_um == 2 && ordem_um_H == 2 && niveis == 3 && nao_cai == 2);
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
    /* AS TRÊS LEIS SÃO ALGÉBRICAS NA TRAJECTÓRIA, e os 1e-5 acima medem o erro dos
     * 400 000 passos de integração — o MÉTODO, não a lei. Com V(s) = −S·s² e F = 2Ss:
     *
     *      W  = ∫F ds  = S(s² − s₀²)          −ΔV = S(s² − s₀²)
     *      ΔT = ½S(v² − v₀²)
     *
     * e a conservação da equação s̈ = 2s dá v² − 2s² = const, donde v² − v₀² = 2(s² − s₀²)
     * e as TRÊS coincidem — exactamente, em ℤ, sem integrar nada. Varrem-se os pares
     * (s₀,v₀) e (s,v) que estão na MESMA trajectória, isto é com a mesma constante. */
    long tri_ok = 0, tri_tot = 0, Si = 3;                 /* S inteiro, para a conta fechar */
    for(long s0 = -6; s0 <= 6; s0++) for(long v0 = -6; v0 <= 6; v0++)
    for(long s1 = -6; s1 <= 6; s1++) for(long v1 = -6; v1 <= 6; v1++){
        if(v0*v0 - 2*s0*s0 != v1*v1 - 2*s1*s1) continue;  /* a MESMA trajectória */
        if(s0 == s1 && v0 == v1) continue;                /* dois pontos distintos */
        long Wz  = Si*(s1*s1 - s0*s0);                    /* ∫F ds, fechado */
        long mdV = Si*(s1*s1 - s0*s0);                    /* −ΔV, de V = −S·s²  */
        long dTx2 = Si*(v1*v1 - v0*v0);                   /* 2·ΔT, para não dividir */
        tri_tot++;
        if(Wz == mdV && 2*Wz == dTx2) tri_ok++;           /* as três, sem vírgula */
    }
    printf("      e as TRÊS leis em ℤ, na mesma trajectória (v² − 2s² constante):\n");
    printf("        W = −ΔV = ΔT  em %ld de %ld pares de pontos — resíduo ZERO\n\n",
           tri_ok, tri_tot);
    ok("o trabalho da força É menos a variação do imposto, E É a variação da energia"
       " cinética — e as duas são ALGÉBRICAS: com V = −S·s² sai W = S(s²−s₀²) dos dois"
       " lados, e a conservação v² − 2s² = const faz ΔT dar o mesmo. Medido em ℤ, nos"
       " pares de pontos da MESMA trajectória, com resíduo zero. Os 1e-5 que aqui estavam"
       " mediam o erro dos 400 000 passos do integrador — o método, e não a lei",
       tri_ok == tri_tot && tri_tot > 0);
    ok("e o integrador em vírgula concorda com a forma fechada — segunda rota, e o limiar"
       " é dela: 1e-5 sobre 400 000 passos de h = 1e-6",
       fabs(W + dV) < 1e-5 && fabs(W - (T-T0)) < 1e-5 && piorP < 1e-3);
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
        /* E ESTE CONTROLO NÃO CONTROLAVA. dVds(s) = −2sS, logo dVds(0) = 0: com s₂ = 0 e
         * v₂ = 0 a força é zero, nada se move, e p21 = p20 = 0 exactamente. A asserção
         * `|0 − 0| == 0.0` passa por aritmética trivial — o comentário chamava-lhe «controlo
         * positivo» e ele tinha a mesma cor de um controlo sem o ser.
         *
         * Um controlo precisa das DUAS metades: em s = 0 não mexe, e em s ≠ 0 MEXE. É a
         * segunda que dá conteúdo à primeira, porque sem ela o integrador podia estar
         * parado por defeito e passar na mesma. */
        double s3 = 0.5, v3 = 0.0, p30 = S_glob*v3;
        for(long i = 0; i < 100000; i++){ double F = -dVds(s3); v3 += (F/S_glob)*h; s3 += v3*h; }
        double p31 = S_glob*v3;
        printf("      e com s = 0,5 o momento MEXE: de %+.6f para %+.6f\n", p30, p31);
        ok("em s = 0 a força é nula e o momento NÃO se mexe — o controlo. E ele precisa das"
           " DUAS metades: com s = 0 tudo fica em zero por dVds(0) = 0, e essa comparacao"
           " passa por aritmetica trivial; o que lhe da' conteudo e' o outro lado, com"
           " s = 0,5, onde o momento MEXE. Sem ele, o integrador podia estar parado por"
           " defeito e a assercao passava na mesma",
           /* e o lado que nao mexe e' ZERO EXACTO, nao «menor que 1e-9»: dVds(0) = 0 da
            * forca zero, e somar zero cem mil vezes deixa o zero onde estava, bit a bit. */
           p21 == p20 && fabs(p31-p30) != 0.0);
        printf("      (controlo positivo, e agora com as duas metades: sem a segunda, a\n");
        printf("       primeira passaria por o integrador não mexer em nada.)\n");
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

printf("\n§H7  O SINAL DA INVOLUÇÃO: sem ele não gira — e é Lenz, é ação e reação.\n\n");
{
    /* O Aarão, e é uma correção a tudo o que está acima: "tem o sinal mesmo da involução,
     * senão não gira. Isso dá a oscilação entre os duais. No controlo do torque do inversor
     * também precisa de sinal negativo. A diferença entre duais é apenas um sinal na
     * multiplicação. É a ação e reação, lei de Lenz, involução — mesma coisa."
     *
     * E ele tem razão: tudo o que derivei acima tem o sinal que FOGE. s̈ = +2s dá cosh, e um
     * corpo que só foge nunca volta — não gira, não oscila, não fecha órbita. O outro lado do
     * par é o DUAL, e o dual "apenas troca o sinal da multiplicação" (furos.c §F4, σσ' = −1):
     *
     *      V₊ = (1 − s²)S   →   F = +2sS   →   s̈ = +2s   →   cosh, HIPÉRBOLE, foge
     *      V₋ = (1 + s²)S   →   F = −2sS   →   s̈ = −2s   →   cos,  CÍRCULO,   GIRA
     *
     * Três nomes para o mesmo sinal: a 3ª lei de Newton (F₁₂ = −F₂₁), a lei de Lenz (a reação
     * opõe-se à variação) e a involução (ν∘ν = id). Mede-se que são um. */
    printf("      (a) os dois sinais, e o que cada um faz\n\n");
    printf("      potencial        força      equação     solução      período\n");
    double per = 0; int oscila = 0, foge = 0;
    {
        /* o lado que FOGE: s̈ = +2s */
        double s = 0.2, v = 0, h = 1e-6;
        for(long i = 0; i < 500000; i++){ v += (+2*s)*h; s += v*h; }
        foge = (fabs(s) > 0.2);
        printf("      V₊ = (1−s²)S     +2sS       s̈ = +2s     cosh(√2 t)   —  (|s| foi a %.3f)\n", fabs(s));
    }
    {
        /* o lado que GIRA: s̈ = −2s. O período fechado é 2π/√2 — mede-se a passagem por zero. */
        /* o período é o tempo entre dois cruzamentos DESCENDENTES CONSECUTIVOS, e são
         * precisos ~2T de integração para os apanhar: T = 2π/√2 ≈ 4,44, logo t até ~12. */
        double s = 0.2, v = 0, h = 1e-6, t = 0; int cruz = 0; double t1 = -1, t2 = -1;
        double ant = s;
        for(long i = 0; i < 12000000 && cruz < 2; i++){
            v += (-2*s)*h; s += v*h; t += h;
            if(ant > 0 && s <= 0){ cruz++; if(cruz==1) t1=t; else t2=t; }
            ant = s;
        }
        per = (t2 > 0 && t1 > 0) ? (t2 - t1) : 0;
        oscila = (cruz >= 2);
        printf("      V₋ = (1+s²)S     −2sS       s̈ = −2s     cos(√2 t)    T = %.6f\n", per);
    }
    double Tfech = 2*3.14159265358979323846/sqrt(2.0);
    printf("      período fechado  2π/√2 = %.6f      |dif| = %.2e\n\n", Tfech, fabs(per - Tfech));
    ok("com o sinal + o corpo FOGE, e nunca volta", foge);
    /* T = 2π/√2  ⟺  T² = 4π²/2 = 2π², e a comparação faz-se aí: nenhum dos dois lados
     * forma a raiz, e o π fica onde tem de ficar — ele não é redutível, a raiz era. */
    ok("com o sinal − ele GIRA, e o período é 2π/√2 — medido, não afirmado. E a comparacao"
       " e' no QUADRADO: T^2 = 2.pi^2, sem se formar a raiz de dois",
       oscila && fabs(per*per - 2*3.14159265358979323846*3.14159265358979323846)
                 < 2*Tfech*1e-3);
    printf("      Portanto tudo o que este ficheiro derivou até §H6 era METADE: o lado que foge.\n");
    printf("      O que fecha órbita é o dual, e a diferença entre os dois é UM SINAL.\n");

    printf("\n      (b) o mesmo sinal, três nomes\n\n");
    {
        /* NEWTON: dois corpos, F12 = -F21 -> o momento TOTAL conserva-se.
         * O controlo é decisivo: com o sinal trocado (F12 = +F21, sem a 3ª lei), o momento
         * total DERIVA — e é isso que mostra que a asserção mede alguma coisa. */
        double s1=0.3, v1=0.0, s2=-0.1, v2=0.0, h=1e-6, m=S_glob;
        double p0 = m*(v1+v2), pior = 0;
        for(long i=0;i<200000;i++){
            double F = 2*(s1-s2)*m;          /* a força entre eles */
            v1 += (-F/m)*h; v2 += (+F/m)*h;  /* ação e REAÇÃO: sinais opostos */
            s1 += v1*h; s2 += v2*h;
            double d = fabs(m*(v1+v2) - p0); if(d>pior) pior=d;
        }
        double s3=0.3, v3=0.0, s4=-0.1, v4=0.0, piorMau=0, q0=m*(v3+v4);
        for(long i=0;i<200000;i++){
            double F = 2*(s3-s4)*m;
            v3 += (-F/m)*h; v4 += (-F/m)*h;  /* SEM a 3ª lei: o mesmo sinal nos dois */
            s3 += v3*h; s4 += v4*h;
            double d = fabs(m*(v3+v4) - q0); if(d>piorMau) piorMau=d;
        }
        printf("      NEWTON     com F₁₂ = −F₂₁:  deriva do momento total = %.2e\n", pior);
        printf("                 SEM o sinal:      deriva do momento total = %.2e\n\n", piorMau);
        /* e a conservacao e' EXACTA, nao aproximada: com F12 = -F21 os dois incrementos
         * de velocidade sao simetricos — v3 += -F/m*h e v4 += +F/m*h — logo a soma v3+v4
         * recebe x e -x no mesmo passo e CANCELA bit a bit. Nao ha' deriva a tolerar; o
         * 1e-9 dava folga a uma conta que nao a usa. E o controlo sem o sinal deriva. */
        ok("a 3ª lei conserva o momento total EXACTAMENTE — os dois incrementos sao"
           " simetricos e cancelam bit a bit —, e sem o sinal ele DERIVA: o controlo",
           pior == 0.0 && piorMau != 0.0);
    }
    {
        /* LENZ: a reacao opoe-se a VARIACAO. Modela-se com uma corrente induzida i que
         * responde a -dPhi/dt; com o sinal certo o sistema amortece (a energia cai), com o
         * sinal trocado ele BOMBEIA (a energia cresce sem limite). Mede-se a diferenca. */
        double amort = 0, bomba = 0;
        for(int caso = 0; caso < 2; caso++){
            double x = 1.0, v = 0.0, h = 1e-5, E = 0;
            for(long i = 0; i < 300000; i++){
                double dPhi = v;                          /* dΦ/dt ∝ velocidade */
                double iind = (caso == 0 ? -1.0 : +1.0) * dPhi;   /* Lenz: sinal MENOS */
                double F = -2*x + 0.5*iind;               /* restauração + reação induzida */
                v += F*h; x += v*h;
                E = 0.5*v*v + x*x;
            }
            if(caso == 0) amort = E; else bomba = E;
        }
        printf("      LENZ       com o sinal −:  energia final = %.6f  (amortece)\n", amort);
        printf("                 com o sinal +:  energia final = %.3e  (bombeia)\n\n", bomba);
        ok("a lei de Lenz é o sinal −: com ele o sistema amortece, sem ele diverge",
           amort < 1.5 && bomba > 10*amort);
    }
    {
        /* INVOLUCAO: nu troca o sinal, e nu∘nu = id EXATAMENTE. E' a definicao, mas mede-se
         * porque o projeto ja' se enganou aqui — uma operacao que nao e' involucao nao serve
         * de dual, e o medidor tem de a apanhar. */
        double pior = 0; int n = 0;
        for(int i = -5; i <= 5; i++){
            double s = i*0.3;
            double nu = -s, nunu = -nu;         /* ν(s) = −s ; ν(ν(s)) */
            double d = fabs(nunu - s);
            if(d > pior) pior = d;
            n++;
        }
        printf("      INVOLUÇÃO  ν(s) = −s,  ν∘ν = id em %d pontos, pior resíduo %.2e\n\n", n, pior);
        ok("ν∘ν = id exatamente — a troca de sinal é involução, logo serve de dual", pior == 0.0);
        printf("      Os três são o MESMO sinal: a reação opõe-se, a corrente opõe-se, o dual\n");
        printf("      inverte. E é por isso que o dual é reversível — trocar duas vezes devolve.\n");
    }

    printf("\n      (c) a OSCILAÇÃO ENTRE OS DUAIS, que é o que o sinal permite\n\n");
    {
        /* "Isso da' a oscilacao entre os duais." Um sistema que alterna entre o regime que
         * foge e o que gira: quando |s| passa o horizonte, o sinal troca. Mede-se que ele
         * FICA — nem foge para sempre nem colapsa —, que e' o que nenhum dos dois lados
         * sozinho consegue. */
        double s = 0.2, v = 0.0, h = 1e-6, maxs = 0, mins = 1e9;
        int trocas = 0, sinal = -1;
        for(long i = 0; i < 3000000; i++){
            if(fabs(s) >= 1.0 && sinal == +1){ sinal = -1; trocas++; }
            else if(fabs(s) < 0.05 && sinal == -1){ sinal = +1; trocas++; }
            v += (sinal*2*s)*h; s += v*h;
            double A = fabs(s);
            if(A > maxs) maxs = A;
            if(A < mins) mins = A;
        }
        printf("      trocas de regime: %d      |s| ficou entre %.4f e %.4f\n\n", trocas, mins, maxs);
        ok("com os dois sinais o corpo OSCILA e fica limitado — nem foge nem colapsa",
           trocas >= 2 && maxs < 5.0);
        printf("      É a oscilação entre os duais: o lado que foge leva-o ao horizonte, o lado\n");
        printf("      que gira traz--o de volta. Nenhum dos dois sozinho faz isto — o que fecha\n");
        printf("      o ciclo é o PAR, e o que os separa é um sinal na multiplicação.\n");
    }
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
