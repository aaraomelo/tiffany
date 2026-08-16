/* reta.h — AS OPERAÇÕES DA RECTA GEOMÉTRICA, TODAS INTEIRAS. Uma casa para elas.
 *
 * O Aarão: «vamos centralizar as operações matemáticas; cria essa biblioteca de funções
 * matemáticas da recta geométrica, tudo inteiro — deve facilitar essa centralização.»
 *
 * E o levantamento diz porquê. No repositório há 237 nomes de função definidos em mais
 * de um ficheiro, e os campeões são exactamente as operações desta recta:
 *
 *      28×  mdc          20×  mul          12×  norma        11×  pot
 *      26×  ok           19×  q            12×  mmc           9×  primo
 *
 * Vinte e oito cópias do máximo divisor comum — e três delas dentro da própria `lib/`,
 * com nomes diferentes: `al_mdc` (algebra.h), `iz_gcd` (inteiros.h) e `pl_mdc` (poli.h).
 * Não é desleixo de quem escreveu: é o que acontece quando não há um sítio.
 *
 * ── O QUE ESTE CABEÇALHO É, E O QUE NÃO É ────────────────────────────────────────────
 *
 * NÃO é a 238.ª cópia. Onde já existe uma canónica, ela é INCLUÍDA e apontada — e o que
 * fica aqui é só o que não tinha casa, mais o mapa de quem é quem. Reescrever o que já
 * está medido seria trocar uma duplicação por duas.
 *
 * ── O MAPA: onde vive cada operação, e qual é a canónica ─────────────────────────────
 *
 *   mdc, mmc, Bézout           `inteiros.h`  iz_gcd, iz_diofantina        ← A CANÓNICA
 *   racionais p/q              `racionais.h` qz, qz_soma, qz_mult, qz_menor
 *   polinómios em ℤ[x]         `poli.h`      pz_mul (convolução), pz_div_exata,
 *                                            pol_sturm_reais, pz_beta_pisot
 *   matrizes, posto, núcleo    `linear.h`    mat_posto, mat_nucleo, mat_inversa
 *   a régua e os convergentes  `aritmetica.h` nt_fc, nt_convergentes
 *   o corpo dual σ, σ†         `corpos.h`, `dual32.h`
 *
 * E o que ESTA casa acrescenta, porque não estava em lado nenhum e nasceu medido nesta
 * migração — cada uma com o medidor onde foi provada:
 *
 *   rt_det_bareiss   o determinante EXACTO de ordem qualquer, sem divisão inexacta
 *   rt_det_mod       o mesmo em 𝔽ₚ, para quando os intermédios não cabem
 *   rt_inv_mod       o inverso em 𝔽ₚ por Fermat (7 cópias no repo antes disto)
 *   rt_ipow          a potência inteira (5 cópias antes disto)
 *   rt_orbita        a órbita de ∞ — os convergentes, [p:q] ⟼ [m·p+q : p]
 *   rt_reverte       a REVERSÃO dos coeficientes: o dual do polinómio
 *   rt_hurwitz_est   o regime pelos coeficientes: Routh–Hurwitz, sem calcular raiz
 *
 * ── A REGRA DA CASA, e ela vale aqui como em todo o lado ─────────────────────────────
 *
 * Tudo inteiro. Onde aparece uma divisão, ou ela é EXACTA (e prova-se) ou o resultado é
 * um par (numerador, denominador) e a comparação faz-se por produto cruzado. Não há
 * limiar nenhum neste ficheiro: «é zero» é uma pergunta sobre o número, e «é menor que
 * a régua que escolhi» é uma pergunta sobre a régua.
 */
#ifndef RETA_H
#define RETA_H

#include "inteiros.h"      /* iz_gcd, iz_diofantina — a canónica do mdc */

#ifndef RT_MAX
#define RT_MAX 64          /* a ordem máxima das matrizes aqui — e ela VERIFICA-SE */
#endif

/* ── A POTÊNCIA INTEIRA ──────────────────────────────────────────────────────────────
 * Cinco cópias no repositório antes disto. Sem vírgula e sem `pow`. */
static long rt_ipow(long base, int e){
    long r = 1;
    while(e-- > 0) r *= base;
    return r;
}

/* ── O INVERSO EM 𝔽ₚ, POR FERMAT ────────────────────────────────────────────────────
 * a^(p−2) mod p. Sete cópias antes disto. Nenhuma divisão. */
static long rt_inv_mod(long a, long p){
    long r = 1, e = p - 2;
    a = ((a % p) + p) % p;
    while(e){ if(e & 1) r = r*a % p; a = a*a % p; e >>= 1; }
    return r;
}

/* ── O DETERMINANTE EXACTO, DE ORDEM QUALQUER — BAREISS ──────────────────────────────
 * A eliminação de Gauss divide, e por isso pede vírgula e um limiar para o pivô. A de
 * Bareiss divide pelo pivô ANTERIOR, e essa divisão é exacta em ℤ — é o teorema. Fica
 * tudo inteiro, e o limiar do pivô desaparece: `== 0` é a pergunta certa.
 *
 * O CUIDADO, que custou uma asserção falhada no `matricial.c` §M11: os intermédios de
 * Bareiss são MENORES da matriz, e crescem muito mais que o resultado. Numa 5×5 com
 * entradas da ordem de 10^10 chegam a 10^42 e o `long` enrola calado — o determinante
 * final cabia (10^14) e saía errado à mesma. Quando as entradas forem grandes, usar
 * `rt_det_mod` em vários primos.
 *
 * A matriz é dada em G com passo `passo`, e é CONSUMIDA. */
static long rt_det_bareiss(long *G, int n, int passo){
    long ant = 1;
    int sinal = 1;
    for(int k = 0; k < n - 1; k++){
        if(G[k*passo + k] == 0){                       /* troca por uma linha com pivô */
            int t = -1;
            for(int i = k + 1; i < n; i++) if(G[i*passo + k]){ t = i; break; }
            if(t < 0) return 0;                        /* coluna nula: o det É zero */
            for(int j = 0; j < n; j++){
                long v = G[k*passo + j];
                G[k*passo + j] = G[t*passo + j];
                G[t*passo + j] = v;
            }
            sinal = -sinal;
        }
        for(int i = k + 1; i < n; i++)
            for(int j = k + 1; j < n; j++)
                G[i*passo + j] = (G[i*passo + j]*G[k*passo + k]
                                  - G[i*passo + k]*G[k*passo + j]) / ant;
        ant = G[k*passo + k];
    }
    return sinal*G[(n-1)*passo + (n-1)];
}

/* ── O MESMO DETERMINANTE EM 𝔽ₚ ──────────────────────────────────────────────────────
 * Para quando os intermédios de Bareiss não cabem: em 𝔽ₚ nada cresce. */
static long rt_det_mod(long *G, int n, int passo, long p){
    long d = 1;
    for(int k = 0; k < n; k++){
        int piv = -1;
        for(int i = k; i < n; i++) if(((G[i*passo+k] % p) + p) % p){ piv = i; break; }
        if(piv < 0) return 0;
        if(piv != k){
            for(int j = 0; j < n; j++){
                long t = G[k*passo+j]; G[k*passo+j] = G[piv*passo+j]; G[piv*passo+j] = t;
            }
            d = (p - d) % p;
        }
        long a = ((G[k*passo+k] % p) + p) % p;
        d = d*a % p;
        long iv = rt_inv_mod(a, p);
        for(int i = k+1; i < n; i++){
            long f = ((G[i*passo+k] % p) + p) % p * iv % p;
            if(!f) continue;
            for(int j = k; j < n; j++)
                G[i*passo+j] = (((G[i*passo+j] - f*G[k*passo+j]) % p) + p) % p;
        }
    }
    return d;
}

/* ── A ÓRBITA DE ∞: OS CONVERGENTES ──────────────────────────────────────────────────
 * O paper (prop:orbita) diz que a acção é a matriz vezes o vector, «sem uma divisão»:
 *
 *      [p:q] ⟼ [m·p + q : p],   a partir de [1:0] = ∞
 *
 * e o que sai é exactamente a recorrência dos convergentes, com |det| = 1 em todos os
 * passos. É assim que o ponto fixo se MOSTRA — porque ele não cabe no andar (é isso o
 * corte), e um decimal truncado não é o objecto: é outro número. */
static void rt_orbita(long m, int k, long *p, long *q){
    long P = 1, Q = 0;                       /* ∞ = [1:0], pela Lei 0 */
    for(int t = 0; t < k; t++){ long np = m*P + Q; Q = P; P = np; }
    *p = P; *q = Q;
}

/* ── A REVERSÃO DOS COEFICIENTES: O DUAL DO POLINÓMIO ────────────────────────────────
 * p*(x) = xⁿ·p(1/x) — os coeficientes ao contrário. É ela que troca DENTRO por FORA no
 * disco, e é por isso que contar UM zero basta em vez de contar n−1 (Rouché no dual,
 * `matricial.c` §M17). E é ela que faz do RECÍPROCO do ouro a raiz do polinómio
 * revertido, que é a autodualidade do `solar.c`. */
static void rt_reverte(const long *a, int n, long *r){
    for(int k = 0; k <= n; k++) r[k] = a[n-k];
}

/* ── O REGIME PELOS COEFICIENTES: ROUTH–HURWITZ ──────────────────────────────────────
 * Todas as raízes com Re < 0 ⟺ os menores principais da matriz de Hurwitz são todos
 * positivos. Sai dos COEFICIENTES, e não precisa de calcular raiz nenhuma.
 *
 * O QUE ELE NÃO DECIDE, e diz-se: com todos os menores nulos, ele não distingue a BORDA
 * (raízes no eixo imaginário) do CAOS. Para isso é preciso o segundo passo — procurar um
 * factor x² + c com c > 0 por divisão exacta —, e é o `geral.c` §G4 que os junta.
 *
 * a[] vai do maior grau ao menor, com n = grau. Devolve 1 se estável. */
static int rt_hurwitz_est(const long *a, int n){
    for(int m = 1; m <= n; m++){
        long H[RT_MAX*RT_MAX];
        for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
            int idx = n - 2*(i+1) + (j+1);
            H[i*m + j] = (idx >= 0 && idx <= n) ? a[n-idx] : 0;
        }
        if(rt_det_bareiss(H, m, m) <= 0) return 0;
    }
    return 1;
}

/* ── O TECTO VERIFICA-SE, e não se documenta ─────────────────────────────────────────
 * Um `#define` que ninguém testa é documentação, não limite — e já pôs esta casa a não
 * terminar uma vez. Quem usa `rt_det_bareiss` com n > RT_MAX escreve fora do array; esta
 * função diz onde está a fronteira, e o medidor que a chama prova que ela é respeitada. */
static int rt_cabe(int n){ return n > 0 && n <= RT_MAX; }

#endif /* RETA_H */
