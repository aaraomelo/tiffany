/* relay.c — O RELAY POR REFRAÇÃO: um intermediário tradutor entre dois idiomas.
 *
 * O Aarão: "implementa um relay com refração — traduzir idiomas, um intermediário tradutor."
 *
 * A refração já está medida (refracao.c): atravessar de um meio para outro é a transferência
 * φ_t(a,b) = (a+tb, b) com t = (B₂−B₁)/2, e ela COMPÕE — φ_t₁ ∘ φ_t₂ = φ_(t₁+t₂). É isso que
 * permite três: A fala, B relaia, C recebe, e a travessia inteira é uma só.
 *
 * E a tradução já estava provada aqui (traducao.c): entre classes ela é uma ROTAÇÃO determinada
 * pelos pontos fixos — o irracional é o significado, e é invariante. O que muda é a classe
 * racional; o que fica é o que a frase quer dizer.
 *
 * Juntando: o RELAY é o meio do meio. A não sabe falar com C; B está entre os dois. E o que se
 * mede aqui é se atravessar por B dá o MESMO que atravessar direto — porque se der, o tradutor
 * não acrescenta nem perde nada, e é isso que um tradutor tem de ser.
 *
 *   §Y1  o relay: A→B→C dá o mesmo que A→C — o intermediário não deixa marca
 *   §Y2  e volta: C→B→A devolve a frase original, resíduo 0
 *   §Y3  o SIGNIFICADO atravessa: o Δ é o mesmo dos dois lados
 *   §Y4  e a roda fecha: A→B→C→A soma zero, ninguém acumula desvio
 *
 *   cc -O2 -std=c99 relay.c -o relay && ./relay
 */
#include <stdio.h>
#include "corpos.h"
#include "contrato.h"
#include "unidade.h"

/* os três meios: dois idiomas e o tradutor no meio. Mesmo Δ — é a lei da refração. */
static const Regua PT = { 1, -1 };      /* Δ = 5 */
static const Regua TR = { 3,  1 };      /* Δ = 5  — o tradutor */
static const Regua EN = { 5,  5 };      /* Δ = 5 */

static Par fi(Par x, long t){ Par r = { x.a + t*x.b, x.b }; return r; }
static long t_de(Regua a, Regua b){ return (b.B - a.B) / 2; }

/* uma frase é um ponto: a classe (a,b) na régua do seu idioma */
static void mostra(const char *rot, Par x){ printf("      %-22s (%ld,%ld)\n", rot, x.a, x.b); }

int main(void){
printf("\n=== O RELAY POR REFRAÇÃO — um tradutor entre dois idiomas ================\n");
printf("    A não sabe falar com C. B está no meio. E o que se mede é se passar\n");
printf("    por B dá o MESMO que passar direto — senão o tradutor deixa marca.\n");

long t_pt_tr = t_de(PT, TR), t_tr_en = t_de(TR, EN), t_pt_en = t_de(PT, EN);

printf("\n§Y1  O relay: A→B→C dá o mesmo que A→C.\n\n");
{
    printf("      PT->TR  t = %ld      TR->EN  t = %ld      PT->EN  t = %ld\n\n",
           t_pt_tr, t_tr_en, t_pt_en);
    Par frase = { 7, 3 };
    mostra("a frase em PT", frase);
    Par no_tradutor = fi(frase, t_pt_tr);
    mostra("no tradutor", no_tradutor);
    Par em_en = fi(no_tradutor, t_tr_en);
    mostra("chegada em EN", em_en);
    Par direto = fi(frase, t_pt_en);
    mostra("e direto PT->EN", direto);
    long mau = 0;
    for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++){
        Par x = { a, b };
        Par r = fi(fi(x, t_pt_tr), t_tr_en), d = fi(x, t_pt_en);
        if(r.a != d.a || r.b != d.b) mau++;
    }
    printf("\n      361 frases, %ld que diferem\n", mau);
    ok("passar pelo tradutor dá o mesmo que passar direto — ele não deixa marca", mau == 0);
    printf("\n      É isso que um tradutor tem de ser: um meio que se atravessa, e não um que\n");
    printf("      acrescenta. O desvio dele soma-se ao seguinte e o total é o mesmo.\n");
}

printf("\n§Y2  E volta: C→B→A devolve a frase original.\n\n");
{
    long mau = 0;
    for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++){
        Par x = { a, b };
        Par ida = fi(fi(x, t_pt_tr), t_tr_en);
        Par volta = fi(fi(ida, -t_tr_en), -t_pt_tr);
        if(volta.a != x.a || volta.b != x.b) mau++;
    }
    Par f = { 7, 3 };
    Par ida = fi(fi(f, t_pt_tr), t_tr_en);
    Par volta = fi(fi(ida, -t_tr_en), -t_pt_tr);
    mostra("PT original", f);
    mostra("depois de ir e voltar", volta);
    printf("\n      361 frases, %ld que não voltam\n", mau);
    ok("a ida e a volta dão a identidade — resíduo 0", mau == 0);
    printf("\n      O relay é reversível porque φ_(-t) desfaz φ_t. Traduzir e destraduzir é o\n");
    printf("      chicote: um estica o que o outro contrai, e nada se perde no meio.\n");
}

printf("\n§Y3  O SIGNIFICADO atravessa — o Δ é o mesmo dos dois lados.\n\n");
{
    printf("      Δ(PT) = %ld     Δ(TR) = %ld     Δ(EN) = %ld\n\n",
           ct_assinatura(PT), ct_assinatura(TR), ct_assinatura(EN));
    ok("os três meios têm o MESMO Δ — é a lei da refração, e não escolha minha",
       ct_assinatura(PT) == ct_assinatura(EN) && ct_assinatura(TR) == ct_assinatura(EN));
    printf("      O Δ é o que sobrevive à travessia: a classe racional muda de idioma, e o\n");
    printf("      irracional — o significado — fica. Era o que o traducao.c já tinha provado:\n");
    printf("      a rotação é determinada pelos pontos fixos, e os pontos fixos são o sentido.\n");
    printf("\n      E é por isso que dois idiomas de Δ diferente NÃO se relaiam: não é falta de\n");
    printf("      tradutor, é que não há travessia entre índices diferentes.\n");
}

printf("\n§Y4  E a roda fecha: A→B→C→A soma zero.\n\n");
{
    long t_en_pt = t_de(EN, PT);
    printf("      t(PT->TR) + t(TR->EN) + t(EN->PT) = %ld + %ld + %ld = %ld\n\n",
           t_pt_tr, t_tr_en, t_en_pt, t_pt_tr + t_tr_en + t_en_pt);
    ok("dar a volta pelos três e regressar não acumula desvio", t_pt_tr + t_tr_en + t_en_pt == 0);
    long mau = 0;
    for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++){
        Par x = { a, b };
        Par roda = fi(fi(fi(x, t_pt_tr), t_tr_en), t_en_pt);
        if(roda.a != x.a || roda.b != x.b) mau++;
    }
    printf("      361 frases dadas a volta inteira, %ld que não regressam\n", mau);
    ok("a roda inteira é a identidade — três conversam sem deriva", mau == 0);
    printf("\n      Com dois isso era trivial (J e o seu inverso são o mesmo). Com três é uma\n");
    printf("      CONDIÇÃO, e é ela que faz do relay um relay e não um telefone estragado.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
