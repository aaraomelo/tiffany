/* espurio.c — O TERMO ESPÚRIO: mete-se, e testa-se se COLA. Não cola.
 *
 * O Aarão: "dá uma de Einstein e mete termos espúrios aí, vê se cola."
 *
 * O Λ de Einstein é o exemplo: acrescentou-se um termo para a teoria dizer o que se queria (um
 * universo estático), e depois não se sustentou. Aqui a tentação é a mesma e tem nome: a minha
 * conta de "27 ordenados" quebrou porque o parâmetro empata elementos distintos (quebra.c). O
 * termo espúrio que a salvaria é um DESEMPATE.
 *
 * Então mete-se — de verdade, implementado — e testa-se. Não se argumenta se cola: mede-se.
 *
 *     ordem espúria:   compara pelo parâmetro; se empatar, desempata pelo NUMERADOR
 *
 * Isso torna a relação TOTAL e ANTISSIMÉTRICA — o que eu queria. E depois testa-se contra as
 * cláusulas que uma ordem de corpo tem de cumprir. É aí que se vê.
 *
 *   §X1  o termo espúrio, implementado: e ele COMPRA a totalidade
 *   §X2  mas quebra a COMPATIBILIDADE com ⊗ — o par exibido
 *   §X3  e quebra com ⊕ também
 *   §X4  e a marca do espúrio: depende da REPRESENTAÇÃO, não do objeto
 *   §X5  o veredito: não cola, e vê-se onde
 *
 *   cc -O2 -std=c99 espurio.c -o espurio && ./espurio
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

/* o elemento do corpo: o PAR (a,b) — não a sua classe. Dois pares distintos podem ter o
 * mesmo parâmetro a/b, e é esse o empate que o termo espúrio vai "resolver". */
typedef struct { long a, b; } El;
static Par param(El x){ return ra_classe((Par){x.a, x.b}); }

/* A ORDEM ESPÚRIA: pelo parâmetro; se empatar, pelo numerador BRUTO. */
static int esp_cmp(El x, El y){
    int s = ra_cmp(param(x), param(y));
    if(s) return s;
    return (x.a > y.a) - (x.a < y.a);            /* ← O TERMO ESPÚRIO */
}
/* o produto no corpo: componente a componente (é o telescópico/óptico: razões compõem) */
static El mul(El x, El y){ El r = { x.a*y.a, x.b*y.b }; return r; }
static El add(El x, El y){ El r = { x.a*y.b + y.a*x.b, x.b*y.b }; return r; }

int main(void){
printf("\n=== O TERMO ESPÚRIO =======================================================\n");
printf("    Mete-se, e testa-se. Não se argumenta se cola — mede-se.\n");

printf("\n§X1  O termo espúrio COMPRA a totalidade. Funciona, à primeira vista.\n\n");
{
    int mau = 0; long empates = 0, casos = 0;
    for(long a=1;a<=14;a++) for(long b=1;b<=14;b++)
    for(long c=1;c<=14;c++) for(long d=1;d<=14;d++){
        El x={a,b}, y={c,d};
        int s = esp_cmp(x,y);
        if(s != -esp_cmp(y,x)) mau++;                    /* antissimétrica ✓ */
        if(s == 0 && !(a==c && b==d)) empates++;         /* empates que restam */
        casos++;
    }
    printf("      (2,1) vs (4,2)   mesmo parâmetro (2), numerador 2 < 4  →  (2,1) < (4,2)\n");
    printf("      empates restantes entre elementos distintos: %ld\n", empates);
    ok("com o desempate, a relação fica TOTAL e antissimétrica — comprou o que eu queria",
       mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      Está feito: o que era pré-ordem virou ordem total. Se eu parasse aqui, a minha\n");
    printf("      conta de 27 voltava a fechar. É exatamente o momento em que se para de medir.\n");
}

printf("\n§X2  Mas QUEBRA a compatibilidade com ⊗. O par exibido.\n\n");
{
    int mau = 0; long quebras = 0, casos = 0;
    El pior_x={0,0}, pior_y={0,0}, pior_c={0,0};
    for(long a=1;a<=10;a++) for(long b=1;b<=10;b++)
    for(long c=1;c<=10;c++) for(long d=1;d<=10;d++)
    for(long e=1;e<=6;e++) for(long f=1;f<=6;f++){
        El x={a,b}, y={c,d}, z={e,f};
        int s  = esp_cmp(x,y);
        int sz = esp_cmp(mul(x,z), mul(y,z));
        if(s && sz != s){
            if(!quebras){ pior_x=x; pior_y=y; pior_c=z; }
            quebras++;
        }
        casos++;
    }
    (void)mau; (void)pior_x; (void)pior_y; (void)pior_c;
    printf("      quebras encontradas: %ld em %ld triplos\n", quebras, casos);
    ok("EU PREVI QUE QUEBRAVA. NÃO QUEBRA — zero em toda a varredura positiva", quebras == 0);
    printf("\n      A minha previsão estava ERRADA, e o resultado é este: no cone positivo o\n");
    printf("      desempate pelo numerador É compatível com ⊗. E a razão é simples quando se vê:\n");
    printf("      se a/b = c/d e a < c, multiplicar por e/f com e > 0 dá ae < ce — o numerador\n");
    printf("      escala por positivo, e positivo preserva.\n");
    printf("\n      Eu escrevi o texto do \"não cola\" ANTES de correr. Ficou aqui, corrigido, porque\n");
    printf("      é o registo de eu ter previsto o que queria em vez do que sai.\n");
}

printf("\n§X3  E quebra com ⊕ também.\n\n");
{
    long quebras = 0, casos = 0;
    El px={0,0}, py={0,0}, pz={0,0};
    for(long a=1;a<=9;a++) for(long b=1;b<=9;b++)
    for(long c=1;c<=9;c++) for(long d=1;d<=9;d++)
    for(long e=1;e<=5;e++) for(long f=1;f<=5;f++){
        El x={a,b}, y={c,d}, z={e,f};
        int s  = esp_cmp(x,y);
        int sz = esp_cmp(add(x,z), add(y,z));
        if(s && sz != s){ if(!quebras){ px=x; py=y; pz=z; } quebras++; }
        casos++;
    }
    (void)px; (void)py; (void)pz;
    printf("      quebras: %ld em %ld triplos\n", quebras, casos);
    ok("nem com ⊕ quebra — o termo espúrio SOBREVIVE às duas operações", quebras == 0);
    printf("\n      Duas previsões minhas, duas erradas. O termo cola nas cláusulas que eu escolhi\n");
    printf("      para o atacar — e escolhi-as por serem as que eu achava que ele falharia.\n");
}

printf("\n§X4  A marca do espúrio: depende da REPRESENTAÇÃO, não do objeto.\n\n");
{
    int mau = 0;
    /* (2,1) e (4,2) são o MESMO elemento do corpo das razões — a mesma classe. O desempate
     * distingue-os pelo numerador, isto é, por COMO foram escritos. */
    El x = {2,1}, y = {4,2};
    Par px = param(x), py = param(y);
    if(ra_cmp(px,py) != 0) mau++;                        /* a mesma classe */
    if(esp_cmp(x,y) == 0) mau++;                         /* mas o espúrio separa */
    printf("      (2,1) e (4,2)   mesma classe (parâmetro 2)   mas o espúrio diz (2,1) < (4,2)\n");
    printf("      logo a \"ordem\" depende de COMO se escreveu o elemento, não do que ele é\n");
    ok("o termo espúrio ordena a ESCRITA, não o objeto — é essa a sua marca", mau == 0);
    printf("\n      É o teste que o rastro.c já usava para o ouro: a mesma classe tem de deixar o\n");
    printf("      MESMO rastro. Este não deixa — logo não é do corpo, é da folha em que se escreveu.\n");
}

printf("\n§X5  O veredito: COLA nas operações, e cai noutro sítio.\n\n");
{
    conclui("cola em ⊗ e ⊕ — e cai em BOA DEFINIÇÃO: não é função da classe");
    printf("      previ                  que quebrava em ⊗ e em ⊕\n");
    printf("      mediu-se               ZERO quebras nas duas — a previsão estava errada\n");
    printf("      onde cai de facto      §X4: separa (2,1) de (4,2), que são a MESMA classe\n");
    printf("      logo                   não é ordem no CORPO — é ordem nos representantes\n");
    printf("\n      É uma queda diferente da que eu anunciei, e mais subtil: o termo não parte a\n");
    printf("      álgebra — parte a IDENTIDADE. Ele ordena pares, e o corpo é feito de classes.\n");
    printf("      Perguntar-lhe \"quem é maior\" entre dois nomes do mesmo elemento é pergunta que\n");
    printf("      o corpo não tem.\n");
    printf("\n      E o Λ continua a ser a analogia certa, agora melhor: o termo de Einstein também\n");
    printf("      não quebrava as equações — era CONSISTENTE. Caiu por não corresponder ao mundo.\n");
    printf("      Este cai por não corresponder ao objeto. Nos dois casos, a álgebra aguentou.\n");
    printf("\n      E se eu tivesse parado no §X3 com o texto que já lá estava escrito, tinha\n");
    printf("      anunciado uma quebra que não existe — a favor da minha tese anterior.\n");
}

printf("\n=== NÃO COLA ==============================================================\n");
printf("  Meteu-se o termo espúrio — desempatar pelo numerador quando o parâmetro empata — e\n");
printf("  testou-se:\n\n");
printf("    compra    totalidade e antissimetria: a pré-ordem vira ordem\n");
printf("    e COLA    zero quebras em ⊗ e em ⊕ — a minha previsão de que partia estava ERRADA\n");
printf("    cai em    BOA DEFINIÇÃO: separa (2,1) de (4,2), que são a mesma classe\n\n");
printf("  Não é ordem no CORPO — é ordem nos representantes. O termo não parte a álgebra: parte\n");
printf("  a identidade. E o Λ fica melhor como analogia: o de Einstein também era consistente,\n");
printf("  e caiu por não corresponder ao mundo. Este cai por não corresponder ao objeto.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
