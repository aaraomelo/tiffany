/* conversa.c — A ASSISTENTE. Corpus vazio que cresce do que se conversar, em português.
 *
 * Desenho fixado com o Aarão: a unidade é o PAR (fala, resposta), uma resposta só, e o corpus
 * nasce vazio — ela aprende do que conversarmos.
 *
 * TUDO EM DISCO, NADA EM RAM. A assistente antiga montava vocabulário, postings e órbitas com
 * malloc, e por isso nunca chegou a correr sobre 1,2M de frases. Aqui não há tabela em memória:
 * a fala cifra-se, desce a árvore em pread, e a resposta mora no nó terminal.
 *
 * O NÓ É UM CONJUNTO, NÃO UMA LARGURA. A primeira árvore que escrevi tinha 256 slots por nó —
 * 4 KB para guardar um ou dois filhos — e medi 153 MB em 2000 linhas, que dava 92 GB no corpus
 * todo. O nó guarda agora SÓ OS FILHOS QUE EXISTEM, seis por registo e um encadeado para o resto.
 * É a lição do dia: usar o conjunto, não a largura fixa.
 *
 * As três buscas são as três do mórfico, e cai-se de uma para a outra:
 *
 *   EROSÃO      o prefixo          a fala tal como veio
 *   DILATAÇÃO   a subsequência     a fala com ruído, ou com as palavras trocadas
 *   DECRETO     "não sei"          o único método sem dual — e é ele que se recusa a inventar
 *
 *   ./conversa <base> aprende "a fala" "a resposta"
 *   ./conversa <base> responde "a fala"
 *   ./conversa <base> conversa                 (modo interativo)
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct { long a, b; } Slot;
#define SL 16
static int fd = -1;

static Slot le(long i){ Slot s = {0,0}; pread(fd, &s, SL, i*SL); return s; }
static void grava(long i, Slot s){ pwrite(fd, &s, SL, i*SL); }

/* O cabeçalho: [0] = primeiro slot livre, [1] = quantos pares aprendidos. */
#define H_LIVRE 0
#define H_PARES 1
#define RAIZ    2
#define LARG    6            /* filhos por registo de nó — o resto encadeia */
/* nó = [nfilhos | resposta][s1|f1][s2|f2]...[s6|f6][0|continuação] = 8 slots */
#define NOSL    8

static long novo_no(void){
    long l = le(H_LIVRE).a;
    if(l < RAIZ + NOSL) l = RAIZ + NOSL;
    Slot h = { l + NOSL, 0 }; grava(H_LIVRE, h);
    for(int k = 0; k < NOSL; k++){ Slot z = {0,0}; grava(l + k, z); }
    return l;
}
/* o filho pelo símbolo; abre se abrir != 0 */
static long filho(long no, long sim, int abrir){
    for(;;){
        Slot cab = le(no);
        for(int k = 1; k <= LARG; k++){
            Slot p = le(no + k);
            if(p.a == sim) return p.b;
            if(!p.a && abrir){
                long f = novo_no();
                Slot np = { sim, f }; grava(no + k, np);
                Slot nc = { cab.a + 1, cab.b }; grava(no, nc);
                return f;
            }
        }
        Slot cont = le(no + NOSL - 1);
        if(cont.b){ no = cont.b; continue; }
        if(!abrir) return 0;
        long c = novo_no();
        Slot nc = { 0, c }; grava(no + NOSL - 1, nc);
        no = c;
    }
}
/* O texto guarda-se onde couber, comprimento à frente do registo (não da cifra). */
static long poe_texto(const char *s){
    size_t n = strlen(s);
    long base = le(H_LIVRE).a;
    if(base < RAIZ + NOSL) base = RAIZ + NOSL;
    Slot c = { (long)n, 0 }; grava(base, c);
    size_t ns = (n + SL - 1) / SL;
    for(size_t k = 0; k < ns; k++){
        Slot w; memset(&w, 0, SL);
        size_t r = n - k*SL; if(r > SL) r = SL;
        memcpy(&w, s + k*SL, r);
        grava(base + 1 + (long)k, w);
    }
    Slot l = { base + 1 + (long)ns, 0 }; grava(H_LIVRE, l);
    return base;
}
static void le_texto(long base, char *out, size_t lim){
    size_t n = (size_t)le(base).a, m = n < lim - 1 ? n : lim - 1;
    for(size_t k = 0; k*SL < m; k++){
        Slot w = le(base + 1 + (long)k);
        size_t r = m - k*SL; if(r > SL) r = SL;
        memcpy(out + k*SL, &w, r);
    }
    out[m] = 0;
}
/* O SÍMBOLO, COM O ACENTO ACERTADO.
 *
 * Eu tratava cada byte como símbolo, e em UTF-8 o 'é' são DOIS bytes (0xC3 0xA9) que não se
 * parecem nada com o 'e'. Em português isso manda "és" e "es" para lados opostos da árvore, e
 * numa assistente que aprende de conversa é o caso comum, não a excepção.
 *
 * A acentuação é ROUPA: 'é' e 'e' são a mesma letra vestida. Então o símbolo é a letra NUA, e o
 * acento fica no texto guardado, que é onde ele serve — a resposta sai acentuada, o caminho não
 * se parte. Mesma ideia do resto do dia: o que identifica é a estrutura, não a superfície.
 *
 * Avança o ponteiro pelo símbolo inteiro: uma letra é um passo, tenha um byte ou dois. */
static long prox_simb(const char **p){
    unsigned char c = (unsigned char)*(*p)++;
    if(c == 0xC3 && **p){                          /* os acentuados do português vivem aqui */
        unsigned char d = (unsigned char)*(*p)++;
        if(d >= 0x80 && d <= 0x9F) d += 0x20;      /* maiúscula acentuada -> minúscula */
        if(d >= 0xA0 && d <= 0xA5) c = 'a';
        else if(d == 0xA7)         c = 'c';        /* ç */
        else if(d >= 0xA8 && d <= 0xAB) c = 'e';
        else if(d >= 0xAC && d <= 0xAF) c = 'i';
        else if(d == 0xB1)         c = 'n';        /* ñ */
        else if(d >= 0xB2 && d <= 0xB6) c = 'o';
        else if(d >= 0xB9 && d <= 0xBC) c = 'u';
        else c = 'a';                               /* qualquer outro do bloco: cai na base */
    }
    if(c >= 'A' && c <= 'Z') c += 32;
    return (long)c - 31;
}

static void aprende(const char *fala, const char *resp){
    long no = RAIZ;
    for(const char *p = fala; *p; ) no = filho(no, prox_simb(&p), 1);
    Slot cab = le(no);
    long r = poe_texto(resp);
    Slot nc = { cab.a, r }; grava(no, nc);          /* uma resposta só: a nova substitui */
    Slot pc = le(H_PARES); pc.a++; grava(H_PARES, pc);
    printf("aprendido — %ld par(es) no corpus\n", pc.a);
}
/* EROSÃO: o prefixo. Desce enquanto houver caminho e devolve a resposta mais funda que viu. */
static long erosao(const char *fala, int *fundo){
    long no = RAIZ, achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) break;
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){ achou = cab.b; *fundo = d; }
    }
    return achou;
}
/* DILATAÇÃO: a subsequência. Salta um símbolo quando o caminho morre — a fala com ruído. */
static long dilatacao(const char *fala, int *fundo){
    long no = RAIZ, achou = 0; *fundo = 0;
    int d = 0;
    for(const char *p = fala; *p; ){
        long f = filho(no, prox_simb(&p), 0);
        if(!f) continue;                            /* o símbolo não serve: salta-o */
        no = f; d++;
        Slot cab = le(no);
        if(cab.b){ achou = cab.b; *fundo = d; }
    }
    return achou;
}
static void responde(const char *fala){
    int d = 0;
    long r = erosao(fala, &d);
    const char *via = "erosão (prefixo)";
    if(!r){ r = dilatacao(fala, &d); via = "dilatação (subsequência)"; }
    if(!r){
        printf("não sei.\n");                        /* o DECRETO: sem dual, e não inventa */
        printf("   (nada no corpus alcança esta fala — ensina-me com: aprende)\n");
        return;
    }
    char t[1024]; le_texto(r, t, sizeof t);
    printf("%s\n", t);
    printf("   (%s, %d símbolo(s) de caminho)\n", via, d);
}

int main(int argc, char **argv){
    if(argc < 3){
        fprintf(stderr, "uso: conversa <base> aprende \"fala\" \"resposta\"\n"
                        "     conversa <base> responde \"fala\"\n"
                        "     conversa <base> conversa\n");
        return 2;
    }
    fd = open(argv[1], O_RDWR|O_CREAT, 0644);
    if(fd < 0){ perror("base"); return 2; }
    if(le(H_LIVRE).a < RAIZ + NOSL){ Slot h = { RAIZ + NOSL, 0 }; grava(H_LIVRE, h); }

    if(!strcmp(argv[2], "aprende") && argc >= 5) aprende(argv[3], argv[4]);
    else if(!strcmp(argv[2], "responde") && argc >= 4) responde(argv[3]);
    else if(!strcmp(argv[2], "conversa")){
        printf("corpus com %ld par(es). Escreve a fala; para ensinar: = a resposta\n\n",
               le(H_PARES).a);
        char linha[1024], ultima[1024] = "";
        while(fgets(linha, sizeof linha, stdin)){
            size_t n = strlen(linha);
            while(n && (linha[n-1] == '\n' || linha[n-1] == '\r')) linha[--n] = 0;
            if(!n) continue;
            if(linha[0] == '=' && ultima[0]){ aprende(ultima, linha + 1); continue; }
            strcpy(ultima, linha);
            responde(linha);
        }
    } else { fprintf(stderr, "comando desconhecido\n"); close(fd); return 2; }
    close(fd);
    return 0;
}
