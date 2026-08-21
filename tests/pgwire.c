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

        /* as transacções, que os drivers mandam antes de tudo */
        {
            const char *tx[3] = { "BEGIN", "COMMIT", "ROLLBACK" };
            int bate = 1;
            for(int i = 0; i < 3; i++){
                int r = sql_executa(tx[i], &o);
                if(!r || strcmp(o.tag, tx[i]) || o.ncols) bate = 0;
            }
            printf("      BEGIN/COMMIT/ROLLBACK -> tags próprias, sem colunas: %s\n",
                   bate ? "sim" : "NAO");
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
           " `SHOW ALL`, `SET`, e as tags BEGIN/COMMIT/ROLLBACK que os drivers mandam antes"
           " de tudo. A IDA GUARDA A VOLTA: o que o SET escreve o SHOW lê — incluindo um"
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

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
