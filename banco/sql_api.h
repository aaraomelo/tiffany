/* sql_api.h — porta C do motor SQL (banco/sql.c) para pgwire / apps.
 *
 * Não é Postgres. É a face estável: abrir base, executar, capturar linhas.
 * O bytecode ISA e o .mem continuam dentro de sql.c.
 */
#ifndef SQL_API_H
#define SQL_API_H

/* DEZASSEIS, e não oito. O oito bastava para as tabelas desta casa e para tudo
 * o que os medidores pediam — e caiu no primeiro cliente real: o `\d` do psql
 * pede TREZE colunas de propriedades numa só linha, lê-as por POSIÇÃO, e
 * truncar não serve, porque a posição oito passa a ser outra coisa. O limite
 * era invisível enquanto quem perguntava éramos nós. */
#define SQL_OUT_MAX_COLS 16
#define SQL_OUT_MAX_ROWS 64
#define SQL_OUT_CELL     64
#define SQL_TIPO_INT4    23
#define SQL_TIPO_TEXT    25

typedef struct {
    int ok;
    char err[160];
    char tag[80];                 /* CommandComplete: "SELECT 2", "INSERT 0 1", … */
    int ncols;
    int nrows;
    char col[SQL_OUT_MAX_COLS][32];
    /* O TIPO ACOMPANHA A COLUNA. Sem isto o wire anunciava tudo como int4 —
     * incluindo `SELECT version()`, que devolve texto —, e um cliente que
     * confiasse no OID convertia mal. O valor ia certo e o TIPO ia errado, que é
     * o defeito que nenhuma comparação de strings apanha. */
    int tipo[SQL_OUT_MAX_COLS];   /* PG_OID_INT4 (23) ou PG_OID_TEXT (25) */
    char cell[SQL_OUT_MAX_ROWS][SQL_OUT_MAX_COLS][SQL_OUT_CELL];
} SqlOut;

int  sql_abrir(const char *base);
void sql_fechar(void);
/* executa e, se out!=NULL, preenche tag/linhas (SELECT) ou só tag (DDL/DML) */
int  sql_executa(const char *sql, SqlOut *out);
/* os nomes das colunas de uma tabela — para o catálogo, que não os pode inventar */
int  sql_cols_de(const char *tabela, char nomes[][32], int cap);

#endif
