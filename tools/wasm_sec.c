/* wasm_sec.c — secções custom wasm (node.erg, erg.fita) em C puro.
 *   wasm_sec embute <wasm_in> <payload> <sec_name> <wasm_out>
 *   wasm_sec extrai <wasm_in> <sec_name> <out>
 *   wasm_sec cadeia <node|bash|powershell>
 *   wasm_sec asm_sobe <fonte.erg> <isa.wasm> <saida.wasm>
 *   wasm_sec asm_desce <modulo.wasm> <saida.erg>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef ERG_BIN
#define ERG_BIN "tools\\bin\\erg_new.exe"
#endif

#define SEC_FITA "erg.fita"

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

static int leb_write(unsigned char *out, int cap, uint64_t v) {
    int n = 0;
    do {
        if (n >= cap) return -1;
        unsigned char b = (unsigned char)(v & 0x7f);
        v >>= 7;
        if (v) b |= 0x80;
        out[n++] = b;
    } while (v);
    return n;
}

static unsigned char *read_file(const char *path, long *out_sz) {
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

static int write_file(const char *path, const unsigned char *buf, long sz) {
    FILE *f = fopen(path, "wb");
    if (!f) return -1;
    if (fwrite(buf, 1, (size_t)sz, f) != (size_t)sz) { fclose(f); return -1; }
    fclose(f);
    return 0;
}

static int run_cmd(const char *cmd) {
    return system(cmd);
}

static unsigned char *append_custom(const unsigned char *wasm, long wn,
                                    const char *name, const unsigned char *payload, long pn,
                                    long *out_sz) {
    if (wn < 8 || wasm[0] != 0 || wasm[1] != 0x61 || wasm[2] != 0x73 || wasm[3] != 0x6d) {
        fprintf(stderr, "não é wasm\n");
        return NULL;
    }
    int nlen = (int)strlen(name);
    unsigned char nleb[8];
    int nn = leb_write(nleb, 8, (uint64_t)nlen);
    long corpo_len = (long)nn + nlen + pn;
    unsigned char cleb[8];
    int cn = leb_write(cleb, 8, (uint64_t)corpo_len);
    long sec_len = 1 + (long)cn + corpo_len;
    unsigned char *out = (unsigned char *)malloc((size_t)wn + (size_t)sec_len);
    if (!out) return NULL;
    memcpy(out, wasm, (size_t)wn);
    long o = wn;
    out[o++] = 0;
    memcpy(out + o, cleb, (size_t)cn);
    o += cn;
    memcpy(out + o, nleb, (size_t)nn);
    o += nn;
    memcpy(out + o, name, (size_t)nlen);
    o += nlen;
    memcpy(out + o, payload, (size_t)pn);
    o += pn;
    if (out_sz) *out_sz = o;
    return out;
}

static unsigned char *append_custom_chain(unsigned char *wasm, long wn,
                                          const char *name, const unsigned char *payload, long pn,
                                          long *out_sz) {
    unsigned char *next = append_custom(wasm, wn, name, payload, pn, out_sz);
    if (wasm && next != wasm) free(wasm);
    return next;
}

static unsigned char *extrai_custom(const unsigned char *wasm, long wn, const char *name,
                                    long *out_sz) {
    if (wn < 8) return NULL;
    int nlen = (int)strlen(name);
    int p = 8;
    while (p < wn) {
        int id = wasm[p++];
        Leb sz = leb_read(wasm, (int)wn, p);
        p = sz.next;
        int body = p;
        p += sz.v;
        if (id != 0) continue;
        Leb nl = leb_read(wasm, (int)wn, body);
        if (nl.v != nlen) continue;
        if (memcmp(wasm + nl.next, name, (size_t)nlen) != 0) continue;
        int payload = nl.next + nlen;
        int plen = body + sz.v - payload;
        if (plen < 0) return NULL;
        unsigned char *out = (unsigned char *)malloc((size_t)plen + 1);
        if (!out) return NULL;
        memcpy(out, wasm + payload, (size_t)plen);
        out[plen] = 0;
        if (out_sz) *out_sz = plen;
        return out;
    }
    return NULL;
}

static int custom_present(const unsigned char *wasm, long wn, const char *name) {
    long sz;
    unsigned char *p = extrai_custom(wasm, wn, name, &sz);
    if (p) free(p);
    return p != NULL;
}

static unsigned char *monta_fita(const char *erg_path, long *out_sz) {
    char fita_path[512];
    char cmd[768];
    snprintf(fita_path, sizeof fita_path, "%s.fita.tmp", erg_path);
    snprintf(cmd, sizeof cmd, "%s monta \"%s\" \"%s\"", ERG_BIN, erg_path, fita_path);
    if (run_cmd(cmd) != 0) return NULL;
    return read_file(fita_path, out_sz);
}

static int cmd_embute(int argc, char **argv) {
    if (argc < 6) {
        fprintf(stderr, "uso: wasm_sec embute <wasm_in> <payload> <sec_name> <wasm_out>\n");
        return 2;
    }
    long wn, en, out_sz;
    unsigned char *wasm = read_file(argv[2], &wn);
    unsigned char *payload = read_file(argv[3], &en);
    if (!wasm || !payload) { perror("read"); return 1; }
    unsigned char *out = append_custom(wasm, wn, argv[4], payload, en, &out_sz);
    free(wasm);
    free(payload);
    if (!out) return 1;
    if (write_file(argv[5], out, out_sz) != 0) { perror(argv[5]); free(out); return 1; }
    free(out);
    return 0;
}

static int cmd_extrai(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "uso: wasm_sec extrai <wasm_in> <sec_name> <out>\n");
        return 2;
    }
    long wn, esz;
    unsigned char *wasm = read_file(argv[2], &wn);
    if (!wasm) { perror(argv[2]); return 1; }
    unsigned char *payload = extrai_custom(wasm, wn, argv[3], &esz);
    free(wasm);
    if (!payload) {
        fprintf(stderr, "secção «%s» em falta\n", argv[3]);
        return 1;
    }
    if (write_file(argv[4], payload, esz) != 0) { perror(argv[4]); free(payload); return 1; }
    free(payload);
    return 0;
}

typedef struct { const char *wasm; const char *erg; const char *corre; const char *sec; } Backend;

static const Backend BACKENDS[] = {
    { "assets/figuras/wasm/node.wasm", "conecthus/backends/node/celula.erg",
      "conecthus/backends/node/node_corre.erg", "node.erg" },
    { "assets/figuras/wasm/bash.wasm", "conecthus/backends/bash/celula.erg",
      "conecthus/backends/bash/bash_corre.erg", "bash.erg" },
    { "assets/figuras/wasm/powershell.wasm", "conecthus/backends/powershell/celula.erg",
      "conecthus/backends/powershell/powershell_corre.erg", "powershell.erg" },
    { NULL, NULL, NULL, NULL }
};

static const Backend *achar_backend(const char *nome) {
    for (int i = 0; BACKENDS[i].wasm; i++) {
        if (!strcmp(nome, "node") && strstr(BACKENDS[i].wasm, "node.wasm")) return &BACKENDS[i];
        if (!strcmp(nome, "bash") && strstr(BACKENDS[i].wasm, "bash.wasm")) return &BACKENDS[i];
        if (!strcmp(nome, "powershell") && strstr(BACKENDS[i].wasm, "powershell.wasm")) return &BACKENDS[i];
    }
    return NULL;
}

static int atomically_replace(const char *dest, unsigned char *buf, long sz) {
    char tmp[512];
    snprintf(tmp, sizeof tmp, "%s.tmp", dest);
    if (write_file(tmp, buf, sz) != 0) { perror(tmp); return 1; }
    if (remove(dest) != 0) { perror("remove"); return 1; }
    if (rename(tmp, dest) != 0) { perror("rename"); return 1; }
    return 0;
}

static int cmd_cadeia(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "uso: wasm_sec cadeia <node|bash|powershell>\n");
        return 2;
    }
    const Backend *b = achar_backend(argv[2]);
    if (!b) { fprintf(stderr, "backend «%s» desconhecido\n", argv[2]); return 1; }
    long wn, en, out_sz, fita_sz = 0;
    unsigned char *wasm = read_file(b->wasm, &wn);
    unsigned char *erg = read_file(b->erg, &en);
    if (!wasm) { fprintf(stderr, "wasm em falta: %s\n", b->wasm); return 1; }
    if (!erg) { fprintf(stderr, "erg em falta: %s\n", b->erg); return 1; }
    unsigned char *out = append_custom(wasm, wn, b->sec, erg, en, &out_sz);
    free(wasm);
    free(erg);
    if (!out) return 1;
    unsigned char *fita = monta_fita(b->corre, &fita_sz);
    if (fita && fita_sz > 0) {
        out = append_custom_chain(out, out_sz, SEC_FITA, fita, fita_sz, &out_sz);
        free(fita);
    }
    if (atomically_replace(b->wasm, out, out_sz) != 0) { free(out); return 1; }
    fprintf(stderr, "wasm_sec: %s+%s embutidos em %s (+%ld bytes)\n",
            b->sec, fita_sz > 0 ? SEC_FITA : "—", b->wasm, out_sz - wn);
    free(out);
    return 0;
}

static int cmd_asm_sobe(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr, "uso: wasm_sec asm_sobe <fonte.erg> <isa.wasm> <saida.wasm>\n");
        return 2;
    }
    long wn, fita_sz, out_sz;
    unsigned char *wasm = read_file(argv[3], &wn);
    unsigned char *fita = monta_fita(argv[2], &fita_sz);
    if (!wasm) { fprintf(stderr, "isa wasm em falta: %s\n", argv[3]); free(fita); return 1; }
    if (!fita || fita_sz <= 0) { fprintf(stderr, "monta falhou: %s\n", argv[2]); free(wasm); return 1; }
    unsigned char *out = append_custom(wasm, wn, SEC_FITA, fita, fita_sz, &out_sz);
    free(wasm);
    free(fita);
    if (!out) return 1;
    if (write_file(argv[4], out, out_sz) != 0) { perror(argv[4]); free(out); return 1; }
    fprintf(stderr, "wasm_sec: asm_sobe %s → %s (%ldB fita)\n", argv[2], argv[4], fita_sz);
    free(out);
    return 0;
}

static int cmd_asm_desce(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "uso: wasm_sec asm_desce <modulo.wasm> <saida.erg>\n");
        return 2;
    }
    long wn, fsz;
    unsigned char *wasm = read_file(argv[2], &wn);
    if (!wasm) { perror(argv[2]); return 1; }
    unsigned char *fita = extrai_custom(wasm, wn, SEC_FITA, &fsz);
    free(wasm);
    if (!fita) { fprintf(stderr, "secção «%s» em falta\n", SEC_FITA); return 1; }
    char fita_path[512];
    char cmd[768];
    snprintf(fita_path, sizeof fita_path, "%s.fita.tmp", argv[3]);
    if (write_file(fita_path, fita, fsz) != 0) { perror(fita_path); free(fita); return 1; }
    free(fita);
    snprintf(cmd, sizeof cmd, "%s desmonta \"%s\" > \"%s\"", ERG_BIN, fita_path, argv[3]);
    if (run_cmd(cmd) != 0) { fprintf(stderr, "desmonta falhou\n"); return 1; }
    fprintf(stderr, "wasm_sec: asm_desce %s → %s\n", argv[2], argv[3]);
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "uso: wasm_sec embute|extrai|cadeia|asm_sobe|asm_desce ...\n");
        return 2;
    }
    if (!strcmp(argv[1], "embute")) return cmd_embute(argc, argv);
    if (!strcmp(argv[1], "extrai")) return cmd_extrai(argc, argv);
    if (!strcmp(argv[1], "cadeia")) return cmd_cadeia(argc, argv);
    if (!strcmp(argv[1], "asm_sobe")) return cmd_asm_sobe(argc, argv);
    if (!strcmp(argv[1], "asm_desce")) return cmd_asm_desce(argc, argv);
    fprintf(stderr, "comando «%s» desconhecido\n", argv[1]);
    return 2;
}
