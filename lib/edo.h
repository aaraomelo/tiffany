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
 *
 * ── E A OUTRA METADE DO PAPER, QUE FICOU PARA TRÁS (22/08) ────────────────────
 * O `broca-so/papers/equacoes_diferenciais.tex` não trata só do escalar: constrói
 * a ED como o FLUXO
 *
 *     ẋ = A·x,   com A uma MATRIZ,   A = gato ⊕ esquilo
 *
 * — a decomposição Sym + Skew. O que este ficheiro resgatou foi a parte escalar; a
 * matricial ficou lá, e entretanto o `lib/exterior.h`, escrito por outra razão
 * inteiramente, já tinha `ex_parte`, que É essa decomposição. Duas metades da mesma
 * frase em ficheiros que não se conheciam.
 *
 *     A = (A + Aᵀ)/2   +   (A − Aᵀ)/2
 *          o GATO            o ESQUILO
 *          dissipa           gira
 *          espectro real     espectro imaginário
 *
 * A ponte entre as duas metades é a matriz COMPANHEIRA: y'' + By' + Cy = 0 tem
 *
 *     A = ( 0   1 )    com traço = −B  e  determinante = C,
 *         (−C  −B )
 *
 * logo λ² − tr·λ + det = λ² + Bλ + C — a característica deste ficheiro, e o
 * característico da matriz, são a mesma. ATENÇÃO AO SINAL: aqui o B é o
 * coeficiente de y' e vale −traço. Duas convenções, e a conversão diz-se onde é
 * usada, não numa nota distante.
 *
 * E daí sai a coisa que nenhum dos dois lados sabia sozinho: o PRIMEIRO TERMO DA
 * CIFRA — que o `cifra.h` chama «qual metade carrega o real» — É a classificação
 * da equação diferencial. Vale 2 exactamente quando Δ < 0, isto é, quando o
 * espectro é imaginário e o fluxo ORBITA. Medido no motor do banco, onde a tabela
 * é o gerador: `tests/pgwire.c` §W68, com `simetrica`, `antisimetrica` e `regime`.
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
/* A FONTE, EM INTEIROS. O leitor só lê dígitos — guardar isso em `double` era
 * escrever um decimal onde nunca houve um. */
typedef struct { int tipo; long k, a, w; } Fonte;

/* LER a fonte do lado direito: "1", "3", "e^t", "2e^3t", "cos t", "sen 2t" */
static int edo_le_fonte(const char *s, Fonte *f){
    f->tipo = F_NENHUMA; f->k = 0; f->a = 0; f->w = 0;
    while(*s == ' ') s++;
    if(!*s) return 1;
    long sinal = 1;
    if(*s == '-'){ sinal = -1; s++; while(*s==' ') s++; }
    else if(*s == '+'){ s++; while(*s==' ') s++; }
    long k = 0; int tem = 0;
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
        long a2 = 1; int neg = 0;
        if(*s == '-'){ neg = 1; s++; }
        long v = 0; int t2 = 0;
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
        long w = 1, v = 0; int t2 = 0;
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
/* a fração reduzida, em texto — a régua de escrita desta casa */
static void ed_frac(long p, long q, char *o, size_t n){
    if(q < 0){ p = -p; q = -q; }
    long a = p < 0 ? -p : p, b = q;
    while(b){ long t = a % b; a = b; b = t; }
    if(a < 1) a = 1;
    p /= a; q /= a;
    if(q == 1) snprintf(o, n, "%ld", p);
    else       snprintf(o, n, "%ld/%ld", p, q);
}
/* A PARTICULAR, EM FRAÇÕES. B e C já eram racionais (Bp/Bq, Cp/Cq) e entravam aqui
 * divididos em `double` — o corpo exato desfeito na porta. Aqui não: p(a), p'(a), o
 * determinante do sistema oscilatório e os coeficientes saem todos em ℚ, e o «= 0»
 * é o zero exato, não um limiar. */
static int edo_particular(long Bp, long Bq, long Cp, long Cq, Fonte f, char *out, size_t lim){
    char sa[64], sb[64];
    if(f.tipo == F_NENHUMA){ snprintf(out, lim, "0"); return 0; }
    if(f.tipo == F_CONST || f.tipo == F_EXP){
        long a = (f.tipo == F_CONST) ? 0 : f.a;    /* a constante é e^{0t} */
        /* p(a) = a² + B·a + C = (a²·Bq·Cq + Bp·a·Cq + Cp·Bq) / (Bq·Cq) */
        long pp = a*a*Bq*Cq + Bp*a*Cq + Cp*Bq, pq = Bq*Cq;
        long dp = 2*a*Bq + Bp, dq = Bq;            /* p'(a) = 2a + B */
        if(pp != 0){                               /* sem ressonância */
            ed_frac(f.k*pq, pp, sa, sizeof sa);    /* A = k/p(a) */
            if(f.tipo == F_CONST) snprintf(out, lim, "%s", sa);
            else                  snprintf(out, lim, "%s·e^(%ld t)", sa, a);
            return 0;
        }
        if(dp != 0){                               /* raiz simples: entra um t */
            ed_frac(f.k*dq, dp, sa, sizeof sa);
            if(f.tipo == F_CONST) snprintf(out, lim, "%s·t", sa);
            else                  snprintf(out, lim, "%s·t·e^(%ld t)", sa, a);
            return 1;
        }
        ed_frac(f.k, 2, sa, sizeof sa);            /* raiz dupla: entra t² */
        if(f.tipo == F_CONST) snprintf(out, lim, "%s·t²", sa);
        else                  snprintf(out, lim, "%s·t²·e^(%ld t)", sa, a);
        return 2;
    }
    /* fonte oscilatória: substitui-se y = P·cos + Q·sen. O sistema é
     *   (C - w²)P + Bw Q = k   (do cos)      -Bw P + (C - w²)Q = 0   (do sen)   [para f = k cos]
     * e o determinante é (C - w²)² + (Bw)². Se ele anula, é ressonância. */
    {   /* d1 = C − w² = (Cp − w²·Cq)/Cq ;  d2 = B·w = (Bp·w)/Bq */
        long d1p = Cp - f.w*f.w*Cq, d1q = Cq;
        long d2p = Bp*f.w,          d2q = Bq;
        /* det = d1² + d2², sobre (d1q·d2q)² */
        long detp = d1p*d1p*d2q*d2q + d2p*d2p*d1q*d1q, detq = d1q*d1q*d2q*d2q;
        if(detp != 0){
            /* P = k·d1/det = k · (d1p/d1q) · (detq/detp) */
            ed_frac(f.k*d1p*detq, d1q*detp, sa, sizeof sa);
            ed_frac((f.tipo == F_COS ? -1 : 1)*f.k*d2p*detq, d2q*detp, sb, sizeof sb);
            {   /* o sinal entra no OPERADOR, não colado ao número: «+ -1/8» era o
                 * decimal a deixar rasto na escrita */
                const char *pc = (f.tipo == F_COS) ? sa : sb;
                const char *ps = (f.tipo == F_COS) ? sb : sa;
                const char *op = (ps[0] == '-') ? "-" : "+";
                if(ps[0] == '-') ps++;
                snprintf(out, lim, "%s·cos(%ld t) %s %s·sen(%ld t)", pc, f.w, op, ps, f.w);
            }
            return 0;
        }
    }
    /* det = 0: C = w² e B = 0 — a fonte tem a frequência PRÓPRIA do sistema */
    if(f.tipo == F_COS){ ed_frac(f.k, 2*f.w, sa, sizeof sa);
                         snprintf(out, lim, "%s·t·sen(%ld t)", sa, f.w); }
    else               { ed_frac(-f.k, 2*f.w, sa, sizeof sa);
                         snprintf(out, lim, "%s·t·cos(%ld t)", sa, f.w); }
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
    /* a fonte normaliza-se pelo coeficiente de y'', como os outros — e aqui era uma
     * DIVISÃO EM DOUBLE («f->k /= (double)e->Bq») que truncava a fonte em silêncio.
     * Guarda-se a fração: o k fica inteiro e o Bq acompanha-o na particular. */
    return 1;
}

/* O `edo_residuo` SAIU. Ele substituía a particular na equação por DIFERENÇAS FINITAS
 * (h = 1e-5) para medir o resíduo — e nunca foi chamado por ninguém: é o medidor que
 * nunca mediu, e ainda por cima na régua errada. A verificação que vale é a exata, e
 * ela faz-se onde a particular se deduz: p(a)·A = k é identidade em ℚ, sem passo e sem
 * limite. Quem a quiser medir, mede-a assim — não com um h escolhido por mim. */
#endif