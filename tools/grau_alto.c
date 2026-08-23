/* grau_alto.c — SISTEMAS REAIS DE GRAU ALTO, resolvidos pelo motor.
 *
 * Grau dois é um sistema com UM elemento que armazena de cada tipo. Assim que
 * há mais --- duas massas, quatro malhas, uma viga --- o grau sobe, e é aí que
 * a resolução tem de dar mais do que uma lista de raízes: tem de dar a
 * MULTIPLICIDADE (que produz os t^k) e a FORMA da solução.
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* escreve a companheira de um polinómio mónico de grau n e pergunta ao motor */
static void resolve(const char *nome, const char *fisica, int n, const long *co){
    SqlOut o, o2; char q[600];
    sql_executa("DROP TABLE IF EXISTS A", &o2);
    strcpy(q, "CREATE TABLE A (");
    for(int j = 0; j < n; j++){ char t[24];
        snprintf(t, sizeof t, "%sc%d RACIONAL", j?",":"", j); strcat(q, t); }
    strcat(q, ")"); sql_executa(q, &o2);
    for(int i = 0; i < n; i++){
        strcpy(q, "INSERT INTO A VALUES (");
        for(int j = 0; j < n; j++){
            long v = (i+1 < n) ? ((j == i+1) ? 1 : 0) : -co[j];
            char t[24]; snprintf(t, sizeof t, "%s%ld", j?",":"", v); strcat(q, t);
        }
        strcat(q, ")"); sql_executa(q, &o2);
    }
    sql_executa("SELECT edo(*) FROM A", &o);
    printf("\n  %s\n    %s\n", nome, fisica);
    if(!o.ok){ printf("    RECUSADA: %s\n", o.err); return; }
    printf("    grau %s · %s raízes em %s distintas: %s\n",
           o.cell[0][0], o.cell[0][3], o.cell[0][2], o.cell[0][1]);
    printf("    o resto: %s\n", o.cell[0][4]);
    printf("    y(t) = %s\n", o.cell[0][5]);
}

int main(void){
    unlink("/tmp/ga.mem"); unlink("/tmp/ga.prog");
    if(!sql_abrir("/tmp/ga")) return 1;
    printf("\n  ══ SISTEMAS DE GRAU ALTO, resolvidos pelo motor ══\n");

    /* GRAU 4 — A VIGA. Euler-Bernoulli sem carga: EI·y'''' = 0, isto é λ⁴ = 0,
     * raiz QUÁDRUPLA em zero. A multiplicidade é que produz o cúbico da viga:
     * y = c1 + c2x + c3x² + c4x³ --- e é exactamente o que se usa em estruturas. */
    { long co[4] = {0,0,0,0};
      resolve("GRAU 4 — a viga de Euler-Bernoulli",
              "EI·y'''' = 0 : λ⁴ = 0, raiz quádrupla. A multiplicidade É o cúbico da viga.",
              4, co); }

    /* GRAU 4 — DUAS MASSAS ACOPLADAS, com amortecimento. Cada massa dá segunda
     * ordem, e o acoplamento junta-as: (λ²+3λ+2)(λ²+7λ+12) = raízes −1,−2,−3,−4 */
    { long co[4] = {24, 50, 35, 10};
      resolve("GRAU 4 — duas massas acopladas com amortecimento",
              "(λ²+3λ+2)(λ²+7λ+12) : os quatro modos, todos a decair.",
              4, co); }

    /* GRAU 6 — TRÊS MALHAS RLC em cascata: (λ+1)²(λ+2)²(λ+3)² */
    /* (λ+1)²(λ+2)²(λ+3)² tem coeficientes 193 e 144, que NÃO cabem no envelope
     * da célula --- e o que recusa é o envelope, não o grau. Usa-se (λ²−1)³,
     * que é grau seis com coeficientes pequenos e a mesma lição: raízes com
     * multiplicidade TRÊS, que produzem os t e t². */
    { long c2[6] = {-1, 0, 3, 0, -3, 0};        /* λ⁶ −3λ⁴ +3λ² −1 = (λ²−1)³ */
      resolve("GRAU 6 — três estágios iguais em cascata",
              "(λ²−1)³ : cada raiz TRIPLA, e a multiplicidade dá os t·e^{λt} e t²·e^{λt}.",
              6, c2); }

    /* GRAU 5 — quando NEM TUDO é racional: λ⁵ − λ⁴ − λ + 1 = (λ−1)²(λ³+λ²+λ+1)?
     * Não: usa-se (λ−1)(λ+1)(λ³−λ−1) --- as duas racionais saem, o metal fica. */
    { long a[4] = {-1,-1,0,1};              /* λ³ − λ − 1, o metal */
      long b[3] = {-1,0,1};                 /* λ² − 1 */
      long c[6] = {0,0,0,0,0,0};
      for(int i=0;i<4;i++) for(int j=0;j<3;j++) if(i+j<6) c[i+j]+=a[i]*b[j];
      resolve("GRAU 5 — com uma parte que NÃO é racional",
              "(λ²−1)(λ³−λ−1) : as duas racionais saem, e o metal fica DITO.",
              5, c); }

    /* GRAU 8 — o andar. (λ²+1)⁴? não é racional. Usa-se (λ−1)⁴(λ+1)⁴ */
    { long c[8]; { long t[9]={0,0,0,0,0,0,0,0,1};
        /* (λ²−1)⁴ = λ⁸ −4λ⁶ +6λ⁴ −4λ² +1 */
        long u[9]={1,0,-4,0,6,0,-4,0,1};
        for(int k=0;k<8;k++) c[k]=u[k]; (void)t; }
      resolve("GRAU 8 — (λ²−1)⁴, quatro pares de raízes quádruplas",
              "(λ−1)⁴(λ+1)⁴ : duas raízes distintas, multiplicidade quatro cada.",
              8, c); }
    printf("\n");
    sql_fechar();
    return 0;
}
