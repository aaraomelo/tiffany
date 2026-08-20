/* numeros.h — TEORIA DOS NÚMEROS: E É TUDO A MESMA ÓRBITA.
 *
 *  ordem do coordenador fecha o andar com a frase que o organiza todo:
 *
 *     Euclides = MDC = Bézout = FC        «são diferentes saídas da mesma órbita»
 *
 * E é literal, não é slogan: a descida a = bq + r produz, do MESMO rastro, o máximo
 * divisor comum (o último resto não nulo), os coeficientes de Bézout (subindo a cadeia) e
 * os termos da fração contínua (os quocientes). Uma órbita, três leituras — o que esta
 * casa chama o rastro que carrega mais do que a folha.
 *
 * Por isso aqui quase nada é motor novo: o `iz_gcd` já é o Euclides estendido, o
 * `nt_fatora` já dá a fatoração, o `lado` (cifra.h) já faz a FC dos irracionais
 * quadráticos. Falta a FC dos RACIONAIS (que é a descida nua), o inverso modular, o φ, o
 * μ e o Teorema Chinês. E tudo em inteiros — nem aqui entra decimal.
 *
 * Precisa de `inteiros.h` e `naturais.h`. */
#ifndef NUMEROS_H
#define NUMEROS_H

/* ── O INVERSO MODULAR, por Euclides ESTENDIDO e não por tentativa ─────────────
 * «Use Euclides estendido, não tentativa» — e a diferença não é de estilo: a tentativa
 * custa n passos e a órbita custa log n. Existe ⟺ gcd(a,n) = 1, e a testemunha é o x
 * de Bézout reduzido. */
static int nm_inv_mod(long a, long n, long *x){
    if(n <= 1) return 0;
    long u, v, g = iz_gcd(a, n, &u, &v);
    if(g != 1) return 0;                       /* sem inverso, e o gcd diz porquê */
    long r = u % n;
    if(r < 0) r += n;
    if(x) *x = r;
    return 1;
}
/* ── φ DE EULER, pela fórmula do produto sobre os primos ───────────────────────
 * φ(n) = n·∏(1 − 1/p), e faz-se em INTEIROS: n/p·(p−1) por cada primo distinto — a
 * divisão é exata porque p | n. */
static long nm_phi(long n){
    if(n < 1) return 0;
    long pr[NT_FAT]; int ex[NT_FAT];
    int k = nt_fatora(n, pr, ex, NT_FAT);
    long r = n;
    for(int i = 0; i < k; i++) r = r / pr[i] * (pr[i] - 1);
    return r;
}
/* e o SEGUNDO caminho: contar os coprimos à mão. Têm de concordar. */
static long nm_phi_conta(long n){
    long c = 0;
    for(long k = 1; k <= n; k++) if(iz_gcd(k, n, 0, 0) == 1) c++;
    return c;
}
/* ── μ DE MÖBIUS ──────────────────────────────────────────────────────────────
 * 1 se n = 1; 0 se algum expoente > 1 (tem quadrado); (−1)^k se é livre de quadrados. */
static int nm_mu(long n){
    if(n < 1) return 0;
    if(n == 1) return 1;
    long pr[NT_FAT]; int ex[NT_FAT];
    int k = nt_fatora(n, pr, ex, NT_FAT);
    for(int i = 0; i < k; i++) if(ex[i] > 1) return 0;
    return (k % 2) ? -1 : 1;
}
/* Σ_{d|n} μ(d) — «cancelamento na árvore dos divisores»: 1 em n=1, 0 em n>1 */
static long nm_soma_mu(long n){
    long s = 0;
    for(long d = 1; d <= n; d++) if(n % d == 0) s += nm_mu(d);
    return s;
}
/* ── O TEOREMA CHINÊS DO RESTO ────────────────────────────────────────────────
 * x ≡ a (mod m), x ≡ b (mod n) com gcd(m,n) = 1: solução ÚNICA módulo mn, e ela
 * constrói-se com o inverso — não se procura. */
static int nm_tcr(long a, long m, long b, long n, long *x, long *mod){
    if(m <= 0 || n <= 0) return 0;
    /* o critério, dito à cabeça — e é o SEGUNDO fecho, não o único: o `nm_inv_mod`
     * abaixo também recusa quando m e n não são coprimos, porque aí o inverso não
     * existe. Apagar esta linha não muda o resultado (medido por mutação); ela está cá
     * para NOMEAR a razão, que é o que uma linha de guarda deve fazer. */
    if(iz_gcd(m, n, 0, 0) != 1) return 0;      /* sem coprimalidade não há unicidade */
    long inv;
    if(!nm_inv_mod(m % n, n, &inv)) return 0;
    long t = ((b - a) % n + n) % n;
    t = (t * inv) % n;                          /* x = a + m·t */
    long r = (a + m * t) % (m * n);
    if(r < 0) r += m * n;
    if(x) *x = r;
    if(mod) *mod = m * n;
    return 1;
}
/* ── A FRAÇÃO CONTÍNUA DE UM RACIONAL — que É a descida de Euclides ────────────
 * Os termos são os QUOCIENTES da mesma cadeia de restos que dá o gcd. Devolve quantos,
 * e escreve-os em q. Para a/b com b > 0. */
static size_t nm_fc(long a, long b, long *q, size_t max){
    size_t k = 0;
    while(b != 0 && k < max){
        long t = a / b, r = a - t*b;
        if(r < 0){ t--; r += b; }                /* o quociente pelo CHÃO, sempre */
        q[k++] = t;
        a = b; b = r;
    }
    return k;
}
/* o convergente p_k/q_k pela recorrência — e é a VOLTA da FC */
static void nm_convergente(const long *q, size_t k, long *p, long *d){
    long pn = 1, qn = 0, pa = 0, qa = 1;
    for(size_t i = 0; i <= k; i++){
        long pp = q[i]*pn + pa, dd = q[i]*qn + qa;
        pa = pn; qa = qn; pn = pp; qn = dd;
    }
    *p = pn; *d = qn;
}
/* ── A ORDEM MULTIPLICATIVA — o menor e > 0 com a^e ≡ 1 ────────────────────────
 * É ela que faz de Fermat e de Euler o mesmo teorema visto de dois lados: a ordem
 * DIVIDE φ(n), e por isso a^φ(n) = 1. */
static long nm_ordem(long a, long n){
    if(iz_gcd(a, n, 0, 0) != 1) return 0;
    long r = 1 % n, e = 0;
    for(long k = 1; k <= n; k++){
        r = (r * (a % n)) % n;
        if(r < 0) r += n;
        e = k;
        if(r == 1 % n) return e;
    }
    return 0;
}
/* ── A INFINITUDE, com o primo CONSTRUÍDO — «não apenas reproduza a prova» ─────
 * N = p₁…pₙ + 1 não é divisível por nenhum dos pᵢ (deixa resto 1), logo o seu menor
 * fator primo é NOVO. E devolve-se esse fator, não o N. */
static long nm_primo_novo(const long *p, int n, long *N){
    long prod = 1;
    for(int i = 0; i < n; i++) prod *= p[i];
    long m = prod + 1;
    if(N) *N = m;
    for(long d = 2; d*d <= m; d++) if(m % d == 0) return d;   /* o menor fator primo */
    return m;                                                  /* ele próprio é primo */
}
#endif
