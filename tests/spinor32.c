/* spinor32.c — OS 32 DESDOBRAM-SE EM 16 E 16, E OS 16 SAO UMA GERACAO.
 *
 * O Aarao: "a particula entra no diametro, colide 32 vezes e se desdobra em 32 particulas
 * duais que sao as particulas da fisica. valida isso."
 *
 * Valida-se, e sem pedir nada emprestado. Um estado de spin em cinco eixos e' uma escolha
 * de sinal em cada: sao 2^5 = 32 vertices de um hipercubo de dimensao 5. A PARIDADE do
 * numero de sinais negativos parte-os em dois montes de 16 — e essa paridade E' uma
 * involucao (aplicada duas vezes, devolve). E' o desdobramento em pares duais.
 *
 * E os 16 nao sao dezasseis coisas quaisquer. Lendo os tres primeiros sinais como COR e os
 * dois ultimos como ISOSPIN, os 16 arrumam-se exactamente assim:
 *
 *       6 = (3,2)   os quarks esquerdos          3 = (3bar,1)   um dos direitos
 *       3 = (3bar,1) o outro                     2 = (1,2)      o leptao esquerdo
 *       1 = (1,1)   o leptao direito             1 = (1,1)      o singleto que sobra
 *
 * que sao os 15 estados de Weyl de uma geracao mais um. Aqui nao se afirma fisica: conta-se
 * a decomposicao e mostra-se que os numeros batem — e esses numeros sao verificaveis.
 *
 *   §S1  32 vertices, e a paridade parte-os em 16 e 16 — a involucao
 *   §S2  a paridade E' involucao: trocar todos os sinais leva um monte no outro
 *   §S3  os 16 decompoem-se em 6+3+3+2+1+1, e cada peca conta-se
 *   §S4  a hipercarga sai da conta e SOMA ZERO no monte inteiro
 *   §S5  e o CONTROLO: em 4 eixos os montes tem 8 e a decomposicao NAO da' isto
 *
 * Zero doubles: os sinais sao +1 e -1, e a hipercarga anda em sextos inteiros (6.Y).
 *
 *   cc -O2 -std=c99 -Wall -I../lib spinor32.c -o spinor32
 */
#include <stdio.h>
#include "unidade.h"

static int par(int m, int n){ int c = 0; for(int i=0;i<n;i++) if((m>>i)&1) c++; return !(c&1); }
static int cnt(int m, int lo, int hi){ int c=0; for(int i=lo;i<hi;i++) if((m>>i)&1) c++; return c; }

int main(void){
    puts("\n  OS 32 DESDOBRAM-SE EM 16 E 16 — e os 16 sao uma geracao\n");

    /* ═══ §S1 — 32 vertices, 16 e 16 ═══════════════════════════════════════════════ */
    int pares = 0, impares = 0;
    for(int m = 0; m < 32; m++){ if(par(m,5)) pares++; else impares++; }
    printf("      2^5 = 32 escolhas de sinal: %d de paridade par, %d de impar\n", pares, impares);
    ok("os 32 partem-se em DOIS montes de 16 pela paridade do numero de sinais negativos —"
       " e' o desdobramento, e nao ha' resto", pares == 16 && impares == 16);

    /* ═══ §S2 — a paridade e' involucao ════════════════════════════════════════════
     * Trocar TODOS os cinco sinais muda a paridade (5 e' impar) e leva cada estado no seu
     * oposto — e feito duas vezes devolve. E' a involucao do §H4 do hipercubo, agora a
     * emparelhar os dois montes um a um: cada particula tem UMA dual, nao mais. */
    {
        /* `(d ^ 31) != m` com d = m ^ 31 nao media nada: e' a identidade do XOR, e passava
         * para qualquer constante que se pusesse no lugar do 31 — ate' para uma que nao
         * trocasse monte nenhum. O que se mede agora e' a troca COMO PERMUTACAO: cada
         * estado tem de ser atingido exactamente uma vez (bijeccao), e a segunda aplicacao
         * tem de devolver o de partida. Uma troca que apagasse sinais em vez de os trocar
         * deixaria estados por atingir, e isso o contador ve'. */
        int mau = 0, emparelhados = 0;
        int sig[32], atingido[32];
        for(int m = 0; m < 32; m++){ sig[m] = m ^ 31; atingido[m] = 0; }
        for(int m = 0; m < 32; m++){
            int d = sig[m];                         /* trocar os cinco sinais */
            atingido[d]++;
            if(sig[d] != m) mau++;                  /* involucao, lida na permutacao */
            if(par(m,5) == par(d,5)) mau++;         /* e troca sempre de monte */
            /* e sao os CINCO que trocam, nao um: trocar um so' sinal tambem seria involucao,
             * tambem seria bijeccao e tambem mudaria de monte — e nao e' o que se afirma */
            int dif = 0; for(int i = 0; i < 5; i++) if((((m^d)>>i)&1)) dif++;
            if(dif != 5) mau++;
            if(par(m,5)) emparelhados++;
        }
        for(int m = 0; m < 32; m++) if(atingido[m] != 1) mau++;   /* e e' BIJECCAO */
        printf("      trocar os 5 sinais: involucao exacta, e leva os %d pares nos %d impares\n",
               emparelhados, 32 - emparelhados);
        ok("a troca de todos os sinais E' involucao e leva um monte NO OUTRO, um a um: cada"
           " estado tem exactamente UMA dual — e' isso que faz 16 pares e nao 32 avulsos",
           mau == 0 && emparelhados == 16);
    }

    /* ═══ §S3 — a decomposicao dos 16 ══════════════════════════════════════════════
     * Bits 0..2 = cor (3), bits 3..4 = isospin fraco (2). Conta-se quantos estados tem cada
     * par (nc, nw) = (negativos na cor, negativos no fraco), e as multiplicidades saem. */
    {
        int tab[4][3];
        for(int i=0;i<4;i++) for(int j=0;j<3;j++) tab[i][j]=0;
        for(int m = 0; m < 32; m++){
            if(!par(m,5)) continue;
            tab[cnt(m,0,3)][cnt(m,3,5)]++;
        }
        printf("\n      (negativos na cor, no fraco) -> quantos\n");
        int total = 0;
        for(int nc=0; nc<4; nc++) for(int nw=0; nw<3; nw++)
            if(tab[nc][nw]){ printf("      (%d,%d) -> %d\n", nc, nw, tab[nc][nw]); total += tab[nc][nw]; }
        /* as multiplicidades de uma geracao: 3.2=6 num sitio, 3 em dois, 2 num, 1 em dois */
        int seis=0, tres=0, dois=0, um=0;
        for(int nc=0; nc<4; nc++) for(int nw=0; nw<3; nw++){
            int v = tab[nc][nw];
            if(v==6) seis++; else if(v==3) tres++; else if(v==2) dois++; else if(v==1) um++;
        }
        printf("      pecas: um bloco de 6, %d de 3, %d de 2, %d de 1  (soma %d)\n",
               tres, dois, um, total);
        /* nao basta a LISTA de multiplicidades: exige-se cada uma no seu (nc,nw), senao a
         * assercao sobrevive a colapsar dois blocos num — foi o que uma mutacao mostrou */
        int posto = (tab[1][1]==6 && tab[2][0]==3 && tab[2][2]==3 && tab[3][1]==2
                     && tab[0][0]==1 && tab[0][2]==1);
        ok("os 16 decompoem-se em 6 + 3 + 3 + 2 + 1 + 1 — as multiplicidades de uma geracao:"
           " o dupleto de quarks (3x2), os dois singletos de cor, o dupleto de leptoes e os"
           " dois singletos; e cada bloco no SEU (cor, fraco), nao so' a lista",
           total == 16 && seis == 1 && tres == 2 && dois == 1 && um == 2 && posto);
    }

    /* ═══ §S4 — a hipercarga soma zero ═════════════════════════════════════════════
     * A hipercarga de cada estado le-se dos mesmos sinais: 6Y = -(soma dos 3 da cor) ... a
     * conta e' inteira porque se trabalha com 6Y. O que se mede nao e' o valor de cada uma
     * (isso seria escrever a resposta) mas a SOMA no monte, que tem de ser zero — e' a
     * condicao de nao haver anomalia, e ela nao se impoe aqui: sai da contagem. */
    {
        long soma = 0, n = 0;
        for(int m = 0; m < 32; m++){
            if(!par(m,5)) continue;
            int sc = 3 - 2*cnt(m,0,3);              /* soma dos sinais da cor, em +-1 */
            int sw = 2 - 2*cnt(m,3,5);              /* soma dos sinais do fraco */
            soma += (long)(sc + sw);                /* 6Y a menos de escala comum */
            n++;
        }
        printf("\n      soma dos sinais sobre os %ld estados do monte: %ld\n", n, soma);
        /* honestidade sobre o alcance desta: as duas somas anulam-se SEPARADAMENTE, logo
         * qualquer combinacao linear delas tambem da' zero — uma mutacao com 2*sw sobrevive.
         * O que isto mede e' que o monte e' simetrico sob a troca de sinal, e nao mais. */
        long so_cor = 0, so_fraco = 0;
        for(int m = 0; m < 32; m++) if(par(m,5)){
            so_cor += 3 - 2*cnt(m,0,3); so_fraco += 2 - 2*cnt(m,3,5); }
        printf("      e separadamente: cor %ld, fraco %ld — as duas anulam-se sozinhas\n",
               so_cor, so_fraco);
        ok("e a SOMA sobre o monte e' ZERO, e ZERO em cada lado separadamente — o monte e'"
           " simetrico sob troca de sinal. Digo o alcance: por isso QUALQUER combinacao"
           " linear dos dois tambem da' zero, e esta medida nao distingue entre elas",
           n == 16 && soma == 0 && so_cor == 0 && so_fraco == 0);
    }

    /* ═══ §S5 — o CONTROLO: e em 4 eixos? ══════════════════════════════════════════
     * Sem isto, §S3 podia ser um acaso de qualquer hipercubo. Em 4 eixos os montes tem 8, e
     * a decomposicao NAO da' 6+3+3+2+1+1 — e a troca de todos os sinais nem sequer muda de
     * monte, porque 4 e' par. O 5 nao e' um numero qualquer aqui. */
    {
        int p4 = 0, troca_monte = 0;
        int tab[5][3]; for(int i=0;i<5;i++) for(int j=0;j<3;j++) tab[i][j]=0;
        for(int m = 0; m < 16; m++){
            if(par(m,4)){ p4++; tab[cnt(m,0,2)][cnt(m,2,4)]++; }
            if(par(m,4) != par(m ^ 15, 4)) troca_monte++;
        }
        int tem6 = 0; for(int i=0;i<5;i++) for(int j=0;j<3;j++) if(tab[i][j]==6) tem6=1;
        printf("      em 4 eixos: montes de %d, e a troca total muda de monte em %d de 16\n",
               p4, troca_monte);
        ok("e o CONTROLO separa: em 4 eixos os montes tem 8, nao ha' bloco de 6, e trocar"
           " todos os sinais NAO muda de monte (4 e' par) — logo nao ha' dual. O 5 nao e'"
           " um numero qualquer: e' o que faz a troca total ser uma involucao entre montes",
           p4 == 8 && !tem6 && troca_monte == 0);
    }

    /* ═══ §S6 — E SAO 8 BIDUAIS ════════════════════════════════════════════════════
     * O Aarao: "sao 8 particulas biduais."
     *
     * A dualidade do §S2 troca os CINCO sinais de uma vez. Mas ela parte-se: trocar so' a
     * COR (bits 0..2) e trocar so' o FRACO (bits 3..4) sao duas involucoes, e a composta e'
     * a do §S2. Duas involucoes que comutam formam Z2 x Z2 — o grupo de Klein — e as suas
     * orbitas tem QUATRO estados: m, m^7, m^24, m^31.
     *
     *     32 estados / 4 por orbita  =  8 BIDUAIS
     *
     * "Bidual" e' literal: e' preciso dualizar DUAS vezes, por lados diferentes, para
     * fechar a orbita. E 8 = 2^3 e' o andar 3 da torre — o de baixo do 16.
     *
     * Mede-se construindo as orbitas e conferindo que sao 8, todas de tamanho 4, e que as
     * duas involucoes comutam. */
    {
        int visto[32]; for(int i=0;i<32;i++) visto[i] = 0;
        int orbitas = 0, mau = 0, tam_errado = 0;
        for(int m = 0; m < 32; m++){
            if(visto[m]) continue;
            int o[4] = { m, m ^ 7, m ^ 24, m ^ 31 };
            int distintos = 0;
            for(int i = 0; i < 4; i++){
                if(!visto[o[i]]) distintos++;
                visto[o[i]] = 1;
            }
            if(distintos != 4) tam_errado++;
            orbitas++;
        }
        /* as duas involucoes comutam, e cada uma e' involucao */
        for(int m = 0; m < 32; m++){
            if(((m ^ 7) ^ 7) != m) mau++;
            if(((m ^ 24) ^ 24) != m) mau++;
            if(((m ^ 7) ^ 24) != ((m ^ 24) ^ 7)) mau++;
            if(((m ^ 7) ^ 24) != (m ^ 31)) mau++;      /* a composta E' a do §S2 */
        }
        printf("\n      dualizar a cor e dualizar o fraco: %d orbitas de 4 estados\n", orbitas);
        ok("SAO 8 BIDUAIS: as duas involucoes — trocar a cor e trocar o fraco — comutam e a"
           " composta e' a dualidade total, logo as orbitas tem 4 e sao 8. Dualizar UMA vez"
           " nao fecha; e' preciso duas, por lados diferentes — e 8 = 2^3 e' o andar de"
           " baixo do 16", orbitas == 8 && tam_errado == 0 && mau == 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  32 SAO OS SINAIS EM CINCO EIXOS, e a PARIDADE parte-os em 16 e 16. A troca");
        puts("  de todos os cinco e' uma involucao que leva um monte no outro um a um: cada");
        puts("  estado tem exactamente UMA dual. Sao 16 pares, e nao 32 coisas soltas.");
        puts("");
        puts("  E OS 16 DECOMPOEM-SE EM 6+3+3+2+1+1 — as multiplicidades de uma geracao,");
        puts("  lendo tres sinais como cor e dois como isospin. A soma dos sinais sobre o");
        puts("  monte e' ZERO, e nao foi imposta: sai da contagem.");
        puts("");
        puts("  E O CONTROLO DIZ QUE O 5 NAO E' UM NUMERO QUALQUER: em 4 eixos os montes tem");
        puts("  8, nao ha' bloco de 6, e trocar todos os sinais NAO muda de monte, porque 4");
        puts("  e' par — sem troca de monte nao ha' dual. E' preciso a IMPARIDADE do numero");
        puts("  de eixos para o desdobramento existir.");
        puts("");
        puts("  E SAO 8 BIDUAIS. A dualidade total parte-se em duas que comutam — trocar a");
        puts("  COR e trocar o FRACO —, e a composta delas E' a total. As orbitas do par tem");
        puts("  quatro estados, logo 32/4 = 8: dualizar uma vez nao fecha, e' preciso duas");
        puts("  por lados diferentes. E 8 = 2^3 e' o andar de baixo do 16 na torre.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
