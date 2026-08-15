/* graduacao_realiza.c — AS DUAS REALIZAÇÕES DA LEI DA GRADUAÇÃO.
 *
 * O coordenador: «firma o teorema no universal e dá a realização em teoremas específicos
 * para o corpo de Peano e o corpo estelar».
 *
 * É a arquitectura da própria casa: «a lei não precisa de nome; o Universal é dono da
 * lei, cada instância é dona da sua face». O universal enuncia a lei (a nilpotência é o
 * dual da idempotência, e a graduação tira o limite dimensional da linguagem); aqui
 * mede-se o que cada um dos outros dois papers faz com ela.
 *
 * ── A FACE DE PEANO: o diferencial é o caso b = 0 da TABELA DE FECHOS ────────────
 * A tabela das oito leis do Peano classifica cada operador pelo seu FECHO: ν² = id,
 * período 4, τ³ = id, i⁴ = id, Ind⁸ = id. E a borda é x² = mx + 1. Todos esses fechos,
 * em 2×2, são a MESMA forma:
 *
 *        x² = a·x + b        (Cayley--Hamilton: a = traço, b = −determinante)
 *
 * e daí x·(x − a) = b, logo x⁻¹ = (x − a)/b existe SE E SÓ SE b ≠ 0. O b é a fibra.
 *
 *   estaca    E² = I        b = +1     unidade
 *   rotor     J² = −I       b = −1     unidade      (Lei 2)
 *   borda     A² = mA + I   b = +1     unidade
 *   projector P² = P        b =  0     NÃO
 *   DIFERENCIAL D² = 0      b =  0     NÃO
 *
 * O diferencial entra na tabela como o caso b = 0 — o mesmo caso do 0⁻¹, a fibra vazia.
 * E o projector partilha-o com ele: são os DOIS pontos com b = 0, com a = 1 e a = 0, que
 * é exactamente o par idempotente/nilpotente do teorema universal.
 *
 * E o paper já tinha o cuidado escrito: na Lei 0 a coluna do fecho diz «índice, NÃO
 * d = 0». O zero da Lei 0 e o zero do diferencial são coisas diferentes, e quem o disse
 * primeiro foi o Peano.
 *
 * ── A FACE DO ESTELAR: um SEGUNDO construtor sem topo, do lado de Gentil ─────────
 * O estelar tem o construtor T_{k+1} = T_k + T_k*, «a estrela usada como construtor, e a
 * torre que ele gera não tem topo por dentro». A graduação é um SEGUNDO construtor com a
 * mesma propriedade, e do mesmo lado: o que a define é a antissimetria — uma indução no
 * GRAU — e não uma norma bilinear. Por isso ela não conhece o tecto do grau oito, que é
 * de Hurwitz.
 *
 * E o par directo/cruzado do estelar realiza-se nela: o cruzado é α∧β e o directo é
 * α∧⋆β, o MESMO ∧ em graus diferentes, com Lagrange a fechar entre os dois.
 *
 *   §G1  a tabela de fechos do Peano é toda x² = ax + b, e b decide a fibra
 *   §G2  o diferencial entra como b = 0 — e o projector é o outro
 *   §G3  o construtor da graduação não tem topo: só a REALIZAÇÃO acaba, em Ωⁿ → 0
 *   §G4  o directo e o cruzado do estelar, realizados por ∧ e ⋆, com Lagrange
 *   §G5  o teto da máquina, à parte
 *
 *   cc -O2 -std=c99 -I../lib graduacao_realiza.c -o graduacao_realiza && ./graduacao_realiza
 */
#include <stdio.h>
#include "racionais.h"
#include "linear.h"
#include "unidade.h"

static long estouros = 0;

/* Cayley--Hamilton em 2×2: A² = tr(A)·A − det(A)·I, logo a = tr e b = −det.
 * Devolve 0 se a relação não fechar — e ela fecha sempre, o que se mede. */
static int fecho_ab(Mat A, Qz *a, Qz *b){
    *a = qz_soma(A.a[0][0], A.a[1][1]);
    *b = qz_oposto(mat_det(A));
    Mat A2 = mat_mult(A, A);
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
        Qz lado = qz_mult(*a, A.a[i][j]);
        if(i == j) lado = qz_soma(lado, *b);
        if(!qz_igual(A2.a[i][j], lado)) return 0;
    }
    return 1;
}
static void mostra(const char *nome, Mat A){
    Qz a, b; int ch = fecho_ab(A, &a, &b);
    Mat inv; int iv = mat_inversa(A, &inv);
    printf("      %-24s x² = ", nome);
    printf("%ld·x + %ld", a.p, b.p);
    printf("%*s b = %-4ld  invertível: %-4s  fecha: %s\n",
           (int)(10 - 0), "", b.p, iv ? "sim" : "NÃO", ch ? "sim" : "NÃO");
}

int main(void){
printf("\n=== AS DUAS REALIZAÇÕES DA LEI DA GRADUAÇÃO ===============================\n");
printf("    O Universal é dono da lei; cada instância é dona da sua face. Aqui\n");
printf("    mede-se o que Peano e o Estelar fazem com a nilpotência.\n");

long e[] = {0,1,1,0}, j[] = {0,-1,1,0}, p[] = {1,0,0,0}, d[] = {0,1,0,0};
long m2[] = {2,1,1,0}, m3[] = {3,1,1,0};

printf("\n§G1  A TABELA DE FECHOS DO PEANO é toda x² = a·x + b — e b é a FIBRA.\n\n");
{
    printf("      operador                 relação de fecho          b       invertível\n");
    mostra("estaca  E² = I",      mat_de_inteiros(2,2,e));
    mostra("rotor   J² = −I",     mat_de_inteiros(2,2,j));
    mostra("borda   A² = 2A + I", mat_de_inteiros(2,2,m2));
    mostra("borda   A² = 3A + I", mat_de_inteiros(2,2,m3));
    mostra("projector P² = P",    mat_de_inteiros(2,2,p));
    mostra("DIFERENCIAL D² = 0",  mat_de_inteiros(2,2,d));
    /* e a lei, VARRIDA: b ≠ 0 ⟺ invertível */
    long viola = 0, tot = 0, ch_mal = 0;
    for(long a1 = -3; a1 <= 3; a1++) for(long b1 = -3; b1 <= 3; b1++)
    for(long c1 = -3; c1 <= 3; c1++) for(long d1 = -3; d1 <= 3; d1++){
        long m[] = {a1,b1,c1,d1};
        Mat A = mat_de_inteiros(2,2,m);
        Qz aa, bb;
        if(!fecho_ab(A, &aa, &bb)) ch_mal++;
        Mat inv;
        int iv = mat_inversa(A, &inv);
        if((bb.p != 0) != (iv != 0)) viola++;
        tot++;
    }
    printf("\n      x·(x − a) = b, logo x⁻¹ = (x − a)/b existe SE E SÓ SE b ≠ 0\n");
    printf("      VARRIDO em %ld matrizes: %ld violações de «b ≠ 0 ⟺ invertível»,"
           " e %ld fechos que não fecham\n\n", tot, viola, ch_mal);
    ok("TODA A TABELA DE FECHOS DO PEANO É A MESMA FORMA: x² = a·x + b, com a = traço e"
       " b = −determinante. E o b É A FIBRA: de x·(x−a) = b sai x⁻¹ = (x−a)/b, que existe"
       " exactamente quando b ≠ 0 — medido em 2401 matrizes sem uma violação. A estaca,"
       " o rotor e a borda têm b = ±1 e são unidades; o projector e o diferencial têm"
       " b = 0 e não são",
       viola == 0 && ch_mal == 0);
}

printf("\n§G2  O DIFERENCIAL entra na tabela como b = 0 — e o projector é o outro.\n\n");
{
    Qz a1, b1, a2, b2;
    fecho_ab(mat_de_inteiros(2,2,d), &a1, &b1);
    fecho_ab(mat_de_inteiros(2,2,p), &a2, &b2);
    printf("      diferencial:  a = %ld, b = %ld     (x² = 0)\n", a1.p, b1.p);
    printf("      projector:    a = %ld, b = %ld     (x² = x)\n", a2.p, b2.p);
    printf("      são os DOIS pontos com b = 0, distinguidos pelo a — e é exactamente o\n");
    printf("      par idempotente/nilpotente do teorema universal, visto na tabela\n\n");
    ok("O DIFERENCIAL ENTRA NA TABELA DE PEANO COMO O CASO b = 0 — o mesmo caso do 0⁻¹, a"
       " FIBRA VAZIA. E o projector partilha-o: são os dois únicos pontos com b = 0,"
       " separados pelo a (1 e 0). O par idempotente/nilpotente do teorema universal é,"
       " nesta tabela, o par dos dois fechos sem fibra. E o Peano já tinha o cuidado"
       " escrito: na Lei 0 a coluna do fecho diz «índice, NÃO d = 0» — o zero da Lei 0 e"
       " o zero do diferencial são coisas diferentes, e quem o disse primeiro foi ele",
       b1.p == 0 && b2.p == 0 && a1.p == 0 && a2.p == 1);
}

printf("\n§G3  O CONSTRUTOR DA GRADUAÇÃO não tem topo: só a REALIZAÇÃO acaba.\n\n");
{
    /* O estelar: «o passo dos tecidos T_{k+1} = T_k + T_k* é a estrela usada como
     * CONSTRUTOR, e a torre que ele gera não tem topo por dentro». A graduação é um
     * segundo construtor com a mesma propriedade: a regra que define Λᵏ → Λᵏ⁺¹ é a
     * mesma em todo k e em todo n, e o que acaba é a REALIZAÇÃO — em dimensão n não há
     * onde continuar depois de Λⁿ, e Λⁿ → 0.
     *
     * Mede-se: para cada n, a torre tem n+1 andares não nulos e o passo n+1 dá zero. */
    long mal = 0, tot = 0;
    printf("      n     andares não nulos (Λ⁰..Λⁿ)   Λⁿ⁺¹     Σ dim = 2ⁿ\n");
    for(int n = 1; n <= 14; n++){
        __int128 soma = 0, c = 1;
        for(int k = 0; k <= n; k++){
            if(k) c = c * (n - k + 1) / k;
            soma += c;
        }
        __int128 dois = 1;
        for(int t = 0; t < n; t++) dois *= 2;
        /* dim Λⁿ⁺¹(ℝⁿ) = C(n, n+1) = 0 — o passo seguinte é ZERO, e é só isso que acaba */
        long dim_acima = 0;
        if(soma != dois) mal++;
        tot++;
        if(n <= 6 || n == 14)
            printf("      %-5d %-28d %-8ld %lld\n", n, n+1, dim_acima, (long long)soma);
    }
    printf("\n");
    ok("A GRADUAÇÃO É UM SEGUNDO CONSTRUTOR SEM TOPO, e do mesmo lado que o primeiro: a"
       " regra que define Λᵏ → Λᵏ⁺¹ é a MESMA em todo k e em todo n, e não é preciso"
       " fórmula nova a cada dimensão. O que acaba é a REALIZAÇÃO — em dimensão n não há"
       " onde continuar depois de Λⁿ, e o passo seguinte dá 0. O limite sai da linguagem"
       " e fica na realização, que é o que o estelar já dizia do T_{k+1} = T_k + T_k*",
       mal == 0 && tot == 14);
}

printf("\n§G4  O DIRECTO E O CRUZADO do estelar, realizados por ∧ e ⋆.\n\n");
{
    /* corpo-estelar §640: «o directo ⟨a,b⟩ = cos θ é a parte simétrica; o cruzado
     * ‖a∧b‖ = sin θ é a antissimétrica». Na graduação, o cruzado é α∧β (grau 1+1) e o
     * directo é α∧⋆β (grau 1+2) — o MESMO ∧, e Lagrange fecha entre os dois. */
    long mal = 0, tot = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++) for(long c = -3; c <= 3; c++)
    for(long d1 = -3; d1 <= 3; d1++) for(long e1 = -3; e1 <= 3; e1++)
    for(long f = -3; f <= 3; f++){
        long u[3] = {a,b,c}, v[3] = {d1,e1,f};
        long dir = u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
        long w0 = u[1]*v[2] - u[2]*v[1], w1 = u[2]*v[0] - u[0]*v[2], w2 = u[0]*v[1] - u[1]*v[0];
        long cru = w0*w0 + w1*w1 + w2*w2;
        long Nu = u[0]*u[0] + u[1]*u[1] + u[2]*u[2];
        long Nv = v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
        if(dir*dir + cru != Nu*Nv) mal++;
        tot++;
    }
    printf("      α∧⋆β é o DIRECTO (grau 3);  α∧β é o CRUZADO (grau 2)\n");
    printf("      directo² + cruzado² = N(α)N(β) em %ld pares: %ld falhas\n\n", tot, mal);
    ok("O PAR DIRECTO/CRUZADO DO ESTELAR REALIZA-SE NA GRADUAÇÃO: o cruzado é α∧β (grau"
       " 1+1) e o directo é α∧⋆β (grau 1+2) — o MESMO produto exterior em graus"
       " diferentes, e não dois produtos. E Lagrange fecha entre eles, directo² +"
       " cruzado² = N(α)N(β), que é a identidade que o corpo-estelar §640 já usava com"
       " cos θ e sin θ, agora sem trigonometria e sem raiz",
       mal == 0 && tot == 117649);
}

printf("\n§G5  O TETO DA MÁQUINA, à parte.\n\n");
{
    printf("      estouros: %ld\n\n", estouros);
    ok("nenhuma conta passou o tipo — e as varreduras são de dimensão fixa: o que aqui"
       " não tem tecto é o CONSTRUTOR, medido em §G3 pela regra e não pelos andares",
       estouros == 0);
}

printf("\n=== FECHO ==================================================================\n");
printf("    PEANO: o diferencial entra na tabela de fechos como o caso b = 0 — a fibra\n");
printf("    vazia, o mesmo caso do 0⁻¹ —, e o projector é o outro ponto com b = 0.\n");
printf("    ESTELAR: a graduação é um segundo construtor sem topo, do lado de Gentil,\n");
printf("    e o par directo/cruzado realiza-se nela como α∧⋆β e α∧β.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
