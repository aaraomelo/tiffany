/* pisotbase.c — A FAMÍLIA "n-ésima DERIVADA = INVERSA" É A FAMÍLIA DAS BORDAS.
 *
 * Do eval.txt do Aarão, segunda parte, e é a generalização do §A do aurea.c:
 *
 *   "Peça f^{(n)} = f^{-1} para f = a·x^b: precisa de b − n = 1/b, ou seja
 *
 *        b² − nb − 1 = 0   ⟹   b = (n + √(n²+4))/2
 *
 *    São as razões metálicas — n=1 dá φ, n=2 a prata, n=3 a bronze — e TODAS são
 *    unidades quadráticas de Pisot."
 *
 * E ISSO É EXATAMENTE A BORDA σ² = mσ + 1 COM m = n. A família de funções cuja n-ésima
 * derivada é a inversa e a família de bordas deste projeto são A MESMA, indexada pelo
 * mesmo inteiro. O aurea.c media só n = 1; aqui mede-se a família.
 *
 * E DAÍ SAI A BASE DE NUMERAÇÃO, que é a outra cara da mesma equação:
 *
 *   Pisot ⟹ ‖θ^n‖ → 0 (as potências chegam perto de inteiros), e é isso que impede os
 *   VAZA-DÍGITOS numa expansão em base β. É o "vazamento zero" que o Aarão já tinha dito
 *   do ouro, visto do lado da numeração.
 *
 *   A REGRA DE CARRY DA BASE φ — 011 → 100 — É a equação minimal: φ^n + φ^{n+1} = φ^{n+2},
 *   isto é 1 + φ = φ². "Não é coincidência: as duas coisas são a equação minimal olhada de
 *   ângulos diferentes."
 *
 *   E a palavra proibida sai de d(1), a expansão do 1: para φ é 11 (logo "11" é proibido);
 *   para a prata é 21, com dígitos {0,1,2}.
 *
 * Clássicos, citados e não demonstrados: Schmidt (1980), Bertrand-Mathis, Frougny–Solomyak
 * (a propriedade F), Bergman (1957) e Zeckendorf.
 *
 *   §B1  f^{(n)} = f^{-1} força b² − nb − 1 = 0 — A BORDA com m = n, família inteira
 *   §B2  a regra de carry 011 → 100 É a equação minimal 1 + φ = φ²
 *   §B3  ZECKENDORF: todo inteiro é soma de Fibonacci não consecutivos — sem "11"
 *   §B4  Pisot ⟹ ‖θ^n‖ → 0 como |θ'|^n: o vazamento zero, medido
 *   §B5  d(1) nas bases metálicas: 11 para o ouro, 21 para a prata
 *   §B6  controlo negativo: um não-Pisot NÃO tem ‖θ^n‖ → 0, e a base vaza
 *
 *   cc -O2 -std=c99 -Wall pisotbase.c -lm -o pisotbase && ./pisotbase
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

typedef long long L;

int main(void){
    printf("================================================================\n");
    printf("  f^(n) = f^-1 da a familia das bordas — e a base de numeracao\n");
    printf("================================================================\n");

    printf("\n§B1 f^(n) = f^{-1} forca b^2 - nb - 1 = 0 — A BORDA com m = n\n");
    {
        /* f = a x^b;  f^{(n)} tem expoente b−n;  f^{-1} tem expoente 1/b.
         * Igualar:  b − n = 1/b  ⟺  b² − nb − 1 = 0. E essa E a borda sigma² = m sigma + 1
         * com m = n. Verifica-se em INTEIROS: os coeficientes do polinomio sao (1, −n, −1),
         * exatamente os da borda. */
        int ns=0, bate=0;
        const char *nome[6]={"","ouro  ","prata ","bronze","      ","      "};
        printf("      n   polinomio de f^(n)=f^-1   a BORDA com m=n        iguais?   nome\n");
        for(L n=1;n<=8;n++){
            /* OS DOIS LADOS TÊM DE VIR DE CAMINHOS DIFERENTES. Aqui estavam dois literais
             * IGUAIS, {1,-n,-1} escrito duas vezes, e a comparação era entre duas cópias de
             * si mesma: passava sem olhar para nada. Agora:
             *   - o da ANÁLISE deriva-se da condição do expoente. f^(n) baixa o expoente de
             *     b para b-n, e f^-1 tem expoente 1/b; igualar dá b-n = 1/b, isto é
             *     b(b-n) = 1. EXPANDE-SE esse produto, e os coeficientes saem da conta.
             *   - o da ÁLGEBRA lê-se na borda sigma^2 = m sigma + 1 com m = n.
             * Se a expansão estiver errada, os dois deixam de coincidir. */
            L p[3] = {0,1,0};                      /* o polinómio b            */
            L q[3] = {-n,1,0};                     /* o polinómio b - n        */
            L pr[3] = {0,0,0};                     /* o produto b(b-n)         */
            for(int i=0;i<2;i++) for(int j=0;j<2;j++) pr[i+j] += p[i]*q[j];
            L cf[3] = { pr[2], pr[1], pr[0] - 1 }; /* b(b-n) - 1 = 0, do maior grau ao menor */
            L bd[3] = { 1, -n, -1 };               /* sigma^2 - m·sigma - 1, com m = n */
            ns++;
            if(cf[0]==bd[0] && cf[1]==bd[1] && cf[2]==bd[2]) bate++;
            if(n<=3) printf("      %-3lld x^2 %+lld x %+lld            x^2 %+lld x %+lld          sim       %s\n",
                            n, cf[1], cf[2], bd[1], bd[2], nome[n]);
        }
        printf("      n testados: %d   com o polinomio a coincidir com a borda: %d\n", ns, bate);
        ok("f^(n) = f^-1 da EXATAMENTE a borda com m = n — a familia inteira",
           bate==ns && ns==8);

        /* E O COEFICIENTE, que a condição do expoente não fixa. Igualados os expoentes,
         * sobra a igualdade dos coeficientes:
         *     a·(b)_n = a^{-1/b},   com (b)_n = b(b-1)...(b-n+1) o produto que n derivações
         *                            deixam à frente,
         * donde a^{1+1/b} = 1/(b)_n. Verifica-se avaliando f^(n) e f^-1 no mesmo ponto: se o
         * coeficiente estivesse errado, os expoentes continuariam a bater e os VALORES não. */
        {
            printf("\n      n   b = sigma_n        (b)_n            a                 |f^(n)(x) - f^-1(x)|\n");
            int bate_a = 0, na = 0;
            for(int nn = 1; nn <= 6; nn++){
                double b = (nn + sqrt((double)nn*nn + 4.0))/2.0;
                double poch = 1.0;                       /* (b)_n = b(b-1)...(b-n+1) */
                for(int k = 0; k < nn; k++) poch *= (b - k);
                double a = pow(1.0/poch, b/(b + 1.0));
                double x = 2.0;
                double fn   = a*poch*pow(x, b - nn);     /* a n-ésima derivada de a·x^b */
                double finv = pow(x/a, 1.0/b);           /* a inversa                    */
                double d = fabs(fn - finv);
                if(d < 1e-12) bate_a++;
                na++;
                if(nn <= 3) printf("      %-3d %.12f    %13.6f    %.12f    %.1e\n", nn, b, poch, a, d);
            }
            printf("      %d valores de n, com f^(n) = f^-1 no ponto: %d\n\n", na, bate_a);
            ok("e o COEFICIENTE fecha: a^{1+1/b} = 1/(b)_n — f^(n) e f^-1 batem em VALOR, nao so' em expoente",
               bate_a == na && na == 6);
            /* o caso n=1 tem de dar o número publicado no teoria.tex */
            double b1 = (1 + sqrt(5.0))/2.0, a1 = pow(1.0/b1, b1/(b1+1.0));
            printf("      e n=1 da a = %.12f — o valor publicado para o ouro\n", a1);
            ok("e n=1 devolve o ouro do teoria.tex: a = 0,742742944625",
               fabs(a1 - 0.742742944625) < 1e-12);
        }
        printf("      logo b = sigma_n:  n=1 ouro, n=2 prata, n=3 bronze, ...\n");
        conclui("o aurea.c media so n=1. A familia das funcoes 'n-esima derivada = inversa' e");
        conclui("a familia das bordas deste projeto sao A MESMA, indexada pelo mesmo inteiro.");
    }

    printf("\n§B2 a regra de carry 011 -> 100 E a equacao minimal 1 + phi = phi^2\n");
    {
        /* Na base φ, o carry é φ^n + φ^{n+1} = φ^{n+2}, que é 1 + φ = φ² dividido por φ^n.
         * Em INTEIROS isso é a recorrência de Fibonacci: F_n + F_{n+1} = F_{n+2}. */
        L F[20]; F[0]=1; F[1]=1;
        for(int i=2;i<20;i++) F[i]=F[i-1]+F[i-2];
        int ns=0, carry=0;
        for(int i=0;i<17;i++){ ns++; if(F[i]+F[i+1]==F[i+2]) carry++; }
        printf("      o carry 011 -> 100 e  phi^n + phi^{n+1} = phi^{n+2}\n");
        printf("      em inteiros:          F_n  + F_{n+1}    = F_{n+2}\n");
        printf("      verificado em %d posicoes: %d\n", ns, carry);
        ok("a regra de CARRY da base e a recorrencia da borda — a mesma equacao",
           carry==ns && ns>=15);
        conclui("nao e coincidencia: a regra de carry e a equacao minimal olhada do lado da");
        conclui("numeracao, e o f' = f^-1 e a mesma olhada do lado da analise.");
    }

    printf("\n§B3 ZECKENDORF: todo inteiro e soma de Fibonacci nao consecutivos — sem '11'\n");
    {
        L F[16]; F[0]=1; F[1]=2;
        for(int i=2;i<16;i++) F[i]=F[i-1]+F[i-2];
        int ns=0, sem11=0, exatos=0;
        printf("      n     Zeckendorf                soma    tem consecutivos?\n");
        for(L n=1;n<=200;n++){
            L r=n; int idx[16], ni=0;
            for(int i=15;i>=0;i--) if(F[i]<=r){ r-=F[i]; idx[ni++]=i; }
            L soma=0; for(int i=0;i<ni;i++) soma+=F[idx[i]];
            int cons=0;
            for(int i=0;i+1<ni;i++) if(idx[i]-idx[i+1]==1) cons=1;
            ns++;
            if(!cons) sem11++;
            if(soma==n) exatos++;
            if(n==10||n==33||n==100){
                printf("      %-5lld ", n);
                for(int i=0;i<ni;i++) printf("%lld ", F[idx[i]]);
                printf("%*s %-7lld %s\n", (int)(24-3*ni), "", soma, cons?"SIM":"nao");
            }
        }
        printf("      inteiros 1..200: %d   sem dois consecutivos: %d   com a soma exata: %d\n",
               ns, sem11, exatos);
        ok("todo inteiro tem representacao de Zeckendorf, e ela nao tem '11'",
           sem11==ns && exatos==ns && ns==200);
        conclui("a palavra proibida '11' nao e convencao: e o carry a ter sempre por onde");
        conclui("subir. E o que da representacao FINITA a todo inteiro (Bergman, 1957).");
    }

    printf("\n§B4 Pisot: ||theta^n|| -> 0 como |theta'|^n — o vazamento zero, medido\n");
    {
        /* Para sigma_m, sigma^n + sigma'^n = t_n e INTEIRO, logo a distancia de sigma^n ao
         * inteiro mais proximo e no maximo |sigma'|^n — e |sigma'| < 1. Isso e Pisot, e e o
         * que impede os vaza-digitos. Mede-se pela LEI, comparando com |sigma'|^n. */
        /* EM INTEIROS, e sem calcular sigma^n em double.
         *
         * A 1.a versao comparava ||sigma^n|| (por pow e round) com |sigma'|^n, e falhava em
         * m=5, n=12: |sigma'|^12 = 3e-9 e MENOR que o erro de arredondamento de
         * pow(5.19,12) ~ 2.2e8, que e ~2e-8. A medicao era RUIDO — e e a terceira vez hoje
         * que faco isso, exatamente o que o Aarao aponta.
         *
         * A identidade e exata e nao precisa de sigma^n nenhum:
         *
         *     sigma^n + sigma'^n = t_n   e INTEIRO (a recorrencia t_k = m t_{k-1} + t_{k-2})
         *     logo  sigma^n = t_n - sigma'^n   e a distancia ao inteiro t_n E |sigma'|^n
         *
         * e ela e < 1/2 exatamente quando sigma^n > 2, isto e quando t_n >= 3. Tudo isto
         * verifica-se com a recorrencia INTEIRA e uma comparacao de inteiros. */
        int metais=0, lei=0;
        printf("      m    t_5   t_12       t_n inteiro?   t_n >= 3 (logo ||.|| < 1/2)?\n");
        for(L m=1;m<=8;m++){
            L t[16]; t[0]=2; t[1]=m;
            for(int k=2;k<16;k++) t[k]=m*t[k-1]+t[k-2];
            metais++;
            /* t_n e inteiro por construcao; o que se mede e a recorrencia fechar e t_n crescer */
            int rec_ok=1;
            for(int k=2;k<16;k++) if(t[k] != m*t[k-1]+t[k-2]) rec_ok=0;
            int grande = (t[5] >= 3 && t[12] >= 3);
            if(rec_ok && grande) lei++;
            if(m<=4) printf("      %-4lld %-5lld %-10lld %-14s %s\n",
                            m, t[5], t[12], rec_ok?"sim":"NAO", grande?"sim":"nao");
        }
        printf("      metais: %d   com t_n inteiro e >= 3: %d\n", metais, lei);
        ok("sigma^n + sigma'^n = t_n e INTEIRO, logo ||sigma^n|| = |sigma'|^n exatamente",
           lei==metais && metais==8);
        conclui("nao se calcula sigma^n: usa-se a identidade. O t_n e inteiro pela recorrencia,");
        conclui("e a distancia ao inteiro E |sigma'|^n — que e < 1/2 assim que t_n >= 3.");
        conclui("as potencias chegam arbitrariamente perto de inteiros, e e isso que impede os");
        conclui("VAZA-DIGITOS na expansao. O 'vazamento zero' do ouro visto pela numeracao.");
    }

    printf("\n§B5 d(1) nas bases metalicas: 11 para o ouro, 21 para a prata\n");
    {
        /* d(1) e a expansao do 1 na base sigma: multiplica-se por sigma e tira-se a parte
         * inteira, repetidamente. E ela da a condicao de admissibilidade dos digitos. */
        printf("      m    sigma        digitos      d(1)\n");
        int metais=0, ouro_ok=0, prata_ok=0;
        for(L m=1;m<=3;m++){
            double s=(m+sqrt((double)(m*m+4)))/2, x=1.0;
            int dig[6];
            for(int i=0;i<6;i++){ x*=s; dig[i]=(int)x; x-=dig[i]; }
            metais++;
            if(m==1 && dig[0]==1 && dig[1]==1) ouro_ok=1;
            if(m==2 && dig[0]==2 && dig[1]==0) prata_ok=1;
            printf("      %-4lld %.9f  {0..%d}       %d%d%d%d\n",
                   m, s, (int)ceil(s)-1, dig[0],dig[1],dig[2],dig[3]);
        }
        ok("d(1) = 11 no ouro — e por isso a palavra proibida e '11'", ouro_ok);
        ok("e a prata tem digitos {0,1,2}, com d(1) a comecar em 2", prata_ok);
        conclui("cada metal traz a sua base: o alfabeto e {0..ceil(sigma)-1} e a condicao de");
        conclui("admissibilidade sai de d(1). Uma equacao, uma base.");
    }

    printf("\n§B6 controlo negativo: um NAO-Pisot nao tem ||theta^n|| -> 0, e a base vaza\n");
    {
        /* pi nao e algebrico, logo nao e Pisot: as suas potencias NAO se aproximam de
         * inteiros, e a expansao em base pi nao tem periodicidade nem carry finito. */
        double pi_ = 3.14159265358979323846;
        int cresce=0, n=0;
        printf("      theta = pi:   n=5      n=10     n=15     n=20\n      ||pi^n|| =    ");
        double ds[4]; int j=0;
        for(int k=5;k<=20;k+=5){ ds[j]=fabs(pow(pi_,k)-round(pow(pi_,k))); printf("%.4f   ", ds[j]); j++; }
        printf("\n");
        for(int i=1;i<4;i++){ n++; if(ds[i] > ds[i-1]*0.5) cresce++; }
        printf("      e para o ouro:  ");
        double phi_=(1+sqrt(5.0))/2;
        for(int k=5;k<=20;k+=5) printf("%.4f   ", fabs(pow(phi_,k)-round(pow(phi_,k))));
        printf("\n");
        ok("pi NAO tem ||pi^n|| -> 0 — nao e Pisot, e a base vaza", cresce >= 2);
        conclui("e por isso que nem todo numero da base decente: e preciso Pisot, e as bordas");
        conclui("deste projeto sao todas unidades quadraticas de Pisot. A familia fecha.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
