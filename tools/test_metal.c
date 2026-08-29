/* test_metal.c — valida cadeia física sem Node: C→wasm→ERG→erg corre.
 *   gcc -O2 -std=c99 -w tools/test_metal.c -o tools/bin/test_metal.exe
 *   tools/bin/test_metal.exe */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define ARENA_SIZE 65536
#define NULO_DISCO 8
#define MEM_SLOTS 65500
#define RODATA_RELOC_BASE 65400
#define RODATA_TAG (RODATA_RELOC_BASE + 8)  /* wasm 65544 */
#define OFF_NIN 24576
#define OFF_NOUT 24578
#define OFF_IN 256
#define OFF_OUT 16384
#define CAP 8192

static void arena_para_mem(const unsigned char *arena, unsigned char *mem) {
    memset(mem, 0, MEM_SLOTS * 2);
    for (int s = 0; s < ARENA_SIZE; s++)
        mem[(NULO_DISCO + s) * 2] = arena[s];
}

/* rodata wasm (tag em 65544) — mesma figura que o módulo MOVE lê. */
static void semeia_rodata(unsigned char *mem, const char *backend) {
    const char *tag = NULL;
    if (!strcmp(backend, "node")) tag = "console.log(";
    else if (!strcmp(backend, "bash")) tag = "echo ";
    else if (!strcmp(backend, "powershell")) tag = "Write-Output ";
    if (!tag) return;
    int off = RODATA_TAG;
    for (int i = 0; tag[i]; i++)
        mem[(NULO_DISCO + off + i) * 2] = (unsigned char)tag[i];
}

static void mem_para_arena(const unsigned char *mem, unsigned char *arena) {
    for (int s = 0; s < ARENA_SIZE; s++)
        arena[s] = mem[(NULO_DISCO + s) * 2];
}

static void seed_script(unsigned char *arena, const char *script) {
    memset(arena, 0, ARENA_SIZE);
    int n = (int)strlen(script);
    if (n > CAP) n = CAP;
    memcpy(arena + OFF_IN, script, (size_t)n);
    arena[OFF_NIN] = (unsigned char)(n & 255);
    arena[OFF_NIN + 1] = (unsigned char)((n >> 8) & 255);
}

static int read_stdout_len(const unsigned char *arena, char *out, int cap) {
    int n = arena[OFF_NOUT] + arena[OFF_NOUT + 1] * 256;
    if (n > CAP) n = CAP;
    if (n >= cap) n = cap - 1;
    memcpy(out, arena + OFF_OUT, (size_t)n);
    out[n] = 0;
    return n;
}

static int run_cmd(const char *cmd) {
    int rc = system(cmd);
    return rc;
}

static long run_corre_passos(const char *cmd) {
    FILE *p = _popen(cmd, "r");
    if (!p) return -1;
    char line[256];
    long passos = -1;
    while (fgets(line, sizeof line, p)) {
        long n;
        if (sscanf(line, "%ld passos", &n) == 1) passos = n;
    }
    _pclose(p);
    return passos;
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

static int test_backend(const char *erg_bin, const char *backend, const char *script,
                        const char *expect, long teto_passos) {
    char wasm[512], erg[512], fita[512], mem[512], cmd[1024];
    snprintf(wasm, sizeof wasm, "assets/figuras/wasm/%s.wasm", backend);
    snprintf(erg, sizeof erg, "conecthus/backends/%s/%s_corre.erg", backend, backend);
    snprintf(fita, sizeof fita, "conecthus/backends/%s/%s_corre.fita.bin", backend, backend);
    char dir[256];
    snprintf(dir, sizeof dir, ".torre/reino_test_metal_%s", backend);
    snprintf(cmd, sizeof cmd, "mkdir \"%s\" 2>nul", dir);
    run_cmd(cmd);
    snprintf(mem, sizeof mem, "%s/mem.dat", dir);

    snprintf(cmd, sizeof cmd, "tools\\bin\\traduz.exe conecthus\\backends\\%s\\interpretar.c -o %s", backend, wasm);
    if (run_cmd(cmd) != 0) { printf("#UNIT falha traduz %s\n", backend); return 0; }

    snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_erg.exe %s %s_corre %s", wasm, backend, erg);
    if (run_cmd(cmd) != 0) { printf("#UNIT falha wasm_erg %s\n", backend); return 0; }

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", erg_bin, erg, fita);
    if (run_cmd(cmd) != 0) { printf("#UNIT falha monta %s\n", backend); return 0; }

    unsigned char arena[ARENA_SIZE];
    static unsigned char membuf[MEM_SLOTS * 2];
    seed_script(arena, script);
    arena_para_mem(arena, membuf);
    semeia_rodata(membuf, backend);
    FILE *mf = fopen(mem, "wb");
    if (!mf) { perror(mem); return 0; }
    fwrite(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);

    semeia_consts(erg_bin, mem, erg);

    snprintf(cmd, sizeof cmd, "%s corre \"%s\" \"%s\" 2000000", erg_bin, fita, mem);
    long passos = run_corre_passos(cmd);
    if (passos < 0) { printf("#UNIT falha corre passos %s\n", backend); return 0; }

    mf = fopen(mem, "rb");
    if (!mf) return 0;
    fread(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);
    mem_para_arena(membuf, arena);

    char out[CAP + 1];
    read_stdout_len(arena, out, sizeof out);
    int ok_out = strstr(out, expect) != NULL;
    int ok_passos = passos > 0 && passos <= teto_passos;
    printf("#UNIT %s %s_corre stdout «%s» expect «%s» (%ld passos, teto %ld)\n",
           ok_out && ok_passos ? "ok" : "falha", backend, out, expect, passos, teto_passos);
    return ok_out && ok_passos;
}

int main(void) {
    int falhas = 0, feitas = 0;
    const char *erg = "tools\\bin\\erg_new.exe";

    feitas++;
    if (test_backend(erg, "node", "console.log('metal')", "metal", 3000)) { }
    else falhas++;

    feitas++;
    if (test_backend(erg, "bash", "echo bench\n", "bench", 5000)) { }
    else falhas++;

    feitas++;
    if (test_backend(erg, "powershell", "Write-Output bench\n", "bench", 5000)) { }
    else falhas++;

    printf("#TOTAL %d %d\n", feitas, falhas);
    return falhas ? 1 : 0;
}
