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

#endif
