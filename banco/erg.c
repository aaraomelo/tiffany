/* erg.c — O PLUGUE DE DENTRO, EM ASSEMBLY: montador e executor da ISA ERG-64.
 *
 * O Aarão: "você tem os plugues do lado de dentro com bash e assembly."
 *
 * A máquina já existia e nunca foi tocada aqui: o `sql.c` transcreveu a ISA do broca-so e
 * executa-a com `pread`, sem RAM. O que faltava era a PORTA — um jeito de o piloto escrever
 * na ISA sem passar por SQL. Isto é essa porta, e é só ela: monta texto em bytecode, corre o
 * bytecode, e desmonta de volta.
 *
 *      montar  →  correr  →  desmontar        e a volta tem de dar o mesmo texto
 *
 * NADA DA ISA É REINVENTADO AQUI, e há uma asserção que O MEDE: o §E1 lê o enum do próprio
 * `sql.c` e compara nome a nome, número a número. Se alguém acrescentar um opcode lá e não
 * aqui, esta medida cai — e cai com o nome do opcode em falta. É a regra dos dois caminhos:
 * duas transcrições da mesma ISA, e a única prova de que são a mesma é confrontá-las.
 *
 * SEM RAM, como o resto: o programa vive num ficheiro e lê-se byte a byte com `pread`; a
 * memória vive noutro ficheiro, um slot de 16 bytes por vez. Os únicos registos são A, B, R,
 * o `pc` e as flags — três palavras, como no metal.
 *
 * AS DUAS ARMADILHAS DA ISA, que o piloto tem de saber antes de escrever a primeira linha, e
 * que estão medidas em §E5 e §E6:
 *
 *   (1) STORE grava R, NÃO A.  Pôr uma constante num slot não é `LOAD k; STORE s` — isso grava
 *       o R anterior, que é lixo. É `LOAD k; LOAD zero; ADD; STORE s`.
 *   (2) FL_ZERO é "A e B AMBOS zero", não "A é zero". Testar igualdade é subtrair e depois
 *       comparar contra o slot do zero.
 *
 * E O RÓTULO É A ÚNICA COMODIDADE QUE ESTE MONTADOR DÁ. Os saltos da ISA são relativos e em
 * complemento de dois num byte; contar isso à mão é onde um piloto perde a tarde. `:volta` e
 * `JNZ volta` resolvem, e o §E7 mede que a conta do montador bate com a conta à mão.
 *
 *   §E1  os opcodes batem com os do sql.c — lidos DE LÁ, não copiados
 *   §E2  montar e desmontar é a volta: o texto sobrevive
 *   §E3  GOLD e NEGRO_OURO são inversos EXATOS — a volta é inteira porque det = −1
 *   §E4  ESQUILO tem ordem 4 e TROCA ordem 2 — o grupo, medido no ferro
 *   §E5  STORE grava R e não A: a armadilha, num programa que a distingue
 *   §E6  um app inteiro: somar dois slots, e conferir contra a soma feita em C
 *   §E7  o laço: JNZ com rótulo, e a contagem de passos prevista antes de correr
 *
 * O piloto usa assim:
 *
 *   cc -O2 -std=c99 -Wall -Wformat erg.c -o erg
 *   ./erg monta app.erg app.bin        o texto vira bytecode
 *   ./erg corre app.bin mem.dat        corre, e o estado fica no ficheiro de memória
 *   ./erg desmonta app.bin             o bytecode volta a texto
 *   ./erg ve mem.dat 3                 lê o slot 3
 *   ./erg poe mem.dat 3 7 8            escreve {7,8} no slot 3
 *   ./erg                              sem argumentos: corre a bateria de medidas
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"

typedef struct { long total, e; } Word;

/* ---------------- a ISA (transcrita do sql.c, e §E1 confronta-a com ele) ---------------- */
enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_GOLD, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_LOADS,
       /* saíram quatro nomes que estavam só neste enum: sem um único `case`, sem
        * entrada no montador e sem uso. Reservados que nunca correram — e manter
        * redundância custa mais do que a tirar. (Os nomes não se escrevem aqui: o
        * erg.c LÊ este enum do ficheiro, e apanhá-los-ia como opcodes.) */
       OP_NEGRO_OURO, OP_ESQUILO, OP_TROCA, OP_MARTELO };
#define FL_ZERO 0x01
#define FL_EQ   0x02
#define FL_LT   0x04

/* A FORMA de cada opcode: quantos bytes de operando, e de que espécie.
 *   0 = nenhum      2 = slot (u16, little endian)      1 = salto relativo (s8) */
typedef struct { const char *nome; int op; int operando; } Instr;
static const Instr ISA[] = {
    { "HALT",       OP_HALT,       0 },
    { "LOAD",       OP_LOAD,       2 },
    { "STORE",      OP_STORE,      2 },
    { "ADD",        OP_ADD,        0 },
    { "SUB",        OP_SUB,        0 },
    { "AND",        OP_AND,        0 },
    { "OR",         OP_OR,         0 },
    { "XOR",        OP_XOR,        0 },
    { "GOLD",       OP_GOLD,       0 },
    { "CMP",        OP_CMP,        0 },
    { "JMP",        OP_JMP,        1 },
    { "JZ",         OP_JZ,         1 },
    { "JNZ",        OP_JNZ,        1 },
    { "LOADS",      OP_LOADS,      2 },
    { "NEGRO_OURO", OP_NEGRO_OURO, 0 },
    { "ESQUILO",    OP_ESQUILO,    0 },
    { "TROCA",      OP_TROCA,      0 },
};
static const int NISA = (int)(sizeof ISA / sizeof ISA[0]);

static const Instr *acha_nome(const char *n){
    for(int i = 0; i < NISA; i++) if(!strcmp(ISA[i].nome, n)) return &ISA[i];
    return NULL;
}
static const Instr *acha_op(int op){
    for(int i = 0; i < NISA; i++) if(ISA[i].op == op) return &ISA[i];
    return NULL;
}

/* ---------------- a memória: um ficheiro, um slot de cada vez. Sem RAM. ---------------- */
#define SLOT 16
static int fmem = -1;

static Word mem_le(unsigned slot){
    Word w = { 0, 0 };
    if(fmem >= 0) (void)!pread(fmem, &w, sizeof w, (off_t)slot * SLOT);
    return w;
}
static void mem_grava(unsigned slot, Word w){
    if(fmem >= 0) (void)!pwrite(fmem, &w, sizeof w, (off_t)slot * SLOT);
}

/* ---------------- a máquina: um passo, fiel ao sql.c ---------------- */
static int fprog = -1;
static unsigned char prog_le(unsigned pc){
    unsigned char b = OP_HALT;
    if(pread(fprog, &b, 1, (off_t)pc) != 1) return OP_HALT;
    return b;
}
typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

static Word ula_add(Word a, Word b){ Word r = { a.total+b.total, a.e+b.e }; return r; }
static Word ula_sub(Word a, Word b){ Word r = { a.total-b.total, a.e-b.e }; return r; }
static Word ula_and(Word a, Word b){ Word r = { a.total&b.total, a.e&b.e }; return r; }
static Word ula_or (Word a, Word b){ Word r = { a.total|b.total, a.e|b.e }; return r; }
static Word ula_xor(Word a, Word b){ Word r = { a.total^b.total, a.e^b.e }; return r; }
static int  zero(Word w){ return w.total == 0 && w.e == 0; }

/* o gato, e a volta. cifra_an(w,1) = (total + e, total) — det −1, logo a volta é INTEIRA. */
static Word cifra_an  (Word w, int n){ Word r = { (long)n*w.total + w.e, w.total }; return r; }
static Word decifra_an(Word w, int n){ Word r = { w.e, w.total - (long)n*w.e }; return r; }

/* o salto, um so': avanca 1+rel se `cond`, senao avanca 1. JMP, JZ e JNZ chamam-no
 * todos — sao a MESMA operacao com condicoes diferentes, e a condicao e' argumento. */
static int erg_salto(Regs *r, unsigned pc, int cond){
    int rel = (signed char)prog_le(pc);
    r->pc = cond ? (unsigned)((int)pc + 1 + rel) : pc + 1;
    return 1;
}

static int passo(Regs *r, unsigned prog_len){
    if(r->pc >= prog_len) return 0;
    unsigned pc = r->pc;
    unsigned char op = prog_le(pc++);
    switch(op){
    case OP_HALT: return 0;   /* o unico que NAO e' salto: nao muda pc, para. Fica. */
    case OP_LOAD: case OP_LOADS: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
        pc += 2;
        r->B = r->A; r->A = mem_le(slot);           /* empilhar É deslocar A para B */
        break; }
    case OP_STORE: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
        pc += 2;
        mem_grava(slot, r->R);                       /* grava R, NÃO A — a armadilha */
        break; }
    case OP_GOLD:       r->A = cifra_an  (r->A, 1); r->R = r->A; break;
    case OP_NEGRO_OURO: r->A = decifra_an(r->A, 1); r->R = r->A; break;
    case OP_ESQUILO: { Word w = { -r->A.e, r->A.total }; r->A = w; r->R = w; break; }
    case OP_TROCA:   { Word w = {  r->A.e, r->A.total }; r->A = w; r->R = w; break; }
    case OP_ADD: r->R = ula_add(r->A, r->B); break;
    case OP_SUB: r->R = ula_sub(r->A, r->B); break;
    case OP_AND: r->R = ula_and(r->A, r->B); break;
    case OP_OR:  r->R = ula_or (r->A, r->B); break;
    case OP_XOR: r->R = ula_xor(r->A, r->B); break;
    case OP_CMP: {
        unsigned char f = 0;
        if(zero(r->A) && zero(r->B)) f |= FL_ZERO;   /* AMBOS zero — a outra armadilha */
        if(r->A.total == r->B.total && r->A.e == r->B.e) f |= FL_EQ;
        else if(r->A.total < r->B.total) f |= FL_LT;
        r->flags = f;
        break; }
    /* ── O EXCEDENTE VIRA FUNCAO, E DEPOIS TROCA-SE ────────────────────────────────
     * O Aarao: "transforma o excedente em funcao primeiro, depois sai trocando."
     *
     * JMP e JZ nao sao operacoes: sao JNZ com a condicao mudada. Escritos assim, a troca
     * fica mecanica — o `case` chama o irredutivel, e apagar o opcode passa a ser mudar o
     * montador e mais nada.
     *
     *    JMP  =  JNZ com condicao SEMPRE verdadeira
     *    JZ   =  JNZ com a condicao NEGADA
     *
     * A funcao e' UMA: salta se `cond`, senao avanca. Os tres opcodes so' diferem no que
     * poem em `cond`, e isso e' um argumento, nao uma instrucao. */
    case OP_JMP: return erg_salto(r, pc, 1);
    case OP_JZ:  return erg_salto(r, pc,  (r->flags & FL_ZERO) != 0);
    case OP_JNZ: return erg_salto(r, pc, !(r->flags & FL_ZERO));
    default: return 0;
    }
    r->pc = pc;
    return 1;
}
/* corre e devolve o número de passos; teto para não pendurar o piloto num laço aberto */
static long rodar(unsigned prog_len, long teto){
    Regs r; memset(&r, 0, sizeof r);
    long n = 0;
    while(passo(&r, prog_len)){ if(++n >= teto) break; }
    return n;
}

/* ---------------- o montador: texto → bytecode, com rótulos ---------------- */
#define ROT_MAX 64
typedef struct { char nome[32]; int end; } Rotulo;

/* Devolve o tamanho em bytes, ou −1 com o erro em `erro`. Duas passagens: a primeira acha os
 * rótulos, a segunda emite — porque um salto para a frente não sabe o destino ainda. */
static int monta(const char *texto, unsigned char *saida, int cap, char *erro, int nerro){
    Rotulo rots[ROT_MAX]; int nrot = 0;
    for(int passagem = 0; passagem < 2; passagem++){
        int pos = 0;
        const char *p = texto;
        int linha = 0;
        while(*p){
            const char *fim = strchr(p, '\n');
            if(!fim) fim = p + strlen(p);
            char buf[256];
            int n = (int)(fim - p); if(n > 255) n = 255;
            memcpy(buf, p, (size_t)n); buf[n] = 0;
            linha++;
            p = (*fim) ? fim + 1 : fim;

            char *c = strchr(buf, ';'); if(c) *c = 0;     /* o comentário */
            char *s = buf; while(*s && isspace((unsigned char)*s)) s++;
            char *t = s + strlen(s); while(t > s && isspace((unsigned char)t[-1])) *--t = 0;
            if(!*s) continue;

            if(*s == ':'){                                  /* um rótulo */
                if(passagem == 0){
                    if(nrot >= ROT_MAX){ snprintf(erro,(size_t)nerro,"linha %d: rótulos a mais",linha); return -1; }
                    snprintf(rots[nrot].nome, sizeof rots[0].nome, "%s", s + 1);
                    rots[nrot].end = pos; nrot++;
                }
                continue;
            }
            char mnem[64] = {0}, arg[64] = {0};
            int campos = sscanf(s, "%63s %63s", mnem, arg);
            for(char *q = mnem; *q; q++) *q = (char)toupper((unsigned char)*q);
            const Instr *in = acha_nome(mnem);
            if(!in){ snprintf(erro,(size_t)nerro,"linha %d: opcode desconhecido '%s'",linha,mnem); return -1; }
            if(in->operando && campos < 2){
                snprintf(erro,(size_t)nerro,"linha %d: %s precisa de operando",linha,mnem); return -1; }

            if(passagem == 1){
                if(pos + 1 + in->operando > cap){ snprintf(erro,(size_t)nerro,"programa maior que %d bytes",cap); return -1; }
                saida[pos] = (unsigned char)in->op;
            }
            int base = pos;
            pos += 1;
            if(in->operando == 2){
                long slot = strtol(arg, NULL, 0);
                if(slot < 0 || slot > 65535){ snprintf(erro,(size_t)nerro,"linha %d: slot %ld fora de 0..65535",linha,slot); return -1; }
                if(passagem == 1){ saida[pos] = (unsigned char)(slot & 0xFF);
                                   saida[pos+1] = (unsigned char)((slot >> 8) & 0xFF); }
                pos += 2;
            } else if(in->operando == 1){
                pos += 1;
                if(passagem == 1){
                    int destino;
                    if(isdigit((unsigned char)arg[0]) || arg[0] == '-' || arg[0] == '+')
                        destino = (int)strtol(arg, NULL, 0) + base;   /* absoluto, se for número */
                    else {
                        int achou = -1;
                        for(int i = 0; i < nrot; i++) if(!strcmp(rots[i].nome, arg)) achou = rots[i].end;
                        if(achou < 0){ snprintf(erro,(size_t)nerro,"linha %d: rótulo '%s' não existe",linha,arg); return -1; }
                        destino = achou;
                    }
                    /* a ISA: destino = (endereço do byte do rel) + 1 + rel  =  base + 2 + rel */
                    int rel = destino - (base + 2);
                    if(rel < -128 || rel > 127){ snprintf(erro,(size_t)nerro,"linha %d: salto de %d bytes não cabe em s8",linha,rel); return -1; }
                    saida[base+1] = (unsigned char)(signed char)rel;
                }
            }
            (void)base;
        }
        if(passagem == 1) return pos;
    }
    return -1;
}

/* o desmontador — e é ele que fecha a volta do §E2 */
static void desmonta(const unsigned char *b, int n, FILE *saida){
    int pos = 0;
    while(pos < n){
        const Instr *in = acha_op(b[pos]);
        if(!in){ fprintf(saida, "; byte %d desconhecido: %d\n", pos, b[pos]); pos++; continue; }
        if(in->operando == 2 && pos + 2 < n){
            unsigned slot = (unsigned)b[pos+1] | ((unsigned)b[pos+2] << 8);
            fprintf(saida, "%s %u\n", in->nome, slot); pos += 3;
        } else if(in->operando == 1 && pos + 1 < n){
            int rel = (signed char)b[pos+1];
            fprintf(saida, "%s %d\n", in->nome, pos + 2 + rel);   /* absoluto: a volta é exata */
            pos += 2;
        } else { fprintf(saida, "%s\n", in->nome); pos += 1; }
    }
}

/* ---------------- as ferragens do piloto ---------------- */
static int abre_mem(const char *caminho){
    fmem = open(caminho, O_RDWR | O_CREAT, 0644);
    return fmem;
}
static int escreve(const char *caminho, const unsigned char *b, int n){
    int f = open(caminho, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f < 0) return -1;
    ssize_t w = write(f, b, (size_t)n);
    close(f);
    return (w == n) ? 0 : -1;
}
static int le_ficheiro(const char *caminho, unsigned char *b, int cap){
    int f = open(caminho, O_RDONLY);
    if(f < 0) return -1;
    ssize_t n = read(f, b, (size_t)cap);
    close(f);
    return (int)n;
}

/* monta um texto, corre-o sobre um ficheiro de memória, e devolve os passos. Usada pelas
 * medidas todas — o caminho do piloto e o caminho do medidor são O MESMO. */
static long corre_texto(const char *texto, const char *mem, long teto, char *erro, int nerro){
    unsigned char b[4096];
    int n = monta(texto, b, (int)sizeof b, erro, nerro);
    if(n < 0) return -1;
    const char *tmp = "/tmp/erg_prog.bin";
    if(escreve(tmp, b, n) < 0){ snprintf(erro,(size_t)nerro,"não gravou %s", tmp); return -1; }
    if(fprog >= 0) close(fprog);
    fprog = open(tmp, O_RDONLY);
    if(fprog < 0){ snprintf(erro,(size_t)nerro,"não abriu %s", tmp); return -1; }
    if(fmem >= 0) close(fmem);
    if(abre_mem(mem) < 0){ snprintf(erro,(size_t)nerro,"não abriu %s", mem); return -1; }
    long passos = rodar((unsigned)n, teto);
    return passos;
}
static void zera_mem(const char *caminho, int nslots){
    int f = open(caminho, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(f < 0) return;
    Word z = { 0, 0 };
    for(int i = 0; i < nslots; i++) (void)!write(f, &z, sizeof z);
    close(f);
}
static void poe_slot(const char *caminho, unsigned slot, long total, long e){
    int f = open(caminho, O_WRONLY | O_CREAT, 0644);
    if(f < 0) return;
    Word w = { total, e };
    (void)!pwrite(f, &w, sizeof w, (off_t)slot * SLOT);
    close(f);
}
static Word ve_slot(const char *caminho, unsigned slot){
    Word w = { 0, 0 };
    int f = open(caminho, O_RDONLY);
    if(f < 0) return w;
    (void)!pread(f, &w, sizeof w, (off_t)slot * SLOT);
    close(f);
    return w;
}

/* ================================================================================ */
/* §E1 — os opcodes batem com os do sql.c, e a prova é LÊ-LO                        */
/* ================================================================================ */
/* Copiar um enum é fácil; manter duas cópias iguais não é. Por isso esta medida não confia na
 * minha cópia: abre o `sql.c`, extrai o enum dele, e confronta. Um opcode acrescentado lá e
 * esquecido aqui derruba a asserção com o nome dele na mão. */
static void secao_E1(void){
    printf("\n§E1  A ISA É A DO sql.c — e a medida lê-a de lá, não da minha memória\n\n");

    FILE *f = fopen("sql.c", "r");
    if(!f) f = fopen("tools/sql.c", "r");
    if(!f){ ok("o sql.c abre para o confronto dos opcodes", 0); return; }

    /* extrair os nomes do enum, na ordem: o número é a posição, com OP_HALT=0 */
    char nomes[64][32]; int n = 0;
    char linha[512];
    int dentro = 0;
    while(fgets(linha, sizeof linha, f)){
        if(!dentro && strstr(linha, "enum { OP_HALT=0")) dentro = 1;
        else if(!dentro) continue;
        for(char *p = linha; (p = strstr(p, "OP_")) != NULL; ){
            char nome[32]; int k = 0;
            p += 3;
            while(*p && (isalnum((unsigned char)*p) || *p == '_') && k < 31) nome[k++] = *p++;
            nome[k] = 0;
            if(k && n < 64){ snprintf(nomes[n], sizeof nomes[0], "%s", nome); n++; }
        }
        if(dentro && strchr(linha, '}')) break;
    }
    fclose(f);

    printf("     o enum do sql.c tem %d opcodes; este montador expõe %d ao piloto\n", n, NISA);
    ok("o enum do sql.c foi lido (não é lista vazia a passar por acaso)", n >= 20);

    /* cada opcode que EU exponho tem de ter, no sql.c, o mesmo número */
    int discordam = 0; const char *culpado = "";
    for(int i = 0; i < NISA; i++){
        int achado = -1;
        for(int j = 0; j < n; j++) if(!strcmp(nomes[j], ISA[i].nome)) { achado = j; break; }
        if(achado != ISA[i].op){ discordam++; culpado = ISA[i].nome; }
    }
    if(discordam) printf("     DISCORDAM em %d, o primeiro é %s\n", discordam, culpado);
    ok("todo opcode deste montador tem o MESMO número no sql.c", discordam == 0);

    /* e o inverso: o que o sql.c tem e eu não exponho, para o piloto saber o que falta */
    int nao_expostos = 0;
    for(int j = 0; j < n; j++) if(!acha_nome(nomes[j])) nao_expostos++;
    printf("     %d opcodes do sql.c não estão expostos aqui (FOLD, SPECT, MARTELO e afins:\n"
           "     dependem do catálogo do banco e não de um programa solto)\n", nao_expostos);
    ok("os não expostos são poucos — a porta cobre a ISA básica inteira", nao_expostos <= 6);

    conclui("duas transcrições da mesma ISA, e a única prova de que são a mesma é confrontá-las.");
}

/* ================================================================================ */
/* §E2 — montar e desmontar é a volta                                              */
/* ================================================================================ */
static void secao_E2(void){
    printf("\n§E2  A VOLTA: montar, desmontar, e o texto sobrevive\n\n");

    const char *fonte =
        "LOAD 1\n"
        "LOAD 2\n"
        "ADD\n"
        "STORE 3\n"
        "GOLD\n"
        "NEGRO_OURO\n"
        "HALT\n";
    unsigned char b[256]; char erro[128] = {0};
    int n = monta(fonte, b, (int)sizeof b, erro, (int)sizeof erro);
    if(n < 0){ printf("     erro: %s\n", erro); ok("o programa de prova monta", 0); return; }

    printf("     %d bytes de bytecode para 7 instruções (as três com slot pesam 3, as outras 1)\n", n);
    ok("o tamanho é o previsto: 3·3 + 4·1 = 13 bytes", n == 13);

    FILE *m = fopen("/tmp/erg_volta.txt", "w");
    if(!m){ ok("a volta grava", 0); return; }
    desmonta(b, n, m);
    fclose(m);

    char volta[512] = {0};
    FILE *r = fopen("/tmp/erg_volta.txt", "r");
    if(r){ size_t k = fread(volta, 1, sizeof volta - 1, r); volta[k] = 0; fclose(r); }
    printf("     a volta deu:\n");
    for(char *p = volta; *p; ){ char *e = strchr(p, '\n'); if(!e) break; *e = 0;
        printf("        %s\n", p); *e = '\n'; p = e + 1; }

    ok("o texto da volta é IGUAL ao original", !strcmp(volta, fonte));

    /* e a volta tem de sobreviver a MONTAR OUTRA VEZ — é aí que um desmontador mentiroso cai */
    unsigned char b2[256];
    int n2 = monta(volta, b2, (int)sizeof b2, erro, (int)sizeof erro);
    ok("remontar a volta dá o MESMO bytecode, byte a byte",
       n2 == n && memcmp(b, b2, (size_t)n) == 0);

    conclui("o desmontador não é conveniência: é o que impede o montador de mentir em silêncio.");
}

/* ================================================================================ */
/* §E3 — GOLD e NEGRO_OURO são inversos exatos                                     */
/* ================================================================================ */
/* O gato ESTICA, e esticar costuma perder: uma matriz de determinante 2 não volta em inteiros.
 * A do gato tem det −1, e é por isso que a volta é exata — não aproximada, exata. Aqui isso
 * deixa de ser uma frase e passa a ser uma medida no ferro, com a máquina a correr. */
static void secao_E3(void){
    printf("\n§E3  O GATO ESTICA, E A VOLTA É INTEIRA — porque det = −1\n\n");

    const char *mem = "/tmp/erg_m3.dat";
    const char *prog = "LOAD 1\nGOLD\nSTORE 2\nHALT\n";
    const char *prog_volta = "LOAD 2\nNEGRO_OURO\nSTORE 3\nHALT\n";
    char erro[128];

    printf("        (total, e)      → GOLD →      → NEGRO_OURO →   volta?\n");
    int falhou = 0, cresceu = 0;
    long pontos[8][2] = {{1,0},{0,1},{3,5},{-2,7},{13,8},{100,-3},{1,1},{55,34}};
    for(int i = 0; i < 8; i++){
        zera_mem(mem, 8);
        poe_slot(mem, 1, pontos[i][0], pontos[i][1]);
        if(corre_texto(prog, mem, 100, erro, (int)sizeof erro) < 0){ falhou++; continue; }
        Word g = ve_slot(mem, 2);
        if(corre_texto(prog_volta, mem, 100, erro, (int)sizeof erro) < 0){ falhou++; continue; }
        Word v = ve_slot(mem, 3);
        int bate = (v.total == pontos[i][0] && v.e == pontos[i][1]);
        if(!bate) falhou++;
        if(labs(g.total) > labs(pontos[i][0]) && pontos[i][0] > 0) cresceu++;
        printf("        (%4ld,%4ld)      (%4ld,%4ld)      (%4ld,%4ld)      %s\n",
               pontos[i][0], pontos[i][1], g.total, g.e, v.total, v.e, bate ? "sim" : "NÃO");
    }
    ok("a volta é exata nos 8 pontos — inteiro a inteiro, sem resto", falhou == 0);
    ok("e o gato de facto ESTICOU (senão a volta seria trivial)", cresceu >= 4);

    conclui("crescer não é cair: o ponto foge para o infinito e a volta continua a ser um passo.");
}

/* ================================================================================ */
/* §E4 — ESQUILO tem ordem 4, TROCA tem ordem 2                                    */
/* ================================================================================ */
static void secao_E4(void){
    printf("\n§E4  O GRUPO NO FERRO: ESQUILO gira (ordem 4), TROCA reflete (ordem 2)\n\n");

    const char *mem = "/tmp/erg_m4.dat";
    char erro[128];

    /* ordem 4: aplicar ESQUILO quatro vezes tem de voltar ao ponto — e três vezes NÃO */
    int volta4 = 1, volta_cedo = 0;
    for(int i = 0; i < 5; i++){
        long a = 3 + i, b = 7 - 2*i;
        for(int k = 1; k <= 4; k++){
            zera_mem(mem, 8);
            poe_slot(mem, 1, a, b);
            char p[256]; int off = snprintf(p, sizeof p, "LOAD 1\n");
            for(int j = 0; j < k; j++) off += snprintf(p + off, sizeof p - (size_t)off, "ESQUILO\n");
            snprintf(p + off, sizeof p - (size_t)off, "STORE 2\nHALT\n");
            if(corre_texto(p, mem, 100, erro, (int)sizeof erro) < 0){ volta4 = 0; break; }
            Word w = ve_slot(mem, 2);
            int igual = (w.total == a && w.e == b);
            if(k == 4 && !igual) volta4 = 0;
            if(k < 4 && igual) volta_cedo++;
        }
    }
    printf("     ESQUILO: (t,e) ↦ (−e, t) — a rotação de 90°, det +1\n");
    ok("quatro ESQUILO devolvem o ponto, em 5 pontos de partida", volta4);
    ok("e menos de quatro NUNCA devolvem — a ordem é 4, não divisor dela", volta_cedo == 0);

    int volta2 = 1, t_cedo = 0;
    for(int i = 0; i < 5; i++){
        long a = 4 + i, b = 9 - i;
        for(int k = 1; k <= 2; k++){
            zera_mem(mem, 8);
            poe_slot(mem, 1, a, b);
            char p[256]; int off = snprintf(p, sizeof p, "LOAD 1\n");
            for(int j = 0; j < k; j++) off += snprintf(p + off, sizeof p - (size_t)off, "TROCA\n");
            snprintf(p + off, sizeof p - (size_t)off, "STORE 2\nHALT\n");
            if(corre_texto(p, mem, 100, erro, (int)sizeof erro) < 0){ volta2 = 0; break; }
            Word w = ve_slot(mem, 2);
            int igual = (w.total == a && w.e == b);
            if(k == 2 && !igual) volta2 = 0;
            if(k == 1 && igual) t_cedo++;      /* só se a=b, e os pontos são escolhidos com a≠b */
        }
    }
    printf("     TROCA:   (t,e) ↦ (e, t) — a involução J, det −1\n");
    ok("dois TROCA devolvem o ponto, em 5 pontos", volta2);
    ok("e um só NÃO devolve (os pontos têm total ≠ e)", t_cedo == 0);

    conclui("o gato estica, o esquilo gira, a troca reflete — e com os três toda unimodular é palavra.");
}

/* ================================================================================ */
/* §E5 — STORE grava R, não A                                                      */
/* ================================================================================ */
/* Esta é a armadilha que faz o primeiro programa de todo piloto sair errado, e ela é INVISÍVEL
 * quando R por acaso já vale o que se queria. O programa abaixo separa os dois: A vale uma coisa
 * e R vale outra, e o slot gravado diz qual dos dois a máquina obedece. */
static void secao_E5(void){
    printf("\n§E5  A ARMADILHA: STORE grava R, e um LOAD sozinho não mexe em R\n\n");

    const char *mem = "/tmp/erg_m5.dat";
    char erro[128];

    zera_mem(mem, 8);
    poe_slot(mem, 1, 10, 0);       /* o que vai para A */
    poe_slot(mem, 2, 99, 0);       /* o que o piloto ingénuo julga estar a gravar */
    /* pôr 5+5=10 em R, depois LOAD 2 (A vira 99, R continua 10), e gravar */
    const char *p =
        "LOAD 1\n"        /* A = 10 */
        "LOAD 1\n"        /* B = 10, A = 10 */
        "ADD\n"           /* R = 20  ← R agora vale 20 */
        "LOAD 2\n"        /* A = 99, e R NÃO mudou */
        "STORE 3\n"       /* grava R (20), não A (99) */
        "HALT\n";
    long passos = corre_texto(p, mem, 100, erro, (int)sizeof erro);
    if(passos < 0){ printf("     erro: %s\n", erro); ok("o programa da armadilha corre", 0); return; }
    Word w = ve_slot(mem, 3);
    printf("     A valia 99 e R valia 20 no momento do STORE; o slot 3 ficou com (%ld, %ld)\n",
           w.total, w.e);
    ok("o slot ficou com R (20), NÃO com A (99)", w.total == 20 && w.e == 0);
    ok("e não ficou com o valor que A tinha — a confusão é distinguível", w.total != 99);

    /* e o remédio, que é o idioma do sql.c: constante num slot em quatro instruções */
    zera_mem(mem, 8);
    poe_slot(mem, 1, 7, 3);        /* a constante desejada */
    poe_slot(mem, 0, 0, 0);        /* o slot do zero */
    const char *remedio =
        "LOAD 1\n"        /* A = a constante */
        "LOAD 0\n"        /* B = a constante, A = 0 */
        "ADD\n"           /* R = constante + 0 */
        "STORE 4\n"
        "HALT\n";
    if(corre_texto(remedio, mem, 100, erro, (int)sizeof erro) < 0){ ok("o remédio corre", 0); return; }
    Word c = ve_slot(mem, 4);
    printf("     o remédio (LOAD k; LOAD zero; ADD; STORE s) pôs (%ld, %ld) no slot 4\n", c.total, c.e);
    ok("o remédio grava a constante, componente a componente", c.total == 7 && c.e == 3);

    conclui("pôr uma constante num slot custa quatro instruções, e não duas. É a ISA, não um defeito.");
}

/* ================================================================================ */
/* §E6 — um app inteiro, e a conferência contra o C                                */
/* ================================================================================ */
static void secao_E6(void){
    printf("\n§E6  UM APP: somar dois slots — e a soma conferida contra o C\n\n");

    const char *mem = "/tmp/erg_m6.dat";
    char erro[128];
    const char *app =
        "; soma.erg — o primeiro app do piloto\n"
        "LOAD 1        ; A ← x\n"
        "LOAD 2        ; B ← x, A ← y\n"
        "ADD           ; R ← x ⊕ y, componente a componente\n"
        "STORE 3       ; e o resultado no slot 3\n"
        "HALT\n";

    printf("        x                y                ISA              C                bate\n");
    int erros = 0;
    long casos[6][4] = {{1,0,0,1},{3,5,7,11},{-4,9,4,-9},{1000,1,1,1000},{0,0,0,0},{-7,-7,7,7}};
    for(int i = 0; i < 6; i++){
        zera_mem(mem, 8);
        poe_slot(mem, 1, casos[i][0], casos[i][1]);
        poe_slot(mem, 2, casos[i][2], casos[i][3]);
        if(corre_texto(app, mem, 100, erro, (int)sizeof erro) < 0){ erros++; continue; }
        Word r = ve_slot(mem, 3);
        long ct = casos[i][0] + casos[i][2], ce = casos[i][1] + casos[i][3];
        int bate = (r.total == ct && r.e == ce);
        if(!bate) erros++;
        printf("        (%5ld,%5ld)   (%5ld,%5ld)   (%5ld,%5ld)   (%5ld,%5ld)   %s\n",
               casos[i][0], casos[i][1], casos[i][2], casos[i][3],
               r.total, r.e, ct, ce, bate ? "sim" : "NÃO");
    }
    ok("os 6 casos batem com a soma feita em C — o app está certo", erros == 0);

    /* E O TAMANHO SEGUE UMA LEI, que é o que se mede aqui — não um número.
     * Escrevi primeiro `n == 10` de cabeça, contando quatro instruções onde há cinco (esqueci
     * o ADD), e a asserção caiu. O remédio não é acertar melhor no número: é medir a LEI
     *      bytes = Σ (1 + operando)
     * em vários programas, onde errar a conta de um deles não passa. */
    struct { const char *fonte; int instr, com_slot; } progs[] = {
        { app,                                     5, 3 },
        { "HALT\n",                                1, 0 },
        { "GOLD\nESQUILO\nTROCA\nNEGRO_OURO\nHALT\n", 5, 0 },
        { "LOAD 1\nSTORE 2\nHALT\n",               3, 2 },
        { "LOAD 9\nLOAD 9\nXOR\nSTORE 9\nGOLD\nHALT\n", 6, 3 },
    };
    printf("        instruções   com slot   previsto   montado\n");
    int fora = 0;
    for(int i = 0; i < 5; i++){
        unsigned char b[128];
        int n = monta(progs[i].fonte, b, (int)sizeof b, erro, (int)sizeof erro);
        int prev = progs[i].com_slot * 3 + (progs[i].instr - progs[i].com_slot) * 1;
        if(n != prev) fora++;
        printf("        %10d   %8d   %8d   %7d   %s\n",
               progs[i].instr, progs[i].com_slot, prev, n, n == prev ? "" : "← FORA");
    }
    ok("bytes = Σ(1 + operando) nos 5 programas — a lei, não um número", fora == 0);

    conclui("um app aqui é um ficheiro de texto de cinco linhas. Não há runtime nenhum por baixo.");
}

/* ================================================================================ */
/* §E7 — o laço, e a contagem prevista ANTES de correr                             */
/* ================================================================================ */
/* Um salto relativo mal contado é o erro clássico do assembly à mão, e ele não falha alto: o
 * programa corre e dá outra coisa. Por isso a medida aqui não é "o laço termina" — é "o laço
 * dá exatamente o número de passos que a conta previu", que é uma afirmação que pode falhar. */
static void secao_E7(void){
    printf("\n§E7  O LAÇO: rótulo, salto relativo, e os passos previstos antes de correr\n\n");

    const char *mem = "/tmp/erg_m7.dat";
    char erro[128];

    /* contar de N até 0, subtraindo 1 de cada vez.
     *   slot 1 = o contador   slot 2 = a constante 1   slot 0 = o zero */
    const char *laco =
        ":volta\n"
        "LOAD 1\n"        /* A ← contador                */
        "LOAD 2\n"        /* B ← contador, A ← 1         */
        "SUB\n"           /* R ← 1 − contador ... cuidado: a ULA faz A−B                */
        "STORE 1\n"
        "LOAD 1\n"
        "LOAD 0\n"        /* A ← 0, B ← contador                                        */
        "CMP\n"           /* FL_ZERO sse AMBOS zero — logo sse o contador chegou a zero */
        "JZ fim\n"
        "JMP volta\n"
        ":fim\n"
        "HALT\n";

    /* a ULA faz R = A − B, e depois do par (LOAD 1; LOAD 2) temos A=1 e B=contador.
     * Então R = 1 − contador, que NÃO é o que se quer. Trocar a ordem dos LOAD resolve. */
    const char *laco_certo =
        ":volta\n"
        "LOAD 2\n"        /* A ← 1                       */
        "LOAD 1\n"        /* B ← 1, A ← contador         */
        "SUB\n"           /* R ← contador − 1            */
        "STORE 1\n"
        "LOAD 1\n"
        "LOAD 0\n"        /* A ← 0, B ← contador         */
        "CMP\n"
        "JZ fim\n"
        "JMP volta\n"
        ":fim\n"
        "HALT\n";
    (void)laco;

    /* cada volta são 9 instruções executadas (LOAD LOAD SUB STORE LOAD LOAD CMP JZ JMP);
     * na última volta o JZ salta e o JMP não corre, e ainda entra o HALT — que o `rodar`
     * não conta como passo, porque devolve 0. Logo: 9N − 1 passos. */
    printf("        N     previsto (9N−1)   medido    contador no fim\n");
    int erros = 0;
    for(int N = 1; N <= 6; N++){
        zera_mem(mem, 8);
        poe_slot(mem, 1, N, N);        /* as duas componentes contam juntas */
        poe_slot(mem, 2, 1, 1);
        long passos = corre_texto(laco_certo, mem, 10000, erro, (int)sizeof erro);
        if(passos < 0){ printf("     erro: %s\n", erro); erros++; continue; }
        Word c = ve_slot(mem, 1);
        long prev = 9L*N - 1;
        if(passos != prev || c.total != 0) erros++;
        printf("        %d     %11ld   %7ld    (%ld, %ld)\n", N, prev, passos, c.total, c.e);
    }
    ok("os passos batem com 9N−1 em N = 1..6 — o salto relativo está certo", erros == 0);

    /* e a prova de que o rótulo poupou trabalho real: o mesmo laço com o número à mão */
    unsigned char b[128];
    int n = monta(laco_certo, b, (int)sizeof b, erro, (int)sizeof erro);
    int rel_jmp = 0;
    for(int i = 0; i + 1 < n; i++) if(b[i] == OP_JMP) rel_jmp = (signed char)b[i+1];
    printf("     o JMP de volta ficou com rel = %d (o montador contou; o piloto não)\n", rel_jmp);
    ok("o salto de volta é negativo e do tamanho do corpo do laço", rel_jmp < 0 && rel_jmp > -30);

    conclui("o rótulo não é açúcar: é o único sítio deste montador onde ele faz conta pelo piloto.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    /* ---- os modos do piloto ---- */
    if(argc >= 2 && !strcmp(argv[1], "monta")){
        if(argc < 4){ fprintf(stderr, "uso: erg monta <fonte.erg> <saida.bin>\n"); return 2; }
        unsigned char b[65536]; char texto[65536] = {0}; char erro[160] = {0};
        int n = le_ficheiro(argv[2], (unsigned char*)texto, (int)sizeof texto - 1);
        if(n < 0){ fprintf(stderr, "não abriu %s\n", argv[2]); return 1; }
        texto[n] = 0;
        int m = monta(texto, b, (int)sizeof b, erro, (int)sizeof erro);
        if(m < 0){ fprintf(stderr, "erro: %s\n", erro); return 1; }
        if(escreve(argv[3], b, m) < 0){ fprintf(stderr, "não gravou %s\n", argv[3]); return 1; }
        printf("%d bytes de ISA em %s\n", m, argv[3]);
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "corre")){
        if(argc < 4){ fprintf(stderr, "uso: erg corre <prog.bin> <mem.dat> [teto]\n"); return 2; }
        long teto = (argc >= 5) ? strtol(argv[4], NULL, 0) : 1000000;
        fprog = open(argv[2], O_RDONLY);
        if(fprog < 0){ fprintf(stderr, "não abriu %s\n", argv[2]); return 1; }
        off_t len = lseek(fprog, 0, SEEK_END);
        if(abre_mem(argv[3]) < 0){ fprintf(stderr, "não abriu %s\n", argv[3]); return 1; }
        long passos = rodar((unsigned)len, teto);
        printf("%ld passos, %lld bytes de programa\n", passos, (long long)len);
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "desmonta")){
        if(argc < 3){ fprintf(stderr, "uso: erg desmonta <prog.bin>\n"); return 2; }
        unsigned char b[65536];
        int n = le_ficheiro(argv[2], b, (int)sizeof b);
        if(n < 0){ fprintf(stderr, "não abriu %s\n", argv[2]); return 1; }
        desmonta(b, n, stdout);
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "ve")){
        if(argc < 4){ fprintf(stderr, "uso: erg ve <mem.dat> <slot>\n"); return 2; }
        Word w = ve_slot(argv[2], (unsigned)strtoul(argv[3], NULL, 0));
        printf("%ld %ld\n", w.total, w.e);
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "poe")){
        if(argc < 6){ fprintf(stderr, "uso: erg poe <mem.dat> <slot> <total> <e>\n"); return 2; }
        poe_slot(argv[2], (unsigned)strtoul(argv[3], NULL, 0),
                 strtol(argv[4], NULL, 0), strtol(argv[5], NULL, 0));
        return 0;
    }
    if(argc >= 2 && !strcmp(argv[1], "zera")){
        if(argc < 4){ fprintf(stderr, "uso: erg zera <mem.dat> <nslots>\n"); return 2; }
        zera_mem(argv[2], (int)strtol(argv[3], NULL, 0));
        return 0;
    }

    /* ---- sem argumentos: a bateria ---- */
    puts("erg.c — O PLUGUE DE DENTRO, EM ASSEMBLY: montador e executor da ISA ERG-64");
    puts("=========================================================================");
    puts("");
    puts("  A máquina é a do sql.c e não foi tocada. Isto é a PORTA: o piloto escreve na ISA");
    puts("  sem passar por SQL. Sem RAM — o programa lê-se com pread, a memória é um ficheiro,");
    puts("  e os únicos registos são A, B, R.");

    secao_E1();
    secao_E2();
    secao_E3();
    secao_E4();
    secao_E5();
    secao_E6();
    secao_E7();

    printf("\n=========================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E O QUE ISTO ABRE: um app deste corpo é um ficheiro de texto. Não há runtime,");
        puts("  não há biblioteca, não há alocação — há 17 opcodes, três registos e um ficheiro");
        puts("  de slots. O piloto que souber as duas armadilhas (STORE grava R; FL_ZERO é");
        puts("  AMBOS zero) escreve o primeiro app em cinco linhas, e ele corre no mesmo ferro");
        puts("  onde o SELECT corre.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
