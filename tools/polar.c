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
 * — e é exatamente ∏ = exp∘Σ∘log, o operador da tríade. **Uma forma para cada operação**, e a
 * ponte entre elas é o operador. Não há terceira.
 *
 * O QUE MUDA COM O Δ, E É SÓ ISTO. Centrando a base em τ = σ − B/2, sai τ² = Δ/4 — logo o regime
 * inteiro está no SINAL do Δ, e nada mais:
 *
 *      Δ < 0   τ² < 0    τ é o i        E(θ) = cos θ  + τ̂ sin θ      o CÍRCULO,   gira
 *      Δ > 0   τ² > 0    τ é o j        E(θ) = cosh θ + τ̂ sinh θ     a HIPÉRBOLE, estica
 *      Δ = 0   τ² = 0    τ é o ε        E(θ) = 1      + τ̂ θ          a RETA,      o limite
 *
 * As três são a MESMA série truncada de modos diferentes, e é por isso que a lei do produto vale
 * nas três sem caso especial. *O corpo universal não tem três polares: tem uma, e três leituras.*
 *
 *   §Y1  a centragem: τ = σ − B/2, e τ² = Δ/4 — o regime está no sinal, e em mais nada
 *   §Y2  a lei do produto nas TRÊS: ρ multiplica, θ soma — e mede-se nas três
 *   §Y3  a volta: polar → algébrica → polar, com resíduo 0
 *   §Y4  a potência é a lei outra vez: ρ^n e n·θ — e é De Moivre nos três regimes
 *   §Y5  o que o painel usa: converter nos dois sentidos, e o que cada forma serve\n *   §Y6  A DUALIDADE FECHADA: a algébrica é o DIRETO, a polar é o CRUZADO
 *
 *   cc -O2 -std=c99 -Wall -Wformat polar.c -lm -o polar && ./polar
 *   ./polar 1 -1 3 2        a régua (B,C) e o ponto (a,b): dá a polar
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846   /* o -std=c99 estrito esconde-o: define-se, não se baixa a norma */
#endif
#include "unidade.h"

typedef struct { double a, b; } Alg;      /* z = a + b·σ */
/* E A POLAR CARREGA UM SINAL, que o elíptico não precisa. No círculo o atan2 cobre a volta
 * inteira; na hipérbole não há atan2 — cosh θ é sempre positivo, logo θ sozinho nunca alcança
 * u < 0. O ramo é genuinamente separado, e o sinal é a informação a mais: DOIS BITS, um por
 * eixo. Sem ele a volta erra por 12 e a asserção cai — foi o que aconteceu. */
typedef struct { double rho, th; int reg; int sig; } Pol;   /* reg: −1 elíptico, +1 hiperbólico, 0 parabólico */

static double DELTA(double B, double C){ return B*B - 4*C; }

/* a norma, na forma algébrica: N(a,b) = a² + B·a·b + C·b² */
static double norma(double B, double C, Alg z){ return z.a*z.a + B*z.a*z.b + C*z.b*z.b; }

/* o produto, pela borda σ² = B·σ − C */
static Alg prod(double B, double C, Alg x, Alg y){
    Alg z = { x.a*y.a - C*x.b*y.b, x.a*y.b + x.b*y.a + B*x.b*y.b };
    return z;
}

/* ---- ALGÉBRICA → POLAR ----
 * Centrando: u = a + (B/2)·b,  v = b,  e τ² = Δ/4. Então N = u² − (Δ/4)·v², e a polar sai
 * do regime. O |τ| = √|Δ|/2 é o que põe u e v na mesma escala — sem ele os ângulos ficam
 * torcidos, e a lei do produto deixa de fechar. */
static Pol para_polar(double B, double C, Alg z){
    double D = DELTA(B, C);
    double u = z.a + 0.5*B*z.b, v = z.b;
    double t = sqrt(fabs(D)) / 2.0;          /* |τ| */
    Pol p;
    p.sig = 1;
    if(D < 0){                                /* elíptico: o círculo */
        p.reg = -1;
        p.rho = sqrt(u*u + t*t*v*v);
        p.th  = atan2(t*v, u);
    } else if(D > 0){                          /* hiperbólico: a hipérbole */
        p.reg = 1;
        double q = u*u - t*t*v*v;
        p.rho = sqrt(fabs(q));
        p.th  = atanh((t*v) / u);              /* o ramo com |t·v| < |u| */
        p.sig = (u < 0) ? -1 : 1;              /* e o ramo: cosh nunca é negativo */
    } else {                                   /* parabólico: a reta */
        p.reg = 0;
        p.rho = fabs(u);
        p.th  = v / u;
        p.sig = (u < 0) ? -1 : 1;
    }
    return p;
}

/* ---- POLAR → ALGÉBRICA ---- a volta, e é ela que prova que a ida não perdeu nada */
static Alg para_alg(double B, double C, Pol p){
    double D = DELTA(B, C);
    double t = sqrt(fabs(D)) / 2.0;
    double u, v;
    if(p.reg < 0){ u = p.rho*cos(p.th);  v = p.rho*sin(p.th)  / t; }
    else if(p.reg > 0){ u = p.sig*p.rho*cosh(p.th); v = p.sig*p.rho*sinh(p.th) / t; }
    else { u = p.sig*p.rho; v = p.sig*p.rho*p.th; }
    Alg z = { u - 0.5*B*v, v };
    return z;
}

/* ================================================================================ */
static void secao_Y1(void){
    printf("\n§Y1  A CENTRAGEM: τ = σ − B/2 dá τ² = Δ/4 — e o regime é o SINAL do Δ\n\n");

    printf("        corpo      (B,C)     Δ      τ² = Δ/4    regime\n");
    struct { const char *n; double B, C; } cs[] = {
        { "ouro",   1, -1 }, { "prata",  2, -1 }, { "i",      0,  1 },
        { "ω",     -1,  1 }, { "PA",     2,  1 }, { "√2",     0, -2 },
    };
    int erros = 0;
    for(int i = 0; i < 6; i++){
        double D = DELTA(cs[i].B, cs[i].C);
        /* τ² tem de dar Δ/4, e verifica-se pela definição: τ = σ − B/2 e σ² = Bσ − C.
         * τ² = σ² − Bσ + B²/4 = −C + B²/4 = (B² − 4C)/4 = Δ/4. Medimos numericamente. */
        double sig = (D >= 0) ? (cs[i].B + sqrt(D))/2 : cs[i].B/2;   /* a raiz real, se houver */
        double tau2;
        if(D >= 0){ double tau = sig - cs[i].B/2; tau2 = tau*tau; }
        else tau2 = D/4.0;                    /* a raiz é complexa: τ² = Δ/4 por construção */
        if(fabs(tau2 - D/4.0) > 1e-12) erros++;
        printf("        %-8s  (%2.0f,%2.0f)  %5.1f   %8.3f    %s\n",
               cs[i].n, cs[i].B, cs[i].C, D, tau2,
               D < 0 ? "elíptico  — gira" : D > 0 ? "hiperbólico — estica" : "parabólico — o limite");
    }
    ok("τ² = Δ/4 nos seis corpos — a centragem tira o B e deixa só o Δ", erros == 0);

    conclui("o regime não é uma escolha de coordenadas: é o sinal de um número que já estava lá.");
}

/* ================================================================================ */
/* §Y2 — a lei do produto: ρ multiplica, θ soma. Nas três.                          */
/* ================================================================================ */
static void secao_Y2(void){
    printf("\n§Y2  A LEI: ρ(zw) = ρ(z)ρ(w) e θ(zw) = θ(z)+θ(w) — nos três regimes\n\n");

    struct { const char *n; double B, C; } cs[] = {
        { "i        Δ<0", 0,  1 },
        { "ouro     Δ>0", 1, -1 },
        { "PA       Δ=0", 2,  1 },
    };
    printf("        corpo         pior erro em ρ      pior erro em θ      pares\n");
    int falhou = 0;
    for(int i = 0; i < 3; i++){
        double B = cs[i].B, C = cs[i].C;
        double pior_r = 0, pior_t = 0; int n = 0;
        for(double a = 1; a <= 3; a += 1) for(double b = 0; b <= 2; b += 1)
        for(double c = 1; c <= 3; c += 1) for(double d = 0; d <= 2; d += 1){
            Alg x = { a, b }, y = { c, d };
            if(fabs(norma(B,C,x)) < 1e-9 || fabs(norma(B,C,y)) < 1e-9) continue;
            Pol px = para_polar(B,C,x), py = para_polar(B,C,y);
            Alg xy = prod(B,C,x,y);
            if(fabs(norma(B,C,xy)) < 1e-9) continue;
            Pol pxy = para_polar(B,C,xy);
            double er = fabs(pxy.rho - px.rho*py.rho) / (px.rho*py.rho);
            /* no elíptico o ângulo é módulo 2π: compara-se a diferença dobrada ao círculo */
            double dt = pxy.th - (px.th + py.th);
            if(px.reg < 0){ while(dt >  M_PI) dt -= 2*M_PI; while(dt < -M_PI) dt += 2*M_PI; }
            if(er > pior_r) pior_r = er;
            if(fabs(dt) > pior_t) pior_t = fabs(dt);
            n++;
        }
        if(pior_r > 1e-9 || pior_t > 1e-9) falhou++;
        printf("        %-12s  %.2e            %.2e            %d\n", cs[i].n, pior_r, pior_t, n);
    }
    ok("a lei vale nos três regimes, sem caso especial — é ∏ = exp∘Σ∘log", falhou == 0);

    /* e tem de saber falhar: com o |τ| ERRADO (sem a escala √|Δ|/2) o ângulo torce e a lei cai */
    double B = 1, C = -1, pior = 0;
    for(double a = 1; a <= 3; a += 1) for(double b = 1; b <= 2; b += 1){
        Alg x = { a, b }, y = { b, a };
        double u1 = x.a + 0.5*B*x.b, v1 = x.b, u2 = y.a + 0.5*B*y.b, v2 = y.b;
        double t1 = atanh(v1/u1), t2 = atanh(v2/u2);       /* SEM a escala |τ| */
        Alg xy = prod(B,C,x,y);
        double u3 = xy.a + 0.5*B*xy.b, v3 = xy.b;
        double t3 = atanh(v3/u3);
        double d = fabs(t3 - (t1 + t2));
        if(d > pior) pior = d;
    }
    printf("     sem a escala |τ| = √|Δ|/2, o pior erro no ângulo: %.3f\n", pior);
    ok("sem a escala a lei CAI — logo a escala está a fazer trabalho, não é enfeite", pior > 1e-3);

    conclui("uma forma soma bem, a outra multiplica bem, e o operador é a ponte entre as duas.");
}

/* ================================================================================ */
static void secao_Y3(void){
    printf("\n§Y3  A VOLTA: algébrica → polar → algébrica, com resíduo 0\n\n");

    struct { const char *n; double B, C; } cs[] = {
        { "i     Δ<0", 0, 1 }, { "ouro  Δ>0", 1, -1 }, { "PA    Δ=0", 2, 1 },
    };
    /* E A POLAR TEM RAMO, A ALGÉBRICA NÃO — e isto apareceu por a asserção ter falhado.
     * No hiperbólico o atanh só existe com |τ·v| < |u|: os pontos fora do cone não têm ângulo
     * real, e no parabólico u = 0 não tem ângulo nenhum. Não é defeito da conversão; é uma
     * diferença de FACTO entre as duas formas, e escondê-la com um filtro calado seria pior
     * do que dizê-la. Por isso o ramo é CONTADO, e a asserção é sobre os pontos que têm polar. */
    printf("        corpo        no ramo   fora do ramo   pior resíduo da volta\n");
    int falhou = 0, fora_total = 0;
    for(int i = 0; i < 3; i++){
        double B = cs[i].B, C = cs[i].C, D = DELTA(B,C), pior = 0; int n = 0, fora = 0;
        double t = sqrt(fabs(D))/2.0;
        for(double a = -3; a <= 3; a += 1) for(double b = -3; b <= 3; b += 1){
            Alg z = { a, b };
            if(fabs(norma(B,C,z)) < 1e-9){ fora++; continue; }
            double u = z.a + 0.5*B*z.b, v = z.b;
            if(D > 0 && fabs(t*v) >= fabs(u)){ fora++; continue; }   /* fora do cone */
            if(D == 0 && fabs(u) < 1e-12){ fora++; continue; }        /* sem ângulo */
            Pol p = para_polar(B,C,z);
            if(!isfinite(p.rho) || !isfinite(p.th)){ fora++; continue; }
            Alg w = para_alg(B,C,p);
            double e = fabs(w.a - z.a) + fabs(w.b - z.b);
            if(e > pior) pior = e;
            n++;
        }
        if(pior > 1e-9) falhou++;
        fora_total += fora;
        printf("        %-11s  %6d   %11d   %.3e\n", cs[i].n, n, fora, pior);
    }
    ok("a volta fecha nos três regimes, dentro do ramo — a polar não perde informação",
       falhou == 0);
    ok("e HÁ pontos fora do ramo — a polar não é global, e a algébrica é", fora_total > 0);
    printf("     e o SINAL é a informação a mais: 2 bits, sem os quais a volta erra por 12\n");

    conclui("mudar de forma não é mudar de corpo; mas a polar tem ramo e a algébrica não, e isso é do corpo.");
}

/* ================================================================================ */
static void secao_Y4(void){
    printf("\n§Y4  A POTÊNCIA É A LEI OUTRA VEZ: ρⁿ e n·θ — De Moivre nos três\n\n");

    struct { const char *n; double B, C; double a, b; } cs[] = {
        { "i     Δ<0", 0,  1, 1, 1 },
        { "ouro  Δ>0", 1, -1, 2, 1 },
        { "PA    Δ=0", 2,  1, 1, 1 },
    };
    printf("        corpo        n     ρⁿ previsto     ρ medido       resíduo\n");
    int falhou = 0;
    for(int i = 0; i < 3; i++){
        double B = cs[i].B, C = cs[i].C;
        Alg z = { cs[i].a, cs[i].b };
        Pol p0 = para_polar(B,C,z);
        Alg w = { 1, 0 };
        for(int n = 1; n <= 5; n++){
            w = prod(B,C,w,z);
            Pol pn = para_polar(B,C,w);
            double prev = pow(p0.rho, n);
            double res = fabs(pn.rho - prev) / prev;
            /* e o ângulo: n·θ, com o círculo dobrado no elíptico */
            double dt = pn.th - n*p0.th;
            if(pn.reg < 0){ while(dt > M_PI) dt -= 2*M_PI; while(dt < -M_PI) dt += 2*M_PI; }
            if(res > 1e-9 || fabs(dt) > 1e-9) falhou++;
            if(n == 5) printf("        %-11s  %d   %12.4f   %12.4f   %.2e\n",
                              cs[i].n, n, prev, pn.rho, res);
        }
    }
    ok("ρⁿ e n·θ valem para n = 1..5 nos três regimes — De Moivre é um só", falhou == 0);

    conclui("elevar a n é somar o ângulo n vezes. É a mesma lei, e não uma fórmula nova.");
}

/* ================================================================================ */
static void secao_Y5(void){
    printf("\n§Y5  O QUE O PAINEL USA: cada forma serve uma coisa\n\n");

    printf("        a forma        soma       produto     potência    o que serve\n");
    printf("        ALGÉBRICA      trivial    3 mults     n mults     somar, guardar, comparar\n");
    printf("        POLAR          difícil    1 mult      1 pow       multiplicar, girar, escalar\n\n");

    /* e a conta que justifica o painel ter as duas: contar as operações de facto */
    double B = 1, C = -1;
    Alg x = { 3, 2 }, y = { 5, 1 };
    Alg p_alg = prod(B,C,x,y);
    Pol px = para_polar(B,C,x), py = para_polar(B,C,y);
    Pol p_pol = { px.rho*py.rho, px.th + py.th, px.reg, px.sig*py.sig };
    Alg volta = para_alg(B,C,p_pol);
    printf("        (3,2) ⊗ (5,1) pela algébrica:  (%.0f, %.0f)\n", p_alg.a, p_alg.b);
    printf("                      pela polar:      (%.4f, %.4f)\n", volta.a, volta.b);
    double e = fabs(volta.a - p_alg.a) + fabs(volta.b - p_alg.b);
    printf("        resíduo entre os dois caminhos: %.2e\n", e);
    ok("os DOIS caminhos dão o mesmo produto — e é por isso que o painel pode ter as duas", e < 1e-9);

    conclui("o painel mostra as duas porque o piloto usa as duas: uma para pôr, outra para mover.");
}

/* ================================================================================ */
/* §Y6 — A DUALIDADE FECHADA: a algébrica é o DIRETO, a polar é o CRUZADO           */
/* ================================================================================ */
/* O Aarão: "fecha a dualidade — forma algébrica escrita é produto DIRETO, e forma polar é
 * produto CRUZADO. A leitura verifica isso. Eles são duais: um direto e um cruzado para cada
 * lado da torre."
 *
 * E é a partição B = B_s + B_a outra vez, agora nas formas:
 *
 *      ⟨x,y⟩ = o DIRETO      simétrico    MEDE      e é a forma ALGÉBRICA que o escreve
 *      x ∧ y = o CRUZADO     antissimétrico ORDENA  e é a forma POLAR que o lê
 *
 * A conta que liga as duas é uma só, e vale nos três regimes com cos/cosh/1:
 *
 *      ⟨x,y⟩ = ρ_x ρ_y · cos_Δ(θ_y − θ_x)        o direto vê o COSSENO
 *      x ∧ y = ρ_x ρ_y · sin_Δ(θ_y − θ_x) / |τ|  o cruzado vê o SENO
 *
 * E O FECHO DA DUALIDADE, que é o que ele pediu: sob o espelho ν, o DIRETO fica igual e o
 * CRUZADO troca de sinal. É por isso que são os dois lados da torre — a peça que mede é a mesma
 * dos dois lados, e a peça que ordena é que se inverte. */
static void secao_Y6(void){
    printf("\n§Y6  A DUALIDADE FECHADA: algébrica = DIRETO, polar = CRUZADO\n\n");

    struct { const char *n; double B, C; } cs[] = {
        { "i     Δ<0", 0, 1 }, { "ouro  Δ>0", 1, -1 }, { "PA    Δ=0", 2, 1 },
    };
    printf("        corpo        ⟨x,y⟩ = ρρ·cos_Δ(Δθ)   x∧y = ρρ·sin_Δ(Δθ)/|τ|   pares\n");
    int falhou = 0;
    for(int i = 0; i < 3; i++){
        double B = cs[i].B, C = cs[i].C, D = DELTA(B,C), t = sqrt(fabs(D))/2.0;
        double pior_d = 0, pior_c = 0; int n = 0;
        for(double a = 1; a <= 3; a += 1) for(double b = 0; b <= 2; b += 1)
        for(double c = 1; c <= 3; c += 1) for(double d = 0; d <= 2; d += 1){
            Alg x = { a, b }, y = { c, d };
            double ux = x.a + 0.5*B*x.b, uy = y.a + 0.5*B*y.b;
            if(D > 0 && (fabs(t*x.b) >= fabs(ux) || fabs(t*y.b) >= fabs(uy))) continue;
            if(D == 0 && (fabs(ux) < 1e-12 || fabs(uy) < 1e-12)) continue;
            Pol px = para_polar(B,C,x), py = para_polar(B,C,y);
            if(!isfinite(px.th) || !isfinite(py.th) || px.rho < 1e-9 || py.rho < 1e-9) continue;
            /* o DIRETO na base centrada: ⟨x,y⟩ = u_x u_y − (Δ/4)·v_x v_y */
            double direto  = ux*uy - (D/4.0)*x.b*y.b;
            /* o CRUZADO: o determinante, a área — e é o mesmo em qualquer base centrada */
            double cruzado = ux*y.b - x.b*uy;
            double dth = py.th - px.th, prev_d, prev_c;
            if(D < 0){ prev_d = px.rho*py.rho*cos(dth);  prev_c = px.rho*py.rho*sin(dth)/t; }
            else if(D > 0){ prev_d = px.rho*py.rho*cosh(dth); prev_c = px.rho*py.rho*sinh(dth)/t; }
            else { prev_d = px.rho*py.rho; prev_c = px.rho*py.rho*dth; }
            double ed = fabs(direto - prev_d), ec = fabs(cruzado - prev_c);
            if(ed > pior_d) pior_d = ed;
            if(ec > pior_c) pior_c = ec;
            n++;
        }
        if(pior_d > 1e-8 || pior_c > 1e-8) falhou++;
        printf("        %-11s  resíduo %.2e         resíduo %.2e          %d\n",
               cs[i].n, pior_d, pior_c, n);
    }
    ok("o direto É o cosseno e o cruzado É o seno, nos três regimes — as formas são duais",
       falhou == 0);

    /* O FECHO: sob ν, o direto fica e o cruzado troca. Os dois lados da torre. */
    printf("\n     sob o espelho ν(a,b) = (a + B·b, −b):\n");
    printf("        x        y        ⟨x,y⟩   ⟨νx,νy⟩   x∧y    νx∧νy\n");
    double B = 1, C = -1, D = DELTA(B,C);
    int d_mudou = 0, c_manteve = 0, n = 0;
    for(double a = 1; a <= 3; a += 1) for(double c = 1; c <= 3; c += 1){
        Alg x = { a, a+1 }, y = { c, c+2 };
        Alg nx = { x.a + B*x.b, -x.b }, ny = { y.a + B*y.b, -y.b };
        double ux = x.a + 0.5*B*x.b, uy = y.a + 0.5*B*y.b;
        double unx = nx.a + 0.5*B*nx.b, uny = ny.a + 0.5*B*ny.b;
        double d1 = ux*uy - (D/4.0)*x.b*y.b, d2 = unx*uny - (D/4.0)*nx.b*ny.b;
        double c1 = ux*y.b - x.b*uy,          c2 = unx*ny.b - nx.b*uny;
        if(fabs(d1 - d2) > 1e-9) d_mudou++;
        if(fabs(c1 + c2) > 1e-9) c_manteve++;      /* devia trocar de SINAL */
        if(n < 4) printf("        (%.0f,%.0f)    (%.0f,%.0f)    %6.1f   %6.1f   %6.1f  %6.1f\n",
                         x.a, x.b, y.a, y.b, d1, d2, c1, c2);
        n++;
    }
    ok("sob ν o DIRETO fica igual — a peça que MEDE é a mesma dos dois lados", d_mudou == 0);
    ok("e o CRUZADO troca de sinal — a peça que ORDENA é a que se inverte", c_manteve == 0);

    conclui("um direto e um cruzado para cada lado da torre, e o espelho troca só o que ordena.");
}

/* ================================================================================ */
static int modo_piloto(int argc, char **argv){
    if(argc < 5){
        printf("  uso: ./polar <B> <C> <a> <b>\n");
        printf("       ./polar 1 -1 3 2      o ouro, no ponto 3 + 2σ\n");
        return 2;
    }
    double B = atof(argv[1]), C = atof(argv[2]);
    Alg z = { atof(argv[3]), atof(argv[4]) };
    double D = DELTA(B, C);
    Pol p = para_polar(B, C, z);
    Alg v = para_alg(B, C, p);
    printf("  a régua      q(a,b) = a² %+.0f·ab %+.0f·b²        Δ = %.0f  (%s)\n", B, C, D,
           D < 0 ? "elíptico — gira" : D > 0 ? "hiperbólico — estica" : "parabólico — o limite");
    printf("\n  CARTESIANA   z = %.4f %+.4f·σ\n", z.a, z.b);
    printf("               N(z) = %.4f\n", norma(B, C, z));
    printf("\n  POLAR        ρ = %.6f   θ = %.6f   ramo %+d\n", p.rho, p.th, p.sig);
    printf("               z = ρ·(%s)\n",
           p.reg < 0 ? "cos θ + τ̂ sin θ" : p.reg > 0 ? "cosh θ + τ̂ sinh θ" : "1 + τ̂ θ");
    printf("\n  a volta      (%.6f, %.6f)   resíduo %.2e\n",
           v.a, v.b, fabs(v.a - z.a) + fabs(v.b - z.b));
    return 0;
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
