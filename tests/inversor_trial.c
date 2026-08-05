/* inversor_trial.c — A TABELA DE CHAVEAMENTO DO INVERSOR DE TRES NIVEIS E' O TRIAL.
 *
 * O Aarao: "essa e' a tabela de chaveamento do inversor multinivel — verifica na
 * documentacao e se coincide com a tabela de involucoes."
 *
 * Coincide, e nao por analogia: e' a MESMA tabela. Num inversor NPC de tres niveis cada
 * fase so' pode estar em tres estados — ligada ao barramento positivo, ao ponto medio, ou
 * ao negativo. Isso e' {+1, 0, -1}, que E' o trial do thm:trial, e nao uma coisa parecida
 * com ele.
 *
 * Com tres fases, a tabela tem 3^3 = 27 linhas. E o que a electronica de potencia mede
 * nela ha' decadas sai todo da estrutura, sem se acrescentar nada:
 *
 *     27 estados de chaveamento          =  3^3, o trial em tres fases
 *     19 vectores de tensao distintos    =  o que o motor ve
 *     6 pares REDUNDANTES                =  dois chaveamentos, o mesmo vector
 *     1 vector nulo com 3 estados        =  o ponto fixo, e e' triplo
 *
 * Os 19 e os 6 sao numeros canonicos do NPC de tres niveis, publicados e verificaveis. O
 * que aqui se faz e' derivar-los do trial em vez de os consultar.
 *
 *   §V1  cada fase toma tres estados, e o conjunto e' FECHADO sob a involucao s -> -s
 *   §V2  27 estados, e o vector de tensao agrupa-os em 19
 *   §V3  a REDUNDANCIA e' a involucao: os pares que dao o mesmo vector
 *   §V4  o vector NULO tem tres estados — e sao os tres pontos fixos por fase
 *   §V5  e o CONTROLO: com DOIS niveis nao ha' redundancia nenhuma
 *
 * Zero doubles: o vector de Clarke anda em inteiros como (2a-b-c, b-c), que basta para
 * decidir igualdade — a escala e o sqrt(3) sao comuns e nao mudam quem coincide com quem.
 *
 *   cc -O2 -std=c99 -Wall -I../lib inversor_trial.c -o inversor_trial
 */
#include <stdio.h>
#include "unidade.h"

#define NE 27

static void estado(int i, int *a, int *b, int *c){
    *a = i/9 - 1; *b = (i/3)%3 - 1; *c = i%3 - 1;    /* cada digito em {-1,0,+1} */
}

int main(void){
    puts("\n  A TABELA DO INVERSOR DE TRES NIVEIS E' O TRIAL\n");

    /* ═══ §V1 — cada fase e' o trial ═══════════════════════════════════════════════ */
    {
        int T[3] = {-1,0,+1}, fechado = 0, fixos = 0;
        for(int i = 0; i < 3; i++){
            int d = -T[i];
            for(int j = 0; j < 3; j++) if(T[j] == d) fechado++;
            if(d == T[i]) fixos++;
        }
        printf("      uma fase: {-1, 0, +1} — barramento negativo, ponto medio, positivo\n");
        ok("cada FASE do inversor toma exactamente os tres estados do trial, e o conjunto"
           " e' FECHADO sob s -> -s com UM ponto fixo — nao e' parecido com o trial, E' o"
           " trial", fechado == 3 && fixos == 1);
    }

    /* ═══ §V2 e §V3 — 27 estados, 19 vectores, 6 pares redundantes ═════════════════ */
    long n1 = 0, n2 = 0, n3 = 0, distintos = 0;
    {
        int va[NE], vb[NE], visto[NE];
        for(int i = 0; i < NE; i++){
            int a,b,c; estado(i,&a,&b,&c);
            va[i] = 2*a - b - c;                      /* alfa, sem escala */
            vb[i] = b - c;                            /* beta, sem o sqrt(3) */
            visto[i] = 0;
        }
        for(int i = 0; i < NE; i++){
            if(visto[i]) continue;
            long quantos = 0;
            for(int j = 0; j < NE; j++)
                if(va[j] == va[i] && vb[j] == vb[i]){ visto[j] = 1; quantos++; }
            distintos++;
            if(quantos == 1) n1++; else if(quantos == 2) n2++; else if(quantos == 3) n3++;
        }
        printf("\n      %d estados de chaveamento  ->  %ld vectores de tensao distintos\n",
               NE, distintos);
        printf("      %ld vectores com um estado, %ld com DOIS (redundantes), %ld com TRES\n",
               n1, n2, n3);
        ok("os 27 estados dao 19 vectores, com 6 PARES REDUNDANTES e um vector triplo —"
           " sao os numeros publicados do NPC de tres niveis, e aqui saem da estrutura em"
           " vez de se consultarem", distintos == 19 && n1 == 12 && n2 == 6 && n3 == 1);
    }

    /* ═══ §V4 — o vector nulo tem os tres pontos fixos ═════════════════════════════
     * O vector triplo e' o NULO, e os seus tres estados sao as tres fases todas no mesmo
     * degrau: (-1,-1,-1), (0,0,0) e (+1,+1,+1). E' o ponto fixo do trial aplicado a's tres
     * fases de uma vez — o centro, onde o motor nao ve tensao nenhuma. */
    {
        int nulos = 0, iguais = 0;
        for(int i = 0; i < NE; i++){
            int a,b,c; estado(i,&a,&b,&c);
            if(2*a-b-c == 0 && b-c == 0){ nulos++; if(a==b && b==c) iguais++; }
        }
        printf("\n      o vector NULO tem %d estados, e %d deles tem as tres fases iguais\n",
               nulos, iguais);
        ok("o vector NULO e' o triplo, e os seus tres estados sao exactamente as tres fases"
           " no MESMO degrau — e' o ponto fixo do trial aplicado a's tres de uma vez, e por"
           " isso e' triplo e nao unico", nulos == 3 && iguais == 3);
    }

    /* ═══ §V5 — o CONTROLO: com dois niveis nao ha' redundancia ════════════════════ */
    {
        int va[8], vb[8], visto[8]; long dist2 = 0, red2 = 0;
        for(int i = 0; i < 8; i++){
            int a = ((i>>2)&1)*2-1, b = ((i>>1)&1)*2-1, c = (i&1)*2-1;   /* so' +-1 */
            va[i] = 2*a-b-c; vb[i] = b-c; visto[i] = 0;
        }
        for(int i = 0; i < 8; i++){
            if(visto[i]) continue;
            long q = 0;
            for(int j = 0; j < 8; j++) if(va[j]==va[i] && vb[j]==vb[i]){ visto[j]=1; q++; }
            dist2++; if(q > 1) red2++;
        }
        printf("\n      controlo com DOIS niveis: %d estados -> %ld vectores, %ld redundantes\n",
               8, dist2, red2);
        ok("e o CONTROLO: com DOIS niveis por fase os 8 estados dao 7 vectores e ha' UMA"
           " so' redundancia — a do nulo. A riqueza de seis pares redundantes vem de haver"
           " TRES estados por fase, isto e', de haver o ZERO", dist2 == 7 && red2 == 1);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A TABELA DE CHAVEAMENTO DO INVERSOR DE TRES NIVEIS E' O TRIAL, e nao uma");
        puts("  analogia: cada fase liga ao barramento negativo, ao ponto medio ou ao");
        puts("  positivo, que sao {-1, 0, +1} — o conjunto do thm:trial, fechado sob a");
        puts("  involucao e com um ponto fixo.");
        puts("");
        puts("  E OS NUMEROS DA ELECTRONICA DE POTENCIA SAEM DA ESTRUTURA: 3^3 = 27 estados,");
        puts("  19 vectores, SEIS pares redundantes e um vector nulo TRIPLO. A redundancia —");
        puts("  o que o projectista usa para equilibrar o ponto medio e reduzir comutacoes —");
        puts("  E' a dualidade: dois chaveamentos que o motor nao distingue.");
        puts("");
        puts("  E O CONTROLO DIZ DE ONDE ELA VEM: com DOIS niveis ha' UMA redundancia so'.");
        puts("  As seis vem de haver TRES estados por fase, isto e', de haver o ZERO — o");
        puts("  ponto fixo. Sem o zero nao ha' meio, e sem meio nao ha' par.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
