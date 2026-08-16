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
 * Tudo em ARITMÉTICA NATURAL — `unsigned long`, sem sinal, porque é isso que ℕ é. O 0
 * entra («deixa a estrutura algébrica mais limpa», e é a escolha dele). */
#ifndef NATURAIS_H
#define NATURAIS_H

/* ── OS NATURAIS NÃO TÊM SINAL, e o tipo tem de o dizer ────────────────────────
 * Este ficheiro estava todo em `long` — com sinal — para representar ℕ. Um natural não
 * tem sinal, e o tipo que o carrega não deve ter: `unsigned long` é a aritmética natural,
 * e nela a maioria dos guardas que aqui estavam deixam de ser precisos, porque o caso que
 * eles protegiam não existe.
 *
 * O que MUDA de facto, e é a razão para o fazer:
 *   · «zero não é sucessor» (P3) passa a ser uma afirmação sobre o TIPO — em ℕ o sucessor
 *     nunca é zero, e o único modo de o ser é o wrap, que se CONTA à parte em vez de se
 *     esconder num `if`;
 *   · `a <= 0` vira `a == 0`, porque não há negativos para proteger;
 *   · e a subtracção não aparece em lado nenhum, que é o que ℕ diz.
 *
 * A ÚNICA excepção é Bézout: o gcd é natural, mas os COEFICIENTES x, y têm sinal — a
 * identidade gcd = ax + by precisa deles em ℤ. Isso diz-se, e é o andar seguinte da
 * escada, não uma falha deste. */
static long nt_wrap = 0;        /* os sucessores que saíram do tipo — contados, não escondidos */

/* ── P1..P5: os axiomas, e os que são verificáveis verificam-se ────────────────── */
static unsigned long nt_S(unsigned long n){                /* o sucessor */
    nt_wrap += (n == (unsigned long)-1);                    /* saiu do tipo: conta-se */
    return n + 1;
}
static int nt_p3(unsigned long n){ return nt_S(n) != 0; }  /* zero não é sucessor */
static int nt_p4(unsigned long a, unsigned long b){        /* o sucessor é injetivo */
    return nt_S(a) != nt_S(b) || a == b;
}
/* ── a ADIÇÃO pela definição recursiva: n+0=n, n+S(m)=S(n+m) ───────────────────── */
static unsigned long nt_soma(unsigned long n, unsigned long m){
    unsigned long r = n;
    for(unsigned long k = 0; k < m; k++) r = nt_S(r);       /* m vezes o sucessor */
    return r;
}
/* ── a MULTIPLICAÇÃO: n·0=0, n·S(m)=n·m+n — a soma iterada ─────────────────────── */
static unsigned long nt_mult(unsigned long n, unsigned long m){
    unsigned long r = 0;
    for(unsigned long k = 0; k < m; k++) r = nt_soma(r, n);
    return r;
}
/* ── a ORDEM: a ≤ b ⟺ ∃c: a+c = b — e o c EXIBE-SE, que é a testemunha ───────────
 * Em ℕ a testemunha é b − a, e ela SÓ existe quando a ≤ b: é a subtracção parcial, que
 * é exactamente a fibra que este andar não paga. O laço que aqui estava percorria
 * b+1 valores para achar o que a diferença dá de uma vez — e a definição recursiva
 * continua a ser medida, no medidor, contra esta. */
static int nt_le(unsigned long a, unsigned long b, unsigned long *c){
    if(a > b) return 0;                        /* a fibra não existe: é o preço de ℕ */
    if(c) *c = b - a;                          /* e a testemunha é a diferença */
    return 1;
}
/* ── a DIVISÃO COM RESTO: b = aq + r, 0 ≤ r < a, e existe UM par só ────────────── */
static int nt_divide(unsigned long b, unsigned long a,
                     unsigned long *q, unsigned long *r){
    if(a == 0) return 0;                       /* era `a <= 0`: sem sinal, é só o zero */
    *q = b / a; *r = b % a;
    return 1;
}
/* ── a DIVISIBILIDADE: a | b ⟺ ∃k: b = ak ─────────────────────────────────────── */
static int nt_div(unsigned long a, unsigned long b){ return a && b % a == 0; }

/* ── O MDC, e o BÉZOUT junto: gcd = ax + by, com x e y EXIBIDOS ────────────────
 * É o Euclides estendido, e é a mesma órbita de restos que a casa já usa. A
 * identidade não se afirma: substitui-se e mede-se.
 *
 * E AQUI O SINAL É NECESSÁRIO, e é o único sítio: o gcd é natural, mas os coeficientes
 * de Bézout vivem em ℤ — a identidade gcd = ax + by pede-os. É o andar seguinte da
 * escada a ser usado por dentro deste, e diz-se em vez de se disfarçar. */
static unsigned long nt_gcd(unsigned long a, unsigned long b, long *x, long *y){
    long x0 = 1, y0 = 0, x1 = 0, y1 = 1;
    while(b){
        unsigned long q = a / b, t;
        t = a - q*b; a = b; b = t;
        long u = x0 - (long)q*x1; x0 = x1; x1 = u;
        u = y0 - (long)q*y1; y0 = y1; y1 = u;
    }
    if(x) *x = x0;
    if(y) *y = y0;
    return a;
}
/* ── OS PRIMOS: p > 1 cujos únicos divisores são 1 e p ─────────────────────────── */
static int nt_primo(unsigned long p){
    if(p < 2) return 0;                        /* em ℕ isto é {0, 1}, e mais nada */
    for(unsigned long d = 2; d*d <= p; d++) if(p % d == 0) return 0;
    return 1;
}
/* ── A FATORAÇÃO em primos, e a VOLTA obrigatória: o produto reconstrói ────────── */
#define NT_FAT 64
static int nt_fatora(unsigned long n, unsigned long *pr, int *ex, int max){
    int k = 0;
    for(unsigned long d = 2; d*d <= n && k < max; d++){
        if(n % d) continue;
        int e = 0;
        while(n % d == 0){ n /= d; e++; }
        pr[k] = d; ex[k] = e; k++;
    }
    if(n > 1 && k < max){ pr[k] = n; ex[k] = 1; k++; }
    return k;
}
static unsigned long nt_refaz(const unsigned long *pr, const int *ex, int k){  /* a volta */
    unsigned long n = 1;
    for(int i = 0; i < k; i++)
        for(int e = 0; e < ex[i]; e++) n *= pr[i];
    return n;
}
/* ── o menor elemento (boa ordenação), com o conjunto dado por bits ───────────── */
static int nt_menor(unsigned conj, int n, unsigned long *m){
    for(int k = 0; k < n; k++) if(conj & (1u << k)){ *m = (unsigned long)k; return 1; }
    return 0;                                          /* vazio: não tem menor */
}
#endif
