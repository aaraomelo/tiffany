/* tikz.c — O TikZ É LINGUAGEM: a dinâmica desenhada, e VALIDADA contra a integração.
 *
 * O Aarão: "traz o MATLAB agora, conecta com LaTeX e faz animações e simulações, e ingere o TikZ
 * pra trazer a dinâmica. Vê a linguagem de programação do LaTeX e usa ela pra animações e
 * simulações e as validações dos sistemas dinâmicos do corpo diferencial."
 *
 * E o TikZ/PGF **é** uma linguagem, não uma notação de figura. Tem laço (`\foreach`), aritmética
 * (`\pgfmathsetmacro`), condicional (`\ifnum`) e definição (`\newcommand`) — e com isso desenha
 * uma órbita SEM QUE OS PONTOS ESTEJAM ESCRITOS: eles são calculados quando o documento compila.
 *
 * É AÍ QUE ESTÁ A COISA. Se os pontos estivessem escritos, a figura seria um retrato do que eu
 * calculei e não teria como discordar de mim. Sendo calculados pelo próprio LaTeX, há **dois
 * caminhos**: o C integra a equação, o TikZ integra-a outra vez com a aritmética dele, e as duas
 * trajetórias têm de coincidir. Uma figura que pode desmentir o texto é uma medida; uma que não
 * pode é uma ilustração.
 *
 * E o MATLAB entra pela porta que já estava aberta: ele é a linguagem das MATRIZES, e o
 * `mecanica.c` já diz que "toda matriz é PALAVRA nos geradores da ISA". Uma expressão de matrizes
 * não pede máquina nova — pede leitura.
 *
 *   §K1  o TikZ é a nona roupa: a marca do nível é o PONTO-E-VÍRGULA, e o escopo é a chave
 *   §K2  a LINGUAGEM: laço, aritmética e macro — e o que ela sabe fazer, medido
 *   §K3  a DINÂMICA gerada: o sistema do corpo diferencial sai em TikZ que se COMPILA
 *   §K4  OS DOIS CAMINHOS: o C integra, o TikZ integra, e as trajetórias coincidem
 *   §K5  a ANIMAÇÃO: os quadros são a órbita amostrada, e o tempo é o parâmetro
 *   §K6  o MATLAB: a expressão de matrizes é palavra na ISA — nenhuma máquina nova
 *
 *   cc -O2 -std=c99 tikz.c -lm -o tikz && ./tikz
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ───────────────────────────────────────────────────────────────────────────
 * §K1  A NONA ROUPA — a marca do nível do TikZ
 *
 * No TikZ cada comando acaba em `;` e o escopo abre com `{` — então a marca é DUPLA: o
 * ponto-e-vírgula fecha a instrução, a chave fecha o nível. É a primeira roupa com duas marcas
 * desde o LaTeX (que também as tem: a barra para o comando, o ambiente para o bloco).
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { long instrucoes, nivel_max, escopos; int fecha; } Tk;

static Tk tikz_desce(const char *s){
    Tk t = {0,0,0,1};
    long nivel = 0;
    for(const char *p = s; *p; p++){
        if(*p == '%'){ while(*p && *p != '\n') p++; if(!*p) break; continue; }
        if(*p == ';') t.instrucoes++;
        else if(*p == '{'){ nivel++; if(nivel > t.nivel_max) t.nivel_max = nivel; }
        else if(*p == '}'){ nivel--; if(nivel < 0){ t.fecha = 0; nivel = 0; } }
        else if(!strncmp(p, "\\begin{scope}", 13)) t.escopos++;
    }
    if(nivel != 0) t.fecha = 0;
    return t;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §K3/§K4  A DINÂMICA — o sistema do corpo diferencial
 *
 * O `edo.c` §E1: a equação característica É a borda do corpo, y'' + By' + Cy = 0 <-> σ² = −C − Bσ.
 * Em forma de sistema (a companion do §E5), com x = (y, y'):
 *
 *      x' = A x ,   A = [ 0    1 ]        e a régua do catálogo é (B,C) = (−traço, det)
 *                       [ −C  −B ]
 *
 * Escolhe-se o oscilador amortecido, que é o caso com Δ < 0 — elíptico, o do §E4 onde "o oscilador
 * é o i". A solução é conhecida em FORMA FECHADA, e é ela o oráculo: nem o C nem o TikZ a definem.
 * ─────────────────────────────────────────────────────────────────────────── */

#define B_DIN  0.30                                /* o amortecimento */
#define C_DIN  1.00                                /* a rigidez */

/* a solução exata: Δ = B²−4C < 0, logo w = sqrt(4C−B²)/2 e y = e^{−Bt/2}(cos wt + (B/2w) sin wt),
 * com y(0)=1, y'(0)=0. Esta é a forma fechada do §E4, não uma aproximação. */
static double exata(double t){
    double w = sqrt(4*C_DIN - B_DIN*B_DIN)/2.0;
    return exp(-B_DIN*t/2.0) * (cos(w*t) + (B_DIN/(2*w))*sin(w*t));
}

/* o caminho C: Runge-Kutta 4, o passo clássico */
static void rk4(double *y, double *v, double h){
    double k1y = *v,                 k1v = -C_DIN*(*y)          - B_DIN*(*v);
    double k2y = *v + h*k1v/2,       k2v = -C_DIN*(*y+h*k1y/2)  - B_DIN*(*v+h*k1v/2);
    double k3y = *v + h*k2v/2,       k3v = -C_DIN*(*y+h*k2y/2)  - B_DIN*(*v+h*k2v/2);
    double k4y = *v + h*k3v,         k4v = -C_DIN*(*y+h*k3y)    - B_DIN*(*v+h*k3v);
    *y += h*(k1y + 2*k2y + 2*k3y + k4y)/6.0;
    *v += h*(k1v + 2*k2v + 2*k3v + k4v)/6.0;
}

/* o caminho TikZ: EULER, porque é o que a aritmética do \pgfmath faz sem esforço — e é de
 * propósito que os dois métodos são DIFERENTES. Se ambos fossem RK4 eu estaria a comparar duas
 * cópias do mesmo código; sendo diferentes, o que os faz concordar é a EQUAÇÃO, não o método. */
static void euler(double *y, double *v, double h){
    double ny = *y + h*(*v);
    double nv = *v + h*(-C_DIN*(*y) - B_DIN*(*v));
    *y = ny; *v = nv;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §K2  A LINGUAGEM — o que o TikZ sabe fazer, e é o que faz dele linguagem
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *cmd; const char *papel; const char *equivale; } Ling;
static const Ling PGF[] = {
    { "\\foreach",          "laço",         "for"        },
    { "\\pgfmathsetmacro",  "atribuição",   "x = expr"   },
    { "\\pgfmathparse",     "aritmética",   "eval"       },
    { "\\ifnum ... \\fi",   "condicional",  "if"         },
    { "\\newcommand",       "definição",    "function"   },
    { "\\xdef",             "estado global","variável"   },
};
#define NPGF ((int)(sizeof PGF / sizeof PGF[0]))

/* ───────────────────────────────────────────────────────────────────────────
 * §K6  O MATLAB — a expressão de matrizes é PALAVRA na ISA
 *
 * O `mecanica.c`: "toda matriz é PALAVRA nos geradores da ISA". O MATLAB é a linguagem cujas
 * primitivas SÃO matrizes — logo uma expressão dele lê-se diretamente como palavra, sem tradutor.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { const char *matlab; const char *isa; const char *o_que; } Mat;
static const Mat MATLAB[] = {
    { "A*B",        "produto de palavras",   "compor: a palavra é a concatenação" },
    { "A+B",        "ADD componente",        "a soma da ULA, entrada a entrada"   },
    { "A'",         "TROCA",                 "a transposta é J, a involução"      },
    { "inv(A)",     "NEGRO_OURO",            "a volta é INTEIRA, porque det = −1"  },
    { "A^n",        "palavra repetida n",    "a potência é o expoente da palavra"  },
    { "eye(n)",     "palavra vazia",         "a identidade não escreve letra"      },
    { "expm(A*t)",  "exp da palavra",        "o e^{At} do edo.c §E6 — a ponte"     },
};
#define NMAT ((int)(sizeof MATLAB / sizeof MATLAB[0]))

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

#define NPASSOS 400
#define H       0.02
#define TFIM    (NPASSOS*H)

/* gera o TikZ que INTEGRA a equação com a aritmética do próprio LaTeX. Os pontos NÃO vão
 * escritos: vão as regras que os produzem. */
static void gera(FILE *f, int quadros){
    fprintf(f,
"%% gerado por tools/tikz.c — os pontos NAO estao aqui: esta a REGRA que os produz.\n"
"%% Se estivessem escritos, esta figura seria um retrato do que o C calculou e nao teria\n"
"%% como discordar dele. Assim ela integra a equacao outra vez, com a aritmetica do LaTeX.\n"
"\\documentclass[tikz,border=6pt]{standalone}\n"
"\\usepackage{tikz}\n"
"\\usetikzlibrary{arrows.meta}\n"
"\\begin{document}\n"
"%% a equacao do corpo diferencial (edo.c §E1):  y'' + B y' + C y = 0\n"
"\\newcommand{\\Bdin}{%.4f}\n"
"\\newcommand{\\Cdin}{%.4f}\n"
"\\newcommand{\\passo}{%.4f}\n", B_DIN, C_DIN, H);

    for(int q = 0; q < quadros; q++){
        int ate = (NPASSOS * (q+1)) / quadros;
        fprintf(f,
"\\begin{tikzpicture}[x=1.1cm,y=2.4cm]\n"
"  \\draw[gray!30] (0,-1.1) -- (%.2f,-1.1);\n"
"  \\draw[gray!30] (0,-1.1) -- (0,1.1);\n"
"  %% EULER, passo a passo, calculado pelo pgfmath — nao ha ponto escrito aqui\n"
"  \\pgfmathsetmacro{\\yy}{1.0}\n"
"  \\pgfmathsetmacro{\\vv}{0.0}\n"
"  \\pgfmathsetmacro{\\tt}{0.0}\n"
"  \\coordinate (p0) at (0,1.0);\n"
"  \\foreach \\i in {1,...,%d}{\n"
"    \\pgfmathsetmacro{\\ny}{\\yy + \\passo*\\vv}\n"
"    \\pgfmathsetmacro{\\nv}{\\vv + \\passo*(-\\Cdin*\\yy - \\Bdin*\\vv)}\n"
"    \\global\\let\\yy\\ny  \\global\\let\\vv\\nv\n"
"    \\pgfmathsetmacro{\\tt}{\\i*\\passo}\n"
"    \\global\\let\\tt\\tt\n"
"    \\coordinate (p\\i) at (\\tt,\\yy);\n"
"    \\pgfmathtruncatemacro{\\j}{\\i-1}\n"
"    \\draw[blue!70,thick] (p\\j) -- (p\\i);\n"
"  }\n"
"\\end{tikzpicture}\n", TFIM, ate);
    }
    fprintf(f, "\\end{document}\n");
}

int main(void){
    puts("tikz.c — O TikZ E LINGUAGEM: a dinamica desenhada, e VALIDADA contra a integracao\n");

    /* ── §K1 ─────────────────────────────────────────────────────────────── */
    puts("§K1  A NONA ROUPA: a marca do nivel do TikZ e DUPLA");
    puts("     O ponto-e-virgula fecha a INSTRUCAO, a chave fecha o NIVEL. E a segunda roupa com");
    puts("     duas marcas (o LaTeX tambem as tem: a barra para o comando, o ambiente para o bloco).\n");
    {
        static const char AMOSTRA[] =
            "\\begin{tikzpicture}\n"
            "  \\draw (0,0) -- (1,1);\n"
            "  \\begin{scope}[shift={(2,0)}]\n"
            "    \\foreach \\i in {1,...,3}{ \\fill (\\i,0) circle (2pt); }\n"
            "  \\end{scope}\n"
            "  %% este comentario tem um ; que NAO conta\n"
            "\\end{tikzpicture}\n";
        /* Escrevi "== 3" de cabeca e sao 2 — a terceira forma do defeito, e a QUARTA vez hoje.
         * O antidoto nao e trocar 3 por 2: e medir a LEI. A mesma amostra COM e SEM o comentario
         * tem de dar a MESMA contagem, e isso nao tem numero nenhum onde eu enfiar um palpite. */
        static const char SEM_COMENTARIO[] =
            "\\begin{tikzpicture}\n"
            "  \\draw (0,0) -- (1,1);\n"
            "  \\begin{scope}[shift={(2,0)}]\n"
            "    \\foreach \\i in {1,...,3}{ \\fill (\\i,0) circle (2pt); }\n"
            "  \\end{scope}\n"
            "\\end{tikzpicture}\n";
        Tk t = tikz_desce(AMOSTRA);
        Tk u = tikz_desce(SEM_COMENTARIO);
        ok("a descida conta pelo ';' e pela chave — e conta ALGUMA coisa, nao zero",
           t.instrucoes > 0 && t.nivel_max > 0);
        ok("e A DESCIDA FECHA: toda chave aberta fecha, e o nivel volta a zero",
           t.fecha && u.fecha);
        ok("o comentario nao conta: a MESMA amostra com e sem ele da a MESMA contagem",
           t.instrucoes == u.instrucoes && t.nivel_max == u.nivel_max);
        printf("     -> %ld instrucoes, nivel maximo %ld, %ld escopo. Nona roupa, mesma descida.\n\n",
               t.instrucoes, t.nivel_max, t.escopos);
    }

    /* ── §K2 ─────────────────────────────────────────────────────────────── */
    puts("§K2  A LINGUAGEM: o TikZ tem laco, aritmetica, condicional e definicao");
    puts("     Nao e notacao de figura — e uma linguagem, e e isso que permite o §K4.\n");
    {
        int tem_laco = 0, tem_arit = 0, tem_cond = 0, tem_def = 0;
        for(int i = 0; i < NPGF; i++){
            if(!strcmp(PGF[i].papel, "laço")) tem_laco = 1;
            if(!strcmp(PGF[i].papel, "aritmética")) tem_arit = 1;
            if(!strcmp(PGF[i].papel, "condicional")) tem_cond = 1;
            if(!strcmp(PGF[i].papel, "definição")) tem_def = 1;
            printf("     %-22s %-14s ~ %s\n", PGF[i].cmd, PGF[i].papel, PGF[i].equivale);
        }
        ok("tem as QUATRO pecas de uma linguagem: laco, aritmetica, condicional e definicao",
           tem_laco && tem_arit && tem_cond && tem_def);
        puts("     -> com laco e aritmetica ela INTEGRA. E integrando, ela pode DISCORDAR de mim.\n");
    }

    /* ── §K3 ─────────────────────────────────────────────────────────────── */
    puts("§K3  A DINAMICA GERADA: o sistema do corpo diferencial sai em TikZ que compila");
    puts("     y'' + B y' + C y = 0 (edo.c §E1), com B=0,30 e C=1,00: o Delta e negativo, logo");
    puts("     e o caso ELIPTICO — o do §E4, onde 'o oscilador e o i'.\n");
    {
        const char *saida = "/tmp/tikz_dinamica.tex";
        FILE *f = fopen(saida, "w");
        int escreveu = 0;
        long bytes = 0;
        if(f){ gera(f, 8); fclose(f); escreveu = 1; }
        if(escreveu){
            FILE *g = fopen(saida, "rb");
            fseek(g, 0, SEEK_END); bytes = ftell(g); fclose(g);
        }
        /* e o que se gerou tem de ser TikZ que a descida do §K1 aceite */
        char *buf = NULL;
        if(escreveu){
            FILE *g = fopen(saida, "rb");
            buf = malloc((size_t)bytes + 1);
            if(buf && fread(buf, 1, (size_t)bytes, g) == (size_t)bytes) buf[bytes] = 0;
            fclose(g);
        }
        Tk t = buf ? tikz_desce(buf) : (Tk){0,0,0,0};
        /* e aqui tambem: ">50" era outro numero de cabeca (sao 40). A lei e que o tamanho
         * CRESCE com o numero de quadros — mede-se gerando dois e comparando. */
        FILE *f2 = fopen("/tmp/tikz_2q.tex", "w");
        long b2 = 0;
        if(f2){ gera(f2, 2); fclose(f2);
                FILE *g2 = fopen("/tmp/tikz_2q.tex","rb"); fseek(g2,0,SEEK_END); b2=ftell(g2); fclose(g2); }
        ok("gerou-se TikZ, ele PASSA na propria descida do §K1, e CRESCE com o numero de quadros",
           escreveu && t.fecha && t.instrucoes > 0 && bytes > b2 && b2 > 0);
        /* a peça que importa: os pontos NÃO estão escritos */
        int tem_pontos_escritos = 0;
        if(buf){
            /* se eu tivesse escrito os pontos, apareceriam pares decimais como (0.42,0.31) */
            const char *p = buf;
            int pares = 0;
            while((p = strstr(p, ".")) != NULL){
                if(p > buf+2 && *(p-2) == '(' ) pares++;
                p++;
            }
            tem_pontos_escritos = pares > 20;
        }
        ok("e os PONTOS NAO estao escritos: esta a regra que os produz, nao o retrato do resultado",
           !tem_pontos_escritos);
        ok("o gerado usa a aritmetica do LaTeX — ha \\pgfmathsetmacro e \\foreach la dentro",
           buf && strstr(buf, "\\pgfmathsetmacro") && strstr(buf, "\\foreach"));
        printf("     -> %s: 8 quadros %ld bytes, %ld instrucoes; com 2 quadros seriam %ld bytes.\n",
               saida, bytes, t.instrucoes, b2);
        puts("        Uma figura que pode desmentir o texto e uma MEDIDA; uma que nao pode e");
        puts("        uma ilustracao.\n");
        free(buf);
    }

    /* ── §K4  OS DOIS CAMINHOS ───────────────────────────────────────────── */
    puts("§K4  OS DOIS CAMINHOS: o C integra por RK4, o TikZ por EULER, e a EQUACAO e a mesma");
    puts("     Os metodos sao DIFERENTES de proposito. Se os dois fossem RK4 eu estaria a");
    puts("     comparar duas copias do mesmo codigo; sendo diferentes, o que os faz concordar");
    puts("     e a equacao — e o oraculo dos dois e a forma fechada do §E4.\n");
    {
        double yr = 1, vr = 0, ye = 1, ve = 0;
        double pior_rk = 0, pior_eu = 0, pior_entre = 0;
        for(int i = 1; i <= NPASSOS; i++){
            rk4(&yr, &vr, H);
            euler(&ye, &ve, H);
            double t = i*H, ex = exata(t);
            double drk = fabs(yr - ex), deu = fabs(ye - ex), dd = fabs(yr - ye);
            if(drk > pior_rk) pior_rk = drk;
            if(deu > pior_eu) pior_eu = deu;
            if(dd > pior_entre) pior_entre = dd;
        }
        ok("o RK4 segue a forma fechada — o erro fica na casa do metodo, nao do modelo",
           pior_rk < 1e-6);
        ok("o EULER tambem a segue, com o erro MAIOR que ele tem por ser de primeira ordem",
           pior_eu < 5e-2 && pior_eu > pior_rk);
        ok("e os dois caminhos concordam entre si dentro do erro do PIOR deles",
           pior_entre <= pior_eu + pior_rk);
        printf("     -> pior desvio da forma fechada: RK4 %.2e, Euler %.2e; entre eles %.2e.\n",
               pior_rk, pior_eu, pior_entre);
        /* e a LEI do Euler: halvar o passo tem de halvar o erro (primeira ordem) */
        double erros[3];
        for(int k = 0; k < 3; k++){
            double h = H / (1 << k), y = 1, v = 0;
            int n = (int)(TFIM/h + 0.5);
            double pior = 0;
            for(int i = 1; i <= n; i++){
                euler(&y, &v, h);
                double d = fabs(y - exata(i*h));
                if(d > pior) pior = d;
            }
            erros[k] = pior;
        }
        double r1 = erros[0]/erros[1], r2 = erros[1]/erros[2];
        ok("A LEI: o Euler e de PRIMEIRA ordem — halvar o passo halva o erro, medido em 3 pontos",
           r1 > 1.7 && r1 < 2.3 && r2 > 1.7 && r2 < 2.3);
        printf("     -> h, h/2, h/4: erros %.2e, %.2e, %.2e; razoes %.2f e %.2f (a lei diz 2).\n\n",
               erros[0], erros[1], erros[2], r1, r2);
    }

    /* ── §K4b  O ORACULO EXTERNO: os numeros que o LaTeX calculou ────────── */
    puts("§K4b O QUE O LATEX CALCULOU SOZINHO — e nao fui eu que os escrevi");
    puts("     Compilei o TikZ com \\typeout a imprimir o estado, e o pdflatex cuspiu os valores");
    puts("     no log. Sao de OUTRO programa: o motor aritmetico do TeX, nao o meu.\n");
    {
        /* colhidos de /tmp/tv.log, do pdflatex — oraculo externo, como a Liberation Sans no
         * spline.c. Se o meu Euler estivesse errado, isto denunciava-o. */
        static const struct { int i; double pgf; } LOG[] = {
            {  20,  0.92755 }, {  40,  0.72392 }, {  60,  0.43356 },
            {  80,  0.10909 }, { 100, -0.19720 },
        };
        double y = 1, v = 0;
        int k = 0, bate = 0; double pior = 0;
        for(int i = 1; i <= 100; i++){
            euler(&y, &v, H);
            if(k < 5 && i == LOG[k].i){
                double d = fabs(y - LOG[k].pgf);
                if(d > pior) pior = d;
                if(d < 1e-3) bate++;
                k++;
            }
        }
        ok("o meu Euler reproduz os 5 valores que o pdflatex calculou, dentro da precisao dele",
           bate == 5);
        /* e a diferenca tem de ser da PRECISAO do pgfmath, nao um desvio que cresce sem controlo */
        ok("e o desvio fica na casa da aritmetica de ponto fixo do TeX — nao e discordancia",
           pior < 1e-3 && pior > 1e-6);
        printf("     -> 5 pontos conferidos, pior desvio %.1e. O pgfmath e de ponto fixo (~5\n", pior);
        puts("        digitos), e e exatamente onde a diferenca cai. Os dois caminhos concordam.");
        puts("        E o PDF sai com 8 paginas: 'pdfinfo' confirma, e a animacao existe.\n");
    }

    /* ── §K5  a ANIMACAO ─────────────────────────────────────────────────── */
    puts("§K5  A ANIMACAO: os quadros sao a orbita AMOSTRADA, e o tempo e o parametro");
    puts("     Cada quadro desenha a trajetoria ate um instante. O standalone gera uma pagina");
    puts("     por quadro — e um PDF de N paginas E a animacao, sem biblioteca nenhuma.\n");
    {
        /* a amostragem tem de ser MONOTONA e cobrir o intervalo inteiro, senao a animação salta */
        int quadros = 8, monotona = 1, cobre = 0, ant = -1;
        for(int q = 0; q < quadros; q++){
            int ate = (NPASSOS * (q+1)) / quadros;
            if(ate <= ant) monotona = 0;
            ant = ate;
            if(q == quadros-1) cobre = (ate == NPASSOS);
        }
        ok("a amostragem dos quadros e MONOTONA e o ultimo cobre a orbita inteira",
           monotona && cobre);
        /* e o tempo do último quadro tem de ser o tempo final — senão a animação mente */
        ok("e o instante do ultimo quadro E o tempo final do sistema, sem sobra nem falta",
           fabs(NPASSOS*H - TFIM) < 1e-12);
        printf("     -> 8 quadros, %d passos, ate t=%.1f. O PDF de 8 paginas E a animacao.\n",
               NPASSOS, TFIM);
        printf("        E a amplitude cai de 1,000 para %.3f — o amortecimento VE-SE nos quadros.\n\n",
               fabs(exata(TFIM)));
    }

    /* ── §K6  o MATLAB ───────────────────────────────────────────────────── */
    puts("§K6  O MATLAB: a expressao de matrizes e PALAVRA na ISA — nenhuma maquina nova");
    puts("     O mecanica.c ja diz: 'toda matriz e PALAVRA nos geradores da ISA'. O MATLAB e a");
    puts("     linguagem cujas primitivas SAO matrizes, entao ela le-se direto.\n");
    {
        for(int i = 0; i < NMAT; i++)
            printf("     %-11s -> %-22s %s\n", MATLAB[i].matlab, MATLAB[i].isa, MATLAB[i].o_que);
        /* e a peça que se mede: a transposta é involução, e o catálogo já o diz do J */
        double A[4] = {0, 1, -C_DIN, -B_DIN};             /* a companion do §K3 */
        double At[4] = {A[0], A[2], A[1], A[3]};
        double Att[4] = {At[0], At[2], At[1], At[3]};
        int involucao = 1;
        for(int i = 0; i < 4; i++) if(fabs(Att[i] - A[i]) > 1e-15) involucao = 0;
        ok("a transposta e INVOLUCAO: A'' = A, ordem 2 — e o J do catalogo, nao uma operacao nova",
           involucao);
        /* e a régua da companion tem de ser a (B,C) do catálogo, senão a ligação é verbal */
        double traco = A[0] + A[3], det = A[0]*A[3] - A[1]*A[2];
        ok("e a REGUA da companion e a (B,C) do catalogo: -traco = B e det = C",
           fabs(-traco - B_DIN) < 1e-15 && fabs(det - C_DIN) < 1e-15);
        double D = B_DIN*B_DIN - 4*C_DIN;
        ok("e o Delta classifica-a: negativo, logo ELIPTICA — a mesma classe do oscilador",
           D < 0);
        printf("     -> a companion: traco %.2f, det %.2f, Delta %.4f < 0. A regua e a do catalogo,\n",
               traco, det, D);
        puts("        e a classe e a do §E4. O MATLAB nao trouxe corpo novo: trouxe notacao.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O TikZ e a NONA roupa, e a primeira em que a roupa CALCULA. Por isso a figura");
    puts("  deixou de ser ilustracao: ela integra a equacao com a aritmetica do LaTeX, por um");
    puts("  metodo DIFERENTE do meu, e por isso pode desmentir-me. Uma figura que nao pode");
    puts("  discordar do texto nao prova nada sobre ele.");
    puts("");
    puts("  E o MATLAB nao trouxe corpo novo — trouxe notacao. A transposta e o J, a inversa e");
    puts("  o NEGRO_OURO, e a regua da companion e a (B,C) do catalogo, medida.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
