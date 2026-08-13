/* conecthus/core/eixos_texto.h — eixos de realização em texto (Peano 𝒱).
 * caixa (acento/segmentação), teclado (d_K), fonético (d_φ), forma.
 * Dados reais: layout ABNT2/QWERTY + mapa grafema→fonema PT.
 * Não é sintaxe do .claim — runtime / UI. */
#ifndef CONECTHUS_EIXOS_TEXTO_H
#define CONECTHUS_EIXOS_TEXTO_H

/* Distâncias ≥0. Sem texto → 0. */
long eixos_caixa(const char *a, const char *b);     /* cafe↔café */
long eixos_teclado(const char *a, const char *b);   /* geometria d_K */
long eixos_fonetico(const char *a, const char *b);  /* d_φ */
long eixos_forma(const char *a, const char *b);     /* Levenshtein grafemas */

/* Preenche D[4..7] (caixa, teclado, fonético, forma). D deve ter ≥8 longs. */
void eixos_dist_texto(const char *a, const char *b, long D8[8]);

#endif
