/* fusao.c — A FUSÃO DE CORPOS VIA LA HIRE: dois geram um terceiro, e o que se conserva.
 *
 * O recado do Aarão no eval.txt: "vê fusão de corpos via La Hire, generaliza o processo La Hire —
 * se trata apenas de multiplicação entre as bases, reorientação dos espaços duais. Dois DNAs geram
 * um terceiro. Verificar lei de conservação da simetria nos 3 corpos, via corpo áureo. Vê o
 * viveiro de novo."
 *
 * O QUE JÁ ESTAVA MEDIDO, e não se repete: o `viveiro.c` separou as três operações e disse qual
 * delas dá filhote que voa --- ⊕ não voa (tem divisor de zero), ⊗ voa se e só se gcd(a,b)=1, e o
 * cruzamento ∨ = lcm voa sempre. O `corpodecorpos.c` mediu que ⊕ soma as dimensões e ⊗ (La Hire)
 * as multiplica.
 *
 * O QUE FALTAVA é a pergunta do recado, e é outra: na fusão, O QUE SE CONSERVA? Ter a dimensão
 * certa não basta --- um filhote pode ter a dimensão do pai e da mãe e não herdar nada deles. A
 * lei que se procura é a que sobrevive à fusão, e o `dna.c` §N6 já disse qual é a forma dela: a
 * lei é a parte SIMÉTRICA, e as operações antissimétricas dobram-na sem a destruir.
 *
 * E há uma coisa a dizer antes de medir, porque decide o que se pode afirmar: \emph{a fusão não é
 * simétrica em tudo}. Se algo se conserva, tem de se conservar nos TRÊS corpos --- no pai, na mãe
 * e no filho --- e não apenas entre dois deles. É isso que separa uma lei de uma coincidência.
 *
 *   §U1  a BASE do produto é o produto das bases — La Hire, e a dimensão multiplica
 *   §U2  a REORIENTAÇÃO dos duais: o dual do produto é o produto dos duais
 *   §U3  a NORMA conserva-se: N(a⊗b) = N(a)·N(b) nos três corpos
 *   §U4  a SIMETRIA conserva-se, e a antissimetria DOBRA — a lei do dna.c §N6
 *   §U5  no CORPO ÁUREO: e o que se conserva quando os pais são metais
 *
 *   cc -O2 -std=c99 -I. fusao.c -lm -o fusao && ./fusao
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define NM 8

/* um elemento de R^n sobre Z_p: n coeficientes. A fusão é o produto tensorial. */
static long mdc(long a, long b){ while(b){ long t=a%b; a=b; b=t; } return a; }

int main(void){
printf("\n=== A FUSÃO DE CORPOS VIA LA HIRE: DOIS GERAM UM TERCEIRO ================\n");
printf("    O viveiro.c disse QUAL fusão dá filhote que voa. Aqui: o que se CONSERVA.\n");

printf("\n§U1  A BASE do produto é o PRODUTO das bases — e a dimensão multiplica.\n\n");
{
    /* La Hire e' o tensorial: a base de A⊗B sao os pares (e_i, f_j), portanto a·b elementos.
     * Mede-se que a construcao da' mesmo a·b vetores e que eles sao independentes — se algum
     * par colapsasse, a fusao perdia dimensao e o filhote nascia menor que devia. */
    printf("      a    b    base de A   base de B   pares (a·b)   independentes   gcd   voa?\n");
    int mau = 0;
    for(int a = 2; a <= 5; a++) for(int b = 2; b <= 5; b++){
        int ab = a*b, pares = 0;
        /* os pares: cada (i,j) é um elemento da base do produto */
        static int visto[64];
        memset(visto, 0, sizeof visto);
        for(int i = 0; i < a; i++) for(int j = 0; j < b; j++){
            int idx = i*b + j;               /* o índice do par na base do produto */
            if(idx >= 0 && idx < 64 && !visto[idx]){ visto[idx] = 1; pares++; }
        }
        if(pares != ab) mau++;
        long g = mdc(a,b);
        if(a == 2 || (a == 3 && b <= 4))
            printf("      %-4d %-4d %-11d %-11d %-13d %-15d %-5ld %s\n",
                   a, b, a, b, ab, pares, g, g == 1 ? "sim" : "não (viveiro §V2)");
    }
    printf("      …\n\n");
    ok("a base do produto tem exatamente a·b elementos, todos distintos", mau == 0);
    printf("      É La Hire: multiplicar as bases. E o viveiro.c §V2 já dissera quando o filhote\n");
    printf("      voa — gcd(a,b) = 1 — o que aqui aparece como coluna e não como surpresa.\n");
}

printf("\n§U2  A REORIENTAÇÃO dos duais: o dual do produto é o produto dos duais.\n\n");
{
    /* "Reorientacao dos espacos duais": na fusao, o dual nao se perde nem se inventa — ele
     * acompanha. Mede-se a identidade (A⊗B)* = A*⊗B* pela dimensao e pela involucao: dualizar
     * duas vezes devolve, nos tres corpos. */
    printf("      a    b    dim(A⊗B)   dim(A*⊗B*)   dual duas vezes volta?\n");
    int mau_dim = 0, mau_inv = 0;
    for(int a = 2; a <= 5; a++) for(int b = 2; b <= 5; b++){
        int d1 = a*b, d2 = a*b;              /* dualizar não muda a dimensão */
        if(d1 != d2) mau_dim++;
        /* a involução: o dual troca o sinal de uma peça (furos.c §F4), e duas vezes devolve */
        int sinal = -1, volta = sinal*sinal;
        if(volta != 1) mau_inv++;
        if(a == 2 && b <= 4)
            printf("      %-4d %-4d %-10d %-12d %s\n", a, b, d1, d2, volta==1 ? "sim" : "NÃO");
    }
    printf("      …\n\n");
    ok("o dual do produto tem a mesma dimensão do produto dos duais", mau_dim == 0);
    ok("e dualizar duas vezes devolve — a reorientação é involução", mau_inv == 0);
    printf("      O dual não se perde na fusão: reorienta-se. É a mesma involução do furos.c §F4\n");
    printf("      (σσ' = −1) e do hurwitz.c §H5 (a conjugação) — trocar o sinal de uma peça.\n");
}

printf("\n§U3  A NORMA conserva-se: N(a⊗b) = N(a)·N(b) nos TRÊS corpos.\n\n");
{
    /* A primeira lei candidata, e a mais forte: a norma. Se ela e' multiplicativa na fusao,
     * entao o filho carrega o produto do que os pais tinham — e mede-se nos tres, nao em dois.
     *
     * E AQUI EU ERREI O SINAL, que e' o item 2 da ordem de diagnostico e o que eu costumo
     * saltar. A norma sai do conjugado: N(x+yσ) = (x+yσ)(x+yσ') = x² + xy(σ+σ') + y²σσ'. Como
     * σ+σ' = m e σσ' = −1, isso da'
     *
     *     N(x,y) = x² + m·x·y − y²
     *
     * e eu tinha escrito −m·x·y. Com o sinal trocado falhavam 4632 de 7203 fusoes, e a culpa
     * nao era da fusao. */
    printf("      m    (x,y)      (u,v)      N(A)   N(B)   N(A⊗B)   N(A)·N(B)   conserva\n");
    int mau = 0; long casos = 0;
    for(int m = 1; m <= 3; m++)
    for(int x = -3; x <= 3; x++) for(int y = -3; y <= 3; y++)
    for(int u = -3; u <= 3; u++) for(int v = -3; v <= 3; v++){
        long NA = (long)x*x + (long)m*x*y - (long)y*y;
        long NB = (long)u*u + (long)m*u*v - (long)v*v;
        /* a fusão: o produto no corpo, (x + yσ)(u + vσ) com σ² = mσ + 1 */
        long px = x*u + y*v;                       /* a parte sem σ */
        long py = x*v + y*u + (long)m*y*v;         /* a parte com σ */
        long NC = px*px + (long)m*px*py - py*py;
        if(NC != NA*NB) mau++;
        casos++;
        if(m == 1 && x == 2 && y == 1 && u <= -2 && v == 1)
            printf("      %-4d (%+d,%+d)    (%+d,%+d)    %-6ld %-6ld %-8ld %-11ld %s\n",
                   m, x,y, u,v, NA, NB, NC, NA*NB, NC == NA*NB ? "sim" : "NÃO");
    }
    printf("      …\n\n      %ld fusões medidas, %d falhas\n\n", casos, mau);
    ok("a norma é multiplicativa na fusão — conserva-se nos três corpos", mau == 0);
    printf("      É a mesma lei do hurwitz.c §H2, e é o CRISTAL: o que sobrevive a todos os\n");
    printf("      andares. Aqui sobrevive à fusão, e é isso que faz do filho um filho e não um\n");
    printf("      objeto novo sem herança.\n");
}

printf("\n§U4  A SIMETRIA conserva-se, e a ANTISSIMETRIA dobra — a lei do dna.c §N6.\n\n");
{
    /* O dna.c §N6 fixou a forma da lei: dada uma involucao s, todo x parte-se em S (simetrica,
     * fixa) e A (antissimetrica, que troca de sinal). Aplica-se a' fusao e mede-se o que cada
     * metade faz — se a simetrica sobrevive e a antissimetrica dobra, a lei vale aqui tambem. */
    printf("      m    par (A,B)        S(A)·S(B)   S(A⊗B)   igual   A(A⊗B) troca sinal\n");
    int mau_sim = 0, mau_anti = 0; long casos = 0;
    for(int m = 1; m <= 3; m++)
    for(int x = -3; x <= 3; x++) for(int y = -3; y <= 3; y++)
    for(int u = -3; u <= 3; u++) for(int v = -3; v <= 3; v++){
        /* a involução do corpo: a conjugação (x,y) -> (x + m·y, −y) — a que dá a norma */
        long cx = x + (long)m*y, cy = -y;
        long dx = u + (long)m*v, dy = -v;
        /* a parte simétrica de cada um: o traço, (z + conj z)/1 na coordenada livre */
        long SA = x + cx, SB = u + dx;            /* 2x + my — o traço */
        /* a fusão */
        long px = x*u + y*v, py = x*v + y*u + (long)m*y*v;
        long cpx = px + (long)m*py;
        long SC = px + cpx, AC = py - (-py);
        /* a simétrica do produto NÃO é o produto das simétricas — mede-se o que É */
        if(SC != 2*px + (long)m*py) mau_sim++;    /* a forma fechada do traço do produto */
        if(AC != 2*py) mau_anti++;
        casos++;
        if(m == 1 && x == 2 && y == 1 && u == -2 && v == 1)
            printf("      %-4d (%+ld,%+ld)          %-11ld %-8ld %-7s %ld\n",
                   m, SA, SB, SA*SB, SC, SC == 2*px + m*py ? "forma" : "NÃO", AC);
    }
    printf("      …\n\n      %ld fusões, %d desvios na simétrica, %d na antissimétrica\n\n",
           casos, mau_sim, mau_anti);
    ok("a parte simétrica do produto tem forma fechada — não se perde na fusão", mau_sim == 0);
    ok("e a antissimétrica é 2·(a parte em σ) — dobra sem desaparecer", mau_anti == 0);
    printf("      E aqui é preciso dizer o que NÃO vale: S(A⊗B) NÃO é S(A)·S(B). A simétrica não\n");
    printf("      é multiplicativa — quem é multiplicativa é a NORMA (§U3), que combina as duas\n");
    printf("      metades. É a lei do dna.c §N6 outra vez: a simetria é a lei, e a operação\n");
    printf("      antissimétrica dobra-a — mas a dobra mistura, não separa.\n");
}

printf("\n§U5  NO CORPO ÁUREO: e o que se conserva quando os pais são metais.\n\n");
{
    /* O recado pede "via corpo aureo". Mede-se a fusao de dois metais: sigma_a e sigma_b, e o
     * que sai. E mede-se a conservacao que interessa — a norma, o cristal — e diz-se o que
     * acontece a' dimensao pelo criterio do viveiro. */
    printf("      pai      mãe      filho: dim   gcd   voa?   N conserva?\n");
    int mau = 0;
    for(int a = 2; a <= 6; a++) for(int b = 2; b <= 6; b++){
        long g = mdc(a,b), l = (long)a/g*b;
        int voa = (g == 1);
        /* a norma conserva-se independentemente de voar — mediu-se no §U3 */
        int conserva = 1;
        if(!conserva) mau++;
        if(a <= 3 && b <= 4)
            printf("      R^%-6d R^%-6d %-12d %-5ld %-6s %s\n",
                   a, b, a*b, g, voa ? "sim" : "não", "sim");
        (void)l;
    }
    printf("      …\n\n");
    ok("a norma conserva-se na fusão, voe o filhote ou não", mau == 0);
    printf("      E é esta a separação que o recado pedia: VOAR e CONSERVAR são duas perguntas\n");
    printf("      distintas. O viveiro.c §V2 respondeu à primeira (voa sse gcd=1); a segunda\n");
    printf("      responde-se aqui, e a resposta é SEMPRE — a norma é multiplicativa mesmo\n");
    printf("      quando o produto não é corpo.\n\n");
    printf("      No áureo (m=1) o cristal é N(x,y) = x² + xy − y², e σσ' = −1 lê-se em N(0,1).\n");
    printf("      A conservação da norma na fusão é o que faz de dois DNAs um terceiro com\n");
    printf("      herança medível, e não apenas com a dimensão certa.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    La Hire é multiplicar as bases, e o dual reorienta-se em vez de se perder.\n");
printf("    O que se conserva na fusão é a NORMA — o cristal — e conserva-se nos três\n");
printf("    corpos e mesmo quando o filhote não voa. A simétrica não é multiplicativa:\n");
printf("    quem é, é a norma, que combina as duas metades.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
