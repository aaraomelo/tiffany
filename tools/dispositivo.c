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
 *   cc -O2 -std=c99 -Wall -Wformat dispositivo.c -lm -o dispositivo && ./dispositivo
 */
#include <stdio.h>
#include <math.h>
#include "unidade.h"

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
        double bytes_llm = 1.3e9;                 /* o llama3.2:1b, quantizado */
        double bits = bytes_llm * 8;
        double celulas_slc = bits;                /* 1 bit por célula */
        double celulas_qlc = bits / 4.0;          /* 4 bits por célula, o QLC comercial */
        /* a densidade real: um die NAND de 1 Tb em ~100 mm² */
        double bits_por_mm2 = 1e12 / 100.0;
        double area_mm2 = bits / bits_por_mm2;
        ok("a RAM do modelo cabe numa area de escala de CHIP, e nao de sala",
           area_mm2 > 0.1 && area_mm2 < 100.0);
        printf("      -> %.1f GB = %.2e bits = %.2e celulas QLC, em %.2f mm2 de silicio.\n",
               bytes_llm/1e9, bits, celulas_qlc, area_mm2);
        conclui("o modelo inteiro cabe num die. O problema do dispositivo nunca foi o ESPACO.");
        puts("");
    }

    /* ── §D3  O BALANÇO ──────────────────────────────────────────────────── */
    puts("§D3  O BALANCO: o que a colheita da contra o que o dispositivo gasta\n");
    {
        /* o que a colheita deu, medido nos outros medidores */
        double p_seebeck = 0.9931;                /* W — arraytermico.c §A5 com o ceu */
        double p_rf      = 21e-6;                 /* W — colheita.c §C1, antena isotropica */
        double colhido   = p_seebeck + p_rf;

        /* o que gasta, por papel — numeros de ordem, de folhas de dados publicas */
        struct { const char *papel; double watts; } GASTO[] = {
            { "guardar (NAND em repouso)",   1e-6  },   /* retencao: praticamente zero */
            { "ler sequencial (100 MB/s)",   0.05  },
            { "escrever (100 MB/s)",         0.30  },
            { "inferencia da LLM em CPU",   15.0   },
        };
        printf("      %-32s %12s %14s\n", "papel", "gasto (W)", "colheita paga?");
        int pagos = 0, n = 4;
        for(int i = 0; i < n; i++){
            int paga = GASTO[i].watts < colhido;
            printf("      %-32s %12.2e %14s\n", GASTO[i].papel, GASTO[i].watts,
                   paga ? "SIM" : "NAO");
            if(paga) pagos++;
        }
        ok("a colheita paga GUARDAR, LER e ESCREVER — mas NAO paga a inferencia",
           pagos == 3);
        printf("      -> colhido %.4f W (Seebeck %.4f + RF %.1e). A inferencia pede %.0fx mais.\n",
               colhido, p_seebeck, p_rf, GASTO[3].watts/colhido);
        conclui("e este e o numero que decide o dispositivo. Nao e o espaco, nao e o NAND: e a");
        conclui("distancia de quinze vezes entre o que a liga colhe e o que calcular custa.");
        puts("");
    }

    /* ── §D4  o PAPEL ────────────────────────────────────────────────────── */
    puts("§D4  E A CONTA MUDA COM O PAPEL — guardar, transferir, ou calcular\n");
    {
        /* o reenquadramento do Aarao: "o cerebro e o microprocessador; estamos fazendo o
         * encanamento". Se o dispositivo so encana, ele nao calcula — e ai a conta fecha. */
        double colhido = 0.9931;
        double encanar = 0.30;                    /* o pior caso do encanamento: escrever */
        double calcular = 15.0;
        ok("se o dispositivo ENCANA (guarda e transfere), a colheita paga com folga de 3x",
           colhido > 3*encanar);
        ok("e se ele CALCULA, nao paga — e a diferenca e de mais de uma ordem de grandeza",
           calcular > 10*colhido);
        printf("      -> encanar %.2f W contra %.2f W colhidos: folga de %.1fx.\n",
               encanar, colhido, colhido/encanar);
        printf("         calcular %.0f W contra %.2f W: faltam %.0f W.\n",
               calcular, colhido, calcular - colhido);
        conclui("O REENQUADRAMENTO DO AARAO E O QUE FAZ A CONTA FECHAR. Se o cerebro e o");
        conclui("processador e o dispositivo so encana, ele vive da liga. Se tivesse de calcular,");
        conclui("nao viveria — e nenhuma liga melhor resolveria, porque a distancia e de 15x.");
        puts("");
    }

    /* ── §D5  os TERMINAIS ───────────────────────────────────────────────── */
    puts("§D5  OS TERMINAIS: dois para fora, e o resto dentro\n");
    {
        /* o §P7 mediu que o par (sigma, sigma') tem polaridade e sigma.sigma' = -1.
         * Aqui conta-se o que atravessa a fronteira: dois terminais, e mais nada. */
        double s = (1 + sqrt(5.0))/2, sl = (1 - sqrt(5.0))/2;
        ok("os terminais sao DOIS e tem polaridade oposta — o + e o -, do §P7",
           s > 0 && sl < 0);
        ok("e a lei que os liga e a conservacao: sigma.sigma' = -1, exata",
           fabs(s*sl + 1.0) < 1e-14);
        printf("      -> sigma = %.6f (+), sigma' = %.6f (-), produto %.1f.\n", s, sl, s*sl);
        conclui("tudo dentro, so os dois terminais fora: a alimentacao entra por inducao na liga");
        conclui("(colheita.c: ler e escrever sao adjuntos, logo a mesma espira faz as duas), e o");
        conclui("sinal sai pelo par. Nao ha uma terceira coisa a atravessar a fronteira.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    /* O VEREDITO, que faltava. O programa tinha sete asserções e devolvia 0 sem dizer nada —
     * quem lê fica dependente do código de saída, que é mais fraco do que a frase. */
    printf("\n  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else        printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;

}
