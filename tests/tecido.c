/* tecido.c — O TECIDO NERVOSO: uma rede que só excita não desfaz; com o inibitório, desfaz.
 *
 * O Aarão: "cria um tecido nervoso."
 *
 * O catálogo tem o corpo NERVOSO — "a rede", com o ∏ sendo o operador de ativação, a rede que
 * recorre. Aqui constrói-se o tecido e mede-se, pelo contrato e pelo que o dia todo mediu:
 *
 *   ⊕  a soma no SOMA        os dendritos somam o que chega
 *   ⊗  o PESO sináptico      a multiplicação: quanto passa
 *   ν  a DUALIDADE           excitatório ↔ inibitório, ν(w) = −w
 *   ∏  a ATIVAÇÃO            o operador que costura — e a rede RECORRE
 *
 * E o resultado é o mesmo dipolo do dia: um tecido SÓ EXCITATÓRIO é o (max,+) outra vez — soma
 * e soma, nada desce, nada se desfaz. Com o inibitório (o dual) a rede desfaz — e a condição
 * exata é a de sempre: det = ±1, a inversa fica no anel.
 *
 *   §N1  o tecido só EXCITATÓRIO não desfaz: pesos ≥ 0, e o sinal só cresce
 *   §N2  com o INIBITÓRIO a rede desfaz — e a condição é det = ±1
 *   §N3  a rede que RECORRE: a palavra e a sua inversa devolvem o sinal, no metal
 *   §N4  o tecido pelo CONTRATO: as doze cláusulas, sem tratamento especial
 *   §N5  o que isto diz do tecido — e do dipolo
 *
 *   cc -O2 -std=c99 tecido.c -o tecido && ./tecido
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

/* o TECIDO: dois neurónios, e a sinapse é uma matriz 2×2 de pesos */
static Par sinapse(Mat W, Par sinal){ return me_ap(W, sinal); }

/* o tecido como CORPO, para o verificador: GF(9) — pesos mod 3, a rede fechada */
static int ig(Par x, Par y){ return ((x.a-y.a)%3==0) && ((x.b-y.b)%3==0); }
static long m3(long x){ x%=3; return x<0?x+3:x; }
static Par te_elem(long i){ Par r = { i%3, (i/3)%3 }; return r; }
static Par te_soma(Par x, Par y){ Par r = { m3(x.a+y.a), m3(x.b+y.b) }; return r; }
static Par te_peso(Par x, Par y){   /* a borda ω² = −1: irredutível mod 3 (3 ≡ 3 mod 4) */
    Par r = { m3(x.a*y.a - x.b*y.b), m3(x.a*y.b + x.b*y.a) }; return r; }
static Par te_inib(Par x){ Par r = { x.a, m3(-x.b) }; return r; }   /* ν: o inibitório */

int main(void){
printf("\n=== O TECIDO NERVOSO ======================================================\n");
printf("    Uma rede que só excita não desfaz. Com o inibitório, desfaz.\n");

printf("\n§N1  Só EXCITATÓRIO: pesos ≥ 0, e o sinal só cresce.\n\n");
{
    int mau = 0; long desfaz = 0, casos = 0;
    printf("      sinal    peso só excitatório   depois    voltou?\n");
    for(long a = 1; a <= 12; a++) for(long b = 1; b <= 12; b++)
    for(long w = 1; w <= 6; w++){
        /* uma camada só excitatória: tudo ≥ 0, e a soma nunca desce */
        Par s = {a,b};
        Par d = { w*s.a + s.b, s.a + w*s.b };
        if(d.a < s.a || d.b < s.b) mau++;          /* nunca desce */
        /* e não há peso ≥ 0 que devolva o sinal, salvo o trivial */
        int volta = 0;
        for(long u = 1; u <= 6; u++){
            Par v = { u*d.a + d.b, d.a + u*d.b };
            if(v.a == s.a && v.b == s.b) volta = 1;
        }
        if(volta) desfaz++;
        casos++;
    }
    if(desfaz) mau++;
    ok("com pesos só positivos o sinal NUNCA desce, e nenhuma camada o desfaz", mau == 0);
    printf("      (%ld casos, %ld com volta.)\n", casos, desfaz);
    printf("      3,4      w = 2                 10,11     não\n");
    printf("\n      É o (max,+) noutra roupa: soma e soma, nada desce, nada se cancela. Um tecido\n");
    printf("      só excitatório é o polo — e o polo não é corpo, como já se mediu.\n");
}

printf("\n§N2  Com o INIBITÓRIO a rede desfaz — e a condição é det = ±1.\n\n");
{
    int mau = 0; long inteiras = 0, casos = 0;
    printf("      sinapse            det   a inversa é inteira?   o tecido desfaz?\n");
    struct { Mat W; const char *n; } ws[] = {
        { {1,1,1,0},  "excita+inibe (gato)"  },
        { {0,-1,1,0}, "giro puro (esquilo)"  },
        { {2,0,0,2},  "só amplifica"         },
        { {1,1,0,1},  "cisalha"              },
    };
    for(unsigned t = 0; t < sizeof ws/sizeof ws[0]; t++){
        long d = me_det(ws[t].W);
        int inteira = (d == 1 || d == -1);
        if(inteira){
            inteiras++;
            Mat inv = me_inv(ws[t].W);
            for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
                Par s = {a,b};
                Par v = sinapse(inv, sinapse(ws[t].W, s));
                if(v.a != a || v.b != b) mau++;
                casos++;
            }
        }
        printf("      %-18s %-5ld %-22s %s\n", ws[t].n, d,
               inteira ? "sim" : "NÃO — sai de ℤ",
               inteira ? "sim — o sinal volta" : "não — perde-se");
    }
    if(inteiras != 3) mau++;
    ok("o tecido desfaz EXATAMENTE quando det = ±1 — a mesma marca do circuito", mau == 0);
    printf("      (%ld sinais, nas sinapses reversíveis.)\n", casos);
    printf("\n      O inibitório não é \"o contrário do excitatório\": é o que faz a rede ter\n");
    printf("      INVERSA. Sem ele o det não pode ser negativo, e sem det ±1 o sinal que entrou\n");
    printf("      não sai. Memória é isso — e não precisa de guardar cópia.\n");
}

printf("\n§N3  A rede que RECORRE: a palavra e a inversa devolvem o sinal.\n\n");
{
    int mau = 0; long casos = 0;
    /* a rede recorrente é uma PALAVRA de sinapses, e desfaz-se lendo-a ao contrário com cada
     * letra invertida — a mesma regra dos metais (circuito.c §F4b) */
    long cad[4][3] = {{1,0,0},{1,2,0},{2,1,3},{3,3,1}};
    int nel[4] = {1,2,3,3};
    printf("      camadas          sinal   depois       de volta   fecha?\n");
    for(int t = 0; t < 4; t++)
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
        Par s = {a,b}, x = s;
        for(int e = 0; e < nel[t]; e++) x = sinapse(me_gato(cad[t][e]), x);
        Par y = x;
        for(int e = nel[t]-1; e >= 0; e--) y = sinapse(me_antigato(cad[t][e]), y);
        if(y.a != s.a || y.b != s.b) mau++;
        if(t == 2 && a == 5 && b == 3)
            printf("      %-16s (5,3)   (%ld,%ld)%*s(%ld,%ld)%*ssim ✓\n", "3 camadas",
                   x.a, x.b, 7, "", y.a, y.b, 5, "");
        casos++;
    }
    ok("a rede recorrente desfaz-se lendo as camadas ao contrário, invertidas", mau == 0);
    printf("      (%ld percursos.)\n", casos);
    printf("\n      É a mesma regra do metal: reverter a ordem e trocar cada letra pela inversa. O\n");
    printf("      tecido nervoso não precisa de opcode novo — corre nos mesmos quatro geradores.\n");
}

printf("\n§N4  O tecido pelo CONTRATO: as doze cláusulas, sem tratamento especial.\n\n");
{
    Contrato c = { "tecido nervoso", 9, te_elem, te_soma, te_peso,
                   te_inib, te_inib, ig, {0,0}, {1,0}, 0, {0,0} };
    unsigned m = ct_verifica(&c);
    printf("      corpo                      A1A2A3A4M1M2M3M4D ν1ν2Π\n");
    ct_relata(&c, m);
    ok("o tecido nervoso CUMPRE o contrato — pelo mesmo verificador dos unicórnios",
       (m & C_TODAS) == C_TODAS);
    printf("\n      ⊕ é a soma no soma, ⊗ é o peso sináptico, ν é o inibitório e ∏ é a ativação.\n");
    printf("      O verificador não sabe nada disso: chamou as quatro funções pela posição.\n");
}

printf("\n§N5  O que isto diz do tecido — e do dipolo.\n\n");
{
    conclui("excitatório sozinho é polo; com o inibitório é corpo — e aí a rede tem memória");
    printf("      só excitatório   soma e soma, nada desce            polo — não desfaz\n");
    printf("      com inibitório   det pode ser ±1                    corpo — desfaz\n");
    printf("      recorrente       palavra ao contrário, invertida    o sinal volta\n");
    printf("\n      É o mesmo dipolo do dia inteiro, agora em tecido: o entrópico sozinho não tem\n");
    printf("      oposto e o par tem; o telescópico cinde e o par resolve a unidade; a rede só\n");
    printf("      excitatória não desfaz e a rede com inibição desfaz.\n");
    printf("\n      E o que NÃO se afirma: não se mediu aqui nada sobre neurónios reais, nem sobre\n");
    printf("      aprendizagem, nem sobre ativação não-linear. Este tecido é LINEAR, e é dele que\n");
    printf("      se falou. A ativação entra como ∏ e cumpre a cláusula; se ela for não-linear, o\n");
    printf("      que está aqui não se aplica sem se medir de novo.\n");
}

printf("\n=== O TECIDO ==============================================================\n");
printf("  ⊕ a soma no soma, ⊗ o peso sináptico, ν o inibitório, ∏ a ativação.\n\n");
printf("    só excitatório   o sinal NUNCA desce — é o (max,+) noutra roupa, e é polo\n");
printf("    com inibitório   det = ±1 é possível, e aí a rede DESFAZ o que fez\n");
printf("    recorrente       desfaz-se lendo as camadas ao contrário, cada uma invertida\n");
printf("    e é corpo        cumpre as doze cláusulas, pelo mesmo verificador dos unicórnios\n\n");
printf("  O inibitório não é o contrário do excitatório: é o que dá INVERSA à rede. E memória é\n");
printf("  isso — o sinal que entrou pode sair, sem se guardar cópia.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
