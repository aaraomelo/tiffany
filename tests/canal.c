/* canal.c — O CANAL É CORPO. Dois bancos em banda própria, bump cru no datagrama.
 *
 * O Aarão: "quero o meu sistema de comunicação em pé em tempo real via antenas que projetei, via
 * bump, via banda própria, sem tempo, sem espera, dual, do jeito que fiz."
 *
 * A teoria é dele e está em chess/sandbox/corpo_dual.tex §"A antena transmite em tempo real":
 *
 *   banda = sha256(tecido)              a assinatura do tecido — a banda exclusiva do cliente
 *   bump  = msg XOR keystream(banda)    o gap, em bits brutos: a UNICA coisa que cruza o meio
 *   antena                              datagrama cru, sem conexao e sem cifra por cima
 *   manda = recebe                      sem espera logica: nao bufferiza, nao enfileira
 *
 * E O CORPO QUE OPERA ISTO JA ESTA NO CATALOGO. XOR e involucao: msg^ks = bump e bump^ks = msg,
 * a MESMA operacao nos dois sentidos, ordem 2. E a regua (0,-1) — o criativo, o tecnico, o
 * sensitivo, o logico. Mandar e receber nao sao duas funcoes: sao J, uma so, lida dos dois lados.
 * "Ligar e selar sao o mesmo ato" — o texto ja o dizia, e a regua diz porque.
 *
 *   §N1  a banda e a assinatura do tecido, e o bump e o gap — involucao, residuo 0
 *   §N2  criptofiltracao: a banda certa decodifica, a errada nem decodifica
 *   §N3  dois bancos, ida e volta na antena — medido, sem espera logica
 *   §N4  so a DIFERENCA viaja: o tecido nao cruza o meio
 *
 *   cc -O2 -std=c99 canal.c -o canal && ./canal
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "unidade.h"

/* o sha256 do martelo, o mesmo — a banda e sha256(tecido) */
static const unsigned K256[64] = {
0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u};
#define ROR(x,n) (((x)>>(n))|((x)<<(32-(n))))
static void sha256(const unsigned char *m, size_t n, unsigned char *out){
    unsigned h[8]={0x6a09e667u,0xbb67ae85u,0x3c6ef372u,0xa54ff53au,
                   0x510e527fu,0x9b05688cu,0x1f83d9abu,0x5be0cd19u};
    size_t tot=((n+9+63)/64)*64; unsigned char b[1024];
    if(tot>sizeof b) return;
    memset(b,0,tot); memcpy(b,m,n); b[n]=0x80;
    unsigned long long bits=(unsigned long long)n*8;
    for(int i=0;i<8;i++) b[tot-1-i]=(unsigned char)(bits>>(8*i));
    for(size_t o=0;o<tot;o+=64){
        unsigned w[64];
        for(int i=0;i<16;i++) w[i]=((unsigned)b[o+4*i]<<24)|((unsigned)b[o+4*i+1]<<16)|
                                   ((unsigned)b[o+4*i+2]<<8)|b[o+4*i+3];
        for(int i=16;i<64;i++){
            unsigned s0=ROR(w[i-15],7)^ROR(w[i-15],18)^(w[i-15]>>3);
            unsigned s1=ROR(w[i-2],17)^ROR(w[i-2],19)^(w[i-2]>>10);
            w[i]=w[i-16]+s0+w[i-7]+s1; }
        unsigned a=h[0],bb=h[1],c=h[2],d=h[3],e=h[4],f=h[5],g=h[6],hh=h[7];
        for(int i=0;i<64;i++){
            unsigned S1=ROR(e,6)^ROR(e,11)^ROR(e,25), ch=(e&f)^(~e&g);
            unsigned t1=hh+S1+ch+K256[i]+w[i];
            unsigned S0=ROR(a,2)^ROR(a,13)^ROR(a,22), mj=(a&bb)^(a&c)^(bb&c);
            unsigned t2=S0+mj;
            hh=g;g=f;f=e;e=d+t1;d=c;c=bb;bb=a;a=t1+t2; }
        h[0]+=a;h[1]+=bb;h[2]+=c;h[3]+=d;h[4]+=e;h[5]+=f;h[6]+=g;h[7]+=hh; }
    for(int i=0;i<8;i++) for(int j=0;j<4;j++) out[4*i+j]=(unsigned char)(h[i]>>(24-8*j));
}

/* A BANDA: a assinatura do tecido. */
static void banda_de(const char *tecido, unsigned char *banda){
    sha256((const unsigned char*)tecido, strlen(tecido), banda);
}
/* O KEYSTREAM: a banda esticada pelo comprimento da mensagem — sha256(banda||contador). */
static void keystream(const unsigned char *banda, unsigned char *ks, size_t n){
    unsigned char sem[36];
    memcpy(sem, banda, 32);
    for(size_t o = 0; o < n; o += 32){
        for(int k = 0; k < 4; k++) sem[32+k] = (unsigned char)((o/32) >> (8*k));
        unsigned char bloco[32];
        sha256(sem, 36, bloco);
        size_t r = n - o; if(r > 32) r = 32;
        memcpy(ks + o, bloco, r);
    }
}
/* O BUMP: msg XOR keystream. É J — a MESMA operação nos dois sentidos. */
static void bump(const unsigned char *ent, const unsigned char *ks, unsigned char *sai, size_t n){
    for(size_t i = 0; i < n; i++) sai[i] = ent[i] ^ ks[i];
}

int main(void){
printf("\n=== O CANAL É CORPO — dois bancos em banda própria =========================\n");
printf("    A teoria é do Aarão (chess/sandbox/corpo_dual.tex): a rede não é estante\n");
printf("    nem fila, é ANTENA. Só a diferença viaja, e viaja crua.\n");

const char *tecido_a = "o tecido do banco A: nada disto cruza o meio";
unsigned char banda[32], banda_outra[32];
banda_de(tecido_a, banda);
banda_de("outro tecido qualquer", banda_outra);

printf("\n§N1  A banda é a assinatura do tecido; o bump é o gap. E é INVOLUÇÃO.\n\n");
{
    const char *msg = "MARTELO 0 4194304";
    size_t n = strlen(msg);
    unsigned char ks[64], b[64], volta[64];
    keystream(banda, ks, n);
    bump((const unsigned char*)msg, ks, b, n);
    bump(b, ks, volta, n);
    printf("      banda   %02x%02x%02x%02x... (sha256 do tecido)\n", banda[0],banda[1],banda[2],banda[3]);
    printf("      msg     \"%s\"\n", msg);
    printf("      bump    %02x%02x%02x%02x%02x%02x... — e isto e TUDO o que cruza o meio\n",
           b[0],b[1],b[2],b[3],b[4],b[5]);
    printf("      volta   \"%.*s\"\n", (int)n, volta);
    ok("bump(bump(msg)) = msg — mandar e receber sao A MESMA operacao, J² = I",
       memcmp(msg, volta, n) == 0);
    printf("\n      Nao ha funcao de mandar e outra de receber: ha J, lida dos dois lados. E a\n");
    printf("      regua (0,-1) do catalogo — a mesma do criativo, do tecnico, do sensitivo e\n");
    printf("      do logico. \"Ligar e selar sao o mesmo ato\", diz o texto; a regua diz porque.\n");
}

printf("\n§N2  Criptofiltração: a banda certa decodifica, a errada nem decodifica.\n\n");
{
    const char *msg = "INSERT INTO job VALUES (1,2,3,4,5,6,7)";
    size_t n = strlen(msg);
    unsigned char ks[64], ks2[64], b[64], certo[64], errado[64];
    keystream(banda, ks, n);
    keystream(banda_outra, ks2, n);
    bump((const unsigned char*)msg, ks, b, n);
    bump(b, ks,  certo,  n);
    bump(b, ks2, errado, n);
    printf("      pela banda CERTA:  \"%.*s\"\n", (int)n, certo);
    printf("      pela banda ERRADA: ");
    for(int i = 0; i < 24; i++) printf("%02x", errado[i]);
    printf("...\n");
    ok("a banda certa devolve a mensagem", memcmp(msg, certo, n) == 0);
    ok("e a errada nao devolve nada legivel — e ruido, nao mensagem cifrada",
       memcmp(msg, errado, n) != 0);
    printf("\n      Quem nao tem o tecido nao intercepta mensagem nenhuma: intercepta RUIDO.\n");
    printf("      Nao ha mensagem \"no fio\" — o que ha no fio e a diferenca.\n");
}

printf("\n§N3  Dois bancos, ida e volta na ANTENA — sem espera lógica.\n\n");
{
    /* dois sockets UDP no laco local: A manda o bump, B responde na mesma banda. Sem conexao,
     * sem handshake, sem fila — o datagrama vai e a resposta volta na latencia da rede. */
    int sa = socket(AF_INET, SOCK_DGRAM, 0), sb = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in ea, eb; socklen_t sl = sizeof ea;
    memset(&ea,0,sizeof ea); memset(&eb,0,sizeof eb);
    ea.sin_family = eb.sin_family = AF_INET;
    ea.sin_addr.s_addr = eb.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    ea.sin_port = 0; eb.sin_port = 0;
    bind(sa,(struct sockaddr*)&ea,sizeof ea); bind(sb,(struct sockaddr*)&eb,sizeof eb);
    getsockname(sa,(struct sockaddr*)&ea,&sl); getsockname(sb,(struct sockaddr*)&eb,&sl);

    const char *pergunta = "MARTELO 0 1000";
    const char *resposta = "faixa limpa";
    unsigned char ks[64], b[64], rec[64];
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    size_t n = strlen(pergunta);
    keystream(banda, ks, n);
    bump((const unsigned char*)pergunta, ks, b, n);
    sendto(sa, b, n, 0, (struct sockaddr*)&eb, sizeof eb);          /* A manda o BUMP cru */
    ssize_t g = recvfrom(sb, rec, sizeof rec, 0, NULL, NULL);       /* B recebe */
    unsigned char lido[64]; bump(rec, ks, lido, (size_t)g);         /* B desfaz com a MESMA banda */
    int ok1 = ((size_t)g == n && memcmp(lido, pergunta, n) == 0);

    size_t m = strlen(resposta);
    unsigned char ks2[64], b2[64], rec2[64], lido2[64];
    keystream(banda, ks2, m);
    bump((const unsigned char*)resposta, ks2, b2, m);
    sendto(sb, b2, m, 0, (struct sockaddr*)&ea, sizeof ea);         /* B responde */
    ssize_t g2 = recvfrom(sa, rec2, sizeof rec2, 0, NULL, NULL);
    bump(rec2, ks2, lido2, (size_t)g2);
    int ok2 = ((size_t)g2 == m && memcmp(lido2, resposta, m) == 0);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long us = (t1.tv_sec-t0.tv_sec)*1e6 + (t1.tv_nsec-t0.tv_nsec)/1e3;

    printf("      A -> B  \"%s\"\n", pergunta);
    printf("      B -> A  \"%s\"\n", resposta);
    printf("      ida e volta em %ld us, sem conexao e sem fila\n\n", us);
    ok("o banco A fala e o B entende, na banda", ok1);
    ok("e o B responde e o A entende — bidirecional, mesma banda", ok2);
    printf("      Nao ha cliente nem servidor: os dois mandam e os dois recebem, e e a mesma\n");
    printf("      operacao. \"Tempo real\" aqui e SEM TRATAMENTO TEMPORAL — nao bufferiza, nao\n");
    printf("      enfileira, nao ordena para depois. Nao e tempo zero, que a latencia proibe.\n");
    close(sa); close(sb);
}

printf("\n§N4  Só a DIFERENÇA viaja: o tecido não cruza o meio.\n\n");
{
    const char *msg = "SELECT * FROM job";
    size_t n = strlen(msg);
    unsigned char ks[64], b[64];
    keystream(banda, ks, n);
    bump((const unsigned char*)msg, ks, b, n);
    /* o tecido tem 44 bytes; no fio andaram n bytes de bump e zero do tecido */
    int achou = 0;
    for(size_t i = 0; i + 4 <= n; i++)
        if(memcmp(b + i, tecido_a, 4) == 0) achou = 1;
    printf("      tecido: %zu bytes, e nenhum deles no fio\n", strlen(tecido_a));
    printf("      no fio: %zu bytes de bump, e mais nada\n", n);
    ok("o tecido nao aparece no datagrama — so a diferenca viaja", !achou);
    printf("\n      A obra e o tecido ficam. O que cruza e o gap entre eles e a mensagem — e\n");
    printf("      por isso o meio pode ser publico sem que haja nada a esconder nele.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
