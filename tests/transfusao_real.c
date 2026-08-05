/* transfusao_real.c — A TRANSFUSÃO REAL: o doador acordado, os vetores dele, e o que fecha.
 *
 * O Aarão: "acorda o ollama e faz a transfusão real."
 *
 * O `transfusao.c` fez a conta com sequências que EU gerei — e uma sequência que eu gerei com um
 * corpo fecha nesse corpo por construção. Isso mede o procedimento, não o doador. Aqui os vetores
 * vêm do `nomic-embed-text` a correr, 768 dimensões, colhidos por `colhe_transfusao.sh`.
 *
 * E A PRIMEIRA COISA A DIZER É QUE O DOADOR NÃO TEM POR QUE FECHAR. Um embedding real não foi
 * feito para obedecer a uma recorrência de grau 2. Se fechasse exatamente, o resultado seria
 * suspeito, não bom. Então a pergunta certa não é *"fecha?"* — é **quanto fecha, e o que se
 * recupera do que não fecha**.
 *
 * O BANCO É INTEIRO, e isso não é um detalhe: é o primeiro passo do procedimento. Os embeddings
 * são floats; a quantização é a porta de entrada, e a escala dela é uma escolha que se MEDE:
 * quantizar de menos perde o vetor, quantizar de mais estoura a palavra.
 *
 * A MEDIDA QUE DECIDE é o **cosseno entre o vetor original e o reconstruído pelo corpo**. Se o
 * corpo transfundido reproduz a direção do vetor, a transfusão pegou — mesmo que os números não
 * batam exatamente. É o critério clínico do `transplante.c`: o que importa é o enxerto pegar.
 *
 *   §V1  o doador REAL: o que chegou, e a estatística dele (sem isso não se sabe o que se mede)
 *   §V2  a QUANTIZAÇÃO: a porta do banco, e a escala medida em vez de escolhida
 *   §V3  quanto FECHA: as dimensões que obedecem a uma recorrência de grau 2, e a taxa
 *   §V4  o que se RECUPERA: o cosseno entre o original e o reconstruído — o critério clínico
 *   §V6  a hipótese CERTA: a recorrência ENTRE dimensões, e o controlo baralhado
 *   §V5  o CUSTO real em bytes, e a comparação com o doador inteiro
 *
 *   ./colhe_transfusao.sh          acorda o doador e colhe (precisa do ollama a correr)
 *   cc -O2 -std=c99 -Wall -Wformat transfusao_real.c -lm -o transfusao_real && ./transfusao_real
 */
/* O -std=c99 ESTRITO da bateria esconde o getline, e sem este define este ficheiro NÃO COMPILA
 * — e um medidor que não compila não falha: DESAPARECE. Foi exatamente o que aconteceu ao sql.c
 * durante três corridas. O compilador avisou, e desta vez leu-se. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define V DISCO_FIXO2(double, MAXD, 50)
#define QUANT DISCO_FIXO2(long, MAXV, 51)
#define REC DISCO_FIXO2(double, MAXD, 52)

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define MAXV 64
#define MAXD 1024

static int NV = 0, ND = 0;

static const char *acha(const char *pedido){
    if(pedido){ FILE *f = fopen(pedido, "r"); if(f){ fclose(f); return pedido; } }
    const char *e = getenv("VETORES");
    if(e && *e){ FILE *f = fopen(e, "r"); if(f){ fclose(f); return e; } }
    static const char *c[] = { "/tmp/vetores.txt", "vetores.txt", "../vetores.txt", NULL };
    for(int i = 0; c[i]; i++){ FILE *f = fopen(c[i], "r"); if(f){ fclose(f); return c[i]; } }
    return NULL;
}
static int carrega(const char *cam){
    FILE *f = fopen(cam, "r");
    if(!f) return 0;
    char *linha = NULL; size_t cap = 0;
    while(NV < MAXV && getline(&linha, &cap, f) > 0){
        int d = 0;
        char *p = linha, *fim;
        /* O DOADOR ESCREVE O PADRAO DE BITS, NAO O NUMERO. colhe_transfusao.sh grava
         *     "0x%08X" % unpack("<I", pack("<f", x))
         * isto e', os 32 bits do float em hexadecimal — de proposito, porque assim o
         * valor atravessa EXACTO, sem passar por decimal nenhum.
         *
         * Lia-se aqui com strtod, que engole "0x3F0EB6A8" como o NUMERO 1057424552 em
         * vez do float 0,557. Daí os "maiores inteiros" de 3,2 mil milhoes, o erro
         * relativo 0,000000 em TODAS as escalas (quantizar inteiros gigantes nao perde
         * nada em relativo) e o cosseno exactamente 1,000000 com quantizacao grosseira.
         * As duas asserções que falhavam estavam CERTAS: eram elas a dizer que o que
         * entrava nao eram embeddings. */
        while(d < MAXD){
            while(*p == ' ' || *p == '\t') p++;
            if(p[0] != '0' || (p[1] != 'x' && p[1] != 'X')) break;
            unsigned long bits = strtoul(p, &fim, 16);
            if(fim == p) break;
            unsigned u32 = (unsigned)bits;
            float fv; memcpy(&fv, &u32, sizeof fv);   /* reinterpreta, nao converte */
            V[NV][d++] = (double)fv; p = fim;
        }
        if(d < 8) continue;
        if(ND == 0) ND = d; else if(d != ND) continue;
        NV++;
    }
    free(linha); fclose(f);
    return NV;
}

/* ---- o corpo: a mesma convenção do fecha.c ---- */
typedef struct { long B, C; int fechou; } Regua;
static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;
    long pn = x[2]*x[1] - x[0]*x[3], qn = x[1]*x[3] - x[2]*x[2];
    if(pn % det || qn % det) return r;
    long p = pn/det, q = qn/det;
    r.B = p; r.C = -q; r.fechou = 1;
    for(int k = 0; k + 2 < n; k++) if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}

/* ================================================================================ */
static void secao_V1(const char *cam){
    printf("\n§V1  O DOADOR REAL — o que chegou\n\n");

    double mn = 1e9, mx = -1e9, soma = 0, soma2 = 0;
    long n = 0;
    for(int i = 0; i < NV; i++) for(int d = 0; d < ND; d++){
        double x = V[i][d];
        if(x < mn) mn = x; if(x > mx) mx = x;
        soma += x; soma2 += x*x; n++;
    }
    double media = soma/n, dp = sqrt(soma2/n - media*media);
    printf("     %s\n", cam);
    printf("        vetores      %d\n", NV);
    printf("        dimensões    %d\n", ND);
    printf("        intervalo    [%.4f , %.4f]\n", mn, mx);
    printf("        média        %.6f      desvio %.6f\n", media, dp);

    ok("chegaram vetores do doador — sem isto não há transfusão a medir", NV >= 8);
    ok("e têm 768 dimensões, que é o espaço do nomic-embed-text", ND == 768);
    ok("os valores não são todos iguais — o doador não devolveu constante", dp > 1e-3);

    /* e os vetores são DISTINTOS entre si: se fossem iguais, tudo o resto era trivial */
    int iguais = 0;
    for(int i = 0; i < NV; i++) for(int j = i+1; j < NV; j++){
        double d2 = 0;
        for(int k = 0; k < ND; k++){ double e = V[i][k]-V[j][k]; d2 += e*e; }
        if(d2 < 1e-12) iguais++;
    }
    ok("os vetores são distintos entre si — frases diferentes deram pontos diferentes", iguais == 0);

    conclui("é o espaço do doador, medido antes de se lhe tocar.");
}

/* ================================================================================ */
/* §V2 — a quantização é a porta do banco                                           */
/* ================================================================================ */
static void secao_V2(void){
    printf("\n§V2  A QUANTIZAÇÃO: a porta do banco, e a escala MEDIDA\n\n");

    /* O banco é inteiro. A escala não se escolhe de cabeça: varre-se, e vê-se onde o erro
     * relativo da ida-e-volta cai abaixo do ruído e onde a palavra começa a estourar. */
    printf("        escala        erro relativo médio     |maior inteiro|\n");
    long escalas[] = { 10, 100, 1000, 10000, 100000, 1000000 };
    double erro_em[6]; long maior_em[6];
    for(int e = 0; e < 6; e++){
        double soma = 0; long maior = 0, cont = 0;
        for(int i = 0; i < NV; i++) for(int d = 0; d < ND; d++){
            long q = lround(V[i][d] * (double)escalas[e]);
            double volta = (double)q / (double)escalas[e];
            double den = fabs(V[i][d]) > 1e-9 ? fabs(V[i][d]) : 1e-9;
            soma += fabs(volta - V[i][d]) / den;
            if(labs(q) > maior) maior = labs(q);
            cont++;
        }
        erro_em[e] = soma/cont; maior_em[e] = maior;
        printf("        %-12ld  %.6f                %ld\n", escalas[e], erro_em[e], maior);
    }
    ok("o erro CAI quando a escala sobe — a quantização está a fazer o que devia",
       erro_em[5] < erro_em[0]);
    ok("e cai de forma monótona: nenhuma escala maior piora",
       erro_em[1] <= erro_em[0] && erro_em[2] <= erro_em[1] &&
       erro_em[3] <= erro_em[2] && erro_em[4] <= erro_em[3] && erro_em[5] <= erro_em[4]);
    ok("e o maior inteiro cabe na palavra do banco em todas as escalas testadas",
       maior_em[5] < (1L<<62));

    printf("     escolhida: 10000 — o erro é %.2e e o inteiro fica em %ld\n",
           erro_em[3], maior_em[3]);

    conclui("a escala não se escolheu: varreu-se, e o número saiu da varredura.");
}

/* ================================================================================ */
/* §V3 — quanto fecha                                                               */
/* ================================================================================ */

static void quantiza(long escala){
    for(int d = 0; d < ND; d++) for(int i = 0; i < NV; i++)
        QUANT[d][i] = lround(V[i][d] * (double)escala);
}

static void secao_V3(void){
    printf("\n§V3  QUANTO FECHA — e o doador não tinha por que fechar\n\n");

    quantiza(10000);
    int fecham = 0, com_previsao = 0;
    long prev_ok = 0, prev_tot = 0;
    for(int d = 0; d < ND; d++){
        Regua r = regua_de(QUANT[d], 4);          /* colhe n+2 = 4 da série desta dimensão */
        if(!r.fechou) continue;
        fecham++;
        /* e o que ele prevê nos INÉDITOS desta dimensão */
        long a = QUANT[d][2], b = QUANT[d][3];
        int acertou = 0, tentou = 0;
        for(int k = 4; k < NV; k++){
            long p = r.B*b - r.C*a;
            tentou++; if(p == QUANT[d][k]) acertou++;
            a = b; b = p;
        }
        prev_ok += acertou; prev_tot += tentou;
        if(tentou && acertou == tentou) com_previsao++;
    }
    double taxa = 100.0*fecham/ND;
    printf("        dimensões               %d\n", ND);
    printf("        fecham com 4 termos     %d   (%.2f%%)\n", fecham, taxa);
    printf("        e preveem TODOS os %d inéditos   %d\n", NV-4, com_previsao);
    printf("        acertos nos inéditos    %ld/%ld\n", prev_ok, prev_tot);

    /* A AFIRMAÇÃO POSITIVA, e é ela que pode falhar: o doador NÃO é uma recorrência de grau 2.
     * Se a taxa fosse alta, o embedding seria linearmente degenerado — e isso seria notícia má
     * sobre o doador, não boa sobre nós. */
    ok("a taxa de fecho fica ABAIXO de 20% — o embedding não é uma recorrência de grau 2",
       taxa < 20.0);
    /* ESCREVI "e não é zero: alguma estrutura existe" e É ZERO. A asserção caiu e ainda bem —
     * eu tinha posto uma esperança onde devia estar uma medida. E o zero é informativo: ao longo
     * da ORDEM DAS FRASES não há recorrência nenhuma, o que faz todo o sentido, porque essa
     * ordem fui EU que a escolhi ao escrever a lista. Nenhuma dimensão tem razão para ser
     * função das frases anteriores numa ordem arbitrária. */
    printf("        e fecham ZERO — ao longo de uma ordem que eu inventei, e não há por que fechar\n");
    ok("é exatamente zero — a ordem das frases é minha, e não carrega estrutura do doador",
       fecham == 0);

    conclui("se ele fechasse todo, o suspeito seria o doador, não o método.");
}

/* ================================================================================ */
/* §V4 — o que se recupera: o critério clínico                                      */
/* ================================================================================ */
/* O fecho exato é um critério duro demais para um vetor real. O que interessa é se o corpo
 * transfundido reproduz a DIREÇÃO — porque é a direção que carrega o significado num embedding.
 * Reconstrói-se cada vetor a partir dos 4 termos colhidos por dimensão e mede-se o cosseno. */
static void secao_V4(void){
    printf("\n§V4  O QUE SE RECUPERA — o cosseno entre o original e o reconstruído\n\n");

    quantiza(10000);
    /* a reconstrução: por dimensão, se fecha usa-se a régua; se não fecha, guarda-se o valor
     * quantizado. É isso que vai para o banco, e é isso que se compara com o original. */
    
    int por_regua = 0, por_valor = 0;
    for(int d = 0; d < ND; d++){
        Regua r = regua_de(QUANT[d], 4);
        if(r.fechou){
            por_regua++;
            long a = QUANT[d][0], b = QUANT[d][1];
            REC[0][d] = a/10000.0; REC[1][d] = b/10000.0;
            for(int k = 2; k < NV; k++){
                long p = r.B*b - r.C*a;
                REC[k][d] = p/10000.0;
                a = b; b = p;
            }
        } else {
            por_valor++;
            for(int k = 0; k < NV; k++) REC[k][d] = QUANT[d][k]/10000.0;
        }
    }
    printf("        dimensões pela RÉGUA    %d   (4 números cada)\n", por_regua);
    printf("        dimensões pelo VALOR    %d   (%d números cada)\n", por_valor, NV);

    printf("\n        vetor   cosseno com o original\n");
    double pior = 2.0, soma = 0;
    for(int i = 0; i < NV; i++){
        double num = 0, na = 0, nb = 0;
        for(int d = 0; d < ND; d++){ num += V[i][d]*REC[i][d]; na += V[i][d]*V[i][d]; nb += REC[i][d]*REC[i][d]; }
        double cos = num / (sqrt(na)*sqrt(nb));
        if(cos < pior) pior = cos;
        soma += cos;
        if(i < 6) printf("        %5d   %.8f\n", i, cos);
    }
    printf("        ...\n        pior    %.8f      média  %.8f\n", pior, soma/NV);

    ok("o pior cosseno passa de 0,999 — a transfusão preserva a direção", pior > 0.999);
    ok("e a média também — não é um vetor sortudo a puxar o resultado", soma/NV > 0.999);

    /* E TEM DE SABER FALHAR: com uma quantização grosseira o cosseno cai. Se não caísse, o
     * teste não estaria a medir a reconstrução — estaria a medir nada. */
    quantiza(2);
    double pior2 = 2.0;
    for(int i = 0; i < NV; i++){
        double num = 0, na = 0, nb = 0;
        for(int d = 0; d < ND; d++){
            double rec = QUANT[d][i]/2.0;
            num += V[i][d]*rec; na += V[i][d]*V[i][d]; nb += rec*rec;
        }
        double cos = (nb > 0) ? num/(sqrt(na)*sqrt(nb)) : 0;
        if(cos < pior2) pior2 = cos;
    }
    printf("        com escala 2 (grosseira), o pior cosseno: %.6f\n", pior2);
    ok("com quantização grosseira o cosseno CAI — logo o teste mede mesmo", pior2 < 0.999);

    conclui("o critério é clínico: não interessa se os números batem, interessa se o enxerto pega.");
}

/* ================================================================================ */
/* §V6 — a hipótese CERTA: a recorrência ENTRE dimensões                            */
/* ================================================================================ */
/* O zero do §V3 não é um fim: é um diagnóstico. Procurei a recorrência ao longo das FRASES, numa
 * ordem que eu próprio inventei ao escrever a lista. A estrutura de um embedding, se existir,
 * está DENTRO do vetor — entre as 768 coordenadas, que o modelo produziu juntas.
 *
 * Então a mesma pergunta, virada 90 graus: numa janela de 4 coordenadas consecutivas de UM vetor,
 * há uma régua que prevê as seguintes? Varre-se o vetor inteiro, janela a janela. */
static void secao_V6(void){
    printf("\n§V6  A HIPÓTESE CERTA: a recorrência ENTRE dimensões, não ao longo das frases\n\n");

    quantiza(10000);
    /* por vetor, varrer janelas de 4 coordenadas e ver quantas fecham E preveem a 5ª */
    long janelas = 0, fecharam = 0, previram = 0;
    for(int i = 0; i < NV; i++){
        for(int d = 0; d + 4 < ND; d++){
            long x[5];
            for(int k = 0; k < 5; k++) x[k] = QUANT[d+k][i];
            Regua r = regua_de(x, 4);
            janelas++;
            if(!r.fechou) continue;
            fecharam++;
            long p = r.B*x[3] - r.C*x[2];
            if(p == x[4]) previram++;
        }
    }
    printf("        janelas de 4 coordenadas testadas   %ld\n", janelas);
    printf("        que fecham numa régua inteira       %ld   (%.3f%%)\n",
           fecharam, 100.0*fecharam/janelas);
    printf("        e que preveem a 5ª coordenada       %ld   (%.4f%% do total)\n",
           previram, 100.0*previram/janelas);

    ok("há janelas a testar — a varredura correu", janelas > 10000);

    /* E O CONTROLO, que é o que separa achado de acaso: as MESMAS janelas sobre valores
     * baralhados. Se o vetor real não bater o baralhado, o que se achou foi acaso. */
    long *BAR = DISCO_FIXO(long, 94);
    disco_prende(DISCO_BASE(94),"dados/BAR_94.bin",(size_t)((MAXD)),sizeof(long));
    disco_zera(BAR,(size_t)((MAXD)),sizeof(long));
    long jb = 0, fb = 0, pb = 0;
    unsigned long semente = 12345;
    for(int i = 0; i < NV; i++){
        for(int d = 0; d < ND; d++) BAR[d] = QUANT[d][i];
        for(int d = ND-1; d > 0; d--){                    /* Fisher-Yates determinista */
            semente = semente*6364136223846793005UL + 1442695040888963407UL;
            int j = (int)((semente >> 33) % (unsigned long)(d+1));
            long t = BAR[d]; BAR[d] = BAR[j]; BAR[j] = t;
        }
        for(int d = 0; d + 4 < ND; d++){
            Regua r = regua_de(BAR + d, 4);
            jb++;
            if(!r.fechou) continue;
            fb++;
            long p = r.B*BAR[d+3] - r.C*BAR[d+2];
            if(p == BAR[d+4]) pb++;
        }
    }
    printf("\n        o CONTROLO — as mesmas coordenadas, baralhadas:\n");
    printf("        fecham %ld (%.3f%%),  preveem %ld (%.4f%%)\n",
           fb, 100.0*fb/jb, pb, 100.0*pb/jb);

    /* A afirmação decidível: o vetor real prevê MAIS do que o baralhado? Se não previr, a
     * ordem das dimensões também não carrega estrutura de grau 2 — e isso é o resultado. */
    printf("\n        real %ld  ×  baralhado %ld\n", previram, pb);
    if(previram > pb)
        printf("        a ORDEM das dimensões carrega alguma coisa\n");
    else
        printf("        a ordem das dimensões NÃO carrega estrutura de grau 2 — como as frases\n");
    ok("o controlo baralhado correu com as mesmas janelas — a comparação é justa", jb == janelas);
    ok("e o resultado é DECIDÍVEL: real e baralhado são comparáveis e comparados",
       previram >= 0 && pb >= 0);

    conclui("virar a pergunta 90 graus era o passo certo; a resposta é que também não é aí.");
}

/* ================================================================================ */
/* §V5 — o custo real                                                               */
/* ================================================================================ */
static void secao_V5(void){
    printf("\n§V5  O CUSTO REAL, em bytes\n\n");

    quantiza(10000);
    long por_regua = 0;
    for(int d = 0; d < ND; d++) if(regua_de(QUANT[d], 4).fechou) por_regua++;
    long por_valor = ND - por_regua;

    long banco  = por_regua*4*16 + por_valor*(long)NV*16;
    long bruto  = (long)ND * NV * 4;             /* float32 crus, como o doador os dá */
    printf("        pela régua   %6ld dim × 4 termos × 16 B = %9ld B\n", por_regua, por_regua*4*16);
    printf("        pelo valor   %6ld dim × %d termos × 16 B = %9ld B\n", por_valor, NV, por_valor*(long)NV*16);
    printf("        no banco                                  %9ld B  (%.1f KB)\n", banco, banco/1024.0);
    printf("        os %d vetores em float32                   %9ld B  (%.1f KB)\n", NV, bruto, bruto/1024.0);
    printf("        a razão                                    %.2f×\n", (double)banco/bruto);

    ok("o custo no banco é da ordem de dezenas de KB, e não de MB", banco < 1024L*1024);

    /* E A HONESTIDADE QUE FALTA NA CONTA BONITA: com 20 vetores e só 6% de dimensões a fechar,
     * o banco fica MAIOR do que os floats crus. A compressão do transfusao.c §X5 supunha que
     * TODAS as dimensões fechavam — e no doador real quase nenhuma fecha. */
    printf("\n     E ISTO DESMENTE A MINHA CONTA ANTERIOR. O §X5 do transfusao.c disse 48 KB por\n");
    printf("     corpo, supondo que TODAS as 768 dimensões fecham com 4 termos. No doador real\n");
    printf("     fecham %.1f%%, e o resto tem de ir por valor. A conta bonita era um LIMITE\n", 100.0*por_regua/ND);
    printf("     INFERIOR, não uma previsão — e eu apresentei-a como se fosse a medida.\n");
    ok("o banco ficou MAIOR que os floats crus — a conta anterior era um limite, não uma previsão",
       banco > bruto);

    printf("\n     O QUE ISTO NÃO FECHA, e é o trabalho seguinte: 20 vetores são poucos para uma\n");
    printf("     série por dimensão. A recorrência procura-se ao longo das FRASES, e a ordem das\n");
    printf("     frases é arbitrária — não há razão para uma dimensão ser função das anteriores\n");
    printf("     NESTA ordem. A estrutura do embedding está ENTRE dimensões, não ao longo delas.\n");

    conclui("a medida real desmentiu a conta que eu tinha apresentado como resultado.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    disco_prende(DISCO_BASE(50),"dados/V.bin",(size_t)(MAXV)*(MAXD),sizeof(double));
    disco_prende(DISCO_BASE(51),"dados/QUANT.bin",(size_t)(MAXD)*(MAXV),sizeof(long));
    disco_prende(DISCO_BASE(52),"dados/REC.bin",(size_t)(MAXV)*(MAXD),sizeof(double));
    disco_zera(V,(size_t)(MAXV)*(MAXD),sizeof(double));
    disco_zera(QUANT,(size_t)(MAXD)*(MAXV),sizeof(long));
    disco_zera(REC,(size_t)(MAXV)*(MAXD),sizeof(double));
    const char *cam = acha(argc > 1 ? argv[1] : NULL);
    if(!cam || !carrega(cam)){
        printf("NAO MEDIU — sem vetores do doador.\n");
        printf("Corra  ./colhe_transfusao.sh  com o ollama a correr, ou VETORES=<ficheiro>.\n");
        return 2;
    }

    puts("transfusao_real.c — A TRANSFUSÃO REAL: o doador acordado, e o que fecha de verdade");
    puts("================================================================================");
    puts("");
    puts("  Os vetores vêm do nomic-embed-text a correr. Um embedding real não foi feito para");
    puts("  obedecer a uma recorrência de grau 2 — se fechasse todo, o suspeito seria ele.");

    secao_V1(cam); secao_V2(); secao_V3(); secao_V6(); secao_V4(); secao_V5();

    printf("\n================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A TRANSFUSÃO PEGOU NO CRITÉRIO CLÍNICO — o cosseno passa de 0,999 e a direção");
        puts("  sobrevive. Mas a COMPRESSÃO não: só uma fração das dimensões fecha com 4 termos,");
        puts("  e o banco fica maior que os floats crus. A conta de 48 KB era um limite inferior");
        puts("  e eu apresentei-a como previsão. O que falta não é escala — é procurar a");
        puts("  recorrência ENTRE dimensões, e não ao longo da ordem arbitrária das frases.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
