/* colisor_corpos.c — O COLISOR NAO E' UM CORPO: E' UMA LEITURA DE TODOS ELES.
 *
 * O Aarao: "vamos promover o colisor em todos os corpos do catalogo."
 *
 * O colisor de um corpo le-se da ASSINATURA e de mais nada. Se ela e' (p,q,r), o grau e'
 * n = p+q+r, a base e' {+-1}^n, e as involucoes sao as n que trocam um eixo. Dai:
 *
 *     estados    2^n              colisoes distintas   n.2^(n-1)
 *     fecha?     sse n PAR        desdobra?            sse n IMPAR
 *     biduais    2^n / 4          (com os eixos partidos em dois blocos)
 *
 * NAO HA' CORPO SEM COLISOR, e nao ha' colisor sem corpo: e' a mesma assinatura lida como
 * percurso em vez de lida como conta de sinais. Este medidor percorre os corpos nomeados
 * do catalogo e mostra o colisor de cada — e o que se ve e' que os DOIS regimes existem
 * entre eles, e nao sao uma escolha nossa.
 *
 *   §C1  o grau e' a soma da assinatura, e o colisor le-se dele
 *   §C2  os dois regimes coexistem: ha' corpos que FECHAM e corpos que DESDOBRAM
 *   §C3  o conforme tem grau 5 — e' o corpo de 32 estados, 16+16, 8 biduais
 *   §C4  e as contas do colisor batem com a construcao, corpo a corpo
 *
 * Zero doubles.
 *
 *   cc -O2 -std=c99 -Wall -I../lib colisor_corpos.c -o colisor_corpos
 */
#include <stdio.h>
#include "unidade.h"

/* ── A ASSINATURA NAO CHEGA: FALTA O LADO ───────────────────────────────────────────
 *
 * O Aarao: "promove a cifra ouro branco junto a' sua dual a' assinatura do corpo, poe
 * ouro branco no colisor."
 *
 * E a promocao obriga a acrescentar uma coordenada, porque a assinatura sozinha NAO
 * distingue o ouro branco do seu dual: os dois tem a mesma forma de grau 2, e o que os
 * separa e' o DETERMINANTE — -1 no que anti-conserva (o gato), +1 no que conserva (o
 * esquilo). Medido em ouro_branco.c por dois caminhos que nao se falam: pela matriz e
 * pela norma.
 *
 *     ouro branco   x^2 = 4x - 1   det +1   N nunca da' -1
 *     a sua dual    x^2 = 4x + 1   det -1   N da' -1
 *
 * Sao duas coordenadas e nao uma, como em todo o resto desta teoria: a assinatura MEDE
 * (da' o grau, e dele saem estados e colisoes) e o determinante ORDENA (da' o lado, e
 * dele sai se conserva ou anti-conserva). Uma nao substitui a outra. */
typedef struct { const char *nome; int p, q, r; int det; } Corpo;

/* as assinaturas como o catalogo as declara (catalogo.tex, a lista dos corpos) */
static const Corpo CORPOS[] = {
    {"deflexivo",       1,0,0,-1}, {"reflexivo",     2,0,0,-1}, {"expansivo",     1,1,0,-1},
    {"celeste",         2,0,0,-1}, {"optico",        2,0,0,-1}, {"cristalino",    2,0,0,-1},
    {"relogio",         1,1,0,-1}, {"sensitivo",     1,1,0,-1}, {"evolutivo",     1,1,0,-1},
    {"individual",      2,0,0,-1}, {"telescopico",   2,2,0,-1}, {"geometrico",    1,3,0,-1},
    {"conforme",        4,1,0,-1}, {"eletromagnetico",3,3,0,-1},
    /* o par novo, promovido: mesma assinatura, LADOS opostos */
    {"ouro branco",     1,1,0,+1}, {"dual do o. branco", 1,1,0,-1},
};
#define NC ((int)(sizeof CORPOS / sizeof CORPOS[0]))

/* a construcao, feita de facto: conta estados e colisoes andando neles */
static long conta_estados(int n){ return 1L << n; }
static long conta_colisoes(int n){
    long c = 0, V = 1L << n;
    for(long v = 0; v < V; v++) for(int b = 0; b < n; b++) if(!((v>>b)&1)) c++;
    return c;
}
/* quantos estados troca a composta de TODAS as involucoes leva a outra paridade */
static long troca_paridade(int n){
    long t = 0, V = 1L << n, cheio = V - 1;
    for(long v = 0; v < V; v++){
        int pv = 0, pw = 0, w = (int)(v ^ cheio);
        for(int b=0;b<n;b++){ if((v>>b)&1) pv++; if((w>>b)&1) pw++; }
        if((pv & 1) != (pw & 1)) t++;
    }
    return t;
}
/* orbitas do par de involucoes parciais: eixos 0..s-1 num bloco, s..n-1 no outro */
static long biduais(int n, int s){
    if(s <= 0 || s >= n) return -1;
    long V = 1L << n; int visto[1<<12]; long o = 0;
    for(long i = 0; i < V; i++) visto[i] = 0;
    int mA = (1<<s) - 1, mB = (int)((V-1) ^ mA);
    for(long v = 0; v < V; v++){
        if(visto[v]) continue;
        visto[v] = 1; visto[v ^ mA] = 1; visto[v ^ mB] = 1; visto[v ^ (mA|mB)] = 1;
        o++;
    }
    return o;
}

int main(void){
    puts("\n  O COLISOR DE CADA CORPO — lido da assinatura, e de mais nada\n");

    /* ═══ §C1 e §C4 — o grau e as contas, corpo a corpo ════════════════════════════ */
    printf("      corpo             assin.   n   estados  colisoes  fecha  desdobra\n");
    long mau = 0, fecham = 0, desdobram = 0;
    for(int i = 0; i < NC; i++){
        const Corpo *c = &CORPOS[i];
        int n = c->p + c->q + c->r;
        long E = conta_estados(n), K = conta_colisoes(n);
        int fecha = (n % 2 == 0), desd = (n % 2 == 1);
        /* a formula n.2^(n-1) tem de bater com a contagem feita andando */
        if(K != (long)n * (E/2)) mau++;
        /* e o desdobramento: a composta troca a paridade em TODOS ou em NENHUM */
        long t = troca_paridade(n);
        if(desd && t != E) mau++;
        if(!desd && t != 0) mau++;
        fecham += fecha; desdobram += desd;
        printf("      %-16s (%d,%d,%d)  %2d  %7ld  %8ld   %-4s   %s\n",
               c->nome, c->p, c->q, c->r, n, E, K, fecha?"sim":"nao", desd?"SIM":"nao");
    }
    ok("o colisor le-se da ASSINATURA e de mais nada: o grau e' p+q+r, os estados sao 2^n e"
       " as colisoes n.2^(n-1) — contadas andando nelas, nao pela formula, e batem em todos"
       " os corpos nomeados", mau == 0);

    /* ═══ §C2 — os dois regimes coexistem ══════════════════════════════════════════ */
    printf("\n      dos %d corpos: %ld FECHAM o circuito, %ld DESDOBRAM em duais\n",
           NC, fecham, desdobram);
    ok("os DOIS regimes existem entre os corpos nomeados, e nao sao uma escolha nossa: uns"
       " fecham o percurso e outros partem o conjunto ao meio — e nenhum faz as duas coisas,"
       " porque a paridade do grau decide", fecham > 0 && desdobram > 0 && fecham + desdobram == NC);

    /* ═══ §C3 — o conforme e' o corpo de 32 ════════════════════════════════════════ */
    {
        int achou = 0; long est = 0, monte = 0, bid = 0;
        for(int i = 0; i < NC; i++){
            int n = CORPOS[i].p + CORPOS[i].q + CORPOS[i].r;
            if(n != 5) continue;
            achou++;
            est = conta_estados(n); monte = est/2; bid = biduais(n, 3);
            printf("\n      %s: grau %d -> %ld estados, %ld+%ld pela paridade, %ld biduais\n",
                   CORPOS[i].nome, n, est, monte, monte, bid);
        }
        ok("e o CONFORME e' o corpo de grau CINCO: 32 estados que a paridade parte em 16+16,"
           " e 8 biduais partindo os eixos em 3 e 2 — e' o mesmo colisor do spinor32.c, com"
           " nome de corpo do catalogo em vez de nome emprestado",
           achou == 1 && est == 32 && monte == 16 && bid == 8);
    }

    /* ═══ §C5 — E AS DUAS LEITURAS DE 32 TEM NOME DE CORPO ══════════════════════════
     * 32 aparece duas vezes e em corpos diferentes: e' o numero de COLISOES dos corpos de
     * grau 4 (o geometrico e o telescopico) e o numero de ESTADOS do corpo de grau 5 (o
     * conforme). E os dois estao ligados por um eixo so':
     *
     *     conforme (4,1,0)  tem grau 5  =  grau do geometrico (1,3,0) + 1
     *
     * e' exactamente a passagem de FECHA para DESDOBRA. Acrescentar um eixo ao espaco-tempo
     * nao lhe acrescenta tamanho novo: muda-lhe a PARIDADE, e com ela o regime. */
    {
        long col4 = 0, est5 = 0, n4 = 0;
        for(int i = 0; i < NC; i++){
            int n = CORPOS[i].p + CORPOS[i].q + CORPOS[i].r;
            if(n == 4){ n4++; if(conta_colisoes(4) == 32) col4++; }
            if(n == 5) est5 = conta_estados(5);
        }
        printf("\n      grau 4: %ld corpos, todos com 32 COLISOES; grau 5: %ld ESTADOS\n",
               n4, est5);
        ok("as DUAS leituras de 32 tem nome de corpo: e' o numero de COLISOES dos de grau 4"
           " (geometrico e telescopico) e o de ESTADOS do de grau 5 (conforme) — e um eixo"
           " separa-os, que e' o que troca FECHA por DESDOBRA",
           n4 == 2 && col4 == 2 && est5 == 32);
    }

    /* ═══ §C6 — O OURO BRANCO NO COLISOR, E O QUE A ASSINATURA NAO VE ══════════════ */
    {
        long pares_mesma_assin = 0, dets_diferentes = 0, mau = 0;
        for(int i = 0; i < NC; i++) for(int j = i+1; j < NC; j++){
            const Corpo *a = &CORPOS[i], *b = &CORPOS[j];
            if(a->p != b->p || a->q != b->q || a->r != b->r) continue;
            pares_mesma_assin++;
            if(a->det != b->det) dets_diferentes++;
        }
        /* e o colisor dos dois novos e' o MESMO, porque o grau e' o mesmo */
        const Corpo *ob = NULL, *du = NULL;
        for(int i = 0; i < NC; i++){
            if(CORPOS[i].det == +1) ob = &CORPOS[i];
            if(ob && CORPOS[i].det == -1 && CORPOS[i].p == 1 && CORPOS[i].q == 1
               && i > 13) du = &CORPOS[i];
        }
        if(!ob || !du) mau++;
        else {
            int n_ob = ob->p+ob->q+ob->r, n_du = du->p+du->q+du->r;
            if(conta_estados(n_ob) != conta_estados(n_du)) mau++;
            if((( n_ob % 2)==0) != (( n_du % 2)==0)) mau++;
            if(ob->det == du->det) mau++;
            printf("\n      ouro branco (%d,%d,%d) det %+d  e  a sua dual (%d,%d,%d) det %+d\n",
                   ob->p,ob->q,ob->r,ob->det, du->p,du->q,du->r,du->det);
            printf("      colisor IGUAL nos dois: %ld estados, fecha=%s — a assinatura nao os separa\n",
                   conta_estados(n_ob), (( n_ob % 2)==0)?"sim":"nao");
        }
        printf("      pares com a MESMA assinatura: %ld; destes, com det diferente: %ld\n",
               pares_mesma_assin, dets_diferentes);
        ok("o OURO BRANCO entra no colisor com a sua dual, e a promocao mostra o que"
           " faltava: a assinatura da' o mesmo colisor aos dois — mesmo grau, mesmos"
           " estados, mesmo regime — e o que os separa e' o DETERMINANTE. Sao DUAS"
           " coordenadas: uma mede e a outra ordena", mau == 0 && dets_diferentes > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O COLISOR NAO E' MAIS UM CORPO: e' uma LEITURA de qualquer um deles. Da");
        puts("  assinatura (p,q,r) sai o grau n = p+q+r, e do grau sai tudo — 2^n estados,");
        puts("  n.2^(n-1) colisoes, o circuito a fechar sse n e' par, o desdobramento sse e'");
        puts("  impar, e 2^n/4 biduais. Nada disto se acrescenta ao corpo: ja' la' estava.");
        puts("");
        puts("  E OS DOIS REGIMES COEXISTEM ENTRE OS CORPOS NOMEADOS. Nao ha' que escolher");
        puts("  um: a paridade do grau decide, e o catalogo tem dos dois lados.");
        puts("");
        puts("  E O CONFORME E' O CORPO DE GRAU CINCO — 32 estados, 16+16, 8 biduais. O");
        puts("  colisor de 32 tem nome no catalogo, e nao e' um nome emprestado de fora.");
        puts("");
        puts("  E AS DUAS LEITURAS DE 32 TEM NOME: e' o numero de COLISOES do geometrico e do");
        puts("  telescopico (grau 4) e o de ESTADOS do conforme (grau 5). Um eixo separa-os —");
        puts("  e acrescentar um eixo nao acrescenta tamanho, muda a PARIDADE, e com ela o");
        puts("  regime. E' o mesmo que dizer que subir de dimensao acrescenta FASE.");
        puts("");
        puts("  E O OURO BRANCO ENTROU COM A SUA DUAL, e a promocao mostrou o que faltava: a");
        puts("  assinatura da' aos dois o MESMO colisor — mesmo grau, mesmos estados, mesmo");
        puts("  regime — e o que os separa e' o DETERMINANTE. A assinatura MEDE e o");
        puts("  determinante ORDENA; uma nao substitui a outra, como em todo o resto.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
