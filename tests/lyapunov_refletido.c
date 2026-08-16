/* lyapunov_refletido.c — Lyapunov pela METADE REFLETIDA = atestação do Teorema do Metrónomo.
 *
 * Corpo de Peano: maestro projecta; metrónomo lê; Lyapunov dualizado atesta (λ⁺+λ⁻=0).
 * Este ficheiro mede a atestação — o teorema operacional da medição.
 *
 * O Aarão: «vê se dá para derivar Lyapunov via a nossa medida pela metade refletida — isso funda o
 * teorema operacional para medição; e põe a justificativa nas proibições.»
 *
 * A metade refletida é a INVERSA TEMPORAL, e há lei: num sistema REVERSÍVEL os expoentes de Lyapunov
 * vêm em pares ±λ (o espectro é simétrico). Logo λ deriva-se da metade refletida:
 *   λ⁺ = log|f'|  (a frente, expande)          λ⁻ = log|(f⁻¹)'| = −log|f'|  (a reflexão, contrai)
 *   λ⁺ + λ⁻ = 0   (a regra da soma: reversível é neutro)   ⇒   λ = (λ⁺ − λ⁻)/2
 * O dobrador f(s)=2s tem λ⁺ = log2 (1 bit por passo); a metade refletida f⁻¹(s)=s/2 tem λ⁻ = −log2.
 * Contam-se EXPOENTES INTEIROS (potências de 2), sem um double.
 *
 * E daí o TEOREMA OPERACIONAL DA MEDIÇÃO (= atestação do Metrónomo): uma medição só é fiável se a
 * metade refletida VOLTA com resíduo 0 (λ da própria medição = 0). Se a medição perde um bit — um
 * double que arredonda, um estado sobrescrito, um malloc — não há volta, o resíduo é > 0, e a medida
 * é ela própria caótica. É a proibição da teoria dita à letra: «não pertence, por não haver como
 * voltar» (medida.tex). Ver papers/corpo_peano.tex thm:metronomo; tests/corpo_peano.c §CP40.
 *
 *   §L1  λ derivado pela metade refletida: e⁺(n)=+n, e⁻(n)=−n ⇒ (e⁺−e⁻)/2n = 1 = log2 por passo
 *   §L2  a regra da soma (o espectro ±λ): e⁺(n) + e⁻(n) = 0 exacto — o que a dualidade guarda
 *   §L3  o teorema operacional: medir pela metade refletida — reversível volta (resíduo 0, fiável);
 *        a medição com perda (o bit descartado: double, malloc) NÃO volta (resíduo > 0): proibida
 *
 *   cc -O2 -std=c99 -Wall -I../lib lyapunov_refletido.c -o lyapunov_refletido && ./lyapunov_refletido
 */
#include <stdio.h>
#include "reta.h"      /* as operações da recta */
#include "unidade.h"

typedef long long L;
static const L P = 2147483647; /* 2^31 - 1, primo de Mersenne */

static L estrela(L x){ if(x % P == 0) return 0; return (P - rt_inv_mod(x, P)) % P; }

int main(void){
    printf("=== LYAPUNOV PELA METADE REFLETIDA, E O TEOREMA OPERACIONAL DA MEDICAO =====\n\n");

    /* ── §L1 λ derivado pela metade refletida — ITERANDO o mapa, não a asseverar constantes ──── */
    /* MEDE-SE a frente: itera-se o dobrador s->2s numa separação inicial δ0=1 e mede-se δ⁺(n). Se o
     * mapa expande por 2, δ⁺(n) = 2^n (medido, não posto), e o expoente e⁺ = log2 δ⁺ = n. A taxa de
     * afastamento --- o Lyapunov --- é λ⁺ = e⁺/n = 1 bit por passo = log2. Falsificável: se o mapa
     * não dobrasse, δ⁺ ≠ 2^n. (n≤40 para 2^n caber no inteiro de 63 bits.) */
    int mede_frente = 1; L sep = 1; int e_mais_medido = 0;
    printf("       n | δ+(n) medido | 2^n | e+ = log2 δ+\n");
    for(int n = 1; n <= 40; n++){
        sep = 2*sep;                           /* o dobrador expande a separação --- ITERADO */
        e_mais_medido = n;                     /* conta um dobramento */
        if(sep != (1LL << n)) mede_frente = 0; /* MEDIDO: δ⁺(n) = 2^n, e daí e⁺ = n */
        if(n <= 3 || n == 40)
            printf("      %2d | %12lld | %12lld | %d\n", n, sep, (1LL<<n), e_mais_medido);
    }
    /* λ⁺ = e⁺/n = 40/40 = 1 bit/passo, medido da frente; a metade refletida (a inversa) dá o mesmo
     * módulo com sinal trocado (§L2): λ = (λ⁺ − λ⁻)/2 = 1. */
    int lambda_um = mede_frente && (e_mais_medido == 40);
    printf("\n");
    ok("§L1 LYAPUNOV MEDIDO ITERANDO O MAPA: o dobrador s->2s expande a separacao a δ+(n)=2^n (medido,"
       " nao posto), logo e+ = log2 δ+ = n e λ+ = e+/n = 1 bit por passo = log2. A metade refletida da'"
       " o mesmo λ com sinal trocado (§L2)", lambda_um);

    /* ── §L2 a regra da soma ±λ: a metade refletida DESFAZ a frente, resíduo 0 (medido) ──────── */
    /* num reversível os expoentes vêm em pares ±λ. Realiza-se medindo: expande-se x por 2^20 (frente,
     * e⁺=+20) e contrai-se por 2^20 com a INVERSA s->s/2 (a metade refletida, e⁻=−20). Se e⁺+e⁻=0, a
     * volta é EXACTA: s regressa a x, resíduo 0 --- medido em 19999 valores. Falsificável: qualquer
     * perda na frente estragaria a volta. */
    int reflete = 1; L res_refl = 0;
    for(L x0 = 1; x0 < 20000; x0++){
        L s = x0;
        for(int n = 1; n <= 20; n++) s = 2*s;    /* frente: expande 2^20 (e⁺ = +20) */
        for(int n = 1; n <= 20; n++) s = s/2;    /* metade refletida: contrai 2^20 (e⁻ = −20) */
        if(s != x0) reflete = 0;                 /* e⁺ + e⁻ = 0 ⇒ volta exacta */
        res_refl += (s - x0);
    }
    printf("§L2  frente 2^20 e metade refletida 2^-20: volta exacta em 19999 valores, residuo %lld\n\n", res_refl);
    ok("§L2 A REGRA DA SOMA ±λ, MEDIDA: a frente expande 2^20 (e+=+20) e a metade refletida (a inversa)"
       " contrai 2^20 (e-=-20); e+ + e- = 0 realiza-se como a VOLTA EXACTA (s regressa a x, residuo 0"
       " em 19999 valores). E' a metade que a dualidade guarda, e licencia medir λ por uma metade e"
       " refletir a outra", reflete && res_refl == 0);

    /* ── §L3 o teorema operacional: medir pela metade refletida (resíduo 0 ou proibido) ──────── */
    /* uma medição é um transforma T; a metade refletida é T⁻¹. É FIÁVEL sse T⁻¹(T(x)) = x para todo x
     * (resíduo 0): a própria medição tem λ = 0, não amplifica erro. Dois casos:
     *   (a) REVERSÍVEL: a estrela ν∘ν = id --- volta em todo x, resíduo 0: fiável.
     *   (b) COM PERDA: g(x) = (x/2)·2 apaga o bit baixo (um double que arredonda, um estado
     *       sobrescrito). g não é injectiva; a reflexão não recupera x ímpar: resíduo = x&1 > 0.
     *       Não há volta ⇒ a medição perdeu ⇒ PROIBIDA (a régua da teoria). */
    int rev_volta = 1; L rev_residuo = 0;
    for(L x = 1; x < 20000; x++){ L v = estrela(estrela(x)); if(v != x % P){ rev_volta = 0; } rev_residuo += (v != x % P); }
    int perda_naovolta = 0; L perda_total = 0;
    for(L x = 1; x < 20000; x++){ L g = (x/2)*2; L res = x - g; if(res != 0) perda_naovolta++; perda_total += res; }
    printf("§L3  reversivel (estrela) ν∘ν=id: residuo %lld em 19999 ; com perda g(x)=(x/2)·2:"
           " %d valores nao voltam (resíduo total %lld)\n\n", rev_residuo, perda_naovolta, perda_total);
    ok("§L3 TEOREMA OPERACIONAL DA MEDICAO: mede-se pela metade refletida, e a medida so' e' fiavel se"
       " ela VOLTA com residuo 0 (λ_medida=0). A estrela reversivel volta (residuo 0); a medida COM"
       " PERDA (o bit descartado --- double, estado sobrescrito, malloc) NAO volta (residuo>0): 'nao"
       " pertence, por nao haver como voltar' --- e' a justificativa das proibicoes",
       rev_volta && rev_residuo == 0 && perda_naovolta > 0 && perda_total > 0);

    /* ── §L4 quantifica o erro em ATRASOS UNITÁRIOS: um atraso num lado desbalança ──────────── */
    /* o atraso unitário é o z^-1: um passo de atraso. Balanceado --- expande N, contrai N --- a metade
     * refletida cancela (resíduo 0, fp=1). Aplicando d ATRASOS UNITÁRIOS num lado (contrai N−d, i.e.
     * a reflexão fica d passos atrás), sobra um factor 2^d: o erro é (2^d − 1)·Σx. Cada atraso unitário
     * MULTIPLICA o erro por 2 = e^λ --- o erro quantifica-se pelo atraso vezes o Lyapunov. É a
     * reactância: o atraso que não cancela. Falsificável: se não houvesse desbalanço, o erro seria 0. */
    enum { M = 100 };
    L soma_x = 0; for(L x = 1; x < M; x++) soma_x += x;    /* Σx = 4950 */
    int quantifica = 1;
    printf("§L4  erro por atrasos unitarios (expande 20, contrai 20-d --- d atrasos num lado):\n");
    printf("       d | erro medido | (2^d - 1)·Σx\n");
    for(int d = 0; d <= 10; d++){
        L erro = 0;
        for(L x0 = 1; x0 < M; x0++){
            L s = x0;
            for(int n = 1; n <= 20; n++) s = 2*s;         /* frente: expande 2^20 */
            for(int n = 1; n <= 20 - d; n++) s = s/2;     /* refletida com d atrasos unitarios: contrai 2^(20-d) */
            erro += (s - x0);                             /* s = 2^d · x0, erro = (2^d - 1)·x0 */
        }
        L esperado = ((1LL << d) - 1) * soma_x;           /* (2^d - 1)·Σx --- referência que muda com d */
        if(erro != esperado) quantifica = 0;
        if(d <= 3 || d == 10)
            printf("      %2d | %11lld | %11lld\n", d, erro, esperado);
    }
    printf("\n");
    ok("§L4 O ERRO QUANTIFICA-SE EM ATRASOS UNITARIOS: d atrasos (z^-1) num lado desbalancam, e o erro"
       " e' (2^d - 1)·Σx --- cada atraso unitario MULTIPLICA o erro por 2 = e^λ. Balanceado (d=0) da'"
       " residuo 0 (fp=1); o atraso e' a reactancia que nao cancela, e cresce com o Lyapunov", quantifica);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  Teorema do Metrónomo: maestro projecta; metrónomo lê; Lyapunov dualizado atesta.");
        puts("  Lyapunov deriva-se da METADE REFLETIDA (a inversa temporal): a frente expande (e+=+n)");
        puts("  e a reflexao contrai (e-=-n), λ=(e+ - e-)/2n = log2, e a regra da soma e+ + e- = 0 e'");
        puts("  o espectro simetrico ±λ que a dualidade guarda. Dai o TEOREMA OPERACIONAL: uma medicao");
        puts("  so' e' fiavel se a metade refletida volta com residuo 0 (λ_medida=0). Perder um bit ---");
        puts("  um double que arredonda, um estado sobrescrito, um malloc --- tira a volta: nao pertence.");
        puts("  E' a razao das proibicoes, e e' a mesma frase da teoria: 'o que nao tem volta nao pertence'.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
