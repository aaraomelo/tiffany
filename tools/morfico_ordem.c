/* morfico_ordem.c — E O MÓRFICO? Eu excluí-o sem procurar a régua dele.
 *
 * O Aarão: "e o outro que não é ordenado, não é porquê? Você não ordenou — está trabalhando com
 * metade e julgando lei de novo?"
 *
 * Está certo em perguntar, e eu tenho de olhar antes de responder. O que eu fiz foi: peguei o
 * mórfico como RETICULADO DE MÁSCARAS, achei incomparáveis na inclusão, mostrei que GF(2) tem
 * característica 2, e excluí. Tudo verdade — e tudo sobre a REPRESENTAÇÃO que eu escolhi.
 *
 * A régua do mórfico, o catálogo di-la: o operador é a ADJUNÇÃO δ⊣ε, isto é, dilatação e erosão.
 * E a dilatação COMPÕE: dilatar por B₁ e depois por B₂ é dilatar por B₁⊕B₂ (Minkowski). Logo o
 * PARÂMETRO da deformação é o elemento estruturante — e para as bolas, é o RAIO.
 *
 *     dil(dil(A, B_r), B_s) = dil(A, B_{r+s})       o raio SOMA
 *
 * E o raio é um inteiro: ORDENADO, e a ordem é compatível com a soma. Se isto se medir, o
 * mórfico ordena como os outros — e o meu "único fora" era eu a não ter procurado.
 *
 *   §M1  a dilatação COMPÕE: B_r depois B_s é B_{r+s} — medido
 *   §M2  logo o parâmetro é o RAIO, e o raio ordena — total e compatível
 *   §M3  e a ordem age no objeto: dilatar mais é conter mais — monótona
 *   §M4  o veredito: 28 de 28, e o meu "um fora" era meia estrutura
 *
 *   cc -O2 -std=c99 morfico_ordem.c -o morfico_ordem && ./morfico_ordem
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

#define NN 16
/* a bola de raio r em Z/NN: {−r,…,r} */
static unsigned bola(int r){
    unsigned B = 0;
    for(int k = -r; k <= r; k++) B |= 1u << (((k % NN) + NN) % NN);
    return B;
}

int main(void){
printf("\n=== E O MÓRFICO? ==========================================================\n");
printf("    Eu excluí-o sem procurar a régua dele. Procuro agora.\n");

printf("\n§M1  A dilatação COMPÕE: dilatar por B_r e depois por B_s é dilatar por B_{r+s}.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      A            r   s   dil(dil(A,B_r),B_s)   dil(A,B_{r+s})   iguais?\n");
    for(unsigned A = 1; A < 512; A += 7)
    for(int r = 0; r <= 3; r++) for(int s = 0; s <= 3; s++){
        if(r + s > 6) continue;
        unsigned p1 = mo_dil(mo_dil(A, bola(r), NN), bola(s), NN);
        unsigned p2 = mo_dil(A, bola(r+s), NN);
        if(p1 != p2) mau++;
        if(A == 1 && r == 1 && s == 2)
            printf("      {0}          %d   %d   %#x%*s%#x%*s%s\n", r, s, p1, 16-6, "", p2,
                   14-6, "", p1==p2 ? "sim ✓" : "NÃO");
        casos++;
    }
    ok("dilatar por raio r e depois por s É dilatar por r+s — o raio SOMA", mau == 0);
    printf("      (%ld composições.)\n", casos);
    printf("\n      É a associatividade de Minkowski, e é o que faz do RAIO um parâmetro: compor\n");
    printf("      deformações morfológicas é SOMAR raios. Como no telescópico, como no sucessor.\n");
}

printf("\n§M2  Logo o parâmetro é o RAIO — e o raio ORDENA, total e compatível.\n\n");
{
    int mau = 0; long casos = 0;
    for(int r = 0; r <= 7; r++) for(int s = 0; s <= 7; s++){
        int cmp = (r > s) - (r < s);
        if(cmp != -((s > r) - (s < r))) mau++;                /* total e antissimétrico */
        for(int k = 0; k <= 4; k++){
            int c2 = (r+k > s+k) - (r+k < s+k);
            if(c2 != cmp) mau++;                              /* somar preserva */
        }
        casos++;
    }
    ok("o raio é ordem TOTAL em ℕ, e somar um raio preserva a ordem — compatível", mau == 0);
    printf("      (%ld pares de raios.)\n", casos);
    printf("\n      É exatamente a forma dos outros 27: o parâmetro da deformação é um número, e\n");
    printf("      esse número ordena. Não é ordem imposta — é a que a composição traz.\n");
}

printf("\n§M3  E a ordem AGE no objeto: dilatar mais contém mais.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      raio   dil({0}, B_r)      contém o anterior?\n");
    for(unsigned A = 1; A < 512; A += 11)
    for(int r = 0; r <= 5; r++){
        unsigned menor = mo_dil(A, bola(r), NN);
        unsigned maior = mo_dil(A, bola(r+1), NN);
        if((menor & ~maior) != 0) mau++;                      /* r ≤ r+1 ⟹ dil_r ⊆ dil_{r+1} */
        if(A == 1 && r <= 2)
            printf("      %d      %#-18x sim ✓\n", r, menor);
        casos++;
    }
    ok("r ≤ s ⟹ dil(A,B_r) ⊆ dil(A,B_s) — a ordem do raio é MONÓTONA no objeto", mau == 0);
    printf("      (%ld dilatações.)\n", casos);
    printf("\n      Então a ordem não fica só no parâmetro: ela ordena o efeito. Dilatar por raio\n");
    printf("      maior dá conjunto maior, sempre — e é isso que uma régua faz.\n");
}

printf("\n§M4  O veredito: 28 de 28, e o meu \"um fora\" era meia estrutura.\n\n");
{
    conclui("o mórfico ORDENA pelo raio da deformação — e a conta passa a 28 de 28");
    printf("      eu peguei         o reticulado de máscaras, e a inclusão\n");
    printf("      achei             incomparáveis, e característica 2 — TUDO VERDADE\n");
    printf("      e concluí         \"não ordena\", e excluí\n");
    printf("      não procurei      a régua da DEFORMAÇÃO: o raio, que soma e ordena\n");
    printf("\n      É a QUINTA vez hoje, e a pergunta dele nomeia-a: \"está a trabalhar com metade e\n");
    printf("      a julgar lei de novo?\" Estava. ℤ[i] pelo elíptico; o reticulado pela parábola;\n");
    printf("      ℝ pela régua; o par pela classe; e agora a máscara pelo mórfico.\n");
    printf("\n      E note-se o que NÃO muda: a característica 2 continua a impedir uma ordem de\n");
    printf("      CORPO nas máscaras. O que muda é que essa não é a régua do mórfico — a régua é\n");
    printf("      a adjunção, e o parâmetro dela é o raio.\n");
}

printf("\n=== O MÓRFICO ORDENA ======================================================\n");
printf("  A régua do mórfico é a ADJUNÇÃO, e o parâmetro dela é o RAIO do elemento estruturante:\n\n");
printf("    compõe      dil(dil(A,B_r),B_s) = dil(A,B_{r+s}) — o raio SOMA\n");
printf("    ordena      ℕ é total, e somar preserva — compatível\n");
printf("    e age       r ≤ s ⟹ dil(A,B_r) ⊆ dil(A,B_s) — monótona no objeto\n\n");
printf("  Logo 28 de 28. O meu \"um fora\" vinha de eu ter pegado o reticulado de máscaras — que é\n");
printf("  metade — e chamado lei ao que lá achei. Quinta vez hoje, e a mesma forma.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
