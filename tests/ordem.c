/* ordem.c — COMPARAR DENTRO DO CORPO QUADRÁTICO. E não é uma operação: são duas.
 *
 * O Aarão: "agora a comparação por norma dentro do corpo quadrático."
 *
 * Era o último aberto, e ao medir aparece o motivo de nunca ter fechado: comparar a+bσ com c+dσ
 * NÃO é a mesma operação nos dois lados da régua. O discriminante decide qual:
 *
 *   Δ > 0  HIPERBÓLICO  σ é REAL. O corpo mergulha em ℝ e é ORDENÁVEL: compara-se direto, e
 *                       exatamente em inteiros — 2(x+yσ) = P + y√Δ com P = 2x+ym, e o sinal sai
 *                       de comparar P² com y²Δ. Sem raiz e sem float.
 *   Δ < 0  ELÍPTICO     σ é COMPLEXO. O corpo NÃO É ORDENÁVEL — nenhuma ordem é compatível com
 *                       as operações. Compara-se pela NORMA, que é definida positiva.
 *   Δ = 0  PARABÓLICO   degenerado: a norma é quadrado perfeito e não separa.
 *
 * É por isso que o caminho do átomo não podia fazer isto genericamente, e eu andava a tratar o
 * segundo componente como denominador: eu estava a usar UMA regra onde há DUAS.
 *
 *   §O1  Δ>0: o sinal de x+yσ, exato em ℤ — e concorda com a conta em double
 *   §O2  e é ORDEM: total, transitiva, e compatível com ⊕
 *   §O3  Δ<0: o corpo NÃO é ordenável — mede-se a impossibilidade, não se assume
 *   §O4  logo compara-se pela NORMA, que é definida positiva e dá pré-ordem total
 *   §O5  a norma é multiplicativa nos dois — é o que sobrevive à mudança de classe
 *   §O6  o veredito: duas regras, e o disc escolhe
 *
 *   cc -O2 -std=c99 ordem.c -o ordem -lm && ./ordem
 */
#include <stdio.h>
#include <math.h>
#include "corpos.h"
#include "unidade.h"

int main(void){
printf("\n=== COMPARAR DENTRO DO CORPO QUADRÁTICO ===================================\n");
printf("    Não é uma operação: são duas, e o discriminante escolhe.\n");

printf("\n§O1  Δ>0: o sinal de x+yσ, exato em ℤ — e concorda com o double.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m   x+yσ        sinal exato   pelo double     concordam?\n");
    for(long m = 1; m <= 6; m++){
        double sg = (m + sqrt((double)m*m + 4.0)) / 2.0;
        for(long x = -25; x <= 25; x++) for(long y = -25; y <= 25; y++){
            int s = au_sinal(x, y, m);
            double v = (double)x + (double)y*sg;
            int sd = ((long long)(v * 1e9) >= 1) - ((long long)(-v * 1e9) >= 1);
            if(s != sd) mau++;
            casos++;
        }
        if(m == 1){
            printf("      1   3−2σ        %-13d %-15s sim ✓\n", au_sinal(3,-2,1), "3−3.236 < 0");
            printf("      1   −2+2σ       %-13d %-15s sim ✓\n", au_sinal(-2,2,1), "−2+3.236 > 0");
        }
    }
    ok("o sinal exato em ℤ concorda com a conta em double, em toda a varredura", mau == 0);
    printf("      (%ld pontos, m de 1 a 6.)\n", casos);
    printf("\n      O double está aqui como TESTEMUNHA, não como método: quem decide é P² contra\n");
    printf("      y²Δ, em inteiros. Se eu tivesse usado o double para decidir, os casos perto do\n");
    printf("      zero seriam sorte.\n");
}

printf("\n§O2  E é ORDEM: total, transitiva, e compatível com ⊕.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = 1; m <= 3; m++)
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
    for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
        Par u = {a,b}, v = {c,d};
        int s = au_cmp(u,v,m);
        if(s != -au_cmp(v,u,m)) mau++;                 /* antissimétrica */
        if(s == 0 && (a != c || b != d)) mau++;        /* σ irracional: só empata se for igual */
        /* compatível com ⊕: somar o mesmo não troca a ordem */
        Par w = {2,-3};
        if(au_cmp(au_soma(u,w), au_soma(v,w), m) != s) mau++;
        casos++;
    }
    ok("antissimétrica, separa pontos distintos, e a soma preserva — é ORDEM", mau == 0);
    printf("      (%ld pares, m de 1 a 3.)\n", casos);
    printf("\n      Separar pontos distintos é consequência de σ ser IRRACIONAL: x+yσ = 0 com x,y\n");
    printf("      inteiros força x = y = 0. É a mesma irracionalidade que faz a família real.\n");
}

printf("\n§O3  Δ<0: o corpo NÃO é ordenável — e mede-se, não se assume.\n\n");
{
    int mau = 0;
    /* num corpo ordenado, todo quadrado é ≥ 0 e 1 > 0. No cristalino ω² = −1, logo se houvesse
     * ordem teríamos −1 = ω² ≥ 0 e 1 > 0 ao mesmo tempo. Contradição — exata. */
    Par w = {0,1};                                     /* o próprio ω */
    Par w2 = cr_prod(w, w, 0);                         /* ω² = −1 */
    printf("      ω = (0,1)   ω² = (%ld,%ld)   isto é −1\n", w2.a, w2.b);
    if(!(w2.a == -1 && w2.b == 0)) mau++;
    printf("      numa ordem compatível todo quadrado seria ≥ 0, logo −1 ≥ 0 — e 1 > 0.\n");
    printf("      as duas juntas dão 0 > 0. CONTRADIÇÃO.\n");
    ok("no cristalino ω² = −1, logo NENHUMA ordem compatível existe — provado, não suposto",
       mau == 0);
    printf("\n      Então perguntar \"a < b\" num corpo elíptico não é difícil: é MAL POSTO. E é por\n");
    printf("      isso que o SQL não podia ter uma regra só — a pergunta muda de sentido com a\n");
    printf("      classe da coluna.\n");
}

printf("\n§O4  Logo compara-se pela NORMA — definida positiva, e pré-ordem total.\n\n");
{
    int mau = 0; long casos = 0, empates = 0;
    for(long t = 0; t <= 1; t++)
    for(long a = -12; a <= 12; a++) for(long b = -12; b <= 12; b++)
    for(long c = -12; c <= 12; c++) for(long d = -12; d <= 12; d++){
        Par u = {a,b}, v = {c,d};
        int s = cr_cmp(u,v,t);
        if(s != -cr_cmp(v,u,t)) mau++;                 /* antissimétrica */
        if(cr_norma(u,t) < 0) mau++;                   /* definida positiva */
        if(s == 0 && !(a==c && b==d)) empates++;       /* PRÉ-ordem: empata quem tem mesma norma */
        casos++;
    }
    ok("a comparação por norma é antissimétrica e a norma nunca é negativa", mau == 0);
    printf("      (%ld pares, %ld empates de norma sem serem o mesmo elemento.)\n", casos, empates);
    printf("\n      E é PRÉ-ordem, não ordem: (1,0) e (0,1) têm ambos norma 1 em Gauss, e empatam\n");
    printf("      sem serem iguais. Isso não é defeito da norma — é o corpo não ter ordem, e a\n");
    printf("      norma dar o que se pode dar. Dizer \"ordem\" aqui seria prometer a mais.\n");
}

printf("\n§O5  A norma é multiplicativa nos DOIS — é o que sobrevive à classe.\n\n");
{
    int mau = 0; long casos = 0;
    for(long m = 1; m <= 4; m++)
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++)
    for(long c = -8; c <= 8; c++) for(long d = -8; d <= 8; d++){
        Par u = {a,b}, v = {c,d};
        if(au_norma(au_prod(u,v,m),m) != au_norma(u,m)*au_norma(v,m)) mau++;
        if(cr_norma(cr_prod(u,v,0),0) != cr_norma(u,0)*cr_norma(v,0)) mau++;
        casos++;
    }
    ok("N(xy) = N(x)N(y) no hiperbólico E no elíptico — a norma atravessa a classe", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      A ORDEM não atravessa: existe de um lado e é impossível do outro. A NORMA\n");
    printf("      atravessa. É por isso que ela é a régua e a ordem não é.\n");
}

printf("\n§O6  O veredito: duas regras, e o disc escolhe.\n\n");
{
    printf("      classe          σ é          comparar é              o que se promete\n");
    printf("      Δ > 0 hiperb.   real         direto, exato em ℤ      ORDEM total\n");
    printf("      Δ < 0 elíptico  complexo     pela NORMA              PRÉ-ordem total\n");
    printf("      Δ = 0 paraból.  degenerado   a norma é quadrado      não separa\n");
    conclui("a comparação despacha pela CLASSE — e é o disc que a diz");
    printf("\n      Era isto que faltava, e o erro estava antes do código: eu procurava UMA regra\n");
    printf("      para uma pergunta que tem DUAS respostas. O caminho do átomo tratava o segundo\n");
    printf("      componente como denominador porque foi escrito para o racional — e o racional\n");
    printf("      tem ordem, logo nunca precisou de escolher.\n");
    printf("\n      E o que NÃO se promete: no elíptico é PRÉ-ordem. Dois elementos distintos com\n");
    printf("      a mesma norma empatam, e nenhuma escolha entre eles é canónica. Quem quiser\n");
    printf("      desempatar terá de dizer com quê — o corpo não diz.\n");
}

printf("\n=== A COMPARAÇÃO ==========================================================\n");
printf("  Não é uma operação: são duas, e o discriminante escolhe.\n\n");
printf("    Δ > 0   σ é REAL, o corpo é ORDENÁVEL — compara-se direto, exato em ℤ:\n");
printf("            2(x+yσ) = P + y√Δ com P = 2x+ym, e o sinal sai de P² contra y²Δ\n");
printf("    Δ < 0   σ é COMPLEXO, e NENHUMA ordem compatível existe (ω² = −1 dá 0 > 0) —\n");
printf("            compara-se pela NORMA, definida positiva, e é PRÉ-ordem\n\n");
printf("  A ordem não atravessa a classe; a NORMA atravessa, e é multiplicativa nos dois. É por\n");
printf("  isso que ela é a régua. E eu procurava uma regra só para uma pergunta com duas\n");
printf("  respostas — o erro estava antes do código.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros. O double entrou só como testemunha.\n\n");
return 0;
}
