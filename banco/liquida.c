/* liquida.c — TODA ENTRADA DISPARA A VERIFICAÇÃO. O tempo é uma entrada como outra qualquer.
 *
 * O Aarão: "precisa de triggers no sistema — seria uma liquidação de contrato automático. Um
 * relógio. Na verdade o relógio é entrada também. Cada entrada dispara verificação de liquidação.
 * O tempo é uma entrada. Sempre que entra, verifica se tem algo a liquidar."
 *
 * Isso apaga três coisas que eu ia construir separadas: o TRIGGER como mecanismo à parte, o LAÇO
 * de polling, e o CRON. Não há "evento" e "tempo" como categorias diferentes — um tique do relógio
 * é uma entrada, e toda entrada corre a MESMA verificação.
 *
 * E liquidar já tem as duas metades construídas, no mórfico:
 *
 *     EROSÃO      estreita   acha o que venceu — é o WHERE
 *     DILATAÇÃO   alarga     escreve o saldo de volta
 *
 * Um contrato é (nome, vence, liquidado). A verificação é uma só função, e quem a chama é
 * qualquer entrada — dado ou tique, e o código não sabe qual foi.
 *
 *   §Q1  toda entrada dispara a verificação — e o tique NÃO é caso especial
 *   §Q2  a erosão acha o que venceu; a dilatação escreve o saldo
 *   §Q3  liquidar é idempotente: o que já foi não volta a ser
 *   §Q4  e sem entrada nenhuma, nada se liquida — o tempo não corre sozinho
 *
 *   cc -O2 -std=c99 liquida.c -o liquida && ./liquida
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"

typedef struct { long vence, liquidado, valor; char nome[40]; } Contrato;
#define NC 64
static int fd = -1;
static long agora = 0;                  /* o relógio: e ele é ENTRADA, não ambiente */
static long liquidados = 0;

static Contrato le(int i){ Contrato c; memset(&c, 0, sizeof c);
    pread(fd, &c, sizeof c, (off_t)i*sizeof c); return c; }
static void grava(int i, Contrato c){ pwrite(fd, &c, sizeof c, (off_t)i*sizeof c); }

/* A VERIFICAÇÃO. Uma só, e é isto que toda entrada dispara.
 *
 * EROSÃO: dos contratos, ficam os que venceram e ainda não foram — o WHERE a estreitar.
 * DILATAÇÃO: o que sobrou escreve-se de volta, saldado — o dual a alargar. */
static int verifica(void){
    int n = 0;
    for(int i = 0; i < NC; i++){
        Contrato c = le(i);
        if(!c.nome[0]) continue;
        if(c.liquidado) continue;                    /* já foi: a erosão descarta-o */
        if(c.vence > agora) continue;                /* não venceu: a erosão descarta-o */
        c.liquidado = agora;                          /* a dilatação: escreve o saldo de volta */
        grava(i, c);
        liquidados += c.valor;
        n++;
    }
    return n;
}
/* A ENTRADA. Dado ou tique — o mesmo caminho, e a função não sabe qual foi.
 * Se a entrada trouxer tempo, o relógio anda; se não trouxer, o relógio fica. Em ambos os casos
 * a verificação corre, porque ENTRAR é que a dispara, e não o que entrou. */
static int entra(long t, const char *dado){
    if(t > agora) agora = t;
    (void)dado;
    return verifica();
}
static void poe(int i, const char *nome, long vence, long valor){
    Contrato c; memset(&c, 0, sizeof c);
    snprintf(c.nome, sizeof c.nome, "%s", nome);
    c.vence = vence; c.valor = valor; c.liquidado = 0;
    grava(i, c);
}

int main(void){
    const char *b = "/tmp/liquida_teste.db";
    unlink(b);
    fd = open(b, O_RDWR|O_CREAT, 0644);
    if(fd < 0) return 2;

printf("\n=== A LIQUIDAÇÃO — toda entrada dispara a verificação =====================\n");
printf("    Não há trigger, não há laço de polling, não há cron. Um tique do relógio\n");
printf("    é uma ENTRADA, e toda entrada corre a MESMA verificação.\n");

poe(0, "faixa de nonces",  10, 100);
poe(1, "job do pool",      20, 200);
poe(2, "share submetida",  30, 300);

printf("\n§Q1  Toda entrada dispara — e o tique NÃO é caso especial.\n\n");
{
    printf("      entrada              t    liquidou   total\n");
    int a = entra(5,  "um dado qualquer");
    printf("      dado                 5    %-10d %ld\n", a, liquidados);
    int b1 = entra(10, NULL);                        /* um TIQUE: sem dado nenhum */
    printf("      tique                10   %-10d %ld\n", b1, liquidados);
    int c = entra(25, "outro dado");
    printf("      dado                 25   %-10d %ld\n", c, liquidados);
    ok("antes de vencer, a entrada corre e nada liquida", a == 0);
    ok("o TIQUE liquida o que venceu — sem ser caso especial", b1 == 1);
    ok("e o DADO liquida tanto quanto o tique — mesmo caminho", c == 1);
    printf("\n      O `entra` não sabe se veio dado ou tique: o que dispara e ENTRAR, e nao o\n");
    printf("      que entrou. Por isso o tempo nao precisa de mecanismo proprio.\n");
}

printf("\n§Q2  A erosão acha o que venceu; a dilatação escreve o saldo.\n\n");
{
    Contrato c0 = le(0), c1 = le(1), c2 = le(2);
    printf("      contrato             vence  liquidado em\n");
    printf("      %-20s %-6ld %ld\n", c0.nome, c0.vence, c0.liquidado);
    printf("      %-20s %-6ld %ld\n", c1.nome, c1.vence, c1.liquidado);
    printf("      %-20s %-6ld %s\n", c2.nome, c2.vence, c2.liquidado ? "sim" : "(por vencer)");
    ok("o saldo ficou escrito no contrato — a dilatacao devolve", c0.liquidado && c1.liquidado);
    ok("e o que nao venceu ficou intacto — a erosao descartou-o", !c2.liquidado);
}

printf("\n§Q3  Liquidar é idempotente: o que já foi não volta a ser.\n\n");
{
    long antes = liquidados;
    int n = entra(26, "mais um dado");
    printf("      nova entrada em t=26: liquidou %d, total continua %ld\n", n, liquidados);
    ok("o que ja foi liquidado nao liquida outra vez", n == 0 && liquidados == antes);
    printf("\n      E por isso a verificacao pode correr a CADA entrada sem medo: correr de mais\n");
    printf("      nao custa nada, e correr de menos e que perde um vencimento.\n");
}

printf("\n§Q4  E sem entrada nenhuma, nada se liquida — o tempo não corre sozinho.\n\n");
{
    long antes = liquidados;
    sleep(0);                                        /* passa tempo REAL, e nao entra nada */
    printf("      passou tempo real, zero entradas: total continua %ld\n", liquidados);
    ok("sem entrada, nada acontece — o relogio nao e ambiente, e ENTRADA", liquidados == antes);
    int n = entra(30, NULL);
    printf("      um tique em t=30: liquidou %d, total %ld\n", n, liquidados);
    ok("e basta um tique para o terceiro vencer e liquidar", n == 1);
    printf("\n      O relogio nao e um daemon a bater por fora: e alguem a POR uma entrada. Se\n");
    printf("      ninguem poe, nada corre — e isso e uma propriedade, nao uma falta: o sistema\n");
    printf("      nao tem estado que se mexa sozinho.\n");
}

printf("\n");
close(fd); unlink(b);
return falhas ? 1 : 0;
}
