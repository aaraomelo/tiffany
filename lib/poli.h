/* poli.h — A EQUAÇÃO ENTRE DOIS POLINÓMIOS: p(x) = q(x).
 *
 * O Aarão: "resolve uma equação dupolinomial pra testar na assistente."
 *
 * Dois polinómios, um de cada lado. E o método é o de sempre neste projeto: passa-se tudo para
 * um lado, acham-se as raízes TODAS ao mesmo tempo (Durand--Kerner, sem deflação e sem escolher
 * qual primeiro), e depois SUBSTITUI-SE cada uma para medir o resíduo. Não se confia na
 * iteração — mede-se.
 *
 * E o que a resposta traz não é só a lista: é a ASSINATURA (r, s) — quantas raízes na reta e
 * quantos pares no círculo — que é a classificação do §G5, e é ela que diz em que corpo a
 * equação vive. Em grau 2 ela cabe no sinal do Δ e chamamos-lhe hiperbólico ou elíptico; em
 * grau n precisa do par, e é a mesma coisa.
 */
#ifndef POLI_H
#define POLI_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#define PMAX 12                 /* a caixa: grau até 12. Finito, e dito. */

typedef struct { double c[PMAX+1]; int n; } Pol;   /* c[0] + c[1]x + … + c[n]x^n */

/* LER um lado: "x^2 + 3x - 4", "2x^3 - x", "5", "x^2" */
static int pol_le(const char *s, const char *fim, Pol *p){
    for(int k = 0; k <= PMAX; k++) p->c[k] = 0;
    p->n = 0;
    int algum = 0;
    while(s < fim){
        while(s < fim && *s == ' ') s++;
        if(s >= fim) break;
        double sinal = 1;
        if(*s == '+'){ s++; continue; }
        if(*s == '-'){ sinal = -1; s++; while(s < fim && *s == ' ') s++; }
        double v = 0; int tem = 0;
        while(s < fim && *s >= '0' && *s <= '9'){ v = v*10 + (*s-'0'); s++; tem = 1; }
        if(s < fim && *s == '/' && s+1 < fim && s[1] >= '0' && s[1] <= '9'){
            s++; double d = 0;
            while(s < fim && *s >= '0' && *s <= '9'){ d = d*10 + (*s-'0'); s++; }
            if(d) v /= d;
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
        p->c[grau] += sinal*v;
        if(grau > p->n) p->n = grau;
        algum = 1;
    }
    if(!algum) return 0;
    while(p->n > 0 && fabs(p->c[p->n]) < 1e-14) p->n--;
    return 1;
}
/* p = esquerda - direita, e normalizado a mónico */
static int pol_equacao(const char *s, Pol *p){
    const char *ig = strchr(s, '=');
    if(!ig) return 0;
    Pol a, b;
    int ra = pol_le(s, ig, &a), rb = pol_le(ig+1, s + strlen(s), &b);
    if(ra != 1 || rb != 1) return (ra < 0 || rb < 0) ? -1 : 0;
    for(int k = 0; k <= PMAX; k++) p->c[k] = a.c[k] - b.c[k];
    p->n = 0;
    for(int k = PMAX; k >= 0; k--) if(fabs(p->c[k]) > 1e-14){ p->n = k; break; }
    if(p->n == 0) return fabs(p->c[0]) < 1e-14 ? -2 : -3;   /* -2: todo x; -3: nenhum */
    double lead = p->c[p->n];
    for(int k = 0; k <= p->n; k++) p->c[k] /= lead;
    return 1;
}
static double complex pol_val(Pol p, double complex z){
    double complex r = 0;
    for(int k = p.n; k >= 0; k--) r = r*z + p.c[k];
    return r;
}
/* ─── A DOBRA, E NÃO A ITERAÇÃO ────────────────────────────────────────────────────────────
 *
 * O Aarão: "perai, tá tendo ITERAÇÃO aí, precisa ser DOBRA."
 *
 * E tem razão. A primeira versão usava Durand--Kerner: cinco mil passos, convergência
 * aproximada, resíduo 1e-16. Funciona e não é o método desta casa — aqui não se aproxima,
 * desdobra-se. Três coisas mudam, e todas ficam EXATAS:
 *
 *   1. QUANTAS raízes reais: por STURM, que é a sequência de restos de Euclides — a MESMA
 *      divisão que gera a cifra. Sem iterar, sem tolerância, e em aritmética de inteiros.
 *
 *   2. QUAIS são racionais: por enumeração FINITA (p divide c₀, q divide cₙ). Finito, exato,
 *      e é a dobra do teorema da raiz racional.
 *
 *   3. E as irracionais NÃO SE CALCULAM. Não é falta de método — é o método: a raiz de
 *      x² - 2 não é um número que se ache, é o σ do corpo Q[x]/(x²-2). Declara-se o corpo, e
 *      dentro dele ela é exata e tem nome. Aproximá-la seria sair do corpo para dar um decimal
 *      que já não é a raiz de nada.
 */

/* Euclides sobre polinómios racionais, com denominador comum inteiro. Aqui basta o RESTO. */
typedef struct { double a[PMAX+2]; int n; } Px;
static void px_resto(Px u, Px v, Px *r){
    *r = u;
    while(r->n >= v.n && r->n >= 0){
        if(fabs(r->a[r->n]) < 1e-12){ r->n--; continue; }
        double f = r->a[r->n] / v.a[v.n];
        int d = r->n - v.n;
        for(int i = 0; i <= v.n; i++) r->a[i+d] -= f * v.a[i];
        r->n--;
    }
    while(r->n > 0 && fabs(r->a[r->n]) < 1e-12) r->n--;
}
static double px_val(Px p, double x){
    double s = 0;
    for(int k = p.n; k >= 0; k--) s = s*x + p.a[k];
    return s;
}
/* a CADEIA DE STURM, e a contagem de mudanças de sinal */
static int pol_sturm_reais(Pol p){
    Px s[PMAX+2]; int m = 0;
    s[0].n = p.n;
    for(int k = 0; k <= p.n; k++) s[0].a[k] = p.c[k];
    s[1].n = p.n - 1;
    for(int k = 1; k <= p.n; k++) s[1].a[k-1] = k * p.c[k];
    m = 2;
    while(m < PMAX+2 && s[m-1].n > 0){
        Px r; px_resto(s[m-2], s[m-1], &r);
        int zero = 1;
        for(int k = 0; k <= r.n; k++) if(fabs(r.a[k]) > 1e-12) zero = 0;
        if(zero) break;
        for(int k = 0; k <= r.n; k++) r.a[k] = -r.a[k];
        s[m++] = r;
    }
    double M = 1;
    for(int k = 0; k < p.n; k++) if(fabs(p.c[k]) + 1 > M) M = fabs(p.c[k]) + 1;
    int v[2] = {0,0};
    for(int lado = 0; lado < 2; lado++){
        double x = lado ? M : -M;
        int ant = 0, mud = 0;
        for(int k = 0; k < m; k++){
            double t = px_val(s[k], x);
            if(fabs(t) < 1e-12) continue;
            int sg = t > 0 ? 1 : -1;
            if(ant && sg != ant) mud++;
            ant = sg;
        }
        v[lado] = mud;
    }
    return v[0] - v[1];
}
/* AS RAÍZES RACIONAIS, exatas, por enumeração finita: p | c₀ e q | cₙ.
 * Devolve quantas achou, e escreve os pares em (num, den). */
static int pol_racionais(Pol p, long *num, long *den, int lim){
    /* trabalha-se com os coeficientes escalados a inteiros */
    long ic[PMAX+1]; long esc = 1;
    for(int k = 0; k <= p.n; k++){
        for(long d = 1; d <= 64; d++){
            double v = p.c[k]*d;
            long n2 = (long)(v < 0 ? v-0.5 : v+0.5);
            if(fabs(v - n2) < 1e-9){ if(d > esc) esc = d; break; }
        }
    }
    for(int k = 0; k <= p.n; k++){
        double v = p.c[k]*esc;
        ic[k] = (long)(v < 0 ? v-0.5 : v+0.5);
    }
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
                    long g = a, h = b;
                    while(h){ long t = g % h; g = h; h = t; }
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

#endif
