/* algebra.h — A ÁLGEBRA GLOBAL DE R^n: uma só máquina, e o CORPO é o parâmetro.
 *
 * O Aarão: "desenvolve a notação algébrica pros complexos no R^n; a assistente deve operar nessa
 * álgebra global."
 *
 * Até aqui o i estava CRAVADO na célula: um campo `ip` ao lado do `val`, e σ² = −1 escrito à mão
 * no código. Isso é o caso n=2 de uma borda particular, com o particular por dentro da máquina.
 * Aqui o particular sai para fora:
 *
 *   um elemento é     x = x₀ + x₁σ + … + x_{n−1}σ^{n−1},  com xᵢ em Q
 *   um corpo é        a BORDA  σ^n = b₀ + b₁σ + … + b_{n−1}σ^{n−1}
 *
 * e as operações são as da notação algébrica: somar e multiplicar polinómios, e REDUZIR pela
 * borda até o grau descer abaixo de n. Nada mais. O que era ℚ[i] é agora n=2 com borda (−1,0);
 * o ouro é n=2 com (1,1); e n=3,4,… saem sem uma linha nova.
 *
 * É a subsecção da teoria escrita em código: a marcação que a tupla deixa implícita passa a ser
 * o símbolo σ, e o corpo deixa de ser uma escolha do programa para ser um argumento.
 *
 * A FAMÍLIA REAL, QUE É A CIFRA DO REI. O Aarão fixou o nome e a sinonímia, e os dois ficam:
 * a família real é a BASE ORTONORMAL da álgebra em R^n — os n marcadores {1, σ, σ², …, σ^(n−1)},
 * um por eixo — e família real e cifra do rei são o MESMO objeto, não duas coisas que coincidem.
 * A cifra é a sequência de termos que gera; a família é o conjunto de eixos que ela varre. O
 * nome escolhe-se pelo lado de que se olha.
 *
 * E há uma distinção fina, que saiu da medida e não estava prevista (algebra.c §A6): a potência
 * do gerador NUNCA sai do espaço gerado — isso é a construção — mas pode cair SOBRE um eixo
 * (com sinal) ou numa COMBINAÇÃO deles. Cai sobre um eixo exatamente quando a borda é monomial,
 * σ^n = ±σ^j: é o caso do i, o flip puro, e daí as unidades de Z[i] serem ±os eixos. O ouro
 * combina já em σ² = 1 + σ, e o 6º ciclotómico também, apesar de ter ordem 6.
 * CAIR SOBRE UM EIXO É MAIS FORTE DO QUE TER ORDEM FINITA.
 */
#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define AL_N 8                          /* dimensão máxima; a régua não corta o objeto, mas a
                                         * caixa da máquina acaba — e isso diz-se */
typedef struct { long p[2*AL_N], q[2*AL_N]; int n; } Elem;   /* espaço para o grau dobrado */

static long al_mdc(long a, long b){
    if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ long t = a % b; a = b; b = t; }
    return a ? a : 1;
}
static void al_red(long *p, long *q){
    if(*q < 0){ *p = -*p; *q = -*q; }
    long g = al_mdc(*p, *q); *p /= g; *q /= g;
}
/* a += b, em Q */
static void al_qsoma(long ap, long aq, long bp, long bq, long *rp, long *rq){
    long g = al_mdc(aq, bq);
    *rq = aq * (bq / g);
    *rp = ap * (bq / g) + bp * (aq / g);
    al_red(rp, rq);
}
static void al_qmul(long ap, long aq, long bp, long bq, long *rp, long *rq){
    *rp = ap * bp; *rq = aq * bq; al_red(rp, rq);
}

static Elem al_zero(int n){
    Elem e; e.n = n;
    for(int k = 0; k < 2*AL_N; k++){ e.p[k] = 0; e.q[k] = 1; }
    return e;
}
static Elem al_um(int n){ Elem e = al_zero(n); e.p[0] = 1; return e; }
static Elem al_sigma(int n){ Elem e = al_zero(n); if(n > 1) e.p[1] = 1; return e; }

/* A REDUÇÃO PELA BORDA — é ela o corpo, e é a única coisa que distingue um do outro.
 *
 * σ^k, para k >= n, baixa por σ^k = σ^(k−n)·σ^n = σ^(k−n)·(b₀ + b₁σ + …). Faz-se do grau mais
 * alto para baixo, e cada passo desce pelo menos um grau, logo termina. */
static void al_reduz(Elem *x, const Elem *borda){
    int n = x->n;
    for(int k = 2*n - 2; k >= n; k--){
        if(x->p[k] == 0) continue;
        long cp = x->p[k], cq = x->q[k];
        x->p[k] = 0; x->q[k] = 1;
        for(int j = 0; j < n; j++){
            if(borda->p[j] == 0) continue;
            long tp, tq;
            al_qmul(cp, cq, borda->p[j], borda->q[j], &tp, &tq);
            al_qsoma(x->p[k-n+j], x->q[k-n+j], tp, tq, &x->p[k-n+j], &x->q[k-n+j]);
        }
    }
}
static Elem al_soma(Elem a, Elem b, int sinal){
    Elem r = al_zero(a.n);
    for(int k = 0; k < a.n; k++)
        al_qsoma(a.p[k], a.q[k], sinal*b.p[k], b.q[k], &r.p[k], &r.q[k]);
    return r;
}
/* multiplicar É multiplicar polinómios, e depois baixar. A fórmula recursiva da teoria é
 * exatamente isto, escrita por níveis em vez de por graus. */
static Elem al_prod(Elem a, Elem b, const Elem *borda){
    Elem r = al_zero(a.n);
    for(int i = 0; i < a.n; i++){
        if(a.p[i] == 0) continue;
        for(int j = 0; j < a.n; j++){
            if(b.p[j] == 0) continue;
            long tp, tq;
            al_qmul(a.p[i], a.q[i], b.p[j], b.q[j], &tp, &tq);
            al_qsoma(r.p[i+j], r.q[i+j], tp, tq, &r.p[i+j], &r.q[i+j]);
        }
    }
    al_reduz(&r, borda);
    return r;
}
static Elem al_pot(Elem a, long e, const Elem *borda){
    Elem r = al_um(a.n);
    for(long k = 0; k < e; k++) r = al_prod(r, a, borda);
    return r;
}
static int al_igual(Elem a, Elem b){
    for(int k = 0; k < a.n; k++){
        long ap = a.p[k], aq = a.q[k], bp = b.p[k], bq = b.q[k];
        al_red(&ap, &aq); al_red(&bp, &bq);
        if(ap != bp || aq != bq) return 0;
    }
    return 1;
}

/* ESCREVER na notação algébrica: "1 + 2s - 3s^2", com o marcador dito. */
static void al_escreve(Elem x, char *out, size_t lim, const char *marca){
    size_t o = 0; out[0] = 0;
    int algum = 0;
    for(int k = 0; k < x.n; k++){
        long p = x.p[k], q = x.q[k];
        al_red(&p, &q);
        if(p == 0) continue;
        char c[64], t[96];
        if(q == 1) snprintf(c, sizeof c, "%ld", p < 0 ? -p : p);
        else       snprintf(c, sizeof c, "%ld/%ld", p < 0 ? -p : p, q);
        const char *sinal = algum ? (p < 0 ? " - " : " + ") : (p < 0 ? "-" : "");
        if(k == 0)      snprintf(t, sizeof t, "%s%s", sinal, c);
        else if(k == 1) snprintf(t, sizeof t, "%s%s%s", sinal, strcmp(c,"1") ? c : "", marca);
        else            snprintf(t, sizeof t, "%s%s%s^%d", sinal, strcmp(c,"1") ? c : "", marca, k);
        size_t l = strlen(t);
        if(o + l + 1 >= lim) break;
        memcpy(out + o, t, l); o += l; out[o] = 0;
        algum = 1;
    }
    if(!algum) snprintf(out, lim, "0");
}

/* LER a borda: "s^2 = -1"  ou  "s^2 = s + 1"  ou  "s^3 = s + 1".
 * Devolve n, ou 0 se não for uma borda. */
static int al_le_borda(const char *s, Elem *borda, char *marca);
/* LER um elemento na notação algébrica: "1 + 2s", "s", "3 - s^2". */
static int al_le_elem(const char **ps, int n, const char *marca, Elem *out);

static int al_num(const char **ps, long *p, long *q){
    const char *s = *ps; long v = 0; int tem = 0;
    while(*s == ' ') s++;
    while(*s >= '0' && *s <= '9'){ v = v*10 + (*s - '0'); s++; tem = 1; }
    if(!tem) return 0;
    *p = v; *q = 1;
    if(*s == '/' && s[1] >= '0' && s[1] <= '9'){
        s++; long d = 0;
        while(*s >= '0' && *s <= '9'){ d = d*10 + (*s - '0'); s++; }
        if(d) *q = d;
    }
    *ps = s; return 1;
}
static int al_le_elem(const char **ps, int n, const char *marca, Elem *out){
    const char *s = *ps;
    Elem r = al_zero(n);
    int sinal = 1, algum = 0;
    size_t ml = strlen(marca);
    for(;;){
        while(*s == ' ') s++;
        if(*s == '+'){ sinal = 1; s++; continue; }
        if(*s == '-'){ sinal = -1; s++; continue; }
        long cp = 1, cq = 1;
        int temnum = al_num(&s, &cp, &cq);
        while(*s == ' ') s++;
        int grau = 0;
        if(!strncmp(s, marca, ml) && !(s[ml] >= 'a' && s[ml] <= 'z')){
            s += ml; grau = 1;
            if(*s == '^'){ s++; long g = 0; while(*s >= '0' && *s <= '9'){ g = g*10 + (*s-'0'); s++; }
                           grau = (int)g; }
        } else if(!temnum) break;
        if(grau >= n) return -1;                     /* grau acima da dimensão declarada */
        al_qsoma(r.p[grau], r.q[grau], sinal*cp, cq, &r.p[grau], &r.q[grau]);
        algum = 1;
        while(*s == ' ') s++;
        if(*s != '+' && *s != '-') break;
    }
    if(!algum) return 0;
    *ps = s; *out = r; return 1;
}
static int al_le_borda(const char *s, Elem *borda, char *marca){
    while(*s == ' ') s++;
    if(!(*s >= 'a' && *s <= 'z')) return 0;
    marca[0] = *s; marca[1] = 0; s++;
    if(*s != '^') return 0;
    s++;
    long n = 0;
    while(*s >= '0' && *s <= '9'){ n = n*10 + (*s - '0'); s++; }
    if(n < 2 || n > AL_N) return 0;
    while(*s == ' ') s++;
    if(*s != '=') return 0;
    s++;
    Elem b;
    if(al_le_elem(&s, (int)n, marca, &b) != 1) return 0;
    *borda = b;
    return (int)n;
}

/* a régua do corpo, quando n = 2: traço e determinante da borda, e a classe.
 * σ² = b₀ + b₁σ  <=>  σ² − b₁σ − b₀ = 0,  logo traço B = b₁ e det C = −b₀. */
static void al_regua2(Elem borda, long *B, long *C, long *D){
    *B = borda.p[1] / (borda.q[1] ? borda.q[1] : 1);
    *C = -borda.p[0] / (borda.q[0] ? borda.q[0] : 1);
    *D = (*B)*(*B) - 4*(*C);
}

#endif
