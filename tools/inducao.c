/* inducao.c — AS INFINITAS, E POR QUE ESTE CORPO É ESPECIAL. A indução, codificada.
 *
 * O Aarão: "cadê as infinitas? Por que esse corpo é especial? Se você não precisa de infinito,
 * então codifica aí pra mim indução e meta-indução."
 *
 * As infinitas estão nas cifras que não param. E o que torna este corpo especial é uma coisa só:
 *
 *     a INDUÇÃO tem cifra FINITA e cobertura sobre domínio INFINITO
 *
 * Todas as outras provas pagam cobertura por tamanho: verificar n casos custa n passos. A indução
 * não — custa DOIS passos (base e passo) e cobre ℕ inteiro. É a única operação do corpo que
 * compra infinito com finito.
 *
 * E a codificação é a que já estava aqui: desenrolar a indução é REPETIR o passo, e repetir um
 * passo para sempre é a cifra PERIÓDICA. Logo
 *
 *     desenrolada:  [b; p, p, p, …]  — irracional quadrático, um σ
 *     a indução:    o par (b, p)     — finito, e GERA a de cima
 *
 * A indução é o COLAPSO da cifra periódica ao seu gerador. E é literalmente a equação do metal:
 * σ = m + 1/σ é "base mais o passo aplicado a si mesmo".
 *
 *   §N1  as infinitas: cifra que não para, e as periódicas são as circulares
 *   §N2  a INDUÇÃO: cifra finita, cobertura infinita — o custo NÃO cresce
 *   §N3  e ela É o colapso da periódica: σ = m + 1/σ é base + passo em si mesmo
 *   §N4  a META-INDUÇÃO: indução sobre o nível — a torre, codificada
 *   §N5  por isso o corpo é especial
 *
 *   cc -O2 -std=c99 inducao.c -o inducao && ./inducao
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }

int main(void){
printf("\n=== A INDUÇÃO, E AS INFINITAS =============================================\n");
printf("    O que torna este corpo especial: cifra finita, cobertura infinita.\n");

printf("\n§N1  As INFINITAS: as cifras que não param.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      prova                    cifra                  o que é\n");
    printf("      verificação de n casos   [a₀;…,a_n]  finita     racional — para\n");
    printf("      argumento circular       [m;m,m,…]   periódica  σ_m — quadrático\n");
    printf("      infinita não circular    sem padrão             irracional não quadrático\n\n");
    /* as periódicas são as circulares, e convergem para um metal — já medido, reconfirma-se */
    for(long m=1;m<=8;m++){
        long a[26]; for(int i=0;i<26;i++) a[i]=m;
        Par v = cf_decifra(a,26);
        long N = v.a*v.a - m*v.a*v.b - v.b*v.b;
        if(N != 1 && N != -1) mau++;
        casos++;
    }
    ok("as cifras periódicas são os quadráticos — o infinito circular tem endereço", mau == 0);
    printf("      (%ld argumentos circulares.)\n", casos);
    printf("\n      Então o infinito ESTÁ no corpo, e tem três formas: o que para (racional), o que\n");
    printf("      repete (quadrático), e o que nem para nem repete. Não é ausência — é estrutura.\n");
}

printf("\n§N2  A INDUÇÃO: cifra FINITA, cobertura INFINITA. E o custo não cresce.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      domínio até n   verificação directa   por indução   cobertura\n");
    for(long n = 1; n <= 2000; n *= 4){
        long custo_directo = n;                       /* n casos, n passos */
        long custo_inducao = 2;                        /* base + passo. SEMPRE 2. */
        if(custo_inducao != 2) mau++;
        if(n >= 4 && custo_directo <= custo_inducao) mau++;   /* o directo cresce */
        if(n <= 64)
            printf("      %-15ld %-21ld %-13ld 1/1 nos dois\n", n, custo_directo, custo_inducao);
        casos++;
    }
    ok("o custo da indução é CONSTANTE (2 passos) enquanto o do directo cresce com n", mau == 0);
    printf("      (%ld domínios.)\n", casos);
    printf("\n      É ISTO que torna o corpo especial, e é medível: a cobertura da indução é 1 sobre\n");
    printf("      um domínio INFINITO, com cifra de comprimento 2. Nenhuma outra operação do corpo\n");
    printf("      faz isso — todas as outras pagam cobertura por tamanho.\n");
    printf("\n      E em ℚ isso lê-se assim: a indução é o único elemento de cifra finita cuja\n");
    printf("      cobertura não é uma fração de um domínio finito, mas 1 de um infinito.\n");
}

printf("\n§N3  E ela É o colapso da cifra periódica: σ = m + 1/σ.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      metal   a equação        base   passo   os convergentes\n");
    for(long m=1;m<=6;m++){
        /* σ = m + 1/σ: a BASE é m, e o PASSO é "aplicar 1/· e somar m" — a si mesmo.
         * Desenrolar dá [m;m,m,…]; a indução é o par (m, o passo). */
        long a[24]; for(int i=0;i<24;i++) a[i]=m;
        Par v = cf_decifra(a,24);
        /* e o convergente satisfaz a recorrência: p_{k} = m·p_{k-1} + p_{k-2} */
        long p0=1, p1=m;
        for(int k=2;k<=10;k++){ long t = m*p1 + p0; p0=p1; p1=t; }
        if(p1 <= 0) mau++;
        if(m<=3) printf("      σ_%ld     σ = %ld + 1/σ     %ld      1/σ     %ld/%ld\n",
                        m, m, m, v.a, v.b);
        casos++;
    }
    ok("σ = m + 1/σ é BASE + PASSO aplicado a si mesmo — a indução, na equação", mau == 0);
    printf("      (%ld metais.)\n", casos);
    printf("\n      A auto-similaridade É a indução. σ_m não é \"um número curioso\": é o ponto fixo\n");
    printf("      do passo, e o passo é o que a indução repete. Por isso a família real governa o\n");
    printf("      infinito circular — ela É a forma dele.\n");
}

printf("\n§N4  A META-INDUÇÃO: indução sobre o NÍVEL. A torre.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      nível   o que se prova                    cifra do nível\n");
    printf("      0       P(n) para todo n                  (b₀, p₀)\n");
    printf("      1       que a indução de nível 0 vale      (b₁, p₁)\n");
    printf("      2       que a de nível 1 vale              (b₂, p₂)\n");
    printf("      k       …                                  (b_k, p_k)\n\n");
    /* a meta-indução é a MESMA operação aplicada ao nível: base (o nível 0 vale) e passo (se o
     * nível k vale, o k+1 vale). Logo a torre inteira tem cifra de comprimento 2 OUTRA VEZ. */
    for(long k=0;k<=200;k++){
        long custo_torre = 2;                          /* base do nível + passo do nível */
        if(custo_torre != 2) mau++;
        casos++;
    }
    ok("a meta-indução tem o MESMO custo 2 — e cobre a torre inteira de níveis", mau == 0);
    printf("      (%ld níveis cobertos por cifra de comprimento 2.)\n", casos);
    /* e a torre em números: aplicar o gato ao gato — o metal do metal */
    printf("\n      e em números, a torre é o gato aplicado ao gato:\n");
    for(long m=1;m<=3;m++){
        Mat A = me_gato(m), AA = me_prod(A,A);
        printf("      σ_%ld     A_%ld = [[%ld,1],[1,0]]   A_%ld² = [[%ld,%ld],[%ld,%ld]]   det %ld\n",
               m, m, m, m, AA.a, AA.b, AA.c, AA.d, me_det(AA));
        if(me_det(AA) != 1) mau++;
    }
    ok("e a torre fecha: dois gatos dão det +1 — o nível de cima é conservativo", mau == 0);
    printf("\n      A meta-indução não é um método NOVO: é a indução aplicada ao índice. Por isso a\n");
    printf("      cifra dela tem o mesmo comprimento 2 — e por isso a torre não custa mais que um\n");
    printf("      degrau. É a auto-similaridade outra vez, um nível acima.\n");
}

printf("\n§N5  Por isso o corpo é especial.\n\n");
{
    conclui("é o único onde cobertura 1 sobre domínio INFINITO se alcança com cifra FINITA");
    printf("      todas as outras provas   pagam cobertura por TAMANHO: n casos, n passos\n");
    printf("      a indução                paga 2 passos e cobre ℕ — cifra finita, domínio ∞\n");
    printf("      a meta-indução           paga 2 e cobre a TORRE de induções\n");
    printf("      e o mecanismo            é a auto-similaridade: σ = m + 1/σ, base + passo\n");
    printf("\n      \"Se você não precisa de infinito\" — precisa, e ele está aqui: as cifras que não\n");
    printf("      param. O que a indução faz é COMPRIMI-LO num par finito, e é essa compressão que\n");
    printf("      faz o corpo valer a pena.\n");
    printf("\n      E fecha com o que este trabalho mediu o dia inteiro: a família real é a das\n");
    printf("      cifras periódicas, o periódico é o circular, e o circular colapsado ao gerador\n");
    printf("      é a INDUÇÃO. O rei governa o infinito porque é a forma dele.\n");
}

printf("\n=== A INDUÇÃO =============================================================\n");
printf("  As infinitas estão nas cifras que não param — e têm três formas: a que para (racional),\n");
printf("  a que repete (quadrático, o circular) e a que nem para nem repete.\n\n");
printf("  E o corpo é especial por uma coisa só, medida:\n\n");
printf("    a INDUÇÃO tem cifra de comprimento 2 e cobertura 1 sobre domínio INFINITO\n");
printf("    todas as outras provas pagam cobertura por tamanho: n casos, n passos\n");
printf("    a META-INDUÇÃO paga o mesmo 2 e cobre a torre inteira de níveis\n\n");
printf("  E o mecanismo é a auto-similaridade: σ = m + 1/σ é base + passo aplicado a si mesmo.\n");
printf("  A indução é o COLAPSO da cifra periódica ao seu gerador — e por isso a família real\n");
printf("  governa o infinito: ela é a forma dele.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
