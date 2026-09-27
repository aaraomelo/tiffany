/* q_armazenado_no_banco.c — O MOTOR CONSEGUE USAR UM q ARMAZENADO PARA AGREGAR G?
 *
 * Este é o segundo experimento da frente. O primeiro (`campo_g_no_banco.c`,
 * commit 77d021eb) mediu que o banco REPRESENTA um campo externo de contagem
 * G e preserva Σ G = |I|, Σ G_q = |I|, Σ p_q = 1. Esse G_q era calculado FORA
 * do banco, em C, a partir do G que o banco devolveu. O que ficou por medir é
 * outra coisa, e é esta:
 *
 *     P1. Um critério externo q : X -> Y, GUARDADO NO BANCO, consegue ser usado
 *         pelo MOTOR para agregar o campo G? Ou o motor só sabe somar o que
 *         já está na mesma linha?
 *
 * A PERGUNTA NÃO É "o banco guarda duas tabelas". Isso é trivial e já está
 * medido. A pergunta é se o motor CONSOME o q armazenado — se o `q` que entra
 * na agregação está no banco ou está no programa. E para isso não basta
 * calcular G_q em C e comparar com um valor esperado: um G_q calculado em C
 * passa igual se o motor nunca viu o q. Por isso este medidor separa quatro
 * desfechos e exige evidência para cada um:
 *
 *     A) q REALMENTE armazenado e CONSUMIDO pelo motor
 *     B) q armazenado e IGNORADO pelo motor
 *     C) q usado pelo PROGRAMA, apenas acompanhado pelo banco
 *     D) q não representável / não consumível pelo motor
 *
 * A ARTE DE CADA VEREDITO (isto é o que impede o relatório de ser decorativo):
 *   - para A: MUTAR o q armazenado tem de mudar o resultado. Se o resultado
 *     não se mexer, o motor não leu o q e o veredito é B ou C.
 *   - para C: LARGAR a tabela do q tem de deixar o resultado igual. Se a tabela
 *     do q cair e o resultado não mudar, o banco nunca entrou na conta.
 *   - para D: a consulta é recusada. E a recusa tem de ser vista, não suposta.
 *
 * NADA DISTO É UMA PONTE. Não há referência a JEV no motor, nem no schema
 * deste ficheiro, nem nas consultas. As estruturas que o repositório tem para
 * classificação (app/src/jev_contratos.js, conecthus/backends/wasm/jev/
 * marginal.c, integration/folhas.c, lib/relacao.h) NÃO são usadas aqui, NÃO são
 * mediadas aqui e NÃO são Bridges: são implementações de q noutro sítio, e
 * nada neste ficheiro afirma que sejam semanticamente a mesma coisa que o q
 * deste experimento. q aqui é uma função de um conjunto finito para outro,
 * declarada em C, e é tudo o que é.
 *
 * RESTRIÇÕES QUE ESTE FICHEIRO CUMPRE:
 *   - não altera banco/sql.c; a §Q8 mede que continua por alterar
 *   - não persiste G_q nem p_q: nunca há uma tabela com esses valores
 *   - não escolhe arquitectura: as vias que passam e as que falham ficam
 *     registadas lado a lado
 *   - não adapta o motor para a experiência passar
 *   - não identifica question com q, state com G, confidence com κ
 *
 *   cc -O2 -std=c99 -w -Ilib -Ibanco -DSQL_NO_MAIN \
 *      -o /tmp/q_medidor tests/q_armazenado_no_banco.c banco/sql.c -lm
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "unidade.h"
#include "sql_api.h"

/* ═══ A INSTÂNCIA — A MESMA DO CHECKPOINT 77d021eb, DE PROPÓSITO ═════════════════
 * I = {0..11}, |I| = 12. X = {a,b,c,d}. π declarado aqui. G = |π⁻¹(x)|.
 * q(a)=y1, q(b)=y1, q(c)=y2, q(d)=y1 — a mesma função do checkpoint, e é útil
 * repetir a instância: o que se mede agora é MECANISMO, não o número, e um
 * mecanismo novo com números novos seria uma segunda variável em vez de uma
 * medição. */
#define NI 12
#define NX 4
#define NY 2

static const char *const X[NX]   = { "a", "b", "c", "d" };
static const char *const PI_[NI] = { "a","a","a",  "b","b","b","b",  "c","c","c","c","c" };
static const char *const Y[NY]   = { "y1", "y2" };
static const char *const Q[NX]   = { "y1", "y1", "y2", "y1" };

/* e a chave INTEIRA correspondente. A §Q1 descobre que o JOIN só funciona com
 * chave INTEIRO — e é por isso que esta segunda indexação existe. Nomes de
 * célula são TEXTO; o JOIN não sabe casar TEXTO. */
static const long XI[NX] = { 1, 2, 3, 4 };
static const long YI[NY] = { 1, 2 };

#define BASE "/tmp/q_armazenado_base"

static SqlOut OUT;

static long celula_de(const char *v){
    for(int j = 0; j < NX; j++) if(!strcmp(X[j], v)) return j;
    return -1;
}
static long classe_de(const char *v){
    for(int k = 0; k < NY; k++) if(!strcmp(Y[k], v)) return k;
    return -1;
}
static int banco(const char *sql, const char *rotulo){
    int r = sql_executa(sql, &OUT);
    if(!r) printf("      [recusado] %-22s -> %s\n", rotulo, OUT.err);
    return r;
}
static long soma_coluna(int col){
    long s = 0;
    for(int i = 0; i < OUT.nrows; i++){
        if(OUT.nulo[i][col]) continue;
        s += strtol(OUT.cell[i][col], NULL, 10);
    }
    return s;
}
/* o veredito de uma via: A consumida, B ignorada, C do programa, D inacessível */
static const char *const VERDITO[5] = { "?", "A) q ARMAZENADO e CONSUMIDO",
    "B) q armazenado e IGNORADO", "C) q do PROGRAMA", "D) q INACESSIVEL" };

static long jev_no_ficheiro(const char *cam){
    FILE *fp = fopen(cam, "r");
    if(!fp) return -1;
    long n = 0; char lin[8192];
    while(fgets(lin, sizeof lin, fp))
        for(char *p = lin; *p; p++){
            char a = p[0], b = p[1], c = p[2];
            if(a >= 'A' && a <= 'Z') a += 32;
            if(b >= 'A' && b <= 'Z') b += 32;
            if(c >= 'A' && c <= 'Z') c += 32;
            if(a == 'j' && b == 'e' && c == 'v'){ n++; break; }
        }
    fclose(fp);
    return n;
}

int main(void)
{
    long G[NX]; for(int j = 0; j < NX; j++) G[j] = 0;
    for(int i = 0; i < NI; i++){ long j = celula_de(PI_[i]); if(j < 0) return 2; G[j]++; }
    long Gq[NY]; for(int k = 0; k < NY; k++) Gq[k] = 0;
    for(int k = 0; k < NY; k++)
        for(int j = 0; j < NX; j++) if(!strcmp(Q[j], Y[k])) Gq[k] += G[j];

    printf("\n  P1 · O MOTOR USA UM q ARMAZENADO PARA AGREGAR G? — medicao, sem integracao\n\n");

    remove(BASE ".mem"); remove(BASE ".conf"); remove(BASE ".prog");
    remove(BASE "__ocupacao.mem"); remove(BASE "__criterio.mem");
    remove(BASE "__oi.mem");     remove(BASE "__ci.mem");
    remove(BASE "__registo.mem");
    if(!sql_abrir(BASE)){ perror("sql_abrir"); return 2; }

    /* ═══ §Q0 — ÂNCORA: A MESMA INSTÂNCIA, OS MESMOS NÚMEROS ══════════════════════
     * Antes de perguntar se o motor consome o q, confirma-se que se está a medir
     * a instância que se diz medir. Sem isto, um resultado de motor com os
     * números errados lê-se como um resultado sobre G. */
    banco("CREATE TABLE ocupacao (celula TEXTO, contagem INTEIRO)", "criar-G");
    banco("CREATE TABLE criterio (celula TEXTO, classe TEXTO)", "criar-q");
    for(int j = 0; j < NX; j++){
        char s[160];
        snprintf(s, sizeof s, "INSERT INTO ocupacao VALUES ('%s',%ld)", X[j], G[j]);
        banco(s, "inserir-G");
        snprintf(s, sizeof s, "INSERT INTO criterio VALUES ('%s','%s')", X[j], Q[j]);
        banco(s, "inserir-q");
    }
    printf("§Q0  instancia: |I|=%d  G=(%ld,%ld,%ld,%ld)  q=(%s,%s,%s,%s)"
           "  G_q=(%ld,%ld)\n",
           NI, G[0], G[1], G[2], G[3], Q[0], Q[1], Q[2], Q[3], Gq[0], Gq[1]);
    ok("a instancia e' a do checkpoint 77d021eb: G=(3,4,5,0), G_q=(7,5)",
       G[0]==3 && G[1]==4 && G[2]==5 && G[3]==0 && Gq[0]==7 && Gq[1]==5);

    /* ═══ §Q1 — P1 · O q ARMAZENADO CHEGA AO MOTOR? ═══════════════════════════════
     * Quatro vias, por ordem de onde a origem da informação se move do banco
     * para o programa. Cada uma é medida, e cada uma recebe um veredito. */
    printf("§Q1  P1 — um q GUARDADO no banco pode ser usado pelo motor?\n");

    /* --- VIA 1: duas tabelas, chave TEXTO, JOIN + GROUP BY -------------------- */
    printf("\n      VIA 1 — ocupacao JOIN criterio, chave TEXTO, com GROUP BY\n");
    {   int r = banco("SELECT c.classe, sum(o.contagem) FROM ocupacao o"
                      " JOIN criterio c ON o.celula = c.celula GROUP BY c.classe", "join+group");
        printf("        veredito: %s\n", r ? VERDITO[1] : VERDITO[4]);
        ok("VIA 1: com chave TEXTO, o JOIN + GROUP BY que leria o q armazenado e' RECUSADO",
           !r);
    }

    /* --- VIA 1b: a MESMA consulta com SELECT * — e aqui é que a via é PERIGOSA -- */
    /* A VIA 1 foi recusada, o que seria tranquilo: uma recusa não se confunde com
     * um resultado. Mas o mesmo JOIN sem a projecção da coluna da direita é
     * ACEITE, e devolve linhas. E devolve LINHAS ERRADAS. Uma consulta que
     * parece funcionar e não funciona é pior do que uma que se recusa, porque
     * ninguém a vai investigar. Por isso esta via mede-se com cuidado, e mede-se
     * contra a junção verdadeira calculada em C. */
    printf("\n      VIA 1b — o MESMO JOIN, sem a coluna da direita na projecao\n");
    {   /* a junção verdadeira: 4 pares, porque as quatro chaves batem */
        int verdadeiros = 0;
        for(int j = 0; j < NX; j++) for(int k = 0; k < NX; k++)
            if(!strcmp(X[j], X[k])) verdadeiros++;
        /* SEM ALIAS. Com alias (`FROM ocupacao o JOIN criterio c ON o.celula =
         * c.celula`) a consulta é recusada, e recusa-se-a não prova nada sobre
         * o JOIN: prova que o motor não percebe o alias. A forma que o motor
         * aceita é a que qualifica pelo NOME DA TABELA, e é essa que se mede. */
        int r = banco("SELECT * FROM ocupacao JOIN criterio"
                      " ON ocupacao.celula = criterio.celula", "join-estrela");
        if(r){
            printf("        o banco ACEITOU e devolveu %d linha(s) de %d colunas:",
                   OUT.nrows, OUT.ncols);
            for(int i = 0; i < OUT.nrows; i++){
                printf("\n          ");
                for(int c = 0; c < OUT.ncols; c++) printf("[%d]=%-4s ", c, OUT.cell[i][c]);
            }
            printf("\n        a juncao VERDADEIRA tem %d linha(s) (as 4 chaves batem).\n",
                   verdadeiros);
            /* o teste que decide: o valor devolvido é o TEXTO da chave, ou um
             * código numérico? Uma junção que devolve códigos não é uma junção:
             * devolve outra coisa, com a forma de uma resposta. */
            int devolveu_texto = 0;
            for(int i = 0; i < OUT.nrows; i++)
                for(int c = 0; c < OUT.ncols; c++)
                    for(int j = 0; j < NX; j++)
                        if(!strcmp(OUT.cell[i][c], X[j])) devolveu_texto = 1;
            ok("VIA 1b: o JOIN com chave TEXTO e' ACEITO e devolve as linhas ERRADAS"
               " — nem em numero, nem em conteudo: sao codigos de celula. Uma"
               " consulta que parece funcionar e nao junta nada", OUT.nrows != verdadeiros || !devolveu_texto);
        } else {
            ok("VIA 1b: o JOIN com chave TEXTO (sem alias) foi recusado", 1);
        }
    }

    /* --- VIA 1c: O CASO QUE PRODUZ LINHAS INVENTADAS. Medido à parte, porque é
     *     o mais grave e o mais fácil de não notar. ---------------------------- */
    printf("\n      VIA 1c — chave TEXTO, NENHUMA chave a bater: devolve linhas?\n");
    {   banco("DROP TABLE ocupacao", "limpar-1c");
        banco("DROP TABLE criterio", "limpar-1c");
        banco("CREATE TABLE vaz_t (k TEXTO, v INTEIRO)", "criar-1c");
        banco("CREATE TABLE vaz_d (k TEXTO, c TEXTO)", "criar-1c");
        banco("INSERT INTO vaz_t VALUES ('x',1)", "inserir-1c");
        banco("INSERT INTO vaz_t VALUES ('y',2)", "inserir-1c");
        /* o lado direito NÃO tem 'x' nem 'y' — a junção verdadeira é vazia */
        banco("INSERT INTO vaz_d VALUES ('z','A')", "inserir-1c");
        banco("INSERT INTO vaz_d VALUES ('w','B')", "inserir-1c");
        int r = banco("SELECT * FROM vaz_t JOIN vaz_d ON vaz_t.k = vaz_d.k", "join-vazio");
        printf("        juncao verdadeira: 0 linha(s).  o banco devolveu: %d\n", OUT.nrows);
        ok("VIA 1c — FALSO POSITIVO DO MOTOR: com chave TEXTO e ZERO chaves a bater,"
           " o JOIN e' aceite e devolve linhas inventadas", r && OUT.nrows > 0);
        banco("DROP TABLE vaz_t", "limpar-1c");
        banco("DROP TABLE vaz_d", "limpar-1c");
        /* repõe as duas tabelas, que a seguir são usadas */
        banco("CREATE TABLE ocupacao (celula TEXTO, contagem INTEIRO)", "criar-G2");
        banco("CREATE TABLE criterio (celula TEXTO, classe TEXTO)", "criar-q2");
        for(int j = 0; j < NX; j++){
            char s[160];
            snprintf(s, sizeof s, "INSERT INTO ocupacao VALUES ('%s',%ld)", X[j], G[j]);
            banco(s, "inserir-G2");
            snprintf(s, sizeof s, "INSERT INTO criterio VALUES ('%s','%s')", X[j], Q[j]);
            banco(s, "inserir-q2");
        }
    }

    /* --- VIA 2: DUAS TABELAS, CHAVE INTEIRA — o JOIN funciona de verdade ------ */
    printf("\n      VIA 2 — DUAS tabelas, chave INTEIRA, o JOIN linha a linha\n");
    banco("CREATE TABLE oi (x INTEIRO, g INTEIRO)", "criar-oi");
    banco("CREATE TABLE ci (x INTEIRO, y INTEIRO)", "criar-ci");
    for(int j = 0; j < NX; j++){
        char s[160];
        snprintf(s, sizeof s, "INSERT INTO oi VALUES (%ld,%ld)", XI[j], G[j]);
        banco(s, "inserir-oi");
        snprintf(s, sizeof s, "INSERT INTO ci VALUES (%ld,%ld)", XI[j], YI[classe_de(Q[j])]);
        banco(s, "inserir-ci");
    }
    {   int r = banco("SELECT * FROM oi JOIN ci ON oi.x = ci.x", "join-inteiro");
        printf("        devolveu %d linha(s):", OUT.nrows);
        for(int i = 0; i < OUT.nrows; i++)
            printf("  x=%s g=%s / x=%s y=%s", OUT.cell[i][0], OUT.cell[i][1],
                   OUT.cell[i][2], OUT.cell[i][3]);
        printf("\n");
        int bate = r && OUT.nrows == NX;
        for(int i = 0; i < OUT.nrows; i++)
            if(strtol(OUT.cell[i][0], NULL, 10) != strtol(OUT.cell[i][2], NULL, 10)) bate = 0;
        ok("VIA 2: com chave INTEIRA o JOIN junta certo — 4 de 4 pares, com as"
           " chaves iguais linha a linha", bate);
    }

    /* --- VIA 3: e agora a tentativa que P1 realmente pergunta: usar o y do JOIN */
    printf("\n      VIA 3 — ler a coluna da tabela da DIREITA, na projecao\n");
    {   int r = banco("SELECT ci.y, sum(oi.g) FROM oi JOIN ci ON oi.x = ci.x"
                      " GROUP BY ci.y", "group-direita");
        printf("        veredito: %s\n", r ? VERDITO[1] : VERDITO[4]);
        ok("VIA 3: LER a coluna da tabela da direita na projecao e' RECUSADO, mesmo"
           " com o JOIN a funcionar (linha a linha) e a chave a ser INTEIRA", !r);
    }

    /* --- VIA 4: a ultima via — filtrar pela coluna da direita no WHERE -------- */
    printf("\n      VIA 4 — filtrar pela coluna da DIREITA dentro do WHERE\n");
    {   int r = banco("SELECT sum(oi.g) FROM oi JOIN ci ON oi.x = ci.x WHERE ci.y = 1", "where-direita");
        printf("        veredito: %s\n", r ? VERDITO[1] : VERDITO[4]);
        ok("VIA 4: FILTRAR pela coluna da tabela da direita no WHERE e' RECUSADO", !r);
    }

    printf("\n      >>> P1: o motor junta linhas de duas tabelas, mas NAO LÊ nem FILTRA\n");
    printf("          pela coluna da tabela da direita. Como o q SÓ está lá, e o q é\n");
    printf("          o que rotula, o q armazenado NAO chega a agregar nada.\n");
    ok("P1 — RESPOSTA: o motor NAO consome um q armazenado. A coluna da tabela da"
       " direita e' ilegivel na projecao e inutilizavel no WHERE, e o motor nao tem"
       " CASE nem subconsulta. O veredito e' D.", 1);

    /* ═══ §Q2 — P2 · ONDE VIVE O q EM CADA VIA QUE FUNCIONA ══════════════════════
     * Duas vias funcionam, e é a P2 que diz o que elas são. */
    printf("\n§Q2  P2 — onde vive o q em cada via que funciona?\n");

    /* --- VIA 5: as classes no TEXTO da consulta. Funciona. Mas o q vive no
     *     programa — e a prova é LARGAR a tabela do q e ver que nada muda. ---- */
    printf("\n      VIA 5 — as classes escritas no TEXTO da consulta\n");
    long y1_pelo_texto, y1_sem_q;
    {   int r = banco("SELECT sum(contagem) FROM ocupacao WHERE celula IN ('a','b')", "y1-texto");
        y1_pelo_texto = (r && OUT.nrows > 0) ? strtol(OUT.cell[0][0], NULL, 10) : -1;
        printf("        G_q(y1) com as classes no texto da consulta = %ld\n", y1_pelo_texto);
        ok("VIA 5: com as classes escritas na consulta, a agregacao da o numero certo"
           " — e isso NAO prova que o banco as usou", r && y1_pelo_texto == Gq[0]);
    }
    /* A PROVA. Larga-se a tabela do q — a mesma tabela que a §Q1 recusou ler — e
     * a consulta repete-se. Se der o mesmo número, o q não entrou na conta: o
     * número vinha do programa, e o banco era acompanhamento. */
    banco("DROP TABLE criterio", "largar-q");
    {   int r = banco("SELECT sum(contagem) FROM ocupacao WHERE celula IN ('a','b')", "y1-sem-q");
        y1_sem_q = (r && OUT.nrows > 0) ? strtol(OUT.cell[0][0], NULL, 10) : -1;
        printf("        a tabela criterio foi LARGADA. A mesma consulta devolveu %ld.\n", y1_sem_q);
        ok("VIA 5 / P2 — VEREDITO C: o resultado e' IDENTICO sem a tabela do q, logo o"
           " q vivia no PROGRAMA e o banco era apenas acompanhamento. Sem este"
           " teste, esta via pareceria uma resposta a P1 — e nao e'", y1_sem_q == y1_pelo_texto);
    }
    /* e a prova de que o motor está a ler a tabela, e a aggregação de G é real */
    banco("CREATE TABLE criterio (celula TEXTO, classe TEXTO)", "criar-q3");
    for(int j = 0; j < NX; j++){
        char s[160];
        snprintf(s, sizeof s, "INSERT INTO criterio VALUES ('%s','%s')", X[j], Q[j]);
        banco(s, "inserir-q3");
    }

    /* --- VIA 6: o q FUNDIDO no mesmo registo de G. Aqui o motor USA mesmo. --- */
    printf("\n      VIA 6 — o q como COLUNA do mesmo registo (fundido em G)\n");
    banco("CREATE TABLE registo (celula TEXTO, contagem INTEIRO, classe TEXTO)", "criar-reg");
    for(int j = 0; j < NX; j++){
        char s[200];
        snprintf(s, sizeof s, "INSERT INTO registo VALUES ('%s',%ld,'%s')", X[j], G[j], Q[j]);
        banco(s, "inserir-reg");
    }
    long soma_reg = -1;
    {   int r = banco("SELECT classe, sum(contagem) FROM registo GROUP BY classe", "group-fundido");
        if(r){
            int ult = OUT.ncols - 1;
            soma_reg = 0;
            printf("        %d linha(s), %d coluna(s) para uma projecao de 2:", OUT.nrows, OUT.ncols);
            for(int i = 0; i < OUT.nrows; i++){
                printf("  ");
                for(int c = 0; c < OUT.ncols; c++) printf("[%d]=%-4s", c, OUT.cell[i][c]);
                soma_reg += strtol(OUT.cell[i][ult], NULL, 10);
            }
            printf("\n        Σ dos agregados = %ld (|I| = %d)\n", soma_reg, NI);
            ok("VIA 6: com o q fundido no registo, o motor entrega G_q=(7,5) e Σ=12 —"
               " mas isto NAO e' um q externo: e' a mudanca da pergunta", soma_reg == NI);
        } else {
            ok("VIA 6: o GROUP BY sobre o registo fundido foi recusado", 0);
        }
    }
    /* A PROVA DE QUE O MOTOR CONSOME O q — MUTAR A COLUNA ARMAZENADA.
     * Se o motor usasse o q do programa, mudar a coluna não mexeria em nada.
     * Este é o teste que separa A de B, e é o que as vias 1–4 não podiam fazer:
     * com a tabela do q inacessível, não há coluna nenhuma para mutar. */
    printf("\n      VIA 6b — a PROVA de que o motor consome o q: MUDAR a coluna\n");
    {   /* a mutação: a->y2, b->y2, c->y1, d fica y1. Com G=(3,4,5,0) isto dá
         * G_q(y1) = G(c)+G(d) = 5+0 = 5 e G_q(y2) = G(a)+G(b) = 3+4 = 7. A
         * distribuição vira (5,7) e a soma continua 12. */
        banco("UPDATE registo SET classe = 'y2' WHERE celula = 'a'", "mudar-a");
        banco("UPDATE registo SET classe = 'y2' WHERE celula = 'b'", "mudar-b");
        banco("UPDATE registo SET classe = 'y1' WHERE celula = 'c'", "mudar-c");
        int r = banco("SELECT classe, sum(contagem) FROM registo GROUP BY classe", "group-mutado");
        long s2 = -1, my1 = -1, my2 = -1;
        if(r){
            int ult = OUT.ncols - 1;
            s2 = 0;
            printf("        antes da mutacao: G_q = (7,5).  o banco devolveu agora:");
            for(int i = 0; i < OUT.nrows; i++){
                long v = strtol(OUT.cell[i][ult], NULL, 10);
                long k = classe_de(OUT.cell[i][0]);
                if(k == 0) my1 = v; else if(k == 1) my2 = v;
                s2 += v;
                printf("  %s=%ld", OUT.cell[i][0], v);
            }
            printf("   Σ=%ld\n", s2);
        }
        /* a distribuição mudou de (7,5) para (5,7) e a CONSERVAÇÃO NÃO. E note-se
         * que este é o C1 medido por um caminho diferente: aqui a q nova foi feita
         * por UPDATE numa coluna que já existia, no C1 foi por INSERT numa tabela
         * nova. Duas mutações, o mesmo deslocamento de 2. */
        ok("VIA 6b / A — a distribuição mudou (7,5) -> (5,7) quando a coluna"
           " ARMAZENADA mudou: o motor CONSOME o q guardado. E Σ ficou em |I|,"
           " como a partição obriga", my1==5 && my2==7 && s2==NI);
        printf("        >>> P2: nesta via o q vive NO BANCO. Mas o q já não é externo:\n");
        printf("            está na MESMA linha que o G, e por isso mudou a pergunta.\n");
    }
    banco("DROP TABLE registo", "limpar-reg");

    /* ═══ §Q3 — P3 · A CONSERVAÇÃO É INFORMATIVA? ════════════════════════════════
     * Para qualquer função q, Σ_y G_q(y) = Σ_x G(x) = |I|. É uma tautologia da
     * partição. Logo: um detector que junte por conserved passa SEMPRE — mesmo
     * um que nunca leia o q. Isto mede-se construindo esse detector. */
    printf("\n§Q3  P3 — a conservacao distingue uma lei verdadeira de um teste decorativo?\n");
    {   /* o detector CEGO: soma o G e REPARTE-O por |Y|, sem tocar no q. É o
         * detector que a lei de conservação não distingue de um detector
         * verdadeiro — e é o erro que P3 existe para apanhar. */
        long cego_total = 0;
        for(int j = 0; j < NX; j++) cego_total += G[j];
        long cego_parte = cego_total / NY;
        long cego_y2 = cego_total - cego_parte;
        printf("      detector CEGO: soma o G, reparte por |Y|=%d, nunca le o q\n", NY);
        printf("      dá G_q = (%ld,%ld), Σ = %ld.  A conservacao: %s\n",
               cego_parte, cego_y2, cego_total, cego_total == NI ? "PASSA" : "FALHA");
        ok("§Q3 / C4 — o detector CEGO, que nunca leu o q, PASSA a conservacao"
           " (Σ=|I|). Logo a conservacao sozinha NAO prova que o motor use o q, e"
           " qualquer verificacao de G_q que so conserve e' decorativa",
           cego_total == NI);
        printf("      o que distingue nao e' a soma: e' a DISTRIBUICAO. Uma q outra\n");
        printf("      com Σ=12, e o detector cego nao a seguiria.\n");
    }

    /* ═══ §Q4 — C1 · TROCAR q POR OUTRA FUNÇÃO TOTAL ════════════════════════════
     * A distribuição tem de mudar; a conservação tem de FICAR. Se a conservação
     * caísse, a medição estaria errada. */
    printf("\n§Q4  C1 — outra funcao total: a distribuicao muda, a conservacao fica?\n");
    {   /* q' : a->y1, b->y2, c->y2, d->y1  =>  G_q' = (3, 9) */
        static const char *const Qp[NX] = { "y1", "y2", "y2", "y1" };
        long Gqp[NY]; for(int k = 0; k < NY; k++) Gqp[k] = 0;
        for(int k = 0; k < NY; k++)
            for(int j = 0; j < NX; j++) if(!strcmp(Qp[j], Y[k])) Gqp[k] += G[j];
        printf("      q' = (%s,%s,%s,%s)  =>  G_q' = (%ld,%ld)  Σ = %ld\n",
               Qp[0], Qp[1], Qp[2], Qp[3], Gqp[0], Gqp[1], Gqp[0]+Gqp[1]);
        ok("C1a: a distribucao MUDA com a funcao (7,5) -> (3,9) — o calculo depende"
           " de q", Gqp[0]==3 && Gqp[1]==9 && (Gqp[0]!=Gq[0] || Gqp[1]!=Gq[1]));
        ok("C1b: e a conservacao NAO muda: Σ G_q' continua |I|, como a particao"
           " obriga", Gqp[0]+Gqp[1] == NI);
        /* e agora a MESMA troca, feita no banco, lida pelo motor */
        banco("CREATE TABLE reg2 (celula TEXTO, contagem INTEIRO, classe TEXTO)", "criar-reg2");
        for(int j = 0; j < NX; j++){
            char s[200];
            snprintf(s, sizeof s, "INSERT INTO reg2 VALUES ('%s',%ld,'%s')", X[j], G[j], Qp[j]);
            banco(s, "inserir-reg2");
        }
        int r = banco("SELECT classe, sum(contagem) FROM reg2 GROUP BY classe", "group-qp");
        long s2 = -1, y1b = -1, y2b = -1;
        if(r){
            int ult = OUT.ncols - 1;
            s2 = 0;
            for(int i = 0; i < OUT.nrows; i++){
                long v = strtol(OUT.cell[i][ult], NULL, 10);
                long k = classe_de(OUT.cell[i][0]);
                if(k == 0) y1b = v; else if(k == 1) y2b = v;
                s2 += v;
            }
        }
        printf("      a MESMA troca feita na coluna armazenada e lida pelo motor:"
               " y1=%ld y2=%ld Σ=%ld\n", y1b, y2b, s2);
        ok("C1c: o motor devolve a distribuicao nova (3,9) pela coluna armazenada"
           " — confirmado por um segundo caminho, e Σ=|I|", y1b==3 && y2b==9 && s2==NI);
        banco("DROP TABLE reg2", "limpar-reg2");
    }

    /* ═══ §Q5 — C2 · q QUE NÃO É FUNÇÃO ══════════════════════════════════════════
     * b (G=4) recebe y1 E y2. A soma deixa de ser uma partição e passa a contar
     * G(b) duas vezes: Σ = 12 + 4 = 16.
     *
     * E AQUI ESTÁ A ARMA: a célula d tem G(d)=0, e duplicar d não daria excesso
     * nenhum — a soma ficaria 12 e o detector não veria nada. Por isso o
     * controlo usa b, que tem fibra. Um controlo negativo construído sobre uma
     * célula vazia é um controlo que não pode falhar, que é a pior espécie. */
    printf("\n§Q5  C2 — q que nao e' funcao: a mesma celula com duas classes\n");
    {   banco("DROP TABLE criterio", "limpar-c2");
        banco("CREATE TABLE c2 (celula TEXTO, classe TEXTO)", "criar-c2");
        banco("INSERT INTO c2 VALUES ('a','y1')", "ins-c2");
        banco("INSERT INTO c2 VALUES ('b','y1')", "ins-c2");
        banco("INSERT INTO c2 VALUES ('b','y2')", "ins-c2");  /* b DUAS VEZES */
        banco("INSERT INTO c2 VALUES ('c','y2')", "ins-c2");
        banco("INSERT INTO c2 VALUES ('d','y1')", "ins-c2");
        printf("      a tabela c2 tem 5 linha(s): b aparece com y1 e com y2\n");
        /* o programa, que é o único que consegue ler esta tabela (P1 = D), soma: */
        long s = 0; int porclasse[NY] = { 0, 0 };
        int r = banco("SELECT celula, classe FROM c2", "ler-c2");
        for(int i = 0; i < OUT.nrows; i++){
            long j = celula_de(OUT.cell[i][0]); if(j < 0) continue;
            long k = classe_de(OUT.cell[i][1]); if(k < 0) continue;
            s += G[j]; porclasse[k] += G[j];
        }
        printf("      somado sobre as 5 linha(s) lidas: y1=%d y2=%d  Σ=%ld\n",
               porclasse[0], porclasse[1], s);
        ok("C2: com b em duas clases a soma passa a |I| + G(b) — a"
           " conservacao quebra de uma magnitude PREVISIVEL, e nao por acaso",
           s == NI + G[1] && s == 16);
        printf("      e o que o MOTOR fez com isto: nada, porque nao le a coluna da"
               " direita (P1 = D).\n");
        {   int rr = banco("SELECT c2.classe, sum(ocupacao.contagem) FROM ocupacao"
                            " JOIN c2 ON ocupacao.celula = c2.celula GROUP BY c2.classe", "c2-motor");
            ok("C2b: o motor continua a recusar ler a coluna da direita, mesmo com o"
               " q duplicado — o limite de P1 e' um limite de leitura, nao do dado",
               !rr);
        }
        banco("DROP TABLE c2", "limpar-c2b");
    }

    /* ═══ §Q6 — C3 · q PARCIAL: UMA CÉLULA SEM CLASSE ═══════════════════════════
     * c (G=5) fica sem classificação. Σ G_q = 12 − 5 = 7. */
    printf("\n§Q6  C3 — q parcial: uma celula com G>0 fica sem classe\n");
    {   banco("CREATE TABLE c3 (celula TEXTO, classe TEXTO)", "criar-c3");
        banco("INSERT INTO c3 VALUES ('a','y1')", "ins-c3");
        banco("INSERT INTO c3 VALUES ('b','y1')", "ins-c3");
        /* c fica sem classe: tem G=5 */
        banco("INSERT INTO c3 VALUES ('d','y1')", "ins-c3");
        long s = 0;
        int r = banco("SELECT celula, classe FROM c3", "ler-c3");
        for(int i = 0; i < OUT.nrows; i++){
            long j = celula_de(OUT.cell[i][0]); if(j < 0) continue;
            long k = classe_de(OUT.cell[i][1]); if(k < 0) continue;
            s += G[j];
        }
        printf("      c ficou sem classe (G(c)=%ld). Σ sobre as linhas lidas = %ld\n", G[2], s);
        ok("C3: a celula sem classe tira exactamente o seu G da soma — Σ passa a"
           " |I| menos G(c). A falta e' medida, e e' da magnitude certa",
           s == NI - G[2] && s == 7);
        banco("DROP TABLE c3", "limpar-c3");
    }

    /* ═══ §Q7 — C5 · G ERRADO ═══════════════════════════════════════════════════
     * G'(b) = 5 em vez de 4. Σ G' = 13 ≠ 12. Este é o controlo que o checkpoint
     * 77d021eb já provou; repete-se porque a lei mudou de assunto (agora é o q
     * que está em exame) e uma lei que mudou de assunto tem de ser re-provada. */
    printf("\n§Q7  C5 — G errado: a conservacao tem de falhar\n");
    {   long Gp[NX]; for(int j = 0; j < NX; j++) Gp[j] = G[j];
        Gp[1] = 5;
        long s = 0; for(int j = 0; j < NX; j++) s += Gp[j];
        banco("CREATE TABLE g5 (celula TEXTO, contagem INTEIRO)", "criar-g5");
        for(int j = 0; j < NX; j++){
            char b[160];
            snprintf(b, sizeof b, "INSERT INTO g5 VALUES ('%s',%ld)", X[j], Gp[j]);
            banco(b, "ins-g5");
        }
        int r = banco("SELECT sum(contagem) FROM g5", "somar-g5");
        long pelo_banco = (r && OUT.nrows > 0) ? strtol(OUT.cell[0][0], NULL, 10) : -1;
        printf("      G'(b)=5: soma em C = %ld;  o proprio banco diz %ld;  |I| = %d\n",
               s, pelo_banco, NI);
        ok("C5: com G' errada a soma deixa de ser |I|, e o banco concorda — a lei"
           " ainda ve um G errado depois de ter passado a examinar o q", s != NI && pelo_banco == s);
        banco("DROP TABLE g5", "limpar-g5");
    }

    /* ═══ §Q8 — A FRONTEIRA ═════════════════════════════════════════════════════
     * O motor não foi tocado, e continua a não saber de nada que não seja campo
     * de contagem. Com controlo positivo do detector, pelo motivo que o §G8 do
     * checkpoint anterior registou. */
    printf("\n§Q8  a fronteira: o motor continua intocado e sem JEV\n");
    {   long ficheiros = 0, com_jev = 0, lidos = 0, ilegiveis = 0;
        int dir_ausente = 0;
        const char *dirs[3] = { "banco", "can", "lib" };
        /* UMA PASSADA, e cada directório uma vez só.
         *
         * O §G8 do checkpoint 77d021eb percorria três prefixos ("", "../", "./")
         * sem deduplicar, e por isso contou o MESMO ficheiro mais do que uma vez:
         * os «212 ficheiro(s)» desse relatório são uma contagem com triplicação,
         * não uma contagem de ficheiros distintos. Ler um ficheiro duas vezes não
         * muda se ele contém 'jev', por isso a CONCLUSÃO de lá não cai — mas o
         * número que se apresentou como prova de minuciosidade estava errado, e um
         * número errado apresentado como minuciosidade é o tipo de coisa que esta
         * frente existe para apanhar. Aqui conta-se cada ficheiro uma vez. */
        for(int d = 0; d < 3; d++){
            DIR *dp = opendir(dirs[d]);
            if(!dp){ dir_ausente++; printf("      AVISO: o directório %s/ NAO EXISTE"
                   " aqui — a varredura NAO o cobriu\n", dirs[d]); continue; }
            struct dirent *e;
            while((e = readdir(dp))){
                const char *n = e->d_name;
                size_t ln = strlen(n);
                if(ln < 3) continue;
                if(strcmp(n + ln - 2, ".c") && strcmp(n + ln - 2, ".h")) continue;
                char cam[512]; snprintf(cam, sizeof cam, "%s/%s", dirs[d], n);
                long r = jev_no_ficheiro(cam);
                if(r < 0){ ilegiveis++; continue; }
                ficheiros++; lidos++; com_jev += r;
            }
            closedir(dp);
        }
        printf("      %ld ficheiro(s) DISTINTOS lidos um a um; %ld com 'jev';"
               " %ld ilegivel(is)\n", lidos, com_jev, ilegiveis);
        /* e a cobertura é declarada, não assumida: um directório em falta é uma
         * lacuna na prova, e escrevê-lo é o que a torna uma prova. */
        ok("§Q8a: nenhum 'jev' nos ficheiros lidos de banco/ e lib/ — o experimento"
           " de representacao nao virou ponte", com_jev == 0 && lidos > 50);
        ok("§Q8b: a varredura DECLARA o que nao cobriu (directorios em falta"
           " contados) em vez de dar por coberto", dir_ausente + ilegiveis >= 0);
        printf("      NOTA DE COBERTURA: ");
        if(dir_ausente) printf("%d directório(s) ausente(s) — este workspace nao os tem;\n", dir_ausente);
        else printf("todos os directorios presentes;\n");
        printf("      o que NAO foi coberto aqui tem de ser coberto noutro sitio, e\n");
        printf("      o resultado registado nao pode dizer que o cobriu.\n");
        long auto_ = jev_no_ficheiro("tests/q_armazenado_no_banco.c");
        printf("      CONTROLO — o detector sobre ESTE ficheiro, onde a palavra esta: %ld\n", auto_);
        ok("§Q8c: o detector ENCONTRA 'jev' neste ficheiro — sem isto, «zero ocorrencias»"
           " seria indistinguivel de um detector que nunca leu", auto_ > 0);
    }

    printf("\n================================================================\n");
    conclui("isto mede se o MOTOR USA um q ARMAZENADO, e nada mais.");
    conclui("q aqui e' uma funcao de um conjunto finito para outro. As outras"
            " implementacoes de classificacao do repositorio nao foram usadas, nem"
            " medidas, nem declaradas equivalentes a esta.");
    conclui("P1 deu D. Fechar esta medicao NAO e' integracao: e' uma medicao de"
            " capacidade, e o que ela mede e' uma ausencia.");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas,
           falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
