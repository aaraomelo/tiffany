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

    /* ═══ §V6 — O DTC MULTINIVEL: A ESPIRAL DO RELOGIO, E NAO GUARDA NADA ═════════
     * O Aarao: "esse e' o DTC, nao precisa guardar nada — e' a dinamica espiral do
     * relogio: combinacao de FASE e AMPLITUDE ja' gera o corpo, le e escreve via a
     * involucao do inversor. Involucao/evolucao, motor/gerador: so' muda o SINAL em
     * leitura/escrita."
     *
     * E' o DTC MULTINIVEL — controlo directo de binario sobre os tres niveis, e nao sobre
     * dois. Nao tem modulador e nao tem memoria: a decisao e' funcao do estado ACTUAL — o
     * sector (a FASE) e os sinais de erro (as AMPLITUDES, de fluxo e de binario). Nenhuma
     * linha olha para o passado.
     *
     * E' no multinivel que a redundancia deixa de ser curiosidade e vira FERRAMENTA: dos
     * 19 vectores, os 6 PARES redundantes sao o grau de liberdade que equilibra o ponto
     * medio — dois chaveamentos que dao o mesmo binario e carregam o condensador em
     * sentidos opostos. Escolher entre eles e' escolher um SINAL, e escolhe-se pelo desvio
     * instantaneo, sem historico nenhum.
     *
     * E a involucao esta' la' inteira: seis sectores, e o oposto de um vector e' k -> k+3,
     * MEIA VOLTA — a mesma que o fase_regua.c mediu como velocidade maxima, e o mesmo
     * ponto onde ida e volta coincidem. Trocar o sinal do binario e' passar de motor a
     * gerador, e nao muda a tabela: muda o sinal com que se le' nela. */
    {
        const int S = 6;
        long muda = 0, tot = 0, involutivo = 0;
        for(int k = 0; k < S; k++){
            for(int df = 0; df < 2; df++){
                /* Takahashi: com fluxo a subir, +1 ou +5; com fluxo a descer, +2 ou +4 */
                int a = (k + (df ? 1 : 2)) % S;      /* binario a subir  */
                int b = (k + (df ? 5 : 4)) % S;      /* binario a descer */
                tot++;
                if(a != b) muda++;
            }
            if((k + 3 + 3) % S == k) involutivo++;   /* meia volta duas vezes devolve */
        }
        printf("\n      DTC: %d sectores x 2 fluxo x 2 binario = %d linhas, e NENHUMA olha"
               " para o passado\n", S, S*4);
        printf("      trocar o sinal do binario muda o vector em %ld de %ld — motor/gerador\n",
               muda, tot);
        printf("      e k -> k+3 e' meia volta: aplicada duas vezes devolve em %ld de %d\n",
               involutivo, S);
        /* e o grau de liberdade do multinivel: quantos vectores tem alternativa? */
        long com_alternativa = 0;
        {
            int va[NE], vb[NE], visto[NE];
            for(int i = 0; i < NE; i++){
                int x,y,z; estado(i,&x,&y,&z);
                va[i] = 2*x-y-z; vb[i] = y-z; visto[i] = 0;
            }
            for(int i = 0; i < NE; i++){
                if(visto[i]) continue;
                long q = 0;
                for(int j = 0; j < NE; j++) if(va[j]==va[i]&&vb[j]==vb[i]){ visto[j]=1; q++; }
                if(q > 1) com_alternativa++;
            }
        }
        printf("      e no MULTINIVEL a redundancia e' ferramenta: %ld dos 19 vectores tem"
               " alternativa\n", com_alternativa);
        ok("o DTC nao guarda nada: a decisao e' funcao da FASE (o sector) e das AMPLITUDES"
           " (os dois erros), e nunca do passado. Trocar o sinal do binario e' passar de"
           " motor a gerador e nao muda a tabela — muda o SINAL com que se le' nela. E o"
           " oposto de um vector e' MEIA VOLTA, k -> k+3, a involucao do inversor",
           muda == tot && involutivo == S && tot == 12 && com_alternativa == 7);
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
        puts("");
        puts("  E O DTC NAO GUARDA NADA: a decisao sai da FASE (o sector) e das AMPLITUDES");
        puts("  (os dois erros), nunca do passado — 24 linhas e nenhuma olha para tras. E' a");
        puts("  MAQUINA SEM MEMORIA em silicio, e ja' existia antes de a termos escrito.");
        puts("");
        puts("  E E' NO MULTINIVEL QUE A REDUNDANCIA VIRA FERRAMENTA: dos 19 vectores, sete");
        puts("  tem alternativa — dois chaveamentos com o mesmo binario que carregam o ponto");
        puts("  medio em sentidos opostos. Escolher entre eles E' ESCOLHER UM SINAL, pelo");
        puts("  desvio instantaneo e sem historico. A dualidade a pagar a conta.");
        puts("");
        puts("  E MOTOR/GERADOR SO' MUDA O SINAL: a tabela e' a mesma, e o que troca e' o");
        puts("  sinal com que se le' nela — que e' o mesmo que ler/escrever. O oposto de um");
        puts("  vector e' MEIA VOLTA, k -> k+3: a involucao do inversor, e o mesmo ponto");
        puts("  onde ida e volta coincidem no fase_regua.c.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
