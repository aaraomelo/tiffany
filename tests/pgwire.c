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
#include <netinet/in.h>
#include <arpa/inet.h>
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pgwire_sess.h"
#include "pqlike.h"

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
            /* duas ligações: a boa e a do gume */
            for(n = 0; n < 2; n++){
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

            /* (3) O GUME: o erro tem de CHEGAR como erro, e não como zero linhas */
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
           " engana quem chama",
           mal == 0 && nrows_ref == 2 && ncols_ref == 3);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
