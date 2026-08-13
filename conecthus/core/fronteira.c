/* conecthus/core/fronteira.c — interface entre Claims (sem fundir). */
#include "fronteira.h"
#include <stdio.h>
#include <string.h>

long fronteira_gut_5w2h(const int *score, const int *acao, int n){
    if(!score || !acao || n < 2) return 0;
    long inv = 0;
    for(int i = 0; i < n - 1; i++){
        int a = acao[i], b = acao[i + 1];
        if(a < 0 || a >= n || b < 0 || b >= n){ inv++; continue; }
        if(score[a] < score[b]) inv++;
    }
    return inv;
}

long fronteira_why_ishikawa(int root, const int *causes, int n){
    if(!causes || n < 1) return 1;
    for(int i = 0; i < n; i++)
        if(causes[i] == root) return 0;
    return 1;
}

long fronteira_pdca_vsm(int plan_target, int vsm_future){
    long d = (long)plan_target - (long)vsm_future;
    return d < 0 ? -d : d;
}

long residual_estacao(int projectado, int medido){
    long d = (long)projectado - (long)medido;
    return d < 0 ? -d : d;
}

int estacao_fecha(int projectado, int medido, int external){
    return (external != 0) && residual_estacao(projectado, medido) == 0;
}

int implante_emit(int p, int m, int ext, char *out, int cap){
    long R = residual_estacao(p, m);
    if(!out || cap < 24) return -1;
    return snprintf(out, (size_t)cap, "implante P=%d M=%d R=%ld fonte=%s",
                    p, m, R, ext ? "MES" : "reu");
}

int implante_parse(const char *src, int *p, int *m, int *ext){
    if(!src || !p || !m || !ext) return -1;
    int pp = 0, mm = 0, rr = 0;
    char fonte[8];
    memset(fonte, 0, sizeof fonte);
    if(sscanf(src, "implante P=%d M=%d R=%d fonte=%7s", &pp, &mm, &rr, fonte) != 4)
        return -1;
    *p = pp; *m = mm;
    *ext = (fonte[0] == 'M') ? 1 : 0;
    if(residual_estacao(pp, mm) != rr) return -1;
    return 0;
}

long residual_patria(int local, int live){
    return (local && live) ? 0 : 1;
}

int patria_fecha(int local, int live, int external){
    return (external != 0) && local && live;
}
