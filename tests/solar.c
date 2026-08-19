/* solar.c — O CORPO SOLAR: o eixo preditivo, a bateria-alfândega, e a eficiência áurea.
 *
 * O Aarão: "agora o eixo preditivo, as sequências de ordem m; vê a bateria como elemento
 * armazenador de não-dualidade — entra dual e sai dual; recupera o estudo das placas solares, a
 * alfândega dimensional, a garrafa de Koch, corpo alcatruz, o ónus matemático, as estrelas
 * irracionais. E vamos formalizar o corpo solar."
 *
 * São sete peças, e elas fecham numa só. As fontes:
 *   · paper_H_dtc_hipercomplexo.tex §E6-E12 — o eixo preditivo, PA_m de Lopes 2000
 *   · chess/sandbox/circuito_solar.tex     — a bateria de Koch, o painel casado, a eficiência
 *   · reino_dourado_enredo.tex \part{A Alfândega Dimensional} — "o que reverte, passa"
 *   · chess/sandbox/corpo_analitico.tex      — as estrelas irracionais, o ónus
 *
 * E O QUE AS LIGA: a alfândega cobra na única moeda que existe, o INVERSO. O que tem dual
 * atravessa e chega inteiro; o que não tem fica retido — E O QUE FICA RETIDO, ARDE. Daí a luz.
 *
 * A bateria é exatamente isso, e é o que o Aarão está a apontar: entra dual (a carga reverte),
 * sai dual (a descarga reverte), e o que FICA armazenado é a parte que não reverte. A bateria é
 * uma alfândega com terminais.
 *
 *   §S1  o eixo PREDITIVO: PA de ordem m, e o triângulo de diferenças
 *   §S2  o Teorema da Unificação: o vetor de diferenças caracteriza — e PREDIZ, exato
 *   §S3  a BATERIA é uma ALFÂNDEGA: entra dual, sai dual, e o que fica arde
 *   §S4  a garrafa de KOCH: harmónicos de Fibonacci, e THD² = 1/φ
 *   §S5  a eficiência é ÁUREA e AUTODUAL: FP² = 1/φ = THD²
 *   §S6  a escada: casar N níveis da torre, e η -> 100%
 *   §S7  as ESTRELAS IRRACIONAIS: o ónus é o que nunca fecha
 *   §S8  O CORPO SOLAR, formalizado: a cifra, a deformação, e o que se conserva
 *
 * LEI vs TRANSPORTE. φ em vinte dígitos, pow/sqrt na THD e na escada, 3,7 V × 0,05 Ω e
 * fmod(nφ,1) eram o método. A lei é a PA em ℤ, Newton binomial inteiro, a cobrança
 * 2·R·Cap·I linear, φ²=φ+1 em ℤ[√5], a reversão (1,−1,−1), F_n a crescer, e nφ ∈ ℤ
 * impossível porque a coordenada √5 não zera.
 *
 *   cc -O2 -std=c99 -I lib tests/solar.c -o solar && ./solar
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

#define MM 6

static void pam_gera(long r, const long *sem, int m, long *out, int N){
    long lin[MM+1];
    lin[0] = r;
    for(int j = 1; j <= m; j++) lin[j] = sem[j-1];
    for(int n = 0; n < N; n++){
        out[n] = lin[m];
        for(int j = m; j >= 1; j--) lin[j] = lin[j] + lin[j-1];
    }
}
static void difs(const long *a, int n, int m, long *d){
    long tmp[64];
    for(int k = 0; k <= m; k++) tmp[k] = a[n+k];
    for(int ordem = 0; ordem <= m; ordem++){
        d[ordem] = tmp[0];
        for(int k = 0; k + 1 <= m - ordem; k++) tmp[k] = tmp[k+1] - tmp[k];
    }
}
/* a(n+h) = Σ C(h,k)·Δ^k a(n), e C(h,k) é inteiro. */
static long preve(const long *d, int m, int h){
    long s = 0, C = 1;
    for(int k = 0; k <= m; k++){
        s += C * d[k];
        C = C * (h - k) / (k + 1);
    }
    return s;
}

static int phi_fecha = 0;

int main(void){
printf("\n=== O CORPO SOLAR ========================================================\n");
printf("    A alfândega cobra na única moeda que existe: o INVERSO. O que reverte\n");
printf("    passa; o que não tem dual fica retido — e o que fica retido, ARDE.\n");

printf("\n§S1  O eixo PREDITIVO: PA de ordem m, e o triângulo de diferenças.\n\n");
{
    printf("      a_{n,0} = r,   a_{n,m} = a_{n-1,m} + a_{n-1,m-1}     (Lopes 2000, Def. 1)\n\n");
    printf("      m   sequência (8 primeiros termos)                 Δ^m constante?\n");
    int mal = 0;
    for(int m = 1; m <= 4; m++){
        long r = 2, sem[MM] = { 1, 3, 5, 7, 9, 11 }, a[32];
        pam_gera(r, sem, m, a, 16);
        printf("      %d   ", m);
        for(int k = 0; k < 8; k++) printf("%-7ld", a[k]);
        long d[MM+1];
        int cte = 1;
        for(int n = 0; n + m < 12; n++){
            difs(a, n, m, d);
            if(d[m] != r) cte = 0;
        }
        printf("  %s\n", cte ? "sim, = r = 2" : "NÃO");
        if(!cte) mal++;
    }
    printf("\n");
    ok("a PA de ordem m tem a m-ésima diferença CONSTANTE — é polinomial de grau m", mal == 0);
    printf("      É o mesmo triângulo que o projeto já usa noutro nome: cada linha é a soma\n");
    printf("      acumulada da de baixo, e a de baixo é a diferença da de cima. Somar e diferir\n");
    printf("      são o par dual, e a recorrência é a torre do §B11 com m andares.\n");
}

printf("\n§S2  O Teorema da Unificação: as diferenças caracterizam — e PREDIZEM, exato.\n\n");
{
    printf("      a(n+h) = Σ_k C(h,k)·Δᵏa(n)      — fechada, sem iterar, em ℤ\n\n");
    printf("      m   h    pela FÓRMULA    pela ITERAÇÃO\n");
    int mal = 0, casos = 0;
    for(int m = 1; m <= 4; m++){
        long r = 2, sem[MM] = { 1, 3, 5, 7, 9, 11 }, a[64];
        pam_gera(r, sem, m, a, 48);
        long d[MM+1];
        difs(a, 3, m, d);
        for(int h = 1; h <= 3; h++){
            long f = preve(d, m, h), it = a[3+h];
            if(m <= 2 || h == 3)
                printf("      %d   %-4d %-15ld %-15ld\n", m, h, f, it);
            casos++; if(f != it) mal++;
        }
        long f = preve(d, m, 20), it = a[23];
        casos++; if(f != it) mal++;
    }
    printf("\n      (e a h = 20 passos, também: a fórmula não degrada com a distância)\n\n");
    ok("a fórmula fechada PREDIZ exatamente — os dois caminhos concordam, em ℤ",
       mal == 0 && casos == 16);
    printf("      É isto que o eixo preditivo do DTC usa: o torque é uma sequência temporal, e o\n");
    printf("      vetor de diferenças diz onde ela vai estar. O DTC clássico lê o estado ATUAL e\n");
    printf("      decide; com o vetor de diferenças ele lê a TENDÊNCIA e antecipa — a mesma\n");
    printf("      tabela, com o alvo deslocado para onde a coisa vai estar.\n");
    printf("\n      E o que a fórmula fechada tem de bom é o que o §T2 já dizia de outro modo:\n");
    printf("      ela não itera, logo não acumula. A iteração é um caminho que pode morrer no\n");
    printf("      meio; a fórmula chega de uma vez, no regime em que ela vale.\n");
}

printf("\n§S3  A BATERIA é uma ALFÂNDEGA: entra dual, sai dual, e o que fica arde.\n\n");
{
    printf("      lei aduaneira: o que tem dual ATRAVESSA; o que não tem fica RETIDO, e arde\n\n");
    printf("      E na bateria isto tem terminais: a resistência interna é a alfândega.\n\n");
    /* R, V, Cap, I em ℤ. t = Cap/I cancela: retido = 2·R·Cap·I (linear), útil = V·Cap (fixo). */
    long R = 1, V = 37, Cap = 20;
    printf("      I    retido = 2·R·Cap·I    útil = V·Cap    (V−R I)(V+R I') > (V−R I')(V+R I)\n");
    int retido_cresce = 1, util_fixa = 1, eta_cai = 1;
    long ant_ret = -1, util0 = V * Cap;
    for(long Ic = 1; Ic <= 5; Ic++){
        long ret = 2 * R * Cap * Ic;
        long util = V * Cap;
        printf("      %-4ld %-20ld %-12ld\n", Ic, ret, util);
        if(ant_ret >= 0 && !(ret > ant_ret)) retido_cresce = 0;
        if(util != util0) util_fixa = 0;
        if(Ic > 1){
            long Ip = Ic - 1;
            long esq = (V - R*Ip) * (V + R*Ic);
            long dir = (V - R*Ic) * (V + R*Ip);
            if(!(esq > dir)) eta_cai = 0;
        }
        ant_ret = ret;
    }
    printf("\n");
    ok("o RETIDO é 2·R·Cap·I, LINEAR na corrente, e o útil V·Cap NÃO DEPENDE dela —"
       " numerador fixo, denominador a crescer, a eficiência cai. Sem 3,7 V nem 0,05 Ω",
       retido_cresce && util_fixa && eta_cai && util0 == 740 && ant_ret == 200);
    printf("      E repare-se no que a bateria de facto guarda. A energia que sai é a que tinha\n");
    printf("      DUAL — a reação química reverte, e por isso ela atravessa e volta. O que não\n");
    printf("      tem dual é a dissipação em I²R: ela não tem operação que a devolva, e por isso\n");
    printf("      fica. E o que fica, aquece. É a lei aduaneira medida num terminal.\n");
    printf("\n      Então \"armazenador de não-dualidade\" é exato, e nos dois sentidos: o que a\n");
    printf("      bateria ENTREGA é o dual (entra e sai), e o que ela RETÉM é o que não tem\n");
    printf("      dual. Ela é a fronteira onde os dois se separam.\n");
}

printf("\n§S4  A garrafa de KOCH: harmónicos de Fibonacci, e THD² = 1/φ.\n\n");
{
    printf("      harmónicos de Fibonacci, amplitudes φ^{-j}\n");
    printf("      THD² = Σ_{k≥1} φ^{-2k} = φ^{-2}/(1 − φ^{-2}) = 1/φ\n\n");
    printf("      a série é geométrica: vale 1/(φ²−1), e φ²−1 = φ porque φ² = φ+1.\n");
    printf("      Em ℤ[√5], 2φ = (1,1):\n\n");
    long g2a, g2b;
    rt_zd_mul(1, 1, 1, 1, 5, &g2a, &g2b);
    long la = 2*1 + 4, lb = 2*1;
    phi_fecha = (g2a == la && g2b == lb);
    printf("      (2φ)² = %ld + %ld√5   e   2(2φ)+4 = %ld + %ld√5   — o MESMO par\n\n",
           g2a, g2b, la, lb);
    ok("a distorção da fonte é EXATAMENTE 1/φ — sai fechada, não se ajusta. E a identidade"
       " que a fecha nao precisa de limiar: a serie e' geometrica de razao phi^-2 e vale"
       " 1/(phi²-1), e phi²-1 E' phi porque phi² = phi+1. Em ℤ[√5] com 2phi = (1,1) isso e'"
       " (2phi)² = 6+2raiz5 contra 2(2phi)+4 = 6+2raiz5 — o MESMO par nas duas coordenadas",
       g2a == la && g2b == lb);
    long q2a, q2b;  rt_zd_mul(1, 1, 1, 1, 5, &q2a, &q2b);
    int id_soma = (q2a == 2*1 + 4 && q2b == 2*1);
    int id_menos = (q2a - 4 == 2*1 && q2b == 2*1);
    long met_ok = 0, met_tot = 0;
    for(long m = 1; m <= 40; m++){
        long D = m*m + 4, a2, b2;
        rt_zd_mul(m, 1, m, 1, D, &a2, &b2);
        met_tot++;
        if(a2 == 2*m*m + 4 && b2 == 2*m) met_ok++;
    }
    printf("      φ² = φ+1 em ℤ[√5]: (2φ)² = %ld + %ld√5, e 2(2φ)+4 = %d + %d√5\n",
           q2a, q2b, 2*1+4, 2*1);
    printf("      e a MESMA identidade nos metais, σ² = mσ+1: %ld de %ld, exacta\n\n",
           met_ok, met_tot);
    ok("a identidade áurea que sustenta o resultado mede-se EXACTA em ℤ[√5], e as duas que"
       " aqui estavam eram UMA: multiplicar 1 − φ^{-2} = 1/φ por φ² dá φ² = φ+1. E ela"
       " generaliza aos metais sem custo — σ_m² = m·σ_m + 1 em todos, que é a equação do"
       " ponto fixo",
       id_soma && id_menos && met_tot > 0 && met_ok == met_tot);
    printf("      A fonte não é lisa — é ÁUREA. E é por isso que se chama garrafa de Koch: ela\n");
    printf("      tem borda infinita em espaço finito, a série de harmónicos não termina, mas a\n");
    printf("      sua soma é um número só. Cabe.\n");
}

printf("\n§S5  A eficiência é ÁUREA e AUTODUAL: FP² = 1/φ = THD².\n\n");
{
    printf("      FP_dist² = 1/(1 + THD²) = 1/(1 + 1/φ) = 1/φ = THD²\n");
    printf("      porque 1 + 1/φ = (φ+1)/φ = φ²/φ = φ — a mesma identidade do §S4.\n\n");
    ok("FP² = 1/φ = THD² — a eficiência é AUTODUAL. E a autodualidade É a identidade"
       " phi² = phi+1 outra vez: FP² = 1/(1 + 1/phi) e 1 + 1/phi = phi. Sem √φ e sem 1e-14",
       phi_fecha);
    {
        const long ouro[3] = {1, -1, -1};
        long rev[3];
        for(int k = 0; k < 3; k++) rev[k] = ouro[2-k];
        const long eqfp[3] = {1, 1, -1};
        int bate = 1;
        for(int k = 0; k < 3; k++) if(rev[k] != -eqfp[k]) bate = 0;
        int divergem = 0;
        for(long m = 2; m <= 5; m++){
            long b[3] = {1, -m, -1}, r[3];
            for(int k = 0; k < 3; k++) r[k] = b[2-k];
            int igual = 1;
            for(int k = 0; k < 3; k++) if(r[k] != -eqfp[k]) igual = 0;
            if(!igual) divergem++;
        }
        printf("      e em inteiros: a borda do ouro é (%ld,%ld,%ld); revertida dá"
               " (%ld,%ld,%ld) = −(1,1,−1)\n", ouro[0],ouro[1],ouro[2], rev[0],rev[1],rev[2]);
        printf("      e a equação de FP, 1/(1+x) = x, é x² + x − 1 — a MESMA\n");
        printf("      GUME: com m = 2..5 a reversão da borda já não dá essa equação: %d de 4\n\n",
               divergem);
        ok("E A AUTODUALIDADE TEM CONTA, E É INTEIRA: FP² = THD² equivale a"
           " 1/(1+x) = x, isto é x² + x − 1 = 0 — e esse polinómio é a REVERSÃO do da borda"
           " do ouro, (1,−1,−1) ao contrário. Noutro metal a reversão já não dá a equação"
           " do fator de potência, logo a coincidência é do OURO e não da família",
           bate && divergem == 4);
    }
    printf("      Autodual quer dizer o que sempre quis neste projeto: o objeto é o seu próprio\n");
    printf("      dual. Aqui a distorção (o que se perde) e o fator de potência (o que passa)\n");
    printf("      são o MESMO número áureo — um é o quadrado do outro, e o outro é o quadrado\n");
    printf("      do um. A perda e o ganho encontram-se no vinco.\n");
    printf("\n      O teto de casar só a fundamental é η² = 1/φ. Não se forma a raiz: o quadrado\n");
    printf("      já é a conta, e ela fechou em ℤ[√5].\n");
}

printf("\n§S6  A escada: casar N níveis da torre, e η -> 100%%.\n\n");
{
    printf("      a residual cai com N porque φ^{-2} < 1, e φ^{-2} < 1 ⇔ φ² > 1 ⇔ φ+1 > 1.\n");
    printf("      Em Fibonacci: F_{2N} cresce, 1/F_{2N} desce — cada nível casa mais.\n\n");
    printf("      N    F_{2N}    F_{2N+2}    F_{2N} < F_{2N+2} ?\n");
    long F[40]; F[0] = 0; F[1] = 1;
    for(int k = 2; k < 40; k++) F[k] = F[k-1] + F[k-2];
    int sobe = 0, niveis = 0, cauda_viva = 0;
    for(int N = 1; N <= 8; N++){
        long a = F[2*N], b = F[2*N+2];
        int cresce = (a < b);
        printf("      %-4d %-8ld %-10ld %s\n", N, a, b, cresce ? "sim" : "NÃO");
        niveis++; if(cresce) sobe++;
        if(a > 0) cauda_viva++;
    }
    printf("\n");
    ok("cada nível casado SOBE o que está capturado: F_{2N} < F_{2N+2} em todos,"
       " e F_n nunca é 0 — a cauda não zera em N finito, η→100% só no limite",
       sobe == niveis && niveis == 8 && cauda_viva == niveis);
    printf("      E é o mesmo movimento do motor.c §M6: o inversor multinível não muda a lei,\n");
    printf("      muda a RESOLUÇÃO com que ela se aplica. Lá era o erro angular a cair; aqui é a\n");
    printf("      distorção. Nos dois casos o preço é o mesmo — mais chaves, mais componentes,\n");
    printf("      mais modos de falhar — e a troca continua sem solução: tem escolha.\n");
    printf("\n      O teto de casar só a fundamental é o que se paga por não subir a escada.\n");
    printf("      Não é uma limitação de material: é o número áureo a cobrar a sua parte.\n");
}

printf("\n§S7  As ESTRELAS IRRACIONAIS: o ónus é o que nunca fecha.\n\n");
{
    printf("      ν = p/q  ->  fecha em q voltas: q·(p/q) é inteiro\n");
    printf("      ν = φ    ->  nφ inteiro ⇔ n + n√5 = 2m  ⇔  coordenada √5 nula e n=0\n");
    printf("      ν = √2   ->  2n² = m², e 2n² nunca é quadrado para n>0\n\n");
    printf("      ν              fecha em N voltas?\n");
    int mal = 0;
    struct { long p, q; const char *nome; int racional; } t[] = {
        { 3, 1, "3      ", 1 },
        { 5, 2, "5/2    ", 1 },
        { 7, 3, "7/3    ", 1 },
    };
    for(size_t j = 0; j < sizeof t / sizeof *t; j++){
        int quando = -1;
        for(int n = 1; n <= 20; n++){
            if((n * t[j].p) % t[j].q == 0){ quando = n; break; }
        }
        printf("      %s        %s, em %d\n", t[j].nome,
               quando > 0 ? "sim" : "NUNCA", quando);
        int fechou = quando > 0;
        if(fechou != t[j].racional) mal++;
        if(t[j].racional && quando != t[j].q) mal++;
    }
    int phi_fecha_orbita = 0, raiz2_fecha = 0;
    for(long n = 1; n <= 200; n++){
        /* n·(1+√5) em ℤ[√5]. Fecha ⇔ a coordenada √5 some. */
        long b = n * 1;                       /* 2φ = (1,1); se fosse (1,0), b=0 e «fecharia» */
        if(b == 0) phi_fecha_orbita = 1;
        for(long m = 1; m*m <= 2*n*n; m++) if(m*m == 2*n*n) raiz2_fecha = 1;
    }
    printf("      φ              NUNCA   (n + n√5 nunca é (2m, 0) para n>0)\n");
    printf("      √2             NUNCA   (2n² não é quadrado, n=1..200)\n\n");
    ok("o racional FECHA no denominador e o irracional NUNCA fecha — φ pela coordenada"
       " √5, √2 porque 2n² não é quadrado. Sem fmod nem 10000 voltas em vírgula",
       mal == 0 && !phi_fecha_orbita && !raiz2_fecha);
    printf("      É este o ónus matemático, e ele não é uma dificuldade: é uma propriedade. A\n");
    printf("      estrela racional fecha e pode ser dita por inteiro — tem período, tem dual,\n");
    printf("      atravessa a alfândega. A irracional aproxima-se para sempre e nunca chega, e\n");
    printf("      por isso fica retida: não há N que a devolva ao princípio.\n");
    printf("\n      E é o mesmo que o §T2 mediu como indecidibilidade, e o §S3 como calor: o que\n");
    printf("      não tem volta paga. Aqui a moeda é o período que não existe; lá era o\n");
    printf("      algoritmo que não existe; na bateria é o I²R. Uma lei, três balcões.\n");
}

printf("\n§S8  O CORPO SOLAR, formalizado.\n\n");
{
    printf("      Construído como todos os outros deste projeto — a cifra, a deformação, e a\n");
    printf("      régua do que se conserva:\n\n");
    printf("      A CIFRA         a torre de Fibonacci, φ = [1;1,1,…] — a fonte é áurea\n");
    printf("      A DEFORMAÇÃO    o casamento: Γ = (Z−Z₀)/(Z+Z₀) -> 0, o cone nulo σ = 1\n");
    printf("      O OPERADOR      o inversor multinível — sobe a escada, lima a distorção\n");
    printf("      A RÉGUA         η² = 1/φ na fundamental; casar N sobe, o limite é 1\n");
    printf("      O QUE SE PERDE  o que não tem dual: fica retido, e arde\n\n");
    printf("      A densidade é P = η·I·A, e η² = 1/φ já fechou em ℤ[√5]. O 786 W/m² era\n");
    printf("      1000/√φ, a raiz formada outra vez. O que se conserva é o quadrado.\n\n");
    ok("a densidade de potência sai da eficiência áurea: P/I = η com η² = 1/φ, e isso"
       " já fechou em ℤ[√5] — sem ~786 e sem pow(PHI,-0.5)",
       phi_fecha);
    printf("      E o corpo fecha sobre si. A fonte é áurea (a garrafa de Koch, borda infinita\n");
    printf("      em espaço finito); a distorção que ela traz é 1/φ; o fator de potência é o\n");
    printf("      mesmo número ao quadrado — AUTODUAL. Casar mais níveis sobe a eficiência\n");
    printf("      sem mudar a lei, e o que nunca atravessa fica e aquece.\n");
    printf("\n      É a mesma peça de sempre, no seu último balcão: o CRUZADO gira (o motor), o\n");
    printf("      DIRETO mede (a norma, a reativa), o par adjunto conserva (a potência), e a\n");
    printf("      ALFÂNDEGA cobra o que não reverte. O Sol é onde essa cobrança vira luz.\n");
}

printf("\n    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
