/* meio_aberto.c — [0,1[ É MAIS ADEQUADO: dá a bijeção E devolve o inverso que faltava.
 *
 * O Aarão: "e [0,1] é problema seu — tem elementos, tem operações, tem corpo ou não? E você disse
 * ali que elíptico não ordena — ordena ou não? E você não acha que seria mais adequado [0,1[ pra
 * ter bijeção?"
 *
 * Sim, é mais adequado — e por duas razões, não uma:
 *
 *   BIJEÇÃO   em [0,1] os pontos 0 e 1 são o MESMO ângulo (a volta fecha). O fechado conta-o
 *             duas vezes; [0,1[ conta uma. É o domínio fundamental do círculo.
 *
 *   O INVERSO ele VOLTA. Com a soma módulo 1, todo x tem oposto: x + (1−x) = 1 ≡ 0. É
 *             exatamente o que faltava em [0,1] e que me fez dizer "não é corpo".
 *
 * Então a objeção que eu levantei desaparece ao trocar o intervalo — e o intervalo fechado era
 * escolha minha, mais uma vez.
 *
 *   §H1  em [0,1] os extremos são o mesmo ângulo — a bijeção falha, e conta-se duas vezes
 *   §H2  em [0,1[ a bijeção fecha — um ponto por ângulo
 *   §H3  e o INVERSO volta: x + (1−x) ≡ 0 mod 1, para todo x
 *   §H4  o que isso faz e o que NÃO faz — honesto
 *
 *   cc -O2 -std=c99 meio_aberto.c -o meio_aberto && ./meio_aberto
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

static Par q(long a, long b){ return ra_classe((Par){a,b}); }
/* a soma no círculo: x + y, e se passar de 1, tira-se 1 */
static Par som1(Par x, Par y){
    Par s = ra_soma(x,y);
    if(ra_cmp(s, q(1,1)) >= 0) s = ra_soma(s, q(-1,1));
    return s;
}

int main(void){
printf("\n=== [0,1[ ================================================================\n");
printf("    Mais adequado, e por duas razões — a bijeção e o inverso.\n");

printf("\n§H1  Em [0,1] os extremos são o MESMO ângulo. Conta-se duas vezes.\n\n");
{
    int mau = 0;
    /* o ângulo é a fração da volta. 0 e 1 são a mesma direção — a volta fechou. */
    Par zero = q(0,1), um = q(1,1);
    /* como ângulos: 0 e 1 diferem de exatamente uma volta */
    Par dif = ra_soma(um, (Par){-zero.a, zero.b});
    if(ra_cmp(dif, q(1,1)) != 0) mau++;
    printf("      0 e 1 diferem de %ld/%ld volta — são a MESMA direção\n", dif.a, dif.b);
    printf("      logo [0,1] tem DOIS nomes para um ponto: a bijeção falha nos extremos\n");
    ok("em [0,1] os extremos são o mesmo ângulo — o fechado conta-o duas vezes", mau == 0);
    printf("\n      É o mesmo defeito que eu já tinha visto noutro sítio hoje e não liguei: (2,1) e\n");
    printf("      (4,2) eram dois nomes do mesmo elemento. Aqui são 0 e 1.\n");
}

printf("\n§H2  Em [0,1[ a bijeção FECHA — um ponto por ângulo.\n\n");
{
    int mau = 0; long casos = 0, dups = 0;
    for(long p=0;p<24;p++) for(long r=1;r<=24;r++){
        Par a = q(p,r);
        if(ra_cmp(a, q(1,1)) >= 0) continue;          /* [0,1[ : exclui o 1 */
        for(long s=0;s<24;s++) for(long t=1;t<=24;t++){
            Par b = q(s,t);
            if(ra_cmp(b, q(1,1)) >= 0) continue;
            /* dois ângulos distintos em [0,1[ nunca são a mesma direção */
            if(ra_cmp(a,b) != 0){
                Par d = ra_soma(a, (Par){-b.a, b.b});
                if(ra_cmp(d, q(1,1)) == 0 || ra_cmp(d, q(-1,1)) == 0) dups++;
            }
        }
        casos++;
    }
    if(dups) mau++;
    ok("em [0,1[ dois pontos distintos nunca são o mesmo ângulo — a bijeção fecha", mau == 0);
    printf("      (%ld ângulos, %ld duplicados.)\n", casos, dups);
    printf("\n      É o domínio fundamental do círculo, e é a base do rei: a volta inteira é a\n");
    printf("      unidade, e cada ângulo tem UM nome.\n");
}

printf("\n§H3  E o INVERSO VOLTA: x + (1−x) ≡ 0 mod 1, para todo x.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      x         1−x       x + (1−x) mod 1\n");
    for(long p=0;p<30;p++) for(long r=1;r<=30;r++){
        Par x = q(p,r);
        if(ra_cmp(x, q(1,1)) >= 0) continue;
        Par op = ra_soma(q(1,1), (Par){-x.a, x.b});
        if(ra_cmp(op, q(1,1)) >= 0) op = q(0,1);      /* o oposto de 0 é 0 */
        Par s = som1(x, op);
        if(ra_cmp(s, q(0,1)) != 0) mau++;
        casos++;
    }
    printf("      1/3       2/3       0/1 ✓\n");
    printf("      3/4       1/4       0/1 ✓\n");
    printf("      0/1       0/1       0/1 ✓   ← o zero é o seu próprio oposto\n");
    ok("em [0,1[ com soma mod 1, TODO x tem oposto — o inverso que faltava VOLTA", mau == 0);
    printf("      (%ld ângulos.)\n", casos);
    printf("\n      E era ESTA a objeção que eu levantei contra [0,1]: \"falta o inverso aditivo,\n");
    printf("      logo não é corpo\". A objeção some ao trocar o intervalo — e o intervalo fechado\n");
    printf("      era escolha MINHA.\n");
}

printf("\n§H4  O que isso faz, e o que NÃO faz.\n\n");
{
    conclui("[0,1[ com soma mod 1 é GRUPO — e o que falta para corpo é a segunda operação");
    printf("      FAZ        a bijeção com o círculo: um nome por ângulo\n");
    printf("      FAZ        o inverso aditivo volta: x + (1−x) ≡ 0\n");
    printf("      FAZ        e é a base do rei — a volta inteira por unidade\n");
    printf("      NÃO FAZ    sozinho não dá corpo: é GRUPO (ℝ/ℤ), com uma operação\n");
    printf("\n      Para corpo faltaria o ⊗ fechado em [0,1[ compatível com esta soma. Não o tenho, e\n");
    printf("      não invento — mas a objeção que eu tinha levantado, essa, caiu.\n");
    printf("\n      E o que fica dito com clareza, que é o que ele pediu:\n");
    printf("        [0,1] com aquelas operações   NÃO é corpo — falta o inverso aditivo\n");
    printf("        o elíptico                     ORDENA — pela elongação, medido\n");
    printf("        [0,1[ é mais adequado          SIM — bijeção E inverso, as duas\n");
}

printf("\n=== [0,1[ =================================================================\n");
printf("  Mais adequado, e por duas razões:\n\n");
printf("    a bijeção   em [0,1] o 0 e o 1 são o MESMO ângulo — dois nomes, um ponto.\n");
printf("                Em [0,1[ cada ângulo tem um nome só: é o domínio fundamental\n");
printf("    o inverso   com soma mod 1, x + (1−x) ≡ 0 para TODO x — volta o que faltava\n\n");
printf("  E a objeção que eu levantara contra [0,1] cai com a troca do intervalo, que era escolha\n");
printf("  minha. O que fica: [0,1[ com soma mod 1 é GRUPO. Para corpo faltaria o ⊗, e esse não o\n");
printf("  tenho — e não invento.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais.\n\n");
return 0;
}
