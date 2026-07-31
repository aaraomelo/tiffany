#include <stdio.h>
#include "caminho.h"
#include "unidade.h"
int main(void){
printf("\n=== A DESCIDA — o endereço é o caminho, o formato é a roupa ================\n");
printf("\n§P1  JSON: o nível é o parêntese.\n\n");
{
    const char *j = "{\"a\":[1,2,{\"b\":[\"x\",\"y\\\"z\",\"w\"]}]}";
    const char *raiz = strchr(j, '[');
    int cam[] = { 2 };
    size_t n; const char *v = js_caminho(raiz, cam, 1, &n);
    /* o 2.o elemento e um objeto, nao string: a descida diz que nao ha string ali */
    printf("      caminho [2] num objeto -> %s (e o certo: nao e string)\n", v ? "string" : "nada");
    ok("a descida nao inventa: onde nao ha string, devolve nada", v == NULL);
    const char *dentro = js_desce(raiz, 2);
    const char *arr = strchr(dentro, '[');
    int c2[] = { 1 };
    v = js_caminho(arr, c2, 1, &n);
    printf("      caminho [2]->[1] = \"%.*s\"  (com aspa escapada dentro)\n", (int)n, v);
    ok("e desce por dentro do aninhamento, com escape e tudo", v && n == 4);
}
printf("\n§P2  YAML: o nível é a indentação.\n\n");
{
    const char *y = "banco:\n  faixa: 4194304\n  banda: ouro\noutro: 1\n";
    size_t n; const char *v = desce(&FORMATOS[1], y + 7, 1, 1, &n);
    printf("      nivel 2, item 1 = \"%.*s\"\n", (int)n, v ? v : "");
    ok("desce pela indentacao, e para quando o nivel sobe", v && !memcmp(v, "  banda", 7));
}
printf("\n§P3  Markdown: o nível é a contagem de #.\n\n");
{
    const char *m = "# um\n## a\ntexto\n## b\n## c\n# dois\n";
    size_t n; const char *v = desce(&FORMATOS[2], m, 2, 2, &n);
    printf("      nivel ##, item 2 = \"%.*s\"\n", (int)n, v ? v : "");
    ok("desce pela contagem de #, e a lei e a mesma", v && !memcmp(v, "## c", 4));
}
printf("\n§P4  E a assinatura e sempre so UMA: a cifra, pelo mesmo codificador.\n\n");
{
    printf("      formato   razao  sinal   cifra (pelo cifra_geral, o dos 31 corpos)\n");
    static long C[3][64]; size_t NC[3];
    for(int i = 0; i < 3; i++){
        NC[i] = cifra_geral(0, 0, FORMATOS[i].razao, FORMATOS[i].sinal,
                            FORMATOS[i].razao, C[i], 64);
        printf("      %-9s %-6ld %-7ld [", FORMATOS[i].nome, FORMATOS[i].razao, FORMATOS[i].sinal);
        for(size_t k = 0; k < NC[i]; k++) printf("%s%ld", k?";":"", C[i][k]);
        printf("]\n");
    }
    size_t pre = 0;
    while(pre < NC[1] && pre < NC[2] && C[1][pre] == C[2][pre]) pre++;
    printf("\n      yaml e md partilham %zu termo(s): distancia 1/%ld na regua de sempre\n",
           pre, 1L << pre);
    int dif = (NC[0] != NC[1]) || memcmp(C[0], C[1], NC[0]*sizeof(long)) != 0;
    ok("cada formato tem cifra pelo MESMO codificador dos corpos, e sao distintas", dif);
    printf("\n      Nao ha codificador para formatos: e o cifra_geral, que ja encodava qualquer\n");
    printf("      corpo. Um formato e (razao, sinal) como todos — a razao e quantos simbolos\n");
    printf("      por nivel, o sinal e se a marca FECHA. O parentese fecha (-1, as duas\n");
    printf("      direcoes cancelam-se); a indentacao e o cardinal so se acumulam (+1).\n");
}

printf("\n      Os tres sao a MESMA descida: o que muda e quem le a marca do nivel — o\n");
printf("      parentese, os espacos, os cardinais. A lei e a do tesseracto, M_k =\n");
printf("      M_{k-1}·A_1: o nivel k carrega o k-1. Trocar de formato nao troca a descida.\n\n");
return falhas ? 1 : 0;
}
