/* stratum.h — O POOL COMO BACKEND. O banco lê o job de um slot; não sabe que houve TCP.
 *
 * Mesma fronteira do canal: acima de S_POOL, LOAD vai buscar um campo do job corrente e STORE
 * emite a share. O protocolo fica atrás destas funções, e é por isso que ele é indistinguível —
 * trocar stratum por outra coisa é trocar aqui dentro, e nem uma linha de SQL muda.
 *
 * O JSON do stratum é de linha e de forma conhecida; não é preciso analisador geral, e um
 * analisador geral seria peça a mais. Lê-se o campo pela posição, que é o que o protocolo define.
 */
#ifndef STRATUM_H
#define STRATUM_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

typedef struct {
    int fd;
    char extranonce1[32]; int en1_len;
    int  en2_size;
    char job_id[64];
    unsigned char prevhash[32], merkle_raiz[32];
    unsigned versao, nbits, ntime;
    int tem_job;
    char buf[8192]; size_t nbuf;
    int id;
} Pool;

static int hexval(int c){
    return (c>='0'&&c<='9')?c-'0':(c>='a'&&c<='f')?c-'a'+10:(c>='A'&&c<='F')?c-'A'+10:-1;
}
static int st_hex(const char *s, size_t n, unsigned char *out){
    if(n % 2) return 0;
    for(size_t i = 0; i < n; i += 2){
        int a = hexval(s[i]), b = hexval(s[i+1]);
        if(a < 0 || b < 0) return 0;
        out[i/2] = (unsigned char)(a*16 + b);
    }
    return 1;
}
/* A TRADUÇÃO É DO HIPERCORPO, E NÃO DE VARREDURA DE STRING.
 *
 * Eu escrevi um analisador que contava aspas e ele partiu duas vezes: primeiro com o array da
 * merkle branch (que desloca as posições conforme o número de ramos), depois com uma ASPA
 * ESCAPADA — JSON válido, e devolvia zeros. Remendar terceira vez seria esperar a quarta.
 *
 * JSON é recursão auto-similar: um nível contém cópias de si, que é a lei do tesseracto
 * (`M_k = M_{k-1}A_1`, o nível k carrega o k-1). E a posição de um campo NÃO é uma contagem de
 * símbolos: é o seu CAMINHO — params, depois o 5.o — e caminho é o que a cifra endereça. Logo
 * não se varre: DESCE-SE, um nível por termo, e o aninhamento fica certo por construção porque
 * o aninhamento É a descida.
 *
 * Uma string lê-se num sítio só, e é lá que o escape se respeita — não espalhado por um scanner. */
static const char *js_fim_string(const char *p){        /* p aponta ao " de abertura */
    p++;
    while(*p){
        if(*p == '\\' && p[1]) p += 2;                  /* o escape: dois símbolos, um valor */
        else if(*p == '"') return p;
        else p++;
    }
    return NULL;
}
static const char *js_fim_valor(const char *p){         /* o fim do valor que começa em p */
    if(*p == '"'){ const char *e = js_fim_string(p); return e ? e + 1 : NULL; }
    if(*p == '[' || *p == '{'){
        char ab = *p, fe = (ab == '[') ? ']' : '}';
        int prof = 0;
        while(*p){
            if(*p == '"'){ const char *e = js_fim_string(p); if(!e) return NULL; p = e + 1; continue; }
            if(*p == ab) prof++;
            else if(*p == fe){ prof--; if(!prof) return p + 1; }
            p++;
        }
        return NULL;
    }
    while(*p && *p != ',' && *p != ']' && *p != '}') p++;
    return p;
}
/* Desce um nível: devolve o k-ésimo elemento do container que começa em p. */
static const char *js_desce(const char *p, int k){
    if(*p != '[' && *p != '{') return NULL;
    p++;
    for(int n = 0; *p; n++){
        while(*p == ' ' || *p == '\t') p++;
        if(*p == ']' || *p == '}') return NULL;
        const char *fim = js_fim_valor(p);
        if(!fim) return NULL;
        if(n == k) return p;
        p = fim;
        while(*p == ' ' || *p == '\t') p++;
        if(*p == ',') p++;
    }
    return NULL;
}
/* O CAMINHO: um termo por nível, como a cifra. Devolve a string do fim, sem as aspas. */
static const char *js_caminho(const char *raiz, const int *cam, int n, size_t *len){
    const char *p = raiz;
    for(int i = 0; i < n; i++){
        p = js_desce(p, cam[i]);
        if(!p) return NULL;
    }
    if(*p != '"') return NULL;
    const char *e = js_fim_string(p);
    if(!e) return NULL;
    *len = (size_t)(e - p - 1);
    return p + 1;
}
/* o k-ésimo parâmetro do mining.notify, pelo caminho */
static const char *st_param(const char *linha, int k, size_t *len){
    const char *p = strstr(linha, "\"params\"");
    if(!p) return NULL;
    while(*p && *p != '[') p++;
    if(!*p) return NULL;
    int cam[1] = { k };
    return js_caminho(p, cam, 1, len);
}
static int st_envia(Pool *P, const char *s){
    size_t n = strlen(s);
    return write(P->fd, s, n) == (ssize_t)n;
}
static int st_liga(Pool *P, const char *host, int porta, const char *user){
    memset(P, 0, sizeof *P);
    P->id = 1;
    struct addrinfo dica, *res = NULL;
    char sp[16]; snprintf(sp, sizeof sp, "%d", porta);
    memset(&dica, 0, sizeof dica);
    dica.ai_family = AF_INET; dica.ai_socktype = SOCK_STREAM;
    if(getaddrinfo(host, sp, &dica, &res) || !res) return 0;
    P->fd = socket(res->ai_family, res->ai_socktype, 0);
    if(P->fd < 0 || connect(P->fd, res->ai_addr, res->ai_addrlen) < 0){ freeaddrinfo(res); return 0; }
    freeaddrinfo(res);
    int um = 1; setsockopt(P->fd, IPPROTO_TCP, TCP_NODELAY, &um, sizeof um);
    char m[512];
    snprintf(m, sizeof m, "{\"id\":%d,\"method\":\"mining.subscribe\",\"params\":[]}\n", P->id++);
    if(!st_envia(P, m)) return 0;
    snprintf(m, sizeof m,
             "{\"id\":%d,\"method\":\"mining.authorize\",\"params\":[\"%s\",\"x\"]}\n", P->id++, user);
    return st_envia(P, m);
}
/* uma linha do pool, se houver. Não bloqueia mais do que o socket bloquear. */
static int st_linha(Pool *P, char *saida, size_t max){
    char *nl;
    while(!(nl = memchr(P->buf, '\n', P->nbuf))){
        if(P->nbuf + 1 >= sizeof P->buf){ P->nbuf = 0; return 0; }
        ssize_t g = read(P->fd, P->buf + P->nbuf, sizeof P->buf - P->nbuf - 1);
        if(g <= 0) return 0;
        P->nbuf += (size_t)g;
    }
    size_t n = (size_t)(nl - P->buf);
    if(n >= max) n = max - 1;
    memcpy(saida, P->buf, n); saida[n] = 0;
    memmove(P->buf, nl + 1, P->nbuf - n - 1);
    P->nbuf -= n + 1;
    return 1;
}
/* Trata o que o pool disser. mining.notify enche o job; subscribe enche o extranonce. */
static void st_trata(Pool *P, const char *l){
    if(strstr(l, "mining.notify")){
        size_t n;
        const char *v;
        if((v = st_param(l, 0, &n)) && n < sizeof P->job_id){ memcpy(P->job_id, v, n); P->job_id[n]=0; }
        if((v = st_param(l, 1, &n)) && n == 64) st_hex(v, n, P->prevhash);
        if((v = st_param(l, 5, &n)) && n == 8){ unsigned char b[4]; st_hex(v,n,b);
            P->versao = ((unsigned)b[0]<<24)|((unsigned)b[1]<<16)|((unsigned)b[2]<<8)|b[3]; }
        if((v = st_param(l, 6, &n)) && n == 8){ unsigned char b[4]; st_hex(v,n,b);
            P->nbits = ((unsigned)b[0]<<24)|((unsigned)b[1]<<16)|((unsigned)b[2]<<8)|b[3]; }
        if((v = st_param(l, 7, &n)) && n == 8){ unsigned char b[4]; st_hex(v,n,b);
            P->ntime = ((unsigned)b[0]<<24)|((unsigned)b[1]<<16)|((unsigned)b[2]<<8)|b[3]; }
        P->tem_job = 1;
    } else if(strstr(l, "\"result\"") && strstr(l, "mining.notify") == NULL && !P->en1_len){
        /* a resposta do subscribe traz o extranonce1 e o tamanho do extranonce2 */
        const char *p = strrchr(l, '"');
        if(p){ const char *ini = p; while(ini > l && *(ini-1) != '"') ini--;
               size_t n = (size_t)(p - ini);
               if(n && n < sizeof P->extranonce1){ memcpy(P->extranonce1, ini, n);
                                                   P->extranonce1[n]=0; P->en1_len=(int)n; } }
    }
}
/* A SHARE de volta: STORE no slot de share vira mining.submit. */
static int st_submete(Pool *P, const char *user, unsigned nonce){
    char m[512];
    snprintf(m, sizeof m,
        "{\"id\":%d,\"method\":\"mining.submit\",\"params\":"
        "[\"%s\",\"%s\",\"00000000\",\"%08x\",\"%08x\"]}\n",
        P->id++, user, P->job_id, P->ntime, nonce);
    return st_envia(P, m);
}
#endif
