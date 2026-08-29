/* traduz_c_asm_shell.c — cadeia C → assembly → metal (bash + powershell, sem Node).
 *   interpretar.c → traduz → wasm → wasm_erg --all → celula.erg
 *   wasm_sec embute/extrai → *_corre.erg → erg corre
 *
 *   gcc -O2 -std=c99 -w tests/traduz_c_asm_shell.c -o tools/bin/traduz_c_asm_shell.exe
 *   tools/bin/traduz_c_asm_shell.exe
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

typedef struct {
    const char *nome;
    const char *tag;
    const char *fonte;
    const char *wasm;
    const char *celula;
    const char *corre;
    const char *fita;
    const char *move;
    const char *sec_erg;
    const char *rodata;
    const char *script;
    const char *expect;
    const char *volta;
    const char *reino;
} Shell;

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

static int test_metal_shell(const Shell *sh, const char *erg_bin) {
    char mem[512], cmd[1024];
    snprintf(cmd, sizeof cmd, "mkdir \"%s\" 2>nul", sh->reino);
    run_cmd(cmd);
    snprintf(mem, sizeof mem, "%s/mem.dat", sh->reino);

    snprintf(cmd, sizeof cmd, "tools\\bin\\traduz.exe \"%s\" -o \"%s\"", sh->fonte, sh->wasm);
    if (run_cmd(cmd) != 0) return 0;

    char export_name[64];
    snprintf(export_name, sizeof export_name, "%s_corre", sh->nome);
    snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_erg.exe \"%s\" %s \"%s\"",
             sh->wasm, export_name, sh->corre);
    if (run_cmd(cmd) != 0) return 0;

    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", erg_bin, sh->corre, sh->fita);
    if (run_cmd(cmd) != 0) return 0;

    unsigned char *arena = arena_metal;
    unsigned char *membuf = membuf_metal;
    seed_script(arena, sh->script);
    arena_para_mem(arena, membuf);
    semeia_rodata(membuf, sh->rodata);
    FILE *mf = fopen(mem, "wb");
    if (!mf) return 0;
    fwrite(membuf, 1, MEM_SLOTS * 2, mf);
    fclose(mf);

    semeia_consts(erg_bin, mem, sh->corre);

    snprintf(cmd, sizeof cmd, "%s corre \"%s\" \"%s\" 2000000", erg_bin, sh->fita, mem);
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
    return strstr(out, sh->expect) != NULL;
}

static void test_shell(const Shell *sh) {
    const char *traduz = "tools\\bin\\traduz.exe";
    const char *wasm_erg = "tools\\bin\\wasm_erg.exe";
    char cmd[1024];
    char q[128];

    snprintf(q, sizeof q, "%s0 %s interpretar.c", sh->tag, sh->nome);
    unit_check(q, file_exists(sh->fonte));

    snprintf(q, sizeof q, "%s0 traduz disponível", sh->tag);
    unit_check(q, file_exists(traduz));

    snprintf(q, sizeof q, "%s0 wasm_erg disponível", sh->tag);
    unit_check(q, file_exists(wasm_erg));

    snprintf(cmd, sizeof cmd, "mkdir assets\\figuras\\wasm 2>nul");
    run_cmd(cmd);

    snprintf(cmd, sizeof cmd, "%s \"%s\" -o \"%s\"", traduz, sh->fonte, sh->wasm);
    snprintf(q, sizeof q, "%s1 sobe(%s) → wasm", sh->tag, sh->nome);
    unit_check(q, run_cmd(cmd) == 0 && file_size(sh->wasm) > 100);

    snprintf(q, sizeof q, "%s1 export %s", sh->tag, sh->move);
    unit_check(q, wasm_has_export(sh->wasm, sh->move));

    snprintf(cmd, sizeof cmd, "%s \"%s\" --all \"%s\"", wasm_erg, sh->wasm, sh->celula);
    snprintf(q, sizeof q, "%s2 wasm→assembly (--all)", sh->tag);
    unit_check(q, run_cmd(cmd) == 0 && file_size(sh->celula) > 500);

    snprintf(q, sizeof q, "%s2 assembly menciona %s", sh->tag, sh->move);
    unit_check(q, wasm_has_export(sh->celula, sh->move));

    snprintf(q, sizeof q, "%s2 assembly tem LOAD/STORE/HALT", sh->tag);
    unit_check(q, erg_has_opcode(sh->celula, "LOAD") &&
                  erg_has_opcode(sh->celula, "STORE") &&
                  erg_has_opcode(sh->celula, "HALT"));

    {
        char *t = read_all(sh->celula, NULL);
        int sem = t && strstr(t, "; wasm 0x") == NULL;
        snprintf(q, sizeof q, "%s2 sem stubs wasm 0x", sh->tag);
        unit_check(q, sem);
        free(t);
    }

    {
        long sz0 = file_size(sh->wasm);
        snprintf(cmd, sizeof cmd,
                 "tools\\bin\\wasm_sec.exe embute \"%s\" \"%s\" %s \"%s\"",
                 sh->wasm, sh->celula, sh->sec_erg, sh->wasm);
        snprintf(q, sizeof q, "%s3 embute %s", sh->tag, sh->sec_erg);
        unit_check(q, run_cmd(cmd) == 0 && file_size(sh->wasm) > sz0);

        snprintf(q, sizeof q, "%s3 wasm cresceu com secção", sh->tag);
        unit_check(q, file_size(sh->wasm) > sz0 + 100);

        snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_sec.exe extrai \"%s\" %s \"%s\"",
                 sh->wasm, sh->sec_erg, sh->volta);
        snprintf(q, sizeof q, "%s4 extrai %s", sh->tag, sh->sec_erg);
        unit_check(q, run_cmd(cmd) == 0 && file_size(sh->volta) > 500);

        snprintf(q, sizeof q, "%s4 desce preserva %s", sh->tag, sh->move);
        unit_check(q, wasm_has_export(sh->volta, sh->move));

        snprintf(q, sizeof q, "%s4 desce ≈ celula.erg", sh->tag);
        unit_check(q, dif_files(sh->volta, sh->celula) == 0);
    }

    snprintf(q, sizeof q, "%s5 metal %s_corre stdout %s", sh->tag, sh->nome, sh->expect);
    unit_check(q, test_metal_shell(sh, "tools\\bin\\erg_new.exe"));
}

static int manifesto_bash_asm(void) {
    char *txt = read_all("conecthus/backends/manifesto.json", NULL);
    if (!txt) return 0;
    int ok1 = strstr(txt, "conecthus/backends/bash/celula.erg") != NULL &&
              strstr(txt, "tools/wasm_sec.c") != NULL &&
              strstr(txt, "\"asm\"") != NULL;
    free(txt);
    return ok1;
}

static int manifesto_medidor_shell(void) {
    char *txt = read_all("conecthus/backends/manifesto.json", NULL);
    if (!txt) return 0;
    int ok1 = strstr(txt, "traduz_c_asm_shell.c") != NULL;
    free(txt);
    return ok1;
}

int main(void) {
    static const Shell BACKENDS[] = {
        {
            "bash", "CB",
            "conecthus/backends/bash/interpretar.c",
            "assets/figuras/wasm/bash.wasm",
            "conecthus/backends/bash/celula.erg",
            "conecthus/backends/bash/bash_corre.erg",
            "conecthus/backends/bash/bash_corre.fita.bin",
            "bash_move", "bash.erg", "echo ",
            "echo 42\n", "42",
            ".torre/volta_bash.erg",
            ".torre/reino_asm_bash",
        },
        {
            "powershell", "CP",
            "conecthus/backends/powershell/interpretar.c",
            "assets/figuras/wasm/powershell.wasm",
            "conecthus/backends/powershell/celula.erg",
            "conecthus/backends/powershell/powershell_corre.erg",
            "conecthus/backends/powershell/powershell_corre.fita.bin",
            "powershell_move", "powershell.erg", "Write-Output ",
            "Write-Output 42\n", "42",
            ".torre/volta_ps.erg",
            ".torre/reino_asm_ps",
        },
    };

    run_cmd("mkdir .torre 2>nul");

    unit_check("CAS0 shells bash+powershell", file_exists(BACKENDS[0].fonte) &&
                                               file_exists(BACKENDS[1].fonte));
    unit_check("CAS0 wasm_sec disponível", file_exists("tools\\bin\\wasm_sec.exe"));

    for (int i = 0; i < 2; i++)
        test_shell(&BACKENDS[i]);

    unit_check("CAS6 bash cadeia asm/wasm_sec", manifesto_bash_asm());
    unit_check("CAS7 medidor traduz_c_asm_shell.c", manifesto_medidor_shell());

    printf("#TOTAL %d %d\n", n_feitas, n_falhas);
    return n_falhas ? 1 : 0;
}
