/* dual16.c — 32 BITS SÃO DOIS DUAIS DE 16, medido contra a régua larga.
 *
 * §E0  produto 16×16 → par D32, contra int32
 * §E1  ordem por produto cruzado
 * §E2  soma e diferença do par
 * §E3  operações ⊕ e ⊗ no par racional — racionais.tex Def. def:ops
 *
 *   cc -O2 -std=c99 -I lib tests/dual16.c -o dual16 && ./dual16
 */
#include <stdio.h>
#include <stdint.h>
#include "dual16.h"
#include "unidade.h"

static int32_t largo(int16_t a, int16_t b){ return (int32_t)a * (int32_t)b; }

static int cmp32(int32_t x, int32_t y){ return x < y ? -1 : (x > y ? 1 : 0); }

int main(void){
    printf("\n=== 32 BITS SÃO DOIS DUAIS DE 16 ===\n");

    printf("\n§E0 O produto 16×16 como par, contra int32.\n\n");
    {
        long mal = 0, casos = 0;
        const int V[] = { 0, 1, 2, 127, 255, 256, 1000, 2000, 3000, 32767, -1, -2, -32768 };
        int n = (int)(sizeof V / sizeof *V);
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
            int16_t a = (int16_t)V[i], b = (int16_t)V[j];
            D32 p = d16_mult(a, b);
            int32_t e = largo(a, b);
            casos++;
            if(d32_to_i32(p) != e) mal++;
        }
        for(int a = -200; a <= 200; a++) for(int b = -200; b <= 200; b++){
            D32 p = d16_mult((int16_t)a, (int16_t)b);
            int32_t e = largo((int16_t)a, (int16_t)b);
            casos++;
            if(d32_to_i32(p) != e) mal++;
        }
        printf("      %ld produtos: %ld divergências\n", casos, mal);
        ok("o par D32 concorda com int32 em todos os produtos 16×16", mal == 0 && casos > 80000);
    }

    printf("\n§E1 A ordem por produto cruzado.\n\n");
    {
        long mal = 0, casos = 0, ing = 0;
        for(int i = -50; i <= 50; i++) for(int j = -50; j <= 50; j++){
            int16_t a = (int16_t)(i * 131), b = (int16_t)(j * 127);
            int16_t c = (int16_t)((i + 1) * 137), d = (int16_t)((j - 1) * 139);
            int meu = d16_cmp_prod(a, b, c, d);
            int reg = cmp32(largo(a, b), largo(c, d));
            casos++;
            if(meu != reg) mal++;
            int16_t ab = (int16_t)(a * b), cd = (int16_t)(c * d);
            int naive = (ab < cd) ? -1 : (ab > cd ? 1 : 0);
            if(naive != reg) ing++;
        }
        printf("      %ld comparações: %ld divergências; int16 sozinho erra %ld\n",
               casos, mal, ing);
        ok("d16_cmp_prod decide a ordem onde int16 transborda", mal == 0 && ing > 100);
    }

    printf("\n§E2 Soma e diferença do par D32.\n\n");
    {
        long mal = 0, casos = 0;
        for(int i = -100; i <= 100; i++) for(int j = -100; j <= 100; j++){
            int16_t a = (int16_t)(i * 59), b = (int16_t)(j * 61);
            D32 x = d16_mult(a, b);
            D32 y = d16_mult(b, a);
            int32_t ex = largo(a, b), ey = largo(b, a);
            D32 s = d16_soma(x, y);
            int32_t es = ex + ey;
            casos++;
            if(d32_to_i32(s) != es) mal++;
            if(ex >= ey){
                D32 df = d16_menos(x, y);
                if(d32_to_i32(df) != ex - ey) mal++;
            }
        }
        printf("      %ld pares: %ld divergências\n", casos, mal);
        ok("d16_soma e d16_menos concordam com int32", mal == 0);
    }

    printf("\n§E3 Operações ⊕ e ⊗ no par — racionais.tex.\n\n");
    {
        long mal_m = 0, mal_s = 0, casos = 0;
        for(int a = -8; a <= 8; a++) for(int b = 1; b <= 8; b++)
        for(int c = -8; c <= 8; c++) for(int d = 1; d <= 8; d++){
            D32par pm, ps;
            d16_par_mult((int16_t)a, (int16_t)b, (int16_t)c, (int16_t)d, &pm);
            d16_par_soma((int16_t)a, (int16_t)b, (int16_t)c, (int16_t)d, &ps);
            int32_t ac = largo((int16_t)a, (int16_t)c);
            int32_t bd = largo((int16_t)b, (int16_t)d);
            int32_t ad = largo((int16_t)a, (int16_t)d);
            int32_t bc = largo((int16_t)b, (int16_t)c);
            casos++;
            if(d32_to_i32(pm.p) != ac || d32_to_i32(pm.q) != bd) mal_m++;
            if(d32_to_i32(ps.p) != ad + bc || d32_to_i32(ps.q) != bd) mal_s++;
        }
        printf("      %ld quádruplos: produto %ld erros, soma %ld erros\n",
               casos, mal_m, mal_s);
        ok("(a,b)⊗(c,d)=(ac,bd) exacto no par D32", mal_m == 0);
        ok("(a,b)⊕(c,d)=(ad+bc,bd) exacto no par D32", mal_s == 0);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
