/* meia_passagem.c — CORTAR AO MEIO JA' VALIDA. Uma passagem, e nao duas.
 *
 * (Terceiro nome tentado. protocolo.c ja' existia — as seis fases da tunica contra o painel —
 * e metades.c tambem, sobre nao haver "nao e' corpo". Escrevi por cima dos dois na mesma
 * sequencia: o primeiro com Write, que AVISOU "updated" e eu vi; o segundo por Python, que
 * nao avisa nada. A licao que faltava: verificar ANTES de escrever, e nao esperar pelo aviso,
 * porque nem todos os caminhos o dao. Ambos recuperados do git.)
 *
 * O Aarao: «se voce reverter o sinal da reversao vai dar DUAS VEZES o objecto. Entao se
 * cortar na metade ja' valida sem quebrar nada. Verificacao seria: inverte o sinal da
 * reversao e divide por dois — ve' se da' certo como protocolo.»
 *
 * Da'. E e' mais barato do que reverter. A reversao precisa de DOIS passos — ida e volta —
 * e le' o residuo no fim. Isto precisa de UM: aplica-se a involucao uma vez e leem-se as
 * duas metades ao mesmo tempo.
 *
 *      S = (x + x^dag)/2      a parte SIMETRICA      — o que NAO se moveu
 *      A = (x - x^dag)/2      a parte ANTISSIMETRICA — o objecto, e e' o sinal invertido
 *
 *      S + A = x              residuo 0, numa passagem
 *
 * E porque e' que cortar ao meio nao quebra nada: porque as duas metades RECONSTROEM. Nao
 * se perde nada ao dividir — perde-se ao ficar com uma so'. E' a mesma frase de sempre, e
 * agora e' o protocolo: a estaca move (da' o A) e a cruz mede o que nao se moveu (da' o S).
 *
 * E ha' aqui um facto que faz o protocolo FECHAR em inteiros: as duas somas sao SEMPRE
 * PARES, porque x + x^dag = 2c e x - x^dag = 2(x-c). A divisao por dois e' exacta, e nao e'
 * sorte — e' a involucao a garanti-la. Num objecto que nao seja involutivo ela falha.
 *
 *   §P1  S + A = x com residuo 0, numa passagem so'
 *   §P2  a divisao por dois e' EXACTA: as duas somas sao sempre pares, e a paridade vem da
 *        involucao — nao e' sorte nem arredondamento
 *   §P3  e as metades sao o que dizem ser: S^dag = S (nao se move) e A^dag = -A (e' o que
 *        se move). Cada uma le-se pela sua propria involucao, sem comparar com nada
 *   §P4  o protocolo custa UMA passagem contra DUAS da reversao — conta-se
 *   §P5  o CONTROLO: num objecto que NAO e' involucao, a divisao por dois deixa de ser
 *        exacta e a soma das metades deixa de reconstruir. O protocolo acusa
 *
 * Sem um unico numero esperado escrito: todas as asseroes sao residuo 0.
 *
 *   cc -O2 -std=c99 -Wall -I../lib meia_passagem.c -o meia_passagem && ./meia_passagem
 */
#include <stdio.h>
#include "unidade.h"

#define N 400

static long res(long a, long b){ long d = a - b; return d < 0 ? -d : d; }

/* a involucao: a estaca de centro c */
static long dag(long x, long c){ return 2*c - x; }
/* e um impostor: uma translacao, que NAO e' involucao */
static long nao_dag(long x, long c){ return x + c; }

int main(void)
{
    long falhas = 0;
    puts("\n=== O PROTOCOLO: inverter o sinal e dividir por dois ===\n");

    /* ═══ §P1 — S + A = x, numa passagem ════════════════════════════════════════════ */
    {
        long resid = 0, casos = 0;
        for(long c = -30; c <= 30; c++)
            for(long x = -N; x <= N; x++){
                long d = dag(x, c);                    /* UMA aplicacao, e mais nenhuma */
                long S = (x + d) / 2;                  /* o que nao se moveu */
                long A = (x - d) / 2;                  /* duas vezes o objecto, a meio */
                resid += res(S + A, x);
                casos++;
            }
        printf("  §P1  S + A reconstroi x em %ld casos:  residuo %ld\n\n", casos, resid);
        ok("o PROTOCOLO fecha: aplicada a involucao UMA vez, a meia-soma e a meia-diferenca"
           " reconstroem o objecto com residuo zero. Cortar ao meio nao quebra nada porque as"
           " duas metades reconstroem — o que se perde e' ficar com uma so'. E nao houve ida e"
           " volta: houve uma passagem", resid == 0 && casos > 0);
    }

    /* ═══ §P2 — a divisao por dois e' EXACTA, e a paridade vem da involucao ═════════ */
    {
        long impares = 0, casos = 0;
        for(long c = -30; c <= 30; c++)
            for(long x = -N; x <= N; x++){
                long d = dag(x, c);
                if((x + d) % 2) impares++;             /* tem de ser sempre par */
                if((x - d) % 2) impares++;
                casos += 2;
            }
        printf("  §P2  somas impares (que quebrariam a divisao) em %ld:  %ld\n\n", casos, impares);
        ok("e a divisao por dois e' EXACTA, o que faz o protocolo fechar em inteiros: as duas"
           " somas sao sempre pares, e nao por sorte — x + x^dag e' 2c e x - x^dag e' 2(x-c),"
           " logo e' a involucao que garante a paridade. Em nenhum dos casos houve um resto a"
           " arredondar", impares == 0 && casos > 0);
    }

    /* ═══ §P3 — as metades sao o que dizem ser ══════════════════════════════════════ */
    {
        long r_sim = 0, r_ant = 0;
        for(long c = -30; c <= 30; c++)
            for(long x = -N; x <= N; x++){
                long d = dag(x, c);
                long S = (x + d) / 2, A = (x - d) / 2;
                /* S nao se move: aplicar a involucao a S devolve S */
                r_sim += res(dag(S, c), S);
                /* A e' o que se move: a sua involucao propria (em torno de zero) devolve -A */
                r_ant += res(-A, -A);                  /* trivial; o teste real e' o seguinte */
                /* A muda de sinal quando x passa para o outro lado do centro */
                long x_esp = dag(x, c);                /* o espelho de x */
                long d2 = dag(x_esp, c);
                long A_esp = (x_esp - d2) / 2;
                r_ant += res(A_esp, -A);               /* o A do espelho E' -A */
            }
        printf("  §P3  a metade simetrica nao se move: residuo %ld;"
               "  a antissimetrica troca de sinal no espelho: residuo %ld\n\n", r_sim, r_ant);
        ok("e as metades sao o que dizem ser, e le-se sem comparar com valor nenhum: a parte"
           " simetrica e' fixa pela involucao — aplicada, devolve-se a si propria — e a"
           " antissimetrica troca de sinal quando o ponto passa para o outro lado do centro."
           " Uma e' o que a cruz mede, a outra e' o que a estaca move, e agora sao duas metades"
           " da mesma passagem", r_sim == 0 && r_ant == 0);
    }

    /* ═══ §P4 — o custo: UMA passagem contra DUAS ══════════════════════════════════ */
    {
        long chamadas_prot = 0, chamadas_rev = 0;
        for(long x = -N; x <= N; x++){
            long c = 7;
            long d = dag(x, c); chamadas_prot++;              /* protocolo: uma */
            long S = (x + d)/2, A = (x - d)/2; (void)S; (void)A;
            long y = dag(x, c); chamadas_rev++;               /* reversao: ida ... */
            long z = dag(y, c); chamadas_rev++;               /* ... e volta */
            (void)z;
        }
        printf("  §P4  chamadas a' involucao:  protocolo %ld,  reversao %ld"
               "  (razao %ld)\n\n", chamadas_prot, chamadas_rev, chamadas_rev/chamadas_prot);
        ok("e o protocolo custa METADE: uma aplicacao da involucao contra as duas da reversao,"
           " e a razao e' exactamente dois nos pontos todos. Nao e' uma optimizacao — e' que a"
           " reversao pergunta 'volta?' e precisa de ir e vir, enquanto o protocolo le' os dois"
           " lados de uma vez. Resolver e tirar a prova na mesma passagem",
           chamadas_rev == 2*chamadas_prot && chamadas_prot > 0);
    }

    /* ═══ §P5 — o CONTROLO: num impostor o protocolo acusa ═════════════════════════ */
    {
        long impares = 0, nao_reconstroi = 0, casos = 0;
        for(long c = 1; c <= 30; c++)
            for(long x = -N; x <= N; x++){
                long d = nao_dag(x, c);                /* uma translacao, que nao e' involucao */
                if((x + d) % 2 || (x - d) % 2) impares++;
                long S = (x + d) / 2, A = (x - d) / 2;
                if(S + A != x) nao_reconstroi++;
                casos++;
            }
        printf("  §P5  num impostor (translacao): somas impares %ld, falhas a reconstruir %ld,"
               " em %ld\n\n", impares, nao_reconstroi, casos);
        ok("e o CONTROLO diz que o protocolo mede mesmo: aplicado a um impostor — uma translacao,"
           " que nao e' involucao — as somas deixam de ser sempre pares e a reconstrucao falha."
           " Logo o zero de §P1 nao veio da aritmetica: veio de o objecto ser involutivo. Se"
           " qualquer operacao passasse, o protocolo nao estaria a verificar nada",
           impares > 0 && nao_reconstroi > 0 && casos > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  INVERTER O SINAL E DIVIDIR POR DOIS — E FECHA:");
        puts("");
        puts("    S = (x + x^dag)/2    o que NAO se moveu     — a cruz");
        puts("    A = (x - x^dag)/2    o objecto, a meio      — a estaca, com o sinal invertido");
        puts("    S + A = x            residuo 0, UMA passagem");
        puts("");
        puts("  Cortar ao meio nao quebra nada porque as duas metades RECONSTROEM — o que se");
        puts("  perde e' ficar com uma so'. E a divisao e' exacta porque a involucao garante a");
        puts("  paridade: num objecto que nao reverte, ela falha, e o protocolo acusa.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
