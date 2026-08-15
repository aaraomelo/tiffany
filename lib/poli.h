/* poli.h — O POLINÓMIO, E A EQUAÇÃO DE QUALQUER GRAU. EM INTEIROS E FRAÇÕES.
 *
 * A versão anterior escrevia a doutrina certa e fazia o contrário: dizia «sem iterar, sem
 * tolerância, e em aritmética de inteiros» e guardava os coeficientes em `double`, comparava
 * com `1e-12`, e — o pior — ao ler «1/2» dividia (perdendo a fração) para depois ADIVINHAR
 * o denominador de volta com um laço até 64. O corpo é exato: o coeficiente é uma FRAÇÃO,
 * p/q em inteiros, como a célula da fita já era.
 *
 * O que muda com isso, tudo para melhor:
 *   — ler «x^2 - 1/2» guarda 1 e 2, e não 0,5;
 *   — a cadeia de Sturm corre em INTEIROS por pseudo-divisão (só os SINAIS contam, logo
 *     multiplicar por constante positiva é de graça), e conta-se em ±∞ pelos coeficientes
 *     LÍDERES — sem cota, sem avaliação, sem limiar;
 *   — a raiz racional sai do teorema da raiz racional sobre a forma inteira, já exata.
 *
 * E o que NÃO muda é o método: as raízes irracionais não se calculam — declara-se o corpo
 * ℚ[x]/(p) e lá dentro elas têm nome. Quando se quer a forma normalizada de uma delas, é a
 * FRAÇÃO CONTÍNUA (cifra.h, PQa em inteiros), que é exata e fecha no período.
 */
#ifndef POLI_H
#define POLI_H
#include <string.h>

#define PMAX 12                 /* a caixa: grau até 12. Finito, e dito. */
#define PLIM 3000000000000000L  /* o tecto dos inteiros: acima disto pára-se e diz-se */

typedef struct { long p[PMAX+1], q[PMAX+1]; int n; } Pol;  /* c_k = p[k]/q[k], q[k] > 0 */

static long pl_mdc(long a, long b){
    if(a < 0) a = -a;
    if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
static void pl_reduz(long *p, long *q){
    if(*q < 0){ *p = -*p; *q = -*q; }
    long g = pl_mdc(*p, *q);
    *p /= g; *q /= g;
    if(*p == 0) *q = 1;
}
/* a/b + c/d, exato */
static void pl_soma(long a, long b, long c, long d, long *p, long *q){
    *p = a*d + c*b; *q = b*d; pl_reduz(p, q);
}
static void pol_zera(Pol *p){
    for(int k = 0; k <= PMAX; k++){ p->p[k] = 0; p->q[k] = 1; }
    p->n = 0;
}
static void pol_ajusta(Pol *p){
    p->n = 0;
    for(int k = PMAX; k >= 0; k--) if(p->p[k]){ p->n = k; break; }
}

/* LER um lado: "x^2 + 3x - 4", "2x^3 - x", "5", "x^2", "x - 1/2" */
static int pol_le(const char *s, const char *fim, Pol *p){
    pol_zera(p);
    int algum = 0;
    while(s < fim){
        while(s < fim && *s == ' ') s++;
        if(s >= fim) break;
        long sinal = 1;
        if(*s == '+'){ s++; continue; }
        if(*s == '-'){ sinal = -1; s++; while(s < fim && *s == ' ') s++; }
        long v = 0, den = 1; int tem = 0;
        while(s < fim && *s >= '0' && *s <= '9'){ v = v*10 + (*s-'0'); s++; tem = 1; }
        if(s < fim && *s == '/' && s+1 < fim && s[1] >= '0' && s[1] <= '9'){
            s++; long d = 0;
            while(s < fim && *s >= '0' && *s <= '9'){ d = d*10 + (*s-'0'); s++; }
            if(d) den = d;                       /* a fração FICA fração */
        }
        if(!tem) v = 1;
        while(s < fim && *s == ' ') s++;
        int grau = 0;
        if(s < fim && (*s == 'x' || *s == 'X')){
            s++; grau = 1;
            if(s < fim && *s == '^'){
                s++; long g = 0;
                while(s < fim && *s >= '0' && *s <= '9'){ g = g*10 + (*s-'0'); s++; }
                grau = (int)g;
            }
        } else if(!tem) return 0;
        if(grau > PMAX) return -1;
        pl_soma(p->p[grau], p->q[grau], sinal*v, den, &p->p[grau], &p->q[grau]);
        algum = 1;
    }
    if(!algum) return 0;
    pol_ajusta(p);
    return 1;
}
/* p = esquerda − direita, normalizado a mónico. Tudo em frações: o mónico é dividir
 * cada coeficiente pelo líder, que é multiplicar pela fração inversa. */
static int pol_equacao(const char *s, Pol *p){
    const char *ig = strchr(s, '=');
    if(!ig) return 0;
    Pol a, b;
    int ra = pol_le(s, ig, &a), rb = pol_le(ig+1, s + strlen(s), &b);
    if(ra != 1 || rb != 1) return (ra < 0 || rb < 0) ? -1 : 0;
    pol_zera(p);
    for(int k = 0; k <= PMAX; k++)
        pl_soma(a.p[k], a.q[k], -b.p[k], b.q[k], &p->p[k], &p->q[k]);
    pol_ajusta(p);
    if(p->n == 0) return p->p[0] == 0 ? -2 : -3;   /* -2: todo x; -3: nenhum */
    long lp = p->p[p->n], lq = p->q[p->n];
    for(int k = 0; k <= p->n; k++){                 /* c_k · (lq/lp) */
        long np = p->p[k] * lq, nq = p->q[k] * lp;
        pl_reduz(&np, &nq);
        p->p[k] = np; p->q[k] = nq;
    }
    return 1;
}
/* A FORMA INTEIRA: limpam-se os denominadores (mmc) e tira-se o conteúdo. Os sinais e as
 * raízes não mudam — multiplicar por constante positiva é de graça. */
static int pol_ic(Pol p, long *ic){
    long m = 1;
    for(int k = 0; k <= p.n; k++){
        long g = pl_mdc(m, p.q[k]);
        if(m / g > PLIM / p.q[k]) return 0;         /* não cabe: diz-se, não se arredonda */
        m = (m / g) * p.q[k];
    }
    long cont = 0;
    for(int k = 0; k <= p.n; k++){
        if(p.p[k] && m / p.q[k] > PLIM / (p.p[k] < 0 ? -p.p[k] : p.p[k])) return 0;
        ic[k] = p.p[k] * (m / p.q[k]);
        cont = pl_mdc(cont, ic[k]);
    }
    if(cont > 1) for(int k = 0; k <= p.n; k++) ic[k] /= cont;
    return 1;
}

/* ─── A DOBRA, E NÃO A ITERAÇÃO ────────────────────────────────────────────────────────────
 *
 * O Aarão: "perai, tá tendo ITERAÇÃO aí, precisa ser DOBRA."
 *
 * E tem razão. A primeira versão usava Durand--Kerner: cinco mil passos, convergência
 * aproximada, resíduo 1e-16. Funciona e não é o método desta casa — aqui não se aproxima,
 * desdobra-se. Três coisas, e todas EXATAS:
 *
 *   1. QUANTAS raízes reais: por STURM, a sequência de restos de Euclides — a MESMA divisão
 *      que gera a cifra. Sem iterar, sem tolerância, e em aritmética de inteiros: os restos
 *      são PSEUDO-restos (multiplicados por potência positiva do líder, o que não mexe em
 *      sinal nenhum) e a contagem faz-se em ±∞, onde o sinal é o do coeficiente líder.
 *
 *   2. QUAIS são racionais: por enumeração FINITA (p divide c₀, q divide cₙ). Finito, exato,
 *      e é a dobra do teorema da raiz racional.
 *
 *   3. E as irracionais NÃO SE CALCULAM. Não é falta de método — é o método: a raiz de
 *      x² − 2 não é um número que se ache, é o σ do corpo ℚ[x]/(x²−2). Declara-se o corpo, e
 *      dentro dele ela é exata e tem nome. Aproximá-la seria sair do corpo para dar um decimal
 *      que já não é raiz de nada. A forma normalizada dela é a FRAÇÃO CONTÍNUA (cifra.h).
 */
typedef struct { long a[PMAX+2]; int n; } Pz;      /* polinómio INTEIRO */
static void pz_ajusta(Pz *p){ while(p->n > 0 && p->a[p->n] == 0) p->n--; }
static int pz_nulo(Pz p){ for(int k = 0; k <= p.n; k++) if(p.a[k]) return 0; return 1; }
static void pz_conteudo(Pz *p){
    long g = 0;
    for(int k = 0; k <= p->n; k++) g = pl_mdc(g, p->a[k]);
    if(g > 1) for(int k = 0; k <= p->n; k++) p->a[k] /= g;
}
/* R = −prem(A,B): o pseudo-resto, com fator sempre POSITIVO (potência par quando o líder
 * de B é negativo). Devolve 0 se sair dos inteiros — e aí diz-se, não se inventa. */
static int pz_prem(Pz A, Pz B, Pz *R){
    if(B.n < 0 || B.a[B.n] == 0) return 0;
    long lb = B.a[B.n];
    int d = A.n - B.n;
    if(d < 0){ *R = A; return 1; }
    long fator = 1;
    int e = d + 1;
    if(lb < 0 && (e & 1)) e++;                      /* potência par: o fator fica positivo */
    for(int i = 0; i < e; i++){
        if(fator > PLIM / (lb < 0 ? -lb : lb)) return 0;
        fator *= lb;
    }
    if(fator < 0) return 0;
    *R = A;
    for(int k = 0; k <= R->n; k++){
        if(R->a[k] && (fator > PLIM / (R->a[k] < 0 ? -R->a[k] : R->a[k]))) return 0;
        R->a[k] *= fator;
    }
    while(R->n >= B.n && R->n >= 0){
        if(R->a[R->n] == 0){ R->n--; continue; }
        long f = R->a[R->n] / lb;
        if(R->a[R->n] % lb) return 0;               /* o fator devia ter chegado */
        int dd = R->n - B.n;
        for(int i = 0; i <= B.n; i++){
            long t = f * B.a[i];
            R->a[i+dd] -= t;
        }
        R->n--;
    }
    pz_ajusta(R);
    for(int k = 0; k <= R->n; k++) R->a[k] = -R->a[k];   /* Sturm quer o simétrico */
    pz_conteudo(R);
    return 1;
}
/* quantas raízes reais DISTINTAS. Devolve −1 se a cadeia não coube nos inteiros. */
static int pol_sturm_reais(Pol p){
    long ic[PMAX+1];
    if(!pol_ic(p, ic)) return -1;
    Pz s[PMAX+2]; int m = 0;
    s[0].n = p.n;
    for(int k = 0; k <= p.n; k++) s[0].a[k] = ic[k];
    s[1].n = p.n - 1;
    for(int k = 1; k <= p.n; k++) s[1].a[k-1] = (long)k * ic[k];
    pz_ajusta(&s[1]); pz_conteudo(&s[1]);
    m = 2;
    while(m < PMAX+2 && s[m-1].n > 0){
        Pz r;
        if(!pz_prem(s[m-2], s[m-1], &r)) return -1;
        if(pz_nulo(r)) break;
        s[m++] = r;
    }
    /* a contagem em ±∞: o sinal de cada S_i é o do seu líder (em +∞) e o do líder vezes
     * (−1)^grau (em −∞). Sem cota e sem avaliar: os extremos são estrutura, não números. */
    int v[2] = {0,0};
    for(int lado = 0; lado < 2; lado++){
        int ant = 0, mud = 0;
        for(int k = 0; k < m; k++){
            long l = s[k].a[s[k].n];
            if(l == 0) continue;
            int sg = l > 0 ? 1 : -1;
            if(lado == 0 && (s[k].n & 1)) sg = -sg;   /* −∞ */
            if(ant && sg != ant) mud++;
            ant = sg;
        }
        v[lado] = mud;
    }
    return v[0] - v[1];
}
/* ─── A FATORAÇÃO, E ELA É A VOLTA DA CONVOLUÇÃO ─────────────────────────────────────
 *
 * O produto de polinómios É a convolução — «a forma aditiva da multiplicação vista pela
 * Transformada Universal» (Corpo Universal, a ponte que saiu do mapa) — e a linha de
 * Pascal é o caso (x+1)ⁿ dela: distribuir é convolver, e os binomiais são «a base
 * natural do ⊕» (Newton, medido em progressoes.c §P2).
 *
 * Fatorar é a VOLTA: a deconvolução. E como a volta só existe fora dos divisores de
 * zero, faz-se pelo caminho exato — divide-se e exige-se RESTO ZERO. Nunca se escreve
 * um fator sem o multiplicar de volta.
 *
 * E há uma classe que a casa domina inteira: a FAMÍLIA METÁLICA e os Pisot. Para
 * β(n,m) = xⁿ − m·x^{n−1} − 1 com m ≥ 2, a casa provou por ROUCHÉ NO DUAL que n−1
 * raízes ficam dentro do círculo e uma fora (a de Pisot). Daí a irredutibilidade sai
 * de graça e sem calcular raiz nenhuma: se β = f·g com f, g mónicos inteiros e ambos
 * não constantes, um deles tem TODAS as raízes dentro do disco — e então |c₀(g)| =
 * ∏|raízes| < 1 com c₀ inteiro ≠ 0, o que é impossível. Um argumento inteiro, exato,
 * e é o dual que o torna curto. */
static void pz_mul(Pz a, Pz b, Pz *r){             /* o produto = a CONVOLUÇÃO */
    r->n = a.n + b.n;
    for(int k = 0; k <= r->n && k <= PMAX+1; k++) r->a[k] = 0;
    for(int i = 0; i <= a.n; i++)
        for(int j = 0; j <= b.n && i+j <= PMAX+1; j++)
            r->a[i+j] += a.a[i] * b.a[j];
}
/* a divisão EXATA: devolve 1 e o quociente sse o resto é ZERO (a deconvolução) */
static int pz_div_exata(Pz a, Pz b, Pz *q){
    if(b.n < 0 || b.a[b.n] == 0) return 0;
    Pz r = a;
    q->n = a.n - b.n;
    if(q->n < 0) return 0;
    for(int k = 0; k <= q->n; k++) q->a[k] = 0;
    while(r.n >= b.n){
        if(r.a[r.n] == 0){ r.n--; if(r.n < 0) break; continue; }
        if(r.a[r.n] % b.a[b.n]) return 0;          /* não divide em ℤ: não é fator */
        long f = r.a[r.n] / b.a[b.n];
        int d = r.n - b.n;
        q->a[d] = f;
        for(int i = 0; i <= b.n; i++) r.a[i+d] -= f * b.a[i];
        r.n--;
        if(r.n < 0) break;
    }
    for(int k = 0; k <= r.n && k >= 0; k++) if(r.a[k]) return 0;   /* resto ≠ 0 */
    return 1;
}
/* a linha de Pascal, pela recorrência (sem fatoriais, logo sem estouro) */
static long pl_binom(int n, int k){
    if(k < 0 || k > n) return 0;
    long r = 1;
    for(int i = 1; i <= k; i++) r = r * (n - k + i) / i;
    return r;
}
/* é β(n,m) = xⁿ − m·x^{n−1} − 1 (m ≥ 2)? devolve m, ou 0. E o m=1 fica de fora porque
 * é lá que a prova falha — as raízes sextas de Selmer caem SOBRE a circunferência. */
static long pz_beta_pisot(Pz p){
    if(p.n < 2 || p.a[p.n] != 1 || p.a[0] != -1) return 0;
    for(int k = 1; k < p.n - 1; k++) if(p.a[k]) return 0;
    long m = -p.a[p.n - 1];
    return m >= 2 ? m : 0;
}
/* e a família metálica de grau 2: x² − m x − 1 */
static long pz_metalica(Pz p){
    if(p.n != 2 || p.a[2] != 1 || p.a[0] != -1) return 0;
    long m = -p.a[1];
    return m >= 1 ? m : 0;
}

/* AS RAÍZES RACIONAIS, exatas, por enumeração finita: p | c₀ e q | cₙ.
 * Devolve quantas achou, e escreve os pares em (num, den). */
static int pol_racionais(Pol p, long *num, long *den, int lim){
    long ic[PMAX+1];
    if(!pol_ic(p, ic)) return 0;
    long c0 = ic[0] < 0 ? -ic[0] : ic[0], cn = ic[p.n] < 0 ? -ic[p.n] : ic[p.n];
    if(cn == 0) return 0;
    int achadas = 0;
    if(c0 == 0){ num[achadas] = 0; den[achadas] = 1; achadas++; c0 = 1; }
    for(long a = 1; a <= c0 && achadas < lim; a++){
        if(c0 % a) continue;
        for(long b = 1; b <= cn && achadas < lim; b++){
            if(cn % b) continue;
            for(int sg = 1; sg >= -1 && achadas < lim; sg -= 2){
                /* avalia p(sg·a/b) exatamente, em inteiros: soma ic[k]·(sg·a)^k·b^(n-k) */
                long acc = 0, pa = 1;
                for(int k = 0; k <= p.n; k++){
                    long pb = 1;
                    for(int j = 0; j < p.n - k; j++) pb *= b;
                    acc += ic[k] * pa * pb;
                    pa *= sg*a;
                }
                if(acc == 0){
                    long g = pl_mdc(a, b);
                    long nn = sg*a/g, dd = b/g;
                    int rep = 0;
                    for(int t = 0; t < achadas; t++) if(num[t]==nn && den[t]==dd) rep = 1;
                    if(!rep){ num[achadas] = nn; den[achadas] = dd; achadas++; }
                }
            }
        }
    }
    return achadas;
}

/* ── O CÁLCULO: DERIVAR E INTEGRAR SÃO UMA OPERAÇÃO, COM SINAL ────────────────
 *
 * Integrar é o inverso de derivar, e pelo padrão ouro isso não são duas funções: é
 * UMA com o sinal a decidir — +1 desce o grau (c_k ↦ k·c_k) e −1 sobe-o
 * (c_k ↦ c_k/(k+1)). Em frações é EXATO: 1/3 é 1/3 e não 0,333.
 *
 * E o que a composição mede é a razão de existir do «+C»:
 *   DERIVA(−1) e depois (+1)  →  a IDENTIDADE (integrar e derivar devolve)
 *   DERIVA(+1) e depois (−1)  →  p − p(0): o termo constante É o NÚCLEO da derivada,
 *                                e é por isso que a volta o pede de volta.
 * A constante não é uma convenção de escrita: é o que a operação apaga, medido. */
static void pol_calculo(Pol p, int sentido, Pol *r){
    pol_zera(r);
    if(sentido > 0){                                /* +1: derivar */
        for(int k = 1; k <= p.n; k++){
            long np = p.p[k] * k, nq = p.q[k];
            pl_reduz(&np, &nq);
            r->p[k-1] = np; r->q[k-1] = nq;
        }
    } else {                                        /* −1: integrar (a constante fica 0) */
        for(int k = 0; k <= p.n && k+1 <= PMAX; k++){
            long np = p.p[k], nq = p.q[k] * (k + 1);
            pl_reduz(&np, &nq);
            r->p[k+1] = np; r->q[k+1] = nq;
        }
    }
    pol_ajusta(r);
}
/* o valor num racional x = xp/xq, exato: soma c_k·x^k em frações */
static void pol_val_q(Pol p, long xp, long xq, long *vp, long *vq){
    long ap = 0, aq = 1;                            /* Horner: a = a·x + c_k */
    for(int k = p.n; k >= 0; k--){
        long np = ap * xp, nq = aq * xq;            /* a·x */
        pl_reduz(&np, &nq);
        pl_soma(np, nq, p.p[k], p.q[k], &ap, &aq);
    }
    *vp = ap; *vq = aq;
}

/* ── A MULTIPLICAÇÃO ⊗ E A DIVISÃO SÃO UMA OPERAÇÃO, COM SINAL ─────────────────
 * O Corpo Universal já o diz: a divisão é a FIBRA INVERSA da fusão — «dado z = u⊗v
 * e um dos fatores, a fibra resolve-se». Logo não são duas: é a CONVOLUÇÃO com um
 * sinal. +1 convolve (fusão), −1 deconvolve (a fibra, e só existe onde é exata).
 * Devolve 1 se a operação fecha; 0 quando a fibra não existe (o divisor de zero). */
static void pz_mul(Pz a, Pz b, Pz *r);
static int pz_div_exata(Pz a, Pz b, Pz *q);
static int CONV(Pz a, Pz b, int sentido, Pz *r){
    if(sentido > 0){ pz_mul(a, b, r); return 1; }
    return pz_div_exata(a, b, r);
}

/* ─── AS CINCO OPERAÇÕES, NO POLINÓMIO ───────────────────────────────────────────
 *
 * O Corpo Universal fixa as cinco com a conservação de cada uma, e aqui elas realizam-se
 * no corpo dos polinómios — sem régua nova, e cada uma com a sua medida:
 *
 *   Soma ⊕          a dobra: E(u⊕u) = 2E(u), e a retração devolve
 *   Multiplicação ⊗ a fusão = CONVOLUÇÃO: E(u⊗v) = E(u)·E(v) (a norma multiplica)
 *   Divisão         a FIBRA: a = q·b + r com grau(r) < grau(b) — e a volta reconstrói
 *   Dual †          a INVOLUÇÃO: o recíproco ν(p) = xⁿ·p(1/x), com ν∘ν = id, que troca
 *                   dentro por fora — é ele o ν da prova de Pisot por Rouché
 *   Inversão        a volta: só é admissível quem a tem (b ≠ 0, e o resto fecha)
 *
 * A energia é a mesma do quadro: E(p) = Σ c_k² (o segundo momento). */
static long pz_energia(Pz p){
    long e = 0;
    for(int k = 0; k <= p.n; k++) e += p.a[k]*p.a[k];
    return e;
}
static void pz_soma(Pz a, Pz b, Pz *r){
    r->n = a.n > b.n ? a.n : b.n;
    for(int k = 0; k <= r->n; k++)
        r->a[k] = (k <= a.n ? a.a[k] : 0) + (k <= b.n ? b.a[k] : 0);
    pz_ajusta(r);
}
/* O DUAL: o recíproco. Troca dentro por fora (as raízes vão para 1/raiz), e é
 * INVOLUÇÃO — ν∘ν = id quando o termo constante não é nulo. */
static void pz_dual(Pz p, Pz *r){
    r->n = p.n;
    for(int k = 0; k <= p.n; k++) r->a[k] = p.a[p.n - k];
    pz_ajusta(r);
}
/* A DIVISÃO COM RESTO — a fibra. Em ℤ o quociente só fecha se o líder de b divide, e
 * por isso devolve-se também o FATOR de pseudo-divisão: vale lc(b)^k·a = q·b + r, e o
 * k é dito, não escondido. Com b mónico o fator é 1 e a identidade é a limpa. */
static int pz_div_resto(Pz a, Pz b, Pz *q, Pz *r, long *fator){
    if(b.n < 0 || b.a[b.n] == 0) return 0;
    long lb = b.a[b.n];
    *fator = 1;
    int d = a.n - b.n;
    if(d < 0){ q->n = 0; q->a[0] = 0; *r = a; return 1; }
    for(int i = 0; i <= d; i++){                    /* o fator: lc(b)^(d+1) */
        if(*fator > PLIM / (lb < 0 ? -lb : lb)) return 0;
        *fator *= lb;
    }
    Pz w = a;
    for(int k = 0; k <= w.n; k++){
        if(w.a[k] && (*fator > PLIM / (w.a[k] < 0 ? -w.a[k] : w.a[k]))) return 0;
        w.a[k] *= *fator;
    }
    q->n = d;
    for(int k = 0; k <= d; k++) q->a[k] = 0;
    while(w.n >= b.n){
        if(w.a[w.n] == 0){ w.n--; if(w.n < 0) break; continue; }
        if(w.a[w.n] % lb) return 0;
        long f = w.a[w.n] / lb;
        int dd = w.n - b.n;
        q->a[dd] = f;
        for(int i = 0; i <= b.n; i++) w.a[i+dd] -= f * b.a[i];
        w.n--;
        if(w.n < 0) break;
    }
    if(w.n < 0){ w.n = 0; w.a[0] = 0; }
    pz_ajusta(&w);
    *r = w;
    return 1;
}

/* O MDC — E ELE É A ÓRBITA DO INVERSOR.
 *
 * «Euclides é a dinâmica do inversor: o passo (p,q) ↦ (q, p − aq) é X∘T^{−a}, a órbita
 * do racional desce até à folha, e A FOLHA É O GCD» (Corpo Universal, thm:dinamica-
 * inversor). No polinómio é a mesma órbita, com o pseudo-resto no lugar do resto — e o
 * que sobra quando a descida pára é o máximo divisor comum.
 *
 * É a MESMA cadeia que gera a fração contínua e a cadeia de Sturm: uma lei, três
 * balcões. Aqui só se pede o fim da descida.
 *
 * Devolve 0 se a cadeia não coube nos inteiros — e aí diz-se, não se arredonda. */
static int pz_mdc(Pz a, Pz b, Pz *g, int *passos){
    *passos = 0;
    pz_ajusta(&a); pz_ajusta(&b);
    if(pz_nulo(b)){ *g = a; pz_conteudo(g); return 1; }
    if(pz_nulo(a)){ *g = b; pz_conteudo(g); return 1; }
    pz_conteudo(&a); pz_conteudo(&b);
    for(int volta = 0; volta < 64; volta++){
        if(a.n < b.n){ Pz t = a; a = b; b = t; }    /* o maior à frente */
        if(pz_nulo(b)) break;
        Pz r;
        if(!pz_prem(a, b, &r)) return 0;             /* o pseudo-resto (com o sinal do Sturm) */
        (*passos)++;
        a = b; b = r;
        pz_ajusta(&b);
        if(pz_nulo(b)) break;
    }
    *g = a;
    pz_conteudo(g);
    if(g->a[g->n] < 0) for(int k = 0; k <= g->n; k++) g->a[k] = -g->a[k];
    return 1;
}

/* O FATORADOR. Devolve quantos fatores achou e escreve-os em `fs` (o conteúdo inteiro
 * vai em `*cont`). Cada fator é verificado pela DIVISÃO EXATA, e no fim o produto de
 * todos tem de reconstruir o polinómio byte a byte — a volta, ou não houve fatoração.
 *
 * A ordem é a da casa: primeiro o conteúdo (o escalar), depois as raízes racionais (os
 * fatores lineares, por enumeração finita), e o que sobra tenta partir-se em dois por
 * busca limitada — com a divisão exata a decidir, nunca uma aproximação. Onde nada
 * divide, diz-se: irredutível quando há prova (grau ≤ 3 sem raiz racional, ou a família
 * de Pisot), e «não achei fator» quando não há — que não é o mesmo. */
#define PFMAX 16
static int pz_fatora(Pz p, Pz *fs, int max, long *cont){
    *cont = 0;
    for(int k = 0; k <= p.n; k++) *cont = pl_mdc(*cont, p.a[k]);
    if(*cont < 1) *cont = 1;
    if(p.a[p.n] < 0) *cont = -*cont;               /* o líder positivo, por convenção */
    for(int k = 0; k <= p.n; k++) p.a[k] /= *cont;
    int nf = 0;
    /* 1. os fatores LINEARES, das raízes racionais: (q·x − p) por cada p/q */
    for(int outra = 1; outra && p.n > 0 && nf < max; ){
        outra = 0;
        long c0 = p.a[0] < 0 ? -p.a[0] : p.a[0], cn = p.a[p.n] < 0 ? -p.a[p.n] : p.a[p.n];
        if(c0 == 0){                                /* x é fator */
            Pz d; d.n = 1; d.a[0] = 0; d.a[1] = 1;
            Pz q;
            if(pz_div_exata(p, d, &q)){ fs[nf++] = d; p = q; outra = 1; continue; }
        }
        for(long a = 1; a <= c0 && !outra; a++){
            if(c0 % a) continue;
            for(long b = 1; b <= cn && !outra; b++){
                if(cn % b) continue;
                for(int sg = 1; sg >= -1 && !outra; sg -= 2){
                    Pz d; d.n = 1; d.a[0] = -sg*a; d.a[1] = b;   /* b·x − sg·a */
                    Pz q;
                    if(pz_div_exata(p, d, &q)){
                        fs[nf++] = d; p = q; outra = 1;
                    }
                }
            }
        }
    }
    /* 2. o que sobra: tenta partir-se em dois fatores inteiros, por busca limitada nos
     *    coeficientes (o líder e o termo constante DIVIDEM os do polinómio — é a mesma
     *    dobra do teorema da raiz racional, um grau acima). */
    while(p.n >= 4 && nf < max){
        int achou = 0;
        long cn = p.a[p.n] < 0 ? -p.a[p.n] : p.a[p.n];
        long c0 = p.a[0] < 0 ? -p.a[0] : p.a[0];
        for(long la = 1; la <= cn && !achou; la++){
            if(cn % la) continue;
            for(long c = -c0; c <= c0 && !achou; c++){
                if(c == 0 || c0 % (c < 0 ? -c : c)) continue;
                for(long b = -20; b <= 20 && !achou; b++){
                    Pz d; d.n = 2; d.a[2] = la; d.a[1] = b; d.a[0] = c;
                    Pz q;
                    if(pz_div_exata(p, d, &q)){ fs[nf++] = d; p = q; achou = 1; }
                }
            }
        }
        if(!achou) break;
    }
    if(p.n > 0 && nf < max) fs[nf++] = p;          /* o que restou é fator também */
    return nf;
}
/* A VOLTA: o produto dos fatores (vezes o conteúdo) reconstrói o original? */
static int pz_confere(Pz orig, Pz *fs, int nf, long cont){
    Pz acc; acc.n = 0; acc.a[0] = cont;
    for(int k = 0; k < nf; k++){ Pz r; pz_mul(acc, fs[k], &r); acc = r; }
    if(acc.n != orig.n) return 0;
    for(int k = 0; k <= orig.n; k++) if(acc.a[k] != orig.a[k]) return 0;
    return 1;
}

#endif
