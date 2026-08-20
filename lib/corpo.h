/* corpo.h — ONDE «TODA OPERAÇÃO QUE TEM FIBRA TEM VOLTA» VIRA ESTRUTURA FORMAL.
 *
 *  ordem do coordenador fecha o andar com a frase que fecha a escada toda:
 *
 *     «corpo é praticamente o ponto em que "toda operação que tem fibra tem volta" vira
 *      uma estrutura algébrica formal. A EXCEÇÃO continua sendo exatamente a que vocês
 *      já descobriram: 0⁻¹ não existe.»
 *
 * E a escada com a reversibilidade nova em cada salto:
 *
 *     ℕ: + ×      ℤ: a ↦ −a      ℚ: a ↦ a⁻¹ (a≠0)      ℝ: completude      K: fechada
 *
 * Por isso este ficheiro é pequeno: quase tudo já corre. O `nm_inv_mod` já é o inverso
 * por Euclides, o `an_corpo`/`an_dominio` já decidem, o `nm_ordem` já dá o elemento
 * primitivo. O que falta é a EXTENSÃO — 𝔽ₚ[x]/(f) e ℚ(√2) — e é ela que transforma
 *
 *     corpo → polinómio → fatoração → irredutibilidade → quociente
 *
 * na mesma máquina de antes.
 *
 * ── A CODIFICAÇÃO ─────────────────────────────────────────────────────────────
 * Um elemento de 𝔽ₚ[x]/(f) com deg f = n é um polinómio de grau < n sobre 𝔽ₚ. Codifica-se
 * pelo ÍNDICE na base p: o elemento k tem por coeficientes os dígitos de k. Assim os
 * elementos são 0..pⁿ−1 e a tábua entra direta no `Anel` que já existe — o corpo novo é
 * medido pelo MESMO código que mediu o ℤ₅, e não por uma segunda régua.
 *
 * Precisa de `racionais.h`, `inteiros.h`, `naturais.h`, `numeros.h` e `estrutura.h`. */
#ifndef CORPO_H
#define CORPO_H

#define FX_MAX 10

/* ── POLINÓMIOS SOBRE 𝔽ₚ ───────────────────────────────────────────────────────── */
static int fx_m(int v, int p){ int r = v % p; return r < 0 ? r + p : r; }

static void fx_de_indice(long k, int p, int n, int *c){
    for(int i = 0; i < n; i++){ c[i] = (int)(k % p); k /= p; }
}
static long fx_para_indice(const int *c, int p, int n){
    long k = 0;
    for(int i = n - 1; i >= 0; i--) k = k * p + fx_m(c[i], p);
    return k;
}
/* o RESTO da divisão por f MÓNICO de grau n, sobre 𝔽ₚ — a mesma descida de sempre */
static void fx_resto(int *a, int ga, const int *f, int n, int p){
    for(int d = ga; d >= n; d--){
        int co = fx_m(a[d], p);
        if(!co) continue;
        for(int i = 0; i <= n; i++)
            a[d - n + i] = fx_m(a[d - n + i] - co * f[i], p);
    }
}
/* IRREDUTÍVEL: nenhum mónico de grau 1..n/2 o divide. Para os graus deste andar (≤ 4)
 * a divisão por tentativa é exaustiva, e é ela a prova — não uma heurística. */
static int fx_irredutivel(const int *f, int n, int p){
    if(n < 2) return 0;
    for(int g = 1; 2*g <= n; g++){
        long total = 1;
        for(int i = 0; i < g; i++) total *= p;          /* os mónicos de grau g */
        for(long k = 0; k < total; k++){
            int d[FX_MAX] = {0};
            fx_de_indice(k, p, g, d);
            d[g] = 1;                                    /* mónico */
            /* divide f por d e vê se o resto é zero */
            int a[2*FX_MAX] = {0};
            for(int i = 0; i <= n; i++) a[i] = fx_m(f[i], p);
            for(int e = n; e >= g; e--){
                int co = fx_m(a[e], p);
                if(!co) continue;
                for(int i = 0; i <= g; i++)
                    a[e - g + i] = fx_m(a[e - g + i] - co * d[i], p);
            }
            int zero = 1;
            for(int i = 0; i < g; i++) if(a[i]) zero = 0;
            if(zero) return 0;                           /* d divide f: redutível */
        }
    }
    return 1;
}
/* A EXTENSÃO 𝔽ₚ[x]/(f) COMO ANEL — e é o `an_corpo` de sempre que decide se é corpo */
static int corpo_ext(int p, const int *f, int n, Anel *R){
    long q = 1;
    for(int i = 0; i < n; i++) q *= p;
    if(q > ES_MAX) return 0;
    R->n = (int)q;
    for(long a = 0; a < q; a++) for(long b = 0; b < q; b++){
        int ca[FX_MAX] = {0}, cb[FX_MAX] = {0}, s[FX_MAX] = {0}, m[2*FX_MAX] = {0};
        fx_de_indice(a, p, n, ca);
        fx_de_indice(b, p, n, cb);
        for(int i = 0; i < n; i++) s[i] = fx_m(ca[i] + cb[i], p);
        R->soma[a][b] = (int)fx_para_indice(s, p, n);
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
            m[i+j] = fx_m(m[i+j] + ca[i]*cb[j], p);
        fx_resto(m, 2*n - 2, f, n, p);
        R->mult[a][b] = (int)fx_para_indice(m, p, n);
    }
    return 1;
}
/* a CARACTERÍSTICA: o menor n > 0 com n·1 = 0. Devolve 0 se não houver até ao teto —
 * e o teto DIZ-SE, porque «não achei» não é «não existe». */
static int corpo_carac(const Anel *R, int teto){
    int um = 1 % R->n, s = 0;
    for(int n = 1; n <= teto; n++){
        s = R->soma[s][um];
        if(s == 0) return n;
    }
    return 0;
}
/* o ELEMENTO PRIMITIVO: gera K× inteiro. Devolve −1 se não houver (e não deve acontecer
 * num corpo finito — é o teorema 21 dele). */
static int corpo_primitivo(const Anel *R){
    int q = R->n;
    for(int g = 1; g < q; g++){
        int visto[ES_MAX] = {0}, x = 1 % q, k = 0;
        for(k = 1; k < q; k++){
            x = R->mult[x][g];
            if(x == 0) break;
            if(visto[x]) break;
            visto[x] = 1;
        }
        int quantos = 0;
        for(int i = 0; i < q; i++) if(visto[i]) quantos++;
        if(quantos == q - 1) return g;
    }
    return -1;
}
/* o inverso num anel dado por tábua — e a AUSÊNCIA devolve −1, que é o gume do andar */
/* E AQUI O GUARDA É REDUNDANTE — DE PROPÓSITO, E É A TESE DO ANDAR.
 * Medido por mutação: apagar a linha do zero não muda um único resultado, porque a
 * BUSCA já recusa sozinha — 0·b = 0 ≠ 1 para todo b, logo o ciclo esgota-se e devolve
 * −1. O zero não fica sem inverso por decreto nosso: fica porque a fibra é LITERALMENTE
 * vazia, e a máquina descobre-o a procurar. A linha está cá para NOMEAR a razão. */
static int corpo_inv(const Anel *R, int a){
    int um = 1 % R->n;
    if(a == 0) return -1;                     /* 0⁻¹ NÃO EXISTE: a fibra é vazia */
    for(int b = 0; b < R->n; b++) if(R->mult[a][b] == um) return b;
    return -1;
}

/* ── ℚ(√2): A EXTENSÃO POR RADICAL, EXATA ──────────────────────────────────────
 * Todo elemento é a + b√2 com a, b ∈ ℚ, e o inverso sai da NORMA:
 *     (a + b√2)(a − b√2) = a² − 2b²
 * logo (a + b√2)⁻¹ = (a − b√2)/(a² − 2b²), e o denominador só é 0 se a = b = 0 —
 * porque √2 ∉ ℚ, que é o teorema do andar dos reais a servir este. */
typedef struct { Qz a, b; } Qs;               /* a + b√d */

static Qs qs(Qz a, Qz b){ Qs r; r.a = a; r.b = b; return r; }
static Qs qs_soma(Qs x, Qs y){ return qs(qz_soma(x.a,y.a), qz_soma(x.b,y.b)); }
static Qs qs_mult(Qs x, Qs y, long d){        /* (a+b√d)(c+e√d) = (ac+d·be) + (ae+bc)√d */
    Qz ac = qz_mult(x.a, y.a), be = qz_mult(x.b, y.b);
    Qz ae = qz_mult(x.a, y.b), bc = qz_mult(x.b, y.a);
    return qs(qz_soma(ac, qz_mult(qz_de_inteiro(d), be)), qz_soma(ae, bc));
}
static Qs qs_conj(Qs x){ return qs(x.a, qz_oposto(x.b)); }   /* o dual: b ↦ −b */
static Qz qs_norma(Qs x, long d){             /* N(x) = a² − d·b², e é ela a fibra */
    return qz_soma(qz_mult(x.a,x.a), qz_oposto(qz_mult(qz_de_inteiro(d), qz_mult(x.b,x.b))));
}
static int qs_inverso(Qs x, long d, Qs *r){
    Qz N = qs_norma(x, d);
    if(N.p == 0) return 0;                    /* só o zero, porque √d ∉ ℚ */
    Qz ia, ib;
    if(!qz_divide(x.a, N, &ia)) return 0;
    if(!qz_divide(qz_oposto(x.b), N, &ib)) return 0;
    *r = qs(ia, ib);
    return 1;
}
static int qs_igual(Qs x, Qs y){ return qz_igual(x.a,y.a) && qz_igual(x.b,y.b); }
#endif
