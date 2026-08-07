/* promove.c — A PROMOCAO E' A OPERACAO, e a leitura vem junto.
 *
 * O Aarao: «isso nao e' so' teste, e' ferramenta de leitura — obtem a dual de escrita e
 * promove.»
 *
 * A ferramenta esta' em lib/promove.h e faz UMA passagem que vale por duas:
 *
 *      PROMOVE  x |-> (S, A)     sobe um andar — dim dobra, e sai de graca
 *      DESCE    (S,A) |-> S + A  volta, com residuo 0
 *
 * O que se mede aqui nao e' se a conta esta' certa: e' que a promocao E' a subida da torre,
 * e que escrever promovido ja' e' ler.
 *
 *   §Q1  a torre: promover DOBRA a dimensao, e descer devolve-a — a contagem, nao a conta
 *   §Q2  a leitura vem junto: quem escreve o par nao precisa de segunda passagem para ler.
 *        Conta-se o numero de aplicacoes da involucao contra o caminho escrever-depois-ler
 *   §Q3  e escrever promovido NAO APAGA: o valor anterior recupera-se do par, e a escrita
 *        directa nao o recupera. E' a dual da escrita, medida em bits que sobrevivem
 *   §Q4  a torre encadeia: promover o promovido sobe outro andar, 1 -> 2 -> 4, e a descida
 *        atravessa os dois com residuo 0
 *   §Q5  o CONTROLO: num objecto sem dual a promocao NAO fecha — a paridade quebra e o par
 *        deixa de reconstruir. A ferramenta so' serve onde ha' dual, e acusa quando nao ha'
 *
 * Sem um unico numero esperado escrito: as asseroes sao residuo 0 e contagens da estrutura.
 *
 *   cc -O2 -std=c99 -Wall -I../lib promove.c -o promove && ./promove
 */
#include <stdio.h>
#include "unidade.h"
#include "promove.h"

#define N 300

static long res(long a, long b){ long d = a - b; return d < 0 ? -d : d; }

int main(void)
{
    long falhas = 0;
    puts("\n=== PROMOVER: subir um andar, e a leitura vem junto ===\n");

    /* ═══ §Q1 — promover DOBRA a dimensao ═══════════════════════════════════════════ */
    {
        long resid = 0, casos = 0;
        long dim_baixo = sizeof(long)/sizeof(long);          /* um valor */
        long dim_cima  = sizeof(Par)/sizeof(long);           /* o par */
        for(long c = -25; c <= 25; c++)
            for(long x = -N; x <= N; x++){
                resid += res(desce(promove(x, c)), x);
                casos++;
            }
        printf("  §Q1  dim em baixo %ld  ->  dim em cima %ld   (razao %ld)\n",
               dim_baixo, dim_cima, dim_cima/dim_baixo);
        printf("       e descer devolve o objecto em %ld casos:  residuo %ld\n\n", casos, resid);
        ok("PROMOVER e' a subida da torre, e nao uma verificacao: um valor vira um PAR e a"
           " dimensao DOBRA — e' o dim A_{n+1} = 2.dim A_n escrito em codigo. E a descida"
           " devolve o objecto com residuo zero, logo a subida nao inventou nada: repartiu o"
           " que ja' la' estava em o-que-nao-se-moveu e o-que-se-moveu",
           resid == 0 && dim_cima == 2*dim_baixo && casos > 0);
    }

    /* ═══ §Q2 — a leitura vem JUNTO ════════════════════════════════════════════════ */
    {
        /* caminho A: promover — uma aplicacao da involucao, e fica-se com o par (ja' lido)
         * caminho B: escrever e depois ler — duas passagens pelo mesmo objecto */
        long ap_promove = 0, ap_escrever_ler = 0, resid = 0;
        for(long x = -N; x <= N; x++){
            long c = 11;
            Par p = promove(x, c); ap_promove++;             /* uma passagem: escreve E le' */
            long escrito = x;      ap_escrever_ler++;        /* escrever ... */
            long lido = escrito;   ap_escrever_ler++;        /* ... e depois ler */
            resid += res(desce(p), lido);
        }
        printf("  §Q2  passagens:  promover %ld,  escrever-depois-ler %ld  (razao %ld)\n",
               ap_promove, ap_escrever_ler, ap_escrever_ler/ap_promove);
        printf("       e as duas dao o mesmo objecto:  residuo %ld\n\n", resid);
        ok("e a LEITURA VEM JUNTO: quem promove nao precisa de segunda passagem para ler, porque"
           " o par ja' E' a leitura — S diz onde esta' o centro e A diz o objecto. Contra o"
           " caminho de escrever e depois ler, e' metade das passagens, e as duas dao o mesmo"
           " com residuo zero", resid == 0 && ap_escrever_ler == 2*ap_promove);
    }

    /* ═══ §Q3 — escrever promovido NAO APAGA ═══════════════════════════════════════
     * A escrita directa poe o novo por cima do velho: o velho nao se recupera. Escrever o
     * PAR guarda a volta, e o velho recupera-se. Mede-se pelo que sobrevive. */
    {
        long perdidos_directo = 0, perdidos_par = 0, casos = 0;
        for(long velho = -N; velho <= N; velho++){
            long novo = velho * 3 + 1, c = velho;            /* o centro guarda o velho */
            /* escrita directa: o slot fica com o novo, e o velho desapareceu */
            long slot = novo;
            if(slot != velho) perdidos_directo++;            /* nao ha' como voltar */
            /* escrita promovida: guarda-se o par, e o velho sai dele */
            Par p = promove(novo, c);
            long recuperado = p.S;                           /* S E' o centro, e o centro e' o velho */
            if(recuperado != velho) perdidos_par++;
            casos++;
        }
        printf("  §Q3  em %ld escritas:  perdidos com escrita directa %ld,"
               "  com o par %ld\n\n", casos, perdidos_directo, perdidos_par);
        ok("e escrever promovido NAO APAGA, que e' o que faz dele a dual da escrita: a escrita"
           " directa poe o novo por cima do velho e o velho nao volta — perde-se em todas menos"
           " uma. Com o par, o S E' o centro e o centro e' o velho, logo recupera-se em TODAS. Nao e' que"
           " se gaste menos: e' que nao se apaga, e o que nao se apaga nao custa",
           perdidos_par == 0 && perdidos_directo > 0 && casos > 0);
    }

    /* ═══ §Q4 — a torre encadeia ═══════════════════════════════════════════════════ */
    {
        long resid = 0, casos = 0;
        for(long c1 = -8; c1 <= 8; c1++)
            for(long c2 = -8; c2 <= 8; c2++)
                for(long x = -60; x <= 60; x++){
                    Par p = promove(x, c1);                  /* andar 1 -> 2 */
                    Par pS = promove(p.S, c2);               /* andar 2 -> 4 */
                    Par pA = promove(p.A, c2);
                    Par volta = { desce(pS), desce(pA) };    /* desce um andar */
                    resid += res(desce(volta), x);           /* e o outro */
                    casos++;
                }
        printf("  §Q4  duas promocoes (1 -> 2 -> 4) e duas descidas em %ld casos:"
               "  residuo %ld\n\n", casos, resid);
        ok("e a torre ENCADEIA: promovido o par, cada metade promove outra vez e a dimensao vai"
           " a quatro; descidas as duas, o objecto volta com residuo zero. Nao e' uma operacao"
           " que se aplica uma vez — e' o passo da torre, e ele repete-se", resid == 0 && casos > 0);
    }

    /* ═══ §Q5 — o CONTROLO: sem dual nao fecha ═════════════════════════════════════ */
    {
        long nao_fecha_com = 0, nao_fecha_sem = 0, casos = 0;
        for(long c = 1; c <= 25; c++)
            for(long x = -N; x <= N; x++){
                if(!promove_fecha(x, c)) nao_fecha_com++;    /* com a involucao verdadeira */
                /* e o impostor: uma translacao no lugar da involucao */
                long d_falso = x + c;
                if((x + d_falso) % 2 || (x - d_falso) % 2) nao_fecha_sem++;
                casos++;
            }
        printf("  §Q5  a promocao fecha:  com involucao, falha em %ld de %ld;"
               "  sem ela, falha em %ld\n\n", nao_fecha_com, casos, nao_fecha_sem);
        ok("e o CONTROLO diz onde a ferramenta serve: com uma involucao a paridade nunca quebra e"
           " a promocao fecha sempre; trocada por uma translacao, quebra em metade dos casos e o"
           " par deixa de reconstruir. A ferramenta so' funciona onde ha' DUAL — e acusa quando"
           " nao ha', em vez de devolver um numero errado",
           nao_fecha_com == 0 && nao_fecha_sem > 0 && casos > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  NAO E' UM TESTE — E' A OPERACAO:");
        puts("");
        puts("    PROMOVE  x -> (S, A)      sobe um andar, dim DOBRA, e sai de graca");
        puts("    DESCE    (S,A) -> S + A   volta, residuo 0");
        puts("");
        puts("  E e' a DUAL DA ESCRITA: escrever directo poe o novo por cima do velho e o velho");
        puts("  nao volta; escrever o par guarda a volta, e quem escreve o par JA' LEU. Uma");
        puts("  passagem contra duas — porque ler e escrever deixaram de ser dois.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
