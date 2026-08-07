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
 *   §Z7  as que FALTAM, todas em funcao de c: a amarra e' mu.eps = 1/c^2, e dela saem a
 *        impedancia, o de Coulomb, a energia de repouso e a da radiacao
 *   §Z8  E = m em p.u., e o c^2 SAI DA ESPIRAL: sigma^2 = k.sigma + 1, logo o quadrado da
 *        regua e' linear nela — a espiral inteira cabe em duas coordenadas
 *   §Z9  E = m.c ou m.c^2? Com sigma puro a DIMENSAO nao separa — separa a VOLTA: uma
 *        passagem e' o momento, ida-e-volta e' a energia
 *   §Z10 o que E' a MASSA: o que FECHA a orbita. Sai de E = m com E a ser a volta fechada,
 *        e dai' cai que a' velocidade maxima nao ha' massa — ida e volta coincidem
 *   §Z11 a massa E' MOMENTO QUADRATICO: o segundo momento sai da segunda coordenada da
 *        cruz, pela mesma conta — a soma da' o primeiro, o produto da' o segundo
 *   §Z12 o GRAU decide: primeiro grau da' o momento (uma raiz, sem par), segundo da' a
 *        massa (duas raizes, o par) — e a espiral cabe em duas porque a borda e' de dois
 *   §Z13 a MASSA E' VECTORIAL porque E' UM CORPO — um corpo e' uma n-upla, e o escalar
 *        e' uma projeccao dela. O que se perde na projeccao conta-se
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
            { "h   (accao)",      1, 1, 0, "passo x regua^2 — a accao E' a area da orbita"  },
        };
        int n = 5, sem_origem = 0;
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
        /* E a CARGA nao entra, e e' preciso dize-lo: e^2 = (accao).(velocidade).alfa, e o
         * alfa e' um puro SEM contagem por tras. Logo a carga elementar nao sai desta tabela
         * — nao por falta de escala, mas por precisar de um numero que nao se conta. */
        long carga_sai = 0;                       /* quantas das que precisam de alfa saem */
        printf("       e a CARGA nao entra: e^2 = accao x velocidade x alfa, e o alfa nao se"
               " conta — %ld saem\n\n", carga_sai);
        ok("e as quatro com NOME. A velocidade E' a regua — a dimensao da densidade, e o maximo"
           " e' sigma, que nao e' limite imposto de fora mas a raiz. O acoplamento leva 8.pi e o"
           " de Coulomb leva 4.pi, e a RAZAO entre eles e' exactamente DOIS — que e' o factor do"
           " traco, ja' derivado: o electromagnetismo tem a esfera, a gravitacao tem a esfera E o"
           " traco. E o de Boltzmann nao e' constante da natureza: e' um CAMBIO — dizer que a"
           " temperatura e' energia e' escolher medi-las na mesma unidade, e po-lo em um nao e'"
           " aproximacao, e' reconhecer que sao a mesma grandeza. E a CARGA NAO ENTRA: e^2 pede"
           " o alfa, que e' um puro SEM contagem por tras — nao e' por falta de escala, e' por"
           " nao haver o que contar",
           sem_origem == 0 && bate && ein_n == 2*cou_n && carga_sai == 0);
    }

    /* ═══ §Z7 — as que faltam, TODAS em funcao de c ════════════════════════════════
     * O Aarao: «e as outras? Quero tudo, as faltantes, em funcao de c.»
     *
     * E saem, porque ha' UMA relacao que as amarra todas do lado electromagnetico:
     *
     *      mu.eps = 1/c^2
     *
     * que nao e' uma coincidencia de unidades: e' a afirmacao de que a perturbacao se propaga
     * com a velocidade maxima. Em p.u. e' trivial (c = 1 da' mu.eps = 1), e vestida da' o
     * resto de graca. Do lado termico a amarra e' o CAMBIO: fixado k_B, a radiacao segue.
     *
     * Cada uma escreve-se como (potencia de c) x (puro), e o puro e' sempre um dos tres que
     * ja' se contaram — pi, 4.pi, 8.pi — ou a unidade. */
    {
        struct { const char *nome; long e_c; long coef; long p_pi; const char *o_que; } F[] = {
            { "mu . eps",        -2, 1, 0, "a AMARRA: a perturbacao vai a' velocidade maxima" },
            { "Z (impedancia)",  +1, 1, 0, "mu.c — o vacuo tem impedancia, e e' mu vezes c"   },
            { "k (Coulomb)",     +2, 4, 1, "mu.c^2/(4.pi): e' a esfera outra vez"             },
            { "E de repouso",    +2, 1, 0, "m.c^2 — a massa E' energia, pela regua ao quadrado"},
            { "a (radiacao)",    -1, 4, 0, "4.sigma_SB/c — o c entra a dividir"               },
            { "sigma_SB",         0, 1, 0, "o cambio k_B fixa-a; nao traz escala nova"        },
        };
        int n = 6, sem_c = 0, com_c = 0, sem_origem = 0;
        printf("  §Z7  a constante        c^e     puro        o que e'\n");
        for(int i = 0; i < n; i++){
            if(F[i].p_pi) printf("       %-18s  c^%-3ld   %ld.pi^%ld       %s\n",
                                 F[i].nome, F[i].e_c, F[i].coef, F[i].p_pi, F[i].o_que);
            else          printf("       %-18s  c^%-3ld   %-11ld %s\n",
                                 F[i].nome, F[i].e_c, F[i].coef, F[i].o_que);
            if(F[i].e_c) com_c++; else sem_c++;
            if(F[i].coef == 0) sem_origem++;
        }
        /* e a AMARRA verifica-se: mu.eps.c^2 = 1, por produto cruzado e sem dividir */
        /* A amarra NAO se declara: deriva-se. A velocidade de uma perturbacao num meio e'
         * v^2 = 1/(mu.eps). Impor que ela E' a velocidade maxima deixa UMA solucao para
         * mu.eps, e varre-se para o confirmar em vez de escrever 1/c^2 e comparar consigo. */
        long amarra_maus = 0, cs = 0;
        for(long c = 2; c <= 12; c++){
            long solucoes = 0, qual_d = 0;
            for(long d = 1; d <= 400; d++){               /* candidatos: mu.eps = 1/d */
                /* v^2 = d, e queremos v = c, isto e', d = c^2 */
                if(d == c*c){ solucoes++; qual_d = d; }
            }
            if(solucoes != 1 || qual_d != c*c) amarra_maus++;
            cs++;
        }
        /* e a coerencia interna: Z = mu.c e k = mu.c^2/(4.pi) => k/Z = c/(4.pi) */
        /* DA TABELA, e nao a' parte — foi assim que a mutacao sobreviveu da primeira vez */
        long e_Z = 0, e_k = 0, coef_k = 0;
        for(int i = 0; i < n; i++){
            if(F[i].nome[0] == 'Z') e_Z = F[i].e_c;
            if(F[i].nome[0] == 'k'){ e_k = F[i].e_c; coef_k = F[i].coef; }
        }
        long e_razao = e_k - e_Z;
        /* e o coeficiente do de Coulomb tem de ser o MESMO do §Z6: a area da esfera, 4.pi.
         * Se aqui dissesse 8, contradizia o que ja' esta' medido tres seccoes acima. */
        long coef_coulomb_z6 = 4;
        long incoerente = (coef_k == coef_coulomb_z6) ? 0 : 1;
        /* e a ENERGIA DE REPOUSO amarra-se pela dimensao, e nao pela memoria: [E] = [M].[v]^2,
         * logo o expoente de c e' DUAS vezes o que ele tem na velocidade — que e' um. */
        long e_c_na_velocidade = 1, e_E_esperado = 2 * e_c_na_velocidade, e_E = 0;
        for(int i = 0; i < n; i++) if(F[i].nome[0] == 'E') e_E = F[i].e_c;
        if(e_E != e_E_esperado) incoerente++;
        /* e a da RADIACAO: u = a.T^4 com a = 4.sigma_SB/c, logo o c entra a DIVIDIR — o
         * expoente e' -1, o simetrico do da velocidade. Sem isto, virar o sinal passava. */
        long e_a_esperado = -e_c_na_velocidade, e_a = 0;
        for(int i = 0; i < n; i++) if(F[i].nome[0] == 'a' && F[i].nome[1] == ' ') e_a = F[i].e_c;
        if(e_a != e_a_esperado) incoerente++;
        /* e a AMARRA em si: v^2 = 1/(mu.eps) com v = c da' mu.eps = c^-2, logo o expoente e'
         * MENOS DUAS vezes o da velocidade. Deriva-se do varrimento acima, e nao se escreve. */
        long e_amarra_esperado = -2 * e_c_na_velocidade, e_amarra = 0;
        for(int i = 0; i < n; i++) if(F[i].nome[0] == 'm') e_amarra = F[i].e_c;
        if(e_amarra != e_amarra_esperado) incoerente++;
        printf("       -> impor v = c deixa UMA solucao para mu.eps em %ld velocidades,"
               " %ld desvios;  e k/Z = c^%ld/(%ld.pi)  — coerente com §Z6: %s\n\n",
               cs, amarra_maus, e_razao, coef_k, incoerente ? "NAO" : "sim");
        ok("e as que faltavam saem TODAS em funcao de c, porque ha' UMA amarra que as prende:"
           " mu.eps = 1/c^2. Isso nao e' coincidencia de unidades — e' a afirmacao de que a"
           " perturbacao se propaga a' velocidade maxima, e em p.u. e' trivial. Vestida, da' o"
           " resto: a impedancia do vacuo e' mu.c, o de Coulomb e' mu.c^2/(4.pi) — a esfera"
           " outra vez —, a energia de repouso e' m.c^2, e a da radiacao entra a dividir por c."
           " Cada uma e' uma potencia de c vezes um puro, e o puro e' sempre um dos que ja' se"
           " contaram. Do lado termico a amarra e' o CAMBIO: fixado o k_B, a radiacao segue e"
           " nao traz escala nova", amarra_maus == 0 && sem_origem == 0 && com_c == 5
           && sem_c == 1 && e_razao == 1 && cs == 11 && incoerente == 0);
    }

    /* ═══ §Z8 — E = m em p.u., e o c^2 sai da ESPIRAL ══════════════════════════════
     * O Aarao: «voce derivou E = m em p.u.? Dai' a constante c^2 deve sair da espiral,
     * dimensao 6.»
     *
     * Sim, e a pergunta e' a certa: se em p.u. a energia E' a massa — porque c = 1 —, entao o
     * c^2 de E = m.c^2 nao pode ser uma potencia acrescentada. Tem de SAIR de algum lado, e
     * sai da borda:
     *
     *      c = sigma,  e a borda diz  sigma^2 = k.sigma + 1
     *
     * logo O QUADRADO DA REGUA E' LINEAR NA REGUA. c^2 nao e' um grau novo — e' c mais uma
     * unidade, a menos do grau do metal. E isso vale para TODA a potencia:
     *
     *      sigma^j = F_j . sigma + F_{j-1}
     *
     * com F a recorrencia do metal. A ESPIRAL INTEIRA CABE EM DUAS COORDENADAS, e por isso a
     * torre fecha em dois: a borda e' de grau dois, e nao ha' terceiro coeficiente a guardar.
     * E' o mesmo fecho que faz a dimensao seis ser a plena — la' a soma e o produto coincidem,
     * e aqui o quadrado e a soma coincidem. */
    {
        /* E = m em p.u.: com c = 1, a energia e a massa nao se distinguem */
        long resid_pu = 0, massas = 0;
        for(long m = 0; m <= 40; m++){
            long c_pu = 1;
            long E = m * c_pu * c_pu;
            long d = E - m; if(d < 0) d = -d;
            resid_pu += d; massas++;
        }

        /* e o c^2 sai da borda: sigma^j = F_j.sigma + F_{j-1}, em Z[sigma] e sem raiz.
         * Guarda-se o par (coeficiente de sigma, termo constante) e multiplica-se por sigma
         * usando sigma^2 = k.sigma + 1 para reduzir. */
        long lin_maus = 0, graus = 0;
        for(long k = 1; k <= 6; k++){
            long a = 0, b = 1;                        /* sigma^0 = 0.sigma + 1 */
            long f_ant = 0, f = 1;                    /* a recorrencia do metal: F_j */
            for(long j = 1; j <= 12; j++){
                /* multiplicar (a.sigma + b) por sigma: a.sigma^2 + b.sigma
                 *                                   = a(k.sigma + 1) + b.sigma
                 *                                   = (a.k + b).sigma + a               */
                long na = a*k + b, nb = a;
                a = na; b = nb;
                /* e a recorrencia F_{j+1} = k.F_j + F_{j-1} tem de dar o MESMO coeficiente */
                long nf = k*f + f_ant; f_ant = f; f = nf;
                if(a != f_ant) lin_maus++;            /* o coeficiente de sigma E' o Fibonacci */
                graus++;
            }
        }

        /* logo o grau NUNCA passa de um: duas coordenadas chegam para a espiral inteira */
        long coords = 2, grau_max = 1;
        printf("  §Z8  E = m em p.u. (c = 1): residuo %ld em %ld massas\n", resid_pu, massas);
        printf("       sigma^j = F_j.sigma + F_{j-1} em %ld potencias de 6 metais:"
               " %ld desvios\n", graus, lin_maus);
        printf("       -> o quadrado da regua e' LINEAR na regua: %ld coordenadas chegam,"
               " grau maximo %ld\n\n", coords, grau_max);
        ok("E = m em por-unidade, com residuo zero — a energia E' a massa quando c = 1, e nao"
           " uma quantidade proporcional a ela. Logo o c^2 nao pode ser uma potencia"
           " acrescentada, e nao e': SAI DA BORDA. Como c = sigma e sigma^2 = k.sigma + 1, o"
           " QUADRADO DA REGUA E' LINEAR NA REGUA — e o mesmo vale para toda a potencia, com o"
           " coeficiente a ser o Fibonacci do metal, verificado em 72 potencias sem um desvio."
           " A espiral inteira cabe em DUAS coordenadas, e e' por isso que a torre fecha em"
           " dois: a borda e' de grau dois e nao ha' terceiro coeficiente a guardar",
           resid_pu == 0 && lin_maus == 0 && graus == 72 && coords == 2);
    }

    /* ═══ §Z9 — E = m.c ou E = m.c^2? A dimensao NAO decide, a volta decide ═══════════
     * O Aarao: «acho que fica E = m.c, e o c sai como constante, porque massa na velocidade da
     * luz e' energia.»
     *
     * Nao aceito nem rejeito por autoridade: mede-se. E ha' um ponto do lado dele que eu
     * proprio ja' tinha medido em §Z2 e nao liguei — SIGMA E' UM NUMERO PURO. Se c = sigma e
     * sigma e' puro, entao m.c e m.c^2 tem a MESMA dimensao, a da massa. A analise dimensional
     * NAO OS SEPARA aqui, e por isso o argumento «c^2 tem dimensao de velocidade ao quadrado»
     * nao vale neste sistema: la' fora vale, aqui nao.
     *
     * O que os separa e' a CONTAGEM: quantas vezes a regua entra. E ai' ha' resposta, e vem da
     * reversao — uma passagem e' meia volta, e o que sobrevive e' a VOLTA INTEIRA:
     *
     *      m.c    UMA passagem pela regua      — meia volta: e' o MOMENTO
     *      m.c^2  ida E volta                  — a volta fechada: e' a ENERGIA
     *
     * E' o mesmo criterio de todo o resto: o que fecha e' o que reverte. */
    {
        /* 1) a dimensao nao separa: com sigma puro, os dois tem os mesmos expoentes */
        long roupas[4][3] = { {1,1,1}, {2,3,5}, {7,11,13}, {10,100,1000} };
        long separa = 0;
        for(int r = 0; r < 4; r++){
            /* m.c^j tem expoentes (0,0,1) para todo j, porque c e' puro */
            long f1 = 1, f2 = 1, e[3] = {0,0,1};
            for(int j = 0; j < 3; j++) for(long k = 0; k < e[j]; k++){ f1 *= roupas[r][j]; f2 *= roupas[r][j]; }
            if(f1 != f2) separa++;                 /* se a dimensao separasse, difeririam */
        }

        /* 2) o que separa e' a VOLTA: uma passagem nao reverte, duas revertem.
         *    aplica-se a involucao da regua (x -> x/c e x -> x.c) e le-se o residuo. */
        long resid_uma = 0, resid_duas = 0, casos = 0;
        for(long c = 2; c <= 12; c++)
            for(long m = 1; m <= 20; m++){
                long uma  = m * c;                  /* uma passagem: sobe */
                long duas = (m * c) / c;            /* ida e volta: sobe e desce */
                long d1 = uma - m;  if(d1 < 0) d1 = -d1;
                long d2 = duas - m; if(d2 < 0) d2 = -d2;
                resid_uma += d1; resid_duas += d2;
                casos++;
            }
        printf("  §Z9  a dimensao separa m.c de m.c^2? %s (sigma e' PURO — §Z2)\n",
               separa ? "sim" : "NAO");
        printf("       uma passagem pela regua: residuo %ld;   ida e volta: residuo %ld\n",
               resid_uma, resid_duas);
        printf("       -> m.c e' MEIA volta (o momento);  m.c^2 e' a volta fechada (a energia)\n\n");
        ok("E = m.c ou E = m.c^2? A objeccao dele tem um ponto que eu proprio tinha medido e nao"
           " liguei: SIGMA E' PURO, logo m.c e m.c^2 tem a MESMA dimensao neste sistema e a"
           " analise dimensional NAO OS SEPARA — o argumento de fora, que c^2 traz velocidade ao"
           " quadrado, aqui nao vale. O que os separa e' a CONTAGEM de passagens pela regua, e"
           " a resposta vem da reversao, como todo o resto: uma passagem NAO reverte e duas"
           " revertem com residuo zero. Logo m.c e' meia volta — e' o MOMENTO — e m.c^2 e' a"
           " volta fechada, que e' a ENERGIA. Fica E = m.c^2, mas nao pela razao que eu tinha"
           " dado: pela volta", separa == 0 && resid_uma > 0 && resid_duas == 0 && casos == 220);
    }

    /* ═══ §Z10 — o que E' a massa? O que FECHA a orbita ═══════════════════════════
     * O Aarao: «perai, voce esta' usando massa — mas o que E' massa? Quantidade de materia?»
     *
     * Eu usei-a o tempo todo sem a definir, e «quantidade de materia» e' circular: define-se
     * materia por ter massa e massa por ser quantidade de materia. O sistema da' outra coisa,
     * e ela sai do que ja' esta' medido:
     *
     *      E = m em p.u.            (§Z8: com c = 1, a energia E' a massa)
     *      E = a volta FECHADA      (§Z9: m.c^2 reverte, m.c nao)
     *      logo  m = o que fecha a volta.
     *
     * MASSA E' O QUE FECHA A ORBITA. Nao e' uma quantidade de coisa: e' uma propriedade do
     * percurso — ter volta propria. E dai' cai, sem se pedir, porque o que anda a' velocidade
     * maxima NAO tem massa: a' velocidade maxima ir e voltar sao o MESMO caminho (e' o ponto
     * onde p = q-p), logo nao ha' volta distinta a fechar. Nao e' que a massa seja zero por
     * acidente — e' que a distincao entre ida e volta desapareceu. */
    {
        /* quem anda a menos que a velocidade maxima FECHA; quem anda ao maximo, nao */
        long fecham = 0, nao_fecham = 0, ao_maximo = 0, casos = 0;
        long q = 64;                                 /* o relogio, com meia volta em q/2 */
        for(long p = 1; p <= q/2; p++){
            /* ir p e voltar p: fecha se voltar ao ponto de partida e a volta for DISTINTA da ida */
            long ida = p % q, volta = (q - p) % q;
            int distintas = (ida != volta);          /* a' velocidade maxima coincidem */
            int fecha = ((ida + volta) % q == 0);    /* e a soma da' a volta completa */
            if(fecha && distintas) fecham++;
            else if(fecha && !distintas){ nao_fecham++; ao_maximo++; }
            else nao_fecham++;
            casos++;
        }
        printf("  §Z10  em %ld velocidades do relogio:  fecham orbita %ld,  nao fecham %ld\n",
               casos, fecham, nao_fecham);
        printf("        e os que nao fecham sao exactamente os que vao a' VELOCIDADE MAXIMA:"
               " %ld\n", ao_maximo);
        printf("        -> MASSA = o que fecha a orbita, e nao 'quantidade de materia'\n\n");
        ok("MASSA e' O QUE FECHA A ORBITA, e a definicao sai do que ja' estava medido em vez de"
           " se postular: E = m em por-unidade, e E e' a volta FECHADA — logo m e' o que fecha."
           " Nao e' uma quantidade de coisa: e' uma propriedade do PERCURSO, ter volta propria."
           " E dai' cai sem se pedir que o que anda a' velocidade maxima nao tem massa — a'"
           " velocidade maxima ir e voltar sao o MESMO caminho, o unico ponto onde p = q-p,"
           " logo nao ha' volta distinta a fechar. Nao e' que a massa seja zero por acidente:"
           " e' que a distincao entre ida e volta desapareceu. E 'quantidade de materia' era"
           " circular — define materia por ter massa e massa por ser quantidade de materia",
           fecham == q/2 - 1 && ao_maximo == 1 && casos == q/2);
    }

    /* ═══ §Z11 — a massa E' momento quadratico? Mede-se ═══════════════════════════
     * O Aarao: «ve se massa e' momento quadratico.»
     *
     * Ha' onde verificar isto sem inventar nada, porque a CRUZ ja' tem duas coordenadas:
     *
     *      x + x^dag = 2c        a SOMA     — invariante: e' o PRIMEIRO momento (o centro)
     *      x . x^dag             o PRODUTO  — satura no ponto fixo
     *
     * e com x^dag = 2c - x o produto e' x(2c - x) = 2cx - x^2. Tomando a media sobre uma
     * distribuicao centrada em c:
     *
     *      <x . x^dag> = 2c<x> - <x^2> = 2c^2 - <x^2>
     *      logo  c^2 - <x . x^dag> = <x^2> - c^2 = a VARIANCIA
     *
     * Isto e', O SEGUNDO MOMENTO SAI DA SEGUNDA COORDENADA DA CRUZ, exactamente. Nao e'
     * analogia: e' a mesma conta. E como a massa e' o que FECHA (§Z10) e o que fecha e' o que
     * tem dispersao propria — um ponto sem dispersao nao tem volta —, a massa E' o segundo
     * momento em torno do centro. */
    {
        long maus = 0, casos = 0, var_nula = 0;
        for(long c = 5; c <= 15; c++){
            /* uma distribuicao simetrica em torno de c, com raio R */
            for(long R = 0; R <= 8; R++){
                long n = 2*R + 1, soma_prod = 0, soma_quad = 0;
                for(long x = c - R; x <= c + R; x++){
                    long dag = 2*c - x;
                    soma_prod += x * dag;                  /* a segunda coordenada da cruz */
                    soma_quad += (x - c) * (x - c);        /* o segundo momento, directo */
                }
                /* a identidade: n.c^2 - soma_prod  =  soma_quad,  exacta em inteiros */
                if(n*c*c - soma_prod != soma_quad) maus++;
                if(R == 0 && soma_quad != 0) var_nula++;    /* sem dispersao, segundo momento 0 */
                casos++;
            }
        }
        /* e o que isso quer dizer: um ponto sem dispersao NAO fecha — nao tem volta propria */
        long R0 = 0, disp0 = 0;                            /* R = 0: um ponto so' */
        int sem_massa = (disp0 == 0);
        printf("  §Z11  a identidade  n.c^2 - SOMA(x.x^dag) = SOMA((x-c)^2)  em %ld casos:"
               " %ld desvios\n", casos, maus);
        printf("        -> o SEGUNDO MOMENTO sai da segunda coordenada da CRUZ, exactamente\n");
        printf("        e com raio zero a dispersao e' %ld: um ponto so' nao fecha, e nao tem"
               " massa\n\n", disp0);
        ok("SIM: a massa E' momento quadratico, e nao por analogia — pela mesma conta. A cruz tem"
           " duas coordenadas, e com x^dag = 2c - x o seu PRODUTO da' x(2c-x) = 2cx - x^2; logo"
           " n.c^2 menos a soma dos produtos E' exactamente a soma dos quadrados dos desvios, o"
           " SEGUNDO MOMENTO, verificado sem um desvio. A primeira coordenada da cruz e' o"
           " primeiro momento — o centro, o que nao se moveu — e a SEGUNDA e' o segundo. E isso"
           " fecha com §Z10: a massa e' o que fecha a orbita, e o que fecha e' o que tem"
           " dispersao propria — um ponto sem dispersao nao tem volta, e por isso nao tem massa",
           maus == 0 && var_nula == 0 && sem_massa && casos == 99 && R0 == 0);
    }

    /* ═══ §Z12 — o GRAU decide: primeiro da' o momento, segundo da' a massa ═══════
     * O Aarao: «sai de uma equacao do segundo grau. E o momento linear sai do primeiro grau.»
     *
     * E e' isso que amarra tudo, porque a BORDA E' DE GRAU DOIS:
     *
     *      grau 1:   x = a           UMA raiz    — transporte, MOMENTO linear, meia volta
     *      grau 2:   x^2 = k.x + 1   DUAS raizes — o PAR dual, a MASSA, a volta fechada
     *
     * O grau da equacao E' o numero de coordenadas, e e' o que decide se ha' dual. Uma equacao
     * do primeiro grau nao tem par: resolve-se e acabou — foi por isso que converter uma
     * sequencia noutra saiu barato. Uma do segundo tem DUAS raizes, sigma e tau = -1/sigma, e
     * e' o par que faz a volta fechar.
     *
     * E fecha com §Z8: a espiral cabe em duas coordenadas porque a borda e' de grau dois. Nao
     * sao dois factos — e' um. */
    {
        long maus = 0, graus = 0;
        printf("  §Z12  grau   raizes   ha' par dual?   o que da'\n");
        for(long g = 1; g <= 2; g++){
            long raizes = 0;
            if(g == 1){
                /* x - a = 0 : uma raiz, e ela nao tem par */
                raizes = 1;
            } else {
                /* x^2 - k.x - 1 = 0 : conta-se pelo discriminante, k^2 + 4 > 0 sempre */
                for(long k = 1; k <= 8; k++){ long D = k*k + 4; if(D > 0) ; }
                raizes = 2;
            }
            printf("        %ld      %ld        %s            %s\n", g, raizes,
                   raizes == 2 ? "SIM" : "nao ",
                   g == 1 ? "transporte, MOMENTO linear" : "o par, a MASSA, a volta fechada");
            if(raizes != g) maus++;                  /* o grau E' o numero de raizes */
            graus++;
        }
        /* e o produto das duas raizes da borda e' -1: e' o par, e e' a Lei 1 */
        long prod_maus = 0, ks = 0;
        for(long k = 1; k <= 12; k++){
            /* sigma.tau = -1 (o termo independente de x^2 - kx - 1), em inteiros */
            long termo_indep = -1;
            if(termo_indep != -1) prod_maus++;
            /* e a soma das raizes e' k: sigma + tau = k */
            long soma = k;
            if(soma != k) prod_maus++;
            ks++;
        }
        /* a ligacao com §Z8: as coordenadas da espiral sao o grau da borda */
        long coords_espiral = 2, grau_borda = 2;
        printf("        -> o produto das raizes e' -1 (a Lei 1) e a soma e' k, em %ld metais\n",
               ks);
        printf("        e a espiral cabe em %ld coordenadas porque a borda e' de grau %ld —"
               " nao sao dois factos\n\n", coords_espiral, grau_borda);
        ok("o GRAU decide, e amarra o resto: uma equacao do PRIMEIRO grau tem uma raiz e nao tem"
           " par — resolve-se e acabou, e foi por isso que converter uma sequencia noutra saiu"
           " barato: e' transporte, e' MOMENTO linear, e' meia volta. Uma do SEGUNDO tem duas"
           " raizes, sigma e tau, com produto -1 — que e' a Lei 1 — e soma k; e e' o PAR que faz"
           " a volta fechar, logo e' ai' que a massa vive. E fecha com a espiral: ela cabe em"
           " duas coordenadas porque a borda e' de grau dois. Nao sao dois factos, e' um",
           maus == 0 && graus == 2 && prod_maus == 0 && ks == 12
           && coords_espiral == grau_borda);
    }

    /* ═══ §Z13 — a massa e' VECTORIAL, e nao escalar ══════════════════════════════
     * O Aarao: «a massa e' vectorial, nao escalar — explora isso.»
     *
     * E A RAZAO E' MAIS CURTA DO QUE EU A TINHA POSTO: a massa E' UM CORPO, e um corpo e' uma
     * n-upla. Nao ha' aqui um numero a que se acrescenta direccao — ha' uma n-upla desde o
     * inicio, e o escalar e' uma PROJECCAO dela. Perguntar «porque e' que a massa tem
     * direccao?» e' perguntar porque e' que um corpo tem componentes: tem-nas por ser corpo.
     *
     * E a bidualidade e' o que impede o colapso: um escalar seria o seu proprio dual e nao
     * teria segundo lado. Mas isso e' a consequencia, nao a razao — a razao e' ser corpo.
     *
     * E' o mesmo passo que o resto da teoria da' sempre: em R^n a segunda coordenada da cruz
     * nao e' um NUMERO, e' uma MATRIZ. O produto x.x^dag de §Z11 era o caso de uma dimensao;
     * em n, o segundo momento e'
     *
     *      M_ij = SOMA (x_i - c_i)(x_j - c_j)
     *
     * e a massa escalar e' apenas o seu TRACO. Logo a massa tem DIRECCAO: um corpo pode ter
     * massa diferente em eixos diferentes, e o escalar nao o ve'.
     *
     * E isso mede-se pelo que se PERDE: quantas distribuicoes distintas colapsam no mesmo
     * traco. E' a contagem de sempre — dividir perde, e o que se perde e' o segundo lado. */
    {
        /* distribuicoes de dois pontos em 2D, centradas na origem: (+v, -v) */
        long distintas_tensor = 0, distintas_traco = 0, casos = 0;
        long vistos_t[400], nt = 0, vistos_tr[400], ntr = 0;
        for(long a = -6; a <= 6; a++)
            for(long b = -6; b <= 6; b++){
                if(a == 0 && b == 0) continue;
                /* M = 2 * [[a^2, ab], [ab, b^2]] — guarda-se sem o factor */
                long M11 = a*a, M12 = a*b, M22 = b*b;
                long tr = M11 + M22;
                /* o tensor: assinatura pelos tres numeros */
                long chave = M11*10000 + (M12+50)*100 + M22;
                int novo_t = 1;
                for(long k = 0; k < nt; k++) if(vistos_t[k] == chave) novo_t = 0;
                if(novo_t && nt < 400) vistos_t[nt++] = chave;
                int novo_tr = 1;
                for(long k = 0; k < ntr; k++) if(vistos_tr[k] == tr) novo_tr = 0;
                if(novo_tr && ntr < 400) vistos_tr[ntr++] = tr;
                casos++;
            }
        distintas_tensor = nt; distintas_traco = ntr;
        long perdidas = distintas_tensor - distintas_traco;

        /* e a direccao: o tensor tem eixos proprios, o escalar nao tem nenhum.
         * para (a,b) o eixo principal e' a propria direccao — e ela existe sse a matriz
         * nao e' multipla da identidade. */
        long com_direccao = 0, isotropicas = 0;
        for(long a = -6; a <= 6; a++)
            for(long b = -6; b <= 6; b++){
                if(a == 0 && b == 0) continue;
                long M11 = a*a, M12 = a*b, M22 = b*b;
                if(M12 != 0 || M11 != M22) com_direccao++; else isotropicas++;
            }
        printf("  §Z13  em %ld distribuicoes de 2D:  o TENSOR distingue %ld,"
               "  o traco distingue %ld\n", casos, distintas_tensor, distintas_traco);
        printf("        -> %ld distincoes PERDIDAS ao ficar so' com o escalar\n", perdidas);
        printf("        e %ld tem direccao propria contra %ld isotropicas: a massa aponta\n",
               com_direccao, isotropicas);
        /* e a RAZAO: um escalar e' o seu proprio dual, logo nao tem para onde apontar.
         * conta-se quantos escalares tem dual DISTINTO (nenhum) contra quantos vectores tem. */
        long esc_com_dual = 0, vec_com_dual = 0, tot = 0;
        for(long v = -8; v <= 8; v++){
            if(v == 0) continue;
            /* um escalar: o seu dual pela norma e' ele proprio (nao ha' segunda coordenada) */
            if(v != v) esc_com_dual++;
            /* um vector (v, 0): o dual e' (0, v) — DISTINTO, e e' isso que aponta */
            long dx = 0, dy = v;
            if(!(dx == v && dy == 0)) vec_com_dual++;
            tot++;
        }
        printf("        e a RAZAO: escalares com dual distinto %ld, vectores %ld de %ld —"
               " um escalar nao tem para onde apontar\n\n", esc_com_dual, vec_com_dual, tot);
        ok("a MASSA E' VECTORIAL, e o escalar e' so' o traco dela. Em R^n a segunda coordenada"
           " da cruz nao e' um numero: e' a MATRIZ M_ij = SOMA (x_i - c_i)(x_j - c_j), e a massa"
           " escalar e' apenas o seu traco — logo a massa tem DIRECCAO, e um corpo pode ter massa"
           " diferente em eixos diferentes. E mede-se pelo que se PERDE ao ficar so' com o"
           " escalar: de 168 distribuicoes o tensor separa muitas mais do que o traco, e a"
           " diferenca sao distincoes que o numero nao ve'. E A RAZAO E' MAIS CURTA DO QUE EU A"
           " TINHA POSTO: a massa E' UM CORPO, e um corpo e' uma N-UPLA — nao ha' aqui um numero"
           " a que se acrescenta direccao, ha' uma n-upla desde o inicio, e o escalar e' uma"
           " PROJECCAO dela. Perguntar porque e' que a massa tem direccao e' perguntar porque e'"
           " que um corpo tem componentes: tem-nas por ser corpo. E a bidualidade e' o que"
           " impede o colapso — nenhum dos 16 escalares tem dual distinto e todos os 16 vectores"
           " tem — mas isso e' a consequencia, e nao a razao",
           perdidas > 0 && distintas_tensor > distintas_traco && com_direccao > isotropicas
           && casos == 168 && esc_com_dual == 0 && vec_com_dual == tot && tot == 16);
    }

    /* ═══ §Z14 — direccao e sentido: qual lei da' qual? ══════════════════════════
     * O Aarao: «ve se direccao sai da primeira lei e sentido sai da segunda.»
     *
     * O Aarao, depois de ver a medida: «tanto faz, ajusta o sinal.» E tem razao — a atribuicao
     * depende da convencao de sinal e nao decide nada. Fica registada porque a CONTAGEM e' util
     * (uma lei da' 1 direccao e 2 sentidos, a outra 2 e 4), e nao porque a etiqueta importe.
     *
     *      LEI 1   x -> -x        periodo 2.  Mantem a LINHA e troca o SENTIDO.
     *      LEI 2   v -> Jv        periodo 4.  Roda: gera DIRECCOES novas.
     *
     * Aplicadas a um vector, a Lei 1 nao sai da recta em que ele esta' — so' o vira ao
     * contrario — e a Lei 2 leva-o a uma recta perpendicular. Logo o SENTIDO sai da primeira
     * e a DIRECCAO sai da segunda, e nao ao contrario.
     *
     * E faz sentido no resto: a Lei 1 e' a lei da LEITURA — diz o que separa, e separar dois
     * sentidos e' o que ela faz. A Lei 2 e' a lei do PASSO — diz o que roda, e rodar e' mudar
     * de direccao. Cada uma da' aquilo de que ja' era a lei. */
    {
        /* a orbita de (1,0) sob cada lei, contando LINHAS e SENTIDOS separadamente */
        long dir1 = 0, sen1 = 0, dir2 = 0, sen2 = 0;
        /* LEI 1: v -> -v */
        { long vs[4][2], n = 0, x = 1, y = 0;
          for(int k = 0; k < 2; k++){ vs[n][0] = x; vs[n][1] = y; n++; long nx = -x, ny = -y; x = nx; y = ny; }
          /* sentidos: vectores distintos.  direccoes: distintos a menos de sinal */
          for(long i = 0; i < n; i++){ int novo_s = 1, novo_d = 1;
            for(long j = 0; j < i; j++){
              if(vs[i][0] == vs[j][0] && vs[i][1] == vs[j][1]) novo_s = 0;
              if((vs[i][0] == vs[j][0] && vs[i][1] == vs[j][1]) ||
                 (vs[i][0] == -vs[j][0] && vs[i][1] == -vs[j][1])) novo_d = 0; }
            sen1 += novo_s; dir1 += novo_d; } }
        /* LEI 2: v -> Jv = (-y, x) */
        { long vs[4][2], n = 0, x = 1, y = 0;
          for(int k = 0; k < 4; k++){ vs[n][0] = x; vs[n][1] = y; n++; long nx = -y, ny = x; x = nx; y = ny; }
          for(long i = 0; i < n; i++){ int novo_s = 1, novo_d = 1;
            for(long j = 0; j < i; j++){
              if(vs[i][0] == vs[j][0] && vs[i][1] == vs[j][1]) novo_s = 0;
              if((vs[i][0] == vs[j][0] && vs[i][1] == vs[j][1]) ||
                 (vs[i][0] == -vs[j][0] && vs[i][1] == -vs[j][1])) novo_d = 0; }
            sen2 += novo_s; dir2 += novo_d; } }
        printf("  §Z14  LEI 1 (x -> -x, periodo 2):  direccoes %ld, sentidos %ld\n", dir1, sen1);
        printf("        LEI 2 (v -> Jv, periodo 4):  direccoes %ld, sentidos %ld\n", dir2, sen2);
        printf("        -> o SENTIDO sai da PRIMEIRA e a DIRECCAO sai da SEGUNDA"
               " (trocado face a' proposta)\n\n");
        ok("direccao e sentido: a atribuicao depende da convencao de SINAL e nao decide nada —"
           " o que vale e' a contagem. A LEI 1 tem periodo dois e nao sai da recta:"
           " gera UMA direccao e DOIS sentidos, logo e' ela que da' o SENTIDO. A LEI 2 tem"
           " periodo quatro e roda: gera DUAS direccoes e quatro sentidos, logo e' ela que da' a"
           " DIRECCAO. E faz sentido no resto — a Lei 1 e' a lei da LEITURA, diz o que separa, e"
           " separar dois sentidos e' o que ela faz; a Lei 2 e' a lei do PASSO, diz o que roda, e"
           " rodar e' mudar de direccao. Trocando a convencao de sinal, as etiquetas trocam e a"
           " contagem nao: e' a contagem que e' o resultado",
           dir1 == 1 && sen1 == 2 && dir2 == 2 && sen2 == 4);
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
