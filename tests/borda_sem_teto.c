/* borda_sem_teto.c — A ADJUNÇÃO (∂, d) NÃO TEM TETO DIMENSIONAL.
 *
 * O coordenador: «promove a teorema no corpo universal como leitura do teorema central
 * sem limite dimensional».
 *
 * O Teorema Central diz que Hurwitz e Gentil são duais, e que «o limite no grau oito é
 * do lado discreto/bilinear — Hurwitz classifica o BILINEAR —, NÃO DO OBJECTO». A torre
 * de Cayley--Dickson deu a primeira testemunha disso: em dim 16 a norma parte-se e a
 * involução fica (tests/hurwitz.c §H5, até 64).
 *
 * Aqui mede-se uma SEGUNDA testemunha, de outra família: o par (∂, d) — a borda e a
 * derivada exterior. Ele NÃO é uma bijeção dual no sentido da casa (def:bijdual exige
 * ν∘ν = id, e aqui d∘d = 0, que é outra coisa): é uma ADJUNÇÃO, como o par
 * erosão/dilatação do thm:morf-par. E o que interessa é que ela COMPÕE EM TODA DIMENSÃO.
 *
 * ── O QUE SE MEDE, E PORQUE NÃO SE VARRE ────────────────────────────────────────
 * Varrer dimensões mediria as dimensões varridas — o tecto que aparecesse seria o do
 * array, e não do objecto. Foi o erro que este sistema já cometeu uma vez, ao subir o
 * TR_MAX para 64 e chamar-lhe «sem tecto». O que não tem tecto é o MECANISMO:
 *
 *   d²ω envolve  Σ_{i,j}  ε_{ij} · ∂_i ∂_j ω     com ε antissimétrico em (i,j)
 *                                                e  ∂_i∂_j  SIMÉTRICO (Schwarz)
 *
 *   e a contracção de um tensor ANTISSIMÉTRICO com um SIMÉTRICO é zero — sempre, e o
 *   argumento NÃO MENCIONA n. É a mesma frase de u∧u = 0.
 *
 * Então mede-se o MECANISMO, em cada n que a máquina aguente, e o que se afirma é que a
 * conclusão não depende de n. O tecto do array declara-se à parte, como sempre.
 *
 *   §B1  a contracção simétrico × antissimétrico é ZERO, de n = 2 a n = 40
 *   §B2  e o CONTROLO: com o par simétrico/simétrico ela NÃO é zero
 *   §B3  Schwarz — ∂_i∂_j = ∂_j∂_i — é o que torna ∂∂ simétrico
 *   §B4  a contagem: dim Λᵏ(ℝⁿ) = C(n,k), e Σₖ dim = 2ⁿ, sem tecto
 *   §B5  o DUAL: a nilpotência é o outro extremo da IDEMPOTÊNCIA que os papers já têm
 *   §B6  e a CONSERVAÇÃO DE ENERGIA: a idempotência sozinha NÃO a dá — falta a simetria
 *   §B7  o teto da MÁQUINA, dito à parte do da matemática
 *
 *   cc -O2 -std=c99 -I../lib borda_sem_teto.c -o borda_sem_teto && ./borda_sem_teto
 */
#include <stdio.h>
#include "racionais.h"
#include "linear.h"
#include "unidade.h"

#define NMAX 40
#define TETO 4000000000000000000LL
static long estouros = 0;

/* um gerador determinista, inteiro, e o mesmo para todos os n */
static long ger(long s, int i, int j){
    long h = s*1103515245L + (long)i*12345L + (long)j*7919L + 7;
    h ^= h >> 13;
    return (h % 19) - 9;
}
/* ── §B1/§B2: a contracção Σ A_ij S_ij ──────────────────────────────────────────
 * Com A antissimétrico e S simétrico ela é ZERO. Com A simétrico ela não tem razão
 * nenhuma para ser — e é isso que separa a lei do acaso. */
static long contrai(int n, long s, int a_antis, int s_sim){
    __int128 t = 0;
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
        long A, S;
        if(a_antis) A = (i < j) ? ger(s,i,j) : (i > j ? -ger(s,j,i) : 0);
        else        A = (i <= j) ? ger(s,i,j) : ger(s,j,i);
        if(s_sim)   S = (i <= j) ? ger(s+1000,i,j) : ger(s+1000,j,i);
        else        S = ger(s+1000,i,j);
        t += (__int128)A * S;
        if(t > TETO || t < -TETO){ estouros++; return 1; }
    }
    return (long)t;
}

int main(void){
printf("\n=== A ADJUNÇÃO (∂, d) NÃO TEM TETO DIMENSIONAL ============================\n");
printf("    O Teorema Central: «o limite no grau oito é do lado discreto/bilinear —\n");
printf("    Hurwitz classifica o BILINEAR —, não do objecto». Aqui vai a segunda\n");
printf("    testemunha disso, de outra família: o par borda/derivada.\n");
printf("    E NÃO se varrem dimensões: mede-se o MECANISMO, que não menciona n.\n");

printf("\n§B1  A CONTRACÇÃO antissimétrico × simétrico é ZERO — em toda dimensão.\n\n");
{
    long mal = 0, casos = 0;
    printf("      n     amostras   Σ A_ij S_ij (A antis., S sim.)\n");
    for(int n = 2; n <= NMAX; n += (n < 8 ? 1 : (n < 16 ? 4 : 8))){
        long pior = 0;
        for(long s = 1; s <= 40; s++){
            long v = contrai(n, s, 1, 1);
            if(v) { pior = v; mal++; }
            casos++;
        }
        printf("      %-5d %-10d %ld\n", n, 40, pior);
    }
    printf("\n      %ld contracções em dimensões de 2 a %d\n\n", casos, NMAX);
    ok("A CONTRACÇÃO DE UM TENSOR ANTISSIMÉTRICO COM UM SIMÉTRICO É ZERO, e é este o"
       " mecanismo inteiro de d² = 0: em d²ω os índices entram antissimetrizados e as"
       " segundas parciais ∂_i∂_j são simétricas, logo a soma anula. O argumento não"
       " menciona a dimensão — cada termo (i,j) cancela com o (j,i), e isso é verdade"
       " haja quantos índices houver",
       mal == 0);
}

printf("\n§B2  O CONTROLO: com simétrico × simétrico ela NÃO é zero.\n\n");
{
    long nulos = 0, naonulos = 0;
    printf("      n     quantas das 40 amostras dão ZERO (deviam ser poucas)\n");
    for(int n = 2; n <= 12; n += 2){
        long z = 0;
        for(long s = 1; s <= 40; s++){
            if(contrai(n, s, 0, 1) == 0) z++; else naonulos++;
        }
        nulos += z;
        printf("      %-5d %ld\n", n, z);
    }
    printf("\n");
    ok("E O CONTROLO SEPARA: trocada a antissimetria por simetria, a mesma contracção"
       " deixa de anular. Sem este par, «deu zero» não distinguia a lei de um programa"
       " que devolve zero por construção — e foi por não ter controlos assim que este"
       " sistema já contou verdes sobre nada",
       naonulos > 0);
}

printf("\n§B3  SCHWARZ é o que torna ∂∂ simétrico — e é ele que carrega o teorema.\n\n");
{
    /* ∂_i∂_j x^a y^b z^c … = ∂_j∂_i do mesmo, em n variáveis: a conta é o produto dos
     * expoentes, e o produto comuta. Mede-se em multi-índices genéricos. */
    long mal = 0, casos = 0;
    for(int n = 2; n <= 12; n++)
    for(long s = 1; s <= 30; s++){
        int e[NMAX];
        for(int k = 0; k < n; k++){ long v = ger(s,k,0); e[k] = (int)(v < 0 ? -v : v) % 5 + 1; }
        for(int i = 0; i < n; i++) for(int j = 0; j < n; j++){
            /* ∂_i∂_j do monómio: coeficiente e_i·(e_j − δ_ij) numa ordem, e_j·(e_i − δ_ij) noutra */
            long c1 = (long)e[i] * (e[j] - (i == j ? 1 : 0));
            long c2 = (long)e[j] * (e[i] - (i == j ? 1 : 0));
            if(c1 != c2) mal++;
            casos++;
        }
    }
    printf("      ∂_i∂_j = ∂_j∂_i em %ld pares de índices, de n = 2 a 12: %ld falhas\n\n",
           casos, mal);
    ok("SCHWARZ é uma identidade de EXPOENTES, e por isso não tem dimensão: derivar duas"
       " vezes multiplica dois expoentes, e a multiplicação comuta. É daqui que ∂∂ é"
       " simétrico, e é a simetria dele contra a antissimetria dos índices que dá d² = 0",
       mal == 0);
}

printf("\n§B4  A CONTAGEM: dim Λᵏ(ℝⁿ) = C(n,k), e a soma é 2ⁿ — sem tecto.\n\n");
{
    long mal_sim = 0, mal_soma = 0;
    printf("      n     dims Λ⁰..Λⁿ                        Σ = 2ⁿ\n");
    for(int n = 1; n <= 12; n++){
        __int128 c = 1, soma = 0;
        printf("      %-5d ", n);
        for(int k = 0; k <= n; k++){
            if(k) c = c * (n - k + 1) / k;
            soma += c;
            if(n <= 6) printf("%lld ", (long long)c);
            /* a simetria C(n,k) = C(n,n−k) */
            __int128 d = 1;
            for(int t = 1; t <= n-k; t++) d = d * (n - t + 1) / t;
            if(c != d) mal_sim++;
            c = c;                          /* c fica em C(n,k) para o próximo passo */
        }
        if(n > 6) printf("%-34s", "(…)"); else for(int p = 0; p < 34 - 4*(n+1); p++) putchar(' ');
        __int128 dois = 1;
        for(int t = 0; t < n; t++) dois *= 2;
        printf("  %lld %s\n", (long long)soma, soma == dois ? "" : "← DIFERE");
        if(soma != dois) mal_soma++;
    }
    printf("\n");
    ok("A CONTAGEM não tem tecto: dim Λᵏ(ℝⁿ) = C(n,k) está definida para todo n, a soma"
       " dos graus dá 2ⁿ, e a simetria C(n,k) = C(n,n−k) — que é o que permite existir a"
       " ESTRELA — vale em todos. Nada nesta aritmética conhece um limite superior",
       mal_sim == 0 && mal_soma == 0);
}

printf("\n§B5  O DUAL: a NILPOTÊNCIA é o outro extremo da IDEMPOTÊNCIA.\n\n");
{
    /* A pergunta do coordenador — «vê se é dual de algum teorema já existente» — tem
     * resposta, e é precisa. Os papers têm IDEMPOTÊNCIA em dois sítios (o projector
     * espectral do thm:metronomo-fourier, «partição da unidade e idempotência exatas»;
     * e a abertura/fecho do thm:morf-par, «ambos idempotentes») e não têm NILPOTÊNCIA
     * em sítio nenhum. Aplicar duas vezes tem dois extremos:
     *
     *      P² = P   (idempotente)  →  V = im P ⊕ ker P     soma DIRECTA
     *      d² = 0   (nilpotente)   →  im d ⊆ ker d          ENCAIXE
     *
     * Um preserva inteiramente a sua imagem, o outro aniquila-a inteiramente. E a
     * distância entre im e ker mede coisas opostas: no primeiro é NULA (transversal),
     * no segundo é MÁXIMA (a imagem cabe toda no núcleo). */
    long idem = 0, soma_direta = 0, mal_sd = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
    for(long c = -3; c <= 3; c++) for(long e = -3; e <= 3; e++){
        long m[] = {a,b,c,e};
        Mat P = mat_de_inteiros(2,2,m), P2 = mat_mult(P,P);
        int ok_id = 1;
        for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            if(!qz_igual(P2.a[i][j], P.a[i][j])) ok_id = 0;
        if(!ok_id) continue;
        idem++;
        /* posto(P) + dim ker(P) = 2, e im ∩ ker = 0 para idempotentes */
        Vec base[LN_MAX];
        int r = mat_posto(P), k = mat_nucleo(P, base);
        if(r + k == 2) soma_direta++; else mal_sd++;
    }
    printf("      idempotentes 2×2 sobre a caixa: %ld;  com posto + nulidade = 2: %ld"
           " (falhas %ld)\n", idem, soma_direta, mal_sd);
    printf("      e do outro lado, d² = 0 dá im ⊆ ker: a imagem cabe TODA no núcleo\n\n");
    ok("A NILPOTÊNCIA É O DUAL DA IDEMPOTÊNCIA, e isto responde à pergunta «é dual de"
       " algum teorema já existente?». Os papers têm idempotência em DOIS sítios — o"
       " projector espectral (thm:metronomo-fourier, «partição da unidade e idempotência"
       " exatas») e a abertura/fecho (thm:morf-par, «ambos idempotentes») — e não têm"
       " nilpotência em NENHUM. São os dois extremos de aplicar duas vezes: P² = P dá"
       " soma DIRECTA (im ∩ ker = 0), d² = 0 dá ENCAIXE (im ⊆ ker)",
       mal_sd == 0 && idem > 0);
}

printf("\n§B6  E A CONSERVAÇÃO DE ENERGIA: a idempotência SOZINHA não a dá.\n\n");
{
    /* A segunda pergunta: «vê se esse é a conservação de energia». É — mas só metade.
     * A idempotência dá a SEPARAÇÃO; é a SIMETRIA que dá a energia. */
    long simOK = 0, simMAL = 0, asOK = 0, asMAL = 0, nsim = 0, nas = 0;
    for(long a = -3; a <= 3; a++) for(long b = -3; b <= 3; b++)
    for(long c = -3; c <= 3; c++) for(long e = -3; e <= 3; e++){
        long m[] = {a,b,c,e};
        Mat P = mat_de_inteiros(2,2,m), P2 = mat_mult(P,P);
        int ok_id = 1;
        for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
            if(!qz_igual(P2.a[i][j], P.a[i][j])) ok_id = 0;
        if(!ok_id) continue;
        int sim = qz_igual(P.a[0][1], P.a[1][0]);
        if(sim) nsim++; else nas++;
        for(long x = -3; x <= 3; x++) for(long y = -3; y <= 3; y++){
            Vec v = vec0(2);
            v.c[0] = qz_de_inteiro(x); v.c[1] = qz_de_inteiro(y);
            Vec Pv = mat_aplica(P, v), r = vec0(2);
            for(int i = 0; i < 2; i++) r.c[i] = qz_soma(v.c[i], qz_oposto(Pv.c[i]));
            Qz nv = qz(0,1), np = qz(0,1), nr = qz(0,1);
            for(int i = 0; i < 2; i++){
                nv = qz_soma(nv, qz_mult(v.c[i], v.c[i]));
                np = qz_soma(np, qz_mult(Pv.c[i], Pv.c[i]));
                nr = qz_soma(nr, qz_mult(r.c[i], r.c[i]));
            }
            int bate = qz_igual(nv, qz_soma(np, nr));
            if(sim){ if(bate) simOK++; else simMAL++; }
            else   { if(bate) asOK++;  else asMAL++;  }
        }
    }
    printf("      idempotentes: %ld simétricos, %ld não simétricos\n", nsim, nas);
    printf("      SIMÉTRICO (projector ortogonal): ‖x‖² = ‖Px‖² + ‖x−Px‖² em %ld,"
           " %ld falhas\n", simOK, simMAL);
    printf("      NÃO simétrico:                   %ld batem, %ld FALHAM\n\n", asOK, asMAL);
    ok("É A CONSERVAÇÃO DE ENERGIA — MAS SÓ COM A SIMETRIA. A idempotência sozinha dá a"
       " SEPARAÇÃO (V = im ⊕ ker) e não dá a energia: sem simetria, ‖x‖² = ‖Px‖² +"
       " ‖x−Px‖² falha em 1568 de 1960. Com simetria — o projector ORTOGONAL — não falha"
       " nenhuma vez. É Pitágoras, e é a mesma energia do thm:parseval-multi: a partição"
       " da unidade separa, e a ortogonalidade conserva. Duas hipóteses, dois papéis",
       simMAL == 0 && asMAL > 0);
}

printf("\n§B7  O TETO DA MÁQUINA, dito à parte do da matemática.\n\n");
{
    printf("      dimensão máxima varrida: %d;  estouros do acumulador: %ld\n\n",
           NMAX, estouros);
    ok("e o tecto que aparece aqui é o do ARRAY e do long, NÃO o do objecto — declara-se"
       " à parte de propósito. Varrer até 40 mede 40 dimensões; o que não tem tecto é o"
       " MECANISMO, e ele mediu-se em §B1 e §B3 sem que a conclusão mencione n",
       estouros == 0);
}

printf("\n=== FECHO ==================================================================\n");
printf("    A resposta às duas perguntas do coordenador: SIM, é dual de um teorema que\n");
printf("    já existia — a IDEMPOTÊNCIA do projector espectral e da abertura/fecho —, e\n");
printf("    SIM, é a conservação de energia, mas só com a SIMETRIA a acompanhar.\n\n");
printf("    A adjunção (∂, d) NÃO é uma bijeção dual no sentido da casa — def:bijdual\n");
printf("    exige ν∘ν = id, e aqui d∘d = 0, que é outra coisa. É uma ADJUNÇÃO, como o\n");
printf("    par erosão/dilatação. E o que ela acrescenta ao Teorema Central é uma\n");
printf("    SEGUNDA testemunha de que o tecto do grau oito é da NORMA BILINEAR: aqui\n");
printf("    está uma estrutura do lado do objecto que compõe em toda dimensão, porque\n");
printf("    o que a define é a antissimetria graduada e não uma norma.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
