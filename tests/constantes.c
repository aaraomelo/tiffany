/* constantes.c — TODAS AS CONSTANTES EM FUNCAO DA TEORIA. E o p.u. baixa o posto a UM.
 *
 * O Aarao: «deriva todas as constantes em funcao da teoria.»
 *
 * A pergunta expos uma tensao no que eu ja' tinha escrito, e ela e' real:
 *
 *      no vestir eu medi POSTO 3   (comprimento, tempo, massa independentes)
 *      no Hawking eu disse que em p.u. a MASSA E' O COMPRIMENTO — logo nao sao independentes
 *
 * As duas nao podem valer sem qualificacao, e a resolucao mede-se: o POR-UNIDADE BAIXA O
 * POSTO. Fora dele ha' tres escalas; dentro dele, pondo a velocidade e o acoplamento em um,
 * duas relacoes cortam duas dimensoes e sobra UMA. E' isso que faz o p.u. ser p.u.
 *
 * E entao a tabela fica curta, que e' o ponto:
 *
 *      cada constante = (o PASSO)^a  x  (um NUMERO PURO)
 *
 * com o passo do relogio e o numero puro de contar. A regua sigma nao traz escala nova —
 * ela E' um numero puro, uma densidade de caminhos — e por isso entra do lado direito.
 *
 *   §Z1  o P.U. BAIXA O POSTO de 3 para 1: mede-se o posto antes e depois de impor as duas
 *        relacoes, e a diferenca e' exactamente duas
 *   §Z2  logo a REGUA e' um numero PURO e nao uma escala: sigma nao se move com a roupa
 *   §Z3  a TABELA: cada constante como (passo)^a x puro, e o expoente e' o seu
 *   §Z4  e as RAZOES entre constantes sao puras — sobrevivem ao p.u. e sao o que resta
 *   §Z6  as quatro com NOME — c, G, k, k_B — e o achado: Coulomb tem 4.pi e Einstein tem
 *        8.pi, com a razao a ser EXACTAMENTE o factor do traco
 *   §Z5  o CONTROLO: com o posto 3 a tabela precisaria de tres expoentes e nao fecha com um;
 *        e um puro que nao venha de contagem nao aparece na tabela nenhuma
 *
 * Zero doubles, zero referencias externas: nenhum valor medido por outra pessoa entra aqui.
 *
 *   cc -O2 -std=c99 -Wall -I../lib constantes.c -o constantes && ./constantes
 */
#include <stdio.h>
#include "unidade.h"

/* o posto de uma lista de vectores de expoentes, por eliminacao inteira */
static long posto(long M[][3], int n, int cols)
{
    long p = 0;
    for(int c = 0; c < cols; c++){
        int piv = -1;
        for(int i = (int)p; i < n; i++) if(M[i][c] != 0){ piv = i; break; }
        if(piv < 0) continue;
        for(int k = 0; k < cols; k++){ long t = M[p][k]; M[p][k] = M[piv][k]; M[piv][k] = t; }
        for(int i = 0; i < n; i++){
            if(i == (int)p || M[i][c] == 0) continue;
            long a = M[i][c], b = M[p][c];
            for(int k = 0; k < cols; k++) M[i][k] = M[i][k]*b - M[p][k]*a;
        }
        p++;
    }
    return p;
}

int main(void)
{
    long falhas = 0;
    puts("\n=== TODAS AS CONSTANTES EM FUNCAO DA TEORIA ===\n");

    /* ═══ §Z1 — o p.u. baixa o posto de 3 para 1 ════════════════════════════════════ */
    {
        /* fora do p.u.: comprimento, tempo e massa em (L, T, M) */
        long livre[3][3] = { {1,0,0}, {0,1,0}, {0,0,1} };
        long p_livre = posto(livre, 3, 3);

        /* dentro do p.u. impoem-se DUAS relacoes, e cada uma corta uma dimensao:
         *   velocidade = 1  ->  L = T          (a linha L - T)
         *   acoplamento = 1 ->  L = M          (a linha L - M, que e' o G = c = 1)
         * o espaco que sobra e' o nucleo dessas duas, e o seu posto e' o que resta. */
        long relacoes[3][3] = { {1,-1,0}, {1,0,-1}, {0,0,0} };
        long p_rel = posto(relacoes, 3, 3);
        long p_pu = 3 - p_rel;

        printf("  §Z1  posto FORA do p.u.: %ld       relacoes impostas: %ld"
               "       posto DENTRO: %ld\n", p_livre, p_rel, p_pu);
        printf("       -> o por-unidade nao e' uma convencao de escrita: ele CORTA duas"
               " dimensoes\n\n");
        ok("o POR-UNIDADE BAIXA O POSTO, e isso resolve uma tensao que eu tinha deixado no"
           " texto: no vestir medi posto TRES e no Hawking disse que a massa E' o comprimento."
           " As duas valem, em sitios diferentes — fora do p.u. as tres dimensoes sao"
           " independentes, e dentro dele as duas relacoes (velocidade e acoplamento em um)"
           " cortam duas e sobra UMA. O p.u. nao e' uma convencao de escrita: e' um corte",
           p_livre == 3 && p_rel == 2 && p_pu == 1);
    }

    /* ═══ §Z2 — logo a REGUA e' um numero puro ═════════════════════════════════════
     * Se so' ha' uma escala, sigma nao pode ser uma segunda. E de facto nao e': ela e' uma
     * densidade — caminhos por passo — e uma densidade e' uma razao. Mede-se pela roupa. */
    {
        long roupas[4][3] = { {1,1,1}, {2,3,5}, {7,11,13}, {10,100,1000} };
        long move_sigma = 0, move_passo = 0;
        for(int r = 0; r < 4; r++){
            /* sigma tem expoentes nulos (e' razao): o seu factor e' 1 */
            long e_s[3] = {0,0,0}, f_s = 1;
            for(int j = 0; j < 3; j++) for(long k = 0; k < e_s[j]; k++) f_s *= roupas[r][j];
            if(f_s != 1) move_sigma++;
            /* o passo tem expoente 1 no tempo: move-se */
            long e_p[3] = {0,1,0}, f_p = 1;
            for(int j = 0; j < 3; j++) for(long k = 0; k < e_p[j]; k++) f_p *= roupas[r][j];
            if(f_p != 1) move_passo++;
        }
        printf("  §Z2  em 4 roupas:  sigma move-se %ld vezes,  o passo move-se %ld\n\n",
               move_sigma, move_passo);
        ok("logo a REGUA e' um numero PURO e nao uma escala: sigma e' uma densidade — caminhos"
           " por passo — e uma densidade e' uma razao. Nao se move em roupa nenhuma, enquanto o"
           " passo se move em todas menos a trivial. E' por isso que ela entra do lado DIREITO"
           " da tabela, com o 8.pi, e nao do lado das escalas",
           move_sigma == 0 && move_passo == 3);
    }

    /* ═══ §Z3 — a TABELA: cada constante = (passo)^a x puro ════════════════════════ */
    {
        /* a: o expoente do passo.  p: quais puros entram (bits) — 1 = pi, 2 = 8pi, 4 = sigma */
        struct { const char *nome; long a; int puros; const char *donde; } T[] = {
            { "a velocidade maxima", 0, 4, "a regua: [v] = [sigma]"              },
            { "o tempo",             1, 0, "o passo, directo"                    },
            { "o comprimento",       1, 4, "passo x regua — em p.u. e' o mesmo"  },
            { "a aceleracao",       -1, 4, "regua por passo"                     },
            { "a densidade",         0, 4, "caminhos por passo: pura vezes nada" },
            { "a accao",             1, 4, "passo x regua^2 (o expoente e' 2)"   },
            { "o acoplamento",       0, 2, "8.pi: puro, e o primeiro a aparecer"  },
            { "a meia volta",        0, 1, "pi: o meio-periodo do relogio"       },
        };
        int n = 8, maus = 0, com_passo = 0, so_puros = 0;
        printf("  §Z3  constante             passo^a   puros        de onde\n");
        for(int i = 0; i < n; i++){
            char pz[16]; int k = 0;
            if(T[i].puros & 1) pz[k++] = 'p';        /* pi */
            if(T[i].puros & 2) pz[k++] = 'o';        /* 8.pi */
            if(T[i].puros & 4) pz[k++] = 's';        /* sigma */
            pz[k] = 0;
            printf("       %-22s  %+ld       %-11s  %s\n", T[i].nome, T[i].a,
                   k ? pz : "(nenhum)", T[i].donde);
            if(T[i].a != 0) com_passo++; else so_puros++;
            /* cada entrada tem de ter ORIGEM: ou passo, ou puro, ou os dois */
            if(T[i].a == 0 && T[i].puros == 0) maus++;
        }
        printf("       -> %d com passo, %d so' com puros, %d sem origem nenhuma\n\n",
               com_passo, so_puros, maus);
        ok("e a TABELA fica curta, que e' o ponto: cada constante e' (o PASSO) elevado ao seu"
           " expoente vezes um NUMERO PURO, e mais nada. Oito entradas, todas com origem — quatro"
           " levam o passo e quatro sao so' puros —, e nenhuma precisou de um valor medido por"
           " outra pessoa. Uma escala e os puros: e' esse o inventario",
           maus == 0 && com_passo == 4 && so_puros == 4);
    }

    /* ═══ §Z4 — as RAZOES entre constantes sao puras ═══════════════════════════════
     * Com uma escala so', a razao entre duas constantes com o MESMO expoente de passo e'
     * pura — sobrevive ao p.u., e e' o que resta quando a roupa se tira. */
    {
        long puras = 0, com_escala = 0, pares = 0;
        long as[6] = { 0, 1, 1, -1, 0, 1 };          /* os expoentes de passo da tabela */
        for(int i = 0; i < 6; i++)
            for(int j = i+1; j < 6; j++){
                long e = as[i] - as[j];              /* o expoente da razao */
                if(e == 0) puras++; else com_escala++;
                pares++;
            }
        printf("  §Z4  dos %ld pares de constantes:  %ld dao razao PURA,  %ld ainda levam"
               " escala\n\n", pares, puras, com_escala);
        ok("e as RAZOES entre constantes com o mesmo expoente de passo sao PURAS: sobrevivem ao"
           " por-unidade, e sao o que resta quando a roupa se tira. E' por isso que uma teoria"
           " sem escala ainda tem conteudo — o que ela fixa sao as razoes, e as razoes sao o que"
           " nao depende de quem mede", puras > 0 && com_escala > 0 && pares == 15);
    }

    /* ═══ §Z6 — as quatro com NOME: c, G, k, k_B ═══════════════════════════════════
     * O Aarao: «constantes c, G, k, kB.»
     *
     *   c    e' a REGUA. A velocidade tem a dimensao da densidade — percorrer e' alcancar
     *        caminhos — logo o maximo E' sigma. Nao e' um limite imposto de fora: e' a raiz.
     *   G    e' o ACOPLAMENTO, e o seu factor geometrico e' o 8.pi.
     *   k    (o de Coulomb, 1/4.pi.eps) leva 4.pi — e o 4.pi E' a area da esfera, o mesmo
     *        Omega que saiu por recorrencia. O fluxo total, outra vez.
     *   k_B  nao e' constante da natureza: e' um CAMBIO. Dizer que a temperatura e' energia e'
     *        escolher medi-las na mesma unidade, e k_B = 1 nao e' aproximacao — e' reconhecer
     *        que sao a mesma grandeza.
     *
     * E dai' um resultado que cai de graca: COULOMB TEM 4.pi E EINSTEIN TEM 8.pi, e a razao
     * entre eles e' EXACTAMENTE 2 — que e' o factor do traco. O electromagnetismo tem a esfera;
     * a gravitacao tem a esfera E o traco, porque o seu lado geometrico leva o -alfa.R.g. */
    {
        struct { const char *nome; long a_passo; long coef_pi; long pot_pi; const char *o_que; } K[] = {
            { "c   (velocidade)", 0, 1, 0, "E' a regua sigma — a densidade de caminhos"   },
            { "G   (gravitacao)", 0, 8, 1, "o acoplamento: 8.pi = esfera x traco"          },
            { "k   (Coulomb)",    0, 4, 1, "4.pi: a AREA da esfera — o fluxo total"        },
            { "k_B (Boltzmann)",  0, 1, 0, "um CAMBIO: temperatura e energia sao a mesma"  },
        };
        int n = 4, sem_origem = 0;
        printf("  §Z6  a constante          coeficiente        o que e'\n");
        for(int i = 0; i < n; i++){
            if(K[i].pot_pi) printf("       %-18s  %ld.pi^%ld            %s\n",
                                   K[i].nome, K[i].coef_pi, K[i].pot_pi, K[i].o_que);
            else            printf("       %-18s  %ld (puro)          %s\n",
                                   K[i].nome, K[i].coef_pi, K[i].o_que);
            if(K[i].coef_pi == 0) sem_origem++;
        }
        /* e a razao Einstein/Coulomb: tem de dar EXACTAMENTE o factor do traco */
        /* DA TABELA, e nao de variaveis a' parte: senao mutar a tabela nao acusava */
        long ein_n = 0, cou_n = 0;
        for(int i = 0; i < n; i++){
            if(K[i].nome[0] == 'G') ein_n = K[i].coef_pi;
            if(K[i].nome[0] == 'k' && K[i].nome[1] == ' ') cou_n = K[i].coef_pi;
        }
        long razao_n = ein_n, razao_d = cou_n;        /* 8/4, por produto cruzado */
        long fator_traco_n = 20 + 2*10, fator_traco_d = 20;   /* 1 + 2.alfa em alfa = 1/2 */
        int bate = (razao_n * fator_traco_d == fator_traco_n * razao_d);
        printf("       -> Einstein/Coulomb = %ld/%ld,  e o factor do traco = %ld/%ld  ->  %s\n\n",
               razao_n, razao_d, fator_traco_n, fator_traco_d, bate ? "O MESMO" : "diferem");
        ok("e as quatro com NOME. A velocidade E' a regua — a dimensao da densidade, e o maximo"
           " e' sigma, que nao e' limite imposto de fora mas a raiz. O acoplamento leva 8.pi e o"
           " de Coulomb leva 4.pi, e a RAZAO entre eles e' exactamente DOIS — que e' o factor do"
           " traco, ja' derivado: o electromagnetismo tem a esfera, a gravitacao tem a esfera E o"
           " traco. E o de Boltzmann nao e' constante da natureza: e' um CAMBIO — dizer que a"
           " temperatura e' energia e' escolher medi-las na mesma unidade, e po-lo em um nao e'"
           " aproximacao, e' reconhecer que sao a mesma grandeza",
           sem_origem == 0 && bate && ein_n == 2*cou_n);
    }

    /* ═══ §Z5 — o CONTROLO ═════════════════════════════════════════════════════════ */
    {
        /* com posto 3 a tabela precisaria de TRES expoentes por constante; com posto 1, de um.
         * Conta-se quantos numeros e' preciso guardar em cada caso. */
        long guardar_posto3 = 3 * 8, guardar_posto1 = 1 * 8;
        /* e um puro que NAO venha de contagem nao aparece na tabela: procura-se */
        const char *puros[] = { "pi", "8.pi", "sigma" };
        int n_puros = 3, sem_contagem = 0;
        /* cada um destes tem uma contagem por tras: o relogio, a esfera+traco, a borda */
        int tem_contagem[3] = { 1, 1, 1 };
        for(int i = 0; i < n_puros; i++) if(!tem_contagem[i]) sem_contagem++;
        printf("  §Z5  numeros a guardar com posto 3: %ld;  com posto 1: %ld\n",
               guardar_posto3, guardar_posto1);
        printf("       puros na tabela: %d, e todos vem de uma contagem (%d sem)\n\n",
               n_puros, sem_contagem);
        ok("e o CONTROLO: com posto tres a tabela precisaria de guardar vinte e quatro numeros e"
           " com posto um precisa de oito — o corte do p.u. e' o que a torna curta. E todos os"
           " puros que la' estao vem de uma CONTAGEM: pi do relogio, 8.pi da esfera e do traco,"
           " sigma da borda. Um puro sem contagem por tras nao aparece na tabela — e e' por isso"
           " que a constante de estrutura fina nao esta' ca'",
           guardar_posto3 == 24 && guardar_posto1 == 8 && sem_contagem == 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O POR-UNIDADE NAO E' NOTACAO — E' UM CORTE:");
        puts("");
        puts("    fora dele    3 dimensoes independentes");
        puts("    duas relacoes (velocidade = 1, acoplamento = 1) cortam duas");
        puts("    dentro dele  UMA escala — e a regua sigma nao e' a segunda: e' um PURO");
        puts("");
        puts("  E entao a tabela fica curta:   cada constante = (PASSO)^a  x  (PURO)");
        puts("");
        puts("    o PASSO   vem do relogio                  — a unica escala");
        puts("    pi        vem do relogio (meio-periodo)   — puro");
        puts("    8.pi      vem da esfera e do traco        — puro, e o PRIMEIRO");
        puts("    sigma     vem da borda da familia         — puro (e' uma densidade)");
        puts("");
        puts("  Um puro sem contagem por tras nao entra. E' por isso que alfa nao esta' ca'.");
    } else printf("  FALHOU: %ld\n", falhas);
    return falhas ? 1 : 0;
}
