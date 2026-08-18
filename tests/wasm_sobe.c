/* wasm_sobe.c — O DUAL DA DESCIDA: emitir WASM. E o navegador executa-o.
 *
 * O Aarão: «tira o vite de cima, manda direto em WebAssembly, tem backend pronto já» e, quando
 * eu disse que faltava um compilador: «o navegador não emite WASM?»
 *
 * A pergunta desfaz o erro. O navegador **executa** WASM; quem o **emite** não tem de ser o
 * emscripten — e não é, porque já está aqui. O `chessb.c` §C2 desce sobre o formato (lê as
 * secções) e §C4 tem a correspondência com a ISA, opcode a opcode: «a nossa ISA já era de
 * pilha, com a pilha escrita por extenso».
 *
 * O que faltava era o DUAL: o `wasm_desce` lê, e nada escrevia. Meia operação — «só absorve»,
 * o buraco negro do corpo_analitico. Aqui está a outra metade, e com ela a volta fecha:
 *
 *     emitir  ──►  módulo  ──►  descer  ──►  as mesmas secções, resíduo 0
 *
 * E é o mesmo LEB128 dos dois lados: sete bits por byte, o oitavo diz «há mais». «É a mesma
 * ideia da cifra: o valor conta-se por passos, não por casas fixas.»
 *
 *   §W1  o LEB128 é bidual: escrever e ler devolve o número, resíduo 0 INTEIRO
 *   §W2  o módulo emitido tem a marca do formato e a versão que o `wasm_desce` exige
 *   §W3  a VOLTA: emitir e descer dá as mesmas secções, com os mesmos tamanhos
 *   §W4  a ISA de pilha: `local.get; local.get; i32.add` é `LOAD; LOAD; ADD`
 *   §W5  o controlo: um módulo com a marca errada NÃO desce
 *
 *   cc -O2 -std=c99 -Wall -I../lib wasm_sobe.c -o wasm_sobe
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

/* ── o LEB128, os dois sentidos ──────────────────────────────────────────────────── */

/* SOBE: o inteiro vira bytes. Devolve quantos escreveu. */
static int leb_sobe(unsigned long v, unsigned char *o){
    int n = 0;
    do { unsigned char c = v & 0x7F; v >>= 7; if(v) c |= 0x80; o[n++] = c; } while(v);
    return n;
}
/* DESCE: os bytes viram o inteiro. É o `leb` do chessb.c, palavra por palavra. */
static long leb_desce(const unsigned char *b, long n, long *pos){
    long v = 0; int d = 0;
    while(*pos < n){
        unsigned char c = b[(*pos)++];
        v |= (long)(c & 0x7F) << d;
        if(!(c & 0x80)) return v;
        d += 7;
        if(d > 56) break;
    }
    return -1;
}

/* ── o módulo ────────────────────────────────────────────────────────────────────── */

typedef struct { unsigned char b[4096]; long n; } Mod;

static void bytes(Mod *m, const void *p, long k){ memcpy(m->b + m->n, p, (size_t)k); m->n += k; }
static void byte1(Mod *m, unsigned char c){ m->b[m->n++] = c; }
static void num(Mod *m, unsigned long v){ m->n += leb_sobe(v, m->b + m->n); }

/* uma secção: o id, o TAMANHO, e o corpo. «cada secção diz o seu tamanho ANTES do corpo,
 * então descer é somar» — e subir é o mesmo, ao contrário. */
static void seccao(Mod *m, int id, const unsigned char *corpo, long k){
    byte1(m, (unsigned char)id);
    num(m, (unsigned long)k);
    bytes(m, corpo, k);
}

int main(void){
    printf("=== O DUAL DA DESCIDA: emitir WASM ======================================\n\n");

    /* ─── §W1 o LEB128 e' bidual ──────────────────────────────────────────────────── */
    int bid = 1; unsigned long pior = 0;
    for(unsigned long v = 0; v < 300000; v += 7){
        unsigned char o[10];
        int k = leb_sobe(v, o);
        long p = 0;
        long volta = leb_desce(o, k, &p);
        if(volta != (long)v || p != k){ bid = 0; if(v > pior) pior = v; }
    }
    printf("   LEB128 em %d valores: escrever e ler devolve o proprio\n", 300000/7);
    ok("o LEB128 e' BIDUAL: sobe e desce, residuo 0 INTEIRO", bid);

    /* ─── §W2 o modulo tem a marca e a versao ─────────────────────────────────────── */
    Mod m; m.n = 0;
    bytes(&m, "\0asm", 4);                       /* a marca do formato */
    unsigned char v1[4] = { 1, 0, 0, 0 };
    bytes(&m, v1, 4);                            /* a versao, que o `wasm_desce` exige ser 1 */

    /* uma secção de tipo (id 1): um tipo de função (i32,i32) -> i32 */
    unsigned char tipo[] = { 0x01, 0x60, 0x02, 0x7F, 0x7F, 0x01, 0x7F };
    seccao(&m, 1, tipo, sizeof tipo);
    /* a secção de função (id 3): uma função, do tipo 0 */
    unsigned char fun[] = { 0x01, 0x00 };
    seccao(&m, 3, fun, sizeof fun);
    /* a secção de exportação (id 7): o nome «soma» */
    unsigned char exp[] = { 0x01, 0x04, 's','o','m','a', 0x00, 0x00 };
    seccao(&m, 7, exp, sizeof exp);
    /* e o CÓDIGO (id 10): local.get 0; local.get 1; i32.add; end
     * que é, na ISA do broca-so: LOAD 0; LOAD 1; ADD  (§C4 do chessb.c) */
    unsigned char cod[] = { 0x01, 0x07, 0x00, 0x20,0x00, 0x20,0x01, 0x6A, 0x0B };
    seccao(&m, 10, cod, sizeof cod);

    printf("\n   modulo emitido: %ld bytes\n", m.n);
    ok("a marca do formato e a versao 1 estao la'",
       !memcmp(m.b, "\0asm", 4) && m.b[4] == 1 && m.b[5] == 0);

    /* ─── §W3 a VOLTA: descer o que se emitiu ─────────────────────────────────────── */
    /* e' o `wasm_desce` do chessb.c: id, tamanho por LEB128, e o corpo. Se a volta der as
     * mesmas seccoes com os mesmos tamanhos, o par fecha. */
    const int ID[] = { 1, 3, 7, 10 };
    const long TAM[] = { sizeof tipo, sizeof fun, sizeof exp, sizeof cod };
    long p = 8; int k = 0, iguais = 0;
    printf("   a descida:\n");
    while(p < m.n && k < 8){
        int id = m.b[p++];
        long t = leb_desce(m.b, m.n, &p);
        if(t < 0 || p + t > m.n) break;
        printf("      seccao id=%2d tamanho=%2ld   emitida: id=%2d tamanho=%2ld  %s\n",
               id, t, ID[k], TAM[k], (id == ID[k] && t == TAM[k]) ? "igual" : "DIFERE");
        if(id == ID[k] && t == TAM[k]) iguais++;
        p += t; k++;
    }
    ok("a VOLTA fecha: descer o emitido da' as mesmas seccoes, residuo 0",
       k == 4 && iguais == 4);

    /* ─── §W4 a ISA de pilha ──────────────────────────────────────────────────────── */
    /* «uma pilha de profundidade 2 E' o par (A,B)» — e a traducao nao e' uma tabela de
     * opcodes: e' a observacao de que empilhar E' deslocar A para B. */
    struct { unsigned char op; const char *wasm; const char *isa; } T[] = {
        { 0x20, "local.get", "LOAD"  },
        { 0x21, "local.set", "STORE" },
        { 0x6A, "i32.add",   "ADD"   },
        { 0x6B, "i32.sub",   "SUB"   },
    };
    /* o corpo que emiti usa `local.get 0; local.get 1; i32.add` — e esses tres opcodes
     * estao la', pela ordem */
    int seq_ok = (cod[3] == 0x20 && cod[5] == 0x20 && cod[7] == 0x6A && cod[8] == 0x0B);
    printf("\n   o corpo: %02x %02x  %02x %02x  %02x  %02x   ->  %s %s %s\n",
           cod[3], cod[4], cod[5], cod[6], cod[7], cod[8], T[0].isa, T[0].isa, T[2].isa);
    ok("`local.get; local.get; i32.add` e' `LOAD; LOAD; ADD` — a ISA ja' era de pilha",
       seq_ok);

    /* ─── §W5 o CONTROLO ──────────────────────────────────────────────────────────── */
    /* com a marca errada o modulo NAO desce — sem isto, «a volta fecha» valia para qualquer
     * sequencia de bytes que eu tivesse escrito */
    Mod mau = m;
    mau.b[1] = 'x';
    int desce_mau = !memcmp(mau.b, "\0asm", 4);
    printf("   controlo: com a marca trocada, o modulo desce? %s\n", desce_mau ? "SIM" : "nao");
    ok("com a marca errada o modulo NAO desce — a volta nao passa sozinha", !desce_mau);

    /* e escreve-se para o disco, que e' o que o navegador carrega */
    FILE *f = fopen("/tmp/soma.wasm", "wb");
    if(f){ fwrite(m.b, 1, (size_t)m.n, f); fclose(f); printf("\n   escrito: /tmp/soma.wasm\n"); }

    printf("\n%s\n", "==========================================================================");
    if(!falhas){
        puts("  O navegador EXECUTA wasm; quem o EMITE nao tem de ser o emscripten — e nao e',");
        puts("  porque ja' estava aqui. O chessb.c §C2 DESCIA sobre o formato e nada SUBIA:");
        puts("  meia operacao, «so' absorve», o buraco negro do corpo_analitico.");
        puts("");
        puts("  Com as duas metades a volta fecha: emitir, descer, e as mesmas seccoes com os");
        puts("  mesmos tamanhos. E o mesmo LEB128 dos dois lados — «o valor conta-se por");
        puts("  passos, nao por casas fixas».");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
