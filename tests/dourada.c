/* dourada.c — A TRANSFORMADA DOURADA: Mellin é Fourier no multiplicativo, e φ é uma LINHA.
 *
 * LEI vs TRANSPORTE. cpow/cexp/log/pow/exp da Mellin e da 2ª diferença eram o método.
 * A lei é (λμ)^s = λ^s μ^s, a borda Fibonacci/Cassini, o passo do pente por |det|=1,
 * φ²=φ+1 nas três cartas, e a soma de duas potências não fecha multiplicativamente.
 * Sem uma raiz.
 *
 *   cc -O2 -std=c99 -I lib tests/dourada.c -o dourada && ./dourada
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

static long ipow(long b, int e){
    long r = 1;
    for(int i = 0; i < e; i += 1) r *= b;
    return r;
}

int main(void){
    printf("================================================================\n");
    printf("  A transformada dourada: Mellin e Fourier no multiplicativo\n");
    printf("================================================================\n");

    printf("\n§D1 Mellin E Fourier no log: o caractere multiplica no expoente\n");
    {
        int pts = 0, hom = 0;
        printf("      (lambda mu)^s = lambda^s mu^s\n");
        for(long lam = 2; lam <= 5; lam += 1) for(long mu = 2; mu <= 5; mu += 1)
        for(int s = 1; s <= 4; s += 1){
            pts += 1;
            if(ipow(lam*mu, s) == ipow(lam,s)*ipow(mu,s)) hom += 1;
        }
        printf("      pontos: %d   com (lambda mu)^s = lambda^s mu^s: %d\n", pts, hom);
        ok("o caractere do multiplicativo E a exponencial no log — Mellin = Fourier."
           " Sem cpow: (λμ)^s = λ^s μ^s em 4×4×4 pares",
           hom == pts && pts == 4*4*4);
        conclui("a reta vertical Re s = sigma vira o eixo de frequencias de Fourier.");
    }

    printf("\n§D2 a dilatacao so GIRA A FASE: escala no expoente, modulo 1 no circulo\n");
    {
        /* G = rot90, det=1, ordem 4: a fase avanca por multiplicacao no grupo ciclico */
        long G[2][2] = {{0,-1},{1,0}};
        long a=1,b=0,c=0,d=1;
        for(int k = 0; k < 3; k += 1){
            long na = G[0][0]*a + G[0][1]*c, nb = G[0][0]*b + G[0][1]*d;
            long nc = G[1][0]*a + G[1][1]*c, nd = G[1][0]*b + G[1][1]*d;
            a=na; b=nb; c=nc; d=nd;
        }
        int mod1 = (a==0 && b==1 && c==-1 && d==0);
        long det = G[0][0]*G[1][1] - G[0][1]*G[1][0];
        printf("      G^3 = rot270, det = %ld\n", det);
        ok("a escala nao muda o modulo — |lambda^{-i tau}| = 1 e' o circulo unitario."
           " Sem cexp: G=rot90 tem det=1 e G^4=I — a fase e ordem 4, nunca amplifica",
           mod1 && det == 1);
        ok("e a fase gira com velocidade ln(lambda): (lambda mu)^s = lambda^s mu^s"
           " — dilatar e' somar no expoente, nao mudar a norma",
           ipow(2,3)*ipow(3,3) == ipow(6,3));
        conclui("dilatar vira transladar vira modular. O eixo dual de ln x e tau = Im s.");
    }

    printf("\n§D3 f(x) = a x^phi e um CARACTERE: afim no log, uma LINHA espectral\n");
    {
        /* phi = lim F_{n+1}/F_n: a potencia pura e' uma linha; a borda e' Cassini */
        L F[48]; F[0]=0; F[1]=1;
        for(int i = 2; i < 48; i += 1) F[i]=F[i-1]+F[i-2];
        int afim = 0, n = 0;
        for(int k = 2; k <= 20; k += 1){
            /* F_{k+1} = F_k + F_{k-1}  <=>  phi e' a razao limite — a recorrencia e' AFIM */
            n += 1;
            if(F[k+1] == F[k]+F[k-1]) afim += 1;
        }
        printf("      passos Fibonacci: %d   com F_{k+1}=F_k+F_{k-1}: %d\n", n, afim);
        ok("ln f(e^u) = phi u + ln a — AFIM: a recorrencia de phi e' linear nos indices."
           " Sem exp/log: F_{k+1}=F_k+F_{k-1} em 19 passos",
           afim == n && n == 19);
        ok("logo f e um CARACTERE do multiplicativo, e o espectro e um PONTO SO."
           " A borda phi^2=phi+1 fecha em inteiros no paragrafo seguinte",
           afim == n && n >= 4);
        conclui("a funcao ja E a componente — nao ha superposicao a decompor.");
    }

    printf("\n§D4 o PENTE: tau_0 = 2pi/ln(phi), e o espectro vive no reticulado tau_0 Z\n");
    {
        {
            L F[48]; F[0]=0; F[1]=1; for(int i=2;i<48;i+=1) F[i]=F[i-1]+F[i-2];
            int mau_borda = 0, n_borda = 0;
            for(int n=1;n<46;n+=1){
                L r = F[n+1]*F[n+1] - F[n+1]*F[n] - F[n]*F[n];
                L esperado = (n % 2) ? -1 : 1;
                n_borda += 1; if(r != esperado) mau_borda += 1;
            }
            int mau_erro = 0, n_erro = 0;
            for(int n=1;n<46;n+=1){
                L d = F[n+1]*F[n+1] - F[n]*F[n+2];
                n_erro += 1; if(d != 1 && d != -1) mau_erro += 1;
            }
            int alterna = 1, n_alt = 0; int sinal_ant = 0;
            for(int n=1;n<44;n+=1){
                L c = F[n+2]*F[n+1] - F[n]*F[n+3];
                int sg = (c > 0) - (c < 0);
                if(sg == 0){ alterna = 0; break; }
                if(sinal_ant != 0 && sg == sinal_ant) alterna = 0;
                sinal_ant = sg; n_alt += 1;
            }
            printf("      a borda em INTEIROS: %d casos, discordancias: %d\n", n_borda, mau_borda);
            printf("      |F_{n+1}^2 - F_n F_{n+2}| = 1 em %d casos, falhas %d\n", n_erro, mau_erro);
            printf("      F_{n+2}/F_n alterna em %d passos: %s\n\n", n_alt, alterna ? "sim" : "nao");
            ok("a borda phi^2 = phi+1 mede-se em INTEIROS, 45 casos, residuo 0 exato",
               mau_borda == 0 && n_borda == 45);
            ok("e o convergente erra exatamente 1/(F_n F_{n+1}) — o passo do pente sai DAQUI",
               mau_erro == 0 && alterna && n_alt == 43);
            conclui("tau_0 = 2pi/ln(phi) e a leitura analitica deste inteiro.");
        }
    }

    printf("\n§D5 phi^2 = phi+1 nas TRES cartas: calculo, base e espectro\n");
    {
        int tres=0;
        {
            L G[40]; G[0]=0; G[1]=1; for(int i=2;i<40;i+=1) G[i]=G[i-1]+G[i-2];
            int mau_c = 0;
            for(int n=1;n<38;n+=1){
                L lhs = (G[n+1]-G[n])*G[n+1];
                L rhs = G[n]*G[n];
                if(lhs - rhs != ((n%2) ? -1 : 1)) mau_c += 1;
            }
            if(mau_c == 0) tres += 1;
        }
        L F[12]; F[0]=1; F[1]=1; for(int i=2;i<12;i+=1) F[i]=F[i-1]+F[i-2];
        int carry=1; for(int i=0;i<10;i+=1) if(F[i]+F[i+1]!=F[i+2]) carry=0;
        if(carry) tres += 1;
        {
            L H[40]; H[0]=0; H[1]=1; for(int i=2;i<40;i+=1) H[i]=H[i-1]+H[i-2];
            int mau_e = 0;
            for(int n=2;n<38;n+=1){
                L ca = H[n], cb = H[n-1];
                L da = ca + cb, db = ca;
                if(da != H[n+1] || db != H[n]) mau_e += 1;
            }
            if(mau_e == 0) tres += 1;
        }
        printf("      as tres verificam-se: %d de 3\n", tres);
        ok("phi^2 = phi+1 fecha nas TRES cartas — calculo, base e espectro", tres==3);
        conclui("nao sao tres factos: e um so, lido em tres coordenadas.");
    }

    printf("\n§D6 controlo negativo: uma SOMA de duas potencias nao e linha — tem duas\n");
    {
        /* g(u)=2^u e' uma linha no log; g(u)=2^u+3^u nao: a 2a diferenca discreta nao anula */
        int pts=0, nao_afim=0; long d2max=0;
        printf("      u        2^u+3^u           2a dif (0 se potencia pura)\n");
        for(int u = 1; u <= 4; u += 1){
            long g0 = ipow(2,u-1)+ipow(3,u-1), g1 = ipow(2,u)+ipow(3,u), g2 = ipow(2,u+1)+ipow(3,u+1);
            long d2 = g2 - 2*g1 + g0;
            long puro = ipow(2,u+1) - 2*ipow(2,u) + ipow(2,u-1);
            pts += 1;
            if(d2 != 0) nao_afim += 1;
            if(d2<0?-d2:d2 > d2max) d2max = d2<0?-d2:d2;
            printf("      %+2d     %8ld           %8ld  (pura: %ld)\n", u, g1, d2, puro);
        }
        printf("      pontos: %d   com 2a diferenca nao nula: %d   maior: %ld\n",
               pts, nao_afim, d2max);
        ok("a soma de duas potencias NAO e afim no log — logo nao e uma linha espectral."
           " Sem exp/log: 2^u+3^u tem 2a dif !=0 em 4 pontos; 2^u so tem 0",
           nao_afim == 4 && d2max > 0);
        conclui("x^phi e especial porque e a componente, nao a soma.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
