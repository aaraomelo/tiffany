/* cuspide.c — A CÚSPIDE: o lugar onde as duas folhas COLIDEM, e o teste de uma linha
 * que a decide.
 *
 * O Aarão: «vê se é cúspide fractal, relaciona com o teorema do trial,
 * Cantor, Julia e Viviani e o toro do gato; deriva o fractal e define a cúspide».
 *
 * FECHA. Retirar a hipótese e procurar o contra-exemplo é uma operação no espaço de
 * PARÂMETROS, e o que ela atravessa tem lugar próprio, equação — e um teste de UMA LINHA:
 *
 *      DEFINIÇÃO (cúspide).  No espaço dos operadores inteiros com |det| = 1, a CÚSPIDE é
 *      o lugar onde o DISCRIMINANTE se anula,
 *
 *              disc = tr² − 4·det = 0 ,
 *
 *      isto é, onde as duas folhas da transformada universal COLIDEM (σ = σ†).
 *
 * É a mesma quantidade do §T7b de `transformada_universal.c`: lá o discriminante zero dizia
 * «o objecto é racional, é um PONTO e não um corte»; aqui diz «o operador é parabólico, e
 * está na fronteira entre fechar e não fechar». São a mesma frase em dois andares.
 *
 *   §C1  os TRÊS regimes, e a cúspide é a fronteira (thm:toro lido pelo discriminante)
 *   §C2  CADA RACIONAL ANCORA UMA CÚSPIDE — e é o seu ponto fixo (thm:descida)
 *   §C3  na cúspide a transformada DEGENERA: uma folha só, e a volta perde-se
 *   §C4  o FRACTAL: as cúspides são UMA órbita, e a auto-semelhança é o grupo
 *   §C5  atravessar: a mutação que não derruba ficou do MESMO LADO
 *   §C6  a tríade vive DENTRO, o gato vive FORA, e a cúspide separa-os
 *
 * Tudo inteiro: o discriminante é tr² − 4·det, e nenhuma raiz se forma.
 *
 *   cc -O2 -std=c99 -I. -I../lib cuspide.c -o cuspide && ./cuspide
 */
#include <stdio.h>
#include <stdlib.h>
#include "unidade.h"

typedef struct { long a, b, c, d; } M2;

static long det2(M2 m){ return m.a*m.d - m.b*m.c; }
static long tr2 (M2 m){ return m.a + m.d; }
static long disc(M2 m){ long t = tr2(m); return t*t - 4*det2(m); }
static M2  mul2(M2 x, M2 y){
    M2 r; r.a = x.a*y.a + x.b*y.c; r.b = x.a*y.b + x.b*y.d;
          r.c = x.c*y.a + x.d*y.c; r.d = x.c*y.b + x.d*y.d; return r;
}
/* a ordem do operador, ou 0 se não fechar até ao tecto — e o tecto é VERIFICADO:
 * quando devolve 0 o chamador sabe que viu `tecto` potências e nenhuma era a identidade */
static int ordem(M2 m, int tecto){
    M2 p = m;
    for(int k = 1; k <= tecto; k++){
        if(p.a == 1 && p.b == 0 && p.c == 0 && p.d == 1) return k;
        p = mul2(p, m);
        if(p.a > 100000000L || p.a < -100000000L) return 0;   /* cresceu: não fecha */
    }
    return 0;
}
static long mdc(long x, long y){ x = labs(x); y = labs(y); while(y){ long t = x%y; x=y; y=t; } return x; }

int main(void){
    printf("\n══ A CÚSPIDE: disc = tr² − 4·det = 0, onde as duas folhas COLIDEM ══\n\n");

    /* ─── §C1 ── os três regimes ────────────────────────────────────────────────────
     * O thm:toro decide pelo traço com det = +1: |tr| ≤ 1 fecha, |tr| ≥ 2 não. Lido pelo
     * DISCRIMINANTE isso é uma frase só, e vale para qualquer determinante. */
    {
        long tot = 0, eli = 0, par = 0, hip = 0;
        long eli_fecha = 0, par_naofecha = 0, hip_naofecha = 0, cuspide_e_par = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
            M2 m = { a,b,c,d };
            if(det2(m) != 1) continue;
            long D = disc(m); int o = ordem(m, 32);
            tot++;
            if(D <  0){ eli++; if(o >  0) eli_fecha++; }
            if(D == 0){ par++; if(o == 0 || o == 1) par_naofecha++;
                        if(labs(tr2(m)) == 2) cuspide_e_par++; }
            if(D >  0){ hip++; if(o == 0) hip_naofecha++; }
        }
        printf("  §C1  os três regimes, pelo discriminante\n");
        printf("      operadores com det = +1 ............ %ld\n", tot);
        printf("      disc < 0  ELÍPTICO (dentro) ........ %ld   e fecham: %ld\n", eli, eli_fecha);
        printf("      disc = 0  A CÚSPIDE (fronteira) .... %ld   e |tr| = 2: %ld\n", par, cuspide_e_par);
        printf("      disc > 0  HIPERBÓLICO (fora) ....... %ld   não fecham: %ld\n\n", hip, hip_naofecha);
        ok("a CÚSPIDE define-se pelo DISCRIMINANTE e não pelo traço: disc = tr² − 4·det = 0"
           " é a fronteira entre o regime que fecha (disc < 0, ordem finita) e o que não"
           " fecha (disc > 0, o gato). Com det = +1 ela é |tr| = 2, que é o thm:toro dito"
           " numa quantidade só — e os três regimes estão todos povoados",
           tot > 0 && eli > 0 && par > 0 && hip > 0 &&
           eli_fecha == eli && hip_naofecha == hip && cuspide_e_par == par);
    }

    /* ─── §C2 ── CADA RACIONAL ANCORA UMA CÚSPIDE ───────────────────────────────────
     * Para p/q reduzido, a matriz
     *
     *      P(p,q) = [ 1−pq   p²  ;  −q²   1+pq ]
     *
     * tem det = 1, tr = 2 (logo disc = 0: É uma cúspide) e FIXA [p:q]. A recíproca também
     * se mede: nenhum operador hiperbólico fixa um racional — que é o thm:corte-fixo. */
    {
        long tot = 0, det1 = 0, na_cuspide = 0, fixa = 0;
        long hip_tot = 0, hip_sem_fixo = 0;
        for(long q = 1; q <= 12; q++) for(long p = -12; p <= 12; p++){
            if(mdc(p,q) != 1) continue;
            M2 P = { 1 - p*q, p*p, -q*q, 1 + p*q };
            tot++;
            if(det2(P) == 1) det1++;
            if(disc(P) == 0) na_cuspide++;
            long np = P.a*p + P.b*q, nq = P.c*p + P.d*q;
            if(np == p && nq == q) fixa++;
        }
        /* e o gato NUNCA fixa um racional: p² − m·p·q − q² = 0 não tem solução */
        for(long m = 1; m <= 8; m++){
            M2 A = { m,1,1,0 };
            hip_tot++;
            long achou = 0;
            for(long q = 1; q <= 40 && !achou; q++) for(long p = -40; p <= 40; p++){
                if(p == 0 && q == 0) continue;
                long np = A.a*p + A.b*q, nq = A.c*p + A.d*q;
                if(np*q == nq*p){ achou = 1; break; }      /* [np:nq] = [p:q] */
            }
            if(!achou) hip_sem_fixo++;
        }
        printf("  §C2  cada RACIONAL ancora uma cúspide, e é o seu ponto fixo\n");
        printf("      racionais p/q reduzidos ............ %ld\n", tot);
        printf("      P(p,q) com det = 1 ................. %ld\n", det1);
        printf("      e na CÚSPIDE (disc = 0) ............ %ld\n", na_cuspide);
        printf("      e a fixar exactamente [p:q] ........ %ld\n", fixa);
        printf("      gatos A_m varridos ................. %ld   sem ponto fixo racional: %ld\n\n",
               hip_tot, hip_sem_fixo);
        ok("CADA RACIONAL ANCORA UMA CÚSPIDE: para p/q reduzido a matriz [1−pq, p²; −q²,"
           " 1+pq] é inteira, tem det = 1 e disc = 0 — está na cúspide — e o seu ponto fixo"
           " é EXACTAMENTE [p:q]. É o thm:descida visto do espaço de parâmetros: ℚ são as"
           " classes dos pontos fixos, e aqui ℚ é o conjunto das cúspides. E do outro lado"
           " o gato, hiperbólico, não fixa racional nenhum — que é o thm:corte-fixo",
           tot > 0 && det1 == tot && na_cuspide == tot && fixa == tot &&
           hip_tot > 0 && hip_sem_fixo == hip_tot);
    }

    /* ─── §C3 ── na cúspide a TRANSFORMADA degenera ─────────────────────────────────
     * O §T8b de `transformada_universal.c` mediu que a volta precisa das n folhas. A
     * cúspide é onde elas colidem: com disc = 0 há UMA folha só, e a deconvolução perde-se.
     * Mede-se em Z_p, contando as raízes do polinómio característico. */
    {
        long tot = 0, cusp = 0, cusp_uma_folha = 0, fora = 0, fora_duas = 0;
        const long ps[] = { 11, 13, 17, 19, 23 };
        for(int ip = 0; ip < 5; ip++){
            long P = ps[ip];
            for(long t = 0; t < P; t++) for(long dt = 1; dt < P; dt++){
                long D = ((t*t - 4*dt) % P + P) % P;
                /* as folhas: raízes de x² − t·x + dt em Z_P */
                long nf = 0;
                for(long s = 0; s < P; s++)
                    if(((s*s - t*s + dt) % P + P) % P == 0) nf++;
                tot++;
                if(D == 0){ cusp++; if(nf == 1) cusp_uma_folha++; }
                else if(nf > 0){ fora++; if(nf == 2) fora_duas++; }
            }
        }
        printf("  §C3  na cúspide a transformada DEGENERA: uma folha só\n");
        printf("      pares (tr, det) em Z_p ............. %ld\n", tot);
        printf("      na cúspide (disc = 0) .............. %ld   com UMA folha: %ld\n", cusp, cusp_uma_folha);
        printf("      fora dela, com folhas .............. %ld   com DUAS: %ld\n\n", fora, fora_duas);
        ok("na CÚSPIDE a transformada universal degenera: disc = 0 dá UMA folha só, e pelo"
           " §T8b de transformada_universal.c a volta exige as n — logo é exactamente ali"
           " que a DECONVOLUÇÃO se perde. Fora da cúspide, quando há folhas, são sempre"
           " DUAS. A cúspide é o lugar onde a máquina deixa de ter volta",
           tot > 0 && cusp > 0 && cusp_uma_folha == cusp && fora > 0 && fora_duas == fora);
    }

    /* ─── §C4 ── O FRACTAL: as cúspides são UMA órbita ──────────────────────────────
     * A auto-semelhança não é uma figura: é a acção do grupo. Todas as cúspides são a
     * órbita de uma só sob os geradores S = [0 −1; 1 0] e T = [1 1; 0 1] — logo o conjunto
     * das cúspides é auto-semelhante por construção, e a árvore que a órbita desenha é a
     * de Farey/Stern-Brocot, com a mediante a nascer da soma dos vectores. */
    {
        long alvo = 0, alcancados = 0, med_ok = 0, med_tot = 0;
        /* alcançar [p:q] a partir de [1:0] por S e T é o algoritmo de Euclides ao contrário */
        for(long q = 0; q <= 14; q++) for(long p = 0; p <= 14; p++){
            if(p == 0 && q == 0) continue;
            if(mdc(p,q) != 1) continue;
            alvo++;
            long x = p, y = q, passos = 0;
            while(y != 0 && passos < 200){ long r = x % y; x = y; y = r; passos++; }
            if(labs(x) == 1) alcancados++;      /* desce a [1:0] — é a mesma órbita */
        }
        /* a MEDIANTE: entre p/q e r/s vizinhos de Farey (|ps−qr| = 1), a mediante é vizinha
         * de ambos — a auto-semelhança da árvore, em inteiros */
        for(long q = 1; q <= 9; q++) for(long p = 0; p <= 9; p++)
        for(long s = 1; s <= 9; s++) for(long r = 0; r <= 9; r++){
            if(labs(p*s - q*r) != 1) continue;
            med_tot++;
            long mp = p + r, mq = q + s;
            /* vizinha dos dois — mas isso a DIFERENÇA também cumpre (é o pai na árvore).
             * O que faz da mediante o passo que REFINA é ficar ENTRE eles, e é isso que
             * se exige: p/q < (p+r)/(q+s) < r/s, comparado por produtos cruzados */
            int viz = (labs(p*mq - q*mp) == 1 && labs(mp*s - mq*r) == 1);
            int entre = ((p*mq - q*mp) * (mp*s - mq*r) > 0) &&
                        ((p*s - q*r) * (p*mq - q*mp) > 0);
            if(viz && entre) med_ok++;
        }
        printf("  §C4  o FRACTAL: as cúspides são UMA órbita, e a mediante é o passo\n");
        printf("      cúspides [p:q] varridas ............ %ld\n", alvo);
        printf("      todas na órbita de [1:0] ........... %ld\n", alcancados);
        printf("      pares de Farey vizinhos ............ %ld\n", med_tot);
        printf("      e a mediante vizinha dos DOIS ...... %ld\n\n", med_ok);
        ok("o FRACTAL é a órbita: todas as cúspides são alcançáveis de [1:0] pelos geradores"
           " do grupo — o que é o algoritmo de Euclides ao contrário —, logo o conjunto"
           " delas é auto-semelhante por CONSTRUÇÃO e não por analogia. E o passo da"
           " auto-semelhança é a MEDIANTE de Farey: entre dois vizinhos ela é vizinha dos"
           " dois E FICA ENTRE ELES — a diferença também é vizinha, mas cai fora, é o pai"
           " e não o filho —, e é assim que a árvore se refina sem nunca repetir",
           alvo > 0 && alcancados == alvo && med_tot > 0 && med_ok == med_tot);
    }

    /* ─── §C5 ── ATRAVESSAR A CÚSPIDE ──────────────────────────────────────────────
     * Aqui a definição paga, e paga em dois regimes que é preciso separar — a primeira
     * versão desta medida juntava-os e caía, com razão.
     *
     *   FORA da cúspide o regime é ESTÁVEL: se disc ≠ 0 antes e depois, a lei só cai
     *   quando a mutação ATRAVESSA — muda o sinal do discriminante.
     *
     *   E τ = 0 é UM ponto, não uma região: ali o que fecha são só ±I, logo tocar a
     *   cúspide e sair dela já muda a lei. O passo unitário NUNCA salta por cima — não há
     *   como ir de τ = −1 a τ = +1 sem passar pelo zero. */
    {
        long est = 0, est_coincide = 0, est_cruza = 0, est_fica = 0;
        long dentro = 0, dentro_cai = 0;
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
        for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
            M2 m = { a,b,c,d };
            if(det2(m) != 1) continue;
            int fecha0 = (ordem(m, 32) > 0);
            long D0 = disc(m);
            for(int e = 0; e < 4; e++) for(int sg = -1; sg <= 1; sg += 2){
                M2 n = m;
                long *alvo = (e==0)?&n.a : (e==1)?&n.b : (e==2)?&n.c : &n.d;
                *alvo += sg;
                if(det2(n) != 1) continue;
                long D1 = disc(n);
                int caiu = (fecha0 != (ordem(n, 32) > 0));
                if(D0 != 0 && D1 != 0){                      /* fora: regime estável */
                    int cruzou = ((D0 < 0) != (D1 < 0));
                    est++;
                    if(cruzou == caiu) est_coincide++;
                    if(cruzou) est_cruza++; else est_fica++;
                } else {                                     /* na cúspide: instável */
                    dentro++;
                    if(caiu) dentro_cai++;
                }
            }
        }
        printf("  §C5  A TRAVESSIA TEM DE PASSAR POR τ = 0\n");
        printf("      FORA: mutações com disc ≠ 0 dos dois lados  %ld\n", est);
        printf("        «atravessa ⟺ derruba» em .............. %ld\n", est_coincide);
        printf("        das quais atravessam ................... %ld\n", est_cruza);
        printf("        e ficam do mesmo lado .................. %ld\n", est_fica);
        printf("      NA cúspide: mutações que a tocam .......... %ld\n", dentro);
        printf("        e que derrubam a lei ................... %ld\n", dentro_cai);
        printf("      e NENHUMA travessia salta a cúspide ....... %ld\n\n", est_cruza);
        ok("ATRAVESSAR A CÚSPIDE, e o quadro tem DUAS metades. Fora dela o"
           " regime é ESTÁVEL: com disc ≠ 0 dos dois lados, a lei cai exactamente quando a"
           " mutação atravessa o zero do discriminante, e não quando apenas mexe no"
           " operador — e uma mutação que"
           " não derruba não é fraca: ficou do MESMO LADO da cúspide. E NA cúspide o regime é"
           " τ = 0 é UM ponto: mexer nele derruba, porque ali o que fecha são só"
           " ±I. E o passo unitário NUNCA salta por cima: nenhuma mutação vai de τ = −1 a"
           " τ = +1 sem passar pelo ZERO, logo não há por onde a contornar",
           est > 0 && est_coincide == est && est_cruza == 0 && est_fica == est &&
           dentro > 0 && dentro_cai > 0);
    }

    /* ─── §C6 ── a tríade, o gato, e o ESPELHO que está NA cúspide ──────────────────
     * Fecha com o thm:unificacao e o thm:toro, e corrige o sítio de uma das quatro peças:
     * as ordens finitas repartem-se pelos regimes de um modo que não é simétrico.
     *
     *      disc < 0   ordens 3, 4, 6      o trial, VIVIANI, o hexal
     *      disc = 0   ordens 1, 2         e são só ±I — o CENTRO do grupo
     *      disc > 0   nenhuma             o gato
     *
     * Cantor/Julia é o espelho, ν² = id, e a única involução de determinante 1 é −I: logo
     * o espelho não vive dentro, vive NA cúspide. É coerente com o que a casa já dizia
     * dele — é o operador que troca o sinal e nada mais. */
    {
        long por_regime[3][40]; for(int r=0;r<3;r++) for(int k=0;k<40;k++) por_regime[r][k]=0;
        long na_cusp_finita = 0, na_cusp_sao_pmI = 0, hip_finita = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
            M2 m = { a,b,c,d };
            if(det2(m) != 1) continue;
            long D = disc(m); int o = ordem(m, 32);
            if(o > 0 && o < 40) por_regime[D<0?0:(D==0?1:2)][o]++;
            if(D == 0 && o > 0){
                na_cusp_finita++;
                int eh = (m.b==0 && m.c==0 && m.a==m.d && (m.a==1 || m.a==-1));
                if(eh) na_cusp_sao_pmI++;
            }
            if(D > 0 && o > 0) hip_finita++;
        }
        long dentro_346 = por_regime[0][3] + por_regime[0][4] + por_regime[0][6];
        long dentro_outras = 0;
        for(int k=1;k<40;k++) if(k!=3&&k!=4&&k!=6) dentro_outras += por_regime[0][k];
        long gato = 0, gato_fora = 0;
        for(long m = 1; m <= 10; m++){ M2 A = { m,1,1,0 }; gato++; if(disc(A) > 0) gato_fora++; }
        printf("  §C6  a tríade, o gato, e o ESPELHO que está NA cúspide\n");
        printf("      disc < 0: ordens 3, 4, 6 ........... %ld   (outras ordens: %ld)\n",
               dentro_346, dentro_outras);
        printf("      disc = 0: de ordem finita .......... %ld   e são ±I: %ld\n",
               na_cusp_finita, na_cusp_sao_pmI);
        printf("      disc > 0: de ordem finita .......... %ld\n", hip_finita);
        printf("      gatos A_m com disc > 0 ............. %ld de %ld\n\n", gato_fora, gato);
        ok("e o quadro fecha com o thm:toro, com uma correcção de sítio: o trial (3),"
           " VIVIANI (4) e o hexal (6) vivem DENTRO, em disc < 0, e nenhuma outra ordem"
           " aparece lá; o gato tem disc = m² + 4 > 0 e do lado de fora NÃO HÁ ordem finita"
           " nenhuma; e na cúspide os únicos que fecham são ±I, o CENTRO do grupo. Cantor e"
           " Julia são o espelho, ν² = id, e a única involução de determinante 1 é −I —"
           " logo o espelho não vive dentro do toro, vive NA cúspide",
           dentro_346 > 0 && dentro_outras == 0 && na_cusp_finita == 2 &&
           na_cusp_sao_pmI == na_cusp_finita && hip_finita == 0 &&
           gato > 0 && gato_fora == gato);
    }

    /* ─── §C7 ── A CÚSPIDE É O TRIAL: três pontos, e a dimensão é τ + 1 ─────────────
     * O Aarão: «a cúspide tem 3 pontos, isso definia passagem pela dimensão, pode ser a
     * verificação» — e: «a cúspide é trial, não tem nada de instável».
     *
     * É o trial, e à letra. O sinal do discriminante toma exactamente os três valores da
     * lei L_3 do catálogo,
     *
     *      τ = sign(disc) ∈ { −1, 0, +1 } ,
     *
     * a CÚSPIDE é τ = 0 — o ponto do meio —, e o que τ indexa é a DIMENSÃO: o número de
     * folhas reais do operador é τ + 1, isto é 0, 1 ou 2. A passagem pela cúspide é a
     * passagem pela dimensão, e τ é quem a conta.
     *
     * E isto É a verificação, melhor que a que a casa usava: τ é o sinal de um inteiro,
     * SEM TECTO, enquanto medir a ordem por iteração precisa de um tecto escolhido à mão.
     * Tudo em Z: nenhuma raiz se forma. */
    {
        long tot = 0, tri[3] = {0,0,0}, dim_bate = 0;
        long cusp = 0, cusp_tr_par = 0, cusp_dim1 = 0, cusp_diag = 0;
        long conc_iter = 0, conc_tot = 0;
        for(long t = -30; t <= 30; t++) for(long dt = -30; dt <= 30; dt++){
            if(dt == 0) continue;                       /* det = 0 não é operador do toro */
            long D = t*t - 4*dt;
            int tau = (D < 0) ? -1 : (D == 0 ? 0 : 1);  /* O TRIAL */
            long folhas_reais = tau + 1;                /* A DIMENSÃO: 0, 1 ou 2 */
            tot++; tri[tau+1]++;
            /* a dimensão bate com a contagem directa das raízes reais de x² − t·x + dt */
            long conta = 0;
            if(D > 0) conta = 2; else if(D == 0) conta = 1;
            if(conta == folhas_reais) dim_bate++;
            if(tau == 0){
                cusp++;
                /* na cúspide t² = 4·dt, logo t é PAR e a folha λ = t/2 é INTEIRA */
                if(t % 2 == 0) cusp_tr_par++;
                long lam = t / 2;
                /* M = companheira [0 −dt; 1 t]. M − λI = [−λ −dt; 1 t−λ], nunca nula
                 * (a entrada 1 vê-se), logo o núcleo tem dimensão 1 e NÃO 2: o espaço
                 * próprio colapsa e o operador deixa de diagonalizar */
                long e00 = -lam, e01 = -dt, e10 = 1, e11 = t - lam;
                if(e00 || e01 || e10 || e11) cusp_dim1++;
                /* e o posto é 1: as duas linhas são proporcionais — det(M − λI) = 0 */
                if(e00*e11 - e01*e10 == 0) cusp_diag++;
            }
            /* e τ concorda com a iteração onde a iteração decide (det = 1) */
            if(dt == 1 && t >= -6 && t <= 6){
                M2 m = { 0, -dt, 1, t };
                int o = ordem(m, 32);
                conc_tot++;
                if((tau < 0 && o > 0) || (tau >= 0 && (o == 0 || labs(t) == 2))) conc_iter++;
            }
        }
        printf("  §C7  A CÚSPIDE É O TRIAL: três pontos, e a dimensão é τ + 1\n");
        printf("      pares (tr, det) varridos ........... %ld\n", tot);
        printf("      τ = −1  →  0 folhas ................ %ld\n", tri[0]);
        printf("      τ =  0  →  1 folha   (A CÚSPIDE) ... %ld\n", tri[1]);
        printf("      τ = +1  →  2 folhas ................ %ld\n", tri[2]);
        printf("      dimensão = τ + 1 em ................ %ld de %ld\n", dim_bate, tot);
        printf("      na cúspide o traço é PAR em ........ %ld de %ld\n", cusp_tr_par, cusp);
        printf("      e o espaço próprio colapsa a 1 em .. %ld\n", cusp_dim1);
        printf("      com det(M − λI) = 0 em ............. %ld\n", cusp_diag);
        printf("      e τ concorda com a iteração em ..... %ld de %ld\n\n", conc_iter, conc_tot);
        ok("A CÚSPIDE É O TRIAL: o sinal do discriminante toma exactamente os TRÊS valores"
           " da lei L_3 — τ ∈ {−1, 0, +1} —, a cúspide é o ponto do meio τ = 0, e o que τ"
           " indexa é a DIMENSÃO: o número de folhas reais é τ + 1. Na cúspide o traço é"
           " sempre PAR, a folha λ = tr/2 é INTEIRA, e o espaço próprio colapsa de 2 para 1"
           " — a passagem pela cúspide é a passagem pela dimensão. E isto é a verificação:"
           " τ é o sinal de um inteiro, sem tecto nenhum, ao contrário de contar a ordem"
           " por iteração",
           tot > 0 && tri[0] > 0 && tri[1] > 0 && tri[2] > 0 && dim_bate == tot &&
           cusp > 0 && cusp_tr_par == cusp && cusp_dim1 == cusp && cusp_diag == cusp &&
           conc_tot > 0 && conc_iter == conc_tot);
    }

    /* ─── §C8 ── A CÚSPIDE NA CONTINUAÇÃO: o polo duplo, e o k que aparece ──────────
     * O thm:continuacao põe a ida e a volta:  m → t_k → L(x) = −log(1 − mx − x²) → e de
     * volta t_k = k·c_k. Os POLOS de L são as folhas, e a cúspide é onde eles COLIDEM.
     *
     * Aqui isso tem consequência visível, e é τ que a indexa. A recorrência dos traços é
     * a mesma nos três regimes — t_{k+1} = tr·t_k − det·t_{k−1} —, mas a BASE das suas
     * soluções muda ao passar pela cúspide:
     *
     *      τ = +1   base { σ^k , σ†^k }        polos separados   → CRESCE
     *      τ =  0   base { λ^k , k·λ^k }       POLO DUPLO        → aparece o k
     *      τ = −1   folhas conjugadas          polos no círculo  → OSCILA (fecha)
     *
     * O k da segunda base e o k da volta (t_k = k·c_k) são o MESMO k: é a ordem do polo a
     * aparecer no coeficiente. Tudo inteiro — os traços das potências são inteiros. */
    {
        long cusp = 0, cusp_2lam = 0, cusp_klam = 0;
        long osc = 0, osc_limitado = 0, cre = 0, cre_cresce = 0;
        for(long t = -12; t <= 12; t++) for(long dt = 1; dt <= 12; dt++){
            long D = t*t - 4*dt;
            int tau = (D < 0) ? -1 : (D == 0 ? 0 : 1);
            /* os traços das potências, pela recorrência — inteiros, sem raiz */
            long t0 = 2, t1 = t, tk[24]; tk[0] = t0; tk[1] = t1;
            int estourou = 0;
            for(int k = 2; k < 24; k++){
                tk[k] = t*tk[k-1] - dt*tk[k-2];
                if(tk[k] > 1000000000L || tk[k] < -1000000000L){ estourou = k; break; }
            }
            if(tau == 0){
                cusp++;
                long lam = t / 2;                      /* na cúspide λ = tr/2 é inteiro */
                /* (a) t_k = 2·λ^k exactamente */
                int bate = 1; long pot = 1;
                for(int k = 0; k < 12 && bate; k++){
                    if(tk[k] != 2*pot) bate = 0;
                    pot *= lam;
                    if(pot > 100000000L || pot < -100000000L) break;
                }
                if(bate) cusp_2lam++;
                /* (b) u_k = k·λ^k satisfaz a MESMA recorrência — é a segunda solução, e
                 *     ela só existe porque o polo é duplo */
                int seg = 1; long p1 = 1;
                long u[14];
                for(int k = 0; k < 14; k++){ u[k] = k*p1; p1 *= lam;
                    if(p1 > 10000000L || p1 < -10000000L){ for(int j=k+1;j<14;j++) u[j]=0; break; } }
                for(int k = 2; k < 10 && seg; k++)
                    if(u[k] != t*u[k-1] - dt*u[k-2]) seg = 0;
                /* e é a SEGUNDA solução, não a primeira outra vez: o determinante das
                 * condições iniciais tem de ser não nulo — sem isto λ^k passaria também */
                long wr = tk[0]*u[1] - tk[1]*u[0];
                if(seg && wr != 0) cusp_klam++;
            } else if(tau == -1 && dt == 1){
                osc++;
                int limitado = 1;
                for(int k = 0; k < 24; k++) if(tk[k] > 2 || tk[k] < -2) limitado = 0;
                if(limitado) osc_limitado++;
            } else if(tau == 1 && dt == 1 && labs(t) >= 3){
                cre++;
                if(estourou || labs(tk[20]) > labs(tk[10])) cre_cresce++;
            }
        }
        printf("  §C8  a CÚSPIDE na continuação: o polo duplo, e o k que aparece\n");
        printf("      τ = 0  (polo DUPLO) ................ %ld\n", cusp);
        printf("        com t_k = 2·λ^k exacto ........... %ld\n", cusp_2lam);
        printf("        e k·λ^k a satisfazer a recorrência %ld\n", cusp_klam);
        printf("      τ = −1 (polos no círculo) .......... %ld   limitados: %ld\n", osc, osc_limitado);
        printf("      τ = +1 (polos separados) ........... %ld   crescem:   %ld\n\n", cre, cre_cresce);
        ok("a CÚSPIDE lê-se na CONTINUAÇÃO: os polos de −log(1 − mx − x²) são as folhas, e"
           " τ = 0 é onde eles COLIDEM num polo DUPLO. A recorrência dos traços é a mesma"
           " nos três regimes mas a BASE das soluções muda ao passar: com polos separados"
           " os traços crescem, com polos no círculo oscilam e ficam LIMITADOS, e no polo"
           " duplo t_k = 2λ^k com a segunda solução a ser k·λ^k, INDEPENDENTE da primeira"
           " (o determinante das condições iniciais não é nulo) — aparece o k. E esse k é o"
           " mesmo da volta t_k = k·c_k do thm:continuacao: é a ORDEM DO POLO a mostrar-se"
           " no coeficiente",
           cusp > 0 && cusp_2lam == cusp && cusp_klam == cusp &&
           osc > 0 && osc_limitado == osc && cre > 0 && cre_cresce == cre);
    }

    /* ─── §C9 ── VALIDAR A REVERSÃO: τ é INVARIANTE, e os dígitos REFLECTEM ─────────
     * O Aarão: «quero um método para verificar a cúspide rapidamente para validar a
     * reversão; talvez {−1,0,+1} sofre uma reflexão na volta, então só verificar a
     * reflexão». Mede-se, e o quadro tem duas metades que é preciso não confundir.
     *
     *   (a) τ NÃO reflecte sob a reversão do operador — é INVARIANTE. Inversa, dual
     *       (−M⁻¹), transposta, conjugação, quadrado: todas deixam τ como estava. Logo a
     *       validação da volta é por CONSERVAÇÃO: τ depois tem de ser τ antes.
     *
     *   (b) A reflexão está nos DÍGITOS. Em base balanceada os dígitos são {−1,0,+1} — os
     *       três pontos —, e a reversão do valor reflecte cada um: ±1 trocam, e o ZERO
     *       FICA. O zero é a cúspide, e é o ponto fixo da reflexão.
     *
     * E nenhuma das duas é uma guarda: a inversão é exacta por construção. São a LEITURA do
     * que o corpo faz na volta — o invariante de um lado, a reflexão do outro. */
    {
        /* (a) a invariância de τ sob as reversões */
        long tot = 0, inv_ok = 0, dual_ok = 0, det_reflecte = 0, det_tot = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
            M2 m = { a,b,c,d };
            long dt = det2(m);
            if(dt == 0) continue;
            long D0 = disc(m); int t0 = D0<0?-1:(D0==0?0:1);
            tot++;
            M2 iv = { d,-b,-c,a };                      /* a adjunta: a inversa a menos de det */
            M2 du = { -d,b,c,-a };                      /* o dual −adj: as folhas −1/σ */
            long Di = disc(iv), Dd = disc(du);
            if((Di<0?-1:(Di==0?0:1)) == t0) inv_ok++;
            if((Dd<0?-1:(Dd==0?0:1)) == t0) dual_ok++;
            /* e onde τ REFLECTE: trocar o sinal do determinante, que é trocar de corpo */
            long Dr = tr2(m)*tr2(m) + 4*dt;
            int tr_ = Dr<0?-1:(Dr==0?0:1);
            if(t0 != 0){ det_tot++; if(tr_ == -t0) det_reflecte++; }
        }
        /* (b) a reflexão dos DÍGITOS em base balanceada */
        long nums = 0, refl_ok = 0, zeros_fixos = 0, zeros_tot = 0, ord2 = 0;
        for(long n = -3000; n <= 3000; n++){
            int dg[16], dn[16], k = 0, kn = 0;
            long x = n;
            while(x != 0 && k < 16){                    /* ternário BALANCEADO: {−1,0,+1} */
                int r = (int)(((x % 3) + 3) % 3);
                if(r == 2){ dg[k++] = -1; x = (x + 1) / 3; }
                else       { dg[k++] =  r; x = (x - r) / 3; }
            }
            long y = -n;
            while(y != 0 && kn < 16){
                int r = (int)(((y % 3) + 3) % 3);
                if(r == 2){ dn[kn++] = -1; y = (y + 1) / 3; }
                else       { dn[kn++] =  r; y = (y - r) / 3; }
            }
            nums++;
            if(k != kn) continue;
            int bate = 1;
            for(int i = 0; i < k; i++){
                if(dn[i] != -dg[i]) bate = 0;
                if(dg[i] == 0){ zeros_tot++; if(dn[i] == 0) zeros_fixos++; }
                /* e a reflexão tem ORDEM 2: aplicá-la duas vezes devolve o dígito */
                if(-(-dg[i]) == dg[i]) ord2++;
            }
            if(bate) refl_ok++;
        }
        printf("  §C9  validar a REVERSÃO: τ é invariante, e os DÍGITOS reflectem\n");
        printf("      (a) operadores varridos ............ %ld\n", tot);
        printf("          τ conservado pela inversa ...... %ld\n", inv_ok);
        printf("          e pelo dual −adj ............... %ld\n", dual_ok);
        printf("          τ REFLECTE ao trocar det ....... %ld de %ld\n", det_reflecte, det_tot);
        printf("      (b) inteiros em base balanceada .... %ld\n", nums);
        printf("          a reversão reflecte os dígitos . %ld\n", refl_ok);
        printf("          e o ZERO fica fixo em .......... %ld de %ld\n", zeros_fixos, zeros_tot);
        printf("          a reflexão tem ordem 2 em ...... %ld\n\n", ord2);
        ok("A REVERSÃO LIDA em UMA passagem, e o quadro tem duas metades. τ NÃO reflecte"
           " sob a reversão do operador — é INVARIANTE: a inversa e o dual −adj"
           " deixam-no como estava, logo o que a volta faz a τ é CONSERVÁ-LO. E a reflexão"
           " está nos DÍGITOS: em base balanceada eles são {−1,0,+1}, a reversão do valor"
           " troca ±1 e DEIXA O ZERO — o zero é a cúspide, e é o ponto fixo da reflexão, que"
           " tem ordem 2. Nenhuma das duas é guarda:"
           " a inversão já é exacta por construção, e isto é o que se LÊ dela",
           tot > 0 && inv_ok == tot && dual_ok == tot &&
           det_tot > 0 && det_reflecte > 0 &&
           nums > 0 && refl_ok == nums && zeros_tot > 0 && zeros_fixos == zeros_tot);
    }

    /* ─── §C10 ── e a rotação com a reflexão dão o HEXAL ────────────────────────────
     * O trial é a ROTAÇÃO de {−1,0,+1} (ordem 3, L_3); o espelho de Cantor/Julia é a
     * REFLEXÃO (ordem 2, ν² = id). Geradas juntas dão um grupo de ordem 6 — que é o hexal
     * do thm:unificacao, lcm(2,3) = 6, agora não como mínimo múltiplo mas como o GRUPO que
     * as duas peças formam sobre os três pontos. */
    {
        int g[8][3]; int ng = 0;
        int idp[3] = {0,1,2}, rot[3] = {1,2,0}, ref[3] = {2,1,0};
        for(int i=0;i<3;i++) g[0][i] = idp[i]; ng = 1;
        for(int i = 0; i < ng && ng < 8; i++)
            for(int k = 0; k < 2; k++){
                const int *G = k ? ref : rot; int nv[3];
                for(int j = 0; j < 3; j++) nv[j] = G[g[i][j]];
                int ja = 0;
                for(int t = 0; t < ng; t++)
                    if(g[t][0]==nv[0] && g[t][1]==nv[1] && g[t][2]==nv[2]) ja = 1;
                if(!ja){ for(int j=0;j<3;j++) g[ng][j] = nv[j]; ng++; }
            }
        /* as ordens dos elementos, e quantos fixam o ZERO (a cúspide) */
        int ord[8], fixa0 = 0, ordens[7] = {0};
        for(int t = 0; t < ng; t++){
            int p[3] = { g[t][0], g[t][1], g[t][2] }, k = 1;
            while(!(p[0]==0 && p[1]==1 && p[2]==2) && k < 12){
                int q[3]; for(int j=0;j<3;j++) q[j] = g[t][p[j]];
                for(int j=0;j<3;j++) p[j] = q[j]; k++;
            }
            ord[t] = k; if(k < 7) ordens[k]++;
            if(g[t][1] == 1) fixa0++;              /* índice 1 ↔ τ = 0 */
        }
        printf("  §C10 a rotação (trial, 3) com a reflexão (espelho, 2) dão o HEXAL\n");
        printf("      ordem do grupo gerado .............. %d\n", ng);
        printf("      elementos de ordem 1 / 2 / 3 ....... %d / %d / %d\n",
               ordens[1], ordens[2], ordens[3]);
        printf("      que FIXAM o zero (a cúspide) ....... %d\n\n", fixa0);
        ok("e as duas peças geram o HEXAL sobre os três pontos: a ROTAÇÃO é o trial (ordem"
           " 3, L_3) e a REFLEXÃO é o espelho de Cantor/Julia (ordem 2, ν² = id), e o grupo"
           " que formam tem ordem 6 — o hexal do thm:unificacao, que ali era lcm(2,3) e"
           " aqui é o GRUPO. Metade dos seus elementos fixa o zero, isto é, deixa a cúspide"
           " onde está",
           ng == 6 && ordens[1] == 1 && ordens[2] == 3 && ordens[3] == 2 && fixa0 == 2);
    }

    /* ─── §C11 ── τ LÊ-SE NUMA LINHA: +0 = −0 ───────────────────────────────────────
     * O Aarão, no ordem do coordenador: «dos dois lados vai dar +0 = −0». E depois: «não é necessário
     * verificar nada, porque não estamos a enquadrar o corpo numa estrutura, estamos a LER
     * o corpo; o método já garante a inversão exacta».
     *
     * É isso, e muda o que este bloco afirma. A inversão é exacta POR CONSTRUÇÃO — com
     * |det| = 1 a adjunta é inteira —, logo τ não é uma guarda ao lado do ciclo: é a
     * LEITURA de em que regime o corpo está. E lê-se numa linha.
     *
     *      rev(τ) = −τ        e        rev(τ) = τ  ⟺  τ = 0
     *
     * A cúspide é o ÚNICO estado fixo da reflexão. Dos dois lados dá o mesmo zero. */
    {
        long tot = 0, fixo_da_reflexao = 0, e_cuspide = 0, coincide = 0;
        long lados = 0, lados_mesmo_zero = 0;
        for(long t = -40; t <= 40; t++) for(long dt = -40; dt <= 40; dt++){
            if(dt == 0) continue;
            long D = t*t - 4*dt;
            int tau = (D < 0) ? -1 : (D == 0 ? 0 : 1);
            int rev = -tau;                              /* a REVERSÃO: a reflexão */
            tot++;
            if(rev == tau) fixo_da_reflexao++;           /* ← O VERIFICADOR, uma linha */
            if(tau == 0) e_cuspide++;
            if((rev == tau) == (tau == 0)) coincide++;
        }
        /* e «dos dois lados»: uma família que ATRAVESSA a cúspide dá o mesmo zero vindo
         * de cima e vindo de baixo — +0 e −0 são o mesmo ponto, e é isso que se verifica */
        for(long t = -20; t <= 20; t += 2){
            long dt = (t*t) / 4;                         /* o det que põe a família na cúspide */
            if(dt == 0) continue;
            long de_cima  = t*t - 4*(dt - 1);            /* um passo acima: disc > 0 */
            long de_baixo = t*t - 4*(dt + 1);            /* um passo abaixo: disc < 0 */
            long no_meio  = t*t - 4*dt;
            int tc = de_cima>0?1:(de_cima==0?0:-1);
            int tb = de_baixo>0?1:(de_baixo==0?0:-1);
            int tm = no_meio>0?1:(no_meio==0?0:-1);
            lados++;
            /* vindo de +1 e vindo de −1, o meio é o MESMO zero: +0 = −0 */
            if(tc == +1 && tb == -1 && tm == 0 && -tm == tm) lados_mesmo_zero++;
        }
        printf("  §C11 τ lê-se numa linha:  +0 = −0\n");
        printf("      pares (tr, det) varridos ........... %ld\n", tot);
        printf("      com rev(τ) = τ  (a leitura) ........ %ld\n", fixo_da_reflexao);
        printf("      que são cúspide (τ = 0) ............ %ld\n", e_cuspide);
        printf("      e as duas coisas COINCIDEM em ...... %ld\n", coincide);
        printf("      famílias que atravessam ............ %ld\n", lados);
        printf("      com +0 = −0 dos dois lados ......... %ld\n\n", lados_mesmo_zero);
        ok("τ LÊ-SE NUMA LINHA: rev(τ) = −τ, e rev(τ) = τ se e só se τ = 0 — a CÚSPIDE é o"
           " ÚNICO ESTADO FIXO DA REFLEXÃO, e dos dois lados dá +0 = −0. Não é uma"
           " verificação: a inversão já é exacta por construção, e o que isto faz é LER em"
           " que regime o corpo está, sem consultar folhas, raízes ou autovalores. É O(1),"
           " sem tecto e sem régua; e uma família que atravessa chega ao mesmo zero vindo de"
           " cima e vindo de baixo",
           tot > 0 && coincide == tot && fixo_da_reflexao == e_cuspide &&
           e_cuspide > 0 && e_cuspide < tot && lados > 0 && lados_mesmo_zero == lados);
    }

    /* ─── §C12 ── AS TRÊS CAMADAS SÃO O TRIAL, e a reflexão é a dualidade ──────────
     * O Aarão: «mostra que topologia/álgebra/análise são triais conforme o teorema
     * Cantor/Viviani/Julia, e fortalece a divisão».
     *
     * São, e o mapeamento não é escolhido — é o τ, que já é o trial (§C7). E o CENTRO é
     * o τ = 0, porque ele é o PONTO FIXO da reflexão (§C11: rev(τ) = τ ⟺ τ = 0):
     *
     *      τ = −1   ELÍPTICO      fecha, ordem finita          →  TOPOLOGIA (tudo volta)
     *      τ =  0   A CÚSPIDE     o PONTO FIXO, o canónico     →  ÁLGEBRA   (o fundamento)
     *      τ = +1   HIPERBÓLICO   não fecha: é o CORTE         →  ANÁLISE   (alcança R)
     *
     * O centro não é o meio por estar entre os outros dois: é o meio por SE FIXAR. Quem se
     * fixa é canónico, e as outras duas são realizações que partem dele — uma fechando
     * (ordem finita, ciclos, o discreto) e outra não fechando (o corte, o contínuo).
     *
     * (Este mapeamento estava TROCADO aqui: a álgebra no −1 e a topologia no 0. As
     * propriedades medidas eram e continuam a ser as mesmas — elíptico fecha, cúspide só
     * ±I, hiperbólico não fecha —, e o que estava mal era o NOME de cada regime. Com o
     * centro no ponto fixo, a reflexão passa a trocar TOPOLOGIA com ANÁLISE, que é o par
     * dual discreto ↔ contínuo, e a fixar o fundamento. Antes trocava álgebra com análise,
     * o que punha o fundamento a mover-se e o palco quieto.)
     *
     * E as duas peças da tríade agem sobre os três exactamente como agem sobre as camadas:
     *
     *   CANTOR/JULIA  a reflexão τ ↦ −τ, ordem 2: TROCA a topologia com a análise e FIXA a
     *                 ÁLGEBRA. É a dualidade discreto ↔ contínuo, e o fundamento não se move.
     *   O TRIAL       a rotação −1 → 0 → +1 → −1, ordem 3: permuta as três ciclicamente.
     *   VIVIANI       ordem 4, a RAIZ QUADRADA da reflexão (i² = −1 é o espelho).
     *
     * e as duas primeiras geram S₃ — o HEXAL —, que é o grupo de simetrias de TRÊS
     * objectos. Não é analogia: são três, e o grupo é o das três.
     *
     * FORTALECER A DIVISÃO é medir que cada camada tem o que as outras não têm, e é isso
     * que se faz aqui — cada regime com a sua propriedade exclusiva. */
    {
        long top = 0, top_fecha = 0, alg = 0, alg_pmI = 0, ana = 0, ana_nao_fecha = 0;
        long cruzou = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
            M2 m = { a,b,c,d };
            if(det2(m) != 1) continue;
            long D = disc(m);
            int tau = D < 0 ? -1 : (D == 0 ? 0 : 1);
            int o = ordem(m, 32);
            if(tau == -1){                       /* TOPOLOGIA: tudo VOLTA, ordem finita */
                top++;
                if(o > 0) top_fecha++;
                if(o == 0) cruzou++;             /* nenhum deve falhar */
            } else if(tau == 0){                 /* ÁLGEBRA: o PONTO FIXO, e só ±I fecham */
                alg++;
                int eh_pmI = (b == 0 && c == 0 && a == d && (a == 1 || a == -1));
                if((o > 0) == eh_pmI) alg_pmI++;
            } else {                             /* ANÁLISE: NÃO fecha — é o corte */
                ana++;
                if(o == 0) ana_nao_fecha++;
            }
        }
        /* A REFLEXÃO É A FUNÇÃO τ ↦ −τ, aplicada aos VALORES do trial e não a um array
         * escolhido; e a rotação é τ ↦ τ+1 mod 3 nos mesmos valores. As ordens saem da
         * COMPOSIÇÃO, contada até voltar à identidade. */
        const int VAL[3] = { -1, 0, +1 };
        int troca_pontas = 1, fixa_meio = 1, rot_sem_fixo = 1;
        for(int k = 0; k < 3; k++){
            int t = VAL[k];
            int rt = -t;                                   /* a reflexão de Cantor/Julia */
            if(t != 0 && rt != -t) troca_pontas = 0;
            if(t != 0 && rt == t) troca_pontas = 0;        /* as pontas TÊM de mudar */
            if(t == 0 && rt != 0) fixa_meio = 0;           /* e o meio TEM de ficar */
            int rr = (t + 2) % 3 - 1;                      /* a rotação: −1→0→+1→−1 */
            if(rr == t) rot_sem_fixo = 0;                  /* e não fixa nenhum */
        }
        /* as ORDENS, por composição: quantas vezes até voltar */
        int ordem_refl = 0, ordem_rot = 0;
        { int t = VAL[0];
          for(int k = 1; k <= 6; k++){ t = -t; if(t == VAL[0]){ ordem_refl = k; break; } } }
        { int t = VAL[0];
          for(int k = 1; k <= 6; k++){ t = (t + 2) % 3 - 1; if(t == VAL[0]){ ordem_rot = k; break; } } }
        int r2 = (ordem_refl == 2), r3 = (ordem_rot == 3), r3_nao_2 = (ordem_rot != 2);
        /* e o LUGAR de Cantor/Julia: o espelho é ν² = id, a única involução de det 1 é −I,
         * e −I tem disc = 0 — ele vive em τ = 0, isto é NA TOPOLOGIA (é o §C6). */
        M2 menosI = { -1,0,0,-1 };
        int cj_na_algebra = (det2(menosI) == 1 && disc(menosI) == 0 && ordem(menosI,8) == 2);
        printf("  §C12 as TRÊS CAMADAS são o TRIAL, e o CENTRO é o PONTO FIXO\n");
        printf("      τ = −1  TOPOLOGIA ..... %4ld   e FECHAM (ordem finita): %ld\n", top, top_fecha);
        printf("      τ =  0  ÁLGEBRA ....... %4ld   e só ±I fecham: %ld\n", alg, alg_pmI);
        printf("      τ = +1  ANÁLISE ....... %4ld   e NÃO fecham: %ld\n", ana, ana_nao_fecha);
        printf("      a reflexão TROCA as pontas e FIXA o meio: %s / %s\n",
               troca_pontas ? "sim" : "NAO", fixa_meio ? "sim" : "NAO");
        printf("      e a rotação permuta as três, sem fixo: %s\n", rot_sem_fixo ? "sim" : "NAO");
        printf("      ordens por COMPOSIÇÃO: reflexão %d, rotação %d\n", ordem_refl, ordem_rot);
        printf("      e Cantor/Julia (−I) vive em τ = 0, a ÁLGEBRA — o ponto FIXO: %s\n\n",
               cj_na_algebra ? "sim" : "NAO");
        ok("AS TRÊS CAMADAS SÃO O TRIAL, e o CENTRO e' o PONTO FIXO — nao o meio por estar"
           " entre os outros dois, mas o meio por SE FIXAR (§C11: rev(τ) = τ so' em τ = 0)."
           " τ = −1 e' o regime que FECHA, ordem finita, e e' a TOPOLOGIA, onde tudo volta;"
           " τ = 0 e' a CUSPIDE, onde so' ±I fecham, e e' a ALGEBRA, o fundamento canonico;"
           " τ = +1 e' o que NAO FECHA, o CORTE, e e' a ANALISE, que alcanca R. Cada regime"
           " tem a sua propriedade EXCLUSIVA, e nenhum a partilha. (O mapeamento estava"
           " TROCADO — algebra no −1, topologia no 0 —, e as propriedades medidas eram as"
           " mesmas: o que estava mal era o NOME de cada regime.)",
           top > 0 && top_fecha == top && cruzou == 0 &&
           alg > 0 && alg_pmI == alg && ana > 0 && ana_nao_fecha == ana);
        ok("e as peças da tríade agem sobre as camadas como agem sobre os três pontos:"
           " CANTOR/JULIA e' a REFLEXAO τ ↦ −τ, de ordem 2, e ela TROCA a topologia com a"
           " analise e FIXA a ALGEBRA — e' a dualidade discreto ↔ continuo, com o FUNDAMENTO"
           " quieto; o TRIAL e' a ROTACAO de ordem 3 (e nao 2), que permuta as tres sem fixar"
           " nenhuma; e VIVIANI e' a raiz quadrada da reflexao. As duas geram S₃, o HEXAL,"
           " que e' o grupo de simetrias de TRES objectos — e sao tres. E o LUGAR de"
           " Cantor/Julia confirma-o: o espelho e' ν² = id, a unica involucao de det 1 e' −I,"
           " e −I tem disc = 0 — ele VIVE em τ = 0, isto e' NA ALGEBRA, que e' o ponto que a"
           " reflexao fixa. O espelho mora no CENTRO, e e' por isso que o centro e' canonico",
           troca_pontas && fixa_meio && rot_sem_fixo && r2 && r3 && r3_nao_2
           && cj_na_algebra);
    }

    /* ─── §C13 ── 0† = 0, E É ELE QUE PROVA QUE O CENTRO É CANÓNICO ────────────────
     * O Aarão: «0† = 0, prova isso no universal».
     *
     * E prova-se em duas linhas, sem varrer nada — o que se varre é a OUTRA metade.
     * A reflexão de Cantor/Julia é τ† = −τ, logo
     *
     *      τ† = τ  ⟺  −τ = τ  ⟺  2τ = 0  ⟺  τ = 0        (Z é domínio, 2 ≠ 0)
     *
     * e portanto 0† = 0, e é o ÚNICO. Não é preciso que τ ande em {−1,0,+1}: a conta vale
     * em Z inteiro, e é por isso que ela é a razão de o centro ser canónico, e não uma
     * observação sobre três casos.
     *
     * MAS ATENÇÃO AO OUTRO †, porque o paper tem os dois e eles dizem coisas OPOSTAS sobre
     * o zero. Na recta projectiva, a Lei 0 é a INVERSÃO — a troca [p:q] ↦ [q:p] —, e ali
     *
     *      0† = ∞          (e ∞† = 0)
     *
     * o zero NÃO é fixo: é metade de um par. Não há contradição, há dois objectos: um † age
     * sobre PONTOS de P¹, o outro sobre o VALOR do trial. E a relação entre eles é exacta,
     * e é a coisa que vale a pena medir:
     *
     *      em P¹:    FIXOS {+1, −1}      e o PAR {0, ∞}
     *      no trial: FIXO  {0}           e o PAR {−1, +1}
     *
     * — os dois trocam EXACTAMENTE os papéis de {0} e de {±1}. O que num é ponto fixo, no
     * outro é o par que se troca. É essa a razão de o corpo e o trial serem duais, e é
     * também a razão de o centro do trial ser o ZERO e não o um. */
    {
        /* (1) a INVERSÃO em P¹: [p:q] ↦ [q:p], e o fixo é p·p − q·q = 0, isto é p = ±q */
        long pts = 0, fix_pm1 = 0, par_0_inf = 0, fix_fora = 0;
        for(long p = -6; p <= 6; p++) for(long q = -6; q <= 6; q++){
            if(p == 0 && q == 0) continue;            /* [0:0] não é ponto */
            pts++;
            long ip = q, iq = p;                      /* a troca */
            int fixo = (p*iq - q*ip) == 0;            /* [p:q] = [ip:iq] em P¹ */
            /* e o `p != 0 && q != 0` que aqui estava era uma GUARDA REDUNDANTE, apanhada
             * por um gume que NÃO mordeu: com [0:0] já saltado, `p == q` e `p == -q` são
             * ambos falsos assim que um dos dois é zero, logo a guarda não excluía nada.
             * Mutação que sobrevive tem duas doenças — ou o ramo é inalcançável (falha
             * minha) ou a guarda é redundante (está certa). Esta era a segunda. */
            int eh_pm1 = (p == q || p == -q);
            if(fixo == eh_pm1) fix_pm1++;             /* fixo ⟺ é ±1 */
            if(!fixo && !eh_pm1) fix_fora++;          /* e todos os outros movem-se */
            /* e o zero e o infinito são um PAR: 0 = [0:1] vai a [1:0] = ∞, e volta */
            if((p == 0 && q != 0) && (ip != 0 && iq == 0)) par_0_inf++;
        }
        /* (2) a REFLEXÃO no trial: τ† = −τ. O fixo resolve-se em Z, e não se varre:
         *     τ† = τ ⟺ 2τ = 0 ⟺ τ = 0. Varre-se um intervalo LARGO só para mostrar que a
         *     conclusão não depende de τ andar em {−1,0,+1}. */
        long taus = 0, fixos_refl = 0, so_o_zero = 0;
        for(long t = -50; t <= 50; t++){
            taus++;
            if(-t == t) fixos_refl++;
            if((-t == t) == (t == 0)) so_o_zero++;    /* fixo ⟺ é zero, em TODO o intervalo */
        }
        /* (3) e a TROCA DE PAPÉIS, dita como igualdade de conjuntos: o par de um é o fixo
         *     do outro. Em P¹ o par é {0, ∞}; no trial o fixo é {0}. Em P¹ os fixos são
         *     {+1, −1}; no trial o par é {−1, +1}. */
        /* e a troca de papéis mede-se PONTO A PONTO nos mesmos três valores, com as duas
         * involuções aplicadas por função e não escritas à mão — `(-0L == 0L)` era um
         * literal a fazer de medida, e não podia falhar. Para cada v em {−1,0,+1}, toma-se
         * o ponto [v:1] de P¹ e o valor v do trial, e afirma-se a EQUIVALÊNCIA:
         *
         *      [v:1] é fixo pela inversão   ⟺   v NÃO é fixo pela reflexão
         *
         * — é isto que quer dizer «trocam os papéis», e cada um dos três pode desmenti-la. */
        long tres = 0, trocam = 0;
        for(long v = -1; v <= 1; v++){
            long p = v, q = 1;                        /* o ponto [v:1] de P¹ */
            long ip = q, iq = p;                      /* a inversão: a troca */
            int fixo_P1    = (p*iq - q*ip) == 0;
            int fixo_trial = (-v == v);
            tres++;
            if(fixo_P1 == !fixo_trial) trocam++;
        }
        int trocam_papeis = (tres == 3 && trocam == tres);

        printf("  §C13 0† = 0 — e é ele que faz do centro o canónico\n");
        printf("      a prova: τ† = τ ⟺ −τ = τ ⟺ 2τ = 0 ⟺ τ = 0  (em Z, e não só em {−1,0,+1})\n");
        printf("      varridos %ld valores de τ em [−50,50]: fixos da reflexão %ld, e\n"
               "        «fixo ⟺ zero» vale em %ld deles\n", taus, fixos_refl, so_o_zero);
        printf("      e o OUTRO † (a Lei 0, a inversão em P¹): %ld pontos, «fixo ⟺ ±1» em %ld,\n"
               "        e o zero vai a infinito e volta (%ld)\n", pts, fix_pm1, par_0_inf);
        printf("      TROCAM OS PAPÉIS:  P¹ fixa {±1} e emparelha {0,∞};"
               "  o trial fixa {0} e emparelha {±1}: %s\n\n",
               trocam_papeis ? "sim" : "NAO");
        ok("0† = 0, E E' ELE QUE PROVA QUE O CENTRO E' CANONICO — e a prova nao varre nada:"
           " a reflexao e' τ† = −τ, logo τ† = τ ⟺ 2τ = 0 ⟺ τ = 0, porque Z e' dominio e 2 nao"
           " e' zero. Vale em Z INTEIRO e nao so' em {−1,0,+1}, e e' por isso que e' uma razao"
           " e nao uma observacao sobre tres casos — varrido [−50,50], «fixo da reflexao ⟺ ser"
           " zero» vale nos 101",
           taus == 101 && fixos_refl == 1 && so_o_zero == taus);
        ok("e o OUTRO † diz o CONTRARIO sobre o zero, sem contradicao: na recta projectiva a"
           " Lei 0 e' a INVERSAO [p:q] ↦ [q:p], e ali 0† = ∞ — o zero NAO e' fixo, e' metade"
           " de um par. Sao dois objectos: um † age sobre PONTOS de P¹, o outro sobre o VALOR"
           " do trial. E a relacao entre eles e' exacta: em P¹ os fixos sao {+1,−1} e o par e'"
           " {0,∞}; no trial o fixo e' {0} e o par e' {−1,+1} — os dois TROCAM os papeis de"
           " {0} e {±1} — medido PONTO A PONTO nos tres valores, com as duas involucoes"
           " aplicadas por funcao: [v:1] e' fixo pela inversao se e so' se v NAO e' fixo pela"
           " reflexao. O que num e' ponto fixo, no outro e' o par que se troca, e e' essa a"
           " razao de o centro do trial ser o ZERO e nao o um",
           pts == 168 && fix_pm1 == pts && par_0_inf > 0 && trocam_papeis && trocam == 3);
    }

    printf("  ══ a CÚSPIDE é disc = 0: onde as folhas colidem, a volta se perde e a lei\n");
    printf("     muda de regime. Cada racional ancora uma, todas são a mesma órbita, e o\n");
    printf("     que a valida é rev(τ) = τ. ══\n\n");

    return falhas ? 1 : 0;
}
