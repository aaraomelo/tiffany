/* palavra8.c — duas Word_8 bit a bit; F_8; ℕ; largura 8→16→32.
 *
 *   cc -O2 -std=c99 -I lib -o /tmp/palavra8 tests/palavra8.c && /tmp/palavra8
 */
#include "unidade.h"
#include "palavra8.h"
#include <stdio.h>

int main(void){
    printf("Word_8: bit a bit, F_8, ℕ, largura\n\n");
    w8_torre();

    /* §P0 bit a bit — neuronio: XOR/AND em qualquer par */
    {
        long mal = 0;
        for(int a = 0; a < 256; a++) for(int b = 0; b < 256; b++){
            Word8 x = (Word8)a, y = (Word8)b;
            if(w8_xor(x, y) != (Word8)(a ^ b)) mal++;
            if(w8_and(x, y) != (Word8)(a & b)) mal++;
            for(int k = 0; k < 8; k++)
                if(w8_bit(x, k) != ((a >> k) & 1)) mal++;
        }
        printf("§P0  ⊕ ∧ e coordenadas e_k=2^k em todos os pares Word_8: %ld falhas\n", mal);
        ok("duas palavras operam bit a bit (𝔽₂⁸)", mal == 0);
    }

    /* §P1 F_8 — corpo no topo da torre binária */
    {
        long mal = 0, inv = 0;
        for(int a = 0; a < 256; a++){
            if(w8_som_f8((Word8)a, (Word8)a) != 0) mal++;
            if(a && w8_mul_f8((Word8)a, w8_inv_f8((Word8)a)) != 1) inv++;
        }
        for(int a = 0; a < 256; a++) for(int b = 0; b < 256; b++){
            Word8 p = w8_mul_f8((Word8)a, (Word8)b);
            Word8 q = w8_mul_f8((Word8)b, (Word8)a);
            if(p != q) mal++;
        }
        printf("§P1  F_8: a⊕a=0; comutativo; inversas: %ld / inv %ld\n", mal, inv);
        ok("produto F_8 em Word_8 (bn_mul k=3)", mal == 0 && inv == 0);
    }

    /* §P2 ℕ — ponto fixo (⊕,∧) contra + da máquina */
    {
        long mal = 0;
        for(int a = 0; a < 256; a++) for(int b = 0; b < 256; b++){
            if(w8_soma_N((Word8)a, (Word8)b) != (unsigned long)(a + b)) mal++;
            if(w8_mult_N((Word8)a, (Word8)b) != (unsigned long)(a * b)) mal++;
        }
        printf("§P2  soma/produto ℕ no suporte Word_8: %ld falhas\n", mal);
        ok("Word_8 sobe a ℕ pela iteração (⊕,∧)", mal == 0);
    }

    /* §P3 largura — mesma dobra w=8,16,32 */
    {
        long mal = 0;
        uint64_t s = 1;
        for(int t = 0; t < 2000; t++){
            int ws[] = {8, 16, 32};
            for(int i = 0; i < 3; i++){
                int w = ws[i];
                uint64_t a = lg_prox(&s) & lg_masc(w);
                uint64_t b = lg_prox(&s) & lg_masc(w);
                LgPar p = w8_mult_larg(w, a, b);
                uint64_t ref = a * b;
                if(lg_val(p, w) != (ref & ((((uint64_t)1 << (2*w)) - 1)))){
                    /* 2w pode exceder 64 quando w=32: comparar mascarado */
                    uint64_t mw2 = (w < 32) ? ((((uint64_t)1 << (2*w)) - 1)) : ~(uint64_t)0;
                    if(lg_val(p, w) != (ref & mw2)) mal++;
                }
            }
        }
        printf("§P3  lg_mult w∈{8,16,32} vs produto da máquina: %ld falhas\n", mal);
        ok("propaga na torre de larguras a partir de Word_8", mal == 0);
    }

    /* §P4 par Word_8² — Lei 7 */
    {
        long mal = 0;
        for(int a = 0; a < 64; a++) for(int b = 0; b < 64; b++)
        for(int c = 0; c < 64; c++) for(int d = 0; d < 64; d++){
            W8Par x = w8_par((Word8)a, (Word8)b), y = w8_par((Word8)c, (Word8)d);
            int eq = w8_par_equiv(x, y);
            int ref = ((a + d) == (b + c));
            if(eq != ref) mal++;
        }
        printf("§P4  ~ em Word_8² (cruz uint16): %ld falhas\n", mal);
        ok("par ≠ classe: equivalência sem projectar", mal == 0);
    }

    return falhas ? 1 : 0;
}
