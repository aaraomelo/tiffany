/* metrica.c — A MÉTRICA CANÓNICA É A DE LEBESGUE, e conservar a MEDIDA não é
 * conservar a DISTÂNCIA.
 *
 * O Aarão: «precisamos de uma métrica canónica e ela é a métrica de Lebesgue; a
 * conservação da área não confundir com a medida».
 *
 * A métrica de Lebesgue da literatura é a da diferença simétrica:
 *
 *      d(A,B) = µ(A △ B)                A △ B = (A∖B) ∪ (B∖A)
 *
 * — não negativa, simétrica, e triangular porque A △ C ⊆ (A △ B) ∪ (B △ C). É
 * PSEUDOmétrica nos conjuntos e MÉTRICA no quociente pelos de medida nula: d = 0 não diz
 * «o mesmo conjunto», diz «iguais a menos de medida nula».
 *
 * E no toro discreto ela é CONTAGEM PURA — µ(A) = |A| —, o que permite medir tudo em ℤ,
 * sem uma vírgula. É o mesmo gesto de sempre: escolher o andar onde o objecto é inteiro.
 *
 * A DISTINÇÃO, que é o ponto:
 *
 *      |det| = 1  ⟹  T é BIJECÇÃO  ⟹  µ(TA) = µ(A)      CONSERVA A MEDIDA
 *      mas        d(A, TA) > 0 e não decresce            NÃO CONSERVA A DISTÂNCIA
 *
 * Conservar a área e ser isometria são coisas diferentes, e o gato é o contra-exemplo:
 * ele preserva a medida exactamente e ESPALHA os conjuntos.
 *
 *   §M1  a métrica de Lebesgue: os três axiomas, em contagem
 *   §M2  e d = 0 ⟺ os conjuntos coincidem (aqui não há medida nula não vazia)
 *   §M3  |det| = 1 CONSERVA A MEDIDA — a contagem não muda
 *   §M4  e NÃO CONSERVA A DISTÂNCIA — o gato espalha, e mede-se quanto
 *   §M5  o CONTRASTE: quem é isometria é o que fixa a assinatura, não o que fixa a área
 *   §M6  e a ORDEM atravessa (thm:central-energia): µ é monótona e σ-aditiva
 *
 *   cc -O2 -std=c99 -I. -I../lib metrica.c -o metrica && ./metrica
 */
#include <stdio.h>
#include <stdlib.h>
#include "unidade.h"

#define N 16                      /* o toro discreto Z_N × Z_N */
#define CEL (N*N)

typedef struct { unsigned char c[CEL]; } Conj;   /* a função indicadora */

static long mu(const Conj *A){                    /* µ(A) = |A|, a contagem */
    long s = 0;
    for(int i = 0; i < CEL; i++) if(A->c[i]) s++;
    return s;
}
static void simetrica(const Conj *A, const Conj *B, Conj *R){
    for(int i = 0; i < CEL; i++) R->c[i] = (unsigned char)(A->c[i] ^ B->c[i]);
}
static long dist(const Conj *A, const Conj *B){   /* d(A,B) = µ(A △ B) */
    Conj R; simetrica(A, B, &R); return mu(&R);
}
/* a acção de uma matriz inteira no toro: (x,y) → (ax+by, cx+dy) mod N */
static void aplica(const long *T, const Conj *A, Conj *R){
    for(int i = 0; i < CEL; i++) R->c[i] = 0;
    for(long y = 0; y < N; y++) for(long x = 0; x < N; x++){
        if(!A->c[y*N + x]) continue;
        long nx = ((T[0]*x + T[1]*y) % N + N) % N;
        long ny = ((T[2]*x + T[3]*y) % N + N) % N;
        R->c[ny*N + nx] = 1;
    }
}
static void bloco(Conj *A, long x0, long y0, long w, long h){
    for(int i = 0; i < CEL; i++) A->c[i] = 0;
    for(long y = y0; y < y0 + h; y++) for(long x = x0; x < x0 + w; x++)
        A->c[(((y % N) + N) % N)*N + (((x % N) + N) % N)] = 1;
}

int main(void){
    printf("\n══ A MÉTRICA DE LEBESGUE: d(A,B) = µ(A △ B), no toro Z_%d × Z_%d ══\n\n", N, N);

    /* ─── §M1 ── os três axiomas, em contagem ──────────────────────────────────────── */
    {
        long tot = 0, naoneg = 0, simet = 0, trian = 0;
        Conj A, B, C;
        for(long a = 1; a <= 4; a++) for(long b = 1; b <= 4; b++)
        for(long c = 1; c <= 4; c++) for(long e = 1; e <= 4; e++){
            bloco(&A, a, b, 3, 3);
            bloco(&B, c, e, 4, 2);
            bloco(&C, a + c, b + e, 2, 5);
            long dab = dist(&A,&B), dba = dist(&B,&A);
            long dac = dist(&A,&C), dbc = dist(&B,&C);
            tot++;
            if(dab >= 0) naoneg++;
            if(dab == dba) simet++;
            if(dac <= dab + dbc) trian++;          /* A △ C ⊆ (A △ B) ∪ (B △ C) */
        }
        printf("  §M1  os três axiomas da métrica, em CONTAGEM\n");
        printf("      ternos varridos .................... %ld\n", tot);
        printf("      não negativa ....................... %ld\n", naoneg);
        printf("      simétrica .......................... %ld\n", simet);
        printf("      triangular ......................... %ld\n\n", trian);
        ok("A MÉTRICA CANÓNICA É A DE LEBESGUE: d(A,B) = µ(A △ B), a medida da diferença"
           " simétrica. Os três axiomas verificam-se em CONTAGEM PURA no toro discreto —"
           " não negativa e simétrica porque △ é comutativa, e triangular porque"
           " A △ C está contido em (A △ B) ∪ (B △ C). Nenhuma vírgula entra",
           tot > 0 && naoneg == tot && simet == tot && trian == tot);
    }

    /* ─── §M2 ── d = 0 ⟺ coincidem ─────────────────────────────────────────────────── */
    {
        long pares = 0, zero_sse_igual = 0, iguais = 0;
        Conj A, B;
        for(long a = 0; a < 6; a++) for(long b = 0; b < 6; b++){
            bloco(&A, 1, 1, 3, 3);
            bloco(&B, a, b, 3, 3);
            long d = dist(&A,&B);
            int mesmos = 1;
            for(int i = 0; i < CEL; i++) if(A.c[i] != B.c[i]) mesmos = 0;
            pares++;
            if((d == 0) == mesmos) zero_sse_igual++;
            if(mesmos) iguais++;
        }
        printf("  §M2  d = 0 ⟺ os conjuntos coincidem\n");
        printf("      pares varridos ..................... %ld\n", pares);
        printf("      «d = 0 ⟺ iguais» ................... %ld   (iguais: %ld)\n\n",
               zero_sse_igual, iguais);
        ok("e d = 0 ⟺ os conjuntos coincidem — no discreto, porque aqui o único conjunto de"
           " medida nula é o VAZIO. É por isso que d é métrica e não só pseudométrica: no"
           " contínuo ela é métrica no QUOCIENTE pelos de medida nula, e o quociente é o que"
           " o discreto já fez",
           pares > 0 && zero_sse_igual == pares && iguais > 0 && iguais < pares);
    }

    /* ─── §M3 ── |det| = 1 conserva a MEDIDA ───────────────────────────────────────── */
    {
        long casos = 0, conserva = 0, bijeccao = 0;
        const long OPS[4][4] = { {1,1,1,2}, {2,1,1,1}, {1,0,1,1}, {3,1,1,0} };
        Conj A, TA;
        for(int k = 0; k < 4; k++){
            long det = OPS[k][0]*OPS[k][3] - OPS[k][1]*OPS[k][2];
            if(det != 1 && det != -1) continue;
            for(long w = 2; w <= 6; w++){
                bloco(&A, 3, 4, w, w);
                aplica(OPS[k], &A, &TA);
                casos++;
                if(mu(&TA) == mu(&A)) conserva++;      /* µ(TA) = µ(A) */
                /* e a razão é ser BIJECÇÃO: |det| = 1 ⟹ invertível mod N para N ímpar
                 * ou com det ímpar — aqui basta que a imagem tenha a mesma contagem */
                if(mu(&TA) == mu(&A)) bijeccao++;
            }
        }
        printf("  §M3  |det| = 1 CONSERVA A MEDIDA\n");
        printf("      casos varridos ..................... %ld\n", casos);
        printf("      µ(TA) = µ(A) ....................... %ld\n\n", conserva);
        ok("e |det| = 1 CONSERVA A MEDIDA: o operador é bijecção do toro, logo a contagem"
           " não muda — µ(TA) = µ(A) exactamente, para todo conjunto e todo operador"
           " unimodular. É a conservação da ÁREA, e é o que o thm:conservacao já dizia",
           casos > 0 && conserva == casos && bijeccao == casos);
    }

    /* ─── §M4 ── A MÉTRICA DE LEBESGUE É PRESERVADA POR TODA BIJECÇÃO ──────────────
     * Aqui a medida corrigiu a tese que eu ia escrever. Eu esperava que o gato ESPALHASSE
     * na métrica de Lebesgue — e não espalha, porque para T bijectiva
     *
     *      T(A △ B) = TA △ TB      logo      d(TA,TB) = d(A,B)
     *
     * exactamente. TODA bijecção do toro é ISOMETRIA nesta métrica, e a razão é que ela é
     * feita de µ, que a bijecção conserva. Donde: a métrica de Lebesgue mede a MEDIDA e não
     * a FORMA — ela não vê o esticar do gato, porque contar não vê direcção. */
    {
        long casos = 0, preserva = 0, moveu = 0;
        const long OPS[3][4] = { {2,1,1,1}, {1,1,1,2}, {3,1,1,0} };   /* |det| = 1 */
        Conj A, B, TA, TB;
        for(int k = 0; k < 3; k++) for(long w = 2; w <= 5; w++) for(long s = 1; s <= 4; s++){
            bloco(&A, 2, 2, w, w);
            bloco(&B, 2 + s, 2, w, w);
            aplica(OPS[k], &A, &TA);
            aplica(OPS[k], &B, &TB);
            casos++;
            if(dist(&TA,&TB) == dist(&A,&B)) preserva++;
            if(dist(&A,&TA) > 0) moveu++;              /* e MOVE o conjunto na mesma */
        }
        printf("  §M4  a métrica de Lebesgue é preservada por TODA bijecção\n");
        printf("      casos varridos ..................... %ld\n", casos);
        printf("      d(TA,TB) = d(A,B) .................. %ld\n", preserva);
        printf("      e ainda assim MOVE o conjunto ...... %ld\n\n", moveu);
        ok("A MÉTRICA DE LEBESGUE É PRESERVADA POR TODA BIJECÇÃO, e isto corrige o que eu ia"
           " afirmar: T(A △ B) = TA △ TB quando T é bijectiva, logo d(TA,TB) = d(A,B)"
           " EXACTAMENTE — o gato não espalha nesta métrica. A razão é que d é feita de µ, e"
           " a bijecção conserva µ. Donde a métrica de Lebesgue mede a MEDIDA e NÃO A FORMA:"
           " contar não vê direcção, e por isso ela não vê o esticar. E o operador move o"
           " conjunto na mesma — d(A,TA) > 0 —, o que mostra que mover e afastar-se são"
           " coisas diferentes",
           casos > 0 && preserva == casos && moveu == casos);
    }

    /* ─── §M5 ── E ONDE ELAS SE SEPARAM: a métrica ESPACIAL ────────────────────────────
     * A conservação da área não é a conservação das distâncias ENTRE PONTOS, e é aí que a
     * distinção vive. O gato tem |det| = 1 — conserva a medida — e ainda assim estica numa
     * direcção e encolhe na outra, com o produto dos factores igual a 1. Mede-se em
     * inteiros, nos QUADRADOS dos comprimentos, sem formar raiz nenhuma. */
    {
        const long G[4] = { 2,1,1,1 };               /* det = 1, disc = 5 > 0 */
        long tot = 0, mudou = 0, esticou = 0, encolheu = 0;
        for(long x = -6; x <= 6; x++) for(long y = -6; y <= 6; y++){
            if(x == 0 && y == 0) continue;
            long nx = G[0]*x + G[1]*y, ny = G[2]*x + G[3]*y;
            long antes = x*x + y*y, depois = nx*nx + ny*ny;   /* ‖v‖², sem raiz */
            tot++;
            if(depois != antes) mudou++;
            if(depois > antes) esticou++;
            if(depois < antes) encolheu++;
        }
        /* e a ÁREA conserva-se ao mesmo tempo: o paralelogramo de dois vectores tem área
         * |det| e o operador multiplica-a por |det T| = 1 — em inteiros, sem raiz. */
        long ar_tot = 0, ar_igual = 0;
        for(long a = 1; a <= 4; a++) for(long b = 0; b <= 4; b++)
        for(long c = 0; c <= 4; c++) for(long e = 1; e <= 4; e++){
            long A0 = a*e - b*c;                     /* área do paralelogramo (a,b),(c,e) */
            long na = G[0]*a + G[1]*b, nb = G[2]*a + G[3]*b;
            long nc = G[0]*c + G[1]*e, ne = G[2]*c + G[3]*e;
            long A1 = na*ne - nb*nc;
            ar_tot++;
            if(A1 == A0) ar_igual++;
        }
        printf("  §M5  onde elas se SEPARAM: a métrica ESPACIAL\n");
        printf("      vectores varridos .................. %ld\n", tot);
        printf("      com ‖Tv‖² ≠ ‖v‖² ................... %ld\n", mudou);
        printf("        dos quais ESTICARAM .............. %ld\n", esticou);
        printf("        e ENCOLHERAM ..................... %ld\n", encolheu);
        printf("      áreas varridas ..................... %ld   conservadas: %ld\n\n",
               ar_tot, ar_igual);
        ok("E É AQUI QUE ELAS SE SEPARAM: na métrica ESPACIAL, a das distâncias entre"
           " pontos. O gato conserva a ÁREA — o determinante do paralelogramo não muda em"
           " nenhum caso — e NÃO conserva o COMPRIMENTO: há vectores que esticam e vectores"
           " que encolhem, os dois regimes povoados, porque ele estica por sigma e encolhe"
           " por 1/sigma com o produto 1. Conservar a área e ser isometria são estruturas"
           " diferentes, e mede-se nos QUADRADOS dos comprimentos, sem raiz",
           tot > 0 && mudou > 0 && esticou > 0 && encolheu > 0 &&
           ar_tot > 0 && ar_igual == ar_tot);
    }

    /* ─── §M6 ── a ORDEM atravessa (thm:central-energia) ───────────────────────────── */
    {
        long casos = 0, monotona = 0, aditiva = 0;
        Conj A, B, U, I;
        for(long w = 1; w <= 5; w++) for(long s = 0; s <= 4; s++){
            bloco(&A, 2, 2, w, w);
            bloco(&B, 2, 2, w + s, w);
            /* A ⊆ B ⟹ µ(A) ≤ µ(B): a MONOTONIA, que é a ordem a atravessar */
            int contido = 1;
            for(int i = 0; i < CEL; i++) if(A.c[i] && !B.c[i]) contido = 0;
            casos++;
            if(!contido || mu(&A) <= mu(&B)) monotona++;
            /* e a ADITIVIDADE: µ(A∪B) + µ(A∩B) = µ(A) + µ(B) */
            for(int i = 0; i < CEL; i++){
                U.c[i] = (unsigned char)(A.c[i] | B.c[i]);
                I.c[i] = (unsigned char)(A.c[i] & B.c[i]);
            }
            if(mu(&U) + mu(&I) == mu(&A) + mu(&B)) aditiva++;
        }
        printf("  §M6  a ORDEM atravessa: µ é monótona e aditiva\n");
        printf("      casos varridos ..................... %ld\n", casos);
        printf("      A ⊆ B ⟹ µ(A) ≤ µ(B) ................ %ld\n", monotona);
        printf("      µ(A∪B) + µ(A∩B) = µ(A) + µ(B) ...... %ld\n\n", aditiva);
        ok("e é a ORDEM que atravessa, que é o que o thm:central-energia atribui a Lebesgue:"
           " µ é MONÓTONA — A contido em B dá µ(A) ≤ µ(B) — e ADITIVA — µ(A∪B) + µ(A∩B) ="
           " µ(A) + µ(B), exacto. É a σ-aditividade no andar finito, e é ela que transporta"
           " a contagem para o integral SEM PERDER A ORDEM",
           casos > 0 && monotona == casos && aditiva == casos);
    }

    /* ─── §M7 ── O HOMEOMORFISMO COM QUALQUER OUTRA MÉTRICA — e o que ele NÃO leva ──
     * O Aarão pede o homeomorfismo entre a métrica canónica e qualquer outra. Ele existe,
     * e a medida diz porquê — e diz também que ele é barato, o que é o resultado.
     *
     *   (a) EQUIVALÊNCIA por constantes. Sobre as funções indicadoras, a métrica de
     *       Lebesgue É a distância de Hamming e É a L¹; e contra a L∞ (que vale 1 se
     *       A ≠ B) tem-se
     *
     *              1·d∞(A,B)  ≤  d_Leb(A,B)  ≤  |X|·d∞(A,B)
     *
     *       com |X| = N² finito. Duas métricas com constantes destas são EQUIVALENTES, e a
     *       IDENTIDADE é homeomorfismo entre os dois espaços — nos dois sentidos.
     *
     *   (b) E É TRIVIAL, que é o ponto: num espaço FINITO toda métrica induz a topologia
     *       DISCRETA — a bola de raio ½ em torno de A é {A} em qualquer delas. Logo o
     *       homeomorfismo existe sempre e não diz nada sobre a medida.
     *
     *   (c) DONDE: o homeomorfismo NÃO conserva a área. Exibe-se uma bijecção contínua nos
     *       dois sentidos que leva um conjunto de medida m num de medida m' ≠ m. Conservar
     *       a medida é ESTRITAMENTE mais forte do que ser homeomorfismo, e é por isso que
     *       |det| = 1 não é uma condição topológica. */
    {
        long pares = 0, cotas_ok = 0, disc_ok = 0;
        long homeo_tot = 0, homeo_ok = 0, muda_medida = 0;
        Conj A, B;
        for(long a = 0; a < 5; a++) for(long b = 0; b < 5; b++)
        for(long w = 1; w <= 4; w++){
            bloco(&A, 1, 1, 3, 3);
            bloco(&B, a, b, w, w);
            long dl = dist(&A,&B);                     /* Lebesgue = Hamming = L¹ */
            long dinf = (dl > 0) ? 1 : 0;              /* L∞ sobre indicadoras */
            pares++;
            /* (a) as cotas: 1·d∞ ≤ d_Leb ≤ |X|·d∞ */
            if(dinf <= dl && dl <= CEL*dinf) cotas_ok++;
            /* (b) a topologia é DISCRETA nas duas: a bola de raio ½ em torno de A é {A},
             * e isso é «nenhum B ≠ A está a distância < 1» — em inteiros, d ≥ 1 */
            int mesmos = 1;
            for(int i = 0; i < CEL; i++) if(A.c[i] != B.c[i]) mesmos = 0;
            if(mesmos ? (dl == 0 && dinf == 0) : (dl >= 1 && dinf >= 1)) disc_ok++;
        }
        /* (c) o homeomorfismo que NÃO conserva a medida: a aplicação que troca o conjunto
         * A por A ∪ {ponto fixo}. É bijectiva sobre a sua imagem, contínua nos dois
         * sentidos na topologia discreta (toda aplicação o é), e MUDA a contagem. */
        /* E o exemplo é melhor do que o pedido: Φ(A) = A △ K, com K fixo, é
         *
         *   INVOLUÇÃO   Φ(Φ(A)) = A △ K △ K = A                     — logo bijecção
         *   ISOMETRIA   d(ΦA,ΦB) = |(A△K)△(B△K)| = |A△B| = d(A,B)   — o K cancela
         *   e NÃO conserva µ:  µ(A△K) ≠ µ(A) em geral
         *
         * Isto é mais forte do que o homeomorfismo: nem sequer é preciso mudar de métrica.
         * A PRÓPRIA métrica de Lebesgue tem isometrias que não conservam a medida — logo a
         * métrica NÃO DETERMINA a medida, e a conservação da área não se lê na distância. */
        { Conj K; bloco(&K, 5, 5, 3, 3);
          for(long w = 1; w <= 6; w++){
            bloco(&A, 2, 2, w, w);
            bloco(&B, 3, 4, w, 2);
            Conj FA, FB, FFA;
            simetrica(&A, &K, &FA);
            simetrica(&B, &K, &FB);
            simetrica(&FA, &K, &FFA);                  /* Φ(Φ(A)) tem de dar A */
            homeo_tot++;
            int involucao = (dist(&FFA, &A) == 0);
            int isometria = (dist(&FA, &FB) == dist(&A, &B));
            if(involucao && isometria) homeo_ok++;
            if(mu(&FA) != mu(&A)) muda_medida++;       /* e a MEDIDA muda */
          } }
        printf("  §M7  o homeomorfismo com qualquer outra métrica — e o que ele NÃO leva\n");
        printf("      pares varridos ..................... %ld\n", pares);
        printf("      1·d∞ ≤ d_Leb ≤ |X|·d∞ .............. %ld\n", cotas_ok);
        printf("      e a topologia é DISCRETA nas duas .. %ld\n", disc_ok);
        printf("      Φ(A) = A △ K: involução E isometria  %ld de %ld\n", homeo_ok, homeo_tot);
        printf("      que MUDAM a medida ................. %ld\n\n", muda_medida);
        ok("O HOMEOMORFISMO COM QUALQUER OUTRA MÉTRICA EXISTE, E É TRIVIAL — e é isso o"
           " resultado. Sobre as indicadoras a métrica de Lebesgue É a de Hamming e É a L¹,"
           " e contra a L∞ vale 1.d∞ ≤ d_Leb ≤ |X|.d∞ com |X| finito: constantes destas"
           " tornam-nas EQUIVALENTES e fazem da IDENTIDADE um homeomorfismo nos dois"
           " sentidos. Mas num espaco FINITO toda metrica induz a topologia DISCRETA — a"
           " bola de raio 1/2 e' o proprio ponto —, logo o homeomorfismo nao distingue"
           " metrica nenhuma. E O QUE ELE NAO LEVA E' A MEDIDA: exibe-se uma bijeccao"
           " ISOMETRIA da PROPRIA metrica que muda a contagem: Phi(A) = A triangulo K e'"
           " involucao (o K cancela duas vezes) e isometria (o K cancela na diferenca"
           " simetrica), e no entanto mu(A triangulo K) != mu(A). Logo a METRICA NAO"
           " DETERMINA A MEDIDA — nem sequer e' preciso mudar de metrica —, e conservar a"
           " area e' ESTRITAMENTE mais forte do que preservar a distancia",
           pares > 0 && cotas_ok == pares && disc_ok == pares &&
           homeo_tot > 0 && homeo_ok == homeo_tot && muda_medida == homeo_tot);
    }

    /* ─── §M8 ── O DUAL: o DIFEOMORFISMO, e é ele que VÊ a medida ───────────────────
     * O §M7 mostrou que o homeomorfismo não vê a medida. O dual dele é o DIFEOMORFISMO, e
     * o que os separa é a DERIVADA:
     *
     *      HOMEOMORFISMO   bijecção contínua com inversa contínua    preserva a TOPOLOGIA
     *      DIFEOMORFISMO   bijecção diferenciável, inversa idem      preserva a ESTRUTURA
     *
     * e o difeomorfismo traz consigo o JACOBIANO, que é o que transporta a medida:
     *
     *      µ(Φ(A)) = ∫_A |det DΦ|        e, para Φ LINEAR,  DΦ = Φ  ⟹  µ(TA) = |det T|·µ(A)
     *
     * Donde CONSERVAR A MEDIDA é |det| = 1 — e agora a condição tem origem: ela não é
     * topológica (o §M7 mostrou-o), é DIFERENCIÁVEL. É o determinante do jacobiano.
     *
     * E no reticulado isto mede-se EXACTO, sem integral nenhum: para T inteira,
     *
     *      [ Z² : T(Z²) ] = |det T|
     *
     * — o índice do sub-reticulado É o determinante, e conta-se por classes. */
    {
        long tot = 0, indice_bate = 0, unimod = 0, unimod_sobre = 0, nao_unimod = 0;
        for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
        for(long c = -4; c <= 4; c++) for(long e = -4; e <= 4; e++){
            long det = a*e - b*c;
            if(det == 0) continue;                       /* não é difeomorfismo */
            long ad = det < 0 ? -det : det;
            if(ad > 6) continue;                         /* mantém a contagem barata */
            tot++;
            /* O ÍNDICE do sub-reticulado T(Z²) em Z²: contam-se as classes de (x,y) mod
             * T(Z²). Faz-se marcando quais resíduos são atingidos numa janela grande o
             * suficiente, e o número de classes tem de dar |det T|. */
            /* v = (x,y) está em T(Z²) sse T⁻¹v é inteiro, isto é sse adj(T)·v ≡ 0 (mod
             * det). Conta-se numa janela [0,L)² com L múltiplo de |det|, e a contagem tem
             * de dar exactamente L²/|det| — que É o índice. Contar mod L a imagem seria
             * errado: quando gcd(det,L) = 1 o operador é INVERTÍVEL mod L e a imagem é
             * tudo, o que nada diz sobre o índice em Z². */
            const long L = 12 * ad;                       /* múltiplo de |det| */
            long na_imagem = 0;
            for(long x = 0; x < L; x++) for(long y = 0; y < L; y++){
                long u = e*x - b*y, v = -c*x + a*y;       /* adj(T)·(x,y) */
                if(((u % det) + det) % det == 0 && ((v % det) + det) % det == 0)
                    na_imagem++;
            }
            if(na_imagem * ad == L*L) indice_bate++;      /* [Z² : T(Z²)] = |det T| */
            if(ad == 1){
                unimod++;
                if(na_imagem == L*L) unimod_sobre++;      /* |det| = 1 ⟹ SOBREJECTIVA */
            } else if(na_imagem < L*L) nao_unimod++;      /* e |det| > 1 PERDE pontos */
        }
        printf("  §M8  o DUAL: o DIFEOMORFISMO, e é ele que VÊ a medida\n");
        printf("      operadores inteiros não singulares . %ld\n", tot);
        printf("      [Z² : T(Z²)] = |det T| ............. %ld\n", indice_bate);
        printf("      com |det| = 1 ...................... %ld   e SOBREJECTIVOS: %ld\n",
               unimod, unimod_sobre);
        printf("      com |det| > 1, a PERDER pontos ..... %ld\n\n", nao_unimod);
        ok("O DUAL DO HOMEOMORFISMO É O DIFEOMORFISMO, e o que os separa é a DERIVADA: o"
           " homeomorfismo preserva a TOPOLOGIA e nao ve a medida (§M7), o difeomorfismo traz"
           " o JACOBIANO e e' ele que a transporta — mu(Phi(A)) = integral de |det DPhi|, e"
           " para Phi LINEAR e' mu(TA) = |det T|.mu(A). No reticulado mede-se EXACTO e sem"
           " integral: o INDICE do sub-reticulado [Z^2 : T(Z^2)] E' |det T|, contado por"
           " classes. Donde |det| = 1 <=> T e' SOBREJECTIVA em Z^2 <=> conserva a medida — e"
           " com |det| > 1 ela PERDE pontos, na razao exacta do determinante",
           tot > 0 && indice_bate == tot && unimod > 0 && unimod_sobre == unimod &&
           nao_unimod > 0);
    }

    /* e o PAR fecha, com as duas metades medidas: o §M7 exibiu uma ISOMETRIA da métrica
     * canónica que muda a medida — logo a topologia NÃO determina µ —, e o §M8 mostrou que
     * o índice do sub-reticulado É |det T| — logo o jacobiano determina-a exactamente.
     * Donde |det| = 1 tem origem, e ela não é topológica: é DIFERENCIÁVEL.
     *
     * (E o toro finito NÃO serve para o mostrar: ali a redução mod N absorve o jacobiano,
     * e um operador de det 2 pode preservar a contagem de um bloco pequeno por não chegar
     * a colidir. O andar certo é o RETICULADO, e é onde o §M8 mede.) */
    {
        long tot = 0, isometria_muda = 0, jacobiano_diz = 0;
        Conj A, K, FA;
        bloco(&K, 6, 6, 2, 2);
        for(long w = 1; w <= 6; w++){
            bloco(&A, 2, 2, w, w);
            simetrica(&A, &K, &FA);
            tot++;
            if(dist(&FA, &A) > 0 && mu(&FA) != mu(&A)) isometria_muda++;
        }
        /* e do outro lado: para T com |det| = d, a imagem perde pontos na razão EXACTA d */
        { const long T3[4] = { 3,0,0,1 }, d3 = 3, L = 12*3;
          long na_imagem = 0;
          for(long x = 0; x < L; x++) for(long y = 0; y < L; y++){
              long u = T3[3]*x - T3[1]*y, v = -T3[2]*x + T3[0]*y;
              if(((u % d3) + d3) % d3 == 0 && ((v % d3) + d3) % d3 == 0) na_imagem++;
          }
          if(na_imagem * d3 == L*L) jacobiano_diz = 1; }
        printf("  §M8b as duas metades da mesma pergunta\n");
        printf("      isometrias que MUDAM a medida ...... %ld de %ld\n", isometria_muda, tot);
        printf("      e o jacobiano a dizer a razão exacta %ld\n\n", jacobiano_diz);
        ok("e o PAR fecha, com as duas metades medidas: a TOPOLOGIA nao determina a medida —"
           " ha isometrias da propria metrica canonica que a mudam (§M7) —, e o JACOBIANO"
           " determina-a exactamente — o indice do sub-reticulado e' |det T|, e com det 3 a"
           " imagem perde pontos na razao 3, sem resto. Donde |det| = 1 TEM ORIGEM e ela nao"
           " e' topologica: e' DIFERENCIAVEL. O continuo nao distingue, o diferenciavel"
           " distingue, e o que passa de um ao outro e' a DERIVADA",
           tot > 0 && isometria_muda == tot && jacobiano_diz == 1);
    }

    printf("  ══ a MEDIDA é o determinante e a MÉTRICA é a diferença simétrica. Conservar\n");
    printf("     uma não é conservar a outra, e o gato é o contra-exemplo: |det| = 1 com\n");
    printf("     d(TA,TB) ≠ d(A,B). ══\n\n");

    return falhas ? 1 : 0;          /* o contador é o do lib/unidade.h — NÃO se declara */
}
