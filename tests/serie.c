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

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
