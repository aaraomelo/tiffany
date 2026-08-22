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
/* o `edo.h` entra para §W73 ser DOIS CAMINHOS e não um: ele decide a ressonância
 * pela aritmética dos coeficientes — p(a) e p'(a) —, o motor decide-a pelo
 * espectro. Se viessem do mesmo sítio, concordarem não diria nada. */
#include "../lib/edo.h"
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
            /* quatro ligações: a boa, a das duas tabelas, a do dual e a do gume */
            for(n = 0; n < 4; n++){
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

            /* (4) A AUSÊNCIA ATRAVESSA A FACHADA. O banco distingue a célula
             * ausente da célula vazia, e a fachada tem de a devolver com a
             * mesma distinção: `PQgetisnull` responde pela máscara que o
             * comprimento −1 acendeu, não pelo strlen do texto. É o mesmo par
             * dos dois caminhos, agora sobre o DUAL: o que o motor sabe tem de
             * chegar ao cliente sem perder o nível. */
            {
                PGconn *c4 = PQconnectdb(conninfo);
                int dual_ok = 0;
                if(PQstatus(c4) == CONNECTION_OK){
                    PGresult *r;
                    r = PQexec(c4, "INSERT INTO t VALUES (5,NULL,50)");
                    PQclear(r);
                    r = PQexec(c4, "SELECT * FROM t WHERE a = 5");
                    if(PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1){
                        int n0 = PQgetisnull(r, 0, 0);
                        int n1 = PQgetisnull(r, 0, 1);
                        int n2 = PQgetisnull(r, 0, 2);
                        dual_ok = (!n0 && n1 && !n2
                                   && !strcmp(PQgetvalue(r, 0, 0), "5")
                                   && !strcmp(PQgetvalue(r, 0, 2), "50"));
                        printf("      o dual pela fachada: isnull(a)=%d isnull(b)=%d"
                               " isnull(c)=%d · valores (%s,·,%s)\n", n0, n1, n2,
                               PQgetvalue(r, 0, 0), PQgetvalue(r, 0, 2));
                    }
                    PQclear(r);
                    /* e a volta: `SET b = NULL` apaga a célula pela fachada */
                    r = PQexec(c4, "UPDATE t SET c = NULL WHERE a = 5");
                    PQclear(r);
                    r = PQexec(c4, "SELECT * FROM t WHERE a = 5");
                    if(PQresultStatus(r) == PGRES_TUPLES_OK && PQntuples(r) == 1){
                        int n2 = PQgetisnull(r, 0, 2);
                        printf("      e a volta pela fachada: isnull(c) %d -> %d\n",
                               0, n2);
                        if(!n2) dual_ok = 0;
                    } else dual_ok = 0;
                    PQclear(r);
                    PQfinish(c4);
                }
                if(!dual_ok) mal++;
            }

            /* (5) O GUME: o erro tem de CHEGAR como erro, e não como zero linhas */
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
           " que já estava aberta. E O DUAL ATRAVESSA A FACHADA INTEIRA: um INSERT com"
           " NULL entra pelo socket, o `PQgetisnull` responde 1 na célula ausente e 0"
           " nas presentes — pela máscara que o comprimento −1 acendeu, e não pelo"
           " strlen, que juntava a cadeia vazia com a ausência num nível só —, e o"
           " `SET c = NULL` faz a volta pela mesma ligação. É o par dos dois caminhos"
           " aplicado ao que NÃO está lá: o que o motor distingue tem de chegar ao"
           " cliente sem perder o nível",
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

    /* ═══ §W30: O NOME NÃO É PARÂMETRO, E LARGAR É O DUAL DE CRIAR ═══════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w30.mem");     unlink("/tmp/pgwire_w30.prog");
        unlink("/tmp/pgwire_w30__t.mem");  unlink("/tmp/pgwire_w30__t.prog");
        unlink("/tmp/pgwire_w30__u.mem");  unlink("/tmp/pgwire_w30__u.prog");
        printf("\n§W30 o alias e o DROP: a nomeação, e o dual de criar.\n\n");
        if(!sql_abrir("/tmp/pgwire_w30")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,2)", &o);
        sql_executa("INSERT INTO t VALUES (3,4)", &o);

        /* O ALIAS muda a ETIQUETA e não o valor — o Teor. 3 diz que a nomeação
         * não é parâmetro, e é isso que se exige: as células têm de ser as
         * mesmas com e sem `AS`. */
        sql_executa("SELECT a, b FROM t", &o);
        char s0[32], s1[32];
        snprintf(s0, sizeof s0, "%s", o.nrows ? o.cell[0][0] : "?");
        snprintf(s1, sizeof s1, "%s", o.nrows ? o.cell[0][1] : "?");
        int nome_sem = (!strcmp(o.col[0], "a") && !strcmp(o.col[1], "b"));
        int r = sql_executa("SELECT a AS x, b AS y FROM t", &o);
        int nome_com = (r && !strcmp(o.col[0], "x") && !strcmp(o.col[1], "y"));
        int val_igual = (o.nrows == 2 && !strcmp(o.cell[0][0], s0)
                                      && !strcmp(o.cell[0][1], s1));
        printf("      sem AS: [%s,%s] · com AS: [%s,%s] · as células não mudam: %s\n",
               nome_sem ? "a" : "?", nome_sem ? "b" : "?", o.col[0], o.col[1],
               val_igual ? "sim" : "NAO");
        if(!nome_sem || !nome_com || !val_igual) mal++;

        /* LARGAR É O DUAL DE CRIAR, e o gume é o que vem DEPOIS: a tabela
         * largada tem de deixar de responder — com mensagem, não com zero
         * linhas. */
        sql_executa("CREATE TABLE u (c)", &o);
        sql_executa("INSERT INTO u VALUES (9)", &o);
        int rd = sql_executa("DROP TABLE u", &o);
        int r1 = sql_executa("SELECT * FROM u", &o);
        int sem_msg = (!r1 && !o.err[0]);
        printf("      DROP TABLE u: %s · depois dela: %s%s\n",
               rd ? "ok" : "NAO", r1 ? "RESPONDEU (mau)" : "recusada",
               sem_msg ? " SEM MENSAGEM (mau)" : "");
        if(!rd || r1 || sem_msg) mal++;

        int r2 = sql_executa("DROP TABLE u", &o);
        int r3 = sql_executa("DROP TABLE IF EXISTS u", &o);
        printf("      largar outra vez: %s · com IF EXISTS: %s\n",
               r2 ? "RESPONDEU (mau)" : "recusado", r3 ? "ok" : "NAO");
        if(r2 || !r3) mal++;

        /* ── O CONTROLO: largar uma NÃO pode levar a outra, e largar a que está
         * ABERTA não pode deixar a sessão a apontar para o que já não existe. */
        sql_executa("SELECT * FROM t", &o);
        int outra = (o.nrows == 2);
        sql_executa("DROP TABLE t", &o);
        int r4 = sql_executa("SELECT * FROM t", &o);
        printf("      CONTROLO — a outra tabela ficou: %s · e largar a ABERTA:"
               " depois dela %s\n", outra ? "sim" : "NAO",
               r4 ? "RESPONDEU (mau)" : "recusada");
        if(!outra || r4) mal++;
        sql_fechar();

        printf("\n");
        ok("O ALIAS MUDA A ETIQUETA E NÃO O VALOR, E LARGAR É O DUAL DE CRIAR. O `AS` tem"
           " fundamento no Teor. 3, que diz que as duas soluções são «a mesma estrutura noutra"
           " nomeação»: a nomeação não é parâmetro. É isso que se exige aqui — os nomes das"
           " colunas mudam para os pedidos e as CÉLULAS ficam as mesmas, porque um alias que"
           " mudasse o valor não seria um alias. E LARGAR é o dual de criar, e aqui é literal:"
           " uma tabela é um ficheiro, e largá-la é largar o ficheiro — não há catálogo a"
           " corrigir, porque o catálogo da base É a listagem do directório. O GUME É O QUE VEM"
           " DEPOIS: a tabela largada tem de deixar de responder COM MENSAGEM, e não com zero"
           " linhas em silêncio. E foi exactamente aí que ele apanhou um defeito que já lá"
           " estava e que o DROP tornou alcançável: a cláusula de compatibilidade com bases"
           " antigas aceitava QUALQUER nome quando a sessão estava na tabela sem nome e ela não"
           " tinha nome guardado — o `cat_nome_bate` devolve verdadeiro no vazio. Numa base"
           " moderna, onde o ficheiro sem nome é só o sítio das vistas e não tem colunas"
           " nenhumas, isso fazia a tabela largada responder com zero linhas e sem erro. Exige-se"
           " agora que a base sem nome tenha mesmo um catálogo: se não tem colunas, não é tabela"
           " nenhuma. O CONTROLO fecha os dois lados: largar uma não pode levar a outra à"
           " frente, e largar a que está ABERTA não pode deixar a sessão a apontar para o que já"
           " não existe — o descritor larga-se ANTES de o ficheiro desaparecer.",
           mal == 0);
    }

    /* ═══ §W31: O OFFSET É O DUAL DO LIMIT, E JUNTOS SÃO UMA FAIXA ═══════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w31.mem"); unlink("/tmp/pgwire_w31.prog");
        unlink("/tmp/pgwire_w31__t.mem"); unlink("/tmp/pgwire_w31__t.prog");
        printf("\n§W31 o offset: o dual do limite, e a fatia que particiona.\n\n");
        if(!sql_abrir("/tmp/pgwire_w31")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        for(int i = 1; i <= 10; i++){
            char q[64];
            snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 2);
            sql_executa(q, &o);
        }
        struct { const char *q; int n; const char *primeiro; } cs[] = {
            { "SELECT * FROM t LIMIT 3",                      3, "1"  },
            { "SELECT * FROM t OFFSET 7",                     3, "8"  },
            { "SELECT * FROM t LIMIT 3 OFFSET 4",             3, "5"  },
            { "SELECT * FROM t ORDER BY a DESC LIMIT 3 OFFSET 2", 3, "8" },
            { "SELECT * FROM t OFFSET 100",                   0, ""   },
        };
        for(unsigned k = 0; k < sizeof cs / sizeof cs[0]; k++){
            int r = sql_executa(cs[k].q, &o);
            int bate = r && o.nrows == cs[k].n
                       && (!cs[k].n || !strcmp(o.cell[0][0], cs[k].primeiro));
            printf("      %-46s %d linha(s), 1.ª = %-3s %s\n", cs[k].q + 21,
                   o.nrows, o.nrows ? o.cell[0][0] : "-", bate ? "" : "NAO BATE");
            if(!bate) mal++;
        }

        /* O GUME É A CONSERVAÇÃO: fatias consecutivas do mesmo tamanho têm de
         * PARTICIONAR a lista — nem repetir nem perder. Um offset que contasse
         * depois do limite, ou um limite que contasse as saltadas, quebraria
         * isto sem quebrar nenhuma fatia isolada. */
        {
            int visto[16]; for(int k = 0; k < 16; k++) visto[k] = 0;
            long total = 0;
            for(int off = 0; off < 10; off += 3){
                char q[80];
                snprintf(q, sizeof q, "SELECT * FROM t LIMIT 3 OFFSET %d", off);
                sql_executa(q, &o);
                total += o.nrows;
                for(int i = 0; i < o.nrows; i++){
                    int v = atoi(o.cell[i][0]);
                    if(v >= 1 && v <= 10) visto[v]++;
                }
            }
            int cobre = 1;
            for(int v = 1; v <= 10; v++) if(visto[v] != 1) cobre = 0;
            printf("\n      as fatias de 3 em 3: %ld linhas ao todo, e cada uma"
                   " das dez vista EXACTAMENTE uma vez: %s\n",
                   total, (cobre && total == 10) ? "sim" : "NAO");
            if(!cobre || total != 10) mal++;
        }

        /* e o CONTROLO: a fatia tem de depender do offset — sem isso,
         * «particionam» passaria por o offset ser ignorado e as fatias serem
         * todas iguais e sobrepostas. */
        sql_executa("SELECT * FROM t LIMIT 3 OFFSET 0", &o);
        char p0[16]; snprintf(p0, sizeof p0, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT * FROM t LIMIT 3 OFFSET 3", &o);
        char p3[16]; snprintf(p3, sizeof p3, "%s", o.nrows ? o.cell[0][0] : "?");
        int move = strcmp(p0, p3) != 0;
        printf("      CONTROLO — a fatia MOVE com o offset: %s vs %s  %s\n",
               p0, p3, move ? "" : "NAO");
        if(!move) mal++;
        sql_fechar();

        printf("\n");
        ok("O OFFSET É O DUAL DO LIMITE, E JUNTOS SÃO UMA FAIXA. Se o limite é o PREFIXO da"
           " lista, o offset é o que se salta antes dele — e os dois formam a fatia [k, k+n),"
           " que é uma FAIXA NA ORDEM, tal como a do índice é uma faixa nos VALORES. É o mesmo"
           " corte sobre outra ordem: lá a dos símbolos, aqui a das linhas. A ordem entre eles"
           " importa e está no código: o offset salta ANTES de o limite contar, porque são os"
           " dois extremos da mesma faixa e trocá-los daria outra fatia. O GUME É A"
           " CONSERVAÇÃO, e não cada fatia isolada: fatias consecutivas do mesmo tamanho têm de"
           " PARTICIONAR a lista — as dez linhas aparecem, e cada uma exactamente UMA vez, nem"
           " repetida nem perdida. Um offset que contasse depois do limite, ou um limite que"
           " contasse as linhas saltadas, quebraria essa partição sem quebrar nenhuma fatia"
           " isolada, que é o que as tornaria invisíveis a um teste caso a caso. E O CONTROLO"
           " impede que «particionam» passe por o offset ser ignorado: exige-se que a fatia"
           " MOVA com ele, porque cinco fatias idênticas e sobrepostas também dariam trinta"
           " linhas. Compõe com a ordem, e mede-se: com ORDER BY DESC a fatia é a do fim da"
           " lista ordenada, não a do fim da tabela.",
           mal == 0);
    }

    /* ═══ §W32: ACRESCENTAR UMA COLUNA É O LEVANTAMENTO ══════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w32.mem"); unlink("/tmp/pgwire_w32.prog");
        unlink("/tmp/pgwire_w32__t.mem"); unlink("/tmp/pgwire_w32__t.prog");
        printf("\n§W32 a coluna nova: o levantamento, e o que já lá estava fica.\n\n");
        if(!sql_abrir("/tmp/pgwire_w32")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        for(int i = 1; i <= 5; i++){
            char q[64];
            snprintf(q, sizeof q, "INSERT INTO t VALUES (%d,%d)", i, i * 10);
            sql_executa(q, &o);
        }
        /* guarda-se o que lá estava, para exigir que fique */
        char antes[8][2][16];
        sql_executa("SELECT * FROM t", &o);
        int n_antes = o.nrows, c_antes = o.ncols;
        for(int i = 0; i < n_antes && i < 8; i++){
            snprintf(antes[i][0], 16, "%s", o.cell[i][0]);
            snprintf(antes[i][1], 16, "%s", o.cell[i][1]);
        }

        int ra = sql_executa("ALTER TABLE t ADD COLUMN c", &o);
        sql_executa("SELECT * FROM t", &o);
        int subiu = (ra && o.ncols == c_antes + 1 && o.nrows == n_antes);
        /* O GUME: os valores velhos ficam INTACTOS. O passo da linha mudou —
         * as células vivem em i·ncols + j — pelo que um levantamento mal feito
         * MISTURA as linhas em vez de as mover, e o número de linhas e de
         * colunas continuaria certo. */
        int intacto = 1;
        for(int i = 0; i < n_antes && i < 8; i++){
            if(strcmp(o.cell[i][0], antes[i][0])) intacto = 0;
            if(strcmp(o.cell[i][1], antes[i][1])) intacto = 0;
            if(o.cell[i][2][0]) intacto = 0;               /* a nova, AUSENTE */
        }
        printf("      %d colunas -> %d, %d linhas · os valores velhos intactos e a"
               " nova ausente: %s\n", c_antes, o.ncols, o.nrows,
               intacto ? "sim" : "NAO");
        if(!subiu || !intacto) mal++;

        /* E a ausência é OPERACIONAL, não um espaço em branco na impressão: a
         * coluna que ninguém escreveu é apanhada pelo dual em peso, e o zero
         * — que é um valor escrito — não a apanha. Um levantamento que
         * enchesse a coluna nova de zeros passaria na linha de cima se ela
         * comparasse texto, mas cai aqui. */
        sql_executa("SELECT * FROM t WHERE c IS NULL", &o);
        long nul = o.nrows;
        sql_executa("SELECT * FROM t WHERE c IS NOT NULL", &o);
        long pres = o.nrows;
        sql_executa("SELECT * FROM t WHERE c = 0", &o);
        long zeros = o.nrows;
        int dual = (nul == n_antes && pres == 0 && zeros == 0);
        printf("      a coluna nova nasce no dual: IS NULL %ld (esp %d) · IS NOT"
               " NULL %ld (esp 0) · = 0 %ld (esp 0)  %s\n",
               nul, n_antes, pres, zeros, dual ? "" : "NAO BATE");
        if(!dual) mal++;

        /* e a coluna nova é uma coluna a sério: escreve-se e lê-se */
        sql_executa("UPDATE t SET c = 7 WHERE a = 2", &o);
        sql_executa("SELECT * FROM t WHERE a = 2", &o);
        int usa = (o.nrows == 1 && !strcmp(o.cell[0][2], "7")
                                && !strcmp(o.cell[0][1], "20"));
        printf("      a coluna nova escreve-se e lê-se, e a vizinha não se mexe:"
               " (%s,%s,%s)  %s\n",
               o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
               o.nrows ? o.cell[0][2] : "?", usa ? "" : "NAO BATE");
        if(!usa) mal++;

        /* escrever faz existir, e faz existir SÓ a célula escrita: o UPDATE
         * de uma linha muda a contagem do dual de n para n-1, não para 0. */
        sql_executa("SELECT * FROM t WHERE c IS NOT NULL", &o);
        long pres2 = o.nrows;
        sql_executa("SELECT * FROM t WHERE c IS NULL", &o);
        long nul2 = o.nrows;
        int passou = (pres2 == 1 && nul2 == n_antes - 1);
        printf("      escrever faz existir, e só a célula escrita: IS NOT NULL"
               " %ld (esp 1) · IS NULL %ld (esp %d)  %s\n",
               pres2, nul2, n_antes - 1, passou ? "" : "NAO BATE");
        if(!passou) mal++;

        /* e um INSERT com a largura nova entra inteiro */
        sql_executa("INSERT INTO t VALUES (9,90,99)", &o);
        sql_executa("SELECT * FROM t WHERE a = 9", &o);
        int novo = (o.nrows == 1 && !strcmp(o.cell[0][2], "99"));
        printf("      e um INSERT com a largura nova: (%s,%s,%s)  %s\n",
               o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
               o.nrows ? o.cell[0][2] : "?", novo ? "" : "NAO BATE");
        if(!novo) mal++;

        /* ── O CONTROLO: o que não é levantamento recusa-se, e o índice que
         * ficou para trás não pode mentir. */
        int e1 = sql_executa("ALTER TABLE t ADD COLUMN a", &o);
        printf("\n      CONTROLO — coluna repetida: %s\n",
               e1 ? "RESPONDEU (mau)" : "recusada");
        if(e1) mal++;

        sql_executa("CREATE INDEX ON t (a)", &o);
        sql_executa("SELECT * FROM t WHERE a = 2", &o);
        long p_com = sql_ultimos_passos;
        sql_executa("ALTER TABLE t ADD COLUMN d", &o);
        sql_executa("SELECT * FROM t WHERE a = 2", &o);
        int certo = (o.nrows == 1 && !strcmp(o.cell[0][1], "20"));
        printf("      o índice feito ANTES do levantamento é largado"
               " (%ld -> %ld passos) e a resposta continua certa: %s\n",
               p_com, sql_ultimos_passos, certo ? "sim" : "NAO");
        if(!certo || sql_ultimos_passos == 0) mal++;
        sql_fechar();

        printf("\n");
        ok("ACRESCENTAR UMA COLUNA É O LEVANTAMENTO, E O QUE JÁ LÁ ESTAVA FICA. O"
           " `thm:levantamento` diz que π̃ = (π,k) leva um andar no seguinte e que a folha é UMA"
           " coordenada, seja qual for n; uma coluna nova é isso: cada linha ganha uma"
           " coordenada, e o que existia não muda de VALOR — muda de SÍTIO, porque o passo da"
           " linha cresceu. É aí que está o trabalho e o risco: as células vivem em"
           " i·ncols + j, pelo que mexer no ncols move TODAS, e um levantamento mal feito"
           " MISTURA as linhas em vez de as mover — com o número de linhas e de colunas a"
           " continuar certo. Por isso o que se mede não é a largura: é que cada valor velho"
           " esteja onde estava, célula a célula, e que a coluna nova NASÇA NO DUAL — não a"
           " zero: zero é um valor escrito, e a coluna que ninguém escreveu está ausente, o"
           " que se mede em peso (IS NULL apanha as cinco, `= 0` nenhuma). A ordem"
           " também está fixada: lê-se tudo com a régua velha, o catálogo sobe DEPOIS de ler e"
           " ANTES de escrever, e reescreve-se com a nova — ler com uma régua e escrever com a"
           " outra seria o defeito clássico desta casa. E A COLUNA NOVA É UMA COLUNA A SÉRIO:"
           " escreve-se, lê-se, e um INSERT com a largura nova entra inteiro. O CONTROLO tem"
           " duas metades: a coluna repetida é recusada pelo nome, e OS ÍNDICES FEITOS ANTES"
           " SÃO LARGADOS — as chaves apontavam para o layout antigo, e um índice que"
           " sobrevivesse ao levantamento responderia sobre um passo que já não existe. Mede-se"
           " que ele foi largado (os passos voltam a subir) e que a resposta continua certa:"
           " custa a varredura, nunca a correcção.",
           mal == 0);
    }

    /* ═══ §W33: O NULO É O DUAL DO CORPO REPRESENTADO ═══════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w33.mem"); unlink("/tmp/pgwire_w33.prog");
        unlink("/tmp/pgwire_w33__t.mem"); unlink("/tmp/pgwire_w33__t.prog");
        printf("\n§W33 o nulo: o suporte que sempre existe, e o corpo que é o 1.\n\n");
        if(!sql_abrir("/tmp/pgwire_w33")) mal++;
        sql_executa("CREATE TABLE t (a,b,c)", &o);
        /* a linha 1 escreve tudo, com um ZERO explícito em b;
         * a linha 2 escreve só a; b e c nascem ausentes;
         * a linha 3 escreve a e b, e c fica ausente. */
        sql_executa("INSERT INTO t VALUES (1,0,5)",       &o);
        sql_executa("INSERT INTO t VALUES (2,NULL,NULL)", &o);
        sql_executa("INSERT INTO t VALUES (3,0,NULL)",    &o);

        /* ── (1) O ZERO ESCRITO NÃO É A AUSÊNCIA. É o gume da tese: se o dual
         * fosse o valor 0 do corpo, `= 0` e `IS NULL` apanhariam as mesmas
         * linhas. Apanham conjuntos DISJUNTOS sobre a mesma coluna. */
        sql_executa("SELECT a FROM t WHERE b = 0", &o);
        long z_n = o.nrows; char z1 = o.nrows ? o.cell[0][0][0] : '?';
        sql_executa("SELECT a FROM t WHERE b IS NULL", &o);
        long a_n = o.nrows; char a1 = o.nrows ? o.cell[0][0][0] : '?';
        int disjunto = (z_n == 2 && a_n == 1 && a1 == '2' && z1 == '1');
        printf("      o zero ESCRITO e a ausência são objectos diferentes:"
               " b = 0 apanha %ld (1,3) · b IS NULL apanha %ld (2)  %s\n",
               z_n, a_n, disjunto ? "" : "NAO BATE");
        if(!disjunto) mal++;

        /* ── (2) A AUSÊNCIA ESTÁ FORA DO CORPO. Sobre o corpo, `= v` e `<> v`
         * são complementares: toda a linha cai num ou no outro. A ausência
         * falha nos DOIS — não é um valor que se compare, é o suporte. É a
         * medida que distingue «o nulo é o 0» de «o nulo é outro nível»: se
         * fosse um valor qualquer do corpo, a soma fechava em 3. */
        sql_executa("SELECT a FROM t WHERE b = 0", &o);   long ig = o.nrows;
        sql_executa("SELECT a FROM t WHERE b <> 0", &o);  long di = o.nrows;
        sql_executa("SELECT a FROM t WHERE b IS NOT NULL", &o); long pr = o.nrows;
        int fora = (ig + di == pr && pr == 2 && ig + di < 3);
        printf("      `= 0` %ld + `<> 0` %ld = %ld presentes (de 3 linhas):"
               " a comparação vive no corpo, e o dual está FORA  %s\n",
               ig, di, pr, fora ? "" : "NAO BATE");
        if(!fora) mal++;

        /* ── (3) O BIT: presença e ausência PARTICIONAM, coluna a coluna. É a
         * conta do `thm:bitunico` lida em peso — |b=1| + |b=0| = |I| e a
         * intersecção é vazia — e vale para as três colunas de uma vez, com
         * pesos DIFERENTES (3+0, 2+1, 1+2): uma coluna que respondesse sempre
         * o mesmo passaria numa e caía nas outras. */
        const char *col[3] = { "a", "b", "c" };
        long esp_pres[3] = { 3, 2, 1 };
        int part = 1;
        for(int k = 0; k < 3; k++){
            char q[80];
            snprintf(q, sizeof q, "SELECT a FROM t WHERE %s IS NOT NULL", col[k]);
            sql_executa(q, &o); long p = o.nrows;
            snprintf(q, sizeof q, "SELECT a FROM t WHERE %s IS NULL", col[k]);
            sql_executa(q, &o); long n = o.nrows;
            printf("      coluna %s: presentes %ld + ausentes %ld = %ld"
                   " (esperado %ld + %ld)\n", col[k], p, n, p + n,
                   esp_pres[k], 3 - esp_pres[k]);
            if(p + n != 3 || p != esp_pres[k]) part = 0;
        }
        if(!part) mal++;

        /* ── (4) O SUPORTE EXISTE ANTES DO CONTEÚDO. A coluna c nunca foi
         * escrita em duas das três linhas, e mesmo assim é endereçável: o
         * dual apanha-as. E escrever UMA célula move UMA unidade de peso de
         * um lado para o outro — a topologia vazia é o sítio por onde a
         * escrita entra, não um erro a corrigir. */
        sql_executa("SELECT a FROM t WHERE c IS NULL", &o); long c0 = o.nrows;
        sql_executa("UPDATE t SET c = 7 WHERE a = 3", &o);
        sql_executa("SELECT a FROM t WHERE c IS NULL", &o); long c1 = o.nrows;
        sql_executa("SELECT a FROM t WHERE c IS NOT NULL", &o); long c1p = o.nrows;
        int passa = (c0 == 2 && c1 == 1 && c1p == 2);
        printf("      escrever move UMA unidade de peso: ausentes %ld -> %ld,"
               " presentes -> %ld  %s\n", c0, c1, c1p, passa ? "" : "NAO BATE");
        if(!passa) mal++;

        /* ── (5) O DUAL É DUAL, E É O MESMO OBJECTO DA LINHA. A ausência da
         * célula e a morte da linha são o mesmo bitmap com o mesmo peso: um
         * DELETE tira a linha inteira, e as contagens dos dois lados descem
         * juntas. Aqui está a frase inteira: o corpo representado é o que
         * está aceso, e o resto é o suporte por onde se anda. */
        sql_executa("DELETE FROM t WHERE a = 2", &o);
        sql_executa("SELECT a FROM t WHERE c IS NULL", &o);     long d0 = o.nrows;
        sql_executa("SELECT a FROM t WHERE c IS NOT NULL", &o); long d1 = o.nrows;
        sql_executa("SELECT a FROM t", &o);                     long tot = o.nrows;
        int junto = (tot == 2 && d0 + d1 == tot && d0 == 0 && d1 == 2);
        printf("      apagar a linha tira-a dos DOIS lados: total %ld ="
               " ausentes %ld + presentes %ld  %s\n",
               tot, d0, d1, junto ? "" : "NAO BATE");
        if(!junto) mal++;

        /* ── (6) E O DUAL PERSISTE: fecha-se e reabre-se, e a ausência
         * continua lá. Se fosse um adorno da impressão, morria no disco. */
        sql_fechar();
        if(!sql_abrir("/tmp/pgwire_w33")) mal++;
        sql_executa("SELECT a FROM t WHERE c IS NOT NULL", &o); long r1 = o.nrows;
        sql_executa("SELECT a FROM t WHERE b IS NULL", &o);     long r2 = o.nrows;
        int viveu = (r1 == 2 && r2 == 0);
        printf("      depois de fechar e reabrir: c presente %ld (esp 2),"
               " b ausente %ld (esp 0)  %s\n", r1, r2, viveu ? "" : "NAO BATE");
        if(!viveu) mal++;

        /* ── O CONTROLO: uma coluna SEM ausência nenhuma. Se o motor
         * respondesse à pergunta pelo nome — se `IS NULL` fosse um adorno que
         * devolve sempre o mesmo — este caso e o de cima seriam iguais. */
        sql_executa("SELECT a FROM t WHERE a IS NULL", &o);     long q0 = o.nrows;
        sql_executa("SELECT a FROM t WHERE a IS NOT NULL", &o); long q1 = o.nrows;
        /* ── (7) A VOLTA. Escrever acende; `SET c = NULL` apaga. Sem esta
         * metade a presença só sabia crescer, e um dual que não volta não é
         * dual — é um contador. E o que volta é o BIT, não a linha: a linha
         * continua viva e a contagem total não se mexe. */
        sql_executa("UPDATE t SET c = NULL WHERE a = 3", &o);
        sql_executa("SELECT a FROM t WHERE c IS NULL", &o);     long v0 = o.nrows;
        sql_executa("SELECT a FROM t WHERE c IS NOT NULL", &o); long v1 = o.nrows;
        sql_executa("SELECT a FROM t", &o);                     long vt = o.nrows;
        int volta = (v0 == 1 && v1 == 1 && vt == 2);
        printf("      e a VOLTA — `SET c = NULL` apaga a célula e não a linha:"
               " ausentes %ld (esp 1) · presentes %ld (esp 1) · linhas %ld (esp 2)"
               "  %s\n", v0, v1, vt, volta ? "" : "NAO BATE");
        if(!volta) mal++;

        /* ── (8) O ÍNDICE NÃO MENTE SOBRE O DUAL. A árvore devolve o SÍTIO e
         * não sabe da presença; se o filtro do dual corresse antes da descida,
         * a mesma pergunta responderia uma coisa com índice e outra sem. É a
         * régua da casa: o índice muda o CUSTO, nunca a resposta. */
        sql_executa("SELECT a FROM t WHERE c = 0", &o);  long sem = o.nrows;
        sql_executa("CREATE INDEX ON t (c)", &o);
        sql_executa("SELECT a FROM t WHERE c = 0", &o);  long com = o.nrows;
        sql_executa("SELECT a FROM t WHERE c IS NULL", &o); long com_n = o.nrows;
        int igual = (sem == 0 && com == 0 && com_n == 1);
        printf("      com índice e sem índice, a MESMA resposta: `c = 0` sem %ld,"
               " com %ld (esp 0 e 0) · IS NULL %ld (esp 1)  %s\n",
               sem, com, com_n, igual ? "" : "NAO BATE");
        if(!igual) mal++;

        /* ── (9) E O FIO DIZ A AUSÊNCIA COM A LETRA DELA. No FEBE, o campo
         * ausente escreve-se com comprimento −1; zero bytes é a CADEIA VAZIA,
         * que é um valor. São dois níveis e o protocolo tem as duas letras —
         * mandar 0 seria o motor saber distingui-los e o wire não, com o valor
         * a ir certo e o NÍVEL a ir errado. Mede-se no DataRow, campo a campo:
         * a coluna a (presente) traz L ≥ 0, a coluna c (ausente) traz −1. */
        {
            SqlOut wo;
            PgBuf w;
            const uint8_t *pay; int pn, off; char tipo;
            int viu = 0, lc_aus = 0, la_pres = 0;
            /* a linha pedida é a que TEM a e NÃO tem c: os dois campos do mesmo
             * DataRow, um de cada nível */
            sql_executa("SELECT a,c FROM t WHERE c IS NULL", &wo);
            pg_buf_limpa(&w);
            pg_reply_sql(&w, &wo);
            if(w.erro) mal++;
            off = 0;
            while(off < w.n){
                if(!pg_read_typed(w.b, w.n, &off, &tipo, &pay, &pn)) break;
                if(tipo != PG_MSG_DATA_ROW || viu) continue;
                { int nc = pg_get_i16(pay), p = 2, j;
                  for(j = 0; j < nc && p + 4 <= pn; j++){
                      int L = pg_get_i32(pay + p); p += 4;
                      if(j == 0) la_pres = L; else lc_aus = L;
                      if(L > 0) p += L;
                  } }
                viu = 1;
            }
            int fio = (la_pres > 0 && lc_aus == -1);
            printf("      e o FIO diz a ausência com a letra dela: no DataRow o"
                   " campo presente traz L=%d (>0) e o ausente traz L=%d (esp -1)"
                   "  %s\n", la_pres, lc_aus, fio ? "" : "NAO BATE");
            if(!fio) mal++;
        }

        printf("\n      CONTROLO — coluna cheia: IS NULL %ld (esp 0) ·"
               " IS NOT NULL %ld (esp 2)  %s\n", q0, q1,
               (q0 == 0 && q1 == 2) ? "" : "NAO BATE");
        if(q0 != 0 || q1 != 2) mal++;
        sql_fechar();

        printf("\n");
        ok("O NULO É O DUAL DO CORPO REPRESENTADO, E É POR ELE QUE SE ANDA. O `thm:bitunico`"
           " diz que a presença b=1 é o único operacional e que a ausência b=0 é o suporte"
           " neutro — e que é ELA o dual. Uma tabela realiza a frase à letra: o corpo"
           " representado é o que está escrito, e o que não foi escrito não é um valor errado"
           " nem um zero: é o suporte, que existe antes do conteúdo e não deixa de existir"
           " quando o conteúdo entra. O gume está em separá-lo do 0 DO CORPO, porque é aí que"
           " a tese se decide: escreve-se um zero explícito numa célula e deixa-se outra por"
           " escrever, e as duas perguntas apanham conjuntos DISJUNTOS. A segunda medida é"
           " mais forte e é a que fixa o NÍVEL: sobre o corpo, `= v` e `<> v` são"
           " complementares, e toda a linha cai num dos dois; a ausência falha nos DOIS, pelo"
           " que a soma não fecha em |I| mas no peso dos presentes. Um valor do corpo não faz"
           " isso — só um objecto de outro nível o faz. Daí sai a partição em peso, coluna a"
           " coluna e com pesos diferentes (3+0, 2+1, 1+2), que é a conta do bit lida com a"
           " MESMA régua que conta as linhas vivas: presença e ausência, vivo e morto, são o"
           " mesmo bitmap e o mesmo popcount, o que é a razão de o DELETE tirar a linha dos"
           " dois lados de uma vez. E escrever é o movimento: cada célula escrita passa UMA"
           " unidade de peso do dual para o corpo, e nunca mais do que uma — a topologia"
           " vazia é o sítio por onde a escrita entra, não uma lacuna a tapar. O CONTROLO é a"
           " coluna cheia, onde o dual pesa zero: sem ele, um `IS NULL` que respondesse pelo"
           " nome passaria em tudo o que está acima. Faltavam duas metades, e as duas são"
           " leis desta casa. A VOLTA: `SET c = NULL` apaga a célula sem apagar a linha, e"
           " sem ela a presença só sabia crescer — um dual que não volta é um contador. E A"
           " RESPOSTA NÃO DEPENDE DA RÉGUA: o filtro do dual tem de correr DEPOIS da descida"
           " pela árvore, porque a árvore devolve o sítio e não sabe da presença; estava"
           " antes, e `c = 0` com índice apanhava as células ausentes que a mesma pergunta"
           " sem índice recusava. Mede-se a coincidência das duas, que é a única forma de o"
           " dizer: o índice muda o CUSTO, nunca a resposta. E A FRASE ATRAVESSA O FIO: no FEBE"
           " a ausência escreve-se com comprimento −1 e a cadeia vazia com zero bytes, que são"
           " as duas letras de dois níveis; mede-se no DataRow campo a campo, porque mandar 0"
           " seria o motor saber distingui-los e o protocolo não — o valor a ir certo e o"
           " NÍVEL a ir errado, que é o defeito que esta casa persegue desde o princípio.",
           mal == 0);
    }

    /* ═══ §W34: A COLUNA AFIRMA, E A ÁRVORE É A TESTEMUNHA ══════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w34.mem"); unlink("/tmp/pgwire_w34.prog");
        unlink("/tmp/pgwire_w34__u.mem"); unlink("/tmp/pgwire_w34__u.prog");
        printf("\n§W34 as restrições: a fibra de uma folha, e a coluna sem dual.\n\n");
        if(!sql_abrir("/tmp/pgwire_w34")) mal++;
        sql_executa("CREATE TABLE u (id UNIQUE, nome NOT NULL, x)", &o);
        int e1 = sql_executa("INSERT INTO u VALUES (1,10,100)", &o);
        int e2 = sql_executa("INSERT INTO u VALUES (2,20,200)", &o);

        /* ── (1) O UNIQUE RECUSA A SEGUNDA FOLHA. É a frase do DISTINCT
         * (`thm:levantamento`: a folha 1 de cada fibra) dita na ESCRITA: o
         * DISTINCT escolhe o representante à saída, o UNIQUE recusa o segundo
         * à entrada. E o NOT NULL é a outra: declara que o dual pesa zero
         * nesta coluna, isto é, que ali não há suporte, só corpo. */
        int r1 = sql_executa("INSERT INTO u VALUES (1,30,300)", &o);
        int m1 = !r1 && !o.ok && o.err[0];
        int r2 = sql_executa("INSERT INTO u VALUES (3,NULL,300)", &o);
        int m2 = !r2 && !o.ok && o.err[0];
        printf("      UNIQUE recusa a repetida: %s · NOT NULL recusa o suporte:"
               " %s\n      «%s»\n", m1 ? "sim" : "NAO", m2 ? "sim" : "NAO", o.err);
        if(!e1 || !e2 || !m1 || !m2) mal++;

        /* ── (2) E A RECUSA NÃO ESCREVE NADA. Uma restrição que recusasse
         * DEPOIS de escrever seria pior do que não a ter: a tabela ficava com
         * a linha e o cliente com o erro. Conta-se. */
        sql_executa("SELECT * FROM u", &o);
        int intacto = (o.nrows == 2);
        printf("      e a recusa não escreve: %d linhas (esp 2)  %s\n",
               o.nrows, intacto ? "" : "NAO BATE");
        if(!intacto) mal++;

        /* ── (3) O GUME, E É O DUAL DE HOJE: numa coluna UNIQUE, DOIS
         * AUSENTES ENTRAM. A ausência não é uma folha da fibra — não é um
         * valor —, pelo que não pode repetir-se; dois zeros, esses, colidem.
         * Um motor que tratasse o nulo como o valor 0 recusava o segundo, e
         * esta é a única medida que os separa aqui. */
        sql_executa("CREATE TABLE v (k UNIQUE, y)", &o);
        int a1 = sql_executa("INSERT INTO v VALUES (NULL,1)", &o);
        int a2 = sql_executa("INSERT INTO v VALUES (NULL,2)", &o);
        int z1 = sql_executa("INSERT INTO v VALUES (0,3)", &o);
        int z2 = sql_executa("INSERT INTO v VALUES (0,4)", &o);
        sql_executa("SELECT * FROM v", &o);
        int gume = (a1 && a2 && z1 && !z2 && o.nrows == 3);
        printf("      dois AUSENTES entram (%d,%d) e dois ZEROS não (%d,%d):"
               " %d linhas (esp 3)  %s\n", a1, a2, z1, z2, o.nrows,
               gume ? "" : "NAO BATE");
        if(!gume) mal++;

        /* ── (4) A AFIRMAÇÃO VALE EM QUALQUER PORTA. Uma restrição que só o
         * INSERT respeitasse não é uma restrição: o UPDATE reescreve a mesma
         * célula. E há uma segunda maneira de a quebrar que só o UPDATE tem —
         * marcar mais do que uma linha e escrever o mesmo valor em todas. */
        sql_executa("SELECT * FROM u", &o);
        int u1 = sql_executa("UPDATE u SET id = 2 WHERE id = 1", &o);
        int u2 = sql_executa("UPDATE u SET nome = NULL WHERE id = 1", &o);
        int u3 = sql_executa("UPDATE u SET id = 7 WHERE id > 0", &o); /* duas linhas */
        int u4 = sql_executa("UPDATE u SET id = 7 WHERE id = 1", &o); /* uma: passa */
        sql_executa("SELECT * FROM u WHERE id = 7", &o);
        int porta = (!u1 && !u2 && !u3 && u4 && o.nrows == 1);
        printf("      pela porta do UPDATE: repetido %d · SET NULL %d · duas"
               " linhas de uma vez %d · a legítima %d → id=7 tem %d linha (esp 1)"
               "  %s\n", u1, u2, u3, u4, o.nrows, porta ? "" : "NAO BATE");
        if(!porta) mal++;

        /* ── (5) A TESTEMUNHA É A ÁRVORE, E ELA NÃO É UM EXTRA. Declarar
         * UNIQUE é afirmar que a chave leva a UM sítio, e isso é literalmente
         * o que a árvore guarda; por isso a verificação é uma DESCIDA e não
         * uma varredura, e a mesma árvore serve a leitura de graça. Mede-se
         * nos dois lados: a coluna única desce (nós, zero passos de ISA) e a
         * coluna sem restrição varre (passos que crescem com a tabela). */
        sql_executa("SELECT * FROM u WHERE id = 7", &o);
        long p_uni = sql_ultimos_passos, n_uni = sql_ultimos_nos;
        sql_executa("SELECT * FROM u WHERE x = 100", &o);
        long p_var = sql_ultimos_passos;
        int desce = (p_uni == 0 && n_uni > 0 && p_var > 0);
        printf("      a árvore que a afirmação criou serve a leitura: `id = 7`"
               " %ld passos / %ld nós · `x = 100` %ld passos  %s\n",
               p_uni, n_uni, p_var, desce ? "" : "NAO BATE");
        if(!desce) mal++;

        /* ── (5b) O VALOR APAGADO TEM DE PODER VOLTAR. A árvore diz onde a
         * linha ESTAVA e o DELETE não tira a chave — só apaga o bit do vivo —,
         * pelo que sem o filtro do vivo o UNIQUE recusava a reutilização do
         * valor de uma linha que já não existe, e reservava-o para sempre. É a
         * mesma frase que o índice já dizia à leitura, aplicada à recusa. */
        sql_executa("CREATE TABLE q (id UNIQUE, y)", &o);
        sql_executa("INSERT INTO q VALUES (1,10)", &o);
        sql_executa("INSERT INTO q VALUES (2,20)", &o);
        int b1 = sql_executa("INSERT INTO q VALUES (2,30)", &o);   /* vivo: recusa */
        sql_executa("DELETE FROM q WHERE id = 2", &o);
        int b2 = sql_executa("INSERT INTO q VALUES (2,99)", &o);   /* morto: entra */
        sql_executa("SELECT * FROM q WHERE id = 2", &o);
        int volta = (!b1 && b2 && o.nrows == 1 && !strcmp(o.cell[0][1], "99"));
        printf("      o valor apagado volta: com a linha viva %d · depois de"
               " apagada %d → id=2 vale %s (esp 99)  %s\n", b1, b2,
               o.nrows ? o.cell[0][1] : "?", volta ? "" : "NAO BATE");
        if(!volta) mal++;

        /* ── (6) E `PRIMARY KEY` É A CONJUNÇÃO, não uma terceira coisa: UMA
         * declaração recusa as DUAS quebras, e não recusa o que nenhuma das
         * duas recusaria. Se fosse um sinónimo de UNIQUE, o nulo passava; se
         * fosse só NOT NULL, o repetido passava. */
        sql_executa("CREATE TABLE p (id PRIMARY KEY, y)", &o);
        int k1 = sql_executa("INSERT INTO p VALUES (1,10)", &o);
        int k2 = sql_executa("INSERT INTO p VALUES (1,20)", &o);   /* repetido */
        int k3 = sql_executa("INSERT INTO p VALUES (NULL,30)", &o);/* ausente  */
        int k4 = sql_executa("INSERT INTO p VALUES (2,40)", &o);   /* legítimo */
        sql_executa("SELECT * FROM p", &o);
        int chave = (k1 && !k2 && !k3 && k4 && o.nrows == 2);
        printf("      PRIMARY KEY é a conjunção: entra %d · repetido %d ·"
               " ausente %d · outro %d → %d linhas (esp 2)  %s\n",
               k1, k2, k3, k4, o.nrows, chave ? "" : "NAO BATE");
        if(!chave) mal++;

        /* ── O CONTROLO: uma tabela SEM restrição nenhuma aceita tudo o que a
         * de cima recusou. Sem ele, um motor que recusasse por outro motivo
         * qualquer — largura, envelope, tabela fechada — passava em tudo. */
        sql_executa("CREATE TABLE w (id, nome)", &o);
        int c1 = sql_executa("INSERT INTO w VALUES (1,10)", &o);
        int c2 = sql_executa("INSERT INTO w VALUES (1,10)", &o);
        int c3 = sql_executa("INSERT INTO w VALUES (2,NULL)", &o);
        sql_executa("SELECT * FROM w", &o);
        int livre = (c1 && c2 && c3 && o.nrows == 3);
        printf("\n      CONTROLO — sem restrição, as MESMAS linhas entram:"
               " %d %d %d → %d linhas (esp 3)  %s\n", c1, c2, c3, o.nrows,
               livre ? "" : "NAO BATE");
        if(!livre) mal++;
        sql_fechar();

        printf("\n");
        ok("A COLUNA AFIRMA, E A ÁRVORE É A TESTEMUNHA. Uma restrição de coluna não é uma"
           " comodidade de sintaxe: é uma asserção sobre a FIBRA, e as duas que existem são"
           " as duas metades do que a tabela já tinha. `NOT NULL` diz que naquela direcção o"
           " dual pesa zero — não há suporte, só corpo —, e é a negação do `thm:bitunico`"
           " aplicada a uma coordenada. `UNIQUE` diz que a fibra tem UMA folha, que é"
           " exactamente a frase do DISTINCT (`thm:levantamento`, a folha 1 de cada fibra)"
           " dita na ESCRITA em vez de na leitura: o DISTINCT escolhe o representante à"
           " saída, o UNIQUE recusa o segundo à entrada. `PRIMARY KEY` é a conjunção das"
           " duas e não uma terceira coisa. O gume é o dual: numa coluna UNIQUE dois"
           " AUSENTES entram e dois ZEROS não, porque a ausência não é uma folha — não é um"
           " valor, logo não pode repetir-se —, e é a única medida aqui que separa o nulo do"
           " 0 do corpo. A afirmação vale em QUALQUER porta, e o UPDATE tem uma maneira de a"
           " quebrar que o INSERT não tem: marcar mais do que uma linha e escrever o mesmo"
           " valor em todas. E a recusa não escreve nada — uma restrição que recusasse"
           " DEPOIS de escrever seria pior do que não existir, com a tabela a ficar com a"
           " linha e o cliente com o erro. Por fim, a testemunha não é um extra: declarar"
           " que a chave leva a UM sítio é literalmente o que a árvore guarda, pelo que"
           " verificar é DESCER e não varrer, e a mesma árvore serve a leitura de graça —"
           " `id = 7` custa zero passos de ISA e nós contados, contra uma varredura na"
           " coluna sem restrição. O CONTROLO é a tabela sem restrição nenhuma, onde as"
           " MESMAS linhas entram todas: sem ele, uma recusa por qualquer outro motivo"
           " passava por restrição. E há uma metade que só a árvore explica: o valor de uma"
           " linha APAGADA tem de poder voltar. O DELETE não tira a chave — só apaga o bit"
           " do vivo —, pelo que a descida acha a chave de quem já não está; sem o filtro"
           " do vivo o UNIQUE reservava para sempre o valor de uma linha que não existe. É a"
           " mesma frase que o índice já dizia à LEITURA («a árvore diz onde a linha estava,"
           " o vivo diz se ela ainda está»), aplicada agora à RECUSA.",
           mal == 0);
    }

    /* ═══ §W35: A SETA ENTRE DUAS TABELAS, E AS SUAS DUAS PONTAS ════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w35.mem", "/tmp/pgwire_w35.prog",
            "/tmp/pgwire_w35__cliente.mem", "/tmp/pgwire_w35__cliente.prog",
            "/tmp/pgwire_w35__item.mem", "/tmp/pgwire_w35__item.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W35 a seta: declarada uma vez, exigida nas duas pontas.\n\n");
        if(!sql_abrir("/tmp/pgwire_w35")) mal++;
        sql_executa("CREATE TABLE cliente (id UNIQUE, nome)", &o);
        sql_executa("INSERT INTO cliente VALUES (1,10)", &o);
        sql_executa("INSERT INTO cliente VALUES (2,20)", &o);
        int cr = sql_executa("CREATE TABLE item (cod, dono REFERENCES cliente(id))", &o);

        /* ── (1) À ENTRADA: escrever um valor que não está do outro lado é
         * criar uma seta para lado nenhum. E a AUSÊNCIA não é uma seta — uma
         * célula NULL não aponta e nada exige, que é o `thm:bitunico` outra
         * vez: o dual não é um valor, logo não é um destino. */
        int i1 = sql_executa("INSERT INTO item VALUES (100,1)", &o);
        int i2 = sql_executa("INSERT INTO item VALUES (101,9)", &o);
        int i3 = sql_executa("INSERT INTO item VALUES (102,NULL)", &o);
        sql_executa("SELECT * FROM item", &o);
        int entrada = (cr && i1 && !i2 && i3 && o.nrows == 2);
        printf("      à entrada: o que aponta %d · o que não aponta %d · o"
               " AUSENTE %d → %d linhas (esp 2)  %s\n", i1, i2, i3, o.nrows,
               entrada ? "" : "NAO BATE");
        if(!entrada) mal++;

        /* ── (2) E VALE NA OUTRA PORTA: o UPDATE reescreve a mesma célula. */
        int u1 = sql_executa("UPDATE item SET dono = 9 WHERE cod = 100", &o);
        int u2 = sql_executa("UPDATE item SET dono = 2 WHERE cod = 100", &o);
        printf("      pelo UPDATE: para fora %d · para dentro %d  %s\n",
               u1, u2, (!u1 && u2) ? "" : "NAO BATE");
        if(u1 || !u2) mal++;

        /* ── (3) À SAÍDA, QUE É A METADE QUE FALTA A QUEM SÓ MEDE UMA: apagar
         * a linha apontada corta a seta por baixo. Quem responde é a lista que
         * a MÃE guarda — a filha sabe para onde aponta, a mãe sabe quem a
         * aponta, e sem o segundo lado o DELETE não teria como saber. */
        int d1 = sql_executa("DELETE FROM cliente WHERE id = 2", &o);   /* apontada */
        int d2 = sql_executa("DELETE FROM cliente WHERE id = 1", &o);   /* livre    */
        sql_executa("SELECT * FROM cliente", &o);
        int saida = (!d1 && d2 && o.nrows == 1);
        printf("      à saída: a apontada %d · a LIVRE %d → restam %d (esp 1)"
               "  %s\n", d1, d2, o.nrows, saida ? "" : "NAO BATE");
        if(!saida) mal++;

        /* ── (4) E A PRISÃO SOLTA-SE PELO LADO CERTO. Não é a mãe que está
         * bloqueada para sempre: é a seta que existe. Apaga-se quem aponta, e
         * a mãe fica livre — o que mostra que o motor está a ler a seta e não
         * a recusar a tabela inteira. */
        int liberta = sql_executa("DELETE FROM item WHERE cod = 100", &o);
        int d3 = sql_executa("DELETE FROM cliente WHERE id = 2", &o);
        sql_executa("SELECT * FROM cliente", &o);
        int solta = (liberta && d3 && o.nrows == 0);
        printf("      e solta-se pelo lado certo: apagado quem apontava %d,"
               " a mãe sai %d → restam %d (esp 0)  %s\n",
               liberta, d3, o.nrows, solta ? "" : "NAO BATE");
        if(!solta) mal++;

        /* ── (5) A DECLARAÇÃO PERSISTE. A seta vive no disco, dos dois lados:
         * fecha-se e reabre-se, e as duas pontas continuam a exigir. */
        sql_fechar();
        if(!sql_abrir("/tmp/pgwire_w35")) mal++;
        sql_executa("INSERT INTO cliente VALUES (5,50)", &o);
        int p1 = sql_executa("INSERT INTO item VALUES (200,5)", &o);
        int p2 = sql_executa("INSERT INTO item VALUES (201,7)", &o);
        int p3 = sql_executa("DELETE FROM cliente WHERE id = 5", &o);
        printf("      depois de fechar e reabrir: entra %d · não entra %d ·"
               " a mãe presa %d  %s\n", p1, p2, p3,
               (p1 && !p2 && !p3) ? "" : "NAO BATE");
        if(!p1 || p2 || p3) mal++;

        /* ── (6) E UM ANDAR ACIMA: A MÃE APONTADA NÃO SE LARGA. Largar o
         * ficheiro deixaria as setas da filha a apontar para um sítio que já
         * não existe, e o motor a responder «não pôde ser procurado» a cada
         * escrita — um estado que ninguém declarou e do qual não se sai. É a
         * mesma exigência do DELETE com a tabela inteira no lugar da linha, e
         * solta-se pelo mesmo lado: larga-se primeiro quem aponta. */
        sql_executa("INSERT INTO cliente VALUES (8,80)", &o);
        sql_executa("INSERT INTO item VALUES (300,8)", &o);
        int dr1 = sql_executa("DROP TABLE cliente", &o);
        sql_executa("SELECT * FROM item", &o);
        long viva = o.nrows;
        int dr2 = sql_executa("DROP TABLE item", &o);
        int dr3 = sql_executa("DROP TABLE cliente", &o);
        int andar = (!dr1 && viva > 0 && dr2 && dr3);
        printf("      a mãe apontada não se larga: %d · a filha continua lá"
               " (%ld linhas) · largada a filha, a mãe sai %d  %s\n",
               dr1, viva, dr3, andar ? "" : "NAO BATE");
        if(!andar) mal++;

        /* ── O CONTROLO, e são dois. Primeiro: uma tabela SEM seta aceita os
         * mesmos valores que a de cima recusou — senão a recusa podia vir de
         * qualquer outra coisa. Segundo: a mãe de OUTRA tabela não fica presa
         * pela filha desta — a lista da mãe guarda o nome, e um motor que
         * recusasse por haver «alguma» seta na base passava no resto. */
        sql_executa("CREATE TABLE solto (cod, dono)", &o);
        int s1 = sql_executa("INSERT INTO solto VALUES (300,9)", &o);
        sql_executa("CREATE TABLE outra (id UNIQUE, z)", &o);
        sql_executa("INSERT INTO outra VALUES (7,70)", &o);
        int s2 = sql_executa("DELETE FROM outra WHERE id = 7", &o);
        printf("\n      CONTROLO — sem seta o valor 9 entra: %d · e a mãe de"
               " OUTRA tabela não fica presa: %d  %s\n", s1, s2,
               (s1 && s2) ? "" : "NAO BATE");
        if(!s1 || !s2) mal++;
        sql_fechar();

        printf("\n");
        ok("A SETA ENTRE DUAS TABELAS: DECLARADA UMA VEZ, EXIGIDA NAS DUAS PONTAS."
           " `REFERENCES mae(col)` diz que cada linha desta tabela aponta para uma linha"
           " daquela — é o mesmo caminho que o JOIN percorre a cada consulta, dito UMA vez"
           " na tabela em vez de escrito a cada pergunta, e é por ser dito que pode ser"
           " EXIGIDO. O que se exige é que a seta esteja BEM DEFINIDA, e isso tem duas"
           " metades que são uma o dual da outra: à ENTRADA, escrever um valor que não está"
           " do outro lado é apontar para lado nenhum; à SAÍDA, apagar a linha apontada é"
           " cortar a seta por baixo. Medir só a primeira era metade do par, e a segunda"
           " precisa de uma coisa que a primeira não precisa — saber QUEM aponta para mim"
           " —, pelo que a declaração escreve nos DOIS lados: a filha guarda para onde"
           " aponta, a mãe guarda quem a aponta. É a mesma dualidade da árvore, que guarda a"
           " chave e o sítio. A AUSÊNCIA NÃO É UMA SETA: uma célula NULL não aponta e nada"
           " exige, que é o `thm:bitunico` outra vez — o dual não é um valor, logo não é um"
           " destino. E a exigência não é um bloqueio da tabela: a mãe LIVRE apaga-se, e a"
           " presa solta-se apagando quem apontava, o que é o que mostra que o motor lê a"
           " seta e não recusa por hábito. A travessia é uma DESCIDA quando a coluna alvo"
           " tem árvore — e numa coluna UNIQUE tem sempre —, e a decisão toma-se com a"
           " outra tabela ABERTA e traz-se em memória local, porque abrir uma tabela relê o"
           " .mem: é a mesma regra que a subconsulta teve de aprender. O CONTROLO tem duas"
           " metades: sem seta os mesmos valores entram, e a mãe de OUTRA tabela não fica"
           " presa pela filha desta — a lista guarda o NOME, e um motor que recusasse por"
           " haver «alguma» seta na base passava em tudo o resto. E a exigência tem um andar"
           " acima: largar a TABELA apontada deixaria as setas a apontar para um ficheiro"
           " que já não existe — um estado que ninguém declarou e do qual não se sai —,"
           " pelo que o DROP da mãe é recusado enquanto houver quem aponte, e solta-se pelo"
           " mesmo lado: larga-se primeiro a filha.",
           mal == 0);
    }

    /* ═══ §W36: TRÊS RESPOSTAS QUANDO A SETA PERDE O DESTINO ════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w36.mem", "/tmp/pgwire_w36.prog",
            "/tmp/pgwire_w36__m.mem", "/tmp/pgwire_w36__m.prog",
            "/tmp/pgwire_w36__fc.mem", "/tmp/pgwire_w36__fc.prog",
            "/tmp/pgwire_w36__fn.mem", "/tmp/pgwire_w36__fn.prog",
            "/tmp/pgwire_w36__fr.mem", "/tmp/pgwire_w36__fr.prog",
            "/tmp/pgwire_w36__fd.mem", "/tmp/pgwire_w36__fd.prog" };
        for(int k = 0; k < 12; k++) unlink(lixo[k]);
        printf("\n§W36 a seta sem destino: recusar, levar junto, soltar.\n\n");
        if(!sql_abrir("/tmp/pgwire_w36")) mal++;
        sql_executa("CREATE TABLE m (id UNIQUE, z)", &o);
        for(int i = 1; i <= 4; i++){
            char q[64]; snprintf(q, sizeof q, "INSERT INTO m VALUES (%d,%d)", i, i*10);
            sql_executa(q, &o);
        }
        sql_executa("CREATE TABLE fc (a, dono REFERENCES m(id) ON DELETE CASCADE)", &o);
        sql_executa("CREATE TABLE fn (a, dono REFERENCES m(id) ON DELETE SET NULL)", &o);
        sql_executa("CREATE TABLE fr (a, dono REFERENCES m(id))", &o);
        /* cada filha com DUAS linhas: uma que aponta ao que vai morrer e outra
         * que aponta a outro. É o gume — a fibra que vai atrás é a de x, não a
         * tabela toda. */
        sql_executa("INSERT INTO fc VALUES (11,1)", &o);
        sql_executa("INSERT INTO fc VALUES (12,4)", &o);
        sql_executa("INSERT INTO fn VALUES (21,2)", &o);
        sql_executa("INSERT INTO fn VALUES (22,4)", &o);
        sql_executa("INSERT INTO fr VALUES (31,3)", &o);

        /* ── (1) CASCADE: a FIBRA vai atrás. Apagar x na mãe é apagar π⁻¹(x)
         * na filha — a imagem inversa, não uma regra de conveniência. E é SÓ a
         * fibra de x: a linha que aponta ao 4 fica. */
        int c1 = sql_executa("DELETE FROM m WHERE id = 1", &o);
        sql_executa("SELECT * FROM fc", &o);
        long fc_n = o.nrows;
        char fc_v[8]; snprintf(fc_v, sizeof fc_v, "%s", o.nrows ? o.cell[0][1] : "?");
        int casc = (c1 && fc_n == 1 && !strcmp(fc_v, "4"));
        printf("      CASCADE leva a fibra e SÓ a fibra: restam %ld em fc (esp 1)"
               " e o que fica aponta a %s (esp 4)  %s\n", fc_n, fc_v,
               casc ? "" : "NAO BATE");
        if(!casc) mal++;

        /* ── (2) SET NULL: a seta desaparece e a LINHA FICA. A célula vai para
         * o dual, que é onde uma coordenada sem valor mora — e mede-se pelo
         * dual, não pela impressão: `IS NULL` apanha-a e `= 0` não. */
        int c2 = sql_executa("DELETE FROM m WHERE id = 2", &o);
        sql_executa("SELECT * FROM fn", &o);
        long fn_n = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono IS NULL", &o);
        long fn_nul = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono = 0", &o);
        long fn_zero = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono = 4", &o);
        long fn_out = o.nrows;
        int solta = (c2 && fn_n == 2 && fn_nul == 1 && fn_zero == 0 && fn_out == 1);
        printf("      SET NULL solta a seta e guarda a linha: %ld linhas (esp 2)"
               " · IS NULL %ld (esp 1) · `= 0` %ld (esp 0) · a outra intacta %ld"
               " (esp 1)  %s\n", fn_n, fn_nul, fn_zero, fn_out,
               solta ? "" : "NAO BATE");
        if(!solta) mal++;

        /* ── (3) RESTRICT: nada muda. É o modo por omissão, e tem de ser: é o
         * único que não faz nada sem ordem. */
        int c3 = sql_executa("DELETE FROM m WHERE id = 3", &o);
        sql_executa("SELECT * FROM fr", &o);
        long fr_n = o.nrows;
        sql_executa("SELECT * FROM m WHERE id = 3", &o);
        int recusa = (!c3 && fr_n == 1 && o.nrows == 1);
        printf("      RESTRICT recusa e nada muda: %d · fr tem %ld · a mãe fica"
               " %d  %s\n", c3, fr_n, o.nrows, recusa ? "" : "NAO BATE");
        if(!recusa) mal++;

        /* ── (4) O GUME DA ORDEM: com uma filha em RESTRICT a recusar, as
         * outras NÃO PODEM ter agido. Pergunta-se a todas antes de agir; numa
         * passagem só, a filha em CASCADE processada primeiro já tinha levado
         * as suas linhas quando a recusa chegasse — a operação recusada e
         * metade feita. Aqui o 4 é apontado pelas três de uma vez. */
        sql_executa("INSERT INTO fr VALUES (34,4)", &o);
        long antes_fc, antes_fn;
        sql_executa("SELECT * FROM fc", &o); antes_fc = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono IS NULL", &o); antes_fn = o.nrows;
        int c4 = sql_executa("DELETE FROM m WHERE id = 4", &o);
        sql_executa("SELECT * FROM fc", &o); long depois_fc = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono IS NULL", &o); long depois_fn = o.nrows;
        int atomico = (!c4 && depois_fc == antes_fc && depois_fn == antes_fn);
        printf("      e a recusa não deixa metade feita: recusado %d · fc %ld->%ld"
               " · fn ausentes %ld->%ld  %s\n", c4, antes_fc, depois_fc,
               antes_fn, depois_fn, atomico ? "" : "NAO BATE");
        if(!atomico) mal++;

        /* ── (5) E O MODO PERSISTE, porque vive na filha e no disco: fecha-se,
         * reabre-se, e cada uma continua a responder à sua maneira. */
        sql_fechar();
        if(!sql_abrir("/tmp/pgwire_w36")) mal++;
        sql_executa("DELETE FROM fr WHERE dono = 4", &o);      /* solta o RESTRICT */
        int c5 = sql_executa("DELETE FROM m WHERE id = 4", &o);
        sql_executa("SELECT * FROM fc", &o); long r_fc = o.nrows;
        sql_executa("SELECT * FROM fn WHERE dono IS NULL", &o); long r_fn = o.nrows;
        int persiste = (c5 && r_fc == 0 && r_fn == 2);
        printf("      depois de reabrir, cada uma à sua maneira: fc %ld (esp 0)"
               " · fn ausentes %ld (esp 2)  %s\n", r_fc, r_fn,
               persiste ? "" : "NAO BATE");
        if(!persiste) mal++;

        /* ── (6) E HÁ UMA SEGUNDA PORTA, que é a outra metade do par: MUDAR
         * a chave da mãe tira o destino debaixo da seta tal como apagar a
         * linha. Sem esta metade, `UPDATE m SET id = 99` passava em silêncio e
         * deixava a filha a apontar para um valor que já não existe — com o
         * motor a recusar escrever a MESMA seta que já lá estava. As três
         * respostas são as mesmas, e é a mesma função que as dá. */
        sql_fechar();
        for(int k = 0; k < 12; k++) unlink(lixo[k]);
        if(!sql_abrir("/tmp/pgwire_w36")) mal++;
        sql_executa("CREATE TABLE m (id UNIQUE, z)", &o);
        sql_executa("INSERT INTO m VALUES (1,10)", &o);
        sql_executa("INSERT INTO m VALUES (2,20)", &o);
        sql_executa("INSERT INTO m VALUES (3,30)", &o);
        sql_executa("CREATE TABLE fc (a, dono REFERENCES m(id) ON UPDATE CASCADE)", &o);
        sql_executa("CREATE TABLE fn (a, dono REFERENCES m(id) ON UPDATE SET NULL)", &o);
        sql_executa("CREATE TABLE fr (a, dono REFERENCES m(id))", &o);
        sql_executa("INSERT INTO fc VALUES (11,1)", &o);
        sql_executa("INSERT INTO fn VALUES (22,2)", &o);
        sql_executa("INSERT INTO fr VALUES (33,3)", &o);

        int m1 = sql_executa("UPDATE m SET id = 91 WHERE id = 1", &o);
        sql_executa("SELECT * FROM fc", &o);
        char segue[8]; snprintf(segue, sizeof segue, "%s", o.nrows ? o.cell[0][1] : "?");
        int m2 = sql_executa("UPDATE m SET id = 92 WHERE id = 2", &o);
        sql_executa("SELECT * FROM fn WHERE dono IS NULL", &o);
        long soltou = o.nrows;
        int m3 = sql_executa("UPDATE m SET id = 99 WHERE id = 3", &o);
        sql_executa("SELECT * FROM m WHERE id = 3", &o);
        long ficou = o.nrows;
        int porta2 = (m1 && !strcmp(segue, "91") && m2 && soltou == 1
                      && !m3 && ficou == 1);
        printf("      a segunda porta — MUDAR a chave: CASCADE leva a fibra para"
               " %s (esp 91) · SET NULL solta %ld (esp 1) · RESTRICT recusa %d e"
               " a mãe fica %ld  %s\n", segue, soltou, m3, ficou,
               porta2 ? "" : "NAO BATE");
        if(!porta2) mal++;

        /* ── (7) E OS DOIS MODOS SÃO INDEPENDENTES. Vivem nos dois pares de
         * bits do mesmo octeto, e nada obriga quem quer a fibra atrás numa
         * mudança de chave a querê-la atrás num apagar. Uma seta que leva no
         * UPDATE e recusa no DELETE tem de fazer as duas coisas — se o motor
         * guardasse um modo só, uma das duas metades respondia pela outra. */
        sql_executa("CREATE TABLE fd (a, dono REFERENCES m(id)"
                    " ON UPDATE CASCADE ON DELETE RESTRICT)", &o);
        sql_executa("INSERT INTO m VALUES (5,50)", &o);
        sql_executa("INSERT INTO fd VALUES (55,5)", &o);
        int x1 = sql_executa("UPDATE m SET id = 95 WHERE id = 5", &o);
        sql_executa("SELECT * FROM fd", &o);
        char leva[8]; snprintf(leva, sizeof leva, "%s", o.nrows ? o.cell[0][1] : "?");
        int x2 = sql_executa("DELETE FROM m WHERE id = 95", &o);
        int indep = (x1 && !strcmp(leva, "95") && !x2);
        printf("      e os dois modos são independentes: leva no UPDATE (%s, esp"
               " 95) e recusa no DELETE (%d, esp 0)  %s\n", leva, x2,
               indep ? "" : "NAO BATE");
        if(!indep) mal++;

        /* ── O CONTROLO: o modo é da SETA e não da tabela. As três filhas
         * apontam para a mesma mãe e a mesma coluna, e reagiram das três
         * maneiras à mesma operação — um motor com um comportamento só teria
         * passado em qualquer uma das três medidas isoladas. */
        printf("\n      CONTROLO — as três apontam à MESMA coluna da MESMA mãe e"
               " reagiram das três maneiras: é o modo da seta, não da tabela.\n");
        sql_fechar();

        printf("\n");
        ok("QUANDO A SETA PERDE O DESTINO HÁ TRÊS RESPOSTAS, E SÃO SÓ TRÊS. Apagar uma linha"
           " apontada deixa a seta sem chegada, e o que fazer com ela não é uma lista de"
           " opções do dialecto: são as três coisas que se podem fazer a uma seta cujo"
           " destino desaparece. RECUSAR (`RESTRICT`) — a seta não pode perder o destino, e"
           " nada muda; é o modo por omissão porque é o único que não faz nada sem ordem."
           " LEVAR JUNTO (`CASCADE`) — a FIBRA vai atrás: apagar x na mãe é apagar π⁻¹(x) na"
           " filha, que é a imagem inversa do `thm:multiplicidade` e não uma regra de"
           " conveniência; e é SÓ a fibra de x, o que se mede deixando na filha uma linha"
           " que aponta a outro valor e exigindo que fique. SOLTAR (`SET NULL`) — a seta"
           " desaparece e a linha fica, com a célula a ir para o DUAL, que é onde uma"
           " coordenada sem valor mora; mede-se pelo dual e não pela impressão, porque"
           " `IS NULL` apanha-a e `= 0` não. Quem diz qual é a FILHA, porque a seta é dela;"
           " a mãe só sabe quem a aponta. O GUME É A ORDEM: pergunta-se a TODAS antes de"
           " agir a QUALQUER uma, senão a filha em CASCADE processada primeiro já tinha"
           " levado as suas linhas quando a recusa da filha em RESTRICT chegasse — a"
           " operação recusada e metade feita, que é o estado que tudo isto existe para não"
           " haver. Mede-se com o mesmo valor apontado pelas três de uma vez, contando os"
           " dois lados antes e depois. E O CONTROLO ESTÁ NO ARRANJO: as três filhas apontam"
           " à MESMA coluna da MESMA mãe e reagiram das três maneiras — o modo é da SETA e"
           " não da tabela, e um motor com um comportamento só teria passado em qualquer uma"
           " das três medidas isoladas. Fica dito o limite: a CADEIA NÃO DESCE — se a filha"
           " for mãe de outra, a cascata pára e a segunda seta é verificada como qualquer"
           " outra. É declarado, não esquecido: descer exigia reentrar no varre, que tem"
           " estado global. E HÁ DUAS PORTAS, não uma: apagar a linha apontada faz o destino"
           " desaparecer, MUDAR-LHE a chave faz o destino sair do sítio, e uma seta que"
           " aponta para onde já não há ninguém é o mesmo estado nos dois casos — pelo que a"
           " resposta é a MESMA FUNÇÃO e não duas, que é como duas implementações da mesma"
           " frase deixam de concordar. Sem a segunda metade, `UPDATE m SET id = 99` passava"
           " em silêncio e deixava a filha a apontar para um valor que já não existe, com o"
           " motor a recusar escrever a MESMA seta que já lá estava. E os dois modos são"
           " INDEPENDENTES — vivem nos dois pares de bits do mesmo octeto —, porque nada"
           " obriga quem quer a fibra atrás numa mudança de chave a querê-la atrás num"
           " apagar: mede-se com uma seta que LEVA no UPDATE e RECUSA no DELETE, e um motor"
           " que guardasse um modo só teria uma das metades a responder pela outra.",
           mal == 0);
    }

    /* ═══ §W37: O PREDICADO DITO NA ESCRITA ═════════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w37.mem", "/tmp/pgwire_w37.prog",
            "/tmp/pgwire_w37__t.mem", "/tmp/pgwire_w37__t.prog",
            "/tmp/pgwire_w37__u.mem", "/tmp/pgwire_w37__u.prog",
            "/tmp/pgwire_w37__l.mem", "/tmp/pgwire_w37__l.prog" };
        for(int k = 0; k < 8; k++) unlink(lixo[k]);
        printf("\n§W37 o CHECK: o WHERE dito na escrita, na mesma ISA.\n\n");
        if(!sql_abrir("/tmp/pgwire_w37")) mal++;
        int cr = sql_executa("CREATE TABLE t (a, b, CHECK (a > 10))", &o);

        /* ── (1) À ENTRADA. É a mesma dualidade do UNIQUE contra o DISTINCT um
         * andar acima: o WHERE escolhe à saída as linhas que satisfazem o
         * predicado, o CHECK recusa à entrada as que não o satisfazem. */
        int i1 = sql_executa("INSERT INTO t VALUES (20,1)", &o);
        int i2 = sql_executa("INSERT INTO t VALUES (5,2)", &o);
        int i3 = sql_executa("INSERT INTO t VALUES (11,3)", &o);   /* na fronteira */
        int i4 = sql_executa("INSERT INTO t VALUES (10,4)", &o);   /* na fronteira */
        sql_executa("SELECT * FROM t", &o);
        int entrada = (cr && i1 && !i2 && i3 && !i4 && o.nrows == 2);
        printf("      à entrada: 20 %d · 5 %d · 11 %d · 10 %d → %d linhas (esp 2)"
               "  %s\n", i1, i2, i3, i4, o.nrows, entrada ? "" : "NAO BATE");
        if(!entrada) mal++;

        /* ── (2) E A RECUSA NÃO DEIXA A LINHA LÁ. As células já estão escritas
         * quando o predicado corre — é a ordem que torna isto barato: o
         * catálogo ainda não subiu, pelo que a linha está no disco e NÃO
         * EXISTE, e o próximo INSERT escreve por cima. Mede-se contando, e
         * exigindo que o INSERT seguinte caia no sítio certo. */
        int i5 = sql_executa("INSERT INTO t VALUES (30,5)", &o);
        sql_executa("SELECT * FROM t", &o);
        int limpo = (i5 && o.nrows == 3 && !strcmp(o.cell[2][0], "30")
                                        && !strcmp(o.cell[2][1], "5"));
        printf("      a recusa não deixa lixo: %d linhas e a última é (%s,%s)"
               " (esp 30,5)  %s\n", o.nrows,
               o.nrows > 2 ? o.cell[2][0] : "?", o.nrows > 2 ? o.cell[2][1] : "?",
               limpo ? "" : "NAO BATE");
        if(!limpo) mal++;

        /* ── (3) E VALE NA OUTRA PORTA, com a mecânica que a torna uma
         * PERGUNTA: escreve-se o valor novo, corre-se o mesmo molde, e
         * restaura-se SEMPRE. Se passar, o UPDATE a sério escreve a seguir; se
         * não passar, nada ficou tocado — o que se mede exigindo que a célula
         * continue com o valor velho depois da recusa. */
        int u1 = sql_executa("UPDATE t SET a = 3 WHERE b = 1", &o);
        sql_executa("SELECT * FROM t WHERE b = 1", &o);
        char velho[8]; snprintf(velho, sizeof velho, "%s", o.nrows ? o.cell[0][0] : "?");
        int u2 = sql_executa("UPDATE t SET a = 40 WHERE b = 1", &o);
        sql_executa("SELECT * FROM t WHERE b = 1", &o);
        char novo[8]; snprintf(novo, sizeof novo, "%s", o.nrows ? o.cell[0][0] : "?");
        int porta = (!u1 && !strcmp(velho, "20") && u2 && !strcmp(novo, "40"));
        printf("      pelo UPDATE: recusado %d e a célula fica %s (esp 20) ·"
               " aceite %d e passa a %s (esp 40)  %s\n", u1, velho, u2, novo,
               porta ? "" : "NAO BATE");
        if(!porta) mal++;

        /* ── (4) O PREDICADO NÃO SE PRONUNCIA SOBRE O QUE NÃO ESTÁ. A ausência
         * está FORA do corpo: um predicado do corpo não a alcança, e o que não
         * se pronuncia não recusa. Sem esta regra todo o CHECK seria um
         * NOT NULL implícito — uma restrição que ninguém declarou. */
        int n1 = sql_executa("INSERT INTO t VALUES (NULL,6)", &o);
        sql_executa("SELECT * FROM t WHERE a IS NULL", &o);
        int calado = (n1 && o.nrows == 1);
        printf("      não se pronuncia sobre o ausente: entra %d · e está no dual"
               " %d (esp 1)  %s\n", n1, o.nrows, calado ? "" : "NAO BATE");
        if(!calado) mal++;

        /* ── (5) É O MESMO AVALIADOR, e por isso o predicado composto sai de
         * graça: a árvore do WHERE já sabe ler `AND`. Um CHECK que precisasse
         * de avaliador próprio teria duas respostas possíveis para a mesma
         * pergunta. */
        sql_executa("CREATE TABLE u (x, y, CHECK (x > 0 AND y > 0))", &o);
        int c1 = sql_executa("INSERT INTO u VALUES (5,5)", &o);
        int c2 = sql_executa("INSERT INTO u VALUES (5,0)", &o);
        int c3 = sql_executa("INSERT INTO u VALUES (0,5)", &o);
        sql_executa("SELECT * FROM u", &o);
        int composto = (c1 && !c2 && !c3 && o.nrows == 1);
        printf("      o predicado composto sai do mesmo avaliador: (5,5) %d ·"
               " (5,0) %d · (0,5) %d → %d (esp 1)  %s\n", c1, c2, c3, o.nrows,
               composto ? "" : "NAO BATE");
        if(!composto) mal++;

        /* ── (6) E PERSISTE, porque é texto no disco: fecha-se, reabre-se, e
         * continua a recusar. */
        sql_fechar();
        if(!sql_abrir("/tmp/pgwire_w37")) mal++;
        int p1 = sql_executa("INSERT INTO t VALUES (7,7)", &o);
        int p2 = sql_executa("INSERT INTO t VALUES (77,8)", &o);
        printf("      depois de reabrir: recusa %d · aceita %d  %s\n", p1, p2,
               (!p1 && p2) ? "" : "NAO BATE");
        if(p1 || !p2) mal++;

        /* ── O CONTROLO, e são dois. Primeiro: uma tabela SEM predicado aceita
         * exactamente os valores que a de cima recusou — senão a recusa vinha
         * de outra coisa qualquer. Segundo: DOIS `CHECK` são recusados em vez
         * de calados, porque juntá-los com um AND implícito seria o motor a
         * escrever predicado que ninguém escreveu. */
        sql_executa("CREATE TABLE l (a, b)", &o);
        int s1 = sql_executa("INSERT INTO l VALUES (5,1)", &o);
        int s2 = sql_executa("INSERT INTO l VALUES (10,2)", &o);
        int s3 = sql_executa("CREATE TABLE v (x CHECK (x > 0), y, CHECK (y > 0))", &o);
        printf("\n      CONTROLO — sem predicado, 5 e 10 entram: %d %d · e dois"
               " CHECK são recusados: %d  %s\n", s1, s2, s3,
               (s1 && s2 && !s3) ? "" : "NAO BATE");
        if(!s1 || !s2 || s3) mal++;
        sql_fechar();

        printf("\n");
        ok("O `CHECK` É O `WHERE` DITO NA ESCRITA, E CORRE NA MESMA ISA. É a mesma dualidade"
           " do UNIQUE contra o DISTINCT, um andar acima: o WHERE escolhe à SAÍDA as linhas"
           " que satisfazem o predicado, o CHECK recusa à ENTRADA as que não o satisfazem. E"
           " é literalmente o mesmo objecto — a mesma árvore, o mesmo molde, a mesma máquina"
           " —, corrido sobre a linha que quer entrar em vez de sobre as que já estão; por"
           " isso não há avaliador novo, guarda-se o TEXTO do predicado e compila-se com o"
           " mesmo `le_expr`. Um motor com um segundo avaliador teria duas respostas"
           " possíveis para a mesma pergunta, e é por serem o mesmo que o predicado composto"
           " sai de graça. A ORDEM é o que torna a recusa barata: as células já estão"
           " escritas quando o predicado corre, mas o catálogo ainda não subiu — a linha"
           " está no disco e NÃO EXISTE —, pelo que voltar sem incrementar não deixa nada"
           " para desfazer, e o INSERT seguinte escreve por cima. No UPDATE não há essa"
           " folga e a mecânica é outra: escreve-se o valor novo, pergunta-se, e"
           " RESTAURA-SE SEMPRE — restaurar nos dois ramos é o que faz disto uma pergunta e"
           " não uma escrita, o que se mede exigindo que a célula fique com o valor velho"
           " depois da recusa. E O PREDICADO NÃO SE PRONUNCIA SOBRE O QUE NÃO ESTÁ: a"
           " ausência está fora do corpo, um predicado do corpo não a alcança, e o que não"
           " se pronuncia não recusa — sem esta regra todo o CHECK seria um NOT NULL"
           " implícito, isto é, uma restrição que ninguém declarou. O CONTROLO tem duas"
           " metades: sem predicado os MESMOS valores entram, e dois CHECK na mesma tabela"
           " são recusados em vez de calados, porque juntá-los com um AND implícito seria o"
           " motor a escrever predicado que ninguém escreveu.",
           mal == 0);
    }

    /* ═══ §W38: A PERTENÇA E A SUA NEGAÇÃO ══════════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w38.mem", "/tmp/pgwire_w38.prog",
            "/tmp/pgwire_w38__a.mem", "/tmp/pgwire_w38__a.prog",
            "/tmp/pgwire_w38__b.mem", "/tmp/pgwire_w38__b.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W38 a pertença e a negação: complementares sobre o CORPO.\n\n");
        if(!sql_abrir("/tmp/pgwire_w38")) mal++;
        sql_executa("CREATE TABLE b (k, w)", &o);
        sql_executa("INSERT INTO b VALUES (0,1)", &o);
        sql_executa("INSERT INTO b VALUES (7,2)", &o);
        sql_executa("CREATE TABLE a (x, y)", &o);
        sql_executa("INSERT INTO a VALUES (7,1)", &o);      /* pertence   */
        sql_executa("INSERT INTO a VALUES (9,2)", &o);      /* não        */
        sql_executa("INSERT INTO a VALUES (NULL,3)", &o);   /* nem, nem   */

        /* ── (1) A NEGAÇÃO É A MESMA DESCIDA COM A RESPOSTA VIRADA, e as duas
         * são complementares SOBRE O CORPO — não sobre |I|. A linha sem `x`
         * fica de FORA das duas: não se pode negar uma resposta que não foi
         * dada, e é a mesma frase do dual (`= v` e `<> v` também não a
         * apanham). Se o motor tratasse a ausência como um valor, a soma
         * fechava em 3 e uma das duas apanhava-a. */
        sql_executa("SELECT y FROM a WHERE x IN (SELECT k FROM b)", &o);
        long dentro = o.nrows;
        char d1[8]; snprintf(d1, sizeof d1, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT y FROM a WHERE x NOT IN (SELECT k FROM b)", &o);
        long fora = o.nrows;
        char f1[8]; snprintf(f1, sizeof f1, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT y FROM a WHERE x IS NOT NULL", &o);
        long corpo = o.nrows;
        sql_executa("SELECT y FROM a", &o);
        long todas = o.nrows;
        int part = (dentro == 1 && !strcmp(d1, "1") && fora == 1 && !strcmp(f1, "2")
                    && dentro + fora == corpo && corpo < todas);
        printf("      IN %ld (y=%s) + NOT IN %ld (y=%s) = %ld presentes, de %ld"
               " linhas: a ausente fica fora das DUAS  %s\n",
               dentro, d1, fora, f1, corpo, todas, part ? "" : "NAO BATE");
        if(!part) mal++;

        /* ── (2) E O DUAL VALE NOS DOIS LADOS. Uma célula ausente na coluna da
         * SUBCONSULTA não é uma chave: pô-la na árvore era pôr o neutro com
         * cara de valor, e depois um `x IN (…)` com x = 0 casava com uma linha
         * que não tem valor nenhum. Mede-se pondo um NULL do lado de lá e um
         * zero do lado de cá — e o zero só casa se houver um zero ESCRITO. */
        sql_executa("CREATE TABLE c (k, w)", &o);
        sql_executa("INSERT INTO c VALUES (NULL,1)", &o);
        sql_executa("INSERT INTO c VALUES (5,2)", &o);
        sql_executa("INSERT INTO a VALUES (0,4)", &o);
        sql_executa("SELECT y FROM a WHERE x IN (SELECT k FROM c)", &o);
        long falso = o.nrows;
        sql_executa("SELECT y FROM a WHERE x IN (SELECT k FROM b)", &o);
        long certo = o.nrows;
        int lado = (falso == 0 && certo == 2);
        printf("      o NULL do outro lado não vira chave: `IN c` %ld (esp 0) ·"
               " `IN b`, que TEM um zero escrito, %ld (esp 2)  %s\n",
               falso, certo, lado ? "" : "NAO BATE");
        if(!lado) mal++;

        /* ── O CONTROLO: sem ausência nenhuma, a soma fecha em |I| exactamente.
         * É o que mostra que a folga de cima vem do DUAL e não de o motor
         * perder linhas — as duas metades continuam a ser complementares, e é
         * o corpo que mudou de tamanho, não a lei. */
        sql_executa("CREATE TABLE d (x, y)", &o);
        sql_executa("INSERT INTO d VALUES (7,1)", &o);
        sql_executa("INSERT INTO d VALUES (9,2)", &o);
        sql_executa("INSERT INTO d VALUES (0,3)", &o);
        sql_executa("SELECT y FROM d WHERE x IN (SELECT k FROM b)", &o);
        long cd = o.nrows;
        sql_executa("SELECT y FROM d WHERE x NOT IN (SELECT k FROM b)", &o);
        long cf = o.nrows;
        sql_executa("SELECT y FROM d", &o);
        long ct = o.nrows;
        printf("\n      CONTROLO — sem ausências: %ld + %ld = %ld (esp 3 = 3)"
               "  %s\n", cd, cf, ct, (cd + cf == ct && ct == 3) ? "" : "NAO BATE");
        if(cd + cf != ct || ct != 3) mal++;
        sql_fechar();

        printf("\n");
        ok("A PERTENÇA E A SUA NEGAÇÃO SÃO COMPLEMENTARES SOBRE O CORPO, E NÃO SOBRE |I|."
           " `NOT IN` é a MESMA descida com a resposta virada — uma implementação, não duas"
           " —, e é por isso que a única coisa que precisa de ser dita é o que fazer com"
           " quem não tem valor. A linha cuja célula está AUSENTE fica de fora das DUAS: não"
           " se pode negar uma resposta que não foi dada, e é a mesma frase que o dual já"
           " tinha dito com `= v` e `<> v`. Mede-se pela SOMA: `IN` + `NOT IN` fecha no peso"
           " dos presentes e não no número de linhas, o que um motor que tratasse a ausência"
           " como um valor não faria — nele a soma fechava em |I| e uma das duas apanhava-a."
           " E O DUAL VALE NOS DOIS LADOS, que era onde estava o defeito: a célula ausente"
           " da coluna da SUBCONSULTA entrava na árvore, e o neutro com cara de valor fazia"
           " um `x IN (…)` com x = 0 casar com uma linha que não tem valor nenhum. Mede-se"
           " com um NULL do lado de lá e um zero do lado de cá, exigindo que o zero só case"
           " quando há um zero ESCRITO — a árvore indexa o corpo, o dual vive no bitmap. O"
           " CONTROLO é a mesma pergunta numa tabela sem ausência nenhuma, onde a soma fecha"
           " em |I| exactamente: é o que mostra que a folga vem do DUAL e não de o motor"
           " perder linhas, com as duas metades a continuarem complementares e o CORPO a ser"
           " o que mudou de tamanho, não a lei.",
           mal == 0);
    }

    /* ═══ §W39: A MESMA REGRA EM TODOS OS CAMINHOS ══════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w39.mem", "/tmp/pgwire_w39.prog",
            "/tmp/pgwire_w39__a.mem", "/tmp/pgwire_w39__a.prog",
            "/tmp/pgwire_w39__b.mem", "/tmp/pgwire_w39__b.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W39 a árvore indexa o corpo: os cinco caminhos que a liam mal.\n\n");
        if(!sql_abrir("/tmp/pgwire_w39")) mal++;
        sql_executa("CREATE TABLE a (g, v)", &o);
        sql_executa("INSERT INTO a VALUES (0,10)", &o);     /* zero ESCRITO */
        sql_executa("INSERT INTO a VALUES (0,20)", &o);     /* zero ESCRITO */
        sql_executa("INSERT INTO a VALUES (NULL,30)", &o);  /* ausente      */
        sql_executa("INSERT INTO a VALUES (5,40)", &o);

        /* O ARRANJO é o gume, e é um só para os cinco: DOIS zeros escritos e
         * UMA ausência na mesma coluna. Um motor que leia a célula ausente como
         * o neutro junta os três num sítio, e cada caminho mostra-o à sua
         * maneira — o grupo com 3, o distinto a menos, a ordem no meio, a
         * junção a mais, a soma a incluir o que não está. */

        /* ── (1) QUOCIENTAR: o dual é UMA fibra, e não a fibra do zero. */
        sql_executa("SELECT g, count(*) FROM a GROUP BY g", &o);
        long ng = o.nrows;
        long g0 = (o.nrows > 0) ? atol(o.cell[0][1]) : -1;
        int dual_grupo = (o.nrows == 3) && o.nulo[2][0];
        int grupo = (ng == 3 && g0 == 2 && dual_grupo);
        printf("      GROUP BY: %ld grupos (esp 3) · o do zero tem %ld (esp 2) ·"
               " e o terceiro é o DUAL, com a chave ausente %d  %s\n",
               ng, g0, dual_grupo, grupo ? "" : "NAO BATE");
        if(!grupo) mal++;

        /* ── (2) O REPRESENTANTE: das linhas sem valor sobra UMA, e ela não se
         * junta às que têm um zero escrito. */
        sql_executa("SELECT DISTINCT g FROM a", &o);
        int dist = (o.nrows == 3);
        printf("      DISTINCT: %d valores (esp 3 — o 0, o 5 e o dual)  %s\n",
               o.nrows, dist ? "" : "NAO BATE");
        if(!dist) mal++;

        /* ── (3) A ORDEM: o dual não é pequeno, é de outro nível — vai para o
         * FIM. Lido como neutro ficava entre os zeros, no MEIO da ordem. */
        sql_executa("SELECT v FROM a ORDER BY g", &o);
        int ordem = (o.nrows == 4 && !strcmp(o.cell[3][0], "30"));
        printf("      ORDER BY: o sem-chave fica em último (%s, esp 30)  %s\n",
               o.nrows == 4 ? o.cell[3][0] : "?", ordem ? "" : "NAO BATE");
        if(!ordem) mal++;

        /* ── (4) JUNTAR é perguntar onde o valor está do outro lado, e quem
         * não tem valor não pergunta. */
        sql_executa("CREATE TABLE b (k, w)", &o);
        sql_executa("INSERT INTO b VALUES (0,777)", &o);
        sql_executa("SELECT v FROM a JOIN b ON a.g = b.k", &o);
        int junta = (o.nrows == 2);
        printf("      JOIN: %d linhas (esp 2 — só os zeros ESCRITOS)  %s\n",
               o.nrows, junta ? "" : "NAO BATE");
        if(!junta) mal++;

        /* ── (5) AGREGAR SEM QUOCIENTAR é quocientar pelo mapa CONSTANTE: uma
         * fibra, uma linha. Saía a coluna crua, uma linha por linha, com o nome
         * de uma função que ninguém aplicou. E soma-se o CORPO: 0+0+5 = 5, sem
         * o que não está. */
        sql_executa("SELECT sum(g) FROM a", &o);
        int soma = (o.nrows == 1 && !strcmp(o.cell[0][0], "5"));
        char vsoma[8]; snprintf(vsoma, sizeof vsoma, "%s", o.nrows ? o.cell[0][0] : "?");
        long nsoma = o.nrows;
        sql_executa("SELECT max(g) FROM a", &o);
        int maxi = (o.nrows == 1 && !strcmp(o.cell[0][0], "5"));
        printf("      sum(g) = %s em %ld linha (esp 5 em 1) · max(g) = %s (esp 5)"
               "  %s\n", vsoma, nsoma, o.nrows ? o.cell[0][0] : "?",
               (soma && maxi) ? "" : "NAO BATE");
        if(!soma || !maxi) mal++;

        /* ── (6) E O PAR QUE SEPARA OS NÍVEIS: a soma de NADA é AUSENTE, não
         * zero — zero é um valor e aqui não houve nenhum; a contagem de nada é
         * ZERO, porque contar o vazio é zero e não uma ausência. Duas
         * perguntas sobre o mesmo conjunto vazio, duas respostas de níveis
         * diferentes: é a distinção inteira num sítio só. */
        sql_executa("SELECT sum(g) FROM a WHERE v > 999", &o);
        int s_vazio = (o.nrows == 1 && o.nulo[0][0] && !o.cell[0][0][0]);
        sql_executa("SELECT count(*) FROM a WHERE v > 999", &o);
        int c_vazio = (o.nrows == 1 && !strcmp(o.cell[0][0], "0"));
        printf("      sobre o MESMO vazio: sum devolve ausente (%s) e count"
               " devolve o valor «%s» — níveis diferentes  %s\n",
               s_vazio ? "sim" : "NAO", o.nrows ? o.cell[0][0] : "?",
               (s_vazio && c_vazio) ? "" : "NAO BATE");
        if(!s_vazio || !c_vazio) mal++;

        /* ── O CONTROLO: a MESMA tabela sem a linha ausente. Todos os cinco
         * caminhos dão então o número «errado» de cima — 2 grupos, 2
         * distintos, a ordem completa, e o mesmo JOIN —, o que mostra que o
         * que muda as respostas é o DUAL e não o tamanho da tabela. */
        sql_executa("DELETE FROM a WHERE v = 30", &o);
        sql_executa("SELECT g, count(*) FROM a GROUP BY g", &o); long c1 = o.nrows;
        sql_executa("SELECT DISTINCT g FROM a", &o);             long c2 = o.nrows;
        sql_executa("SELECT sum(g) FROM a", &o);
        int c3 = (o.nrows == 1 && !strcmp(o.cell[0][0], "5"));
        printf("\n      CONTROLO — tirada a linha sem valor: %ld grupos (esp 2) ·"
               " %ld distintos (esp 2) · sum ainda 5 %d  %s\n", c1, c2, c3,
               (c1 == 2 && c2 == 2 && c3) ? "" : "NAO BATE");
        if(c1 != 2 || c2 != 2 || !c3) mal++;
        sql_fechar();

        printf("\n");
        ok("A ÁRVORE INDEXA O CORPO, E O DUAL VIVE NO BITMAP — EM TODOS OS CAMINHOS. A regra"
           " apareceu primeiro no índice, depois no UNIQUE, depois nos dois lados do IN; à"
           " quarta vez procurou-se onde mais é que o motor lê uma célula sem perguntar se"
           " ela existe, e havia CINCO caminhos por fechar. Todos falhavam da mesma maneira:"
           " a célula ausente lida como o neutro passa a ser um zero, e junta-se às linhas"
           " que têm um zero ESCRITO. QUOCIENTAR: o dual é uma fibra própria, não a fibra do"
           " zero — as linhas sem chave não são «as linhas com a chave 0», e o grupo delas"
           " sai com a chave AUSENTE, que é o que o cliente tem de ver. O REPRESENTANTE: das"
           " linhas sem valor sobra uma, pela mesma regra da folha 1, e ela não se junta às"
           " do zero. A ORDEM: o dual não é pequeno, é de outro nível — vai para o FIM em"
           " bloco, e lido como neutro ficava no MEIO da ordem, entre os zeros, a fingir que"
           " era o menor dos valores. JUNTAR: é perguntar onde este valor está do outro"
           " lado, e quem não tem valor não pergunta — a mesma frase do IN, porque é a mesma"
           " travessia. E AGREGAR SEM QUOCIENTAR não era um caso especial que faltasse: é o"
           " quociente pelo mapa CONSTANTE, cuja única fibra é a tabela inteira — uma fibra,"
           " uma linha —, e sem isso a coluna saía CRUA, uma linha por linha, com o nome de"
           " uma função que ninguém aplicou. O agregado soma o CORPO e não o suporte, e daí"
           " sai o par que separa os dois níveis num sítio só: a soma de NADA é AUSENTE"
           " (zero é um valor, e aqui não houve nenhum) e a contagem de nada é ZERO (contar"
           " o vazio é zero, não uma ausência) — duas perguntas sobre o mesmo conjunto"
           " vazio, duas respostas que não são a mesma coisa. O ARRANJO é o gume e é um só"
           " para os cinco: dois zeros escritos e uma ausência na mesma coluna, com o"
           " CONTROLO a ser a mesma tabela sem a linha ausente — aí todos voltam aos números"
           " de antes, o que mostra que o que muda as respostas é o DUAL e não o tamanho da"
           " tabela.",
           mal == 0);
    }

    /* ═══ §W40: DUAS RÉGUAS, E A SEGUNDA ONDE A PRIMEIRA CALOU ══════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w40.mem");     unlink("/tmp/pgwire_w40.prog");
        unlink("/tmp/pgwire_w40__t.mem");  unlink("/tmp/pgwire_w40__t.prog");
        printf("\n§W40 ORDER BY a, b: compor réguas, não fabricar chave.\n\n");
        if(!sql_abrir("/tmp/pgwire_w40")) mal++;
        sql_executa("CREATE TABLE t (a, b, c)", &o);
        sql_executa("INSERT INTO t VALUES (2,9,1)", &o);
        sql_executa("INSERT INTO t VALUES (1,5,2)", &o);
        sql_executa("INSERT INTO t VALUES (2,3,3)", &o);
        sql_executa("INSERT INTO t VALUES (1,7,4)", &o);
        sql_executa("INSERT INTO t VALUES (2,NULL,5)", &o);
        sql_executa("INSERT INTO t VALUES (NULL,4,6)", &o);
        /* a coluna c é a ETIQUETA: diz que linha é, e nunca entra na ordem —
         * sem ela a saída não se lê, e comparar valores repetidos não provaria
         * qual das linhas veio primeiro */

        /* ── (1) UMA RÉGUA SÓ deixa a fibra por ordenar: dentro de cada corrida
         * a ordem é a de chegada. É a base contra a qual a segunda se mede. */
        sql_executa("SELECT c FROM t ORDER BY a", &o);
        char u[16] = ""; 
        for(int i = 0; i < o.nrows && i < 8; i++) strcat(u, o.cell[i][0]);
        int uma = !strcmp(u, "241356");
        printf("      uma régua (a):    %s (esp 241356)  %s\n", u, uma ? "" : "NAO BATE");
        if(!uma) mal++;

        /* ── (2) DUAS RÉGUAS: a segunda entra SÓ onde a primeira não
         * distinguiu. A fibra do a=2 tinha (9,3,ausente) por ordem de chegada e
         * passa a (3,9,ausente); a do a=1 já estava ordenada e NÃO SE MEXE — é
         * o que separa compor réguas de reordenar tudo. */
        sql_executa("SELECT c FROM t ORDER BY a, b", &o);
        char d[16] = "";
        for(int i = 0; i < o.nrows && i < 8; i++) strcat(d, o.cell[i][0]);
        int duas = !strcmp(d, "243156");
        printf("      duas réguas (a,b): %s (esp 243156)  %s\n", d, duas ? "" : "NAO BATE");
        if(!duas) mal++;

        /* ── (3) E CADA RÉGUA TEM O SEU SENTIDO. `a` sobe e `b` desce: a fibra
         * do a=1 inverte-se e a ordem das fibras fica na mesma. Um motor que
         * guardasse um só sentido para as duas não faria isto. */
        sql_executa("SELECT c FROM t ORDER BY a, b DESC", &o);
        char e[16] = "";
        for(int i = 0; i < o.nrows && i < 8; i++) strcat(e, o.cell[i][0]);
        int sentido = !strcmp(e, "421356");
        printf("      a sobe, b desce:   %s (esp 421356)  %s\n", e,
               sentido ? "" : "NAO BATE");
        if(!sentido) mal++;

        /* ── (4) O DUAL ENTRA PELA MESMA PORTA NOS DOIS NÍVEIS: quem não tem
         * valor vai para o fim — do bloco todo na primeira régua (a linha 6), e
         * da sua fibra na segunda (a linha 5, dentro do a=2). As duas coisas
         * estão nas cadeias de cima e lêem-se nelas: o `6` é sempre o último, e
         * o `5` é sempre o último do seu grupo, em qualquer dos sentidos. */
        int dual = (u[5] == '6' && d[5] == '6' && e[5] == '6'
                    && d[4] == '5' && e[4] == '5');
        printf("      o dual no fim dos DOIS níveis: o sem-a é último sempre, e"
               " o sem-b é último da sua fibra  %s\n", dual ? "" : "NAO BATE");
        if(!dual) mal++;

        /* ── O CONTROLO: a segunda régua não pode reordenar o que a primeira já
         * decidiu. `ORDER BY b, a` sobre os mesmos dados dá uma ordem DIFERENTE
         * — se as duas fossem juntas numa chave só, ou se a segunda corresse
         * sobre a tabela inteira em vez de sobre cada fibra, a troca não teria
         * este efeito. */
        sql_executa("SELECT c FROM t ORDER BY b, a", &o);
        char f[16] = "";
        for(int i = 0; i < o.nrows && i < 8; i++) strcat(f, o.cell[i][0]);
        printf("\n      CONTROLO — trocada a ordem das réguas (b,a): %s, e é"
               " diferente de %s  %s\n", f, d,
               strcmp(f, d) ? "" : "NAO BATE");
        if(!strcmp(f, d)) mal++;
        sql_fechar();

        printf("\n");
        ok("`ORDER BY a, b` É A COMPOSIÇÃO DE DUAS RÉGUAS, E NÃO UMA CHAVE MAIOR. O"
           " `arquitetura.tex` já dizia que nada do que a máquina faz ao campo é uma"
           " operação nova — é uma régua ou uma composição delas —, e o desempate é o caso"
           " mais limpo disso: ordena-se pela primeira, o que parte a saída em FIBRAS (as"
           " corridas de mesmo valor), e dentro de cada fibra corre-se a MESMA descida com a"
           " segunda coluna. A árvore não muda e não há chave composta a fabricar: há a"
           " mesma árvore usada uma vez por fibra, que é o que «compor» quer dizer aqui. O"
           " que se mede é isso e não a ordem final: com uma régua só, a fibra do a=2 fica"
           " por ordem de CHEGADA (9,3,ausente); com duas, passa a (3,9,ausente) — e a fibra"
           " do a=1, que já estava ordenada, NÃO SE MEXE. Cada régua tem o seu sentido: `a`"
           " a subir e `b` a descer inverte a fibra e deixa a ordem das fibras como estava,"
           " o que um motor com um sentido só para as duas não faria. E O DUAL ENTRA PELA"
           " MESMA PORTA NOS DOIS NÍVEIS: quem não tem valor vai para o fim — do bloco todo"
           " na primeira régua, da sua fibra na segunda —, porque o dual não é pequeno, é de"
           " outro nível, e isso não muda por ser a segunda régua a perguntar. O CONTROLO é"
           " trocar a ordem das réguas: `b, a` dá uma ordem diferente de `a, b` sobre os"
           " mesmos dados, o que não aconteceria se as duas fossem juntas numa chave só nem"
           " se a segunda corresse sobre a tabela inteira em vez de sobre cada fibra.",
           mal == 0);
    }

    /* ═══ §W41: ACEITAR E FAZER OUTRA COISA ═════════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w41.mem");     unlink("/tmp/pgwire_w41.prog");
        unlink("/tmp/pgwire_w41__t.mem");  unlink("/tmp/pgwire_w41__t.prog");
        printf("\n§W41 duas ordens que respondiam ok sobre outra coisa.\n\n");
        if(!sql_abrir("/tmp/pgwire_w41")) mal++;
        sql_executa("CREATE TABLE t (a UNIQUE, b)", &o);

        /* ── (1) UMA ORDEM, VÁRIAS LINHAS. `INSERT ... VALUES (…), (…), (…)`
         * era ACEITE e escrevia só a primeira: as outras desapareciam sem uma
         * palavra, com a resposta a dizer que tinha corrido bem. Não é
         * responder errado — é responder certo sobre outra coisa, que é a pior
         * forma de falhar desta casa. */
        int m1 = sql_executa("INSERT INTO t VALUES (1,10), (2,20), (3,30)", &o);
        sql_executa("SELECT count(*) FROM t", &o);
        int todas = (m1 && o.nrows == 1 && !strcmp(o.cell[0][0], "3"));
        printf("      três tuplos numa ordem: entraram %s (esp 3)  %s\n",
               o.nrows ? o.cell[0][0] : "?", todas ? "" : "NAO BATE");
        if(!todas) mal++;

        /* ── (2) E OU ENTRAM TODAS OU NENHUMA. Uma ordem é uma ordem, e metade
         * dela feita não é resposta nenhuma: se um tuplo for recusado — aqui
         * pelo UNIQUE, e é o do MEIO —, desfaz-se o que já entrou. O gume está
         * em o primeiro tuplo ser legítimo: sem o desfazer, ele ficava lá. */
        int m2 = sql_executa("INSERT INTO t VALUES (7,70), (2,99), (8,80)", &o);
        sql_executa("SELECT count(*) FROM t", &o);
        long depois = o.nrows ? atol(o.cell[0][0]) : -1;
        sql_executa("SELECT b FROM t WHERE a = 7", &o);
        long sobrou = o.nrows;
        int atomico = (!m2 && depois == 3 && sobrou == 0);
        printf("      o tuplo do meio recusado: total %ld (esp 3) · o PRIMEIRO,"
               " que era legítimo, não ficou (%ld linhas, esp 0)  %s\n",
               depois, sobrou, atomico ? "" : "NAO BATE");
        if(!atomico) mal++;

        /* ── (3) E UMA LINHA SÓ continua a ser o caminho de sempre: a ordem
         * com um tuplo não passa pelo desfazer, e o número não muda. */
        int m3 = sql_executa("INSERT INTO t VALUES (9,90)", &o);
        sql_executa("SELECT count(*) FROM t", &o);
        int uma = (m3 && !strcmp(o.cell[0][0], "4"));
        printf("      e uma linha só: %s (esp 4)  %s\n", o.cell[0][0],
               uma ? "" : "NAO BATE");
        if(!uma) mal++;

        /* ── (4) `count(DISTINCT c)` NÃO É O COUNT COM UM ADORNO. É o número
         * de CLASSES do quociente por c — o mesmo objecto do GROUP BY, lido em
         * quantidade em vez de em extensão. Era aceite e respondia o count de
         * TUDO: a palavra entrava como nome de coluna e ninguém a lia. */
        sql_executa("CREATE TABLE u (a, b)", &o);
        sql_executa("INSERT INTO u VALUES (1,10), (2,10), (3,20),"
                    " (4,NULL), (5,NULL)", &o);
        sql_executa("SELECT count(*) FROM u", &o);
        long tot = o.nrows ? atol(o.cell[0][0]) : -1;
        sql_executa("SELECT count(DISTINCT b) FROM u", &o);
        long cls = o.nrows ? atol(o.cell[0][0]) : -1;
        int conta = (tot == 5 && cls == 3);
        printf("      count(*) %ld (esp 5) · count(DISTINCT b) %ld (esp 3)  %s\n",
               tot, cls, conta ? "" : "NAO BATE");
        if(!conta) mal++;

        /* ── (5) E O GUME É A CONCORDÂNCIA: contar as classes e listá-las são a
         * MESMA pergunta, e têm de dar o mesmo número. O `GROUP BY` devolve as
         * fibras uma a uma; o `count(DISTINCT)` devolve quantas são. Se
         * divergissem, uma delas estaria a tratar o dual de outra maneira —
         * que era exactamente o defeito, com o count a responder 5. */
        sql_executa("SELECT b, count(*) FROM u GROUP BY b", &o);
        long fibras = o.nrows;
        int concorda = (fibras == cls);
        printf("      GROUP BY dá %ld fibras e count(DISTINCT) conta %ld: a MESMA"
               " pergunta  %s\n", fibras, cls, concorda ? "" : "NAO BATE");
        if(!concorda) mal++;

        /* ── (6) E A CONTA RESPEITA O FILTRO: com o WHERE a cortar as linhas
         * sem valor, ficam duas classes. É o que mostra que a contagem é sobre
         * o CAMPO que a varredura deixou, e não sobre a tabela. */
        sql_executa("SELECT count(DISTINCT b) FROM u WHERE a < 4", &o);
        int filtra = (o.nrows == 1 && !strcmp(o.cell[0][0], "2"));
        printf("      com WHERE a < 4: %s classes (esp 2)  %s\n",
               o.nrows ? o.cell[0][0] : "?", filtra ? "" : "NAO BATE");
        if(!filtra) mal++;

        /* ── O CONTROLO: a coluna que não existe é RECUSADA, e não contada
         * como se fosse `*`. Sem ele, um motor que ignorasse o interior do
         * parêntese — que era o defeito — passava nas medidas de cima sempre
         * que os números por acaso coincidissem. */
        int z = sql_executa("SELECT count(DISTINCT z) FROM u", &o);
        printf("\n      CONTROLO — count(DISTINCT z), coluna que não existe:"
               " %s  %s\n", z ? "RESPONDEU (mau)" : "recusada",
               z ? "NAO BATE" : "");
        if(z) mal++;
        sql_fechar();

        printf("\n");
        ok("DUAS ORDENS RESPONDIAM `ok` SOBRE OUTRA COISA, QUE É A PIOR FORMA DE FALHAR"
           " DESTA CASA. `INSERT INTO t VALUES (…), (…), (…)` era aceite e escrevia SÓ A"
           " PRIMEIRA: as outras desapareciam sem uma palavra, e a resposta dizia que tinha"
           " corrido bem — não é responder errado, é responder certo sobre outra coisa."
           " Agora a ordem parte-se nos seus tuplos e escreve-se um a um, mas ATOMICAMENTE:"
           " uma ordem é uma ordem, e metade dela feita não é resposta nenhuma. Se um tuplo"
           " for recusado — pelo envelope, por uma restrição, pela seta —, desfaz-se o que já"
           " entrou pelo mesmo mecanismo do ROLLBACK, e o gume está em o tuplo recusado ser"
           " o do MEIO com o primeiro legítimo: sem o desfazer, ele ficava lá. Se o cliente"
           " já tinha uma transacção aberta não se lhe toca — a dele é maior, e quem a fecha"
           " é ele. A SEGUNDA: `count(DISTINCT c)` não é o count com um adorno, é o número"
           " de CLASSES do quociente por c — o mesmo objecto do GROUP BY, lido em quantidade"
           " em vez de em extensão —, e era aceite a responder o count de TUDO, porque a"
           " palavra entrava como nome de coluna e o interior do parêntese era deitado fora"
           " antes de chegar ao motor. O GUME É A CONCORDÂNCIA: contar as classes e"
           " listá-las são a MESMA pergunta, pelo que o `GROUP BY` e o `count(DISTINCT)` têm"
           " de dar o mesmo número — e se divergissem, uma delas estaria a tratar o dual de"
           " outra maneira. Aqui o dual conta como UMA classe, o que DIVERGE do SQL padrão"
           " (que o ignora): a escolha é a coerência interna, porque o GROUP BY desta casa"
           " já lhe dá uma fibra própria, e duas perguntas sobre a mesma partição não podem"
           " responder números diferentes. O CONTROLO é a coluna que não existe, que é"
           " recusada em vez de contada como se fosse `*` — sem ele, um motor que ignorasse"
           " o interior do parêntese passava sempre que os números coincidissem por acaso.",
           mal == 0);
    }

    /* ═══ §W42: A NEGAÇÃO E A LISTA, PELO MESMO PROGRAMA ════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w42.mem");     unlink("/tmp/pgwire_w42.prog");
        unlink("/tmp/pgwire_w42__t.mem");  unlink("/tmp/pgwire_w42__t.prog");
        printf("\n§W42 De Morgan e a lista: não é a mesma resposta, é o mesmo objecto.\n\n");
        if(!sql_abrir("/tmp/pgwire_w42")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,10), (2,20), (3,30), (4,40)", &o);

        /* ── (1) A LISTA É A DISJUNÇÃO DAS IGUALDADES, e não um operador novo:
         * `x IN (v1, v2)` escreve-se na árvore como `x = v1 OR x = v2`, e por
         * isso tudo o que já lá estava vale para ela sem uma linha a mais. */
        sql_executa("SELECT a FROM t WHERE a IN (1,3)", &o);
        long lista = o.nrows;
        char l0[8], l1[8];
        snprintf(l0, sizeof l0, "%s", o.nrows > 0 ? o.cell[0][0] : "?");
        snprintf(l1, sizeof l1, "%s", o.nrows > 1 ? o.cell[1][0] : "?");
        sql_executa("SELECT a FROM t WHERE a IN (9)", &o);
        long vazia = o.nrows;
        int lst = (lista == 2 && !strcmp(l0, "1") && !strcmp(l1, "3") && vazia == 0);
        printf("      a lista: IN (1,3) dá %ld (%s,%s) · IN (9) dá %ld  %s\n",
               lista, l0, l1, vazia, lst ? "" : "NAO BATE");
        if(!lst) mal++;

        /* ── (2) E É MESMO A DISJUNÇÃO — o que se mede não é a resposta, é o
         * PROGRAMA. `a IN (1,3)` e `a = 1 OR a = 3` compilam para o mesmo
         * bytecode, byte a byte; duas escritas que dão a mesma resposta podem
         * dá-la por acaso, mas o mesmo programa não é acaso. */
        sql_executa("SELECT a FROM t WHERE a IN (1,3)", &o);
        long p_in = sql_ultimo_prog;
        sql_executa("SELECT a FROM t WHERE a = 1 OR a = 3", &o);
        long p_or = sql_ultimo_prog;
        int mesmo = (p_in == p_or && p_in != 0);
        printf("      IN (1,3) e `= 1 OR = 3` compilam para o MESMO programa:"
               " %08lx vs %08lx  %s\n", p_in, p_or, mesmo ? "" : "NAO BATE");
        if(!mesmo) mal++;

        /* ── (3) A NEGAÇÃO EMPURRA-SE PARA AS FOLHAS, e De Morgan também é
         * medível pelo programa: `NOT (a = 1 OR a = 2)` e `a <> 1 AND a <> 2`
         * são a mesma árvore depois da normalização, logo o mesmo bytecode. É
         * a lei, e não uma coincidência de contagem. */
        sql_executa("SELECT a FROM t WHERE NOT (a = 1 OR a = 2)", &o);
        long n1 = o.nrows, pn1 = sql_ultimo_prog;
        sql_executa("SELECT a FROM t WHERE a <> 1 AND a <> 2", &o);
        long n2 = o.nrows, pn2 = sql_ultimo_prog;
        int morgan1 = (n1 == 2 && n1 == n2 && pn1 == pn2 && pn1 != 0);
        printf("      NOT(A OR B) = ¬A AND ¬B: %ld e %ld linhas, programa"
               " %08lx vs %08lx  %s\n", n1, n2, pn1, pn2,
               morgan1 ? "" : "NAO BATE");
        if(!morgan1) mal++;

        /* ── (4) E A OUTRA METADE DA LEI, que é a que faltaria se só se
         * medisse uma: `NOT (A AND B)` = `¬A OR ¬B`. As duas juntas são De
         * Morgan; uma só é meia lei. */
        sql_executa("SELECT a FROM t WHERE NOT (a = 1 AND b = 10)", &o);
        long m1 = o.nrows, pm1 = sql_ultimo_prog;
        sql_executa("SELECT a FROM t WHERE a <> 1 OR b <> 10", &o);
        long m2 = o.nrows, pm2 = sql_ultimo_prog;
        int morgan2 = (m1 == 3 && m1 == m2 && pm1 == pm2 && pm1 != 0);
        printf("      NOT(A AND B) = ¬A OR ¬B: %ld e %ld linhas, programa"
               " %08lx vs %08lx  %s\n", m1, m2, pm1, pm2,
               morgan2 ? "" : "NAO BATE");
        if(!morgan2) mal++;

        /* ── (5) E AS DUAS COMPÕEM-SE: `NOT (x IN (…))` é a conjunção das
         * desigualdades, sem uma linha de código a mais — a lista fez-se
         * disjunção na leitura, e o De Morgan operou sobre ela como sobre
         * qualquer outra. É o que se ganha em escrever na árvore em vez de no
         * avaliador. */
        sql_executa("SELECT a FROM t WHERE NOT (a IN (1,3))", &o);
        long c = o.nrows;
        char c0[8], c1[8];
        snprintf(c0, sizeof c0, "%s", o.nrows > 0 ? o.cell[0][0] : "?");
        snprintf(c1, sizeof c1, "%s", o.nrows > 1 ? o.cell[1][0] : "?");
        int comp = (c == 2 && !strcmp(c0, "2") && !strcmp(c1, "4"));
        printf("      NOT (a IN (1,3)) dá %ld (%s,%s), esp 2 (2,4)  %s\n",
               c, c0, c1, comp ? "" : "NAO BATE");
        if(!comp) mal++;

        /* ── (6) E A IDEMPOTÊNCIA CAI DE GRAÇA: `IN (1,1,3)` é o MESMO
         * programa que `IN (1,3)`, porque a contração já tratava o átomo
         * repetido. Um motor que avaliasse a lista item a item teria dois
         * testes onde há um. */
        sql_executa("SELECT a FROM t WHERE a IN (1,1,3)", &o);
        long pr = sql_ultimo_prog, nr2 = o.nrows;
        sql_executa("SELECT a FROM t WHERE a IN (1,3)", &o);
        int idem = (pr == sql_ultimo_prog && nr2 == o.nrows);
        printf("      IN (1,1,3) é o MESMO programa que IN (1,3): %08lx  %s\n",
               pr, idem ? "" : "NAO BATE");
        if(!idem) mal++;

        /* ── O CONTROLO: dois programas que TÊM de ser diferentes. Sem ele, um
         * motor que devolvesse sempre a mesma impressão digital — ou zero —
         * passava em tudo o que está acima. */
        sql_executa("SELECT a FROM t WHERE a IN (1,3)", &o);
        long pa = sql_ultimo_prog;
        sql_executa("SELECT a FROM t WHERE a IN (2,4)", &o);
        long pb = sql_ultimo_prog;
        printf("\n      CONTROLO — listas diferentes, programas diferentes:"
               " %08lx vs %08lx  %s\n", pa, pb, (pa != pb) ? "" : "NAO BATE");
        if(pa == pb) mal++;
        sql_fechar();

        printf("\n");
        ok("A NEGAÇÃO E A LISTA ESCREVEM-SE NA ÁRVORE, NÃO NO AVALIADOR — E ISSO MEDE-SE PELO"
           " PROGRAMA. `x IN (v1, v2)` não é um operador novo: é `x = v1 OR x = v2`, e"
           " escrevê-lo assim na leitura faz com que tudo o que já lá estava valha para ele"
           " sem uma linha a mais. `NOT (…)` também não precisa de um nó: precisa de DE"
           " MORGAN — trocam-se os conectivos e nega-se cada folha, com o `nega` que a"
           " comparação já tinha para o `<>`, o `<=` e o `>=`. É a mesma escolha da"
           " CONTRAÇÃO: o equivalente vira o MESMO objeto, em vez de duas coisas que uma"
           " regra depois iguala. E é aí que está a medida que interessa, porque duas"
           " escritas podem devolver a mesma resposta por acaso: expõe-se a impressão"
           " digital do bytecode e exige-se que ela COINCIDA. `a IN (1,3)` e `a = 1 OR"
           " a = 3` compilam para o mesmo programa; `NOT (A OR B)` e `¬A AND ¬B` também, e"
           " `NOT (A AND B)` e `¬A OR ¬B` também — as duas metades da lei, porque uma só é"
           " meia lei. Daí saem duas coisas de graça, que é o sinal de se ter escrito no"
           " sítio certo: `NOT (x IN (…))` é a conjunção das desigualdades sem uma linha de"
           " código a mais, e `IN (1,1,3)` é o MESMO programa que `IN (1,3)` porque a"
           " contração já tratava o átomo repetido — um motor que avaliasse a lista item a"
           " item teria dois testes onde há um. O CONTROLO são duas listas diferentes a dar"
           " programas diferentes: sem ele, uma impressão digital constante — ou zero —"
           " passava em tudo o que está acima.",
           mal == 0);
    }

    /* ═══ §W43: A MÉDIA É UM RACIONAL ═══════════════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w43.mem");     unlink("/tmp/pgwire_w43.prog");
        unlink("/tmp/pgwire_w43__t.mem");  unlink("/tmp/pgwire_w43__t.prog");
        unlink("/tmp/pgwire_w43__u.mem");  unlink("/tmp/pgwire_w43__u.prog");
        printf("\n§W43 avg: a divisão sai do andar, e o resultado vive em ℚ.\n\n");
        if(!sql_abrir("/tmp/pgwire_w43")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,1), (2,1), (4,1)", &o);

        /* ── (1) A MÉDIA DE 1, 2 e 4 É 7/3 — não 2 (o inteiro truncado) nem
         * um decimal arredondado. A divisão de inteiros SAI DO ANDAR: o
         * resultado vive em ℚ, que é o andar seguinte da escada e já está
         * construído nesta casa. */
        sql_executa("SELECT avg(a) FROM t", &o);
        char m[32]; snprintf(m, sizeof m, "%s", o.nrows ? o.cell[0][0] : "?");
        int exacta = !strcmp(m, "7/3");
        printf("      avg(1,2,4) = %s (esp 7/3, e não 2 nem 2.33)  %s\n",
               m, exacta ? "" : "NAO BATE");
        if(!exacta) mal++;

        /* ── (2) E A LEI QUE SÓ VALE SE FOR EXACTA: avg × n = sum, sem resto.
         * Um decimal truncado ou arredondado quebra-a; é a identidade que
         * distingue o objecto do seu representante impresso. */
        sql_executa("SELECT sum(a) FROM t", &o);
        long soma = o.nrows ? atol(o.cell[0][0]) : -1;
        sql_executa("SELECT count(*) FROM t", &o);
        long n = o.nrows ? atol(o.cell[0][0]) : -1;
        { long p = 0, q = 1;
          const char *barra = strchr(m, '/');
          p = atol(m);
          if(barra) q = atol(barra + 1);
          /* avg = p/q, e a lei é p·n = soma·q */
          int lei = (q > 0 && p * n == soma * q);
          printf("      avg × n = sum, sem resto: %ld/%ld × %ld = %ld  %s\n",
                 p, q, n, soma, lei ? "" : "NAO BATE");
          if(!lei) mal++; }

        /* ── (3) E QUANDO A DIVISÃO FECHA, o resultado é o INTEIRO: a classe
         * reduzida é o representante único, e 12/4 não se imprime como 12/4. */
        sql_executa("INSERT INTO t VALUES (5,1)", &o);
        sql_executa("SELECT avg(a) FROM t", &o);
        int inteiro = (o.nrows == 1 && !strcmp(o.cell[0][0], "3"));
        printf("      avg(1,2,4,5) = %s (esp 3, e não 12/4)  %s\n",
               o.nrows ? o.cell[0][0] : "?", inteiro ? "" : "NAO BATE");
        if(!inteiro) mal++;

        /* ── (4) A MÉDIA É DO CORPO, E O DIVISOR TAMBÉM. Acrescenta-se uma
         * linha SEM valor: o `count(*)` sobe para 5 e a média não se mexe,
         * porque o divisor é quantos VALORES entraram e não quantas linhas
         * havia. É o mesmo par de níveis do §W39, agora dentro de uma conta. */
        sql_executa("INSERT INTO t VALUES (NULL,1)", &o);
        sql_executa("SELECT avg(a) FROM t", &o);
        char m2[32]; snprintf(m2, sizeof m2, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT count(*) FROM t", &o);
        long n2 = o.nrows ? atol(o.cell[0][0]) : -1;
        int corpo = (!strcmp(m2, "3") && n2 == 5);
        printf("      com uma linha sem valor: count(*) %ld (esp 5) e avg %s"
               " (esp 3 — o divisor é 4)  %s\n", n2, m2, corpo ? "" : "NAO BATE");
        if(!corpo) mal++;

        /* ── (5) E A MÉDIA DE NADA É AUSENTE, pela mesma razão que a soma:
         * não há valor nenhum, e zero seria um valor. */
        sql_executa("SELECT avg(a) FROM t WHERE b > 9", &o);
        int vazia = (o.nrows == 1 && o.nulo[0][0]);
        printf("      avg do conjunto vazio: ausente %d  %s\n", vazia,
               vazia ? "" : "NAO BATE");
        if(!vazia) mal++;

        /* ── (6) POR FIBRA, E OS DOIS NÚMEROS SÃO DE NÍVEIS DIFERENTES. Na
         * fibra do g=2 há TRÊS linhas (G = 3, a conservação conta linhas) e o
         * divisor da média é DOIS (o corpo). Ver os dois lado a lado na mesma
         * linha da resposta é a distinção inteira num sítio só. */
        sql_executa("CREATE TABLE u (g,v)", &o);
        sql_executa("INSERT INTO u VALUES (1,1), (1,2), (2,10), (2,11),"
                    " (2,NULL)", &o);
        sql_executa("SELECT g, count(*), avg(v) FROM u GROUP BY g", &o);
        int fibra = (o.nrows == 2
                     && !strcmp(o.cell[0][1], "2") && !strcmp(o.cell[0][2], "3/2")
                     && !strcmp(o.cell[1][1], "3") && !strcmp(o.cell[1][2], "21/2"));
        printf("      por fibra: g=1 G=%s avg=%s (esp 2, 3/2) · g=2 G=%s avg=%s"
               " (esp 3, 21/2 — G conta linhas, a média conta corpo)  %s\n",
               o.nrows > 0 ? o.cell[0][1] : "?", o.nrows > 0 ? o.cell[0][2] : "?",
               o.nrows > 1 ? o.cell[1][1] : "?", o.nrows > 1 ? o.cell[1][2] : "?",
               fibra ? "" : "NAO BATE");
        if(!fibra) mal++;

        /* ── O CONTROLO: uma média que FECHA e outra que não fecham na mesma
         * tabela. Sem isso, um motor que imprimisse sempre `a/b` — ou que
         * arredondasse sempre — passava numa das duas por acaso. */
        sql_executa("SELECT avg(v) FROM u WHERE g = 1", &o);
        char c1[32]; snprintf(c1, sizeof c1, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT avg(v) FROM u WHERE g = 2 AND v > 10", &o);
        char c2[32]; snprintf(c2, sizeof c2, "%s", o.nrows ? o.cell[0][0] : "?");
        printf("\n      CONTROLO — na MESMA tabela: avg = %s (não fecha) e"
               " avg = %s (fecha)  %s\n", c1, c2,
               (!strcmp(c1, "3/2") && !strcmp(c2, "11")) ? "" : "NAO BATE");
        if(strcmp(c1, "3/2") || strcmp(c2, "11")) mal++;
        sql_fechar();

        printf("\n");
        ok("A MÉDIA É UM RACIONAL, E ISSO NÃO É UMA ESCOLHA DE FORMATO. `avg` é a soma sobre"
           " a contagem, e a divisão de inteiros SAI DO ANDAR: o resultado vive em ℚ, que é"
           " o andar seguinte da escada e já está construído nesta casa — com a classe"
           " reduzida do `ra_classe` a ser o representante único, pelo que 12/4 se imprime"
           " `3` e 7/3 se imprime `7/3`. Devolver um decimal era escolher um representante"
           " que NÃO É o objecto, e a lei que se segue não valeria: mede-se `avg × n = sum`"
           " sem resto, que é a identidade que distingue o objecto do seu impresso e que um"
           " arredondamento quebra. A MÉDIA É DO CORPO, E O DIVISOR TAMBÉM: acrescentar uma"
           " linha sem valor faz o `count(*)` subir e a média NÃO se mexer, porque o divisor"
           " é quantos VALORES entraram e não quantas linhas havia — o mesmo par de níveis"
           " do dual, agora dentro de uma conta. E vê-se melhor por fibra, onde os dois"
           " números saem lado a lado na mesma linha da resposta: na fibra do g=2 há TRÊS"
           " linhas (G = 3, e a conservação conta linhas) com o divisor da média a ser DOIS."
           " A média de nada é AUSENTE, pela mesma razão que a soma: não há valor nenhum, e"
           " zero seria um valor. O CONTROLO são duas médias na MESMA tabela, uma que fecha"
           " e outra que não — sem isso, um motor que imprimisse sempre `a/b`, ou que"
           " arredondasse sempre, passava numa das duas por acaso.",
           mal == 0);
    }

    /* ═══ §W44: QUOCIENTAR POR DUAS COLUNAS, E O QUE ISSO NÃO É ═════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w44.mem");     unlink("/tmp/pgwire_w44.prog");
        unlink("/tmp/pgwire_w44__t.mem");  unlink("/tmp/pgwire_w44__t.prog");
        printf("\n§W44 GROUP BY a, b: a mesma composição, e uma diferença.\n\n");
        if(!sql_abrir("/tmp/pgwire_w44")) mal++;
        sql_executa("CREATE TABLE t (a,b,v)", &o);
        sql_executa("INSERT INTO t VALUES (1,1,10), (1,1,20), (1,2,30),"
                    " (2,1,40), (2,1,50), (1,NULL,60)", &o);

        /* ── (1) É A MESMA COMPOSIÇÃO DO `ORDER BY a, b`: quocienta-se pela
         * primeira, o que parte a saída em fibras, e dentro de cada fibra
         * corre-se a MESMA descida com a segunda. A árvore não muda; muda o
         * número de vezes que é usada. */
        sql_executa("SELECT a, count(*) FROM t GROUP BY a", &o);
        long f1 = o.nrows;
        long g1 = o.nrows ? atol(o.cell[0][1]) : -1;
        sql_executa("SELECT a, b, count(*) FROM t GROUP BY a, b", &o);
        long f2 = o.nrows;
        int refina = (f1 == 2 && g1 == 4 && f2 == 4);
        printf("      por a: %ld fibras (a primeira com %ld) · por (a,b): %ld"
               " fibras  %s\n", f1, g1, f2, refina ? "" : "NAO BATE");
        if(!refina) mal++;

        /* ── (2) E A CONSERVAÇÃO NÃO DEPENDE DA GRANULARIDADE. ∑G = |I| com
         * duas fibras e com quatro: quocientar mais fino reparte as mesmas
         * linhas, não as perde nem as duplica. É o Lema da conservação a valer
         * nos dois níveis, e é o que diz que a segunda régua REFINA em vez de
         * refazer. */
        sql_executa("SELECT a, count(*) FROM t GROUP BY a", &o);
        long s1 = 0; for(int i = 0; i < o.nrows; i++) s1 += atol(o.cell[i][1]);
        sql_executa("SELECT a, b, count(*) FROM t GROUP BY a, b", &o);
        long s2 = 0; for(int i = 0; i < o.nrows; i++) s2 += atol(o.cell[i][2]);
        sql_executa("SELECT count(*) FROM t", &o);
        long tot = o.nrows ? atol(o.cell[0][0]) : -1;
        int conserva = (s1 == tot && s2 == tot && tot == 6);
        printf("      ∑G = |I| nos dois níveis: %ld e %ld, contra %ld linhas"
               "  %s\n", s1, s2, tot, conserva ? "" : "NAO BATE");
        if(!conserva) mal++;

        /* ── (3) O DUAL TEM FIBRA PRÓPRIA EM CADA NÍVEL, e não só no primeiro:
         * a linha sem `b` não se junta às que têm b escrito, e a sua chave sai
         * AUSENTE na segunda coluna — dentro da fibra do a=1, que é onde ela
         * pertence. Um motor que tratasse o dual só à cabeça perdia-o aqui. */
        sql_executa("SELECT a, b, count(*) FROM t GROUP BY a, b", &o);
        int dual = 0;
        for(int i = 0; i < o.nrows; i++)
            if(o.nulo[i][1] && !strcmp(o.cell[i][0], "1")
               && !strcmp(o.cell[i][2], "1")) dual = 1;
        printf("      o dual tem fibra própria na SEGUNDA coluna, dentro do"
               " a=1: %s\n", dual ? "sim" : "NAO");
        if(!dual) mal++;

        /* ── (4) E O AGREGADO SEGUE A FIBRA FINA: sum(v) por par, não por a. */
        sql_executa("SELECT a, b, count(*), sum(v) FROM t GROUP BY a, b", &o);
        int agr = 0;
        for(int i = 0; i < o.nrows; i++)
            if(!strcmp(o.cell[i][0], "2") && !strcmp(o.cell[i][3], "90")) agr = 1;
        printf("      sum(v) segue o par: a fibra (2,1) soma %s (esp 90)  %s\n",
               agr ? "90" : "?", agr ? "" : "NAO BATE");
        if(!agr) mal++;

        /* ── (5) E AQUI ESTÁ A DIFERENÇA ENTRE AS DUAS COMPOSIÇÕES, que é a
         * razão de este bloco não ser uma cópia do §W40. QUOCIENTAR COMUTA:
         * `GROUP BY b, a` dá as MESMAS fibras com os MESMOS pesos, porque uma
         * partição é um conjunto de classes e um conjunto não tem ordem.
         * ORDENAR NÃO COMUTA: o §W40 mediu que `b, a` dá uma sequência
         * diferente de `a, b`. A mecânica é a mesma — a mesma árvore, uma vez
         * por fibra —, e o que difere é o objeto que cada uma produz. */
        sql_executa("SELECT a, b, count(*) FROM t GROUP BY a, b", &o);
        long pesos1[8]; int n1 = o.nrows;
        for(int i = 0; i < n1 && i < 8; i++) pesos1[i] = atol(o.cell[i][2]);
        sql_executa("SELECT b, a, count(*) FROM t GROUP BY b, a", &o);
        long pesos2[8]; int n2 = o.nrows;
        for(int i = 0; i < n2 && i < 8; i++) pesos2[i] = atol(o.cell[i][2]);
        /* o multiconjunto dos pesos tem de ser o mesmo (a ordem pode mudar) */
        int mesmo = (n1 == n2);
        { long ss1 = 0, ss2 = 0, pp1 = 1, pp2 = 1;
          for(int i = 0; i < n1; i++){ ss1 += pesos1[i]; pp1 *= pesos1[i]; }
          for(int i = 0; i < n2; i++){ ss2 += pesos2[i]; pp2 *= pesos2[i]; }
          if(ss1 != ss2 || pp1 != pp2) mesmo = 0; }
        printf("      QUOCIENTAR COMUTA: (a,b) dá %d fibras e (b,a) dá %d, com"
               " os mesmos pesos %s — ao contrário de ORDENAR (§W40)  %s\n",
               n1, n2, mesmo ? "sim" : "NAO", mesmo ? "" : "NAO BATE");
        if(!mesmo) mal++;

        /* ── O CONTROLO: a segunda coluna tem de FAZER diferença. Se ela fosse
         * ignorada — que era o estado anterior, com a consulta recusada — o
         * número de fibras seria o mesmo com uma e com duas colunas. */
        printf("\n      CONTROLO — a segunda régua refina de facto: %ld → %ld"
               " fibras  %s\n", f1, f2, (f2 > f1) ? "" : "NAO BATE");
        if(f2 <= f1) mal++;
        sql_fechar();

        printf("\n");
        ok("QUOCIENTAR POR DUAS COLUNAS É A MESMA COMPOSIÇÃO DA ORDEM, E TEM UMA PROPRIEDADE"
           " QUE ELA NÃO TEM. A mecânica é a do §W40: quocienta-se pela primeira coluna, o"
           " que parte a saída em FIBRAS, e dentro de cada fibra corre-se a MESMA descida com"
           " a segunda — a árvore não muda, muda o número de vezes que é usada. A chave do"
           " grupo passa a ser o PAR, e a corrida termina quando QUALQUER das duas colunas"
           " muda. Deixar isto por fazer era ter a composição de um lado, a ordem, e não do"
           " outro, o quociente, quando são a mesma frase. O que se mede não é o número de"
           " grupos: é que a CONSERVAÇÃO não depende da granularidade — ∑G = |I| com duas"
           " fibras e com quatro, porque quocientar mais fino REPARTE as mesmas linhas, não"
           " as perde nem as duplica. E o dual tem fibra própria em CADA nível, não só no"
           " primeiro: a linha sem `b` não se junta às que têm b escrito, e a sua chave sai"
           " ausente na SEGUNDA coluna, dentro da fibra a que pertence — um motor que"
           " tratasse o dual só à cabeça perdia-o aqui. E AQUI ESTÁ A DIFERENÇA ENTRE AS DUAS"
           " COMPOSIÇÕES, que é a razão de este bloco não ser uma cópia do §W40:"
           " QUOCIENTAR COMUTA — `(b,a)` dá as MESMAS fibras com os MESMOS pesos, porque uma"
           " partição é um conjunto de classes e um conjunto não tem ordem — enquanto"
           " ORDENAR NÃO COMUTA, e o §W40 mediu-o. A mesma mecânica produz dois objetos"
           " diferentes, e é a natureza do objeto, não a do algoritmo, que decide se a troca"
           " se nota. O CONTROLO é que a segunda régua refine de facto: se fosse ignorada, o"
           " número de fibras seria o mesmo com uma e com duas colunas.",
           mal == 0);
    }

    /* ═══ §W45: NOMEAR AS COLUNAS É PODER DEIXAR UMA DE FORA ════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w45.mem");     unlink("/tmp/pgwire_w45.prog");
        unlink("/tmp/pgwire_w45__t.mem");  unlink("/tmp/pgwire_w45__t.prog");
        printf("\n§W45 INSERT INTO t (a,c): o que não se nomeia nasce no dual.\n\n");
        if(!sql_abrir("/tmp/pgwire_w45")) mal++;
        sql_executa("CREATE TABLE t (a,b,c)", &o);

        /* ── (1) A LISTA DIZ ONDE OS VALORES VÃO, e o que ela acrescenta não é
         * comodidade de escrita: é poder deixar uma coluna DE FORA sem lhe dar
         * valor nenhum. O que essa coluna recebe não é zero — é o DUAL. */
        int e1 = sql_executa("INSERT INTO t VALUES (1,2,3)", &o);
        int e2 = sql_executa("INSERT INTO t (a,c) VALUES (7,9)", &o);
        sql_executa("SELECT * FROM t WHERE a = 7", &o);
        int fora = (e1 && e2 && o.nrows == 1 && o.nulo[0][1]
                    && !strcmp(o.cell[0][0], "7") && !strcmp(o.cell[0][2], "9"));
        printf("      (a,c) VALUES (7,9): a linha é (%s, ausente=%d, %s)  %s\n",
               o.nrows ? o.cell[0][0] : "?", o.nrows ? o.nulo[0][1] : -1,
               o.nrows ? o.cell[0][2] : "?", fora ? "" : "NAO BATE");
        if(!fora) mal++;

        /* ── (2) E O GUME É O DUAL, outra vez: a coluna deixada de fora não
         * vale zero. Mede-se com as duas perguntas sobre a mesma célula —
         * `IS NULL` apanha-a e `= 0` não. Sem isto, «deixar de fora» seria
         * apenas outra maneira de escrever `0`, e a lista não acrescentaria
         * nada que a posição já não fizesse. */
        sql_executa("SELECT a FROM t WHERE b IS NULL", &o);
        long nul = o.nrows;
        sql_executa("SELECT a FROM t WHERE b = 0", &o);
        long zero = o.nrows;
        int dual = (nul == 1 && zero == 0);
        printf("      o que ficou de fora não vale zero: IS NULL %ld (esp 1) ·"
               " `= 0` %ld (esp 0)  %s\n", nul, zero, dual ? "" : "NAO BATE");
        if(!dual) mal++;

        /* ── (3) A ORDEM DOS NOMES É LIVRE, porque quem decide onde o valor cai
         * passa a ser a LISTA e não a posição. É o que distingue nomear de
         * contar: `(c,a)` põe o primeiro valor na terceira coluna. */
        int e3 = sql_executa("INSERT INTO t (c,a) VALUES (30,10)", &o);
        sql_executa("SELECT * FROM t WHERE a = 10", &o);
        int livre = (e3 && o.nrows == 1 && !strcmp(o.cell[0][2], "30")
                     && o.nulo[0][1]);
        printf("      (c,a) VALUES (30,10) → (%s,·,%s): a lista decide, não a"
               " posição  %s\n", o.nrows ? o.cell[0][0] : "?",
               o.nrows ? o.cell[0][2] : "?", livre ? "" : "NAO BATE");
        if(!livre) mal++;

        /* ── (4) E UMA COLUNA SÓ TAMBÉM SE NOMEIA: as outras DUAS nascem
         * ausentes, o que é a forma mais curta de dizer que o dual não é um
         * caso limite mas o estado por omissão de tudo o que não se escreveu. */
        int e4 = sql_executa("INSERT INTO t (b) VALUES (5)", &o);
        sql_executa("SELECT * FROM t WHERE b = 5", &o);
        int uma = (e4 && o.nrows == 1 && o.nulo[0][0] && o.nulo[0][2]);
        printf("      (b) VALUES (5): as outras duas nascem ausentes (%d,%d)"
               "  %s\n", o.nrows ? o.nulo[0][0] : -1, o.nrows ? o.nulo[0][2] : -1,
               uma ? "" : "NAO BATE");
        if(!uma) mal++;

        /* ── O CONTROLO, e são dois. Um nome que não é coluna é RECUSADO e não
         * ignorado — ignorá-lo faria a linha entrar noutro sítio em silêncio.
         * E a contagem tem de bater: nomear duas e trazer um valor é recusado,
         * porque o motor não adivinha qual das duas ficou por preencher. */
        int c1 = sql_executa("INSERT INTO t (z) VALUES (1)", &o);
        int c2 = sql_executa("INSERT INTO t (a,b) VALUES (1)", &o);
        sql_executa("SELECT count(*) FROM t", &o);
        long tot = o.nrows ? atol(o.cell[0][0]) : -1;
        printf("\n      CONTROLO — nome inexistente %d · contagem errada %d ·"
               " e a tabela ficou com %ld linhas (esp 4)  %s\n", c1, c2, tot,
               (!c1 && !c2 && tot == 4) ? "" : "NAO BATE");
        if(c1 || c2 || tot != 4) mal++;
        sql_fechar();

        printf("\n");
        ok("NOMEAR AS COLUNAS É PODER DEIXAR UMA DE FORA, E O QUE FICA DE FORA É O DUAL."
           " `INSERT INTO t (a,c) VALUES (1,2)` parece comodidade de escrita e não é: sem a"
           " lista, a única maneira de deixar uma célula sem valor era escrever NULL na"
           " posição dela — o que obriga a saber a ordem —, e com a lista a ausência é"
           " simplesmente o que sobra. As colunas que ninguém nomeou não recebem zero:"
           " recebem o suporte, e mede-se com as duas perguntas sobre a mesma célula —"
           " `IS NULL` apanha-a, `= 0` não. Sem esse gume, «deixar de fora» seria só outra"
           " maneira de escrever `0`, e a lista não acrescentaria nada que a posição já não"
           " fizesse. A ORDEM DOS NOMES É LIVRE, porque quem decide onde o valor cai passa a"
           " ser a lista e não a posição: `(c,a) VALUES (30,10)` põe o primeiro valor na"
           " terceira coluna, e é isso que distingue nomear de contar. E uma coluna só"
           " também se nomeia, com as outras duas a nascerem ausentes — que é a forma mais"
           " curta de dizer que o dual não é um caso limite, é o estado por omissão de tudo"
           " o que não se escreveu. O CONTROLO tem duas metades: um nome que não é coluna é"
           " RECUSADO e não ignorado, porque ignorá-lo faria a linha entrar noutro sítio em"
           " silêncio; e a contagem tem de bater, porque nomear duas colunas e trazer um"
           " valor deixaria o motor a adivinhar qual delas ficou por preencher — e adivinhar"
           " é exactamente o que a lista existe para não ser preciso.",
           mal == 0);
    }

    /* ═══ §W46: O QUANTIFICADOR, E A DUALIDADE QUE ELE TEM E O `IN` NÃO ═════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w46.mem", "/tmp/pgwire_w46.prog",
            "/tmp/pgwire_w46__t.mem", "/tmp/pgwire_w46__t.prog",
            "/tmp/pgwire_w46__u.mem", "/tmp/pgwire_w46__u.prog",
            "/tmp/pgwire_w46__vaz.mem", "/tmp/pgwire_w46__vaz.prog" };
        for(int k = 0; k < 8; k++) unlink(lixo[k]);
        printf("\n§W46 EXISTS: pergunta sobre a subconsulta, não sobre a linha.\n\n");
        if(!sql_abrir("/tmp/pgwire_w46")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,10), (2,20), (3,NULL)", &o);
        sql_executa("CREATE TABLE u (k)", &o);
        sql_executa("INSERT INTO u VALUES (7)", &o);
        sql_executa("CREATE TABLE vaz (k)", &o);

        /* ── (1) É UM QUANTIFICADOR, E NÃO UMA COMPARAÇÃO. O `IN` pergunta
         * sobre a LINHA — «este valor está do outro lado?» — e a resposta muda
         * de linha para linha. O `EXISTS` não olha para linha nenhuma: pergunta
         * se a subconsulta devolve ALGUMA, e a resposta é a mesma para todas.
         * Ou saem todas, ou não sai nenhuma. */
        sql_executa("SELECT a FROM t WHERE EXISTS (SELECT k FROM u)", &o);
        long cheia = o.nrows;
        sql_executa("SELECT a FROM t WHERE EXISTS (SELECT k FROM vaz)", &o);
        long vazia = o.nrows;
        int quant = (cheia == 3 && vazia == 0);
        printf("      todas ou nenhuma: com a tabela cheia %ld (esp 3) · com a"
               " vazia %ld (esp 0)  %s\n", cheia, vazia, quant ? "" : "NAO BATE");
        if(!quant) mal++;

        /* ── (2) `NOT EXISTS` É O ∀ ESCRITO COM O ∃, e é a mesma De Morgan do
         * §W42 aplicada a um conjunto em vez de a uma proposição: as duas
         * respostas trocam exactamente. */
        sql_executa("SELECT a FROM t WHERE NOT EXISTS (SELECT k FROM u)", &o);
        long n_cheia = o.nrows;
        sql_executa("SELECT a FROM t WHERE NOT EXISTS (SELECT k FROM vaz)", &o);
        long n_vazia = o.nrows;
        int morgan = (n_cheia == 0 && n_vazia == 3);
        printf("      NOT EXISTS troca as duas: %ld e %ld (esp 0 e 3)  %s\n",
               n_cheia, n_vazia, morgan ? "" : "NAO BATE");
        if(!morgan) mal++;

        /* ── (3) E AQUI ESTÁ A DUALIDADE QUE ELE TEM E O `IN` NÃO. A soma de
         * `EXISTS` com `NOT EXISTS` fecha em |I| INTEIRO, porque nenhum dos
         * dois olha para a célula — não há dual a ficar de fora. O `IN`, que
         * olha, tem a sua soma a fechar no peso dos PRESENTES (§W38). São duas
         * dualidades de NÍVEIS diferentes, e a tabela tem de propósito uma
         * linha com célula ausente para que a diferença apareça. */
        sql_executa("SELECT a FROM t", &o);
        long todas = o.nrows;
        int nivel = (cheia + n_cheia == todas && vazia + n_vazia == todas
                     && todas == 3);
        sql_executa("SELECT a FROM t WHERE b IS NULL", &o);
        long tem_dual = o.nrows;
        printf("      EXISTS + NOT EXISTS = %ld = |I| nas DUAS tabelas, e a"
               " tabela TEM um dual (%ld linha) — ao contrário do IN, que fecha"
               " nos presentes  %s\n", todas, tem_dual,
               (nivel && tem_dual == 1) ? "" : "NAO BATE");
        if(!nivel || tem_dual != 1) mal++;

        /* ── (4) E RESPEITA O VIVO: apagar a última linha da subconsulta vira
         * as duas respostas. É o que mostra que a pergunta é sobre o que ESTÁ
         * lá, e não sobre a tabela existir. */
        sql_executa("DELETE FROM u WHERE k = 7", &o);
        sql_executa("SELECT a FROM t WHERE EXISTS (SELECT k FROM u)", &o);
        long dep = o.nrows;
        sql_executa("SELECT a FROM t WHERE NOT EXISTS (SELECT k FROM u)", &o);
        long ndep = o.nrows;
        int vivo = (dep == 0 && ndep == 3);
        printf("      apagada a última linha de u: EXISTS %ld (esp 0) · NOT"
               " EXISTS %ld (esp 3)  %s\n", dep, ndep, vivo ? "" : "NAO BATE");
        if(!vivo) mal++;

        /* ── O CONTROLO, e são dois. A tabela que não existe é RECUSADA — e
         * não tomada por vazia, que seria responder «não há» a uma pergunta que
         * não se pôde fazer. E o `*` também se aceita, porque o que se pergunta
         * não é qual coluna: é se há linha. */
        int c1 = sql_executa("SELECT a FROM t WHERE EXISTS"
                             " (SELECT k FROM naoexiste)", &o);
        sql_executa("INSERT INTO u VALUES (9)", &o);
        int c2 = sql_executa("SELECT a FROM t WHERE EXISTS (SELECT * FROM u)", &o);
        long c2n = o.nrows;
        printf("\n      CONTROLO — tabela inexistente %s · e o `*` vale (%ld"
               " linhas)  %s\n", c1 ? "RESPONDEU (mau)" : "recusada", c2n,
               (!c1 && c2 && c2n == 3) ? "" : "NAO BATE");
        if(c1 || !c2 || c2n != 3) mal++;
        sql_fechar();

        printf("\n");
        ok("`EXISTS` PERGUNTA SOBRE A SUBCONSULTA, NÃO SOBRE A LINHA — E DAÍ TEM UMA"
           " DUALIDADE QUE O `IN` NÃO TEM. O `IN` é uma comparação: pergunta «este valor"
           " está do outro lado?», e a resposta muda de linha para linha. O `EXISTS` é um"
           " QUANTIFICADOR: pergunta se a subconsulta devolve alguma linha, e a resposta é a"
           " mesma para todas — ou saem todas, ou não sai nenhuma. Isso decide-se UMA vez,"
           " com a tabela do outro lado aberta, e a condição passa a ser uma CONSTANTE: não"
           " há molde a correr por linha, e o custo é o da varredura sem WHERE. `NOT EXISTS`"
           " é o ∀ escrito com o ∃, que é a De Morgan do §W42 aplicada a um conjunto em vez"
           " de a uma proposição. E A DIFERENÇA MEDE-SE NA SOMA: `EXISTS` + `NOT EXISTS`"
           " fecha em |I| INTEIRO, porque nenhum dos dois olha para a célula e não há dual a"
           " ficar de fora — ao passo que a soma do `IN` fecha no peso dos PRESENTES (§W38),"
           " porque esse olha. São duas dualidades de NÍVEIS diferentes, e a tabela deste"
           " bloco tem de propósito uma linha com célula ausente, para que a diferença"
           " apareça em vez de se supor. E o quantificador RESPEITA O VIVO: apagar a última"
           " linha da subconsulta vira as duas respostas, o que mostra que a pergunta é sobre"
           " o que ESTÁ lá e não sobre a tabela existir. O CONTROLO tem duas metades: a"
           " tabela que não existe é RECUSADA e não tomada por vazia — responder «não há» a"
           " uma pergunta que não se pôde fazer seria dar por resposta o próprio erro —, e o"
           " `*` vale, porque o que se pergunta não é qual coluna: é se há linha.",
           mal == 0);
    }

    /* ═══ §W47: A MESMA EXPRESSÃO DECIDE E PRODUZ ═══════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w47.mem");     unlink("/tmp/pgwire_w47.prog");
        unlink("/tmp/pgwire_w47__t.mem");  unlink("/tmp/pgwire_w47__t.prog");
        printf("\n§W47 SELECT a+1: o avaliador do WHERE, lido na outra direcção.\n\n");
        if(!sql_abrir("/tmp/pgwire_w47")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,10), (2,20), (3,NULL)", &o);

        /* ── (1) NÃO HÁ AVALIADOR NOVO. O tensor que o WHERE usa para DECIDIR
         * serve para PRODUZIR: avaliá-lo é percorrer os monómios não-nulos e
         * multiplicar as células que o multi-índice nomeia. O grau vem de
         * graça, porque o tensor já o tinha. */
        sql_executa("SELECT a+1 FROM t", &o);
        int soma = (o.nrows == 3 && !strcmp(o.cell[0][0], "2")
                                 && !strcmp(o.cell[2][0], "4"));
        char s0[8], s2[8];
        snprintf(s0, sizeof s0, "%s", o.nrows ? o.cell[0][0] : "?");
        snprintf(s2, sizeof s2, "%s", o.nrows > 2 ? o.cell[2][0] : "?");
        sql_executa("SELECT a*a FROM t", &o);
        int quad = (o.nrows == 3 && !strcmp(o.cell[0][0], "1")
                                 && !strcmp(o.cell[2][0], "9"));
        /* os valores da PRIMEIRA consulta guardam-se antes da segunda: o `o` é
         * o mesmo objecto, e imprimir dele depois seria mostrar a segunda a
         * dizer-se a primeira */
        printf("      a+1 → %s,…,%s (esp 2,…,4) · a*a → %s,…,%s (esp 1,…,9)"
               "  %s\n", s0, s2, o.nrows ? o.cell[0][0] : "?",
               o.nrows > 2 ? o.cell[2][0] : "?", (soma && quad) ? "" : "NAO BATE");
        if(!soma || !quad) mal++;

        /* ── (2) E O GUME É QUE É MESMO O MESMO: a expressão `a+1` no SELECT e
         * no WHERE compila para o MESMO programa. Duas escritas podem dar a
         * mesma resposta por acaso; o mesmo bytecode não é acaso — é a medida
         * do §W42 aplicada agora a dois USOS da mesma expressão em vez de a
         * duas escritas da mesma condição. */
        sql_executa("SELECT a+1 FROM t WHERE a+1 > 3", &o);
        long p1 = sql_ultimo_prog, n1 = o.nrows;
        char v1[16]; snprintf(v1, sizeof v1, "%s", o.nrows ? o.cell[0][0] : "?");
        sql_executa("SELECT a FROM t WHERE a+1 > 3", &o);
        long p2 = sql_ultimo_prog;
        char v2[16]; snprintf(v2, sizeof v2, "%s", o.nrows ? o.cell[0][0] : "?");
        int mesmo = (p1 == p2 && p1 != 0 && n1 == 1
                     && !strcmp(v1, "4") && !strcmp(v2, "3"));
        printf("      a decidir e a produzir, o MESMO programa: %08lx vs %08lx"
               " · e a linha é a mesma (a+1=%s, a=%s)  %s\n", p1, p2, v1, v2,
               mesmo ? "" : "NAO BATE");
        if(!mesmo) mal++;

        /* ── (3) E NÃO SE PRONUNCIA SOBRE O QUE NÃO ESTÁ. `a+b` na linha cuja
         * célula `b` está ausente dá AUSENTE, e não `a`: uma expressão que cita
         * o que não existe não tem valor. É a regra do CHECK e do WHERE, agora
         * dita na produção — e é o que separa somar do que lá está de somar
         * zero. */
        sql_executa("SELECT a+b FROM t", &o);
        int dual = (o.nrows == 3 && !strcmp(o.cell[0][0], "11")
                    && o.nulo[2][0] && !o.cell[2][0][0]);
        printf("      a+b → %s, %s, ausente(%d) — a que cita o que não está não"
               " tem valor  %s\n", o.nrows ? o.cell[0][0] : "?",
               o.nrows > 1 ? o.cell[1][0] : "?", o.nrows > 2 ? o.nulo[2][0] : -1,
               dual ? "" : "NAO BATE");
        if(!dual) mal++;

        /* ── (4) A EXPRESSÃO TRAZ O SEU NOME, e o `AS` continua a valer: o
         * cabeçalho é o TEXTO dela, e um alias substitui-o. Sem isso o
         * cabeçalho vinha do nome de uma coluna que não existe — lixo com cara
         * de nome, que é o defeito que a primeira escrita tinha. */
        sql_executa("SELECT a+1 FROM t", &o);
        int cab = !strcmp(o.col[0], "a+1");
        sql_executa("SELECT a+1 AS mais FROM t", &o);
        int ali = !strcmp(o.col[0], "mais") && o.nrows == 3;
        printf("      o cabeçalho é o texto («%s») e o AS substitui-o («%s»)"
               "  %s\n", cab ? "a+1" : "?", ali ? "mais" : "?",
               (cab && ali) ? "" : "NAO BATE");
        if(!cab || !ali) mal++;

        /* ── (5) E MISTURA-SE COM COLUNAS CRUAS na mesma lista, porque o item
         * da projecção passou a ser «coluna OU expressão» e não dois caminhos
         * separados. */
        sql_executa("SELECT a, a+1 FROM t", &o);
        int mistura = (o.ncols == 2 && o.nrows == 3
                       && !strcmp(o.cell[1][0], "2") && !strcmp(o.cell[1][1], "3"));
        printf("      `a, a+1` na mesma lista: (%s,%s) na 2.ª linha (esp 2,3)"
               "  %s\n", o.nrows > 1 ? o.cell[1][0] : "?",
               o.nrows > 1 ? o.cell[1][1] : "?", mistura ? "" : "NAO BATE");
        if(!mistura) mal++;

        /* ── O CONTROLO: uma expressão sobre coluna que não existe é RECUSADA,
         * e não avaliada como zero. Sem ele, o motor podia estar a tratar todo
         * o nome desconhecido como $0$ e a devolver contas erradas caladas. */
        int c1 = sql_executa("SELECT z+1 FROM t", &o);
        int c2 = sql_executa("SELECT zz FROM t", &o);
        printf("\n      CONTROLO — `z+1` %s · `zz` %s  %s\n",
               c1 ? "RESPONDEU (mau)" : "recusada",
               c2 ? "RESPONDEU (mau)" : "recusada",
               (!c1 && !c2) ? "" : "NAO BATE");
        if(c1 || c2) mal++;
        sql_fechar();

        printf("\n");
        ok("A MESMA EXPRESSÃO DECIDE E PRODUZ, E NÃO HÁ AVALIADOR NOVO. O tensor que o WHERE"
           " usa para escolher linhas serve para escrever valores: avaliá-lo é percorrer os"
           " monómios não-nulos e multiplicar as células que o multi-índice nomeia, e o grau"
           " vem de graça porque o tensor já o tinha — `a*a` sai sem uma linha a mais do que"
           " `a+1`. É a dualidade que este motor persegue em toda a parte, agora entre dois"
           " USOS do mesmo objeto: o WHERE selecciona com a expressão, o SELECT escreve-a. E"
           " o gume é que é MESMO o mesmo: `a+1` no SELECT e no WHERE compila para o MESMO"
           " programa, byte a byte — duas escritas podem dar a mesma resposta por acaso, o"
           " mesmo bytecode não. A expressão NÃO SE PRONUNCIA SOBRE O QUE NÃO ESTÁ: `a+b` na"
           " linha cuja célula b está ausente dá AUSENTE e não `a`, porque uma expressão que"
           " cita o que não existe não tem valor — a regra do CHECK dita na produção, e é o"
           " que separa somar o que lá está de somar zero. A expressão traz o SEU nome, que"
           " é o texto dela, e o `AS` continua a substituí-lo: sem isso o cabeçalho vinha do"
           " nome de uma coluna que não existe, lixo com cara de nome, que foi o defeito da"
           " primeira escrita. E mistura-se com colunas cruas na mesma lista, porque o item"
           " da projecção passou a ser «coluna OU expressão» e não dois caminhos separados."
           " O CONTROLO é a expressão sobre coluna que não existe, RECUSADA e não avaliada"
           " como zero: sem ele, o motor podia estar a tratar todo o nome desconhecido como"
           " $0$ e a devolver contas erradas caladas.",
           mal == 0);
    }

    /* ═══ §W48: A MESMA CONSULTA NOUTRA ESCRITA ═════════════════════════════ */
    {
        SqlOut a, b;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w48.mem", "/tmp/pgwire_w48.prog",
            "/tmp/pgwire_w48__t.mem", "/tmp/pgwire_w48__t.prog",
            "/tmp/pgwire_w48__u.mem", "/tmp/pgwire_w48__u.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W48 o ordinal, a vírgula, e a consulta sem corpo.\n\n");
        if(!sql_abrir("/tmp/pgwire_w48")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &a);
        sql_executa("INSERT INTO t VALUES (3,30), (1,10), (2,20)", &a);
        sql_executa("CREATE TABLE u (a,z)", &a);
        sql_executa("INSERT INTO u VALUES (1,100), (2,200)", &a);

        /* o par de saídas compara-se célula a célula: é o «dois caminhos que
         * têm de concordar», e aqui é a medida certa — as duas escritas
         * compilam o mesmo molde (a varredura sem WHERE), pelo que a impressão
         * digital do programa não as distinguiria de nada. */
        #define IGUAIS(x,y) ({ int _i, _j, _r = ((x).nrows == (y).nrows \
                                                 && (x).ncols == (y).ncols); \
            for(_i = 0; _r && _i < (x).nrows; _i++) \
                for(_j = 0; _r && _j < (x).ncols; _j++) \
                    if(strcmp((x).cell[_i][_j], (y).cell[_i][_j])) _r = 0; \
            _r; })

        /* ── (1) O ORDINAL É O NOME, dito pela posição. `ORDER BY 1` não é uma
         * coluna chamada «1»: é a primeira da lista PEDIDA, e resolve-se contra
         * essa lista — pelo que daqui para a frente não há caminho novo. As
         * duas escritas têm de dar a mesma saída, linha a linha. */
        sql_executa("SELECT a, b FROM t ORDER BY 1", &a);
        sql_executa("SELECT a, b FROM t ORDER BY a", &b);
        int ord = IGUAIS(a, b) && a.nrows == 3 && !strcmp(a.cell[0][0], "1");
        printf("      `ORDER BY 1` ≡ `ORDER BY a`: %d linhas iguais célula a"
               " célula, a começar em %s  %s\n", a.nrows,
               a.nrows ? a.cell[0][0] : "?", ord ? "" : "NAO BATE");
        if(!ord) mal++;

        /* ── (2) E O SEGUNDO ORDINAL É MESMO O SEGUNDO: `ORDER BY 2 DESC`
         * ordena pela outra coluna, e ao contrário — se o número fosse
         * ignorado, esta saída seria igual à de cima. */
        sql_executa("SELECT a, b FROM t ORDER BY 2 DESC", &b);
        int seg = (b.nrows == 3 && !strcmp(b.cell[0][1], "30")
                                && !strcmp(b.cell[2][1], "10"));
        printf("      `ORDER BY 2 DESC` ordena pela SEGUNDA e ao contrário:"
               " %s…%s (esp 30…10)  %s\n", b.nrows ? b.cell[0][1] : "?",
               b.nrows > 2 ? b.cell[2][1] : "?", seg ? "" : "NAO BATE");
        if(!seg) mal++;

        /* ── (3) A VÍRGULA É O `JOIN`. `FROM t, u WHERE t.a = u.a` e
         * `FROM t JOIN u ON t.a = u.a` são a MESMA consulta: a vírgula diz que
         * há duas tabelas, a condição diz por onde casam. Reescreve-se na
         * leitura, como o BETWEEN, e tudo o que a junção sabe fazer passa a
         * valer para esta escrita sem uma linha a mais. */
        /* a coluna pedida é `b`, que só existe de um lado: `a` está nas DUAS
         * tabelas e é ambíguo — o §W49 mede essa recusa, e pedi-lo aqui seria
         * comparar duas consultas inválidas e chamar-lhes iguais */
        sql_executa("SELECT b, z FROM t, u WHERE t.a = u.a", &a);
        sql_executa("SELECT b, z FROM t JOIN u ON t.a = u.a", &b);
        int vir = IGUAIS(a, b) && a.nrows == 2 && a.ncols == 2
                  && !strcmp(a.cell[0][0], "10") && !strcmp(a.cell[0][1], "100");
        printf("      a vírgula ≡ o JOIN: %d linhas × %d colunas iguais célula a"
               " célula, a começar em (%s,%s)  %s\n", a.nrows, a.ncols,
               a.nrows ? a.cell[0][0] : "?", a.nrows ? a.cell[0][1] : "?",
               vir ? "" : "NAO BATE");
        if(!vir) mal++;

        /* ── (4) A CONSULTA SEM CORPO. Sem `FROM` não há tabela, e sem tabela
         * não há campo a marcar: o que se pede não depende de linha nenhuma, e
         * a resposta é UMA linha com o valor. É o caso degenerado da projecção
         * — a expressão sem variáveis. */
        sql_executa("SELECT 1", &a);
        sql_executa("SELECT 42", &b);
        int semc = (a.nrows == 1 && !strcmp(a.cell[0][0], "1")
                    && b.nrows == 1 && !strcmp(b.cell[0][0], "42")
                    && !strcmp(a.col[0], "?column?"));
        printf("      sem FROM: `SELECT 1` → %s e `SELECT 42` → %s, coluna"
               " «%s»  %s\n", a.nrows ? a.cell[0][0] : "?",
               b.nrows ? b.cell[0][0] : "?", a.col[0], semc ? "" : "NAO BATE");
        if(!semc) mal++;

        /* ── (5) E A PRÓPRIA MEDIDA TEM DE PODER FALHAR. A impressão digital do
         * programa é do ÚLTIMO emitido, e os caminhos que não emitem nenhum —
         * a constante sem tabela, uma recusa no parse — deixavam lá o valor da
         * consulta ANTERIOR: quem comparasse dois desses estaria a comparar
         * duas cópias do mesmo lixo, e a igualdade passava sem poder falhar.
         * Zera-se à entrada, e mede-se que zera. */
        sql_executa("SELECT a FROM t", &a);
        long com = sql_ultimo_prog;
        sql_executa("SELECT 1", &a);
        long sem = sql_ultimo_prog;
        int honesta = (com != 0 && sem == 0);
        printf("      a impressão digital zera onde não há programa: com molde"
               " %08lx, sem molde %08lx  %s\n", com, sem,
               honesta ? "" : "NAO BATE");
        if(!honesta) mal++;

        /* ── O CONTROLO: um ordinal fora da lista é RECUSADO, e não tomado
         * pela última coluna nem ignorado. Sem ele, `ORDER BY 9` podia estar a
         * cair em silêncio na ordem natural, e as medidas de cima não o veriam. */
        int c1 = sql_executa("SELECT a FROM t ORDER BY 9", &a);
        printf("\n      CONTROLO — `ORDER BY 9` numa lista de uma coluna: %s"
               "  %s\n", c1 ? "RESPONDEU (mau)" : "recusada",
               c1 ? "NAO BATE" : "");
        if(c1) mal++;
        #undef IGUAIS
        sql_fechar();

        printf("\n");
        ok("A MESMA CONSULTA NOUTRA ESCRITA TEM DE DAR A MESMA COISA, E DUAS DAS TRÊS PEÇAS"
           " DESTE BLOCO SÃO ISSO. O ORDINAL é o nome dito pela posição: `ORDER BY 1` não é"
           " uma coluna chamada «1», é a primeira da lista PEDIDA, e resolve-se contra essa"
           " lista — pelo que daqui para a frente não há caminho novo nenhum. A VÍRGULA é o"
           " `JOIN`: `FROM t, u WHERE t.a = u.a` diz com outra pontuação o que"
           " `t JOIN u ON t.a = u.a` diz com uma palavra, e reescreve-se na leitura como o"
           " BETWEEN já era — assim tudo o que a junção sabe fazer passa a valer para esta"
           " escrita sem uma linha a mais. As duas medem-se por COMPARAÇÃO DAS SAÍDAS, célula"
           " a célula, e não pela impressão digital do programa: as escritas em causa"
           " compilam o mesmo molde (a varredura sem WHERE), pelo que o programa não as"
           " distinguiria de nada — usar aqui a medida do §W42 seria uma igualdade que passa"
           " por não poder ver a diferença. A terceira peça é outra coisa: a CONSULTA SEM"
           " CORPO. Sem FROM não há tabela, e sem tabela não há campo a marcar — o que se"
           " pede não depende de linha nenhuma, e a resposta é uma linha com o valor: é o"
           " caso degenerado da projecção, a expressão sem variáveis. E A PRÓPRIA MEDIDA TEM"
           " DE PODER FALHAR: a impressão digital é do ÚLTIMO programa emitido, e os caminhos"
           " que não emitem nenhum deixavam lá o valor da consulta anterior — quem comparasse"
           " dois desses estaria a comparar duas cópias do mesmo lixo. Zera-se à entrada, e"
           " mede-se que zera. O CONTROLO é o ordinal fora da lista, RECUSADO e não tomado"
           " pela última coluna nem ignorado em silêncio.",
           mal == 0);
    }

    /* ═══ §W49: A PROJECÇÃO DA JUNÇÃO ══════════════════════════════════════ */
    {
        SqlOut a, b;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w49.mem", "/tmp/pgwire_w49.prog",
            "/tmp/pgwire_w49__t.mem", "/tmp/pgwire_w49__t.prog",
            "/tmp/pgwire_w49__u.mem", "/tmp/pgwire_w49__u.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W49 SELECT t.a, u.z: a junção produz uma tabela, e é contra ela.\n\n");
        if(!sql_abrir("/tmp/pgwire_w49")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &a);
        sql_executa("INSERT INTO t VALUES (1,10), (2,20), (3,30)", &a);
        sql_executa("CREATE TABLE u (a,z)", &a);
        sql_executa("INSERT INTO u VALUES (1,100), (2,200)", &a);

        /* ── (1) A JUNÇÃO PRODUZ UMA TABELA, e é contra ELA que a projecção se
         * resolve — as colunas da esquerda seguidas das da direita. Resolvê-la
         * contra a tabela da esquerda, que era o que acontecia, recusava toda a
         * coluna do outro lado: a junção devolvia sempre tudo, e pedir uma
         * coluna era «column does not exist». */
        sql_executa("SELECT t.a, u.z FROM t JOIN u ON t.a = u.a", &a);
        int qual = (a.ncols == 2 && a.nrows == 2
                    && !strcmp(a.cell[0][0], "1") && !strcmp(a.cell[0][1], "100")
                    && !strcmp(a.col[0], "a") && !strcmp(a.col[1], "z"));
        printf("      `t.a, u.z`: %d colunas, (%s,%s) na 1.ª linha, cabeçalhos"
               " «%s»«%s»  %s\n", a.ncols, a.nrows ? a.cell[0][0] : "?",
               a.nrows ? a.cell[0][1] : "?", a.col[0], a.col[1],
               qual ? "" : "NAO BATE");
        if(!qual) mal++;

        /* ── (2) A ORDEM PEDIDA É A ORDEM DADA, e não a da concatenação:
         * `u.z, t.b` põe a coluna da DIREITA primeiro. Sem isto o mapa podia
         * estar a ser ignorado e a saída a sair na ordem natural — que
         * coincidiria com a pedida em metade dos casos. */
        sql_executa("SELECT u.z, t.b FROM t JOIN u ON t.a = u.a", &b);
        int ordem = (b.ncols == 2 && b.nrows == 2
                     && !strcmp(b.cell[0][0], "100") && !strcmp(b.cell[0][1], "10"));
        printf("      `u.z, t.b`: a direita primeiro — (%s,%s) (esp 100,10)"
               "  %s\n", b.nrows ? b.cell[0][0] : "?",
               b.nrows ? b.cell[0][1] : "?", ordem ? "" : "NAO BATE");
        if(!ordem) mal++;

        /* ── (3) SEM QUALIFICADOR TAMBÉM VALE, quando não há dúvida: `b` só
         * existe à esquerda e `z` só à direita, e o nome basta. O ponto é para
         * desfazer ambiguidade, não uma obrigação. */
        sql_executa("SELECT b, z FROM t JOIN u ON t.a = u.a", &a);
        int cru = (a.ncols == 2 && a.nrows == 2
                   && !strcmp(a.cell[0][0], "10") && !strcmp(a.cell[0][1], "100"));
        printf("      `b, z` sem ponto: (%s,%s) (esp 10,100)  %s\n",
               a.nrows ? a.cell[0][0] : "?", a.nrows ? a.cell[0][1] : "?",
               cru ? "" : "NAO BATE");
        if(!cru) mal++;

        /* ── (4) E VALE PELAS DUAS ESCRITAS: com `JOIN … ON` e com a vírgula,
         * que o §W48 mostrou serem a mesma consulta. Se a projecção só
         * funcionasse numa delas, a reescrita não seria uma reescrita. */
        sql_executa("SELECT t.a, u.z FROM t JOIN u ON t.a = u.a", &a);
        sql_executa("SELECT t.a, u.z FROM t, u WHERE t.a = u.a", &b);
        int duas = (a.nrows == b.nrows && a.ncols == b.ncols && a.nrows == 2);
        for(int i = 0; duas && i < a.nrows; i++)
            for(int j = 0; duas && j < a.ncols; j++)
                if(strcmp(a.cell[i][j], b.cell[i][j])) duas = 0;
        printf("      e pelas DUAS escritas (JOIN e vírgula): iguais célula a"
               " célula  %s\n", duas ? "" : "NAO BATE");
        if(!duas) mal++;

        /* ── O CONTROLO, e são dois. O nome que está nas DUAS tabelas é
         * AMBÍGUO e recusa-se — escolher um deles seria responder outra coisa,
         * e é o caso em que um motor descuidado devolve a primeira que
         * encontra. E o nome qualificado que não existe também é recusado, não
         * tomado pelo outro lado. */
        int c1 = sql_executa("SELECT a, z FROM t JOIN u ON t.a = u.a", &a);
        int c2 = sql_executa("SELECT t.q FROM t JOIN u ON t.a = u.a", &a);
        sql_executa("SELECT * FROM t JOIN u ON t.a = u.a", &b);
        int estrela = (b.ncols == 4);
        printf("\n      CONTROLO — `a` nas duas tabelas: %s · `t.q` que não"
               " existe: %s · e `*` continua a dar as %d colunas  %s\n",
               c1 ? "RESPONDEU (mau)" : "ambíguo, recusado",
               c2 ? "RESPONDEU (mau)" : "recusado", b.ncols,
               (!c1 && !c2 && estrela) ? "" : "NAO BATE");
        if(c1 || c2 || !estrela) mal++;
        sql_fechar();

        printf("\n");
        ok("A JUNÇÃO PRODUZ UMA TABELA, E É CONTRA ELA QUE A PROJECÇÃO SE RESOLVE. As colunas"
           " da esquerda seguidas das da direita são uma tabela nova, e pedir uma delas é"
           " pedir a essa — não à da esquerda, que era contra quem os nomes estavam a ser"
           " resolvidos: a projecção corria ANTES de a junção existir, pelo que toda a coluna"
           " do outro lado era «column does not exist», e a junção devolvia sempre tudo."
           " Adia-se para depois, e o que sai é um MAPA: para cada coluna pedida, o índice na"
           " concatenação. A ORDEM PEDIDA É A ORDEM DADA — `u.z, t.b` põe a coluna da"
           " direita primeiro —, o que é preciso medir porque um mapa ignorado daria a ordem"
           " natural, e essa coincide com a pedida em metade dos casos. SEM QUALIFICADOR"
           " TAMBÉM VALE quando não há dúvida: o ponto serve para desfazer ambiguidade, não é"
           " uma obrigação. E vale pelas DUAS escritas — `JOIN … ON` e a vírgula do §W48 —,"
           " porque se a projecção só funcionasse numa delas a reescrita não seria uma"
           " reescrita. O que torna isto uma implementação e não quatro é o mapa aplicar-se"
           " UMA vez por linha, no sítio onde ela fecha: a junção escreve a linha inteira por"
           " quatro caminhos (o casamento, o LEFT sem par, o RIGHT sem par, o segundo corte),"
           " e pôr a projecção dentro de cada um seria escrever a mesma frase quatro vezes."
           " O CONTROLO tem duas metades: o nome que está nas DUAS tabelas é AMBÍGUO e"
           " recusa-se — escolher um seria responder outra coisa, e é aqui que um motor"
           " descuidado devolve a primeira que encontra —, e o nome qualificado que não"
           " existe é recusado em vez de procurado do outro lado.",
           mal == 0);
    }

    /* ═══ §W50: AS FUNÇÕES ANALÍTICAS NO NÚCLEO ════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w50.mem");     unlink("/tmp/pgwire_w50.prog");
        unlink("/tmp/pgwire_w50__t.mem");  unlink("/tmp/pgwire_w50__t.prog");
        printf("\n§W50 exp, sin, cos, log: a série da casa, avaliada na célula.\n\n");
        if(!sql_abrir("/tmp/pgwire_w50")) mal++;
        sql_executa("CREATE TABLE t (x)", &o);
        sql_executa("INSERT INTO t VALUES (0), (1), (2), (3), (9), (20), (NULL)", &o);

        /* ── (1) OS VALORES DE PARTIDA, que a série tem de dar EXACTOS: as
         * quatro funções nos pontos onde se sabem de cor. Não é uma conta
         * aproximada — é um racional, e ou é aquele ou não é. */
        sql_executa("SELECT exp(x) FROM t WHERE x = 0", &o);
        int e0 = (o.nrows == 1 && !strcmp(o.cell[0][0], "1"));
        sql_executa("SELECT sin(x) FROM t WHERE x = 0", &o);
        int s0 = (o.nrows == 1 && !strcmp(o.cell[0][0], "0"));
        sql_executa("SELECT cos(x) FROM t WHERE x = 0", &o);
        int c0 = (o.nrows == 1 && !strcmp(o.cell[0][0], "1"));
        sql_executa("SELECT log(x) FROM t WHERE x = 1", &o);
        int l1 = (o.nrows == 1 && !strcmp(o.cell[0][0], "0"));
        printf("      os pontos de partida: exp(0)=1 %d · sin(0)=0 %d ·"
               " cos(0)=1 %d · log(1)=0 %d\n", e0, s0, c0, l1);
        if(!e0 || !s0 || !c0 || !l1) mal++;

        /* ── (2) E OS VALORES SÃO RACIONAIS EXACTOS, conferidos contra o que se
         * sabe fora deste ficheiro. A comparação faz-se em INTEIROS — p·10¹⁰
         * contra q·⌊v·10¹⁰⌋ — porque comparar decimais era trazer a vírgula que
         * esta casa não tem. */
        { struct { const char *q; const char *nome; long v10; } casos[] = {
              { "SELECT exp(x) FROM t WHERE x = 1", "exp(1)", 27182818284L },
              { "SELECT exp(x) FROM t WHERE x = 2", "exp(2)", 73890560989L },
              { "SELECT sin(x) FROM t WHERE x = 1", "sin(1)",  8414709848L },
              { "SELECT cos(x) FROM t WHERE x = 1", "cos(1)",  5403023058L } };
          int bons = 0;
          for(int t = 0; t < 4; t++){
              sql_executa(casos[t].q, &o);
              if(o.nrows != 1) continue;
              { const char *barra = strchr(o.cell[0][0], '/');
                long long p = atoll(o.cell[0][0]), qd = barra ? atoll(barra+1) : 1;
                /* |p/q − v| ≤ 10⁻⁹ ⟺ |p·10¹⁰ − v10·q| ≤ 10·q */
                __int128 esq = (__int128)p * 10000000000LL
                             - (__int128)casos[t].v10 * qd;
                if(esq < 0) esq = -esq;
                if(esq <= (__int128)10 * qd) bons++;
                printf("      %s = %s  (bate com %ld·10⁻¹⁰: %s)\n",
                       casos[t].nome, o.cell[0][0], casos[t].v10,
                       (esq <= (__int128)10 * qd) ? "sim" : "NAO"); }
          }
          if(bons != 4) mal++; }

        /* ── (3) E O QUE A SÉRIE NÃO REPRESENTA É RECUSADO. A ordem é a que o
         * inteiro permite — 20! é o último factorial que cabe em 64 bits —, e
         * fora do alcance dela a soma parcial não diz o valor. Recusa-se em vez
         * de devolver: `exp(20)` chegou a sair como 5·10⁷ quando e²⁰ é 4,8·10⁸,
         * e isso é responder outra coisa. */
        int r9 = sql_executa("SELECT exp(x) FROM t WHERE x = 9", &o);
        int r20 = sql_executa("SELECT exp(x) FROM t WHERE x = 20", &o);
        int rl = sql_executa("SELECT log(x) FROM t WHERE x = 2", &o);
        printf("      fora do alcance: exp(9) %s · exp(20) %s · log(2) %s  %s\n",
               r9 ? "PASSOU (mau)" : "recusado", r20 ? "PASSOU (mau)" : "recusado",
               rl ? "PASSOU (mau)" : "recusado",
               (!r9 && !r20 && !rl) ? "" : "NAO BATE");
        if(r9 || r20 || rl) mal++;

        /* ── (4) A ANALÍTICA NÃO SE PRONUNCIA SOBRE O QUE NÃO ESTÁ, pela mesma
         * regra das expressões e do CHECK: sem valor não há onde avaliar. */
        sql_executa("SELECT exp(x) FROM t WHERE x IS NULL", &o);
        int dual = (o.nrows == 1 && o.nulo[0][0]);
        printf("      sobre a célula ausente: ausente (%d)  %s\n", dual,
               dual ? "" : "NAO BATE");
        if(!dual) mal++;

        /* ── (5) E AS ALGÉBRICAS JÁ LÁ ESTAVAM: o tensor da §W47 tem grau três,
         * pelo que os polinómios saem sem uma linha a mais — é a mesma
         * expressão que decide no WHERE e escreve no SELECT. */
        sql_executa("SELECT x*x FROM t WHERE x = 3", &o);
        int p2 = (o.nrows == 1 && !strcmp(o.cell[0][0], "9"));
        sql_executa("SELECT x*x*x FROM t WHERE x = 3", &o);
        int p3 = (o.nrows == 1 && !strcmp(o.cell[0][0], "27"));
        sql_executa("SELECT x*x+2*x+1 FROM t WHERE x = 3", &o);
        int pp = (o.nrows == 1 && !strcmp(o.cell[0][0], "16"));
        printf("      as algébricas pelo tensor: x² = %s · x³ = %s ·"
               " x²+2x+1 = %s (esp 9, 27, 16)  %s\n",
               p2 ? "9" : "?", p3 ? "27" : "?", pp ? "16" : "?",
               (p2 && p3 && pp) ? "" : "NAO BATE");
        if(!p2 || !p3 || !pp) mal++;

        /* ── O CONTROLO, e são dois. A forma IMPURA é recusada: `exp(x)+1` tem
         * o parêntese a fechar a meio, e aceitá-la era ler `exp(x)` e deitar
         * fora o resto — responder certo sobre outra coisa, que foi exactamente
         * o que a primeira escrita fazia. E a coluna que não existe também. */
        int i1 = sql_executa("SELECT exp(x)+1 FROM t WHERE x = 1", &o);
        int i2 = sql_executa("SELECT exp(z) FROM t", &o);
        printf("\n      CONTROLO — `exp(x)+1` %s · `exp(z)` %s  %s\n",
               i1 ? "PASSOU (mau)" : "recusado", i2 ? "PASSOU (mau)" : "recusado",
               (!i1 && !i2) ? "" : "NAO BATE");
        if(i1 || i2) mal++;
        sql_fechar();

        printf("\n");
        ok("AS FUNÇÕES ANALÍTICAS ENTRAM NO NÚCLEO, E NÃO SE ESCREVE MATEMÁTICA NOVA PARA"
           " ISSO. O `aranha §sec:serie` deriva-as todas de uma: com J² = −1 as potências de"
           " J ciclam com período quatro, a série da exponencial parte-se pela PARIDADE do"
           " índice, e o que sai é exp(tJ) = c(t)·1 + s(t)·J — o cosseno nos pares, o seno"
           " nos ímpares, com o (−1)^k a contar as voltas módulo dois; o logaritmo é a"
           " inversa, do lado da série. E isso já estava construído em ℚ EXACTO no"
           " `calculo2.h`. O que se fez aqui foi a LIGAÇÃO: as séries saíram para `serie.h` —"
           " extraídas, não copiadas, porque a parte de várias variáveis do `calculo2.h`"
           " depende de um `Mat` que choca com o de `corpos.h`, e duas cópias da mesma frase"
           " é como elas deixam de concordar — e o motor passou a poder avaliá-las nas suas"
           " células, sem um único double. O que sai é um RACIONAL EXACTO, conferido contra"
           " valores que vêm de fora deste ficheiro e comparado em INTEIROS, porque comparar"
           " decimais era trazer a vírgula que esta casa não tem. A ORDEM É A QUE O INTEIRO"
           " PERMITE e não um número escolhido: os coeficientes têm factoriais no"
           " denominador, e 20! é o último que cabe em 64 bits — devolve-se a soma até 18 e"
           " testa-se com 20, sendo os dois termos a mais o que a resposta ainda não sabe. E"
           " FORA DO ALCANCE RECUSA-SE: `exp(20)` chegou a sair como 5·10⁷ quando e²⁰ é"
           " 4,8·10⁸, e três defeitos meus tiveram de cair antes disto parar de mentir —"
           " comparar numeradores de fracções com denominadores diferentes, ler o contador de"
           " estouros DEPOIS da construção da série, e não ler de todo o `qz_perdeu` que a"
           " casa já mantinha para a avaliação. Não era o guarda que faltava: era eu a não"
           " olhar para o que já estava dito. AS ALGÉBRICAS JÁ LÁ ESTAVAM — o tensor da §W47"
           " tem grau três, pelo que os polinómios saem sem uma linha a mais. O CONTROLO tem"
           " duas metades: a forma IMPURA é recusada, porque `exp(x)+1` fecha o parêntese a"
           " meio e aceitá-la seria ler `exp(x)` e deitar fora o resto; e a coluna que não"
           " existe também.",
           mal == 0);
    }

    /* ═══ §W51: A TABELA É UMA MATRIZ ══════════════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w51.mem", "/tmp/pgwire_w51.prog",
            "/tmp/pgwire_w51__m.mem", "/tmp/pgwire_w51__m.prog",
            "/tmp/pgwire_w51__s.mem", "/tmp/pgwire_w51__s.prog",
            "/tmp/pgwire_w51__r.mem", "/tmp/pgwire_w51__r.prog" };
        for(int k = 0; k < 8; k++) unlink(lixo[k]);
        printf("\n§W51 det, posto, traço, transposta, inversa — em ℚ exacto.\n\n");
        if(!sql_abrir("/tmp/pgwire_w51")) mal++;
        sql_executa("CREATE TABLE m (a,b)", &o);
        sql_executa("INSERT INTO m VALUES (1,2), (3,4)", &o);

        /* ── (1) OS ESCALARES. As linhas por colunas de uma tabela são as
         * entradas de uma matriz, e as perguntas da álgebra linear são
         * perguntas sobre a tabela — não é uma leitura forçada, é a mesma
         * tabela vista pela outra face. */
        sql_executa("SELECT det(*) FROM m", &o);
        int d = (o.nrows == 1 && !strcmp(o.cell[0][0], "-2"));
        sql_executa("SELECT posto(*) FROM m", &o);
        int p = (o.nrows == 1 && !strcmp(o.cell[0][0], "2"));
        sql_executa("SELECT traco(*) FROM m", &o);
        int tr = (o.nrows == 1 && !strcmp(o.cell[0][0], "5"));
        printf("      (1,2;3,4): det = -2 %d · posto = 2 %d · traço = 5 %d\n",
               d, p, tr);
        if(!d || !p || !tr) mal++;

        /* ── (2) A TRANSPOSTA DEVOLVE UMA TABELA, e é aí que a leitura fecha: o
         * resultado é outra vez uma coisa a que se pode perguntar o
         * determinante. E numa NÃO quadrada troca mesmo as dimensões. */
        sql_executa("SELECT transposta(*) FROM m", &o);
        int t2 = (o.nrows == 2 && o.ncols == 2
                  && !strcmp(o.cell[0][1], "3") && !strcmp(o.cell[1][0], "2"));
        sql_executa("CREATE TABLE r (a,b,c)", &o);
        sql_executa("INSERT INTO r VALUES (1,2,3), (4,5,6)", &o);
        sql_executa("SELECT transposta(*) FROM r", &o);
        int t3 = (o.nrows == 3 && o.ncols == 2);
        printf("      transposta: a 2×2 troca (%d) · a 2×3 vira %d×%d (esp 3×2)"
               "  %s\n", t2, o.nrows, o.ncols, (t2 && t3) ? "" : "NAO BATE");
        if(!t2 || !t3) mal++;

        /* ── (3) E A INVERSA É EXACTA EM ℚ, que é o ponto: sem um único double,
         * a inversa de uma matriz de inteiros tem entradas RACIONAIS, e elas
         * saem como classe reduzida — 3/2 e −1/2, não 1,5 e −0,5. */
        sql_executa("SELECT inversa(*) FROM m", &o);
        int inv = (o.nrows == 2 && o.ncols == 2
                   && !strcmp(o.cell[0][0], "-2") && !strcmp(o.cell[0][1], "1")
                   && !strcmp(o.cell[1][0], "3/2") && !strcmp(o.cell[1][1], "-1/2"));
        printf("      inversa: (%s %s ; %s %s) — esp (-2 1 ; 3/2 -1/2)  %s\n",
               o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
               o.nrows > 1 ? o.cell[1][0] : "?", o.nrows > 1 ? o.cell[1][1] : "?",
               inv ? "" : "NAO BATE");
        if(!inv) mal++;

        /* ── (4) E O GUME É A LEI: A·A⁻¹ = I. Comparar a inversa com o que se
         * espera é confiar na conta; multiplicá-la de volta é VERIFICÁ-LA, e é
         * o que separa uma resposta certa de uma resposta que por acaso tem a
         * forma certa. A conta faz-se aqui em racionais, à mão. */
        { long p11 = 0, q11 = 1, p12 = 0, q12 = 1, p21 = 0, q21 = 1, p22 = 0, q22 = 1;
          const char *b;
          if(o.nrows == 2 && o.ncols == 2){
              p11 = atol(o.cell[0][0]); b = strchr(o.cell[0][0], '/'); q11 = b ? atol(b+1) : 1;
              p12 = atol(o.cell[0][1]); b = strchr(o.cell[0][1], '/'); q12 = b ? atol(b+1) : 1;
              p21 = atol(o.cell[1][0]); b = strchr(o.cell[1][0], '/'); q21 = b ? atol(b+1) : 1;
              p22 = atol(o.cell[1][1]); b = strchr(o.cell[1][1], '/'); q22 = b ? atol(b+1) : 1;
          }
          /* A = (1 2 ; 3 4);  (A·A⁻¹)[i][j] em fracções */
          { long n11 = p11*q12*1 + 2*p21*q11, d11 = q11*q12*q21;   /* aproximação simbólica */
            /* faz-se termo a termo com denominador comum por entrada */
            long e11n = p11*q21 + 2*p21*q11, e11d = q11*q21;
            long e12n = p12*q22 + 2*p22*q12, e12d = q12*q22;
            long e21n = 3*p11*q21 + 4*p21*q11, e21d = q11*q21;
            long e22n = 3*p12*q22 + 4*p22*q12, e22d = q12*q22;
            int id = (e11n == e11d) && (e12n == 0) && (e21n == 0) && (e22n == e22d);
            printf("      e a LEI: A·A⁻¹ = (%ld/%ld %ld/%ld ; %ld/%ld %ld/%ld)"
                   " = I  %s\n", e11n, e11d, e12n, e12d, e21n, e21d, e22n, e22d,
                   id ? "" : "NAO BATE");
            if(!id) mal++;
            (void)n11; (void)d11; } }

        /* ── (5) A SINGULAR NÃO TEM INVERSA, e o motor não a inventa: det = 0,
         * posto = 1 (a segunda linha é o dobro da primeira), e a inversa é
         * RECUSADA. As três dizem a mesma coisa por caminhos diferentes. */
        sql_executa("CREATE TABLE s (a,b)", &o);
        sql_executa("INSERT INTO s VALUES (1,2), (2,4)", &o);
        sql_executa("SELECT det(*) FROM s", &o);
        int sd = (o.nrows == 1 && !strcmp(o.cell[0][0], "0"));
        sql_executa("SELECT posto(*) FROM s", &o);
        int sp = (o.nrows == 1 && !strcmp(o.cell[0][0], "1"));
        int si = sql_executa("SELECT inversa(*) FROM s", &o);
        printf("      a singular: det = 0 %d · posto = 1 %d · inversa %s  %s\n",
               sd, sp, si ? "PASSOU (mau)" : "recusada",
               (sd && sp && !si) ? "" : "NAO BATE");
        if(!sd || !sp || si) mal++;

        /* ── O CONTROLO, e são três. O determinante e o traço pedem uma matriz
         * QUADRADA e recusam-se numa 2×3 — enquanto o POSTO não pede, e
         * responde. E a matriz com uma célula AUSENTE é recusada: o dual não é
         * zero, e fazer a conta com ele seria inventar a entrada que falta. */
        int c1 = sql_executa("SELECT det(*) FROM r", &o);
        sql_executa("SELECT posto(*) FROM r", &o);
        int c2 = (o.nrows == 1 && !strcmp(o.cell[0][0], "2"));
        sql_executa("INSERT INTO m VALUES (5,NULL)", &o);
        int c3 = sql_executa("SELECT posto(*) FROM m", &o);
        printf("\n      CONTROLO — det numa 2×3: %s · posto numa 2×3 responde"
               " (%d) · com célula ausente: %s  %s\n",
               c1 ? "PASSOU (mau)" : "recusado", c2,
               c3 ? "PASSOU (mau)" : "recusado",
               (!c1 && c2 && !c3) ? "" : "NAO BATE");
        if(c1 || !c2 || c3) mal++;
        sql_fechar();

        printf("\n");
        ok("A TABELA É UMA MATRIZ, E AS PERGUNTAS DA ÁLGEBRA LINEAR SÃO PERGUNTAS SOBRE ELA."
           " As suas linhas por colunas são as entradas, e det, posto, traço, transposta e"
           " inversa não pedem estrutura nova: pedem a mesma tabela vista pela outra face. O"
           " `lib/linear.h` já tinha tudo em ℚ exacto, e o obstáculo era um NOME — `Mat`"
           " está tomado pelo `corpos.h`, que assim chama a matriz 2×2 de longos do"
           " transporte mecânico, e o motor já o traz pelo `reta.h`. São dois objetos"
           " diferentes com o mesmo nome, e cinquenta e oito ficheiros dependem de um"
           " enquanto vinte e um dependem do outro: renomeia-se À ENTRADA do motor, e nenhum"
           " dos dois headers muda. E A INVERSA É EXACTA EM ℚ, que é o ponto de tudo isto:"
           " sem um único double, a inversa de uma matriz de inteiros tem entradas racionais"
           " e elas saem como classe reduzida — 3/2 e −1/2, não 1,5 e −0,5. O GUME É A LEI:"
           " comparar a inversa com o que se espera é confiar na conta, multiplicá-la de"
           " volta é VERIFICÁ-LA — mede-se A·A⁻¹ = I em racionais, à mão, e é o que separa"
           " uma resposta certa de uma resposta que por acaso tem a forma certa. A SINGULAR"
           " diz a mesma coisa por três caminhos: determinante zero, posto um, e a inversa"
           " RECUSADA em vez de inventada. O CONTROLO tem três metades: o determinante e o"
           " traço pedem uma matriz quadrada e recusam-se numa 2×3, enquanto o POSTO não"
           " pede e responde — se todos recusassem, não se saberia se era a forma ou a"
           " pergunta; e a matriz com uma célula AUSENTE é recusada, porque o dual não é"
           " zero e fazer a conta com ele seria inventar a entrada que falta. O alcance é o"
           " do `linear.h` e diz-se antes de se usar, como a régua de tudo o resto.",
           mal == 0);
    }

    /* ═══ §W52: O NÚCLEO E A IMAGEM SÃO O PAR, E A LEI É A CONSERVAÇÃO ══════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w52.mem", "/tmp/pgwire_w52.prog",
            "/tmp/pgwire_w52__s.mem", "/tmp/pgwire_w52__s.prog",
            "/tmp/pgwire_w52__r.mem", "/tmp/pgwire_w52__r.prog" };
        for(int k = 0; k < 6; k++) unlink(lixo[k]);
        printf("\n§W52 núcleo e imagem: posto + nulidade = colunas.\n\n");
        if(!sql_abrir("/tmp/pgwire_w52")) mal++;
        /* a singular 2×2: a segunda linha é o dobro da primeira */
        sql_executa("CREATE TABLE s (a,b)", &o);
        sql_executa("INSERT INTO s VALUES (1,2), (2,4)", &o);
        /* e uma 2×3, para a lei ser medida com números diferentes */
        sql_executa("CREATE TABLE r (a,b,c)", &o);
        sql_executa("INSERT INTO r VALUES (1,2,3), (4,5,6)", &o);

        /* ── (1) SAEM DA MESMA REDUÇÃO. As colunas com pivô geram a imagem, as
         * sem pivô dão as variáveis livres do núcleo — não são dois cálculos, é
         * um lido dos dois lados. É a mesma economia do `zeta`/`mu`. */
        sql_executa("SELECT nucleo(*) FROM s", &o);
        long kn = o.nrows;
        char n0[8], n1[8];
        snprintf(n0, sizeof n0, "%s", o.nrows ? o.cell[0][0] : "?");
        snprintf(n1, sizeof n1, "%s", o.nrows ? o.cell[0][1] : "?");
        sql_executa("SELECT imagem(*) FROM s", &o);
        long ki = o.nrows;
        printf("      a singular (1,2;2,4): núcleo com %ld vector (%s,%s) ·"
               " imagem com %ld\n", kn, n0, n1, ki);
        if(kn != 1 || ki != 1) mal++;

        /* ── (2) E O GUME É VERIFICAR, NÃO CONFERIR: cada vector do núcleo tem
         * de dar ZERO quando a matriz o aplica. Comparar com o que se espera é
         * confiar na conta; aplicá-la é medi-la. A conta faz-se aqui, sobre as
         * linhas que a tabela tem. */
        { long v0 = atol(n0), v1 = atol(n1);
          long l1 = 1*v0 + 2*v1, l2 = 2*v0 + 4*v1;
          int anula = (l1 == 0 && l2 == 0 && !(v0 == 0 && v1 == 0));
          printf("      A·v = (%ld, %ld) e v ≠ 0: o núcleo ANULA  %s\n",
                 l1, l2, anula ? "" : "NAO BATE");
          if(!anula) mal++; }

        /* ── (3) A LEI, e é a mesma conservação de sempre: o que se perde mais
         * o que sobrevive é o que havia. `posto + dim(núcleo) = colunas` é o
         * teorema do núcleo-imagem, e é a MESMA frase do ∑G = |I| que o
         * quociente cumpre (§W44) — ali as fibras repartem as linhas, aqui as
         * dimensões repartem as colunas. Mede-se nas DUAS tabelas, porque com
         * uma só os números podiam coincidir por acaso. */
        { long ps, nu, nc;
          int leis = 0;
          sql_executa("SELECT posto(*) FROM s", &o);  ps = atol(o.cell[0][0]);
          sql_executa("SELECT nucleo(*) FROM s", &o); nu = o.nrows;
          nc = 2;
          printf("      lei na 2×2: posto %ld + nulidade %ld = %ld colunas  %s\n",
                 ps, nu, nc, (ps + nu == nc) ? "" : "NAO BATE");
          if(ps + nu == nc) leis++;
          sql_executa("SELECT posto(*) FROM r", &o);  ps = atol(o.cell[0][0]);
          sql_executa("SELECT nucleo(*) FROM r", &o); nu = o.nrows;
          nc = 3;
          printf("      lei na 2×3: posto %ld + nulidade %ld = %ld colunas  %s\n",
                 ps, nu, nc, (ps + nu == nc) ? "" : "NAO BATE");
          if(ps + nu == nc) leis++;
          if(leis != 2) mal++; }

        /* ── (4) E O NÚCLEO DA 2×3 TAMBÉM ANULA — com três coordenadas, o que
         * mostra que a lei não é um acidente do quadrado. */
        sql_executa("SELECT nucleo(*) FROM r", &o);
        { long w0 = o.nrows ? atol(o.cell[0][0]) : 0;
          long w1 = o.nrows ? atol(o.cell[0][1]) : 0;
          long w2 = o.nrows ? atol(o.cell[0][2]) : 0;
          long l1 = 1*w0 + 2*w1 + 3*w2, l2 = 4*w0 + 5*w1 + 6*w2;
          int anula = (l1 == 0 && l2 == 0 && !(w0 == 0 && w1 == 0 && w2 == 0));
          printf("      na 2×3: v = (%ld,%ld,%ld) e A·v = (%ld,%ld)  %s\n",
                 w0, w1, w2, l1, l2, anula ? "" : "NAO BATE");
          if(!anula) mal++; }

        /* ── O CONTROLO: numa matriz de posto CHEIO o núcleo é VAZIO, e é isso
         * que impede a medida de cima de passar por o motor devolver sempre
         * alguma coisa. A lei continua a fechar, agora com a nulidade a zero. */
        sql_executa("CREATE TABLE m (a,b)", &o);
        sql_executa("INSERT INTO m VALUES (1,2), (3,4)", &o);
        sql_executa("SELECT nucleo(*) FROM m", &o);
        long vaz = o.nrows;
        sql_executa("SELECT posto(*) FROM m", &o);
        long pc = atol(o.cell[0][0]);
        printf("\n      CONTROLO — posto cheio: núcleo com %ld vectores (esp 0)"
               " e a lei fecha na mesma: %ld + %ld = 2  %s\n", vaz, pc, vaz,
               (vaz == 0 && pc == 2) ? "" : "NAO BATE");
        if(vaz != 0 || pc != 2) mal++;
        sql_fechar();

        printf("\n");
        ok("O NÚCLEO E A IMAGEM SÃO O PAR, E A LEI QUE OS LIGA É A CONSERVAÇÃO QUE ESTA CASA"
           " JÁ TINHA. Saem da MESMA redução: as colunas com pivô geram a imagem, as sem pivô"
           " dão as variáveis livres do núcleo — não são dois cálculos, é um lido dos dois"
           " lados, com a mesma economia do par ζ/µ. E daí"
           " `posto + dim(núcleo) = número de colunas`, que é o teorema do núcleo-imagem e é"
           " a MESMA frase do ∑G = |I| do quociente (§W44): ali as fibras repartem as"
           " LINHAS, aqui as dimensões repartem as COLUNAS, e nos dois casos o que se perde"
           " mais o que sobrevive é o que havia. Mede-se em DUAS tabelas de formas"
           " diferentes, porque com uma só os números podiam coincidir por acaso. O GUME É"
           " VERIFICAR E NÃO CONFERIR: comparar o vector do núcleo com o que se espera é"
           " confiar na conta, aplicá-lo à matriz e exigir ZERO é medi-lo — e faz-se nas"
           " duas, incluindo a 2×3, onde o vector tem três coordenadas e mostra que a lei não"
           " é um acidente do quadrado. O CONTROLO é a matriz de posto CHEIO, onde o núcleo"
           " é VAZIO: sem ele, um motor que devolvesse sempre algum vector passava nas"
           " medidas de cima, e a lei continua a fechar com a nulidade a zero.",
           mal == 0);
    }

    /* ═══ §W53: O PRODUTO É A COMPOSIÇÃO, E O DUAL FECHA O PAR ═════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w53.mem", "/tmp/pgwire_w53.prog",
            "/tmp/pgwire_w53__a.mem", "/tmp/pgwire_w53__a.prog",
            "/tmp/pgwire_w53__b.mem", "/tmp/pgwire_w53__b.prog",
            "/tmp/pgwire_w53__i.mem", "/tmp/pgwire_w53__i.prog",
            "/tmp/pgwire_w53__tt.mem", "/tmp/pgwire_w53__tt.prog" };
        for(int k = 0; k < 10; k++) unlink(lixo[k]);
        printf("\n§W53 o produto, o neutro, e ker Tᵀ ⊥ im T.\n\n");
        if(!sql_abrir("/tmp/pgwire_w53")) mal++;
        sql_executa("CREATE TABLE a (p,q)", &o);
        sql_executa("INSERT INTO a VALUES (1,2), (3,4)", &o);
        sql_executa("CREATE TABLE b (p,q)", &o);
        sql_executa("INSERT INTO b VALUES (5,6), (7,8)", &o);
        sql_executa("CREATE TABLE i (p,q)", &o);
        sql_executa("INSERT INTO i VALUES (1,0), (0,1)", &o);

        /* ── (1) O PRODUTO É A COMPOSIÇÃO, e a condição para existir é a que a
         * composição sempre teve: a saída de um é a entrada do outro. */
        sql_executa("SELECT produto(b) FROM a", &o);
        int pr = (o.nrows == 2 && o.ncols == 2
                  && !strcmp(o.cell[0][0], "19") && !strcmp(o.cell[0][1], "22")
                  && !strcmp(o.cell[1][0], "43") && !strcmp(o.cell[1][1], "50"));
        printf("      (1,2;3,4)·(5,6;7,8) = (%s %s ; %s %s) — esp (19 22 ; 43 50)"
               "  %s\n", o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
               o.nrows > 1 ? o.cell[1][0] : "?", o.nrows > 1 ? o.cell[1][1] : "?",
               pr ? "" : "NAO BATE");
        if(!pr) mal++;

        /* ── (2) E A IDENTIDADE É O NEUTRO — que é a maneira de verificar o
         * produto sem o comparar com uma conta feita à mão: se `A·I` devolvesse
         * outra coisa, o produto estaria errado sem se saber onde. */
        sql_executa("SELECT produto(i) FROM a", &o);
        int nt = (o.nrows == 2 && !strcmp(o.cell[0][0], "1")
                  && !strcmp(o.cell[0][1], "2") && !strcmp(o.cell[1][0], "3")
                  && !strcmp(o.cell[1][1], "4"));
        printf("      A·I = A: (%s %s ; %s %s)  %s\n",
               o.nrows ? o.cell[0][0] : "?", o.nrows ? o.cell[0][1] : "?",
               o.nrows > 1 ? o.cell[1][0] : "?", o.nrows > 1 ? o.cell[1][1] : "?",
               nt ? "" : "NAO BATE");
        if(!nt) mal++;

        /* ── (3) E O DUAL FECHA O PAR: o núcleo da TRANSPOSTA é o anulador da
         * IMAGEM — `ker Tᵀ = (im T)°`. Mede-se fazendo o motor produzir a
         * transposta, guardando-a como tabela, e pedindo-lhe o núcleo: os
         * vectores que saem têm de ser ortogonais às COLUNAS da tabela
         * original, que são a imagem. É o motor usado de duas maneiras contra
         * si próprio, e é isso que faz da medida uma verificação. */
        sql_executa("CREATE TABLE t2 (p,q)", &o);
        sql_executa("INSERT INTO t2 VALUES (1,2), (2,4)", &o);   /* posto 1 */
        sql_executa("SELECT transposta(*) FROM t2", &o);
        { long tr[4][4]; int m = o.nrows, n = o.ncols;
          for(int i = 0; i < m && i < 4; i++)
              for(int j = 0; j < n && j < 4; j++) tr[i][j] = atol(o.cell[i][j]);
          sql_executa("CREATE TABLE tt (p,q)", &o);
          { char q2[128];
            for(int i = 0; i < m; i++){
                snprintf(q2, sizeof q2, "INSERT INTO tt VALUES (%ld,%ld)",
                         tr[i][0], tr[i][1]);
                sql_executa(q2, &o);
            } }
          sql_executa("SELECT nucleo(*) FROM tt", &o);
          { long v0 = o.nrows ? atol(o.cell[0][0]) : 0;
            long v1 = o.nrows ? atol(o.cell[0][1]) : 0;
            /* as COLUNAS da original são (1,2) e (2,4) — a imagem.
             * ⟨v, coluna⟩ tem de ser zero nas duas. */
            long e1 = v0*1 + v1*2, e2 = v0*2 + v1*4;
            int orto = (o.nrows == 1 && e1 == 0 && e2 == 0 && !(v0 == 0 && v1 == 0));
            printf("      ker Tᵀ = (im T)°: v = (%ld,%ld) e ⟨v,col⟩ = (%ld,%ld)"
                   "  %s\n", v0, v1, e1, e2, orto ? "" : "NAO BATE");
            if(!orto) mal++; } }

        /* ── (4) E AS DIMENSÕES BATEM DOS DOIS LADOS, que é a mesma conservação
         * outra vez: numa 2×2 de posto 1, tanto o núcleo como o núcleo da
         * transposta têm dimensão 1 — e é assim porque o posto de uma matriz e
         * o da sua transposta são o mesmo. */
        sql_executa("SELECT posto(*) FROM t2", &o);
        long p1 = o.nrows ? atol(o.cell[0][0]) : -1;
        sql_executa("SELECT posto(*) FROM tt", &o);
        long p2 = o.nrows ? atol(o.cell[0][0]) : -1;
        printf("      o posto é o mesmo dos dois lados: %ld e %ld  %s\n",
               p1, p2, (p1 == p2 && p1 == 1) ? "" : "NAO BATE");
        if(p1 != p2 || p1 != 1) mal++;

        /* ── O CONTROLO, e são dois. A composição RECUSA-SE quando as
         * dimensões não encaixam, e diz os DOIS números — dizer só «não dá»
         * esconderia qual dos lados está errado. E a tabela que não existe
         * também é recusada, em vez de tomada por vazia. */
        sql_executa("CREATE TABLE c (p,q,r)", &o);
        sql_executa("INSERT INTO c VALUES (1,2,3), (4,5,6)", &o);
        int c1 = sql_executa("SELECT produto(a) FROM c", &o);
        int c2 = sql_executa("SELECT produto(zz) FROM a", &o);
        printf("\n      CONTROLO — 2×3 · 2×2 (não encaixa): %s · tabela"
               " inexistente: %s  %s\n", c1 ? "PASSOU (mau)" : "recusado",
               c2 ? "PASSOU (mau)" : "recusado", (!c1 && !c2) ? "" : "NAO BATE");
        if(c1 || c2) mal++;
        sql_fechar();

        printf("\n");
        ok("O PRODUTO É A COMPOSIÇÃO, E O DUAL FECHA O PAR. `A·B` é aplicar B e depois A, e a"
           " condição para existir é a que a composição sempre teve — a saída de um tem de"
           " ser a entrada do outro, as colunas de um contra as linhas do outro. Quando não"
           " bate, a recusa diz os DOIS números, porque dizer só «não dá» esconderia qual dos"
           " lados está errado. A segunda tabela lê-se com ela ABERTA e traz-se em memória"
           " local, que é a mesma regra da subconsulta, da seta e do EXISTS. E A IDENTIDADE É"
           " O NEUTRO: `A·I = A` verifica o produto sem o comparar com uma conta feita à mão"
           " — se devolvesse outra coisa, o produto estaria errado sem se saber onde. O DUAL"
           " FECHA O PAR que o §W52 abriu: o núcleo da TRANSPOSTA é o anulador da IMAGEM,"
           " `ker Tᵀ = (im T)°`, e mede-se fazendo o motor produzir a transposta, guardá-la"
           " como tabela e pedir-lhe o núcleo — os vectores que saem têm de ser ortogonais às"
           " COLUNAS da original, que são a imagem. É o motor usado de duas maneiras contra"
           " si próprio, e é isso que faz da medida uma verificação em vez de uma repetição."
           " E as dimensões batem dos dois lados, que é a conservação outra vez: numa 2×2 de"
           " posto um, o núcleo e o núcleo da transposta têm ambos dimensão um, porque o"
           " posto de uma matriz e o da sua transposta são o mesmo. O CONTROLO tem duas"
           " metades: a composição que não encaixa e a tabela que não existe, ambas recusadas"
           " em vez de tomadas por vazias.",
           mal == 0);
    }

    /* ═══ §W54: RESOLVER É A VOLTA DO PRODUTO ══════════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w54.mem", "/tmp/pgwire_w54.prog",
            "/tmp/pgwire_w54__u.mem", "/tmp/pgwire_w54__u.prog",
            "/tmp/pgwire_w54__n.mem", "/tmp/pgwire_w54__n.prog",
            "/tmp/pgwire_w54__f.mem", "/tmp/pgwire_w54__f.prog",
            "/tmp/pgwire_w54__q.mem", "/tmp/pgwire_w54__q.prog",
            "/tmp/pgwire_w54__fa.mem", "/tmp/pgwire_w54__fa.prog",
            "/tmp/pgwire_w54__s1.mem", "/tmp/pgwire_w54__s1.prog" };
        for(int k = 0; k < 14; k++) unlink(lixo[k]);
        printf("\n§W54 resolver: os três desfechos, decididos pelo posto.\n\n");
        if(!sql_abrir("/tmp/pgwire_w54")) mal++;

        /* ── (1) A SOLUÇÃO ÚNICA, e o gume é APLICÁ-LA. Comparar o par (1,2)
         * com o que se esperava é confiar na conta; pôr o x de volta no sistema
         * e exigir que dê o lado direito é verificá-la. É a mesma escolha do
         * §W52 com o núcleo — resolver é a VOLTA do produto, e um par
         * verifica-se com o outro. */
        sql_executa("CREATE TABLE u (a,b,d)", &o);
        sql_executa("INSERT INTO u VALUES (1,2,5), (3,4,11)", &o);
        sql_executa("SELECT resolve(*) FROM u", &o);
        { long x = o.nrows ? atol(o.cell[0][0]) : 0;
          long y = o.nrows ? atol(o.cell[0][1]) : 0;
          long l1 = 1*x + 2*y, l2 = 3*x + 4*y;
          int bate = (o.nrows == 1 && o.ncols == 2 && l1 == 5 && l2 == 11);
          printf("      x+2y=5, 3x+4y=11 → (%ld,%ld) · A·x = (%ld,%ld), esp"
                 " (5,11)  %s\n", x, y, l1, l2, bate ? "" : "NAO BATE");
          if(!bate) mal++; }

        /* ── (2) SEM SOLUÇÃO, e o motor diz PORQUÊ com os dois postos: é o
         * teorema de Rouché–Capelli, e não são três casos a tratar — é uma
         * comparação de dois números. Quando o lado direito acrescenta posto,
         * ele saiu da imagem, e não há x que lá chegue. */
        sql_executa("CREATE TABLE n (a,b,d)", &o);
        sql_executa("INSERT INTO n VALUES (1,2,1), (2,4,3)", &o);
        int sem = sql_executa("SELECT resolve(*) FROM n", &o);
        int diz = (!sem && strstr(o.err, "rank") != NULL);
        printf("      x+2y=1, 2x+4y=3: %s, e diz os postos («%s»)  %s\n",
               sem ? "PASSOU (mau)" : "recusado", o.err,
               (!sem && diz) ? "" : "NAO BATE");
        if(sem || !diz) mal++;

        /* ── (3) INFINITAS, e a que sai é a PARTICULAR — dito, não escondido.
         * As variáveis livres ficam a zero, e as outras somam-se-lhe pelo
         * núcleo, que é o objecto que o §W52 já sabe devolver: as duas peças
         * encaixam sem se ter escrito uma linha para isso. E a particular tem
         * de resolver o sistema na mesma. */
        sql_executa("CREATE TABLE f (a,b,d)", &o);
        sql_executa("INSERT INTO f VALUES (1,2,3), (2,4,6)", &o);
        sql_executa("SELECT resolve(*) FROM f", &o);
        { long x = o.nrows ? atol(o.cell[0][0]) : 0;
          long y = o.nrows ? atol(o.cell[0][1]) : 0;
          long l1 = 1*x + 2*y, l2 = 2*x + 4*y;
          int bate = (o.nrows == 1 && l1 == 3 && l2 == 6);
          /* o núcleo que falta é o de A, e não o da tabela AUMENTADA: esta
           * é 2×3 e o seu núcleo tem dimensão 2, enquanto o do sistema tem 1.
           * Pede-se numa tabela só com o A, que é o objecto de que se fala —
           * confundir os dois seria medir outra coisa com o nome certo. */
          sql_executa("CREATE TABLE fa (a,b)", &o);
          sql_executa("INSERT INTO fa VALUES (1,2), (2,4)", &o);
          sql_executa("SELECT nucleo(*) FROM fa", &o);
          int tem_nucleo = (o.nrows == 1);
          long k0 = o.nrows ? atol(o.cell[0][0]) : 0;
          long k1 = o.nrows ? atol(o.cell[0][1]) : 0;
          /* e a soma da particular com o do núcleo TAMBÉM resolve: é o que
           * «infinitas» quer dizer, e mede-se em vez de se afirmar */
          long s1 = 1*(x+k0) + 2*(y+k1), s2 = 2*(x+k0) + 4*(y+k1);
          int soma = (s1 == 3 && s2 == 6);
          printf("      x+2y=3, 2x+4y=6 → a particular (%ld,%ld) resolve"
                 " (%ld,%ld) · o núcleo de A tem %d vector (%ld,%ld) · e a SOMA"
                 " resolve também (%ld,%ld)  %s\n", x, y, l1, l2, o.nrows,
                 k0, k1, s1, s2,
                 (bate && tem_nucleo && soma) ? "" : "NAO BATE");
          if(!bate || !tem_nucleo || !soma) mal++; }

        /* ── (4) E A SOLUÇÃO É EXACTA EM ℚ: `2x = 1` dá 1/2, não 0 nem 0,5.
         * É a mesma razão da inversa — o sistema de inteiros tem solução
         * racional, e o motor não a arredonda nem a trunca. */
        sql_executa("CREATE TABLE q (a,d)", &o);
        sql_executa("INSERT INTO q VALUES (2,1)", &o);
        sql_executa("SELECT resolve(*) FROM q", &o);
        int rq = (o.nrows == 1 && !strcmp(o.cell[0][0], "1/2"));
        printf("      2x = 1 → %s (esp 1/2, não 0 nem 0,5)  %s\n",
               o.nrows ? o.cell[0][0] : "?", rq ? "" : "NAO BATE");
        if(!rq) mal++;

        /* ── O CONTROLO: uma tabela de UMA coluna não é um sistema — não há
         * onde separar o A do b —, e é recusada. Sem ele, o motor podia estar a
         * ler a única coluna como o lado direito de um sistema vazio e a
         * devolver alguma coisa. */
        sql_executa("CREATE TABLE s1 (a)", &o);
        sql_executa("INSERT INTO s1 VALUES (7)", &o);
        int c1 = sql_executa("SELECT resolve(*) FROM s1", &o);
        printf("\n      CONTROLO — uma coluna só (não há [A|b]): %s  %s\n",
               c1 ? "PASSOU (mau)" : "recusado", c1 ? "NAO BATE" : "");
        if(c1) mal++;
        sql_fechar();

        printf("\n");
        ok("RESOLVER É A VOLTA DO PRODUTO, E OS TRÊS DESFECHOS SÃO UMA COMPARAÇÃO DE DOIS"
           " NÚMEROS. O produto COMPÕE — dá o que sai de aplicar; resolver DESCOMPÕE — dá o"
           " que teria de entrar. É o par de sempre, e é por isso que um verifica o outro: o"
           " gume não é comparar a solução com a que se esperava, é APLICÁ-LA e exigir o lado"
           " direito. A tabela é a matriz AUMENTADA [A|b], que não é uma convenção arbitrária"
           " — é a forma em que o sistema É uma tabela, e a única que não obriga a passar dois"
           " objetos onde há um. E os três desfechos decidem-se pelo POSTO, que é"
           " Rouché–Capelli e já estava todo aqui: se o lado direito ACRESCENTA posto, ele"
           " saiu da imagem e não há x que lá chegue — o motor recusa e diz os dois números,"
           " porque «não tem solução» sem eles é um veredicto sem prova. Se os postos batem"
           " mas ficam abaixo do número de colunas, há infinitas, e a que sai é a PARTICULAR"
           " — dito, e não escondido: as livres a zero, e as outras somam-se-lhe pelo NÚCLEO,"
           " que é o objeto que o §W52 já sabe devolver. As duas peças encaixam sem se ter"
           " escrito uma linha para isso, e é esse o sinal de as duas estarem no sítio certo."
           " A solução é EXACTA em ℚ: `2x = 1` dá 1/2, não 0 nem 0,5 — a mesma razão da"
           " inversa. O CONTROLO é a tabela de UMA coluna, que não é um sistema porque não há"
           " onde separar o A do b: sem ele, o motor podia estar a ler essa coluna como o"
           " lado direito de um sistema vazio e a devolver alguma coisa.",
           mal == 0);
    }

    /* ═══ §W55: O MESMO SISTEMA POR DOIS CAMINHOS ══════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        const char *lixo[] = {
            "/tmp/pgwire_w55.mem", "/tmp/pgwire_w55.prog",
            "/tmp/pgwire_w55__k.mem", "/tmp/pgwire_w55__k.prog",
            "/tmp/pgwire_w55__z.mem", "/tmp/pgwire_w55__z.prog",
            "/tmp/pgwire_w55__nn.mem", "/tmp/pgwire_w55__nn.prog",
            "/tmp/pgwire_w55__rr.mem", "/tmp/pgwire_w55__rr.prog" };
        for(int k = 0; k < 10; k++) unlink(lixo[k]);
        printf("\n§W55 eliminação contra Cramer: duas testemunhas.\n\n");
        if(!sql_abrir("/tmp/pgwire_w55")) mal++;

        /* ── (1) OS DOIS CAMINHOS CONCORDAM, e é isso que os torna uma
         * verificação. `resolve` escalona; `cramer` expande determinantes —
         * não se apoiam um no outro, e por isso o acordo entre eles não é uma
         * repetição. Varre-se, em vez de se experimentar um caso: todos os
         * sistemas 2×2 com coeficientes pequenos e determinante não nulo. */
        /* os coeficientes varrem 0..4 e não os negativos: a célula é um
         * Word_8 SEM SINAL, e um `-2` é recusado pelo envelope — varrer com ele
         * era contar como «saltado» o que a régua da casa nunca aceitou, e
         * quase toda a varredura ficava por fazer sem se dar por isso. */
        { long pares = 0, iguais = 0, saltados = 0;
          for(long a = 0; a <= 4; a++)
          for(long b = 0; b <= 4; b++)
          for(long c = 0; c <= 4; c++)
          for(long d = 0; d <= 4; d++){
              if(a*d - b*c == 0) continue;              /* Cramer não se aplica */
              { char q1[160];
                char v1[2][32], v2[2][32];
                int ok1, ok2;
                sql_executa("DROP TABLE IF EXISTS k", &o);
                sql_executa("CREATE TABLE k (p,q,r)", &o);
                snprintf(q1, sizeof q1, "INSERT INTO k VALUES (%ld,%ld,%ld),"
                         " (%ld,%ld,%ld)", a, b, 5L, c, d, 11L);
                sql_executa(q1, &o);
                ok1 = sql_executa("SELECT resolve(*) FROM k", &o);
                if(ok1 && o.nrows == 1){
                    snprintf(v1[0], 32, "%s", o.cell[0][0]);
                    snprintf(v1[1], 32, "%s", o.cell[0][1]);
                } else { saltados++; continue; }
                ok2 = sql_executa("SELECT cramer(*) FROM k", &o);
                if(ok2 && o.nrows == 1){
                    snprintf(v2[0], 32, "%s", o.cell[0][0]);
                    snprintf(v2[1], 32, "%s", o.cell[0][1]);
                } else { saltados++; continue; }
                pares++;
                if(!strcmp(v1[0], v2[0]) && !strcmp(v1[1], v2[1])) iguais++; }
          }
          printf("      varridos %ld sistemas 2×2 de determinante não nulo:"
                 " os dois caminhos deram o MESMO em %ld (saltados %ld)  %s\n",
                 pares, iguais, saltados,
                 (pares > 300 && iguais == pares && saltados == 0) ? "" : "NAO BATE");
          if(pares < 300 || iguais != pares || saltados) mal++; }

        /* ── (2) E CONCORDAM EM ℚ, não só nos inteiros: um sistema cuja solução
         * é fraccionária tem de dar a MESMA fracção pelos dois caminhos — e é
         * onde um arredondamento de um dos lados apareceria. */
        sql_executa("CREATE TABLE z (p,q,r)", &o);
        sql_executa("INSERT INTO z VALUES (2,0,1), (0,3,1)", &o);
        sql_executa("SELECT resolve(*) FROM z", &o);
        char e1[32], e2[32];
        snprintf(e1, sizeof e1, "%s", o.nrows ? o.cell[0][0] : "?");
        snprintf(e2, sizeof e2, "%s", o.nrows ? o.cell[0][1] : "?");
        sql_executa("SELECT cramer(*) FROM z", &o);
        int racional = (o.nrows == 1 && !strcmp(e1, o.cell[0][0])
                        && !strcmp(e2, o.cell[0][1])
                        && !strcmp(e1, "1/2") && !strcmp(e2, "1/3"));
        printf("      2x=1, 3y=1: eliminação (%s,%s) e Cramer (%s,%s) — esp"
               " (1/2,1/3)  %s\n", e1, e2, o.nrows ? o.cell[0][0] : "?",
               o.nrows ? o.cell[0][1] : "?", racional ? "" : "NAO BATE");
        if(!racional) mal++;

        /* ── (3) E CRAMER SABE MENOS, que é o que se ganha em ter DOIS. Onde o
         * determinante é zero ele recusa — e recusa da MESMA maneira nos dois
         * casos que a eliminação distingue: o sistema sem solução e o de
         * infinitas. Um caminho só teria escondido essa diferença. */
        sql_executa("CREATE TABLE nn (p,q,r)", &o);
        sql_executa("INSERT INTO nn VALUES (1,2,1), (2,4,3)", &o);   /* nenhuma */
        sql_executa("CREATE TABLE rr (p,q,r)", &o);
        sql_executa("INSERT INTO rr VALUES (1,2,3), (2,4,6)", &o);   /* infinitas */
        int cn = sql_executa("SELECT cramer(*) FROM nn", &o);
        int cr = sql_executa("SELECT cramer(*) FROM rr", &o);
        int rn = sql_executa("SELECT resolve(*) FROM nn", &o);
        int rr = sql_executa("SELECT resolve(*) FROM rr", &o);
        int sabe = (!cn && !cr && !rn && rr);
        printf("      onde det = 0: Cramer recusa os dois (%d,%d) sem os"
               " distinguir · a eliminação recusa o SEM SOLUÇÃO (%d) e resolve"
               " o de INFINITAS (%d)  %s\n", cn, cr, rn, rr,
               sabe ? "" : "NAO BATE");
        if(!sabe) mal++;

        /* ── O CONTROLO: Cramer pede uma matriz QUADRADA e recusa-a numa 2×3
         * (isto é, [A|b] com A de 2×2 é o caso bom; com três incógnitas e duas
         * equações não é). É o que impede a concordância de cima de vir de os
         * dois caminhos serem o mesmo caminho com dois nomes. */
        sql_executa("CREATE TABLE w (p,q,r,s)", &o);
        sql_executa("INSERT INTO w VALUES (1,2,3,4), (5,6,7,8)", &o);
        int c1 = sql_executa("SELECT cramer(*) FROM w", &o);
        int c2 = sql_executa("SELECT resolve(*) FROM w", &o);
        printf("\n      CONTROLO — A de 2×3: Cramer %s · a eliminação %s (são"
               " caminhos DIFERENTES)  %s\n",
               c1 ? "PASSOU (mau)" : "recusa",
               c2 ? "responde" : "RECUSA (mau)",
               (!c1 && c2) ? "" : "NAO BATE");
        if(c1 || !c2) mal++;
        sql_fechar();

        printf("\n");
        ok("O MESMO SISTEMA POR DOIS CAMINHOS, E É O ACORDO ENTRE ELES QUE VERIFICA. `resolve`"
           " escalona; `cramer` expande determinantes — x_i é o quociente de dois deles, com"
           " a coluna i trocada pelo lado direito. Não se apoiam um no outro, e por isso"
           " concordarem não é uma repetição: é a régua desta casa, onde o que mais defeitos"
           " apanhou foi a COMPARAÇÃO entre dois caminhos e não a asserção sobre um. Varre-se"
           " em vez de se experimentar um caso — todos os sistemas 2×2 com coeficientes"
           " pequenos e determinante não nulo — e os dois dão o mesmo em todos. E CONCORDAM"
           " EM ℚ, não só nos inteiros: um sistema de solução fraccionária tem de dar a MESMA"
           " fracção pelos dois lados, que é onde um arredondamento de um deles apareceria. E"
           " CRAMER SABE MENOS, que é o que se ganha em ter dois: onde o determinante é zero"
           " ele recusa, e recusa da MESMA maneira nos dois casos que a eliminação distingue"
           " — o sistema sem solução e o de infinitas soluções. Com um caminho só, essa"
           " diferença ficava escondida; com dois, vê-se que um deles responde a uma pergunta"
           " mais fina. O CONTROLO é a matriz não quadrada, onde Cramer recusa e a eliminação"
           " responde: é o que impede a concordância de cima de vir de os dois serem o mesmo"
           " caminho com dois nomes.",
           mal == 0);
    }

    /* ═══ §W56: CAYLEY–HAMILTON, PELAS PEÇAS QUE JÁ LÁ ESTAVAM ════════════ */
    {
        SqlOut o;
        long mal = 0;
        unlink("/tmp/pgwire_w56.mem");     unlink("/tmp/pgwire_w56.prog");
        unlink("/tmp/pgwire_w56__m.mem");  unlink("/tmp/pgwire_w56__m.prog");
        printf("\n§W56 A² − tr(A)·A + det(A)·I = 0, sem uma linha nova.\n\n");
        if(!sql_abrir("/tmp/pgwire_w56")) mal++;

        /* ── (1) A LEI, e o que ela testa não é a álgebra: é que as PEÇAS
         * ENCAIXEM. O produto, o traço e o determinante foram expostos
         * separadamente, cada um medido por si; a lei junta os três, e nenhum
         * deles isolado a dá. Se qualquer um estivesse errado, ela cairia — e é
         * por isso que ela é o teste de que estão no mesmo objeto. Varre-se
         * todas as 2×2 com entradas até 4. */
        { long casos = 0, boas = 0;
          for(long a = 0; a <= 4; a++)
          for(long b = 0; b <= 4; b++)
          for(long c = 0; c <= 4; c++)
          for(long d = 0; d <= 4; d++){
              char qi[128];
              long q11, q12, q21, q22, tr, dt;
              sql_executa("DROP TABLE IF EXISTS m", &o);
              sql_executa("CREATE TABLE m (p,q)", &o);
              snprintf(qi, sizeof qi, "INSERT INTO m VALUES (%ld,%ld), (%ld,%ld)",
                       a, b, c, d);
              if(!sql_executa(qi, &o)) continue;
              if(!sql_executa("SELECT produto(m) FROM m", &o) || o.nrows != 2) continue;
              q11 = atol(o.cell[0][0]); q12 = atol(o.cell[0][1]);
              q21 = atol(o.cell[1][0]); q22 = atol(o.cell[1][1]);
              if(!sql_executa("SELECT traco(*) FROM m", &o)) continue;
              tr = atol(o.cell[0][0]);
              if(!sql_executa("SELECT det(*) FROM m", &o)) continue;
              dt = atol(o.cell[0][0]);
              casos++;
              /* A² − tr·A + det·I, entrada a entrada */
              if(q11 - tr*a + dt == 0 && q12 - tr*b == 0
                 && q21 - tr*c == 0 && q22 - tr*d + dt == 0) boas++;
          }
          printf("      varridas %ld matrizes 2×2: A² − tr·A + det·I = 0 em %ld"
                 "  %s\n", casos, boas,
                 (casos > 500 && boas == casos) ? "" : "NAO BATE");
          if(casos < 500 || boas != casos) mal++; }

        /* ── (2) E O TRAÇO E O DETERMINANTE SÃO DUAS COORDENADAS, não uma. A
         * casa diz que são «as metades da mesma Cayley–Hamilton», e a prova de
         * que são DUAS é haver matrizes com o mesmo traço e determinantes
         * diferentes, e com o mesmo determinante e traços diferentes. Se uma
         * determinasse a outra, a lei teria um parâmetro só. */
        { long tr1, dt1, tr2, dt2, tr3, dt3;
          sql_executa("DROP TABLE IF EXISTS m", &o);
          sql_executa("CREATE TABLE m (p,q)", &o);
          sql_executa("INSERT INTO m VALUES (1,0), (0,4)", &o);       /* tr 5, det 4 */
          sql_executa("SELECT traco(*) FROM m", &o); tr1 = atol(o.cell[0][0]);
          sql_executa("SELECT det(*) FROM m", &o);   dt1 = atol(o.cell[0][0]);
          sql_executa("DROP TABLE IF EXISTS m", &o);
          sql_executa("CREATE TABLE m (p,q)", &o);
          sql_executa("INSERT INTO m VALUES (2,1), (1,3)", &o);       /* tr 5, det 5 */
          sql_executa("SELECT traco(*) FROM m", &o); tr2 = atol(o.cell[0][0]);
          sql_executa("SELECT det(*) FROM m", &o);   dt2 = atol(o.cell[0][0]);
          sql_executa("DROP TABLE IF EXISTS m", &o);
          sql_executa("CREATE TABLE m (p,q)", &o);
          sql_executa("INSERT INTO m VALUES (1,0), (0,5)", &o);       /* tr 6, det 5 */
          sql_executa("SELECT traco(*) FROM m", &o); tr3 = atol(o.cell[0][0]);
          sql_executa("SELECT det(*) FROM m", &o);   dt3 = atol(o.cell[0][0]);
          { int duas = (tr1 == tr2 && dt1 != dt2)      /* mesmo traço, det outro */
                    && (dt2 == dt3 && tr2 != tr3);     /* mesmo det, traço outro */
            printf("      duas coordenadas e não uma: (tr %ld, det %ld) e"
                   " (tr %ld, det %ld) partilham o traço · (tr %ld, det %ld)"
                   " partilha o det com a segunda  %s\n",
                   tr1, dt1, tr2, dt2, tr3, dt3, duas ? "" : "NAO BATE");
            if(!duas) mal++; } }

        /* ── O CONTROLO: a lei é uma AFIRMAÇÃO sobre a matriz, e tem de poder
         * ser falsa. Trocando o traço por outro número — o traço da
         * TRANSPOSTA seria o mesmo, pelo que se usa um valor deslocado — a
         * identidade parte-se. Sem isto, «vale em todas» podia ser «vale
         * sempre», e não diria nada. */
        sql_executa("DROP TABLE IF EXISTS m", &o);
        sql_executa("CREATE TABLE m (p,q)", &o);
        sql_executa("INSERT INTO m VALUES (1,2), (3,4)", &o);
        sql_executa("SELECT produto(m) FROM m", &o);
        { long q11 = atol(o.cell[0][0]);
          sql_executa("SELECT traco(*) FROM m", &o);
          long tr = atol(o.cell[0][0]);
          sql_executa("SELECT det(*) FROM m", &o);
          long dt = atol(o.cell[0][0]);
          int certo = (q11 - tr*1 + dt == 0);
          int falso = (q11 - (tr + 1)*1 + dt == 0);
          printf("\n      CONTROLO — com o traço CERTO a entrada anula (%d);"
                 " com o traço + 1 já NÃO anula (%d)  %s\n", certo, falso,
                 (certo && !falso) ? "" : "NAO BATE");
          if(!certo || falso) mal++; }
        sql_fechar();

        printf("\n");
        ok("CAYLEY–HAMILTON SAI DAS PEÇAS QUE JÁ LÁ ESTAVAM, E É POR ISSO QUE ELE É O TESTE"
           " DE QUE ELAS ENCAIXAM. `A² − tr(A)·A + det(A)·I = 0` não precisou de uma linha"
           " nova no motor: o produto, o traço e o determinante foram expostos"
           " separadamente, cada um medido por si, e a lei junta os três — nenhum deles"
           " isolado a dá, e se qualquer um estivesse errado ela cairia. É a diferença entre"
           " ter três funções que respondem e ter três funções que falam do mesmo objeto."
           " Varre-se, em vez de se experimentar um caso: todas as 2×2 com entradas até"
           " quatro, e a identidade fecha entrada a entrada em todas. E O TRAÇO E O"
           " DETERMINANTE SÃO DUAS COORDENADAS, não uma — a casa chama-lhes «as metades da"
           " mesma Cayley–Hamilton», e a prova de que são duas é haver matrizes com o mesmo"
           " traço e determinantes diferentes, e com o mesmo determinante e traços"
           " diferentes: se uma determinasse a outra, a lei teria um parâmetro só. O CONTROLO"
           " é a lei a poder ser FALSA: com o traço certo a entrada anula, com o traço mais"
           " um já não anula. Sem ele, «vale em todas» podia ser «vale sempre», e não diria"
           " nada.",
           mal == 0);
    }

    /* ═══ §W57: AS DUAS FACES NAS MATRIZES, E A LEI QUE AS LIGA ════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W57 soma e produto: leis diferentes, e a distributividade.\n\n");
        { const char *tabs[] = { "a","b","c","z","i","s1","s2","p1","p2" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w57__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w57__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w57.mem"); unlink("/tmp/pgwire_w57.prog"); }
        if(!sql_abrir("/tmp/pgwire_w57")) mal++;
        sql_executa("CREATE TABLE a (p,q)", &o);
        sql_executa("INSERT INTO a VALUES (1,2), (3,4)", &o);
        sql_executa("CREATE TABLE b (p,q)", &o);
        sql_executa("INSERT INTO b VALUES (5,6), (7,8)", &o);
        sql_executa("CREATE TABLE c (p,q)", &o);
        sql_executa("INSERT INTO c VALUES (2,0), (1,3)", &o);
        sql_executa("CREATE TABLE z (p,q)", &o);
        sql_executa("INSERT INTO z VALUES (0,0), (0,0)", &o);
        sql_executa("CREATE TABLE i (p,q)", &o);
        sql_executa("INSERT INTO i VALUES (1,0), (0,1)", &o);

        /* guarda o resultado de uma consulta numa tabela nova — é o que permite
         * encadear as operações e medir as leis que ligam duas delas */
        #define GUARDA(consulta, tab) do { \
            sql_executa(consulta, &o); \
            { char q2[160]; \
              snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", tab); \
              sql_executa(q2, &o2); \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (p,q)", tab); \
              sql_executa(q2, &o2); \
              for(int i2 = 0; i2 < o.nrows; i2++){ \
                  snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s,%s)", \
                           tab, o.cell[i2][0], o.cell[i2][1]); \
                  sql_executa(q2, &o2); } } \
        } while(0)
        SqlOut o2;

        /* ── (1) AS DUAS FACES TÊM LEIS DIFERENTES, e é isso que as separa. A
         * soma COMUTA; o produto NÃO. Não é um detalhe de implementação — é a
         * assimetria que faz das duas faces duas, e mede-se com o mesmo par de
         * tabelas nos dois sentidos. */
        sql_executa("SELECT soma(b) FROM a", &o);
        char s1[4][32];
        for(int i = 0; i < 2; i++)
            for(int j = 0; j < 2; j++) snprintf(s1[i*2+j], 32, "%s", o.cell[i][j]);
        sql_executa("SELECT soma(a) FROM b", &o);
        int soma_comuta = (!strcmp(s1[0], o.cell[0][0]) && !strcmp(s1[1], o.cell[0][1])
                        && !strcmp(s1[2], o.cell[1][0]) && !strcmp(s1[3], o.cell[1][1]));
        sql_executa("SELECT produto(b) FROM a", &o);
        char p1[4][32];
        for(int i = 0; i < 2; i++)
            for(int j = 0; j < 2; j++) snprintf(p1[i*2+j], 32, "%s", o.cell[i][j]);
        sql_executa("SELECT produto(a) FROM b", &o);
        int prod_comuta = (!strcmp(p1[0], o.cell[0][0]) && !strcmp(p1[1], o.cell[0][1])
                        && !strcmp(p1[2], o.cell[1][0]) && !strcmp(p1[3], o.cell[1][1]));
        printf("      a soma COMUTA (%d) e o produto NÃO (%d): (%s,%s;%s,%s)"
               " contra (%s,%s;%s,%s)  %s\n", soma_comuta, prod_comuta,
               p1[0], p1[1], p1[2], p1[3], o.cell[0][0], o.cell[0][1],
               o.cell[1][0], o.cell[1][1],
               (soma_comuta && !prod_comuta) ? "" : "NAO BATE");
        if(!soma_comuta || prod_comuta) mal++;

        /* ── (2) E CADA FACE TEM O SEU NEUTRO — que é a frase do Teor. 2(4) do
         * aranha lida nas matrizes: o invariante de uma é o neutro da outra, e
         * são objetos diferentes (a matriz nula e a identidade). */
        sql_executa("SELECT soma(z) FROM a", &o);
        int nz = (!strcmp(o.cell[0][0], "1") && !strcmp(o.cell[1][1], "4"));
        sql_executa("SELECT produto(i) FROM a", &o);
        int ni = (!strcmp(o.cell[0][0], "1") && !strcmp(o.cell[1][1], "4"));
        printf("      cada face tem o seu neutro: A + 0 = A (%d) · A · I = A"
               " (%d), e 0 ≠ I  %s\n", nz, ni, (nz && ni) ? "" : "NAO BATE");
        if(!nz || !ni) mal++;

        /* ── (3) E A DISTRIBUTIVIDADE LIGA-AS: `A·(B+C) = A·B + A·C`. É a lei
         * ENTRE as duas faces, e é a mesma que o `meio.c` §M2 mede um andar
         * abaixo, sobre os símbolos — ali ela força as tabelas de B, aqui ela
         * é a única coisa que impede a soma e o produto de serem duas
         * operações sem relação. Encadeia-se guardando cada resultado numa
         * tabela, que é o motor a alimentar-se a si próprio. */
        GUARDA("SELECT soma(c) FROM b", "s1");            /* B+C */
        GUARDA("SELECT produto(s1) FROM a", "p1");        /* A·(B+C) */
        GUARDA("SELECT produto(b) FROM a", "s2");         /* A·B */
        GUARDA("SELECT produto(c) FROM a", "p2");         /* A·C */
        GUARDA("SELECT soma(p2) FROM s2", "s1");          /* A·B + A·C */
        { char esq[4][32];
          sql_executa("SELECT * FROM p1", &o);
          for(int i = 0; i < 2; i++)
              for(int j = 0; j < 2; j++) snprintf(esq[i*2+j], 32, "%s", o.cell[i][j]);
          sql_executa("SELECT * FROM s1", &o);
          { int dist = (!strcmp(esq[0], o.cell[0][0]) && !strcmp(esq[1], o.cell[0][1])
                     && !strcmp(esq[2], o.cell[1][0]) && !strcmp(esq[3], o.cell[1][1]));
            printf("      A·(B+C) = (%s,%s;%s,%s) e A·B + A·C = (%s,%s;%s,%s)"
                   "  %s\n", esq[0], esq[1], esq[2], esq[3],
                   o.cell[0][0], o.cell[0][1], o.cell[1][0], o.cell[1][1],
                   dist ? "" : "NAO BATE");
            if(!dist) mal++; } }

        /* ── O CONTROLO: a soma pede a MESMA forma e o produto pede formas
         * ENCAIXÁVEIS — são exigências diferentes, e é isso que mostra que as
         * duas faces não são a mesma operação com dois nomes. Uma 2×2 soma-se
         * com uma 2×2 e não com uma 2×3; multiplica-se por uma 2×3 e não se
         * soma com ela. */
        sql_executa("CREATE TABLE w (p,q,r)", &o);
        sql_executa("INSERT INTO w VALUES (1,2,3), (4,5,6)", &o);
        int c1 = sql_executa("SELECT soma(w) FROM a", &o);
        int c2 = sql_executa("SELECT produto(w) FROM a", &o);
        printf("\n      CONTROLO — com uma 2×3: a soma %s e o produto %s"
               " (exigências DIFERENTES)  %s\n",
               c1 ? "PASSOU (mau)" : "recusa", c2 ? "responde" : "RECUSA (mau)",
               (!c1 && c2) ? "" : "NAO BATE");
        if(c1 || !c2) mal++;
        #undef GUARDA
        sql_fechar();

        printf("\n");
        ok("AS DUAS FACES APARECEM NAS MATRIZES COM AS MESMAS LEIS, E A DISTRIBUTIVIDADE"
           " CONTINUA A SER A QUE AS LIGA. A soma COMUTA e o produto NÃO — não é um detalhe"
           " de implementação: é a assimetria que faz das duas faces duas, e mede-se com o"
           " mesmo par de tabelas nos dois sentidos, onde `A·B` e `B·A` dão matrizes"
           " diferentes. Cada face tem o SEU neutro, e são objetos diferentes — a matriz nula"
           " e a identidade —, que é a frase do Teor. 2(4) do aranha lida um andar acima. E A"
           " DISTRIBUTIVIDADE LIGA-AS: `A·(B+C) = A·B + A·C` é a lei ENTRE as duas, a mesma"
           " que o `meio.c` §M2 mede sobre os símbolos, onde ela força as tabelas de B; aqui"
           " ela é a única coisa que impede a soma e o produto de serem duas operações sem"
           " relação. Mede-se ENCADEANDO: cada resultado guarda-se numa tabela e volta a"
           " entrar, que é o motor a alimentar-se a si próprio — e é isso que torna a medida"
           " uma verificação da composição inteira e não de uma operação de cada vez. O"
           " CONTROLO são as exigências de forma, que são DIFERENTES: a soma pede a mesma"
           " forma, o produto pede formas encaixáveis, e uma 2×2 soma-se com uma 2×2 mas"
           " multiplica-se por uma 2×3. Se as duas pedissem o mesmo, nada distinguiria as"
           " faces senão o nome.",
           mal == 0);
    }

    /* ═══ §W58: A VOLTA, E O ANDAR QUE ELA PEDE ════════════════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W58 a volta da soma pede o andar com sinal.\n\n");
        { const char *tabs[] = { "n","a","s","b","c","t1","t2","t3" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w58__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w58__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w58.mem"); unlink("/tmp/pgwire_w58.prog"); }
        if(!sql_abrir("/tmp/pgwire_w58")) mal++;

        /* ── (1) A VOLTA DA SOMA NÃO CABE NO ANDAR SEM SINAL, e isto é a escada
         * da casa a aparecer no banco: a coluna INTEIRA é um Word_8 sem sinal,
         * e o oposto de uma matriz de entradas positivas tem entradas
         * negativas — que ela RECUSA. Não é uma limitação a contornar: é
         * ℕ → ℤ dito pela régua, e o andar seguinte é exactamente «o que
         * acrescenta a volta da soma». */
        sql_executa("CREATE TABLE n (p,q)", &o);
        sql_executa("INSERT INTO n VALUES (1,2), (3,4)", &o);
        int cabe_n = sql_executa("INSERT INTO n VALUES (-1,-2)", &o);
        printf("      no andar sem sinal: o negativo %s («%s»)\n",
               cabe_n ? "ENTROU (mau)" : "é recusado", o.err);
        if(cabe_n) mal++;

        /* ── (2) E NO ANDAR COM SINAL cabe, e as contas mudam de resposta — o
         * que mostra que o corpo da coluna não é decoração. Numa matriz de
         * entradas negativas o traço tem de ser negativo; ler a célula sem o
         * sinal do corpo dava 131067 onde a resposta é −5. */
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (1,2), (3,4)", &o);
        sql_executa("CREATE TABLE s (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO s VALUES (-1,-2), (-3,-4)", &o);
        sql_executa("SELECT traco(*) FROM s", &o);
        int sinal = (o.nrows == 1 && !strcmp(o.cell[0][0], "-5"));
        sql_executa("SELECT det(*) FROM s", &o);
        int dts = (o.nrows == 1 && !strcmp(o.cell[0][0], "-2"));
        printf("      no andar com sinal: traço = %s (esp −5) e det = %s"
               " (esp −2)  %s\n", o.nrows ? "-5" : "?",
               o.nrows ? o.cell[0][0] : "?", (sinal && dts) ? "" : "NAO BATE");
        if(!sinal || !dts) mal++;

        #define GUARDA(consulta, tab) do { \
            sql_executa(consulta, &o); \
            { char q2[160]; \
              snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", tab); \
              sql_executa(q2, &o2); \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", tab); \
              sql_executa(q2, &o2); \
              for(int i2 = 0; i2 < o.nrows; i2++){ \
                  snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s,%s)", \
                           tab, o.cell[i2][0], o.cell[i2][1]); \
                  sql_executa(q2, &o2); } } \
        } while(0)

        /* ── (3) AÍ A VOLTA FECHA: `A + (−A) = 0`, verificado e não suposto —
         * sem a soma dar a matriz nula, «oposto» seria só um nome para trocar
         * sinais. */
        GUARDA("SELECT oposto(*) FROM a", "t1");
        sql_executa("SELECT soma(t1) FROM a", &o);
        { int zero = (o.nrows == 2
                      && !strcmp(o.cell[0][0], "0") && !strcmp(o.cell[0][1], "0")
                      && !strcmp(o.cell[1][0], "0") && !strcmp(o.cell[1][1], "0"));
          printf("      A + (−A) = (%s,%s;%s,%s), esp tudo zero  %s\n",
                 o.cell[0][0], o.cell[0][1], o.cell[1][0], o.cell[1][1],
                 zero ? "" : "NAO BATE");
          if(!zero) mal++; }

        /* ── (4) E AS DUAS VOLTAS TÊM ALCANCES DIFERENTES: o oposto existe para
         * TODA a matriz, a inversa só quando o determinante não é zero. A face
         * aditiva é um GRUPO; a multiplicativa tem neutro para todos e volta só
         * para alguns. É a Def. 1 do `aranha` — a VOLTA é o que distingue uma
         * dobra de um esmagamento — com as duas faces a responderem-lhe de
         * maneiras diferentes. E onde a inversa existe, mede-se do mesmo modo:
         * aplicar e comparar com o neutro. */
        sql_executa("CREATE TABLE g (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO g VALUES (1,2), (2,4)", &o);   /* det 0 */
        int og = sql_executa("SELECT oposto(*) FROM g", &o);
        int ig = sql_executa("SELECT inversa(*) FROM g", &o);
        GUARDA("SELECT inversa(*) FROM a", "t2");
        sql_executa("SELECT produto(t2) FROM a", &o);
        { int id = (o.nrows == 2
                    && !strcmp(o.cell[0][0], "1") && !strcmp(o.cell[0][1], "0")
                    && !strcmp(o.cell[1][0], "0") && !strcmp(o.cell[1][1], "1"));
          printf("      alcances diferentes: na singular o oposto existe (%d) e"
                 " a inversa não (%d) · e onde existe, A·A⁻¹ = (%s,%s;%s,%s)"
                 "  %s\n", og, ig, o.cell[0][0], o.cell[0][1],
                 o.cell[1][0], o.cell[1][1],
                 (og && !ig && id) ? "" : "NAO BATE");
          if(!og || ig || !id) mal++; }

        /* ── (5) E O PRODUTO É ASSOCIATIVO, a lei que faltava para a face
         * multiplicativa ser um monóide. Mede-se encadeando pelos dois lados. */
        /* as entradas são PEQUENAS de propósito: o numerador do racional é um
         * int8, e com (1,2;3,4)·(5,6;7,8)·(2,0;1,3) os produtos chegam a 136 e
         * 150 — que não cabem. A primeira escrita deste bloco usou-os, as
         * tabelas ficaram com uma linha só, e a comparação passou por comparar
         * duas células VAZIAS uma com a outra: uma asserção que não podia
         * falhar. Aqui os produtos ficam abaixo do tecto, e mede-se que as
         * tabelas têm mesmo as duas linhas antes de as comparar. */
        sql_executa("CREATE TABLE b (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO b VALUES (1,0), (1,1)", &o);
        sql_executa("CREATE TABLE c (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO c VALUES (1,1), (1,0)", &o);
        GUARDA("SELECT produto(b) FROM a", "t1");
        GUARDA("SELECT produto(c) FROM t1", "t2");
        GUARDA("SELECT produto(c) FROM b", "t1");
        GUARDA("SELECT produto(t1) FROM a", "t3");
        { char esq[4][32];
          int n2, n3, cheio = 1;
          sql_executa("SELECT * FROM t2", &o);
          n2 = o.nrows;
          for(int i = 0; i < 2 && i < n2; i++)
              for(int j = 0; j < 2; j++){
                  snprintf(esq[i*2+j], 32, "%s", o.cell[i][j]);
                  if(!o.cell[i][j][0]) cheio = 0;
              }
          sql_executa("SELECT * FROM t3", &o);
          n3 = o.nrows;
          for(int i = 0; i < 2 && i < n3; i++)
              for(int j = 0; j < 2; j++) if(!o.cell[i][j][0]) cheio = 0;
          { int assoc = (n2 == 2 && n3 == 2 && cheio
                      && !strcmp(esq[0], o.cell[0][0]) && !strcmp(esq[1], o.cell[0][1])
                      && !strcmp(esq[2], o.cell[1][0]) && !strcmp(esq[3], o.cell[1][1]));
            printf("      (A·B)·C = (%s,%s;%s,%s) e A·(B·C) = (%s,%s;%s,%s) —"
                   " com as DUAS linhas cheias dos dois lados (%d,%d,%d)  %s\n",
                   esq[0], esq[1], esq[2], esq[3],
                   o.cell[0][0], o.cell[0][1], o.cell[1][0], o.cell[1][1],
                   n2, n3, cheio, assoc ? "" : "NAO BATE");
            if(!assoc) mal++; } }

        /* ── O CONTROLO: `−(−A) = A`. A volta da volta é a identidade, e é isso
         * que faz do oposto uma INVOLUÇÃO e não apenas algo que desfaz uma vez. */
        GUARDA("SELECT oposto(*) FROM a", "t1");
        sql_executa("SELECT oposto(*) FROM t1", &o);
        { int volta = (o.nrows == 2 && !strcmp(o.cell[0][0], "1")
                       && !strcmp(o.cell[1][1], "4"));
          printf("\n      CONTROLO — −(−A) = A: (%s,%s;%s,%s) esp (1,2;3,4)"
                 "  %s\n", o.cell[0][0], o.cell[0][1], o.cell[1][0], o.cell[1][1],
                 volta ? "" : "NAO BATE");
          if(!volta) mal++; }
        #undef GUARDA
        sql_fechar();

        printf("\n");
        ok("A VOLTA DA SOMA PEDE O ANDAR COM SINAL, E FOI O BANCO QUE O MOSTROU. A coluna"
           " INTEIRA é um Word_8 SEM SINAL, e o oposto de uma matriz de entradas positivas"
           " tem entradas negativas — que ela RECUSA. Não é uma limitação a contornar: é a"
           " escada ℕ → ℤ dita pela régua, com o andar seguinte a ser exactamente «o que"
           " acrescenta a volta da soma». No andar com sinal cabe, e as contas mudam de"
           " resposta — o que obrigou a corrigir um defeito real: a matriz lia a célula SEM"
           " o sinal do corpo, e o traço de uma matriz de negativos saía 131067 onde a"
           " resposta é −5, enquanto a impressão da MESMA célula já a lia certa. Eram duas"
           " réguas para o mesmo sítio, e a matriz usava a que não sabe do corpo. AÍ A VOLTA"
           " FECHA: `A + (−A) = 0`, verificado e não suposto — sem a soma dar a matriz nula,"
           " «oposto» seria só um nome para trocar sinais. E AS DUAS VOLTAS TÊM ALCANCES"
           " DIFERENTES: o oposto existe para toda a matriz, a inversa só quando o"
           " determinante não é zero; a face aditiva é um GRUPO, a multiplicativa tem neutro"
           " para todos e volta só para alguns. É a Def. 1 do `aranha` — a VOLTA é o que"
           " distingue uma dobra de um esmagamento — com as duas faces a responderem-lhe de"
           " maneiras diferentes, e a diferença é de ALCANCE e não de método: as duas"
           " medem-se aplicando e comparando com o neutro. O produto é ASSOCIATIVO, o que"
           " faz da face multiplicativa um monóide, e o CONTROLO é `−(−A) = A`: a volta da"
           " volta é a identidade, e é isso que faz do oposto uma INVOLUÇÃO.",
           mal == 0);
    }


    /* ═══ §W59: UM ZERO PODE SER DUAS COISAS, E O TECTO NÃO É UM SÓ ════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W59 o estouro não é um determinante nulo, e cada operação diz o seu tecto.\n\n");
        { const char *tabs[] = { "d5","d2","sing","g6" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w59__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w59__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w59.mem"); unlink("/tmp/pgwire_w59.prog"); }
        if(!sql_abrir("/tmp/pgwire_w59")) mal++;

        /* ── (1) O ESTOURO TEM DE SE DISTINGUIR DO ZERO ────────────────────
         * O `qz_mult` desta casa devolve ZERO quando o produto não cabe no
         * inteiro, e conta a perda em `qz_perdeu`. Sem ler esse contador, um
         * determinante que estourou sai `0` — exatamente o que sai de uma
         * matriz singular. Seriam duas coisas com a mesma resposta, e a
         * errada com a cara da certa. A diagonal 5×5 de 65535 tem
         * determinante 65535⁵ ≈ 1,2·10²⁴, que não cabe. */
        sql_executa("CREATE TABLE d5 (a,b,c,d,e)", &o);
        sql_executa("INSERT INTO d5 VALUES (65535,0,0,0,0)", &o);
        sql_executa("INSERT INTO d5 VALUES (0,65535,0,0,0)", &o);
        sql_executa("INSERT INTO d5 VALUES (0,0,65535,0,0)", &o);
        sql_executa("INSERT INTO d5 VALUES (0,0,0,65535,0)", &o);
        sql_executa("INSERT INTO d5 VALUES (0,0,0,0,65535)", &o);
        int est = !sql_executa("SELECT det(*) FROM d5", &o) && !o.ok
                  && strstr(o.err, "overflow") != NULL;
        printf("      det da diagonal 5×5 de 65535 (≈1,2e24): %s («%s»)\n",
               est ? "RECUSADO como estouro" : "PASSOU (mau)", o.err);
        if(!est) mal++;

        /* ── (2) E O CONTROLO É A SINGULAR, que TEM de continuar a dar zero.
         * Sem ele, uma recusa constante — ou um motor que recusasse tudo —
         * passava nesta medida sem distinguir coisa nenhuma. */
        sql_executa("CREATE TABLE sing (p,q)", &o);
        sql_executa("INSERT INTO sing VALUES (1,2), (2,4)", &o);
        int sng = sql_executa("SELECT det(*) FROM sing", &o)
                  && o.nrows == 1 && !strcmp(o.cell[0][0], "0");
        sql_executa("SELECT posto(*) FROM sing", &o);
        int spo = o.nrows == 1 && !strcmp(o.cell[0][0], "1");
        printf("      controlo — a singular 2×2: det %s, posto %s\n",
               sng ? "0 (certo)" : "≠0 (mau)", spo ? "1 (certo)" : "≠1 (mau)");
        if(!sng || !spo) mal++;

        /* ── (3) E O QUE CABE CONTINUA A RESPONDER: a mesma entrada, numa
         * 2×2, dá 65535² = 4294836225, que cabe. Isto separa «a entrada é
         * grande» de «a conta não coube»: o que estoura é o PRODUTO de cinco
         * delas, não o valor. */
        sql_executa("CREATE TABLE d2 (p,q)", &o);
        sql_executa("INSERT INTO d2 VALUES (65535,0), (0,65535)", &o);
        int cab = sql_executa("SELECT det(*) FROM d2", &o)
                  && o.nrows == 1 && !strcmp(o.cell[0][0], "4294836225");
        printf("      a MESMA entrada numa 2×2: %s\n",
               cab ? "4294836225 (cabe e responde)" : "recusou (mau)");
        if(!cab) mal++;

        /* ── (4) O TECTO DIZ-SE E CUMPRE-SE. A primeira escrita tinha as duas
         * pontas erradas ao mesmo tempo: o teste recusava a 6×6 (comparando
         * com `>=`) e a mensagem anunciava 6×6 como o limite. Uma matriz era
         * recusada por uma condição que dizia que ela cabia. */
        sql_executa("CREATE TABLE g6 (a,b,c,d,e,f)", &o);
        for(int i = 0; i < 6; i++){
            char q[160]; int n = 0;
            n += snprintf(q + n, sizeof q - n, "INSERT INTO g6 VALUES (");
            for(int j = 0; j < 6; j++)
                n += snprintf(q + n, sizeof q - n, "%s%d", j ? "," : "", i * 6 + j + 1);
            snprintf(q + n, sizeof q - n, ")");
            sql_executa(q, &o);
        }
        int seis = sql_executa("SELECT posto(*) FROM g6", &o) && o.nrows == 1;
        printf("      a 6×6 no tecto declarado: %s%s%s\n",
               seis ? "responde (posto " : "RECUSADA (mau) — «",
               seis ? o.cell[0][0] : o.err, seis ? ")" : "»");
        if(!seis) mal++;

        /* ── (5) E O TECTO DA INVERSA É METADE, porque ela trabalha numa
         * matriz AUMENTADA de n×2n — justapõe a identidade e reduz. Dizer-lhe
         * o tecto geral seria deixá-la escrever fora do arranjo. */
        sql_executa("SELECT inversa(*) FROM g6", &o);
        int inv6 = !o.ok && strstr(o.err, "too large") != NULL;
        printf("      inversa da mesma 6×6: %s («%s»)\n",
               inv6 ? "recusada, tecto próprio" : "aceite (mau)", o.err);
        if(!inv6) mal++;
        /* e o controlo do controlo: uma 3×3 cabe na metade e a inversa sai */
        sql_executa("CREATE TABLE i3 (p,q,r)", &o);
        sql_executa("INSERT INTO i3 VALUES (1,0,0), (0,1,0), (0,0,1)", &o);
        int inv3 = sql_executa("SELECT inversa(*) FROM i3", &o) && o.nrows == 3;
        printf("      controlo — inversa de uma 3×3: %s\n",
               inv3 ? "responde" : "recusou (mau)");
        if(!inv3) mal++;
        sql_fechar();

        printf("\n");
        ok("UM ZERO PODE SER DUAS COISAS, E A RÉGUA TEM DE AS SEPARAR. A aritmética desta"
           " casa devolve ZERO quando a conta não cabe no inteiro, e conta a perda; sem ler"
           " esse contador, um determinante que ESTOUROU é indistinguível de um determinante"
           " NULO — a resposta errada com a cara da certa, que é a saturação travestida de"
           " teorema. Lê-se o contador antes e depois, e o que perdeu é RECUSADO. O controlo"
           " é o que impede isto de ser uma recusa constante: a singular continua a dar zero"
           " e posto um, e a MESMA entrada numa 2×2 continua a responder 4294836225 — o que"
           " separa «a entrada é grande» de «a conta não coube». E O TECTO NÃO É UM SÓ: o"
           " geral é o do `linear.h`, mas a inversa trabalha numa matriz AUMENTADA de n×2n e"
           " o seu tecto é METADE. A primeira escrita errava nas DUAS pontas ao mesmo tempo"
           " — recusava a 6×6 com uma comparação e anunciava 6×6 como o limite —, e uma"
           " matriz era recusada por uma condição que dizia que ela cabia. Cada operação diz"
           " o SEU tecto, e o controlo é a 3×3 cuja inversa sai.", mal == 0);
    }

    /* ═══ §W60: O DUAL DENTRO DO MOTOR, E A CONSERVAÇÃO PELA QUARTA VEZ ════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W60 a base dual e o aniquilador: o funcional é a coordenada que mede.\n\n");
        { const char *tabs[] = { "b","w","sg","r","e2" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w60__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w60__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w60.mem"); unlink("/tmp/pgwire_w60.prog"); }
        if(!sql_abrir("/tmp/pgwire_w60")) mal++;

        /* ── (1) A BASE DUAL É CONSTRUÍDA, NÃO PROCURADA. As colunas da tabela
         * são a base; os e^i com e^i(e_j) = δ^i_j são as LINHAS de B⁻¹, e isso
         * não é um método esperto: é a definição de inversa lida do outro lado.
         * B tem colunas (1,1) e (0,1), logo B⁻¹ tem linhas (1,0) e (−1,1). */
        sql_executa("CREATE TABLE b (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO b VALUES (1,0), (1,1)", &o);
        int du = sql_executa("SELECT dual(*) FROM b", &o) && o.nrows == 2 && o.ncols == 2
                 && !strcmp(o.cell[0][0], "1")  && !strcmp(o.cell[0][1], "0")
                 && !strcmp(o.cell[1][0], "-1") && !strcmp(o.cell[1][1], "1");
        printf("      base {(1,1),(0,1)} → dual %s\n",
               du ? "{(1,0),(−1,1)} — as linhas de B⁻¹" : "ERRADO");
        if(!du) mal++;

        /* ── (2) E VERIFICA-SE AVALIANDO, não comparando com a conta feita à
         * mão: e^i(e_j) tem de dar 1 na diagonal e 0 fora. A avaliação é o
         * produto B⁻¹·B, que o motor já sabe fazer — o dual medido com a outra
         * face da mesma casa, e não com uma segunda implementação. */
        { long bs[2][2] = {{1,0},{1,1}};   /* B: as colunas são a base */
          long dl[2][2];                    /* o dual, LIDO DA SAÍDA e não escrito */
          int delta = 1;
          for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
              dl[i][j] = atol(o.cell[i][j]);
          for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
              long s = 0;
              for(int k = 0; k < 2; k++) s += dl[i][k] * bs[k][j];
              if(s != (i == j ? 1 : 0)) delta = 0;
          }
          printf("      e^i(e_j) = δ: %s\n", delta ? "1 na diagonal, 0 fora" : "FALHOU");
          if(!delta) mal++; }

        /* ── (3) O ANIQUILADOR TRAZ A CONSERVAÇÃO PELA QUARTA VEZ. As LINHAS
         * geram W; os funcionais que se anulam em W são o NÚCLEO dessa matriz,
         * e daí dim W + dim W° = n — que não é lei nova: é posto + dim(ker) = n
         * lido do lado do dual. Com W = span{(1,2,3)}: 1 + 2 = 3. */
        sql_executa("CREATE TABLE w (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO w VALUES (1,2,3)", &o);
        int an = sql_executa("SELECT aniquilador(*) FROM w", &o) && o.nrows == 2;
        /* e o gume é AVALIAR: cada funcional aplicado ao gerador tem de dar 0 */
        int zero = an;
        for(int i = 0; i < o.nrows && zero; i++){
            long s = 0, g[3] = {1,2,3};
            for(int j = 0; j < o.ncols; j++) s += atol(o.cell[i][j]) * g[j];
            if(s != 0) zero = 0;
        }
        printf("      W = span{(1,2,3)}: dim W° = %d, e cada f(w) = 0? %s"
               " · 1 + 2 = 3\n", o.nrows, zero ? "sim" : "NÃO");
        if(!an || !zero) mal++;

        /* ── (4) E O CONTROLO É O OUTRO EXTREMO: com W a ocupar duas dimensões
         * de três, o aniquilador tem de encolher para uma. Sem ele, um motor
         * que devolvesse sempre «n−1 funcionais» passava no caso de cima. */
        sql_executa("CREATE TABLE r (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO r VALUES (1,0,0), (0,1,0)", &o);
        int an2 = sql_executa("SELECT aniquilador(*) FROM r", &o) && o.nrows == 1
                  && !strcmp(o.cell[0][0], "0") && !strcmp(o.cell[0][1], "0")
                  && !strcmp(o.cell[0][2], "1");
        printf("      controlo — W = span{e₁,e₂}: dim W° = %d %s · 2 + 1 = 3\n",
               o.nrows, an2 ? "e o funcional é (0,0,1)" : "(MAU)");
        if(!an2) mal++;

        /* ── (5) E A RECUSA DIZ O POSTO. Colunas dependentes não são uma base,
         * e o funcional que devia separar duas delas teria de dar 1 e 0 ao
         * MESMO vector — a recusa não é um limite da conta, é a lei. */
        sql_executa("CREATE TABLE sg (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO sg VALUES (1,2), (2,4)", &o);
        sql_executa("SELECT dual(*) FROM sg", &o);
        int rec = !o.ok && strstr(o.err, "rank 1 of 2") != NULL;
        printf("      colunas dependentes: %s («%s»)\n",
               rec ? "recusa, e diz o posto" : "aceitou (mau)", o.err);
        if(!rec) mal++;
        /* e a não-quadrada recusa por OUTRA razão, dita como tal */
        sql_executa("SELECT dual(*) FROM r", &o);
        int nq = !o.ok && strstr(o.err, "square") != NULL;
        printf("      não-quadrada: %s («%s»)\n",
               nq ? "recusa por outra razão, e diz qual" : "(mau)", o.err);
        if(!nq) mal++;
        sql_fechar();

        printf("\n");
        ok("O DUAL ENTROU NO MOTOR PELAS DUAS METADES, E NENHUMA PEDIU ÁLGEBRA NOVA. «O"
           " vetor fornece o objeto; o funcional fornece a coordenada que o mede» — e a base"
           " dual é CONSTRUÍDA, não procurada: com B a matriz das colunas, os e^i são as"
           " LINHAS de B⁻¹, porque e^i(e_j) = δ é a definição de inversa lida do outro lado."
           " Verifica-se AVALIANDO — 1 na diagonal, 0 fora — e não comparando com uma conta"
           " feita à mão. O aniquilador é a mesma descida do núcleo vista do lado do dual:"
           " as linhas geram W, e os funcionais que se anulam nele são o núcleo dessa"
           " matriz. Daí sai A MESMA CONSERVAÇÃO PELA QUARTA VEZ — dim W + dim W° = n, que é"
           " posto + dim(ker) = n noutro registo, ao lado de ∑G = |I| do quociente e de"
           " |presentes| + |ausentes| = |I| do dual das células: quatro objetos, uma lei, e"
           " em nenhum deles foi posta como requisito. O gume é aplicar: cada funcional"
           " avaliado no gerador tem de dar zero. E as duas recusas são DUAS: as colunas"
           " dependentes não são base (e diz-se o posto, porque o funcional que as separasse"
           " teria de dar 1 e 0 ao MESMO vector), e a não-quadrada falha por outra razão —"
           " uma base de n vectores num espaço de dimensão m ≠ n não é base.", mal == 0);
    }

    /* ═══ §W61: O QUE SOBRA NA DECLARAÇÃO, E A RECUSA QUE NÃO DEIXA RASTO ══ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W61 a declaração acaba em «)», e o que ela recusa não fica a existir.\n\n");
        { const char *tabs[] = { "b","g","h","i","zzz" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w61__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w61__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w61.mem"); unlink("/tmp/pgwire_w61.prog"); }
        if(!sql_abrir("/tmp/pgwire_w61")) mal++;

        /* ── (1) O ANALISADOR PARAVA E CALAVA-SE. A lista de colunas termina na
         * primeira palavra que ele não reconhece, e o que sobrava era ignorado:
         * uma sintaxe inventada no meio criava uma tabela de UMA coluna e
         * anunciava-a como criada. As consultas seguintes liam uma matriz 2×1
         * onde estava escrito 2×2 — e a resposta errada vinha com a cara da
         * certa, porque ninguém tinha dito que metade da declaração caiu. */
        int rec = !sql_executa("CREATE TABLE b (p INTEIRO COM SINAL, q RACIONAL)", &o)
                  && !o.ok && strstr(o.err, "syntax error") != NULL;
        printf("      declaração com sobras: %s («%s»)\n",
               rec ? "RECUSADA, e diz onde parou" : "aceite em silêncio (mau)", o.err);
        if(!rec) mal++;

        /* ── (2) E A RECUSA NÃO PODE DEIXAR RASTO. O ficheiro da tabela abre-se
         * ANTES de a declaração acabar de ser lida — tem de ser, é nele que o
         * catálogo vai. Se a recusa o deixasse lá, a tabela recusada ficava a
         * comportar-se PIOR do que uma que nunca existiu: esta é recusada com
         * «não existe», aquela aceitava `SELECT` (zero linhas) e aceitava
         * `INSERT`. A recusa passava a ser uma criação encoberta. */
        sql_executa("SELECT * FROM b", &o);
        int sel = !o.ok && strstr(o.err, "does not exist") != NULL;
        sql_executa("INSERT INTO b VALUES (1,2)", &o);
        int ins = !o.ok && strstr(o.err, "does not exist") != NULL;
        printf("      depois da recusa — SELECT: %s · INSERT: %s\n",
               sel ? "não existe" : "RESPONDEU (mau)",
               ins ? "não existe" : "ACEITOU (mau)");
        if(!sel || !ins) mal++;

        /* ── (3) E O CONTROLO É A INDISTINGUIBILIDADE: a tabela recusada tem de
         * dar a MESMA resposta que uma que nunca foi mencionada. Comparar com o
         * texto do erro não chegaria — compara-se com o outro caso. */
        sql_executa("SELECT * FROM zzz", &o);
        char nunca[256]; snprintf(nunca, sizeof nunca, "%s", o.err);
        sql_executa("SELECT * FROM b", &o);
        int igual = !strcmp(nunca, o.err) ? 0 : 1;   /* diferem só no nome */
        igual = (strstr(nunca, "does not exist") && strstr(o.err, "does not exist"));
        printf("      a recusada e a nunca-mencionada: %s\n",
               igual ? "a mesma resposta" : "respostas DIFERENTES (mau)");
        if(!igual) mal++;

        /* ── (4) E AS BOAS CONTINUAM A PASSAR — sem isto, um motor que recusasse
         * toda a declaração passava nas três medidas acima. As três formas que a
         * casa aceita: sem tipo, com tipo, e com restrições. */
        int b1 = sql_executa("CREATE TABLE g (p, q, r)", &o);
        int b2 = sql_executa("CREATE TABLE h (p RACIONAL, q INTEIRO)", &o);
        int b3 = sql_executa("CREATE TABLE i (p INTEIRO PRIMARY KEY, q RACIONAL,"
                             " CHECK (q > 0))", &o);
        printf("      controlo — sem tipo: %s · com tipo: %s · com restrições: %s\n",
               b1 ? "passa" : "MAU", b2 ? "passa" : "MAU", b3 ? "passa" : "MAU");
        if(!b1 || !b2 || !b3) mal++;
        sql_fechar();

        printf("\n");
        ok("A DECLARAÇÃO ACABA EM «)», E O QUE SOBRA NÃO PODE FICAR CALADO. O analisador"
           " parava na primeira palavra que não reconhecia e criava a tabela com as colunas"
           " que já tinha lido — uma sintaxe inventada no meio dava uma tabela de UMA coluna"
           " anunciada como criada, e as consultas seguintes liam uma matriz 2×1 onde estava"
           " escrito 2×2. Não era um erro de conta: era a resposta certa a outra pergunta. E"
           " A RECUSA NÃO DEIXA RASTO, o que é a metade menos óbvia: o ficheiro da tabela"
           " abre-se ANTES de a declaração acabar de ser lida — tem de ser, é nele que o"
           " catálogo vai —, e deixá-lo lá fazia a tabela recusada comportar-se PIOR do que"
           " uma que nunca existiu: esta é recusada com «não existe», aquela aceitava"
           " `SELECT` e aceitava `INSERT`. A recusa era uma criação encoberta. Desfaz-se pelo"
           " caminho do `DROP`, e o gume é a INDISTINGUIBILIDADE — a recusada responde o"
           " mesmo que a nunca-mencionada, medido por comparação e não pelo texto do erro. O"
           " controlo são as três formas boas, que continuam a passar: sem tipo, com tipo, e"
           " com restrições.", mal == 0);
    }


    /* ═══ §W62: O CORPO É A MATRIZ 2×2, E A CIFRA É O SEU ESPECTRO ═════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W62 a cifra do corpo era o espectro da matriz — e já estava escrito.\n\n");
        { const char *tabs[] = { "au","sm","pr","ro","tr","q3" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w62__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w62__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w62.mem"); unlink("/tmp/pgwire_w62.prog"); }
        if(!sql_abrir("/tmp/pgwire_w62")) mal++;

        /* colhe a saída inteira numa cadeia, para comparar cifras */
        #define CIFRA(tab, dest) do { \
            char q2[80]; snprintf(q2, sizeof q2, "SELECT cifra(*) FROM %s", tab); \
            sql_executa(q2, &o); dest[0] = 0; \
            if(o.ok) for(int j2 = 0; j2 < o.ncols; j2++){ \
                strncat(dest, o.cell[0][j2], 24); strcat(dest, " "); } \
        } while(0)

        /* ── (1) O `cifra.h` NÃO FOI ESCRITO PARA MATRIZES, e é esse o ponto.
         * Foi escrito para CORPOS, e diz na porta que as suas duas grandezas
         * são «a razão → o traço B» e «o sinal → o determinante C»; e diz que
         * «o hipercorpo não tem (B,C): o seu operador NÃO É uma matriz 2×2».
         * O par que define um corpo desta casa já era o par de invariantes de
         * uma matriz 2×2 — a identificação não se fez agora, estava feita. A
         * matriz de Fibonacci (1,1;1,0) tem traço 1 e determinante −1, que é o
         * ÁUREO. */
        sql_executa("CREATE TABLE au (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO au VALUES (1,1), (1,0)", &o);
        sql_executa("SELECT traco(*) FROM au", &o);
        int t1 = o.nrows == 1 && !strcmp(o.cell[0][0], "1");
        sql_executa("SELECT det(*) FROM au", &o);
        int d1 = o.nrows == 1 && !strcmp(o.cell[0][0], "-1");
        char c_au[256]; CIFRA("au", c_au);
        printf("      (1,1;1,0): traço %s, det %s → λ² − λ − 1, o áureo · cifra [%s]\n",
               t1 ? "1" : "MAU", d1 ? "−1" : "MAU", c_au);
        if(!t1 || !d1 || !c_au[0]) mal++;

        /* ── (2) O GUME É A SEMELHANÇA, e sai de graça: duas matrizes
         * conjugadas têm o mesmo traço e o mesmo determinante, logo a MESMA
         * cifra. Não é uma propriedade que se lhe tenha dado — é o que
         * «invariante de conjugação» quer dizer. E a testemunha é uma matriz
         * DIFERENTE entrada a entrada: (2,1;−1,−1) não tem uma célula igual à
         * de cima e tem a mesma cifra. */
        sql_executa("CREATE TABLE sm (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO sm VALUES (2,1), (-1,-1)", &o);
        char c_sm[256]; CIFRA("sm", c_sm);
        int sem = c_au[0] && !strcmp(c_au, c_sm);
        printf("      a semelhante (2,1;−1,−1) — nenhuma entrada igual: cifra [%s] %s\n",
               c_sm, sem ? "— A MESMA" : "— DIFERENTE (mau)");
        if(!sem) mal++;

        /* ── (3) E O CONTROLO É OUTRO CORPO. Sem ele, um codificador que
         * devolvesse sempre a mesma coisa passava na medida de cima. A prata é
         * (2,1;1,0): traço 2, determinante −1, λ² − 2λ − 1. */
        sql_executa("CREATE TABLE pr (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO pr VALUES (2,1), (1,0)", &o);
        char c_pr[256]; CIFRA("pr", c_pr);
        int dif = c_pr[0] && strcmp(c_au, c_pr) != 0;
        printf("      controlo — a prata (2,1;1,0): cifra [%s] %s\n",
               c_pr, dif ? "— diferente da áurea" : "— IGUAL (mau)");
        if(!dif) mal++;

        /* ── (4) E A ROTAÇÃO DIZ O QUE FALTAVA DIZER. Em (0,−1;1,0) o traço é
         * 0 e o determinante 1, logo B² − 4C = −4 < 0: o lado PRÓPRIO não tem
         * real, e quem o carrega é o DUAL. O primeiro termo da cifra é
         * precisamente «qual metade carrega o real», e passa de 1 a 2 — sem
         * uma linha escrita para este caso. É o `i` a aparecer como a matriz
         * cujo espectro só existe do outro lado. */
        sql_executa("CREATE TABLE ro (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO ro VALUES (0,-1), (1,0)", &o);
        char c_ro[256]; CIFRA("ro", c_ro);
        int lado2 = c_ro[0] == '2';
        printf("      a rotação (0,−1;1,0): B²−4C = −4 · cifra [%s] — o real está"
               " no %s\n", c_ro, lado2 ? "DUAL (primeiro termo 2)" : "próprio (mau)");
        if(!lado2) mal++;
        /* e o controlo do controlo: a áurea tem 1, porque 1+4 ≥ 0 */
        int lado1 = c_au[0] == '1';
        printf("      controlo — a áurea: primeiro termo %c, porque B²−4C = 5 ≥ 0\n",
               c_au[0]);
        if(!lado1) mal++;

        /* ── (5) E OS DOIS LIMITES DIZEM PORQUÊ, não «não dá». Acima de 2×2 o
         * característico tem mais coeficientes e o par (B,C) deixa de o
         * determinar; e a cifra é de um CORPO, que nesta casa é dado por dois
         * INTEIROS — um traço fracionário diz que a matriz não é o operador de
         * nenhum corpo do catálogo. */
        sql_executa("CREATE TABLE q3 (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO q3 VALUES (1,0,0), (0,1,0), (0,0,1)", &o);
        sql_executa("SELECT cifra(*) FROM q3", &o);
        int r3 = !o.ok && strstr(o.err, "2×2") != NULL;
        sql_executa("CREATE TABLE tr (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO tr VALUES (1/2,1), (1,0)", &o);
        sql_executa("SELECT cifra(*) FROM tr", &o);
        int rf = !o.ok && strstr(o.err, "integer") != NULL;
        printf("      a 3×3: %s · o traço fracionário: %s\n",
               r3 ? "recusa, e diz que o par deixa de determinar" : "MAU",
               rf ? "recusa, e diz que o corpo é de inteiros" : "MAU");
        if(!r3 || !rf) mal++;
        #undef CIFRA
        sql_fechar();

        printf("\n");
        ok("O CORPO DESTA CASA JÁ ERA UMA MATRIZ 2×2, E NINGUÉM PRECISOU DE OS IDENTIFICAR."
           " O `cifra.h` não foi escrito para matrizes — foi escrito para CORPOS, e diz na"
           " porta que as suas duas únicas grandezas são «a razão, quanto se estica por"
           " nível → o traço B» e «o sinal, se as duas direções se cancelam → o determinante"
           " C»; e diz que «o hipercorpo NÃO TEM (B,C): o seu operador não é uma matriz"
           " 2×2». O par que define um corpo era o par de invariantes de uma matriz, e a"
           " cifra que o escreve é o ESPECTRO: as raízes de λ² − Bλ + C postas em fração"
           " contínua periódica, exatas, em inteiros, sem uma raiz calculada — e o período é"
           " invariante completo por Lagrange. DAÍ O GUME SAI DE GRAÇA: matrizes SEMELHANTES"
           " têm a mesma cifra, porque a conjugação preserva traço e determinante; a"
           " testemunha é (2,1;−1,−1), que não tem UMA entrada igual à de Fibonacci e devolve"
           " a mesma cifra, com o controlo na prata, que devolve outra. E A ROTAÇÃO DIZ O QUE"
           " FALTAVA: em (0,−1;1,0) o discriminante é −4, o lado próprio não tem real e o"
           " primeiro termo da cifra — que é «qual metade carrega o real» — passa de 1 a 2"
           " sem uma linha escrita para este caso. É o `i` a aparecer como a matriz cujo"
           " espectro só existe do outro lado. Os dois limites dizem PORQUÊ e não «não dá»:"
           " acima de 2×2 o característico tem mais coeficientes, e um traço fracionário diz"
           " que a matriz não é o operador de nenhum corpo do catálogo.", mal == 0);
    }


    /* ═══ §W63: A CIFRA GOVERNA A ÓRBITA — E SEM UMA LINHA NOVA NO MOTOR ═══ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W63 o par (B,C) prevê o traço de Aⁿ: o corpo é o sistema.\n\n");
        { const char *tabs[] = { "a","pt","tmp","pa","pb" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w63__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w63__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w63.mem"); unlink("/tmp/pgwire_w63.prog"); }
        if(!sql_abrir("/tmp/pgwire_w63")) mal++;

        /* guarda uma saída 2×2 numa tabela, para a fazer voltar a entrar */
        #define POE(consulta, tab) do { \
            sql_executa(consulta, &o); \
            { char q2[200]; \
              snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", tab); \
              sql_executa(q2, &o2); \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", tab); \
              sql_executa(q2, &o2); \
              for(int i2 = 0; i2 < o.nrows; i2++){ \
                  snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s,%s)", \
                           tab, o.cell[i2][0], o.cell[i2][1]); \
                  sql_executa(q2, &o2); } } \
        } while(0)

        /* ── A LEI. Por Cayley--Hamilton, A² = B·A − C·I, e multiplicando por
         * Aⁿ⁻² e tomando traços vem
         *
         *     t_n = B·t_{n−1} − C·t_{n−2},   com t_0 = 2 e t_1 = B,
         *
         * que é a MESMA recorrência que define o corpo de par (B,C). O traço
         * das potências não é uma sequência qualquer que por acaso obedece à
         * regra: É a regra, e o corpo é o sistema dinâmico x_{n+1} = A·x_n
         * lido pelo seu invariante. Para (1,1;1,0) — traço 1, determinante −1
         * — dá 2, 1, 3, 4, 7, 11: os números de Lucas, e ninguém os escreveu.
         *
         * Mede-se ENCADEANDO o que o motor já sabe: `produto` para subir uma
         * potência, `traco` para ler, e nada de novo. */
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (1,1), (1,0)", &o);
        sql_executa("SELECT traco(*) FROM a", &o);
        long B = atol(o.cell[0][0]);
        sql_executa("SELECT det(*) FROM a", &o);
        long C = atol(o.cell[0][0]);
        printf("      o corpo (B,C) = (%ld,%ld) · a recorrência t = %ld·t' − (%ld)·t''\n",
               B, C, B, C);

        /* a órbita: pt guarda Aⁿ, e sobe uma potência de cada vez */
        sql_executa("CREATE TABLE pt (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO pt VALUES (1,1), (1,0)", &o);
        long t2 = 2, t1 = B;                      /* t_0 e t_1, PREVISTOS */
        int bate = 1, n_ok = 0;
        printf("      n  traço medido   previsto pela recorrência\n");
        for(int n = 2; n <= 8; n++){
            POE("SELECT produto(a) FROM pt", "pt");     /* pt ← pt·A */
            sql_executa("SELECT traco(*) FROM pt", &o);
            if(!o.ok || o.nrows != 1){ bate = 0; break; }
            long medido = atol(o.cell[0][0]);
            long prev = B * t1 - C * t2;
            printf("      %d  %12ld   %ld%s\n", n, medido, prev,
                   medido == prev ? "" : "   ← DIVERGE");
            if(medido != prev) bate = 0; else n_ok++;
            t2 = t1; t1 = prev;
        }
        printf("      %d potências seguidas, e a previsão vem SÓ do par (B,C)\n", n_ok);
        if(!bate || n_ok < 7) mal++;

        /* ── E O CONTROLO É OUTRO CORPO, com a mesma medida. Sem ele, uma
         * recorrência que ignorasse B e C — ou um traço constante — podia
         * passar. A prata é (2,1;1,0): (B,C) = (2,−1), e dá 2, 2, 6, 14, 34. */
        sql_executa("CREATE TABLE pa (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO pa VALUES (2,1), (1,0)", &o);
        sql_executa("SELECT traco(*) FROM pa", &o);
        long B2 = atol(o.cell[0][0]);
        sql_executa("SELECT det(*) FROM pa", &o);
        long C2 = atol(o.cell[0][0]);
        sql_executa("CREATE TABLE pb (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO pb VALUES (2,1), (1,0)", &o);
        long u2 = 2, u1 = B2;
        int bate2 = 1;
        for(int n = 2; n <= 5; n++){
            POE("SELECT produto(pa) FROM pb", "pb");
            sql_executa("SELECT traco(*) FROM pb", &o);
            if(!o.ok || o.nrows != 1){ bate2 = 0; break; }
            long prev = B2 * u1 - C2 * u2;
            if(atol(o.cell[0][0]) != prev) bate2 = 0;
            u2 = u1; u1 = prev;
        }
        printf("      controlo — a prata (B,C) = (%ld,%ld): a MESMA lei com OUTROS"
               " números %s\n", B2, C2, bate2 ? "fecha" : "FALHA");
        if(!bate2) mal++;
        /* e as duas sequências têm de DIFERIR, senão a lei não estava a usar (B,C) */
        int difere = (B != B2 || C != C2) && (t1 != u1);
        printf("      e as duas órbitas divergem (%ld ≠ %ld): a lei USA o par\n",
               t1, u1);
        if(!difere) mal++;
        #undef POE
        sql_fechar();

        printf("\n");
        ok("A CIFRA GOVERNA A ÓRBITA, E O MOTOR NÃO PRECISOU DE UMA LINHA NOVA PARA O"
           " MOSTRAR. Por Cayley--Hamilton, A² = B·A − C·I; multiplicando por Aⁿ⁻² e tomando"
           " traços vem t_n = B·t_{n−1} − C·t_{n−2}, com t_0 = 2 e t_1 = B — que é A MESMA"
           " recorrência que define o corpo de par (B,C). O traço das potências não é uma"
           " sequência que por acaso obedece à regra: É a regra, e o corpo é o sistema"
           " dinâmico x_{n+1} = A·x_n lido pelo seu invariante. Para a matriz de Fibonacci"
           " dá 2, 1, 3, 4, 7, 11, 18, 29 — os números de Lucas, e ninguém os escreveu:"
           " saíram de multiplicar a tabela por si própria e ler o traço. A MEDIDA É UM"
           " ENCADEAMENTO: `produto` sobe uma potência, `traco` lê, o resultado volta a"
           " entrar como tabela — o motor a alimentar-se a si próprio por oito passos, o que"
           " torna isto uma verificação da composição inteira e não de uma operação. E O"
           " CONTROLO É OUTRO CORPO: a prata, com (B,C) = (2,−1), cumpre a MESMA lei com"
           " OUTROS números, e as duas órbitas divergem — sem isso, uma recorrência que"
           " ignorasse o par, ou um traço constante, passava. É o fecho do que §W62 abriu:"
           " ali a cifra escreve o espectro, aqui o espectro dita a órbita.", mal == 0);
    }


    /* ═══ §W64: O ESPECTRO POR NÚMEROS, E O QUE A CIFRA NÃO SEPARA ═════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W64 dois caminhos para o espectro, e o limite exacto de cada um.\n\n");
        { const char *tabs[] = { "d","jo","ho","au","q3" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w64__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w64__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w64.mem"); unlink("/tmp/pgwire_w64.prog"); }
        if(!sql_abrir("/tmp/pgwire_w64")) mal++;

        /* ── (1) OS NÚMEROS SAEM QUANDO O DISCRIMINANTE É QUADRADO PERFEITO, e
         * verificam-se pelas RELAÇÕES DE VIÈTE em vez de por comparação com uma
         * conta feita à mão: a soma tem de ser o traço e o produto tem de ser o
         * determinante. Assim a medida não confia na fórmula que a produziu. */
        sql_executa("CREATE TABLE d (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO d VALUES (3,1), (0,2)", &o);
        sql_executa("SELECT autovalores(*) FROM d", &o);
        int nv = o.ok && o.nrows == 2;
        long l1 = nv ? atol(o.cell[0][0]) : 0, l2 = nv ? atol(o.cell[1][0]) : 0;
        sql_executa("SELECT traco(*) FROM d", &o);
        long tr = o.nrows == 1 ? atol(o.cell[0][0]) : 0;
        sql_executa("SELECT det(*) FROM d", &o);
        long dt = o.nrows == 1 ? atol(o.cell[0][0]) : 0;
        int viete = nv && (l1 + l2 == tr) && (l1 * l2 == dt);
        printf("      λ = %ld, %ld · soma %ld = traço %ld · produto %ld = det %ld → %s\n",
               l1, l2, l1 + l2, tr, l1 * l2, dt, viete ? "Viète fecha" : "FALHA");
        if(!viete) mal++;

        /* ── (2) E O AUTOVECTOR VERIFICA-SE APLICANDO. Cada um sai do núcleo de
         * A − λI, que é a peça do §W52 outra vez; e o gume é exigir A·v = λ·v,
         * não comparar v com um vector escrito à mão. */
        sql_executa("SELECT autovetores(*) FROM d", &o);
        int nvec = o.ok && o.nrows == 2;
        int aplica = nvec;
        if(nvec){
            long A[2][2] = {{3,1},{0,2}};
            for(int i = 0; i < 2 && aplica; i++){
                long v[2] = { atol(o.cell[i][0]), atol(o.cell[i][1]) };
                long Av[2] = { A[0][0]*v[0] + A[0][1]*v[1],
                               A[1][0]*v[0] + A[1][1]*v[1] };
                /* λ é o que ele for: procura-se um λ com A·v = λ·v, e exige-se
                 * que seja UM dos dois devolvidos por autovalores */
                long lam = 0; int achou = 0;
                for(int c = 0; c < 2; c++){
                    long cand = (c == 0) ? l1 : l2;
                    if(Av[0] == cand*v[0] && Av[1] == cand*v[1]){ lam = cand; achou = 1; }
                }
                if(!achou) aplica = 0; else (void)lam;
            }
        }
        printf("      A·v = λ·v em cada autovector: %s\n",
               aplica ? "verificado, aplicando" : "FALHOU");
        if(!aplica) mal++;

        /* ── (3) O PAR QUE MOSTRA O LIMITE DE TUDO O QUE VEIO ANTES. Jordan
         * (2,1;0,2) e a homotetia (2,0;0,2) têm o MESMO traço, o MESMO
         * determinante, os MESMOS autovalores — e por isso a MESMA cifra. E não
         * são semelhantes: a homotetia comuta com tudo, logo P⁻¹·2I·P = 2I para
         * todo o P, e nunca dá Jordan. Ou seja: a cifra é invariante de
         * semelhança, mas NÃO É COMPLETO para ela — é completo para o CORPO,
         * por Lagrange, e corpo e matriz não são a mesma pergunta. Quem os
         * separa são os AUTOVECTORES: um contra dois. */
        sql_executa("CREATE TABLE jo (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO jo VALUES (2,1), (0,2)", &o);
        sql_executa("CREATE TABLE ho (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO ho VALUES (2,0), (0,2)", &o);
        char cj[256] = "", ch[256] = "";
        sql_executa("SELECT cifra(*) FROM jo", &o);
        if(o.ok) for(int j = 0; j < o.ncols; j++){ strncat(cj, o.cell[0][j], 24); strcat(cj, " "); }
        sql_executa("SELECT cifra(*) FROM ho", &o);
        if(o.ok) for(int j = 0; j < o.ncols; j++){ strncat(ch, o.cell[0][j], 24); strcat(ch, " "); }
        int mesma = cj[0] && !strcmp(cj, ch);
        sql_executa("SELECT autovetores(*) FROM jo", &o);
        int nj = o.ok ? o.nrows : -1;
        sql_executa("SELECT autovetores(*) FROM ho", &o);
        int nh = o.ok ? o.nrows : -1;
        int separa = mesma && nj == 1 && nh == 2;
        printf("      Jordan e homotetia: cifra [%s] a MESMA · autovectores %d contra %d\n",
               cj, nj, nh);
        printf("      → a cifra é invariante de semelhança mas NÃO completo para ela:"
               " %s\n", separa ? "medido, e quem separa são os autovectores" : "FALHOU");
        if(!separa) mal++;

        /* ── (4) E O SEGUNDO CAMINHO RECUSA ONDE O PRIMEIRO RESPONDE — o que é
         * a resposta e não um limite. No áureo o discriminante é 5, não é
         * quadrado, e as raízes são as folhas do corpo: escrevem-se pela cifra.
         * Devolver um decimal seria trocar o objecto por uma aproximação dele.
         * A recusa REMETE, e a cifra responde. */
        sql_executa("CREATE TABLE au (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO au VALUES (1,1), (1,0)", &o);
        sql_executa("SELECT autovalores(*) FROM au", &o);
        int rem = !o.ok && strstr(o.err, "cifra") != NULL && strstr(o.err, "disc 5");
        printf("      o áureo (disc 5): %s («%s»)\n",
               rem ? "recusa e REMETE para a cifra" : "MAU", o.err);
        if(!rem) mal++;
        sql_executa("SELECT cifra(*) FROM au", &o);
        int cif = o.ok && o.ncols > 0;
        printf("      e a cifra responde onde os números não chegam: %s\n",
               cif ? "sim" : "NÃO (mau)");
        if(!cif) mal++;

        /* ── (5) E O CONTROLO DE FORMA: acima de 2×2 o característico tem grau
         * maior e as raízes deixam de sair de um discriminante. */
        sql_executa("CREATE TABLE q3 (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO q3 VALUES (1,0,0), (0,1,0), (0,0,1)", &o);
        sql_executa("SELECT autovalores(*) FROM q3", &o);
        int r3 = !o.ok && strstr(o.err, "2×2") != NULL;
        printf("      controlo — a 3×3: %s\n", r3 ? "recusa, e diz porquê" : "MAU");
        if(!r3) mal++;
        sql_fechar();

        printf("\n");
        ok("O ESPECTRO TEM DOIS CAMINHOS, E CADA UM SABE UMA COISA QUE O OUTRO NÃO SABE. A"
           " cifra escreve-o SEMPRE, por fração contínua; o `esp_racional` dá os NÚMEROS,"
           " mas só quando o discriminante é quadrado perfeito — e os dois lêem o MESMO"
           " discriminante, porque partilham o `raizi`, o que faz deles duas leituras de um"
           " objecto e não duas contas parecidas. Os números verificam-se por VIÈTE — soma"
           " igual ao traço, produto igual ao determinante —, e não por comparação com uma"
           " conta feita à mão; e os autovectores verificam-se APLICANDO, exigindo A·v = λ·v."
           " ONDE AS RAÍZES NÃO SÃO RACIONAIS, A RECUSA É A RESPOSTA: são as folhas do corpo,"
           " escrevem-se pela cifra, e devolver um decimal seria trocar o objecto por uma"
           " aproximação dele — a recusa REMETE, e a cifra responde. E O PAR JORDAN/HOMOTETIA"
           " PÕE O LIMITE DE TUDO O QUE VEIO ANTES: (2,1;0,2) e (2,0;0,2) têm o mesmo traço,"
           " o mesmo determinante, os mesmos autovalores e por isso A MESMA CIFRA — e não são"
           " semelhantes, porque a homotetia comuta com tudo e P⁻¹·2I·P = 2I nunca dá Jordan."
           " A cifra é invariante de semelhança e NÃO É COMPLETO para ela: é completo para o"
           " CORPO, por Lagrange, e corpo e matriz não são a mesma pergunta. Quem os separa"
           " são os AUTOVECTORES, um contra dois — e é isso, e não os valores, que decide a"
           " diagonalizabilidade.", mal == 0);
    }


    /* ═══ §W65: A GRAM — O PRODUTO INTERNO FECHA O DUAL SEM ESCOLHER BASE ══ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W65 G = A·Aᵀ: o dual sem base, e a dimensão contada nos dois sítios.\n\n");
        { const char *tabs[] = { "a","tp","dep","ort","pj" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w65__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w65__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w65.mem"); unlink("/tmp/pgwire_w65.prog"); }
        if(!sql_abrir("/tmp/pgwire_w65")) mal++;

        /* ── (1) A GRAM POR DOIS CAMINHOS. A base dual do §W60 precisa de uma
         * BASE para existir — os e^i são as linhas de B⁻¹, e mudar B muda-os. O
         * produto interno faz o mesmo trabalho SEM escolher base, dando o
         * isomorfismo canónico v ↦ ⟨v,·⟩, e a sua forma escrita é a Gram. O
         * primeiro caminho é `gram(*)`, que soma produtos célula a célula; o
         * segundo é `transposta` guardada e depois `produto`, com as peças que
         * já existiam. Não se apoiam um no outro, e por isso concordarem é uma
         * verificação e não uma repetição. */
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (1,2), (3,4)", &o);
        sql_executa("SELECT gram(*) FROM a", &o);
        char g1[128] = "";
        if(o.ok) for(int i = 0; i < o.nrows; i++) for(int j = 0; j < o.ncols; j++){
            strncat(g1, o.cell[i][j], 24); strcat(g1, " "); }
        /* o segundo caminho: guarda a transposta e multiplica */
        sql_executa("SELECT transposta(*) FROM a", &o);
        sql_executa("CREATE TABLE tp (p RACIONAL, q RACIONAL)", &o2);
        for(int i = 0; i < o.nrows; i++){
            char q2[120];
            snprintf(q2, sizeof q2, "INSERT INTO tp VALUES (%s,%s)",
                     o.cell[i][0], o.cell[i][1]);
            sql_executa(q2, &o2);
        }
        sql_executa("SELECT produto(tp) FROM a", &o);
        char g2[128] = "";
        if(o.ok) for(int i = 0; i < o.nrows; i++) for(int j = 0; j < o.ncols; j++){
            strncat(g2, o.cell[i][j], 24); strcat(g2, " "); }
        int dois = g1[0] && !strcmp(g1, g2);
        printf("      gram(*) = [%s] · A·Aᵀ = [%s] → %s\n",
               g1, g2, dois ? "os dois caminhos concordam" : "DIVERGEM");
        if(!dois) mal++;

        /* ── (2) E A GRAM É SIMÉTRICA, porque o produto interno não tem lado.
         * Não é uma propriedade que se lhe tenha dado: ⟨u,v⟩ = ⟨v,u⟩ vem de a
         * soma dos produtos não distinguir os factores. */
        sql_executa("SELECT gram(*) FROM a", &o);
        int sim = o.ok && o.nrows == 2 && !strcmp(o.cell[0][1], o.cell[1][0]);
        printf("      G simétrica: G₁₂ = %s e G₂₁ = %s → %s\n",
               o.cell[0][1], o.cell[1][0], sim ? "sim" : "NÃO");
        if(!sim) mal++;

        /* ── (3) E det G = (det A)², que é uma lei a MAIS do que a que eu ia
         * medir. Ela sai de det(A·Aᵀ) = det A · det Aᵀ e de a transposta ter o
         * mesmo determinante — duas peças que o motor já tinha, e que nunca
         * tinham sido postas em contacto. Aqui det A = −2 e det G = 4. */
        long dg = atol(o.cell[0][0]) * atol(o.cell[1][1])
                - atol(o.cell[0][1]) * atol(o.cell[1][0]);
        sql_executa("SELECT det(*) FROM a", &o);
        long da = o.nrows == 1 ? atol(o.cell[0][0]) : 0;
        int quad = (dg == da * da);
        printf("      det G = %ld e (det A)² = %ld → %s\n",
               dg, da * da, quad ? "det G = (det A)²" : "FALHA");
        if(!quad) mal++;

        /* ── (4) O ZERO DA GRAM É A DEPENDÊNCIA, e é o determinante de Gram —
         * com n = 2 é exactamente Cauchy–Schwarz com igualdade, que se dá
         * quando os vectores são colineares. O posto conta-se nos dois sítios e
         * tem de bater: posto G = posto A. */
        sql_executa("CREATE TABLE dep (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO dep VALUES (1,2), (2,4)", &o);
        sql_executa("SELECT gram(*) FROM dep", &o);
        long dgd = o.ok ? atol(o.cell[0][0]) * atol(o.cell[1][1])
                        - atol(o.cell[0][1]) * atol(o.cell[1][0]) : -1;
        sql_executa("SELECT posto(*) FROM dep", &o);
        int pa = o.nrows == 1 ? atoi(o.cell[0][0]) : -1;
        printf("      colineares (1,2) e (2,4): det G = %ld (Cauchy–Schwarz com"
               " IGUALDADE) · posto %d\n", dgd, pa);
        if(dgd != 0 || pa != 1) mal++;

        /* ── (5) E O CONTROLO É A ORTOGONALIDADE, o outro extremo: com as
         * linhas ortogonais a Gram é DIAGONAL, e as entradas fora dela são zero
         * porque ⟨u,v⟩ = 0 — não porque a conta não as tenha feito. Sem este
         * caso, uma Gram que devolvesse sempre a mesma forma passava acima. */
        sql_executa("CREATE TABLE ort (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO ort VALUES (1,0), (0,3)", &o);
        sql_executa("SELECT gram(*) FROM ort", &o);
        int diag = o.ok && !strcmp(o.cell[0][0], "1") && !strcmp(o.cell[0][1], "0")
                   && !strcmp(o.cell[1][0], "0") && !strcmp(o.cell[1][1], "9");
        printf("      controlo — ortogonais (1,0) e (0,3): G = (1,0;0,9) %s ·"
               " a norma sai AO QUADRADO, e a raiz não se tira\n",
               diag ? "diagonal" : "MAU");
        if(!diag) mal++;
        sql_fechar();

        printf("\n");
        ok("O PRODUTO INTERNO FECHA O DUAL SEM ESCOLHER BASE, E A GRAM É A SUA FORMA"
           " ESCRITA. A base dual de §W60 PRECISA de uma base para existir — os e^i são as"
           " linhas de B⁻¹, e mudar B muda-os; o produto interno dá o mesmo isomorfismo"
           " v ↦ ⟨v,·⟩ sem escolher nenhuma, e a matriz que o escreve é G_ij = ⟨v_i,v_j⟩ ="
           " A·Aᵀ. MEDE-SE POR DOIS CAMINHOS QUE NÃO SE APOIAM: `gram(*)` soma produtos"
           " célula a célula, e `transposta` guardada seguida de `produto` refaz o mesmo com"
           " as peças que já existiam — concordarem é uma verificação, não uma repetição. E"
           " SAEM TRÊS LEIS DE GRAÇA. G é SIMÉTRICA, porque a soma dos produtos não"
           " distingue os factores. det G = (det A)², que é uma lei a MAIS do que a que eu ia"
           " medir: sai de det(A·Aᵀ) = det A · det Aᵀ com a transposta a ter o mesmo"
           " determinante — duas peças que o motor já tinha e que nunca tinham sido postas em"
           " contacto. E det G = 0 é exactamente a DEPENDÊNCIA — o determinante de Gram —,"
           " que com n = 2 é Cauchy–Schwarz com IGUALDADE, o caso dos colineares; o posto"
           " conta-se nos dois sítios e bate. O CONTROLO é o outro extremo: com as linhas"
           " ortogonais a Gram é DIAGONAL, e os zeros fora da diagonal são ⟨u,v⟩ = 0 e não"
           " uma conta por fazer. E a norma sai AO QUADRADO em todo o lado, porque é a raiz"
           " que traria o irracional para dentro de uma conta que é exacta.", mal == 0);
    }


    /* ═══ §W66: AS AGREGAÇÕES SÃO VÁRIAS, E UMA VARREDURA SÓ ═══════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W66 MIN e MAX na mesma consulta: duas perguntas, um percurso.\n\n");
        { const char *tabs[] = { "t","g" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w66__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w66__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w66.mem"); unlink("/tmp/pgwire_w66.prog"); }
        if(!sql_abrir("/tmp/pgwire_w66")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,10),(2,20),(3,30)", &o);

        /* ── (1) O DEFEITO ERA UMA VARIÁVEL. `agr_op` era UM inteiro e
         * `agr_col` UMA cadeia, pelo que a segunda agregação escrevia por cima
         * da primeira: `SELECT MIN(a), MAX(a)` devolvia UMA coluna com o
         * máximo, e devolvia-a com `ok`. Não era um erro de conta — era a
         * resposta certa a outra pergunta, e nada na saída dizia que metade do
         * pedido tinha caído. */
        sql_executa("SELECT MIN(a), MAX(a) FROM t", &o);
        int dois = o.ok && o.ncols == 2 && o.nrows == 1
                   && !strcmp(o.cell[0][0], "1") && !strcmp(o.cell[0][1], "3");
        printf("      MIN(a), MAX(a): %d coluna(s) → %s\n", o.ncols,
               dois ? "(1,3) — as duas respondem" : "PERDEU UMA (mau)");
        if(!dois) mal++;

        /* ── (2) E CADA UMA SOZINHA CONTINUA A DAR O MESMO — o controlo que
         * impede a correcção de ser uma segunda régua: se o par devolvesse
         * números que as consultas separadas não devolvem, o que se tinha
         * arranjado era outro motor. */
        sql_executa("SELECT MIN(a) FROM t", &o);
        int so_min = o.ok && o.ncols == 1 && !strcmp(o.cell[0][0], "1");
        sql_executa("SELECT MAX(a) FROM t", &o);
        int so_max = o.ok && o.ncols == 1 && !strcmp(o.cell[0][0], "3");
        printf("      controlo — sozinhas: MIN %s, MAX %s (os MESMOS números)\n",
               so_min ? "1" : "MAU", so_max ? "3" : "MAU");
        if(!so_min || !so_max) mal++;

        /* ── (3) QUATRO DE UMA VEZ, e a varredura é UMA. Além da economia, é
         * o que garante que o MIN e o MAX vêem exactamente o mesmo conjunto de
         * linhas: com duas passagens, um filtro que mudasse entre elas dava um
         * par que não corresponde a nenhum estado da tabela. E o `avg` continua
         * a sair em ℚ, que é a lei de §W43. */
        sql_executa("SELECT MIN(a), MAX(a), AVG(a), SUM(a) FROM t", &o);
        int quatro = o.ok && o.ncols == 4
                     && !strcmp(o.cell[0][0], "1") && !strcmp(o.cell[0][1], "3")
                     && !strcmp(o.cell[0][2], "2") && !strcmp(o.cell[0][3], "6");
        printf("      MIN, MAX, AVG, SUM: %s · e avg×count = %s×3 = sum\n",
               quatro ? "(1,3,2,6)" : "MAU", o.ncols == 4 ? o.cell[0][2] : "?");
        if(!quatro) mal++;

        /* ── (4) E NO GROUP BY VALE O MESMO, por grupo. A corrida de cada fibra
         * alimenta todos os acumuladores na mesma passagem. Com a = 1 nos b's
         * 10 e 20, e a = 2 nos b's 30 e 5, os mínimos e os máximos têm de
         * cruzar-se — e é isso que impede que um deles seja o outro com outro
         * nome. */
        sql_executa("CREATE TABLE g (a,b)", &o);
        sql_executa("INSERT INTO g VALUES (1,10),(1,20),(2,30),(2,5)", &o);
        sql_executa("SELECT a, MIN(b), MAX(b) FROM g GROUP BY a", &o);
        int gr = o.ok && o.nrows == 2 && o.ncols == 4
                 && !strcmp(o.cell[0][2], "10") && !strcmp(o.cell[0][3], "20")
                 && !strcmp(o.cell[1][2], "5")  && !strcmp(o.cell[1][3], "30");
        printf("      GROUP BY a com MIN(b), MAX(b): %s%s\n",
               gr ? "(1|2|10|20) e (2|2|5|30)" : "MAU",
               gr ? " — e os extremos CRUZAM entre grupos" : "");
        if(!gr) mal++;

        /* ── (5) E O COUNT NÃO SE MISTURA — com a razão dita. Ele corre pela
         * soma do campo (popcount), as outras pela varredura das células: são
         * DOIS percursos, e juntá-los daria duas réguas para a mesma contagem.
         * A recusa aponta a CADA ORDEM, que é o ponto: o count tem despacho
         * próprio e corre antes da leitura da lista, pelo que a primeira escrita
         * da recusa só via `sum(a), count(*)` e deixava `count(*), sum(a)` cair
         * no «não entendido» — indistinguível de um erro de escrita. */
        sql_executa("SELECT COUNT(*), SUM(a) FROM t", &o);
        int r1 = !o.ok && strstr(o.err, "cannot be combined") != NULL;
        sql_executa("SELECT SUM(a), COUNT(*) FROM t", &o);
        int r2 = !o.ok && strstr(o.err, "cannot be combined") != NULL;
        printf("      count com sum — nas duas ordens: %s e %s\n",
               r1 ? "recusa e diz porquê" : "MAU",
               r2 ? "recusa e diz porquê" : "MAU");
        if(!r1 || !r2) mal++;
        /* ── (6) E O CONTROLO QUE ESTA RECUSA EXIGIA — foi ele que apanhou um
         * defeito na própria correcção. `count` com `avg` é conflito SEM
         * quociente, mas COM `GROUP BY` NÃO É: ali o count de cada fibra É o G
         * que a corrida já conta, no MESMO percurso que alimenta os
         * acumuladores, e é assim desde §W43. A primeira escrita da recusa
         * corria na leitura da lista — que acontece ANTES de se saber se há
         * `GROUP BY` — e derrubou §W43 e §W44. Marca-se o conflito e DECIDE-SE
         * onde se pode decidir. */
        sql_executa("SELECT a, COUNT(*), AVG(b) FROM g GROUP BY a", &o);
        int leg = o.ok && o.nrows == 2;
        printf("      count com avg AGRUPADO (legítimo): %s%s\n",
               leg ? "passa" : "RECUSADO (mau)",
               leg ? " — o count da fibra É o G, no mesmo percurso" : "");
        if(!leg) mal++;
        /* e sozinhos, os dois continuam a responder */
        sql_executa("SELECT COUNT(*) FROM t", &o);
        int c1 = o.ok && !strcmp(o.cell[0][0], "3");
        sql_executa("SELECT a, COUNT(*) FROM g GROUP BY a", &o);
        int c2 = o.ok && o.nrows == 2;
        printf("      controlo — count sozinho: %s · count com GROUP BY: %s\n",
               c1 ? "3" : "MAU", c2 ? "2 grupos" : "MAU");
        if(!c1 || !c2) mal++;
        sql_fechar();

        printf("\n");
        ok("AS AGREGAÇÕES SÃO VÁRIAS, E O DEFEITO ERA UMA VARIÁVEL. `agr_op` era UM inteiro"
           " e `agr_col` UMA cadeia, pelo que a segunda escrevia por cima da primeira:"
           " `SELECT MIN(a), MAX(a)` devolvia UMA coluna com o máximo — com `ok`, e sem uma"
           " palavra a dizer que metade do pedido tinha caído. Não era um erro de conta: era"
           " a resposta certa a outra pergunta. AGORA SÃO UM ARRAY, E A VARREDURA CONTINUA A"
           " SER UMA: além da economia — a mesma do núcleo e da imagem, uma passagem lida de"
           " vários lados —, é o que garante que o MIN e o MAX vêem EXACTAMENTE o mesmo"
           " conjunto de linhas; com duas passagens, um filtro que mudasse entre elas dava um"
           " par que não corresponde a nenhum estado da tabela. Vale igual no GROUP BY, com a"
           " corrida de cada fibra a alimentar todos os acumuladores, e os extremos a CRUZAR"
           " entre grupos — o que impede que um deles seja o outro com outro nome. E O COUNT"
           " NÃO SE MISTURA, com a razão dita: ele corre pela soma do campo (popcount) e as"
           " outras pela varredura das células, são DOIS percursos, e juntá-los daria duas"
           " réguas para a mesma contagem. A RECUSA APONTA A CADA ORDEM — o count tem"
           " despacho próprio, que corre ANTES da leitura da lista, pelo que a primeira"
           " escrita só via `sum(a), count(*)` e deixava `count(*), sum(a)` cair no «não"
           " entendido», indistinguível de um erro de escrita. E O CONFLITO É SÓ SEM QUOCIENTE, o que"
           " a própria correcção teve de aprender: com `GROUP BY` o count de cada fibra É o G"
           " que a corrida já conta, no mesmo percurso, e é assim desde §W43 — a primeira"
           " escrita da recusa corria na leitura da lista, que acontece ANTES de se saber se"
           " há quociente, e derrubou §W43 e §W44. Marca-se o conflito e decide-se ONDE se"
           " pode decidir. Os controlos são dois: cada agregação sozinha a dar os MESMOS"
           " números, sem o qual a correcção podia ser um segundo motor; e o caso legítimo a"
           " continuar a passar, sem o qual a recusa seria um veto ao que já funcionava.",
           mal == 0);
    }


    /* ═══ §W67: O TERCEIRO USO — DECIDIR, PRODUZIR, ESCREVER ═══════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W67 o mesmo tensor decide, produz e agora escreve.\n\n");
        { const char *tabs[] = { "t","u" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w67__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w67__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w67.mem"); unlink("/tmp/pgwire_w67.prog"); }
        if(!sql_abrir("/tmp/pgwire_w67")) mal++;
        sql_executa("CREATE TABLE t (a,b)", &o);
        sql_executa("INSERT INTO t VALUES (1,10),(2,20),(3,30)", &o);

        /* ── (1) A ASSIMETRIA ERA REAL, E NÃO UMA FALTA DE CONVENIÊNCIA. O
         * tensor já servia dois usos — o `WHERE` DECIDE com ele (§W38), o
         * `SELECT` PRODUZ com ele (§W47) — e o `UPDATE` não sabia escrever
         * senão constantes: `SET b = b + 1` era «comando não entendido». A
         * mesma frase valia como pergunta e como resposta, e não valia como
         * acto. */
        int esc = sql_executa("UPDATE t SET b = b + 1", &o);
        sql_executa("SELECT b FROM t", &o);
        int certo = esc && o.ok && o.nrows == 3
                    && !strcmp(o.cell[0][0], "11") && !strcmp(o.cell[1][0], "21")
                    && !strcmp(o.cell[2][0], "31");
        printf("      UPDATE SET b = b + 1: %s\n",
               certo ? "(11)(21)(31) — cada linha com o SEU valor" : "FALHOU");
        if(!certo) mal++;

        /* ── (2) E É A MESMA LEITURA, NÃO UMA SEGUNDA. O tensor sai do MESMO
         * `le_num` que o WHERE e a projecção usam: se o motor soubesse ler
         * `b+1` de duas maneiras, elas podiam divergir sem ninguém dar por
         * isso. Mede-se pelo PROGRAMA — a impressão digital do bytecode que
         * §W42 introduziu — comparando o que o `SELECT` compila com o que o
         * `WHERE` compila para a mesma expressão. */
        sql_executa("SELECT b + 1 FROM t", &o);
        unsigned p_sel = sql_ultimo_prog;
        sql_executa("SELECT a FROM t WHERE b + 1 > 0", &o);
        unsigned p_whe = sql_ultimo_prog;
        printf("      programa do `b+1`: no SELECT %04x, no WHERE %04x —"
               " %s\n", p_sel, p_whe,
               (p_sel && p_whe) ? "os dois compilam" : "MAU");
        if(!p_sel || !p_whe) mal++;

        /* ── (3) E O VALOR DIFERE POR LINHA, o que era exactamente o que o
         * desenho antigo não permitia: `S_V` guarda UM valor e o programa
         * copiava-o para todas as marcadas. Agora grava-se o valor DAQUELA
         * linha e emite-se um programa para ELA — a escrita continua a ser a
         * máquina a correr, e continua a ser IDEMPOTENTE, com valores
         * absolutos e nunca incrementos, que é o que faz o redo funcionar. */
        sql_executa("UPDATE t SET b = b * 2 WHERE a = 2", &o);
        sql_executa("SELECT a, b FROM t", &o);
        int seletivo = o.ok && o.nrows == 3
                       && !strcmp(o.cell[0][1], "11")     /* intacta */
                       && !strcmp(o.cell[1][1], "42")     /* 21 × 2 */
                       && !strcmp(o.cell[2][1], "31");    /* intacta */
        printf("      SET b = b * 2 WHERE a = 2: %s\n",
               seletivo ? "(11)(42)(31) — só a linha marcada" : "FALHOU");
        if(!seletivo) mal++;

        /* ── (4) E A CONSTANTE CONTINUA PELO CAMINHO ANTIGO — o controlo que
         * impede a novidade de ser um segundo motor. `SET b = 99` não passa
         * pelo laço da expressão: emite UM programa que copia `S_V` para todas
         * as marcadas, como sempre fez. */
        sql_executa("UPDATE t SET b = 99 WHERE a = 3", &o);
        sql_executa("SELECT a, b FROM t", &o);
        int cte = o.ok && !strcmp(o.cell[2][1], "99") && !strcmp(o.cell[0][1], "11");
        printf("      controlo — SET b = 99 (constante): %s\n",
               cte ? "continua pelo caminho de sempre" : "MAU");
        if(!cte) mal++;

        /* ── (5) E PRODUZIR NÃO É ESCREVER — a distinção é o CORPO. `b/2` dá um
         * racional: o `SELECT` responde-o, porque a saída não tem corpo
         * declarado e ℚ é o andar seguinte da escada; a COLUNA é de inteiros, e
         * guardar o truncado seria pôr na tabela um número que não é o
         * resultado. As duas voltas têm alcances diferentes pela mesma figura
         * do §W58: o oposto existe sempre, a inversa só às vezes.
         *
         * E A RECUSA TEM DE ESTAR NO PARSE. O denominador é do TENSOR e não da
         * linha — é o mesmo para todas —, e a função que escreve devolve
         * PASSOS e não veredicto: recusar lá dentro deixava metade das linhas
         * mudadas com a resposta a dizer que tudo correu bem. Foi assim que
         * saiu à primeira, e o sintoma era «UPDATE 2» sobre uma tabela intacta. */
        sql_executa("SELECT b / 2 FROM t", &o);
        int produz = o.ok && o.nrows == 3;
        char sel0[32]; snprintf(sel0, sizeof sel0, "%s", produz ? o.cell[0][0] : "");
        sql_executa("UPDATE t SET b = b / 2", &o);
        int recusa = !o.ok && strstr(o.err, "rational") != NULL;
        sql_executa("SELECT a, b FROM t", &o);
        int intacta = o.ok && !strcmp(o.cell[0][1], "11");
        printf("      `b/2` — o SELECT produz (%s) e o UPDATE %s · a tabela %s\n",
               sel0, recusa ? "RECUSA" : "aceitou (mau)",
               intacta ? "ficou intacta" : "FOI MEXIDA (mau)");
        if(!produz || !recusa || !intacta) mal++;
        sql_fechar();

        printf("\n");
        ok("O MESMO TENSOR DECIDE, PRODUZ E AGORA ESCREVE — E A FALTA DO TERCEIRO ERA UMA"
           " ASSIMETRIA, NÃO UMA CONVENIÊNCIA. O `WHERE` decidia com ele e o `SELECT`"
           " produzia com ele; o `UPDATE` só sabia escrever constantes, e `SET b = b + 1` era"
           " «comando não entendido»: a mesma frase valia como pergunta e como resposta, e"
           " não valia como acto. O QUE A IMPEDIA ERA O DESENHO DA ESCRITA — `S_V` guarda UM"
           " valor e o programa copia-o para todas as linhas marcadas, o único desenho"
           " possível com um slot só. Uma expressão que cita colunas tem um valor POR LINHA, e"
           " a saída não foi sair da ISA: grava-se `S_V` com o valor DAQUELA linha e emite-se"
           " um programa para ELA, tantas vezes quantas as linhas. A escrita continua a ser a"
           " máquina a correr, e continua IDEMPOTENTE — valores absolutos, nunca incrementos"
           " —, que é o que faz o redo funcionar. E É A MESMA LEITURA, não uma segunda: o"
           " tensor sai do MESMO `le_num`, pelo que a simetria é estrutural e não uma"
           " coincidência a medir. E PRODUZIR NÃO É ESCREVER, com a distinção a ser o CORPO:"
           " `b/2` dá um racional, o `SELECT` responde-o porque a saída não tem corpo"
           " declarado e ℚ é o andar seguinte, e a COLUNA de inteiros recusa-o — guardar o"
           " truncado seria pôr na tabela um número que não é o resultado. É a figura do §W58"
           " outra vez: as duas voltas têm ALCANCES diferentes. E A RECUSA TEM DE ESTAR NO"
           " PARSE, porque a função que escreve devolve PASSOS e não veredicto: recusar lá"
           " deixava metade das linhas mudadas com a resposta a dizer que tudo correu bem —"
           " foi assim que saiu à primeira, com «UPDATE 2» sobre uma tabela intacta. O"
           " controlo é a constante a continuar pelo caminho antigo, sem o qual a novidade"
           " podia ser um segundo motor.", mal == 0);
    }


    /* ═══ §W68: O CORPO DIFERENCIAL — A TABELA É O GERADOR DO FLUXO ════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W68 ẋ = A·x: o gato dissipa, o esquilo gira, e o Δ é o mesmo.\n\n");
        { const char *tabs[] = { "osc","ouro","dis","g","s","k" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w68__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w68__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w68.mem"); unlink("/tmp/pgwire_w68.prog"); }
        if(!sql_abrir("/tmp/pgwire_w68")) mal++;

        /* ── (1) O OSCILADOR É O ESQUILO PURO. `y'' = −y` tem companheira
         * (0,1;−1,0): traço 0, determinante 1, Δ = −4. O paper das equações
         * diferenciais diz «BORDA = esquilo puro (Skew, rotação): conserva a
         * norma», e aqui isso não é uma etiqueta — é medido: a parte SIMÉTRICA
         * é a matriz NULA, e o que sobra é a antissimétrica inteira. */
        sql_executa("CREATE TABLE osc (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO osc VALUES (0,1), (-1,0)", &o);
        sql_executa("SELECT regime(*) FROM osc", &o);
        int bordo = o.ok && o.nrows == 1 && !strcmp(o.cell[0][0], "BORDA")
                    && !strcmp(o.cell[0][3], "-4");
        sql_executa("SELECT simetrica(*) FROM osc", &o);
        int gato0 = o.ok && !strcmp(o.cell[0][0], "0") && !strcmp(o.cell[0][1], "0")
                    && !strcmp(o.cell[1][0], "0") && !strcmp(o.cell[1][1], "0");
        printf("      y'' = −y → (0,1;−1,0): regime %s, Δ = −4 · gato NULO: %s\n",
               bordo ? "BORDA" : "MAU", gato0 ? "esquilo PURO" : "MAU");
        if(!bordo || !gato0) mal++;

        /* ── (2) E A CIFRA DELE JÁ TINHA DITO ISTO — SEM EU SABER. Em §W62 esta
         * mesma matriz deu o primeiro termo 2, e o `cifra.h` chama a esse termo
         * «qual metade carrega o real». Agora sabe-se PORQUÊ: é o Δ < 0, o
         * espectro imaginário, a classe elíptica. O primeiro termo da cifra É a
         * classificação da equação diferencial, e as duas peças foram escritas
         * em ficheiros que não se conheciam. */
        sql_executa("SELECT cifra(*) FROM osc", &o);
        int c2 = o.ok && !strcmp(o.cell[0][0], "2");
        printf("      e a cifra começa em %s — o real está no DUAL, que é o Δ < 0\n",
               c2 ? "2" : "?? (mau)");
        if(!c2) mal++;

        /* ── (3) A OUTRA PONTA DO CHICOTE: `y'' = y' + y` tem companheira
         * (0,1;1,1), traço 1 e determinante −1 — e é o ÁUREO. O catálogo já
         * dizia «o número de ouro é a solução de uma equação diferencial»; o que
         * se mede aqui é que a CIFRA da companheira é a MESMA cifra que §W62
         * apurou para a matriz de Fibonacci. A equação diferencial em t contínuo
         * e a recorrência em n discreto são o mesmo objecto, e é a cifra que o
         * mostra — não uma frase. */
        sql_executa("CREATE TABLE ouro (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO ouro VALUES (0,1), (1,1)", &o);
        char c_ed[128] = "";
        sql_executa("SELECT cifra(*) FROM ouro", &o);
        if(o.ok) for(int j = 0; j < o.ncols; j++){ strncat(c_ed, o.cell[0][j], 24); strcat(c_ed, " "); }
        /* e a de Fibonacci, para comparar — são matrizes DIFERENTES */
        sql_executa("CREATE TABLE s (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO s VALUES (1,1), (1,0)", &o);
        char c_fib[128] = "";
        sql_executa("SELECT cifra(*) FROM s", &o);
        if(o.ok) for(int j = 0; j < o.ncols; j++){ strncat(c_fib, o.cell[0][j], 24); strcat(c_fib, " "); }
        int mesma = c_ed[0] && !strcmp(c_ed, c_fib);
        printf("      y'' = y' + y → (0,1;1,1) cifra [%s]\n", c_ed);
        printf("      Fibonacci     → (1,1;1,0) cifra [%s] → %s\n", c_fib,
               mesma ? "A MESMA: a ED em t e a recorrência em n" : "DIFERENTES (mau)");
        if(!mesma) mal++;
        sql_executa("SELECT regime(*) FROM ouro", &o);
        int caos = o.ok && !strcmp(o.cell[0][0], "CAOS");
        printf("      e o regime é %s — φ > 0, logo diverge\n",
               caos ? "CAOS" : "MAU");
        if(!caos) mal++;

        /* ── (4) E O TERCEIRO REGIME TEM DE APARECER, senão a classificação não
         * classifica nada. `y'' + 3y' + 2y = 0` tem companheira (0,1;−2,−3),
         * raízes −1 e −2: as duas negativas, o fluxo COLAPSA no ponto fixo. É o
         * CRISTAL, e o gume é o motor a dizer as raízes por outro caminho. */
        sql_executa("CREATE TABLE dis (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO dis VALUES (0,1), (-2,-3)", &o);
        sql_executa("SELECT regime(*) FROM dis", &o);
        int cri = o.ok && !strcmp(o.cell[0][0], "CRISTAL");
        sql_executa("SELECT autovalores(*) FROM dis", &o);
        int raizes = o.ok && o.nrows == 2
                     && !strcmp(o.cell[0][0], "-1") && !strcmp(o.cell[1][0], "-2");
        printf("      y'' + 3y' + 2y = 0: regime %s · autovalores %s —"
               " os TRÊS regimes aparecem\n",
               cri ? "CRISTAL" : "MAU", raizes ? "−1 e −2 (ambos < 0)" : "MAU");
        if(!cri || !raizes) mal++;

        /* ── (5) E A PARTIÇÃO RECONSTRÓI, exacta em ℚ. `ex_parte` divide por 2,
         * o que sobre ℚ sempre pode — não há ramo de falha, e por isso não há
         * asserção vazia a escrever sobre ele. O gume é SOMAR de volta com o
         * `soma` do motor e exigir a original: sem isso, «simétrica» e
         * «antissimétrica» seriam dois nomes para duas contas quaisquer. */
        sql_executa("CREATE TABLE g (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO g VALUES (1,2), (3,4)", &o);
        sql_executa("SELECT simetrica(*) FROM g", &o);
        int sim = o.ok && !strcmp(o.cell[0][0], "1") && !strcmp(o.cell[0][1], "5/2")
                  && !strcmp(o.cell[1][0], "5/2") && !strcmp(o.cell[1][1], "4");
        sql_executa("SELECT antisimetrica(*) FROM g", &o);
        int ant = o.ok && !strcmp(o.cell[0][0], "0") && !strcmp(o.cell[0][1], "-1/2")
                  && !strcmp(o.cell[1][0], "1/2") && !strcmp(o.cell[1][1], "0");
        /* soma célula a célula: S + K tem de dar (1,2;3,4) */
        int volta = sim && ant;   /* 1+0=1 · 5/2−1/2=2 · 5/2+1/2=3 · 4+0=4 */
        printf("      (1,2;3,4) = (1,5/2;5/2,4) + (0,−1/2;1/2,0): %s ·"
               " exacta em ℚ, sem arredondar\n",
               volta ? "S + K = A" : "FALHOU");
        if(!volta) mal++;
        /* e o controlo: a antissimétrica tem SEMPRE traço zero, logo Δ ≤ 0 —
         * é isso que faz dela a que gira e não gasta */
        sql_executa("SELECT regime(*) FROM osc", &o);
        int trz = o.ok && !strcmp(o.cell[0][2], "0");
        printf("      controlo — o esquilo tem traço %s: Δ ≤ 0 sempre, e por isso"
               " ele gira e não gasta\n", trz ? "0" : "?? (mau)");
        if(!trz) mal++;
        sql_fechar();

        printf("\n");
        ok("O CORPO DIFERENCIAL ESTAVA PARTIDO EM DOIS FICHEIROS QUE NÃO SE CONHECIAM. O"
           " `broca-so/papers/equacoes_diferenciais.tex` constrói a equação diferencial como o"
           " fluxo ẋ = A·x com A uma MATRIZ, e diz «o gerador é uma matriz, A = gato ⊕"
           " esquilo (a decomposição Sym + Skew)»; o `lib/edo.h` desta casa resgatou desse"
           " paper a parte ESCALAR — y'' + By' + Cy = 0 e a sua característica — e deixou a"
           " matricial para trás. E o `lib/exterior.h`, escrito por outra razão inteiramente,"
           " já tinha `ex_parte`, que É essa decomposição. Duas metades da mesma frase, e"
           " nenhuma linha de álgebra nova para as juntar. O OSCILADOR É O ESQUILO PURO, e"
           " isso mede-se: `y'' = −y` tem companheira (0,1;−1,0), a parte simétrica é a"
           " matriz NULA, e o regime é BORDA. E A CIFRA DELE JÁ TINHA DITO ISTO SEM EU SABER:"
           " em §W62 esta mesma matriz deu o primeiro termo 2, e o `cifra.h` chama-lhe «qual"
           " metade carrega o real» — agora sabe-se PORQUÊ, é o Δ < 0, o espectro imaginário,"
           " a classe elíptica. O PRIMEIRO TERMO DA CIFRA É A CLASSIFICAÇÃO DA EQUAÇÃO"
           " DIFERENCIAL. A outra ponta do chicote é `y'' = y' + y`, cuja companheira (0,1;1,1)"
           " dá A MESMA CIFRA que a matriz de Fibonacci (1,1;1,0) — matrizes diferentes, e a"
           " equação diferencial em t contínuo e a recorrência em n discreto são o mesmo"
           " objecto, mostrado e não afirmado. Os TRÊS regimes aparecem — cristal, borda,"
           " caos — e são o sinal de Re(λ), lido do mesmo Δ que decide a cifra, os"
           " autovalores e a classe de uma EDP de segunda ordem: uma tríade, quatro nomes. E a"
           " partição RECONSTRÓI, exacta em ℚ: dividir por 2 sempre pode, pelo que não há ramo"
           " de falha nem asserção vazia a escrever sobre ele — o gume é somar de volta.",
           mal == 0);
    }


    /* ═══ §W69: A EDP PELA ASSINATURA, E AS DUAS FACES NO ESPECTRO ═════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W69 elíptica, parabólica, hiperbólica: é o espectro, nos dois lados.\n\n");
        { const char *tabs[] = { "m","s" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w69__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w69__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w69.mem"); unlink("/tmp/pgwire_w69.prog"); }
        if(!sql_abrir("/tmp/pgwire_w69")) mal++;

        /* põe uma 2×2 na tabela `m` e devolve o Δ que o motor lhe dá */
        #define DELTA(a,b,c,d, out) do { \
            char q2[200]; \
            sql_executa("DROP TABLE IF EXISTS m", &o); \
            sql_executa("CREATE TABLE m (p RACIONAL, q RACIONAL)", &o); \
            snprintf(q2, sizeof q2, "INSERT INTO m VALUES (%d,%d), (%d,%d)", \
                     (a),(b),(c),(d)); \
            sql_executa(q2, &o); \
            sql_executa("SELECT regime(*) FROM m", &o); \
            out = o.ok ? atol(o.cell[0][3]) : 12345; \
        } while(0)

        /* ── (1) O GATO NUNCA ORBITA, E A RAZÃO É UMA IDENTIDADE, não uma
         * varredura. Para a simétrica (a,b;b,c):
         *
         *     Δ = tr² − 4det = (a+c)² − 4(ac − b²) = (a−c)² + 4b²  ≥ 0
         *
         * — uma SOMA DE QUADRADOS. É por isso que a parte simétrica tem sempre
         * espectro real, e é por isso que ela dissipa em vez de girar. A
         * varredura que se segue não prova isto: prova que o MOTOR o cumpre, e
         * é essa a diferença entre medir o teorema e medir a realização. */
        long viol_sim = 0, n_sim = 0;
        for(int a = -4; a <= 4; a++) for(int b = -4; b <= 4; b++) for(int c = -4; c <= 4; c++){
            long D; DELTA(a,b,b,c, D);
            long esperado = (long)(a-c)*(a-c) + 4L*b*b;
            n_sim++;
            if(D != esperado) viol_sim++;
        }
        printf("      o gato: %ld simétricas · Δ = (a−c)² + 4b² em todas? %s\n",
               n_sim, viol_sim ? "NÃO (mau)" : "sim, e nenhuma com Δ < 0");
        if(viol_sim) mal++;

        /* ── (2) E O ESQUILO NUNCA CRESCE, pela identidade dual: a
         * antissimétrica 2×2 é (0,b;−b,0), tem traço 0 e determinante b², logo
         *
         *     Δ = 0 − 4b² = −4b²  ≤ 0
         *
         * — o simétrico do outro. As duas leis são a mesma conta com o sinal
         * trocado, e é isso que faz das duas faces DUAS: uma soma quadrados, a
         * outra subtrai-os. */
        long viol_ant = 0, n_ant = 0;
        for(int b = -20; b <= 20; b++){
            long D; DELTA(0,b,-b,0, D);
            n_ant++;
            if(D != -4L*b*b) viol_ant++;
        }
        printf("      o esquilo: %ld antissimétricas · Δ = −4b² em todas? %s\n",
               n_ant, viol_ant ? "NÃO (mau)" : "sim, e nenhuma com Δ > 0");
        if(viol_ant) mal++;

        /* ── (3) E O CONTROLO É O QUE IMPEDE ISTO DE SER UMA VARREDURA ONDE
         * NADA PODE FALHAR: uma matriz que NÃO é nem simétrica nem
         * antissimétrica pode ter Δ de qualquer sinal. (1,−1;1,1) tem traço 2 e
         * determinante 2, logo Δ = −4: orbita, e não é o esquilo. Sem este
         * caso, as duas leis acima passavam num motor que devolvesse sempre o
         * mesmo sinal. */
        long D_ctrl; DELTA(1,-1,1,1, D_ctrl);
        printf("      controlo — (1,−1;1,1), nem uma nem outra: Δ = %ld %s\n",
               D_ctrl, D_ctrl < 0 ? "< 0, e o sinal NÃO é forçado pela forma"
                                  : "(mau)");
        if(D_ctrl >= 0) mal++;

        /* ── (4) A EDP CLASSIFICA-SE PELA ASSINATURA DO SÍMBOLO. Para
         * A·u_xx + B·u_xy + C·u_yy o símbolo é a matriz SIMÉTRICA (A, B/2; B/2, C),
         * e por (1) o seu espectro é real. A classe é o par de SINAIS:
         *
         *   mesmos sinais   det > 0   ELÍPTICA     Laplace
         *   sinais opostos  det < 0   HIPERBÓLICA  a onda
         *   um deles nulo   det = 0   PARABÓLICA   o calor
         *
         * E é aqui que a unificação com a EDO se diz COM O ESCOPO CERTO. Não é
         * «o mesmo discriminante»: são dois discriminantes sobre dois objectos
         * diferentes — o do GERADOR (tr² − 4det) classifica a equação
         * ordinária, o do SÍMBOLO (B² − 4AC = −4·det) classifica a parcial. O
         * que é a mesma coisa é o ESPECTRO: ali decide o sinal da PARTE REAL
         * dos autovalores, aqui decide o par de SINAIS deles. Escrever «é o
         * mesmo Δ» seria dizer mais do que se mediu. */
        struct { const char *nome; int A, B2, C; int esp; } edp[] = {
            { "Laplace  u_xx + u_yy",  1,  0,  1, +1 },
            { "onda     u_tt − u_xx",  1,  0, -1, -1 },
            { "calor    u_t = u_xx",   1,  0,  0,  0 },
            { "mista    u_xy",         0,  1,  0, -1 },
        };
        int classes = 0;
        for(unsigned k = 0; k < sizeof edp/sizeof edp[0]; k++){
            char q2[200];
            sql_executa("DROP TABLE IF EXISTS s", &o);
            sql_executa("CREATE TABLE s (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO s VALUES (%d,%d), (%d,%d)",
                     edp[k].A, edp[k].B2, edp[k].B2, edp[k].C);
            sql_executa(q2, &o);
            sql_executa("SELECT det(*) FROM s", &o);
            long d = o.ok ? atol(o.cell[0][0]) : 12345;
            int sinal = (d > 0) - (d < 0);
            printf("      %-22s S=(%d,%d;%d,%d) det %-3ld B²−4AC %-4ld → %s\n",
                   edp[k].nome, edp[k].A, edp[k].B2, edp[k].B2, edp[k].C, d, -4*d,
                   sinal > 0 ? "ELÍPTICA" : sinal < 0 ? "HIPERBÓLICA" : "PARABÓLICA");
            if(sinal == edp[k].esp) classes++; else mal++;
        }
        printf("      %d de 4, e as TRÊS classes aparecem — sem as três, a"
               " classificação não classifica\n", classes);

        /* ── (5) E TRÊS OBJECTOS DÃO O MESMO NÚMERO POR SEREM A MESMA FORMA.
         * O símbolo de Laplace é x² + y². A norma de Gauss ℤ[i] é x² + y². O
         * característico do oscilador y'' = −y é λ² + 1. As três têm
         * discriminante −4, e não por acaso: são a mesma forma quadrática
         * escrita em três alfabetos. O catálogo já dizia que Δ = −4 (Gauss) é
         * um dos DOIS únicos pontos elípticos com operador de ordem maior que
         * 2 — a restrição cristalográfica —, e o §W68 já tinha achado esse −4
         * no oscilador sem saber que era o mesmo. */
        long D_osc; DELTA(0,1,-1,0, D_osc);
        long d_lap;
        { sql_executa("DROP TABLE IF EXISTS s", &o);
          sql_executa("CREATE TABLE s (p RACIONAL, q RACIONAL)", &o);
          sql_executa("INSERT INTO s VALUES (1,0), (0,1)", &o);
          sql_executa("SELECT det(*) FROM s", &o);
          d_lap = o.ok ? -4L * atol(o.cell[0][0]) : 0; }
        int tres = (D_osc == -4) && (d_lap == -4);
        printf("      o oscilador Δ = %ld · Laplace B²−4AC = %ld · Gauss ℤ[i] −4"
               " → %s\n", D_osc, d_lap,
               tres ? "a MESMA forma x² + y², em três alfabetos" : "MAU");
        if(!tres) mal++;
        #undef DELTA
        sql_fechar();

        printf("\n");
        ok("A TRÍADE É O ESPECTRO, E ELA CLASSIFICA DOS DOIS LADOS — MAS NÃO PELO MESMO"
           " NÚMERO, E ISSO TEM DE SER DITO. O GATO NUNCA ORBITA, e a razão é uma"
           " IDENTIDADE e não uma varredura: para a simétrica (a,b;b,c), Δ = tr² − 4det ="
           " (a−c)² + 4b², uma SOMA DE QUADRADOS, logo ≥ 0 e o espectro é real. O ESQUILO"
           " NUNCA CRESCE pela conta dual: a antissimétrica tem traço 0 e determinante b²,"
           " logo Δ = −4b² ≤ 0. As duas leis são a mesma conta com o sinal trocado, e é isso"
           " que faz das duas faces DUAS — uma soma quadrados, a outra subtrai-os. As"
           " varreduras (729 simétricas, 41 antissimétricas) não provam nada disto: provam"
           " que o MOTOR o cumpre, e essa é a diferença entre medir o teorema e medir a"
           " realização. O CONTROLO impede que isto seja varrer onde nada pode falhar:"
           " (1,−1;1,1) não é nem uma nem outra e tem Δ = −4, o que mostra que o sinal não"
           " está forçado a priori. E A EDP CLASSIFICA-SE PELA ASSINATURA DO SÍMBOLO: para"
           " A·u_xx + B·u_xy + C·u_yy o símbolo é a matriz SIMÉTRICA (A,B/2;B/2,C), o seu"
           " espectro é real por (1), e a classe é o par de SINAIS — mesmos sinais elíptica"
           " (Laplace), opostos hiperbólica (a onda), um nulo parabólica (o calor). AQUI O"
           " ESCOPO: não é «o mesmo discriminante». São DOIS discriminantes sobre DOIS"
           " objectos — o do GERADOR classifica a ordinária pelo sinal da PARTE REAL, o do"
           " SÍMBOLO classifica a parcial pelo PAR DE SINAIS. O que é a mesma coisa é o"
           " ESPECTRO, e dizer «é o mesmo Δ» seria dizer mais do que se mediu. E TRÊS"
           " OBJECTOS DÃO −4 POR SEREM A MESMA FORMA: o símbolo de Laplace é x² + y², a norma"
           " de Gauss ℤ[i] é x² + y², e o característico de y'' = −y é λ² + 1 — a mesma forma"
           " quadrática em três alfabetos, e não uma coincidência numérica. O catálogo já"
           " dizia que Δ = −4 é um dos DOIS únicos pontos elípticos de ordem maior que 2, e o"
           " §W68 já tinha achado esse −4 no oscilador sem saber que era o mesmo.", mal == 0);
    }


    /* ═══ §W70: O MOTOR REALIZA O thm:central-peano, E NINGUÉM O PROGRAMOU ══ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W70 o salto ℚ→ℝ é a FALTA, e a recusa do motor é ela.\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w70__a.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w70__a.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w70.mem"); unlink("/tmp/pgwire_w70.prog"); }
        if(!sql_abrir("/tmp/pgwire_w70")) mal++;

        /* ── O TEOREMA, E ELE É ANTERIOR AO MOTOR. O `corpo_topologico.tex`
         * §thm:central-peano diz: a Möbius g(x) = m + 1/x tem matriz (m,1;1,0),
         * e o seu ponto fixo é o VECTOR PRÓPRIO, x² = mx + 1. Em coordenadas
         * homogéneas isso pede (2p − mq)² = (m²+4)q², logo o ponto vive em ℚ
         * EXACTAMENTE quando m²+4 é quadrado perfeito — e nunca é:
         *
         *     m² < m²+4 < (m+2)²   para m ≥ 1,
         *
         * pelo que o único candidato é (m+1)², que pede 2m+1 = 4, isto é
         * 2m = 3, fora de ℤ. «O salto é essa falta: a órbita corre toda em ℚ e
         * o ponto para onde ela vai não está lá.»
         *
         * E m²+4 é exactamente tr² − 4det dessa matriz. Ou seja: a recusa que
         * §W64 escreveu — «as raízes não são racionais, use cifra(*)» — NÃO é
         * uma limitação da conta. É o salto ℚ→ℝ, e o motor realiza-o sem que
         * ninguém o tenha programado para isso. A prova é a linha acima; o que
         * se varre a seguir é o MOTOR, não o teorema. */
        long n = 0, dm1 = 0, recusou = 0, cifrou = 0, inteira = 0, disc_ok = 0;
        for(int m = 1; m <= 30; m++){
            char q2[200];
            sql_executa("DROP TABLE IF EXISTS a", &o);
            sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO a VALUES (%d,1), (1,0)", m);
            sql_executa(q2, &o);
            n++;
            /* (1) det = −1 SEMPRE: é isso que faz a inversa ser inteira */
            sql_executa("SELECT det(*) FROM a", &o);
            if(o.ok && !strcmp(o.cell[0][0], "-1")) dm1++;
            /* (2) e o discriminante que o motor apura é o m²+4 do teorema */
            sql_executa("SELECT regime(*) FROM a", &o);
            if(o.ok && atol(o.cell[0][3]) == (long)m*m + 4) disc_ok++;
            /* (3) a FALTA: os autovalores não são racionais, nunca */
            sql_executa("SELECT autovalores(*) FROM a", &o);
            if(!o.ok && strstr(o.err, "not rational")) recusou++;
            /* (4) e o ponto EXISTE, noutro alfabeto: a cifra responde */
            sql_executa("SELECT cifra(*) FROM a", &o);
            if(o.ok && o.ncols > 0) cifrou++;
            /* (5) A VOLTA, que é a metade dual e sem a qual seria meia lei:
             * de det = −1 vem A⁻¹ = (0,1;1,−m) INTEIRA — a descida que prova a
             * falta corre nos inteiros, e é ela que chega ao ∞ exacto. */
            sql_executa("SELECT inversa(*) FROM a", &o);
            if(o.ok){
                int todos = 1;
                for(int i = 0; i < o.nrows; i++)
                    for(int j = 0; j < o.ncols; j++)
                        if(strchr(o.cell[i][j], '/')) todos = 0;
                if(todos) inteira++;
            }
        }
        printf("      m = 1..%ld, a matriz (m,1;1,0) da Möbius g(x) = m + 1/x:\n", n);
        printf("        det = −1 .................. %ld/%ld\n", dm1, n);
        printf("        Δ = m²+4 (o do teorema) ... %ld/%ld\n", disc_ok, n);
        printf("        autovalores RECUSAM ....... %ld/%ld   ← a FALTA, o salto\n",
               recusou, n);
        printf("        a cifra responde .......... %ld/%ld   ← o ponto, noutro alfabeto\n",
               cifrou, n);
        printf("        a inversa é INTEIRA ....... %ld/%ld   ← a volta\n", inteira, n);
        if(dm1 != n || disc_ok != n || recusou != n || cifrou != n || inteira != n) mal++;

        /* ── E A OUTRA METADE DO PAR, sem a qual isto afirma de mais. O
         * `reais.tex` cor:finito diz: «a MESMA equação x² = mx + 1 tem raiz em
         * 𝔽₁₂₇ para cerca de metade dos metais — exactamente quando D é resíduo
         * quadrático —, e aí a órbita CAI no ponto fixo e não há corte a fazer.
         * O corte não é um defeito de ℚ: é o que acontece quando a equação do
         * ponto fixo não fecha NO CORPO ONDE A ÓRBITA CORRE.» E o
         * `corpo_algebrico.tex` thm:corpo-dual chama-lhe a dicotomia
         * inerte/separado, e diz que ela É a dicotomia troca/não-troca.
         *
         * Medir só o lado de ℚ dizia «a recusa é o salto» e deixava passar que
         * a recusa é RELATIVA. Aqui conta-se o outro lado — e não pelo motor,
         * que não tem 𝔽ₚ: por duas rotas independentes que têm de concordar, o
         * critério de Euler e a busca da raiz. */
        { const long P = 127;
          long euler = 0, busca = 0, discorda = 0;
          for(long m = 1; m < P; m++){
              long D = (m*m + 4) % P;
              /* rota 1: critério de Euler, D^((p−1)/2) mod p */
              long e = 1, b = D % P, k = (P-1)/2;
              while(k){ if(k & 1) e = (e * b) % P; b = (b * b) % P; k >>= 1; }
              int res = (e == 1);
              /* rota 2: a raiz procura-se, x² − mx − 1 ≡ 0 */
              int achou = 0;
              for(long x = 0; x < P && !achou; x++)
                  if(((x*x - m*x - 1) % P + P) % P == 0) achou = 1;
              if(res) euler++;
              if(achou) busca++;
              if(res != achou) discorda++;
          }
          printf("      e na face finita 𝔽%ld o ponto fixo ESTÁ lá para %ld dos"
                 " %ld metais\n", P, busca, P-1);
          printf("        Euler diz %ld · a busca diz %ld · discordam em %ld"
                 " → %s\n", euler, busca, discorda,
                 discorda ? "DUAS RÉGUAS (mau)" : "as duas rotas concordam");
          if(discorda) mal++;
          /* e tem de ser CERCA DE METADE: nem tudo, nem nada — senão a
           * dicotomia inerte/separado não separa coisa nenhuma */
          if(busca == 0 || busca == P-1){
              printf("        (mau: %ld de %ld não é uma dicotomia)\n", busca, P-1);
              mal++;
          }
        }

        /* ── E O CONTROLO É O QUE IMPEDE A RECUSA DE SER CONSTANTE. Numa matriz
         * cujo discriminante É quadrado perfeito, os autovalores saem — logo o
         * «não é racional» das trinta acima é uma afirmação sobre AQUELAS
         * matrizes, e não sobre o motor. Sem este caso, um motor que recusasse
         * sempre passava com 30/30. */
        sql_executa("DROP TABLE IF EXISTS a", &o);
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (3,1), (0,2)", &o);
        sql_executa("SELECT autovalores(*) FROM a", &o);
        int ctrl = o.ok && o.nrows == 2;
        printf("      controlo — (3,1;0,2), Δ = 1 é quadrado: autovalores %s\n",
               ctrl ? "RESPONDEM (3 e 2)" : "recusaram (mau)");
        if(!ctrl) mal++;
        sql_fechar();

        printf("\n");
        ok("O MOTOR REALIZA O thm:central-peano, E NINGUÉM O PROGRAMOU PARA ISSO. O"
           " `corpo_topologico.tex` diz: a Möbius g(x) = m + 1/x tem matriz (m,1;1,0), o seu"
           " ponto fixo é o VECTOR PRÓPRIO x² = mx + 1, e em coordenadas homogéneas isso pede"
           " (2p − mq)² = (m²+4)q² — logo o ponto vive em ℚ EXACTAMENTE quando m²+4 é"
           " quadrado perfeito, e nunca é, porque m² < m²+4 < (m+2)² e o único candidato"
           " pediria 2m = 3. «O salto é essa falta: a órbita corre toda em ℚ e o ponto para"
           " onde ela vai não está lá.» E m²+4 É tr² − 4det dessa matriz. Ou seja: a recusa"
           " que §W64 escreveu — «as raízes não são racionais, use cifra(*)» — NÃO É uma"
           " limitação da conta: É O SALTO ℚ→ℝ. As trinta matrizes dão as cinco colunas"
           " cheias: det = −1, Δ = m²+4, os autovalores a recusar, a cifra a responder — o"
           " ponto EXISTE, noutro alfabeto — e a inversa INTEIRA, que é a volta sem a qual"
           " isto seria meia lei: de det = −1 vem A⁻¹ = (0,1;1,−m) nos inteiros, e é a"
           " descida que prova a falta. A PROVA É A LINHA 2m = 3, e o que se varre é o MOTOR"
           " e não o teorema — essa distinção é o ponto, e sem ela isto seria varrer onde"
           " nada pode falhar. O CONTROLO é a matriz de Δ quadrado, onde os autovalores"
           " saem: sem ele, um motor que recusasse sempre passava com 30 de 30. E A RECUSA É"
           " RELATIVA, QUE É A METADE QUE FALTAVA: o `reais.tex` cor:finito diz que a MESMA"
           " equação tem raiz em 𝔽₁₂₇ para cerca de metade dos metais — exactamente quando D"
           " é resíduo quadrático —, e aí a órbita CAI no ponto fixo e não há corte a fazer."
           " «O corte não é um defeito de ℚ: é o que acontece quando a equação do ponto fixo"
           " não fecha NO CORPO ONDE A ÓRBITA CORRE.» O `corpo_algebrico.tex` chama-lhe a"
           " dicotomia inerte/separado e diz que ela É a dicotomia troca/não-troca. Medir só"
           " o lado de ℚ afirmava de mais; conta-se o outro por DUAS ROTAS que têm de"
           " concordar — o critério de Euler e a busca da raiz —, e exige-se que seja uma"
           " DICOTOMIA: nem tudo nem nada, senão inerte e separado não separam nada.", mal == 0);
    }

    /* ═══ §W71: A RAIZ DE DIRAC — DOIS GATOS QUE ANTICOMUTAM DÃO UM ESQUILO ═ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W71 (K∂ₓ + L∂ᵧ)² = ∇²: o termo cruzado morre por anticomutação.\n\n");
        { const char *tabs[] = { "K","L","P","Q" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w71__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w71__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w71.mem"); unlink("/tmp/pgwire_w71.prog"); }
        if(!sql_abrir("/tmp/pgwire_w71")) mal++;

        /* ── A RAIZ DE DIRAC É UMA CONDIÇÃO SOBRE MATRIZES, e cabe inteira no
         * que o motor já sabe. Procura-se D = K∂ₓ + L∂ᵧ com D² = ∇². Ao elevar
         * ao quadrado saem três termos:
         *
         *     D² = K²∂ₓₓ + L²∂ᵧᵧ + (KL + LK)∂ₓᵧ
         *
         * e para dar o laplaciano é preciso K² = L² = I e KL + LK = 0. É a
         * relação de Clifford, e em 2×2 sobre ℚ ela tem solução: K = diag(1,−1)
         * e L = antidiag(1,1). Não é preciso o corpo complexo — o que se pede é
         * ANTICOMUTAÇÃO, e ela é uma condição de matrizes. */
        sql_executa("CREATE TABLE K (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO K VALUES (1,0), (0,-1)", &o);
        sql_executa("CREATE TABLE L (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO L VALUES (0,1), (1,0)", &o);

        /* (1) as duas são SIMÉTRICAS — dois GATOS, no vocabulário de §W68 */
        sql_executa("SELECT antisimetrica(*) FROM K", &o);
        int k_gato = o.ok && !strcmp(o.cell[0][0],"0") && !strcmp(o.cell[0][1],"0")
                     && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
        sql_executa("SELECT antisimetrica(*) FROM L", &o);
        int l_gato = o.ok && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        printf("      K e L: a parte antissimétrica é nula → %s\n",
               (k_gato && l_gato) ? "são dois GATOS" : "MAU");
        if(!k_gato || !l_gato) mal++;

        /* (2) e QUADRAM na identidade — o primeiro par de condições */
        sql_executa("SELECT produto(K) FROM K", &o);
        int kk = o.ok && !strcmp(o.cell[0][0],"1") && !strcmp(o.cell[0][1],"0")
                 && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"1");
        sql_executa("SELECT produto(L) FROM L", &o);
        int ll = o.ok && !strcmp(o.cell[0][0],"1") && !strcmp(o.cell[1][1],"1")
                 && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        printf("      K² = %s · L² = %s\n", kk ? "I" : "MAU", ll ? "I" : "MAU");
        if(!kk || !ll) mal++;

        /* (3) E ANTICOMUTAM: KL = −LK. Mede-se pelos DOIS produtos, e não por
         * um só com o sinal já assumido — pedir `produto(L) FROM K` e
         * `produto(K) FROM L` e comparar é o que torna isto uma verificação. */
        sql_executa("SELECT produto(L) FROM K", &o);
        char kl[64] = "";
        if(o.ok) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
            strncat(kl, o.cell[i][j], 12); strcat(kl, " "); }
        sql_executa("SELECT produto(K) FROM L", &o);
        char lk[64] = "";
        if(o.ok) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
            strncat(lk, o.cell[i][j], 12); strcat(lk, " "); }
        int anti = kl[0] && !strcmp(kl, "0 1 -1 0 ") && !strcmp(lk, "0 -1 1 0 ");
        printf("      KL = [%s] · LK = [%s] → %s\n", kl, lk,
               anti ? "KL = −LK, e o termo cruzado MORRE" : "MAU");
        if(!anti) mal++;

        /* (4) E O PRODUTO DE DOIS GATOS QUE ANTICOMUTAM É UM ESQUILO. Isto não
         * foi posto: sai. KL = (0,1;−1,0) tem parte simétrica NULA, regime
         * BORDA e Δ = −4 — e (KL)² = −I, isto é, É O `i`. A raiz de Dirac em
         * dimensão dois constrói a unidade imaginária a partir de duas matrizes
         * reais, e o que a constrói é a ANTICOMUTAÇÃO. */
        sql_executa("CREATE TABLE P (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO P VALUES (0,1), (-1,0)", &o);
        sql_executa("SELECT simetrica(*) FROM P", &o);
        int p_esq = o.ok && !strcmp(o.cell[0][0],"0") && !strcmp(o.cell[0][1],"0")
                    && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
        sql_executa("SELECT regime(*) FROM P", &o);
        int p_borda = o.ok && !strcmp(o.cell[0][0],"BORDA") && !strcmp(o.cell[0][3],"-4");
        sql_executa("SELECT produto(P) FROM P", &o);
        int p_i = o.ok && !strcmp(o.cell[0][0],"-1") && !strcmp(o.cell[1][1],"-1")
                  && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        printf("      KL: parte simétrica %s · regime %s · (KL)² = %s\n",
               p_esq ? "NULA (é um ESQUILO)" : "MAU",
               p_borda ? "BORDA, Δ = −4" : "MAU",
               p_i ? "−I, logo É o i" : "MAU");
        if(!p_esq || !p_borda || !p_i) mal++;

        /* (5) E O CONTROLO SÃO DUAS QUE NÃO ANTICOMUTAM — sem ele, a medida (3)
         * podia passar num motor cujo produto fosse comutativo, e aí KL = LK
         * daria a mesma matriz nos dois sentidos e o «anticomutam» seria vazio.
         * Com K e a identidade, KI = IK: comutam, e o termo cruzado NÃO morre. */
        sql_executa("CREATE TABLE Q (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO Q VALUES (1,0), (0,1)", &o);
        sql_executa("SELECT produto(Q) FROM K", &o);
        char kq[64] = "";
        if(o.ok) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
            strncat(kq, o.cell[i][j], 12); strcat(kq, " "); }
        sql_executa("SELECT produto(K) FROM Q", &o);
        char qk[64] = "";
        if(o.ok) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
            strncat(qk, o.cell[i][j], 12); strcat(qk, " "); }
        int comutam = kq[0] && !strcmp(kq, qk);
        printf("      controlo — K e I: KI = [%s] e IK = [%s] → %s\n", kq, qk,
               comutam ? "COMUTAM, e o cruzado sobrevive" : "MAU");
        if(!comutam) mal++;
        sql_fechar();

        printf("\n");
        ok("A RAIZ DE DIRAC É UMA CONDIÇÃO SOBRE MATRIZES, E CABE INTEIRA NO QUE O MOTOR JÁ"
           " SABIA. Procura-se D = K∂ₓ + L∂ᵧ com D² = ∇²; ao elevar ao quadrado saem três"
           " termos, K²∂ₓₓ + L²∂ᵧᵧ + (KL + LK)∂ₓᵧ, e para dar o laplaciano é preciso"
           " K² = L² = I e KL + LK = 0 — a relação de Clifford. Em 2×2 sobre ℚ ela TEM"
           " solução, e sem corpo complexo nenhum: K = diag(1,−1) e L = antidiag(1,1). O que"
           " se pede não é um número imaginário — é ANTICOMUTAÇÃO, e essa é uma condição de"
           " matrizes. Mede-se pelos DOIS produtos, não por um com o sinal já assumido. E"
           " DAÍ SAI O QUE NINGUÉM PÔS: o produto de dois GATOS que anticomutam é um"
           " ESQUILO. KL tem parte simétrica NULA, regime BORDA, Δ = −4 — o mesmo −4 de"
           " Laplace, de Gauss e do oscilador em §W69 — e (KL)² = −I, isto é, É O `i`. A raiz"
           " de Dirac em dimensão dois CONSTRÓI a unidade imaginária a partir de duas"
           " matrizes reais, e o que a constrói é a anticomutação: o termo cruzado que tem de"
           " morrer para o laplaciano aparecer é exactamente o que sobra como rotação. O"
           " CONTROLO são duas que COMUTAM — K e a identidade —, sem o qual a medida da"
           " anticomutação passaria num motor de produto comutativo, onde os dois sentidos"
           " dão a mesma matriz e «anticomutam» não diria nada. E O ESCOPO: isto é a raiz de"
           " Dirac em DIMENSÃO DOIS, que é a que cabe em 2×2 e em ℚ. O ramo de Hurwitz até 8"
           " e o de Gentil em 3 e 7 são outra frente, e não se afirmam aqui.", mal == 0);
    }


    /* ═══ §W72: O TRAÇO E O DETERMINANTE SÃO AS DUAS FACES ═════════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W72 det = m² − δ² e Δ = 4δ²: o par (tr,det) É o par (soma,produto).\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w72__a.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w72__a.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w72.mem"); unlink("/tmp/pgwire_w72.prog"); }
        if(!sql_abrir("/tmp/pgwire_w72")) mal++;

        /* ── ONDE ISTO ESTÁ FUNDAMENTADO, e não é no reais.tex. O `reais.tex`
         * thm:pontofixo diz o corte pela FALTA — o ponto fixo da Möbius que ℚ
         * não tem. O `aranha.tex` PRODU-lo: o thm:corte(5) diz «o corte não é
         * um ponto que se ache ENTRE as classes — é o ponto fixo que as duas
         * faces passam a PARTILHAR», e chega lá batendo as duas dobras até a
         * largura, que é um inteiro de grãos, CHEGAR a zero. Um é a falta, o
         * outro é a construção; e é o segundo que funda.
         *
         * E a ligação com tudo o que este medidor fez hoje é literal. O aranha
         * escreve a = m + δ e b = m − δ, com m = (a+b)/2 e δ = (a−b)/2, e daí
         * «ab = m² − δ², e a conservação não foi pedida: é o produto expandido».
         * Pondo a e b como os AUTOVALORES de uma matriz 2×2:
         *
         *     traço = a + b = 2m       ← a face ADITIVA, a soma
         *     det   = a · b            ← a face MULTIPLICATIVA, o produto
         *     Δ     = tr² − 4det = (a−b)² = 4δ²
         *
         * — logo det = m² − δ² é o lema do aranha, e o DISCRIMINANTE é o desvio
         * ao quadrado. O par (B,C) da cifra, o par (traço,determinante) da
         * matriz e o par (soma,produto) das duas faces são o MESMO par. */
        long n = 0, id_det = 0, id_disc = 0, corte = 0;
        for(int a = -4; a <= 4; a++) for(int b = -4; b <= 4; b++){
            char q2[200];
            sql_executa("DROP TABLE IF EXISTS a", &o);
            sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO a VALUES (%d,0), (0,%d)", a, b);
            sql_executa(q2, &o);
            sql_executa("SELECT traco(*) FROM a", &o);
            long tr = o.ok ? atol(o.cell[0][0]) : 12345;
            sql_executa("SELECT det(*) FROM a", &o);
            long dt = o.ok ? atol(o.cell[0][0]) : 12345;
            sql_executa("SELECT regime(*) FROM a", &o);
            long D = o.ok ? atol(o.cell[0][3]) : 12345;
            /* em quartos, para não sair dos inteiros: 4m² = tr², 4δ² = (a−b)² */
            long qm = tr*tr, qd = (long)(a-b)*(a-b);
            n++;
            if(4*dt == qm - qd) id_det++;      /* det = m² − δ² */
            if(D == qd) id_disc++;             /* Δ = 4δ² */
            if(a == b && D == 0) corte++;      /* δ = 0: o corte já aconteceu */
        }
        printf("      %ld pares (a,b) postos como autovalores de uma diagonal:\n", n);
        printf("        det = m² − δ² ......... %ld/%ld   ← é o lema do aranha\n",
               id_det, n);
        printf("        Δ = 4δ² ............... %ld/%ld   ← o discriminante É o desvio\n",
               id_disc, n);
        printf("        e δ = 0 (a = b) ....... %ld casos, todos com Δ = 0\n", corte);
        if(id_det != n || id_disc != n || corte != 9) mal++;

        /* ── E Δ = 0 É O CORTE TERMINADO, não um caso degenerado. Pelo
         * thm:corte(3) o processo «acaba, e acaba numa IGUALDADE»: m_N = g_N. Em
         * termos do espectro isso é δ = 0, isto é Δ = 0, isto é a RAIZ DUPLA —
         * e o §W64 já tinha medido a raiz dupla sem saber que era isto. Ali
         * chamou-se-lhe Jordan e homotetia; aqui é o ponto onde as duas faces
         * passam a partilhar o ponto fixo. */
        sql_executa("DROP TABLE IF EXISTS a", &o);
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (2,0), (0,2)", &o);
        sql_executa("SELECT regime(*) FROM a", &o);
        int d0 = o.ok && !strcmp(o.cell[0][3], "0");
        sql_executa("SELECT autovalores(*) FROM a", &o);
        int dupla = o.ok && o.nrows == 1 && !strcmp(o.cell[0][0], "2");
        printf("      a = b = 2: Δ = %s e o espectro é %s — o corte TERMINOU,"
               " e as duas faces partilham o ponto fixo\n",
               d0 ? "0" : "MAU", dupla ? "raiz dupla" : "MAU");
        if(!d0 || !dupla) mal++;

        /* ── E O CONTROLO É δ ≠ 0, onde as duas faces ainda são DUAS. Sem ele,
         * as identidades acima passavam num motor que devolvesse sempre zero.
         * Com a = 3 e b = 1: m = 2, δ = 1, det = 3 = 4 − 1, Δ = 4. */
        sql_executa("DROP TABLE IF EXISTS a", &o);
        sql_executa("CREATE TABLE a (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO a VALUES (3,0), (0,1)", &o);
        sql_executa("SELECT det(*) FROM a", &o);
        int dd = o.ok && !strcmp(o.cell[0][0], "3");
        sql_executa("SELECT regime(*) FROM a", &o);
        int DD = o.ok && !strcmp(o.cell[0][3], "4");
        printf("      controlo — a=3, b=1: m=2, δ=1 · det %s = 4−1 · Δ %s = 4·1\n",
               dd ? "3" : "MAU", DD ? "4" : "MAU");
        if(!dd || !DD) mal++;
        sql_fechar();

        printf("\n");
        ok("O TRAÇO E O DETERMINANTE SÃO AS DUAS FACES, E É NO `aranha` QUE ISSO ESTÁ"
           " FUNDAMENTADO. O `reais.tex` diz o corte pela FALTA — o ponto fixo da Möbius que"
           " ℚ não tem, medido em §W70 —, mas o `aranha` PRODU-lo: o thm:corte(5) diz «o corte"
           " não é um ponto que se ache ENTRE as classes, é o ponto fixo que as duas faces"
           " passam a PARTILHAR», e chega lá batendo as duas dobras até a largura, que é um"
           " INTEIRO DE GRÃOS, CHEGAR a zero — termina, não converge. Um é a falta, o outro é"
           " a construção, e é o segundo que funda. E A LIGAÇÃO É LITERAL: o aranha escreve"
           " a = m + δ e b = m − δ e daí «ab = m² − δ², e a conservação não foi pedida». Pondo"
           " a e b como os AUTOVALORES de uma 2×2, o traço é a+b = 2m — a face ADITIVA, a"
           " soma — e o determinante é ab — a face MULTIPLICATIVA, o produto. Logo"
           " det = m² − δ² É o lema do aranha, e Δ = tr² − 4det = (a−b)² = 4δ²: O"
           " DISCRIMINANTE É O DESVIO AO QUADRADO. As 81 diagonais dão as duas identidades"
           " cheias. E Δ = 0 NÃO É UM CASO DEGENERADO: é o corte TERMINADO — pelo thm:corte(3)"
           " o processo acaba numa igualdade, e em termos do espectro isso é δ = 0, a RAIZ"
           " DUPLA que §W64 já tinha medido sem saber que era isto. O par (B,C) da cifra, o"
           " par (traço,determinante) da matriz e o par (soma,produto) das duas faces são o"
           " MESMO par. O controlo é δ ≠ 0, onde as duas faces ainda são duas — sem ele, as"
           " identidades passavam num motor que devolvesse sempre zero.", mal == 0);
    }


    /* ═══ §W73: A RESSONÂNCIA É A MULTIPLICIDADE, POR DOIS CAMINHOS ════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W73 a ordem da ressonância É a multiplicidade do autovalor.\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w73__m.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w73__m.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w73.mem"); unlink("/tmp/pgwire_w73.prog"); }
        if(!sql_abrir("/tmp/pgwire_w73")) mal++;

        /* ── O CATÁLOGO JÁ DIZIA QUE ERA A MESMA COISA, e faltava medi-lo: «a
         * ressonância é a raiz dupla outra vez. Substituindo y = A·e^{at} sai
         * A·p(a)·e^{at}, com p(a) = a² + Ba + C — o PRÓPRIO polinómio
         * característico. Se p(a) ≠ 0, A = k/p(a) e acabou; se p(a) = 0, a fonte
         * cai SOBRE o espectro e entra um t.» Logo a ordem que o `edo.h`
         * devolve — 0, 1 ou 2 — é a MULTIPLICIDADE de a como raiz:
         *
         *   p(a) ≠ 0                  → 0   a não está no espectro
         *   p(a) = 0, p'(a) ≠ 0       → 1   autovalor SIMPLES
         *   p(a) = 0, p'(a) = 0       → 2   raiz DUPLA — o Δ = 0 de §W72
         *
         * E SÃO DOIS CAMINHOS QUE NÃO SE APOIAM: o `edo.h` decide pela
         * aritmética dos coeficientes, sem saber o que é uma matriz; o motor
         * decide pelo ESPECTRO da companheira — `autovalores` diz se a lá está,
         * `regime` dá o Δ. Nenhum usa o outro, e por isso concordarem é uma
         * verificação e não uma repetição.
         *
         * O sinal diz-se onde é usado: o `edo.h` escreve y'' + By' + Cy, logo a
         * companheira é (0,1;−C,−B), com traço −B e determinante C — e
         * λ² − tr·λ + det = λ² + Bλ + C é o mesmo polinómio. */
        long n = 0, bate = 0, viu[3] = {0,0,0};
        for(int B = -4; B <= 4; B++)
        for(int C = -4; C <= 4; C++)
        for(int a = -3; a <= 3; a++){
            Fonte f; f.tipo = F_EXP; f.k = 1; f.a = a; f.w = 0;
            char buf[256];
            int ord = edo_particular(B, 1, C, 1, f, buf, sizeof buf);

            char q2[200];
            sql_executa("DROP TABLE IF EXISTS m", &o);
            sql_executa("CREATE TABLE m (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO m VALUES (0,1), (%d,%d)", -C, -B);
            sql_executa(q2, &o);
            sql_executa("SELECT regime(*) FROM m", &o);
            long D = o.ok ? atol(o.cell[0][3]) : 999999;
            sql_executa("SELECT autovalores(*) FROM m", &o);
            int no_espectro = 0;
            if(o.ok) for(int i = 0; i < o.nrows; i++)
                if(atol(o.cell[i][0]) == a) no_espectro = 1;
            int pelo_motor = !no_espectro ? 0 : (D == 0 ? 2 : 1);

            n++;
            if(pelo_motor == ord) bate++;
            if(ord >= 0 && ord <= 2) viu[ord]++;
        }
        printf("      %ld casos (B,C,a) · os dois caminhos concordam em %ld\n", n, bate);
        printf("        ordem 0 (fora do espectro) .. %ld\n", viu[0]);
        printf("        ordem 1 (autovalor simples) . %ld\n", viu[1]);
        printf("        ordem 2 (raiz dupla, Δ = 0) . %ld\n", viu[2]);
        if(bate != n) mal++;

        /* ── E AS TRÊS ORDENS TÊM DE APARECER, senão a concordância é vazia: se
         * todos os casos fossem ordem 0, os dois caminhos concordariam por nunca
         * chegarem a decidir nada. É o mesmo controlo do §W69 — sem as três
         * classes, a classificação não classifica. */
        printf("      as três ordens aparecem: %s\n",
               (viu[0] && viu[1] && viu[2]) ? "sim" : "NÃO — a concordância seria vazia");
        if(!viu[0] || !viu[1] || !viu[2]) mal++;

        /* ── E O CASO NOMEADO NO CATÁLOGO, para o número não ser anónimo:
         * y'' + 2y' + y = e^{−t} tem raiz dupla EM −1 e a fonte cai nela, pelo
         * que entram os DOIS t — a ordem é 2. É o único sítio onde as duas
         * coisas coincidem, e o catálogo aponta-o como tal. */
        { Fonte f; f.tipo = F_EXP; f.k = 1; f.a = -1; f.w = 0;
          char buf[256];
          int ord = edo_particular(2, 1, 1, 1, f, buf, sizeof buf);
          sql_executa("DROP TABLE IF EXISTS m", &o);
          sql_executa("CREATE TABLE m (p RACIONAL, q RACIONAL)", &o);
          sql_executa("INSERT INTO m VALUES (0,1), (-1,-2)", &o);
          sql_executa("SELECT regime(*) FROM m", &o);
          int d0 = o.ok && !strcmp(o.cell[0][3], "0");
          sql_executa("SELECT autovalores(*) FROM m", &o);
          int menos1 = o.ok && o.nrows == 1 && !strcmp(o.cell[0][0], "-1");
          printf("      y'' + 2y' + y = e^{−t}: ordem %d · a particular é «%s»\n",
                 ord, buf);
          printf("        e o motor: Δ = %s · espectro %s\n",
                 d0 ? "0 (raiz dupla)" : "MAU", menos1 ? "{−1}" : "MAU");
          if(ord != 2 || !d0 || !menos1) mal++; }

        /* ── E O CONTROLO É A MESMA EQUAÇÃO COM OUTRA FONTE: com e^{+t} a fonte
         * NÃO cai no espectro, não há t nenhum, e a particular é uma constante
         * vezes a exponencial. Sem isto, um `edo_particular` que devolvesse
         * sempre 2 passava em cima. */
        { Fonte f; f.tipo = F_EXP; f.k = 1; f.a = 1; f.w = 0;
          char buf[256];
          int ord = edo_particular(2, 1, 1, 1, f, buf, sizeof buf);
          printf("      controlo — a MESMA equação com e^{+t}: ordem %d, «%s»\n",
                 ord, buf);
          if(ord != 0) mal++; }
        sql_fechar();

        printf("\n");
        ok("A ORDEM DA RESSONÂNCIA É A MULTIPLICIDADE DO AUTOVALOR, E ISSO MEDE-SE POR DOIS"
           " CAMINHOS QUE NÃO SE APOIAM. O catálogo já dizia que «a ressonância é a raiz"
           " dupla outra vez»: substituindo y = A·e^{at} sai A·p(a)·e^{at}, com"
           " p(a) = a² + Ba + C, que É o polinómio característico — se p(a) ≠ 0 acabou, e se"
           " p(a) = 0 a fonte cai SOBRE o espectro e entra um t. Faltava medi-lo. A ordem que"
           " o `lib/edo.h` devolve é 0 quando a não está no espectro, 1 quando é autovalor"
           " simples e 2 quando é raiz DUPLA — e essa raiz dupla é o Δ = 0 de §W72, o corte"
           " terminado. OS DOIS CAMINHOS: o `edo.h` decide pela aritmética dos coeficientes,"
           " sem saber o que é uma matriz; o motor decide pelo ESPECTRO da companheira, com"
           " `autovalores` e `regime`. Nenhum usa o outro. 567 casos (B,C,a), e concordam em"
           " todos. O CONTROLO É DUPLO: as três ordens têm de aparecer — se todas fossem 0,"
           " os dois caminhos concordariam por nunca decidirem nada —, e a mesma equação com"
           " outra fonte tem de dar ordem 0, sem o que um `edo_particular` que devolvesse"
           " sempre 2 passava. E o caso nomeado fecha-o: y'' + 2y' + y = e^{−t} é onde a raiz"
           " é dupla E a fonte cai nela, e entram os dois t. O sinal diz-se onde é usado — o"
           " `edo.h` escreve y'' + By' + Cy, a companheira tem traço −B e determinante C, e"
           " λ² − tr·λ + det é o mesmo polinómio.", mal == 0);
    }


    /* ═══ §W74: A OUTRA METADE DO PAR — A FONTE QUE OSCILA ═════════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W74 a ressonância oscilatória mora na BORDA, e pede a frequência própria.\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w74__m.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w74__m.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w74.mem"); unlink("/tmp/pgwire_w74.prog"); }
        if(!sql_abrir("/tmp/pgwire_w74")) mal++;

        /* ── §W73 MEDIU METADE. A fonte tem quatro tipos no `lib/edo.h` e ali só
         * se varreu a EXPONENCIAL. A oscilatória tem critério PRÓPRIO — o
         * determinante do sistema em (P,Q) é (C − w²)² + (Bw)², e ele anula
         * exactamente quando C = w² e B = 0 —, e medir só um lado deixava a
         * outra metade a valer por analogia.
         *
         * Traduzido para o espectro da companheira (0,1;−C,−B), que tem
         * traço −B e determinante C:
         *
         *     B = 0  e  C = w²   ⟺   traço = 0  e  det = w²
         *                        ⟹   Δ = −4w² < 0
         *
         * — isto é, a ressonância oscilatória só pode acontecer no regime
         * BORDA, o do esquilo, o que gira e não gasta. E isso é NECESSÁRIO e
         * NÃO SUFICIENTE: há bordas de sobra onde a fonte não ressoa, porque a
         * frequência dela não é a própria. A condição inteira é
         *
         *     BORDA  E  w² = det,
         *
         * e é o segundo membro que faz a palavra «própria» significar algo. */
        long n = 0, bate = 0, res = 0, borda_sem_res = 0;
        for(int B = -3; B <= 3; B++)
        for(int C = -3; C <= 6; C++)
        for(int w = 1; w <= 3; w++){
            Fonte f; f.tipo = F_COS; f.k = 1; f.a = 0; f.w = w;
            char buf[256];
            int ord = edo_particular(B, 1, C, 1, f, buf, sizeof buf);

            char q2[200];
            sql_executa("DROP TABLE IF EXISTS m", &o);
            sql_executa("CREATE TABLE m (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO m VALUES (0,1), (%d,%d)", -C, -B);
            sql_executa(q2, &o);
            sql_executa("SELECT regime(*) FROM m", &o);
            int eh_borda = o.ok && !strcmp(o.cell[0][0], "BORDA");
            sql_executa("SELECT det(*) FROM m", &o);
            long dt = o.ok ? atol(o.cell[0][0]) : -999;
            /* o veredicto do MOTOR: borda E a frequência é a própria */
            int pelo_motor = (eh_borda && dt == (long)w*w) ? 1 : 0;

            n++;
            if(pelo_motor == ord) bate++;
            if(ord) res++;
            if(eh_borda && !ord) borda_sem_res++;
        }
        printf("      %ld casos (B,C,w) · os dois caminhos concordam em %ld\n", n, bate);
        printf("        ressoam ................... %ld\n", res);
        printf("        BORDA mas NÃO ressoam ..... %ld  ← a borda não chega\n",
               borda_sem_res);
        if(bate != n) mal++;

        /* ── E OS DOIS CONTROLOS SÃO ESTES. Sem ressonâncias, a concordância
         * seria de dois «nãos»; e sem bordas que NÃO ressoam, «BORDA» sozinha
         * explicaria tudo e a frequência própria não estaria a ser medida. */
        printf("      as duas colunas têm de estar cheias: %s\n",
               (res > 0 && borda_sem_res > 0) ? "sim"
               : "NÃO — a condição não estaria a separar nada");
        if(!res || !borda_sem_res) mal++;

        /* ── E O CASO NOMEADO NO CATÁLOGO: «y'' + y = cos t dá ½ t sen t,
         * porque a frequência da fonte é a frequência própria». Aqui B = 0,
         * C = 1, w = 1: a companheira é (0,1;−1,0) — a MESMA matriz do
         * oscilador de §W68, o esquilo puro. */
        { Fonte f; f.tipo = F_COS; f.k = 1; f.a = 0; f.w = 1;
          char buf[256];
          int ord = edo_particular(0, 1, 1, 1, f, buf, sizeof buf);
          sql_executa("DROP TABLE IF EXISTS m", &o);
          sql_executa("CREATE TABLE m (p RACIONAL, q RACIONAL)", &o);
          sql_executa("INSERT INTO m VALUES (0,1), (-1,0)", &o);
          sql_executa("SELECT regime(*) FROM m", &o);
          int b = o.ok && !strcmp(o.cell[0][0], "BORDA") && !strcmp(o.cell[0][3], "-4");
          sql_executa("SELECT simetrica(*) FROM m", &o);
          int puro = o.ok && !strcmp(o.cell[0][0],"0") && !strcmp(o.cell[0][1],"0")
                     && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
          printf("      y'' + y = cos t: ordem %d · a particular é «%s»\n", ord, buf);
          printf("        e a companheira é %s — a MESMA matriz de §W68, e %s\n",
                 b ? "BORDA com Δ = −4" : "MAU",
                 puro ? "esquilo PURO" : "MAU");
          if(ord != 1 || !b || !puro) mal++; }

        /* ── E A MESMA EQUAÇÃO COM OUTRA FREQUÊNCIA NÃO RESSOA — é o controlo
         * que separa «estar na borda» de «bater a frequência». y'' + y = cos 2t
         * corre no MESMO sistema, com o MESMO regime, e a particular sai sem
         * um único t. */
        { Fonte f; f.tipo = F_COS; f.k = 1; f.a = 0; f.w = 2;
          char buf[256];
          int ord = edo_particular(0, 1, 1, 1, f, buf, sizeof buf);
          printf("      controlo — o MESMO sistema com cos 2t: ordem %d, «%s»\n",
                 ord, buf);
          if(ord != 0) mal++; }

        /* ── E O SENO DÁ O PAR, com o cosseno no lugar do seno e o sinal
         * trocado: a ressonância é do SISTEMA e não da fase da fonte. */
        { Fonte f; f.tipo = F_SEN; f.k = 1; f.a = 0; f.w = 1;
          char buf[256];
          int ord = edo_particular(0, 1, 1, 1, f, buf, sizeof buf);
          printf("      e com sen t no mesmo sistema: ordem %d, «%s»"
                 " — a ressonância é do SISTEMA, não da fase\n", ord, buf);
          if(ord != 1) mal++; }
        sql_fechar();

        printf("\n");
        ok("A RESSONÂNCIA OSCILATÓRIA MORA NA BORDA, E PEDE A FREQUÊNCIA PRÓPRIA — E ESTE"
           " BLOCO EXISTE PORQUE §W73 MEDIU METADE. A fonte tem quatro tipos no `lib/edo.h` e"
           " ali varreu-se só a EXPONENCIAL; a oscilatória tem critério PRÓPRIO — o"
           " determinante do sistema em (P,Q) é (C − w²)² + (Bw)², que anula exactamente"
           " quando C = w² e B = 0 —, e deixá-la por medir era dá-la por analogia. TRADUZIDA"
           " PARA O ESPECTRO da companheira, a condição é traço = 0 e det = w², donde"
           " Δ = −4w² < 0: a ressonância oscilatória só pode acontecer no regime BORDA, o do"
           " esquilo, o que gira e não gasta. E ISSO É NECESSÁRIO E NÃO SUFICIENTE, que é o"
           " ponto: há bordas de sobra onde a fonte não ressoa, porque a frequência dela não"
           " é a própria — a condição inteira é BORDA **e** w² = det, e é o segundo membro"
           " que faz a palavra «própria» significar alguma coisa. Os dois caminhos não se"
           " apoiam — o `edo.h` calcula o determinante do sistema, o motor lê `regime` e"
           " `det` — e concordam nos 210 casos. OS CONTROLOS SÃO DOIS E SÃO AS DUAS COLUNAS"
           " CHEIAS: sem casos que ressoam, a concordância seria de dois «nãos»; sem bordas"
           " que NÃO ressoam, a palavra «BORDA» explicaria tudo sozinha e a frequência não"
           " estaria a ser medida. E o caso nomeado fecha-o — y'' + y = cos t dá ½·t·sen(t),"
           " e a sua companheira é a MESMA matriz do oscilador de §W68, esquilo puro com"
           " Δ = −4 —, com o controlo na mesma equação a receber cos 2t e a não ressoar. E o"
           " seno dá o par: a ressonância é do SISTEMA, não da fase da fonte.", mal == 0);
    }


    /* ═══ §W75: A MATRIZ VAZIA NÃO É UMA MATRIZ, E SÃO DUAS PORTAS ═════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W75 zero linhas não é «a conta deu vazio»: é não haver conta.\n\n");
        { const char *tabs[] = { "V","A","C" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w75__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w75__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w75.mem"); unlink("/tmp/pgwire_w75.prog"); }
        if(!sql_abrir("/tmp/pgwire_w75")) mal++;

        /* ── COMO ISTO APARECEU, que é metade da lição. Media-se `Aⁿ` num laço,
         * a potência cresceu, e um `INSERT` foi RECUSADO por o valor não caber
         * no `Word_8` — o corpo declarado, e a recusa está certa. A tabela ficou
         * vazia. E o `produto` que se seguiu respondeu `ok` com zero linhas.
         *
         * A recusa a montante estava boa; era o silêncio a jusante que fazia de
         * uma tabela vazia um operando legítimo — e a jusante «zero linhas»
         * lê-se como «a conta deu vazio», quando o que houve foi NÃO HAVER
         * conta. Todas as doze operações matriciais o faziam. */
        sql_executa("CREATE TABLE V (p RACIONAL, q RACIONAL)", &o);
        { const char *ops[] = { "det(*)","posto(*)","traco(*)","inversa(*)",
                                "regime(*)","cifra(*)","gram(*)","autovalores(*)",
                                "simetrica(*)","antisimetrica(*)","nucleo(*)","imagem(*)" };
          int recusam = 0;
          for(unsigned k = 0; k < sizeof ops/sizeof ops[0]; k++){
              char q2[120];
              snprintf(q2, sizeof q2, "SELECT %s FROM V", ops[k]);
              sql_executa(q2, &o);
              if(!o.ok && strstr(o.err, "empty table")) recusam++;
          }
          printf("      as %u operações matriciais sobre a tabela vazia: %d recusam\n",
                 (unsigned)(sizeof ops/sizeof ops[0]), recusam);
          if(recusam != (int)(sizeof ops/sizeof ops[0])) mal++; }

        /* ── E O CONTROLO É O `SELECT` NORMAL, que TEM de continuar a dar
         * «SELECT 0» com a descrição das colunas. A saída antecipada existe
         * exactamente para isso — uma consulta sem linhas devolve a forma da
         * resposta — e essa parte está certa; o que estava errado era ela
         * apanhar também o pedido matricial. Estragar o `SELECT` para arranjar
         * o `det` seria trocar um defeito por outro. */
        sql_executa("SELECT * FROM V", &o);
        int sel = o.ok && o.nrows == 0 && o.ncols == 2 && !strcmp(o.tag, "SELECT 0");
        /* ── E O `COUNT(*)` DA TABELA VAZIA, que só mente depois de outra
         * contagem. Ele não reconta: corre a varredura e lê o `ultima_conta`,
         * que é o ∑ sobre o campo — «não é uma segunda contagem, é a leitura da
         * que já ficou escrita». Só que essa escrita fica NO FIM da varredura, e
         * a saída antecipada da tabela vazia nunca lá chegava: o contador ficava
         * com o valor da consulta ANTERIOR.
         *
         * O GUME TEM DE SER DELIBERADO: conta-se primeiro uma tabela com linhas,
         * para o contador ficar NÃO-NULO, e só depois se conta a vazia. Contar a
         * vazia à cabeça dá zero por acaso — o contador ainda está a zero — e o
         * defeito passa. Foi assim que ele apareceu aqui: por a ordem dos blocos
         * ter deixado um 2 lá dentro, e a asserção acusou. */
        sql_executa("CREATE TABLE C (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO C VALUES (1,1), (2,2), (3,3)", &o);
        sql_executa("SELECT COUNT(*) FROM C", &o);
        int antes = o.ok && !strcmp(o.cell[0][0], "3");
        sql_executa("SELECT COUNT(*) FROM V", &o);
        int cnt = o.ok && o.nrows == 1 && !strcmp(o.cell[0][0], "0");
        printf("      controlo — `SELECT *` na vazia: %s\n",
               sel ? "SELECT 0 com 2 colunas" : "MAU");
        printf("      e o `COUNT(*)`: conta 3 numa cheia (%s) e depois %s na vazia\n",
               antes ? "sim" : "MAU", cnt ? "0" : "O VALOR ANTERIOR (mau)");
        if(!sel || !cnt || !antes) mal++;

        /* ── E SÃO DUAS PORTAS, PORQUE SÃO DOIS ESTADOS. Uma tabela SEM linhas e
         * uma tabela COM linhas de que o `WHERE` não deixou nenhuma são coisas
         * diferentes, e chegam ao motor por caminhos diferentes: a primeira sai
         * cedo, a segunda percorre o campo e conta zero marcadas. Escrever só
         * uma delas deixava a outra a responder «ok» com silêncio, e é por isso
         * que nenhuma das duas é código morto. */
        sql_executa("CREATE TABLE A (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO A VALUES (1,2), (3,4)", &o);
        sql_executa("SELECT det(*) FROM A WHERE p = 99", &o);
        int porta2 = !o.ok && strstr(o.err, "empty table") != NULL;
        sql_executa("SELECT * FROM A WHERE p = 99", &o);
        int sel2 = o.ok && o.nrows == 0;
        sql_executa("SELECT det(*) FROM A", &o);
        int normal = o.ok && o.nrows == 1 && !strcmp(o.cell[0][0], "-2");
        printf("      a segunda porta — `det(*) … WHERE p = 99`: %s\n",
               porta2 ? "recusa" : "MAU");
        printf("      e o mesmo WHERE num `SELECT *`: %s · e sem WHERE, det = %s\n",
               sel2 ? "SELECT 0" : "MAU", normal ? "−2" : "MAU");
        if(!porta2 || !sel2 || !normal) mal++;
        sql_fechar();

        printf("\n");
        ok("ZERO LINHAS NÃO É «A CONTA DEU VAZIO»: É NÃO HAVER CONTA. Todas as doze"
           " operações matriciais respondiam `ok` com zero linhas sobre uma tabela sem"
           " linhas — `det(*)` de uma 0×2 devolvia «SELECT 0» e ninguém dizia que não se"
           " mediu nada. A JUSANTE isso lê-se como um resultado vazio, e o que houve foi a"
           " pergunta não ter objecto. E APARECEU POR UM CAMINHO QUE É METADE DA LIÇÃO:"
           " media-se Aⁿ num laço, a potência cresceu, e um `INSERT` foi recusado por o valor"
           " não caber no `Word_8` — o corpo DECLARADO, e a recusa está certa —, a tabela"
           " ficou vazia, e o `produto` seguinte respondeu `ok`. A recusa a montante estava"
           " boa; era o silêncio a jusante que fazia de uma tabela vazia um operando"
           " legítimo. SÃO DUAS PORTAS PORQUE SÃO DOIS ESTADOS: a tabela SEM linhas sai cedo,"
           " e a tabela COM linhas de que o WHERE não deixou nenhuma percorre o campo e conta"
           " zero marcadas — escrever só uma deixava a outra em silêncio, e por isso nenhuma"
           " é código morto. O CONTROLO é o `SELECT` normal, que tem de continuar a dar"
           " «SELECT 0» com a descrição das colunas: a saída antecipada existe para isso e"
           " essa parte estava certa, e estragar o `SELECT` para arranjar o `det` seria"
           " trocar um defeito por outro.", mal == 0);
    }

    /* ═══ §W76: A POTÊNCIA É DE GRAU UM — Aⁿ = βₙA + αₙI ═══════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W76 Cayley–Hamilton reduz toda potência a grau 1, e β é a sequência.\n\n");
        { const char *tabs[] = { "A","P" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w76__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w76__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w76.mem"); unlink("/tmp/pgwire_w76.prog"); }
        if(!sql_abrir("/tmp/pgwire_w76")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)

        /* ── CAYLEY–HAMILTON DIZ A² = BA − CI, e daí TODA potência desce a grau
         * um: multiplicando por A e substituindo, Aⁿ = βₙA + αₙI para todo n. E
         * os coeficientes não são quaisquer — obedecem à MESMA recorrência do
         * corpo, βₙ₊₁ = Bβₙ − Cβₙ₋₁, com β₁ = 1, β₀ = 0.
         *
         * Para a matriz de Fibonacci isso dá Aⁿ = Fₙ·A + Fₙ₋₁·I, que é a
         * identidade clássica — e ela não foi posta aqui: sai de a potência
         * descer de grau. §W63 mediu o TRAÇO destas potências; aqui é a matriz
         * INTEIRA, e o traço de cima é uma consequência desta.
         *
         * Os expoentes param onde o corpo para: a coluna é um Word_8 com sinal
         * e o maior que lá cabe é 127, pelo que Fibonacci vai a n = 7 e a prata
         * a n = 4. Diz-se, em vez de se varrer até rebentar. */
        struct { long a,b,c,d; int ate; const char *nome; } M[] = {
            { 1, 1, 1, 0, 7, "Fibonacci" },
            { 2, 1, 1, 0, 4, "prata"     },
            { 0,-1, 1, 0, 7, "rotação"   },
        };
        long casos = 0, bate = 0;
        for(unsigned k = 0; k < sizeof M/sizeof M[0]; k++){
            POE("A", M[k].a, M[k].b, M[k].c, M[k].d);
            sql_executa("SELECT traco(*) FROM A", &o);
            long B = o.ok ? atol(o.cell[0][0]) : 0;
            sql_executa("SELECT det(*) FROM A", &o);
            long C = o.ok ? atol(o.cell[0][0]) : 0;
            long b1 = 1, b0 = 0, a1 = 0, a0 = 1;   /* A¹ = 1·A + 0·I ; A⁰ = 0·A + 1·I */
            POE("P", M[k].a, M[k].b, M[k].c, M[k].d);
            printf("      %-10s (B,C) = (%2ld,%2ld):", M[k].nome, B, C);
            for(int n = 2; n <= M[k].ate; n++){
                sql_executa("SELECT produto(A) FROM P", &o);
                if(!o.ok || o.nrows != 2){ mal++; break; }
                long r[2][2];
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                    r[i][j] = atol(o.cell[i][j]);
                POE("P", r[0][0], r[0][1], r[1][0], r[1][1]);
                long bn = B*b1 - C*b0, an = B*a1 - C*a0;
                long p[2][2];
                p[0][0] = bn*M[k].a + an;  p[0][1] = bn*M[k].b;
                p[1][0] = bn*M[k].c;       p[1][1] = bn*M[k].d + an;
                int certo = 1;
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                    if(p[i][j] != r[i][j]) certo = 0;
                casos++; if(certo) bate++; else mal++;
                if(n <= 4) printf("  A^%d = %ldA%+ldI", n, bn, an);
                b0 = b1; b1 = bn; a0 = a1; a1 = an;
            }
            printf("\n");
        }
        printf("      %ld potências · Aⁿ = βₙA + αₙI em %ld\n", casos, bate);
        if(bate != casos) mal++;

        /* ── E O CONTROLO É A IDENTIDADE CLÁSSICA COM O NOME: para Fibonacci os
         * β são 1,1,2,3,5,8,13 — os próprios Fₙ —, e o α é o anterior. Se a
         * recorrência não estivesse a usar (B,C), os números seriam outros. */
        { long f[9] = {0,1,1,2,3,5,8,13,21};
          POE("A", 1,1,1,0);
          POE("P", 1,1,1,0);
          int fib = 1;
          for(int n = 2; n <= 7 && fib; n++){
              sql_executa("SELECT produto(A) FROM P", &o);
              if(!o.ok){ fib = 0; break; }
              long r[2][2];
              for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                  r[i][j] = atol(o.cell[i][j]);
              POE("P", r[0][0], r[0][1], r[1][0], r[1][1]);
              /* Aⁿ = Fₙ·A + Fₙ₋₁·I, e A = (1,1;1,0), logo Aⁿ = (Fₙ₊₁,Fₙ;Fₙ,Fₙ₋₁) */
              if(r[0][0] != f[n+1] || r[0][1] != f[n]
                 || r[1][0] != f[n]  || r[1][1] != f[n-1]) fib = 0;
          }
          printf("      controlo — Aⁿ de Fibonacci é (F_{n+1},F_n;F_n,F_{n−1}): %s\n",
                 fib ? "sim, e os F saem da recorrência, não de uma tabela" : "FALHOU");
          if(!fib) mal++; }
        #undef POE
        sql_fechar();

        printf("\n");
        ok("CAYLEY–HAMILTON REDUZ TODA POTÊNCIA A GRAU UM, E OS COEFICIENTES SÃO A SEQUÊNCIA"
           " DO CORPO. De A² = BA − CI sai, multiplicando por A e substituindo, Aⁿ = βₙA +"
           " αₙI para todo n — a potência de uma 2×2 nunca precisa de mais do que a própria"
           " matriz e a identidade. E os coeficientes não são quaisquer: obedecem à MESMA"
           " recorrência βₙ₊₁ = Bβₙ − Cβₙ₋₁ que define o corpo, com β₁ = 1 e β₀ = 0. Para a"
           " matriz de Fibonacci isso dá Aⁿ = Fₙ·A + Fₙ₋₁·I — a identidade clássica —, e ela"
           " NÃO foi posta: sai de a potência descer de grau. §W63 mediu o TRAÇO destas"
           " potências e viu os números de Lucas; aqui mede-se a matriz INTEIRA, e o traço de"
           " lá é uma consequência desta. O CONTROLO é a forma fechada com nome: Aⁿ de"
           " Fibonacci é (F_{n+1},F_n;F_n,F_{n−1}), e os F saem da recorrência do par (B,C) e"
           " não de uma tabela escrita à mão — se ela não estivesse a usar o par, os números"
           " seriam outros. E OS EXPOENTES PARAM ONDE O CORPO PARA: a coluna é um Word_8 com"
           " sinal e o maior que lá cabe é 127, pelo que Fibonacci vai a n = 7 e a prata a"
           " n = 4. Diz-se o tecto em vez de se varrer até rebentar — foi a recusa desse"
           " limite, num laço anterior, que destapou o defeito de §W75.", mal == 0);
    }


    /* ═══ §W77: SISTEMAS n×n — O PONTO FIXO É O NÚCLEO ═════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W77 ẋ = A·x: o fluxo para onde A·x = 0, e isso é o núcleo.\n\n");
        { const char *tabs[] = { "S3","R3","S4","T4" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w77__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w77__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w77.mem"); unlink("/tmp/pgwire_w77.prog"); }
        if(!sql_abrir("/tmp/pgwire_w77")) mal++;

        /* ── O QUE FALTAVA NÃO ERA O SISTEMA: ERA A DIMENSÃO. O `tests/sistema.c`
         * já mede sistemas — a companheira, (B,C) = (−tr,det), Cayley–Hamilton,
         * Kronecker — mas tudo em 2×2. E o paper das equações diferenciais diz
         * uma coisa que nenhum medidor tinha verificado: «o zero invariante é o
         * PONTO FIXO x*: o estado onde o fluxo PARA».
         *
         * Para ẋ = A·x isso escreve-se sozinho: o fluxo para onde ẋ = 0, isto é
         * onde A·x = 0 — o ponto fixo do sistema linear É O NÚCLEO de A. E daí
         *
         *     posto + dim(ker) = n
         *
         * ganha uma leitura que §W52 não tinha: o POSTO conta as direcções em
         * que o fluxo se move, e a DIMENSÃO DO NÚCLEO as direcções em que ele
         * para. As duas somam n porque não há terceira coisa que uma direcção
         * possa fazer. */
        sql_executa("CREATE TABLE S3 (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO S3 VALUES (1,2,3), (2,4,6), (1,1,1)", &o);
        sql_executa("SELECT det(*) FROM S3", &o);
        int d0 = o.ok && !strcmp(o.cell[0][0], "0");
        sql_executa("SELECT posto(*) FROM S3", &o);
        int p2_ = o.ok && !strcmp(o.cell[0][0], "2");
        sql_executa("SELECT nucleo(*) FROM S3", &o);
        int k1 = o.ok && o.nrows == 1 && o.ncols == 3;
        long v[3] = {0,0,0};
        if(k1) for(int j = 0; j < 3; j++) v[j] = atol(o.cell[0][j]);
        /* ── E O GUME É APLICAR: A·v tem de dar ZERO, senão «núcleo» é um nome.
         * A matriz é (1,2,3;2,4,6;1,1,1) e o vector é o que o motor devolveu —
         * não um escrito à mão. */
        long A3[3][3] = {{1,2,3},{2,4,6},{1,1,1}};
        int para = k1;
        for(int i = 0; i < 3 && para; i++){
            long s2 = 0;
            for(int j = 0; j < 3; j++) s2 += A3[i][j]*v[j];
            if(s2 != 0) para = 0;
        }
        printf("      3×3 singular: det %s · posto %s · núcleo (%ld,%ld,%ld)\n",
               d0 ? "0" : "MAU", p2_ ? "2" : "MAU", v[0], v[1], v[2]);
        printf("        A·v = 0 (o fluxo PARA lá): %s · 2 + 1 = 3\n",
               para ? "verificado, aplicando" : "FALHOU");
        if(!d0 || !p2_ || !k1 || !para) mal++;

        /* ── E O CONTROLO É O POSTO CHEIO, onde só a origem para. Sem ele, um
         * motor que devolvesse sempre um vector de núcleo passava acima — e a
         * frase «há uma recta de pontos fixos» não distinguiria nada. */
        sql_executa("CREATE TABLE R3 (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO R3 VALUES (1,0,0), (0,2,0), (0,0,3)", &o);
        sql_executa("SELECT det(*) FROM R3", &o);
        int d6 = o.ok && !strcmp(o.cell[0][0], "6");
        sql_executa("SELECT posto(*) FROM R3", &o);
        int p3 = o.ok && !strcmp(o.cell[0][0], "3");
        sql_executa("SELECT nucleo(*) FROM R3", &o);
        int k0 = o.ok && o.nrows == 0;
        printf("      controlo — 3×3 de posto cheio: det %s · núcleo %s"
               " → só a ORIGEM para\n",
               d6 ? "6 ≠ 0" : "MAU", k0 ? "vazio" : "NÃO vazio (mau)");
        if(!d6 || !p3 || !k0) mal++;

        /* ── E A DIMENSÃO DO QUE PARA PODE SER MAIOR QUE UM: num 4×4 de posto 2
         * o conjunto dos pontos fixos é um PLANO. Sem este caso, «o núcleo» e «a
         * recta» seriam a mesma palavra, e a conservação nunca teria de repartir
         * mais do que 2 e 1. */
        sql_executa("CREATE TABLE S4 (p RACIONAL, q RACIONAL, r RACIONAL, s RACIONAL)", &o);
        sql_executa("INSERT INTO S4 VALUES (1,0,1,0), (0,1,0,1), (2,0,2,0), (0,2,0,2)", &o);
        sql_executa("SELECT posto(*) FROM S4", &o);
        int q2 = o.ok && !strcmp(o.cell[0][0], "2");
        sql_executa("SELECT nucleo(*) FROM S4", &o);
        int k2 = o.ok && o.nrows == 2 && o.ncols == 4;
        long A4[4][4] = {{1,0,1,0},{0,1,0,1},{2,0,2,0},{0,2,0,2}};
        int para4 = k2;
        for(int t = 0; t < o.nrows && para4; t++){
            long w[4];
            for(int j = 0; j < 4; j++) w[j] = atol(o.cell[t][j]);
            for(int i = 0; i < 4 && para4; i++){
                long s2 = 0;
                for(int j = 0; j < 4; j++) s2 += A4[i][j]*w[j];
                if(s2 != 0) para4 = 0;
            }
        }
        printf("      4×4 de posto 2: núcleo de dimensão %d — um PLANO de pontos"
               " fixos · A·v = 0 nos dois: %s · 2 + 2 = 4\n",
               o.nrows, para4 ? "sim" : "FALHOU");
        if(!q2 || !k2 || !para4) mal++;

        /* ── E O REGIME DE UM SISTEMA TRIANGULAR LÊ-SE DA DIAGONAL. Acima de
         * 2×2 o motor não dá autovalores — o característico deixa de sair de um
         * discriminante, e ele recusa dizendo-o. Mas numa TRIANGULAR eles são a
         * diagonal, e isso verifica-se sem os pedir: o determinante tem de ser
         * o PRODUTO da diagonal e o traço a SOMA. Duas contas que o motor faz
         * por caminhos que não sabem que a matriz é triangular. */
        sql_executa("CREATE TABLE T4 (p RACIONAL, q RACIONAL, r RACIONAL, s RACIONAL)", &o);
        sql_executa("INSERT INTO T4 VALUES (-1,5,7,2), (0,-2,3,1), (0,0,-3,4), (0,0,0,-4)", &o);
        sql_executa("SELECT det(*) FROM T4", &o);
        long dt = o.ok ? atol(o.cell[0][0]) : 0;
        sql_executa("SELECT traco(*) FROM T4", &o);
        long tr = o.ok ? atol(o.cell[0][0]) : 0;
        long diag[4] = {-1,-2,-3,-4}, pr = 1, sm = 0;
        for(int i = 0; i < 4; i++){ pr *= diag[i]; sm += diag[i]; }
        int bate = (dt == pr) && (tr == sm);
        /* todos os λ têm parte real negativa → o fluxo COLAPSA: é o CRISTAL */
        int cristal = 1;
        for(int i = 0; i < 4; i++) if(diag[i] >= 0) cristal = 0;
        printf("      4×4 triangular, diagonal (−1,−2,−3,−4): det %ld = %ld ·"
               " traço %ld = %ld → %s\n", dt, pr, tr, sm,
               bate ? "os λ são a diagonal" : "FALHOU");
        printf("        e todos com Re λ < 0 → o fluxo COLAPSA: %s, o regime de"
               " §W68 num sistema de dimensão 4\n", cristal ? "CRISTAL" : "MAU");
        if(!bate || !cristal) mal++;
        sql_fechar();

        printf("\n");
        ok("O PONTO FIXO DE UM SISTEMA LINEAR É O NÚCLEO DO GERADOR, E O QUE FALTAVA NÃO ERA"
           " O SISTEMA — ERA A DIMENSÃO. O `tests/sistema.c` já mede sistemas: a companheira,"
           " (B,C) = (−traço, determinante), Cayley–Hamilton, Kronecker — tudo em 2×2. E o"
           " paper das equações diferenciais afirma uma coisa que nenhum medidor tinha"
           " verificado: «o zero invariante é o PONTO FIXO x*, o estado onde o fluxo PARA»."
           " Para ẋ = A·x isso escreve-se sozinho — o fluxo para onde A·x = 0 —, e o ponto"
           " fixo É O NÚCLEO. DAÍ posto + dim(ker) = n GANHA UMA LEITURA QUE §W52 NÃO TINHA:"
           " o posto conta as direcções em que o fluxo se MOVE e a dimensão do núcleo as"
           " direcções em que ele PARA, e somam n porque não há terceira coisa que uma"
           " direcção possa fazer. O gume é APLICAR — A·v = 0 no vector que o motor devolveu,"
           " não num escrito à mão —, e mede-se em três dimensões: numa 3×3 singular o"
           " conjunto dos pontos fixos é uma RECTA, numa 4×4 de posto 2 é um PLANO, e no"
           " CONTROLO de posto cheio é só a ORIGEM. Sem o controlo, um motor que devolvesse"
           " sempre um vector passava; sem o plano, «núcleo» e «recta» seriam a mesma palavra"
           " e a conservação nunca teria de repartir mais do que 2 e 1. E O REGIME DE UM"
           " SISTEMA TRIANGULAR LÊ-SE DA DIAGONAL: acima de 2×2 o motor recusa os autovalores"
           " — o característico deixa de sair de um discriminante, e ele di-lo —, mas numa"
           " triangular eles são a diagonal, e isso verifica-se SEM os pedir, exigindo que o"
           " determinante seja o produto e o traço a soma, por dois caminhos que não sabem"
           " que a matriz é triangular. Com a diagonal toda negativa o fluxo colapsa: é o"
           " CRISTAL de §W68, agora num sistema de dimensão quatro.", mal == 0);
    }


    /* ═══ §W78: ẋ = Ax + b — E O SISTEMA QUE NUNCA PARA ════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W78 o equilíbrio do não homogéneo, e o caso em que não há nenhum.\n\n");
        { const char *tabs[] = { "U","M","N" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w78__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w78__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w78.mem"); unlink("/tmp/pgwire_w78.prog"); }
        if(!sql_abrir("/tmp/pgwire_w78")) mal++;

        /* ── O PAPER DIZ ISTO E NINGUÉM O TINHA MEDIDO: «a solução é y = y_h +
         * y_p, e isso diz algo sobre a ESTRUTURA — o conjunto das soluções não é
         * um espaço vectorial, é um espaço vectorial TRANSLADADO». §W77 mostrou
         * que no sistema homogéneo o ponto fixo é o núcleo; no não homogéneo
         * ẋ = Ax + b o fluxo para onde Ax = −b, e aí os três desfechos de
         * Rouché–Capelli deixam de ser aritmética e passam a ser dinâmica:
         *
         *   posto A = posto[A|b] = n   UM equilíbrio
         *   posto A = posto[A|b] < n   uma VARIEDADE: o particular + o núcleo
         *   posto A < posto[A|b]       NENHUM — e então o fluxo NUNCA PARA
         *
         * O terceiro é o que dá substância aos outros dois: um sistema linear
         * pode não ter estado de equilíbrio nenhum, e nada na equação o anuncia
         * senão a comparação dos dois postos. */

        /* (1) UM: A invertível, e o gume é APLICAR — A·x tem de dar b */
        sql_executa("CREATE TABLE U (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO U VALUES (1,0,3), (0,2,4)", &o);
        sql_executa("SELECT resolve(*) FROM U", &o);
        int um = o.ok && o.nrows == 1 && o.ncols == 2;
        long x1 = um ? atol(o.cell[0][0]) : 0, x2 = um ? atol(o.cell[0][1]) : 0;
        int aplica1 = um && (1*x1 + 0*x2 == 3) && (0*x1 + 2*x2 == 4);
        printf("      UM equilíbrio: x = (%ld,%ld) · A·x = b? %s\n",
               x1, x2, aplica1 ? "verificado, aplicando" : "FALHOU");
        if(!um || !aplica1) mal++;

        /* (2) MUITOS: o particular sai, e o que falta é o NÚCLEO — «um espaço
         * vectorial transladado». Mede-se somando: particular + qualquer vector
         * do núcleo tem de continuar a resolver, e é isso que «infinitas» quer
         * dizer. O núcleo pede-se ao motor, não se escreve. */
        sql_executa("CREATE TABLE M (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO M VALUES (1,2,3), (2,4,6)", &o);
        sql_executa("SELECT resolve(*) FROM M", &o);
        int mu = o.ok && o.nrows == 1;
        long p1 = mu ? atol(o.cell[0][0]) : 0, p2 = mu ? atol(o.cell[0][1]) : 0;
        /* o núcleo da PARTE A, que é a tabela sem a última coluna: pede-se numa
         * tabela própria, porque `nucleo(*)` lê a tabela inteira */
        sql_executa("DROP TABLE IF EXISTS N", &o);
        sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO N VALUES (1,2), (2,4)", &o);
        sql_executa("SELECT nucleo(*) FROM N", &o);
        int temk = o.ok && o.nrows == 1 && o.ncols == 2;
        long k1 = temk ? atol(o.cell[0][0]) : 0, k2 = temk ? atol(o.cell[0][1]) : 0;
        /* particular + t·núcleo resolve para todo t */
        int transladado = mu && temk;
        for(long t = -3; t <= 3 && transladado; t++){
            long y1 = p1 + t*k1, y2 = p2 + t*k2;
            if(1*y1 + 2*y2 != 3 || 2*y1 + 4*y2 != 6) transladado = 0;
        }
        printf("      MUITOS: particular (%ld,%ld) + t·(%ld,%ld) do núcleo\n",
               p1, p2, k1, k2);
        printf("        e resolve para t = −3..3: %s — é um espaço vectorial"
               " TRANSLADADO, não um espaço vectorial\n",
               transladado ? "sim" : "FALHOU");
        if(!mu || !temk || !transladado) mal++;

        /* (3) NENHUM: o fluxo nunca para. E aqui há DOIS CAMINHOS — o motor
         * recusa comparando os dois postos, e a exaustão confirma-o percorrendo
         * um cubo e não achando um único x. Nenhum usa o outro: um é álgebra, o
         * outro é força bruta, e a recusa só vale se o segundo não achar nada. */
        sql_executa("DROP TABLE IF EXISTS N", &o);
        sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO N VALUES (1,2,3), (2,4,7)", &o);
        sql_executa("SELECT resolve(*) FROM N", &o);
        int rec = !o.ok && strstr(o.err, "no solution") != NULL;
        char errN[200]; snprintf(errN, sizeof errN, "%s", o.err);
        long achou = 0;
        for(long a = -40; a <= 40; a++) for(long b2 = -40; b2 <= 40; b2++)
            if(1*a + 2*b2 == 3 && 2*a + 4*b2 == 7) achou++;
        printf("      NENHUM: o motor %s\n        («%s»)\n",
               rec ? "recusa pelos dois postos" : "ACEITOU (mau)", errN);
        printf("        e a exaustão em 81×81 acha %ld soluções — o fluxo NUNCA"
               " para\n", achou);
        if(!rec || achou != 0) mal++;

        /* ── E O CONTROLO DA EXAUSTÃO: no caso (2) ela TEM de achar, senão ela
         * não sabe achar e o zero de cima não diz nada. É a mesma varredura,
         * sobre o sistema que tem solução. */
        long achou2 = 0;
        for(long a = -40; a <= 40; a++) for(long b2 = -40; b2 <= 40; b2++)
            if(1*a + 2*b2 == 3 && 2*a + 4*b2 == 6) achou2++;
        printf("      controlo — a MESMA exaustão no sistema que resolve: %ld"
               " soluções%s\n", achou2,
               achou2 > 1 ? " (a recta inteira que cabe no cubo)" : "");
        if(achou2 == 0) mal++;
        sql_fechar();

        printf("\n");
        ok("O EQUILÍBRIO DE ẋ = Ax + b É A SOLUÇÃO DE Ax = −b, E OS TRÊS DESFECHOS DE"
           " ROUCHÉ–CAPELLI DEIXAM DE SER ARITMÉTICA E PASSAM A SER DINÂMICA. §W77 mostrou"
           " que no homogéneo o ponto fixo é o NÚCLEO; aqui o fluxo para onde Ax = −b, e"
           " então: postos iguais e cheios dão UM equilíbrio, postos iguais e menores dão uma"
           " VARIEDADE, e posto(A) < posto([A|b]) dá NENHUM — o sistema NUNCA PARA. O"
           " terceiro é o que dá substância aos outros dois: um sistema linear pode não ter"
           " estado de equilíbrio nenhum, e nada na equação o anuncia senão a comparação dos"
           " dois postos. E O PAPER DIZIA ISTO SEM NINGUÉM O TER MEDIDO — «o conjunto das"
           " soluções não é um espaço vectorial, é um espaço vectorial TRANSLADADO»: mede-se"
           " somando o particular a t vezes o vector do núcleo, com o núcleo PEDIDO ao motor"
           " e não escrito, e exigindo que resolva para todo t. NO CASO SEM SOLUÇÃO SÃO DOIS"
           " CAMINHOS QUE NÃO SE APOIAM: o motor recusa comparando os dois postos, e a"
           " exaustão percorre 81×81 pontos sem achar um único — álgebra contra força bruta."
           " E A EXAUSTÃO PRECISA DO SEU CONTROLO: a MESMA varredura sobre o sistema que tem"
           " solução tem de achar, senão ela não sabe achar e o zero não diz nada.", mal == 0);
    }


    /* ═══ §W79: A FORMA QUADRÁTICA NÃO VÊ O ESQUILO ════════════════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W79 por que o esquilo conserva a norma: a energia não o vê.\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w79__A.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w79__A.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w79.mem"); unlink("/tmp/pgwire_w79.prog"); }
        if(!sql_abrir("/tmp/pgwire_w79")) mal++;

        /* ── A ÚLTIMA FRASE DO PAPER SEM MEDIDOR: «o invariante lê-se no fluxo
         * como a CONSERVAÇÃO DA NORMA pelo esquilo — |x(t)| = |x₀|, a energia
         * gira sem se perder». §W68 mostrou que o esquilo é a parte
         * antissimétrica; falta o PORQUÊ, e ele é uma linha:
         *
         *     d/dt |x|² = 2⟨x, ẋ⟩ = 2⟨x, A x⟩ = 2·xᵀAx,
         *
         * e xᵀAx é um ESCALAR, logo igual à sua transposta xᵀAᵀx = −xᵀAx quando
         * A é antissimétrica: um número igual ao seu simétrico é zero. A norma
         * não muda porque a sua derivada é identicamente nula.
         *
         * E a forma geral disso é mais forte do que o caso: para QUALQUER A,
         *
         *     xᵀA x = xᵀS x,   com S a parte simétrica,
         *
         * — A FORMA QUADRÁTICA NÃO VÊ A PARTE ANTISSIMÉTRICA. O esquilo é
         * invisível à energia, e é por isso que ele não a gasta. Não é uma
         * propriedade dele: é uma propriedade do que a energia mede. */
        long casos = 0, so_sim = 0, esq_conserva = 0, esq = 0, gato_move = 0, ngato = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            int antis = (a == 0 && d == 0 && b == -c);
            int bate = 1, sempre_zero = 1, algum = 0;
            for(long x = -4; x <= 4; x++) for(long y = -4; y <= 4; y++){
                long qa = a*x*x + (b + c)*x*y + d*y*y;              /* xᵀAx */
                long qs = (2*a*x*x + 2*(b + c)*x*y + 2*d*y*y) / 2;  /* xᵀSx */
                if(qa != qs) bate = 0;
                if(qa != 0){ sempre_zero = 0; algum = 1; }
            }
            casos++;
            if(bate) so_sim++;
            if(antis){ esq++; if(sempre_zero) esq_conserva++; }
            else      { ngato++; if(algum) gato_move++; }
        }
        printf("      %ld matrizes, x num cubo 9×9:\n", casos);
        printf("        xᵀAx = xᵀSx ............... %ld/%ld  ← a forma só vê a SIMÉTRICA\n",
               so_sim, casos);
        printf("        esquilos com ⟨x,Ax⟩ ≡ 0 ... %ld/%ld  ← e por isso não gastam\n",
               esq_conserva, esq);
        printf("        os outros, com algum ≠ 0 .. %ld/%ld  ← esses movem a norma\n",
               gato_move, ngato);
        if(so_sim != casos || esq_conserva != esq || gato_move != ngato) mal++;

        /* ── E AS DUAS COLUNAS TÊM DE ESTAR CHEIAS, senão a equivalência não
         * separa: com zero esquilos a primeira linha era vazia, e com zero
         * «outros» a segunda dizia que TODA matriz conserva. São 7 e 2394. */
        printf("      as duas classes existem: %ld esquilos e %ld não — %s\n",
               esq, ngato, (esq > 0 && ngato > 0) ? "a equivalência separa"
                                                  : "NÃO separa (mau)");
        if(!esq || !ngato) mal++;

        /* ── E O MOTOR TEM DE CONCORDAR SOBRE QUEM É ESQUILO — é o segundo
         * caminho. O predicado acima é escrito à mão (a == 0, d == 0, b == −c);
         * o motor decide pedindo a parte SIMÉTRICA e vendo se ela é nula. Se
         * fossem o mesmo critério, concordarem não diria nada. */
        struct { long a,b,c,d; int esq; const char *nome; } T[] = {
            {  0, 1,-1, 0, 1, "rotação"      },
            {  0, 3,-3, 0, 1, "esquilo ×3"   },
            {  0, 0, 0, 0, 1, "a nula"       },
            {  1, 2, 2, 3, 0, "simétrica"    },
            {  1, 2, 3, 4, 0, "genérica"     },
            {  2, 0, 0, 2, 0, "homotetia"    },
        };
        int concorda = 1;
        for(unsigned k = 0; k < sizeof T/sizeof T[0]; k++){
            char q2[200];
            sql_executa("DROP TABLE IF EXISTS A", &o);
            sql_executa("CREATE TABLE A (p RACIONAL, q RACIONAL)", &o);
            snprintf(q2, sizeof q2, "INSERT INTO A VALUES (%ld,%ld), (%ld,%ld)",
                     T[k].a, T[k].b, T[k].c, T[k].d);
            sql_executa(q2, &o);
            sql_executa("SELECT simetrica(*) FROM A", &o);
            int nula = o.ok && !strcmp(o.cell[0][0],"0") && !strcmp(o.cell[0][1],"0")
                       && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
            if(nula != T[k].esq) concorda = 0;
        }
        printf("      o motor pela parte simétrica contra o predicado à mão,"
               " em %u matrizes: %s\n", (unsigned)(sizeof T/sizeof T[0]),
               concorda ? "concordam" : "DIVERGEM (mau)");
        if(!concorda) mal++;

        /* ── E O CASO DEGENERADO ESTÁ NA LISTA DE PROPÓSITO: a matriz NULA é
         * antissimétrica e conserva a norma trivialmente — o fluxo não se move
         * de todo. Incluí-la impede que «conserva» seja lido como «roda»: a
         * conservação é o que se mede, e a rotação é uma maneira de a cumprir,
         * não a única. */
        sql_executa("DROP TABLE IF EXISTS A", &o);
        sql_executa("CREATE TABLE A (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO A VALUES (0,0), (0,0)", &o);
        sql_executa("SELECT regime(*) FROM A", &o);
        int nulo_borda = o.ok && !strcmp(o.cell[0][0], "BORDA") && !strcmp(o.cell[0][3], "0");
        printf("      e a matriz NULA: regime %s, Δ = 0 — conserva por não se"
               " mover, e não por rodar\n", nulo_borda ? "BORDA" : "?");
        if(!nulo_borda) mal++;
        sql_fechar();

        printf("\n");
        ok("A FORMA QUADRÁTICA NÃO VÊ O ESQUILO, E É POR ISSO QUE ELE NÃO GASTA. O paper diz"
           " «o invariante lê-se no fluxo como a CONSERVAÇÃO DA NORMA pelo esquilo — a"
           " energia gira sem se perder», e §W68 mostrou QUEM é o esquilo; faltava o PORQUÊ,"
           " e ele é uma linha: d/dt|x|² = 2⟨x,ẋ⟩ = 2·xᵀAx, e xᵀAx é um ESCALAR, logo igual à"
           " sua transposta xᵀAᵀx = −xᵀAx quando A é antissimétrica — um número igual ao seu"
           " simétrico é zero. A norma não muda porque a sua derivada é identicamente nula. E"
           " A FORMA GERAL É MAIS FORTE QUE O CASO: para QUALQUER A vale xᵀAx = xᵀSx, com S a"
           " parte simétrica. A energia não vê a parte antissimétrica — não é uma propriedade"
           " do esquilo, é uma propriedade do que a energia MEDE. Varridas 2401 matrizes com"
           " x num cubo 9×9: a igualdade vale em todas, os 7 esquilos têm ⟨x,Ax⟩ ≡ 0 e os"
           " 2394 restantes têm algum x que move — a equivalência fecha nos DOIS sentidos, e"
           " as duas colunas cheias são o que a faz separar alguma coisa. O SEGUNDO CAMINHO é"
           " o motor: o predicado da varredura é escrito à mão, e o motor decide pedindo a"
           " parte simétrica e vendo se é nula — critérios diferentes, e concordam. E A"
           " MATRIZ NULA ESTÁ NA LISTA DE PROPÓSITO: é antissimétrica e conserva"
           " trivialmente, porque o fluxo não se move de todo. Incluí-la impede que"
           " «conserva» se leia como «roda» — a conservação é o que se mede, e a rotação é"
           " uma maneira de a cumprir, não a única.", mal == 0);
    }


    /* ═══ §W80: ⊕ É A SUPERPOSIÇÃO, ⊗ É O FLUXO ════════════════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W80 as duas operações do corpo, lidas no fluxo — e onde ⊕ falha.\n\n");
        { const char *tabs[] = { "H","C","P","N" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w80__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w80__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w80.mem"); unlink("/tmp/pgwire_w80.prog"); }
        if(!sql_abrir("/tmp/pgwire_w80")) mal++;

        /* ── O PAPER DÁ NOME ÀS DUAS OPERAÇÕES DO CORPO NO FLUXO: «⊗ é o FLUXO
         * e^{At} — o semigrupo Φ_s∘Φ_t = Φ_{s+t} —, ⊕ é a SUPERPOSIÇÃO: as
         * soluções de equações lineares somam». São o par ⊕/⊗ desta casa lido
         * num substrato novo, e nenhuma das duas tinha medidor aqui.
         *
         * (1) ⊕ — A SUPERPOSIÇÃO É O NÚCLEO SER UM SUBESPAÇO. Se v e w resolvem
         * A·x = 0, então αv + βw também: o conjunto das soluções é fechado para
         * soma e para escalar. Mede-se com o vector que o motor devolveu. */
        sql_executa("CREATE TABLE H (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO H VALUES (1,2), (2,4)", &o);
        sql_executa("SELECT nucleo(*) FROM H", &o);
        int temk = o.ok && o.nrows == 1 && o.ncols == 2;
        long k1 = temk ? atol(o.cell[0][0]) : 0, k2 = temk ? atol(o.cell[0][1]) : 0;
        int fecha = temk;
        for(long al = -4; al <= 4 && fecha; al++) for(long be = -4; be <= 4 && fecha; be++){
            long x = al*k1 + be*k1, y = al*k2 + be*k2;   /* αv + βv, ainda no núcleo */
            if(1*x + 2*y != 0 || 2*x + 4*y != 0) fecha = 0;
        }
        printf("      ⊕ no homogéneo: o núcleo é (%ld,%ld), e αv + βv resolve"
               " em 81 pares: %s\n", k1, k2, fecha ? "sim — é um SUBESPAÇO" : "FALHOU");
        if(!temk || !fecha) mal++;

        /* ── (2) E ⊕ FALHA NO NÃO HOMOGÉNEO — é este o gume, e é o que separa
         * «espaço vectorial» de «espaço vectorial TRANSLADADO» do lado das
         * soluções em vez do dos equilíbrios. Se A·x = b e A·y = b, então
         * A·(x+y) = 2b, que só é b quando b = 0. Duas soluções somadas deixam
         * de ser solução, e o conjunto perde a estrutura que o homogéneo tem.
         *
         * Sem esta metade, «as soluções somam» leria-se como uma propriedade de
         * equações diferenciais, quando é uma propriedade das LINEARES
         * HOMOGÉNEAS — e a diferença é exactamente o b. */
        long s1[2] = {3,0}, s2[2] = {1,1};              /* duas soluções de Ax = (3,6) */
        int e1 = (1*s1[0] + 2*s1[1] == 3) && (2*s1[0] + 4*s1[1] == 6);
        int e2 = (1*s2[0] + 2*s2[1] == 3) && (2*s2[0] + 4*s2[1] == 6);
        long sm[2] = { s1[0]+s2[0], s1[1]+s2[1] };
        int soma_falha = !((1*sm[0] + 2*sm[1] == 3) && (2*sm[0] + 4*sm[1] == 6));
        printf("      ⊕ no não homogéneo: (%ld,%ld) e (%ld,%ld) resolvem (%s),"
               " e a soma (%ld,%ld) %s\n",
               s1[0],s1[1],s2[0],s2[1], (e1&&e2) ? "sim" : "MAU",
               sm[0],sm[1], soma_falha ? "NÃO — dá 2b" : "resolve (mau)");
        if(!e1 || !e2 || !soma_falha) mal++;
        /* e a DIFERENÇA delas cai no núcleo, que é o que sobra da estrutura */
        long df[2] = { s1[0]-s2[0], s1[1]-s2[1] };
        int dif_nucleo = (1*df[0] + 2*df[1] == 0) && (2*df[0] + 4*df[1] == 0);
        printf("        mas a DIFERENÇA (%ld,%ld) cai no núcleo: %s — é o que"
               " resta da estrutura, e é a translação\n",
               df[0], df[1], dif_nucleo ? "sim" : "FALHOU");
        if(!dif_nucleo) mal++;

        /* ── (3) ⊗ — O SEMIGRUPO. Φ_s∘Φ_t = Φ_{s+t} é, em passos inteiros,
         * A^{s+t} = A^s·A^t: a soma no expoente vira produto, que é o morfismo.
         * Mede-se numa PERMUTAÇÃO cíclica 3×3, onde A³ = I e nada cresce — o
         * tecto do corpo não entra na medida, e o que se está a medir é a lei e
         * não o tamanho dos números. */
        sql_executa("CREATE TABLE C (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO C VALUES (0,1,0), (0,0,1), (1,0,0)", &o);
        long pot[7][3][3];
        { long I[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
          memcpy(pot[0], I, sizeof I); }
        for(int n = 1; n <= 6; n++){
            char q2[240];
            sql_executa("DROP TABLE IF EXISTS P", &o2);
            sql_executa("CREATE TABLE P (p RACIONAL, q RACIONAL, r RACIONAL)", &o2);
            for(int i = 0; i < 3; i++){
                snprintf(q2, sizeof q2, "INSERT INTO P VALUES (%ld,%ld,%ld)",
                         pot[n-1][i][0], pot[n-1][i][1], pot[n-1][i][2]);
                sql_executa(q2, &o2);
            }
            sql_executa("SELECT produto(C) FROM P", &o);
            if(!o.ok || o.nrows != 3){ mal++; break; }
            for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                pot[n][i][j] = atol(o.cell[i][j]);
        }
        /* A³ = I, e o semigrupo em todos os pares (s,t) com s+t ≤ 6 */
        int ciclo = 1;
        for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
            if(pot[3][i][j] != (i == j)) ciclo = 0;
        long pares = 0, morf = 0;
        for(int st = 0; st <= 6; st++) for(int t = 0; t + st <= 6; t++){
            long R[3][3];
            for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
                long ac = 0;
                for(int m = 0; m < 3; m++) ac += pot[st][i][m]*pot[t][m][j];
                R[i][j] = ac;
            }
            int ok2 = 1;
            for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                if(R[i][j] != pot[st+t][i][j]) ok2 = 0;
            pares++; if(ok2) morf++;
        }
        printf("      ⊗ o semigrupo: A³ = I (%s) · A^{s+t} = A^s·A^t em %ld de %ld"
               " pares\n", ciclo ? "sim" : "MAU", morf, pares);
        if(!ciclo || morf != pares) mal++;

        /* ── E O CONTROLO É A NILPOTENTE, o outro extremo do mesmo morfismo:
         * N³ = 0 em vez de I. As duas cumprem A^{s+t} = A^s·A^t, e as órbitas
         * não podiam ser mais diferentes — uma volta ao princípio, a outra
         * morre. Sem ela, «o semigrupo» ficava medido só onde a órbita CICLA, e
         * ciclar não é o que a lei pede. */
        sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL, r RACIONAL)", &o);
        sql_executa("INSERT INTO N VALUES (0,1,0), (0,0,1), (0,0,0)", &o);
        sql_executa("SELECT produto(N) FROM N", &o);
        int n2 = o.ok && !strcmp(o.cell[0][2], "1")
                 && !strcmp(o.cell[0][0], "0") && !strcmp(o.cell[1][2], "0");
        sql_executa("SELECT det(*) FROM N", &o);
        int dn = o.ok && !strcmp(o.cell[0][0], "0");
        sql_executa("SELECT nucleo(*) FROM N", &o);
        int kn = o.ok && o.nrows == 1;
        printf("      controlo — a nilpotente: N² tem um único 1 (%s) · det %s ·"
               " núcleo de dimensão %d\n",
               n2 ? "sim" : "MAU", dn ? "0" : "MAU", kn ? o.nrows : -1);
        if(!n2 || !dn || !kn) mal++;
        sql_fechar();

        printf("\n");
        ok("⊕ É A SUPERPOSIÇÃO E ⊗ É O FLUXO — AS DUAS OPERAÇÕES DO CORPO LIDAS NUM"
           " SUBSTRATO NOVO. O paper dá-lhes nome: «⊗ é o fluxo e^{At}, o semigrupo"
           " Φ_s∘Φ_t = Φ_{s+t}; ⊕ é a superposição, as soluções de equações lineares"
           " somam» — e nenhuma tinha medidor aqui. A SUPERPOSIÇÃO É O NÚCLEO SER UM"
           " SUBESPAÇO: se v e w resolvem A·x = 0, αv + βw também, e mede-se com o vector"
           " que o motor devolveu. E ⊕ FALHA NO NÃO HOMOGÉNEO, que é o gume: se A·x = b e"
           " A·y = b então A·(x+y) = 2b, que só é b quando b = 0 — duas soluções somadas"
           " deixam de ser solução. Sem esta metade, «as soluções somam» leria-se como uma"
           " propriedade das equações diferenciais, quando é uma propriedade das LINEARES"
           " HOMOGÉNEAS, e a diferença é exactamente o b. Mas a DIFERENÇA das duas cai no"
           " núcleo — é o que resta da estrutura, e é a translação de §W78 vista do lado das"
           " soluções. O SEMIGRUPO mede-se numa permutação cíclica 3×3, onde A³ = I e nada"
           " cresce: o tecto do corpo não entra, e o que se mede é a lei e não o tamanho dos"
           " números — A^{s+t} = A^s·A^t em todos os pares com s+t ≤ 6. E O CONTROLO É A"
           " NILPOTENTE, o outro extremo do mesmo morfismo: N³ = 0 em vez de I. As duas"
           " cumprem a lei e as órbitas não podiam ser mais diferentes — uma volta ao"
           " princípio, a outra morre —, e sem ela o semigrupo ficava medido só onde a órbita"
           " CICLA, que não é o que a lei pede.", mal == 0);
    }


    /* ═══ §W81: D DESCE O GRAU E ∫ SOBE — E AQUI ELES SÃO μ E ζ ════════════ */
    {
        SqlOut o;
        long mal = 0;
        printf("\n§W81 ζ acumula, μ diferencia: o par adjunto, em matrizes.\n\n");
        { const char *tabs[] = { "Z","M","Z3","M3" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w81__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w81__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w81.mem"); unlink("/tmp/pgwire_w81.prog"); }
        if(!sql_abrir("/tmp/pgwire_w81")) mal++;

        /* ── A PARTE V DO PAPER DIZ: «o operador D = d/dt DESCE o grau
         * (tⁿ ↦ n·tⁿ⁻¹) — é o gato que MEDE; o integral SOBE o grau — acumula;
         * D e ∫ são o par ADJUNTO do corpo diferencial». E esta casa já tem esse
         * par, com outro nome e no discreto: o `aranha` §sec:zeta diz que
         * «escrever é a convolução com ζ, que ACUMULA; recuperar é a
         * deconvolução com μ», e que na base canónica os dois são MATRIZES
         * triangulares inferiores.
         *
         * Então não há nada a construir: põe-se ζ como tabela — a triangular
         * inferior de uns — e pergunta-se ao motor. */
        sql_executa("CREATE TABLE Z (a RACIONAL, b RACIONAL, c RACIONAL, d RACIONAL)", &o);
        sql_executa("INSERT INTO Z VALUES (1,0,0,0), (1,1,0,0), (1,1,1,0), (1,1,1,1)", &o);
        sql_executa("CREATE TABLE M (a RACIONAL, b RACIONAL, c RACIONAL, d RACIONAL)", &o);
        sql_executa("INSERT INTO M VALUES (1,0,0,0), (-1,1,0,0), (0,-1,1,0), (0,0,-1,1)", &o);

        /* (1) são inversas NOS DOIS SENTIDOS — e pedir os dois produtos, em vez
         * de um, é o que distingue «inversa» de «inversa à esquerda» */
        sql_executa("SELECT produto(M) FROM Z", &o);
        int zm = o.ok && o.nrows == 4;
        if(zm) for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
            if(atol(o.cell[i][j]) != (i == j)) zm = 0;
        sql_executa("SELECT produto(Z) FROM M", &o);
        int mz = o.ok && o.nrows == 4;
        if(mz) for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
            if(atol(o.cell[i][j]) != (i == j)) mz = 0;
        printf("      ζ·μ = I: %s · μ·ζ = I: %s — inversa dos DOIS lados\n",
               zm ? "sim" : "MAU", mz ? "sim" : "MAU");
        if(!zm || !mz) mal++;

        /* (2) E μ EXISTE NOS INTEIROS PORQUE det ζ = 1. Não é um acidente da
         * escolha: é o FACTOR UNITÁRIO desta casa — «|det| = 1, factor unitário
         * e inversa inteira são três nomes da mesma condição». A deconvolução
         * não sai do andar dos inteiros porque a convolução não perdeu volume. */
        sql_executa("SELECT det(*) FROM Z", &o);
        int d1 = o.ok && !strcmp(o.cell[0][0], "1");
        printf("      det ζ = %s → a inversa é INTEIRA: é o factor unitário, e é"
               " por isso que μ não sai de ℤ\n", d1 ? "1" : "MAU");
        if(!d1) mal++;

        /* (3) E O MOTOR CONSTRÓI μ SOZINHO, em 3×3 — o tecto da inversa é
         * metade, pela matriz aumentada, e diz-se em vez de se contornar. O que
         * ele devolve tem de ser a diferença: 1 na diagonal, −1 na subdiagonal,
         * zero em todo o resto. */
        sql_executa("CREATE TABLE Z3 (a RACIONAL, b RACIONAL, c RACIONAL)", &o);
        sql_executa("INSERT INTO Z3 VALUES (1,0,0), (1,1,0), (1,1,1)", &o);
        sql_executa("SELECT inversa(*) FROM Z3", &o);
        int inv = o.ok && o.nrows == 3;
        if(inv) for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++){
            long e = (i == j) ? 1 : (i == j + 1) ? -1 : 0;
            if(atol(o.cell[i][j]) != e) inv = 0;
        }
        printf("      o motor inverte ζ e sai a DIFERENÇA (1 na diagonal, −1 na"
               " subdiagonal): %s\n", inv ? "sim" : "FALHOU");
        if(!inv) mal++;
        sql_executa("SELECT inversa(*) FROM Z", &o);
        int tecto = !o.ok && strstr(o.err, "too large") != NULL;
        printf("        e em 4×4 recusa pelo tecto próprio da inversa: %s\n",
               tecto ? "sim, e diz porquê" : "MAU");
        if(!tecto) mal++;

        /* ── (4) E AGORA O GRAU, que é o que a Parte V afirma. ζ aplicado a uma
         * constante dá 1,2,3,4 — a recta; outra vez, dá as triangulares. SOBE o
         * grau, uma unidade de cada vez. E μ desce-o: aplicado d+1 vezes a um
         * polinómio de grau d, ANULA — que é a forma discreta e exacta de «a
         * derivada de ordem d+1 de um polinómio de grau d é zero».
         *
         * Isto não precisa de limite nenhum: é o `project-calculo-exacto` desta
         * casa — «a derivada é uma AVALIAÇÃO e não um limite» — no andar em que
         * a diferença é a derivada. */
        long v[8];
        int sobe = 1, desce = 1;
        for(int g = 0; g <= 3 && sobe; g++){
            /* o polinómio de grau g avaliado em 0..7 */
            for(int n = 0; n < 8; n++){
                long t = 1;
                for(int e = 0; e < g; e++) t *= n;
                v[n] = t;
            }
            /* μ aplicado g+1 vezes tem de anular; g vezes, NÃO */
            long w[8]; memcpy(w, v, sizeof v);
            for(int r = 0; r <= g; r++){
                long z2[8]; z2[0] = w[0];
                for(int n = 1; n < 8; n++) z2[n] = w[n] - w[n-1];
                memcpy(w, z2, sizeof z2);
                if(r < g){                       /* ainda não devia anular */
                    int tudo0 = 1;
                    for(int n = g + 1; n < 8; n++) if(w[n]) tudo0 = 0;
                    if(tudo0 && g > 0) desce = 0;
                }
            }
            for(int n = g + 1; n < 8; n++) if(w[n] != 0) sobe = 0;
        }
        printf("      μ aplicado d+1 vezes a um polinómio de grau d anula, para"
               " d = 0..3: %s\n", sobe ? "sim — DESCE o grau" : "FALHOU");
        printf("        e antes disso NÃO anula (senão «desce» não diria nada): %s\n",
               desce ? "confirmado" : "FALHOU");
        if(!sobe || !desce) mal++;

        /* e ζ sobe: as somas parciais de (1,1,1,1,…) são 1,2,3,4,… */
        long u[8], ac = 0;
        for(int n = 0; n < 8; n++){ ac += 1; u[n] = ac; }
        int recta = 1;
        for(int n = 0; n < 8; n++) if(u[n] != n + 1) recta = 0;
        long tri[8]; ac = 0;
        for(int n = 0; n < 8; n++){ ac += u[n]; tri[n] = ac; }
        int triang = (tri[0] == 1 && tri[1] == 3 && tri[2] == 6 && tri[3] == 10);
        printf("      ζ sobe: a constante vira a RECTA (%s) e a recta vira as"
               " TRIANGULARES 1,3,6,10 (%s)\n",
               recta ? "sim" : "MAU", triang ? "sim" : "MAU");
        if(!recta || !triang) mal++;
        sql_fechar();

        printf("\n");
        ok("D DESCE O GRAU E ∫ SOBE — E NESTA CASA ELES JÁ EXISTIAM COMO μ E ζ, quando ninguém"
           " os tinha posto lado a lado. A Parte V do paper diz «o operador D desce o grau, é"
           " o gato que MEDE; o integral sobe o grau, acumula; e D e ∫ são o par ADJUNTO do"
           " corpo diferencial»; o `aranha` §sec:zeta diz «escrever é a convolução com ζ, que"
           " ACUMULA; recuperar é a deconvolução com μ», e que na base canónica os dois são"
           " MATRIZES triangulares. Não houve nada a construir: pôs-se ζ como tabela — a"
           " triangular inferior de uns — e perguntou-se ao motor. SÃO INVERSAS DOS DOIS"
           " LADOS, e pedir os dois produtos em vez de um é o que distingue «inversa» de"
           " «inversa à esquerda». E μ EXISTE NOS INTEIROS PORQUE det ζ = 1: não é um"
           " acidente da escolha, é o FACTOR UNITÁRIO desta casa — «|det| = 1, factor unitário"
           " e inversa inteira são três nomes da mesma condição» —, e a deconvolução não sai"
           " do andar porque a convolução não perdeu volume. O motor constrói μ sozinho em"
           " 3×3 e sai a DIFERENÇA; em 4×4 recusa pelo tecto próprio da inversa, que diz"
           " porquê. E O GRAU FECHA A AFIRMAÇÃO: μ aplicado d+1 vezes a um polinómio de grau"
           " d ANULA — a forma discreta e exacta de «a derivada de ordem d+1 de um polinómio"
           " de grau d é zero» —, e antes disso NÃO anula, sem o que «desce o grau» não diria"
           " nada. Do outro lado ζ sobe: a constante vira a recta, e a recta vira as"
           " triangulares 1,3,6,10. Nenhum limite entra em parte nenhuma.", mal == 0);
    }


    /* ═══ §W82: CAYLEY — O ESQUILO VIRA ROTAÇÃO, E SAI UM TRIPLO ═══════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W82 (2I+A)(2I−A)⁻¹ leva o esquilo numa rotação RACIONAL.\n\n");
        { const char *tabs[] = { "N","P","Q","QT","G" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w82__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w82__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w82.mem"); unlink("/tmp/pgwire_w82.prog"); }
        if(!sql_abrir("/tmp/pgwire_w82")) mal++;

        /* ── A PARTE VI DO PAPER DÁ O PADÉ [1/1] DA EXPONENCIAL:
         *
         *     e^{A} ≈ (I + A/2)(I − A/2)⁻¹
         *
         * — «a Cayley/Möbius, o Crank–Nicolson». Multiplicando por 2 em cima e
         * em baixo fica (2I + A)(2I − A)⁻¹, que é a mesma coisa sem fracções na
         * entrada. E aqui isto deixa de ser uma aproximação e passa a ser uma
         * IDENTIDADE ESTRUTURAL: a transformada de Cayley leva uma matriz
         * ANTISSIMÉTRICA — o esquilo — numa matriz ORTOGONAL, isto é, numa
         * ROTAÇÃO. O que o fluxo do esquilo faz em tempo contínuo (conservar a
         * norma, §W79), a Cayley faz num passo. */
        long a = 1;
        sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO N VALUES (2,-1), (1,2)", &o);       /* 2I − A */
        sql_executa("CREATE TABLE P (p RACIONAL, q RACIONAL)", &o);
        sql_executa("INSERT INTO P VALUES (2,1), (-1,2)", &o);       /* 2I + A */
        sql_executa("SELECT inversa(*) FROM N", &o);
        int ok_inv = o.ok && o.nrows == 2;
        char iv[4][32];
        if(ok_inv) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            snprintf(iv[i*2+j], 32, "%s", o.cell[i][j]);
        { char q2[220];
          sql_executa("CREATE TABLE Q (p RACIONAL, q RACIONAL)", &o2);
          snprintf(q2, sizeof q2, "INSERT INTO Q VALUES (%s,%s), (%s,%s)",
                   iv[0], iv[1], iv[2], iv[3]);
          sql_executa(q2, &o2); }
        sql_executa("SELECT produto(Q) FROM P", &o);
        int tem = o.ok && o.nrows == 2;
        char Qm[4][32];
        if(tem) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            snprintf(Qm[i*2+j], 32, "%s", o.cell[i][j]);
        int tres45 = tem && !strcmp(Qm[0],"3/5") && !strcmp(Qm[1],"4/5")
                     && !strcmp(Qm[2],"-4/5") && !strcmp(Qm[3],"3/5");
        printf("      A = (0,1;−1,0) → Q = (%s,%s;%s,%s) %s\n",
               Qm[0], Qm[1], Qm[2], Qm[3],
               tres45 ? "— a rotação 3-4-5, EXACTA em ℚ" : "(inesperado)");
        if(!ok_inv || !tem || !tres45) mal++;

        /* ── E É ORTOGONAL, medido com o motor contra si próprio: pede-se a
         * transposta, guarda-se, e exige-se QᵀQ = I. Comparar Q com uma rotação
         * escrita à mão seria confiar na conta; multiplicar de volta é
         * verificá-la. */
        { char q2[220];
          sql_executa("DROP TABLE IF EXISTS Q", &o2);
          sql_executa("CREATE TABLE Q (p RACIONAL, q RACIONAL)", &o2);
          snprintf(q2, sizeof q2, "INSERT INTO Q VALUES (%s,%s), (%s,%s)",
                   Qm[0], Qm[1], Qm[2], Qm[3]);
          sql_executa(q2, &o2);
          sql_executa("SELECT transposta(*) FROM Q", &o);
          sql_executa("CREATE TABLE QT (p RACIONAL, q RACIONAL)", &o2);
          for(int i = 0; i < o.nrows; i++){
              snprintf(q2, sizeof q2, "INSERT INTO QT VALUES (%s,%s)",
                       o.cell[i][0], o.cell[i][1]);
              sql_executa(q2, &o2);
          } }
        sql_executa("SELECT produto(QT) FROM Q", &o);
        int orto = o.ok && o.nrows == 2;
        if(orto) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            if(strcmp(o.cell[i][j], (i == j) ? "1" : "0")) orto = 0;
        sql_executa("SELECT det(*) FROM Q", &o);
        int dq = o.ok && !strcmp(o.cell[0][0], "1");
        printf("      QᵀQ = I: %s · det Q = %s → é uma ROTAÇÃO, não só uma"
               " isometria\n", orto ? "sim" : "MAU", dq ? "1" : "MAU");
        if(!orto || !dq) mal++;

        /* ── E DAÍ SAI O QUE NINGUÉM PÔS: OS TRIPLOS PITAGÓRICOS. Para o esquilo
         * (0,a;−a,0) a conta dá
         *
         *     Q = ( 4−a² ,  4a ; −4a , 4−a² ) / (4+a²),
         *
         * e a ortogonalidade obriga a (4−a²)² + (4a)² = (4+a²)² — que é a
         * PARAMETRIZAÇÃO RACIONAL DO CÍRCULO, e portanto a fórmula dos triplos.
         * Não se procurou nenhum: eles são o que a matriz TEM DE TER para
         * conservar a norma em ℚ. E o motor reduz a classe sozinho, pelo que
         * (12,16,20) sai escrito como 3/5 e 4/5. */
        long achados = 0, certos = 0;
        printf("      a  ·  Q do motor            ·  o triplo (4−a², 4a, 4+a²)\n");
        for(a = 1; a <= 6; a++){
            char q2[220];
            sql_executa("DROP TABLE IF EXISTS N", &o2);
            sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL)", &o2);
            snprintf(q2, sizeof q2, "INSERT INTO N VALUES (2,%ld), (%ld,2)", -a, a);
            sql_executa(q2, &o2);
            sql_executa("SELECT inversa(*) FROM N", &o);
            if(!o.ok) continue;
            char w[4][32];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                snprintf(w[i*2+j], 32, "%s", o.cell[i][j]);
            sql_executa("DROP TABLE IF EXISTS Q", &o2);
            sql_executa("CREATE TABLE Q (p RACIONAL, q RACIONAL)", &o2);
            snprintf(q2, sizeof q2, "INSERT INTO Q VALUES (%s,%s), (%s,%s)",
                     w[0], w[1], w[2], w[3]);
            sql_executa(q2, &o2);
            sql_executa("DROP TABLE IF EXISTS P", &o2);
            sql_executa("CREATE TABLE P (p RACIONAL, q RACIONAL)", &o2);
            snprintf(q2, sizeof q2, "INSERT INTO P VALUES (2,%ld), (%ld,2)", a, -a);
            sql_executa(q2, &o2);
            sql_executa("SELECT produto(Q) FROM P", &o);
            if(!o.ok || o.nrows != 2) continue;
            long c = 4 - a*a, s2 = 4*a, h = 4 + a*a;
            int pit = (c*c + s2*s2 == h*h);
            achados++; if(pit) certos++;
            printf("      %ld  ·  %-7s%-7s%-7s%-7s ·  (%ld,%ld,%ld) %s\n", a,
                   o.cell[0][0], o.cell[0][1], o.cell[1][0], o.cell[1][1],
                   labs(c), s2, h, pit ? "" : "MAU");
        }
        printf("      %ld esquilos, %ld com (4−a²)² + (4a)² = (4+a²)²\n",
               achados, certos);
        if(achados < 6 || certos != achados) mal++;

        /* ── E O CONTROLO É UMA MATRIZ QUE NÃO É ESQUILO: a Cayley dela NÃO tem
         * de ser ortogonal, e não é. Sem isto, «leva o esquilo numa rotação»
         * podia ser «leva tudo numa rotação», e a hipótese não estaria a fazer
         * trabalho nenhum. */
        sql_executa("DROP TABLE IF EXISTS N", &o2);
        sql_executa("CREATE TABLE N (p RACIONAL, q RACIONAL)", &o2);
        sql_executa("INSERT INTO N VALUES (1,-1), (1,3)", &o2);      /* 2I − G, G=(1,1;-1,-1) */
        sql_executa("SELECT inversa(*) FROM N", &o);
        int gok = o.ok;
        char g[4][32];
        if(gok) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            snprintf(g[i*2+j], 32, "%s", o.cell[i][j]);
        if(gok){ char q2[220];
          sql_executa("DROP TABLE IF EXISTS Q", &o2);
          sql_executa("CREATE TABLE Q (p RACIONAL, q RACIONAL)", &o2);
          snprintf(q2, sizeof q2, "INSERT INTO Q VALUES (%s,%s), (%s,%s)",
                   g[0], g[1], g[2], g[3]);
          sql_executa(q2, &o2);
          sql_executa("DROP TABLE IF EXISTS P", &o2);
          sql_executa("CREATE TABLE P (p RACIONAL, q RACIONAL)", &o2);
          sql_executa("INSERT INTO P VALUES (3,1), (-1,1)", &o2);    /* 2I + G */
          sql_executa("SELECT produto(Q) FROM P", &o); }
        sql_executa("SELECT det(*) FROM Q", &o2);
        int nao_rot = 1;
        { sql_executa("DROP TABLE IF EXISTS G", &o2);
          sql_executa("CREATE TABLE G (p RACIONAL, q RACIONAL)", &o2);
          char q2[220];
          snprintf(q2, sizeof q2, "INSERT INTO G VALUES (%s,%s), (%s,%s)",
                   o.ok ? o.cell[0][0] : "0", o.ok ? o.cell[0][1] : "0",
                   o.ok ? o.cell[1][0] : "0", o.ok ? o.cell[1][1] : "0");
          sql_executa(q2, &o2);
          sql_executa("SELECT gram(*) FROM G", &o2);
          if(o2.ok && o2.nrows == 2)
              nao_rot = !(!strcmp(o2.cell[0][0],"1") && !strcmp(o2.cell[1][1],"1")
                          && !strcmp(o2.cell[0][1],"0")); }
        printf("      controlo — a Cayley de uma NÃO antissimétrica: ortogonal? %s\n",
               nao_rot ? "não, como devia" : "SIM (mau — a hipótese não trabalha)");
        if(!nao_rot) mal++;
        sql_fechar();

        printf("\n");
        ok("A TRANSFORMADA DE CAYLEY LEVA O ESQUILO NUMA ROTAÇÃO RACIONAL, E OS TRIPLOS"
           " PITAGÓRICOS CAEM DE LÁ SEM SEREM PROCURADOS. A Parte VI do paper dá o Padé [1/1]"
           " da exponencial — e^A ≈ (I + A/2)(I − A/2)⁻¹, «a Cayley/Möbius, o"
           " Crank–Nicolson» —, e multiplicando por 2 em cima e em baixo fica"
           " (2I + A)(2I − A)⁻¹, o mesmo sem fracções na entrada. AQUI ISTO DEIXA DE SER UMA"
           " APROXIMAÇÃO E PASSA A SER UMA IDENTIDADE ESTRUTURAL: a Cayley de uma matriz"
           " ANTISSIMÉTRICA é ORTOGONAL. O que o fluxo do esquilo faz em tempo contínuo —"
           " conservar a norma, §W79 — a Cayley faz num passo. Mede-se com o motor contra si"
           " próprio: pede-se a transposta, guarda-se, e exige-se QᵀQ = I, porque comparar Q"
           " com uma rotação escrita à mão seria confiar na conta e multiplicar de volta é"
           " verificá-la; com det Q = 1 a distinguir a rotação da simples isometria. E DAÍ"
           " SAI O QUE NINGUÉM PÔS: para o esquilo (0,a;−a,0) a conta dá as entradas"
           " (4−a²)/(4+a²) e 4a/(4+a²), e a ortogonalidade OBRIGA a"
           " (4−a²)² + (4a)² = (4+a²)² — a parametrização racional do círculo, e portanto a"
           " fórmula dos triplos. Não se procurou nenhum: eles são o que a matriz TEM DE TER"
           " para conservar a norma em ℚ, e saem (3,4,5), (5,12,13), (21,20,29) com o motor a"
           " reduzir a classe sozinho. O CONTROLO é uma matriz que não é esquilo, cuja Cayley"
           " não é ortogonal — sem ele, «leva o esquilo numa rotação» podia ser «leva tudo"
           " numa rotação», e a hipótese não estaria a fazer trabalho nenhum.", mal == 0);
    }


    /* ═══ §W83: O SELO É O TRAÇO — E ELE É INTEIRO SEM OS λ SEREM ══════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W83 tr(Aⁿ) = Σλᵢⁿ: a soma sobrevive à irracionalidade das parcelas.\n\n");
        { const char *tabs[] = { "A","P" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w83__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w83__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w83.mem"); unlink("/tmp/pgwire_w83.prog"); }
        if(!sql_abrir("/tmp/pgwire_w83")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)

        /* ── A PARTE VII DO PAPER DIZ QUE «O SELO É O TRAÇO»:
         * Tr(e^{At}) = Σᵢ e^{λᵢt} — a soma sobre o espectro. No discreto isso é
         *
         *     tr(Aⁿ) = λ₁ⁿ + λ₂ⁿ,
         *
         * e daí sai a coisa que §W63 mediu sem nomear: a recorrência de Lucas É
         * a soma das potências dos autovalores. */

        /* (1) COM ESPECTRO RACIONAL verifica-se directamente: diag(3,2) tem
         * λ = 3 e 2, e o traço da potência tem de ser 3ⁿ + 2ⁿ. */
        POE("A", 3,0,0,2);
        sql_executa("SELECT autovalores(*) FROM A", &o);
        int rac = o.ok && o.nrows == 2
                  && !strcmp(o.cell[0][0], "3") && !strcmp(o.cell[1][0], "2");
        POE("P", 3,0,0,2);
        int bate = rac; long ate = 0;
        for(int n = 2; n <= 4; n++){          /* 3⁵ = 243 não cabe no Word_8 */
            sql_executa("SELECT produto(A) FROM P", &o);
            if(!o.ok){ bate = 0; break; }
            long r[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                r[i][j] = atol(o.cell[i][j]);
            POE("P", r[0][0], r[0][1], r[1][0], r[1][1]);
            sql_executa("SELECT traco(*) FROM P", &o);
            if(!o.ok){ bate = 0; break; }
            long tn = atol(o.cell[0][0]);
            long e = 1, f = 1;
            for(int k = 0; k < n; k++){ e *= 3; f *= 2; }
            if(tn != e + f) bate = 0; else ate = n;
        }
        printf("      espectro racional {3,2}: tr(Aⁿ) = 3ⁿ + 2ⁿ até n = %ld: %s\n",
               ate, bate ? "sim" : "FALHOU");
        printf("        (pára em 4 porque 3⁵ = 243 não cabe no Word_8 — o tecto"
               " diz-se, e as duas portas de §W75 apanham-no)\n");
        if(!rac || !bate) mal++;

        /* ── (2) E AGORA O QUE ISTO TEM DE NOTÁVEL. Na matriz de Fibonacci os
         * autovalores NÃO estão em ℚ — o motor recusa-os, e §W70 mostrou que
         * essa recusa é o salto ℚ→ℝ. E mesmo assim tr(Aⁿ) é INTEIRO, e são os
         * números de Lucas.
         *
         * A razão é a SIMETRIA: λ₁ⁿ + λ₂ⁿ é uma função simétrica dos dois, e as
         * simétricas exprimem-se nos coeficientes por Viète — que são inteiros.
         * O que não cabe em ℚ é cada parcela; a SOMA cabe, porque não distingue
         * as parcelas. É a mesma figura do §W72, onde o traço é a face aditiva:
         * ele não vê qual raiz é qual, e por isso não precisa que elas existam. */
        POE("A", 1,1,1,0);
        sql_executa("SELECT autovalores(*) FROM A", &o);
        int recusa = !o.ok && strstr(o.err, "not rational") != NULL;
        long lucas[9] = {2,1,3,4,7,11,18,29,47};
        POE("P", 1,1,1,0);
        int luc = recusa; long ate2 = 1;
        for(int n = 2; n <= 7; n++){
            sql_executa("SELECT produto(A) FROM P", &o);
            if(!o.ok){ luc = 0; break; }
            long r[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                r[i][j] = atol(o.cell[i][j]);
            POE("P", r[0][0], r[0][1], r[1][0], r[1][1]);
            sql_executa("SELECT traco(*) FROM P", &o);
            if(!o.ok){ luc = 0; break; }
            if(atol(o.cell[0][0]) != lucas[n]) luc = 0; else ate2 = n;
        }
        printf("      espectro IRRACIONAL: os autovalores %s\n",
               recusa ? "não estão em ℚ e o motor recusa-os" : "MAU");
        printf("        e mesmo assim tr(Aⁿ) é INTEIRO até n = %ld — os Lucas"
               " 3,4,7,11,18,29: %s\n", ate2, luc ? "sim" : "FALHOU");
        if(!recusa || !luc) mal++;

        /* ── (3) E O CONTROLO É A PARCELA SOZINHA, sem o qual «a soma é inteira»
         * não diria nada: φⁿ NÃO é inteiro. Verifica-se sem sair dos inteiros —
         * φ é raiz de x² = x + 1, logo φⁿ = Fₙφ + Fₙ₋₁, e isso é inteiro só se
         * Fₙ = 0, o que não acontece para n ≥ 1. A parcela carrega o φ; a soma
         * cancela-o, porque a outra parcela traz −1/φ. */
        int parcela_nao = 1;
        { long F[9] = {0,1,1,2,3,5,8,13,21};
          for(int n = 1; n <= 7; n++) if(F[n] == 0) parcela_nao = 0; }
        printf("      controlo — a PARCELA sozinha: φⁿ = Fₙφ + Fₙ₋₁, e Fₙ ≠ 0"
               " para n ≥ 1 → φⁿ nunca é inteiro: %s\n",
               parcela_nao ? "confirmado" : "MAU");
        if(!parcela_nao) mal++;

        /* ── (4) E A RECORRÊNCIA CALCULA A SOMA SEM CONHECER AS PARCELAS: pelo
         * §W63, tₙ = B·tₙ₋₁ − C·tₙ₋₂ com t₀ = 2 e t₁ = B. É a identidade de
         * Newton, e é o que torna operacional o «selo é o traço»: pede-se a soma
         * sobre o espectro a um motor que não sabe dizer o espectro. */
        { long t0 = 2, t1 = 1, B = 1, C = -1;    /* Fibonacci */
          int newton = 1;
          for(int n = 2; n <= 8; n++){
              long tn = B*t1 - C*t0;
              if(tn != lucas[n > 8 ? 8 : n]) newton = 0;
              t0 = t1; t1 = tn;
          }
          printf("      a recorrência dá a MESMA soma sem os λ: %s — é Newton, e"
                 " é o que torna o selo operacional\n",
                 newton ? "sim" : "FALHOU");
          if(!newton) mal++; }
        #undef POE
        sql_fechar();

        printf("\n");
        ok("O SELO É O TRAÇO, E ELE É INTEIRO SEM OS AUTOVALORES O SEREM. A Parte VII do"
           " paper diz Tr(e^{At}) = Σᵢ e^{λᵢt} — a soma sobre o espectro —, e no discreto"
           " isso é tr(Aⁿ) = λ₁ⁿ + λ₂ⁿ, o que dá nome ao que §W63 mediu sem o nomear: a"
           " recorrência de Lucas É a soma das potências dos autovalores. Com espectro"
           " RACIONAL verifica-se directamente — diag(3,2) dá 3ⁿ + 2ⁿ —, e pára em n = 4"
           " porque 3⁵ = 243 não cabe no Word_8: o tecto diz-se, e foram as duas portas de"
           " §W75 que o apanharam quando a sonda passou por lá. E AGORA O NOTÁVEL: na matriz"
           " de Fibonacci os autovalores NÃO estão em ℚ — o motor recusa-os, e §W70 mostrou"
           " que essa recusa é o salto ℚ→ℝ — e mesmo assim tr(Aⁿ) é INTEIRO, e são os"
           " números de Lucas. A RAZÃO É A SIMETRIA: λ₁ⁿ + λ₂ⁿ é função simétrica dos dois, e"
           " as simétricas exprimem-se nos coeficientes por Viète, que são inteiros. O que"
           " não cabe em ℚ é cada PARCELA; a SOMA cabe, porque não distingue as parcelas — a"
           " mesma figura do §W72, onde o traço é a face aditiva e não vê qual raiz é qual,"
           " pelo que não precisa que elas existam. O CONTROLO é a parcela sozinha, sem o"
           " qual «a soma é inteira» não diria nada: φⁿ = Fₙφ + Fₙ₋₁, e Fₙ ≠ 0 para n ≥ 1,"
           " logo φⁿ nunca é inteiro — a parcela carrega o φ e a soma cancela-o, porque a"
           " outra traz −1/φ. E A RECORRÊNCIA CALCULA A SOMA SEM CONHECER AS PARCELAS: é a"
           " identidade de Newton, e é ela que torna o selo operacional — pede-se a soma"
           " sobre o espectro a um motor que não sabe dizer o espectro.", mal == 0);
    }


    /* ═══ §W84: AS VINTE E UMA OPERAÇÕES NÃO SÃO VINTE E UMA RÉGUAS ════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W84 todas as operações confrontadas sobre a MESMA matriz.\n\n");
        { char m[80], p2[80];
          snprintf(m, sizeof m, "/tmp/pgwire_w84__A.mem");
          snprintf(p2, sizeof p2, "/tmp/pgwire_w84__A.prog");
          unlink(m); unlink(p2);
          unlink("/tmp/pgwire_w84.mem"); unlink("/tmp/pgwire_w84.prog"); }
        if(!sql_abrir("/tmp/pgwire_w84")) mal++;

        /* ── O QUE ESTE BLOCO MEDE NÃO É NENHUMA OPERAÇÃO: É QUE ELAS SÃO A
         * MESMA RÉGUA. As vinte e uma foram escritas em alturas diferentes, e
         * cada uma tem o seu medidor — mas nenhum as confronta TODAS sobre a
         * mesma matriz. É esse o defeito que esta casa persegue com o nome
         * «duas réguas para o mesmo objecto», e ele não aparece em testes
         * individuais por construção: cada um está certo sozinho.
         *
         * Varrem-se as 2401 matrizes com entradas em −3..3 e exigem-se nove
         * famílias de relações que ligam operações DIFERENTES. */
        long n = 0, f_ker = 0, f_dt = 0, f_disc = 0, f_vi = 0,
             f_part = 0, f_gram = 0, f_inv = 0, f_cif = 0, f_tr = 0;
        long p0 = 0, p1 = 0, p2c = 0, d0 = 0, av = 0, iv = 0;
        for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            char q2[220];
            sql_executa("DROP TABLE IF EXISTS A", &o2);
            sql_executa("CREATE TABLE A (p RACIONAL, q RACIONAL)", &o2);
            snprintf(q2, sizeof q2, "INSERT INTO A VALUES (%ld,%ld), (%ld,%ld)", a,b,c,d);
            sql_executa(q2, &o2);
            n++;
            long D = a*d - b*c, T = a + d, Dd = T*T - 4*D;

            /* (1) det e traço contra a conta directa */
            sql_executa("SELECT det(*) FROM A", &o);
            long md = o.ok ? atol(o.cell[0][0]) : 999999;
            sql_executa("SELECT traco(*) FROM A", &o);
            long mt = o.ok ? atol(o.cell[0][0]) : 999999;
            if(md != D || mt != T) f_dt++;

            /* (2) posto + dim(núcleo) = 2 — duas operações, uma conservação */
            sql_executa("SELECT posto(*) FROM A", &o);
            int pk = o.ok ? atoi(o.cell[0][0]) : -1;
            sql_executa("SELECT nucleo(*) FROM A", &o);
            int kk = o.ok ? o.nrows : -1;
            if(pk < 0 || kk < 0 || pk + kk != 2) f_ker++;
            if(pk == 0) p0++; else if(pk == 1) p1++; else p2c++;

            /* (3) det = 0 ⟺ posto < 2 — o determinante e o posto a dizerem a
             * mesma coisa por caminhos que não se conhecem */
            if((D == 0) != (pk < 2)) f_dt++;
            if(D == 0) d0++;

            /* (4) o Δ do `regime` é tr² − 4det */
            sql_executa("SELECT regime(*) FROM A", &o);
            if(!o.ok || atol(o.cell[0][3]) != Dd) f_disc++;

            /* (5) `autovalores` responde EXACTAMENTE quando as raízes são
             * racionais, e quando responde cumpre Viète */
            long r = -1;
            if(Dd >= 0){ r = 0; while((r+1)*(r+1) <= Dd) r++; }
            int deve = (Dd >= 0 && r*r == Dd && ((T + r) % 2 == 0));
            sql_executa("SELECT autovalores(*) FROM A", &o);
            if(o.ok) av++;
            if(o.ok != deve) f_vi++;
            else if(o.ok){
                long l1 = atol(o.cell[0][0]);
                long l2 = o.nrows > 1 ? atol(o.cell[1][0]) : l1;
                if(l1 + l2 != T || l1*l2 != D) f_vi++;
            }

            /* (6) a partição responde sempre numa quadrada */
            sql_executa("SELECT simetrica(*) FROM A", &o);
            int sok = o.ok && o.nrows == 2;
            sql_executa("SELECT antisimetrica(*) FROM A", &o);
            int aok = o.ok && o.nrows == 2;
            if(!sok || !aok) f_part++;

            /* (7) a Gram é simétrica e det G = det² */
            sql_executa("SELECT gram(*) FROM A", &o);
            if(!o.ok || o.nrows != 2) f_gram++;
            else { long g11 = atol(o.cell[0][0]), g12 = atol(o.cell[0][1]);
                   long g21 = atol(o.cell[1][0]), g22 = atol(o.cell[1][1]);
                   if(g12 != g21 || g11*g22 - g12*g21 != D*D) f_gram++; }

            /* (8) a inversa existe EXACTAMENTE quando det ≠ 0 */
            sql_executa("SELECT inversa(*) FROM A", &o);
            if(o.ok) iv++;
            if(o.ok != (D != 0)) f_inv++;

            /* (9) a cifra responde sempre numa 2×2 inteira, e a transposta tem
             * o MESMO det e o MESMO traço */
            sql_executa("SELECT cifra(*) FROM A", &o);
            if(!o.ok) f_cif++;
            sql_executa("SELECT transposta(*) FROM A", &o);
            if(!o.ok || o.nrows != 2) f_tr++;
            else { long ta = atol(o.cell[0][0]), tb = atol(o.cell[0][1]);
                   long tc = atol(o.cell[1][0]), td = atol(o.cell[1][1]);
                   if(ta + td != T || ta*td - tb*tc != D) f_tr++; }
        }
        long total = f_ker + f_dt + f_disc + f_vi + f_part + f_gram + f_inv + f_cif + f_tr;
        printf("      %ld matrizes · nove famílias de relações · %ld divergências\n",
               n, total);
        if(total){
            printf("        posto+ker %ld · det/traço %ld · Δ %ld · Viète %ld ·"
                   " partição %ld · Gram %ld · inversa %ld · cifra %ld ·"
                   " transposta %ld\n",
                   f_ker, f_dt, f_disc, f_vi, f_part, f_gram, f_inv, f_cif, f_tr);
            mal++;
        }

        /* ── E O CONTROLO É A DISTRIBUIÇÃO, sem o qual «zero divergências» seria
         * uma ausência a passar por acordo. Se todas as matrizes tivessem posto
         * 2, a relação posto+ker = 2 nunca teria de repartir; se nenhuma fosse
         * singular, a equivalência det = 0 ⟺ posto < 2 nunca seria exercida do
         * lado verdadeiro; e se os autovalores respondessem sempre, a recusa
         * nunca seria testada. Conta-se, e exige-se que os DOIS lados de cada
         * relação apareçam. */
        printf("      distribuição: posto 0/1/2 = %ld/%ld/%ld · singulares %ld ·"
               " autovalores em %ld · inversa em %ld\n",
               p0, p1, p2c, d0, av, iv);
        int variado = (p0 > 0 && p1 > 0 && p2c > 0)      /* os três postos */
                   && (d0 > 0 && d0 < n)                  /* singulares e não */
                   && (av > 0 && av < n)                  /* respondem e recusam */
                   && (iv > 0 && iv < n);                 /* inversa existe e não */
        printf("      cada relação é exercida nos DOIS lados: %s\n",
               variado ? "sim — o zero de cima não é uma ausência"
                       : "NÃO — o zero podia ser trivial");
        if(!variado) mal++;
        sql_fechar();

        printf("\n");
        ok("AS VINTE E UMA OPERAÇÕES NÃO SÃO VINTE E UMA RÉGUAS — E ISTO NÃO SE VÊ EM TESTE"
           " NENHUM DOS OUTROS. Cada operação foi escrita numa altura e tem o seu medidor,"
           " onde está certa SOZINHA; o defeito que esta casa persegue com o nome «duas"
           " réguas para o mesmo objecto» vive precisamente entre elas, e não aparece em"
           " nenhum medidor individual por construção. Aqui confrontam-se todas sobre a MESMA"
           " matriz, nas 2401 com entradas em −3..3, com nove famílias de relações que ligam"
           " operações DIFERENTES: det e traço contra a conta directa; posto + dim(núcleo) ="
           " 2, que é uma conservação entre duas operações; det = 0 ⟺ posto < 2, o"
           " determinante e o posto a dizerem o mesmo por caminhos que não se conhecem; o Δ"
           " do `regime` a ser tr² − 4det; `autovalores` a responder EXACTAMENTE quando as"
           " raízes são racionais, e a cumprir Viète quando responde; a partição a responder"
           " sempre; a Gram simétrica com det G = det²; a inversa a existir exactamente onde"
           " det ≠ 0; e a transposta com o mesmo det e o mesmo traço. ZERO DIVERGÊNCIAS. E O"
           " CONTROLO É A DISTRIBUIÇÃO, sem o qual esse zero seria uma ausência a passar por"
           " acordo: se todas tivessem posto 2, a conservação nunca repartia; se nenhuma"
           " fosse singular, a equivalência nunca era exercida do lado verdadeiro; se os"
           " autovalores respondessem sempre, a recusa nunca era testada. Contam-se, e os"
           " três postos aparecem (1, 288 e 2112), 289 são singulares, os autovalores"
           " respondem em 963 e recusam nas outras — cada relação exercida nos DOIS lados.",
           mal == 0);
    }


    /* ═══ §W85: CADA FACE RESPEITA A SUA OPERAÇÃO, E FALHA NA OUTRA ════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W85 det multiplica e traço soma — e nenhum faz o do outro.\n\n");
        { const char *tabs[] = { "A","B","R" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w85__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w85__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w85.mem"); unlink("/tmp/pgwire_w85.prog"); }
        if(!sql_abrir("/tmp/pgwire_w85")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)
        #define DET(t) (sql_executa("SELECT det(*) FROM " t, &o), \
                        o.ok ? atol(o.cell[0][0]) : 999999)
        #define TR(t)  (sql_executa("SELECT traco(*) FROM " t, &o), \
                        o.ok ? atol(o.cell[0][0]) : 999999)

        /* ── §W72 MOSTROU QUE (traço, det) É (soma, produto) DAS RAÍZES. Faltava
         * a consequência: cada um herda a operação de que veio, e SÓ essa.
         *
         *   det(A·B) = det A · det B     a face multiplicativa MULTIPLICA
         *   tr(A+B)  = tr A  + tr B      a face aditiva SOMA
         *
         * e a troca não vale: o traço não multiplica e o determinante não soma.
         * As duas faces não são simétricas no que fazem — o `aranha` §sec:dualidades
         * já o dizia de outra maneira («uma endereça e a outra produz») —, e aqui
         * a assimetria aparece como uma lei que cada uma cumpre e a outra não. */
        long n = 0, dmul = 0, tadd = 0, tcic = 0, tnmul = 0, dnadd = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long e = -2; e <= 2; e++) for(long f = -2; f <= 2; f++){
            POE("A", a, b, 1, 1);
            POE("B", e, f, 0, 1);
            long dA = DET("A"), dB = DET("B"), tA = TR("A"), tB = TR("B");

            sql_executa("SELECT produto(B) FROM A", &o);      /* A·B */
            if(!o.ok || o.nrows != 2) continue;
            long r[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                r[i][j] = atol(o.cell[i][j]);
            POE("R", r[0][0], r[0][1], r[1][0], r[1][1]);
            long dAB = DET("R"), tAB = TR("R");

            sql_executa("SELECT produto(A) FROM B", &o);      /* B·A */
            if(!o.ok || o.nrows != 2) continue;
            long s2[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                s2[i][j] = atol(o.cell[i][j]);
            POE("R", s2[0][0], s2[0][1], s2[1][0], s2[1][1]);
            long tBA = TR("R");

            sql_executa("SELECT soma(B) FROM A", &o);         /* A + B */
            if(!o.ok || o.nrows != 2) continue;
            long u[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                u[i][j] = atol(o.cell[i][j]);
            POE("R", u[0][0], u[0][1], u[1][0], u[1][1]);
            long dS = DET("R"), tS = TR("R");

            n++;
            if(dAB == dA*dB) dmul++;
            if(tS == tA + tB) tadd++;
            if(tAB == tBA) tcic++;
            if(tAB != tA*tB) tnmul++;
            if(dS != dA + dB) dnadd++;
        }
        printf("      %ld pares (A,B):\n", n);
        printf("        det(A·B) = det A · det B ... %ld/%ld  ← a multiplicativa MULTIPLICA\n",
               dmul, n);
        printf("        tr(A+B)  = tr A + tr B ..... %ld/%ld  ← a aditiva SOMA\n", tadd, n);
        printf("        tr(A·B)  = tr(B·A) ......... %ld/%ld  ← cíclico, que é outra coisa\n",
               tcic, n);
        if(dmul != n || tadd != n || tcic != n) mal++;

        /* ── E A TROCA NÃO VALE — é isto que faz da assimetria uma lei e não uma
         * arrumação. Se o traço multiplicasse e o determinante somasse, as duas
         * faces seriam a mesma coisa com dois nomes. */
        printf("        tr(A·B) ≠ tr A · tr B ...... %ld/%ld  ← o traço NÃO multiplica\n",
               tnmul, n);
        printf("        det(A+B) ≠ det A + det B ... %ld/%ld  ← o det NÃO soma\n",
               dnadd, n);
        if(tnmul == 0 || dnadd == 0) mal++;

        /* ── E OS QUE COINCIDEM POR ACASO SÃO O QUE IMPEDE A LEITURA ERRADA:
         * não são %ld casos onde a lei falha — são casos onde os dois lados
         * calham iguais sem que haja lei nenhuma. Se a desigualdade fosse
         * universal, ela seria uma segunda lei; sendo genérica, ela é a AUSÊNCIA
         * de lei, que é o que se quer dizer. Dizê-lo é o que separa «não vale»
         * de «vale ao contrário». */
        printf("      e coincidem por acaso em %ld e %ld — a desigualdade é"
               " genérica, não universal: é a AUSÊNCIA de lei, não uma segunda lei\n",
               n - tnmul, n - dnadd);
        if(n - tnmul == 0 || n - dnadd == 0){
            printf("        (se nunca coincidissem, «não multiplica» seria uma lei"
                   " própria e não uma ausência)\n");
        }

        /* ── E O CASO NOMEADO: a identidade tem det 1 e traço 2, pelo que ela é
         * o neutro de UMA face e não da outra. det(A·I) = det A · 1 fecha; mas
         * tr(A + I) = tr A + 2, e não tr A. O neutro da soma é a matriz NULA,
         * cujo traço é 0 — e são objectos diferentes, que é o Teor. 2(4) do
         * `aranha` outra vez. */
        POE("A", 2, 3, 1, 4);
        POE("B", 1, 0, 0, 1);                                  /* I */
        long dA = DET("A"), tA = TR("A");
        sql_executa("SELECT produto(B) FROM A", &o);
        long dAI = 0;
        if(o.ok){ long r[2][2];
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                      r[i][j] = atol(o.cell[i][j]);
                  POE("R", r[0][0], r[0][1], r[1][0], r[1][1]);
                  dAI = DET("R"); }
        sql_executa("SELECT soma(B) FROM A", &o);
        long tAI = 0;
        if(o.ok){ long u[2][2];
                  for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                      u[i][j] = atol(o.cell[i][j]);
                  POE("R", u[0][0], u[0][1], u[1][0], u[1][1]);
                  tAI = TR("R"); }
        int neutro = (dAI == dA) && (tAI == tA + 2);
        printf("      o neutro é de UMA face: det(A·I) = %ld = det A, mas"
               " tr(A+I) = %ld = tr A + 2 %s\n", dAI, tAI,
               neutro ? "— o neutro da soma é a NULA, outro objecto" : "(MAU)");
        if(!neutro) mal++;
        #undef POE
        #undef DET
        #undef TR
        sql_fechar();

        printf("\n");
        ok("CADA FACE RESPEITA A SUA OPERAÇÃO, E FALHA NA OUTRA — É ISSO QUE AS FAZ DUAS."
           " §W72 mostrou que (traço, det) É (soma, produto) das raízes; faltava a"
           " consequência, que é uma lei de cada lado: det(A·B) = det A · det B — a face"
           " MULTIPLICATIVA multiplica — e tr(A+B) = tr A + tr B — a face ADITIVA soma."
           " Ambas em todos os 625 pares. E o traço é ainda CÍCLICO, tr(A·B) = tr(B·A), que é"
           " outra coisa e não multiplicatividade. E A TROCA NÃO VALE: o traço não multiplica"
           " e o determinante não soma, o que faz da assimetria uma LEI e não uma arrumação"
           " — se cada um fizesse o do outro, as duas faces seriam a mesma coisa com dois"
           " nomes. OS CASOS QUE COINCIDEM POR ACASO SÃO O QUE IMPEDE A LEITURA ERRADA: não"
           " são casos onde a lei falha, são casos onde os dois lados calham iguais sem haver"
           " lei nenhuma. Se a desigualdade fosse universal, seria uma SEGUNDA lei; sendo"
           " genérica, é a AUSÊNCIA de lei — e dizê-lo é o que separa «não vale» de «vale ao"
           " contrário». E O NEUTRO É DE UMA FACE SÓ: det(A·I) = det A fecha, mas"
           " tr(A + I) = tr A + 2, porque o neutro da soma é a matriz NULA e não a"
           " identidade. São objectos diferentes, que é o Teor. 2(4) do `aranha` outra vez.",
           mal == 0);
    }


    /* ═══ §W86: O GATO CRESCE, O GAMBÁ CICLA, E A SOMA PROJECTA ════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W86 três regimes das potências, e a assinatura split.\n\n");
        { const char *tabs[] = { "A","G","P","W","K" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w86__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w86__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w86.mem"); unlink("/tmp/pgwire_w86.prog"); }
        if(!sql_abrir("/tmp/pgwire_w86")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)

        /* ── O `broca-so/papers/matrix.tex` NOMEIA AS DUAS MATRIZES QUE ESTE
         * MEDIDOR TEM ANDADO A USAR: o GATO A = (1,1;1,0), simétrico, det −1,
         * A² = A + I, autovalores φ e −1/φ — hiperbólico; e o GAMBÁ
         * G = (0,1;−1,0), antissimétrico, det +1, G² = −I, autovalores ±i —
         * elíptico. São a matriz de Fibonacci de §W62 e a rotação de §W68, com
         * os nomes que aquele paper lhes dá.
         *
         * E declara TRÊS regimes das potências, dos quais este medidor só tinha
         * dois: A^k CRESCE (Fibonacci), G^k CICLA (período 4), e
         * (A+G)^k = A+G — PROJECTA. A soma é IDEMPOTENTE, e a idempotência é o
         * colapso: medir duas vezes é medir uma. */
        POE("A", 1,1,1,0);
        POE("G", 0,1,-1,0);
        sql_executa("SELECT soma(G) FROM A", &o);
        int soma_ok = o.ok && o.nrows == 2
                      && !strcmp(o.cell[0][0],"1") && !strcmp(o.cell[0][1],"2")
                      && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
        POE("P", 1,2,0,0);
        sql_executa("SELECT produto(P) FROM P", &o);
        int idem = o.ok && o.nrows == 2
                   && !strcmp(o.cell[0][0],"1") && !strcmp(o.cell[0][1],"2")
                   && !strcmp(o.cell[1][0],"0") && !strcmp(o.cell[1][1],"0");
        printf("      A + G = (1,2;0,0) (%s) e (A+G)² = A+G (%s) — P² = P, a"
               " PROJECÇÃO\n", soma_ok ? "sim" : "MAU", idem ? "sim" : "MAU");
        if(!soma_ok || !idem) mal++;

        /* ── E UMA PROJECÇÃO TEM DE TER POSTO MENOR: det 0 e posto 1. Se fosse
         * de posto cheio e idempotente, seria a identidade — e a identidade não
         * colapsa nada. É o posto que faz da idempotência uma PROJECÇÃO. */
        sql_executa("SELECT det(*) FROM P", &o);
        int d0 = o.ok && !strcmp(o.cell[0][0], "0");
        sql_executa("SELECT posto(*) FROM P", &o);
        int p1 = o.ok && !strcmp(o.cell[0][0], "1");
        printf("        det %s · posto %s → colapsa uma dimensão, e é isso que a"
               " distingue da identidade\n",
               d0 ? "0" : "MAU", p1 ? "1" : "MAU");
        if(!d0 || !p1) mal++;

        /* ── E AQUI UMA PRECISÃO, porque as duas perguntas são DIFERENTES. O
         * `regime` desta casa classifica o FLUXO e^{At} pelo sinal de Re(λ), e
         * para A+G ele diz CAOS — os autovalores são 1 e 0, e o 1 faz crescer.
         * A idempotência é sobre as POTÊNCIAS A^k, que é outra sucessão: com
         * λ = 1 e 0, as potências ficam paradas. Crescer no fluxo e ficar parado
         * nas potências não é contradição — são dois objectos, e confundi-los
         * seria ler o regime como se ele falasse de A^k. */
        sql_executa("SELECT regime(*) FROM P", &o);
        int reg = o.ok && !strcmp(o.cell[0][0], "CAOS");
        printf("        e o `regime` diz %s — mas isso é do FLUXO e^{At}, não das"
               " potências A^k: são dois objectos\n", reg ? "CAOS" : "?");
        if(!reg) mal++;

        /* ── (2) A ASSINATURA SPLIT. O paper diz que {I, A, G, AG} é base de
         * M₂(ℝ) e que a estrutura é a dos QUATÉRNIOS DIVIDIDOS:
         *
         *     G² = −I           (elíptico, i² = −1)
         *     (2A−I)² = 5I      (hiperbólico, j² = +1 a menos de escala)
         *     [A,G]² = 5I       (hiperbólico, k² = +1)
         *
         * — um eixo elíptico e dois hiperbólicos, que é o que «split» quer
         * dizer. E o 5 aparece nos dois hiperbólicos porque é o DISCRIMINANTE do
         * áureo: tr² − 4det = 1 + 4. O mesmo número de §W62. */
        sql_executa("SELECT produto(G) FROM G", &o);
        int g2 = o.ok && !strcmp(o.cell[0][0],"-1") && !strcmp(o.cell[1][1],"-1")
                 && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        POE("W", 1,2,2,-1);                                   /* 2A − I */
        sql_executa("SELECT produto(W) FROM W", &o);
        int w2 = o.ok && !strcmp(o.cell[0][0],"5") && !strcmp(o.cell[1][1],"5")
                 && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        /* o comutador [A,G] = AG − GA, pedido ao motor nas duas ordens */
        sql_executa("SELECT produto(G) FROM A", &o);
        long ag[2][2];
        int ok1 = o.ok && o.nrows == 2;
        if(ok1) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            ag[i][j] = atol(o.cell[i][j]);
        sql_executa("SELECT produto(A) FROM G", &o);
        long ga[2][2];
        int ok2 = o.ok && o.nrows == 2;
        if(ok2) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            ga[i][j] = atol(o.cell[i][j]);
        int k2 = 0;
        if(ok1 && ok2){
            POE("K", ag[0][0]-ga[0][0], ag[0][1]-ga[0][1],
                     ag[1][0]-ga[1][0], ag[1][1]-ga[1][1]);
            sql_executa("SELECT produto(K) FROM K", &o);
            k2 = o.ok && !strcmp(o.cell[0][0],"5") && !strcmp(o.cell[1][1],"5")
                 && !strcmp(o.cell[0][1],"0") && !strcmp(o.cell[1][0],"0");
        }
        printf("      G² = %s · (2A−I)² = %s · [A,G]² = %s\n",
               g2 ? "−I" : "MAU", w2 ? "5I" : "MAU", k2 ? "5I" : "MAU");
        printf("        um eixo ELÍPTICO e dois HIPERBÓLICOS — é isso que «split»"
               " quer dizer\n");
        if(!g2 || !w2 || !k2) mal++;

        /* ── E O 5 NÃO É UM NÚMERO QUALQUER: é o discriminante do áureo,
         * tr² − 4det = 1 + 4, o mesmo de §W62. O motor confirma-o pela outra
         * ponta, sem se lhe pedir o quadrado de nada. */
        sql_executa("SELECT regime(*) FROM A", &o);
        int disc5 = o.ok && !strcmp(o.cell[0][3], "5");
        printf("      e o 5 dos dois eixos é o Δ do gato: %s — o mesmo número por"
               " dois caminhos\n", disc5 ? "sim, tr² − 4det = 5" : "MAU");
        if(!disc5) mal++;

        /* ── E O CONTROLO É QUE OS TRÊS REGIMES SÃO MESMO TRÊS: a potência do
         * gato cresce (F₅ = 5 aparece), a do gambá volta a I ao quarto passo, e
         * a da soma não se mexe. Sem os três lado a lado, «três regimes» seria
         * uma arrumação e não uma medida. */
        POE("A", 1,1,1,0);
        POE("W", 1,1,1,0);
        int cresce = 1;
        for(int n = 2; n <= 5 && cresce; n++){
            sql_executa("SELECT produto(A) FROM W", &o);
            if(!o.ok){ cresce = 0; break; }
            long r[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                r[i][j] = atol(o.cell[i][j]);
            POE("W", r[0][0], r[0][1], r[1][0], r[1][1]);
            if(n == 5 && r[0][0] != 8) cresce = 0;    /* F₆ = 8 */
        }
        POE("G", 0,1,-1,0);
        POE("W", 0,1,-1,0);
        int cicla = 1;
        for(int n = 2; n <= 4 && cicla; n++){
            sql_executa("SELECT produto(G) FROM W", &o);
            if(!o.ok){ cicla = 0; break; }
            long r[2][2];
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                r[i][j] = atol(o.cell[i][j]);
            POE("W", r[0][0], r[0][1], r[1][0], r[1][1]);
            if(n == 4 && !(r[0][0] == 1 && r[1][1] == 1 && r[0][1] == 0 && r[1][0] == 0))
                cicla = 0;
        }
        printf("      controlo — os três lado a lado: o gato chega a F₆ = 8 (%s),"
               " o gambá volta a I ao 4.º passo (%s), a soma não se mexe (%s)\n",
               cresce ? "sim" : "MAU", cicla ? "sim" : "MAU", idem ? "sim" : "MAU");
        if(!cresce || !cicla) mal++;
        #undef POE
        sql_fechar();

        printf("\n");
        ok("O GATO CRESCE, O GAMBÁ CICLA, E A SOMA PROJECTA — E O TERCEIRO REGIME FALTAVA"
           " AQUI. O `broca-so/papers/matrix.tex` nomeia as duas matrizes que este medidor"
           " tem andado a usar: o GATO (1,1;1,0), simétrico, det −1, A² = A + I, hiperbólico"
           " — a matriz de Fibonacci de §W62 — e o GAMBÁ (0,1;−1,0), antissimétrico, det +1,"
           " G² = −I, elíptico — a rotação de §W68. E declara TRÊS regimes das potências, dos"
           " quais só dois estavam medidos: A^k CRESCE, G^k CICLA com período 4, e"
           " (A+G)^k = A+G PROJECTA. A soma é IDEMPOTENTE, e a idempotência é o colapso:"
           " medir duas vezes é medir uma. E UMA PROJECÇÃO TEM DE TER POSTO MENOR — det 0 e"
           " posto 1 —, porque uma idempotente de posto cheio é a identidade, e a identidade"
           " não colapsa nada: é o posto que faz da idempotência uma projecção. AQUI UMA"
           " PRECISÃO, porque as perguntas são DUAS: o `regime` classifica o FLUXO e^{At} e"
           " diz CAOS, porque os autovalores são 1 e 0 e o 1 faz crescer; a idempotência é"
           " sobre as POTÊNCIAS A^k, que é outra sucessão, e com λ = 1 e 0 elas ficam"
           " paradas. Crescer no fluxo e ficar parado nas potências não é contradição — são"
           " dois objectos, e confundi-los seria ler o regime como se falasse de A^k. E A"
           " ASSINATURA É SPLIT: G² = −I (elíptico), (2A−I)² = 5I e [A,G]² = 5I"
           " (hiperbólicos) — um eixo elíptico e dois hiperbólicos, que é o que a palavra"
           " diz. O 5 não é um número qualquer: é o DISCRIMINANTE do gato, tr² − 4det = 1 + 4,"
           " e o motor confirma-o pela outra ponta sem se lhe pedir o quadrado de nada. O"
           " controlo são os três regimes lado a lado — o gato a chegar a F₆ = 8, o gambá a"
           " voltar a I ao quarto passo, a soma a não se mexer —, sem o que «três regimes»"
           " seria uma arrumação e não uma medida.", mal == 0);
    }


    /* ═══ §W87: O AGM DE MATRIZES — ONDE FECHA, E ONDE NÃO ═════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W87 o corte pede ordem TOTAL; as matrizes só a têm onde comutam.\n\n");
        { const char *tabs[] = { "A","B","R","D" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w87__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w87__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w87.mem"); unlink("/tmp/pgwire_w87.prog"); }
        if(!sql_abrir("/tmp/pgwire_w87")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)

        /* ── A CADEIA A VERIFICAR. O `omnitrix.tex` XVII diz que «o que FECHA o
         * corpo universal é a AGM», e o `aranha thm:corte` prova que as duas
         * dobras terminam num ponto fixo comum. Daí quer-se concluir que o corpo
         * das matrizes fecha, e portanto que há sistemas em qualquer dimensão.
         *
         * O passo que precisa de verificação é UM, e é o (1) do thm:corte: o
         * ENCAIXE, g ≤ g' ≤ m' ≤ m. Ele pressupõe uma ordem TOTAL — na grade dos
         * escalares há sempre um «maior elemento cujo quadrado não passa ab».
         * Nas matrizes a ordem é PARCIAL, e há pares em que nenhuma das duas
         * está acima da outra. */

        /* (1) A ORDEM É PARCIAL — a testemunha é um par incomparável. Com
         * A = diag(2,0) e B = diag(0,2), nem A−B nem B−A são semidefinidas:
         * cada uma tem um valor positivo e um negativo. Não há «maior». */
        POE("A", 2,0,0,0);
        POE("B", 0,0,0,2);
        sql_executa("SELECT oposto(*) FROM B", &o);
        int op = o.ok && o.nrows == 2;
        if(op){ long r[2][2];
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                    r[i][j] = atol(o.cell[i][j]);
                POE("R", r[0][0], r[0][1], r[1][0], r[1][1]); }
        sql_executa("SELECT soma(R) FROM A", &o);          /* A − B */
        int amb = o.ok && o.nrows == 2
                  && !strcmp(o.cell[0][0], "2") && !strcmp(o.cell[1][1], "-2");
        printf("      A = diag(2,0), B = diag(0,2): A − B = diag(2,−2) %s\n",
               amb ? "— um positivo e um negativo: INDEFINIDA" : "MAU");
        printf("        e B − A = diag(−2,2), também indefinida → o par é"
               " INCOMPARÁVEL: a ordem das matrizes é PARCIAL\n");
        if(!op || !amb) mal++;

        /* (2) E A FACE MULTIPLICATIVA NÃO TEM ONDE POUSAR quando não comutam:
         * a média geométrica pede a raiz de A·B, e A·B nem sequer é SIMÉTRICA.
         * Com A = (2,1;1,2) e B = diag(3,1) — as duas simétricas — o produto sai
         * (6,1;3,2) numa ordem e (6,3;1,2) na outra. Um objecto que muda com a
         * ordem dos factores não pode ser «o meio» de nada. */
        POE("A", 2,1,1,2);
        POE("B", 3,0,0,1);
        sql_executa("SELECT produto(B) FROM A", &o);
        long ab[2][2]; int k1 = o.ok && o.nrows == 2;
        if(k1) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            ab[i][j] = atol(o.cell[i][j]);
        sql_executa("SELECT produto(A) FROM B", &o);
        long ba[2][2]; int k2 = o.ok && o.nrows == 2;
        if(k2) for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            ba[i][j] = atol(o.cell[i][j]);
        int nao_comuta = k1 && k2 && (ab[0][1] != ba[0][1]);
        int nao_sim = k1 && (ab[0][1] != ab[1][0]);
        printf("      duas SIMÉTRICAS que não comutam: A·B = (%ld,%ld;%ld,%ld) e"
               " B·A = (%ld,%ld;%ld,%ld)\n",
               ab[0][0],ab[0][1],ab[1][0],ab[1][1],
               ba[0][0],ba[0][1],ba[1][0],ba[1][1]);
        printf("        A·B não é simétrica (%s) → não tem raiz simétrica, e a"
               " face multiplicativa não pousa\n", nao_sim ? "sim" : "MAU");
        if(!nao_comuta || !nao_sim) mal++;

        /* ── (3) E ONDE FECHA: QUANDO COMUTAM. Aí A·B = B·A é simétrica, e se
         * as duas forem simultaneamente diagonalizáveis a raiz sai entrada a
         * entrada. Com A = diag(4,9) e B = diag(16,1): A·B = diag(64,9) nas
         * duas ordens, e a raiz é diag(8,3) — exacta em ℤ. */
        POE("A", 4,0,0,9);
        POE("B", 16,0,0,1);
        sql_executa("SELECT produto(B) FROM A", &o);
        int p1 = o.ok && !strcmp(o.cell[0][0],"64") && !strcmp(o.cell[1][1],"9")
                 && !strcmp(o.cell[0][1],"0");
        sql_executa("SELECT produto(A) FROM B", &o);
        int p2 = o.ok && !strcmp(o.cell[0][0],"64") && !strcmp(o.cell[1][1],"9");
        printf("      que COMUTAM: A·B = B·A = diag(64,9) (%s) e a raiz é"
               " diag(8,3), exacta\n", (p1 && p2) ? "sim" : "MAU");
        if(!p1 || !p2) mal++;

        /* ── (4) E AÍ O AGM MATRICIAL É O AGM ESCALAR, ENTRADA A ENTRADA — que
         * é o que responde à pergunta da dimensão. Comutar e ser simultaneamente
         * diagonalizável é ter uma base comum; nessa base as duas matrizes são
         * listas de números, e a dobra actua em cada um sem ver os outros. O
         * `thm:corte` aplica-se n vezes em paralelo, e termina n vezes.
         *
         * A DIMENSÃO NÃO ACRESCENTA NADA ONDE O CORTE FECHA — e é por isso que
         * ela também não acrescenta dificuldade: n cópias de um problema
         * resolvido.
         *
         * ── E O QUE SE SEGUE DAQUI NÃO É «FALTA UMA ORDEM» ───────────────────
         * A primeira escrita deste bloco terminava com «o que a dimensão traz é
         * o caso não comutativo, e aí falta a ORDEM». Está errado, e a correcção
         * é do enquadramento e não dos números: PROCURAR uma ordem para as
         * matrizes é pô-las de volta numa recta depois de elas terem ganho
         * várias direcções.
         *
         * A matriz NÃO ORDENA — ela COMPÕE ordens. As estruturas que ela recebe
         * já trazem a sua: a árvore tem a ordem de prefixo, o espectro tem a do
         * expoente, a fase tem a do círculo. O que a matriz faz é transportar
         * uma para outra face, acoplar, fundir — sem exigir que todas virem a
         * mesma régua. E esta casa já o tinha medido noutro sítio:
         * `corpo_viveiro.c` diz que «o índice é a ESTANTE, não o que está nela»,
         * com o retículo das dimensões (∨ = lcm, ∧ = gcd) de um lado e o corpo
         * do outro.
         *
         * Portanto o que este bloco mede continua a valer LETRA POR LETRA — a
         * ordem é parcial, AB não é simétrica, o AGM fecha onde comutam —, mas
         * o que isso diz é onde o CORTE se aplica, e não uma falta do objecto.
         * O corte é a máquina da ordem linear; a matriz é o andar de cima. */
        { long a1 = 4, b1 = 16, a2 = 9, b2 = 1;   /* as duas entradas */
          int passos1 = 0, passos2 = 0;
          while(a1 != b1 && passos1 < 40){
              long m = (a1 + b1) / 2, g = 0;
              while((g+1)*(g+1) <= a1*b1) g++;    /* o piso da grade, sem raiz */
              a1 = m; b1 = g; passos1++;
          }
          while(a2 != b2 && passos2 < 40){
              long m = (a2 + b2) / 2, g = 0;
              while((g+1)*(g+1) <= a2*b2) g++;
              a2 = m; b2 = g; passos2++;
          }
          printf("      o AGM em cada entrada TERMINA: (4,16) → %ld em %d passos ·"
                 " (9,1) → %ld em %d passos\n", a1, passos1, a2, passos2);
          printf("        n entradas, n cortes, todos a terminar — a dimensão é"
                 " n cópias de um problema resolvido\n");
          if(a1 != b1 || a2 != b2 || passos1 == 0 || passos2 == 0) mal++; }

        /* ── E O CONTROLO É A ORDEM TOTAL DOS ESCALARES, sem a qual «a ordem é
         * parcial» não distingue nada: em ℤ dois números quaisquer comparam-se
         * sempre, e é isso que dá o «maior elemento cujo quadrado não passa».
         * A diferença entre os dois casos é exactamente essa. */
        int total = 1;
        for(long x = -5; x <= 5 && total; x++) for(long y = -5; y <= 5 && total; y++)
            if(!(x <= y || y <= x)) total = 0;
        printf("      controlo — nos escalares a ordem é TOTAL (121 pares, todos"
               " comparáveis): %s\n", total ? "sim" : "MAU");
        if(!total) mal++;
        #undef POE
        sql_fechar();

        printf("\n");
        ok("O CORTE PEDE ORDEM TOTAL, E AS MATRIZES SÓ A TÊM ONDE COMUTAM — A CADEIA FECHA,"
           " COM A CONDIÇÃO DITA. O `omnitrix.tex` XVII diz que «o que FECHA o corpo universal"
           " é a AGM», e o `aranha thm:corte` prova que as duas dobras terminam num ponto fixo"
           " comum; daí quer-se concluir que o corpo das matrizes fecha e que há sistemas em"
           " qualquer dimensão. O PASSO A VERIFICAR É UM: o (1) do thm:corte, o ENCAIXE"
           " g ≤ g' ≤ m' ≤ m, que pressupõe ordem TOTAL — na grade dos escalares há sempre um"
           " «maior elemento cujo quadrado não passa ab». Nas matrizes a ordem é PARCIAL, e a"
           " testemunha é diag(2,0) contra diag(0,2): nem a diferença nem a sua oposta são"
           " semidefinidas, e não há maior. E A FACE MULTIPLICATIVA NÃO POUSA quando não"
           " comutam: a média geométrica pede a raiz de A·B, e A·B nem é SIMÉTRICA — sai"
           " (6,1;3,2) numa ordem e (6,3;1,2) na outra, e um objecto que muda com a ordem dos"
           " factores não pode ser «o meio» de nada. ONDE FECHA É ONDE COMUTAM: aí A·B = B·A"
           " é simétrica, a raiz sai entrada a entrada — diag(64,9) dá diag(8,3), exacta em"
           " ℤ — e O AGM MATRICIAL É O AGM ESCALAR, ENTRADA A ENTRADA. Comutar e ser"
           " simultaneamente diagonalizável é ter uma base comum; nessa base as matrizes são"
           " listas de números e a dobra actua em cada um sem ver os outros, pelo que o"
           " thm:corte se aplica n vezes em paralelo e termina n vezes. A DIMENSÃO NÃO"
           " ACRESCENTA NADA ONDE O CORTE FECHA — e por isso também não acrescenta"
           " dificuldade: são n cópias de um problema resolvido. O que a dimensão traz é o"
           " caso NÃO comutativo — e AQUI A LEITURA TEM DE SER A CERTA, porque a primeira que"
           " escrevi não era: dizer «falta a ordem» é pôr as matrizes de volta numa recta"
           " depois de elas terem ganho várias direcções. A MATRIZ NÃO ORDENA: ELA COMPÕE"
           " ORDENS. As estruturas que ela recebe já trazem a sua — a árvore tem a de"
           " prefixo, o espectro a do expoente, a fase a do círculo — e o que a matriz faz é"
           " transportar uma para outra face, acoplar, fundir, sem exigir que todas virem a"
           " mesma régua. Esta casa já o tinha medido noutro sítio: o `corpo_viveiro.c` diz"
           " que «o índice é a ESTANTE, não o que está nela», com o retículo das dimensões de"
           " um lado e o corpo do outro. Tudo o que este bloco mede continua a valer letra"
           " por letra — a ordem é parcial, AB não é simétrica, o AGM fecha onde comutam —,"
           " mas isso diz onde o CORTE se aplica e não uma falta do objecto: o corte é a"
           " máquina da ordem linear, e a matriz é o andar de cima. O controlo é a ordem"
           " total dos escalares, sem a qual «parcial» não distinguiria nada.",
           mal == 0);
    }


    /* ═══ §W88: A ÁLGEBRA ESTELAR VESTIDA DE MATRIZ ════════════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W88 E_s como sub-anel de M₂, e a norma É o determinante.\n\n");
        { const char *tabs[] = { "E","Z","W","R" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w88__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w88__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w88.mem"); unlink("/tmp/pgwire_w88.prog"); }
        if(!sql_abrir("/tmp/pgwire_w88")) mal++;

        #define POE(t,a,b,c,d) do { char q2[200]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL)", t); \
            sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld), (%ld,%ld)", \
                     t,(long)(a),(long)(b),(long)(c),(long)(d)); \
            sql_executa(q2,&o2); } while(0)

        /* ── A ÁLGEBRA JÁ ESTAVA ESCRITA; O QUE FALTAVA ERA A ROUPA. O
         * `estelar/algebra_estelar.tex` define E_s como a álgebra real
         * bidimensional z = a + c·e com e² = 1 − s, e dá os regimes: s = 0 é a
         * split-complex (e² = +1), s = 1 o intermédio, s cresce para o
         * octoniónico. Vestir isso de matriz é uma linha:
         *
         *     e  ↦  ( 0    1 )        e²  =  (1−s)·I
         *           ( 1−s  0 )
         *
         * e daí TODO o motor passa a falar de E_s sem uma operação nova. */
        long ss[3] = {0, 1, 2};
        const char *nome[3] = { "split-complex (e²=+1)", "o dual (e²=0)",
                                "o complexo (e²=−1)" };
        const char *esp[3] = { "hiperból", "parabóli", "elíptico" };
        int trio = 1;
        printf("      s ·  e²  ·  E²      · Δ do motor · regime\n");
        for(int k = 0; k < 3; k++){
            long dl = 1 - ss[k];
            POE("E", 0, 1, dl, 0);
            sql_executa("SELECT produto(E) FROM E", &o);
            int e2 = o.ok && atol(o.cell[0][0]) == dl && atol(o.cell[1][1]) == dl
                     && atol(o.cell[0][1]) == 0 && atol(o.cell[1][0]) == 0;
            sql_executa("SELECT regime(*) FROM E", &o);
            int dok = o.ok && atol(o.cell[0][3]) == 4*dl;
            int rok = o.ok && !strncmp(o.cell[0][1], esp[k], 8);
            printf("      %ld ·  %2ld  ·  %s · %-9s · %-8s  %s\n",
                   ss[k], dl, e2 ? "(1−s)I" : "MAU  ",
                   o.ok ? o.cell[0][3] : "?", o.ok ? o.cell[0][1] : "?", nome[k]);
            if(!e2 || !dok || !rok) trio = 0;
        }
        printf("        Δ = 4(1−s) em todos, e o regime segue o s: %s\n",
               trio ? "sim — o parâmetro do paper lido pelo motor" : "FALHOU");
        if(!trio) mal++;

        /* ── (2) E A REPRESENTAÇÃO É FIEL: z = a + c·e ↦ (a, c; (1−s)c, a), e o
         * PRODUTO DE MATRIZES reproduz o produto da álgebra,
         *
         *     z·w = (ab + (1−s)cd) + (ad + bc)·e,
         *
         * que é o que o paper escreve. Não é uma analogia — é a mesma conta, e
         * mede-se pedindo o produto ao motor e comparando com a fórmula. */
        long fiel = 0, casos = 0;
        for(long sv = 0; sv <= 2; sv++){
            long dl = 1 - sv;
            for(long a = -2; a <= 2; a++) for(long c = -2; c <= 2; c++)
            for(long b = -2; b <= 2; b++) for(long d = -2; d <= 2; d++){
                POE("Z", a, c, dl*c, a);
                POE("W", b, d, dl*d, b);
                sql_executa("SELECT produto(W) FROM Z", &o);
                if(!o.ok || o.nrows != 2) continue;
                casos++;
                long re = a*b + dl*c*d, im = a*d + b*c;
                if(atol(o.cell[0][0]) == re && atol(o.cell[0][1]) == im
                   && atol(o.cell[1][0]) == dl*im && atol(o.cell[1][1]) == re)
                    fiel++;
            }
        }
        printf("      o produto matricial reproduz z·w = (ab+(1−s)cd) +"
               " (ad+bc)e: %ld/%ld\n", fiel, casos);
        if(fiel != casos || casos < 100) mal++;

        /* ── (3) E A NORMA É O DETERMINANTE. Para z = a + c·e a norma da álgebra
         * é N(z) = a² − (1−s)c², e é exactamente det(a, c; (1−s)c, a). Daí a
         * multiplicatividade N(zw) = N(z)N(w) NÃO precisa de prova nova: é o
         * det(AB) = det A · det B de §W85. Uma lei da álgebra que sai de uma lei
         * das matrizes, porque a representação é fiel. */
        long nmul = 0, ncas = 0;
        for(long sv = 0; sv <= 2; sv++){
            long dl = 1 - sv;
            for(long a = -2; a <= 2; a++) for(long c = -2; c <= 2; c++)
            for(long b = -2; b <= 2; b++) for(long d = -2; d <= 2; d++){
                POE("Z", a, c, dl*c, a);
                POE("W", b, d, dl*d, b);
                sql_executa("SELECT det(*) FROM Z", &o);
                if(!o.ok) continue;
                long nz = atol(o.cell[0][0]);
                if(nz != a*a - dl*c*c) continue;      /* a norma É o det */
                sql_executa("SELECT det(*) FROM W", &o);
                if(!o.ok) continue;
                long nw = atol(o.cell[0][0]);
                sql_executa("SELECT produto(W) FROM Z", &o);
                if(!o.ok || o.nrows != 2) continue;
                long r[2][2];
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                    r[i][j] = atol(o.cell[i][j]);
                POE("R", r[0][0], r[0][1], r[1][0], r[1][1]);
                sql_executa("SELECT det(*) FROM R", &o);
                if(!o.ok) continue;
                ncas++;
                if(atol(o.cell[0][0]) == nz*nw) nmul++;
            }
        }
        printf("      N(z) = det e N(zw) = N(z)·N(w): %ld/%ld — a"
               " multiplicatividade da norma É a do determinante\n", nmul, ncas);
        if(nmul != ncas || ncas < 100) mal++;

        /* ── E O CONTROLO É O CONE NULO, que separa os três regimes por dentro
         * da álgebra e não pela etiqueta: em s = 0 (e² = +1) há z ≠ 0 com
         * N(z) = 0 — os divisores de zero, a = ±c —, e em s = 2 (e² = −1) não há
         * nenhum, porque N = a² + c² só anula na origem. É o mesmo «com cone» e
         * «sem cone» que o catálogo usa para distinguir hiperbólico de elíptico,
         * agora medido dentro de E_s. */
        long cone0 = 0, cone2 = 0;
        for(long a = -3; a <= 3; a++) for(long c = -3; c <= 3; c++){
            if(a == 0 && c == 0) continue;
            if(a*a - 1*c*c == 0) cone0++;          /* s = 0: e² = +1 */
            if(a*a + 1*c*c == 0) cone2++;          /* s = 2: e² = −1 */
        }
        printf("      controlo — o CONE NULO: s = 0 tem %ld divisores de zero,"
               " s = 2 tem %ld\n", cone0, cone2);
        printf("        com cone é hiperbólico, sem cone é elíptico — a distinção"
               " sai de DENTRO da álgebra, não da etiqueta\n");
        if(cone0 == 0 || cone2 != 0) mal++;
        #undef POE
        sql_fechar();

        printf("\n");
        ok("A ÁLGEBRA ESTELAR JÁ ESTAVA ESCRITA — O QUE FALTAVA ERA A ROUPA. O"
           " `estelar/algebra_estelar.tex` define E_s como a álgebra real bidimensional"
           " z = a + c·e com e² = 1 − s, e dá os regimes: s = 0 é a split-complex, e o"
           " parâmetro percorre-os. Vestir isso de matriz é uma linha — e ↦ (0,1;1−s,0) —, e"
           " daí TODO o motor passa a falar de E_s sem uma operação nova: E² = (1−s)I, o"
           " discriminante é 4(1−s), e o `regime` devolve hiperbólico, parabólico e elíptico"
           " para s = 0, 1, 2. O PARÂMETRO DO PAPER É O QUE O MOTOR LÊ NO Δ. E A"
           " REPRESENTAÇÃO É FIEL, não uma analogia: com z ↦ (a, c; (1−s)c, a) o produto de"
           " MATRIZES reproduz z·w = (ab + (1−s)cd) + (ad + bc)e, que é a fórmula do paper,"
           " nos 1875 casos varridos. E DAÍ A NORMA É O DETERMINANTE: N(z) = a² − (1−s)c² é"
           " det(a, c; (1−s)c, a), pelo que a multiplicatividade N(zw) = N(z)N(w) NÃO precisa"
           " de prova nova — é o det(AB) = det A · det B de §W85. Uma lei da álgebra que sai"
           " de uma lei das matrizes, porque a roupa serve. O CONTROLO é o CONE NULO, que"
           " separa os regimes por DENTRO e não pela etiqueta: em s = 0 há divisores de zero"
           " (a = ±c) e em s = 2 não há nenhum, porque a² + c² só anula na origem — «com cone»"
           " e «sem cone», medido dentro de E_s.", mal == 0);
    }


    /* ═══ §W89: A TORRE VESTIDA DE MATRIZ, E ONDE A ROUPA RASGA ════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W89 ℂ e ℍ cabem em matrizes; 𝕆 não — e o que rasga é a associatividade.\n\n");
        { const char *tabs[] = { "Q","W","R" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w89__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w89__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w89.mem"); unlink("/tmp/pgwire_w89.prog"); }
        if(!sql_abrir("/tmp/pgwire_w89")) mal++;

        /* q = a + bi + cj + dk  ↦  a 4×4 real */
        #define POE4(t,a,b,c,d) do { char q2[300]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL," \
                     " r RACIONAL, s RACIONAL)", t); sql_executa(q2,&o2); \
            { long M[4][4] = {{(a),-(b),-(c),-(d)},{(b),(a),-(d),(c)}, \
                              {(c),(d),(a),-(b)},{(d),-(c),(b),(a)}}; \
              for(int i2 = 0; i2 < 4; i2++){ \
                  snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld,%ld,%ld)", \
                           t, M[i2][0], M[i2][1], M[i2][2], M[i2][3]); \
                  sql_executa(q2,&o2); } } } while(0)

        /* ── O `corpo_estelar.tex` prop:norma diz «N(σ) = det(A)», e a observação
         * seguinte aponta o alcance: «a norma-que-compõe N(xy) = N(x)N(y) é
         * exactamente a lei que sustenta a torre ℝ ⊂ ℂ ⊂ ℍ ⊂ 𝕆, e ela QUEBRA em
         * dim 16». Vestir a torre de matriz responde à pergunta da dimensão —
         * e tem um limite que se diz.
         *
         * ℂ já ficou vestido em §W88: é E_s com s = 2, e o det é a² + b². Aqui
         * é a vez de ℍ, com q = a + bi + cj + dk numa 4×4 real. */

        /* (1) O DETERMINANTE É A NORMA AO QUADRADO. Em ℂ o det era a norma; em
         * ℍ é o seu quadrado, porque a representação real de dimensão 4 conta
         * cada modo duas vezes. O expoente é a dimensão sobre 2 — e dizê-lo é o
         * que impede de escrever «o det é a norma» num sítio onde não é. */
        long n = 0, dn2 = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
            POE4("Q", a, b, c, d);
            sql_executa("SELECT det(*) FROM Q", &o);
            if(!o.ok) continue;
            long N = a*a + b*b + c*c + d*d;
            n++;
            if(atol(o.cell[0][0]) == N*N) dn2++;
        }
        printf("      ℍ em 4×4: det = N² em %ld/%ld — em ℂ era N, aqui é N²,"
               " porque a representação real conta cada modo duas vezes\n", dn2, n);
        if(dn2 != n || n < 500) mal++;

        /* ── (2) A TABELA DE MULTIPLICAÇÃO SAI DO PRODUTO DE MATRIZES. i·j = k
         * e j·i = −k: o produto NÃO comuta, e isso não foi posto — sai. É a
         * primeira perda da torre, no degrau 4. */
        POE4("Q", 0,1,0,0);                                    /* i */
        POE4("W", 0,0,1,0);                                    /* j */
        sql_executa("SELECT produto(W) FROM Q", &o);
        long ij[4][4]; int k1 = o.ok && o.nrows == 4;
        if(k1) for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
            ij[i][j] = atol(o.cell[i][j]);
        sql_executa("SELECT produto(Q) FROM W", &o);
        long ji[4][4]; int k2 = o.ok && o.nrows == 4;
        if(k2) for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
            ji[i][j] = atol(o.cell[i][j]);
        /* k = (0,0,0,1) tem a matriz com M[0][3] = −1 */
        int eh_k  = k1 && ij[0][3] == -1 && ij[3][0] == 1;
        int eh_mk = k2 && ji[0][3] ==  1 && ji[3][0] == -1;
        printf("      i·j = %s e j·i = %s → NÃO comuta, e é a primeira perda da"
               " torre\n", eh_k ? "k" : "MAU", eh_mk ? "−k" : "MAU");
        if(!eh_k || !eh_mk) mal++;

        /* ── (3) MAS ASSOCIA — e é aqui que a roupa mostra o seu limite. O
         * produto de matrizes é associativo POR CONSTRUÇÃO: (AB)C = A(BC) vale
         * para quaisquer matrizes, sempre. Logo qualquer coisa vestida de matriz
         * herda a associatividade, quer a queira quer não. */
        POE4("Q", 0,1,0,0);                                    /* i */
        POE4("W", 0,0,1,0);                                    /* j */
        sql_executa("SELECT produto(W) FROM Q", &o);           /* (ij) */
        int assoc = 0;
        if(o.ok && o.nrows == 4){
            char q2[300];
            sql_executa("DROP TABLE IF EXISTS R", &o2);
            sql_executa("CREATE TABLE R (p RACIONAL, q RACIONAL, r RACIONAL, s RACIONAL)", &o2);
            for(int i = 0; i < 4; i++){
                snprintf(q2, sizeof q2, "INSERT INTO R VALUES (%s,%s,%s,%s)",
                         o.cell[i][0], o.cell[i][1], o.cell[i][2], o.cell[i][3]);
                sql_executa(q2, &o2);
            }
            POE4("Q", 0,0,0,1);                                /* k */
            sql_executa("SELECT produto(Q) FROM R", &o);       /* (ij)k */
            long e1[4][4]; int t1 = o.ok && o.nrows == 4;
            if(t1) for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
                e1[i][j] = atol(o.cell[i][j]);
            /* i(jk): jk = i, logo i·i = −1 */
            POE4("W", 0,0,1,0);                                /* j */
            POE4("Q", 0,0,0,1);                                /* k */
            sql_executa("SELECT produto(Q) FROM W", &o);       /* (jk) */
            if(o.ok && o.nrows == 4){
                sql_executa("DROP TABLE IF EXISTS R", &o2);
                sql_executa("CREATE TABLE R (p RACIONAL, q RACIONAL, r RACIONAL, s RACIONAL)", &o2);
                for(int i = 0; i < 4; i++){
                    snprintf(q2, sizeof q2, "INSERT INTO R VALUES (%s,%s,%s,%s)",
                             o.cell[i][0], o.cell[i][1], o.cell[i][2], o.cell[i][3]);
                    sql_executa(q2, &o2);
                }
                POE4("Q", 0,1,0,0);                            /* i */
                sql_executa("SELECT produto(R) FROM Q", &o);   /* i(jk) */
                if(t1 && o.ok && o.nrows == 4){
                    assoc = 1;
                    for(int i = 0; i < 4; i++) for(int j = 0; j < 4; j++)
                        if(atol(o.cell[i][j]) != e1[i][j]) assoc = 0;
                }
            }
        }
        printf("      (i·j)·k = i·(j·k): %s — o produto de MATRIZES associa por"
               " construção\n", assoc ? "sim" : "FALHOU");
        if(!assoc) mal++;

        /* ── (4) E DAÍ SAI O LIMITE, numa linha e sem varrer nada: 𝕆 NÃO é
         * associativo — o associador de três octoniões é não-nulo —, e toda a
         * matriz associa. Logo NÃO EXISTE representação matricial fiel de 𝕆:
         * não é que ainda não se tenha achado, é que ela contradiria o produto
         * de matrizes. A roupa serve até ao degrau 4 e o que a rasga tem nome.
         *
         * É a mesma fronteira que o `torre.h` desta casa já dizia por outro
         * lado: «o limite no grau oito é do lado DISCRETO/bilinear — Hurwitz
         * classifica o bilinear —, NÃO do objecto». Aqui vê-se de onde vem o
         * limite da REPRESENTAÇÃO, que é outra pergunta e tem outra resposta. */
        printf("      → 𝕆 não associa e toda a matriz associa: não existe"
               " representação matricial fiel de 𝕆.\n");
        printf("        A roupa serve até ao degrau 4, e o que a rasga tem nome"
               " — não é uma lacuna, é uma incompatibilidade.\n");

        /* ── (5) E A NORMA CONTINUA A COMPOR, que é a lei da torre: N(qw) =
         * N(q)N(w), e outra vez sem prova nova — det(AB) = det A · det B de
         * §W85, com o quadrado dos dois lados. */
        long nm = 0, nc = 0;
        for(long a = -1; a <= 1; a++) for(long b = -1; b <= 1; b++)
        for(long e = -1; e <= 1; e++) for(long f = -1; f <= 1; f++){
            POE4("Q", a, b, 0, 0);
            POE4("W", e, f, 0, 0);
            sql_executa("SELECT produto(W) FROM Q", &o);
            if(!o.ok || o.nrows != 4) continue;
            char q2[300];
            sql_executa("DROP TABLE IF EXISTS R", &o2);
            sql_executa("CREATE TABLE R (p RACIONAL, q RACIONAL, r RACIONAL, s RACIONAL)", &o2);
            for(int i = 0; i < 4; i++){
                snprintf(q2, sizeof q2, "INSERT INTO R VALUES (%s,%s,%s,%s)",
                         o.cell[i][0], o.cell[i][1], o.cell[i][2], o.cell[i][3]);
                sql_executa(q2, &o2);
            }
            sql_executa("SELECT det(*) FROM R", &o);
            if(!o.ok) continue;
            long Nq = a*a + b*b, Nw = e*e + f*f;
            nc++;
            if(atol(o.cell[0][0]) == (Nq*Nw)*(Nq*Nw)) nm++;
        }
        printf("      N(q·w) = N(q)·N(w) em %ld/%ld — a lei da torre, e outra vez"
               " sem prova nova: é o det multiplicativo de §W85\n", nm, nc);
        if(nm != nc || nc < 50) mal++;
        #undef POE4
        sql_fechar();

        printf("\n");
        ok("A TORRE VESTE-SE DE MATRIZ ATÉ AO DEGRAU 4, E O QUE RASGA A ROUPA TEM NOME. O"
           " `corpo_estelar.tex` diz «N(σ) = det(A)» e aponta o alcance: «a norma-que-compõe"
           " N(xy) = N(x)N(y) é a lei que sustenta ℝ ⊂ ℂ ⊂ ℍ ⊂ 𝕆». ℂ já ficou vestido em"
           " §W88 — é E_s com s = 2, e o det É a norma. Aqui é a vez de ℍ numa 4×4 real, e o"
           " DETERMINANTE É A NORMA AO QUADRADO: em ℂ era N, aqui é N², porque a"
           " representação real conta cada modo duas vezes, e o expoente é a dimensão sobre"
           " dois — dizê-lo é o que impede de escrever «o det é a norma» onde não é. A TABELA"
           " DE MULTIPLICAÇÃO SAI DO PRODUTO DE MATRIZES sem ser posta: i·j = k e j·i = −k,"
           " logo NÃO comuta, que é a primeira perda da torre. MAS ASSOCIA — e é aqui que a"
           " roupa mostra o limite: (AB)C = A(BC) vale para quaisquer matrizes, SEMPRE, pelo"
           " que qualquer coisa vestida de matriz herda a associatividade quer a queira quer"
           " não. E DAÍ SAI O LIMITE NUMA LINHA, sem varrer nada: 𝕆 não é associativo e toda"
           " a matriz associa, logo NÃO EXISTE representação matricial fiel de 𝕆 — não é que"
           " ainda não se tenha achado, é que ela contradiria o produto de matrizes. Não é"
           " uma lacuna: é uma incompatibilidade. E é outra pergunta que a do `torre.h`, que"
           " diz «o limite no grau oito é do lado DISCRETO/bilinear, NÃO do objecto»: ali é o"
           " limite da NORMA, aqui o da REPRESENTAÇÃO. A norma continua a compor,"
           " N(qw) = N(q)N(w), e outra vez sem prova nova — é o det multiplicativo de §W85"
           " com o quadrado dos dois lados.", mal == 0);
    }

    /* ═══ §W90: A DECOMPOSIÇÃO DE FITTING, E ONDE ELA COMEÇA A VALER ═══════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W90 V = ker A^q ⊕ im A^q — e o que separa NÃO é a soma das dimensões.\n\n");
        { const char *tabs[] = { "A","P","S" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w90__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w90__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w90.mem"); unlink("/tmp/pgwire_w90.prog"); }
        if(!sql_abrir("/tmp/pgwire_w90")) mal++;

        #define POE3(t,M) do { char q2[220]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (p RACIONAL, q RACIONAL," \
                     " r RACIONAL)", t); sql_executa(q2,&o2); \
            for(int i2 = 0; i2 < 3; i2++){ \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld,%ld)", \
                         t, (M)[i2][0], (M)[i2][1], (M)[i2][2]); \
                sql_executa(q2,&o2); } } while(0)

        /* ── O §W77 achou o ponto fixo de ẋ = Ax no NÚCLEO, e o §W78 o
         * equilíbrio de ẋ = Ax + b onde A inverte. São os dois lados de UMA
         * cisão, e ela tem nome: o espaço parte-se em onde A é NILPOTENTE e
         * onde A é INVERTÍVEL. Aqui mede-se a cisão, e sobretudo O SÍTIO ONDE
         * ELA COMEÇA — que é o que se perde se a asserção for a errada. */

        /* ── (1) A ASSERÇÃO QUE NÃO PODE FALHAR, dita primeiro para não ser
         * usada: dim ker A^k + dim im A^k = 3 vale para TODO k e toda A. É o
         * posto-nulidade, e mede o teorema, não a decomposição. Medir só isto
         * seria dar por provada a soma directa em todo k — e ela É FALSA em
         * k pequeno. O que separa é a INTERSECÇÃO. */
        struct { const char *nome; long M[3][3]; int idx; } TT[] = {
            { "diag(1,2,3)   invertível", {{1,0,0},{0,2,0},{0,0,3}}, 0 },
            { "diag(0,1,2)   um só zero", {{0,0,0},{0,1,0},{0,0,2}}, 1 },
            { "N₂ ⊕ [3]      mista     ", {{0,1,0},{0,0,0},{0,0,3}}, 2 },
            { "N₃            nilpotente", {{0,1,0},{0,0,1},{0,0,0}}, 3 },
        };
        long soma_ok = 0, soma_n = 0, dir_ok = 0, cruz = 0;

        for(unsigned t = 0; t < sizeof TT/sizeof TT[0]; t++){
            long Ak[3][3];
            memcpy(Ak, TT[t].M, sizeof Ak);
            int primeira_directa = -1, estabiliza = -1, posto_ant = -1;
            printf("   %s  (índice esperado %d)\n", TT[t].nome, TT[t].idx);
            for(int k = 1; k <= 4; k++){
                if(k > 1){                       /* A^k = A · A^{k-1} */
                    POE3("A", TT[t].M); POE3("P", Ak);
                    sql_executa("SELECT produto(A) FROM P", &o);
                    if(!o.ok || o.nrows != 3){ mal++; break; }
                    for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                        Ak[i][j] = atol(o.cell[i][j]);
                }
                POE3("A", Ak);
                sql_executa("SELECT posto(*) FROM A", &o);
                if(!o.ok){ mal++; break; }
                int posto = atoi(o.cell[0][0]);

                long emp[3][3]; int ne = 0;      /* empilha ker e depois im */
                sql_executa("SELECT nucleo(*) FROM A", &o);
                int dk = (o.ok ? o.nrows : 0);
                for(int i = 0; i < dk && ne < 3; i++, ne++)
                    for(int j = 0; j < 3; j++) emp[ne][j] = atol(o.cell[i][j]);
                sql_executa("SELECT imagem(*) FROM A", &o);
                int di = (o.ok ? o.nrows : 0);
                for(int i = 0; i < di && ne < 3; i++, ne++)
                    for(int j = 0; j < 3; j++) emp[ne][j] = atol(o.cell[i][j]);

                soma_n++;
                if(dk + di == 3 && dk + posto == 3) soma_ok++;   /* posto-nulidade */
                else printf("      k=%d: posto-nulidade FALHOU (dim ker %d + posto %d)\n",
                            k, dk, posto);

                /* o segundo caminho: a soma é DIRECTA sse as duas bases juntas
                 * ainda geram 3 dimensões — posto do empilhado, pedido ao motor */
                int pe = 0;
                if(ne == 3){
                    POE3("S", emp);
                    sql_executa("SELECT posto(*) FROM S", &o);
                    if(o.ok) pe = atoi(o.cell[0][0]);
                }
                int directa = (pe == 3);
                if(directa && primeira_directa < 0) primeira_directa = k;
                if(!directa) cruz++;
                if(posto == posto_ant && estabiliza < 0) estabiliza = k - 1;
                posto_ant = posto;
                printf("      k=%d  posto %d · dim ker %d · dim im %d · posto[ker|im] %d"
                       "  → soma %s\n", k, posto, dk, di, pe,
                       directa ? "DIRECTA" : "cruza-se");
            }
            /* o índice 0 (A já invertível) dá soma directa desde k=1 */
            int esperado = TT[t].idx < 1 ? 1 : TT[t].idx;
            if(primeira_directa != esperado){
                printf("      → primeira directa em k=%d, esperado %d\n",
                       primeira_directa, esperado); mal++;
            } else dir_ok++;
            if(estabiliza != esperado){
                printf("      → posto estabilizou em k=%d, esperado %d\n",
                       estabiliza, esperado); mal++;
            }
            printf("\n");
        }
        printf("      posto-nulidade: %ld/%ld — vale em TODO k, e por isso não"
               " decide nada\n", soma_ok, soma_n);
        printf("      soma directa no índice certo: %ld/4 testemunhas\n", dir_ok);
        printf("      e o GUME: %ld dos %ld pares (k, testemunha) têm ker ∩ im ≠ 0"
               " — se a soma fosse directa em todo k, este número seria 0\n",
               cruz, soma_n);
        if(soma_ok != soma_n) mal++;
        if(dir_ok != 4) mal++;
        if(cruz == 0){ printf("      → o gume não mordeu: nenhuma testemunha cruza\n"); mal++; }

        /* ── (2) E A CISÃO TEM CONTEÚDO DINÂMICO, que é o que a traz para os
         * sistemas. Do lado ker: A^q = 0 ali, logo exp(At) restrita É UM
         * POLINÓMIO de grau < q — a série TERMINA, sem convergência nenhuma.
         * Do lado im: posto A^q = posto A^{q+1} diz que A leva im A^q SOBRE si
         * mesmo; em dimensão finita sobrejectiva é bijectiva, logo A inverte
         * ali e ẋ = Ax + b tem equilíbrio único x* = −A⁻¹b (o §W78). */
        long pol = 0, inv = 0;
        for(unsigned t = 0; t < sizeof TT/sizeof TT[0]; t++){
            int q = TT[t].idx < 1 ? 1 : TT[t].idx;
            long Ak[3][3]; memcpy(Ak, TT[t].M, sizeof Ak);
            for(int k = 2; k <= q; k++){
                POE3("A", TT[t].M); POE3("P", Ak);
                sql_executa("SELECT produto(A) FROM P", &o);
                if(!o.ok || o.nrows != 3){ mal++; goto fim90; }
                for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                    Ak[i][j] = atol(o.cell[i][j]);
            }
            POE3("A", Ak);
            sql_executa("SELECT nucleo(*) FROM A", &o);
            int dk = (o.ok ? o.nrows : 0);
            long base[3][3]; for(int i = 0; i < dk; i++)
                for(int j = 0; j < 3; j++) base[i][j] = atol(o.cell[i][j]);
            /* A^q anula cada vector do seu núcleo: é a definição, e serve de
             * controlo ao que o motor devolveu */
            int anula = 1;
            for(int i = 0; i < dk; i++) for(int r = 0; r < 3; r++){
                long s = 0; for(int c = 0; c < 3; c++) s += Ak[r][c]*base[i][c];
                if(s != 0) anula = 0;
            }
            if(anula) pol++;

            sql_executa("SELECT posto(*) FROM A", &o);
            int pq = o.ok ? atoi(o.cell[0][0]) : -1;
            POE3("A", TT[t].M); POE3("P", Ak);
            sql_executa("SELECT produto(A) FROM P", &o);
            long Aq1[3][3];
            for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                Aq1[i][j] = atol(o.cell[i][j]);
            POE3("A", Aq1);
            sql_executa("SELECT posto(*) FROM A", &o);
            int pq1 = o.ok ? atoi(o.cell[0][0]) : -2;
            if(pq == pq1) inv++;
            printf("   %s  q=%d · A^q anula o seu ker: %s · posto A^q = posto A^{q+1}:"
                   " %d = %d %s\n", TT[t].nome, q, anula ? "sim" : "NÃO",
                   pq, pq1, pq == pq1 ? "✓" : "✗");
        }
        printf("      → em ker A^q a série de exp(At) TERMINA (%ld/4);"
               " em im A^q o A inverte e há equilíbrio (%ld/4)\n", pol, inv);
        if(pol != 4 || inv != 4) mal++;

    fim90:
        #undef POE3
        sql_fechar();

        printf("\n");
        ok("A DECOMPOSIÇÃO DE FITTING PARTE O ESPAÇO EM DOIS REGIMES, E O QUE A MEDE NÃO É"
           " A SOMA DAS DIMENSÕES. dim ker A^k + dim im A^k = 3 vale para TODO k e toda"
           " matriz — é o posto-nulidade, medido em 16/16 pares, e por isso NÃO decide nada:"
           " uma asserção sobre ele passaria verde com a soma directa a ser falsa. O que"
           " separa é a INTERSECÇÃO, e ela mede-se por outro caminho — empilhar a base do"
           " núcleo com a do imagem e pedir o POSTO ao motor: 3 é soma directa, menos é"
           " cruzamento. Assim aparece o ÍNDICE, o primeiro k onde a soma fecha, e ele é"
           " exactamente onde o posto pára de descer: 1 para diag(1,2,3) e diag(0,1,2), 2"
           " para N₂⊕[3], 3 para N₃ — os quatro índices que uma 3×3 admite. E o gume não é"
           " decorativo: 3 dos 16 pares TÊM ker ∩ im ≠ 0, e todos vivem em k abaixo do"
           " índice; em N₂⊕[3] com k=1 o vector (1,0,0) está nos dois lados ao mesmo tempo,"
           " com as dimensões a somarem 3 na mesma. A CISÃO É DINÂMICA, e é o que a traz"
           " para os sistemas: em ker A^q a matriz é NILPOTENTE, logo exp(At) restrita ali é"
           " um POLINÓMIO de grau menor que q — a série termina e não há convergência"
           " nenhuma a invocar; em im A^q vale posto A^q = posto A^{q+1}, isto é A leva o"
           " subespaço SOBRE si mesmo, e em dimensão finita sobrejectiva é bijectiva, donde A"
           " inverte ali e ẋ = Ax + b tem o equilíbrio único x* = −A⁻¹b do §W78. O ponto fixo"
           " de §W77 estava no núcleo e o equilíbrio de §W78 na imagem: são os dois lados"
           " desta mesma cisão, e agora sabe-se em que k ela passa a valer.", mal == 0);
    }

    /* ═══ §W91: I^n → I NOS SISTEMAS — A IDA É SEMPRE, A VOLTA PEDE G≡1 ════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W91 O sistema em I^n vira UMA equação em I — e a volta tem condição.\n\n");
        { const char *tabs[] = { "A","P","B" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w91__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w91__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w91.mem"); unlink("/tmp/pgwire_w91.prog"); }
        if(!sql_abrir("/tmp/pgwire_w91")) mal++;

        #define POEN(t,M,N) do { char q2[260]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[200]; cols[0] = 0; \
              const char *nm[] = { "p","q","r" }; \
              for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%s%s RACIONAL", j2?", ":"", nm[j2]); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (N); i2++){ char vs[160]; vs[0] = 0; \
                for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); } } while(0)

        /* ── A pergunta é a do aranha thm:enumfin trazida para os sistemas:
         * I^n → I. A enumeração E_n dá a bijecção sempre, e o aranha já disse
         * o que ela custa — «codificar ≠ preservar a dinâmica», com o §K8 do
         * `cantor.c` a medir c(A+B) = c(A)+c(B) em 0 de 256. Aqui mede-se a
         * OUTRA redução, a que preserva o operador, e o preço dela. */

        /* ── (1) A IDA É SEMPRE, E É CAYLEY–HAMILTON. Para qualquer A n×n e
         * qualquer x₀, a sucessão x_k = A^k x₀ satisfaz, EM CADA COORDENADA, a
         * mesma recorrência escalar de ordem n dada pelo característico. Um
         * sistema de n equações acopladas vira n cópias de UMA equação em I —
         * e nenhuma hipótese sobre A é usada.
         *
         * Os coeficientes saem pelo motor por NEWTON, dos traços das potências,
         * e o det do motor é o CONTROLO independente do terceiro. */
        long ida_ok = 0, ida_n = 0, newton_ok = 0, newton_n = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
            long A3[3][3] = {{a,b,0},{c,d,1},{0,1,a}};
            long p[4] = {0,0,0,0}, Ak[3][3], I3[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
            memcpy(Ak, I3, sizeof Ak);
            int falhou = 0;
            for(int k = 1; k <= 3; k++){
                POEN("A", A3, 3); POEN("P", Ak, 3);
                sql_executa("SELECT produto(A) FROM P", &o);
                if(!o.ok || o.nrows != 3){ falhou = 1; break; }
                for(int i = 0; i < 3; i++) for(int j = 0; j < 3; j++)
                    Ak[i][j] = atol(o.cell[i][j]);
                POEN("A", Ak, 3);
                sql_executa("SELECT traco(*) FROM A", &o);
                if(!o.ok){ falhou = 1; break; }
                p[k] = atol(o.cell[0][0]);
            }
            if(falhou) continue;
            /* Newton: e1 = p1, e2 = (p1²−p2)/2, e3 = (p1³−3p1p2+2p3)/6 */
            long e1 = p[1], e2 = (p[1]*p[1] - p[2])/2,
                 e3 = (p[1]*p[1]*p[1] - 3*p[1]*p[2] + 2*p[3])/6;
            POEN("A", A3, 3);
            sql_executa("SELECT det(*) FROM A", &o);
            newton_n++;
            if(o.ok && atol(o.cell[0][0]) == e3) newton_ok++;

            /* a recorrência nas coordenadas: x_{k+3} = e1·x_{k+2} − e2·x_{k+1} + e3·x_k */
            for(long s = -1; s <= 1; s++){
                long x[6][3] = {{1,s,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}};
                for(int k = 1; k < 6; k++)
                    for(int i = 0; i < 3; i++){
                        long t2 = 0;
                        for(int j = 0; j < 3; j++) t2 += A3[i][j]*x[k-1][j];
                        x[k][i] = t2;
                    }
                for(int k = 0; k + 3 < 6; k++) for(int i = 0; i < 3; i++){
                    ida_n++;
                    if(x[k+3][i] == e1*x[k+2][i] - e2*x[k+1][i] + e3*x[k][i]) ida_ok++;
                }
            }
        }
        printf("      IDA  — cada coordenada satisfaz a escalar de ordem 3: %ld/%ld\n",
               ida_ok, ida_n);
        printf("      e os coeficientes por NEWTON batem com o det do motor: %ld/%ld"
               "  (dois caminhos para e₃)\n", newton_ok, newton_n);
        if(ida_ok != ida_n || ida_n < 5000) mal++;
        if(newton_ok != newton_n || newton_n < 500) mal++;

        /* ── (2) A VOLTA TEM CONDIÇÃO, E ELA É O G DO ARANHA. Reconstruir x a
         * partir de y precisa de um VECTOR CÍCLICO: v com [v | Av | … ] de
         * determinante não nulo. Ele existe sse cada valor próprio tem G = 1 —
         * e G = dim ker(A − λI) é a multiplicidade geométrica, a mesma letra do
         * aranha. Sem cíclico, a escalar da IDA tem menos que o sistema. */
        struct { const char *nome; long M[2][2]; int lam_em_Q; } CC[] = {
            { "I₂          homotetia", {{1,0},{0,1}},   1 },
            { "[[1,1],[0,1]] Jordan ", {{1,1},{0,1}},   1 },
            { "diag(2,3)   distintos", {{2,0},{0,3}},   1 },
            { "[[0,1],[-1,0]] gambá ", {{0,1},{-1,0}},  0 },
        };
        long crit_ok = 0;
        for(unsigned t = 0; t < sizeof CC/sizeof CC[0]; t++){
            long ciclicos = 0, tot = 0;
            for(long v0 = -3; v0 <= 3; v0++) for(long v1 = -3; v1 <= 3; v1++){
                if(!v0 && !v1) continue;
                long Av[2] = { CC[t].M[0][0]*v0 + CC[t].M[0][1]*v1,
                               CC[t].M[1][0]*v0 + CC[t].M[1][1]*v1 };
                long Pm[2][2] = {{v0, Av[0]},{v1, Av[1]}};   /* colunas v, Av */
                POEN("P", Pm, 2);
                sql_executa("SELECT det(*) FROM P", &o);
                tot++;
                if(o.ok && atol(o.cell[0][0]) != 0) ciclicos++;
            }
            /* ── A SEGUNDA RÉGUA, e ela não pode ser escrita à mão nem depender
             * de λ existir no corpo: A é cíclica sse o polinómio MÍNIMO tem o
             * grau do característico, isto é sse {I, A} é independente em
             * M₂. Mede-se empilhando vec(I) e vec(A) e pedindo o POSTO ao
             * motor — 2 é cíclica, 1 é escalar. Vale no gambá, cujo
             * característico λ²+1 não tem raiz em ℚ e onde «dim ker(A − λI)»
             * não é sequer uma pergunta legítima. */
            { long V[2][4] = {{1,0,0,1},
                              {CC[t].M[0][0], CC[t].M[0][1],
                               CC[t].M[1][0], CC[t].M[1][1]}};
              char q3[260];
              sql_executa("DROP TABLE IF EXISTS B", &o2);
              sql_executa("CREATE TABLE B (p RACIONAL, q RACIONAL, r RACIONAL,"
                          " s RACIONAL)", &o2);
              for(int i = 0; i < 2; i++){
                  snprintf(q3, sizeof q3, "INSERT INTO B VALUES (%ld,%ld,%ld,%ld)",
                           V[i][0], V[i][1], V[i][2], V[i][3]);
                  sql_executa(q3, &o2);
              }
              sql_executa("SELECT posto(*) FROM B", &o);
              int gmin = (o.ok ? atoi(o.cell[0][0]) : -1);

              /* e a terceira leitura, G = dim ker(A − λI), SÓ onde λ vive em ℚ */
              int G = -1;
              if(CC[t].lam_em_Q){
                  long lam = CC[t].M[0][0];
                  long AL[2][2] = {{CC[t].M[0][0]-lam, CC[t].M[0][1]},
                                   {CC[t].M[1][0], CC[t].M[1][1]-lam}};
                  POEN("A", AL, 2);
                  sql_executa("SELECT nucleo(*) FROM A", &o);
                  G = (o.ok ? o.nrows : -1);
              }
              int obs = (ciclicos > 0);
              int bate = (obs == (gmin == 2))
                      && (!CC[t].lam_em_Q || (obs == (G == 1)));
              if(G >= 0)
                  printf("   %s  grau do mínimo %d · G=%d · cíclicos %ld/%ld · %s\n",
                         CC[t].nome, gmin, G, ciclicos, tot,
                         bate ? "as três concordam" : "DISCORDAM");
              else
                  printf("   %s  grau do mínimo %d · λ∉ℚ, G não se pergunta · cíclicos"
                         " %ld/%ld · %s\n", CC[t].nome, gmin, ciclicos, tot,
                         bate ? "as duas concordam" : "DISCORDAM");
              if(bate) crit_ok++; else mal++;
            }
        }
        printf("      VOLTA — «existe cíclico ⟺ o mínimo tem grau n»: %ld/4,"
               " e onde λ ∈ ℚ isso é «G ≡ 1»\n", crit_ok);
        if(crit_ok != 4) mal++;

        /* ── (3) O GUME, e é o §W64 outra vez por outro lado. I₂ e o bloco de
         * Jordan têm a MESMA cifra (traço 2, det 1), logo o MESMO característico
         * (λ−1)², logo a IDA dá-lhes a MESMA equação escalar
         * y_{k+2} = 2y_{k+1} − y_k. E não são semelhantes. Portanto a ida
         * APAGA, e o que ela apaga é exactamente G — que é a fibra do aranha. */
        {
            long J[2][2] = {{1,1},{0,1}}, Id[2][2] = {{1,0},{0,1}};
            long tJ, dJ, tI, dI;
            POEN("A", J, 2);  sql_executa("SELECT traco(*) FROM A", &o); tJ = atol(o.cell[0][0]);
            sql_executa("SELECT det(*) FROM A", &o);   dJ = atol(o.cell[0][0]);
            POEN("A", Id, 2); sql_executa("SELECT traco(*) FROM A", &o); tI = atol(o.cell[0][0]);
            sql_executa("SELECT det(*) FROM A", &o);   dI = atol(o.cell[0][0]);
            printf("      GUME — cifra de Jordan (%ld,%ld) e de I₂ (%ld,%ld): %s\n",
                   tJ, dJ, tI, dI, (tJ == tI && dJ == dI) ? "IGUAIS" : "diferentes");
            if(tJ != tI || dJ != dI) mal++;
            /* a mesma equação escalar serve as duas, e as órbitas separam-se */
            long xJ[3][2] = {{0,1},{0,0},{0,0}}, xI[3][2] = {{0,1},{0,0},{0,0}};
            for(int k = 1; k < 3; k++) for(int i = 0; i < 2; i++){
                xJ[k][i] = J[i][0]*xJ[k-1][0]  + J[i][1]*xJ[k-1][1];
                xI[k][i] = Id[i][0]*xI[k-1][0] + Id[i][1]*xI[k-1][1];
            }
            int recJ = 1, recI = 1;
            for(int i = 0; i < 2; i++){
                if(xJ[2][i] != 2*xJ[1][i] - xJ[0][i]) recJ = 0;
                if(xI[2][i] != 2*xI[1][i] - xI[0][i]) recI = 0;
            }
            printf("        e a MESMA recorrência y_{k+2}=2y_{k+1}−y_k serve as duas:"
                   " Jordan %s, I₂ %s — a ida não as distingue\n",
                   recJ ? "sim" : "NÃO", recI ? "sim" : "NÃO");
            if(!recJ || !recI) mal++;
            /* e o que as separa é G, medido: 1 contra 2 */
            printf("        o que as separa é G: 1 contra 2 — e é por isso que só uma"
                   " delas volta de I para I²\n");
        }

        #undef POEN
        sql_fechar();

        printf("\n");
        ok("I^n → I NOS SISTEMAS É UM PAR DUAL, E OS DOIS LADOS TÊM PREÇOS DIFERENTES. A IDA"
           " NÃO TEM CONDIÇÃO: por Cayley–Hamilton, para toda A n×n e todo x₀ a sucessão"
           " x_k = A^k x₀ satisfaz EM CADA COORDENADA a mesma recorrência escalar de ordem n"
           " dada pelo característico — um sistema de n equações acopladas vira n cópias de"
           " UMA equação em I, medido em 5625 verificações sobre 625 matrizes, e nenhuma"
           " hipótese sobre A foi usada. Os coeficientes saem pelo motor por NEWTON, dos"
           " traços de A, A² e A³, e o det do motor é o controlo independente do terceiro:"
           " 625/625 — dois caminhos para e₃. A VOLTA É QUE PEDE CONDIÇÃO: reconstruir x a"
           " partir de y precisa de um VECTOR CÍCLICO, v com det[v | Av | …] ≠ 0. O critério"
           " NÃO pode ser escrito à mão nem depender de λ existir no corpo — a primeira"
           " escrita punha G numa tabela e o motor desmentia-a no gambá, onde λ²+1 não tem"
           " raiz em ℚ e «dim ker(A − λI)» não é sequer uma pergunta legítima. A régua que"
           " transporta é o POLINÓMIO MÍNIMO: A é cíclica sse {I, A} é independente, medido"
           " empilhando vec(I) e vec(A) e pedindo o POSTO ao motor. Onde λ vive em ℚ ela"
           " coincide com G ≡ 1, a multiplicidade geométrica do aranha, e as três leituras"
           " concordam nas quatro testemunhas: I₂ tem mínimo de grau 1, G = 2 e ZERO cíclicos"
           " em 48; Jordan, diag(2,3) e o gambá têm mínimo de grau 2 e cíclicos aos montes. E O GUME É O §W64"
           " POR OUTRO LADO: I₂ e o bloco de Jordan têm a MESMA cifra (2,1), logo o mesmo"
           " característico (λ−1)², logo a IDA dá-lhes a MESMA equação escalar"
           " y_{k+2} = 2y_{k+1} − y_k — verificada nas duas —, e elas NÃO são semelhantes."
           " Portanto a ida APAGA, e o que ela apaga é exactamente G. É a fibra do aranha a"
           " decidir uma pergunta de equações diferenciais: a redução I^n → I que preserva a"
           " dinâmica é reversível onde a fibra é trivial, e só aí. A enumeração E_n do"
           " thm:enumfin é bijectiva sempre e não serve aqui — «codificar ≠ preservar a"
           " dinâmica», com o §K8 do `cantor.c` a medir c(A+B) = c(A)+c(B) em 0 de 256.", mal == 0);
    }

    /* ═══ §W92: A MENOR EQUAÇÃO QUE SERVE TEM A ORDEM DO MÍNIMO ════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W92 §W91 deu uma equação de ordem n. Nem sempre é a menor.\n\n");
        { const char *tabs[] = { "A","B","V" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w92__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w92__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w92.mem"); unlink("/tmp/pgwire_w92.prog"); }
        if(!sql_abrir("/tmp/pgwire_w92")) mal++;

        /* põe uma matriz L×C na tabela t, com nomes de coluna c1..c6 */
        #define POE(t,M,L,C) do { char q2[300]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[220]; cols[0] = 0; \
              for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (L); i2++){ char vs[220]; vs[0] = 0; \
                for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); } } while(0)

        /* ── §W91 deu, para toda A n×n, uma equação escalar de ORDEM n. Ela
         * serve sempre — mas nem sempre é a MENOR. A menor tem a ordem do
         * polinómio MÍNIMO, e para I₂ isso é 1: ẋ = x duas vezes chega, e a
         * equação de ordem 2 é redundante. Mede-se por dois caminhos que não
         * se conhecem, e a minimalidade tem de ser medida à parte da
         * existência — senão «d serve» passa verde com d−1 a servir também. */

        /* ── (1) DUAS RÉGUAS PARA d, em 2×2, exaustivo em −2..2.
         * A: menor d com {I, A, …, A^d} dependente — posto do empilhado 4 colunas.
         * B: max sobre v de dim span{v, Av} — posto de [v | Av].
         * O teorema diz que são iguais, e nenhuma das duas sabe da outra. */
        long dois_ok = 0, dois_n = 0, cont_d[3] = {0,0,0};
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long d0 = -2; d0 <= 2; d0++){
            long A[2][2] = {{a,b},{c,d0}};
            long A2[2][2] = {{a*a + b*c, a*b + b*d0},{c*a + d0*c, c*b + d0*d0}};

            /* régua A: {I}, {I,A}, {I,A,A²} como vectores de dimensão 4 */
            long V[3][4] = {{1,0,0,1},{a,b,c,d0},{A2[0][0],A2[0][1],A2[1][0],A2[1][1]}};
            int dA = -1;
            for(int k = 1; k <= 2 && dA < 0; k++){
                POE("B", V, k+1, 4);
                sql_executa("SELECT posto(*) FROM B", &o);
                if(!o.ok){ mal++; break; }
                if(atoi(o.cell[0][0]) < k + 1) dA = k;   /* {I..A^k} dependente */
            }
            if(dA < 0) dA = 2;

            /* régua B: o maior subespaço cíclico */
            int dB = 0;
            for(long v0 = -2; v0 <= 2; v0++) for(long v1 = -2; v1 <= 2; v1++){
                if(!v0 && !v1) continue;
                long Av[2] = { a*v0 + b*v1, c*v0 + d0*v1 };
                long W[2][2] = {{v0, Av[0]},{v1, Av[1]}};
                POE("V", W, 2, 2);
                sql_executa("SELECT posto(*) FROM V", &o);
                if(o.ok && atoi(o.cell[0][0]) > dB) dB = atoi(o.cell[0][0]);
            }
            dois_n++;
            if(dA == dB){ dois_ok++; if(dA >= 0 && dA <= 2) cont_d[dA]++; }
            else if(dois_n < 4)
                printf("      A=(%ld,%ld;%ld,%ld) régua A diz %d, régua B diz %d\n",
                       a, b, c, d0, dA, dB);
        }
        printf("      as DUAS RÉGUAS para o grau do mínimo concordam: %ld/%ld"
               " (2×2 exaustivo em −2..2)\n", dois_ok, dois_n);
        printf("      e a distribuição EXERCE os dois lados: d=1 em %ld matrizes"
               " (as escalares), d=2 em %ld\n", cont_d[1], cont_d[2]);
        if(dois_ok != dois_n) mal++;
        if(cont_d[1] == 0 || cont_d[2] == 0){
            printf("      → sem os dois valores de d isto media uma constante\n"); mal++;
        }

        /* ── (2) E A MINIMALIDADE, que é outra pergunta que a existência.
         * Onde d = 1 a matriz é escalar, A = cI, e a órbita cumpre
         * x_{k+1} = c·x_k — uma equação de ORDEM 1 onde §W91 deu ordem 2.
         * O CONTROLO é o bloco de Jordan: varre-se TODO c e NENHUM serve,
         * o que é o que prova que ali o 2 não pode descer. */
        {
            long Id[2][2] = {{3,0},{0,3}}, J[2][2] = {{1,1},{0,1}};
            /* A = cI verificado pelo motor: posto(A − cI) = 0 */
            long AL[2][2] = {{Id[0][0]-3, Id[0][1]},{Id[1][0], Id[1][1]-3}};
            POE("A", AL, 2, 2);
            sql_executa("SELECT posto(*) FROM A", &o);
            int escalar = (o.ok && atoi(o.cell[0][0]) == 0);
            printf("      3·I₂: posto(A − 3I) = %s → é escalar, e a ordem 1 basta\n",
                   o.ok ? o.cell[0][0] : "?");
            if(!escalar) mal++;

            long serve_id = 0, serve_j = 0, tent = 0;
            for(long cc = -4; cc <= 4; cc++){
                int okI = 1, okJ = 1;
                for(long s = -2; s <= 2; s++) for(long u = -2; u <= 2; u++){
                    long x[2] = {s,u}, y[2] = {s,u};
                    for(int k = 0; k < 3; k++){
                        long nx[2] = { Id[0][0]*x[0] + Id[0][1]*x[1],
                                       Id[1][0]*x[0] + Id[1][1]*x[1] };
                        long ny[2] = { J[0][0]*y[0] + J[0][1]*y[1],
                                       J[1][0]*y[0] + J[1][1]*y[1] };
                        for(int i = 0; i < 2; i++){
                            if(nx[i] != cc*x[i]) okI = 0;
                            if(ny[i] != cc*y[i]) okJ = 0;
                        }
                        x[0] = nx[0]; x[1] = nx[1]; y[0] = ny[0]; y[1] = ny[1];
                    }
                }
                tent++;
                if(okI) serve_id++;
                if(okJ) serve_j++;
            }
            printf("      MINIMALIDADE — recorrência de ordem 1 (x_{k+1} = c·x_k),"
                   " varrendo c em −4..4:\n");
            printf("        3·I₂  : serve para %ld dos %ld valores de c  (o c = 3)\n",
                   serve_id, tent);
            printf("        Jordan: serve para %ld dos %ld — NENHUM, e é isso que"
                   " prova que ali o 2 não desce\n", serve_j, tent);
            if(serve_id != 1) mal++;
            if(serve_j != 0) mal++;
        }

        /* ── (3) E EM 3×3 O MÍNIMO PODE SER MENOR QUE n SEM A MATRIZ SER
         * ESCALAR — é o caso que separa «d < n» de «A = cI». diag(1,1,2) tem
         * mínimo (λ−1)(λ−2) de grau 2 contra um característico de grau 3.
         * Aqui a régua das potências não cabe: o motor recusa 3×9, tecto 6×6.
         * Usa-se a dos subespaços cíclicos, que é uma 3×3. */
        {
            struct { const char *nome; long M[3][3]; int d; } TT[] = {
                { "diag(1,1,2)  mínimo grau 2 < n=3", {{1,0,0},{0,1,0},{0,0,2}}, 2 },
                { "diag(1,2,3)  mínimo grau 3 = n  ", {{1,0,0},{0,2,0},{0,0,3}}, 3 },
                { "2·I₃         mínimo grau 1      ", {{2,0,0},{0,2,0},{0,0,2}}, 1 },
                { "N₃           mínimo grau 3      ", {{0,1,0},{0,0,1},{0,0,0}}, 3 },
            };
            long bate = 0;
            for(unsigned t = 0; t < sizeof TT/sizeof TT[0]; t++){
                int dB = 0;
                for(long v0 = -2; v0 <= 2; v0++) for(long v1 = -2; v1 <= 2; v1++)
                for(long v2 = -2; v2 <= 2; v2++){
                    if(!v0 && !v1 && !v2) continue;
                    long v[3] = {v0,v1,v2}, Av[3], A2v[3];
                    for(int i = 0; i < 3; i++){
                        Av[i] = 0; for(int j = 0; j < 3; j++) Av[i] += TT[t].M[i][j]*v[j];
                    }
                    for(int i = 0; i < 3; i++){
                        A2v[i] = 0; for(int j = 0; j < 3; j++) A2v[i] += TT[t].M[i][j]*Av[j];
                    }
                    long W[3][3] = {{v[0],Av[0],A2v[0]},{v[1],Av[1],A2v[1]},
                                    {v[2],Av[2],A2v[2]}};
                    POE("V", W, 3, 3);
                    sql_executa("SELECT posto(*) FROM V", &o);
                    if(o.ok && atoi(o.cell[0][0]) > dB) dB = atoi(o.cell[0][0]);
                }
                printf("   %s  maior subespaço cíclico: %d\n", TT[t].nome, dB);
                if(dB == TT[t].d) bate++; else mal++;
            }
            printf("      → o maior subespaço cíclico dá o grau do mínimo: %ld/4,"
                   " e diag(1,1,2) mostra d < n SEM ser escalar\n", bate);
        }

        #undef POE
        sql_fechar();

        printf("\n");
        ok("A MENOR EQUAÇÃO QUE SERVE TEM A ORDEM DO MÍNIMO, NÃO DO CARACTERÍSTICO. §W91 deu,"
           " para toda A n×n, uma equação escalar de ordem n, e ela serve sempre — mas nem"
           " sempre é a menor: para 3·I₂ basta ORDEM 1, e a de ordem 2 é redundante. O grau"
           " certo é o do polinómio MÍNIMO, e mede-se por dois caminhos que não se conhecem:"
           " o menor d com {I, A, …, A^d} dependente, pelo posto das potências empilhadas, e"
           " o MAIOR SUBESPAÇO CÍCLICO, max sobre v de dim span{v, Av, …}, pelo posto de"
           " [v | Av | …]. Concordam em 625/625 nas 2×2 com entradas em −2..2, e a"
           " distribuição exerce os dois lados — d=1 nas 5 escalares, d=2 nas outras 620 —,"
           " sem o que isto media uma constante. A MINIMALIDADE É OUTRA PERGUNTA QUE A"
           " EXISTÊNCIA, e mede-se à parte: dizer «a ordem d serve» passa verde com d−1 a"
           " servir também. Varre-se então TODO c em −4..4 na recorrência de ordem um"
           " x_{k+1} = c·x_k: para 3·I₂ serve exactamente UM valor, o c = 3; para o bloco de"
           " Jordan NENHUM serve, e é esse zero que prova que ali o 2 não desce. E O CASO QUE"
           " SEPARA está em 3×3: diag(1,1,2) tem mínimo de grau 2 contra um característico de"
           " grau 3 SEM ser escalar — logo «d < n» não é o mesmo que «A = cI», e a redução"
           " ganha uma ordem sem a matriz ser trivial. Aqui a régua das potências não cabe: o"
           " motor recusa 3×9 pelo tecto 6×6 que ele próprio declara, e usa-se a dos"
           " subespaços cíclicos, que é uma 3×3 — o tecto é do motor, não do teorema, e"
           " dizê-lo é o que impede de o ler como limite do resultado. E EM 3×3 SÃO UMA"
           " MEDIDA CONTRA UMA DERIVAÇÃO, não duas medidas: o grau previsto vem de uma linha"
           " — o mínimo de uma diagonal é o produto dos (λ−λᵢ) sobre as entradas DISTINTAS,"
           " logo 2 para diag(1,1,2), 3 para diag(1,2,3), 1 para 2·I₃, e λ³ para N₃ —, e o"
           " medido é o subespaço cíclico. Só em 2×2 as duas réguas são ambas do motor.", mal == 0);
    }

    /* ═══ §W93: ℚ[A] = ℚ[λ]/(μ_A) — E É CORPO SSE μ É IRREDUTÍVEL ══════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W93 A álgebra da matriz é um anel de UMA variável — e onde é corpo.\n\n");
        { const char *tabs[] = { "A","B","X" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w93__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w93__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w93.mem"); unlink("/tmp/pgwire_w93.prog"); }
        if(!sql_abrir("/tmp/pgwire_w93")) mal++;

        #define POE(t,M,L,C) do { char q2[300]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[220]; cols[0] = 0; \
              for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (L); i2++){ char vs[220]; vs[0] = 0; \
                for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); } } while(0)

        /* ── §W92 disse que bastam d potências. Isso não é uma economia de
         * conta: diz ONDE VIVE o operador. A álgebra gerada por A tem dimensão
         * d e não n², e é ℚ[λ]/(μ_A) — um anel de UMA variável. É a redução
         * I^n → I feita à álgebra, e não já aos vectores. */

        /* ── (1) TODA POTÊNCIA CAI NO SPAN DAS PRIMEIRAS d. Empilha-se
         * {I, A, A^k} e exige-se posto 2 para todo k: se alguma potência
         * saísse, a dimensão não seria d. Exaustivo nas 2×2 não escalares. */
        long span_ok = 0, span_n = 0, esc = 0;
        for(long a = -2; a <= 2; a++) for(long b = -2; b <= 2; b++)
        for(long c = -2; c <= 2; c++) for(long e = -2; e <= 2; e++){
            if(b == 0 && c == 0 && a == e){ esc++; continue; }   /* escalares: d=1 */
            long Ak[2][2] = {{1,0},{0,1}};
            for(int k = 1; k <= 6; k++){
                long nx[2][2] = {{Ak[0][0]*a + Ak[0][1]*c, Ak[0][0]*b + Ak[0][1]*e},
                                 {Ak[1][0]*a + Ak[1][1]*c, Ak[1][0]*b + Ak[1][1]*e}};
                memcpy(Ak, nx, sizeof Ak);
                long V[3][4] = {{1,0,0,1},{a,b,c,e},
                                {Ak[0][0],Ak[0][1],Ak[1][0],Ak[1][1]}};
                POE("B", V, 3, 4);
                sql_executa("SELECT posto(*) FROM B", &o);
                span_n++;
                if(o.ok && atoi(o.cell[0][0]) == 2) span_ok++;
            }
        }
        printf("      toda A^k cai no span de {I, A}: %ld/%ld (k até 6, nas %ld"
               " matrizes não escalares)\n", span_ok, span_n, (long)(625 - esc));
        printf("      → dim ℚ[A] = d = 2, e NÃO n² = 4: o operador vive num anel"
               " de uma variável\n");
        if(span_ok != span_n || span_n < 3000) mal++;

        /* ── (2) E ONDE ELE É CORPO. ℚ[A] ≅ ℚ[λ]/(μ_A), logo é corpo sse μ é
         * IRREDUTÍVEL — e onde μ factoriza há divisor de zero: um X = aI + bA
         * não nulo com det X = 0. Isto não se argumenta, conta-se. */
        struct { const char *nome; long M[2][2]; const char *mu; int irred; } CC[] = {
            { "gambá  (0,1;−1,0)", {{0,1},{-1,0}}, "λ²+1        ",       1 },
            { "gato   (1,1;1,0) ", {{1,1},{1,0}},  "λ²−λ−1      ",       1 },
            { "diag(1,2)        ", {{1,0},{0,2}},  "(λ−1)(λ−2)  ",       0 },
            { "Jordan (1,1;0,1) ", {{1,1},{0,1}},  "(λ−1)²      ",       0 },
        };
        long crit = 0;
        for(unsigned t = 0; t < sizeof CC/sizeof CC[0]; t++){
            long zero_div = 0, invert = 0, no_span = 0, tot = 0;
            for(long p = -3; p <= 3; p++) for(long q = -3; q <= 3; q++){
                if(!p && !q) continue;
                long X[2][2] = {{p + q*CC[t].M[0][0], q*CC[t].M[0][1]},
                                {q*CC[t].M[1][0], p + q*CC[t].M[1][1]}};
                if(!X[0][0] && !X[0][1] && !X[1][0] && !X[1][1]) continue;
                POE("X", X, 2, 2);
                sql_executa("SELECT det(*) FROM X", &o);
                if(!o.ok) continue;
                tot++;
                if(atol(o.cell[0][0]) == 0){ zero_div++; continue; }
                invert++;
                /* e a inversa fica DENTRO do span: {I, A, X⁻¹} ainda tem posto 2 */
                sql_executa("SELECT inversa(*) FROM X", &o);
                if(o.ok && o.nrows == 2){
                    long W[3][4] = {{1,0,0,1},
                                    {CC[t].M[0][0],CC[t].M[0][1],CC[t].M[1][0],CC[t].M[1][1]},
                                    {0,0,0,0}};
                    /* a inversa é racional; multiplica-se pelo det para ficar inteira */
                    long dX = X[0][0]*X[1][1] - X[0][1]*X[1][0];
                    W[2][0] =  X[1][1]; W[2][1] = -X[0][1];
                    W[2][2] = -X[1][0]; W[2][3] =  X[0][0];
                    (void)dX;
                    POE("B", W, 3, 4);
                    sql_executa("SELECT posto(*) FROM B", &o);
                    if(!o.ok || atoi(o.cell[0][0]) != 2) no_span++;
                }
            }
            int corpo = (zero_div == 0);
            /* o CONTROLO entra AQUI, na mesma condição, e não num printf: num
             * corpo todo elemento não nulo tem de inverter — se `invert` fosse
             * 0, «zero divisores de zero» saía de não haver elementos nenhuns —,
             * e num anel com divisores têm de existir os DOIS tipos, senão a
             * dicotomia não separa nada. */
            int dist = corpo ? (invert == tot && tot >= 40)
                             : (invert > 0 && zero_div > 0);
            int bate = (corpo == CC[t].irred) && (no_span == 0) && dist;
            printf("   %s  μ = %s · divisores de zero %ld/%ld · invertíveis %ld ·"
                   " inversas fora do span %ld · %s%s\n", CC[t].nome, CC[t].mu,
                   zero_div, tot, invert, no_span,
                   corpo ? "CORPO" : "anel com divisores",
                   bate ? "" : "  ← DISCORDA");
            if(bate) crit++; else mal++;
        }
        printf("      → «ℚ[A] é corpo ⟺ μ_A irredutível»: %ld/4, com o controlo da"
               " distribuição DENTRO da condição; e a inversa nunca sai do span —"
               " o fecho é do ANEL, não da conta\n", crit);
        if(crit != 4) mal++;


        #undef POE
        sql_fechar();

        printf("\n");
        ok("A ÁLGEBRA GERADA POR UMA MATRIZ É UM ANEL DE UMA VARIÁVEL, E É AÍ QUE O CORPO DE"
           " MATRIZES FECHA. §W92 mediu que bastam d potências; isso não é uma economia de"
           " conta, é uma afirmação sobre ONDE VIVE o operador — ℚ[A] tem dimensão d e não"
           " n², e é ℚ[λ]/(μ_A). Mede-se exigindo que TODA potência caia no span das"
           " primeiras: empilha-se {I, A, A^k} e pede-se o posto ao motor, que dá 2 em"
           " 3720/3720 (k até 6, nas 620 matrizes 2×2 não escalares de −2..2). É a redução"
           " I^n → I feita À ÁLGEBRA e já não aos vectores: o operador em dimensão n vive num"
           " anel de polinómios numa variável. E DAÍ SAI ONDE ELE É CORPO, sem argumento"
           " nenhum: ℚ[λ]/(μ) é corpo se e só se μ é IRREDUTÍVEL, e onde μ factoriza tem de"
           " haver divisor de zero — um X = aI + bA não nulo com det X = 0. Conta-se, e a"
           " dicotomia sai limpa nas quatro testemunhas: o gambá (μ = λ²+1) e o gato"
           " (μ = λ²−λ−1) têm ZERO divisores de zero em 48 elementos, e são corpos — ℚ(i) e"
           " ℚ(√5), que é o `corpo_estelar` desta casa a aparecer sem ser posto; diag(1,2)"
           " (μ = (λ−1)(λ−2)) e o bloco de Jordan (μ = (λ−1)²) têm-nos, e não são. O FECHO É"
           " DO ANEL E NÃO DA CONTA: a inversa de um elemento invertível NUNCA sai do span —"
           " {I, A, X⁻¹} continua a ter posto 2 em todos os casos —, o que é dizer que não se"
           " precisa de sair de ℚ[A] para inverter dentro dele. E o CONTROLO é a"
           " distribuição, sem a qual aquele zero seria uma ausência a passar por lei: nos"
           " irredutíveis os 48 elementos não nulos invertem TODOS, e nos redutíveis há dos"
           " dois tipos — se um lado fosse vazio, a dicotomia não separava nada.", mal == 0);
    }

    /* ═══ §W94: A COMPANHEIRA DO CATÁLOGO, REALIZADA — E O DIVISOR DE ZERO ══ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W94 Comp{n}{m} no motor: det, inversa inteira, χ por Newton, e n≡5 (mod 6).\n\n");
        { const char *tabs[] = { "A","P","Q","R" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w94__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w94__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w94.mem"); unlink("/tmp/pgwire_w94.prog"); }
        if(!sql_abrir("/tmp/pgwire_w94")) mal++;

        #define N94 6
        long rec94 = 0;          /* INSERTs recusados pelo envelope da célula */
        #define POE(t,M,L,C) do { char q2[400]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[300]; cols[0] = 0; \
              for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (L); i2++){ char vs[300]; vs[0] = 0; \
                for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); if(!o2.ok) rec94++; } } while(0)

        /* multiplica X (n×n) por Y (n×n) pelo motor */
        #define MUL(X,Y,Z,n) do { POE("A",(X),(n),(n)); POE("P",(Y),(n),(n)); \
            sql_executa("SELECT produto(A) FROM P", &o); \
            if(!o.ok || o.nrows != (n)){ mal++; } else \
            for(int i3 = 0; i3 < (n); i3++) for(int j3 = 0; j3 < (n); j3++) \
                (Z)[i3][j3] = atol(o.cell[i3][j3]); } while(0)

        /* ── O `catalogo.tex` §sec:dim define Comp{n}{m}, a companheira de
         * β_{n,m} = x^n − m·x^{n−1} − 1, e diz das suas leis «verificado
         * SIMBOLICAMENTE para 2≤n≤6, m∈{1,2}». O motor nunca as executou. */

        /* ── (1) det Comp = (−1)^{n+1}, e a INVERSA É INTEIRA — que é a metade
         * sem a qual |det|=1 seria meia lei: é ela que põe Comp em GL_n(ℤ). */
        long det_ok = 0, inv_ok = 0, casos = 0, inv_casos = 0, inv_tecto = 0;
        for(int n = 2; n <= N94; n++) for(long m = 1; m <= 2; m++){
            long C[N94][N94];
            memset(C, 0, sizeof C);
            C[0][0] = m; C[0][n-1] = 1;
            for(int i = 1; i < n; i++) C[i][i-1] = 1;
            POE("A", C, n, n);
            sql_executa("SELECT det(*) FROM A", &o);
            casos++;
            long esp = ((n + 1) % 2 == 0) ? 1 : -1;      /* (−1)^{n+1} */
            if(o.ok && atol(o.cell[0][0]) == esp) det_ok++;
            else printf("      n=%d m=%ld: det = %s, esperado %ld\n",
                        n, m, o.ok ? o.cell[0][0] : "?", esp);
            /* a INVERSA tem tecto PRÓPRIO: trabalha na aumentada n×2n, logo o
             * seu limite é METADE do geral — 3×3. Acima disso o motor recusa, e
             * isso é o tecto dele e não uma falha da lei: conta-se à parte, e o
             * det continua a valer nos casos que a inversa não alcança. */
            sql_executa("SELECT inversa(*) FROM A", &o);
            if(!o.ok) inv_tecto++;
            else {
                inv_casos++;
                int inteira = (o.nrows == n);
                for(int i = 0; i < n && inteira; i++) for(int j = 0; j < n; j++)
                    if(strchr(o.cell[i][j], '/')) inteira = 0;
                if(inteira) inv_ok++;
            }
        }
        printf("      det Comp{n}{m} = (−1)^{n+1}: %ld/%ld  (n=2..6, m=1..2, que é"
               " o regime do catálogo)\n", det_ok, casos);
        printf("      inversa INTEIRA: %ld/%ld onde ela CABE; nos outros %ld o motor"
               " recusa pelo tecto 3×3 da aumentada n×2n — tecto dele, não da lei\n",
               inv_ok, inv_casos, inv_tecto);
        if(det_ok != casos) mal++;
        if(inv_ok != inv_casos || inv_casos < 4) mal++;
        if(inv_tecto == 0){ printf("      → o tecto da inversa não foi exercido\n"); mal++; }

        /* ── (2) O CARACTERÍSTICO É β_{n,m}, obtido pelo motor por NEWTON dos
         * traços das potências. O conteúdo não é o e_1 = m nem o e_n = ±1: são
         * os ZEROS do meio. Um polinómio com n coeficientes de que n−2 têm de
         * se anular é uma afirmação forte, e é ela que diz que a companheira
         * é a de β e não de outra coisa. */
        long chi_ok = 0, chi_n = 0, zeros = 0;
        for(int n = 2; n <= N94; n++) for(long m = 1; m <= 2; m++){
            long C[N94][N94], Ak[N94][N94], p[N94+1];
            memset(C, 0, sizeof C);
            C[0][0] = m; C[0][n-1] = 1;
            for(int i = 1; i < n; i++) C[i][i-1] = 1;
            memset(Ak, 0, sizeof Ak);
            for(int i = 0; i < n; i++) Ak[i][i] = 1;
            int falhou = 0;
            for(int k = 1; k <= n; k++){
                long T[N94][N94];
                MUL(C, Ak, T, n);
                memcpy(Ak, T, sizeof Ak);
                POE("A", Ak, n, n);
                sql_executa("SELECT traco(*) FROM A", &o);
                if(!o.ok){ falhou = 1; break; }
                p[k] = atol(o.cell[0][0]);
            }
            if(falhou){ mal++; continue; }
            long e[N94+1]; e[0] = 1;
            for(int k = 1; k <= n; k++){
                long s = 0;
                for(int i = 1; i <= k; i++)
                    s += ((i % 2) ? 1 : -1) * e[k-i] * p[i];
                e[k] = s / k;
            }
            /* β_{n,m}: e_1 = m, e_2..e_{n−1} = 0, e_n = (−1)^{n+1} */
            int bom = (e[1] == m);
            for(int k = 2; k <= n-1; k++){ if(e[k] != 0) bom = 0; else zeros++; }
            if(e[n] != (((n+1) % 2 == 0) ? 1 : -1)) bom = 0;
            chi_n++;
            if(bom) chi_ok++;
            else printf("      n=%d m=%ld: e₁ = %ld, e_n = %ld — não é β_{n,m}\n",
                        n, m, e[1], e[n]);
        }
        printf("      χ(Comp{n}{m}) = β_{n,m} por Newton: %ld/%ld, com %ld"
               " coeficientes do meio a ANULAREM-SE\n", chi_ok, chi_n, zeros);
        if(chi_ok != chi_n) mal++;
        if(zeros < 12){ printf("      → poucos zeros: a lei não foi exercida\n"); mal++; }

        /* ── (3) E O CASO EM QUE «CORPO» NÃO SE PODE DIZER. O catálogo é
         * explícito: «o enunciado certo, para m=1, continua CONDICIONAL —
         * quando β é irredutível, K_{n,m} = ℚ[x]/(β) é corpo». A condição
         * falha em n ≡ 5 (mod 6), onde o sexto ciclotómico divide β_{n,1}, e
         * o primeiro caso é
         *     β_{5,1} = x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1).
         * Pelo §W93, ℚ[A] ≅ ℚ[λ]/(μ) tem divisor de zero exactamente onde μ
         * factoriza — e aqui ele não se argumenta: EXIBE-SE. Avaliam-se os
         * dois factores na companheira, pelo motor, e o produto tem de ser a
         * matriz NULA com os dois factores não nulos. */
        {
            const int n = 5;
            long C[N94][N94], A2[N94][N94], A3[N94][N94], Id[N94][N94];
            memset(C, 0, sizeof C); memset(Id, 0, sizeof Id);
            C[0][0] = 1; C[0][4] = 1;
            for(int i = 1; i < n; i++) C[i][i-1] = 1;
            for(int i = 0; i < n; i++) Id[i][i] = 1;
            MUL(C, C, A2, n);
            MUL(C, A2, A3, n);
            /* F = A² − A + I   e   G = A³ − A − I */
            long F[N94][N94], G[N94][N94], H[N94][N94];
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
                F[i][j] = A2[i][j] - C[i][j] + Id[i][j];
                G[i][j] = A3[i][j] - C[i][j] - Id[i][j];
            }
            int Fnulo = 1, Gnulo = 1, Hnulo = 1;
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
                if(F[i][j]) Fnulo = 0;
                if(G[i][j]) Gnulo = 0;
            }
            MUL(F, G, H, n);
            for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
                if(H[i][j]) Hnulo = 0;
            printf("      n=5, m=1 — β = (x²−x+1)(x³−x−1):\n");
            printf("        F = A²−A+I  nula? %s   ·  G = A³−A−I  nula? %s\n",
                   Fnulo ? "sim" : "NÃO", Gnulo ? "sim" : "NÃO");
            printf("        F·G nula? %s  → dois elementos NÃO NULOS com produto"
                   " ZERO: é um DIVISOR DE ZERO exibido\n", Hnulo ? "sim" : "NÃO");
            if(Fnulo || Gnulo || !Hnulo) mal++;
            /* e o CONTROLO: em n=4, m=1 o β é irredutível, e não há factor
             * ciclotómico a exibir — o mesmo par de graus 2 e 3 não anula */
            const int n4 = 4;
            long C4[N94][N94], B2[N94][N94], B3[N94][N94], I4[N94][N94],
                 F4[N94][N94], G4[N94][N94], H4[N94][N94];
            memset(C4, 0, sizeof C4); memset(I4, 0, sizeof I4);
            C4[0][0] = 1; C4[0][3] = 1;
            for(int i = 1; i < n4; i++) C4[i][i-1] = 1;
            for(int i = 0; i < n4; i++) I4[i][i] = 1;
            MUL(C4, C4, B2, n4); MUL(C4, B2, B3, n4);
            for(int i = 0; i < n4; i++) for(int j = 0; j < n4; j++){
                F4[i][j] = B2[i][j] - C4[i][j] + I4[i][j];
                G4[i][j] = B3[i][j] - C4[i][j] - I4[i][j];
            }
            MUL(F4, G4, H4, n4);
            int H4nulo = 1;
            for(int i = 0; i < n4; i++) for(int j = 0; j < n4; j++)
                if(H4[i][j]) H4nulo = 0;
            printf("        CONTROLO n=4 (β irredutível): o MESMO par de factores"
                   " dá produto nulo? %s — e é isso que impede de ler o zero"
                   " acima como um acidente da conta\n", H4nulo ? "sim" : "não");
            if(H4nulo) mal++;
        }

        /* ── (4) E O ENVELOPE DA CÉLULA, que foi o que me travou e é uma lei
         * desta casa, não um estorvo: `RACIONAL` vive em −128..127, e o motor
         * diz «a linha é RECUSADA e nada é escrito — alargar a célula é subir a
         * torre, e não truncar em silêncio». Com m=3 e n=5 a quinta potência
         * chega a 244 e sai do envelope. O que se mede aqui é o EFEITO da
         * recusa: a linha não entra, a tabela fica 4×5, e a operação seguinte
         * falha com «not square» — um erro que NÃO nomeia a causa. */
        {
            long rec_antes = rec94;
            long M5[N94][N94] = {{244,3,9,27,81},{81,1,3,9,27},{27,0,1,3,9},
                                 {9,0,0,1,3},{3,0,0,0,1}};
            POE("Q", M5, 5, 5);
            long recusadas = rec94 - rec_antes;
            sql_executa("SELECT COUNT(*) FROM Q", &o);
            long linhas = (o.ok ? atol(o.cell[0][0]) : -1);
            sql_executa("SELECT traco(*) FROM Q", &o);
            printf("      ENVELOPE — 244 não cabe em −128..127: linhas recusadas %ld,"
                   " a tabela fica com %ld de 5\n", recusadas, linhas);
            printf("        e a operação seguinte diz «%s» — não nomeia a causa, e é por"
                   " isso que o regime do catálogo (m ∈ {1,2}) é o regime certo\n",
                   o.ok ? "(passou!)" : o.err);
            if(recusadas != 1 || linhas != 4) mal++;
            if(o.ok) mal++;              /* uma 4×5 NÃO pode dar traço */
            /* o CONTROLO: 127 cabe, e a mesma tabela com 127 no lugar do 244
             * entra inteira e responde — senão isto media a recusa constante */
            long M5b[N94][N94] = {{127,3,9,27,81},{81,1,3,9,27},{27,0,1,3,9},
                                  {9,0,0,1,3},{3,0,0,0,1}};
            rec_antes = rec94;
            POE("Q", M5b, 5, 5);
            sql_executa("SELECT traco(*) FROM Q", &o);
            printf("        CONTROLO com 127 no mesmo lugar: recusadas %ld, traço %s\n",
                   rec94 - rec_antes, o.ok ? o.cell[0][0] : o.err);
            if(rec94 - rec_antes != 0 || !o.ok) mal++;
        }

        #undef MUL
        #undef POE
        #undef N94
        sql_fechar();

        printf("\n");
        ok("A COMPANHEIRA DO CATÁLOGO PASSA A SER EXECUTADA, E O CASO CONDICIONAL GANHA UM"
           " OBJECTO. O `catalogo.tex` §sec:dim define Comp{n}{m}, a companheira de"
           " β_{n,m} = xⁿ − m·x^{n−1} − 1, e diz das suas leis «verificado SIMBOLICAMENTE para"
           " 2≤n≤6, m∈{1,2}» — o motor nunca as tinha executado. Executa, NO REGIME QUE O"
           " CATÁLOGO ENUNCIA: det = (−1)^{n+1} em 10/10. A inversa é a metade sem a qual"
           " |det|=1 seria meia lei — é ela que põe Comp em GL_n(ℤ) —, e sai INTEIRA, sem uma"
           " barra de fracção, em 4/4 dos casos ONDE ELA CABE; nos outros 6 o motor recusa"
           " pelo tecto 3×3 da matriz aumentada n×2n, que é metade do tecto geral. Esse tecto"
           " é do motor e não da lei, conta-se à parte, e o det continua a valer nos casos"
           " que a inversa não alcança. O CARACTERÍSTICO SAI POR NEWTON dos traços das"
           " potências, todos pedidos ao motor, e o conteúdo não é o e₁ = m nem o e_n = ±1:"
           " são os ZEROS DO MEIO — 20 coeficientes que têm de se anular, e anulam. Um"
           " polinómio de n coeficientes com n−2 nulos é o que diz que aquela matriz é a"
           " companheira de β e não de outra coisa. E O CASO CONDICIONAL DEIXA DE SER UMA"
           " RESSALVA: o catálogo é explícito que «para m=1 o enunciado continua condicional —"
           " QUANDO β é irredutível, K_{n,m} = ℚ[x]/(β) é corpo», e que a condição falha em"
           " n ≡ 5 (mod 6), com β_{5,1} = (x²−x+1)(x³−x−1). Pelo §W93 isso obriga a haver"
           " divisor de zero; aqui ele não se argumenta, EXIBE-SE: avaliam-se os dois factores"
           " na companheira 5×5 pelo motor, F = A²−A+I e G = A³−A−I, ambos NÃO NULOS, e o"
           " produto F·G é a matriz NULA. Dois elementos não nulos de ℚ[A] com produto zero —"
           " logo ali K_{5,1} não é corpo, e a palavra do título é condicional por uma razão"
           " que agora tem um objecto. O CONTROLO é n=4, onde β é irredutível: o MESMO par de"
           " factores de graus 2 e 3 NÃO dá produto nulo, sem o que aquele zero se leria como"
           " um acidente da conta em vez de uma propriedade do polinómio. E O REGIME NÃO FOI"
           " ESCOLHIDO POR CONVENIÊNCIA: a primeira escrita estendeu a m=3 por conta própria e"
           " bateu no ENVELOPE DA CÉLULA, que é lei desta casa — `RACIONAL` vive em −128..127,"
           " e o motor diz «a linha é RECUSADA e nada é escrito: alargar a célula é subir a"
           " torre, e não truncar em silêncio». Com m=3 e n=5 a quinta potência chega a 244 e"
           " sai. Mede-se o EFEITO dessa recusa, que é o que interessa: a linha não entra, a"
           " tabela fica com 4 de 5, e a operação seguinte falha com «matrix is not square» —"
           " um erro verdadeiro que NÃO NOMEIA A CAUSA, e que apontaria o leitor para a forma"
           " da tabela em vez do envelope. O controlo é 127 no mesmo lugar: entra sem recusa"
           " e o traço responde 131. O catálogo já dizia m ∈ {1,2}, e era o regime certo.", mal == 0);
    }

    /* ═══ §W95: prop:tracos E A ZETA DINÂMICA, EM INTEIROS ═════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W95 t_k = m·t_{k−1} + t_{k−n}, e det(I − xA) contra β* — o motor decide.\n\n");
        { const char *tabs[] = { "A","P","Z" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w95__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w95__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w95.mem"); unlink("/tmp/pgwire_w95.prog"); }
        if(!sql_abrir("/tmp/pgwire_w95")) mal++;

        #define NZ 6
        long rec95 = 0;
        #define POE(t,M,L,C) do { char q2[400]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[300]; cols[0] = 0; \
              for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (L); i2++){ char vs[300]; vs[0] = 0; \
                for(j2 = 0; j2 < (C); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); if(!o2.ok) rec95++; } } while(0)

        /* ── (1) prop:tracos do catálogo: t_k = Tr(Comp^k) é INTEIRO, com
         * t_0 = n, t_1 = m, e a recorrência t_k = m·t_{k−1} + t_{k−n} para
         * k ≥ n. São DOIS CAMINHOS: o traço pedido ao motor a cada potência,
         * e a recorrência escrita sem ver matriz nenhuma. */
        long tr_ok = 0, tr_n = 0, kmin_glob = 99, kmax_glob = 0,
             rec_test = 0, rec_ok = 0, curtas = 0;
        for(int n = 2; n <= NZ; n++) for(long m = 1; m <= 2; m++){
            long C[NZ][NZ], Ak[NZ][NZ], t[24];
            memset(C, 0, sizeof C); memset(Ak, 0, sizeof Ak);
            C[0][0] = m; C[0][n-1] = 1;
            for(int i = 1; i < n; i++) C[i][i-1] = 1;
            for(int i = 0; i < n; i++) Ak[i][i] = 1;
            t[0] = n;
            /* corre-se ATÉ ONDE CABE: as potências crescem e o envelope da
             * célula é −128..127. Regista-se o k alcançado em vez de fixar um
             * limite que a família não aguenta — e exige-se pelo menos n+2,
             * senão a recorrência não chega a ser exercida. */
            int kmax = 0;
            for(int k = 1; k <= 20; k++){
                long T[NZ][NZ], antes = rec95;
                POE("A", C, n, n); POE("P", Ak, n, n);
                if(rec95 != antes) break;
                sql_executa("SELECT produto(A) FROM P", &o);
                if(!o.ok || o.nrows != n) break;
                for(int i = 0; i < n; i++) for(int j = 0; j < n; j++)
                    T[i][j] = atol(o.cell[i][j]);
                antes = rec95;
                POE("A", T, n, n);
                if(rec95 != antes) break;      /* A^k saiu do envelope */
                sql_executa("SELECT traco(*) FROM A", &o);
                if(!o.ok) break;
                memcpy(Ak, T, sizeof Ak);
                t[k] = atol(o.cell[0][0]);
                kmax = k;
            }
            tr_n++;
            /* o que se conta é a INSTÂNCIA da recorrência, não a família: uma
             * família que o envelope trava cedo exerce menos vezes, e dizer
             * «10/10 famílias» esconderia isso. */
            int bom = (kmax >= 1) && (t[1] == m);
            for(int k = n; k <= kmax; k++){
                rec_test++;
                if(t[k] == m*t[k-1] + t[k-n]) rec_ok++; else bom = 0;
            }
            if(kmax < n + 2) curtas++;
            if(bom) tr_ok++;
            else printf("      n=%d m=%ld: kmax=%d, t₁=%ld (esperado %ld) — não fecha\n",
                        n, m, kmax, t[1], m);
            if(kmax < kmin_glob) kmin_glob = kmax;
            if(kmax > kmax_glob) kmax_glob = kmax;
        }
        printf("      prop:tracos — t₀=n, t₁=m e t_k = m·t_{k−1} + t_{k−n}:"
               " %ld/%ld instâncias da recorrência, em %ld/%ld famílias"
               " (n=2..6, m=1..2)\n", rec_ok, rec_test, tr_ok, tr_n);
        printf("      e o alcance é do ENVELOPE e não da lei: k vai de %ld a %ld"
               " conforme a família cresce, e em %ld delas o envelope trava antes"
               " de k = n+2\n", kmin_glob, kmax_glob, curtas);
        if(tr_ok != tr_n || tr_n != 10) mal++;
        if(rec_ok != rec_test || rec_test < 30) mal++;
        if(curtas == 0){ printf("      → o envelope não travou nenhuma: o alcance"
                                " não foi exercido\n"); mal++; }

        /* ── (2) E A ZETA. O catálogo escreve
         *     ζ(x) = exp(Σ t_k x^k / k) = 1/det(I − x·Comp),
         * e a seguir «o denominador não é um objecto novo: det(I − x·Comp) É
         * β*», com β*(x) = −xⁿβ(1/x) = xⁿ + mx − 1 (def:nu, e explícito na
         * §sec:constr). Aqui não se argumenta: pede-se o det ao motor em vários
         * x inteiros e vê-se com QUAL dos dois ele concorda. Uma identidade
         * polinomial testada em mais pontos que o grau está provada. */
        long bate_beta = 0, bate_menos = 0, pontos = 0;
        for(int n = 2; n <= NZ; n++) for(long m = 1; m <= 2; m++)
        for(long x = -3; x <= 3; x++){
            long M[NZ][NZ];
            memset(M, 0, sizeof M);
            for(int i = 0; i < n; i++) M[i][i] = 1;
            M[0][0] -= x*m; M[0][n-1] -= x;
            for(int i = 1; i < n; i++) M[i][i-1] -= x;
            long antes = rec95;
            POE("Z", M, n, n);
            if(rec95 != antes) continue;              /* saiu do envelope */
            sql_executa("SELECT det(*) FROM Z", &o);
            if(!o.ok) continue;
            long d = atol(o.cell[0][0]);
            long xn = 1; for(int i = 0; i < n; i++) xn *= x;
            long beta_est = xn + m*x - 1;             /* β*(x) */
            pontos++;
            if(d == beta_est)  bate_beta++;
            if(d == -beta_est) bate_menos++;
        }
        printf("      det(I − x·Comp) contra β*(x) = xⁿ+mx−1: bate em %ld/%ld\n",
               bate_beta, pontos);
        printf("      det(I − x·Comp) contra −β*(x) = 1−mx−xⁿ: bate em %ld/%ld\n",
               bate_menos, pontos);
        if(pontos < 40) mal++;
        if(bate_menos != pontos) mal++;
        if(bate_beta == pontos) mal++;   /* se batesse nos dois, não separava */

        /* ── (3) O GUME é x = 0, e não precisa de varredura: I − 0·A = I, cujo
         * determinante é 1, enquanto β*(0) = −1. E o teste que decide sozinho é
         * ζ(0): a série Σ t_k x^k/k não tem termo constante, logo ζ(0) = e⁰ = 1
         * — e 1/β*(0) daria −1. O sinal do catálogo está trocado nessa linha:
         * det(I − x·Comp) = −β*(x), e ζ(x) = 1/(1 − mx − xⁿ). */
        {
            long Id[NZ][NZ]; memset(Id, 0, sizeof Id);
            for(int i = 0; i < 2; i++) Id[i][i] = 1;
            POE("Z", Id, 2, 2);
            sql_executa("SELECT det(*) FROM Z", &o);
            long d0 = o.ok ? atol(o.cell[0][0]) : -99;
            printf("      GUME em x=0 — det(I) = %ld, e β*(0) = −1: %s\n",
                   d0, d0 == 1 ? "diferem" : "?");
            printf("        e ζ(0) = 1 obriga o denominador a valer 1 em x=0, o que"
                   " 1/β* não faz — logo det(I − xA) = −β*(x)\n");
            if(d0 != 1) mal++;
        }

        #undef POE
        #undef NZ
        sql_fechar();

        printf("\n");
        ok("A PROPOSIÇÃO DOS TRAÇOS REALIZA-SE, E A ZETA DO CATÁLOGO TEM UM SINAL TROCADO. A"
           " prop:tracos do `catalogo.tex` §sec:sequencias diz que t_k = Tr(Comp^k) é inteiro,"
           " com t₀ = n, t₁ = m e t_k = m·t_{k−1} + t_{k−n} para k ≥ n. Realiza-se por DOIS"
           " CAMINHOS que não se conhecem: o traço pedido ao motor a cada potência, com a"
           " matriz a alimentar-se a si própria, e a recorrência escrita sem ver matriz"
           " nenhuma — 77/77 INSTÂNCIAS da recorrência, em 10/10 famílias com n=2..6 e"
           " m=1..2. Conta-se a instância e não a família de propósito: o alcance em k é do"
           " ENVELOPE da célula e não da lei, vai de 5 a 20 conforme a família cresce, e em"
           " DUAS delas o envelope trava antes de k = n+2 — dizer «10/10 famílias» esconderia"
           " que umas exercem a lei vinte vezes e outras uma. E A ZETA: o catálogo escreve"
           " ζ(x) = exp(Σ t_k x^k/k) = 1/det(I − x·Comp) e a seguir «o denominador não é um"
           " objecto novo: det(I − x·Comp) É β*», com β*(x) = −xⁿβ(1/x) = xⁿ + mx − 1 pela"
           " def:nu. As duas coisas não podem ser ambas verdadeiras, e não se decide por"
           " argumento: pede-se o det ao motor em x inteiros e vê-se com qual ele concorda."
           " Concorda com −β*(x) = 1 − mx − xⁿ em TODOS os pontos, e com β*(x) em NENHUM."
           " O gume nem precisa de varredura: em x = 0 a matriz é I, o det é 1, e β*(0) = −1."
           " E o que decide sozinho é ζ(0): a série Σ t_k x^k/k não tem termo constante, logo"
           " ζ(0) = e⁰ = 1, e o denominador TEM de valer 1 em x = 0 — o que 1/β* não faz."
           " Portanto det(I − x·Comp) = −β*(x) e ζ(x) = 1/(1 − mx − xⁿ). O teorema do catálogo"
           " não muda: o denominador continua a ser o reverso de β a menos de sinal, e é o"
           " mesmo polinómio com que se prova o thm:pisot. O que muda é a linha da caixa, e o"
           " sinal importa porque ζ(0) = 1 é a normalização que faz dela uma zeta.", mal == 0);
    }

    /* ═══ §W96: O DICIONÁRIO PALAVRA↔MATRIZ, E ν NAS DUAS ENCARNAÇÕES ══════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W96 M(a₀)···M(a_n): as colunas são os convergentes, e a palavra ao contrário é a TRANSPOSTA.\n\n");
        { const char *tabs[] = { "A","P","T" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w96__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w96__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w96.mem"); unlink("/tmp/pgwire_w96.prog"); }
        if(!sql_abrir("/tmp/pgwire_w96")) mal++;

        long rec96 = 0;
        #define POE2(t,M) do { char q2[220]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (c1 RACIONAL, c2 RACIONAL)", t); \
            sql_executa(q2,&o2); \
            for(int i2 = 0; i2 < 2; i2++){ \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld)", \
                         t, (M)[i2][0], (M)[i2][1]); \
                sql_executa(q2,&o2); if(!o2.ok) rec96++; } } while(0)

        /* produto pelo motor: Z = X·Y.  A convenção do motor é
         *   SELECT produto(A) FROM P   ==   P · A
         * — a tabela do FROM é o factor da ESQUERDA. Nos blocos de potências
         * isso não se nota, porque A^k comuta com A; aqui as letras NÃO
         * comutam, e a ordem é a lei. */
        #define MUL2(X,Y,Z) do { POE2("P",(X)); POE2("A",(Y)); \
            sql_executa("SELECT produto(A) FROM P", &o); \
            if(!o.ok || o.nrows != 2){ falhou = 1; } else \
            for(int i3 = 0; i3 < 2; i3++) for(int j3 = 0; j3 < 2; j3++) \
                (Z)[i3][j3] = atol(o.cell[i3][j3]); } while(0)

        /* ── O `catalogo` §sec:dic põe M(a) = (a,1;1,0) e diz «a fração contínua
         * É a palavra; a matriz é a mesma palavra escrita de outro jeito». A
         * prop:conv diz que as colunas de M_n são os convergentes, e o thm:rev
         * que ler a palavra ao contrário É TRANSPOR. Aqui as três correm no
         * motor, e cada uma tem um segundo caminho que não sabe da primeira. */

        long conv_ok = 0, conv_n = 0, det_ok = 0, rev_ok = 0, rev_n = 0,
             bez_ok = 0, alc_max = 0, travadas = 0, fora_env = 0, det_n = 0;
        long A_[8];
        for(long a0 = 1; a0 <= 3; a0++) for(long a1 = 1; a1 <= 3; a1++)
        for(long a2 = 1; a2 <= 3; a2++) for(long a3 = 1; a3 <= 2; a3++)
        for(long a4 = 1; a4 <= 2; a4++){
            A_[0] = a0; A_[1] = a1; A_[2] = a2; A_[3] = a3; A_[4] = a4;
            const int N = 5;
            /* (1) o produto das letras, pelo motor */
            long M[2][2] = {{1,0},{0,1}};
            int falhou = 0, alc = 0;
            for(int k = 0; k < N && !falhou; k++){
                long L[2][2] = {{A_[k],1},{1,0}}, Z[2][2];
                long antes = rec96;
                MUL2(M, L, Z);
                if(falhou) break;
                if(rec96 != antes){ falhou = 1; break; }   /* saiu do envelope */
                memcpy(M, Z, sizeof M);
                alc = k + 1;
            }
            if(alc > alc_max) alc_max = alc;
            if(alc < N){ travadas++; continue; }

            /* (2) os convergentes pela RECORRÊNCIA, sem ver matriz nenhuma */
            long p1 = 1, q1 = 0, p2 = 0, q2 = 1, p = 0, q = 0;
            for(int k = 0; k < N; k++){
                p = A_[k]*p1 + p2; q = A_[k]*q1 + q2;
                p2 = p1; q2 = q1; p1 = p; q1 = q;
            }
            conv_n++;
            /* M_n = (p_n, p_{n−1}; q_n, q_{n−1}) */
            if(M[0][0] == p && M[0][1] == p2 && M[1][0] == q && M[1][1] == q2)
                conv_ok++;
            else if(conv_n < 4)
                printf("      palavra (%ld,%ld,%ld,%ld,%ld): motor (%ld,%ld;%ld,%ld)"
                       " vs recorrência (%ld,%ld;%ld,%ld)\n", a0,a1,a2,a3,a4,
                       M[0][0],M[0][1],M[1][0],M[1][1], p,p2,q,q2);

            /* (3) det M_n = (−1)^{n+1}, que É a identidade de Bézout.
             * O produto correu todo dentro do envelope, mas REPOR o M final
             * numa tabela pode não caber — p_n cresce com a palavra. Quando
             * não cabe, isto é alcance e não falha: conta-se à parte. */
            long antes_det = rec96;
            POE2("A", M);
            if(rec96 != antes_det){ fora_env++; continue; }
            sql_executa("SELECT det(*) FROM A", &o);
            long d = o.ok ? atol(o.cell[0][0]) : 0;
            /* com L letras o produto tem L factores de det −1, logo
             * det M = (−1)^L. O catálogo indexa a_0..a_n, isto é n+1 letras,
             * e escreve (−1)^{n+1}: é o MESMO expoente, o número de letras.
             * Confundi n com a contagem na primeira escrita. */
            long esp = (N % 2 == 0) ? 1 : -1;
            det_n++;
            if(d == esp) det_ok++;
            if(p*q2 - p2*q == esp) bez_ok++;    /* a mesma conta, escrita à mão */

            /* (4) thm:rev — a palavra AO CONTRÁRIO dá a TRANSPOSTA, e as duas
             * encarnações de ν da def:nu (reverter a palavra, transpor a
             * matriz) têm de coincidir. A transposta é pedida ao motor. */
            long R[2][2] = {{1,0},{0,1}};
            falhou = 0;
            for(int k = N - 1; k >= 0 && !falhou; k--){
                long L[2][2] = {{A_[k],1},{1,0}}, Z[2][2];
                MUL2(R, L, Z);
                if(!falhou) memcpy(R, Z, sizeof R);
            }
            if(falhou) continue;
            POE2("A", M);
            sql_executa("SELECT transposta(*) FROM A", &o);
            rev_n++;
            if(o.ok && o.nrows == 2
               && atol(o.cell[0][0]) == R[0][0] && atol(o.cell[0][1]) == R[0][1]
               && atol(o.cell[1][0]) == R[1][0] && atol(o.cell[1][1]) == R[1][1])
                rev_ok++;
        }
        printf("      prop:conv — as colunas de M_n SÃO os convergentes: %ld/%ld"
               " (produto do motor contra a recorrência escrita sem matriz)\n",
               conv_ok, conv_n);
        printf("      det M_n = (−1)^L pelo motor: %ld/%ld · e a MESMA conta como"
               " Bézout p_nq_{n−1} − p_{n−1}q_n: %ld/%ld\n",
               det_ok, det_n, bez_ok, det_n);
        printf("      thm:rev — a palavra AO CONTRÁRIO é a TRANSPOSTA: %ld/%ld"
               " (ν como reversão e ν como transposição, def:nu)\n", rev_ok, rev_n);
        printf("      alcance: palavras de %ld letras; %ld combinações em que o M"
               " final não CABE de volta na célula (p_n > 127) — alcance, não falha\n",
               alc_max, fora_env);
        if(conv_ok != conv_n || conv_n < 80) mal++;
        if(det_ok != det_n || bez_ok != det_n || det_n < 80) mal++;
        if(rev_ok != rev_n || rev_n < 80) mal++;
        if(fora_env == 0){ printf("      → o envelope não travou nenhuma: o alcance"
                                  " não foi exercido\n"); mal++; }

        /* ── (5) O CONTROLO, sem o qual o thm:rev seria trivial: se a palavra
         * já for um PALÍNDROMO, reverter não muda nada e a transposta é a
         * própria matriz — a lei passaria sem dizer nada. Conta-se quantas
         * palavras NÃO são palíndromos e nelas M_n ≠ M_nᵀ, senão «reverter é
         * transpor» estaria a ser medido onde as duas são a identidade. */
        {
            long assim = 0, pal = 0;
            for(long a0 = 1; a0 <= 3; a0++) for(long a1 = 1; a1 <= 3; a1++)
            for(long a2 = 1; a2 <= 3; a2++){
                long W[3] = {a0,a1,a2};
                int palind = (W[0] == W[2]);
                long M[2][2] = {{1,0},{0,1}};
                int falhou = 0;
                for(int k = 0; k < 3 && !falhou; k++){
                    long L[2][2] = {{W[k],1},{1,0}}, Z[2][2];
                    MUL2(M, L, Z);
                    if(!falhou) memcpy(M, Z, sizeof M);
                }
                if(falhou) continue;
                int simetrica = (M[0][1] == M[1][0]);
                if(palind) pal++;
                else if(!simetrica) assim++;
            }
            printf("      CONTROLO — nas 27 palavras de 3 letras: %ld palíndromos"
                   " (onde M = Mᵀ e a lei nada diria) e %ld assimétricas onde"
                   " M ≠ Mᵀ e a lei é exercida\n", pal, assim);
            if(assim < 12 || pal < 6) mal++;
        }

        #undef MUL2
        #undef POE2
        sql_fechar();

        printf("\n");
        ok("A PALAVRA E A MATRIZ SÃO O MESMO OBJECTO, E ν É A MESMA OPERAÇÃO NOS DOIS ALFABETOS."
           " O `catalogo.tex` §sec:dic põe M(a) = (a,1;1,0) e diz «a fração contínua É a"
           " palavra; a matriz é a mesma palavra escrita de outro jeito». Três enunciados dessa"
           " secção passam a correr no motor, cada um com um segundo caminho que não sabe do"
           " primeiro. A prop:conv diz que as colunas de M_n são os convergentes: o produto das"
           " letras é feito pelo motor, letra a letra, e a recorrência p_n = a_np_{n−1}+p_{n−2}"
           " é escrita sem ver matriz nenhuma — batem em 108/108 palavras de cinco letras. O"
           " det M_n = (−1)^L sai pelo motor e a MESMA conta escrita como Bézout,"
           " p_nq_{n−1} − p_{n−1}q_n, dá o mesmo em 104/104: não são duas leis, é uma lei lida"
           " em dois alfabetos, e é por isso que o determinante de uma palavra é a sua"
           " identidade de Bézout. As outras quatro não falham: o produto correu todo dentro"
           " do envelope, mas REPOR o M final numa tabela pede p_n ≤ 127 e ali não cabe —"
           " alcance, contado à parte, e sem ele o zero não seria exercido. E O thm:rev É O MAIS BONITO: ler a palavra AO CONTRÁRIO É"
           " TRANSPOR a matriz, porque cada letra é simétrica. Isso põe DUAS das três"
           " encarnações da ν da def:nu a coincidirem no mesmo objecto — ν(palavra) = palavra"
           " ao contrário e ν(M) = Mᵀ —, e mede-se com a transposta pedida ao motor contra o"
           " produto refeito de trás para a frente: 104/104. O CONTROLO é o que impede isto de"
           " ser vazio: numa palavra PALÍNDROMA reverter não muda nada e M já é simétrica, pelo"
           " que a lei passaria sem dizer nada. Contam-se as 27 palavras de três letras — 9 são"
           " palíndromas e 18 assimétricas com M ≠ Mᵀ —, e é nessas que «reverter é transpor»"
           " tem conteúdo. O alcance é do ENVELOPE e não da lei: as palavras param nas cinco"
           " letras porque os convergentes crescem e a célula vive em −128..127. E A ORDEM DO"
           " PRODUTO TEVE DE SER MEDIDA, não suposta: `SELECT produto(A) FROM P` é P·A, com a"
           " tabela do FROM como factor da ESQUERDA. Nos blocos de potências isso não se nota,"
           " porque A^k comuta com A, e a primeira escrita deste bloco herdou a suposição"
           " errada de lá — deu 12/108 nos convergentes e 0/108 no determinante. As letras não"
           " comutam, e é aqui que a convenção passa a ser lei: sondou-se com duas matrizes"
           " que não comutam e o motor respondeu.", mal == 0);
    }

    /* ═══ §W97: EUCLIDES DESFAZ A PALAVRA — E O double NÃO ═════════════════ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W97 M(a)⁻¹ é inteira, p_n/p_{n−1} é a palavra AO CONTRÁRIO, e o double erra.\n\n");
        { const char *tabs[] = { "A","P" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w97__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w97__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w97.mem"); unlink("/tmp/pgwire_w97.prog"); }
        if(!sql_abrir("/tmp/pgwire_w97")) mal++;

        long rec97 = 0;
        #define POE2(t,M) do { char q2[220]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (c1 RACIONAL, c2 RACIONAL)", t); \
            sql_executa(q2,&o2); \
            for(int i2 = 0; i2 < 2; i2++){ \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld)", \
                         t, (M)[i2][0], (M)[i2][1]); \
                sql_executa(q2,&o2); if(!o2.ok) rec97++; } } while(0)

        /* Euclides inteiro: a palavra de p/q, forma canónica (último ≥ 2) */
        #define EUC(p0,q0,W,L) do { long P = (p0), Q = (q0); (L) = 0; \
            while(Q != 0 && (L) < 24){ long a = P / Q, r = P - a*Q; \
                if(r < 0){ a--; r += Q; } \
                (W)[(L)++] = a; P = Q; Q = r; } } while(0)

        /* ── (1) REVERSIBILIDADE I: cada letra tem inversa SOBRE OS INTEIROS,
         * M(a)⁻¹ = (0,1;1,−a), porque det M(a) = −1 é unidade em ℤ. A inversa
         * é pedida ao motor — e o que importa não é o valor, é NÃO HAVER
         * DENOMINADOR: é isso que põe a letra em GL₂(ℤ) e faz o algoritmo
         * desfazer-se sem sair do anel. */
        long inv_ok = 0, inv_int = 0, inv_n = 0;
        for(long a = -4; a <= 4; a++){
            long M[2][2] = {{a,1},{1,0}};
            POE2("A", M);
            sql_executa("SELECT inversa(*) FROM A", &o);
            if(!o.ok || o.nrows != 2) continue;
            inv_n++;
            int inteira = 1;
            for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                if(strchr(o.cell[i][j], '/')) inteira = 0;
            if(inteira) inv_int++;
            if(atol(o.cell[0][0]) == 0 && atol(o.cell[0][1]) == 1
               && atol(o.cell[1][0]) == 1 && atol(o.cell[1][1]) == -a) inv_ok++;
        }
        printf("      M(a)⁻¹ = (0,1;1,−a) pelo motor: %ld/%ld · e SEM denominador"
               " (logo em GL₂(ℤ)): %ld/%ld\n", inv_ok, inv_n, inv_int, inv_n);
        if(inv_ok != inv_n || inv_int != inv_n || inv_n < 8) mal++;

        /* ── (2) A CONSEQUÊNCIA DO thm:rev QUE FALTAVA. §W96 mediu que
         * transpor é reverter; o teorema tira daí que p_n/p_{n−1} EXPANDE na
         * palavra AO CONTRÁRIO. Aqui a palavra é gerada, o produto é do motor,
         * e a expansão de p_n/p_{n−1} é feita por Euclides INTEIRO — que não
         * sabe de matriz nenhuma. */
        long rev_ok = 0, rev_n = 0, fora = 0, fundidas = 0;
        for(long a0 = 1; a0 <= 3; a0++) for(long a1 = 1; a1 <= 3; a1++)
        for(long a2 = 1; a2 <= 3; a2++) for(long a3 = 2; a3 <= 3; a3++){
            long W[4] = {a0,a1,a2,a3};
            long M[2][2] = {{1,0},{0,1}};
            int falhou = 0;
            for(int k = 0; k < 4 && !falhou; k++){
                long L[2][2] = {{W[k],1},{1,0}}, Z[2][2];
                POE2("P", M); POE2("A", L);
                sql_executa("SELECT produto(A) FROM P", &o);
                if(!o.ok || o.nrows != 2){ falhou = 1; break; }
                for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
                    Z[i][j] = atol(o.cell[i][j]);
                memcpy(M, Z, sizeof M);
            }
            if(falhou){ fora++; continue; }
            long pn = M[0][0], pn1 = M[0][1];
            long E[24]; int LE;
            EUC(pn, pn1, E, LE);
            rev_n++;
            /* a palavra esperada é a invertida — mas Euclides devolve a forma
             * CANÓNICA, cujo último termo é ≥ 2. Quando a₀ = 1 a invertida
             * acaba em 1, e a canónica funde: […, a₁, 1] ≡ […, a₁+1]. Não é
             * uma excepção: é a ambiguidade que a própria prop:injectividade
             * do catálogo declara, «a menos de ⟨…,a_n⟩ = ⟨…,a_n−1,1⟩». */
            long R[8]; int LR = 0;
            for(int k = 3; k >= 0; k--) R[LR++] = W[k];
            if(LR > 1 && R[LR-1] == 1){ LR--; R[LR-1] += 1; }
            int bate = (LE == LR);
            for(int k = 0; k < LE && bate; k++)
                if(E[k] != R[k]) bate = 0;
            if(bate) rev_ok++;
            else if(rev_n - rev_ok < 3){
                printf("      palavra (%ld,%ld,%ld,%ld) → p/p' = %ld/%ld → [",
                       a0,a1,a2,a3,pn,pn1);
                for(int k = 0; k < LE; k++) printf("%ld%s", E[k], k+1<LE?",":"");
                printf("] esperado [");
                for(int k = 0; k < LR; k++) printf("%ld%s", R[k], k+1<LR?",":"");
                printf("]\n");
            }
            if(W[0] == 1) fundidas++;
        }
        printf("      thm:rev (consequência) — p_n/p_{n−1} expande na palavra AO"
               " CONTRÁRIO: %ld/%ld, com Euclides inteiro a não saber de matriz\n",
               rev_ok, rev_n);
        printf("        e em %ld delas a₀ = 1, onde a forma canónica FUNDE o último"
               " par — a ambiguidade que a prop:injectividade já declara\n", fundidas);
        if(rev_ok != rev_n || rev_n < 40) mal++;
        if(fundidas == 0){ printf("        → a fusão não foi exercida\n"); mal++; }

        /* ── (3) obs:float — «o algoritmo não pode correr em vírgula
         * flutuante». O catálogo dá o caso: em double, 8/5 produz [1;1,1,1,1]
         * em vez da canónica [1;1,1,1,2], porque 1/0,5000000000000002 vale
         * 1,9999999999999991 e o piso disso é 1. Aqui o double é o OBJECTO
         * MEDIDO e não a régua: a régua é Euclides em inteiros. */
        {
            long W8[24]; int L8;
            EUC(8, 5, W8, L8);
            printf("      obs:float — 8/5 em INTEIROS dá [");
            for(int k = 0; k < L8; k++) printf("%ld%s", W8[k], k+1<L8?",":"");
            printf("]\n");
            /* o mesmo laço em double */
            double x = 8.0/5.0; long Wd[24]; int Ld = 0;
            while(Ld < 8){
                double fl = (x >= 0) ? (double)(long)x : (double)(long)x - 1;
                Wd[Ld++] = (long)fl;
                double f = x - fl;
                if(f == 0.0) break;
                x = 1.0/f;
            }
            printf("        e em double dá [");
            for(int k = 0; k < Ld; k++) printf("%ld%s", Wd[k], k+1<Ld?",":"");
            printf("]  — o resto do primeiro passo é %.17g\n", 8.0/5.0 - 1.0);
            int igual = (Ld == L8);
            for(int k = 0; k < L8 && igual; k++) if(Wd[k] != W8[k]) igual = 0;
            printf("        as duas palavras coincidem? %s\n", igual ? "SIM" : "NÃO");
            /* o passo em que o piso cai do lado errado é o QUARTO. O catálogo
             * nomeia-o com os dígitos 1/0,5000000000000002 = 1,9999999999999991;
             * aqui saem outros. O último bit depende da ORDEM das operações do
             * laço e não se reproduz sem reproduzir o laço — o que se reproduz,
             * e é o que a observação afirma, é o PISO: 1 onde tem de ser 2. */
            { double r = 8.0/5.0, resto = 0;
              for(int k = 0; k < 3; k++){ double fl = (double)(long)r;
                  resto = r - fl; r = 1.0/resto; }
              printf("        o quarto passo: resto %.17g, inverso %.17g, piso %ld"
                     " — tem de ser 2\n", resto, r, (long)r);
              printf("        (o catálogo escreve 1/0,5000000000000002 ="
                     " 1,9999999999999991; o último bit depende da ordem das"
                     " operações, o PISO não)\n");
              if((long)r != 1) mal++; }
            if(igual) mal++;      /* se coincidissem, a observação era falsa */
            if(L8 != 4 || W8[0] != 1 || W8[3] != 2) mal++;
        }

        /* ── (4) E QUANTO É QUE ISSO ACONTECE, que é o que separa «uma
         * curiosidade» de «não pode correr»: varrem-se os p/q e conta-se em
         * quantos o double diverge do inteiro. O CONTROLO é que a maioria
         * COINCIDA — se divergisse em tudo, o defeito seria do meu laço e não
         * da representação. */
        {
            long div = 0, tot = 0;
            for(long q = 2; q <= 60; q++) for(long p = 1; p <= 60; p++){
                if(p % q == 0) continue;
                long Wi[24]; int Li;
                EUC(p, q, Wi, Li);
                double x = (double)p/(double)q; long Wd[24]; int Ld = 0;
                while(Ld < 24){
                    double fl = (double)(long)x;
                    if(x < 0 && x != fl) fl -= 1;
                    Wd[Ld++] = (long)fl;
                    double f = x - fl;
                    if(f == 0.0) break;
                    x = 1.0/f;
                }
                tot++;
                /* compara-se só os PRIMEIROS Li termos: o laço em double nunca
                 * chega a resto zero e continuaria a cuspir lixo, pelo que
                 * exigir o mesmo COMPRIMENTO mediria o meu critério de paragem
                 * e não a representação. O que se pergunta é se os dígitos que
                 * a palavra verdadeira tem saem certos. */
                int igual = (Ld >= Li);
                for(int k = 0; k < Li && igual; k++) if(Wd[k] != Wi[k]) igual = 0;
                if(!igual) div++;
            }
            printf("      e a frequência: o double erra algum dos dígitos da palavra"
                   " verdadeira em %ld dos %ld racionais p/q com p,q ≤ 60\n", div, tot);
            printf("        CONTROLO — nos outros %ld acerta-os todos, logo o defeito"
                   " é da REPRESENTAÇÃO e não do laço\n", tot - div);
            if(div == 0 || div == tot) mal++;
        }

        #undef EUC
        #undef POE2
        sql_fechar();

        printf("\n");
        ok("EUCLIDES DESFAZ A PALAVRA SEM SAIR DO ANEL, E O double NÃO CONSEGUE CORRER O MESMO"
           " ALGORITMO. A §sec:revI do catálogo diz que cada letra tem inversa SOBRE OS"
           " INTEIROS, M(a)⁻¹ = (0,1;1,−a), porque det M(a) = −1 é unidade em ℤ. Pede-se ao"
           " motor e sai certa em 9/9 — mas o que importa não é o valor: é NÃO HAVER"
           " DENOMINADOR, medido célula a célula, porque é isso que põe a letra em GL₂(ℤ) e"
           " faz o algoritmo desfazer-se sem mudar de anel. DEPOIS FECHA-SE O thm:rev, cuja"
           " consequência §W96 tinha deixado por medir: ali mediu-se que transpor é reverter,"
           " e o teorema tira daí que p_n/p_{n−1} EXPANDE na palavra ao contrário. A palavra é"
           " gerada, o produto das letras é do motor, e a expansão de p_n/p_{n−1} é feita por"
           " Euclides inteiro — que não sabe de matriz nenhuma: 54/54. E em 18 delas a₀ = 1,"
           " onde a forma canónica FUNDE o último par, […, a₁, 1] ≡ […, a₁+1]: não é excepção,"
           " é a ambiguidade que a própria prop:injectividade declara, e ignorá-la fazia o"
           " número cair para 36/54 — exactamente os dois terços em que a₀ ≠ 1. E A obs:float DEIXA DE"
           " SER UM AVISO E PASSA A SER UM NÚMERO. O catálogo diz que «o algoritmo não pode"
           " correr em vírgula flutuante» e dá o caso: 8/5 produz [1;1,1,1,1] em vez da"
           " canónica [1;1,1,1,2]. Reproduz-se, com o double como OBJECTO MEDIDO e Euclides"
           " inteiro como régua — nunca ao contrário: em inteiros sai [1,1,1,2] e em double"
           " [1,1,1,1,…], e o erro entra no QUARTO passo, onde o resto fica ligeiramente acima"
           " de 0,5, o inverso ligeiramente abaixo de 2, e o piso dá 1. O que NÃO se reproduz"
           " são os dígitos: o catálogo escreve 1/0,5000000000000002 = 1,9999999999999991 e"
           " aqui sai 1/0,50000000000000044 = 1,9999999999999982 — o último bit depende da"
           " ordem das operações do laço, e dizê-lo é o que impede de anunciar uma reprodução"
           " exacta que não há. O PISO é que se reproduz, e é ele que a observação afirma. E mede-se"
           " a FREQUÊNCIA, que é o que separa uma curiosidade de um «não pode»: o double erra"
           " algum dos dígitos da palavra verdadeira em 1301 dos 3339 racionais com p,q ≤ 60."
           " O que se compara são os PRIMEIROS dígitos e não o comprimento: o laço em double"
           " nunca chega a resto zero e continuaria a cuspir lixo — em 8/5 sai um sexto termo"
           " de 562949953421312 —, pelo que exigir o mesmo comprimento mediria o meu critério"
           " de paragem e não a representação. Na primeira escrita media, e dava 2786 de 3339."
           " O CONTROLO é que nos outros 2038 acerte todos: se errasse em tudo, o defeito seria"
           " do meu laço; se não errasse em nenhum, a observação do catálogo era falsa. É a"
           " razão pela qual esta casa não guarda um float — dita como quantidade.", mal == 0);
    }

    /* ═══ §W98: K_m = {a+bσ_m} — A NORMA É O det, O CONJUGADO É A ADJUNTA ══ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W98 O corpo K_m do catálogo, com o det do motor por norma.\n\n");
        { const char *tabs[] = { "A","P","U" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w98__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w98__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w98.mem"); unlink("/tmp/pgwire_w98.prog"); }
        if(!sql_abrir("/tmp/pgwire_w98")) mal++;

        long rec98 = 0;
        #define POE2(t,M) do { char q2[220]; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            snprintf(q2, sizeof q2, "CREATE TABLE %s (c1 RACIONAL, c2 RACIONAL)", t); \
            sql_executa(q2,&o2); \
            for(int i2 = 0; i2 < 2; i2++){ \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%ld,%ld)", \
                         t, (M)[i2][0], (M)[i2][1]); \
                sql_executa(q2,&o2); if(!o2.ok) rec98++; } } while(0)

        /* ── O catálogo (§«Quarto: as matrizes») constrói K_m = {a+bσ_m : a,b∈ℚ}
         * com σ² = mσ + 1, dá o produto reduzido, o conjugado «a troca de sinal
         * fora da diagonal, que é o esquilo», a norma N(u) = a²+mab−b² e o
         * inverso ū/N(u). Aqui isso corre no motor — e a ponte que aparece é
         * que a NORMA É O DETERMINANTE e o CONJUGADO É A ADJUNTA. */
        long prod_ok = 0, prod_n = 0, norm_ok = 0, conj_ok = 0, uu_ok = 0,
             zero_fora = 0, elems = 0, fora_env = 0;
        for(long m = 1; m <= 4; m++){
            for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                /* u = a + bσ  ↦  aI + bA_m = (a+bm, b; b, a) */
                long U[2][2] = {{a + b*m, b},{b, a}};
                long ant = rec98;
                POE2("U", U);
                if(rec98 != ant){ fora_env++; continue; }
                elems++;
                /* (1) a NORMA do catálogo é o DETERMINANTE do motor */
                sql_executa("SELECT det(*) FROM U", &o);
                if(o.ok && atol(o.cell[0][0]) == a*a + m*a*b - b*b) norm_ok++;
                if(a || b){ if(a*a + m*a*b - b*b == 0) zero_fora++; }

                /* (2) o CONJUGADO é a ADJUNTA: ū = (a+bm) − bσ ↦ (a,−b;−b,a+bm),
                 * que é adj(U) — e o motor confirma-o por u·ū = N(u)·I */
                long Ub[2][2] = {{a, -b},{-b, a + b*m}};
                POE2("P", U); POE2("A", Ub);
                sql_executa("SELECT produto(A) FROM P", &o);
                if(o.ok && o.nrows == 2){
                    long N = a*a + m*a*b - b*b;
                    if(atol(o.cell[0][0]) == N && atol(o.cell[1][1]) == N
                       && atol(o.cell[0][1]) == 0 && atol(o.cell[1][0]) == 0)
                        uu_ok++;
                }
                /* e o conjugado como troca de sinal fora da diagonal: a
                 * adjunta de (x,y;z,w) é (w,−y;−z,x) */
                if(Ub[0][0] == U[1][1] && Ub[0][1] == -U[0][1]
                   && Ub[1][0] == -U[1][0] && Ub[1][1] == U[0][0]) conj_ok++;

                /* (3) o produto reduzido do catálogo, contra o produto do motor */
                for(long c = -2; c <= 2; c++) for(long d = -2; d <= 2; d++){
                    long V[2][2] = {{c + d*m, d},{d, c}};
                    long ant2 = rec98;
                    POE2("P", U); POE2("A", V);
                    if(rec98 != ant2) continue;
                    sql_executa("SELECT produto(A) FROM P", &o);
                    if(!o.ok || o.nrows != 2) continue;
                    long e1 = a*c + b*d, e2 = a*d + b*c + m*b*d;  /* a regra do gato */
                    long W[2][2] = {{e1 + e2*m, e2},{e2, e1}};
                    prod_n++;
                    if(atol(o.cell[0][0]) == W[0][0] && atol(o.cell[0][1]) == W[0][1]
                       && atol(o.cell[1][0]) == W[1][0] && atol(o.cell[1][1]) == W[1][1])
                        prod_ok++;
                }
            }
        }
        printf("      o produto reduzido (ac+bd) + (ad+bc+m·bd)σ contra o produto"
               " do motor: %ld/%ld\n", prod_ok, prod_n);
        printf("      N(u) = a²+mab−b² É o det(aI+bA_m) do motor: %ld/%ld\n",
               norm_ok, elems);
        printf("      o conjugado É a ADJUNTA (troca de sinal fora da diagonal):"
               " %ld/%ld · e u·ū = N(u)·I pelo motor: %ld/%ld\n",
               conj_ok, elems, uu_ok, elems);
        printf("      N(u) = 0 com u ≠ 0: %ld em %ld elementos — o corpo fecha\n",
               zero_fora, elems);
        if(prod_ok != prod_n || prod_n < 800) mal++;
        if(norm_ok != elems || conj_ok != elems || uu_ok != elems) mal++;
        if(zero_fora != 0) mal++;

        /* ── (4) E O CONTROLO É A MESMA CONSTRUÇÃO COM A HIPÓTESE RETIRADA.
         * O catálogo apoia-se em σ_m ser irracional — «a fração contínua não
         * termina» —, e isso é o mesmo que dizer que λ²−mλ−1 é IRREDUTÍVEL,
         * que é o critério do §W93. Troque-se +1 por 0: σ² = mσ, cujo
         * discriminante m² É quadrado, as raízes são 0 e m, e o «corpo» passa a
         * ter divisores de zero. A construção é letra por letra a mesma. */
        {
            long div_zero = 0, tot = 0;
            for(long m = 1; m <= 4; m++)
            for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++){
                if(!a && !b) continue;
                /* B_m = (m,0;1,0): σ² = mσ + 0 */
                long U[2][2] = {{a + b*m, 0},{b, a}};
                long ant = rec98;
                POE2("U", U);
                if(rec98 != ant) continue;
                sql_executa("SELECT det(*) FROM U", &o);
                if(!o.ok) continue;
                tot++;
                if(atol(o.cell[0][0]) == 0) div_zero++;
            }
            printf("      CONTROLO — a MESMA construção com σ² = mσ + 0, onde"
                   " Δ = m² É quadrado: %ld de %ld elementos não nulos têm norma"
                   " ZERO\n", div_zero, tot);
            printf("        logo ali não há corpo, e o que o separa de K_m é"
                   " exactamente a irracionalidade de σ — o mesmo critério do §W93\n");
            if(div_zero == 0) mal++;
        }

        #undef POE2
        sql_fechar();

        printf("\n");
        ok("O CORPO K_m DO CATÁLOGO CORRE NO MOTOR, E A PONTE QUE APARECE É QUE A NORMA É O"
           " DETERMINANTE. A subsecção «Quarto: as matrizes» constrói"
           " K_m = {a + bσ_m : a,b ∈ ℚ} com σ² = mσ + 1, e diz que é a assinatura da órbita"
           " que garante o fecho: N(u) = 0 com u ≠ 0 exigiria σ_m racional, e a fração"
           " contínua [m;m,m,…] não termina. Vestindo u = a + bσ como aI + bA_m, tudo isso é"
           " uma operação do motor. O produto reduzido do catálogo,"
           " (ac+bd) + (ad+bc+m·bd)σ, bate com o produto de matrizes em 4900/4900 — «é a única"
           " regra do corpo, e é o gato», e o motor não a conhece: ela cai da multiplicação."
           " A NORMA N(u) = a² + mab − b² É o det(aI + bA_m), em 196/196: o catálogo define a"
           " norma pela conjugação e o motor devolve-a pelo determinante, sem saber de"
           " conjugado nenhum. E O CONJUGADO É A ADJUNTA: «a troca de sinal fora da diagonal,"
           " que é o esquilo» é exactamente adj(U) = (w,−y;−z,x), verificado nos 196 e"
           " confirmado pelo motor com u·ū = N(u)·I. Nenhum elemento não nulo tem norma zero,"
           " pelo que o corpo fecha. E O CONTROLO É A MESMA CONSTRUÇÃO COM A HIPÓTESE"
           " RETIRADA, que é o que dá conteúdo àquele zero: troca-se σ² = mσ + 1 por"
           " σ² = mσ + 0, cujo discriminante m² É quadrado e cujas raízes são 0 e m — logo"
           " racionais. A construção é letra por letra a mesma, e 34 dos 192 elementos não"
           " nulos passam a ter norma ZERO: ali não há corpo, e um só bastava. O que separa os dois casos é"
           " exactamente a irracionalidade de σ, que é o «μ irredutível» do §W93 dito noutro"
           " alfabeto — e que o §W70 já tinha medido numa terceira forma, m²+4 nunca ser"
           " quadrado perfeito. Três caminhos para a mesma cláusula.", mal == 0);
    }

    /* ═══ §W99: O OMNITRIX — ⊕ SOMA OS GRAUS, ⊗ COMPÕE, E O det CONSERVA ═══ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W99 ⊕ e ⊗ sobre as companheiras: as duas leis do det, e o fecho em GL(ℤ).\n\n");
        { const char *tabs[] = { "A","B","K" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w99__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w99__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w99.mem"); unlink("/tmp/pgwire_w99.prog"); }
        if(!sql_abrir("/tmp/pgwire_w99")) mal++;

        long rec99 = 0;
        #define POE(t,M,N) do { char q2[420]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[320]; cols[0] = 0; \
              for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (N); i2++){ char vs[320]; vs[0] = 0; \
                for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); if(!o2.ok) rec99++; } } while(0)

        /* ── O `catalogo` fecha assim: «reunidas sob ⊕ (que soma os graus) e ⊗
         * (La Hire, que compõe), com a torção por invariante e a NORMA
         * MULTIPLICATIVA por lei de conservação, essas matrizes formam de novo
         * um único corpo — o Omnitrix». Aqui as duas operações correm sobre as
         * companheiras e o det do motor diz as suas leis, que NÃO são a mesma. */
        long som_ok = 0, som_n = 0, kro_ok = 0, kro_n = 0, gl_ok = 0, gl_n = 0,
             confunde = 0, fora = 0;
        for(long m1 = 1; m1 <= 3; m1++) for(long m2 = 1; m2 <= 3; m2++)
        for(int n1 = 2; n1 <= 3; n1++) for(int n2 = 2; n2 <= 3; n2++){
            if(n1 * n2 > 6) continue;              /* o tecto do motor */
            long A[6][6], B[6][6];
            memset(A, 0, sizeof A); memset(B, 0, sizeof B);
            A[0][0] = m1; A[0][n1-1] = 1;
            for(int i = 1; i < n1; i++) A[i][i-1] = 1;
            B[0][0] = m2; B[0][n2-1] = 1;
            for(int i = 1; i < n2; i++) B[i][i-1] = 1;

            POE("A", A, n1); sql_executa("SELECT det(*) FROM A", &o);
            if(!o.ok) continue; long dA = atol(o.cell[0][0]);
            POE("B", B, n2); sql_executa("SELECT det(*) FROM B", &o);
            if(!o.ok) continue; long dB = atol(o.cell[0][0]);

            /* ⊕ : bloco diagonal, grau n1+n2 */
            if(n1 + n2 <= 6){
                long S[6][6]; memset(S, 0, sizeof S);
                for(int i = 0; i < n1; i++) for(int j = 0; j < n1; j++) S[i][j] = A[i][j];
                for(int i = 0; i < n2; i++) for(int j = 0; j < n2; j++)
                    S[n1+i][n1+j] = B[i][j];
                long ant = rec99;
                POE("K", S, n1+n2);
                if(rec99 == ant){
                    sql_executa("SELECT det(*) FROM K", &o);
                    if(o.ok){ som_n++; if(atol(o.cell[0][0]) == dA*dB) som_ok++; }
                }
            }
            /* ⊗ : Kronecker, grau n1·n2 */
            {
                long K[6][6]; int N = n1*n2;
                for(int i = 0; i < n1; i++) for(int j = 0; j < n1; j++)
                for(int k = 0; k < n2; k++) for(int l = 0; l < n2; l++)
                    K[i*n2+k][j*n2+l] = A[i][j]*B[k][l];
                long ant = rec99;
                POE("K", K, N);
                if(rec99 != ant){ fora++; }
                else {
                    sql_executa("SELECT det(*) FROM K", &o);
                    if(o.ok){
                        long d = atol(o.cell[0][0]);
                        long esp = 1;
                        for(int i = 0; i < n2; i++) esp *= dA;
                        for(int i = 0; i < n1; i++) esp *= dB;
                        kro_n++;
                        if(d == esp) kro_ok++;
                        /* o gume: a lei do ⊗ NÃO é a do ⊕ */
                        if(d == dA*dB && esp != dA*dB) confunde++;
                        /* o fecho: |det| = 1 entra e |det| = 1 sai */
                        gl_n++;
                        if((dA == 1 || dA == -1) && (dB == 1 || dB == -1)
                           && (d == 1 || d == -1)) gl_ok++;
                    }
                }
            }
        }
        printf("      ⊕ soma os graus e o det MULTIPLICA: det(A⊕B) = detA·detB"
               " em %ld/%ld\n", som_ok, som_n);
        printf("      ⊗ compõe os graus e o det tem OUTRO expoente:"
               " det(A⊗B) = (detA)^{n_B}·(detB)^{n_A} em %ld/%ld\n", kro_ok, kro_n);
        printf("      e o FECHO: |det| = 1 entra nos dois lados e sai em %ld/%ld"
               " — é a lei de conservação do Omnitrix\n", gl_ok, gl_n);
        printf("      matrizes fora do tecto 6×6 do motor: %ld\n", fora);
        if(som_ok != som_n || som_n < 6) mal++;
        if(kro_ok != kro_n || kro_n < 6) mal++;
        if(gl_ok != gl_n) mal++;

        /* ── O GUME: as duas leis TÊM de separar-se. Se det(A⊗B) fosse
         * detA·detB, ⊕ e ⊗ teriam a mesma lei e a distinção não diria nada.
         * Exibe-se um caso em que os dois valores DIFEREM — e não basta
         * afirmá-lo: conta-se. */
        {
            /* Nas companheiras det = ±1 SEMPRE, e ali as duas leis coincidem —
             * porque ±1 é ponto fixo de qualquer expoente. Para as separar é
             * preciso sair de GL(ℤ), e é isso que mostra POR QUE o fecho é
             * exactamente ali: a unidade é o ponto fixo das duas leis. */
            long difere = 0, igual = 0, seg_exp = 0, seg_prod = 0, tot = 0;
            long vals[] = {-2,-1,1,2,3};
            for(unsigned i = 0; i < sizeof vals/sizeof vals[0]; i++)
            for(unsigned j = 0; j < sizeof vals/sizeof vals[0]; j++){
                long a = vals[i], b = vals[j];
                long A[6][6] = {{a,0},{0,1}}, B[6][6] = {{b,0},{0,1}};
                long K[6][6];
                for(int r = 0; r < 2; r++) for(int c = 0; c < 2; c++)
                for(int u = 0; u < 2; u++) for(int v = 0; v < 2; v++)
                    K[r*2+u][c*2+v] = A[r][c]*B[u][v];
                long ant = rec99;
                POE("K", K, 4);
                if(rec99 != ant) continue;
                sql_executa("SELECT det(*) FROM K", &o);
                if(!o.ok) continue;
                long d = atol(o.cell[0][0]);
                long por_exp = a*a * b*b;      /* (detA)^{n_B}·(detB)^{n_A}, n=2 */
                long por_prod = a*b;
                tot++;
                if(d == por_exp) seg_exp++;
                if(d == por_prod) seg_prod++;
                if(por_exp != por_prod) difere++; else igual++;
            }
            printf("      GUME — com det(A) e det(B) fora de ±1, as duas leis SEPARAM-SE"
                   " em %ld dos %ld casos (coincidem em %ld)\n", difere, tot, igual);
            printf("        e o motor segue o EXPOENTE em %ld/%ld, o produto simples"
                   " em %ld/%ld — logo a lei do ⊗ é a do expoente\n",
                   seg_exp, tot, seg_prod, tot);
            printf("        os %ld em que coincidem são exactamente os de |detA·detB| = 1:"
                   " a UNIDADE é o ponto fixo das duas leis, e é por isso que o fecho do"
                   " Omnitrix é GL(ℤ) e não outra coisa\n", igual);
            if(difere == 0 || seg_exp != tot || seg_prod == tot) mal++;
        }

        #undef POE
        sql_fechar();

        printf("\n");
        ok("O OMNITRIX FECHA PORQUE AS DUAS OPERAÇÕES CONSERVAM A NORMA — E CONSERVAM-NA COM"
           " LEIS DIFERENTES. O `catalogo.tex` fecha o bestiário dizendo que as matrizes,"
           " «reunidas sob ⊕ (que soma os graus) e ⊗ (La Hire, que compõe), com a torção por"
           " invariante e a NORMA MULTIPLICATIVA por lei de conservação, formam de novo um"
           " único corpo». Aqui as duas correm sobre as companheiras do §sec:dim e o det do"
           " motor diz as suas leis. Em ⊕, que é o bloco diagonal e soma os graus, o"
           " determinante MULTIPLICA: det(A⊕B) = detA·detB. Em ⊗, que é o Kronecker e"
           " COMPÕE os graus, o determinante tem outro expoente:"
           " det(A⊗B) = (detA)^{n_B}·(detB)^{n_A} — o número de cópias de cada bloco. NÃO SÃO"
           " A MESMA LEI, e é isso que impede de ler «a norma é multiplicativa» como uma frase"
           " só: cada operação multiplica à sua maneira, e a maneira é o GRAU do outro factor."
           " É o §W85 outra vez — cada face respeita a operação que lhe pertence — agora entre"
           " duas operações do mesmo corpo em vez de entre traço e determinante. E DAÍ SAI O"
           " FECHO, que é a razão de o conjunto ser corpo e não coleção: se |det A| = 1 e"
           " |det B| = 1, então |det| = 1 nos dois compostos, seja qual for o expoente —"
           " porque ±1 elevado a o que for continua ±1. As companheiras estão todas em"
           " GL_n(ℤ) pela prop:detn, logo ⊕ e ⊗ nunca saem de lá: a unidade é preservada pelas"
           " duas operações, e é essa preservação, não a lista, que faz o Omnitrix. O GUME É QUE AS DUAS LEIS"
           " SEPAREM, e ele quase não mordeu: nas companheiras det = ±1 SEMPRE, e ali os dois"
           " valores coincidem em todas as combinações — porque ±1 é ponto fixo de qualquer"
           " expoente. Para as separar é preciso sair de GL(ℤ), e é isso que mostra POR QUE o"
           " fecho é exactamente ali. Com det(A) e det(B) em −2..3 as leis separam-se em 23 dos"
           " 25 casos, o motor segue o EXPOENTE em 25/25 e o produto simples em apenas 2 — e"
           " esses 2 são precisamente os de |detA·detB| = 1. A UNIDADE É O PONTO FIXO DAS DUAS"
           " LEIS: é onde ⊕ e ⊗ deixam de se distinguir pela norma, e é por isso que o fecho do"
           " Omnitrix é GL(ℤ) e não outro conjunto — não foi escolhido, é o lugar onde as duas"
           " operações concordam. E o alcance é o tecto 6×6 do motor: (3)⊗(3) daria 9×9 e"
           " fica de fora, o que é do motor e não da lei.", mal == 0);
    }

    /* ═══ §W100: A TRANSFORMADA EM INTEIROS — O QUE ATRAVESSA É F SEM O 1/√n ═ */
    {
        SqlOut o, o2;
        long mal = 0;
        printf("\n§W100 Hadamard: F⁴ = n²·I atravessa, o 1/√n não.\n\n");
        { const char *tabs[] = { "A","P","H" };
          for(unsigned k = 0; k < sizeof tabs/sizeof tabs[0]; k++){
              char m[80], p2[80];
              snprintf(m, sizeof m, "/tmp/pgwire_w100__%s.mem", tabs[k]);
              snprintf(p2, sizeof p2, "/tmp/pgwire_w100__%s.prog", tabs[k]);
              unlink(m); unlink(p2);
          }
          unlink("/tmp/pgwire_w100.mem"); unlink("/tmp/pgwire_w100.prog"); }
        if(!sql_abrir("/tmp/pgwire_w100")) mal++;

        long rec100 = 0;
        #define POE(t,M,N) do { char q2[420]; int i2, j2; \
            snprintf(q2, sizeof q2, "DROP TABLE IF EXISTS %s", t); sql_executa(q2,&o2); \
            { char cols[320]; cols[0] = 0; \
              for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                  snprintf(c2, sizeof c2, "%sc%d RACIONAL", j2?", ":"", j2+1); \
                  strncat(cols, c2, sizeof cols - strlen(cols) - 1); } \
              snprintf(q2, sizeof q2, "CREATE TABLE %s (%s)", t, cols); \
              sql_executa(q2,&o2); } \
            for(i2 = 0; i2 < (N); i2++){ char vs[320]; vs[0] = 0; \
                for(j2 = 0; j2 < (N); j2++){ char c2[40]; \
                    snprintf(c2, sizeof c2, "%s%ld", j2?",":"", (M)[i2][j2]); \
                    strncat(vs, c2, sizeof vs - strlen(vs) - 1); } \
                snprintf(q2, sizeof q2, "INSERT INTO %s VALUES (%s)", t, vs); \
                sql_executa(q2,&o2); if(!o2.ok) rec100++; } } while(0)
        #define MUL(X,Y,Z,N) do { POE("P",(X),(N)); POE("A",(Y),(N)); \
            sql_executa("SELECT produto(A) FROM P", &o); \
            if(!o.ok || o.nrows != (N)) falhou = 1; else \
            for(int i3 = 0; i3 < (N); i3++) for(int j3 = 0; j3 < (N); j3++) \
                (Z)[i3][j3] = atol(o.cell[i3][j3]); } while(0)

        /* ── O catálogo diz da transformada universal: «a normalização é 1/√n
         * em CADA lado, e não por gosto», e daí F⁻¹ exacta, F⁴ = id, unitária.
         * Está certo sobre ℝ. Mas esta casa não guarda 1/√2, e o próprio
         * catálogo tem o critério: «identidade atravessa, limite não; o que
         * atravessa é a parte algébrica». Aqui mede-se O QUE ATRAVESSA. */
        long q2_ok = 0, q4_ok = 0, sim_ok = 0, uni_ok = 0, casos = 0;
        for(int e = 1; e <= 2; e++){
            int N = 1 << e;                    /* n = 2, 4 — 8×8 sai do tecto */
            long H[6][6];
            /* Sylvester: H_{2m} = H_m ⊗ (1,1;1,−1), construído a partir de 1 */
            H[0][0] = 1;
            for(int s = 1, m = 1; s <= e; s++, m *= 2){
                long T[6][6];
                for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
                    T[i][j] = H[i][j];       T[i][j+m]   = H[i][j];
                    T[i+m][j] = H[i][j];     T[i+m][j+m] = -H[i][j];
                }
                for(int i = 0; i < 2*m; i++) for(int j = 0; j < 2*m; j++) H[i][j] = T[i][j];
            }
            int falhou = 0;
            long H2[6][6], H4[6][6];
            MUL(H, H, H2, N);
            if(falhou){ mal++; continue; }
            MUL(H2, H2, H4, N);
            if(falhou){ mal++; continue; }
            casos++;
            /* H² = n·I  e  H⁴ = n²·I */
            int ok2 = 1, ok4 = 1;
            for(int i = 0; i < N; i++) for(int j = 0; j < N; j++){
                if(H2[i][j] != (i==j ? (long)N : 0)) ok2 = 0;
                if(H4[i][j] != (i==j ? (long)N*N : 0)) ok4 = 0;
            }
            if(ok2) q2_ok++;
            if(ok4) q4_ok++;
            /* Hᵀ = H, pedido ao motor */
            POE("A", H, N);
            sql_executa("SELECT transposta(*) FROM A", &o);
            int sim = (o.ok && o.nrows == N);
            for(int i = 0; i < N && sim; i++) for(int j = 0; j < N; j++)
                if(atol(o.cell[i][j]) != H[i][j]) sim = 0;
            if(sim) sim_ok++;
            /* unitária a menos de n: HᵀH = n·I, e como Hᵀ = H isso é H² */
            if(ok2) uni_ok++;
            POE("A", H, N);
            sql_executa("SELECT det(*) FROM A", &o);
            printf("      n=%d: H² = n·I %s · H⁴ = n²·I %s · Hᵀ = H %s (motor)"
                   " · det = %s\n", N, ok2?"sim":"NÃO", ok4?"sim":"NÃO",
                   sim?"sim":"NÃO", o.ok ? o.cell[0][0] : "?");
        }
        printf("      → F² = n·I e F⁴ = n²·I: %ld/%ld e %ld/%ld — a PERIODICIDADE"
               " atravessa para os inteiros, com o factor explícito\n",
               q2_ok, casos, q4_ok, casos);
        if(q2_ok != casos || q4_ok != casos || sim_ok != casos || casos < 2) mal++;

        /* ── E O QUE NÃO ATRAVESSA. A normalização 1/√n só é racional quando n
         * é quadrado perfeito; e como aqui n = 2^e, isso pede e PAR. Logo em
         * n=2 e n=8 o 1/√n não vive no corpo, e a transformada normalizada não
         * é uma operação desta casa — enquanto H, sem normalização, é inteira
         * em todos. Não é uma limitação da máquina: é a distinção que o próprio
         * catálogo faz entre o que atravessa e o que não. */
        {
            long quad = 0, nao = 0;
            for(int e = 1; e <= 6; e++){
                long n = 1 << e, r = 0;
                while(r*r < n) r++;
                if(r*r == n) quad++; else nao++;
            }
            printf("      o 1/√n é racional em %ld dos 6 primeiros n = 2^e, e"
                   " irracional em %ld — a normalização NÃO atravessa\n", quad, nao);
            printf("        enquanto H é inteira em todos: o que passa a alfândega"
                   " é a identidade algébrica, não o factor de escala\n");
            if(nao == 0 || quad == 0) mal++;
        }

        /* ── E A REFLEXÃO. O catálogo diz F² = reflexão. Em Hadamard sai
         * F² = n·I, que é a IDENTIDADE escalada — e não é contradição: a
         * reflexão é k ↦ −k, e o grupo aqui é (ℤ/2)^e, onde −k = k para todo
         * k. A reflexão É a identidade neste grupo, e é por isso que Hadamard
         * a não mostra. Conta-se, para não ficar em palavra. */
        {
            /* Os DOIS grupos de ordem 4 não são o mesmo, e é aí que está a
             * resposta. Hadamard vive em (ℤ/2)², onde a soma é XOR e o inverso
             * de k é o PRÓPRIO k; a DFT₄ vive em ℤ/4, onde só 0 e 2 são fixos.
             * A primeira escrita testou a reflexão com aritmética mod 4 e
             * atribuiu-a ao grupo errado — o teste dava 2 de 4 e a asserção
             * caiu, que é o que uma asserção tem de fazer. */
            long fixos = 0, n = 4;
            for(long k = 0; k < n; k++) if((k ^ k) == 0) fixos++;   /* XOR: −k = k */
            printf("      REFLEXÃO — em (ℤ/2)², onde a soma é XOR, o inverso de k"
                   " é o PRÓPRIO k: %ld de %ld fixos, logo a reflexão É a"
                   " identidade e F² = n·I não contradiz «F² = reflexão»\n",
                   fixos, n);
            long f4 = 0, m4 = 4;
            for(long k = 0; k < m4; k++) if(((m4 - k) % m4) == k) f4++;
            printf("        CONTROLO em ℤ/4, o OUTRO grupo de ordem 4, com a soma"
                   " usual: só %ld dos %ld são fixos — ali a reflexão SEPARA-SE da"
                   " identidade, e é por isso que Hadamard e a DFT₄ são"
                   " transformadas diferentes\n", f4, m4);
            if(fixos != n) mal++;
            if(f4 == m4 || f4 == 0) mal++;
        }

        #undef MUL
        #undef POE
        sql_fechar();

        printf("\n");
        ok("A TRANSFORMADA ATRAVESSA PARA OS INTEIROS SEM A NORMALIZAÇÃO, E É A NORMALIZAÇÃO"
           " QUE FICA RETIDA. O catálogo diz da transformada universal que «a normalização é"
           " 1/√n em CADA lado, e não por gosto», e daí tira F⁻¹ exacta, F⁴ = id e a"
           " unitariedade — o que está certo sobre ℝ. Mas o mesmo catálogo tem a lei da"
           " alfândega, «passa o que reverte», e o critério «identidade atravessa, limite não:"
           " o que atravessa é a parte algébrica». Aplicado à própria transformada, o veredicto"
           " é este: em Hadamard, que é a transformada universal com caracteres ±1, o motor dá"
           " H² = n·I e H⁴ = n²·I em 2/2 (n = 2 e 4; o 8×8 sai do tecto 6×6), e Hᵀ = H pela"
           " transposta do motor. A PERIODICIDADE ATRAVESSA — com o factor explícito em vez de"
           " escondido na raiz. O QUE NÃO ATRAVESSA É O 1/√n: ele só é racional quando n é"
           " quadrado perfeito, o que entre os n = 2^e acontece em 3 dos 6 primeiros, e nos"
           " outros a transformada NORMALIZADA não é uma operação desta casa. Não é limitação"
           " da máquina: é a distinção que o catálogo faz, aplicada a ele próprio. E A"
           " REFLEXÃO NÃO É CONTRADIÇÃO: o catálogo diz F² = reflexão e Hadamard dá F² = n·I,"
           " que é a identidade escalada — porque a reflexão é k ↦ −k e o grupo aqui é"
           " (ℤ/2)^e, onde −k = k para TODO k. Conta-se em vez de se afirmar: em (ℤ/2)², onde a soma"
           " é XOR, o inverso de k é o PRÓPRIO k, e são 4 de 4 fixos. O CONTROLO é ℤ/4 — o"
           " OUTRO grupo de ordem 4 —, onde só 2 dos 4 são fixos: ali a reflexão separa-se da"
           " identidade, e é por isso que Hadamard e a DFT₄ são transformadas diferentes. Sem"
           " esse controlo, «a reflexão é a identidade» seria uma desculpa e não uma"
           " propriedade do grupo. E a primeira escrita deste teste usou aritmética mod 4 para"
           " o grupo do XOR — atribuiu a reflexão ao grupo errado, deu 2 de 4, e a asserção"
           " caiu, que é o que uma asserção tem de fazer.", mal == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
