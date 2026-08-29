/* traduz_asm_wasm.c — ponte ASM ↔ WASM em C (sem Node).
 *   soma.erg → erg monta/desmonta → wasm_sec asm_sobe/asm_desce (isa.wasm + erg.fita)
 *
 *   gcc -O2 -std=c99 -w tests/traduz_asm_wasm.c -o tools/bin/traduz_asm_wasm.exe
 *   tools/bin/traduz_asm_wasm.exe
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ERG_BIN "tools\\bin\\erg_new.exe"
#define TRADUZ_BIN "tools\\bin\\traduz.exe"
#define WASM_SEC "tools\\bin\\wasm_sec.exe"
#define SOMA "banco/apps/soma.erg"
#define ISA_C "tools/isa.c"
#define ISA_WASM "assets/figuras/wasm/isa.wasm"

static int n_falhas = 0, n_feitas = 0;

static void unit_check(const char *q, int cond) {
    n_feitas++;
    if (!cond) n_falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

static int run_cmd(const char *cmd) {
    return system(cmd);
}

static long file_size(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fclose(f);
    return sz;
}

static int file_exists(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static unsigned char *read_all(const char *path, long *out_sz) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = (unsigned char *)malloc((size_t)sz);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    if (out_sz) *out_sz = sz;
    return buf;
}

static int dif_bytes(const unsigned char *a, long sa, const unsigned char *b, long sb) {
    if (sa != sb) return 1;
    return memcmp(a, b, (size_t)sa) != 0;
}

static int dif_files_bin(const char *a, const char *b) {
    long sa, sb;
    unsigned char *A = read_all(a, &sa);
    unsigned char *B = read_all(b, &sb);
    if (!A || !B) { free(A); free(B); return -1; }
    int d = dif_bytes(A, sa, B, sb);
    free(A);
    free(B);
    return d;
}

static int garante_isa_wasm(void) {
    if (file_exists(ISA_WASM)) return 1;
    run_cmd("mkdir assets\\figuras\\wasm 2>nul");
    char cmd[512];
    snprintf(cmd, sizeof cmd, "%s \"%s\" -o \"%s\"", TRADUZ_BIN, ISA_C, ISA_WASM);
    return run_cmd(cmd) == 0 && file_exists(ISA_WASM);
}

static int wasm_valido(const char *path) {
    unsigned char *b = read_all(path, NULL);
    if (!b) return 0;
    int ok1 = b[0] == 0 && b[1] == 0x61 && b[2] == 0x73 && b[3] == 0x6d;
    free(b);
    return ok1;
}

static int wasm_tem_sec(const char *wasm_path, const char *sec) {
    char cmd[512];
    char tmp[256];
    snprintf(tmp, sizeof tmp, ".torre/extrai_%s.bin", sec);
    snprintf(cmd, sizeof cmd, "%s extrai \"%s\" %s \"%s\"", WASM_SEC, wasm_path, sec, tmp);
    if (run_cmd(cmd) != 0) return 0;
    long sz = file_size(tmp);
    return sz > 0;
}

static int wasm_tem_move(const char *path) {
    long sz;
    unsigned char *b = read_all(path, &sz);
    if (!b) return 0;
    int found = 0;
    for (long i = 0; i <= sz - 4; i++) {
        if (b[i] == 'M' && b[i + 1] == 'O' && b[i + 2] == 'V' && b[i + 3] == 'E') {
            found = 1;
            break;
        }
    }
    free(b);
    return found;
}

int main(void) {
    char cmd[1024];
    const char *fita0 = ".torre/aw_fita0.bin";
    const char *fita1 = ".torre/aw_fita1.bin";
    const char *volta = ".torre/aw_volta.erg";
    const char *wasm_out = ".torre/aw_soma.wasm";
    const char *wasm2 = ".torre/aw_soma2.wasm";
    const char *erg_desce = ".torre/aw_desce.erg";
    const char *fita_desce = ".torre/aw_fita_desce.bin";

    run_cmd("mkdir .torre 2>nul");

    unit_check("AW0 soma.erg existe", file_exists(SOMA));
    unit_check("AW0 erg disponível", file_exists(ERG_BIN));

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", ERG_BIN, SOMA, fita0);
    unit_check("AW0 monta(soma.erg) > 0", run_cmd(cmd) == 0 && file_size(fita0) > 0);

    snprintf(cmd, sizeof cmd, "%s desmonta \"%s\" > \"%s\"", ERG_BIN, fita0, volta);
    unit_check("AW1 desmonta(monta) gera texto", run_cmd(cmd) == 0 && file_size(volta) > 0);

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", ERG_BIN, volta, fita1);
    unit_check("AW1 remonta fita", run_cmd(cmd) == 0 && file_size(fita1) > 0);
    unit_check("AW1 desmonta(monta) byte a byte", dif_files_bin(fita0, fita1) == 0);

    unit_check("AW2 traduz disponível", file_exists(TRADUZ_BIN));
    unit_check("AW2 wasm_sec disponível", file_exists(WASM_SEC));
    unit_check("AW2 isa.wasm disponível", garante_isa_wasm());

    snprintf(cmd, sizeof cmd, "%s asm_sobe \"%s\" \"%s\" \"%s\"",
             WASM_SEC, SOMA, ISA_WASM, wasm_out);
    unit_check("AW2 asm_sobe gera wasm válido",
               run_cmd(cmd) == 0 && wasm_valido(wasm_out));
    unit_check("AW2 secção erg.fita presente", wasm_tem_sec(wasm_out, "erg.fita"));

    snprintf(cmd, sizeof cmd, "%s extrai \"%s\" erg.fita \"%s\"",
             WASM_SEC, wasm_out, fita_desce);
    unit_check("AW2 fita extraída = monta(original)",
               run_cmd(cmd) == 0 && dif_files_bin(fita0, fita_desce) == 0);

    snprintf(cmd, sizeof cmd, "%s asm_desce \"%s\" \"%s\"", WASM_SEC, wasm_out, erg_desce);
    unit_check("AW3 asm_desce gera erg", run_cmd(cmd) == 0 && file_size(erg_desce) > 0);

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", ERG_BIN, erg_desce, fita1);
    unit_check("AW3 desce(sobe) remonta fita",
               run_cmd(cmd) == 0 && dif_files_bin(fita0, fita1) == 0);

    unit_check("AW4 módulo menciona MOVE", wasm_tem_move(wasm_out));

    {
        const char *fita5 = ".torre/aw_fita5.bin";
        snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", ERG_BIN, SOMA, fita5);
        unit_check("AW5 monta erg nativo = monta directo",
                   run_cmd(cmd) == 0 && dif_files_bin(fita0, fita5) == 0);
    }

    snprintf(cmd, sizeof cmd, "%s asm_desce \"%s\" \"%s\"", WASM_SEC, wasm_out, erg_desce);
    run_cmd(cmd);
    snprintf(cmd, sizeof cmd, "%s asm_sobe \"%s\" \"%s\" \"%s\"",
             WASM_SEC, erg_desce, ISA_WASM, wasm2);
    unit_check("AW6 sobe∘desce preserva erg.fita",
               run_cmd(cmd) == 0 && wasm_tem_sec(wasm2, "erg.fita"));

    {
        char tmp[256];
        snprintf(tmp, sizeof tmp, ".torre/aw_fita6.bin");
        snprintf(cmd, sizeof cmd, "%s extrai \"%s\" erg.fita \"%s\"",
                 WASM_SEC, wasm2, tmp);
        unit_check("AW6 fita idempotente",
                   run_cmd(cmd) == 0 && dif_files_bin(fita0, tmp) == 0);
    }

    printf("#TOTAL %d %d\n", n_feitas, n_falhas);
    return n_falhas ? 1 : 0;
}
