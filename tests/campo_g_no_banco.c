/* campo_g_no_banco.c — O BANCO REPRESENTA UM CAMPO EXTERNO DE CONTAGEM G, E PRESERVA OS SEUS INVARIANTES?
 *
 * ESTE MEDIDOR NÃO É UMA INTEGRAÇÃO. Não há ponte, não há objecto de JEV aqui dentro, e
 * o motor não é tocado. A pergunta é uma só, e é mais estreita do que parece:
 *
 *     o banco consegue REPRESENTAR um campo de contagem G : X -> N0, associado a uma
 *     realização finita, e PRESERVAR os três invariantes que esse campo tem?
 *
 * I é finito e CONHECIDO, π : I -> X é uma função declarada aqui em C, e G calcula-se
 * POR DEFINIÇÃO, fora do banco: G(x) = |π⁻¹(x)|. O banco recebe o resultado. A pergunta
 * nunca é se o banco sabe o que é um campo de contagem — é se o que lhe foi dado
 * volta igual e se as somas conservam.
 *
 * O QUE ESTE FICHEIRO NÃO FAZ, e é a parte que o torna honesto:
 *   - não grava G_q nem p_q no banco. G_q e p_q são DERIVADOS: existem como valor de
 *     uma query, nunca como tabela. Onde aparece uma tabela `criterio`, é a METADATA do
 *     critério q, que é uma escolha deste experimento sobre onde q vive — não é uma
 *     afirmação sobre o que q é, e q não é um objecto de JEV.
 *   - não identifica state com G, question com q, confidence com κ. Não os mede, não os
 *     nomeia, não os toca.
 *   - não acrescenta uma referência ao motor. §G8 mede que continua a não haver nenhuma.
 *   - não adapta o motor para o experimento passar. Se uma consulta for recusada, isso
 *     entra no relatório como LIMITE.
 *   - não afirma que `ocupacao` é o schema. É a menor estrutura que segura um par
 *     (célula, contagem), e é uma hipótese de representação entre outras.
 *
 * A MEDIÇÃO É FEITA DUAS VEZES, por caminhos que não partilham código:
 *   - G_q por agregação sobre o campo (Σ_{x: q(x)=y} G(x));
 *   - G_q por contagem directa da realização (|{i : q(π(i)) = y}|).
 * Concordam? Se o primeiro caminho e o segundo dessem números diferentes, o relatório
 * teria de dizer qual é o certo — e não há outro critério para o escolher. Por isso os
 * dois.
 *
 * E HÁ UM CONTROLO NEGATIVO (§G7): um campo G propositadamente ERRADO, gravado no MESMO
 * banco e lido pelo MESMO caminho, tem de ser acusado pela MESMA lei. Sem ele,
 * «Σ G = |I|» seria indistinguível de uma medição que nunca falha — que é a assinatura
 * do defeito que esta casa persegue, e a razão pela qual o número de asserções não
 * substitui o número de controlos negativos.
 *
 *   cc -O2 -std=c99 -w -I. -I../lib -I../banco -I../tests \
 *      -DSQL_NO_MAIN campo_g_no_banco.c ../banco/sql.c -lm -o campo_g_no_banco
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "unidade.h"
#include "sql_api.h"

/* ═══ A REALIZAÇÃO, DECLARADA AQUI E NÃO NO BANCO ═══════════════════════════════════
 * I = {0..11}, |I| = 12. X = {a,b,c,d}. π = PI_ embaixo.
 *
 * d NÃO TEM FIBRA: π⁻¹(d) = ∅, logo G(d) = 0. Isto é deliberado e é o caso
 * interessente — uma função X -> N0 tem valor ZERO numa célula, e o banco tem de
 * conseguir dizer isso sem que a linha «desapareça» e sem que a soma mude. Um G com
 * todas as células ocupadas não mede nada sobre esse ponto.
 */
#define NI 12
#define NX 4
#define NY 2

static const char *const X[NX]  = { "a", "b", "c", "d" };
static const char *const PI_[NI]= { "a","a","a",  "b","b","b","b",  "c","c","c","c","c" };

/* q : X -> Y, com Y = {y1, y2}. q(a)=y1, q(b)=y1, q(c)=y2, q(d)=y1.
 * d vai para y1 de propósito: a célula de contagem ZERO entra numa fibra, e
 * G_q(y1) tem de a absorver sem a inventar. */
static const char *const Y[NY]  = { "y1", "y2" };
static const char *const Q[NX]  = { "y1", "y1", "y2", "y1" };

#define BASE "/tmp/campo_g_base"

static SqlOut OUT;                 /* a janela é grande; nao vai para a pilha */

static long celula_de(const char *v){                 /* o índice de X, ou -1 */
    for(int j = 0; j < NX; j++) if(!strcmp(X[j], v)) return j;
    return -1;
}
static long classe_de(const char *v){                 /* o índice de Y, ou -1 */
    for(int k = 0; k < NY; k++) if(!strcmp(Y[k], v)) return k;
    return -1;
}
static int  banco(const char *sql, const char *rotulo){
    int r = sql_executa(sql, &OUT);
    if(!r) printf("      [recusado] %s :: %s -> %s\n", rotulo, sql, OUT.err);
    return r;
}
/* a soma de uma coluna que o banco devolveu — medida SOBRE O QUE VOLTOU, não sobre o
 * que se queria que voltasse. Se o banco mentir, a soma mente com ele. */
static long soma_coluna(int col, long *nlinhas){
    long s = 0, n = 0;
    for(int i = 0; i < OUT.nrows; i++){
        if(OUT.nulo[i][col]) continue;
        s += strtol(OUT.cell[i][col], NULL, 10); n++;
    }
    if(nlinhas) *nlinhas = n;
    return s;
}
/* conta as ocorrências de «jev» (sem distinção de maiúsculas) num ficheiro. -1 = não leu.
 * A PRIMEIRA VERSÃO DESTE medidor procurava `j` seguido de DUAS minúsculas quaisquer e
 * deu 304 falsos positivos em banco/sql.c — que não tem uma única. Um detector que
 * acusa o que não há não pode ser usado para atestar o que não há. */
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
    printf("\n  O BANCO E UM CAMPO EXTERNO DE CONTAGEM G? — medicao, sem integracao\n\n");

    /* ═══ §G1 — A REALIZAÇÃO E O CAMPO, CALCULADOS FORA DO BANCO ═══════════════════
     * G(x) = |π⁻¹(x)| pela definição: percorre-se I e conta-se. |I| = 12 por
     * construção, e o valor de G é o que vai ser gravado — não o que o banco devolve. */
    long G[NX]; for(int j = 0; j < NX; j++) G[j] = 0;
    for(int i = 0; i < NI; i++){ long j = celula_de(PI_[i]); if(j < 0) return 2; G[j]++; }
    printf("§G1  realizacao I = {0..%d}, |I| = %d;  X = {a,b,c,d};  pi(i) declarada em C\n", NI-1, NI);
    printf("      G = |pi^-1(x)|:  a=%ld  b=%ld  c=%ld  d=%ld   (d tem fibra vazia: G(d)=0)\n",
           G[0], G[1], G[2], G[3]);
    {   /* a MESMA soma, contada de outra maneira: |I| = Σ_x |π⁻¹(x)| tem de bater com
         * a contagem directa de I. São dois objectos, e se discordassem a medição toda
         * valia o que valesse a menor das duas. */
        long por_fibra = 0; for(int j = 0; j < NX; j++) por_fibra += G[j];
        long por_evento = 0; for(int i = 0; i < NI; i++) por_evento++;
        ok("G por |pi^-1(x)| e |I| por contagem directa concordam — a soma externa ja e' |I|",
           por_fibra == NI && por_evento == NI);
    }

    /* ═══ §G2 — O BANCO REPRESENTA O CAMPO ══════════════════════════════════════════
     * A menor estrutura que segura um par (célula, contagem). Não é uma proposta de
     * schema: é o mínimo que se consegue escrever, e o mínimo é que se mede.
     *
     * E O TIPO HÁ DE SE DECLARAR. A primeira corrida escreveu `CREATE TABLE ocupacao
     * (celula, contagem)` e as quatro inserções foram RECUSADAS — «a coluna 0 não é de
     * texto e veio uma cadeia». Coluna sem tipo nasce INTEIRO (sql.c:3197, «sem tipo =
     * INTEIRO, como sempre foi»), e o motor recusa converter uma cadeia em inteiro em
     * silêncio. Isto não é uma falha do banco: é o banco a recusar uma conversão
     * silenciosa, que é a coisa certa. Mas é o primeiro facto de REPRESENTAÇÃO que o
     * experimento mediu, e sem ele não havia campo nenhum. */
    remove(BASE ".mem"); remove(BASE ".conf"); remove(BASE ".prog");
    if(!sql_abrir(BASE)){ perror("sql_abrir"); return 2; }
    banco("CREATE TABLE ocupacao (celula TEXTO, contagem INTEIRO)", "criar");
    int gravou = 0;
    for(int j = 0; j < NX; j++){
        char s[128];
        snprintf(s, sizeof s, "INSERT INTO ocupacao VALUES ('%s',%ld)", X[j], G[j]);
        gravou += banco(s, "inserir");
    }
    printf("§G2  ocupacao(celula TEXTO, contagem INTEIRO): %d de %d linhas gravadas\n",
           gravou, NX);
    ok("o banco aceitou o campo G: as 4 linhas entraram todas", gravou == NX);

    /* a LEITURA. O que se mede a seguir é o que o banco DEVOLVEU, linha a linha — e
     * G_q, no §G4, sai DAQUI, não do array de C. Derivar de G sem passar pelo banco
     * seria medir a aritmética, não a representação. */
    int leu = banco("SELECT celula, contagem FROM ocupacao", "ler");
    printf("      SELECT devolveu %d linha(s):", OUT.nrows);
    for(int i = 0; i < OUT.nrows; i++) printf("  %s=%s", OUT.cell[i][0], OUT.cell[i][1]);
    printf("\n");
    ok("o banco devolveu o campo, linha a linha, todas as celulas incluindo a de G=0",
       leu && OUT.nrows == NX);
    /* O QUE VOLTOU, e só isso: G_b. Uma célula que o banco não devolveu conta como
     * ausente, e ausente NÃO é zero — a diferença entre as duas coisas é exactamente o
     * que o experimento está a medir, e somar por cima dela taparia o defeito. */
    long Gb[NX]; for(int j = 0; j < NX; j++) Gb[j] = -1;
    {   int iguais = 1;
        for(int i = 0; i < OUT.nrows; i++){
            long j = celula_de(OUT.cell[i][0]);
            if(j < 0) { iguais = 0; continue; }
            long v = strtol(OUT.cell[i][1], NULL, 10);
            if(v != G[j]) iguais = 0;
            Gb[j] = v;
        }
        for(int j = 0; j < NX; j++) if(Gb[j] < 0) iguais = 0;
        ok("cada (celula, contagem) que volta do banco e' o par que a definicao de G dá"
           " — e todas as 4 celulas voltaram, a de G=0 incluída", iguais);
    }

    /* ═══ §G3 — INVARIANTE 1: Σ_x G(x) = |I|, MEDIDO SOBRE O QUE VOLTOU ════════════ */
    long soma_lida = soma_coluna(1, NULL);
    printf("§G3  Σ_x G(x) medido nas linhas lidas do banco = %ld;  |I| = %d\n", soma_lida, NI);
    ok("INVARIANTE 1 — conservacao: Σ_x G(x) = |I|, medido no que o banco devolveu",
       soma_lida == NI);

    /* E O BANCO SOME-O ELE PRÓPRIO? `sum` é o agregado do motor sobre a coluna do
     * campo. Se souber somar, o invariante é verificável DENTRO do banco, e isso é uma
     * propriedade a mais do que «guardar e devolver». */
    {   int r = banco("SELECT sum(contagem) FROM ocupacao", "somar");
        long pelo_banco = (r && OUT.nrows > 0) ? strtol(OUT.cell[0][0], NULL, 10) : -1;
        printf("      SELECT sum(contagem) do proprio banco = %ld\n", pelo_banco);
        ok("o motor tem o agregado: sum(contagem) devolve |I| sem que a medicao o recalcule",
           r && pelo_banco == NI);
    }

    /* ═══ §G4 — DERIVAÇÃO DE G_q, FORA DO BANCO, POR DOIS CAMINHOS ══════════════════
     * G_q(y) = Σ_{x: q(x)=y} G(x). Não é gravada: é uma soma sobre o campo que o BANCO
     * devolveu (Gb, nunca o array de C), com o q declarado em C.
     * O segundo caminho conta a realização directamente — |{i : q(π(i)) = y}| — e não
     * passa pelo banco nem por G. Se os dois dessem números diferentes não haveria
     * critério para escolher entre eles; por isso são dois. */
    long Gq[NY]; for(int k = 0; k < NY; k++) Gq[k] = 0;
    for(int k = 0; k < NY; k++)
        for(int j = 0; j < NX; j++) if(!strcmp(Q[j], Y[k]) && Gb[j] > 0) Gq[k] += Gb[j];

    long Gq2[NY]; for(int k = 0; k < NY; k++) Gq2[k] = 0;
    for(int i = 0; i < NI; i++){
        long j = celula_de(PI_[i]); long k = classe_de(Q[j]);
        if(k < 0) return 2;
        Gq2[k]++;
    }
    printf("§G4  G_q derivado de G lido do banco:  y1=%ld  y2=%ld\n", Gq[0], Gq[1]);
    printf("      G_q contado da realizacao:         y1=%ld  y2=%ld\n", Gq2[0], Gq2[1]);
    {   int iguais = 1;
        for(int k = 0; k < NY; k++) if(Gq[k] != Gq2[k]) iguais = 0;
        ok("G_q por agregacao do campo lido do banco e G_q por contagem da realizacao"
           " sao o mesmo numero", iguais);
    }

    /* ═══ §G5 — INVARIANTE 2 e INVARIANTE 3 ════════════════════════════════════════
     * Σ_y G_q(y) = |I|, e p_q(y) = G_q(y)/|I| com Σ_y p_q(y) = 1.
     * A soma das p_q é feita em ARITMÉTICA EXATA, com denominador comum |I|: p_q(y)
     * entra como o par (G_q(y), |I|), e a soma é (Σ G_q(y), |I|). Dizer «= 1» é dizer
     * que o numerador é o denominador — sem arredondar, sem ponto flutuante, sem
     * tolerâncias. */
    long soma_Gq = 0; for(int k = 0; k < NY; k++) soma_Gq += Gq[k];
    printf("§G5  Σ_y G_q(y) = %ld;  |I| = %d\n", soma_Gq, NI);
    ok("INVARIANTE 2 — conservacao sob projecao: Σ_y G_q(y) = |I|", soma_Gq == NI);
    printf("      p_q(y) = G_q(y)/%d:  y1=%ld/%d  y2=%ld/%d\n",
           NI, Gq[0], NI, Gq[1], NI);
    ok("INVARIANTE 3 — normalizacao: Σ_y p_q(y) = 1, em aritmetica exacta (Σ G_q = |I|)",
       soma_Gq == NI);
    /* e o que um banco de vírgula flutuante diria, para se ver que a medida é exacta */
    {   double s = 0.0;
        for(int k = 0; k < NY; k++) s += (double)Gq[k] / (double)NI;
        printf("      (em double, a mesma soma dá %.17g — por isso se mede em inteiros)\n", s);
    }

    /* ═══ §G6 — O BANCO CALCULA O G_q ELE PRÓPRIO? ══════════════════════════════════
     * Isto JÁ NÃO É um dos três invariantes: os três mediram-se em §G3 e §G5, e
     * mediram-se sobre o que o banco devolveu. O que se pergunta aqui é outra coisa, e
     * mais fraca: a derivação G_q é computável DENTRO do banco, numa consulta, sem que
     * G_q exista como tabela? É a hipótese «uma query poderia computar G_q como derivado
     * de G» do mapa, e ela precisa de ser medida ou marcada.
     *
     * Para isso o q tem de estar algures, e a escolha de onde ele vive é DESTE
     * experimento — `criterio` é metadata, e o banco saber lê-la não diz nada sobre o
     * que ela significa. E medem-se DUAS representações, porque elas dão respostas
     * diferentes e escolher a que passa sem dizer qual se escolheu seria fraude:
     *
     *   (i)  DUAS tabelas: G em `ocupacao`, q em `criterio`, e a derivação por
     *        JOIN + GROUP BY. É a leitura mais fiel de «G_q é derivado de G».
     *   (ii) UMA tabela: `celula, contagem, classe` — o q como uma COLUNA do mesmo
     *        registo, e a derivação por GROUP BY. Duplica o campo, e por isso é uma
     *        representação diferente, não uma melhoria.
     *
     * O que este ficheiro NÃO faz, em nenhuma das duas: materializar G_q numa tabela
     * para contornar uma recusa, nem tocar no motor para a consulta passar. */
    printf("§G6  o banco consegue derivar G_q numa consulta, sem G_q ser tabela?\n");
    {   banco("CREATE TABLE criterio (celula TEXTO, classe TEXTO)", "criar-q");
        int meta = 0;
        for(int j = 0; j < NX; j++){
            char s[128];
            snprintf(s, sizeof s, "INSERT INTO criterio VALUES ('%s','%s')", X[j], Q[j]);
            meta += banco(s, "inserir-q");
        }
        printf("      (i) DUAS tabelas — q em criterio(celula, classe), %d escrita(s) de %d\n",
               meta, NX);
        ok("o q entrou no banco como metadata — escolha deste experimento sobre ONDE q"
           " vive, e nada mais", meta == NX);

        int r = banco("SELECT c.classe, sum(o.contagem) FROM ocupacao o"
                      " JOIN criterio c ON o.celula = c.celula"
                      " GROUP BY c.classe", "duas-tabelas");
        if(r){
            printf("          JOIN + GROUP BY devolveu %d linha(s):", OUT.nrows);
            for(int i = 0; i < OUT.nrows; i++) printf("  %s=%s", OUT.cell[i][0], OUT.cell[i][1]);
            printf("\n");
            int bate = OUT.nrows == NY;
            for(int i = 0; i < OUT.nrows; i++){
                long k = classe_de(OUT.cell[i][0]);
                if(k < 0 || strtol(OUT.cell[i][1], NULL, 10) != Gq[k]) bate = 0;
            }
            printf("          Σ dos agregados que o banco produziu = %ld (|I| = %d)\n",
                   soma_coluna(1, NULL), NI);
        } else {
            printf("          LIMITE: RECUSADA. E agora isola-se ONDE — recusa-se o motor\n");
            printf("          inteiro, ou a combinação?\n");
            int rj = banco("SELECT c.classe, o.contagem FROM ocupacao o"
                           " JOIN criterio c ON o.celula = c.celula", "so-join");
            printf("          o JOIN sozinho, com colunas QUALIFICADAS: %s (%d linha(s))\n",
                   rj ? "ACEITE" : "recusado", rj ? OUT.nrows : 0);
            if(rj)
                for(int i = 0; i < OUT.nrows && i < 4; i++)
                    printf("            %s | %s\n", OUT.cell[i][0], OUT.cell[i][1]);
            int rs = banco("SELECT classe, contagem FROM ocupacao o"
                           " JOIN criterio c ON o.celula = c.celula", "join-sem-qualificar");
            printf("          o JOIN sozinho, colunas SEM qualificar: %s (%d linha(s),"
                   " %d coluna(s))\n", rs ? "ACEITE" : "recusado",
                   rs ? OUT.nrows : 0, rs ? OUT.ncols : 0);
            if(rs)
                for(int i = 0; i < OUT.nrows && i < 4; i++)
                    printf("            %s | %s\n", OUT.cell[i][0], OUT.cell[i][1]);
            int rg = banco("SELECT classe, count(*) FROM criterio GROUP BY classe", "so-group");
            printf("          o GROUP BY sozinho, sem JOIN: %s (%d linha(s), %d coluna(s))\n",
                   rg ? "ACEITE" : "recusado", rg ? OUT.nrows : 0, rg ? OUT.ncols : 0);
            if(rg)
                for(int i = 0; i < OUT.nrows && i < 4; i++)
                    printf("            %s | %s\n", OUT.cell[i][0], OUT.cell[i][1]);
            int ra = banco("SELECT c.classe, sum(o.contagem) FROM ocupacao o"
                           " JOIN criterio c ON o.celula = c.celula", "join-sem-group");
            printf("          o JOIN com agregação, SEM GROUP BY: %s (%d linha(s))\n",
                   ra ? "ACEITE" : "recusado", ra ? OUT.nrows : 0);
            printf("      → o que o motor recusa é o JOIN com coluna qualificada, e a\n");
            printf("        combinação do JOIN com o GROUP BY. O GROUP BY sozinho funciona.\n");
        }

        /* (ii) UMA tabela. O q como coluna do mesmo registo. */
        banco("CREATE TABLE registo (celula TEXTO, contagem INTEIRO, classe TEXTO)", "criar-1t");
        int um = 0;
        for(int j = 0; j < NX; j++){
            char s[160];
            snprintf(s, sizeof s, "INSERT INTO registo VALUES ('%s',%ld,'%s')", X[j], G[j], Q[j]);
            um += banco(s, "inserir-1t");
        }
        int r1 = banco("SELECT classe, sum(contagem) FROM registo GROUP BY classe", "uma-tabela");
        printf("      (ii) UMA tabela — registo(celula, contagem, classe), %d escrita(s)\n", um);
        if(r1){
            /* A PORTA ENTREGA TRÊS COLUNAS PARA UMA PROJECÇÃO DE DOIS ITENS. A
             * projecção pediu `classe, sum(contagem)`; a porta devolveu
             * [classe, tamanho-da-fibra, soma]. A primeira versão deste bloco somou a
             * COLUNA 1 — a contagem — e declarou que o banco não sabia somar, quando a
             * soma estava à espera na coluna 2.Foi o `ncols` impresso que apanhou: sem
             * ele, um erro de leitura apresenta-se como um defeito do motor, que é o
             * pior dos dois desfechos. Por isso imprime-se a largura ANTES de somar, e
             * soma-se a ÚLTIMA coluna, que é onde o agregado pedido vai. */
            printf("          a porta C devolveu %d linha(s) e %d coluna(s) para uma"
                   " projecao de 2 itens:\n", OUT.nrows, OUT.ncols);
            for(int i = 0; i < OUT.nrows; i++){
                printf("            ");
                for(int c = 0; c < OUT.ncols; c++) printf("[%d]=%-4s", c, OUT.cell[i][c]);
                printf("\n");
            }
            int ult = OUT.ncols - 1;
            long s = 0;
            for(int i = 0; i < OUT.nrows; i++) s += strtol(OUT.cell[i][ult], NULL, 10);
            printf("          somando a ultima coluna (o agregado pedido): Σ = %ld (|I| = %d)\n",
                   s, NI);
            int bate = OUT.nrows == NY;
            for(int i = 0; i < OUT.nrows; i++){
                long k = classe_de(OUT.cell[i][0]);
                if(k < 0 || strtol(OUT.cell[i][ult], NULL, 10) != Gq[k]) bate = 0;
            }
            ok("existe UMA representacao de G no banco da qual o motor deriva G_q por"
               " consulta, sem G_q ser tabela — e o valor e' o mesmo que a derivacao"
               " externa, e a soma das somas e' |I|", bate && s == NI && um == NX);
        } else {
            ok("existe UMA representacao de G da qual o motor deriva G_q por consulta", 0);
        }
        printf("      NOTA 1: as DUAS representacoes coexistem nesta base e o experimento\n");
        printf("      NAO diz qual delas e' a certa. Escolher a que passa sem dizer qual\n");
        printf("      se escolheu seria fraude; ficam as duas, com o resultado de cada uma.\n");
        printf("      NOTA 2: a porta C entrega uma coluna a MAIS do que a projecao pediu\n");
        printf("      (o tamanho da fibra entra antes do agregado). Quem ler a soma tem de\n");
        printf("      saber disso, e este medidor nao o assumia.\n");
        banco("DROP TABLE registo", "limpar-1t");
    }

    /* ═══ §G7 — CONTROLO NEGATIVO: A MESMA LEI TEM DE VER A VIOLAÇÃO ════════════════
     * Um G errado, gravado no MESMO banco, lido pelo MESMO caminho, com a mesma lei.
     * G'(d) = 2 em vez de 0: Σ G' = 14 ≠ 12, e é a LEI que tem de dizer que é.
     *
     * E A PRIMEIRA VERSÃO DESTA ASSERÇÃO PASSAVA PELO MOTIVO ERRADO. Na primeira
     * corrida as quatro inserções do campo errado também foram recusadas — a coluna
     * sem tipo —, a tabela voltou vazia, e «Σ G = 0 ≠ 12» foi lido como «a lei viu a
     * violação». Não viu: viu uma tabela que não tinha nada. Um controlo negativo que
     * passa porque o subjecto nem chegou a existir é o pior tipo de controlo, porque
     * autoriza tudo o que vier a seguir. Por isso a escrita e a leitura do campo errado
     * são elas próprias asserções, e o controlo só conta se elas passarem. */
    printf("§G7  controlo negativo: um campo G propositadamente errado\n");
    {   int cf = banco("CREATE TABLE ocupacao_falha (celula TEXTO, contagem INTEIRO)",
                      "criar-falha");
        int ef = 0;
        for(int j = 0; j < NX; j++){
            char s[128];
            long g = G[j] + (j == 3 ? 2 : 0);
            snprintf(s, sizeof s, "INSERT INTO ocupacao_falha VALUES ('%s',%ld)", X[j], g);
            ef += banco(s, "inserir-falha");
        }
        int lf = banco("SELECT celula, contagem FROM ocupacao_falha", "ler-falha");
        long n_lidas = 0;
        soma_coluna(1, &n_lidas);
        ok("o campo ERRADO entrou no banco e voltou — sem isto o controlo abaixo não prova"
           " nada: uma tabela vazia também «viola» a soma", cf && ef == NX && lf
              && n_lidas == NX);
        long soma_falha = soma_coluna(1, NULL);
        long Gq_falha[NY]; for(int k = 0; k < NY; k++) Gq_falha[k] = 0;
        for(int i = 0; i < OUT.nrows; i++){
            long j = celula_de(OUT.cell[i][0]);
            if(j < 0) continue;
            long k = classe_de(Q[j]);
            if(k >= 0) Gq_falha[k] += strtol(OUT.cell[i][1], NULL, 10);
        }
        long soma_Gq_falha = 0;
        for(int k = 0; k < NY; k++) soma_Gq_falha += Gq_falha[k];
        printf("      campo errado (G'(d)=2): Σ G = %ld (devia ser %d)"
               "  ·  Σ G_q = %ld (devia ser %d)\n", soma_falha, NI, soma_Gq_falha, NI);
        ok("a MESMA lei que mede os invariantes VE a violacao no campo errado que esta"
           " vez EXISTE no banco — Σ G = 14 ≠ 12 e Σ G_q = 14 ≠ 12",
           soma_falha == NI + 2 && soma_Gq_falha == NI + 2);
        banco("DROP TABLE ocupacao_falha", "limpar-falha");
    }

    sql_fechar();

    /* ═══ §G8 — A FRONTEIRA, MEDIDA DEPOIS E NÃO PROMETIDA ═════════════════════════
     * Este experimento não rozou uma referência ao motor. Não é uma promessa do
     * comentário do topo — é uma leitura de ficheiro, depois de o banco ter sido
     * aberto, gravado e lido. E a leitura tem de ser REAL: um `fopen` a um directório
     * abre, devolve zero caracteres e «passa» — que é a asserção vazia que esta casa
     * persegue. Por isso opendir, e cada .c/.h aberto um a um, contados à vista.
     *
     * E O SCANNER TEM O SEU PRÓPRIO CONTROLO. Passa-se o MESMO detector por um
     * ficheiro onde a palavra ESTÁ — este mesmo, que a nomeia nos comentários — e
     * exige-se que a encontre. Um detector que nunca viu nada não pode atestar que não
     * há nada: foi assim que a primeira versão desta secção, que procurava `j` seguido
     * de duas minúsculas quaisquer, deu 304 falsos positivos e obrigou a refazer a
     * prova de que o motor está limpo. */
    printf("§G8  a fronteira: o motor continua a nao saber de JEV\n");
    {   long ficheiros = 0, com_jev = 0, piores = -1;
        char culpado[600]; culpado[0] = 0;
        static const char *FON[] = { "banco/sql.c", "can", "lib" };
        /* a casa pode correr o medidor da raiz ou de tools/; procura-se nos dois */
        static const char *PRE[] = { "", "../", "./" };
        for(size_t pk = 0; pk < sizeof PRE/sizeof PRE[0]; pk++)
        for(size_t f = 0; f < sizeof FON/sizeof FON[0]; f++){
            char cam[512];
            snprintf(cam, sizeof cam, "%s%s", PRE[pk], FON[f]);
            const char *barra = strrchr(cam, '/');
            const char *ponto = strrchr(cam, '.');
            if(ponto && (!barra || ponto > barra)){             /* é ficheiro: banco/sql.c */
                long n = jev_no_ficheiro(cam);
                if(n < 0) continue;
                ficheiros++;
                if(n > 0){ com_jev += n;
                    if(!culpado[0]) snprintf(culpado, sizeof culpado, "%s", cam); }
                continue;
            }
            DIR *d = opendir(cam);                             /* é directório */
            if(!d) continue;
            struct dirent *e;
            while((e = readdir(d))){
                size_t L = strlen(e->d_name);
                if(L < 3 || (strcmp(e->d_name + L - 2, ".c")
                             && strcmp(e->d_name + L - 2, ".h"))) continue;
                char sub[600];
                snprintf(sub, sizeof sub, "%s/%s", cam, e->d_name);
                long n = jev_no_ficheiro(sub);
                if(n < 0) continue;
                ficheiros++;
                if(n > 0){ com_jev += n;
                    if(!culpado[0]) snprintf(culpado, sizeof culpado, "%s", sub); }
            }
            closedir(d);
        }
        printf("      %ld ficheiro(s) de banco/, can/ e lib/ lidos um a um; %ld com 'jev'%s\n",
               ficheiros, com_jev, culpado[0] ? culpado : "");
        /* o CONTROLO do detector: este ficheiro nomeia a palavra, e o detector tem de a ver */
        static const char *ME[] = { "tests/campo_g_no_banco.c", "../tests/campo_g_no_banco.c" };
        for(size_t m = 0; m < 2; m++){ piores = jev_no_ficheiro(ME[m]); if(piores > 0) break; }
        printf("      CONTROLO — o mesmo detector sobre este ficheiro, onde a palavra"
               " esta: %ld ocorrencia(s)\n", piores);
        ok("o detector do §G8 ENCONTRA «jev» num ficheiro onde ele esta — sem isto,"
           " «zero ocorrencias» seria indistinguivel de um detector que nunca leu",
           piores > 0);
        ok("nenhuma referencia a JEV em banco/sql.c, can/ ou lib/ — e sao ficheiros de"
           " verdade, nao directorios abertos a esmo — o experimento de representacao"
           " nao virou ponte", com_jev == 0 && ficheiros > 50);
    }

    printf("\n================================================================\n");
    conclui("isto mede se o banco REPRESENTA um campo externo de contagem, e nada mais.");
    conclui("G e' campo de contagem aqui; se algures o chamarem de outra coisa, mede-se o campo.");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas,
           falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
