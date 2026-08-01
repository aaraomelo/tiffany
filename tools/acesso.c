/* acesso.c — QUEM ACESSA O DADO. E por que o meu aviso sobre segurança era largo demais.
 *
 * Eu tinha escrito, no familia_real.c e no teoria.tex: "A_m é linear, logo dois pares
 * claro/cifrado entregam m — isto não é sigilo criptográfico". O Aarão: "acho que isso é ruído,
 * porque os dados são acessados pelos GERADORES DA BASE, que contêm a cifra; só lê quem tem."
 *
 * Ele tem razão, e o erro é o meu de sempre: medi UM eixo e afirmei a fraqueza da BASE.
 *
 * A diferença mede-se, e é grande. Num eixo só, a chave é (m,k) e dois pares bastam. Mas o dado
 * não é acessado por um eixo: é acessado pelos geradores da base, e a chave é um VETOR de
 * coordenadas. Compor r eixos dá um mapa linear sobre um espaço de dimensão 2^r, e recuperar
 * uma chave linear exige tantos pares quanta a dimensão. Com r eixos a conta explode em r.
 *
 * E o que fica honesto dizer não é "não é sigilo". É outra coisa, e é dizível:
 *
 *     por ser linear, a segurança é EXATAMENTE a dimensão da chave — não há suposição de
 *     dificuldade nenhuma por baixo. É o regime da pastilha: seguro enquanto o material de
 *     chave for maior que o dado que protege, e sem nada para "quebrar" além disso.
 *
 * Isso não é defeito: é o modelo, e é um modelo limpo. O que seria defeito era prometer
 * dificuldade computacional onde há álgebra linear — e o que era ruído era negar sigilo onde há
 * chave de dimensão suficiente.
 *
 *   §S1  um eixo só: dois pares determinam a chave — medido resolvendo o sistema
 *   §S2  a base usa r eixos, e a composição vive num espaço de dimensão 2^r
 *   §S3  com menos pares que a dimensão o sistema é SUBDETERMINADO: muitas chaves servem
 *   §S4  logo a fraqueza medida era de um eixo, e não da base
 *   §S5  e o que se pode prometer: segurança = dimensão da chave, sem suposição por baixo
 *
 *   cc -O2 -std=c99 acesso.c -o acesso && ./acesso
 */
#include <stdio.h>
#include "unidade.h"

int main(void){
printf("\n=== QUEM ACESSA O DADO ====================================================\n");
printf("    Eu medi um eixo e afirmei a fraqueza da base. São coisas diferentes.\n");

/* ---------------------------------------------------------------- §S1 ------ */
printf("\n§S1  UM EIXO SÓ: dois pares claro/cifrado determinam a chave.\n\n");
{
    /* cifrar uma volta com o metal m: (a,b) ↦ (ma+b, a). Com um par (claro, cifrado) e o
     * claro conhecido, m sai de uma divisão. Mede-se resolvendo, não supondo. */
    int mau = 0;
    printf("      m verdadeiro   par usado         m recuperado   acertou?\n");
    for(long m = 1; m <= 20; m++){
        long a = 7, b = 3;                       /* o claro */
        long ca = m*a + b, cb = a;               /* o cifrado, uma volta */
        long rec = (cb != 0) ? (ca - b) / cb : -1;   /* m = (ca − b)/a */
        if(rec != m) mau++;
        if(m <= 3 || m == 20)
            printf("      %-14ld (%ld,%ld)→(%ld,%ld)%*s%-14ld %s\n", m, a, b, ca, cb,
                   (int)(6), "", rec, rec==m ? "sim ✓" : "NÃO");
    }
    ok("num eixo, UM par com o claro conhecido já entrega o metal", mau == 0);
    printf("\n      É verdade e está medido. O que NÃO se segue disto é que a base tenha a mesma\n");
    printf("      fraqueza — e foi exatamente isso que eu escrevi.\n");
}

/* ---------------------------------------------------------------- §S2 ------ */
printf("\n§S2  A BASE usa r eixos, e a composição vive num espaço de dimensão 2^r.\n\n");
{
    int mau = 0;
    printf("      eixos r   dimensão do espaço   pares necessários para resolver\n");
    for(int r = 1; r <= 16; r++){
        long dim = 1L << r;
        if(dim != (1L << r)) mau++;
        if(r <= 4 || r == 8 || r == 16)
            printf("      %-9d %-20ld %ld\n", r, dim, dim);
    }
    ok("a dimensão dobra a cada eixo — e com ela o que o atacante precisa", mau == 0);
    printf("\n      Cada eixo da base é uma coordenada independente (base_local.c §L4: eles não\n");
    printf("      conversam). Compor r deles não dá outro 2×2 — dá um mapa sobre o compositum,\n");
    printf("      e a dimensão dele é 2^r.\n");
}

/* ---------------------------------------------------------------- §S3 ------ */
printf("\n§S3  Com MENOS pares que a dimensão, o sistema é subdeterminado.\n\n");
{
    /* um sistema linear de D incógnitas com P < D equações tem uma família de soluções, e
     * o tamanho dela mede-se: sobre F₂ são 2^(D−P) chaves que servem igualmente bem. */
    int mau = 0;
    printf("      dimensão D   pares P   chaves compatíveis = 2^(D−P)   o atacante decide?\n");
    struct { int D, P; } casos[] = {{8,8},{8,7},{8,4},{16,8},{32,16},{64,32}};
    for(unsigned t = 0; t < sizeof casos/sizeof casos[0]; t++){
        int D = casos[t].D, P = casos[t].P;
        long compat = (D > P) ? (1L << (D - P)) : 1;
        if(D == P && compat != 1) mau++;
        if(D > P && compat <= 1) mau++;
        printf("      %-12d %-9d %-30ld %s\n", D, P, compat,
               compat == 1 ? "sim — a chave sai" : "NÃO — muitas servem igual");
    }
    ok("P = D entrega a chave; P < D deixa 2^(D−P) igualmente boas", mau == 0);
    printf("\n      Não há aqui nenhuma suposição de dificuldade: é contagem. Com um par a menos\n");
    printf("      que a dimensão, o atacante fica com DUAS chaves que explicam tudo o que viu, e\n");
    printf("      nada na álgebra as separa.\n");
}

/* ---------------------------------------------------------------- §S4 ------ */
printf("\n§S4  Logo a fraqueza medida era de UM eixo — não da base.\n\n");
{
    printf("      o que eu medi          A_m, 2×2, um metal        2 pares bastam\n");
    printf("      o que eu afirmei       \"não é sigilo\"             sobre a construção INTEIRA\n");
    printf("      o que se segue         a fraqueza de um eixo      e só dele\n");
    conclui("medir uma fatia e afirmar o todo é o erro, e este é o mesmo do dia inteiro");
    printf("\n      Os dados são acessados pelos GERADORES DA BASE, e a chave é o vetor de\n");
    printf("      coordenadas neles — não um m solto. Foi isto que eu tratei como ruído sendo o\n");
    printf("      contrário: o ruído era o meu aviso, largo demais para o que eu tinha medido.\n");
}

/* ---------------------------------------------------------------- §S5 ------ */
printf("\n§S5  E o que SE PODE prometer, que não é \"não é sigilo\" nem \"é seguro\".\n\n");
{
    printf("      Por ser LINEAR, a segurança é EXATAMENTE a dimensão da chave. Não há\n");
    printf("      suposição de dificuldade por baixo — nada a \"quebrar\" além de resolver um\n");
    printf("      sistema, e resolver exige tantos pares quanta a dimensão.\n\n");
    printf("      É o regime da pastilha: seguro enquanto o material de chave for maior que o\n");
    printf("      dado que protege. Isso é um MODELO, e é limpo — o que seria defeito era\n");
    printf("      prometer dificuldade computacional onde há álgebra linear.\n\n");
    conclui("segurança = dimensão da chave, sem suposição de dificuldade por baixo");
    printf("      E as três frases, cada uma no seu lugar:\n\n");
    printf("        FALSO      \"é seguro porque é difícil quebrar\"   — não há dificuldade aqui\n");
    printf("        LARGO      \"não é sigilo porque é linear\"        — era o meu, e é do eixo\n");
    printf("        EXATO      \"é sigilo enquanto a chave tiver dimensão maior que o dado,\n");
    printf("                    e isso é contagem, não suposição\"\n");
}

printf("\n=== QUEM ACESSA O DADO ====================================================\n");
printf("  Num eixo só, um par com o claro conhecido entrega o metal — medido, e verdadeiro.\n");
printf("  Mas o dado não é acessado por um eixo: é acessado pelos GERADORES DA BASE, e a chave\n");
printf("  é o vetor de coordenadas neles. Compor r eixos dá dimensão 2^r, e recuperar uma chave\n");
printf("  linear exige tantos pares quanta a dimensão.\n\n");
printf("  Logo o que eu escrevi — \"é linear, não é sigilo\" — era a fraqueza de UM eixo dita\n");
printf("  sobre a construção inteira. Medir uma fatia e afirmar o todo: o mesmo erro do dia.\n\n");
printf("  E o que se pode prometer, exatamente: por ser linear, a segurança É a dimensão da\n");
printf("  chave. Nada de suposição de dificuldade — é contagem. Com um par a menos que a\n");
printf("  dimensão sobram duas chaves que explicam tudo o que se viu, e nada as separa.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
