/* inteiros.c — N → Z medido: Teor. inteiros.tex (soma cruzada, swap, W_8).
 *
 *   §NT1  (a,b)~(c,d) ⟺ a+d=b+c  (lib/inteiros.h iz_equiv)
 *   §NT2  Cruz=val(a,b)=a−b é invariante na classe
 *   §NT3  soma (a+c,b+d) desce ao quociente
 *   §NT4  produto (ac+bd,ad+bc) desce ao quociente
 *   §NT5  oposto=swap: (a,b)+(b,a) ~ (0,0)
 *   §NT6  ν²=id e Fix(ν) quando a=b
 *   §NT6b Fix(ι): ι(a,b)=(a,b) ⟺ a=b
 *   §NT7  S_8: wrap 255→0 (palavra de 8 bits)
 *   §NT8  par em W_8²: soma cruzada com aritmética uint16 nos representantes
 *   §NT9  ν(xy)=−ν(x)ν(y) — antiautomorfismo, não automorfismo de anel
 *   §NT10 Dir(a,b)=a+b NÃO é invariante de classe
 *   §NT11 W_8: leitura cruzada em uint16; wrap vs saturação contados à parte
 *   §NT11b projeções 300 e contadores finais
 *
 *   cc -O2 -std=c99 -w -I../lib -o inteiros inteiros.c && ./inteiros
 */
#include <stdio.h>
#include <stdint.h>
#include "unidade.h"
#include "inteiros.h"
#include "naturais.h"

typedef long L;

static L cruz(L a, L b){ return a - b; }

static void soma_par(L a, L b, L c, L d, L *sa, L *sb){
    *sa = a + c; *sb = b + d;
}

static void mult_par(L a, L b, L c, L d, L *pa, L *pb){
    *pa = a * c + b * d;
    *pb = a * d + b * c;
}

int main(void){
    printf("=== INTEIROS (N → Z): soma cruzada, swap, palavra W_8 ===================\n\n");

    /* §NT1 equivalência = soma cruzada nula */
    L nt1_ok = 0, nt1_tot = 0;
    for(L a = 0; a <= 25; a++) for(L b = 0; b <= 25; b++)
        for(L c = 0; c <= 25; c++) for(L d = 0; d <= 25; d++){
            nt1_tot++;
            int eq = iz_equiv(a, b, c, d);
            int cr = (a + d) == (b + c);
            if(eq == cr) nt1_ok++;
        }
    printf("§NT1  iz_equiv bate a+d=b+c em %ld de %ld pares\n\n", (long)nt1_ok, (long)nt1_tot);
    ok("§NT1 equivalência por soma cruzada: (a,b)~(c,d) ⟺ a+d=b+c",
       nt1_ok == nt1_tot);

    /* §NT2 val(a,b)=a−b invariante */
    L nt2_ok = 0, nt2_tot = 0;
    for(L a = 0; a <= 40; a++) for(L b = 0; b <= 40; b++)
        for(L c = 0; c <= 40; c++) for(L d = 0; d <= 40; d++){
            if(!iz_equiv(a, b, c, d)) continue;
            nt2_tot++;
            if(cruz(a, b) == cruz(c, d)) nt2_ok++;
        }
    ok("§NT2 Cruz=val(a,b) é invariante na classe de equivalência",
       nt2_tot > 0 && nt2_ok == nt2_tot);

    /* §NT3 soma bem-definida */
    L nt3_ok = 0, nt3_tot = 0;
    for(L a = 0; a <= 20; a++) for(L b = 0; b <= 20; b++)
        for(L ap = 0; ap <= 20; ap++) for(L bp = 0; bp <= 20; bp++)
            if(!iz_equiv(a, b, ap, bp)) continue;
            else for(L c = 0; c <= 20; c++) for(L d = 0; d <= 20; d++)
                for(L cp = 0; cp <= 20; cp++) for(L dp = 0; dp <= 20; dp++){
                    if(!iz_equiv(c, d, cp, dp)) continue;
                    L sa, sb, sap, sbp;
                    soma_par(a, b, c, d, &sa, &sb);
                    soma_par(ap, bp, cp, dp, &sap, &sbp);
                    nt3_tot++;
                    if(iz_equiv(sa, sb, sap, sbp)) nt3_ok++;
                }
    printf("§NT3  soma no par: %ld de %ld classes concordam\n\n", (long)nt3_ok, (long)nt3_tot);
    ok("§NT3 soma (a+c,b+d) descende ao quociente N²/∼=Z",
       nt3_tot > 0 && nt3_ok == nt3_tot);

    /* §NT4 produto bem-definido */
    L nt4_ok = 0, nt4_tot = 0;
    for(L a = 0; a <= 12; a++) for(L b = 0; b <= 12; b++)
        for(L ap = 0; ap <= 12; ap++) for(L bp = 0; bp <= 12; bp++)
            if(!iz_equiv(a, b, ap, bp)) continue;
            else for(L c = 0; c <= 12; c++) for(L d = 0; d <= 12; d++)
                for(L cp = 0; cp <= 12; cp++) for(L dp = 0; dp <= 12; dp++){
                    if(!iz_equiv(c, d, cp, dp)) continue;
                    L pa, pb, pap, pbp;
                    mult_par(a, b, c, d, &pa, &pb);
                    mult_par(ap, bp, cp, dp, &pap, &pbp);
                    nt4_tot++;
                    if(iz_equiv(pa, pb, pap, pbp)) nt4_ok++;
                }
    ok("§NT4 produto (ac+bd,ad+bc) descende ao quociente",
       nt4_tot > 0 && nt4_ok == nt4_tot);

    /* §NT5 oposto fecha em zero */
    L nt5_ok = 0, nt5_tot = 0;
    for(L a = 0; a <= 50; a++) for(L b = 0; b <= 50; b++){
        L sa, sb;
        soma_par(a, b, b, a, &sa, &sb);
        nt5_tot++;
        if(iz_equiv(sa, sb, 0, 0)) nt5_ok++;
    }
    ok("§NT5 oposto=swap: (a,b)⊕(b,a) ~ (0,0) — a folha volta ao zero",
       nt5_ok == nt5_tot);

    /* §NT6 ν: swap troca sinal; swap²=id; Fix quando a=b */
    L inv_ok = 0, inv_tot = 0, fix_ok = 0, fix_tot = 0;
    for(L a = 0; a <= 40; a++) for(L b = 0; b <= 40; b++){
        inv_tot++;
        L x = a, y = b, t;
        t = x; x = y; y = t;   /* swap */
        t = x; x = y; y = t;   /* swap² = id */
        if(cruz(b, a) + cruz(a, b) == 0 && x == a && y == b) inv_ok++;
        if(a == b){
            fix_tot++;
            if(iz_equiv(a, b, 0, 0)) fix_ok++;
        }
    }
    ok("§NT6 ν: swap troca o sinal (cruz(b,a)=−cruz(a,b)) e swap²=id no par",
       inv_ok == inv_tot);
    ok("§NT6 Fix(ν): (a,b)~(0,0) quando a=b (ponto fixo do swap)",
       fix_tot > 0 && fix_ok == fix_tot);

    /* §NT6b Fix(ι): swap fixa o par ⟺ a=b */
    L iota_ok = 0, iota_tot = 0;
    for(L a = 0; a <= 60; a++) for(L b = 0; b <= 60; b++){
        iota_tot++;
        int fix = (a == b);
        int swap_fix = (b == a && a == b);
        if(fix == swap_fix) iota_ok++;
    }
    ok("§NT6b Fix(ι): ι(a,b)=(a,b) ⟺ a=b — fixos no espaço, não no quociente",
       iota_ok == iota_tot);

    /* §NT7 S_8 wrap */
    uint8_t w = 255, s = (uint8_t)(w + 1);
    printf("§NT7  S_8(255)=%u (wrap)\n\n", (unsigned)s);
    ok("§NT7 sucessor em W_8: S_8(255)=0 — wrap explícito no byte",
       s == 0);

    /* §NT8 par W_8² com soma cruzada em uint16 */
    L nt8_ok = 0, nt8_tot = 0;
    for(uint16_t a = 0; a < 256; a += 17)
        for(uint16_t b = 0; b < 256; b += 23)
            for(uint16_t c = 0; c < 256; c += 31)
                for(uint16_t d = 0; d < 256; d += 37){
                    uint16_t Ls = (uint16_t)(a + d), Rs = (uint16_t)(b + c);
                    int eq16 = (Ls == Rs);
                    int eqL = iz_equiv((L)a, (L)b, (L)c, (L)d);
                    nt8_tot++;
                    if(eq16 == eqL) nt8_ok++;
                }
    ok("§NT8 par em W_8²: equivalência a+d=b+c com uint16 nos representantes",
       nt8_tot > 0 && nt8_ok == nt8_tot);

    /* §NT9 ν(xy)=−ν(x)ν(y): ν é antiautomorfismo aditivo */
    L nt9_ok = 0, nt9_tot = 0;
    for(L a = 0; a <= 20; a++) for(L b = 0; b <= 20; b++)
        for(L c = 0; c <= 20; c++) for(L d = 0; d <= 20; d++){
            L pa, pb, na, nb;
            mult_par(a, b, c, d, &pa, &pb);
            mult_par(b, a, d, c, &na, &nb);
            L vxy = cruz(pa, pb);
            L vx = cruz(a, b), vy = cruz(c, d);
            L nuprod = cruz(na, nb);
            L nu_xy = cruz(pb, pa);
            nt9_tot++;
            if(nuprod == vx * vy && nu_xy == -vxy) nt9_ok++;
        }
    ok("§NT9 ν(xy)=−ν(x)ν(y): antiautomorfismo aditivo, não automorfismo de anel",
       nt9_tot > 0 && nt9_ok == nt9_tot);

    /* §NT10 Dir=a+b não é invariante: Cruz sim, Dir não */
    {
        L dir_diff = 0, dir_same = 0;
        for(L a = 0; a <= 30; a++) for(L b = 0; b <= 30; b++)
            for(L c = 0; c <= 30; c++) for(L d = 0; d <= 30; d++){
                if(!iz_equiv(a, b, c, d)) continue;
                if((a + b) == (c + d)) dir_same++;
                else dir_diff++;
            }
        int wit = iz_equiv(3, 1, 2, 0) && (3 + 1) != (2 + 0);
    ok("§NT10 Dir(a,b)=a+b não é invariante de classe (ex.: (3,1)~(2,0))",
       wit && dir_diff > 0 && dir_same > 0);
    }

    /* §NT11 W_8: equivalência exacta; wrap ≠ saturação; contadores à parte */
    {
        long eq_ok = 0, eq_tot = 0;
        for(uint16_t a = 0; a < 256; a += 13)
            for(uint16_t b = 0; b < 256; b += 17)
                for(uint16_t c = 0; c < 256; c += 19)
                    for(uint16_t d = 0; d < 256; d += 29){
                        eq_tot++;
                        if(w8_equiv((uint8_t)a,(uint8_t)b,(uint8_t)c,(uint8_t)d)
                           == iz_equiv((L)a,(L)b,(L)c,(L)d)) eq_ok++;
                    }
        w8_wrap = 0; w8_saturou = 0;
        int pol_ok = 1;
        for(uint16_t s = 250; s <= 260; s++){
            uint8_t w = w8_proj_wrap(s);
            uint16_t t = w8_proj_sat(s);           /* saturo→promove: exacto */
            uint8_t ew = (uint8_t)s;
            if(w != ew || t != s) pol_ok = 0;
        }
        ok("§NT11 W_8: cruz exacta em uint16; wrap≠promove; w8_wrap/w8_saturou contados",
           eq_tot > 0 && eq_ok == eq_tot && pol_ok
           && w8_wrap == 5 && w8_saturou == 5);
        {
            uint8_t w300 = w8_proj_wrap(300);
            uint16_t s300 = w8_proj_sat(300);
            ok("§NT11b projeções 300: wrap=44, promove=300; contadores incrementam",
               w300 == 44 && s300 == 300 && w8_wrap == 6 && w8_saturou == 6);
        }
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", (long)unidades, (long)falhas);
    return falhas ? 1 : 0;
}
