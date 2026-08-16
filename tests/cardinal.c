/* cardinal.c — O QUE ESTÁ CONSTRUÍDO É O CONTÍNUO, E NÃO OS RACIONAIS.
 *
 * O Corpo Universal (thm:central-continuo) constrói
 *
 *      ℝ ≅ {caminhos}/∼,     com ∼ a identificar os nós da árvore,
 *
 * e mede lá as quatro peças: existência e unicidade do limite pelo pombal,
 * sobrejectividade na classe comparável, unicidade módulo a borda, e a completude —
 * o Cauchy dos truncados estabiliza bit a bit DENTRO do objecto.
 *
 * FALTAVA A PEÇA QUE SEPARA ESSE OBJECTO DE ℚ, e é ela que este medidor põe: a
 * DIAGONAL. Porque tudo o que se mede aqui corre em inteiros, e daí a pergunta legítima
 * — «então são os inteiros?». Não são, e a prova é construtiva:
 *
 *      ℚ ENUMERA-SE          — exibe-se a lista, e todo racional aparece nela
 *      OS CAMINHOS NÃO       — dada QUALQUER lista, constrói-se o caminho que falta
 *
 * A segunda é a diagonal de Cantor, e não é um argumento sobre infinito: é uma
 * CONSTRUÇÃO, e o que se mede é ela a funcionar em toda a lista dada. Nenhuma varredura
 * decide um ∀ — mas exibir o que falta a uma lista é finito, e decide.
 *
 * E o habitante que fecha o caso: √2−1 tem caminho, decidido por teste de chão EXACTO em
 * inteiros — (m+2^k)² contra 2·2^{2k} —, e NENHUM racional está no corte dele. Logo o
 * objecto tem habitantes que ℚ não tem, e a sua construção não é a de ℚ.
 *
 *   §C1  ℚ ENUMERA-SE: a lista existe, e todo racional aparece — com o índice exibido
 *   §C2  A DIAGONAL: o que falta constrói-se — e FORA DA BORDA, senão há buraco
 *   §C3  √2−1 TEM CAMINHO, por teste de chão inteiro — e é irracional, medido
 *   §C4  E O CORTE DELE NÃO TEM RACIONAL NENHUM: o encaixe aperta e nada racional fica
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib cardinal.c -o cardinal && ./cardinal
 */
#include <stdio.h>
#include "unidade.h"

#define CD_PROF 40

int main(void){
    printf("\n=== O QUE ESTÁ CONSTRUÍDO É O CONTÍNUO — e a diagonal é a prova ===\n");

    /* ═══ §C1  ℚ ENUMERA-SE ══════════════════════════════════════════════════ */
    printf("\n§C1 ℚ enumera-se: a lista existe, e todo racional aparece nela.\n\n");
    {
        /* A enumeração por diagonais de Cantor sobre (p,q) com q ≥ 1: percorre-se
         * p+q = 2, 3, 4, … e dentro de cada soma varre-se p. Todo par aparece, e num
         * índice FINITO que se exibe. É isto que ℚ tem e o contínuo não vai ter. */
        long alvos = 0, achados = 0, pior_indice = 0;
        for(long q = 1; q <= 12; q++) for(long p = 0; p <= 12; p++){
            /* procura-se (p,q) na enumeração, e conta-se em que passo aparece */
            long idx = 0, achou = -1;
            for(long soma = 1; soma <= 40 && achou < 0; soma++)
                for(long a = 0; a <= soma && achou < 0; a++){
                    long b = soma - a;
                    if(b < 1) continue;
                    idx++;
                    if(a == p && b == q) achou = idx;
                }
            alvos++;
            if(achou > 0){ achados++; if(achou > pior_indice) pior_indice = achou; }
        }
        printf("      %ld pares (p,q) com q ≥ 1: %ld encontrados na lista, e o mais tardio"
               " aparece no índice %ld — FINITO\n", alvos, achados, pior_indice);
        ok("ℚ ENUMERA-SE, E O ÍNDICE EXIBE-SE: a lista por diagonais de Cantor sobre (p,q)"
           " apanha todos os pares medidos, cada um num índice FINITO que se mostra. É"
           " esta propriedade que o contínuo NÃO vai ter, e é por ela que os dois se"
           " separam",
           achados == alvos && alvos == 156 && pior_indice > 0);
    }

    /* ═══ §C2  A DIAGONAL — e ela tem de sair FORA DA BORDA ══════════════════ */
    printf("\n§C2 A diagonal: dada QUALQUER lista de caminhos, o que falta constrói-se —\n");
    printf("    e constrói-se FORA da borda, senão o argumento tem buraco.\n\n");
    {
        /* Um caminho é uma sucessão de bits. A diagonal ingénua
         *
         *      d[k] = 1 − lista[k][k]
         *
         * difere do k-ésimo na posição k. MAS ISSO NÃO CHEGA, e o buraco é a BORDA:
         * o objecto é {caminhos}/∼, e ∼ identifica os nós da árvore — um DIÁDICO tem
         * DOIS caminhos (…0111… e …1000…). Se a diagonal sair eventualmente constante,
         * ela é um diádico, e pode estar na lista pela OUTRA representação. A diferença
         * numa posição não é, por si, diferença no objecto.
         *
         * A construção que fecha usa as duas paridades:
         *
         *      d[2k]   = 1 − lista[k][2k]     ← difere do k-ésimo na posição 2k
         *      d[2k+1] = k mod 2              ← alterna, logo NUNCA é constante no fim
         *
         * As ímpares dão infinitos zeros e infinitos uns, logo d NÃO é diádico, logo a
         * classe de d é um SINGULAR: diferir num bit é diferir no objecto. */
        long listas = 0, difere = 0, fora_borda = 0, ingenua_na_borda = 0;
        long est = 5;
        printf("      L     difere do k-ésimo em 2k?   ambos os bits nas ímpares?"
               "   a ingénua caía na borda?\n");
        for(int L = 2; L <= 30; L++){
            int lista[32][80], d[80], W = 2*L + 2;
            for(int i = 0; i < L; i++)
                for(int k = 0; k < W; k++){
                    est = (est*1103515245L + 12345L) % 2147483647L;
                    lista[i][k] = (int)((est >> 9) & 1);
                }
            for(int k = 0; k < L; k++){
                d[2*k]   = 1 - lista[k][2*k];
                d[2*k+1] = k & 1;
            }
            /* (a) difere do k-ésimo NA POSIÇÃO 2k, para todo k */
            int dif = 1;
            for(int k = 0; k < L; k++) if(d[2*k] == lista[k][2*k]) dif = 0;
            /* (b) e as ímpares trazem os DOIS bits, logo d não é eventualmente constante,
             *     logo não é diádico, logo a sua classe módulo ∼ é um singular */
            int tem0 = 0, tem1 = 0;
            for(int k = 0; k < L; k++){ if(d[2*k+1] == 0) tem0 = 1; else tem1 = 1; }
            /* (c) e o que a INGÉNUA arriscava: sair eventualmente constante na cauda */
            int ing[32], c0 = 0, c1 = 0;
            for(int k = 0; k < L; k++) ing[k] = 1 - lista[k][k];
            for(int k = L/2; k < L; k++){ if(ing[k]) c1++; else c0++; }
            listas++;
            if(dif) difere++;
            if(tem0 && tem1) fora_borda++;
            if(c0 == 0 || c1 == 0) ingenua_na_borda++;   /* cauda constante: diádico */
            if(L <= 4 || L == 30)
                printf("      %-5d %-26s %-31s %s\n", L, dif ? "sim" : "NÃO",
                       (tem0 && tem1) ? "sim (não é diádico)" : "NÃO",
                       (c0 == 0 || c1 == 0) ? "SIM" : "não");
        }
        printf("      %ld listas: difere em %ld, fora da borda em %ld · e a diagonal"
               " INGÉNUA caía na borda em %ld\n", listas, difere, fora_borda, ingenua_na_borda);
        ok("A DIAGONAL CONSTRÓI O QUE FALTA, E CONSTRÓI-O FORA DA BORDA: pôr apenas"
           " d[k] = 1 − lista[k][k] não fecha, porque o objecto é {caminhos}/∼ e um"
           " DIÁDICO tem DOIS caminhos — a diagonal ingénua pode sair eventualmente"
           " constante e estar na lista pela outra representação. A construção que fecha"
           " gasta as duas paridades: d[2k] = 1 − lista[k][2k] difere do k-ésimo NA"
           " POSIÇÃO 2k, e d[2k+1] alterna, o que dá infinitos zeros e infinitos uns —"
           " logo d NÃO é diádico, logo a classe dele é um SINGULAR e diferir num bit É"
           " diferir no objecto. Verificado nas 29 listas, e a construção é FINITA: não"
           " afirma nada sobre o infinito, exibe o que falta a uma lista DADA",
           difere == listas && fora_borda == listas && listas == 29);
    }

    /* ═══ §C3  √2−1 TEM CAMINHO, POR TESTE DE CHÃO INTEIRO ═══════════════════ */
    printf("\n§C3 √2−1 tem caminho — e o teste de chão é uma comparação de inteiros.\n\n");
    {
        /* O caminho de x sai do teste m/2^k < x. Para x = √2−1:
         *
         *      m/2^k < √2 − 1  ⟺  m + 2^k < 2^k·√2  ⟺  (m + 2^k)² < 2·2^{2k}
         *
         * inteiro puro, e decide sem avaliar raiz nenhuma. Mede-se que o caminho existe
         * em toda a profundidade, que o encaixe aperta, e que x é IRRACIONAL — porque
         * (m+2^k)² = 2·2^{2k} não tem solução: 2 não é quadrado perfeito. */
        long prof = 0, decide = 0, aperta = 0, empate = 0;
        long m = 0, pot = 1, ant_den = 1;
        printf("      k     m/2^k          (m+2^k)² < 2·2^{2k} ?   empate?\n");
        for(int k = 1; k <= 28; k++){
            m *= 2; pot *= 2;
            long a = m + 1 + pot;
            /* cabe? (m+1+2^k) ≤ 2·2^k, logo a² ≤ 4·2^{2k}: com k ≤ 28 cabe em long */
            if(a*a < 2*pot*pot) m += 1;
            prof++;
            /* o teste DECIDE: nunca há empate, porque 2 não é quadrado */
            long b = m + pot;
            if(b*b != 2*pot*pot) decide++; else empate++;
            /* e o encaixe aperta: o denominador dobra */
            if(pot > ant_den) aperta++;
            ant_den = pot;
            if(k <= 4 || k == 28)
                printf("      %-5d %-14s %-23s %s\n", k, "…",
                       (b*b < 2*pot*pot) ? "sim" : "não", (b*b == 2*pot*pot) ? "SIM" : "nunca");
        }
        printf("      %ld profundidades: decidiu em %ld, apertou em %ld, empates: %ld\n",
               prof, decide, aperta, empate);
        printf("      e o caminho até 28: m = %ld sobre 2^28 = %ld\n", m, pot);
        ok("√2−1 TEM CAMINHO, E O TESTE DE CHÃO É UMA COMPARAÇÃO DE INTEIROS:"
           " m/2^k < √2−1 ⟺ (m+2^k)² < 2·2^{2k}, decidido sem avaliar raiz nenhuma nas 28"
           " profundidades. E NUNCA HÁ EMPATE — (m+2^k)² = 2·2^{2k} não tem solução porque"
           " 2 não é quadrado perfeito —, o que é dizer que o habitante é IRRACIONAL: o"
           " caminho existe, e nenhum diádico está nele",
           decide == prof && empate == 0 && aperta == prof && prof == 28);
    }

    /* ═══ §C4  E O CORTE NÃO TEM RACIONAL NENHUM ═════════════════════════════ */
    printf("\n§C4 O corte de √2−1 não tem racional nenhum — e isso é o que o separa de ℚ.\n\n");
    {
        /* Se algum racional p/q estivesse NO corte, teria (p+q)² = 2q². Varre-se, e não
         * há — mas a varredura é só o controlo: a razão é que 2 não é quadrado, e essa
         * prova-se pelo PASSO, não pela lista. Mede-se o passo: se (p+q)² = 2q² com q
         * mínimo, então (2q−p−q)² = 2(p+q−q)² com denominador MENOR — descida infinita,
         * impossível em ℕ. Aqui exibe-se a descida a partir de qualquer candidato. */
        long cands = 0, no_corte = 0;
        for(long q = 1; q <= 200; q++) for(long p = 0; p <= 2*q; p++){
            long e = p + q;
            if(e*e != 2*q*q) continue;                 /* não está no corte */
            cands++; no_corte++;
        }
        /* E O PASSO DA DESCIDA, medido como IDENTIDADE — que é o que PODE falhar.
         * Contar zero soluções não mede nada: mede-se a implicação que as proibiria.
         * A descida (a,b) ↦ (2b−a, a−b) cumpre, para TODO par:
         *
         *      (2b−a)² − 2(a−b)² = −(a² − 2b²)
         *
         * logo se a² − 2b² = 0 o derivado também é zero; e no intervalo b < a < 2b
         * tem-se 0 < a−b < b, ou seja um denominador ESTRITAMENTE MENOR. Duas coisas
         * juntas dão a descida infinita, impossível em ℕ. A identidade mede-se em todo
         * par, e o encolhimento no intervalo onde a raiz vive. */
        long pares = 0, ident = 0, enc_cas = 0, encolhe = 0;
        for(long b = 1; b <= 120; b++) for(long a = 0; a <= 3*b; a++){
            long esq = (2*b-a)*(2*b-a) - 2*(a-b)*(a-b);
            long dir = -(a*a - 2*b*b);
            pares++;
            if(esq == dir) ident++;
            if(b < a && a < 2*b){                      /* onde √2·b vive */
                enc_cas++;
                if(0 < a-b && a-b < b) encolhe++;
            }
        }
        printf("      %ld racionais p/q com q ≤ 200 no corte de √2−1: %ld\n", cands, no_corte);
        printf("      e o PASSO, medido como identidade: (2b−a)² − 2(a−b)² = −(a²−2b²) em"
               " %ld de %ld pares · e encolhe (0 < a−b < b) em %ld de %ld no intervalo\n",
               ident, pares, encolhe, enc_cas);
        ok("O CORTE DE √2−1 NÃO TEM RACIONAL NENHUM, E A RAZÃO É O PASSO E NÃO A"
           " VARREDURA: nenhum p/q com q ≤ 200 lá está — mas isso é o controlo. A razão é"
           " que (p+q)² = 2q² obrigaria a uma DESCIDA INFINITA em ℕ: a identidade"
           " (2b−a)² − 2(a−b)² = −(a²−2b²) vale em todos os pares medidos, logo leva"
           " solução em solução; e no intervalo b < a < 2b o denominador ENCOLHE"
           " estritamente. As duas juntas proíbem a solução. Logo o objecto tem habitantes que ℚ não"
           " tem, e a construção não é a de ℚ",
           no_corte == 0 && ident == pares && encolhe == enc_cas && enc_cas > 100);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  ℚ enumera-se; os caminhos não — e a diagonal CONSTRÓI o que falta.\n");
        printf("  √2−1 tem caminho, por teste de chão inteiro, e no corte dele\n");
        printf("  não há racional nenhum. O objecto é o CONTÍNUO, e não ℚ.\n");
        printf("  E tudo isto correu em inteiros: a aritmética é do CAMINHO,\n");
        printf("  e o habitante é o caminho — não um limite à espera de existir.\n");
    }
    return falhas ? 1 : 0;
}
