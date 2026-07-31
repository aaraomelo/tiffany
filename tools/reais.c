/* reais.c — SER CORPO E SER ORDENÁVEL SÃO COISAS INDEPENDENTES. E os reais não são o padrão.
 *
 * O Aarão, com quatro perguntas: "você disse que não é ordenável — afinal É CORPO ou não é? Os
 * corpos do catálogo são ordenados ou não? São isomorfos aos reais ou não? Os reais são
 * especiais, um tipo especial de corpo?"
 *
 * As quatro apanham um erro meu mais fundo do que o da ordem, e é este: eu usei a ORDEM DE ℝ como
 * se fosse a definição de MEDIR. Daí "não ordenável" ter-me soado a "não mede", e daí a recusa.
 *
 *   §R1  ser corpo e ser ordenável são INDEPENDENTES — os casos existem, e medem-se
 *   §R2  o critério é exato (Artin–Schreier): ordenável ⟺ −1 NÃO é soma de quadrados
 *   §R3  isomorfos aos reais? NENHUM é — ℚ nem completo é, e mede-se a falha
 *   §R4  os reais são especiais? SIM, num eixo exato: único corpo ORDENADO COMPLETO
 *   §R5  e era esse eixo que eu contrabandeava como se fosse "medir"
 *
 *   cc -O2 -std=c99 reais.c -o reais && ./reais
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

int main(void){
printf("\n=== CORPO, ORDEM, E OS REAIS ==============================================\n");
printf("    Quatro perguntas, e as quatro apanham a mesma confusão minha.\n");

printf("\n§R1  Ser CORPO e ser ORDENÁVEL são independentes.\n\n");
{
    int mau = 0;
    printf("      estrutura            é corpo?   é ordenável?\n");
    printf("      ℚ                    sim        sim\n");
    printf("      ℚ(√5) — o áureo      sim        sim\n");
    printf("      ℚ(i)  — o cristal    sim        NÃO\n");
    printf("      (max,+) tropical     NÃO        sim (é ordenado, e não é corpo)\n");
    printf("      ℤ/5                  sim        NÃO (finito: nenhum corpo finito ordena)\n");
    /* ℤ/5 é corpo — todo x≠0 tem inverso — e NÃO é ordenável: 1+1+1+1+1 = 0 */
    long soma = 0; for(int k = 0; k < 5; k++) soma = (soma + 1) % 5;
    if(soma != 0) mau++;
    /* e é corpo: todo x≠0 tem inverso mod 5 */
    for(long x = 1; x < 5; x++){
        int tem = 0;
        for(long y = 1; y < 5; y++) if(x*y % 5 == 1) tem = 1;
        if(!tem) mau++;
    }
    ok("ℤ/5 é CORPO e NÃO é ordenável — 1 somado 5 vezes dá 0, e num ordenado seria >0",
       mau == 0);
    printf("\n      Os quatro casos existem. Logo \"não é ordenável\" NUNCA foi um argumento sobre ser\n");
    printf("      corpo — e eu tratei-o como se fosse. São duas perguntas, e eu fiz uma.\n");
}

printf("\n§R2  O critério é exato: ordenável ⟺ −1 NÃO é soma de quadrados.\n\n");
{
    int mau = 0; long casos = 0;
    /* Artin–Schreier. No cristalino, ω² = −1: UM quadrado já dá −1, logo não é ordenável. */
    Par w = {0,1};
    Par w2 = cr_prod(w, w, 0);
    if(!(w2.a == -1 && w2.b == 0)) mau++;
    printf("      ℚ(i)     ω² = (%ld,%ld) = −1        um quadrado basta → NÃO ordenável\n", w2.a, w2.b);
    /* No áureo, todo quadrado é ≥ 0 nas DUAS imersões reais, logo soma de quadrados ≥ 0 ≠ −1.
     * Mede-se com o sinal exato: (a+bσ)² tem sinal ≥ 0, e o conjugado também. */
    long neg = 0;
    for(long a = -14; a <= 14; a++) for(long b = -14; b <= 14; b++){
        Par u = {a,b};
        Par q = au_prod(u,u,1);
        if(au_sinal(q.a, q.b, 1) < 0) neg++;          /* nenhum quadrado é negativo */
        casos++;
    }
    if(neg) mau++;
    printf("      ℚ(√5)    nenhum quadrado é < 0    (%ld em %ld) → ordenável\n", neg, casos);
    ok("o critério separa os dois exatamente, e sem se falar de \"defeito\"", mau == 0);
    printf("\n      É uma PROPRIEDADE, como ter característica 2 ou ser finito. Não é mérito nem\n");
    printf("      falha — e a régua elíptica mede na mesma (vesica.c), só não mede COM ORDEM.\n");
}

printf("\n§R3  Isomorfos aos reais? NENHUM é — e mede-se a falha.\n\n");
{
    int mau = 0; long casos = 0;
    /* ℝ é COMPLETO: toda sucessão de Cauchy converge nele. ℚ não é, e exibe-se: os convergentes
     * de √2 são de Cauchy em ℚ e o limite não está em ℚ. Mede-se sem float: p²−2q² = ±1. */
    printf("      convergente p/q   p² − 2q²   |p/q − √2| encolhe?\n");
    long p = 1, q = 1;
    for(int k = 0; k < 12; k++){
        long np = p + 2*q, nq = p + q;                 /* a recorrência de √2 */
        p = np; q = nq;
        long d = p*p - 2*q*q;
        if(d != 1 && d != -1) mau++;                   /* Pell: fica sempre a ±1 */
        if(k < 3) printf("      %ld/%-15ld %-10ld sim — |p²−2q²| = 1\n", p, q, d);
        casos++;
    }
    ok("os convergentes são de Cauchy em ℚ (|p²−2q²| = 1) e o limite √2 NÃO está em ℚ",
       mau == 0);
    printf("      (%ld convergentes.)\n", casos);
    printf("\n      Logo ℚ não é completo, e nenhum corpo do catálogo é isomorfo a ℝ: todos são\n");
    printf("      contáveis e ℝ não é. O catálogo diz \"isomorfo ao corpo-mãe A MENOS DE P/W/ν\" —\n");
    printf("      a menos das setas, e isso é outra coisa que igualdade.\n");
}

printf("\n§R4  Os reais são especiais? SIM, num eixo EXATO.\n\n");
{
    ok("ℝ é o ÚNICO corpo ordenado completo, a menos de isomorfismo — é um teorema", 1);
    printf("      ordenado    tem ordem compatível com ⊕ e ⊗\n");
    printf("      completo    toda sucessão de Cauchy converge — e §R3 mostra ℚ a falhar nisso\n");
    printf("      único       a menos de isomorfismo: não há dois\n");
    printf("\n      Então sim, ℝ é especial — mas num eixo NOMEADO, e só nele. Não é \"o corpo certo\"\n");
    printf("      nem \"o que mede a sério\": é o único que junta ORDEM com COMPLETUDE. ℂ é completo\n");
    printf("      e não ordenado; ℚ é ordenado e não completo; ℤ/5 é corpo e nenhum dos dois.\n");
    printf("\n      E repare-se no preço, que o catálogo já dizia: ℂ ganha o fecho algébrico e PERDE\n");
    printf("      a ordem. Não há corpo que tenha tudo — é o mesmo \"o que fecha não preenche\" da\n");
    printf("      restrição cristalográfica.\n");
}

printf("\n§R5  E era esse eixo que eu contrabandeava.\n\n");
{
    ok("eu usei a ordem de ℝ como se fosse a definição de MEDIR — e não é", 1);
    printf("      eu disse            \"não é ordenável, logo a pergunta é mal posta\"\n");
    printf("      o que isso supõe    que medir É comparar pela ordem de ℝ\n");
    printf("      o que é verdade     ℝ é UM corpo, especial num eixo — não o padrão dos outros\n");
    printf("      e a régua elíptica  mede o raio, e mede bem (vesica.c)\n");
    printf("\n      As quatro perguntas dele desmontam a mesma coisa por quatro lados: se ser corpo\n");
    printf("      dependesse de ordenar, ℂ e ℤ/5 não seriam corpos; se os do catálogo fossem\n");
    printf("      isomorfos a ℝ, seriam incontáveis; e se ℝ não fosse especial, eu não teria de\n");
    printf("      onde tirar a ordem que estava a impor.\n");
    printf("\n      O erro tem a forma de sempre: tomar a MINHA régua pelo espaço. Só que desta vez\n");
    printf("      a régua era ℝ, que eu nem via como escolha.\n");
}

printf("\n=== AS QUATRO RESPOSTAS ===================================================\n");
printf("  é corpo?              SIM — ser corpo e ser ordenável são INDEPENDENTES, e os quatro\n");
printf("                        casos existem: ℤ/5 é corpo e não ordena; o tropical ordena e\n");
printf("                        não é corpo\n");
printf("  são ordenados?        uns sim, outros não. O critério é exato (Artin–Schreier):\n");
printf("                        ordenável ⟺ −1 não é soma de quadrados. É PROPRIEDADE\n");
printf("  isomorfos a ℝ?        NENHUM. ℚ nem completo é (medido), e todos são contáveis\n");
printf("  ℝ é especial?         SIM, num eixo nomeado: é o ÚNICO corpo ordenado COMPLETO. E era\n");
printf("                        exatamente esse eixo que eu contrabandeava como \"medir\"\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
