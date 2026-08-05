/* colisor_ferramenta.c — O COLISOR ESCOLHE O PERCURSO, E ISSO APANHA UM DEFEITO.
 *
 * O Aarao: "ai se aplica o colisor — ele e' ferramenta, nao apenas classificacao."
 *
 * Ate' aqui o colisor foi lido: de (p,q,r) sai o grau, e do grau saem estados, colisoes e
 * regime. Isso classifica. Aqui ele OPERA: dado o numero de LADOS, ele diz quantos passos
 * fecham a orbita e QUAIS — e a resposta apanha um erro que estava escrito no repositorio.
 *
 *   §F1  com UM lado a orbita e' de 2, e o mesmo lado duas vezes fecha
 *   §F2  com DOIS lados a orbita e' de 4, e o mesmo lado duas vezes NAO fecha
 *   §F3  o percurso que o colisor da' fecha SEMPRE — verificado andando nele
 *   §F4  e o CONTROLO: o percurso ERRADO (mesmo lado repetido) leva ao ANTIPODA
 *   §F5  e a distancia ao antipoda e' a MESMA que a de um passo so' — que e' porque
 *        um residuo de meia orbita se confunde com "nao voltou"
 *
 * Zero doubles, zero .bss.
 *
 *   cc -O2 -std=c99 -Wall -I../lib colisor_ferramenta.c -o colisor_ferramenta
 */
#include <stdio.h>
#include "colisor.h"
#include "unidade.h"

static int popc(unsigned x){ int c=0; while(x){ c += x&1u; x >>= 1; } return c; }

int main(void){
    puts("\n  O COLISOR COMO FERRAMENTA — ele escolhe o percurso\n");

    /* dois lados: eixos 0..2 num bloco, 3..4 no outro. E' a particao do spinor32 §S6. */
    unsigned mA = 0x07u, mB = 0x18u;          /* 00111 e 11000 */
    unsigned masc1[1] = { mA };
    unsigned masc2[2] = { mA, mB };

    /* ═══ §F1 — um lado: a orbita e' de 2 ══════════════════════════════════════════ */
    {
        int p[8], k = colisor_percurso(1, p), mau = 0;
        for(unsigned x = 0; x < 32; x++)
            if(colisor_anda(x, masc1, p, k) != x) mau++;
        printf("      1 lado : o colisor manda %d passos", k);
        printf(" (%d", p[0]); for(int i=1;i<k;i++) printf(",%d", p[i]); printf(")\n");
        ok("com UM lado a orbita e' de dois e o percurso do colisor fecha em todos os 32"
           " estados — a involucao repetida E' a identidade, e ai `v.v = id` esta' certo",
           k == 2 && mau == 0);
    }

    /* ═══ §F2 e §F3 — dois lados: sao quatro, e alternados ═════════════════════════ */
    {
        int p[8], k = colisor_percurso(2, p), mau = 0;
        for(unsigned x = 0; x < 32; x++)
            if(colisor_anda(x, masc2, p, k) != x) mau++;
        printf("      2 lados: o colisor manda %d passos", k);
        printf(" (%d", p[0]); for(int i=1;i<k;i++) printf(",%d", p[i]); printf(")\n");
        ok("com DOIS lados sao QUATRO passos, e ALTERNADOS — o percurso que o colisor da'"
           " fecha em todos os 32 estados, verificado andando nele e nao pela formula",
           k == 4 && p[0]==0 && p[1]==1 && p[2]==0 && p[3]==1 && mau == 0);
    }

    /* ═══ §F4 — o CONTROLO: o percurso errado ══════════════════════════════════════
     * E' o que o tresp.c faz: aplica a MESMA involucao duas vezes num objecto de dois
     * lados. Mede-se onde isso vai parar — e nao e' a casa. */
    {
        int errado[2] = { 0, 0 };            /* o mesmo lado duas vezes */
        long voltou = 0, foi_antipoda = 0;
        for(unsigned x = 0; x < 32; x++){
            unsigned y = colisor_anda(x, masc2, errado, 2);
            if(y == x) voltou++;
        }
        /* e o percurso de DOIS passos por lados DIFERENTES leva ao antipoda do bloco */
        int meio[2] = { 0, 1 };
        for(unsigned x = 0; x < 32; x++){
            unsigned y = colisor_anda(x, masc2, meio, 2);
            if(y == (x ^ (mA | mB))) foi_antipoda++;
        }
        printf("\n      mesmo lado 2x: voltou em %ld de 32 — e por lados diferentes,"
               " %ld de 32 caem no ANTIPODA\n", voltou, foi_antipoda);
        ok("o percurso ERRADO nao e' um erro pequeno: aplicar o mesmo lado duas vezes"
           " devolve a identidade e portanto NAO TESTA NADA do segundo lado; e dois passos"
           " por lados diferentes levam ao ANTIPODA, que e' meia orbita e nao a casa",
           voltou == 32 && foi_antipoda == 32);
    }

    /* ═══ §F5 — e por isso meia orbita parece "nao voltou" ═════════════════════════
     * A distancia do antipoda ao ponto de partida e' o peso da mascara toda. E a distancia
     * de UM passo e' o peso de uma mascara. Se as duas forem da mesma ordem, um residuo de
     * meia orbita e' indistinguivel de "foi para longe e ficou" — que foi exactamente a
     * leitura errada que se fez do tresp.c: la' a ida dava 0,402 e o controlo 0,421, quase
     * iguais, e isso lia-se como ruido quando era ESTRUTURA. */
    {
        int d_meia = popc(mA | mB), d_um = popc(mA), d_outro = popc(mB);
        printf("      distancias em bits: um passo %d ou %d; meia orbita %d\n",
               d_um, d_outro, d_meia);
        ok("a distancia de MEIA ORBITA e' da mesma ordem da de um passo — logo um residuo"
           " de meia orbita confunde-se com 'foi e nao voltou'. E' preciso o colisor para"
           " os separar: ele diz quantos passos faltam, em vez de se medir o ruido",
           /* so' a conta que PODE falhar: se os blocos se sobrepusessem, o peso da
            * mascara total seria MENOR que a soma dos dois — e a orbita nao teria 4 */
           d_meia == d_um + d_outro && d_um > 0 && d_outro > 0);
    }

    /* ═══ §F6 — TRES BATIDAS SAO A VOLTA ═══════════════════════════════════════════
     * O Aarao: "isso sao 3 colisoes e volta, nao passa pelo 0 — ja' temos referencias
     * dessas 3 batidas no texto." Tem, em oito sitios do catalogo, sempre com a mesma
     * frase, e a conta e' F^4 = id => F^3 = F^{-1}.
     *
     * FECHAR e VOLTAR sao numeros diferentes: fechar leva 4, voltar leva 3. Mede-se com
     * um operador de periodo 4 de facto — a rotacao por i no plano inteiro, (a,b) ->
     * (-b,a), que e' a matriz do §S do texto e nao precisa de virgula flutuante. */
    {
        long mau = 0, tocou_partida = 0, tocou_zero = 0;
        for(int a0 = -6; a0 <= 6; a0++) for(int b0 = -6; b0 <= 6; b0++){
            if(!a0 && !b0) continue;                    /* o ponto fixo fica de fora */
            int a = a0, b = b0;
            /* tres batidas */
            int a3 = a0, b3 = b0;
            for(int k = 0; k < 3; k++){ int t = -b3; b3 = a3; a3 = t;
                if(a3 == a0 && b3 == b0) tocou_partida++;
                if(!a3 && !b3) tocou_zero++; }
            /* uma batida no sentido OPOSTO: (a,b) -> (b,-a) */
            int ai = b0, bi = -a0;
            if(a3 != ai || b3 != bi) mau++;
            /* e a quarta batida fecha */
            int t = -b3; b3 = a3; a3 = t;
            if(a3 != a0 || b3 != b0) mau++;
            (void)a; (void)b;
        }
        printf("\n      periodo 4: tres batidas contra uma no sentido oposto -> %ld discordancias\n", mau);
        printf("      e no caminho: passou pela partida %ld vezes, pelo zero %ld\n",
               tocou_partida, tocou_zero);
        ok("TRES BATIDAS SAO A VOLTA: com periodo 4, tres aplicacoes num sentido dao"
           " exactamente uma no sentido oposto (F^3 = F^-1), e a quarta fecha. E o percurso"
           " NAO passa pelo ponto de partida nem pelo zero no meio — FECHAR leva quatro,"
           " VOLTAR leva tres, e os dois numeros nao se substituem",
           mau == 0 && tocou_partida == 0 && tocou_zero == 0
           && colisor_volta(4) == 3 && colisor_passos(2) == 4);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O COLISOR NAO CLASSIFICA: ELE ESCOLHE O PERCURSO. Dado o numero de LADOS,");
        puts("  diz quantos passos fecham a orbita e QUAIS — dois passos com um lado, e");
        puts("  QUATRO ALTERNADOS com dois.");
        puts("");
        puts("  E ISSO APANHA UM DEFEITO QUE ESTAVA ESCRITO: uma prova de involucao que");
        puts("  aplique a MESMA involucao duas vezes num objecto de dois lados nao testa o");
        puts("  segundo lado — devolve a identidade e nao mede nada dele. E se os lados");
        puts("  forem diferentes mas so' dois, chega-se ao ANTIPODA, que e' meia orbita.");
        puts("");
        puts("  E A MEIA ORBITA CONFUNDE-SE COM 'NAO VOLTOU', porque a distancia dela e' da");
        puts("  ordem da de um passo. Foi essa a leitura errada do tresp.c — ida 0,402 e");
        puts("  controlo 0,421, quase iguais, lidos como ruido quando eram ESTRUTURA. O");
        puts("  colisor separa-os sem medir ruido nenhum: diz quantos passos faltam.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
