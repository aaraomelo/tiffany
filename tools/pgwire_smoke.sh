#!/bin/bash
# pgwire_smoke.sh — sobe listener Tiffany, cliente FEBE nosso (não psql), SELECT.
#   bash tools/pgwire_smoke.sh
set -euo pipefail
cd "$(dirname "$0")/.."
BASE=/tmp/pgwire_smoke
SRV=/tmp/pgwire_srv
PORTO=55432

cc -O2 -std=c99 -w -Ilib -Ibanco -DSQL_NO_MAIN -o "$SRV" banco/pgwire.c banco/sql.c -lm
cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sqlb banco/sql.c -lm
rm -f "$BASE.mem" "$BASE.prog"
/tmp/sqlb "$BASE" "CREATE TABLE t (a,b,c)" >/dev/null
/tmp/sqlb "$BASE" "INSERT INTO t VALUES (1,2,3)" >/dev/null

"$SRV" "$BASE" "$PORTO" &
SPID=$!
trap 'kill $SPID 2>/dev/null || true' EXIT
sleep 0.4

cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/pgwire_cli_smoke -x c - <<'EOF'
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "pgwire.h"
static int send_all(int fd, const void *b, int n){
    const unsigned char *p = b; int o = 0;
    while(o < n){ ssize_t w = send(fd, p + o, (size_t)(n - o), 0);
                  if(w <= 0) return -1; o += (int)w; }
    return 0;
}
static int recv_n(int fd, void *b, int n){
    unsigned char *p = b; int o = 0;
    while(o < n){ ssize_t r = recv(fd, p + o, (size_t)(n - o), 0);
                  if(r <= 0) return -1; o += (int)r; }
    return 0;
}
static int ate_ready(int fd){
    for(;;){
        unsigned char h[5]; int32_t mlen, pay;
        if(recv_n(fd, h, 5) < 0) return -1;
        mlen = pg_get_i32(h + 1); pay = mlen - 4;
        while(pay > 0){
            unsigned char t[256]; int k = pay > 256 ? 256 : pay;
            if(recv_n(fd, t, k) < 0) return -1; pay -= k;
        }
        if(h[0] == 'Z') return 0;
        if(h[0] == 'E') return -1;
    }
}
int main(void){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in a; PgBuf w;
    memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(55432);
    if(connect(fd, (struct sockaddr *)&a, sizeof a) < 0){ perror("connect"); return 1; }
    pg_buf_limpa(&w); pg_put_startup(&w, PG_PROTO_3_0, "smoke", "smoke");
    if(send_all(fd, w.b, w.n) < 0 || ate_ready(fd) < 0){ puts("startup falhou"); return 1; }
    pg_buf_limpa(&w); pg_put_query(&w, "SELECT * FROM t");
    if(send_all(fd, w.b, w.n) < 0 || ate_ready(fd) < 0){ puts("query falhou"); return 1; }
    pg_buf_limpa(&w); pg_put_terminate(&w); send_all(fd, w.b, w.n);
    puts("pgwire_smoke: SELECT ok via FEBE");
    return 0;
}
EOF

/tmp/pgwire_cli_smoke
echo "smoke FEBE ok (porto $PORTO)"
