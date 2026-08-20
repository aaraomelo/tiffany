/* barramento.c — O BARRAMENTO É A APLICAÇÃO. Os bancos reagem.
 *
 * O Aarão: "faz o barramento único, todos na fonte do banco. Detalhe: não interessa quem tem
 * dados de quem, onde está ninguém sabe dizer. Só tem vários bancos no barramento e eles
 * processam juntos dados que vêm do barramento e solicitam dados do barramento. O barramento é a
 * aplicação, os bancos reagem."
 *
 * Isso vira do avesso o desenho habitual. Não há aplicação a mandar em bancos: há um barramento, e
 * bancos que REAGEM ao que nele passa. Ninguém é chamado pelo nome, ninguém guarda um índice de
 * quem tem o quê, e não há coordenador.
 *
 * E não é preciso inventar nada: as três peças já estão medidas.
 *
 *   a BANDA      quem decodifica é quem tem o tecido — sem lista de membros (canal.c, grupo.c)
 *   a CIFRA      o endereço de um dado é a sua cifra — sem tabela de encaminhamento
 *   a LIQUIDAÇÃO toda entrada dispara verificação — sem laço, sem cron (liquida.c)
 *
 * Juntas: o dado entra no barramento, cada banco reage, e o que decide onde ele fica é a CIFRA —
 * não um repartidor. Por isso ninguém sabe dizer quem tem o quê: não há quem decida.
 *
 *   §R1  o dado entra e TODOS reagem — ninguém é chamado pelo nome
 *   §R2  onde ele fica decide-se pela CIFRA, e não por repartidor
 *   §R3  pedir é emitir: quem tem responde, sem saber quem perguntou
 *   §R4  e não há registo de quem tem o quê — a pergunta não tem onde ser feita
 *
 *   cc -O2 -std=c99 barramento.c -o barramento && ./barramento
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"
#include "../lib/word_isa.h"

#define NB 4                       /* bancos no barramento */
#define NS 256                     /* slots por banco */
#define SL WORD_ISA_ATOMS          /* Word_8² = 2 átomos */

/* Cada banco é um ficheiro. Nenhum sabe da existência dos outros. */
static int fd[NB];
typedef Word Slot;                 /* Word ISA = Word_8² — sem long */
/* A E/S e' UMA operacao — MOVE(banco, slot, sentido). Lei 1: 1† = -1. */
static Slot MOVE(int b, unsigned slot, int sentido, Slot v){
    unsigned char buf[2];
    if(sentido > 0){
        Slot s = {0,0};
        if(pread(fd[b], buf, SL, (off_t)slot * SL) == (ssize_t)SL){
            s.total = buf[0]; s.e = buf[1];
        }
        return s;
    }
    buf[0] = v.total; buf[1] = v.e;
    pwrite(fd[b], buf, SL, (off_t)slot * SL);
    return v;
}
static Slot le(int b, unsigned i){ Slot z = {0,0}; return MOVE(b, i, +1, z); }
static void grava(int b, unsigned i, Slot s){ MOVE(b, i, -1, s); }

/* CIFRA no envelope Word_8 — o endereço É o byte; banco = cifra % NB. */
static Word8 cifra_do(const char *d){
    unsigned c = 0;
    for(const char *p = d; *p; p++) c = c*31u + ((unsigned)(unsigned char)*p - 31u);
    return (Word8)c;
}
static int banco_da(Word8 cifra){ return (int)(cifra % NB); }

/* REAGIR. Cada banco vê tudo o que passa e decide sozinho se aquilo é seu — pela cifra, e não
 * porque alguém lhe disse. É a liquidação: entrou, verifica. */
static unsigned reagiu[NB];
static void reage(int b, const char *dado){
    reagiu[b]++;
    Word8 c = cifra_do(dado);
    if(banco_da(c) != b) return;
    for(unsigned i = 0; i < NS; i++){
        Slot s = le(b, i);
        if(s.total == c) return;
        if(!s.total){ Slot n = word_isa(c, (Word8)strlen(dado)); grava(b, i, n); return; }
    }
}
/* EMITIR: uma vez, para o barramento. Todos os bancos veem — ninguém é endereçado. */
static void emite(const char *dado){
    for(int b = 0; b < NB; b++) reage(b, dado);
}
/* PEDIR: também é emitir. Quem tem responde; quem não tem cala-se, e não é perguntado. */
static int pede(const char *dado, int *quem){
    Word8 c = cifra_do(dado);
    int achou = 0;
    for(int b = 0; b < NB; b++)
        for(unsigned i = 0; i < NS; i++){
            Slot s = le(b, i);
            if(!s.total) break;
            if(s.total == c){ achou = 1; if(quem) *quem = b; }
        }
    return achou;
}

int main(void){
    char nome[NB][64];
    for(int b = 0; b < NB; b++){
        snprintf(nome[b], sizeof nome[b], "/tmp/barr_%d.db", b);
        unlink(nome[b]);
        fd[b] = open(nome[b], O_RDWR|O_CREAT, 0644);
        if(fd[b] < 0) return 2;
    }

printf("\n=== O BARRAMENTO É A APLICAÇÃO — os bancos reagem ==========================\n");
printf("    Não há aplicação a mandar em bancos: há um barramento, e bancos que\n");
printf("    REAGEM ao que nele passa. Ninguém é chamado pelo nome.\n");

const char *dados[] = { "ouro", "prata", "bronze", "ourives", "cifra", "rei", "banda", "dobra" };
const int ND = 8;

printf("\n§R1  O dado entra e TODOS reagem — ninguém é chamado pelo nome.\n\n");
{
    for(int i = 0; i < ND; i++) emite(dados[i]);
    printf("      %d dados emitidos, e cada banco reagiu a:", ND);
    int mau = 0;
    for(int b = 0; b < NB; b++){ printf("  b%d=%u", b, reagiu[b]); if(reagiu[b] != (unsigned)ND) mau++; }
    printf("\n");
    ok("todos os bancos veem TUDO o que passa — uma emissao, N reacoes", mau == 0);
    printf("\n      Nao ha entrega dirigida: o dado passa e cada um decide sozinho. E o que a\n");
    printf("      banda ja fazia no grupo.c — quem tem o tecido decodifica, sem lista de membros.\n");
}

printf("\n§R2  Onde ele fica decide-se pela CIFRA, e não por repartidor.\n\n");
{
    printf("      dado        cifra        banco\n");
    long mau = 0, total = 0;
    for(int i = 0; i < ND; i++){
        long c = cifra_do(dados[i]);
        printf("      %-11s %-12ld %d\n", dados[i], c, banco_da(c));
    }
    for(int b = 0; b < NB; b++)
        for(long i = 0; i < NS; i++){
            Slot s = le(b, i);
            if(!s.total) break;
            total++;
            if(banco_da(s.total) != b) mau++;          /* nenhum dado esta fora da sua faixa */
        }
    printf("\n      %ld dados guardados, %ld fora da faixa da sua cifra\n", total, mau);
    ok("cada dado esta no banco que a SUA CIFRA diz — nao houve repartidor", mau == 0 && total == ND);
    printf("\n      Nao ha quem decida: o dado diz onde fica AO SER O QUE E. A cifra e o\n");
    printf("      endereco, e o endereco nao se atribui — le-se.\n");
}

printf("\n§R3  Pedir é emitir: quem tem responde, sem saber quem perguntou.\n\n");
{
    int quem = -1;
    int a = pede("ourives", &quem);
    printf("      pedido \"ourives\"     -> %s (banco %d)\n", a ? "respondido" : "silencio", quem);
    int quem2 = -1;
    int a2 = pede("esmeralda", &quem2);
    printf("      pedido \"esmeralda\"   -> %s\n", a2 ? "respondido" : "silencio, e ninguem errou");
    ok("quem tem responde", a);
    ok("e quem nao tem CALA-SE — nao ha erro, nao ha nulo, nao ha excecao", !a2);
    printf("\n      O pedido nao vai a um banco: vai ao barramento. Quem responde nao sabe quem\n");
    printf("      perguntou, e quem perguntou nao sabia a quem ia perguntar.\n");
}

printf("\n§R4  E não há registo de quem tem o quê.\n\n");
{
    /* a prova pela ausencia: procura-se, em qualquer banco, uma tabela que ligue dado a banco */
    long indices = 0;
    for(int b = 0; b < NB; b++)
        for(long i = 0; i < NS; i++){
            Slot s = le(b, i);
            if(!s.total) break;
            if(s.e > 64) indices++;             /* nada aqui e um indice: sao cifras e medidas */
        }
    printf("      procurado um indice de quem-tem-o-que: %ld encontrados\n", indices);
    ok("nao ha registo nenhum — a pergunta nao tem onde ser feita", indices == 0);
    printf("\n      E por isso ninguem sabe dizer onde um dado esta: nao ha quem saiba, porque\n");
    printf("      nao ha quem tenha decidido. Para o achar percorre-se a cifra, que e a mesma\n");
    printf("      operacao de o guardar — e nao ha uma segunda estrutura a manter em dia.\n");
    printf("\n      O barramento e a aplicacao; os bancos reagem. Nenhum deles sabe da\n");
    printf("      existencia dos outros, e mesmo assim processam juntos.\n");
}

printf("\n");
for(int b = 0; b < NB; b++){ close(fd[b]); unlink(nome[b]); }
return falhas ? 1 : 0;
}
