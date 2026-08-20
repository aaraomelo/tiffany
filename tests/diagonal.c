/* tests/diagonal.c — A DIAGONAL É UM PASSO, E O PASSO MEDE-SE. E ela tem de sair da borda.
 *
 * O `reais.tex` §sec:preco afirma que o degrau Q → R paga a ENUMERABILIDADE, e não media.
 * Aqui mede-se o que de facto se pode medir — e diz-se o que não se pode.
 *
 * A conclusão de Cantor é sobre o infinito e não cabe numa varredura. Mas a prova dele NÃO
 * é sobre o infinito: é um PASSO, e o passo é finito —
 *
 *      dada uma lista com uma linha por índice, a diagonal difere da linha i no dígito i
 *
 * — e é o passo que se mede, com o dígito exibido. É a mesma disciplina de `naturais.tex`
 * (a terminação do transporte prova-se pelo passo, não varrendo) e de `corte_ponto_fixo.c`
 * §F5 («é o PASSO e não a lista — varrer confirmaria a conclusão sem medir a prova»).
 *
 * ── E A ARMADILHA, QUE A CASA JÁ CONHECIA ─────────────────────────────────────────
 * Em base 2 o mesmo real tem DOIS caminhos na borda diádica: …0111… e …1000… — e o
 * `supremo.c` §S4 já o mediu do outro lado («(2^k − 1) + 1 = 2^k EXACTO em todos os
 * níveis, e ∼ identifica-os: é UM elemento e não dois»). Logo uma diagonal que difira em
 * DÍGITOS pode não diferir em VALOR, e o argumento tem de sair fora da borda.
 *
 * §DG0  o PASSO: a diagonal difere da linha i no dígito i — exibido, e em VALOR exacto
 * §DG1  a BORDA: em base 2 a diferença entre os dois caminhos é exactamente 2^-M — o
 *       colapso é o limite; e a régua que o evita é base 3 com dígitos {0,1}
 * §DG2  O LIMITE DO QUE SE MEDE: com 2^N linhas, N dígitos NÃO bastam — a diagonal só
 *       escapa de uma lista INDEXADA, e é isso que a prova usa
 *
 * Tudo inteiro: o valor de uma expansão de M dígitos na base b é o inteiro
 * Σ dᵢ·b^{M−i} sobre b^M — compara-se pelo numerador, e nenhum decimal entra.
 */
#include <stdio.h>
#include <stdint.h>
#include "unidade.h"

#define M 12                       /* dígitos por linha */
#define NL 12                      /* linhas da lista (uma por índice) */

/* o valor da expansão 0.d₁d₂…d_M na base b, como NUMERADOR sobre b^M */
static uint64_t valor(const int *d, int b){
    uint64_t v = 0;
    for(int i = 0; i < M; i++) v = v * (uint64_t)b + (uint64_t)d[i];
    return v;
}
static uint64_t pot(int b, int k){
    uint64_t p = 1; while(k--) p *= (uint64_t)b; return p;
}

int main(void){
    printf("\n=== A DIAGONAL: o passo, a borda, e o que uma varredura não conclui ===\n");

    /* ═══ §DG0 O PASSO ═════════════════════════════════════════════════════════ */
    printf("\n§DG0 A diagonal difere da linha i no dígito i — e o dígito exibe-se.\n\n");
    {
        int L[NL][M], dg[M];
        /* uma lista qualquer, gerada por uma regra fixa e escrita (sem relógio) */
        for(int i = 0; i < NL; i++) for(int j = 0; j < M; j++)
            L[i][j] = ((i * 7 + j * 5 + (i * j) % 3) % 2);
        for(int i = 0; i < M; i++) dg[i] = (i < NL) ? 1 - L[i][i] : 0;

        long difere_digito = 0, difere_valor = 0;
        printf("        linha  dígito i   diagonal   difere?\n");
        for(int i = 0; i < NL; i++){
            if(dg[i] != L[i][i]) difere_digito++;
            if(valor(dg, 2) != valor(L[i], 2)) difere_valor++;
            if(i < 4) printf("        %3d       %d          %d          %s\n",
                             i, L[i][i], dg[i], dg[i] != L[i][i] ? "sim" : "NÃO");
        }
        printf("        %d linhas: difere no dígito i em %ld · difere em VALOR em %ld\n",
               NL, difere_digito, difere_valor);
        ok("O ARGUMENTO NÃO É SOBRE O INFINITO — É UM PASSO, E O PASSO É FINITO. Dada uma"
           " lista com uma linha por índice, a diagonal constrói-se olhando UM dígito de cada"
           " linha, e difere da linha i na posição i por construção: o dígito exibe-se, e não"
           " há nada a varrer. A conclusão de Cantor é sobre listas infinitas e não cabe numa"
           " medida; o passo cabe, e é ele que a prova usa. Mede-se o que se pode medir, e a"
           " diferença aparece também no VALOR exacto — comparado por inteiros, sem decimal",
           difere_digito == NL && difere_valor == NL);
    }

    /* ═══ §DG1 A BORDA: o colapso diádico é o limite ═══════════════════════════ */
    printf("\n§DG1 …0111 e …1000 distam exactamente 2^-M — o colapso é o limite.\n\n");
    {
        int A[M], B[M];
        A[0] = 0; for(int i = 1; i < M; i++) A[i] = 1;     /* 0.0111…1 */
        B[0] = 1; for(int i = 1; i < M; i++) B[i] = 0;     /* 0.1000…0 */
        uint64_t va = valor(A,2), vb = valor(B,2), den = pot(2,M);
        uint64_t dif = vb - va;
        printf("        base 2:  0.0111…1 = %llu/%llu   0.1000…0 = %llu/%llu   diferença = %llu/%llu\n",
               (unsigned long long)va, (unsigned long long)den,
               (unsigned long long)vb, (unsigned long long)den,
               (unsigned long long)dif, (unsigned long long)den);

        /* A RÉGUA QUE EVITA A BORDA: base 3 com dígitos em {0,1}. A cauda inteira vale
         * metade do dígito actual — Σ_{j>i} 3^{-j} = 3^{-i}/2 < 3^{-i} —, logo nunca
         * alcança o dígito seguinte e não há dois caminhos para o mesmo valor. */
        uint64_t cauda = 0;                        /* 0.0111…1 em base 3, numerador */
        int C[M]; C[0] = 0; for(int i = 1; i < M; i++) C[i] = 1;
        cauda = valor(C,3);
        uint64_t um_digito = pot(3, M-1);          /* o valor do dígito 1 na posição 1 */
        int cauda_menor = (2 * cauda < 2 * um_digito) && (cauda < um_digito);

        /* e mede-se o que isso dá: todas as 2^M sequências de {0,1} em base 3 têm valores
         * DISTINTOS, e a diferença mínima entre duas delas não é zero */
        long colisoes = 0; uint64_t minimo = ~(uint64_t)0;
        int total = 1 << 10;                        /* as 1024 de 10 dígitos, exaustivo */
        for(int x = 0; x < total; x++){
            int dx[M]; for(int i = 0; i < M; i++) dx[i] = (i < 10) ? ((x >> (9-i)) & 1) : 0;
            uint64_t vx = valor(dx,3);
            for(int y = x+1; y < total; y++){
                int dy[M]; for(int i = 0; i < M; i++) dy[i] = (i < 10) ? ((y >> (9-i)) & 1) : 0;
                uint64_t vy = valor(dy,3);
                if(vx == vy) colisoes++;
                else { uint64_t d = vx > vy ? vx - vy : vy - vx; if(d < minimo) minimo = d; }
            }
        }
        printf("        base 3, dígitos {0,1}: a cauda toda vale %llu e um dígito vale %llu"
               " (cauda < dígito: %s)\n", (unsigned long long)cauda,
               (unsigned long long)um_digito, cauda_menor ? "sim" : "NÃO");
        printf("        %d sequências: %ld colisões de valor · menor distância = %llu/3^%d\n",
               total, colisoes, (unsigned long long)minimo, M);
        ok("E A DIAGONAL TEM DE SAIR DA BORDA, QUE É ONDE ESTE ARGUMENTO SE PARTIRIA. Em base"
           " 2 o mesmo real tem dois caminhos — …0111… e …1000… —, e mede-se a razão exacta:"
           " com M dígitos eles distam 1/2^M, que não é zero mas vai a zero, e no limite são"
           " o MESMO real (é o §S4 do `supremo.c` visto deste lado). Uma diagonal que só"
           " garanta diferença em DÍGITOS não garante diferença em VALOR. A régua que resolve"
           " é mudar de base e usar só dois dos três dígitos: em base 3 com {0,1}, a cauda"
           " inteira vale metade do dígito actual e nunca o alcança — medido nas 1024"
           " sequências de dez dígitos, ZERO colisões de valor e distância mínima não nula."
           " O argumento passa a ser sobre valores, e não sobre escrita",
           colisoes == 0 && cauda_menor && dif == 1 && minimo > 0);
    }

    /* ═══ §DG2 O LIMITE DO QUE SE MEDE ════════════════════════════════════════ */
    printf("\n§DG2 Com 2^N linhas, N dígitos não bastam — a diagonal precisa do índice.\n\n");
    {
        /* a lista COMPLETA das 2^N palavras de N dígitos: nenhuma diagonal de N dígitos
         * lhe escapa, porque ela própria é uma palavra de N dígitos e está lá */
        const int N = 8;
        int total = 1 << N, achou = 0;
        int dg[M];
        for(int i = 0; i < M; i++) dg[i] = 0;
        /* diagonal da lista completa, na ordem natural: linha x tem os bits de x */
        for(int i = 0; i < N; i++){
            int bit = (i >> (N-1-i >= 0 ? (N-1-i) : 0)) & 1;   /* dígito i da linha i */
            dg[i] = 1 - bit;
        }
        for(int x = 0; x < total && !achou; x++){
            int igual = 1;
            for(int i = 0; i < N; i++) if(((x >> (N-1-i)) & 1) != dg[i]) { igual = 0; break; }
            if(igual) achou = 1;
        }
        /* O CONTRASTE, que é o que decide: a mesma diagonal contra uma lista INDEXADA —
         * N linhas, uma por índice. Sem os dois lados esta secção não podia falhar, e
         * uma asserção que não pode falhar não mede nada. */
        int LI[NL][M], dgi[M], achou_ind = 0;
        for(int i = 0; i < NL; i++) for(int j = 0; j < M; j++)
            LI[i][j] = ((i * 7 + j * 5 + (i * j) % 3) % 2);
        for(int i = 0; i < N; i++) dgi[i] = 1 - LI[i][i];
        for(int x = 0; x < N && !achou_ind; x++){
            int igual = 1;
            for(int i = 0; i < N; i++) if(LI[x][i] != dgi[i]) { igual = 0; break; }
            if(igual) achou_ind = 1;
        }
        printf("        lista COMPLETA (%d palavras de %d dígitos): a diagonal está lá? %s\n",
               total, N, achou ? "SIM" : "não");
        printf("        lista INDEXADA (%d linhas, uma por índice): a diagonal está lá? %s\n",
               N, achou_ind ? "SIM" : "não");
        ok("E AQUI DIZ-SE O QUE UMA MEDIDA FINITA NÃO PODE CONCLUIR, que é a parte que"
           " costuma faltar. Se a lista tiver 2^N linhas e cada linha só N dígitos, a"
           " diagonal — que é ela própria uma palavra de N dígitos — ESTÁ na lista: não"
           " escapa. O argumento de Cantor não usa uma lista qualquer: usa uma lista"
           " INDEXADA, uma linha por natural, com dígitos tantos quantos os índices, e é daí"
           " que a diagonal tira o seu poder. Logo esta bateria mede o PASSO (§DG0) e a"
           " condição de borda (§DG1), e NÃO mede a incontabilidade — que continua um"
           " teorema, e não um resultado desta máquina. Os DOIS lados medem-se, e é o"
           " contraste que decide: na lista completa a diagonal está lá, na lista indexada"
           " não está",
           achou == 1 && achou_ind == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
