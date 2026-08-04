/* refracao.c — A REFRAÇÃO É O GANCHO PARA CONVERSAR EM TRÊS.
 *
 * O Aarão: "vê se é o gancho pra conversar em 3."
 *
 * A REFLEXÃO fica no mesmo meio: J, ordem 2, o raio volta. Com ela chega-se a dois e mais nada —
 * ida e volta, e a segunda ida é a primeira outra vez.
 *
 * A REFRAÇÃO atravessa para outro meio. Aqui o meio é o CORPO e o índice é a RÉGUA, e o
 * atravessar já tem peça medida: a transferência φ_t(a,b) = (a+tb, b) com t = (B₂−B₁)/2.
 * É Snell — o desvio é dado pela diferença dos meios.
 *
 * E a pergunta do gancho tem resposta exata: φ COMPÕE? Se φ_t₁ ∘ φ_t₂ = φ_(t₁+t₂), então
 * atravessar A→B→C é uma travessia só, e três é tão possível como dois. Se não compuser, fica-se
 * preso a saltos de dois.
 *
 *   §F1  φ_t leva a régua B₁ na régua B₂ — e o t é (B₂−B₁)/2, não outro
 *   §F2  φ COMPÕE: A→B→C é o mesmo que A→C, resíduo 0
 *   §F3  e o Δ atravessa intacto — é o invariante que sobrevive à travessia
 *   §F4  logo três conversam, e N também: a reflexão não dava isto
 *
 *   cc -O2 -std=c99 refracao.c -o refracao && ./refracao
 */
#include <stdio.h>
#include "corpos.h"
#include "contrato.h"
#include "unidade.h"

/* a transferência: o raio a dobrar ao mudar de meio */
static Par fi(Par x, long t){ Par r = { x.a + t*x.b, x.b }; return r; }
/* o t que leva uma régua na outra */
static long t_de(Regua a, Regua b){ return (b.B - a.B) / 2; }
/* a régua que sai, aplicando φ_t à forma */
static Regua refrata(Regua r, long t){ Regua s = { r.B + 2*t, r.C + r.B*t + t*t }; return s; }

int main(void){
printf("\n=== A REFRAÇÃO — e é ela o gancho para três ==============================\n");
printf("    A reflexão fica no meio e volta: com ela chega-se a DOIS e mais nada.\n");
printf("    A refração atravessa — e a pergunta é se ela COMPÕE.\n");

/* OS TRES MEIOS TEM DE TER O MESMO Δ. Eu tinha escolhido (7,-1), que tem Δ=53, e a travessia
 * chegou a (7,11) em vez dele — porque φ CONSERVA O Δ: Δ' = (B+2t)² − 4(C+Bt+t²) = B²−4C.
 *
 * Isso nao e defeito da refracao: e a LEI dela. O Δ e o indice do meio, e a luz nao muda de
 * indice a meio do caminho — ela dobra DENTRO do que o indice permite. Atravessar classes de Δ
 * seria outra coisa, e nao e isto. */
Regua A = { 1, -1 };        /* Δ = 5 */
Regua B = { 3,  1 };        /* Δ = 5 */
Regua C = { 5,  5 };        /* Δ = 5 */

printf("\n§F1  φ_t leva a régua B₁ na régua B₂ — e o t é (B₂−B₁)/2.\n\n");
{
    long t = t_de(A, B);
    Regua s = refrata(A, t);
    printf("      A = (%ld,%ld)   t = %ld   ->   (%ld,%ld)\n", A.B, A.C, t, s.B, s.C);
    printf("      B = (%ld,%ld)\n\n", B.B, B.C);
    ok("a régua que sai é EXATAMENTE a do outro meio — o t não é escolhido, é lido",
       s.B == B.B && s.C == B.C);
    printf("      É Snell: o desvio é dado pela diferença dos meios, e não por mim.\n");
}

printf("\n§F2  φ COMPÕE: A→B→C é o mesmo que A→C.\n\n");
{
    long t1 = t_de(A, B), t2 = t_de(B, C), td = t_de(A, C);
    printf("      A->B  t₁ = %ld\n      B->C  t₂ = %ld\n      A->C  t  = %ld   (e t₁+t₂ = %ld)\n\n",
           t1, t2, td, t1 + t2);
    ok("os desvios SOMAM — t₁ + t₂ = t", t1 + t2 == td);
    long mau = 0;
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
        Par x = { a, b };
        Par por_dois = fi(fi(x, t1), t2);          /* atravessar dois meios */
        Par direto   = fi(x, td);                  /* atravessar de uma vez */
        if(por_dois.a != direto.a || por_dois.b != direto.b) mau++;
    }
    printf("      289 pontos: %ld que não batem\n", mau);
    ok("atravessar A->B->C é a MESMA travessia que A->C — resíduo 0", mau == 0);
    printf("\n      E é isto o gancho: se compõe, três é tão possível como dois, e N também.\n");
    printf("      A reflexão não dava isto — J∘J = I, e a segunda ida é a primeira outra vez.\n");
}

printf("\n§F3  E o Δ atravessa intacto — é o que sobrevive à travessia.\n\n");
{
    long t1 = t_de(A, B), t2 = t_de(B, C);
    Regua s1 = refrata(A, t1), s2 = refrata(s1, t2);
    printf("      Δ(A) = %ld\n      Δ(B) = %ld\n      Δ(C) = %ld\n",
           ct_assinatura(A), ct_assinatura(B), ct_assinatura(C));
    printf("      Δ depois de A->B->C = %ld\n\n", ct_assinatura(s2));
    /* o Δ NAO se conserva entre reguas diferentes — e nao devia: sao corpos diferentes.
     * o que se conserva e a travessia dar sempre a MESMA regua de chegada. */
    ok("A->B->C chega exatamente à régua C", s2.B == C.B && s2.C == C.C);
    ok("e o Δ atravessa INTACTO — é o invariante da refração", ct_assinatura(s2) == ct_assinatura(A));
    printf("\n      O Δ é o ÍNDICE DO MEIO, e ele não muda: a luz dobra dentro do que o índice\n");
    printf("      permite. Eu tinha escolhido um terceiro meio com outro Δ e a travessia não\n");
    printf("      chegou lá — não por falha, mas porque φ conserva o Δ por construção:\n");
    printf("      Δ = (B+2t)² − 4(C+Bt+t²) = B² − 4C, e o t desaparece da conta.\n");
    printf("\n      Logo a roda de conversa e por CLASSE DE Δ. Atravessar classes seria outra\n");
    printf("      coisa, e nao e refracao — fica dito, e nao suposto.\n");
}

printf("\n§F4  Logo três conversam — e a reflexão não dava isto.\n\n");
{
    printf("      reflexão   J        ordem 2      ida e volta, e para\n");
    printf("      refração   φ_t      SOMA em t    A->B->C->...  sem fim\n\n");
    long t1 = t_de(A, B), t2 = t_de(B, C), t3 = t_de(C, A);
    printf("      e a volta ao princípio: t₁+t₂+t₃ = %ld\n", t1 + t2 + t3);
    ok("dar a volta pelos três e regressar soma ZERO — o circuito fecha", t1 + t2 + t3 == 0);
    printf("\n      Fechar em zero é o que permite conversar em roda: A fala a B, B a C, C a A,\n");
    printf("      e ninguém acumula desvio. Com dois isso é trivial (J e o seu inverso são o\n");
    printf("      mesmo); com três é uma condição, e ela cumpre-se.\n");
    printf("\n      Então SIM: a refração é o gancho. A reflexão fecha em dois porque é involução;\n");
    printf("      a refração compõe, e por isso não há número máximo de participantes.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
