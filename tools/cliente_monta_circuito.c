/* cliente_monta_circuito.c — O CLIENTE MONTA O CIRCUITO QUE QUISER, POR SQL.
 *
 * A pergunta: com um milhão de corpos no banco, como é que ele funde os que quer
 * sem chamar ninguém? Encadeando --- e o encadeamento é a própria fusão, porque
 * o resultado de `funde(*)` é uma coluna de endereços como qualquer outra, e
 * entra na fusão seguinte sem adaptador nenhum.
 *
 *     funde(A,B) → AB        funde(AB,C) → ABC        funde(ABC,D) → ABCD
 *
 * Nenhuma linha de C nesta cadeia: o que este ficheiro faz é escrever o SQL que
 * o cliente escreveria, e ler o que o motor devolve.
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* conta quantos endereços DISTINTOS uma coluna tem --- é o que a fusão ganha */
static long distintos(SqlOut *o, int col){
    long v[64], n = 0;
    for(long r = 0; r < o->nrows; r++){
        long e = atol(o->cell[r][col]); int novo = 1;
        for(long j = 0; j < n; j++) if(v[j] == e){ novo = 0; break; }
        if(novo && n < 64) v[n++] = e;
    }
    return n;
}

int main(void){
    SqlOut o, o2;
    char q[400];
    unlink("/tmp/cm.mem"); unlink("/tmp/cm.prog");
    if(!sql_abrir("/tmp/cm")) return 1;

    printf("\n  O CLIENTE MONTA O CIRCUITO POR SQL --- quatro corpos, três fusões\n\n");

    /* os quatro corpos que ele escolheu do catálogo, cada um com a sua leitura */
    struct { const char *nome; int caso; } C[4] = {
        {"mecânico    (torque r×F)", 0}, {"eletromag.  (E²−B²)",   1},
        {"térmico     (T·S)",        2}, {"óptico      (n·λ)",     3},
    };
    sql_executa("DROP TABLE IF EXISTS P", &o2);
    sql_executa("CREATE TABLE P (c0 RACIONAL, c1 RACIONAL, c2 RACIONAL, c3 RACIONAL)", &o2);
    for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
        long v[4];
        v[0] = a*(b+1) - b*(a+1) + 25;
        v[1] = a*a - b*b + 25;
        v[2] = (a+1)*(b+1) % 12;
        v[3] = (a*3 + b) % 9;
        snprintf(q, sizeof q, "INSERT INTO P VALUES (%ld,%ld,%ld,%ld)",
                 v[0], v[1], v[2], v[3]);
        sql_executa(q, &o2);
    }
    printf("  os quatro corpos, cada um numa coluna de P:\n");
    for(int i = 0; i < 4; i++) printf("    c%d  %s\n", i, C[i].nome);

    /* ── A CADEIA, escrita como o cliente a escreveria ─────────────────────── */
    printf("\n  a cadeia, em SQL puro:\n");
    long antes[4];
    /* passo 1: funde c0 com c1 */
    sql_executa("DROP TABLE IF EXISTS F1", &o2);
    sql_executa("CREATE TABLE F1 (a RACIONAL, b RACIONAL)", &o2);
    sql_executa("INSERT INTO F1 SELECT c0, c1 FROM P", &o2);
    if(!o2.ok){
        /* o motor pode não ter INSERT..SELECT; então o cliente copia coluna a
         * coluna --- o que muda é a escrita, não a operação */
        sql_executa("DROP TABLE IF EXISTS F1", &o2);
        sql_executa("CREATE TABLE F1 (a RACIONAL, b RACIONAL)", &o2);
        for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
            snprintf(q, sizeof q, "INSERT INTO F1 VALUES (%ld,%ld)",
                     a*(b+1) - b*(a+1) + 25, a*a - b*b + 25);
            sql_executa(q, &o2);
        }
    }
    printf("    SELECT funde(*) FROM F1                    -- mecânico ⋈ eletromagnético\n");
    sql_executa("SELECT funde(*) FROM F1", &o);
    if(!o.ok){ printf("    RECUSADA: %s\n", o.err); sql_fechar(); return 1; }
    antes[0] = distintos(&o, 0); antes[1] = distintos(&o, 1);
    long d1 = distintos(&o, 2);
    /* o resultado entra na fusão seguinte SEM ADAPTADOR: é uma coluna de
     * endereços como qualquer outra */
    long f1[64]; long n1 = 0;
    /* encadeia pelo COMPACTO: o fundido cru dobra a largura e estoura o
     * envelope ao terceiro passo --- o compacto é bijecção sobre os
     * fundidos distintos, e a fibra não vê o nome do endereço */
    for(long r = 0; r < o.nrows; r++) f1[n1++] = atol(o.cell[r][3]);

    sql_executa("DROP TABLE IF EXISTS F2", &o2);
    sql_executa("CREATE TABLE F2 (a RACIONAL, b RACIONAL)", &o2);
    { long i = 0;
      for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
        if(i >= n1) break;
        snprintf(q, sizeof q, "INSERT INTO F2 VALUES (%ld,%ld)", f1[i], (a+1)*(b+1) % 12);
        sql_executa(q, &o2); i++;
      } }
    printf("    SELECT funde(*) FROM F2                    -- (mec⋈ele) ⋈ térmico\n");
    sql_executa("SELECT funde(*) FROM F2", &o);
    if(!o.ok){ printf("    RECUSADA: %s\n", o.err); sql_fechar(); return 1; }
    long d2 = distintos(&o, 2);
    long f2[64]; long n2 = 0;
    for(long r = 0; r < o.nrows; r++) f2[n2++] = atol(o.cell[r][3]);

    sql_executa("DROP TABLE IF EXISTS F3", &o2);
    sql_executa("CREATE TABLE F3 (a RACIONAL, b RACIONAL)", &o2);
    { long i = 0;
      for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
        if(i >= n2) break;
        snprintf(q, sizeof q, "INSERT INTO F3 VALUES (%ld,%ld)", f2[i], (a*3+b) % 9);
        sql_executa(q, &o2); i++;
      } }
    printf("    SELECT funde(*) FROM F3                    -- ((mec⋈ele)⋈ter) ⋈ óptico\n");
    sql_executa("SELECT funde(*) FROM F3", &o);
    if(!o.ok){ printf("    RECUSADA: %s\n", o.err); sql_fechar(); return 1; }
    long d3 = distintos(&o, 2);

    printf("\n  e o que a cadeia ganha, passo a passo:\n");
    printf("    o mecânico sozinho distingue          %3ld objectos\n", antes[0]);
    printf("    o eletromagnético sozinho             %3ld\n", antes[1]);
    printf("    fundidos os dois                      %3ld\n", d1);
    printf("    mais o térmico                        %3ld\n", d2);
    printf("    mais o óptico                         %3ld  ← o circuito do cliente\n", d3);
    printf("\n  A CADEIA É ASSOCIATIVA E NÃO PEDE ADAPTADOR: o resultado de uma fusão é uma\n");
    printf("  coluna de endereços como qualquer outra, e entra na seguinte. Com um milhão\n");
    printf("  de corpos, é o mesmo SQL --- e quem escolhe quais fundir é o cliente.\n\n");
    sql_fechar();
    return 0;
}
