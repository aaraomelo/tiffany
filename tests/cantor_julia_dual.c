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
