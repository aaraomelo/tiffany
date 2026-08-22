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
#define SQL_TIPO_INT8    20    /* o count() do Postgres é bigint, não int4 */

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
    /* A AUSÊNCIA ATRAVESSA O FIO. Uma célula vazia e uma célula ausente são
     * objectos diferentes — o `thm:bitunico` diz que a ausência é o dual, não
     * um valor —, e no protocolo isso escreve-se com o comprimento −1, não com
     * zero bytes. Sem esta máscara o motor sabia distingui-las e o wire não:
     * o valor ia certo e o NÍVEL ia errado. */
    unsigned char nulo[SQL_OUT_MAX_ROWS][SQL_OUT_MAX_COLS];
} SqlOut;

int  sql_abrir(const char *base);
void sql_fechar(void);
/* executa e, se out!=NULL, preenche tag/linhas (SELECT) ou só tag (DDL/DML) */
int  sql_executa(const char *sql, SqlOut *out);
/* os nomes das colunas de uma tabela — para o catálogo, que não os pode inventar */
int  sql_cols_de(const char *tabela, char nomes[][32], int cap);
/* A TRANSACÇÃO, e ela é o levantamento do `aranha.tex`: escrever é dobrar — o
 * valor novo cola-se por cima e a célula esquece qual era —, e guardar o
 * anterior é a coordenada que desdobra, com volta exacta. */
void sql_tx_abre(void);
int  sql_tx_desfaz(void);      /* 0 se a pilha encheu: então NÃO se desfaz nada */
void sql_tx_fecha(void);
int  sql_tx_cheia(void);
long sql_tx_escritas(void);
/* o campo G da transacção corrente, lido na pilha: |I|, |supp G| e max G */
void sql_tx_fibra(long *escritas, long *slots_distintos, long *maior_G,
                  long *soma_G);
/* o histograma de uma coluna — o campo G dos seus valores, para o passo
 * espectral: o tamanho de um join é (f*g)(0), a convolução na origem */
/* os passos da última varredura. O molde das linhas não ramifica — as quatro
 * faces correm num passo só —, pelo que este número depende do TAMANHO da
 * tabela e não do conteúdo dela. */
extern long sql_ultimos_passos;
/* os nós visitados na última descida da árvore — a PROFUNDIDADE do índice, que
 * não cresce com o tamanho da tabela. É esta a medida do índice, e não os
 * passos, que com ele são zero: zero não distingue não-correr de não-fazer. */
extern long sql_ultimos_nos;
/* o FNV do bytecode da última varredura: duas escritas que compilam para o
 * MESMO programa são o mesmo objecto, e não duas coisas com a mesma resposta */
extern long sql_ultimo_prog;
int  sql_histograma(const char *tabela, const char *coluna, long *hist, int n,
                    long *fora);

#endif
