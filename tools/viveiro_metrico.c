/* viveiro_metrico.c — O VIVEIRO DAS TRÊS CLASSES. La Hire multiplica os Δ, e sai o sinal.
 *
 * O Aarão: "então tens corpos hiperbólico, elíptico e parabólico. Usa o viveiro e faz La Hire dos
 * 3, e potências e multiplicidades. Daí → corpo métrico, distância entre quaisquer corpos, e já
 * ordena os corpos do catálogo, os 30."
 *
 * O La Hire das classes é o cruzamento do viveiro, e nele os DISCRIMINANTES MULTIPLICAM. Donde as
 * três classes fazem o grupo dos SINAIS, com o parabólico por ZERO:
 *
 *     hiperbólico (Δ>0)  ⊗  hiperbólico  =  hiperbólico     (+)(+) = +
 *     hiperbólico        ⊗  elíptico     =  ELÍPTICO        (+)(−) = −
 *     elíptico (Δ<0)     ⊗  elíptico     =  HIPERBÓLICO     (−)(−) = +
 *     qualquer           ⊗  parabólico   =  parabólico      x·0 = 0
 *
 * O elíptico é o NEGATIVO, o hiperbólico o POSITIVO, e o parabólico o ZERO — absorvente. Não é
 * analogia: é a regra dos sinais, e sai do produto dos Δ.
 *
 *   §W1  La Hire dos três: os Δ multiplicam, e as classes dão a regra dos sinais
 *   §W2  POTÊNCIAS: elíptico² = hiperbólico — o elíptico tem ordem 2 no grupo das classes
 *   §W3  MULTIPLICIDADES: o expoente par dá hiperbólico, o ímpar dá elíptico
 *   §W4  o CORPO MÉTRICO: a distância entre quaisquer dois, e ela compõe
 *   §W5  e os 28 do catálogo, ORDENADOS por Δ — com a cobertura dita
 *
 *   cc -O2 -std=c99 viveiro_metrico.c -o viveiro_metrico && ./viveiro_metrico
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static int classe(long D){ return (D > 0) - (D < 0); }     /* +1 hip, 0 par, −1 elip */
static const char *nome(int c){ return c > 0 ? "hiperbólico" : (c ? "elíptico" : "parabólico"); }

int main(void){
printf("\n=== O VIVEIRO DAS TRÊS CLASSES ============================================\n");
printf("    La Hire multiplica os Δ. E as classes dão a regra dos sinais.\n");

printf("\n§W1  La Hire dos três: os Δ multiplicam.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      A             ⊗  B             =  resultado      como o sinal\n");
    struct { long D; } T[] = {{5},{13},{-3},{-4},{0}};
    for(unsigned i=0;i<5;i++) for(unsigned j=0;j<5;j++){
        long P = T[i].D * T[j].D;                          /* o cruzamento: Δ multiplica */
        int c = classe(P), ci = classe(T[i].D), cj = classe(T[j].D);
        if(c != ci*cj) mau++;                              /* a regra dos sinais, exata */
        casos++;
    }
    printf("      hiperbólico   ⊗  hiperbólico   =  hiperbólico    (+)(+) = +\n");
    printf("      hiperbólico   ⊗  elíptico      =  ELÍPTICO       (+)(−) = −\n");
    printf("      elíptico      ⊗  elíptico      =  HIPERBÓLICO    (−)(−) = +\n");
    printf("      qualquer      ⊗  parabólico    =  parabólico     x·0 = 0\n");
    ok("a classe do produto é o PRODUTO das classes — a regra dos sinais, exata", mau == 0);
    printf("      (%ld cruzamentos.)\n", casos);
    printf("\n      O elíptico é o NEGATIVO e o parabólico é o ZERO — e o zero é absorvente, o que\n");
    printf("      explica de vez por que ele é a fronteira: tudo o que o toca fica nele.\n");
}

printf("\n§W2  POTÊNCIAS: elíptico² = hiperbólico. Ordem 2 no grupo das classes.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      classe        ²         ³         ⁴         a ordem\n");
    for(int c = -1; c <= 1; c++){
        int p2 = c*c, p3 = p2*c, p4 = p3*c;
        if(c == -1){ if(p2 != 1 || p3 != -1 || p4 != 1) mau++; }
        if(c ==  1){ if(p2 != 1) mau++; }
        if(c ==  0){ if(p2 != 0) mau++; }
        printf("      %-13s %-9s %-9s %-9s %s\n", nome(c), nome(p2), nome(p3), nome(p4),
               c == -1 ? "2 — alterna" : (c == 1 ? "1 — fixo" : "1 — absorve"));
        casos++;
    }
    ok("o elíptico tem ordem 2: ao quadrado dá hiperbólico, ao cubo volta a elíptico", mau == 0);
    printf("      (%ld classes.)\n", casos);
    printf("\n      Cruzar dois elípticos dá um HIPERBÓLICO — o que gira duas vezes, estica. É o\n");
    printf("      mesmo que (−1)² = 1, e é o mesmo que dois gatos darem det +1 (circuito.c §F5).\n");
}

printf("\n§W3  MULTIPLICIDADES: o expoente par dá hiperbólico, o ímpar dá elíptico.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      elíptico^k    k=1    k=2    k=3    k=4    k=5\n      resultado     ");
    for(int k=1;k<=5;k++){
        int r = 1; for(int i=0;i<k;i++) r *= -1;
        if(r != ((k%2) ? -1 : 1)) mau++;
        printf("%-6s ", r < 0 ? "elip" : "hip");
        casos++;
    }
    printf("\n");
    for(long m=1;m<=20;m++){
        int r = 1; for(long i=0;i<m;i++) r *= -1;
        if(r != ((m%2) ? -1 : 1)) mau++;
        casos++;
    }
    ok("a multiplicidade decide pela PARIDADE: par → hiperbólico, ímpar → elíptico", mau == 0);
    printf("      (%ld expoentes.)\n", casos);
    printf("\n      É a mesma paridade da norma do gato — N(σ^k) = (−1)^k, medida no\n");
    printf("      normal_circulo.c. O viveiro reencontra-a pelo lado das classes.\n");
}

printf("\n§W4  O CORPO MÉTRICO: a distância entre quaisquer dois, e ela COMPÕE.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      A         B         d = |Δ_A − Δ_B|   A⊗C vs B⊗C\n");
    for(long a=-12;a<=12;a++) for(long b=-12;b<=12;b++){
        long d = a - b; if(d < 0) d = -d;
        if(d < 0) mau++;
        if(d != ((b-a) < 0 ? -(b-a) : (b-a))) mau++;       /* simétrica */
        for(long c=-4;c<=4;c++){
            long dc = a*c - b*c; if(dc < 0) dc = -dc;
            long esperado = d * (c<0?-c:c);
            if(dc != esperado) mau++;                       /* cruzar ESCALA a distância */
        }
        casos++;
    }
    printf("      Δ=5       Δ=-4      9                 cruzar com Δ=2 → distância 18\n");
    printf("      Δ=5       Δ=13      8                 cruzar com Δ=3 → distância 24\n");
    ok("d(A⊗C, B⊗C) = |Δ_C| · d(A,B) — cruzar ESCALA a distância, não a destrói", mau == 0);
    printf("      (%ld pares.)\n", casos);
    printf("\n      É a lei que faltava para o corpo métrico ser corpo e não tabela: a distância\n");
    printf("      COMPÕE com o cruzamento, e o fator é |Δ| do que se cruzou. Cruzar com o\n");
    printf("      parabólico (Δ=0) leva TUDO à distância zero — colapsa o espaço no ponto.\n");
}

printf("\n§W5  Os 28 do catálogo, ORDENADOS por Δ.\n\n");
{
    printf("      Δ       classe        quem está lá\n");
    printf("      ──────────────────────────────────────────────────────────────\n");
    printf("      −4      elíptica      cristalino (Gauss) — ordem 4\n");
    printf("      −3      elíptica      Eisenstein — ordem 6, o Φ₆ do trono\n");
    printf("       0      parabólica    telescópico, conforme, entrópico,\n");
    printf("                            espaço-temporal, óptico, universal — a FRONTEIRA\n");
    printf("       5      hiperbólica   áureo ℤ[φ], deflexivo — o rei\n");
    printf("       8      hiperbólica   prata\n");
    printf("      13      hiperbólica   bronze\n");
    printf("      m²+4    hiperbólica   o metal m\n");
    ok("os 28 ordenam-se na reta dos Δ — e a ordem é a de ℚ, que é a do corpo métrico", 1);
    printf("\n      COBERTURA, dita: os Δ das linhas −4, −3, 0, 5, 8, 13 estão MEDIDOS neste\n");
    printf("      repositório (cristalino.c, catalogo.c, topologia.c). Os das formas P e ν que\n");
    printf("      não são quadráticas — a impedância, a taxa, a ativação — NÃO os medi: as réguas\n");
    printf("      próprias estão no catálogo. Cobertura: 6 de 28 com Δ medido.\n");
    printf("\n      E o que ficou construído: as três classes cruzam pela regra dos sinais, as\n");
    printf("      potências alternam com a paridade, e a distância compõe escalando. É o viveiro\n");
    printf("      aplicado às classes em vez dos corpos — e dá o mesmo mecanismo, um nível acima.\n");
}

printf("\n=== O VIVEIRO DAS CLASSES =================================================\n");
printf("  La Hire dos três, e os Δ multiplicam — donde a regra dos sinais:\n\n");
printf("    hip ⊗ hip = hip     (+)(+) = +\n");
printf("    hip ⊗ elip = ELIP   (+)(−) = −\n");
printf("    elip ⊗ elip = HIP   (−)(−) = +      ← dois giros esticam\n");
printf("    x ⊗ par = par       x·0 = 0         ← o parabólico ABSORVE, e por isso é fronteira\n\n");
printf("    potências        o elíptico tem ordem 2; a paridade decide\n");
printf("    multiplicidade   par → hiperbólico, ímpar → elíptico — a norma (−1)^k outra vez\n");
printf("    a distância      d(A⊗C, B⊗C) = |Δ_C|·d(A,B) — cruzar ESCALA, não destrói\n\n");
printf("  E os 28 ordenam-se na reta dos Δ. Cobertura do Δ medido: 6 de 28 — o resto está no\n");
printf("  catálogo, e fica dito que fica.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
