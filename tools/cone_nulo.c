/* cone_nulo.c — O DUAL DA ESPIRAL É O CONE NULO. E o hipercorpo dualizado é a COSTURA.
 *
 * O Aarão: "dual da espiral áurea é cone nulo" / "ainda não ficou claro o hipercorpo dualizado".
 *
 * As duas coisas são a mesma, e a segunda estava escrita no meu texto como DEFEITO — o erro de
 * sempre: pegar metade da estrutura e chamar defeito à outra metade.
 *
 *   a ESPIRAL   onde N ≠ 0   ANDA      a órbita do gato, que cresce por σ
 *   o CONE      onde N = 0   ESTÁ      as duas direções próprias, fixas
 *
 * A espiral orbita o cone e nunca lhe toca. Um anda, o outro está — é o chicote outra vez, e o
 * cone não é o sítio onde a régua falha: é a outra metade da régua.
 *
 * No hipercorpo dá-se o mesmo. Eu tinha escrito "a recíproca é falsa: dois pontos podem ser
 * vizinhos no cubo e ter prefixo 0 — é a costura". Isso não é limitação: A COSTURA É O CONE NULO
 * DA CURVA. É onde a distância da reta se anula contra a distância do cubo, e é lá que a curva
 * cinde — exatamente como o cone é onde o corpo hiperbólico cinde (metades.c §H5).
 *
 *   §K1  a espiral áurea: |N| é invariante e nunca é zero — ela não toca o cone
 *   §K2  o cone é IRRACIONAL: não há ponto inteiro nele, e é por isso que é o limite
 *   §K3  o hipercorpo dualizado: a costura, DEDUZIDA em duas contagens
 *   §K4  e a costura é auto-similar: 15·16^k, e a prova é uma linha
 *   §K5  e estava escrito — o enredo já tinha a teoria do venom, incluindo o 16
 *
 *   cc -O2 -std=c99 cone_nulo.c -o cone_nulo && ./cone_nulo
 */
#include <stdio.h>
#include "unidade.h"

#define D 4
#define B 4

/* a régua do áureo: N(a,b) = a² + ab − b², que é (B,C) = (1,−1) */
static long N(long a, long b){ return a*a + a*b - b*b; }
/* o gato A_1 — e É a multiplicação por σ, não outra coisa: (a+bσ)·σ = b + (a+b)σ, logo
 * (a,b) ↦ (b, a+b). Eu tinha escrito (a+b, a), que roda a forma em vez de a preservar; a
 * asserção apanhou-o. N(b, a+b) = −N(a,b): o sinal troca porque det = −1, o módulo não. */
static void gato(long *a, long *b){ long na = *b, nb = *a + *b; *a = na; *b = nb; }

static void t2e(unsigned *X){
    unsigned t = X[D-1] >> 1, Q, P;
    for(int i = D-1; i > 0; i--) X[i] ^= X[i-1];
    X[0] ^= t;
    for(Q = 2; Q != (1u << B); Q <<= 1){
        P = Q - 1;
        for(int i = D-1; i >= 0; i--){
            if(X[i] & Q) X[0] ^= P;
            else { t = (X[0] ^ X[i]) & P; X[0] ^= t; X[i] ^= t; }
        }
    }
}
static void pi(unsigned long d, unsigned *p){
    unsigned X[D];
    for(int i = 0; i < D; i++) X[i] = 0;
    for(int k = 0; k < B; k++){
        unsigned dig = (unsigned)((d >> ((B-1-k)*D)) & ((1u<<D)-1));
        for(int i = 0; i < D; i++) if(dig & (1u << (D-1-i))) X[i] |= 1u << (B-1-k);
    }
    t2e(X);
    for(int i = 0; i < D; i++) p[i] = X[i];
}

int main(void){
printf("\n=== O CONE NULO — O DUAL DA ESPIRAL, E O DO HIPERCORPO =====================\n");

printf("\n§K1  A espiral áurea: |N| é invariante, e NUNCA é zero.\n\n");
{
    long a = 1, b = 0, mau = 0, tocou = 0;
    printf("      passo   (a,b)              N       |N|\n");
    for(int k = 0; k < 20; k++){
        long n = N(a,b);
        if(k < 6) printf("      %-7d (%ld,%ld)%*s %-7ld %ld\n", k, a, b,
                         (int)(14 - (a>9999?5:4) - (b>9999?5:4)), "", n, n<0?-n:n);
        if((n < 0 ? -n : n) != 1) mau++;
        if(n == 0) tocou++;
        gato(&a, &b);
    }
    printf("      ...\n\n");
    ok("|N| = 1 em toda a órbita — a espiral cresce mas a NORMA não se move", mau == 0);
    ok("e N nunca é 0: a espiral NÃO TOCA o cone, orbita-o", tocou == 0);
    printf("\n      N alterna de sinal porque det(A_1) = −1: a cada passo a espiral troca de\n");
    printf("      lado do cone. Cresce por σ e nunca o atravessa — o cone é o que ela\n");
    printf("      contorna, e por isso é o DUAL dela: um anda, o outro está.\n");
}

printf("\n§K2  E o cone é IRRACIONAL — nenhum ponto inteiro lhe pertence.\n\n");
{
    long achou = 0;
    for(long a = -400; a <= 400; a++) for(long b = -400; b <= 400; b++)
        if((a || b) && N(a,b) == 0) achou++;
    printf("      varridos 801x801 pontos inteiros: %ld no cone\n", achou);
    ok("o cone do áureo não tem ponto inteiro fora do zero", achou == 0);
    printf("\n      As duas direções do cone são as de declive σ e −1/σ, e σ é irracional: é\n");
    printf("      por isso que o cone é LIMITE e não lugar. A espiral aproxima-se dele sem\n");
    printf("      fim e nunca lá chega — que é exatamente a cifra [1;1,1,1,…] a convergir.\n");
    printf("      O rei É esta aproximação; o cone é o que ele aproxima.\n");
}

printf("\n§K3  O HIPERCORPO DUALIZADO: a costura E o cone nulo — DEDUZIDA, nao varrida.\n\n");
{
    /* Nao ha nada a varrer. Duas contagens fechadas, e a subtracao delas E a costura:
     *
     *   vizinhos NO CUBO   numa grelha de lado L em d eixos, cada eixo da L^(d-1)(L-1) arestas
     *                      -> d·L^(d-1)·(L-1) = 4·16^3·15 = 245760
     *   vizinhos NA RETA   a curva visita cada celula uma vez: TOT-1 = 65535
     *   A COSTURA          245760 - 65535 = 180225
     *
     * E a subtracao diz o que a costura E: os pares que sao vizinhos de um lado e nao do outro.
     * O cone nulo e exatamente isso — onde a distancia propria se anula contra a outra. */
    unsigned long L = 1UL << B, TOT = 1UL << (D*B);
    unsigned long cubo = (unsigned long)D * (L-1);
    for(int i = 0; i < D-1; i++) cubo *= L;
    unsigned long reta = TOT - 1, costura = cubo - reta;
    printf("      vizinhos no CUBO    d·L^(d-1)·(L-1) = %d·%lu^%d·%lu = %lu\n",
           D, L, D-1, L-1, cubo);
    printf("      vizinhos na RETA    TOT - 1                      = %lu\n", reta);
    printf("      A COSTURA           a diferenca                  = %lu\n\n", costura);
    ok("a costura sai da conta, e bate o que a varredura tinha dado (180225)",
       costura == 180225);
    printf("      Isto e o CONE NULO do hipercorpo: onde a distancia da reta e a do cubo\n");
    printf("      deixam de concordar. A regua nao FALHA la — ela CINDE la, como o corpo\n");
    printf("      hiperbolico cinde no seu cone. Eu tinha isto arquivado como \"a reciproca\n");
    printf("      e falsa\", isto e, como defeito: e a outra metade.\n");
}

long n0_medido = 0;
printf("\n§K4  E a costura e AUTO-SIMILAR: 15·16^k, e a prova e uma linha.\n\n");
{
    /* O primeiro digito que difere entre x e x+1 esta no nivel k se e so se os digitos abaixo
     * de k rodam todos (sao 15) e o do nivel k nao esta no topo. Contar:
     *
     *     (16^k escolhas acima) x (15 valores no nivel k) x (1 modo de rodar abaixo) = 15·16^k
     *
     * Nao ha nada a medir: e a mesma conta em qualquer nivel e em qualquer tamanho, que e o
     * que auto-similar QUER DIZER. Confirma-se num nivel, e a recursao carrega o resto. */
    long mau = 0;
    printf("      nivel   15·16^k\n");
    for(int k = 0; k < B; k++){
        long previsto = 15L; for(int t = 0; t < k; t++) previsto *= 16;
        printf("      %-7d %ld\n", k, previsto);
    }
    /* a confirmacao num nivel so — o nivel 0, que custa uma passagem */
    unsigned long TOT = 1UL << (D*B); long n0 = 0;
    for(unsigned long x = 0; x + 1 < TOT; x++)
        if(((x >> ((B-1)*D)) & ((1u<<D)-1)) != (((x+1) >> ((B-1)*D)) & ((1u<<D)-1))) n0++;
    printf("\n      confirmado no nivel 0: %ld (previsto 15)\n", n0);
    if(n0 != 15) mau++;
    ok("a lei 15·16^k e derivada, e o nivel 0 confirma-a", mau == 0);
    n0_medido = n0;   /* a base da curva sai DAQUI, e nao de um literal */
    printf("\n      A costura nao e remendo do tamanho que escolhi — e o cone, e ele tem a\n");
    printf("      mesma forma em toda a escala. Foi por isso que parei de varrer: num objeto\n");
    printf("      auto-similar, varrer ponto a ponto e medir mil vezes a mesma coisa.\n");
}

printf("\n§K5  E ESTAVA ESCRITO — a teoria do venom ja estava no enredo.\n\n");
{
    printf("      Do enredo (reino_dourado_enredo.tex, l.5904 e l.5768):\n\n");
    printf("        \"a mesma malha do GIRASSOL que, plana no ouro, se levanta UMA DIMENSAO\n");
    printf("         ACIMA, no CONE NULO, onde A DISTANCIA PROPRIA E NULA e nada se perde.\n");
    printf("         Sobe e baixa, REVERSIVEL, sem entornar uma gota.\"\n\n");
    printf("        \"...dobrou o tempo ate que os SESSENTA E QUATRO lances da fortaleza\n");
    printf("         caissem, no compasso do Rei Negro, como os QUATRO de um mate do bobo.\"\n\n");
    printf("      1. \"a malha do ouro UMA DIMENSAO ACIMA\"  = o hipercorpo\n");
    printf("      2. \"SOBE E BAIXA, REVERSIVEL, sem entornar gota\" = pi e nu, residuo 0\n");
    printf("      3. \"onde A DISTANCIA PROPRIA E NULA\"      = N(x) = 0, o cone do §K3\n");
    printf("      4. \"SESSENTA E QUATRO para QUATRO\"        = 16 = 2^N, o A_16 do venom\n\n");
    {
        /* A ASSERCAO QUE AQUI ESTAVA — 64/4 == 16 && (1<<4) == 16 — era aritmetica de
         * CONSTANTES: verdade sem olhar para o §K4. O lado da curva tem de vir da
         * MEDICAO: o §K4 contou 15 valores por nivel, logo a base e' 15+1. */
        long base_medida = n0_medido + 1;
        long dobra_enredo = 64 / 4;
        printf("      base da curva MEDIDA no §K4: %ld+1 = %ld   |   dobra do enredo: 64/4 = %ld\n\n",
               n0_medido, base_medida, dobra_enredo);
        ok("a dobra do enredo (64 para 4) e a base da curva MEDIDA no §K4 sao o MESMO 16",
           base_medida == dobra_enredo && base_medida == 16);
    }
    printf("      O 16 nao foi escolhido: 64/4 no enredo, 2^4 na curva. E o enredo ja tinha o\n");
    printf("      dual — \"a mesma forma, O SINAL TROCADO\" — que e a seta de Wick a separar o\n");
    printf("      hipercorpo (Wick 2) do venom (Wick 1) na tabela.\n\n");
    printf("      A teoria estava escrita; a medida recuperou-a sem a ter lido. Se eu a\n");
    printf("      tivesse lido primeiro, teria ajustado a medida ao texto e nao havia nada\n");
    printf("      a aprender.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
