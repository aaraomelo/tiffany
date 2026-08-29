/* wasm_erg.c — wasm (pilha) → ERG-64 (registadores), chessb §C4 / fisica.tex
 *
 * Caminhada do operador (fis:def:op, fis:thm:zeroinf, fis:def:duomorf, fis:thm:contraria):
 *   ∂∘∂=id  — um passo ERG por opcode; laço = vertical k (fis:def:coord2)
 *   NULO=8  — bloco 0–∞: slot ISA ↔ arena; rodata ≥65536 → 65400+
 *   𝒟 (a=1) — fusão compare→branch: troca materializar⊗ por salto⊕ (duomorfismo, não iso)
 *   óptica    — fis:caixa: LOADA·LOADA·VINCO entre espelhos; INC paralelo = |det|=1
 *   π/ρ       — traduz enche (sobrejetivo); passos contam injetivo; mesma folga Φ
 *
 * Trial ~ 𝒟 na fita (espelha interpretar.c — duomorfismo-pipe.md §Trial):
 *   ~ célula     LOAD/STORE/LOADS — mesmo slot arena (π(i)=π(j))
 *   ~ diferença  VINCO+JZ/JNZ — prefixo, c==q/10/13, i==nin; 𝒟 funde compare→salto
 *   ~ razão      SUB16+TROCA — LT/GE/magnitude; laço shell evita; 𝒟 troca com diferença
 * Execução ERG (trial na travessia, não no retorno C):
 *   VINCO≠0 + JZ exit  → trial −1 (vácuo: prefix fail, recusa)
 *   laço continua      → trial +1 (radiação: opera corpo)
 *   break sem exit     → trial 0 (matéria: fronteira LF/CR ou vazio)
 *
 * Uso:
 *   wasm_erg <modulo.wasm> <export_name> [saida.erg]
 *   wasm_erg <modulo.wasm> --all [saida.erg]   (todos os exports de função)
 * Escreve ; CONST slot valor por linha antes do assembly. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define LOCAL_BASE 600
#define TEMP_BASE 900
#define CONST_BASE 1200
#define ZERO 0

#define MAX_LINES 8192
#define MAX_STACK 512
#define MAX_CONSTS 256
#define MAX_CODES 64
#define MAX_EXPORTS 32
#define LINE_LEN 128

typedef struct { int v; int next; } Leb;

static Leb leb_read(const unsigned char *buf, int len, int pos) {
    uint64_t v = 0;
    int s = 0;
    int p = pos;
    while (p < len) {
        unsigned char c = buf[p++];
        v |= (uint64_t)(c & 0x7f) << s;
        if ((c & 0x80) == 0) return (Leb){ (int)v, p };
        s += 7;
        if (s > 63) break;
    }
    fprintf(stderr, "LEB inválido\n");
    exit(1);
}

typedef struct { int id; const unsigned char *body; int len; } Sec;

static int parse_sections(const unsigned char *buf, int n, Sec *secs, int max) {
    if (n < 8 || buf[1] != 0x61 || buf[2] != 0x73 || buf[3] != 0x6d) {
        fprintf(stderr, "não é wasm\n");
        return -1;
    }
    int ns = 0, p = 8;
    while (p < n && ns < max) {
        int id = buf[p++];
        Leb sz = leb_read(buf, n, p);
        p = sz.next;
        secs[ns].id = id;
        secs[ns].body = buf + p;
        secs[ns].len = sz.v;
        ns++;
        p += sz.v;
    }
    return ns;
}

typedef struct { char name[64]; int kind; int idx; } Export;

typedef struct {
    int import_func_count;
    unsigned char *codes[MAX_CODES];
    int code_lens[MAX_CODES];
    int n_codes;
    Export exports[MAX_EXPORTS];
    int n_exports;
} Mod;

static int skip_locals(const unsigned char *body, int blen, int p0) {
    int p = p0;
    Leb ng = leb_read(body, blen, p);
    p = ng.next;
    for (int g = 0; g < ng.v; g++) {
        Leb cnt = leb_read(body, blen, p);
        p = cnt.next + 1;
    }
    return p;
}

static int detect_call_target(const unsigned char *body, int blen) {
    int p = skip_locals(body, blen, 0);
    if (p < blen && body[p] == 0x10) {
        Leb idx = leb_read(body, blen, p + 1);
        if (idx.next < blen && (body[idx.next] == 0x0f || body[idx.next] == 0x0b))
            return idx.v;
    }
    return -1;
}

static void parse_module(const unsigned char *buf, int n, Mod *mod) {
    memset(mod, 0, sizeof *mod);
    Sec secs[32];
    int ns = parse_sections(buf, n, secs, 32);
    for (int si = 0; si < ns; si++) {
        Sec *s = &secs[si];
        const unsigned char *b = s->body;
        int blen = s->len;
        int p;
        if (s->id == 2) {
            Leb nn = leb_read(b, blen, 0);
            p = nn.next;
            for (int i = 0; i < nn.v; i++) {
                Leb ml = leb_read(b, blen, p);
                p = ml.next + ml.v;
                Leb nl = leb_read(b, blen, p);
                p = nl.next + nl.v;
                int kind = b[p++];
                if (kind == 0) mod->import_func_count++;
                else p += 2;
            }
        }
        if (s->id == 10) {
            Leb nn = leb_read(b, blen, 0);
            p = nn.next;
            for (int i = 0; i < nn.v && mod->n_codes < MAX_CODES; i++) {
                Leb fs = leb_read(b, blen, p);
                mod->codes[mod->n_codes] = (unsigned char *)(b + fs.next);
                mod->code_lens[mod->n_codes] = fs.v;
                mod->n_codes++;
                p = fs.next + fs.v;
            }
        }
        if (s->id == 7) {
            Leb nn = leb_read(b, blen, 0);
            p = nn.next;
            for (int i = 0; i < nn.v && mod->n_exports < MAX_EXPORTS; i++) {
                Leb nl = leb_read(b, blen, p);
                p = nl.next;
                int nlen = nl.v < 63 ? nl.v : 63;
                memcpy(mod->exports[mod->n_exports].name, b + p, (size_t)nlen);
                mod->exports[mod->n_exports].name[nlen] = 0;
                p += nl.v;
                mod->exports[mod->n_exports].kind = b[p++];
                Leb idx = leb_read(b, blen, p);
                mod->exports[mod->n_exports].idx = idx.v;
                p = idx.next;
                mod->n_exports++;
            }
        }
    }
}

typedef struct { int kind; char start[32]; char end[32]; } Ctrl;

typedef struct {
    const unsigned char *body;
    int blen;
    const Mod *mod;
    int p;
    char lines[MAX_LINES][LINE_LEN];
    int n_lines;
    int stack[MAX_STACK];
    int sp;
    int const_keys[MAX_CONSTS];
    int const_slots[MAX_CONSTS];
    int n_consts;
    int temp_n;
    int label_n;
    Ctrl control[64];
    int n_ctrl;
    int depth;
    char epilogue[32];
    int has_epilogue;
    int sv[1400];
    int r_slot; /* slot cujo valor está em R (−1 = desconhecido); STORE não clobber */
    /* afim: slot ≅ base + off (régua/óptica — compor NULO+OFF_IN antes do i) */
    int aff_base[1400]; /* −2 desconhecido, −1 const pura (off=valor), ≥0 base */
    int aff_off[1400];
    int loads_src[1400]; /* slot veio de LOADS (byte arena) — não INC */
    int load8_addr[1400]; /* byte temp → slot endereço LOADA (−1 = n/a) */
    int optica_ptr[1400]; /* endereço usado em travessia LOADA·LOADA·VINCO (fis:caixa) */
} Ctx;

static void sv_init(Ctx *c) {
    for (int i = 0; i < 1400; i++) {
        c->sv[i] = -1;
        c->aff_base[i] = -2;
        c->aff_off[i] = 0;
        c->loads_src[i] = 0;
        c->load8_addr[i] = -1;
        c->optica_ptr[i] = 0;
    }
}

static void sv_set(Ctx *c, int slot, int val) {
    if (slot >= 0 && slot < 1400) c->sv[slot] = val & 0xFFFF;
}

static int sv_get(Ctx *c, int slot) {
    if (slot < 0 || slot >= 1400) return -1;
    return c->sv[slot];
}

static void sv_clear(Ctx *c, int slot) {
    if (slot >= 0 && slot < 1400) {
        c->sv[slot] = -1;
        c->aff_base[slot] = -2;
        c->aff_off[slot] = 0;
    }
}

static void aff_set_const(Ctx *c, int slot, int val) {
    if (slot < 0 || slot >= 1400) return;
    c->aff_base[slot] = -1;
    c->aff_off[slot] = val & 0xFFFF;
}

static void aff_set_base(Ctx *c, int slot, int base, int off) {
    if (slot < 0 || slot >= 1400) return;
    c->aff_base[slot] = base;
    c->aff_off[slot] = off & 0xFFFF;
}

static int is_local_slot(int s) {
    return s >= LOCAL_BASE && s < TEMP_BASE;
}

static int is_counter_local(Ctx *c, int s) {
    return is_local_slot(s) && !c->loads_src[s];
}

static void emit(Ctx *c, const char *s) {
    if (c->n_lines >= MAX_LINES) { fprintf(stderr, "overflow lines\n"); exit(1); }
    snprintf(c->lines[c->n_lines++], LINE_LEN, "%s", s);
    /* R só cai com ops que escrevem R; LOAD/CMP/LOADS mexem A·B·flags (óptica: régua ≠ motor) */
    if (s[0] == ':') return;
    if (!strncmp(s, "JMP ", 4) || !strncmp(s, "JZ ", 3) || !strncmp(s, "JNZ ", 4)) return;
    if (!strncmp(s, "STORE", 5)) return;
    if (!strncmp(s, "LOAD ", 5) || !strncmp(s, "LOADS ", 6)) return;
    if (!strncmp(s, "CMP", 3)) return;
    c->r_slot = -1;
}

static int slot_local(Ctx *c, int i) { (void)c; return LOCAL_BASE + i; }
static int fresh_temp(Ctx *c) { return TEMP_BASE + c->temp_n++; }
static void new_label(Ctx *c, const char *pfx, char *out, int cap) {
    snprintf(out, (size_t)cap, "%s%d", pfx, c->label_n++);
}

#define RODATA_RELOC_BASE 65400

/* Endereços wasm ≥65536 não cabem em slot u16 da ISA — remapeia para a faixa alta. */
static int reloc_addr(int k) {
    if (k >= 65536) return RODATA_RELOC_BASE + (k - 65536);
    return k;
}

static int slot_const(Ctx *c, int k) {
    k = reloc_addr(k);
    for (int i = 0; i < c->n_consts; i++)
        if (c->const_keys[i] == k) return c->const_slots[i];
    if (c->n_consts >= MAX_CONSTS) { fprintf(stderr, "overflow consts\n"); exit(1); }
    int slot = CONST_BASE + c->n_consts;
    c->const_keys[c->n_consts] = k;
    c->const_slots[c->n_consts] = slot;
    c->n_consts++;
    sv_set(c, slot, k);
    aff_set_const(c, slot, k);
    return slot;
}

static void push(Ctx *c, int slot) {
    if (c->sp >= MAX_STACK) { fprintf(stderr, "pilha wasm\n"); exit(1); }
    c->stack[c->sp++] = slot;
}

static int pop(Ctx *c) {
    if (c->sp <= 0) { fprintf(stderr, "pilha vazia @%d op=0x%02x\n", c->p, c->p > 0 ? c->body[c->p-1] : 0); exit(1); }
    return c->stack[--c->sp];
}

static void emit_r_from(Ctx *c, int slot) {
    char buf[LINE_LEN];
    if (c->r_slot == slot) return; /* R já tem o valor — evita LOAD·LOAD0·ADD */
    snprintf(buf, sizeof buf, "LOAD %d", slot);
    emit(c, buf);
    emit(c, "LOAD 0");
    emit(c, "ADD");
    c->r_slot = slot;
}

static void emit_store_r(Ctx *c, int slot) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "STORE %d", slot);
    emit(c, buf);
    c->r_slot = slot; /* STORE grava R; R intacto */
}

/* Se a última linha é STORE slot e R ainda tem esse valor, apaga-a (temp ponte). */
static int drop_dead_store(Ctx *c, int slot) {
    char expect[48];
    if (c->r_slot != slot || c->n_lines <= 0) return 0;
    snprintf(expect, sizeof expect, "STORE %d", slot);
    if (strcmp(c->lines[c->n_lines - 1], expect) != 0) return 0;
    c->n_lines--;
    return 1;
}

/* Reescreve STORE slot → STORE dst (local.set sobre temp fresco). */
static int rewrite_store_to(Ctx *c, int slot, int dst) {
    char expect[48];
    if (c->r_slot != slot || c->n_lines <= 0) return 0;
    snprintf(expect, sizeof expect, "STORE %d", slot);
    if (strcmp(c->lines[c->n_lines - 1], expect) != 0) return 0;
    snprintf(c->lines[c->n_lines - 1], LINE_LEN, "STORE %d", dst);
    c->r_slot = dst;
    if (c->loads_src[slot]) c->loads_src[dst] = 1;
    return 1;
}

static void emit_cmp_zero(Ctx *c, int slot, const char *jump) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "LOAD %d", slot);
    emit(c, buf);
    emit(c, "LOAD 0");
    emit(c, "CMP");
    snprintf(buf, sizeof buf, "JZ %s", jump);
    emit(c, buf);
}

/* =/≠ face ⊕: SUB + flags←zero(R) — vinco fis:thm:simbolos (4); ~ diferença; JZ/JNZ trial */

/* Apaga LOADA addr; LOAD 0; ADD; STORE byte (materialização desnecessária na óptica). */
static int remove_load8_byte_emit(Ctx *c, int byte_slot, int addr_slot) {
    char st[48], loads[48];
    snprintf(st, sizeof st, "STORE %d", byte_slot);
    snprintf(loads, sizeof loads, "LOADS %d", addr_slot);
    for (int i = c->n_lines - 1; i >= 3; i--) {
        if (strcmp(c->lines[i], st) != 0) continue;
        if (strcmp(c->lines[i - 1], "ADD") != 0) continue;
        if (strcmp(c->lines[i - 2], "LOAD 0") != 0) continue;
        if (strcmp(c->lines[i - 3], loads) != 0) continue;
        for (int j = i - 3; j + 4 < c->n_lines; j++)
            strcpy(c->lines[j], c->lines[j + 4]);
        c->n_lines -= 4;
        c->loads_src[byte_slot] = 0;
        c->load8_addr[byte_slot] = -1;
        return 1;
    }
    return 0;
}

/* fis:caixa + fis:def:coord2 — espelhos OFF_IN+k / RODATA_TAG+k; LOADA·LOADA·VINCO (~ diferença).
 * JZ a seguir (emit_branch_fused): VINCO≠0 → trial −1 (prefix fail); =0 → continua (+1). */
static int emit_vinco_espelhos(Ctx *c, int a, int b) {
    if (!c->loads_src[a] || !c->loads_src[b]) return 0;
    int aa = c->load8_addr[a], ab = c->load8_addr[b];
    if (aa < 0 || ab < 0) return 0;
    if (c->aff_base[aa] != c->aff_base[ab] || c->aff_base[aa] < 0) return 0;
    if (!remove_load8_byte_emit(c, b, ab)) return 0;
    if (!remove_load8_byte_emit(c, a, aa)) return 0;
    c->optica_ptr[aa] = 1;
    c->optica_ptr[ab] = 1;
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "LOADS %d", aa);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOADS %d", ab);
    emit(c, buf);
    emit(c, "VINCO");
    c->r_slot = -1;
    return 1;
}

static void emit_vinco(Ctx *c, int a, int b) {
    if (emit_vinco_espelhos(c, a, b)) return; /* óptica: ~ diferença sem materializar byte */
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "LOAD %d", a);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", b);
    emit(c, buf);
    emit(c, "VINCO");
    c->r_slot = -1;
}

static int const_val(Ctx *c, int slot) {
    for (int i = 0; i < c->n_consts; i++)
        if (c->const_slots[i] == slot) return c->const_keys[i];
    return -1;
}

static void emit_div256(Ctx *c, int a) {
    int t = fresh_temp(c);
    emit_r_from(c, a);
    emit(c, "TROCA");
    emit_store_r(c, t);
    push(c, t);
}

/* i32.rem_u 256: byte baixo do Word (nout % 256). */
static void emit_mod256(Ctx *c, int a) {
    int t = fresh_temp(c);
    emit_r_from(c, a);
    emit_store_r(c, t);
    push(c, t);
}

static int drop_affine_add(Ctx *c, int temp, int base, int old_off) {
    /* apaga LOAD off; LOAD base; ADD16; STORE temp no fim da fita */
    char l0[48], l1[48], st[48];
    int off_slot = slot_const(c, old_off);
    if (c->n_lines < 4) return 0;
    snprintf(l0, sizeof l0, "LOAD %d", off_slot);
    snprintf(l1, sizeof l1, "LOAD %d", base);
    snprintf(st, sizeof st, "STORE %d", temp);
    int n = c->n_lines;
    if (strcmp(c->lines[n - 1], st) != 0) return 0;
    if (strcmp(c->lines[n - 2], "ADD16") != 0) return 0;
    /* ordem LOAD off; LOAD base  OU  LOAD base; LOAD off */
    int ok = (strcmp(c->lines[n - 4], l0) == 0 && strcmp(c->lines[n - 3], l1) == 0)
          || (strcmp(c->lines[n - 4], l1) == 0 && strcmp(c->lines[n - 3], l0) == 0);
    if (!ok) return 0;
    c->n_lines -= 4;
    return 1;
}

static void emit_binop(Ctx *c, const char *op) {
    int b = pop(c), a = pop(c);
    /* ADD16 +1 num local → INC slot (fis:algoritmo: escrita G+=1 num passo) */
    if (!strcmp(op, "ADD16")) {
        int ca = const_val(c, a), cb = const_val(c, b);
        int tgt = -1;
        if (ca == 1 && is_counter_local(c, b)) tgt = b;
        else if (cb == 1 && is_counter_local(c, a)) tgt = a;
        if (tgt >= 0) {
            char buf[LINE_LEN];
            snprintf(buf, sizeof buf, "INC %d", tgt);
            emit(c, buf);
            c->r_slot = tgt;
            {
                int v = sv_get(c, tgt);
                if (v >= 0) sv_set(c, tgt, (v + 1) & 0xFFFF);
                else sv_clear(c, tgt);
            }
            push(c, tgt);
            return;
        }
    }
    /* só const⊗const — locais com sv (ex. i no laço) não são invariantes */
    if (a >= CONST_BASE && b >= CONST_BASE) {
        int va = sv_get(c, a), vb = sv_get(c, b);
        if (va >= 0 && vb >= 0) {
            int r = -1;
            if (!strcmp(op, "ADD16")) r = (va + vb) & 0xFFFF;
            else if (!strcmp(op, "SUB16")) r = (va - vb) & 0xFFFF;
            else if (!strcmp(op, "MUL16")) r = (va * vb) & 0xFFFF;
            else if (!strcmp(op, "AND")) r = va & vb;
            else if (!strcmp(op, "OR")) r = va | vb;
            else if (!strcmp(op, "XOR")) r = va ^ vb;
            if (r >= 0) {
                push(c, slot_const(c, r));
                return;
            }
        }
    }
    /* ADD16 afim: (base+off)+const → base+(off+const) — compõe NULO∘OFF_IN (óptica/régua) */
    if (!strcmp(op, "ADD16")) {
        int ca = const_val(c, a), cb = const_val(c, b);
        int base = -2, off = 0, cst = -1, other = -1;
        if (ca >= 0 && c->aff_base[b] >= 0) {
            base = c->aff_base[b]; off = c->aff_off[b]; cst = ca; other = b;
        } else if (cb >= 0 && c->aff_base[a] >= 0) {
            base = c->aff_base[a]; off = c->aff_off[a]; cst = cb; other = a;
        } else if (ca >= 0 && is_local_slot(b)) {
            base = b; off = 0; cst = ca; other = -1;
        } else if (cb >= 0 && is_local_slot(a)) {
            base = a; off = 0; cst = cb; other = -1;
        }
        if (base >= 0 && cst >= 0) {
            int new_off = (off + cst) & 0xFFFF;
            int t = fresh_temp(c);
            char buf[LINE_LEN];
            if (other >= 0)
                drop_affine_add(c, other, base, off);
            snprintf(buf, sizeof buf, "LOAD %d", slot_const(c, new_off));
            emit(c, buf);
            snprintf(buf, sizeof buf, "LOAD %d", base);
            emit(c, buf);
            emit(c, "ADD16");
            snprintf(buf, sizeof buf, "STORE %d", t);
            emit(c, buf);
            c->r_slot = t;
            if (t >= 0 && t < 1400) c->sv[t] = -1;
            aff_set_base(c, t, base, new_off);
            push(c, t);
            return;
        }
    }
    int t = fresh_temp(c);
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "LOAD %d", a);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", b);
    emit(c, buf);
    emit(c, op);
    snprintf(buf, sizeof buf, "STORE %d", t);
    emit(c, buf);
    c->r_slot = t;
    sv_clear(c, t);
    push(c, t);
}

static void emit_load8_imm(Ctx *c, int slot) {
    int t = fresh_temp(c);
    char buf[LINE_LEN];
    emit_r_from(c, slot);
    snprintf(buf, sizeof buf, "STORE %d", t);
    emit(c, buf);
    sv_clear(c, t);
    push(c, t);
}

static void emit_load8_at(Ctx *c, int addr_slot) {
    int addr = sv_get(c, addr_slot);
    int t = fresh_temp(c);
    char buf[LINE_LEN];
    /* Só imediato para endereços no pool CONST; temps/locais mudam em runtime. */
    if (addr >= 0 && addr_slot >= CONST_BASE) {
        emit_load8_imm(c, addr);
        return;
    }
    snprintf(buf, sizeof buf, "LOADS %d", addr_slot);
    emit(c, buf);
    emit(c, "LOAD 0");
    emit(c, "ADD");
    snprintf(buf, sizeof buf, "STORE %d", t);
    emit(c, buf);
    c->r_slot = t;
    c->loads_src[t] = 1;
    c->load8_addr[t] = addr_slot;
    sv_clear(c, t);
    push(c, t);
}

static void emit_store8_at(Ctx *c, int val_slot, int addr_slot) {
    int addr = sv_get(c, addr_slot);
    char buf[LINE_LEN];
    emit_r_from(c, val_slot);
    drop_dead_store(c, val_slot);
    if (addr >= 0 && addr_slot >= CONST_BASE) {
        emit_store_r(c, addr);
    } else {
        snprintf(buf, sizeof buf, "STORE_IND %d", addr_slot);
        emit(c, buf);
    }
}

/* i32 relacional via SUB16 + TROCA (Word = par de bytes; estrela no slot). */
static void emit_push_i32(Ctx *c, int one_or_zero) {
    int t = fresh_temp(c);
    emit_r_from(c, slot_const(c, one_or_zero));
    emit_store_r(c, t);
    push(c, t);
}

static void emit_i32_eq(Ctx *c, int a, int b) {
    int diff = fresh_temp(c), res = fresh_temp(c);
    char lEq[32], lEnd[32];
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "LOAD %d", a);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", b);
    emit(c, buf);
    emit(c, "SUB");
    snprintf(buf, sizeof buf, "STORE %d", diff);
    emit(c, buf);
    new_label(c, "EQ", lEq, sizeof lEq);
    new_label(c, "ED", lEnd, sizeof lEnd);
    emit_cmp_zero(c, diff, lEq);
    emit_r_from(c, slot_const(c, 0));
    emit_store_r(c, res);
    snprintf(buf, sizeof buf, "JMP %s", lEnd);
    emit(c, buf);
    snprintf(buf, sizeof buf, ":%s", lEq);
    emit(c, buf);
    emit_r_from(c, slot_const(c, 1));
    emit_store_r(c, res);
    snprintf(buf, sizeof buf, ":%s", lEnd);
    emit(c, buf);
    push(c, res);
}

static void emit_i32_gt(Ctx *c, int a, int b) {
    int diff = fresh_temp(c), td = fresh_temp(c), ts = fresh_temp(c), res = fresh_temp(c);
    char lEq[32], lNotGt[32], lEnd[32];
    char buf[LINE_LEN];
    int c255 = slot_const(c, 255);
    snprintf(buf, sizeof buf, "LOAD %d", a);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", b);
    emit(c, buf);
    emit(c, "SUB16");
    snprintf(buf, sizeof buf, "STORE %d", diff);
    emit(c, buf);
    new_label(c, "EQ", lEq, sizeof lEq);
    new_label(c, "NG", lNotGt, sizeof lNotGt);
    new_label(c, "ED", lEnd, sizeof lEnd);
    emit_cmp_zero(c, diff, lEq);
    snprintf(buf, sizeof buf, "LOAD %d", diff);
    emit(c, buf);
    emit(c, "TROCA");
    snprintf(buf, sizeof buf, "STORE %d", td);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", td);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", c255);
    emit(c, buf);
    emit(c, "SUB");
    snprintf(buf, sizeof buf, "STORE %d", ts);
    emit(c, buf);
    emit_cmp_zero(c, ts, lNotGt);
    emit_r_from(c, slot_const(c, 1));
    emit_store_r(c, res);
    snprintf(buf, sizeof buf, "JMP %s", lEnd);
    emit(c, buf);
    snprintf(buf, sizeof buf, ":%s", lNotGt);
    emit(c, buf);
    emit_r_from(c, slot_const(c, 0));
    emit_store_r(c, res);
    snprintf(buf, sizeof buf, "JMP %s", lEnd);
    emit(c, buf);
    snprintf(buf, sizeof buf, ":%s", lEq);
    emit(c, buf);
    emit_r_from(c, slot_const(c, 0));
    emit_store_r(c, res);
    snprintf(buf, sizeof buf, ":%s", lEnd);
    emit(c, buf);
    push(c, res);
}

static void emit_i32_not(Ctx *c, int v) {
    int t = fresh_temp(c);
    char lZ[32], lE[32];
    char buf[LINE_LEN];
    new_label(c, "NZ", lZ, sizeof lZ);
    new_label(c, "NE", lE, sizeof lE);
    emit_cmp_zero(c, v, lZ);
    emit_r_from(c, slot_const(c, 0));
    emit_store_r(c, t);
    snprintf(buf, sizeof buf, "JMP %s", lE);
    emit(c, buf);
    snprintf(buf, sizeof buf, ":%s", lZ);
    emit(c, buf);
    emit_r_from(c, slot_const(c, 1));
    emit_store_r(c, t);
    snprintf(buf, sizeof buf, ":%s", lE);
    emit(c, buf);
    push(c, t);
}

/* Fusão compare→br_if/if — duomorfismo-pipe.md §Fusão / §Trial wasm_erg.
 * 𝒟: evita materializar 0/1 Word (face ⊗); salto na face ⊕ (VINCO/CMP).
 * FC_EQ/NE → ~ diferença; FC_LT/GE → ~ razão (TROCA) — shell hot path evita LT/GE. */
enum { FC_EQ, FC_NE, FC_LT, FC_GE, FC_EQZ };

static int read_block_type(Ctx *c);
static void read_instrs(Ctx *c);

/* GT/LT via τ no Word (TROCA = fis:thm:simbolos τ) + teste de ZERO (só JZ/JNZ).
 * ~ razão (𝒟 troca com diferença): magnitude/ordem — não no laço prefixo/cópia shell.
 * Diferença 16-bit: sinal no bit 7 de .e — AND 0x80, não «==255».
 * lib/simbolos.h: oito relações → um bloco + nega; ISA só salta em FL_ZERO. */
static void emit_branch_if_gt(Ctx *c, int left, int right, const char *target) {
    char skip[32], buf[LINE_LEN];
    int diff = fresh_temp(c), td = fresh_temp(c);
    int c80 = slot_const(c, 128); /* bit de sinal do byte alto */
    new_label(c, "BF", skip, sizeof skip);
    snprintf(buf, sizeof buf, "LOAD %d", left);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", right);
    emit(c, buf);
    emit(c, "SUB16");
    snprintf(buf, sizeof buf, "STORE %d", diff);
    emit(c, buf);
    emit_cmp_zero(c, diff, skip);          /* = → não LT */
    snprintf(buf, sizeof buf, "LOAD %d", diff);
    emit(c, buf);
    emit(c, "TROCA");                      /* τ: .e → .total */
    snprintf(buf, sizeof buf, "STORE %d", td);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", td);
    emit(c, buf);
    snprintf(buf, sizeof buf, "LOAD %d", c80);
    emit(c, buf);
    emit(c, "AND");
    emit(c, "LOAD 0");
    emit(c, "CMP");
    snprintf(buf, sizeof buf, "JZ %s", skip); /* bit sinal apagado → não LT */
    emit(c, buf);
    snprintf(buf, sizeof buf, "JMP %s", target);
    emit(c, buf);
    snprintf(buf, sizeof buf, ":%s", skip);
    emit(c, buf);
}

static void emit_branch_fused(Ctx *c, int a, int b, int kind, const char *target) {
    char skip[32], buf[LINE_LEN];
    int diff;
    int za = const_val(c, a), zb = const_val(c, b);
    new_label(c, "BF", skip, sizeof skip);
    switch (kind) {
    case FC_EQ:
        /* ~ diferença: VINCO=0 → JZ target (igual); ≠0 → trial −1 se target=exit */
        /* = com 0: régua colapsa a FL_ZERO directo (gravidade → vinco) */
        if (za == 0) { emit_cmp_zero(c, b, target); break; }
        if (zb == 0) { emit_cmp_zero(c, a, target); break; }
        emit_vinco(c, a, b);
        snprintf(buf, sizeof buf, "JZ %s", target);
        emit(c, buf);
        break;
    case FC_NE:
        if (za == 0) {
            emit_cmp_zero(c, b, skip);
            snprintf(buf, sizeof buf, "JMP %s", target);
            emit(c, buf);
            snprintf(buf, sizeof buf, ":%s", skip);
            emit(c, buf);
            break;
        }
        if (zb == 0) {
            emit_cmp_zero(c, a, skip);
            snprintf(buf, sizeof buf, "JMP %s", target);
            emit(c, buf);
            snprintf(buf, sizeof buf, ":%s", skip);
            emit(c, buf);
            break;
        }
        emit_vinco(c, a, b);
        snprintf(buf, sizeof buf, "JNZ %s", skip);
        emit(c, buf);
        snprintf(buf, sizeof buf, "JMP %s", target);
        emit(c, buf);
        snprintf(buf, sizeof buf, ":%s", skip);
        emit(c, buf);
        break;
    case FC_LT:
        emit_branch_if_gt(c, b, a, target);
        break;
    case FC_GE:
        emit_branch_if_gt(c, b, a, skip);
        snprintf(buf, sizeof buf, "JMP %s", target);
        emit(c, buf);
        snprintf(buf, sizeof buf, ":%s", skip);
        emit(c, buf);
        break;
    default:
        fprintf(stderr, "emit_branch_fused kind\n");
        exit(1);
    }
}

static void emit_skip_fused(Ctx *c, int a, int b, int kind, const char *else_lbl) {
    char buf[LINE_LEN];
    int diff;
    int za = const_val(c, a), zb = const_val(c, b);
    switch (kind) {
    case FC_EQ:
        if (za == 0) {
            snprintf(buf, sizeof buf, "LOAD %d", b);
            emit(c, buf);
            emit(c, "LOAD 0");
            emit(c, "CMP");
            snprintf(buf, sizeof buf, "JNZ %s", else_lbl);
            emit(c, buf);
            break;
        }
        if (zb == 0) {
            snprintf(buf, sizeof buf, "LOAD %d", a);
            emit(c, buf);
            emit(c, "LOAD 0");
            emit(c, "CMP");
            snprintf(buf, sizeof buf, "JNZ %s", else_lbl);
            emit(c, buf);
            break;
        }
        emit_vinco(c, a, b);
        snprintf(buf, sizeof buf, "JNZ %s", else_lbl);
        emit(c, buf);
        break;
    case FC_NE:
        if (za == 0) { emit_cmp_zero(c, b, else_lbl); break; }
        if (zb == 0) { emit_cmp_zero(c, a, else_lbl); break; }
        emit_vinco(c, a, b);
        snprintf(buf, sizeof buf, "JZ %s", else_lbl);
        emit(c, buf);
        break;
    case FC_LT:
        emit_branch_if_gt(c, a, b, else_lbl);
        break;
    case FC_GE:
        emit_branch_if_gt(c, b, a, else_lbl);
        break;
    case FC_EQZ:
        snprintf(buf, sizeof buf, "LOAD %d", a);
        emit(c, buf);
        emit(c, "LOAD 0");
        emit(c, "CMP");
        snprintf(buf, sizeof buf, "JNZ %s", else_lbl);
        emit(c, buf);
        break;
    default:
        fprintf(stderr, "emit_skip_fused kind\n");
        exit(1);
    }
}

static void read_if_fused(Ctx *c, int a, int b, int kind) {
    char else_lbl[32], end_lbl[32], buf[LINE_LEN];
    read_block_type(c);
    new_label(c, "EL", else_lbl, sizeof else_lbl);
    new_label(c, "EI", end_lbl, sizeof end_lbl);
    c->control[c->n_ctrl++] = (Ctrl){ 0, "", "" };
    strncpy(c->control[c->n_ctrl - 1].end, end_lbl, 31);
    emit_skip_fused(c, a, b, kind, else_lbl);
    c->depth++;
    read_instrs(c);
    c->depth--;
    if (c->p < c->blen && c->body[c->p] == 0x05) {
        c->p++;
        snprintf(buf, sizeof buf, "JMP %s", end_lbl);
        emit(c, buf);
        snprintf(buf, sizeof buf, ":%s", else_lbl);
        emit(c, buf);
        c->depth++;
        read_instrs(c);
        c->depth--;
    } else {
        snprintf(buf, sizeof buf, ":%s", else_lbl);
        emit(c, buf);
    }
    if (c->p < c->blen && c->body[c->p] == 0x0b) c->p++;
    snprintf(buf, sizeof buf, ":%s", end_lbl);
    emit(c, buf);
    c->n_ctrl--;
}

static int try_fuse_br_if(Ctx *c, int a, int b, int kind) {
    if (c->p >= c->blen || c->body[c->p] != 0x0d) return 0;
    Leb d = leb_read(c->body, c->blen, c->p + 1);
    if (d.v >= c->n_ctrl) return 0;
    c->p = d.next;
    Ctrl *ctrl = &c->control[c->n_ctrl - 1 - d.v];
    const char *target = ctrl->kind == 1 ? ctrl->start : ctrl->end;
    emit_branch_fused(c, a, b, kind, target);
    return 1;
}

/* if (cond) { br d } ≡ br_if d−1 — o frame do if não conta (vinco =, sem EL/JMP) */
static int try_fuse_if_br(Ctx *c, int a, int b, int kind) {
    if (c->p >= c->blen || c->body[c->p] != 0x04) return 0;
    int save = c->p;
    int p = c->p + 1;
    unsigned char bt = c->body[p++];
    if (bt != 0x40 && bt != 0x7f && bt != 0x7e) { c->p = save; return 0; }
    if (p >= c->blen || c->body[p] != 0x0c) { c->p = save; return 0; }
    Leb d = leb_read(c->body, c->blen, p + 1);
    p = d.next;
    if (p >= c->blen || c->body[p] != 0x0b) { c->p = save; return 0; }
    /* br dentro do if: depth inclui o if; fora, br_if usa depth−1 */
    if (d.v < 1 || (d.v - 1) >= c->n_ctrl) { c->p = save; return 0; }
    c->p = p + 1;
    Ctrl *ctrl = &c->control[c->n_ctrl - 1 - (d.v - 1)];
    const char *target = ctrl->kind == 1 ? ctrl->start : ctrl->end;
    emit_branch_fused(c, a, b, kind, target);
    return 1;
}

static int try_fuse_if(Ctx *c, int a, int b, int kind) {
    if (try_fuse_if_br(c, a, b, kind)) return 1;
    if (c->p >= c->blen || c->body[c->p] != 0x04) return 0;
    c->p++;
    read_if_fused(c, a, b, kind);
    return 1;
}

static int invert_fc(int kind) {
    switch (kind) {
    case FC_EQ: return FC_NE;
    case FC_NE: return FC_EQ;
    case FC_LT: return FC_GE;
    case FC_GE: return FC_LT;
    default: return kind;
    }
}

static int try_fuse_cmp_control(Ctx *c, int a, int b, int kind) {
    /* while(cond): traduz emite cond; eqz; br_if — complementar (fis:thm:simbolos §9) */
    if (c->p < c->blen && c->body[c->p] == 0x45) {
        int save = c->p;
        c->p++;
        int inv = invert_fc(kind);
        if (try_fuse_br_if(c, a, b, inv)) return 1;
        if (try_fuse_if(c, a, b, inv)) return 1;
        c->p = save;
    }
    if (try_fuse_br_if(c, a, b, kind)) return 1;
    if (try_fuse_if(c, a, b, kind)) return 1;
    return 0;
}

static int try_fuse_eqz_control(Ctx *c, int v) {
    if (c->p < c->blen && c->body[c->p] == 0x0d) {
        Leb d = leb_read(c->body, c->blen, c->p + 1);
        if (d.v < c->n_ctrl) {
            c->p = d.next;
            Ctrl *ctrl = &c->control[c->n_ctrl - 1 - d.v];
            const char *target = ctrl->kind == 1 ? ctrl->start : ctrl->end;
            emit_cmp_zero(c, v, target);
            return 1;
        }
    }
    if (c->p < c->blen && c->body[c->p] == 0x04) {
        c->p++;
        read_if_fused(c, v, 0, FC_EQZ);
        return 1;
    }
    return 0;
}

static void emit_br(Ctx *c, int depth) {
    if (depth >= c->n_ctrl) { fprintf(stderr, "br depth\n"); exit(1); }
    Ctrl *ctrl = &c->control[c->n_ctrl - 1 - depth];
    const char *target = ctrl->kind == 1 ? ctrl->start : ctrl->end;
    char buf[LINE_LEN];
    snprintf(buf, sizeof buf, "JMP %s", target);
    /* laço: br 0 no corpo + JMP implícito no end → um só */
    if (c->n_lines > 0 && strcmp(c->lines[c->n_lines - 1], buf) == 0) return;
    emit(c, buf);
}

static int read_block_type(Ctx *c) {
    unsigned char t = c->body[c->p++];
    if (t == 0x40 || t == 0x7f || t == 0x7e) return 0;
    fprintf(stderr, "blocktype 0x%02x\n", t);
    exit(1);
}

static void read_instrs(Ctx *c);

static void read_instrs(Ctx *c) {
    while (c->p < c->blen) {
        unsigned char op = c->body[c->p++];
        char buf[LINE_LEN], lbl[32];
        switch (op) {
        case 0x00: case 0x01: break;
        case 0x02: {
            read_block_type(c);
            new_label(c, "BE", lbl, sizeof lbl);
            c->control[c->n_ctrl++] = (Ctrl){ 0, "", "" };
            strncpy(c->control[c->n_ctrl - 1].end, lbl, 31);
            c->depth++;
            read_instrs(c);
            c->depth--;
            /* consome o end que fez return — senão if/else externo vê o mesmo 0x0b */
            if (c->p < c->blen && c->body[c->p] == 0x0b) c->p++;
            snprintf(buf, sizeof buf, ":%s", lbl);
            emit(c, buf);
            c->n_ctrl--;
            break;
        }
        case 0x03: {
            read_block_type(c);
            new_label(c, "LS", lbl, sizeof lbl);
            char end[32];
            new_label(c, "LE", end, sizeof end);
            snprintf(buf, sizeof buf, ":%s", lbl);
            emit(c, buf);
            c->control[c->n_ctrl++] = (Ctrl){ 1, "", "" };
            strncpy(c->control[c->n_ctrl - 1].start, lbl, 31);
            strncpy(c->control[c->n_ctrl - 1].end, end, 31);
            c->depth++;
            read_instrs(c);
            c->depth--;
            if (c->p < c->blen && c->body[c->p] == 0x0b) c->p++;
            snprintf(buf, sizeof buf, "JMP %s", lbl);
            if (!(c->n_lines > 0 && strcmp(c->lines[c->n_lines - 1], buf) == 0))
                emit(c, buf);
            snprintf(buf, sizeof buf, ":%s", end);
            emit(c, buf);
            c->n_ctrl--;
            break;
        }
        case 0x04: {
            read_block_type(c);
            int cond = pop(c);
            char else_lbl[32], end_lbl[32];
            new_label(c, "EL", else_lbl, sizeof else_lbl);
            new_label(c, "EI", end_lbl, sizeof end_lbl);
            c->control[c->n_ctrl++] = (Ctrl){ 0, "", "" };
            strncpy(c->control[c->n_ctrl - 1].end, end_lbl, 31);
            emit_cmp_zero(c, cond, else_lbl);
            c->depth++;
            read_instrs(c);
            c->depth--;
            if (c->p < c->blen && c->body[c->p] == 0x05) {
                c->p++;
                snprintf(buf, sizeof buf, "JMP %s", end_lbl);
                emit(c, buf);
                snprintf(buf, sizeof buf, ":%s", else_lbl);
                emit(c, buf);
                c->depth++;
                read_instrs(c);
                c->depth--;
            } else {
                snprintf(buf, sizeof buf, ":%s", else_lbl);
                emit(c, buf);
            }
            if (c->p < c->blen && c->body[c->p] == 0x0b) c->p++;
            snprintf(buf, sizeof buf, ":%s", end_lbl);
            emit(c, buf);
            c->n_ctrl--;
            break;
        }
        case 0x05:
            if (c->depth > 0) {
                c->p--;
                return;
            }
            fprintf(stderr, "else inesperado @%d\n", c->p - 1);
            exit(1);
        case 0x0b:
            if (c->depth > 0) {
                c->p--;
                return;
            }
            break;
        case 0x0c: {
            Leb d = leb_read(c->body, c->blen, c->p);
            c->p = d.next;
            emit_br(c, d.v);
            break;
        }
        case 0x0d: {
            Leb d = leb_read(c->body, c->blen, c->p);
            c->p = d.next;
            int cond = pop(c);
            if (d.v >= c->n_ctrl) {
                fprintf(stderr, "br_if depth=%d n_ctrl=%d @%d\n", d.v, c->n_ctrl, c->p - 1);
                exit(1);
            }
            Ctrl *ctrl = &c->control[c->n_ctrl - 1 - d.v];
            const char *target = ctrl->kind == 1 ? ctrl->start : ctrl->end;
            char skip[32];
            new_label(c, "BI", skip, sizeof skip);
            emit_cmp_zero(c, cond, skip);
            snprintf(buf, sizeof buf, "JMP %s", target);
            emit(c, buf);
            snprintf(buf, sizeof buf, ":%s", skip);
            emit(c, buf);
            break;
        }
        case 0x0f:
            if (c->sp > 0) emit_r_from(c, pop(c));
            return;
        case 0x10: {
            Leb fi = leb_read(c->body, c->blen, c->p);
            c->p = fi.next;
            int code_idx = fi.v - c->mod->import_func_count;
            if (code_idx < 0 || code_idx >= c->mod->n_codes) {
                fprintf(stderr, "call import %d\n", fi.v);
                exit(1);
            }
            Ctx *sub = (Ctx *)calloc(1, sizeof(Ctx));
            if (!sub) { fprintf(stderr, "oom\n"); exit(1); }
            sub->body = c->mod->codes[code_idx];
            sub->blen = c->mod->code_lens[code_idx];
            sub->mod = c->mod;
            sub->p = skip_locals(sub->body, sub->blen, 0);
            sub->r_slot = -1;
            read_instrs(sub);
            for (int i = 0; i < sub->n_lines; i++) emit(c, sub->lines[i]);
            for (int i = 0; i < sub->n_consts; i++)
                slot_const(c, sub->const_keys[i]);
            if (sub->temp_n > c->temp_n) c->temp_n = sub->temp_n;
            free(sub);
            break;
        }
        case 0x1a: pop(c); break;
        case 0x20: {
            Leb i = leb_read(c->body, c->blen, c->p);
            c->p = i.next;
            push(c, slot_local(c, i.v));
            break;
        }
        case 0x21: {
            Leb i = leb_read(c->body, c->blen, c->p);
            c->p = i.next;
            int v = pop(c);
            int dst = slot_local(c, i.v);
            if (v == dst) break; /* INC/local.set no-op */
            /* temp acabado de materializar em R: reescreve STORE t → STORE local */
            if (rewrite_store_to(c, v, dst)) {
                int vv = sv_get(c, v);
                if (vv >= 0) sv_set(c, dst, vv);
                else sv_clear(c, dst);
                break;
            }
            emit_r_from(c, v);
            emit_store_r(c, dst);
            if (c->loads_src[v]) c->loads_src[dst] = 1;
            int vv = sv_get(c, v);
            if (vv >= 0) sv_set(c, dst, vv);
            else sv_clear(c, dst);
            break;
        }
        case 0x22: {
            Leb i = leb_read(c->body, c->blen, c->p);
            c->p = i.next;
            int v = pop(c);
            int dst = slot_local(c, i.v);
            if (rewrite_store_to(c, v, dst)) {
                push(c, dst);
                break;
            }
            emit_r_from(c, v);
            emit_store_r(c, dst);
            push(c, dst);
            break;
        }
        case 0x41: {
            Leb k = leb_read(c->body, c->blen, c->p);
            c->p = k.next;
            push(c, slot_const(c, k.v));
            break;
        }
        case 0x28: {
            c->p++;
            Leb off = leb_read(c->body, c->blen, c->p);
            c->p = off.next;
            int addr = pop(c);
            if (off.v != 0) {
                int a = fresh_temp(c), o = slot_const(c, off.v);
                snprintf(buf, sizeof buf, "LOAD %d", addr);
                emit(c, buf);
                snprintf(buf, sizeof buf, "LOAD %d", o);
                emit(c, buf);
                emit(c, "ADD16");
                snprintf(buf, sizeof buf, "STORE %d", a);
                emit(c, buf);
                int va = sv_get(c, addr), vo = sv_get(c, o);
                if (va >= 0 && vo >= 0) sv_set(c, a, va + vo);
                else sv_clear(c, a);
                addr = a;
            }
            emit_load8_at(c, addr);
            break;
        }
        case 0x2d: {
            c->p++;
            Leb off = leb_read(c->body, c->blen, c->p);
            c->p = off.next;
            int addr = pop(c);
            if (off.v != 0) {
                int a = fresh_temp(c), o = slot_const(c, off.v);
                snprintf(buf, sizeof buf, "LOAD %d", addr);
                emit(c, buf);
                snprintf(buf, sizeof buf, "LOAD %d", o);
                emit(c, buf);
                emit(c, "ADD16");
                snprintf(buf, sizeof buf, "STORE %d", a);
                emit(c, buf);
                int va = sv_get(c, addr), vo = sv_get(c, o);
                if (va >= 0 && vo >= 0) sv_set(c, a, va + vo);
                else sv_clear(c, a);
                addr = a;
            }
            emit_load8_at(c, addr);
            break;
        }
        case 0x36: {
            c->p++;
            Leb off = leb_read(c->body, c->blen, c->p);
            c->p = off.next;
            int val = pop(c), addr = pop(c);
            if (off.v != 0) {
                int a = fresh_temp(c), o = slot_const(c, off.v);
                snprintf(buf, sizeof buf, "LOAD %d", addr);
                emit(c, buf);
                snprintf(buf, sizeof buf, "LOAD %d", o);
                emit(c, buf);
                emit(c, "ADD16");
                snprintf(buf, sizeof buf, "STORE %d", a);
                emit(c, buf);
                int va = sv_get(c, addr), vo = sv_get(c, o);
                if (va >= 0 && vo >= 0) sv_set(c, a, va + vo);
                else sv_clear(c, a);
                addr = a;
            }
            emit_r_from(c, val);
            drop_dead_store(c, val);
            if (sv_get(c, addr) >= 0 && addr >= CONST_BASE)
                emit_store_r(c, sv_get(c, addr));
            else {
                snprintf(buf, sizeof buf, "STORE_IND %d", addr);
                emit(c, buf);
            }
            break;
        }
        case 0x3a: {
            c->p++;
            Leb off = leb_read(c->body, c->blen, c->p);
            c->p = off.next;
            int val = pop(c), addr = pop(c);
            if (off.v != 0) {
                int a = fresh_temp(c), o = slot_const(c, off.v);
                snprintf(buf, sizeof buf, "LOAD %d", addr);
                emit(c, buf);
                snprintf(buf, sizeof buf, "LOAD %d", o);
                emit(c, buf);
                emit(c, "ADD16");
                snprintf(buf, sizeof buf, "STORE %d", a);
                emit(c, buf);
                int va = sv_get(c, addr), vo = sv_get(c, o);
                if (va >= 0 && vo >= 0) sv_set(c, a, va + vo);
                else sv_clear(c, a);
                addr = a;
            }
            emit_store8_at(c, val, addr);
            break;
        }
        case 0x46: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_EQ)) break;
            emit_i32_eq(c, a, b);
            break;
        }
        case 0x47: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_NE)) break;
            emit_i32_eq(c, a, b);
            int eq = pop(c);
            emit_i32_not(c, eq);
            break;
        }
        case 0x48: case 0x49: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_LT)) break;
            emit_i32_gt(c, b, a);
            break;
        }
        case 0x4a: case 0x4b: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_LT)) break;
            emit_i32_gt(c, a, b);
            break;
        }
        case 0x4c: case 0x4d: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_GE)) break;
            emit_i32_gt(c, a, b);
            int gt = pop(c);
            emit_i32_not(c, gt);
            break;
        }
        case 0x4e: case 0x4f: {
            int b = pop(c), a = pop(c);
            if (try_fuse_cmp_control(c, a, b, FC_GE)) break;
            emit_i32_gt(c, b, a);
            int gt = pop(c);
            emit_i32_not(c, gt);
            break;
        }
        case 0x45: {
            int a = pop(c);
            if (try_fuse_eqz_control(c, a)) break;
            int t = fresh_temp(c);
            char zlbl[32], endlbl[32];
            new_label(c, "EZ", zlbl, sizeof zlbl);
            new_label(c, "E0", endlbl, sizeof endlbl);
            emit_cmp_zero(c, a, zlbl);
            emit_r_from(c, slot_const(c, 0));
            emit_store_r(c, t);
            snprintf(buf, sizeof buf, "JMP %s", endlbl);
            emit(c, buf);
            snprintf(buf, sizeof buf, ":%s", zlbl);
            emit(c, buf);
            emit_r_from(c, slot_const(c, 1));
            emit_store_r(c, t);
            snprintf(buf, sizeof buf, ":%s", endlbl);
            emit(c, buf);
            push(c, t);
            break;
        }
        case 0x2c: {
            c->p++;
            Leb off = leb_read(c->body, c->blen, c->p);
            c->p = off.next;
            int addr = pop(c);
            if (off.v != 0) {
                int a = fresh_temp(c), o = slot_const(c, off.v);
                snprintf(buf, sizeof buf, "LOAD %d", addr);
                emit(c, buf);
                snprintf(buf, sizeof buf, "LOAD %d", o);
                emit(c, buf);
                emit(c, "ADD16");
                snprintf(buf, sizeof buf, "STORE %d", a);
                emit(c, buf);
                int va = sv_get(c, addr), vo = sv_get(c, o);
                if (va >= 0 && vo >= 0) sv_set(c, a, va + vo);
                else sv_clear(c, a);
                addr = a;
            }
            emit_load8_at(c, addr);
            break;
        }
        case 0x6a: emit_binop(c, "ADD16"); break;
        case 0x6b: emit_binop(c, "SUB16"); break;
        case 0x6c: emit_binop(c, "MUL16"); break;
        case 0x6d: {
            int b = pop(c), a = pop(c);
            if (const_val(c, b) == 256) emit_div256(c, a);
            else { fprintf(stderr, "div só por 256 @%d\n", c->p - 1); exit(1); }
            break;
        }
        case 0x6e: case 0x6f: {
            int b = pop(c), a = pop(c);
            if (const_val(c, b) == 256) emit_mod256(c, a);
            else { fprintf(stderr, "rem só por 256 @%d\n", c->p - 1); exit(1); }
            break;
        }
        case 0x71: emit_binop(c, "AND"); break;
        case 0x72: emit_binop(c, "OR"); break;
        case 0x73: emit_binop(c, "XOR"); break;
        default:
            fprintf(stderr, "opcode 0x%02x @%d\n", op, c->p - 1);
            exit(1);
        }
    }
}

static int parse_loads(const char *ln, int *slot) {
    return sscanf(ln, "LOADS %d", slot) == 1 ? 1 : 0;
}

static int parse_load(const char *ln, int *slot) {
    return sscanf(ln, "LOAD %d", slot) == 1 ? 1 : 0;
}

static int parse_store(const char *ln, int *slot) {
    return sscanf(ln, "STORE %d", slot) == 1 ? 1 : 0;
}

static int parse_inc(const char *ln, int *slot) {
    return sscanf(ln, "INC %d", slot) == 1 ? 1 : 0;
}

static int parse_jmp_label(const char *ln, char *lbl, int cap) {
    return sscanf(ln, "JMP %31s", lbl) == 1 ? 1 : 0;
}

static int parse_label(const char *ln, char *lbl, int cap) {
    if (ln[0] != ':') return 0;
    snprintf(lbl, (size_t)cap, "%s", ln + 1);
    return 1;
}

static void line_remove_range(Ctx *c, int start, int count) {
    if (count <= 0 || start < 0 || start + count > c->n_lines) return;
    for (int i = start; i + count < c->n_lines; i++)
        strcpy(c->lines[i], c->lines[i + count]);
    c->n_lines -= count;
}

static void line_insert(Ctx *c, int idx, const char *s) {
    if (c->n_lines >= MAX_LINES || idx < 0 || idx > c->n_lines) return;
    for (int i = c->n_lines; i > idx; i--)
        strcpy(c->lines[i], c->lines[i - 1]);
    snprintf(c->lines[idx], LINE_LEN, "%s", s);
    c->n_lines++;
}

/* Pós-passo fis:caixa — prefixo rodata: elide recálculo afim; mantém LOADS·LOADS·VINCO.
 * ~ diferença entre espelhos; INC paralelo nos ponteiros antes do JMP (|det|=1). */
static void optica_caixa_pass(Ctx *c) {
    for (int i = 0; i + 2 < c->n_lines; i++) {
        int a1 = -1, a2 = -1, c_in = -1, c_ro = -1, k_slot = -1;
        if (!parse_loads(c->lines[i], &a1)) continue;
        if (!parse_loads(c->lines[i + 1], &a2)) continue;
        if (strcmp(c->lines[i + 2], "VINCO") != 0) continue;
        if (i < 8) continue;
        int p = i - 8;
        int s1 = -1, s2 = -1;
        if (!parse_load(c->lines[p], &c_in)) continue;
        if (!parse_load(c->lines[p + 1], &k_slot)) continue;
        if (strcmp(c->lines[p + 2], "ADD16") != 0) continue;
        if (!parse_store(c->lines[p + 3], &s1) || s1 != a1) continue;
        if (!parse_load(c->lines[p + 4], &c_ro)) continue;
        {
            int ck = -1;
            if (!parse_load(c->lines[p + 5], &ck) || ck != k_slot) continue;
        }
        if (strcmp(c->lines[p + 6], "ADD16") != 0) continue;
        if (!parse_store(c->lines[p + 7], &s2) || s2 != a2) continue;
        char lbl[32] = "";
        int jmp_at = -1, inc_k = -1, inc_k_idx = -1;
        for (int j = i + 3; j < c->n_lines && j < i + 30; j++) {
            if (!parse_inc(c->lines[j], &inc_k) || inc_k != k_slot) continue;
            inc_k_idx = j;
            for (int k = j + 1; k < c->n_lines && k <= j + 6; k++) {
                if (parse_jmp_label(c->lines[k], lbl, sizeof lbl)) {
                    jmp_at = k;
                    break;
                }
            }
            if (jmp_at >= 0) break;
        }
        if (jmp_at < 0 || lbl[0] == 0 || inc_k_idx < 0) continue;
        int lbl_idx = -1;
        char loop_lbl[32];
        for (int j = 0; j < p; j++) {
            if (parse_label(c->lines[j], loop_lbl, sizeof loop_lbl) && !strcmp(loop_lbl, lbl)) {
                lbl_idx = j;
                break;
            }
        }
        if (lbl_idx < 0) continue;
        line_remove_range(c, p, 8);
        jmp_at -= 8;
        i = p + 2;
        char buf[LINE_LEN];
        snprintf(buf, sizeof buf, "LOAD %d", c_in);
        line_insert(c, lbl_idx, buf);
        snprintf(buf, sizeof buf, "STORE %d", a1);
        line_insert(c, lbl_idx + 1, buf);
        snprintf(buf, sizeof buf, "LOAD %d", c_ro);
        line_insert(c, lbl_idx + 2, buf);
        snprintf(buf, sizeof buf, "STORE %d", a2);
        line_insert(c, lbl_idx + 3, buf);
        /* translacção paralela (fis:caixa): INC nos dois ponteiros antes do JMP */
        for (int j = i; j < c->n_lines && j < i + 24; j++) {
            if (!parse_inc(c->lines[j], &inc_k) || inc_k != k_slot) continue;
            for (int k = j + 1; k < c->n_lines && k <= j + 6; k++) {
                if (!parse_jmp_label(c->lines[k], lbl, sizeof lbl)) continue;
                snprintf(buf, sizeof buf, "INC %d", a1);
                line_insert(c, k, buf);
                k++;
                snprintf(buf, sizeof buf, "INC %d", a2);
                line_insert(c, k, buf);
                break;
            }
            break;
        }
        return;
    }
}

static int export_to_erg(const unsigned char *buf, int n, const char *export_name, FILE *out) {
    Mod mod;
    parse_module(buf, n, &mod);
    int found = 0, func_idx = 0;
    for (int i = 0; i < mod.n_exports; i++) {
        if (mod.exports[i].kind == 0 && strcmp(mod.exports[i].name, export_name) == 0) {
            found = 1;
            func_idx = mod.exports[i].idx;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "export «%s» em falta\n", export_name);
        return 1;
    }
    int code_idx = func_idx - mod.import_func_count;
    if (code_idx < 0 || code_idx >= mod.n_codes) {
        fprintf(stderr, "export sem corpo\n");
        return 1;
    }
    const unsigned char *body = mod.codes[code_idx];
    int blen = mod.code_lens[code_idx];
    int callee = detect_call_target(body, blen);
    if (callee >= 0) {
        int ci = callee - mod.import_func_count;
        if (ci >= 0 && ci < mod.n_codes) {
            body = mod.codes[ci];
            blen = mod.code_lens[ci];
        }
    }
    Ctx *ctx = (Ctx *)calloc(1, sizeof(Ctx));
    if (!ctx) { fprintf(stderr, "oom\n"); return 1; }
    ctx->body = body;
    ctx->blen = blen;
    ctx->mod = &mod;
    ctx->p = skip_locals(body, blen, 0);
    ctx->r_slot = -1;
    sv_init(ctx);
    read_instrs(ctx);
    optica_caixa_pass(ctx);
    int has_halt = 0;
    for (int i = 0; i < ctx->n_lines; i++)
        if (strcmp(ctx->lines[i], "HALT") == 0) has_halt = 1;
    if (!has_halt) emit(ctx, "HALT");

    fprintf(out, "; %s — wasm→ERG (chessb §C4)\n\n", export_name);
    for (int i = 0; i < ctx->n_consts; i++)
        fprintf(out, "; CONST %d %d\n", ctx->const_slots[i], ctx->const_keys[i]);
    for (int i = 0; i < ctx->n_lines; i++)
        fprintf(out, "%s\n", ctx->lines[i]);
    free(ctx);
    return 0;
}

static int all_exports_to_erg(const unsigned char *buf, int n, FILE *out) {
    Mod mod;
    parse_module(buf, n, &mod);
    fprintf(out, "; wasm → assembly ERG-64 — cadeia física (sem runtime)\n");
    fprintf(out, "; gerado por wasm_erg.exe (C)\n\n");
    for (int i = 0; i < mod.n_exports; i++) {
        if (mod.exports[i].kind != 0) continue;
        if (export_to_erg(buf, n, mod.exports[i].name, out) != 0)
            return 1;
        fputc('\n', out);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "uso: wasm_erg <modulo.wasm> <export_name|--all> [saida.erg]\n");
        return 2;
    }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = (unsigned char *)malloc((size_t)sz);
    if (!buf || fread(buf, 1, (size_t)sz, f) != (size_t)sz) { perror("read"); return 1; }
    fclose(f);
    int all = strcmp(argv[2], "--all") == 0;
    FILE *out = stdout;
    const char *out_path = NULL;
    if (all) {
        if (argc >= 4) out_path = argv[3];
    } else if (argc >= 4) {
        out_path = argv[3];
    }
    if (out_path) {
        out = fopen(out_path, "w");
        if (!out) { perror(out_path); free(buf); return 1; }
    } else if (all) {
        fprintf(stderr, "uso: wasm_erg <modulo.wasm> --all <saida.erg>\n");
        free(buf);
        return 2;
    }
    int rc = all ? all_exports_to_erg(buf, (int)sz, out) : export_to_erg(buf, (int)sz, argv[2], out);
    if (out != stdout) fclose(out);
    free(buf);
    return rc;
}
