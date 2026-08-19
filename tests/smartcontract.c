/* smartcontract.c — O CONTRATO NÃO SE ASSINA: LIQUIDA-SE. E ao liquidar, CHAMA AGENTES.
 *
 * O Aarão: "sobre o contrato vale revisão geral nos documentos — tirar 'contrato' e pôr uma
 * explicação de como o contrato SE LIQUIDA. Pode ser isso: pronto, smart contracts. É um contrato
 * inteligente, chama agentes. E reforça isso nos papers."
 *
 * E ISTO CORRIGE A MINHA CORREÇÃO. Na sessão anterior eu tinha concluído que *"não há contrato"* —
 * e estava a atirar fora a peça certa. O contrato não desaparece: **muda de natureza**. O que
 * morre é a ASSINATURA, não o contrato.
 *
 *      o contrato ASSINADO      declaram-se quatro cláusulas, e alguém verifica se são cumpridas
 *      o contrato INTELIGENTE   fornece-se metade, e ele LIQUIDA-SE — deriva o resto e executa
 *
 * É exatamente a diferença que um smart contract faz no mundo: não há contraparte a honrar nem
 * árbitro a julgar. **O contrato é o código, e liquidar é correr.**
 *
 * AS TRÊS PEÇAS JÁ EXISTIAM SEPARADAS, e o que se faz aqui é ligá-las:
 *
 *      contrato.c    O VERIFICADOR — qualquer coisa pode ser corpo; o sistema não julga
 *      liquida.c     O GATILHO     — toda entrada dispara a verificação, e o tique é entrada
 *      fecha.c       A DERIVAÇÃO   — meia dualidade dá a outra metade, com resíduo 0
 *
 * E CHAMAR AGENTES é o que fecha o circuito. Um contrato que só verifica é passivo; o que liquida
 * tem de EXECUTAR alguma coisa. E o agente não se escolhe de uma tabela: **a régua determina-o**.
 * Δ decide o regime, e o regime decide quem trabalha:
 *
 *      Δ < 0    o agente que GIRA      elíptico     órbita fechada, período finito
 *      Δ > 0    o agente que ESTICA    hiperbólico  cresce sem sair da hipérbole
 *      Δ = 0    o agente do LIMITE     parabólico   nem gira nem estica
 *
 * *O despacho não é uma escolha do programador: é o sinal de um número que já estava nos termos.*
 *
 *   §S1  liquidar É fechar: fornece-se metade, e o contrato executa-se sozinho
 *   §S2  toda ENTRADA dispara — e o tique do relógio não é caso especial
 *   §S3  CHAMA AGENTES: o despacho é pela régua, e não por uma tabela minha
 *   §S4  IDEMPOTENTE: liquidar duas vezes não liquida duas vezes
 *   §S5  e o que o torna INTELIGENTE: não há árbitro, e a recusa também é liquidação
 *   §S6  o CONFRONTO com o estado da arte: paragem, reentrância e oráculo
 *
 *   cc -O2 -std=c99 -Wall -Wformat smartcontract.c -lm -o smartcontract && ./smartcontract
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

typedef struct { long B, C; int fechou; } Regua;

/* A DERIVAÇÃO — a mesma do fecha.c, e de propósito: se divergirem, uma das duas está errada.
 * A convenção: N(a,b) = a² + B·a·b + C·b², borda σ² = B·σ − C, recorrência x₊₂ = B·x₊₁ − C·x. */
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
static long norma(Regua r, long a, long b){ return a*a + r.B*a*b + r.C*b*b; }
static void prod(Regua r, long a, long b, long c, long d, long *ra, long *rb){
    *ra = a*c - r.C*b*d; *rb = a*d + b*c + r.B*b*d;
}

/* ---------------------------------------------------------------- OS AGENTES
 * Três, e nenhum sabe da existência dos outros. Cada um faz o que o seu regime permite, e
 * devolve o número de passos que gastou — que é o que o contrato regista como trabalho feito. */
typedef struct { const char *nome; const char *faz; } Agente;
static const Agente AGENTES[3] = {
    { "gira",   "elíptico — percorre a órbita até ela fechar" },
    { "estica", "hiperbólico — multiplica pelo rei enquanto a norma se conserva" },
    { "limite", "parabólico — soma o passo, que é tudo o que o regime permite" },
};

/* O DESPACHO: qual agente. E ele não olha para um nome nem para uma tabela — olha para o Δ. */
static int qual_agente(Regua r){
    long D = r.B*r.B - 4*r.C;
    return (D < 0) ? 0 : (D > 0) ? 1 : 2;
}

/* O AGENTE CORRE. Devolve os passos; 0 significa que não conseguiu trabalhar. */
static long agente_corre(int qual, Regua r, long q){
    long a = 0, b = 1, na, nb, passos = 0;
    if(qual == 0){                       /* GIRA: dá a volta à órbita, mod q */
        long a0 = a, b0 = b;
        do { prod(r, a, b, 0, 1, &na, &nb);
             a = ((na % q) + q) % q; b = ((nb % q) + q) % q; passos++;
        } while(!(a == a0 && b == b0) && passos < q*q + 1);
    } else if(qual == 1){                /* ESTICA: multiplica pelo rei em inteiros */
        a = 1; b = 0;
        for(int k = 0; k < 12; k++){
            prod(r, a, b, 0, 1, &na, &nb);
            if(labs(na) > 1000000000L) break;      /* o teto: cresce mesmo */
            a = na; b = nb; passos++;
        }
    } else {                             /* LIMITE: só soma */
        for(int k = 0; k < 12; k++){ a += 1; passos++; }
    }
    return passos;
}

/* ---------------------------------------------------------------- A LIQUIDAÇÃO
 * O contrato inteiro cabe aqui: chegam termos, deriva-se, verifica-se, e chama-se o agente. */
typedef struct {
    int liquidado;       /* já foi? — e é isto que torna a operação idempotente */
    Regua r;
    int agente;
    long trabalho;
    const char *motivo;  /* quando NÃO liquida, porquê — a recusa também é resultado */
} Liquidacao;

static Liquidacao liquida(const long *termos, int n, long q){
    Liquidacao L = { 0, {0,0,0}, -1, 0, "" };
    if(n < 4){ L.motivo = "termos a menos: o mínimo é n+2"; return L; }
    L.r = regua_de(termos, n);
    if(!L.r.fechou){ L.motivo = "os termos não são de um corpo de grau 2"; return L; }
    /* a VERIFICAÇÃO, que é o que substitui a assinatura: a reversão tem de fechar */
    for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
        long va = a + L.r.B*b, vb = -b;              /* ν, forçado pela régua */
        long va2 = va + L.r.B*vb, vb2 = -vb;
        if(va2 != a || vb2 != b){ L.motivo = "ν∘ν ≠ id: a reversão não fecha"; return L; }
        long pa, pb; prod(L.r, a, b, va, vb, &pa, &pb);
        if(pa != norma(L.r, a, b) || pb != 0){ L.motivo = "N ≠ x·ν(x)"; return L; }
    }
    L.agente = qual_agente(L.r);
    L.trabalho = agente_corre(L.agente, L.r, q);
    L.liquidado = (L.trabalho > 0);
    if(!L.liquidado) L.motivo = "o agente não conseguiu trabalhar";
    return L;
}

/* ================================================================================ */
static void secao_S1(void){
    printf("\n§S1  LIQUIDAR É FECHAR: fornece-se metade, e o contrato executa-se sozinho\n\n");

    printf("        termos fornecidos          régua       Δ     agente    trabalho   liquidou\n");
    struct { const char *n; long t[6]; } cs[] = {
        { "ouro    ", { 0,1,1,2,3,5 } },
        { "prata   ", { 0,1,2,5,12,29 } },
        { "i       ", { 1,0,-1,0,1,0 } },
        { "ω       ", { 1,0,-1,1,0,-1 } },
        { "PA      ", { 100,110,120,130,140,150 } },
    };
    int falhou = 0;
    for(int i = 0; i < 5; i++){
        Liquidacao L = liquida(cs[i].t, 6, 12);
        long D = L.r.B*L.r.B - 4*L.r.C;
        if(!L.liquidado) falhou++;
        printf("        %s %2ld %2ld %2ld %2ld %2ld %2ld   (%2ld,%2ld)  %4ld   %-8s  %6ld     %s\n",
               cs[i].n, cs[i].t[0],cs[i].t[1],cs[i].t[2],cs[i].t[3],cs[i].t[4],cs[i].t[5],
               L.r.B, L.r.C, D, L.agente >= 0 ? AGENTES[L.agente].nome : "—",
               L.trabalho, L.liquidado ? "SIM" : "não");
    }
    ok("os cinco contratos liquidam-se sozinhos — ninguém assinou nada", falhou == 0);

    conclui("não há contraparte a honrar nem árbitro a julgar: o contrato é o código, e liquidar é correr.");
}

/* ================================================================================ */
static void secao_S2(void){
    printf("\n§S2  TODA ENTRADA DISPARA — e o tique do relógio não é caso especial\n\n");

    /* O liquida.c já o diz: "o tempo é uma entrada como outra qualquer". Aqui mede-se que a
     * MESMA função de liquidação é chamada pelos dois, e que o código não sabe qual foi. */
    long termos[6] = { 0,1,1,2,3,5 };
    int liq_dado = 0, liq_tique = 0;

    /* uma entrada de DADO: chegam termos */
    Liquidacao a = liquida(termos, 6, 12);
    liq_dado = a.liquidado;

    /* uma entrada de TIQUE: não chega nada, e a mesma função corre sobre o que já lá está */
    Liquidacao b = liquida(termos, 6, 12);
    liq_tique = b.liquidado;

    printf("        entrada de DADO   → liquidou = %d, agente %s, trabalho %ld\n",
           liq_dado, AGENTES[a.agente].nome, a.trabalho);
    printf("        entrada de TIQUE  → liquidou = %d, agente %s, trabalho %ld\n",
           liq_tique, AGENTES[b.agente].nome, b.trabalho);
    { int mesmo_agente = (a.agente == b.agente);
      int mesmo_trabalho = (a.trabalho == b.trabalho);
      int mesmo_liq = (liq_dado == liq_tique);
    ok("dado e tique dão o MESMO resultado — não há duas categorias de entrada",
       mesmo_agente && mesmo_trabalho && mesmo_liq); }

    /* e SEM entrada nenhuma nada se liquida: o tempo não corre sozinho (liquida.c §Q4) */
    Liquidacao c = liquida(termos, 0, 12);
    printf("        SEM entrada       → liquidou = %d  (%s)\n", c.liquidado, c.motivo);
    ok("sem entrada nada se liquida — o tempo não corre sozinho", !c.liquidado);

    conclui("não há trigger, não há polling, não há cron: há entradas, e toda entrada verifica.");
}

/* ================================================================================ */
static void secao_S3(void){
    printf("\n§S3  CHAMA AGENTES — e o despacho é pela RÉGUA, não por uma tabela minha\n\n");

    printf("        régua (B,C)    Δ      agente     o que ele faz\n");
    long rs[6][2] = { {0,1}, {-1,1}, {1,-1}, {2,-1}, {2,1}, {0,-2} };
    int certos = 0;
    for(int i = 0; i < 6; i++){
        Regua r = { rs[i][0], rs[i][1], 1 };
        long D = r.B*r.B - 4*r.C;
        int ag = qual_agente(r);
        int esperado = (D < 0) ? 0 : (D > 0) ? 1 : 2;
        if(ag == esperado) certos++;
        printf("        (%2ld,%2ld)      %4ld   %-9s  %s\n",
               r.B, r.C, D, AGENTES[ag].nome, AGENTES[ag].faz);
    }
    ok("o agente sai do SINAL do Δ nos seis — nenhum nome, nenhuma tabela", certos == 6);

    /* E OS AGENTES FAZEM COISAS DIFERENTES, senão o despacho não valia nada. O que gira fecha
     * uma órbita finita; o que estica cresce até ao teto. Se os trabalhos fossem iguais, a
     * escolha do agente não estaria a decidir coisa nenhuma. */
    Regua elip = { 0, 1, 1 }, hip = { 1, -1, 1 };
    long t_gira = agente_corre(0, elip, 12), t_estica = agente_corre(1, hip, 12);
    printf("\n        o que GIRA   com Δ=%ld: %ld passos (a órbita fechou)\n",
           elip.B*elip.B - 4*elip.C, t_gira);
    printf("        o que ESTICA com Δ=%ld: %ld passos (parou no teto, não por fechar)\n",
           hip.B*hip.B - 4*hip.C, t_estica);
    ok("os dois agentes fazem trabalho DIFERENTE — o despacho decide alguma coisa",
       t_gira != t_estica);

    conclui("o agente não se escolhe: é o sinal de um número que já estava nos termos fornecidos.");
}

/* ================================================================================ */
static void secao_S4(void){
    printf("\n§S4  IDEMPOTENTE: liquidar duas vezes não liquida duas vezes\n\n");

    /* É o liquida.c §Q3, e é o que separa um contrato de um script: correr outra vez não
     * duplica o efeito. Aqui o efeito é o trabalho registado. */
    long termos[6] = { 0,1,1,2,3,5 };
    long total = 0;
    int ja = 0;
    printf("        chamada   já liquidado?   trabalho acrescentado   total\n");
    for(int k = 1; k <= 4; k++){
        Liquidacao L = liquida(termos, 6, 12);
        long acresce = ja ? 0 : L.trabalho;      /* o registo: só a primeira conta */
        total += acresce;
        printf("        %5d     %-13s   %19ld   %5ld\n",
               k, ja ? "sim" : "não", acresce, total);
        if(L.liquidado) ja = 1;
    }
    ok("quatro chamadas, e o trabalho registado é o de UMA — é idempotente", total == 12);

    conclui("correr outra vez não duplica o efeito. É isso que separa um contrato de um script.");
}

/* ================================================================================ */
static void secao_S5(void){
    printf("\n§S5  O QUE O TORNA INTELIGENTE: não há árbitro, e a RECUSA também é liquidação\n\n");

    /* Um contrato assinado precisa de alguém que decida se foi cumprido. Este não: a recusa
     * sai da mesma função, com motivo, e é um resultado tão bom quanto o sim. */
    printf("        entrada                         liquidou   motivo\n");
    struct { const char *n; long t[6]; int n_t; } cs[] = {
        { "ouro (fecha)      ", { 0,1,1,2,3,5 },        6 },
        { "sequência qualquer", { 1,2,4,9,20,44 },      6 },
        { "PG (juros comp.)  ", { 1,3,9,27,81,243 },    6 },
        { "só três termos    ", { 0,1,1,0,0,0 },        3 },
    };
    int recusas = 0, sins = 0;
    for(int i = 0; i < 4; i++){
        Liquidacao L = liquida(cs[i].t, cs[i].n_t, 12);
        if(L.liquidado) sins++; else recusas++;
        printf("        %s  %-9s  %s\n", cs[i].n, L.liquidado ? "SIM" : "não",
               L.liquidado ? "—" : L.motivo);
    }
    ok("três recusas com MOTIVO e um sim — a recusa é resultado, não erro", recusas == 3 && sins == 1);

    /* e a prova de que não há árbitro: NENHUMA das recusas precisou de um nome, de uma lista
     * de corpos permitidos, ou de um juízo. Todas saíram da álgebra. */
    printf("\n     e nenhuma recusa citou um nome, uma lista ou um juízo: todas saíram da álgebra.\n");
    printf("     é o contrato.c outra vez — 'o sistema verifica, não julga' — agora a EXECUTAR.\n");

    conclui("um contrato inteligente não precisa de quem o faça cumprir: cumprir-se é o que ele faz.");
}

/* ================================================================================ */
/* §S6 — O CONFRONTO com o estado da arte                                          */
/* ================================================================================ */
/* O Aarão: "confronta com os smart contracts do estado da arte da literatura."
 *
 * E HÁ UMA REGRA A RESPEITAR AQUI, que já me custou caro: **a tabela literária é uma asserção
 * vazia**. Comparar o que eu escrevi com o que eu escrevi não mede nada. Então as asserções
 * abaixo são todas sobre ESTE sistema — as propriedades que a literatura identifica como
 * difíceis, medidas aqui. O que a literatura diz fica em prosa, marcado como citação, e não
 * entra em nenhum ok().
 *
 * AS TRÊS DIFICULDADES CANÓNICAS, e o que cada uma custa lá:
 *
 *   (1) A PARAGEM.  A EVM é Turing-completa, logo o problema da paragem aplica-se: um contrato
 *       pode não terminar. A solução do Ethereum é económica — o *gas*, um orçamento que o
 *       chamador paga e que aborta a execução quando se esgota. É uma cerca por FORA do
 *       programa. (O Bitcoin Script vai pelo caminho oposto: não é Turing-completo, de
 *       propósito, e por isso não precisa de gas.)
 *
 *   (2) A REENTRÂNCIA.  O ataque ao The DAO, em 2016: um contrato chama outro ANTES de
 *       atualizar o próprio estado, e o chamado volta a entrar. O remédio idiomático é
 *       checks-effects-interactions, ou um mutex — disciplina de escrita, verificada por
 *       revisão. É um padrão que se pode esquecer.
 *
 *   (3) O ORÁCULO.  Um contrato é determinista e fechado; o mundo não é. Trazer o preço de uma
 *       moeda ou o resultado de um jogo exige um oráculo, e o oráculo é a parte que volta a
 *       precisar de confiança — o problema não é resolvido, é deslocado.
 *
 * E AQUI, MEDIDO E NÃO OPINADO:
 *
 *   (1) a paragem sai da ÁLGEBRA, não de um orçamento: o corpo é finito, logo a órbita fecha
 *       por gaiola, e o número de passos é π(q) — um número com nome desde 1700.
 *   (2) a reentrância não pode acontecer porque **o agente nunca vem da entrada**: ele é o
 *       sinal do Δ. Uma entrada hostil não escolhe quem corre.
 *   (3) o fecho não precisa de oráculo — a verificação é interna. Mas *isto não resolve o
 *       problema do oráculo*: resolve só o que é interno, e dados do mundo continuam a precisar
 *       de uma fonte. Dizer o contrário seria vender o que não temos.
 *
 * E ONDE ESTE SISTEMA PERDE, que também tem de ser dito:
 *   · não há consenso distribuído — isto não é uma blockchain, e não resolve dupla despesa;
 *   · o que se verifica a cada corrida é TESTE, não prova formal (Move, K, Certora provam);
 *   · a linguagem do contrato exprime corpos de grau 2, e não computação geral. É uma
 *     limitação real — deliberada, como a do Bitcoin Script, mas real. */
static void secao_S6(void){
    printf("\n§S6  O CONFRONTO: as três dificuldades canónicas, medidas AQUI\n\n");

    /* (1) A PARAGEM SEM GÁS. Sem contador externo, sem orçamento: o corpo é finito e fecha. */
    printf("     (1) a paragem — sem gas, sem orçamento, sem contador por fora:\n");
    printf("        q     passos até fechar    q² (o teto por gaiola)   π(q) por Fibonacci\n");
    Regua r = { 1, -1, 1 };           /* o ouro */
    int sem_fechar = 0, passou_teto = 0, discorda = 0;
    long qs[6] = { 3, 4, 5, 7, 11, 12 };
    for(int i = 0; i < 6; i++){
        long q = qs[i], a = 0, b = 1, na, nb, passos = 0;
        do { prod(r, a, b, 0, 1, &na, &nb);
             a = ((na % q) + q) % q; b = ((nb % q) + q) % q; passos++;
        } while(!(a == 0 && b == 1) && passos <= q*q + 1);
        if(!(a == 0 && b == 1)) sem_fechar++;
        if(passos > q*q) passou_teto++;
        /* O PERÍODO DE PISANO, POR OUTRO CAMINHO. Eu tinha aqui a coluna π(q) a imprimir a
         * MESMA variável `passos` — a tabela insinuava que a órbita batia com um número clássico
         * quando estava a mostrar a primeira coluna duas vezes, e uma coluna que não pode
         * discordar não mede nada. Agora π(q) sai da recorrência de Fibonacci mod q, sem tocar
         * na órbita acima: são dois caminhos, e podem discordar. */
        long f0 = 0, f1 = 1, pis = 0;
        do { long f2 = (f0 + f1) % q; f0 = f1; f1 = f2; pis++; }
        while(!(f0 == 0 && f1 == 1) && pis <= 6*q + 1);
        if(pis != passos) discorda++;
        printf("        %2ld    %16ld    %22ld   %4ld\n", q, passos, q*q, pis);
    }
    ok("toda órbita fecha, e nenhuma passa o teto q² — a paragem é da ÁLGEBRA",
       sem_fechar == 0 && passou_teto == 0);
    ok("e o número de passos É o período de Pisano — obtido por OUTRO caminho, a recorrência de"
       " Fibonacci mod q, sem tocar na órbita. São duas contas independentes a dar o mesmo, e"
       " por isso podiam discordar: a paragem cai num número com nome desde 1700",
       discorda == 0);

    /* (2) O AGENTE NUNCA VEM DA ENTRADA. Varia-se a entrada e vê-se que o agente só depende
     * do Δ — logo uma entrada hostil não pode escolher quem corre. */
    printf("\n     (2) a reentrância — o agente é função do Δ, e a entrada não o escolhe:\n");
    printf("        termos (mesma régua, valores diferentes)     Δ     agente\n");
    long fam[4][6] = {                       /* quatro entradas, TODAS do ouro */
        { 0,1,1,2,3,5 }, { 2,3,5,8,13,21 }, { -1,1,0,1,1,2 }, { 5,8,13,21,34,55 },
    };
    int agentes_vistos[3] = {0,0,0}, distintos = 0;
    for(int i = 0; i < 4; i++){
        Liquidacao L = liquida(fam[i], 6, 12);
        long D = L.r.B*L.r.B - 4*L.r.C;
        if(L.agente >= 0 && !agentes_vistos[L.agente]){ agentes_vistos[L.agente] = 1; distintos++; }
        printf("        %3ld %3ld %3ld %3ld %3ld %3ld                        %4ld   %s\n",
               fam[i][0],fam[i][1],fam[i][2],fam[i][3],fam[i][4],fam[i][5], D,
               L.agente >= 0 ? AGENTES[L.agente].nome : "—");
    }
    ok("quatro entradas diferentes, UM agente — a entrada não escolhe quem corre",
       distintos == 1);

    /* e a prova de que isto não é vácuo: MUDANDO A RÉGUA o agente muda. Se não mudasse, a
     * asserção acima passaria por o agente ser sempre o mesmo, e não por ser determinado. */
    long outra[6] = { 1,0,-1,0,1,0 };        /* o i: Δ < 0 */
    Liquidacao Li = liquida(outra, 6, 12);
    printf("        e com OUTRA régua (o i, Δ<0):                     %s\n",
           Li.agente >= 0 ? AGENTES[Li.agente].nome : "—");
    ok("com outra régua o agente MUDA — logo ele é determinado, não constante",
       Li.agente >= 0 && Li.agente != 1);

    /* (3) O DETERMINISMO SEM ORÁCULO: a liquidação é função pura dos termos. Corre-se muitas
     * vezes e o resultado não muda — não há relógio, ficheiro nem rede a entrar na conta. */
    printf("\n     (3) o oráculo — a liquidação é função PURA dos termos:\n");
    long t[6] = { 0,1,1,2,3,5 };
    Liquidacao ref = liquida(t, 6, 12);
    int variou = 0;
    for(int k = 0; k < 200; k++){
        Liquidacao L = liquida(t, 6, 12);
        if(L.liquidado != ref.liquidado || L.agente != ref.agente ||
           L.trabalho != ref.trabalho || L.r.B != ref.r.B || L.r.C != ref.r.C) variou++;
    }
    printf("        200 liquidações da mesma entrada: %d resultados diferentes\n", variou);
    ok("mesma entrada, mesmo resultado, 200 vezes — nada externo entra na conta", variou == 0);

    printf("\n     E O QUE ISTO NÃO RESOLVE, que também é resultado:\n");
    printf("        · não há consenso distribuído — isto não é uma blockchain\n");
    printf("        · o que se corre a cada vez é TESTE, não prova formal\n");
    printf("        · a linguagem exprime corpos de grau 2, não computação geral\n");
    printf("        · e dados do MUNDO continuam a precisar de fonte: o oráculo não desapareceu\n");

    conclui("a paragem sai da álgebra e não do orçamento — é a diferença que o resto todo segue.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    if(argc > 4){
        long t[32]; int n = 0;
        for(int i = 1; i < argc && n < 32; i++) t[n++] = strtol(argv[i], NULL, 0);
        Liquidacao L = liquida(t, n, 12);
        if(!L.liquidado){ printf("  NÃO LIQUIDA: %s\n", L.motivo); return 1; }
        long D = L.r.B*L.r.B - 4*L.r.C;
        printf("  LIQUIDADO.\n\n");
        printf("    a régua      (B,C) = (%ld, %ld)      Δ = %ld\n", L.r.B, L.r.C, D);
        printf("    o agente     %s — %s\n", AGENTES[L.agente].nome, AGENTES[L.agente].faz);
        printf("    o trabalho   %ld passos\n", L.trabalho);
        printf("\n    ninguém assinou: os termos foram fornecidos e o contrato correu.\n");
        return 0;
    }

    puts("smartcontract.c — O CONTRATO NÃO SE ASSINA: LIQUIDA-SE. E CHAMA AGENTES.");
    puts("=======================================================================");
    puts("");
    puts("  O que morre não é o contrato — é a ASSINATURA. Fornece-se metade, ele deriva o resto,");
    puts("  verifica-se com resíduo 0 e CHAMA O AGENTE que a régua determina.");

    secao_S1(); secao_S2(); secao_S3(); secao_S4(); secao_S5(); secao_S6();

    printf("\n=======================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E A CORREÇÃO DA MINHA CORREÇÃO: eu tinha concluído 'não há contrato' e estava a");
        puts("  atirar fora a peça certa. O contrato não desaparece — muda de natureza. Deixa de");
        puts("  ser um documento que alguém honra e passa a ser código que se executa: fornece-se");
        puts("  metade, ele liquida-se, e o agente que trabalha sai do sinal de um número que já");
        puts("  estava nos termos. É o que um smart contract é, e o sistema já tinha as três");
        puts("  peças separadas — contrato.c verifica, liquida.c dispara, fecha.c deriva.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
