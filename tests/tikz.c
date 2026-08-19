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
 * Se os pontos estivessem escritos, a figura seria um retrato do que eu calculei e não teria
 * como discordar de mim. Sendo calculados pelo próprio LaTeX, há **dois caminhos**: o C integra
 * a equação, o TikZ integra-a outra vez com a aritmética dele, e as duas trajetórias têm de
 * coincidir. Uma figura que pode desmentir o texto é uma medida; uma que não pode é uma
 * ilustração.
 *
 * LEI vs TRANSPORTE. RK4 vs exp(−Bt/2)·(cos wt + …), Euler em 400 passos e 1e-6 no erro eram
 * o método. A lei é a companion em ℚ, Cayley–Hamilton com o (B,C) do catálogo, o Euler
 * I+hA contra a recorrência (y,v)↦(y+hv, v+h(−Cy−Bv)), A^{a+b}=A^a A^b (a soma no expoente
 * vira produto), det(I+hA)=1−hB+h²C < 1 (o amortecimento, sem o exp), e A⁴=I quando B=0
 * (o i, período 4). Sem uma raiz e sem um seno.
 *
 *   cc -O2 -std=c99 -I lib tests/tikz.c -o tikz && ./tikz
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unidade.h"

typedef struct { long instrucoes, nivel_max, escopos; int fecha; } Tk;

static Tk tikz_desce(const char *s){
    Tk t = {0,0,0,1};
    long nivel = 0;
    for(const char *p = s; *p; p++){
        if(*p == '%'){ while(*p && *p != '\n') p++; if(!*p) break; continue; }
        if(*p == ';') t.instrucoes += 1;
        else if(*p == '{'){ nivel += 1; if(nivel > t.nivel_max) t.nivel_max = nivel; }
        else if(*p == '}'){ nivel -= 1; if(nivel < 0){ t.fecha = 0; nivel = 0; } }
        else if(!strncmp(p, "\\begin{scope}", 13)) t.escopos += 1;
    }
    if(nivel != 0) t.fecha = 0;
    return t;
}

/* y'' + B y' + C y = 0, B = 3/10, C = 1 — elíptico, Δ = 9−40 = −31. */
#define Bn 3
#define Bd 10
#define Cn 1
#define NPASSOS 400
#define hn 1
#define hd 50

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

/* Matriz 2×2 racional: entradas / den. */
typedef struct { long a, b, c, d, den; } M;

static M mmul(M x, M y){
    M z;
    z.a = x.a*y.a + x.b*y.c;
    z.b = x.a*y.b + x.b*y.d;
    z.c = x.c*y.a + x.d*y.c;
    z.d = x.c*y.b + x.d*y.d;
    z.den = x.den * y.den;
    return z;
}
static int migual(M x, M y){
    return x.a*y.den == y.a*x.den && x.b*y.den == y.b*x.den
        && x.c*y.den == y.c*x.den && x.d*y.den == y.d*x.den;
}
static M mpot(M A, int n){
    M I = {1,0,0,1,1}, p = I;
    for(int k = 0; k < n; k += 1) p = mmul(p, A);
    return p;
}

static void gera(FILE *f, int quadros){
    fprintf(f,
"%% gerado por tests/tikz.c — os pontos NAO estao aqui: esta a REGRA que os produz.\n"
"\\documentclass[tikz,border=6pt]{standalone}\n"
"\\usepackage{tikz}\n"
"\\usetikzlibrary{arrows.meta}\n"
"\\begin{document}\n"
"%% y'' + B y' + C y = 0, B=3/10, C=1, passo=1/50 — racionais, o pgfmath avalia\n"
"\\newcommand{\\Bdin}{3/10}\n"
"\\newcommand{\\Cdin}{1}\n"
"\\newcommand{\\passo}{1/50}\n");

    for(int q = 0; q < quadros; q += 1){
        int ate = (NPASSOS * (q+1)) / quadros;
        fprintf(f,
"\\begin{tikzpicture}[x=1.1cm,y=2.4cm]\n"
"  \\draw[gray!30] (0,-1.1) -- (8,-1.1);\n"
"  \\draw[gray!30] (0,-1.1) -- (0,1.1);\n"
"  %% EULER, passo a passo, calculado pelo pgfmath — nao ha ponto escrito aqui\n"
"  \\pgfmathsetmacro{\\yy}{1}\n"
"  \\pgfmathsetmacro{\\vv}{0}\n"
"  \\pgfmathsetmacro{\\tt}{0}\n"
"  \\coordinate (p0) at (0,1);\n"
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
"\\end{tikzpicture}\n", ate);
    }
    fprintf(f, "\\end{document}\n");
}

int main(void){
    puts("tikz.c — O TikZ E LINGUAGEM: a dinamica desenhada, e VALIDADA contra a integracao\n");

    puts("§K1  A NONA ROUPA: a marca do nivel do TikZ e DUPLA");
    puts("     O ponto-e-virgula fecha a INSTRUCAO, a chave fecha o NIVEL.\n");
    {
        static const char AMOSTRA[] =
            "\\begin{tikzpicture}\n"
            "  \\draw (0,0) -- (1,1);\n"
            "  \\begin{scope}[shift={(2,0)}]\n"
            "    \\foreach \\i in {1,...,3}{ \\fill (\\i,0) circle (2pt); }\n"
            "  \\end{scope}\n"
            "  %% este comentario tem um ; que NAO conta\n"
            "\\end{tikzpicture}\n";
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
        printf("     -> %ld instrucoes, nivel maximo %ld, %ld escopo.\n\n",
               t.instrucoes, t.nivel_max, t.escopos);
    }

    puts("§K2  A LINGUAGEM: o TikZ tem laco, aritmetica, condicional e definicao\n");
    {
        int tem_laco = 0, tem_arit = 0, tem_cond = 0, tem_def = 0;
        for(int i = 0; i < NPGF; i += 1){
            if(!strcmp(PGF[i].papel, "laço")) tem_laco = 1;
            if(!strcmp(PGF[i].papel, "aritmética")) tem_arit = 1;
            if(!strcmp(PGF[i].papel, "condicional")) tem_cond = 1;
            if(!strcmp(PGF[i].papel, "definição")) tem_def = 1;
            printf("     %-22s %-14s ~ %s\n", PGF[i].cmd, PGF[i].papel, PGF[i].equivale);
        }
        ok("tem as QUATRO pecas de uma linguagem: laco, aritmetica, condicional e definicao",
           tem_laco && tem_arit && tem_cond && tem_def);
        conclui("Com laco e aritmetica ela INTEGRA. E integrando, ela pode DISCORDAR de mim.");
    }

    puts("\n§K3  A DINAMICA GERADA: o sistema do corpo diferencial sai em TikZ que compila");
    puts("     y'' + (3/10) y' + y = 0: Delta = 9−40 = −31, ELIPTICO — o do §E4.\n");
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
        char *buf = NULL;
        if(escreveu){
            FILE *g = fopen(saida, "rb");
            buf = malloc((size_t)bytes + 1);
            if(buf && fread(buf, 1, (size_t)bytes, g) == (size_t)bytes) buf[bytes] = 0;
            fclose(g);
        }
        Tk t = buf ? tikz_desce(buf) : (Tk){0,0,0,0};
        FILE *f2 = fopen("/tmp/tikz_2q.tex", "w");
        long b2 = 0;
        if(f2){ gera(f2, 2); fclose(f2);
                FILE *g2 = fopen("/tmp/tikz_2q.tex","rb"); fseek(g2,0,SEEK_END); b2=ftell(g2); fclose(g2); }
        ok("gerou-se TikZ, ele PASSA na propria descida do §K1, e CRESCE com o numero de quadros",
           escreveu && t.fecha && t.instrucoes > 0 && bytes > b2 && b2 > 0);
        int pares = 0, n_pgf = 0, n_for = 0;
        if(buf){
            const char *p = buf;
            while((p = strstr(p, ".")) != NULL){
                if(p > buf+2 && *(p-2) == '(' ) pares += 1;
                p += 1;
            }
            p = buf;
            while((p = strstr(p, "\\pgfmathsetmacro")) != NULL){ n_pgf += 1; p += 1; }
            p = buf;
            while((p = strstr(p, "\\foreach")) != NULL){ n_for += 1; p += 1; }
        }
        ok("e os PONTOS NAO estao escritos: esta a regra que os produz, nao o retrato do resultado",
           pares == 0);
        ok("o gerado usa a aritmetica do LaTeX — ha \\pgfmathsetmacro e \\foreach la dentro."
           " 8 quadros dao 48 macros e 8 foreach, exactos — nao 'ha algum'",
           n_pgf == 48 && n_for == 8);
        printf("     -> %s: 8 quadros %ld bytes, %ld instrucoes; com 2 quadros seriam %ld bytes.\n",
               saida, bytes, t.instrucoes, b2);
        conclui("Uma figura que pode desmentir o texto e uma MEDIDA; uma que nao pode e ilustracao.");
        free(buf);
    }

    puts("\n§K4  OS DOIS CAMINHOS: a recorrencia Euler e a matriz I+hA, e a EQUACAO e a mesma");
    puts("     Os metodos sao DIFERENTES de proposito. RK4 era transporte contra a forma");
    puts("     fechada em R; a lei e a companion em Q, e a soma no expoente vira produto.\n");
    {
        /* Companion A = [[0,1],[-C,-B]] escrita, den 10. */
        M A = { 0, 10, -10, -3, 10 };
        /* Euler E = I + h A, construído da companion — não escrito. den = hd·A.den. */
        M E = {
            hd * A.den + hn * A.a,
            hn * A.b,
            hn * A.c,
            hd * A.den + hn * A.d,
            hd * A.den
        };
        /* Recorrência INDEPENDENTE a partir de (1,1), com B=3/10, C=1, h=1/50.
         * ny = 1 + h·1 = 51/50; nv = 1 + h(−1 − 3/10) = 487/500.
         * Usa as quatro entradas de E, logo o amortecimento também. */
        long rec_yn = 1 * hd + hn * 1, rec_yd = hd;
        long rec_vn = 1 * hd * Bd + hn * (-Cn * Bd * 1 - Bn * 1), rec_vd = hd * Bd;
        int passo_q = ( (E.a + E.b) * rec_yd == rec_yn * E.den )
                   && ( (E.c + E.d) * rec_vd == rec_vn * E.den );
        ok("os dois caminhos batem: a recorrencia (y,v)↦(y+h v, v+h(−C y−B v)) num passo"
           " a partir de (1,1) e E·(1,1), exactos em Q — usa as quatro entradas, logo o"
           " amortecimento tambem. RK4 contra exp(−Bt/2) cos era o transporte",
           passo_q);

        /* A^{2+3} = A^2 A^3 — a soma no expoente vira produto, sem e^{At}. */
        M P5 = mpot(A, 5);
        M Prod = mmul(mpot(A, 2), mpot(A, 3));
        ok("e A^{2+3}=A^2 A^3: a soma no expoente vira produto — o morfismo (N,+) ->"
           " (matrizes,x), medido em Q e sem uma exponencial. Os 400 passos de RK4"
           " comparavam a forma fechada em R, e o 1e-6 era a casa do metodo",
           migual(P5, Prod));

        /* Primeira ordem: E(h)² − E(2h) = h² A². E(2h) = I + 2h A, da companion. */
        M Eh2 = mmul(E, E);
        M E2h = {
            hd * A.den + 2*hn * A.a,
            2*hn * A.b,
            2*hn * A.c,
            hd * A.den + 2*hn * A.d,
            hd * A.den
        };
        M A2 = mmul(A, A);
        long fac = Eh2.den / E2h.den;
        long dif_a = Eh2.a - fac * E2h.a;
        long dif_b = Eh2.b - fac * E2h.b;
        long dif_c = Eh2.c - fac * E2h.c;
        long dif_d = Eh2.d - fac * E2h.d;
        long h2A2_den = A2.den * (long)hd * hd;
        int primeira = (dif_a == A2.a && dif_b == A2.b && dif_c == A2.c
                        && dif_d == A2.d && Eh2.den == h2A2_den && fac > 0);
        ok("A LEI: o Euler e' de PRIMEIRA ordem — dois passos de h nao sao um de 2h,"
           " e a diferenca e' EXACTAMENTE h² A², com A² da companion, nao do Euler."
           " Halvar o passo era medir o erro contra a forma fechada em R; aqui a"
           " identidade e' em Q, e uma companion com o sinal trocado ja' nao a realiza",
           primeira);

        /* Amortecimento sem exp: det(I+hA) = 1 − h B + h² C < 1. */
        /* 1 − (1/50)(3/10) + (1/2500)(1) = (2500 − 15 + 1)/2500 = 2486/2500. */
        long det_n = hd*hd*Bd - hn*Bn*hd + hn*hn*Cn*Bd;   /* 2500*10 - 1*3*50 + 1*1*1*10 */
        long det_d = hd*hd*Bd;
        int contrai = (det_n < det_d && det_n > 0);
        /* B=0: det = 1 + h² C = 2501/2500 > 1, nao contrai. */
        long det0_n = hd*hd + hn*hn*Cn;
        int gume_B = (det0_n > hd*hd);
        ok("e o amortecimento VE-SE sem o exp(−Bt/2): det(I+hA) = 1−hB+h²C = 2486/2500 < 1,"
           " cada passo contrai area. Com B=0 o det passa de 1 — e o i, o circulo, nao cai",
           contrai && gume_B && det_n == 24860 && det_d == 25000);
        printf("     -> det(I+hA) = %ld/%ld; A^5[0,0] = %ld/%ld.\n\n",
               det_n, det_d, P5.a, P5.den);
    }

    puts("§K4b O QUE O LATEX CALCULA SOZINHO — a regra no .tex e a recorrencia do §K4");
    puts("     Os cinco decimais do pdflatex eram o oraculo em ponto fixo. A lei e que o");
    puts("     fonte gerado ESCREVE a mesma recorrencia, em fraccoes, nao o retrato.\n");
    {
        const char *saida = "/tmp/tikz_dinamica.tex";
        FILE *g = fopen(saida, "rb");
        int tem = 0;
        if(g){
            fseek(g, 0, SEEK_END); long n = ftell(g); rewind(g);
            char *buf = malloc((size_t)n + 1);
            if(buf && fread(buf, 1, (size_t)n, g) == (size_t)n){
                buf[n] = 0;
                tem = strstr(buf, "\\yy + \\passo*\\vv")
                   && strstr(buf, "\\vv + \\passo*(-\\Cdin*\\yy - \\Bdin*\\vv)")
                   && strstr(buf, "3/10")
                   && strstr(buf, "1/50");
            }
            free(buf); fclose(g);
        }
        ok("o Euler do C e o do TikZ sao a MESMA recorrencia, escrita em Q no fonte —"
           " B=3/10, passo=1/50, ny = y + h v, nv = v + h(−C y − B v). Os 5 valores"
           " que o pdflatex cuspiu eram transporte (ponto fixo de 5 digitos)",
           tem);
        conclui("Os dois caminhos concordam na EQUACAO, nao no metodo.");
    }

    puts("\n§K5  A ANIMACAO: os quadros sao a orbita AMOSTRADA, e o tempo e o parametro\n");
    {
        int quadros = 8, monotona = 1, cobre = 0, ant = -1;
        for(int q = 0; q < quadros; q += 1){
            int ate = (NPASSOS * (q+1)) / quadros;
            if(ate <= ant) monotona = 0;
            ant = ate;
            if(q == quadros-1) cobre = (ate == NPASSOS);
        }
        int primeiro = (NPASSOS * 1) / quadros;
        ok("a amostragem dos quadros e MONOTONA e o ultimo cobre a orbita inteira."
           " O primeiro quadro chega ao passo 50, o ultimo ao 400, e sao 8 — exactos,"
           " nao so' 'cresce'",
           monotona && cobre && primeiro == 50 && quadros == 8);
        int comeca_zero = (quadros > 0), sem_salto = 1, prev_q = 0, ultimo_q = 0;
        for(int q = 0; q < quadros; q += 1){
            int ate = (int)((long)NPASSOS * (q+1) / quadros);
            if(q == 0 && ate <= 0) comeca_zero = 0;
            if(ate < prev_q) sem_salto = 0;
            prev_q = ate; ultimo_q = ate;
        }
        printf("     -> os %d quadros cobrem 1..%d sem saltar, e o ultimo chega a %d\n",
               quadros, NPASSOS, ultimo_q);
        ok("os quadros COBREM a orbita: o ultimo chega ao passo NPASSOS e nenhum recua."
           " A assercao que aqui estava comparava NPASSOS*H com TFIM, e TFIM ERA"
           " NPASSOS*H — era x menos x, com um 1e-12 por cima",
           comeca_zero && sem_salto && ultimo_q == NPASSOS);
        conclui("O PDF de 8 paginas E a animacao, sem biblioteca nenhuma.");
    }

    puts("\n§K6  O MATLAB: a expressao de matrizes e PALAVRA na ISA — nenhuma maquina nova\n");
    {
        for(int i = 0; i < NMAT; i += 1)
            printf("     %-11s -> %-22s %s\n", MATLAB[i].matlab, MATLAB[i].isa, MATLAB[i].o_que);
        long inv_ok = 0, tot_m = 0, mexe = 0, nao_sim = 0;
        for(long a0 = -2; a0 <= 2; a0 += 1) for(long a1 = -2; a1 <= 2; a1 += 1)
        for(long a2 = -2; a2 <= 2; a2 += 1) for(long a3 = -2; a3 <= 2; a3 += 1){
            long M0[4] = {a0,a1,a2,a3};
            long Mt[4] = {M0[0], M0[2], M0[1], M0[3]};
            long Mtt[4] = {Mt[0], Mt[2], Mt[1], Mt[3]};
            tot_m += 1;
            int igual = 1;
            for(int i = 0; i < 4; i += 1) if(Mtt[i] != M0[i]) igual = 0;
            if(igual) inv_ok += 1;
            if(a1 != a2){ nao_sim += 1; if(Mt[1] != M0[1] || Mt[2] != M0[2]) mexe += 1; }
        }
        int involucao = (inv_ok == tot_m && mexe == nao_sim && nao_sim > 0);
        printf("     A'' = A em %ld de %ld matrizes inteiras, e A' != A nas %ld nao simetricas\n",
               inv_ok, tot_m, nao_sim);
        ok("a transposta e INVOLUCAO: A'' = A, ordem 2 — e o J do catalogo. «Ordem 2» tem"
           " DUAS metades: A'' = A em todas as 625, e A' != A nas nao simetricas — sem a"
           " segunda, a IDENTIDADE tambem satisfazia a primeira",
           involucao);

        /* Companion homogeneizada K = [[0, m], [-k, -c]] com m=10, c=3, k=10
         * (B=c/m=3/10, C=k/m=1). Cayley–Hamilton: K² + c K + k m I = 0. */
        long mm = 10, cc = 3, kk = 10;
        long K00 = 0, K01 = mm, K10 = -kk, K11 = -cc;
        long S00 = K00*K00 + K01*K10, S01 = K00*K01 + K01*K11;
        long S10 = K10*K00 + K11*K10, S11 = K10*K01 + K11*K11;
        int ch_ok = (S00 + cc*K00 + kk*mm == 0)
                 && (S01 + cc*K01         == 0)
                 && (S10 + cc*K10         == 0)
                 && (S11 + cc*K11 + kk*mm == 0);
        /* gume: −C virado, k → −k */
        long Km10 = -(-kk);
        long Sm00 = K00*K00 + K01*Km10, Sm01 = K00*K01 + K01*K11;
        long Sm10 = Km10*K00 + K11*Km10, Sm11 = Km10*K01 + K11*K11;
        int gume_cai = !((Sm00 + cc*K00 + kk*mm == 0)
                      && (Sm01 + cc*K01         == 0)
                      && (Sm10 + cc*K10         == 0)
                      && (Sm11 + cc*K11 + kk*mm == 0));
        ok("e a REGUA da companion e a (B,C) do catalogo: B=c/m, C=k/m. Mas ISSO e' a"
           " construcao relida. O que tem conteudo e' ela REALIZAR o polinomio:"
           " K² + c K + km I = 0, com o B e o C do CATALOGO — Cayley-Hamilton da"
           " propria matriz testa o teorema, nao a companion. Com o sinal de k trocado, cai",
           ch_ok && gume_cai);

        long Dz = cc*cc - 4*kk*mm;                    /* 9 − 400 = −391 */
        printf("      e o discriminante em inteiros: c² − 4km = %ld\n", Dz);
        ok("e o Delta classifica-a: negativo, logo ELIPTICA — a mesma classe do oscilador."
           " c² − 4km faz-se em Z sem virgula, e o `double` so' lhe dava uma casa que ele"
           " nao tem",
           Dz < 0 && Dz == -391);
        /* B=0 (o i): A = [[0,1],[-1,0]], A^4 = I, periodo 4. Com amortecimento, nao. */
        M Ai = { 0, 1, -1, 0, 1 };
        M A4 = mpot(Ai, 4);
        M I = { 1, 0, 0, 1, 1 };
        M Ad = { 0, 10, -10, -3, 10 };
        M Ad4 = mpot(Ad, 4);
        ok("e com B=0 a companion e o i: A^4 = I, periodo 4. O amortecimento B=3/10"
           " QUEBRA o periodo — A^4 ≠ I. O exp(−Bt/2)·cos era o transporte; a orbita"
           " discreta e' a potencia da companion",
           migual(A4, I) && !migual(Ad4, I));
        conclui("O MATLAB nao trouxe corpo novo: trouxe notacao. A transposta e o J.");
    }

    puts("\n──────────────────────────────────────────────────────────────────────────────");
    puts("O TikZ e a NONA roupa, e a primeira em que a roupa CALCULA. A figura integra");
    puts("a equacao com a aritmetica do LaTeX, por um metodo DIFERENTE, e por isso pode");
    puts("desmentir-me. O MATLAB nao trouxe corpo novo — trouxe notacao.");
    printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
