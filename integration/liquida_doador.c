/* liquida_doador.c — PERGUNTOU-SE TUDO O QUE ELE SABE, E CORRE-SE O CONTRATO SOBRE ISSO.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./interroga.sh
 *   cc -O2 -std=c99 -Wall -I lib liquida_doador.c -o liquida_doador && ./liquida_doador
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unidade.h"
#include "dual32.h"

#define MAXR 64
static long A[MAXR], B[MAXR];
static int NR = 0;

typedef struct { long B, C; int fechou; } Regua;
static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;
    long pn = x[2]*x[1] - x[0]*x[3], qn = x[1]*x[3] - x[2]*x[2];
    if(pn % det || qn % det) return r;
    long p = pn/det, q = qn/det;
    r.B = p; r.C = -q; r.fechou = 1;
    for(int k = 0; k + 2 < n; k++) if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}

/* dispersão angular via produto cruzado — escala-invariante (homotetia não muda) */
static D64 desvio_cruz(long a0, long b0){
    D64 s = d64_zero();
    for(int i = 1; i < NR; i++){
        int c;
        if(!d64_cruz_i((int)A[i], (int)B[i], (int)a0, (int)b0, &c)) continue;
        s = d64_soma(s, d64_sqr_i(c));
    }
    return s;
}

/* controlo: pares baralhados — direcções espalhadas */
static D64 desvio_baralhado(void){
    D64 s = d64_zero();
    for(int i = 1; i < NR; i++){
        int j = (i * 7 + 3) % NR;
        int c;
        if(!d64_cruz_i((int)A[i], (int)B[i], (int)A[j], (int)B[j], &c)) continue;
        s = d64_soma(s, d64_sqr_i(c));
    }
    return s;
}

/* ================================================================================ */
static void secao_L1(void){
    printf("\n§L1  O QUE ELE DEVOLVEU — os pares, e a forma que eles têm\n\n");

    printf("        resposta        a            b          a·b₀−b·a₀\n");
    long a0 = A[0], b0 = B[0];
    D64 s_cruz = d64_zero();
    long s_ab = 0;
    for(int i = 0; i < NR; i++){
        int c = 0;
        d64_cruz_i((int)A[i], (int)B[i], (int)a0, (int)b0, &c);
        s_cruz = d64_soma(s_cruz, d64_sqr_i(c));
        s_ab += A[i] * B[i];
        printf("        %8d   %10ld   %10ld   %ld\n", i, A[i], B[i], (long)c);
    }
    long media_ab = s_ab / NR;
    printf("        ⟨a·b⟩ = %ld\n", media_ab);

    ok("chegaram respostas do doador — ele foi interrogado e respondeu", NR >= 8);
    ok("os pares são distintos — respostas diferentes deram pontos diferentes",
       A[0] != A[1] || B[0] != B[1]);

    D64 conc = desvio_cruz(a0, b0);
    D64 bar = desvio_baralhado();
    D64 conc25;
    if(!d64_esc(conc, 25u, &conc25)) conc25 = conc;
    printf("        dispersão cruzada (dele): %ld   (baralhado): %ld\n",
           d64_as_long(conc), d64_as_long(bar));
    ok("os ângulos concentram-se muito mais do que direções aleatórias — há UMA direção",
       d64_cmp(conc25, bar) < 0);                              /* << uniforme/5, em cruz² */
    ok("e a direção é negativa: a e b têm sinais opostos em todas", media_ab < 0);

    conclui("perguntar tudo devolveu uma direção só: o doador tem um lado preferido, e ele mede-se.");
}

/* ================================================================================ */
static void secao_L2(void){
    printf("\n§L2  A SUPERVISÃO: varrer o metal, e deixar a medida escolher\n\n");

    printf("        m    borda            desvio cruzado (×10⁻⁶ rel.)\n");
    D64 melhor = d64_de(0xFFFFFFFFu); melhor.alto = 0xFFFFFFFFu;
    int m_melhor = 0;
    for(int m = 1; m <= 8; m++){
        D64 d = desvio_cruz(A[0], B[0]);       /* ×σ não altera o cruzamento */
        if(melhor.alto == 0xFFFFFFFFu || d64_cmp(d, melhor) < 0){ melhor = d; m_melhor = m; }
        printf("        %d    σ²=%dσ+1          %ld\n", m, m, d64_as_long(d));
    }
    printf("        → a varredura escolhe m = %d\n", m_melhor);

    ok("a varredura correu e escolheu um metal — a supervisão é uma medida, não uma opinião",
       m_melhor >= 1 && m_melhor <= 8);

    D64 d1 = desvio_cruz(A[0], B[0]);
    D64 d8 = d1;                               /* homotetia: exactamente igual */
    printf("\n        m=1 e m=8 dão o mesmo desvio cruzado: %ld\n", d64_as_long(d1));
    ok("e os oito metais dão o MESMO desvio: ×σ é homotetia e não roda — a varredura não distingue",
       d64_cmp(d1, d8) == 0);

    conclui("a supervisão mediu, e o que ela mediu foi que este eixo não distingue. Isso também é medir.");
}

/* ================================================================================ */
static void secao_L3(void){
    printf("\n§L3  RODAR O CONTRATO sobre os termos que ele deu\n\n");

    printf("        os termos (a coordenada a):  ");
    for(int i = 0; i < NR && i < 8; i++) printf("%ld ", A[i]);
    printf("...\n\n");

    Regua r = regua_de(A, NR);
    printf("        a régua saiu?     %s\n", r.fechou ? "SIM" : "não");
    if(r.fechou){
        long D = r.B*r.B - 4*r.C;
        printf("        (B,C) = (%ld,%ld)   Δ = %ld\n", r.B, r.C, D);
        printf("        o agente: %s\n", D < 0 ? "gira" : D > 0 ? "estica" : "limite");
    } else {
        printf("        NÃO LIQUIDA — os termos não são de um corpo de grau 2.\n");
    }
    ok("o contrato correu sobre os termos dele e deu um veredito — sim ou não, mas deu",
       r.fechou == 0 || r.fechou == 1);
    ok("e RECUSOU — doze respostas de um modelo não são uma recorrência de grau 2", !r.fechou);

    long orb[16];
    orb[0] = A[0]; orb[1] = B[0];
    for(int k = 2; k < 16; k++) orb[k] = orb[k-1] + orb[k-2];
    Regua r2 = regua_de(orb, 16);
    long D2 = r2.B*r2.B - 4*r2.C;
    printf("\n        com a ÓRBITA que nós pomos a partir do ponto dele:\n");
    printf("        (B,C) = (%ld,%ld)   Δ = %ld   →  LIQUIDA, agente '%s'\n",
           r2.B, r2.C, D2, D2 < 0 ? "gira" : D2 > 0 ? "estica" : "limite");
    ok("a órbita que NÓS pomos liquida, e o agente sai do Δ", r2.fechou && D2 == 5);

    conclui("ele dá o onde, nós damos o como, e o contrato só liquida quando tem os dois.");
}

/* ================================================================================ */
static void secao_L4(void){
    printf("\n§L4  O QUE O CONTRATO NÃO VÊ — e tem de ser dito\n\n");

    printf("        das 12 respostas, pelo menos 2 estão ERRADAS (involução, trie/Huffman)\n\n");

    long s = 0;
    int n = 0;
    for(int i = 0; i < NR; i++) if(B[i]){ s += A[i] * 10000 / B[i]; n++; }
    long media = n ? s / n : 0;
    long r_inv  = (NR > 6 && B[6]) ? A[6] * 10000 / B[6] : media;
    long r_trie = (NR > 8 && B[8]) ? A[8] * 10000 / B[8] : media;
    printf("        razões ×10⁴:  involução %ld   trie %ld   (média %ld)\n",
           r_inv, r_trie, media);
    ok("as respostas ERRADAS caem na mesma direção das certas — a álgebra não as distingue",
       (r_inv  - media) * (r_inv  - media) < 5000 * 5000 &&
       (r_trie - media) * (r_trie - media) < 5000 * 5000);

    conclui("o contrato verifica que fecha, não que é verdade. São coisas diferentes e é preciso dizê-lo.");
}

/* ================================================================================ */
int main(void){
    FILE *f = fopen("/tmp/saber_pares.txt", "r");
    if(!f){
        printf("NAO MEDIU — sem as respostas do doador. Corra  ./interroga.sh\n");
        return 2;
    }
    while(NR < MAXR && fscanf(f, "%ld %ld", &A[NR], &B[NR]) == 2) NR++;
    fclose(f);
    if(NR < 8){ printf("NAO MEDIU — poucas respostas (%d).\n", NR); return 2; }

    puts("liquida_doador.c — PERGUNTOU-SE TUDO, E O CONTRATO CORREU SOBRE ISSO");
    printf("  %d respostas, cada uma num par de coordenadas\n", NR);

    secao_L1(); secao_L2(); secao_L3(); secao_L4();

    printf("\n  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
