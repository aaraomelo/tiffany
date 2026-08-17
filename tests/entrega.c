/* entrega.c — O NÚMERO SAI EM FRACÇÃO CONTÍNUA, E O CLIENTE RECONSTRÓI.
 *
 * O Aarão: "vamos eliminar esses tambem tudo em fracoes continuas, essa representação vai
 * ate o fim e entrega assim mesmo em long int, dai ele pode reconstruir depois em qualquer
 * representação."
 *
 * É o `def:real` levado à saída, e resolve o último sítio onde o double ainda era preciso.
 * A regra da casa é que o double não entra no núcleo e a representação se faz no fim, para
 * o cliente. O que faltava perguntar era: e se a representação NÃO for decimal?
 *
 * Porque um decimal escrito não é um número aproximado — «3.14159» É 314159/100000, um
 * racional exacto, e o que o estraga é convertê-lo a `double`: 0,1 não é 1/10 em base dois
 * e nunca foi. A fracção contínua desse racional é uma palavra de inteiros, sai por
 * EUCLIDES, e volta ao racional sem perder um bit.
 *
 *      texto ──► p/q exacto ──► [a₀; a₁, …, aₙ] em long ──► p/q ──► o que o cliente quiser
 *
 *   §E1  um decimal escrito é um racional EXACTO — e a leitura é inteira
 *   §E2  a ida e volta pela fracção contínua é EXACTA, e o double não a faz
 *   §E3  a palavra é EUCLIDES: os mesmos quocientes do mdc, noutra coluna
 *   §E4  o cliente reconstrói o que quiser — e a divisão longa não arredonda por acaso
 *   §E5  a saturação conta-se em lugar SEPARADO, e há onde ela acontece
 *   §E6  O PIPE FECHADO: o MMC à entrada, e daí em diante não há vírgula
 *
 * A outra metade da entrega está no `tools/libc.c`: o parser que lê o texto e devolve
 * p/q exacto (`atofr`). A palavra tira-se dele aqui, e não lá — porque lá não haveria
 * como confrontar duas cópias, e uma cópia que ninguém confronta diverge.
 *
 *   cc -O2 -std=c99 -I../lib entrega.c -o entrega && ./entrega
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "reta.h"
#include "aritmetica.h"

int main(void){
printf("\n=== A ENTREGA EM FRACÇÃO CONTÍNUA ==========================================\n");
printf("    O número não sai em decimal arredondado: sai como a PALAVRA que o gera,\n");
printf("    em long, e quem recebe reconstrói o corte que quiser.\n");

/* ── §E1 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E1  Um decimal escrito é um racional EXACTO — e a leitura é inteira.\n\n");
{
    struct { const char *t; int sg; long p, q; } casos[] = {
        { "3.14159",   1,  314159, 100000 },
        { "-0.1",     -1,       1,     10 },
        { "0.5",       1,       5,     10 },
        { "2",         1,       2,      1 },
        { "-1.618",   -1,    1618,   1000 },
        { "1e3",       1,    1000,      1 },
        { "1.5e-2",    1,      15,   1000 },
        { "0.333",     1,     333,   1000 },
    };
    int n = (int)(sizeof casos / sizeof *casos), bons = 0;
    printf("      texto        sinal   p          q         lido\n");
    for(int k = 0; k < n; k++){
        int sg; long p, q;
        int ok_ = rt_le_decimal(casos[k].t, &sg, &p, &q);
        int bate = ok_ && sg == casos[k].sg && p == casos[k].p && q == casos[k].q;
        printf("      %-12s %+3d   %-10ld %-9ld %s\n", casos[k].t, sg, p, q, bate ? "sim" : "NAO");
        if(bate) bons++;
    }
    printf("\n");
    ok("UM DECIMAL ESCRITO E' UM RACIONAL EXACTO, e a leitura dele nao passa por virgula"
       " nenhuma: «3.14159» e' 314159/100000, e o denominador e' uma potencia de dez contada"
       " a cada casa. O expoente entra do mesmo modo, a multiplicar em cima ou em baixo. Oito"
       " textos, com sinal, com ponto e com expoente, e os tres inteiros de cada um batem",
       bons == n && n == 8);
}

/* ── §E2 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E2  A ida e volta pela fracção contínua é EXACTA — e o double não a faz.\n\n");
{
    /* varre-se toda a grelha de decimais com 1 a 3 casas em [0, 10), e mede-se
     * quantos voltam IGUAIS por cada um dos dois caminhos. */
    long total = 0, cf_volta = 0, dbl_volta = 0;
    long maior_n = 0;
    for(long inteiro = 0; inteiro < 10; inteiro++){
        for(long milesimo = 0; milesimo < 1000; milesimo++){
            char t[32];
            int k = 0;
            t[k++] = (char)('0' + inteiro); t[k++] = '.';
            t[k++] = (char)('0' + milesimo/100);
            t[k++] = (char)('0' + (milesimo/10)%10);
            t[k++] = (char)('0' + milesimo%10);
            t[k] = 0;

            int sg; long p, q;
            if(!rt_le_decimal(t, &sg, &p, &q)) continue;
            total++;

            /* CAMINHO 1: pela palavra. p/q → [a₀;…] → p'/q', e compara-se por produto
             * cruzado, que é como se comparam fracções sem as dividir. */
            RtCf c;
            rt_cf_de(sg, p, q, &c);
            if(c.n > maior_n) maior_n = c.n;
            long p2, q2;
            if(!c.saturou && rt_cf_para(&c, &p2, &q2)){
                if(p2*q == p*q2) cf_volta++;      /* a MESMA fracção, exactamente */
            }

            /* CAMINHO 2: pelo double, que é o que a libc faz. O valor v = p/q em vírgula
             * flutuante, e a volta v·q comparada com p — em inteiros, sem tolerância. */
            {
                double v = (double)p / (double)q;
                double back = v * (double)q;
                if(back == (double)p) dbl_volta++;
            }
        }
    }
    printf("      decimais de 3 casas varridos: %ld\n", total);
    printf("      voltam EXACTOS pela fraccao continua: %ld  (%ld%%)\n", cf_volta, 100*cf_volta/total);
    printf("      voltam EXACTOS pelo double:           %ld  (%ld%%)\n", dbl_volta, 100*dbl_volta/total);
    printf("      e a palavra mais comprida tem %ld quocientes\n\n", maior_n);
    ok("A IDA E VOLTA PELA PALAVRA E' EXACTA EM TODOS, e a comparacao e' por produto cruzado"
       " — a MESMA fraccao, e nao uma que difere abaixo de uma regua. Sao 10000 decimais de"
       " tres casas, e nenhum se perde",
       cf_volta == total && total == 10000);
    ok("E O DOUBLE NAO A FAZ, que e' o lado sem o qual a asserção acima nao dizia nada: pelo"
       " mesmo caminho, com os mesmos numeros, ele perde uma parte deles — porque 0,1 nao e'"
       " um decimo em base dois. Este medidor nao afirma que a palavra e' boa: mostra os dois"
       " numeros lado a lado, e a diferenca entre eles e' o motivo de existir",
       dbl_volta < total && dbl_volta > 0);
}

/* ── §E3 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E3  A palavra é EUCLIDES — os mesmos quocientes do mdc, noutra coluna.\n\n");
{
    /* «Euclides = MDC = Bézout = FC é a MESMA descida em colunas diferentes». Mede-se:
     * os quocientes da fracção contínua de p/q são os quocientes do algoritmo do mdc,
     * e o último resto não nulo é o mdc. Duas rotas pelo mesmo objecto. */
    long pares = 0, bate_q = 0, bate_mdc = 0;
    for(long p = 1; p <= 60; p++) for(long q = 1; q <= 60; q++){
        RtCf c;
        rt_cf_de(1, p, q, &c);
        if(c.saturou) continue;
        pares++;
        /* o mdc, pela descida de Euclides escrita aqui, e os seus quocientes */
        long P = p, Q = q; int i = 0, igual = 1;
        while(Q != 0){
            long quo = P / Q, r = P % Q;
            if(i >= c.n || c.a[i] != quo) igual = 0;
            i++; P = Q; Q = r;
        }
        if(igual && i == c.n) bate_q++;
        if(P == rt_mdc(p, q)) bate_mdc++;         /* e o último resto não nulo É o mdc */
    }
    printf("      pares (p,q) em [1,60]²: %ld ; os quocientes batem em %ld ; o ultimo resto\n"
           "      nao nulo e' o mdc em %ld\n\n", pares, bate_q, bate_mdc);
    ok("A PALAVRA E' EUCLIDES: os quocientes da fraccao continua de p/q SAO os quocientes do"
       " algoritmo do mdc, um a um e na mesma ordem, e o ultimo resto nao nulo E' o mdc — que"
       " e' a mesma descida lida em duas colunas. Varridos 3600 pares, e nenhum discorda",
       bate_q == pares && bate_mdc == pares && pares == 3600);
}

/* ── §E4 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E4  O cliente reconstrói o que quiser — e a divisão longa não arredonda por acaso.\n\n");
{
    struct { const char *t; int casas; const char *esp; } casos[] = {
        { "3.14159",  5, "3.14159"    },
        { "-0.1",     3, "-0.100"     },
        { "0.333",    6, "0.333000"   },
        { "2",        0, "2"          },
        { "1.5e-2",   4, "0.0150"     },
    };
    int n = (int)(sizeof casos / sizeof *casos), bons = 0;
    printf("      texto        casas   reconstruido   esperado\n");
    for(int k = 0; k < n; k++){
        int sg; long p, q;
        rt_le_decimal(casos[k].t, &sg, &p, &q);
        RtCf c;  rt_cf_de(sg, p, q, &c);
        long p2, q2;  rt_cf_para(&c, &p2, &q2);       /* passou pela PALAVRA, ida e volta */
        char d[64];
        rt_escreve_decimal(p2 < 0 ? -1 : 1, p2, q2, casos[k].casas, d, sizeof d);
        int bate = (strcmp(d, casos[k].esp) == 0);
        printf("      %-12s %-6d  %-14s %-10s %s\n", casos[k].t, casos[k].casas, d,
               casos[k].esp, bate ? "" : "  <-- NAO");
        if(bate) bons++;
    }
    printf("\n");
    ok("E O CLIENTE RECONSTROI: da palavra sai o decimal com as casas que ele pedir, por"
       " DIVISAO LONGA em inteiros — o resto e' sempre menor que q, logo nada transborda e"
       " nada arredonda por acaso. Cinco reconstrucoes, e cada uma passou pela palavra na"
       " ida e na volta antes de ser escrita",
       bons == n && n == 5);

    /* e a outra representação que a mesma palavra dá: os CONVERGENTES, que são a
     * aproximação racional óptima em cada denominador */
    int sg; long p, q;
    rt_le_decimal("3.14159", &sg, &p, &q);
    RtCf c; rt_cf_de(sg, p, q, &c);
    unsigned long ua[RT_CF_MAX], up[RT_CF_MAX], uq[RT_CF_MAX];
    for(int i = 0; i < c.n; i++) ua[i] = (unsigned long)c.a[i];
    int nc = nt_convergentes(ua, c.n, up, uq);
    printf("      e a MESMA palavra da outra representacao — os convergentes de 3.14159:\n      ");
    for(int i = 0; i < nc && i < 6; i++) printf("%lu/%lu  ", up[i], uq[i]);
    printf("\n      (o 22/7 e o 355/113 sao os de sempre, e sairam da palavra sem se pedirem)\n\n");
    long achou_227 = 0, achou_355 = 0;
    for(int i = 0; i < nc; i++){
        if(up[i] == 22 && uq[i] == 7) achou_227 = 1;
        if(up[i] == 355 && uq[i] == 113) achou_355 = 1;
    }
    ok("E A MESMA PALAVRA DA' OUTRA REPRESENTACAO: os convergentes, que sao a melhor"
       " aproximacao racional em cada denominador. De «3.14159» saem 22/7 e 355/113 sem"
       " ninguem os pedir — e e' este o sentido de «reconstroi o que quiser»: a palavra"
       " nao e' UMA representacao, e' o objecto de que TODAS elas se tiram",
       achou_227 && achou_355 && nc > 3);
}

/* ── §E5 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E5  A saturação conta-se em lugar SEPARADO — e há onde ela acontece.\n\n");
{
    /* `thm:meta-inducao`: falha de representação ≠ contra-exemplo. Aqui a representação
     * tem dois tectos declarados — o comprimento da palavra e o long dos produtos — e os
     * dois têm de ter onde bater, senão «nunca satura» valia por nunca se lhe pedir. */
    long recusou_texto = 0, coube = 0, tentou = 0;
    const char *grandes[] = {
        "1234567890123456789.5",           /* o inteiro sozinho já enche o long */
        "0.12345678901234567890",          /* e aqui é o denominador */
        "1e30",                            /* e aqui é o expoente */
        "3.14159",                         /* este cabe: é o controlo do outro lado */
    };
    for(int k = 0; k < 4; k++){
        int sg; long p, q;
        tentou++;
        if(rt_le_decimal(grandes[k], &sg, &p, &q)) coube++; else recusou_texto++;
    }
    /* e o tecto da PALAVRA: uma fracção cuja CF é comprida — os Fibonacci consecutivos
     * dão a palavra mais comprida possível para o tamanho, toda de uns. E o tecto
     * RT_CF_MAX tem de ter onde bater: um #define que ninguém testa é documentação e
     * não limite, e este ficheiro seria o sítio errado para aprender isso outra vez. */
    long uns = 0, cf_n = 0, saturou_palavra = 0, coube_palavra = 0, fib_passos = 0;
    {
        long f1 = 1, f2 = 1;
        for(int i = 0; i < 40; i++){ long t = f1 + f2; f1 = f2; f2 = t; }
        RtCf c; rt_cf_de(1, f2, f1, &c);
        cf_n = c.n;
        for(int i = 0; i < c.n; i++) if(c.a[i] == 1) uns++;

        /* e continua-se a subir Fibonacci até a palavra NÃO CABER: aí `saturou` acende */
        long g1 = f1, g2 = f2;
        for(int i = 0; i < 40; i++){
            if(g2 > 4611686018427387903L - g1) break;      /* o long também tem tecto */
            long t = g1 + g2; g1 = g2; g2 = t;
            RtCf d; rt_cf_de(1, g2, g1, &d);
            fib_passos++;
            if(d.saturou) saturou_palavra++; else coube_palavra++;
        }
    }
    /* e o outro tecto, o dos produtos: `nt_convergentes` da aritmetica.h pára quando o
     * produto deixaria de caber, e conta em `nt_saturou` — pede-se-lhe uma palavra longa
     * de propósito para ver que ele sabe recusar. */
    long conv_parou = 0;
    {
        unsigned long ea[RT_CF_MAX], ep[RT_CF_MAX], eq[RT_CF_MAX];
        for(int i = 0; i < RT_CF_MAX; i++) ea[i] = 1;      /* a palavra do ouro, toda de uns */
        int nc2 = nt_convergentes(ea, RT_CF_MAX, ep, eq);
        conv_parou = (nc2 < RT_CF_MAX);
    }
    printf("      textos que nao cabem no long: %ld de %ld (e %ld cabe, que e' o controlo)\n",
           recusou_texto, tentou, coube);
    printf("      a palavra de dois Fibonacci consecutivos tem %ld quocientes, %ld deles UNS\n",
           cf_n, uns);
    printf("      subindo mais Fibonacci: %ld passos, a palavra COUBE em %ld e nao coube em %ld\n",
           fib_passos, coube_palavra, saturou_palavra);
    printf("      e a aritmetica.h contou %ld saturacoes — os convergentes pararam antes do fim: %s\n\n",
           nt_saturou, conv_parou ? "sim" : "NAO");
    ok("A SATURACAO TEM ONDE ACONTECER, e por isso «nao satura» quer dizer alguma coisa:"
       " tres textos que nao cabem no long sao RECUSADOS — o inteiro, o denominador e o"
       " expoente, um de cada — e o quarto cabe, que e' o controlo do outro lado. Nada e'"
       " cortado calado: quem nao cabe volta zero e diz-se",
       recusou_texto == 3 && coube == 1 && tentou == 4);
    ok("E A PALAVRA MAIS COMPRIDA E' A DO OURO: dois Fibonacci consecutivos dao a fraccao"
       " continua toda de UNS, que e' a mais lenta a convergir e a mais comprida para o"
       " tamanho — o real mais lento, medido aqui como o pior caso da representacao. Nao e'"
       " coincidencia: e' a mesma extremalidade do phi que o geometrico.tex prova",
       cf_n > 30 && uns == cf_n - 1);
    ok("E OS DOIS TECTOS TEM ONDE BATER, que e' o que os torna limites e nao comentarios: o"
       " RT_CF_MAX da palavra acende ao subir Fibonacci — ha' passos onde cabe e passos onde"
       " NAO cabe, e quem nao cabe marca `saturou` em vez de sair cortado —, e os"
       " convergentes da aritmetica.h param antes do fim quando o produto deixaria de caber,"
       " a contar em nt_saturou. A saturacao esta' nesta assercao e em nenhuma outra: as"
       " sete de cima nao a veem",
       saturou_palavra > 0 && coube_palavra > 0 && conv_parou && nt_saturou > 0);
}

/* ── §E6 ─────────────────────────────────────────────────────────────────────── */
printf("\n§E6  O PIPE FECHADO: o MMC à entrada, e daí em diante não há vírgula.\n\n");
{
    /* A ENTREGA resolvia a SAÍDA. A entrada estava a meio: cada texto virava p/q sozinho,
     * e um conjunto de dados ficava com denominadores diferentes — o que obriga a cruzar
     * fracções a cada operação. O MMC dos denominadores resolve-o de uma vez:
     *
     *      textos ──► UNIDADE (mmc dos q) ──► inteiros ──► operar ──► palavra ──► cliente
     *
     * e no meio, entre a unidade e a palavra, NÃO HÁ VÍRGULA em sítio nenhum. */
    const char *dados[] = { "0.6", "0.45", "1.25", "2.5", "3", "0.125" };
    const int N = 6;
    long u = rt_unidade_comum(dados, N);
    long z[8];
    int conv = rt_para_unidade(dados, N, u, z);

    printf("      entrada:   ");
    for(int i = 0; i < N; i++) printf("%s ", dados[i]);
    printf("\n      unidade:   %ld  (o MMC dos denominadores)\n      inteiros:  ", u);
    for(int i = 0; i < conv; i++) printf("%ld ", z[i]);
    printf("\n");

    /* AGORA OPERA-SE, e tudo é inteiro. Três operações que a casa usa: a soma (que em
     * unidade comum é a soma dos inteiros), o produto directo, e o cruzado. */
    long soma = 0;
    for(int i = 0; i < conv; i++) soma += z[i];
    long a3[3] = { z[0], z[1], z[2] }, b3[3] = { z[3], z[4], z[5] };
    long dir = rt_dir(a3, b3, 3), cr[3];
    rt_cruz3(a3, b3, cr);
    long lag = rt_lagrange(a3, b3, 3);
    printf("      soma = %ld/%ld ; Dir = %ld/%ld² ; Cruz = (%ld,%ld,%ld)/%ld² ; Lagrange fecha: %s\n",
           soma, u, dir, u, cr[0], cr[1], cr[2], u, lag ? "sim" : "NAO");

    /* E A SAÍDA: cada resultado sai como PALAVRA, com o denominador que a unidade dá. */
    RtCf cs;  rt_cf_de(soma < 0 ? -1 : 1, soma, u, &cs);
    long ps, qs;  rt_cf_para(&cs, &ps, &qs);
    char dec[32];  rt_escreve_decimal(ps < 0 ? -1 : 1, ps, qs, 4, dec, sizeof dec);
    printf("      a soma entregue: palavra [");
    for(int i = 0; i < cs.n; i++) printf("%ld%s", cs.a[i], i+1 < cs.n ? ";" : "");
    printf("] = %ld/%ld = %s\n", ps, qs, dec);

    /* O CONTROLO DO PIPE: a soma dos racionais, calculada pelo caminho ANTIGO — cruzando
     * fracções uma a uma, sem unidade comum — tem de dar o MESMO. São dois caminhos, e o
     * segundo não passa pelo MMC. */
    long an = 0, ad = 1;
    for(int i = 0; i < N; i++){
        int sg; long p2, q2;
        rt_le_decimal(dados[i], &sg, &p2, &q2);
        long nn = an*q2 + sg*p2*ad, nd = ad*q2;
        long g = rt_mdc(nn, nd); if(g > 1){ nn /= g; nd /= g; }
        an = nn; ad = nd;
    }
    int pipe_bate = (soma * ad == an * u);        /* soma/u == an/ad, por produto cruzado */
    printf("      e o CONTROLO, somando fracção a fracção sem unidade comum: %ld/%ld — bate: %s\n\n",
           an, ad, pipe_bate ? "sim" : "NAO");

    /* e o que o pipe RECUSA: um decimal com casas a mais faz o MMC não caber, e diz-se */
    const char *fundo[] = { "0.1", "0.123456789012345678901" };
    long u_fundo = rt_unidade_comum(fundo, 2);

    ok("O PIPE FECHA DOS DOIS LADOS: o MMC dos denominadores entra a' ENTRADA e leva o"
       " conjunto todo a INTEIROS de uma vez — «0.6 0.45 1.25 2.5 3 0.125» tem unidade 1000"
       " —, opera-se la' dentro (soma, Dir, Cruz, Lagrange) sem uma virgula, e a saida e' a"
       " PALAVRA com o denominador que a unidade da'. Entre a unidade e a palavra nao ha'"
       " virgula em sitio nenhum",
       u == 1000 && conv == N && lag && u_fundo == 0);

    ok("E O CONTROLO NAO PASSA PELO MMC: a mesma soma calculada a' antiga, cruzando fraccoes"
       " uma a uma e reduzindo pelo mdc a cada passo, da' o MESMO racional — comparado por"
       " produto cruzado. Sao dois caminhos pelo mesmo objecto, e o segundo nao sabe que a"
       " unidade comum existe. E o pipe RECUSA quando a unidade nao cabe no long, em vez de"
       " truncar calado",
       pipe_bate && u_fundo == 0);
}

printf("\n  ─────────────────────────────────────────────────────────────\n");
printf("  O que sai daqui sao longs. O decimal, a fraccao, os convergentes e o\n");
printf("  double sao todos reconstrucoes DELES — e nenhuma delas e' o numero.\n");
printf("  O numero e' a palavra.\n");
printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
return falhas ? 1 : 0;
}
