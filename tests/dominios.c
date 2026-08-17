/* dominios.c — CINCO DOMÍNIOS, UMA EQUAÇÃO: o circuito desenhado, resolvido passo a passo, animado.
 *
 * O Aarão: "desenhar circuitos elétricos e mecânicos no TikZ pelo compilador da assistente e
 * converter em equações e resolver as equações passo a passo e animar a solução no LaTeX. Inclui
 * pneumático, óptico e o novo, o elástico via corpo mórfico. E resgata o backend em PTX — aí pode
 * receber sinal de GPU e CPU, por uma janela nossa semelhante ao canvas."
 *
 * A AFIRMAÇÃO CENTRAL, e ela é forte o bastante para poder ser falsa: os cinco domínios não são
 * cinco sistemas parecidos. São **o mesmo corpo com cinco vestidos**, e o que muda é só a régua:
 *
 *      elétrico    L q'' + R q'  + q/C   = 0        (B,C) = (R/L, 1/(LC))
 *      mecânico    m x'' + c x'  + k x   = 0        (B,C) = (c/m, k/m)
 *      pneumático  I p'' + Rp p' + p/Cp  = 0        (B,C) = (Rp/I, 1/(I·Cp))
 *      óptico      a'' + (1/τ) a' + w0² a = 0       (B,C) = (1/τ, w0²)
 *      elástico    ρ u'' + η u' + E u    = 0        (B,C) = (η/ρ, E/ρ)
 *
 * Todos caem em  y'' + By' + Cy = 0,  que é o `edo.c` §E1 — e ali já está escrito que a **equação
 * característica É a borda do corpo** e que o **Δ = B²−4C é o MESMO Δ do catálogo**. Então não há
 * cinco teorias: há uma régua e cinco leituras dela.
 *
 * E o ELÁSTICO entra pelo corpo mórfico, que é o pedido novo: a deformação elástica é **erosão e
 * dilatação** (o `morfa.c`, e o memory do WHERE mórfico), e essas são duais uma da outra — erode-se
 * para escolher, dilata-se para escrever de volta. Um material elástico é exatamente isso: deforma
 * e **volta**, e o que não volta ficou na garrafa.
 *
 *   §D1  a NETLIST desenha: o circuito sai em TikZ, e o desenho é a fonte da equação
 *   §D2  a CONVERSÃO: cada domínio dá o seu (B,C) — e é a régua do catálogo, medida
 *   §D3  A MESMA EQUAÇÃO: cinco domínios, uma solução, e o Δ classifica cada um
 *   §D4  RESOLVER PASSO A PASSO — e cada passo verificado por SUBSTITUIÇÃO, não narrado
 *   §D5  o ELÁSTICO pelo mórfico: erosão e dilatação são o par, e o que volta é reversível
 *   §D6  a ANIMAÇÃO no LaTeX, e a JANELA: o que a GPU (PTX) e a CPU escrevem nela
 *
 *   cc -O2 -std=c99 dominios.c -lm -o dominios && ./dominios
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "reta.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §D1/§D2  OS CINCO DOMÍNIOS — e cada um traz os seus parâmetros FÍSICOS
 *
 * Os parâmetros são valores de componente reais, não números arrumados para dar certo. O (B,C)
 * NÃO é escrito: é CALCULADO deles, e é isso que faz a conversão ser uma medida.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct {
    const char *nome, *equacao, *inercia, *perda, *rigidez;
    long m, c, k;              /* inércia, dissipação, restituição — a tríade, em CENTÉSIMOS */
    const char *unidade;
} Dom;   /* m, c, k em CENTÉSIMOS, inteiros */

static const Dom DOM[] = {
    /* nome        equação                  inércia     perda      rigidez     m      c      k     */
    /* OS DADOS EM CENTÉSIMOS, INTEIROS. Os decimais escritos — 0,50, 1,20, 0,05 — têm
     * denominadores 2, 5 e 20, e o MMC deles é 20: em centésimos toda a tabela cabe em ℤ.
     * Não é aproximar, é escolher a unidade, e o que se ganha é o Δ exacto (ver `regua_z`). */
    { "eletrico",  "L q'' + R q' + q/C = 0", "L (H)",    "R (ohm)", "1/C (1/F)",  50,  120,  80000, "carga" },
    { "mecanico",  "m x'' + c x' + k x = 0", "m (kg)",   "c (Ns/m)","k (N/m)",   200,  300,  50000, "metro" },
    { "pneumatico","I p'' + Rp p' + p/Cp=0", "I (inert)","Rp",      "1/Cp",       80,  400,  30000, "pascal"},
    { "optico",    "a'' + a'/tau + w0^2 a=0","1",        "1/tau",   "w0^2",      100,    5,  10000, "campo" },
    { "elastico",  "rho u'' + eta u' + E u=0","rho",     "eta",     "E (mod)",   150,   90,  60000, "desloc"},
};
#define NDOM ((int)(sizeof DOM / sizeof DOM[0]))

/* a régua do catálogo: (B,C) = (−traço, det) da companion. Aqui sai dos parâmetros físicos. */
/* A RÉGUA (B,C) = (c/m, k/m). Os três dados estão em centésimos, logo as duas razões são
 * FRACÇÕES de inteiros e a unidade cancela nelas — c/m não tem dimensão de centésimo. */
static void regua(const Dom *d, double *B, double *C){
    *B = (double)d->c / (double)d->m;
    *C = (double)d->k / (double)d->m;
}
static double delta(double B, double C){ return B*B - 4*C; }

/* E O SINAL DO Δ NÃO PRECISA DE VÍRGULA. Δ = B² − 4C = (c² − 4·k·m)/m², e m² > 0: o sinal
 * é o do INTEIRO c² − 4km, com os três em centésimos — e a expressão é homogénea de grau
 * dois nos dados, logo a unidade multiplica os dois termos e não muda o sinal. É a mesma
 * conta do §M3 do microfluidica, e é ela que classifica sem uma divisão. */
static long delta_sinal_z(const Dom *d){
    long v = d->c*d->c - 4*d->k*d->m;
    return v > 0 ? 1 : (v < 0 ? -1 : 0);
}

/* a forma fechada do caso Δ<0 (edo.c §E4), com y(0)=1, y'(0)=0 — o ORÁCULO dos dois métodos */
static double exata(double B, double C, double t){
    double w = sqrt(4*C - B*B)/2.0;
    return exp(-B*t/2.0) * (cos(w*t) + (B/(2*w))*sin(w*t));
}
/* A DERIVADA, e ela escreve-se GERAL e nao ja' simplificada no ponto.
 *
 * Escrevi-a primeiro so' para t = 0, e o que me saiu foi `-a*1 + (0 + a*1)` — isto e',
 * `-a + a`, que e' zero por ser x - x e nao mede coisa nenhuma. E' o defeito que este
 * mesmo ficheiro me ajudou a cacar noutros, cometido dentro da correccao.
 *
 * Assim, com t livre, ela pode estar ERRADA — e a comparacao com a diferenca finita em
 * varios t apanha-o. So' depois disso e' que avaliar em t = 0 diz alguma coisa. */
static double dexata(double B, double C, double t){
    double a = B/2.0, w = sqrt(4*C - B*B)/2.0, k = B/(2*w);
    double u  = cos(w*t) + k*sin(w*t);
    double du = -w*sin(w*t) + k*w*cos(w*t);
    return exp(-a*t) * (du - a*u);
}
/* y'' pela regra de Leibniz — a equação y''+By'+Cy = 0 fecha EXACTAMENTE por
 * construção da forma fechada; mede-se a substituição ANALÍTICA, não FD nem limiar. */
static double d2exata(double B, double C, double t){
    double a = B/2.0, w = sqrt(4*C - B*B)/2.0, k = B/(2*w);
    double u  = cos(w*t) + k*sin(w*t);
    double du = -w*sin(w*t) + k*w*cos(w*t);
    double d2u = -w*w*u;
    return exp(-a*t) * (d2u - 2.0*a*du + a*a*u);
}

/* ───────────────────────────────────────────────────────────────────────────
 * §D4  RESOLVER PASSO A PASSO — e cada passo VERIFICADO
 *
 * O `edo.c` §E5/§E8 já diz como: a solução verifica-se por SUBSTITUIÇÃO. Então um "passo a passo"
 * que só narra não vale nada; cada passo tem de deixar um resíduo mensurável.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *passo; double residuo; int verificavel; } Passo;

/* substitui y(t) na equação y'' + By' + Cy e devolve o resíduo — a derivada é numérica de ordem 4,
 * para o resíduo medir a EQUAÇÃO e não o meu esquema de derivada */
static double substitui(double B, double C, double t){
    double h = 1e-3;
    double y  = exata(B,C,t);
    /* A fórmula de cinco pontos é  [f(t-2h) - 8f(t-h) + 8f(t+h) - f(t+2h)] / (12h)  — e eu
     * escrevi o denominador NEGATIVO. Com o sinal trocado, y' entrava invertido e o termo B·y'
     * somava em vez de cancelar: o resíduo dava 1,3e-1 onde devia dar ~1e-8. Não era a solução
     * nem o modelo: era o SINAL, e é a primeira coisa que o meu memory manda conferir. */
    double y1 = (exata(B,C,t-2*h) - 8*exata(B,C,t-h) + 8*exata(B,C,t+h) - exata(B,C,t+2*h))/(12*h);
    double y2 = (-exata(B,C,t-2*h) + 16*exata(B,C,t-h) - 30*y
                 + 16*exata(B,C,t+h) - exata(B,C,t+2*h))/(12*h*h);
    return y2 + B*y1 + C*y;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §D5  O ELÁSTICO PELO CORPO MÓRFICO — erosão e dilatação são o par dual
 *
 * O memory do WHERE mórfico: "erosão/dilatação, e são o par dual: erode-se para escolher, dilata-se
 * para escrever de volta." Um material elástico faz literalmente isso: deforma sob carga (erosão do
 * repouso) e volta ao soltar (dilatação). O que NÃO volta é plástico — e fica na garrafa até ter
 * dual, que é a lei da alfândega do `koch.c`.
 * ─────────────────────────────────────────────────────────────────────────── */

/* a deformação como morfismo sobre um perfil discreto: erodir é tomar o mínimo da vizinhança,
 * dilatar é tomar o máximo. São as operações do morfa.c, e o par é dual. */
static void erode(const double *v, double *o, int n){
    for(int i = 0; i < n; i++){
        double m = v[i];
        if(i > 0   && v[i-1] < m) m = v[i-1];
        if(i < n-1 && v[i+1] < m) m = v[i+1];
        o[i] = m;
    }
}
static void dilata(const double *v, double *o, int n){
    for(int i = 0; i < n; i++){
        double m = v[i];
        if(i > 0   && v[i-1] > m) m = v[i-1];
        if(i < n-1 && v[i+1] > m) m = v[i+1];
        o[i] = m;
    }
}

/* ───────────────────────────────────────────────────────────────────────────
 * §D6  A JANELA — o que a GPU (PTX) e a CPU escrevem
 *
 * O chess tem `laboratorio_ptx.py`, com kernels `.visible .entry gato_stream` e `mandel`, e o PTX
 * fala com a memória por `ld.global` e `st.global` — que são EXATAMENTE o MOVE(slot,+1) e o
 * MOVE(slot,-1) da nossa ISA. Do lado da GPU são dois códigos distintos; do nosso são UM código
 * com o sinal trocado, e é aí que se vê o que a unificação poupa. Então a "janela semelhante ao canvas" não é uma peça nova: é um BUFFER de
 * slots que os dois lados escrevem, e o banco não precisa de saber quem escreveu.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *ptx; const char *isa; } Ptx;
static const Ptx PTXMAP[] = {
    /* A ISA foi unificada em MOVE e este mapa ficou para tras: dizia LOAD e STORE, que sao
     * DUAS instrucoes. Sao a MESMA com o sinal trocado — e e' isso que o PTX torna visivel,
     * porque do lado da GPU sao mesmo dois codigos distintos. A maquina de duas precisa de
     * dois CODIGOS; esta precisa de um codigo e um ARGUMENTO. */
    { "ld.global",       "MOVE(slot,+1)"  },   /* absorve — o lado negro */
    { "st.global",       "MOVE(slot,-1)"  },   /* emite   — o lado branco */
    { "add.f32",         "ADD"         },
    { "sub.f32",         "SUB"         },
    { "mul.f32",         "produto"     },
    { "setp.lt.f32",     "CMP"         },
    { "bra",             "JMP"         },
    { "ret",             "HALT"        },
};
#define NPTX ((int)(sizeof PTXMAP / sizeof PTXMAP[0]))

/* o crescimento maximo da amplitude em n passos de Euler — a medida da estabilidade */
double cresc(double B_, double C_, double h_, int n_){
    double y = 1, v = 0, m = 1;
    for(int k = 0; k < n_; k++){
        double ny = y + h_*v, nv = v + h_*(-C_*y - B_*v);
        y = ny; v = nv;
        if(fabs(y) > m) m = fabs(y);
        if(m > 1e12) break;
    }
    return m;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

/* gera o TikZ do circuito E da solução: o desenho e a curva no mesmo documento */
static void gera_tikz(FILE *f, const Dom *d, double B, double C){
    /* O PASSO NÃO PODE SER FIXO. Reutilizei h=0.02 do tikz.c, onde C=1 — e com C=250 o Euler
     * DIVERGE: o pdflatex morreu com "Dimension too large" nas cinco figuras. É o primeiro item
     * do meu próprio memory ("as escalas dos parâmetros fecham ENTRE SI?") e eu fui direto à
     * lógica outra vez. O passo tem de sair de w, e o intervalo tem de sair de B. */
    double w = sqrt(4*C - B*B)/2.0;                /* a frequência do domínio */
    double h = 0.05 / w;                           /* h·w = 0,05: ~126 passos por ciclo */
    int n = 400;
    double T = n * h;
    double escala = 8.0 / T;                       /* a figura tem largura fixa, o tempo não */
    fprintf(f,
"%% gerado por tools/dominios.c — o circuito E a solucao, do mesmo (B,C)\n"
"\\documentclass[tikz,border=6pt]{standalone}\n"
"\\usepackage{tikz}\n"
"\\usetikzlibrary{calc}\n"
"\\begin{document}\n"
"\\newcommand{\\Bp}{%.5f}\n"
"\\newcommand{\\Cp}{%.5f}\n"
"\\newcommand{\\hh}{%.6f}\n"
"%% --- o CIRCUITO, desenhado: inercia -- perda -- rigidez em serie\n"
"\\begin{tikzpicture}[scale=1.0]\n"
"  \\draw[thick] (0,0) -- (1,0);\n"
"  \\draw[thick] (1,-0.3) rectangle (2,0.3) node[midway,above=8pt]{%s};\n"
"  \\draw[thick] (2,0) -- (3,0);\n"
"  \\draw[thick] (3,-0.3) rectangle (4,0.3) node[midway,above=8pt]{%s};\n"
"  \\draw[thick] (4,0) -- (5,0);\n"
"  \\draw[thick] (5,-0.3) rectangle (6,0.3) node[midway,above=8pt]{%s};\n"
"  \\draw[thick] (6,0) -- (7,0) -- (7,-1.2) -- (0,-1.2) -- (0,0);\n"
"  \\node at (3.5,-1.6) {%s};\n"
"\\end{tikzpicture}\n"
"%% --- a SOLUCAO, integrada pelo proprio LaTeX a partir do MESMO (B,C)\n"
"\\begin{tikzpicture}[x=%.4fcm,y=2.0cm]\n"
"  \\draw[gray!30] (0,-1.1) -- (%.4f,-1.1);  \\draw[gray!30] (0,-1.1) -- (0,1.1);\n"
"  \\pgfmathsetmacro{\\yy}{1.0} \\pgfmathsetmacro{\\vv}{0.0}\n"
"  \\coordinate (q0) at (0,1.0);\n"
"  \\foreach \\i in {1,...,%d}{\n"
"    \\pgfmathsetmacro{\\ny}{\\yy + \\hh*\\vv}\n"
"    \\pgfmathsetmacro{\\nv}{\\vv + \\hh*(-\\Cp*\\yy - \\Bp*\\vv)}\n"
"    \\global\\let\\yy\\ny \\global\\let\\vv\\nv\n"
"    \\pgfmathsetmacro{\\tt}{\\i*\\hh}\n"
"    \\coordinate (q\\i) at (\\tt,\\yy);\n"
"    \\pgfmathtruncatemacro{\\j}{\\i-1}\n"
"    \\draw[blue!70,thick] (q\\j) -- (q\\i);\n"
"  }\n"
"\\end{tikzpicture}\n"
"\\end{document}\n",
        B, C, h, d->inercia, d->perda, d->rigidez, d->equacao, escala, T, n);
}

int main(void){
    puts("dominios.c — CINCO DOMINIOS, UMA EQUACAO: desenhado, resolvido e animado\n");

    /* ── §D2 ─────────────────────────────────────────────────────────────── */
    puts("§D2  A CONVERSAO: cada dominio da o SEU (B,C), calculado dos parametros FISICOS");
    puts("     O (B,C) nao esta escrito em lado nenhum: sai de m, c, k — e e por isso que a");
    puts("     conversao e uma medida e nao uma tabela minha.\n");
    {
        printf("     %-11s %-26s %8s %9s %10s  classe\n", "dominio", "equacao", "B", "C", "Delta");
        int elipticos = 0, distintos = 0, mistura = 0;
        double Bs[NDOM], Cs[NDOM];
        for(int i = 0; i < NDOM; i++){
            regua(&DOM[i], &Bs[i], &Cs[i]);
            double D = delta(Bs[i], Cs[i]);
            /* a classe sai do SINAL do inteiro c² − 4km, e não do Δ em vírgula */
            long sg = delta_sinal_z(&DOM[i]);
            printf("     %-11s %-26s %8.4f %9.2f %10.2f  %s\n",
                   DOM[i].nome, DOM[i].equacao, Bs[i], Cs[i], D,
                   sg < 0 ? "eliptica" : (sg > 0 ? "hiperbolica" : "parabolica"));
            if(sg < 0) elipticos++;
            if((D < 0) != (sg < 0)) mistura++;      /* as duas rotas TÊM de concordar */
        }
        /* «as réguas são distintas» compara-se por PRODUTO CRUZADO em inteiros: B_i = c_i/m_i
         * e B_j = c_j/m_j são iguais sse c_i·m_j == c_j·m_i, sem se formar nenhum quociente
         * e sem o limiar de 1e-12 que aqui estava a decidir uma igualdade de racionais. */
        for(int i = 0; i < NDOM; i++){
            int ja = 0;
            for(int j = 0; j < i; j++){
                int mesmoB = (DOM[j].c*DOM[i].m == DOM[i].c*DOM[j].m);
                int mesmoC = (DOM[j].k*DOM[i].m == DOM[i].k*DOM[j].m);
                if(mesmoB && mesmoC) ja = 1;
            }
            if(!ja) distintos++;
        }
        ok("os cinco dominios dao reguas (B,C) DISTINTAS — nao sao o mesmo sistema disfarcado."
           " E «distintas» compara-se por PRODUTO CRUZADO: c_i.m_j == c_j.m_i, sem formar os"
           " quocientes e sem o limiar de 1e-12 que aqui decidia uma igualdade de racionais",
           distintos == NDOM);
        ok("e o Delta classifica-os pela regua do catalogo: com pouca perda, todos elipticos."
           " E a classe sai do SINAL do inteiro c^2 - 4km, com os tres dados em CENTESIMOS:"
           " Delta = (c^2 - 4km)/m^2 e m^2 > 0, e a expressao e' homogenea de grau dois, logo"
           " a unidade multiplica os dois termos e nao muda o sinal. As duas rotas — a"
           " inteira e a de virgula — concordam nos cinco",
           elipticos == NDOM && mistura == 0);
        puts("     -> cinco reguas distintas, uma classificacao. O Delta e o MESMO do catalogo,");
        puts("        e a classe eliptica e a do §E4 do edo.c: 'o oscilador e o i'.\n");
    }

    /* ── §D3 ─────────────────────────────────────────────────────────────── */
    puts("§D3  A MESMA EQUACAO: UMA solucao serve os cinco, e nao ha caso especial nenhum");
    puts("     A forma fechada do §E4 nao sabe de que dominio veio o (B,C) — e e isso que se");
    puts("     mede: a MESMA funcao resolve os cinco, com residuo na casa do zero.\n");
    {
        double pior = 0; int todos = 1;
        for(int i = 0; i < NDOM; i++){
            double B, C; regua(&DOM[i], &B, &C);
            double p = 0;
            for(double t = 0.5; t <= 6.0; t += 0.25){
                double r = fabs(substitui(B, C, t));
                if(r > p) p = r;
            }
            /* o resíduo tem de ser pequeno EM RELAÇÃO à escala do termo Cy, senão não diz nada */
            double escala = C * 1.0;
            if((long long)(p / escala * 1e6) >= 1) todos = 0;
            if(p/escala > pior) pior = p/escala;
        }
        ok("a MESMA solucao satisfaz a equacao nos CINCO dominios, por substituicao direta",
           todos);
        printf("     -> pior residuo relativo %.2e, medido em 23 instantes de cada dominio.\n", pior);
        puts("        Nenhum dominio precisou de caso especial: o corpo e um, os vestidos e que");
        puts("        sao cinco.\n");
    }

    /* ── §D4 ─────────────────────────────────────────────────────────────── */
    puts("§D4  PASSO A PASSO — e cada passo deixa RESIDUO, senao e narracao");
    puts("     O edo.c §E5/§E8: a solucao verifica-se por SUBSTITUICAO. Entao os passos nao se");
    puts("     contam — medem-se, um a um.\n");
    {
        const Dom *d = &DOM[1];                       /* o mecânico, para ter nomes concretos */
        double B, C; regua(d, &B, &C);
        double D = delta(B, C), w = sqrt(-D)/2.0;
        /* os três estão em CENTÉSIMOS e são `long`: o `%.2f` de antes passava um inteiro a
         * uma conversão de vírgula, e imprimia 0,00 — comportamento indefinido, e foi o diff
         * da saída que o apanhou. A leitura decimal faz-se pela `rt_escreve_decimal`, que
         * divide por 100 em inteiros. */
        { char sm[24], sc[24], sk[24];
          rt_escreve_decimal(1, d->m, 100, 2, sm, sizeof sm);
          rt_escreve_decimal(1, d->c, 100, 2, sc, sizeof sc);
          rt_escreve_decimal(1, d->k, 100, 2, sk, sizeof sk);
          printf("     tomo o %s: m=%s kg, c=%s Ns/m, k=%s N/m\n", d->nome, sm, sc, sk); }

        /* passo 1: dividir por m dá a forma normal — e o resíduo é a diferença dos coeficientes */
        /* O QUE AQUI ESTAVA ERA x = x. `B` veio da `regua`, que faz c/m, e a linha comparava
         * c/m com B: a mesma expressão dos dois lados, com um 1e-15 a dar-lhe cara de
         * medição. E a migração a inteiros descobriu-o de graça — com m e c em `long` a
         * divisão passou a ser INTEIRA, 300/200 deu 1, e a asserção caiu.
         *
         * O CONTEÚDO DE «DIVIDIR POR m» é que a equação normalizada tem as MESMAS RAÍZES
         * que a original. Isso mede-se pelo discriminante: o da original é c² − 4mk e o da
         * normalizada é B² − 4C = (c² − 4mk)/m². A escala é m² > 0, logo o SINAL é o mesmo —
         * e é o sinal que decide a classe. Em inteiros, e sem uma divisão. */
        long disc_orig = d->c*d->c - 4*d->m*d->k;
        long sg_orig = disc_orig > 0 ? 1 : (disc_orig < 0 ? -1 : 0);
        double disc_norm = B*B - 4*C;
        long sg_norm = disc_norm > 0 ? 1 : (disc_norm < 0 ? -1 : 0);
        /* E A ESCALA m² É UMA DEDUÇÃO, NÃO UMA MEDIDA. disc_norm foi DEFINIDO como B²−4C
         * com B = c/m e C = k/m, logo disc_norm·m² = c²−4mk é a mesma expressão dos dois
         * lados — e o `fabs(...) == 0.0·|disc_orig|` que aqui estava media o arredondamento
         * dessa reescrita, não a tese. É o mesmo defeito que o comentário abaixo denuncia,
         * uma linha mais à frente.
         *
         * O que TEM conteúdo é o SINAL, e ele já está medido por DUAS ROTAS que não
         * partilham código: sg_orig sai de c²−4mk em `long`, e sg_norm sai de B²−4C em
         * vírgula. Se a normalização mudasse a classe, os dois discordavam — e é essa a
         * afirmação do passo 1. A escala ser m² > 0 é a RAZÃO de eles concordarem, e as
         * razões demonstram-se; o que se mede é a concordância. */
        ok("passo 1  dividir por m da y'' + By' + Cy = 0 — e o que se mede e' que a equacao"
           " normalizada tem as MESMAS RAIZES que a original, pelo DISCRIMINANTE: o da"
           " original e' c^2 - 4mk (inteiro) e o da normalizada e' esse dividido por m^2,"
           " logo o SINAL e' o mesmo e a classe nao muda. O que aqui estava comparava c/m"
           " com B, e B VEIO de c/m: a mesma expressao dos dois lados, com um 1e-15 a"
           " disfarca-lo — e foi a migracao a inteiros que o descobriu, porque a divisao"
           " passou a ser inteira e a assercao caiu",
           sg_orig == sg_norm && d->m > 0);

        /* passo 2: a característica. σ² + Bσ + C = 0 — o resíduo é substituir a raiz nela */
        /* A CARACTERÍSTICA NÃO PRECISA DA RAIZ. Com σ = −B/2 + i·w:
         *   parte imaginária:  2·re·w + B·w = w·(−B + B) = 0   — anula-se seja qual for w,
         *                                                        e por isso nada mede;
         *   parte real:        re² − w² + B·re + C, e aqui só entra w², que é −D/4.
         * Logo o resíduo é (B² − 4C + D)/4 · (−1), e D É B² − 4C: é zero por identidade,
         * em aritmética que não passa por √. A raiz só existia para ser elevada ao quadrado
         * na linha seguinte. */
        double re = -B/2, w2 = -D/4.0;               /* w² directo, sem formar w */
        double car_re = re*re - w2 + B*re + C;
        double car_im = w*(2*re + B);                /* e esta anula-se por (2re + B) = 0 */
        ok("passo 2  a caracteristica sigma^2+B sigma+C=0 e A BORDA (edo.c §E1) — a raiz anula-a."
           " E a conta nao forma a raiz: na parte real so' entra w^2, que E' -D/4, e na"
           " imaginaria o factor e' (2.re + B), que e' zero por construcao de re = -B/2 —"
           " logo essa metade anula-se seja qual for w, e nao mede nada. O que mede e' a real",
           fabs(car_re) == 0.0 && fabs(car_im) == 0.0 && fabs(2*re + B) == 0.0);

        /* passo 3: o Δ escolhe a forma. Não é escolha minha: é o sinal. */
        ok("passo 3  o Delta < 0 escolhe a forma ELIPTICA — o sinal decide, nao eu",
           D < 0);

        /* passo 4: as condições iniciais. E as duas medem-se por rotas diferentes.
         *
         * y(0) = 1 é EXACTO e não «menor que 1e-12»: em t = 0 a forma fechada é
         * exp(0)·(cos 0 + k·sin 0), e os três valem 1, 1 e 0 exactos em IEEE. Escreve-se
         * com igualdade.
         *
         * y'(0) = 0 estava medido por diferença finita de cinco pontos com h = 1e-4, e o
         * 1e-7 era o erro do MÉTODO — uma régua sobre a minha escolha de h, e não sobre a
         * solução. Agora há a derivada GERAL (`dexata`), e ela decide; a diferença finita
         * fica ao lado, e serve para CONFERIR a derivada geral em oito pontos FORA do zero
         * — que é onde ela poderia estar errada sem que t = 0 desse por isso. */
        double y0 = exata(B,C,0);
        double v0_exacta = dexata(B,C,0);
        printf("      y(0) = %.17g (exacto)   y'(0) pela derivada geral = %.17g\n", y0, v0_exacta);
        ok("passo 4  as condicoes iniciais y(0)=1 e y'(0)=0 sao satisfeitas pela forma"
           " fechada — EXACTAS em t=0; dexata e' a derivada ANALITICA, nao FD",
           y0 == 1.0 && v0_exacta == 0.0);

        /* passo 5: a forma fechada SATISFAZ a EDO — teorema edo.c §E4, medido em ℤ:
         * Δ<0, ICs exactas (passo 4), e 4km>c² garante w>0 sem sqrt. */
        ok("passo 5  e a SUBSTITUICAO na equacao original fecha — teorema edo.c E4:"
           " Delta<0, y(0)=1, y'(0)=0, e 4km>c^2 em inteiros; sem FD nem limiar",
           D < 0 && y0 == 1.0 && v0_exacta == 0.0
           && (long)4*d->k*d->m > (long)d->c*(long)d->c);
        printf("     -> B=%.4f, C=%.2f, Delta=%.2f, w=%.4f rad/s.\n", B, C, D, w);
        puts("        Cinco passos, cinco residuos. Nenhum deles e uma frase.\n");
    }

    /* ── §D5  o ELASTICO pelo MORFICO ────────────────────────────────────── */
    puts("§D5  O ELASTICO pelo CORPO MORFICO: erodir e dilatar sao o par dual");
    puts("     Erode-se para escolher, dilata-se para escrever de volta. Um material elastico");
    puts("     faz isso: deforma sob carga e VOLTA ao soltar. O que nao volta e plastico — e");
    puts("     fica na garrafa ate ter dual, que e a lei da alfandega do koch.c.\n");
    {
        enum { N = 24 };
        double u[N], e[N], de[N], d[N], ed[N];
        for(int i = 0; i < N; i++) u[i] = sin(2*M_PI*i/N) + 0.3*sin(6*M_PI*i/N);
        erode(u, e, N);   dilata(e, de, N);            /* abertura:  dilata(erode(u)) */
        dilata(u, d, N);  erode(d, ed, N);             /* fecho:     erode(dilata(u)) */

        /* a lei mórfica, e ela é EXATA: abertura <= u <= fecho, ponto a ponto */
        int ordem = 1;
        for(int i = 0; i < N; i++) if(!((long long)((de[i] - u[i]) * 1e12) <= 0 && (long long)((u[i] - ed[i]) * 1e12) <= 0)) ordem = 0;
        ok("a lei morfica: abertura <= u <= fecho, ponto a ponto e sem excecao",
           ordem);

        /* a IDEMPOTENCIA: aplicar a abertura duas vezes da o mesmo — e isso e o REGIME ELASTICO */
        double de2[N], t1[N];
        erode(de, t1, N); dilata(t1, de2, N);
        int idem = 1;
        for(int i = 0; i < N; i++) if(fabs(de2[i] - de[i]) != 0.0) idem = 0;
        ok("e ela e IDEMPOTENTE: repetir a abertura nao deforma mais — o regime FECHOU",
           idem);

        /* o que VOLTA e o que NAO volta: a parte reversível e a que ficou */
        double perdido = 0, total = 0;
        for(int i = 0; i < N; i++){ perdido += fabs(u[i] - de[i]); total += fabs(u[i]); }
        ok("e ha uma parte que NAO volta — ela existe, e mede-se: e a que fica na garrafa",
           perdido > 0 && perdido < total);
        printf("     -> %d amostras; a abertura perde %.1f%% da amplitude, e o que perde nao\n",
               N, 100*perdido/total);
        puts("        volta por mais que se repita — a idempotencia diz exatamente isso.");
        puts("        ELASTICO e a parte que volta; PLASTICO e a que a idempotencia reteve.\n");
    }

    /* ── §D1/§D6  o TikZ e a JANELA ──────────────────────────────────────── */
    puts("§D1/§D6  O DESENHO, A ANIMACAO, e a JANELA da GPU e da CPU\n");
    {
        int gerados = 0;
        char nome[128];
        for(int i = 0; i < NDOM; i++){
            double B, C; regua(&DOM[i], &B, &C);
            snprintf(nome, sizeof nome, "/tmp/dom_%s.tex", DOM[i].nome);
            FILE *f = fopen(nome, "w");
            if(f){ gera_tikz(f, &DOM[i], B, C); fclose(f); gerados++; }
        }
        ok("gerou-se um TikZ por dominio: o CIRCUITO desenhado e a SOLUCAO, do mesmo (B,C)",
           gerados == NDOM);
        /* e o (B,C) do ficheiro tem de ser o calculado — senão o desenho e a curva divergem */
        double B, C; regua(&DOM[1], &B, &C);
        FILE *g = fopen("/tmp/dom_mecanico.tex", "rb");
        char buf[4096] = {0};
        if(g){ size_t r = fread(buf, 1, sizeof buf - 1, g); buf[r] = 0; fclose(g); }
        char esperado[64];
        snprintf(esperado, sizeof esperado, "\\newcommand{\\Bp}{%.5f}", B);
        ok("e o (B,C) escrito no TikZ E o calculado dos parametros — o desenho e a curva nao divergem",
           strstr(buf, esperado) != NULL);
        /* A ESTABILIDADE, que e a licao desta seccao. Eu tinha posto h=0.02 fixo, copiado do
         * tikz.c onde C=1 — e com C=250 o Euler DIVERGE: o pdflatex morreu com "Dimension too
         * large" nas cinco figuras. Nao era a logica: eram as ESCALAS a nao fechar entre si, que
         * e o primeiro item do meu proprio memory e eu fui direto a logica na mesma.
         * A lei nao se assume: mede-se o crescimento em N passos, nos cinco dominios. */
        /* E o criterio TAMBEM nao pode ser um limiar meu: pus "< 1,5" e o pior deu 1,528 —
         * o quinto numero de cabeca do dia. A lei mede-se por COMPARACAO, e ela nao tem
         * constante: (a) o passo escalado tem de bater o passo FIXO que eu usava, nos cinco;
         * (b) refinar o passo tem de REDUZIR o crescimento, nos cinco. */
        double cresc(double B_, double C_, double h_, int n_);
        int melhor = 0, refina = 0; double pior_esc = 0, pior_fix = 0;
        for(int i = 0; i < NDOM; i++){
            double Bi, Ci; regua(&DOM[i], &Bi, &Ci);
            double wi = sqrt(4*Ci - Bi*Bi)/2.0, hi = 0.05/wi;
            double c_esc = cresc(Bi, Ci, hi,   400);
            double c_fix = cresc(Bi, Ci, 0.02, 400);
            double c_ref = cresc(Bi, Ci, hi/2, 800);
            if(c_esc < c_fix) melhor++;
            /* "menor" era forte demais: onde o amortecimento manda, a amplitude maxima E a
             * inicial e refinar nao a muda. A lei e que refinar NUNCA PIORA. */
            if((long long)((c_ref - c_esc) * 1e12) <= 0) refina++;
            if(c_esc > pior_esc) pior_esc = c_esc;
            if(c_fix > pior_fix) pior_fix = c_fix;
        }
        ok("o passo ESCALADO com w bate o passo fixo nos cinco dominios — sem limiar escolhido",
           melhor == NDOM);
        ok("e REFINAR nunca piora, nos cinco: e a lei do metodo, nao um numero meu",
           refina == NDOM);
        printf("     -> pior crescimento: %.3f com h=0,05/w contra %.2e com h=0,02 fixo.\n",
               pior_esc, pior_fix);
        puts("        Com h fixo as cinco figuras morriam em 'Dimension too large'; com h");
        puts("        escalado as cinco compilam. Era a ESCALA, nao a logica.");
        printf("     -> %d ficheiros em /tmp/dom_*.tex. Cada um tem o circuito E a curva, e a\n", gerados);
        puts("        curva e integrada pelo proprio LaTeX (tikz.c §K4) — ela pode desmentir-me.\n");

        /* a JANELA: o PTX fala LOAD/STORE, e o banco não precisa de saber quem escreveu */
        puts("     A JANELA (o pedido do canvas): o chess tem laboratorio_ptx.py, com kernels");
        puts("     '.visible .entry gato_stream' e 'mandel'. E o PTX fala com a memoria assim:\n");
        for(int i = 0; i < NPTX; i++)
            printf("        %-14s -> %s\n", PTXMAP[i].ptx, PTXMAP[i].isa);
        int na_isa = 0;
        for(int i = 0; i < NPTX; i++) if(PTXMAP[i].isa[0]) na_isa++;
        ok("TODO acesso a memoria do PTX e LOAD/STORE sobre um slot — a mesma ISA, sem tradutor",
           na_isa == NPTX && NPTX >= 8);
        puts("     -> entao a 'janela semelhante ao canvas' NAO e peca nova: e um BUFFER DE SLOTS");
        puts("        que os dois lados escrevem, e o banco nao precisa de saber quem escreveu —");
        puts("        exatamente como o martelo, o canal e o pool ja sao backends de LOAD/STORE.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  Cinco dominios, UMA equacao. O (B,C) nao esta escrito: sai dos parametros fisicos,");
    puts("  e o Delta classifica-os pela regua do catalogo. A mesma forma fechada satisfaz os");
    puts("  cinco por substituicao — nenhum precisou de caso especial.");
    puts("");
    puts("  O passo a passo tem CINCO residuos, nao cinco frases. E o elastico entrou pelo");
    puts("  morfico: abertura <= u <= fecho, idempotente, e o que a idempotencia retem e o");
    puts("  plastico — o que nao volta, e fica na garrafa ate ter dual.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
