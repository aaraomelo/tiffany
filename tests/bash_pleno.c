/* bash_pleno.c — mede o pleno bash (popen) no metal.
 *
 *   cc -O2 -std=c99 -w tests/bash_pleno.c conecthus/backends/bash/interpretar.c banco/bash.c -o /tmp/bash_pleno
 *   /tmp/bash_pleno
 */
#include <stdio.h>
#include <string.h>

extern unsigned char arena[65536];

int bash_escreve(int in_off, int n);
int bash_le(int out_off, int max);
int bash_pronto(void);
int bash_pendente(void);
int bash_corre(void);

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++;
    if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

int main(void){
#ifdef _WIN32
    const char *cmd = "echo ouro";
#else
    const char *cmd = "echo ouro\n";
#endif
    int n = (int)strlen(cmd);
    int i = 0;
    while(i < n){
        arena[128 + i] = (unsigned char)cmd[i];
        i = i + 1;
    }
    ok("escreve aceita echo ouro", bash_escreve(128, n) == n);
    ok("stdin pendente", bash_pendente() == n);
    ok("stdout vazio antes", bash_pronto() == 0);

    int got = bash_corre();
    ok("corre devolve bytes", got > 0);
    ok("stdin limpo depois", bash_pendente() == 0);
    ok("stdout pronto", bash_pronto() == got);

    char out[64];
    int m = bash_le(0, 63);
    if(m >= 63) m = 62;
    out[m] = 0;
    i = 0;
    while(i < m){ out[i] = (char)arena[i]; i = i + 1; }
    ok("saida contem ouro", strstr(out, "ouro") != 0);

    printf("#TOTAL %d %d\n", feitas, falhas);
    return falhas ? 1 : 0;
}
