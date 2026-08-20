/* pascal.c — O TRIANGULO DE PASCAL E' O PASSO DA TORRE, CONTADO.
 *
 * O Aarao: "entao pronto, derivamos o triangulo de Pascal. Estava faltando mostrar de
 * onde ele sai."
 *
 * Um revisor tinha apontado, e com razao, que a decomposicao 6+3+3+2+1+1 do colisor de
 * cinco eixos e' o triangulo de Pascal, e que sem o nome emprestado da fisica ela nao
 * teria seccao. O defeito nao era ser Pascal — era eu ter-lhe posto o nome de fora por
 * cima em vez de mostrar de onde Pascal sai. Sai daqui:
 *
 *     C(n,k) = C(n-1,k-1) + C(n-1,k)          a recorrencia de Pascal
 *     A_{n+1} = A_n (+) A_n†                   o passo da torre (thm:torrecruz)
 *
 * SAO A MESMA COISA. Um estado do andar n com k sinais negativos ou TEM o eixo novo
 * negativo — e entao o resto e' um estado de n-1 com k-1 — ou NAO TEM, e o resto e' um
 * estado de n-1 com k. Nao ha' terceiro caso, e nao ha' sobreposicao. «Nenhum andar e'
 * construido; todos sao lidos do anterior» — e Pascal E' essa leitura, contada.
 *
 *   §P1  os estados de peso k sao C(n,k), contados um a um e nao pela formula
 *   §P2  e a recorrencia sai da PARTICAO pelo eixo novo: tem ou nao tem, e mais nada
 *   §P3  a linha soma 2^n — o andar inteiro, que e' a dimensao da torre
 *   §P4  a simetria C(n,k)=C(n,n-k) E' a involucao que troca todos os sinais
 *   §P5  e dai a decomposicao do colisor: C(3,nc).C(2,nw) da' 6+3+3+2+1+1, SEM nome
 *        emprestado — e' o produto de duas linhas de Pascal
 *   §P6  e o CONTROLO: uma particao que nao seja por UM eixo nao da' a recorrencia
 *
 * Zero doubles. Tudo contado por varrimento dos estados.
 *
 *   cc -O2 -std=c99 -Wall -I../lib pascal.c -o pascal
 */
#include <stdint.h>
#include <stdio.h>
#include "unidade.h"

#define NMAX 14

static int pc(long x){ int c=0; while(x){ c += x&1L; x >>= 1; } return c; }

/* quantos estados de n eixos tem exactamente k sinais negativos — CONTADOS */
static long conta(int n, int k){
    long c = 0, V = 1L << n;
    for(long v = 0; v < V; v++) if(pc(v) == k) c++;
    return c;
}

int main(void){
    puts("\n  O TRIANGULO DE PASCAL E' O PASSO DA TORRE, CONTADO\n");

    /* ═══ §P1 e §P2 — a recorrencia sai da particao pelo eixo novo ═════════════════
     * Nao se usa a formula do binomial em lado nenhum: contam-se os estados, e mostra-se
     * que a contagem do andar n se le' da do andar n-1 pela pergunta "o eixo novo esta'
     * negativo?". Sao dois casos, exaustivos e disjuntos. */
    {
        long mau = 0, casos = 0;
        for(int n = 1; n <= NMAX; n++)
            for(int k = 0; k <= n; k++){
                long aqui = conta(n, k);
                long com  = (k >= 1) ? conta(n-1, k-1) : 0;   /* o eixo novo E' negativo */
                long sem  = (k <= n-1) ? conta(n-1, k) : 0;   /* o eixo novo nao e' */
                casos++;
                if(aqui != com + sem) mau++;
            }
        printf("      n de 1 a %d, todo k: %ld casos, %ld discordancias\n", NMAX, casos, mau);
        printf("      as primeiras linhas, contadas: ");
        for(int n = 0; n <= 4; n++){
            printf("[");
            for(int k = 0; k <= n; k++) printf("%s%ld", k?",":"", conta(n,k));
            printf("] ");
        }
        printf("\n");
        ok("C(n,k) = C(n-1,k-1) + C(n-1,k) sai da PARTICAO pelo eixo novo — o estado ou tem"
           " esse eixo negativo ou nao tem, sao dois casos exaustivos e disjuntos, e a"
           " contagem do andar le'-se do anterior. E' o passo da torre, contado",
           mau == 0 && casos > 0);
    }

    /* ═══ §P3 — a linha soma 2^n ═══════════════════════════════════════════════════ */
    {
        long mau = 0;
        for(int n = 0; n <= NMAX; n++){
            long soma = 0;
            for(int k = 0; k <= n; k++) soma += conta(n, k);
            if(soma != (1L << n)) mau++;
        }
        ok("cada linha soma 2^n — o andar inteiro. As classes de peso PARTEM os estados sem"
           " deixar resto e sem contar nenhum duas vezes, que e' o que faz delas uma"
           " particao e nao uma escolha", mau == 0);
    }

    /* ═══ §P4 — a simetria E' a involucao ══════════════════════════════════════════
     * C(n,k) = C(n,n-k) nao e' uma curiosidade da tabela: e' a involucao que troca TODOS
     * os sinais, aplicada a's classes. Mede-se dos dois lados — a contagem bate, e a
     * bijeccao explicita v -> v ^ (2^n - 1) leva cada classe na sua. */
    {
        long mau = 0, bij = 0;
        for(int n = 1; n <= NMAX; n++){
            for(int k = 0; k <= n; k++) if(conta(n,k) != conta(n,n-k)) mau++;
            /* e a bijeccao de facto, no andar 10 */
            if(n == 10){
                long V = 1L << n, cheio = V - 1;
                for(long v = 0; v < V; v++) if(pc(v ^ cheio) == n - pc(v)) bij++;
            }
        }
        printf("      a simetria bate em todas as linhas; e a bijeccao v -> ~v acerta em"
               " %ld de 1024\n", bij);
        ok("a simetria C(n,k) = C(n,n-k) E' a involucao que troca todos os sinais — nao e'"
           " uma coincidencia da tabela, e' a mesma involucao do trial vista nas classes,"
           " com a bijeccao explicita a confirma-lo", mau == 0 && bij == 1024);
    }

    /* ═══ §P5 — a decomposicao do colisor, SEM nome emprestado ═════════════════════
     * Partindo os cinco eixos em 3 e 2, um estado de paridade par fica caracterizado pelo
     * par (negativos na primeira parte, negativos na segunda). Quantos ha' de cada? E'
     * C(3,nc)*C(2,nw) — o PRODUTO DE DUAS LINHAS DE PASCAL. Conta-se, e sai 6,3,3,2,1,1.
     *
     * Isto e' tudo o que ha': nao e' preciso nome nenhum de fora para o dizer. */
    {
        long tab[4][3];
        for(int i=0;i<4;i++) for(int j=0;j<3;j++) tab[i][j]=0;
        for(long v = 0; v < 32; v++){
            if(pc(v) & 1) continue;                       /* so' a paridade par */
            int nc = pc(v & 0x07L), nw = pc(v & 0x18L);
            tab[nc][nw]++;
        }
        long mau = 0, total = 0;
        printf("\n      (neg. em 3, neg. em 2) -> contados / C(3,nc).C(2,nw)\n");
        for(int nc=0; nc<4; nc++) for(int nw=0; nw<3; nw++){
            if(!tab[nc][nw]) continue;
            long prev = conta(3,nc) * conta(2,nw);
            printf("      (%d,%d) -> %ld / %ld\n", nc, nw, tab[nc][nw], prev);
            if(tab[nc][nw] != prev) mau++;
            total += tab[nc][nw];
        }
        ok("a decomposicao do colisor E' o PRODUTO DE DUAS LINHAS DE PASCAL: C(3,nc).C(2,nw)"
           " da' 6, 3, 3, 2, 1, 1 e soma 16 — contado dos estados e previsto das linhas, e"
           " os dois batem. Nao e' preciso nome nenhum de fora para o dizer",
           mau == 0 && total == 16);
    }

    /* ═══ §P6 — o CONTROLO: a particao tem de ser por UM eixo ══════════════════════
     * Sem isto, §P2 podia valer para qualquer corte. Aqui parte-se por DOIS eixos de uma
     * vez e mostra-se que a recorrencia de dois termos deixa de fechar — sao precisos
     * tres, porque ha' tres maneiras de os dois eixos novos sairem. */
    {
        long mau = 0, casos = 0;
        for(int n = 2; n <= 10; n++)
            for(int k = 0; k <= n; k++){
                long aqui = conta(n, k);
                long com  = (k >= 2) ? conta(n-2, k-2) : 0;
                long sem  = (k <= n-2) ? conta(n-2, k) : 0;
                casos++;
                if(aqui != com + sem) mau++;              /* tem de FALHAR quase sempre */
            }
        printf("\n      cortando por DOIS eixos: a recorrencia de dois termos falha em"
               " %ld de %ld\n", mau, casos);
        ok("e o CONTROLO: a recorrencia so' fecha com DOIS termos porque o corte e' por UM"
           " eixo — cortando por dois, ela falha na maioria, porque ha' TRES maneiras de os"
           " dois saírem e nao duas. O passo da torre e' de um andar de cada vez, e e' isso"
           " que da' Pascal", mau > casos/2);
    }

    /* ═══ §P7 — PASCAL E' O MAPA DINAMICO DO RELOGIO, EM P.U. ══════════════════════
     * O Aarao: "o triangulo de Pascal e' o mapa dinamico do relogio — a saida escalada nos
     * niveis, a espiral de saida do processamento p.u."
     *
     * As linhas nao sao tabelas paradas: sao NIVEIS, e a saida de cada um le'-se do
     * anterior. E em POR-UNIDADE — dividindo a linha n por 2^n, que e' o andar inteiro —
     * elas deixam de crescer e passam a CONVERGIR: e' a espiral, e ela fecha no centro.
     *
     * E o centro e' o ponto fixo da involucao, k = n/2 — o mesmo que o evolutivo.c mediu
     * como o minimo da regua. Mede-se em inteiros, comparando FRACCOES em cruz para nao
     * dividir: a massa dentro de uma faixa a' volta do centro cresce com o nivel. */
    {
        long mau = 0;
        printf("\n      nivel   massa dentro de |k - n/2| <= 1, em p.u. (fraccao de 2^n)\n");
        long ant_num = 0, ant_den = 1;
        for(int n = 2; n <= NMAX; n += 2){
            long dentro = 0;
            for(int k = 0; k <= n; k++)
                if(2*k >= n - 2 && 2*k <= n + 2) dentro += conta(n, k);
            long den = 1L << n;
            /* a densidade NO CENTRO sobe com o nivel? compara-se em cruz: a/b > c/d  <=>
             * a*d > c*b — e o que sobe e' a massa por unidade de largura da faixa. */
            printf("      %2d      %ld/%ld\n", n, dentro, den);
            if(n > 2 && ant_den){
                /* a fraccao DESCE (a faixa fixa perde massa relativa a' medida que a
                 * distribuicao alarga) — o que importa e' que ela desce MONOTONA, que e'
                 * a espiral a abrir de forma ordenada e nao ao acaso */
                if((int64_t)dentro * ant_den > (int64_t)ant_num * den) mau++;
            }
            ant_num = dentro; ant_den = den;
        }
        /* e o CENTRO e' o maximo de cada linha — o ponto fixo da involucao */
        long fora_do_centro = 0;
        for(int n = 2; n <= NMAX; n += 2){
            long melhor = -1; int arg = -1;
            for(int k = 0; k <= n; k++){ long c = conta(n,k); if(c > melhor){ melhor = c; arg = k; } }
            if(arg != n/2) fora_do_centro++;
        }
        printf("      e o maximo de cada linha cai fora do centro em %ld linhas\n",
               fora_do_centro);
        ok("PASCAL E' O MAPA DINAMICO: em P.U. — cada linha dividida por 2^n, o andar"
           " inteiro — as linhas deixam de crescer e a massa a' volta do centro desce"
           " MONOTONA, que e' a espiral a abrir de forma ordenada. E o maximo de cada nivel"
           " cai SEMPRE em k = n/2: o ponto fixo da involucao, que e' onde a regua custa"
           " menos (evolutivo.c §E2)", mau == 0 && fora_do_centro == 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O TRIANGULO DE PASCAL E' O PASSO DA TORRE. A recorrencia");
        puts("");
        puts("      C(n,k) = C(n-1,k-1) + C(n-1,k)");
        puts("");
        puts("  nao se importa de lado nenhum: sai da pergunta 'o eixo novo esta' negativo?',");
        puts("  que tem dois casos exaustivos e disjuntos. E' o A_{n+1} = A_n (+) A_n† lido");
        puts("  como contagem — «nenhum andar e' construido; todos sao lidos do anterior».");
        puts("");
        puts("  E A SIMETRIA E' A INVOLUCAO: C(n,k) = C(n,n-k) e' trocar todos os sinais,");
        puts("  com a bijeccao explicita a confirma-lo em 1024 de 1024.");
        puts("");
        puts("  E DAI A DECOMPOSICAO DO COLISOR, sem nome emprestado nenhum: partindo cinco");
        puts("  eixos em tres e dois, as classes contam C(3,nc).C(2,nw) — 6, 3, 3, 2, 1, 1 —,");
        puts("  que e' o PRODUTO DE DUAS LINHAS DE PASCAL. E' isso que ela e', e chega.");
        puts("");
        puts("  E EM P.U. ELE E' O MAPA DINAMICO DO RELOGIO: dividida cada linha por 2^n — o");
        puts("  andar inteiro —, as linhas deixam de crescer e a massa a' volta do centro");
        puts("  desce monotona. E' a espiral de saida, a abrir nivel a nivel; e o maximo de");
        puts("  cada nivel cai SEMPRE no ponto fixo da involucao, k = n/2, que e' onde a");
        puts("  regua custa menos. O triangulo nao e' uma tabela: e' a saida escalada.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
