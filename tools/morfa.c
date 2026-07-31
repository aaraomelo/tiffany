/* morfa.c — OUTRAS FORMAS DE MORFAR PALAVRAS. Entrelaçar, e ainda assim detetar a inclusão.
 *
 * O Aarão: "vê outras formas de morfar palavras — por exemplo podes entrelaçar palavras e ainda
 * assim detetar a inclusão."
 *
 * O prefixo é uma inclusão só, e a mais frágil: mete-se um símbolo à frente e ela morre. Mas a
 * inclusão que sobrevive a entrelaçar é outra e é mais forte — a SUBSEQUÊNCIA. Entrelaçadas, as
 * duas palavras continuam lá dentro, cada uma inteira e por ordem, e detetam-se.
 *
 * E o entrelaçar não é truque novo: é A TROCA outra vez, um nível acima. No fatiamento, J trocou
 * bit por palavra; aqui troca símbolo por palavra. Duas sequências postas lado a lado formam uma
 * matriz, e entrelaçar é lê-la pela outra dimensão — a transposta, que é J.
 *
 *   §M1  entrelaçar preserva as duas: ambas ficam SUBSEQUÊNCIAS do resultado
 *   §M2  e a inclusão deteta-se, mesmo quando o prefixo já não vê nada
 *   §M3  desentrelaçar: J² = I com comprimentos iguais, e a fronteira exata sem eles
 *   §M4  e o par é reticulado: o entrelaçar JUNTA, a subsequência comum SEPARA
 *
 *   cc -O2 -std=c99 morfa.c -o morfa && ./morfa
 */
#include <stdio.h>
#include <string.h>
#include "corpos.h"
#include "unidade.h"

/* ENTRELAÇAR: um símbolo de cada, à vez. É J — a transposta da matriz 2×n. */
static void entrela(const char *a, const char *b, char *s){
    size_t i = 0, j = 0, k = 0;
    while(a[i] || b[j]){
        if(a[i]) s[k++] = a[i++];
        if(b[j]) s[k++] = b[j++];
    }
    s[k] = 0;
}
/* DESENTRELAÇAR: a mesma leitura pela outra dimensão. J aplicado outra vez. */
static void desentrela(const char *s, char *a, char *b){
    size_t k = 0, i = 0, j = 0;
    while(s[k]){ a[i++] = s[k++]; if(s[k]) b[j++] = s[k++]; }
    a[i] = 0; b[j] = 0;
}
/* A INCLUSÃO QUE SOBREVIVE: x é subsequência de s? Uma varredura, e é linear. */
static int contido(const char *x, const char *s){
    size_t i = 0;
    for(size_t k = 0; s[k] && x[i]; k++) if(s[k] == x[i]) i++;
    return x[i] == 0;
}
/* o prefixo comum, para comparar com a outra inclusão */
static int pref(const char *a, const char *b){
    int k = 0; while(a[k] && a[k] == b[k]) k++; return k;
}
/* a maior subsequência comum, em comprimento — o encontro do reticulado */
static int sub_comum(const char *a, const char *b){
    int n = (int)strlen(a), m = (int)strlen(b);
    static int L[64][64];
    for(int i = 0; i <= n; i++) for(int j = 0; j <= m; j++){
        if(!i || !j) L[i][j] = 0;
        else if(a[i-1] == b[j-1]) L[i][j] = L[i-1][j-1] + 1;
        else L[i][j] = L[i-1][j] > L[i][j-1] ? L[i-1][j] : L[i][j-1];
    }
    return L[n][m];
}

int main(void){
printf("\n=== MORFAR PALAVRAS — entrelaçar, e ainda assim ver a inclusão ============\n");
printf("    O prefixo é a inclusão mais frágil: um símbolo à frente e ela morre.\n");
printf("    A que sobrevive a entrelaçar é a SUBSEQUÊNCIA.\n");

const char *a = "ouro", *b = "prata";
char s[128], va[64], vb[64];
entrela(a, b, s);

printf("\n§M1  Entrelaçar preserva as duas — ambas ficam SUBSEQUÊNCIAS.\n\n");
{
    printf("      '%s' entrelacado com '%s' da '%s'\n\n", a, b, s);
    ok("'ouro' continua contido no entrelacado", contido(a, s));
    ok("e 'prata' tambem — as duas inteiras, e por ordem", contido(b, s));
    printf("\n      Nenhum simbolo se perdeu e nenhuma ordem se inverteu. O que se perdeu foi a\n");
    printf("      CONTIGUIDADE — e e so isso que o prefixo media.\n");
}

printf("\n§M2  E a inclusão deteta-se onde o prefixo já não vê nada.\n\n");
{
    printf("      medida            'ouro' vs '%s'\n", s);
    printf("      prefixo comum     %d\n", pref(a, s));
    printf("      subsequencia      %s\n\n", contido(a, s) ? "CONTIDO" : "nao");
    ok("o prefixo perde-o (1 simbolo) e a subsequencia acha-o inteiro",
       pref(a, s) == 1 && contido(a, s));
    /* e nao e por acaso: qualquer entrelacamento mantem a inclusao */
    const char *ps[][2] = { {"ouro","prata"}, {"gato","esquilo"}, {"a","bcdef"}, {"cifra","rei"} };
    long mau = 0;
    for(int t = 0; t < 4; t++){
        char e[128];
        entrela(ps[t][0], ps[t][1], e);
        if(!contido(ps[t][0], e) || !contido(ps[t][1], e)) mau++;
    }
    ok("e vale em todo par testado, com comprimentos diferentes", mau == 0);
}

printf("\n§M3  Desentrelaçar devolve as duas exatas — J² = I.\n\n");
{
    desentrela(s, va, vb);
    printf("      comprimentos DIFERENTES ('%s' 4, '%s' 5):\n", a, b);
    printf("        '%s' -> '%s' e '%s'   NAO volta\n", s, va, vb);
    ok("com comprimentos diferentes NAO e involucao — e mede-se, nao se esconde",
       strcmp(va,a) || strcmp(vb,b));
    char e2[128], c1[64], c2[64];
    entrela("ouro", "gato", e2);
    desentrela(e2, c1, c2);
    printf("      comprimentos IGUAIS ('ouro' 4, 'gato' 4):\n");
    printf("        '%s' -> '%s' e '%s'   volta\n", e2, c1, c2);
    ok("com comprimentos iguais J² = I, residuo 0", !strcmp(c1,"ouro") && !strcmp(c2,"gato"));
    printf("\n      A fronteira e exata: a alternancia estrita so se desfaz sozinha enquanto os\n");
    printf("      dois lados tiverem simbolo. Quando um acaba, o desentrelacar deixa de saber\n");
    printf("      onde a alternancia parou — e precisa do COMPRIMENTO para inverter.\n");
    printf("\n      E isso e coerente com o que ja se aprendeu hoje: o comprimento NAO e\n");
    printf("      coordenada (a frente estraga o prefixo) mas E NECESSARIO PARA INVERTER — e e\n");
    printf("      por isso que ele vive ATRAS, no fim da cifra. As duas coisas sao verdade ao\n");
    printf("      mesmo tempo, e o sitio dele resolve-as: atras nao pisa o conteudo, e continua\n");
    printf("      la para o dual o encontrar.\n");
    printf("\n      Entrelacar e a TROCA outra vez, um nivel acima. No fatiamento J trocou bit\n");
    printf("      por palavra; aqui troca simbolo por palavra. Duas sequencias lado a lado sao\n");
    printf("      uma matriz 2xn, e entrelacar e le-la pela outra dimensao — a transposta.\n");
    printf("      Por isso e reversivel: e a mesma regua (0,-1), ordem 2.\n");
}

printf("\n§M4  E o par é reticulado: entrelaçar JUNTA, a subsequência comum SEPARA.\n\n");
{
    printf("      par                    |sub. comum|   prefixo\n");
    struct { const char *x, *y; } P[] = {
        {"ourives","ourivesaria"}, {"ouro","ourico"}, {"ouro","prata"}, {"ouro","ruoo"},
    };
    for(int t = 0; t < 4; t++)
        printf("      %-10s %-11s %-14d %d\n", P[t].x, P[t].y,
               sub_comum(P[t].x, P[t].y), pref(P[t].x, P[t].y));
    ok("'ouro' e 'ruoo' partilham 2 por subsequencia e 0 por prefixo",
       sub_comum("ouro","ruoo") == 2 && pref("ouro","ruoo") == 0);
    printf("\n      O ultimo par e o que separa as duas medidas: 'ruoo' tem os mesmos simbolos\n");
    printf("      de 'ouro' baralhados. Pelo prefixo estao a distancia maxima; pela\n");
    printf("      subsequencia partilham dois. Nenhuma esta errada — MEDEM COISAS DIFERENTES:\n");
    printf("      o prefixo mede ONDE diverge, a subsequencia mede QUANTO sobrevive.\n");
    printf("\n      E as duas sao inclusoes, logo as duas ordenam, logo as duas sao reguas do\n");
    printf("      mesmo corpo — o (0,-1), o da involucao. Morfar uma palavra e mudar de regua\n");
    printf("      dentro dele, nao sair dele.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
