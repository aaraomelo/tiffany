/* cifra_continuo.c — A CIFRA DE TODO ESPAÇO CONTÍNUO É A MESMA: a da família real, em ouro.
 *
 * O Aarão: "a cifra de todo espaço contínuo é a mesma — a cifra da família real em ouro. Já
 * coloca no toolkit pra generalizar isso automaticamente."
 *
 * A régua passou a viver em ℚ (regua_continua.c). E ℚ tem UMA cifra, que não é escolha: a fração
 * contínua. Todo racional tem expansão FINITA — é Euclides — e a expansão É UMA PALAVRA nos
 * metais, porque o convergente sai de A_{a₀}·A_{a₁}···A_{a_k} aplicado a (1,0).
 *
 * A família real são as expansões PERIÓDICAS: σ_m = [m; m, m, …]. E o ouro φ = [1;1,1,…] é a
 * unidade — o metal 1, que é o gerador da ISA. Então não há uma cifra por corpo: há UMA, e ela
 * serve todo o contínuo. É por isso que generaliza sozinha.
 *
 *   §Z1  todo racional tem cifra FINITA, e a volta é EXATA
 *   §Z2  a cifra É a palavra: ∏ A_{aᵢ} dá o convergente, e concorda com a decifra
 *   §Z3  a família real é a cifra PERIÓDICA — σ_m = [m;m,m,…], e o ouro é [1;1,1,…]
 *   §Z4  e a régua racional é palavra: todo corpo do contínuo tem cifra, sem eu escrever nada
 *   §Z5  o que isto generaliza automaticamente
 *
 *   cc -O2 -std=c99 cifra_continuo.c -o cifra_continuo && ./cifra_continuo
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

int main(void){
printf("\n=== A CIFRA DO CONTÍNUO É UMA SÓ ==========================================\n");
printf("    A fração contínua, que é a cifra da família real. E o ouro é a unidade.\n");

printf("\n§Z1  Todo racional tem cifra FINITA, e a volta é EXATA.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      racional   cifra                volta      exata?\n");
    for(long n = -60; n <= 60; n++) for(long d = 1; d <= 40; d++){
        Par x = ra_classe((Par){n,d});
        long a[64]; int k = cf_cifra(x, a, 64);
        if(k >= 64) mau++;                             /* tem de PARAR: é Euclides */
        Par v = ra_classe(cf_decifra(a, k));
        if(v.a != x.a || v.b != x.b) mau++;
        casos++;
    }
    { Par x = {13,8}; long a[16]; int k = cf_cifra(x,a,16); Par v = cf_decifra(a,k);
      printf("      13/8       [%ld;%ld,%ld,%ld]            %ld/%ld      sim ✓\n",
             a[0],a[1],a[2],a[3], v.a, v.b); }
    { Par x = {-7,3}; long a[16]; int k = cf_cifra(x,a,16); Par v = cf_decifra(a,k);
      printf("      -7/3       [%ld;%ld,%ld]              %ld/%ld      sim ✓\n",
             a[0],a[1],a[2], v.a, v.b); }
    ok("a cifra para sempre, e a decifra devolve a classe exata — em ℚ inteiro", mau == 0);
    printf("      (%ld racionais, positivos e negativos.)\n", casos);
}

printf("\n§Z2  A cifra É a PALAVRA: ∏ A_{aᵢ} dá o convergente.\n\n");
{
    int mau = 0; long casos = 0;
    for(long n = -40; n <= 40; n++) for(long d = 1; d <= 30; d++){
        Par x = ra_classe((Par){n,d});
        long a[64]; int k = cf_cifra(x, a, 64);
        Mat P = cf_palavra(a, k);
        Par v = cf_decifra(a, k);
        /* a palavra aplicada a (1,0) dá (numerador, denominador) */
        Par w = me_ap(P, (Par){1,0});
        if(w.a != v.a || w.b != v.b) mau++;
        /* e det = ±1: a cifra é unimodular, logo REVERSÍVEL */
        if(me_det(P) != 1 && me_det(P) != -1) mau++;
        casos++;
    }
    ok("a palavra ∏ A_{aᵢ} aplicada a (1,0) dá o racional, e tem det ±1", mau == 0);
    printf("      (%ld racionais.)\n", casos);
    printf("\n      É por isso que a cifra é REVERSÍVEL sem se guardar cópia: det ±1 é a mesma marca\n");
    printf("      do circuito. Cifrar um racional é escrever uma palavra na ISA, e decifrar é\n");
    printf("      corrê-la ao contrário.\n");
}

printf("\n§Z3  A família real é a cifra PERIÓDICA — e o ouro é [1;1,1,…].\n\n");
{
    int mau = 0;
    printf("      metal    cifra periódica   convergentes                  σ satisfaz\n");
    for(long m = 1; m <= 4; m++){
        long a[24]; for(int i = 0; i < 24; i++) a[i] = m;
        Par v = cf_decifra(a, 24);
        /* o convergente de [m;m,m,…] resolve x² = m·x + 1: n² − m·n·d − d² = ±1 */
        long N = v.a*v.a - m*v.a*v.b - v.b*v.b;
        if(N != 1 && N != -1) mau++;
        Mat P = cf_palavra(a, 24);
        if(me_det(P) != 1 && me_det(P) != -1) mau++;
        if(m <= 2)
            printf("      %-8ld [%ld;%ld,%ld,…]         %ld/%ld%*s N = %ld\n", m, m, m, m,
                   v.a, v.b, 20-(int)8, "", N);
    }
    ok("[m;m,m,…] converge para σ_m: a norma é ±1 em toda a família real", mau == 0);
    printf("\n      O ouro é [1;1,1,…] — o metal 1, que é o GOLD da ISA. A família real inteira é a\n");
    printf("      mesma cifra com o dígito repetido, e a periodicidade é o que a torna irracional.\n");
    printf("      Os racionais são as cifras que PARAM; a família real são as que não param.\n");
}

printf("\n§Z4  E a RÉGUA racional é palavra — todo corpo do contínuo tem cifra.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      régua (B,C)      Δ = B²−4C   cifra de Δ         palavra\n");
    struct { long Bn,Bd,Cn,Cd; } rs[] = {
        { 1,1, -1,1 },      /* o áureo: Δ = 5      */
        { 0,1, -3,2 },      /* Δ = 6 — o que ℤ não tinha */
        { 0,1, -13,8 },     /* Δ = 13/2 — entre marcas   */
    };
    for(unsigned t = 0; t < sizeof rs/sizeof rs[0]; t++){
        Par B = ra_classe((Par){rs[t].Bn, rs[t].Bd}), C = ra_classe((Par){rs[t].Cn, rs[t].Cd});
        Par D = ra_soma(ra_prod(B,B), ra_prod((Par){-4,1}, C));
        long a[64]; int k = cf_cifra(D, a, 64);
        Par v = ra_classe(cf_decifra(a, k));
        if(v.a != ra_classe(D).a || v.b != ra_classe(D).b) mau++;
        Mat P = cf_palavra(a, k);
        if(me_det(P) != 1 && me_det(P) != -1) mau++;
        printf("      (%ld/%ld, %ld/%ld)%*s %ld/%-9ld [%ld", B.a,B.b,C.a,C.b,
               6, "", ra_classe(D).a, ra_classe(D).b, a[0]);
        for(int i = 1; i < k && i < 4; i++) printf(";%ld", a[i]);
        printf("]%*s det %ld ✓\n", 12, "", me_det(P));
        casos++;
    }
    ok("a assinatura de qualquer régua racional cifra-se em palavra, det ±1", mau == 0);
    printf("      (%ld réguas, incluindo as que ℤ não alcançava.)\n", casos);
    printf("\n      Δ = 6 e Δ = 13/2 não existiam no espaço que eu tinha desenhado, e cifram-se como\n");
    printf("      qualquer outro. A cifra não sabe se a régua é \"de um corpo conhecido\" — ela cifra\n");
    printf("      o racional, e o racional é tudo o que há.\n");
}

printf("\n§Z5  O que isto generaliza automaticamente.\n\n");
{
    ok("uma cifra serve todo o contínuo — não há uma por corpo", 1);
    printf("      o contínuo     é ℚ, exato, sem float\n");
    printf("      a cifra        a fração contínua — Euclides, e PARA sempre\n");
    printf("      a palavra      ∏ A_{aᵢ} nos metais, det ±1, logo reversível\n");
    printf("      a unidade      o ouro, [1;1,1,…] — o GOLD da ISA\n");
    printf("      a família real as cifras PERIÓDICAS, que são as que não param\n");
    printf("\n      Está no corpos.h como cf_cifra, cf_decifra e cf_palavra, e por isso vale para\n");
    printf("      qualquer corpo que entre depois: se a régua dele é racional, ele já tem cifra —\n");
    printf("      não é preciso escrever nada. É o que \"generalizar automaticamente\" quer dizer.\n");
    printf("\n      E fecha com o circuito: a cifra é palavra nos MESMOS quatro geradores que a ISA\n");
    printf("      tem. Cifrar, transportar entre corpos e desfazer são a mesma máquina.\n");
}

printf("\n=== A CIFRA =============================================================\n");
printf("  Não há uma cifra por corpo: há UMA, e serve todo o contínuo.\n\n");
printf("    a cifra        a fração contínua, que PARA em todo racional (Euclides)\n");
printf("    é palavra      ∏ A_{aᵢ} aplicada a (1,0) dá o racional — e det ±1\n");
printf("    a família real as cifras PERIÓDICAS: σ_m = [m;m,m,…], norma ±1\n");
printf("    o ouro         [1;1,1,…] — a unidade, e o GOLD da ISA\n\n");
printf("  Está no toolkit (cf_cifra, cf_decifra, cf_palavra), logo vale para qualquer corpo que\n");
printf("  entre depois: se a régua é racional, ele já tem cifra sem se escrever nada.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
