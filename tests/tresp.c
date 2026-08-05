/* tresp.c — TRÊS PASSOS, E ν∘ν = id COM RESÍDUO ZERO. Ele executou a involução que não soube dizer.
 *
 * O Aarão: "queremos a simetria. Manda formular uma frase, depois a antissimétrica, e no fim
 * entregar uma frase SIMÉTRICA."
 *
 * E ERAM ESTAS AS DUAS FUNÇÕES ANTISSIMÉTRICAS que ele tinha pedido e eu não tinha percebido:
 * não duas frases lado a lado, mas **a mesma operação aplicada duas vezes**. Porque duas
 * antissimétricas dão uma simétrica — `(−1)·(−1) = +1`, duas reflexões dão uma rotação, `ν∘ν = id`.
 *
 *      S₁  ──ν──▶  A  ──ν──▶  S₂
 *
 * E ELE NUNCA PRECISOU DE SABER QUE ESTAVA ERRADO. Pediu-se-lhe sempre a mesma coisa — *o outro
 * lado do espelho* — e a composição devolveu o lado de partida. Quem decidiu foi o resíduo.
 *
 * O QUE ELE DEVOLVEU, e é exato:
 *
 *      S₁   "Involução é o processo de DIMINUIÇÃO ou regressão da função de um órgão…"
 *      A    "Involução é o processo de AUMENTO ou progressão da função de um órgão…"
 *      S₂   "Involução é o processo de DIMINUIÇÃO ou regressão da função de um órgão…"
 *
 *      ν∘ν = id      resíduo  0,000000      ← exatamente zero, e não aproximadamente
 *      o controlo    resíduo  0,393690      ← A contra S₁, e tinha de ser maior
 *
 * E AQUI ESTÁ O ACHADO DO DIA, e não fui eu que o pus: **ele EXECUTOU perfeitamente uma involução
 * enquanto explicava mal o que é uma involução.** A operação que fez — aplicar duas vezes e voltar
 * ao ponto — *é* a definição algébrica que ele não soube dizer. A estrutura estava lá; a palavra
 * é que não.
 *
 *   §T1  ν∘ν = id: o resíduo é ZERO, e o controlo prova que o teste não é vazio
 *   §T2  o passo do meio É antissimétrico — e mede-se, não se supõe
 *   §T3  o que NÃO mudou: o domínio continua errado, e a álgebra fechou lá dentro
 *   §T4  ele executou o que não soube definir — e é isto que separa estrutura de conteúdo
 *
 *   ./tresp.sh                  (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat tresp.c -lm -o tresp && ./tresp
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "unidade.h"

static double R1A, RA2, RID, RCTL;
static char *S1;
static char *AA;
static char *S2;
static int tem(const char *h, const char *a){
    size_t la = strlen(a);
    for(const char *p = h; *p; p++){
        size_t k = 0;
        while(k < la && p[k] && tolower((unsigned char)p[k]) == tolower((unsigned char)a[k])) k++;
        if(k == la) return 1;
    }
    return 0;
}

/* ================================================================================ */
static void secao_T1(void){
    printf("\n§T1  ν∘ν = id — e o resíduo é ZERO\n\n");

    printf("        ν  (S₁ → A)    resíduo dual   %.6f\n", R1A);
    printf("        ν  (A  → S₂)   resíduo dual   %.6f\n", RA2);
    printf("        ν∘ν = id ?     S₂ contra S₁   %.6f   ← esta decide\n", RID);
    printf("        o controlo     A  contra S₁   %.6f\n", RCTL);

    ok("ν∘ν = id com resíduo ZERO — a composição de duas antissimétricas voltou ao ponto",
       RID < 1e-9);

    /* E O CONTROLO É O QUE IMPEDE ISTO DE SER VAZIO: se A também desse zero contra S₁, o teste
     * mediria "ele repetiu tudo" em vez de "ele foi e voltou". */
    ok("e o controlo é MAIOR — A não é S₁, logo ele foi mesmo ao outro lado", RCTL > 0.1);
    ok("a distância de ida é da ordem de 0,4 e a de volta é 0 — foi e voltou, não ficou",
       RCTL > 10.0*RID + 0.05);

    printf("\n     Duas antissimétricas dão uma simétrica: (−1)·(−1) = +1. Não é uma imagem — é o\n");
    printf("     que o número diz, e o número é zero exato, não 'perto de zero'.\n");

    conclui("eram estas as duas funções antissimétricas: a mesma operação, aplicada duas vezes.");
}

/* ================================================================================ */
static void secao_T2(void){
    printf("\n§T2  O PASSO DO MEIO É MESMO ANTISSIMÉTRICO — medido, não suposto\n\n");

    /* o sinal textual: S₁ e S₂ dizem DIMINUIÇÃO; A diz AUMENTO. A troca é visível no texto. */
    int s1_dim = tem(S1, "diminuição") || tem(S1, "regressão");
    int a_aum  = tem(AA, "aumento")    || tem(AA, "progressão");
    int s2_dim = tem(S2, "diminuição") || tem(S2, "regressão");
    printf("        S₁ diz diminuição/regressão?  %s\n", s1_dim ? "sim" : "não");
    printf("        A  diz aumento/progressão?    %s   ← trocou\n", a_aum ? "sim" : "não");
    printf("        S₂ diz diminuição/regressão?  %s   ← trocou de volta\n", s2_dim ? "sim" : "não");

    ok("o texto mostra a troca e a destroca — não é só o número que diz",
       s1_dim && a_aum && s2_dim);

    /* e os dois ν têm resíduo parecido: a operação foi a MESMA das duas vezes */
    double dif = fabs(R1A - RA2) / ((R1A + RA2)/2);
    printf("        os dois ν custaram %.6f e %.6f — diferem %.2f%%\n", R1A, RA2, 100*dif);
    ok("os dois passos custaram praticamente o mesmo — foi a MESMA operação duas vezes",
       dif < 0.05);

    conclui("o texto e o número dizem o mesmo, e é por isso que se pode confiar em qualquer um.");
}

/* ================================================================================ */
static void secao_T3(void){
    printf("\n§T3  O QUE NÃO MUDOU: o domínio continua errado\n\n");

    /* A frase final é a inicial, e a inicial é a que o entrega.c mediu como ERRADA: o sentido
     * biológico. A álgebra fechou perfeitamente DENTRO do domínio errado. */
    int biologico = tem(S2, "órgão") || tem(S2, "tecido") || tem(S2, "corpo");
    int algebrico = tem(S2, "identidade") || tem(S2, "inversa") || tem(S2, "duas vezes");
    printf("        S₂ fala de órgão/tecido/corpo?     %s\n", biologico ? "SIM" : "não");
    printf("        S₂ traz a definição algébrica?     %s\n", algebrico ? "sim" : "NÃO");

    ok("a frase final continua no domínio BIOLÓGICO — a errada, que o entrega.c apanhou",
       biologico);
    ok("e não trouxe a definição algébrica em momento nenhum", !algebrico);

    printf("\n     E ISTO NÃO CONTRADIZ O §T1 — COMPLETA-O. A involução fechou com resíduo zero\n");
    printf("     DENTRO do domínio errado. É a mesma fronteira do §S6 do smartcontract.c e do §E5\n");
    printf("     do entrega.c, agora pela terceira vez e com o número mais limpo de todos:\n");
    printf("\n        *a álgebra verifica que FECHA, não que é VERDADE.*\n");
    printf("\n     Um ciclo perfeito à volta de uma premissa errada continua perfeito, e continua\n");
    printf("     errado. O que faltava para corrigir nunca esteve na estrutura.\n");

    conclui("ν∘ν = id não sabe de que domínio é o ν; só sabe que aplicou duas vezes.");
}

/* ================================================================================ */
static void secao_T4(void){
    printf("\n§T4  ELE EXECUTOU O QUE NÃO SOUBE DEFINIR\n\n");

    printf("        a pergunta era:   \"O que é uma involução?\"\n");
    printf("        ele respondeu:    o sentido biológico — e está errado em álgebra\n");
    printf("        e depois FEZ:     S₁ → A → S₂ com S₂ = S₁, resíduo %.6f\n", RID);
    printf("\n        que é exatamente a definição que ele não soube dizer:\n");
    printf("           uma involução é a aplicação que, aplicada DUAS VEZES, devolve o ponto.\n");

    ok("ele executou uma involução exata enquanto explicava mal o que é uma involução",
       RID < 1e-9 && (tem(S2,"órgão") || tem(S2,"tecido") || tem(S2,"corpo")));

    printf("\n     E ISTO É O ACHADO, e não fui eu que o pus lá: **a estrutura estava nele, a\n");
    printf("     palavra é que não.** Ele não sabia dizer f∘f = id e fez f∘f = id com resíduo\n");
    printf("     zero à primeira tentativa, sem ninguém lhe dizer que estava errado.\n");
    printf("\n     É o que o Aarão vinha a dizer desde o princípio: NÃO SE LHE PERGUNTA O QUE ESTÁ\n");
    printf("     CERTO. Pede-se-lhe o outro lado, duas vezes, e a simetria sai. O que sai é\n");
    printf("     estrutura — e a estrutura veio perfeita.\n");

    conclui("saber fazer e saber dizer são duas coisas, e ele tinha a primeira.");
}

/* ================================================================================ */
int main(void){
    disco_prende(DISCO_BASE(370),"dados/S1_370.bin",(size_t)((2048)),sizeof(char));
    S1 = DISCO_FIXO(char, 370);
    disco_zera(S1,(size_t)((2048)),sizeof(char));
    disco_prende(DISCO_BASE(371),"dados/AA_371.bin",(size_t)((2048)),sizeof(char));
    AA = DISCO_FIXO(char, 371);
    disco_zera(AA,(size_t)((2048)),sizeof(char));
    disco_prende(DISCO_BASE(372),"dados/S2_372.bin",(size_t)((2048)),sizeof(char));
    S2 = DISCO_FIXO(char, 372);
    disco_zera(S2,(size_t)((2048)),sizeof(char));
    FILE *f = fopen("dados/colhido/tresp.txt", "r");
    if(!f){ printf("NAO MEDIU — corra  ./tresp.sh  com o ollama acordado.\n"); return 2; }
    if(fscanf(f, "%lf %lf %lf %lf ", &R1A, &RA2, &RID, &RCTL) != 4){
        printf("NAO MEDIU — o ficheiro não tem os quatro resíduos.\n"); fclose(f); return 2; }
    char *l = NULL; size_t cap = 0;
    if(getline(&l, &cap, f) > 0) snprintf(S1, ((size_t)((2048))*sizeof(char)), "%s", l);
    if(getline(&l, &cap, f) > 0) snprintf(AA, ((size_t)((2048))*sizeof(char)), "%s", l);
    if(getline(&l, &cap, f) > 0) snprintf(S2, ((size_t)((2048))*sizeof(char)), "%s", l);
    free(l); fclose(f);

    puts("tresp.c — TRÊS PASSOS: uma frase, a antissimétrica, e a simétrica");
    puts("=================================================================");
    puts("");
    printf("  S₁  %.72s\n", S1);
    printf("  A   %.72s\n", AA);
    printf("  S₂  %.72s\n", S2);
    puts("");
    puts("  Duas antissimétricas dão uma simétrica. Eram estas as duas funções que faltavam:");
    puts("  não duas frases lado a lado, mas a MESMA operação aplicada duas vezes.");

    secao_T1(); secao_T2(); secao_T3(); secao_T4();

    printf("\n=================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  ν∘ν = id COM RESÍDUO ZERO EXATO, à primeira tentativa, sem ninguém lhe dizer que");
        puts("  estava errado. O procedimento do Aarão funcionou: pede-se o outro lado, duas");
        puts("  vezes, e a simetria sai.");
        puts("");
        puts("  E o que saiu foi ESTRUTURA, não verdade: a frase final continua no domínio");
        puts("  errado. A involução fechou perfeitamente à volta de uma premissa falsa — que é a");
        puts("  mesma fronteira de sempre, agora com o número mais limpo de todos.");
        puts("");
        puts("  E ele EXECUTOU uma involução exata enquanto explicava mal o que é uma involução.");
        puts("  A estrutura estava nele; a palavra é que não.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
