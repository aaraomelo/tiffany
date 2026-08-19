/* polar.c — A FORMA ALGÉBRICA E A POLAR DO CORPO UNIVERSAL, e são a mesma lei três vezes.
 *
 * O Aarão: "vale rever a forma algébrica e polar do corpo universal. A polar entra no painel, vai
 * ser muito usada. A cartesiana também."
 *
 * A FORMA ALGÉBRICA é a que o `fecha.c` entrega: z = a + b·σ, com a borda σ² = B·σ − C. É a que
 * soma bem — componente a componente, e é Clifford.
 *
 * A FORMA POLAR é a que multiplica bem: z = ρ · E(θ), onde o produto vira
 *
 *      ρ(zw) = ρ(z)·ρ(w)          os módulos MULTIPLICAM
 *      θ(zw) = θ(z) + θ(w)        os ângulos SOMAM
 *
 * — e é exatamente ∏ = exp∘Σ∘log. **Uma forma para cada operação**, e a ponte é o operador.
 *
 * LEI vs TRANSPORTE. sqrt, atan2, cosh, sinh e 1e-9 sobre ρ e θ eram o método. A lei vive em
 * ℤ[σ] e em ℤ[√Δ]: N(xy)=N(x)N(y), a composição (2u+v√Δ)(2u'+v'√Δ)=2(2u₃+v₃√Δ), o cone
 * P² > Δ v², De Moivre por rt_zd_pot, Lagrange da forma, e o espelho ν. Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/polar.c -o polar && ./polar
 *   ./polar 1 -1 3 2        a régua (B,C) e o ponto (a,b): dá a polar em ℤ
 */
#include <stdio.h>
#include <stdlib.h>
#include "reta.h"
#include "unidade.h"

typedef struct { long a, b; } Alg;

static long DELTA(long B, long C){ return B*B - 4*C; }
static long norma(long B, long C, Alg z){ return z.a*z.a + B*z.a*z.b + C*z.b*z.b; }
static Alg prod(long B, long C, Alg x, Alg y){
    Alg z = { x.a*y.a - C*x.b*y.b, x.a*y.b + x.b*y.a + B*x.b*y.b };
    return z;
}
/* 2u = 2a + B b, v = b.  (2u)² − Δ v² = 4 N. */
static void centro(long B, Alg z, long *P, long *v){
    *P = 2*z.a + B*z.b;
    *v = z.b;
}
static int no_ramo(long B, long C, Alg z){
    long D = DELTA(B, C), N = norma(B, C, z);
    if(N == 0) return 0;
    long P, v; centro(B, z, &P, &v);
    if(D > 0) return P*P > D*v*v;       /* |τ v| < |u|  sem a raiz */
    if(D == 0) return P != 0;
    return 1;                           /* elíptico: todo N ≠ 0 */
}

static void secao_Y1(void){
    printf("\n§Y1  A CENTRAGEM: τ = σ − B/2 dá τ² = Δ/4 — e o regime é o SINAL do Δ\n\n");
    printf("        corpo      (B,C)     Δ      (2τ)² em ℤ[σ]        regime\n");
    struct { const char *n; long B, C; } cs[] = {
        { "ouro",   1, -1 }, { "prata",  2, -1 }, { "i",      0,  1 },
        { "ω",     -1,  1 }, { "PA",     2,  1 }, { "√2",     0, -2 },
    };
    int erros = 0, esc_ok = 0, sig_ok = 0, mult_ok = 0;
    for(int i = 0; i < 6; i += 1){
        long B = cs[i].B, C = cs[i].C, D = DELTA(B, C);
        Alg dois_tau = { -B, 2 };
        Alg q = prod(B, C, dois_tau, dois_tau);
        if(q.a == D) esc_ok += 1; else erros += 1;
        if(q.b == 0)  sig_ok += 1; else erros += 1;
        Alg x = { 3, 2 }, y = { 5, 1 };
        if(norma(B,C,prod(B,C,x,y)) == norma(B,C,x)*norma(B,C,y)) mult_ok += 1; else erros += 1;
        printf("        %-8s  (%2ld,%2ld)  %5ld   (2τ)² = %ld %+ld·σ    %s\n",
               cs[i].n, B, C, D, q.a, q.b,
               D < 0 ? "elíptico  — gira" : D > 0 ? "hiperbólico — estica" : "parabólico — o limite");
    }
    ok("τ² = Δ/4 NOS SEIS CORPOS, E A IDENTIDADE É INTEIRA: escreve-se sem o meio como"
       " (2σ − B)² = Δ, e mede-se pelo produto do próprio corpo — a parte escalar dá"
       " EXACTAMENTE Δ e a parte em σ dá EXACTAMENTE zero, sem uma raiz e sem um limiar."
       " O que aqui estava comparava D/4 com D/4 em metade dos casos. E a norma"
       " multiplicativa N(xy) = N(x)N(y) amarra o produto nos seis",
       erros == 0 && esc_ok == 6 && sig_ok == 6 && mult_ok == 6);
    conclui("o regime não é uma escolha de coordenadas: é o sinal de um número que já estava lá.");
}

static void secao_Y2(void){
    printf("\n§Y2  A LEI: ρ(zw) = ρ(z)ρ(w) e θ(zw) = θ(z)+θ(w) — nos três regimes\n\n");
    printf("        corpo         pares   N(xy)=N(x)N(y)   polar = 2·centro(xy)\n");
    struct { const char *n; long B, C; } cs[] = {
        { "i        Δ<0", 0,  1 },
        { "ouro     Δ>0", 1, -1 },
        { "PA       Δ=0", 2,  1 },
    };
    int falhou = 0;
    for(int i = 0; i < 3; i += 1){
        long B = cs[i].B, C = cs[i].C, D = DELTA(B, C);
        int n = 0, n_ok = 0, p_ok = 0;
        for(long a = 1; a <= 3; a += 1) for(long b = 0; b <= 2; b += 1)
        for(long c = 1; c <= 3; c += 1) for(long d = 0; d <= 2; d += 1){
            Alg x = { a, b }, y = { c, d };
            if(norma(B,C,x) == 0 || norma(B,C,y) == 0) continue;
            Alg xy = prod(B,C,x,y);
            n += 1;
            if(norma(B,C,xy) == norma(B,C,x)*norma(B,C,y)) n_ok += 1;
            long P1, v1, P2, v2, P3, v3, Pa, va;
            centro(B, x, &P1, &v1); centro(B, y, &P2, &v2); centro(B, xy, &P3, &v3);
            rt_zd_mul(P1, v1, P2, v2, D, &Pa, &va);
            if(Pa == 2*P3 && va == 2*v3) p_ok += 1;
        }
        printf("        %-12s  %-6d  %d/%d            %d/%d\n", cs[i].n, n, n_ok, n, p_ok, n);
        if(n_ok != n || p_ok != n || n == 0) falhou += 1;
    }
    ok("a lei vale nos três regimes, sem caso especial — é ∏ = exp∘Σ∘log."
       " ρ multiplica porque N é multiplicativa; θ soma porque (2u+v√Δ) COMPÕE, e o"
       " produto em ℤ[√Δ] é exactamente 2 vezes o centro do produto em ℤ[σ]."
       " atan2/cosh/1e-9 mediam IEEE",
       falhou == 0);

    long B = 1, C = -1, D = DELTA(B, C);
    int cai = 0, tot = 0;
    for(long a = 1; a <= 3; a += 1) for(long b = 1; b <= 2; b += 1){
        Alg x = { a, b }, y = { b, a };
        Alg xy = prod(B,C,x,y);
        long P1, v1, P2, v2, P3, v3, Pa, va, Pw, vw;
        centro(B, x, &P1, &v1); centro(B, y, &P2, &v2); centro(B, xy, &P3, &v3);
        rt_zd_mul(P1, v1, P2, v2, D, &Pa, &va);
        rt_zd_mul(P1, v1, P2, v2, 1, &Pw, &vw);     /* SEM a escala: Δ trocado por 1 */
        tot += 1;
        if(!(Pw == 2*P3 && vw == 2*v3)) cai += 1;
    }
    printf("     sem a escala (Δ trocado por 1), a composição falha em %d de %d\n", cai, tot);
    ok("sem a escala a lei CAI — logo a escala está a fazer trabalho, não é enfeite."
       " atanh(v/u) sem |τ| era o IEEE do ângulo; agora Δ=1 contra Δ=5, em Z",
       cai == tot && tot > 0);
    conclui("uma forma soma bem, a outra multiplica bem, e o operador é a ponte entre as duas.");
}

static void secao_Y3(void){
    printf("\n§Y3  A VOLTA: algébrica → polar → algébrica, com resíduo 0\n\n");
    printf("        corpo        no ramo   fora do ramo   volta (centro) exacta\n");
    struct { const char *n; long B, C; } cs[] = {
        { "i     Δ<0", 0, 1 }, { "ouro  Δ>0", 1, -1 }, { "PA    Δ=0", 2, 1 },
    };
    int falhou = 0, fora_total = 0;
    for(int i = 0; i < 3; i += 1){
        long B = cs[i].B, C = cs[i].C;
        int n = 0, fora = 0, volta = 0;
        for(long a = -3; a <= 3; a += 1) for(long b = -3; b <= 3; b += 1){
            Alg z = { a, b };
            if(!no_ramo(B, C, z)){ fora += 1; continue; }
            long P, v; centro(B, z, &P, &v);
            Alg w = { (P - B*v)/2, v };            /* a volta do centro: exacta em ℤ */
            n += 1;
            if(w.a == z.a && w.b == z.b) volta += 1;
        }
        if(volta != n || n == 0) falhou += 1;
        fora_total += fora;
        printf("        %-11s  %6d   %11d   %d/%d\n", cs[i].n, n, fora, volta, n);
    }
    ok("a volta fecha nos três regimes, dentro do ramo — a polar não perde informação."
       " sqrt/atan2/1e-9 eram a reconstrução trigonométrica; (2u,v) → (a,b) e' a"
       " inversa da centragem, exacta, e (2u − B v) é sempre par",
       falhou == 0);
    ok("e HÁ pontos fora do ramo — a polar não é global, e a algébrica é."
       " Contam-se: elíptico só a origem, hiperbólico o cone, parabólico u=0."
       " fora_total > 0 sobrevivia a tudo; 1+25+7 = 33 é o número",
       fora_total == 33);
    printf("     e o SINAL é a informação a mais: 2 bits, sem os quais a volta erra por 12\n");
    conclui("mudar de forma não é mudar de corpo; mas a polar tem ramo e a algébrica não, e isso é do corpo.");
}

static void secao_Y4(void){
    printf("\n§Y4  A POTÊNCIA É A LEI OUTRA VEZ: ρⁿ e n·θ — De Moivre nos três\n\n");
    printf("        corpo        n     N(z^n)=N(z)^n    (2u+v√Δ)^n = 2^{n-1}·centro\n");
    struct { const char *n; long B, C; long a, b; } cs[] = {
        { "i     Δ<0", 0,  1, 1, 1 },
        { "ouro  Δ>0", 1, -1, 2, 1 },
        { "PA    Δ=0", 2,  1, 1, 1 },
    };
    int falhou = 0;
    for(int i = 0; i < 3; i += 1){
        long B = cs[i].B, C = cs[i].C, D = DELTA(B, C);
        Alg z = { cs[i].a, cs[i].b };
        long P0, v0; centro(B, z, &P0, &v0);
        long Nz = norma(B, C, z);
        Alg w = { 1, 0 };
        int n_ok = 0, p_ok = 0;
        for(int n = 1; n <= 5; n += 1){
            w = prod(B, C, w, z);
            long Pn, vn; centro(B, w, &Pn, &vn);
            long Nw = 1; for(int k = 0; k < n; k += 1) Nw *= Nz;
            if(norma(B, C, w) == Nw) n_ok += 1;
            long A, Vb; rt_zd_pot(P0, v0, D, n, &A, &Vb);
            long fac = 1; for(int k = 1; k < n; k += 1) fac *= 2;
            if(A == fac*Pn && Vb == fac*vn) p_ok += 1;
            if(n == 5) printf("        %-11s  %d    %s              %s\n",
                              cs[i].n, n,
                              n_ok==5 ? "sim" : "nao",
                              p_ok==5 ? "sim" : "nao");
        }
        if(n_ok != 5 || p_ok != 5) falhou += 1;
    }
    ok("ρⁿ e n·θ valem para n = 1..5 nos três regimes — De Moivre é um só."
       " pow/atan2 eram transporte; N(z^n)=N(z)^n e rt_zd_pot contra o centro de z^n",
       falhou == 0);
    conclui("elevar a n é somar o ângulo n vezes. É a mesma lei, e não uma fórmula nova.");
}

static void secao_Y5(void){
    printf("\n§Y5  O QUE O PAINEL USA: cada forma serve uma coisa\n\n");
    printf("        a forma        soma       produto     potência    o que serve\n");
    printf("        ALGÉBRICA      trivial    3 mults     n mults     somar, guardar, comparar\n");
    printf("        POLAR          difícil    1 mult      1 pow       multiplicar, girar, escalar\n\n");
    long B = 1, C = -1, D = DELTA(B, C);
    Alg x = { 3, 2 }, y = { 5, 1 };
    Alg p_alg = prod(B, C, x, y);
    long P1, v1, P2, v2, Pa, va;
    centro(B, x, &P1, &v1); centro(B, y, &P2, &v2);
    rt_zd_mul(P1, v1, P2, v2, D, &Pa, &va);
    /* Pa = 2 P₃, va = 2 v₃,  a = (P₃ − B v₃)/2 = (Pa − B va)/4 */
    Alg p_pol = { (Pa - B*va)/4, va/2 };
    printf("        (3,2) ⊗ (5,1) pela algébrica:  (%ld, %ld)  — exacto, em ℤ[σ]\n",
           p_alg.a, p_alg.b);
    printf("                      pela polar:      (%ld, %ld)  — ℤ[√Δ] a voltar\n",
           p_pol.a, p_pol.b);
    ok("os DOIS caminhos dão o mesmo produto — e é por isso que o painel pode ter as duas."
       " O resíduo 1e-9 era IEEE da volta trigonométrica; agora as duas rotas fecham em Z",
       p_pol.a == p_alg.a && p_pol.b == p_alg.b);
    conclui("o painel mostra as duas porque o piloto usa as duas: uma para pôr, outra para mover.");
}

static void secao_Y6(void){
    printf("\n§Y6  A DUALIDADE FECHADA: algébrica = DIRETO, polar = CRUZADO\n\n");
    printf("        corpo        Lagrange da forma (dir² − Δ cruz² = 4N·4N)   pares\n");
    struct { const char *n; long B, C; } cs[] = {
        { "i     Δ<0", 0, 1 }, { "ouro  Δ>0", 1, -1 }, { "PA    Δ=0", 2, 1 },
    };
    int falhou = 0;
    for(int i = 0; i < 3; i += 1){
        long B = cs[i].B, C = cs[i].C, D = DELTA(B, C);
        int n = 0, lag = 0;
        for(long a = 1; a <= 3; a += 1) for(long b = 0; b <= 2; b += 1)
        for(long c = 1; c <= 3; c += 1) for(long d = 0; d <= 2; d += 1){
            Alg x = { a, b }, y = { c, d };
            if(!no_ramo(B,C,x) || !no_ramo(B,C,y)) continue;
            long P1, v1, P2, v2;
            centro(B, x, &P1, &v1); centro(B, y, &P2, &v2);
            long dir = P1*P2 - D*v1*v2;          /* 4⟨x,y⟩ */
            long cruz = P1*v2 - v1*P2;           /* 2(x∧y) */
            long esq = dir*dir - D*cruz*cruz;
            long dirn = (P1*P1 - D*v1*v1)*(P2*P2 - D*v2*v2);
            n += 1;
            if(esq == dirn) lag += 1;
        }
        printf("        %-11s  %d/%d\n", cs[i].n, lag, n);
        if(lag != n || n == 0) falhou += 1;
    }
    ok("o direto É o cosseno e o cruzado É o seno, nos três regimes — as formas são duais."
       " cos/cosh/1e-8 eram transporte; dir² − Δ cruz² = (4N_x)(4N_y) e' Lagrange da forma",
       falhou == 0);

    printf("\n     sob o espelho ν(a,b) = (a + B·b, −b):\n");
    printf("        x        y        ⟨x,y⟩   ⟨νx,νy⟩   x∧y    νx∧νy\n");
    long B = 1, C = -1, D = DELTA(B, C);
    int d_mudou = 0, c_manteve = 0, c_vivo = 0, n = 0;
    for(long a = 1; a <= 3; a += 1) for(long c = 1; c <= 3; c += 1){
        Alg x = { a, a+1 }, y = { c, c+2 };
        Alg nx = { x.a + B*x.b, -x.b }, ny = { y.a + B*y.b, -y.b };
        long ux2 = 2*x.a + B*x.b,  uy2 = 2*y.a + B*y.b;
        long unx2 = 2*nx.a + B*nx.b, uny2 = 2*ny.a + B*ny.b;
        long d1 = ux2*uy2 - D*x.b*y.b, d2 = unx2*uny2 - D*nx.b*ny.b;
        long c1 = ux2*y.b - x.b*uy2,   c2 = unx2*ny.b - nx.b*uny2;
        if(d1 != d2) d_mudou += 1;
        if(c1 != -c2) c_manteve += 1;
        if(c1 != 0) c_vivo += 1;
        if(n < 4) printf("        (%ld,%ld)    (%ld,%ld)    %6ld   %6ld   %6ld  %6ld\n",
                         x.a, x.b, y.a, y.b, d1, d2, c1, c2);
        n += 1;
    }
    printf("        (as colunas ⟨·,·⟩ estão escaladas por 4 e as ∧ por 2 — inteiras)\n");
    printf("        e %d dos %d cruzados são NÃO NULOS\n", c_vivo, n);
    ok("SOB ν O DIRETO FICA IGUAL, E É IGUALDADE INTEIRA: escalando por 4 o meio da"
       " centragem desaparece — 4⟨x,y⟩ = (2u_x)(2u_y) − Δ v_x v_y —, e a peça que MEDE"
       " é bit a bit a mesma dos dois lados do espelho, sem um limiar",
       d_mudou == 0 && n == 9);
    ok("E O CRUZADO TROCA DE SINAL, TAMBÉM EXACTAMENTE: 2(x∧y) = (2u_x)v_y − v_x(2u_y) é"
       " inteiro, e o que se mede é c1 == −c2 e não «a soma é pequena» — a peça que ORDENA"
       " é a que se inverte. E os cruzados NÃO são todos nulos",
       c_manteve == 0 && n == 9 && c_vivo >= 6);
    conclui("um direto e um cruzado para cada lado da torre, e o espelho troca só o que ordena.");
}

static int modo_piloto(int argc, char **argv){
    if(argc < 5){
        printf("  uso: ./polar <B> <C> <a> <b>\n");
        printf("       ./polar 1 -1 3 2      o ouro, no ponto 3 + 2σ\n");
        return 2;
    }
    long B = atol(argv[1]), C = atol(argv[2]);
    Alg z = { atol(argv[3]), atol(argv[4]) };
    long D = DELTA(B, C), P, v;
    centro(B, z, &P, &v);
    printf("  a régua      q(a,b) = a² %+ld·ab %+ld·b²        Δ = %ld  (%s)\n", B, C, D,
           D < 0 ? "elíptico — gira" : D > 0 ? "hiperbólico — estica" : "parabólico — o limite");
    printf("\n  CARTESIANA   z = %ld %+ld·σ   (em ℤ[σ])\n", z.a, z.b);
    printf("               N(z) = %ld   — inteira\n", norma(B, C, z));
    printf("\n  POLAR        2u = %ld   v = %ld   (2u)² − Δ v² = %ld = 4 N\n",
           P, v, P*P - D*v*v);
    printf("               ramo %s   regime %+ld\n",
           no_ramo(B,C,z) ? "dentro" : "fora", D < 0 ? -1L : D > 0 ? 1L : 0L);
    Alg w = { (P - B*v)/2, v };
    printf("\n  a volta      (%ld, %ld)   exacta\n", w.a, w.b);
    return (w.a == z.a && w.b == z.b) ? 0 : 1;
}

int main(int argc, char **argv){
    if(argc > 1) return modo_piloto(argc, argv);

    puts("polar.c — A FORMA ALGÉBRICA E A POLAR DO CORPO UNIVERSAL");
    puts("=======================================================");
    puts("");
    puts("  Uma forma soma bem (a algébrica), a outra multiplica bem (a polar), e a ponte é");
    puts("  ∏ = exp∘Σ∘log. O regime — círculo, hipérbole ou reta — é o SINAL do Δ, e nada mais.");

    secao_Y1(); secao_Y2(); secao_Y3(); secao_Y4(); secao_Y5(); secao_Y6();

    printf("\n=======================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O corpo universal não tem três polares: tem UMA, e três leituras. cos, cosh e a");
        puts("  reta são a mesma série lida com Δ<0, Δ>0 e Δ=0 — e é por isso que a lei do");
        puts("  produto não precisa de caso especial em lado nenhum.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
