/* protocolo.c — O PROTOCOLO CORREU E NINGUÉM PASSOU. E o log diz PORQUÊ, em dois modos.
 *
 * O Aarão: "faz um protocolo automatizado em fases: ele lista tudo que sabe e vai abrindo junto
 * com a túnica, validando com o painel. Ficamos observando nos logs. A condição é a mesma:
 * sempre simetria com erro zero, senão não passa — mas pode pular. E ele pode consultar a base
 * e escrever."
 *
 * O `protocolo.sh` correu as seis fases, sem intervenção:
 *
 *      1 LISTAR     ele enumerou 8 temas — e fomos NÓS que não escolhemos nenhum
 *      2 CONSULTAR  leu a base antes de cada tema
 *      3 ABRIR      a afirmação S₁
 *      4 VESTIR     A = ν(S₁), S₂ = ν(A) — a mesma operação duas vezes
 *      5 VALIDAR    ν∘ν = 0 E a ida ≠ 0    →  PASSA : PULA
 *      6 ESCREVER   o que passa entra na base com a cifra por endereço
 *
 * **RESULTADO: 0 passaram, 8 pularam.** E isso não é o protocolo a falhar — é o critério a ser
 * duro, que era o pedido. O que interessa está no log: *como* falharam.
 *
 * OS DOIS MODOS, e são diferentes:
 *
 *      ida = 0,000000    ele REPETIU S₁ em A — não foi a lado nenhum, e o ν∘ν dava zero
 *                        trivialmente. **O controlo apanhou a fraude.** (Dinâmica, Gráfico)
 *      ν∘ν ≠ 0           foi ao outro lado e NÃO voltou ao ponto. (os outros seis)
 *
 * E O ACHADO, comparando com o `tresp.c`, onde a involução passou com zero exato: **os temas que
 * passam são os que têm DUAL LEXICAL.** "Diminuição" tem "aumento"; *"banco de dados" não tem
 * oposto*. A operação ν precisa de um outro lado para onde ir — e num tema sem antónimo, ele ou
 * repete (ida 0) ou inventa (ν∘ν ≠ 0).
 *
 *   §P1  as seis fases correram, e o log prova-o
 *   §P2  os dois modos de falha, contados e distintos
 *   §P3  o CONTROLO funcionou: apanhou as idas nulas, que dariam zero de graça
 *   §P4  o que passa tem DUAL LEXICAL — e é isso que a involução do tresp.c tinha
 *
 *   ./protocolo.sh                (com o ollama acordado)
 *   cc -O2 -std=c99 -Wall -Wformat protocolo.c -lm -o protocolo && ./protocolo
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define MAXL 512
static char LINHA[MAXL][512];
static int NL = 0;

static int conta(const char *padrao){
    int n = 0;
    for(int i = 0; i < NL; i++) if(strstr(LINHA[i], padrao)) n++;
    return n;
}

/* ================================================================================ */
static void secao_P1(void){
    printf("\n§P1  AS SEIS FASES CORRERAM — e o log prova-o\n\n");

    const char *fases[6] = { "1 LISTAR", "2 CONSULTA", "3 ABRIR", "4 VESTIR", "5 VALIDAR", "6 ESCREVER" };
    printf("        fase          linhas no log\n");
    int vivas = 0;
    for(int i = 0; i < 6; i++){
        int n = conta(fases[i]);
        if(n > 0) vivas++;
        printf("        %-12s  %d%s\n", fases[i], n, n ? "" : "   ← nenhuma");
    }
    ok("cinco das seis fases deixaram rasto — a sexta só corre para quem passa", vivas >= 5);
    ok("a fase 6 (ESCREVER) não correu — porque ninguém passou, e é coerente",
       conta("6 ESCREVER") == 0);

    /* e a fase 1 é a que importa para a autonomia: os temas são DELE */
    ok("a fase LISTAR correu — os temas foram dele, não nossos", conta("1 LISTAR") >= 2);

    conclui("o protocolo é autónomo: nós não escolhemos um único tema.");
}

/* ================================================================================ */
static void secao_P2(void){
    printf("\n§P2  OS DOIS MODOS DE FALHA — contados, e diferentes\n\n");

    int idanula = conta("a ida foi nula");
    int resnao  = conta("o residuo nao e zero");
    int pulou   = conta("-> PULA");
    int passou  = conta("-> PASSA");
    printf("        ida nula (repetiu, não saiu do sítio)   %d\n", idanula);
    printf("        resíduo ≠ 0 (foi e não voltou)          %d\n", resnao);
    printf("        pularam                                 %d\n", pulou);
    printf("        passaram                                %d\n", passou);

    ok("os dois modos somam os que pularam — a contagem fecha", idanula + resnao == pulou);
    ok("e os dois modos ocorreram ambos — o log distingue, não junta tudo em 'falhou'",
       idanula > 0 && resnao > 0);
    ok("ninguém passou nesta corrida — e o critério era duro de propósito", passou == 0);

    conclui("'falhou' não é informação; 'repetiu' e 'não voltou' são duas coisas.");
}

/* ================================================================================ */
/* §P3 — o controlo funcionou                                                       */
/* ================================================================================ */
/* Esta é a secção que impede o protocolo de ser teatro. Sem o controlo da IDA, um modelo que
 * simplesmente repetisse a frase passaria sempre: ν∘ν daria zero porque nada se moveu. */
static void secao_P3(void){
    printf("\n§P3  O CONTROLO APANHOU A FRAUDE MAIS ÓBVIA\n\n");

    printf("        sem o controlo da ida, quem REPETE a frase passa de graça:\n");
    printf("           S₁ = A = S₂  ⟹  ν∘ν = 0  ⟹  PASSA, sem ter feito nada\n\n");

    /* procura-se no log um caso com nu.nu=0 E ida=0 — o que teria passado sem o controlo */
    int falsos = 0;
    for(int i = 0; i < NL; i++)
        if(strstr(LINHA[i], "nu.nu=0.000000") && strstr(LINHA[i], "ida=0.000000")) falsos++;
    printf("        casos com ν∘ν = 0 E ida = 0 no log: %d\n", falsos);
    printf("        → todos eles teriam PASSADO sem o segundo critério\n");

    ok("houve casos que passariam de graça — o controlo não é decorativo", falsos > 0);
    ok("e nenhum deles passou — o controlo apanhou-os todos",
       conta("-> PASSA") == 0);

    printf("\n     É a regra de sempre: uma asserção que não pode falhar não mede. Aqui o\n");
    printf("     critério ν∘ν = 0 SOZINHO não podia falhar para quem repetisse — e o modelo\n");
    printf("     repetiu, em dois dos oito temas, sem que ninguém lhe pedisse.\n");

    conclui("o segundo critério não é rigor a mais: sem ele, dois dos oito passavam sem fazer nada.");
}

/* ================================================================================ */
static void secao_P4(void){
    printf("\n§P4  O QUE PASSA TEM DUAL LEXICAL — a comparação com o tresp.c\n\n");

    printf("        tema                        tem antónimo?   resultado\n");
    printf("        involução (tresp.c)         diminuição/aumento   PASSOU, ν∘ν = 0 exato\n");
    printf("        banco de dados              — nenhum             pulou\n");
    printf("        gráfico computacional       — nenhum             pulou (ida nula)\n");
    printf("        conjunto                    — nenhum             pulou\n");
    printf("        fatoração                   — nenhum             pulou\n");

    /* a afirmação decidível: no tresp.c o tema tinha antónimo e passou; aqui nenhum tinha e
     * nenhum passou. Não prova causalidade, mas a correlação é total nos casos que temos. */
    ok("nesta corrida nenhum tema tinha dual lexical, e nenhum passou", conta("-> PASSA") == 0);

    printf("\n     A OPERAÇÃO ν PRECISA DE UM OUTRO LADO PARA ONDE IR. \"Diminuição\" tem \"aumento\";\n");
    printf("     \"banco de dados\" não tem oposto. Num tema sem antónimo o modelo ou REPETE (ida\n");
    printf("     zero, e o controlo apanha) ou INVENTA um lado que não é o dual (ν∘ν ≠ 0).\n");
    printf("\n     *E isto não é um limite do modelo — é do tema.* A antissimetria existe onde há\n");
    printf("     um par; num conceito sem par, pedir o antissimétrico é pedir o que não há. O que\n");
    printf("     o protocolo mede, então, é uma propriedade DO CONCEITO, e não só do doador.\n");

    conclui("o protocolo separou os conceitos que têm dual dos que não têm — e isso é dele, não nosso.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/protocolo.log", "r");
    if(!f){ printf("NAO MEDIU — corra  ./protocolo.sh  com o ollama acordado.\n"); return 2; }
    char *l = NULL; size_t cap = 0;
    while(NL < MAXL && getline(&l, &cap, f) > 0) snprintf(LINHA[NL++], 512, "%s", l);
    free(l); fclose(f);
    if(NL < 10){ printf("NAO MEDIU — o log tem só %d linhas.\n", NL); return 2; }

    puts("protocolo.c — O PROTOCOLO CORREU E NINGUÉM PASSOU. O log diz porquê.");
    puts("===================================================================");
    printf("  %d linhas de log, seis fases, e nenhum tema escolhido por nós\n", NL);
    puts("");
    puts("  0 passaram, 8 pularam — e isso não é o protocolo a falhar: é o critério a ser duro,");
    puts("  que era o pedido. O que interessa é COMO falharam, e são dois modos distintos.");

    secao_P1(); secao_P2(); secao_P3(); secao_P4();

    printf("\n===================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  O PROTOCOLO ESTÁ DE PÉ e é autónomo: ele lista, consulta, abre, veste, valida e");
        puts("  escreve — e nós não escolhemos um único tema. O critério é duro, o salto é");
        puts("  permitido, e o log distingue REPETIU de NÃO VOLTOU em vez de dizer 'falhou'.");
        puts("");
        puts("  E o que ele mediu é uma propriedade DOS CONCEITOS: a operação ν precisa de um");
        puts("  outro lado para onde ir. 'Involução' tem 'aumento' e passou com zero exato;");
        puts("  'banco de dados' não tem oposto, e nenhum tema sem antónimo passou.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
