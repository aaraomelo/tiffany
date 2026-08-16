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
/* O RESTO DA DIVISÃO DE BAREISS vigia-se aqui, e não em cada chamador. A divisão pelo
 * pivô anterior é EXACTA por teorema — os intermédios são menores da matriz —, logo um
 * resto não nulo não é imprecisão: é defeito, e conta-se. O `geral.c` tinha esta vigia
 * escrita à mão dentro do seu laço; agora quem chama a lib herda-a de graça. */
static long rt_bareiss_resto = 0;

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
            {
                long num = G[i*passo + j]*G[k*passo + k] - G[i*passo + k]*G[k*passo + j];
                if(num % ant != 0) rt_bareiss_resto++;      /* não pode acontecer */
                G[i*passo + j] = num / ant;
            }
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

/* ═══════════════════════════════════════════════════════════════════════════════════
 * A NORMALIZAÇÃO, E O QUE MAIS SE REPETE — o levantamento por padrão, segunda volta.
 *
 *      25×  o produto interno  Σ a[i]·b[i]        ← já era rt_dir, e 25 reescrevem-no
 *      23×  o módulo positivo  ((x%p)+p)%p
 *      15×  a transposta       M[j][i]
 *      11×  a norma²           Σ x[i]·x[i]        ← já era rt_norma
 *      11×  a potência de matriz  P ← P·C
 *       9×  a NORMALIZAÇÃO    v/‖v‖, v/z
 *       5×  a troca           t=a; a=b; b=t
 *       4×  o traço           Σ M[i][i]
 *       4×  a identidade      (i==j) ? 1 : 0
 *
 * ── E NORMALIZAR NÃO É DIVIDIR ──────────────────────────────────────────────────────
 *
 * É a peça que mais custa, e a que mais engana. Dividir por ‖v‖ traz uma RAIZ, a raiz
 * traz a vírgula, e a vírgula traz o limiar — foi assim que o `forca.c` acabou a medir
 * cos²+sin²=1 com uma régua de quinze casas, quando a identidade que ele queria era
 * Lagrange, e Lagrange é inteira.
 *
 * Em ℤ normalizar é GUARDAR O PAR. O cosseno ao quadrado entre dois vectores é
 *
 *      cos²(a,b)  =  ⟨a,b⟩² / (‖a‖²·‖b‖²)
 *
 * um racional EXACTO — numerador e denominador inteiros, sem uma raiz —, e as
 * comparações entre direcções fazem-se por produto cruzado. A raiz só apareceria se
 * alguém pedisse o cosseno em vez do seu quadrado, e ninguém precisa: as perguntas
 * («são paralelos?», «são perpendiculares?», «qual é mais alinhado?») respondem-se
 * todas no quadrado.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* o cos² como PAR (numerador, denominador) — a normalização sem dividir e sem raiz */
static void rt_cos2(const long *a, const long *b, int n, long *num, long *den){
    long d = rt_dir(a, b, n);
    *num = d*d;
    *den = rt_norma(a, n) * rt_norma(b, n);
}

/* e as três perguntas que a normalização servia, respondidas no quadrado:
 *   paralelos      ⟺ cos² = 1  ⟺  ⟨a,b⟩² = ‖a‖²‖b‖²   (Lagrange: o cruzado é nulo)
 *   perpendiculares⟺ cos² = 0  ⟺  ⟨a,b⟩ = 0
 *   mais alinhado  ⟺ compara-se cos² por PRODUTO CRUZADO, sem dividir */
static int rt_paralelos(const long *a, const long *b, int n){
    long nu, de; rt_cos2(a, b, n, &nu, &de);
    return nu == de;
}
static int rt_perp(const long *a, const long *b, int n){ return rt_dir(a, b, n) == 0; }
static int rt_mais_alinhado(const long *a, const long *b, const long *c, const long *d, int n){
    long n1, d1, n2, d2;
    rt_cos2(a, b, n, &n1, &d1);
    rt_cos2(c, d, n, &n2, &d2);
    return n1*d2 > n2*d1;                     /* cruzado, e sem uma divisão */
}

/* ── O MÓDULO POSITIVO: 23 cópias ────────────────────────────────────────────────────
 * Em C o resto herda o sinal do dividendo, logo −3 % 7 dá −3 e não 4. O representante
 * canónico em ℤ/pℤ é o não negativo — é a mesma dobra do sinal, um andar acima. */
static long rt_mod(long x, long p){ return ((x % p) + p) % p; }

/* ── A TRANSPOSTA: 15 cópias — e ela É o espelho τ ───────────────────────────────────
 * τ(M) = Mᵀ é a involução que reparte Dir e Cruz nas matrizes. Não é um utilitário: é
 * a dobra, escrita como operação. */
static void rt_transpoe(const long *M, int n, long *T){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) T[i*n + j] = M[j*n + i];
}

/* ── O PRODUTO E A POTÊNCIA DE MATRIZES: 11 cópias ───────────────────────────────────*/
static void rt_mul_mat(const long *A, const long *B, int n, long *C){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
        long s = 0;
        for(int k = 0; k < n; k++) s += A[i*n + k]*B[k*n + j];
        C[i*n + j] = s;
    }
}
/* (o parâmetro chamava-se `I`, e `I` é a unidade imaginária de <complex.h>: qualquer
 *  ficheiro que inclua os dois deixava de compilar. Um nome de uma letra num header é um
 *  nome que colide.) */
static void rt_identidade(long *Id, int n){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) Id[i*n + j] = (i == j);
}
static long rt_traco(const long *M, int n){
    long s = 0;
    for(int i = 0; i < n; i++) s += M[i*n + i];
    return s;
}

/* ═══════════════════════════════════════════════════════════════════════════════════
 * O CRUZADO, O BIVECTOR E A ORDEM SEM RAIZ — `thm:cruzado-potencia`.
 *
 * O teorema diz que o cruzado se transforma pelo determinante:
 *
 *      Cruz(Au, Av) = det(A)·Cruz(u,v)      e logo      (det A)^k na potência
 *      |det A| = 1  ⟹  |Cruz| INVARIANTE em toda a órbita
 *
 * e daqui a raiz sai de cena, porque a raiz é a INVERSA da potência e o cruzado não vê
 * nem uma nem outra. As perguntas métricas respondem-se no par (Dir, Cruz), ambos
 * inteiros, e o par fecha por LAGRANGE:
 *
 *      Dir(u,v)² + ‖u∧v‖² = Dir(u,u)·Dir(v,v)
 *
 * que é cos² + sin² = 1 sem nunca formar nem o cosseno nem o seno.
 *
 * ── E ESTAS SÃO AS CÓPIAS QUE ISTO RECOLHE ──────────────────────────────────────────
 *
 *      298  sqrt/hypot em 85 ficheiros      — quase todos a decidir uma ORDEM
 *       79  produto interno em laço, 39     — é rt_dir, e já cá estava
 *       33  o determinante 2×2 à mão, 19    — `a*d − b*c` e `u[0]*v[1] − u[1]*v[0]`
 *       14  norma ao quadrado em laço, 9    — é rt_norma
 *
 * A FRONTEIRA, dita: onde o valor da raiz é o RESULTADO — um comprimento a entregar ao
 * cliente —, ela faz-se no fim e fora do núcleo, como todo o resto da representação.
 * O que estas funções tiram são as raízes que apareciam para responder a uma pergunta
 * que nunca foi sobre o comprimento.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* O CRUZADO EM ℤ²: a entrada independente de Cruz, e É o determinante da matriz que os
 * dois vectores formam. Uma coisa, dois nomes — e é a área. */
static long rt_cruz2(const long *u, const long *v){ return u[0]*v[1] - u[1]*v[0]; }

/* o mesmo, com as quatro entradas soltas: o `a*d − b*c` que aparece 23 vezes no repo */
static long rt_det2(long a, long b, long c, long d){ return a*d - b*c; }

/* aplicar uma matriz n×n a um vector, e o caso 2×2 que é o da órbita */
static void rt_aplica(const long *M, const long *v, int n, long *r){
    for(int i = 0; i < n; i++){
        long s = 0;
        for(int j = 0; j < n; j++) s += M[i*n + j] * v[j];
        r[i] = s;
    }
}

/* ‖u∧v‖² PELA FORMA FECHADA: Dir(u,u)·Dir(v,v) − Dir(u,v)². É o que se usa; existe em
 * TODA a dimensão (ao contrário do produto vectorial, que só vive em 3 e 7 por Hurwitz),
 * e não se constrói a matriz n×n para um número que sai em três produtos. */
static long rt_bivetor2(const long *a, const long *b, int n){
    long aa = rt_dir(a, a, n), bb = rt_dir(b, b, n), ab = rt_dir(a, b, n);
    return aa*bb - ab*ab;
}

/* e PELA SOMA DAS COMPONENTES: Σ_{i<j} (aᵢbⱼ − aⱼbᵢ)², a norma de Frobenius do bivector.
 * É a SEGUNDA ROTA, e existe por uma razão precisa: sem ela, verificar Lagrange contra
 * `rt_bivetor2` seria comparar a definição consigo própria — foi esse o defeito que o
 * §S3 do semantico.c tinha, com um limiar de 1e-12 por cima a dar-lhe cara de medição.
 * Custa O(n²) e a outra custa O(n): usa-se para MEDIR, não para calcular. */
static long rt_bivetor_soma(const long *a, const long *b, int n){
    long s = 0;
    for(int i = 0; i < n; i++)
        for(int j = i + 1; j < n; j++){
            long c = a[i]*b[j] - a[j]*b[i];
            s += c*c;
        }
    return s;
}

/* LAGRANGE: devolve 1 se as duas rotas fecham. A identidade é HOMOGÉNEA de grau 4, logo
 * escalar os dois vectores por um factor comum não a move — é isso que permite medi-la
 * exacta sobre dados que chegaram em vírgula, depois de escalados a inteiros. */
static int rt_lagrange(const long *a, const long *b, int n){
    return rt_bivetor_soma(a, b, n) == rt_bivetor2(a, b, n);
}

/* A ORDEM SEM RAIZ: −1 se ‖a‖ < ‖b‖, +1 se maior, 0 se iguais. Não é uma aproximação da
 * comparação de normas: É ela, porque x ↦ x² é monótona nos não negativos e a pergunta
 * nunca foi sobre o comprimento — era sobre a ORDEM. Aqui estão as 298 raízes. */
static int rt_ordem_norma(const long *a, const long *b, int n){
    long na = rt_dir(a, a, n), nb = rt_dir(b, b, n);
    return na < nb ? -1 : (na > nb ? 1 : 0);
}

/* e a mesma pergunta sobre dois escalares não negativos, que é onde o sqrt aparecia para
 * comparar: |x| < |y| ⟺ x² < y², e os quadrados são inteiros */
static int rt_ordem_abs(long x, long y){
    long ax = x < 0 ? -x : x, ay = y < 0 ? -y : y;
    return ax < ay ? -1 : (ax > ay ? 1 : 0);
}

/* E O QUADRADO PERFEITO, que é a única pergunta em que a raiz é mesmo a resposta: existe
 * r inteiro com r² = x? Por busca binária em inteiros, sem uma única operação de vírgula.
 * Devolve 1 e escreve r, ou 0 se x não é quadrado (ou é negativo). É o que decide o
 * `rt_fixo_candidato`: D = m²+4 ser quadrado perfeito é o ponto fixo cair no racional. */
static int rt_raiz_exacta(long x, long *r){
    if(x < 0) return 0;
    if(x < 2){ if(r) *r = x; return 1; }
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    if(lo*lo != x) return 0;
    if(r) *r = lo;
    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════════════
 * A ENTREGA — o número sai em FRACÇÃO CONTÍNUA, e o cliente reconstrói o que quiser.
 *
 * O Aarão: «essa representação vai até ao fim e entrega assim mesmo em long int, daí ele
 * pode reconstruir depois em qualquer representação».
 *
 * É o `def:real` e o `prop:alfabeto` levados à saída. Um decimal escrito NÃO é um número
 * aproximado: «3.14159» é 314159/100000, um racional EXACTO, e o que o perde é convertê-lo
 * a `double` — 0,1 não é 1/10 em base dois, e nunca foi. A fracção contínua desse racional
 * é uma palavra de inteiros, sai por Euclides, e reconstrói o racional de volta sem perder
 * um bit. O que se entrega são longs; o que o cliente faz com eles é dele.
 *
 *      texto  ──►  p/q exacto  ──►  [a₀; a₁, …, aₙ]  ──►  p/q  ──►  a representação que quiser
 *
 * A fracção contínua propriamente já vive em `aritmetica.h` — `nt_fc`, `nt_convergentes`,
 * `nt_cmp`, `nt_mediante`, sobre ℕ e com a saturação contada à parte. Aqui está só o que
 * falta para a ponte: a leitura do TEXTO, o sinal, e a reconstrução.
 *
 * E O SINAL VAI À PARTE, que não é preguiça: é a Lei 1. ℤ = ℕ + o SINAL, e o sinal é a
 * DOBRA de espectro {+1,−1} — a mesma involução que reparte Dir e Cruz um andar acima.
 * A fracção contínua é a palavra sobre ℕ; o sinal é a dobra que a leva a ℤ.
 * ═══════════════════════════════════════════════════════════════════════════════════ */

#define RT_CF_MAX 48

typedef struct { int sinal; long a[RT_CF_MAX]; int n; int saturou; } RtCf;

/* «−3.14159» ⟼ sinal −1, p = 314159, q = 100000. EXACTO, e sem uma divisão de reais.
 * Devolve 1 se leu, 0 se o texto não é um decimal ou se os inteiros não chegam.
 * O expoente («1e3») também entra, e multiplica p ou q por 10 tantas vezes. */
static int rt_le_decimal(const char *s, int *sinal, long *p, long *q){
    int i = 0;
    while(s[i] == ' ' || s[i] == '\t') i++;
    *sinal = 1;
    if(s[i] == '-'){ *sinal = -1; i++; }
    else if(s[i] == '+') i++;
    long v = 0, d = 1;
    int digitos = 0;
    while(s[i] >= '0' && s[i] <= '9'){
        if(v > 922337203685477580L) return 0;              /* não cabe: diz-se, não se finge */
        v = v*10 + (s[i]-'0'); i++; digitos++;
    }
    if(s[i] == '.'){
        i++;
        while(s[i] >= '0' && s[i] <= '9'){
            if(v > 922337203685477580L || d > 922337203685477580L) return 0;
            v = v*10 + (s[i]-'0'); d *= 10; i++; digitos++;
        }
    }
    if(!digitos) return 0;
    if(s[i] == 'e' || s[i] == 'E'){
        i++;
        int es = 1;
        if(s[i] == '-'){ es = -1; i++; }
        else if(s[i] == '+') i++;
        int e = 0;
        while(s[i] >= '0' && s[i] <= '9'){ e = e*10 + (s[i]-'0'); i++; if(e > 30) return 0; }
        for(int k = 0; k < e; k++){
            if(es > 0){ if(v > 922337203685477580L) return 0; v *= 10; }
            else       { if(d > 922337203685477580L) return 0; d *= 10; }
        }
    }
    *p = v; *q = d;
    return 1;
}

/* p/q ⟼ a palavra [a₀; a₁, …, aₙ]. É EUCLIDES — a mesma descida do mdc, lida noutra coluna.
 * O sinal entra como está e não se mistura com os quocientes. */
static void rt_cf_de(int sinal, long p, long q, RtCf *c){
    c->sinal = (p == 0) ? 1 : sinal;
    c->n = 0; c->saturou = 0;
    long P = p < 0 ? -p : p, Q = q < 0 ? -q : q;
    while(Q != 0){
        if(c->n >= RT_CF_MAX){ c->saturou = 1; return; }    /* conta-se, não se corta calado */
        c->a[c->n++] = P / Q;
        long r = P % Q;
        P = Q; Q = r;
    }
}

/* e a VOLTA: da palavra ao racional, pela recorrência dos convergentes lida de trás para a
 * frente. Devolve 1, ou 0 se um produto não coubesse. */
static int rt_cf_para(const RtCf *c, long *p, long *q){
    if(c->n == 0){ *p = 0; *q = 1; return 1; }
    long P = 1, Q = 0;                                      /* ∞ = [1:0], como em rt_orbita */
    for(int k = c->n - 1; k >= 0; k--){
        long ak = c->a[k];
        if(ak != 0 && P > 4611686018427387903L / (ak > 1 ? ak : 1)) return 0;
        long np = ak*P + Q;
        Q = P; P = np;
    }
    *p = c->sinal * P; *q = Q;
    return 1;
}

/* A RECONSTRUÇÃO DO CLIENTE: de p/q sai o decimal com quantas casas ele pedir, por DIVISÃO
 * LONGA em inteiros — o resto é sempre menor que q, logo nada transborda e nada arredonda
 * por acaso. Escreve em `d` e devolve o número de bytes. Isto não é «converter a double e
 * imprimir»: é o cliente a ler a palavra na base que lhe apetecer. */
static int rt_escreve_decimal(int sinal, long p, long q, int casas, char *d, int max){
    int k = 0;
    if(q == 0){ if(max > 3){ d[0]='n'; d[1]='a'; d[2]='n'; d[3]=0; } return 3; }
    long P = p < 0 ? -p : p;
    if(sinal < 0 && (P != 0) && k < max-1) d[k++] = '-';
    long inteiro = P / q, r = P % q;
    char buf[24]; int nb = 0;
    if(inteiro == 0) buf[nb++] = '0';
    while(inteiro > 0){ buf[nb++] = (char)('0' + inteiro % 10); inteiro /= 10; }
    while(nb > 0 && k < max-1) d[k++] = buf[--nb];
    if(casas > 0 && k < max-1){
        d[k++] = '.';
        for(int i = 0; i < casas && k < max-1; i++){
            r *= 10;
            d[k++] = (char)('0' + r / q);
            r %= q;
        }
    }
    d[k] = 0;
    return k;
}

/* ═══════════════════════════════════════════════════════════════════════════════════
 * A INDUÇÃO E A META-INDUÇÃO — a recursão formalizada, e sem casos especiais.
 *
 * `def:inducao` e `thm:meta-inducao` do Corpo Universal. O par é o de sempre:
 *
 *      INDUÇÃO       o passo SOBE: base em 0, e P(n) ⟹ P(n+1)      projecta (λ⁺)
 *      DESCIDA       o dual: nega-se a tese — «há um PRIMEIRO andar N onde P falha» —
 *                    e mostra-se que esse N não existe                    lê (λ⁻)
 *      META-INDUÇÃO  o passo SOBRE os passos: mede a indução e valida o que ela
 *                    conservou
 *
 * As duas primeiras usam o mesmo facto e nada mais: ℕ é bem ordenado. São duais no
 * sentido desta casa — a involução é inverter a ordem, e ν∘ν = id: a indução diz o que
 * HÁ no andar seguinte, a descida diz o que NÃO HÁ em andar nenhum, e é a mesma frase.
 *
 * ── E O QUE A META-INDUÇÃO MEDE É O PASSO ───────────────────────────────────────────
 *
 * Se o passo C(n) → C(n+1) é uma construção cujo corpo NÃO MENCIONA n, então ∀n C(n)
 * segue da FORMA do passo. O resultado não é C(0), …, C(N):
 *
 *      ∀ n ≥ 0,  C(n)
 *
 * e a ausência de tecto é consequência da forma da construção, não de uma varredura.
 *
 *      «Uma tabela de andares prova os andares da tabela.»
 *
 * É a frase que esta sessão andou a aprender à sua custa: a cláusula do supremo estava
 * verificada numa janela de quarenta valores com a asserção a dizer «todos», e o
 * conserto não foi alargar a janela — foi medir o PASSO e deixar a indução dar o resto.
 *
 * ── E A SATURAÇÃO NÃO É UM RESULTADO ────────────────────────────────────────────────
 *
 *      falha de representação  ≠  contra-exemplo matemático
 *
 * Se a realização finita transborda ao k-ésimo passo, mediu-se o tamanho do tipo e não a
 * lei. A regra que daí sai é obrigatória: quando a primeira realização atinge o limite,
 * verifica-se numa SEGUNDA independente, e a saturação conta-se em lugar SEPARADO dos
 * defeitos. (É por isso que `rt_det_mod` existe ao lado de `rt_det_bareiss`.)
 * ═══════════════════════════════════════════════════════════════════════════════════ */

/* a INDUÇÃO: verifica o PASSO P(n) ⟹ P(n+1) em toda a gama, e a base. Devolve 1 se o
 * passo nunca falha; se falhar, escreve em `onde` o primeiro n em que falhou.
 * O passo recebe n e um contexto opaco — o corpo dele é que não pode mencionar n para
 * a meta-indução valer, e isso é do chamador, não desta função. */
static int rt_induz(int (*base)(void*), int (*passo)(long, void*),
                    long ate, void *ctx, long *onde){
    if(onde) *onde = -1;
    if(base && !base(ctx)){ if(onde) *onde = 0; return 0; }
    for(long n = 0; n < ate; n++)
        if(!passo(n, ctx)){ if(onde) *onde = n; return 0; }
    return 1;
}

/* a DESCIDA: o dual. Procura o PRIMEIRO andar onde P falha e devolve-o, ou −1 se não
 * existe. É a mesma varredura da indução lida ao contrário — e o que a torna prova é
 * voltar VAZIA quando o passo é verdadeiro, com um controlo que mostra que ela sabe
 * achar quando há. */
static long rt_desce(int (*P)(long, void*), long ate, void *ctx){
    for(long n = 0; n <= ate; n++) if(!P(n, ctx)) return n;
    return -1;
}

#endif /* RETA_H */
