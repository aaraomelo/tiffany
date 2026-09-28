/* lib/iota2.h — ι₂ : X₂ → X₃   (família de BYTES, sec:byte-carry)
 *
 * Semântica (CONTRATO_X3.md §2):
 *   ι₂(b₀, b₁) = (b₀, b₁, 0)
 *   b₀ e b₁ passam intactos; a terceira coordenada é o resíduo 0 na posição
 *   de peso 256². Determinístico, sem estado, sem alocação.
 *
 * Instância d=2 da definição canónica ÚNICA, def:iota-d
 * (redes/campos.tex, sec:byte-carry, linhas 397-427):
 *   ι_d(b₀,…,b_{d−1}) = (b₀,…,b_{d−1}, 0)
 * Não é lei nova: é a substituição de d por 2 numa definição já publicada em
 * 25a19dfa. Nenhuma cópia da definição vive neste ficheiro.
 *
 * X₃ aqui é a família de bytes de campos.tex, NÃO a cadeia X₁→X₂→X₃→X₄ da
 * escada (CONTRATO_X3.md §0). O tipo chama-se Iota2Out e não X₃ porque
 * tests/gerador.c:565 já declara um X₃ local, e esse é degrau de escada.
 *
 * Representação: três coordenadas byte num struct, deliberadamente não
 * empacotadas. Motivo (CONTRATO_X3.md §3.3): as três posições têm de ser
 * legíveis por canais independentes, e em memória separada são-no por
 * construção. Um word de 24 bits teria o mesmo poder de medida, mas obrigaria
 * a extrair as posições à mão e a separação dos canais passaria a depender
 * de quem lê, não da forma do dado.
 *
 * Não altera: iota1.h, CONTRATO_X2.md, CONTRATO_X3.md, campos.tex, espaco.tex.
 * Reutiliza: palavra8.h (Word8). */
#ifndef IOTA2_H
#define IOTA2_H
#include <stdint.h>
#include "palavra8.h"

/* As três coordenadas de X₃, little-endian como em campos.tex:392
 * (b_k tem peso 256^k). 0-based: π₀ = b0, π₁ = b1, π₂ = b2. */
typedef struct { Word8 b0, b1, b2; } Iota2Out;

static Iota2Out iota2(Word8 b0, Word8 b1){
    Iota2Out y;
    y.b0 = b0;   /* preservada exactamente */
    y.b1 = b1;   /* preservada exactamente */
    y.b2 = (Word8)0;   /* resíduo 0 na posição nova */
    return y;
}
#endif
