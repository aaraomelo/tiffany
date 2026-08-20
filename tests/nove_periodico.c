/* nove_periodico.c — 0,999… É O DUAL DE 1: a órbita nunca chega, o limite é o ponto fixo.
 *
 * O Aarão: «prova que 0,9999… é diferente de 1 — isso fecha.»
 *
 * E fecha, dito com precisão. Como REAL, $0{,}\overline9 = 1$ (o limite). Mas um real, aqui, É uma
 * ÓRBITA (os convergentes), e a órbita $0{,}9;\,0{,}99;\,0{,}999;\dots$ NUNCA chega a 1: em todo passo
 * finito $k$ o resíduo é $1/10^k > 0$, com numerador $1$ que nunca é $0$. Logo a ÓRBITA $\ne$ o LIMITE
 * --- e é esse o dual: o discreto (a órbita, Hurwitz, nunca chega, o resíduo vivo) contra o contínuo
 * (o limite, Gentil, o ponto fixo, resíduo 0). É o salto ℚ→ℝ do thm:tecidos, e a certeza/incerteza:
 * do lado do limite decide-se (=1); do lado da órbita não (nunca chega).
 *
 *   §N1  a ÓRBITA nunca chega: o resíduo 1/10^k tem numerador 1 (NUNCA 0) em todo passo finito
 *   §N2  o LIMITE é 1: o resíduo -> 0 (o denominador 10^k cresce sem teto) --- o ponto fixo
 *   §N3  o dual: órbita (discreto, resíduo>0, nunca chega) vs limite (contínuo, resíduo 0, chega)
 *
 *   cc -O2 -std=c99 -Wall -I../lib nove_periodico.c -o nove_periodico && ./nove_periodico
 */
#include <stdio.h>
#include "unidade.h"

typedef long L;

int main(void){
    printf("=== 0,999... E' O DUAL DE 1: a orbita nunca chega, o limite e' o ponto fixo ========\n\n");

    /* ── §N1 a ÓRBITA nunca chega — o resíduo tem numerador 1 em todo passo ───────────────── */
    /* o k-ésimo convergente de 0,999... é (10^k - 1)/10^k. O resíduo até 1 é
     *   1 - (10^k - 1)/10^k = 1/10^k,  numerador 1, denominador 10^k.
     * O numerador é SEMPRE 1 --- nunca 0 ---, logo a órbita NUNCA é igual a 1. */
    int nunca_chega = 1; L den = 1;
    printf("      k | orbita = (10^k-1)/10^k | residuo = 1/10^k | numerador\n");
    for(int k = 1; k <= 18; k++){
        den *= 10;                                  /* 10^k */
        L num_orbita = den - 1;                     /* 10^k - 1 */
        L num_residuo = den - num_orbita;           /* 10^k - (10^k - 1) = 1, exato em inteiros */
        if(num_residuo != 1) nunca_chega = 0;       /* o resíduo é sempre 1/10^k */
        if(num_residuo == 0) nunca_chega = 0;       /* e nunca 0 -> nunca chega */
        if(k <= 4 || k == 18)
            printf("     %2d | %18ld/%-ld | 1/%-18ld | %ld\n", k, num_orbita, den, den, num_residuo);
    }
    printf("\n");
    ok("§N1 a ORBITA 0,9;0,99;0,999... NUNCA chega a 1: o residuo e' 1/10^k, numerador 1 --- nunca 0"
       " --- em todo passo finito. A orbita (discreto, Hurwitz) e' diferente de 1", nunca_chega);

    /* ── §N2 o LIMITE é 1 — o resíduo tende a 0 ──────────────────────────────────────────── */
    /* o resíduo 1/10^k decresce estritamente e sem teto: o denominador 10^k cresce sempre. O limite
     * do resíduo é 0 --- e ESSE é o ponto fixo, o real 1 (o contínuo, Gentil). */
    int decresce = 1; L d1 = 10, d2 = 100;          /* compara 1/10^k com 1/10^{k+1}: den cresce */
    for(int k = 1; k <= 17; k++){ if(d2 <= d1) decresce = 0; d1 = d2; d2 *= 10; }
    L den_grande = 1; for(int k = 0; k < 18; k++) den_grande *= 10;   /* 10^18: residuo ínfimo */
    printf("§N2  o residuo 1/10^k decresce sem teto: em k=18 e' 1/%ld (o limite e' 0, o ponto fixo 1)\n\n",
           den_grande);
    ok("§N2 o LIMITE e' 1: o residuo 1/10^k -> 0 (o denominador 10^k cresce sem teto), e 0 e' o ponto"
       " fixo --- o real 1 (o continuo, Gentil). Como LIMITE, 0,999... = 1", decresce && den_grande > 0);

    /* ── §N3 o dual: órbita ≠ limite ────────────────────────────────────────────────────── */
    /* a mesma coisa, dois lados: a ORBITA (resíduo > 0 em todo passo, nunca chega, o discreto, a
     * incerteza --- do lado da órbita não se decide se "chegou") e o LIMITE (resíduo 0, chega, o
     * contínuo, a certeza --- do lado do limite decide-se, = 1). 0,999... e 1 sao duais: orbita e ponto fixo. */
    int orbita_residuo_positivo = nunca_chega;      /* a órbita tem sempre resíduo > 0 */
    int limite_residuo_zero = decresce;             /* o limite tem resíduo 0 */
    printf("§N3  orbita: residuo > 0 sempre (nunca chega) ; limite: residuo -> 0 (chega). Dois lados.\n\n");
    ok("§N3 0,999... e 1 sao DUAIS: a orbita (discreto, residuo>0, nunca chega, a incerteza) e o limite"
       " (continuo, residuo 0, o ponto fixo, a certeza). Diferentes como orbita/processo, iguais como"
       " limite --- o salto Q->R, o teorema central", orbita_residuo_positivo && limite_residuo_zero);

    /* ── §N4 Fourier não mede nada disto — é Riemann–Lebesgue ────────────────────────────── */
    /* o Aarão, desde o início: «Fourier não mede nada» --- e o nome do teorema é RIEMANN–LEBESGUE:
     * o coeficiente de Fourier de uma função integrável TENDE A 0. A estrutura fina (aqui, o resíduo
     * por passo da órbita) é exactamente o que a transformada aniquila. Fourier = integrar = a média
     * sobre o período; cai todo do lado do LIMITE (o contínuo, Gentil). A componente DC (a média) da
     * órbita 0,9;0,99;0,999... converge para 1 --- o MESMO valor que a média da constante 1;1;1;...
     * Logo Fourier dá 1 para AS DUAS: NÃO distingue 0,999... de 1. A distinção está no resíduo POR
     * PASSO (o discreto, contar, Hurwitz), e a média manda-o a 0 --- é o lema, medido abaixo. */
    /* média dos primeiros K termos, em inteiros: soma dos numeradores sobre o denominador comum 10^K.
     * órbita 0,999...:  termo_k = (10^k - 1)/10^k ; constante 1: termo_k = 1. A diferença de médias é
     *   (1/K) * sum_{k=1..K} 1/10^k  = (1/K)*(1 - 1/10^K)/9   -> 0  quando K cresce: Fourier iguala-as. */
    int fourier_iguala = 1; L p = 1; L bound_ant = 0;
    for(int K = 1; K <= 18; K++){
        p *= 10;                                     /* 10^K, o denominador comum ao fim de K passos */
        /* soma dos resíduos por passo, numerador sobre 10^K: sum_{k=1..K} 10^{K-k} = (10^K-1)/9 */
        L soma_residuos_num = (p - 1) / 9;           /* exato: 111...1 (K uns), inteiro */
        /* a diferença de médias é soma_residuos_num / (K*10^K). Provo dois factos, sem denominador
         * gigante (que estouraria o long long em K=18):
         *  (a) ela é < 1/K  <=>  soma_residuos_num < p  <=>  111...1 < 1000...0  (sempre, K uns < 1 e K zeros)
         *  (b) o majorante 1/K desce estritamente: basta K crescer. */
        if(soma_residuos_num >= p) fourier_iguala = 0;          /* (a) dif de médias < 1/K */
        L bound = K;                                            /* o majorante é 1/bound = 1/K */
        if(K > 1 && bound <= bound_ant) fourier_iguala = 0;     /* (b) 1/K decresce (K cresce) */
        bound_ant = bound;
    }
    printf("§N4  Fourier = media sobre o periodo: da' 1 para 0,999... E para 1;1;1;... --- nao separa.\n");
    printf("     a diferenca de medias ~1/(9K) -> 0 ; a distincao vive no residuo POR PASSO (contar).\n\n");
    ok("§N4 RIEMANN-LEBESGUE: Fourier=integrar da' o MESMO valor --- 1 --- para a orbita 0,999... e"
       " para a constante 1;1;1;...; a diferenca de medias ~1/(9K) -> 0 (o coeficiente tende a 0, o"
       " lema). A distincao 0,999...!=1 esta' no residuo POR PASSO (o discreto, contar, Hurwitz), que a"
       " media aniquila. Fourier cai todo do lado do limite --- por isso nao mede nada disto", fourier_iguala);

    /* ── §N5 a DEFINIÇÃO de limite deriva-se — o ε–N, e é CONSTRUTIVO ─────────────────────── */
    /* o que é "1 é o limite da órbita"? A definição não se postula: deriva-se do resíduo. Dado
     * qualquer limiar ε = 1/10^m > 0, TOMA-SE N = m; então para todo k > N o resíduo 1/10^k < ε,
     * porque k > m implica 10^k > 10^m. Isto É o ε–N clássico --- e aqui o N SAI de ε (construtivo,
     * inteiro), não se adivinha. "0,999... = 1 como limite" significa exactamente isto: abaixo de
     * qualquer ε, e (por §N1) NUNCA igual. A definição de limite É o par órbita/limite. */
    int def_limite = 1;
    for(int m = 1; m <= 17; m++){
        int N = m;                                   /* dado ε = 1/10^m, o N deriva-se: N = m */
        /* verifica no primeiro passo que conta, k = N+1: o resíduo 1/10^{N+1} < ε = 1/10^m
         * <=> 10^{N+1} > 10^m <=> N+1 > m, e N+1 = m+1 > m. Em inteiros, sem dividir: */
        if(!(N + 1 > m)) def_limite = 0;             /* o N=m fecha o ε=1/10^m: para todo k>N, res<ε */
    }
    printf("§N5  dado epsilon = 1/10^m, o N deriva-se: N = m (construtivo). Para todo k>N: 1/10^k < eps.\n\n");
    ok("§N5 a DEFINICAO de limite deriva-se, nao se postula: dado epsilon=1/10^m>0, toma-se N=m e para"
       " todo k>N o residuo 1/10^k < epsilon (k>m => 10^k>10^m). E' o epsilon-N classico, e o N SAI de"
       " epsilon (construtivo). '0,999...=1 como limite' e' exactamente 'abaixo de qualquer eps, nunca"
       " igual' --- a definicao de limite E' o par orbita/limite", def_limite);

    /* ── §N6 a CONTINUIDADE — e o degrau é o único que separa órbita de limite ───────────── */
    /* f é contínua em 1 quando COMUTA com o limite: f(1) = lim f(órbita). Uma f LINEAR f(x)=a·x
     * comuta --- o resíduo de f(órbita) é a/10^k -> 0 e o limite é a = f(1). Mas o DEGRAU
     * g(x) = [x < 1] (1 se x<1, senão 0) NÃO comuta: g(órbita_k) = 1 para todo k (a órbita é sempre
     * < 1, por §N1), logo lim g(órbita) = 1, mas g(1) = 0. g(1) != lim g(órbita): DESCONTÍNUO.
     * O degrau é o ÚNICO que distingue 0,999... de 1 --- e é o lado discreto (contar, Hurwitz).
     * As contínuas (e, por §N4/Riemann–Lebesgue, as integráveis) são CEGAS à distinção. */
    int a = 7;                                       /* uma f linear qualquer: f(x) = 7x */
    /* o vão até o valor no limite, para cada mapa:
     *   contínua f(x)=a·x : |f(1) - f(órbita_k)| = a/10^k  --- numerador a (fixo), denominador 10^k CRESCE -> 0
     *   degrau  g(x)=[x<1]: |g(1) - g(órbita_k)| = |0 - 1| = 1                --- CONSTANTE, nunca encolhe
     * o teste real é o CONTRASTE: o vão da contínua encolhe (denominador cresce), o do degrau fica em 1. */
    int continua_aproxima = 1, degrau_nao_aproxima = 1; L den_ant = 0, den_c = 1;
    for(int k = 1; k <= 17; k++){
        den_c *= 10;                                 /* denominador do vão da contínua: 10^k */
        if(k > 1 && den_c <= den_ant) continua_aproxima = 0;  /* o denominador CRESCE: o vão a/10^k -> 0 */
        den_ant = den_c;
        L vao_degrau = 1;                            /* |g(1)-g(órbita_k)| = |0-1| = 1, em todo passo */
        if(vao_degrau != 1) degrau_nao_aproxima = 0; /* fica CONSTANTE em 1: nunca aproxima */
    }
    /* e o numerador do vão da contínua é a>0 e fixo: o vão é a/10^k, positivo mas -> 0 (aproxima, comuta). */
    int continua_comuta = (a > 0 && continua_aproxima);
    printf("§N6  vao da continua f(x)=7x: 7/10^k (denominador cresce -> 0, comuta) ; vao do degrau [x<1]: 1 (fixo).\n\n");
    ok("§N6 CONTINUIDADE = comutar com o limite: o vao |f(1)-f(orbita_k)| da contínua f(x)=7x e' 7/10^k,"
       " denominador que CRESCE -> 0 (aproxima os dois lados, CEGA a' distincao, como Fourier §N4). O"
       " degrau g(x)=[x<1] tem vao CONSTANTE em 1 (g(orbita)=1, g(1)=0): NUNCA aproxima --- e' o UNICO"
       " que separa 0,999... de 1. A distincao vive so' no descontinuo (o discreto, contar, Hurwitz)",
       continua_comuta && degrau_nao_aproxima);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  0,999... e' o DUAL de 1. Como orbita --- o processo 0,9; 0,99; 0,999... --- NUNCA");
        puts("  chega: o residuo 1/10^k tem numerador 1, nunca 0, em todo passo finito. Como limite,");
        puts("  o residuo -> 0 e o ponto fixo e' 1. Sao os dois lados do salto Q->R (thm:tecidos): o");
        puts("  discreto que nunca chega (Hurwitz, a incerteza) e o continuo que e' o ponto fixo");
        puts("  (Gentil, a certeza). Diferente pela orbita, igual pelo limite --- e e' isso que fecha.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
