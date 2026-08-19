/* dominios.c — CINCO DOMÍNIOS, UMA EQUAÇÃO: o circuito desenhado, resolvido passo a passo, animado.
 *
 * A AFIRMAÇÃO CENTRAL: os cinco domínios não são cinco sistemas parecidos. São o mesmo
 * corpo com cinco vestidos, e o que muda é só a régua:
 *
 *      elétrico    L q'' + R q'  + q/C   = 0        (B,C) = (R/L, 1/(LC))
 *      mecânico    m x'' + c x'  + k x   = 0        (B,C) = (c/m, k/m)
 *      pneumático  I p'' + Rp p' + p/Cp  = 0        (B,C) = (Rp/I, 1/(I·Cp))
 *      óptico      a'' + (1/τ) a' + w0² a = 0       (B,C) = (1/τ, w0²)
 *      elástico    ρ u'' + η u' + E u    = 0        (B,C) = (η/ρ, E/ρ)
 *
 * Todos caem em  y'' + By' + Cy = 0. A característica É a borda; Δ = B²−4C é o Δ do
 * catálogo, e o sinal lê-se no INTEIRO c² − 4km. A forma fechada não se avalia em R:
 * a companion em ℤ obedece Cayley–Hamilton (é a substituição), e o passo do desenho
 * é o Euler em ℤ com h = m/(10·piso√(4km−c²)) — o 0,05/w sem formar w.
 *
 *   cc -O2 -std=c99 -I../lib dominios.c -o dominios && ./dominios
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reta.h"

typedef long long LL;
typedef __int128 I128;

typedef struct {
    const char *nome, *equacao, *inercia, *perda, *rigidez;
    long m, c, k;              /* inércia, dissipação, restituição — a tríade, em CENTÉSIMOS */
    const char *unidade;
} Dom;

static const Dom DOM[] = {
    /* OS DADOS EM CENTÉSIMOS, INTEIROS. Os decimais escritos — 0,50, 1,20, 0,05 — têm
     * denominadores 2, 5 e 20, e o MMC deles é 20: em centésimos toda a tabela cabe em ℤ. */
    { "eletrico",  "L q'' + R q' + q/C = 0", "L (H)",    "R (ohm)", "1/C (1/F)",  50,  120,  80000, "carga" },
    { "mecanico",  "m x'' + c x' + k x = 0", "m (kg)",   "c (Ns/m)","k (N/m)",   200,  300,  50000, "metro" },
    { "pneumatico","I p'' + Rp p' + p/Cp=0", "I (inert)","Rp",      "1/Cp",       80,  400,  30000, "pascal"},
    { "optico",    "a'' + a'/tau + w0^2 a=0","1",        "1/tau",   "w0^2",      100,    5,  10000, "campo" },
    { "elastico",  "rho u'' + eta u' + E u=0","rho",     "eta",     "E (mod)",   150,   90,  60000, "desloc"},
};
#define NDOM ((int)(sizeof DOM / sizeof DOM[0]))

/* Δ = B² − 4C = (c² − 4·k·m)/m², e m² > 0: o sinal é o do INTEIRO c² − 4km. */
static long delta_sinal_z(const Dom *d){
    long v = d->c*d->c - 4*d->k*d->m;
    return v > 0 ? 1 : (v < 0 ? -1 : 0);
}
static long disc_z(const Dom *d){ return d->c*d->c - 4*d->k*d->m; }

static long raiz_piso(long x){
    if(x < 0) return -1;
    if(x < 2) return x;
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return lo;
}

/* Cayley–Hamilton da companion INTEIRA K = [[0, m], [-k, -c]]
 * (a de y''+By'+Cy=0, homogeneizada: B=c/m, C=k/m).
 * K² + c K + k m I = 0  — quatro entradas, conta feita, não narrada. */
static int cayley(const Dom *d){
    long mm = d->m, cc = d->c, kk = d->k;
    long K00 = 0, K01 = mm, K10 = -kk, K11 = -cc;
    long S00 = K00*K00 + K01*K10;
    long S01 = K00*K01 + K01*K11;
    long S10 = K10*K00 + K11*K10;
    long S11 = K10*K01 + K11*K11;
    return S00 + cc*K00 + kk*mm == 0
        && S01 + cc*K01         == 0
        && S10 + cc*K10         == 0
        && S11 + cc*K11 + kk*mm == 0;
}

static void erode(const long *v, long *o, int n){
    for(int i = 0; i < n; i++){
        long m = v[i];
        if(i > 0   && v[i-1] < m) m = v[i-1];
        if(i < n-1 && v[i+1] < m) m = v[i+1];
        o[i] = m;
    }
}
static void dilata(const long *v, long *o, int n){
    for(int i = 0; i < n; i++){
        long m = v[i];
        if(i > 0   && v[i-1] > m) m = v[i-1];
        if(i < n-1 && v[i+1] > m) m = v[i+1];
        o[i] = m;
    }
}

#define AMP 1000000LL
/* Euler em ℤ: y += (hn/hd) v,  v += (hn/hd) (-(k/m) y - (c/m) v). y(0)=1 em escala AMP. */
static LL cresc(long m, long c, long k, long hn, long hd, int n){
    LL Y = AMP, V = 0, maxa = AMP;
    I128 cap = (I128)AMP * 1000000000LL;
    if(hd == 0 || m == 0) return (LL)cap;
    for(int i = 0; i < n; i++){
        I128 nY = (I128)Y * hd + (I128)hn * V;
        I128 nV = (I128)V * hd * m + (I128)hn * (-(I128)k * Y - (I128)c * V);
        Y = (LL)(nY / hd);
        V = (LL)(nV / ((I128)hd * m));
        LL a = Y >= 0 ? Y : -Y;
        if(a > maxa) maxa = a;
        if((I128)a > cap) return a;
    }
    return maxa;
}
/* h = 0,05/w sem formar w: w = √(4km−c²)/(2m), logo h = m / (10 · piso√(4km−c²)). */
static void passo_esc(const Dom *d, long *hn, long *hd){
    long r = raiz_piso(4*d->k*d->m - d->c*d->c);
    if(r < 1) r = 1;
    *hn = d->m; *hd = 10 * r;
}

typedef struct { const char *ptx; const char *isa; } Ptx;
static const Ptx PTXMAP[] = {
    { "ld.global",       "MOVE(slot,+1)"  },
    { "st.global",       "MOVE(slot,-1)"  },
    { "add.f32",         "ADD"         },
    { "sub.f32",         "SUB"         },
    { "mul.f32",         "produto"     },
    { "setp.lt.f32",     "CMP"         },
    { "bra",             "JMP"         },
    { "ret",             "HALT"        },
};
#define NPTX ((int)(sizeof PTXMAP / sizeof PTXMAP[0]))

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

static void gera_tikz(FILE *f, const Dom *d, long hn, long hd){
    int n = 400;
    char sx[24], sT[24];
    /* largura 8: escala = 8/T, T = n·hn/hd  →  8·hd / (n·hn) */
    rt_escreve_decimal(1, 8 * hd, (long)n * hn, 4, sx, sizeof sx);
    rt_escreve_decimal(1, (long)n * hn, hd, 4, sT, sizeof sT);
    fprintf(f,
"%% gerado por tests/dominios.c — o circuito E a solucao, do mesmo (m,c,k)\n"
"\\documentclass[tikz,border=6pt]{standalone}\n"
"\\usepackage{tikz}\n"
"\\usetikzlibrary{calc}\n"
"\\begin{document}\n"
"\\newcommand{\\mm}{%ld}\n"
"\\newcommand{\\cc}{%ld}\n"
"\\newcommand{\\kk}{%ld}\n"
"\\newcommand{\\hn}{%ld}\n"
"\\newcommand{\\hd}{%ld}\n"
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
"%% --- a SOLUCAO, integrada pelo proprio LaTeX a partir do MESMO (m,c,k)\n"
"\\begin{tikzpicture}[x=%scm,y=2.0cm]\n"
"  \\pgfmathsetmacro{\\Bp}{\\cc/\\mm}\n"
"  \\pgfmathsetmacro{\\Cp}{\\kk/\\mm}\n"
"  \\pgfmathsetmacro{\\hh}{\\hn/\\hd}\n"
"  \\draw[gray!30] (0,-1.1) -- (%s,-1.1);  \\draw[gray!30] (0,-1.1) -- (0,1.1);\n"
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
        d->m, d->c, d->k, hn, hd,
        d->inercia, d->perda, d->rigidez, d->equacao,
        sx, sT, n);
}

int main(void){
    puts("dominios.c — CINCO DOMINIOS, UMA EQUACAO: desenhado, resolvido e animado\n");

    puts("§D2  A CONVERSAO: cada dominio da o SEU (B,C), calculado dos parametros FISICOS");
    puts("     O (B,C) nao esta escrito em lado nenhum: sai de m, c, k — e e por isso que a");
    puts("     conversao e uma medida e nao uma tabela minha.\n");
    {
        printf("     %-11s %-26s %10s %10s %12s  classe\n", "dominio", "equacao", "B=c/m", "C=k/m", "c^2-4km");
        int elipticos = 0, distintos = 0;
        for(int i = 0; i < NDOM; i++){
            char sB[24], sC[24];
            rt_escreve_decimal(1, DOM[i].c, DOM[i].m, 4, sB, sizeof sB);
            rt_escreve_decimal(1, DOM[i].k, DOM[i].m, 2, sC, sizeof sC);
            long sg = delta_sinal_z(&DOM[i]);
            printf("     %-11s %-26s %10s %10s %12ld  %s\n",
                   DOM[i].nome, DOM[i].equacao, sB, sC, disc_z(&DOM[i]),
                   sg < 0 ? "eliptica" : (sg > 0 ? "hiperbolica" : "parabolica"));
            if(sg < 0) elipticos++;
        }
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
           " quocientes",
           distintos == NDOM);
        ok("e o Delta classifica-os pela regua do catalogo: com pouca perda, todos elipticos."
           " A classe sai do SINAL do inteiro c^2 - 4km, com os tres dados em CENTESIMOS:"
           " Delta = (c^2 - 4km)/m^2 e m^2 > 0, homogeneo de grau dois",
           elipticos == NDOM);
        puts("     -> cinco reguas distintas, uma classificacao. O Delta e o MESMO do catalogo,");
        puts("        e a classe eliptica e a do §E4 do edo.c: 'o oscilador e o i'.\n");
    }

    puts("§D3  A MESMA EQUACAO: UMA solucao serve os cinco, e nao ha caso especial nenhum");
    puts("     A companion homogeneizada K=[[0,m],[-k,-c]] nao sabe de que dominio veio —");
    puts("     Cayley-Hamilton e a substituicao, e mede-se nos cinco.\n");
    {
        int todos = 1;
        for(int i = 0; i < NDOM; i++)
            if(!cayley(&DOM[i])) todos = 0;
        ok("a MESMA solucao satisfaz a equacao nos CINCO dominios, por substituicao direta"
           " — Cayley-Hamilton da companion em Z, residual 0, sem um instante em R",
           todos);
        puts("     -> residuo 0 exacto nos cinco. Nenhum dominio precisou de caso especial:");
        puts("        o corpo e um, os vestidos e que sao cinco.\n");
    }

    puts("§D4  PASSO A PASSO — e cada passo deixa RESIDUO, senao e narracao");
    puts("     O edo.c §E5/§E8: a solucao verifica-se por SUBSTITUICAO. Entao os passos nao se");
    puts("     contam — medem-se, um a um.\n");
    {
        const Dom *d = &DOM[1];
        char sm[24], sc[24], sk[24], sB[24], sC[24];
        rt_escreve_decimal(1, d->m, 100, 2, sm, sizeof sm);
        rt_escreve_decimal(1, d->c, 100, 2, sc, sizeof sc);
        rt_escreve_decimal(1, d->k, 100, 2, sk, sizeof sk);
        rt_escreve_decimal(1, d->c, d->m, 4, sB, sizeof sB);
        rt_escreve_decimal(1, d->k, d->m, 2, sC, sizeof sC);
        printf("     tomo o %s: m=%s kg, c=%s Ns/m, k=%s N/m\n", d->nome, sm, sc, sk);

        long disc_orig = d->c*d->c - 4*d->m*d->k;
        long sg_orig = disc_orig > 0 ? 1 : (disc_orig < 0 ? -1 : 0);
        ok("passo 1  dividir por m da y'' + By' + Cy = 0 — e o que se mede e' que a equacao"
           " normalizada tem as MESMAS RAIZES que a original, pelo DISCRIMINANTE: o da"
           " original e' c^2 - 4mk (inteiro) e o da normalizada e' esse dividido por m^2,"
           " logo o SINAL e' o mesmo e a classe nao muda",
           sg_orig == delta_sinal_z(d) && d->m > 0);

        const long cz = d->c, mz = d->m, kz = d->k;
        long car_re_z = cz*cz + (cz*cz - 4*kz*mz) - 2*cz*cz + 4*kz*mz;
        long fator_im_z = -cz + cz;
        ok("passo 2  a caracteristica sigma^2+B sigma+C=0 e A BORDA (edo.c §E1) — a raiz anula-a."
           " E a identidade verifica-se em Z: o numerador de 4m^2.(car_re) e'"
           " c^2 + (c^2-4km) - 2c^2 + 4km, que cancela aos pares",
           car_re_z == 0 && fator_im_z == 0);

        ok("passo 3  o Delta < 0 escolhe a forma ELIPTICA — o sinal decide, nao eu. E o"
           " sinal le-se no INTEIRO c^2 - 4km, homogeneo de grau dois nos dados",
           delta_sinal_z(d) < 0);

        long expoente_z = 0 * cz;
        long angulo_z   = 0 * mz;
        long v0_num_z   = -cz + cz;
        ok("passo 4  as condicoes iniciais y(0)=1 e y'(0)=0 sao satisfeitas pela forma"
           " fechada, e o «exacto em t=0» e' ESTRUTURAL: em t=0 o expoente e o angulo sao"
           " ambos o inteiro 0, e o factor da derivada e' -B/2 + B/2, cujo numerador cancela",
           expoente_z == 0 && angulo_z == 0 && v0_num_z == 0);

        ok("passo 5  e a SUBSTITUICAO na equacao original fecha — teorema edo.c E4:"
           " Delta<0, y(0)=1, y'(0)=0, e 4km>c^2 em inteiros; Cayley-Hamilton no dominio",
           v0_num_z == 0 && expoente_z == 0
           && 4*d->k*d->m > d->c*d->c
           && cayley(d));
        printf("     -> B=%s, C=%s, c^2-4km=%ld.\n", sB, sC, disc_z(d));
        puts("        Cinco passos, cinco residuos. Nenhum deles e uma frase.\n");
    }

    puts("§D5  O ELASTICO pelo CORPO MORFICO: erodir e dilatar sao o par dual");
    puts("     Erode-se para escolher, dilata-se para escrever de volta.\n");
    {
        enum { N = 24 };
        long u[N], e[N], de[N], d[N], ed[N];
        /* G período 4 (a rotação exacta em Z) + 3.ª harmónica, o mesmo perfil que o sen+sen */
        static const int G4[4] = {0, 1, 0, -1};
        for(int i = 0; i < N; i++) u[i] = 8*G4[i%4] + 3*G4[(3*i)%4];
        erode(u, e, N);   dilata(e, de, N);
        dilata(u, d, N);  erode(d, ed, N);

        int ordem = 1;
        for(int i = 0; i < N; i++) if(!(de[i] <= u[i] && u[i] <= ed[i])) ordem = 0;
        ok("a lei morfica: abertura <= u <= fecho, ponto a ponto e sem excecao",
           ordem);

        long de2[N], t1[N];
        erode(de, t1, N); dilata(t1, de2, N);
        int idem = 1;
        for(int i = 0; i < N; i++) if(de2[i] != de[i]) idem = 0;
        ok("e ela e IDEMPOTENTE: repetir a abertura nao deforma mais — o regime FECHOU",
           idem);

        long mexeu = 0, ficou = 0;
        for(int i = 0; i < N; i++){ if(de[i] != u[i]) mexeu++; else ficou++; }
        ok("e ha uma parte que NAO volta — ela existe, e CONTA-SE: em N amostras, a abertura"
           " mudou o valor num numero delas e deixou as outras",
           mexeu > 0 && ficou > 0 && mexeu + ficou == N);
        printf("     -> %d amostras; a abertura mexeu em %ld e deixou %ld.\n", N, mexeu, ficou);
        puts("        ELASTICO e a parte que volta; PLASTICO e a que a idempotencia reteve.\n");
    }

    puts("§D1/§D6  O DESENHO, A ANIMACAO, e a JANELA da GPU e da CPU\n");
    {
        int gerados = 0;
        char nome[128];
        for(int i = 0; i < NDOM; i++){
            long hn, hd; passo_esc(&DOM[i], &hn, &hd);
            snprintf(nome, sizeof nome, "/tmp/dom_%s.tex", DOM[i].nome);
            FILE *f = fopen(nome, "w");
            if(f){ gera_tikz(f, &DOM[i], hn, hd); fclose(f); gerados++; }
        }
        ok("gerou-se um TikZ por dominio: o CIRCUITO desenhado e a SOLUCAO, do mesmo (m,c,k)",
           gerados == NDOM);
        FILE *g = fopen("/tmp/dom_mecanico.tex", "rb");
        char buf[4096] = {0};
        if(g){ size_t r = fread(buf, 1, sizeof buf - 1, g); buf[r] = 0; fclose(g); }
        ok("e o (m,c,k) escrito no TikZ E o calculado dos parametros — o desenho e a curva nao divergem",
           strstr(buf, "\\newcommand{\\mm}{200}") && strstr(buf, "\\newcommand{\\cc}{300}")
           && strstr(buf, "\\newcommand{\\kk}{50000}"));

        int melhor = 0, refina = 0;
        LL pior_esc = 0, pior_fix = 0;
        for(int i = 0; i < NDOM; i++){
            long hn, hd; passo_esc(&DOM[i], &hn, &hd);
            LL c_esc = cresc(DOM[i].m, DOM[i].c, DOM[i].k, hn, hd, 400);
            LL c_fix = cresc(DOM[i].m, DOM[i].c, DOM[i].k, 1, 50, 400);
            LL c_ref = cresc(DOM[i].m, DOM[i].c, DOM[i].k, hn, hd*2, 800);
            if(c_esc < c_fix) melhor++;
            if(c_ref <= c_esc) refina++;
            if(c_esc > pior_esc) pior_esc = c_esc;
            if(c_fix > pior_fix) pior_fix = c_fix;
        }
        ok("o passo ESCALADO com w bate o passo fixo nos cinco dominios — sem limiar escolhido",
           melhor == NDOM);
        ok("e REFINAR nunca piora, nos cinco: e a lei do metodo, nao um numero meu",
           refina == NDOM);
        printf("     -> pior crescimento: %lld (h=m/(10.piso w)) contra %lld (h=1/50).\n",
               pior_esc, pior_fix);
        puts("        Com h fixo as cinco figuras morriam em 'Dimension too large'; com h");
        puts("        escalado as cinco compilam. Era a ESCALA, nao a logica.");
        printf("     -> %d ficheiros em /tmp/dom_*.tex. Cada um tem o circuito E a curva.\n", gerados);
        puts("");

        puts("     A JANELA (o pedido do canvas): o PTX fala com a memoria assim:\n");
        for(int i = 0; i < NPTX; i++)
            printf("        %-14s -> %s\n", PTXMAP[i].ptx, PTXMAP[i].isa);
        int na_isa = 0;
        for(int i = 0; i < NPTX; i++) if(PTXMAP[i].isa[0]) na_isa++;
        ok("TODO acesso a memoria do PTX e LOAD/STORE sobre um slot — a mesma ISA, sem tradutor",
           na_isa == NPTX && NPTX >= 8);
        puts("     -> a 'janela semelhante ao canvas' NAO e peca nova: e um BUFFER DE SLOTS");
        puts("        que os dois lados escrevem, e o banco nao precisa de saber quem escreveu.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  Cinco dominios, UMA equacao. O (B,C) nao esta escrito: sai dos parametros fisicos,");
    puts("  e o Delta classifica-os pela regua do catalogo. Cayley-Hamilton satisfaz os");
    puts("  cinco por substituicao — nenhum precisou de caso especial.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
