#include <stdio.h>
#include <string.h>

int bash_corre(void);

int main(void) {
    extern unsigned char arena[65536];
    const char *script = "echo bench\n";
    int n = (int)strlen(script);
    memset(arena, 0, 65536);
    memcpy(arena + 256, script, (size_t)n);
    arena[24576] = (unsigned char)(n & 255);
    arena[24577] = (unsigned char)((n >> 8) & 255);
    int r = bash_corre();
    int nout = arena[24578] + arena[24579] * 256;
    printf("ret=%d nout=%d out=%.*s\n", r, nout, nout, arena + 16384);
    return 0;
}
