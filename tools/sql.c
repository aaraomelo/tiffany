
/* sql.c — SQL NO METAL. Compila para a ISA, e a memória é o DISCO. Sem RAM.
 *
 * Nada aqui é simulado por cima de estruturas em memória: o SQL vira BYTECODE da ISA do
 * broca-so (ula/instrucoes.h), o bytecode vive num arquivo, a memória da máquina vive noutro
 * arquivo, e o interpretador lê instrução por instrução com pread. O único estado em RAM são
 * os três registradores (A, B, R), o pc e as flags — como no metal.
 *
 * A ISA, transcrita de ula/instrucoes.c (não reinventada):
 *
 *   HALT                     para
 *   LOAD  slot(u16)          B←A ; A←mem[slot]
 *   STORE slot(u16)          mem[slot]←R            (grava R, NÃO A)
 *   ADD SUB AND OR XOR       R ← ula(A,B)           (componente a componente)
 *   GOLD SILVER BRONZE       A←metal(A) ; R←A
 *   CMP                      FL_ZERO sse A e B são AMBOS zero; FL_EQ se iguais
 *   JMP JZ JNZ  rel(s8)      pc ← pc + 1 + rel
 *   FOLD UNFOLD PROJECT LIFT as folhas e as projeções
 *
 * Duas consequências da ISA real, que moldam o compilador:
 *   (1) STORE grava R, então pôr uma constante num slot é LOAD k, LOAD zero, ADD, STORE.
 *   (2) FL_ZERO é "ambos zero", então a igualdade a=k testa-se por SUB e depois CMP com A=0:
 *       LOAD col, LOAD k, SUB, STORE tmp, LOAD tmp, LOAD zero, CMP  →  FL_ZERO sse col=k.
 *   (3) o endereço do slot é IMEDIATO (u16 na instrução): não há indexação indireta. Logo o
 *       compilador DESENROLA a varredura — ele lê o catálogo antes de compilar e emite o
 *       código das linhas que existem. O programa é compilado para o estado atual da tabela.
 *
 * Mapa da memória (slots de 16 bytes: {long total, long e}):
 *   0   catálogo {ncols, nrows}
 *   1   a constante 0        2  a constante 1        3  temporário
 *   4   o contador de casamentos
 *   8   a constante da consulta (o k do WHERE)
 *   16+ o bitmap de casamento, uma linha por slot
 *   1024+ as linhas: linha i, coluna j  →  slot 1024 + i*ncols + j
 *
 *   cc -O2 -std=c99 sql.c -o sql
 *   ./sql <base> "CREATE TABLE t (a,b,c)"
 *   ./sql <base> "INSERT INTO t VALUES (7,8,9)"
 *   ./sql <base> "SELECT * FROM t WHERE a = 7"
 *   ./sql teste
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <strings.h>
#include <sys/wait.h>

/* ---------------- a ISA (transcrita) ---------------- */
enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_SILVER, OP_GOLD, OP_BRONZE, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_UNFOLD, OP_PROJECT, OP_LIFT, OP_LOADS, OP_SPECT };
#define FL_ZERO 0x01
#define FL_EQ   0x02
#define FL_LT   0x04

typedef struct { long total, e; } Word;
typedef struct { Word A, B, R; unsigned pc; unsigned char flags; } Regs;

/* ---------------- os slots ---------------- */
#define S_CAT     0
#define S_ZERO    1
#define S_UM      2
#define S_TMP     3
#define S_CONTA   4
#define S_MASK    5          /* a máscara do bit de sinal — é ela que dá o < e o >     */
#define S_ACC     6          /* o acumulador booleano da cláusula inteira                */
#define S_V       7          /* o valor do SET                                          */
#define S_K       8          /* 8..23  a constante de cada condição                     */
#define S_COND    24         /* 24..39 o resultado de cada condição (0 ou 1)            */
#define S_TERMO   40         /* 40..47 o resultado de cada termo (as condições em AND)  */
#define S_UME     48         /* o "um" no campo .e — para incrementar nrows              */
#define S_DIA     58         /* o DIÁRIO: {total = ação pendente, e = coluna do SET}      */
#define S_MT      57         /* mascara {total=todos os bits, e=0} — limpa o .e apos GOLD */
#define S_KZ      49         /* 49..56  o zero de cada comparação (a contração compara com 0) */
#define S_LIN     4096       /* 4096+  o rascunho de cada átomo: acc, prod, cnt, passo…   */
#define S_EXPR    64         /* 64..191  os temporários da árvore da expressão          */
#define S_MATCH   256        /* bitmap do resultado, uma linha por slot (256..511)      */
#define MAXCOND   4          /* condições por termo                                     */
#define MAXTERMO  4          /* termos ligados por OR                                   */
#define S_VIVO    512        /* a linha existe? o DELETE zera aqui (512..1023)          */
#define S_LINHAS  1024
#define MAXLIN    250
#define MAXNO     64         /* nós da árvore do WHERE                                  */
#define SLOTSZ    16

/* ---------------- a memória É o disco ---------------- */
static int fmem = -1, fprog = -1;

static Word mem_le(unsigned slot){
    Word w = {0,0};
    if(pread(fmem, &w, SLOTSZ, (off_t)slot*SLOTSZ) != SLOTSZ){ w.total = 0; w.e = 0; }
    return w;
}
static void mem_grava(unsigned slot, Word w){
    pwrite(fmem, &w, SLOTSZ, (off_t)slot*SLOTSZ);
}
/* A BARREIRA DO BANCO: dado, fsync, ponteiro, fsync.
 *
 * banco.c já tinha esta disciplina e o SQL não: aqui as células e o catálogo iam no MESMO
 * programa, sem nada entre eles. A ordem do programa estava certa, mas ordem de programa não
 * é ordem no disco — sem fsync o sistema pode pôr o catálogo no prato antes das células, e
 * uma queda no meio deixa o catálogo a contar uma linha que não existe.
 *
 * Com a barreira, as duas quedas possíveis são as duas boas: antes do ponteiro, a linha é
 * invisível e não faz mal nenhum; depois dele, a linha está inteira. Nunca meia. */
static void barreira(void){ if(fmem >= 0) fsync(fmem); }

/* Travamento injetado, para MEDIR a barreira em vez de a afirmar. SQL_TRAVA=1 mata o processo
 * logo depois do dado e antes do ponteiro — que é o único ponto onde a ordem importa.
 * _exit(9) modela queda de PROCESSO: o que já foi para o núcleo sobrevive. Queda de energia
 * é mais dura, e é contra ela que o fsync existe; o teste cobre a ordem, não o prato. */
static int trava_em = 0;               /* o teste põe aqui, sem depender do ambiente */
static void trava_se_pedido(int ponto){
    const char *e = getenv("SQL_TRAVA");
    if(trava_em == ponto || (e && atoi(e) == ponto)) _exit(9);
}
static unsigned char prog_le(unsigned pc){
    unsigned char b = OP_HALT;
    if(pread(fprog, &b, 1, (off_t)pc) != 1) return OP_HALT;
    return b;
}

/* ---------------- a máquina: um passo da ISA, fiel ---------------- */
static Word ula_add(Word a, Word b){ Word r = { a.total+b.total, a.e+b.e }; return r; }
static Word ula_sub(Word a, Word b){ Word r = { a.total-b.total, a.e-b.e }; return r; }
static Word ula_and(Word a, Word b){ Word r = { a.total&b.total, a.e&b.e }; return r; }
static Word ula_or (Word a, Word b){ Word r = { a.total|b.total, a.e|b.e }; return r; }
static Word ula_xor(Word a, Word b){ Word r = { a.total^b.total, a.e^b.e }; return r; }
static int  zero(Word w){ return w.total == 0 && w.e == 0; }
/* os metais, transcritos de broca-so ula/cifra.c — NAO reinventados:
 *   cifra_an(w,n) = word_make(n*w.total + w.e, w.total)
 * Com n=1 isto e (a,b) -> (a+b, a): o DESLOCAMENTO de Fibonacci, que e a multiplicacao
 * pelo rei medida em coroa.c §A5. GOLD e a multiplicacao por sigma, e ja estava na ISA. */
static Word cifra_an(Word w, int n){ Word r = { (long)n*w.total + w.e, w.total }; return r; }

static int passo(Regs *r, unsigned prog_len){
    if(r->pc >= prog_len) return 0;
    unsigned pc = r->pc;
    unsigned char op = prog_le(pc++);
    switch(op){
    case OP_HALT: return 0;
    case OP_LOAD: case OP_LOADS: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
        pc += 2;
        r->B = r->A; r->A = mem_le(slot);
        break; }
    case OP_STORE: {
        unsigned slot = (unsigned)prog_le(pc) | ((unsigned)prog_le(pc+1) << 8);
        pc += 2;
        mem_grava(slot, r->R);
        break; }
    case OP_GOLD:   r->A = cifra_an(r->A, 1); r->R = r->A; break;
    case OP_SILVER: r->A = cifra_an(r->A, 2); r->R = r->A; break;
    case OP_BRONZE: r->A = cifra_an(r->A, 3); r->R = r->A; break;
    case OP_ADD: r->R = ula_add(r->A, r->B); break;
    case OP_SUB: r->R = ula_sub(r->A, r->B); break;
    case OP_AND: r->R = ula_and(r->A, r->B); break;
    case OP_OR:  r->R = ula_or (r->A, r->B); break;
    case OP_XOR: r->R = ula_xor(r->A, r->B); break;
    case OP_CMP: {
        unsigned char f = 0;
        if(zero(r->A) && zero(r->B)) f |= FL_ZERO;
        if(r->A.total == r->B.total && r->A.e == r->B.e) f |= FL_EQ;
        else if(r->A.total < r->B.total) f |= FL_LT;
        r->flags = f;
        break; }
    case OP_JMP: { int rel = (signed char)prog_le(pc); pc = (unsigned)((int)pc + 1 + rel); r->pc = pc; return 1; }
    case OP_JZ:  { int rel = (signed char)prog_le(pc);
                   pc = (r->flags & FL_ZERO) ? (unsigned)((int)pc + 1 + rel) : pc + 1;
                   r->pc = pc; return 1; }
    case OP_JNZ: { int rel = (signed char)prog_le(pc);
                   pc = !(r->flags & FL_ZERO) ? (unsigned)((int)pc + 1 + rel) : pc + 1;
                   r->pc = pc; return 1; }
    default: return 0;
    }
    r->pc = pc;
    return 1;
}
static long rodar(unsigned prog_len){
    Regs r; memset(&r, 0, sizeof r);
    long passos = 0;
    while(passo(&r, prog_len)){ if(++passos > 50000000L) break; }
    return passos;
}

/* ---------------- o montador: escreve o bytecode NO DISCO ---------------- */
static unsigned pc_emit = 0;
static void emit1(unsigned char b){ pwrite(fprog, &b, 1, (off_t)pc_emit); pc_emit++; }
/* A VARREDURA É UMA PROGRESSÃO ARITMÉTICA NO ENDEREÇO.
 *
 * A ISA não tem endereçamento indireto — o slot é imediato na instrução (e LOADS, que eu
 * esperava que fosse indireto, é LOAD com a cifra espectral; fui ver em broca-so). Por isso o
 * compilador desenrolava a varredura: um bloco por linha, e o bytecode crescia LINEARMENTE com
 * a tabela. Medido antes desta mudança: 147 bytes por linha, 75 KB para 512 linhas.
 *
 * Mas o endereço de cada linha É uma PA. A linha i, coluna j, mora em S_LINHAS + i·ncols + j:
 * passo constante ncols. O bitmap e o vivo andam de 1 em 1. E o resto não anda.
 *
 * Então emite-se UM bloco — o da linha 0 — e anda-se com ele: antes de cada passagem, cada
 * endereço que depende da linha avança o seu passo. O bytecode passa a ser O(1) na tabela, e
 * quem varre é a progressão, não o compilador.
 *
 * O passo sai da FAIXA do slot, sem tocar em nenhum lugar de chamada: quem está nas linhas anda
 * ncols, quem está no bitmap ou no vivo anda 1, e os temporários e constantes não andam. */
#define NREL 512
static struct { unsigned off, base; long passo; } rel[NREL];
static int nrel = 0;
static long rel_ncols = 0;            /* > 0 só enquanto se emite o MOLDE */

static long passo_do_slot(unsigned s){
    if(!rel_ncols) return 0;
    if(s >= S_LINHAS) return rel_ncols;      /* a linha inteira: passo = ncols */
    if(s >= S_VIVO)   return 1;              /* o vivo: uma por linha           */
    if(s >= S_MATCH && s < S_VIVO) return 1; /* o bitmap: uma por linha         */
    return 0;                                /* constantes e rascunho: parados  */
}
static long mdc_l(long a, long b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ long t=a%b; a=b; b=t; } return a?a:1; }
static void emit_slot(unsigned char op, unsigned slot){
    long p = passo_do_slot(slot);
    emit1(op);
    if(p && nrel < NREL){ rel[nrel].off = pc_emit; rel[nrel].base = slot; rel[nrel].passo = p; nrel++; }
    emit1((unsigned char)(slot & 0xFF)); emit1((unsigned char)(slot >> 8));
}
/* anda o molde uma linha: cada sítio de realocação avança o seu passo */
static void rel_anda(long i){
    for(int t = 0; t < nrel; t++){
        unsigned v = (unsigned)(rel[t].base + i * rel[t].passo);
        unsigned char lo = (unsigned char)(v & 0xFF), hi = (unsigned char)(v >> 8);
        pwrite(fprog, &lo, 1, (off_t)rel[t].off);
        pwrite(fprog, &hi, 1, (off_t)rel[t].off + 1);
    }
}
/* põe a constante do slot k no slot destino: LOAD k, LOAD zero, ADD, STORE dest */
static void emit_copia(unsigned de, unsigned para){
    emit_slot(OP_LOAD, de);
    emit_slot(OP_LOAD, S_ZERO);
    emit1(OP_ADD);
    emit_slot(OP_STORE, para);
}

/* ---------------- SQL: o analisador ---------------- */
static void pula(const char **p){ while(**p && isspace((unsigned char)**p)) (*p)++; }
static int palavra(const char **p, const char *w){
    pula(p);
    size_t n = strlen(w);
    if(strncasecmp(*p, w, n) == 0 && (!isalnum((unsigned char)(*p)[n]))){ *p += n; return 1; }
    return 0;
}
static int numero(const char **p, long *v){
    pula(p);
    int sinal = 1;
    if(**p == '-'){ sinal = -1; (*p)++; }
    if(!isdigit((unsigned char)**p)) return 0;
    long x = 0;
    while(isdigit((unsigned char)**p)){ x = x*10 + (**p - '0'); (*p)++; }
    *v = sinal * x;
    return 1;
}
static int ident(const char **p, char *out, size_t cap){
    pula(p);
    size_t k = 0;
    if(**p == '*'){ (*p)++; snprintf(out, cap, "*"); return 1; }
    while(isalnum((unsigned char)**p) || **p == '_'){ if(k+1 < cap) out[k++] = **p; (*p)++; }
    out[k] = 0;
    return k > 0;
}

/* ---------------- os comandos ---------------- */
static int cria(const char *resto){
    const char *p = resto;
    char nome[64];
    if(!ident(&p, nome, sizeof nome)) return 0;
    pula(&p); if(*p != '(') return 0; p++;
    long ncols = 0; char c[64];
    while(1){ if(!ident(&p, c, sizeof c)) break; ncols++; pula(&p);
              if(*p == ','){ p++; continue; } break; }
    /* o catálogo é escrito PELA MÁQUINA: constantes + STORE, compilado e executado */
    pc_emit = 0;
    Word w; w.total = ncols; w.e = 0; mem_grava(S_K, w);       /* a constante entra pela memória */
    w.total = 0; w.e = 0; mem_grava(S_ZERO, w);
    w.total = -1; w.e = 0; mem_grava(S_MT, w);      /* AND com isto zera o .e e guarda o total */
    w.total = 1; w.e = 0; mem_grava(S_UM, w);
    emit_copia(S_K, S_CAT);                                     /* cat.total = ncols */
    emit1(OP_HALT);
    rodar(pc_emit);
    Word cat = mem_le(S_CAT); cat.e = 0; mem_grava(S_CAT, cat); /* nrows = 0 */
    printf("tabela %s criada: %ld colunas\n", nome, ncols);
    return 1;
}

static int insere(const char *resto){
    const char *p = resto;
    char nome[64];
    if(!palavra(&p, "INTO")) return 0;
    if(!ident(&p, nome, sizeof nome)) return 0;
    if(!palavra(&p, "VALUES")) return 0;
    pula(&p); if(*p != '(') return 0; p++;

    Word cat = mem_le(S_CAT);
    long ncols = cat.total, nrows = cat.e;
    long v[64], nv = 0;
    long den[16]; for(int q = 0; q < 16; q++) den[q] = 1;
    while(nv < ncols && numero(&p, &v[nv])){
        /* O VALOR RACIONAL. A Word tem duas componentes e um racional é um par: o numerador
         * no total e o denominador no e. Guarda-se a CLASSE — reduzida pelo mdc, denominador
         * positivo —, que é o que racional_pg.c §Q1 mediu ser o representante único. */
        const char *volta = p;
        pula(&p);
        if(*p == '/'){
            const char *ap = p + 1;
            long q;
            if(numero(&ap, &q) && q != 0){
                p = ap;
                if(q < 0){ q = -q; v[nv] = -v[nv]; }
                long g = mdc_l(v[nv], q);
                if(g > 1){ v[nv] /= g; q /= g; }
                den[nv] = q;
            } else p = volta;
        } else p = volta;
        nv++; pula(&p); if(*p == ','){ p++; continue; } break;
    }
    if(nv != ncols){ printf("erro: a tabela tem %ld colunas, vieram %ld\n", ncols, nv); return 0; }

    /* compila o INSERT: cada valor entra por um slot de constante e é gravado pela ISA */
    pc_emit = 0;
    Word w = {0,0}; mem_grava(S_ZERO, w);
    for(long j = 0; j < ncols; j++){
        w.total = v[j]; w.e = den[j];      /* e = denominador; 1 para inteiro */
        mem_grava(S_K + (unsigned)j, w);                        /* a constante, na memória */
        emit_copia(S_K + (unsigned)j, S_LINHAS + (unsigned)(nrows*ncols + j));
    }
    /* nrows++ pela própria máquina: LOAD cat, LOAD um, ADD, STORE — mas nrows é o campo .e,
     * e a ULA soma componente a componente; então a constante um vai no campo .e. */
    /* a linha nasce VIVA (o DELETE zera este slot).
     * ATENÇÃO à ordem: emit_copia só EMITE; o programa roda depois, no rodar(). Se a
     * constante for sobrescrita entre as duas fases, a máquina lê o valor trocado — foi
     * exatamente o que aconteceu aqui quando S_UM servia às duas coisas. Cada constante
     * tem o seu slot. */
    w.total = 1; w.e = 0; mem_grava(S_UM, w);
    emit_copia(S_UM, S_VIVO + (unsigned)nrows);
    emit1(OP_HALT);
    long passos = rodar(pc_emit);        /* FASE 1: só o dado */

    barreira();                          /* o dado está no prato antes de existir o ponteiro */
    trava_se_pedido(1);                  /* e é aqui que o teste derruba, para ver o que sobra */

    /* FASE 2: só então o ponteiro. nrows vive no campo .e, e a ULA soma componente a
     * componente — por isso o incremento é uma constante com .e = 1. */
    pc_emit = 0;                         /* fase 2 é um programa PRÓPRIO: rodar() parte de 0 */
    w.total = 0; w.e = 1; mem_grava(S_UME, w);
    emit_slot(OP_LOAD, S_CAT);
    emit_slot(OP_LOAD, S_UME);
    emit1(OP_ADD);
    emit_slot(OP_STORE, S_CAT);
    emit1(OP_HALT);
    passos += rodar(pc_emit);
    barreira();

    cat = mem_le(S_CAT);
    printf("1 linha inserida (%ld colunas) — %u bytes de ISA, %ld passos; agora %ld linhas\n",
           ncols, pc_emit, passos, cat.e);
    return 1;
}

/* As três ações que uma varredura pode ter na linha que casa. */
enum { ACAO_MARCA, ACAO_SET, ACAO_APAGA };

/* Emite o teste da condição e o bloco de ação para UMA linha.
 *
 * A ISA não tem salto por FL_LT — só JZ/JNZ, que olham FL_ZERO. Então as três comparações
 * reduzem-se todas a um teste de ZERO, e a diferença entre elas é só ordem de subtração:
 *
 *   col = k   ->  dif = k − col ;                CMP com A=0  ->  FL_ZERO sse igual   (JZ)
 *   col < k   ->  dif = col − k ; dif AND sinal; CMP com A=0  ->  FL_ZERO sse col ≥ k (JNZ)
 *   col > k   ->  dif = k − col ; dif AND sinal; CMP com A=0  ->  FL_ZERO sse col ≤ k (JNZ)
 *
 * O sinal extrai-se por AND com o bit 63 — a ISA não tem deslocamento, mas tem AND, e isso
 * basta: negativo é exatamente quem tem esse bit aceso.
 */
/* A árvore do WHERE. Com parênteses, a cláusula deixa de ser plana e vira árvore de
 * verdade — e a gramática é a do SQL:
 *
 *     expr  := termo (OR termo)*
 *     termo := fator (AND fator)*
 *     fator := '(' expr ')' | coluna <op> número
 *
 * Os seis operadores são TRÊS mais uma negação: != é não-=, <= é não->, >= é não-<. E negar
 * um slot que vale 0 ou 1 é XOR com 1 — opcode que a ISA já tem. Não se inventou comparação
 * nova: acrescentou-se um XOR.
 */
#define NCOL 6                 /* colunas que uma expressão pode citar */
#define CMAX 8                 /* (histórico: era o teto da soma repetida — ver emit_mul_zeck) */

#define NI    (NCOL+1)         /* símbolo 0 = a constante 1; 1..NCOL = as colunas */
#define KGRAU 3                /* ordem do tensor: grau máximo do monômio          */
#define NMON  343              /* NI^KGRAU — as casas do multi-índice (7^3)        */

/* O TENSOR DE GRAU k, com MULTI-ÍNDICE.
 *
 * Um monômio é uma tupla ORDENADA de KGRAU símbolos, e o símbolo 0 é a constante 1 — logo a
 * tupla (0,0,0) é o termo constante, (0,0,i) é linear, (0,i,j) é quadrático, (i,j,l) é cúbico.
 * Graus diferentes não são casos diferentes: são a mesma tabela com mais ou menos zeros.
 *
 * A posição ORDENADA é o endereço, e é só isso que faz a comutatividade desaparecer: as k!
 * escritas de um monômio caem na mesma casa por aritmética de índice, não por regra
 * (tools/tensor.c §T2, onde a contagem C(n+k−1,k) foi conferida).
 *
 * As duas operações são as do tensor: a SOMA soma casa a casa; o PRODUTO junta os multi-índices
 * (os graus somam) e multiplica os coeficientes. O parêntese entra como posição e sai. */
/* O TENSOR SOBRE ℚ: um denominador COMUM por tensor.
 *
 * As classes inteiras do racional_pg.c entram aqui: cada tensor é (coeficientes, denominador),
 * e o denominador é um só para o tensor inteiro — não um por monómio. Isso basta porque as duas
 * operações são lineares no denominador:
 *
 *     soma      cruza:      (A,p) + (B,q) = (qA + pB, pq)
 *     produto   multiplica: (A,p) · (B,q) = (A·B, pq)
 *
 * E na hora de emitir, o denominador SOME: a contração já pôs tudo de um lado e a comparação é
 * contra ZERO, logo (N/D) OP 0 ⟺ N OP 0 desde que D > 0 — e mantém-se D > 0 por construção.
 * O racional entra no analisador, opera como classe, e sai inteiro para o metal. */
struct tensor { long c[NMON]; long den; };

static void mi_ordena(int *d){                         /* ordenação por inserção, KGRAU pequeno */
    for(int i = 1; i < KGRAU; i++)
        for(int j = i; j > 0 && d[j] < d[j-1]; j--){ int t = d[j]; d[j] = d[j-1]; d[j-1] = t; }
}
static int mi_cod(int *d){                             /* a tupla ordenada é o endereço */
    mi_ordena(d);
    int r = 0;
    for(int t = KGRAU-1; t >= 0; t--) r = r*NI + d[t];
    return r;
}
static void mi_de(int cod, int *d){
    for(int t = 0; t < KGRAU; t++){ d[t] = cod % NI; cod /= NI; }
}
static int mi_grau(int cod){
    int d[KGRAU], g = 0; mi_de(cod, d);
    for(int t = 0; t < KGRAU; t++) if(d[t]) g++;
    return g;
}
static struct tensor ten_zero(void){ struct tensor t; memset(&t,0,sizeof t); t.den = 1; return t; }
/* o representante da classe: divide tudo pelo mdc comum, e deixa o denominador positivo */
static void ten_reduz(struct tensor *t){
    if(t->den < 0){ t->den = -t->den; for(int i = 0; i < NMON; i++) t->c[i] = -t->c[i]; }
    long g = t->den;
    for(int i = 0; i < NMON; i++) if(t->c[i]) g = mdc_l(g, t->c[i]);
    if(g > 1){ t->den /= g; for(int i = 0; i < NMON; i++) t->c[i] /= g; }
    if(t->den == 0) t->den = 1;
}
static void ten_mon(struct tensor *t, int *d, long c){ t->c[mi_cod(d)] += c; }
static void ten_const(struct tensor *t, long k){
    int d[KGRAU]; memset(d, 0, sizeof d); ten_mon(t, d, k);
}
static void ten_var(struct tensor *t, int col){
    int d[KGRAU]; memset(d, 0, sizeof d); d[0] = col + 1; ten_mon(t, d, 1);
}
static struct tensor ten_soma(struct tensor a, struct tensor b, int sinal){
    long p = a.den ? a.den : 1, q = b.den ? b.den : 1;      /* (A,p) ± (B,q) = (qA ± pB, pq) */
    struct tensor r = ten_zero();
    for(int i = 0; i < NMON; i++) r.c[i] = q * a.c[i] + sinal * p * b.c[i];
    r.den = p * q;
    ten_reduz(&r);
    return r;
}
/* o produto: junta os multi-índices — os graus SOMAM. Passar de KGRAU é recusado, e dizer
 * isso é melhor que truncar em silêncio. */
static int ten_mul(struct tensor *r, const struct tensor *a, const struct tensor *b){
    *r = ten_zero();
    for(int x = 0; x < NMON; x++){
        if(!a->c[x]) continue;
        int dx[KGRAU]; mi_de(x, dx);
        for(int y = 0; y < NMON; y++){
            if(!b->c[y]) continue;
            int dy[KGRAU]; mi_de(y, dy);
            if(mi_grau(x) + mi_grau(y) > KGRAU) return 0;
            int d[KGRAU], k = 0;
            for(int t = 0; t < KGRAU; t++) if(dx[t]) d[k++] = dx[t];
            for(int t = 0; t < KGRAU; t++) if(dy[t]) d[k++] = dy[t];
            while(k < KGRAU) d[k++] = 0;
            ten_mon(r, d, a->c[x] * b->c[y]);
        }
    }
    r->den = (a->den ? a->den : 1) * (b->den ? b->den : 1);   /* os denominadores multiplicam */
    ten_reduz(r);
    return 1;
}
static int ten_constante(struct tensor t){
    for(int i = 1; i < NMON; i++) if(t.c[i]) return 0;
    return t.c[0] == t.c[0];
}

enum { NO_COND, NO_AND, NO_OR };
struct no {
    int tipo;
    int esq, dir;          /* índices na árvore                                   */
    int op;                /* '=', '<', '>'                                       */
    int nega;              /* 1 se o resultado deve ser invertido (!=, <=, >=)     */
    struct tensor v;       /* o lado numérico já contraído: L − R                  */
    int decidido;          /* 0 = precisa da linha; 1 = já se sabe (const)         */
    int valor;             /* se decidido: 0 ou 1                                  */
    int atomo;             /* índice do átomo distinto, depois da contração        */
};
struct arvore {
    struct no no[MAXNO];
    int n;
    int raiz;
    /* a CONTRAÇÃO: os átomos distintos, depois de normalizar a árvore */
    int natomo;
    unsigned long asig[16];
    int aop[16], anega[16];
    struct tensor av[16];
};
static int le_expr(const char **p, struct arvore *a);

static int novo_no(struct arvore *a){
    if(a->n >= MAXNO) return -1;
    memset(&a->no[a->n], 0, sizeof a->no[0]);
    return a->n++;
}
/* expressão numérica com PRODUTO e parênteses:
 *     num   := termo (('+'|'-') termo)*
 *     termo := fator ('*' fator)*
 *     fator := '(' num ')' | inteiro | coluna
 * Contrai enquanto lê: soma acumula na casa, produto multiplica os tensores. Cada parêntese
 * que entra num produto vira uma POSIÇÃO do tensor — é a regra de tools/tensor.c. */
static int le_soma(const char **p, struct tensor *t);

static int le_fator_num(const char **p, struct tensor *t){
    pula(p);
    if(**p == '('){
        (*p)++;
        if(!le_soma(p, t)) return 0;
        pula(p);
        if(**p != ')') return 0;
        (*p)++;
        return 1;
    }
    *t = ten_zero();
    if(isdigit((unsigned char)**p)){
        long k;
        if(!numero(p, &k)) return 0;
        ten_const(t, k);
        const char *volta = *p;                  /* é fração? só se vier / e depois número */
        pula(p);
        if(**p == '/'){
            const char *ap = *p + 1;
            long q;
            if(numero(&ap, &q) && q != 0){
                *p = ap;
                t->den = q;
                ten_reduz(t);
                return 1;
            }
        }
        *p = volta;
        return 1;
    }
    if(isalpha((unsigned char)**p)){
        char nome[64];
        if(!ident(p, nome, sizeof nome)) return 0;
        int col = (int)(nome[0] - 'a');
        if(col < 0 || col >= NCOL) return 0;
        ten_var(t, col);                           /* o símbolo col+1 é a coluna */
        return 1;
    }
    return 0;
}
static int le_produto(const char **p, struct tensor *t){
    if(!le_fator_num(p, t)) return 0;
    while(1){
        pula(p);
        if(**p == '*'){
            (*p)++;
            struct tensor u, r;
            if(!le_fator_num(p, &u)) return 0;
            if(!ten_mul(&r, t, &u)) return 0;      /* passou do grau máximo */
            *t = r;
            continue;
        }
        /* A DIVISÃO POR CONSTANTE. Com o tensor sobre ℚ ela é a recíproca, e nada mais: o
         * numerador vira denominador. Só se divide por CONSTANTE — dividir por uma coluna
         * daria uma função que não é polinómio, e o tensor não a representa; então recusa-se
         * em voz alta, que é melhor que aceitar e responder outra coisa.
         *
         * Antes desta linha o `/` depois de uma coluna era SALTADO em silêncio: `a / 2 > 2`
         * lia-se como outra coisa e devolvia seis linhas onde devia devolver duas. Silêncio
         * assim é o pior defeito que um banco pode ter. */
        if(**p == '/'){
            (*p)++;
            struct tensor u;
            if(!le_fator_num(p, &u)) return 0;
            if(!ten_constante(u) || u.c[0] == 0){
                printf("erro: só se divide por constante não-nula (o tensor é polinomial)\n");
                return 0;
            }
            long num = u.c[0], den = u.den ? u.den : 1;
            t->den *= (num < 0 ? -num : num);      /* recíproca: numerador vira denominador */
            for(int i = 0; i < NMON; i++) t->c[i] *= (num < 0 ? -den : den);
            ten_reduz(t);
            continue;
        }
        break;
    }
    return 1;
}
static int le_soma(const char **p, struct tensor *t){
    pula(p);
    int sinal = 1;
    if(**p == '-'){ sinal = -1; (*p)++; }
    else if(**p == '+'){ (*p)++; }
    if(!le_produto(p, t)) return 0;
    if(sinal < 0) *t = ten_soma(ten_zero(), *t, -1);
    while(1){
        pula(p);
        int s2;
        if(**p == '+') s2 = 1;
        else if(**p == '-') s2 = -1;
        else break;
        (*p)++;
        struct tensor u;
        if(!le_produto(p, &u)) return 0;
        *t = ten_soma(*t, u, s2);
    }
    return 1;
}
static int le_num(const char **p, struct tensor *v){ return le_soma(p, v); }

static int le_fator(const char **p, struct arvore *a){
    pula(p);
    /* O '(' é AMBÍGUO: pode abrir um grupo booleano — (a=3 OR b>5) — ou um fator numérico —
     * (a+b)*(a-b) > 0. Tenta-se primeiro a COMPARAÇÃO; se não fechar, volta-se ao ponto de
     * partida e lê-se como grupo. Sem este retrocesso, `(a+b)*(a-b) > 0` era lido como grupo,
     * falhava no ')' e a cláusula inteira caía fora. */
    const char *salvo = *p;
    int nsalvo = a->n;
    struct tensor L, R;
    if(le_num(p, &L)){
        pula(p);
        if(**p=='!' || **p=='<' || **p=='>' || **p=='=') goto tem_comparacao;
    }
    *p = salvo; a->n = nsalvo;
    if(**p == '('){
        (*p)++;
        int e = le_expr(p, a);
        pula(p);
        if(**p != ')') return -1;
        (*p)++;
        return e;
    }
    return -1;
tem_comparacao:;
    int op = 0, nega = 0;
    if(**p == '!' && (*p)[1] == '=')        { op = '='; nega = 1; *p += 2; }
    else if(**p == '<' && (*p)[1] == '=')   { op = '>'; nega = 1; *p += 2; }   /* ≤ é não-> */
    else if(**p == '>' && (*p)[1] == '=')   { op = '<'; nega = 1; *p += 2; }   /* ≥ é não-< */
    else if(**p == '=')                     { op = '='; (*p)++; }
    else if(**p == '<')                     { op = '<'; (*p)++; }
    else if(**p == '>')                     { op = '>'; (*p)++; }
    else return -1;
    if(!le_num(p, &R)) return -1;

    int i = novo_no(a); if(i < 0) return -1;
    struct no *n = &a->no[i];
    n->tipo = NO_COND; n->op = op; n->nega = nega;
    n->v = ten_soma(L, R, -1);                     /* L op R  ⟺  (L−R) op 0 */

    /* CANONIZAR o par (vetor, operador): sem isto, `b > 20` e `20 < b` são o mesmo fato
     * escrito com vetores opostos, e emitiriam bytecode diferente.
     *   v < 0  ⟺  (−v) > 0        — logo o '<' desaparece
     *   v = 0  ⟺  (−v) = 0        — logo o sinal do '=' é livre, e fixa-se */
    if(op == '<'){
        n->v = ten_soma(ten_zero(), n->v, -1);
        n->op = '>';
    } else if(op == '='){
        long primeiro = 0;
        for(int i = 1; i < NMON && !primeiro; i++) if(n->v.c[i]) primeiro = n->v.c[i];
        if(!primeiro) primeiro = n->v.c[0];
        if(primeiro < 0) n->v = ten_soma(ten_zero(), n->v, -1);
    }
    op = n->op;

    /* CONTRAÇÃO decide na compilação: se não sobrou variável, a resposta já se sabe.
     * E se der falso, isso não é erro do cliente — é uma CONDIÇÃO DE PARADA. */
    if(ten_constante(n->v)){
        long d = n->v.c[0];
        int vale = (op == '=') ? (d == 0) : (op == '<') ? (d < 0) : (d > 0);
        if(nega) vale = !vale;
        n->decidido = 1; n->valor = vale;
    }
    return i;
}
static int le_termo(const char **p, struct arvore *a){
    int e = le_fator(p, a);
    if(e < 0) return -1;
    while(palavra(p, "AND")){
        int d = le_fator(p, a);
        if(d < 0) return -1;
        int i = novo_no(a); if(i < 0) return -1;
        a->no[i].tipo = NO_AND; a->no[i].esq = e; a->no[i].dir = d;
        e = i;
    }
    return e;
}
static int le_expr(const char **p, struct arvore *a){
    int e = le_termo(p, a);
    if(e < 0) return -1;
    while(palavra(p, "OR")){
        int d = le_termo(p, a);
        if(d < 0) return -1;
        int i = novo_no(a); if(i < 0) return -1;
        a->no[i].tipo = NO_OR; a->no[i].esq = e; a->no[i].dir = d;
        e = i;
    }
    return e;
}
/* ---------------- A CONTRAÇÃO ----------------
 * Antes de emitir, a árvore é NORMALIZADA — do mesmo jeito que expressao.c contrai uma
 * expressão num tensor: quem é equivalente vira o MESMO objeto, e não duas coisas que uma
 * regra depois iguala. Três coisas acontecem, e nenhuma é reescrita ad hoc:
 *
 *   (1) cada condição vira um ÁTOMO com assinatura (coluna, operador, negação, constante).
 *       Átomos iguais são o MESMO átomo — logo `a=3 OR a=3` tem um átomo, não dois.
 *   (2) os filhos de AND/OR são ORDENADOS pela assinatura. Como as duas operações são
 *       comutativas, a ordem escrita é acidente; ordenar é escolher o representante.
 *   (3) A op A colapsa em A (idempotência), que cai sozinha depois de (1) e (2).
 *
 * O efeito medível: WHERE equivalentes escritos de formas diferentes emitem o MESMO bytecode,
 * byte a byte. O programa passa a depender da classe, não da escrita.
 */
static unsigned long mistura(unsigned long h, unsigned long v){
    h ^= v + 0x9e3779b97f4a7c15UL + (h << 6) + (h >> 2);
    return h;
}
static unsigned long sig_de(struct arvore *a, int i, unsigned long *cache){
    if(cache[i]) return cache[i];
    struct no *n = &a->no[i];
    unsigned long h;
    if(n->tipo == NO_COND){
        h = mistura(1469598103934665603UL, (unsigned long)n->op);
        h = mistura(h, (unsigned long)n->nega);
        h = mistura(h, (unsigned long)n->decidido);
        h = mistura(h, (unsigned long)n->valor);
        for(int i = 0; i < NMON; i++) if(n->v.c[i]) h = mistura(mistura(h,(unsigned long)i),
                                                                (unsigned long)n->v.c[i]);
    } else {
        unsigned long s1 = sig_de(a, n->esq, cache), s2 = sig_de(a, n->dir, cache);
        if(s1 > s2){ int t = n->esq; n->esq = n->dir; n->dir = t;   /* (2) ordena */
                     unsigned long ts = s1; s1 = s2; s2 = ts; }
        h = mistura(mistura((unsigned long)n->tipo + 7, s1), s2);
    }
    cache[i] = h ? h : 1;
    return cache[i];
}
static int normaliza(struct arvore *a, int i, unsigned long *cache){
    struct no *n = &a->no[i];
    if(n->tipo == NO_COND) return i;
    n->esq = normaliza(a, n->esq, cache);
    n->dir = normaliza(a, n->dir, cache);
    memset(cache, 0, sizeof(unsigned long) * MAXNO);
    unsigned long s1 = sig_de(a, n->esq, cache), s2 = sig_de(a, n->dir, cache);
    if(s1 == s2) return n->esq;                                     /* (3) A op A = A */
    /* (2) ordena TAMBÉM aqui: dentro de sig_de a troca só acontece ao descer num nó
     * composto, então a raiz — e todo nó visto de cima — ficava por ordenar. Era por isso
     * que `a=3 AND b>20` e `b>20 AND a=3` davam bytecode diferente com o mesmo resultado. */
    if(s1 > s2){ int t = n->esq; n->esq = n->dir; n->dir = t; }
    return i;
}
static void junta_atomos(struct arvore *a, int i, unsigned long *cache){
    struct no *n = &a->no[i];
    if(n->tipo != NO_COND){ junta_atomos(a, n->esq, cache); junta_atomos(a, n->dir, cache); return; }
    unsigned long h = sig_de(a, i, cache);
    for(int j = 0; j < a->natomo; j++)
        if(a->asig[j] == h){ n->atomo = j; return; }                 /* (1) já existe */
    if(a->natomo >= 16){ n->atomo = 0; return; }
    int j = a->natomo++;
    a->asig[j] = h; a->aop[j] = n->op; a->anega[j] = n->nega; a->av[j] = n->v;
    n->atomo = j;
}
static void contrai_arvore(struct arvore *a){
    unsigned long cache[MAXNO];
    memset(cache, 0, sizeof cache);
    a->raiz = normaliza(a, a->raiz, cache);
    memset(cache, 0, sizeof cache);
    a->natomo = 0;
    junta_atomos(a, a->raiz, cache);
}

/* Três respostas, e a do meio é a que faltava:
 *    1  há WHERE e ele analisa      → filtra
 *    0  não há WHERE                → varre tudo, e é o que o cliente pediu
 *   −1  há WHERE e ele NÃO analisa  → RECUSA
 *
 * Antes só havia 1 e 0, e o WHERE quebrado caía no 0 — isto é, um filtro que o compilador não
 * entendeu devolvia a TABELA INTEIRA. Num banco isso é o defeito mais caro que existe: o
 * cliente pede um recorte, recebe tudo, e nada avisa. */
static int le_where(const char **p, struct arvore *a){
    memset(a, 0, sizeof *a);
    if(!palavra(p, "WHERE")) return 0;              /* não há WHERE: varrer tudo é o pedido */
    a->raiz = le_expr(p, a);
    if(a->raiz < 0) return -1;                      /* há, e não analisa: recusar */
    contrai_arvore(a);                 /* contrai ANTES de emitir */
    return 1;
}

static void emit_teste(unsigned sc, int cmp_op, long k, unsigned destino, unsigned kslot){
    /* cada condição tem o SEU slot de constante — nada compartilhado entre compilar e executar */
    Word w; w.total = k; w.e = 0;
    mem_grava(kslot, w);

    emit_copia(S_ZERO, destino);
    if(cmp_op == '='){
        emit_slot(OP_LOAD, sc);
        emit_slot(OP_LOAD, kslot);
        emit1(OP_SUB);
        emit_slot(OP_STORE, S_TMP);
        emit_slot(OP_LOAD, S_TMP);
        emit_slot(OP_LOAD, S_ZERO);
        emit1(OP_CMP);
        emit1(OP_JZ); emit1(2);
    } else {
        if(cmp_op == '<'){ emit_slot(OP_LOAD, kslot); emit_slot(OP_LOAD, sc); }
        else             { emit_slot(OP_LOAD, sc); emit_slot(OP_LOAD, kslot); }
        emit1(OP_SUB);
        emit_slot(OP_STORE, S_TMP);
        emit_slot(OP_LOAD, S_TMP);
        emit_slot(OP_LOAD, S_MASK);
        emit1(OP_AND);
        emit_slot(OP_STORE, S_TMP);
        emit_slot(OP_LOAD, S_TMP);
        emit_slot(OP_LOAD, S_ZERO);
        emit1(OP_CMP);
        emit1(OP_JNZ); emit1(2);
    }
    emit1(OP_JMP);
    unsigned pos = pc_emit; emit1(0);
    unsigned ini = pc_emit;
    emit_copia(S_UM, destino);
    unsigned char rel = (unsigned char)(pc_emit - ini);
    pwrite(fprog, &rel, 1, (off_t)pos);
}

/* MULTIPLICAÇÃO POR CONSTANTE, NAS COORDENADAS DO REI.
 *
 * A ISA não tem MUL, e a versão anterior fazia soma repetida |c| vezes — linear no VALOR, e
 * com um `if(n > CMAX) n = CMAX` que TRUNCAVA o coeficiente em silêncio: um WHERE com 20*a
 * virava 8*a e devolvia a resposta errada sem avisar. Os dois problemas caem juntos.
 *
 * A ISA já tem a multiplicação pelo rei: GOLD é cifra_an(w,1) = (total + e, total), que é o
 * deslocamento (a,b) ↦ (a+b, a) medido em coroa.c §A5. Partindo de (x, 0), aplicar GOLD k−1
 * vezes dá (F(k)·x, F(k−1)·x): multiplicar por Fibonacci é DESLOCAR, e custa k opcodes.
 *
 * E coroa.c §A3 diz o resto: todo inteiro é soma de Fibonacci NÃO CONSECUTIVOS, de um único
 * jeito. Logo
 *      n·x = Σ F(k_i)·x = Σ GOLD^(k_i − 1) (x)
 * e o custo passa de n para o número de dígitos de Zeckendorf, que é ~log_φ(n). Nada foi
 * inventado: o opcode já estava lá, e as coordenadas são as do rei. */
static void emit_mul_zeck(unsigned acc, unsigned termo, long n, int soma, unsigned tmp){
    long fib[92]; int nf = 2;
    fib[0] = 1; fib[1] = 2;                       /* F(2), F(3), … — a base de Zeckendorf */
    while(fib[nf-1] <= n/2 + 1 && nf < 90){ fib[nf] = fib[nf-1] + fib[nf-2]; nf++; }
    long r = n;
    for(int i = nf-1; i >= 0 && r > 0; i--){
        if(fib[i] > r) continue;
        r -= fib[i];
        /* tmp ← termo, com o .e limpo, e depois i deslocamentos */
        emit_slot(OP_LOAD, termo);
        emit_slot(OP_LOAD, S_MT);
        emit1(OP_AND);
        emit_slot(OP_STORE, tmp);
        /* GOLD^k parte de (x,0) e dá total = F(k+1)·x. Como fib[i] = F(i+2), o número de
         * deslocamentos é i+1, e NÃO i — errar isto acerta só quando o coeficiente é 1. */
        for(int t = 0; t <= i; t++){
            emit_slot(OP_LOAD, tmp);
            emit1(OP_GOLD);
            emit_slot(OP_STORE, tmp);
        }
        emit_slot(OP_LOAD, tmp);                  /* limpa o .e que o deslocamento deixou */
        emit_slot(OP_LOAD, S_MT);
        emit1(OP_AND);
        emit_slot(OP_STORE, tmp);
        if(soma){ emit_slot(OP_LOAD, acc); emit_slot(OP_LOAD, tmp); emit1(OP_ADD); }
        else    { emit_slot(OP_LOAD, tmp); emit_slot(OP_LOAD, acc); emit1(OP_SUB); }
        emit_slot(OP_STORE, acc);
    }
}

/* os átomos distintos, avaliados UMA vez por linha.
 *
 * O átomo já vem CONTRAÍDO num vetor: c0 + Σ c_i·x_i, comparado com 0. Se o vetor não tem
 * variável, nada é emitido — o valor já se sabe. Se tem, a forma linear é montada no metal:
 * a ISA não tem multiplicação, então c_i·x_i é soma repetida |c_i| vezes, e o compilador
 * conhece c_i, logo o laço é desenrolado. */
/* multiplica dois slots em tempo de EXECUÇÃO: dest = X · Y.
 *
 * A ISA não tem MUL, e aqui o multiplicador não é constante conhecida (é o valor de outra
 * coluna) — logo não dá para desenrolar. Vira laço: soma X a si mesmo |Y| vezes, com o passo
 * e o incremento escolhidos pelo SINAL de Y. Custa |Y| voltas, e isso é propriedade do metal,
 * não do compilador: multiplicar sem multiplicador custa contar. */
static void emit_mul(unsigned dest, unsigned X, unsigned Y, unsigned base){
    unsigned cnt = base, passo = base+1, delta = base+2, tmp = base+3;
    emit_copia(S_ZERO, dest);
    emit_copia(Y, cnt);

    /* o sinal de cnt escolhe o par (passo, delta) */
    emit_slot(OP_LOAD, cnt);
    emit_slot(OP_LOAD, S_MASK);
    emit1(OP_AND);
    emit_slot(OP_STORE, tmp);
    emit_slot(OP_LOAD, tmp);
    emit_slot(OP_LOAD, S_ZERO);
    emit1(OP_CMP);                                   /* FL_ZERO sse cnt ≥ 0 */
    emit1(OP_JZ);
    unsigned pos_pos = pc_emit; emit1(0);
    /* cnt < 0 : passo = −X, delta = +1 */
    unsigned ini_neg = pc_emit;
    emit_slot(OP_LOAD, X); emit_slot(OP_LOAD, S_ZERO); emit1(OP_SUB);
    emit_slot(OP_STORE, passo);                       /* R = 0 − X */
    emit_copia(S_UM, delta);
    emit1(OP_JMP);
    unsigned pos_fim_neg = pc_emit; emit1(0);
    unsigned ini_pos = pc_emit;
    /* cnt ≥ 0 : passo = +X, delta = −1 */
    emit_copia(X, passo);
    emit_slot(OP_LOAD, S_UM); emit_slot(OP_LOAD, S_ZERO); emit1(OP_SUB);
    emit_slot(OP_STORE, delta);                       /* R = 0 − 1 */
    unsigned depois = pc_emit;
    { unsigned char r = (unsigned char)(ini_pos - ini_neg);   pwrite(fprog, &r, 1, (off_t)pos_pos); }
    { unsigned char r = (unsigned char)(depois - ini_pos);    pwrite(fprog, &r, 1, (off_t)pos_fim_neg); }

    /* o laço: enquanto cnt != 0 { dest += passo ; cnt += delta } */
    unsigned topo = pc_emit;
    emit_slot(OP_LOAD, cnt);
    emit_slot(OP_LOAD, S_ZERO);
    emit1(OP_CMP);                                    /* FL_ZERO sse cnt == 0 */
    emit1(OP_JZ);
    unsigned pos_sai = pc_emit; emit1(0);
    unsigned corpo = pc_emit;
    emit_slot(OP_LOAD, dest); emit_slot(OP_LOAD, passo); emit1(OP_ADD); emit_slot(OP_STORE, dest);
    emit_slot(OP_LOAD, cnt);  emit_slot(OP_LOAD, delta); emit1(OP_ADD); emit_slot(OP_STORE, cnt);
    emit1(OP_JMP);
    unsigned pos_volta = pc_emit; emit1(0);
    unsigned fim = pc_emit;
    { unsigned char r = (unsigned char)(int)((int)topo - (int)pos_volta - 1);
      pwrite(fprog, &r, 1, (off_t)pos_volta); }
    { unsigned char r = (unsigned char)(fim - corpo); pwrite(fprog, &r, 1, (off_t)pos_sai); }
}

/* os átomos distintos, avaliados UMA vez por linha.
 *
 * O átomo vem contraído num TENSOR simétrico de grau ≤ 2. A forma monta-se no metal:
 *   grau 0  a constante, direto;
 *   grau 1  coeficiente CONHECIDO em compilação  → soma repetida, desenrolada;
 *   grau 2  x_i·x_j com os dois vindos da linha  → laço (emit_mul), porque não há MUL.
 * E a comparação é sempre contra ZERO — a contração já passou tudo para um lado. */
static void emit_atomos(const struct arvore *a, long linha, long ncols){
    for(int j = 0; j < a->natomo; j++){
        unsigned dest = S_COND + (unsigned)j;
        unsigned acc  = S_LIN + (unsigned)(j*12);
        unsigned prod = acc + 1, prod2 = acc + 2, base = acc + 3;  /* prod, prod2, cnt… */

        if(ten_constante(a->av[j])) continue;         /* decidido: nada se emite */

        Word w; w.total = a->av[j].c[0]; w.e = 0;
        mem_grava(S_K + (unsigned)j, w);
        emit_copia(S_K + (unsigned)j, acc);

        /* percorre os monômios do multi-índice. Grau 0 já entrou; grau 1 tem coeficiente
         * conhecido em compilação (soma desenrolada); grau ≥ 2 precisa multiplicar colunas
         * em tempo de execução, e a ISA não tem MUL — vira cadeia de emit_mul. */
        for(int cod = 1; cod < NMON; cod++){
            long c = a->av[j].c[cod];
            if(!c) continue;
            int d[KGRAU]; mi_de(cod, d);
            if(mi_cod(d) != cod) continue;                 /* só os representantes ordenados */
            int g = mi_grau(cod), fora = 0;
            for(int t = 0; t < KGRAU; t++) if(d[t] && d[t]-1 >= ncols) fora = 1;
            if(fora) continue;
            long n = c < 0 ? -c : c;      /* nada de truncar: o coeficiente entra inteiro */

            unsigned termo;
            if(g == 1){
                int col = 0;
                for(int t = 0; t < KGRAU; t++) if(d[t]) col = d[t]-1;
                termo = S_LINHAS + (unsigned)(linha*ncols + col);
            } else {
                /* a cadeia do produto: prod = x_{i1}; prod = prod · x_{i2}; … */
                int prim = 1;
                for(int t = 0; t < KGRAU; t++){
                    if(!d[t]) continue;
                    unsigned sc = S_LINHAS + (unsigned)(linha*ncols + (d[t]-1));
                    if(prim){ emit_copia(sc, prod); prim = 0; }
                    else     { emit_mul(prod2, prod, sc, base); emit_copia(prod2, prod); }
                }
                termo = prod;
            }
            emit_mul_zeck(acc, termo, n, c > 0, acc + 4);
        }
        emit_teste(acc, a->aop[j], 0, dest, S_KZ + (unsigned)j);
        if(a->anega[j]){
            emit_slot(OP_LOAD, dest);
            emit_slot(OP_LOAD, S_UM);
            emit1(OP_XOR);
            emit_slot(OP_STORE, dest);
        }
    }
}
/* percorre a árvore em pós-ordem; cada nó deixa 0 ou 1 no seu slot */
static void emit_no(const struct arvore *a, int i, long linha, long ncols, unsigned dest){
    const struct no *n = &a->no[i];
    if(n->tipo == NO_COND){
        if(n->decidido) emit_copia(n->valor ? S_UM : S_ZERO, dest);  /* decidido na compilação */
        else            emit_copia(S_COND + (unsigned)n->atomo, dest);
        return;
    }
    unsigned de = dest + 2, dd = dest + 34;            /* dois ramos, slots afastados */
    emit_no(a, n->esq, linha, ncols, de);
    emit_no(a, n->dir, linha, ncols, dd);
    emit_slot(OP_LOAD, de);
    emit_slot(OP_LOAD, dd);
    emit1(n->tipo == NO_AND ? OP_AND : OP_OR);         /* o AND/OR do SQL É o da ISA */
    emit_slot(OP_STORE, dest);
}

static void emit_linha(long i, long ncols, const struct arvore *a, int tem_where,
                       int acao, int col_set)
{
    if(tem_where){
        emit_atomos(a, i, ncols);                   /* cada átomo distinto, uma vez só */
        emit_no(a, a->raiz, i, ncols, S_EXPR);
        emit_copia(S_EXPR, S_ACC);
    } else {
        emit_copia(S_UM, S_ACC);
    }

    emit_slot(OP_LOAD, S_ACC);
    emit_slot(OP_LOAD, S_VIVO + (unsigned)i);
    emit1(OP_AND);
    emit_slot(OP_STORE, S_ACC);

    emit_slot(OP_LOAD, S_ACC);
    emit_slot(OP_LOAD, S_ZERO);
    emit1(OP_CMP);
    emit1(OP_JZ);
    unsigned pos = pc_emit; emit1(0);
    unsigned ini = pc_emit;
    /* A varredura NÃO grava mais o efeito: ela só MARCA. O bitmap de casamento passa a ser o
     * diário de intenção, e quem aplica é a fase 3, depois do compromisso. Assim a queda no
     * meio da varredura não deixa metade das linhas mudadas. */
    (void)col_set;
    emit_copia(S_UM, S_MATCH + (unsigned)i);
    emit_slot(OP_LOAD, S_CONTA);
    emit_slot(OP_LOAD, S_UM);
    emit1(OP_ADD);
    emit_slot(OP_STORE, S_CONTA);
    unsigned char rel = (unsigned char)(pc_emit - ini);
    pwrite(fprog, &rel, 1, (off_t)pos);
}

/* prepara as constantes e devolve o catálogo */
static void prepara(long v){
    Word w = {0,0};
    mem_grava(S_ZERO, w);
    mem_grava(S_CONTA, w);
    w.total = 1; w.e = 0;                 mem_grava(S_UM, w);
    w.total = v; w.e = 0;                 mem_grava(S_V, w);
    w.total = (long)(1UL << 63); w.e = 0; mem_grava(S_MASK, w);   /* o bit de sinal */
}


/* FASE 3: aplica o que o diário mandou. Programa próprio, desenrolado, sem indireção — o
 * bitmap diz quais linhas, e o compilador já conhece cada índice.
 *
 * É IDEMPOTENTE de propósito: escreve valores absolutos, nunca incrementos. Por isso pode ser
 * repetida na abertura sem estragar nada, que é o que faz o redo funcionar. */
static long aplica_diario(long ncols, long nrows, int acao, int col_set){
    pc_emit = 0;
    for(long i = 0; i < nrows; i++){
        emit_slot(OP_LOAD, S_MATCH + (unsigned)i);
        emit_slot(OP_LOAD, S_ZERO);
        emit1(OP_CMP);
        emit1(OP_JZ);
        unsigned pos = pc_emit; emit1(0);
        unsigned ini = pc_emit;
        if(acao == ACAO_SET) emit_copia(S_V,    S_LINHAS + (unsigned)(i*ncols + col_set));
        else                 emit_copia(S_ZERO, S_VIVO  + (unsigned)i);
        unsigned char rel = (unsigned char)(pc_emit - ini);
        pwrite(fprog, &rel, 1, (off_t)pos);
    }
    emit1(OP_HALT);
    return rodar(pc_emit);
}

/* Na abertura: se o diário ficou aberto, uma queda apanhou a base entre o compromisso e o
 * fim da aplicação. Refaz-se, e só então se fecha o diário. */
static void refaz_diario(void){
    Word d = mem_le(S_DIA);
    if(d.total == 0) return;
    Word cat = mem_le(S_CAT);
    printf("-- diário aberto: refazendo %s\n", d.total == ACAO_SET+1 ? "um UPDATE" : "um DELETE");
    aplica_diario(cat.total, cat.e, (int)d.total - 1, (int)d.e);
    barreira();
    Word z = {0,0}; mem_grava(S_DIA, z);
    barreira();
}

static int varre(const char *resto, int acao){
    const char *p = resto;
    char nome[64], alvo[64];
    long v = 0;
    int col_set = 0, tem_where;
    struct arvore cl;

    if(acao == ACAO_MARCA){
        char cols[64];
        if(!ident(&p, cols, sizeof cols)) return 0;
        if(!palavra(&p, "FROM")) return 0;
        if(!ident(&p, nome, sizeof nome)) return 0;
    } else if(acao == ACAO_SET){
        if(!ident(&p, nome, sizeof nome)) return 0;
        if(!palavra(&p, "SET")) return 0;
        if(!ident(&p, alvo, sizeof alvo)) return 0;
        pula(&p); if(*p != '=') return 0; p++;
        if(!numero(&p, &v)) return 0;
        col_set = (int)(alvo[0] - 'a'); if(col_set < 0) col_set = 0;
    } else {
        if(!palavra(&p, "FROM")) return 0;
        if(!ident(&p, nome, sizeof nome)) return 0;
    }
    tem_where = le_where(&p, &cl);
    if(tem_where < 0){
        printf("erro: o WHERE não foi entendido — a consulta é RECUSADA, e nada é devolvido\n");
        return 0;
    }

    Word cat = mem_le(S_CAT);
    long ncols = cat.total, nrows = cat.e;
    if(nrows <= 0){ printf("(vazio)\n"); return 1; }

    /* GUARDA DO VALOR RACIONAL — e ONDE o denominador se perde, para quem retomar.
     *
     * A aritmética descarta o .e em silêncio: emit_mul_zeck faz AND com S_MT (máscara
     * {total = todos os bits, e = 0}) antes de cada deslocamento, e é isso que faz "a = 3"
     * funcionar com denominador 1 — o .e nunca chega à comparação. Com denominador ≠ 1 a
     * conta ficaria errada, e por isso se recusa.
     *
     * E o fundo do problema tem nome, que o Aarão deu: comparar gato com cachorro. A coluna
     * passou a viver numa régua (par p/q) e a constante continuou na antiga (.e = 0). Para
     * haver igualdade os dois lados têm de estar na MESMA régua — ou ambos crus, ou ambos
     * cifrados. Enquanto um for magnitude e o outro coordenada, não fecha, e mascarar o .e é
     * só esconder a diferença em vez de a resolver.
     *
     * GUARDA DO VALOR RACIONAL.
     *
     * A coluna já guarda p/q no par (total, e). Mas a ULA soma e compara COMPONENTE A
     * COMPONENTE — ela não sabe que aquilo é uma fração. Uma condição sobre coluna racional
     * lê só o numerador e responde errado: com 3/4 guardado, "a > 1" devolvia a linha.
     *
     * Resposta errada em silêncio é o pior defeito que este banco pode ter, e já caiu uma vez
     * hoje (o WHERE que não analisava devolvia a tabela inteira). Então recusa-se.
     *
     * O que falta para aceitar: emitir a multiplicação cruzada em tempo de execução —
     * num·q OP k·den, com emit_mul —, que é a mesma conta do racional_pg.c §Q4 posta no metal.
     * É trabalho, não é impossível, e fica dito como o que falta. */
    if(tem_where > 0){
        for(long i = 0; i < nrows; i++) for(long j = 0; j < ncols; j++)
            if(mem_le(S_LINHAS + (unsigned)(i*ncols + j)).e > 1){
                printf("erro: há valor RACIONAL guardado (linha %ld, coluna %c) e a comparação\n"
                       "      ainda lê só o numerador — a consulta é RECUSADA em vez de responder\n"
                       "      errado. Falta a multiplicação cruzada no metal (racional_pg.c §Q4).\n",
                       i, (char)('a'+j));
                return 0;
            }
    }

    prepara(v);
    Word z = {0,0};
    for(long i = 0; i < nrows; i++) mem_grava(S_MATCH + (unsigned)i, z);

    /* UM molde só, o da linha 0 — e depois a PA anda com ele por todas as linhas. */
    pc_emit = 0; nrel = 0; rel_ncols = ncols;
    emit_linha(0, ncols, &cl, tem_where, acao, col_set);
    emit1(OP_HALT);
    rel_ncols = 0;
    long passos = 0;
    for(long i = 0; i < nrows; i++){ rel_anda(i); passos += rodar(pc_emit); }

    unsigned long soma = 1469598103934665603UL;
    for(unsigned q = 0; q < pc_emit; q++){ soma ^= prog_le(q); soma *= 1099511628211UL; }

    long achou = mem_le(S_CONTA).total;
    if(acao != ACAO_MARCA){
        /* o bitmap (o diário) já está no disco; agora o COMPROMISSO, e só depois o efeito. */
        barreira();
        trava_se_pedido(2);                       /* queda ANTES do compromisso: nada mudou */
        Word d; d.total = acao + 1; d.e = col_set;
        mem_grava(S_DIA, d);
        barreira();                               /* ← o ponto de compromisso */
        trava_se_pedido(3);                       /* queda DEPOIS: a abertura refaz */
        passos += aplica_diario(ncols, nrows, acao, col_set);
        barreira();
        trava_se_pedido(4);                       /* queda aqui: refaz de novo, e é idempotente */
        Word z2 = {0,0}; mem_grava(S_DIA, z2);
        barreira();
    }
    const char *nome_acao = acao == ACAO_MARCA ? "lida(s)" : (acao == ACAO_SET ? "atualizada(s)" : "apagada(s)");
    printf("-- %u bytes de ISA [%04lx], %d átomo(s), %ld passos, %ld linha(s) %s\n",
           pc_emit, soma & 0xFFFF, tem_where ? cl.natomo : 0, passos, achou, nome_acao);
    if(acao != ACAO_MARCA) return 1;
    for(long i = 0; i < nrows; i++){
        if(mem_le(S_MATCH + (unsigned)i).total == 0) continue;
        printf("   ");
        for(long j = 0; j < ncols; j++){
            Word c = mem_le(S_LINHAS + (unsigned)(i*ncols + j));
            if(c.e > 1) printf("%ld/%ld", c.total, c.e);      /* a classe, como veio */
            else        printf("%ld", c.total);
            if(j+1 < ncols) printf(" | ");
        }
        printf("\n");
    }
    return 1;
}

static int executa(const char *sql){
    const char *p = sql;
    if(palavra(&p, "CREATE")){ if(!palavra(&p, "TABLE")) return 0; return cria(p); }
    if(palavra(&p, "INSERT")) return insere(p);
    if(palavra(&p, "SELECT")) return varre(p, ACAO_MARCA);
    if(palavra(&p, "UPDATE")) return varre(p, ACAO_SET);
    if(palavra(&p, "DELETE")) return varre(p, ACAO_APAGA);
    printf("nao entendi: %s\n", sql);
    return 0;
}

static int abrir_base(const char *base){
    char m[512], g[512];
    snprintf(m, sizeof m, "%s.mem", base);
    snprintf(g, sizeof g, "%s.prog", base);
    fmem  = open(m, O_RDWR|O_CREAT, 0644);
    fprog = open(g, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fmem < 0 || fprog < 0) return 0;
    ftruncate(fmem, (off_t)(S_LINHAS + 65536) * SLOTSZ);
    refaz_diario();          /* antes de qualquer comando: fechar o que ficou aberto */
    return 1;
}
static void fechar_base(void){ if(fmem>=0){fsync(fmem);close(fmem);} if(fprog>=0){fsync(fprog);close(fprog);} }

int main(int argc, char **argv){
    if(argc >= 2 && !strcmp(argv[1], "teste")){
        const char *base = "/tmp/sql_teste";
        unlink("/tmp/sql_teste.mem"); unlink("/tmp/sql_teste.prog");
        if(!abrir_base(base)) return 2;
        printf("\n=== SQL NO METAL: compila para a ISA, memória no disco, sem RAM ==========\n\n");
        printf("$ CREATE TABLE t (a,b,c)\n"); executa("CREATE TABLE t (a,b,c)");
        printf("\n");
        const char *ins[5] = {
            "INSERT INTO t VALUES (7,10,20)",
            "INSERT INTO t VALUES (3,30,40)",
            "INSERT INTO t VALUES (7,50,60)",
            "INSERT INTO t VALUES (9,70,80)",
            "INSERT INTO t VALUES (3,90,99)" };
        for(int i = 0; i < 5; i++){ printf("$ %s\n", ins[i]); executa(ins[i]); }
        printf("\n$ SELECT * FROM t\n");  executa("SELECT * FROM t");

        printf("\n-- A CONTRAÇÃO NUMÉRICA: a expressão vira vetor, e o vetor decide.\n");
        printf("   ([xxxx] = soma do bytecode; escritas equivalentes têm de dar a mesma)\n");
        const char *pares[][2] = {
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE a + b - a > 20"},
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE b - 20 > 0"},
            {"SELECT * FROM t WHERE a = 3",         "SELECT * FROM t WHERE a + 0 = 3 + 0"},
            {"SELECT * FROM t WHERE b > 20",        "SELECT * FROM t WHERE 20 < b"},
        };
        for(unsigned q = 0; q < sizeof pares/sizeof pares[0]; q++){
            printf("\n$ %s\n", pares[q][0]); executa(pares[q][0]);
            printf("$ %s\n", pares[q][1]);   executa(pares[q][1]);
        }

        printf("\n-- expressão dos DOIS lados, com coeficiente:\n");
        printf("\n$ SELECT * FROM t WHERE 2*a + b > c\n");
        executa("SELECT * FROM t WHERE 2*a + b > c");
        printf("\n$ SELECT * FROM t WHERE b - a > 30\n");
        executa("SELECT * FROM t WHERE b - a > 30");

        printf("\n-- O COEFICIENTE NAS COORDENADAS DO REI: GOLD é o deslocamento.\n");
        printf("   Antes: soma repetida |c| vezes, com o coeficiente TRUNCADO em silêncio se\n");
        printf("   passasse de 8 — a resposta saía errada sem aviso. Agora c·x = Σ F(k)·x, e\n");
        printf("   cada F(k) é GOLD^(k−1): o opcode já estava na ISA (broca-so ula/cifra.c).\n\n");
        printf("   (cada linha diz os bytes; a soma repetida gastaria 10 por unidade)\n");
        {
            long cs[] = {1, 2, 5, 13, 34, 100, 1000, 100000};
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                char buf[128];
                snprintf(buf, sizeof buf, "SELECT * FROM t WHERE a * %ld = 0", cs[q]);
                printf("$ c = %-7ld  (soma repetida gastaria %ld bytes)\n", cs[q], cs[q]*10);
                executa(buf);
            }
        }
        printf("\n   O bytecode cresce com o número de dígitos de Zeckendorf (~log_φ c), e não\n");
        printf("   com o valor: de c=1000 para c=100000 o valor faz cem vezes e o código não\n");
        printf("   chega a três.\n");

        printf("\n-- e a contração continua a identificar as escritas equivalentes:\n");
        printf("\n$ SELECT * FROM t WHERE a + a = 14\n");
        executa("SELECT * FROM t WHERE a + a = 14");
        printf("$ SELECT * FROM t WHERE a * 2 = 14\n");
        executa("SELECT * FROM t WHERE a * 2 = 14");
        printf("$ SELECT * FROM t WHERE a * 12 - a * 12 = 0   (contrai a constante)\n");
        executa("SELECT * FROM t WHERE a * 12 - a * 12 = 0");

        printf("\n-- A BARREIRA DO BANCO: dado, fsync, ponteiro, fsync. E a queda no meio.\n");
        printf("   O INSERT era um programa só: as células e o catálogo sem nada entre eles.\n");
        printf("   Ordem de programa não é ordem no disco — sem fsync o catálogo pode chegar\n");
        printf("   ao prato antes das células, e a queda deixa o catálogo a contar uma linha\n");
        printf("   que não existe. Agora são duas fases com barreira, e mede-se a queda.\n\n");
        {
            Word c0 = mem_le(S_CAT);
            printf("   linhas antes da queda                    %ld\n", c0.e);
            pid_t f = fork();
            if(f == 0){                       /* o filho cai entre o dado e o ponteiro */
                trava_em = 1;
                insere(" INTO t VALUES (555,666,777)");
                _exit(0);
            }
            int st = 0; waitpid(f, &st, 0);
            printf("   o filho saiu com                         %d (9 = derrubado na barreira)\n",
                   WIFEXITED(st) ? WEXITSTATUS(st) : -1);
            Word c1 = mem_le(S_CAT);
            printf("   linhas depois da queda                   %ld\n", c1.e);
            printf("   %s\n", c0.e == c1.e ? "a linha caída é INVISÍVEL — nunca meia ✓"
                                             : "O CATÁLOGO MOVEU-SE SEM O DADO ✗");
            printf("\n   E a base segue escrevendo por cima do órfão:\n");
        }
        printf("$ INSERT INTO t VALUES (1,2,3)\n");
        executa("INSERT INTO t VALUES (1,2,3)");
        printf("$ SELECT * FROM t\n");
        executa("SELECT * FROM t");
        printf("\n   As duas quedas possíveis são as duas boas: antes do ponteiro, a linha não\n");
        printf("   existe e não faz mal; depois dele, está inteira. Nunca meia.\n");
        printf("   (_exit modela queda de PROCESSO — o que já foi ao núcleo sobrevive. Contra\n");
        printf("    queda de energia quem responde é o fsync, e é por isso que ele está lá.)\n");

        printf("\n-- A PA NO ENDEREÇO: um molde só, e a progressão varre a tabela.\n");
        printf("   A ISA não tem endereçamento indireto — o slot é imediato, e LOADS, que eu\n");
        printf("   esperava indireto, é LOAD com a cifra espectral (fui ver em broca-so). Por\n");
        printf("   isso a varredura era DESENROLADA: um bloco por linha, e o bytecode crescia\n");
        printf("   linearmente com a tabela — 147 bytes por linha, medidos.\n\n");
        printf("   Mas o endereço da linha É uma PA: linha i, coluna j mora em\n");
        printf("   S_LINHAS + i·ncols + j, passo constante. Então emite-se UM molde e ANDA-SE\n");
        printf("   com ele. O passo sai da faixa do slot, sem tocar em lugar de chamada nenhum.\n\n");
        printf("   linhas na tabela   bytes de ISA da consulta\n");
        {
            long antes = mem_le(S_CAT).e;
            printf("   %-18ld ", antes);
            executa("SELECT * FROM t WHERE a = 7");
        }
        printf("\n   Antes: O(linhas). Agora: O(1). Quem varre é a progressão, não o compilador —\n");
        printf("   e é a PA a fazer o trabalho para que ela foi feita.\n");

        printf("\n-- O DIÁRIO: o UPDATE e o DELETE são TUDO OU NADA.\n");
        printf("   O INSERT tinha ponteiro natural (o catálogo) e bastava a ordem. Estes não:\n");
        printf("   mudam em cima, e uma queda no meio da varredura deixaria metade das linhas\n");
        printf("   mudadas — que é pior que não ter mudado nenhuma, porque ninguém sabe qual.\n\n");
        printf("   Agora a varredura só MARCA (o bitmap é o diário), e depois vêm três passos:\n");
        printf("     1  o diário no disco          barreira\n");
        printf("     2  o COMPROMISSO no disco     barreira   ← daqui em diante, vai acontecer\n");
        printf("     3  a aplicação                barreira\n");
        printf("     4  o diário fechado           barreira\n");
        printf("   E a abertura confere: diário aberto quer dizer queda entre 2 e 4, e REFAZ.\n");
        printf("   A aplicação escreve valores absolutos, nunca incrementos — por isso refazer\n");
        printf("   duas vezes dá o mesmo que refazer uma.\n\n");
        {
            const int pontos[3] = {2, 3, 4};
            const char *quando[3] = {"antes do compromisso", "depois do compromisso",
                                     "depois de aplicar"};
            const char *espera[3] = {"nada mudou", "refez e completou", "refez, idempotente"};
            for(int q = 0; q < 3; q++){
                char mem[512], sv[512];
                snprintf(mem, sizeof mem, "%s.mem", base);
                snprintf(sv,  sizeof sv,  "%s.mem.sv", base);
                fechar_base();
                { char cmd[1100]; snprintf(cmd, sizeof cmd, "cp %s %s", mem, sv); if(system(cmd)){} }
                abrir_base(base);
                pid_t f = fork();
                if(f == 0){ trava_em = pontos[q]; varre(" t SET c = 4242 WHERE a >= 7", ACAO_SET); _exit(0); }
                int st = 0; waitpid(f, &st, 0);
                fechar_base(); abrir_base(base);
                long ncols = mem_le(S_CAT).total, nrows = mem_le(S_CAT).e, mudadas = 0;
                for(long i = 0; i < nrows; i++)
                    if(mem_le(S_LINHAS + (unsigned)(i*ncols + 2)).total == 4242) mudadas++;
                printf("   queda %-22s saiu %d   linhas com o valor novo: %ld   (%s)\n",
                       quando[q], WIFEXITED(st) ? WEXITSTATUS(st) : -1, mudadas, espera[q]);
                fechar_base();
                { char cmd[1100]; snprintf(cmd, sizeof cmd, "mv %s %s", sv, mem); if(system(cmd)){} }
                abrir_base(base);
            }
        }
        printf("\n   Antes do compromisso: nenhuma. Depois: TODAS as que casavam. Nunca um\n");
        printf("   pedaço — e quem fecha a conta é a abertura, sozinha, sem ninguém pedir.\n");

        printf("\n-- A PARADA: o cliente pode pedir o impossível, e isso é uma resposta.\n");
        printf("\n$ SELECT * FROM t WHERE a - a = 5\n");
        executa("SELECT * FROM t WHERE a - a = 5");
        printf("\n$ SELECT * FROM t WHERE a - a = 0\n");
        executa("SELECT * FROM t WHERE a - a = 0");
        printf("\n$ SELECT * FROM t WHERE b > 20 AND a - a = 5\n");
        executa("SELECT * FROM t WHERE b > 20 AND a - a = 5");
        printf("\n$ SELECT * FROM t WHERE b > 20 OR a - a = 0\n");
        executa("SELECT * FROM t WHERE b > 20 OR a - a = 0");

        printf("\n-- o resto continua de pé:\n");
        printf("\n$ UPDATE t SET c = 111 WHERE a >= 7 AND a != 9\n");
        executa("UPDATE t SET c = 111 WHERE a >= 7 AND a != 9");
        printf("$ SELECT * FROM t\n"); executa("SELECT * FROM t");
        printf("\n$ DELETE FROM t WHERE (a <= 3 AND b >= 90)\n");
        executa("DELETE FROM t WHERE (a <= 3 AND b >= 90)");
        printf("$ SELECT * FROM t\n"); executa("SELECT * FROM t");
        fechar_base();

        /* fecha e REABRE: o dado tem de estar no disco, não na memória do processo */
        printf("\n-- fechado. reabrindo o arquivo e consultando de novo:\n");
        char m[512]; snprintf(m, sizeof m, "%s.mem", base);
        fmem = open(m, O_RDWR);
        char g[512]; snprintf(g, sizeof g, "%s.prog", base);
        fprog = open(g, O_RDWR|O_CREAT|O_TRUNC, 0644);
        printf("$ SELECT * FROM t WHERE a = 7\n"); executa("SELECT * FROM t WHERE a = 7");
        fechar_base();
        printf("\n");
        return 0;
    }
    if(argc >= 3){
        if(!abrir_base(argv[1])){ perror("base"); return 2; }
        int r = executa(argv[2]);
        fechar_base();
        return r ? 0 : 1;
    }
    fprintf(stderr, "uso: sql teste | sql <base> \"<comando SQL>\"\n");
    return 2;
}
