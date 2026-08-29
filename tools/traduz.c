/* traduz.c — A SUBIDA: C sobe para WASM. O dual do `wasm_desce`.
 *
 * O Aarão: «isso tem que rodar no front do cliente, PDF gerado no front via WASM, sem servidor
 * — é o correto, não há motivo para ser diferente».
 *
 * E logo a seguir a correcção que dá o nome a este ficheiro:
 *
 *     «não é compilador. O interpretador não tem tempo nem dissipação, não compila, só
 *      traduz — seria melhor tradutor até, que é o que é.»
 *
 * Está certo, e não é uma questão de palavra. COMPILAR TEM UM TEMPO: uma etapa que corre
 * antes, produz outra coisa, e deita fora a estrutura de partida para dar lugar à de chegada.
 * O que se deita fora não volta, e o que não volta DISSIPA — é o mesmo critério do
 * `corpo_analitico.tex`, «apagar é a única operação que não se desfaz».
 *
 * Isto não faz nada disso. Não há representação intermédia, não há AST, não há alocador de
 * registadores, não há passagem de optimização: lê e emite ao mesmo tempo, numa volta só.
 * O C e o WASM são DUAS RÉGUAS DO MESMO OBJECTO, e ir de uma à outra é reexprimir, não
 * transformar. Por isso a tradução tem dual, e o dual mede-se: o módulo emitido **devolve**
 * as assinaturas que entraram, resíduo 0. Um compilador não devolveria — teria dissipado.
 *
 * E METADE DA ESTRADA JÁ ESTAVA ESCRITA. O `chessb.c` §C4:
 *
 *     «o wasm empilha; nós temos A, B, R. A tradução não é uma tabela de opcodes — é a
 *      observação de que uma pilha de profundidade 2 É o par (A,B) [...] A nossa ISA já era
 *      de pilha, com a pilha escrita por extenso.»
 *
 * A outra metade é a mesma observação um andar acima: **uma expressão em C é uma árvore, e
 * lê-la por baixo É empilhar.** A pilha não se constrói — ela já lá estava, na árvore.
 *
 *   ── o que sobe ──────────────────────────────────────────────────────────────────────
 *   tipos      int (i32), long (i64), double (f64), void
 *   funções    parâmetros, locais, chamadas (incluindo para a frente), recursão
 *   operadores + - * / %  == != < > <= >=  && || !  & | ^ << >>  = += -= *= /=  ++ --
 *   controlo   if/else, while, for, do/while, break, continue, return
 *              (o `switch` não é construção nova: é a mesma cadeia de if/else —
 *               catalogo.tex; a volta wasm-c reconstitui if, nunca br_table)
 *   ── e o que dele se mede (tests/traduz_volta.js) ────────────────────────────────────
 *   os DOIS CAMINHOS: o mesmo ficheiro pelo `cc` do sistema e por este, e os números batem
 *   a VOLTA:          o módulo devolve as assinaturas — porque traduzir preserva
 *
 *   cc -O2 -std=c99 -Wall traduz.c -o bin/traduz
 *   ./bin/traduz entrada.c -o saida.wasm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include "../lib/disco.h"   /* «o ficheiro É o vector»: o endereço é CONSTANTE, não variável */

/* ─────────────────────────────────────────────────────── o buffer, e o LEB128 dos dois sinais */

#define CAP_COD  (1<<20)
#define CAP_MOD  (1<<21)
#define CAP_SRC  (1<<23)

typedef struct { unsigned char *b; long n, cap; } Buf;

/* ── ZERO EM .BSS: OS REGISTOS E OS VECTORES VIVEM NO DISCO ──────────────────────────
 *
 * O Aarão: «tira esse .bss — a especificação é clara, as instruções vão nos slots do
 * disco». Estava medido: 148 045 bytes de .bss, símbolo a símbolo, contra a regra que o
 * disco.h enuncia — «um ponteiro global são 8 bytes em .bss, e 8 não é 0».
 *
 * Os vectores ganham cada um a sua base (48…60); os escalares — os registos do tradutor —
 * vivem juntos num slot só, este Reg. Os valores iniciais semeiam-se em prende_tudo,
 * porque o disco PERSISTE entre corridas e a .bss zerava sozinha: aqui zera-se e
 * semeia-se à mão, no mesmo sítio onde as bases se prendem. */
typedef struct {
    long pos, linha, lit_n, txt_n;
    long disco, img_ini, img_fim;
    long quadro, quadro_usado, quadro_fita;    /* quadro_fita: a fita dos `...` no quadro */
    long m_frase;                              /* onde começou a frase que está a ser escrita */
    long mn, mp;
    const unsigned char *m;                    /* o módulo lido, na descida */
    int nfpt, nestr, nfun, fun_act, nslot, nloc, nlaco, nlab, np, nq;
    int n_end, n_ass, n_mac, n_fita, fita_sitio;
    int em_braco, prof, prim_end, precisa_fita;
    int sp_slot;
    int tmp_ret;                               /* onde o valor espera enquanto o quadro fecha */
    int disco_publico;                         /* um global sem `static` pede o disco na porta */
    int campo_vec;                             /* o último campo lido era um vector? */
    int n_imp;                                 /* imports env (ex.: __fich_miss); deslocam o índice das funções */
    Buf cod, mod, sec, corpos;
} Reg;
#define RG            DISCO_FIXO(Reg, 61)
#define POS           (RG->pos)
#define LINHA         (RG->linha)
#define LIT_N         (RG->lit_n)
#define TXT_N         (RG->txt_n)
#define DISCO         (RG->disco)
#define IMG_INI       (RG->img_ini)
#define IMG_FIM       (RG->img_fim)
#define QUADRO        (RG->quadro)
#define QUADRO_USADO  (RG->quadro_usado)
#define QUADRO_FITA   (RG->quadro_fita)
#define M_FRASE       (RG->m_frase)
#define MN            (RG->mn)
#define MP            (RG->mp)
#define M             (RG->m)
#define NFPT          (RG->nfpt)
#define NESTR         (RG->nestr)
#define NFUN          (RG->nfun)
#define FUN_ACT       (RG->fun_act)
#define NSLOT         (RG->nslot)
#define NLOC          (RG->nloc)
#define NLACO         (RG->nlaco)
#define NLAB          (RG->nlab)
#define NP            (RG->np)
#define NQ            (RG->nq)
#define N_END         (RG->n_end)
#define N_ASS         (RG->n_ass)
#define N_MAC         (RG->n_mac)
#define N_FITA        (RG->n_fita)
#define FITA_SITIO    (RG->fita_sitio)
#define EM_BRACO      (RG->em_braco)
#define PROF          (RG->prof)          /* quantos blocos wasm abertos agora */
#define PRIM_END      (RG->prim_end)
#define PRECISA_FITA  (RG->precisa_fita)
#define SP_SLOT       (RG->sp_slot)
#define TMP_RET       (RG->tmp_ret)
#define DISCO_PUBLICO (RG->disco_publico)
#define CAMPO_VEC     (RG->campo_vec)
#define N_IMP         (RG->n_imp)
#define WASM_FUN(f)   ((f) + N_IMP)
#define COD           (RG->cod)
#define MOD           (RG->mod)
#define SEC           (RG->sec)
#define CORPOS        (RG->corpos)
#define T             (*DISCO_FIXO(Tok, 58))
#define SRC           DISCO_FIXO(char, 60)

static void bput(Buf *o, unsigned c){
    if(o->n >= o->cap){ fprintf(stderr, "traduz: buffer cheio\n"); exit(2); }
    o->b[o->n++] = (unsigned char)c;
}
static void bmany(Buf *o, const void *p, long k){
    if(o->n + k > o->cap){ fprintf(stderr, "traduz: buffer cheio\n"); exit(2); }
    memcpy(o->b + o->n, p, (size_t)k); o->n += k;
}
/* o LEB128 sem sinal — o mesmo do `leb` do chessb.c, ao contrário */
static void bu(Buf *o, unsigned long v){
    do { unsigned c = v & 0x7F; v >>= 7; if(v) c |= 0x80; bput(o, c); } while(v);
}
/* e o COM sinal: o bit 0x40 do último byte É o sinal, e é por isso que o par se fecha —
 * o valor conta-se por passos e o sinal viaja no passo, não numa casa à parte */
static void bs(Buf *o, long v){
    int mais = 1;
    while(mais){
        unsigned c = (unsigned)(v & 0x7F);
        v >>= 7;
        if((v == 0 && !(c & 0x40)) || (v == -1 && (c & 0x40))) mais = 0;
        else c |= 0x80;
        bput(o, c);
    }
}
/* inserir no MEIO do que já se emitiu: é assim que a promoção do operando esquerdo entra
 * depois de ele já estar escrito — sem segunda passagem e sem AST */
static void bins(Buf *o, long onde, const unsigned char *p, long k){
    if(o->n + k > o->cap){ fprintf(stderr, "traduz: buffer cheio\n"); exit(2); }
    memmove(o->b + onde + k, o->b + onde, (size_t)(o->n - onde));
    memcpy(o->b + onde, p, (size_t)k);
    o->n += k;
}

/* ─────────────────────────────────────────────────────────────────── os tipos e os opcodes */

/* o `unsigned char` NÃO é o `char`: um lê 0..255 e o outro −128..127. Ler um byte de um
 * ficheiro com sinal é o mesmo defeito do `& 255` — só que aqui a linguagem já o diz. */
enum { TI32 = 0, TI64 = 1, TF64 = 2, TVOID = 3, TI8 = 4, TU8 = 5 };
/* a base passa a ter uma casa maior: as cinco de sempre, e daí para cima uma por estrutura */
#define BASE(t)   ((t) & 255)
#define PTR(t)    ((t) >> 8)
#define MKT(b,p)  ((b) | ((p) << 8))
#define EST0      8
#define FPT0      64
#define E_ESTR(t) (!PTR(t) && BASE(t) >= EST0 && BASE(t) < FPT0)
#define E_FPT(t)  (!PTR(t) && BASE(t) >= FPT0)

/* ── O PONTEIRO DE FUNÇÃO É UM ÍNDICE ────────────────────────────────────────────────
 * Em wasm não se salta para um endereço: salta-se para uma ENTRADA DE TABELA, e o que se
 * passa é o número dela. É a mesma coisa que a ISA faz com o `pc` — o destino é dado, e o
 * dado é um número. A assinatura vai na instrução, para o motor poder recusar o que não
 * bate: um salto para a função errada falha alto em vez de correr o que calhar. */
typedef struct { int ret, npar, par[8]; } Perfil;
#define FPT        DISCO_FIXO(Perfil, 48)

static int acha_fpt(int ret, int npar, const int *par){
    for(int i = 0; i < NFPT; i++){
        if(FPT[i].ret != ret || FPT[i].npar != npar) continue;
        int igual = 1;
        for(int k = 0; k < npar; k++) if(FPT[i].par[k] != par[k]) igual = 0;
        if(igual) return i;
    }
    if(NFPT >= 64) return -1;
    FPT[NFPT].ret = ret; FPT[NFPT].npar = npar;
    for(int k = 0; k < npar; k++) FPT[NFPT].par[k] = par[k];
    return NFPT++;
}

/* ── A ESTRUTURA ────────────────────────────────────────────────────────────────────
 * Um `struct` não é um valor do wasm — não há lá tipo composto. Mas também não precisa de
 * ser: uma estrutura é um pedaço de disco com nomes para os deslocamentos, e cada campo é um
 * MOVE no endereço somado. Ao descer ela desaparece: fica `*(int*)(e + k)`, que é o que ela
 * sempre foi por baixo. */
typedef struct {
    char nome[64];
    int  ncampos;
    char cnome[48][64];
    int  ctipo[48];
    long cdesl[48];
    long cbytes[48];   /* o tamanho declarado de cada campo — o sizeof de um campo-vector */
    int  cvec[48];     /* o campo é vector? decai em ponteiro, como um slot-vector decai */
    long tam;
} Estrutura;
#define ESTR       DISCO_FIXO(Estrutura, 44)

static int acha_estr(const char *n){
    for(int i = 0; i < NESTR; i++) if(!strcmp(ESTR[i].nome, n)) return i;
    return -1;
}

static int tam_de(int t){
    if(PTR(t)) return 4;
    if(BASE(t) >= FPT0) return 4;
    if(BASE(t) >= EST0) return (int)ESTR[BASE(t) - EST0].tam;
    switch(BASE(t)){ case TI8: case TU8: return 1; case TI32: return 4; case TI64: case TF64: return 8; }
    return 0;
}
/* na pilha do wasm há três tipos: o `char` e o endereço viajam em i32 */
static int aritm(int t){
    if(PTR(t)) return TI32;
    if(BASE(t) >= EST0) return TI32;           /* estrutura e ponteiro de função: um número */
    return (BASE(t) == TI8 || BASE(t) == TU8) ? TI32 : BASE(t);
}
static unsigned val_t(int t){
    int a = aritm(t);
    return a == TI32 ? 0x7F : a == TI64 ? 0x7E : 0x7C;
}

/* a conversão de a para b — um opcode, e é o mesmo em qualquer sentido do par */
static unsigned conv_op(int a, int b){
    a = aritm(a); b = aritm(b);
    if(a == b) return 0;
    if(a == TI32 && b == TI64) return 0xAC;   /* i64.extend_i32_s   */
    if(a == TI32 && b == TF64) return 0xB7;   /* f64.convert_i32_s  */
    if(a == TI64 && b == TF64) return 0xB9;   /* f64.convert_i64_s  */
    if(a == TI64 && b == TI32) return 0xA7;   /* i32.wrap_i64       */
    if(a == TF64 && b == TI32) return 0xAA;   /* i32.trunc_f64_s    */
    if(a == TF64 && b == TI64) return 0xB0;   /* i64.trunc_f64_s    */
    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────── o lexer */

enum { TK_FIM, TK_NUM, TK_REAL, TK_ID, TK_PUN, TK_TEXTO };


/* f64 como BITS (IEEE754), sem o tipo double no compilador */
static uint64_t F64_0 = 0ULL;
static uint64_t F64_1 = 0x3FF0000000000000ULL;
static uint64_t f64_neg_bits(uint64_t b){ return b ^ 0x8000000000000000ULL; }

/* divisão exacta sem 128 bits: computa q = floor((num<<shift)/den) e r = (num<<shift)%den */
static void divmod_shift_u64(uint64_t num, uint64_t den, unsigned shift, uint64_t *q, uint64_t *r){
    uint64_t qq = 0, rr = 0;
    for(int i = 63 + (int)shift; i >= 0; i--){
        uint64_t bit = 0;
        if((unsigned)i >= shift) bit = (num >> ((unsigned)i - shift)) & 1ULL;
        rr = (rr << 1) | bit;
        qq <<= 1;
        if(rr >= den){ rr -= den; qq |= 1ULL; }
    }
    *q = qq;
    *r = rr;
}

static uint64_t f64_bits_pq(uint64_t num, uint64_t den, int neg){
    if(num == 0) return neg ? 0x8000000000000000ULL : 0ULL;
    if(den == 0) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL; /* ±inf */
    int uexp = 0;
    while(num < den){ num <<= 1; uexp--; if(uexp < -1075) return neg ? 0x8000000000000000ULL : 0ULL; }
    while(den && num >= (den << 1)){
        den <<= 1;
        uexp++;
        if(uexp > 1024) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
    }
    /* 1 <= num/den < 2 */
    uint64_t mant, rem;
    divmod_shift_u64(num, den, 52, &mant, &rem); /* mant = floor((num<<52)/den) */
    if((rem << 1) > den || ((rem << 1) == den && (mant & 1ULL))) mant++;
    if(mant == (1ULL << 53)){ mant >>= 1; uexp++; }
    int bexp = uexp + 1023;
    if(bexp <= 0){
        if(bexp < -52) return neg ? 0x8000000000000000ULL : 0ULL;
        unsigned shift = (unsigned)(52 + bexp); /* bexp<=0 => shift in [0,52] */
        uint64_t dummy;
        divmod_shift_u64(num, den, shift, &mant, &dummy);
        uint64_t out = mant & ((1ULL << 52) - 1);
        return neg ? out | 0x8000000000000000ULL : out;
    }
    if(bexp >= 2047) return neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
    uint64_t out = ((uint64_t)(bexp & 0x7FF) << 52) | (mant & ((1ULL << 52) - 1));
    return neg ? out | 0x8000000000000000ULL : out;
}

static uint64_t f64_bits_from_i(long v){
    return f64_bits_pq(v < 0 ? (uint64_t)(-v) : (uint64_t)v, 1, v < 0);
}

static uint64_t f64_de_texto(const char *s, char **fim){
    const char *p = s;
    while(*p == ' ' || *p == '\t') p++;
    int neg = 0;
    if(*p == '-'){ neg = 1; p++; }
    else if(*p == '+') p++;
    /* caminho 1: hex-float C99 (emitido por real_txt_bits) */
    if(p[0] == '0' && (p[1] == 'x' || p[1] == 'X')){
        p += 2;
        uint64_t mant_hex = 0;
        int frac_len = 0;
        int houve = 0;

        while(isxdigit((unsigned char)*p)){
            int d;
            if(*p >= '0' && *p <= '9') d = *p - '0';
            else if(*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
            else d = *p - 'A' + 10;
            mant_hex = (mant_hex << 4) | (uint64_t)d;
            p++;
            houve = 1;
        }

        if(*p == '.'){
            p++;
            while(isxdigit((unsigned char)*p)){
                int d;
                if(*p >= '0' && *p <= '9') d = *p - '0';
                else if(*p >= 'a' && *p <= 'f') d = *p - 'a' + 10;
                else d = *p - 'A' + 10;
                mant_hex = (mant_hex << 4) | (uint64_t)d;
                p++;
                frac_len++;
                houve = 1;
            }
        }

        if(!houve){
            if(fim) *fim = (char*)s;
            return 0;
        }

        if(*p == 'p' || *p == 'P'){
            p++;
            int es = 0;
            if(*p == '+' || *p == '-'){ es = (*p == '-'); p++; }
            int E = 0;
            while(*p >= '0' && *p <= '9'){ E = E * 10 + (*p - '0'); p++; }
            if(es) E = -E;

            int shift = E - 4 * frac_len; /* valor = M * 2^shift */

            if(mant_hex == 0){
                if(fim) *fim = (char*)p;
                return neg ? 0x8000000000000000ULL : 0ULL;
            }

            int h = 63 - __builtin_clzll(mant_hex); /* mant_hex tem bits em [h..0] com h>=0 */
            int exponent2 = h + shift;

            if(exponent2 > 1023){
                if(fim) *fim = (char*)p;
                uint64_t inf = neg ? 0xFFF0000000000000ULL : 0x7FF0000000000000ULL;
                return inf;
            }
            if(exponent2 < -1074){
                if(fim) *fim = (char*)p;
                return neg ? 0x8000000000000000ULL : 0ULL;
            }

            /* normalização/rounding para mantissa de 53 bits */
            uint64_t mant53 = 0;
            if(h > 52){
                unsigned r = (unsigned)(h - 52);
                uint64_t rem = mant_hex & ((1ULL << r) - 1);
                mant53 = mant_hex >> r;
                uint64_t half = 1ULL << (r - 1);
                if(rem > half || (rem == half && (mant53 & 1ULL))) mant53++;
            } else {
                mant53 = mant_hex << (52 - h);
            }
            if(mant53 == (1ULL << 53)){
                mant53 >>= 1;
                exponent2++;
            }

            if(exponent2 >= -1022){
                uint64_t exp_field = (uint64_t)(exponent2 + 1023);
                uint64_t frac_field = mant53 & ((1ULL << 52) - 1);
                uint64_t out = (exp_field << 52) | frac_field;
                if(neg) out |= 0x8000000000000000ULL;
                if(fim) *fim = (char*)p;
                return out;
            }

            /* subnormal: exponent field 0, valor = frac * 2^-1074 */
            int shift_sub = (-1022 - exponent2); /* >=1 */
            if(shift_sub >= 64){
                if(fim) *fim = (char*)p;
                return neg ? 0x8000000000000000ULL : 0ULL;
            }
            uint64_t rem = 0;
            uint64_t frac_field = mant53;
            if(shift_sub > 0){
                rem = mant53 & ((1ULL << shift_sub) - 1);
                frac_field = mant53 >> shift_sub;
                uint64_t half = 1ULL << (shift_sub - 1);
                if(rem > half || (rem == half && (frac_field & 1ULL))) frac_field++;
                /* se arredondar para o próximo normal, cai no mínimo normal */
                if(frac_field == (1ULL << 52)){
                    frac_field = 0;
                    uint64_t out = (1ULL << 52);
                    if(neg) out |= 0x8000000000000000ULL;
                    if(fim) *fim = (char*)p;
                    return out;
                }
            }
            frac_field &= ((1ULL << 52) - 1);
            if(neg) frac_field |= 0x8000000000000000ULL;
            if(fim) *fim = (char*)p;
            return frac_field;
        }
        /* formato hex sem 'p': não é hex-float reconhecido */
    }

    /* caminho 2: decimal (o comportamento original) */
    uint64_t num = 0; int frac = 0, houve = 0;
    while(*p >= '0' && *p <= '9'){ num = num * 10 + (uint64_t)(*p - '0'); p++; houve = 1; }
    if(*p == '.'){ p++; while(*p >= '0' && *p <= '9'){ num = num * 10 + (uint64_t)(*p - '0'); frac++; p++; houve = 1; } }
    if(!houve){ if(fim) *fim = (char*)s; return 0; }
    int exp = 0;
    if(*p == 'e' || *p == 'E'){
        const char *t = p + 1; int es = 0;
        if(*t == '+' || *t == '-'){ es = (*t == '-'); t++; }
        if(*t >= '0' && *t <= '9'){
            int e = 0; while(*t >= '0' && *t <= '9'){ e = e * 10 + (*t - '0'); t++; }
            exp = es ? -e : e; p = t;
        }
    }
    uint64_t den = 1;
    int e10 = exp - frac;
    if(e10 >= 0){ for(int i = 0; i < e10; i++) num *= 10; }
    else { for(int i = 0; i < -e10; i++) den *= 10; }
    if(fim) *fim = (char*)p;
    return f64_bits_pq(num, den, neg);
}

static void real_txt_bits(uint64_t bits, char *o, long cap){
    /* hex-float garante reconstituição exacta dos bits no cc do sistema */
    int sign = (int)((bits >> 63) & 1);
    uint64_t exp = (bits >> 52) & 0x7FF;
    uint64_t frac = bits & ((1ULL << 52) - 1);

    if(exp == 0x7FF){
        if(frac == 0) snprintf(o, (size_t)cap, "%sinf", sign ? "-" : "");
        else          snprintf(o, (size_t)cap, "%snan", sign ? "-" : "");
        return;
    }
    if(exp == 0){
        if(frac == 0){
            snprintf(o, (size_t)cap, "%s0.0", sign ? "-" : "");
            return;
        }
        /* subnormal: value = frac * 2^-1074, e em hex-float isso corresponde a p=-1022 */
        snprintf(o, (size_t)cap, "%s0x0.%013" PRIx64 "p-1022", sign ? "-" : "", frac);
        return;
    }

    int e = (int)exp - 1023;
    snprintf(o, (size_t)cap, "%s0x1.%013" PRIx64 "p%+d", sign ? "-" : "", frac, e);
}


typedef struct { int k; long i; uint64_t d; int longo; char s[64]; } Tok;

/* SRC, POS, LINHA e o Tok corrente vivem no disco — ver o bloco dos registos, no topo. */

/* os bytes do último texto lido. Ficam aqui e não no `Tok` porque um texto não tem tamanho
 * fixo — e quem o quiser copia-o ANTES de pedir o token seguinte. */
#define LIT        DISCO_FIXO(unsigned char, 30)

/* a barra invertida: o que ela diz é UM byte, e o byte é que conta. Sem isto um `\n` entrava
 * como dois caracteres e o texto que sobe já não era o que estava escrito. */
static int escapa(long *p){
    int c = SRC[(*p)++];
    if(c != '\\') return c;
    c = SRC[(*p)++];
    switch(c){
        case 'n': return '\n';   case 't': return '\t';   case 'r': return '\r';
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            int v = c - '0', k = 1;
            while(k < 3 && SRC[*p] >= '0' && SRC[*p] <= '7'){ v = v*8 + (SRC[(*p)++] - '0'); k++; }
            return v & 255;
        }
        case 'x': {
            int v = 0;
            while(isxdigit((unsigned char)SRC[*p])){
                int d = SRC[(*p)++];
                d = (d <= '9') ? d - '0' : (d | 32) - 'a' + 10;
                v = v*16 + d;
            }
            return v & 255;
        }
        case 'a': return 7;  case 'b': return 8;  case 'f': return 12;  case 'v': return 11;
        default:  return c;                       /* \\  \"  \'  e o resto: o próprio */
    }
}

static void erro(const char *m){
    fprintf(stderr, "traduz: linha %ld: %s (perto de \"%s\")\n", LINHA, m, T.s);
    exit(1);
}

/* os operadores de dois e três caracteres, os mais compridos primeiro */
static const char *PUNS[] = {
    "...", "<<=", ">>=", "->", "++", "--", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||",
    "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", 0
};

static void avanca(void){
    /* o branco e os comentários — os dois, que o `//` é tão C99 como o resto */
    for(;;){
        while(SRC[POS] && isspace((unsigned char)SRC[POS])){ if(SRC[POS]=='\n') LINHA++; POS++; }
        if(SRC[POS] == '/' && SRC[POS+1] == '/'){ while(SRC[POS] && SRC[POS] != '\n') POS++; continue; }
        if(SRC[POS] == '/' && SRC[POS+1] == '*'){
            POS += 2;
            while(SRC[POS] && !(SRC[POS]=='*' && SRC[POS+1]=='/')){ if(SRC[POS]=='\n') LINHA++; POS++; }
            if(SRC[POS]) POS += 2;
            continue;
        }
        if(SRC[POS] == '#'){ while(SRC[POS] && SRC[POS] != '\n') POS++; continue; }
        break;
    }
    T.s[0] = 0;
    if(!SRC[POS]){ T.k = TK_FIM; strcpy(T.s, "<fim>"); return; }

    if(isdigit((unsigned char)SRC[POS])){
        char *fim;
        long i = strtol(SRC + POS, &fim, 0);
        if(*fim == '.' || *fim == 'e' || *fim == 'E'){
            T.k = TK_REAL; T.d = f64_de_texto(SRC + POS, &fim);
        } else {
            T.k = TK_NUM; T.i = i; T.longo = 0;
            /* o `L` nao e' decoracao: ele DIZ o tipo, e sem ele a descida nao sabe devolver
             * um `i64.const` — sairia um i32 promovido, que sao outros bytes */
            while(*fim == 'L' || *fim == 'l' || *fim == 'U' || *fim == 'u'){
                if(*fim == 'L' || *fim == 'l') T.longo = 1;
                fim++;
            }
        }
        long k = fim - (SRC + POS); if(k > 63) k = 63;
        memcpy(T.s, SRC + POS, (size_t)k); T.s[k] = 0;
        POS = fim - SRC;
        return;
    }
    if(SRC[POS] == '"'){
        POS++;
        LIT_N = 0;
        while(SRC[POS] && SRC[POS] != '"'){
            if(LIT_N < 8191) LIT[LIT_N++] = (unsigned char)escapa(&POS);
            else escapa(&POS);
        }
        if(SRC[POS] == '"') POS++;
        /* e a JUNÇÃO: "ab" "cd" em C é um texto só, e quem escreve linhas longas conta com
         * isso. Junta-se aqui, que é onde a linguagem o diz — não no meio da expressão. */
        for(;;){
            long g = POS;
            while(SRC[g] && isspace((unsigned char)SRC[g])) g++;
            if(SRC[g] != '"') break;
            POS = g + 1;
            while(SRC[POS] && SRC[POS] != '"'){
                if(LIT_N < 8191) LIT[LIT_N++] = (unsigned char)escapa(&POS);
                else escapa(&POS);
            }
            if(SRC[POS] == '"') POS++;
        }
        LIT[LIT_N] = 0;
        T.k = TK_TEXTO;
        snprintf(T.s, sizeof T.s, "\"%.40s\"", (char*)LIT);
        return;
    }
    if(SRC[POS] == '\''){
        POS++;
        T.k = TK_NUM; T.longo = 0;
        T.i = (long)(char)escapa(&POS);
        if(SRC[POS] == '\'') POS++;
        snprintf(T.s, sizeof T.s, "%ld", T.i);
        return;
    }
    if(isalpha((unsigned char)SRC[POS]) || SRC[POS] == '_'){
        long i = 0;
        while(isalnum((unsigned char)SRC[POS]) || SRC[POS] == '_'){ if(i < 63) T.s[i++] = SRC[POS]; POS++; }
        T.s[i] = 0; T.k = TK_ID;
        return;
    }
    for(int i = 0; PUNS[i]; i++){
        long L = (long)strlen(PUNS[i]);
        if(!strncmp(SRC + POS, PUNS[i], (size_t)L)){
            strcpy(T.s, PUNS[i]); POS += L; T.k = TK_PUN; return;
        }
    }
    T.s[0] = SRC[POS++]; T.s[1] = 0; T.k = TK_PUN;
}

static int e_pun(const char *p){ return T.k == TK_PUN && !strcmp(T.s, p); }
static int e_id (const char *p){ return T.k == TK_ID  && !strcmp(T.s, p); }
static void come(const char *p){ if(!e_pun(p) && !e_id(p)) erro(p); avanca(); }
static int  aceita(const char *p){ if(e_pun(p) || e_id(p)){ avanca(); return 1; } return 0; }

static int tipo_de_nome(const char *s){
    if(!strcmp(s, "int"))    return TI32;
    if(!strcmp(s, "long"))   return TI64;
    if(!strcmp(s, "double") || !strcmp(s, "f64")) return TF64;
    if(!strcmp(s, "float"))  return TF64;
    if(!strcmp(s, "char"))     return TI8;
    if(!strcmp(s, "short"))    return TI32;
    if(!strcmp(s, "void"))     return TVOID;
    if(!strcmp(s, "unsigned"))  return TI32;
    if(!strcmp(s, "signed"))    return TI32;
    if(!strcmp(s, "va_list"))   return MKT(TI8, 1);   /* a fita é um endereço, e mais nada */
    /* os do sistema: um FILE é uma AGULHA no disco, e um tamanho é um número. Nenhum deles
     * traz estrutura nenhuma para dentro — é o slot que responde, como no canal do sql.c. */
    if(!strcmp(s, "FILE"))      return TI32;
    if(!strcmp(s, "size_t"))    return TI32;
    if(!strcmp(s, "time_t"))    return TI64;
    if(!strcmp(s, "uint64_t"))  return TI64;
    if(!strcmp(s, "int64_t"))   return TI64;
    if(!strcmp(s, "uint32_t"))  return TI32;
    if(!strcmp(s, "int32_t"))   return TI32;
    { int e = acha_estr(s); if(e >= 0) return MKT(EST0 + e, 0); }
    return -1;
}
static int le_tipo(void);

/* lê o que vem depois do tipo: ou um nome, ou `(*nome)(assinatura)`. Devolve o tipo final
 * e deixa o nome em `nome`. Estava escrito só para o nome simples, e por isso um parâmetro
 * que é uma função não passava do `*`. */
static int le_declarador(int base, char *nome){
    if(e_pun("(")){
        avanca();
        if(!e_pun("*")) erro("declarador estranho");
        avanca();
        /* SEM NOME é o declarador abstracto — `(int(*)(int,int))` — e é a forma que a
         * descida escreve para dizer «isto é um salto com esta assinatura». */
        if(T.k == TK_ID){ strcpy(nome, T.s); avanca(); }
        else nome[0] = 0;
        come(")");
        come("(");
        int par[8], np = 0;
        while(!e_pun(")") && T.k != TK_FIM){
            if(e_id("void") && !np){
                long g = POS; Tok gt = T; long gl = LINHA;
                avanca();
                if(e_pun(")")) break;
                POS = g; T = gt; LINHA = gl;
            }
            int pt = le_tipo();
            if(T.k == TK_ID) avanca();
            if(np < 8) par[np++] = pt;
            if(!aceita(",")) break;
        }
        come(")");
        int k = acha_fpt(base, np, par);
        if(k < 0) erro("assinaturas de função a mais");
        return MKT(FPT0 + k, 0);
    }
    if(T.k == TK_ID){ strcpy(nome, T.s); avanca(); }
    else nome[0] = 0;
    return base;
}

static int e_tipo(void){
    if(T.k != TK_ID) return 0;
    if(!strcmp(T.s, "struct") || !strcmp(T.s, "const") || !strcmp(T.s, "volatile")
       || !strcmp(T.s, "register") || !strcmp(T.s, "inline")
       || !strcmp(T.s, "static") || !strcmp(T.s, "extern")
       || !strcmp(T.s, "unsigned") || !strcmp(T.s, "signed")) return 1;
    return tipo_de_nome(T.s) >= 0;
}

/* um tipo escrito: a base, e quantas estrelas vêm a seguir */
static int le_tipo(void);

/* os campos de uma estrutura, lidos uma vez e usados nos dois sítios: a declaração com
 * nome e a ANÓNIMA no meio de uma declaração. Estava só num, e a anónima não subia. */
/* o tamanho de um vector aceita CONTA CONSTANTE — `N * 20 + 4` — com precedência: o
 * produto antes da soma, e parênteses. Números só: uma variável aqui seria pedir em
 * execução, e isso não existe. */
static long num_const(void);
static long num_const_prim(void){
    if(e_pun("(")){ avanca(); long v = num_const(); come(")"); return v; }
    int neg = 0;
    if(e_pun("-")){ neg = 1; avanca(); }
    if(T.k != TK_NUM) erro("o tamanho quer números");
    long v = T.i; avanca();
    return neg ? -v : v;
}
static long num_const_termo(void){
    long v = num_const_prim();
    for(;;){
        if(e_pun("*")){ avanca(); v = v * num_const_prim(); }
        else if(e_pun("/")){ avanca(); long d = num_const_prim(); v = d ? v / d : 0; }
        else break;
    }
    return v;
}
static long num_const(void){
    long v = num_const_termo();
    for(;;){
        if(e_pun("+")){ avanca(); v = v + num_const_termo(); }
        else if(e_pun("-")){ avanca(); v = v - num_const_termo(); }
        else break;
    }
    return v;
}

static int le_campos(const char *nome){
    if(NESTR >= 64) erro("estruturas a mais");
    Estrutura *E = &ESTR[NESTR];
    memset(E, 0, sizeof *E);
    come("{");
    while(!e_pun("}") && T.k != TK_FIM){
        int ct = le_tipo();
        for(;;){
            if(T.k != TK_ID) break;
            char cn[64]; strcpy(cn, T.s); avanca();
            int t = ct; long quantos = 0;
            if(e_pun("[")){
                avanca();
                if(!e_pun("]")) quantos = num_const();
                come("]");
                t = MKT(BASE(ct), PTR(ct) + 1);
            }
            long tam1 = tam_de(ct); if(tam1 < 1) tam1 = 1;
            long al = tam1 > 8 ? 8 : tam1;
            E->tam = (E->tam + al - 1) / al * al;
            strcpy(E->cnome[E->ncampos], cn);
            E->ctipo[E->ncampos] = t;
            E->cdesl[E->ncampos] = E->tam;
            E->cbytes[E->ncampos] = tam1 * (quantos > 0 ? quantos : 1);
            E->cvec[E->ncampos] = (quantos > 0);
            E->ncampos++;
            E->tam += tam1 * (quantos > 0 ? quantos : 1);
            if(!aceita(",")) break;
            while(e_pun("*")) avanca();
        }
        come(";");
    }
    come("}");
    E->tam = (E->tam + 7) / 8 * 8;
    snprintf(E->nome, sizeof E->nome, "%s", nome);
    NESTR++;
    return MKT(EST0 + NESTR - 1, 0);
}

static int le_tipo(void){
    int b = -1;
    /* AS PALAVRAS DO TIPO LÊEM-SE TODAS, e por qualquer ordem: `const unsigned char`,
     * `long int`, `unsigned long`. Eu lia UMA e parava — e `unsigned char *s` dava um
     * `int` seguido de um `char` que ninguém esperava. */
    int sem_sinal = 0, houve = 0;
    for(;;){
        if(e_id("const") || e_id("volatile") || e_id("register") || e_id("static")
           || e_id("inline") || e_id("extern")){ avanca(); continue; }
        if(e_id("unsigned")){ sem_sinal = 1; houve = 1; avanca(); continue; }
        if(e_id("signed")){ houve = 1; avanca(); continue; }
        if(e_id("long")){ b = TI64; houve = 1; avanca(); continue; }
        if(e_id("short")){ b = TI32; houve = 1; avanca(); continue; }
        if(e_id("int")){ if(b < 0) b = TI32; houve = 1; avanca(); continue; }
        if(e_id("char")){ b = sem_sinal ? TU8 : TI8; houve = 1; avanca(); continue; }
        if(e_id("double") || e_id("f64") || e_id("float")){ b = TF64; houve = 1; avanca(); continue; }
        if(e_id("void")){ b = TVOID; houve = 1; avanca(); continue; }
        break;
    }
    if(b == TI8 && sem_sinal) b = TU8;
    if(b < 0 && houve) b = TI32;
    if(b < 0){
        if(e_id("struct")){
            avanca();
            if(e_pun("{")){                    /* anónima: nasce aqui e não tem nome */
                static int anon = 0;
                char n[64]; snprintf(n, sizeof n, "@anon%d", anon++);
                b = le_campos(n);
            } else {
                int e = acha_estr(T.s);
                if(e < 0) erro("estrutura desconhecida");
                b = MKT(EST0 + e, 0);
                avanca();
            }
        } else { b = tipo_de_nome(T.s); avanca(); }
    }
    int p = 0;
    while(e_pun("*")){ p++; avanca(); }
    return MKT(b, p);
}

/* ────────────────────────────────────────────────────────────────── as tabelas */

/* MAX_FUN — quantas funções o módulo acomoda. Subiu de 256 para 512 quando o
 * tests/tex.c passou as 256 e deixou de traduzir: o limite é do TRADUTOR, não do
 * programa, e um #define que ninguém testa é documentação e não limite. O
 * tests/tex_wasm.js é quem o exercita, e a mensagem abaixo diz o número. */
#define MAX_FUN 512
#define MAX_LOC 512   /* o tex_core `compila` passa dos 256 locais do wasm */

/* ── O `...` É UMA FITA ────────────────────────────────────────────────────────────────
 * Os argumentos a mais não cabem na assinatura — mas cabem no disco. Quem chama escreve-os em
 * slots de oito bytes num pedaço do seu quadro e passa o ENDEREÇO; quem recebe lê slots. É o
 * MOVE outra vez, e por isso a assinatura no wasm tem sempre um parâmetro a mais, que é a
 * fita. Sem isto o `printf` obrigava a reescrever os 134 sítios que o chamam. */
typedef struct {
    char nome[64];
    int ret, npar, par[16];
    int loc[MAX_LOC], nloc;
    int variadica, idx_fita;
    int interna;          /* `static`: existe, corre, e NÃO SAI. É o que a palavra diz. */
} Fun;
#define FITA_SLOTS 16
#define FUNS       DISCO_FIXO(Fun, 41)

/* TODO local do wasm nasce por esta porta — e ela GUARDA o teto. O transbordo
 * silencioso escrevia por cima do `nloc` (o campo a seguir ao `loc[]`) e o módulo
 * saía inválido com `local.set` do índice errado, sem um erro que o dissesse. */
static void erro(const char *m);
static int loc_poe(int t){
    if(FUNS[FUN_ACT].nloc >= MAX_LOC) erro("locais do wasm a mais numa função");
    int i = FUNS[FUN_ACT].npar + FUNS[FUN_ACT].nloc;
    FUNS[FUN_ACT].loc[FUNS[FUN_ACT].nloc++] = t;
    return i;
}

typedef struct { char nome[64]; int tipo, idx; int quadro, vector, slot; long desloc;
                 long bytes; /* o tamanho DECLARADO: o sizeof de um vector é isto, não o ponteiro */ } Loc;

/* ── MOVE(endereço, sentido) ───────────────────────────────────────────────────────────
 *
 * «MOVE(slot, +1) do slot para o registo — o velho LOAD; MOVE(slot, −1) do registo para o
 * slot — o velho STORE.»  (tests/move.c, e §sec:estrela do corpo_analitico)
 *
 * Não são duas funções com nomes diferentes: é uma, e o sentido é o sinal. Escrevê-las
 * separadas era dar dois nomes ao mesmo — «e dois nomes para o mesmo não são um par, são
 * redundância» (catalogo, §a ISA tem UMA operação).
 *
 * E o endereço CALCULA-SE. No `move.c` o slot é parâmetro (`slot[s]`), e o `disco.h` diz o
 * padrão inteiro: «o ficheiro É o vector», e usa-se «com o mesmo `buf[i]` de sempre». O
 * `u16` imediato do broca-so está dito onde pertence — como consequência daquela ISA, que
 * molda aquele compilador. O que não há é RAM: guardar não dissipa, construir e destruir é
 * que dissipa. */

/* o disco: os slots com nome, e o endereço sai da ordem em que foram declarados */
typedef struct { char nome[64]; int tipo; long endereco; int vector; long bytes; } Slot;
#define SLOTS      DISCO_FIXO(Slot, 43)

/* O ZERO NÃO É SLOT DE NINGUÉM. Em C o `0` é o ponteiro nulo, e um objecto no endereço zero
 * é indistinguível de «nenhum» — `strstr` a achar no princípio devolvia o mesmo que a não
 * achar. Reservam-se os primeiros oito bytes, e o `if(p)` volta a querer dizer o que diz.
 * A descida sabe da reserva e desconta-a: é uma propriedade do tradutor, e simétrica. */
#define NULO 8

/* A IMAGEM DO DISCO: os bytes que já lá estão quando o módulo arranca.
 *
 * Um texto entre aspas não é código: é disco que nasce escrito. Vai para a fita como tudo o
 * resto, e o que o programa recebe é o ENDEREÇO — que é o que `char *` sempre foi.
 *
 * E emite-se UM SEGMENTO só, do primeiro byte escrito ao último. Não por economia: é para a
 * volta fechar. A descida devolve `char DISCO[k];` para a parte que nasce a zero e
 * `char LIT[n] = "..."` para a que nasce escrita, e ao subir outra vez dá exactamente este
 * segmento, no mesmo sítio e com os mesmos bytes. Vários segmentos esparsos não teriam uma
 * forma em C que os devolvesse. */
/* a imagem cobre o DISCO, e o disco é do tamanho que as declarações somam. Estava em 1 MB e
 * um programa com uma área de saída de 8 MB não cabia — o literal ia parar fora dela. */
#define IMG_MAX (1<<25)   /* 32 MB: o monte do tex.wasm empurrou os inicializados para cima */
#define IMG        DISCO_FIXO(unsigned char, 31)
/* IMG_INI e IMG_FIM vivem no Reg; nascem a −1, semeados em prende_tudo. */

static void escreve_imagem(long onde, const unsigned char *b, long n){
    if(onde + n > IMG_MAX){ fprintf(stderr, "traduz: imagem cheia\n"); exit(2); }
    memcpy(IMG + onde, b, (size_t)n);
    if(IMG_INI < 0 || onde < IMG_INI) IMG_INI = onde;
    if(onde + n > IMG_FIM) IMG_FIM = onde + n;
}

static int acha_slot(const char *n){
    for(int i = 0; i < NSLOT; i++) if(!strcmp(SLOTS[i].nome, n)) return i;
    return -1;
}

static long slot_do_texto(const unsigned char *b, long n){
    for(long i = 0; IMG_INI >= 0 && i + n < IMG_FIM; i++)
        if(!memcmp(IMG + i, b, (size_t)n + 1)) return i;
    long onde = DISCO;
    DISCO += n + 1;
    escreve_imagem(onde, b, n + 1);
    return onde;
}

/* ── AS TABELAS QUE NASCEM ESCRITAS ──────────────────────────────────────────────────
 *
 * `static const Sec SECS[] = {{"chapter",'C',1}, ...}` não é código: é DISCO com o valor já
 * lá. Escreve-se na imagem, elemento a elemento, e o programa recebe o endereço — como
 * qualquer outro slot. Um texto dentro dela vale o endereço dele, que é o que `char *` é.
 *
 * Só entram constantes: números, letras, textos e o menos unário. Uma conta aqui seria
 * código a correr antes de haver programa, e isso não existe. */
static long le_um_valor(int tipo, unsigned char *saco, long onde);

static long le_agregado(int elem, unsigned char *saco, long cap, long *quantos){
    come("{");
    long n = 0, tam = tam_de(elem);
    if(tam < 1) tam = 1;
    while(!e_pun("}") && T.k != TK_FIM){
        if((n + 1) * tam > cap) erro("tabela grande de mais para a imagem");
        le_um_valor(elem, saco, n * tam);
        n++;
        if(!aceita(",")) break;
    }
    come("}");
    *quantos = n;
    return n * tam;
}

static long le_um_valor(int tipo, unsigned char *saco, long onde){
    long t = tam_de(tipo);
    if(t < 1) t = 1;
    if(E_ESTR(tipo)){
        Estrutura *E = &ESTR[BASE(tipo) - EST0];
        come("{");
        for(int k = 0; k < E->ncampos && !e_pun("}") && T.k != TK_FIM; k++){
            le_um_valor(E->ctipo[k], saco, onde + E->cdesl[k]);
            if(!aceita(",")) break;
        }
        come("}");
        return t;
    }
    if(e_pun("{")){
        long q = 0;
        le_agregado(MKT(BASE(tipo), PTR(tipo) ? PTR(tipo) - 1 : 0), saco + onde, 1 << 16, &q);
        return t;
    }
    long v = 0; uint64_t d = 0; int real = 0, neg = 0;
    if(e_pun("-")){ neg = 1; avanca(); }
    if(T.k == TK_TEXTO){
        unsigned char b[8192]; long n = LIT_N;
        memcpy(b, LIT, (size_t)n + 1);
        avanca();
        v = slot_do_texto(b, n);
    } else if(T.k == TK_NUM){ v = neg ? -T.i : T.i; avanca(); }
    else if(T.k == TK_REAL){ d = neg ? f64_neg_bits(T.d) : T.d; real = 1; avanca(); }
    else if(T.k == TK_ID){ avanca(); }
    else erro("valor constante na tabela");

    if(!PTR(tipo) && BASE(tipo) == TF64){
        uint64_t x = real ? d : f64_bits_from_i(v);
        memcpy(saco + onde, &x, 8);
        return 8;
    }
    long x = real ? (long)d : v;
    for(long k = 0; k < t; k++) saco[onde + k] = (unsigned char)((x >> (8*k)) & 255);
    return t;
}
static void abre_slot(const char *nome, int tipo, long quantos){
    if(NSLOT >= 256){ fprintf(stderr, "traduz: slots a mais\n"); exit(2); }
    long t = tam_de(tipo); if(t < 1) t = 1;
    long al = t > 8 ? 8 : t;
    DISCO = (DISCO + al - 1) / al * al;
    strcpy(SLOTS[NSLOT].nome, nome);
    SLOTS[NSLOT].tipo = tipo;
    SLOTS[NSLOT].endereco = DISCO;
    SLOTS[NSLOT].vector = (quantos > 0);
    SLOTS[NSLOT].bytes = t * (quantos > 0 ? quantos : 1);
    NSLOT++;
    DISCO += t * (quantos > 0 ? quantos : 1);
}
#define LOCS       DISCO_FIXO(Loc, 42)

static int acha_fun(const char *n){
    for(int i = 0; i < NFUN; i++) if(!strcmp(FUNS[i].nome, n)) return i;
    return -1;
}
static int acha_loc(const char *n){
    for(int i = NLOC - 1; i >= 0; i--) if(!strcmp(LOCS[i].nome, n)) return i;
    return -1;
}

/* ────────────────────────────────────────────────────── o corpo que se emite, e a profundidade */

#define COD_B      DISCO_FIXO(unsigned char, 32)
#define PEDACO     DISCO_FIXO(unsigned char, 37)
#define TMP_MOVE   DISCO_FIXO(unsigned char, 38)
#define SACO       DISCO_FIXO(unsigned char, 39)
#define SAIDA_MAC  DISCO_FIXO(char, 40)
/* COD vive no Reg; o ponteiro e o tecto semeiam-se em prende_tudo. */

static void MOVE(int tipo, int sentido){
    unsigned op;
    int a = aritm(tipo);
    int oito = (!PTR(tipo) && (BASE(tipo) == TI8 || BASE(tipo) == TU8));
    int sem_sinal = (!PTR(tipo) && BASE(tipo) == TU8);
    if(sentido > 0) op = oito ? (sem_sinal ? 0x2D : 0x2C)
                              : a == TI32 ? 0x28 : a == TI64 ? 0x29 : 0x2B;
    else            op = oito ? 0x3A : a == TI32 ? 0x36 : a == TI64 ? 0x37 : 0x39;
    bput(&COD, op);
    bu(&COD, (unsigned long)(oito ? 0 : a == TI32 ? 2 : 3));   /* o alinhamento, natural */
    bu(&COD, 0);                                               /* e o desvio, que é zero */
}

/* ── O QUADRO: um vector local é disco, e o ponteiro dele é um slot ───────────────────
 *
 * Um `char buf[256]` dentro de uma função não cabe num local do wasm — os locais são valores,
 * e um vector é uma FITA. Então ele vai para o disco, como tudo, e o que a função faz é
 * baixar um ponteiro à entrada e repô-lo à saída:
 *
 *     à entrada   SP = SP − n          o quadro abre
 *     buf         é o endereço SP + k
 *     à saída     SP = SP + n          e fecha, exactamente onde abriu
 *
 * NÃO É ALOCAÇÃO: nada se pede a ninguém, e o que se baixa repõe-se. É a mesma involução de
 * sempre — abrir e fechar são a mesma operação com o sinal trocado —, e é por isso que duas
 * chamadas encaixadas da mesma função não se pisam.
 *
 * E o tamanho sabe-se ANTES de emitir, porque se lê o corpo primeiro: sem isso a constante do
 * prólogo mudava de comprimento ao ser corrigida, e corrigir um LEB128 pelo meio desloca tudo
 * o que vem depois. */
/* A fita do quadro: involução SP−n / SP+n (corpo_analitico §estrela) — NÃO é armazenamento.
 * A estrela não guarda nada; o que cabe aqui é a régua do frame aberto, e fecha onde
 * abriu. Se o SP esmaga literais, o defeito é cópia de vector no quadro (ex.: `Linha`
 * duplicada), não «faltam megabytes»: corrige-se in-place, sem alargar a fita. */
#define PILHA_BYTES 65536
/* SP_SLOT, QUADRO, QUADRO_USADO, TMP_RET, QUADRO_FITA, N_FITA/FITA_SITIO e M_FRASE vivem
 * no Reg. Uma fita POR SÍTIO: duas chamadas na mesma frase não podem partilhar a fita,
 * senão a segunda escreve por cima do que a primeira ainda não leu. */
/* Dentro de um braço de `?:`, `&&` ou `||` não pode nascer uma FRASE. O emissor sabe pô-la
 * lá; a descida é que não a sabe tirar — uma frase dentro de uma expressão não tem forma em
 * C, e ela sairia para fora do braço, a correr sempre. Então recusa-se, que é melhor do que
 * traduzir uma coisa por outra: escreve-se com um `if`, e é mais claro na mesma. */
typedef struct { int quebra, segue; } Laco;
#define LACOS      DISCO_FIXO(Laco, 55)

static void salto(int alvo){ bput(&COD, 0x0C); bu(&COD, (unsigned long)(PROF - 1 - alvo)); }
static void salto_se(int alvo){ bput(&COD, 0x0D); bu(&COD, (unsigned long)(PROF - 1 - alvo)); }

/* ────────────────────────────────────────────────────────────────── as expressões */

static int expr(void);
static int ternario(void);

/* promove o que está na pilha: se o topo é `de` e queremos `para`, emite a conversão */
static void promove_topo(int de, int para){
    unsigned op = conv_op(de, para);
    if(op) bput(&COD, op);
}
/* e o esquerdo, que já foi escrito: a conversão entra ANTES do direito */
static void promove_esq(long marca, int de, int para){
    unsigned op = conv_op(de, para);
    if(op){ unsigned char c = (unsigned char)op; bins(&COD, marca, &c, 1); }
}

static void carrega_local(int i){ bput(&COD, 0x20); bu(&COD, (unsigned long)LOCS[i].idx); }
static void endereco(long a){ bput(&COD, 0x41); bs(&COD, a); }

/* o que um nome local VALE: um vector de quadro vale o ENDEREÇO (SP + k), os outros valem o
 * local do wasm. Escrito uma vez — estava em dois sítios e o segundo não sabia do quadro:
 * emitia `local.get` de um índice que não existe, e o motor recusava o módulo inteiro. */
/* ESCREVER NUM NOME LOCAL. Um local do wasm grava-se com `local.set`; um local do QUADRO
 * grava-se com MOVE(−1), e para isso o endereço tem de ir à frente do valor. Estava só a
 * leitura a saber disto: `i++` num local com morada mexia no local do wasm que ninguém lê, e
 * o `i` nunca andava — o laço não acabava. Uma régua para os dois sentidos. */
static void carrega_nome_local(int i){
    if(LOCS[i].slot){                       /* um `static`: o nome vale o endereço do slot */
        endereco(SLOTS[LOCS[i].slot - 1].endereco);
        return;
    }
    if(LOCS[i].quadro){
        endereco(SLOTS[SP_SLOT].endereco);
        MOVE(TI32, +1);
        if(LOCS[i].desloc){ bput(&COD, 0x41); bs(&COD, LOCS[i].desloc); bput(&COD, 0x6A); }
        return;
    }
    carrega_local(i);
}
static void grava_local(int i){ bput(&COD, 0x21); bu(&COD, (unsigned long)LOCS[i].idx); }
static void grava_e_deixa(int i){ bput(&COD, 0x22); bu(&COD, (unsigned long)LOCS[i].idx); }

/* O PASSO DE UM: num número é um, NUM ENDEREÇO é um SLOT. `p++` sobre `int*` anda quatro
 * bytes, não um — e a régua é do corpo apontado, como em toda a aritmética de endereço.
 * E o tipo escolhe-se por `aritm`: um endereço é i32, e a comparação crua mandava-o para os
 * reais, que é onde ele nunca esteve. */
static void um(int t){
    int a = aritm(t);
    if(a == TI32){ bput(&COD, 0x41); bs(&COD, PTR(t) ? tam_de(MKT(BASE(t), PTR(t)-1)) : 1); }
    else if(a == TI64){ bput(&COD, 0x42); bs(&COD, 1); }
    else { uint64_t d = F64_1; bput(&COD, 0x44); bmany(&COD, &d, 8); }
}
static unsigned op_soma(int t){ int a = aritm(t); return a == TI32 ? 0x6A : a == TI64 ? 0x7C : 0xA0; }
static unsigned op_sub (int t){ int a = aritm(t); return a == TI32 ? 0x6B : a == TI64 ? 0x7D : 0xA1; }

/* a tabela dos binários: por tipo, e é a mesma operação com três assinaturas */
static unsigned bin_op(const char *o, int t){
    if(t == TF64){
        if(!strcmp(o,"+"))  return 0xA0;
        if(!strcmp(o,"-"))  return 0xA1;
        if(!strcmp(o,"*"))  return 0xA2;
        if(!strcmp(o,"/"))  return 0xA3;
        if(!strcmp(o,"==")) return 0x61;
        if(!strcmp(o,"!=")) return 0x62;
        if(!strcmp(o,"<"))  return 0x63;
        if(!strcmp(o,">"))  return 0x64;
        if(!strcmp(o,"<=")) return 0x65;
        if(!strcmp(o,">=")) return 0x66;
        return 0;
    }
    int L = (t == TI64);
    if(!strcmp(o,"+"))  return L?0x7C:0x6A;
    if(!strcmp(o,"-"))  return L?0x7D:0x6B;
    if(!strcmp(o,"*"))  return L?0x7E:0x6C;
    if(!strcmp(o,"/"))  return L?0x7F:0x6D;
    if(!strcmp(o,"%"))  return L?0x81:0x6F;
    if(!strcmp(o,"&"))  return L?0x83:0x71;
    if(!strcmp(o,"|"))  return L?0x84:0x72;
    if(!strcmp(o,"^"))  return L?0x85:0x73;
    if(!strcmp(o,"<<")) return L?0x86:0x74;
    if(!strcmp(o,">>")) return L?0x87:0x75;
    if(!strcmp(o,"==")) return L?0x51:0x46;
    if(!strcmp(o,"!=")) return L?0x52:0x47;
    if(!strcmp(o,"<"))  return L?0x53:0x48;
    if(!strcmp(o,">"))  return L?0x55:0x4A;
    if(!strcmp(o,"<=")) return L?0x57:0x4C;
    if(!strcmp(o,">=")) return L?0x59:0x4E;
    return 0;
}
static int e_comparacao(const char *o){
    return !strcmp(o,"==")||!strcmp(o,"!=")||!strcmp(o,"<")||!strcmp(o,">")
        || !strcmp(o,"<=")||!strcmp(o,">=");
}

/* `primaria` deixa o ENDEREÇO de tudo o que vive em memória, e diz que o deixou. Quem lê
 * carrega no fim; quem escreve grava no fim. Uma travessia só da expressão, e por isso o
 * `FITA[i].total` do lado esquerdo é exactamente o mesmo caminho do lado direito. */

static int unaria(void);
static void chamada_indirecta(int t);

static int primaria(void){
    PRIM_END = 0;
    if(e_id("sizeof")){
        /* sizeof é uma CONSTANTE DE TRADUÇÃO: `sizeof(tipo)` sai do tam_de; `sizeof expr`
         * tipa a expressão e DEITA FORA o código que ela emitiu — o C diz que o operando
         * não se avalia, e aqui isso é literal: o cursor do código volta atrás. */
        avanca();
        long tam = -1;
        if(e_pun("(")){
            long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
            avanca();
            if(e_tipo()){ int t = le_tipo(); come(")"); tam = tam_de(t); }
            else { POS = g_pos; LINHA = g_lin; T = g_tok; }
        }
        if(tam < 0 && T.k == TK_ID){
            /* `sizeof v` de um vector é o tamanho DECLARADO — não o ponteiro em que ele
             * decai — e `sizeof t->campo` sai do cbytes da estrutura. O lookahead decide;
             * o que não for nenhum dos dois cai no tipa-e-descarta, abaixo. */
            long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
            char nm[64]; strcpy(nm, T.s);
            avanca();
            int seta = e_pun("->"), ponto = e_pun(".");
            if(!seta && !ponto && !e_pun("[")){
                int i = acha_loc(nm);
                if(i >= 0) tam = (LOCS[i].vector && LOCS[i].bytes > 0)
                               ? LOCS[i].bytes : tam_de(LOCS[i].tipo);
                else {
                    int g = acha_slot(nm);
                    if(g >= 0) tam = (SLOTS[g].vector && SLOTS[g].bytes > 0)
                                   ? SLOTS[g].bytes : tam_de(SLOTS[g].tipo);
                }
            } else if(seta || ponto){
                avanca();
                if(T.k == TK_ID){
                    char cn[64]; strcpy(cn, T.s);
                    avanca();
                    if(!e_pun("->") && !e_pun(".") && !e_pun("[")){
                        int bt = -1;
                        int i = acha_loc(nm);
                        if(i >= 0) bt = LOCS[i].tipo;
                        else { int g = acha_slot(nm); if(g >= 0) bt = SLOTS[g].tipo; }
                        if(bt >= 0 && BASE(bt) >= EST0 && BASE(bt) < FPT0){
                            Estrutura *E = &ESTR[BASE(bt) - EST0];
                            for(int k = 0; k < E->ncampos; k++)
                                if(!strcmp(E->cnome[k], cn) && E->cbytes[k] > 0)
                                    tam = E->cbytes[k];
                        }
                    }
                }
            }
            if(tam < 0){ POS = g_pos; LINHA = g_lin; T = g_tok; }
        }
        if(tam < 0){
            long m = COD.n;
            int g_braco = EM_BRACO;                    /* o operando pode ter `&&`/`?:`, e
                                                        * como se DEITA FORA, o contador que
                                                        * eles subiram tem de voltar também */
            int t = unaria();
            COD.n = m;                                 /* o operando não se avalia */
            EM_BRACO = g_braco;
            tam = tam_de(t);
        }
        bput(&COD, 0x41); bs(&COD, tam);
        return TI32;
    }
    if(T.k == TK_NUM){
        long v = T.i; int L = T.longo; avanca();
        if(L || v > 2147483647L || v < -2147483648L){ bput(&COD, 0x42); bs(&COD, v); return TI64; }
        bput(&COD, 0x41); bs(&COD, v); return TI32;
    }
    if(T.k == TK_REAL){ uint64_t d = T.d; avanca(); bput(&COD, 0x44); bmany(&COD, &d, 8); return TF64; }
    if(T.k == TK_TEXTO){
        unsigned char b[8192]; long n = LIT_N;
        memcpy(b, LIT, (size_t)n + 1);
        avanca();
        /* o mesmo texto duas vezes é o mesmo slot: não são duas coisas, é uma escrita duas
         * vezes. E é determinístico, logo a volta continua a fechar. */
        long onde = -1;
        for(long i = 0; IMG_INI >= 0 && i + n < IMG_FIM; i++)
            if(!memcmp(IMG + i, b, (size_t)n + 1)){ onde = i; break; }
        if(onde < 0){
            onde = DISCO;
            DISCO += n + 1;
            escreve_imagem(onde, b, n + 1);
        }
        endereco(onde);
        return MKT(TI8, 1);
    }
    if(e_pun("(")){
        avanca();
        if(e_tipo()){                              /* o molde: (int)x, (char*)x */
            int alvo = le_tipo(); come(")");
            /* A MARCA APAGA-SE. Ela diz «o que está na pilha é uma MORADA», e quem a deixa
             * ligada é o que estava DENTRO — depois do molde já é valor. Ficando ligada, quem
             * chama lia outra vez: `(int)v` virava `*(int*)((int)v)`, e o número saía 0. */
            if(PTR(alvo)){                         /* mudar de régua não mexe no valor */
                int t; { extern int unaria_ext(void); t = unaria_ext(); }
                (void)t;
                PRIM_END = 0;
                return alvo;
            }
            /* a unária, para o molde morder só o que vem a seguir */
            long m = COD.n; (void)m;
            int t;
            { extern int unaria_ext(void); t = unaria_ext(); }
            /* `(void)x` AVALIA e DESCARTA — é o idioma de «li e não uso». O valor tem de sair
             * da pilha com um drop, senão fica pendente e a função void fecha com 1 a mais. */
            if(BASE(alvo) == TVOID && !PTR(alvo)){
                if(t != TVOID) bput(&COD, 0x1A);       /* drop */
                PRIM_END = 0;
                return TVOID;
            }
            promove_topo(t, alvo);
            PRIM_END = 0;
            return alvo;
        }
        int t = expr(); come(")");
        PRIM_END = 0;                              /* entre parênteses também é valor */
        return t;
    }
    if(T.k == TK_ID){
        char nome[64]; strcpy(nome, T.s); avanca();

        /* O DISCO CRESCE CONTADO, não reservado. `__disco_paginas()` diz quantas páginas o
         * disco tem AGORA; `__disco_cresce(n)` estende-o de `n` páginas e devolve a antiga
         * fronteira (ou −1). São `memory.size` e `memory.grow` — o mmap do disco.h em wasm:
         * o motor pagina o que se toca e larga o resto. NÃO é RAM a dissipar nem o infinito
         * reservado — é o disco a estender-se pelo que se ESCREVE. */
        if(!strcmp(nome, "__disco_paginas")){
            come("("); come(")");
            bput(&COD, 0x3F); bput(&COD, 0x00);            /* memory.size */
            return TI32;
        }
        if(!strcmp(nome, "__disco_cresce")){
            come("(");
            int t = expr(); promove_topo(t, TI32);
            come(")");
            bput(&COD, 0x40); bput(&COD, 0x00);            /* memory.grow */
            return TI32;
        }
        /* fopen miss → o host infla 1 slot do LS e faz poe. Import env.__fich_miss:
         * síncrono (o Map JS já tem os bytes); o DISCO só cresce com o que o fopen pede. */
        if(!strcmp(nome, "__fich_miss")){
            if(N_IMP < 1) erro("__fich_miss sem import no módulo");
            come("(");
            int t = expr(); promove_topo(t, TI32);
            come(")");
            bput(&COD, 0x10); bu(&COD, 0);                  /* call import 0 */
            return TI32;
        }

        /* `va_arg(ap, T)` lê a fita e anda um slot. O valor GUARDA-SE num local antes de o
         * ponteiro andar: deixá-lo na pilha punha o avanço entre o valor e quem o usa, e
         * não há C que diga isso — a mesma razão do valor de retorno com quadro. */
        if(!strcmp(nome, "va_arg")){
            if(EM_BRACO) erro("va_arg dentro de um braço de ?: — escreve-o com um `if`");
            come("(");
            if(T.k != TK_ID) erro("va_arg pede a fita");
            int ia = acha_loc(T.s);
            if(ia < 0) erro("va_arg pede uma fita local");
            avanca(); come(",");
            int tv = le_tipo(); come(")");
            int tmp = loc_poe(aritm(tv));
            long m0 = COD.n;
            carrega_local(ia); MOVE(tv, +1);
            bput(&COD, 0x21); bu(&COD, (unsigned long)tmp);
            carrega_local(ia);
            bput(&COD, 0x41); bs(&COD, 8);
            bput(&COD, 0x6A);
            grava_local(ia);
            /* e o bloco SOBE ao princípio da frase: ler e andar são FRASES, e deixá-las no
             * meio de um valor já empilhado é a coisa que não tem forma em C. Fica só o
             * `local.get` do valor, que é o que a expressão pede. */
            long tam = COD.n - m0;
            if(M_FRASE < m0){
                memcpy(PEDACO, COD.b + m0, (size_t)tam);
                memmove(COD.b + M_FRASE + tam, COD.b + M_FRASE, (size_t)(m0 - M_FRASE));
                memcpy(COD.b + M_FRASE, PEDACO, (size_t)tam);
            }
            bput(&COD, 0x20); bu(&COD, (unsigned long)tmp);
            return tv;
        }
        if(!strcmp(nome, "va_end")){
            come("("); while(!e_pun(")") && T.k != TK_FIM) avanca(); come(")");
            bput(&COD, 0x41); bs(&COD, 0);        /* nada a fazer, e diz-se com um zero */
            return TI32;
        }
        if(!strcmp(nome, "va_start")){
            come("(");
            if(T.k != TK_ID) erro("va_start pede a fita");
            int ia = acha_loc(T.s);
            if(ia < 0) erro("va_start pede uma fita local");
            avanca();
            while(!e_pun(")") && T.k != TK_FIM) avanca();
            come(")");
            if(!FUNS[FUN_ACT].variadica) erro("va_start numa função que não tem `...`");
            bput(&COD, 0x20); bu(&COD, (unsigned long)FUNS[FUN_ACT].idx_fita);
            grava_local(ia);
            bput(&COD, 0x41); bs(&COD, 0);
            return TI32;
        }

        if(e_pun("(")){                            /* a chamada */
            /* UM PARÂMETRO PONTEIRO-DE-FUNÇÃO chama-se como qualquer função: `poe(ctx, ...)`.
             * Mas `poe` é um LOCAL, não um nome global — e chamar por ele é o salto por tabela
             * que a ISA já faz com o pc: empurra-se o índice (o valor do local) e faz-se a
             * chamada indirecta, que o motor recusa se a assinatura não bater. «IRRADIA». */
            int il = acha_loc(nome);
            if(il >= 0 && E_FPT(LOCS[il].tipo)){
                int t = LOCS[il].tipo;
                carrega_nome_local(il);            /* o índice da função fica na pilha, no fim */
                chamada_indirecta(t);
                return FPT[BASE(t) - FPT0].ret;
            }
            /* UMA GLOBAL ponteiro-de-função (um SLOT) chama-se igual --- é o g_disco/g_carrega das
             * COSTURAS. Lê-se o índice do slot (endereço + MOVE +1) e faz-se a chamada indirecta,
             * como para o local. O host atribui a costura antes de o núcleo compor. */
            int gs = acha_slot(nome);
            if(gs >= 0 && E_FPT(SLOTS[gs].tipo)){
                int t = SLOTS[gs].tipo;
                endereco(SLOTS[gs].endereco);      /* o endereço do slot da costura */
                MOVE(t, +1);                       /* lê o índice da função (i32) */
                chamada_indirecta(t);
                return FPT[BASE(t) - FPT0].ret;
            }
            avanca();
            int f = acha_fun(nome);
            if(f < 0){ strcpy(T.s, nome); erro("função desconhecida"); }
            int fixos = FUNS[f].variadica ? FUNS[f].npar - 1 : FUNS[f].npar;
            int meu_sitio = FUNS[f].variadica ? FITA_SITIO++ : 0;
            long m_fixos = COD.n; (void)m_fixos;
            int n = 0, extra = 0;
            long m_fita = -1;
            while(!e_pun(")")){
                if(n < fixos){
                    int alvo_p = FUNS[f].par[n];
                    if(E_ESTR(alvo_p)){
                        /* a cópia vai para o quadro de quem chama, e o que se passa é a
                         * morada dela — oito bytes de cada vez, que é o slot do disco */
                        if(SP_SLOT < 0) erro("estrutura por valor numa função sem quadro");
                        long tam = tam_de(alvo_p), dest = QUADRO_FITA + (long)FITA_SITIO * FITA_SLOTS * 8;
                        int tmp = loc_poe(TI32);
                        int t = expr();
                        (void)t;
                        bput(&COD, 0x21); bu(&COD, (unsigned long)tmp);     /* a origem */
                        for(long o = 0; o < tam; o += 8){
                            endereco(SLOTS[SP_SLOT].endereco); MOVE(TI32, +1);
                            if(dest + o){ bput(&COD, 0x41); bs(&COD, dest + o); bput(&COD, 0x6A); }
                            bput(&COD, 0x20); bu(&COD, (unsigned long)tmp);
                            if(o){ bput(&COD, 0x41); bs(&COD, o); bput(&COD, 0x6A); }
                            MOVE(TI64, +1);
                            MOVE(TI64, -1);
                        }
                        endereco(SLOTS[SP_SLOT].endereco); MOVE(TI32, +1);
                        if(dest){ bput(&COD, 0x41); bs(&COD, dest); bput(&COD, 0x6A); }
                        FITA_SITIO++;
                        n++;
                        if(!aceita(",")) break;
                        continue;
                    }
                    int t = expr();
                    promove_topo(t, alvo_p);
                    n++;
                } else {
                    if(!FUNS[f].variadica){ strcpy(T.s, nome); erro("número de argumentos"); }
                    if(EM_BRACO) erro("chamada com `...` dentro de um braço de ?: — usa um `if`");
                    if(m_fita < 0) m_fita = COD.n;
                    if(extra >= FITA_SLOTS) erro("argumentos a mais na fita");
                    if(SP_SLOT < 0) erro("chamada com `...` numa função sem quadro");
                    endereco(SLOTS[SP_SLOT].endereco); MOVE(TI32, +1);
                    long desl = QUADRO_FITA + (long)meu_sitio * FITA_SLOTS * 8 + 8 * extra;
                    if(desl){ bput(&COD, 0x41); bs(&COD, desl); bput(&COD, 0x6A); }
                    int t = expr();
                    MOVE(t, -1);
                    extra++;
                }
                if(!aceita(",")) break;
            }
            come(")");
            if(n != fixos){ strcpy(T.s, nome); erro("número de argumentos"); }
            if(FUNS[f].variadica){
                if(m_fita < 0) m_fita = COD.n;
                /* A FITA SOBE AO PRINCÍPIO DA FRASE, e não só à frente dos argumentos
                 * fixos. A ordem de avaliação dos argumentos é indeterminada em C, e esta é a
                 * única que a volta sabe dizer: o que desce são FRASES antes da expressão, e
                 * ao subir têm de voltar a ficar exactamente aí. Com duas chamadas na mesma
                 * expressão, subir só até aos fixos punha as escritas da segunda antes da
                 * chamada da primeira — e o programa passava a fazer outra coisa. */
                long tam = COD.n - m_fita;
                long onde = M_FRASE < m_fita ? M_FRASE : m_fita;
                memcpy(TMP_MOVE, COD.b + m_fita, (size_t)tam);
                memmove(COD.b + onde + tam, COD.b + onde, (size_t)(m_fita - onde));
                memcpy(COD.b + onde, TMP_MOVE, (size_t)tam);
                /* e o endereço da fita é o último argumento */
                endereco(SLOTS[SP_SLOT].endereco); MOVE(TI32, +1);
                long d0 = QUADRO_FITA + (long)meu_sitio * FITA_SLOTS * 8;
                if(d0){ bput(&COD, 0x41); bs(&COD, d0); bput(&COD, 0x6A); }
            }
            bput(&COD, 0x10); bu(&COD, (unsigned long)WASM_FUN(f));
            PRIM_END = 0;                          /* o que uma função devolve é valor */
            return FUNS[f].ret;
        }
        {   /* o nome de uma função sem `(` é o ÍNDICE dela: é o que se passa a quem salta */
            int f2 = acha_fun(nome);
            if(f2 >= 0 && acha_loc(nome) < 0){
                int par[8], np = FUNS[f2].npar;
                for(int k = 0; k < np && k < 8; k++) par[k] = FUNS[f2].par[k];
                int k2 = acha_fpt(FUNS[f2].ret, np, par);
                if(k2 < 0) erro("assinaturas a mais");
                endereco(f2);
                return MKT(FPT0 + k2, 0);
            }
        }
        int i = acha_loc(nome);
        if(i < 0){
            int g = acha_slot(nome);
            if(g < 0){ strcpy(T.s, nome); erro("nome desconhecido"); }
            /* o nome de um slot É o endereço dele. Se for vector, fica o endereço (o vector
             * decai em ponteiro, como em C); se for escalar, MOVE(+1) lê-o. */
            endereco(SLOTS[g].endereco);
            int tg = SLOTS[g].tipo;
            if(SLOTS[g].vector) return MKT(BASE(tg), PTR(tg) + 1);   /* decai: é um valor */
            if(e_pun("++") || e_pun("--")){
                /* pós-incremento de um SLOT: o valor velho fica na pilha e o slot avança
                 * numa mini-frase emitida aqui — endereço, valor, ±1, e o MOVE(−1). Serve
                 * a frase `N++;` e o `X[N++]` por igual, porque é a mesma expressão. */
                if(aritm(tg) == TF64) erro("++ num double: escreve por extenso");
                int desce_um = e_pun("--");
                avanca();
                MOVE(tg, +1);                              /* o valor velho: o resultado  */
                endereco(SLOTS[g].endereco);               /* o endereço, para o store    */
                endereco(SLOTS[g].endereco); MOVE(tg, +1); /* o valor outra vez           */
                bput(&COD, aritm(tg) == TI64 ? 0x42 : 0x41); bs(&COD, 1);
                bput(&COD, desce_um ? op_sub(tg) : op_soma(tg));
                MOVE(tg, -1);                              /* e o slot avança             */
                return tg;
            }
            PRIM_END = 1;                                            /* o resto é endereço */
            return tg;
        }
        int t = LOCS[i].tipo;
        if(LOCS[i].slot){
            carrega_nome_local(i);
            if(!SLOTS[LOCS[i].slot - 1].vector) PRIM_END = 1;
            else return MKT(BASE(t), PTR(t) + 1);
            return t;
        }
        if(LOCS[i].quadro){
            if(!LOCS[i].vector && (e_pun("++") || e_pun("--")))
                erro("++ num local com morada, dentro de uma expressão: põe-no numa frase à parte");
            carrega_nome_local(i);
            if(!LOCS[i].vector) PRIM_END = 1;     /* escalar no quadro: é uma morada */
            else if(E_ESTR(t)) PRIM_END = 1;
            return t;
        }
        if(e_pun("++") || e_pun("--")){            /* o pós-fixo: o valor VELHO fica */
            int mais = e_pun("++"); avanca();
            carrega_local(i); carrega_local(i); um(t);
            bput(&COD, mais ? op_soma(t) : op_sub(t));
            grava_local(i);
            return t;
        }
        carrega_local(i);
        return t;
    }
    erro("expressão"); return TI32;
}

/* o índice não é uma operação nova: `a[i]` é o endereço somado e depois MOVE(+1) */
static int endereco_indexado(int t){
    if(!PTR(t)) erro("[ ] pede um endereço");
    int alvo = MKT(BASE(t), PTR(t) - 1);
    int ti = expr();
    promove_topo(ti, TI32);
    long passo = tam_de(alvo);
    if(passo != 1){ bput(&COD, 0x41); bs(&COD, passo); bput(&COD, 0x6C); }
    bput(&COD, 0x6A);
    come("]");
    return alvo;
}
/* O CAMPO É O ENDEREÇO SOMADO. `s.campo` e `p->campo` são a mesma conta: um deles já tem o
 * endereço na pilha (uma estrutura vale o endereço dela), o outro tem-no porque é ponteiro.
 * Deixa o ENDEREÇO do campo e devolve o tipo dele. */
static int campo_de(int t, int seta){
    int b = seta ? MKT(BASE(t), PTR(t) - 1) : t;
    if(!E_ESTR(b)) erro(seta ? "-> pede um ponteiro para estrutura" : ". pede uma estrutura");
    Estrutura *E = &ESTR[BASE(b) - EST0];
    if(T.k != TK_ID) erro("o campo tem nome");
    int k = -1;
    for(int i = 0; i < E->ncampos; i++) if(!strcmp(E->cnome[i], T.s)) k = i;
    if(k < 0) erro("essa estrutura não tem esse campo");
    avanca();
    if(E->cdesl[k]){ bput(&COD, 0x41); bs(&COD, E->cdesl[k]); bput(&COD, 0x6A); }
    /* um campo-vector DECAI: o tipo guardado JÁ vem com a estrela (o parse põe-na), e o
     * endereço que ficou na pilha É o ponteiro — quem chamou lê o CAMPO_VEC e não carrega */
    CAMPO_VEC = E->cvec[k];
    return E->ctipo[k];
}

/* a cadeia pós-fixa deixa um ENDEREÇO sempre que o que está em cima é memória; o valor só se
 * lê no fim, e por isso a mesma função serve para ler e para escrever. */
/* O SALTO POR TABELA. Um ponteiro de função é um ÍNDICE, e chamar por ele é dizer ao motor
 * qual a assinatura — ele recusa se não bater, o que é a régua a valer em execução. */
static void chamada_indirecta(int t){
    int k = BASE(t) - FPT0;
    /* O ÍNDICE ESTÁ NO TOPO — e o call_indirect quer-o DEPOIS dos argumentos. Em vez de
     * mover bytes já emitidos (frágil, e o `sizeof` de um array virou o de um ponteiro), faz-se
     * como o va_arg: guarda-se o índice num local, emitem-se os argumentos, e repõe-se o índice
     * no fim. Um local temporário — a mesma involução de sempre, sem tocar no código escrito. */
    int tmp = loc_poe(TI32);
    bput(&COD, 0x21); bu(&COD, (unsigned long)tmp);         /* local.set tmp: guarda o índice */
    come("(");
    int n = 0;
    while(!e_pun(")") && T.k != TK_FIM){
        int ta = expr();
        if(n < FPT[k].npar) promove_topo(ta, FPT[k].par[n]);
        n++;
        if(!aceita(",")) break;
    }
    come(")");
    bput(&COD, 0x20); bu(&COD, (unsigned long)tmp);         /* local.get tmp: o índice, no fim */
    bput(&COD, 0x11);
    /* tipos: funções, depois FPT, depois (se houver) o tipo do import — o FPT não anda */
    bu(&COD, (unsigned long)(NFUN + k));
    bu(&COD, 0);                               /* a tabela, que é uma só */
}

static int posfixa_end(int *tipo){
    int t = primaria();
    int endereco_na_pilha = PRIM_END || E_ESTR(t);
    for(;;){
        if(E_FPT(t) && e_pun("(")){
            int r = FPT[BASE(t) - FPT0].ret;
            chamada_indirecta(t);
            t = r;
            endereco_na_pilha = 0;
            continue;
        }
        if(e_pun("[")){
            avanca();
            if(endereco_na_pilha && !E_ESTR(t)){ MOVE(t, +1); endereco_na_pilha = 0; }
            if(E_ESTR(t)) erro("indexar uma estrutura: usa o campo");
            t = endereco_indexado(t);
            endereco_na_pilha = 1;
            if(E_ESTR(t)) continue;         /* vector de estruturas: fica o endereço */
            if(endereco_na_pilha && (e_pun(".") || e_pun("->") || e_pun("["))){ MOVE(t, +1); endereco_na_pilha = 0; }
            continue;
        }
        if(e_pun(".")){
            avanca();
            if(!endereco_na_pilha && !E_ESTR(t)) erro(". pede uma estrutura");
            t = campo_de(t, 0);
            endereco_na_pilha = 1;
            if(CAMPO_VEC){ CAMPO_VEC = 0; endereco_na_pilha = 0; }
            if(E_ESTR(t)) continue;
            if(endereco_na_pilha && (e_pun(".") || e_pun("->") || e_pun("["))){ MOVE(t, +1); endereco_na_pilha = 0; }
            continue;
        }
        if(e_pun("->")){
            avanca();
            if(endereco_na_pilha){ MOVE(t, +1); endereco_na_pilha = 0; }
            t = campo_de(t, 1);
            endereco_na_pilha = 1;
            if(CAMPO_VEC){ CAMPO_VEC = 0; endereco_na_pilha = 0; }
            if(E_ESTR(t)) continue;
            if(endereco_na_pilha && (e_pun(".") || e_pun("->") || e_pun("["))){ MOVE(t, +1); endereco_na_pilha = 0; }
            continue;
        }
        break;
    }
    *tipo = t;
    return endereco_na_pilha && !E_ESTR(t);
}

static int posfixa(void){
    int t;
    if(posfixa_end(&t)) MOVE(t, +1);
    return t;
}

static int unaria(void){
    if(e_pun("-")){
        /* `-1` NÃO é uma subtracção: é uma constante, e é isso que o C diz. Emitir `0 − 1`
         * dava o mesmo número e três bytes a mais — e a volta apanhou-o, porque `case -1:`
         * sobe como constante e descia como conta. */
        long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
        avanca();
        if(T.k == TK_NUM){
            long v = -T.i; int L = T.longo; avanca();
            if(L || v > 2147483647L || v < -2147483648L){ bput(&COD, 0x42); bs(&COD, v); return TI64; }
            bput(&COD, 0x41); bs(&COD, v); return TI32;
        }
        if(T.k == TK_REAL){
            uint64_t d = f64_neg_bits(T.d); avanca();
            bput(&COD, 0x44); bmany(&COD, &d, 8); return TF64;
        }
        POS = g_pos; LINHA = g_lin; T = g_tok;
        avanca(); long m = COD.n; int t = unaria();
        if(t == TF64) bput(&COD, 0x9A);            /* f64.neg */
        else {                                     /* 0 - x, e o zero entra ANTES */
            unsigned char z[2] = { (unsigned char)(t==TI64?0x42:0x41), 0x00 };
            bins(&COD, m, z, 2);
            bput(&COD, op_sub(t));
        }
        return t;
    }
    if(e_pun("+")){ avanca(); return unaria(); }
    if(e_pun("!")){
        avanca(); int t = unaria();
        if(t == TF64){ uint64_t z = F64_0; bput(&COD, 0x44); bmany(&COD, &z, 8); bput(&COD, 0x61); }
        else bput(&COD, t == TI64 ? 0x50 : 0x45);  /* eqz */
        return TI32;
    }
    if(e_pun("*")){                                 /* MOVE(+1): do slot para o registo */
        avanca(); int t = unaria();
        if(!PTR(t)) erro("* pede um endereço");
        int alvo = MKT(BASE(t), PTR(t) - 1);
        MOVE(alvo, +1);
        return alvo;
    }
    if(e_pun("&")){                                 /* o endereço, que é o próprio slot */
        avanca();
        {   /* & de um CAMINHO — `&t->b`, `&x.c`, `&v[i]`, `&((T*)f(...))[i]`: a posfixa já
             * deixa o ENDEREÇO na pilha; o & é só não o carregar. O lookahead separa-o do
             * & de nome simples; o que nem nome é, só pode ser caminho. */
            int caminho = (T.k != TK_ID);
            if(!caminho){
                long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
                avanca();
                caminho = e_pun("->") || e_pun(".") || e_pun("[");
                POS = g_pos; LINHA = g_lin; T = g_tok;
            }
            if(caminho){
                int t2;
                int fica = posfixa_end(&t2);
                if(!fica && !E_ESTR(t2)) erro("& de um valor");
                return MKT(BASE(t2), PTR(t2) + 1);
            }
        }
        int il = acha_loc(T.s);
        if(il >= 0){
            if(!LOCS[il].quadro) erro("& de um local que não foi visto como tal — corpo estranho");
            int tl = LOCS[il].tipo;
            avanca();
            carrega_nome_local(il);
            return LOCS[il].vector ? tl : MKT(BASE(tl), PTR(tl) + 1);
        }
        int g = acha_slot(T.s);
        if(g < 0) erro("& pede um slot ou um local com morada");
        avanca();
        int tg = SLOTS[g].tipo;
        endereco(SLOTS[g].endereco);
        return MKT(BASE(tg), PTR(tg) + 1);
    }
    if(e_pun("~")){
        avanca(); int t = unaria();
        if(t == TI64){ bput(&COD, 0x42); bs(&COD, -1); bput(&COD, 0x85); }
        else { bput(&COD, 0x41); bs(&COD, -1); bput(&COD, 0x73); }
        return t;
    }
    if(e_pun("++") || e_pun("--")){                /* o pré-fixo: o valor NOVO fica */
        int mais = e_pun("++"); avanca();
        if(T.k != TK_ID) erro("++ pede um nome");
        int i = acha_loc(T.s); if(i < 0) erro("nome desconhecido");
        if(LOCS[i].quadro) erro("++ num local com morada, dentro de uma expressão: frase à parte");
        int t = LOCS[i].tipo; avanca();
        carrega_local(i); um(t); bput(&COD, mais ? op_soma(t) : op_sub(t));
        grava_e_deixa(i);
        return t;
    }
    return posfixa();
}
int unaria_ext(void){ return unaria(); }

/* a subida por precedência: cada nível chama o de baixo, e o que sai já é a pilha */
static int nivel(int p);

static const char *NIVEIS[][5] = {
    { "|",  0, 0, 0, 0 },
    { "^",  0, 0, 0, 0 },
    { "&",  0, 0, 0, 0 },
    { "==", "!=", 0, 0, 0 },
    { "<",  ">", "<=", ">=", 0 },
    { "<<", ">>", 0, 0, 0 },
    { "+",  "-", 0, 0, 0 },
    { "*",  "/", "%", 0, 0 },
};
#define N_NIVEIS 8

static int casa(int p){
    if(T.k != TK_PUN) return 0;
    for(int i = 0; i < 5 && NIVEIS[p][i]; i++) if(!strcmp(T.s, NIVEIS[p][i])) return 1;
    return 0;
}

static int nivel(int p){
    if(p >= N_NIVEIS) return unaria();
    long m_esq = COD.n;
    int te = nivel(p + 1);
    while(casa(p)){
        char o[4]; strcpy(o, T.s); avanca();
        long m_dir = COD.n;
        int td = nivel(p + 1);
        /* o deslocamento não promove: `x << 3` é do tipo de x, e o 3 vai ao tipo dele */
        if(!strcmp(o, "<<") || !strcmp(o, ">>")){
            promove_topo(td, te);
            bput(&COD, bin_op(o, te));
            m_esq = m_esq;                          /* te fica */
            continue;
        }
        /* SOMAR A UM ENDEREÇO ANDA EM SLOTS. O `+1` de um `int*` são quatro bytes, e é o
         * tamanho do slot que diz quantos — a régua é do corpo apontado, não do endereço. */
        if((!strcmp(o, "+") || !strcmp(o, "-")) && PTR(te) && !PTR(td)){
            promove_topo(td, TI32);
            long passo = tam_de(MKT(BASE(te), PTR(te) - 1));
            if(passo != 1){ bput(&COD, 0x41); bs(&COD, passo); bput(&COD, 0x6C); }
            bput(&COD, bin_op(o, TI32));
            continue;
        }
        /* O MAIS LARGO MEDE-SE PELA LARGURA, não pelo número do tipo. O `char` tem código 4
         * e o `long` tem 1 — comparar os códigos punha `long + char` a dar `char`, e um
         * `r*10 + (c & 255)` truncava para 32 bits sem dizer nada. E o resultado normaliza-se:
         * em C a aritmética de `char` dá `int`, e é isso que evita a mesma pergunta a seguir. */
        int alvo = aritm(te) >= aritm(td) ? te : td;
        if(!PTR(alvo) && BASE(alvo) < EST0) alvo = aritm(alvo);
        if(te != alvo) promove_esq(m_dir, te, alvo);
        if(td != alvo) promove_topo(td, alvo);
        unsigned op = bin_op(o, alvo);
        if(!op) erro("operador não suportado neste tipo");
        bput(&COD, op);
        te = e_comparacao(o) ? TI32 : alvo;
    }
    return te;
}

/* o valor vira 0/1 em i32, que é o que o `if` do wasm quer na pilha */
static void normaliza_bool(int t){
    int a = aritm(t);
    if(a == TI64){ bput(&COD, 0x50); bput(&COD, 0x45); }
    else if(a == TF64){ uint64_t z = F64_0; bput(&COD, 0x44); bmany(&COD, &z, 8); bput(&COD, 0x62); }
}

/* o && e o || CURTOCIRCUITAM, e em wasm isso é um `if` com resultado — não um salto à mão */
static int logico_e(void){
    int t = nivel(0);
    while(e_pun("&&")){
        avanca();
        if(t == TI64) bput(&COD, 0x50), bput(&COD, 0x45);   /* i64 -> i32 booleano */
        else if(t == TF64){ uint64_t z=F64_0; bput(&COD,0x44); bmany(&COD,&z,8); bput(&COD,0x62); }
        bput(&COD, 0x04); bput(&COD, 0x7F);                  /* if (result i32) */
        PROF++;
        long g_frase = M_FRASE; M_FRASE = COD.n; EM_BRACO++;  /* o braço é o seu princípio */
        int td = nivel(0);
        if(td == TI64) bput(&COD, 0x50), bput(&COD, 0x45);
        else if(td == TF64){ uint64_t z=F64_0; bput(&COD,0x44); bmany(&COD,&z,8); bput(&COD,0x62); }
        else { bput(&COD, 0x45); bput(&COD, 0x45); }         /* normaliza para 0/1 */
        bput(&COD, 0x05);                                    /* else */
        bput(&COD, 0x41); bs(&COD, 0);
        bput(&COD, 0x0B); PROF--;                            /* end */
        EM_BRACO--; M_FRASE = g_frase;
        t = TI32;
    }
    return t;
}
static int logico_ou(void){
    int t = logico_e();
    while(e_pun("||")){
        avanca();
        if(t == TI64) bput(&COD, 0x50), bput(&COD, 0x45);
        else if(t == TF64){ uint64_t z=F64_0; bput(&COD,0x44); bmany(&COD,&z,8); bput(&COD,0x62); }
        bput(&COD, 0x04); bput(&COD, 0x7F); PROF++;
        bput(&COD, 0x41); bs(&COD, 1);
        bput(&COD, 0x05);
        long g_frase = M_FRASE; M_FRASE = COD.n; EM_BRACO++;
        int td = logico_e();
        if(td == TI64) bput(&COD, 0x50), bput(&COD, 0x45);
        else if(td == TF64){ uint64_t z=F64_0; bput(&COD,0x44); bmany(&COD,&z,8); bput(&COD,0x62); }
        else { bput(&COD, 0x45); bput(&COD, 0x45); }
        bput(&COD, 0x0B); PROF--;
        EM_BRACO--; M_FRASE = g_frase;
        t = TI32;
    }
    return t;
}

/* O TERNÁRIO É O MESMO `if` COM RESULTADO. Não há aqui construção nova: o `&&` já o usa, e a
 * única diferença é que o tipo do resultado não é forçosamente `i32` — vem dos dois braços, e o
 * que estiver abaixo sobe ao do outro. O tipo do bloco escreve-se PROVISÓRIO e corrige-se
 * depois, que é quando ele se sabe: um byte, no sítio onde ficou. */
static int ternario(void){
    int t = logico_ou();
    if(!e_pun("?")) return t;
    avanca();
    normaliza_bool(t);
    bput(&COD, 0x04);
    long m_tipo = COD.n; bput(&COD, 0x7F);
    PROF++;
    /* O LIMITE DE SUBIDA PÁRA AQUI. Uma frase que sobe (o `va_arg`, a fita dos `...`) não
     * pode atravessar a fronteira de um braço: sairia de dentro do bloco e o código ficava
     * malformado — o motor recusava o módulo com um alinhamento absurdo, que é o que se vê
     * quando o fluxo de bytes se desalinha. Cada braço é o seu próprio princípio. */
    long g_frase = M_FRASE;
    M_FRASE = COD.n; EM_BRACO++;
    int ta = expr();
    come(":");
    long m_senao = COD.n; bput(&COD, 0x05);
    M_FRASE = COD.n;
    int tb = ternario();
    EM_BRACO--; M_FRASE = g_frase;                    /* à DIREITA: a?b:c?d:e é a?b:(c?d:e) */
    int alvo = aritm(ta) >= aritm(tb) ? ta : tb;
    if(!PTR(alvo) && BASE(alvo) < EST0) alvo = aritm(alvo);
    if(aritm(ta) != aritm(alvo)){
        unsigned op = conv_op(ta, alvo);
        if(op){ unsigned char c = (unsigned char)op; bins(&COD, m_senao, &c, 1); }
    }
    if(aritm(tb) != aritm(alvo)) promove_topo(tb, alvo);
    COD.b[m_tipo] = (unsigned char)val_t(alvo);
    bput(&COD, 0x0B); PROF--;
    return alvo;
}

/* a atribuição é o único que associa à DIREITA, e deixa o valor: por isso `local.tee` */
static int expr(void){
    /* ESCREVER NUM SLOT é o mesmo MOVE com o sinal trocado. Tenta-se ler um destino — um
     * `*e` ou um `e[i]` — deixando o ENDEREÇO na pilha; se não vier um `=`, recua-se, e
     * recuar inclui o código já emitido: a posição do buffer volta atrás e nada fica. */
    /* e a frase pode começar em `(`: as tabelas em disco expandem para um cast —
     * `((Hif*)disco_buf(...))[i].campo = v` — e o recuo protege o que não for designador */
    if(e_pun("*") || e_pun("(") || (T.k == TK_ID && (acha_slot(T.s) >= 0 || acha_loc(T.s) >= 0))){
        long g_pos = POS, g_lin = LINHA, g_cod = COD.n; Tok g_tok = T;
        int g_braco = EM_BRACO, g_cvec = CAMPO_VEC;   /* o ensaio recua TAMBÉM as bandeiras */
        int alvo = -1;
        if(e_pun("*")){
            avanca();
            int t = unaria();
            if(PTR(t)) alvo = MKT(BASE(t), PTR(t) - 1);
        } else if(1){
            int t;
            if(posfixa_end(&t)) alvo = t;
        } else {
            char nome[64]; strcpy(nome, T.s); avanca();
            if(e_pun("[")){
                avanca();
                int i = acha_loc(nome);
                int t;
                if(i >= 0){ carrega_nome_local(i); t = LOCS[i].tipo; }
                else {
                    int g = acha_slot(nome);
                    endereco(SLOTS[g].endereco);
                    int tg = SLOTS[g].tipo;
                    t = SLOTS[g].vector ? MKT(BASE(tg), PTR(tg) + 1) : tg;
                }
                if(PTR(t)) alvo = endereco_indexado(t);
            } else if(e_pun(".") || e_pun("->")){
                /* um campo à esquerda do `=`: recua-se ao nome e usa-se o MESMO designador
                 * que serve para ler. Escrever e ler não são duas travessias da expressão. */
                POS = g_pos; LINHA = g_lin; T = g_tok; COD.n = g_cod;
                int t;
                if(posfixa_end(&t)) alvo = t;
            } else {
                /* e o SLOT ESCALAR à esquerda do `=`. Faltava, e o buraco era exactamente o
                 * mesmo de sempre: escrevi o caso quando só havia vectores. Um slot escalar é
                 * um slot — escreve-se com o mesmo MOVE(−1), e o endereço é o dele. */
                int g = acha_slot(nome);
                if(g >= 0 && !SLOTS[g].vector){
                    endereco(SLOTS[g].endereco);
                    alvo = SLOTS[g].tipo;
                }
            }
        }
        if(alvo >= 0 && e_pun("=")){
            avanca();
            int td = expr();
            promove_topo(td, alvo);
            MOVE(alvo, -1);
            return TVOID;                       /* o valor ficou no slot, não na pilha */
        }
        if(alvo >= 0){
            /* o op= num caminho qualquer — `p->y -= v`, `E[i].n += 1`. O endereço já está
             * na pilha (é o do store); o valor velho pede o endereço OUTRA VEZ, e ele sai
             * da MESMA travessia relida — um designador de op= não tem efeitos; quem os
             * tiver escreve a frase à parte, como os locais com morada. */
            /* e o `caminho++` — como FRASE, é o `+= 1` sem o 1 escrito e o valor deita-se fora
             * (o expr_efeito/instrucao dão drop no não-void); como VALOR (`x = s->cur++`,
             * `a[s->cur++]`, `f(s->cur++)`), o VELHO FICA. Antes devolvia TVOID e o velho sumia:
             * em contexto de valor o i32.add seguinte ficava com um lado só e o motor recusava o
             * módulo. `local.tee` guarda o velho ao passar, e repõe-se no fim como resultado. */
            if(e_pun("++") || e_pun("--")){
                if(aritm(alvo) == TF64) erro("++ num double: escreve por extenso");
                int desce1 = e_pun("--");
                long f_pos = POS, f_lin = LINHA; Tok f_tok = T;
                POS = g_pos; LINHA = g_lin; T = g_tok;
                { int t2; if(e_pun("*")){ avanca(); (void)unaria(); } else posfixa_end(&t2); }
                MOVE(alvo, +1);                                  /* o VELHO, no topo */
                int tmp = loc_poe((aritm(alvo) == TI64) ? TI64 : TI32);
                bput(&COD, 0x22); bu(&COD, (unsigned long)tmp);  /* local.tee: o velho fica E guarda-se */
                POS = f_pos; LINHA = f_lin; T = f_tok;
                avanca();
                bput(&COD, aritm(alvo) == TI64 ? 0x42 : 0x41); bs(&COD, 1);
                bput(&COD, desce1 ? op_sub(alvo) : op_soma(alvo));
                MOVE(alvo, -1);                                  /* store VELHO±1 */
                bput(&COD, 0x20); bu(&COD, (unsigned long)tmp);  /* local.get: o VELHO é o resultado */
                return alvo;
            }
            static const char *AT2[] = { "+=","-=","*=","/=","%=","&=","|=","^=", 0 };
            int q2 = -1;
            for(int i = 0; AT2[i]; i++) if(e_pun(AT2[i])){ q2 = i; break; }
            if(q2 >= 0){
                char o[3] = { AT2[q2][0], 0, 0 };
                long f_pos = POS, f_lin = LINHA; Tok f_tok = T;
                POS = g_pos; LINHA = g_lin; T = g_tok;
                { int t2; if(e_pun("*")){ avanca(); (void)unaria(); } else posfixa_end(&t2); }
                MOVE(alvo, +1);                        /* o valor velho, do 2.o endereço */
                POS = f_pos; LINHA = f_lin; T = f_tok;
                avanca();                              /* consome o op= */
                int td = expr();
                promove_topo(td, alvo);
                bput(&COD, bin_op(o, alvo));
                MOVE(alvo, -1);
                return TVOID;
            }
        }
        POS = g_pos; LINHA = g_lin; T = g_tok; COD.n = g_cod;
        EM_BRACO = g_braco; CAMPO_VEC = g_cvec;
    }
    if(T.k == TK_ID && acha_loc(T.s) >= 0){
        long guarda_pos = POS; long guarda_lin = LINHA; Tok guarda = T;
        char nome[64]; strcpy(nome, T.s);
        avanca();
        static const char *AT[] = { "=","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>=",0 };
        int qual = -1;
        for(int i = 0; AT[i]; i++) if(e_pun(AT[i])){ qual = i; break; }
        if(qual >= 0){
            char o[4] = { 0,0,0,0 };
            if(qual > 0){ size_t L = strlen(AT[qual]) - 1; memcpy(o, AT[qual], L); o[L] = 0; }
            avanca();
            int i = acha_loc(nome);
            int t = LOCS[i].tipo;
            /* UM `static` LOCAL É UM SLOT, não um local do wasm — o seu `idx` é −1, e um
             * `local.set −1` era um LEB gigante que o motor recusava. `tentado = 1` escreve-se
             * com MOVE(−1) ao slot, como qualquer disco: o endereço à frente do valor. */
            if(LOCS[i].slot && !SLOTS[LOCS[i].slot - 1].vector){
                long addr = SLOTS[LOCS[i].slot - 1].endereco;
                endereco(addr);
                if(qual > 0){ endereco(addr); MOVE(t, +1); }   /* o valor velho, para o op= */
                int td = expr();
                promove_topo(td, t);
                if(qual > 0) bput(&COD, bin_op(o, t));
                MOVE(t, -1);
                return TVOID;
            }
            if(LOCS[i].quadro && !LOCS[i].vector)
                erro("atribuição a um local com morada, dentro de uma expressão: frase à parte");
            if(qual > 0) carrega_local(i);
            long m = COD.n;
            int td = expr();
            if(qual == 0) promove_topo(td, t);
            else {
                if(!strcmp(o,"<<") || !strcmp(o,">>")) promove_topo(td, t);
                else promove_topo(td, t);
                (void)m;
                bput(&COD, bin_op(o, t));
            }
            grava_e_deixa(i);
            return t;
        }
        POS = guarda_pos; LINHA = guarda_lin; T = guarda;   /* não era atribuição: recua */
    }
    return ternario();
}

/* ────────────────────────────────────────────────────────────────── as instruções */

static void instrucao(void);

/* UMA EXPRESSAO QUE SO' VALE PELO EFEITO. Nao se pede valor, e por isso nao se emite um para
 * o deitar fora: o `x = e` grava (`local.set`) em vez de gravar-e-deixar seguido de `drop`, e
 * o `x++` nao carrega o velho que ninguem vai ler.
 *
 * E ESTA' NUM SO' SITIO. Estava na instrucao e nao no incremento do `for` — a mesma operacao
 * com duas reguas —, e a volta acusou-o: o corpo descia certo e os bytes nao fechavam. Uma
 * letra colada: o sitio esquecido foi o que se escreveu quando so' havia um. */
static void expr_efeito(const char *fim){
    if(T.k == TK_ID && acha_loc(T.s) >= 0){
        long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
        char nome[64]; strcpy(nome, T.s);
        avanca();
        int i = acha_loc(nome), t = LOCS[i].tipo;
        int no_quadro = (LOCS[i].quadro && !LOCS[i].vector)
                     || (LOCS[i].slot && !SLOTS[LOCS[i].slot - 1].vector);
        if(e_pun("++") || e_pun("--")){
            int mais = e_pun("++"); avanca();
            if(e_pun(fim)){
                if(no_quadro){
                    carrega_nome_local(i);                    /* o endereço, para gravar */
                    carrega_nome_local(i); MOVE(t, +1);       /* e o valor que lá está */
                    um(t); bput(&COD, mais ? op_soma(t) : op_sub(t));
                    MOVE(t, -1);
                } else {
                    carrega_local(i); um(t); bput(&COD, mais ? op_soma(t) : op_sub(t));
                    grava_local(i);
                }
                if(!strcmp(fim, ";")) avanca();
                return;
            }
        } else {
            static const char *AT[] = { "=","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>=",0 };
            int qual = -1;
            for(int k = 0; AT[k]; k++) if(e_pun(AT[k])){ qual = k; break; }
            if(qual >= 0){
                char o[4] = { 0,0,0,0 };
                if(qual > 0){ size_t L = strlen(AT[qual]) - 1; memcpy(o, AT[qual], L); o[L] = 0; }
                avanca();
                if(no_quadro){
                    carrega_nome_local(i);                    /* o endereço vai à frente */
                    if(qual > 0){ carrega_nome_local(i); MOVE(t, +1); }
                    int td = expr();
                    promove_topo(td, t);
                    if(qual > 0) bput(&COD, bin_op(o, t));
                    MOVE(t, -1);
                } else {
                    if(qual > 0) carrega_local(i);
                    int td = expr();
                    promove_topo(td, t);
                    if(qual > 0) bput(&COD, bin_op(o, t));
                    grava_local(i);
                }
                if(!strcmp(fim, ";")) come(";");
                return;
            }
        }
        POS = g_pos; LINHA = g_lin; T = g_tok;
    }
    { int t = expr(); if(t != TVOID) bput(&COD, 0x1A); if(!strcmp(fim, ";")) come(";"); }
}

/* Lê o corpo à frente só para somar os vectores locais, e devolve o lexer onde estava. É
 * preciso saber o tamanho ANTES de emitir o prólogo: a constante dele é um LEB128, e corrigir
 * um LEB128 pelo meio desloca tudo o que vem depois. */

/* ── QUEM TEM O ENDEREÇO TOMADO VIVE NO QUADRO ───────────────────────────────────────
 * Um local do wasm é um VALOR: não tem morada, e `&x` não tem o que devolver. Quem escreve
 * `sscanf(s, "%lf", &v)` está a dar uma morada — logo o `v` tem de ter uma, e a morada que
 * há neste sistema é a do disco. Lê-se o corpo à frente para saber quais são, antes de os
 * declarar: depois de declarado já é tarde. */
#define MAX_END 64
#define END_TOMADO DISCO_FIXO2(char, 64, 49)

static int tem_endereco_tomado(const char *n){
    for(int i = 0; i < N_END; i++) if(!strcmp(END_TOMADO[i], n)) return 1;
    return 0;
}

/* Lê o corpo à frente DUAS vezes: a primeira para saber quem tem morada (o `&x` pode vir
 * depois da declaração), a segunda para somar o que o quadro precisa. Contava só os vectores
 * — e um escalar com morada ficava de fora, a função nem abria quadro, e o ponteiro dele não
 * existia: o endereço saía do slot −1, que é lixo. */
static long mede_quadro(void){
    long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
    PRECISA_FITA = 0;
    N_END = 0;

    /* primeira volta: quem tem morada, e quem chama com `...` */
    int prof = 0;
    do {
        if(e_pun("{")) prof++;
        else if(e_pun("}")) prof--;
        else if(e_pun("&")){
            avanca();
            if(T.k == TK_ID && !tem_endereco_tomado(T.s) && N_END < MAX_END)
                strcpy(END_TOMADO[N_END++], T.s);
            continue;
        }
        else if(T.k == TK_ID && acha_fun(T.s) >= 0){
            int f = acha_fun(T.s);
            if(FUNS[f].variadica) PRECISA_FITA++;
            else for(int k = 0; k < FUNS[f].npar; k++)
                if(E_ESTR(FUNS[f].par[k])){ PRECISA_FITA++; break; }
        }
        avanca();
    } while(prof > 0 && T.k != TK_FIM);
    POS = g_pos; LINHA = g_lin; T = g_tok;

    /* segunda volta: o tamanho — os vectores, e os escalares que ganharam morada */
    long soma = 0;
    prof = 0;
    do {
        if(e_pun("{")) prof++;
        else if(e_pun("}")) prof--;
        else if(e_tipo() && prof > 0){
            /* `static` dentro da função é SLOT, não quadro (declara() já o sabe).
             * Contá-lo aqui fazia SP−n esmagar literais: `static DX[2048]` ≈ 50 KB. */
            if(e_id("static")){
                int p2 = 0;
                while(T.k != TK_FIM){
                    if(e_pun("(") || e_pun("[") || e_pun("{")) p2++;
                    else if(e_pun(")") || e_pun("]") || e_pun("}")){ if(p2 == 0) break; p2--; }
                    else if(p2 == 0 && e_pun(";")){ avanca(); break; }
                    avanca();
                }
                continue;
            }
            int base = le_tipo();
            for(;;){
                if(T.k != TK_ID) break;
                char nome[64]; strcpy(nome, T.s);
                avanca();
                long quantos = 0; int e_vector = 0;
                if(e_pun("[")){
                    e_vector = 1;
                    avanca();
                    if(!e_pun("]")) quantos = num_const();
                    if(e_pun("]")) avanca();
                }
                if(e_vector || tem_endereco_tomado(nome)){
                    long t = tam_de(base); if(t < 1) t = 1;
                    long al = t > 8 ? 8 : t;
                    soma = (soma + al - 1) / al * al;
                    soma += t * (quantos > 0 ? quantos : 1);
                }
                /* SALTAR O INICIALIZADOR: `long a = 0, c = 0;` tem `= 0` antes da vírgula, e
                 * sem o saltar o laço parava em `a` e não contava `c` — o quadro ficava 8 bytes
                 * curto e o último local escrevia fora (o `declara` conta os dois). */
                if(e_pun("=")){
                    avanca();
                    int prof2 = 0;
                    while(T.k != TK_FIM){
                        if(e_pun("(") || e_pun("[") || e_pun("{")) prof2++;
                        else if(e_pun(")") || e_pun("]") || e_pun("}")){ if(prof2 == 0) break; prof2--; }
                        else if(prof2 == 0 && (e_pun(",") || e_pun(";"))) break;
                        avanca();
                    }
                }
                if(!e_pun(",")) break;
                avanca();
                while(e_pun("*")) avanca();
            }
            continue;
        }
        avanca();
    } while(prof > 0 && T.k != TK_FIM);
    POS = g_pos; LINHA = g_lin; T = g_tok;
    return soma;
}

/* o slot do ponteiro do quadro, aberto quando a primeira função precisa dele */
static void garante_sp(void){
    if(SP_SLOT >= 0) return;
    abre_slot("__SP", TI32, 0);
    SP_SLOT = acha_slot("__SP");
}
/* abrir e fechar o quadro são A MESMA operação com o sinal trocado */
static void move_quadro(int sinal){
    endereco(SLOTS[SP_SLOT].endereco);
    endereco(SLOTS[SP_SLOT].endereco); MOVE(TI32, +1);
    bput(&COD, 0x41); bs(&COD, QUADRO);
    bput(&COD, sinal > 0 ? 0x6A : 0x6B);
    MOVE(TI32, -1);
}

static void declara(void){
    /* `static` DENTRO de uma função não é um local: é um SLOT. Ele sobrevive à chamada, e o
     * que sobrevive vive no disco. O nome leva o da função à frente porque duas funções podem
     * ter o mesmo, e são coisas diferentes. */
    int estatico = e_id("static");
    int base = le_tipo();
    if(estatico){
        for(;;){
            if(T.k != TK_ID) erro("nome na declaração");
            char nome[128];
            snprintf(nome, sizeof nome, "%s@%s", FUNS[FUN_ACT].nome, T.s);
            char curto[64]; snprintf(curto, sizeof curto, "%.63s", T.s);
            avanca();
            long quantos = 0;
            int era_array = 0;
            if(e_pun("[")){
                era_array = 1;                         /* `[]` faz vector; `= 0` sozinho NÃO */
                avanca();
                if(!e_pun("]")) quantos = num_const();
                come("]");
            }
            unsigned char saco[1 << 16];
            long bytes = 0, q = 0;
            int tem = 0;
            if(e_pun("=")){
                avanca();
                if(e_pun("{")){ bytes = le_agregado(base, SACO, (1<<18), &q); tem = 1; }
                else if(T.k == TK_TEXTO){
                    long n = LIT_N; memcpy(SACO, LIT, (size_t)n + 1);
                    bytes = n + 1; q = n + 1; avanca(); tem = 1;
                } else { bytes = le_um_valor(base, SACO, 0); q = 1; tem = 1; }
            }
            if(quantos <= 0) quantos = q > 0 ? q : 1;
            if(acha_slot(nome) < 0){
                /* vector SÓ se houve `[]` — um `static int t = 0` é ESCALAR, e marcá-lo
                 * vector fazia a leitura parar no endereço e a escrita virar `local.set −1`. */
                abre_slot(nome, base, era_array ? quantos : 0);
                if(tem) escreve_imagem(SLOTS[acha_slot(nome)].endereco, SACO, bytes);
            }
            /* e ganha um nome LOCAL que aponta para o slot, para o corpo o ver */
            if(NLOC < MAX_LOC){
                memset(&LOCS[NLOC], 0, sizeof LOCS[0]);
                strcpy(LOCS[NLOC].nome, curto);
                LOCS[NLOC].tipo = base;
                LOCS[NLOC].idx = -1;
                LOCS[NLOC].slot = acha_slot(nome) + 1;
                NLOC++;
            }
            if(aceita(",")) continue;
            come(";"); return;
        }
    }
    /* CADA DECLARADOR TEM AS SUAS ESTRELAS, e conta-as da base NUA. Em `Par *a, *s`, o `*` de
     * cada um é dele: le_tipo já comeu o do PRIMEIRO (a base vem com ele), mas o do segundo
     * conta-se a partir de `Par`, não de `Par*` — senão `*s` virava `Par**`. */
    int base_nua = MKT(BASE(base), 0);
    int primeiro = 1;
    for(;;){
        int bt = primeiro ? base : base_nua;
        primeiro = 0;
        while(e_pun("*")){ bt = MKT(BASE(bt), PTR(bt) + 1); avanca(); }
        if(T.k != TK_ID) erro("nome na declaração");
        if(NLOC >= MAX_LOC) erro("locais a mais");
        memset(&LOCS[NLOC], 0, sizeof LOCS[0]);
        strcpy(LOCS[NLOC].nome, T.s);
        LOCS[NLOC].tipo = bt;
        LOCS[NLOC].quadro = 0; LOCS[NLOC].desloc = 0;
        LOCS[NLOC].idx  = loc_poe(bt);
        int i = NLOC++;
        LOCS[i].quadro = 0; LOCS[i].desloc = 0; LOCS[i].vector = 0;
        int no_quadro = tem_endereco_tomado(LOCS[i].nome);
        avanca();
        if(no_quadro && !e_pun("[")){
            long t1 = tam_de(bt); if(t1 < 1) t1 = 1;
            long al = t1 > 8 ? 8 : t1;
            QUADRO_USADO = (QUADRO_USADO + al - 1) / al * al;
            LOCS[i].quadro = 1; LOCS[i].vector = 0;
            LOCS[i].desloc = QUADRO_USADO;
            QUADRO_USADO += t1;
            FUNS[FUN_ACT].nloc--;
            if(aceita("=")){
                /* o valor inicial escreve-se no slot, que é onde ele agora mora */
                carrega_nome_local(i);
                int t = expr();
                promove_topo(t, bt);
                MOVE(bt, -1);
            }
            if(aceita(",")) continue;
            come(";"); break;
        }
        if(e_pun("[")){
            avanca();
            long quantos = 0;
            if(!e_pun("]")) quantos = num_const();
            come("]");
            long t = tam_de(bt); if(t < 1) t = 1;
            long al = t > 8 ? 8 : t;
            QUADRO_USADO = (QUADRO_USADO + al - 1) / al * al;
            LOCS[i].quadro = 1; LOCS[i].vector = 1;
            LOCS[i].desloc = QUADRO_USADO;
            LOCS[i].tipo = MKT(BASE(bt), PTR(bt) + 1);
            LOCS[i].bytes = t * (quantos > 0 ? quantos : 1);
            QUADRO_USADO += t * (quantos > 0 ? quantos : 1);
            FUNS[FUN_ACT].nloc--;                  /* não é um local do wasm: é disco */
            NLOC--; NLOC++;
            if(aceita(",")) continue;
            come(";"); break;
        }
        if(aceita("=")){
            int t = expr();
            promove_topo(t, bt);
            grava_local(i);
        }
        if(aceita(",")) continue;
        come(";"); break;
    }
}

static void bloco(void){
    int marca = NLOC;                                /* o escopo fecha onde abriu */
    come("{");
    while(!e_pun("}") && T.k != TK_FIM) instrucao();
    come("}");
    NLOC = marca;
}

static void condicao(void){                          /* deixa um i32 booleano na pilha */
    M_FRASE = COD.n;                                 /* a condição é uma frase, e é aqui */
    int t = expr();
    if(t == TI64) bput(&COD, 0x50), bput(&COD, 0x45);
    else if(t == TF64){ uint64_t z = F64_0; bput(&COD, 0x44); bmany(&COD, &z, 8); bput(&COD, 0x62); }
}

static void instrucao(void){
    M_FRASE = COD.n;                       /* daqui para a frente é uma frase nova */
    EM_BRACO = 0;                          /* uma frase NUNCA está dentro de um braço de ?:;
                                            * um ensaio de ternário que recuou podia deixar
                                            * o contador positivo, e a frase seguinte — um
                                            * snprintf variádico — herdava a proibição alheia */
    if(e_pun("{")){ bloco(); return; }
    if(e_pun(";")){ avanca(); return; }
    if(e_tipo()){ declara(); return; }

    if(e_id("if")){
        avanca(); come("("); condicao(); come(")");
        bput(&COD, 0x04); bput(&COD, 0x40); PROF++;
        instrucao();
        if(e_id("else")){ avanca(); bput(&COD, 0x05); instrucao(); }
        bput(&COD, 0x0B); PROF--;
        return;
    }
    if(e_id("while")){
        avanca();
        int d_quebra = PROF; bput(&COD, 0x02); bput(&COD, 0x40); PROF++;
        int d_segue  = PROF; bput(&COD, 0x03); bput(&COD, 0x40); PROF++;
        come("("); condicao(); come(")");
        bput(&COD, 0x45);                            /* i32.eqz: sai quando é falso */
        salto_se(d_quebra);
        LACOS[NLACO].quebra = d_quebra; LACOS[NLACO].segue = d_segue; NLACO++;
        instrucao();
        NLACO--;
        salto(d_segue);
        bput(&COD, 0x0B); PROF--;
        bput(&COD, 0x0B); PROF--;
        return;
    }
    if(e_id("do")){
        avanca();
        int d_quebra = PROF; bput(&COD, 0x02); bput(&COD, 0x40); PROF++;
        int d_segue  = PROF; bput(&COD, 0x03); bput(&COD, 0x40); PROF++;
        LACOS[NLACO].quebra = d_quebra; LACOS[NLACO].segue = d_segue; NLACO++;
        instrucao();
        NLACO--;
        come("while"); come("("); condicao(); come(")"); come(";");
        salto_se(d_segue);                           /* volta enquanto for verdade */
        bput(&COD, 0x0B); PROF--;
        bput(&COD, 0x0B); PROF--;
        return;
    }
    if(e_id("for")){
        avanca(); come("(");
        int marca = NLOC;
        if(!e_pun(";")){
            if(e_tipo()) declara();
            else expr_efeito(";");
        } else avanca();

        int d_quebra = PROF; bput(&COD, 0x02); bput(&COD, 0x40); PROF++;
        int d_segue  = PROF; bput(&COD, 0x03); bput(&COD, 0x40); PROF++;
        if(!e_pun(";")){ condicao(); bput(&COD, 0x45); salto_se(d_quebra); }
        come(";");
        /* o incremento tem de correr no `continue` também — por isso o corpo leva um bloco
         * só dele: saltar para o fim DELE cai no incremento, não fora do laço */
        long m_inc = COD.n;                          /* o incremento emite-se aqui e MUDA-SE */
        int d_corpo = PROF; bput(&COD, 0x02); bput(&COD, 0x40); PROF++;
        LACOS[NLACO].quebra = d_quebra; LACOS[NLACO].segue = d_corpo; NLACO++;
        /* guarda o texto do incremento, para o emitir DEPOIS do corpo */
        long inc_pos = POS, inc_lin = LINHA; Tok inc_tok = T;
        int paren = 0;
        while(!(paren == 0 && e_pun(")")) && T.k != TK_FIM){
            if(e_pun("(")) paren++;
            if(e_pun(")")) paren--;
            avanca();
        }
        come(")");
        instrucao();                                  /* o corpo */
        NLACO--;
        bput(&COD, 0x0B); PROF--;                     /* fecha o bloco do corpo */
        (void)m_inc; (void)d_corpo;
        {                                             /* agora sim, o incremento */
            long dep_pos = POS, dep_lin = LINHA; Tok dep_tok = T;
            POS = inc_pos; LINHA = inc_lin; T = inc_tok;
            if(!e_pun(")")) expr_efeito(")");
            POS = dep_pos; LINHA = dep_lin; T = dep_tok;
        }
        salto(d_segue);
        bput(&COD, 0x0B); PROF--;
        bput(&COD, 0x0B); PROF--;
        NLOC = marca;
        return;
    }
    /* O SWITCH É UMA CADEIA DE `if/else`, e não uma construção nova. O wasm tem `br_table`,
     * mas escolhê-lo custava a volta: a cadeia desce como cadeia e torna a subir nos mesmos
     * bytes, e o `br_table` não tem forma em C que o devolva.
     *
     * O valor mede-se UMA vez e fica num local — `switch(f(x))` não pode chamar `f` por cada
     * caso. E a queda de um caso para o seguinte NÃO sobe: aqui recusa-se, em vez de traduzir
     * uma coisa por outra. Os quatro do `tex.c` acabam todos em `return`. */
    if(e_id("switch")){
        avanca(); come("(");
        int t = expr(); come(")");
        int idx = loc_poe(aritm(t));
        bput(&COD, 0x21); bu(&COD, (unsigned long)idx);
        come("{");
        int abertos = 0;
        while(!e_pun("}") && T.k != TK_FIM){
            if(e_id("case")){
                avanca();
                int neg = 0;
                if(e_pun("-")){ neg = 1; avanca(); }
                if(T.k != TK_NUM) erro("o caso quer um número — um nome só depois do #define");
                long v = neg ? -T.i : T.i;
                avanca(); come(":");
                bput(&COD, 0x20); bu(&COD, (unsigned long)idx);
                if(aritm(t) == TI64){ bput(&COD, 0x42); bs(&COD, v); bput(&COD, 0x51); }
                else { bput(&COD, 0x41); bs(&COD, v); bput(&COD, 0x46); }
                bput(&COD, 0x04); bput(&COD, 0x40); PROF++; abertos++;
                while(!e_id("case") && !e_id("default") && !e_pun("}") && T.k != TK_FIM){
                    if(e_id("break")){
                        long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
                        avanca(); come(";");
                        if(!e_id("case") && !e_id("default") && !e_pun("}"))
                            { POS = g_pos; LINHA = g_lin; T = g_tok;
                              erro("break a meio de um caso: ainda não sobe"); }
                        break;
                    }
                    instrucao();
                }
                bput(&COD, 0x05);                    /* o resto vai para o senão */
                continue;
            }
            if(e_id("default")){
                avanca(); come(":");
                while(!e_pun("}") && T.k != TK_FIM){
                    if(e_id("break")){ avanca(); come(";"); break; }
                    instrucao();
                }
                continue;
            }
            erro("dentro de um switch só há `case` e `default`");
        }
        come("}");
        while(abertos-- > 0){ bput(&COD, 0x0B); PROF--; }
        return;
    }

    if(e_id("break")){
        avanca(); come(";");
        if(!NLACO) erro("break fora de laço");
        salto(LACOS[NLACO-1].quebra);
        return;
    }
    if(e_id("continue")){
        avanca(); come(";");
        if(!NLACO) erro("continue fora de laço");
        salto(LACOS[NLACO-1].segue);
        return;
    }
    if(e_id("return")){
        avanca();
        if(!e_pun(";")){
            int t = expr();
            promove_topo(t, FUNS[FUN_ACT].ret);
        }
        come(";");
        /* O VALOR GUARDA-SE ANTES DE O QUADRO FECHAR. Não é uma finura: `return c[i]` lê o
         * quadro, e repor o ponteiro primeiro fá-lo ler acima dele — o endereço é SP+k, e
         * mexer no SP muda o endereço. Deixá-lo na pilha do wasm também não serve, porque
         * então a reposição fica ENTRE o valor e a saída, e não há C que diga isso. Guarda-se
         * num local, fecha-se o quadro, e sai-se com ele: correcto, e dizível nos dois
         * sentidos. Quem apanhou isto foi a volta — o C que voltou lia depois de fechar. */
        if(QUADRO > 0){
            if(TMP_RET >= 0){ bput(&COD, 0x21); bu(&COD, (unsigned long)TMP_RET); }
            move_quadro(+1);
            if(TMP_RET >= 0){ bput(&COD, 0x20); bu(&COD, (unsigned long)TMP_RET); }
        }
        bput(&COD, 0x0F);                             /* return */
        return;
    }
    expr_efeito(";");
}

/* ──────────────────────────────────────────────── a primeira volta: as assinaturas */

/* as funções têm de se conhecer todas antes de qualquer corpo — senão chamar para a frente
 * era impossível, e a recursão mútua também. Por isso passa-se DUAS vezes pelo texto: a
 * primeira só lê as assinaturas, a segunda emite. */
static void colhe_assinaturas(void){
    POS = 0; LINHA = 1; avanca();
    int prof = 0;
    while(T.k != TK_FIM){
        if(e_pun("{")){ prof++; avanca(); continue; }
        if(e_pun("}")){ prof--; avanca(); continue; }
        /* `typedef struct { ... } Nome;` e `struct Nome { ... };` — as duas formas, e o que
         * ambas fazem é o mesmo: dar nomes a deslocamentos dentro de um pedaço de disco. */
        if(prof == 0 && (e_id("typedef") || e_id("struct"))){
            long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
            int tipedef = e_id("typedef");
            if(tipedef) avanca();
            if(!e_id("struct")){ POS = g_pos; LINHA = g_lin; T = g_tok; avanca(); continue; }
            avanca();
            char marca[64] = "";
            if(T.k == TK_ID && !e_pun("{")){ strcpy(marca, T.s); avanca(); }
            if(!e_pun("{")){ POS = g_pos; LINHA = g_lin; T = g_tok; avanca(); continue; }
            avanca();
            if(NESTR >= 64) erro("estruturas a mais");
            Estrutura *E = &ESTR[NESTR];
            memset(E, 0, sizeof *E);
            E->tam = 0;
            while(!e_pun("}") && T.k != TK_FIM){
                int ct = le_tipo();
                for(;;){
                    if(T.k != TK_ID) break;
                    char cn[64]; strcpy(cn, T.s); avanca();
                    int t = ct; long quantos = 0;
                    if(e_pun("[")){
                        avanca();
                        if(!e_pun("]")) quantos = num_const();
                        come("]");
                        t = MKT(BASE(ct), PTR(ct) + 1);      /* um campo vector é o endereço */
                    }
                    long tam1 = tam_de(quantos > 0 ? ct : ct);
                    if(tam1 < 1) tam1 = 1;
                    long al = tam1 > 8 ? 8 : tam1;
                    E->tam = (E->tam + al - 1) / al * al;
                    strcpy(E->cnome[E->ncampos], cn);
                    E->ctipo[E->ncampos] = t;
                    E->cdesl[E->ncampos] = E->tam;
                    E->cbytes[E->ncampos] = tam1 * (quantos > 0 ? quantos : 1);
                    E->cvec[E->ncampos] = (quantos > 0);
                    E->ncampos++;
                    E->tam += tam1 * (quantos > 0 ? quantos : 1);
                    if(!aceita(",")) break;
                    while(e_pun("*")) avanca();
                }
                come(";");
            }
            come("}");
            E->tam = (E->tam + 7) / 8 * 8;                  /* o todo alinha a oito */
            if(tipedef && T.k == TK_ID){ strcpy(E->nome, T.s); avanca(); }
            else if(marca[0]) strcpy(E->nome, marca);
            else erro("a estrutura precisa de um nome");
            NESTR++;
            while(!e_pun(";") && T.k != TK_FIM) avanca();
            avanca();
            continue;
        }
        if(prof == 0 && (e_tipo() || e_id("static"))){
            int interna = 0;
            if(e_id("static")){ interna = 1; avanca(); }
            if(!e_tipo()){ avanca(); continue; }
            int ret = le_tipo();
            if(e_pun("(")){
                /* `ret (*nome)(assinatura);` --- um ponteiro-de-função GLOBAL: as COSTURAS
                 * (g_disco, g_carrega). O le_declarador parseia o `(*nome)(sig)` e devolve o tipo
                 * FPT + o nome; regista-se como SLOT, para o corpo o chamar por indirecção. O host
                 * escreve o índice da função nesse slot antes de o núcleo compor. */
                char fnome[64];
                int ft = le_declarador(ret, fnome);
                if(E_FPT(ft) && fnome[0] && acha_slot(fnome) < 0) abre_slot(fnome, ft, 0);
                if(e_pun("=")){ avanca(); while(!e_pun(";") && T.k != TK_FIM) avanca(); }
                aceita(";");
                continue;
            }
            if(T.k != TK_ID){ continue; }
            char nome[64]; strcpy(nome, T.s); avanca();
            if(!e_pun("(")){
                /* não é função: é um SLOT do disco, e o endereço sai da ordem em que se
                 * declarou. Nada é pedido em tempo de execução — o disco tem o tamanho que
                 * as declarações somam. E a porta segue o `static` aqui como nas funções:
                 * só um global PÚBLICO põe o disco na exportação — «apenas MOVE não tem
                 * opcode inventado DISCO: o MOVE corre aonde senão no disco?» */
                if(!interna) DISCO_PUBLICO = 1;
                for(;;){
                    long quantos = 0, aberto = 0;
                    if(e_pun("[")){
                        avanca();
                        if(!e_pun("]")) quantos = num_const();
                        come("]");
                    }
                    int teve_igual = 0;
                    if(e_pun("=")){ avanca(); teve_igual = 1; }
                    if(teve_igual && e_pun("{")){
                        long q = 0;
                        long bytes = le_agregado(ret, SACO, (1<<18), &q);
                        if(quantos <= 0) quantos = q > 0 ? q : 1;
                        if(acha_slot(nome) < 0){
                            abre_slot(nome, ret, quantos);
                            escreve_imagem(SLOTS[acha_slot(nome)].endereco, SACO, bytes);
                        }
                        aberto = 1;
                        if(!aceita(",")){ come(";"); break; }
                        while(e_pun("*")) avanca();
                        if(T.k != TK_ID) break;
                        strcpy(nome, T.s); avanca();
                        continue;
                    }
                    if(teve_igual && T.k != TK_TEXTO){
                        /* o escalar com valor: o slot nasce ESCRITO na imagem — o mesmo
                         * caminho do agregado, para um só. `static long C = 0, D = 1;` */
                        int neg = 0;
                        if(e_pun("-")){ neg = 1; avanca(); }
                        unsigned char b8[8];
                        long nb = tam_de(ret); if(nb < 1 || nb > 8) nb = 8;
                        if(T.k == TK_NUM){ long v = neg ? -T.i : T.i; avanca(); memcpy(b8, &v, 8); }
                        else if(T.k == TK_REAL){ uint64_t d = neg ? f64_neg_bits(T.d) : T.d; avanca(); memcpy(b8, &d, 8); }
                        else erro("inicializador global que não sobe");
                        if(acha_slot(nome) < 0){
                            abre_slot(nome, ret, quantos);
                            escreve_imagem(SLOTS[acha_slot(nome)].endereco, b8, nb);
                        }
                        aberto = 1;
                    }
                    if(T.k == TK_TEXTO){
                        unsigned char b[8192]; long n = LIT_N;
                        memcpy(b, LIT, (size_t)n + 1);
                        avanca();
                        /* `char X[] = "ab"` são três bytes, com o zero — é o que o C diz.
                         * `char X[n] = "ab"` são n, e o resto nasce a zero. */
                        if(quantos <= 0) quantos = n + 1;
                        if(acha_slot(nome) < 0){
                            abre_slot(nome, ret, quantos);
                            long onde = SLOTS[acha_slot(nome)].endereco;
                            long q = n + 1 < quantos ? n + 1 : quantos;
                            escreve_imagem(onde, b, q);
                        }
                        aberto = 1;
                    }
                    if(!aberto && acha_slot(nome) < 0) abre_slot(nome, ret, quantos);
                    if(!aceita(",")) break;
                    while(e_pun("*")) avanca();
                    if(T.k != TK_ID) break;
                    strcpy(nome, T.s); avanca();
                }
                continue;
            }
            avanca();
            /* a declaração para a frente e a definição são o MESMO nome: se já entrou, não
             * entra outra vez — senão o módulo exporta o nome duas vezes e o motor recusa-o */
            if(acha_fun(nome) >= 0){
                while(!e_pun(")") && T.k != TK_FIM) avanca();
                avanca();
                continue;
            }
            if(NFUN >= MAX_FUN){
                /* dizer QUANTAS: sem o número, quem lê não sabe se falta uma ou cem */
                char m[96];
                snprintf(m, sizeof m, "funções a mais: %ld, e o tecto é %d", (long)NFUN + 1, MAX_FUN);
                erro(m);
            }
            Fun *f = &FUNS[NFUN];
            memset(f, 0, sizeof *f);
            strcpy(f->nome, nome); f->ret = ret; f->interna = interna;
            while(!e_pun(")") && T.k != TK_FIM){
                /* `void` só quer dizer «sem parâmetros» se vier o fecho a seguir. Estava a engolir o
                 * `void` de `void *ctx` e a tropeçar no `*` — o vazio e o endereço de coisa
                 * nenhuma são a mesma palavra e não são a mesma coisa. */
                if(e_id("void")){
                    long g = POS; Tok gt = T; long gl = LINHA;
                    avanca();
                    if(e_pun(")")) continue;
                    POS = g; T = gt; LINHA = gl;
                }
                if(e_pun("...")){ avanca(); f->variadica = 1; break; }
                if(!e_tipo()) erro("tipo do parâmetro");
                /* uma estrutura por valor viaja como MORADA DE UMA CÓPIA: quem chama
                 * copia-a para o seu quadro e passa o endereço. É isso que «por valor»
                 * quer dizer — o que o chamado mexer não volta para trás. */
                char pn[64];
                int pt = le_declarador(le_tipo(), pn);
                f->par[f->npar++] = pt;
                if(!aceita(",")) break;
            }
            come(")");
            if(f->variadica){
                f->idx_fita = f->npar;
                f->par[f->npar++] = MKT(TI8, 1);      /* o endereço da fita, no fim */
            }
            NFUN++;
            continue;
        }
        avanca();
    }
}

/* ──────────────────────────────────────────────────────────── o módulo */

#define MOD_B      DISCO_FIXO(unsigned char, 33)
#define SEC_B      DISCO_FIXO(unsigned char, 34)
#define CORPOS_B   DISCO_FIXO(unsigned char, 35)
#define CORPO_OFF  DISCO_FIXO(long, 50)
#define CORPO_TAM  DISCO_FIXO(long, 51)
#define REVERSO_B  DISCO_FIXO(char, 62)     /* a CAUDA REVERSÍVEL: os # e comentários do original */
static long REV_N = 0, REV_CFG = 0;         /* total, e onde a config (#) acaba e o comentário começa */

static void seccao(int id, Buf *corpo){
    bput(&MOD, (unsigned)id);
    bu(&MOD, (unsigned long)corpo->n);
    bmany(&MOD, corpo->b, corpo->n);
    corpo->n = 0;
}

/* A CAUDA REVERSÍVEL. O lexer descarta os # e os comentários --- e descartar é apagar, o buraco
 * negro que não reverte. Em vez disso codificam-se numa secção CUSTOM (id 0), que o runtime IGNORA
 * (o módulo corre igual) mas a volta LÊ --- exactamente como o `.tex` viaja invisível no PDF pelo
 * `/Type/FonteTeX`. A estrela guarda a segunda metade; a volta reverte-a. A config (os #) fica no
 * CENTRO (logo a seguir ao código), os comentários no fim. */
static void seccao_custom(const char *nome, const unsigned char *dados, long n){
    long ln = (long)strlen(nome);
    SEC.n = 0;
    bu(&SEC, (unsigned long)ln); bmany(&SEC, nome, ln);   /* [namelen][name] */
    bmany(&SEC, dados, n);                                /* [payload] */
    seccao(0, &SEC);                                      /* id 0 = custom, ignorada pelo runtime */
}
/* varre o ORIGINAL (antes da expansão de macros, que reescreve o SRC) e guarda os # e os
 * comentários em REVERSO_B: [config: linhas #][comentários]. Salta strings e chars. */
static void captura_reverso(const char *s){
    Buf r; r.b = (unsigned char*)REVERSO_B; r.n = 0; r.cap = (1<<21);
    for(long i = 0; s[i]; ){                              /* 1: as directivas # (a config) */
        char c = s[i];
        if(c=='"'||c=='\''){ char q=c; i++; while(s[i]&&s[i]!=q){ if(s[i]=='\\'&&s[i+1])i++; i++; } if(s[i])i++; continue; }
        if(c=='#' && (i==0||s[i-1]=='\n')){ long j=i; while(s[j]&&s[j]!='\n')j++;
            /* um `/*` SEM fecho na mesma linha ficava PENDURADO no replay da
             * config e engolia as directivas seguintes na recaptura — o
             * round-trip da libc divergia 14551 bytes por isto (auditoria
             * 14/08). Corta-se a directiva no abridor; o comentário inteiro
             * viaja pela passagem 2, nada se perde. */
            long fim=j;
            for(long k2=i;k2+1<j;k2++){
                if(s[k2]=='/'&&s[k2+1]=='*'){
                    long m2=k2+2; int fechou=0;
                    while(m2+1<j){ if(s[m2]=='*'&&s[m2+1]=='/'){ fechou=1; break; } m2++; }
                    if(!fechou){ fim=k2; while(fim>i&&s[fim-1]==' ')fim--; }
                    break;
                }
            }
            bmany(&r,s+i,fim-i); bput(&r,'\n'); i=j; continue; }
        i++;
    }
    REV_CFG = r.n;
    for(long i = 0; s[i]; ){                              /* 2: os comentários */
        char c = s[i];
        if(c=='"'||c=='\''){ char q=c; i++; while(s[i]&&s[i]!=q){ if(s[i]=='\\'&&s[i+1])i++; i++; } if(s[i])i++; continue; }
        if(c=='/'&&s[i+1]=='/'){ long j=i; while(s[j]&&s[j]!='\n')j++; bmany(&r,s+i,j-i); bput(&r,'\n'); i=j; continue; }
        if(c=='/'&&s[i+1]=='*'){ long j=i+2; while(s[j]&&!(s[j]=='*'&&s[j+1]=='/'))j++; if(s[j])j+=2; bmany(&r,s+i,j-i); bput(&r,'\n'); i=j; continue; }
        i++;
    }
    REV_N = r.n;
}

/* ═══════════════════════════════════════════════════════════════════════════════════════
 *  A DESCIDA  —  +1 absorve
 *
 *  «MOVE(slot, sentido): +1 absorve, −1 emite: é a interface. […] −1 sozinho só emite, +1
 *   sozinho só absorve, cada um meia operação.»  (corpo_analitico.tex)
 *
 *  Até aqui só havia o −1, e por isso a medida só podia COMPARAR — e comparar é ler metade,
 *  e metade dissipa. Com o +1 a medida deixa de comparar: REVERTE, e lê o resíduo.
 *
 *  E a reversão é literal. A pilha do wasm devolve a árvore do C porque a árvore era a pilha:
 *  desempilhar dois e empilhar `(a op b)` é a mesma observação lida ao contrário. Não há
 *  heurística nem reconstrução aproximada — o que sobe desce.
 * ═══════════════════════════════════════════════════════════════════════════════════════ */

#define MAXQ    64
#define CAP_TXT (1<<20)

#define TXT        DISCO_FIXO(char, 36)
#define PILHA      DISCO_FIXO2(char, 512, 52)

static void tput(const char *s){
    long L = (long)strlen(s);
    if(TXT_N + L >= CAP_TXT){ fprintf(stderr, "traduz: texto cheio\n"); exit(2); }
    memcpy(TXT + TXT_N, s, (size_t)L); TXT_N += L; TXT[TXT_N] = 0;
}
static void tins(long onde, const char *s){
    long L = (long)strlen(s);
    if(TXT_N + L >= CAP_TXT){ fprintf(stderr, "traduz: texto cheio\n"); exit(2); }
    memmove(TXT + onde + L, TXT + onde, (size_t)(TXT_N - onde));
    memcpy(TXT + onde, s, (size_t)L); TXT_N += L; TXT[TXT_N] = 0;
}
/* O QUE ENTRA VEM DE ONDE VAI SAIR. `puxa` devolve um ponteiro para o slot que acabou de
 * libertar, e `empurra` escrevia NESSE MESMO slot enquanto o lia — a origem e o destino eram
 * a mesma casa. Formata-se à parte e só depois se copia: uma escrita que lê o que apaga não
 * é uma escrita, é meia. */
static void empurra(const char *fmt, ...){
    if(NP >= 64){ fprintf(stderr, "traduz: pilha cheia\n"); exit(2); }
    char tmp[512];
    va_list ap; va_start(ap, fmt);
    vsnprintf(tmp, sizeof tmp, fmt, ap);
    va_end(ap);
    memcpy(PILHA[NP], tmp, sizeof tmp); NP++;
}
static int DBG_FN; static unsigned DBG_OP; static long DBG_MP;
static unsigned DBG_ANEL[64]; static long DBG_AMP[64]; static int DBG_NP[64]; static int DBG_I;
static const char *puxa(void){
    if(NP <= 0){ fprintf(stderr, "traduz: pilha vazia na descida (fn=%d op=0x%02X MP=%ld)\n",
                          DBG_FN, DBG_OP, (long)DBG_MP);
                 for(int z = 0; z < 64; z++){ int k = (DBG_I + z) & 63;
                     fprintf(stderr, "  anel[%2d] MP=%ld op=0x%02X NP=%d\n", z, DBG_AMP[k], (unsigned)DBG_ANEL[k], DBG_NP[k]); }
                 exit(2); }
    return PILHA[--NP];
}

/* os quadros: cada bloco estruturado do wasm é um bloco do C, e fecha onde abriu */
enum { Q_SE, Q_SENAO, Q_LACO, Q_CORPO, Q_EXPR };
typedef struct {
    int  tipo;
    long ini;                 /* onde o texto deste quadro começa */
    long ini_inc;             /* onde o incremento do `for` começa */
    char cond[512];
    int  tem_cond, e_faz, e_para, fecha;
    char entao[512];
} Quadro;
#define QS         DISCO_FIXO(Quadro, 53)

/* OS ROTULOS, QUE NAO SAO OS QUADROS. Um laco e' UM quadro do C e DOIS rotulos do wasm — o
 * bloco de fora, para onde salta o `break`, e o `loop` de dentro, para onde salta o
 * `continue`. Contar quadros em vez de rotulos punha o `br 3` a apontar para o vizinho, e um
 * `break` descia como `continue`: o programa continuava a compilar e fazia outra coisa.
 * Nenhuma assercao o via — a VOLTA viu, num byte. */
enum { R_NEUTRO, R_QUEBRA, R_SEGUE };
typedef struct { int papel, quadro; } Rotulo;
#define LAB        DISCO_FIXO(Rotulo, 54)
static void abre_rotulo(int papel, int quadro){ LAB[NLAB].papel = papel; LAB[NLAB].quadro = quadro; NLAB++; }

static void recua(int prof){ for(int i = 0; i < prof + 1; i++) tput("    "); }
static void frase(int prof, const char *fmt, ...){
    char l[1024]; va_list ap; va_start(ap, fmt);
    vsnprintf(l, sizeof l, fmt, ap); va_end(ap);
    recua(prof); tput(l); tput("\n");
}

/* o número que volta tem de voltar NO SEU TIPO: um `double` sem ponto lê-se inteiro, e um
 * `long` sem o `L` lê-se int — e aí os bytes que saem já não são os que entraram */
static void real_txt(uint64_t d, char *o, long cap){
    real_txt_bits(d, o, cap);
}

typedef struct { unsigned op; const char *sim; } Sinal;
static const Sinal SINAIS[] = {
    {0x6A,"+"},{0x6B,"-"},{0x6C,"*"},{0x6D,"/"},{0x6F,"%"},
    {0x71,"&"},{0x72,"|"},{0x73,"^"},{0x74,"<<"},{0x75,">>"},
    {0x46,"=="},{0x47,"!="},{0x48,"<"},{0x4A,">"},{0x4C,"<="},{0x4E,">="},
    {0x7C,"+"},{0x7D,"-"},{0x7E,"*"},{0x7F,"/"},{0x81,"%"},
    {0x83,"&"},{0x84,"|"},{0x85,"^"},{0x86,"<<"},{0x87,">>"},
    {0x51,"=="},{0x52,"!="},{0x53,"<"},{0x55,">"},{0x57,"<="},{0x59,">="},
    {0xA0,"+"},{0xA1,"-"},{0xA2,"*"},{0xA3,"/"},
    {0x61,"=="},{0x62,"!="},{0x63,"<"},{0x64,">"},{0x65,"<="},{0x66,">="},
};
#define NSINAIS ((int)(sizeof SINAIS / sizeof SINAIS[0]))
static const char *sinal_de(unsigned op){
    for(int i = 0; i < NSINAIS; i++) if(SINAIS[i].op == op) return SINAIS[i].sim;
    return 0;
}
static const char *molde_de(unsigned op){
    switch(op){
        case 0xAC: case 0xB0: return "long";
        case 0xB7: case 0xB9: return "double";
        case 0xA7: case 0xAA: return "int";
    }
    return 0;
}

/* o módulo lido */
/* M, MN e MP — o módulo lido e o cursor — vivem no Reg, no disco. */
static unsigned long d_u(void){
    unsigned long v = 0; int d = 0, c;
    do { c = M[MP++]; v |= (unsigned long)(c & 0x7F) << d; d += 7; } while(c & 0x80);
    return v;
}
static long d_s(void){
    long v = 0; int d = 0, c;
    do { c = M[MP++]; v |= (long)(c & 0x7F) << d; d += 7; } while(c & 0x80);
    if(d < 64 && (c & 0x40)) v |= -(1L << d);
    return v;
}

static const char *NOME_T[4] = { "int", "long", "double", "void" };
static int t_de_val(unsigned v){ return v == 0x7F ? TI32 : v == 0x7E ? TI64 : TF64; }

typedef struct { int par[16], npar, ret; } Assin;
static int DESCE_NIMP;
static Assin DESCE_IMP_ASS[8];
static char DESCE_IMP_NOME[8][64];
#define ASS        DISCO_FIXO(Assin, 56)
#define NOMES      DISCO_FIXO2(char, 64, 57)
/* quem está na secção de exportação. Quem NÃO está desce como `static` — sem isto a volta
 * abria a porta a todos e sobe(desce(M)) vinha com sete exports a mais que o M.
 * No disco, como os outros: um static aqui eram 256 bytes de .bss, e 256 não é 0. E o disco
 * PERSISTE entre corridas — por isso zera-se no início da descida, onde a .bss zerava só. */
#define EXPORTADA  DISCO_FIXO(unsigned char, 47)

/* A descida do corpo de uma função. Devolve 0 se encontrou algo que não sabe desfazer —
 * e dizê-lo é melhor do que inventar C que não corresponde. */
static int desce_corpo(long fim, int fidx){
    DBG_FN = fidx;
    NP = 0; NQ = 0; NLAB = 0;
    int prof = 0;
    while(MP < fim){
        unsigned op = M[MP++];
        DBG_OP = op; DBG_MP = MP;
        DBG_ANEL[DBG_I & 63] = op; DBG_AMP[DBG_I & 63] = MP; DBG_NP[DBG_I & 63] = NP; DBG_I++;

        /* a condição de um laço: `<c> i32.eqz br_if <bloco de fora>` — o `eqz` está lá porque
         * o wasm sai quando é verdade e o C fica enquanto é verdade; ao voltar, tira-se. */
        if(op == 0x45 && MP < fim && M[MP] == 0x0D && NQ > 0
           && QS[NQ-1].tipo == Q_LACO && !QS[NQ-1].tem_cond){
            MP++; (void)d_u();                                          /* salta para fora */
            snprintf(QS[NQ-1].cond, sizeof QS[NQ-1].cond, "%s", puxa());
            QS[NQ-1].tem_cond = 1;
            continue;
        }

        const char *sim = sinal_de(op);
        if(sim){
            const char *b = puxa(); char bb[512]; snprintf(bb, sizeof bb, "%s", b);
            const char *a = puxa();
            empurra("(%s %s %s)", a, sim, bb);
            continue;
        }
        const char *molde = molde_de(op);
        if(molde){ empurra("(%s)(%s)", molde, puxa()); continue; }

        switch(op){
        case 0x20: empurra("v%lu", d_u()); break;                       /* local.get */
        case 0x21: {                                                    /* local.set */
            unsigned long i = d_u();
            char v[512]; snprintf(v, sizeof v, "%s", puxa());
            /* O NOME NAO SOBREVIVE A' ESCRITA QUE O INVALIDA. Se por baixo ficou o proprio
             * `vi` e o que se grava e' `(vi ± 1)`, isto e' o pos-fixo: o valor que ficou na
             * pilha e' o VELHO, e escrever `vi` depois de `vi` mudar diz o novo — outra
             * coisa. Devolve-se `(vi++)`, que e' o que subiu. */
            char eu[32], mais[544], menos[544];
            snprintf(eu, sizeof eu, "v%lu", i);
            snprintf(mais,  sizeof mais,  "(v%lu + 1)", i);
            snprintf(menos, sizeof menos, "(v%lu - 1)", i);
            if(NP > 0 && !strcmp(PILHA[NP-1], eu) && (!strcmp(v, mais) || !strcmp(v, menos))){
                NP--;
                empurra("(v%lu%s)", i, !strcmp(v, mais) ? "++" : "--");
                break;
            }
            frase(prof, "v%lu = %s;", i, v);
        } break;
        case 0x22: { unsigned long i = d_u(); empurra("(v%lu = %s)", i, puxa()); } break;
        case 0x41: empurra("%ld", d_s()); break;                        /* i32.const */
        case 0x42: empurra("%ldL", d_s()); break;                       /* i64.const */
        case 0x44: { uint64_t d; memcpy(&d, M + MP, 8); MP += 8;
                     char o[64]; real_txt(d, o, sizeof o); empurra("%s", o); } break;
        /* MOVE(+1): do slot para o registo. Volta como `*(T*)(endereço)` — a forma que
         * torna a subir exactamente no mesmo opcode, sem eu ter de adivinhar se aquilo era
         * um vector ou um escalar: o endereço está lá, e o endereço é o slot. */
        case 0x28: case 0x29: case 0x2B: case 0x2C: case 0x2D: {
            const char *T2 = op == 0x28 ? "int" : op == 0x29 ? "long" : op == 0x2B ? "double"
                : "unsigned char";
            (void)d_u(); (void)d_u();                                   /* alinhamento, desvio */
            empurra("*(%s*)(%s)", T2, puxa());
        } break;
        /* e MOVE(-1): do registo para o slot */
        case 0x36: case 0x37: case 0x39: case 0x3A: case 0x3C: {
            const char *T2 = op == 0x36 ? "int" : op == 0x37 ? "long" : op == 0x39 ? "double"
                : "unsigned char";
            (void)d_u(); (void)d_u();
            char v[512]; snprintf(v, sizeof v, "%s", puxa());
            frase(prof, "*(%s*)(%s) = %s;", T2, puxa(), v);
        } break;
        case 0x3F: MP++; empurra("__disco_paginas()"); break;           /* memory.size */
        case 0x40: MP++; empurra("__disco_cresce(%s)", puxa()); break;  /* memory.grow */
        case 0x45: case 0x50: empurra("(!%s)", puxa()); break;          /* eqz */
        case 0x9A: empurra("(-%s)", puxa()); break;                     /* f64.neg */
        case 0x1A: frase(prof, "%s;", puxa()); break;                   /* drop: era um efeito */
        case 0x0F:                                                      /* return */
            if(ASS[fidx].ret == TVOID) frase(prof, "return;");
            else frase(prof, "return %s;", puxa());
            break;
        case 0x10: {                                                    /* call */
            unsigned long f = d_u();
            if(f < (unsigned long)DESCE_NIMP){
                /* a chamada é ao IMPORT: nome e assinatura da secção 2 */
                char args[512] = ""; int n = DESCE_IMP_ASS[f].npar;
                char peca[16][512];
                for(int i = n - 1; i >= 0; i--) snprintf(peca[i], 512, "%s", puxa());
                for(int i = 0; i < n; i++){
                    if(i) strncat(args, ", ", sizeof args - strlen(args) - 1);
                    strncat(args, peca[i], sizeof args - strlen(args) - 1);
                }
                if(DESCE_IMP_ASS[f].ret == TVOID) frase(prof, "%s(%s);", DESCE_IMP_NOME[f], args);
                else empurra("%s(%s)", DESCE_IMP_NOME[f], args);
                break;
            }
            f -= (unsigned long)DESCE_NIMP;
            char args[512] = ""; int n = ASS[f].npar;
            char peca[16][512];
            for(int i = n - 1; i >= 0; i--) snprintf(peca[i], 512, "%s", puxa());
            for(int i = 0; i < n; i++){
                if(i) strncat(args, ", ", sizeof args - strlen(args) - 1);
                strncat(args, peca[i], sizeof args - strlen(args) - 1);
            }
            if(ASS[f].ret == TVOID) frase(prof, "%s(%s);", NOMES[f], args);
            else empurra("%s(%s)", NOMES[f], args);
        } break;

        case 0x04: {                                                    /* if */
            unsigned bt = M[MP++];
            if(bt != 0x40){                                             /* if com resultado */
                QS[NQ].tipo = Q_EXPR; QS[NQ].ini = TXT_N; QS[NQ].fecha = 1;
                snprintf(QS[NQ].cond, sizeof QS[NQ].cond, "%s", puxa());
                QS[NQ].entao[0] = 0; abre_rotulo(R_NEUTRO, NQ); NQ++;
            } else {
                QS[NQ].tipo = Q_SE; QS[NQ].ini = TXT_N; QS[NQ].fecha = 1;
                frase(prof, "if(%s){", puxa());
                abre_rotulo(R_NEUTRO, NQ); NQ++; prof++;
            }
        } break;

        case 0x05:                                                      /* else */
            if(NQ > 0 && QS[NQ-1].tipo == Q_EXPR){
                snprintf(QS[NQ-1].entao, sizeof QS[NQ-1].entao, "%s", puxa());
                QS[NQ-1].tipo = Q_SENAO;
            } else {
                prof--; frase(prof, "} else {"); prof++;
            }
            break;

        case 0x02: {                                                    /* block */
            MP++;                                                       /* o tipo, que é vazio */
            if(MP < fim && M[MP] == 0x03){                              /* block+loop = um laço */
                MP += 2;
                QS[NQ].tipo = Q_LACO; QS[NQ].ini = TXT_N; QS[NQ].tem_cond = 0;
                QS[NQ].cond[0] = 0;              /* limpo: senão herda a condição do anterior */
                QS[NQ].e_faz = 0; QS[NQ].e_para = 0; QS[NQ].ini_inc = -1; QS[NQ].fecha = 2;
                abre_rotulo(R_QUEBRA, NQ);                    /* o bloco de fora: o `break` */
                abre_rotulo(R_SEGUE,  NQ);                    /* o `loop`: o `continue` */
                NQ++; prof++;
            } else {                                                    /* o corpo de um `for` */
                QS[NQ].tipo = Q_CORPO; QS[NQ].ini = TXT_N; QS[NQ].fecha = 1;
                abre_rotulo(R_SEGUE, NQ);         /* o corpo do `for`: o `continue` faz o passo */
                NQ++;
                if(NQ >= 2 && QS[NQ-2].tipo == Q_LACO) QS[NQ-2].e_para = 1;
            }
        } break;

        case 0x03: return 0;                                            /* loop sem bloco: não sei */

        case 0x0C: {                                                    /* br */
            int L = NLAB - 1 - (int)d_u();
            /* a volta ao princípio do laço é a própria estrutura, e não se escreve;
             * qualquer outro salto é um `break` ou um `continue` */
            if(L < 0) return 0;
            /* A VOLTA DO LAÇO é a que está AO NÍVEL DO PRÓPRIO LAÇO — isto é, a que aponta
             * para o rótulo mais interior que está aberto. Um `continue` dentro de um `if`
             * aponta para o mesmo sítio mas de mais fundo, e eu estava a deitá-lo fora como
             * se fosse a volta: o corpo passava a cair para a frente em vez de voltar. */
            if(L == NLAB - 1 && LAB[L].papel == R_SEGUE && MP < fim && M[MP] == 0x0B
               && QS[LAB[L].quadro].tipo == Q_LACO) break;
            frase(prof, LAB[L].papel == R_QUEBRA ? "break;" : "continue;");
        } break;

        case 0x0D: {                                                    /* br_if */
            int L = NLAB - 1 - (int)d_u();
            char c[512]; snprintf(c, sizeof c, "%s", puxa());
            if(L < 0) return 0;
            int q = LAB[L].quadro;
            if(LAB[L].papel == R_SEGUE && QS[q].tipo == Q_LACO && !QS[q].e_para
               && MP < fim && M[MP] == 0x0B){
                snprintf(QS[q].cond, sizeof QS[q].cond, "%s", c);       /* o `do…while` */
                QS[q].tem_cond = 1; QS[q].e_faz = 1;
            } else frase(prof, "if(%s) %s", c, LAB[L].papel == R_QUEBRA ? "break;" : "continue;");
        } break;

        case 0x0B: {                                                    /* end */
            if(NQ == 0) return 1;                                       /* o fim da função */
            if(NLAB > 0) NLAB--;                                        /* um `end`, um rótulo */
            Quadro *q = &QS[NQ-1];
            if(q->tipo == Q_EXPR || q->tipo == Q_SENAO){
                /* o `&&` e o `||`: o emissor normaliza com dois `eqz`, e ao voltar tiram-se —
                 * a normalização é dele, não da fonte, e deixá-la duplicava-a */
                char b[512]; snprintf(b, sizeof b, "%s", puxa());
                char *dentro = b;
                char limpo[512];
                if(!strncmp(b, "(!(!", 4)){
                    long L = (long)strlen(b);
                    if(L > 6){ snprintf(limpo, sizeof limpo, "%.*s", (int)(L - 6), b + 4); dentro = limpo; }
                }
                /* O && E O || CONHECEM-SE PELA FORMA, e a forma é inteira: o `||` tem `1` de
                 * um lado E a dupla negação do outro. Olhar só para o `1` fazia um ternário
                 * `c ? 1 : y` descer como `(c || y)` — que ao subir leva a normalização que ele
                 * não tinha, e os bytes deixavam de fechar. */
                char e[512]; snprintf(e, sizeof e, "%s", q->entao);
                int dupla_dir = !strncmp(b, "(!(!", 4);
                int dupla_esq = !strncmp(e, "(!(!", 4);
                if(!strcmp(q->entao, "1") && dupla_dir) empurra("(%s || %s)", q->cond, dentro);
                else if(!strcmp(b, "0") && dupla_esq){
                    char l2[512];
                    long L = (long)strlen(e);
                    snprintf(l2, sizeof l2, "%.*s", (int)(L - 6), e + 4);
                    empurra("(%s && %s)", q->cond, l2);
                } else empurra("(%s ? %s : %s)", q->cond, e, b);
                NQ--;
                break;
            }
            if(q->tipo == Q_SE){
                prof--; frase(prof, "}");
                NQ--;
                break;
            }
            if(q->tipo == Q_CORPO){
                q->ini_inc = TXT_N;                    /* o que vier a seguir é o incremento */
                if(NQ >= 2 && QS[NQ-2].tipo == Q_LACO) QS[NQ-2].ini_inc = TXT_N;
                NQ--;
                break;
            }
            /* Q_LACO: dois `end` fecham-no, e ao segundo escreve-se a cabeça */
            if(--q->fecha > 0) break;
            prof--;
            char cab[600];
            /* um laço SEM condição é `for(;;)` — e é preciso dizê-lo, senão sai um `while`
             * com a condição de quem passou por aqui antes. O `strchr` da biblioteca tem um,
             * e foi ele que o mostrou. */
            if(!q->tem_cond && !q->e_faz && !q->e_para){
                snprintf(cab, sizeof cab, "for(;;){\n");
                char rec0[64] = ""; for(int i = 0; i < prof + 1; i++) strcat(rec0, "    ");
                char tudo0[700]; snprintf(tudo0, sizeof tudo0, "%s%s", rec0, cab);
                tins(q->ini, tudo0);
                frase(prof, "}");
                NQ--;
                break;
            }
            if(q->e_para){
                char inc[512] = "";
                if(q->ini_inc >= 0 && TXT_N > q->ini_inc){
                    long L = TXT_N - q->ini_inc;
                    if(L > 500) L = 500;
                    memcpy(inc, TXT + q->ini_inc, (size_t)L); inc[L] = 0;
                    char *fimc = strchr(inc, ';'); if(fimc) *fimc = 0;
                    char *ini = inc; while(*ini == ' ') ini++;
                    memmove(inc, ini, strlen(ini) + 1);
                    TXT_N = q->ini_inc; TXT[TXT_N] = 0;   /* sai do corpo e vai para a cabeça */
                }
                snprintf(cab, sizeof cab, "for(; %s; %s){\n", q->cond, inc);
            } else if(q->e_faz){
                snprintf(cab, sizeof cab, "do {\n");
            } else {
                snprintf(cab, sizeof cab, "while(%s){\n", q->cond);
            }
            char rec[64] = ""; for(int i = 0; i < prof + 1; i++) strcat(rec, "    ");
            char tudo[700]; snprintf(tudo, sizeof tudo, "%s%s", rec, cab);
            tins(q->ini, tudo);
            if(q->e_faz) frase(prof, "} while(%s);", q->cond);
            else frase(prof, "}");
            NQ--;
        } break;

        default:
            return 0;                                   /* um opcode que eu não emito: digo-o */
        }
    }
    return 1;
}

/* a descida do módulo inteiro: as secções, e depois cada corpo */
/* A ESTRELA REVERTE A CAUDA: relê as secções custom (id 0) e reemite os # e comentários no C, para
 * que sobe(desce(M)) recapture a MESMA cauda --- é o dual de captura_reverso, e fecha o round-trip. */
static void reverte_cauda(FILE *o){
    long p = 8;
    while(p < MN){
        int id = M[p++];
        MP = p; long t = (long)d_u(); long body = MP;    /* d_u() avança MP para o corpo */
        p = body + t;
        if(id == 0){                                     /* custom: [namelen][name][payload] */
            MP = body; long ln = (long)d_u(); long payload = MP + ln;
            long plen = body + t - payload;
            if(plen > 0) fwrite(M + payload, 1, (size_t)plen, o);   /* as # linhas ou os comentários, verbatim */
        }
    }
}

static int desce_modulo(const unsigned char *b, long n, FILE *o){
    M = b; MN = n;
    if(n < 8 || memcmp(b, "\0asm", 4) || b[4] != 1) return 0;
    long off[16] = {0};   /* o `tam[16]` era preenchido e nunca lido; o tamanho serve
                           * antes para GUARDAR contra um modulo truncado, abaixo */
    MP = 8;
    while(MP < MN){
        int id = M[MP++]; long t = (long)d_u();
        if(t < 0 || MP + t > MN) return 0;        /* a seccao tem de caber no modulo */
        if(id >= 0 && id < 16) off[id] = MP;
        MP += t;
    }
    if(!off[1] || !off[3] || !off[10]) return 0;

    /* os tipos */
    MP = off[1];
    long nt = (long)d_u();
    Assin tipos[MAX_FUN];
    for(long i = 0; i < nt && i < MAX_FUN; i++){
        MP++;                                            /* 0x60 */
        long np = (long)d_u();
        tipos[i].npar = (int)np;
        for(long j = 0; j < np; j++) tipos[i].par[j] = t_de_val(M[MP++]);
        long nr = (long)d_u();
        tipos[i].ret = nr ? t_de_val(M[MP++]) : TVOID;
    }
    /* OS IMPORTS DESLOCAM O ÍNDICE — a subida sabe-o (WASM_FUN = f + N_IMP)
     * e a descida ESQUECIA: com o env.__fich_miss no módulo, toda chamada
     * resolvia o NOME e o npar do vizinho — e a pilha «esvaziava» no fputc
     * porque o npar lido era o da função seguinte (auditoria 14/08, o anel
     * de opcodes). Lê-se a secção 2, guarda-se o tipo e o nome de cada
     * import de função, e TODO índice vindo do bytecode desconta-os. */
    int nimp_fn = 0;
    static Assin IMP_ASS[8];
    static char IMP_NOME[8][64];
    if(off[2]){
        MP = off[2];
        long ni = (long)d_u();
        for(long i = 0; i < ni; i++){
            long Lm = (long)d_u(); MP += Lm;               /* módulo (env) */
            long Ln = (long)d_u();
            char nomei[64]; long k2 = Ln < 63 ? Ln : 63;
            memcpy(nomei, M + MP, (size_t)k2); nomei[k2] = 0; MP += Ln;
            int kindi = M[MP++];
            if(kindi == 0){
                unsigned long ti = d_u();
                if(nimp_fn < 8){
                    IMP_ASS[nimp_fn] = tipos[ti < (unsigned long)nt ? ti : 0];
                    snprintf(IMP_NOME[nimp_fn], 64, "%s", nomei);
                    nimp_fn++;
                }
            } else if(kindi == 2){ (void)d_u(); if(M[MP-1] & 1) (void)d_u(); }
            else { (void)d_u(); }
        }
    }
    DESCE_NIMP = nimp_fn;
    for(int i = 0; i < nimp_fn && i < 8; i++){ DESCE_IMP_ASS[i] = IMP_ASS[i]; snprintf(DESCE_IMP_NOME[i], 64, "%s", IMP_NOME[i]); }
    /* que tipo tem cada função */
    MP = off[3];
    long nf = (long)d_u();
    for(long i = 0; i < nf && i < MAX_FUN; i++) ASS[i] = tipos[d_u()];
    N_ASS = (int)nf;
    /* e o nome */
    for(int i = 0; i < N_ASS; i++) snprintf(NOMES[i], 64, "f%d", i);
    memset(EXPORTADA, 0, MAX_FUN);      /* o disco persiste; um 1 de outra corrida mentia aqui */
    int disco_sai = 0;      /* o módulo pôs o disco na porta? a descida devolve o mesmo */
    if(off[7]){
        MP = off[7];
        long ne = (long)d_u();
        for(long i = 0; i < ne; i++){
            long L = (long)d_u();
            char nome[64]; long k = L < 63 ? L : 63;
            memcpy(nome, M + MP, (size_t)k); nome[k] = 0; MP += L;
            int kind = M[MP++]; unsigned long idx = d_u();
            if(kind == 0 && idx >= (unsigned long)nimp_fn
               && idx - nimp_fn < (unsigned long)N_ASS){
                snprintf(NOMES[idx - nimp_fn], 64, "%s", nome);
                EXPORTADA[idx - nimp_fn] = 1;
            }
            if(kind == 2) disco_sai = 1;
        }
    }

    /* o disco: a secção diz as páginas, e uma página são 65536 slots de um byte. E a
     * secção de dados diz o que nasce escrito — devolve-se em três pedaços, porque é a
     * forma em C que torna a subir no MESMO segmento, no mesmo sítio: o que vem antes,
     * o que nasce escrito, e o que vem depois. */
    long total = 0;
    if(off[5]){
        MP = off[5];
        long quantas = (long)d_u();
        if(quantas > 0){
            int lim = M[MP++];
            long pag = (long)d_u();
            if(lim) (void)d_u();                     /* o tecto, que é o mesmo */
            total = pag * 65536;
        }
    }
    if(total > 0){
        long ini = 0, n = 0, dados = 0;
        if(off[11]){
            MP = off[11];
            long nseg = (long)d_u();
            if(nseg > 0){
                (void)d_u();                          /* o segmento activo */
                MP++;                                 /* 0x41 */
                ini = d_s();
                MP++;                                 /* 0x0B */
                n = (long)d_u();
                dados = MP;
            }
        }
        /* sem nada escrito, o disco é um só e chama-se DISCO. Chamou-se MEMORIA até o Aarão
         * apontar o óbvio: num sistema reversível memória não existe — memória é o que se
         * reconstrói e dissipa; o que persiste e não custa é disco. O nome estava a
         * contradizer a especificação dentro do próprio contrato. */
        /* desconta-se a reserva do zero: o tradutor põe-na sozinho, e declará-la outra vez
         * empurrava tudo oito bytes para a frente */
        if(n == 0){ fprintf(o, "%schar DISCO[%ld];\n\n", disco_sai ? "" : "static ", total - 8); }
        else {
        if(ini - 8 > 0) fprintf(o, "%schar DISCO[%ld];\n", disco_sai ? "" : "static ", ini - 8);
        if(n > 0){
            fprintf(o, "%schar LIT[%ld] = \"", disco_sai ? "" : "static ", n);
            for(long i = 0; i < n; i++){
                unsigned c = M[dados + i];
                if(c == '"' || c == '\\') fprintf(o, "\\%c", c);
                else if(c >= 32 && c < 127)  fputc((int)c, o);
                else fprintf(o, "\\%03o", c);
            }
            fprintf(o, "\";\n");
        }
        long resto = total - ini - n;
        if(resto > 0) fprintf(o, "%schar RESTO[%ld];\n", disco_sai ? "" : "static ", resto);
        fprintf(o, "\n");
        }
    }

    /* e os corpos. Primeiro as assinaturas todas, para que chamar para a frente volte igual */
    for(int i = 0; i < N_ASS; i++){
        fprintf(o, "%s%s %s(", EXPORTADA[i] ? "" : "static ", NOME_T[ASS[i].ret], NOMES[i]);
        if(!ASS[i].npar) fprintf(o, "void");
        for(int j = 0; j < ASS[i].npar; j++)
            fprintf(o, "%s%s v%d", j ? ", " : "", NOME_T[ASS[i].par[j]], j);
        fprintf(o, ");\n");
    }
    fprintf(o, "\n");

    MP = off[10];
    long nc = (long)d_u();
    for(long i = 0; i < nc && i < N_ASS; i++){
        long tcorpo = (long)d_u();
        long fim = MP + tcorpo;
        long ngrupos = (long)d_u();
        int tl[MAX_LOC]; int nl = 0;
        for(long g = 0; g < ngrupos; g++){
            long quantos = (long)d_u();
            int t = t_de_val(M[MP++]);
            for(long k = 0; k < quantos && nl < MAX_LOC; k++) tl[nl++] = t;
        }
        fprintf(o, "%s%s %s(", EXPORTADA[i] ? "" : "static ", NOME_T[ASS[i].ret], NOMES[i]);
        if(!ASS[i].npar) fprintf(o, "void");
        for(int j = 0; j < ASS[i].npar; j++)
            fprintf(o, "%s%s v%d", j ? ", " : "", NOME_T[ASS[i].par[j]], j);
        fprintf(o, "){\n");
        /* os locais vêm todos declarados no topo, pela ordem do índice: é assim que a volta
         * lhes devolve o mesmo número, e o número é o que o wasm guarda */
        for(int k = 0; k < nl; k++)
            fprintf(o, "    %s v%d;\n", NOME_T[tl[k]], ASS[i].npar + k);

        TXT_N = 0; TXT[0] = 0;
        int bom = desce_corpo(fim, (int)i);
        if(!bom){ fprintf(stderr, "traduz: não sei desfazer o corpo de %s\n", NOMES[i]); return 0; }
        fputs(TXT, o);
        fprintf(o, "}\n\n");
        MP = fim;
    }
    reverte_cauda(o);      /* a estrela reverte: os # e comentários voltam ao C, e o round-trip fecha */
    return 1;
}

/* ── O #define, QUE É TEXTUAL E É ONDE A LINGUAGEM O PÕE ─────────────────────────────
 *
 * Um `#define EIXO_ESCALA (+1)` não é uma variável: é o texto a dizer-se de outra maneira
 * antes de alguém o ler. Por isso troca-se aqui, no texto, e não no avaliador — e por isso o
 * que sobe são os mesmos bytes que subiriam se estivesse escrito à mão.
 *
 * Só as de nome, que são as que valem: um `#define f(x)` é outra coisa e não se finge que sobe.
 * E NÃO SE TROCA DENTRO DE ASPAS nem de plicas: lá dentro o nome é conteúdo, não nome. */
#define MAC_MAX 512
#define MAC_N      DISCO_FIXO2(char, 64, 45)
#define MAC_V      DISCO_FIXO2(char, 256, 46)

static int nome_char(int c){ return isalnum((unsigned char)c) || c == '_'; }

static void colhe_macros(void){
    long i = 0;
    while(SRC[i]){
        if(SRC[i] == '#' && (i == 0 || SRC[i-1] == '\n')){
            long j = i + 1;
            while(SRC[j] == ' ' || SRC[j] == '\t') j++;
            if(!strncmp(SRC + j, "define", 6) && !nome_char(SRC[j+6])){
                j += 6;
                while(SRC[j] == ' ' || SRC[j] == '\t') j++;
                long n0 = j;
                while(nome_char(SRC[j])) j++;
                long nn = j - n0;
                int tem_par = (SRC[j] == '(');       /* de função: essa não sobe */
                while(SRC[j] == ' ' || SRC[j] == '\t') j++;
                long v0 = j;
                while(SRC[j] && SRC[j] != '\n'){
                    /* O COMENTÁRIO NÃO É VALOR: o pré-processador do C tira-o antes de
                     * expandir, e aqui também. Um comentário de bloco que continuava na
                     * linha seguinte entrava no valor SEM fecho, e cada uso da macro
                     * injetava um comentário aberto no meio do código: o lexer engolia
                     * chavetas e o varrimento de morada transbordava para as vizinhas. */
                    if(SRC[j] == '/' && (SRC[j+1] == '*' || SRC[j+1] == '/')) break;
                    if(SRC[j] == '\\' && SRC[j+1] == '\n') j += 2; else j++;
                }
                if(!tem_par && nn > 0 && nn < 63 && j - v0 < 250 && N_MAC < MAC_MAX){
                    memcpy(MAC_N[N_MAC], SRC + n0, (size_t)nn); MAC_N[N_MAC][nn] = 0;
                    long vn = j - v0;
                    memcpy(MAC_V[N_MAC], SRC + v0, (size_t)vn); MAC_V[N_MAC][vn] = 0;
                    while(vn > 0 && (MAC_V[N_MAC][vn-1] == ' ' || MAC_V[N_MAC][vn-1] == '\r'))
                        MAC_V[N_MAC][--vn] = 0;
                    if(vn > 0) N_MAC++;
                }
            }
            while(SRC[i] && SRC[i] != '\n') i++;
            continue;
        }
        i++;
    }
}

/* devolve 1 se trocou alguma coisa */
static int troca_macros(void){
    long o = 0, i = 0; int trocou = 0;
    while(SRC[i]){
        if(SRC[i] == '"' || SRC[i] == '\''){        /* conteúdo: passa inteiro */
            int fim = SRC[i];
            SAIDA_MAC[o++] = SRC[i++];
            while(SRC[i] && SRC[i] != fim){
                if(SRC[i] == '\\'){ SAIDA_MAC[o++] = SRC[i++]; if(SRC[i]) SAIDA_MAC[o++] = SRC[i++]; }
                else SAIDA_MAC[o++] = SRC[i++];
            }
            if(SRC[i]) SAIDA_MAC[o++] = SRC[i++];
            continue;
        }
        if(SRC[i] == '/' && SRC[i+1] == '*'){
            SAIDA_MAC[o++] = SRC[i++]; SAIDA_MAC[o++] = SRC[i++];
            while(SRC[i] && !(SRC[i] == '*' && SRC[i+1] == '/')) SAIDA_MAC[o++] = SRC[i++];
            continue;
        }
        if(SRC[i] == '#' && (i == 0 || SRC[i-1] == '\n')){
            while(SRC[i] && SRC[i] != '\n') SAIDA_MAC[o++] = SRC[i++];
            continue;
        }
        if(nome_char(SRC[i]) && (i == 0 || !nome_char(SRC[i-1]))){
            long j = i; while(nome_char(SRC[j])) j++;
            long n = j - i;
            int achou = -1;
            for(int k = 0; k < N_MAC; k++)
                if((long)strlen(MAC_N[k]) == n && !memcmp(MAC_N[k], SRC + i, (size_t)n)){ achou = k; break; }
            if(achou >= 0){
                long v = (long)strlen(MAC_V[achou]);
                if(o + v + 8 < (1<<22)){
                    memcpy(SAIDA_MAC + o, MAC_V[achou], (size_t)v); o += v;
                    i = j; trocou = 1; continue;
                }
            }
            while(i < j) SAIDA_MAC[o++] = SRC[i++];
            continue;
        }
        SAIDA_MAC[o++] = SRC[i++];
    }
    SAIDA_MAC[o] = 0;
    /* a base tem tecto fixo: não há realloc a pedir em execução, há o tecto a valer */
    if(trocou){
        if(o + 1 > CAP_SRC){ fprintf(stderr, "traduz: macros estouram a base\n"); exit(2); }
        memcpy(SRC, SAIDA_MAC, (size_t)o + 1);
    }
    return trocou;
}

/* AS LAJES. O padrão está escrito no catálogo, §A máquina sem memória: o ficheiro É o
 * vector, e o endereço é uma CONSTANTE do programa — um ponteiro global seriam 8 bytes, e
 * 8 não é 0. Prende-se uma vez, no princípio, e o `.bss` fica vazio. */
/* E AS LAJES NASCEM A ZERO. O `.bss` nascia; um ficheiro não — ele guarda o que a corrida
 * anterior lá deixou, e a segunda tradução do mesmo fonte saía diferente da primeira. Zerar
 * é o preço de a memória estar no disco, e paga-se uma vez no arranque. */
static void prende_tudo(void){
    disco_prende(DISCO_BASE(30), "dados/tz_lit.bin", (size_t)(8192), sizeof(unsigned char));
    disco_prende(DISCO_BASE(31), "dados/tz_img.bin", (size_t)(IMG_MAX), sizeof(unsigned char));
    disco_prende(DISCO_BASE(32), "dados/tz_cod_b.bin", (size_t)(CAP_COD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(33), "dados/tz_mod_b.bin", (size_t)(CAP_MOD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(34), "dados/tz_sec_b.bin", (size_t)(CAP_MOD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(35), "dados/tz_corpos_b.bin", (size_t)(CAP_MOD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(62), "dados/tz_reverso.bin", (size_t)(CAP_MOD), sizeof(char));   /* a cauda reversível */
    disco_prende(DISCO_BASE(36), "dados/tz_txt.bin", (size_t)(CAP_TXT), sizeof(char));
    disco_prende(DISCO_BASE(37), "dados/tz_pedaco.bin", (size_t)(CAP_COD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(38), "dados/tz_tmp_move.bin", (size_t)(CAP_COD), sizeof(unsigned char));
    disco_prende(DISCO_BASE(39), "dados/tz_saco.bin", (size_t)((1<<18)), sizeof(unsigned char));
    disco_prende(DISCO_BASE(40), "dados/tz_saida_mac.bin", (size_t)((1<<22)), sizeof(char));
    disco_prende(DISCO_BASE(41), "dados/tz_funs.bin", (size_t)(MAX_FUN), sizeof(Fun));
    disco_prende(DISCO_BASE(42), "dados/tz_locs.bin", (size_t)(MAX_LOC), sizeof(Loc));
    disco_prende(DISCO_BASE(43), "dados/tz_slots.bin", (size_t)(256), sizeof(Slot));
    disco_prende(DISCO_BASE(44), "dados/tz_estr.bin", (size_t)(64), sizeof(Estrutura));
    disco_prende(DISCO_BASE(45), "dados/tz_mac_n.bin", (size_t)(MAC_MAX*64), sizeof(char));
    disco_prende(DISCO_BASE(46), "dados/tz_mac_v.bin", (size_t)(MAC_MAX*256), sizeof(char));
    disco_prende(DISCO_BASE(47), "dados/tz_exportada.bin", (size_t)(MAX_FUN), sizeof(unsigned char));
    disco_prende(DISCO_BASE(48), "dados/tz_fpt.bin", (size_t)64, sizeof(Perfil));
    disco_prende(DISCO_BASE(49), "dados/tz_end_tomado.bin", (size_t)(MAX_END*64), sizeof(char));
    disco_prende(DISCO_BASE(50), "dados/tz_corpo_off.bin", (size_t)MAX_FUN, sizeof(long));
    disco_prende(DISCO_BASE(51), "dados/tz_corpo_tam.bin", (size_t)MAX_FUN, sizeof(long));
    disco_prende(DISCO_BASE(52), "dados/tz_pilha.bin", (size_t)(64*512), sizeof(char));
    disco_prende(DISCO_BASE(53), "dados/tz_qs.bin", (size_t)MAXQ, sizeof(Quadro));
    disco_prende(DISCO_BASE(54), "dados/tz_lab.bin", (size_t)128, sizeof(Rotulo));
    disco_prende(DISCO_BASE(55), "dados/tz_lacos.bin", (size_t)32, sizeof(Laco));
    disco_prende(DISCO_BASE(56), "dados/tz_ass.bin", (size_t)MAX_FUN, sizeof(Assin));
    disco_prende(DISCO_BASE(57), "dados/tz_nomes.bin", (size_t)(MAX_FUN*64), sizeof(char));
    disco_prende(DISCO_BASE(58), "dados/tz_tok.bin", (size_t)1, sizeof(Tok));
    disco_prende(DISCO_BASE(59), "dados/tz_alvo.bin", (size_t)64, sizeof(unsigned char));
    disco_prende(DISCO_BASE(60), "dados/tz_src.bin", (size_t)CAP_SRC, sizeof(char));
    disco_prende(DISCO_BASE(61), "dados/tz_reg.bin", (size_t)1, sizeof(Reg));

    /* O ZERO E AS SEMENTES. A .bss zerava antes do main de graça; o disco persiste, então
     * zera-se aqui — e o que nascia com valor semeia-se a seguir, no mesmo sítio. O SRC não
     * se zera: o terminador escreve-se ao ler, e o que fica para lá dele nunca é lido. */
    memset(RG, 0, sizeof(Reg));
    memset(FPT, 0, 64 * sizeof(Perfil));
    memset(END_TOMADO, 0, (size_t)(MAX_END*64));
    memset(CORPO_OFF, 0, MAX_FUN * sizeof(long));
    memset(CORPO_TAM, 0, MAX_FUN * sizeof(long));
    memset(PILHA, 0, (size_t)(64*512));
    memset(QS, 0, (size_t)MAXQ * sizeof(Quadro));
    memset(LAB, 0, (size_t)128 * sizeof(Rotulo));
    memset(LACOS, 0, (size_t)32 * sizeof(Laco));
    memset(ASS, 0, (size_t)MAX_FUN * sizeof(Assin));
    memset(NOMES, 0, (size_t)(MAX_FUN*64));
    memset(&T, 0, sizeof(Tok));
    LINHA = 1;
    DISCO = NULO;
    IMG_INI = -1; IMG_FIM = -1;
    SP_SLOT = -1; TMP_RET = -1;
    COD.b = COD_B;       COD.n = 0;    COD.cap = CAP_COD;
    MOD.b = MOD_B;       MOD.n = 0;    MOD.cap = CAP_MOD;
    SEC.b = SEC_B;       SEC.n = 0;    SEC.cap = CAP_MOD;
    CORPOS.b = CORPOS_B; CORPOS.n = 0; CORPOS.cap = CAP_MOD;
    memset(LIT, 0, (size_t)(8192) * sizeof(unsigned char));
    memset(IMG, 0, (size_t)(IMG_MAX) * sizeof(unsigned char));
    memset(COD_B, 0, (size_t)(CAP_COD) * sizeof(unsigned char));
    memset(MOD_B, 0, (size_t)(CAP_MOD) * sizeof(unsigned char));
    memset(SEC_B, 0, (size_t)(CAP_MOD) * sizeof(unsigned char));
    memset(CORPOS_B, 0, (size_t)(CAP_MOD) * sizeof(unsigned char));
    memset(TXT, 0, (size_t)(CAP_TXT) * sizeof(char));
    memset(PEDACO, 0, (size_t)(CAP_COD) * sizeof(unsigned char));
    memset(TMP_MOVE, 0, (size_t)(CAP_COD) * sizeof(unsigned char));
    memset(SACO, 0, (size_t)((1<<18)) * sizeof(unsigned char));
    memset(SAIDA_MAC, 0, (size_t)((1<<22)) * sizeof(char));
    memset(FUNS, 0, (size_t)(MAX_FUN) * sizeof(Fun));
    memset(LOCS, 0, (size_t)(MAX_LOC) * sizeof(Loc));
    memset(SLOTS, 0, (size_t)(256) * sizeof(Slot));
    memset(ESTR, 0, (size_t)(64) * sizeof(Estrutura));
    memset(MAC_N, 0, (size_t)(MAC_MAX*64) * sizeof(char));
    memset(MAC_V, 0, (size_t)(MAC_MAX*256) * sizeof(char));
}

int main(int argc, char **argv){
    prende_tudo();
    const char *ent = 0, *sai = "saida.wasm";
    for(int i = 1; i < argc; i++){
        if(!strcmp(argv[i], "-o") && i + 1 < argc) sai = argv[++i];
        else ent = argv[i];
    }
    if(!ent){ fprintf(stderr, "uso: traduz entrada.c -o saida.wasm   (ou entrada.wasm -o volta.c)\n"); return 2; }

    FILE *f = fopen(ent, "rb");
    if(!f){ fprintf(stderr, "traduz: não abre %s\n", ent); return 2; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    /* o fonte entra direto na base — era um malloc, e heap anónimo é RAM como .bss é */
    if(n + 1 > CAP_SRC){ fprintf(stderr, "traduz: fonte maior que a base (%d)\n", CAP_SRC); return 2; }
    if(fread(SRC, 1, (size_t)n, f) != (size_t)n){ fprintf(stderr,"traduz: leitura\n"); return 2; }
    SRC[n] = 0; fclose(f);
    captura_reverso(SRC);   /* guarda os # e comentários do ORIGINAL, antes de a expansão reescrever o SRC */

    /* O SENTIDO SAI DO PRÓPRIO OBJECTO, não de uma opção: um módulo diz-se pela marca. É a
     * mesma interface do corpo_analitico — `MOVE(slot, sentido)`: −1 emite, +1 absorve. */
    if(n >= 4 && !memcmp(SRC, "\0asm", 4)){
        FILE *o = fopen(sai, "wb");
        if(!o){ fprintf(stderr, "traduz: não escreve %s\n", sai); return 2; }
        int bom = desce_modulo((const unsigned char *)SRC, n, o);
        fclose(o);
        if(!bom){ fprintf(stderr, "traduz: a descida não fechou\n"); return 3; }
        printf("traduz: %s -> %s   +1 absorve: %d funções\n", ent, sai, N_ASS);
        return 0;
    }

    colhe_macros();
    for(int i = 0; i < 8 && troca_macros(); i++) ;   /* encaixadas, e com fundo */

    /* o import só existe se o fonte o chama — módulos sem fopen-miss ficam sem secção 2,
     * e a volta (traduz_volta) não parte. */
    N_IMP = 0;
    {
        long i = 0;
        while(SRC[i]){
            if(SRC[i] == '_' && !strncmp(SRC + i, "__fich_miss", 11)){
                char c = SRC[i + 11];
                if(c == '(' || c == ' ' || c == '\t' || c == '\n' || c == '\r'){ N_IMP = 1; break; }
            }
            i++;
        }
    }

    colhe_assinaturas();

    /* a segunda volta: os corpos */
    POS = 0; LINHA = 1; avanca();
    int visto = 0;
    while(T.k != TK_FIM){
        if(e_id("static")){ avanca(); continue; }
        /* a definição de uma estrutura já foi colhida na primeira volta: aqui salta-se por
         * cima dela inteira. Sem isto o `struct {` anónimo de um typedef caía no leitor de
         * tipos, que — bem — não conhece uma estrutura sem nome. */
        if(e_id("typedef") || e_id("struct")){
            long g_pos = POS, g_lin = LINHA; Tok g_tok = T;
            if(e_id("typedef")) avanca();
            if(e_id("struct")) avanca();
            if(T.k == TK_ID && !e_pun("{")) avanca();
            if(e_pun("{")){
                int d = 0;
                do { if(e_pun("{")) d++; if(e_pun("}")) d--; avanca(); }
                while(d > 0 && T.k != TK_FIM);
                while(!e_pun(";") && T.k != TK_FIM) avanca();
                avanca();
                continue;
            }
            POS = g_pos; LINHA = g_lin; T = g_tok;
        }
        if(!e_tipo()){ avanca(); continue; }
        /* O TIPO LÊ-SE COM A MESMA RÉGUA DAS OUTRAS PASSAGENS. Aqui estava escrito à mão e
         * parava no `*` — logo `char *f(void)` era vista na primeira volta e ficava SEM CORPO
         * na segunda, e o módulo saía com uma função vazia que o motor recusa. Duas réguas
         * para o mesmo tipo, mais uma vez. */
        int ret = le_tipo(); (void)ret;
        if(T.k != TK_ID){ continue; }
        char nome[64]; strcpy(nome, T.s); avanca();
        if(!e_pun("(")){ while(!e_pun(";") && T.k != TK_FIM) avanca(); avanca(); continue; }

        int fi = acha_fun(nome);
        if(fi < 0) erro("função sem assinatura");
        FUN_ACT = fi;
        FUNS[fi].nloc = 0;
        NLOC = 0; NLACO = 0; PROF = 0; COD.n = 0;

        avanca();
        while(!e_pun(")") && T.k != TK_FIM){          /* os parâmetros viram locais 0..n-1 */
            /* `void` só quer dizer «sem parâmetros» se vier o fecho a seguir. Estava a engolir o
                 * `void` de `void *ctx` e a tropeçar no `*` — o vazio e o endereço de coisa
                 * nenhuma são a mesma palavra e não são a mesma coisa. */
                if(e_id("void")){
                    long g = POS; Tok gt = T; long gl = LINHA;
                    avanca();
                    if(e_pun(")")) continue;
                    POS = g; T = gt; LINHA = gl;
                }
            /* O NOME VEM DO DECLARADOR, e não de espreitar o token seguinte: ele já o
             * consumiu, e forjar um token para o reler fazia o `avanca()` comer a vírgula.
             * Quem lê o declarador sabe o nome; quem o pede não tem de o adivinhar. */
            char pn[64];
            int pt = le_declarador(le_tipo(), pn);
            if(pn[0]){
                memset(&LOCS[NLOC], 0, sizeof LOCS[0]);
                strcpy(LOCS[NLOC].nome, pn);
                LOCS[NLOC].tipo = pt;
                LOCS[NLOC].idx  = NLOC;
                NLOC++;
            }
            if(!aceita(",")) break;
        }
        come(")");
        if(e_pun(";")){ avanca(); continue; }          /* só a declaração */

        QUADRO = mede_quadro(); TMP_RET = -1;
        QUADRO_FITA = 0; QUADRO_USADO = 0;
        N_FITA = PRECISA_FITA; FITA_SITIO = 0;
        if(N_FITA > 0){
            QUADRO_USADO = (long)N_FITA * FITA_SLOTS * 8;
            QUADRO += (long)N_FITA * FITA_SLOTS * 8;
        }
        if(QUADRO > 0){
            garante_sp();
            if(FUNS[fi].ret != TVOID){
                TMP_RET = FUNS[fi].npar + FUNS[fi].nloc;
                FUNS[fi].loc[FUNS[fi].nloc++] = FUNS[fi].ret;
            }
            move_quadro(-1);
        }
        bloco();
        if(QUADRO > 0) move_quadro(+1);
        /* o wasm exige um valor no fim se a função devolve algo — o C deixa-o implícito */
        if(FUNS[fi].ret != TVOID){
            if(FUNS[fi].ret == TF64){ uint64_t z = F64_0; bput(&COD, 0x44); bmany(&COD, &z, 8); }
            else { bput(&COD, FUNS[fi].ret == TI64 ? 0x42 : 0x41); bs(&COD, 0); }
        }
        bput(&COD, 0x0B);                              /* end */

        /* o corpo: locais + código, com o tamanho à frente — como o chessb.c descreve */
        Buf b; unsigned char tmp[CAP_COD]; b.b = tmp; b.n = 0; b.cap = CAP_COD;
        bu(&b, (unsigned long)FUNS[fi].nloc);
        for(int i = 0; i < FUNS[fi].nloc; i++){ bu(&b, 1); bput(&b, val_t(FUNS[fi].loc[i])); }
        bmany(&b, COD.b, COD.n);
        CORPO_OFF[fi] = CORPOS.n; CORPO_TAM[fi] = b.n;
        bmany(&CORPOS, b.b, b.n);
        visto++;
    }
    if(!visto){ fprintf(stderr, "traduz: nenhuma função com corpo\n"); return 2; }

    /* a fita onde os quadros abrem: fica no fim do disco e o ponteiro desce nela. Só existe
     * se alguma função pediu quadro — quem não tem vector local não paga fita nenhuma. */
    if(SP_SLOT >= 0){
        DISCO = (DISCO + 7) / 8 * 8;
        DISCO += PILHA_BYTES;
        unsigned char v[4];
        v[0] = (unsigned char)(DISCO & 255);      v[1] = (unsigned char)((DISCO >> 8) & 255);
        v[2] = (unsigned char)((DISCO >> 16) & 255); v[3] = (unsigned char)((DISCO >> 24) & 255);
        escreve_imagem(SLOTS[SP_SLOT].endereco, v, 4);
    }

    /* ── as secções ── */
    bmany(&MOD, "\0asm", 4);
    { unsigned char v[4] = { 1, 0, 0, 0 }; bmany(&MOD, v, 4); }

    bu(&SEC, (unsigned long)(NFUN + NFPT + N_IMP));   /* 1: tipo — funções, saltos, import */
    for(int i = 0; i < NFUN; i++){
        bput(&SEC, 0x60);
        bu(&SEC, (unsigned long)FUNS[i].npar);
        for(int j = 0; j < FUNS[i].npar; j++) bput(&SEC, val_t(FUNS[i].par[j]));
        if(FUNS[i].ret == TVOID) bu(&SEC, 0);
        else { bu(&SEC, 1); bput(&SEC, val_t(FUNS[i].ret)); }
    }
    for(int i = 0; i < NFPT; i++){
        bput(&SEC, 0x60);
        bu(&SEC, (unsigned long)FPT[i].npar);
        for(int j = 0; j < FPT[i].npar; j++) bput(&SEC, val_t(FPT[i].par[j]));
        if(FPT[i].ret == TVOID) bu(&SEC, 0);
        else { bu(&SEC, 1); bput(&SEC, val_t(FPT[i].ret)); }
    }
    if(N_IMP > 0){                                     /* __fich_miss(i32) -> i32 */
        bput(&SEC, 0x60);
        bu(&SEC, 1); bput(&SEC, 0x7F);
        bu(&SEC, 1); bput(&SEC, 0x7F);
    }
    seccao(1, &SEC);

    if(N_IMP > 0){                                     /* 2: import env.__fich_miss */
        bu(&SEC, (unsigned long)N_IMP);
        {
            const char *mod = "env", *nm = "__fich_miss";
            bu(&SEC, (unsigned long)strlen(mod)); bmany(&SEC, mod, (long)strlen(mod));
            bu(&SEC, (unsigned long)strlen(nm));  bmany(&SEC, nm,  (long)strlen(nm));
            bput(&SEC, 0x00);                          /* func */
            bu(&SEC, (unsigned long)(NFUN + NFPT));    /* tipo anexado */
        }
        seccao(2, &SEC);
    }

    bu(&SEC, (unsigned long)NFUN);                     /* 3: função */
    for(int i = 0; i < NFUN; i++) bu(&SEC, (unsigned long)i);
    seccao(3, &SEC);

    if(NFPT > 0){                                      /* 4: a tabela dos saltos */
        bu(&SEC, 1);
        bput(&SEC, 0x70);                              /* funcref */
        bput(&SEC, 0x01);
        bu(&SEC, (unsigned long)NFUN);
        bu(&SEC, (unsigned long)NFUN);
        seccao(4, &SEC);
    }

    /* 5: a memória. O disco tem o tamanho que as declarações somam, e a página do wasm são
     * 65536 bytes — não há aqui nada a pedir nem a devolver em tempo de execução. */
    long paginas = DISCO > 0 ? (DISCO + 65535) / 65536 : 0;
    if(paginas > 0){
        bu(&SEC, 1);
        /* O MÍNIMO É O QUE AS DECLARAÇÕES SOMAM; o MÁXIMO é o tecto até onde o disco pode
         * ESTENDER-SE por `__disco_cresce` — contado, não reservado. O motor pagina o que
         * se toca (lazy), então declarar o tecto não custa RAM: um disco que nunca se escreve
         * não ocupa nada, como o vector grande nunca escrito do corpo_analitico. Reservar o
         * tamanho fixo é que era o infinito — um `char MONTE[12M]` forçava a página a existir.
         * Aqui o disco começa no que basta e cresce pelo que se escreve. */
        bput(&SEC, 0x01);
        bu(&SEC, (unsigned long)paginas);              /* min: as declarações */
        /* 4096 páginas = 256 MB: o slot do PDF do tradutor é 128 MB, e o corpo
         * (fontes + .tex + saída) pede folga. O tecto antigo (512 = 32 MB) fazia
         * memory.grow falhar a meio da composição no navegador. */
        long tecto = paginas < 4096 ? 4096 : paginas + 512;
        bu(&SEC, (unsigned long)tecto);
        seccao(5, &SEC);
    }

    /* SÓ SAI O QUE NÃO É `static`. Uma função interna existe e corre; o que ela não faz é
     * aparecer na porta. Eu exportava tudo — e então, para o módulo mostrar uma operação só,
     * apaguei os auxiliares e colei os corpos deles lá dentro: a soma passou a estar escrita
     * três vezes. Não tirei ruído, multipliquei-o. O `static` já dizia o que fazer. */
    int n_sai = 0;
    for(int i = 0; i < NFUN; i++) if(!FUNS[i].interna) n_sai++;
    bu(&SEC, (unsigned long)(n_sai + (paginas > 0 && DISCO_PUBLICO ? 1 : 0)));   /* 7: exportação */
    for(int i = 0; i < NFUN; i++){
        if(FUNS[i].interna) continue;
        long L = (long)strlen(FUNS[i].nome);
        bu(&SEC, (unsigned long)L);
        bmany(&SEC, FUNS[i].nome, L);
        bput(&SEC, 0x00);
        bu(&SEC, (unsigned long)WASM_FUN(i));
    }
    if(paginas > 0 && DISCO_PUBLICO){   /* o disco só sai se um global público o pedir */
        /* o comprimento NÃO se escreve à mão: escrevi `6` para sete letras e o disco saiu
         * exportado como «MEMORI». E a volta não o apanhou — a descida não lê o nome de um
         * export que não é função, e é um sítio onde a involução não chega. */
        const char *nm = "DISCO";
        bu(&SEC, (unsigned long)strlen(nm)); bmany(&SEC, nm, (long)strlen(nm));
        bput(&SEC, 0x02); bu(&SEC, 0);
    }
    seccao(7, &SEC);

    if(NFPT > 0){                                      /* 9: quem está em cada entrada */
        bu(&SEC, 1);
        bu(&SEC, 0);
        bput(&SEC, 0x41); bs(&SEC, 0); bput(&SEC, 0x0B);
        bu(&SEC, (unsigned long)NFUN);
        for(int i = 0; i < NFUN; i++) bu(&SEC, (unsigned long)WASM_FUN(i));
        seccao(9, &SEC);
    }

    bu(&SEC, (unsigned long)NFUN);                     /* 10: código */
    for(int i = 0; i < NFUN; i++){
        bu(&SEC, (unsigned long)CORPO_TAM[i]);
        bmany(&SEC, CORPOS.b + CORPO_OFF[i], CORPO_TAM[i]);
    }
    seccao(10, &SEC);

    if(IMG_INI >= 0 && IMG_FIM > IMG_INI){        /* 11: os dados — um segmento só */
        bu(&SEC, 1);
        bu(&SEC, 0);                              /* o segmento activo, na memória 0 */
        bput(&SEC, 0x41); bs(&SEC, IMG_INI); bput(&SEC, 0x0B);   /* onde começa */
        bu(&SEC, (unsigned long)(IMG_FIM - IMG_INI));
        bmany(&SEC, IMG + IMG_INI, IMG_FIM - IMG_INI);
        seccao(11, &SEC);
    }

    /* A CAUDA REVERSÍVEL: o que o lexer descartou volta ao módulo em secções custom (o runtime
     * ignora-as; a volta lê-as). A config (#) no CENTRO --- logo a seguir ao código ---, os
     * comentários no fim. É a metade que a estrela guarda para não apagar. */
    if(REV_CFG > 0)      seccao_custom("gk.config",  (unsigned char*)REVERSO_B,           REV_CFG);
    if(REV_N > REV_CFG)  seccao_custom("gk.reverso", (unsigned char*)REVERSO_B + REV_CFG, REV_N - REV_CFG);

    FILE *o = fopen(sai, "wb");
    if(!o){ fprintf(stderr, "traduz: não escreve %s\n", sai); return 2; }
    fwrite(MOD.b, 1, (size_t)MOD.n, o);
    fclose(o);
    printf("traduz: %s -> %s   -1 emite: %d funções, %ld bytes\n", ent, sai, NFUN, MOD.n);
    return 0;
}
