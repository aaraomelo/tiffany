/* node_pleno.c — pleno Node no metal.
 *
 *   cc -O2 -std=c99 -w -DPLENO_NODE tests/node_pleno.c conecthus/backends/node/interpretar.c banco/node.c -o node_pleno
 *   set PATH com node; node_pleno
 */
#include <stdio.h>
#include <string.h>

extern unsigned char arena[65536];

int node_escreve(int in_off, int n);
int node_le(int out_off, int max);
int node_pronto(void);
int node_pendente(void);
int node_corre(void);

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++;
    if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

int main(void){
    const char *cmd = "console.log('ouro')";
    int n = (int)strlen(cmd);
    int i = 0;
    while(i < n){
        arena[128 + i] = (unsigned char)cmd[i];
        i = i + 1;
    }
    ok("§N1 escreve script", node_escreve(128, n) == n);
    ok("§N1 pendente", node_pendente() == n);
    int got = node_corre();
    if(got <= 0){
        ok("§N1 node no PATH (TIFFANY_NODE)", 0);
        printf("#TOTAL %d %d\n", feitas, falhas);
        return falhas ? 1 : 0;
    }
    ok("§N1 corre", got > 0);
    char out[64];
    int m = node_le(0, 63);
    if(m >= 63) m = 62;
    out[m] = 0;
    i = 0;
    while(i < m){ out[i] = (char)arena[i]; i = i + 1; }
    ok("§N1 saida contem ouro", strstr(out, "ouro") != 0);
    printf("#TOTAL %d %d\n", feitas, falhas);
    return falhas ? 1 : 0;
}
