#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

static int leb(const unsigned char *b, int n, int p, int *v) {
    uint64_t x = 0;
    int s = 0, i = p;
    while (i < n) {
        unsigned char c = b[i++];
        x |= (uint64_t)(c & 0x7f) << s;
        if ((c & 0x80) == 0) { *v = (int)x; return i; }
        s += 7;
    }
    return -1;
}

int main(int argc, char **argv) {
    if (argc < 3) return 1;
    FILE *f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char *buf = malloc((size_t)sz);
    fread(buf, 1, (size_t)sz, f);
    fclose(f);
    int want = atoi(argv[2]);
    int p = 8, fi = 0;
    while (p < sz) {
        int id = buf[p++];
        int sl, np = leb(buf, (int)sz, p, &sl);
        if (np < 0) break;
        p = np;
        const unsigned char *body = buf + p;
        if (id == 10) {
            int nn, cp = leb(body, sl, 0, &nn);
            for (int k = 0; k < nn; k++) {
                int fs, cp2 = leb(body, sl, cp, &fs);
                const unsigned char *code = body + cp2;
                if (k == want) {
                    int ng, j = leb(code, fs, 0, &ng);
                    for (int g = 0; g < ng; g++) {
                        int cnt, j2 = leb(code, fs, j, &cnt);
                        j = j2 + 1;
                    }
                    printf("func %d code len %d start %d\n", k, fs, j);
                    while (j < fs) {
                        int op = code[j++];
                        printf("  @%d 0x%02x", j - 1, op);
                        if (op == 0x20 || op == 0x21 || op == 0x22 || op == 0x41 || op == 0x10) {
                            int v, j2 = leb(code, fs, j, &v);
                            printf(" %d", v);
                            j = j2;
                        } else if (op == 0x0c || op == 0x0d) {
                            int v, j2 = leb(code, fs, j, &v);
                            printf(" depth=%d", v);
                            j = j2;
                        } else if (op == 0x28 || op == 0x2d || op == 0x36 || op == 0x3a || op == 0x2c) {
                            j++;
                            int v, j2 = leb(code, fs, j, &v);
                            printf(" off=%d", v);
                            j = j2;
                        }
                        putchar('\n');
                    }
                    free(buf);
                    return 0;
                }
                cp = cp2 + fs;
            }
        }
        p += sl;
    }
    free(buf);
    return 0;
}
