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
#include "unidade.h"

typedef struct { long a, b; } Slot;
#define SL 16
#define NSL 8
#define LARG 6
#define RAIZ 2
#define H_LIVRE 0
static int fd = -1;
static Slot le(long i){ Slot s = {0,0}; pread(fd, &s, SL, i*SL); return s; }
static void grava(long i, Slot s){ pwrite(fd, &s, SL, i*SL); }
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
    long b = le(H_LIVRE).a; if(b < RAIZ + NSL) b = RAIZ + NSL;
    Slot c = { (long)n, 0 }; grava(b, c);
    size_t ns = (n + SL - 1) / SL;
    for(size_t k = 0; k < ns; k++){
        Slot w; memset(&w, 0, SL);
        size_t r = n - k*SL; if(r > SL) r = SL;
        memcpy(&w, s + k*SL, r); grava(b + 1 + (long)k, w);
    }
    Slot l = { b + 1 + (long)ns, 0 }; grava(H_LIVRE, l);
    return b;
}
static void le_txt(long b, char *o, size_t lim){
    size_t n = (size_t)le(b).a, m = n < lim - 1 ? n : lim - 1;
    for(size_t k = 0; k*SL < m; k++){
        Slot w = le(b + 1 + (long)k);
        size_t r = m - k*SL; if(r > SL) r = SL;
        memcpy(o + k*SL, &w, r);
    }
    o[m] = 0;
}
/* O LEXICO: a palavra desce, e a traducao mora no terminal. E a roupa do idioma, e mais nada. */
static void poe(const char *de, const char *para){
    long no = RAIZ;
    for(const char *p = de; *p; p++) no = filho(no, simb(*p), 1);
    Slot c = le(no); c.b = poe_txt(para); grava(no, c);
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
            char t[256]; le_txt(ult, t, sizeof t);
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

printf("\n");
unlink(b1); unlink(b2);
return falhas ? 1 : 0;
}
