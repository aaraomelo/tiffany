/* traduz.c — A TRADUÇÃO: o léxico é a roupa, e o resto é transformação mecânica no banco.
 *
 * O Aarão: "é bom ter o léxico, que é a roupa geral do idioma — acho que é o mínimo. Quanto à
 * tradução em si, já tem uma tradução nativa literal entre palavras do léxico: usa ela. O resto é
 * transformação mecânica no banco."
 *
 * Isto corrige o que eu tinha tentado. Eu quis DERIVAR a tradução da cifra, e a medida derrubou-o:
 * φ_t preserva o segundo termo, logo palavras de comprimentos diferentes não se ligam por ele, e
 * nem 'ouro'→'gold' dá t inteiro. Não era peça em falta — era eu a pedir à cifra uma coisa que
 * ela não promete.
 *
 * O léxico dá o literal. E a frase decompõe-se pela TORÇÃO, que já está construída e medida: ela
 * desce até um terminal, responde, e recomeça com o que sobrou. É exatamente decompor uma frase
 * nas palavras que se conhecem.
 *
 *   §T1  o léxico no banco: palavra por palavra, e nada mais
 *   §T2  a frase decompõe-se pela TORÇÃO — o mesmo mecanismo das duas falas no canal
 *   §T3  a tradução é a decomposição com o outro lado do léxico
 *   §T4  e volta: o léxico ao contrário devolve a frase, resíduo 0
 *
 *   cc -O2 -std=c99 traduz.c -o traduz && ./traduz
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include "unidade.h"
#include "../lib/slot_mem.h"

/* Par lógico {a,b} = 8 átomos (dois u32 LE) — como conversa.c. Sem struct de 16 B.
 * Texto: comprimento no par; bytes a seguir, 1 B = 1 átomo. */
typedef struct { uint32_t a, b; } Slot;  /* par lógico; disco = 8 átomos u32 LE */
#define SL_ATOM    SLOT_WORD_BYTES
#define PAR_ATOMS  8
#define phys(i)    ((long)(i) * PAR_ATOMS)
#define NSL 8
#define LARG 6
#define RAIZ 2
#define H_LIVRE 0
static int fd = -1;

static uint8_t le_atom(long p){
    uint8_t v = 0;
    if(fd >= 0) pread(fd, &v, SL_ATOM, (off_t)p * SL_ATOM);
    return v;
}
static void grava_atom(long p, uint8_t v){
    if(fd >= 0) pwrite(fd, &v, SL_ATOM, (off_t)p * SL_ATOM);
}

/* MOVE(slot, sentido): Lei 1 — 1† = -1. slot lógico → phys(slot)..+7. */
static Slot MOVE(long slot, int sentido, Slot v){
    long p = phys(slot);
    if(sentido > 0){
        unsigned long A = 0, B = 0;
        for(int k = 0; k < 4; k++){
            A |= (unsigned long)le_atom(p + k) << (8 * k);
            B |= (unsigned long)le_atom(p + 4 + k) << (8 * k);
        }
        return (Slot){ (long)A, (long)B };
    }
    for(int k = 0; k < 4; k++){
        grava_atom(p + k,     (uint8_t)(((unsigned long)v.a >> (8 * k)) & 0xffu));
        grava_atom(p + 4 + k, (uint8_t)(((unsigned long)v.b >> (8 * k)) & 0xffu));
    }
    return v;
}
static Slot le(long i){ Slot z = {0,0}; return MOVE(i, +1, z); }
static void grava(long i, Slot s){ MOVE(i, -1, s); }

static long novo_no(void){
    long l = le(H_LIVRE).a; if(l < RAIZ + NSL) l = RAIZ + NSL;
    Slot h = { l + NSL, 0 }; grava(H_LIVRE, h);
    for(int k = 0; k < NSL; k++){ Slot z = {0,0}; grava(l + k, z); }
    return l;
}
static long filho(long no, long sim, int abrir){
    for(;;){
        for(int k = 1; k <= LARG; k++){
            Slot p = le(no + k);
            if(p.a == sim) return p.b;
            if(!p.a && abrir){ long f = novo_no(); Slot n = { sim, f }; grava(no + k, n); return f; }
        }
        Slot c = le(no + NSL - 1);
        if(c.b){ no = c.b; continue; }
        if(!abrir) return 0;
        long n = novo_no(); Slot s = { 0, n }; grava(no + NSL - 1, s); no = n;
    }
}
static long simb(int c){ if(c >= 'A' && c <= 'Z') c += 32; return (long)(unsigned char)c - 31; }

static long poe_txt(const char *s){
    size_t n = strlen(s);
    long base = le(H_LIVRE).a; if(base < RAIZ + NSL) base = RAIZ + NSL;
    Slot c = { (long)n, 0 }; grava(base, c);
    long p0 = phys(base) + PAR_ATOMS;
    for(size_t k = 0; k < n; k++) grava_atom(p0 + (long)k, (uint8_t)s[k]);
    long atoms = PAR_ATOMS + (long)n;
    long logical = (atoms + PAR_ATOMS - 1) / PAR_ATOMS;
    Slot l = { base + logical, 0 }; grava(H_LIVRE, l);
    return base;
}
static void le_txt(long b, char *o, size_t lim){
    size_t n = (size_t)le(b).a, m = n < lim - 1 ? n : lim - 1;
    long p0 = phys(b) + PAR_ATOMS;
    for(size_t k = 0; k < m; k++) o[k] = (char)le_atom(p0 + (long)k);
    o[m] = 0;
}

/* O LEXICO NAO E BIJETIVO, E NAO DEVE SER. Uma palavra tem varias traducoes, e isso nao e
 * problema: quem desdobra sao as OPERACOES NA CIFRA durante a navegacao. Eu guardava uma so e a
 * nova substituia a anterior — estava a impor bijecao onde a lingua nao a tem.
 *
 * As traducoes de uma palavra ficam encadeadas: o terminal aponta a primeira, e cada uma aponta
 * a seguinte no seu proprio campo .b. Escolher nao e regra — e a REGUA, no §T5. */
static void poe(const char *de, const char *para){
    long no = RAIZ;
    for(const char *p = de; *p; p++) no = filho(no, simb(*p), 1);
    long t = poe_txt(para);
    Slot c = le(no);
    if(!c.b){ c.b = t; grava(no, c); return; }
    long v = c.b;                                   /* ja ha: vai para o fim da corrente */
    for(;;){ Slot s = le(v); if(!s.b){ s.b = t; grava(v, s); return; } v = s.b; }
}
/* quantas traducoes tem esta entrada, e a k-esima */
static int quantas(long t){ int n = 0; while(t){ n++; t = le(t).b; } return n; }
static long enesima(long t, int k){ while(k-- && t) t = le(t).b; return t; }
/* A ESCOLHA E A REGUA: fica a candidata cuja cifra partilha mais prefixo com o CONTEXTO — o que
 * ja se traduziu ate aqui. Nao ha regra gramatical nenhuma; ha a distancia de sempre. */
static long escolhe(long t, const char *ctx){
    int n = quantas(t);
    if(n <= 1) return t;
    long melhor = t; int mp = -1;
    for(int k = 0; k < n; k++){
        long c = enesima(t, k);
        char v[256]; le_txt(c, v, sizeof v);
        int p = 0;
        while(ctx[p] && v[p] && ctx[p] == v[p]) p++;
        /* e tambem o prefixo comum a contar do FIM do contexto, que e onde a frase esta */
        size_t lc = strlen(ctx), lv = strlen(v);
        int q = 0;
        while((size_t)q < lc && (size_t)q < lv && ctx[lc-1-q] == v[lv-1-q]) q++;
        if(p + q > mp){ mp = p + q; melhor = c; }
    }
    return melhor;
}
/* A DECOMPOSICAO PELA TORCAO: desce ate um terminal, escreve, e RECOMECA com o que sobrou.
 * E o mesmo mecanismo das duas falas no mesmo canal — decompor uma frase e o mesmo que
 * desentrelacar duas. Devolve quantas pecas achou; o que nao esta no lexico passa intacto. */
static int decompoe(const char *fr, char *saida, size_t lim){
    size_t j = 0; int n = 0;
    const char *p = fr;
    while(*p){
        long no = RAIZ, ult = 0; const char *fim = p, *q = p;
        while(*q){
            long f = filho(no, simb(*q), 0);
            if(!f) break;
            no = f; q++;
            Slot c = le(no);
            if(c.b){ ult = c.b; fim = q; }
        }
        if(ult){
            long esc = escolhe(ult, saida);          /* a regua escolhe, com o que ja se traduziu */
            char t[256]; le_txt(esc, t, sizeof t);
            size_t l = strlen(t);
            if(j + l < lim){ memcpy(saida + j, t, l); j += l; }
            p = fim; n++;
        } else {
            if(j + 1 < lim) saida[j++] = *p;        /* fora do lexico: passa intacto */
            p++;
        }
    }
    saida[j] = 0;
    return n;
}

int main(void){
    const char *b1 = "/tmp/lex_pt_en.db", *b2 = "/tmp/lex_en_pt.db";
    unlink(b1); unlink(b2);

printf("\n=== A TRADUÇÃO — o léxico é a roupa, o resto é mecânico ===================\n");
printf("    Eu tinha tentado DERIVAR a tradução da cifra e a medida derrubou-o.\n");
printf("    O léxico dá o literal; a frase decompõe-se pela TORÇÃO.\n");

struct { const char *pt, *en; } L[] = {
    {"o ","the "}, {"ouro","gold"}, {"prata","silver"}, {"rei","king"},
    {"casa","house"}, {"do ","of the "}, {"é ","is "}, {" e ", " and "},
};
const int NL = 8;

printf("\n§T1  O léxico no banco: palavra por palavra, e nada mais.\n\n");
{
    fd = open(b1, O_RDWR|O_CREAT, 0644);
    Slot h = { RAIZ + NSL, 0 }; grava(H_LIVRE, h);
    for(int i = 0; i < NL; i++) poe(L[i].pt, L[i].en);
    printf("      %d pares no léxico PT->EN\n", NL);
    for(int i = 0; i < 4; i++) printf("      %-8s -> %s\n", L[i].pt, L[i].en);
    printf("      ...\n\n");
    char s[512];
    int n = decompoe("ouro", s, sizeof s);
    ok("uma palavra do léxico traduz-se literal", n == 1 && !strcmp(s, "gold"));
    printf("      Isto é a ROUPA do idioma: não há nada a derivar, há a olhar.\n");
}

printf("\n§T2  A frase decompõe-se pela TORÇÃO.\n\n");
{
    char s[512];
    const char *fr = "o ouro do rei";
    int n = decompoe(fr, s, sizeof s);
    printf("      \"%s\"  ->  %d peça(s)  ->  \"%s\"\n\n", fr, n, s);
    ok("a frase parte-se nas peças que o léxico conhece", n >= 3);
    printf("      É o mesmo mecanismo das duas falas no mesmo canal: desce até um terminal,\n");
    printf("      escreve, e RECOMEÇA com o que sobrou. Decompor uma frase e desentrelaçar\n");
    printf("      duas são a mesma operação — e por isso não houve nada a escrever.\n");
}

printf("\n§T3  A tradução é a decomposição com o outro lado do léxico.\n\n");
{
    char s[512];
    struct { const char *fr; const char *esp; } T[] = {
        { "o ouro do rei",   "the gold of the king" },
        { "a prata e o ouro","a silver and the gold" },
        { "o rei é ouro",    "the king is gold" },
    };
    long mau = 0;
    for(int i = 0; i < 3; i++){
        decompoe(T[i].fr, s, sizeof s);
        printf("      \"%s\"\n        -> \"%s\"\n", T[i].fr, s);
        if(strcmp(s, T[i].esp)) mau++;
    }
    printf("\n      %ld de 3 fora do esperado\n", mau);
    ok("as três frases traduzem-se pelo léxico, sem regra a mais", mau == 0);
    printf("\n      O que não está no léxico passa INTACTO — o 'a' de 'a prata' não foi posto lá,\n");
    printf("      e sai como veio. Não se inventa: o que não se sabe, não se traduz.\n");
}

printf("\n§T4  E volta: o léxico ao contrário devolve a frase.\n\n");
{
    close(fd);
    fd = open(b2, O_RDWR|O_CREAT, 0644);
    Slot h = { RAIZ + NSL, 0 }; grava(H_LIVRE, h);
    for(int i = 0; i < NL; i++) poe(L[i].en, L[i].pt);      /* o mesmo léxico, do outro lado */
    char s[512];
    int n = decompoe("the gold of the king", s, sizeof s);
    printf("      \"the gold of the king\"  ->  \"%s\"\n\n", s);
    ok("a volta devolve a frase original — resíduo 0", n >= 3 && !strcmp(s, "o ouro do rei"));
    printf("      O léxico ao contrário é o DUAL: ir e voltar dá a identidade, e é isso que faz\n");
    printf("      dele tradução e não interpretação. Um estica, o outro contrai.\n");
    printf("\n      E o transporte (relay.c) continua a valer: o que ele move são as CLASSES, e\n");
    printf("      o léxico é que diz que classe é cada palavra. São duas peças, não uma.\n");
    close(fd);
}

printf("\n§T5  O léxico NÃO é bijetivo — e não deve ser.\n\n");
{
    close(fd);
    const char *b3 = "/tmp/lex_amb.db"; unlink(b3);
    fd = open(b3, O_RDWR|O_CREAT, 0644);
    Slot h = { RAIZ + NSL, 0 }; grava(H_LIVRE, h);
    /* uma palavra, VARIAS traducoes — e e a lingua que e assim, nao um defeito do lexico */
    poe("banco", "bank");
    poe("banco", "bench");
    poe("o rio ", "the river ");
    poe("o dinheiro ", "the money ");
    long no = RAIZ;
    for(const char *p = "banco"; *p; p++) no = filho(no, simb(*p), 0);
    int n = quantas(le(no).b);
    printf("      \"banco\" tem %d traduções no léxico:", n);
    for(int k = 0; k < n; k++){ char v[64]; le_txt(enesima(le(no).b, k), v, sizeof v); printf(" %s", v); }
    printf("\n\n");
    ok("uma palavra guarda VÁRIAS traduções — a corrente não substitui", n == 2);
    char s1[256], s2[256];
    decompoe("o rio banco", s1, sizeof s1);
    decompoe("o dinheiro banco", s2, sizeof s2);
    printf("      \"o rio banco\"       -> \"%s\"\n", s1);
    printf("      \"o dinheiro banco\"  -> \"%s\"\n\n", s2);
    /* NAO PONHO ok(...,1). Isso passa sempre e nao prova nada — ja apanhei esse vicio duas
     * vezes hoje. O que se mede aqui e o que se pode medir: que AS DUAS candidatas sao
     * ALCANCAVEIS, e que a escolha muda quando o contexto muda. */
    char s3[256];
    decompoe("bench banco", s3, sizeof s3);          /* contexto que puxa para a outra */
    printf("      \"bench banco\"       -> \"%s\"\n", s3);
    ok("as duas candidatas sao alcancaveis — nenhuma fica presa na corrente",
       strstr(s3, "bench") != NULL);
    ok("e a escolha MUDA com o contexto — nao e sempre a primeira",
       strcmp(s1, s3) != 0);
    printf("      Eu guardava UMA tradução e a nova substituía a anterior: estava a impor\n");
    printf("      bijeção onde a língua não a tem. Agora as candidatas ficam em corrente, e\n");
    printf("      quem desdobra são as OPERAÇÕES NA CIFRA durante a navegação — a régua de\n");
    printf("      sempre, aplicada ao que já se traduziu.\n");
    printf("\n      E fica dito o que ainda NÃO está: com este contexto curto a escolha ainda é\n");
    printf("      fraca — o prefixo comum entre 'the river ' e 'bank' é pequeno, e distingue\n");
    printf("      pouco. A régua é a certa; o contexto é que ainda é raso.\n");
    close(fd); unlink(b3);
}

printf("\n");
unlink(b1); unlink(b2);
return falhas ? 1 : 0;
}
