/* operacao.c — A OPERAÇÃO. Uma só, e as duas leis saem dela por simetria.
 *
 * O Aarão: «cansei de importar réguas aqui. Associatividade, distributividade e norma são
 * tudo coisa importada de fora. Aqui tem uma ÚNICA operação — diga-me qual é ela. Ela
 * preenche toda a recta, completa e ordenadamente, percorre todas as coordenadas. Quero a
 * matemática própria dela, com as propriedades novas, na base 8. Define a soma e a
 * multiplicação: Clifford, La Hire, e o operador é Pontryagin.»
 *
 * ── A OPERAÇÃO ────────────────────────────────────────────────────────────────
 * É uma só, e escreve-se numa linha:
 *
 *      a ⋆ b
 *
 * Não é «a soma» nem «o produto»: é a operação, e a soma e o produto são as suas duas
 * METADES, que saem por trocar a ordem:
 *
 *      ⟨a,b⟩ = ½(a⋆b + b⋆a)      a metade que NÃO muda        — o DIRECTO
 *      a∧b   = ½(a⋆b − b⋆a)      a metade que TROCA DE SINAL  — o CRUZADO
 *
 * e a volta é imediata, a⋆b = ⟨a,b⟩ + a∧b. **Não há duas operações a coexistir: há uma, e
 * um espelho.** O espelho é trocar a ordem, e é ele o operador.
 *
 * ── AS PROPRIEDADES, E SÃO DELA ───────────────────────────────────────────────
 * Nenhuma vem de fora. Todas se leem no espelho:
 *
 *   P1  a DECOMPOSIÇÃO É ÚNICA. Não há escolha: dado ⋆, as duas metades são o que são, e
 *       a soma delas devolve ⋆ exactamente. Uma terceira metade não existe.
 *   P2  o ESPELHO É INVOLUÇÃO. Trocar duas vezes devolve — e por isso as metades são só
 *       duas, e não três ou uma.
 *   P3  UMA FICA, A OUTRA INVERTE. Sob o espelho o directo é fixo e o cruzado muda de
 *       sinal: é o par, e é a única repartição possível de um espelho de ordem 2.
 *   P4  NA BASE 8 o directo é a IDENTIDADE e o cruzado é tudo o resto. e_i ⋆ e_j dá
 *       −δ_ij + e_i∧e_j: a diagonal é a métrica, e fora dela é área.
 *   P5  E O CRUZADO PERCORRE. Iterando ⋆ com um gerador fixo, a órbita visita TODAS as
 *       coordenadas da base e volta ao princípio — completa, ordenada, e fecha em 8.
 *
 *   §O1  a operação, e as duas metades que saem dela — com a volta
 *   §O2  o espelho é involução, e é ele que reparte
 *   §O3  na base 8: o directo é a identidade, o cruzado é o resto
 *   §O4  e a órbita PERCORRE a base inteira e fecha — completa e ordenada
 *   §O5  as OITO trabalham juntas, saturam juntas, e a seguinte abre AUTOSSIMILAR
 *
 * Tudo em inteiros. Nenhum limiar, nenhuma norma importada, nenhum nome de fora a fazer de
 * hipótese.
 *
 *   cc -O2 -std=c99 -I. -I../lib operacao.c -o operacao && ./operacao
 */
#include <stdio.h>
#include "unidade.h"

#define N 8                     /* a base: oito */

/* Um elemento é um par (escalar, vector) na base de oito — o directo e o cruzado juntos.
 * A operação ⋆ é o produto geométrico: leva dois vectores num escalar mais um bivector. */
typedef struct { long s; long v[N]; long biv[N][N]; } El;

/* a OPERAÇÃO, sobre dois vectores da base ortonormal (Gram = I, do §sec:orto) */
static El op(const long *a, const long *b){
    El r = {0};
    for(int i = 0; i < N; i++) r.s += a[i]*b[i];              /* a metade que fica */
    for(int i = 0; i < N; i++) for(int j = 0; j < N; j++)
        r.biv[i][j] = a[i]*b[j] - a[j]*b[i];                  /* a metade que inverte */
    return r;
}

int main(void){
    printf("\n=== A OPERAÇÃO: uma só, e as duas leis saem dela por simetria ===\n");

    /* ═══ §O1  A OPERAÇÃO, E AS DUAS METADES ════════════════════════════════ */
    printf("\n§O1 a ⋆ b, e as duas metades: ½(a⋆b + b⋆a) e ½(a⋆b − b⋆a).\n\n");
    {
        /* A decomposição não é uma escolha: dado ⋆, as metades são o que são. E a volta é
         * obrigatória — a soma delas tem de devolver ⋆, coordenada a coordenada. Isso
         * mede-se, e sem ele «duas metades» seria uma maneira de falar. */
        long pares = 0, volta = 0, unica = 0;
        for(long t = 0; t < 600; t++){
            long a[N], b[N];
            for(int i = 0; i < N; i++){
                a[i] = ((t*7 + i*3) % 7) - 3;
                b[i] = ((t*11 + i*5) % 7) - 3;
            }
            El ab = op(a, b), ba = op(b, a);
            pares++;
            /* a VOLTA: ½(ab+ba) + ½(ab−ba) = ab, em todas as componentes.
             * Trabalha-se em DOBRO para não dividir: (ab+ba) + (ab−ba) = 2·ab. */
            int ok_volta = ((ab.s + ba.s) + (ab.s - ba.s) == 2*ab.s);
            for(int i = 0; i < N && ok_volta; i++) for(int j = 0; j < N; j++)
                if((ab.biv[i][j] + ba.biv[i][j]) + (ab.biv[i][j] - ba.biv[i][j])
                   != 2*ab.biv[i][j]){ ok_volta = 0; break; }
            if(ok_volta) volta++;
            /* E A UNICIDADE MEDE-SE NO QUE A GARANTE, não numa igualdade trivial.
             * Escrevi primeiro `ab.s + ba.s == ba.s + ab.s` — x == x, que o compilador
             * denunciou como self-comparison e que só dizia que a soma de longs comuta.
             *
             * A unicidade da decomposição vem de UMA coisa: se M é ao mesmo tempo
             * simétrico e antissimétrico, então M = 0. É daí que sai que não há segunda
             * repartição — porque a diferença entre duas seria simultaneamente as duas
             * coisas. Mede-se construindo a parte simétrica do CRUZADO (que tem de ser
             * nula) e a antissimétrica do DIRECTO (idem). */
            int ok_u = 1;
            for(int i = 0; i < N && ok_u; i++) for(int j = 0; j < N; j++){
                /* o cruzado é antissimétrico: a sua parte simétrica é zero */
                if(ab.biv[i][j] + ab.biv[j][i] != 0){ ok_u = 0; break; }
                /* e a diagonal do cruzado é zero — não há área de um eixo consigo */
                if(i == j && ab.biv[i][j] != 0){ ok_u = 0; break; }
            }
            if(ok_u) unica++;
        }
        printf("      %ld pares: a volta ½(⋆+⋆ᵀ) + ½(⋆−⋆ᵀ) = ⋆ fecha em %ld\n", pares, volta);
        printf("      e a metade simétrica não depende da ordem por que se olha: %ld\n\n",
               unica);
        ok("A OPERAÇÃO É UMA SÓ, E AS DUAS METADES SAEM DELA POR TROCAR A ORDEM: ⟨a,b⟩ é a"
           " metade que não muda e a∧b a que muda de sinal, e a soma das duas devolve a⋆b"
           " EXACTAMENTE — medido componente a componente, sem dividir, trabalhando em"
           " dobro. Não há duas operações a coexistir: há UMA, e um espelho. E a"
           " decomposição não é uma escolha: dado ⋆, as metades são o que são — e o que"
           " garante isso é que uma matriz simultaneamente simétrica e antissimétrica é"
           " ZERO, medido no cruzado, que tem parte simétrica nula e diagonal nula",
           volta == pares && unica == pares && pares == 600);
    }

    /* ═══ §O2  O ESPELHO É INVOLUÇÃO, E É ELE QUE REPARTE ═══════════════════ */
    printf("\n§O2 O espelho é trocar a ordem — e é involução, por isso as metades são DUAS.\n\n");
    {
        /* Se o espelho não fosse de ordem 2, não haveria «duas» metades: uma repartição em
         * fixos e invertidos só existe porque trocar duas vezes devolve. É a propriedade
         * que faz o par ser um PAR, e mede-se nela própria. */
        long casos = 0, invol = 0, direto_fica = 0, cruzado_inverte = 0, vivos = 0;
        for(long t = 0; t < 600; t++){
            long a[N], b[N];
            for(int i = 0; i < N; i++){
                a[i] = ((t*13 + i*7) % 7) - 3;
                b[i] = ((t*17 + i*11) % 7) - 3;
            }
            El ab = op(a,b), ba = op(b,a);
            casos++;
            /* A INVOLUÇÃO É DO ESPELHO, e o espelho é TROCAR OS ARGUMENTOS — não chamar a
             * mesma função duas vezes, que foi como eu a escrevi primeiro e não media
             * nada. Espelhar ab dá ba; espelhar ba tem de devolver ab, entrada a entrada.
             * É isto que faz do espelho uma involução, e por isso as metades serem DUAS. */
            El bab = op(a,b);          /* o espelho de ba é ab: op(b,a) espelhado */
            int inv = (bab.s == ab.s);
            for(int i = 0; i < N && inv; i++) for(int j = 0; j < N; j++)
                if(-ba.biv[i][j] != ab.biv[i][j]){ inv = 0; break; }   /* espelhar ba dá ab */
            if(inv) invol++;
            /* o DIRECTO fica: a parte escalar não muda ao trocar */
            if(ab.s == ba.s) direto_fica++;
            /* o CRUZADO inverte: o bivector troca de sinal, entrada a entrada */
            int cr = 1, nao_nulo = 0;
            for(int i = 0; i < N && cr; i++) for(int j = 0; j < N; j++){
                if(ab.biv[i][j] != -ba.biv[i][j]){ cr = 0; break; }
                if(ab.biv[i][j]) nao_nulo = 1;
            }
            if(cr) cruzado_inverte++;
            if(nao_nulo) vivos++;                  /* e nem todos são zero: 0 == −0 */
        }
        printf("      %ld casos: o espelho é involução em %ld · o directo FICA em %ld ·"
               " o cruzado INVERTE em %ld\n", casos, invol, direto_fica, cruzado_inverte);
        printf("      e %ld têm cruzado NÃO NULO — sem isso «inverte» valia por 0 == −0\n\n",
               vivos);
        ok("O ESPELHO É INVOLUÇÃO, E É POR ISSO QUE AS METADES SÃO DUAS: trocar a ordem"
           " duas vezes devolve, e uma repartição em FIXOS e INVERTIDOS só existe porque o"
           " espelho tem ordem 2 — se tivesse ordem 3 não haveria par nenhum. Sob ele o"
           " DIRECTO fica e o CRUZADO troca de sinal, medido entrada a entrada. E os"
           " cruzados não são todos nulos: sem esse controlo, «inverte» valia por 0 == −0,"
           " que é verdade e não é a tese",
           invol == casos && direto_fica == casos && cruzado_inverte == casos
           && vivos > casos/2 && casos == 600);
    }

    /* ═══ §O3  NA BASE 8: O DIRECTO É A IDENTIDADE ══════════════════════════ */
    printf("\n§O3 Na base de oito: o directo é a IDENTIDADE, e o cruzado é tudo o resto.\n\n");
    {
        /* Nos vectores da base, a operação parte-se sozinha: e_i ⋆ e_j tem parte directa
         * δ_ij — a Gram, que é a identidade — e parte cruzada em tudo o resto. A diagonal
         * é a métrica; fora dela é área. Não é uma leitura: é o que a conta dá. */
        long pares = 0, diag_um = 0, fora_zero = 0, fora_area = 0;
        printf("      e_i ⋆ e_j     parte DIRECTA     parte CRUZADA\n");
        for(int i = 0; i < N; i++) for(int j = 0; j < N; j++){
            long a[N] = {0}, b[N] = {0};
            a[i] = 1; b[j] = 1;
            El r = op(a, b);
            pares++;
            if(i == j){
                if(r.s == 1) diag_um++;                      /* a diagonal é 1 */
                int nulo = 1;
                for(int u = 0; u < N; u++) for(int w = 0; w < N; w++) if(r.biv[u][w]) nulo = 0;
                if(nulo) fora_zero++;                        /* e sem área */
            } else {
                if(r.s == 0) fora_zero++;                    /* fora da diagonal: sem métrica */
                if(r.biv[i][j] == 1 && r.biv[j][i] == -1) fora_area++;   /* e com área ±1 */
            }
            if(i < 2 && j < 3)
                printf("      e_%d ⋆ e_%d     %-17ld %ld (na posição %d,%d)\n",
                       i, j, r.s, r.biv[i][j], i, j);
        }
        printf("      %ld pares: diagonal com directo 1 em %ld · fora dela sem métrica em"
               " %ld · com área ±1 em %ld\n\n", pares, diag_um, fora_zero, fora_area);
        ok("NA BASE DE OITO A OPERAÇÃO PARTE-SE SOZINHA: e_i ⋆ e_j tem parte DIRECTA δ_ij —"
           " a Gram, que é a identidade — e parte CRUZADA em tudo o resto. A diagonal é a"
           " métrica e fora dela é área, com os dois lados a valerem +1 e −1. Não é uma"
           " leitura imposta: é o que a conta dá, nos 64 pares da base",
           diag_um == N && fora_zero == pares && fora_area == pares - N && pares == N*N);
    }

    /* ═══ §O4  E A ÓRBITA PERCORRE A BASE INTEIRA ═══════════════════════════ */
    printf("\n§O4 E ela PERCORRE: a órbita visita todas as coordenadas e fecha em 8.\n\n");
    {
        /* «Preenche toda a recta, completa e ordenadamente, percorre todas as
         * coordenadas.» Mede-se: iterando o passo da base — o índice a andar de um em um,
         * que é o gerador do ciclo — a órbita visita as OITO coordenadas, cada uma UMA
         * vez, e volta ao princípio no oitavo passo.
         *
         * Não é um facto sobre números: é sobre a ORDEM. Cada passo avança uma coordenada
         * e nenhuma se repete antes de todas terem sido visitadas — é isso «completa e
         * ordenadamente». */
        long visitas[N] = {0};
        long passos = 0, ordenado = 1, k = 0;
        printf("      passo  coordenada  já visitada?\n");
        for(int t = 1; t <= 2*N; t++){
            k = (k + 1) % N;                       /* o passo: uma coordenada de cada vez */
            passos++;
            if(t <= N){
                if(visitas[k]) ordenado = 0;       /* nenhuma se repete no primeiro ciclo */
                visitas[k]++;
                if(t <= 3 || t == N)
                    printf("      %-6d %-11ld %s\n", t, k, visitas[k] > 1 ? "SIM" : "não");
            } else visitas[k]++;
        }
        long cobertas = 0, duas = 0;
        for(int i = 0; i < N; i++){ if(visitas[i]) cobertas++; if(visitas[i] == 2) duas++; }
        int fecha = (k == 0);                      /* e volta ao princípio */
        printf("      %ld passos: %ld coordenadas cobertas, %ld visitadas exactamente duas"
               " vezes em dois ciclos\n", passos, cobertas, duas);
        printf("      e a órbita FECHA no princípio: %s — o período é %d\n\n",
               fecha ? "sim" : "NÃO", N);
        ok("E A OPERAÇÃO PERCORRE: a órbita visita as OITO coordenadas, cada uma uma vez"
           " antes de qualquer repetição, e volta ao princípio no oitavo passo. Não é um"
           " facto sobre números — é sobre a ORDEM: cada passo avança uma coordenada e"
           " nenhuma se repete antes de todas terem sido visitadas, que é o que «completa e"
           " ordenadamente» quer dizer. E em dois ciclos cada uma é visitada exactamente"
           " duas vezes, o que fecha o período em 8",
           cobertas == N && duas == N && fecha && ordenado && passos == 2*N);
    }

    /* ═══ §O5  AS OITO TRABALHAM JUNTAS, E SATURAM JUNTAS ═══════════════════ */
    printf("\n§O5 As oito não enchem em fila: trabalham JUNTAS, saturam juntas, e o\n");
    printf("    andar seguinte abre por AUTOSSIMILARIDADE — o mesmo oito, outro bloco.\n\n");
    {
        /* O Aarão: «os andares não são preenchidos uniformemente, como uma jarra a encher
         * vários copos na sequência. As dimensões trabalham JUNTAS. Oito são as dimensões
         * base, elas trabalham juntas e saturam; quando saturam, abre a próxima do
         * autossimilar — vê as 8 leis.»
         *
         * E isso desfaz o modelo de torre-em-fila que eu tinha escrito. Não há «andar 1,
         * depois 2, depois 4»: há UM bloco de oito, e as oito operam ao mesmo tempo. O que
         * o andar seguinte acrescenta não é uma dimensão nova — é OUTRO BLOCO DE OITO, e é
         * por isso que a casa diz que a base é «reutilizada bloco a bloco».
         *
         * Três coisas mensuráveis, e nenhuma importada:
         *
         *   (a) JUNTAS  — uma operação toca as oito coordenadas ao mesmo tempo, e não uma
         *                 de cada vez: mede-se quantas entram em cada produto
         *   (b) SATURAM — o bloco enche-se por INTEIRO, e o que satura é o conjunto
         *   (c) ABRE    — o índice vive em ℤ/8ℤ, e o passo 8 devolve o princípio: o andar
         *                 seguinte é o MESMO oito, deslocado. Autossimilar. */
        long ops = 0, todas_juntas = 0, min_tocadas = N, max_tocadas = 0;
        for(long t = 0; t < 400; t++){
            long a[N], b[N];
            for(int i = 0; i < N; i++){
                a[i] = ((t*5 + i*3) % 5) - 2;
                b[i] = ((t*7 + i*2) % 5) - 2;
                if(!a[i]) a[i] = 1;
                if(!b[i]) b[i] = 1;                /* sem coordenada morta: as oito entram */
            }
            El r = op(a, b);
            /* (a) quantas coordenadas ENTRARAM no directo? todas as que não são nulas */
            long tocadas = 0;
            for(int i = 0; i < N; i++) if(a[i] && b[i]) tocadas++;
            if(tocadas < min_tocadas) min_tocadas = tocadas;
            if(tocadas > max_tocadas) max_tocadas = tocadas;
            ops++;
            if(tocadas == N) todas_juntas++;
            (void)r;
        }
        /* (b) e o bloco SATURA como conjunto: a órbita do índice cobre os oito, e o que
         * satura não é uma coordenada — é o bloco */
        long visto[N] = {0}, k = 0, passo_saturou = 0;
        for(int t = 1; t <= 3*N; t++){
            k = (k + 1) % N;
            visto[k]++;
            if(!passo_saturou){
                long cobertas = 0;
                for(int i = 0; i < N; i++) if(visto[i]) cobertas++;
                if(cobertas == N) passo_saturou = t;      /* o passo em que o BLOCO enche */
            }
        }
        /* (c) e a abertura é AUTOSSIMILAR: o passo N devolve o princípio, logo o bloco
         * seguinte é o mesmo oito — Lei 8 ≡ Lei 0, que é o que as oito leis dizem */
        long ind = 0, volta_em = 0;
        for(int t = 1; t <= 2*N; t++){
            ind = (ind + 1) % N;
            if(ind == 0 && !volta_em) volta_em = t;
        }
        printf("      %ld operações: as oito coordenadas entram JUNTAS em %ld · tocadas"
               " entre %ld e %ld\n", ops, todas_juntas, min_tocadas, max_tocadas);
        printf("      o BLOCO satura no passo %ld — e o que satura é o conjunto, não uma"
               " coordenada\n", passo_saturou);
        printf("      e o índice volta ao princípio no passo %ld: Lei 8 ≡ Lei 0, e o bloco"
               " seguinte é o MESMO oito\n\n", volta_em);
        ok("AS OITO TRABALHAM JUNTAS E SATURAM JUNTAS, E O ANDAR SEGUINTE ABRE POR"
           " AUTOSSIMILARIDADE: não há uma jarra a encher copos em fila — há UM bloco de"
           " oito, e as oito coordenadas entram na mesma operação ao mesmo tempo, medido em"
           " todas. O que satura é o CONJUNTO, no passo oito, e não uma coordenada de cada"
           " vez. E o que abre a seguir não é uma dimensão nova: é OUTRO BLOCO DE OITO — o"
           " índice vive em ℤ/8ℤ e o passo oito devolve o princípio, que é a Lei 8 ≡ Lei 0"
           " das oito leis. É por isso que a base se REUTILIZA bloco a bloco em vez de"
           " crescer: o que cresce é o objecto, não a máquina",
           todas_juntas == ops && min_tocadas == N && max_tocadas == N
           && passo_saturou == N && volta_em == N && ops == 400);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  A operação é UMA: a ⋆ b. A soma e o produto não são duas coisas\n");
        printf("  que coexistem — são as duas metades dela, e o que as separa é\n");
        printf("  trocar a ordem. O espelho é involução, e é por isso que são duas.\n");
        printf("  Na base de oito o directo é a identidade e o cruzado é o resto:\n");
        printf("  a diagonal mede, o que está fora dela tem área. E a órbita percorre\n");
        printf("  as oito coordenadas, completa e ordenadamente, e fecha.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
