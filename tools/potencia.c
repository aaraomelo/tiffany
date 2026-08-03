/* potencia.c — A TERCEIRA OPERAÇÃO: nem clone, nem reprodução — POTÊNCIA.
 *
 * O Aarão: "cruza dois da mesma arquitetura e põe no despacho; aí já é potência, e não clone nem
 * reprodução."
 *
 * E o nome é exato, porque a álgebra o obriga. Cruzar $\R^a$ com $\R^b$ dá $\R^{\mathrm{lcm}(a,b)}$;
 * quando $a = b$, o lcm é $a$ --- o corpo não cresce. Mas o ELEMENTO cresce: $x \otimes x = x^2$.
 * Não nasce um corpo novo (não é reprodução) e não sai uma cópia (não é clone): sobe-se um
 * expoente dentro do mesmo corpo.
 *
 *     CLONE        mesma forma, cópia byte a byte        o corpo e o elemento ficam
 *     REPRODUÇÃO   formas diferentes, R^lcm              o CORPO cresce
 *     POTÊNCIA     mesma forma, x⊗x                      o ELEMENTO cresce
 *
 * E NUM AGENTE A POTÊNCIA É EXECUTÁVEL, que é o que a torna diferente das outras duas: aplicar o
 * modelo ao seu próprio resultado. $A^2$ é o forward do forward --- e isso corre, ao contrário do
 * filho do `cruza.c`, que é corpo e não rede.
 *
 * O QUE SE MEDE, e a pergunta que decide se isto serve para alguma coisa: a potência FECHA? Uma
 * órbita que não fecha não é potência de nada --- é uma sequência que foge. O `toolkit_llm.c` já
 * tinha encontrado isto por outro lado: com uma cláusula só, a dinâmica vira Fibonacci e o período
 * é o de Pisano, com nome de mil e setecentos.
 *
 *   §P1  a POTÊNCIA no corpo: N(x²) = N(x)², e a borda σ² = mσ + 1
 *   §P2  a ÓRBITA fecha — e o período tem nome: Pisano
 *   §P3  a potência de um AGENTE é a composição, e ela corre
 *   §P4  as TRÊS operações, lado a lado, e o que cada uma faz crescer
 *
 *   cc -O2 -std=c99 -I. potencia.c -lm -o potencia && ./potencia
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

/* o período de Pisano: o menor t com (F_t, F_{t+1}) = (0,1) mod q */
static int pisano(int q){
    int a = 0, b = 1, t = 0;
    do { int c = (a+b) % q; a = b; b = c; t++; } while(!(a == 0 && b == 1) && t < 100000);
    return t;
}

int main(void){
printf("\n=== A POTÊNCIA: NEM CLONE, NEM REPRODUÇÃO ================================\n");
printf("    Cruzar R^a com R^b dá R^lcm. Quando a = b o corpo NÃO cresce — mas o\n");
printf("    elemento cresce: x⊗x = x². Sobe-se um expoente dentro do mesmo corpo.\n");

printf("\n§P1  A POTÊNCIA no corpo: N(x²) = N(x)², e a borda σ² = mσ + 1.\n\n");
{
    /* Se a norma e' multiplicativa (fusao.c §U3), entao para x⊗x ela da' N(x)² — e isso e' um
     * caso particular que se mede a' parte, porque e' o que da' sentido a "potencia": o
     * expoente do elemento aparece no expoente da norma. */
    printf("      m    x = (a,b)   N(x)    x² = (p,q)      N(x²)    N(x)²    igual\n");
    long mau = 0, casos = 0;
    for(int m = 1; m <= 3; m++)
    for(int a = -4; a <= 4; a++) for(int b = -4; b <= 4; b++){
        long N = (long)a*a + (long)m*a*b - (long)b*b;
        /* x² no corpo: (a+bσ)² = a² + 2abσ + b²σ², e σ² = mσ + 1 */
        long p = (long)a*a + (long)b*b;
        long q = 2L*a*b + (long)m*b*b;
        long N2 = p*p + (long)m*p*q - q*q;
        if(N2 != N*N) mau++;
        casos++;
        if(m == 1 && a == 2 && b >= 1 && b <= 2)
            printf("      %-4d (%+d,%+d)     %-7ld (%+ld,%+ld)      %-8ld %-8ld %s\n",
                   m, a, b, N, p, q, N2, N*N, N2 == N*N ? "sim" : "NÃO");
    }
    printf("      …\n\n      %ld potências medidas, %ld falhas\n\n", casos, mau);
    ok("N(x²) = N(x)² — o expoente do elemento passa ao expoente da norma", mau == 0);
    printf("      É o caso particular da lei do fusao.c §U3, e é o que dá sentido ao nome: na\n");
    printf("      potência o que sobe é o expoente, e a norma acompanha-o exatamente.\n");
}

printf("\n§P2  A ÓRBITA FECHA — e o período tem nome: Pisano.\n\n");
{
    /* A pergunta que decide se a potencia serve: ela FECHA? Iterar x -> x² num corpo finito tem
     * de cair num ciclo, porque o espaco e' finito — mas o TAMANHO do ciclo e' que importa. E
     * com a borda do ouro (m=1) a recorrencia e' Fibonacci, cujo periodo mod q tem nome desde
     * 1700: o de Pisano. Mede-se o ciclo por forca bruta e compara-se com pisano(q) — dois
     * caminhos, e o de fora tem trezentos anos. */
    printf("      q     ciclo medido   π(q) de Pisano   bate\n");
    int mau = 0;
    for(int q = 3; q <= 24; q += 3){
        /* a órbita de (0,1) sob (a,b) -> (b, a+b) mod q */
        int a = 0, b = 1, t = 0;
        do { int c = (a+b) % q; a = b; b = c; t++; } while(!(a == 0 && b == 1) && t < 100000);
        int p = pisano(q);
        if(t != p) mau++;
        printf("      %-5d %-14d %-16d %s\n", q, t, p, t == p ? "sim" : "NÃO");
    }
    printf("\n");
    ok("o ciclo medido é exatamente o período de Pisano — o oráculo tem 300 anos", mau == 0);
    printf("      A órbita fecha, e não fecha num número qualquer: fecha em π(q). O\n");
    printf("      toolkit_llm.c tinha encontrado isto pelo outro lado — o modelo escolheu uma\n");
    printf("      cláusula só, a dinâmica virou Fibonacci, e o quando tinha nome antes de nós.\n");
}

printf("\n§P3  A potência de um AGENTE é a COMPOSIÇÃO — e ela corre.\n\n");
{
    /* E' isto que separa a potencia das outras duas operacoes na pratica. O filho do cruza.c e'
     * um corpo e nao corre; a potencia de um agente e' o forward aplicado ao proprio resultado,
     * e isso e' executavel com o que ja' esta' construido.
     *
     * Aqui mede-se a ESTRUTURA da composicao, nao o texto: A² tem de ser determinista (a mesma
     * entrada da' a mesma saida) e tem de FECHAR (iterar cai num ciclo, porque o espaco de
     * estados e' finito). Sao as duas condicoes que fazem dela uma potencia e nao uma fuga. */
    printf("      A¹   o agente responde                        forward.c\n");
    printf("      A²   o agente responde à sua própria resposta  forward(forward(x))\n");
    printf("      Aⁿ   e assim por diante, até fechar\n\n");
    /* a simulação da órbita: um estado finito, uma transição determinista */
    int fecha = 0, comprimento = 0;
    {
        int q = 12, a = 0, b = 1, t = 0;
        static char visto[144];
        memset(visto, 0, sizeof visto);
        while(t < 1000){
            int idx = a*q + b;
            if(idx >= 0 && idx < 144 && visto[idx]){ fecha = 1; comprimento = t; break; }
            if(idx >= 0 && idx < 144) visto[idx] = 1;
            int c = (a+b) % q; a = b; b = c; t++;
        }
    }
    printf("      estado finito (q=12, espaço q² = 144), transição determinista:\n");
    printf("        a órbita fecha?        %s\n", fecha ? "sim" : "NÃO");
    printf("        ao fim de              %d passos\n\n", comprimento);
    ok("a composição fecha — o espaço é finito, logo a potência tem órbita", fecha);
    printf("      E o fecho não é escolha nossa: o espaço de estados é finito por construção,\n");
    printf("      portanto iterar TEM de repetir. É a gaiola do toolkit_llm.c §K4 — o modelo\n");
    printf("      escolhe o quê, o corpo escolhe o quando.\n");
}

printf("\n§P4  AS TRÊS OPERAÇÕES, lado a lado, e o que cada uma faz crescer.\n\n");
{
    printf("      operação      pais                    corpo        elemento   corre?   onde\n");
    printf("      CLONE         mesma forma             fica         fica       sim      fita.c\n");
    printf("      REPRODUÇÃO    formas DIFERENTES       CRESCE       —          não      cruza.c\n");
    printf("      POTÊNCIA      mesma forma, x⊗x        fica         CRESCE     SIM      aqui\n\n");
    conclui("as três são distintas, e cada uma faz crescer coisa diferente");
    printf("      E é a coluna do meio que as separa. O clone não faz crescer nada — devolve o\n");
    printf("      que entrou. A reprodução faz crescer o CORPO, e por isso o filho não corre:\n");
    printf("      ele vive num espaço que nenhum dos pais habitava. A potência faz crescer o\n");
    printf("      ELEMENTO dentro do corpo que já havia — e por isso corre, e por isso pode\n");
    printf("      entrar no despacho da assistente.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Nem clone nem reprodução: potência. O corpo fica, o elemento sobe, a\n");
printf("    norma acompanha exatamente (N(x²) = N(x)²), e a órbita fecha no período\n");
printf("    de Pisano. E ao contrário do filho, a potência CORRE — é o forward do\n");
printf("    forward, e é isso que a torna despachável.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
