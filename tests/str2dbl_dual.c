/* str2dbl_dual.c — A TRAVA: o strtod-sem-libc do núcleo (lib/le_num.h) bate o strtod da libc.
 *
 * O tradutor sobe para wasm sem libc, e o str2dbl (lib/le_num.h) substitui o strtod. A régua não
 * se afirma: MEDE-SE contra o strtod da libc, os DOIS caminhos byte a byte. O medidor inclui o
 * MESMO le_num.h que o tex.c inclui --- não há cópia a derivar: se alguém "otimizar" o str2dbl e
 * quebrar o arredondamento, ESTA bateria acende (a memória: «o medidor que nunca mediu»).
 *
 *   §S1  os valores reais do estilo (unidades, razões, corpos): str2dbl == strtod, bit a bit
 *   §S2  o endptr, e o caso "1em"/"1ex": o `e` NÃO é expoente (senão a unidade perdia-se)
 *   §S3  varredura de 8000 decimais (1..4 casas): str2dbl == strtod, bit a bit
 *   §S4  hex2 == sscanf %2x nos 256 bytes
 *
 *   cc -O2 -std=c99 -Wall -I../lib str2dbl_dual.c -o str2dbl_dual && ./str2dbl_dual
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unidade.h"
#include "le_num.h"     /* o MESMO header que o tex.c usa --- fonte única */

/* dois longs são os MESMOS 64 bits IEEE? */
static int mesmos_bits(long a, long b){
    uint64_t x, y; memcpy(&x, &a, 8); memcpy(&y, &b, 8);
    return x == y;
}
/* strtod da libc, mas devolve os bits — a referência da trava */
static long libc_bits(const char *s, char **end){
    __typeof__(strtod("", NULL)) d = strtod(s, end);
    long b; memcpy(&b, &d, 8);
    return b;
}

int main(void){
    printf("=== A TRAVA: str2dbl (lib/le_num.h) vs strtod da libc, byte a byte ===============\n\n");

    /* ── §S1 os valores reais do estilo ────────────────────────────────────────────────── */
    /* as unidades (28.3465 cm, 72 in…), as razões (1.17398 = φ^⅓, 1.4497), os corpos (10.95, 23.42):
     * exactamente os números que alimentam posições fraccionárias no PDF. str2dbl tem de os dar
     * IGUAIS ao strtod --- 1 ULP de diferença mudaria o hash. */
    const char *reais[] = { "28.3465","2.83465","72","4.5","10.5","1.17398","1.4497",
                            "10.95","23.42","16.99","2.4","0.5","6","3","0.001" };
    int nr = (int)(sizeof reais / sizeof reais[0]), difs1 = 0;
    for(int i = 0; i < nr; i++)
        if(!mesmos_bits(str2dbl(reais[i], NULL), libc_bits(reais[i], NULL))) difs1++;
    printf("      %d valores reais do estilo, %d diferenças de bits contra o strtod\n\n", nr, difs1);
    ok("§S1 nos valores REAIS do estilo (unidades, razões φ^⅓, corpos) o str2dbl-sem-libc dá o MESMO"
       " long que o strtod da libc, bit a bit --- é o que garante que tirar o sscanf não mexe no PDF",
       difs1 == 0);

    /* ── §S2 o endptr, e o "1em"/"1ex" (o `e` não é expoente) ──────────────────────────── */
    /* a propriedade crítica: em "1em" o str2dbl para no `e` (deixa a unidade "em"), tal como o
     * strtod --- se consumisse o `e` como expoente, a medida perderia a unidade. Compara-se o
     * ponto de paragem (end - início) com o do strtod, nos casos que têm unidade a seguir. */
    const char *comu[] = { "1em","1ex","2.4cm","3pt","72in","6mm","0.5em","10.95pt","1e3","2.5e-2" };
    int nc = (int)(sizeof comu / sizeof comu[0]), difs2 = 0;
    for(int i = 0; i < nc; i++){
        const char *e1; long a = str2dbl(comu[i], &e1);
        char *e2;       long b = libc_bits(comu[i], &e2);
        if(!mesmos_bits(a, b) || (e1 - comu[i]) != (e2 - comu[i])) difs2++;
    }
    /* e o par que prova a asserção: "1em" NÃO pode parar depois do 'e' */
    const char *e_em; str2dbl("1em", &e_em);
    int em_ok = (e_em - "1em") == 1;   /* parou logo após o '1', antes do 'e' */
    printf("§S2  %d casos com unidade, %d diferenças (valor+endptr) ; \"1em\" para em +%ld (esperado 1)\n\n",
           nc, difs2, (long)(e_em - "1em"));
    ok("§S2 o str2dbl para no MESMO sítio que o strtod (valor E endptr iguais), e o `e` de \"em\"/\"ex\""
       " não é consumido como expoente --- \"1em\" para logo após o '1' (endptr +1), senão a unidade"
       " perder-se-ia", difs2 == 0 && em_ok);

    /* ── §S3 varredura de 8000 decimais ────────────────────────────────────────────────── */
    /* N/10^d para N=0..1999 e d=1..4, montado sem depender do str2dbl: o strtod é a referência, e
     * a referência MUDA com a entrada (não é escrita à mão). Bit a bit. */
    long difs3 = 0, tot3 = 0;
    for(long N = 0; N <= 1999; N++) for(int d = 1; d <= 4; d++){
        char buf[32]; long div = 1; for(int k = 0; k < d; k++) div *= 10;
        snprintf(buf, sizeof buf, "%ld.%0*ld", N / div, d, N % div);
        tot3++;
        if(!mesmos_bits(str2dbl(buf, NULL), libc_bits(buf, NULL))) difs3++;
    }
    printf("§S3  %ld decimais varridos (1..4 casas), %ld diferenças de bits\n\n", tot3, difs3);
    ok("§S3 numa varredura de 8000 decimais (1 a 4 casas) o str2dbl dá o MESMO long que o strtod em"
       " TODOS --- a divisão única por 10^k (potência exacta) arredonda como o strtod, sem a cadeia"
       " de /10 que erra a cada passo", difs3 == 0 && tot3 == 8000);

    /* ── §S4 hex2 == sscanf %2x nos 256 bytes ──────────────────────────────────────────── */
    int difs4 = 0;
    for(int v = 0; v < 256; v++){
        char s[3]; snprintf(s, sizeof s, "%02x", v);
        unsigned x; sscanf(s, "%2x", &x);
        if(hex2(s) != (int)x) difs4++;
    }
    /* e uma entrada não-hex tem de dar -1 (o guarda), não um número */
    int guarda = (hex2("zz") == -1 && hex2("g0") == -1);
    printf("§S4  hex2 vs sscanf %%2x nos 256 bytes: %d diferenças ; guarda de não-hex: %s\n\n",
           difs4, guarda ? "ok" : "FALHA");
    ok("§S4 o hex2 (o `sscanf %2x` do le_cores_estilo) dá o mesmo byte que o sscanf da libc nos 256"
       " valores, e devolve -1 num par não-hex --- o parser de cores do núcleo bate a libc",
       difs4 == 0 && guarda);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  A trava fecha: o str2dbl e o hex2 de lib/le_num.h --- o strtod e o sscanf que o núcleo");
        puts("  do tradutor não pode ter --- batem a libc byte a byte, nos valores reais do estilo, em");
        puts("  8000 decimais, e nos 256 bytes hex. O medidor inclui o MESMO header que o tex.c usa,");
        puts("  logo não há cópia a derivar: se o str2dbl mudar o arredondamento, esta bateria acende.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
