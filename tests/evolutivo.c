/* evolutivo.c — A NORMA DO CORPO EVOLUTIVO E' O INVERSO DA REGUA DE FISHER.
 *
 * O Aarao: "evolucao/involucao e' todo o processo. ve o corpo evolutivo no catalogo, ve
 * se esta' bem fundamentado na teoria."
 *
 * Nao estava: `evolu` aparece ZERO vezes em teoria.tex (controlo: `corpo` aparece 108).
 * O corpo esta' todo no catalogo — norma 1-s^2 com s=2p-1, involucao p->1-p, replicador
 * p_i -> p_i w_i / <w> — e a teoria tem as duas metades dele com OUTRO nome: clone e
 * reproducao (sec:clonerep), e a regua de Fisher g(p) = 1/(p(1-p)).
 *
 * Este medidor coze as duas metades, e a costura e' uma identidade INTEIRA:
 *
 *   §E1  (1-s^2) . g(p) = 4, exacto, para todo p — a norma E' o inverso da regua
 *   §E2  o ponto fixo da involucao p->1-p e' onde a regua custa MENOS
 *   §E3  e a reproducao so' sobe: <w>' >= <w>, com igualdade SO' no ponto fixo
 *   §E4  e o CONTROLO: a dinamica trocada DESCE — logo §E3 nao passa por decreto
 *
 * Zero doubles: tudo em inteiros. Os racionais andam como pares (numerador, denominador)
 * e as comparacoes fazem-se em cruz, sem dividir uma unica vez.
 *
 *   cc -O2 -std=c99 -Wall -I../lib evolutivo.c -o evolutivo
 */
#include <stdio.h>
#include "unidade.h"

#define NT 6                          /* tipos de peca no exercito */

int main(void){
    puts("\n  O CORPO EVOLUTIVO — a norma e' o inverso da regua, e o ponto fixo e' o minimo\n");

    /* ═══ §E1 — (1-s^2) . g(p) = 4 ═════════════════════════════════════════════════
     * p = k/N, e dai s = 2p-1 = (2k-N)/N. A norma do corpo evolutivo (catalogo) e'
     *
     *     1 - s^2 = (N^2 - (2k-N)^2) / N^2
     *
     * e a regua de Fisher (teoria, §2630, escrita la' como g = N^2/(k(N-k))) e'
     *
     *     g(p) = 1 / (p(1-p)) = N^2 / (k(N-k)).
     *
     * O produto e' (N^2 - (2k-N)^2) / (k(N-k)), e a afirmacao e' que da' 4 SEMPRE. Isso
     * reduz-se a uma identidade de inteiros, e e' ela que se testa — sem dividir:
     *
     *     N^2 - (2k-N)^2  ==  4 . k . (N-k)
     *
     * Isto PODE falhar: basta o sinal do s estar trocado, ou a norma ser 1+s^2, ou a
     * regua ser p(1-p) em vez do inverso. Varre-se N e k em vez de escolher um. */
    {
        long long mau = 0, casos = 0, pior_esq = 0, pior_dir = 0;
        for(long long N = 2; N <= 400; N++)
            for(long long k = 1; k < N; k++){
                long long s_num = 2*k - N;                    /* s = s_num / N */
                long long esq = N*N - s_num*s_num;            /* numerador de 1-s^2 */
                long long dir = 4*k*(N-k);                    /* o que a regua desconta */
                casos++;
                if(esq != dir){ mau++; if(!pior_esq){ pior_esq = esq; pior_dir = dir; } }
            }
        printf("      N de 2 a 400, todo k interior: %lld casos\n", casos);
        printf("      N^2-(2k-N)^2  contra  4k(N-k):  %lld discordancias\n", mau);
        if(mau) printf("      primeira: %lld != %lld\n", pior_esq, pior_dir);
        ok("(1-s^2) . g(p) = 4 EXACTO para todo p: a norma do corpo evolutivo e' o INVERSO"
           " da regua de Fisher — o catalogo e a teoria mediam o mesmo corpo com nomes"
           " diferentes, e a costura e' uma identidade inteira", mau == 0);
    }

    /* ═══ §E2 — o ponto fixo da involucao e' o minimo da regua ═════════════════════
     * A involucao do corpo e' p -> 1-p, isto e' k -> N-k. O ponto fixo e' k = N/2.
     * A regua e' g = N^2/(k(N-k)), logo ela e' MINIMA onde k(N-k) e' MAXIMO.
     *
     * A teoria ja' tinha o minimo em p=1/2 e diz "derivado e nao escrito" — mas nao diz
     * de que corpo era. Aqui verifica-se que o argmax da regua E' o ponto fixo, para todo
     * N par, achado por varrimento e nao por formula. */
    {
        long long mau = 0, testados = 0;
        for(long long N = 2; N <= 400; N += 2){
            long long melhor = -1, arg = -1;
            for(long long k = 1; k < N; k++){
                long long v = k*(N-k);                        /* 1/g, a menos de N^2 */
                if(v > melhor){ melhor = v; arg = k; }
            }
            testados++;
            /* o argmax tem de ser o ponto fixo da involucao k -> N-k */
            if(arg != N - arg) mau++;
        }
        printf("      %lld valores de N par: o argmax de 1/g e' o ponto fixo de k->N-k\n", testados);
        printf("      discordancias: %lld\n", mau);
        ok("o PONTO FIXO da involucao p->1-p e' exactamente onde a regua custa MENOS —"
           " achado por varrimento, nao por formula: evoluir E' afastar-se do ponto fixo,"
           " e por isso encarece nas pontas", mau == 0 && testados > 0);
    }

    /* ═══ §E3 — a reproducao so' sobe ══════════════════════════════════════════════
     * O replicador do catalogo: p_i -> p_i w_i / <w>. Em inteiros nao ha' divisao nenhuma:
     * se p_i = a_i/D com soma(a) = D, entao
     *
     *     a_i' = a_i . w_i        e        D' = soma(a_i w_i) = D . <w>.
     *
     * <w>  = D'/D          e        <w>' = S2/D'   com S2 = soma(a_i w_i^2).
     *
     * "<w> so' cresce" e' <w>' >= <w>, que em cruz e' S2 . D >= D'^2 — e isso e' Cauchy-
     * Schwarz, com igualdade SO' quando todos os w com peso sao iguais. Nao e' decreto:
     * varrem-se populacoes e aptidoes geradas por uma recorrencia inteira. */
    {
        long long mau = 0, casos = 0, iguais = 0, subiu = 0;
        long long semente = 1;
        for(int rodada = 0; rodada < 2000; rodada++){
            long long a[NT], w[NT], D = 0;
            int uniforme = (rodada % 7 == 0);                 /* de vez em quando, o ponto fixo */
            for(int i = 0; i < NT; i++){
                semente = (semente * 6364136223846793005LL + 1442695040888963407LL);
                a[i] = 1 + ((semente >> 33) & 0x3F);           /* 1..64 */
                semente = (semente * 6364136223846793005LL + 1442695040888963407LL);
                w[i] = uniforme ? 5 : 1 + ((semente >> 35) & 0x0F);   /* 1..16, ou todos 5 */
                D += a[i];
            }
            long long Dl = 0, S2 = 0;
            for(int i = 0; i < NT; i++){ Dl += a[i]*w[i]; S2 += a[i]*w[i]*w[i]; }
            /* <w>' >= <w>  <=>  S2/D' >= D'/D  <=>  S2 . D >= D'^2   (D, D' > 0) */
            casos++;
            if(S2 * D <  Dl * Dl) mau++;                       /* desceu: Fisher falha */
            if(S2 * D == Dl * Dl) iguais++;                    /* parado: o ponto fixo */
            if(S2 * D >  Dl * Dl) subiu++;
            /* e a massa conserva-se por construcao: soma(a') = D', que E' o denominador
             * novo — nao se mede aqui porque dividir uma quantidade por si propria nao e'
             * medir. O que se mede e' o SENTIDO, que e' a lei. */
            if(uniforme && S2 * D != Dl * Dl) mau++;           /* uniforme tem de EMPATAR */
        }
        printf("      %lld populacoes de %d tipos: subiu %lld, empatou %lld, desceu %lld\n",
               casos, NT, subiu, iguais, mau);
        ok("a REPRODUCAO so' sobe — <w>' >= <w> em todas, e o empate acontece SO' onde as"
           " aptidoes sao iguais, que e' o ponto fixo do replicador: a lei de Fisher do"
           " catalogo e' Cauchy-Schwarz no corpo da teoria",
           mau == 0 && subiu > 0 && iguais > 0);
    }

    /* ═══ §E4 — o CONTROLO: e se a dinamica fosse a outra? ═════════════════════════
     * Sem isto, §E3 podia ser verdade por eu ter escolhido bem os numeros. Aqui corre-se a
     * MESMA populacao pela dinamica trocada — a_i' = a_i / w_i, o anti-replicador — e o
     * <w> tem de DESCER. Se descesse nas duas, §E3 nao dizia nada sobre a lei.
     *
     * Em inteiros sem divisao: usa-se u_i = W/w_i com W = produto dos w, que e' proporcional
     * a 1/w_i, e a conta e' a mesma com u no lugar de w para o peso, mantendo w para a
     * aptidao que se mede. */
    {
        long long desceu = 0, nao_desceu = 0;
        long long semente = 99;
        for(int rodada = 0; rodada < 500; rodada++){
            long long a[NT], w[NT], D = 0;
            for(int i = 0; i < NT; i++){
                semente = (semente * 6364136223846793005LL + 1442695040888963407LL);
                a[i] = 1 + ((semente >> 33) & 0x1F);
                semente = (semente * 6364136223846793005LL + 1442695040888963407LL);
                w[i] = 2 + ((semente >> 35) & 0x07);           /* 2..9, e nunca todos iguais */
                D += a[i];
            }
            int todos_iguais = 1;
            for(int i = 1; i < NT; i++) if(w[i] != w[0]) todos_iguais = 0;
            if(todos_iguais) continue;

            long long W = 1; for(int i = 0; i < NT; i++) W *= w[i];   /* <= 9^6 = 531441 */
            /* peso pelo INVERSO: b_i = a_i . (W/w_i) */
            long long Db = 0, Sw = 0, D0 = 0;
            for(int i = 0; i < NT; i++){
                long long b = a[i] * (W / w[i]);
                Db += b;  Sw += b * w[i];  D0 += a[i] * w[i];
            }
            /* <w> antes = D0/D ; <w> depois = Sw/Db.  desce  <=>  Sw . D < D0 . Db */
            if((__int128)Sw * D < (__int128)D0 * Db) desceu++; else nao_desceu++;
        }
        printf("      anti-replicador em %lld populacoes: desceu %lld, nao desceu %lld\n",
               desceu + nao_desceu, desceu, nao_desceu);
        ok("e a dinamica TROCADA faz <w> DESCER em todas — logo o §E3 nao e' uma conta que"
           " sobe por acaso: e' a DIRECCAO do replicador que a faz subir",
           desceu > 0 && nao_desceu == 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O CORPO EVOLUTIVO ESTAVA NOS DOIS DOCUMENTOS COM NOMES DIFERENTES. O");
        puts("  catalogo tem o corpo (norma 1-s^2, involucao p->1-p, o replicador); a teoria");
        puts("  tem as suas duas operacoes (clone e reproducao, sec:clonerep) e a sua regua");
        puts("  (Fisher, g = 1/(p(1-p))) — e nunca lhe chama evolutivo: `evolu` aparece ZERO");
        puts("  vezes em teoria.tex, com `corpo` a aparecer 108 como controlo.");
        puts("");
        puts("  A COSTURA E' UMA IDENTIDADE INTEIRA: N^2-(2k-N)^2 = 4k(N-k), donde");
        puts("  (1-s^2) . g(p) = 4 exacto. A NORMA E' O INVERSO DA REGUA.");
        puts("");
        puts("  E DAI SAI O PROCESSO: o ponto fixo da involucao p->1-p e' onde a regua custa");
        puts("  MENOS — a teoria ja' o derivara em p=1/2 sem dizer de que corpo era. Evoluir");
        puts("  e' afastar-se do ponto fixo, e por isso encarece nas pontas: e' a razao");
        puts("  medida do 'nao ha' vitoria gratis nos extremos' que o catalogo escrevera.");
        puts("");
        puts("  EVOLUCAO/INVOLUCAO E' TODO O PROCESSO: a involucao da' o centro, a reproducao");
        puts("  da' o sentido — e a reproducao so' sobe (Cauchy-Schwarz), com o empate SO'");
        puts("  no ponto fixo. Duas coordenadas, uma que mede e outra que ordena.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
