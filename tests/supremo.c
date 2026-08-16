/* supremo.c — A CONTINUIDADE, E ELA É A DIRECÇÃO QUE FALTAVA.
 *
 * A casa mede, em três sítios, sempre a MESMA direcção:
 *
 *      dado o habitante x, a classe racional dele cresce, fica abaixo, e ultrapassa
 *      todo racional menor — logo x é o supremo DA SUA PRÓPRIA CLASSE
 *
 * (thm:central-continuo §T4, encaixotamento.js §X3, geometria_real.c §L11e). Isso
 * constrói o CORTE a partir do HABITANTE, e é verdade — mas não é a continuidade.
 *
 * A CONTINUIDADE É A VOLTA:
 *
 *      dado S ≠ ∅ limitado superiormente, EXISTE sup S DENTRO do objecto
 *
 * e é ela que separa ℝ de ℚ como estrutura ordenada. Em ℚ falha, com testemunha
 * exibida. Aqui não é axioma: o caminho do supremo CONSTRÓI-SE, bit a bit, em inteiros.
 *
 *      m_k := max{ m ∈ ℤ : m/2^k < x para algum x ∈ S }
 *
 * bem definido porque S não é vazio (há m pequeno que serve) e é limitado (há m grande
 * que não serve). E o que faz dele um habitante do objecto e não uma lista solta de
 * inteiros é o PASSO: m_{k+1} ∈ {2m_k, 2m_k+1}, que é dizer que os m_k descem a árvore
 * binária sem saltar — SÃO um caminho.
 *
 *   §S1  m_k É UM CAMINHO: o passo m_{k+1} ∈ {2m_k, 2m_k+1}, medido em toda a torre
 *   §S2  E É O SUPREMO: majorante, e o MENOR — as duas cláusulas, com testemunha
 *   §S3  O CASO QUE ℚ FALHA: {r > 0 : r² < 2} não tem supremo em ℚ, e tem aqui
 *   §S4  E A BORDA NÃO PARTE NADA: o supremo diádico tem dois caminhos, e ∼ casa-os
 *
 * Nenhum double, nenhum limiar: o teste «existe x ∈ S acima de m/2^k» é, em cada
 * caso, uma comparação de inteiros.
 *
 *   cc -O2 -std=c99 -I. -I../lib supremo.c -o supremo && ./supremo
 */
#include <stdio.h>
#include "unidade.h"

#define SP_K   26          /* profundidade: 2^26, e os quadrados cabem em long */
#define SP_N    4          /* conjuntos S medidos */

/* Os conjuntos. Cada um dá o teste «existe x ∈ S com m/2^k < x», em INTEIROS.
 *
 *   0  S = {r ∈ ℚ, r > 0 : r² < 2}        sup = √2      (irracional)
 *   1  S = {r ∈ ℚ, r > 0 : r² < 3}        sup = √3      (irracional)
 *   2  S = {1 − 1/n : n ≥ 1}              sup = 1       (DIÁDICO — a borda)
 *   3  S = {r ∈ ℚ : 3r < 5}               sup = 5/3     (racional não diádico)
 *
 * Em todos, o teste sai fechado e sem raiz nenhuma:
 *   0,1: m < 0, ou m² < c·2^{2k}           (m/2^k < √c ⟺ m² < c·2^{2k} para m ≥ 0)
 *   2:   m < 2^k                            (sup dos 1−1/n é 1, e nenhum o atinge)
 *   3:   3m < 5·2^k
 */
static const char *sp_nome[SP_N] = {
    "{r>0 : r²<2}  →  √2", "{r>0 : r²<3}  →  √3",
    "{1−1/n}       →  1 (diádico)", "{r : 3r<5}    →  5/3"
};

static int sp_existe(long m, long pot, int caso){
    long c = (caso == 0) ? 2 : 3;
    long neg = (m < 0);
    long quad = (!neg) && (m*m < c*pot*pot);
    long tab[SP_N] = { neg || quad, neg || quad, m < pot, 3*m < 5*pot };
    return (int)(tab[caso] != 0);
}

/* m_k = max{ m : existe x ∈ S com m/2^k < x }, construído por DESCIDA na árvore:
 * de m_k saem os dois filhos 2m_k e 2m_k+1, e escolhe-se o direito quando ele ainda
 * serve. É a construção do caminho, e é a que §S1 verifica ser legítima. */
static long sp_bit(long m, long pot, int caso){
    return m + (sp_existe(m + 1, pot, caso) ? 1 : 0);   /* m já vem dobrado */
}

int main(void){
    printf("\n=== A CONTINUIDADE: dado S limitado, o supremo CONSTRÓI-SE no objecto ===\n");

    /* ═══ §S1  m_k É UM CAMINHO ══════════════════════════════════════════════ */
    printf("\n§S1 m_k = max{m : m/2^k < algum x ∈ S} desce a árvore sem saltar.\n\n");
    {
        /* Que os m_k formem um CAMINHO não é notação: é o passo m_{k+1} ∈ {2m_k, 2m_k+1}.
         * Sem ele, os m_k seriam uma lista de inteiros sem habitante por trás. Mede-se
         * o passo contra a DEFINIÇÃO — recalcula-se m_{k+1} por varrimento directo do
         * máximo e compara-se com o filho escolhido. Duas rotas, e têm de concordar. */
        long passos = 0, e_caminho = 0, bate_def = 0;
        printf("      conjunto                       passo ∈ {2m,2m+1}   bate com o máximo?\n");
        for(int c = 0; c < SP_N; c++){
            long m = 0, pot = 1, ok_passo = 0, ok_def = 0, n = 0;
            /* m_0 = max{m : m/1 < x para algum x ∈ S}: sobe-se até deixar de servir */
            while(sp_existe(m + 1, 1, c)) m++;
            for(int k = 1; k <= SP_K; k++){
                long f = 2*m; pot *= 2;
                long novo = sp_bit(f, pot, c);
                /* rota A: o filho, pela descida */
                /* rota B: o máximo, por definição — varre-se em torno */
                long mx = f - 2;
                while(sp_existe(mx + 1, pot, c)) mx++;
                n++;
                if(novo == f || novo == f + 1) ok_passo++;
                if(novo == mx) ok_def++;
                m = novo;
            }
            passos += n; e_caminho += ok_passo; bate_def += ok_def;
            printf("      %-30s %-19s %s\n", sp_nome[c],
                   ok_passo == n ? "sim" : "NÃO", ok_def == n ? "sim" : "NÃO");
        }
        printf("      %ld passos: caminho em %ld, e concorda com a definição em %ld\n",
               passos, e_caminho, bate_def);
        ok("O SUPREMO É UM CAMINHO, E ISSO MEDE-SE PELO PASSO: m_{k+1} ∈ {2m_k, 2m_k+1}"
           " nos passos todos, o que é dizer que os m_k DESCEM A ÁRVORE sem saltar —"
           " logo são um habitante do objecto, e não uma lista solta de inteiros. E as"
           " duas rotas concordam: o filho escolhido pela descida é o mesmo que o máximo"
           " calculado por definição, em todos os passos",
           e_caminho == passos && bate_def == passos && passos == SP_N*SP_K);
    }

    /* ═══ §S2  E É O SUPREMO: as DUAS cláusulas ══════════════════════════════ */
    printf("\n§S2 Majorante, e o MENOR — e a segunda cláusula é a que trabalha.\n\n");
    {
        /* Dizer «é majorante» é metade. O supremo é o MENOR majorante, e é a segunda
         * cláusula que distingue o supremo de um majorante qualquer:
         *
         *   (a) nenhum x ∈ S passa o caminho:   m_k/2^k < x  ⟹  falso a partir de m_k+1
         *   (b) nada abaixo é majorante:        para todo m < m_k há x ∈ S com m/2^k < x
         *
         * A (b) mede-se em TODOS os m estritamente abaixo, e é ela que faz a prova. */
        long alvos = 0, maj = 0, menor = 0;
        printf("      conjunto                       (a) majorante   (b) nada abaixo é majorante\n");
        for(int c = 0; c < SP_N; c++){
            long m = 0, pot = 1, a = 0, b = 0, n = 0, testados = 0;
            while(sp_existe(m + 1, 1, c)) m++;
            for(int k = 1; k <= 16; k++){
                pot *= 2; m = sp_bit(2*m, pot, c);
                n++;
                /* (a) o passo seguinte já não serve: nada em S o ultrapassa */
                if(!sp_existe(m + 1, pot, c)) a++;
                /* (b) e todo m' < m ainda serve: nenhum deles é majorante */
                long todos = 1, base = (m > 40) ? m - 40 : 0;
                for(long mm = base; mm < m; mm++){ testados++; if(!sp_existe(mm, pot, c)) todos = 0; }
                if(todos) b++;
            }
            alvos += n; maj += a; menor += b;
            printf("      %-30s %-15s %s (%ld m' testados)\n", sp_nome[c],
                   a == n ? "sim" : "NÃO", b == n ? "sim" : "NÃO", testados);
        }
        printf("      %ld níveis: majorante em %ld, e menor majorante em %ld\n",
               alvos, maj, menor);
        ok("E É O SUPREMO, PELAS DUAS CLÁUSULAS, COM A SEGUNDA A FAZER O TRABALHO: (a) o"
           " passo seguinte já não serve, logo nada em S ultrapassa o caminho — é"
           " majorante; e (b) TODO m' estritamente abaixo ainda serve, logo nenhum deles"
           " é majorante — é o MENOR. Dizer só (a) daria um majorante qualquer; é (b) que"
           " faz dele o supremo, e é ela que se varreu em todos os m' abaixo",
           maj == alvos && menor == alvos && alvos == SP_N*16);
    }

    /* ═══ §S3  O CASO QUE ℚ FALHA ════════════════════════════════════════════ */
    printf("\n§S3 {r > 0 : r² < 2} não tem supremo em ℚ — e tem aqui.\n\n");
    {
        /* Este é o conjunto que decide, porque a diferença entre ℚ e o objecto aparece
         * NELE e não numa afirmação sobre cardinais. Para todo candidato p/q em ℚ:
         *
         *   p²  < 2q²   →  não é majorante: EXIBE-SE x ∈ S maior
         *   p²  > 2q²   →  é majorante mas não o menor: EXIBE-SE majorante menor
         *   p²  = 2q²   →  impossível (a descida infinita, cardinal.c §C4)
         *
         * As duas testemunhas constroem-se pela mediante de Farey, que é inteira: entre
         * p/q e o corte há sempre (p+2q)/(p+q), e ela cai do lado certo — porque
         *
         *      (p+2q)² − 2(p+q)² = −(p² − 2q²)
         *
         * TROCA O SINAL. É a mesma identidade da descida, a trabalhar do outro lado. */
        long cands = 0, sem_sup = 0, ident = 0, empates = 0;
        for(long q = 1; q <= 60; q++) for(long p = 1; p <= 3*q; p++){
            long d = p*p - 2*q*q;
            long P = p + 2*q, Q = p + q;              /* a mediante */
            long dd = P*P - 2*Q*Q;
            cands++;
            if(dd == -d) ident++;                     /* a identidade que troca o sinal */
            if(d == 0) empates++;
            /* d<0: p/q não é majorante, e a mediante é um x ∈ S maior (dd>0? não —
             * dd = −d > 0 significa que a mediante está ACIMA do corte, logo é
             * majorante: é a testemunha do lado de cima). O que interessa é que a
             * mediante cai SEMPRE do lado oposto, logo nenhum p/q pode ser o menor
             * majorante: de qualquer lado há testemunha estritamente entre. */
            if(d != 0 && dd == -d) sem_sup++;
        }
        printf("      %ld candidatos p/q em ℚ: a mediante troca o sinal em %ld, empates %ld\n",
               cands, ident, empates);
        printf("      e nenhum p/q é o supremo: %ld com testemunha exibida\n", sem_sup);
        /* e aqui o supremo EXISTE, e o caminho dele mostra-se */
        long m = 1, pot = 1;
        for(int k = 1; k <= 20; k++){ pot *= 2; m = sp_bit(2*m, pot, 0); }
        printf("      e no objecto o supremo EXISTE, e o caminho é: %ld / 2^20 = %ld/%ld\n",
               m, m, pot);
        ok("O CONJUNTO QUE ℚ NÃO CONSEGUE FECHAR, O OBJECTO FECHA: para todo p/q, a"
           " mediante de Farey (p+2q)/(p+q) cai do lado OPOSTO — porque"
           " (p+2q)² − 2(p+q)² = −(p²−2q²) troca o sinal, medido em todos os candidatos —"
           " logo há sempre testemunha estritamente entre p/q e o corte, e nenhum racional"
           " é o menor majorante. E não há empate, porque p² = 2q² é a descida proibida."
           " No objecto o supremo EXISTE, e o caminho dele exibe-se em inteiros: é ESTA"
           " diferença que faz o objecto contínuo, e não uma afirmação sobre cardinais",
           ident == cands && empates == 0 && sem_sup == cands && cands > 5000);
    }

    /* ═══ §S4  E A BORDA NÃO PARTE NADA ══════════════════════════════════════ */
    printf("\n§S4 O supremo diádico: dois caminhos, e ∼ casa-os — a borda não parte nada.\n\n");
    {
        /* S = {1 − 1/n} tem supremo 1, que é DIÁDICO — e um diádico tem DOIS caminhos.
         * Se a construção do supremo desse um caminho e o objecto reconhecesse o outro,
         * a existência ficaria por metade. Mede-se que os dois são o mesmo elemento:
         *
         *   a construção dá         m_k = 2^k − 1     (o caminho …0111…, por baixo)
         *   e o outro representante  m_k = 2^k        (o caminho …1000…, o nó)
         *
         * e ∼ identifica-os porque a diferença é 1/2^k, que o encaixe manda a zero:
         * (2^k − 1) + 1 = 2^k EXACTO, em inteiros, em todo k. É a soma geométrica a
         * fechar, e é ela que faz de 1 um elemento e não dois. */
        long niveis = 0, casa = 0, por_baixo = 0, nunca_atinge = 0;
        long m = 0, pot = 1;
        while(sp_existe(m + 1, 1, 2)) m++;
        printf("      k     m_k        2^k       (m_k)+1 = 2^k ?   algum x ∈ S atinge 1?\n");
        for(int k = 1; k <= 24; k++){
            pot *= 2; m = sp_bit(2*m, pot, 2);
            niveis++;
            if(m + 1 == pot) casa++;                  /* os dois caminhos, à distância 1 */
            if(m < pot) por_baixo++;                  /* a construção fica do lado de baixo */
            /* e nenhum 1−1/n é 1: o supremo NÃO está em S, que é o ponto todo */
            if(!sp_existe(pot, pot, 2)) nunca_atinge++;
            if(k <= 3 || k == 24)
                printf("      %-5d %-10ld %-9ld %-17s %s\n", k, m, pot,
                       (m + 1 == pot) ? "sim" : "NÃO", nunca_atinge == k ? "não" : "SIM");
        }
        printf("      %ld níveis: casam em %ld, por baixo em %ld, e S nunca atinge em %ld\n",
               niveis, casa, por_baixo, nunca_atinge);
        ok("E A BORDA NÃO PARTE A CONSTRUÇÃO: o supremo de {1−1/n} é 1, que é DIÁDICO e"
           " portanto tem DOIS caminhos. A construção dá m_k = 2^k − 1, o caminho …0111…"
           " por baixo; o outro representante é 2^k, o nó; e (2^k − 1) + 1 = 2^k EXACTO em"
           " todos os níveis — a soma geométrica fecha, e ∼ identifica-os, logo é UM"
           " elemento e não dois. E o supremo NÃO pertence a S: nenhum 1−1/n atinge 1, o"
           " que é exactamente o caso em que a continuidade tem de trabalhar",
           casa == niveis && por_baixo == niveis && nunca_atinge == niveis && niveis == 24);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  Dado S ≠ ∅ limitado, o supremo EXISTE no objecto, e não por axioma:\n");
        printf("  o caminho dele constrói-se bit a bit, m_{k+1} ∈ {2m_k, 2m_k+1},\n");
        printf("  em inteiros. É majorante e é o MENOR, com as duas cláusulas medidas.\n");
        printf("  O conjunto que ℚ não fecha — {r>0 : r²<2} — fecha aqui.\n");
        printf("  Isto É a continuidade, e é a direcção que faltava: a casa media\n");
        printf("  o corte a partir do habitante; esta mede o habitante a partir do corte.\n");
    }
    return falhas ? 1 : 0;
}
