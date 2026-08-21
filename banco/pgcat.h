/* pgcat.h — TRIO PG6: o catálogo mínimo, e ele é da SESSÃO e não do banco.
 *
 * Um cliente real (psql, um driver) não começa por consultar dados: começa por
 * perguntar QUEM É o servidor e em que estado está. Essas perguntas não tocam no
 * .mem nem na ISA — são sobre a ligação. Por isso o catálogo é uma FACHADA
 * ANTES do motor, e não mais um comando dentro dele:
 *
 *     sql_executa(sql) ─┬─ pgcat_responde(sql)  →  tratou?  devolve
 *                       └─ executa(sql)         →  o motor, o bytecode, o disco
 *
 * O que se trata aqui, e nada mais:
 *
 *     SELECT version()                a identidade do servidor
 *     SHOW <nome> | SHOW ALL          ler um parâmetro de sessão
 *     SET <nome> = <valor>            escrever um parâmetro de sessão
 *     SELECT current_database()       a base aberta
 *     SELECT current_schema()         o esquema corrente
 *     SELECT current_user | user      quem está ligado
 *     BEGIN | COMMIT | ROLLBACK       as tags que os drivers esperam
 *
 * E A IDA GUARDA A VOLTA: `SET x = v` seguido de `SHOW x` devolve v. Um
 * catálogo que aceitasse o SET e não o lesse de volta não seria catálogo — era
 * um sorvedouro. É essa a asserção que dá conteúdo a esta camada.
 *
 * SEM RAM DINÂMICA: os parâmetros vivem num arranjo fixo, decidido em compilação.
 */
#ifndef PGCAT_H
#define PGCAT_H

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include "sql_api.h"

#define PGCAT_MAX_PARAM   24
#define PGCAT_NOME        48
#define PGCAT_VALOR       64
#define PGCAT_VERSAO      "Tiffany-pgwire/0.1"

typedef struct {
    char nome[PGCAT_NOME];
    char valor[PGCAT_VALOR];
} PgParam;

static PgParam pgcat_par[PGCAT_MAX_PARAM] = {
    /* os que o handshake já anuncia — aqui para poderem ser lidos e escritos */
    { "server_version",  PGCAT_VERSAO },
    { "client_encoding", "UTF8"       },
    { "server_encoding", "UTF8"       },
    { "DateStyle",       "ISO, MDY"   },
    { "TimeZone",        "UTC"        },
    { "application_name","" },
    { "search_path",     "public"     },
    { "is_superuser",    "off"        },
};
static int  pgcat_n = 8;
static char pgcat_base[128] = "tiffany";
static char pgcat_user[64]  = "tiffany";

/* ── utilitários: comparação sem maiúsculas, e o corte dos brancos ─────────── */
static int pgcat_ieq(const char *a, const char *b){
    while(*a && *b){
        if(tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++; b++;
    }
    return *a == 0 && *b == 0;
}
static const char *pgcat_pula(const char *s){
    while(*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    return s;
}
/* consome a palavra se ela lá estiver; devolve o ponteiro depois dela, ou NULL */
static const char *pgcat_pal(const char *s, const char *pal){
    size_t n = strlen(pal);
    s = pgcat_pula(s);
    if(strncasecmp(s, pal, n)) return NULL;
    if(s[n] && !strchr(" \t\r\n;(", s[n])) return NULL;
    return pgcat_pula(s + n);
}
/* copia o identificador (letras, dígitos, _) e devolve o resto */
static const char *pgcat_ident(const char *s, char *dst, int cap){
    int i = 0;
    s = pgcat_pula(s);
    while(*s && (isalnum((unsigned char)*s) || *s == '_' || *s == '.')){
        if(i + 1 < cap) dst[i++] = *s;
        s++;
    }
    dst[i] = 0;
    return s;
}
/* tira aspas simples e o ponto e vírgula final */
static void pgcat_limpa(char *v){
    int n = (int)strlen(v);
    while(n > 0 && (v[n-1] == ';' || v[n-1] == ' ' || v[n-1] == '\t')) v[--n] = 0;
    if(n >= 2 && ((v[0] == '\'' && v[n-1] == '\'') || (v[0] == '"' && v[n-1] == '"'))){
        memmove(v, v + 1, (size_t)(n - 2));
        v[n-2] = 0;
    }
}

static PgParam *pgcat_acha(const char *nome){
    for(int i = 0; i < pgcat_n; i++)
        if(pgcat_ieq(pgcat_par[i].nome, nome)) return &pgcat_par[i];
    return NULL;
}

/* ── as respostas, todas com a forma que o wire espera ─────────────────────── */
static void pgcat_uma(SqlOut *o, const char *col, const char *val, const char *tag){
    if(!o) return;
    memset(o, 0, sizeof *o);
    o->ok = 1; o->ncols = 1; o->nrows = 1;
    snprintf(o->col[0], sizeof o->col[0], "%s", col);
    snprintf(o->cell[0][0], sizeof o->cell[0][0], "%s", val);
    snprintf(o->tag, sizeof o->tag, "%s", tag);
}
static void pgcat_so_tag(SqlOut *o, const char *tag){
    if(!o) return;
    memset(o, 0, sizeof *o);
    o->ok = 1; o->ncols = 0; o->nrows = 0;
    snprintf(o->tag, sizeof o->tag, "%s", tag);
}

/* Devolve 1 se ESTA camada tratou a consulta (e preencheu out); 0 se não é
 * dela — e então o motor corre, como se esta camada não existisse. */
static int pgcat_responde(const char *sql, SqlOut *out){
    const char *p;
    char nome[PGCAT_NOME], valor[PGCAT_VALOR];

    if(!sql) return 0;

    /* ── SHOW ───────────────────────────────────────────────────────────── */
    if((p = pgcat_pal(sql, "SHOW")) != NULL){
        p = pgcat_ident(p, nome, sizeof nome);
        if(pgcat_ieq(nome, "ALL")){
            if(out){
                memset(out, 0, sizeof *out);
                out->ok = 1; out->ncols = 2;
                snprintf(out->col[0], sizeof out->col[0], "name");
                snprintf(out->col[1], sizeof out->col[1], "setting");
                int n = pgcat_n < SQL_OUT_MAX_ROWS ? pgcat_n : SQL_OUT_MAX_ROWS;
                for(int i = 0; i < n; i++){
                    snprintf(out->cell[i][0], SQL_OUT_CELL, "%s", pgcat_par[i].nome);
                    snprintf(out->cell[i][1], SQL_OUT_CELL, "%s", pgcat_par[i].valor);
                }
                out->nrows = n;
                snprintf(out->tag, sizeof out->tag, "SHOW");
            }
            return 1;
        }
        {
            PgParam *q = pgcat_acha(nome);
            /* um parâmetro desconhecido é ERRO, e não uma linha vazia: o cliente
             * tem de saber a diferença entre «vale isto» e «não existe». */
            if(!q){
                if(out){
                    memset(out, 0, sizeof *out);
                    out->ok = 0;
                    snprintf(out->err, sizeof out->err,
                             "unrecognized configuration parameter \"%s\"", nome);
                }
                return 1;
            }
            pgcat_uma(out, q->nome, q->valor, "SHOW");
            return 1;
        }
    }

    /* ── SET nome = valor  |  SET nome TO valor ─────────────────────────── */
    if((p = pgcat_pal(sql, "SET")) != NULL){
        const char *q;
        p = pgcat_ident(p, nome, sizeof nome);
        p = pgcat_pula(p);
        if(*p == '=') p++;
        else if((q = pgcat_pal(p, "TO")) != NULL) p = q;
        else return 0;                       /* não é a nossa forma: passa */
        p = pgcat_pula(p);
        snprintf(valor, sizeof valor, "%s", p);
        pgcat_limpa(valor);
        {
            PgParam *r = pgcat_acha(nome);
            if(!r && pgcat_n < PGCAT_MAX_PARAM) r = &pgcat_par[pgcat_n++];
            if(r){
                snprintf(r->nome, sizeof r->nome, "%s", nome);
                snprintf(r->valor, sizeof r->valor, "%s", valor);
            }
        }
        pgcat_so_tag(out, "SET");
        return 1;
    }

    /* ── as transacções: os drivers mandam-nas antes de tudo ────────────── */
    if(pgcat_pal(sql, "BEGIN"))    { pgcat_so_tag(out, "BEGIN");    return 1; }
    if(pgcat_pal(sql, "COMMIT"))   { pgcat_so_tag(out, "COMMIT");   return 1; }
    if(pgcat_pal(sql, "ROLLBACK")) { pgcat_so_tag(out, "ROLLBACK"); return 1; }

    /* ── SELECT <função de catálogo>() ──────────────────────────────────── */
    if((p = pgcat_pal(sql, "SELECT")) != NULL){
        const char *q = p;
        char alvo[PGCAT_NOME];
        q = pgcat_ident(q, alvo, sizeof alvo);
        /* pg_catalog.version() ≡ version() */
        if(!strncasecmp(alvo, "pg_catalog.", 11)) memmove(alvo, alvo + 11, strlen(alvo) - 10);
        q = pgcat_pula(q);
        {
            int tem_par = (*q == '(');
            if(tem_par){ q++; q = pgcat_pula(q); if(*q != ')') return 0; }
            if(pgcat_ieq(alvo, "version") && tem_par){
                pgcat_uma(out, "version", PGCAT_VERSAO, "SELECT 1"); return 1; }
            if(pgcat_ieq(alvo, "current_database") && tem_par){
                pgcat_uma(out, "current_database", pgcat_base, "SELECT 1"); return 1; }
            if(pgcat_ieq(alvo, "current_schema") && tem_par){
                pgcat_uma(out, "current_schema", "public", "SELECT 1"); return 1; }
            if(pgcat_ieq(alvo, "current_user") || pgcat_ieq(alvo, "user")
               || pgcat_ieq(alvo, "session_user")){
                if(!tem_par){ pgcat_uma(out, alvo, pgcat_user, "SELECT 1"); return 1; }
            }
        }
        return 0;                            /* SELECT normal: é do motor */
    }

    return 0;
}

/* a base aberta, para o current_database() responder o que é */
static void pgcat_base_nome(const char *nome){
    const char *b = nome ? nome : "tiffany";
    const char *s = strrchr(b, '/');
    snprintf(pgcat_base, sizeof pgcat_base, "%s", s ? s + 1 : b);
}

#endif
