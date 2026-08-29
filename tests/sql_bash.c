/* sql_bash.c — BASH MOVE no motor sql.c (subprocesso).
 *
 *   cc -O2 -std=c99 -w -Ilib -Ibanco -o /tmp/sql_bin banco/sql.c -lm
 *   cc -O2 -std=c99 -w -o /tmp/sql_bash tests/sql_bash.c
 *   /tmp/sql_bash /tmp/sql_bin
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#include <direct.h>
#define getpid _getpid
#define mkdir(p,m) _mkdir(p)
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++;
    if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

static int run_sql(const char *bin, const char *base, const char *q, char *out, size_t cap){
    char cmd[4096];
#ifdef _WIN32
    snprintf(cmd, sizeof cmd, "\"%s\" \"%s\" \"%s\"", bin, base, q);
#else
    snprintf(cmd, sizeof cmd, "\"%s\" \"%s\" \"%s\"", bin, base, q);
#endif
    FILE *f = popen(cmd, "r");
    if(!f) return -1;
    size_t n = 0;
    int c;
    while((c = fgetc(f)) != EOF && n + 1 < cap)
        out[n++] = (char)c;
    out[n] = 0;
    return pclose(f);
}

int main(int argc, char **argv){
    const char *bin = (argc > 1) ? argv[1] : "app/.cache/tiffany_sql";
    char base[512];
#ifdef _WIN32
    const char *tmp = getenv("TEMP");
    if(!tmp || !*tmp) tmp = "C:\\temp";
    snprintf(base, sizeof base, "%s\\sql_bash_%d", tmp, (int)getpid());
    mkdir(tmp, 0755);
#else
    snprintf(base, sizeof base, "/tmp/sql_bash_%d", (int)getpid());
#endif
    char out[65536];
    run_sql(bin, base, "BASH MOVE 'echo ouro'", out, sizeof out);
    ok("BASH MOVE atómico contem ouro", strstr(out, "ouro") != 0);
    run_sql(bin, base, "BASH MOVE -1 'echo prata'", out, sizeof out);
    ok("BASH MOVE -1 ok", strstr(out, "BASH MOVE -1") != 0);
    run_sql(bin, base, "BASH MOVE +1", out, sizeof out);
    ok("BASH MOVE +1 contem prata", strstr(out, "prata") != 0);
    printf("#TOTAL %d %d\n", feitas, falhas);
    return falhas ? 1 : 0;
}
