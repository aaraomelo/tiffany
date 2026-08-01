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
/* AS RAÍZES, todas de uma vez. Devolve 1 se convergiu. */
static int pol_raizes(Pol p, double complex *z){
    int n = p.n;
    double complex s = 0.4 + 0.9*I, w = 1;
    for(int k = 0; k < n; k++){ z[k] = w; w *= s; }
    for(int it = 0; it < 5000; it++){
        double mov = 0;
        for(int i = 0; i < n; i++){
            double complex d = 1;
            for(int j = 0; j < n; j++) if(j != i) d *= (z[i] - z[j]);
            if(cabs(d) < 1e-300) continue;
            double complex passo = pol_val(p, z[i]) / d;
            z[i] -= passo;
            if(cabs(passo) > mov) mov = cabs(passo);
        }
        if(mov < 1e-14) return 1;
    }
    return 0;
}
/* o resíduo máximo: substitui cada raiz e mede |p(z)| */
static double pol_residuo(Pol p, double complex *z){
    double m = 0;
    for(int k = 0; k < p.n; k++){ double r = cabs(pol_val(p, z[k])); if(r > m) m = r; }
    return m;
}
/* a assinatura (r, s): r raízes reais, s pares complexos */
static void pol_assinatura(Pol p, double complex *z, int *r, int *s){
    *r = 0; *s = 0;
    for(int k = 0; k < p.n; k++){
        if(fabs(cimag(z[k])) < 1e-9) (*r)++; else (*s)++;
    }
    *s /= 2;
}
/* uma raiz é racional simples? devolve 1 e escreve p/q se for de denominador pequeno */
static int pol_racional(double x, long *pn, long *qn){
    for(long q = 1; q <= 64; q++){
        double v = x*q;
        long n = (long)(v < 0 ? v - 0.5 : v + 0.5);
        if(fabs(v - n) < 1e-9){ *pn = n; *qn = q; return 1; }
    }
    return 0;
}

#endif
