/* powershell_pleno.c — pleno PowerShell no metal (Windows).
 *
 *   cc -O2 -std=c99 -w tests/powershell_pleno.c conecthus/backends/powershell/interpretar.c banco/powershell.c -o powershell_pleno
 *   powershell_pleno
 */
#include <stdio.h>
#include <string.h>

extern unsigned char arena[65536];

int powershell_escreve(int in_off, int n);
int powershell_le(int out_off, int max);
int powershell_pronto(void);
int powershell_pendente(void);
int powershell_corre(void);

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++;
    if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
}

int main(void){
#ifndef _WIN32
    printf("#UNIT ok §P0 skip (nao-Windows)\n");
    printf("#TOTAL 1 0\n");
    return 0;
#else
    const char *cmd = "Write-Output ouro";
    int n = (int)strlen(cmd);
    int i = 0;
    while(i < n){
        arena[128 + i] = (unsigned char)cmd[i];
        i = i + 1;
    }
    ok("§P1 escreve script", powershell_escreve(128, n) == n);
    ok("§P1 pendente", powershell_pendente() == n);
    int got = powershell_corre();
    ok("§P1 corre", got > 0);
    ok("§P1 stdout pronto", powershell_pronto() == got);
    char out[64];
    int m = powershell_le(0, 63);
    if(m >= 63) m = 62;
    out[m] = 0;
    i = 0;
    while(i < m){ out[i] = (char)arena[i]; i = i + 1; }
    ok("§P1 saida contem ouro", strstr(out, "ouro") != 0);
    printf("#TOTAL %d %d\n", feitas, falhas);
    return falhas ? 1 : 0;
#endif
}
