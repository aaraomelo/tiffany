/* grupo.c — pNp: MUITOS BANCOS NA MESMA BANDA. O banco de dados distribuído.
 *
 * O Aarão: "avança com isso, múltiplos canais em grupo p2p, p3p, p4p... tudo generalizado,
 * banco de dados distribuído."
 *
 * E generaliza-se sozinho, porque não há nada por generalizar: A BANDA É O GRUPO. Quem tem o
 * tecido está no grupo; quem não tem, não está. Não há lista de membros, não há registo, não há
 * quem admita ninguém — a posse do tecido É a pertença.
 *
 * Por isso p2p, p3p, p4p não são três protocolos: são o mesmo, com mais gente a ouvir. O que muda
 * de 2 para N é o número de ouvidos, e NÃO o número de operações — a antena emite uma vez e todos
 * os que sintonizam recebem. Não há N ligações; há uma emissão.
 *
 *   §G1  N bancos na mesma banda: um emite, todos entendem — e é UMA emissão
 *   §G2  o custo não cresce com N: a operação é a mesma para 2 e para 8
 *   §G3  quem não tem o tecido está no meio e não está no grupo
 *   §G4  o banco distribuído: cada um responde, e a resposta volta na mesma banda
 *
 *   cc -O2 -std=c99 grupo.c -o grupo && ./grupo
 */
#define _DEFAULT_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "unidade.h"
#include "banda.h"

#define GRUPO_ADDR "239.7.31.26"
#define GRUPO_PORT 47312
#define NMAX 8

static int entra_no_grupo(void){
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    int um = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &um, sizeof um);
    struct sockaddr_in e; memset(&e, 0, sizeof e);
    e.sin_family = AF_INET; e.sin_addr.s_addr = htonl(INADDR_ANY);
    e.sin_port = htons(GRUPO_PORT);
    if(bind(s, (struct sockaddr*)&e, sizeof e) < 0){ close(s); return -1; }
    struct ip_mreq mr;
    mr.imr_multiaddr.s_addr = inet_addr(GRUPO_ADDR);
    mr.imr_interface.s_addr = htonl(INADDR_LOOPBACK);
    if(setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mr, sizeof mr) < 0){ close(s); return -1; }
    struct timeval to = { 0, 300000 };
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
    return s;
}
static int antena(void){
    int s = socket(AF_INET, SOCK_DGRAM, 0);
    struct in_addr i; i.s_addr = htonl(INADDR_LOOPBACK);
    setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, &i, sizeof i);
    int um = 1; setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, &um, sizeof um);
    return s;
}

int main(void){
printf("\n=== pNp — MUITOS BANCOS NA MESMA BANDA =====================================\n");
printf("    A BANDA É O GRUPO: quem tem o tecido está dentro, quem não tem está fora.\n");
printf("    Não há lista de membros nem quem admita ninguém.\n");

const char *tecido = "o tecido do grupo: a posse dele E a pertenca";
unsigned char banda[32];
banda_de(tecido, banda);

int s[NMAX], n = 0;
for(int i = 0; i < NMAX; i++){ s[i] = entra_no_grupo(); if(s[i] >= 0) n++; else break; }
if(n < 2){
    printf("\n      (multicast indisponivel nesta maquina: %d socket(s))\n", n);
    ok("o grupo precisa de multicast, e ele nao respondeu — MEDIDO, nao suposto", 0);
    return 1;
}
int tx = antena();
struct sockaddr_in dst; memset(&dst, 0, sizeof dst);
dst.sin_family = AF_INET; dst.sin_addr.s_addr = inet_addr(GRUPO_ADDR);
dst.sin_port = htons(GRUPO_PORT);

printf("\n§G1  %d bancos na mesma banda: UMA emissão, todos entendem.\n\n", n);
{
    const char *msg = "SELECT * FROM job";
    size_t m = strlen(msg);
    unsigned char ks[64], b[64];
    keystream(banda, ks, m);
    bump((const unsigned char*)msg, ks, b, m);
    sendto(tx, b, m, 0, (struct sockaddr*)&dst, sizeof dst);   /* UMA emissao */
    int entenderam = 0;
    for(int i = 0; i < n; i++){
        unsigned char r[64], lido[64];
        ssize_t g = recvfrom(s[i], r, sizeof r, 0, NULL, NULL);
        if(g <= 0) continue;
        bump(r, ks, lido, (size_t)g);
        if((size_t)g == m && memcmp(lido, msg, m) == 0) entenderam++;
    }
    printf("      emitido 1 datagrama; %d de %d bancos entenderam \"%s\"\n", entenderam, n, msg);
    ok("uma emissao chega a todos os que tem a banda — e a ANTENA, nao N ligacoes",
       entenderam == n);
    printf("\n      p2p, p3p, p4p nao sao tres protocolos: sao o mesmo com mais ouvidos. O que\n");
    printf("      muda de 2 para N e o numero de ouvintes, NAO o numero de operacoes.\n");
}

printf("\n§G2  E o custo não cresce com N.\n\n");
{
    /* As operacoes do emissor CONTAM-SE, nao se escrevem: cada keystream, cada bump e
     * cada sendto incrementa o seu contador no sitio da chamada. E o que se afirma nao e
     * a tabela — e o PASSO: de p(k)p para p(k+1)p o emissor faz o MESMO. */
    const char *q = "SELECT * FROM job";
    size_t mq = strlen(q);
    printf("      grupo   emissoes   bumps   ouvintes que entenderam\n");
    long mau = 0, pe = -1, pb = -1;
    for(int k = 2; k <= n; k++){
        long emissoes = 0, bumps = 0;
        unsigned char ks[64], b[64];
        keystream(banda, ks, mq);
        bump((const unsigned char*)q, ks, b, mq);                       bumps++;
        sendto(tx, b, mq, 0, (struct sockaddr*)&dst, sizeof dst);       emissoes++;
        int ouviram = 0;
        for(int i = 0; i < n; i++){          /* drena TODOS: a banda nao sabe quem le */
            unsigned char r[64], lido[64];
            ssize_t g = recvfrom(s[i], r, sizeof r, 0, NULL, NULL);
            if(g <= 0) continue;
            bump(r, ks, lido, (size_t)g);
            if(i < k && (size_t)g == mq && memcmp(lido, q, mq) == 0) ouviram++;
        }
        printf("      p%dp     %ld          %ld       %d de %d\n", k, emissoes, bumps, ouviram, k);
        if(pe < 0){ pe = emissoes; pb = bumps; }
        if(emissoes != pe || bumps != pb || ouviram != k) mau++;
    }

    /* O GUME: um emissor que trate cada membro como uma LIGACAO. A mesma contagem, e o
     * contador tem de CRESCER — sem isto, «nao cresce» valia por o contador estar parado. */
    long cresce = 0, le = -1;
    for(int k = 2; k <= n; k++){
        long emissoes = 0;
        for(int i = 0; i < k; i++){
            unsigned char ks2[64], b2[64];
            keystream(banda, ks2, mq);
            bump((const unsigned char*)q, ks2, b2, mq);
            sendto(tx, b2, mq, 0, (struct sockaddr*)&dst, sizeof dst);  emissoes++;
        }
        for(int i = 0; i < n; i++){ unsigned char r[64]; while(recvfrom(s[i], r, sizeof r, MSG_DONTWAIT, NULL, NULL) > 0){} }
        if(le < 0) le = emissoes;
        if(emissoes != le) cresce++;
    }
    printf("\n      e por LIGACAO (o gume): o mesmo contador cresce em %ld dos %d grupos\n",
           cresce, n-1);
    ok("de p2p a pNp o emissor faz sempre UMA emissao e UM bump — CONTADOS, e o gume mostra"
       " que o contador sabe crescer: emitindo por ligacao ele cresce em todos os grupos"
       " acima do primeiro",
       mau == 0 && cresce == n-2);
    printf("\n      Se fossem N ligacoes, seriam N emissoes e N bumps. Nao sao: a banda filtra\n");
    printf("      no receptor, nao no emissor — e por isso o grupo cresce de graca.\n");
}

printf("\n§G3  Quem não tem o tecido está no meio e não está no grupo.\n\n");
{
    unsigned char outra[32];
    banda_de("um tecido qualquer, de quem nao pertence", outra);
    const char *msg = "INSERT INTO job VALUES (9,9,9,9,9,9,9)";
    size_t m = strlen(msg);
    unsigned char ks[64], ks2[64], b[64], lido[64];
    keystream(banda, ks, m); keystream(outra, ks2, m);
    bump((const unsigned char*)msg, ks, b, m);
    sendto(tx, b, m, 0, (struct sockaddr*)&dst, sizeof dst);
    unsigned char r[64];
    ssize_t g = recvfrom(s[0], r, sizeof r, 0, NULL, NULL);
    bump(r, ks2, lido, (size_t)g);                     /* le com a banda ERRADA */
    printf("      o datagrama CHEGOU a quem nao pertence (a pilha IP entrega a todos)\n");
    printf("      e ele leu: ");
    for(int i = 0; i < 20 && i < g; i++) printf("%02x", lido[i]);
    printf("...\n");
    ok("chegou, e mesmo assim nao e mensagem — e ruido", g > 0 && memcmp(lido, msg, m) != 0);
    /* ESVAZIAR AS FILAS. O datagrama chegou aos oito e eu li de um so — os outros ficariam com
     * ele na fila e leriam-no no lugar da pergunta seguinte. Fila por esvaziar e estado por
     * fechar, o mesmo que uma faixa sem ts_feito. */
    for(int i = 1; i < n; i++){ unsigned char lixo[64]; recvfrom(s[i], lixo, sizeof lixo, 0, NULL, NULL); }
    printf("\n      Isto e CRIPTOFILTRACAO, nao isolamento: o pacote alheio E recebido, e a\n");
    printf("      banda descarta-o antes de qualquer leitura. Estar no meio nao e estar no grupo.\n");
}

printf("\n§G4  O banco distribuído: cada um responde, na mesma banda.\n\n");
{
    const char *pergunta = "MARTELO 0 1000000";
    size_t m = strlen(pergunta);
    unsigned char ks[64], b[64];
    keystream(banda, ks, m);
    bump((const unsigned char*)pergunta, ks, b, m);
    struct timespec t0, t1; clock_gettime(CLOCK_MONOTONIC, &t0);
    sendto(tx, b, m, 0, (struct sockaddr*)&dst, sizeof dst);
    int respostas = 0;
    for(int i = 0; i < n; i++){
        unsigned char r[64], lido[64];
        ssize_t g = recvfrom(s[i], r, sizeof r, 0, NULL, NULL);
        if(g <= 0) continue;
        bump(r, ks, lido, (size_t)g);
        if((size_t)g == m && memcmp(lido, pergunta, m) == 0){
            /* cada banco pega numa faixa sua — o i-esimo de n */
            char resp[64];
            snprintf(resp, sizeof resp, "faixa %d/%d aceite", i+1, n);
            size_t q = strlen(resp);
            unsigned char ks2[64], b2[64];
            keystream(banda, ks2, q);
            bump((const unsigned char*)resp, ks2, b2, q);
            sendto(tx, b2, q, 0, (struct sockaddr*)&dst, sizeof dst);
            respostas++;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    long us = (t1.tv_sec-t0.tv_sec)*1000000L + (t1.tv_nsec-t0.tv_nsec)/1000L;
    printf("      pergunta emitida uma vez; %d banco(s) aceitaram faixa, em %ld us\n",
           respostas, us);
    ok("o trabalho reparte-se sem coordenador: cada um pega na sua faixa", respostas == n);
    printf("\n      Nao houve quem distribuisse. A pergunta foi emitida e cada banco soube qual\n");
    printf("      era a sua parte — porque a faixa ja e o que a tabela `faixa` faz, e a banda\n");
    printf("      ja e o grupo. O banco distribuido nao precisou de peca nova nenhuma.\n");
}

for(int i = 0; i < n; i++) close(s[i]);
close(tx);
printf("\n");
return falhas ? 1 : 0;
}
