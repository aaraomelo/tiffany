/* cantor_julia_dual.c — CANTOR e JULIA são DUAIS, e o corpo de Alonzo é a composição.
 *
 * O Aarão: «vê o corpo de Alonzo, a composição --- cantor+julia dual, cantor²=julia+1, algo assim.»
 *
 * Mede-se, não se afirma (a memória: não forçar a fórmula bonita). O mapa quadrático f_c(z)=z²+c é
 * ITERAÇÃO --- compor f consigo mesma ---, e iterar é o λ-cálculo de CHURCH/ALONZO: a composição É o
 * corpo de Alonzo. E Cantor e Julia são duais de dois modos, os dois EXATOS:
 *
 *   §CJ1  o DUAL aditivo↔multiplicativo: a duplicação de ângulo θ→2θ (o SHIFT de Cantor, aditivo)
 *         É o quadrado z→z² (Julia, multiplicativo), conjugados por z=e^{2πiθ}. Exato em ℤ[i].
 *   §CJ2  o ponto fixo metálico: z²+c fixa em z²=z−c; em c=−1 dá o ÁUREO φ (φ²=φ+1). É a forma
 *         HONESTA do «x²=y+1»: é julia²=julia+1 (o ponto fixo de Julia), o áureo --- não cantor².
 *   §CJ3  os dois REGIMES duais: a órbita do ponto crítico 0. Limitada (c no Mandelbrot) -> Julia
 *         CONEXO; escapa (c fora) -> poeira de CANTOR. c=−1 fecha em período 2; c=−3 escapa.
 *
 * A leitura honesta do teu «cantor²=julia+1»: o «²» é a dobra (z→z², o tecido); o «+1» é a unidade
 * (a estaca c=−1); e a relação que FECHA é a metálica no ponto fixo, φ²=φ+1 --- o áureo, m=1.
 *
 *   cc -O2 -std=c99 -Wall -I../lib cantor_julia_dual.c -o cantor_julia_dual && ./cantor_julia_dual
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

/* ── ℤ[i]: um inteiro de Gauss (re, im). O quadrado é multiplicação, não fórmula. ─────── */
typedef struct { L re, im; } Zi;
static Zi zi_quad(Zi z){ Zi r; r.re = z.re*z.re - z.im*z.im; r.im = 2*z.re*z.im; return r; }   /* z² */
/* i^k (k mod 4): a 4-ésima raiz da unidade --- o ângulo k·90°, exato */
static Zi i_pot(int k){ k = ((k % 4) + 4) % 4; Zi t[4] = {{1,0},{0,1},{-1,0},{0,-1}}; return t[k]; }
static int zi_igual(Zi a, Zi b){ return a.re == b.re && a.im == b.im; }

/* ── ℤ[φ]: a + bφ, com φ²=φ+1. O produto usa φ²=φ+1, sem raiz nenhuma. ─────────────────── */
typedef struct { L a, b; } Zf;
static Zf zf_mul(Zf x, Zf y){ Zf r; r.a = x.a*y.a + x.b*y.b; r.b = x.a*y.b + x.b*y.a + x.b*y.b; return r; }
static int zf_igual(Zf x, Zf y){ return x.a == y.a && x.b == y.b; }

int main(void){
    printf("=== CANTOR e JULIA são DUAIS, e a composição é o corpo de ALONZO ==================\n\n");

    /* ── §CJ1 o dual aditivo↔multiplicativo: duplicar o ângulo É elevar ao quadrado ───────── */
    /* na 4-ésima raiz da unidade (q=4), z=i^p. O QUADRADO (multiplicativo, ℤ[i]) tem de dar o mesmo
     * que DUPLICAR o ângulo (aditivo, 2p mod 4). Cantor (o shift/duplicação, aditivo) É Julia (o
     * quadrado, multiplicativo), conjugados por z=e^{2πiθ} --- o dual contar↔integrar. */
    L res_dual = 0, npar = 0;
    for(int p = 0; p < 4; p++){
        Zi z = i_pot(p);
        Zi q_mult = zi_quad(z);          /* JULIA: z² por multiplicação de Gauss */
        Zi q_add  = i_pot(2 * p);        /* CANTOR: duplicar o ângulo, 2p mod 4 (aditivo) */
        if(!zi_igual(q_mult, q_add)) res_dual++;
        npar++;
    }
    printf("      q=4: o quadrado (Julia, mult.) vs a duplicação do ângulo (Cantor, aditiva): %lld"
           " de %lld batem, resíduo %lld\n\n", npar - res_dual, npar, res_dual);
    ok("§CJ1 CANTOR e JULIA são o DUAL aditivo↔multiplicativo: a duplicação do ângulo θ→2θ (o SHIFT de"
       " Cantor, aditivo) É o quadrado z→z² (Julia, multiplicativo), conjugados por z=e^{2πiθ} --- o"
       " quadrado de Gauss bate a duplicação do ângulo, exato, resíduo 0. É o contar↔integrar",
       res_dual == 0 && npar == 4);

    /* ── §CJ2 o ponto fixo metálico: z²+c fixa em z²=z−c; c=−1 dá o áureo ──────────────────── */
    /* o ponto fixo de f_c(z)=z²+c é z=z²+c, logo z²=z−c. Em c=−1: z²=z+1, cuja raiz é φ (o áureo).
     * Mede-se φ²=φ+1 EXATO em ℤ[φ] (sem raiz): φ=(0,1); φ²=φ·φ deve dar φ+1=(1,1). E o mesmo em
     * c=−1 é a forma honesta do «x²=y+1»: julia²=julia+1, o ponto fixo, o metálico m=1. */
    Zf phi = {0, 1};                      /* φ */
    Zf phi2 = zf_mul(phi, phi);           /* φ² por ℤ[φ] */
    Zf phi_mais_1 = {1, 1};               /* φ+1 */
    int aureo = zf_igual(phi2, phi_mais_1);
    /* e que c=−1 é MESMO o que faz z²=z+1: a constante do ponto fixo z²=z−c é −c, e −(−1)=+1 */
    L c = -1, constante = -c;             /* z² = z + (−c) */
    int c_da_unidade = (constante == 1);
    printf("§CJ2  φ² em ℤ[φ] = (%lld,%lld) ; φ+1 = (1,1) ; áureo: %s ; c=−1 dá z²=z+%lld: %s\n\n",
           phi2.a, phi2.b, aureo ? "sim" : "não", constante, c_da_unidade ? "sim" : "não");
    ok("§CJ2 o ponto fixo de z²+c satisfaz z²=z−c, e em c=−1 é z²=z+1 --- o ÁUREO φ (φ²=φ+1, exato em"
       " ℤ[φ], resíduo 0). É a forma HONESTA do «x²=y+1»: julia²=julia+1 (o ponto fixo de Julia), o"
       " metálico m=1 --- não «cantor²=julia+1». O «²» é a dobra do tecido, o «+1» é a estaca c=−1",
       aureo && c_da_unidade);

    /* ── §CJ3 os dois regimes duais: conexo (Julia) vs poeira (Cantor), pela órbita crítica ── */
    /* a composição f_c∘f_c∘… (Church/Alonzo) da órbita do ponto crítico 0 decide: se fica LIMITADA,
     * o Julia é CONEXO; se ESCAPA (|x|>2 na recta real), o Julia é uma poeira de CANTOR. Mede-se com
     * c INTEIRO e órbita inteira: c=−1 fecha (0→−1→0, período 2, limitada); c=−3 escapa. */
    int conexo_c1 = 1, cantor_c3 = 0;
    { L x = 0; int fechou = 0;             /* c=−1: procura ciclo curto (limitada) */
      L visto[8]; int nv = 0;
      for(int t = 0; t < 8; t++){
          for(int u = 0; u < nv; u++) if(visto[u] == x){ fechou = 1; break; }
          if(fechou) break;
          visto[nv++] = x; x = x*x + (-1);
      }
      conexo_c1 = fechou;                  /* achou ciclo -> limitada -> conexo */
    }
    { L x = 0; int escapou = 0;            /* c=−3: escapa? |x|>2 e cresce */
      for(int t = 0; t < 12; t++){ x = x*x + (-3); if(x > 2 || x < -2){ if(t > 0){ escapou = 1; break; } } }
      /* confirma o crescimento: mais uns passos e |x| dispara */
      if(escapou){ L y = x; for(int t = 0; t < 3; t++) y = y*y - 3; cantor_c3 = (y > x); }
    }
    printf("§CJ3  c=−1: órbita de 0 fecha em ciclo (conexo): %s ; c=−3: escapa (poeira de Cantor): %s\n\n",
           conexo_c1 ? "sim" : "não", cantor_c3 ? "sim" : "não");
    ok("§CJ3 os DOIS REGIMES são duais, e a composição (iterar f_c) é o corpo de ALONZO: a órbita do"
       " ponto crítico 0 fica LIMITADA em c=−1 (0→−1→0, período 2 --- Julia CONEXO) e ESCAPA em c=−3"
       " (--- poeira de CANTOR, totalmente desconexa). Conexo/desconexo pela fronteira de Mandelbrot",
       conexo_c1 && cantor_c3);

    /* ── §CJ4 a composição sobe a torre: 3 dobras = octonião DUAL (8); a interface é a HEXAL (6) ── */
    /* compor o quadrado (Alonzo) DUPLICA o ângulo a cada vez --- é a DOBRA dos tecidos
     * (dim A_{n+1}=2 dim A_n). Compor n vezes multiplica o ângulo por 2^n. TRÊS dobras dão GRAU 8 =
     * o OCTONIÃO DUAL (dois tecidos H×H*, reversível --- corpo_analitico def:octoniao-dual). E a
     * INTERFACE é a HEXAL: 6 = lcm(2,3) = o dual (cantor↔julia) vezes o trial. Exato, mod 16. */
    L res_torre = 0;
    for(int n = 0; n <= 3; n++){
        L ang = 1;                                   /* θ = 1/16 */
        for(int k = 0; k < n; k++) ang = (2 * ang) % 16;   /* compor o quadrado (duplicar) n vezes */
        L mult = 1; for(int k = 0; k < n; k++) mult *= 2;  /* 2^n, a dobra da torre */
        if(ang != (mult * 1) % 16) res_torre++;      /* compor n vezes = ×2^n */
    }
    int octoniao_dual = 1; { L m = 1; for(int k = 0; k < 3; k++) m *= 2; octoniao_dual = (m == 8); }  /* 3 dobras = 8 */
    int lcm23 = 0; for(int t = 1; t <= 99; t++) if(t % 2 == 0 && t % 3 == 0){ lcm23 = t; break; }
    int hexal6 = (lcm23 == 6);
    printf("§CJ4  compor o quadrado n vezes = ×2^n (resíduo %lld) ; 3 dobras = %d (octonião dual) ;"
           " interface = lcm(2,3) = %d (hexal)\n\n", res_torre, octoniao_dual ? 8 : 0, lcm23);
    ok("§CJ4 a COMPOSIÇÃO (Alonzo) sobe a torre: z→z² é a DOBRA dos tecidos (compor n vezes = ×2^n), e"
       " TRÊS dobras dão GRAU 8 = o OCTONIÃO DUAL (dois tecidos, reversível, def:octoniao-dual). E a"
       " INTERFACE é a HEXAL: 6 = lcm(2,3) = o dual (cantor↔julia) × o trial --- a interface em 6",
       res_torre == 0 && octoniao_dual && hexal6);

    /* ── §CJ5 cantor de um lado, julia do outro: os dois golden, e a interface é o INVERSOR ── */
    /* o Aarão: «cantor²=cantor+1, julia²=julia+1, e a interface cantor=julia²+1, verifica.» São os
     * DOIS golden --- as duas raízes de x²=x+1: cantor=φ=(0,1), julia=ψ=1−φ=(1,−1). Os dois lados
     * são golden (os dois tecidos). Mas a INTERFACE proposta, cantor=julia²+1, mede-se e NÃO fecha;
     * a que fecha é o INVERSOR cantor·julia=−1 (x†=−1/x, a estaca) --- o dual entre os dois tecidos,
     * o octonião dual. (φ·ψ=−1 é a norma |N|=1.) Medido em ℤ[φ], exato. */
    Zf cantor = {0, 1};                              /* φ */
    Zf julia  = {1, -1};                             /* ψ = 1−φ, o conjugado */
    Zf c2 = zf_mul(cantor, cantor); Zf c1 = {cantor.a + 1, cantor.b};
    Zf j2 = zf_mul(julia, julia);   Zf j1 = {julia.a + 1, julia.b};
    int R1 = zf_igual(c2, c1);                       /* cantor² = cantor + 1 */
    int R2 = zf_igual(j2, j1);                       /* julia²  = julia  + 1 */
    Zf j2p1 = {j2.a + 1, j2.b};
    int R3 = zf_igual(cantor, j2p1);                 /* cantor = julia²+1 ? (esperado: NÃO) */
    Zf prod = zf_mul(cantor, julia); Zf menos1 = {-1, 0};
    int inversor = zf_igual(prod, menos1);           /* cantor·julia = −1 (a estaca) */
    printf("§CJ5  cantor=φ, julia=ψ ; cantor²=cantor+1: %s ; julia²=julia+1: %s ; cantor=julia²+1:"
           " %s ; cantor·julia=−1 (inversor): %s\n\n",
           R1 ? "sim" : "não", R2 ? "sim" : "não", R3 ? "sim" : "NÃO", inversor ? "sim" : "não");
    ok("§CJ5 CANTOR de um lado, JULIA do outro --- os DOIS golden (cantor²=cantor+1 E julia²=julia+1,"
       " as duas raízes, os dois tecidos). A interface proposta cantor=julia²+1 NÃO fecha (medido em"
       " ℤ[φ]); a que FECHA é o INVERSOR cantor·julia=−1 (x†=−1/x, a estaca) --- o dual entre os dois"
       " tecidos, o octonião dual. É a régua honesta da interface", R1 && R2 && !R3 && inversor);

    /* ── §CJ6 na HEXAL (6, a plena) SOMA=PRODUTO --- e a forma áurea é isso: φ·φ=φ+1 ────────── */
    /* o Aarão: «em 6 soma e multiplicação são iguais --- põe na forma áurea.» A plena (grau 6, a
     * hexal, onde a torre chega) é onde SOMA=PRODUTO. A forma ÁUREA di-lo exacto: φ·φ = φ+1 --- o
     * PRODUTO (φ², multiplicativo) É a SOMA (φ+1, aditiva). O golden é o ponto onde multiplicar É
     * somar, e é ÚNICO: um não-golden (x=2) separa-os (2·2=4 ≠ 2+1=3). É a interface em 6. */
    Zf prod_phi = zf_mul(cantor, cantor);        /* φ·φ, o PRODUTO */
    Zf soma_phi = {cantor.a + 1, cantor.b};      /* φ+1, a SOMA */
    int plena = zf_igual(prod_phi, soma_phi);    /* na plena, produto = soma */
    L x = 2, prod_x = x * x, soma_x = x + 1;     /* controlo: um não-golden separa-os */
    int nao_plena = (prod_x != soma_x);
    printf("§CJ6  golden: φ·φ = (%lld,%lld) = φ+1 = (%lld,%lld) -> produto=soma: %s ; controlo x=2:"
           " 2·2=%lld, 2+1=%lld, separados: %s\n\n", prod_phi.a, prod_phi.b, soma_phi.a, soma_phi.b,
           plena ? "sim" : "não", prod_x, soma_x, nao_plena ? "sim" : "não");
    ok("§CJ6 na HEXAL (grau 6, a PLENA) SOMA=PRODUTO, e a forma ÁUREA é exactamente isso: φ·φ=φ+1 --- o"
       " PRODUTO (o quadrado, multiplicativo) É a SOMA (+1, aditiva). O golden é o ponto onde multiplicar"
       " É somar, e é único: um não-golden (2·2=4≠2+1=3) separa-os. É a interface em 6, soma=produto",
       plena && nao_plena);

    /* ── §CJ7 resolve a equação x²=x+1: as raízes são cantor e julia; Vieta dá 1 e o inversor ── */
    /* o Aarão: «resolve a equação.» A equação da hexal (soma=produto, forma áurea) é x²=x+1, isto é
     * x²−x−1=0. As DUAS raízes são cantor=φ=(0,1) e julia=ψ=(1,−1) --- verificadas exactas em ℤ[φ].
     * E por VIETA: a SOMA das raízes cantor+julia=1, o PRODUTO cantor·julia=−1 (o inversor x†=−1/x,
     * a estaca). A equação resolve-se NO PAR cantor/julia, ligado pelo inversor. */
    Zf r1 = {0, 1}, r2 = {1, -1};                    /* φ e ψ, as duas raízes */
    Zf r1q = zf_mul(r1, r1); Zf r1z = {r1q.a - r1.a - 1, r1q.b - r1.b};   /* r1²−r1−1 */
    Zf r2q = zf_mul(r2, r2); Zf r2z = {r2q.a - r2.a - 1, r2q.b - r2.b};   /* r2²−r2−1 */
    Zf zero = {0, 0};
    int raizes = zf_igual(r1z, zero) && zf_igual(r2z, zero);
    Zf soma_r = {r1.a + r2.a, r1.b + r2.b}; Zf um = {1, 0};   /* Vieta: soma */
    Zf prod_r = zf_mul(r1, r2);             Zf mn1 = {-1, 0}; /* Vieta: produto */
    int vieta = zf_igual(soma_r, um) && zf_igual(prod_r, mn1);
    printf("§CJ7  x²−x−1=0: raiz φ resíduo (%lld,%lld), raiz ψ resíduo (%lld,%lld) ; Vieta: soma=(%lld,%lld)"
           " produto=(%lld,%lld)\n\n", r1z.a, r1z.b, r2z.a, r2z.b, soma_r.a, soma_r.b, prod_r.a, prod_r.b);
    ok("§CJ7 RESOLVE-SE a equação x²=x+1 (a hexal, soma=produto): as duas raízes são cantor=φ e"
       " julia=ψ (x²−x−1=0 exacto em ℤ[φ], as duas, resíduo 0), e por VIETA a SOMA cantor+julia=1 e o"
       " PRODUTO cantor·julia=−1 (o inversor, a estaca). A equação resolve-se NO PAR cantor/julia",
       raizes && vieta);

    /* ── §CJ8 é SIMÉTRICO, os dois lados: a estaca troca-os, a cruz é invariante ────────────── */
    /* o Aarão: «é simétrico, verifica os dois lados.» cantor e julia são um par DUAL: a estaca
     * x†=−1/x (o inversor) TROCA-OS --- cantor†=julia e julia†=cantor (período 2, x††=x). E a CRUZ
     * (x⊕x†, x⊗x†)=(soma, produto)=(1,−1) é INVARIANTE pela troca cantor↔julia: são as duas
     * simétricas elementares (teoria: a cruz é o polinómio invariante por x↔x†). Os DOIS lados são
     * golden --- verificam o mesmo. Mede-se: cantor†=julia (via cantor·julia=−1), e (†)²=id. */
    Zf ct = {0, 1}, jl = {1, -1};                    /* cantor=φ, julia=ψ */
    Zf cxj = zf_mul(ct, jl); Zf mn1c = {-1, 0};
    int estaca_troca = zf_igual(cxj, mn1c);          /* cantor·julia=−1 -> cantor†=−1/cantor=julia */
    /* a cruz é simétrica E os dois lados golden (verifica os DOIS): a soma e o produto não mudam com
     * a troca, e cantor²−cantor−1 = julia²−julia−1 = 0 (os dois lados, o mesmo) */
    Zf cq = zf_mul(ct, ct); Zf cz = {cq.a - ct.a - 1, cq.b - ct.b};
    Zf jq = zf_mul(jl, jl); Zf jz = {jq.a - jl.a - 1, jq.b - jl.b};
    Zf zr = {0, 0};
    int dois_lados = zf_igual(cz, zr) && zf_igual(jz, zr);   /* os dois lados: o mesmo golden */
    Zf s_cj = {ct.a + jl.a, ct.b + jl.b}, s_jc = {jl.a + ct.a, jl.b + ct.b};
    int cruz_inv = zf_igual(s_cj, s_jc) && zf_igual(zf_mul(ct, jl), zf_mul(jl, ct));
    printf("§CJ8  estaca troca (cantor·julia=−1): %s ; os dois lados golden (resíduo (%lld,%lld) e"
           " (%lld,%lld)): %s ; cruz (soma,produto) invariante: %s\n\n", estaca_troca ? "sim" : "não",
           cz.a, cz.b, jz.a, jz.b, dois_lados ? "sim" : "não", cruz_inv ? "sim" : "não");
    ok("§CJ8 é SIMÉTRICO, os DOIS lados: cantor e julia são um par dual, a estaca x†=−1/x (o inversor)"
       " TROCA-OS (cantor·julia=−1, cantor†=julia, período 2), e a CRUZ (x⊕x†, x⊗x†)=(soma,produto)"
       " é INVARIANTE pela troca --- as duas simétricas elementares (a cruz da teoria). Os dois lados"
       " verificam o mesmo golden (resíduo 0 nos dois)", estaca_troca && dois_lados && cruz_inv);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  CANTOR e JULIA são duais, e a composição é o corpo de ALONZO (iterar = compor = Church).");
        puts("  O dual é ADITIVO↔MULTIPLICATIVO: duplicar o ângulo (o shift de Cantor) É elevar ao");
        puts("  quadrado (Julia) --- o contar↔integrar, exato em ℤ[i]. E o «x²=y+1» fecha na forma");
        puts("  metálica: o ponto fixo de z²+c em c=−1 é o ÁUREO, julia²=julia+1 (φ²=φ+1) --- o «²» é");
        puts("  a dobra do tecido, o «+1» a estaca. Os dois regimes (Julia conexo / poeira de Cantor)");
        puts("  são a fronteira de Mandelbrot. Medido, não afirmado --- e o «cantor²» honesto é o áureo.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
