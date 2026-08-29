/* traduz_c_asm_node.c — cadeia C → assembly → metal (sem Node runtime).
 *   interpretar.c → traduz → node.wasm → wasm_erg --all → celula.erg
 *   node_corre.erg → erg monta/corre → stdout
 *
 *   gcc -O2 -std=c99 -w tests/traduz_c_asm_node.c -o tools/bin/traduz_c_asm_node.exe
 *   tools/bin/traduz_c_asm_node.exe
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARENA_SIZE 65536
#define NULO_DISCO 8
#define MEM_SLOTS 65500
#define RODATA_TAG 65408
#define OFF_NIN 24576
#define OFF_NOUT 24578
#define OFF_IN 256
#define OFF_OUT 16384
#define CAP 8192

static unsigned char arena_metal[ARENA_SIZE];
static unsigned char membuf_metal[MEM_SLOTS * 2];

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

static char *read_all(const char *path, long *out_sz) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc((size_t)sz + 1);
    if (!buf) { fclose(f); return NULL; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        free(buf);
        fclose(f);
        return NULL;
    }
    buf[sz] = 0;
    fclose(f);
    if (out_sz) *out_sz = sz;
    return buf;
}

static int buf_has(const char *buf, long sz, const char *needle) {
    size_t nl = strlen(needle);
    if (nl == 0 || sz < (long)nl) return 0;
    for (long i = 0; i <= sz - (long)nl; i++)
        if (memcmp(buf + i, needle, nl) == 0) return 1;
    return 0;
}

static int wasm_has_export(const char *path, const char *name) {
    long sz;
    char *buf = read_all(path, &sz);
    if (!buf) return 0;
    int found = buf_has(buf, sz, name);
    free(buf);
    return found;
}

static int dif_files(const char *a, const char *b) {
    long sa, sb;
    char *A = read_all(a, &sa);
    char *B = read_all(b, &sb);
    if (!A || !B) { free(A); free(B); return -1; }
    int d = (sa != sb);
    if (!d) d = memcmp(A, B, (size_t)sa) != 0;
    free(A);
    free(B);
    return d;
}

static int erg_has_opcode(const char *path, const char *op) {
    char *txt = read_all(path, NULL);
    if (!txt) return 0;
    int found = strstr(txt, op) != NULL;
    free(txt);
    return found;
}

static void arena_para_mem(const unsigned char *arena, unsigned char *mem) {
    memset(mem, 0, MEM_SLOTS * 2);
    for (int s = 0; s < ARENA_SIZE; s++)
        mem[(NULO_DISCO + s) * 2] = arena[s];
}

static void mem_para_arena(const unsigned char *mem, unsigned char *arena) {
    for (int s = 0; s < ARENA_SIZE; s++)
        arena[s] = mem[(NULO_DISCO + s) * 2];
}

static void semeia_rodata(unsigned char *mem) {
    const char *tag = "console.log(";
    int off = RODATA_TAG;
    for (int i = 0; tag[i]; i++)
        mem[(NULO_DISCO + off + i) * 2] = (unsigned char)tag[i];
}

static void seed_script(unsigned char *arena, const char *script) {
    memset(arena, 0, ARENA_SIZE);
    int n = (int)strlen(script);
    if (n > CAP) n = CAP;
    memcpy(arena + OFF_IN, script, (size_t)n);
    arena[OFF_NIN] = (unsigned char)(n & 255);
    arena[OFF_NIN + 1] = (unsigned char)((n >> 8) & 255);
}

static int parse_consts(const char *erg_path, int slots[256], int vals[256]) {
    FILE *f = fopen(erg_path, "r");
    if (!f) return 0;
    char line[256];
    int n = 0;
    while (fgets(line, sizeof line, f) && n < 256) {
        int slot, val;
        if (sscanf(line, "; CONST %d %d", &slot, &val) == 2) {
            slots[n] = slot;
            vals[n] = val;
            n++;
        }
    }
    fclose(f);
    return n;
}

static void semeia_consts(const char *erg_bin, const char *mem_path, const char *erg_path) {
    int slots[256], vals[256];
    int n = parse_consts(erg_path, slots, vals);
    char cmd[512];
    snprintf(cmd, sizeof cmd, "%s poe \"%s\" 0 0 0", erg_bin, mem_path);
    run_cmd(cmd);
    for (int i = 0; i < n; i++) {
        int v = vals[i];
        snprintf(cmd, sizeof cmd, "%s poe \"%s\" %d %d %d", erg_bin, mem_path, slots[i],
                 v & 255, (v >> 8) & 255);
        run_cmd(cmd);
    }
}

static int test_can5_metal(const char *erg_bin) {
    const char *wasm = "assets/figuras/wasm/node.wasm";
    const char *erg = "conecthus/backends/node/node_corre.erg";
    const char *fita = "conecthus/backends/node/node_corre.fita.bin";
    const char *dir = ".torre/reino_asm_node";
    char mem[512], cmd[1024];

    snprintf(cmd, sizeof cmd, "mkdir \"%s\" 2>nul", dir);
    run_cmd(cmd);
    snprintf(mem, sizeof mem, "%s/mem.dat", dir);

    snprintf(cmd, sizeof cmd,
             "tools\\bin\\traduz.exe conecthus\\backends\\node\\interpretar.c -o %s", wasm);
    if (run_cmd(cmd) != 0) return 0;

    snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_erg.exe %s node_corre %s", wasm, erg);
    if (run_cmd(cmd) != 0) return 0;

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", erg_bin, erg, fita);
    if (run_cmd(cmd) != 0) return 0;

    unsigned char *arena = arena_metal;
    unsigned char *membuf = membuf_metal;
    seed_script(arena, "console.log('42')");
    arena_para_mem(arena, membuf);
    semeia_rodata(membuf);
    FILE *mf = fopen(mem, "wb");
    if (!mf) return 0;
    fwrite(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);

    semeia_consts(erg_bin, mem, erg);

    snprintf(cmd, sizeof cmd, "%s corre \"%s\" \"%s\" 2000000", erg_bin, fita, mem);
    if (run_cmd(cmd) != 0) return 0;

    mf = fopen(mem, "rb");
    if (!mf) return 0;
    fread(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);
    mem_para_arena(membuf, arena);

    int n = arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256;
    char out[CAP + 1];
    if (n > CAP) n = CAP;
    memcpy(out, arena + OFF_OUT, (size_t)n);
    out[n] = 0;
    return strstr(out, "42") != NULL;
}

static int manifesto_tem_asm(void) {
    char *txt = read_all("conecthus/backends/manifesto.json", NULL);
    if (!txt) return 0;
    int ok1 = strstr(txt, "\"asm\"") != NULL && strstr(txt, "celula.erg") != NULL;
    free(txt);
    return ok1;
}

int main(void) {
    const char *fonte = "conecthus/backends/node/interpretar.c";
    const char *wasm = "assets/figuras/wasm/node.wasm";
    const char *celula = "conecthus/backends/node/celula.erg";
    const char *traduz = "tools\\bin\\traduz.exe";
    const char *wasm_erg = "tools\\bin\\wasm_erg.exe";
    char cmd[1024];
    char wasm_a[512], wasm_b[512];

    unit_check("CAN0 interpretar.c existe", file_exists(fonte));
    unit_check("CAN0 traduz disponível", file_exists(traduz));
    unit_check("CAN0 wasm_erg disponível", file_exists(wasm_erg));

    snprintf(cmd, sizeof cmd, "mkdir assets\\figuras\\wasm 2>nul");
    run_cmd(cmd);

    snprintf(cmd, sizeof cmd, "%s \"%s\" -o \"%s\"", traduz, fonte, wasm);
    unit_check("CAN1 sobe(interpretar.c) → node.wasm", run_cmd(cmd) == 0 && file_size(wasm) > 100);
    unit_check("CAN1 export node_move", wasm_has_export(wasm, "node_move"));

    snprintf(cmd, sizeof cmd, "%s \"%s\" --all \"%s\"", wasm_erg, wasm, celula);
    unit_check("CAN2 wasm→assembly (--all)", run_cmd(cmd) == 0 && file_size(celula) > 500);
    unit_check("CAN2 assembly menciona node_move", wasm_has_export(celula, "node_move"));
    unit_check("CAN2 assembly tem LOAD", erg_has_opcode(celula, "LOAD"));
    unit_check("CAN2 assembly tem STORE", erg_has_opcode(celula, "STORE"));
    unit_check("CAN2 assembly tem HALT", erg_has_opcode(celula, "HALT"));
    {
        char *t = read_all(celula, NULL);
        int sem = t && strstr(t, "; wasm 0x") == NULL;
        unit_check("CAN2 sem stubs wasm 0x", sem);
        free(t);
    }

    unit_check("CAN0 wasm_sec disponível", file_exists("tools\\bin\\wasm_sec.exe"));

    {
        long sz0 = file_size(wasm);
        const char *volta = ".torre/volta_node.erg";
        snprintf(cmd, sizeof cmd,
                 "tools\\bin\\wasm_sec.exe embute \"%s\" \"%s\" node.erg \"%s\"",
                 wasm, celula, wasm);
        unit_check("CAN3 embute node.erg no wasm", run_cmd(cmd) == 0 && file_size(wasm) > sz0);
        unit_check("CAN3 wasm cresceu com secção", file_size(wasm) > sz0 + 100);
        snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_sec.exe extrai \"%s\" node.erg \"%s\"", wasm, volta);
        unit_check("CAN4 extrai node.erg", run_cmd(cmd) == 0 && file_size(volta) > 500);
        unit_check("CAN4 desce preserva node_move", wasm_has_export(volta, "node_move"));
        unit_check("CAN4 desce ≈ celula.erg", dif_files(volta, celula) == 0);
    }

    unit_check("CAN5 metal node_corre stdout 42", test_can5_metal("tools\\bin\\erg_new.exe"));

    unit_check("CAN6 manifesto asm/celula.erg", manifesto_tem_asm());

    snprintf(wasm_a, sizeof wasm_a, ".torre/asm_node_a.wasm");
    snprintf(wasm_b, sizeof wasm_b, ".torre/asm_node_b.wasm");
    snprintf(cmd, sizeof cmd, "mkdir .torre 2>nul");
    run_cmd(cmd);
    snprintf(cmd, sizeof cmd, "%s \"%s\" -o \"%s\"", traduz, fonte, wasm_a);
    run_cmd(cmd);
    snprintf(cmd, sizeof cmd, "%s \"%s\" -o \"%s\"", traduz, fonte, wasm_b);
    run_cmd(cmd);
    unit_check("CAN7 sobe(C) determinístico", dif_files(wasm_a, wasm_b) == 0);

    printf("#TOTAL %d %d\n", n_feitas, n_falhas);
    return n_falhas ? 1 : 0;
}
