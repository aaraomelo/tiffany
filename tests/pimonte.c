/* pimonte.c — PI POR CONTAGEM: a ordenacao dos intervalos, e o Monte Carlo que a aproxima.
 *
 * O Aarao: "vc pega uma ordenacao especifica dos intervalos e lanca monte carlo, ele vai
 *           aproximar a area circular da regua" — e depois: "vai tender a pi".
 *
 * E' o terceiro caminho para o mesmo numero, e os tres nao se parecem:
 *
 *   1. SERIE     (pidual.c)  — Machin, Euler, Hermann: somas alternadas em inteiros
 *   2. DINAMICA  (a Teoria)  — pi := o menor t > 0 com exp(tJ)·1 = -1, o meio-periodo
 *   3. CONTAGEM  (aqui)      — quantos pontos do reticulado caem dentro
 *
 * E o terceiro tem uma coisa que os outros nao tem: E' EXATO SEM SER APROXIMADO. contar
 * pontos de Z^2 dentro de x^2+y^2 <= R^2 e' uma pergunta de inteiros e a resposta e' um
 * inteiro. Nenhuma serie, nenhum limite, nenhum float — e mesmo assim N(R)/R^2 -> pi.
 *
 * A ORDENACAO E' A REGUA. O reticulado e' a "ordenacao especifica dos intervalos": as casas
 * de tamanho 1 encaixadas no plano. Contar quantas caem dentro E' medir a area com essa
 * regua, e afinar a regua (R maior) e' apertar o encaixe. E' o teorema dos encaixantes com
 * o circulo no meio.
 *
 *   §M1  a contagem exata: N(R) e' inteiro, e N(R)/R^2 aproxima-se de pi
 *   §M2  o erro decresce: a regua mais fina mede melhor, e mede-se QUANTO
 *   §M3  o Monte Carlo: proposta cega + criterio dentro/fora — o CORPO TERMICO a medir
 *   §M4  os dois caminhos concordam entre si e com o pi das series
 *   §M5  e a contagem e' DUAL da medida: N cresce como R^2 e a casa vale 1/R^2
 *
 * Sem doubles no nucleo: as comparacoes sao x*x+y*y <= R*R, tudo em inteiros. Os quocientes
 * finais sao impressos em milionesimos inteiros, nao em virgula flutuante.
 *
 *   cc -O2 -std=c99 -Wall pimonte.c -o pimonte && ./pimonte
 */
#include <stdio.h>
#include "../lib/unidade.h"

typedef long L;

/* N(R) = #{(x,y) em Z^2 : x^2 + y^2 <= R^2} — contagem EXATA, so' inteiros */
static L conta_disco(L R){
    L n=0;
    for(L x=-R; x<=R; x++){
        L lim = R*R - x*x;
        for(L y=0; y*y<=lim; y++) n += (y==0) ? 1 : 2;   /* simetria em y */
    }
    return n;
}

static unsigned sem;
static unsigned prox(void){ sem = sem*1103515245u + 12345u; return (sem>>16)&0x7fff; }

int main(void){
    puts("\n  PI POR CONTAGEM — a regua do reticulado, e o acaso que a imita\n");

    /* ═══ §M1 — a contagem exata ═══════════════════════════════════════════════════════
     * pi em milionesimos = N(R)*1000000 / R^2, tudo inteiro. */
    puts("      R        N(R) exato      N/R^2 (milionesimos)");
    L Rs[6] = {10, 30, 100, 300, 1000, 3000};
    L q[6];
    for(int i=0;i<6;i++){
        L R=Rs[i], N=conta_disco(R);
        q[i] = N*1000000LL/(R*R);
        printf("      %5ld  %14ld      %ld\n", R, N, q[i]);
    }
    /* pi em milionesimos = 3141592 (nao escrito: vem de pidual.c, que o produziu por tres
     * somas alternadas independentes). aqui uso-o so' para COMPARAR, e a comparacao e' o
     * segundo caminho a bater no primeiro. */
    const L PI6 = 3141592;
    L err_grosso = q[0]-PI6; if(err_grosso<0) err_grosso=-err_grosso;
    L err_fino   = q[5]-PI6; if(err_fino<0)   err_fino=-err_fino;
    ok("a contagem de pontos do reticulado aproxima pi: com R=3000 o desvio e' minusculo",
       err_fino < 3000);

    /* ═══ §M2 — a regua mais fina mede melhor, e mede-se quanto ════════════════════════ */
    printf("      desvio com R=10: %ld milionesimos;  com R=3000: %ld  (reduziu %ldx)\n",
           err_grosso, err_fino, err_fino? err_grosso/err_fino : 0);
    ok("apertar o encaixe melhora a medida: o desvio cai por mais de uma ordem de grandeza",
       err_fino*10 < err_grosso);
    /* e nao e' monotono ponto a ponto — a contagem oscila em torno de pi, que e' o proprio
     * conteudo do problema do circulo. verifica-se que oscila, em vez de se afirmar. */
    int subiu=0, desceu=0;
    for(int i=1;i<6;i++){ if(q[i]>q[i-1]) subiu++; else if(q[i]<q[i-1]) desceu++; }
    ok("e a convergencia OSCILA em vez de descer sempre — houve passos nos dois sentidos",
       subiu>0 && desceu>0);

    /* ═══ §M3 — o Monte Carlo: o corpo termico a medir ═════════════════════════════════
     * proposta cega (um ponto ao acaso no quadrado) + criterio (dentro ou fora). e' o par
     * do corpo termico, com o criterio no caso mais duro que ha': temperatura zero, aceita
     * ou rejeita sem meio-termo. */
    const L LADO = 100000;
    L tentativas[3] = {10000, 200000, 2000000};
    L mc[3];
    puts("\n      tentativas        dentro      4*dentro/tent (milionesimos)");
    for(int i=0;i<3;i++){
        sem = 987654321u;
        L T=tentativas[i], dentro=0;
        for(L t=0;t<T;t++){
            L x = ((L)prox()*LADO)/32768, y = ((L)prox()*LADO)/32768;
            if(x*x + y*y <= LADO*LADO) dentro++;         /* criterio, em inteiros */
        }
        mc[i] = dentro*4000000LL/T;
        printf("      %10ld  %12ld      %ld\n", T, dentro, mc[i]);
    }
    L emc = mc[2]-PI6; if(emc<0) emc=-emc;
    ok("o Monte Carlo tende a pi: com 2 milhoes de tentativas o desvio e' pequeno",
       emc < 5000);

    /* e o acaso e' PIOR que a contagem exata com trabalho comparavel — o que e' o ponto:
     * a proposta cega paga a cegueira. */
    printf("      desvio Monte Carlo (2e6 pontos): %ld;  contagem exata (R=3000): %ld\n",
           emc, err_fino);
    ok("e a contagem exata bate o acaso: quem tem a regua ordenada nao precisa de tentar",
       err_fino <= emc);

    /* ═══ §M4 — os caminhos concordam ══════════════════════════════════════════════════ */
    L dif = q[5]-mc[2]; if(dif<0) dif=-dif;
    ok("os DOIS caminhos concordam entre si a menos de milesimos: reticulado e acaso dao o"
       " mesmo numero, e e' o mesmo que as series deram", dif < 10000);

    /* ═══ §M5 — a contagem e a medida sao o par ════════════════════════════════════════
     * N(R) cresce como R^2 e cada casa vale 1/R^2: o produto e' que nao se move. e' o
     * teorema da entropia com k e r do reticulado. */
    int mau=0;
    for(int i=1;i<6;i++){
        L Ra=Rs[i-1], Rb=Rs[i];
        L Na=conta_disco(Ra), Nb=conta_disco(Rb);
        /* Nb/Na deve aproximar (Rb/Ra)^2 — comparado por produto cruzado, sem divisao */
        L esq = Nb*Ra*Ra, dir = Na*Rb*Rb;
        L d = esq>dir ? esq-dir : dir-esq;
        /* O LIMITE NAO E' ESCOLHIDO: o erro de N(R) face a pi·R^2 e' da ordem do
         * PERIMETRO sobre a AREA, isto e' ~1/R — as casas cortadas pela borda. Logo o
         * desvio esperado entre dois degraus e' da ordem de 1/Ra, e e' isso que se exige.
         * (na 1.a versao pus 1% a olho e falhou em R=10, onde 1/R ja' vale 10%: o limiar
         *  inventado era o defeito, e a medicao estava certa.) */
        if(d*Ra > 4*dir) mau++;                  /* tolerancia 4/Ra, derivada da borda */
    }
    ok("N(R) cresce como R^2 em todos os degraus, dentro do erro de borda ~4/R: a contagem sobe o que a"
       " casa desce, e o produto e' que fica", !mau);

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  O TERCEIRO CAMINHO. Pi ja' tinha sido produzido por somas alternadas e");
        puts("  DEFINIDO como o meio-periodo do fluxo. Aqui e' CONTADO: quantas casas do");
        puts("  reticulado cabem no disco. Nenhum limite, nenhuma serie, nenhum float — uma");
        puts("  pergunta de inteiros com resposta inteira, e mesmo assim N(R)/R^2 vai dar la'.");
        puts("");
        puts("  E A ORDENACAO E' A REGUA. O reticulado e' o encaixe: casas de tamanho 1");
        puts("  arrumadas no plano, e apertar a regua e' aumentar R. E' o teorema dos");
        puts("  encaixantes com o circulo no meio — e o que sobrevive ao aperto e' pi.");
        puts("");
        puts("  E O ACASO PAGA A CEGUEIRA. O Monte Carlo chega ao mesmo sitio e chega pior:");
        puts("  com dois milhoes de tentativas fica atras da contagem exata. A proposta sem");
        puts("  ordem funciona — so' custa mais. Quem tem a regua ordenada nao precisa de");
        puts("  tentar, e e' essa a diferenca entre os dois lados do corpo termico.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
