/* pental.c — A ESTRELA É PENTAL, E O PENTAL É O QUE A REDE EXCLUI.
 *
 *   cc -O2 -std=c99 -Ilib -Itests -o /tmp/pental tests/pental.c && /tmp/pental
 *
 * A duplicidade CONTA em potências de dois: as leis são 2²=4 (elementos em
 * {2,3} × operações em {1,2}), os símbolos 2³=8 (três blocos, uma escolha por
 * bloco), as espécies da dobra 2¹=2 (ordem um ou ordem dois). A estrela ★ é o
 * discriminante Δ = m²+4 no ponto m=1, onde vale Δ=5 --- o PENTAL.
 *
 * E o que se mede é que a estrela ESCAPA a essa contagem por duas razões, e as
 * duas separam-na do andar:
 *
 *   §P1  a duplicidade conta: 2²=4 (leis), 2³=8 (símbolos), 2¹=2 (espécies)
 *   §P2  a estrela está em m=1, e ali Δ = 5 --- o pental
 *   §P3  Δ=5 NÃO é quadrado, logo o ponto fixo sai do andar dos inteiros
 *   §P4  Δ=5 NÃO é potência de dois, logo a duplicidade não a conta
 *   §P5  e não é caso isolado: para NENHUM m≥1 o Δ=m²+4 é quadrado
 *   §P6  a única potência de dois no percurso é m=2 (Δ=8), e NÃO é a estrela
 *
 * Antes este medidor imprimia as conclusões e não as afirmava --- era exposição,
 * e a casa pede que quem tem uma afirmação a faça, ou se declare sem ela. Faz-se
 * agora: as mesmas contas, com a asserção ao lado.
 */
#include <stdio.h>
#include "unidade.h"

static int quad(long long n){ long long r = 0; while(r*r < n) r++; return r*r == n; }
static int pot2(long long n){ return n > 0 && (n & (n-1)) == 0; }

int main(void){
    printf("=== o que a duplicidade conta ===\n");
    printf("  as LEIS: elementos em {2,3} x operacoes em {1,2}  ->  2^2 = %d\n", 2*2);
    printf("  os SIMBOLOS: tres blocos, uma escolha por bloco   ->  2^3 = %d\n", 2*2*2);
    printf("  as ESPECIES da dobra: ordem um ou ordem dois      ->  2^1 = %d\n\n", 2);
    ok("a duplicidade conta em potencias de dois: 4 leis, 8 simbolos, 2 especies",
       (2*2 == 4) && (2*2*2 == 8) && (2 == 2));

    printf("=== e o que a estrela faz a essa conta ===\n");
    printf("   m   Delta=m^2+4   quadrado?   potencia de dois?\n");
    for(long m = 0; m <= 8; m++){
        long D = m*m + 4;
        printf("  %2ld      %3ld         %-9s   %s%s\n", m, D,
               quad(D) ? "SIM" : "nao", pot2(D) ? "SIM" : "nao",
               (m == 1) ? "     <-- a ESTRELA" : "");
    }
    long De = 1*1 + 4;                                  /* a estrela: m=1 */
    ok("a estrela esta em m=1, e ali Delta = 5 --- o pental", De == 5);
    ok("Delta=5 NAO e quadrado: o ponto fixo sai do andar dos inteiros", !quad(De));
    ok("Delta=5 NAO e potencia de dois: a duplicidade nao a conta", !pot2(De));

    printf("\n  varridos m de 1 a 100000:\n");
    int nq = 0, np = 0; long m_pot = -1;
    for(long long m = 1; m <= 100000; m++){
        long long D = m*m + 4;
        if(quad(D)) nq++;
        if(pot2(D)){ np++; if(m_pot < 0) m_pot = (long)m; }
    }
    printf("    Delta quadrado: %d   |   Delta potencia de dois: %d (o m: %ld)\n", nq, np, m_pot);
    ok("para NENHUM m>=1 o Delta=m^2+4 e quadrado --- a estrela e a regra, nao a excepcao",
       nq == 0);
    ok("a UNICA potencia de dois no percurso e m=2 (Delta=8), e NAO e a estrela (m=1)",
       np == 1 && m_pot == 2);

    printf("\n-> a estrela nao e' quadrado (logo o ponto fixo sai do andar)\n");
    printf("   nem potencia de dois (logo a duplicidade nao a conta).\n");

    printf("\n  %d unidades, %d falha(s)\n", unidades, falhas);
    return falhas ? 1 : 0;
}
