/* pqlike.h — A FACHADA libpq, ESCRITA AQUI, SOBRE O NOSSO PRÓPRIO WIRE (Trio PG5).
 *
 * O que uma aplicação C espera de um banco não é o nosso argv: é `PQconnectdb`,
 * `PQexec`, `PQntuples`, `PQgetvalue`. Essa superfície é a PORTA — e a porta
 * escreve-se, não se importa. Aqui não se linka `-lpq` do sistema: cada byte que
 * sai daqui é o FEBE do `lib/pgwire.h`, e do outro lado está o `banco/pgwire.c`,
 * que fala com o `banco/sql.c`. A forma dos nomes vem da documentação do libpq; a
 * realização é da casa.
 *
 *     app C  ──PQexec──►  pqlike.h  ──FEBE──►  pgwire.c  ──►  sql.c  ──►  .mem
 *
 * ── O QUE É DIFERENTE DO libpq, E É DE PROPÓSITO ──────────────────────────────
 *
 * SEM malloc. O libpq devolve `PGresult *` de memória pedida em execução; esta casa
 * não pede memória em execução (é a mesma decisão do `tools/libc.c`, que também não
 * tem malloc). O resultado vive DENTRO da ligação, um de cada vez, e `PQclear` não
 * liberta nada: marca o slot como livre. A consequência tem de ser dita porque muda
 * o contrato: **dois PQexec seguidos sobrescrevem o resultado do primeiro**, e quem
 * precisa dos dois copia antes. Um `PGresult *` daqui não sobrevive ao PQexec
 * seguinte, e o libpq de verdade sobreviveria.
 *
 * SEM assíncrono, sem cursores, sem COPY, sem LISTEN/NOTIFY. Só o Simple Query, que
 * é o que o metal mede. O que não está, recusa-se com erro claro — não em silêncio.
 *
 * TAMANHOS FIXOS. As linhas e as colunas cabem no que o `sql_api.h` já declara
 * (SQL_OUT_MAX_ROWS, SQL_OUT_MAX_COLS, SQL_OUT_CELL); acima disso o resultado
 * marca truncado e diz-se, em vez de crescer calado.
 *
 *   cc -O2 -std=c99 -Ilib -Ibanco  …  (precisa de pgwire.h e sql_api.h)
 */
#ifndef PQLIKE_H
#define PQLIKE_H

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "pgwire.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* ── os nomes do libpq, com os mesmos valores onde eles são observáveis ─────── */
typedef enum { CONNECTION_OK = 0, CONNECTION_BAD = 1 } PQConnStatusType;
typedef enum {
    PGRES_EMPTY_QUERY = 0,
    PGRES_COMMAND_OK  = 1,     /* DDL/DML: não devolve linhas */
    PGRES_TUPLES_OK   = 2,     /* SELECT: devolve linhas */
    PGRES_FATAL_ERROR = 7
} PQExecStatusType;

#ifndef PQLIKE_RECV_CAP
#define PQLIKE_RECV_CAP 16384
#endif

typedef struct {
    PQExecStatusType estado;
    int ncols, nrows;
    int truncado;                       /* linhas/colunas que não couberam */
    char col[SQL_OUT_MAX_COLS][32];
    /* O TIPO QUE O SERVIDOR ANUNCIOU. Antes o cliente saltava o OID e ninguém
     * via que tudo ia como int4 — inclusive o texto do catálogo. Guardar aqui é
     * o que permite ao teste comparar o que foi dito com o que era. */
    int  ftype[SQL_OUT_MAX_COLS];       /* typeOID */
    int  fsize[SQL_OUT_MAX_COLS];       /* typlen: 4 (int4) ou −1 (variável) */
    char cell[SQL_OUT_MAX_ROWS][SQL_OUT_MAX_COLS][SQL_OUT_CELL];
    char tag[80];                       /* CommandComplete: "SELECT 2", … */
    char err[200];
    int  vivo;                          /* 0 = slot livre (PQclear) */
} PGresult;

typedef struct {
    int fd;
    PQConnStatusType estado;
    char err[200];
    char servidor[64];                  /* ParameterStatus server_version */
    int32_t pid, chave;                 /* BackendKeyData */
    PGresult res;                       /* UM resultado de cada vez — ver o cabeçalho */
} PGconn;

/* ── I/O do cliente: nomes próprios, para não colidir com o listener ────────── */
static int pq_send_all(int fd, const void *buf, int n){
    const uint8_t *p = (const uint8_t *)buf;
    int o = 0;
    while(o < n){
        ssize_t w = send(fd, p + o, (size_t)(n - o), 0);
        if(w <= 0) return -1;
        o += (int)w;
    }
    return 0;
}
static int pq_recv_n(int fd, void *buf, int n){
    uint8_t *p = (uint8_t *)buf;
    int o = 0;
    while(o < n){
        ssize_t r = recv(fd, p + o, (size_t)(n - o), 0);
        if(r <= 0) return -1;
        o += (int)r;
    }
    return 0;
}

/* conninfo mínimo: "host=… port=… user=… dbname=…", separado por espaços.
 * O que não vier tem default declarado — e o default DIZ-SE, não se adivinha. */
static void pq_le_conninfo(const char *s, char *host, int hn, int *porto,
                           char *user, int un, char *base, int bn){
    snprintf(host, (size_t)hn, "127.0.0.1");
    *porto = 5432;
    snprintf(user, (size_t)un, "tiffany");
    base[0] = 0;
    if(!s) return;
    while(*s){
        const char *k, *v;
        int kl, vl;
        while(*s == ' ' || *s == '\t') s++;
        if(!*s) break;
        k = s;
        while(*s && *s != '=' && *s != ' ') s++;
        kl = (int)(k - k) + (int)(s - k);
        if(*s != '='){ while(*s && *s != ' ') s++; continue; }
        s++;
        v = s;
        while(*s && *s != ' ') s++;
        vl = (int)(s - v);
        if(kl == 4 && !strncmp(k, "host", 4)){
            if(vl < hn){ memcpy(host, v, (size_t)vl); host[vl] = 0; }
        }else if(kl == 4 && !strncmp(k, "port", 4)){
            char t[16];
            if(vl < (int)sizeof t){ memcpy(t, v, (size_t)vl); t[vl] = 0; *porto = atoi(t); }
        }else if(kl == 4 && !strncmp(k, "user", 4)){
            if(vl < un){ memcpy(user, v, (size_t)vl); user[vl] = 0; }
        }else if(kl == 6 && !strncmp(k, "dbname", 6)){
            if(vl < bn){ memcpy(base, v, (size_t)vl); base[vl] = 0; }
        }
    }
}

/* Lê o handshake até ReadyForQuery, guardando o que o backend disse de si. */
static int pq_le_ate_ready(PGconn *c){
    for(;;){
        uint8_t hdr[5], pay[PQLIKE_RECV_CAP];
        int32_t mlen, pn;
        if(pq_recv_n(c->fd, hdr, 5) < 0) return 0;
        mlen = pg_get_i32(hdr + 1);
        if(mlen < 4 || mlen - 4 > PQLIKE_RECV_CAP) return 0;
        pn = mlen - 4;
        if(pn > 0 && pq_recv_n(c->fd, pay, pn) < 0) return 0;
        if(pn < PQLIKE_RECV_CAP) pay[pn] = 0;
        switch((char)hdr[0]){
        case PG_MSG_AUTH:
            if(pn < 4 || pg_get_i32(pay) != PG_AUTH_OK){
                snprintf(c->err, sizeof c->err,
                         "autenticação não é trust (método %d)",
                         pn >= 4 ? (int)pg_get_i32(pay) : -1);
                return 0;
            }
            break;
        case PG_MSG_PARAM_STATUS: {
            const char *nome = (const char *)pay;
            const char *valor = nome + strlen(nome) + 1;
            if(!strcmp(nome, "server_version"))
                snprintf(c->servidor, sizeof c->servidor, "%s", valor);
            break; }
        case PG_MSG_BACKEND_KEY:
            if(pn >= 8){ c->pid = pg_get_i32(pay); c->chave = pg_get_i32(pay + 4); }
            break;
        case PG_MSG_ERROR: {
            /* campos code+string até ao 0 final; queremos o 'M' */
            int i = 0;
            while(i < pn && pay[i]){
                char code = (char)pay[i];
                const char *v = (const char *)(pay + i + 1);
                if(code == PG_ERR_MESSAGE) snprintf(c->err, sizeof c->err, "%s", v);
                i += 1 + (int)strlen(v) + 1;
            }
            return 0; }
        case PG_MSG_READY:
            return 1;
        default:
            break;                      /* NoticeResponse e afins: ignoram-se */
        }
    }
}

static PGconn *PQconnectdb(const char *conninfo);   /* declarado antes do uso interno */

static PGconn *PQconnectdb(const char *conninfo){
    static PGconn c;                    /* uma ligação por processo — sem malloc */
    char host[64], user[64], base[64];
    int porto = 5432;
    struct sockaddr_in a;
    PgBuf w;

    memset(&c, 0, sizeof c);
    c.fd = -1;
    c.estado = CONNECTION_BAD;
    pq_le_conninfo(conninfo, host, (int)sizeof host, &porto,
                   user, (int)sizeof user, base, (int)sizeof base);
    if(!base[0]) snprintf(base, sizeof base, "%s", user);

    c.fd = socket(AF_INET, SOCK_STREAM, 0);
    if(c.fd < 0){ snprintf(c.err, sizeof c.err, "socket: falhou"); return &c; }
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = inet_addr(host);
    a.sin_port = htons((uint16_t)porto);
    if(connect(c.fd, (struct sockaddr *)&a, sizeof a) < 0){
        snprintf(c.err, sizeof c.err, "connect %s:%d falhou", host, porto);
        close(c.fd); c.fd = -1;
        return &c;
    }

    pg_buf_limpa(&w);
    pg_put_startup(&w, PG_PROTO_3_0, user, base);
    if(w.erro || pq_send_all(c.fd, w.b, w.n) < 0){
        snprintf(c.err, sizeof c.err, "startup não saiu");
        close(c.fd); c.fd = -1;
        return &c;
    }
    if(!pq_le_ate_ready(&c)){
        if(!c.err[0]) snprintf(c.err, sizeof c.err, "handshake não fechou em ReadyForQuery");
        close(c.fd); c.fd = -1;
        return &c;
    }
    c.estado = CONNECTION_OK;
    return &c;
}

static PQConnStatusType PQstatus(const PGconn *c){
    return c ? c->estado : CONNECTION_BAD;
}
static const char *PQerrorMessage(const PGconn *c){
    return (c && c->err[0]) ? c->err : "";
}
static const char *PQparameterStatus(const PGconn *c, const char *nome){
    if(!c || !nome) return NULL;
    if(!strcmp(nome, "server_version")) return c->servidor;
    return NULL;
}
static int PQbackendPID(const PGconn *c){ return c ? (int)c->pid : 0; }

static void PQfinish(PGconn *c){
    if(!c) return;
    if(c->fd >= 0){
        PgBuf w;
        pg_buf_limpa(&w);
        pg_put_terminate(&w);
        if(!w.erro) pq_send_all(c->fd, w.b, w.n);   /* 'X': despedir-se é do protocolo */
        close(c->fd);
        c->fd = -1;
    }
    c->estado = CONNECTION_BAD;
}

/* ── PQexec: Query('Q') → linhas → ReadyForQuery ────────────────────────────── */
static PGresult *PQexec(PGconn *c, const char *sql){
    PgBuf q;
    PGresult *r;

    if(!c) return NULL;
    r = &c->res;
    memset(r, 0, sizeof *r);
    r->vivo = 1;
    r->estado = PGRES_FATAL_ERROR;

    if(c->estado != CONNECTION_OK || c->fd < 0){
        snprintf(r->err, sizeof r->err, "ligação não está aberta");
        return r;
    }
    pg_buf_limpa(&q);
    pg_put_query(&q, sql ? sql : "");
    if(q.erro || pq_send_all(c->fd, q.b, q.n) < 0){
        snprintf(r->err, sizeof r->err, "Query não saiu");
        return r;
    }

    for(;;){
        uint8_t hdr[5], pay[PQLIKE_RECV_CAP];
        int32_t mlen, pn;
        if(pq_recv_n(c->fd, hdr, 5) < 0){
            snprintf(r->err, sizeof r->err, "ligação caiu a meio da resposta");
            return r;
        }
        mlen = pg_get_i32(hdr + 1);
        if(mlen < 4 || mlen - 4 > PQLIKE_RECV_CAP){
            snprintf(r->err, sizeof r->err, "mensagem de %d bytes não cabe", (int)mlen);
            return r;
        }
        pn = mlen - 4;
        if(pn > 0 && pq_recv_n(c->fd, pay, pn) < 0){
            snprintf(r->err, sizeof r->err, "payload truncado");
            return r;
        }
        if(pn < PQLIKE_RECV_CAP) pay[pn] = 0;

        switch((char)hdr[0]){
        case PG_MSG_ROW_DESC: {
            int nf = pn >= 2 ? pg_get_i16(pay) : 0, i = 2, k;
            r->ncols = nf > SQL_OUT_MAX_COLS ? SQL_OUT_MAX_COLS : nf;
            if(nf > SQL_OUT_MAX_COLS) r->truncado = 1;
            for(k = 0; k < nf && i < pn; k++){
                const char *nome = (const char *)(pay + i);
                int L = (int)strlen(nome);
                if(k < r->ncols){
                    snprintf(r->col[k], sizeof r->col[k], "%s", nome);
                    /* o OID e o typlen vêm depois de nome\0 + tableOID(4) + attr(2) */
                    if(i + L + 1 + 4 + 2 + 4 + 2 <= pn){
                        r->ftype[k] = pg_get_i32(pay + i + L + 1 + 4 + 2);
                        r->fsize[k] = pg_get_i16(pay + i + L + 1 + 4 + 2 + 4);
                    }
                }
                i += L + 1 + 4 + 2 + 4 + 2 + 4 + 2;   /* table,attr,oid,len,mod,fmt */
            }
            r->estado = PGRES_TUPLES_OK;
            break; }
        case PG_MSG_DATA_ROW: {
            int nc = pn >= 2 ? pg_get_i16(pay) : 0, i = 2, k;
            int linha = r->nrows;
            if(linha >= SQL_OUT_MAX_ROWS){ r->truncado = 1; break; }
            for(k = 0; k < nc && i + 4 <= pn; k++){
                int32_t L = pg_get_i32(pay + i);
                i += 4;
                if(L < 0){                                  /* NULL do protocolo */
                    if(k < SQL_OUT_MAX_COLS) r->cell[linha][k][0] = 0;
                    continue;
                }
                if(i + L > pn) break;
                if(k < SQL_OUT_MAX_COLS){
                    int n = L < SQL_OUT_CELL - 1 ? L : SQL_OUT_CELL - 1;
                    if(n < L) r->truncado = 1;
                    memcpy(r->cell[linha][k], pay + i, (size_t)n);
                    r->cell[linha][k][n] = 0;
                }
                i += L;
            }
            r->nrows++;
            break; }
        case PG_MSG_CMD_COMPLETE:
            snprintf(r->tag, sizeof r->tag, "%s", (const char *)pay);
            if(r->estado != PGRES_TUPLES_OK) r->estado = PGRES_COMMAND_OK;
            break;
        case PG_MSG_ERROR: {
            int i = 0;
            while(i < pn && pay[i]){
                char code = (char)pay[i];
                const char *v = (const char *)(pay + i + 1);
                if(code == PG_ERR_MESSAGE) snprintf(r->err, sizeof r->err, "%s", v);
                i += 1 + (int)strlen(v) + 1;
            }
            r->estado = PGRES_FATAL_ERROR;
            break; }
        case PG_MSG_READY:
            return r;                    /* 'Z' fecha o ciclo, e só ele */
        default:
            break;
        }
    }
}

static PQExecStatusType PQresultStatus(const PGresult *r){
    return r ? r->estado : PGRES_FATAL_ERROR;
}
static const char *PQresStatus(PQExecStatusType s){
    switch(s){
    case PGRES_EMPTY_QUERY: return "PGRES_EMPTY_QUERY";
    case PGRES_COMMAND_OK:  return "PGRES_COMMAND_OK";
    case PGRES_TUPLES_OK:   return "PGRES_TUPLES_OK";
    default:                return "PGRES_FATAL_ERROR";
    }
}
static int PQntuples(const PGresult *r){ return r ? r->nrows : 0; }
static int PQnfields(const PGresult *r){ return r ? r->ncols : 0; }
/* o tipo que o servidor anunciou para a coluna — o mesmo nome do libpq */
static int PQftype(const PGresult *r, int c){
    return (r && c >= 0 && c < r->ncols) ? r->ftype[c] : 0;
}
static int PQfsize(const PGresult *r, int c){
    return (r && c >= 0 && c < r->ncols) ? r->fsize[c] : 0;
}
static const char *PQfname(const PGresult *r, int col){
    if(!r || col < 0 || col >= r->ncols) return NULL;
    return r->col[col];
}
static const char *PQgetvalue(const PGresult *r, int lin, int col){
    if(!r || lin < 0 || lin >= r->nrows || col < 0 || col >= r->ncols) return "";
    return r->cell[lin][col];
}
static int PQgetisnull(const PGresult *r, int lin, int col){
    const char *v = PQgetvalue(r, lin, col);
    return v[0] == 0;
}
static const char *PQcmdStatus(const PGresult *r){ return r ? r->tag : ""; }
static const char *PQresultErrorMessage(const PGresult *r){ return r ? r->err : ""; }
static int PQresultTruncado(const PGresult *r){ return r ? r->truncado : 0; }

/* PQclear NÃO liberta: marca o slot. Está aqui para o código da aplicação ser o
 * mesmo, e para o dizer — ver o cabeçalho sobre o resultado que não sobrevive ao
 * PQexec seguinte. */
static void PQclear(PGresult *r){ if(r) r->vivo = 0; }

#endif
