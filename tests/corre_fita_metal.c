/* corre_fita_metal.c — erg.fita embutida corre no metal (paridade com isa+fita browser).
 *   Extrai fita do wasm → erg corre → stdout (mesma fita que correBackendMetal usa).
 *
 *   gcc -O2 -std=c99 -w tests/corre_fita_metal.c -o tools/bin/corre_fita_metal.exe
 *   tools/bin/corre_fita_metal.exe
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

static unsigned char arena_buf[ARENA_SIZE];
static unsigned char membuf[MEM_SLOTS * 2];

static int n_falhas = 0, n_feitas = 0;

static void unit_check(const char *q, int cond) {
    n_feitas++;
    if (!cond) n_falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

static int run_cmd(const char *cmd) {
    return system(cmd);
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

static void semeia_rodata(unsigned char *mem, const char *tag) {
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

static int test_backend(const char *backend, const char *script, const char *expect, const char *rodata) {
    char wasm[512], corre[512], fita_ext[512], mem[512], cmd[1024];
    char dir[256];
    snprintf(wasm, sizeof wasm, "assets/figuras/wasm/%s.wasm", backend);
    snprintf(corre, sizeof corre, "conecthus/backends/%s/%s_corre.erg", backend, backend);
    snprintf(fita_ext, sizeof fita_ext, ".torre/cfm_%s_fita.bin", backend);
    snprintf(dir, sizeof dir, ".torre/reino_cfm_%s", backend);
    snprintf(mem, sizeof mem, "%s/mem.dat", dir);

    snprintf(cmd, sizeof cmd, "mkdir \"%s\" 2>nul & mkdir .torre 2>nul", dir);
    run_cmd(cmd);

    snprintf(cmd, sizeof cmd,
             "tools\\bin\\wasm_sec.exe extrai \"%s\" erg.fita \"%s\"", wasm, fita_ext);
    if (run_cmd(cmd) != 0) return 0;

    seed_script(arena_buf, script);
    arena_para_mem(arena_buf, membuf);
    semeia_rodata(membuf, rodata);
    FILE *mf = fopen(mem, "wb");
    if (!mf) return 0;
    fwrite(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);

    semeia_consts("tools\\bin\\erg_new.exe", mem, corre);

    snprintf(cmd, sizeof cmd,
             "tools\\bin\\erg_new.exe corre \"%s\" \"%s\" 2000000", fita_ext, mem);
    if (run_cmd(cmd) != 0) return 0;

    mf = fopen(mem, "rb");
    if (!mf) return 0;
    fread(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);
    mem_para_arena(membuf, arena_buf);

    int n = arena_buf[OFF_NOUT] + arena_buf[OFF_NOUT + 1] * 256;
    char out[CAP + 1];
    if (n > CAP) n = CAP;
    memcpy(out, arena_buf + OFF_OUT, (size_t)n);
    out[n] = 0;
    return strstr(out, expect) != NULL;
}

int main(void) {
    unit_check("IF0 wasm_sec disponível", 1);

    unit_check("IF1 node fita embutida → stdout 42",
               test_backend("node", "console.log('42')", "42", "console.log("));

    unit_check("IF2 bash fita embutida → stdout 42",
               test_backend("bash", "echo 42\n", "42", "echo "));

    unit_check("IF3 powershell fita embutida → stdout 42",
               test_backend("powershell", "Write-Output 42\n", "42", "Write-Output "));

    printf("#TOTAL %d %d\n", n_feitas, n_falhas);
    return n_falhas ? 1 : 0;
}
