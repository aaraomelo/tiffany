/* conecthus/lang/parse_claim.c — parser mínimo do formato textual Claim.
 * Sem AST sofisticada: linha a linha, chave valor. Claim ≠ Result. */
#include "claim.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void trim(char *s){
    char *a = s; while(*a && isspace((unsigned char)*a)) a++;
    if(a != s) memmove(s, a, strlen(a) + 1);
    size_t n = strlen(s);
    while(n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = 0;
}

static int comeca(const char *s, const char *pfx){
    size_t n = strlen(pfx);
    return strncmp(s, pfx, n) == 0 && (s[n] == 0 || isspace((unsigned char)s[n]));
}

static const char *depois(const char *s, const char *pfx){
    size_t n = strlen(pfx);
    const char *p = s + n;
    while(*p && isspace((unsigned char)*p)) p++;
    return p;
}

static void copia(char *dst, int cap, const char *src){
    if(cap < 1) return;
    int i = 0;
    while(src[i] && i < cap - 1){ dst[i] = src[i]; i++; }
    dst[i] = 0;
}

int claim_parse(const char *src, Claim *out){
    if(!src || !out) return -1;
    memset(out, 0, sizeof *out);
    out->law = -1;
    char linha[512];
    const char *p = src;
    int viu_claim = 0, viu_end = 0;
    while(*p){
        int i = 0;
        while(*p && *p != '\n' && i < (int)sizeof linha - 1) linha[i++] = *p++;
        linha[i] = 0;
        if(*p == '\n') p++;
        trim(linha);
        if(linha[0] == 0 || linha[0] == '#') continue;
        if(comeca(linha, "claim")){
            copia(out->name, sizeof out->name, depois(linha, "claim"));
            viu_claim = 1; continue;
        }
        if(comeca(linha, "end")){ viu_end = 1; break; }
        if(comeca(linha, "law")){ out->law = (int)strtol(depois(linha, "law"), 0, 10); continue; }
        if(comeca(linha, "object")){ copia(out->object, sizeof out->object, depois(linha, "object")); continue; }
        if(comeca(linha, "step")){ copia(out->step, sizeof out->step, depois(linha, "step")); continue; }
        if(comeca(linha, "back")){ copia(out->back, sizeof out->back, depois(linha, "back")); continue; }
        if(comeca(linha, "measure")){ copia(out->measure, sizeof out->measure, depois(linha, "measure")); continue; }
        if(comeca(linha, "invariant")){ copia(out->invariant, sizeof out->invariant, depois(linha, "invariant")); continue; }
        if(comeca(linha, "mutate")){ copia(out->mutate, sizeof out->mutate, depois(linha, "mutate")); continue; }
        if(comeca(linha, "classify")){ copia(out->classify, sizeof out->classify, depois(linha, "classify")); continue; }
        /* chave desconhecida: não engolir residual= no Claim */
        if(comeca(linha, "residual")) return -1;
    }
    if(!viu_claim || !viu_end) return -1;
    if(out->name[0] == 0 || out->law < 0) return -1;
    if(out->step[0] == 0 || out->back[0] == 0 || out->measure[0] == 0) return -1;
    return 0;
}

int claim_parse_file(const char *path, Claim *out){
    FILE *f = fopen(path, "rb");
    if(!f) return -1;
    if(fseek(f, 0, SEEK_END) != 0){ fclose(f); return -1; }
    long n = ftell(f);
    if(n < 0 || n > (1 << 20)){ fclose(f); return -1; }
    rewind(f);
    char *buf = (char *)malloc((size_t)n + 1);
    if(!buf){ fclose(f); return -1; }
    size_t rd = fread(buf, 1, (size_t)n, f);
    fclose(f);
    buf[rd] = 0;
    int r = claim_parse(buf, out);
    free(buf);
    return r;
}

int claim_emit(const Claim *c, char *out, int cap){
    if(!c || !out || cap < 32) return -1;
    int n = snprintf(out, (size_t)cap,
        "claim %s\n"
        "law %d\n"
        "object %s\n"
        "step %s\n"
        "back %s\n"
        "measure %s\n"
        "invariant %s\n"
        "mutate %s\n"
        "classify %s\n"
        "end\n",
        c->name, c->law, c->object, c->step, c->back, c->measure,
        c->invariant, c->mutate, c->classify);
    if(n < 0 || n >= cap) return -1;
    return n;
}
