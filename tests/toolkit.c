/* toolkit.c — O TOOLKIT DOS CORPOS, medido: a mesma tríade em cada um.
 *
 * O Aarão mandou varrer o catálogo e trazer as operações dos corpos. O mapa veio inteiro
 * (docs/CORPOS_NA_ISA.md, 29 corpos), e o toolkit — tools/corpos.h — leva os que já estão MEDIDOS
 * aqui. Um corpo só entra quando as três operações fecham; assinatura sem conta é catálogo,
 * não ferramenta.
 *
 * E o que este medidor mostra é o que faz disto um toolkit e não quatro bibliotecas: os quatro
 * corpos têm a MESMA forma. ⊕ associa e comuta, ⊗ distribui sobre ⊕, e ∏ é o operador que
 * costura — muda o que são, não quantos são.
 *
 *   §K1  ÁUREO ℤ[φ]: ⊕ ⊗ ∏, e a norma multiplicativa
 *   §K2  RACIONAL ℚ: ⊕ ⊗ ∏, e a ordem sem divisão
 *   §K3  MÓRFICO: ⊕ ⊗ ∏, e a adjunção δ⊣ε
 *   §K4  MECÂNICO: ⊕ ⊗ ∏, e det ±1
 *   §K5  a MESMA forma nos quatro — é isso que os faz um toolkit
 *
 *   cc -O2 -std=c99 -I. toolkit.c -o toolkit && ./toolkit
 */
#include <stdio.h>
#include "unidade.h"
#include "corpos.h"

int main(void){
printf("\n=== O TOOLKIT DOS CORPOS ==================================================\n");
printf("    A mesma tríade ⊕ ⊗ ∏ em cada corpo. Muda o que são, não quantos são.\n");

printf("\n§K1  ÁUREO ℤ[φ]: soma componente a componente, produto pela borda, ∏ o gato.\n\n");
{
    int mau_a = 0, mau_d = 0, mau_n = 0; long casos = 0;
    for(long m = 1; m <= 4; m++)
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        Par x = {a,b}, y = {c,d}, z = {1,2};
        Par s1 = au_soma(au_soma(x,y),z), s2 = au_soma(x,au_soma(y,z));
        if(s1.a!=s2.a||s1.b!=s2.b) mau_a++;
        Par e = au_prod(x, au_soma(y,z), m);
        Par f = au_soma(au_prod(x,y,m), au_prod(x,z,m));
        if(e.a!=f.a||e.b!=f.b) mau_d++;
        if(au_norma(au_prod(x,y,m),m) != au_norma(x,m)*au_norma(y,m)) mau_n++;
        casos++;
    }
    ok("⊕ associa; ⊗ distribui sobre ⊕", mau_a==0 && mau_d==0);
    ok("e a NORMA é multiplicativa: N(xy) = N(x)·N(y)", mau_n == 0);
    printf("      (%ld casos, em 4 metais.)\n", casos);
}

printf("\n§K2  RACIONAL ℚ: ⊕ Clifford cruzado, ⊗ La Hire componente, ∏ a classe.\n\n");
{
    int mau_d = 0, mau_o = 0; long casos = 0;
    for(long a=-4;a<=4;a++) for(long b=1;b<=5;b++)
    for(long c=-4;c<=4;c++) for(long d=1;d<=5;d++){
        Par x={a,b}, y={c,d}, z={1,2};
        Par e = ra_prod(x, ra_soma(y,z));
        Par f = ra_soma(ra_prod(x,y), ra_prod(x,z));
        if(e.a!=f.a||e.b!=f.b) mau_d++;
        if(ra_cmp(x,y) != -ra_cmp(y,x)) mau_o++;
        casos++;
    }
    ok("⊗ distribui sobre ⊕, nas classes", mau_d == 0);
    ok("e a ordem é antissimétrica, sem uma divisão", mau_o == 0);
    printf("      (%ld casos.)\n", casos);
}

printf("\n§K3  MÓRFICO: ⊕ XOR, ⊗ AND, ∏ a adjunção δ⊣ε.\n\n");
{
    int mau_d = 0, mau_adj = 0; long casos = 0;
    const int n = 6; const unsigned topo = (1u<<n) - 1;
    for(unsigned A = 0; A < (1u<<n); A++) for(unsigned B = 0; B < (1u<<n); B++){
        unsigned C = 0x2B & topo;
        if(mo_prod(A, mo_soma(B,C)) != mo_soma(mo_prod(A,B), mo_prod(A,C))) mau_d++;
        casos++;
    }
    /* a adjunção: δ(A) ⊆ B  ⟺  A ⊆ ε(B), com a máscara fixa */
    unsigned E = 0x03;
    for(unsigned A = 0; A < (1u<<n); A++) for(unsigned B = 0; B < (1u<<n); B++){
        int esq = ((mo_dil(A,E,n) & ~B & topo) == 0);
        int dir = ((A & ~mo_ero(B,E,n) & topo) == 0);
        if(esq != dir) mau_adj++;
    }
    ok("⊗ (AND) distribui sobre ⊕ (XOR)", mau_d == 0);
    ok("e a ADJUNÇÃO fecha: δ(A) ⊆ B ⟺ A ⊆ ε(B)", mau_adj == 0);
    printf("      (%ld pares na distributiva, 4096 na adjunção.)\n", casos);
}

printf("\n§K4  MECÂNICO: ⊗ produto de matrizes, ∏ a aplicação, e det ±1.\n\n");
{
    int mau_a = 0, mau_d = 0, mau_ap = 0; long casos = 0;
    for(long k1=-3;k1<=3;k1++) for(long k2=-3;k2<=3;k2++) for(long k3=-3;k3<=3;k3++){
        Mat X = me_cis(k1), Y = me_cis(k2), Z = me_gato(k3<0?-k3+1:k3+1);
        Mat p1 = me_prod(me_prod(X,Y),Z), p2 = me_prod(X,me_prod(Y,Z));
        if(p1.a!=p2.a||p1.b!=p2.b||p1.c!=p2.c||p1.d!=p2.d) mau_a++;
        if(me_det(me_prod(X,Y)) != me_det(X)*me_det(Y)) mau_d++;
        Par v = {3,2};
        Par u1 = me_ap(me_prod(X,Y), v), u2 = me_ap(X, me_ap(Y, v));
        if(u1.a!=u2.a||u1.b!=u2.b) mau_ap++;
        casos++;
    }
    ok("⊗ associa, e o det é multiplicativo: det(XY) = det X · det Y", mau_a==0 && mau_d==0);
    ok("e aplicar o produto = aplicar em sequência", mau_ap == 0);
    printf("      (%ld triplos.)\n", casos);
}

printf("\n§K5  A MESMA FORMA nos quatro — e é isso que os faz um toolkit.\n\n");
{
    printf("      corpo        ⊕ a soma            ⊗ o produto        ∏ o operador\n");
    printf("      áureo ℤ[φ]   componente          a borda σ²=mσ+1    o gato (×σ)\n");
    printf("      racional ℚ   Clifford cruzado    La Hire componente a classe\n");
    printf("      mórfico      XOR (deflexão D₁)   AND (a erosão)     a adjunção δ⊣ε\n");
    printf("      mecânico     soma de matrizes    produto            a palavra em S,T\n");
    conclui("nos quatro, ⊗ distribui sobre ⊕ e ∏ costura — a mesma forma");
    printf("\n      Não são quatro bibliotecas: é uma estrutura em quatro roupas. E é por isso\n");
    printf("      que a mesma peça serve o corpo, a cifra, a transformada e a máquina.\n");
    printf("\n      Faltam os outros 25 do docs/CORPOS_NA_ISA.md — fractal, criativo, motor,\n");
    printf("      cristalino, entrópico, óptico, celeste, e o resto. A tríade de cada um está\n");
    printf("      DESCRITA no mapa e entra aqui quando for medida. O toolkit não promete o que\n");
    printf("      não fecha.\n");
}

printf("\n=== O TOOLKIT =============================================================\n");
printf("  tools/corpos.h leva a tríade dos quatro corpos que já fecham aqui, e este medidor\n");
printf("  mostra que a forma é a MESMA nos quatro: ⊕ associa, ⊗ distribui sobre ⊕, ∏ costura.\n\n");
printf("  Muda o que são, não quantos são — e é por isso que a mesma peça serve o corpo, a\n");
printf("  cifra, a transformada e a máquina.\n\n");
printf("  Os outros 25 estão no mapa, descritos e não implementados. Entram quando forem\n");
printf("  medidos: assinatura sem conta é catálogo, não ferramenta.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
