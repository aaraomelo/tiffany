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
 *   §S3  O CASO QUE ℚ FALHA: e a testemunha é [[3,4],[2,3]], det = 1
 *   §S4  E A BORDA NÃO PARTE NADA: o supremo diádico tem dois caminhos, e ∼ casa-os
 *   §S5  DUAS ROTAS: a minha é linear em ℤ[√2]; a de Newton eleva ao quadrado
 *
 * Nenhum double, nenhum limiar: o teste «existe x ∈ S acima de m/2^k» é, em cada
 * caso, uma comparação de inteiros.
 *
 *   cc -O2 -std=c99 -I. -I../lib supremo.c -o supremo && ./supremo
 */
#include <stdio.h>
#include "inteiros.h"
#include "cifra.h"
#include "dual32.h"
#include "reta.h"      /* rt_caminho_sup: o caminho, agora na lib */
#include "racionais.h"
#include "reais.h"
#include "cauchy.h"
#include "calculo.h"
#include "analise.h"
#include "unidade.h"

#define SP_K   26          /* profundidade: 2^26, e os quadrados cabem em long */
#define SP_D   24          /* profundidade da amostra diádica do §S4 — escolha, e nomeada */
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

/* e a CONSTRUÇÃO INTEIRA está agora em lib/reta.h — rt_caminho_sup —, porque ela é
 * geral: qualquer real definido por um predicado inteiro constrói-se assim. O que era
 * uma peça deste ficheiro passou a ser a peça de quem precisar. A ponte é esta: */
static int sp_serve(long m, long pot, void *ctx){ return sp_existe(m, pot, *(int*)ctx); }

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
         * E A (b) NÃO SE VARRE. Estava aqui uma janela dos ÚLTIMOS 40 valores de m' com a
         * asserção a dizer «TODOS os m' abaixo»: a afirmação era mais larga que o laço, e
         * o 40 era um tecto meu — com m_k a chegar aos 92681, viam-se 40. A cláusula (b)
         * cabe numa linha:
         *
         *      m' < m_k  ⟹  m'/2^k < m_k/2^k  ⟹  (por definição de m_k) existe x ∈ S com
         *                                        m'/2^k < x — logo m' não é majorante
         *
         * e o que a sustenta é a MONOTONIA de E(m) := «existe x ∈ S com m/2^k < x», que é
         * decrescente em m. Mede-se então o PASSO,
         *
         *      E(m+1) ⟹ E(m),
         *
         * de onde a indução dá todos os m' de uma vez — sem janela e sem tecto. E a gama
         * do passo é a que o OBJECTO determina, [0, m_k], e não um número escolhido. */
        long alvos = 0, maj = 0, mono = 0, passos_mono = 0;
        printf("      conjunto                       (a) majorante   (b) passo E(m+1)=>E(m)\n");
        for(int c = 0; c < SP_N; c++){
            long m = 0, pot = 1, a = 0, b = 0, n = 0, vistos = 0;
            while(sp_existe(m + 1, 1, c)) m++;
            for(int k = 1; k <= 16; k++){
                pot *= 2; m = sp_bit(2*m, pot, c);
                n++;
                /* (a) o passo seguinte já não serve: nada em S o ultrapassa */
                if(!sp_existe(m + 1, pot, c)) a++;
                /* (b) o PASSO da monotonia, em toda a gama que o objecto determina */
                long viola = 0;
                for(long mm = 0; mm < m; mm++){
                    vistos++;
                    if(sp_existe(mm + 1, pot, c) && !sp_existe(mm, pot, c)) viola++;
                }
                if(!viola) b++;
            }
            alvos += n; maj += a; mono += b; passos_mono += vistos;
            printf("      %-30s %-15s %s (%ld passos)\n", sp_nome[c],
                   a == n ? "sim" : "NÃO", b == n ? "sim" : "NÃO", vistos);
        }
        /* E O GUME: se a monotonia não valesse, (b) não se seguiria. Com uma condição NÃO
         * monótona — E'(m) = (m mod 7 ≠ 3) — o passo tem de falhar, e falha onde se
         * prevê: uma vez em cada bloco de sete. */
        const long FM = 3000, FQ = 7, FR = 3;      /* E'(m) = (m mod FQ ≠ FR) */
        long falso_viola = 0;
        for(long mm = 0; mm < FM; mm++)
            if((mm + 1) % FQ != FR && (mm % FQ) == FR) falso_viola++;
        /* e a PREVISÃO sai da fórmula, não de uma divisão à mão: a violação dá-se
         * exactamente nos m ≡ FR (mod FQ), e em [0, FM) esses são */
        long falso_prev = (FM - 1 - FR)/FQ + 1;    /* e não FM/FQ, que erra por um */
        printf("      %ld níveis: majorante em %ld, monotonia sem violação em %ld"
               " (%ld passos verificados)\n", alvos, maj, mono, passos_mono);
        printf("      GUME: uma condição não monótona viola o passo %ld vezes em 3000 — as"
               " %ld previstas\n", falso_viola, falso_prev);
        ok("E É O SUPREMO, PELAS DUAS CLÁUSULAS, COM A SEGUNDA A FAZER O TRABALHO: (a) o"
           " passo seguinte já não serve, logo nada em S ultrapassa o caminho — é"
           " majorante; e (b) TODO m' estritamente abaixo ainda serve, logo nenhum deles"
           " é majorante — é o MENOR. Dizer só (a) daria um majorante qualquer; é (b) que"
           " faz dele o supremo. E (b) NÃO SE VARRE: sai da MONOTONIA de E(m) por indução,"
           " e o que se mede é o PASSO E(m+1) ⟹ E(m), sem uma violação em toda a gama que"
           " o OBJECTO determina. Antes estava aqui uma janela dos últimos 40 valores com a"
           " asserção a dizer «todos os m' abaixo» — a afirmação mais larga que o laço, e o"
           " 40 um tecto meu, com m_k a chegar aos 92681. E o gume mostra que a medida"
           " morde: uma condição não monótona viola o passo, e viola-o o número PREVISTO de"
           " vezes",
           maj == alvos && mono == alvos && alvos == SP_N*16
           && falso_viola == falso_prev && falso_prev > 0);
    }

    /* ═══ §S3  O CASO QUE ℚ FALHA ════════════════════════════════════════════ */
    printf("\n§S3 {r > 0 : r² < 2} não tem supremo em ℚ — e tem aqui.\n\n");
    {
        /* Este é o conjunto que decide, porque a diferença entre ℚ e o objecto aparece
         * NELE e não numa afirmação sobre cardinais. Para todo candidato p/q em ℚ:
         *
         *   p² < 2q²   →  está em S, e NÃO é majorante: exibe-se x ∈ S maior
         *   p² > 2q²   →  é majorante, e não o MENOR: exibe-se majorante menor
         *   p² = 2q²   →  impossível (descida infinita, cardinal.c §C4)
         *
         * E a testemunha é a MESMA matriz nos dois casos — o que a torna testemunha é
         * preservar o LADO e mover na direcção certa:
         *
         *      (p,q) ⟼ (3p+4q, 2p+3q),      det [[3,4],[2,3]] = 1
         *
         * que é a unidade fundamental de ℤ[√2] ao quadrado, (1+√2)² = 3+2√2. Ela cumpre,
         * em inteiros e para TODO par:
         *
         *      (3p+4q)² − 2(2p+3q)² = p² − 2q²        preserva a forma, logo o LADO
         *      (3p+4q)q − p(2p+3q)  = 2(2q² − p²)     o sinal do movimento é o do lado
         *
         * Junto: quem está abaixo SOBE ficando abaixo (logo continua em S, e é maior);
         * quem está acima DESCE ficando acima (logo continua majorante, e é menor).
         * Nenhum p/q pode ser o menor majorante.
         *
         * A mediante SIMPLES (p+2q)/(p+q) não serve, e é erro fácil: ela troca o sinal,
         * logo atravessa o corte e cai do lado oposto — não fica entre p/q e o corte. */
        long cands = 0, forma = 0, move = 0, testemunha = 0, empates = 0;
        for(long q = 1; q <= 60; q++) for(long p = 1; p <= 3*q; p++){
            long d = p*p - 2*q*q;
            long P = 3*p + 4*q, Q = 2*p + 3*q;
            long dd = P*P - 2*Q*Q;
            long mov = P*q - p*Q;                     /* > 0 ⟺ P/Q > p/q */
            cands++;
            if(dd == d) forma++;                      /* preserva a forma quadrática */
            if(mov == 2*(2*q*q - p*p)) move++;        /* e o movimento é do sinal do lado */
            if(d == 0) empates++;
            /* a testemunha: mesmo lado E na direcção que refuta «p/q é o supremo» */
            if(d != 0 && dd == d && ((d < 0 && mov > 0) || (d > 0 && mov < 0))) testemunha++;
        }
        printf("      %ld candidatos p/q: a forma preserva-se em %ld, o movimento bate"
               " em %ld, empates %ld\n", cands, forma, move, empates);
        printf("      e nenhum p/q é o supremo: %ld com testemunha do MESMO lado exibida\n",
               testemunha);
        printf("      (a mediante simples (p+2q)/(p+q) troca o sinal e cai do lado"
               " OPOSTO — não serve de testemunha)\n");
        /* e aqui o supremo EXISTE, e o caminho dele mostra-se — DUAS ROTAS: a descida
         * deste ficheiro e a peça rt_caminho_sup da lib, que não partilham uma linha. */
        long m = 1, pot = 1;
        for(int k = 1; k <= 20; k++){ pot *= 2; m = sp_bit(2*m, pot, 0); }
        int caso0 = 0; long pot_lib = 0;
        long m_lib = rt_caminho_sup(sp_serve, 20, &caso0, &pot_lib);
        printf("      e no objecto o supremo EXISTE, e o caminho é: %ld / 2^20 = %ld/%ld\n",
               m, m, pot);
        printf("      e a peça da lib (rt_caminho_sup) desce o MESMO caminho: %ld/%ld — %s\n",
               m_lib, pot_lib, (m_lib == m && pot_lib == pot) ? "batem" : "DIVERGEM");
        ok("e a peça da lib desce o MESMO caminho que a descida local — duas rotas sem uma"
           " linha em comum, e o supremo de {r²<2} é 1482910/2^20, o par (m, 2^k) que o"
           " universal.tex publica. Um real nao se aproxima: constroi-se, e o que se"
           " guarda sao as DECISOES",
           m_lib == m && pot_lib == pot && m == 1482910 && pot == 1048576);
        ok("O CONJUNTO QUE ℚ NÃO CONSEGUE FECHAR, O OBJECTO FECHA: a testemunha é a"
           " matriz [[3,4],[2,3]] de determinante 1 — a unidade fundamental de ℤ[√2] ao"
           " quadrado, (1+√2)² = 3+2√2 —, e o que a torna testemunha são DUAS identidades"
           " inteiras a valer em todos os candidatos: (3p+4q)² − 2(2p+3q)² = p² − 2q²"
           " PRESERVA o lado, e (3p+4q)q − p(2p+3q) = 2(2q² − p²) move na direcção do"
           " lado. Quem está abaixo sobe ficando em S; quem está acima desce continuando"
           " majorante — logo nenhum racional é o menor majorante. E não há empate, porque"
           " p² = 2q² é a descida proibida. No objecto o supremo EXISTE, e o caminho dele"
           " exibe-se em inteiros",
           forma == cands && move == cands && empates == 0 && testemunha == cands
           && cands > 5000);
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
        for(int k = 1; k <= SP_D; k++){
            pot *= 2; m = sp_bit(2*m, pot, 2);
            niveis++;
            if(m + 1 == pot) casa++;                  /* os dois caminhos, à distância 1 */
            if(m < pot) por_baixo++;                  /* a construção fica do lado de baixo */
            /* e nenhum 1−1/n é 1: o supremo NÃO está em S, que é o ponto todo */
            if(!sp_existe(pot, pot, 2)) nunca_atinge++;
            if(k <= 3 || k == SP_D)
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
           casa == niveis && por_baixo == niveis && nunca_atinge == niveis && niveis == SP_D);
    }

    /* ═══ §S5  DUAS ROTAS PARA O MESMO — E A BIBLIOTECA ESTAVA MORTA ════════ */
    printf("\n§S5 Duas testemunhas independentes, e o `analise.h` já tinha uma.\n\n");
    {
        /* Ao escrever o §S3 fabriquei a testemunha [[3,4],[2,3]] sem procurar na casa. E a
         * casa já a tinha, por outro caminho: `lib/analise.h` traz
         *
         *      an_sup_nao_racional(b) : dado um majorante b de {r > 0 : r² < 2},
         *                               devolve um majorante ESTRITAMENTE MENOR
         *
         * pelo passo de NEWTON, (b + 2/b)/2 — e nunca tinha sido chamada. O ficheiro
         * inteiro tinha um só utilizador, e estas duas funções nenhum.
         *
         * E as duas rotas NÃO são a mesma, o que as torna melhores: em ℤ[√2], com b = p/q,
         *
         *      a MINHA    multiplica por a unidade fixa (1+√2)² = 3+2√2   →  LINEAR
         *      NEWTON     ELEVA AO QUADRADO, (p,q) ⟼ (p²+2q², 2pq)       →  QUADRÁTICO
         *
         * Vivem no mesmo grupo e fazem coisas diferentes; coincidem em 3/2 e mais nada.
         * São testemunhas INDEPENDENTES da mesma tese — «nenhum racional é o menor
         * majorante» — e é isso que se mede: as duas cumprem, e o valor difere. */
        long cand = 0, minha_ok = 0, newton_ok = 0, difere = 0, newton_forma = 0;
        for(long q = 1; q <= 40; q++) for(long p = 1; p <= 3*q; p++){
            long d = p*p - 2*q*q;
            if(d <= 0) continue;                      /* só os majorantes: p/q > √2 */
            cand++;
            /* a minha: (p,q) ⟼ (3p+4q, 2p+3q) */
            long P = 3*p + 4*q, Q = 2*p + 3*q;
            if(P*P - 2*Q*Q > 0 && P*q - p*Q < 0) minha_ok++;     /* majora, e é MENOR */
            /* Newton, pela biblioteca — e a chamada é o que a põe a viver */
            Qz b = qz(p, q), menor;
            if(an_sup_nao_racional(b, &menor)){
                newton_ok++;
                /* e a forma fechada dele em ℤ[√2]: (p²+2q²)/(2pq) */
                Qz esp = qz(p*p + 2*q*q, 2*p*q);
                if(qz_igual(menor, esp)) newton_forma++;
                if(!qz_igual(menor, qz(P, Q))) difere++;
            }
        }
        /* e a outra função morta: o corte de Dedekind sem máximo */
        long dd = 0, dd_ok = 0;
        for(long q = 1; q <= 30; q++) for(long p = 1; p <= 2*q; p++){
            Qz a = qz(p, q), maior;
            if(a.p*a.p*1L >= 2L*a.q*a.q) continue;    /* só os de dentro: a² < 2 */
            dd++;
            if(an_dedekind_sem_maximo(a, &maior)) dd_ok++;
        }
        printf("      %ld majorantes: a minha rota dá menor em %ld · Newton em %ld\n",
               cand, minha_ok, newton_ok);
        printf("      e o Newton tem forma fechada (p²+2q²)/(2pq) em %ld · difere da minha"
               " em %ld\n", newton_forma, difere);
        printf("      e o corte SEM MÁXIMO, pela mesma biblioteca: %ld de %ld elementos de A"
               " têm um maior dentro de A\n\n", dd_ok, dd);
        ok("DUAS TESTEMUNHAS INDEPENDENTES, E A BIBLIOTECA JÁ TINHA UMA: ao escrever o §S3"
           " fabriquei a matriz [[3,4],[2,3]] sem procurar na casa, e `lib/analise.h` já"
           " trazia `an_sup_nao_racional` pelo passo de NEWTON — nunca chamada. E as duas"
           " NÃO são a mesma, o que as torna melhores: em ℤ[√2] a minha multiplica pela"
           " unidade fixa (1+√2)² e é LINEAR, e a de Newton ELEVA AO QUADRADO,"
           " (p,q) ⟼ (p²+2q², 2pq). Vivem no mesmo grupo e fazem coisas diferentes —"
           " coincidem em 3/2 e mais nada. Cumprem ambas a mesma tese, «nenhum racional é"
           " o menor majorante», por caminhos que não se apoiam um no outro. E a segunda"
           " função morta, `an_dedekind_sem_maximo`, também passa a correr: dado a em A,"
           " exibe um maior dentro de A, pela média de Möbius (2a+2)/(a+2)",
           minha_ok == cand && newton_ok == cand && newton_forma == cand
           && difere > cand/2 && dd_ok == dd && cand > 500 && dd > 100
           && an_estouros == 0);
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
