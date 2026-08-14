/* fala.c — Antena local da assistente: protocolo TFAL + cristal por banda.
 *
 *   cada cliente → Assinatura(corpo) → banda=sha256 → base .torre/fala/<hex>/
 *   HELLO em claro; FALA/APRENDE/RESPOSTA em bump
 *   tradutor continua no front; aqui só fala↔corpus (conversa)
 *
 *   cc -O2 -std=c99 -Wall -I../lib -I. fala.c -o fala
 *   ./fala [.torre/fala] [127.0.0.1] [47314]
 *   CONVERSA=./conversa ./fala
 *
 * Cliente de teste:
 *   ./fala cliente <assinatura> "a fala"
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "fala.h"

static const char *RAIZ = ".fala";
static const char *CONVERSA_BIN = "./bin/conversa";
static char HOST[64] = "127.0.0.1";
static int PORT = FALA_PORT;

static int le_exacto(int fd, void *buf, size_t n){
    unsigned char *p = buf; size_t o = 0;
    while(o < n){
        ssize_t r = read(fd, p + o, n - o);
        if(r <= 0) return -1;
        o += (size_t)r;
    }
    return 0;
}
static int escreve_tudo(int fd, const void *buf, size_t n){
    const unsigned char *p = buf; size_t o = 0;
    while(o < n){
        ssize_t w = write(fd, p + o, n - o);
        if(w <= 0) return -1;
        o += (size_t)w;
    }
    return 0;
}

/* Corre conversa e captura stdout (uma linha / bloco). */
static int conversa_cmd(const char *base, const char *cmd,
                        const char *a, const char *b,
                        char *out, size_t outcap){
    int pipefd[2];
    if(pipe(pipefd) < 0) return -1;
    pid_t pid = fork();
    if(pid < 0){ close(pipefd[0]); close(pipefd[1]); return -1; }
    if(pid == 0){
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        dup2(pipefd[1], 2);
        close(pipefd[1]);
        if(!strcmp(cmd, "responde"))
            execl(CONVERSA_BIN, "conversa", base, "responde", a, (char*)0);
        else if(!strcmp(cmd, "aprende"))
            execl(CONVERSA_BIN, "conversa", base, "aprende", a, b, (char*)0);
        _exit(127);
    }
    close(pipefd[1]);
    size_t o = 0;
    while(o + 1 < outcap){
        ssize_t r = read(pipefd[0], out + o, outcap - 1 - o);
        if(r <= 0) break;
        o += (size_t)r;
    }
    out[o] = 0;
    close(pipefd[0]);
    int st = 0; waitpid(pid, &st, 0);
    /* limpa trailing newlines */
    while(o && (out[o-1]=='\n' || out[o-1]=='\r')) out[--o] = 0;
    return (WIFEXITED(st) && WEXITSTATUS(st) == 0) ? 0 : -1;
}

static void garante_base(const char *base){
    char tmp[768];
    snprintf(tmp, sizeof tmp, "%s", base);
    for(char *p = tmp + 1; *p; p++){
        if(*p == '/'){ *p = 0; mkdir(tmp, 0755); *p = '/'; }
    }
    mkdir(base, 0755);
}

static int envia(int fd, unsigned char op, unsigned seq,
                 const unsigned char *banda, const char *txt){
    unsigned char frame[FALA_HDR + FALA_MAX];
    size_t n = txt ? strlen(txt) : 0;
    int m = fala_empacota(frame, sizeof frame, op, seq, banda, txt, n);
    if(m < 0) return -1;
    return escreve_tudo(fd, frame, (size_t)m);
}

static int recebe(int fd, unsigned char *op, unsigned *seq,
                  const unsigned char *banda,
                  char *buf, size_t buflen){
    unsigned char hdr[FALA_HDR];
    if(le_exacto(fd, hdr, FALA_HDR) < 0) return -1;
    if(!fala_magic_ok(hdr) || hdr[4] != FALA_VER) return -2;
    unsigned len = fala_rb32(hdr + 10);
    if(len > FALA_MAX) return -3;
    unsigned char pack[FALA_HDR + FALA_MAX];
    memcpy(pack, hdr, FALA_HDR);
    if(len && le_exacto(fd, pack + FALA_HDR, len) < 0) return -1;
    size_t nout = 0;
    return fala_desempacota(pack, FALA_HDR + len, op, seq, banda, buf, buflen, &nout);
}

static void sessao(int fd){
    unsigned char banda[32];
    int tem_banda = 0;
    char assinatura[512];
    char base[640];
    char texto[FALA_MAX + 1];
    unsigned char op; unsigned seq;

    for(;;){
        const unsigned char *buse = tem_banda ? banda : NULL;
        if(recebe(fd, &op, &seq, buse, texto, sizeof texto) < 0) break;

        if(op == FALA_HELLO){
            if(!texto[0] || strlen(texto) >= sizeof assinatura){
                envia(fd, FALA_ERR, seq, NULL, "HELLO sem Assinatura");
                continue;
            }
            snprintf(assinatura, sizeof assinatura, "%s", texto);
            fala_banda_de_assinatura(assinatura, banda);
            tem_banda = 1;
            char hex[33]; fala_hex16(banda, hex);
            snprintf(base, sizeof base, "%s/%s", RAIZ, hex);
            garante_base(base);
            char okmsg[128];
            snprintf(okmsg, sizeof okmsg, "banda=%s", hex);
            envia(fd, FALA_RESPOSTA, seq, banda, okmsg);
            fprintf(stderr, "fala: HELLO assinatura→banda %s\n", hex);
            continue;
        }

        if(!tem_banda){
            envia(fd, FALA_ERR, seq, NULL, "precisa HELLO primeiro");
            continue;
        }

        if(op == FALA_FALA){
            char resp[FALA_MAX];
            if(conversa_cmd(base, "responde", texto, NULL, resp, sizeof resp) < 0){
                /* conversa pode não existir: decreto honesto */
                envia(fd, FALA_NAO_SEI, seq, banda, "nao sei");
                continue;
            }
            if(strstr(resp, "nao sei") || strstr(resp, "não sei") ||
               strstr(resp, "NAO SEI") || strstr(resp, "nada no corpus")){
                envia(fd, FALA_NAO_SEI, seq, banda, resp[0] ? resp : "nao sei");
            } else {
                envia(fd, FALA_RESPOSTA, seq, banda, resp[0] ? resp : "(vazio)");
            }
            continue;
        }

        if(op == FALA_APRENDE){
            /* payload: fala\\0resposta  ou  fala\\tresposta */
            char *sep = strchr(texto, '\t');
            if(!sep) sep = strstr(texto, "\n=\n");
            if(!sep){ envia(fd, FALA_ERR, seq, banda, "APRENDE quer fala\\tresposta"); continue; }
            if(*sep == '\t'){ *sep = 0; sep++; }
            else { *sep = 0; sep += 3; }
            char sink[256];
            conversa_cmd(base, "aprende", texto, sep, sink, sizeof sink);
            envia(fd, FALA_RESPOSTA, seq, banda, "aprendido");
            continue;
        }

        envia(fd, FALA_ERR, seq, banda, "op desconhecida");
    }
}

static int servidor(void){
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){ perror("socket"); return 1; }
    int um = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &um, sizeof um);
    struct sockaddr_in a; memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)PORT);
    if(inet_pton(AF_INET, HOST, &a.sin_addr) != 1){ fprintf(stderr, "host\n"); return 1; }
    if(bind(s, (struct sockaddr*)&a, sizeof a) < 0){ perror("bind"); return 1; }
    if(listen(s, 8) < 0){ perror("listen"); return 1; }
    fprintf(stderr, "fala: antena %s:%d  raiz=%s  conversa=%s\n",
            HOST, PORT, RAIZ, CONVERSA_BIN);
    for(;;){
        int c = accept(s, NULL, NULL);
        if(c < 0){ if(errno == EINTR) continue; perror("accept"); break; }
        pid_t p = fork();
        if(p == 0){ close(s); sessao(c); close(c); _exit(0); }
        close(c);
        if(p > 0){ while(waitpid(-1, NULL, WNOHANG) > 0); }
    }
    close(s);
    return 0;
}

/* Cliente CLI de fumo: HELLO + FALA. */
static int cliente(const char *assinatura, const char *fala){
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s < 0){ perror("socket"); return 1; }
    struct sockaddr_in a; memset(&a, 0, sizeof a);
    a.sin_family = AF_INET;
    a.sin_port = htons((unsigned short)PORT);
    inet_pton(AF_INET, HOST, &a.sin_addr);
    if(connect(s, (struct sockaddr*)&a, sizeof a) < 0){ perror("connect"); return 1; }

    unsigned char banda[32];
    fala_banda_de_assinatura(assinatura, banda);
    unsigned char op; unsigned seq = 1;
    char buf[FALA_MAX];

    if(envia(s, FALA_HELLO, seq, NULL, assinatura) < 0) return 1;
    if(recebe(s, &op, &seq, banda, buf, sizeof buf) < 0) return 1;
    printf("HELLO → %u %s\n", op, buf);

    seq = 2;
    if(envia(s, FALA_FALA, seq, banda, fala) < 0) return 1;
    if(recebe(s, &op, &seq, banda, buf, sizeof buf) < 0) return 1;
    printf("FALA  → op=%u\n%s\n", op, buf);
    close(s);
    return 0;
}

/* -teste: a VOLTA do protocolo, offline e auto-terminante — o daemon não é
 * medidor, mas o seu protocolo é: empacota∘desempacota = id, o bump é a
 * involução (J: ida = volta), e a mutação de um byte acusa. É este modo que
 * a bateria corre (tools/bateria.sh args: fala → -teste). */
static int auto_teste(void){
    int falhas = 0, feitas = 0;
    unsigned char banda[32];
    fala_banda_de_assinatura("assinatura-de-teste", banda);
#define OKT(q, cond) do{ feitas++; int c_ = (cond); if(!c_) falhas++; \
    printf("#UNIT %s %s\n", c_ ? "ok" : "falha", q); }while(0)

    /* §A1 — empacota → desempacota = id, nas quatro ops (HELLO em claro) */
    {
        const unsigned char ops[4] = { FALA_HELLO, FALA_FALA, FALA_APRENDE, FALA_RESPOSTA };
        const char *txt = "a fala é a interface — não há API de grafo";
        int todos = 1;
        for(int i = 0; i < 4; i++){
            unsigned char frame[FALA_HDR + FALA_MAX];
            char volta[FALA_MAX + 1];
            unsigned char op = 0; unsigned seq = 0; size_t nout = 0;
            int m = fala_empacota(frame, sizeof frame, ops[i], 7u + (unsigned)i,
                                  banda, txt, strlen(txt));
            if(m < 0){ todos = 0; continue; }
            int r = fala_desempacota(frame, (size_t)m, &op, &seq, banda,
                                     volta, sizeof volta, &nout);
            if(r < 0 || op != ops[i] || seq != 7u + (unsigned)i ||
               nout != strlen(txt) || memcmp(volta, txt, nout) != 0) todos = 0;
        }
        OKT("§A1 empacota∘desempacota = id nas 4 ops (HELLO claro; resto em bump)", todos);
    }

    /* §A2 — o bump é involução: dois bumps devolvem o claro (J, ida = volta) */
    {
        unsigned char ks[FALA_MAX];
        keystream(banda, ks, 64);
        unsigned char claro[64], uma[64], duas[64];
        for(int i = 0; i < 64; i++) claro[i] = (unsigned char)(i * 7 + 1);
        bump(claro, ks, uma, 64);
        bump(uma, ks, duas, 64);
        OKT("§A2 bump∘bump = id — a involução do keystream (Lei 1 no canal)",
            memcmp(claro, duas, 64) == 0 && memcmp(claro, uma, 64) != 0);
    }

    /* §A3 — a mutação acusa: magic ferido recusa; corpo ferido difere */
    {
        unsigned char frame[FALA_HDR + FALA_MAX];
        char volta[FALA_MAX + 1];
        unsigned char op = 0; unsigned seq = 0; size_t nout = 0;
        const char *txt = "o corpo em trânsito";
        int m = fala_empacota(frame, sizeof frame, FALA_FALA, 9u, banda,
                              txt, strlen(txt));
        frame[0] ^= 1;   /* fere o magic */
        int r1 = fala_desempacota(frame, (size_t)m, &op, &seq, banda,
                                  volta, sizeof volta, &nout);
        frame[0] ^= 1;   /* repara; fere o corpo */
        frame[FALA_HDR] ^= 1;
        int r2 = fala_desempacota(frame, (size_t)m, &op, &seq, banda,
                                  volta, sizeof volta, &nout);
        OKT("§A3 mutação: magic ferido recusa (-2); corpo ferido difere do claro",
            r1 == -2 && r2 == 0 && memcmp(volta, txt, strlen(txt)) != 0);
    }

    /* §A4 — bandas distintas não se leem: o bump da banda errada dá lixo */
    {
        unsigned char outra[32];
        fala_banda_de_assinatura("outra-assinatura", outra);
        unsigned char frame[FALA_HDR + FALA_MAX];
        char volta[FALA_MAX + 1];
        unsigned char op = 0; unsigned seq = 0; size_t nout = 0;
        const char *txt = "cada cliente na sua banda";
        int m = fala_empacota(frame, sizeof frame, FALA_FALA, 11u, banda,
                              txt, strlen(txt));
        fala_desempacota(frame, (size_t)m, &op, &seq, outra,
                         volta, sizeof volta, &nout);
        OKT("§A4 a banda errada não lê: o texto não volta (isolamento por sha256)",
            nout == strlen(txt) && memcmp(volta, txt, nout) != 0);
    }

    printf("\nunidades: %d   falhas: %d\n", feitas, falhas);
    return falhas ? 1 : 0;
#undef OKT
}

int main(int argc, char **argv){
    const char *env;
    if((env = getenv("FALA_RAIZ"))) RAIZ = env;
    if((env = getenv("CONVERSA"))) CONVERSA_BIN = env;
    if((env = getenv("FALA_HOST"))) snprintf(HOST, sizeof HOST, "%s", env);
    if((env = getenv("FALA_PORT"))) PORT = atoi(env);

    if(argc >= 2 && !strcmp(argv[1], "-teste")) return auto_teste();

    if(argc >= 2 && !strcmp(argv[1], "cliente")){
        if(argc < 4){
            fprintf(stderr, "uso: %s cliente <Assinatura> \"fala\"\n", argv[0]);
            return 2;
        }
        return cliente(argv[2], argv[3]);
    }
    if(argc >= 2 && argv[1][0] != '-') RAIZ = argv[1];
    if(argc >= 3) snprintf(HOST, sizeof HOST, "%s", argv[2]);
    if(argc >= 4) PORT = atoi(argv[3]);
    return servidor();
}
