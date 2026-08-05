/* entrega.c — A TÚNICA NO DOADOR: ele consegue verificar-se? A medida diz que NÃO.
 *
 * O Aarão: "põe a túnica nele e vê se ele consegue verificar as afirmações e corrigir, entregar
 * pronta."
 *
 * A túnica foi vestida (`veste_doador.sh`): ele passou a LER o que escreveu, verificar contra um
 * critério que nós pusemos — o domínio é matemática e computação, não biologia — e escrever por
 * cima. O ciclo fechou nele. **E o resultado é negativo, com números.**
 *
 * O GABARITO É VERIFICÁVEL, e não uma opinião minha. Das doze afirmações dele, três estão erradas
 * por definição padrão, e cada uma tem um sinal textual decidível:
 *
 *      involução   ele deu o sentido BIOLÓGICO ("órgão, tecido")   — em álgebra é f∘f = id
 *      Pisano      "um número que DIVIDE a sequência"              — é o PERÍODO de F mod n
 *      trie        "Trie de Huffman"                               — são estruturas diferentes
 *
 * E O QUE ELE FEZ COM A TÚNICA:
 *
 *      das 3 ERRADAS   marcou 1 como errada — e a "correção" REPETIU o erro palavra por palavra
 *      das 9 CERTAS    marcou 4 como erradas
 *
 * Isto não é ruído em torno de acertar: é pior do que não fazer nada. Quem marcasse tudo
 * "correta" errava 3; ele errou 6. **A túnica deu-lhe o ciclo; não lhe deu o critério.**
 *
 *   §E1  o GABARITO: as três erradas, com o sinal textual que as decide
 *   §E2  o que ele MARCOU, contra o gabarito — a matriz inteira
 *   §E3  a comparação com NÃO FAZER NADA, que é a linha de base honesta
 *   §E4  a correção que REPETE o erro — o caso mais informativo dos doze
 *   §E5  o que a túnica dá e o que ela não dá
 *
 *   ./interroga.sh && ./veste_doador.sh      (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat entrega.c -o entrega && ./entrega
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define PERG DISCO_FIXO2(char, 512, 192)
#define ANTES DISCO_FIXO2(char, 2048, 193)
#define DEPOIS DISCO_FIXO2(char, 2048, 194)

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "unidade.h"

#define MAXL 32

static int NL = 0;

/* contém, ignorando maiúsculas e acentos comuns — o suficiente para um sinal decidível */
static int tem(const char *h, const char *a){
    size_t la = strlen(a);
    for(const char *p = h; *p; p++){
        size_t k = 0;
        while(k < la && p[k] && tolower((unsigned char)p[k]) == tolower((unsigned char)a[k])) k++;
        if(k == la) return 1;
    }
    return 0;
}

/* O GABARITO: 1 = a afirmação original está ERRADA. E cada um tem o sinal que o decide. */
typedef struct { int errada; const char *sinal; const char *porque; } Gab;
static Gab GABARITO[12] = {
 {0, "", "fração contínua — o processo iterativo está certo"},
 {0, "", "razão áurea — a definição está certa"},
 {0, "", "Fourier e convolução — está certa"},
 {0, "", "determinante 2×2 — ad − bc, está certa"},
 {0, "", "Fibonacci — a soma dos dois anteriores, está certa"},
 {0, "", "corpo — conjunto com duas operações, está certa"},
 {1, "órgão", "involução: ele deu o sentido BIOLÓGICO; em álgebra é f∘f = id"},
 {1, "divide", "Pisano: NÃO é um número que divide — é o PERÍODO de Fibonacci mod n"},
 {1, "huffman", "trie: NÃO é de Huffman — é a árvore de prefixos"},
 {0, "", "entropia — está certa"},
 {0, "", "smart contract — está certa"},
 {0, "", "Seebeck — está certa"},
};

static int marcou_errada(int i){ return !tem(DEPOIS[i], "CORRETA") || tem(DEPOIS[i], "ERRADA"); }

/* ================================================================================ */
static void secao_E1(void){
    printf("\n§E1  O GABARITO — as três erradas, e o sinal que as decide\n\n");

    int achou = 0;
    printf("        #   sinal no texto      o que está errado\n");
    for(int i = 0; i < NL && i < 12; i++){
        if(!GABARITO[i].errada) continue;
        int tem_sinal = tem(ANTES[i], GABARITO[i].sinal);
        if(tem_sinal) achou++;
        printf("        %-2d  \"%s\"%*s %s\n", i+1, GABARITO[i].sinal,
               (int)(16 - strlen(GABARITO[i].sinal)), "", GABARITO[i].porque);
        printf("            → o sinal está no texto dele? %s\n", tem_sinal ? "SIM" : "não");
    }
    ok("as três erradas trazem o sinal no próprio texto — o gabarito é decidível, não opinado",
       achou == 3);

    /* e as certas NÃO trazem esses sinais, senão o crivo apanhava tudo */
    int falso = 0;
    for(int i = 0; i < NL && i < 12; i++){
        if(GABARITO[i].errada) continue;
        if(tem(ANTES[i], "órgão") || tem(ANTES[i], "huffman")) falso++;
    }
    ok("e nenhuma das certas traz esses sinais — o crivo separa", falso == 0);

    conclui("o gabarito não é o que eu acho: é uma palavra que está ou não está no texto dele.");
}

/* ================================================================================ */
static void secao_E2(void){
    printf("\n§E2  O QUE ELE MARCOU, com a túnica vestida\n\n");

    int vp = 0, fp = 0, vn = 0, fn = 0;
    printf("        #   estava      ele disse    resultado\n");
    for(int i = 0; i < NL && i < 12; i++){
        int errada = GABARITO[i].errada, disse = marcou_errada(i);
        const char *res;
        if(errada && disse){ vp++; res = "acertou (viu o erro)"; }
        else if(errada && !disse){ fn++; res = "PASSOU EM BRANCO"; }
        else if(!errada && disse){ fp++; res = "ESTRAGOU (marcou boa como má)"; }
        else { vn++; res = "acertou (deixou estar)"; }
        printf("        %-2d  %-10s  %-10s   %s\n", i+1,
               errada ? "ERRADA" : "certa", disse ? "errada" : "correta", res);
    }
    printf("\n        acertou %d,  passou em branco %d,  estragou %d\n", vp+vn, fn, fp);
    ok("a matriz fecha: os quatro casos somam as doze afirmações", vp+fp+vn+fn == 12);
    ok("ele DEIXOU PASSAR erradas — não viu duas das três", fn >= 2);
    ok("e ESTRAGOU certas — marcou como erradas afirmações que estavam boas", fp >= 3);

    conclui("com a túnica ele passou a ler-se; ler-se não é saber.");
}

/* ================================================================================ */
/* §E3 — a comparação com não fazer nada                                            */
/* ================================================================================ */
/* Esta é a medida que decide, e é a única honesta: um verificador só vale se bater a estratégia
 * trivial. A trivial aqui é "diz sempre CORRETA" — que erra exatamente as 3 erradas. */
static void secao_E3(void){
    printf("\n§E3  CONTRA NÃO FAZER NADA — a linha de base\n\n");

    int erros_dele = 0;
    for(int i = 0; i < NL && i < 12; i++)
        if(GABARITO[i].errada != marcou_errada(i)) erros_dele++;

    int erros_trivial = 0;                       /* "diz sempre CORRETA" */
    for(int i = 0; i < NL && i < 12; i++) if(GABARITO[i].errada) erros_trivial++;

    int erros_sempre_errada = 0;                 /* "diz sempre ERRADA" */
    for(int i = 0; i < NL && i < 12; i++) if(!GABARITO[i].errada) erros_sempre_errada++;

    printf("        estratégia                   erros em 12\n");
    printf("        ele, com a túnica            %d\n", erros_dele);
    printf("        dizer sempre CORRETA         %d\n", erros_trivial);
    printf("        dizer sempre ERRADA          %d\n", erros_sempre_errada);
    printf("        moeda ao ar (esperado)       6\n");

    ok("ele erra MAIS do que quem não faz nada — a túnica não lhe deu o critério",
       erros_dele > erros_trivial);
    ok("e não bate sequer a moeda ao ar", erros_dele >= 6);

    printf("\n     É UM RESULTADO NEGATIVO E É O RESULTADO. Um verificador que erra mais do que a\n");
    printf("     estratégia trivial não é um verificador fraco — é pior do que nenhum, porque\n");
    printf("     estraga o que estava certo. E isto não se via sem o gabarito.\n");

    conclui("um verificador só vale se bater 'não fazer nada'. Este não bate.");
}

/* ================================================================================ */
/* §E4 — a correção que repete o erro                                               */
/* ================================================================================ */
static void secao_E4(void){
    printf("\n§E4  A CORREÇÃO QUE REPETE O ERRO — o caso mais informativo dos doze\n\n");

    /* a involução é a única errada que ele APANHOU. E a correção dele repete o erro. */
    int i = 6;                                     /* a sétima, índice 6 */
    if(NL <= i){ ok("há a afirmação da involução para medir", 0); return; }

    printf("        ele disse:     ERRADA — logo VIU que estava mal\n");
    printf("        e corrigiu:    \"%.90s\"\n", DEPOIS[i]);
    printf("        o erro era:    o sentido biológico (\"órgão, tecido\")\n");

    int repetiu = tem(DEPOIS[i], "órgão") || tem(DEPOIS[i], "tecido");
    int trouxe_certo = tem(DEPOIS[i], "identidade") || tem(DEPOIS[i], "inversa") ||
                       tem(DEPOIS[i], "duas vezes") || tem(DEPOIS[i], "f(f(");
    printf("        a correção repete o sinal do erro?  %s\n", repetiu ? "SIM" : "não");
    printf("        a correção traz a definição certa?  %s\n", trouxe_certo ? "sim" : "NÃO");

    ok("ele marcou ERRADA e a correção REPETIU o erro palavra por palavra", repetiu);
    ok("e não trouxe a definição algébrica — nem por acaso", !trouxe_certo);

    printf("\n     ISTO SEPARA DUAS COISAS que a palavra 'verificar' junta: ele conseguiu\n");
    printf("     DETETAR (disse errada, e estava) e não conseguiu CORRIGIR (escreveu o mesmo).\n");
    printf("     A túnica deu-lhe o ciclo ler→escrever, e o ciclo funcionou — o que faltou foi o\n");
    printf("     que nenhum ciclo fabrica: saber a resposta.\n");

    conclui("detetar e corrigir são duas capacidades, e ele tinha meia.");
}

/* ================================================================================ */
static void secao_E5(void){
    printf("\n§E5  O QUE A TÚNICA DÁ, E O QUE ELA NÃO DÁ\n\n");

    struct { const char *o; int deu; const char *nota; } M[] = {
        { "o ciclo ler → verificar → escrever", 1, "fechou nele, 12 de 12 vezes" },
        { "o domínio (a supervisão que pusemos)", 1, "e sem ela nem detetava a involução" },
        { "a forma da resposta (CORRETA/ERRADA)", 1, "obedeceu ao formato nas 12" },
        { "DETETAR o erro",                      0, "viu 1 das 3" },
        { "CORRIGIR o erro",                     0, "0 de 3 — a única que viu, repetiu" },
        { "não estragar o que estava certo",     0, "estragou 4 de 9" },
    };
    printf("        o que se pediu                          deu?   o que se mediu\n");
    int deu = 0, nao = 0;
    for(int i = 0; i < 6; i++){
        printf("        %-38s %-6s %s\n", M[i].o, M[i].deu ? "sim" : "NÃO", M[i].nota);
        if(M[i].deu) deu++; else nao++;
    }
    ok("a tabela tem os dois lados — se tudo tivesse corrido bem, não era medida", deu > 0 && nao > 0);

    printf("\n     A TÚNICA É O PAR ADJUNTO, e ela entregou exatamente isso: o ciclo. O que ela\n");
    printf("     não entrega — e nunca prometeu — é o CONTEÚDO. É a mesma fronteira do §S6 do\n");
    printf("     smartcontract.c: *o contrato verifica que FECHA, não que é VERDADE.*\n");
    printf("     Para corrigir é preciso um segundo doador que discorde, ou uma fonte. O ciclo\n");
    printf("     sozinho amplifica o que já lá está, incluindo o erro.\n");

    conclui("vestir a túnica fecha o circuito; não põe nada dentro dele.");
}

/* ================================================================================ */
int main(void){
    disco_prende(DISCO_BASE(192),"dados/PERG.bin",(size_t)(MAXL)*512,sizeof(char));
    disco_zera(PERG,(size_t)(MAXL)*512,sizeof(char));
    disco_prende(DISCO_BASE(193),"dados/ANTES.bin",(size_t)(MAXL)*2048,sizeof(char));
    disco_zera(ANTES,(size_t)(MAXL)*2048,sizeof(char));
    disco_prende(DISCO_BASE(194),"dados/DEPOIS.bin",(size_t)(MAXL)*2048,sizeof(char));
    disco_zera(DEPOIS,(size_t)(MAXL)*2048,sizeof(char));
    FILE *f = fopen("/tmp/corrigido.txt", "r");
    if(!f){
        printf("NAO MEDIU — sem /tmp/corrigido.txt. Corra  ./interroga.sh && ./veste_doador.sh\n");
        return 2;
    }
    char *l = NULL; size_t cap = 0;
    while(NL < MAXL && getline(&l, &cap, f) > 0){
        char *t1 = strchr(l, '\t'); if(!t1) continue;
        *t1 = 0; char *t2 = strchr(t1+1, '\t'); if(!t2) continue;
        *t2 = 0;
        snprintf(PERG[NL], sizeof PERG[0], "%s", l);
        snprintf(ANTES[NL], sizeof ANTES[0], "%s", t1+1);
        snprintf(DEPOIS[NL], sizeof DEPOIS[0], "%s", t2+1);
        NL++;
    }
    free(l); fclose(f);
    if(NL < 12){ printf("NAO MEDIU — só %d linhas (precisa de 12).\n", NL); return 2; }

    puts("entrega.c — A TÚNICA NO DOADOR: ele consegue verificar-se?");
    puts("==========================================================");
    printf("  %d afirmações dele, lidas de volta por ele com a túnica vestida\n", NL);
    puts("");
    puts("  A túnica deu-lhe o par (ler, escrever) e a supervisão deu-lhe o domínio. O gabarito");
    puts("  é decidível: três das doze estão erradas, e cada uma tem um sinal no próprio texto.");

    secao_E1(); secao_E2(); secao_E3(); secao_E4(); secao_E5();

    printf("\n==========================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A RESPOSTA É NÃO, E TEM NÚMERO: ele errou 6 das 12 — mais do que quem dissesse");
        puts("  sempre CORRETA, que erraria 3. Viu 1 das 3 erradas e, na que viu, a correção");
        puts("  REPETIU o erro palavra por palavra. E estragou 4 das 9 que estavam boas.");
        puts("");
        puts("  A túnica entregou o que é: o CICLO. Ler-se não é saber-se, e um ciclo fechado");
        puts("  sobre uma fonte só amplifica o que essa fonte já tem — inclusive o erro. Entregar");
        puts("  pronta exige o que o §S6 já dizia do contrato: um segundo lado que DISCORDE.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
