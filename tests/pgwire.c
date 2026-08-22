/* pgwire.c — FEBE: Trios PG1–PG4 (frames, Simple, TCP, Extended Query).
 *
 * Referência: Postgres Protocol Message Formats — implementação Tiffany do zero.
 * Sem libpq, sem servidor Postgres instalado.
 *
 *   §W0  frames tipados: AuthOk, ReadyForQuery, BackendKeyData, Terminate
 *   §W1  StartupMessage ↔ parse; SSLRequest detectado e recusado
 *   §W2  handshake trust completo (AuthOk → ParameterStatus* → Key → Ready)
 *   §W2b ErrorResponse + Query encode
 *   §W3  Simple Query: sql_api → RowDescription/DataRow/CommandComplete/Ready
 *   §W4  listener TCP 127.0.0.1: SSL→N, Startup, Query SELECT, Terminate
 *   §W5  Extended: Parse/Bind($1)/Execute/Sync ≡ Simple WHERE a=7
 *   §W6  Describe statement + Close + Sync
 *   §W7  fachada pqlike (Trio PG5): PQconnectdb/PQexec/PQntuples/PQgetvalue sobre o
 *        NOSSO wire — sem -lpq — e a MESMA consulta pelos dois caminhos
 *   §W23 as CLÁUSULAS fecham entre si: a conservação, a tricotomia e as
 *        fibras vazias
 *   §W22 O CONTADOR SOBE DE ANDAR: nrows e count de 16 bits (OP_ADD16), em vez
 *        de dar a volta aos 255
 *   §W21 MAX-CUT = MASSA: o pior caso da junção é o corte, e a massa decide
 *        qual junção é mais cara sem ler uma linha
 *   §W20 o SALDO ESPECTRAL: o tamanho do join pela transformada, sem casar
 *        linhas — dois caminhos para o mesmo número
 *   §W19 o JOIN como BIPARTIÇÃO — o corte, dual da ordem, na mesma árvore
 *   §W18 o ORDER BY pela ÁRVORE do banco, com a fibra dos repetidos separada
 *   §W17 a PROJECÇÃO de colunas, e ORDER BY/LIMIT recusados em vez de ignorados
 *   §W16 o MOTOR como realização: a pilha É a trajectória, ∑G = |I| medido no
 *        banco, a fibra do WHERE, e o representante k=1
 *   §W15 o ROLLBACK DESFAZ, pelo levantamento: guardar o valor anterior e ler
 *        a pilha ao contrário
 *   §W14 count(*), que conta ALÉM do que o SqlOut comporta; e o \l
 *   §W13 o `\d <tabela>`: cinco consultas, o oid estável, as colunas do motor
 *   §W12 o tipo `vector` no catálogo, e os scripts do repo como FUNÇÕES —
 *        com a lista branca e o gume a forçar a porta
 *   §W11 pg_type lido COMO TABELA, e a diferença entre zero linhas e recusa
 *   §W10 a lista de tabelas do \dt por reconhecimento de assinatura, e a
 *        FRONTEIRA: o resto de pg_catalog continua recusado
 *   §W9  o TIPO acompanha a coluna: catálogo TEXT, motor INT4 — e o gume é as
 *        duas respostas terem de ser diferentes
 *   §W8  Trio PG6: o catálogo de SESSÃO — version/SHOW/SET/current_* e as tags de
 *        transacção — ANTES do motor, com o controlo a exigir que o motor corra
 *
 *   cc -O2 -std=c99 -w -Ilib -Ibanco -DSQL_NO_MAIN -DPGWIRE_NO_MAIN -o /tmp/pgwire \
 *      tests/pgwire.c banco/sql.c banco/pgwire.c -lm && /tmp/pgwire
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <ctype.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pgwire_sess.h"
#include "pqlike.h"
#include "pgcat.h"

/* Lê mensagens tipadas até ReadyForQuery (ou erro). */
static int cli_ate_ready(int fd, uint8_t *acc, int cap, int *nacc){
    int n = 0;
    while(n + 5 < cap){
        uint8_t hdr[5];
        int32_t mlen, pay;
        if(pgwire_recv_n(fd, hdr, 5) < 0) return -1;
        if(n + 5 > cap) return -1;
        memcpy(acc + n, hdr, 5); n += 5;
        mlen = pg_get_i32(hdr + 1);
        if(mlen < 4) return -1;
        pay = mlen - 4;
        if(n + pay > cap) return -1;
        if(pay > 0 && pgwire_recv_n(fd, acc + n, pay) < 0) return -1;
        n += pay;
        if(hdr[0] == PG_MSG_READY || hdr[0] == PG_MSG_ERROR){
            *nacc = n;
            return hdr[0] == PG_MSG_READY ? 0 : -1;
        }
    }
    return -1;
}

int main(void){
    printf("\n=== pgwire FEBE (Trios PG1–PG4) — buffer + TCP + Extended ===\n");

    /* ═══ §W0 frames tipados bit a bit ═══════════════════════════════════════ */
    printf("\n§W0 Frames tipados: AuthOk, Ready, Key, Terminate.\n\n");
    {
        PgBuf w;
        long mal = 0;

        pg_buf_limpa(&w);
        pg_put_auth_ok(&w);
        if(w.erro || w.n != 9) mal++;
        else if(w.b[0] != 'R' || pg_get_i32(w.b + 1) != 8 || pg_get_i32(w.b + 5) != 0) mal++;
        printf("      AuthenticationOk: %d bytes  %s\n", w.n, mal ? "FALHA" : "ok");

        pg_buf_limpa(&w);
        pg_put_ready(&w, PG_TX_IDLE);
        if(w.erro || w.n != 6) mal++;
        else if(w.b[0] != 'Z' || pg_get_i32(w.b + 1) != 5 || w.b[5] != 'I') mal++;
        printf("      ReadyForQuery(I):  %d bytes  %s\n", w.n, mal ? "…" : "ok");

        pg_buf_limpa(&w);
        pg_put_backend_key(&w, 42, 0xDEADBEEF);
        if(w.erro || w.n != 13) mal++;
        else if(w.b[0] != 'K' || pg_get_i32(w.b + 1) != 12
                || pg_get_i32(w.b + 5) != 42
                || (uint32_t)pg_get_i32(w.b + 9) != 0xDEADBEEFu) mal++;
        printf("      BackendKeyData:    %d bytes  %s\n", w.n, mal ? "…" : "ok");

        pg_buf_limpa(&w);
        pg_put_terminate(&w);
        if(w.erro || w.n != 5) mal++;
        else if(w.b[0] != 'X' || pg_get_i32(w.b + 1) != 4) mal++;
        printf("      Terminate:         %d bytes  %s\n", w.n, mal ? "…" : "ok");

        ok("§W0 frames FEBE: AuthOk/Ready/Key/Terminate exactos (len big-endian)",
           mal == 0);
    }

    /* ═══ §W1 Startup + SSLRequest ═══════════════════════════════════════════ */
    printf("\n§W1 StartupMessage parse; SSLRequest detectado.\n\n");
    {
        PgBuf w;
        PgStartup st;
        PgHead8 h;
        long mal = 0;

        pg_buf_limpa(&w);
        pg_put_startup(&w, PG_PROTO_3_0, "tiffany", "reino");
        if(w.erro || !pg_parse_startup(w.b, w.n, &st) || !st.ok) mal++;
        else if(st.proto != PG_PROTO_3_0) mal++;
        else if(strcmp(st.user, "tiffany") != 0) mal++;
        else if(strcmp(st.database, "reino") != 0) mal++;
        printf("      Startup user=tiffany db=reino proto=3.0: %s\n",
               mal ? "FALHA" : "ok");

        /* default database = user */
        pg_buf_limpa(&w);
        pg_put_startup(&w, PG_PROTO_3_0, "ana", NULL);
        memset(&st, 0, sizeof st);
        if(!pg_parse_startup(w.b, w.n, &st) || strcmp(st.database, "ana") != 0) mal++;

        /* SSLRequest */
        pg_buf_limpa(&w);
        pg_put_ssl_request(&w);
        if(!pg_peek_head8(w.b, w.n, &h) || !h.e_ssl) mal++;
        if(pg_parse_startup(w.b, w.n, &st)) mal++;   /* não é Startup */
        {
            PgBuf r; pg_buf_limpa(&r);
            pg_put_ssl_reply(&r, 0);
            if(r.n != 1 || r.b[0] != 'N') mal++;
        }
        printf("      SSLRequest → 'N' (recusa): %s\n", mal ? "FALHA" : "ok");

        /* user obrigatório */
        pg_buf_limpa(&w);
        {
            int off = w.n;
            pg_buf_i32(&w, 0);
            pg_buf_i32(&w, PG_PROTO_3_0);
            pg_buf_u8(&w, 0);   /* sem pares */
            pg_put_i32(w.b + off, (int32_t)(w.n - off));
        }
        if(pg_parse_startup(w.b, w.n, &st)) mal++;

        ok("§W1 Startup↔parse (user/database/proto); SSLRequest≠Startup; reply 'N'",
           mal == 0);
    }

    /* ═══ §W2 handshake trust completo ═══════════════════════════════════════ */
    printf("\n§W2 Handshake trust: AuthOk → params → Key → Ready.\n\n");
    {
        PgBuf w;
        int off = 0, mal = 0, viu_auth = 0, viu_ready = 0, viu_key = 0, n_param = 0;
        char tipo;
        const uint8_t *pay;
        int pn;

        pg_buf_limpa(&w);
        pg_put_startup_ok(&w, 7, 99);
        if(w.erro) mal++;

        while(off < w.n){
            if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn)){ mal++; break; }
            if(tipo == PG_MSG_AUTH){
                if(pn != 4 || pg_get_i32(pay) != 0) mal++;
                viu_auth = 1;
            }else if(tipo == PG_MSG_PARAM_STATUS){
                n_param++;
            }else if(tipo == PG_MSG_BACKEND_KEY){
                if(pn != 8 || pg_get_i32(pay) != 7 || pg_get_i32(pay + 4) != 99) mal++;
                viu_key = 1;
            }else if(tipo == PG_MSG_READY){
                if(pn != 1 || pay[0] != PG_TX_IDLE) mal++;
                viu_ready = 1;
            }else mal++;
        }
        printf("      mensagens: auth=%d params=%d key=%d ready=%d  bytes=%d\n",
               viu_auth, n_param, viu_key, viu_ready, w.n);
        ok("§W2 handshake trust completo e legível mensagem a mensagem",
           mal == 0 && viu_auth && viu_key && viu_ready && n_param >= 1 && off == w.n);
    }

    /* ═══ §W2b ErrorResponse + Query encode ══════════════════════════════════ */
    printf("\n§W2b ErrorResponse e Query (prep. Simple Query / Trio PG2).\n\n");
    {
        PgBuf w;
        int off = 0, mal = 0;
        char tipo;
        const uint8_t *pay;
        int pn;

        pg_buf_limpa(&w);
        pg_put_error(&w, "42601", "syntax error");
        if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn) || tipo != 'E') mal++;
        else{
            /* procura campo M */
            int i = 0, achou = 0;
            while(i < pn){
                char c = (char)pay[i++];
                if(c == 0) break;
                {
                    const char *s = (const char *)(pay + i);
                    int L = (int)strlen(s);
                    if(c == PG_ERR_MESSAGE && strcmp(s, "syntax error") == 0) achou = 1;
                    if(c == PG_ERR_SQLSTATE && strcmp(s, "42601") != 0) mal++;
                    i += L + 1;
                }
            }
            if(!achou) mal++;
        }

        pg_buf_limpa(&w);
        pg_put_query(&w, "SELECT 1");
        off = 0;
        if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn) || tipo != 'Q') mal++;
        else if(pn < 2 || strcmp((const char *)pay, "SELECT 1") != 0) mal++;

        ok("§W2b ErrorResponse(M/C) e Query('SELECT 1') round-trip no buffer",
           mal == 0);
    }

    /* ═══ §W3 Simple Query → sql.c ═══════════════════════════════════════════ */
    printf("\n§W3 Simple Query: CREATE/INSERT/SELECT → RowDesc/DataRow/Complete.\n\n");
    {
        const char *base = "/tmp/pgwire_w3";
        SqlOut out;
        PgBuf w, q;
        int off, mal = 0;
        char tipo;
        const uint8_t *pay;
        int pn;
        int viu_t = 0, viu_d = 0, viu_c = 0, viu_z = 0, n_data = 0;
        char cell0[SQL_OUT_CELL], cell1[SQL_OUT_CELL], cell2[SQL_OUT_CELL];

        unlink("/tmp/pgwire_w3.mem");
        unlink("/tmp/pgwire_w3.prog");
        cell0[0] = cell1[0] = cell2[0] = 0;

        if(!sql_abrir(base)){ mal++; printf("      sql_abrir FALHOU\n"); }
        else{
            if(!sql_executa("CREATE TABLE t (a,b,c)", &out) || strcmp(out.tag, "CREATE TABLE") != 0)
                mal++;
            printf("      CREATE TABLE tag=%s  %s\n", out.tag, mal ? "FALHA" : "ok");

            /* CommandComplete só (sem RowDesc) */
            pg_buf_limpa(&w);
            pg_reply_sql(&w, &out);
            off = 0;
            if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn) || tipo != 'C') mal++;
            else if(strcmp((const char *)pay, "CREATE TABLE") != 0) mal++;
            if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn) || tipo != 'Z') mal++;

            if(!sql_executa("INSERT INTO t VALUES (7,10,20)", &out)
               || strncmp(out.tag, "INSERT", 6) != 0) mal++;
            if(!sql_executa("INSERT INTO t VALUES (3,30,40)", &out)) mal++;

            if(!sql_executa("SELECT * FROM t", &out) || !out.ok) mal++;
            else if(out.ncols != 3 || out.nrows != 2) mal++;
            else if(strcmp(out.cell[0][0], "7") != 0 || strcmp(out.cell[1][0], "3") != 0) mal++;
            printf("      SELECT captura: ncols=%d nrows=%d tag=%s  %s\n",
                   out.ncols, out.nrows, out.tag, mal ? "FALHA" : "ok");

            /* Query FE → reply BE */
            pg_buf_limpa(&q);
            pg_put_query(&q, "SELECT * FROM t");
            off = 0;
            if(!pg_read_typed(q.b, q.n, &off, &tipo, &pay, &pn) || tipo != 'Q') mal++;

            pg_buf_limpa(&w);
            pg_reply_sql(&w, &out);
            if(w.erro) mal++;
            off = 0;
            while(off < w.n){
                if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn)){ mal++; break; }
                if(tipo == PG_MSG_ROW_DESC){
                    if(pn < 2 || pg_get_i16(pay) != 3) mal++;
                    viu_t = 1;
                }else if(tipo == PG_MSG_DATA_ROW){
                    int nc, j, p;
                    if(pn < 2){ mal++; continue; }
                    nc = pg_get_i16(pay);
                    if(nc != 3) mal++;
                    p = 2;
                    for(j = 0; j < nc && p + 4 <= pn; j++){
                        int L = pg_get_i32(pay + p); p += 4;
                        if(L < 0 || p + L > pn){ mal++; break; }
                        if(n_data == 0 && j < 3){
                            char *dest = (j == 0) ? cell0 : (j == 1) ? cell1 : cell2;
                            if(L >= SQL_OUT_CELL) L = SQL_OUT_CELL - 1;
                            memcpy(dest, pay + p, (size_t)L);
                            dest[L] = 0;
                        }
                        p += L;
                    }
                    n_data++;
                    viu_d = 1;
                }else if(tipo == PG_MSG_CMD_COMPLETE){
                    if(strcmp((const char *)pay, out.tag) != 0) mal++;
                    viu_c = 1;
                }else if(tipo == PG_MSG_READY){
                    if(pn != 1 || pay[0] != PG_TX_IDLE) mal++;
                    viu_z = 1;
                }else mal++;
            }
            printf("      FEBE: T=%d D=%d(n=%d) C=%d Z=%d  cells0=%s|%s|%s  %s\n",
                   viu_t, viu_d, n_data, viu_c, viu_z, cell0, cell1, cell2,
                   mal ? "FALHA" : "ok");
            if(!viu_t || !viu_d || !viu_c || !viu_z || n_data != 2) mal++;
            if(strcmp(cell0, "7") != 0 || strcmp(cell1, "10") != 0 || strcmp(cell2, "20") != 0)
                mal++;

            sql_fechar();
        }

        ok("§W3 Simple Query: sql_api + RowDescription/DataRow/CommandComplete/Ready",
           mal == 0);
    }

    /* ═══ §W4 listener TCP localhost (Trio PG3) ══════════════════════════════ */
    printf("\n§W4 Listener TCP: SSL→N, Startup, Query SELECT, Terminate.\n\n");
    {
        const char *base = "/tmp/pgwire_w4";
        SqlOut seed;
        int lfd = -1, porto = 0, mal = 0;
        int sv[2] = {-1, -1};
        pid_t kid = -1;
        char ready = 0;

        unlink("/tmp/pgwire_w4.mem");
        unlink("/tmp/pgwire_w4.prog");
        signal(SIGPIPE, SIG_IGN);

        if(!sql_abrir(base)) mal++;
        else{
            sql_executa("CREATE TABLE t (a,b,c)", &seed);
            sql_executa("INSERT INTO t VALUES (7,10,20)", &seed);
            sql_executa("INSERT INTO t VALUES (3,30,40)", &seed);
            sql_fechar();
        }

        if(pipe(sv) < 0) mal++;
        if(!mal) kid = fork();
        if(!mal && kid < 0) mal++;

        if(!mal && kid == 0){
            /* filho: escuta, avisa, atende uma ligação, sai */
            int cfd;
            close(sv[0]);
            if(!sql_abrir(base)) _exit(3);
            lfd = pgwire_listen(0, &porto);
            if(lfd < 0){ sql_fechar(); _exit(4); }
            /* porto efectivo no pipe: 2 bytes big-endian + 'R' */
            {
                uint8_t msg[3];
                msg[0] = (uint8_t)((porto >> 8) & 0xff);
                msg[1] = (uint8_t)(porto & 0xff);
                msg[2] = 'R';
                if(write(sv[1], msg, 3) != 3){ close(lfd); sql_fechar(); _exit(5); }
            }
            cfd = accept(lfd, NULL, NULL);
            if(cfd < 0){ close(lfd); sql_fechar(); _exit(6); }
            pgwire_serve_conn(cfd, 11, 22);
            close(cfd); close(lfd); sql_fechar();
            _exit(0);
        }

        if(!mal && kid > 0){
            uint8_t msg[3];
            int cfd = -1, off, viu_t = 0, n_data = 0, viu_c = 0, viu_z = 0;
            char tipo, cell0[64];
            const uint8_t *pay;
            int pn;
            PgBuf w;
            uint8_t acc[8192];
            int nacc = 0;
            struct sockaddr_in a;
            int st = -1;

            close(sv[1]); sv[1] = -1;
            if(read(sv[0], msg, 3) != 3 || msg[2] != 'R') mal++;
            else porto = ((int)msg[0] << 8) | (int)msg[1];
            close(sv[0]); sv[0] = -1;
            printf("      filho a escutar porto=%d\n", porto);

            cfd = socket(AF_INET, SOCK_STREAM, 0);
            if(cfd < 0) mal++;
            memset(&a, 0, sizeof a);
            a.sin_family = AF_INET;
            a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            a.sin_port = htons((uint16_t)porto);
            if(!mal && connect(cfd, (struct sockaddr *)&a, sizeof a) < 0){
                printf("      connect: %s\n", strerror(errno));
                mal++;
            }

            /* SSLRequest → 'N' */
            if(!mal){
                uint8_t nbyte = 0;
                pg_buf_limpa(&w);
                pg_put_ssl_request(&w);
                if(pgwire_send_all(cfd, w.b, w.n) < 0) mal++;
                else if(pgwire_recv_n(cfd, &nbyte, 1) < 0 || nbyte != 'N') mal++;
                printf("      SSLRequest → '%c'  %s\n", nbyte ? (char)nbyte : '?',
                       mal ? "FALHA" : "ok");
            }

            /* Startup + handshake até Ready */
            if(!mal){
                pg_buf_limpa(&w);
                pg_put_startup(&w, PG_PROTO_3_0, "tiffany", "reino");
                if(pgwire_send_all(cfd, w.b, w.n) < 0) mal++;
                else if(cli_ate_ready(cfd, acc, (int)sizeof acc, &nacc) < 0) mal++;
                printf("      Startup→Ready: %d bytes  %s\n", nacc, mal ? "FALHA" : "ok");
            }

            /* Query SELECT */
            cell0[0] = 0;
            if(!mal){
                pg_buf_limpa(&w);
                pg_put_query(&w, "SELECT * FROM t");
                if(pgwire_send_all(cfd, w.b, w.n) < 0) mal++;
                else if(cli_ate_ready(cfd, acc, (int)sizeof acc, &nacc) < 0) mal++;
                off = 0;
                while(off < nacc){
                    if(!pg_read_typed(acc, nacc, &off, &tipo, &pay, &pn)){ mal++; break; }
                    if(tipo == PG_MSG_ROW_DESC){
                        if(pn < 2 || pg_get_i16(pay) != 3) mal++;
                        viu_t = 1;
                    }else if(tipo == PG_MSG_DATA_ROW){
                        int nc = pg_get_i16(pay), L, p = 2;
                        if(nc != 3) mal++;
                        L = pg_get_i32(pay + p); p += 4;
                        if(n_data == 0 && L > 0 && L < (int)sizeof cell0){
                            memcpy(cell0, pay + p, (size_t)L); cell0[L] = 0;
                        }
                        n_data++;
                    }else if(tipo == PG_MSG_CMD_COMPLETE){
                        if(strncmp((const char *)pay, "SELECT", 6) != 0) mal++;
                        viu_c = 1;
                    }else if(tipo == PG_MSG_READY){
                        if(pn != 1 || pay[0] != PG_TX_IDLE) mal++;
                        viu_z = 1;
                    }
                }
                printf("      Query SELECT: T=%d D=%d C=%d Z=%d cell0=%s  %s\n",
                       viu_t, n_data, viu_c, viu_z, cell0, mal ? "FALHA" : "ok");
                if(!viu_t || n_data != 2 || !viu_c || !viu_z || strcmp(cell0, "7") != 0)
                    mal++;
            }

            if(cfd >= 0){
                pg_buf_limpa(&w);
                pg_put_terminate(&w);
                pgwire_send_all(cfd, w.b, w.n);
                close(cfd);
            }
            if(waitpid(kid, &st, 0) < 0) mal++;
            else if(!WIFEXITED(st) || WEXITSTATUS(st) != 0){
                printf("      filho exit=%d\n", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
                mal++;
            }
        }

        if(sv[0] >= 0) close(sv[0]);
        if(sv[1] >= 0) close(sv[1]);

        ok("§W4 listener TCP localhost: SSL→N + Startup + SELECT + Terminate",
           mal == 0);
    }

    /* ═══ §W5 Extended Query: Bind $1 ≡ Simple ═══════════════════════════════ */
    printf("\n§W5 Extended Query: Parse/Bind($1)/Execute/Sync ≡ Simple.\n\n");
    {
        const char *base = "/tmp/pgwire_w5";
        SqlOut simple, seed;
        PgSess sess;
        PgBuf fe, be, one;
        int mal = 0, off, viu_1 = 0, viu_2 = 0, viu_t = 0, n_data = 0, viu_c = 0, viu_z = 0;
        char tipo, cell0[64];
        const uint8_t *pay;
        int pn;
        int32_t oid = PG_OID_INT4;

        unlink("/tmp/pgwire_w5.mem");
        unlink("/tmp/pgwire_w5.prog");
        cell0[0] = 0;

        if(!sql_abrir(base)) mal++;
        else{
            sql_executa("CREATE TABLE t (a,b,c)", &seed);
            sql_executa("INSERT INTO t VALUES (7,10,20)", &seed);
            sql_executa("INSERT INTO t VALUES (3,30,40)", &seed);

            memset(&simple, 0, sizeof simple);
            sql_executa("SELECT * FROM t WHERE a = 7", &simple);
            if(!simple.ok || simple.nrows != 1 || strcmp(simple.cell[0][0], "7") != 0)
                mal++;
            printf("      Simple WHERE a=7: nrows=%d cell=%s  %s\n",
                   simple.nrows, simple.ok ? simple.cell[0][0] : "?",
                   mal ? "FALHA" : "ok");

            pg_sess_limpa(&sess);
            pg_buf_limpa(&be);

            /* Parse */
            pg_buf_limpa(&fe);
            pg_put_parse(&fe, "", "SELECT * FROM t WHERE a = $1", 1, &oid);
            off = 0;
            if(!pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn) || tipo != PG_MSG_PARSE)
                mal++;
            pg_buf_limpa(&one);
            if(pg_sess_fe(&sess, tipo, pay, pn, &one) < 0) mal++;
            pg_buf_bytes(&be, one.b, one.n);

            /* Bind $1 = 7 */
            pg_buf_limpa(&fe);
            pg_put_bind_text1(&fe, "", "", "7");
            off = 0;
            if(!pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn) || tipo != PG_MSG_BIND)
                mal++;
            pg_buf_limpa(&one);
            pg_sess_fe(&sess, tipo, pay, pn, &one);
            pg_buf_bytes(&be, one.b, one.n);

            /* Execute */
            pg_buf_limpa(&fe);
            pg_put_execute(&fe, "", 0);
            off = 0;
            if(!pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn) || tipo != PG_MSG_EXECUTE)
                mal++;
            pg_buf_limpa(&one);
            pg_sess_fe(&sess, tipo, pay, pn, &one);
            pg_buf_bytes(&be, one.b, one.n);

            /* Sync → Ready */
            pg_buf_limpa(&fe);
            pg_put_sync(&fe);
            off = 0;
            if(!pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn) || tipo != PG_MSG_SYNC)
                mal++;
            pg_buf_limpa(&one);
            pg_sess_fe(&sess, tipo, pay, pn, &one);
            pg_buf_bytes(&be, one.b, one.n);

            off = 0;
            while(off < be.n){
                if(!pg_read_typed(be.b, be.n, &off, &tipo, &pay, &pn)){ mal++; break; }
                if(tipo == PG_MSG_PARSE_COMPLETE) viu_1 = 1;
                else if(tipo == PG_MSG_BIND_COMPLETE) viu_2 = 1;
                else if(tipo == PG_MSG_ROW_DESC){
                    if(pg_get_i16(pay) != 3) mal++;
                    viu_t = 1;
                }else if(tipo == PG_MSG_DATA_ROW){
                    int L = pg_get_i32(pay + 2);
                    if(n_data == 0 && L > 0 && L < (int)sizeof cell0){
                        memcpy(cell0, pay + 6, (size_t)L); cell0[L] = 0;
                    }
                    n_data++;
                }else if(tipo == PG_MSG_CMD_COMPLETE){
                    if(strncmp((const char *)pay, "SELECT", 6) != 0) mal++;
                    viu_c = 1;
                }else if(tipo == PG_MSG_READY){
                    if(pay[0] != PG_TX_IDLE) mal++;
                    viu_z = 1;
                }else if(tipo == PG_MSG_ERROR) mal++;
            }
            printf("      Ext: 1=%d 2=%d T=%d D=%d C=%d Z=%d cell0=%s  %s\n",
                   viu_1, viu_2, viu_t, n_data, viu_c, viu_z, cell0,
                   mal ? "FALHA" : "ok");
            if(!viu_1 || !viu_2 || !viu_t || n_data != 1 || !viu_c || !viu_z) mal++;
            if(strcmp(cell0, "7") != 0) mal++;
            if(strcmp(cell0, simple.cell[0][0]) != 0) mal++;

            /* portal SQL ligado */
            if(!sess.portal_u.vivo
               || strcmp(sess.portal_u.sql, "SELECT * FROM t WHERE a = 7") != 0)
                mal++;
            printf("      portal sql=%s  %s\n",
                   sess.portal_u.vivo ? sess.portal_u.sql : "?",
                   mal ? "FALHA" : "ok");

            sql_fechar();
        }
        ok("§W5 Parse/Bind($1=7)/Execute/Sync ≡ Simple WHERE a=7", mal == 0);
    }

    /* ═══ §W6 Describe + Close ═══════════════════════════════════════════════ */
    printf("\n§W6 Describe(S) + Close(S) + Sync.\n\n");
    {
        PgSess sess;
        PgBuf fe, be, one;
        int mal = 0, off, viu_t = 0, viu_n = 0, viu_3 = 0, viu_z = 0;
        char tipo;
        const uint8_t *pay;
        int pn;
        int32_t oid = PG_OID_INT4;

        pg_sess_limpa(&sess);
        pg_buf_limpa(&be);

        pg_buf_limpa(&fe);
        pg_put_parse(&fe, "q1", "SELECT * FROM t WHERE a = $1", 1, &oid);
        off = 0; pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn);
        pg_buf_limpa(&one); pg_sess_fe(&sess, tipo, pay, pn, &one);
        pg_buf_bytes(&be, one.b, one.n);

        pg_buf_limpa(&fe);
        pg_put_describe(&fe, 'S', "q1");
        off = 0; pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn);
        pg_buf_limpa(&one); pg_sess_fe(&sess, tipo, pay, pn, &one);
        pg_buf_bytes(&be, one.b, one.n);

        pg_buf_limpa(&fe);
        pg_put_close_fe(&fe, 'S', "q1");
        off = 0; pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn);
        pg_buf_limpa(&one); pg_sess_fe(&sess, tipo, pay, pn, &one);
        pg_buf_bytes(&be, one.b, one.n);

        pg_buf_limpa(&fe);
        pg_put_sync(&fe);
        off = 0; pg_read_typed(fe.b, fe.n, &off, &tipo, &pay, &pn);
        pg_buf_limpa(&one); pg_sess_fe(&sess, tipo, pay, pn, &one);
        pg_buf_bytes(&be, one.b, one.n);

        off = 0;
        while(off < be.n){
            if(!pg_read_typed(be.b, be.n, &off, &tipo, &pay, &pn)){ mal++; break; }
            if(tipo == PG_MSG_PARSE_COMPLETE){ /* ok */ }
            else if(tipo == PG_MSG_PARAM_DESC){
                if(pg_get_i16(pay) != 1 || pg_get_i32(pay + 2) != PG_OID_INT4) mal++;
                viu_t = 1;
            }else if(tipo == PG_MSG_NO_DATA) viu_n = 1;
            else if(tipo == PG_MSG_CLOSE_COMPLETE) viu_3 = 1;
            else if(tipo == PG_MSG_READY){
                if(pay[0] != PG_TX_IDLE) mal++;
                viu_z = 1;
            }else if(tipo == PG_MSG_ERROR) mal++;
        }
        if(sess.stmt_n.vivo) mal++; /* Close deve ter apagado */
        printf("      Describe t=%d n=%d Close=%d Ready=%d vivo_stmt=%d  %s\n",
               viu_t, viu_n, viu_3, viu_z, sess.stmt_n.vivo, mal ? "FALHA" : "ok");
        ok("§W6 Describe(S)→ParamDesc+NoData; Close(S); Sync→Ready",
           mal == 0 && viu_t && viu_n && viu_3 && viu_z);
    }

    /* ═══ §W7 A FACHADA pqlike (Trio PG5) ════════════════════════════════════
     * A porta que uma aplicação C conhece é `PQconnectdb`/`PQexec`, e ela está
     * escrita AQUI (lib/pqlike.h) sobre o FEBE da casa — não se linka `-lpq`.
     *
     * E o que se mede não é «a fachada corre»: é que os DOIS CAMINHOS pelo mesmo
     * objecto concordam. A mesma base, a mesma consulta, por `sql_api` directo e
     * por PQexec através do socket, têm de dar as mesmas linhas, as mesmas células
     * e a mesma tag. Se só se medisse a fachada, ela podia devolver qualquer coisa
     * consistente consigo própria.
     *
     * O gume é o outro lado: uma consulta INVÁLIDA tem de chegar como
     * PGRES_FATAL_ERROR com mensagem, e não como um resultado vazio — um erro que
     * se parece com «zero linhas» é o pior desfecho para quem chama. */
    printf("\n§W7 pqlike: PQconnectdb/PQexec ≡ sql_api, e o erro chega como erro.\n\n");
    {
        const char *base = "/tmp/pgwire_w7";
        SqlOut ref, seed;
        int sv[2] = {-1, -1}, mal = 0;
        pid_t kid = -1;
        char linhas_ref[SQL_OUT_MAX_ROWS][SQL_OUT_MAX_COLS][SQL_OUT_CELL];
        char tag_ref[80];
        int ncols_ref = 0, nrows_ref = 0;

        unlink("/tmp/pgwire_w7.mem");
        unlink("/tmp/pgwire_w7.prog");
        signal(SIGPIPE, SIG_IGN);

        /* (1) o CAMINHO DIRECTO: semeia e guarda a referência */
        if(!sql_abrir(base)) mal++;
        else{
            sql_executa("CREATE TABLE t (a,b,c)", &seed);
            sql_executa("INSERT INTO t VALUES (7,10,20)", &seed);
            sql_executa("INSERT INTO t VALUES (3,30,40)", &seed);
            if(!sql_executa("SELECT * FROM t", &ref) || !ref.ok) mal++;
            ncols_ref = ref.ncols; nrows_ref = ref.nrows;
            snprintf(tag_ref, sizeof tag_ref, "%s", ref.tag);
            memcpy(linhas_ref, ref.cell, sizeof linhas_ref);
            sql_fechar();
        }
        printf("      directo (sql_api): %d linhas x %d colunas, tag=%s\n",
               nrows_ref, ncols_ref, tag_ref);

        if(pipe(sv) < 0) mal++;
        if(!mal) kid = fork();
        if(!mal && kid < 0) mal++;

        if(!mal && kid == 0){
            int lfd, cfd, porto = 0, n = 0;
            close(sv[0]);
            if(!sql_abrir(base)) _exit(3);
            lfd = pgwire_listen(0, &porto);
            if(lfd < 0){ sql_fechar(); _exit(4); }
            {
                uint8_t msg[3];
                msg[0] = (uint8_t)((porto >> 8) & 0xff);
                msg[1] = (uint8_t)(porto & 0xff);
                msg[2] = 'R';
                if(write(sv[1], msg, 3) != 3){ close(lfd); sql_fechar(); _exit(5); }
            }
            /* três ligações: a boa, a das duas tabelas e a do gume */
            for(n = 0; n < 3; n++){
                cfd = accept(lfd, NULL, NULL);
                if(cfd < 0) break;
                pgwire_serve_conn(cfd, 77, 88);
                close(cfd);
            }
            close(lfd); sql_fechar();
            _exit(0);
        }

        if(!mal && kid > 0){
            uint8_t msg[3];
            int porto = 0, st = -1;
            char conninfo[128];
            PGconn *c;
            PGresult *r;

            close(sv[1]); sv[1] = -1;
            if(read(sv[0], msg, 3) != 3 || msg[2] != 'R') mal++;
            else porto = ((int)msg[0] << 8) | (int)msg[1];
            close(sv[0]); sv[0] = -1;
            snprintf(conninfo, sizeof conninfo,
                     "host=127.0.0.1 port=%d user=tiffany dbname=reino", porto);

            /* (2) o CAMINHO DA FACHADA */
            c = PQconnectdb(conninfo);
            if(PQstatus(c) != CONNECTION_OK){
                printf("      PQconnectdb: %s\n", PQerrorMessage(c));
                mal++;
            }else{
                const char *sv_ver = PQparameterStatus(c, "server_version");
                printf("      PQconnectdb ok · servidor=%s · backend pid=%d\n",
                       sv_ver ? sv_ver : "(sem)", PQbackendPID(c));
                if(!sv_ver || !sv_ver[0]) mal++;

                r = PQexec(c, "SELECT * FROM t");
                if(PQresultStatus(r) != PGRES_TUPLES_OK){
                    printf("      PQexec: %s (%s)\n",
                           PQresStatus(PQresultStatus(r)), PQresultErrorMessage(r));
                    mal++;
                }else{
                    int i, j, difs = 0;
                    if(PQntuples(r) != nrows_ref || PQnfields(r) != ncols_ref) difs++;
                    for(i = 0; i < PQntuples(r) && i < nrows_ref; i++)
                        for(j = 0; j < PQnfields(r) && j < ncols_ref; j++)
                            if(strcmp(PQgetvalue(r, i, j), linhas_ref[i][j]) != 0) difs++;
                    if(strcmp(PQcmdStatus(r), tag_ref) != 0) difs++;
                    if(PQresultTruncado(r)) difs++;
                    printf("      pqlike (FEBE):     %d linhas x %d colunas, tag=%s\n",
                           PQntuples(r), PQnfields(r), PQcmdStatus(r));
                    printf("      célula (0,0)=%s (1,0)=%s · divergências entre os"
                           " dois caminhos: %d\n",
                           PQgetvalue(r, 0, 0), PQgetvalue(r, 1, 0), difs);
                    if(difs) mal++;
                }
                PQclear(r);
                PQfinish(c);
            }

            /* (3) DUAS TABELAS PELA MESMA LIGAÇÃO, e a coluna é da tabela do comando.
             *
             * Foi esta sequência que apanhou um defeito que nenhuma asserção via: o
             * `varre` resolvia a coluna do SET ANTES de abrir a tabela do FROM, logo
             * um `UPDATE cliente SET saldo` logo a seguir a um `SELECT * FROM conta`
             * procurava «saldo» na `conta` e devolvia «column does not exist». Nos
             * testes a tabela do UPDATE era sempre a que já estava aberta, e por isso
             * o defeito não vivia lá. A ligação ponta a ponta é que o mostrou. */
            {
                PGconn *c3 = PQconnectdb(conninfo);
                int seq_ok = 0;
                if(PQstatus(c3) == CONNECTION_OK){
                    PGresult *r;
                    int passos = 0;
                    r = PQexec(c3, "CREATE TABLE conta (dono,valor)");
                    if(PQresultStatus(r) == PGRES_COMMAND_OK) passos++;
                    PQclear(r);
                    r = PQexec(c3, "INSERT INTO conta VALUES (1,99)");
                    if(PQresultStatus(r) == PGRES_COMMAND_OK) passos++;
                    PQclear(r);
                    r = PQexec(c3, "SELECT * FROM conta");          /* a conta fica aberta */
                    if(PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1
                       && PQnfields(r) == 2 && !strcmp(PQfname(r, 0), "dono")) passos++;
                    PQclear(r);
                    /* e agora o UPDATE noutra tabela, com a coluna DELA */
                    r = PQexec(c3, "UPDATE t SET c = 111 WHERE a = 7");
                    if(PQresultStatus(r) == PGRES_COMMAND_OK) passos++;
                    PQclear(r);
                    r = PQexec(c3, "SELECT * FROM t WHERE a = 7");
                    if(PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1
                       && !strcmp(PQgetvalue(r, 0, 2), "111")) passos++;
                    PQclear(r);
                    printf("      duas tabelas pela mesma ligação: %d de 5 passos\n", passos);
                    seq_ok = (passos == 5);
                    PQfinish(c3);
                }
                if(!seq_ok) mal++;
            }

            /* (4) O GUME: o erro tem de CHEGAR como erro, e não como zero linhas */
            {
                PGconn *c2 = PQconnectdb(conninfo);
                int erro_ok = 0;
                if(PQstatus(c2) == CONNECTION_OK){
                    PGresult *r2 = PQexec(c2, "SELECT * FROM tabela_que_nao_existe");
                    erro_ok = (PQresultStatus(r2) == PGRES_FATAL_ERROR)
                           && PQresultErrorMessage(r2)[0] != 0
                           && PQntuples(r2) == 0;
                    printf("      consulta inválida → %s  «%s»\n",
                           PQresStatus(PQresultStatus(r2)), PQresultErrorMessage(r2));
                    PQclear(r2);
                    PQfinish(c2);
                }
                if(!erro_ok) mal++;
            }

            waitpid(kid, &st, 0);
            if(!WIFEXITED(st) || WEXITSTATUS(st) != 0){
                printf("      filho saiu %d/%d\n", WIFEXITED(st), WEXITSTATUS(st));
                mal++;
            }
        }
        if(sv[0] >= 0) close(sv[0]);
        if(sv[1] >= 0) close(sv[1]);

        printf("\n");
        ok("§W7 A FACHADA libpq É NOSSA E OS DOIS CAMINHOS CONCORDAM: `PQconnectdb`,"
           " `PQexec`, `PQntuples`, `PQgetvalue` e `PQcmdStatus` escritos em"
           " lib/pqlike.h sobre o FEBE da casa — sem `-lpq` do sistema —, e a mesma"
           " consulta pela porta directa (sql_api) e pelo socket dá as MESMAS linhas,"
           " as mesmas células e a mesma tag. Medir só a fachada não dizia nada: ela"
           " podia ser consistente consigo própria e errada. E o gume é o lado que"
           " tem de falhar — uma tabela que não existe chega como PGRES_FATAL_ERROR"
           " COM mensagem, e não como um SELECT de zero linhas, que é o desfecho que"
           " engana quem chama. E a mesma ligação serve DUAS tabelas — cria a `conta`,"
           " lê-a, e a seguir faz UPDATE na `t` com uma coluna da `t`: foi essa sequência"
           " que mostrou que a coluna era resolvida ANTES de a tabela ser aberta, e"
           " nenhuma asserção o via porque nos testes a tabela do UPDATE era sempre a"
           " que já estava aberta",
           mal == 0 && nrows_ref == 2 && ncols_ref == 3);
    }

    /* ═══ §W8: TRIO PG6 — o catálogo de SESSÃO, e o controlo que o limita ════
     *
     * Um cliente real não começa por pedir dados: pergunta QUEM É o servidor e
     * em que estado está. Essas perguntas não tocam no .mem — são da ligação —,
     * e por isso o catálogo responde ANTES do motor. O que dá conteúdo a esta
     * camada não é ela responder: é ela NÃO responder ao que não é dela.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W8 Trio PG6: catálogo de sessão (version/SHOW/SET) e o motor intacto.\n\n");
    {
        long mal = 0;
        SqlOut o;
        const char *base = "/tmp/pgwire_w8";
        unlink("/tmp/pgwire_w8.mem");
        unlink("/tmp/pgwire_w8.prog");
        if(!sql_abrir(base)){ mal++; printf("      sql_abrir FALHOU\n"); }

        printf("      consulta                      cols linhas  resultado\n");
        #define PG6(q, esp_cols, esp_val) do {                                  \
            int r = sql_executa((q), &o);                                       \
            int bate = r && o.ncols == (esp_cols) &&                            \
                       ((esp_val) == NULL || !strcmp(o.cell[0][0], (esp_val))); \
            printf("      %-29s %-4d %-7d %s\n", (q), o.ncols, o.nrows,         \
                   bate ? (o.nrows ? o.cell[0][0] : o.tag) : "NAO BATE");       \
            if(!bate) mal++;                                                    \
        } while(0)

        PG6("SELECT version()",          1, "Tiffany-pgwire/0.1");
        PG6("SELECT current_schema()",   1, "public");
        PG6("SELECT current_database()", 1, "pgwire_w8");
        PG6("SELECT current_user",       1, "tiffany");
        PG6("SHOW client_encoding",      1, "UTF8");
        PG6("SHOW server_version",       1, "Tiffany-pgwire/0.1");

        /* A IDA GUARDA A VOLTA: o que o SET escreve, o SHOW lê. Um catálogo que
         * aceitasse o SET e não o devolvesse era um sorvedouro, não um catálogo. */
        {
            int r1 = sql_executa("SET application_name = 'tiffany-psql'", &o);
            int tag_set = r1 && !strcmp(o.tag, "SET") && o.ncols == 0;
            int r2 = sql_executa("SHOW application_name", &o);
            int voltou = r2 && o.nrows == 1 && !strcmp(o.cell[0][0], "tiffany-psql");
            printf("\n      SET application_name -> tag \"%s\"; SHOW devolve \"%s\": %s\n",
                   tag_set ? "SET" : "?", voltou ? o.cell[0][0] : "?",
                   (tag_set && voltou) ? "a ida guarda a volta" : "NAO");
            if(!tag_set || !voltou) mal++;
            /* e um parâmetro NOVO, que não estava na tabela inicial */
            sql_executa("SET tiffany.medida = 529", &o);
            r2 = sql_executa("SHOW tiffany.medida", &o);
            if(!r2 || strcmp(o.cell[0][0], "529")) mal++;
            printf("      e um parâmetro novo: SHOW tiffany.medida = \"%s\"\n",
                   r2 ? o.cell[0][0] : "?");
        }

        /* O DESCONHECIDO É ERRO, e não uma linha vazia: quem chama tem de poder
         * distinguir «vale isto» de «não existe». */
        {
            int r = sql_executa("SHOW nao_existe_este", &o);
            int e_erro = !r && o.err[0] && o.nrows == 0;
            printf("      SHOW de parâmetro inexistente -> %s (%s)\n",
                   e_erro ? "erro COM mensagem" : "NAO", o.err[0] ? o.err : "sem mensagem");
            if(!e_erro) mal++;
        }

        /* SHOW ALL: duas colunas, e tantas linhas quantos os parâmetros */
        {
            int r = sql_executa("SHOW ALL", &o);
            int bate = r && o.ncols == 2 && o.nrows >= 8;
            printf("      SHOW ALL -> %d colunas, %d parâmetros: %s\n",
                   o.ncols, o.nrows, bate ? "sim" : "NAO");
            if(!bate) mal++;
        }

        /* as transacções que os drivers mandam antes de tudo — e são DUAS, não
         * três: o §W15 mostrou que o ROLLBACK não desfazia nada, e ele passou a
         * ser recusado. Esta lista teve de encolher por causa disso. */
        {
            const char *tx[2] = { "BEGIN", "COMMIT" };
            int bate = 1;
            for(int i = 0; i < 2; i++){
                int r = sql_executa(tx[i], &o);
                if(!r || strcmp(o.tag, tx[i]) || o.ncols) bate = 0;
            }
            printf("      BEGIN/COMMIT -> tags próprias, sem colunas: %s\n",
                   bate ? "sim" : "NAO");
            /* o ROLLBACK esteve RECUSADO enquanto não desfazia nada; desde que
             * desfaz (§W15) devolve a tag como os outros. Esta linha mudou de
             * sentido duas vezes, e as duas foram o medidor a segui-lo. */
            { int r = sql_executa("ROLLBACK", &o);
              printf("      e o ROLLBACK -> %s (§W15: desfaz pelo levantamento)\n",
                     r ? "tag ROLLBACK" : "RECUSADO (mau)");
              if(!r || strcmp(o.tag, "ROLLBACK")) bate = 0; }
            if(!bate) mal++;
        }

        /* ── O CONTROLO, e é ele que dá conteúdo a tudo o que está acima ──────
         * A fachada corre ANTES do motor. Se ela respondesse a tudo, o motor
         * nunca corria e nenhuma das asserções acima o notaria. Exige-se então
         * que uma consulta NORMAL atravesse: mesma sessão, dados reais. */
        {
            int r1 = sql_executa("CREATE TABLE pg6 (a,b)", &o);
            int r2 = sql_executa("INSERT INTO pg6 VALUES (7,70)", &o);
            int r3 = sql_executa("INSERT INTO pg6 VALUES (9,90)", &o);
            int r4 = sql_executa("SELECT * FROM pg6 WHERE a = 7", &o);
            int passou = r1 && r2 && r3 && r4 && o.nrows == 1 && o.ncols == 2
                         && !strcmp(o.cell[0][0], "7") && !strcmp(o.cell[0][1], "70");
            printf("\n      CONTROLO — o motor continua a correr:"
                   " SELECT * FROM pg6 WHERE a = 7 -> %d linha(s), (%s,%s): %s\n",
                   o.nrows, o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
                   passou ? "passa ao motor" : "NAO PASSOU");
            if(!passou) mal++;
            /* e um SELECT de coluna, que também não é do catálogo */
            int r5 = sql_executa("SELECT * FROM pg6", &o);
            if(!r5 || o.nrows != 2) mal++;
            printf("      e SELECT * FROM pg6 -> %d linhas (a fachada não engoliu)\n", o.nrows);
        }

        /* ── E PELO WIRE, que é o que o cliente vê ──────────────────────────
         * O catálogo tem de atravessar o protocolo, não só a porta directa: o
         * mesmo servidor, o mesmo socket, e a resposta a bater célula a célula. */
        {
            char ref_ver[64] = "", ref_enc[64] = "";
            int sv8[2] = {-1,-1};
            pid_t kid8 = -1;
            signal(SIGPIPE, SIG_IGN);
            if(sql_executa("SELECT version()", &o) && o.nrows == 1)
                snprintf(ref_ver, sizeof ref_ver, "%s", o.cell[0][0]);
            if(sql_executa("SHOW client_encoding", &o) && o.nrows == 1)
                snprintf(ref_enc, sizeof ref_enc, "%s", o.cell[0][0]);
            sql_fechar();

            if(pipe(sv8) < 0) mal++;
            if(!mal) kid8 = fork();
            if(!mal && kid8 < 0) mal++;
            if(!mal && kid8 == 0){
                int lfd, cfd, porto = 0;
                close(sv8[0]);
                if(!sql_abrir(base)) _exit(3);
                lfd = pgwire_listen(0, &porto);
                if(lfd < 0){ sql_fechar(); _exit(4); }
                {
                    uint8_t m[3];
                    m[0] = (uint8_t)((porto >> 8) & 0xff);
                    m[1] = (uint8_t)(porto & 0xff);
                    m[2] = 'R';
                    if(write(sv8[1], m, 3) != 3){ close(lfd); sql_fechar(); _exit(5); }
                }
                cfd = accept(lfd, NULL, NULL);
                if(cfd >= 0){ pgwire_serve_conn(cfd, 77, 88); close(cfd); }
                close(lfd); sql_fechar();
                _exit(0);
            }
            if(!mal && kid8 > 0){
                uint8_t m[3];
                int porto = 0, st = -1, difs = 0;
                char conninfo[128];
                PGconn *c;
                PGresult *rv, *re;
                close(sv8[1]); sv8[1] = -1;
                if(read(sv8[0], m, 3) != 3 || m[2] != 'R') mal++;
                else porto = ((int)m[0] << 8) | (int)m[1];
                close(sv8[0]); sv8[0] = -1;
                snprintf(conninfo, sizeof conninfo,
                         "host=127.0.0.1 port=%d user=tiffany dbname=reino", porto);
                c = PQconnectdb(conninfo);
                if(PQstatus(c) != CONNECTION_OK){
                    printf("      PQconnectdb: %s\n", PQerrorMessage(c)); mal++;
                }else{
                    rv = PQexec(c, "SELECT version()");
                    if(PQresultStatus(rv) != PGRES_TUPLES_OK || PQntuples(rv) != 1
                       || strcmp(PQgetvalue(rv, 0, 0), ref_ver)) difs++;
                    printf("\n      pelo WIRE:  PQexec(\"SELECT version()\")   = \"%s\"\n",
                           PQresultStatus(rv) == PGRES_TUPLES_OK && PQntuples(rv)
                           ? PQgetvalue(rv, 0, 0) : "(nada)");
                    PQclear(rv);
                    re = PQexec(c, "SHOW client_encoding");
                    if(PQresultStatus(re) != PGRES_TUPLES_OK || PQntuples(re) != 1
                       || strcmp(PQgetvalue(re, 0, 0), ref_enc)) difs++;
                    printf("      pelo WIRE:  PQexec(\"SHOW client_encoding\") = \"%s\"\n",
                           PQresultStatus(re) == PGRES_TUPLES_OK && PQntuples(re)
                           ? PQgetvalue(re, 0, 0) : "(nada)");
                    PQclear(re);
                    printf("      pela PORTA: \"%s\" e \"%s\"  ·  divergências: %d\n",
                           ref_ver, ref_enc, difs);
                    if(difs) mal++;
                    PQfinish(c);
                }
                waitpid(kid8, &st, 0);
            }
            if(sv8[0] >= 0) close(sv8[0]);
            if(sv8[1] >= 0) close(sv8[1]);
        }
        #undef PG6

        printf("\n");
        ok("O CATÁLOGO É DA SESSÃO, E O MOTOR FICA INTACTO — que é o Trio PG6. Um cliente"
           " real não começa por pedir dados: pergunta quem é o servidor e em que estado"
           " está, e essas perguntas não tocam no .mem nem na ISA. Por isso o catálogo é uma"
           " FACHADA ANTES do motor, e não mais um comando dentro dele. Respondem"
           " `SELECT version()`, `current_schema()`, `current_user`, `SHOW <par>`,"
           " `SHOW ALL`, `SET`, e as tags BEGIN e COMMIT que os drivers mandam antes de"
           " tudo — e o ROLLBACK, que esteve recusado enquanto não desfazia nada e voltou"
           " à lista quando o §W15 o pôs a desfazer pelo levantamento. A IDA GUARDA A VOLTA: o que o SET escreve o SHOW lê — incluindo um"
           " parâmetro NOVO, que não estava na tabela inicial —, e um catálogo que aceitasse"
           " o SET sem o devolver era um sorvedouro. O DESCONHECIDO É ERRO com mensagem, e"
           " não uma linha vazia: quem chama tem de distinguir «vale isto» de «não existe»."
           " E O CONTROLO É O QUE DÁ CONTEÚDO A ISTO TUDO: a fachada corre ANTES do motor,"
           " logo se respondesse a tudo o motor nunca correria e NENHUMA das asserções"
           " acima o notaria — exige-se por isso que uma consulta normal atravesse, na"
           " MESMA sessão, com CREATE, dois INSERT e um SELECT com WHERE a devolver a linha"
           " certa. Por fim, os DOIS CAMINHOS: a mesma pergunta pelo socket (PQexec sobre o"
           " nosso FEBE) e pela porta directa dá a mesma resposta — o catálogo atravessa o"
           " protocolo, e não vive só do lado de cá.",
           mal == 0);
    }

    /* ═══ §W9: O TIPO ACOMPANHA A COLUNA ═════════════════════════════════════
     *
     * O RowDescription anunciava TUDO como int4 — inclusive `SELECT version()`,
     * que devolve texto. O valor chegava certo porque o formato do wire é texto;
     * o TIPO é que ia errado, e nenhuma comparação de células o via. Um cliente
     * que confiasse no OID converteria "Tiffany-pgwire/0.1" para inteiro.
     *
     * Aqui mede-se o que o servidor DIZ contra o que a coluna É — e o gume é
     * haver DUAS respostas: se todas as colunas dessem o mesmo OID, o teste
     * passaria sem separar nada.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W9 o tipo acompanha a coluna: catálogo é TEXT, motor é INT4.\n\n");
    {
        const char *base = "/tmp/pgwire_w9";
        long mal = 0;
        int sv9[2] = {-1,-1};
        pid_t kid9 = -1;
        unlink("/tmp/pgwire_w9.mem");
        unlink("/tmp/pgwire_w9.prog");
        signal(SIGPIPE, SIG_IGN);

        /* semeia pela porta directa */
        if(!sql_abrir(base)) mal++;
        else{
            SqlOut sd;
            sql_executa("CREATE TABLE w9 (a,b)", &sd);
            sql_executa("INSERT INTO w9 VALUES (7,70)", &sd);
            sql_fechar();
        }

        if(pipe(sv9) < 0) mal++;
        if(!mal) kid9 = fork();
        if(!mal && kid9 < 0) mal++;
        if(!mal && kid9 == 0){
            int lfd, cfd, porto = 0;
            close(sv9[0]);
            if(!sql_abrir(base)) _exit(3);
            lfd = pgwire_listen(0, &porto);
            if(lfd < 0){ sql_fechar(); _exit(4); }
            {
                uint8_t m[3];
                m[0] = (uint8_t)((porto >> 8) & 0xff);
                m[1] = (uint8_t)(porto & 0xff);
                m[2] = 'R';
                if(write(sv9[1], m, 3) != 3){ close(lfd); sql_fechar(); _exit(5); }
            }
            cfd = accept(lfd, NULL, NULL);
            if(cfd >= 0){ pgwire_serve_conn(cfd, 77, 88); close(cfd); }
            close(lfd); sql_fechar();
            _exit(0);
        }
        if(!mal && kid9 > 0){
            uint8_t m[3];
            int porto = 0, st = -1;
            int viu_text = 0, viu_int4 = 0;
            char conninfo[128];
            PGconn *c;
            close(sv9[1]); sv9[1] = -1;
            if(read(sv9[0], m, 3) != 3 || m[2] != 'R') mal++;
            else porto = ((int)m[0] << 8) | (int)m[1];
            close(sv9[0]); sv9[0] = -1;
            snprintf(conninfo, sizeof conninfo,
                     "host=127.0.0.1 port=%d user=tiffany dbname=reino", porto);
            c = PQconnectdb(conninfo);
            if(PQstatus(c) != CONNECTION_OK){ printf("      PQconnectdb: %s\n",
                                                    PQerrorMessage(c)); mal++; }
            else{
                struct { const char *sql; int oid; int len; const char *porque; } casos[] = {
                    { "SELECT version()",      PG_OID_TEXT, -1, "o catálogo devolve texto" },
                    { "SHOW client_encoding",  PG_OID_TEXT, -1, "um parâmetro é texto"     },
                    { "SELECT * FROM w9",      PG_OID_INT4,  4, "o motor guarda inteiros"  },
                };
                printf("      consulta                   coluna  OID   typlen   esperado   %s\n", "");
                for(unsigned k = 0; k < sizeof casos / sizeof casos[0]; k++){
                    PGresult *r = PQexec(c, casos[k].sql);
                    int oid = 0, len = 0, bate = 0;
                    if(PQresultStatus(r) == PGRES_TUPLES_OK && PQnfields(r) > 0){
                        oid = PQftype(r, 0); len = PQfsize(r, 0);
                        bate = (oid == casos[k].oid && len == casos[k].len);
                        if(oid == PG_OID_TEXT) viu_text = 1;
                        if(oid == PG_OID_INT4) viu_int4 = 1;
                        /* e TODAS as colunas, não só a primeira */
                        for(int j = 0; j < PQnfields(r); j++)
                            if(PQftype(r, j) != casos[k].oid
                               || PQfsize(r, j) != casos[k].len) bate = 0;
                    }
                    printf("      %-26s %-7s %-5d %-8d %-10d %s\n",
                           casos[k].sql, PQnfields(r) ? PQfname(r, 0) : "?",
                           oid, len, casos[k].oid, bate ? casos[k].porque : "NAO BATE");
                    if(!bate) mal++;
                    PQclear(r);
                }
                /* O GUME: as duas respostas TÊM de ser diferentes. Se o servidor
                 * anunciasse um OID único, cada linha acima passaria à mesma —
                 * e era exactamente o defeito que estava lá antes. */
                printf("\n      viu TEXT: %s · viu INT4: %s · o wire separa os dois: %s\n",
                       viu_text ? "sim" : "NAO", viu_int4 ? "sim" : "NAO",
                       (viu_text && viu_int4) ? "sim" : "NAO — o teste não separa nada");
                if(!viu_text || !viu_int4) mal++;
                PQfinish(c);
            }
            waitpid(kid9, &st, 0);
        }
        if(sv9[0] >= 0) close(sv9[0]);
        if(sv9[1] >= 0) close(sv9[1]);

        printf("\n");
        ok("O TIPO ACOMPANHA A COLUNA, e antes não acompanhava: o RowDescription anunciava"
           " TUDO como int4 com typlen 4 — inclusive `SELECT version()`, que devolve texto."
           " O valor chegava certo, porque no wire tudo viaja em formato texto, e por isso"
           " NENHUMA comparação de células o apanhava: era o valor certo com o TIPO errado."
           " Um cliente que confiasse no OID — e é para isso que ele lá está — converteria"
           " «Tiffany-pgwire/0.1» para inteiro. Agora o tipo nasce onde a coluna nasce: o"
           " catálogo declara TEXT, o motor declara INT4, e o typlen anda com o OID (−1 para"
           " variável, 4 para int4), porque anunciar 4 num texto é dizer ao cliente que lá"
           " cabem quatro bytes. Mede-se pelo WIRE, com PQftype e PQfsize — que o cliente"
           " antes nem lia, saltava o OID —, em TODAS as colunas e não só na primeira. E O"
           " GUME É AS DUAS RESPOSTAS TEREM DE SER DIFERENTES: exige-se ver TEXT e ver INT4"
           " na mesma corrida, porque um servidor que anunciasse um OID único passaria em"
           " cada linha isolada — que é precisamente o defeito que aqui estava.",
           mal == 0);
    }

    /* ═══ §W10: A LISTA DE TABELAS, E A FRONTEIRA DECLARADA ══════════════════
     *
     * O `\dt` do psql não pergunta «que tabelas há»: manda uma consulta com JOIN
     * sobre pg_class e pg_namespace, um CASE de oito ramos, duas funções de
     * catálogo e um operador de expressão regular. Aqui RECONHECE-SE a consulta
     * pela assinatura e responde-se com as tabelas do disco — é compatibilidade,
     * não é um pg_catalog, e a asserção diz isso.
     *
     * O GUME é a fronteira: uma consulta a pg_catalog que não seja servida tem de
     * ser RECUSADA. Um reconhecedor demasiado largo responderia a qualquer coisa
     * com «pg_» dentro, e responder ao acaso é pior do que recusar. A fronteira
     * MOVE-SE quando o catálogo cresce — o §W11 trouxe pg_type para dentro, e
     * esta lista teve de ser corrigida por causa disso: foi o medidor a apanhar
     * a contradição entre dois blocos, que é para o que ele serve.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W10 a lista de tabelas do \\dt, e o que fica de fora.\n\n");
    {
        const char *base = "/tmp/pgwire_w10";
        SqlOut o;
        long mal = 0;
        const char *q_dt =
            "SELECT n.nspname as \"Schema\", c.relname as \"Name\", "
            "CASE c.relkind WHEN 'r' THEN 'table' END as \"Type\" "
            "FROM pg_catalog.pg_class c LEFT JOIN pg_catalog.pg_namespace n "
            "ON n.oid = c.relnamespace WHERE c.relkind IN ('r','p','') ORDER BY 1,2;";

        unlink("/tmp/pgwire_w10.mem");
        unlink("/tmp/pgwire_w10.prog");
        unlink("/tmp/pgwire_w10__alfa.mem");
        unlink("/tmp/pgwire_w10__beta.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE beta (x,y)", &o);
        sql_executa("CREATE TABLE alfa (a,b)", &o);

        {
            int r = sql_executa(q_dt, &o);
            int bate = r && o.ncols == 4 && o.nrows == 2
                       && !strcmp(o.cell[0][1], "alfa")     /* ordenado por nome */
                       && !strcmp(o.cell[1][1], "beta")
                       && !strcmp(o.cell[0][0], "public")
                       && !strcmp(o.cell[0][2], "table");
            printf("      a consulta do \\dt -> %d colunas, %d linhas: %s, %s\n",
                   o.ncols, o.nrows,
                   o.nrows > 0 ? o.cell[0][1] : "?", o.nrows > 1 ? o.cell[1][1] : "?");
            printf("      e vêm ORDENADAS por nome (a consulta pede ORDER BY 1,2): %s\n",
                   bate ? "sim" : "NAO");
            if(!bate) mal++;
            /* e as colunas são TEXTO, não int4 */
            for(int j = 0; j < o.ncols; j++)
                if(o.tipo[j] != SQL_TIPO_TEXT) mal++;
        }

        /* ── O GUME: a fronteira. Estas são de pg_catalog e NÃO são a assinatura;
         * têm de ser recusadas, e não respondidas ao acaso. */
        {
            /* A FRONTEIRA MUDOU, e o medidor apanhou-o: o §W11 passou a servir
             * pg_type como tabela, e estas duas linhas — que aqui exigiam recusa
             * — passaram a responder. Ficam de fora do catálogo o que continua
             * fora dele: as junções, as funções e as tabelas não servidas. */
            const char *fora[] = {
                "SELECT pg_catalog.pg_get_userbyid(10)",
                "SELECT * FROM pg_catalog.pg_attribute",
                "SELECT a.attname FROM pg_attribute a JOIN pg_class c ON 1=1",
            };
            int recusadas = 0;
            printf("\n      a fronteira — o que continua fora tem de ser recusado:\n");
            for(unsigned k = 0; k < sizeof fora / sizeof fora[0]; k++){
                int r = sql_executa(fora[k], &o);
                printf("        %-52s %s\n", fora[k], r ? "RESPONDEU (mau)" : "recusado");
                if(!r) recusadas++;
            }
            if(recusadas != 3) mal++;
            printf("      recusadas %d de 3 — responder ao acaso é pior que recusar\n",
                   recusadas);
        }

        /* e o motor continua intacto, como sempre */
        {
            sql_executa("INSERT INTO alfa VALUES (5,50)", &o);
            int r = sql_executa("SELECT * FROM alfa", &o);
            int ok_motor = r && o.nrows == 1 && o.tipo[0] == SQL_TIPO_INT4;
            printf("      e o motor intacto: SELECT * FROM alfa -> %d linha, tipo int4: %s\n",
                   o.nrows, ok_motor ? "sim" : "NAO");
            if(!ok_motor) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O `\\dt` RESPONDE, E A FRONTEIRA ESTÁ DECLARADA. O psql não pergunta «que tabelas"
           " há»: manda uma consulta com JOIN sobre pg_class e pg_namespace, um CASE de oito"
           " ramos, duas funções de catálogo e um operador de expressão regular — e nem o"
           " pg_catalog nem as junções existem aqui. O que se faz é RECONHECER a consulta"
           " pela assinatura e responder com as tabelas que o disco tem, ordenadas por nome"
           " porque é isso que o ORDER BY 1,2 pede, e com as quatro colunas em TEXTO. Isto é"
           " uma camada de compatibilidade, não um catálogo, e vale mais dizê-lo do que"
           " deixar parecer o contrário. O GUME É A FRONTEIRA: três consultas a pg_catalog"
           " que NÃO são a assinatura — pg_type, um SELECT com WHERE sobre typname, e uma"
           " função de catálogo — têm de ser RECUSADAS, e são as três. Um reconhecedor"
           " demasiado largo responderia a qualquer coisa com «pg_» dentro, e responder ao"
           " acaso é pior do que recusar: quem chama sabe lidar com um erro, não sabe lidar"
           " com uma resposta inventada. A FRONTEIRA MOVE-SE quando o catálogo cresce: o"
           " §W11 trouxe o pg_type para dentro, e as duas linhas que aqui exigiam recusá-lo"
           " passaram a falhar — foi este medidor a apanhar a contradição entre dois blocos"
           " seus, que é para o que ele serve. Ficam de fora as funções de catálogo, as"
           " tabelas não servidas e as junções. E o motor continua intacto ao lado, com o"
           " seu int4.",
           mal == 0);
    }

    /* ═══ §W11: pg_type COMO TABELA, e as duas maneiras de não responder ═════
     *
     * O \dt resolveu-se por reconhecimento de assinatura, e ficou dito que era
     * isso. Aqui faz-se o contrário, porque se pode: pg_type É uma tabela, e o
     * que faltava era um SELECT que a soubesse ler. Escreveu-se esse SELECT —
     * colunas, FROM, e um WHERE de uma igualdade. Mais nada.
     *
     * E O QUE SE MEDE É A DIFERENÇA ENTRE AS DUAS MANEIRAS DE NÃO RESPONDER:
     *   · ZERO LINHAS  — a consulta é válida e não há resultado
     *   · RECUSA       — a consulta não cabe no que sabemos servir
     * Confundi-las é o defeito que este próprio ficheiro persegue desde o §W7:
     * um erro disfarçado de resultado vazio é o pior desfecho para quem chama.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W11 pg_type como tabela: o que se serve, e as duas não-respostas.\n\n");
    {
        SqlOut o;
        long mal = 0;

        printf("      consulta                                          resultado\n");
        /* (a) o que SE SERVE */
        {
            struct { const char *q; int nc; int nl; const char *c00; } bons[] = {
                /* dez tipos: os nove base e o `vector` que o §W12 acrescentou.
                 * Este número teve de ser corrigido quando o vector entrou —
                 * foi o medidor a apanhar a mudança, e é para isso que ele
                 * conta linhas em vez de dizer «devolveu alguma coisa». */
                { "SELECT oid, typname FROM pg_type",                       2, 10, "16"   },
                { "SELECT oid FROM pg_type WHERE typname = 'int4'",          1, 1, "23"   },
                { "SELECT typname, typlen FROM pg_catalog.pg_type WHERE oid = 25", 2, 1, "text" },
                { "SELECT * FROM pg_type WHERE typname = 'bool'",            6, 1, "16"   },
            };
            for(unsigned k = 0; k < sizeof bons / sizeof bons[0]; k++){
                int r = sql_executa(bons[k].q, &o);
                int bate = r && o.ncols == bons[k].nc && o.nrows == bons[k].nl
                           && (bons[k].nl == 0 || !strcmp(o.cell[0][0], bons[k].c00));
                printf("      %-49s %d col x %d lin  %s\n", bons[k].q, o.ncols, o.nrows,
                       bate ? "ok" : "NAO BATE");
                if(!bate) mal++;
            }
        }

        /* (b) ZERO LINHAS: a consulta é válida, o tipo é que não existe */
        {
            int r = sql_executa("SELECT typname FROM pg_type WHERE typname = 'nao_ha'", &o);
            int bate = r && o.ok && o.ncols == 1 && o.nrows == 0 && !o.err[0];
            printf("\n      WHERE de um tipo que não existe -> %s (ok=%d, %d linhas, sem erro)\n",
                   bate ? "ZERO LINHAS, com sucesso" : "NAO", o.ok, o.nrows);
            if(!bate) mal++;
        }

        /* (c) RECUSA: a consulta não cabe no que sabemos — e tem de trazer MENSAGEM */
        {
            const char *fora[] = {
                "SELECT typrelid FROM pg_type",                    /* coluna que não servimos */
                "SELECT typname FROM pg_type ORDER BY oid",        /* não há ORDER BY        */
                "SELECT typname FROM pg_type WHERE typlen > 4",    /* só igualdade no WHERE  */
                "SELECT typname FROM pg_type WHERE zzz = 1",       /* coluna no WHERE        */
            };
            int recusadas = 0, com_msg = 0;
            printf("\n      o que NÃO cabe tem de ser recusado COM mensagem:\n");
            for(unsigned k = 0; k < sizeof fora / sizeof fora[0]; k++){
                int r = sql_executa(fora[k], &o);
                if(!r) recusadas++;
                if(!r && o.err[0]) com_msg++;
                printf("        %-46s %s%s\n", fora[k],
                       r ? "RESPONDEU (mau)" : "recusado",
                       (!r && o.err[0]) ? ", com mensagem" : (!r ? ", SEM mensagem" : ""));
            }
            if(recusadas != 4 || com_msg != 4) mal++;
        }

        /* (d) e o TIPO das colunas do próprio catálogo: oid e typlen são int4,
         * typname é text. Um catálogo de tipos que mentisse no seu próprio tipo
         * seria a piada completa. */
        {
            int r = sql_executa("SELECT oid, typname, typlen FROM pg_type", &o);
            int bate = r && o.ncols == 3
                       && o.tipo[0] == SQL_TIPO_INT4
                       && o.tipo[1] == SQL_TIPO_TEXT
                       && o.tipo[2] == SQL_TIPO_INT4;
            printf("\n      e o tipo das colunas do catálogo: oid=%d typname=%d typlen=%d -> %s\n",
                   o.tipo[0], o.tipo[1], o.tipo[2], bate ? "int4/text/int4" : "NAO BATE");
            if(!bate) mal++;
        }

        /* (e) O CONTROLO: os OIDs que o servidor ANUNCIA têm de estar na tabela.
         * Se o RowDescription usasse um número que o pg_type não conhece, o
         * cliente ficava com um tipo que não sabe resolver. */
        {
            int achou_int4 = 0, achou_text = 0;
            sql_executa("SELECT oid FROM pg_type WHERE typname = 'int4'", &o);
            achou_int4 = (o.nrows == 1 && atoi(o.cell[0][0]) == SQL_TIPO_INT4);
            sql_executa("SELECT oid FROM pg_type WHERE typname = 'text'", &o);
            achou_text = (o.nrows == 1 && atoi(o.cell[0][0]) == SQL_TIPO_TEXT);
            printf("      CONTROLO — os OIDs anunciados estão no catálogo:"
                   " int4=%s text=%s\n",
                   achou_int4 ? "sim" : "NAO", achou_text ? "sim" : "NAO");
            if(!achou_int4 || !achou_text) mal++;
        }

        printf("\n");
        ok("pg_type É UMA TABELA, E LÊ-SE COMO TABELA. O `\\dt` resolveu-se por"
           " reconhecimento de assinatura e ficou dito que era isso; aqui fez-se o"
           " contrário, porque se pode: pg_type tem linhas e colunas, e o que faltava era"
           " um SELECT que a soubesse ler — colunas, FROM, e um WHERE de uma igualdade,"
           " mais nada. Servem-se dez tipos, e são os que este servidor sabe ANUNCIAR:"
           " pôr aqui a lista longa do Postgres seria descrever um servidor que não somos."
           " O QUE SE MEDE É A DIFERENÇA ENTRE AS DUAS MANEIRAS DE NÃO RESPONDER, e elas"
           " não são a mesma: um WHERE sobre um tipo inexistente dá ZERO LINHAS COM"
           " SUCESSO — a consulta era válida —, ao passo que uma coluna que não servimos,"
           " um ORDER BY ou uma desigualdade são RECUSADOS COM MENSAGEM. Confundi-las é o"
           " defeito que este ficheiro persegue desde o §W7: um erro disfarçado de"
           " resultado vazio é o pior desfecho para quem chama. À primeira escrita eram"
           " confundidas — a consulta caía no motor e voltava «zero colunas, zero linhas»"
           " com sucesso — e foi preciso fazer com que, a partir do momento em que a"
           " tabela é pg_type, a falha seja NOSSA e traga mensagem. E as colunas do"
           " catálogo declaram o seu próprio tipo (oid e typlen int4, typname text), com o"
           " CONTROLO a exigir que os OIDs que o RowDescription anuncia estejam na tabela"
           " — senão o cliente receberia um tipo que o catálogo não sabe resolver.",
           mal == 0);
    }

    /* ═══ §W12: O TIPO vector, E OS SCRIPTS COMO FUNÇÕES ═════════════════════
     *
     * Duas coisas, e a segunda tem uma faca dentro.
     *
     * O `vector` do pgvector não tem OID fixo — é uma extensão, e o número sai
     * do catálogo da instância. O cliente descobre-o perguntando ao pg_type, e
     * é por isso que este servidor tem de o servir.
     *
     * E as FUNÇÕES: umas calculam aqui (Walsh, exacto, ±1 e sem vírgula); outras
     * CORREM UM SCRIPT DO REPOSITÓRIO. Executar é executar, e é aí que se mede a
     * sério: a lista é branca e fixa em compilação, e o argumento que não passa
     * o filtro RECUSA a chamada — não é saneado em silêncio, porque sanear em
     * silêncio deixa o chamador a pensar que correu o que pediu.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W12 o tipo vector, e os scripts do repositório como funções.\n\n");
    {
        SqlOut o;
        long mal = 0;

        /* (a) o vector no catálogo, para o cliente o descobrir */
        {
            int r = sql_executa("SELECT oid, typlen FROM pg_type WHERE typname = 'vector'", &o);
            int bate = r && o.nrows == 1 && atoi(o.cell[0][0]) == PG_OID_VECTOR
                       && atoi(o.cell[0][1]) == -1;
            printf("      pg_type WHERE typname='vector' -> oid=%s typlen=%s  %s\n",
                   o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
                   bate ? "(o cliente pode descobri-lo)" : "NAO BATE");
            if(!bate) mal++;
        }

        /* (b) as internas, e o Walsh tem de estar CERTO — não basta devolver
         * qualquer coisa entre parênteses rectos. Compara-se com a definição. */
        {
            int r = sql_executa("SELECT tiffany_walsh(3, 4)", &o);
            int bate = r && o.nrows == 1 && o.tipo[0] == SQL_TIPO_TEXT;
            int difs = 0;
            if(bate){
                /* chi_k(j) = (−1)^paridade(k AND j), recalculado aqui, à parte */
                const char *p = o.cell[0][0];
                for(int j = 0; j < 16; j++){
                    int par = 0, v = 3 & j, esperado;
                    while(v){ par ^= v & 1; v >>= 1; }
                    esperado = par ? -1 : 1;
                    while(*p && *p != '-' && !isdigit((unsigned char)*p)) p++;
                    { int lido = atoi(p);
                      if(lido != esperado) difs++;
                      if(*p == '-') p++;
                      while(isdigit((unsigned char)*p)) p++; }
                }
            }
            printf("      tiffany_walsh(3,4) = %s\n", o.nrows ? o.cell[0][0] : "?");
            printf("      e bate com (−1)^paridade(k∧j) nos 16 pontos: %s\n",
                   (bate && !difs) ? "sim" : "NAO");
            if(!bate || difs) mal++;
        }

        /* (c) pg_proc: o cliente tem de poder DESCOBRIR o que há. Uma função que
         * só quem a escreveu conhece não é uma interface. */
        {
            int r = sql_executa("SELECT proname, pronargs FROM pg_proc", &o);
            int bate = r && o.ncols == 4 && o.nrows >= 6;
            printf("\n      pg_proc -> %d funções anunciadas: %s\n", o.nrows,
                   bate ? "sim" : "NAO");
            if(!bate) mal++;
        }

        /* (d) UM SCRIPT DO REPOSITÓRIO — e as DUAS situações são legítimas.
         *
         * O script tem um caminho relativo à raiz do repositório, e um servidor
         * arranca de onde o arrancarem. Isto apareceu MEDIDO: este bloco passava
         * corrido da raiz e falhava dentro da bateria, que corre de outro sítio.
         * A resposta certa não é forçar o caminho — é exigir o comportamento
         * honesto em cada caso: com repositório à vista, o script CORRE; sem
         * ele, a chamada é RECUSADA COM MENSAGEM. O que não pode acontecer é
         * devolver uma linha vazia com sucesso. */
        {
            int r = sql_executa("SELECT tiffany_painel()", &o);
            int correu  = r && o.nrows == 1 && o.cell[0][0][0];
            int recusou = !r && o.err[0];
            printf("      tiffany_painel() -> %s%.48s%s  %s\n",
                   correu ? "\"" : "", correu ? o.cell[0][0] : (o.err[0] ? o.err : "(nada)"),
                   correu ? "\"" : "",
                   correu ? "(o script correu)"
                          : (recusou ? "(sem repositório à vista: recusado com mensagem)"
                                     : "NAO — nem correu nem recusou"));
            if(!correu && !recusou) mal++;
        }

        /* ── O GUME, e é o que decide se isto pode existir ────────────────────
         * Expor scripts por SQL é abrir uma porta. As quatro linhas abaixo são
         * as maneiras de a forçar, e as quatro têm de bater na madeira. */
        {
            const char *ataques[] = {
                "SELECT tiffany_painel('; rm -rf /tmp/alvo')",
                "SELECT tiffany_campo('$(whoami)')",
                "SELECT tiffany_campo('a b')",
                "SELECT tiffany_painel('`id`')",
                "SELECT tiffany_walsh(1)",
                "SELECT rm()",
            };
            int recusados = 0, com_msg = 0;
            printf("\n      o gume — forçar a porta:\n");
            for(unsigned k = 0; k < sizeof ataques / sizeof ataques[0]; k++){
                int r = sql_executa(ataques[k], &o);
                if(!r) recusados++;
                if(!r && o.err[0]) com_msg++;
                printf("        %-45s %s\n", ataques[k],
                       r ? "PASSOU (MAU)" : "recusado");
            }
            printf("      recusados %d de 6, com mensagem %d\n", recusados, com_msg);
            if(recusados != 6) mal++;
            /* e o alvo do primeiro ataque tem de continuar lá */
            {
                FILE *fp = fopen("/tmp/alvo_w12", "w");
                if(fp){ fputs("intacto\n", fp); fclose(fp); }
                sql_executa("SELECT tiffany_painel('; rm -f /tmp/alvo_w12')", &o);
                fp = fopen("/tmp/alvo_w12", "r");
                printf("      e o ficheiro que o ataque queria apagar: %s\n",
                       fp ? "INTACTO" : "APAGADO (MAU)");
                if(!fp) mal++; else fclose(fp);
                unlink("/tmp/alvo_w12");
            }
        }

        /* (e) o CONTROLO: a lista é branca, logo um script do repo que NÃO
         * esteja nela não corre — mesmo existindo e sendo inofensivo. */
        {
            int r = sql_executa("SELECT tiffany_semear()", &o);
            printf("\n      CONTROLO — `tools/semear.sh` existe no repo mas não está na"
                   " lista: %s\n", r ? "CORREU (mau)" : "não corre");
            if(r) mal++;
        }

        printf("\n");
        ok("O vector DECLARA-SE E AS FUNÇÕES CORREM — COM A PORTA FECHADA. O `vector` do"
           " pgvector não tem OID fixo, porque é uma extensão e o número sai do catálogo da"
           " instância: o cliente descobre-o perguntando ao pg_type, e por isso este"
           " servidor serve-o, com typlen −1. Declara-se o tipo; NÃO se finge que o motor o"
           " guarda, porque o motor guarda inteiros. As FUNÇÕES são de duas espécies. As"
           " internas calculam aqui, e mede-se que estão CERTAS e não apenas que devolvem"
           " algo entre parênteses rectos: o caractere de Walsh é recalculado à parte, pela"
           " definição (−1)^paridade(k∧j), e comparado nos dezasseis pontos. As outras"
           " CORREM UM SCRIPT DO REPOSITÓRIO, e é aí que está a faca: executar é executar."
           " A lista é BRANCA e fixa em compilação, os argumentos passam por um filtro de"
           " [A-Za-z0-9_.-], e o que não passa RECUSA a chamada — não é saneado em"
           " silêncio, porque sanear em silêncio deixa quem chamou a pensar que correu o"
           " que pediu. Mede-se a forçar a porta de seis maneiras: ponto e vírgula com rm,"
           " substituição de comando com $() e com crases, espaço no argumento, aridade"
           " errada, e um nome fora da lista — as seis batem na madeira, e um ficheiro"
           " posto como isco continua INTACTO depois do ataque. E o CONTROLO fecha o"
           " raciocínio: um script que EXISTE no repositório e é inofensivo também não"
           " corre, porque não está na lista — é a lista que decide, não o acaso. E HÁ UM"
           " SEGUNDO ACHADO, que veio da bateria e não deste ficheiro: o script tem caminho"
           " relativo à raiz do repositório, e um servidor arranca de onde o arrancarem —"
           " este bloco passava corrido da raiz e FALHAVA dentro da bateria, que corre"
           " noutro sítio. A resposta não foi forçar o caminho: a raiz declara-se em"
           " TIFFANY_RAIZ ou procura-se para cima, e quando não há repositório à vista a"
           " chamada é RECUSADA COM MENSAGEM em vez de devolver vazio. As duas situações"
           " são legítimas e as duas se medem; o que não pode acontecer é a terceira.",
           mal == 0);
    }

    /* ═══ §W13: O `\d <tabela>`, QUE SÃO CINCO CONSULTAS ═════════════════════
     *
     * O psql não pergunta «quais são as colunas». Faz uma sequência: acha o oid
     * pelo nome, pede as propriedades da relação, pede os atributos por oid, e
     * depois pergunta por herança, índices, restrições, regras e gatilhos.
     *
     * E há aqui DUAS coisas que se medem e não são a mesma:
     *   · o oid tem de ser ESTÁVEL — a segunda consulta usa o que a primeira
     *     deu, e um oid que mudasse entre as duas partia a sequência
     *   · as tabelas de catálogo vazias devolvem ZERO LINHAS, não recusa: aqui
     *     SABE-SE que não há herança nem índices. Recusar faria o psql tratar
     *     uma tabela bem descrita como erro.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W13 o \\d <tabela>: o oid estável, as colunas do motor, e o vazio.\n\n");
    {
        const char *base = "/tmp/pgwire_w13";
        SqlOut o;
        long mal = 0;
        int oid1 = 0, oid2 = 0;
        char q[512];

        unlink("/tmp/pgwire_w13.mem");
        unlink("/tmp/pgwire_w13.prog");
        unlink("/tmp/pgwire_w13__conta.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE conta (numero,titular,saldo)", &o);

        /* (1) achar o oid pelo nome — e ele tem de ser ESTÁVEL */
        snprintf(q, sizeof q,
                 "SELECT c.oid, n.nspname, c.relname FROM pg_catalog.pg_class c "
                 "LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
                 "WHERE c.relname OPERATOR(pg_catalog.~) '^(conta)$' ORDER BY 2, 3;");
        {
            int r = sql_executa(q, &o);
            int bate = r && o.nrows == 1 && !strcmp(o.cell[0][2], "conta")
                       && o.tipo[0] == SQL_TIPO_INT4;
            oid1 = o.nrows ? atoi(o.cell[0][0]) : 0;
            r = sql_executa(q, &o);
            oid2 = (r && o.nrows) ? atoi(o.cell[0][0]) : 0;
            printf("      1) o oid de «conta»: %d, e outra vez: %d  -> %s\n",
                   oid1, oid2, (oid1 && oid1 == oid2) ? "ESTÁVEL" : "MUDOU (mau)");
            if(!bate || !oid1 || oid1 != oid2) mal++;
        }

        /* e uma tabela que NÃO existe dá zero linhas, sem erro */
        {
            snprintf(q, sizeof q,
                     "SELECT c.oid, c.relname FROM pg_catalog.pg_class c "
                     "WHERE c.relname OPERATOR(pg_catalog.~) '^(nao_ha_esta)$';");
            int r = sql_executa(q, &o);
            int bate = r && o.ok && o.nrows == 0 && !o.err[0];
            printf("      e uma tabela que não existe -> %d linhas, sem erro: %s\n",
                   o.nrows, bate ? "sim" : "NAO");
            if(!bate) mal++;
        }

        /* (2) as propriedades da relação: TREZE colunas, lidas por POSIÇÃO */
        {
            snprintf(q, sizeof q,
                     "SELECT c.relchecks, c.relkind, c.relhasindex, c.relhasrules, "
                     "c.relhastriggers, false, false, c.relhasoids, false as relispartition, "
                     "'', c.reltablespace, '', c.relpersistence "
                     "FROM pg_catalog.pg_class c WHERE c.oid = '%d';", oid1);
            int r = sql_executa(q, &o);
            int bate = r && o.nrows == 1 && o.ncols == 13 && !strcmp(o.cell[0][1], "r");
            printf("      2) propriedades da relação -> %d colunas (relkind=%s): %s\n",
                   o.ncols, o.nrows ? o.cell[0][1] : "?", bate ? "sim" : "NAO");
            if(!bate) mal++;
            /* e treze não cabiam em oito: o limite subiu por causa disto */
            if(o.ncols > SQL_OUT_MAX_COLS) mal++;
        }

        /* (3) os ATRIBUTOS, e eles vêm do MOTOR — não são inventados */
        {
            snprintf(q, sizeof q,
                     "SELECT a.attname, pg_catalog.format_type(a.atttypid, a.atttypmod) "
                     "FROM pg_catalog.pg_attribute a WHERE a.attrelid = '%d' "
                     "AND a.attnum > 0 ORDER BY a.attnum;", oid1);
            int r = sql_executa(q, &o);
            int bate = r && o.nrows == 3
                       && !strcmp(o.cell[0][0], "numero")
                       && !strcmp(o.cell[1][0], "titular")
                       && !strcmp(o.cell[2][0], "saldo")
                       && !strcmp(o.cell[0][1], "integer");
            printf("      3) as colunas -> %d: %s, %s, %s  (tipo %s)  %s\n", o.nrows,
                   o.nrows > 0 ? o.cell[0][0] : "?", o.nrows > 1 ? o.cell[1][0] : "?",
                   o.nrows > 2 ? o.cell[2][0] : "?", o.nrows ? o.cell[0][1] : "?",
                   bate ? "(vêm do motor)" : "NAO BATEM");
            if(!bate) mal++;
        }

        /* O CONTROLO das colunas: outra tabela, outras colunas. Se viessem de um
         * sítio fixo, as duas dariam o mesmo — e a asserção de cima passaria à
         * mesma. */
        {
            unlink("/tmp/pgwire_w13__par.mem");
            sql_executa("CREATE TABLE par (x,y)", &o);
            snprintf(q, sizeof q,
                     "SELECT a.attname FROM pg_catalog.pg_attribute a "
                     "WHERE a.attrelid = '%d' AND a.attnum > 0;", pgcat_oid_de("par"));
            int r = sql_executa(q, &o);
            int bate = r && o.nrows == 2 && !strcmp(o.cell[0][0], "x")
                       && !strcmp(o.cell[1][0], "y");
            printf("      CONTROLO — outra tabela, outras colunas: %d (%s,%s) %s\n",
                   o.nrows, o.nrows ? o.cell[0][0] : "?", o.nrows > 1 ? o.cell[1][0] : "?",
                   bate ? "sim" : "NAO");
            if(!bate) mal++;
        }

        /* (4) e a sessão NÃO pode ficar mexida: o catálogo abriu outra tabela
         * para ler as colunas, e tem de a repor. */
        {
            sql_executa("INSERT INTO conta VALUES (1,10,100)", &o);
            snprintf(q, sizeof q,
                     "SELECT a.attname FROM pg_catalog.pg_attribute a "
                     "WHERE a.attrelid = '%d' AND a.attnum > 0;", pgcat_oid_de("par"));
            sql_executa(q, &o);                       /* isto abre a «par» */
            int r = sql_executa("SELECT * FROM conta", &o);
            int bate = r && o.nrows == 1 && o.ncols == 3;
            printf("      4) depois de o catálogo ler outra tabela, a sessão continua"
                   " na «conta»: %s\n", bate ? "sim" : "NAO — a sessão foi mexida");
            if(!bate) mal++;
        }

        /* (5) as tabelas de catálogo VAZIAS: zero linhas, e não recusa */
        {
            const char *vazias[] = {
                "SELECT c.oid FROM pg_catalog.pg_inherits i WHERE i.inhrelid = '1'",
                "SELECT i.indexrelid FROM pg_catalog.pg_index i WHERE i.indrelid = '1'",
                "SELECT r.rulename FROM pg_catalog.pg_rewrite r",
                "SELECT t.tgname FROM pg_catalog.pg_trigger t",
            };
            int zeros = 0;
            printf("\n      5) o catálogo vazio — zero linhas, e NÃO recusa:\n");
            for(unsigned k = 0; k < sizeof vazias / sizeof vazias[0]; k++){
                int r = sql_executa(vazias[k], &o);
                int zero = r && o.ok && o.nrows == 0 && !o.err[0];
                if(zero) zeros++;
                printf("        %-52s %s\n", vazias[k],
                       zero ? "0 linhas, com sucesso" : (r ? "linhas a mais" : "RECUSOU (mau)"));
            }
            if(zeros != 4) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O `\\d <tabela>` RESPONDE, E SÃO CINCO CONSULTAS. O psql não pergunta «quais são"
           " as colunas»: acha o oid pelo nome, pede as propriedades da relação, pede os"
           " atributos por oid, e depois pergunta por herança, índices, regras e gatilhos."
           " Três coisas se medem aqui, e nenhuma é a mesma. PRIMEIRA, o oid tem de ser"
           " ESTÁVEL, porque a segunda consulta usa o que a primeira deu — sai do NOME por"
           " hash, e não de um contador, e mede-se pedindo duas vezes. SEGUNDA, as colunas"
           " vêm do MOTOR e não são inventadas: o catálogo abre a tabela, lê os nomes"
           " guardados no .mem e RESTAURA a que estava aberta, porque uma consulta de"
           " catálogo não pode mudar a sessão por baixo de quem a fez — e isso mede-se a"
           " seguir, inserindo e relendo. O CONTROLO é outra tabela com outras colunas: se"
           " os nomes viessem de um sítio fixo, as duas dariam o mesmo e a asserção passava"
           " à mesma. TERCEIRA, as tabelas de catálogo vazias devolvem ZERO LINHAS e não"
           " recusa — aqui SABE-SE que não há herança nem índices, e recusar faria o psql"
           " tratar uma tabela bem descrita como um erro. E houve um limite a cair pelo"
           " caminho: as propriedades da relação são TREZE colunas lidas por posição, e o"
           " SqlOut tinha oito — bastava para as tabelas desta casa e para tudo o que os"
           " medidores pediam, e caiu no primeiro cliente real.",
           mal == 0);
    }

    /* ═══ §W14: count(*) E O QUE ELE CONTA ALÉM DO QUE CABE ══════════════════
     *
     * Uma sondagem com o psql mostrou o que passava e o que não: CREATE,
     * INSERT, UPDATE, DELETE e SELECT atravessavam o wire com as tags certas, e
     * caíam duas coisas — `count(*)` e `\l`.
     *
     * O count é o mais interessante, porque o motor JÁ conta: o S_CONTA guarda
     * quantas linhas casaram com o WHERE, e não satura como o SqlOut, que só
     * materializa as primeiras. Contar é devolver o que a varredura já sabe —
     * não é uma segunda passagem.
     *
     * E É AÍ QUE ESTÁ O GUME: com mais linhas do que cabem no SqlOut, o SELECT
     * trunca e o count NÃO PODE truncar. Se o count fosse o nrows do SqlOut,
     * passaria em toda a tabela pequena e mentiria na primeira grande.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W14 count(*): o que o motor já conta, e o que o SqlOut não comporta.\n\n");
    {
        const char *base = "/tmp/pgwire_w14";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w14.mem");
        unlink("/tmp/pgwire_w14.prog");
        unlink("/tmp/pgwire_w14__g.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE g (a,b)", &o);

        /* (a) o básico, e a bater com o SELECT */
        {
            for(int i = 1; i <= 5; i++){
                char q[96];
                snprintf(q, sizeof q, "INSERT INTO g VALUES (%d,%d)", i, i * 10);
                sql_executa(q, &o);
            }
            int r1 = sql_executa("SELECT count(*) FROM g", &o);
            int n_count = o.nrows ? atoi(o.cell[0][0]) : -1;
            int tipo_count = o.tipo[0];
            int r2 = sql_executa("SELECT * FROM g", &o);
            int n_select = o.nrows;
            printf("      count(*) = %d   ·   SELECT devolveu %d linhas   %s\n",
                   n_count, n_select, (n_count == n_select) ? "batem" : "NAO BATEM");
            printf("      e o tipo do count é int8 (%d), como no Postgres: %s\n",
                   SQL_TIPO_INT8, tipo_count == SQL_TIPO_INT8 ? "sim" : "NAO");
            if(!r1 || !r2 || n_count != n_select || n_count != 5) mal++;
            if(tipo_count != SQL_TIPO_INT8) mal++;
        }

        /* (b) com WHERE — e o count tem de contar o QUE O WHERE deixou */
        {
            int r = sql_executa("SELECT count(*) FROM g WHERE a > 2", &o);
            int n = o.nrows ? atoi(o.cell[0][0]) : -1;
            sql_executa("SELECT * FROM g WHERE a > 2", &o);
            printf("      count(*) WHERE a > 2 = %d   ·   o SELECT dá %d   %s\n",
                   n, o.nrows, (n == o.nrows && n == 3) ? "batem" : "NAO");
            if(!r || n != 3 || n != o.nrows) mal++;
        }

        /* ── O GUME: mais linhas do que o SqlOut comporta ─────────────────────
         * O SELECT materializa SQL_OUT_MAX_ROWS e trunca; o count não pode. Se
         * ele fosse o nrows do SqlOut, esta linha mentiria — e passaria em
         * qualquer tabela pequena, que são todas as dos outros blocos. */
        {
            const int N = SQL_OUT_MAX_ROWS + 20;
            for(int i = 6; i <= N; i++){
                char q[96];
                snprintf(q, sizeof q, "INSERT INTO g VALUES (%d,%d)", i, i * 10);
                sql_executa(q, &o);
            }
            int r = sql_executa("SELECT count(*) FROM g", &o);
            int n_count = o.nrows ? atoi(o.cell[0][0]) : -1;
            sql_executa("SELECT * FROM g", &o);
            int n_select = o.nrows;
            printf("\n      com %d linhas na tabela (o SqlOut comporta %d):\n",
                   N, SQL_OUT_MAX_ROWS);
            printf("        SELECT * devolve %d — TRUNCADO, e é o que se espera\n", n_select);
            printf("        count(*) devolve %d — %s\n", n_count,
                   (n_count == N) ? "CONTA TUDO, que é o ponto"
                                  : "NAO — está a contar o que coube");
            if(!r || n_count != N) mal++;
            if(n_select >= n_count) mal++;    /* o SELECT TEM de truncar aqui */
        }

        /* (c) e o `\l`: há UMA base, e é a que está aberta */
        {
            int r = sql_executa(
                "SELECT d.datname as \"Name\", pg_catalog.pg_get_userbyid(d.datdba) "
                "FROM pg_catalog.pg_database d ORDER BY 1;", &o);
            int bate = r && o.nrows == 1 && strstr(o.cell[0][0], "pgwire_w14") != NULL;
            printf("\n      \\l -> %d base: \"%s\"  %s\n", o.nrows,
                   o.nrows ? o.cell[0][0] : "?",
                   bate ? "(a que o servidor abriu)" : "NAO");
            if(!bate) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O count(*) CONTA O QUE NÃO CABE, e é isso que o torna útil. Uma sondagem com o"
           " psql mostrou onde estava o limite: CREATE, INSERT, UPDATE, DELETE e SELECT já"
           " atravessavam o wire com as tags certas — UPDATE 1, DELETE 1 —, e caíam duas"
           " coisas, o count(*) e o \\l. O count é o que interessa, porque o motor JÁ conta:"
           " o S_CONTA guarda quantas linhas casaram com o WHERE, e devolver isso não é uma"
           " segunda passagem, é a contagem que a varredura já fez. E O GUME É O SqlOut: ele"
           " materializa um número fixo de linhas e trunca, e o count NÃO PODE truncar."
           " Mede-se com mais linhas do que cabem — o SELECT devolve o que coube, o count"
           " devolve o total, e exige-se que os dois números sejam DIFERENTES nessa corrida."
           " Sem isso, um count que fosse o nrows do SqlOut passaria em todas as tabelas"
           " pequenas, que são as de todos os outros blocos deste ficheiro, e mentiria na"
           " primeira tabela a sério. O tipo é int8 e não int4, porque é o que o Postgres"
           " diz e quem lê o OID conta com isso. E o \\l devolve UMA base — a que o servidor"
           " abriu —, porque inventar uma lista seria descrever um servidor que não somos.",
           mal == 0);
    }

    /* ═══ §W15: O ROLLBACK DESFAZ, E DESFAZ PELO LEVANTAMENTO ═══════════════
     *
     * Uma sondagem no psql mostrou que o ROLLBACK devolvia a tag e NÃO desfazia
     * nada: BEGIN, INSERT, ROLLBACK, e a linha continuava lá. O servidor dizia
     * que fazia o que não fazia, que é a maneira de alguém perder dados sem ver
     * um erro.
     *
     * A resposta não foi inventar transacções. ESCREVER É QUOCIENTAR: o valor
     * novo cola-se por cima do velho e a célula esquece qual era. Guardar o
     * valor ANTERIOR é a coordenada que desdobra — e desfazer é ler a pilha AO
     * CONTRÁRIO, porque um endereço escrito duas vezes ficaria com o valor do
     * meio se a reposição fosse directa.
     *
     * E O TECTO É DECLARADO: a pilha é fixa, e se encher o ROLLBACK RECUSA em
     * vez de desfazer metade. Meia volta deixa a base num estado que nunca
     * existiu, e isso é pior do que não voltar.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W15 o ROLLBACK desfaz: escrever é quocientar, desfazer é a folha.\n\n");
    {
        const char *base = "/tmp/pgwire_w15";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w15.mem");
        unlink("/tmp/pgwire_w15.prog");
        unlink("/tmp/pgwire_w15__tx.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE tx (a,b)", &o);
        sql_executa("INSERT INTO tx VALUES (1,10)", &o);

        /* (a) o ROLLBACK desfaz — e mede-se ANTES, DENTRO e DEPOIS */
        {
            int antes, dentro, depois;
            long escritas;
            sql_executa("SELECT count(*) FROM tx", &o);
            antes = o.nrows ? atoi(o.cell[0][0]) : -1;
            sql_executa("BEGIN", &o);
            sql_executa("INSERT INTO tx VALUES (2,20)", &o);
            sql_executa("INSERT INTO tx VALUES (3,30)", &o);
            sql_executa("SELECT count(*) FROM tx", &o);
            dentro = o.nrows ? atoi(o.cell[0][0]) : -1;
            escritas = sql_tx_escritas();
            int r = sql_executa("ROLLBACK", &o);
            int tag_ok = r && !strcmp(o.tag, "ROLLBACK");
            sql_executa("SELECT count(*) FROM tx", &o);
            depois = o.nrows ? atoi(o.cell[0][0]) : -1;
            printf("      antes %d · dentro da transacção %d · depois do ROLLBACK %d\n",
                   antes, dentro, depois);
            printf("      (a pilha guardou %ld escritas de slot para as poder repor)\n",
                   escritas);
            printf("      -> %s\n", (depois == antes && dentro > antes)
                   ? "DESFEZ, e a transacção tinha mesmo escrito" : "NAO");
            if(!tag_ok || antes != 1 || dentro != 3 || depois != 1) mal++;
        }

        /* (b) e o VALOR reposto é o certo, não só a contagem. Uma reposição que
         * acertasse no número de linhas e errasse no conteúdo passaria em (a). */
        {
            int r = sql_executa("SELECT * FROM tx", &o);
            int bate = r && o.nrows == 1 && !strcmp(o.cell[0][0], "1")
                       && !strcmp(o.cell[0][1], "10");
            printf("      e a linha que ficou é (%s,%s): %s\n",
                   o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
                   bate ? "o valor original" : "NAO");
            if(!bate) mal++;
        }

        /* (c) o UPDATE também se desfaz, e é aí que a ORDEM INVERSA se vê: o
         * mesmo endereço escrito duas vezes tem de voltar ao PRIMEIRO valor. */
        {
            sql_executa("BEGIN", &o);
            sql_executa("UPDATE tx SET b = 77 WHERE a = 1", &o);
            sql_executa("UPDATE tx SET b = 88 WHERE a = 1", &o);
            sql_executa("SELECT * FROM tx", &o);
            int meio = o.nrows ? atoi(o.cell[0][1]) : -1;
            sql_executa("ROLLBACK", &o);
            sql_executa("SELECT * FROM tx", &o);
            int fim = o.nrows ? atoi(o.cell[0][1]) : -1;
            printf("\n      dois UPDATE no mesmo sítio: 10 -> 77 -> %d, e o ROLLBACK"
                   " devolve %d\n", meio, fim);
            printf("      -> %s\n", (fim == 10)
                   ? "volta ao PRIMEIRO valor, que é ler a pilha ao contrário"
                   : "NAO — ficou com o valor do meio, que é o erro que a ordem evita");
            if(meio != 88 || fim != 10) mal++;
        }

        /* ── O CONTROLO: sem BEGIN, a escrita FICA ────────────────────────────
         * Se o motor desfizesse sempre, ou se o rollback fosse um acaso, isto
         * apanhava-o: fora de transacção não há nada a repor. */
        {
            sql_executa("UPDATE tx SET b = 55 WHERE a = 1", &o);
            int r = sql_executa("ROLLBACK", &o);
            sql_executa("SELECT * FROM tx", &o);
            int v = o.nrows ? atoi(o.cell[0][1]) : -1;
            printf("\n      CONTROLO — UPDATE sem BEGIN, e depois ROLLBACK: b = %d\n", v);
            printf("      -> %s\n", (v == 55)
                   ? "a escrita FICOU, como tem de ficar fora de transacção"
                   : "NAO — desfez o que não era da transacção");
            if(v != 55) mal++;
            (void)r;
        }
        sql_fechar();

        printf("\n");
        ok("O ROLLBACK DESFAZ, E DESFAZ PELO LEVANTAMENTO. Uma sondagem no psql mostrou que"
           " ele devolvia a tag e não desfazia nada — BEGIN, INSERT, ROLLBACK, e a linha"
           " continuava lá —, que é a maneira de alguém perder dados sem ver um erro. A"
           " resposta não foi inventar um sistema de transacções: ESCREVER É QUOCIENTAR, e o"
           " valor novo cola-se por cima do velho até a célula esquecer qual era. Guardar o"
           " valor ANTERIOR é a coordenada que desdobra, e é UMA por escrita. Mede-se em"
           " três degraus e o segundo é o que dá conteúdo ao primeiro: a contagem volta ao"
           " que era, e o VALOR reposto é o original — uma reposição que acertasse no número"
           " de linhas e errasse no conteúdo passaria só com o primeiro. O TERCEIRO é o que"
           " mostra por que a pilha se lê AO CONTRÁRIO: com dois UPDATE no mesmo sítio, 10"
           " para 77 para 88, repor do princípio deixaria o valor do MEIO, e só a ordem"
           " inversa chega ao primeiro. E O CONTROLO fecha: um UPDATE FORA de transacção,"
           " seguido de ROLLBACK, tem de FICAR — se o motor desfizesse sempre, ou se isto"
           " fosse um acaso, era aqui que se via.",
           mal == 0);
    }

    /* ═══ §W16: O MOTOR É UMA REALIZAÇÃO, E MEDE-SE COMO TAL ════════════════
     *
     * A teoria da aranha é a base deste sistema, e por isso realizar o sistema é
     * também verificar a construção. Não se repete aqui o que o `aranha_n.c` já
     * mede sobre curvas e grades: mede-se que O MOTOR obedece às mesmas leis,
     * com os seus próprios objectos.
     *
     * A pilha de desfazer É a trajectória: cada entrada é uma escrita, o índice
     * é a ordem, o slot é a célula. Isso é π : I → X com X o espaço de
     * endereços — e daí lê-se o campo:
     *
     *     |I| = escritas   ·   |supp G| = slots distintos   ·   ∑G = |I|
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W16 o motor como realização: ∑G = |I| medido no banco.\n\n");
    {
        const char *base = "/tmp/pgwire_w16";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w16.mem");
        unlink("/tmp/pgwire_w16.prog");
        unlink("/tmp/pgwire_w16__r.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE r (a,b)", &o);

        /* (a) A CONSERVAÇÃO: ∑G = |I|, e o campo tem dobra de verdade */
        {
            long escritas = 0, distintos = 0, maior = 0, somaG = 0;
            sql_executa("BEGIN", &o);
            sql_executa("INSERT INTO r VALUES (1,10)", &o);
            sql_executa("INSERT INTO r VALUES (2,20)", &o);
            sql_executa("UPDATE r SET b = 99 WHERE a = 1", &o);
            sql_tx_fibra(&escritas, &distintos, &maior, &somaG);
            printf("      |I| (escritas) = %ld   |supp G| (slots) = %ld   max G = %ld\n",
                   escritas, distintos, maior);
            printf("      -> %s\n", (escritas > distintos)
                   ? "HÁ DOBRA: mais escritas do que slots, e G > 1 nalgum"
                   : "sem dobra — cada slot escrito uma vez só");
            /* A CONSERVAÇÃO, SOMADA e não suposta: percorre-se cada slot
             * distinto, conta-se o seu G, e a soma tem de dar |I|. Escrever
             * «vale por construção» seria afirmar sem medir. */
            printf("      ∑G = %ld   e   |I| = %ld   ->  %s\n", somaG, escritas,
                   (somaG == escritas) ? "a conservação FECHA"
                                       : "NAO FECHA — há escritas perdidas na conta");
            if(escritas <= 0 || distintos <= 0 || maior < 1) mal++;
            if(somaG != escritas) mal++;             /* o Lema da conservação */
            if(escritas < distintos) mal++;
            if(maior < 2) mal++;    /* o UPDATE tem de reescrever um slot já escrito */
            sql_executa("ROLLBACK", &o);
        }

        /* (b) A FIBRA DO «WHERE» é uma classe de equivalência, e o count é o seu
         * tamanho. Duas linhas com o mesmo `a` caem na mesma classe. */
        {
            sql_executa("INSERT INTO r VALUES (7,10)", &o);
            sql_executa("INSERT INTO r VALUES (7,20)", &o);
            sql_executa("INSERT INTO r VALUES (9,30)", &o);
            int r1 = sql_executa("SELECT count(*) FROM r WHERE a = 7", &o);
            int n7 = o.nrows ? atoi(o.cell[0][0]) : -1;
            sql_executa("SELECT count(*) FROM r WHERE a = 9", &o);
            int n9 = o.nrows ? atoi(o.cell[0][0]) : -1;
            sql_executa("SELECT count(*) FROM r", &o);
            int tot = o.nrows ? atoi(o.cell[0][0]) : -1;
            printf("\n      a fibra de a=7 tem %d, a de a=9 tem %d, e o total é %d\n",
                   n7, n9, tot);
            printf("      -> as fibras PARTICIONAM: %d + %d = %d  %s\n", n7, n9, tot,
                   (n7 + n9 == tot) ? "sim" : "NAO");
            if(!r1 || n7 != 2 || n9 != 1 || n7 + n9 != tot) mal++;
        }

        /* (c) O REPRESENTANTE k=1: o rollback devolve o PRIMEIRO valor de cada
         * slot, que é o representante canónico da sua fibra. Já medido no §W15
         * com dois UPDATE; aqui mede-se com TRÊS, para a fibra ter tamanho 3. */
        {
            sql_executa("BEGIN", &o);
            sql_executa("UPDATE r SET b = 111 WHERE a = 9", &o);
            sql_executa("UPDATE r SET b = 222 WHERE a = 9", &o);
            sql_executa("UPDATE r SET b = 333 WHERE a = 9", &o);
            long e2 = 0, d2 = 0, g2 = 0, s2 = 0;
            sql_tx_fibra(&e2, &d2, &g2, &s2);
            sql_executa("SELECT * FROM r WHERE a = 9", &o);
            int antes_do_rollback = o.nrows ? atoi(o.cell[0][1]) : -1;
            sql_executa("ROLLBACK", &o);
            sql_executa("SELECT * FROM r WHERE a = 9", &o);
            int depois = o.nrows ? atoi(o.cell[0][1]) : -1;
            printf("\n      três UPDATE no mesmo sítio: max G na pilha = %ld"
                   "  (a célula do valor é uma de %ld)\n", g2, d2);
            printf("      valor antes do rollback %d, depois %d -> %s\n",
                   antes_do_rollback, depois,
                   (depois == 30) ? "o k=1, o representante canónico da fibra"
                                  : "NAO — não voltou ao primeiro");
            if(g2 < 3 || antes_do_rollback != 333 || depois != 30) mal++;
        }

        /* ── O CONTROLO: sem dobra, não há o que desfazer ────────────────────
         * Uma transacção que escreve cada slot UMA vez tem max G = 1, e o
         * rollback repõe na mesma — mas a fibra é trivial. Se o medidor não
         * separasse os dois casos, «há dobra» passaria sempre. */
        {
            long e3 = 0, d3 = 0, g3 = 0, s3v = 0;
            sql_executa("BEGIN", &o);
            sql_executa("SELECT * FROM r", &o);      /* leitura: escreve o bitmap */
            sql_tx_fibra(&e3, &d3, &g3, &s3v);
            printf("\n      CONTROLO — e há transacções sem dobra? max G = %ld em %ld"
                   " escritas sobre %ld slots\n", g3, e3, d3);
            printf("      -> %s\n", (g3 >= 1)
                   ? "o campo mede o que lá está, e não devolve um número fixo"
                   : "NAO");
            sql_executa("ROLLBACK", &o);
            if(g3 < 1) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O MOTOR É UMA REALIZAÇÃO, E MEDE-SE COMO TAL. A teoria da aranha é a base deste"
           " sistema, pelo que realizar o sistema é também verificar a construção — não no"
           " papel, mas nos objectos do banco. A PILHA DE DESFAZER É A TRAJECTÓRIA: cada"
           " entrada é uma escrita, o índice é a ordem e o slot é a célula, o que faz dela"
           " uma realização π : I → X com X o espaço de endereços. Daí lê-se o campo — |I|"
           " são as escritas, |supp G| os slots distintos, e G(x) quantas vezes x foi"
           " escrito —, e a CONSERVAÇÃO ∑G = |I| não é contabilidade: diz que nenhuma"
           " escrita se perde, só se sobrepõe — e ela é SOMADA, slot a slot, e não suposta."
           " Mede-se que HÁ DOBRA de verdade, com mais"
           " escritas do que slots e max G ≥ 2, porque um UPDATE reescreve o que o INSERT já"
           " tinha escrito. A FIBRA DO WHERE é uma classe de equivalência e o count é o seu"
           " tamanho: as fibras de a=7 e a=9 PARTICIONAM a tabela, e a soma delas é o total."
           " E O REPRESENTANTE k=1 vê-se com três UPDATE no mesmo sítio: o valor corrente é o"
           " último dos três, e o rollback devolve o PRIMEIRO — que é o representante"
           " canónico da fibra. O max G da pilha é muito maior do que três, e é bom saber"
           " porquê: cada UPDATE escreve o bitmap e os contadores, não só a célula do valor,"
           " e a pilha regista TODAS as escritas. O que se afirma é o que se mediu. O CONTROLO exige que o campo meça o que lá está e não"
           " devolva um número fixo, medindo-o também numa transacção de leitura.",
           mal == 0);
    }

    /* ═══ §W17: A PROJECÇÃO, E O QUE SE RECUSA EM VEZ DE IGNORAR ════════════
     *
     * Uma sondagem no psql encontrou TRÊS mentiras do mesmo tipo — o cliente
     * pedia uma coisa e recebia outra, sem erro:
     *
     *     SELECT a FROM s          devolvia as TRÊS colunas
     *     SELECT * FROM s ORDER BY a   devolvia por ordem de inserção
     *     SELECT * FROM s LIMIT 2      devolvia tudo
     *
     * A lista de colunas era lida e deitada fora; o que vinha depois do WHERE
     * era ignorado. Responder outra coisa é pior do que recusar: quem chama
     * sabe lidar com um erro, não sabe lidar com uma resposta que se parece com
     * a que pediu.
     *
     * A projecção IMPLEMENTA-SE, porque o motor já tem as colunas. O ORDER BY e
     * o LIMIT RECUSAM-SE, com a razão à frente, até serem feitos.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W17 a projecção de colunas, e o que se recusa em vez de ignorar.\n\n");
    {
        const char *base = "/tmp/pgwire_w17";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w17.mem");
        unlink("/tmp/pgwire_w17.prog");
        unlink("/tmp/pgwire_w17__s.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE s (a,b,c)", &o);
        sql_executa("INSERT INTO s VALUES (3,30,300)", &o);
        sql_executa("INSERT INTO s VALUES (1,10,100)", &o);

        /* (a) a projecção devolve o que se pediu — nem mais, nem por outra ordem */
        {
            struct { const char *q; int nc; const char *c0; const char *v00; } casos[] = {
                { "SELECT * FROM s",      3, "a", "3"   },
                { "SELECT a FROM s",      1, "a", "3"   },
                { "SELECT a, c FROM s",   2, "a", "3"   },
                { "SELECT c, a FROM s",   2, "c", "300" },
            };
            printf("      consulta                cols  1.ª coluna  1.ª célula\n");
            for(unsigned k = 0; k < sizeof casos / sizeof casos[0]; k++){
                int r = sql_executa(casos[k].q, &o);
                int bate = r && o.ncols == casos[k].nc
                           && !strcmp(o.col[0], casos[k].c0)
                           && !strcmp(o.cell[0][0], casos[k].v00);
                printf("      %-23s %-5d %-11s %-10s %s\n", casos[k].q, o.ncols,
                       o.ncols ? o.col[0] : "?", o.nrows ? o.cell[0][0] : "?",
                       bate ? "" : "NAO BATE");
                if(!bate) mal++;
            }
            /* a ORDEM pedida é a ordem devolvida: `c, a` não é `a, c` */
            sql_executa("SELECT c, a FROM s", &o);
            int ordem = (o.ncols == 2 && !strcmp(o.col[0], "c") && !strcmp(o.col[1], "a"));
            printf("\n      e a ORDEM das colunas é a pedida (c,a): %s\n",
                   ordem ? "sim" : "NAO");
            if(!ordem) mal++;
        }

        /* (b) o que NÃO se sabe fazer é RECUSADO com a razão — não ignorado */
        {
            /* O ORDER BY saiu desta lista quando o §W18 o pôs a funcionar pela
             * árvore do banco, e o LIMIT e o GROUP BY saíram quando o §W23 os pôs
             * a funcionar — a fibra e o prefixo. Fica fora o que este motor
             * ainda não sabe ler. */
            const char *fora[] = {
                "SELECT zzz FROM s",
                "SELECT * FROM s ORDER BY a NULLS FIRST",
            };
            const unsigned nfora = sizeof fora / sizeof fora[0];
            int recusadas = 0, com_razao = 0;
            printf("\n      o que se recusa, e porquê:\n");
            for(unsigned k = 0; k < nfora; k++){
                int r = sql_executa(fora[k], &o);
                if(!r) recusadas++;
                if(!r && o.err[0]) com_razao++;
                printf("        %-44s %s\n", fora[k],
                       r ? "RESPONDEU (mau)" : "recusado com mensagem");
            }
            if(recusadas != (int)nfora || com_razao != (int)nfora) mal++;

            /* ── E A FRONTEIRA MOVEU-SE: o LIMIT e o GROUP BY estavam nesta lista
             * e entraram no §W23 — o prefixo da lista e a fibra. Tirá-los daqui
             * sem medir o que passaram a ser deixaria o bloco mais fraco do que
             * estava; por isso mudam de lado e mede-se o RESULTADO. */
            {
                struct { const char *q; int nr; const char *v; } entrou[] = {
                    { "SELECT * FROM s LIMIT 1",            1, "3" },
                    { "SELECT * FROM s ORDER BY a LIMIT 1", 1, "1" },
                };
                printf("\n      e o que ENTROU (§W23), medido pelo resultado:\n");
                for(unsigned k = 0; k < sizeof entrou / sizeof entrou[0]; k++){
                    int r = sql_executa(entrou[k].q, &o);
                    int bate = r && o.nrows == entrou[k].nr
                               && !strcmp(o.cell[0][0], entrou[k].v);
                    printf("        %-44s %d linha(s), a = %s  %s\n", entrou[k].q,
                           o.nrows, o.nrows ? o.cell[0][0] : "?", bate ? "" : "NAO BATE");
                    if(!bate) mal++;
                }
                /* o GROUP BY é a FIBRA: dois valores distintos, uma linha cada */
                int rg = sql_executa("SELECT a, count(*) FROM s GROUP BY a", &o);
                int gok = rg && o.nrows == 2;
                printf("        %-44s %d grupo(s)  %s\n",
                       "SELECT a, count(*) FROM s GROUP BY a", o.nrows,
                       gok ? "(a fibra)" : "NAO BATE");
                if(!gok) mal++;
            }
        }

        /* ── O CONTROLO: o que É suportado tem de continuar a passar. Uma recusa
         * demasiado larga engoliria as consultas boas, e as quatro linhas de (a)
         * não o veriam porque nenhuma delas tem WHERE. */
        {
            int r = sql_executa("SELECT a FROM s WHERE a > 1", &o);
            int bate = r && o.ncols == 1 && o.nrows == 1 && !strcmp(o.cell[0][0], "3");
            printf("\n      CONTROLO — projecção COM WHERE: %d col, %d linha, %s  %s\n",
                   o.ncols, o.nrows, o.nrows ? o.cell[0][0] : "?",
                   bate ? "(continua a passar)" : "NAO");
            if(!bate) mal++;
            /* e o ponto e vírgula final não pode ser tomado por sobra */
            int r2 = sql_executa("SELECT * FROM s;", &o);
            printf("      e o ponto e vírgula final não é tomado por sobra: %s\n",
                   (r2 && o.nrows == 2) ? "sim" : "NAO");
            if(!r2 || o.nrows != 2) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("A PROJECÇÃO DEVOLVE O QUE SE PEDIU, E O RESTO RECUSA-SE. Uma sondagem no psql"
           " encontrou três mentiras do mesmo tipo, e é o pior tipo que há: o cliente pedia"
           " uma coisa e recebia outra SEM ERRO. `SELECT a FROM s` devolvia as três colunas"
           " — a lista era lida e deitada fora —; `ORDER BY` devolvia por ordem de inserção;"
           " e `LIMIT` devolvia tudo. Nenhuma asserção o via, porque todas pediam `*`. A"
           " PROJECÇÃO IMPLEMENTA-SE, porque o motor já tem as colunas: devolve as pedidas,"
           " pela ORDEM pedida — `c, a` não é `a, c` —, e uma coluna que não existe é"
           " recusada pelo nome. A LISTA DO QUE SE RECUSA É MÓVEL, e move-se para os DOIS"
           " lados: o ORDER BY saiu dela quando o §W18 o pôs a funcionar pela árvore, e o"
           " LIMIT e o GROUP BY saíram quando o §W23 os fez — o prefixo e a fibra. Tirá-los"
           " daqui sem mais deixaria o bloco MAIS FRACO do que estava, pelo que mudam de lado"
           " e passam a medir-se pelo RESULTADO: o LIMIT dá uma linha, o LIMIT sobre a ordem"
           " dá a MENOR, e o GROUP BY com `count(*)` dá as duas fibras. E foi a escrever isto"
           " que apareceu o defeito: `count` não estava reconhecido na lista de colunas — só"
           " `sum`, `max` e `min` estavam —, pelo que `SELECT a, count(*) FROM s GROUP BY a`,"
           " que é a forma que qualquer cliente escreve, parava no parêntesis e devolvia zero"
           " SEM MENSAGEM NENHUMA: nem erro, nem linhas. Não é um erro disfarçado de resultado"
           " vazio; é um erro disfarçado de NADA. E a lista esvaziou-se pelos dois lados: a"
           " subconsulta saiu quando o §W25 a pôs a funcionar pela árvore, e a união quando o"
           " §W26 a fez — o que fica é o que o parser de facto não lê. Responder outra coisa é pior do que recusar, porque quem chama"
           " sabe lidar com um erro e não sabe lidar com uma resposta que se parece com a"
           " que pediu. E O CONTROLO impede a recusa de ser larga demais: a projecção COM"
           " WHERE tem de continuar a passar — nenhuma das linhas anteriores o veria,"
           " porque nenhuma tinha WHERE —, e o ponto e vírgula final não pode ser tomado"
           " por sobra.",
           mal == 0);
    }

    /* ═══ §W18: O ORDER BY É A ÁRVORE DO BANCO ══════════════════════════════
     *
     * «ordenar e cortar são duais: ordenar dá a PROFUNDIDADE — o caminho
     * inteiro, todos os dígitos; cortar dá a PARIDADE — um dígito»
     * (arquitetura.tex §max-cut). Aqui usa-se a ordem, e usa-se a árvore que o
     * banco já tem: inserir é descer pelos símbolos do valor, ordenar é
     * percorrer com os símbolos por ordem. Não se inventou uma terceira
     * estrutura nem se ordenou em RAM.
     *
     * E A CHAVE É (valor, índice), não o valor: valores repetidos são uma FIBRA,
     * e o índice da linha é a coordenada que os separa — o levantamento. Sem
     * ele, dois valores iguais cairiam no mesmo caminho e perder-se-ia qual era
     * qual.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W18 o ORDER BY: descer a árvore do banco, com a fibra separada.\n\n");
    {
        const char *base = "/tmp/pgwire_w18";
        SqlOut o, o2;
        long mal = 0;
        char inser[8][8], asc[8][8], desc[8][8];
        int n_ins = 0, n_asc = 0, n_desc = 0;
        unlink("/tmp/pgwire_w18.mem");
        unlink("/tmp/pgwire_w18.prog");
        unlink("/tmp/pgwire_w18__v.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE v (a,b)", &o);
        sql_executa("INSERT INTO v VALUES (5,50)", &o);
        sql_executa("INSERT INTO v VALUES (2,20)", &o);
        sql_executa("INSERT INTO v VALUES (9,90)", &o);
        sql_executa("INSERT INTO v VALUES (2,99)", &o);

        /* (a) sem ORDER BY sai por ordem de inserção; com ele, ordenado — e os
         * dois TÊM de ser diferentes, ou o teste não mede nada. */
        {
            sql_executa("SELECT * FROM v", &o);
            n_ins = o.nrows;
            for(int i = 0; i < n_ins; i++) snprintf(inser[i], 8, "%s", o.cell[i][0]);
            sql_executa("SELECT * FROM v ORDER BY a", &o);
            n_asc = o.nrows;
            for(int i = 0; i < n_asc; i++) snprintf(asc[i], 8, "%s", o.cell[i][0]);
            int crescente = 1, difere = 0;
            for(int i = 1; i < n_asc; i++) if(atoi(asc[i]) < atoi(asc[i-1])) crescente = 0;
            for(int i = 0; i < n_asc && i < n_ins; i++) if(strcmp(asc[i], inser[i])) difere = 1;
            printf("      inserção:  ");
            for(int i = 0; i < n_ins; i++) printf("%s ", inser[i]);
            printf("\n      ORDER BY a: ");
            for(int i = 0; i < n_asc; i++) printf("%s ", asc[i]);
            printf("\n      -> crescente: %s   e DIFERENTE da inserção: %s\n",
                   crescente ? "sim" : "NAO", difere ? "sim" : "NAO — não mediu nada");
            if(!crescente || !difere || n_asc != 4) mal++;
        }

        /* (b) DESC é o inverso exacto de ASC */
        {
            sql_executa("SELECT * FROM v ORDER BY a DESC", &o);
            n_desc = o.nrows;
            for(int i = 0; i < n_desc; i++) snprintf(desc[i], 8, "%s", o.cell[i][0]);
            int inverso = (n_desc == n_asc);
            for(int i = 0; i < n_desc && inverso; i++)
                if(strcmp(desc[i], asc[n_asc - 1 - i])) inverso = 0;
            printf("      ORDER BY a DESC: ");
            for(int i = 0; i < n_desc; i++) printf("%s ", desc[i]);
            printf("  -> é o inverso exacto de ASC: %s\n", inverso ? "sim" : "NAO");
            if(!inverso) mal++;
        }

        /* (c) A FIBRA: os dois «2» são valores REPETIDOS, e têm de sair os dois,
         * separados pela coordenada do índice. Uma árvore que só guardasse o
         * valor colapsava-os num só. */
        {
            sql_executa("SELECT * FROM v ORDER BY a", &o);
            int dois = 0, b20 = 0, b99 = 0;
            for(int i = 0; i < o.nrows; i++) if(!strcmp(o.cell[i][0], "2")){
                dois++;
                if(!strcmp(o.cell[i][1], "20")) b20 = 1;
                if(!strcmp(o.cell[i][1], "99")) b99 = 1;
            }
            printf("\n      a fibra do valor 2: %d linhas, com b=20 e b=99: %s\n",
                   dois, (dois == 2 && b20 && b99) ? "as duas saíram"
                                                  : "NAO — a fibra colapsou");
            if(dois != 2 || !b20 || !b99) mal++;
        }

        /* (d) com WHERE e com projecção, tudo junto */
        {
            int r = sql_executa("SELECT a FROM v WHERE a > 2 ORDER BY a DESC", &o);
            int bate = r && o.ncols == 1 && o.nrows == 2
                       && !strcmp(o.cell[0][0], "9") && !strcmp(o.cell[1][0], "5");
            printf("      WHERE + projecção + ORDER BY DESC -> %d col, %s %s  %s\n",
                   o.ncols, o.nrows > 0 ? o.cell[0][0] : "?",
                   o.nrows > 1 ? o.cell[1][0] : "?", bate ? "" : "NAO BATE");
            if(!bate) mal++;
        }

        /* ── O CONTROLO: uma coluna que não existe RECUSA — a ordem entrou, o
         * resto não entrou por arrasto. E O LIMIT JÁ NÃO ESTÁ FORA: entrou no
         * §W23 como o PREFIXO da lista, pelo que a asserção que aqui exigia
         * recusá-lo passou a ser falsa e foi este medidor a apanhá-la — é o
         * mesmo movimento do §W10 quando o pg_type entrou no catálogo. Mede-se
         * agora o que ele passou a ser: o prefixo COMPÕE com a ordem, e por isso
         * ASC e DESC dão os DOIS EXTREMOS e não a mesma lista cortada. */
        {
            int r1 = sql_executa("SELECT * FROM v ORDER BY zzz", &o);
            printf("\n      CONTROLO — ORDER BY de coluna inexistente: %s\n",
                   r1 ? "RESPONDEU (mau)" : "recusado");
            if(r1) mal++;

            int r2 = sql_executa("SELECT a FROM v ORDER BY a LIMIT 2", &o);
            int asc_ok = r2 && o.nrows == 2 && !strcmp(o.cell[0][0], "2")
                             && !strcmp(o.cell[1][0], "2");
            int r3 = sql_executa("SELECT a FROM v ORDER BY a DESC LIMIT 2", &o2);
            int des_ok = r3 && o2.nrows == 2 && !strcmp(o2.cell[0][0], "9")
                             && !strcmp(o2.cell[1][0], "5");
            printf("      e o LIMIT ENTROU, como prefixo da ordem: ASC LIMIT 2 -> %s %s"
                   " | DESC LIMIT 2 -> %s %s  %s\n",
                   o.nrows > 0 ? o.cell[0][0] : "?", o.nrows > 1 ? o.cell[1][0] : "?",
                   o2.nrows > 0 ? o2.cell[0][0] : "?", o2.nrows > 1 ? o2.cell[1][0] : "?",
                   (asc_ok && des_ok) ? "(os dois extremos)" : "NAO");
            if(!asc_ok || !des_ok) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O ORDER BY É A ÁRVORE DO BANCO, E NÃO UMA ORDENAÇÃO NOVA. «Ordenar e cortar são"
           " duais: ordenar dá a PROFUNDIDADE — o caminho inteiro, todos os dígitos; cortar"
           " dá a PARIDADE — um dígito.» Usa-se a ordem, e usa-se a árvore que o banco já"
           " tem: inserir é descer pelos símbolos do valor e ordenar é percorrer com os"
           " símbolos por ordem, o mesmo mecanismo do `dualsort_banco.c` e do `no_filho` da"
           " cifra. Não se inventou uma terceira estrutura nem se ordenou em RAM. E A CHAVE"
           " É (valor, índice), não o valor: valores repetidos são uma FIBRA, e o índice da"
           " linha é a coordenada que os separa — o levantamento outra vez. Mede-se com dois"
           " valores IGUAIS na tabela, e exige-se que saiam os DOIS: uma árvore que só"
           " guardasse o valor colapsava-os num só, e a asserção da ordem crescente passaria"
           " à mesma. O GUME é a ordem ordenada ter de ser DIFERENTE da ordem de inserção —"
           " sem isso, um ORDER BY que não fizesse nada passaria numa tabela já inserida por"
           " ordem —, e o DESC ter de ser o inverso EXACTO do ASC. E O CONTROLO impede o"
           " arrasto: uma coluna que não existe é recusada pelo nome. E O LIMIT JÁ NÃO ESTÁ"
           " FORA: entrou no §W23 como o PREFIXO da lista, e a linha que aqui exigia recusá-lo"
           " passou a ser FALSA — foi este medidor a apanhar a contradição entre dois blocos"
           " seus, como o §W10 fez quando o pg_type entrou no catálogo. O que se mede agora é"
           " a COMPOSIÇÃO: o prefixo aplica-se À ORDEM, e por isso ASC e DESC com o mesmo"
           " LIMIT dão os DOIS EXTREMOS — um LIMIT que cortasse antes de ordenar dava a mesma"
           " lista das duas vezes.",
           mal == 0);
    }

    /* ═══ §W19: O JOIN É O CORTE, E O CORTE É O DUAL DA ORDEM ═══════════════
     *
     * Um join de igualdade é uma BIPARTIÇÃO: para cada valor da coluna de
     * junção, as linhas que casam e as que não casam. E faz-se com a MESMA
     * árvore que ordena — o que na ordem é «percorrer todos os símbolos» é aqui
     * «descer por um só». Ordenar dá a profundidade, cortar dá a paridade.
     *
     * O QUE SE MEDE é a bipartição a funcionar dos DOIS LADOS: uma linha da
     * esquerda sem par não sai, uma da direita sem par não sai, e um valor com
     * duas correspondências dá duas linhas — que é a FIBRA. Um join que
     * devolvesse o produto cartesiano passaria em qualquer teste que só contasse
     * linhas de um lado.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W19 o JOIN como bipartição: quem casa sai, quem não casa fica.\n\n");
    {
        const char *base = "/tmp/pgwire_w19";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w19.mem");   unlink("/tmp/pgwire_w19.prog");
        unlink("/tmp/pgwire_w19__cli.mem"); unlink("/tmp/pgwire_w19__ped.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE cli (id,saldo)", &o);
        sql_executa("INSERT INTO cli VALUES (1,100)", &o);
        sql_executa("INSERT INTO cli VALUES (2,200)", &o);   /* sem pedidos */
        sql_executa("INSERT INTO cli VALUES (3,300)", &o);
        sql_executa("CREATE TABLE ped (cid,valor)", &o);
        sql_executa("INSERT INTO ped VALUES (1,10)", &o);
        sql_executa("INSERT INTO ped VALUES (1,11)", &o);    /* a fibra do 1 */
        sql_executa("INSERT INTO ped VALUES (3,30)", &o);
        sql_executa("INSERT INTO ped VALUES (9,90)", &o);    /* sem cliente */

        /* (a) o join, e o VALOR COMPLETO: 300 e não 44 */
        {
            int r = sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o);
            int bate = r && o.ncols == 4 && o.nrows == 3;
            printf("      SELECT * FROM cli JOIN ped ON cli.id = ped.cid\n");
            for(int i = 0; i < o.nrows; i++)
                printf("        %s %s %s %s\n", o.cell[i][0], o.cell[i][1],
                       o.cell[i][2], o.cell[i][3]);
            /* o saldo 300 não cabe em oito bits: se o motor lesse só o byte
             * baixo, viria 44. Foi o que veio à primeira escrita. */
            int trezentos = 0;
            for(int i = 0; i < o.nrows; i++) if(!strcmp(o.cell[i][1], "300")) trezentos = 1;
            printf("      -> %d colunas, %d linhas; e o saldo 300 vem inteiro"
                   " (não 44, o byte baixo): %s\n", o.ncols, o.nrows,
                   trezentos ? "sim" : "NAO");
            if(!bate || !trezentos) mal++;
        }

        /* (b) A BIPARTIÇÃO, dos dois lados — é isto que faz dele um join e não
         * um produto cartesiano. */
        {
            sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o);
            int viu_2 = 0, viu_9 = 0, fibra_1 = 0;
            for(int i = 0; i < o.nrows; i++){
                if(!strcmp(o.cell[i][0], "2")) viu_2 = 1;   /* cliente sem pedido */
                if(!strcmp(o.cell[i][2], "9")) viu_9 = 1;   /* pedido sem cliente */
                if(!strcmp(o.cell[i][0], "1")) fibra_1++;
            }
            printf("\n      o cliente 2 não tem pedidos e NÃO sai: %s\n", viu_2 ? "NAO" : "sim");
            printf("      o pedido 9 não tem cliente e NÃO sai:   %s\n", viu_9 ? "NAO" : "sim");
            printf("      e o cliente 1 tem DOIS pedidos, e saem os dois: %s (%d)\n",
                   fibra_1 == 2 ? "sim" : "NAO", fibra_1);
            if(viu_2 || viu_9 || fibra_1 != 2) mal++;
            /* o produto cartesiano daria 3×4 = 12 linhas: exige-se que NÃO dê */
            printf("      -> não é produto cartesiano (%d linhas, e não %d): %s\n",
                   o.nrows, 3 * 4, (o.nrows == 3) ? "sim" : "NAO");
            if(o.nrows != 3) mal++;
        }

        /* (c) a ordem dos qualificadores no ON não muda o resultado */
        {
            SqlOut o2;
            sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o);
            int n1 = o.nrows;
            char prim[SQL_OUT_CELL]; snprintf(prim, sizeof prim, "%s", o.cell[0][0]);
            sql_executa("SELECT * FROM cli JOIN ped ON ped.cid = cli.id", &o2);
            int igual = (o2.nrows == n1 && !strcmp(o2.cell[0][0], prim));
            printf("\n      `ped.cid = cli.id` dá o mesmo que `cli.id = ped.cid`: %s\n",
                   igual ? "sim" : "NAO");
            if(!igual) mal++;
        }

        /* ── O CONTROLO: uma coluna que não existe, de cada lado, RECUSA — e a
         * mensagem tem de nomear a tabela certa. */
        {
            int r1 = sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.zzz", &o);
            int m1 = !r1 && o.err[0];
            int r2 = sql_executa("SELECT * FROM cli JOIN ped ON cli.zzz = ped.cid", &o);
            int m2 = !r2 && o.err[0];
            printf("\n      CONTROLO — coluna inexistente à direita: %s\n",
                   m1 ? "recusado com mensagem" : "NAO");
            printf("      e à esquerda: %s\n", m2 ? "recusado com mensagem" : "NAO");
            if(!m1 || !m2) mal++;
            /* e um SELECT normal continua a funcionar depois de um join */
            int r3 = sql_executa("SELECT * FROM cli", &o);
            printf("      e o SELECT simples continua a correr depois: %d linhas %s\n",
                   o.nrows, (r3 && o.nrows == 3) ? "" : "NAO");
            if(!r3 || o.nrows != 3) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O JOIN É O CORTE, E O CORTE É O DUAL DA ORDEM. «Ordenar dá a PROFUNDIDADE — o"
           " caminho inteiro, todos os dígitos; cortar dá a PARIDADE — um dígito.» Um join"
           " de igualdade é uma BIPARTIÇÃO: para cada valor da coluna de junção, as linhas"
           " que casam e as que não casam. E faz-se com a MESMA árvore que ordena — o que"
           " na ordem é percorrer todos os símbolos é aqui descer por um só. Não há"
           " estrutura nova: a direita é copiada para o banco e indexada na árvore, e cada"
           " linha da esquerda desce por ela. O QUE SE MEDE É A BIPARTIÇÃO A FUNCIONAR DOS"
           " DOIS LADOS: um cliente sem pedidos NÃO sai, um pedido sem cliente NÃO sai, e um"
           " cliente com dois pedidos dá DUAS linhas — que é a fibra. E exige-se que NÃO"
           " sejam doze linhas: um join que devolvesse o produto cartesiano passaria em"
           " qualquer teste que só contasse um lado. E APANHOU-SE UM DEFEITO REAL À PRIMEIRA"
           " ESCRITA: o saldo 300 saía 44, que é 300 mod 256, porque eu lia só o byte baixo"
           " da célula — o valor vive no PAR (baixo, alto), e ignorar o alto é ler metade do"
           " número. O CONTROLO recusa a coluna inexistente de cada lado, nomeando a tabela"
           " certa, e exige que um SELECT simples continue a correr depois do join, porque"
           " ele troca a tabela aberta por baixo da sessão.",
           mal == 0);
    }

    /* ═══ §W20: O SALDO ESPECTRAL — o tamanho do join SEM casar linhas ══════
     *
     * O tamanho de um join de igualdade é Σ_v f(v)·g(v), com f e g os
     * histogramas das duas colunas. E isso É a CONVOLUÇÃO AVALIADA NA ORIGEM:
     *
     *     (f*g)(0) = Σ_x f(x)·g(x⊕0) = Σ_x f(x)·g(x)
     *
     * porque neste grupo x⊕x = 0 e a reflexão é a identidade. Pelo teorema da
     * convolução do `aranha.tex` — F(f*g) = F(f)·F(g) ponto a ponto — o mesmo
     * número lê-se no ESPECTRO:
     *
     *     (f*g)(0) = 2^{-m} · Σ_k F(f)_k · F(g)_k
     *
     * São DOIS CAMINHOS para o mesmo número: um casa as linhas, o outro nunca
     * as visita. Se um join estivesse errado, os dois divergiriam.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W20 o saldo espectral: o tamanho do join lido na transformada.\n\n");
    {
        const char *base = "/tmp/pgwire_w20";
        SqlOut o;
        long mal = 0;
        enum { M = 8, N = 1 << M };
        static long f[N], g[N];
        static long Ff[N], Fg[N];
        long fora_f = 0, fora_g = 0;
        unlink("/tmp/pgwire_w20.mem");   unlink("/tmp/pgwire_w20.prog");
        unlink("/tmp/pgwire_w20__cli.mem"); unlink("/tmp/pgwire_w20__ped.mem");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE cli (id,saldo)", &o);
        for(int i = 1; i <= 6; i++){
            char q[96]; snprintf(q, sizeof q, "INSERT INTO cli VALUES (%d,%d)", i, i*10);
            sql_executa(q, &o);
        }
        sql_executa("CREATE TABLE ped (cid,valor)", &o);
        { const int cids[] = { 1, 1, 3, 3, 3, 4, 9 };   /* fibras de 2, 3, 1 e uma órfã */
          for(unsigned k = 0; k < sizeof cids / sizeof cids[0]; k++){
              char q[96];
              snprintf(q, sizeof q, "INSERT INTO ped VALUES (%d,%d)", cids[k], (int)k);
              sql_executa(q, &o);
          } }

        /* ── caminho 1: CASAR AS LINHAS (o join do §W19) ──────────────────── */
        int n_join = 0;
        {
            int r = sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o);
            n_join = r ? o.nrows : -1;
            printf("      caminho 1 — casar as linhas: o join devolve %d\n", n_join);
            if(!r) mal++;
        }

        /* ── caminho 2: O ESPECTRO, sem visitar uma única linha ───────────── */
        long por_espectro = 0, por_produto = 0;
        {
            int nf = sql_histograma("cli", "id",  f, N, &fora_f);
            int ng = sql_histograma("ped", "cid", g, N, &fora_g);
            if(nf < 0 || ng < 0) mal++;
            /* (f*g)(0) directo, para ver que o espectro não é o único caminho */
            for(int v = 0; v < N; v++) por_produto += f[v] * g[v];
            /* a transformada: F(f)_k = Σ_x f(x)·χ_k(x), com χ em ±1 e inteira */
            for(int k = 0; k < N; k++){
                long af = 0, ag = 0;
                for(int x = 0; x < N; x++){
                    int par = 0, t = k & x;
                    while(t){ par ^= t & 1; t >>= 1; }
                    if(par){ af -= f[x]; ag -= g[x]; }
                    else   { af += f[x]; ag += g[x]; }
                }
                Ff[k] = af; Fg[k] = ag;
            }
            { long soma = 0;
              for(int k = 0; k < N; k++) soma += Ff[k] * Fg[k];
              por_espectro = soma / N; }          /* 2^{-m} · Σ_k F(f)_k F(g)_k */
            /* A COORDENADA ZERO É A MASSA. χ_0 ≡ 1, logo F(f)_0 = Σ_x f(x) = |I|,
             * o número de linhas vivas. Sem isto o bloco prendia só o PRODUTO
             * Σ_k F(f)_k·F(g)_k, e uma transformada errada podia atravessá-lo:
             * trocar k&x por k|x, por exemplo, deixa o produto INTACTO — porque
             * (k|x)⊕(k|y) = (x⊕y)&~k e ~k percorre o mesmo grupo — mas move a
             * coordenada zero, que aqui tem valor conhecido de antemão. */
            printf("      a coordenada zero: F(f)_0 = %ld (cli tem %d linhas),"
                   " F(g)_0 = %ld (ped tem %d)\n", Ff[0], 6, Fg[0], 7);
            if(Ff[0] != 6 || Fg[0] != 7) mal++;
            printf("      caminho 2 — o espectro:      2^-m · Σ F(f)·F(g) = %ld\n",
                   por_espectro);
            printf("      e o produto directo Σ f(v)·g(v) = %ld\n", por_produto);
            printf("      (valores fora do domínio de %d: %ld e %ld)\n", N, fora_f, fora_g);
        }

        /* ── OS DOIS CAMINHOS TÊM DE DAR O MESMO ─────────────────────────── */
        {
            int batem = (n_join == por_espectro) && (por_espectro == por_produto);
            printf("\n      -> %ld = %ld = %d : %s\n", por_espectro, por_produto, n_join,
                   batem ? "OS DOIS CAMINHOS CONCORDAM" : "DIVERGEM");
            if(!batem) mal++;
            /* e o número tem de ser o esperado: 2 + 3 + 1 pares */
            printf("      e o valor é o esperado (1 tem 2 pedidos, 3 tem 3, 4 tem 1): %s\n",
                   (por_espectro == 6) ? "6, sim" : "NAO");
            if(por_espectro != 6) mal++;
        }

        /* ── O REGIME DAS LINHAS APAGADAS ────────────────────────────────
         * Sem um DELETE, o ramo que salta as linhas mortas nunca corre e o
         * gume não morde lá. Apaga-se o cliente 3, que é o de fibra maior:
         * o saldo tem de CAIR de 6 para 3, nos DOIS caminhos. */
        {
            long f2[N] = {0}, g2[N] = {0}, fx = 0;
            long depois = 0;
            int nj;
            sql_executa("DELETE FROM cli WHERE id = 3", &o);
            sql_histograma("cli", "id",  f2, N, &fx);
            sql_histograma("ped", "cid", g2, N, &fx);
            for(int v = 0; v < N; v++) depois += f2[v] * g2[v];
            nj = sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o)
                 ? o.nrows : -1;
            printf("\n      APAGA-SE o cliente 3 (o de fibra 3): Σ f·g = %ld, join = %d\n",
                   depois, nj);
            printf("      -> %s\n", (depois == 3 && nj == 3)
                   ? "6 - 3 = 3 dos dois lados: a linha morta saiu do histograma"
                   : "NAO — o histograma continua a contar quem já não está lá");
            if(depois != 3 || nj != 3) mal++;
            sql_executa("INSERT INTO cli VALUES (3,30)", &o);   /* repõe */
        }

        /* ── O REGIME DE FORA DO DOMÍNIO ─────────────────────────────────
         * O espectro vive em 2^m pontos. Um valor que não caiba lá é um par
         * que a contagem directa vê e o espectro NÃO — e é por isso que o
         * `fora` se conta. Mede-se a divergência LEGÍTIMA, em vez de fingir
         * que ela não existe: com fora > 0, os dois caminhos podem discordar,
         * e o número espectral é um número certo sobre OUTRO conjunto. */
        {
            long f3[N] = {0}, g3[N] = {0}, ff = 0, fg = 0, prod = 0;
            int nj;
            sql_executa("INSERT INTO cli VALUES (300,1)", &o);
            sql_executa("INSERT INTO ped VALUES (300,99)", &o);   /* casa, e não cabe */
            sql_histograma("cli", "id",  f3, N, &ff);
            sql_histograma("ped", "cid", g3, N, &fg);
            for(int v = 0; v < N; v++) prod += f3[v] * g3[v];
            nj = sql_executa("SELECT * FROM cli JOIN ped ON cli.id = ped.cid", &o)
                 ? o.nrows : -1;
            printf("\n      UM PAR FORA DO DOMÍNIO (300, com 2^m = %d):"
                   " fora = %ld e %ld\n", N, ff, fg);
            printf("      espectro Σ f·g = %ld, join directo = %d\n", prod, nj);
            printf("      -> %s\n",
                   (ff == 1 && fg == 1 && prod == 6 && nj == 7)
                   ? "o join vê 7, o espectro vê 6, e o `fora` DIZ PORQUÊ: 300 não cabe"
                   : "NAO — a divergência não foi contada");
            if(ff != 1 || fg != 1 || prod != 6 || nj != 7) mal++;
            sql_executa("DELETE FROM cli WHERE id = 300", &o);
            sql_executa("DELETE FROM ped WHERE cid = 300", &o);
        }

        /* ── O CONTROLO: sem intersecção, o saldo tem de ser ZERO. Se o cálculo
         * espectral devolvesse sempre um número plausível, era aqui que se via. */
        {
            unlink("/tmp/pgwire_w20__z.mem");
            sql_executa("CREATE TABLE z (k,v)", &o);
            sql_executa("INSERT INTO z VALUES (100,1)", &o);
            sql_executa("INSERT INTO z VALUES (101,2)", &o);
            long h[N]; long fz = 0;
            sql_histograma("z", "k", h, N, &fz);
            long soma = 0;
            for(int v = 0; v < N; v++) soma += f[v] * h[v];
            int r = sql_executa("SELECT * FROM cli JOIN z ON cli.id = z.k", &o);
            printf("\n      CONTROLO — tabelas sem valores em comum:"
                   " Σ f·h = %ld, e o join devolve %d linhas\n",
                   soma, r ? o.nrows : -1);
            printf("      -> %s\n", (soma == 0 && r && o.nrows == 0)
                   ? "zero dos dois lados, como tem de ser"
                   : "NAO — um dos caminhos inventou pares");
            if(soma != 0 || !r || o.nrows != 0) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O TAMANHO DO JOIN LÊ-SE NO ESPECTRO, SEM CASAR UMA LINHA. O número de pares de um"
           " join de igualdade é Σ_v f(v)·g(v), com f e g os histogramas das duas colunas —"
           " e isso É a CONVOLUÇÃO AVALIADA NA ORIGEM, porque neste grupo x⊕x = 0 e a"
           " reflexão é a identidade: (f*g)(0) = Σ_x f(x)·g(x⊕0). Pelo teorema da convolução"
           " do `aranha.tex` — F(f*g) = F(f)·F(g) ponto a ponto — o mesmo número sai de"
           " 2^{-m}·Σ_k F(f)_k·F(g)_k, com os caracteres em ±1 e tudo inteiro, sem uma"
           " vírgula. SÃO DOIS CAMINHOS PARA O MESMO NÚMERO: o primeiro casa as linhas uma a"
           " uma pela árvore (§W19), o segundo NUNCA AS VISITA — transforma dois"
           " histogramas, multiplica ponto a ponto e soma. E os dois têm de dar o mesmo:"
           " seis, que é a soma das fibras (um valor com dois pedidos, outro com três, outro"
           " com um). Se o join estivesse errado, ou se a transformada estivesse errada, os"
           " números divergiam — nenhum dos dois sozinho o mostraria. E O CONTROLO é o caso"
           " em que o saldo tem de ser ZERO: duas tabelas sem um único valor em comum dão"
           " zero dos dois lados, o que impede o cálculo espectral de passar por devolver"
           " sempre um número plausível. E MEDE-SE NOS DOIS REGIMES onde"
           " o cálculo pode mentir sem que se veja. O primeiro é a LINHA APAGADA: apaga-se o"
           " cliente de fibra maior e o saldo tem de cair de seis para três dos dois lados —"
           " um histograma que contasse quem já não está lá dava o número velho e ninguém"
           " reparava. O segundo é O QUE NÃO CABE: o espectro vive em 2^m pontos, e um par"
           " cujo valor caia fora do domínio é visto pela contagem directa e NÃO pelo"
           " espectro. Aqui essa divergência é medida em vez de escondida — o join devolve"
           " sete, o espectro devolve seis, e o contador `fora` diz exactamente porquê. Um"
           " número certo sobre outro conjunto não é um número certo, e o que o declara é o"
           " `fora`, não a esperança de que todos os valores caibam. E PRENDE-SE A"
           " TRANSFORMADA, e não apenas o produto: a coordenada zero é a MASSA, porque"
           " χ_0 ≡ 1 e portanto F(f)_0 = Σ_x f(x) = |I|, o número de linhas vivas — seis e"
           " sete, sabidos de antemão. Sem essa amarra o bloco media só Σ_k F(f)_k·F(g)_k, e"
           " esse produto sobrevive a uma transformada errada: trocar k∧x por k∨x deixa-o"
           " INTACTO, porque (k∨x)⊕(k∨y) = (x⊕y)∧¬k e ¬k percorre o mesmo grupo. O que essa"
           " troca move é a coordenada zero, e é lá que agora se mede.",
           mal == 0);
    }

    /* ═══ §W21: MAX-CUT = MASSA — qual junção é a mais barata ═══════════════
     *
     * O corte de uma bipartição no grafo completo são as arestas ENTRE os dois
     * lados: C = |A|·|B|. E num join essas arestas são EXACTAMENTE os pares
     * candidatos — o produto cartesiano. Logo o max-cut é o PIOR CASO da
     * junção, e não é hipotético: com a coluna de junção constante dos dois
     * lados, o join É o produto cartesiano e devolve C linhas.
     *
     * Pelo Teorema max-cut = massa (arquitetura.tex §sec:cortar), com
     * n = |A|+|B| e δ = |A|−|B|:
     *
     *     C = (n² − δ²)/4        M = n − δ²/n        M = 4C/n
     *
     * As duas são a mesma informação em duas réguas, e o máximo é em δ = 0.
     * Aqui isso lê-se ao contrário: A JUNÇÃO EQUILIBRADA É A MAIS CARA.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W21 max-cut = massa: a junção equilibrada é a mais cara.\n\n");
    {
        const char *base = "/tmp/pgwire_w21";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w21.mem"); unlink("/tmp/pgwire_w21.prog");
        if(!sql_abrir(base)) mal++;

        /* ── A IDENTIDADE, EM INTEIROS ────────────────────────────────────
         * Escreve-se M·n em vez de M para não trazer uma vírgula que os dados
         * nunca tiveram: M·n = n² − δ², e 4C = 4|A||B|. A identidade é exacta
         * e varre-se em TODAS as partições, não numa. */
        {
            int falhou = 0, argmax = -1; long cmax = -1;
            const int n = 8;
            printf("      n = %d — todas as partições:\n", n);
            printf("      |A| |B|  δ   C=|A||B|   M·n = n²−δ²   4C\n");
            for(int na = 0; na <= n; na++){
                int nb = n - na, d = na - nb;
                long C = (long)na * nb;
                long Mn = (long)n*n - (long)d*d;
                if(Mn != 4*C) falhou = 1;
                if(C > cmax){ cmax = C; argmax = d; }
                printf("       %2d  %2d %3d %8ld %12ld %6ld%s\n",
                       na, nb, d, C, Mn, 4*C, (d == 0) ? "   <- δ=0" : "");
            }
            printf("      -> a identidade M·n = 4C vale em TODAS as partições: %s\n",
                   falhou ? "NAO" : "sim");
            printf("      -> o máximo está em δ = %d (C = %ld): %s\n", argmax, cmax,
                   (argmax == 0) ? "o equilíbrio, como o teorema diz" : "NAO");
            if(falhou || argmax != 0) mal++;
        }

        /* ── O TECTO É ATINGIDO: o join com a coluna constante É o produto ──
         * Duas junções candidatas, com o MESMO n = 8 e desequilíbrios opostos.
         * A coluna de junção é constante dos dois lados, portanto TODAS as
         * arestas do corte estão lá e o join devolve C linhas. */
        long C_eq = 0, C_des = 0;
        int lin_eq = 0, lin_des = 0;
        {
            /* a equilibrada: 4 e 4, δ = 0 */
            sql_executa("CREATE TABLE a4 (k,v)", &o);
            for(int i = 0; i < 4; i++){ char q[64];
                snprintf(q, sizeof q, "INSERT INTO a4 VALUES (7,%d)", i); sql_executa(q,&o); }
            sql_executa("CREATE TABLE b4 (k,v)", &o);
            for(int i = 0; i < 4; i++){ char q[64];
                snprintf(q, sizeof q, "INSERT INTO b4 VALUES (7,%d)", 100+i); sql_executa(q,&o); }
            /* a desequilibrada: 1 e 7, δ = 6 — o MESMO n */
            sql_executa("CREATE TABLE a1 (k,v)", &o);
            sql_executa("INSERT INTO a1 VALUES (7,0)", &o);
            sql_executa("CREATE TABLE b7 (k,v)", &o);
            for(int i = 0; i < 7; i++){ char q[64];
                snprintf(q, sizeof q, "INSERT INTO b7 VALUES (7,%d)", 200+i); sql_executa(q,&o); }

            C_eq  = 4L * 4;   C_des = 1L * 7;
            lin_eq  = sql_executa("SELECT * FROM a4 JOIN b4 ON a4.k = b4.k", &o) ? o.nrows : -1;
            lin_des = sql_executa("SELECT * FROM a1 JOIN b7 ON a1.k = b7.k", &o) ? o.nrows : -1;
            printf("\n      a EQUILIBRADA  (4,4) δ=0: C = %2ld, e o join devolve %2d\n",
                   C_eq, lin_eq);
            printf("      a DESEQUILIBRADA (1,7) δ=6: C = %2ld, e o join devolve %2d\n",
                   C_des, lin_des);
            printf("      -> %s\n", (lin_eq == C_eq && lin_des == C_des)
                   ? "o tecto do corte é ATINGIDO nos dois: as arestas são os pares"
                   : "NAO — o join não devolveu o corte");
            if(lin_eq != C_eq || lin_des != C_des) mal++;
        }

        /* ── A DECISÃO, E O QUE A MASSA GOVERNA ─────────────────────────
         * Caminho 1: a álgebra escolhe só de |A| e |B|, sem tocar nos dados.
         * Caminho 2: a máquina emite as linhas e conta-as.
         * Como M·n = 4C, as duas razões têm de ser a MESMA — e compara-se em
         * CRUZ, com inteiros, sem dividir. */
        {
            long Mn_eq = 8L*8 - 0L*0, Mn_des = 8L*8 - 6L*6;
            printf("\n      caminho 1 — a ÁLGEBRA, sem tocar nos dados:"
                   " M·n = %ld e %ld\n", Mn_eq, Mn_des);
            printf("      caminho 2 — a MÁQUINA, linhas emitidas:       %d e %d\n",
                   lin_eq, lin_des);
            printf("      em cruz: %ld·%d = %ld  e  %ld·%d = %ld\n",
                   Mn_eq, lin_des, Mn_eq*lin_des, Mn_des, lin_eq, Mn_des*lin_eq);
            { int batem = (Mn_eq * lin_des == Mn_des * lin_eq) && (Mn_eq > Mn_des);
              printf("      -> %s\n", batem
                     ? "a MESMA razão, e a equilibrada é a mais cara: a massa decide"
                       " sem ler as linhas"
                     : "DIVERGEM");
              if(!batem) mal++; }
        }

        /* ── O ACHADO: A MASSA NÃO GOVERNA AS ESCRITAS, E ISSO MEDE-SE ────
         * Esperava-se que a junção mais cara em massa escrevesse mais. A
         * medição diz o CONTRÁRIO, e o motivo estava no próprio controlo: a
         * massa é SIMÉTRICA e as escritas não são. O que escreve é a FASE 1,
         * que copia a tabela da DIREITA para o banco — logo o custo de
         * construção é |direita|, e a direita da desequilibrada (7) é maior do
         * que a da equilibrada (4). Duas grandezas, e a massa governa uma:
         *
         *     a SAÍDA        C = |A|·|B|      simétrica   <- a massa
         *     a CONSTRUÇÃO   |direita|        assimétrica <- o tamanho do lado
         *
         * Não se corrige o número para a teoria: mede-se a segunda lei também,
         * e diz-se qual delas cada uma decide. */
        {
            long e_eq = 0, e_des = 0, e_troca = 0;
            sql_tx_abre();
            sql_executa("SELECT * FROM a4 JOIN b4 ON a4.k = b4.k", &o);
            e_eq = sql_tx_escritas();
            if(!sql_tx_desfaz()) mal++;
            sql_tx_fecha();
            sql_tx_abre();
            sql_executa("SELECT * FROM a1 JOIN b7 ON a1.k = b7.k", &o);
            e_des = sql_tx_escritas();
            if(!sql_tx_desfaz()) mal++;
            sql_tx_fecha();
            sql_tx_abre();
            sql_executa("SELECT * FROM b7 JOIN a1 ON b7.k = a1.k", &o);
            e_troca = sql_tx_escritas();
            if(!sql_tx_desfaz()) mal++;
            sql_tx_fecha();

            printf("\n      as ESCRITAS reais, pela pilha do levantamento:\n");
            printf("         a4 JOIN b4 (direita = 4 linhas): %ld\n", e_eq);
            printf("         a1 JOIN b7 (direita = 7 linhas): %ld  <- MAIS, e a massa"
                   " é MENOR\n", e_des);
            printf("         b7 JOIN a1 (direita = 1 linha):  %ld  <- a MESMA junção,"
                   " a massa não mudou\n", e_troca);
            printf("      -> %s\n",
                   (e_des > e_eq && e_troca < e_des)
                   ? "a massa NÃO governa as escritas: quem as governa é |direita|"
                   : "NAO — as escritas não seguem o tamanho da direita");
            if(!(e_des > e_eq && e_troca < e_des)) mal++;
            printf("      -> e a MESMA junção pelas duas ordens escreve %ld ou %ld:"
                   " a ordem decide-se pelo LADO, não pela massa\n", e_des, e_troca);
        }

        /* O CONTROLO: se a massa escolhesse sempre a primeira, ou sempre a
         * maior tabela, isto passava à mesma. Inverte-se o par — a mesma
         * junção desequilibrada com os lados TROCADOS tem a MESMA massa,
         * porque C = |A||B| é simétrico, e o RESULTADO também não muda. */
        {
            long Mn_troca = 8L*8 - (-6L)*(-6L), Mn_des = 8L*8 - 6L*6;
            int nl = sql_executa("SELECT * FROM b7 JOIN a1 ON b7.k = a1.k", &o)
                     ? o.nrows : -1;
            printf("\n      CONTROLO — a MESMA junção com os lados trocados:"
                   " M·n = %ld (era %ld), e devolve %d linhas (eram %d)\n",
                   Mn_troca, Mn_des, nl, lin_des);
            printf("      -> %s\n", (Mn_troca == Mn_des && nl == lin_des)
                   ? "iguais: a massa é simétrica, logo escolhe a JUNÇÃO e não a ORDEM"
                   : "NAO — a massa ou o resultado mudaram com a troca, e não podiam");
            if(Mn_troca != Mn_des || nl != lin_des) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("O MAX-CUT É O PIOR CASO DA JUNÇÃO, E A MASSA GOVERNA A SAÍDA — NÃO A CONSTRUÇÃO."
           " As arestas que uma bipartição corta no grafo completo são C = |A|·|B|, e num join"
           " essas arestas são EXACTAMENTE os pares candidatos: o produto cartesiano. O"
           " max-cut deixa por isso de ser um problema de grafos ao lado — é o tecto do"
           " trabalho da junção, e é ATINGIDO: com a coluna de junção constante dos dois"
           " lados, o join devolve as C linhas, medido nos dois casos. Pelo Teorema max-cut ="
           " massa (arquitetura.tex §sec:cortar), com n = |A|+|B| e δ = |A|−|B|, tem-se"
           " C = (n²−δ²)/4 e M = 4C/n, e escreve-se M·n = n² − δ² para não trazer uma vírgula"
           " que os dados nunca tiveram: a identidade é INTEIRA e varre-se em TODAS as"
           " partições de n, não numa. O máximo está em δ = 0, e neste domínio isso lê-se ao"
           " contrário do costume: A JUNÇÃO EQUILIBRADA É A MAIS CARA. Daí a decisão por DOIS"
           " CAMINHOS — a álgebra escolhe só de |A| e |B|, sem tocar nos dados; a máquina"
           " emite e conta —, e as duas razões batem em CRUZ, com inteiros e sem dividir."
           " E AQUI A MEDIÇÃO DERRUBOU A PREVISÃO, o que fica escrito porque é o resultado:"
           " esperava-se que a junção mais cara em massa ESCREVESSE mais, e ela escreve"
           " MENOS. O motivo estava no próprio controlo — a massa é SIMÉTRICA e as escritas"
           " não são. Quem escreve é a fase que copia a tabela da DIREITA para o banco, logo"
           " o custo de construção é |direita|, e a direita da desequilibrada é maior. São"
           " duas grandezas e a massa governa uma: a SAÍDA (C = |A|·|B|, simétrica) e a"
           " CONSTRUÇÃO (|direita|, assimétrica). Mede-se a segunda também, com a MESMA junção"
           " pelas duas ordens a escrever números diferentes — a massa escolhe QUAL junção"
           " fazer, e o tamanho do lado escolhe por que ORDEM fazê-la. Um planeador de"
           " consultas decide as duas coisas à mão, por heurística; aqui a primeira sai do"
           " teorema e a segunda sai medida. O CONTROLO é o que separa as duas: trocar os"
           " lados não muda a massa nem o resultado, e sem ele a asserção passaria mesmo que"
           " a regra fosse «escolhe sempre a primeira».",
           mal == 0);
    }

    /* ═══ §W22: O CONTADOR SOBE DE ANDAR ═══════════════════════════════════
     *
     * `Word` é {Word8 total, e} com `Word8` = uint8_t: DOIS bytes. Ler
     * `mem_le(slot).total` é ler METADE do número, e o motor fazia-o em cinco
     * sítios que guardam ENDEREÇO, CONTADOR ou ÍNDICE. O mais caro era o
     * nrows: vivia no campo `.e` do catálogo e subia com OP_ADD componente a
     * componente, de modo que à linha 256 dava a volta — 300 linhas inseridas
     * respondiam 44 (300 mod 256) ao SELECT, ao ORDER BY e ao count.
     *
     * A CORRECÇÃO É A LEI DA CASA, e não um tecto: `arquitetura.tex
     * §sec:torre` fixa a subida — T_{k+1} = T_k + T_k*, d_{k+1} = 2·d_k —, e
     * diz que «o que cresce é o OBJECTO, não a máquina». O `thm:BI` do
     * `aranha.tex` mostra a cadeia {0,1} ⊂ {0..3} ⊂ {0..15} ⊂ {0..255} com «a
     * dobra a duplicar a largura»: é uma enumeração de andares, não um fim. E
     * o `word_isa.h` diz o que fazer com o que não cabe — «coef. que crescem
     * SOBEM A TORRE».
     *
     * Por isso o nrows SAI do par do catálogo, onde o transporte não pode
     * atravessar do nrows para o ncols, e passa a ocupar os dois componentes
     * de um slot só seu, somado com OP_ADD16. O mesmo para o contador do
     * count. Não se pôs tecto nenhum: quem não cabe promove.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W22 a palavra é UM BIT: a linha é a coordenada, e a largura é argumento.\n\n");
    {
        SqlOut o;
        long mal = 0;
        char q[96];
        const int N = 300;                 /* atravessa a fronteira do byte, e chega:
                                            * medir isto com mil INSERT era arder o
                                            * processador para provar o mesmo */
        unlink("/tmp/pgwire_w22.mem"); unlink("/tmp/pgwire_w22.prog");
        if(!sql_abrir("/tmp/pgwire_w22")) mal++;
        sql_executa("CREATE TABLE g (a,b)", &o);
        for(int i = 1; i <= N; i++){
            snprintf(q, sizeof q, "INSERT INTO g VALUES (%d,7)", i);
            if(!sql_executa(q, &o)) mal++;
        }
        {
            int r = sql_executa("SELECT count(*) FROM g", &o);
            long c = (r && o.nrows) ? strtol(o.cell[0][0], NULL, 10) : -1;
            printf("      %d linhas inseridas, count(*) = %ld\n", N, c);
            printf("      (num contador de UM byte, %d daria %d)\n", N, N % 256);
            printf("      -> %s\n", (c == N) ? "o contador atravessou os 255"
                                             : "NAO — deu a volta");
            if(c != N) mal++;
        }
        {
            int r = sql_executa("SELECT count(*) FROM g WHERE b = 7", &o);
            long c = (r && o.nrows) ? strtol(o.cell[0][0], NULL, 10) : -1;
            printf("      count(*) WHERE b = 7 = %ld — a fibra inteira\n", c);
            if(c != N) mal++;
        }
        sql_fechar();

        /* O CONTROLO: abaixo da fronteira nada mudou. São as tabelas pequenas
         * que todos os outros blocos deste ficheiro usam, e uma subida de andar
         * que as quebrasse passaria despercebida acima. */
        {
            unlink("/tmp/pgwire_w22b.mem"); unlink("/tmp/pgwire_w22b.prog");
            if(!sql_abrir("/tmp/pgwire_w22b")) mal++;
            sql_executa("CREATE TABLE p (a,b)", &o);
            for(int i = 1; i <= 10; i++){
                snprintf(q, sizeof q, "INSERT INTO p VALUES (%d,%d)", i, i*3);
                sql_executa(q, &o);
            }
            sql_executa("SELECT count(*) FROM p", &o);
            long c = o.nrows ? strtol(o.cell[0][0], NULL, 10) : -1;
            int r = sql_executa("SELECT a FROM p ORDER BY a DESC", &o);
            int bem = (c == 10) && r && o.nrows == 10 && !strcmp(o.cell[0][0], "10");
            printf("\n      CONTROLO — dez linhas: count = %ld, ORDER BY DESC dá %d,"
                   " a primeira %s  -> %s\n", c, r ? o.nrows : -1,
                   (r && o.nrows) ? o.cell[0][0] : "?",
                   bem ? "as pequenas continuam intactas" : "NAO");
            if(!bem) mal++;
            sql_fechar();
        }

        printf("\n");
        ok("A PALAVRA É UM BIT, E A LINHA É A COORDENADA. `naturais.tex thm:base`: os produtos"
           " dos geradores dão e_k = 2^k, e essa base é ORTONORMAL para ⟨a,b⟩ = paridade(a∧b),"
           " com ⟨e_i,e_j⟩ = δ_ij; e o `cor:w8`: «a identificação é a IDENTIDADE — o bit j do"
           " inteiro é a COORDENADA j na base». Um bitmap não gasta por isso um slot por linha:"
           " a linha i É a coordenada i, e lê-se como o bit i. Era um slot inteiro por linha —"
           " dezasseis vezes o espaço necessário — e foi daí que saiu o falso tecto que eu"
           " cheguei a escrever como lei. Marcar passou a ser LIGAR a coordenada (slot |= e_k),"
           " apagar a DESLIGÁ-LA (slot &= ¬e_k), e o vivo isola-se com AND e_k e testa-se com"
           " salto próprio — porque a base é ortonormal e cruzar e_k com um acumulador de zero"
           " ou um só sobreviveria na coordenada zero. E A LARGURA É ARGUMENTO, que é a outra"
           " metade: o `lib/largura.h` escreve «UMA LEI PARA TODA A ESCADA, com w PARÂMETRO —"
           " seis andares, um corpo» e separa «o VEÍCULO» do «PARÂMETRO». Aqui estavam `i >> 4`,"
           " `i & 15` e `k < 8`, três palavras fixas a dizer o mesmo número de três maneiras;"
           " agora o átomo diz quantos bits tem, a Word diz quantos átomos tem, e tudo o resto"
           " deriva — se o andar dobrar, dobra sozinho. O mesmo na árvore que ordena, onde"
           " `ORD_LARG 16` convivia com um `4` e um `15` escritos à mão. MEDE-SE ATRAVESSANDO A FRONTEIRA"
           " dos 255 — que é a única maneira de o ver, e não precisa de mais: o count(*) dá"
           " trezentos, com e sem WHERE, onde um contador de um byte daria quarenta e quatro. O CONTROLO é a"
           " tabela de dez linhas, porque são as pequenas que todos os outros blocos deste"
           " ficheiro usam. E fica dito o que é da MÁQUINA e não da teoria: o mapa de slots"
           " ainda tem zonas com capacidades diferentes entre si, e o compilador passa a"
           " recusar se a base não couber no espaço que o mapa lhe deu, em vez de escrever por"
           " cima do vizinho.",
           mal == 0);
    }

    /* ═══ §W23: AS CLÁUSULAS SÃO A TEORIA, E FECHAM ENTRE SI ════════════════
     *
     * Nenhuma das cláusulas que o SELECT ganhou foi acrescentada por ser SQL:
     * GROUP BY é a FIBRA, o count é G(x), HAVING é o WHERE sobre G, DISTINCT é
     * o representante k=1, LIMIT é o prefixo da lista, e LEFT/RIGHT/FULL são a
     * fibra VAZIA de cada lado. Este bloco não as verifica uma a uma contra
     * números escritos à mão — verifica que FECHAM ENTRE SI, que é o que uma
     * teoria faz e uma lista de funcionalidades não.
     * ───────────────────────────────────────────────────────────────────────── */
    printf("\n§W23 as cláusulas fecham entre si: a conservação e a tricotomia.\n\n");
    {
        SqlOut o;
        long mal = 0;
        char q[96];
        unlink("/tmp/pgw23.mem"); unlink("/tmp/pgw23.prog");
        unlink("/tmp/pgw23__t.mem"); unlink("/tmp/pgw23__u.mem");
        if(!sql_abrir("/tmp/pgw23")) mal++;

        /* uma tabela com fibras conhecidas: 7 três vezes, 9 duas, 5 uma */
        sql_executa("CREATE TABLE t (a,b)", &o);
        { const int A[] = {7,9,7,5,7,9}, B[] = {5,2,8,4,3,6};
          for(unsigned i = 0; i < sizeof A / sizeof A[0]; i++){
              snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", A[i], B[i]);
              sql_executa(q, &o);
          } }

        /* ── A CONSERVAÇÃO: ∑G = |I| ──────────────────────────────────────
         * O GROUP BY parte a tabela em fibras. A soma dos tamanhos tem de ser
         * o número de linhas — é o Lema da conservação, e é o primeiro dos dois
         * caminhos: o campo contado por fibra contra o campo contado inteiro. */
        long total = 0, soma_fibras = 0, ngrupos = 0;
        {
            sql_executa("SELECT count(*) FROM t", &o);
            total = o.nrows ? strtol(o.cell[0][0], NULL, 10) : -1;
            sql_executa("SELECT * FROM t GROUP BY a", &o);
            ngrupos = o.nrows;
            for(int r = 0; r < o.nrows; r++) soma_fibras += strtol(o.cell[r][1], NULL, 10);
            printf("      %ld linhas, %ld fibras, ∑G = %ld\n", total, ngrupos, soma_fibras);
            printf("      -> %s\n", (soma_fibras == total)
                   ? "∑G = |I|: a conservação fecha"
                   : "NAO — a soma das fibras não é o número de linhas");
            if(soma_fibras != total || ngrupos != 3) mal++;
        }

        /* ── HAVING PARTE AS FIBRAS EM DUAS, E AS DUAS SOMAM ──────────────
         * G > 1 são as dobradas, G = 1 são aquelas onde π é injetiva. Não há
         * terceira: toda fibra tem G ≥ 1. Logo as duas contagens somam. */
        {
            long dob = 0, inj = 0;
            sql_executa("SELECT * FROM t GROUP BY a HAVING count(*) > 1", &o);
            dob = o.nrows;
            sql_executa("SELECT * FROM t GROUP BY a HAVING count(*) = 1", &o);
            inj = o.nrows;
            printf("      HAVING > 1: %ld fibra(s) dobradas · = 1: %ld injetivas"
                   "  (total %ld)\n", dob, inj, ngrupos);
            printf("      -> %s\n", (dob + inj == ngrupos)
                   ? "partem as fibras em duas e não há terceira"
                   : "NAO — as duas metades não somam");
            if(dob + inj != ngrupos) mal++;
        }

        /* ── DISTINCT DÁ UM POR FIBRA ────────────────────────────────────
         * O representante k=1 existe uma vez em cada fibra (thm:levantamento
         * (3): os k são {1,…,G(x)}), logo contá-los é contar as fibras. Dois
         * caminhos para o mesmo número, por sítios diferentes do motor. */
        {
            int r = sql_executa("SELECT DISTINCT a FROM t", &o);
            printf("      DISTINCT dá %d linha(s); o GROUP BY deu %ld fibras\n",
                   r ? o.nrows : -1, ngrupos);
            printf("      -> %s\n", (r && o.nrows == ngrupos)
                   ? "um representante por fibra, pelos dois caminhos"
                   : "NAO");
            if(!r || o.nrows != ngrupos) mal++;
        }

        /* ── AS AGREGAÇÕES LÊEM A FIBRA ──────────────────────────────────
         * sum, max e min sobre a fibra de 7, cujos b são {5,8,3}. E o gume: a
         * soma tem de ser MAIOR que o máximo, senão qualquer uma passaria por
         * qualquer uma numa fibra de um elemento só. */
        {
            long sm = 0, mx = 0, mn = 0;
            sql_executa("SELECT sum(b) FROM t GROUP BY a", &o);
            for(int r = 0; r < o.nrows; r++)
                if(strtol(o.cell[r][0], NULL, 10) == 7) sm = strtol(o.cell[r][2], NULL, 10);
            sql_executa("SELECT max(b) FROM t GROUP BY a", &o);
            for(int r = 0; r < o.nrows; r++)
                if(strtol(o.cell[r][0], NULL, 10) == 7) mx = strtol(o.cell[r][2], NULL, 10);
            sql_executa("SELECT min(b) FROM t GROUP BY a", &o);
            for(int r = 0; r < o.nrows; r++)
                if(strtol(o.cell[r][0], NULL, 10) == 7) mn = strtol(o.cell[r][2], NULL, 10);
            printf("      a fibra de 7 tem b = {5,8,3}: sum=%ld max=%ld min=%ld\n", sm, mx, mn);
            printf("      -> %s\n", (sm == 16 && mx == 8 && mn == 3 && sm > mx && mn < mx)
                   ? "as três lêem a fibra, e a soma é maior que o máximo"
                   : "NAO");
            if(sm != 16 || mx != 8 || mn != 3) mal++;
        }

        /* ── LIMIT É O PREFIXO ───────────────────────────────────────────── */
        {
            sql_executa("SELECT * FROM t LIMIT 2", &o);
            int n2 = o.nrows;
            sql_executa("SELECT * FROM t LIMIT 99", &o);
            int n99 = o.nrows;
            printf("      LIMIT 2 dá %d; LIMIT 99 (acima do total) dá %d\n", n2, n99);
            printf("      -> %s\n", (n2 == 2 && n99 == total)
                   ? "o prefixo, e um prefixo maior que a lista é a lista"
                   : "NAO");
            if(n2 != 2 || n99 != total) mal++;
        }

        /* ── A TRICOTOMIA: <, = e > PARTEM O PRODUTO ─────────────────────
         *
         * Para cada par (x,y) vale exactamente uma de x<y, x=y, x>y. Logo os
         * três joins somam |A|·|B| — que é o C do max-cut, o tecto do trabalho
         * da junção (§W21). É o «dois caminhos» outra vez: a álgebra dá o
         * produto sem olhar às linhas, e as três junções dão-no somando. */
        {
            sql_executa("CREATE TABLE u (x)", &o);
            for(int v = 10; v <= 30; v += 10){
                snprintf(q, sizeof q, "INSERT INTO u VALUES (%d)", v); sql_executa(q, &o);
            }
            sql_executa("CREATE TABLE w (y)", &o);
            { const int W[] = {10, 25}; 
              for(unsigned i = 0; i < sizeof W / sizeof W[0]; i++){
                  snprintf(q, sizeof q, "INSERT INTO w VALUES (%d)", W[i]);
                  sql_executa(q, &o);
              } }
            long lt = 0, eq = 0, gt = 0;
            sql_executa("SELECT * FROM u JOIN w ON u.x < w.y", &o); lt = o.nrows;
            sql_executa("SELECT * FROM u JOIN w ON u.x = w.y", &o); eq = o.nrows;
            sql_executa("SELECT * FROM u JOIN w ON u.x > w.y", &o); gt = o.nrows;
            printf("\n      3 × 2 = 6 pares:  < dá %ld,  = dá %ld,  > dá %ld,  soma %ld\n",
                   lt, eq, gt, lt + eq + gt);
            printf("      -> %s\n", (lt + eq + gt == 6)
                   ? "a tricotomia fecha: os três partem o produto |A|·|B|"
                   : "NAO — sobram ou faltam pares");
            if(lt + eq + gt != 6) mal++;
        }

        /* ── AS FIBRAS VAZIAS: FULL = interno + só-esquerda + só-direita ── */
        {
            long in = 0, le = 0, ri = 0, fu = 0;
            sql_executa("SELECT * FROM u JOIN w ON u.x = w.y", &o);       in = o.nrows;
            sql_executa("SELECT * FROM u LEFT JOIN w ON u.x = w.y", &o);  le = o.nrows;
            sql_executa("SELECT * FROM u RIGHT JOIN w ON u.x = w.y", &o); ri = o.nrows;
            sql_executa("SELECT * FROM u FULL JOIN w ON u.x = w.y", &o);  fu = o.nrows;
            printf("      interno %ld · LEFT %ld · RIGHT %ld · FULL %ld\n", in, le, ri, fu);
            printf("      -> %s\n", (fu == le + ri - in)
                   ? "FULL = LEFT + RIGHT − interno: as duas fibras vazias, sem contar o meio duas vezes"
                   : "NAO");
            if(fu != le + ri - in) mal++;
        }

        /* ── O CONTROLO: sem fibra vazia, os quatro coincidem ─────────────
         * Se as três formas devolvessem sempre o mesmo, tudo acima passava. */
        {
            sql_executa("DELETE FROM w WHERE y = 25", &o);
            sql_executa("INSERT INTO w VALUES (20)", &o);
            long in = 0, fu = 0;
            sql_executa("SELECT * FROM u JOIN w ON u.x = w.y", &o);      in = o.nrows;
            sql_executa("SELECT * FROM u FULL JOIN w ON u.x = w.y", &o); fu = o.nrows;
            printf("\n      CONTROLO — a direita agora cabe toda na esquerda:"
                   " interno %ld, FULL %ld\n", in, fu);
            printf("      -> %s\n", (fu == in + 1)
                   ? "o FULL só acrescenta a fibra vazia que sobra, e ela é uma"
                   : "NAO — as formas não se distinguem, ou distinguem a mais");
            if(fu != in + 1) mal++;
        }
        sql_fechar();

        printf("\n");
        ok("AS CLÁUSULAS SÃO A TEORIA, E O QUE SE MEDE É FECHAREM ENTRE SI. Nenhuma foi"
           " acrescentada por ser SQL: GROUP BY é a FIBRA e o count é G(x), HAVING é o WHERE"
           " sobre G, DISTINCT é o representante canónico k=1, LIMIT é o prefixo da lista, e"
           " LEFT/RIGHT/FULL são a fibra VAZIA de cada lado. Por isso não se verificam contra"
           " números escritos à mão — verifica-se que as identidades fecham. A CONSERVAÇÃO: a"
           " soma dos tamanhos das fibras é o número de linhas, ∑G = |I|, com o campo contado"
           " por fibra contra o campo contado inteiro. O HAVING PARTE EM DUAS: G > 1 são as"
           " dobradas, G = 1 são as injetivas, e as duas contagens somam as fibras todas —"
           " não há terceira, porque toda fibra tem G ≥ 1. O DISTINCT dá exactamente uma"
           " linha por fibra, porque o representante k=1 existe uma vez em cada (os k são"
           " {1,…,G(x)}), e esse número sai por outro sítio do motor que o do GROUP BY. AS"
           " AGREGAÇÕES lêem a fibra: sobre {5,8,3} dão 16, 8 e 3, com o gume de a soma ter"
           " de ser maior que o máximo — numa fibra de um elemento só as três coincidiriam e"
           " qualquer uma passaria por qualquer uma. E A TRICOTOMIA É O FECHO MAIS BONITO:"
           " para cada par vale exactamente uma de x<y, x=y, x>y, logo as três junções somam"
           " |A|·|B| — que é o C do max-cut, o tecto do trabalho da junção. A álgebra dá o"
           " produto sem olhar às linhas e as três junções dão-no somando. As fibras vazias"
           " fecham da mesma maneira: FULL = LEFT + RIGHT − interno, sem contar o meio duas"
           " vezes. E O CONTROLO impede que tudo isto passe por as formas não se distinguirem:"
           " com a direita contida na esquerda, o FULL acrescenta ao interno exactamente uma"
           " linha, que é a única fibra vazia que sobra.",
           mal == 0);
    }

    /* ═══ §W24: AS QUATRO FACES NUM PASSO, E O CUSTO QUE NÃO OLHA PARA O DADO ══
     *
     * O molde de cada linha eram DOIS saltos condicionais: testar o vivo e
     * saltar, testar o acumulador e saltar, marcar. Saltar é executar no TEMPO —
     * a máquina ramifica, e o caminho que percorre depende do que leu. O par
     * ⊗/⊘ do algoritmo não ramifica: opera. Rodando no ESPAÇO DE FASES, a linha
     * i deixa de ser um instante e passa a ser a COORDENADA e_i, e as quatro
     * faces executam num único passo — 0 a realização (a coordenada), × a
     * convolução (o AND), + a conservação (o OR), 1 a deconvolução (o ∑ do
     * popcount, que é quem responde ao count).
     *
     * ISTO NÃO SE MEDE PELO RESULTADO, porque o resultado não mudou — mede-se
     * pelo CUSTO. Sem ramificação, o número de passos depende do TAMANHO da
     * tabela e não do CONTEÚDO dela: uma condição que casa TUDO e uma que não
     * casa NADA têm de custar EXACTAMENTE o mesmo. Com os saltos custavam
     * diferente, e era isso que dizia que o motor olhava para o dado antes de
     * decidir por onde ir. */
    {
        const char *base = "/tmp/pgwire_w24";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w24.mem");
        unlink("/tmp/pgwire_w24.prog");
        printf("\n§W24 as quatro faces num passo: o custo não olha para o dado.\n\n");
        if(!sql_abrir(base)) mal++;

        /* A CONSULTA TEM DE SER A MESMA, E À PRIMEIRA NÃO ERA.
         *
         * Comparei `a > 0`, `a > 999` e `a > 10` e obtive 1660, 1920 e 2930 —
         * três números diferentes, e a asserção caiu. O defeito era meu: a
         * constante COMPILA, em Zeckendorf, pelo que três constantes dão três
         * PROGRAMAS distintos. Mudei o dado e a chave ao mesmo tempo, e o que
         * medi foi a chave. A mesma condição, sobre tabelas do MESMO tamanho e
         * conteúdo DIFERENTE, é a única forma de o programa ser o mesmo. */
        struct { const char *t; int de; const char *rot; int esp; } casos[] = {
            { "fa", 11, "casa TUDO  ", 20 },   /* 11..30: todos > 10 */
            { "fb",  1, "casa METADE", 10 },   /*  1..20: metade     */
            { "fc", -1, "casa NADA  ",  0 },   /*  1..10 repetidos   */
        };
        long passos[3], linhas[3];
        for(int k = 0; k < 3; k++){
            char q[96];
            snprintf(q, sizeof q, "CREATE TABLE %s (a,b)", casos[k].t);
            sql_executa(q, &o);
            for(int i = 0; i < 20; i++){
                int v = casos[k].de < 0 ? (i % 10) + 1 : casos[k].de + i;
                snprintf(q, sizeof q, "INSERT INTO %s VALUES (%d,%d)", casos[k].t, v, v * 3);
                sql_executa(q, &o);
            }
            snprintf(q, sizeof q, "SELECT * FROM %s WHERE a > 10", casos[k].t);
            sql_executa(q, &o);
            passos[k] = sql_ultimos_passos; linhas[k] = o.nrows;
            printf("      %s (a > 10, 20 linhas): %2ld casam, %ld passos\n",
                   casos[k].rot, linhas[k], passos[k]);
        }
        /* E AQUI A MEDIÇÃO CORRIGIU A AFIRMAÇÃO, que é o resultado deste bloco.
         *
         * Esperava três números IGUAIS e saíram 2960, 2930 e 2900 — não iguais,
         * mas em progressão exacta: 30 passos por cada dez linhas que casam. A
         * dependência do dado não desapareceu; encolheu para TRÊS passos por
         * casamento, e não vem do molde — vem do AVALIADOR da condição, que
         * ainda ramifica com JZ/JNZ curtos. O molde é a parte que deixou de
         * olhar. Afirma-se por isso a lei exacta, que é falsificável:
         *
         *      passos = c·|X| + 3·|I|
         *
         * com |X| as linhas percorridas e |I| as que casam. Um molde que ainda
         * ramificasse dava um coeficiente maior e variável; um que não tivesse
         * fase de avaliação dava 3 = 0. */
        long p_base = passos[2];                    /* o caso em que nada casa */
        int lei = 1;
        for(int k = 0; k < 3; k++)
            if(passos[k] != p_base) lei = 0;
        printf("      -> passos = %ld nos TRÊS, sem termo em |I|: %s\n",
               p_base, lei ? "sim" : "NAO");
        if(!lei) mal++;

        /* e o CONTROLO: se o custo não mudasse NUNCA, a asserção de cima passava
         * por o número ser fixo. Ele tem de mudar com o TAMANHO — é o Θ(|X|) do
         * §sec:algoritmo, o que a varredura percorre. */
        sql_executa("CREATE TABLE f2 (a,b)", &o);
        for(int i = 0; i < 40; i++){
            char q[80];
            snprintf(q, sizeof q, "INSERT INTO f2 VALUES (%d,%d)", 11 + i, i * 3);
            sql_executa(q, &o);
        }
        sql_executa("SELECT * FROM f2 WHERE a > 10", &o);
        long p_grande = sql_ultimos_passos;
        /* o dobro das linhas dobra a BASE — e é a base que se compara, porque o
         * termo 3·|I| também dobrou. Sem isto, «cresce» passaria por qualquer
         * crescimento, e o que se afirma é que ele é o Θ(|X|) da varredura. */
        printf("\n      CONTROLO — o dobro das linhas: %ld contra %ld  %s\n",
               p_grande, p_base, p_grande == 2 * p_base ? "(o dobro exacto: Θ(|X|))" : "NAO");
        if(p_grande != 2 * p_base) mal++;

        /* e as três respostas continuam certas — o custo constante não pode ser
         * comprado com um resultado errado */
        int res = (linhas[0] == 20 && linhas[1] == 10 && linhas[2] == 0);
        printf("      e as respostas: 20, 10, 0 — %s\n", res ? "certas" : "NAO");
        if(!res) mal++;
        sql_fechar();

        printf("\n");
        ok("AS QUATRO FACES CORREM NUM PASSO SÓ, E MEDE-SE PELO CUSTO. O molde de cada linha"
           " eram DOIS saltos condicionais — testar o vivo e saltar, testar o acumulador e"
           " saltar, marcar. Saltar é executar no TEMPO: a máquina ramifica, e o caminho que"
           " percorre depende do que leu. O par ⊗/⊘ do algoritmo não ramifica, OPERA; rodando"
           " no ESPAÇO DE FASES a linha i deixa de ser um instante e passa a ser a COORDENADA"
           " e_i, e as quatro faces executam simultaneamente: 0 a realização, que é a"
           " coordenada; × a convolução, que é o AND do vivo com a condição; + a conservação,"
           " que é o OR que escreve sem apagar; e 1 a deconvolução, que é o ∑ do popcount a"
           " devolver a contagem. O que dispensa o salto é `0 − ACC`: com o acumulador em"
           " {0,1} a subtracção no anel dá a máscara INTEIRA ou zero — o booleano espalhado"
           " por todos os bits — e o AND com a coordenada devolve e_i ou nada. E TEM DE SER O"
           " SUB16: o OP_SUB opera componente a componente, porque a Word é um PAR, e por isso"
           " enchia só o átomo BAIXO — de 84 linhas marcavam-se 44, que são exactamente as de"
           " índice `i mod 16 < 8`. A metade de cima do campo não existia, e foi o número 44 a"
           " dizer qual era o defeito. ISTO NÃO SE MEDE PELO RESULTADO, que não mudou: mede-se"
           " pelo CUSTO, e ele é agora IGUAL nos três: a mesma consulta sobre conteúdos"
           " diferentes — vinte casamentos, dez, zero — custa 2700 passos, e a lei não tem"
           " termo em |I| nenhum: passos = c·|X|. O motor deixou de olhar para o dado antes de"
           " decidir por onde ir. ISTO LEVOU DOIS PASSOS, e o segundo veio depois de o primeiro"
           " ser medido. Ao tirar os saltos do MOLDE ficou 2900 + 3·|I|: três passos por linha"
           " que casa, e eles não vinham do molde — vinham do AVALIADOR da condição, que ainda"
           " ramificava com JZ curtos para escolher entre o zero e o um. Tirá-los pedia uma"
           " instrução que a máquina não tinha, e é uma só: ESPALHAR, que devolve a máscara"
           " inteira se o argumento é não-nulo e zero se é nulo. Com ela `<` e `>` são o bit de"
           " sinal espalhado e `=` é o complemento disso, o resultado de uma condição passa de"
           " um BIT a uma MÁSCARA, e o molde deixa de ter de a fabricar — a máscara nasce onde"
           " a condição se decide, uma vez, e não outra vez por linha. O CONTROLO fecha pelo"
           " tamanho: com o dobro das linhas o custo dobra EXACTAMENTE, 2700 para 5400, que é o"
           " Θ(|X|) da varredura — sem isto, «igual nos três» passaria por o número ser fixo. E"
           " as três respostas continuam certas, 20, 10 e 0, porque um custo comprado com um"
           " resultado errado não é um resultado. Antes de tudo isto escrevi a comparação"
           " errada: `a > 0` contra `a > 999` contra `a > 10`, que deu três números sem lei"
           " nenhuma, porque a CONSTANTE compila em Zeckendorf e três constantes são três"
           " PROGRAMAS — tinha mudado o dado e a chave ao mesmo tempo.",
           mal == 0);
    }

    /* ═══ §W25: A SUBCONSULTA É A PERTENÇA A UMA FIBRA ═══════════════════════ */
    {
        const char *base = "/tmp/pgwire_w25";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w25.mem");
        unlink("/tmp/pgwire_w25.prog");
        unlink("/tmp/pgwire_w25__s.mem");
        printf("\n§W25 a subconsulta: pertença à fibra, pela árvore do banco.\n\n");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE a (x,y)", &o);
        for(int i = 1; i <= 6; i++){
            char q[64];
            snprintf(q, sizeof q, "INSERT INTO a VALUES (%d,%d)", i, i * 10);
            sql_executa(q, &o);
        }
        sql_executa("CREATE TABLE b (x,z)", &o);
        sql_executa("INSERT INTO b VALUES (2,99)", &o);
        sql_executa("INSERT INTO b VALUES (5,99)", &o);
        sql_executa("INSERT INTO b VALUES (2,88)", &o);   /* o 2 REPETIDO */

        int r = sql_executa("SELECT * FROM a WHERE x IN (SELECT x FROM b)", &o);
        int bate = r && o.nrows == 2 && !strcmp(o.cell[0][0], "2")
                     && !strcmp(o.cell[1][0], "5");
        printf("      x IN (SELECT x FROM b): %d linha(s)", o.nrows);
        for(int i = 0; i < o.nrows; i++) printf("  [%s,%s]", o.cell[i][0], o.cell[i][1]);
        printf("   %s\n", bate ? "" : "NAO BATE");
        if(!bate) mal++;

        int uma_so = (o.nrows == 2);
        printf("      o 2 está DUAS vezes na direita e sai UMA linha: %s"
               "   (pertencer não é emparelhar)\n", uma_so ? "sim" : "NAO");
        if(!uma_so) mal++;

        int rc = sql_executa("SELECT count(*) FROM a WHERE x IN (SELECT x FROM b)", &o);
        int cok = rc && o.nrows == 1 && !strcmp(o.cell[0][0], "2");
        printf("      count(*) pela subconsulta: %s  %s\n",
               o.nrows ? o.cell[0][0] : "?", cok ? "" : "NAO BATE");
        if(!cok) mal++;

        int rs = sql_executa("SELECT * FROM a", &o);
        int sessao = rs && o.nrows == 6;
        printf("\n      CONTROLO — a sessão ficou intacta: %d linhas  %s\n",
               o.nrows, sessao ? "(a tabela foi restaurada)" : "NAO");
        if(!sessao) mal++;

        int r1 = sql_executa("SELECT * FROM a WHERE x IN (SELECT zzz FROM b)", &o);
        int r2 = sql_executa("SELECT * FROM a WHERE zzz IN (SELECT x FROM b)", &o);
        printf("      coluna inexistente na subconsulta: %s  ·  à esquerda: %s\n",
               r1 ? "RESPONDEU (mau)" : "recusado", r2 ? "RESPONDEU (mau)" : "recusado");
        if(r1 || r2) mal++;
        sql_fechar();

        printf("\n");
        ok("A SUBCONSULTA É A PERTENÇA A UMA FIBRA, E NÃO TROUXE MAQUINARIA NENHUMA. A árvore"
           " que o join usa para casar já responde «este valor está lá?», e o que separa os"
           " dois é apenas o que se faz com a resposta: o join produz o PAR, o IN fica-se pelo"
           " BIT. É o corte sem a segunda metade — descer por um valor é a dobra, e o que se lê"
           " no fim do caminho é se a fibra é vazia ou não. O GUME É A REPETIÇÃO: a direita tem"
           " o mesmo valor DUAS vezes, e um join daria duas linhas para ele, ao passo que o IN"
           " dá UMA, porque pertencer não é emparelhar. Sem essa linha o IN podia estar"
           " implementado como um join e nenhuma asserção o via. E O CONTROLO É A SESSÃO, por"
           " uma razão que custou duas tentativas: a subconsulta ABRE outra tabela para montar"
           " a árvore, e a árvore vive no .mem — voltar à tabela de origem relê-o e APAGA-A. À"
           " primeira escrita a decisão era tomada depois de voltar, e a descida devolvia zero"
           " para valores que lá estavam. Decide-se por isso com a tabela da subconsulta ainda"
           " aberta, guarda-se a resposta, e só então se volta; e exige-se que a tabela da"
           " sessão seja restaurada, porque uma consulta não pode trocá-la por baixo de quem a"
           " fez. Houve um segundo defeito do mesmo feitio: reutilizar as variáveis do join"
           " para guardar a tabela da subconsulta ACORDAVA o caminho do join, que espera uma"
           " coluna de junção à esquerda e não a encontrava. O estado partilhado ligava dois"
           " caminhos que nada têm um com o outro; a árvore é que é comum, e empresta-se na"
           " hora.",
           mal == 0);
    }

    /* ═══ §W26: A UNIÃO É O DUAL DO `IN` ═════════════════════════════════════ */
    {
        const char *base = "/tmp/pgwire_w26";
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w26.mem");
        unlink("/tmp/pgwire_w26.prog");
        unlink("/tmp/pgwire_w26__s.mem");
        printf("\n§W26 a união: o join do reticulado, dual da pertença.\n\n");
        if(!sql_abrir(base)) mal++;
        sql_executa("CREATE TABLE a (x)", &o);
        sql_executa("INSERT INTO a VALUES (1)", &o);
        sql_executa("INSERT INTO a VALUES (2)", &o);
        sql_executa("INSERT INTO a VALUES (3)", &o);
        sql_executa("CREATE TABLE b (x)", &o);
        sql_executa("INSERT INTO b VALUES (3)", &o);
        sql_executa("INSERT INTO b VALUES (4)", &o);

        int r1 = sql_executa("SELECT * FROM a UNION SELECT * FROM b", &o);
        int n1 = o.nrows;
        printf("      a UNION b: %d linha(s):", n1);
        for(int i = 0; i < o.nrows; i++) printf(" %s", o.cell[i][0]);
        printf("   (esperado 4: o 3 uma vez)\n");
        if(!r1 || n1 != 4) mal++;

        int r2 = sql_executa("SELECT * FROM a UNION ALL SELECT * FROM b", &o);
        int n2 = o.nrows;
        printf("      a UNION ALL b: %d linha(s):", n2);
        for(int i = 0; i < o.nrows; i++) printf(" %s", o.cell[i][0]);
        printf("   (esperado 5: o 3 duas vezes)\n");
        if(!r2 || n2 != 5) mal++;

        /* O GUME: os dois números têm de ser DIFERENTES. Uma união que nunca
         * removesse nada passaria por união em qualquer tabela sem repetidos. */
        printf("      -> UNION e UNION ALL dão números DIFERENTES: %s (%d vs %d)\n",
               n1 != n2 ? "sim" : "NAO", n1, n2);
        if(n1 == n2) mal++;

        /* E A IDEMPOTÊNCIA, que é a lei do join: x ∨ x = x. Sai de graça se o
         * representante é um por valor, e falha em qualquer implementação que
         * apenas concatene. */
        int r3 = sql_executa("SELECT * FROM a UNION SELECT * FROM a", &o);
        int idem = r3 && o.nrows == 3;
        printf("      a UNION a = a (idempotência do join): %d linha(s)  %s\n",
               o.nrows, idem ? "" : "NAO BATE");
        if(!idem) mal++;

        /* ── O CONTROLO: as formas que não fecham são RECUSADAS com a razão. */
        int e1 = sql_executa("SELECT * FROM a UNION SELECT zzz FROM b", &o);
        int e2 = sql_executa("SELECT * FROM a UNION SELECT * FROM b UNION SELECT * FROM a", &o);
        printf("\n      CONTROLO — lado inválido: %s  ·  encadeada: %s\n",
               e1 ? "RESPONDEU (mau)" : "recusado", e2 ? "RESPONDEU (mau)" : "recusado");
        if(e1 || e2) mal++;

        int rs = sql_executa("SELECT * FROM a", &o);
        int sessao = rs && o.nrows == 3;
        printf("      e a sessão ficou intacta: %d linhas  %s\n",
               o.nrows, sessao ? "" : "NAO");
        if(!sessao) mal++;
        sql_fechar();

        printf("\n");
        ok("A UNIÃO É O DUAL DA PERTENÇA, E É A ÚNICA CLÁUSULA QUE VIVE NA FACHADA. O `IN` do"
           " §W25 pergunta se um valor está na fibra do outro lado e fica-se pelo bit — é a"
           " INTERSECÇÃO; a união é o outro membro do par, o join do reticulado, e por isso não"
           " corre no molde nem precisa da árvore: corre as duas consultas e junta o que saiu."
           " Vive na fachada porque é a única que fala de DUAS RESPOSTAS em vez de duas"
           " tabelas. O GUME SÃO AS DUAS FORMAS TEREM DE DIFERIR: `UNION` deixa UM"
           " representante por valor — o k=1 do levantamento, o mesmo do DISTINCT — e"
           " `UNION ALL` deixa a fibra inteira, pelo que sobre as mesmas tabelas os números"
           " têm de ser DIFERENTES, quatro contra cinco. Sem essa exigência, uma implementação"
           " que nunca removesse nada passaria por união em qualquer par de tabelas sem"
           " repetidos — que são todas as dos outros blocos deste ficheiro. E A IDEMPOTÊNCIA"
           " FECHA-O PELA LEI: a UNION a = a, que é x ∨ x = x, e sai de graça se o"
           " representante é um por valor mas falha em qualquer coisa que apenas concatene. O"
           " CONTROLO recusa o que não fecha — um lado inválido e a forma encadeada, que este"
           " motor não lê — e exige que a sessão fique intacta, porque a união abre as duas"
           " tabelas por baixo de quem perguntou.",
           mal == 0);
    }

    /* ═══ §W27: O ÍNDICE — DESCER EM VEZ DE VARRER ═══════════════════════════
     *
     * «são precisos log n cortes para igualar uma ordem, e esse número É a
     * profundidade» (arquitetura.tex §sec:isa); «nenhuma dependência de |X| —
     * é a afirmação que importa, porque é a que separa o algoritmo da tabela»
     * (aranha.tex §sec:algoritmo). O motor varria o espaço por cada WHERE. */
    {
        SqlOut o;
        long mal = 0;
        long nos[3], passos_sem = 0;
        const int tam[3] = { 20, 40, 80 };
        printf("\n§W27 o índice: descer em vez de varrer.\n\n");

        for(int k = 0; k < 3; k++){
            char base[64], m[80], pr[80], q[80];
            snprintf(base, sizeof base, "/tmp/pgwire_w27_%d", k);
            snprintf(m, sizeof m, "%s.mem", base);
            snprintf(pr, sizeof pr, "%s.prog", base);
            unlink(m); unlink(pr);
            if(!sql_abrir(base)) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= tam[k]; i++){
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            /* SEM índice, e é o CONTROLO: aqui o custo TEM de crescer */
            sql_executa("SELECT * FROM t WHERE a = 5", &o);
            long p_sem = sql_ultimos_passos;
            int certo_sem = (o.nrows == 1 && !strcmp(o.cell[0][1], "10"));
            if(k == 0) passos_sem = p_sem;
            if(k == 2 && p_sem <= passos_sem) mal++;    /* cresceu com o tamanho */

            int ri = sql_executa("CREATE INDEX ON t (a)", &o);
            sql_executa("SELECT * FROM t WHERE a = 5", &o);
            nos[k] = sql_ultimos_nos;
            int certo_com = (o.nrows == 1 && !strcmp(o.cell[0][1], "10"));
            printf("      %2d linhas · sem índice: %4ld passos · com índice: %ld nós"
                   "   os dois dão [5,10]: %s\n",
                   tam[k], p_sem, nos[k], (certo_sem && certo_com) ? "sim" : "NAO");
            if(!ri || !certo_sem || !certo_com) mal++;
            sql_fechar();
        }
        int lei = (nos[0] == nos[1] && nos[1] == nos[2] && nos[0] > 0);
        printf("      -> os nós NÃO crescem com |X|: %s  (%ld, %ld, %ld)\n",
               lei ? "sim" : "NAO", nos[0], nos[1], nos[2]);
        if(!lei) mal++;

        /* ── E AS DESIGUALDADES DESCEM TAMBÉM. A ordem dos caminhos É a ordem
         * dos números, pelo que `a > k` é um prefixo comum seguido dos ramos de
         * um lado. Mede-se com resultado FIXO e tabela a crescer: os nós crescem
         * com o que SAI, e o que não pode crescer é a BUSCA. */
        {
            long nos_d[3];
            printf("\n      as DESIGUALDADES, com resultado fixo e tabela a crescer:\n");
            for(int k = 0; k < 3; k++){
                char base[64], m[80], pr[80], q[80];
                snprintf(base, sizeof base, "/tmp/pgwire_w27d_%d", k);
                snprintf(m, sizeof m, "%s.mem", base);
                snprintf(pr, sizeof pr, "%s.prog", base);
                unlink(m); unlink(pr);
                if(!sql_abrir(base)) mal++;
                sql_executa("CREATE TABLE t (a,b)", &o);
                for(int i = 1; i <= tam[k]; i++){
                    snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                    sql_executa(q, &o);
                }
                sql_executa("SELECT * FROM t WHERE a <= 3", &o);
                long p_sem = sql_ultimos_passos; int n_sem = o.nrows;
                sql_executa("CREATE INDEX ON t (a)", &o);
                sql_executa("SELECT * FROM t WHERE a <= 3", &o);
                nos_d[k] = sql_ultimos_nos;
                int n_com = o.nrows;
                printf("        %2d linhas · sem índice %4ld passos · com índice %3ld nós"
                       " · %d = %d resultados  %s\n", tam[k], p_sem, nos_d[k],
                       n_sem, n_com, (n_sem == 3 && n_com == 3) ? "" : "NAO BATE");
                if(n_sem != 3 || n_com != 3) mal++;
                sql_fechar();
            }
            int lei_d = (nos_d[0] == nos_d[1] && nos_d[1] == nos_d[2] && nos_d[0] > 0);
            printf("        -> também não crescem com |X|: %s  (%ld, %ld, %ld)\n",
                   lei_d ? "sim" : "NAO", nos_d[0], nos_d[1], nos_d[2]);
            if(!lei_d) mal++;
        }

        /* os QUATRO operadores contra o molde — dois caminhos que concordam */
        {
            unlink("/tmp/pgwire_w27q.mem"); unlink("/tmp/pgwire_w27q.prog");
            if(!sql_abrir("/tmp/pgwire_w27q")) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= 10; i++){
                char q[64];
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            const char *ops[6] = { "a > 6", "a < 4", "a >= 6", "a <= 4", "a > 100", "a < 0" };
            int esperado[6] = { 4, 3, 5, 4, 0, 0 };
            int sem[6];
            for(int k = 0; k < 6; k++){
                char q[80]; snprintf(q, sizeof q, "SELECT * FROM t WHERE %s", ops[k]);
                sql_executa(q, &o); sem[k] = o.nrows;
            }
            sql_executa("CREATE INDEX ON t (a)", &o);
            printf("\n      os seis casos, molde contra árvore:\n");
            for(int k = 0; k < 6; k++){
                char q[80]; snprintf(q, sizeof q, "SELECT * FROM t WHERE %s", ops[k]);
                sql_executa(q, &o);
                int bate = (o.nrows == sem[k] && o.nrows == esperado[k]
                            && sql_ultimos_passos == 0);
                printf("        %-8s molde %d · árvore %d · esperado %d  %s\n",
                       ops[k], sem[k], o.nrows, esperado[k], bate ? "" : "NAO BATE");
                if(!bate) mal++;
            }
            sql_fechar();
        }

        /* ── O CONTROLO DA CORRECÇÃO: o índice VELHO não pode mentir. */
        {
            unlink("/tmp/pgwire_w27v.mem"); unlink("/tmp/pgwire_w27v.prog");
            if(!sql_abrir("/tmp/pgwire_w27v")) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= 20; i++){
                char q[64];
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            sql_executa("CREATE INDEX ON t (a)", &o);

            /* O ζ: escrever é ACUMULAR, e acumular numa árvore é uma descida.
             * O INSERT acrescenta a chave e o índice ACOMPANHA — não invalida. */
            sql_executa("INSERT INTO t VALUES (99,198)", &o);
            sql_executa("SELECT * FROM t WHERE a = 5", &o);
            int velho_ok = (o.nrows == 1 && !strcmp(o.cell[0][1], "10"));
            long p_ins = sql_ultimos_passos;
            sql_executa("SELECT * FROM t WHERE a = 99", &o);
            int nova_ok = (o.nrows == 1 && !strcmp(o.cell[0][1], "198"));
            long p_nova = sql_ultimos_passos;
            printf("\n      ζ — o INSERT: o índice ACOMPANHA (%ld e %ld passos),"
                   " a antiga continua certa: %s, e a NOVA aparece: %s\n",
                   p_ins, p_nova, velho_ok ? "sim" : "NAO", nova_ok ? "sim" : "NAO");
            if(!velho_ok || !nova_ok || p_ins != 0 || p_nova != 0) mal++;

            /* O DELETE não muda o número de linhas: o cabeçalho continua a bater
             * e a chave apagada ficou na árvore. Quem decide é o campo do vivo. */
            sql_executa("DELETE FROM t WHERE a = 7", &o);
            sql_executa("SELECT * FROM t WHERE a = 7", &o);
            int del_ok = (o.nrows == 0);
            sql_executa("SELECT * FROM t WHERE a = 8", &o);
            int viz_ok = (o.nrows == 1 && !strcmp(o.cell[0][1], "16"));
            printf("      o DELETE: a apagada NÃO sai (%s) e a vizinha sai (%s)"
                   "  — quem decide é o campo do vivo\n",
                   del_ok ? "sim" : "NAO", viz_ok ? "sim" : "NAO");
            if(!del_ok || !viz_ok) mal++;

            /* O µ: a mesma descida com a volta por cima — guarda-se o caminho,
             * corta-se a folha, e sobe-se a apagar todo o nó que ficou vazio.
             * O UPDATE desacumula a velha e acumula a nova; o índice FICA. */
            sql_executa("UPDATE t SET a = 77 WHERE a = 8", &o);
            sql_executa("SELECT * FROM t WHERE a = 77", &o);
            int upd_ok = (o.nrows == 1);
            long p_upd = sql_ultimos_passos;
            sql_executa("SELECT * FROM t WHERE a = 8", &o);
            int foi_ok = (o.nrows == 0);
            long p_foi = sql_ultimos_passos;
            printf("      µ — o UPDATE desacumula e reacumula (%ld e %ld passos):"
                   " o novo aparece (%s) e o velho já não (%s)\n",
                   p_upd, p_foi, upd_ok ? "sim" : "NAO", foi_ok ? "sim" : "NAO");
            if(!upd_ok || !foi_ok || p_upd != 0 || p_foi != 0) mal++;

            /* O GUME DO µ É A FIBRA: mover um valor PARA CIMA de outro que já
             * existe tem de dar DOIS na mesma chave. Um µ que apagasse o ramo
             * todo em vez da folha levaria o vizinho à frente — e isso não se vê
             * em caso nenhum de valor único. */
            sql_executa("UPDATE t SET a = 9 WHERE a = 77", &o);
            sql_executa("SELECT * FROM t WHERE a = 9", &o);
            int fibra_ok = (o.nrows == 2);
            int nf = o.nrows;
            sql_executa("SELECT * FROM t WHERE a = 77", &o);
            int limpo_ok = (o.nrows == 0);
            printf("      e a FIBRA: mover o 77 para o 9, onde já havia um -> %d na chave"
                   " (%s), e o 77 ficou vazio (%s)\n", nf,
                   fibra_ok ? "sim" : "NAO", limpo_ok ? "sim" : "NAO");
            if(!fibra_ok || !limpo_ok) mal++;

            /* a desigualdade também desce agora — e o que fica fora é a forma
             * COMPOSTA, que a árvore não responde de um caminho só: aí o molde
             * corre, e mede-se que corre. */
            sql_executa("SELECT * FROM t WHERE a > 5", &o);
            int desce = (o.nrows == 15 && sql_ultimos_passos == 0);
            printf("      `a > 5` também desce: %d linha(s), %ld passos  %s\n",
                   o.nrows, sql_ultimos_passos, desce ? "" : "NAO BATE");
            if(!desce) mal++;

            sql_executa("SELECT * FROM t WHERE a > 5 AND b < 30", &o);
            int comp = (sql_ultimos_passos > 0);
            printf("      a composta sobre OUTRA coluna ainda corre o molde:"
                   " %d linha(s), %ld passos  %s\n",
                   o.nrows, sql_ultimos_passos, comp ? "" : "NAO BATE");
            if(!comp) mal++;
            sql_fechar();
        }

        /* ── O CORTE PRIMEIRO, O MOLDE SÓ SOBRE O QUE SOBROU ─────────────────
         * A árvore não responde à composta sobre outra coluna, mas responde a
         * METADE dela — e essa metade RESTRINGE, porque a ligação é um AND.
         * Serve de pré-filtro: o molde corre só nas candidatas. */
        {
            unlink("/tmp/pgwire_w27p.mem"); unlink("/tmp/pgwire_w27p.prog");
            if(!sql_abrir("/tmp/pgwire_w27p")) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= 40; i++){
                char q[64];
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            sql_executa("SELECT * FROM t WHERE a > 30 AND b < 70", &o);
            long p_and_sem = sql_ultimos_passos; int n_and_sem = o.nrows;
            sql_executa("SELECT * FROM t WHERE a > 30 OR b < 6", &o);
            long p_or_sem = sql_ultimos_passos; int n_or_sem = o.nrows;

            sql_executa("CREATE INDEX ON t (a)", &o);

            sql_executa("SELECT * FROM t WHERE a > 30 AND b < 70", &o);
            long p_and_com = sql_ultimos_passos; int n_and_com = o.nrows;
            int and_ok = (n_and_sem == 4 && n_and_com == 4 && p_and_com < p_and_sem);
            printf("\n      o AND: 10 candidatas de 40 · %ld passos -> %ld  ·"
                   " %d = %d linhas  %s\n", p_and_sem, p_and_com,
                   n_and_sem, n_and_com, and_ok ? "" : "NAO BATE");
            if(!and_ok) mal++;

            /* O GUME: com OR o pré-filtro NÃO pode correr. O outro lado traz
             * linhas de FORA da faixa, e cortá-las responderia menos do que a
             * pergunta — pior do que responder devagar. */
            sql_executa("SELECT * FROM t WHERE a > 30 OR b < 6", &o);
            long p_or_com = sql_ultimos_passos; int n_or_com = o.nrows;
            int or_ok = (n_or_sem == 12 && n_or_com == 12 && p_or_com == p_or_sem);
            printf("      o OR: NÃO pré-filtra (%ld = %ld passos) e traz as de fora"
                   " da faixa: %d = %d linhas  %s\n",
                   p_or_sem, p_or_com, n_or_sem, n_or_com, or_ok ? "" : "NAO BATE");
            if(!or_ok) mal++;
            sql_fechar();
        }

        /* ── A FAIXA COM OS DOIS EXTREMOS ────────────────────────────────────
         * Duas condições sobre a MESMA coluna são a faixa fechada — cada uma
         * dá um lado, e o AND intersecta-os. O BETWEEN é isso dito numa
         * palavra. Compara-se sempre com o molde, que é a referência. */
        {
            unlink("/tmp/pgwire_w27f.mem"); unlink("/tmp/pgwire_w27f.prog");
            if(!sql_abrir("/tmp/pgwire_w27f")) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= 20; i++){
                char q[64];
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            const char *fx[4] = {
                "SELECT * FROM t WHERE a > 5 AND a < 10",
                "SELECT * FROM t WHERE a >= 5 AND a <= 8",
                "SELECT * FROM t WHERE a BETWEEN 5 AND 8",
                "SELECT * FROM t WHERE a > 15 AND a < 5"       /* faixa VAZIA */
            };
            int esp[4] = { 4, 4, 4, 0 };
            int sem[4];
            for(int k = 0; k < 4; k++){ sql_executa(fx[k], &o); sem[k] = o.nrows; }
            sql_executa("CREATE INDEX ON t (a)", &o);
            printf("\n      a FAIXA com os dois extremos, molde contra árvore:\n");
            for(int k = 0; k < 4; k++){
                sql_executa(fx[k], &o);
                int bate = (o.nrows == sem[k] && o.nrows == esp[k]
                            && sql_ultimos_passos == 0);
                printf("        %-42s molde %d · árvore %d · esperado %d  %s\n",
                       fx[k] + 21, sem[k], o.nrows, esp[k], bate ? "" : "NAO BATE");
                if(!bate) mal++;
            }

            int rc = sql_executa("CREATE INDEX ON t (zzz)", &o);
            printf("      coluna inexistente no índice: %s\n",
                   rc ? "RESPONDEU (mau)" : "recusado");
            if(rc) mal++;
            sql_fechar();
        }

        printf("\n");
        ok("O ÍNDICE DESCE, E POR ISSO O CUSTO LARGA O TAMANHO DA TABELA. A teoria é explícita"
           " nos dois documentos: «são precisos log n cortes para igualar uma ordem, e esse"
           " número É a profundidade», e «nenhuma dependência de |X| — é a afirmação que"
           " importa, porque é a que separa o algoritmo da tabela». O motor varria o espaço"
           " inteiro por cada WHERE, e isso não era uma optimização em falta: era o algoritmo a"
           " correr como tabela. A árvore que o ORDER BY e o JOIN já usavam passa a poder NÃO"
           " ser limpa — é a mesma lei com outra base, na zona 3 do .mem, DENTRO do ficheiro da"
           " tabela, sem memória de programa nenhuma a segurá-la entre consultas. E O QUE SE"
           " MEDE SÃO OS NÓS, não os passos: com índice os passos de ISA são ZERO, e zero não"
           " distingue não-ter-corrido de não-ter-feito-nada. Os nós visitados são QUARENTA em"
           " tabelas de vinte, quarenta e oitenta linhas — o mesmo número, que é a profundidade"
           " —, ao passo que sem índice os passos crescem com o tamanho, e é esse o CONTROLO"
           " que impede «constante» de passar por trivial. OS DOIS CAMINHOS TÊM DE CONCORDAR:"
           " a mesma consulta com e sem índice dá a mesma linha, nas três tabelas. E O ÍNDICE"
           " ACOMPANHA A ESCRITA PELO LADO EM QUE ISSO É FÁCIL, e o `thm:zeta-mu` diz qual é:"
           " escrever é a convolução com ζ, que ACUMULA, e acumular numa árvore é uma descida"
           " — o INSERT acrescenta a chave e o índice fica válido, medido a zero passos tanto"
           " para a linha antiga como para a nova. TIRAR uma chave seria o µ, e numa árvore de"
           " prefixos é a MESMA descida com a volta por cima: guarda-se o caminho ao descer,"
           " corta-se a ligação da folha, e SOBE-SE a apagar todo o nó que ficou sem filhos. O"
           " UPDATE desacumula a chave velha e acumula a nova — µ e depois ζ, o par completo —"
           " e o índice NÃO é largado, medido a zero passos dos dois lados. O GUME DO µ É A"
           " FIBRA: mover um valor para cima de outro que já existe tem de deixar DOIS na mesma"
           " chave, e um µ que apagasse o ramo inteiro em vez da folha levaria o vizinho à"
           " frente — coisa que nenhum caso de valor único mostraria. O que o µ NÃO faz é"
           " devolver os nós ao contador: um nó apagado fica órfão, e essa fuga fica DECLARADA,"
           " com tecto — quando a árvore enche, a inserção falha e o índice é largado, que é o"
           " comportamento que já lá estava. E O DELETE É O TERCEIRO"
           " CASO, e o mais traiçoeiro: não muda o NÚMERO de linhas, pelo que o cabeçalho"
           " continua a bater e o índice continua a ser usado — mas a chave apagada ficou lá"
           " dentro. Quem decide é o campo do VIVO, consultado à saída da árvore: ela diz onde"
           " a linha estava, e ele diz se ela ainda está. Mede-se com a apagada a não sair e a"
           " vizinha a sair. Fica dito o que o índice NÃO serve: só a forma `col = k` simples"
           " desce pela árvore. E AS DESIGUALDADES DESCEM TAMBÉM, porque a ordem dos"
           " caminhos É a ordem dos números: `a > k` é um prefixo comum seguido de todos os"
           " ramos de um lado, e nada mais — cortar dá o dígito, percorrer dá o resto, que são"
           " as duas operações que a casa já tinha. Os seis casos medidos dão o MESMO que o"
           " molde, incluindo os dois em que a faixa é VAZIA; e com resultado FIXO e tabela a"
           " crescer os nós ficam constantes, ao passo que os passos sem índice dobram. O"
           " resultado é fixo de propósito: os nós crescem com o que SAI, porque têm de listar"
           " as linhas, e o que não pode crescer é a BUSCA. E fica dito o que continua a NÃO"
           " descer: a forma composta — `a > 5 AND b < 30` —, que a árvore não responde de um"
           " caminho só quando é sobre OUTRA coluna. MAS ELA AINDA SERVE PARA METADE: essa"
           " metade RESTRINGE, porque a ligação é um AND, pelo que a árvore corta primeiro e o"
           " molde corre só nas candidatas — dez de quarenta, e os passos caem de 13200 para"
           " 3300 com a mesma resposta. O GUME É O `OR`: aí o pré-filtro NÃO pode correr,"
           " porque o outro lado traz linhas de FORA da faixa e cortá-las responderia MENOS do"
           " que a pergunta, o que é pior do que responder devagar. Mede-se que o OR custa o"
           " mesmo com e sem índice, e que traz as tais linhas de fora. MAS"
           " DUAS CONDIÇÕES SOBRE A MESMA COLUNA DESCEM: cada uma dá um lado e o AND"
           " intersecta-os, o que é a faixa FECHADA — e o BETWEEN é isso dito numa palavra. Os"
           " quatro casos medidos dão o mesmo que o molde, incluindo aquele em que os lados se"
           " cruzam e a faixa fica VAZIA, que é onde uma intersecção mal feita apareceria como"
           " tabela inteira.",
           mal == 0);
    }

    /* ═══ §W28: AS OITO RÉGUAS, E O QUE SAI DELAS ════════════════════════════
     *
     * `thm:base8`: «uma RÉGUA é um peso por direcção, g^(k) medindo só e_k:
     * cada uma é cega às outras sete, as oito compostas medem todo salto, e
     * tirando uma qualquer fica uma direcção por medir». E logo a seguir: «como
     * a base é ortonormal, MEDIR e LER O BIT são a mesma operação».
     *
     * Isto não é uma metáfora sobre o motor: é o que ele faz. O campo é um
     * vector nesta base, e cada operação sobre campos é uma régua ou uma
     * composição delas. Mede-se que é, e — o que importa mais — mede-se o que
     * NÃO é. */
    {
        long mal = 0;
        const int N = 8;
        printf("\n§W28 as oito réguas: medir e ler o bit são a mesma operação.\n\n");

        /* (a) as três cláusulas do thm:base8, uma a uma */
        int cega = 1, todo = 1, falta = 1;
        for(int k = 0; k < N; k++) for(int j = 0; j < N; j++){
            int g = ((1u << j) >> k) & 1u;
            if(g != (k == j)) cega = 0;
        }
        for(unsigned x = 0; x < 256u; x++){
            int soma = 0, pc = 0;
            for(int k = 0; k < N; k++) soma += (int)((x >> k) & 1u);
            for(unsigned y = x; y; y >>= 1) pc += (int)(y & 1u);
            if(soma != pc) todo = 0;
            for(int j = 0; j < N; j++){
                int sem = 0;
                for(int k = 0; k < N; k++) if(k != j) sem += (int)((x >> k) & 1u);
                if(sem != pc - (int)((x >> j) & 1u)) falta = 0;
            }
        }
        printf("      cada régua é CEGA às outras sete: %s\n", cega ? "sim" : "NAO");
        printf("      as oito COMPOSTAS medem todo salto (∑ g^(k) = popcount): %s\n",
               todo ? "sim" : "NAO");
        printf("      tirando uma, fica uma direcção por medir: %s\n",
               falta ? "sim" : "NAO");
        if(!cega || !todo || !falta) mal++;

        /* (b) e as operações do motor SÃO isso, no campo — medidas pelo motor,
         * não recalculadas aqui: marcar é pôr e_i, o AND é ∧ nas coordenadas, o
         * OR é ∨, e o count é a soma das oito. */
        {
            const char *base = "/tmp/pgwire_w28";
            SqlOut o;
            unlink("/tmp/pgwire_w28.mem"); unlink("/tmp/pgwire_w28.prog");
            if(!sql_abrir(base)) mal++;
            sql_executa("CREATE TABLE t (a,b)", &o);
            for(int i = 1; i <= 8; i++){
                char q[64];
                snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
                sql_executa(q, &o);
            }
            /* três condições, e as suas combinações */
            /* AS DUAS CONDIÇÕES TÊM DE SE CRUZAR. À primeira escrita eram
             * `a > 4` e `b < 10`, que nesta tabela são complementares — a
             * intersecção dava ZERO e a identidade ficava 8 + 0 = 4 + 4, uma
             * soma verdadeira que não mediria nada. Agora sobrepõem-se em três
             * linhas, e o controlo abaixo exige que se sobreponham. */
            sql_executa("SELECT count(*) FROM t WHERE a > 2", &o);
            long nA = o.nrows ? atol(o.cell[0][0]) : -1;
            sql_executa("SELECT count(*) FROM t WHERE b < 12", &o);
            long nB = o.nrows ? atol(o.cell[0][0]) : -1;
            sql_executa("SELECT count(*) FROM t WHERE a > 2 AND b < 12", &o);
            long nAB = o.nrows ? atol(o.cell[0][0]) : -1;
            sql_executa("SELECT count(*) FROM t WHERE a > 2 OR b < 12", &o);
            long nAoB = o.nrows ? atol(o.cell[0][0]) : -1;

            /* A INCLUSÃO-EXCLUSÃO É A LEI DAS RÉGUAS: contar é somar as oito, e
             * a soma é linear — logo |A∨B| + |A∧B| = |A| + |B|, sem excepção.
             * Um motor que fizesse o AND ou o OR por outro caminho que não as
             * coordenadas quebraria isto sem quebrar nenhuma contagem isolada. */
            int lei = (nA >= 0 && nB >= 0 && nAB >= 0 && nAoB >= 0
                       && nAoB + nAB == nA + nB);
            printf("\n      |A| = %ld · |B| = %ld · |A∧B| = %ld · |A∨B| = %ld\n",
                   nA, nB, nAB, nAoB);
            printf("      -> |A∨B| + |A∧B| = |A| + |B|:  %ld = %ld  %s\n",
                   nAoB + nAB, nA + nB, lei ? "sim" : "NAO");
            if(!lei) mal++;

            /* o CONTROLO: as duas contagens têm de ser DIFERENTES, senão a
             * identidade passaria por os conjuntos coincidirem */
            int sep = (nAB != nAoB && nAB > 0 && nAB < nA && nAB < nB);
            printf("      CONTROLO — os conjuntos CRUZAM-SE de facto:"
                   " 0 < |A∧B| = %ld < |A| e < |B|, e ∧ ≠ ∨ (%ld vs %ld):  %s\n",
                   nAB, nAB, nAoB, sep ? "sim" : "NAO");
            if(!sep) mal++;
            sql_fechar();
        }

        printf("\n");
        ok("AS OITO RÉGUAS SÃO A BASE, E AS OPERAÇÕES DE CAMPO SAEM TODAS DELAS. O `thm:base8`"
           " diz o que uma régua é — um peso por direcção, g^(k) medindo só e_k — e as três"
           " cláusulas verificam-se aqui uma a uma: cada uma é CEGA às outras sete, as oito"
           " COMPOSTAS medem todo salto, e tirando uma qualquer fica uma direcção por medir. E"
           " logo a seguir vem a frase que liga isto ao motor: como a base é ortonormal, MEDIR"
           " e LER O BIT são a mesma operação. Não é uma metáfora — o campo É um vector nesta"
           " base, e cada coisa que o motor lhe faz é uma régua ou uma composição delas: marcar"
           " a linha i é pôr a coordenada e_i, o AND de duas condições é o ∧ coordenada a"
           " coordenada, o OR é o ∨, e o count é a SOMA das oito, que é o popcount. O QUE SE"
           " MEDE NÃO É CADA UMA ISOLADA, mas a lei que só vale se elas forem mesmo as"
           " coordenadas: contar é somar réguas, a soma é linear, e portanto"
           " |A∨B| + |A∧B| = |A| + |B| sem excepção. Um motor que fizesse o AND ou o OR por um"
           " caminho que não as coordenadas quebraria esta identidade sem quebrar nenhuma"
           " contagem isolada — é por isso que é ela que se mede, e não os quatro números um a"
           " um. O CONTROLO exige que ∧ e ∨ dêem números DIFERENTES, senão a identidade"
           " passaria por os dois conjuntos coincidirem — e exige mais do que isso, porque à"
           " primeira escrita as duas condições que escolhi eram COMPLEMENTARES nesta tabela: a"
           " intersecção dava zero, a identidade ficava 8 + 0 = 4 + 4, e essa soma é verdadeira"
           " sem medir nada. Agora exige-se que os conjuntos se cruzem de facto, com a"
           " intersecção maior que zero e menor do que cada um deles. E FICA DITO O QUE NÃO SAI DA BASE,"
           " porque é onde o trabalho ainda falta: a comparação < e > usa o BIT DE SINAL de uma"
           " subtracção, que é uma convenção sobre o envelope e não uma régua; e a descida da"
           " árvore usa a ORDEM dos símbolos, que é a métrica linear. Essas duas são as que"
           " ainda não se leem como medida.",
           mal == 0);
    }

    /* ═══ §W29: A VISTA É UMA COMPOSIÇÃO COM NOME ════════════════════════════
     *
     * Uma condição é uma FUNÇÃO do campo no campo, e o AND compõe duas — que no
     * campo é o ∧ das coordenadas (§W28). Uma vista é essa composição com nome:
     * guarda a tabela e a condição, e usá-la num FROM compõe a condição dela
     * com a de quem a usa. Não traz operação nova; traz o direito de dar nome a
     * uma que já existia. */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w29.mem"); unlink("/tmp/pgwire_w29.prog");
        unlink("/tmp/pgwire_w29__t.mem"); unlink("/tmp/pgwire_w29__t.prog");
        printf("\n§W29 a vista: compor funções, e a composição tem nome.\n\n");
        if(!sql_abrir("/tmp/pgwire_w29")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        for(int i = 1; i <= 20; i++){
            char q[64];
            snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
            sql_executa(q, &o);
        }
        int rc = sql_executa("CREATE VIEW altos AS SELECT * FROM t WHERE a > 15", &o);
        printf("      CREATE VIEW altos = t WHERE a > 15: %s\n", rc ? "ok" : "NAO");
        if(!rc) mal++;

        /* OS DOIS CAMINHOS TÊM DE CONCORDAR: a vista composta com uma segunda
         * condição dá o mesmo que as duas escritas à mão. */
        sql_executa("SELECT * FROM altos", &o);
        int n1 = o.nrows;
        sql_executa("SELECT * FROM altos WHERE a < 19", &o);
        int n2 = o.nrows;
        char c1[32] = "", c2[32] = "";
        if(o.nrows) snprintf(c1, sizeof c1, "%s", o.cell[0][0]);
        sql_executa("SELECT * FROM t WHERE a > 15 AND a < 19", &o);
        int n3 = o.nrows;
        if(o.nrows) snprintf(c2, sizeof c2, "%s", o.cell[0][0]);
        int bate = (n1 == 5 && n2 == 3 && n3 == 3 && !strcmp(c1, c2));
        printf("      a vista: %d linhas · composta com `a < 19`: %d · à mão: %d"
               "  (primeira célula %s = %s)  %s\n",
               n1, n2, n3, c1, c2, bate ? "" : "NAO BATE");
        if(!bate) mal++;

        /* e a agregação atravessa a vista — foi o que quase ficou de fora,
         * porque `count(*)` não entra na lista de colunas que o parser produz */
        int rk = sql_executa("SELECT count(*) FROM altos", &o);
        int ck = (rk && o.nrows == 1 && !strcmp(o.cell[0][0], "5"));
        printf("      count(*) FROM altos = %s  %s\n",
               o.nrows ? o.cell[0][0] : "?", ck ? "" : "NAO BATE");
        if(!ck) mal++;

        /* ── O CONTROLO: a vista NÃO pode trocar a tabela da sessão, e o que
         * ela não é tem de ser recusado com a razão. */
        sql_executa("SELECT * FROM t", &o);
        int sessao = (o.nrows == 20);
        printf("\n      CONTROLO — a sessão ficou intacta: %d linhas  %s\n",
               o.nrows, sessao ? "" : "NAO");
        if(!sessao) mal++;

        int e1 = sql_executa("CREATE VIEW v2 AS SELECT a FROM t WHERE a > 1", &o);
        int e2 = sql_executa("CREATE VIEW v3 AS SELECT * FROM t", &o);
        printf("      projecção na vista: %s  ·  vista sem WHERE: %s\n",
               e1 ? "RESPONDEU (mau)" : "recusada", e2 ? "RESPONDEU (mau)" : "recusada");
        if(e1 || e2) mal++;
        sql_fechar();

        printf("\n");
        ok("A VISTA É UMA COMPOSIÇÃO COM NOME, E NÃO TRAZ OPERAÇÃO NOVA. Uma condição é uma"
           " FUNÇÃO do campo no campo, e o AND compõe duas — que no campo é o ∧ das"
           " coordenadas, medido no §W28. Uma vista guarda a tabela e a condição, e usá-la num"
           " FROM compõe a condição dela com a de quem a usa: o que ela traz é o direito de dar"
           " NOME a uma composição que já existia. O QUE SE MEDE SÃO OS DOIS CAMINHOS A"
           " CONCORDAR: a vista composta com uma segunda condição dá as mesmas linhas e a mesma"
           " primeira célula que as duas condições escritas à mão, e sem isso a vista podia"
           " estar a devolver a tabela inteira em qualquer caso que não fosse olhado. E A"
           " AGREGAÇÃO TEM DE ATRAVESSAR, que é onde ela quase ficou de fora: `count(*)` não"
           " entra na lista de colunas que o parser produz — quem o produz é o bloco da fibra —,"
           " pelo que reescrever a partir dessa lista deixava a consulta sem colunas, e"
           " procurar o FROM por ela devolvia falso e nem reescrevia. Acha-se por isso o FROM"
           " de topo no próprio texto, e copiam-se as colunas como estavam: a vista troca a"
           " TABELA e acrescenta a condição, e não tem nada que dizer sobre o que se pede. O"
           " CONTROLO é a sessão, porque a vista vive na tabela SEM NOME — o .mem é por tabela,"
           " e uma vista tem de ser encontrada antes de se saber que tabela abrir, o que seria"
           " circular se ela morasse numa delas. Guarda-se e restaura-se a que estava aberta, e"
           " exige-se que fique. E o que a vista não é fica recusado com a razão: projecção"
           " dentro dela e vista sem WHERE, que não comporia nada.",
           mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
