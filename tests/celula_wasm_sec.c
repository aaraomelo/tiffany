/* celula_wasm_sec.c — wasm pós-gera_nucleo traz celula.erg + erg.fita embutidos.
 *
 *   gcc -O2 -std=c99 -w tests/celula_wasm_sec.c -o tools/bin/celula_wasm_sec.exe
 *   tools/bin/celula_wasm_sec.exe
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int dif_files(const char *a, const char *b) {
    long sa, sb;
    char *A = read_all(a, &sa);
    char *B = read_all(b, &sb);
    if (!A || !B) { free(A); free(B); return -1; }
    int d = (sa != sb) || memcmp(A, B, (size_t)sa) != 0;
    free(A);
    free(B);
    return d;
}

typedef struct {
    const char *nome;
    const char *wasm;
    const char *celula;
    const char *corre;
    const char *sec;
    const char *move;
} Backend;

static void test_backend(const Backend *b) {
    char q[128], cmd[1024];
    char erg_wasm[256], fita_wasm[256], fita_corre[256];
    long sz_wasm;

    snprintf(q, sizeof q, "CW0 %s wasm existe", b->nome);
    unit_check(q, file_size(b->wasm) > 500);

    sz_wasm = file_size(b->wasm);

    snprintf(erg_wasm, sizeof erg_wasm, ".torre/cw_%s.erg", b->nome);
    snprintf(fita_wasm, sizeof fita_wasm, ".torre/cw_%s_fita.bin", b->nome);
    snprintf(fita_corre, sizeof fita_corre, ".torre/cw_%s_corre_fita.bin", b->nome);

    snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_sec.exe extrai \"%s\" %s \"%s\"",
             b->wasm, b->sec, erg_wasm);
    snprintf(q, sizeof q, "CW1 %s extrai %s", b->nome, b->sec);
    unit_check(q, run_cmd(cmd) == 0 && file_size(erg_wasm) > 500);

    snprintf(q, sizeof q, "CW1 %s erg tem %s", b->nome, b->move);
    {
        long esz;
        char *erg_txt = read_all(erg_wasm, &esz);
        unit_check(q, erg_txt && buf_has(erg_txt, esz, b->move));
        free(erg_txt);
    }

    snprintf(cmd, sizeof cmd, "tools\\bin\\wasm_sec.exe extrai \"%s\" erg.fita \"%s\"",
             b->wasm, fita_wasm);
    snprintf(q, sizeof q, "CW2 %s extrai erg.fita", b->nome);
    unit_check(q, run_cmd(cmd) == 0 && file_size(fita_wasm) > 0);

    snprintf(cmd, sizeof cmd, "tools\\bin\\erg_new.exe monta \"%s\" \"%s\"",
             b->corre, fita_corre);
    run_cmd(cmd);
    snprintf(q, sizeof q, "CW2 %s fita wasm = monta(%s_corre)", b->nome, b->nome);
    unit_check(q, dif_files(fita_wasm, fita_corre) == 0);

    snprintf(q, sizeof q, "CW3 %s wasm cresceu com secções", b->nome);
    unit_check(q, sz_wasm > 5000);

    snprintf(q, sizeof q, "CW4 %s desce ≈ celula.erg", b->nome);
    unit_check(q, dif_files(erg_wasm, b->celula) == 0);
}

int main(void) {
    static const Backend BACKENDS[] = {
        { "node", "assets/figuras/wasm/node.wasm",
          "conecthus/backends/node/celula.erg",
          "conecthus/backends/node/node_corre.erg", "node.erg", "node_move" },
        { "bash", "assets/figuras/wasm/bash.wasm",
          "conecthus/backends/bash/celula.erg",
          "conecthus/backends/bash/bash_corre.erg", "bash.erg", "bash_move" },
        { "powershell", "assets/figuras/wasm/powershell.wasm",
          "conecthus/backends/powershell/celula.erg",
          "conecthus/backends/powershell/powershell_corre.erg", "powershell.erg", "powershell_move" },
    };

    run_cmd("mkdir .torre 2>nul");
    unit_check("CW0 wasm_sec disponível", file_size("tools/bin/wasm_sec.exe") > 0);

    for (int i = 0; i < 3; i++)
        test_backend(&BACKENDS[i]);

    printf("#TOTAL %d %d\n", n_feitas, n_falhas);
    return n_falhas ? 1 : 0;
}
