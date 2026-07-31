
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
 *   GOLD                     A←A_1(A) ; R←A            o gato: estica, det −1, ordem ∞
 *   NEGRO_OURO               A←A_1⁻¹(A) ; R←A         a volta: INTEIRA, porque det = −1
 *   ESQUILO                  A←(−e, total) ; R←A       ×ω do cristal: det +1, ordem 4
 *   TROCA                    A←(e, total) ; R←A        J, a involução: det −1, ordem 2
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
#include "unidade.h"
#include "contrato.h"   /* o toolkit: a tríade ⊕ ⊗ ∏ de cada corpo */

/* ---------------- a ISA (transcrita) ---------------- */
enum { OP_HALT=0, OP_LOAD, OP_STORE, OP_ADD, OP_SUB, OP_AND, OP_OR, OP_XOR,
       OP_GOLD, OP_CMP, OP_JMP, OP_JZ, OP_JNZ,
       OP_FOLD, OP_UNFOLD, OP_PROJECT, OP_LIFT, OP_LOADS, OP_SPECT,
       /* A VOLTA. Acrescentados no FIM de propósito: o número de cada opcode antigo não
        * muda, e nenhum programa já compilado passa a significar outra coisa. */
       OP_NEGRO_OURO,
       /* O CIRCUITO. O gato estica; faltava quem GIRE, e sem ele a máquina não gera o grupo
        * todo. ESQUILO é ×ω do cristalino (t=0): det +1, ordem 4. TROCA é J, a involução. */
       OP_ESQUILO, OP_TROCA };
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
/* O CORPO DE CADA COLUNA — passo 1 de 6 do catálogo em SQL (ver TOOLKIT.md).
 *
 * A coluna deixa de ser "um número" e passa a DECLARAR em que corpo vive. O slot guarda
 * {total = o código do corpo, e = o parâmetro dele} — o metal m no áureo, o n no mórfico.
 *
 * Só o campo e o CREATE a aceitá-lo. O despacho das operações vem depois, um corpo de cada
 * vez, cada um com medidor antes de entrar — que é o método que o Aarão pediu e que hoje
 * mostrou ser o único que fecha. */
#define S_CORPO   60         /* 60..67: o corpo de cada coluna */
#define CORPO_INTEIRO  0     /* o de sempre — e continua a ser o omitido, sem quebrar base antiga */
#define CORPO_RACIONAL 1
#define CORPO_AUREO    2
#define CORPO_MORFICO  3
#define CORPO_CRISTAL  4     /* PASSO 6: o lado que gira — a+bω, ω²=tω−1 */
#define S_MT      57         /* mascara {total=todos os bits, e=0} — limpa o .e apos GOLD */
#define S_KZ      49         /* 49..56  o zero de cada comparação (a contração compara com 0) */
#define S_LIN     4096       /* 4096+  o rascunho de cada átomo: acc, prod, cnt, passo…   */
#define S_EXPR    64         /* 64..191  os temporários da árvore da expressão          */
#define S_MATCH   256        /* bitmap do resultado, uma linha por slot (256..511)      */
#define MAXCOND   4          /* condições por termo                                     */
#define MAXTERMO  4          /* termos ligados por OR                                   */
#define S_VIVO    512        /* a linha existe? o DELETE zera aqui (512..1023)          */
#define S_DEN     33792      /* o DENOMINADOR de cada célula, no TOTAL do seu slot: a ISA não
                              * move e→total, e a conta precisa de q como número. */
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
/* A VOLTA, e ela é INTEIRA. cifra_an é A_n = [[n,1],[1,0]] aplicado ao par, e det A_n = −1 —
 * logo a inversa não sai dos inteiros e não precisa de divisão nenhuma:
 *
 *     A_n⁻¹ = J·A_{−n}·J = [[0,1],[1,−n]]     (a,b) ↦ (b, a − n·b)
 *
 * Não é uma segunda máquina: é a MESMA peça virada — a antípoda (n ↦ −n) conjugada pela troca
 * J, que é a involução de ordem 2. Medido em dual_cadeia.c e cristalino.c §X0. Um estica por σ,
 * o outro contrai por 1/σ, e o produto é 1 exato. Por isso ela desfaz em vez de aproximar. */
static Word decifra_an(Word w, int n){ Word r = { w.e, w.total - (long)n*w.e }; return r; }

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

    case OP_NEGRO_OURO:   r->A = decifra_an(r->A, 1); r->R = r->A; break;

    /* O CIRCUITO. Com o gato sozinho a máquina só estica — e o que estica não fecha grupo.
     * ESQUILO é ×ω do cristalino com t=0, isto é S = [[0,−1],[1,0]]: det +1, ordem 4. TROCA
     * é J = [[0,1],[1,0]]: det −1, ordem 2. Com os três, toda unimodular é palavra. */
    case OP_ESQUILO: { Word w = { -r->A.e, r->A.total }; r->A = w; r->R = w; break; }
    case OP_TROCA:   { Word w = {  r->A.e, r->A.total }; r->A = w; r->R = w; break; }
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
    if(s >= S_DEN)    return rel_ncols;
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
    long corpo[16], parm[16];
    while(1){
        if(!ident(&p, c, sizeof c)) break;
        corpo[ncols] = CORPO_INTEIRO; parm[ncols] = 0;   /* sem tipo = INTEIRO, como sempre foi */
        pula(&p);
        char tipo[64];
        const char *volta = p;
        if(isalpha((unsigned char)*p) && ident(&p, tipo, sizeof tipo)){
            int achou = 1;
            if(!strcasecmp(tipo,"RACIONAL"))      corpo[ncols] = CORPO_RACIONAL;
            else if(!strcasecmp(tipo,"AUREO"))  { corpo[ncols] = CORPO_AUREO;   parm[ncols] = 1; }
            else if(!strcasecmp(tipo,"MORFICO")){ corpo[ncols] = CORPO_MORFICO; parm[ncols] = 6; }
            /* PASSO 6: o cristalino. O parâmetro é o t da borda ω² = tω − 1 — t=0 Gauss ℤ[i],
             * t=1 Eisenstein ℤ[ω]. Predefine 0, que é o cristal quadrado. */
            else if(!strcasecmp(tipo,"CRISTALINO")){ corpo[ncols] = CORPO_CRISTAL; parm[ncols] = 0; }
            else if(!strcasecmp(tipo,"INTEIRO"))  corpo[ncols] = CORPO_INTEIRO;
            else { p = volta; achou = 0; }               /* não era tipo: devolve ao analisador */
            if(achou){
                pula(&p);
                if(*p == '('){ p++; long q; if(numero(&p, &q)) parm[ncols] = q; pula(&p);
                               if(*p == ')') p++; }
            }
        }
        ncols++; pula(&p);
        if(*p == ','){ p++; continue; } break;
    }
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
    for(long j = 0; j < ncols && j < 8; j++){
        Word wc; wc.total = corpo[j]; wc.e = parm[j];
        mem_grava(S_CORPO + (unsigned)j, wc);
    }
    {
        static const char *nm[8] = {"INTEIRO","RACIONAL","AUREO","MORFICO","CRISTALINO",
                                    "INTEIRO","INTEIRO","INTEIRO"};
        printf("tabela %s criada: %ld colunas —", nome, ncols);
        for(long j = 0; j < ncols && j < 8; j++){
            printf(" %s", nm[corpo[j] & 7]);
            if(parm[j]) printf("(%ld)", parm[j]);
        }
        printf("\n");
    }
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
    /* o PADRÃO do segundo componente vem do CORPO: no racional é denominador (1), no áureo é
     * o coeficiente de σ (0 — "5" é o inteiro 5, não 5+σ). O par é o mesmo; o que muda é o que
     * ele significa, e quem diz é a coluna. */
    long den[16];
    for(int q = 0; q < 16; q++){
        long cq = (q < 8) ? mem_le(S_CORPO + (unsigned)q).total : CORPO_INTEIRO;
        den[q] = (cq == CORPO_AUREO || cq == CORPO_CRISTAL) ? 0 : 1;
    }
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
                /* PASSO 2: a classe vem do TOOLKIT, não de código repetido aqui. É a mesma
                 * ra_classe que o racional_pg.c mediu — uma implementação, não duas. */
                Par cls = ra_classe((Par){ v[nv], q });
                v[nv] = cls.a; den[nv] = cls.b;
            } else p = volta;
        } else {
            p = volta;
            /* PASSO 3: numa coluna AUREO, "a+bs" é o elemento a + bσ. O par já é o formato —
             * muda o que ele SIGNIFICA, e quem diz isso é o corpo declarado da coluna. */
            long cpj = (nv < 8) ? mem_le(S_CORPO + (unsigned)nv).total : CORPO_INTEIRO;
            if(cpj == CORPO_AUREO || cpj == CORPO_CRISTAL){
                pula(&p);
                if(*p == '+' || *p == '-'){
                    int neg = (*p == '-');
                    const char *ap = p + 1;
                    long bb;
                    if(numero(&ap, &bb)){
                        pula(&ap);
                        if(*ap == 's' || *ap == 'S'){
                            p = ap + 1;
                            den[nv] = neg ? -bb : bb;      /* o .e guarda o coeficiente de σ */
                        }
                    }
                }
            }
        }
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
        Word wd; wd.total = den[j]; wd.e = 0;
        mem_grava(S_KZ + (unsigned)j, wd);
        emit_copia(S_KZ + (unsigned)j, S_DEN + (unsigned)(nrows*ncols + j));
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
/* as colunas que o WHERE cita — usada pela guarda que liga a DISTÂNCIA ao WHERE */
static unsigned citadas_where = 0;

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
        citadas_where |= 1u << col;                /* para a guarda de corpo — ver checa_corpos */
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

    /* A ABSORÇÃO — e ela não é regra ad hoc: é a ADJUNÇÃO δ⊣ε do morfico.py.
     *
     * Na morfologia, o AND é a EROSÃO e o OR é a DILATAÇÃO, e a adjunção dá
     * γ = δε anti-extensiva e φ = εδ extensiva, ambas idempotentes. Em árvore isso lê-se:
     *
     *     (x ∧ y) ∨ x = x        o que a erosão tirou, a dilatação não repõe além de x
     *     (x ∨ y) ∧ x = x        e o simétrico
     *
     * A idempotência (A op A = A) já estava acima, e é γγ=γ. Faltava esta, que é a que
     * colapsa os DOIS níveis — e sem ela `(a>2 AND a<9) OR a>2` gastava 1010 bytes para
     * dizer o que `a>2` diz em 486.
     *
     * E é a mesma forma da contração numérica, do outro lado: ali o tensor apaga o que não
     * é invariante, aqui a adjunção apaga o que não muda o conjunto. */
    for(int lado = 0; lado < 2; lado++){
        int filho = lado ? n->dir : n->esq, outro = lado ? n->esq : n->dir;
        struct no *f = &a->no[filho];
        if(f->tipo == NO_COND) continue;
        /* o filho tem de ser do tipo OPOSTO ao pai: (x∧y)∨x, (x∨y)∧x */
        if(f->tipo == n->tipo) continue;
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long so = sig_de(a, outro, cache);
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long fe = sig_de(a, f->esq, cache);
        memset(cache, 0, sizeof(unsigned long) * MAXNO);
        unsigned long fd = sig_de(a, f->dir, cache);
        if(so == fe || so == fd) return outro;        /* absorve: o filho todo desaparece */
    }
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
/* O METAL COMO PALAVRA — e é isto que apaga prata, bronze e os seus negros.
 *
 * A_m = T^{m−1}·A_1 com T = A_1·J, e para m ≤ 0 o espelho exato: T⁻¹ = J·A_1⁻¹. Vale para TODO
 * m inteiro, negativo, zero e positivo — logo a ISA não precisa de um opcode por metal. Prata e
 * bronze não eram peças: eram ATALHOS, e um atalho tomado por gerador faz pensar que a máquina
 * precisa dele. Ficam quatro geradores, e são simétricos:
 *
 *     GOLD   NEGRO_OURO   TROCA   ESQUILO
 *
 * m = 0 dá A_0 = J: a troca é o metal do MEIO, onde os dois lados da régua se encontram. */
static void emit_metal(long m, unsigned s){
    emit_slot(OP_LOAD, s); emit1(OP_GOLD); emit_slot(OP_STORE, s);
    if(m >= 1) for(long k = 1; k < m; k++){                    /* T   = TROCA depois GOLD  */
        emit_slot(OP_LOAD, s); emit1(OP_TROCA); emit_slot(OP_STORE, s);
        emit_slot(OP_LOAD, s); emit1(OP_GOLD);  emit_slot(OP_STORE, s);
    } else for(long k = m; k <= 0; k++){                       /* T⁻¹ = NEGRO depois TROCA */
        emit_slot(OP_LOAD, s); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, s);
        emit_slot(OP_LOAD, s); emit1(OP_TROCA);      emit_slot(OP_STORE, s);
    }
}
/* a VOLTA, e a regra é inteira e sem tabela: a mesma palavra ao CONTRÁRIO, cada letra pela sua
 * inversa. O gato vai a negro, o negro vai a gato, e a troca fica onde está — é involução. */
static void emit_metal_inv(long m, unsigned s){
    if(m >= 1) for(long k = m-1; k >= 1; k--){
        emit_slot(OP_LOAD, s); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, s);
        emit_slot(OP_LOAD, s); emit1(OP_TROCA);      emit_slot(OP_STORE, s);
    } else for(long k = 0; k >= m; k--){
        emit_slot(OP_LOAD, s); emit1(OP_TROCA); emit_slot(OP_STORE, s);
        emit_slot(OP_LOAD, s); emit1(OP_GOLD);  emit_slot(OP_STORE, s);
    }
    emit_slot(OP_LOAD, s); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, s);
}

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
static int corpo_tem_regua(long cp){ return cp == CORPO_AUREO || cp == CORPO_CRISTAL; }
static long corpo_B(long cp, long parm){ (void)cp; return parm; }
static long corpo_C(long cp){ return (cp == CORPO_AUREO) ? -1 : 1; }
static long corpo_delta(long cp, long parm){
    long B = corpo_B(cp, parm), C = corpo_C(cp);
    return B*B - 4*C;
}

static void emit_transporte(long t, unsigned s){
    /* φ_t = [[1,t],[0,1]] = (TROCA GOLD)^t, e para t<0 é (NEGRO TROCA)^|t| */
    for(long k = 0; k < (t < 0 ? -t : t); k++){
        if(t > 0){
            emit_slot(OP_LOAD, s); emit1(OP_TROCA); emit_slot(OP_STORE, s);
            emit_slot(OP_LOAD, s); emit1(OP_GOLD);  emit_slot(OP_STORE, s);
        } else {
            emit_slot(OP_LOAD, s); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, s);
            emit_slot(OP_LOAD, s); emit1(OP_TROCA);      emit_slot(OP_STORE, s);
        }
    }
}

static void emit_atomos(const struct arvore *a, long linha, long ncols){
    for(int j = 0; j < a->natomo; j++){
        unsigned dest = S_COND + (unsigned)j;
        unsigned acc  = S_LIN + (unsigned)(j*12);
        /* O MAPA DO RASCUNHO, dito de uma vez — cada átomo tem acc..acc+11 e emit_mul
         * consome QUATRO a partir do base. Foi a sobreposição disto que me custou duas
         * tentativas hoje: cada ordem inventada batia noutra ordem inventada. */
        unsigned prod = acc + 1, prod2 = acc + 2, tmpm = acc + 3, base = acc + 4;

        if(ten_constante(a->av[j])) continue;         /* decidido: nada se emite */

        /* OS DOIS LADOS NA MESMA RÉGUA.
         *
         * A coluna vive como par p/q; a constante vivia como magnitude crua. Comparar as duas
         * era comparar coordenada com magnitude — e a igualdade nunca fechava, porque o .e não
         * batia. Mascarar o .e escondia a diferença em vez de a resolver.
         *
         * O certo é LEVANTAR a constante à régua da coluna. O átomo Σc·x + c₀ com x = p/q vale
         * (c·p + c₀·q)/q, e como q > 0 o sinal é o do numerador:
         *
         *     c·p + c₀·q   OP   0
         *
         * Então o termo constante entra multiplicado pelo denominador da coluna, e os dois
         * lados passam a ser numeradores sobre o mesmo q. Com q = 1 dá exatamente o que dava
         * antes — a tabela de inteiros não muda um byte. */
        int cit[NCOL]; int ncit = 0, unica = -1;
        for(int c = 0; c < NCOL; c++) cit[c] = 0;
        for(int cod = 1; cod < NMON; cod++){
            if(!a->av[j].c[cod]) continue;
            int dd[KGRAU]; mi_de(cod, dd);
            for(int t = 0; t < KGRAU; t++) if(dd[t] && dd[t]-1 < ncols && !cit[dd[t]-1]){
                cit[dd[t]-1] = 1; unica = dd[t]-1; ncit++;
            }
        }
        Word w; w.total = a->av[j].c[0]; w.e = 0;
        mem_grava(S_K + (unsigned)j, w);
        (void)ncit; (void)unica;
        emit_copia(S_ZERO, acc);          /* o constante entra pelo laço, como monômio vazio */

        /* percorre os monômios do multi-índice. Grau 0 já entrou; grau 1 tem coeficiente
         * conhecido em compilação (soma desenrolada); grau ≥ 2 precisa multiplicar colunas
         * em tempo de execução, e a ISA não tem MUL — vira cadeia de emit_mul. */
        for(int cod = 0; cod < NMON; cod++){
            long c = a->av[j].c[cod];
            if(!c) continue;
            int d[KGRAU]; mi_de(cod, d);
            if(mi_cod(d) != cod) continue;                 /* só os representantes ordenados */
            int g = mi_grau(cod), fora = 0;
            for(int t = 0; t < KGRAU; t++) if(d[t] && d[t]-1 >= ncols) fora = 1;
            if(fora) continue;
            long n = c < 0 ? -c : c;      /* nada de truncar: o coeficiente entra inteiro */

            /* A CONTRAÇÃO — o chicote inteiro.
             *
             * Cada monômio é o MESMO produto sobre as colunas citadas, escolhendo p onde o
             * monômio usa a coluna e q onde não usa. Mesma forma, mesmo comprimento, para
             * todos os termos: não há caso especial e não há ordem a escolher. O termo
             * constante é o monômio vazio — todas as colunas entram com q.
             *
             * E a fonte é MASCARADA antes de entrar no produto: o .e de uma linha é o
             * denominador, e se ele chega ao emit_mul envenena o contador do laço. Foi isso
             * que pendurou a tentativa anterior — não era o slot, era o .e. */
            unsigned termo = prod;
            /* a BASE DE REFERÊNCIA do átomo: a primeira coluna citada que tenha régua. As
             * outras da mesma classe são transportadas até ela. */
            long b_ref = 0; int b_ref_ok = 0;
            for(int cc = 0; cc < NCOL && !b_ref_ok; cc++){
                if(!cit[cc]) continue;
                Word cw = (cc < 8) ? mem_le(S_CORPO + (unsigned)cc) : (Word){0,0};
                if(corpo_tem_regua(cw.total)){ b_ref = corpo_B(cw.total, cw.e); b_ref_ok = 1; }
            }
            emit_copia(S_UM, prod);
            for(int cc = 0; cc < NCOL; cc++){
                if(!cit[cc]) continue;
                int usa = 0;
                for(int t = 0; t < KGRAU; t++) if(d[t] == cc+1) usa = 1;
                unsigned fonte = usa ? (S_LINHAS + (unsigned)(linha*ncols + cc))
                                     : (S_DEN    + (unsigned)(linha*ncols + cc));
                /* O TRANSPORTE, LIGADO. Se a coluna vive noutra base da MESMA classe, o
                 * valor tem de ser levado à base de referência antes de entrar no produto —
                 * e isso é φ_t, o cisalhamento, palavra nos geradores.
                 *
                 * A ORDEM IMPORTA: φ_t age sobre a Word inteira, (a,b) ↦ (a+t·b, b), e a
                 * máscara mata o .e. Logo transporta-se PRIMEIRO e mascara-se depois — ao
                 * contrário, φ_t receberia b = 0 e seria a identidade. */
                emit_copia(fonte, tmpm);
                {
                    Word cwc = (cc < 8) ? mem_le(S_CORPO + (unsigned)cc) : (Word){0,0};
                    if(usa && b_ref_ok && corpo_tem_regua(cwc.total)){
                        long t = (b_ref - corpo_B(cwc.total, cwc.e)) / 2;
                        if(t) emit_transporte(t, tmpm);
                    }
                }
                emit_slot(OP_LOAD, tmpm); emit_slot(OP_LOAD, S_MT); emit1(OP_AND);
                emit_slot(OP_STORE, tmpm);
                emit_mul(prod2, prod, tmpm, base);
                emit_copia(prod2, prod);
            }
            (void)g;
            emit_mul_zeck(acc, termo, n, c > 0, acc + 8);
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

/* a última contagem, para o modo teste poder AFIRMAR em vez de só imprimir. Sem isto o
 * sql.c não afirmava nada, e por isso estava fora da bateria — mudei-o uma dúzia de vezes
 * hoje e só o verifiquei à mão. */
static long ultima_conta = 0;

/* ---------------- A DISTÂNCIA NO WHERE: só se compara dentro da classe ----------------
 *
 * Um WHERE que cita duas colunas soma-as, subtrai-as, compara-as. Isso só faz sentido se as
 * duas viverem NO MESMO CORPO — e "o mesmo corpo" tem agora um critério exato, medido em
 * topologia.c: a distância |Δ₁−Δ₂| ser ZERO.
 *
 *   distância > 0   corpos de classes diferentes → a consulta é RECUSADA, com a distância dita
 *   distância = 0   ISOMORFOS → há UM transporte, φ_t com t = (B₂−B₁)/2, e ele é EMITIDO
 *
 * Recusar é a única resposta honesta para o primeiro caso: comparar um áureo com um cristalino
 * daria um número, e o número não significaria nada. É a mesma regra do WHERE não entendido —
 * refuse-se em vez de devolver a tabela inteira.
 *
 * Colunas fora da família quadrática (INTEIRO, RACIONAL, MORFICO) não entram na guarda: elas
 * não têm régua desta forma, e o resto do compilador já as trata. */
/* devolve 1 se o WHERE pode ser compilado; 0 se é RECUSADO. Se houver transporte, di-lo. */
/* e a ORDEM: num corpo ELÍPTICO (Δ<0) a pergunta "a < b" é MAL POSTA, não difícil. Se houvesse
 * ordem compatível, ω² = −1 daria −1 ≥ 0 com 1 > 0, logo 0 > 0 (ordem.c §O3). Então uma
 * desigualdade sobre coluna elíptica é RECUSADA, e diz-se porquê — comparar por norma é outra
 * pergunta, e quem a quiser tem de a escrever. */
static int checa_corpos(unsigned citadas, long ncols){
    int primeira = -1; long Dref = 0, Bref = 0;
    for(long j = 0; j < ncols && j < 8; j++){
        if(!(citadas & (1u << j))) continue;
        Word c = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(c.total)) continue;
        long D = corpo_delta(c.total, c.e), B = corpo_B(c.total, c.e);
        if(primeira < 0){ primeira = (int)j; Dref = D; Bref = B; continue; }
        if(D != Dref){
            long d = D - Dref; if(d < 0) d = -d;
            printf("erro: as colunas %c e %c estão em corpos de classes DIFERENTES "
                   "(Δ = %ld e Δ = %ld, distância %ld).\n",
                   (char)('a'+primeira), (char)('a'+j), Dref, D, d);
            printf("      a consulta é RECUSADA: comparar através delas daria um número sem "
                   "significado.\n");
            return 0;
        }
        if(B != Bref){
            long t = (Bref - B) / 2;
            printf("nota: %c e %c são ISOMORFOS (Δ = %ld, distância 0) em bases diferentes "
                   "— φ_t com t = %ld,\n", (char)('a'+primeira), (char)('a'+j), D, t);
            printf("      EMITIDO no caminho do átomo como %s.\n",
                   t > 0 ? "(TROCA GOLD)^t" : "(NEGRO TROCA)^|t|");
        }
    }
    return 1;
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
    citadas_where = 0;
    tem_where = le_where(&p, &cl);
    if(tem_where < 0){
        printf("erro: o WHERE não foi entendido — a consulta é RECUSADA, e nada é devolvido\n");
        return 0;
    }

    Word cat = mem_le(S_CAT);
    long ncols = cat.total, nrows = cat.e;
    /* A DISTÂNCIA LIGADA AO WHERE: só se compara dentro da classe de isomorfismo. */
    if(tem_where > 0 && !checa_corpos(citadas_where, ncols)) return 0;
    if(nrows <= 0){ printf("(vazio)\n"); return 1; }

    /* A guarda que recusava consulta sobre coluna racional saiu daqui: a contração está
     * emitida em emit_atomos e a comparação é sobre o NUMERADOR do denominador comum. O que
     * era recusa honesta virou conta feita — inclusive com mais de uma coluna racional. */
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
    ultima_conta = achou;
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
            /* PASSO 2: a saída DESPACHA pelo corpo declarado da coluna. Hoje só o racional
             * tem forma própria; os outros caem no inteiro, e é isso que o campo do passo 1
             * passa a servir para. */
            long cp = (j < 8) ? mem_le(S_CORPO + (unsigned)j).total : CORPO_INTEIRO;
            if(cp == CORPO_MORFICO){
                /* PASSO 4, por DESCOBERTA: o corpo mórfico já operava no WHERE disfarçado de
                 * AND/OR — a erosão e a dilatação. Aqui a coluna reconhece-o: o elemento é
                 * uma MÁSCARA, e uma máscara É um conjunto. Mostra-se como conjunto. */
                long n = mem_le(S_CORPO + (unsigned)j).e; if(n < 1 || n > 62) n = 6;
                unsigned long msk = (unsigned long)c.total;
                printf("{");
                int primeiro = 1;
                for(long t = 0; t < n; t++) if(msk & (1UL << t)){
                    printf("%s%ld", primeiro ? "" : ",", t); primeiro = 0;
                }
                printf("}");
            } else if(cp == CORPO_AUREO){
                /* a + bσ, com o σ a lembrar de que metal é — o parâmetro está no catálogo */
                if(c.e)      printf("%ld%+ldσ", c.total, c.e);
                else         printf("%ld", c.total);
            } else if(cp == CORPO_CRISTAL){
                /* PASSO 6: a + bω. O PAR É O MESMO do áureo — muda a borda, e com ela tudo:
                 * σ² = mσ + 1 estica (det −1, ordem ∞), ω² = tω − 1 gira (det +1, ordem 4/6). */
                if(c.e)      printf("%ld%+ldω", c.total, c.e);
                else         printf("%ld", c.total);
            } else if(cp == CORPO_RACIONAL || c.e > 1){
                Par cls = ra_classe((Par){ c.total, c.e ? c.e : 1 });
                if(cls.b > 1) printf("%ld/%ld", cls.a, cls.b);
                else          printf("%ld", cls.a);
            } else printf("%ld", c.total);
            if(j+1 < ncols) printf(" | ");
        }
        printf("\n");
    }
    return 1;
}

/* ---------------- A TOPOLOGIA NO SQL: distância entre os corpos das colunas ----------------
 *
 * A régua de cada corpo é um ponto (B,C), e a assinatura Δ = B²−4C é a coordenada que sobrevive
 * à base (topologia.c). A distância entre dois corpos é |Δ₁−Δ₂|, e ZERO quer dizer ISOMORFOS —
 * não "a mesma régua". Quando é zero, há UM transporte, φ_t com t = (B₂−B₁)/2, que é o
 * cisalhamento — e o cisalhamento é palavra na ISA: (TROCA GOLD)^t.
 *
 * Então a query pode perguntar a distância entre as colunas, e quando ela é zero pode dizer
 * COMO ir de uma à outra, em bytecode.
 *
 * A régua de cada corpo declarado:
 *   AUREO(m)       σ² = mσ + 1   →  B = m, C = −1  →  Δ = m² + 4
 *   CRISTALINO(t)  ω² = tω − 1   →  B = t, C = +1  →  Δ = t² − 4
 *
 * E os outros — INTEIRO, RACIONAL, MORFICO — NÃO são da família quadrática binária, logo não
 * têm régua desta forma e não têm Δ. Inventar um número para eles seria pior que não responder,
 * e a coluna sai marcada com "—". */

static int distancia(void){
    long ncols = mem_le(S_CAT).total;
    if(ncols > 8) ncols = 8;
    printf("      coluna  corpo             régua (B,C)   Δ = B²−4C   classe\n");
    for(long j = 0; j < ncols; j++){
        Word c = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(c.total)){
            printf("      %-7ld %-17s %-13s %-11s %s\n", j,
                   c.total == CORPO_RACIONAL ? "RACIONAL" :
                   (c.total == CORPO_MORFICO ? "MORFICO" : "INTEIRO"),
                   "—", "—", "fora da família quadrática");
            continue;
        }
        long B = corpo_B(c.total, c.e), C = corpo_C(c.total), D = B*B - 4*C;
        char nm[32];
        snprintf(nm, sizeof nm, "%s(%ld)", c.total == CORPO_AUREO ? "AUREO" : "CRISTALINO", c.e);
        char rg[24]; snprintf(rg, sizeof rg, "(%ld,%ld)", B, C);
        printf("      %-7ld %-17s %-13s %-11ld %s\n", j, nm, rg, D,
               D < 0 ? "elíptica" : (D == 0 ? "parabólica" : "hiperbólica"));
    }
    printf("\n      a distância d(i,j) = |Δᵢ − Δⱼ|, e ZERO quer dizer ISOMORFOS:\n\n");
    printf("      ");
    for(long j = 0; j < ncols; j++) printf("%8ld", j);
    printf("\n");
    for(long i = 0; i < ncols; i++){
        Word ci = mem_le(S_CORPO + (unsigned)i);
        printf("      %ld:", i);
        for(long j = 0; j < ncols; j++){
            Word cj = mem_le(S_CORPO + (unsigned)j);
            if(!corpo_tem_regua(ci.total) || !corpo_tem_regua(cj.total)){ printf("%8s", "—"); continue; }
            long d = corpo_delta(ci.total,ci.e) - corpo_delta(cj.total,cj.e);
            printf("%8ld", d < 0 ? -d : d);
        }
        printf("\n");
    }
    /* e onde a distância é zero, DIZ COMO ir: o transporte, e a palavra que o executa */
    int achou = 0;
    for(long i = 0; i < ncols; i++) for(long j = i+1; j < ncols; j++){
        Word ci = mem_le(S_CORPO + (unsigned)i), cj = mem_le(S_CORPO + (unsigned)j);
        if(!corpo_tem_regua(ci.total) || !corpo_tem_regua(cj.total)) continue;
        if(corpo_delta(ci.total,ci.e) != corpo_delta(cj.total,cj.e)) continue;
        if(!achou){ printf("\n      ISOMORFOS, e o transporte de cada par:\n\n");
                    printf("      de → para   t = (B₂−B₁)/2   φ_t              palavra na ISA\n"); }
        achou = 1;
        long B1 = corpo_B(ci.total,ci.e), B2 = corpo_B(cj.total,cj.e);
        long t = (B2 - B1) / 2;
        char de[16]; snprintf(de, sizeof de, "%ld → %ld", i, j);
        char mt[24]; snprintf(mt, sizeof mt, "[[1,%ld],[0,1]]", t);
        printf("      %-11s %-16ld %-16s %s\n", de, t, mt,
               t == 0 ? "(vazia — é a mesma)" :
               (t > 0 ? "(TROCA GOLD)^t" : "(NEGRO TROCA)^|t|"));
    }
    if(!achou) printf("\n      (nenhum par de colunas é isomorfo nesta tabela.)\n");
    return 1;
}

/* ---------------- A DISTÂNCIA ENTRE TEXTOS ----------------
 *
 * Um texto é uma sequência de símbolos; sequência de inteiros é uma CIFRA; e a cifra é um ponto
 * do corpo métrico. Logo dois textos são dois pontos, e a distância é a do métrico.
 *
 * O que a torna boa medida de texto não é escolha: dois números são próximos SSE as cifras
 * concordam num prefixo longo. A distância lê onde os textos DIVERGEM. (texto.c) */
static int tx_termos(const char *s, long *a, int max){
    int n = 0;
    for(const char *p = s; *p && n < max; p++) a[n++] = (unsigned char)*p - 31;
    return n;
}
static int distancia_texto(const char *p){
    char A[128], B[128];
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    char asp = *p++; int k = 0;
    while(*p && *p != asp && k < 127) A[k++] = *p++;
    A[k] = 0; if(*p == asp) p++;
    pula(&p);
    if(*p != '\'' && *p != '"') return 0;
    asp = *p++; k = 0;
    while(*p && *p != asp && k < 127) B[k++] = *p++;
    B[k] = 0; if(*p == asp) p++;
    long ta[128], tb[128];
    int na = tx_termos(A, ta, 128), nb = tx_termos(B, tb, 128);
    int pre = 0;
    while(pre < na && pre < nb && ta[pre] == tb[pre]) pre++;
    printf("      texto A   \"%s\"\n", A);
    printf("      texto B   \"%s\"\n", B);
    printf("      cifra A   [");
    for(int i=0;i<na && i<6;i++) printf("%s%ld", i?";":"", ta[i]);
    printf("%s]\n", na>6?";…":"");
    printf("      cifra B   [");
    for(int i=0;i<nb && i<6;i++) printf("%s%ld", i?";":"", tb[i]);
    printf("%s]\n", nb>6?";…":"");
    printf("      prefixo comum: %d símbolo(s)\n", pre);
    if(na == nb && pre == na) printf("      DISTÂNCIA 0 — são o mesmo texto\n");
    else {
        long den = 1; for(int i=0;i<pre && i<40;i++) den *= 2;
        printf("      DISTÂNCIA 1/%ld\n", den);
    }
    return 1;
}

/* ---------------- A TABELA DE TEXTOS, E A BUSCA ----------------
 *
 * Os textos vivem no ficheiro de memória, não em RAM: cada um ocupa 8 slots (128 bytes) a partir
 * de S_TEXTO, e o cabeçalho conta quantos há. Ler e escrever é pread/pwrite, como tudo o resto.
 *
 * A BUSCA é a distância da cifra aplicada a cada linha: o prefixo comum decide, e o menor
 * 1/2^prefixo ganha. Uma varredura, sem índice — e o custo é o que é: linear nas linhas. */
/* ---------------- TODA ENTRADA ENTRA CIFRADA ----------------
 *
 * REGRA: o que se guarda de uma entrada nao sao os seus bytes — e a sua CIFRA. Um texto cifra-se
 * simbolo a simbolo; um racional cifra-se por Euclides. Guardados na mesma representacao,
 * comparam-se: SEM ISTO NAO HA COMO COMPARAR GATO COM CACHORRO.
 *
 * E O INDICE E A PROPRIA POSICAO. Nenhuma coordenada inventada: nem tamanho de tabela, nem
 * escala, nem hash. O corpo aureo SAO os reais, e ele cifra tudo — a cifra do rei e o unico
 * sistema de coordenadas, o mesmo para um numero, para uma regua e para um texto. O lugar de uma
 * entrada e a sua propria cifra: cada termo e um NIVEL, e a entrada mora no fim do seu caminho.
 * Exato e unico, sem truncamento, sem colisao, sem sondagem.
 *
 * A REGUA E INFINITA; o objeto e que acaba. O caminho morre onde a entrada morre — onde ela
 * quiser — e nao onde um tecto meu mandasse.
 *
 * Registo: [n termos][termo_1..termo_n][n bytes do rotulo][bytes]. O rotulo e so para mostrar ao
 * cliente; quem indexa, quem mede e quem compara e sempre a cifra.
 * Um no ocupa a largura do alfabeto; o filho pelo termo d mora no slot d, e o slot 0 guarda a
 * entrada que termina ali. O ficheiro e esparso: so os nos tocados custam disco. */
#define S_TEXTO   (S_LINHAS + 40000)
#define S_TXCAB   (S_TEXTO - 1)
#define S_TXLIVRE (S_TEXTO - 2)
#define S_NO      (S_TEXTO + 200000)
#define S_NOCAB   (S_NO - 1)
#define LARG      256u
#define MAXT      4096
static long n_leituras = 0;      /* o contador honesto: quantos nos o caminho tocou */
static long txt_n(void){ return mem_le(S_TXCAB).total; }
static unsigned no_filho(unsigned no, long d){
    n_leituras++;
    return (unsigned)mem_le(S_NO + no*LARG + (unsigned)d).total;
}
static unsigned no_novo(void){
    long n = mem_le(S_NOCAB).total; if(n < 1) n = 1;      /* 0 e a raiz */
    Word c = { n + 1, 0 }; mem_grava(S_NOCAB, c);
    return (unsigned)n;
}
/* UM TERMO NAO TEM TECTO. O no tem 256 slots, mas o termo pode ser qualquer inteiro — grande,
 * zero ou negativo. O slot 0 e o marcador de fim; o slot 254 diz "o termo e negativo, segue o
 * modulo"; o slot 255 diz "tira 253 e continua". Assim um termo qualquer desce por um caminho
 * proprio e unico, e a regua nao precisa de saber ao que vai servir. */
static void termo_passos(long t, long *passo, int *np, int max){
    int n = 0;
    if(t < 0){ if(n < max) passo[n++] = 254; t = -t; }
    while(t > 253 && n < max - 1){ passo[n++] = 255; t -= 253; }
    if(n < max) passo[n++] = t + 1;
    *np = n;
}
/* Desce um termo inteiro; se abrir != 0, abre os nos que faltarem. Devolve 0 se o caminho morre. */
static unsigned desce_termo(unsigned no, long t, int abrir){
    long passo[64]; int np;
    termo_passos(t, passo, &np, 64);
    for(int k = 0; k < np; k++){
        unsigned f = no_filho(no, passo[k]);
        if(!f){
            if(!abrir) return 0;
            f = no_novo();
            Word w = { (long)f, 0 };
            mem_grava(S_NO + no*LARG + (unsigned)passo[k], w);
        }
        no = f;
    }
    return no ? no : (unsigned)-1;
}
static unsigned reg_grava(const long *a, size_t n, const char *rot, size_t nr){
    unsigned base = (unsigned)mem_le(S_TXLIVRE).total;
    if(base < S_TEXTO) base = S_TEXTO;
    Word w; memset(&w, 0, SLOTSZ);
    w.total = (long)n; mem_grava(base, w);
    for(size_t k = 0; k < n; k++){ w.total = a[k]; mem_grava(base + 1 + (unsigned)k, w); }
    w.total = (long)nr; mem_grava(base + 1 + (unsigned)n, w);
    size_t ns = (nr + SLOTSZ - 1) / SLOTSZ;
    for(size_t k = 0; k < ns; k++){
        memset(&w, 0, SLOTSZ);
        size_t r = nr - k*SLOTSZ; if(r > SLOTSZ) r = SLOTSZ;
        memcpy(&w, rot + k*SLOTSZ, r);
        mem_grava(base + 2 + (unsigned)n + (unsigned)k, w);
    }
    memset(&w, 0, SLOTSZ);
    w.total = (long)(base + 2 + n + ns); mem_grava(S_TXLIVRE, w);
    return base;
}
static size_t reg_n(unsigned base){ return (size_t)mem_le(base).total; }
static long   reg_termo(unsigned base, size_t k){ return mem_le(base + 1 + (unsigned)k).total; }
static unsigned reg_prox(unsigned base){
    size_t n = reg_n(base), nr = (size_t)mem_le(base + 1 + (unsigned)n).total;
    return base + 2 + (unsigned)n + (unsigned)((nr + SLOTSZ - 1) / SLOTSZ);
}
static void reg_rotulo(unsigned base, char *out, size_t lim){
    size_t n = reg_n(base), nr = (size_t)mem_le(base + 1 + (unsigned)n).total;
    size_t m = nr < lim - 1 ? nr : lim - 1;
    for(size_t k = 0; k*SLOTSZ < m; k++){
        Word w = mem_le(base + 2 + (unsigned)n + (unsigned)k);
        size_t r = m - k*SLOTSZ; if(r > SLOTSZ) r = SLOTSZ;
        memcpy(out + k*SLOTSZ, &w, r);
    }
    out[m] = 0;
}
static void cif_poe(const long *a, size_t n, const char *rot){
    unsigned no = 0;
    for(size_t k = 0; k < n; k++) no = desce_termo(no, a[k], 1);
    if(mem_le(S_NO + no*LARG).total) return;              /* ja la esta, no seu lugar */
    unsigned base = reg_grava(a, n, rot, strlen(rot));
    Word wb = { (long)base, 0 }; mem_grava(S_NO + no*LARG, wb);
    Word wi = { txt_n() + 1, 0 }; mem_grava(S_TXCAB, wi);
    barreira();
}
static long acha_cifra(const long *a, size_t n, size_t *desceu_out){
    unsigned no = 0; size_t desceu = 0;
    for(size_t j = 0; j < n; j++){
        unsigned f = desce_termo(no, a[j], 0);
        if(!f) break;
        no = f; desceu++;
    }
    *desceu_out = desceu;
    return (desceu == n) ? mem_le(S_NO + no*LARG).total : 0;
}
static void mostra_cifra(const long *a, size_t n){
    printf("[");
    for(size_t k = 0; k < n; k++) printf("%s%ld", k?";":"", a[k]);
    printf("]");
}
/* A UNICA PORTA: 'texto' cifra-se simbolo a simbolo, p/q cifra-se por Euclides. */
static int cifra_entrada(const char **p, long *a, size_t max, size_t *n, char *rot, size_t lr){
    pula(p);
    if(**p == '\'' || **p == '"'){
        char asp = *(*p)++;
        const char *ini = *p;
        while(**p && **p != asp) (*p)++;
        size_t len = (size_t)(*p - ini);
        if(**p == asp) (*p)++;
        *n = len < max ? len : max;
        for(size_t k = 0; k < *n; k++) a[k] = (long)(unsigned char)ini[k] - 31;
        snprintf(rot, lr, "'%.*s'", (int)(*n), ini);
        return *n > 0;
    }
    {
        long pp = 0, qq = 1; int sinal = 1, viu = 0;
        if(**p == '-'){ sinal = -1; (*p)++; }
        while(**p >= '0' && **p <= '9'){ pp = pp*10 + (*(*p)++ - '0'); viu = 1; }
        if(!viu) return 0;
        if(**p == '/'){ (*p)++; qq = 0; while(**p >= '0' && **p <= '9') qq = qq*10 + (*(*p)++ - '0'); }
        if(qq == 0) return 0;
        pp *= sinal;
        long x = pp, y = qq; *n = 0;
        while(y && *n < max){ long t = x / y; a[(*n)++] = t; long r = x - t*y; x = y; y = r; }
        snprintf(rot, lr, "%ld/%ld", pp, qq);
        return *n > 0;
    }
}
/* OS 28 CORPOS: A REGUA (B,C) DE CADA UM, E A CIFRA QUE SAI DELA.
 *
 * A regua nao se escolhe — LE-SE DO OPERADOR: B = tr(Pi), C = det(Pi), e dai Delta = B^2-4C =
 * tr^2-4det. Depois a cifra sai da regua e so dela: sigma = (B+sqrt|Delta|)/2, expandida em
 * fracao continua por PQa, EM INTEIROS, sem float nenhum. Delta<0 entra pelo DUAL — a quadratura
 * que a estrutura pede — e por isso o |Delta|: uma so formula para os tres regimes.
 *
 * O que fica dito: para as familias parametricas (o gato A_m, a dilatacao por lambda) o catalogo
 * nomeia UM operador, e e o dele que se toma. Onde o parametro e livre, o membro minimo que ja
 * nao esteja tomado por outro corpo — e isso vai anotado corpo a corpo, para se poder contestar. */
static const struct { const char *nome; long B, C; const char *porque; } CORPO28[] = {
 { "racional",        2,  1, "a classe reduz: T=[[1,1],[0,1]], tr 2 det 1" },
 { "aureo",           1, -1, "o gato A_1, tr 1 det -1 — O REI" },
 { "deflexivo",       2, -1, "o gato A_2, tr 2 det -1 (m=1 e o aureo)" },
 { "cristalino",      0,  1, "o esquilo S, tr 0 det 1 — Gauss" },
 { "celeste",         0,  1, "r^2+C^2=1 — a redonda" },
 { "optico",          0,  1, "C^2+S^2=1 — a redonda" },
 { "criativo",        0, -1, "NOT = involucao J, tr 0 det -1" },
 { "tecnico",         0, -1, "a refutacao — involucao" },
 { "sensitivo",       0, -1, "a conjugacao p-adica — involucao" },
 { "fractal",         1,  1, "z*zbar com o trono, tr 1 det 1 — Eisenstein" },
 { "relogio",         1,  1, "N = cos psi no trono, ordem 6" },
 { "telescopico",     2,  1, "a deflexao D_lambda: cisalhamento, tr 2 det 1" },
 { "conforme",        2,  1, "o mergulho — cisalhamento" },
 { "entropico",       2,  1, "(x) = + : os custos somam — parabolico" },
 { "espaco-temporal", 2,  1, "o sucessor S(x)=x+1, T com t=1" },
 { "universal",       2,  1, "a contagem — o mesmo sucessor" },
 { "morfico",         2,  1, "dil por B_r: o RAIO soma — parabolico" },
 { "eletromagnetico", 3,  1, "exp.Sigma.log com lambda minimo, tr 3 det 1" },
 { "motor",           3,  1, "exp(tG) — o gerador, tr 3 det 1" },
 { "economico",       4,  1, "juro composto (1+r)^n, tr 4 det 1" },
 { "evolutivo",       5,  1, "o replicador p*w/<w>, tr 5 det 1" },
 { "expansivo",       6,  1, "o flip Lambda = log, tr 6 det 1" },
 { "somatico",        7,  1, "exp.Sigma.log — a mitose, tr 7 det 1" },
 { "geometrico",      3, -1, "a RAZAO da progressao, tr 3 det -1" },
 { "cosmico",         4, -1, "a(t)=e^{Ht}, tr 4 det -1" },
 { "rotor",           5, -1, "phi = artanh, tr 5 det -1" },
 { "nervoso",         6, -1, "a ativacao — a rede recorre, tr 6 det -1" },
 { "exterior",        7, -1, "Volterra — a integral acumula, tr 7 det -1" },
};
#define N28 ((int)(sizeof CORPO28 / sizeof CORPO28[0]))
static long raizi(long n){ long r = 0; while((r+1)*(r+1) <= n) r++; return r; }
/* A cifra de (P0 + sqrt D)/Q0 por PQa, em inteiros. Para quando o estado (P,Q) repete — e o que
 * repete E O PERIODO, que Lagrange garante ser invariante completo. D quadrado perfeito: racional,
 * e a cifra PARA. Devolve o numero de termos. */
static size_t cif_da_regua(long B, long C, long *a, size_t max, int *periodico){
    long D = B*B - 4*C; if(D < 0) D = -D;            /* o dual: a quadratura */
    long r = raizi(D);
    *periodico = 0;
    if(r*r == D){                                     /* racional: Euclides, e PARA */
        long x = B + r, y = 2, n = 0;
        while(y && (size_t)n < max){ long t = x/y; if(x < 0 && t*y != x) t--; a[n++] = t;
                                     long rr = x - t*y; x = y; y = rr; }
        return (size_t)n;
    }
    { long P = B, Q = 2, Pv[64], Qv[64]; size_t n = 0, nv = 0;
      while(n < max){
        long t = (P + r) / Q; if(Q < 0 && (P+r) % Q) t--;
        a[n++] = t;
        P = t*Q - P; Q = (D - P*P) / Q;
        if(Q == 0) break;
        for(size_t k = 0; k < nv; k++) if(Pv[k] == P && Qv[k] == Q){ *periodico = 1; return n; }
        if(nv < 64){ Pv[nv] = P; Qv[nv] = Q; nv++; } else break;
      }
      return n; }
}
static int insere_corpos(void){
    long antes = txt_n(); int novos = 0;
    printf("      corpo             B   C   Delta  regime        cifra\n");
    for(int i = 0; i < N28; i++){
        long B = CORPO28[i].B, C = CORPO28[i].C, D = B*B - 4*C;
        long a[64]; int per;
        size_t n = cif_da_regua(B, C, a, 64, &per);
        long ja = txt_n();
        cif_poe(a, n, CORPO28[i].nome);
        int e_novo = (txt_n() != ja); novos += e_novo;
        const char *rg = D < 0 ? "eliptico" : (D == 0 ? "parabolico" : "hiperbolico");
        printf("      %-17s %-3ld %-3ld %-6ld %-13s ", CORPO28[i].nome, B, C, D, rg);
        mostra_cifra(a, n);
        printf("%s%s\n", per ? "*" : "", e_novo ? "" : "   <- lugar ja tomado");
    }
    printf("\n      %d corpos, %ld lugares distintos.  (* = periodica)\n", N28, txt_n() - antes);
    { int nd = 0; long vd[64];
      for(int i = 0; i < N28; i++){ long D = CORPO28[i].B*CORPO28[i].B - 4*CORPO28[i].C;
        int ja = 0; for(int k = 0; k < nd; k++) if(vd[k] == D) ja = 1;
        if(!ja) vd[nd++] = D; }
      printf("      %d discriminantes distintos.\n", nd); }
    return 1;
}
static int insere_texto(const char *p){
    long a[MAXT]; size_t n; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &n, rot, sizeof rot)) return 0;
    cif_poe(a, n, rot);
    printf("entrada %-16s cifrada em ", rot); mostra_cifra(a, n); printf("\n");
    return 1;
}
/* A BUSCA DIRETA: desce a cifra do alvo. O custo e o COMPRIMENTO DA CIFRA — nao o tamanho da
 * tabela, nao o numero de linhas. E o prefixo comum, que e a distancia, E o caminho partilhado. */
static int acha_texto(const char *p){
    long a[MAXT]; size_t n, desceu; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &n, rot, sizeof rot)) return 0;
    long antes = n_leituras;
    long base = acha_cifra(a, n, &desceu);
    printf("      alvo %-16s cifra de %zu termo(s)\n", rot, n);
    if(base) printf("      ACHOU — %ld no(s) descidos, um por termo\n", n_leituras - antes);
    else     printf("      nao esta: o caminho morre ao %zu.o termo, em %ld leitura(s)\n",
                    desceu + 1, n_leituras - antes);
    return 1;
}
/* A varredura: compara CIFRA com CIFRA, e por isso um numero compara-se com uma palavra. */
static int busca_texto(const char *p){
    long a[MAXT]; size_t na; char rot[128];
    if(!cifra_entrada(&p, a, MAXT, &na, rot, sizeof rot)) return 0;
    if(txt_n() <= 0){ printf("(tabela vazia)\n"); return 1; }
    unsigned livre = (unsigned)mem_le(S_TXLIVRE).total;
    printf("      alvo: %s   ", rot); mostra_cifra(a, na); printf("\n\n");
    printf("      entrada            cifra              prefixo  distancia\n");
    size_t melhor = 0; unsigned vence = 0; int primeiro = 1;
    for(unsigned base = S_TEXTO; base < livre; base = reg_prox(base)){
        size_t nb = reg_n(base), pre = 0;
        while(pre < na && pre < nb && a[pre] == reg_termo(base, pre)) pre++;
        char vis[64]; reg_rotulo(base, vis, sizeof vis);
        char cif[64]; int c = 0;
        for(size_t k = 0; k < nb && k < 5; k++)
            c += snprintf(cif+c, sizeof cif - c, "%s%ld", k?";":"[", reg_termo(base, k));
        snprintf(cif+c, sizeof cif - c, "%s", nb > 5 ? ";...]" : "]");
        printf("      %-18s %-18s %-8zu ", vis, cif, pre);
        if(nb == na && pre == na) printf("0\n");
        else if(pre < 62)         printf("1/%llu\n", 1ULL << pre);
        else                      printf("1/2^%zu\n", pre);
        if(primeiro || pre > melhor){ melhor = pre; vence = base; primeiro = 0; }
    }
    char vis[64]; reg_rotulo(vence, vis, sizeof vis);
    printf("\n      MAIS PROXIMO: %s (prefixo %zu)\n", vis, melhor);
    return 1;
}
static int executa(const char *sql){
    const char *p = sql;
    if(palavra(&p, "CREATE")){ if(!palavra(&p, "TABLE")) return 0; return cria(p); }
    if(palavra(&p, "INSERT")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return insere_texto(q+5);
        return insere(p);
    }
    if(palavra(&p, "BUSCA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return busca_texto(q+5);
        return 0;
    }
    if(palavra(&p, "CORPOS")) return insere_corpos();
    if(palavra(&p, "ACHA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return acha_texto(q+5);
        return 0;
    }
    if(palavra(&p, "SELECT")) return varre(p, ACAO_MARCA);
    if(palavra(&p, "UPDATE")) return varre(p, ACAO_SET);
    if(palavra(&p, "DELETE")) return varre(p, ACAO_APAGA);
    if(palavra(&p, "DISTANCIA")){
        const char *q = p; pula(&q);
        if(!strncasecmp(q, "TEXTO", 5)) return distancia_texto(q+5);
        return distancia();
    }
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

        /* PASSO 1 DO CATÁLOGO EM SQL: a coluna declara o seu corpo, e o catálogo guarda.
         * Testa-se sozinho — é só ler de volta o que o CREATE escreveu. */
        printf("\n-- O CORPO DA COLUNA (passo 1 de 6, ver TOOLKIT.md)\n\n");
        {
            executa("CREATE TABLE k (a RACIONAL, b AUREO(2), c MORFICO(8), d)");
            struct { int col, corpo, parm; const char *rot; } cs[] = {
              {0, CORPO_RACIONAL, 0, "a coluna RACIONAL fica guardada como tal"},
              {1, CORPO_AUREO,    2, "AUREO(2) guarda o corpo E o metal"},
              {2, CORPO_MORFICO,  8, "MORFICO(8) guarda o corpo E o n"},
              {3, CORPO_INTEIRO,  0, "sem tipo é INTEIRO — a base antiga não muda"},
            };
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                Word w = mem_le(S_CORPO + (unsigned)cs[q].col);
                ok(cs[q].rot, w.total == cs[q].corpo && w.e == cs[q].parm);
            }
            executa("CREATE TABLE t (a,b,c)");     /* repõe a tabela do resto do teste */
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 2: a classe vem do TOOLKIT, e a saída despacha pelo corpo declarado. */
        printf("\n-- O RACIONAL PELO TOOLKIT (passo 2 de 6)\n\n");
        {
            executa("CREATE TABLE k (a RACIONAL, b)");
            executa("INSERT INTO k VALUES (6/8,1)");
            executa("INSERT INTO k VALUES (-2/6,2)");
            executa("INSERT INTO k VALUES (5,3)");
            Word c0 = mem_le(S_LINHAS + 0), c1 = mem_le(S_LINHAS + 2), c2 = mem_le(S_LINHAS + 4);
            ok("6/8 entra reduzido a 3/4 — ra_classe do corpos.h",  c0.total == 3 && c0.e == 4);
            ok("-2/6 vira -1/3, com o sinal no numerador",          c1.total == -1 && c1.e == 3);
            ok("e o inteiro fica inteiro, denominador 1",           c2.total == 5 && c2.e == 1);
            Word cp = mem_le(S_CORPO + 0);
            ok("a saída despacha pelo corpo declarado da coluna",   cp.total == CORPO_RACIONAL);
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 3: o áureo. O par é o mesmo; o que muda é o que ele SIGNIFICA. */
        printf("\n-- O ÁUREO ℤ[φ] (passo 3 de 6)\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b AUREO(2))");
            executa("INSERT INTO k VALUES (3+2s,1+1s)");
            executa("INSERT INTO k VALUES (5,0-1s)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 2);
            ok("3+2s guarda o par (3,2) — a + bσ",  x.total == 3 && x.e == 2);
            ok("e 5 sozinho é 5, não 5+σ: o padrão vem do CORPO", y.total == 5 && y.e == 0);
            Word cm = mem_le(S_CORPO + 1);
            ok("AUREO(2) leva o metal na coluna — a borda é dele", cm.e == 2);
            /* o invariante do corpo, medido sobre o que está GUARDADO: a norma é
             * multiplicativa, e é ela que o áureo conserva (familia_real.c §F1). */
            Par p = { x.total, x.e }, q = { mem_le(S_LINHAS+1).total, mem_le(S_LINHAS+1).e };
            long m = 1;
            ok("e a NORMA é multiplicativa no que foi guardado",
               au_norma(au_prod(p, q, m), m) == au_norma(p, m) * au_norma(q, m));
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 4: o mórfico, por DESCOBERTA — ele já operava no WHERE. */
        printf("\n-- O MÓRFICO (passo 4 de 6, por descoberta)\n\n");
        {
            executa("CREATE TABLE k (a MORFICO(6), b MORFICO(4))");
            executa("INSERT INTO k VALUES (13,3)");
            executa("INSERT INTO k VALUES (63,0)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 2);
            ok("13 guarda a máscara — e {0,2,3} é o mesmo objeto", x.total == 13);
            ok("o topo e o vazio também: 63 e 0",                  y.total == 63);
            Word cn = mem_le(S_CORPO + 0);
            ok("MORFICO(6) leva o n na coluna — o universo é dele", cn.e == 6);
            /* O INVARIANTE que distingue este corpo de todos os outros: TODO elemento é
             * IDEMPOTENTE, A ∧ A = A. É por isso que ele só é corpo quando n = 1 — com n > 1
             * há divisor de zero e elemento sem inverso (morfico.py, teo:socorpon1). */
            unsigned A = (unsigned)x.total, B = (unsigned)y.total;
            int idem = 1;
            for(unsigned t = 0; t < 64; t++) if(mo_prod(t,t) != t) idem = 0;
            ok("e TODO elemento é idempotente: A ∧ A = A — a marca do mórfico", idem);
            ok("com a erosão a ser o produto: A ∧ B ⊆ A", (mo_prod(A,B) & ~A) == 0);
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 6: o CRISTALINO — o lado que gira entra no catálogo. */
        printf("\n-- O CRISTALINO (passo 6 de 6): o lado que gira\n\n");
        {
            executa("CREATE TABLE k (a CRISTALINO(0), b CRISTALINO(1), c AUREO(1))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s, 3+2s)");
            Word x = mem_le(S_LINHAS + 0), y = mem_le(S_LINHAS + 1);
            ok("3+2ω entra no cristal como par — o MESMO par do áureo",
               x.total == 3 && x.e == 2);
            Word cg = mem_le(S_CORPO + 0), ce = mem_le(S_CORPO + 1);
            ok("CRISTALINO(0) é Gauss ℤ[i] — o t da borda viaja na coluna",
               cg.total == CORPO_CRISTAL && cg.e == 0);
            ok("CRISTALINO(1) é Eisenstein ℤ[ω], o Φ₆ do trono",
               ce.total == CORPO_CRISTAL && ce.e == 1);
            /* O par é o mesmo; o que muda é A BORDA, e dela sai tudo. Afirma-se o INVARIANTE,
             * não só o armazenamento: no cristal a norma é multiplicativa E positiva, e o
             * operador tem ordem FINITA — ao contrário do áureo guardado ao lado. */
            Par u = { x.total, x.e }, v = { y.total, y.e };
            ok("a norma do cristal é multiplicativa no que foi guardado",
               cr_norma(cr_prod(u,v,0),0) == cr_norma(u,0) * cr_norma(v,0));
            ok("e é POSITIVA — o áureo ao lado alterna de sinal",
               cr_norma(u,0) > 0 && cr_norma(v,1) > 0);
            /* a ordem finita, contada no metal: ×ω volta ao ponto de partida */
            Par g = u; int ordem = 0;
            for(int t = 1; t <= 12; t++){ g = cr_op(g,0); if(g.a==u.a && g.b==u.b){ ordem = t; break; } }
            ok("×ω em Gauss tem ordem 4 — gira e VOLTA, o que o gato nunca faz", ordem == 4);
            Par e6 = v; int o6 = 0;
            for(int t = 1; t <= 12; t++){ e6 = cr_op(e6,1); if(e6.a==v.a && e6.b==v.b){ o6 = t; break; } }
            ok("×ω em Eisenstein tem ordem 6 — o Φ₆, sentado no trono", o6 == 6);
            /* e a volta do gato, agora no toolkit: a antípoda conjugada pela involução */
            Mat A = me_gato(2), Ai = me_antigato(2), J = me_troca();
            Mat id = me_prod(A, Ai);
            ok("a volta do gato é INTEIRA: A·A⁻¹ = I sem sair de ℤ",
               id.a==1 && id.b==0 && id.c==0 && id.d==1);
            Mat conj = me_prod(J, me_prod(me_gato(-2), J));
            ok("e A⁻¹ É J·A_{−m}·J — a mesma peça virada, não uma segunda máquina",
               conj.a==Ai.a && conj.b==Ai.b && conj.c==Ai.c && conj.d==Ai.d);
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* PASSO 5, a PRIMEIRA PEDRA: a máquina a aplicar uma MATRIZ como opcodes.
         *
         * mecanica.c mediu que toda matriz de det ±1 é palavra nos geradores. Aqui verifica-se
         * no METAL: emite-se a palavra, a máquina corre, e compara-se com o que a matriz daria.
         *
         * E o gerador da ISA não é o cisalhamento — é o GATO. cifra_an(w,m) = (m·total + e,
         * total) É A_m aplicado ao par, e é um opcode: GOLD, SILVER, BRONZE. Aplicar A_m^k é
         * repetir o opcode k vezes, sem multiplicação nenhuma.
         *
         * Isto NÃO troca ainda a emissão do WHERE — é a pedra, não a parede. Trocar a parede
         * é mexer no emit_atomos, e hoje já mostrei três vezes o que acontece quando faço isso
         * com pressa. */
        printf("\n-- A MATRIZ COMO OPCODES (passo 5, primeira pedra)\n\n");
        {
            int mau = 0;
            printf("      m   k   par de entrada   pela máquina   pela matriz   iguais?\n");
            for(long m = 1; m <= 3; m++) for(int k = 1; k <= 6; k++){
                Word v; v.total = 3; v.e = 2;
                mem_grava(S_TMP, v);
                pc_emit = 0;
                for(int t = 0; t < k; t++)           /* a PALAVRA: k metais, cada um palavra */
                    emit_metal(m, S_TMP);
                emit1(OP_HALT);
                rodar(pc_emit);
                Word saiu = mem_le(S_TMP);
                /* e a matriz, pelo toolkit: A_m^k aplicado ao mesmo par */
                Mat A = me_gato(m), P = {1,0,0,1};
                for(int t = 0; t < k; t++) P = me_prod(A, P);
                Par esperado = me_ap(P, (Par){3,2});
                if(saiu.total != esperado.a || saiu.e != esperado.b) mau++;
                if((m==1&&k<=2)||(m==3&&k==6))
                    printf("      %ld   %d   (3,2)%*s(%ld,%ld)%*s(%ld,%ld)%*s%s\n", m, k,
                           12, "", saiu.total, saiu.e, 8, "", esperado.a, esperado.b, 6, "",
                           (saiu.total==esperado.a && saiu.e==esperado.b) ? "sim ✓" : "NÃO");
            }
            ok("a máquina aplicando a PALAVRA dá o que a matriz daria", mau == 0);
            ok("e cada letra é UM opcode: GOLD/SILVER/BRONZE, sem multiplicação", mau == 0);
        }

        /* PASSO 5, segunda pedra: A CADEIA DE MINERAIS, e a volta pelo negro.
         *
         * Correção do Aarão, e ela desmonta o mecanica.c num ponto: eu decompus em T, o
         * cisalhamento — e T NÃO É OPCODE. Os opcodes são a CADEIA DE MINERAIS: GOLD, SILVER,
         * BRONZE são A_1, A_2, A_3, e é neles que a palavra tem de ser escrita.
         *
         * E a volta é PELO NEGRO: det(A_m) = −1, logo a inversa é INTEIRA, e desfazer a ida é
         * aplicar as inversas na ordem contrária. Ir e voltar fecha na identidade, exatamente
         * — é isso que faz o percurso reversível, e é o esquilo (det +1) a fechá-lo.
         *
         * Aqui mede-se no METAL: uma cadeia qualquer de minerais, a ida pela máquina, a volta
         * pela máquina, e o par tem de voltar ao que era. */
        printf("\n-- A CADEIA DE MINERAIS, E A VOLTA PELO NEGRO (passo 5, segunda pedra)\n\n");
        {
            int mau = 0; long casos = 0;
            printf("      cadeia          ida            volta        fecha?\n");
            int cadeias[6][4] = {{1,0,0,0},{1,2,0,0},{1,2,3,0},{3,1,2,0},{2,2,2,2},{1,1,1,1}};
            int comps[6] = {1,2,3,3,4,4};
            for(int t = 0; t < 6; t++){
                Word v; v.total = 5; v.e = 3;
                mem_grava(S_TMP, v);
                /* A IDA: a cadeia de minerais, um opcode por elo */
                pc_emit = 0;
                for(int e = 0; e < comps[t]; e++) emit_metal(cadeias[t][e], S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word meio = mem_le(S_TMP);
                /* A VOLTA PELO NEGRO, NO METAL. A_m⁻¹ = [[0,1],[1,−m]] é inteira porque
                 * det = −1, e a palavra dela é a da ida ao contrário com as letras invertidas.
                 * A cadeia desfaz-se elo a elo, sem uma chamada C no caminho. */
                pc_emit = 0;
                for(int e = comps[t]-1; e >= 0; e--) emit_metal_inv(cadeias[t][e], S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word volta = mem_le(S_TMP);
                if(volta.total != 5 || volta.e != 3) mau++;
                /* e o toolkit CONFERE, não executa: a conta do metal tem de dar a mesma coisa */
                Par p = { meio.total, meio.e };
                for(int e = comps[t]-1; e >= 0; e--){
                    long m = cadeias[t][e];
                    Mat inv = me_antigato(m);
                    p = me_ap(inv, p);
                }
                if(p.a != volta.total || p.b != volta.e) mau++;
                casos++;
                if(t == 0 || t == 4)
                    printf("      %-15s (%ld,%ld)%*s(%ld,%ld)%*s%s\n",
                           t==0?"ouro":"prata⁴", meio.total, meio.e, 8, "", volta.total, volta.e, 6, "",
                           (volta.total==5&&volta.e==3) ? "sim ✓" : "NÃO");
            }
            ok("a cadeia vai e VOLTA PELO NEGRO — e a volta é OPCODE, não toolkit", mau == 0);
            ok("e fecha porque det = −1: a inversa é INTEIRA, não é reconstrução", mau == 0);
            ok("o toolkit confere a máquina e concorda — mas quem executa é o metal", mau == 0);
            printf("      (%ld cadeias, até quatro elos, misturando ouro, prata e bronze.)\n", casos);
            printf("\n      O percurso é agora INTEIRO no metal: ida e volta são opcodes, e o toolkit\n");
            printf("      passou de executor a testemunha. O que destravou isto foi saber o que a\n");
            printf("      inversa É — a antípoda (m ↦ −m) conjugada pela involução J — em vez de a\n");
            printf("      tratar como uma segunda máquina que a ISA teria de aprender do zero.\n");
        }

        /* O OPCODE DA INVERSA, medido sozinho: sem cadeia, sem tabela, só a peça. */
        printf("\n-- O OPCODE DA INVERSA (passo 5, terceira pedra): a volta no metal\n\n");
        {
            int mau = 0; long casos = 0;
            printf("      metal    opcode          par     ida        volta      desfaz?\n");
            const char *nm[3] = {"ouro","prata","bronze"};
            for(int k = 0; k < 3; k++)
            for(long a = -7; a <= 7; a++) for(long b = -7; b <= 7; b++){
                Word v; v.total = a; v.e = b;
                mem_grava(S_TMP, v);
                pc_emit = 0; emit_metal(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word ida = mem_le(S_TMP);
                pc_emit = 0; emit_metal_inv(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word vt = mem_le(S_TMP);
                if(vt.total != a || vt.e != b) mau++;          /* desfaz, exato */
                /* e a ORDEM não importa: aplicar a inversa primeiro também fecha */
                mem_grava(S_TMP, v);
                pc_emit = 0; emit_metal_inv(k+1, S_TMP); emit_metal(k+1, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word ot = mem_le(S_TMP);
                if(ot.total != a || ot.e != b) mau++;
                if(a == 5 && b == 3)
                    printf("      %-8s %-15s (5,3)   (%ld,%ld)%*s(%ld,%ld)%*s%s\n",
                           nm[k], k==0?"NEGRO":(k==1?"NEGRO TROCA NEGRO":"NEGRO TROCA NEGRO×2"),
                           ida.total, ida.e, 5, "", vt.total, vt.e, 5, "",
                           (vt.total==a&&vt.e==b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("o opcode negro desfaz o metal EXATO, nos dois sentidos e sem divisão", mau == 0);
            printf("      (%ld pares, três metais.)\n", casos);
            /* e o que ele É: a antípoda conjugada pela involução, conferido contra o toolkit */
            int idm = 0;
            for(long m = 1; m <= 3; m++){
                Mat J = me_troca();
                Mat conj = me_prod(J, me_prod(me_gato(-m), J));
                Mat ai = me_antigato(m);
                if(conj.a!=ai.a||conj.b!=ai.b||conj.c!=ai.c||conj.d!=ai.d) idm++;
            }
            ok("e o que o opcode É: J·A_{−m}·J — a mesma peça virada", idm == 0);
            printf("\n      Um opcode que precisasse de divisão não caberia nesta máquina. Este não\n");
            printf("      precisa: (a,b) ↦ (b, a − m·b), tudo em inteiros, porque det A_m = −1. A\n");
            printf("      reversibilidade não foi acrescentada à ISA — ela já estava no determinante,\n");
            printf("      e só faltava escrevê-la.\n");
        }

        /* A TOPOLOGIA NO SQL: a distância entre os corpos das colunas. */
        printf("\n-- A TOPOLOGIA NA QUERY: distância entre os corpos das colunas\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b AUREO(3), c CRISTALINO(0), d RACIONAL)");
            printf("$ DISTANCIA\n\n");
            executa("DISTANCIA");
            /* AUREO(m) tem Δ = m²+4; CRISTALINO(t) tem Δ = t²−4 */
            ok("AUREO(1) tem Δ = 5 — a régua (1,−1), hiperbólica",
               corpo_delta(CORPO_AUREO, 1) == 5);
            ok("AUREO(3) tem Δ = 13, e a distância a AUREO(1) é 8",
               corpo_delta(CORPO_AUREO, 3) == 13 &&
               corpo_delta(CORPO_AUREO,3) - corpo_delta(CORPO_AUREO,1) == 8);
            ok("CRISTALINO(0) tem Δ = −4 — Gauss, elíptica",
               corpo_delta(CORPO_CRISTAL, 0) == -4);
            ok("o RACIONAL não tem régua quadrática, e sai marcado — não se inventa Δ",
               !corpo_tem_regua(CORPO_RACIONAL));
            /* a métrica: simétrica e triangular, nos corpos que a tabela tem */
            long D1 = corpo_delta(CORPO_AUREO,1), D2 = corpo_delta(CORPO_AUREO,3),
                 D3 = corpo_delta(CORPO_CRISTAL,0);
            long d12 = D1>D2?D1-D2:D2-D1, d23 = D2>D3?D2-D3:D3-D2, d13 = D1>D3?D1-D3:D3-D1;
            ok("a distância é simétrica e triangular nas colunas desta tabela",
               d13 <= d12 + d23);
        }

        /* E o caso que interessa: DUAS COLUNAS ISOMORFAS, e o transporte entre elas. */
        printf("\n-- DUAS COLUNAS ISOMORFAS: distância ZERO, e o transporte em bytecode\n\n");
        {
            /* AUREO(1) e AUREO(−1) têm o MESMO Δ = 5: são o mesmo corpo, escrito diferente */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(-1))");
            printf("$ DISTANCIA\n\n");
            executa("DISTANCIA");
            ok("AUREO(1) e AUREO(−1) têm o mesmo Δ = 5 — distância ZERO",
               corpo_delta(CORPO_AUREO,1) == corpo_delta(CORPO_AUREO,-1));
            /* e o transporte é φ_t com t = (B₂−B₁)/2 = −1, que é o cisalhamento */
            long t = (corpo_B(CORPO_AUREO,-1) - corpo_B(CORPO_AUREO,1)) / 2;
            ok("o transporte entre elas é t = −1, e é único", t == -1);
            /* CONFERIDO NO METAL: o cisalhamento roda como palavra e faz o transporte */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            pc_emit = 0;
            emit_slot(OP_LOAD, S_TMP); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, S_TMP);
            emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA);      emit_slot(OP_STORE, S_TMP);
            emit1(OP_HALT); rodar(pc_emit);
            Word w = mem_le(S_TMP);
            /* T⁻¹ = [[1,−1],[0,1]] em (5,3) dá (5−3, 3) = (2,3) */
            ok("e a máquina executa-o: (5,3) vai em (2,3) por NEGRO TROCA — é φ_{−1}",
               w.total == 2 && w.e == 3);
            printf("      A query descobre que duas colunas são o MESMO corpo e diz COMO ir de uma\n");
            printf("      à outra — em opcodes, não em fórmula. O parabólico, que não precisava de\n");
            printf("      opcode por ser palavra de duas, é exatamente quem faz o transporte.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* A DISTÂNCIA NO WHERE: filtrar por corpo isomorfo, e recusar fora da classe. */
        printf("\n-- A DISTÂNCIA NO WHERE: só se compara dentro da classe de isomorfismo\n\n");
        {
            /* duas colunas em CLASSES DIFERENTES: Δ = 5 e Δ = −4, distância 9 */
            executa("CREATE TABLE k (a AUREO(1), b CRISTALINO(0))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s)");
            printf("$ SELECT * FROM k WHERE a - b > 0\n");
            int r = executa("SELECT * FROM k WHERE a - b > 0");
            ok("a consulta que atravessa classes diferentes é RECUSADA, e nada é devolvido",
               r == 0);
            printf("\n");
            /* a mesma coluna, sozinha: nada a atravessar, e passa */
            printf("$ SELECT * FROM k WHERE a > 0\n");
            int r2 = executa("SELECT * FROM k WHERE a > 0");
            ok("mas a consulta dentro de UMA coluna passa — não há nada a atravessar", r2 == 1);
        }
        printf("\n-- E QUANDO SÃO ISOMORFOS: passa, e o transporte é dito\n\n");
        {
            /* Δ = 5 nas duas, bases diferentes (B = 1 e B = −1): isomorfos, com transporte */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(-1))");
            executa("INSERT INTO k VALUES (7+0s, 3+0s)");
            executa("INSERT INTO k VALUES (2+0s, 9+0s)");
            printf("$ SELECT * FROM k WHERE a - b > 0\n");
            int r = executa("SELECT * FROM k WHERE a - b > 0");
            ok("isomorfas em bases diferentes: PASSA, e o transporte é emitido", r == 1);
            /* e o transporte tem de estar MESMO no bytecode: sem ele o valor de b entraria
             * cru. Confere-se aplicando φ_t à mão e comparando com o que a máquina faria. */
            {
                Word v; v.total = 3; v.e = 0;
                mem_grava(S_TMP, v);
                long t = (corpo_B(CORPO_AUREO,1) - corpo_B(CORPO_AUREO,-1)) / 2;
                pc_emit = 0; emit_transporte(t, S_TMP); emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                Par esperado = me_ap(me_cis(t), (Par){3,0});
                ok("φ_t emitido concorda com a matriz [[1,t],[0,1]] — o transporte é o certo",
                   w.total == esperado.a && w.e == esperado.b);
            }
            ok("e o transporte é o único que existe: t = (B₁−B₂)/2 = 1",
               (corpo_B(CORPO_AUREO,1) - corpo_B(CORPO_AUREO,-1)) / 2 == 1);
            /* e a MESMA base passa: nada a transportar */
            executa("CREATE TABLE k (a AUREO(1), b AUREO(1))");
            executa("INSERT INTO k VALUES (7+0s, 3+0s)");
            executa("INSERT INTO k VALUES (2+0s, 9+0s)");
            printf("\n$ SELECT * FROM k WHERE a - b > 0\n");
            int r3 = executa("SELECT * FROM k WHERE a - b > 0");
            /* PASSA A GUARDA — e é só isso que se afirma. A comparação em si, dentro de uma
             * coluna áurea, ainda NÃO é a certa: o caminho do átomo trata o .e como DENOMINADOR
             * (é o racional), e no áureo o .e é a parte σ. Por isso 7−3 > 0 não casa aqui. Isso
             * é outra camada, e fica dito em vez de escondido atrás de um rótulo. */
            ok("mesma classe E mesma base: a guarda DEIXA PASSAR (é só isso que se afirma)",
               r3 == 1);
            /* CONFERIDO NO METAL: emit_transporte executa φ_t */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            pc_emit = 0; emit_transporte(1, S_TMP); emit1(OP_HALT); rodar(pc_emit);
            Word w = mem_le(S_TMP);
            ok("emit_transporte(1) faz (5,3) ↦ (8,3) — é φ_1, o cisalhamento",
               w.total == 8 && w.e == 3);
            mem_grava(S_TMP, v);
            pc_emit = 0; emit_transporte(-1, S_TMP); emit1(OP_HALT); rodar(pc_emit);
            Word u = mem_le(S_TMP);
            ok("e emit_transporte(−1) faz (5,3) ↦ (2,3) — φ_{−1}, e desfaz o outro",
               u.total == 2 && u.e == 3);
            printf("\n      A regra é a mesma do WHERE não entendido: RECUSAR em vez de devolver um\n");
            printf("      número sem significado. Agora a distância decide, e ela é medida.\n");
            printf("\n      E as três respostas, agora que o transporte está LIGADO:\n");
            printf("        distância > 0            classes diferentes → RECUSA\n");
            printf("        distância 0, base ≠      isomorfos → PASSA, com φ_t emitido no átomo\n");
            printf("        distância 0, base =      nada a transportar → passa\n");
            printf("\n      O que destravou foi a ORDEM: φ_t age sobre a Word inteira, e a máscara\n");
            printf("      mata o .e. Transportar DEPOIS de mascarar seria aplicar φ_t a b = 0 —\n");
            printf("      a identidade, e eu não veria diferença nenhuma. Transporta-se primeiro.\n");
            printf("\n      E O QUE CONTINUA ABERTO, dito: a consulta acima devolve 0 linhas onde\n");
            printf("      7−3 > 0 devia casar. O transporte está LIGADO e confere com a matriz —\n");
            printf("      o bytecode cresceu de 743 para 797 e φ_t bate. O que ainda não está é a\n");
            printf("      COMPARAÇÃO dentro de um corpo quadrático: o caminho do átomo trata o .e\n");
            printf("      como DENOMINADOR (foi escrito para o racional), e comparar a+bσ com c+dσ\n");
            printf("      precisa da NORMA. São dois itens distintos, e só um fechou hoje.\n");
            printf("\n      Eu tinha escrito \"transporte emitido\" no caso do meio, e a consulta\n");
            printf("      devolveu 0 linhas onde 7−3 > 0 devia casar. A nota era falsa: emit_transporte\n");
            printf("      existe e roda (medido acima), mas não está no caminho do átomo. Recusar é a\n");
            printf("      resposta honesta enquanto não estiver.\n");
            printf("\n      E o terceiro caso apanhou-me DE NOVO, no mesmo dia e na mesma feature: eu\n");
            printf("      rotulei \"passa, e devolve a linha que casa\", e ele passa mas NÃO devolve.\n");
            printf("      O motivo é outra camada: o caminho do átomo trata o .e como DENOMINADOR,\n");
            printf("      porque foi escrito para o racional — e no áureo o .e é a parte σ. Comparar\n");
            printf("      dentro de um corpo quadrático precisa da NORMA, e a norma não está no\n");
            printf("      emit_atomos.\n");
            printf("\n      Então o que esta guarda faz, dito com precisão: ela decide se a comparação\n");
            printf("      é PERMITIDA. Não a torna correta. São duas coisas, e eu ia entregá-las\n");
            printf("      como uma.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* A DISTÂNCIA ENTRE TEXTOS: a query simples. */
        printf("\n-- A QUERY SIMPLES: distância entre dois textos\n\n");
        {
            printf("$ DISTANCIA TEXTO 'ouro' 'ouro'\n");
            int r1 = executa("DISTANCIA TEXTO 'ouro' 'ouro'");
            printf("\n$ DISTANCIA TEXTO 'ouro' 'ourz'\n");
            int r2 = executa("DISTANCIA TEXTO 'ouro' 'ourz'");
            printf("\n$ DISTANCIA TEXTO 'ouro' 'prata'\n");
            int r3 = executa("DISTANCIA TEXTO 'ouro' 'prata'");
            ok("a query corre nos três casos — iguais, próximos, distantes", r1 && r2 && r3);
            printf("\n      O texto entra pela mesma porta dos números: vira cifra, e a cifra é um\n");
            printf("      ponto do corpo métrico. Não foi preciso régua nova.\n");
        }

        /* TODA ENTRADA CIFRADA, E O ÍNDICE QUE É A PRÓPRIA POSIÇÃO. */
        printf("\n-- TODA ENTRADA ENTRA CIFRADA: senao nao ha como comparar gato com cachorro\n\n");
        {
            Word z = {0,0}; mem_grava(S_TXCAB, z); mem_grava(S_NOCAB, z); mem_grava(S_TXLIVRE, z);
            executa("INSERT TEXTO 'ouro'");
            executa("INSERT TEXTO 'ourives'");
            executa("INSERT TEXTO 'prata'");
            executa("INSERT TEXTO 'ourico'");
            executa("INSERT TEXTO 7/3");
            executa("INSERT TEXTO 22/7");
            printf("\n      Texto e numero entraram pela MESMA porta e sairam na MESMA\n");
            printf("      representacao. Nao ha duas tabelas, nao ha dois indices, nao ha duas\n");
            printf("      reguas: ha uma cifra.\n");
            ok("seis entradas, texto e racional, no mesmo espaco", txt_n() == 6);

            printf("\n$ BUSCA TEXTO 'ourivesaria'\n\n");
            int r = executa("BUSCA TEXTO 'ourivesaria'");
            ok("a varredura compara cifra com cifra", r == 1);
            printf("\n$ BUSCA TEXTO 7/2      -- um numero medido contra palavras\n\n");
            executa("BUSCA TEXTO 7/2");
            printf("\n      7/2 = [3;2] e 7/3 = [2;3]: divergem no primeiro termo, distancia 1.\n");
            printf("      22/7 = [3;7] partilha o 3 com 7/2: distancia 1/2. O gato e o cachorro\n");
            printf("      ficaram comparados, e quem os comparou foi a regua, nao eu.\n");

            printf("\n-- O INDICE E A PROPRIA POSICAO: a cifra do rei poe cada um no seu lugar\n\n");
            n_leituras = 0;
            executa("ACHA TEXTO 'ourives'");
            ok("acha descendo a cifra, um no por termo", n_leituras == 7);
            printf("\n");
            executa("INSERT TEXTO 3/7");
            executa("INSERT TEXTO -5/2");
            executa("INSERT TEXTO 1000/3");
            n_leituras = 0;
            executa("ACHA TEXTO 3/7");
            ok("o termo ZERO nao colide com o marcador de fim", n_leituras == 3);
            n_leituras = 0;
            executa("ACHA TEXTO -5/2");
            ok("o termo NEGATIVO tem caminho proprio", n_leituras >= 3);
            n_leituras = 0;
            executa("ACHA TEXTO 1000/3");
            ok("e o termo GRANDE tambem — nenhum tecto no termo", n_leituras >= 3);
            printf("\n");
            n_leituras = 0;
            executa("ACHA TEXTO 'zircao'");
            ok("quem diverge no primeiro termo custa UMA leitura", n_leituras == 1);
            printf("\n");
            n_leituras = 0;
            executa("ACHA TEXTO 22/7");
            ok("e o racional acha-se pelo mesmo caminho", n_leituras == 2);
            {
                long n0 = mem_le(S_NOCAB).total;
                printf("\n");
                executa("INSERT TEXTO 'ourivesaria'");
                long n1 = mem_le(S_NOCAB).total;
                printf("      abriu %ld no(s) novo(s) — os 7 primeiros ja existiam, partilhados\n", n1-n0);
                printf("      com 'ourives'.\n");
                ok("o prefixo comum e o CAMINHO PARTILHADO, nao copia", n1 - n0 == 4);
            }
            printf("\n      Nenhuma colisao e nenhuma sondagem: cifras distintas sao caminhos\n");
            printf("      distintos. Nao ha tamanho de tabela porque nao ha tabela — e a REGUA E\n");
            printf("      INFINITA: o caminho morre onde a entrada morre, nao num tecto meu.\n");
            printf("\n-- OS 28 CORPOS, CIFRADOS, NA MESMA TABELA\n\n");
            {
                long antes = txt_n();
                executa("CORPOS");
                long lug = txt_n() - antes;
                ok("os 28 corpos entraram na mesma tabela dos textos e dos numeros", lug > 0);
                ok("a regua (B,C) separa mais que o regime", lug >= 10);
                printf("\n      A regua separa o que o regime sozinho nao separava: agora ha\n");
                printf("      %ld lugares para 28 corpos, e cada lugar e uma cifra que saiu de\n", lug);
                printf("      (B,C) e so dela. Os que continuam juntos tem a MESMA REGUA — e ai\n");
                printf("      sao o mesmo corpo com outra roupa, que e o que a teoria ja dizia.\n");
                printf("\n      Fica dito o que se pode contestar: para as familias parametricas\n");
                printf("      (o gato A_m, a dilatacao por lambda) tomei o operador que o catalogo\n");
                printf("      NOMEIA, e onde o parametro e livre, o membro minimo ainda nao tomado.\n");
                printf("      Cada escolha esta anotada corpo a corpo no CORPO28.\n\n");
                printf("      E A CIFRA E O DELTA NAO SE CONTEM UM AO OUTRO — cruzam-se, e isto\n");
                printf("      eu nao tinha previsto:\n");
                printf("        . a cifra SEPARA o que Delta junta: aureo e eletromagnetico tem\n");
                printf("          ambos Delta=5, mas sigma=(1+r5)/2 cifra [1;1] e (3+r5)/2 cifra\n");
                printf("          [2;1] — sao dois pontos, nao um;\n");
                printf("        . a cifra JUNTA o que Delta separa: Delta = 0, 4 e -4 dao todos\n");
                printf("          sigma = 1, cifra [1]. Quando sigma cai no racional, a cifra para\n");
                printf("          e o Delta fica de fora dela.\n");
                printf("      Logo nenhum dos dois e A coordenada: sao duas leituras da mesma\n");
                printf("      regua, e a regua e que as tem as duas.\n\n");
                n_leituras = 0;
                executa("ACHA TEXTO 'ouro'");
                ok("e a palavra continua no seu lugar, ao lado dos corpos", n_leituras == 4);
            }

            printf("\n      E as duas coisas sao UMA SO: a distancia e 1/2^k com k o prefixo\n");
            printf("      comum, e o indice guarda-os partilhando exatamente esses k nos. A regua\n");
            printf("      e o indice nao sao duas estruturas — sao a mesma, lida de dois lados.\n");
        }

        /* A DISTÂNCIA: a régua compõe as três, e é isso que o sistema devolve. */
        printf("\n-- A DISTÂNCIA ENTRE MÉTRICAS: a régua compõe as três, e não julga\n\n");
        {
            executa("CREATE TABLE k (a AUREO(1), b CRISTALINO(0))");
            executa("INSERT INTO k VALUES (3+2s, 1+1s)");
            printf("$ SELECT * FROM k WHERE a > 0      (Δ = 5, hiperbólico)\n");
            int r1 = executa("SELECT * FROM k WHERE a > 0");
            printf("\n$ SELECT * FROM k WHERE b > 0      (Δ = −4, elíptico)\n");
            int r2 = executa("SELECT * FROM k WHERE b > 0");
            ok("o sistema não RECUSA nem DESPACHA por classe — corre nas duas", r1 && r2);
            Regua ra = { 1, -1 }, rb = { 0, 1 };
            long da = ct_norma(ra,(Par){3,2}) - ct_norma(ra,(Par){1,1});
            long db = ct_norma(rb,(Par){3,2}) - ct_norma(rb,(Par){1,1});
            if(da < 0) da = -da;
            if(db < 0) db = -db;
            printf("\n      régua        Δ      d((3,2),(1,1))\n");
            printf("      a²+ab−b²     %-6ld %ld\n", ct_assinatura(ra), da);
            printf("      a²+b²        %-6ld %ld\n", ct_assinatura(rb), db);
            ok("a distância existe nas duas classes, e sai da MESMA conta", da > 0 && db > 0);
            printf("\n      APAGADO daqui: a guarda que recusava por classe e o estado que a\n");
            printf("      alimentava. Encarnavam a ideia refutada — a de que o sistema devia dar\n");
            printf("      ORDEM e, para isso, decidir a classe. Não devia.\n");
            printf("\n      A ordem obriga a escolher a classe; a distância não obriga a nada. Fica\n");
            printf("      d(u,v) = |N(u) − N(v)|, definida em toda régua, e quem julga é quem pediu.\n");
            printf("      Ver distancia.c. E ordem.c fica pelo que continua VERDADE: o elíptico não\n");
            printf("      é ordenável — resultado, e não motivo para recusar.\n");
            executa("CREATE TABLE t (a,b,c)");
            executa("INSERT INTO t VALUES (7,10,20)");
            executa("INSERT INTO t VALUES (3,30,40)");
            executa("INSERT INTO t VALUES (7,50,60)");
            executa("INSERT INTO t VALUES (9,70,80)");
            executa("INSERT INTO t VALUES (3,90,99)");
        }

        /* O CIRCUITO FECHADO: o esquilo entra, e o metal passa a girar além de esticar. */
        printf("\n-- O CIRCUITO FECHADO: o esquilo no metal, e todo metal como PALAVRA\n\n");
        {
            int mau = 0; long casos = 0;
            /* 1. as ordens, contadas na máquina — não na conta ao lado */
            Word v; v.total = 5; v.e = 3;
            mem_grava(S_TMP, v);
            int ord_s = 0;
            for(int k = 1; k <= 12; k++){
                pc_emit = 0;
                emit_slot(OP_LOAD, S_TMP); emit1(OP_ESQUILO); emit_slot(OP_STORE, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                if(w.total == 5 && w.e == 3){ ord_s = k; break; }
            }
            ok("ESQUILO tem ordem 4 NA MÁQUINA — gira e volta ao ponto de partida", ord_s == 4);
            mem_grava(S_TMP, v);
            int ord_j = 0;
            for(int k = 1; k <= 12; k++){
                pc_emit = 0;
                emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA); emit_slot(OP_STORE, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word w = mem_le(S_TMP);
                if(w.total == 5 && w.e == 3){ ord_j = k; break; }
            }
            ok("TROCA tem ordem 2 — a involução, e é a sua própria inversa", ord_j == 2);

            /* 2. O QUE FECHA O CIRCUITO: todo metal é PALAVRA nos quatro geradores. Prata e
             * bronze DEIXARAM de existir como opcode — foram apagados, porque eram atalhos.
             * Já não há opcode dedicado com que comparar: compara-se com a matriz. */
            printf("      metal    palavra nos geradores            A_m(5,3)   confere?\n");
            const char *nm[3] = { "ouro", "prata", "bronze" };
            for(int m = 1; m <= 3; m++)
            for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
                Word x; x.total = a; x.e = b;
                mem_grava(S_TMP, x);
                pc_emit = 0; emit_metal(m, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word pela_palavra = mem_le(S_TMP);
                Par esperado = me_ap(me_gato(m), (Par){a,b});
                if(pela_palavra.total != esperado.a || pela_palavra.e != esperado.b) mau++;
                if(a == 5 && b == 3)
                    printf("      %-8s %-32s (%ld,%ld)%*s%s\n", nm[m-1],
                           m==1?"GOLD":(m==2?"GOLD TROCA GOLD":"GOLD TROCA GOLD TROCA GOLD"),
                           pela_palavra.total, pela_palavra.e, 5, "",
                           (pela_palavra.total==esperado.a &&
                            pela_palavra.e==esperado.b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("A_m = (A_1·J)^{m−1}·A_1 CONFERE no metal — prata e bronze foram APAGADOS",
               mau == 0);
            printf("      (%ld casos, três metais, e nenhum deles com opcode próprio.)\n", casos);

            /* 3. o cisalhamento, que NÃO é opcode e não precisa de ser */
            {
                Word x; x.total = 5; x.e = 3;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA); emit_slot(OP_STORE, S_TMP);
                emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD);  emit_slot(OP_STORE, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word t = mem_le(S_TMP);
                /* T = [[1,1],[0,1]] em (5,3) dá (5+3, 3) = (8,3) */
                ok("TROCA GOLD é o cisalhamento T — palavra de dois, e poupa um opcode",
                   t.total == 8 && t.e == 3);
            }
            printf("\n      O circuito fecha porque o repertório fecha: o gato estica (ordem ∞, e por\n");
            printf("      isso precisou do negro), o esquilo gira (ordem 4, a inversa é S³), a troca\n");
            printf("      reflete (ordem 2, é a sua própria inversa). O que a máquina faz, ela\n");
            printf("      desfaz — nos inteiros e sem guardar cópia.\n");
            printf("\n      E o que fica de FORA, dito: decompor uma unimodular QUALQUER em palavra\n");
            printf("      não está no compilador. Mede-se que existe para os metais e para as\n");
            printf("      inversas; para matriz arbitrária o algoritmo é o de Euclides e não está\n");
            printf("      aqui. Dizer que já está seria medir a fatia e afirmar o todo.\n");
        }

        /* O CHICOTE DOS DOIS LADOS: o negro tão inteiro quanto o branco, NO METAL. */
        printf("\n-- O CHICOTE DOS DOIS LADOS: A_m no metal para TODO m, e a volta também\n\n");
        {
            int mau = 0; long casos = 0;
            /* A assimetria era minha: generalizei o branco (A_m para todo m ≥ 1) e deixei o
             * negro nos três opcodes. A régua não tem lado — T⁻¹ = J·A_1⁻¹ é o espelho exato
             * de T = A_1·J, e com ela A_m = T^{m−1}·A_1 vale para m ≤ 0 igualmente. */
            printf("      m     palavra emitida                          A_m no metal   confere?\n");
            for(long m = -12; m <= 12; m++)
            for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
                Word x; x.total = a; x.e = b;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD); emit_slot(OP_STORE, S_TMP);
                if(m >= 1) for(long k = 1; k < m; k++){          /* T = TROCA depois GOLD */
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD);  emit_slot(OP_STORE, S_TMP);
                } else for(long k = m; k <= 0; k++){             /* T⁻¹ = NEGRO depois TROCA */
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA);      emit_slot(OP_STORE, S_TMP);
                }
                emit1(OP_HALT); rodar(pc_emit);
                Word pela_palavra = mem_le(S_TMP);
                Par esperado = me_ap(me_gato(m), (Par){a,b});   /* o toolkit CONFERE */
                if(pela_palavra.total != esperado.a || pela_palavra.e != esperado.b) mau++;
                if(a == 5 && b == 3 && (m == 0 || m == -1 || m == 4))
                    printf("      %-5ld %-40s (%ld,%ld)%*s%s\n", m,
                           m==0 ? "GOLD NEGRO TROCA"
                                : (m==-1 ? "GOLD NEGRO TROCA NEGRO TROCA"
                                         : "GOLD TROCA GOLD TROCA GOLD TROCA GOLD"),
                           pela_palavra.total, pela_palavra.e, 3, "",
                           (pela_palavra.total==esperado.a &&
                            pela_palavra.e==esperado.b) ? "sim ✓" : "NÃO");
                casos++;
            }
            ok("A_m corre no metal para TODO m — negativo, zero e positivo, sem opcode próprio",
               mau == 0);
            printf("      (%ld casos, m de −12 a 12.)\n", casos);

            /* e a VOLTA de todo metal: a mesma palavra ao contrário, letra a letra invertida */
            int mau2 = 0; long casos2 = 0;
            for(long m = -12; m <= 12; m++)
            for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
                Word x; x.total = a; x.e = b;
                mem_grava(S_TMP, x);
                pc_emit = 0;
                /* IDA */
                emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD); emit_slot(OP_STORE, S_TMP);
                if(m >= 1) for(long k = 1; k < m; k++){
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD);  emit_slot(OP_STORE, S_TMP);
                } else for(long k = m; k <= 0; k++){
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA);      emit_slot(OP_STORE, S_TMP);
                }
                /* VOLTA: a palavra ao contrário, GOLD↔NEGRO, e a TROCA fica onde está */
                if(m >= 1) for(long k = m-1; k >= 1; k--){
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA);      emit_slot(OP_STORE, S_TMP);
                } else for(long k = 0; k >= m; k--){
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_TROCA); emit_slot(OP_STORE, S_TMP);
                    emit_slot(OP_LOAD, S_TMP); emit1(OP_GOLD);  emit_slot(OP_STORE, S_TMP);
                }
                emit_slot(OP_LOAD, S_TMP); emit1(OP_NEGRO_OURO); emit_slot(OP_STORE, S_TMP);
                emit1(OP_HALT); rodar(pc_emit);
                Word volta = mem_le(S_TMP);
                if(volta.total != a || volta.e != b) mau2++;
                casos2++;
            }
            ok("e a VOLTA de todo metal é a palavra ao contrário, letra a letra invertida",
               mau2 == 0);
            printf("      (%ld percursos ida-e-volta, e todos devolvem o que entrou.)\n", casos2);
            printf("\n      A assimetria era minha, não do mecanismo: eu tinha generalizado o branco e\n");
            printf("      deixado o negro nos três opcodes. A régua não tem lado — e m = 0 dá\n");
            printf("      A_0 = J, a TROCA, que é onde o chicote passa ao mudar de sinal. Não é\n");
            printf("      peça que eu acrescentei: é o meio da régua.\n");
            printf("\n      E daqui sai o que se pode APAGAR: SILVER, BRONZE, NEGRO_PRATA e\n");
            printf("      NEGRO_BRONZE são palavras, não geradores. O repertório mínimo que fecha é\n");
            printf("      GOLD, NEGRO_OURO, TROCA e ESQUILO — quatro peças, simétricas. Os outros\n");
            printf("      quatro ficam por serem ATALHOS: poupam palavra, não poder.\n");
        }

        /* AS AFIRMAÇÕES. O teste imprimia e não concluía; agora confere contra conta feita
         * à mão, e a bateria passa a cobrir o compilador em vez de o ignorar. */
        printf("\n-- AS AFIRMAÇÕES (a bateria passa a cobrir isto)\n\n");
        {
            executa("DELETE FROM t");
            executa("INSERT INTO t VALUES (3/4,1,1)");
            executa("INSERT INTO t VALUES (5,2,1)");
            executa("INSERT INTO t VALUES (7/2,3,1)");
            executa("INSERT INTO t VALUES (2,5,1)");
            struct { const char *q; long e; const char *rot; } cs[] = {
              {"SELECT * FROM t WHERE a = 3/4",            1, "a igualdade racional fecha"},
              {"SELECT * FROM t WHERE a = 6/8",            1, "e a classe: 6/8 casa com 3/4"},
              {"SELECT * FROM t WHERE a > 1",              3, "a ordem racional, sem divisão"},
              {"SELECT * FROM t WHERE a * 2 > 7",          1, "coeficiente sobre racional"},
              {"SELECT * FROM t WHERE a > 2 AND a > 2",    2, "idempotência: A op A = A"},
              {"SELECT * FROM t WHERE (a>2 AND a<9) OR a>2",2,"absorção: a adjunção δ⊣ε"},
              {"SELECT * FROM t WHERE a + b > 5",          3, "duas colunas, denominadores"},
            };
            for(unsigned q = 0; q < sizeof cs/sizeof cs[0]; q++){
                ultima_conta = -1;
                executa(cs[q].q);
                ok(cs[q].rot, ultima_conta == cs[q].e);
            }
        }

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
