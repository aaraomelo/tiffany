/* serie.c — A EXPONENCIAL E O π PELA DEFINIÇÃO DA ARANHA, em fatorial e inteiros.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/serie tests/serie.c && /tmp/serie
 */
#include "unidade.h"
#include "serie.h"
#include <stdio.h>

int main(void){
    const long S = 1000000;                    /* a escala: seis casas, tudo inteiro */
    printf("A SÉRIE: exp e π como a aranha os define --- em fatorial, sem uma vírgula\n\n");

    /* ── §S1 A CISÃO POR PARIDADE, E A CONSERVAÇÃO POR DERIVAÇÃO ──────────── */
    {
        printf("§S1  exp(tJ) = c(t)·1 + s(t)·J, e c²+s² ≡ 1 sai por derivação.\n\n");
        long mal = 0;
        printf("      t/S      c(t)        s(t)     c²+s² (÷10⁶)   termos  parou\n");
        long fecha = 0, pontos = 0;
        for(long i = 0; i <= 6; i++){
            long t = i*S/2;
            SfSerie c = sf_c(t, S), s = sf_s(t, S);
            long niv = sf_nivel(t, S);
            printf("      %ld/2  %10ld %10ld  %12ld %6d   %s\n", i, c.valor, s.valor,
                   niv, c.termos, c.parou ? "sim" : "NAO");
            pontos++;
            /* a conservação: c²+s² tem de dar S², a menos do grão da escala */
            if(niv > S - 3000 && niv < S + 3000) fecha++;
            /* E OS TERMOS TÊM DE SE ANULAR: é o thm:serie, e é o que faz o
             * processo TERMINAR em vez de ser truncado */
            if(!c.parou || !s.parou) mal++;
        }
        printf("      → a conservação fecha em %ld de %ld pontos, e a série PAROU em"
               " todos\n", fecha, pontos);
        if(fecha != pontos) mal++;
        printf("\n");
        ok("A EXPONENCIAL SAI DA CISÃO POR PARIDADE, E A CONSERVAÇÃO NÃO SE IMPÕE: DERIVA-SE."
           " Da única hipótese J²=−1 as potências ciclam com período quatro, a série parte-se"
           " pelos índices pares e ímpares, e o que fica é o par (c,s). E c²+s²≡1 sai de"
           " (c²+s²)' = 2cc'+2ss' = −2cs+2sc = 0 com valor 1 em zero --- medido, fecha em"
           " todos os pontos a menos do grão da escala. E O PROCESSO TERMINA, que é o"
           " thm:serie: os termos ANULAM-SE ao fim de finitas parcelas, porque o fatorial"
           " cresce mais depressa que a potência e a divisão inteira leva o termo a zero."
           " Isto não é truncar --- é parar, e o número de termos é o custo, que se conta.",
           mal == 0);
    }

    /* ── §S2 O π, E A FACE POR QUE SE O PROCURA ───────────────────────────── */
    {
        printf("§S2  π = min{t>0 : exp(tJ)·1 = −1}, e só UMA das faces se procura.\n\n");
        long mal = 0;
        printf("      grão      π (em escala)   c(π)        termos\n");
        long estavel = 0, medidos = 0; long ultimo = 0;
        for(long g = S/100; g >= S/10000; g /= 10){
            SfPi p = sf_pi(S, g);
            printf("      1/%-7ld %10ld   %10ld  %8d\n", S/g, p.pi, p.c_no_ponto, p.termos);
            if(!p.achou){ mal++; continue; }
            medidos++;
            /* A OUTRA FACE CONFERE: c(π) tem de dar −S */
            if(p.c_no_ponto > -S - 3000 && p.c_no_ponto < -S + 3000) estavel++;
            ultimo = p.pi;
        }
        printf("      → c(π) = −1 confere em %ld de %ld grãos · π = %ld/%ld\n",
               estavel, medidos, ultimo, S);
        if(estavel != medidos || medidos == 0) mal++;

        /* ── O GUME, e é ele que explica um erro que a primeira escrita fez:
         * procurar pela face ERRADA dá o número errado, e não por defeito da
         * série. Mede-se procurando o mínimo de c em vez do zero de s. */
        {
            long ant = S, achou_c = 0;
            for(long t = S/1000; t < 8*S; t += S/1000){
                SfSerie c = sf_c(t, S);
                if(c.valor <= -S + S/1000 && ant > -S + S/1000){ achou_c = t; break; }
                ant = c.valor;
            }
            printf("      gume: procurando o MÍNIMO de c em vez do zero de s → %ld/%ld\n",
                   achou_c, S);
            printf("            perto do mínimo c(t) ≈ −1 + (t−π)²/2 é PLANA: 10⁻³ no"
                   " valor dá 4·10⁻² no t\n");
            printf("            o erro é da CONDIÇÃO, não da série --- e o zero de s é"
                   " simples, porque s'(π) = c(π) = −1\n");
            /* a face errada TEM de dar um número diferente e pior */
            if(achou_c == 0 || (achou_c > ultimo - S/50 && achou_c < ultimo + S/50)) mal++;
        }
        printf("\n");
        ok("O π É UMA SAÍDA DO RELÓGIO, E LÊ-SE CONTANDO VOLTAS. A definição é sobre o PAR"
           " --- exp(πJ) = −1 quer dizer c(π) = −1 E s(π) = 0 ---, mas as duas faces não"
           " servem igualmente para PROCURAR, e essa distinção custou-me o número errado."
           " Procurar c = −1 é procurar um MÍNIMO, e perto do mínimo a função é plana: um"
           " desvio de 10⁻³ no valor dá 4·10⁻² no t, e sai 3,097. Procurar s = 0 é procurar"
           " um zero SIMPLES, porque s'(π) = c(π) = −1 ≠ 0, e sai 3,1416. É a MESMA equação"
           " lida nas duas faces, e só uma se procura --- a outra CONFERE, e c(π) dá −1 em"
           " todos os grãos. Nenhuma palavra disto é geométrica: não há arco nem área, e o"
           " círculo é o rasto que este fluxo desenha.", mal == 0);
    }

    /* ── §S3 A EXPONENCIAL REAL, e o valor por dois caminhos ──────────────── */
    {
        printf("§S3  e^x pela mesma recorrência, e a lei que o par obriga.\n\n");
        long mal = 0;
        printf("      x     e^x (escala 10⁶)   termos   e^x·e^{−x} (÷10⁶)\n");
        long fecha = 0, pontos = 0;
        for(long i = 1; i <= 4; i++){
            long x = i*S/2;
            SfSerie e1 = sf_exp(x, S), e2 = sf_exp(-x, S);
            long prod = (e1.valor/1000) * (e2.valor/1000);
            printf("      %ld/2  %14ld %8d   %14ld\n", i, e1.valor, e1.termos, prod);
            pontos++;
            /* A LEI: e^x · e^{−x} = 1, e ela não se impõe --- é a segunda equação
             * da Def.~def:op, ∂x·x = 1, com a exponencial no lugar */
            if(prod > S - 5000 && prod < S + 5000) fecha++;
            if(!e1.parou) mal++;
        }
        printf("      → e^x·e^{−x} = 1 em %ld de %ld pontos\n", fecha, pontos);
        if(fecha != pontos) mal++;
        printf("\n");
        ok("E A EXPONENCIAL REAL É A MESMA RECORRÊNCIA SEM O SINAL ALTERNADO, com a lei que"
           " o operador obriga: e^x·e^{−x} = 1 é a segunda equação da def:op, ∂x·x = 1, com"
           " a exponencial no lugar de x --- e ela fecha em todos os pontos medidos, dentro"
           " do grão. Não foi imposta: foi verificada. E os termos anulam-se aqui pelo mesmo"
           " motivo, o fatorial no denominador --- é por isso que esta é a representação que"
           " a casa usa para o irracional, e não fracção nem vírgula: a fracção não alcança"
           " o π e a vírgula perde a exactidão que o resto todo mantém.", mal == 0);
    }

    /* ── §S4 A EDO RESOLVIDA PELA SÉRIE: a raiz NUNCA é precisa ─────────── */
    {
        printf("§S4  a EDO resolve-se pela série, e a raiz irracional nunca se extrai.\n\n");
        long mal = 0;
        /* (a) O CONTROLO: raízes INTEIRAS, onde há forma fechada para confrontar.
         * y'' + 5y' + 6y = 0, y(0)=1, y'(0)=0  →  y = 3e^{-2t} − 2e^{-3t} */
        {
            long co[2] = {6, 5}, d0[2] = {S, 0};
            long lam[2] = {-2*S, -3*S}, amp[2] = {3*S, -2*S};
            printf("      RAÍZES INTEIRAS (o controlo): y''+5y'+6y=0, y(0)=1, y'(0)=0\n");
            printf("      t      pela SÉRIE   pela forma fechada   diferença  termos\n");
            long batem = 0, pontos = 0;
            for(long i = 0; i <= 4; i++){
                long t = i*S/4;
                SfSerie y = sf_edo(co, 2, d0, t, S);
                long f = sf_fechada(lam, amp, 2, t, S);
                long d = y.valor > f ? y.valor - f : f - y.valor;
                printf("      %ld/4  %11ld  %18ld  %9ld %6d\n", i, y.valor, f, d, y.termos);
                pontos++;
                if(d < 3000) batem++;          /* dentro do grão das duas contas */
            }
            printf("      → %ld de %ld dentro do grão --- e a SÉRIE é a mais exacta das"
                   " duas:\n        3e^{-2}−2e^{-3} = 0,306432, e é isso que ela dá\n",
                   batem, pontos);
            if(batem != pontos) mal++;
        }

        /* (b) RAÍZES IRRACIONAIS: a série corre igual, e nada se extrai.
         * y'' − y' − y = 0 tem raízes (1±√5)/2 --- o metal do ouro. */
        {
            long co[2] = {-1, -1}, d0[2] = {S, 0};
            printf("\n      RAÍZES IRRACIONAIS: y''−y'−y=0 (as raízes são (1±√5)/2)\n");
            printf("      t      pela SÉRIE   termos  parou   e a EDO fecha?\n");
            long fecham = 0, pontos = 0;
            for(long i = 1; i <= 4; i++){
                long t = i*S/4;
                SfSerie y = sf_edo(co, 2, d0, t, S);
                /* O GUME: a solução tem de SATISFAZER a equação. Verifica-se pela
                 * segunda diferença, que é y'' aproximado pelo grão:
                 *     y(t+h) − 2y(t) + y(t−h) ≈ h²y''  e  y'' = y' + y */
                long h = S/100;
                SfSerie ym = sf_edo(co, 2, d0, t-h, S), yp = sf_edo(co, 2, d0, t+h, S);
                long dd = yp.valor - 2*y.valor + ym.valor;      /* h²·y'' */
                long d1 = (yp.valor - ym.valor) / 2;            /* h·y' */
                /* h²y'' deve ser h²(y'+y) = h·(h y') + h²y */
                long esperado = (h/1000)*(d1/1000)*1000/1000 + ((h/1000)*(h/1000))*(y.valor/1000)/1000;
                long erro = dd > esperado ? dd - esperado : esperado - dd;
                int fecha = (erro < 400);
                printf("      %ld/4  %11ld %6d   %s     %s (erro %ld)\n", i, y.valor,
                       y.termos, y.parou ? "sim" : "NAO", fecha ? "sim" : "NAO", erro);
                pontos++; if(fecha) fecham++;
                if(!y.parou) mal++;
            }
            printf("      → a equação fecha em %ld de %ld pontos, e NENHUMA raiz foi"
                   " extraída\n", fecham, pontos);
            if(fecham != pontos) mal++;
        }

        printf("\n");
        ok("A EDO RESOLVE-SE PELA SÉRIE, E A RAIZ IRRACIONAL NUNCA SE EXTRAI. As derivadas"
           " em zero saem da PRÓPRIA recorrência --- é ela que as dá --- e a série de Taylor"
           " é fatorial por construção, y = Σ y^{(k)}(0)·t^k/k!. Logo não é preciso saber"
           " QUAIS são as raízes: bastam os coeficientes. Para y''−y'−y=0, cujas raízes são"
           " (1±√5)/2, a série corre exactamente como para as inteiras, os termos anulam-se e"
           " o processo pára. O CONTROLO é o caso de raízes inteiras, onde há forma fechada"
           " para confrontar --- e ali aconteceu o que interessa: a primeira escrita da"
           " série guardava a derivada e o t^k/k! SEPARADOS, multiplicava-os com duas"
           " divisões, e dava 0,16 onde a fechada dá 0,307. O controlo apanhou-a. Corrigida"
           " para o termo carregar já o fatorial, a série passa a dar 0,306432, que é o valor"
           " EXACTO de 3e^{-2}−2e^{-3} --- mais exacta do que o próprio controlo. E o gume da"
           " parte irracional é outro: a solução tem de SATISFAZER a equação, verificado pela"
           " segunda diferença.", mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
