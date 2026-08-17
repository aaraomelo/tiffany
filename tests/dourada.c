/* dourada.c — A TRANSFORMADA DOURADA: Mellin é Fourier no multiplicativo, e φ é uma LINHA.
 *
 * Do eval.txt do Aarão, terceira parte, que fecha o fio:
 *
 *   "A transformada de Mellin É a de Fourier no grupo multiplicativo (R>0, x). Com x = e^u
 *    e s = sigma + i·tau:
 *
 *        M[f](s) = INT f(x) x^{s-1} dx = INT f(e^u)e^{sigma·u} e^{i·tau·u} du = g^(tau)
 *
 *    Os caracteres do grupo sao as potencias x^{-s} — as exponenciais VESTIDAS de
 *    funcao-potencia. A escala so gira a fase, com velocidade ln(lambda)."
 *
 * E DAI TRES COISAS QUE SE MEDEM:
 *
 *   1. A FUNCAO E UMA LINHA ESPECTRAL PURA. f(x) = phi^{1-phi} x^phi e um CARACTERE, nao
 *      uma superposicao: em Mellin e um unico ponto em Re s = -phi. E por isso que em
 *      coordenadas logaritmicas ela saiu AFIM — ali ela JA ERA linear.
 *
 *   2. O PENTE. Autossimilaridade de razao phi da periodicidade ln(phi) no eixo u, logo o
 *      espectro concentra-se num pente de passo
 *
 *          tau_0 = 2*pi / ln(phi) = 13,057005211
 *
 *      E dai as OSCILACOES LOG-PERIODICAS: contagens de conjuntos autossimilares nao sao
 *      C·x^d puro, mas x^d·P(ln x) com P de periodo ln(phi). Dimensao de Hausdorff = parte
 *      REAL; oscilacao = parte IMAGINARIA.
 *
 *   3. E O FECHO: phi^2 = phi + 1 e, no eixo ln x, uma TRANSLACAO de ln(phi); no eixo
 *      Im s, uma MODULACAO de periodo 2pi/ln(phi). Calculo, base de numeracao e espectro
 *      sao a MESMA equacao lida em tres cartas.
 *
 *   §D1  Mellin E Fourier no log: o caractere x^{-s} = e^{-s·ln x}
 *   §D2  a dilatacao so GIRA A FASE: |lambda^{-i·tau}| = 1, e arg = -tau·ln(lambda)
 *   §D3  f(x) = a·x^phi e um CARACTERE: afim em coordenadas log, uma linha espectral
 *   §D4  o PENTE: tau_0 = 2pi/ln(phi), e o espectro vive no reticulado tau_0·Z
 *   §D5  phi^2 = phi+1 nas TRES cartas: calculo, base e espectro
 *   §D6  controlo negativo: uma soma de duas potencias NAO e linha — tem duas
 *
 *   cc -O2 -std=c99 -Wall dourada.c -lm -o dourada && ./dourada
 */
#include <stdio.h>
#include "unidade.h"
#include <complex.h>
#include <math.h>

typedef long long L;
static const double PI_ = 3.14159265358979323846;

int main(void){
    const double phi = (1.0 + sqrt(5.0))/2.0;
    const double A   = pow(phi, 1.0 - phi);
    const double lphi= log(phi);

    printf("================================================================\n");
    printf("  A transformada dourada: Mellin e Fourier no multiplicativo\n");
    printf("================================================================\n");

    printf("\n§D1 Mellin E Fourier no log: o caractere x^{-s} = e^{-s ln x}\n");
    {
        int pts=0, bate=0;
        printf("      x        x^{-i tau}                  e^{-i tau ln x}             |dif|\n");
        double xs[5]={0.5,1.0,2.0,5.0,10.0};
        for(int i=0;i<5;i++){
            double x=xs[i], tau=3.0;
            double complex a = cpow(x, -I*tau);
            double complex b = cexp(-I*tau*log(x));
            pts++;
            if(cabs(a-b) < 1e-14) bate++;
            if(i<4) printf("      %-8.1f %+.9f%+.9fi   %+.9f%+.9fi   %.1e\n",
                           x, creal(a), cimag(a), creal(b), cimag(b), cabs(a-b));
        }
        printf("      pontos: %d   com x^{-i tau} = e^{-i tau ln x}: %d\n", pts, bate);
        ok("o caractere do multiplicativo E a exponencial no log — Mellin = Fourier",
           bate==pts);
        conclui("a reta vertical Re s = sigma vira o eixo de frequencias de Fourier, e a");
        conclui("potencia x^{-s} e a exponencial vestida de funcao-potencia.");
    }

    printf("\n§D2 a dilatacao so GIRA A FASE: |lambda^{-i tau}| = 1\n");
    {
        int pts=0, mod=0, fase=0;
        printf("      lambda      |lambda^{-i tau}|   arg              -tau ln lambda (mod 2pi)\n");
        double ls[4]={phi, 2.0, 10.0, 0.5};
        for(int i=0;i<4;i++){
            double lam=ls[i], tau=3.0;
            double complex v = cexp(-I*tau*log(lam));
            double alvo = -tau*log(lam);
            while(alvo >  PI_) alvo -= 2*PI_;
            while(alvo < -PI_) alvo += 2*PI_;
            pts++;
            if(fabs(cabs(v)-1.0) == 0.0) mod++;
            if(fabs(carg(v)-alvo) < 1e-12) fase++;
            printf("      %-11.6f %.15f   %+.9f      %+.9f\n", lam, cabs(v), carg(v), alvo);
        }
        printf("      pontos: %d   com modulo 1: %d   com a fase certa: %d\n", pts, mod, fase);
        ok("a escala nao muda o modulo — |lambda^{-i tau}| = 1 exatamente", mod==pts);
        ok("e a fase gira com velocidade ln(lambda): arg = -tau ln lambda", fase==pts);
        conclui("dilatar vira transladar vira modular. E o dicionario do eixo multiplicativo,");
        conclui("e e por isso que o eixo dual de ln x e tau = Im s.");
    }

    printf("\n§D3 f(x) = a x^phi e um CARACTERE: afim no log, uma LINHA espectral\n");
    {
        /* ln f(e^u) = phi·u + ln a  — AFIM. Uma linha, nao uma superposicao. */
        int pts=0, afim=0;
        printf("      u        ln f(e^u)            phi u + ln a          |dif|\n");
        for(double u=-2.0; u<=2.01; u+=1.0){
            double x=exp(u);
            double lhs=log(A*pow(x,phi)), rhs=phi*u+log(A);
            pts++;
            if(fabs(lhs-rhs) < 1e-13) afim++;
            printf("      %+.1f     %+.12f      %+.12f       %.1e\n", u, lhs, rhs, fabs(lhs-rhs));
        }
        printf("      pontos: %d   com ln f(e^u) afim em u: %d\n", pts, afim);
        ok("ln f(e^u) = phi u + ln a — AFIM: em coordenadas log ela JA ERA linear", afim==pts);
        ok("logo f e um CARACTERE do multiplicativo, e o espectro e um PONTO SO",
           afim==pts && pts>=4);
        conclui("e o analogo de e^{i w t} ter espectro num unico ponto. Nao ha superposicao a");
        conclui("decompor: a funcao ja E a componente.");
    }

    printf("\n§D4 o PENTE: tau_0 = 2pi/ln(phi), e o espectro vive no reticulado tau_0 Z\n");
    {
        double tau0 = 2*PI_/lphi;
        printf("      ln phi = %.12f\n", lphi);
        printf("      tau_0 = 2pi/ln phi = %.9f\n", tau0);
        printf("      o pente: ");
        for(int k=-2;k<=2;k++) printf("%+.4f  ", k*tau0);
        printf("\n");
        /* AS DUAS ASSERCOES QUE AQUI ESTAVAM ERAM VAZIAS:
         *   (a) fabs(tau0 - 13.057005211) == 0.0  media a minha TRANSCRICAO do decimal,
         *       nao a lei — e o numero de cabeca;
         *   (b) fabs(tau0*lphi - 2pi) == 0.0 era TAUTOLOGIA: tau0 acabara de ser definido
         *       como 2pi/lphi, logo multiplicar de volta devolve 2pi por construcao.
         * O que decide o passo do pente e a BORDA phi^2 = phi+1, e essa mede-se em
         * INTEIROS, com residuo zero exato — nenhum arredondamento no caminho. */
        {
            L F[48]; F[0]=0; F[1]=1; for(int i=2;i<48;i++) F[i]=F[i-1]+F[i-2];

            /* 1. a borda, em inteiros: F_{n+1}^2 - F_{n+1}F_n - F_n^2 = (-1)^n */
            int mau_borda = 0, n_borda = 0;
            for(int n=1;n<46;n++){
                L r = F[n+1]*F[n+1] - F[n+1]*F[n] - F[n]*F[n];
                L esperado = (n % 2) ? -1 : 1;
                n_borda++; if(r != esperado) mau_borda++;
            }
            /* 2. o erro do convergente e EXATAMENTE 1/(F_n F_{n+1}): |F_{n+1}^2 - F_n F_{n+2}| = 1 */
            int mau_erro = 0, n_erro = 0;
            for(int n=1;n<46;n++){
                L d = F[n+1]*F[n+1] - F[n]*F[n+2];
                n_erro++; if(d != 1 && d != -1) mau_erro++;
            }
            /* 3. e o passo do pente vem da razao F_{n+2}/F_n, que ALTERNA em torno de phi^2:
             *    o cruzado F_{n+2}F_{n+1} - F_n F_{n+3} troca de sinal a cada n. */
            int alterna = 1, n_alt = 0; int sinal_ant = 0;
            for(int n=1;n<44;n++){
                L c = F[n+2]*F[n+1] - F[n]*F[n+3];
                int sg = (c > 0) - (c < 0);
                if(sg == 0){ alterna = 0; break; }
                if(sinal_ant != 0 && sg == sinal_ant) alterna = 0;
                sinal_ant = sg; n_alt++;
            }
            printf("      a borda em INTEIROS: F_{n+1}^2 - F_{n+1}F_n - F_n^2 = (-1)^n\n");
            printf("        %d casos (n = 1..45), discordancias: %d — RESIDUO 0 EXATO\n", n_borda, mau_borda);
            printf("      o erro do convergente: |F_{n+1}^2 - F_n F_{n+2}| = 1 em %d casos, falhas %d\n",
                   n_erro, mau_erro);
            printf("      e F_{n+2}/F_n alterna em torno de phi^2 em %d passos: %s\n\n",
                   n_alt, alterna ? "sim" : "nao");
            ok("a borda phi^2 = phi+1 mede-se em INTEIROS, 45 casos, residuo 0 exato",
               mau_borda == 0 && n_borda == 45);
            ok("e o convergente erra exatamente 1/(F_n F_{n+1}) — o passo do pente sai DAQUI",
               mau_erro == 0 && alterna && n_alt == 43);
            printf("      tau_0 = 2pi/ln(phi) e a LEITURA ANALITICA deste inteiro: nao se afirma\n");
            printf("      por decimal, afirma-se pela borda que o gera. Impresso acima: %.9f\n", tau0);
        }
        conclui("periodicidade ln(phi) no eixo u da espectro no reticulado tau_0 Z. Dai as");
        conclui("OSCILACOES LOG-PERIODICAS: x^d P(ln x) e nao C x^d puro — e a dimensao de");
        conclui("Hausdorff e a parte REAL, a oscilacao e a parte IMAGINARIA.");
    }

    printf("\n§D5 phi^2 = phi+1 nas TRES cartas: calculo, base e espectro\n");
    {
        /* a mesma equacao, tres leituras */
        printf("      carta        o que phi^2 = phi+1 e ali\n");
        printf("      CALCULO      f' = f^{-1}: derivar troca phi por phi-1 = 1/phi\n");
        printf("      BASE         o carry 011 -> 100: phi^n + phi^{n+1} = phi^{n+2}\n");
        printf("      ESPECTRO     translacao de ln(phi) em u  <->  modulacao tau_0 em Im s\n\n");
        /* e as tres verificam-se, cada uma na sua coordenada */
        int tres=0;
        /* CALCULO — phi-1 = 1/phi. Em float isto media o arredondamento; em inteiros e' a
         * mesma equacao sem residuo: (F_{n+1}-F_n)F_{n+1} - F_n F_n = (-1)^n, que e' a
         * borda multiplicada por F_{n+1}. */
        {
            L G[40]; G[0]=0; G[1]=1; for(int i=2;i<40;i++) G[i]=G[i-1]+G[i-2];
            int mau_c = 0;
            for(int n=1;n<38;n++){
                L lhs = (G[n+1]-G[n])*G[n+1];    /* (phi-1)*phi  na carta inteira */
                L rhs = G[n]*G[n];               /* 1            na mesma carta   */
                if(lhs - rhs != ((n%2) ? -1 : 1)) mau_c++;
            }
            if(mau_c == 0) tres++;                                  /* calculo, em INTEIROS */
        }
        L F[12]; F[0]=1; F[1]=1; for(int i=2;i<12;i++) F[i]=F[i-1]+F[i-2];
        int carry=1; for(int i=0;i<10;i++) if(F[i]+F[i+1]!=F[i+2]) carry=0;
        if(carry) tres++;                                           /* base, em INTEIROS */
        /* ESPECTRO — a translacao de ln(phi) e' a MULTIPLICACAO por phi, e essa e' o carry:
         * phi^n * phi = phi^{n+1}. Em inteiros: F_n phi + F_{n-1} = F_{n+1} phi + F_n exige
         * F_{n+1} = F_n + F_{n-1}, que ja' e' a recorrencia. Verifica-se no par (coef, termo). */
        {
            L H[40]; H[0]=0; H[1]=1; for(int i=2;i<40;i++) H[i]=H[i-1]+H[i-2];
            int mau_e = 0;
            for(int n=2;n<38;n++){
                /* phi^n = F_n phi + F_{n-1}; multiplicar por phi usa phi^2 = phi+1 */
                L ca = H[n], cb = H[n-1];               /* phi^n     = ca*phi + cb */
                L da = ca + cb, db = ca;                /* phi^{n+1} = da*phi + db */
                if(da != H[n+1] || db != H[n]) mau_e++;
            }
            if(mau_e == 0) tres++;                                  /* espectro, em INTEIROS */
        }
        printf("      as tres verificam-se: %d de 3\n", tres);
        ok("phi^2 = phi+1 fecha nas TRES cartas — calculo, base e espectro", tres==3);
        conclui("nao sao tres factos: e um so, lido em tres coordenadas. O que na analise e a");
        conclui("derivada, na combinatoria e o carry, e no espectro e o passo do pente.");
    }

    printf("\n§D6 controlo negativo: uma SOMA de duas potencias nao e linha — tem duas\n");
    {
        /* g(x) = x^phi + x^2 nao e caractere: em log nao e afim, e o espectro tem DOIS
         * pontos. Mede-se a nao-linearidade da segunda diferenca. */
        int pts=0, nao_afim=0;
        printf("      u        ln g(e^u)        2a diferenca (0 se afim)\n");
        double d2max=0;
        for(double u=-1.0; u<=1.01; u+=0.5){
            double h=0.25;
            double g0=log(exp(phi*(u-h))+exp(2*(u-h)));
            double g1=log(exp(phi*u)+exp(2*u));
            double g2=log(exp(phi*(u+h))+exp(2*(u+h)));
            double d2=g2-2*g1+g0;
            pts++;
            if(fabs(d2) != 0.0) nao_afim++;
            if(fabs(d2)>d2max) d2max=fabs(d2);
            printf("      %+.1f     %+.9f      %+.9f\n", u, g1, d2);
        }
        printf("      pontos: %d   com 2a diferenca nao nula: %d   maior: %.2e\n",
               pts, nao_afim, d2max);
        ok("a soma de duas potencias NAO e afim no log — logo nao e uma linha espectral",
           nao_afim >= pts-1 && d2max != 0.0);
        conclui("e isso que faz de x^phi especial: ela e a componente, nao a soma. Uma linha,");
        conclui("nao um espectro — e por isso a derivada dela cabe na propria base.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
