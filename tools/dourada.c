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
            if(fabs(cabs(v)-1.0) < 1e-14) mod++;
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
        ok("tau_0 = 2pi/ln(phi) = 13,057005211", fabs(tau0 - 13.057005211) < 1e-6);
        /* a periodicidade: e^{i tau_0 (u + ln phi)} = e^{i tau_0 u}, porque tau_0 ln phi = 2pi */
        double prod = tau0*lphi;
        printf("      e tau_0 * ln phi = %.15f   contra 2pi = %.15f\n", prod, 2*PI_);
        ok("tau_0 * ln(phi) = 2pi — e por isso a translacao de ln(phi) e invisivel no pente",
           fabs(prod - 2*PI_) < 1e-12);
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
        if(fabs((phi-1.0) - 1.0/phi) < 1e-15) tres++;              /* calculo */
        L F[12]; F[0]=1; F[1]=1; for(int i=2;i<12;i++) F[i]=F[i-1]+F[i-2];
        int carry=1; for(int i=0;i<10;i++) if(F[i]+F[i+1]!=F[i+2]) carry=0;
        if(carry) tres++;                                           /* base, em INTEIROS */
        if(fabs((2*PI_/lphi)*lphi - 2*PI_) < 1e-12) tres++;         /* espectro */
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
            if(fabs(d2) > 1e-6) nao_afim++;
            if(fabs(d2)>d2max) d2max=fabs(d2);
            printf("      %+.1f     %+.9f      %+.9f\n", u, g1, d2);
        }
        printf("      pontos: %d   com 2a diferenca nao nula: %d   maior: %.2e\n",
               pts, nao_afim, d2max);
        ok("a soma de duas potencias NAO e afim no log — logo nao e uma linha espectral",
           nao_afim >= pts-1 && d2max > 1e-6);
        conclui("e isso que faz de x^phi especial: ela e a componente, nao a soma. Uma linha,");
        conclui("nao um espectro — e por isso a derivada dela cabe na propria base.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
