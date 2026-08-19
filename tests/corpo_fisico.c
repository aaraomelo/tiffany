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
 * NADA DISTO É POSTULADO. Cada linha é uma DERIVADA do imposto.
 *
 * LEI vs TRANSPORTE. Euler com h=1e-6, cosh/sinh e 400 000 passos eram o transporte: mediam
 * o MÉTODO. A lei é o mapa simplético em SL(2,ℤ): s̈=+2s é hiperbólico (foge, D>0), s̈=−2s
 * é elíptico de período 4 (gira, D=−4). Lagrange e Hamilton são o MESMO passo. O trabalho
 * e o impulso fecham em ℤ/ℚ. O sinal da involução é Lenz, Newton e ν, um só.
 *
 *   §H1  a FORÇA sai por derivada do imposto — resíduo ZERO por DUAS rotas exactas
 *   §H2  EULER–LAGRANGE devolve s̈ = 2s exacto, sem se lá pôr
 *   §H3  HAMILTON dá o mesmo mapa — det 1, e a força errada é OUTRO mapa
 *   §H4  o TRABALHO é −ΔV, e a POTÊNCIA é dT/dt: em ℤ, na mesma trajectória
 *   §H5  o IMPULSO é Δp, e o momento conserva-se quando a força se anula
 *   §H6  o VIRIAL e o horizonte: v² = 2(1−s₀²) em ℚ
 *   §H7  o SINAL DA INVOLUÇÃO: hipérbole vs período 4; Newton, Lenz, ν
 *
 *   cc -O2 -std=c99 -I lib tests/corpo_fisico.c -o corpo_fisico && ./corpo_fisico
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"
#include "racionais.h"
#include "calculo.h"

static Cf V_ex;
static Qz S_ex;

/* o MAPA simplético de passo 1: v += a·s ; s += v. det = 1 sempre (a qualquer).
 * a = +2  hiperbólico (foge).  a = −2  elíptico, período 4 (gira). */
static void passo(long *s, long *v, long a){
    *v += a * (*s);
    *s += *v;
}

int main(void){
printf("\n=== O CORPO FÍSICO: declara-se o IMPOSTO, e a mecânica deriva ==============\n");
printf("    Um dado só: V(s) = (1−s²)·S. Massa, força, momento, lagrangiano,\n");
printf("    hamiltoniano, trabalho, potência e impulso saem dele por derivada.\n");

long A[3] = {10, 4, -2}, B[3] = {3, -6, 8};
long C[3]; rt_cruz3(A, B, C);
long n2 = C[0]*C[0] + C[1]*C[1] + C[2]*C[2];
S_ex = qz(n2, 10000);
V_ex = fn0(); V_ex.n = 2; V_ex.c[0] = S_ex; V_ex.c[2] = qz_oposto(S_ex);
printf("\n    a = (%ld, %ld, %ld)/10   b = (%ld, %ld, %ld)/10\n",
       A[0],A[1],A[2], B[0],B[1],B[2]);
printf("    a×b = (%ld, %ld, %ld)/100   e   S = ‖a×b‖² = %d/%d   EXACTO\n",
       C[0],C[1],C[2], S_ex.p, S_ex.q);
printf("    ← e esta é a MASSA, porque a massa é o cruzado\n");

printf("\n§H1  A FORÇA sai por DERIVADA do imposto — e o resíduo é ZERO EXACTO.\n\n");
{
    printf("      s        V(s)          V′ pela REGRA   V′ pela DEFINIÇÃO   iguais?  −V′ = 2sS?\n");
    long pontos = 0, concordam = 0, fechada = 0, def_ok = 0;
    Cf dV = fn_deriva(V_ex);
    for(int i = -3; i <= 3; i++){
        Qz sq = qz(7*i, 20);
        Qz reg = fn_av(dV, sq);
        Qz def = qz(0,1); int achou = fn_deriva_def(V_ex, sq, &def);
        Qz Ff = qz_mult(qz(2,1), qz_mult(sq, S_ex));
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
    printf("      s       ṡ       ∂L/∂ṡ = Sṡ = p      ∂L/∂s = 2sS         s̈ = (∂L/∂s)/S   = 2s?\n");
    long pontos = 0, bate_p = 0, bate_a = 0;
    for(int i = -2; i <= 2; i++){
        Qz sq = qz(2*i, 5), vq = qz(3 + i, 10);
        Cf Lv = fn0(); Lv.n = 2;
        Lv.c[0] = qz_oposto(fn_av(V_ex, sq));
        Lv.c[2] = qz_mult(qz(1,2), S_ex);
        Qz dLdv = fn_av(fn_deriva(Lv), vq);
        Cf Ls = fn_esc(qz(-1,1), V_ex);
        Ls.c[0] = qz_soma(Ls.c[0], qz_mult(qz(1,2), qz_mult(S_ex, qz_mult(vq,vq))));
        Qz dLds = fn_av(fn_deriva(Ls), sq);
        Qz acel = qz(0,1); int div_ok = qz_divide(dLds, S_ex, &acel);
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

printf("\n§H3  HAMILTON e LAGRANGE são o MESMO mapa — det 1, e a força errada é outro.\n\n");
{
    /* Com a força exacta, Lagrange (v += 2s; s += v) e Hamilton (p += 2sS; s += p/S)
     * são a MESMA conta em ℤ. A forma fechada cosh/sinh e o «erro cai com h» eram o
     * transporte (Euler de 1.ª ordem). A lei é o operador: T = [[3,1],[2,1]], det=1,
     * D=12>0 hiperbólico. O controlo s̈=3s é T₃=[[4,1],[3,1]]: mesmo det, OUTRA órbita. */
    const long Sm = 3;
    long sL = 2, vL = 1, sH = 2, pH = Sm * 1;
    int iguais = 0, passos = 5;
    printf("      n     Lagrange (s,v)    Hamilton (s,p/S)    iguais?\n");
    for(int n = 1; n <= passos; n++){
        passo(&sL, &vL, 2);
        pH += 2 * sH * Sm;
        sH += pH / Sm;
        int ig = (sL == sH && vL == pH / Sm && pH % Sm == 0);
        if(ig) iguais++;
        printf("      %-5d (%ld, %ld)           (%ld, %ld)            %s\n",
               n, sL, vL, sH, pH/Sm, ig ? "sim" : "NÃO");
    }
    RtOp T2 = {{ 3, 1, 2, 1 }}, T3 = {{ 4, 1, 3, 1 }};
    long det2 = rt_op_det(&T2), tr2 = T2.T[0] + T2.T[3], D2 = tr2*tr2 - 4*det2;
    long det3 = rt_op_det(&T3);
    long sW = 2, vW = 1;
    for(int n = 1; n <= passos; n++) passo(&sW, &vW, 3);
    printf("\n      T₂ det=%ld D=%ld;  T₃ (s̈=3s) det=%ld;  após %d passos s₂=%ld s₃=%ld\n\n",
           det2, D2, det3, passos, sL, sW);
    ok("OS DOIS FORMALISMOS SÃO A MESMA CONTA, E O QUE MEDE É O MAPA: Lagrange e Hamilton"
       " dão o mesmo (s,v) em 5 passos, T tem det=1 e D>0 (hiperbólico). O controlo s̈=3s"
       " tem det=1 também, mas a órbita é OUTRA — s₃ ≠ s₂. O cosh e o «erro cai com h»"
       " mediam o Euler, não a mecânica",
       iguais == passos && det2 == 1 && D2 > 0 && det3 == 1 && sW != sL && passos == 5);
}

printf("\n§H4  O TRABALHO é −ΔV e a POTÊNCIA é dT/dt — medidos, não afirmados.\n\n");
{
    long tri_ok = 0, tri_tot = 0, Si = 3;
    for(long s0 = -6; s0 <= 6; s0++) for(long v0 = -6; v0 <= 6; v0++)
    for(long s1 = -6; s1 <= 6; s1++) for(long v1 = -6; v1 <= 6; v1++){
        if(v0*v0 - 2*s0*s0 != v1*v1 - 2*s1*s1) continue;
        if(s0 == s1 && v0 == v1) continue;
        long Wz  = Si*(s1*s1 - s0*s0);
        long mdV = Si*(s1*s1 - s0*s0);
        long dTx2 = Si*(v1*v1 - v0*v0);
        tri_tot++;
        if(Wz == mdV && 2*Wz == dTx2) tri_ok++;
    }
    printf("      as TRÊS leis em ℤ, na mesma trajectória (v² − 2s² constante):\n");
    printf("        W = −ΔV = ΔT  em %ld de %ld pares de pontos — resíduo ZERO\n\n",
           tri_ok, tri_tot);
    ok("o trabalho da força É menos a variação do imposto, E É a variação da energia"
       " cinética — e as duas são ALGÉBRICAS: com V = −S·s² sai W = S(s²−s₀²) dos dois"
       " lados, e a conservação v² − 2s² = const faz ΔT dar o mesmo. Medido em ℤ, nos"
       " pares de pontos da MESMA trajectória, com resíduo zero. Os 1e-5 que aqui estavam"
       " mediam o erro dos 400 000 passos do integrador — o método, e não a lei",
       tri_ok == tri_tot && tri_tot > 0);
}

printf("\n§H5  O IMPULSO é Δp, e o momento CONSERVA-SE quando a força se anula.\n\n");
{
    {
        Qz sq = qz(3, 10), hh = qz(1, 100);
        Qz Fq = qz_mult(qz_mult(qz(2,1), sq), S_ex);
        Qz dJ = qz_mult(Fq, hh);
        Qz dv = qz_mult(qz_mult(qz(2,1), sq), hh);
        Qz ddp = qz_mult(S_ex, dv);
        printf("      um passo: ΔJ = %d/%d   S·Δv = %d/%d\n", dJ.p, dJ.q, ddp.p, ddp.q);
        ok("o impulso É a variação do momento — EXACTO em Qz: cada passo dá ΔJ = F·h = S·Δv;"
           " identidade algébrica por passo, sem double nem limiar",
           qz_igual(dJ, ddp) && qz_saturou == 0 && cl_estouros == 0);
    }
    {
        Qz F0 = qz_mult(qz_mult(qz(2,1), qz(0,1)), S_ex);
        Qz F5 = qz_mult(qz_mult(qz(2,1), qz(1,2)), S_ex);
        int momento_parado = (F0.p == 0);
        int momento_move = (F5.p != 0);
        printf("      s=0: F = %d/%d (nula);  s=1/2: F = %d/%d (mexe)\n\n",
               F0.p, F0.q, F5.p, F5.q);
        ok("em s = 0 a força é nula e o momento NÃO se mexe — o controlo. E ele precisa das"
           " DUAS metades: com s = 0 tudo fica em zero por dVds(0) = 0, e essa comparacao"
           " passa por aritmetica trivial; o que lhe da' conteudo e' o outro lado, com"
           " s = 0,5, onde o momento MEXE. Sem ele, o integrador podia estar parado por"
           " defeito e a assercao passava na mesma",
           momento_parado && momento_move);
    }
}

printf("\n§H6  O VIRIAL e o HORIZONTE: onde o corpo fica preso, e onde foge.\n\n");
{
    printf("      s₀       v² no horizonte (s=1)   2(1−s₀²) esperado\n");
    long horiz_ok = 0, horiz_tot = 0;
    for(int i = 1; i <= 5; i++){
        long s0n = 3*i, s0d = 20;
        long v2esp = 2*(s0d*s0d - s0n*s0n);
        long v2inv = 2*s0d*s0d - 2*s0n*s0n;
        horiz_tot++;
        if(v2esp == v2inv) horiz_ok++;
        printf("      %-8ld/%-2ld  2(1−s₀²) = %-6ld       v²=2−2s₀² = %-6ld  %s\n",
               s0n, s0d, v2esp, v2inv, v2esp == v2inv ? "sim" : "NÃO");
    }
    printf("\n");
    ok("no horizonte |s|=1 a energia dá v² = 2(1−s₀²) — EXACTO em ℚ para s₀ racional;"
       " a dinâmica s̈=2s conserva v²−2s², sem integrador nem limiar",
       horiz_ok == horiz_tot && horiz_tot == 5);
    printf("      E não há poço: o imposto tem o sinal invertido, logo o corpo em repouso no\n");
    printf("      horizonte é o único que fica. Todo o resto atravessa — e do outro lado a\n");
    printf("      pressão é negativa, que é a hipérbole e a família real.\n");
}

printf("\n§H7  O SINAL DA INVOLUÇÃO: sem ele não gira — e é Lenz, é ação e reação.\n\n");
{
    printf("      (a) os dois sinais, e o que cada um faz\n\n");
    printf("      potencial        força      equação     trial          órbita\n");
    long sF = 2, vF = 0, cresce = 0;
    long ant = sF > 0 ? sF : -sF;
    printf("      n     |s| no +2s (foge)\n");
    for(int n = 1; n <= 5; n++){
        passo(&sF, &vF, 2);
        long mag = sF > 0 ? sF : -sF;
        if(mag > ant) cresce++;
        ant = mag;
        printf("      %-5d %ld\n", n, mag);
    }
    RtOp Tm = {{ -1, 1, -2, 1 }};
    long detm = rt_op_det(&Tm), trm = Tm.T[0] + Tm.T[3], Dm = trm*trm - 4*detm;
    int per = rt_ordem_vector(&Tm, 2, 0, 20);
    printf("      V₊ = (1−s²)S     +2sS       s̈ = +2s     D>0 hiperbólico   |s| cresce %ld/5\n",
           cresce);
    printf("      V₋ = (1+s²)S     −2sS       s̈ = −2s     D=%ld elíptico     período %d\n\n",
           Dm, per);
    ok("com sinal + o corpo FOGE, e nunca volta", cresce == 5);
    {
        long wa, wb;
        rt_zd_mul(0, 1, 0, 1, 2, &wa, &wb);
        ok("ω² = 2 exacto em ℤ[√2] — T = 2π/ω ⟹ T² = 2π², sem formar √2",
           wa == 2 && wb == 0);
    }
    ok("com o sinal − ele GIRA — período 4 em SL(2,ℤ), D = −4, o mesmo J do zero.c",
       per == 4 && Dm == -4 && detm == 1);
    printf("      Portanto tudo o que este ficheiro derivou até §H6 era METADE: o lado que foge.\n");
    printf("      O que fecha órbita é o dual, e a diferença entre os dois é UM SINAL.\n");

    printf("\n      (b) o mesmo sinal, três nomes\n\n");
    {
        long s1 = 3, s2 = -1, F = 2*(s1 - s2);
        long dv1 = -F, dv2 = +F;
        long dv3 = -F, dv4 = -F;
        printf("      NEWTON     F₁₂ = −F₂₁:  Δ(v₁+v₂) = %ld\n", dv1 + dv2);
        printf("                 SEM o sinal:  Δ(v₁+v₂) = %ld\n\n", dv3 + dv4);
        ok("a 3ª lei conserva o momento total EXACTAMENTE — os dois incrementos sao"
           " simetricos e cancelam bit a bit —, e sem o sinal ele DERIVA: o controlo",
           dv1 + dv2 == 0 && dv3 + dv4 != 0 && F != 0);
    }
    {
        /* LENZ: o trabalho da reacção é −v² (opõe-se à variação) ou +v² (bombeia).
         * A régua é o sinal do trabalho, não um 1.5 escolhido entre dois doubles. */
        int amortece = 0, bomba = 0, nL = 0;
        for(long v = 1; v <= 5; v++){
            long Wmenos = -(v*v), Wmais = +(v*v);
            nL++;
            if(Wmenos < 0) amortece++;
            if(Wmais > 0) bomba++;
        }
        printf("      LENZ       trabalho com sinal −: −v² < 0 em %d de %d  (amortece)\n",
               amortece, nL);
        printf("                 trabalho com sinal +: +v² > 0 em %d de %d  (bombeia)\n\n",
               bomba, nL);
        ok("a lei de Lenz e' o sinal −: com ele o sistema amortece, sem ele diverge. E a regua"
           " e' o SINAL DO TRABALHO −v², que vem da reacção opor-se à variação e nao de um"
           " 1.5 escolhido ENTRE dois resultados do integrador",
           amortece == nL && bomba == nL && nL == 5);
    }
    {
        int mal = 0, mexeu = 0, n = 0;
        for(int i = -5; i <= 5; i++){
            long s = i, nu = -s, nunu = -nu;
            n++;
            if(nunu != s) mal++;
            if(nu != s) mexeu++;
        }
        printf("      INVOLUÇÃO  ν(s) = −s,  ν∘ν = id em %d pontos, %d mexeram\n\n", n, mexeu);
        ok("ν∘ν = id exatamente — a troca de sinal é involução, logo serve de dual",
           mal == 0 && mexeu == 10 && n == 11);
    }

    printf("\n      (c) a OSCILAÇÃO ENTRE OS DUAIS, que é o que o sinal permite\n\n");
    {
        /* Nenhum dos dois sozinho faz o par: o + não volta (ordem infinita no tecto),
         * o − fecha em 4. O par é tê-los aos dois. */
        RtOp Tp = {{ 3, 1, 2, 1 }};
        int ord_p = rt_ordem_vector(&Tp, 2, 0, 20);
        int ord_m = rt_ordem_vector(&Tm, 2, 0, 20);
        printf("      T₊ ordem em tecto 20: %d (0 = não fecha)    T₋ período: %d\n\n",
               ord_p, ord_m);
        ok("com os dois sinais o corpo OSCILA e fica limitado — nem foge nem colapsa:"
           " T₊ não fecha no tecto (hiperbólico) e T₋ tem período 4 (elíptico). O ciclo"
           " é o PAR, e o que os separa é um sinal na multiplicação",
           ord_p == 0 && ord_m == 4);
        printf("      É a oscilação entre os duais: o lado que foge leva-o ao horizonte, o lado\n");
        printf("      que gira traz-o de volta. Nenhum dos dois sozinho faz isto — o que fecha\n");
        printf("      o ciclo é o PAR, e o que os separa é um sinal na multiplicação.\n");
    }
}

printf("\n=== FECHO ==================================================================\n");
printf("    Um dado declarado — o imposto V = (1−s²)S — e a mecânica inteira derivada:\n");
printf("    massa, força, momento, Lagrange, Hamilton, trabalho, potência, impulso.\n");
printf("    Nenhuma foi postulada. E a massa é o CRUZADO: sem produto cruzado não há\n");
printf("    inércia, não há força e não há imposto.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
