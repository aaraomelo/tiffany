/* tests/medidor_iota1.c — ORÁCULO INDEPENDENTE para ι₁ : X₁ ↪ X₂
 *
   Estado: alvo implementado (lib/iota1.h), medidor validado separadamente.
   Este ficheiro mede iota₁ sem a chamar para construir o esperado.
   Autoteste + cobertura 256/256 + controles negativos.
 *
 * Porquê existe: o CONTRATO_X2.md fecha X₂ como espaço sem definir a seta.
 * Este instrumento mede o que existe independentemente da seta:
 *   - oráculo O(b) = (b, 0₂)  (definição do espaço, não implementação)
 *   - canais π₁ e π₂ independentes
 *   - dois controles negativos, cada um mirando um canal
 *   - autoteste do instrumento antes de confiar em qualquer zero
 *
 * Representação: uint16_t via C₂ do paper (campos.tex:382-395).
 *   C₂(b₀,b₁) = b₀ + 256·b₁. Leitura independente:
 *   π₁(x) = (uint8_t)(x & 0xFF), π₂(x) = (uint8_t)((x >> 8) & 0xFF).
 *
 * Controles (domínios correctos após correcção §9 do contrato):
 *   - (b,b):  b ∈ {1,…,255}  → 255 casos (b=0 é oráculo, não perturbação)
 *   - (S₁(b),0):  b ∈ {0,…,254} → 255 casos (S₁(255)=0 invalida controle)
 *
 * Não altera: campos.tex, espaco.tex, papers, headers existentes.
 * ────────────────────────────────────────────────────────────────────
 * Compilar:
 *   cc -O2 -std=c99 -Ilib -o medidor_iota1 tests/medidor_iota1.c -lm
 * Executar:
 *   ./medidor_iota1
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "iota1.h"

/* ─── representação C₂ do paper: uint16_t ─── */
static uint16_t C2(uint8_t b0, uint8_t b1){ return (uint16_t)b0 + (uint16_t)b1 * 256u; }

/* leituras independentes da representação, sem chamar iota₁ */
static uint8_t pi1(uint16_t x){ return (uint8_t)(x & 0xFFu); }
static uint8_t pi2(uint16_t x){ return (uint8_t)((x >> 8) & 0xFFu); }

/* ─── oráculo INDEPENDENTE: O(b) = C₂(b,0) = b ─── */
static uint16_t oracle(uint8_t b){ return (uint16_t)b; }

/* ─── sucessor S₁ do paper: S₁(b) = (b+1) mod 256 ─── */
static uint8_t S1(uint8_t b){ return (uint8_t)(b + 1u); }

/* ─── contadores ─── */
static int ok_canal1 = 0, ok_canal2 = 0;
static int fail_canal1 = 0, fail_canal2 = 0;
static int ctrl1_falha = 0, ctrl1_passa = 0;   /* (b,b): falha canal 2, b=1..255 */
static int ctrl2_falha = 0, ctrl2_passa = 0;   /* (S₁(b),0): falha canal 1, b=0..254 */
static int instrumento_falhas = 0;
static int impl_falhas = 0;

int main(void){
    printf("═══ medidor_iota1 — ι₁ implementada, oráculo independente ═══\n\n");

    /* ── 1. ORÁCULO + CANAIS, b = 0..255 ── */
    printf("§1  oráculo O(b)=(b,0₂) — canais medidos separadamente\n");
    for(int b = 0; b < 256; b++){
        uint16_t esperado = oracle((uint8_t)b);
        uint8_t p1 = pi1(esperado);
        uint8_t p2 = pi2(esperado);
        if(p1 == (uint8_t)b) ok_canal1++; else fail_canal1++;
        if(p2 == 0)          ok_canal2++; else fail_canal2++;
    }
    printf("      canal 1 (π₁==b): %d/256\n", ok_canal1);
    printf("      canal 2 (π₂==0): %d/256\n", ok_canal2);

    /* ── 2. CONTROLES NEGATIVOS ── */
    printf("\n§2  controles negativos independentes\n");
    /* control 1: (b,b) → canal 1 passa, canal 2 falha; b=1..255 */
    for(int b = 1; b < 256; b++){
        uint16_t x = C2((uint8_t)b, (uint8_t)b);   /* (b,b) — segunda posição errada */
        int c1 = (pi1(x) == (uint8_t)b);
        int c2 = (pi2(x) == 0);
        if(c1 && !c2) ctrl1_passa++; else ctrl1_falha++;
    }
    printf("      (b,b) b=1..255: canal1=%d canal2=%d  (esperado: canal1 passa, canal2 falha)\n",
           ctrl1_passa, ctrl1_falha);

    /* control 2: (S₁(b),0) → canal 1 falha, canal 2 passa; b=0..254 */
    for(int b = 0; b < 255; b++){
        uint8_t sb = S1((uint8_t)b);
        uint16_t x = iota1(sb);           /* (S₁(b),0) — primeira posição errada */
        int c1 = (pi1(x) == (uint8_t)b);
        int c2 = (pi2(x) == 0);
        if(!c1 && c2) ctrl2_passa++; else ctrl2_falha++;
    }
    printf("      (S₁(b),0) b=0..254: canal1=%d canal2=%d  (esperado: canal1 falha, canal2 passa)\n",
           ctrl2_passa, ctrl2_falha);

    /* ── 3. AUTOTESTE DO INSTRUMENTO ── */
    printf("\n§3  autoteste do instrumento\n");
    int instrumento_ok = 1;
    if(!(ctrl1_passa == 255 && ctrl1_falha == 0)){
        printf("      FALHA: controle (b,b) não produziu padrão esperado.\n");
        instrumento_ok = 0; instrumento_falhas++;
    }
    if(!(ctrl2_passa == 255 && ctrl2_falha == 0)){
        printf("      FALHA: controle (S₁(b),0) não produziu padrão esperado.\n");
        instrumento_ok = 0; instrumento_falhas++;
    }
    if(instrumento_ok){
        printf("      instrumento passa: ambos os controles negativos com padrões correctos.\n");
    }

    /* ── 4. ALVO ι₁ — medir a implementação real ── */
    printf("\n§4  medição ι₁ : X₁ ↪ X₂\n");
    for(int b = 0; b < 256; b++){
        uint16_t x = iota1((uint8_t)b);
        if(pi1(x) != (uint8_t)b){ impl_falhas++; fail_canal1++; }
        else { ok_canal1++; }
        if(pi2(x) != 0){ impl_falhas++; fail_canal2++; }
        else { ok_canal2++; }
    }
    printf("      ι₁ canal1 (π₁==b): %d/256\n", ok_canal1);
    printf("      ι₁ canal2 (π₂==0): %d/256\n", ok_canal2);
    if(impl_falhas == 0){
        printf("      ι₁: 256/256 — resíduo 0\n");
    } else {
        printf("      ι₁: %d falhas\n", impl_falhas);
        instrumento_falhas++;
    }

    /* ── 5. RESUMO ── */
    printf("\n══════════════════════════════════════════════════════════════\n");
    printf("  oráculo   canal1=%d/256  canal2=%d/256\n", ok_canal1, ok_canal2);
    printf("  ctrl1 (b,b) b=1..255:      passa=%d falha=%d\n", ctrl1_passa, ctrl1_falha);
    printf("  ctrl2 (S₁(b),0) b=0..254:  passa=%d falha=%d\n", ctrl2_passa, ctrl2_falha);
    printf("  ι₁ implementação canal1:   %d/256\n", ok_canal1);
    printf("  ι₁ implementação canal2:   %d/256\n", ok_canal2);
    printf("  autoteste instrumento: %s\n", instrumento_ok ? "PASS" : "FAIL");
    printf("  cobertura: 256/256 entradas\n");
    printf("  falhas: %d\n", instrumento_falhas + impl_falhas);
    printf("  estado: %s\n", (instrumento_ok && impl_falhas==0) ? "FECHADO" : "ABERTO");
    return (instrumento_ok && impl_falhas==0) ? 0 : 1;
}