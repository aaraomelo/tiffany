/* familia_real.c — A FAMÍLIA REAL: cada metal é um gerador, e cada gerador é uma cifra.
 *
 * O Aarão: "essa é a família real, são geradores, pode usar. São os irracionais fora do jogo
 * cifrando, são da família do rei."
 *
 * buracos.c mediu que todo σ_m é irracional e é buraco para a PA e para a PG. Aqui mede-se o
 * que essa família SERVE, e o que ela não serve — porque as duas coisas importam ao banco.
 *
 * O QUE ELA DÁ. O gato A_m = [[m,1],[1,0]] tem det = −1 para todo m, e det ±1 é a condição de a
 * inversa ser INTEIRA. Logo cifrar e decifrar volta exato, sem arredondamento nenhum, em
 * qualquer metal e qualquer número de voltas. É a cifra que a ISA já tinha: cifra_an(w,m) =
 * (m·total + e, total) é exatamente A_m aplicado a (total, e) — broca-so ula/cifra.c.
 *
 * E a chave é o par (m, k): que metal, e quantas voltas. Compor (m,k) com (m,−k) devolve a
 * identidade — AS CHAVES SOMAM ZERO, e somam zero no expoente, exatamente.
 *
 * O QUE ELA NÃO DÁ, e isto tem de ser dito com a mesma clareza: A_m é LINEAR. Isso faz dela uma
 * codificação reversível e ADITIVAMENTE HOMOMÓRFICA — o banco pode somar dados cifrados sem os
 * abrir, que é a propriedade não-custodial que interessa — mas NÃO faz dela sigilo
 * criptográfico. Uma transformação linear cede a quem tiver alguns pares claro/cifrado. O
 * sigilo tem de vir de outro lugar; daqui vem a reversibilidade exata e o homomorfismo.
 *
 *   §F1  det A_m = −1 para todo m, e a inversa é INTEIRA — a família toda é reversível
 *   §F2  cifrar e decifrar volta EXATO, em qualquer metal e qualquer volta
 *   §F3  as chaves somam zero: A_m^k · A_m^(−k) = I, medido
 *   §F4  bandas distintas: decifrar com o metal errado não devolve o dado
 *   §F5  e o homomorfismo: cifrar(x) + cifrar(y) = cifrar(x+y) — some sem abrir
 *
 *   cc -O2 -std=c99 familia_real.c -o familia_real && ./familia_real
 */
#include <stdio.h>
#include "unidade.h"

typedef struct { long a, b; } Par;      /* (total, e) — a Word da ISA */

/* cifrar uma volta com o metal m: exatamente cifra_an de broca-so ula/cifra.c */
static Par cifra(Par w, long m){ Par r = { m*w.a + w.b, w.a }; return r; }
/* decifrar uma volta: a inversa de A_m, que é INTEIRA porque det = −1 */
static Par decifra(Par w, long m){ Par r = { w.b, w.a - m*w.b }; return r; }

static Par cifra_k(Par w, long m, int k){ for(int t = 0; t < k; t++) w = cifra(w, m); return w; }
static Par decifra_k(Par w, long m, int k){ for(int t = 0; t < k; t++) w = decifra(w, m); return w; }

int main(void){
printf("\n=== A FAMÍLIA REAL: geradores, e cada um uma cifra =========================\n");
printf("    Todo σ_m é irracional e é buraco para a PA e a PG (buracos.c). Aqui, o que serve.\n");

/* ---------------------------------------------------------------- §F1 ------ */
printf("\n§F1  det A_m = −1 para todo m — e é isso que faz a inversa ser INTEIRA.\n\n");
{
    int mau = 0;
    printf("      m     det A_m   a inversa é inteira?   A_m^(−1)\n");
    for(long m = 1; m <= 30; m++){
        long det = m*0 - 1*1;
        if(det != -1) mau++;
        if(m <= 4 || m == 30)
            printf("      %-5ld %-9ld %-22s [[0,1],[1,−%ld]]\n", m, det, "sim ✓", m);
    }
    ok("det = −1 em toda a família: nenhum metal perde a volta", mau == 0);
    printf("\n      Determinante ±1 é a condição exata de a matriz inversa ter entradas inteiras.\n");
    printf("      Não é aproximação boa: é reversibilidade sem resto, e vale para TODO metal.\n");
}

/* ---------------------------------------------------------------- §F2 ------ */
printf("\n§F2  Cifrar e decifrar volta EXATO — qualquer metal, qualquer número de voltas.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m     voltas   dado          cifrado                    volta ao dado?\n");
    for(long m = 1; m <= 12; m++)
    for(int k = 1; k <= 20; k++)
    for(long v = -50; v <= 50; v += 7){
        Par w = { v, 0 };
        Par c = cifra_k(w, m, k);
        Par d = decifra_k(c, m, k);
        if(d.a != w.a || d.b != w.b) mau++;
        casos++;
        if((m==1&&k==5&&v==7) || (m==2&&k==10&&v==-1) || (m==12&&k==20&&v==43))
            printf("      %-5ld %-8d (%ld,%ld)%*s(%ld,%ld)%*s%s\n", m, k, w.a, w.b,
                   (int)(9-4), "", c.a, c.b, (int)(20-8), "",
                   (d.a==w.a&&d.b==w.b) ? "sim ✓" : "NÃO");
    }
    ok("a volta é exata em todos os casos, sem um arredondamento", mau == 0);
    printf("      (%ld casos: 12 metais × 20 voltas × 15 dados.)\n", casos);
}

/* ---------------------------------------------------------------- §F3 ------ */
printf("\n§F3  AS CHAVES SOMAM ZERO: compor (m,k) com (m,−k) devolve a identidade.\n\n");
{
    int mau = 0;
    printf("      m     k    −k    k + (−k)   compor devolve o dado?\n");
    for(long m = 1; m <= 8; m++) for(int k = 1; k <= 15; k++){
        Par w = { 12345, -678 };
        Par v = decifra_k(cifra_k(w, m, k), m, k);
        if(v.a != w.a || v.b != w.b) mau++;
        if((m<=2&&k<=2)||(m==8&&k==15))
            printf("      %-5ld %-4d %-5d %-10d %s\n", m, k, -k, 0,
                   (v.a==w.a&&v.b==w.b) ? "sim ✓" : "NÃO");
    }
    ok("a chave e a sua inversa somam zero no expoente, e a composição é a identidade", mau == 0);
    printf("\n      A chave é o par (metal, voltas). O que soma zero é o EXPOENTE — e como o\n");
    printf("      determinante é −1, a volta atrás existe em inteiros e não é reconstrução: é a\n");
    printf("      mesma peça andando para trás.\n");
}

/* ---------------------------------------------------------------- §F4 ------ */
printf("\n§F4  BANDAS DISTINTAS: decifrar com o metal errado não devolve o dado.\n\n");
{
    int mau = 0; long errados = 0, acertos = 0;
    printf("      cifrado com   decifrado com   volta ao dado?   afastamento\n");
    for(long m1 = 1; m1 <= 8; m1++) for(long m2 = 1; m2 <= 8; m2++){
        Par w = { 1000, 0 };
        Par c = cifra_k(w, m1, 6);
        Par d = decifra_k(c, m2, 6);
        int voltou = (d.a == w.a && d.b == w.b);
        if(m1 == m2 && !voltou) mau++;
        if(m1 != m2 && voltou) mau++;
        if(voltou) acertos++; else errados++;
        if((m1==1&&m2<=3)||(m1==3&&m2==3))
            printf("      m=%-11ld m=%-13ld %-16s %ld\n", m1, m2,
                   voltou ? "sim ✓" : "NÃO — banda errada", d.a - w.a);
    }
    ok("só o metal certo devolve o dado; qualquer outro, não", mau == 0);
    printf("      (%ld pares acertam — os da diagonal —, %ld não.)\n", acertos, errados);
    printf("\n      Cada cliente na sua banda: o metal é parte da chave, e a banda errada não\n");
    printf("      devolve nada aproveitável. É o que separa um cliente do outro.\n");
}

/* ---------------------------------------------------------------- §F5 ------ */
printf("\n§F5  O HOMOMORFISMO: somar cifrado é cifrar a soma. E o que isto NÃO é.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      m   k    x     y     cifra(x)+cifra(y)   cifra(x+y)   iguais?\n");
    for(long m = 1; m <= 8; m++) for(int k = 1; k <= 10; k++)
    for(long x = -20; x <= 20; x += 9) for(long y = -20; y <= 20; y += 11){
        Par cx = cifra_k((Par){x,0}, m, k);
        Par cy = cifra_k((Par){y,0}, m, k);
        Par soma = { cx.a + cy.a, cx.b + cy.b };
        Par cxy = cifra_k((Par){x+y,0}, m, k);
        if(soma.a != cxy.a || soma.b != cxy.b) mau++;
        casos++;
        if(m==1 && k==4 && x==-2 && y==-20)
            printf("      %-3ld %-4d %-5ld %-5ld (%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n",
                   m, k, x, y, soma.a, soma.b, 8, "", cxy.a, cxy.b, 3, "");
    }
    ok("somar dois cifrados dá o cifrado da soma — exato, em todos os casos", mau == 0);
    printf("      (%ld casos.)\n", casos);
    printf("\n      É a propriedade NÃO-CUSTODIAL que interessa: o banco soma uma coluna inteira\n");
    printf("      sem abrir um único valor, e o cliente decifra o total no fim. O banco calcula\n");
    printf("      sobre o que não pode ler.\n");
    printf("\n      E o que isto NÃO é, dito com a mesma clareza: A_m é LINEAR, e linear cede a\n");
    printf("      quem tiver alguns pares claro/cifrado — dois bastam para montar o sistema e\n");
    printf("      achar m. Isto é CODIFICAÇÃO REVERSÍVEL E HOMOMÓRFICA, não sigilo\n");
    printf("      criptográfico. O sigilo tem de vir de outro lugar; daqui vem a reversibilidade\n");
    printf("      exata e a soma sem abrir. Confundir as duas coisas seria vender o que não há.\n");
}

printf("\n=== A FAMÍLIA REAL ========================================================\n");
printf("  Todo σ_m é irracional e é buraco para a PA e a PG. E todo A_m tem det = −1, que é a\n");
printf("  condição exata de a inversa ser inteira — logo a família inteira é REVERSÍVEL SEM\n");
printf("  RESTO, e a cifra já estava na ISA: cifra_an(w,m) É o gato A_m.\n\n");
printf("    a chave        o par (metal, voltas)\n");
printf("    somam zero     compor (m,k) com (m,−k) devolve a identidade, exatamente\n");
printf("    a banda        só o metal certo devolve o dado; qualquer outro, não\n");
printf("    o ganho        somar cifrado É cifrar a soma — o banco soma sem abrir\n\n");
printf("  E o limite, dito com a mesma clareza do resto: A_m é linear, então isto é codificação\n");
printf("  reversível e homomórfica, NÃO sigilo criptográfico. Dois pares claro/cifrado entregam\n");
printf("  o metal. O sigilo vem de outro lugar — daqui vem a volta exata e a soma sem abrir.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros, sem um único float.\n\n");
return 0;
}
