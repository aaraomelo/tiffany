/* rlc_sql.c — O RLC RESOLVIDO **PELO SQL**: nenhuma conta fora do motor.
 *
 * A conta está toda em banco/sql.c (a operação EDO, mat_op 31). Aqui só se
 * escreve a companheira na tabela e se lê o que o motor devolve --- nem o Δ,
 * nem as raízes, nem a forma da solução são calculados deste lado.
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include <stdio.h>
#include <string.h>

int main(void){
    SqlOut o, o2;
    char q[240];
    unlink("/tmp/rl.mem"); unlink("/tmp/rl.prog");
    if(!sql_abrir("/tmp/rl")) return 1;
    struct { const char *nome; long R, L, invLC; } K[] = {
      {"sobreamortecido  R=5 L=1 1/LC=6", 5, 1, 6},
      {"crítico          R=4 L=1 1/LC=4", 4, 1, 4},
      {"subamortecido    R=2 L=1 1/LC=5", 2, 1, 5},
      {"sem perdas       R=0 L=1 1/LC=4", 0, 1, 4},
      {"só resistivo     R=3 L=1 1/LC=0", 3, 1, 0},
    };
    int NK = (int)(sizeof K/sizeof K[0]);
    printf("\n  O RLC RESOLVIDO PELO SQL --- a conta está em banco/sql.c, não aqui\n\n");
    printf("  circuito                          a equação (do motor)     Δ   classe"
           "       λ₁, λ₂            y(t)  (do motor)\n");
    printf("  ─────────────────────────────────────────────────────────────────────────"
           "────────────────────────────────────────\n");
    for(int c = 0; c < NK; c++){
        long B = K[c].R / K[c].L, Ce = K[c].invLC;
        sql_executa("DROP TABLE IF EXISTS A", &o2);
        sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
        sql_executa("INSERT INTO A VALUES (0,1)", &o2);
        snprintf(q, sizeof q, "INSERT INTO A VALUES (%ld,%ld)", -Ce, -B);
        sql_executa(q, &o2);
        sql_executa("SELECT edo(*) FROM A", &o);          /* <<< o SQL resolve */
        if(!o.ok){ printf("  %-33s RECUSADA: %s\n", K[c].nome, o.err); continue; }
        printf("  %-33s %-22s %4s  %-11s  %-16s  %s\n", K[c].nome,
               o.cell[0][0], o.cell[0][1], o.cell[0][2],
               (strlen(o.cell[0][3]) + strlen(o.cell[0][4]) < 15)
                   ? ({ static char t[40]; snprintf(t,sizeof t,"%s, %s",o.cell[0][3],o.cell[0][4]); t; })
                   : o.cell[0][3],
               o.cell[0][5]);
    }
    printf("  ─────────────────────────────────────────────────────────────────────────"
           "────────────────────────────────────────\n");
    /* ── E A RECUSA: uma matriz que não é companheira não resolve EDO nenhuma ── */
    sql_executa("DROP TABLE IF EXISTS M", &o2);
    sql_executa("CREATE TABLE M (x RACIONAL, y RACIONAL)", &o2);
    sql_executa("INSERT INTO M VALUES (2,3)", &o2);
    sql_executa("INSERT INTO M VALUES (1,4)", &o2);
    sql_executa("SELECT edo(*) FROM M", &o);
    printf("  a matriz que NÃO é companheira: o motor %s\n",
           o.ok ? "ACEITOU (mal)" : "recusa e diz porquê");
    sql_fechar();
    printf("\n");
    return 0;
}
