/* sql_api.h — porta C do motor SQL (banco/sql.c) para pgwire / apps.
 *
 * Não é Postgres. É a face estável: abrir base, executar, capturar linhas.
 * O bytecode ISA e o .mem continuam dentro de sql.c.
 */
#ifndef SQL_API_H
#define SQL_API_H

#define SQL_OUT_MAX_COLS 8
#define SQL_OUT_MAX_ROWS 64
#define SQL_OUT_CELL     64

typedef struct {
    int ok;
    char err[160];
    char tag[80];                 /* CommandComplete: "SELECT 2", "INSERT 0 1", … */
    int ncols;
    int nrows;
    char col[SQL_OUT_MAX_COLS][32];
    char cell[SQL_OUT_MAX_ROWS][SQL_OUT_MAX_COLS][SQL_OUT_CELL];
} SqlOut;

int  sql_abrir(const char *base);
void sql_fechar(void);
/* executa e, se out!=NULL, preenche tag/linhas (SELECT) ou só tag (DDL/DML) */
int  sql_executa(const char *sql, SqlOut *out);

#endif
