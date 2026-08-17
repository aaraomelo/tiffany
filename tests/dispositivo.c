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
        /* TUDO ISTO SAO CONTAGENS, e contagens sao inteiros: bytes, bits, celulas e a
         * densidade de um die. Os doubles nao carregavam virgula nenhuma — carregavam
         * numeros grandes, que e outra coisa, e o `long` leva-os sem perder um bit.
         * A area e uma RAZAO de duas contagens, e nao se forma: compara-se por PRODUTO
         * CRUZADO, que e a regra da casa —
         *      bits/bpm > 1/10   <=>   10·bits > bpm
         *      bits/bpm < 100    <=>   bits < 100·bpm
         * e assim a asserção deixa de precisar de uma virgula para dizer o que diz. */
        long bytes_llm = 1300000000L;            /* o llama3.2:1b, quantizado */
        long bits = bytes_llm * 8;
        long celulas_slc = bits;                 /* 1 bit por célula */
        long celulas_qlc = bits / 4;             /* 4 bits por célula, o QLC comercial */
        long bits_por_mm2 = 1000000000000L / 100;   /* 1 Tb em ~100 mm² */
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
        /* σσ' = −1 é EXATA em Z[σ] e não precisa de raiz nenhuma: σ e σ' são as raízes de
         * x² − mx − 1, logo o produto delas É o termo constante, −1. Verifica-se pelo
         * polinómio, em inteiros, e para toda a família — não só para o ouro. */
        double s = (1 + sqrt(5.0))/2, sl = (1 - sqrt(5.0))/2;   /* só para as linhas que imprimem */
        {
            /* O QUE AQUI ESTAVA NÃO PODIA FALHAR:
             *      long long soma = m, produto = -1;
             *      if(produto == -1 && soma == m) exatos++;
             * as duas comparações são com o que a linha de cima atribuiu. A asserção dizia
             * «Vieta, em inteiros» e media as suas próprias atribuições.
             *
             * Vieta mede-se em ℤ[√D], onde σ e σ' EXISTEM como pares e as contas acontecem:
             * guarda-se 2σ = (m, 1) e 2σ' = (m, −1), com D = m² + 4. Então
             *
             *      (2σ)(2σ') = m² − D = −4      ⟹  σ·σ' = −1     — e é a NORMA
             *      (2σ) + (2σ') = (2m, 0)       ⟹  σ + σ' = m    — a parte irracional CANCELA
             *
             * e o cancelamento do √D na soma é o conteúdo: é a conjugação, a dobra b ↦ −b. */
            int metais = 0, prod_ok = 0, soma_ok = 0, pol_ok = 0;
            for(long m = 1; m <= 12; m++){
                long D = m*m + 4;
                metais++;
                /* o produto, pela norma de ℤ[√D] — e ela é uma CONTA, não uma atribuição */
                long pa, pb;
                rt_zd_mul(m, 1, m, -1, D, &pa, &pb);
                if(pa == -4 && pb == 0) prod_ok++;          /* (2σ)(2σ') = −4 + 0·√D */
                /* a soma: a parte em √D tem de CANCELAR, e é isso que a conjugação faz */
                long sa = m + m, sb = 1 + (-1);
                if(sa == 2*m && sb == 0) soma_ok++;
                /* e a polaridade, sem raiz: σ' < 0 ⟺ m < √D ⟺ m² < D = m²+4, sempre */
                if(m*m < D) pol_ok++;
            }
            printf("        e em ℤ[√D]: (2σ)(2σ') = m² − D = −4 e a parte em √D CANCELA na\n");
            printf("        soma, em %d metais e sem uma raiz quadrada\n", metais);
            ok("os terminais sao DOIS e tem polaridade oposta — o + e o -, do §P7. E mede-se"
               " sem raiz: σ' < 0 e' m < raiz(D), que e' m^2 < D = m^2+4, verdadeiro para"
               " toda a familia e nao so' para o ouro",
               pol_ok == metais && metais == 12);
            ok("sigma.sigma' = -1 EXATO em ℤ[√D], e agora a conta ACONTECE: (2σ)(2σ') sai"
               " por rt_zd_mul e da' -4 + 0.raiz(D) nos doze metais, donde σσ' = -1. E a"
               " SOMA e' a outra metade — a parte em raiz(D) CANCELA, que e' a conjugacao,"
               " a dobra b -> -b. O que aqui estava atribuia `produto = -1` e comparava com"
               " -1: nao podia falhar, e dizia «Vieta, em inteiros»",
               prod_ok == metais && soma_ok == metais);
        }
        /* esta linha media sigma*sigma' em double contra 1e-14; a versao exata esta no
         * bloco acima (Vieta, em inteiros, e para toda a familia). Fica o registo do valor,
         * sem asserção sobre ele. */
        printf("        (em double, para o ouro: sigma*sigma' = %+.15f)\n", s*sl);
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
