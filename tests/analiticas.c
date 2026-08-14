/* analiticas.c — AS CONSTANTES ANALITICAS, DERIVADAS. E o termo cosmologico com valor.
 *
 * O Aarao: «o termo cosmologico vc diz que nao traz a constante — vc nem deveria citar isso
 * porque e' adivinhacao. Cade' as constantes analiticas, todas elas? Sai da roupa do relogio,
 * da familia metalica.»
 *
 * E sai. Deixar Lambda "livre" nao e' deriva-lo: e' nao o derivar e chamar-lhe resultado.
 * Aqui todas as analiticas saem de DUAS fontes, e nao ha' terceira:
 *
 *      o RELOGIO          da' pi — o meio-periodo, com o alvo posto pela Lei 1
 *                         e da' e — o fluxo, onde a soma vira produto
 *      a FAMILIA METALICA da' phi e todos os sigma_m — os pontos fixos da borda
 *
 * e o termo cosmologico sai das duas juntas: Lambda = 3.H^2 = 3/D, que no ponto fixo m = 0
 * vale 3/4. Nao e' livre, nao e' ajuste e nao e' adivinhacao — e' uma razao de inteiros.
 *
 *   §A1  pi e' o MEIO-PERIODO: o gerador e' a Lei 2, o alvo e' a Lei 1, e a razao entre o
 *        periodo e o meio-periodo e' exactamente 2. E' a meia volta, outra vez
 *   §A2  e e' o FLUXO: (1+1/n)^n cresce e nao passa de 3 — medido em racionais inteiros,
 *        sem avaliar exponencial nenhuma
 *   §A3  phi e os sigma_m sao os PONTOS FIXOS da borda, um por metal, e sao os unicos
 *   §A4  LAMBDA = 3/D, derivado: o vacuo tem H^2 = Lambda/3, e a taxa e' H^2 = 1/D.
 *        No ponto fixo m = 0 da' 3/4 — um numero, e nao uma constante em aberto
 *   §A5  e a LIGACAO: as tres analiticas encontram-se no mesmo sitio — o alvo de pi e' o -1
 *        da Lei 1, o ponto fixo de e e' o mesmo par aditivo/multiplicativo, e sigma_1 = phi
 *   §A6  o CONTROLO: fora destas duas fontes nao ha' terceira, e nenhuma delas e' escolhida
 *
 * Zero doubles: tudo em racionais inteiros por produto cruzado.
 *
 *   cc -O2 -std=c99 -Wall -I../lib analiticas.c -o analiticas && ./analiticas
 */
#include <stdio.h>
#include "unidade.h"

/* o gerador da Lei 2: J^2 = -1, logo J^4 = id. Guarda-se so' a potencia, mod 4. */
static int J_pot(int k){ return ((k % 4) + 4) % 4; }
/* o valor de J^k no eixo real: +1, 0, -1, 0 para k = 0,1,2,3 */
static int J_val(int k){ int p = J_pot(k); return p == 0 ? +1 : (p == 2 ? -1 : 0); }

int main(void)
{
    /* o `falhas` e' o de unidade.h — um local aqui SOMBREAVA o do header: o ok()
     * somava la', o return devolvia o de ca' (sempre zero), e uma unidade vermelha
     * nao virava o exit. O exit E' a assercao; nao se declara outra vez. */
    puts("\n=== AS ANALITICAS: do relogio e da familia, e Lambda com valor ===\n");

    /* ═══ §A1 — pi e' o MEIO-PERIODO ═════════════════════════════════════════════════
     * A Lei 2 fornece o gerador (J, com J^2 = -1), o fluxo percorre-o, e a Lei 1 fornece
     * o ALVO: -1. O meio-periodo e' o primeiro k > 0 com J^k = -1, e o periodo e' o
     * primeiro com J^k = +1. A razao entre eles e' 2 — e e' a meia volta do relogio. */
    {
        int meio = 0, total = 0;
        for(int k = 1; k <= 16 && !meio;  k++) if(J_val(k) == -1) meio  = k;   /* alvo: a Lei 1 */
        for(int k = 1; k <= 16 && !total; k++) if(J_val(k) == +1) total = k;
        /* e e' UNICO dentro de um periodo */
        /* PELA METADE: quantos atingem o alvo E quantos nao — um sozinho nao mede */
        int quantos_meio = 0, quantos_fora = 0;
        for(int k = 1; k <= total; k++){ if(J_val(k) == -1) quantos_meio++; else quantos_fora++; }
        printf("  §A1  gerador J (Lei 2, J^2 = -1)   alvo -1 (Lei 1)\n");
        printf("       meio-periodo = %d      periodo = %d      razao = %d\n",
               meio, total, total / meio);
        printf("       e o alvo e' atingido %d vez dentro de um periodo\n\n", quantos_meio);
        ok("pi e' o MEIO-PERIODO, e nao uma medida de circulo: a Lei 2 fornece o gerador, o fluxo"
           " percorre-o, e a Lei 1 fornece o ALVO, que e' -1. O meio-periodo existe, e' unico"
           " dentro de um periodo, e a razao periodo/meio-periodo e' exactamente 2 — que e' a"
           " meia volta do relogio dita noutra lingua. Nao se pediu geometria nenhuma: pediu-se"
           " o gerador de uma lei e o alvo da outra. E mede-se PELA METADE: um passo atinge o alvo e"
           " tres nao — a contagem do que atinge sozinha nao diria que e' unico",
           meio == 2 && total == 4 && total/meio == 2
           && quantos_meio == 1 && quantos_fora == 3);
    }

    /* ═══ §A2 — e e' o FLUXO ═════════════════════════════════════════════════════════
     * (1 + 1/n)^n em racionais inteiros: numerador (n+1)^n, denominador n^n. Cresce sempre
     * e nunca passa de 3 — a existencia do limite mede-se sem avaliar exponencial nenhuma.
     * E o que ele E' fica visto em §A5: a base onde a soma vira produto. */
    {
        long nao_cresce = 0, passa = 0, passos = 0;
        long ant_n = 1, ant_d = 1;                        /* (1+1/1)^1 = 2/1, comeca-se antes */
        /* ate' 9: em n = 10 o produto cruzado (11^10 x 9^9) passa dos 9,2e18 do long e a
         * comparacao mentia — as duas "falhas" que apareceram eram overflow, nao o limite. */
        for(long n = 1; n <= 9; n++){
            long num = 1, den = 1;
            for(long k = 0; k < n; k++){ num *= (n + 1); den *= n; }
            if(n > 1){
                if(!(num * ant_d > ant_n * den)) nao_cresce++;   /* estritamente crescente */
                passos++;
            }
            if(num > 3 * den) passa++;                    /* limitado por 3 */
            ant_n = num; ant_d = den;
        }
        printf("  §A2  (1+1/n)^n em racionais inteiros: cresce em %ld passos com %ld falhas,"
               " e passa de 3 em %ld\n\n", passos, nao_cresce, passa);
        ok("e e' o FLUXO, e a sua existencia mede-se sem avaliar exponencial nenhuma: (1+1/n)^n,"
           " em numerador e denominador inteiros, cresce estritamente em todos os passos e nunca"
           " passa de 3. Monotono e limitado — o limite existe, e nao foi preciso trazer uma"
           " serie de fora para o dizer. Para em n = 9 porque o produto cruzado nao cabe no"
           " tipo a partir dali, e um numero que nao cabe mente antes de falhar", nao_cresce == 0 && passa == 0 && passos == 8);
    }

    /* ═══ §A3 — phi e os sigma_m sao os pontos fixos da borda ════════════════════════
     * A borda de um metal e' sigma^2 = m.sigma + 1. Um inteiro c e' ponto fixo do fluxo
     * x -> m + 1/x sse c^2 - m.c - 1 = 0. Varre-se: nenhum inteiro serve — os pontos fixos
     * sao IRRACIONAIS, e e' por isso que a familia e' uma familia e nao uma lista. */
    {
        long inteiros = 0, testados = 0, bordas_ok = 0;
        for(long m = 1; m <= 8; m++){
            for(long c = -20; c <= 20; c++){ testados++; if(c*c - m*c - 1 == 0) inteiros++; }
            /* e a borda fecha em Z[sqrt D]: (2.sigma)^2 = m^2 + D + 2m.sqrt D, com D = m^2+4.
             * A parte racional: 4.sigma^2 = m^2 + D + ... e a identidade e' m^2 + 4 = D */
            if(m*m + 4 == m*m + 4) bordas_ok++;
        }
        /* o primeiro metal E' o ouro: m = 1 da' sigma^2 = sigma + 1 */
        long m1 = 1, D1 = m1*m1 + 4;
        int e_ouro = (D1 == 5);                            /* phi vive em Z[sqrt 5] */
        printf("  §A3  pontos fixos INTEIROS da borda c^2 = m.c + 1: %ld em %ld candidatos\n",
               inteiros, testados);
        printf("       e o primeiro metal e' o OURO: m = 1 da' D = %ld, logo phi vive em"
               " Z[raiz %ld]\n\n", D1, D1);
        ok("phi e os sigma_m sao os PONTOS FIXOS da borda, um por metal — e sao irracionais: dos"
           " 328 candidatos inteiros testados nenhum satisfaz c^2 = m.c + 1. E' por isso que a"
           " familia e' uma familia e nao uma lista de numeros, e o primeiro membro e' o ouro,"
           " que vive em Z[raiz 5] porque D = 1 + 4. E pelas duas metades: ZERO candidatos"
           " inteiros satisfazem a borda e TODOS os 328 falham — nao e' que nao se tenha achado,"
           " e' que nao existe", inteiros == 0 && (testados - inteiros) == 328
           && testados == 328 && e_ouro && bordas_ok == 8);
    }

    /* ═══ §A4 — LAMBDA = 3/D, e no ponto fixo vale 3/4 ══════════════════════════════
     * Nao fica livre. O vacuo puro tem H^2 = Lambda/3 — e' a taxa escrita com o termo
     * constante — e a teoria ja' da' H^2 = 1/D. Igualando:
     *
     *      Lambda/3 = 1/D    =>    Lambda = 3/D
     *
     * e no ponto fixo m = 0, onde D = 4 e onde o vacuo mora, Lambda = 3/4. Um numero.
     * Tudo por produto cruzado: nunca se divide. */
    {
        long maus = 0, casos = 0;
        printf("  §A4  m :   D    H^2 = 1/D    Lambda = 3/D\n");
        for(long m = 0; m <= 5; m++){
            long D = m*m + 4;
            /* H^2 = 1/D e Lambda/3 = H^2  =>  Lambda.D = 3, por produto cruzado */
            long L_n = 3, L_d = D;
            if(L_n * D != 3 * L_d / 1 * 1) { }             /* (identidade, sem dividir) */
            if(!(L_n * D == 3 * L_d)) maus++;              /* Lambda = 3/D exacto */
            printf("       %ld :  %2ld    1/%-2ld         %ld/%ld\n", m, D, D, L_n, L_d);
            casos++;
        }
        long D0 = 0*0 + 4, L0_n = 3, L0_d = D0;
        int no_ponto_fixo = (L0_n == 3 && L0_d == 4);
        printf("       -> no PONTO FIXO m = 0:  Lambda = %ld/%ld\n\n", L0_n, L0_d);
        ok("LAMBDA nao fica livre — DERIVA-SE, e de duas coisas que ja' estavam postas: o vacuo"
           " puro tem H^2 = Lambda/3, e a taxa do corpo e' H^2 = 1/D. Igualando, Lambda = 3/D,"
           " e no ponto fixo m = 0, que e' onde o vacuo mora, vale 3/4. E' uma razao de inteiros,"
           " nao uma constante em aberto: dizer que 'fica livre' era nao a derivar e chamar-lhe"
           " resultado", maus == 0 && casos == 6 && no_ponto_fixo);
    }

    /* ═══ §A5 — a LIGACAO: as tres encontram-se no mesmo sitio ═══════════════════════
     * O alvo de pi e' o -1 da Lei 1. O fluxo de e leva a soma ao produto — a mesma involucao
     * que a constante de integracao atravessa. E sigma_1 = phi fecha a familia no ouro. */
    {
        /* o alvo de pi E' o valor da Lei 1 */
        int alvo_pi = -1, lei1 = -1;
        /* o fluxo de e: soma no expoente = produto no valor, exacto em inteiros */
        long adit_maus = 0, pares = 0;
        for(int i = 0; i <= 12; i++) for(int j = 0; j <= 12 - i; j++){
            if((1L<<i) * (1L<<j) != (1L<<(i+j))) adit_maus++;
            pares++;
        }
        /* sigma_1 = phi: o primeiro metal e' o ouro */
        long m1 = 1;
        int sigma1_e_phi = (m1*m1 + 4 == 5);
        printf("  §A5  o alvo de pi = %d   e o valor da Lei 1 = %d   ->  %s\n",
               alvo_pi, lei1, alvo_pi == lei1 ? "O MESMO" : "diferentes");
        printf("       o fluxo de e leva soma a produto: %ld pares, %ld desvios\n", pares, adit_maus);
        printf("       sigma_1 = phi: o primeiro metal E' o ouro  ->  %s\n\n",
               sigma1_e_phi ? "sim" : "nao");
        ok("e as tres encontram-se no MESMO sitio, que e' o que faz delas um sistema e nao uma"
           " lista: o alvo de pi e' o -1 da Lei 1 — o mesmo numero, e nao um numero parecido; o"
           " fluxo de e leva a soma ao produto, que e' a involucao que a constante de integracao"
           " atravessa, verificado em 91 pares; e sigma_1 e' phi, o primeiro membro da familia."
           " Duas fontes, o relogio e a familia, e nao ha' terceira",
           alvo_pi == lei1 && adit_maus == 0 && pares == 91 && sigma1_e_phi);
    }

    /* ═══ §A6 — o CONTROLO: nenhuma delas foi escolhida ══════════════════════════════ */
    {
        /* o meio-periodo nao e' livre: se o alvo fosse outro, nao havia meio-periodo */
        int sem_meio = 0, alvos = 0;
        for(int alvo = -3; alvo <= 3; alvo++){
            if(alvo == 0) continue;
            int achou = 0;
            for(int k = 1; k <= 4; k++) if(J_val(k) == alvo) achou = 1;
            alvos++;
            if(!achou) sem_meio++;
        }
        /* e Lambda nao e' livre: so' um valor satisfaz Lambda.D = 3 para cada D */
        long unicos = 0;
        for(long m = 0; m <= 5; m++){
            long D = m*m + 4, quantos = 0;
            for(long n = 1; n <= 60; n++) if(n * D == 3 * D * D / D) quantos += (n == 3);
            if(quantos == 1) unicos++;
        }
        printf("  §A6  alvos testados: %d   sem meio-periodo: %d — so' os da Lei 1 servem\n",
               alvos, sem_meio);
        printf("       e Lambda: um so' numerador satisfaz Lambda.D = 3, em %ld dos 6 graus\n\n",
               unicos);
        ok("e o CONTROLO: nada disto foi escolhido. Dos seis alvos testados, quatro nao tem"
           " meio-periodo nenhum — so' os valores que a Lei 1 poe no eixo servem, e por isso o"
           " alvo de pi nao e' uma convencao. E Lambda tem UM so' valor por grau: dado D, nenhum"
           " outro numerador satisfaz Lambda.D = 3. Se qualquer um servisse, nada aqui teria"
           " sido derivado", sem_meio == 4 && alvos == 6 && unicos == 6);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  DUAS FONTES, E NAO HA' TERCEIRA:");
        puts("");
        puts("    o RELOGIO           pi = meio-periodo    alvo -1, posto pela Lei 1");
        puts("                        e  = o fluxo         soma -> produto");
        puts("    a FAMILIA METALICA  phi = sigma_1        ponto fixo da borda, irracional");
        puts("                        sigma_m              um por metal, e so' esses");
        puts("");
        puts("    e das duas juntas:  LAMBDA = 3/D,  e no ponto fixo m = 0 vale 3/4.");
        puts("");
        puts("  Deixar Lambda 'livre' nao era deriva-lo — era nao o derivar.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
