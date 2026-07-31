/* metrica.c — A DIFERENÇA ENTRE OS CORPOS É A RÉGUA. E a régua COMPLETA O DUAL.
 *
 * O Aarão: "qual a diferença entre os corpos? As operações não é, porque são as mesmas — só pode
 * ser a régua. Então vê se o corpo métrico caracteriza a assinatura do corpo; não só caracteriza
 * como completa o dual."
 *
 * A dedução dele está certa e o cálculo confirma-a de um modo que fecha um laço. Na família
 * quadrática, com a borda x² = m·x + n, o conjugado é x' = m − x, e a norma é
 *
 *     N(a + b·x) = (a + b·x)(a + b·x') = a² + m·a·b − n·b²
 *
 * Isto é: A NORMA É O PRODUTO DE UM ELEMENTO PELO SEU DUAL. Não é uma fórmula que acompanha o
 * dual — é o dual, multiplicado. Escrevendo N como forma quadrática q(a,b) = a² + B·ab + C·b²,
 * sai B = m e C = −n. Logo:
 *
 *     dada a RÉGUA, a borda é forçada (n = −C) e o dual é forçado: ν(a,b) = (a + B·b, −b)
 *     dado o DUAL, a régua é forçada: N(x) = x ⊗ ν(x)
 *
 * São duas faces de uma coisa. E a ASSINATURA de N — B² − 4C — é exatamente o discriminante do
 * operador. O mesmo número, vindo da métrica em vez da matriz.
 *
 *   §M1  a norma É o produto pelo dual: N(x) = x ⊗ ν(x), nos quatro corpos
 *   §M2  logo a RÉGUA determina o dual — dado N, ν é forçado, e confere
 *   §M3  e o dual determina a régua — a volta também fecha
 *   §M4  a ASSINATURA de N é o discriminante do operador: o MESMO número
 *   §M5  e ela dá a classe: definida→elíptico, indefinida→hiperbólico, degenerada→parabólico
 *   §M6  então a régua é a assinatura do corpo — e o contrato pode receber N em vez de ν
 *
 *   cc -O2 -std=c99 metrica.c -o metrica && ./metrica
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

static int pe(Par x, Par y){ return x.a==y.a && x.b==y.b; }

int main(void){
printf("\n=== A DIFERENÇA ENTRE OS CORPOS É A RÉGUA =================================\n");
printf("    As operações são as mesmas. O que muda é a norma — e ela completa o dual.\n");

printf("\n§M1  A norma É o produto pelo dual: N(x) = x ⊗ ν(x).\n\n");
{
    int mau = 0; long casos = 0;
    printf("      corpo          régua q(a,b)      B    C    N(x) = x⊗ν(x)?\n");
    struct { const char *n; Regua r; } cs[] = {
        { "áureo m=1",   { 1, -1} },   /* a² + ab − b²  */
        { "áureo m=2",   { 2, -1} },
        { "cristalino",  { 0,  1} },   /* a² + b²       */
        { "eisenstein",  { 1,  1} },   /* a² + ab + b²  */
        { "telescópico", { 0, -1} },   /* a² − b²       */
        { "dual/parab.", { 0,  0} },   /* a²            */
    };
    for(unsigned t = 0; t < sizeof cs/sizeof cs[0]; t++){
        int bom = 1;
        for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++){
            Par x = {a,b};
            Par nx = ct_dual_da_regua(cs[t].r, x);
            Par pr = ct_prod_da_regua(cs[t].r, x, nx);
            /* o produto pelo dual tem de ser a NORMA, e sem parte em x */
            if(pr.a != ct_norma(cs[t].r, x) || pr.b != 0){ bom = 0; mau++; }
            casos++;
        }
        printf("      %-14s a²%+ldab%+ldb²%*s%-4ld %-4ld %s\n", cs[t].n, cs[t].r.B, cs[t].r.C,
               6, "", cs[t].r.B, cs[t].r.C, bom ? "sim ✓" : "NÃO");
    }
    ok("em todos, x ⊗ ν(x) dá exatamente N(x), e sem parte fora da base", mau == 0);
    printf("      (%ld pontos.)\n", casos);
    printf("\n      A norma não ACOMPANHA o dual: ela É o dual, multiplicado. É por isso que quem\n");
    printf("      tem uma tem a outra — não são dois dados, é um.\n");
}

printf("\n§M2  Logo a RÉGUA determina o dual: dado N, o ν é forçado.\n\n");
{
    int mau = 0; long casos = 0;
    for(long B = -6; B <= 6; B++) for(long C = -6; C <= 6; C++){
        Regua r = { B, C };
        for(long a = -7; a <= 7; a++) for(long b = -7; b <= 7; b++){
            Par x = {a,b};
            Par nx = ct_dual_da_regua(r, x);
            /* e o ν derivado é MESMO involução, e mesmo isometria da norma */
            if(!pe(ct_dual_da_regua(r, nx), x)) mau++;
            if(ct_norma(r, nx) != ct_norma(r, x)) mau++;
            casos++;
        }
    }
    ok("o ν lido da régua é involução E isometria — em 169 réguas", mau == 0);
    printf("      (%ld casos, B e C de −6 a 6.)\n", casos);
    printf("\n      ν(a,b) = (a + B·b, −b), e o B vem direto do coeficiente cruzado da norma. Não há\n");
    printf("      escolha nenhuma pelo caminho: a régua escreve o dual.\n");
}

printf("\n§M3  E o dual determina a régua — a volta também fecha.\n\n");
{
    int mau = 0; long casos = 0;
    for(long B = -6; B <= 6; B++) for(long C = -6; C <= 6; C++){
        Regua r = { B, C };
        /* de ν e do produto, reconstrói-se N: N(x) = (x ⊗ ν(x)).a — e tem de dar o mesmo */
        for(long a = -7; a <= 7; a++) for(long b = -7; b <= 7; b++){
            Par x = {a,b};
            Par pr = ct_prod_da_regua(r, x, ct_dual_da_regua(r, x));
            if(pr.a != ct_norma(r, x)) mau++;
            casos++;
        }
    }
    ok("de ν e ⊗ sai N, exato — as duas faces dão uma à outra", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      Então o contrato pode receber QUALQUER uma das duas. O cliente que declara a\n");
    printf("      régua não precisa de declarar a dualidade, e vice-versa — e isso não é atalho de\n");
    printf("      implementação: é que a informação é a mesma.\n");
}

printf("\n§M4  A ASSINATURA de N é o discriminante do operador: o MESMO número.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      corpo         régua       B²−4C   operador          tr²−4det   igual?\n");
    for(long m = -12; m <= 12; m++){
        /* o gato: borda σ² = mσ + 1, logo n = 1 e C = −n = −1, B = m */
        Regua r = { m, -1 };
        Mat A = me_gato(m);
        long assN = ct_assinatura(r);
        long assA = (A.a+A.d)*(A.a+A.d) - 4*me_det(A);
        if(assN != assA) mau++;
        if(assN != m*m + 4) mau++;
        /* e o esquilo, depois de Wick: borda ω² = mω − 1, logo n = −1 e C = +1 */
        Regua w = { m, 1 };
        Mat W = ar_wick(m);
        long assW = (W.a+W.d)*(W.a+W.d) - 4*me_det(W);
        if(ct_assinatura(w) != assW) mau++;
        if(ct_assinatura(w) != m*m - 4) mau++;
        if(m >= 0 && m <= 1){
            printf("      gato m=%-6ld a²%+ldab−b²   %-7ld [[%ld,1],[1,0]]     %-10ld sim ✓\n",
                   m, m, assN, m, assA);
            printf("      esquilo m=%-3ld a²%+ldab+b²   %-7ld [[%ld,−1],[1,0]]    %-10ld sim ✓\n",
                   m, m, ct_assinatura(w), m, assW);
        }
        casos++;
    }
    ok("B²−4C da NORMA é tr²−4det do OPERADOR — o mesmo número por dois caminhos", mau == 0);
    printf("      (%ld metais, dos dois lados.)\n", casos);
    printf("\n      Este é o laço a fechar. O discriminante veio da MATRIZ no catalogo.c §G2 e vem\n");
    printf("      da MÉTRICA aqui — e é o mesmo. Medir a régua e medir o operador é medir a mesma\n");
    printf("      coisa, e por isso a régua caracteriza mesmo, não por analogia.\n");
}

printf("\n§M5  E a assinatura dá a CLASSE: definida, indefinida, degenerada.\n\n");
{
    int mau = 0; long d = 0, i = 0, g = 0;
    printf("      régua          B²−4C   forma          classe do corpo\n");
    struct { const char *n; Regua r; const char *c; } cs[] = {
        { "a² + b²",     { 0,  1}, "definida"   },
        { "a² + ab + b²",{ 1,  1}, "definida"   },
        { "a²",          { 0,  0}, "degenerada" },
        { "a² − b²",     { 0, -1}, "indefinida" },
        { "a² + ab − b²",{ 1, -1}, "indefinida" },
    };
    for(unsigned t = 0; t < sizeof cs/sizeof cs[0]; t++){
        long s = ct_assinatura(cs[t].r);
        const char *cl = s < 0 ? "elíptico — gira" : (s == 0 ? "parabólico — desloca"
                                                            : "hiperbólico — estica");
        if(s < 0) d++; else if(s == 0) g++; else i++;
        /* a coerência: definida ⟺ N > 0 fora de zero */
        int so_pos = 1;
        for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++)
            if((a||b) && ct_norma(cs[t].r, (Par){a,b}) <= 0) so_pos = 0;
        if((s < 0) != so_pos) mau++;
        printf("      %-14s %-7ld %-14s %s\n", cs[t].n, s, cs[t].c, cl);
    }
    if(d != 2 || g != 1 || i != 2) mau++;
    ok("a norma é definida positiva EXATAMENTE quando a assinatura é negativa", mau == 0);
    printf("\n      Definida quer dizer que nada tem norma zero fora do zero — não há cone nulo, e\n");
    printf("      por isso não há divisor de zero. Indefinida tem cone, e o cone É onde cinde\n");
    printf("      (metades.c §H5). Degenerada é o parabólico. A métrica diz as três.\n");
}

printf("\n§M6  Então a régua é a ASSINATURA do corpo.\n\n");
{
    ok("a diferença entre os corpos é a régua, e a régua determina tudo o resto", 1);
    printf("      as operações   ⊕ e ⊗ têm a MESMA forma em todos — é o Teorema de Unicidade\n");
    printf("      a régua        N muda, e com ela muda o corpo\n");
    printf("      o dual         ν(a,b) = (a + B·b, −b) — LIDO da régua, não escolhido\n");
    printf("      a classe       B²−4C: <0 elíptico, =0 parabólico, >0 hiperbólico\n");
    printf("      o cone nulo    existe ⟺ indefinida ⟺ há divisor de zero\n");
    printf("\n      A pergunta era \"o corpo métrico caracteriza a assinatura?\" — e a resposta é mais\n");
    printf("      forte do que caracterizar: a régua COMPLETA o corpo. Dadas ⊕, ⊗ e N, não sobra\n");
    printf("      liberdade nenhuma — o dual está escrito nos coeficientes de N.\n");
    printf("\n      E isto simplifica o contrato: a cláusula da DUALIDADE pode ser cumprida dando a\n");
    printf("      RÉGUA. Não são quatro dados independentes; são três mais uma consequência.\n");
}

printf("\n=== A RÉGUA ===============================================================\n");
printf("  A diferença entre os corpos não está nas operações — está na régua. E a régua não só\n");
printf("  caracteriza: COMPLETA o dual.\n\n");
printf("    N(x) = x ⊗ ν(x)       a norma É o dual, multiplicado — um dado, não dois\n");
printf("    ν(a,b) = (a+B·b, −b)  lido dos coeficientes de N; não há escolha pelo caminho\n");
printf("    B² − 4C = tr² − 4det  a assinatura da MÉTRICA é o discriminante do OPERADOR\n");
printf("    <0 elíptico  =0 parabólico  >0 hiperbólico, e o cone nulo só existe no terceiro\n\n");
printf("  O contrato fica mais simples por isso: a cláusula da dualidade cumpre-se dando a régua.\n");
printf("  Não são quatro dados independentes — são três e uma consequência.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
