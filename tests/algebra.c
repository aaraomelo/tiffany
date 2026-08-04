/* algebra.c — A ÁLGEBRA GLOBAL: uma máquina, e o CORPO é o argumento.
 *
 *   §A1  a mesma máquina dá C, o ouro e o ciclotómico — só muda a BORDA
 *   §A2  e sobe de dimensão sem uma linha nova: n = 3, 4, 5
 *   §A3  a notação algébrica é reversível: escrever e ler devolve o mesmo
 *   §A4  o corpo declara-se, e a régua sai dele: traço, determinante, classe
 *   §A6  a FAMÍLIA REAL — a base ortonormal, e quando a potência fica nela
 *   §A5  e o que NÃO fecha diz porquê — a dimensão da caixa, e o grau a mais
 *
 *   cc -O2 -std=c99 algebra.c -o algebra && ./algebra
 */
#include <stdio.h>
#include <string.h>
#include "algebra.h"
#include "unidade.h"

/* resolve "borda | expressão" e escreve o resultado; devolve 0 se falhar */
static int corre(const char *borda_txt, const char *e1, const char *op, const char *e2,
                 char *out, size_t lim){
    Elem borda; char marca[4];
    int n = al_le_borda(borda_txt, &borda, marca);
    if(!n) return 0;
    Elem a, b;
    const char *p = e1;
    if(al_le_elem(&p, n, marca, &a) != 1) return 0;
    p = e2;
    if(al_le_elem(&p, n, marca, &b) != 1) return 0;
    Elem r;
    if(!strcmp(op, "x"))      r = al_prod(a, b, &borda);
    else if(!strcmp(op, "+")) r = al_soma(a, b, +1);
    else if(!strcmp(op, "-")) r = al_soma(a, b, -1);
    else return 0;
    al_escreve(r, out, lim, marca);
    return 1;
}

int main(void){
printf("\n=== A ÁLGEBRA GLOBAL DE R^n — o corpo é o ARGUMENTO ======================\n");
printf("    Até aqui o i estava CRAVADO na célula, com σ² = -1 escrito à mão no\n");
printf("    código. Aqui o particular sai para fora: o elemento é uma tupla escrita\n");
printf("    na notação algébrica, e o corpo é a BORDA que se declara.\n");

printf("\n§A1  A mesma máquina dá C, o ouro e o ciclotómico — só muda a borda.\n\n");
{
    struct { const char *borda; const char *a; const char *op; const char *b; const char *r;
             const char *quem; } t[] = {
        { "s^2 = -1",     "s",      "x", "s",      "-1",     "C: o i" },
        { "s^2 = -1",     "1 + 2s", "x", "1 - 2s", "5",      "C: a norma do par conjugado" },
        { "s^2 = -1",     "1 + s",  "x", "1 + s",  "2s",     "C: (1+i)² = 2i" },
        { "s^2 = s + 1",  "s",      "x", "s",      "1 + s",  "o OURO: σ² = σ + 1" },
        { "s^2 = s + 1",  "s",      "x", "1 + s",  "1 + 2s", "o ouro: σ³ = 1 + 2σ" },
        { "s^2 = 2s + 1", "s",      "x", "s",      "1 + 2s", "a PRATA" },
        { "s^2 = s - 1",  "s",      "x", "s",      "-1 + s", "o 6º CICLOTÓMICO" },
        { "s^2 = -1",     "3",      "x", "s",      "3s",     "o escalar atravessa" },
        { "s^2 = -1",     "1 + 2s", "+", "3 - s",  "4 + s",  "a soma é coordenada a coordenada" },
    };
    int mal = 0;
    printf("      borda           conta                      dá          o quê\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        char b[128] = "?";
        if(!corre(t[k].borda, t[k].a, t[k].op, t[k].b, b, sizeof b)) snprintf(b, sizeof b, "FALHOU");
        printf("      %-15s (%s) %s (%s)%*s %-11s %s\n", t[k].borda, t[k].a, t[k].op, t[k].b,
               (int)(14 - strlen(t[k].a) - strlen(t[k].b)), "", b, t[k].quem);
        if(strcmp(b, t[k].r)) mal++;
    }
    printf("\n");
    ok("a mesma máquina dá C, o ouro, a prata e o ciclotómico — só muda a borda", mal == 0);
    printf("      Não há um ramo por corpo no código. Há UM produto de polinómios e UMA redução,\n");
    printf("      e a redução usa a borda que veio no argumento. O i deixou de ser uma linha\n");
    printf("      escrita à mão para ser σ² = -1, que é o que ele sempre foi.\n");
}

printf("\n§A2  E sobe de dimensão sem uma linha nova.\n\n");
{
    struct { const char *borda; const char *a; const char *op; const char *b; const char *r; } t[] = {
        { "s^3 = s + 1",   "s",         "x", "s",       "s^2" },
        { "s^3 = s + 1",   "s^2",       "x", "s",       "1 + s" },
        { "s^3 = s + 1",   "1 + s",     "x", "1 + s",   "1 + 2s + s^2" },
        { "s^3 = 1",       "s",         "x", "s^2",     "1" },
        { "s^4 = -1",      "s^2",       "x", "s^2",     "-1" },
        { "s^5 = s^4 + 1", "s^4",       "x", "s",       "1 + s^4" },
    };
    int mal = 0;
    printf("      borda            conta                     dá\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        char b[128] = "?";
        if(!corre(t[k].borda, t[k].a, t[k].op, t[k].b, b, sizeof b)) snprintf(b, sizeof b, "FALHOU");
        printf("      %-16s (%s) %s (%s)%*s %s\n", t[k].borda, t[k].a, t[k].op, t[k].b,
               (int)(12 - strlen(t[k].a) - strlen(t[k].b)), "", b);
        if(strcmp(b, t[k].r)) mal++;
    }
    printf("\n");
    ok("n = 3, 4 e 5 correm na mesma máquina, sem código por dimensão", mal == 0);
    printf("      Repare-se em s^3 = 1: é a raiz cúbica da unidade, e s x s² dá 1 — o ciclo\n");
    printf("      fecha em três em vez de quatro. E em s^5 = s^4 + 1, que é o polinómio do\n");
    printf("      OURO em dimensão 5, o do furo: a máquina opera lá na mesma, porque operar\n");
    printf("      não exige que o quociente seja corpo. O que falha lá é a INVERSA, não o\n");
    printf("      produto — e é essa a diferença entre anel e corpo, medida em vez de citada.\n");
}

printf("\n§A3  A notação algébrica é reversível: escrever e ler devolve o mesmo.\n\n");
{
    /* se a notacao e mesmo notacao, e nao uma perda, entao ler o que se escreveu devolve o
     * elemento. E o ponto do Aarao: a marcacao implicita da tupla, escrita, nao perde nada. */
    Elem borda; char marca[4];
    al_le_borda("s^3 = s + 1", &borda, marca);
    int mal = 0, quantos = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++) for(long c = -3; c <= 3; c++){
        Elem x = al_zero(3);
        x.p[0] = a; x.p[1] = b; x.p[2] = c;
        char t[128];
        al_escreve(x, t, sizeof t, marca);
        const char *p = t;
        Elem y;
        int r = al_le_elem(&p, 3, marca, &y);
        quantos++;
        if(a == 0 && b == 0 && c == 0){ if(strcmp(t, "0")) mal++; continue; }
        if(r != 1 || !al_igual(x, y)) mal++;
    }
    printf("      %d tuplas de R^3 escritas e relidas: %d que não voltaram\n\n", quantos, mal);
    ok("escrever a tupla e lê-la de volta devolve a MESMA tupla — resíduo 0", mal == 0);
    printf("      É o que faz da notação algébrica uma notação e não uma perda: a marcação\n");
    printf("      posicional da tupla e a simbólica do polinómio guardam a mesma informação, e\n");
    printf("      a volta prova-o. Se perdesse alguma coisa, ela não voltaria.\n");
}

printf("\n§A4  O corpo declara-se, e a régua sai dele.\n\n");
{
    struct { const char *borda; long B, C, D; const char *classe; const char *quem; } t[] = {
        { "s^2 = -1",     0,  1, -4, "elíptico",    "C — gira e não gasta" },
        { "s^2 = s + 1",  1, -1,  5, "hiperbólico", "o ouro — cresce e gasta" },
        { "s^2 = 2s + 1", 2, -1,  8, "hiperbólico", "a prata" },
        { "s^2 = s - 1",  1,  1, -3, "elíptico",    "o 6º ciclotómico" },
    };
    int mal = 0;
    printf("      borda           B    C    Δ = B²-4C   classe        o quê\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Elem borda; char marca[4];
        al_le_borda(t[k].borda, &borda, marca);
        long B, C, D;
        al_regua2(borda, &B, &C, &D);
        printf("      %-15s %+3ld  %+3ld   %+5ld      %-13s %s\n", t[k].borda, B, C, D,
               D > 0 ? "hiperbólico" : D < 0 ? "elíptico" : "parabólico", t[k].quem);
        if(B != t[k].B || C != t[k].C || D != t[k].D) mal++;
    }
    printf("\n");
    ok("a régua (B, C, Δ) lê-se da borda, e classifica o corpo", mal == 0);
    printf("      σ² = b₀ + b₁σ é σ² − b₁σ − b₀ = 0, logo o traço É b₁ e o determinante é −b₀.\n");
    printf("      A régua do catálogo não precisa de ser dada à parte: ela ESTÁ na borda, e\n");
    printf("      declarar o corpo é declarar a régua. É o mesmo (B,C) dos 44 do catálogo.\n");
}

printf("\n§A6  A FAMÍLIA REAL: a base ortonormal, e quando a potência fica nela.\n\n");
{
    /* O Aarao fixou o nome: FAMILIA REAL e a base ortonormal da algebra em R^n, os n
     * marcadores {1, s, s², …, s^(n-1)}. E ele disse que as potencias das unidades sao
     * potencias de elementos dela, que "so trocam sinal, fazem flip".
     *
     * Mede-se, e aparece uma distincao fina que vale a pena guardar: ficar na familia NAO e o
     * mesmo que ter ordem finita. */
    struct { const char *borda; int n; const char *quem; int fica; } t[] = {
        { "s^2 = -1",    2, "C: o i",             1 },
        { "s^3 = 1",     3, "a raiz cúbica de 1", 1 },
        { "s^4 = -1",    4, "a raiz oitava",      1 },
        { "s^2 = s + 1", 2, "o ouro",             0 },
        { "s^2 = s - 1", 2, "o 6º ciclotómico",   0 },
    };
    int mal = 0;
    printf("      borda           potências de s (as 5 primeiras)          fica na família?\n");
    for(size_t k = 0; k < sizeof t/sizeof *t; k++){
        Elem borda; char marca[4];
        int n = al_le_borda(t[k].borda, &borda, marca);
        Elem s2 = al_zero(n); s2.p[1] = 1;
        char linha[160] = ""; int fica = 1;
        Elem x = al_um(n);
        for(int j = 0; j < 5; j++){
            char b[48]; al_escreve(x, b, sizeof b, marca);
            strncat(linha, b, sizeof linha - strlen(linha) - 2);
            strncat(linha, " ", sizeof linha - strlen(linha) - 2);
            /* fica na família = uma só coordenada, e ela vale ±1 */
            int nz = 0, um = 1;
            for(int c = 0; c < n; c++) if(x.p[c]){ nz++; if(x.p[c] != 1 && x.p[c] != -1) um = 0;
                                                   if(x.q[c] != 1) um = 0; }
            if(nz != 1 || !um) fica = 0;
            x = al_prod(x, s2, &borda);
        }
        printf("      %-15s %-40s %s\n", t[k].borda, linha, fica ? "SIM" : "não");
        if(fica != t[k].fica) mal++;
    }
    printf("\n");
    ok("a potência cai SOBRE um eixo exatamente quando a borda é monomial", mal == 0);
    printf("      A família real É A CIFRA DO REI — o mesmo objeto, e os nomes são sinónimos: a\n");
    printf("      cifra é a sequência que gera, a família é o conjunto de eixos que ela varre. E\n");
    printf("      a potência CAI SOBRE UM EIXO — só troca de eixo e de sinal, o flip puro —\n");
    printf("      exatamente quando a borda é MONOMIAL: s^n = ±s^j. Aí o gerador permuta os\n");
    printf("      eixos e mais nada, e é isso que o i faz em n=2.\n");
    printf("\n      E A DISTINÇÃO FINA, que a medida deu e eu não esperava: a potência NUNCA sai do\n");
    printf("      espaço gerado — isso é a construção — mas pode cair SOBRE um eixo ou numa\n");
    printf("      COMBINAÇÃO deles, e isso não é o mesmo que ter ordem finita. O 6º ciclotómico\n");
    printf("      é elíptico e volta a 1 em seis\n");
    printf("      passos, mas pelo caminho passa por -1 + s, que MISTURA os eixos. Gira, e não é\n");
    printf("      flip. O flip puro é mais forte do que a ordem finita, e são coisas separadas.\n");
    printf("\n      É por isso que as unidades de Z[i] são exatamente ±a família real (quatro), e\n");
    printf("      as de Z[w] são seis mas nem todas são ±base: lá w² = -1 - w. A frase \"as\n");
    printf("      unidades são potências de elementos da família real\" vale onde a borda é\n");
    printf("      monomial, e o i é o caso que interessa aqui.\n");
}

printf("\n§A5  E o que não fecha diz porquê.\n\n");
{
    Elem borda; char marca[4];
    int n1 = al_le_borda("s^2 = -1", &borda, marca);
    const char *p = "1 + s^3";
    Elem x;
    int r1 = al_le_elem(&p, n1, marca, &x);
    int n2 = al_le_borda("s^9 = 1", &borda, marca);
    int n3 = al_le_borda("s^1 = 1", &borda, marca);
    int n4 = al_le_borda("2 = 3", &borda, marca);
    printf("      \"1 + s^3\" com n=2   -> %d   (grau acima da dimensão declarada)\n", r1);
    printf("      \"s^9 = 1\"           -> %d   (a caixa desta máquina acaba em %d)\n", n2, AL_N);
    printf("      \"s^1 = 1\"           -> %d   (n=1 é Q, e aí não há marcador para declarar)\n", n3);
    printf("      \"2 = 3\"             -> %d   (não é borda nenhuma)\n\n", n4);
    ok("o grau acima da dimensão é recusado, e não truncado", r1 == -1);
    ok("e a borda mal formada é recusada, com a dimensão da caixa dita", !n2 && !n3 && !n4);
    printf("      O n=9 é da CAIXA e não da matemática — R^9 existe, e é esta máquina que acaba\n");
    printf("      em %d. É a mesma distinção do 2^100: o número existe, a caixa é que acaba, e\n", AL_N);
    printf("      confundir as duas seria dizer que a matemática tem um limite que é meu.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
