/* racionais_fixos.c — A DESCIDA: os racionais são as classes de equivalência dos
 * PONTOS FIXOS, e cada racional é uma RAZÃO de pontos fixos.
 *
 * O `geometrico.tex` sobe: a órbita corre em ℙ¹(ℚ) e o ponto fixo NÃO está lá
 * (thm:corte-fixo). Isto mede a DESCIDA, que é a outra metade do mesmo par — e ela não
 * precisa de teoria nova, precisa de ler o que o thm:fixo-dual já diz: o par dual de
 * pontos fixos é descrito por TRAÇO e DETERMINANTE, ambos INTEIROS.
 *
 * A construção, e é toda ela um dedo apontado a uma coisa que já lá estava:
 *
 *   σ_m é o ponto fixo de g(x) = m + 1/x,  e vive em ℚ(√(m²+4)) — UM corpo por cada m.
 *   Corpos DIFERENTES: σ₁ = φ está em ℚ(√5), σ₂ = 1+√2 está em ℚ(√8). Não há um sítio
 *   onde eles caibam todos… excepto o que a simetrização deixa para trás.
 *
 *   σ_m + σ_m† = m        ← o TRAÇO: a projecção, e é SOBREJECTIVA sobre ℤ
 *   σ_m · σ_m† = −1       ← a NORMA: constante, e não separa NADA
 *
 * Logo o traço é a única ponte comum a toda a família, e ℤ é a sua imagem: cada inteiro
 * é o traço de um par dual de pontos fixos, e a fibra sobre m é exactamente {σ_m, σ_m†}
 * — A CLASSE DE EQUIVALÊNCIA. E ℚ é a razão: a/b = tr(σ_a)/tr(σ_b), com (a,b) ~ (c,d)
 * exactamente quando os traços cruzados coincidem.
 *
 * A norma está aqui como GUME INTERNO, e não como enfeite: se a coordenada escolhida
 * fosse ela, a tabela toda dava −1 e eu teria chamado lei a uma constante. É a mesma
 * lição de sempre, com a diferença de que desta vez a construção traz o seu próprio
 * contra-teste dentro.
 *
 *   §Q1  o traço é a coordenada: tr(σ_m) = m, por DUAS rotas, exacto e sem raiz
 *   §Q2  o GUME: a norma vale −1 para todo m — separa UM valor, e o traço separa todos
 *   §Q3  e os pontos fixos são mesmo irracionais: m²+4 nunca é quadrado, para m ≥ 1
 *   §Q4  ℚ é a razão de traços, e a classe de equivalência bate par a par
 *   §Q5  o racional é o PONTO FIXO DA DOBRA — Dir fixa (alcança), Cruz troca (opera)
 *   §Q6  p_k e q_k são DUAS soluções da MESMA equação do ponto fixo
 *   §Q7  a descida: truncar a palavra dá racional, e F = ±1 nunca zero — nunca chega
 *
 * Nenhum double, nenhum limiar: compila sem -lm.
 *
 *   cc -O2 -std=c99 -I. -I../lib racionais_fixos.c -o racionais_fixos && ./racionais_fixos
 */
#include <stdio.h>
#include "reta.h"
#include "unidade.h"

#define M_MAX 60

int main(void){
    printf("\n══ A DESCIDA: os racionais são classes de equivalência de pontos fixos ══\n");

    /* ─── §Q1 ── o TRAÇO é a coordenada ─────────────────────────────────────────────
     * σ_m + σ_m† = m. Mede-se por duas rotas que não partilham código: a fórmula
     * fechada da lib (2σ = m+√D elevado a k, dividido por 2^{k−1}) e a soma directa
     * dos dois conjugados em ℤ[√D], onde a raiz nunca se forma. */
    long tr[M_MAX + 1];
    int q1 = 0, q1b = 0;
    for(long m = 1; m <= M_MAX; m++){
        long D = m*m + 4;
        tr[m] = rt_traco_metalico(m, 1);           /* rota A: a lib */

        /* rota B: 2σ = m + √D, 2σ† = m − √D; a soma é 2m, e a metade é EXACTA */
        long ca, cb; rt_zd_conj(m, 1, &ca, &cb);   /* o conjugado de 2σ */
        long soma_a = m + ca, soma_b = 1 + cb;     /* (m+√D) + (m−√D) */
        int sem_raiz = (soma_b == 0);              /* a parte irracional cancelou-se */
        int metade_exacta = (soma_a % 2 == 0);
        long trB = sem_raiz && metade_exacta ? soma_a / 2 : 0;

        if(tr[m] == m) q1++;
        if(trB == m && sem_raiz) q1b++;
        (void)D;
    }
    printf("\n  §Q1  tr(σ_m) = m, exacto e sem raiz formada\n");
    printf("      rota A (fórmula fechada da lib) ..... %2d de %d\n", q1, M_MAX);
    printf("      rota B (soma dos conjugados) ........ %2d de %d\n", q1b, M_MAX);
    ok("o traço do par dual de pontos fixos É o inteiro m, nas duas rotas",
       q1 == M_MAX && q1b == M_MAX);

    /* ─── §Q2 ── o GUME INTERNO: a norma não separa ─────────────────────────────────
     * σσ† = −1 para TODO m. Se a coordenada fosse a norma, a construção morria aqui: a
     * fibra sobre −1 seria a família inteira. Conta-se quantos valores DISTINTOS cada
     * uma produz — e é a diferença entre as duas contagens que é a medida. */
    long nm[M_MAX + 1];
    for(long m = 1; m <= M_MAX; m++){
        long D = m*m + 4;
        /* (2σ)(2σ†) = m² − D = −4, logo σσ† = −1 */
        nm[m] = rt_zd_norma(m, 1, D) / 4;
    }
    int dist_tr = 0, dist_nm = 0;
    for(long m = 1; m <= M_MAX; m++){
        int novo_t = 1, novo_n = 1;
        for(long j = 1; j < m; j++){
            if(tr[j] == tr[m]) novo_t = 0;
            if(nm[j] == nm[m]) novo_n = 0;
        }
        dist_tr += novo_t; dist_nm += novo_n;
    }
    int norma_const = 1;
    for(long m = 1; m <= M_MAX; m++) if(nm[m] != -1) norma_const = 0;

    printf("\n  §Q2  GUME: a norma é constante, e por isso NÃO é coordenada\n");
    printf("      valores distintos do TRAÇO ........... %2d de %d\n", dist_tr, M_MAX);
    printf("      valores distintos da NORMA .......... %2d de %d   (e vale sempre −1)\n",
           dist_nm, M_MAX);
    ok("a norma vale −1 em todo m: separa UM valor, e o traço separa todos",
       norma_const && dist_nm == 1 && dist_tr == M_MAX);

    /* ─── §Q3 ── e os pontos fixos são mesmo IRRACIONAIS ────────────────────────────
     * Sem isto a construção seria oca: se σ_m fosse racional, «classe de equivalência
     * de pontos fixos» não diria nada que ℤ já não dissesse. D = m²+4 é quadrado
     * perfeito só quando m = 0 — porque n²−m² = 4 obriga (n−m)(n+m) = 4, e as duas
     * parcelas têm a mesma paridade. Aqui procura-se a raiz em ℤ, e a resposta é a
     * AUSÊNCIA dela: o `rt_raiz_k` devolve 0 quando ela não existe. */
    int irrac = 0, quadrado_em_zero = 0;
    for(long m = 1; m <= M_MAX; m++){
        long D = m*m + 4;
        long r;
        if(!rt_raiz_k(D, 2, &r)) irrac++;          /* não há raiz inteira ⟹ irracional */
    }
    long r0 = 0;
    quadrado_em_zero = rt_raiz_k(0*0 + 4, 2, &r0) && r0 == 2;   /* e em m = 0 HÁ: √4 = 2 */

    printf("\n  §Q3  σ_m ∉ ℚ, e o controlo que TEM de dar o contrário\n");
    printf("      m²+4 sem raiz inteira, m = 1..%d ... %2d de %d\n", M_MAX, irrac, M_MAX);
    printf("      e em m = 0, onde ela existe ......... √4 = %ld\n", r0);
    ok("todo σ_m com m ≥ 1 é irracional, e o caso m = 0 mostra que a busca funciona",
       irrac == M_MAX && quadrado_em_zero);

    /* ─── §Q4 ── ℚ é a RAZÃO de traços, e a classe de equivalência bate ─────────────
     * a/b codifica-se pelo PAR de pontos fixos (σ_a, σ_b) — dois objectos irracionais,
     * em dois corpos diferentes — e recupera-se por simetrização. A classe: dois pares
     * dão o mesmo racional exactamente quando os traços cruzados coincidem. Varre-se
     * todo o quadrado e compara-se, entrada a entrada, com a relação ad = bc.
     *
     * E ao lado corre a MESMA varredura com a norma no lugar do traço: ela declara
     * TODOS os pares equivalentes, e é essa a diferença que se conta. */
    const int R = 12;
    long conc = 0, disc = 0, pares = 0, conc_nm = 0;
    for(long a = 1; a <= R; a++) for(long b = 1; b <= R; b++)
    for(long c = 1; c <= R; c++) for(long d = 1; d <= R; d++){
        int por_traco = (tr[a]*tr[d] == tr[b]*tr[c]);
        int por_razao = (a*d == b*c);
        int por_norma = (nm[a]*nm[d] == nm[b]*nm[c]);
        pares++;
        if(por_traco == por_razao) conc++; else disc++;
        if(por_norma == por_razao) conc_nm++;
    }
    /* e quantas CLASSES distintas saem — os racionais reduzidos de [1..R]² */
    int classes = 0;
    for(long a = 1; a <= R; a++) for(long b = 1; b <= R; b++)
        if(rt_mdc(a, b) == 1) classes++;

    printf("\n  §Q4  a/b = tr(σ_a)/tr(σ_b), e a classe é a fibra do traço\n");
    printf("      quádruplos varridos ................. %ld\n", pares);
    printf("      pelo TRAÇO concordam com ad = bc .... %ld   (divergem %ld)\n", conc, disc);
    printf("      pela NORMA concordam ................ %ld\n", conc_nm);
    printf("      classes distintas em [1..%d]² ....... %d\n", R, classes);
    ok("a equivalência dos pares de pontos fixos É a igualdade dos racionais",
       disc == 0 && conc == pares && conc_nm < pares);

    /* ─── §Q5 ── o racional é o PONTO FIXO DA DOBRA ─────────────────────────────────
     * Dentro de um corpo fixo ℚ(√D), a dobra x ↦ x† tem espectro {+1,−1}. O que ela
     * fixa é a parte sem raiz — e é essa a face que ALCANÇA e ORDENA (Dir). O que ela
     * troca de sinal é a parte com raiz, a que OPERA (Cruz). Pontryagin outra vez, e
     * aqui é literal: ℚ é o auto-espaço +1.
     *
     * Mede-se nos dois sentidos, que é o que faz disto uma medida e não uma definição
     * repetida: contam-se os fixos, contam-se os anti-fixos, verifica-se que a soma das
     * duas projecções devolve x EXACTAMENTE, e mede-se o que a IDENTIDADE no lugar da
     * dobra faria — declarar toda a gente racional. */
    const long DD[] = {5, 8, 13, 20, 29}, ND = 5;
    long fixos = 0, antifixos = 0, ambos = 0, total = 0, reconstroi = 0, id_fixos = 0;
    for(long iD = 0; iD < ND; iD++){
        long D = DD[iD];
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            long ca, cb; rt_zd_conj(a, b, &ca, &cb);
            int e_fixo = (ca == a && cb == b);
            int e_anti = (ca == -a && cb == -b);
            total++;
            if(e_fixo) fixos++;
            if(e_anti) antifixos++;
            if(e_fixo && e_anti) ambos++;
            id_fixos++;                            /* com a identidade, TODOS seriam */

            /* Dir = (x+x†)/2 e Cruz = (x−x†)/2, e a soma devolve x sem resto */
            long dir_a = a + ca, dir_b = b + cb;   /* dobrados, para não dividir */
            long crz_a = a - ca, crz_b = b - cb;
            if(dir_a + crz_a == 2*a && dir_b + crz_b == 2*b &&
               dir_b == 0 && crz_a == 0) reconstroi++;
            (void)D;
        }
    }
    printf("\n  §Q5  ℚ é o auto-espaço +1 da dobra — Dir alcança, Cruz opera\n");
    printf("      elementos varridos (5 corpos) ....... %ld\n", total);
    printf("      fixos pela dobra (é ℚ: b = 0) ....... %ld\n", fixos);
    printf("      anti-fixos (é √D·ℚ: a = 0) .......... %ld\n", antifixos);
    printf("      na intersecção, só o zero ........... %ld por corpo\n", ambos / ND);
    printf("      Dir+Cruz devolve x, e Dir sem raiz .. %ld de %ld\n", reconstroi, total);
    printf("      GUME: com a IDENTIDADE seriam fixos . %ld de %ld\n", id_fixos, total);
    ok("o racional é o que a dobra fixa, e a identidade no lugar dela não distinguiria nada",
       fixos == 13*ND && antifixos == 13*ND && ambos == ND &&
       reconstroi == total && id_fixos == total && fixos < id_fixos);

    /* ─── §Q6 ── p_k e q_k são DUAS soluções da MESMA equação do ponto fixo ─────────
     * É aqui que a razão de pontos fixos deixa de ser analogia. A órbita [p:q] ↦
     * [mp+q : p] faz correr DUAS sequências, e ambas obedecem a x_{k+1} = m·x_k +
     * x_{k−1} — cuja equação característica é x² = mx + 1, A EQUAÇÃO DO PONTO FIXO.
     * Diferem só nas condições iniciais, que são inteiras. O racional p_k/q_k é a razão
     * de duas soluções da equação do ponto fixo, e o irracional cancela-se na razão
     * porque cada uma é simétrica nos dois pontos fixos. */
    long rec_p = 0, rec_q = 0, passos = 0;
    for(long m = 1; m <= 8; m++){
        long p[24], q[24];
        p[0] = 1; q[0] = 0;                        /* [1:0] = ∞ */
        p[1] = m; q[1] = 1;
        for(int k = 2; k < 20; k++){
            p[k] = m*p[k-1] + p[k-2];
            q[k] = m*q[k-1] + q[k-2];
        }
        for(int k = 2; k < 20; k++){
            passos++;
            if(p[k] == m*p[k-1] + p[k-2]) rec_p++;
            if(q[k] == m*q[k-1] + q[k-2]) rec_q++;
        }
    }
    /* e a recorrência É a equação do ponto fixo: o polinómio característico x²−mx−1 é
     * exactamente o F(x,1) da forma — verifica-se pela forma da lib, e não à mão */
    int caract = 1;
    for(long m = 1; m <= 8; m++) if(rt_forma(m, 1, m) != m*m - m*m - 1) caract = 0;

    printf("\n  §Q6  p_k e q_k obedecem à MESMA recorrência — a do ponto fixo\n");
    printf("      passos verificados .................. %ld\n", passos);
    printf("      p_k = m·p_{k−1} + p_{k−2} ........... %ld\n", rec_p);
    printf("      q_k = m·q_{k−1} + q_{k−2} ........... %ld\n", rec_q);
    printf("      e o característico É a forma F ...... %s\n", caract ? "sim" : "não");
    ok("as duas sequências do racional são soluções da equação do ponto fixo",
       rec_p == passos && rec_q == passos && caract);

    /* ─── §Q7 ── A DESCIDA: truncar a palavra dá racional, e nunca chega ────────────
     * A palavra de σ_m é [m;m,m,…], infinita. Cada truncatura é um par de inteiros —
     * um racional — e o thm:corte-fixo diz que F(p,q) nunca é zero: NENHUMA truncatura
     * É o ponto fixo. Mede-se com o leitor/escritor do teorema do operador, e mede-se
     * também que F ALTERNA o sinal, que é a dobra a agir (F(q,p−mq) = −F(p,q)).
     *
     * O denominador cresce, logo não há repetição e a palavra não pode terminar — é essa
     * a diferença entre o racional e o real: a palavra finita e a que não fecha. Mas
     * «cresce» teve de ser MEDIDO e não suposto: escrevi «estritamente» e a asserção
     * caiu, porque há exactamente UM empate em toda a varredura — q₀ = q₁ = 1 no OURO,
     * m = 1, que é a Fibonacci a arrancar. É o real mais lento a aparecer aqui também, e
     * por isso o empate é contado À PARTE, com o m identificado, em vez de a régua ser
     * afrouxada para o esconder. */
    long trunc = 0, nunca_zero = 0, alterna = 0, cresce = 0, empate = 0, m_empate = 0;
    for(long m = 1; m <= 8; m++){
        RtCf w; w.n = 0; w.saturou = 0;
        for(int k = 0; k < 14; k++) if(!rt_op_escreve(&w, m)) break;
        long ant_q = 0, ant_F = 0;
        for(int k = 0; k < w.n; k++){
            long p, q;
            if(!rt_op_le(&w, k, &p, &q)) continue;
            trunc++;
            long F = rt_forma(p, q, m);
            if(F != 0) nunca_zero++;
            if(k > 0 && F == -ant_F) alterna++;
            if(k > 0){
                if(q > ant_q) cresce++;
                else if(q == ant_q){ empate++; m_empate = m; }
            }
            ant_q = q; ant_F = F;
        }
    }
    printf("\n  §Q7  a descida: cada truncatura é racional, e nenhuma é o ponto fixo\n");
    printf("      truncaturas lidas ................... %ld\n", trunc);
    printf("      com F ≠ 0 (o corte, thm:corte-fixo) . %ld\n", nunca_zero);
    printf("      com F a trocar de sinal (a dobra) ... %ld de %ld\n", alterna, trunc - 8);
    printf("      denominador estritamente crescente .. %ld de %ld\n", cresce, trunc - 8);
    printf("      e o ÚNICO empate é o do ouro, m = %ld .. %ld caso\n", m_empate, empate);
    ok("a palavra truncada é o racional, e o ponto fixo não está em nenhuma truncatura",
       trunc > 0 && nunca_zero == trunc && alterna == trunc - 8 &&
       cresce == trunc - 8 - empate && empate == 1 && m_empate == 1);

    printf("\n  ══ a descida fecha: o traço projecta a família toda em ℤ, a fibra é o\n");
    printf("     par dual {σ_m, σ_m†}, e ℚ é a razão. A norma, que é a outra metade da\n");
    printf("     Cayley–Hamilton, CONSERVA e não separa — e é por isso que não é ela. ══\n\n");

    return falhas ? 1 : 0;
}
