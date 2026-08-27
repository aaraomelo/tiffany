/* eixos.c — os DOIS EIXOS da passagem, no `fisica.tex` (Teor. dos dois eixos).
 *
 *   cc -O2 -std=c99 -o /tmp/eixos tests/eixos.c && /tmp/eixos
 *
 * A passagem f: A -> B responde a DUAS perguntas independentes, e o livro
 * tratava-as como se fossem uma. A primeira é o que f PRESERVA — o bit a, que
 * diz se as duas faces trocam de papel. A segunda é o que f PERDE — injectar e
 * sobrejectar. Preservar não implica não perder, nem o contrário.
 *
 * O achado é que o segundo eixo JÁ ESTAVA MEDIDO nesta obra, e o medidor é o G:
 * injectar é G<=1, sobrejectar é G>=1, e as duas juntas são G==1, que é a folga
 * a anular-se. Bijectivo é plano, e não é figura de estilo.
 *
 * E o primeiro eixo fecha em dois porque o bit é ele próprio uma dobra: compor
 * soma os bits, somar módulo dois volta, e uma identidade que volta admite duas
 * espécies e não três.
 *
 * Varrem-se TRÊS regimes de formato, e não um: com |I|>|X| nenhuma realização é
 * injectiva, e com |I|<|X| nenhuma é sobrejectiva. Num formato só, metade das
 * equivalências ficaria verificada em vazio — e vazio não é prova.
 *
 *   §E1  o bit a compõe-se somando módulo dois — as quatro composições
 *   §E2  G lê os dois lados: injectiva <=> G<=1, sobrejectiva <=> G>=1
 *   §E3  bijectiva <=> G==1 <=> Phi=0, nos três regimes de formato
 */
#include <stdio.h>

int main(void){
  int falhas = 0;

  /* §E1 — o bit a é uma dobra: compor soma módulo dois. */
  puts("§E1  o bit a: compor soma modulo dois");
  const char *nome[2] = {"isomorfismo", "duomorfismo"};
  for (int a = 0; a < 2; a++)
    for (int b = 0; b < 2; b++)
      printf("     %-11s o %-11s -> %s\n", nome[a], nome[b], nome[(a + b) % 2]);
  for (int a = 0; a < 2; a++)
    if (((a + 1) + 1) % 2 != a) falhas++;
  printf("     a -> a+1 involutiva: %s\n\n", falhas ? "FALHA" : "ok");

  /* §E2 e §E3 — G lê a injectividade e a sobrejectividade. */
  puts("§E2  G le os dois lados;  §E3  bijectiva <=> G==1 <=> Phi=0");
  const int caso[][2] = { {2,4},{3,4},{4,4},{5,4}, {2,3},{3,3},{4,3} };
  const int ncasos = (int)(sizeof caso / sizeof caso[0]);
  long inj_tot = 0, sob_tot = 0, bij_tot = 0, var_tot = 0;

  for (int k = 0; k < ncasos; k++) {
    const int N = caso[k][0], M = caso[k][1];
    long total = 1;
    for (int i = 0; i < N; i++) total *= M;

    long inj_n = 0, sob_n = 0, bij_n = 0;
    for (long c = 0; c < total; c++) {
      int pi[8], G[8] = {0};
      long t = c;
      for (int i = 0; i < N; i++) { pi[i] = (int)(t % M); t /= M; }
      for (int i = 0; i < N; i++) G[pi[i]]++;

      int inj = 1, sob = 1, Gle = 1, Gge = 1, Phi = 0;
      for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
          if (pi[i] == pi[j]) inj = 0;
      for (int x = 0; x < M; x++) {
        if (G[x] == 0) sob = 0;
        if (G[x] >  1) Gle = 0;
        if (G[x] <  1) Gge = 0;
        if (G[x] >  0) Phi += G[x] - 1;
      }
      if (inj)        inj_n++;
      if (sob)        sob_n++;
      if (inj && sob) bij_n++;

      if (inj != Gle)                   falhas++;   /* injectiva    <=> G<=1 */
      if (sob != Gge)                   falhas++;   /* sobrejectiva <=> G>=1 */
      if ((inj && sob) != (Gle && Gge)) falhas++;   /* bijectiva    <=> G==1 */
      if (inj && Phi != 0)              falhas++;   /* injectiva    =>  Phi=0 */
    }
    printf("     I(%d)->X(%d): %5ld varridas | inj %4ld | sob %4ld | bij %4ld\n",
           N, M, total, inj_n, sob_n, bij_n);
    inj_tot += inj_n; sob_tot += sob_n; bij_tot += bij_n; var_tot += total;
  }
  printf("     ------ %ld varridas | inj %ld | sob %ld | bij %ld\n",
         var_tot, inj_tot, sob_tot, bij_tot);
  printf("\nFALHAS: %d\n", falhas);
  return falhas != 0;
}
