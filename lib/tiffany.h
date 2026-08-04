/* tiffany.h — A BIBLIOTECA: o corpo, a análise, a física realizada e a bateria.
 *
 * Os dezanove cabeçalhos viviam soltos em tools/, ao lado dos medidores que os usam. Aqui
 * ficam juntos e nomeados por assunto. Os medidores continuam a incluir cada um o seu — o
 * `-I../lib` resolve, e nenhuma das 365 linhas `#include` precisou de mudar. Este ficheiro
 * é para quem CONSOME a biblioteca de fora.
 *
 *   cc -O2 -std=c99 -I lib o_meu.c -lm -o o_meu
 *
 * ─── O QUE ESTE INCLUDE DÁ, e o que não dá ────────────────────────────────────────────────
 *
 * Quinze dos dezanove coexistem num só include, e são os que estão abaixo. Isto não foi
 * suposto: mediu-se, compilando cada cabeçalho sozinho e depois em grupo.
 *
 * QUATRO FICAM DE FORA, e a razão é que foram escritos como unidades independentes — cada um
 * para o seu medidor, com `static` e nomes curtos, sem intenção de conviver:
 *
 *   edo.h      define `Fonte`, que caminho.h também define
 *   gp2.h      define `frob`,  que corpos.h  também define
 *   expr.h     não compila sozinho: conta com declarações do medidor que o inclui
 *   stratum.h  idem
 *
 * Incluem-se à parte, um de cada vez, e funcionam:
 *
 *   cc -O2 -std=c99 -I lib -include edo.h o_meu.c ...
 *
 * Resolver as colisões é possível — renomear `Fonte` e `frob` toca em oito medidores — mas é
 * mudança de interface e não de arrumação. Fica dito em vez de escondido: um include que
 * silenciosamente deixasse dois cabeçalhos de fora seria pior que este comentário.
 */
#ifndef TIFFANY_H
#define TIFFANY_H

/* ---- o corpo e a álgebra ---- */
#include "corpos.h"      /* a borda, a régua (B,C), o dual ν, a norma       */
#include "algebra.h"
#include "poli.h"
#include "gf2n.h"
#include "quat.h"
#include "cifra.h"
#include "contrato.h"

/* ---- a análise ---- */
#include "spline.h"
#include "caminho.h"

/* ---- a física realizada ---- */
#include "eletrico.h"
#include "termica.h"
#include "banda.h"

/* ---- entrada e saída ---- */
#include "pgm.h"
#include "le_emb.h"      /* lê os dois formatos: decimal e 0x%08X exato     */

/* ---- a bateria ---- */
#include "unidade.h"     /* ok("o que se afirma", condição) — 254 medidores */

#endif
