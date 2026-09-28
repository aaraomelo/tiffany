/* lib/iota3.h — ι₃ : X₃ → X₄   (família de BYTES, sec:byte-carry)
 *
 * Semântica (CONTRATO_X4.md §2):
 *   ι₃(b₀, b₁, b₂) = (b₀, b₁, b₂, 0)
 *   b₀, b₁ e b₂ passam intactos; a quarta coordenada é o resíduo 0 na posição
 *   de peso 256³. Determinístico, sem estado, sem alocação.
 *
 * Instância d=3 da definição canónica ÚNICA, def:iota-d
 * (redes/campos.tex, sec:byte-carry, linhas 397-427):
 *   ι_d(b₀,…,b_{d−1}) = (b₀,…,b_{d−1}, 0)
 * Não é lei nova: é a substituição de d por 3 numa definição já publicada em
 * 25a19dfa. Nenhuma cópia da definição vive neste ficheiro. ι₂ (0e3df918) é a
 * instância d=2 da MESMA frase; a ι₃ não a estende nem a copia.
 *
 * X₄ aqui é a família de bytes de campos.tex, NÃO o quarto degrau da cadeia
 * X₁→X₂→X₃→X₄ da escada, que em papers/aranha.tex:4462 e lib/escada.h:10 é
 * "os cortes de X₃" (CONTRATO_X4.md §0). O tipo chama-se Iota3Out e não X₄
 * porque tests/gerador.c:566 já declara um X₄ local, e esse é degrau de escada.
 *
 * Representação: quatro coordenadas byte num struct, deliberadamente não
 * empacotadas. Motivo (CONTRATO_X4.md §3.3): as quatro posições têm de ser
 * legíveis por canais independentes, e em memória separada são-no por
 * construção. Um word de 32 bits teria o mesmo poder de medida, mas obrigaria
 * a extrair as posições à mão e a separação dos canais passaria a depender
 * de quem lê, não da forma do dado.
 *
 * NÃO calcula código nenhum. A codificação C₄ e o oráculo O₃ são do
 * instrumento (CONTRATO_X4.md §6) e vivem em tests/medidor_iota3.c. Um alvo
 * que calculasse o seu próprio código daria ao oráculo nada contra que
 * divergir.
 *
 * Não altera: iota1.h, iota2.h, CONTRATO_X2.md, CONTRATO_X3.md, campos.tex,
 *             espaco.tex, medidor/ESPECIFICACAO.md.
 * Reutiliza: palavra8.h (Word8 = uint8_t). */
#ifndef IOTA3_H
#define IOTA3_H
#include <stdint.h>
#include "palavra8.h"

/* As quatro coordenadas de X₄, little-endian como em campos.tex:392
 * (b_k tem peso 256^k). 0-based: π₀ = b0, π₁ = b1, π₂ = b2, π₃ = b3. */
typedef struct {
    Word8 b0;
    Word8 b1;
    Word8 b2;
    Word8 b3;
} Iota3Out;

static Iota3Out iota3(Word8 b0, Word8 b1, Word8 b2){
    Iota3Out y;
    y.b0 = b0;          /* preservada exactamente */
    y.b1 = b1;          /* preservada exactamente */
    y.b2 = b2;          /* preservada exactamente */
    y.b3 = (Word8)0;    /* resíduo 0 na posição nova, peso 256³ */
    return y;
}
#endif
