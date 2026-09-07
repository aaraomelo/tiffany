/*
 * micro.c - Microprocessador Fractal
 *          modelo de referencia da arquitetura / ISA simulator
 *
 * Tese do projeto (arquitetura.tex sec:isa):
 *   MOVE e a operacao primitiva de TRANSFERENCIA DE ESTADO:
 *     MOVE(destino, sentido)
 *       sentido +1  = leitura  (LOAD)
 *       sentido -1  = escrita  (STORE)
 *       sentido  0  = PC <- destino (JMP)
 *
 *   Computacao = MOVE* + ALU:
 *     LOAD  a     = MOVE(a, +1)
 *     STORE r     = MOVE(r, -1)
 *     JMP   d     = MOVE(d, 0)
 *     instrucao 80-bit =
 *       MOVE(a,+1) -> MOVE(b,+1) -> ALU(OP) -> MOVE(r,-1)
 *
 *   Controles separados:
 *     HALT = controle da maquina de referencia (nao e MOVE)
 *     ALU  = transformacao funcional (api.h: ADD/MUL/...)
 *
 * Alinhamento:
 *   api.h          — ISA canonica (ADD/MUL/SUB/portas, Mat, Univ)
 *   so_cristal.c   — realizacao analogica da triade
 *   micro.c        — arquitetura completa sintetizada a partir de MOVE
 *
 * Contratos formais:
 *   DIV : {0..2^64-1}^2 -> {0..2^64-1}
 *         a = q*b + r,  0 <= r < b  (b>0);  DIV(a,0)=(0,a)
 *   Mem : enderecos logicos de 24 bits; RAM de trabalho 2^12 palavras;
 *         mapeamento A_24 -> A_12 por modulo (contrato do modelo).
 *   ISA inicial = fluxo linear (PC += 10), salvo JMP via MOVE(pc, 0).
 *   NOT/MOV : addr_b e don't-care.
 *
 * Compila:
 *   cc -O2 -std=c11 -Wall -Wextra -I. -o micro micro.c -lm
 *   ./micro
 *
 * Autor: Aarao Melo Lopes | Microprocessador Fractal
 */

#include "micro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdint.h>

/* aliases locais (micro.h usa MICRO_*) */
#define MEM_WORDS  MICRO_MEM_WORDS
#define ADDR_MASK  MICRO_ADDR_MASK

/* ===========================================================================
 * 0. MOVE — operacao primitiva de transferencia de estado
 * =========================================================================== */

void maq_clear(Maquina *M) {
    memset(M, 0, sizeof(*M));
}

/* reducao A_24 -> A_12 (contrato explicito) */
u64 addr_fisico(u64 addr24) {
    return (addr24 & ADDR_MASK) % MEM_WORDS;
}

/*
 * MOVE(destino, sentido)
 *   +1 : canal <- mem[destino]          (LOAD)
 *   -1 : mem[destino] <- canal          (STORE)
 *    0 : pc <- destino                  (JMP)
 * Retorna o valor no canal apos a operacao (util para LOAD).
 */
u64 MOVE(Maquina *M, u64 destino, int sentido) {
    if (sentido > 0) {
        M->canal = M->word[addr_fisico(destino)];
    } else if (sentido < 0) {
        M->word[addr_fisico(destino)] = M->canal;
    } else {
        M->pc = destino;   /* salto */
    }
    return M->canal;
}

/* sintese: LOAD / STORE / JMP sao so MOVE */
u64 LOAD(Maquina *M, u64 addr) {
    return MOVE(M, addr, +1);
}

void STORE(Maquina *M, u64 addr) {
    MOVE(M, addr, -1);
}

void JMP(Maquina *M, u64 dest_pc) {
    MOVE(M, dest_pc, 0);
}

/* STORE com valor explicito (carrega o canal e MOVE -1) */
void STORE_VAL(Maquina *M, u64 addr, u64 val) {
    M->canal = val;
    STORE(M, addr);
}

/* ===========================================================================
 * 1. DIV — contrato formal
 *
 *   DIV : {0,...,2^64-1}^2 -> {0,...,2^64-1}
 *   a = q*b + r,  0 <= r < b   (b > 0)
 *   DIV(a, 0) = (q=0, r=a)
 *   Intermediario em __uint128_t (bit extra no deslocamento).
 * =========================================================================== */

u64 DIV(u64 num, u64 den, u64 *rem) {
    if (den == 0) {
        if (rem) *rem = num;
        return 0;
    }
    u64 q = 0;
    unsigned __int128 r = 0;
    for (int i = 63; i >= 0; i--) {
        r = (r << 1) | ((num >> i) & 1ULL);
        if (r >= den) {
            r -= den;
            q |= (1ULL << i);
        }
    }
    if (rem) *rem = (u64)r;
    return q;
}

u64 DIVQ(u64 a, u64 b) { return DIV(a, b, NULL); }

/* ===========================================================================
 * 2. Relogio / mapa de escala
 * =========================================================================== */

static int ordem_exata(const Univ *U, u64 w) {
    if (U->n == 0 || w == 0) return 0;
    if (u_pot(U, w, U->n) != 1) return 0;
    if (U->n > 1 && u_pot(U, w, U->n / 2) == 1) return 0;
    return 1;
}

static int gerador_valido_torre(u64 p, u64 g, u64 n_max) {
    if ((p - 1) % n_max != 0) return 0;
    Univ U = universal_init(p, g, n_max);
    return ordem_exata(&U, U.w);
}

/*
 * Pre-condicao: p PRIMO.
 * Certifica que g tem ordem p-1 em F_p^* (logo e gerador).
 * Especializado para p=40961 (p-1 = 2^13 * 5); nao testa primalidade.
 */
static int gerador_completo(u64 p, u64 g) {
    /* p-1 = 40960 = 2^13 * 5  para p=40961 */
    u64 pm1 = p - 1;
    Univ U = { .p = p, .g = g, .n = 1, .w = 1, .ninv = 1 };
    if (u_pot(&U, g, pm1) != 1) return 0;           /* g^{p-1}=1 */
    if (u_pot(&U, g, pm1 / 2) == 1) return 0;        /* nao divide por 2 */
    if (u_pot(&U, g, pm1 / 5) == 1) return 0;        /* nao divide por 5 */
    return 1;
}

static Univ universal_init_exato(u64 p, u64 g, u64 n) {
    Univ U = universal_init(p, g, n);
    if (n == 0 || (p - 1) % n != 0) { U.w = 1; return U; }
    if (ordem_exata(&U, U.w)) return U;
    for (u64 k = 1; k < n; k++) {
        u64 cand = u_pot(&U, g, ((p - 1) / n) * k);
        if (ordem_exata(&U, cand)) { U.w = cand; return U; }
    }
    U.w = 1;
    return U;
}

/* ===========================================================================
 * 3. I/O — CF sobre Q_{\ge 0}
 * =========================================================================== */

static int adc(i64 p, i64 q, i64 *a, int maxn) {
    int n = 0;
    if (p < 0 || q <= 0) return -1;
    while (q != 0 && n < maxn) {
        i64 ak = 0, r = p;
        while (r >= q) { r -= q; ak++; }
        a[n++] = ak;
        p = q;
        q = r;
    }
    return n;
}

static void dac(const i64 *a, int n, i64 *p, i64 *q) {
    Mat M = {1, 0, 0, 1};
    for (int k = 0; k < n; k++)
        M = mat_lahire(M, GATO(a[k]));
    *p = M.a;
    *q = M.c;
    if (*q < 0) { *p = -*p; *q = -*q; }
}

/* ===========================================================================
 * 4. Streaming (disco = estado externo; 1 palavra na mao)
 * =========================================================================== */

u64 rotl64(u64 x, unsigned k) {
    k &= 63u;                 /* evita UB: shift >= width */
    if (k == 0) return x;     /* evita x >> 64 */
    return (x << k) | (x >> (64u - k));
}

static void stream(FILE *fi, FILE *fo, int inv, u64 s) {
    int b;
    while ((b = fgetc(fi)) != EOF) {
        u64 k = s & 0xFF;
        int r = (b ^ (int)k) & 0xFF;
        s = rotl64(s, 7) ^ (u64)(inv ? b : r);
        fputc(r, fo);
    }
}

static int stream_roundtrip(size_t nbytes, u64 seed) {
    FILE *fi = tmpfile(), *fo = tmpfile(), *fr = tmpfile();
    if (!fi || !fo || !fr) return -1;
    for (size_t i = 0; i < nbytes; i++)
        fputc((int)((i * 17 + 3) & 0xFF), fi);
    rewind(fi);
    stream(fi, fo, 0, seed);
    rewind(fo);
    stream(fo, fr, 1, seed);
    rewind(fi); rewind(fr);
    int bad = 0, c1, c2;
    while ((c1 = fgetc(fi)) != EOF) {
        c2 = fgetc(fr);
        if (c1 != c2) { bad = 1; break; }
    }
    if (fgetc(fr) != EOF) bad = 1;
    fclose(fi); fclose(fo); fclose(fr);
    return bad;
}

/* ===========================================================================
 * 5. Barramento — arvore de + (composicao de ADD = gato o esquilo)
 * =========================================================================== */

static u64 soma_arvore(const u64 *v, int n) {
    if (n == 1) return v[0];
    int m = n / 2;
    return ADD(soma_arvore(v, m), soma_arvore(v + m, n - m));
}

/* ===========================================================================
 * 6. Instrucao de 80 bits — sintetizada a partir de MOVE
 *
 *   | LOAD a (24) | LOAD b (24) | OP (8) | STORE r (24) |  = 80 bits
 *
 * Semantica (tudo e MOVE + ALU de api.h):
 *   canal <- MOVE(a, +1)
 *   tmp_a = canal
 *   canal <- MOVE(b, +1)
 *   tmp_b = canal
 *   canal <- OP(tmp_a, tmp_b)     [ALU]
 *   MOVE(r, -1)                   [STORE]
 *
 * Opcodes:
 *   0 HALT  1 ADD  2 SUB  3 MUL  4 DIV
 *   5 AND   6 OR   7 XOR  8 NOT  9 MOV
 *   10 JMP  (addr_a = destino PC; addr_b, addr_r don't-care)
 *
 * NOT/MOV: addr_b don't-care.
 * ISA de fluxo linear: PC += 10, salvo JMP.
 * =========================================================================== */

/* Instr80 + opcodes: micro.h */

void encode80(const Instr80 *I, uint8_t out[10]) {
    out[0] = (uint8_t)(I->addr_a      );
    out[1] = (uint8_t)(I->addr_a >>  8);
    out[2] = (uint8_t)(I->addr_a >> 16);
    out[3] = (uint8_t)(I->addr_b      );
    out[4] = (uint8_t)(I->addr_b >>  8);
    out[5] = (uint8_t)(I->addr_b >> 16);
    out[6] = I->op;
    out[7] = (uint8_t)(I->addr_r      );
    out[8] = (uint8_t)(I->addr_r >>  8);
    out[9] = (uint8_t)(I->addr_r >> 16);
}

void decode80(const uint8_t in[10], Instr80 *I) {
    I->addr_a = (uint32_t)in[0] | ((uint32_t)in[1] << 8) | ((uint32_t)in[2] << 16);
    I->addr_b = (uint32_t)in[3] | ((uint32_t)in[4] << 8) | ((uint32_t)in[5] << 16);
    I->op     = in[6];
    I->addr_r = (uint32_t)in[7] | ((uint32_t)in[8] << 8) | ((uint32_t)in[9] << 16);
}

/*
 * exec80 — executa UMA instrucao via MOVE + ALU.
 * Retorna 1 se HALT, 0 caso contrario.
 * *jumped = 1 se IOP_JMP (roda80 nao avanca PC linearmente).
 * Pre-condicao: addr_a, addr_b, addr_r em [0, 2^24-1].
 * NOT/MOV: addr_b nao participa da execucao (nao e carregado).
 */
int exec80(Maquina *M, const Instr80 *I, int *jumped) {
    if (jumped) *jumped = 0;
    if (I->op == IOP_HALT) return 1;          /* controle do simulador */

    if (I->op == IOP_JMP) {
        JMP(M, I->addr_a);                    /* MOVE(pc, 0) */
        if (jumped) *jumped = 1;
        return 0;
    }

    /* LOAD a = MOVE(a, +1) */
    u64 a = LOAD(M, I->addr_a);
    /* LOAD b so se participa do resultado */
    u64 b = 0;
    if (I->op != IOP_NOT && I->op != IOP_MOV)
        b = LOAD(M, I->addr_b);

    u64 r = 0;
    switch (I->op) {
        case IOP_ADD: r = ADD(a, b); break;
        case IOP_SUB: r = SUB(a, b); break;
        case IOP_MUL: r = MUL(a, b); break;
        case IOP_DIV: r = DIVQ(a, b); break;
        case IOP_AND: r = AND(a, b); break;
        case IOP_OR:  r = OR (a, b); break;
        case IOP_XOR: r = XOR(a, b); break;
        case IOP_NOT: r = NOT(a);    break;  /* unario */
        case IOP_MOV: r = a;         break;  /* unario */
        default:                     break;
    }
    /* STORE r = MOVE(r, -1) */
    STORE_VAL(M, I->addr_r, r);
    return 0;
}

/*
 * roda80 — FETCH / DECODE / EXECUTE sobre programa empacotado.
 * Fluxo linear PC += 10, salvo JMP (MOVE pc).
 */
int roda80(Maquina *M, const uint8_t *prog, int nbytes, int *passos) {
    M->pc = 0;
    int steps = 0, halted = 0;
    while (!halted && (int)M->pc + 10 <= nbytes) {
        Instr80 I;
        decode80(prog + M->pc, &I);
        int jumped = 0;
        halted = exec80(M, &I, &jumped);
        steps++;
        if (halted) break;
        /* PC' = destino se JMP; PC+10 caso contrario (inclui JMP 0) */
        if (!jumped)
            M->pc += 10;
    }
    if (passos) *passos = steps;
    return halted;
}

/* ===========================================================================
 * 7. Referencia analogica (libm) — fisica em so_cristal.c
 * =========================================================================== */

static double diodo_log(double I) { return log(I > 0 ? I : 1e-300); }
static double diodo_exp(double V) { return exp(V); }
static double analog_add(double a, double b) { return a + b; }
static double analog_mul(double a, double b) { return diodo_exp(diodo_log(a) + diodo_log(b)); }
static double analog_div(double a, double b) { return diodo_exp(diodo_log(a) - diodo_log(b)); }

#ifndef MICRO_AS_LIB
/* ===========================================================================
 * Testes
 * =========================================================================== */

static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s\n", msg); \
        g_fail++; \
    } \
} while (0)

static void test_move(void) {
    printf("  [MOVE]       ");
    Maquina M;
    maq_clear(&M);
    /* STORE via MOVE(-1) */
    M.canal = 0xDEAD;
    MOVE(&M, 7, -1);
    CHECK(M.word[7] == 0xDEAD, "MOVE store");
    /* LOAD via MOVE(+1) */
    M.canal = 0;
    u64 v = MOVE(&M, 7, +1);
    CHECK(v == 0xDEAD && M.canal == 0xDEAD, "MOVE load");
    /* JMP via MOVE(0) */
    MOVE(&M, 42, 0);
    CHECK(M.pc == 42, "MOVE jmp");
    /* LOAD/STORE wrappers */
    STORE_VAL(&M, 3, 99);
    CHECK(LOAD(&M, 3) == 99, "LOAD/STORE");
    /* mapeamento 24->12 */
    STORE_VAL(&M, MEM_WORDS + 5, 77);
    CHECK(LOAD(&M, 5) == 77, "A24->A12 modulo");
    STORE_VAL(&M, 0xFFFFFF, 55);  /* max 24-bit */
    CHECK(LOAD(&M, 0xFFFFFF % MEM_WORDS) == 55, "addr max 24-bit");
    printf("resid 0\n");
}

static void test_add_ripple(void) {
    printf("  [ADD api.h]  ");
    CHECK(ADD(7, 1) == 8, "7+1");
    int bad = 0;
    for (uint32_t a = 0; a < 256; a++)
        for (uint32_t b = 0; b < 256; b++)
            if (ADD(a, b) != (u64)(a + b)) bad++;
    CHECK(bad == 0, "ADD 8-bit");
    printf("resid 0\n");
}

static void test_mul_div(void) {
    printf("  [MUL/DIV]    ");
    int bad = 0;
    for (u64 a = 1; a <= 200; a++)
        for (u64 b = 1; b <= 200; b++) {
            u64 m = MUL(a, b);
            if (m != a * b) bad++;
            u64 r, q = DIV(m, b, &r);
            if (q != a || r != 0) bad++;
            /* contrato: a = q*b + r, 0 <= r < b */
            DIV(m + 3, b, &r);
            q = DIVQ(m + 3, b);
            if (ADD(MUL(q, b), r) != m + 3 || r >= b) bad++;
        }
    CHECK(bad == 0, "MUL/DIV + contrato a=qb+r");
    u64 r;
    DIV(ULLONG_MAX, 3, &r);
    CHECK(r < 3, "resto < den");
    CHECK(DIVQ(0x8000000000000000ULL, 2) == 0x4000000000000000ULL, "2^63/2");
    CHECK(DIVQ(5, 0) == 0, "DIV(a,0)=(0,a)");
    DIV(5, 0, &r);
    CHECK(r == 5, "resto=a quando den=0");
    printf("resid 0\n");
}

static void test_sub_logic(void) {
    printf("  [SUB/logic]  ");
    CHECK(SUB(10, 3) == 7, "10-3");
    CHECK(AND(0xF0, 0x0F) == 0, "AND");
    CHECK(OR (0xF0, 0x0F) == 0xFF, "OR");
    CHECK(XOR(0xFF, 0x0F) == 0xF0, "XOR");
    CHECK(NOT(0) == ULLONG_MAX, "NOT");
    printf("resid 0\n");
}

static void test_relogio(void) {
    printf("  [relogio]    ");
    const u64 P = 40961, G = 3;
    CHECK(gerador_completo(P, G), "g=3 gerador de F_p^x");
    CHECK(gerador_valido_torre(P, G, 8192), "torre ate 8192");
    int bad = 0;
    for (u64 n = 2; n <= 8192; n *= 2) {
        if ((P - 1) % n != 0) continue;
        Univ U = universal_init_exato(P, G, n);
        if (!ordem_exata(&U, U.w)) { bad++; continue; }
        u64 acc = 1;
        for (u64 t = 0; t < n; t++) acc = u_mul(&U, acc, U.w);
        if (acc != 1) bad++;
    }
    CHECK(bad == 0, "ordem exata em toda torre");
    printf("resid 0\n");
}

static void test_zoom(void) {
    printf("  [zoom w->w2] ");
    const u64 P = 40961, G = 3;
    int bad = 0;
    for (u64 n = 4; n <= 8192; n *= 2) {
        if ((P - 1) % n != 0 || (P - 1) % (n / 2) != 0) continue;
        Univ Un = universal_init_exato(P, G, n);
        Univ Uh = universal_init_exato(P, G, n / 2);
        if (!ordem_exata(&Un, Un.w) || !ordem_exata(&Uh, Uh.w)) { bad++; continue; }
        if (u_mul(&Un, Un.w, Un.w) != Uh.w) bad++;
    }
    CHECK(bad == 0, "w(n)^2 = w(n/2)");
    printf("resid 0\n");
}

static void test_fracao_continua(void) {
    printf("  [I/O CF]     ");
    for (i64 m = -5; m <= 5; m++)
        CHECK(mat_det(GATO(m)) == -1, "det GATO=-1");
    int bad = 0;
    i64 a[64];
    for (i64 p = 1; p <= 80; p++)
        for (i64 q = 1; q <= 80; q++) {
            int n = adc(p, q, a, 64);
            if (n < 0) { bad++; continue; }
            i64 rp, rq;
            dac(a, n, &rp, &rq);
            if (rp * q != rq * p) bad++;
            Mat M = {1, 0, 0, 1};
            for (int k = 0; k < n; k++) M = mat_lahire(M, GATO(a[k]));
            if (mat_det(M) != ((n % 2 == 0) ? 1 : -1)) bad++;
        }
    CHECK(bad == 0, "CF Q>=0 + unimodular");
    CHECK(adc(-1, 5, a, 64) < 0, "rejeita p<0");
    printf("resid 0\n");
}

static void test_barramento(void) {
    printf("  [barramento] ");
    u64 v[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    CHECK(soma_arvore(v, 8) == 36, "arvore=36");
    printf("resid 0\n");
}

static void test_instr80_todos(void) {
    printf("  [instr 80b]  ");
    /* encode/decode com campos no maximo 24-bit */
    Instr80 I = { .addr_a = 0xFFFFFF, .addr_b = 0xFFFFFF, .op = 0xFF, .addr_r = 0xFFFFFF };
    uint8_t buf[10];
    encode80(&I, buf);
    Instr80 J;
    decode80(buf, &J);
    CHECK(J.addr_a == 0xFFFFFF && J.addr_b == 0xFFFFFF &&
          J.op == 0xFF && J.addr_r == 0xFFFFFF, "encode/decode max");

    /* todas as ops via MOVE + ALU */
    struct { uint8_t op; u64 a, b, expect; } casos[] = {
        {IOP_ADD, 15, 27, 42},
        {IOP_SUB, 100, 42, 58},
        {IOP_MUL, 12, 12, 144},
        {IOP_DIV, 100, 7, 14},
        {IOP_AND, 0xF0, 0x0F, 0},
        {IOP_OR,  0xF0, 0x0F, 0xFF},
        {IOP_XOR, 0xFF, 0x0F, 0xF0},
        {IOP_NOT, 0, 0, ULLONG_MAX},  /* b don't-care */
        {IOP_MOV, 99, 0, 99},         /* b don't-care */
    };
    int bad = 0;
    for (int i = 0; i < 9; i++) {
        Maquina M;
        maq_clear(&M);
        STORE_VAL(&M, 0, casos[i].a);
        STORE_VAL(&M, 1, casos[i].b);
        Instr80 ii = {0, 1, casos[i].op, 2};
        encode80(&ii, buf);
        Instr80 halt = {0, 0, IOP_HALT, 0};
        uint8_t prog[20];
        encode80(&ii, prog);
        encode80(&halt, prog + 10);
        roda80(&M, prog, 20, NULL);
        if (LOAD(&M, 2) != casos[i].expect) bad++;
    }
    CHECK(bad == 0, "todas as ops via 80-bit/MOVE");

    /* JMP: pula HALT intermediario */
    {
        Maquina M;
        maq_clear(&M);
        STORE_VAL(&M, 0, 7);
        STORE_VAL(&M, 1, 1);
        /* [0] JMP -> byte 20; [10] HALT (pulado); [20] ADD 0,1 -> 2; [30] HALT */
        uint8_t prog[40];
        Instr80 jmp  = {20, 0, IOP_JMP, 0};
        Instr80 dead = {0, 0, IOP_HALT, 0};
        Instr80 add  = {0, 1, IOP_ADD, 2};
        Instr80 end  = {0, 0, IOP_HALT, 0};
        encode80(&jmp,  prog + 0);
        encode80(&dead, prog + 10);
        encode80(&add,  prog + 20);
        encode80(&end,  prog + 30);
        int steps = 0;
        roda80(&M, prog, 40, &steps);
        CHECK(LOAD(&M, 2) == 8, "JMP pula HALT morto");
        CHECK(steps == 3, "JMP+ADD+HALT = 3 passos");
    }
    /* JMP 0 deve permanecer em 0 (loop), nao avancar para +10 */
    {
        Maquina M;
        maq_clear(&M);
        uint8_t prog[20];
        Instr80 jmp0 = {0, 0, IOP_JMP, 0};   /* JMP to byte 0 */
        Instr80 halt = {0, 0, IOP_HALT, 0};
        encode80(&jmp0, prog);
        encode80(&halt, prog + 10);
        /* limita passos para nao hangar: roda manualmente 3 ciclos */
        M.pc = 0;
        for (int i = 0; i < 3; i++) {
            Instr80 I;
            decode80(prog + M.pc, &I);
            int jumped = 0;
            exec80(&M, &I, &jumped);
            if (!jumped) M.pc += 10;
        }
        CHECK(M.pc == 0, "JMP 0 permanece em PC=0");
    }
    printf("resid 0\n");
}

static void test_controle(void) {
    printf("  [controle]   ");
    Maquina M;
    maq_clear(&M);
    STORE_VAL(&M, 0, 3);
    STORE_VAL(&M, 1, 4);
    STORE_VAL(&M, 2, 5);
    /* (a+b)*c via MOVE-sintese */
    Instr80 is[] = {
        {0, 1, IOP_ADD, 10},
        {10, 2, IOP_MUL, 11},
        {0, 0, IOP_HALT, 0}
    };
    uint8_t prog[30];
    for (int i = 0; i < 3; i++) encode80(&is[i], prog + 10 * i);
    int steps = 0;
    roda80(&M, prog, 30, &steps);
    CHECK(LOAD(&M, 11) == 35, "(3+4)*5");
    CHECK(steps == 3, "3 passos");
    printf("resid 0\n");
}

static void test_analog_reference(void) {
    printf("  [analog ref] ");
    int bad = 0;
    for (int a = 1; a <= 50; a++)
        for (int b = 1; b <= 50; b++) {
            if (llround(analog_mul(a, b)) != (long long)MUL((u64)a, (u64)b)) bad++;
            if (llround(analog_add(a, b)) != (long long)ADD((u64)a, (u64)b)) bad++;
            if (llround(analog_div(a * b, b)) != a) bad++;
        }
    CHECK(bad == 0, "ref numerica log/exp == ISA");
    printf("resid 0\n");
}

static void test_memoria(void) {
    printf("  [memoria]    ");
    /* rotl64 sem UB */
    const u64 x = 0x0123456789ABCDEFULL;
    CHECK(rotl64(x, 0) == x, "rotl k=0");
    CHECK(rotl64(x, 64) == x, "rotl k=64");
    CHECK(rotl64(rotl64(x, 7), 57) == x, "rotl inverso 7+57");
    CHECK(rotl64(x, 1) == ((x << 1) | (x >> 63)), "rotl k=1");
    u64 seed = 0xC0FFEEULL;
    CHECK(stream_roundtrip(1024, seed) == 0, "1 KB");
    CHECK(stream_roundtrip(65536, seed) == 0, "64 KB");
    CHECK(stream_roundtrip(1048576, seed) == 0, "1 MB");
    printf("resid 0\n");
}

/* ===========================================================================
 * Demo
 * =========================================================================== */

static void demo(void) {
    printf("\n== Demo: tudo sintetizado a partir de MOVE ==\n\n");

    printf("1. MOVE e a operacao irredutivel:\n");
    Maquina M;
    maq_clear(&M);
    M.canal = 42;
    MOVE(&M, 0, -1);                 /* STORE */
    printf("   MOVE(0, -1) store canal=42\n");
    M.canal = 0;
    printf("   MOVE(0, +1) load  = %llu\n", (unsigned long long)MOVE(&M, 0, +1));

    printf("\n2. Instrucao 80-bit = 2x LOAD + OP + STORE (tudo MOVE):\n");
    maq_clear(&M);
    STORE_VAL(&M, 0, 15);
    STORE_VAL(&M, 1, 27);
    STORE_VAL(&M, 2, 3);
    STORE_VAL(&M, 3, 6);
    Instr80 is[] = {
        {0, 1, IOP_ADD, 10},
        {10, 2, IOP_MUL, 11},
        {11, 3, IOP_DIV, 12},
        {0, 0, IOP_HALT, 0}
    };
    uint8_t prog[40];
    for (int i = 0; i < 4; i++) encode80(&is[i], prog + 10 * i);
    int steps = 0;
    roda80(&M, prog, 40, &steps);
    printf("   ((15+27)*3)/6 = %llu  (%d tiques)\n",
           (unsigned long long)LOAD(&M, 12), steps);

    printf("\n3. JMP = MOVE(pc, 0):\n");
    printf("   (testado: pula HALT morto, executa ADD)\n");

    printf("\n4. Relogio (g=3 gerador completo de F_40961):\n");
    Univ U = universal_init_exato(40961, 3, 16);
    u64 acc = 1;
    for (int t = 0; t < 16; t++) acc = u_mul(&U, acc, U.w);
    printf("   w^16 = %llu  ordem_exata=%d\n",
           (unsigned long long)acc, ordem_exata(&U, U.w));

    printf("\n");
}

int main(void) {
    printf("Microprocessador Fractal - modelo de referencia (MOVE)\n");
    printf("======================================================\n");
    printf("(ISA: api.h | analogico: so_cristal.c | arquitetura: micro.c)\n");
    printf("Tese: MOVE = transferencia de estado; computacao = MOVE* + ALU.\n\n");

    test_move();
    test_add_ripple();
    test_mul_div();
    test_sub_logic();
    test_relogio();
    test_zoom();
    test_fracao_continua();
    test_barramento();
    test_instr80_todos();
    test_controle();
    test_analog_reference();
    test_memoria();

    if (g_fail == 0)
        printf("\n[OK] todos os testes: resid 0\n");
    else
        printf("\n[X] %d falha(s)\n", g_fail);

    demo();
    return g_fail ? 1 : 0;
}
#endif /* MICRO_AS_LIB */

