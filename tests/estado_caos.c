/* estado_caos.c — ACUMULAR ESTADO NÃO É SÓ REDUNDANTE: introduz vazamento que propaga e vira caos.
 *
 * O Aarão: «não é só redundante acumular estado, isso introduz vazamento que se propaga --- deriva
 * sistema dinâmico e caótico.»
 *
 * A derivação. Guardar estado torna a máquina uma RECORRÊNCIA  s_{n+1} = f(s_n, u_n): o estado de
 * um passo alimenta o seguinte. Um vazamento --- um bit corrompido no dado u_j, ou uma escrita
 * selvagem no estado --- ENTRA na recorrência e propaga: a jusante, todo passo depende dele. E se
 * f EXPANDE (a derivada tem módulo > 1), o vazamento AMPLIFICA como ∏ f' ; é dependência sensível
 * às condições iniciais --- CAOS (a expoente de Lyapunov λ = log|f'| > 0). O mapa expansivo
 * canónico é o dobrador s -> 2s (o shift de Bernoulli): λ = log 2 > 0.
 *
 * O reversível NÃO tem essa recorrência. A estrela trata cada dado independente e reversível
 * (ν∘ν = id, resíduo 0): não há s_n a alimentar s_{n+1}, logo o vazamento não tem onde acumular ---
 * fica no dado que o sofreu, e mais nada. λ = 0 (a involução é neutra, |ν'| = 1 no ponto fixo |x|=1).
 * NÃO usar malloc não é higiene: é REMOVER o canal que transforma um vazamento em caos.
 *
 *   §D1  ESTADO expansivo: corromper UM dado contamina TODOS os passos a jusante, e a contaminação
 *        DOBRA a cada passo (2^k) --- λ = log 2 > 0, dependência sensível: caos
 *   §D2  SEM estado (a estrela): corromper um dado afeta SÓ a sua saída; as outras têm resíduo 0
 *   §D3  a involução ν(x) = -1/x é reversível (ν∘ν = id, resíduo 0) e neutra (λ = 0): sem canal,
 *        sem propagação --- é a razão de não haver malloc, não a redundância
 *
 *   cc -O2 -std=c99 -Wall -I../lib estado_caos.c -o estado_caos && ./estado_caos
 */
#include <stdio.h>
#include "reta.h"      /* as operações da recta */
#include "unidade.h"

typedef long L;
static const L P = 2147483647; /* 2^31 - 1, primo de Mersenne: o corpo dos dados */

/* a estrela: ν(x) = -1/x  (mod P). ν∘ν = id. */
static L estrela(L x){ if(x % P == 0) return 0; return (P - rt_inv_mod(x, P)) % P; }

int main(void){
    printf("=== ACUMULAR ESTADO INTRODUZ VAZAMENTO QUE PROPAGA E VIRA CAOS ============\n\n");

    enum { N = 40, J = 5 };                 /* fluxo de N dados; corrompe-se o dado J */
    L u[N];
    for(int n = 0; n < N; n++) u[n] = (17*n + 3) % P;   /* um fluxo qualquer, determinista */

    /* ── §D1 COM estado (recorrência expansiva s -> 2s + u): o vazamento contamina tudo e amplifica ── */
    /* s_{n+1} = (2 s_n + u_n) mod P. Corromper u_J por +1 injeta δ=1 em s_{J+1}, e daí
     * d_n = 2^{n-J-1} mod P para n > J: DOBRA a cada passo (expande) e nunca é 0 (P primo, 2 invertível).
     * Logo TODOS os N-J-1 passos a jusante ficam contaminados, e a magnitude cresce 2^k --- caos. */
    L s_lim = 0, s_cor = 0;
    L u_cor_J = (u[J] + 1) % P;             /* o dado J corrompido por um vazamento de +1 */
    /* a correção entra em s no próprio passo J: d_J = u_cor_J - u[J] = 1 = 2^0, e daí d_n = 2^(n-J). */
    int contaminados = 0, dobra_sempre = 1; L esperado = 1;   /* d_J = 2^0 = 1 */
    printf("§D1  estado expansivo s->2s+u, vazamento +1 no dado J=%d:\n", J);
    printf("      passo n | d_n = |s_cor - s_lim| mod P | 2^(n-J)\n");
    for(int n = 0; n < N; n++){
        L uu_lim = u[n];
        L uu_cor = (n == J) ? u_cor_J : u[n];
        s_lim = (2*s_lim + uu_lim) % P;
        s_cor = (2*s_cor + uu_cor) % P;
        if(n >= J){
            L d = ((s_cor - s_lim) % P + P) % P;
            if(d != 0) contaminados++;              /* contou um passo contaminado (de J em diante) */
            if(d != esperado) dobra_sempre = 0;     /* d_n = 2^(n-J), a amplificação exata */
            if(n <= J + 5)
                printf("        %3d | %26ld | %ld\n", n, d, esperado);
            esperado = (2*esperado) % P;            /* 2^(n-J+1) para o próximo */
        }
    }
    printf("      ... contaminados de J em diante: %d de %d passos\n\n", contaminados, N - J);
    ok("§D1 COM estado a recorrencia e' expansiva (s->2s): o vazamento de UM dado contamina TODOS os"
       " N-J passos de J em diante e a contaminacao DOBRA a cada passo (d_n = 2^(n-J)) --- dependencia"
       " sensivel, expoente de Lyapunov log 2 > 0: CAOS. O estado e' o canal que propaga e amplifica",
       contaminados == N - J && dobra_sempre);

    /* ── §D2 SEM estado (a estrela, cada dado independente): o vazamento não sai do dado ──────── */
    /* y_n = ν(u_n), sem recorrência: y_n não depende de y_{n-1}. Corromper u_J muda SÓ y_J; todo
     * outro y_n fica idêntico (resíduo 0). O vazamento fica contido. */
    int afetados = 0, outros_zero = 1;
    for(int n = 0; n < N; n++){
        L y_lim = estrela(u[n]);
        L y_cor = estrela((n == J) ? u_cor_J : u[n]);
        L d = ((y_cor - y_lim) % P + P) % P;
        if(d != 0){ afetados++; if(n != J) outros_zero = 0; }   /* só J pode diferir */
    }
    printf("§D2  sem estado y_n = estrela(u_n): afetados pelo vazamento = %d (so' o dado J)\n\n", afetados);
    ok("§D2 SEM estado (a estrela, cada dado independente e reversivel) o vazamento NAO propaga: corromper"
       " o dado J afeta SO' a saida J (afetados=1); toda outra saida tem residuo 0. Sem recorrencia, o"
       " vazamento fica CONTIDO no dado que o sofreu", afetados == 1 && outros_zero);

    /* ── §D3 a involução é reversível (ν∘ν=id) e neutra (λ=0): sem canal, sem caos ───────────── */
    /* ν∘ν = id em todo o corpo (resíduo 0): é a estrela, Lei 1. Reversível => cada passo se desfaz
     * repetindo, não lembrando => não precisa de estado => não há recorrência => λ = 0. */
    int involucao_ok = 1;
    for(L x = 1; x < 20000; x++)
        if(estrela(estrela(x)) != x % P){ involucao_ok = 0; break; }
    printf("§D3  ν∘ν = id em [1,20000): resíduo 0 (reversível, Lei 1) --> λ = 0, sem propagação\n\n");
    ok("§D3 a involucao ν(x)=-1/x cumpre ν∘ν=id (residuo 0): reversivel, logo desfaz-se REPETINDO e nao"
       " LEMBRANDO --- nao precisa de estado, nao ha' recorrencia, λ=0. Nao usar malloc nao e' redundancia:"
       " e' REMOVER o canal (o estado) que transforma um vazamento em caos", involucao_ok);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  Acumular estado torna a maquina uma recorrencia s_{n+1}=f(s_n,u_n): um vazamento entra");
        puts("  e propaga; se f expande (o dobrador, λ=log2>0) AMPLIFICA a cada passo --- caos, dependencia");
        puts("  sensivel (§D1). A estrela trata cada dado independente e reversivel (ν∘ν=id, resíduo 0):");
        puts("  o vazamento fica no dado, nao sai (§D2), e λ=0 (§D3). Nao ha' malloc porque o estado E' o");
        puts("  canal que transforma um vazamento num sistema caotico --- removido o estado, removido o caos.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
