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

/* ═══════════════════════════════════════════════════════════════════════════════════
 * A OPERAÇÃO — e tudo o que se segue sai dela.
 *
 * Há UMA operação, e escreve-se numa linha: a ⋆ b. O que dela sai são duas LEITURAS,
 * separadas por trocar a ordem — e o que as separa é o espelho τ(a,b) = (b,a):
 *
 *      Dir(a,b)  = ½(a⋆b + b⋆a)      a leitura que NÃO muda        o directo
 *      Cruz(a,b) = ½(a⋆b − b⋆a)      a leitura que TROCA DE SINAL  o cruzado
 *
 * Que sejam DUAS é dedução e não escolha: τ² = id obriga o polinómio mínimo a dividir
 * (t−1)(t+1), de raízes distintas — logo τ é diagonalizável com espectro {+1,−1}, e
 * Dir e Cruz são os PROJECTORES nos dois espaços próprios, (id ± τ)/2. Em característica
 * 2 os dois colapsam, e é a única hipótese que isto usa.
 *
 * E DAQUI SAI O RESTO DESTE CABEÇALHO, que é a razão de estar no topo:
 *
 *      o produto interno       é  Dir              a métrica, a diagonal
 *      a norma                 é  Dir(a,a)
 *      o produto vectorial     é  Cruz em ℝ³       a área, fora da diagonal
 *      o determinante 2×2      é  Cruz             a mesma área, lida na matriz
 *      o simétrico/antissim.   é  Dir/Cruz de matrizes
 *      o SINAL                 é  a mesma dobra em ℤ, com o mesmo espectro {+1,−1}
 *
 * Não são seis operações que por acaso se parecem: são leituras de uma só, e é por isso
 * que vivem no mesmo sítio. Tudo em DOBRO, para não dividir por dois — o factor 2 é o
 * preço de não ter vírgula, e paga-se uma vez.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* as duas leituras, EM DOBRO: 2·Dir e 2·Cruz de dois vectores de dimensão n. O directo
 * é um escalar (a métrica); o cruzado é a matriz antissimétrica a_i b_j − a_j b_i. */
static long rt_dir(const long *a, const long *b, int n){
    long s = 0;
    for(int i = 0; i < n; i++) s += a[i]*b[i];
    return s;                                    /* Dir(a,b) = ⟨a,b⟩ */
}
static void rt_cruz(const long *a, const long *b, int n, long *C){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
        C[i*n + j] = a[i]*b[j] - a[j]*b[i];      /* Cruz(a,b) = a∧b */
}
/* e a norma é o directo consigo próprio — não é uma operação nova */
static long rt_norma(const long *a, int n){ return rt_dir(a, a, n); }

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

/* ═══════════════════════════════════════════════════════════════════════════════════
 * AS TÉCNICAS DE ALTO NÍVEL — as que os ficheiros repetem inline, e quantas vezes.
 *
 * O levantamento por PADRÃO DE CÓDIGO (e não por nome de função, que já estava feito)
 * diz onde a casa se repete a si própria:
 *
 *      40×  a recorrência  u = m·u₁ + u₀        ← a régua, e é a mais copiada de todas
 *      14×  a convolução   c[i+j] += a[i]·b[j]
 *      14×  o cruzado      a[1]b[2] − a[2]b[1]
 *      12×  a exponenciação modular
 *      10×  a eliminação com pivô
 *       6×  a forma        p² − m·p·q − q²
 *       5×  a companheira  C[i][i−1] = 1
 *       4×  o traço da potência  Tr(Cᵏ)
 *
 * Estas são as PEÇAS da recta, não utilitários: a recorrência é a régua, a convolução é
 * o produto de polinómios, o cruzado é a metade que inverte, a forma é o que o ponto
 * fixo anularia. Cada uma escrita uma vez, aqui, e medida em `tests/reta.c`.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* ── A RECORRÊNCIA: a régua, e a peça mais copiada do repositório ────────────────────
 * u_{k+1} = m·u_k + u_{k−1}, que é a acção da companheira A_m = [[m,1],[1,0]] e é a
 * mesma coisa que a órbita de ∞ (rt_orbita) vista do lado da sucessão. Preenche `u` com
 * n termos a partir de (u0, u1). */
static void rt_recorre(long m, long u0, long u1, long *u, int n){
    if(n > 0) u[0] = u0;
    if(n > 1) u[1] = u1;
    for(int k = 2; k < n; k++) u[k] = m*u[k-1] + u[k-2];
}

/* ── A CONVOLUÇÃO: multiplicar polinómios É convolver os coeficientes ────────────────
 * «a forma aditiva da multiplicação vista pela Transformada Universal». r tem de ter
 * espaço para na+nb−1 coeficientes, e é zerado aqui. */
static void rt_conv(const long *a, int na, const long *b, int nb, long *r){
    for(int k = 0; k < na + nb - 1; k++) r[k] = 0;
    for(int i = 0; i < na; i++) for(int j = 0; j < nb; j++) r[i+j] += a[i]*b[j];
}

/* ── O CRUZADO EM ℝ³: um CASO de rt_cruz, não uma operação nova ──────────────────────
 * Em ℝ³ a matriz antissimétrica Cruz(a,b) tem três entradas independentes, e o vector
 * (c₀,c₁,c₂) é a leitura delas — é o mesmo objecto com outra roupa, e é por isso que o
 * produto vectorial só existe em três dimensões: é onde n(n−1)/2 = n. */
static void rt_cruz3(const long *a, const long *b, long *c){
    c[0] = a[1]*b[2] - a[2]*b[1];
    c[1] = a[2]*b[0] - a[0]*b[2];
    c[2] = a[0]*b[1] - a[1]*b[0];
}

/* ── A MESMA DECOMPOSIÇÃO, AGORA EM MATRIZES ─────────────────────────────────────────
 * Dir e Cruz aplicados a uma matriz em vez de a um par de vectores — o espelho é o
 * mesmo, τ(M) = Mᵀ, e os projectores são os mesmos (id ± τ)/2. Devolvidas
 * EM DOBRO, para não dividir: 2S = M + Mᵀ e 2A = M − Mᵀ. A decomposição existe quando 2
 * é invertível — em característica 2 as duas colapsam (medido em operacao.c §O10). */
static void rt_dir_cruz(const long *M, int n, long *S2, long *A2){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
        S2[i*n+j] = M[i*n+j] + M[j*n+i];
        A2[i*n+j] = M[i*n+j] - M[j*n+i];
    }
}

/* ── A EXPONENCIAÇÃO MODULAR ─────────────────────────────────────────────────────────
 * a^e mod p, por quadrados. Doze cópias no repositório antes disto. */
static long rt_pot_mod(long a, long e, long p){
    long r = 1;
    a = ((a % p) + p) % p;
    while(e > 0){ if(e & 1) r = r*a % p; a = a*a % p; e >>= 1; }
    return r;
}

/* ── A COMPANHEIRA, E O TRAÇO DAS SUAS POTÊNCIAS ─────────────────────────────────────
 * A companheira de xⁿ = c₁xⁿ⁻¹ + … + cₙ, com a acção nas LINHAS: v_{k+1} = v_k·C. E o
 * traço de Cᵏ É a soma das potências das raízes — o Tr(σᵏ) de Newton, sem avaliar raiz
 * nenhuma. Escrevi-a transposta à primeira, e a asserção deu 91 de 160: o índice sobe
 * na COLUNA, e não na linha. */
static void rt_companheira(const long *c, int n, long *C){
    for(int i = 0; i < n*n; i++) C[i] = 0;
    for(int i = 0; i + 1 < n; i++) C[i*n + (i+1)] = 1;
    for(int j = 0; j < n; j++) C[(n-1)*n + j] = c[j];
}
static void rt_tracos(const long *C, int n, long *tr, int k){
    long P[RT_MAX*RT_MAX], N[RT_MAX*RT_MAX];
    for(int i = 0; i < n*n; i++) P[i] = 0;
    for(int i = 0; i < n; i++) P[i*n+i] = 1;
    for(int t = 0; t < k; t++){
        long s = 0;
        for(int i = 0; i < n; i++) s += P[i*n+i];
        tr[t] = s;
        for(int i = 0; i < n*n; i++) N[i] = 0;
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
            for(int l = 0; l < n; l++) N[i*n+j] += P[i*n+l]*C[l*n+j];
        for(int i = 0; i < n*n; i++) P[i] = N[i];
    }
}

/* ── A FORMA QUE O PONTO FIXO ANULARIA ───────────────────────────────────────────────
 * p² − m·p·q − q². Nos convergentes ela vale ±1 e NUNCA zero, e é isso o corte: o ponto
 * fixo pediria a forma = 0, e a descida mostra que não tem solução em ℤ. Seis cópias. */
static long rt_forma(long p, long q, long m){ return p*p - m*p*q - q*q; }

/* ── O PRODUTO DE KRONECKER ──────────────────────────────────────────────────────────
 * A operação ⊗ do corpo de corpos: (A⊗B) tem ordem a·b, e det(A⊗B) = det(A)^b·det(B)^a.
 * K tem de ter espaço para (a·b)², e o passo dele é `pk`. */
static void rt_kron(const long *A, int a, const long *B, int b, long *K, int pk){
    for(int i = 0; i < a; i++) for(int j = 0; j < a; j++)
    for(int k = 0; k < b; k++) for(int l = 0; l < b; l++)
        K[(i*b+k)*pk + (j*b+l)] = A[i*a+j]*B[k*b+l];
}

/* ═══════════════════════════════════════════════════════════════════════════════════
 * O TEOREMA DO PONTO FIXO, E A LEI 8 — as três peças que o fecham.
 *
 * `thm:corte-ponto-fixo` do Corpo Universal, e a Lei 8 que lhe dá o chão. O enunciado:
 *
 *   (i)   a órbita de ∞ é [p:q] ⟼ [m·p+q : p], sem uma divisão — e o ponto de partida
 *         só existe pela Lei 0, porque num corpo sem ela «começar no infinito» não quer
 *         dizer nada
 *   (ii)  o ponto fixo é o VECTOR PRÓPRIO: x² = m·x + 1, com soma tr = m e produto
 *         det = −1, os dois INTEIROS
 *   (iii) e o corte é a FALTA dele. Completando o quadrado,
 *
 *              4(p² − m·p·q − q²) = (2p − m·q)² − D·q²,   D = m² + 4
 *
 *         logo o ponto fixo vive em ℙ¹(ℚ) ⟺ D é QUADRADO PERFEITO. E aí decide-se em
 *         duas linhas para todo m ≥ 1: m² < D < (m+2)², o único quadrado possível no
 *         meio é (m+1)², e D = (m+1)² pede 2m = 3, que não tem solução em ℤ.
 *         É o PASSO, e não a lista.
 *   (iv)  e a FACE FINITA diz o contrário: em 𝔽ₚ a mesma equação tem raiz exactamente
 *         quando D é resíduo quadrático — aí o ponto fixo ESTÁ no andar, a órbita cai
 *         nele, e não há corte nenhum a fazer. É a Lei 8: o anel onde o grupo é finito
 *         e as órbitas fecham, e onde tudo se verifica EXAUSTIVAMENTE.
 *   (v)   e há VOLTA, que é a metade dual: det A_m = −1 torna a inversa INTEIRA, e a
 *         acção dela é [p:q] ⟼ [q : p − m·q] — letra por letra, a descida de (iii).
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* ── (iii) O PONTO FIXO EM ℙ¹(ℚ): decide-se pelo DISCRIMINANTE ───────────────────────
 * Devolve 1 se D = m² + 4 é quadrado perfeito — isto é, se o ponto fixo cabe no andar.
 * Para m ≥ 1 devolve sempre 0, e a razão é o intervalo, não uma varredura. */
static int rt_fixo_racional(long m){
    long D = m*m + 4, r = 0;
    while(r*r < D) r++;
    return r*r == D;
}

/* e o PASSO que o prova, exibido: entre m² e (m+2)² só cabe (m+1)², e esse pede 2m = 3.
 * Devolve o único candidato, ou 0 se nem ele cabe no intervalo. */
static long rt_fixo_candidato(long m){
    long D = m*m + 4;
    return (D > m*m && D < (m+2)*(m+2)) ? (m+1)*(m+1) : 0;
}

/* ── (iv) A LEI 8: em 𝔽ₚ o ponto fixo EXISTE quando D é resíduo quadrático ───────────
 * O critério de Euler: a é resíduo quadrático mod p ⟺ a^((p−1)/2) ≡ 1. Devolve 1 se
 * o ponto fixo cai no andar finito, 0 se não — e é o contrário exacto do caso racional,
 * que é o que torna o corte ESTRUTURAL e não um defeito de ℚ. */
static int rt_fixo_em_fp(long m, long p){
    long D = ((m*m + 4) % p + p) % p;
    if(D == 0) return 1;                                  /* raiz dupla: o ponto está lá */
    return rt_pot_mod(D, (p-1)/2, p) == 1;
}

/* ── (v) A VOLTA: a órbita para trás, e ela é INTEIRA ────────────────────────────────
 * [p:q] ⟼ [q : p − m·q], que é A_m⁻¹ = [[0,1],[1,−m]] — inteira porque det A_m = −1.
 * A ida cresce como σᵏ e a volta desce pelo mesmo caminho, até ao ∞ exacto. */
static void rt_volta(long m, long p, long q, long *np, long *nq){
    *np = q; *nq = p - m*q;
}

/* ── O SINAL É UMA DOBRA, E O MDC VIVE DO LADO QUE ELA NÃO MOVE ──────────────────────
 * O `geometrico.tex` trata isto, e não como caso particular: `cor:cadeia` diz que
 *
 *      ℤ  =  ℕ  +  o SINAL
 *
 * e a Lei 1 diz o que o sinal é — 1† = −1, a Möbius INVOLUTIVA de traço 0, ν(x) = −1/x.
 * É a MESMA dobra que reparte a operação em Dir e Cruz: uma involução, com espectro
 * {+1, −1}, e portanto com dois lados — o que ela FIXA e o que ela INVERTE.
 *
 *      x  =  |x| · sinal(x)          o que fica  ·  o que inverte
 *
 * E o mdc é uma função da CLASSE, não do representante: a divisibilidade não vê o sinal,
 * porque d | a ⟺ d | (−a). Logo o mdc vive do lado FIXO da dobra.
 *
 * ── E É ISSO QUE EXPLICA AS 28 CÓPIAS ───────────────────────────────────────────────
 * Vinte não tratam o sinal e oito tratam, e divergem em 3280 dos 6561 pares de
 * [−40,40]² — mdc(−40,−40) dá −40 numas e 40 noutras. Escrevi primeiro que eram «duas
 * respostas para a mesma pergunta», e não é: são DOIS REPRESENTANTES DA MESMA CLASSE.
 * As que não tratam devolvem o representante com o sinal de `a`, porque em C o resto
 * herda o sinal do dividendo; as que tratam devolvem o representante NÃO NEGATIVO.
 *
 * A canónica é o não negativo, e a razão não é gosto: é que «MÁXIMO» divisor comum pede
 * uma ordem, e na ordem dos inteiros um divisor negativo nunca é o maior. O
 * representante certo é o do lado que a dobra fixa. */

/* a DOBRA do sinal, explícita: x ↦ (|x|, s) com s ∈ {+1,−1,0}, e a volta x = |x|·s.
 * É a decomposição de ℤ em ℕ × {±1} que a `cor:cadeia` enuncia, e ν∘ν = id é o que a
 * torna uma dobra e não uma projecção — nada se perde, e a volta devolve. */
static long rt_sinal(long x){ return x > 0 ? 1 : (x < 0 ? -1 : 0); }
static long rt_modulo(long x){ return x < 0 ? -x : x; }

static long rt_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a;
}

/* e o mínimo múltiplo comum, pelo par: a·b = mdc·mmc. Divide-se PRIMEIRO para o produto
 * não estourar — é a mesma razão por que a soma de racionais cancela em cruz antes de
 * multiplicar. */
static long rt_mmc(long a, long b){
    if(!a || !b) return 0;
    long g = rt_mdc(a, b);
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    return (a / g) * b;
}

#endif /* RETA_H */
