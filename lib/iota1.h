/* lib/iota1.h — ι₁ : X₁ → X₂
 *
 * Semântica contratada (CONTRATO_X2.md):
 *   ι₁(b) = (b, 0₂)  — injectiva, primeira coordenada b, segunda zero.
 *
 * Representação: uint16_t via C₂ (campos.tex:382-395).
 *   C₂(b₀, b₁) = b₀ + 256·b₁
 *   ι₁(b) = C₂(b, 0) = (uint16_t)b
 *
 * Leituras independentes (sem chamar iota₁):
 *   π₁(x) = (uint8_t)(x & 0xFF)
 *   π₂(x) = (uint8_t)((x >> 8) & 0xFF)
 *
 * Não altera: campos.tex, espaco.tex, papers, headers existentes.
 * Reutiliza: palavra8.h (Word8). 0₂ = resíduo 0 na segunda posição.
 */
#ifndef IOTA1_H
#define IOTA1_H
#include <stdint.h>
#include "palavra8.h"

static uint16_t iota1(Word8 b){ return (uint16_t)b; }  /* C₂(b,0) */

static uint8_t iota1_pi1(uint16_t x){ return (uint8_t)(x & 0xFFu); }
static uint8_t iota1_pi2(uint16_t x){ return (uint8_t)((x >> 8) & 0xFFu); }
#endif