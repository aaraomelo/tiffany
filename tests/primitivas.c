/* tests/primitivas.c — AS CINCO PRIMITIVAS SÃO UMA: o dual emparelha com cada uma.
 *
 * O Aarão: «são 5 primitivas, acho que tem 4 no teorema e uma como corolário — acho que a
 * parte morfológica. Tira o corolário, unifica as 5 primitivas no teorema central.» E
 * depois: «a diferença é o sinal, uma involução pela Lei 2 — dá para diminuir tranquilo.»
 *
 * ── O QUE SE UNIFICA ──────────────────────────────────────────────────────────
 * As cinco operações do Corpo Universal são Soma, Multiplicação, Divisão, Dual e
 * Inversão. O `thm:derivacao-primitivas` já dizia que elas não são independentes — o dual
 * emparelha com cada uma —, mas emparelhava só na ÁLGEBRA, e deixava a parte morfológica
 * de fora, num teorema à parte do Corpo de Peano.
 *
 * O que este ficheiro mede é que são o MESMO emparelhamento, em duas categorias:
 *
 *      NA ÁLGEBRA          M + M†  = tr(M)·I        o CENTRO
 *                          M · M†  = det(M)·I       a MEMBRANA
 *                          M⁻¹     = M†/det M       a INVERSÃO é a divisão do dual
 *
 *      NA ORDEM            ε(A)    = ¬ δ(¬A)        a EROSÃO é a dilatação do dual
 *
 * e é o mesmo †. Na álgebra o dual é a adjunta (adj); na ordem é o complemento. A parte
 * morfológica não é um corolário: é o quinto emparelhamento, lido no reticulado em vez de
 * na matriz.
 *
 * ── E A DIFERENÇA MOSTRA POR QUE SÃO CINCO E NÃO SETE ─────────────────────────
 * A subtracção não está na lista, e não falta: a − b é a ⊕ b†, a soma composta com o
 * sinal. É o emparelhamento mais barato que há, e é ele que explica o padrão — cada
 * operação, emparelhada com o dual, dá a sua parceira, e a parceira não é uma primitiva
 * nova. Pela mesma conta, a divisão é o produto pela inversa, e a inversão é o dual a
 * dividir pelo determinante.
 *
 * ── O QUE ISTO NÃO AFIRMA ─────────────────────────────────────────────────────
 * Não afirma que as cinco se reduzem a uma. Afirma que o DUAL é transversal: ele não é
 * uma operação ao lado das outras quatro, é o que as liga. E na ordem o par não é uma
 * involução — ε∘δ e δ∘ε são idempotentes e não a identidade, que é uma ADJUNÇÃO e não uma
 * bijeção dual. Essa diferença mede-se aqui, porque juntá-las seria o erro.
 *
 * §P0  soma  + dual = o CENTRO          M + M† = tr(M)·I
 * §P1  mult  + dual = a MEMBRANA        M · M† = det(M)·I
 * §P2  divisão + dual = a INVERSÃO      M⁻¹ = M†/det M, e a fibra vazia quando det = 0
 * §P3  dilatação + dual = a EROSÃO      ε(A) = ¬δ(¬A), no reticulado
 * §P4  a diferença é soma + sinal       a − b = a ⊕ b†, e por isso não é primitiva
 * §P5  e o que NÃO se junta: na álgebra é involução, na ordem é adjunção
 */
#include <stdio.h>
#include "dual32.h"
#include "racionais.h"
#include "linear.h"
#include "unidade.h"

/* ── a adjunta 2×2: o dual da álgebra ──────────────────────────────────────────*/
static Mat adj2(Mat M){
    Mat R = mat0(2,2);
    R.a[0][0] = M.a[1][1];              R.a[0][1] = qz_oposto(M.a[0][1]);
    R.a[1][0] = qz_oposto(M.a[1][0]);   R.a[1][1] = M.a[0][0];
    return R;
}
static Mat esc2(Qz l, Mat M){
    Mat R = mat0(2,2);
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++) R.a[i][j] = qz_mult(l, M.a[i][j]);
    return R;
}
static Mat som2(Mat A, Mat B){
    Mat R = mat0(2,2);
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
        R.a[i][j] = qz_soma(A.a[i][j], B.a[i][j]);
    return R;
}
static int ig2(Mat A, Mat B){
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
        if(!qz_igual(A.a[i][j], B.a[i][j])) return 0;
    return 1;
}
/* ── o reticulado: subconjuntos de ℤ_12 como máscaras, e a máscara SIMÉTRICA ────
 * O dual da ordem é o complemento; a dilatação alarga pela vizinhança e a erosão é a
 * dilatação do complemento. Com a vizinhança simétrica a lei fecha; é ela a hipótese. */
#define PR_M 12
static unsigned pr_dilata(unsigned A){
    unsigned R = 0;
    for(int s = 0; s < PR_M; s++) if(A & (1u << s)){
        R |= 1u << ((s + PR_M - 1) % PR_M);
        R |= 1u << s;
        R |= 1u << ((s + 1) % PR_M);
    }
    return R;
}
static unsigned pr_erode(unsigned A){
    unsigned R = 0;
    for(int s = 0; s < PR_M; s++){
        unsigned viz = (1u << ((s + PR_M - 1) % PR_M)) | (1u << s) | (1u << ((s + 1) % PR_M));
        if((A & viz) == viz) R |= 1u << s;
    }
    return R;
}
static unsigned pr_nao(unsigned A){ return (~A) & ((1u << PR_M) - 1); }
/* a mutação: uma vizinhança ASSIMÉTRICA, onde a lei tem de quebrar */
static unsigned pr_dilata_ass(unsigned A){
    unsigned R = 0;
    for(int s = 0; s < PR_M; s++) if(A & (1u << s)){
        R |= 1u << s;
        R |= 1u << ((s + 1) % PR_M);
        R |= 1u << ((s + 2) % PR_M);
    }
    return R;
}
static unsigned pr_erode_ass(unsigned A){
    unsigned R = 0;
    for(int s = 0; s < PR_M; s++){
        unsigned viz = (1u << s) | (1u << ((s+1) % PR_M)) | (1u << ((s+2) % PR_M));
        if((A & viz) == viz) R |= 1u << s;
    }
    return R;
}

int main(void){
    printf("\n=== AS CINCO PRIMITIVAS SÃO UMA: o dual emparelha com cada uma ===\n");

    /* ═══ §P0 SOMA + DUAL = O CENTRO ══════════════════════════════════════════ */
    printf("\n§P0 soma + dual = o CENTRO:  M + M† = tr(M)·I\n\n");
    {
        long mal = 0, casos = 0;
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
        for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
            long v[4] = { a, b, c, d };
            Mat M = mat_de_inteiros(2,2,v);
            Mat S = som2(M, adj2(M));
            Mat T = esc2(qz_de_inteiro(a + d), mat_id(2));
            casos++;
            if(!ig2(S, T)) mal++;
        }
        printf("      M + M† = tr(M)·I em %ld matrizes: %ld divergências\n", casos, mal);
        ok("SOMA COM O DUAL DÁ O CENTRO, e o resultado é um ESCALAR vezes a identidade —"
           " toda a parte que distingue as direcções cancela-se. É a primeira das cinco"
           " ligações, e ela diz o que o dual faz: guarda exactamente a metade que a soma"
           " apagaria",
           mal == 0 && casos == 14641);
    }

    /* ═══ §P1 MULTIPLICAÇÃO + DUAL = A MEMBRANA ══════════════════════════════ */
    printf("\n§P1 multiplicação + dual = a MEMBRANA:  M·M† = det(M)·I\n\n");
    {
        long mal = 0, casos = 0;
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
        for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
            long v[4] = { a, b, c, d };
            Mat M = mat_de_inteiros(2,2,v);
            Mat P = mat_mult(M, adj2(M));
            Mat D = esc2(qz_de_inteiro(a*d - b*c), mat_id(2));
            casos++;
            if(!ig2(P, D)) mal++;
        }
        printf("      M·M† = det(M)·I em %ld matrizes: %ld divergências\n", casos, mal);
        ok("MULTIPLICAÇÃO COM O DUAL DÁ A MEMBRANA, e é o mesmo gesto do §P0 com a outra"
           " operação: outra vez um escalar vezes a identidade, e o escalar é agora o"
           " DETERMINANTE. A soma dá o traço, o produto dá o determinante — os dois"
           " coeficientes do polinómio característico, cada um de uma operação",
           mal == 0 && casos == 14641);
    }

    /* ═══ §P2 DIVISÃO + DUAL = A INVERSÃO, e a fibra vazia ═══════════════════ */
    printf("\n§P2 divisão + dual = a INVERSÃO:  M⁻¹ = M†/det M\n\n");
    {
        long mal = 0, inv = 0, sem = 0, casos = 0;
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
        for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
            long v[4] = { a, b, c, d };
            Mat M = mat_de_inteiros(2,2,v);
            long det = a*d - b*c;
            casos++;
            if(det == 0){ sem++; continue; }        /* a fibra é VAZIA — sem inversa */
            Qz id;
            if(!qz_divide(qz(1,1), qz_de_inteiro(det), &id)){ mal++; continue; }
            Mat I2 = esc2(id, adj2(M));
            if(!ig2(mat_mult(M, I2), mat_id(2))) mal++;
            inv++;
        }
        printf("      M·(M†/det) = I em %ld invertíveis: %ld divergências;  e %ld com"
               " det = 0 — a fibra vazia\n", inv, mal, sem);
        ok("A INVERSÃO É A DIVISÃO DO DUAL, e por isso não é uma primitiva nova: escreve-se"
           " com as que já há. E o caso em que ela não existe diz-se — det = 0, a mesma"
           " FIBRA VAZIA do 0⁻¹ que a escada aritmética desta casa encontrou em todo andar."
           " Uma operação com fibra vazia não é uma falha da operação: é onde ela não está"
           " definida, e contá-lo é parte de a definir",
           mal == 0 && inv > 0 && sem > 0 && casos == 14641);
    }

    /* ═══ §P3 DILATAÇÃO + DUAL = A EROSÃO — e é o MESMO † noutra categoria ═══ */
    printf("\n§P3 dilatação + dual = a EROSÃO:  ε(A) = ¬δ(¬A), no reticulado\n\n");
    {
        long mal = 0, casos = 0;
        for(unsigned A = 0; A < (1u << PR_M); A++){
            casos++;
            if(pr_erode(A) != pr_nao(pr_dilata(pr_nao(A)))) mal++;
        }
        /* a MUTAÇÃO: com a vizinhança assimétrica a lei tem de quebrar */
        long quebra = 0;
        for(unsigned A = 0; A < (1u << PR_M); A++)
            if(pr_erode_ass(A) != pr_nao(pr_dilata_ass(pr_nao(A)))) quebra++;
        printf("      ε(A) = ¬δ(¬A) nos %ld subconjuntos de ℤ₁₂: %ld divergências\n",
               casos, mal);
        printf("      e com a vizinhança ASSIMÉTRICA a lei quebra em %ld deles — a"
               " simetria é hipótese, não decoração\n", quebra);
        ok("E O QUINTO EMPARELHAMENTO É O MORFOLÓGICO, que estava num teorema à parte:"
           " ε(A) = ¬δ(¬A) é o MESMO gesto dos três anteriores, com o dual a ser o"
           " COMPLEMENTO em vez da adjunta. A erosão não é uma operação independente da"
           " dilatação — é a dilatação do dual, exactamente como a inversão é a divisão do"
           " dual. Logo não é corolário de coisa nenhuma: é a quinta linha do mesmo"
           " teorema, lida no reticulado em vez de na matriz. E a hipótese diz-se: com a"
           " vizinhança assimétrica a lei quebra, e o contra-caso está contado",
           mal == 0 && casos == 4096 && quebra > 0);
    }

    /* ═══ §P4 A DIFERENÇA É SOMA + SINAL — e por isso não é primitiva ════════ */
    printf("\n§P4 a diferença é a soma composta com o sinal — e não entra na lista.\n\n");
    {
        long mal = 0, casos = 0;
        for(long ap = -20; ap <= 20; ap++) for(long aq = 1; aq <= 8; aq++)
        for(long bp = -20; bp <= 20; bp++) for(long bq = 1; bq <= 8; bq++){
            Qz a = qz(ap,aq), b = qz(bp,bq);
            casos++;
            /* a − b construído SÓ com ⊕ e † */
            Qz dif = qz_soma(a, qz_oposto(b));
            Qz ref = qz(ap*bq - bp*aq, aq*bq);
            if(!qz_igual(dif, ref)) mal++;
        }
        /* e a involução: †∘† = id */
        long vmal = 0, vc = 0;
        for(long p = -30; p <= 30; p++) for(long q = 1; q <= 10; q++){
            Qz x = qz(p,q);
            vc++;
            if(!qz_igual(qz_oposto(qz_oposto(x)), x)) vmal++;
        }
        printf("      a − b = a ⊕ b† em %ld pares: %ld divergências\n", casos, mal);
        printf("      e †∘† = id em %ld racionais: %ld divergências (Lei 1)\n", vc, vmal);
        ok("A DIFERENÇA NÃO É UMA PRIMITIVA, E É ELA QUE EXPLICA O PADRÃO: a − b é a soma"
           " composta com o sinal, e o sinal é o dual — uma involução, †∘† = id. É o"
           " emparelhamento mais barato que há, e mostra que a lista tem CINCO e não sete"
           " pela mesma razão que a inversão não é nova: cada operação emparelhada com o"
           " dual dá a sua parceira, e a parceira não entra na lista",
           /* e o número de casos é a CONTA dos laços, não um que eu tenha somado de
            * cabeça: escrevi 108224 e são 41·8·41·8 = 107584. A referência escrita à mão
            * é o defeito desta casa, e aqui ela apareceu numa asserção sobre primitivas. */
           mal == 0 && vmal == 0 && casos == 41L*8L*41L*8L);
    }

    /* ═══ §P5 O QUE NÃO SE JUNTA: involução na álgebra, ADJUNÇÃO na ordem ════ */
    printf("\n§P5 O que NÃO se junta: na álgebra é involução, na ordem é adjunção.\n\n");
    {
        /* na álgebra: adj(adj(M)) = M — involução */
        long amal = 0, ac = 0;
        for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
        for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
            long v[4] = { a, b, c, d };
            Mat M = mat_de_inteiros(2,2,v);
            ac++;
            if(!ig2(adj2(adj2(M)), M)) amal++;
        }
        /* na ordem: δε ⊆ A ⊆ εδ, e NENHUM é a identidade */
        long dentro = 0, fora = 0, id_de = 0, id_ed = 0, oc = 0;
        for(unsigned A = 0; A < (1u << PR_M); A++){
            unsigned de = pr_dilata(pr_erode(A));       /* abertura: ⊆ A */
            unsigned ed = pr_erode(pr_dilata(A));       /* fecho:    ⊇ A */
            oc++;
            if((de & A) == de) dentro++;
            if((A & ed) == A) fora++;
            if(de == A) id_de++;
            if(ed == A) id_ed++;
        }
        printf("      na ÁLGEBRA: adj∘adj = id em %ld matrizes, %ld divergências —"
               " involução\n", ac, amal);
        printf("      na ORDEM: δε ⊆ A em %ld/%ld e A ⊆ εδ em %ld/%ld;  e δε = A só em"
               " %ld, εδ = A só em %ld\n", dentro, oc, fora, oc, id_de, id_ed);
        ok("E O QUE NÃO SE JUNTA DIZ-SE, senão a unificação era um atalho: na ÁLGEBRA o"
           " dual é uma INVOLUÇÃO — adj∘adj = id, a Lei 1 desta casa. Na ORDEM não é: δε"
           " está CONTIDA em A e A está contida em εδ, e nenhum dos dois é a identidade"
           " senão numa minoria dos conjuntos. Isso é uma ADJUNÇÃO, não uma bijeção dual —"
           " a mesma distinção que o cone e a espiral já tinham obrigado a fazer, entre"
           " ν∘ν = id e Σ∘Π = Id. O emparelhamento é o mesmo; a natureza do par não é, e"
           " chamar-lhes a mesma coisa seria o erro que esta secção existe para não"
           " cometer",
           amal == 0 && dentro == oc && fora == oc && id_de < oc && id_ed < oc);
    }

    printf("\n=== %ld asserções, %ld falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
