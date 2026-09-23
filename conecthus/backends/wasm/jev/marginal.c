/* conecthus/backends/wasm/jev/marginal.c — JEV no motor (piloto E1–E5).
 * E1: o campo de contagem G(x) = |{i : pi(i) = x}| conserva |I|.
 * E2: as marginais da conjunta igualam as diretas (teorema das marginais do jev.tex),
 *     e cada projecao coincide com o oraculo (selo WASM ≡ oraculo).
 * E3: tres leituras tipadas (Noul/Choice/Score) sobre o mesmo G — composicao.
 * E4: Score fracionario como par racional exato (N, |I|), sem divisao no motor.
 * E5: Noul e Score no mesmo percurso sobre G (um disco, duas roupas); a
 *     segunda leitura nao recria G (funcao duas).
 * Particoes por fronteiras na arena: so comparacoes — o traduz nao recebe
 * div/mod variavel; o odometro dispensa divisao ao ler a conjunta em ordem.
 *
 * Arena (int32 a partir do byte 8):
 *   [0, X)      G(x)
 *   X           |I|
 *   X+1         n  (dimensoes tratadas; extras viram 1 classe)
 *   P = X+2     c0 c1 c2
 *   B = P+3     fronteiras: 3 blocos contiguos de (c_i+1): b_0..b_c (= X)
 *   D1 = B+S    marginais diretas, linha p em 16*p
 *   J = D1+48   conjunta em ordem odometro (ultima coordenada mais rapida)
 *   D2 = J+PROD marginais da conjunta, mesma linha em 16*p
 *   OUT = D2+48 OUT[0]=total G do campo; OUT[1]=|I| lido
 */
unsigned char arena[65536];

int proj(int b, int c, int x){
    int *a = (int *)arena;
    int y = -1;
    int bi;
    for (bi = 0; bi < c; bi++){
        int lo = a[b + bi];
        int hi = a[b + bi + 1];
        if (x >= lo && x < hi){ y = bi; }
    }
    return y;
}

int marginal(int X){
    int *a = (int *)arena;
    int P = X + 2;
    int c0 = a[P], c1 = a[P + 1], c2 = a[P + 2];
    int B = P + 3;
    int S = c0 + c1 + c2 + 3;
    int D1 = B + S;
    int J = D1 + 48;
    int PROD = c0 * c1 * c2;
    int D2 = J + PROD;
    int OUT = D2 + 48;
    int o, x, total;
    int bs0 = B;
    int bs1 = B + c0 + 1;
    int bs2 = B + c0 + 1 + c1 + 1;
    for (o = 0; o < 48; o++){ a[D1 + o] = 0; a[D2 + o] = 0; a[OUT + o] = 0; }
    for (o = 0; o < PROD; o++) a[J + o] = 0;
    total = 0;
    for (x = 0; x < X; x++){
        int g = a[x];
        int y0 = proj(bs0, c0, x);
        int y1 = proj(bs1, c1, x);
        int y2 = proj(bs2, c2, x);
        int idx = y2 + c2 * (y1 + c1 * y0);
        a[J + idx] = a[J + idx] + g;
        a[D1 + y0] = a[D1 + y0] + g;
        a[D1 + 16 + y1] = a[D1 + 16 + y1] + g;
        a[D1 + 32 + y2] = a[D1 + 32 + y2] + g;
        total = total + g;
    }
    {
        int k0 = 0, k1 = 0, k2 = 0;
        for (o = 0; o < PROD; o++){
            int v = a[J + o];
            a[D2 + k0] = a[D2 + k0] + v;
            a[D2 + 16 + k1] = a[D2 + 16 + k1] + v;
            a[D2 + 32 + k2] = a[D2 + 32 + k2] + v;
            k2 = k2 + 1;
            if (k2 == c2){ k2 = 0; k1 = k1 + 1; if (k1 == c1){ k1 = 0; k0 = k0 + 1; } }
        }
    }
    a[OUT] = total;
    a[OUT + 1] = a[X];
    return 0;
}

/* E4 — Score fracionario sem divisao no motor: o funcional
 * L(p_q)=Σ s_i p_q(s_i) sai como o par racional exato (N, |I|) com
 * N = Σ_i s_i G_q(s_i) = Σ_x s_{q(x)} G(x). O valor N/|I| é uma LEITURA
 * do par inteiro, feita fora (oraculo), nunca uma operacao fracionaria aqui.
 *
 * Arena (int32 a partir do byte 8):
 *   [0, X)      G(x)
 *   X           |I|
 *   X+1         m (niveis do Score)
 *   B = X+2     fronteiras do Score (m+1: b_0..b_m = X)
 *   W = B+m+1   pesos s_i (m ints)
 *   OUT = W+m   OUT[0]=N; OUT[1]=|I|; OUT[2]=m
 */
int escore(int X){
    int *a = (int *)arena;
    int m = a[X + 1];
    int B = X + 2;
    int W = B + m + 1;
    int OUT = W + m;
    int x, N = 0;
    for (x = 0; x < X; x++){
        int y = proj(B, m, x);
        N = N + a[W + y] * a[x];
    }
    a[OUT] = N;
    a[OUT + 1] = a[X];
    a[OUT + 2] = m;
    return 0;
}

/* E5 — Paralelismo: Noul e Score no mesmo percurso sobre G, um disco duas
 * roupas. A segunda leitura NAO recria G: o campo [0..X) fica intocado e
 * ambas as leituras saem de uma unica varredura. "Avaliadas em paralelo,
 * cada uma em isolamento" (a leitura conjunta coincide com as isoladas).
 *
 * Arena (int32 a partir do byte 8):
 *   [0, X)      G(x)
 *   X           |I|
 *   X+1         m (niveis do Score)
 *   Bn = X+2    fronteiras do Noul (c=2: 0, corte, X)
 *   Bs = X+5    fronteiras do Score (m+1)
 *   W = Bs+m+1  pesos s_i (m ints)
 *   OUT = W+m   [0..1] Noul G_q(0), G_q(1)
 *               [2..2+m) Score G_q por nivel
 *               [2+m]    N = Σ s_i G_q(s_i)
 *               [3+m]    D = |I|
 *               [4+m]    total recontado (sedo do mesmo percurso)
 */
int duas(int X){
    int *a = (int *)arena;
    int m = a[X + 1];
    int Bn = X + 2;
    int Bs = X + 5;
    int W = Bs + m + 1;
    int OUT = W + m;
    int o, x, N, total;
    for (o = 0; o < m + 5; o++) a[OUT + o] = 0;
    N = 0; total = 0;
    for (x = 0; x < X; x++){
        int g = a[x];
        int yn = proj(Bn, 2, x);
        int ys = proj(Bs, m, x);
        a[OUT + yn] = a[OUT + yn] + g;
        a[OUT + 2 + ys] = a[OUT + 2 + ys] + g;
        N = N + a[W + ys] * g;
        total = total + g;
    }
    a[OUT + m + 2] = N;
    a[OUT + m + 3] = a[X];
    a[OUT + m + 4] = total;
    return 0;
}