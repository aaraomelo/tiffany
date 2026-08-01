/* edo.h — A EQUAÇÃO DIFERENCIAL É A BORDA DO CORPO, COM D NO LUGAR DO σ.
 *
 * O Aarão: "agora a assistente vai resolver equações diferenciais; resgata o corpo diferencial,
 * acho que em chess/ ou broca-so/."
 *
 * Estava em chess/universe/tools/diferencial.c, e o que lá está medido é isto: o regime de uma
 * ED não é imposto, é o SINAL DE Re(λ), e os três regimes SÃO o gato e o esquilo —
 *
 *     CRISTAL  Re λ < 0   colapsa no ponto fixo      (dissipa)
 *     BORDA    Re λ = 0   orbita, conserva a norma   (o esquilo)
 *     CAOS     Re λ > 0   diverge, mistura           (o gato)
 *
 * e o dicionário do paper diz "metal = o autovalor σₙ" e "reta = a taxa Re(λ) = log σ".
 *
 * AQUI FECHA-SE, E NÃO É PRECISO INVENTAR NADA. Uma ED linear de coeficientes constantes
 *
 *     y'' + B y' + C y = 0
 *
 * tem equação característica λ² + Bλ + C = 0. E a borda da álgebra global é
 *
 *     σ² = b₀ + b₁σ,   isto é,   σ² − b₁σ − b₀ = 0.
 *
 * São a MESMA equação, com B = −b₁ e C = −b₀. Resolver a ED é declarar o corpo: o operador de
 * derivação D ocupa o lugar do marcador σ, e o Δ = B² − 4C que classifica as soluções é o MESMO
 * Δ que classifica os corpos do catálogo — hiperbólico, parabólico, elíptico.
 *
 *   Δ > 0   duas raízes reais       exponenciais       o GATO: cresce e gasta
 *   Δ = 0   raiz dupla              t·e^{λt}           a fronteira, o absorvente
 *   Δ < 0   par conjugado           seno e cosseno     o ESQUILO: gira e não gasta
 *
 * E dois casos fecham o círculo com o resto do sistema:
 *   y'' = -y      -> borda σ² = −1     -> é o i, e a solução é a rotação
 *   y'' = y' + y  -> borda σ² = σ + 1  -> é o OURO, e a solução é φ^t
 */
#ifndef EDO_H
#define EDO_H

#include <stdio.h>
#include <string.h>
#include <math.h>

/* y'' + B y' + C y = 0, com B e C racionais */
typedef struct {
    long Bp, Bq, Cp, Cq;      /* os coeficientes, em Q */
    long D;                   /* o discriminante, quando B e C são inteiros */
    int  classe;              /* +1 hiperbólico, 0 parabólico, -1 elíptico */
    int  bom;
} Edo;

/* LER "y'' + 2y' + y = 0", "y'' = y' + y", "y'' = -y", "2y'' - 3y = 0".
 *
 * Junta os dois lados: o que está à direita do '=' entra com sinal trocado. Depois normaliza
 * dividindo pelo coeficiente de y''. */
static int edo_le(const char *s, Edo *e){
    long c[3] = {0,0,0};                   /* c[0]·y + c[1]·y' + c[2]·y'' */
    int lado = 1, achou = 0;
    for(;;){
        while(*s == ' ') s++;
        if(!*s) break;
        if(*s == '='){ lado = -1; s++; continue; }
        int sinal = 1;
        if(*s == '+'){ s++; while(*s==' ') s++; }
        else if(*s == '-'){ sinal = -1; s++; while(*s==' ') s++; }
        long v = 0; int tem = 0;
        while(*s >= '0' && *s <= '9'){ v = v*10 + (*s-'0'); s++; tem = 1; }
        if(!tem) v = 1;
        while(*s == ' ') s++;
        if(*s != 'y'){
            if(tem && v == 0) continue;    /* o "= 0" do fim */
            return 0;
        }
        s++;
        int ordem = 0;
        while(*s == '\'' ){ ordem++; s++; }
        if(ordem > 2) return 0;
        c[ordem] += lado * sinal * v;
        achou = 1;
    }
    if(!achou || c[2] == 0) return 0;
    /* normaliza: divide por c[2] */
    long a = c[2];
    e->Bp = c[1]; e->Bq = a;
    e->Cp = c[0]; e->Cq = a;
    if(e->Bq < 0){ e->Bp = -e->Bp; e->Bq = -e->Bq; }
    if(e->Cq < 0){ e->Cp = -e->Cp; e->Cq = -e->Cq; }
    /* o discriminante: Δ = B² − 4C, em Q; guarda-se o numerador sobre o quadrado comum */
    /* com B = Bp/a e C = Cp/a: Δ = (Bp² − 4·Cp·a)/a² — e o SINAL é o que importa */
    e->D = e->Bp*e->Bp - 4*e->Cp*a/(a?1:1);
    if(a != 1){ e->D = e->Bp*e->Bp - 4*e->Cp*a; }
    e->classe = e->D > 0 ? 1 : e->D < 0 ? -1 : 0;
    e->bom = 1;
    return 1;
}
/* A NÃO HOMOGÉNEA lê-se partindo no '=': se o lado direito tem y, é tudo homogénea e junta-se
 * (é o caso "y'' = -y"); se não tem, a esquerda é a equação e a direita é a FONTE.
 *
 * Simples e sem estado a atravessar: a primeira versão tentou marcar a fonte a meio da leitura
 * dos coeficientes e ficou com duas coisas a acontecer no mesmo laço. Partir primeiro é uma
 * linha e não tem esse defeito. */
/* (definida no fim, depois do tipo Fonte) */

/* a borda equivalente, na notação da álgebra global: σ² = b₀ + b₁σ com b₁ = −B, b₀ = −C */
static void edo_borda(Edo e, char *out, size_t lim){
    long b1p = -e.Bp, b0p = -e.Cp, q = e.Bq;
    char sb0[48], sb1[48];
    if(q == 1) snprintf(sb0, sizeof sb0, "%ld", b0p);
    else       snprintf(sb0, sizeof sb0, "%ld/%ld", b0p, q);
    if(q == 1) snprintf(sb1, sizeof sb1, "%ld", b1p < 0 ? -b1p : b1p);
    else       snprintf(sb1, sizeof sb1, "%ld/%ld", b1p < 0 ? -b1p : b1p, q);
    if(b1p == 0)      snprintf(out, lim, "s^2 = %s", sb0);
    else if(b0p == 0) snprintf(out, lim, "s^2 = %s%ss", b1p < 0 ? "-" : "",
                               strcmp(sb1,"1") ? sb1 : "");
    else              snprintf(out, lim, "s^2 = %s %c %ss", sb0, b1p < 0 ? '-' : '+',
                               strcmp(sb1,"1") ? sb1 : "");
}


/* ─── A NÃO HOMOGÉNEA: y'' + By' + Cy = f(t) ──────────────────────────────────────────────
 *
 * A solução geral é y = y_h + y_p — a homogénea MAIS uma particular. E isso diz uma coisa
 * sobre a estrutura: o conjunto das soluções NÃO é um espaço vetorial, é um espaço vetorial
 * TRANSLADADO. A homogénea é o corpo livre; a fonte desloca-o, e não o deforma.
 *
 * E a RESSONÂNCIA é o mesmo fenómeno da raiz dupla, noutra escala. Substituindo y = A·e^{at}
 * em y'' + By' + Cy sai A·p(a)·e^{at}, com p(a) = a² + Ba + C — o próprio polinómio
 * característico. Se p(a) ≠ 0, A = k/p(a) e acabou. Se p(a) = 0, a fonte cai SOBRE o espectro,
 * não há A que sirva, e é preciso um t a multiplicar. É o mesmo t que aparece quando a raiz é
 * dupla — e é o mesmo motivo: o denominador anulou-se.
 *
 * A fonte é o lado NEGRO da dualidade (o sorvedouro/fonte); a homogénea é o livre.
 */
#define F_NENHUMA 0
#define F_CONST   1     /* k                */
#define F_EXP     2     /* k·e^{at}         */
#define F_COS     3     /* k·cos(wt)        */
#define F_SEN     4     /* k·sen(wt)        */
typedef struct { int tipo; double k, a, w; } Fonte;

/* LER a fonte do lado direito: "1", "3", "e^t", "2e^3t", "cos t", "sen 2t" */
static int edo_le_fonte(const char *s, Fonte *f){
    f->tipo = F_NENHUMA; f->k = 0; f->a = 0; f->w = 0;
    while(*s == ' ') s++;
    if(!*s) return 1;
    double sinal = 1;
    if(*s == '-'){ sinal = -1; s++; while(*s==' ') s++; }
    else if(*s == '+'){ s++; while(*s==' ') s++; }
    double k = 0; int tem = 0;
    while(*s >= '0' && *s <= '9'){ k = k*10 + (*s-'0'); s++; tem = 1; }
    if(!tem) k = 1;
    while(*s == ' ') s++;
    if(!*s){                                   /* só um número: constante */
        if(!tem) return 0;
        f->tipo = F_CONST; f->k = sinal*k;
        if(f->k == 0) f->tipo = F_NENHUMA;
        return 1;
    }
    if(*s == 'e' && s[1] == '^'){
        s += 2;
        double a2 = 1; int neg = 0;
        if(*s == '-'){ neg = 1; s++; }
        double v = 0; int t2 = 0;
        while(*s >= '0' && *s <= '9'){ v = v*10 + (*s-'0'); s++; t2 = 1; }
        if(t2) a2 = v;
        if(*s == 't') s++;
        f->tipo = F_EXP; f->k = sinal*k; f->a = neg ? -a2 : a2;
        return 1;
    }
    if(!strncmp(s, "cos", 3) || !strncmp(s, "sen", 3)){
        int ec = (*s == 'c');
        s += 3;
        while(*s == ' ') s++;
        double w = 1, v = 0; int t2 = 0;
        while(*s >= '0' && *s <= '9'){ v = v*10 + (*s-'0'); s++; t2 = 1; }
        if(t2) w = v;
        if(*s == 't') s++;
        f->tipo = ec ? F_COS : F_SEN; f->k = sinal*k; f->w = w;
        return 1;
    }
    return 0;
}

/* a solução PARTICULAR, e o grau de ressonância (0 = nenhuma, 1 = simples, 2 = dupla).
 * Escreve a forma em `out`, e devolve a ressonância. */
static int edo_particular(double B, double C, Fonte f, char *out, size_t lim){
    if(f.tipo == F_NENHUMA){ snprintf(out, lim, "0"); return 0; }
    if(f.tipo == F_CONST || f.tipo == F_EXP){
        double a = (f.tipo == F_CONST) ? 0 : f.a;      /* a constante é e^{0t} */
        double p  = a*a + B*a + C;                     /* p(a) — o característico na fonte */
        double dp = 2*a + B;                           /* p'(a) */
        if(fabs(p) > 1e-12){
            double A = f.k / p;
            if(f.tipo == F_CONST) snprintf(out, lim, "%g", A);
            else                  snprintf(out, lim, "%g·e^(%g t)", A, a);
            return 0;
        }
        if(fabs(dp) > 1e-12){                          /* raiz simples: entra um t */
            double A = f.k / dp;
            if(f.tipo == F_CONST) snprintf(out, lim, "%g·t", A);
            else                  snprintf(out, lim, "%g·t·e^(%g t)", A, a);
            return 1;
        }
        double A = f.k / 2;                            /* raiz dupla: entra t² */
        if(f.tipo == F_CONST) snprintf(out, lim, "%g·t²", A);
        else                  snprintf(out, lim, "%g·t²·e^(%g t)", A, a);
        return 2;
    }
    /* fonte oscilatória: substitui-se y = P·cos + Q·sen. O sistema é
     *   (C - w²)P + Bw Q = k   (do cos)      -Bw P + (C - w²)Q = 0   (do sen)   [para f = k cos]
     * e o determinante é (C - w²)² + (Bw)². Se ele anula, é ressonância. */
    double d1 = C - f.w*f.w, d2 = B*f.w;
    double det = d1*d1 + d2*d2;
    if(fabs(det) > 1e-12){
        double P, Q;
        if(f.tipo == F_COS){ P = f.k*d1/det;  Q = -f.k*d2/det; }
        else               { P = f.k*d2/det;  Q =  f.k*d1/det; }
        snprintf(out, lim, "%g·cos(%g t) + %g·sen(%g t)", P, f.w, Q, f.w);
        return 0;
    }
    /* det = 0: C = w² e B = 0 — a fonte tem a frequência PRÓPRIA do sistema */
    if(f.tipo == F_COS) snprintf(out, lim, "%g·t·sen(%g t)", f.k/(2*f.w), f.w);
    else                snprintf(out, lim, "%g·t·cos(%g t)", -f.k/(2*f.w), f.w);
    return 1;
}


static int edo_le_nh(const char *s, Edo *e, Fonte *f){
    f->tipo = F_NENHUMA;
    const char *ig = strchr(s, '=');
    if(!ig) return edo_le(s, e);
    if(strchr(ig, 'y')) return edo_le(s, e);      /* y dos dois lados: homogénea */
    char esq[256];
    snprintf(esq, sizeof esq, "%.*s", (int)(ig - s), s);
    if(!edo_le(esq, e)) return 0;
    if(!edo_le_fonte(ig + 1, f)) return 0;
    /* normaliza a fonte pelo coeficiente de y'', como se fez aos outros */
    if(e->Bq != 1) f->k /= (double)e->Bq;
    return 1;
}

/* VERIFICAR: substitui a particular na equação e mede o resíduo. Sem isto eu estaria a confiar
 * numa dedução minha, e a dedução é minha. Faz-se por diferenças finitas de passo pequeno num
 * ponto, o que basta para apanhar um coeficiente errado (que é o erro que aqui se comete). */
static double edo_residuo(double B, double C, Fonte f, double (*yp)(double, void*), void *ctx,
                          double t){
    double h = 1e-5;
    double y0 = yp(t, ctx), yp1 = (yp(t+h,ctx) - yp(t-h,ctx)) / (2*h);
    double yp2 = (yp(t+h,ctx) - 2*y0 + yp(t-h,ctx)) / (h*h);
    double fv = f.tipo == F_CONST ? f.k
              : f.tipo == F_EXP   ? f.k*exp(f.a*t)
              : f.tipo == F_COS   ? f.k*cos(f.w*t)
              : f.tipo == F_SEN   ? f.k*sin(f.w*t) : 0;
    return yp2 + B*yp1 + C*y0 - fv;
}

#endif