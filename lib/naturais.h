/* naturais.h — O CHÃO: e é aqui que o motor DEMONSTRA OS PRÓPRIOS INSTRUMENTOS.
 *
 * O `eval.txt` põe a escada e diz para que serve:
 *
 *     0 → S → + → × → ≤ → | → gcd → primos → fatoração
 *
 * «Naturais podem ser o primeiro módulo em que o motor demonstra os próprios
 * instrumentos que depois usa em todo o resto.» É literal: o `gcd` que já corre na
 * órbita do inversor, a fatoração que já corre nos polinómios, a divisão com resto que
 * já dá o quociente — tudo isso aqui deixa de ser usado e passa a ser PROVADO.
 *
 * E a disciplina é a de sempre, com uma volta a mais: cada operação define-se
 * RECURSIVAMENTE por Peano (n+0=n, n+S(m)=S(n+m); n·0=0, n·S(m)=n·m+n) e mede-se
 * contra a operação primitiva da máquina. DOIS CAMINHOS que têm de concordar — se a
 * definição recursiva e o `+` do processador discordassem, um dos dois estaria errado,
 * e é isso que se quer apanhar.
 *
 * Tudo em inteiros. O 0 entra («deixa a estrutura algébrica mais limpa», e é a escolha
 * dele). */
#ifndef NATURAIS_H
#define NATURAIS_H

/* ── P1..P5: os axiomas, e os que são verificáveis verificam-se ────────────────── */
static long nt_S(long n){ return n + 1; }              /* o sucessor */
static int nt_p3(long n){ return nt_S(n) != 0; }       /* zero não é sucessor */
static int nt_p4(long a, long b){                      /* o sucessor é injetivo */
    return nt_S(a) != nt_S(b) || a == b;
}
/* ── a ADIÇÃO pela definição recursiva: n+0=n, n+S(m)=S(n+m) ───────────────────── */
static long nt_soma(long n, long m){
    long r = n;
    for(long k = 0; k < m; k++) r = nt_S(r);            /* m vezes o sucessor */
    return r;
}
/* ── a MULTIPLICAÇÃO: n·0=0, n·S(m)=n·m+n — a soma iterada ─────────────────────── */
static long nt_mult(long n, long m){
    long r = 0;
    for(long k = 0; k < m; k++) r = nt_soma(r, n);
    return r;
}
/* ── a ORDEM: a ≤ b ⟺ ∃c: a+c = b — e o c EXIBE-SE, que é a testemunha ─────────── */
static int nt_le(long a, long b, long *c){
    for(long k = 0; k <= b; k++) if(nt_soma(a, k) == b){ if(c) *c = k; return 1; }
    return 0;
}
/* ── a DIVISÃO COM RESTO: b = aq + r, 0 ≤ r < a, e existe UM par só ────────────── */
static int nt_divide(long b, long a, long *q, long *r){
    if(a <= 0) return 0;
    long qq = 0, rr = b;
    while(rr >= a){ rr -= a; qq++; }
    *q = qq; *r = rr;
    return 1;
}
/* ── a DIVISIBILIDADE: a | b ⟺ ∃k: b = ak ─────────────────────────────────────── */
static int nt_div(long a, long b){ return a && b % a == 0; }

/* ── O MDC, e o BÉZOUT junto: gcd = ax + by, com x e y EXIBIDOS ────────────────
 * É o Euclides estendido, e é a mesma órbita de restos que a casa já usa. A
 * identidade não se afirma: substitui-se e mede-se. */
static long nt_gcd(long a, long b, long *x, long *y){
    long x0 = 1, y0 = 0, x1 = 0, y1 = 1;
    while(b){
        long q = a / b, t;
        t = a - q*b; a = b; b = t;
        t = x0 - q*x1; x0 = x1; x1 = t;
        t = y0 - q*y1; y0 = y1; y1 = t;
    }
    if(x) *x = x0;
    if(y) *y = y0;
    return a;
}
/* ── OS PRIMOS: p > 1 cujos únicos divisores são 1 e p ─────────────────────────── */
static int nt_primo(long p){
    if(p < 2) return 0;
    for(long d = 2; d*d <= p; d++) if(p % d == 0) return 0;
    return 1;
}
/* ── A FATORAÇÃO em primos, e a VOLTA obrigatória: o produto reconstrói ────────── */
#define NT_FAT 64
static int nt_fatora(long n, long *pr, int *ex, int max){
    int k = 0;
    for(long d = 2; d*d <= n && k < max; d++){
        if(n % d) continue;
        int e = 0;
        while(n % d == 0){ n /= d; e++; }
        pr[k] = d; ex[k] = e; k++;
    }
    if(n > 1 && k < max){ pr[k] = n; ex[k] = 1; k++; }
    return k;
}
static long nt_refaz(const long *pr, const int *ex, int k){    /* a volta */
    long n = 1;
    for(int i = 0; i < k; i++)
        for(int e = 0; e < ex[i]; e++) n *= pr[i];
    return n;
}
/* ── o menor elemento (boa ordenação), com o conjunto dado por bits ───────────── */
static int nt_menor(unsigned conj, int n, long *m){
    for(int k = 0; k < n; k++) if(conj & (1u << k)){ *m = k; return 1; }
    return 0;                                          /* vazio: não tem menor */
}
#endif
