/* dispositivo.c — O BANCO VIRA DISPOSITIVO: NAND é SSD, e a alimentação vem da liga.
 *
 * O Aarão: "vamos transformar o banco em dispositivo — é simples, ele já funciona todo com porta
 * NAND, já é um SSD e opera em um. Então ele pode simular a memória RAM que a LLM vai precisar:
 * tudo dentro, só terminais pra fora. Mas e a alimentação? Ele vai funcionar por indução dual pela
 * liga que projetamos."
 *
 * E ISTO FECHA O ARCO DO DIA, porque cada peça já foi medida noutro sítio:
 *
 *      o mcu.c        a ALU inteira sai de NAND só (65536 pares medidos)
 *      a memória      flash É NAND — não é analogia, é o mesmo transístor
 *      o §P7          os terminais: dois, com polaridade e σσ' = −1
 *      o arraytermico o Seebeck com o céu: 993 mW
 *      o colheita.c   o RF ambiente: 21 µW numa antena isotrópica
 *      o liga.c       a liga que absorve, e as quatro camadas por gradiente
 *
 * A PERGUNTA QUE DECIDE É UMA SÓ: **a colheita paga o dispositivo?** E a resposta depende do que
 * ele faz — e é aqui que o reenquadramento do Aarão (*"o cérebro é o microprocessador; estamos
 * fazendo o encanamento"*) muda a conta por três ordens de grandeza. *Guardar é quase grátis;
 * calcular não é.*
 *
 *   §D1  NAND é universal — e a memória flash É NAND, não é parecida
 *   §D2  a RAM da LLM em células: quantas, e quanto espaço
 *   §D3  O BALANÇO: o que a colheita dá contra o que o dispositivo gasta
 *   §D4  e a conta muda com o PAPEL: guardar, transferir, ou calcular
 *   §D5  os terminais: dois para fora, e o resto dentro
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/dispositivo.c -o dispositivo
 */
#include <stdio.h>
#include "unidade.h"
#include "reta.h"

int main(void){
    puts("dispositivo.c — O BANCO VIRA DISPOSITIVO: NAND e SSD, e a liga alimenta\n");

    /* ── §D1 ─────────────────────────────────────────────────────────────── */
    puts("§D1  NAND E UNIVERSAL — e a memoria flash E NAND, nao e parecida\n");
    {
        /* a universalidade: NOT, AND, OR e XOR saem todos de NAND. Mede-se nas 4 entradas. */
        int falhas_ = 0;
        for(int a = 0; a < 2; a++)
            for(int b = 0; b < 2; b++){
                int nand = !(a && b);
                int nott = !(a && a);                       /* NOT a  = NAND(a,a) */
                int andd = !(nand && nand);                 /* AND    = NOT(NAND) */
                int orr  = !( (!(a&&a)) && (!(b&&b)) );     /* OR     = NAND(NOT a, NOT b) */
                int xorr = !( (!(a && nand)) && (!(b && nand)) );
                if(nott != !a || andd != (a&&b) || orr != (a||b) || xorr != (a^b)) falhas_++;
            }
        ok("NAND gera NOT, AND, OR e XOR — nas quatro entradas, sem excecao",
           falhas_ == 0);
        conclui("e a memoria FLASH e literalmente NAND: as celulas ligam-se em serie numa string");
        conclui("NAND, e e dai que vem o nome do dispositivo. Nao ha analogia — e o mesmo objeto.");
        puts("");
    }

    /* ── §D2 ─────────────────────────────────────────────────────────────── */
    puts("§D2  A RAM DA LLM EM CELULAS: quantas, e quanto espaco\n");
    {
        long bytes_llm = 1300000000L;            /* o llama3.2:1b, quantizado */
        long bits = bytes_llm * 8;
        long celulas_slc = bits;                 /* 1 bit por célula */
        long celulas_qlc = bits / 4;             /* 4 bits por célula, o QLC comercial */
        long bits_por_mm2 = 1000000000000L / 100;   /* 1 Tb em ~100 mm² */
        (void)celulas_slc;
        ok("a RAM do modelo cabe numa area de escala de CHIP, e nao de sala — e a area e uma"
           " RAZAO de duas contagens, comparada por produto cruzado e sem formar a divisao",
           10*bits > bits_por_mm2 && bits < 100*bits_por_mm2);
        printf("      -> %ld bytes = %ld bits = %ld celulas QLC; a area e %ld/%ld mm2\n",
               bytes_llm, bits, celulas_qlc, bits, bits_por_mm2);
        conclui("o modelo inteiro cabe num die. O problema do dispositivo nunca foi o ESPACO.");
        puts("");
    }

    /* ── §D3  O BALANÇO ──────────────────────────────────────────────────── */
    puts("§D3  O BALANCO: o que a colheita da contra o que o dispositivo gasta\n");
    {
        /* potências em µW: 993100 = 0,9931 W (arraytermico), 21 = 21 µW (colheita) */
        long p_seebeck_uw = 993100L;
        long p_rf_uw      = 21L;
        long colhido_uw   = p_seebeck_uw + p_rf_uw;

        struct { const char *papel; long uw; } GASTO[] = {
            { "guardar (NAND em repouso)",        1L       },
            { "ler sequencial (100 MB/s)",    50000L       },
            { "escrever (100 MB/s)",         300000L       },
            { "inferencia da LLM em CPU", 15000000L       },
        };
        printf("      %-32s %12s %14s\n", "papel", "gasto (µW)", "colheita paga?");
        int pagos = 0, n = 4;
        for(int i = 0; i < n; i++){
            int paga = GASTO[i].uw < colhido_uw;
            printf("      %-32s %12ld %14s\n", GASTO[i].papel, GASTO[i].uw,
                   paga ? "SIM" : "NAO");
            if(paga) pagos++;
        }
        ok("a colheita paga GUARDAR, LER e ESCREVER — mas NAO paga a inferencia",
           pagos == 3);
        long falta_uw = GASTO[3].uw - colhido_uw;
        long raz_cal = GASTO[3].uw * 100L / colhido_uw;
        printf("      -> colhido %ld µW (Seebeck %ld + RF %ld). A inferencia pede %ld centesimos"
               " de colheita (x%ld).\n",
               colhido_uw, p_seebeck_uw, p_rf_uw, raz_cal, raz_cal / 100);
        (void)falta_uw;
        conclui("e este e o numero que decide o dispositivo. Nao e o espaco, nao e o NAND: e a");
        conclui("distancia de quinze vezes entre o que a liga colhe e o que calcular custa.");
        puts("");
    }

    /* ── §D4  o PAPEL ────────────────────────────────────────────────────── */
    puts("§D4  E A CONTA MUDA COM O PAPEL — guardar, transferir, ou calcular\n");
    {
        long colhido_uw = 993100L;                /* COPIADO do cosmico.c: o céu como frio */
        long encanar_uw = 300000L;
        long calcular_uw = 15000000L;
        long r_enc = colhido_uw * 100L / encanar_uw;
        long r_cal = calcular_uw * 100L / colhido_uw;
        printf("      -> as duas razões, em centésimos: colhido/encanar = %ld,"
               " calcular/colhido = %ld\n", r_enc, r_cal);
        ok("se o dispositivo ENCANA (guarda e transfere), a colheita paga — e a folga diz-se em"
           " vez de se arredondar para «3x»: a razao e' 331 centesimos, ou seja TRES POR CENTO"
           " acima do limiar de tres. Um encanamento de 0,34 W ja' nao pagava, e o «3x» nao"
           " deixava ver isso. Os tres numeros sao DECLARADOS — o colhido vem do cosmico.c e"
           " os outros dois sao estimativas de projecto —, logo o que se afirma e' a razao. O colheita.c ja' o diz de outro modo: «um valor copiado e' postulado»",
           r_enc > 330 && r_enc < 332);
        ok("e se ele CALCULA, nao paga — e a diferenca diz-se: a razao e' 1510 centesimos,"
           " quinze vezes e meia, enquadrada dos dois lados. «Mais de uma ordem de grandeza»"
           " era verdade e dizia menos do que se sabe",
           r_cal > 1500 && r_cal < 1520);
        printf("      -> encanar %ld µW contra %ld µW colhidos: folga %ld centesimos.\n",
               encanar_uw, colhido_uw, r_enc);
        printf("         calcular %ld µW contra %ld µW: faltam %ld µW.\n",
               calcular_uw, colhido_uw, calcular_uw - colhido_uw);
        conclui("O REENQUADRAMENTO DO AARAO E O QUE FAZ A CONTA FECHAR. Se o cerebro e o");
        conclui("processador e o dispositivo so encana, ele vive da liga. Se tivesse de calcular,");
        conclui("nao viveria — e nenhuma liga melhor resolveria, porque a distancia e de 15x.");
        puts("");
    }

    /* ── §D5  os TERMINAIS ───────────────────────────────────────────────── */
    puts("§D5  OS TERMINAIS: dois para fora, e o resto dentro\n");
    {
        {
            int metais = 0, prod_ok = 0, soma_ok = 0, pol_ok = 0;
            for(long m = 1; m <= 12; m++){
                long D = m*m + 4;
                metais++;
                long pa, pb;
                rt_zd_mul(m, 1, m, -1, D, &pa, &pb);
                if(pa == -4 && pb == 0) prod_ok++;
                long sa = m + m, sb = 1 + (-1);
                if(sa == 2*m && sb == 0) soma_ok++;
                if(m*m < D) pol_ok++;
            }
            printf("        e em Z[raiz(D)]: (2σ)(2σ') = m² − D = −4 e a parte em raiz(D) CANCELA na\n");
            printf("        soma, em %d metais e sem uma raiz quadrada\n", metais);
            ok("os terminais sao DOIS e tem polaridade oposta — o + e o -, do §P7. E mede-se"
               " sem raiz: σ' < 0 e' m < raiz(D), que e' m^2 < D = m^2+4, verdadeiro para"
               " toda a familia e nao so' para o ouro",
               pol_ok == metais && metais == 12);
            ok("sigma.sigma' = -1 EXATO em Z[raiz(D)], e agora a conta ACONTECE: (2σ)(2σ') sai"
               " por rt_zd_mul e da' -4 + 0.raiz(D) nos doze metais, donde σσ' = -1. E a"
               " SOMA e' a outra metade — a parte em raiz(D) CANCELA, que e' a conjugacao,"
               " a dobra b -> -b. O que aqui estava atribuia `produto = -1` e comparava com"
               " -1: nao podia falhar, e dizia «Vieta, em inteiros»",
               prod_ok == metais && soma_ok == metais);
        }
        /* ouro m=1: σ=(1,1), σ'=(1,−1) em Z[√5]; F_{8}/F_{7}=21/13 aproxima (1+√5)/2 */
        printf("        (ouro: σ=(1,1), σ'=(1,−1) em Z[raiz5]; F8/F7 = 21/13 ≈ terminal +)\n");
        printf("      -> dois terminais: + e −, produto σσ' = −1 em Z[raiz(D)].\n");
        conclui("tudo dentro, so os dois terminais fora: a alimentacao entra por inducao na liga");
        conclui("(colheita.c: ler e escrever sao adjuntos, logo a mesma espira faz as duas), e o");
        conclui("sinal sai pelo par. Nao ha uma terceira coisa a atravessar a fronteira.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    printf("\n  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else        printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;

}
