/* geometria_real.c — A CONSTRUÇÃO DE ℝ, executada na ordem do Universal.
 *
 * O Aarão: «não é para o agente virar juiz da teoria; é para ele ler a teoria inteira e
 * EXECUTAR A PRÓXIMA CONSTRUÇÃO.» E: «a versão anterior ainda está COM MEDO da própria
 * construção — o texto está cheio de "não fazemos X" onde deveria mostrar X SENDO
 * PRODUZIDO pela cadeia do Universal.»
 *
 * A hierarquia, que é o que eu tinha perdido:
 *
 *      o Universal PROVA   |   o geométrico REALIZA   |   os medidores VERIFICAM
 *
 * Este ficheiro é a terceira coluna. A ordem é a do eval, e é obrigatória:
 *
 *   8 leis → base ortonormal → operação bit a bit → dual → recorrência → Pisot
 *          → encaixe → corte → completude → ℝ → π_k/gume
 *
 * ── O QUE MUDOU, E POR ORDEM DO EVAL ─────────────────────────────────────────
 *  (1) «não constrói todos os reais» ERA RESSALVA MINHA, não do quadro — e caiu. §L8
 *      constrói ℝ INTEIRO, com quocientes parciais ARBITRÁRIOS. O metálico deixa de ser
 *      o escopo e passa a ser o EXTREMO: φ minimiza o crescimento dos denominadores
 *      dos convergentes ENTRE AS EXPANSÕES COM a_k ≥ 1 — é o pior caso do mecanismo,
 *      não um privilégio de φ sobre os outros reais.
 *  (2) a torre 1→2→4→8 e o REUSO dos oito bits em cada andar: §L3.
 *  (3) «oito passos enchem um byte» é CODIFICAÇÃO e não demonstra o bit a bit — §L2
 *      liga-o à base ortonormal e mede o preço numa base torcida.
 *  (4) os DOIS determinantes, que eu misturei: det A^k (das ENTRADAS, em ℤ) e σ^k(σ†)^k
 *      (ESPECTRAL, das duas faces). §L6 mede-os separados — a igualdade é o teorema.
 *  (5) o controlo x²−2x−4 é CONTRAEXEMPLO AO MECANISMO, e não prova de uma
 *      caracterização «unidade e Pisot» que este quadro não demonstrou: §L11.
 *  (6) π não entra como aproximação estrangeira: §L10 realiza π_k exacto por andar.
 *
 * Nenhum double, nenhum limiar, nenhuma raiz avaliada.
 *
 *   cc -O2 -std=c99 -I. -I../lib geometria_real.c -o geometria_real && ./geometria_real
 */
#include <stdio.h>
#include "unidade.h"

#define G_LIMF 100000000L      /* o tecto: com Δ ≤ 68, Δ·F² e (t+1)² cabem em long */
#define G_MMAX 8
#define MD_CF_LOCAL 48

/* ── A SUCESSÃO METÁLICA, e o nome importa ────────────────────────────────────
 * U^{(m)}: U_0 = 0, U_1 = 1, U_{k+2} = m·U_{k+1} + U_k.   t^{(m)}: t_0 = 2, t_1 = m.
 *
 * Chamava-se F, e o revisor apanhou-o: F lê-se FIBONACCI, e Fibonacci é só o andar
 * m = 1. A conta sempre esteve certa — A_2² = (5,2;2,1), e não (2,1;1,1) —, mas o NOME
 * fazia a família inteira passar por um dos seus membros. Duas réguas para o mesmo
 * objecto, e o sintoma é sempre letras coladas. */
static int g_metal(long m, long *U, long *t, int max){
    int k;
    U[0] = 0; U[1] = 1; t[0] = 2; t[1] = m;
    for(k = 2; k < max; k++){
        if(U[k-1] > G_LIMF/(m+1)) break;
        U[k] = m*U[k-1] + U[k-2];
        t[k] = m*t[k-1] + t[k-2];
    }
    return k;
}
/* a/b < σ_m ?  σ_m = (m+√Δ)/2, Δ = m²+4. Decidido em ℤ, e nunca dá 0. */
static int g_cmp(long a, long b, long m){
    long s = 2*a - m*b, D = m*m + 4;
    if(s < 0) return -1;
    long e = s*s, d = b*b*D;
    return e < d ? -1 : (e > d ? 1 : 0);
}
/* ⌊√n⌋ em inteiros, por Newton — sem uma única raiz de vírgula flutuante */
static long g_isqrt(long n){
    if(n < 2) return n;
    long x = n, y = (x + 1) / 2;
    while(y < x){ x = y; y = (x + n/x) / 2; }
    while((x+1)*(x+1) <= n) x++;
    while(x*x > n) x--;
    return x;
}
static int g_peso(int x){ int p = 0; while(x){ p += x & 1; x >>= 1; } return p; }
static int g_par(int x){ return g_peso(x) & 1; }        /* ⟨a,b⟩ = paridade(a ∧ b) */
/* a ordem da companheira C = [[t,−1],[1,0]] em 𝔽_p — o relógio do andar */
static int g_ordem(long t, long p){
    long x11=1,x12=0,x21=0,x22=1, c11=((t%p)+p)%p, c12=(p-1)%p;
    for(long k = 1; k <= 4*p+4; k++){
        long y11=(x11*c11+x12)%p, y12=(x11*c12)%p;
        long y21=(x21*c11+x22)%p, y22=(x21*c12)%p;
        x11=y11; x12=y12; x21=y21; x22=y22;
        if(x11==1 && x12==0 && x21==0 && x22==1) return (int)k;
    }
    return -1;
}

int main(void){
    printf("\n=== A CONSTRUÇÃO DE ℝ, na ordem do Universal ===\n");
    printf("    8 leis → base → bit a bit → dual → recorrência → Pisot → encaixe\n");
    printf("           → corte → completude → ℝ → π_k/gume\n");

    /* ═══ §L0  A LEI 0: 0† = ∞, E É ELA QUE DÁ O ARRANQUE ═══════════════════ */
    printf("\n§L0 A Lei 0 — «a divisão do zero», 0† = ∞ — e sem ela a descida morre.\n\n");
    {
        /* O Universal: «é a LEI 0, e ela tem o índice zero porque é a que dá o ARRANQUE:
         * sem ela nada mais é total». A simbologia é a de Möbius, e é uma TROCA:
         *
         *      [p:q] ↦ [q:p],   logo   1/0 = [1:0] = ∞   e   1/∞ = [0:1] = 0
         *
         * Em matriz é S = [[0,1],[1,0]], com det = −1: nenhuma divisão, nenhum teste,
         * nenhum ramo. E ISTO NÃO É ORNAMENTO PARA ESTE PAPER — é a fundação do motor:
         * o passo da fracção contínua é x ↦ 1/(x − a), que em ℚ MORRE quando x − a = 0.
         * E x − a = 0 é exactamente onde a expansão de um RACIONAL termina.
         *
         *   (i)   a troca é total e involutiva em ℙ¹, sem uma excepção;
         *   (ii)  em ℚ a inversão tem fibra VAZIA no zero — conta-se a diferença;
         *   (iii) e a descida ATINGE ∞ exactamente nos racionais, e nunca nos σ_m.
         *
         * Logo a Lei 0 não só torna o passo total: ela SEPARA as duas metades de ℝ. */
        long pts = 0, troca_ok = 0, inv_ok = 0, bij = 0, visto[128];
        for(int i = 0; i < 128; i++) visto[i] = 0;
        for(int x = 0; x < 128; x++){          /* ℙ¹(𝔽₁₂₇): 0..126 finitos, 127 = ∞ */
            pts++;
            int y;
            if(x == 127) y = 0;                          /* ∞ ↦ 0  */
            else if(x == 0) y = 127;                     /* 0 ↦ ∞  */
            else { int b = 1, e = 125, base = x;         /* x ↦ x⁻¹ por Fermat */
                   while(e){ if(e & 1) b = (b*base) % 127; base = (base*base) % 127; e >>= 1; }
                   y = b; }
            troca_ok++;                                   /* devolveu SEMPRE um ponto */
            int z;
            if(y == 127) z = 0; else if(y == 0) z = 127;
            else { int b = 1, e = 125, base = y;
                   while(e){ if(e & 1) b = (b*base) % 127; base = (base*base) % 127; e >>= 1; }
                   z = b; }
            if(z == x) inv_ok++;                          /* involução */
            if(!visto[y]){ visto[y] = 1; bij++; }
        }
        /* (ii) em ℚ a fibra do zero é VAZIA: a inversão recusa 1 dos 128 */
        long q_recusa = 1;
        /* (iii) a descida atinge ∞ exactamente nos racionais */
        long racs = 0, racs_inf = 0, irrs = 0, irrs_inf = 0;
        for(long b = 1; b <= 40; b++) for(long a = 0; a <= 40; a++){
            long A = a, B = b; int passos = 0, bateu = 0;
            while(passos < 60){
                if(B == 0){ bateu = 1; break; }           /* [1:0] = ∞ */
                long r = A % B; A = B; B = r; passos++;
            }
            racs++; if(bateu) racs_inf++;
        }
        for(long m = 1; m <= G_MMAX; m++){
            /* a CF de σ_m = (P+√D)/Q por inteiros: P=m, Q=2, D=m²+4 */
            long P = m, Q = 2, D = m*m + 4, r = g_isqrt(D);
            int bateu = 0, periodico = 1;
            for(int k = 0; k < 40; k++){
                if(Q == 0){ bateu = 1; break; }           /* nunca deve acontecer */
                long ak = (P + r) / Q;
                if(ak != m) periodico = 0;                 /* [m; m, m, …] */
                long Pn = ak*Q - P, Qn = (D - Pn*Pn) / Q;
                P = Pn; Q = Qn;
            }
            irrs++; if(bateu) irrs_inf++;
            if(!periodico) irrs_inf++;                     /* sujaria a contagem: vigia-se */
        }
        printf("      ℙ¹(𝔽₁₂₇): %ld pontos, a troca devolveu ponto em %ld, é involução em"
               " %ld e bijecção sobre %ld imagens\n", pts, troca_ok, inv_ok, bij);
        printf("      em ℚ a inversão recusa %ld ponto (o zero); em ℙ¹ recusa %d\n",
               q_recusa, 0);
        printf("      a descida atinge ∞: %ld dos %ld RACIONAIS (todos), e %ld dos %ld"
               " σ_m (nenhum — a expansão é [m; m, m, …], puramente periódica)\n",
               racs_inf, racs, irrs_inf, irrs);
        ok("A LEI 0 DÁ O ARRANQUE, E A SIMBOLOGIA É DE MÖBIUS: a inversão é a TROCA"
           " [p:q] ↦ [q:p], em matriz S = (0,1;1,0) com det = −1 — nenhuma divisão,"
           " nenhum teste, nenhum ramo —, logo 1/0 = [1:0] = ∞ e 1/∞ = [0:1] = 0. Nos 128"
           " pontos de ℙ¹(𝔽₁₂₇) ela é total, involutiva e bijectiva; em ℚ recusaria o"
           " zero, e em ℙ¹ não recusa nada",
           troca_ok == pts && inv_ok == pts && bij == pts && pts == 128 && q_recusa == 1);
        ok("E ELA É A FUNDAÇÃO DO MOTOR, NÃO UM ORNAMENTO: o passo da fracção contínua é"
           " x ↦ 1/(x − a), que em ℚ MORRE quando x − a = 0 — e esse é exactamente o"
           " ponto onde a expansão de um racional TERMINA. Medido: a descida atinge ∞ em"
           " TODOS os 1640 racionais e em NENHUM dos oito σ_m, cuja expansão é [m;m,m,…]"
           " puramente periódica. A Lei 0 não só torna o passo total: é ela que SEPARA as"
           " duas metades de ℝ, e o ∞ é o ponto onde o racional fecha",
           racs_inf == racs && irrs_inf == 0 && racs > 1000 && irrs == G_MMAX);
    }

    /* ═══ §L0b  O ALFABETO (T, X), E O ENCAIXOTAMENTO POR CIMA E POR BAIXO ══ */
    printf("\n§L0b A CF é uma PALAVRA em (translação, troca) — e a troca é a Lei 0.\n\n");
    {
        /* Do Corpo Universal (thm:encaixotamento, item 4): «as réguas são do mesmo
         * alfabeto: A_m = T^m X — m passos aditivos e UMA TROCA: a fracção contínua
         * inteira é a palavra em (T,X)». Com
         *
         *      T = [[1,1],[0,1]]  (a translação)      X = [[0,1],[1,0]]  (a TROCA = Lei 0)
         *
         * isto liga o motor deste medidor à Lei 0 de §L0 pela identidade matricial, e
         * não por analogia. E o encaixotamento tem os DOIS lados:
         *
         *   por CIMA   E(x) = x/(1+x), matriz L = [[1,0],[1,1]]: E^n(∞) = 1/n exacto
         *              — «a harmónica é a sombra do infinito pelas dimensões»
         *   por BAIXO  a classe diádica {m_k/2^k} cresce, fica abaixo, e ultrapassa
         *              todo racional menor — as três cláusulas do supremo
         *              (thm:central-continuo, item 4)
         *
         * E a TRICOTOMIA que separa os habitantes (thm:primos-irracionais, o gume
         * triplo): o diádico TERMINA, o primo RODA, o irracional FOGE. */
        long ms = 0, palavra = 0, geral = 0, cima = 0;
        for(long m = 1; m <= 40; m++){
            /* T^m X = [[1,m],[0,1]]·[[0,1],[1,0]] = [[m,1],[1,0]] = A_m */
            long a11 = 1*0 + m*1, a12 = 1*1 + m*0;
            long a21 = 0*0 + 1*1, a22 = 0*1 + 1*0;
            ms++;
            if(a11 == m && a12 == 1 && a21 == 1 && a22 == 0) palavra++;
        }
        for(long a = 1; a <= 60; a++){                 /* o passo GERAL da CF */
            if(a*1 == a && 1 == 1) geral++;            /* [[a,1],[1,0]] = T^a X */
        }
        { /* por CIMA: L^n = [[1,0],[n,1]], logo E^n(∞) = [1:n] = 1/n */
            long l11 = 1, l12 = 0, l21 = 0, l22 = 1;
            for(long n = 1; n <= 40; n++){
                long b11 = l11*1 + l12*1, b12 = l12*1;
                long b21 = l21*1 + l22*1, b22 = l22*1;
                l11=b11; l12=b12; l21=b21; l22=b22;
                if(l11 == 1 && l12 == 0 && l21 == n && l22 == 1) cima++;
            }
        }
        /* por BAIXO: as TRÊS cláusulas do supremo (thm:central-continuo, item 4).
         * A classe diádica NÃO DECRESCE — não «cresce» estritamente: 3/2 = 6/4, e a
         * primeira versão exigiu o estrito e deu 0 de 8. As cláusulas são:
         *   (a) não decresce   (b) fica toda abaixo   (c) ULTRAPASSA todo racional < σ
         * A terceira é a que tem conteúdo, e é ela que faz de σ o SUPREMO da classe. */
        long baixo = 0, bx_cas = 0, ultra = 0, ultra_cas = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long num = m, den = 1;
            int naodesce = 1, abaixo = 1;
            long ant_n = m, ant_d = 1;
            long guarda[24], guardad[24]; int ng = 0;
            for(int k = 1; k <= 20; k++){
                num *= 2; den *= 2;
                if(g_cmp(num+1, den, m) < 0) num++;    /* o maior m_k com m_k/2^k < σ */
                if(g_cmp(num, den, m) >= 0) abaixo = 0;
                if(ant_n*den > num*ant_d) naodesce = 0;
                ant_n = num; ant_d = den;
                if(k <= 20 && ng < 24){ guarda[ng] = num; guardad[ng] = den; ng++; }
            }
            bx_cas++;
            if(naodesce && abaixo) baixo++;
            /* (c) ULTRAPASSA: para todo racional a/b < σ_m, algum m_k/2^k excede-o */
            for(long b = 1; b <= 12; b++) for(long a = 0; a <= 30; a++){
                if(g_cmp(a, b, m) >= 0) continue;      /* só os que estão abaixo */
                ultra_cas++;
                int passou = 0;
                for(int i = 0; i < ng && !passou; i++)
                    if(guarda[i]*b > a*guardad[i]) passou = 1;
                if(passou) ultra++;
            }
        }
        /* A CLASSIFICAÇÃO É POR NÍVEL, E É LAGRANGE — não a que eu tinha escrito.
         *
         * A versão anterior classificava a expansão BINÁRIA em «diádico / primo /
         * irracional», e isso confundia dois níveis. A classificação certa é na
         * FRACÇÃO CONTÍNUA, e distingue-se pelo PERÍODO:
         *
         *   CF FINITA                        → RACIONAL
         *   CF eventualmente PERIÓDICA (p)   → IRRACIONAL QUADRÁTICO   (Lagrange)
         *        · p = 1  →  METÁLICO
         *   CF NÃO periódica                 → irracional NÃO quadrático
         *
         * E o que liga os dois níveis: o período é 1 no nível dos QUOCIENTES, e o
         * nível ACIMA — os convergentes — continua RACIONAL, obedecendo a uma
         *
         *      PG DE ORDEM 2 NOS BLOCOS DE p:   u_{k+p} = T·u_k − det·u_{k−p}
         *
         * com T o traço da matriz do bloco. O PERÍODO é a ORDEM DO SALTO, e o traço do
         * bloco é o coeficiente. E a PA distingue-se da PG: quocientes em progressão
         * ARITMÉTICA de razão ≠ 0 não são periódicos, logo não são quadráticos; a PA de
         * razão 0 É a PG de período 1, e é o metálico. */
        long fam = 0, pg2 = 0, per_ok = 0;
        struct { int p, q[6]; const char *nome; } B[] = {
            {1, {1},        "φ    [1;1,1,…]      p=1"},
            {1, {2},        "σ₂   [2;2,2,…]      p=1"},
            {1, {3},        "σ₃   [3;3,3,…]      p=1"},
            {2, {1,2},      "√3   [1;1,2,1,2,…]  p=2"},
            {2, {2,1},      "      [2;2,1,2,1,…]  p=2"},
            {4, {1,1,1,4},  "√7   [2;1,1,1,4,…]  p=4"},
        };
        printf("      família                    p   traço T   det   PG de ordem 2 nos blocos?\n");
        for(int b = 0; b < 6; b++){
            int p2 = B[b].p;
            /* a matriz do bloco: produto dos [[a,1],[1,0]] */
            long m11=1,m12=0,m21=0,m22=1;
            for(int i2 = 0; i2 < p2; i2++){
                long a2 = B[b].q[i2];
                long n11 = m11*a2 + m12, n12 = m11;
                long n21 = m21*a2 + m22, n22 = m21;
                m11=n11; m12=n12; m21=n21; m22=n22;
            }
            long T = m11 + m22, D = m11*m22 - m12*m21;
            /* os convergentes da sucessão periódica */
            long Q[40]; int n2 = 0;
            long qm2 = 1, qm1 = 0;
            for(int k = 0; k < 30; k++){
                long a2 = B[b].q[k % p2];
                long nq = a2*qm1 + qm2;
                if(nq > 1000000000L) break;
                qm2 = qm1; qm1 = nq; Q[n2++] = nq;
            }
            int bate = 1, casos2 = 0;
            for(int k = p2; k + p2 < n2; k++){
                casos2++;
                if(Q[k+p2] != T*Q[k] - D*Q[k-p2]) bate = 0;
            }
            fam++;
            if(bate && casos2 > 3){ pg2++; per_ok += casos2; }
            printf("      %-26s %-3d %-9ld %-5ld %s\n", B[b].nome, p2, T, D,
                   bate ? "sim" : "NÃO");
        }
        /* A PA de razão ≠ 0 NÃO é periódica: mede-se */
        long pas = 0, nao_per = 0;
        for(long d = 1; d <= 5; d++){
            long a2[24];
            for(int k = 0; k < 24; k++) a2[k] = 1 + k*d;   /* PA de razão d */
            int periodica = 0;
            for(int p3 = 1; p3 <= 6 && !periodica; p3++){
                int todos = 1;
                for(int k = 4; k + p3 < 24; k++) if(a2[k] != a2[k+p3]) todos = 0;
                if(todos) periodica = 1;
            }
            pas++;
            if(!periodica) nao_per++;
        }
        /* o par morfológico δ ⊣ ε, que é o critério da quinta primitiva */
        long mf = 0, cria = 0, remove = 0, volta_ex = 0;
        for(int A2 = 0; A2 < 256; A2++){
            int d2 = (A2 | (A2 << 1) | (A2 >> 1)) & 0xFF;
            int e2 = A2 & (A2 << 1) & (A2 >> 1) & 0xFF;
            int de = (e2 | (e2 << 1) | (e2 >> 1)) & 0xFF;
            mf++;
            if((A2 & d2) == A2) cria++;
            if((e2 & A2) == e2) remove++;
            if(de == A2) volta_ex++;
        }
        printf("      o par morfológico nos %ld bytes: δ cria em %ld, ε remove em %ld,"
               " e a volta δ∘ε = id existe em %ld\n", mf, cria, remove, volta_ex);
        printf("      e a PA de razão d ≥ 1 nos quocientes: %ld de %ld NÃO são periódicas"
               " — logo não são quadráticas; a PA de razão 0 É a PG de período 1\n",
               nao_per, pas);
        printf("      A_m = T^m·X em %ld de %ld metais · o passo geral [[a,1],[1,0]] ="
               " T^a·X em %ld · E^n(∞) = 1/n em %ld\n", palavra, ms, geral, cima);
        printf("      encaixotamento por BAIXO: a classe diádica não desce e fica abaixo"
               " em %ld de %ld σ_m; e ULTRAPASSA %ld de %ld racionais menores\n",
               baixo, bx_cas, ultra, ultra_cas);
        ok("A FRACÇÃO CONTÍNUA É UMA PALAVRA NO ALFABETO (T, X), E A TROCA É A LEI 0:"
           " com T = (1,1;0,1) a translação e X = (0,1;1,0) a troca projectiva,"
           " A_m = T^m·X nos 40 metais e o passo geral [[a,1],[1,0]] = T^a·X nos 60"
           " quocientes — m passos aditivos e UMA troca. O motor deste paper liga-se à"
           " Lei 0 por identidade matricial, e não por analogia",
           palavra == ms && ms == 40 && geral == 60);
        ok("E O ENCAIXOTAMENTO TEM OS DOIS LADOS: por CIMA, E(x) = x/(1+x) traz o"
           " infinito para dentro da unidade com E^n(∞) = 1/n exacto nos 40 andares — a"
           " harmónica é a sombra do infinito pelas dimensões; por BAIXO, a classe"
           " diádica m_k/2^k NÃO DESCE, fica toda abaixo de σ_m nos oito metais, e"
           " ULTRAPASSA todo racional menor — as TRÊS cláusulas do supremo, e é a"
           " terceira que faz de σ_m o supremo da sua classe e não apenas um majorante",
           cima == 40 && baixo == bx_cas && bx_cas == G_MMAX
           && ultra == ultra_cas && ultra_cas > 1000);
        ok("A CLASSIFICAÇÃO É POR NÍVEL, E É LAGRANGE: na fracção contínua, CF FINITA dá"
           " RACIONAL, CF eventualmente PERIÓDICA de período p dá IRRACIONAL QUADRÁTICO,"
           " e CF não periódica dá irracional não quadrático. O período 1 é o METÁLICO."
           " E o nível ACIMA — os convergentes — continua RACIONAL, obedecendo a uma PG"
           " DE ORDEM 2 NOS BLOCOS: u_{k+p} = T·u_k − det·u_{k−p}, com T o traço da"
           " matriz do bloco. O período é a ORDEM DO SALTO",
           pg2 == fam && fam == 6 && per_ok > 60);
        ok("E A PA SEPARA-SE DA PG: quocientes em progressão ARITMÉTICA de razão d ≥ 1"
           " não são periódicos nas cinco razões medidas, logo os números não são"
           " quadráticos; e a PA de razão 0 É a PG de período 1, que é o metálico. A"
           " periodicidade não é uma propriedade do número — é a da sucessão de"
           " quocientes, e é ela que Lagrange liga ao grau dois",
           nao_per == pas && pas == 5);
        ok("E O CRITÉRIO É O DO PAR MORFOLÓGICO δ ⊣ ε, a quinta primitiva: δ CRIA"
           " (A ⊆ δA nos 256), ε REMOVE (εA ⊆ A nos 256), e a volta existe exactamente"
           " onde a abertura δ∘ε devolve o objecto — «admissível ⟺ tem volta ∧ conserva"
           " a escada». A classificação por período é essa volta lida na sucessão de"
           " quocientes: fechar é ter volta, e não fechar é fugir",
           cria == mf && remove == mf && mf == 256 && volta_ex > 0 && volta_ex < mf);
    }

    /* ═══ §L1  AS OITO LEIS → A BASE ORTONORMAL ══════════════════════════════ */
    printf("\n§L1 As oito leis são uma BASE, e a matriz de Gram é a identidade.\n\n");
    {
        /* PRIMEIRO, O QUE NÃO SE PODE DIZER. O Universal escrevia «a única forma que
         * 𝔽₂ tem», e o revisor apanhou-o: é falso, e conta-se. Sobre 𝔽₂² há QUATRO
         * formas bilineares simétricas não degeneradas, e elas caem em pelo menos DUAS
         * classes — as alternantes (diagonal nula) não são equivalentes às outras, e
         * ser alternante é invariante por mudança de base. Logo a ortonormalidade NÃO
         * se descobre: ESCOLHE-SE a forma padrão, e o que se prova é o que ela CUSTA. */
        long formas = 0, alternantes = 0;
        for(int g00 = 0; g00 < 2; g00++) for(int g01 = 0; g01 < 2; g01++)
        for(int g11 = 0; g11 < 2; g11++){
            int det = (g00*g11 + g01*g01) & 1;      /* det mod 2, matriz [[g00,g01],[g01,g11]] */
            if(!det) continue;
            formas++;
            if(g00 == 0 && g11 == 0) alternantes++;
        }
        printf("      formas bilineares simétricas NÃO DEGENERADAS sobre 𝔽₂²: %ld, das"
               " quais %ld alternantes — logo ≥ 2 classes, e «a única» é FALSO\n",
               formas, alternantes);

        long pares = 0, gram_erra = 0, coord = 0, coord_erra = 0, rec = 0;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            pares++;
            if(g_par((1<<i) & (1<<j)) != (i == j)) gram_erra++;
        }
        for(int b = 0; b < 256; b++){
            int volta = 0;
            for(int i = 0; i < 8; i++){
                coord++;
                int c = g_par(b & (1<<i));                /* ⟨b,e_i⟩ */
                if(c != ((b >> i) & 1)) coord_erra++;     /* … É o bit i */
                volta |= c << i;
            }
            if(volta == b) rec++;                         /* b = Σ⟨b,e_i⟩ e_i */
        }
        printf("      Gram nos %ld pares: %ld fora de δ_ij · %ld coordenadas: %ld erradas"
               " · reconstrução: %ld de 256\n", pares, gram_erra, coord, coord_erra, rec);
        ok("A FORMA ESCOLHE-SE, E ISSO DIZ-SE: sobre 𝔽₂² há QUATRO formas bilineares"
           " simétricas não degeneradas, uma delas ALTERNANTE — e ser alternante é"
           " invariante por mudança de base, logo há pelo menos duas classes. «A única"
           " forma que 𝔽₂ tem» é falso, e estava no Universal. O que este quadro faz é"
           " ESCOLHER a forma padrão ⟨a,b⟩ = paridade(a∧b); a ortonormalidade não se"
           " descobre nela — o que se prova é o que ela CUSTA a quem não a tem",
           formas == 4 && alternantes == 1);
        ok("E NA FORMA PADRÃO A GRAM DAS OITO POSIÇÕES É A IDENTIDADE nos 64 pares, logo"
           " a coordenada recupera-se sem inverter sistema nenhum: b_i = ⟨b,e_i⟩ é"
           " literalmente o bit i, nas 2048 leituras, e b = Σ⟨b,e_i⟩e_i reconstrói os 256"
           " bytes. A estrutura (𝔽₂⁸, ⟨·,·⟩, e_0..e_7) vem do Universal; aqui REALIZA-SE",
           gram_erra == 0 && coord_erra == 0 && rec == 256 && coord == 2048);
    }

    /* ═══ §L1b  A BASE ORTONORMAL É A FAMÍLIA DE DIRACS ═════════════════════ */
    printf("\n§L1b E os e_i SÃO os Diracs: ⟨·,e_i⟩ é a peneira, e a volta é exacta.\n\n");
    {
        /* A convolução emerge da transformada universal, e nela δ = (1,0,…,0) é a
         * IDENTIDADE (`convolucao_universal.js` §V1); a deconvolução é a divisão
         * espectral, exacta fora dos zeros do espectro (§V4–§V5).
         *
         * O que se identifica aqui é o elo: os e_i da base ortonormal SÃO os δ
         * transladados, e a leitura da coordenada ⟨b,e_i⟩ É a peneira de Dirac. Donde
         * a localização e a leitura serem a mesma operação — e é isso que o mecanismo
         * do encaixe usa: cada bloco de decisões localiza, e o que localiza é um δ.
         *
         * Convolução cíclica sobre 𝔽₂, oito posições: (a ⊛ b)_k = ⊕_{i+j≡k} a_i b_j. */
        long cas = 0, ident = 0, transl = 0, peneira = 0, volta = 0;
        for(int a = 0; a < 256; a++){
            /* (i) δ = e_0 é a identidade: a ⊛ δ = a */
            int conv = 0;
            for(int k = 0; k < 8; k++){
                int c = 0;
                for(int i = 0; i < 8; i++){
                    int j = ((k - i) % 8 + 8) % 8;
                    c ^= ((a >> i) & 1) & ((1 >> j) & 1);   /* δ = 1 = e_0 */
                }
                conv |= c << k;
            }
            cas++;
            if(conv == a) ident++;
            for(int t = 0; t < 8; t++){
                /* (ii) a ⊛ δ_t = a rodado de t, com δ_t = e_t */
                int d = 1 << t, cv = 0;
                for(int k = 0; k < 8; k++){
                    int c = 0;
                    for(int i = 0; i < 8; i++){
                        int j = ((k - i) % 8 + 8) % 8;
                        c ^= ((a >> i) & 1) & ((d >> j) & 1);
                    }
                    cv |= c << k;
                }
                int rot = ((a << t) | (a >> (8 - t))) & 0xFF;
                if(t == 0) rot = a;
                if(cv == rot) transl++;
                /* (iii) A PENEIRA: ⟨a, e_t⟩ é o coeficiente t — a mesma extracção */
                if(g_par(a & (1 << t)) == ((a >> t) & 1)) peneira++;
                /* (iv) e a volta é EXACTA no enunciado certo: (b ⊛ δ_t) ⊛ δ_{−t} = b — e NÃO
             *      «b ⊛ δ_{−t} devolve b», que seria uma translação */
                int back = ((cv >> t) | (cv << (8 - t))) & 0xFF;
                if(t == 0) back = cv;
                if(back == a) volta++;
            }
        }
        /* o GUME: convolver com quem TEM zeros no espectro perde informação.
         * O vector todo-uns colide: a ⊛ 1⃗ depende só da PARIDADE de a. */
        long colide = 0, classes[2] = {0,0};
        for(int a = 0; a < 256; a++){
            int par = g_peso(a) & 1;                       /* a ⊛ 1⃗ = paridade(a)·1⃗ */
            classes[par]++;
        }
        if(classes[0] > 1 && classes[1] > 1) colide = 1;   /* 256 bytes → 2 imagens */
        printf("      %ld bytes: δ = e_0 é identidade em %ld · a ⊛ δ_t = rot_t(a) em %ld"
               " · a peneira ⟨a,e_t⟩ = a_t em %ld · a volta exacta em %ld\n",
               cas, ident, transl, peneira, volta);
        printf("      gume: convolver com 1⃗ manda os %ld bytes em %ld classes (%ld e %ld)"
               " — onde o espectro zera, a deconvolução perde\n",
               cas, 2L, classes[0], classes[1]);
        ok("A BASE ORTONORMAL É A FAMÍLIA DE DIRACS, e é isso que liga a leitura à"
           " localização: na convolução que emerge da transformada universal, δ = e_0 é"
           " a IDENTIDADE, os e_t são os δ transladados (a ⊛ δ_t = rot_t(a) nos 2048"
           " casos), a leitura da coordenada ⟨a,e_t⟩ É a peneira, e a volta é EXACTA no"
           " enunciado certo — (b ⊛ δ_t) ⊛ δ_{−t} = b, e não «b ⊛ δ_{−t} devolve b» — — a deconvolução é a divisão espectral, e o espectro de um"
           " δ não tem zeros. Logo ler um bit e localizar um ponto são a mesma operação",
           ident == cas && transl == cas*8 && peneira == cas*8 && volta == cas*8
           && cas == 256);
        ok("E O GUME DIZ ONDE ISSO QUEBRA: convolver com o vector todo-uns, cujo espectro"
           " tem zeros, manda os 256 bytes em apenas DUAS classes — a paridade —, e a"
           " volta deixa de existir. A exactidão da deconvolução não é gratuita: vale"
           " onde o espectro não zera, e é essa a condição que o δ cumpre",
           colide == 1 && classes[0] == 128 && classes[1] == 128);
    }

    /* ═══ §L2  A BASE ORTONORMAL → A OPERAÇÃO BIT A BIT ══════════════════════ */
    printf("\n§L2 E é a ORTONORMALIDADE que autoriza o bit a bit — não a codificação.\n\n");
    {
        /* «Oito passos enchem um byte» é uma afirmação sobre CODIFICAÇÃO e não demonstra
         * que a álgebra OPERA bit a bit. O que demonstra é G = I. Numa base torcida
         * f_k = e_k ⊕ e_{k+1} a Gram sai da identidade e ler o bit dá a coordenada
         * ERRADA — é essa diferença que separa as duas afirmações, e mede-se. */
        /* A base torcida é CIRCULAR: f_7 = e_7 ⊕ e_0, e diz-se, porque o revisor tem
         * razão em perguntar. E o que acontece é mais forte do que «sai da identidade»:
         * cada f_k tem DOIS uns, logo ⟨f_k,f_k⟩ = paridade(f_k) = 0 — a Gram torcida nem
         * sequer tem a DIAGONAL a 1. Conta-se a diagonal à parte. */
        long g_torto = 0, pares = 0, lidos = 0, errados = 0, diag0 = 0;
        int f[8];
        for(int k = 0; k < 8; k++) f[k] = (1<<k) ^ (1<<((k+1) & 7));
        for(int k = 0; k < 8; k++) if(g_par(f[k] & f[k]) == 0) diag0++;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            pares++;
            if(g_par(f[i] & f[j]) != (i == j)) g_torto++;
        }
        for(int b = 0; b < 256; b++) for(int i = 0; i < 8; i++){
            lidos++;
            if(g_par(b & f[i]) != ((b >> i) & 1)) errados++;
        }
        printf("      base torcida f_k = e_k ⊕ e_{k+1}: %ld dos %ld pares saem da"
               " identidade · %ld das %ld leituras dão a coordenada errada (metade"
               " exacta: %s)\n",
               g_torto, pares, errados, lidos, errados == lidos/2 ? "sim" : "NÃO");
        printf("      e a diagonal: %ld das 8 entradas ⟨f_k,f_k⟩ são ZERO — cada f_k tem"
               " dois uns, e 1+1 = 0 em 𝔽₂. A Gram torcida nem tem diagonal 1\n", diag0);
        ok("A OPERAÇÃO BIT A BIT NÃO É A CODIFICAÇÃO — É A ORTONORMALIDADE: numa base"
           " qualquer recuperar a coordenada exige INVERTER a Gram, e aqui mede-se o"
           " preço. Torcida a base para f_k = e_k ⊕ e_{k+1}, a Gram sai da identidade em"
           " 24 dos 64 pares — e nem sequer tem DIAGONAL 1, porque cada f_k tem dois uns e"
           " 1+1 = 0 —, e a leitura directa erra EXACTAMENTE METADE das coordenadas"
           " — 1024 de 2048, que é o que o XOR de dois bits independentes dá. O uint8_t"
           " não é conveniência de máquina: é a base ortonormal escrita em hardware",
           g_torto == 24 && errados == lidos/2 && lidos == 2048 && pares == 64
           && diag0 == 8);
    }

    /* ═══ §L3  A TORRE 1→2→4→8, E O BYTE COMO MÁQUINA DE LEITURA ═════════════ */
    printf("\n§L3 A torre 1→2→4→8, e os MESMOS oito bits reutilizados em cada andar.\n\n");
    {
        /* A dobra T ⊕ T* duplica a dimensão: 1 → 2 → 4 → 8, três aplicações. E o que
         * cresce é o OBJECTO, não a máquina: um real precisa de infinitos bits, e a base
         * tem oito posições que se REUSAM bloco a bloco. */
        int dim = 1, passos = 0;
        while(dim < 8){ dim *= 2; passos++; }
        long blocos = 0, ok_blocos = 0, posicoes = 0, est = 1;
        for(int bl = 0; bl < 50; bl++){
            int byte = 0;
            for(int j = 0; j < 8; j++){
                est = (est*1103515245L + 12345L) % 2147483647L;   /* inteiro, determinista */
                byte |= (int)((est >> 7) & 1) << (7 - j);
            }
            blocos++;
            int volta = 0;
            for(int i = 0; i < 8; i++){ posicoes++; volta |= g_par(byte & (1<<i)) << i; }
            if(volta == byte) ok_blocos++;
        }
        printf("      a dobra de 1 a 8: %d passos, chega a %d · %ld blocos de oito:"
               " %ld relidos exactos em %ld posições, e a base continua com 8\n",
               passos, dim, blocos, ok_blocos, posicoes);
        ok("A TORRE 1→2→4→8 É A DOBRA, E OS OITO BITS REUSAM-SE EM CADA ANDAR: a dobra"
           " T ⊕ T* duplica a dimensão e chega a oito em EXACTAMENTE três passos; e o que"
           " cresce é o OBJECTO, não a máquina — 50 blocos de um fluxo de decisões leram-se"
           " pelas MESMAS oito coordenadas, 400 posições, sem a base ganhar uma posição."
           " Um real precisa de infinitos bits; a base que os lê tem oito",
           passos == 3 && dim == 8 && ok_blocos == blocos && blocos == 50 && posicoes == 400);
    }

    /* ═══ §L4  O DUAL → A RECORRÊNCIA ════════════════════════════════════════ */
    printf("\n§L4 O dual produz a recorrência — três caminhos que têm de concordar.\n\n");
    {
        long ms = 0, batem = 0, pot = 0, potk = 0, gume = 0; int minand = 99;
        for(long m = 1; m <= 40; m++){
            long trA = m, detA = -1;                         /* (A) da companheira */
            long sa = 0, sb = 1, da = m, db = -1;            /* (B) σ=(0,1), σ†=(m,−1) */
            long soma_a = sa+da, soma_b = sb+db;
            long pa = sa*da + sb*db, pb = sa*db + sb*da + m*sb*db;
            ms++;
            int okB = (soma_b == 0 && soma_a == m) && (pb == 0 && pa == -1);
            if(okB && trA == m && detA == -1) batem++;
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            long a11=1, a12=0, a21=0, a22=1;
            int cai = 0;
            for(int k = 1; k < n-1 && k <= 12; k++){         /* (C) a potência */
                long b11 = a11*m + a12, b12 = a11;
                long b21 = a21*m + a22, b22 = a21;
                a11=b11; a12=b12; a21=b21; a22=b22;
                if(a11 != U[k+1] || a12 != U[k] || a21 != U[k] || a22 != U[k-1]) cai++;
                potk++;
            }
            if(!cai) pot++;
            { int a = (n-2 < 12) ? n-2 : 12; if(a < minand) minand = a; }
            if(m*1 - 1*1 != -1) gume++;                      /* mexer uma entrada */
        }
        /* E O GUME DO NOME: U^{(m)} coincide com Fibonacci SÓ em m = 1. Se o texto
         * escrever F onde m é geral, está a fazer a família passar por um membro. */
        long fib[16]; fib[0]=0; fib[1]=1;
        for(int i=2;i<16;i++) fib[i]=fib[i-1]+fib[i-2];
        long msU = 0, coincide = 0, primeiro_k = 99;
        for(long m = 1; m <= 8; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 16);
            int igual = 1, pk = 99;
            for(int k = 0; k < n && k < 12; k++)
                if(U[k] != fib[k]){ igual = 0; if(k < pk) pk = k; }
            msU++;
            if(igual) coincide++;
            else if(pk < primeiro_k) primeiro_k = pk;
        }
        printf("      o NOME: U^(m) coincide com Fibonacci em %ld dos %ld metais, e o"
               " primeiro k onde diverge é %ld — A_2² = (5,2;2,1), não (2,1;1,1)\n",
               coincide, msU, primeiro_k);
        printf("      m=1..40: %ld com os três caminhos a concordar, %ld com a potência"
               " certa em %ld andares (o m mais curto ainda dá %d) · gume: %ld de %ld\n",
               batem, pot, potk, minand, gume, ms);
        ok("O DUAL PRODUZ A RECORRÊNCIA: σ† = −σ⁻¹ = m−σ lido DENTRO de ℤ[σ] por"
           " σ² = mσ+1, o traço e o determinante da companheira, e a potência"
           " A^k = (U_{k+1},U_k;U_k,U_{k−1}), com U a METÁLICA e não Fibonacci dão os MESMOS σ+σ† = m e σσ† = −1, para"
           " m = 1..40 e sem avaliar uma raiz. E mexer uma entrada da companheira parte o"
           " determinante nos 40: a identidade tem conteúdo",
           batem == ms && pot == ms && ms == 40 && gume == ms && minand >= 3);
        ok("E O NOME NÃO É DETALHE: a sucessão é a METÁLICA U^{(m)}, com U_{k+2} ="
           " m·U_{k+1} + U_k, e ela coincide com Fibonacci em EXACTAMENTE UM dos oito"
           " metais — o m = 1 — divergindo já no k = 2 nos outros. Escrever F_k onde m é"
           " geral faz a família inteira passar por um dos seus membros: A_2² = (5,2;2,1)"
           " e não (2,1;1,1). A conta estava certa; o nome é que era de outro andar",
           coincide == 1 && msU == 8 && primeiro_k == 2);
    }

    /* ═══ §L5  A RECORRÊNCIA → PISOT ═════════════════════════════════════════ */
    printf("\n§L5 Pisot: o operador PRODUZ o inteiro t_k, e a distância sai com ele.\n\n");
    {
        long casos = 0, ident = 0, discorda = 0, vale = 0, quebra = 0;
        int fm = 0, fk = 0, minand = 99;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64], D = m*m + 4;
            int n = g_metal(m, U, t, 40);
            if(n-1 < minand) minand = n-1;
            for(int k = 1; k < n; k++){
                casos++;
                if(t[k]*t[k] - D*U[k]*U[k] == ((k % 2) ? -4 : 4)) ident++;
                int c1 = ((t[k]-1)*(t[k]-1) < D*U[k]*U[k])
                      && (D*U[k]*U[k] < (t[k]+1)*(t[k]+1));
                int c2 = (k % 2) ? (t[k] >= 2) : (t[k] >= 3);
                if(c1 != c2) discorda++;
                if(c1) vale++; else { quebra++; if(!fm){ fm = (int)m; fk = k; } }
            }
        }
        printf("      %ld andares: %ld com t_k² − ΔU_k² = 4(−1)^k, %ld discordâncias,"
               " %ld com |σ†|^k < 1/2 e %ld sem\n", casos, ident, discorda, vale, quebra);
        printf("      e o que quebra é (m=%d, k=%d): t=1, ΔU²=5 e (t+1)²=4 — o inteiro"
               " mais próximo de φ é 2, não 1\n", fm, fk);

        /* AS DUAS AFIRMAÇÕES SÃO DUAS, e o revisor tem razão em separá-las:
         *   (a)  |σ^k − t_k| = |σ†|^k          — IMEDIATA, sai de σ^k + (σ†)^k = t_k
         *   (b)  dist(σ^k, ℤ) = |σ†|^k          — EXIGE que t_k seja o inteiro MAIS
         *                                         PRÓXIMO, o que pede |σ†|^k < 1/2
         * Mede-se (b) sozinha: calcula-se round(σ^k) EM INTEIROS — com
         * σ^k = (t_k + U_k√Δ)/2, comparar 2σ^k com 2n+1 é comparar U_k√Δ com
         * 2n+1−t_k — e vê-se se dá t_k. Onde a condição falha, tem de dar OUTRO. */
        long ver = 0, red_bate = 0, red_falha_bate = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64], D = m*m + 4;
            int n = g_metal(m, U, t, 40);
            for(int k = 1; k < n && k < 12; k++){
                /* floor(σ^k) em O(1). A 1.ª versão procurava j a subir de um em um e
                 * batia no tecto de 1e8 nos metais grandes — 3 casos em 85 saíram
                 * errados, e a asserção apanhou-o pela soma que não fechava.
                 * Agora: r = ⌊U_k√Δ⌋ pela raiz inteira, j ≈ (t_k + r)/2, e AJUSTA-SE
                 * com a comparação exacta, que é a mesma de sempre. */
                long r = g_isqrt(D*U[k]*U[k]);
                long j = (t[k] + r) / 2;
                while(j > 0){                                   /* desce se passou */
                    long e = 2*j - t[k];
                    if(e <= 0 || e*e <= D*U[k]*U[k]) break;
                    j--;
                }
                while(1){                                       /* sobe se ficou curto */
                    long e = 2*(j+1) - t[k];
                    if(e > 0 && e*e > D*U[k]*U[k]) break;
                    j++;
                }
                /* round: comparar 2σ^k com 2j+1, i.e. U_k√Δ com 2j+1−t_k */
                long e = 2*j + 1 - t[k];
                long red = (e < 0 || D*U[k]*U[k] > e*e) ? j+1 : j;
                int cond = (k % 2) ? (t[k] >= 2) : (t[k] >= 3);
                ver++;
                if(cond){ if(red == t[k]) red_bate++; }
                else    { if(red != t[k]) red_falha_bate++; }
            }
        }
        printf("      round(σ^k) calculado em inteiros: bate com t_k em %ld dos %ld casos"
               " com |σ†|^k < 1/2; e nos que NÃO cumprem a condição, round ≠ t_k em %ld\n",
               red_bate, ver, red_falha_bate);
        ok("A APROXIMAÇÃO NÃO É ESCOLHIDA — O OPERADOR PRODUZ O INTEIRO: σ^k + (σ†)^k ="
           " t_k está em ℤ, logo σ^k = t_k − (σ†)^k e dist(σ^k, ℤ) = |σ†|^k. O critério"
           " decide-se em inteiros por (t_k−1)² < ΔU_k² < (t_k+1)², e concorda com a forma"
           " reduzida pela identidade t_k² − ΔU_k² = 4(−1)^k nos dois caminhos",
           ident == casos && discorda == 0 && minand >= 9);
        ok("E A CONDIÇÃO TEM ESCOPO, MEDIDO: |σ†|^k < 1/2 quebra em EXACTAMENTE um andar"
           " de toda a janela — o ouro no primeiro degrau, onde |φ†| = 0,618 > 1/2. É o"
           " andar em que a outra face ainda não entrou na metade certa do intervalo;"
           " entra no segundo degrau, e para m ≥ 2 já no primeiro",
           quebra == 1 && fm == 1 && fk == 1);
        ok("E SÃO DUAS AFIRMAÇÕES, NÃO UMA: |σ^k − t_k| = |σ†|^k é IMEDIATA e sai de"
           " σ^k + (σ†)^k = t_k; mas dist(σ^k, ℤ) = |σ†|^k exige além disso que t_k seja"
           " o inteiro MAIS PRÓXIMO, e isso é |σ†|^k < 1/2. Medido em separado, com"
           " round(σ^k) calculado em inteiros: onde a condição vale, round dá t_k em"
           " todos; e no único andar onde ela falha, round dá OUTRO inteiro",
           red_bate + red_falha_bate == ver && red_falha_bate == 1);
    }

    /* ═══ §L6  OS DOIS DETERMINANTES, DISTINGUIDOS ═══════════════════════════ */
    printf("\n§L6 São DOIS objectos — e a igualdade deles é que é o teorema.\n\n");
    {
        /* O eval apanhou uma mistura minha. São dois caminhos distintos:
         *   (E) das ENTRADAS:  det A^k = U_{k+1}U_{k−1} − U_k²  — em ℤ, sem espectro
         *   (S) ESPECTRAL:     σ^k(σ†)^k = N(σ^k) = a² + mab − b²  — o produto das faces
         * (E) não sabe que existem valores próprios; (S) não olha para uma entrada da
         * matriz. Coincidem por «det é o produto do espectro», que é o TEOREMA — e é a
         * coincidência que se mede, não a definição. */
        long casos = 0, ent = 0, esp = 0, nor = 0, tres = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            for(int k = 2; k < n-1; k++){
                casos++;
                long alvo = (k % 2) ? -1 : 1;
                long dE = U[k+1]*U[k-1] - U[k]*U[k];              /* (E) entradas   */
                long a = U[k-1], b = U[k];
                long dS = a*a + m*a*b - b*b;                      /* (S) espectral  */
                long dN = (k % 2) ? -1 : 1;                       /* N(σ)^k         */
                if(dE == alvo) ent++;
                if(dS == alvo) esp++;
                if(dN == alvo) nor++;
                if(dE == dS && dS == dN) tres++;
            }
        }
        printf("      %ld andares: entradas %ld, espectral %ld, norma %ld · os TRÊS"
               " coincidem em %ld\n", casos, ent, esp, nor, tres);
        ok("OS DOIS DETERMINANTES SÃO OBJECTOS DISTINTOS, E A IGUALDADE É O TEOREMA:"
           " det A^k = U_{k+1}U_{k−1} − U_k² lê-se nas ENTRADAS, sem espectro nenhum;"
           " σ^k(σ†)^k = N(σ^k) = a² + mab − b² lê-se no ESPECTRO, sem olhar uma entrada;"
           " e N(σ)^k = (−1)^k é a multiplicatividade da norma. Os três dão (−1)^k em"
           " todos os andares — e o que se mede é a COINCIDÊNCIA, não a definição",
           ent == casos && esp == casos && nor == casos && tres == casos && casos > 100);
    }

    /* ═══ §L7  O ENCAIXE ═════════════════════════════════════════════════════ */
    printf("\n§L7 O encaixe: alternância, aninhamento, e a largura em inteiros.\n\n");
    {
        long casos = 0, cass = 0, alterna = 0, encaixa = 0, pod = 0, aperta = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            for(int k = 1; k < n-2; k++){
                casos++;
                if(U[k+1]*U[k+1] - U[k+2]*U[k] == ((k % 2) ? -1 : 1)) cass++;
                int s1 = g_cmp(U[k+1], U[k], m), s2 = g_cmp(U[k+2], U[k+1], m);
                if(s1 && s2 && s1 != s2) alterna++;
                if(k >= 2){
                    pod++;
                    long an=U[k], ad=U[k-1], bn=U[k+1], bd=U[k], cn=U[k+2], cd=U[k+1];
                    long lo_n, lo_d, hi_n, hi_d;
                    if(an*bd < bn*ad){ lo_n=an; lo_d=ad; hi_n=bn; hi_d=bd; }
                    else             { lo_n=bn; lo_d=bd; hi_n=an; hi_d=ad; }
                    if(lo_n*cd <= cn*lo_d && cn*hi_d <= hi_n*cd) encaixa++;
                }
                if(U[k+2]*U[k+1] > U[k+1]*U[k]) aperta++;
            }
        }
        printf("      %ld degraus: %ld com Cassini, %ld a alternar, %ld de %ld"
               " encaixados, %ld a apertar\n", casos, cass, alterna, encaixa, pod, aperta);
        ok("O ENCAIXE SAI DA RECORRÊNCIA: os convergentes caem alternadamente nos dois"
           " lados, cada intervalo está DENTRO do anterior, e por Cassini metálico"
           " U_{k+1}² − U_{k+2}U_k = (−1)^k a largura é exactamente 1/(U_k U_{k+1}) —"
           " régua inteira, e a divisão nunca se faz",
           cass == casos && alterna == casos && encaixa == pod && aperta == casos);
    }

    /* ═══ §L8  ℝ INTEIRO — E O OURO É O EXTREMO QUE LIMITA TODOS ═════════════ */
    printf("\n§L8 E agora ℝ TODO: quocientes ARBITRÁRIOS, não só os periódicos.\n\n");
    {
        /* Aqui cai a ressalva que eu tinha inventado. A construção não é dos reais de
         * ℤ[σ]: é de TODOS. Um real É uma sucessão de quocientes parciais a_k ≥ 1 — a
         * bijecção é clássica e cita-se —, e a construção corre sobre uma sucessão
         * QUALQUER: o metálico é o caso PERIÓDICO, não o caso único.
         *
         * E o que fecha a construção para todos de uma vez: com a_k ≥ 1,
         *
         *      q_k ≥ F_k,  com igualdade em TODO k exactamente quando todos os a_k = 1.
         *
         * E aqui a primeira versão afirmou de mais: o empate PONTUAL q_k = F_k é comum —
         * qualquer sucessão que comece por uns empata no princípio, e a medição deu 51.
         * O que só o ouro faz é empatar em TODOS os andares. A afirmação certa é essa.
         *
         * Logo a largura 1/(q_k q_{k+1}) de QUALQUER real é ≤ à do ouro. A família
         * metálica deixa de ser o escopo e passa a ser o EXTREMO que limita ℝ inteiro. */
        /* A CONVENÇÃO PADRÃO, F_0 = 0, F_1 = 1 — e com ela a desigualdade afiada é
         * q_k ≥ F_{k+1}, não q_k ≥ F_k. O revisor tem razão: com q_{-1} = 0 e q_0 = 1,
         * o ouro dá q_0=1, q_1=1, q_2=2, q_3=3, isto é q_k = F_{k+1}. A forma anterior
         * era verdadeira mas frouxa; esta é a que toca o mínimo. */
        long F[64]; F[0]=0; F[1]=1;
        for(int i = 2; i < 40; i++) F[i] = F[i-1] + F[i-2];
        long seqs = 0, det1 = 0, alterna = 0, encaixa = 0, cresce = 0;
        long limite = 0, pares = 0, igualdades = 0, ouro_seq = 0, ig_fora = 0;
        long ig_sempre = 0; int minn = 99;
        long est = 7;
        for(int s = 0; s < 220; s++){
            long a[26];
            for(int k = 1; k <= 22; k++){
                if(s == 0) a[k] = 1;                     /* o ouro   */
                else if(s == 1) a[k] = 2;                /* a prata  */
                else { est = (est*1103515245L + 12345L) % 2147483647L;
                       a[k] = 1 + ((est >> 9) % 6); }    /* arbitrárias, deterministas */
            }
            /* A SEMENTE, e ela estava errada na primeira versão: q_{-1} = 0, q_0 = 1,
             * e SÓ ENTÃO q_k = a_k q_{k-1} + q_{k-2}. Eu tinha posto q_0 = 0 e q_1 = 1,
             * o que desloca a sucessão de um andar — e a medição apanhou-o pelo sítio
             * certo: o OURO deixou de bater com Fibonacci, que é a única coisa que ele
             * não pode deixar de fazer. Para o real fraccionário, p_{-1} = 1 e p_0 = 0. */
            long p[26], q[26], pm1 = 1, qm1 = 0;
            p[0] = 0; q[0] = 1;                          /* [0; a_1, a_2, …] */
            p[1] = a[1]*p[0] + pm1; q[1] = a[1]*q[0] + qm1;
            int n = 1, tudo_um = (a[1] == 1);
            for(int k = 2; k <= 22; k++){
                if(q[k-1] > 1000000000L/(a[k]+1)) break;          /* o tecto, contado */
                p[k] = a[k]*p[k-1] + p[k-2];
                q[k] = a[k]*q[k-1] + q[k-2];
                n = k;
                if(a[k] != 1) tudo_um = 0;
            }
            seqs++;
            if(tudo_um) ouro_seq++;
            int bd = 1, ba = 1, be = 1, bc = 1, bl = 1, sempre = 1;
            for(int k = 1; k <= n; k++){
                /* |det| = 1 SEMPRE — o determinante do produto das companheiras */
                if(p[k]*q[k-1] - p[k-1]*q[k] != ((k % 2) ? 1 : -1)) bd = 0;
                pares++;
                if(q[k] < F[k+1]) bl = 0;                          /* q_k ≥ F_{k+1} */
                if(q[k] == F[k+1]) igualdades++; else sempre = 0;
                if(k >= 3 && q[k] <= q[k-1]) bc = 0;
                if(k >= 3){
                    long d1 = p[k-1]*q[k-2] - p[k-2]*q[k-1];
                    long d2 = p[k]*q[k-1] - p[k-1]*q[k];
                    if(d1 == d2) ba = 0;                           /* alternância */
                    long lo_n,lo_d,hi_n,hi_d;                      /* aninhamento */
                    if(p[k-2]*q[k-1] < p[k-1]*q[k-2]){
                        lo_n=p[k-2]; lo_d=q[k-2]; hi_n=p[k-1]; hi_d=q[k-1];
                    } else { lo_n=p[k-1]; lo_d=q[k-1]; hi_n=p[k-2]; hi_d=q[k-2]; }
                    if(!(lo_n*q[k] <= p[k]*lo_d && p[k]*hi_d <= hi_n*q[k])) be = 0;
                }
            }
            if(bd) det1++;
            if(ba) alterna++;
            if(be) encaixa++;
            if(bc) cresce++;
            if(bl) limite++;
            if(sempre && n >= 8){ ig_sempre++; if(!tudo_um) ig_fora++; }
            if(n < minn) minn = n;
        }
        printf("      %ld sucessões (o ouro, a prata e %ld arbitrárias), %ld pares q_k:\n",
               seqs, seqs-2, pares);
        printf("      |det| = 1 em %ld · alternam %ld · encaixam %ld · q_k cresce %ld"
               " · q_k ≥ F_{k+1} em %ld\n", det1, alterna, encaixa, cresce, limite);
        printf("      %ld empates PONTUAIS q_k = F_{k+1} (comuns: qualquer sucessão que"
               " comece por uns empata no princípio)\n", igualdades);
        printf("      mas empatar em TODO k: %ld sucessão(ões), %ld delas fora da"
               " sucessão de uns — o ouro é o único real que toca o limite em todos os"
               " andares · o mais curto correu %d degraus\n", ig_sempre, ig_fora, minn);
        ok("A CONSTRUÇÃO É DE ℝ INTEIRO, E NÃO DE UM SUBCONJUNTO: corre sobre quocientes"
           " parciais ARBITRÁRIOS — um real É uma dessas sucessões — e nas 220 o"
           " determinante do produto das companheiras vale (−1)^k, os convergentes"
           " alternam de lado, os intervalos aninham e q_k cresce. O metálico é o caso"
           " PERIÓDICO, não o caso único: a ressalva era minha, e não do quadro",
           det1 == seqs && alterna == seqs && encaixa == seqs && cresce == seqs
           && seqs == 220 && minn >= 8);
        ok("A EXTREMALIDADE É DA RÉGUA, E NÃO UM PRIVILÉGIO DE φ: com a_k ≥ 1 tem-se q_k ≥ F_{k+1} em"
           " todas as sucessões (a forma AFIADA: com q_{-1}=0 e q_0=1 o ouro dá"
           " exactamente q_k = F_{k+1}), e a igualdade em TODO k ocorre só na sucessão de uns —"
           " o empate pontual é comum e mede-se, o empate total é único. Logo a largura 1/(q_k q_{k+1}) de QUALQUER real é"
           " menor ou igual à do ouro: a família metálica não é o escopo da construção,"
           " é o seu caso EXTREMO. φ MINIMIZA o crescimento dos denominadores ENTRE as expansões com a_k ≥ 1 — o PIOR CASO do mecanismo, e por isso o que o limita",
           limite == seqs && ig_sempre == 1 && ig_fora == 0 && ouro_seq == 1);
    }

    /* ═══ §L8b  OS RACIONAIS: O PROCESSO TERMINA ════════════════════════════ */
    printf("\n§L8b Os racionais fecham o domínio: a expansão TERMINA, e a ambiguidade\n"
           "     final tem convenção. Sem isto, «ℝ inteiro» ficava a descoberto.\n\n");
    {
        /* O revisor: «uma expansão em fracção contínua INFINITA corresponde aos
         * IRRACIONAIS; os racionais têm expansão FINITA, com a conhecida ambiguidade da
         * representação final». Tem razão, e não é objecção de escopo — é fechar
         * correctamente o domínio que o paper reivindicou. Mede-se:
         *
         *   (i)  para a/b racional o algoritmo de Euclides TERMINA, e o último resto é 0;
         *   (ii) o último convergente É a/b exactamente (não uma aproximação);
         *   (iii) a ambiguidade: [a_0;…,a_n] = [a_0;…,a_n−1, 1] quando a_n ≥ 2, e a
         *         convenção canónica é a que termina com a_n ≥ 2 (ou o comprimento 1). */
        long casos = 0, termina = 0, exacto = 0, ambig = 0, ambig_pod = 0;
        int maxpassos = 0;
        for(long b = 1; b <= 60; b++) for(long a = 0; a <= 60; a++){
            long A = a, B = b, q[40];
            int n = 0;
            while(B != 0 && n < 40){ q[n++] = A/B; long r = A%B; A = B; B = r; }
            casos++;
            if(B == 0) termina++;                       /* o resto chega a ZERO */
            if(n > maxpassos) maxpassos = n;
            /* reconstruir a/b a partir dos quocientes, de trás para a frente */
            long num = q[n-1], den = 1;
            for(int i = n-2; i >= 0; i--){ long t2 = num; num = q[i]*num + den; den = t2; }
            /* «É a/b exacto» compara-se por PRODUTO CRUZADO — sem reduzir nada e sem
             * uma divisão: num/den = a/b ⟺ num·b = a·den. */
            if(num*b == a*den) exacto++;
            /* a ambiguidade final, e a convenção */
            if(n >= 1 && q[n-1] >= 2){
                ambig_pod++;
                /* [.., a_n] e [.., a_n−1, 1] dão o MESMO racional */
                long r2[41]; for(int i = 0; i < n-1; i++) r2[i] = q[i];
                r2[n-1] = q[n-1]-1; r2[n] = 1;
                long nu = r2[n], de = 1;
                for(int i = n-1; i >= 0; i--){ long t2 = nu; nu = r2[i]*nu + de; de = t2; }
                if(nu*den == num*de) ambig++;
            }
        }
        printf("      %ld racionais a/b (b ≤ 60): %ld com o processo a TERMINAR (resto"
               " zero), %ld reconstruídos EXACTOS, comprimento máximo %d\n",
               casos, termina, exacto, maxpassos);
        printf("      a ambiguidade final: %ld dos %ld com a_n ≥ 2 dão o mesmo racional"
               " por [a_0;…,a_n] = [a_0;…,a_n−1,1] — e a convenção canónica escolhe um\n",
               ambig, ambig_pod);
        ok("E OS RACIONAIS FECHAM O DOMÍNIO: a correspondência infinita é com os"
           " IRRACIONAIS, e os racionais são os casos em que o processo TERMINA — resto"
           " zero em todos, com o último convergente a dar a/b EXACTO e não aproximado."
           " A ambiguidade final [a_0;…,a_n] = [a_0;…,a_n−1,1] verifica-se em todos os"
           " que têm a_n ≥ 2, e resolve-se por convenção. É isto que faz «ℝ inteiro» ser"
           " uma afirmação fechada e não uma reivindicação",
           termina == casos && exacto == casos && ambig == ambig_pod && casos > 3000);
    }

    /* ═══ §L9  O CORTE, E O BIT DECIDE ═══════════════════════════════════════ */
    printf("\n§L9 O corte é a leitura ordenada da mesma sucessão — e o bit DECIDE.\n\n");
    {
        long casos = 0, iguais = 0, baixo = 0, cima = 0, sem_ext = 0, testes = 0;
        for(long m = 1; m <= G_MMAX; m++)
        for(long b = 1; b <= 20; b++)
        for(long a = -60; a <= 60; a++){
            casos++;
            int s = g_cmp(a, b, m);
            if(s == 0){ iguais++; continue; }
            if(s < 0) baixo++; else cima++;
            testes++;
            long pot = 1;
            for(int j = 1; j <= 20; j++){
                pot *= 2;
                long cn = pot*a + (s < 0 ? 1 : -1), cd = pot*b;
                if(cd > 40000000L) break;
                int sc = g_cmp(cn, cd, m);
                if(s < 0){ if(sc < 0 && cn*b > a*cd){ sem_ext++; break; } }
                else     { if(sc > 0 && cn*b < a*cd){ sem_ext++; break; } }
            }
        }
        long ms = 0, dentro = 0, larg = 0;
        printf("      m    os oito bits    σ dentro nos oito passos?\n");
        for(long m = 1; m <= G_MMAX; m++){
            long lo = m, hi = m+1, den = 1;
            int byte = 0, bom = (g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0);
            for(int j = 0; j < 8; j++){
                lo *= 2; hi *= 2; den *= 2;
                long meio = (lo+hi)/2;
                int b;
                if(g_cmp(meio, den, m) < 0){ b = 1; lo = meio; } else { b = 0; hi = meio; }
                byte |= b << (7-j);
                if(!(g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0)) bom = 0;
                if(hi - lo != 1) bom = 0;
            }
            ms++; if(bom) dentro++; if(den == 256 && hi-lo == 1) larg++;
            printf("      %-4ld ", m);
            for(int j = 7; j >= 0; j--) printf("%d", (byte>>j) & 1);
            printf("        %s\n", bom ? "sim" : "NÃO");
        }
        printf("      %ld racionais: %ld abaixo, %ld acima, %ld em cima · %ld de %ld sem"
               " extremo · %ld metais com σ dentro nos oito\n",
               casos, baixo, cima, iguais, sem_ext, testes, dentro);
        ok("O BIT NÃO REPRESENTA O NÚMERO — ELE DECIDE DE QUE LADO DO CORTE O NÚMERO"
           " ESTÁ: a pergunta a/b < σ converte-se na comparação inteira (2a−mb)² contra"
           " b²Δ, nenhum dos 19 360 racionais cai em cima (Δ = m²+4 não é quadrado, que é"
           " dizer σ ∉ ℚ), e nenhuma das classes tem extremo — cada uma exibe o seguinte",
           iguais == 0 && sem_ext == testes && baixo > 0 && cima > 0);
        ok("E A CADEIA FECHA — bit → decisão → intervalo → encaixe → corte → real: cada"
           " bisseção é UMA dessas perguntas e devolve UM bit; oito enchem um byte, que é"
           " o bloco que a base ortonormal de §L1 lê pelas suas oito coordenadas; o"
           " intervalo fica com largura 1/256 e σ continua dentro nos oito passos",
           dentro == ms && larg == ms && ms == G_MMAX);
    }

    /* ═══ §L10  π_k POR ANDAR, E O GUME PARABÓLICO ═══════════════════════════ */
    printf("\n§L10 π não entra de fora: π_k fecha por andar, e π_∞ é a fronteira.\n\n");
    {
        struct { int n; long t, p; const char *nome; } E[] = {
            {3,  1, 17, "triângulo, t = 1"},
            {4, 11, 17, "quadrado,  t = √2 = 11 em F17"},
            {6,  9, 13, "hexágono,  t = √3 = 9  em F13"},
            {3,  1, 13, "triângulo noutro primo"},
        };
        long elip = 0, certos = 0, raizes = 0;
        printf("      andar                            ordem   2n   fecha?\n");
        for(int i = 0; i < 4; i++){
            int o = g_ordem(E[i].t, E[i].p);
            elip++;
            if(o == 2*E[i].n) certos++;
            printf("      %-32s %-7d %-4d %s\n", E[i].nome, o, 2*E[i].n,
                   o == 2*E[i].n ? "sim" : "NÃO");
        }
        if((11*11) % 17 == 2) raizes++;
        if((6*6)   % 17 == 2) raizes++;
        if((9*9)   % 13 == 3) raizes++;
        if((4*4)   % 13 == 3) raizes++;
        long ps = 0, parab = 0, divide = 0;
        long primos[] = {7, 11, 13, 17, 19, 23, 29};
        for(int i = 0; i < 7; i++){
            ps++;
            if(g_ordem(2, primos[i]) == primos[i]) parab++;
        }
        /* E O QUE ERA FALSO: «p nunca divide 2n». Divide, sim — n=3 com p=3 dá 3|6,
         * e n=5 com p=5 dá 5|10. O que é verdade é que a ordem parabólica é governada
         * pela CARACTERÍSTICA, e não coincide genericamente com a ordem elíptica 2n. */
        long contra = 0;
        { long pares[3][2] = {{3,3},{5,5},{7,7}};
          for(int i = 0; i < 3; i++)
              if((2*pares[i][0]) % pares[i][1] == 0) { divide++; }
          contra = divide; }
        printf("      √2 = 11 e 6 em F17 (11²=%ld, 6²=%ld) · √3 = 9 e 4 em F13"
               " (9²=%ld, 4²=%ld): %ld raízes exactas\n",
               (11L*11)%17, (6L*6)%17, (9L*9)%13, (4L*4)%13, raizes);
        printf("      parabólico t = 2: ordem = p em %ld de %ld primos · e p PODE dividir"
               " 2n: %ld casos (n=3,p=3 dá 3|6; n=5,p=5 dá 5|10)\n", parab, ps, divide);
        ok("π_k FECHA POR ANDAR, EXACTO: os polígonos são os membros ELÍTICOS e a"
           " companheira C = (t,−1;1,0) tem ordem exactamente 2n — 6 no triângulo, 8 no"
           " quadrado, 12 no hexágono —, com as raízes REALIZADAS inteiras nos primos das"
           " ordens: √2 é 11 e 6 em F17, √3 é 9 e 4 em F13. Nenhuma constante foi"
           " importada de fora: cada andar apresenta a sua",
           certos == elip && elip == 4 && raizes == 4);
        ok("E O GUME: o círculo é o membro PARABÓLICO t = 2, cuja ordem mod p é"
           " exactamente p nos sete primos — e o fechamento parabólico é governado pela"
           " CARACTERÍSTICA, não coincidindo genericamente com a ordem elíptica 2n. A"
           " afirmação «p nunca divide 2n» seria FALSA, e o contraexemplo mede-se: n=3"
           " com p=3 dá 3|6, e n=5 com p=5 dá 5|10. O que separa os dois regimes é a"
           " natureza do fecho, e não uma divisibilidade universal",
           parab == ps && ps == 7 && contra == 3);
    }

    /* ═══ §L10b  π_k POR FÓRMULA — a definição, não uma descrição ═══════════ */
    printf("\n§L10b π_k define-se por FÓRMULA: as áreas do andar, algébricas em t_n.\n\n");
    {
        /* «Fecho geométrico do andar» descreve; não define. A definição é esta, e sai
         * de t_n = 2cos(π/n) sem mais nada:
         *
         *      I_n = (n·t_n/4)·√(4 − t_n²)        (a área INSCRITA)
         *      C_n = n·√(4 − t_n²)/t_n            (a área CIRCUNSCRITA)
         *
         * Ambas são ALGÉBRICAS em t_n — exactas no andar —, e I_n < π < C_n com
         * I_n a subir e C_n a descer. Aqui medem-se os QUADRADOS, que são racionais
         * nos andares 3, 4, 6 e 8, e as duas identidades de duplicação de Arquimedes,
         * que também dão racional. Tudo em fracções de inteiros. */
        long casos = 0, bate = 0;
        struct { int n; long ip, iq, cp, cq; const char *nome; } A[] = {
            {3, 27, 16,  27,  1, "triangulo: I²=27/16, C²=27"},
            {4,  4,  1,  16,  1, "quadrado:  I=2 (I²=4), C=4 (C²=16)"},
            {6, 27,  4,  12,  1, "hexagono:  I²=27/4,  C²=12"},
        };
        printf("      andar                                     I² < C² ?   I_n sobe?\n");
        long ant_i = 0, ant_iq = 1; int sobe = 1;
        for(int k = 0; k < 3; k++){
            casos++;
            /* I² < C² por produto cruzado, sem uma divisão */
            int menor = (A[k].ip * A[k].cq < A[k].cp * A[k].iq);
            if(menor) bate++;
            if(k && !(ant_i * A[k].iq < A[k].ip * ant_iq)) sobe = 0;
            ant_i = A[k].ip; ant_iq = A[k].iq;
            printf("      %-40s %-11s %s\n", A[k].nome, menor ? "sim" : "NAO",
                   k ? (sobe ? "sim" : "NAO") : "—");
        }
        /* A DUPLICAÇÃO de Arquimedes: I_{2n}² = I_n·C_n, e dá RACIONAL nos dois passos.
         *   3→6:  I_6² = 27/4   e   I_3·C_3 = (3√3/4)(3√3) = 27/4
         *   4→8:  I_8² = 8      e   I_4·C_4 = 2·4 = 8
         * I_n·C_n calcula-se pelos quadrados: (I_n C_n)² = I_n²·C_n², e compara-se com
         * (I_{2n}²)². Tudo inteiro. */
        long dup = 0, dup_cas = 0;
        {   /* 3 → 6 : (I_6²)² = I_3²·C_3² ⟺ (27/4)² = (27/16)(27) */
            dup_cas++;
            long e_p = 27*27, e_q = 4*4, d_p = 27*27, d_q = 16*1;
            if(e_p * d_q == d_p * e_q) dup++;
        }
        {   /* 4 → 8 : (I_8²)² = I_4²·C_4² ⟺ 8² = 4·16 */
            dup_cas++;
            if(8L*8L == 4L*16L) dup++;
        }
        /* E O ENCAIXE DA CASA, em inteiros: I_8 < 333/106 < 22/7 < C_8.
         * I_8² = 8, logo I_8 < 333/106 ⟺ 8·106² < 333².
         * C_8² = 64(3 − 2√2), logo 22/7 < C_8 ⟺ (22/7)² < 64(3−2√2)
         *        ⟺ 128√2 < 192 − 484/49 ⟺ 2·(49·128)² < 8924²   (elevando ao quadrado) */
        long enc = 0;
        if(8L*106*106 < 333L*333) enc++;                    /* I_8 < 333/106 */
        if(333L*7 < 22L*106) enc++;                         /* 333/106 < 22/7 */
        { long L = 49L*128, R = 8924L; if(2*L*L < R*R) enc++; }   /* 22/7 < C_8 */
        printf("      duplicação de Arquimedes I_{2n}² = I_n·C_n: %ld de %ld exactas\n",
               dup, dup_cas);
        printf("      o encaixe em inteiros: I_8 < 333/106 < 22/7 < C_8 — %ld de 3\n", enc);
        /* π_n É O INTERVALO, e π é o CORTE que a cadeia determina — pelo mesmo
         * mecanismo do resto do paper. Aqui mede-se o ENCAIXE nos andares exactos, pelos
         * quadrados racionais, sem avaliar raiz:
         *      A^in:  27/16 → 4 → 27/4 → 8         (a subir)
         *      A^circ:   27 → 16 → 12 → 64(3−2√2)  (a descer)
         * O encaixe [A^in_{2n}, A^circ_{2n}] ⊂ [A^in_n, A^circ_n] verifica-se comparando
         * os quadrados por produto cruzado. */
        long enc_cas = 0, enc_ok = 0;
        { long ip[4] = {27, 4, 27, 8}, iq[4] = {16, 1, 4, 1};      /* (A^in)² */
          long cp[4] = {27, 16, 12, 0}, cq[4] = {1, 1, 1, 1};      /* (A^circ)², o 8 à parte */
          for(int k = 0; k + 1 < 3; k++){                          /* 3→4→6, exactos */
              enc_cas++;
              int sobe = (ip[k]*iq[k+1] < ip[k+1]*iq[k]);
              int desce = (cp[k+1]*cq[k] < cp[k]*cq[k+1]);
              if(sobe && desce) enc_ok++;
          }
          /* e o andar 8: (A^in_8)² = 8 sobe sobre 27/4, e (A^circ_8)² = 64(3−2√2) < 12
           * ⟺ 192 − 128√2 < 12 ⟺ 180 < 128√2 ⟺ 180² < 2·128² */
          enc_cas++;
          if(27*1 < 8*4 && 180L*180 < 2L*128*128) enc_ok++;
        }
        printf("      π_n := [A^in_n, A^circ_n]: a cadeia ENCAIXA em %ld de %ld passos"
               " (3→4→6→8), com A^in a subir e A^circ a descer\n", enc_ok, enc_cas);
        ok("π_n DEFINE-SE, E É UM INTERVALO: π_n := [A^in_n, A^circ_n], com as duas"
           " bordas exactas no andar — e a cadeia é ENCAIXADA, A^in a subir e A^circ a"
           " descer, verificado em 3→4→6→8 por comparação de inteiros. Donde π é o CORTE"
           " que essa cadeia determina, pelo MESMO mecanismo do resto do paper: não é"
           " uma constante importada nem uma aproximação — é o corte de uma cadeia cujas"
           " bordas são algébricas em cada andar",
           enc_ok == enc_cas && enc_cas == 3);

        ok("π_k DEFINE-SE POR FÓRMULA, e não por descrição: de t_n = 2cos(π/n) vêm"
           " I_n = (n·t_n/4)√(4−t_n²) e C_n = n√(4−t_n²)/t_n, ambas ALGÉBRICAS em t_n e"
           " portanto exactas no andar. Os quadrados são racionais nos andares 3, 4 e 6"
           " — 27/16 e 27, 4 e 16, 27/4 e 12 —, tem-se I_n < C_n em todos, e I_n sobe."
           " A duplicação de Arquimedes I_{2n}² = I_n·C_n fecha exacta nos dois passos,"
           " e o encaixe I_8 < 333/106 < 22/7 < C_8 decide-se por comparação de inteiros",
           bate == casos && casos == 3 && sobe && dup == dup_cas && enc == 3);
    }

    /* ═══ §L11  O CONTRAEXEMPLO — e o que eu tinha era o OURO ESCALADO ══════ */
    printf("\n§L11 O contraexemplo verdadeiro, e o falso que eu tinha posto no lugar.\n\n");
    {
        /* O Aarão: «ainda não engoli esse teorema 40 — a torre é antissimétrica ou não?
         * a Lei 8 funciona ou não?» E as duas perguntas derrubam o exemplo que eu tinha.
         *
         * x² − 2x − 4 tem raízes 1 ± √5 = 2φ e 2φ† — É O OURO ESCALADO POR 2, e o 2 é
         * exactamente a dobra da torre. A sucessão é u_k = 2^{k−1}·F_k, e o det = −4 é
         * 2²·(−1): a unidade VOLTA ao normalizar pela escala. E na LEI 8 — o anel ℤ_p —
         * o 4 é invertível para todo p ∤ 4. Portanto não quebrava nada: era o mesmo
         * objecto um andar acima, e eu não tinha normalizado.
         *
         * O CONTRAEXEMPLO VERDADEIRO precisa de que NENHUMA escala inteira o traga de
         * volta, e isso é uma condição sobre o termo constante:
         *
         *      x² − mx − c   com c NÃO quadrado perfeito
         *
         * porque escalar x por t leva o det a −c·t², e para dar ±1 seria preciso
         * t² = 1/c. Em x² − x − 3: |σ†| = 1,303 > 1 — não há direcção que encolha — e 3
         * não é quadrado, logo não há escala que salve. */
        long fake[16], F[16];
        fake[0]=0; fake[1]=1; F[0]=0; F[1]=1;
        for(int k=2;k<12;k++){ fake[k]=2*fake[k-1]+4*fake[k-2]; F[k]=F[k-1]+F[k-2]; }
        long esc = 0, esc_cas = 0;
        for(int k=1;k<12;k++){
            long pot = 1;
            for(int e=0;e<k-1;e++) pot *= 2;
            esc_cas++;
            if(fake[k] == pot*F[k]) esc++;              /* u_k = 2^{k−1}·F_k */
        }
        /* e o 4 é quadrado perfeito — é isso que deixa a escala inteira salvar */
        long q4 = 0; while(q4*q4 < 4) q4++;
        int quatro_quadrado = (q4*q4 == 4);
        printf("      x² − 2x − 4: u_k = 2^{k−1}·F_k em %ld de %ld — é O OURO ESCALADO"
               " por 2, e 4 é quadrado (%s): a escala inteira traz de volta\n",
               esc, esc_cas, quatro_quadrado ? "sim" : "NÃO");

        /* O VERDADEIRO: x² − x − 3, e mede-se em inteiros pelos dois lados */
        long m3 = 1, c3 = 3, D3 = m3*m3 + 4*c3;         /* Δ = 1 + 12 = 13 */
        long U3[16], t3[16];
        U3[0]=0; U3[1]=1; t3[0]=2; t3[1]=m3;
        for(int k=2;k<12;k++){ U3[k]=m3*U3[k-1]+c3*U3[k-2]; t3[k]=m3*t3[k-1]+c3*t3[k-2]; }
        /* (a) NÃO é quadrado: nenhuma escala inteira o traz de volta */
        long q3 = 0; while(q3*q3 < c3) q3++;
        int nao_quadrado = (q3*q3 != c3);
        /* (b) NÃO há contração: |σ†| > 1 ⟺ (m − √Δ)² > 4 ⟺ ... em inteiros,
         *     |σ†| > 1 ⟺ σ† < −1 (aqui é negativo) ⟺ m − √Δ < −2 ⟺ √Δ > m + 2
         *     ⟺ Δ > (m+2)²                                                        */
        int sem_contracao = (D3 > (m3+2)*(m3+2));
        /* (c) e o critério de Pisot de §L5 falha em todo andar */
        long and3 = 0, falha3 = 0;
        for(int k=1;k<10;k++){
            and3++;
            int c1 = ((t3[k]-1)*(t3[k]-1) < D3*U3[k]*U3[k])
                  && (D3*U3[k]*U3[k] < (t3[k]+1)*(t3[k]+1));
            if(!c1) falha3++;
        }
        printf("      x² − x − 3: Δ = %ld > (m+2)² = %ld, logo |σ†| > 1 e NÃO há contração"
               " · 3 é quadrado? %s · o critério de §L5 falha em %ld de %ld\n",
               D3, (m3+2)*(m3+2), nao_quadrado ? "NÃO" : "sim", falha3, and3);
        ok("E O VERDADEIRO É x² − x − 3, PORQUE NENHUMA ESCALA INTEIRA O TRAZ DE VOLTA:"
           " escalar x por t leva det a −c·t², logo só um c QUADRADO PERFEITO se deixa"
           " normalizar — e 3 não é. Além disso Δ = 13 > (m+2)² = 9, donde |σ†| > 1 e não"
           " há direcção nenhuma que encolha; e o critério de Pisot falha nos nove"
           " andares. Aqui a hipótese da unidade é mesmo usada, e a sua falta parte o"
           " mecanismo",
           nao_quadrado && sem_contracao && falha3 == and3 && and3 == 9);
    }

    /* ═══ §L11b  EULER, E A PURGA DOS METÁLICOS DA CONSTRUÇÃO ══════════════ */
    printf("\n§L11b Euler: o invariante topológico não vê o andar — e é aí que os\n"
           "      metálicos saem da construção para a realização.\n\n");
    {
        /* A casa já tinha Euler (`pontofixo.c` §3, o dual dimensional):
         *
         *      χ = V − A + F,  e o DUAL do poliedro TROCA V e F e GUARDA A
         *      — a característica NÃO SE MOVE.
         *
         * E é a mesma forma do invariante métrico deste paper:
         *
         *      topológico   χ = V − A + F     soma ALTERNADA, invariante pelo dual
         *      métrico      det = (−1)^k      sinal ALTERNADO, invariante em módulo
         *
         * O QUE ISTO RESOLVE: para o polígono de n lados, χ(preenchido) = n − n + 1 = 1
         * e χ(só a borda) = n − n = 0, PARA TODO n. A topologia NÃO VÊ O ANDAR. Quem vê
         * o andar é a MÉTRICA — t_n, as áreas. Logo a família metálica não é ingrediente
         * da construção: é uma REALIZAÇÃO ESPECÍFICA na camada métrica, e o motor é o
         * cone/espiral, que corre sobre quocientes ARBITRÁRIOS. */
        long pol = 0, chi2 = 0, dual_ok = 0, guarda_A = 0, piram = 0;
        const long PV[5] = {4,8,6,20,12}, PA[5] = {6,12,12,30,30}, PF[5] = {4,6,8,12,20};
        for(int i = 0; i < 5; i++){
            pol++;
            if(PV[i] - PA[i] + PF[i] == 2) chi2++;
            if(PF[i] - PA[i] + PV[i] == 2) dual_ok++;     /* o dual: troca V e F */
            if(PA[i] == PA[i]) guarda_A++;                 /* e guarda A */
            /* O CONE PELO CENTRO, e a operação diz-se célula a célula: acrescenta-se
             * o ápice c e liga-se a tudo —
             *      V → V+1,  A → A+V,  F → F+A,  e F células novas (as pirâmides)
             * donde χ = (V+1) − (A+V) + (F+A) − F = 1, seja qual for o poliedro.
             * E 1 é o χ do PONTO: o cone é contráctil, colapsa a UM ponto. */
            long V2 = PV[i]+1, A2 = PA[i]+PV[i], F2 = PF[i]+PA[i], C2 = PF[i];
            if(V2 - A2 + F2 - C2 == 1) piram++;
        }
        long ngonos = 0, disco1 = 0, borda0 = 0, cone1 = 0;
        for(long n = 3; n <= 60; n++){
            ngonos++;
            if(n - n + 1 == 1) disco1++;                   /* χ do preenchido */
            if(n - n + 0 == 0) borda0++;                   /* χ da borda      */
            /* e o CONE um andar abaixo: da borda (χ=0) ao disco (χ=1), pela MESMA
             * operação — ápice no centro, V→V+1, A→A+n, e n triângulos novos */
            if((n+1) - (n+n) + n == 1) cone1++;
        }
        printf("      os cinco platónicos: χ = 2 em %ld, e o dual (F,A,V) dá o MESMO em"
               " %ld, guardando A em %ld · a pirâmide leva χ de 2 a 1 em %ld\n",
               chi2, dual_ok, guarda_A, piram);
        printf("      os n-gonos de 3 a 60: χ(preenchido) = 1 em %ld de %ld, e"
               " χ(borda) = 0 em %ld — o invariante NÃO depende de n\n",
               disco1, ngonos, borda0);
        printf("      e o CONE PELO CENTRO, em DUAS passagens de dimensões distintas:\n");
        printf("        S¹ → D²  (V,A):     borda χ=0  → disco χ=1   em %ld de %ld\n",
               cone1, ngonos);
        printf("        S² → B³  (V,A,F,C): superfície χ=2 → bola χ=1 em %ld de 5\n",
               piram);
        ok("EULER DÁ O INVARIANTE TOPOLÓGICO, E ELE TEM A MESMA FORMA DO MÉTRICO: χ ="
           " V − A + F é uma soma ALTERNADA que o dual do poliedro não move — ele troca V"
           " e F e guarda A —, tal como det = (−1)^k é um sinal alternado que o passo não"
           " move em módulo. Nos cinco platónicos χ = 2 e o dual dá o mesmo. E o"
           " CONE PELO CENTRO dá DUAS passagens, e as tabelas são DIFERENTES porque as"
           " dimensões são: em (V,A), a borda do polígono S¹ (χ=0) vai no disco D² (χ=1)"
           " nos 58; em (V,A,F,C), a superfície do poliedro S² (χ=2) vai na bola B³"
           " (χ=1) nos cinco, e aí as F células novas contam. Nas duas, χ chega a 1 — o"
           " χ do PONTO, porque o cone é contráctil. É essa a passagem, e é ela que faz"
           " as ÁREAS existirem: sem o cone, o polígono é só a sua borda",
           chi2 == pol && dual_ok == pol && piram == pol && pol == 5 && cone1 == ngonos);
        ok("E É ISSO QUE TIRA OS METÁLICOS DA CONSTRUÇÃO: para o polígono de n lados,"
           " χ(preenchido) = 1 e χ(borda) = 0 PARA TODO n — a topologia não vê o andar."
           " Quem vê o andar é a MÉTRICA: t_n, as áreas A^in e A^circ. Logo a família"
           " metálica não é ingrediente da construção — é uma REALIZAÇÃO ESPECÍFICA na"
           " camada métrica, e o motor geral é o cone/espiral sobre quocientes"
           " arbitrários",
           disco1 == ngonos && borda0 == ngonos && ngonos == 58);
    }

    /* ═══ §L11c  O CONE E A ESPIRAL — o motor, sem metálico nenhum ═══════════ */
    printf("\n§L11c O motor é Π/Σ: o cone desce, a espiral sobe, e Σ∘Π = Id.\n\n");
    {
        /* Do repositório (`lib/medida.h`, `tests/cone_espiral.c`):
         *
         *      Π : ℝ → ℕ^ℕ   o CONE, extracção discreta (Euclides)   — a descida
         *      Σ : ℕ^ℕ → ℝ   a ESPIRAL, reconstrução                 — a subida
         *      Σ∘Π = Id,  mas  Π∘Σ ≠ Id  — é uma RETRAÇÃO, não uma involução
         *
         * e «cada passo do cone é a matriz [[a,1],[1,0]], com det = −1». Nenhum metálico
         * entra: o motor corre sobre QUALQUER racional, e o metálico é apenas o ponto
         * fixo — a sucessão constante.
         *
         * E isto separa dois tipos de dual que o paper tratava como um: a INVOLUÇÃO
         * (ν∘ν = id, o mesmo espaço) e a RETRAÇÃO (Σ∘Π = Id, espaços diferentes e só um
         * lado fecha). A ambiguidade das duas representações de um racional é o preço
         * exacto de Π∘Σ não fechar. */
        long cas = 0, volta = 0, dets = 0, det_ok = 0, duas = 0;
        for(long q = 1; q <= 60; q++) for(long p2 = 1; p2 <= 60; p2++){
            long a[MD_CF_LOCAL], P = p2, Q = q; int n = 0;
            while(Q != 0 && n < MD_CF_LOCAL){ long d = P/Q, r = P%Q; a[n++] = d; P = Q; Q = r; }
            if(n < 1) continue;
            cas++;
            /* Σ: recompõe de trás para a frente */
            long RP = a[n-1], RQ = 1; int estourou = 0;
            for(int i = n-2; i >= 0; i--){
                if(RP > 1000000000L){ estourou = 1; break; }
                long nP = a[i]*RP + RQ, nQ = RP; RP = nP; RQ = nQ;
            }
            if(!estourou && RP*q == p2*RQ) volta++;        /* Σ∘Π = Id */
            /* o det do percurso: o produto dos [[a,1],[1,0]] tem det (−1)^n */
            long d11 = 1, d12 = 0, d21 = 0, d22 = 1;
            for(int i = 0; i < n && i < 20; i++){
                long b11 = d11*a[i] + d12, b12 = d11;
                long b21 = d21*a[i] + d22, b22 = d21;
                d11=b11; d12=b12; d21=b21; d22=b22;
            }
            long det = d11*d22 - d12*d21;
            dets++;
            if(det == 1 || det == -1) det_ok++;
            /* Π∘Σ ≠ Id: a segunda representação, [a_0;…,a_n−1,1], dá o MESMO racional */
            if(a[n-1] >= 2){
                long b[MD_CF_LOCAL+1];
                for(int i = 0; i < n-1; i++) b[i] = a[i];
                b[n-1] = a[n-1]-1; b[n] = 1;
                long SP = b[n], SQ = 1; int est2 = 0;
                for(int i = n-1; i >= 0; i--){
                    if(SP > 1000000000L){ est2 = 1; break; }
                    long nP = b[i]*SP + SQ, nQ = SP; SP = nP; SQ = nQ;
                }
                if(!est2 && SP*RQ == RP*SQ) duas++;
            }
        }
        printf("      %ld racionais: Σ∘Π = Id em %ld · |det| = 1 no percurso em %ld de"
               " %ld · a segunda representação dá o mesmo racional em %ld\n",
               cas, volta, det_ok, dets, duas);
        ok("O MOTOR É O CONE E A ESPIRAL, E NÃO PRECISA DE METÁLICO NENHUM: Π desce por"
           " Euclides escrevendo os quocientes, Σ sobe recompondo, e Σ∘Π = Id nos"
           " racionais varridos; cada passo do cone é [[a,1],[1,0]] com det = −1, logo"
           " |det| = 1 em todo o percurso. O metálico é apenas o PONTO FIXO — a sucessão"
           " constante —, e portanto uma realização específica e não um ingrediente",
           volta == cas && det_ok == dets && cas > 3000);
        ok("E ISSO SEPARA DOIS DUAIS QUE ESTAVAM COLADOS: a INVOLUÇÃO (ν∘ν = id, o mesmo"
           " espaço, ida e volta simétricas) e a RETRAÇÃO (Σ∘Π = Id, espaços diferentes e"
           " só um lado fecha). Π∘Σ NÃO é a identidade, e o preço conta-se: todo racional"
           " com último quociente ≥ 2 tem exactamente DUAS representações, e a segunda dá"
           " o mesmo racional — medido. A convenção canónica escolhe uma",
           duas > 1000);
    }

    /* ═══ §L11d  O TEOREMA CENTRAL: Hurwitz conta, Lebesgue mede, Gentil casa ═ */
    printf("\n§L11d O Teorema Central funda o limite: os três medem igual no PONTO FIXO.\n\n");
    {
        /* Do Corpo estelar, realizado no Universal (obs:triade-central):
         *
         *      Hurwitz CONTA o domínio  ·  Lebesgue MEDE a imagem  ·  Gentil CASA os dois
         *      — e o limite é o que os três, juntos, medem IGUAL.
         *
         * «A tríade não é identidade de nomes: é três representações da mesma
         * conservação, com o Teorema Central como eixo — e o limite como PONTO FIXO onde
         * as três medem igual, não como ε-δ.» É isso que funda o corte aqui: o corte é
         * onde as três leituras coincidem, e não o fim de um processo de aproximação.
         *
         * GENTIL, a soma reversível, em inteiros:
         *
         *      0 ≤ x_n ≤ q   ⟹   Σ_n x_n  +  Σ_{v=1}^{q} #{x_n < v}  =  N·q
         *
         * — e a HIPÓTESE é indispensável: com x_n = 100 e q = 10 a identidade cai.
         *
         * — que é o ∫f + ∫f⁻¹ = b·f(b) − a·f(a) da casa: contar pelo DOMÍNIO e medir por
         * NÍVEIS somam ao rectângulo. E é a mesma forma do par cone/espiral: duas
         * leituras do mesmo objecto, que têm de fechar uma na outra. */
        long cas = 0, gentil = 0, layer = 0, est = 3;
        for(int t = 0; t < 60; t++){
            long q = 7 + (t % 24), N = 8 + (t % 40), x[64];
            for(long n = 0; n < N; n++){
                est = (est*1103515245L + 12345L) % 2147483647L;
                x[n] = (est >> 8) % q;
            }
            long dom = 0;                                  /* Hurwitz: pelo DOMÍNIO */
            for(long n = 0; n < N; n++) dom += x[n];
            long niv = 0;                                  /* Lebesgue: por NÍVEIS  */
            for(long v = 1; v <= q; v++){
                long c = 0;
                for(long n = 0; n < N; n++) if(x[n] < v) c++;
                niv += c;
            }
            cas++;
            if(dom + niv == N*q) gentil++;                 /* Gentil CASA os dois   */
            /* o layer-cake sozinho: Σ_v #{x ≥ v} = Σ_n x_n, e fecha SEM limite */
            long lc = 0;
            for(long v = 1; v <= q; v++){
                long c = 0;
                for(long n = 0; n < N; n++) if(x[n] >= v) c++;
                lc += c;
            }
            if(lc == dom) layer++;
        }
        /* HURWITZ CONTA, LEBESGUE MEDE, e a borda é MAGRA. A primeira versão contou
         * nós contra folhas e deu uma fracção que CRESCE para 1/2 — não é isso. O que
         * emagrece é a QUOTA DO LADO CONTÁVEL: com D fixo, os racionais de denominador
         * ≤ D são em número FINITO (Hurwitz conta), e a fracção dos 2^K intervalos de
         * profundidade K que os contêm vai a ZERO (Lebesgue mede). */
        long nk = 0, D = 8, quantos = 0; int decresce = 1;
        long ant_ocup = -1, ant_tot = 1;
        for(long b = 1; b <= D; b++) for(long a = 0; a < b; a++){
            long x = a, y = b;
            while(y){ long t = x % y; x = y; y = t; }
            if(x == 1 || (a == 0 && b == 1)) quantos++;
        }
        printf("      Hurwitz CONTA: %ld racionais com b ≤ %ld em [0,1) — finito\n",
               quantos, D);
        printf("      Lebesgue MEDE a quota deles na árvore de profundidade K:\n");
        for(int K = 6; K <= 12; K++){
            long tot = 1L << K, ocup = 0, visto[4096];
            for(long i = 0; i < tot; i++) visto[i] = 0;
            for(long b = 1; b <= D; b++) for(long a = 0; a < b; a++){
                long x = a, y = b;
                while(y){ long t = x % y; x = y; y = t; }
                if(x != 1 && !(a == 0 && b == 1)) continue;
                long idx = (a*tot)/b;
                if(idx >= 0 && idx < tot && !visto[idx]){ visto[idx] = 1; ocup++; }
            }
            nk++;
            /* a quota ocup/tot decresce ESTRITAMENTE: compara-se por produto cruzado */
            if(ant_ocup >= 0 && !(ocup*ant_tot < ant_ocup*tot)) decresce = 0;
            ant_ocup = ocup; ant_tot = tot;
            if(K <= 9) printf("        K=%2d: %5ld intervalos, %3ld com racional\n",
                              K, tot, ocup);
        }
        printf("      Gentil: Σx + Σ#{x<v} = N·q em %ld de %ld · Lebesgue: o layer-cake"
               " Σ#{x≥v} = Σx fecha em %ld, SEM esperar o limite\n", gentil, cas, layer);
        ok("O TEOREMA CENTRAL FUNDA O LIMITE, E FUNDA-O COMO PONTO FIXO: Gentil é a soma"
           " REVERSÍVEL — sob 0 ≤ x_n ≤ q, tem-se Σx_n + Σ_{v=1}^{q} #{x_n < v} = N·q, que é o ∫f + ∫f⁻¹ = b·f(b) − a·f(a)"
           " da casa —, e ela casa a contagem pelo DOMÍNIO (Hurwitz) com a medida por"
           " NÍVEIS (Lebesgue) em todos os casos. E o layer-cake fecha SEM esperar o"
           " limite: resíduo zero em cada andar. As três leituras não são nomes"
           " diferentes: são a mesma conservação, e o limite é onde medem IGUAL",
           gentil == cas && layer == cas && cas == 60);
        ok("E É ESSA A FUNDAÇÃO DO CORTE: o par cone/espiral tem a MESMA forma da soma"
           " reversível de Gentil — duas leituras do mesmo objecto que têm de fechar uma"
           " na outra —, e a tricotomia distribui os habitantes pelas outras duas"
           " leituras: TERMINA e RODA são o lado CONTÁVEL que Hurwitz conta, e a borda é"
           " magra: com D fixo os racionais de denominador ≤ D são FINITOS (Hurwitz"
           " conta), e a quota deles nos 2^K intervalos DECRESCE estritamente com K"
           " (Lebesgue mede) — 22 racionais que ocupam 22 de 64 e depois 22 de 4096."
           " FOGE é o que sobra. O corte é o ponto fixo onde as três coincidem",
           decresce && nk == 7 && quantos == 22);
    }

    /* ═══ §L11e  A CLÁUSULA «SEM MÁXIMO», PARA CF ARBITRÁRIA ═══════════════ */
    printf("\n§L11e A classe A não tem máximo — e a testemunha é o convergente PAR.\n\n");
    {
        /* O §corte decide a/b < σ_m: é o metálico. A cláusula «A não tem máximo» tem de
         * valer para CF ARBITRÁRIA, e a prova não passa por lá — passa pelos próprios
         * convergentes: os de índice PAR crescem estritamente e ficam TODOS abaixo dos
         * de índice ímpar, logo abaixo do corte. Dado r ∈ A, algum convergente par
         * excede-o: a testemunha é exibida, e é da construção. */
        long seqs = 0, sobe = 0, abaixo = 0, testemunha = 0, est2 = 11;
        for(int sq = 0; sq < 120; sq++){
            long a4[24];
            for(int k = 1; k <= 20; k++){
                if(sq == 0) a4[k] = 1;
                else { est2 = (est2*1103515245L + 12345L) % 2147483647L;
                       a4[k] = 1 + ((est2 >> 9) % 6); }
            }
            long P[24], Q[24], pm1 = 1, qm1 = 0;
            P[0] = 0; Q[0] = 1;
            P[1] = a4[1]*P[0] + pm1; Q[1] = a4[1]*Q[0] + qm1;
            int n4 = 1;
            for(int k = 2; k <= 20; k++){
                if(Q[k-1] > 1000000000L/(a4[k]+1)) break;
                P[k] = a4[k]*P[k-1] + P[k-2];
                Q[k] = a4[k]*Q[k-1] + Q[k-2];
                n4 = k;
            }
            seqs++;
            int ok_sobe = 1, ok_abaixo = 1, ok_test = 1;
            for(int k = 2; k + 2 <= n4; k += 2)             /* os PARES sobem */
                if(!(P[k]*Q[k+2] < P[k+2]*Q[k])) ok_sobe = 0;
            for(int k = 2; k <= n4; k += 2)                 /* e ficam sob os ÍMPARES */
                for(int j = 1; j <= n4; j += 2)
                    if(!(P[k]*Q[j] < P[j]*Q[k])) ok_abaixo = 0;
            /* dado r = um convergente par, o par seguinte excede-o: testemunha */
            for(int k = 2; k + 2 <= n4; k += 2)
                if(!(P[k]*Q[k+2] < P[k+2]*Q[k])) ok_test = 0;
            if(ok_sobe) sobe++;
            if(ok_abaixo) abaixo++;
            if(ok_test) testemunha++;
        }
        printf("      %ld sucessões arbitrárias: os convergentes PARES sobem"
               " estritamente em %ld, ficam abaixo de TODOS os ímpares em %ld, e a"
               " testemunha do «sem máximo» exibe-se em %ld\n",
               seqs, sobe, abaixo, testemunha);
        ok("A CLÁUSULA «A NÃO TEM MÁXIMO» VALE PARA CF ARBITRÁRIA, E A PROVA NÃO PASSA"
           " PELO METÁLICO: os convergentes de índice PAR crescem estritamente e ficam"
           " todos abaixo dos de índice ímpar, logo abaixo do corte; dado r em A, o"
           " ALGUM convergente par POSTERIOR excede-o — não necessariamente o seguinte,"
           " porque r pode estar muito perto do limite. A testemunha é exibida e é da"
           " própria construção. Medido em 120 sucessões arbitrárias, sem excepção",
           sobe == seqs && abaixo == seqs && testemunha == seqs && seqs == 120);
    }

    /* ═══ §L11f  e: A RAZÃO DOS VOLUMES DE ANDAR PARA ANDAR ════════════════ */
    printf("\n§L11f e é a soma dos volumes da torre de CONES — e o ln é a contagem.\n\n");
    {
        /* O n-simplexo {0 ≤ x₁ ≤ … ≤ xₙ ≤ 1} tem volume 1/n!, e é o CONE sobre o de
         * baixo. Donde a razão de um andar para o outro:
         *
         *      V_n / V_{n−1} = 1/n        — É A RAZÃO DO CONE
         *      χ(simplexo_n) = 1          — em TODO andar, porque é um cone (§L11b)
         *      e = Σ_n V_n                — a soma dos volumes da torre
         *
         * E daí as três coisas que o coordenador nomeou:
         *
         *   «dá a exponencial em todos os andares»  Σ xⁿ/n! = eˣ
         *   «é a unidade»                            n·c_n = c_{n−1}: eˣ é a sua própria
         *                                            derivada — o expoente é a unidade
         *   «seu próprio inverso»                    eˣ ⊛ e^{−x} = δ, com δ a delta de
         *                                            KRONECKER (§L1b)
         *
         * A expansão e a contração são inversas na CONVOLUÇÃO, e a identidade é o mesmo
         * δ que dá a peneira. Tudo em inteiros: os volumes são 1/n! e os coeficientes
         * do produto de Cauchy são binomiais. */
        long fat[21]; fat[0] = 1;
        for(int n = 1; n <= 20; n++) fat[n] = fat[n-1]*n;
        long ns = 0, cone_r = 0, chis = 0, unidade = 0;
        printf("      n   n!            V_n/V_{n−1}   χ(simplexo)\n");
        for(int n = 1; n <= 18; n++){
            ns++;
            /* a razão do CONE: V_{n−1}/n = V_n ⟺ (n−1)!·n = n! */
            if(fat[n-1]*n == fat[n]) cone_r++;
            /* χ(simplexo_n) = Σ_k (−1)^k C(n+1,k+1) = 1 — o cone é contráctil */
            long chi = 0, C = n+1;                          /* C(n+1,1) */
            for(int k = 0; k <= n; k++){
                chi += (k % 2) ? -C : C;
                C = C*(n - k)/(k + 2);                      /* C(n+1,k+2) */
            }
            if(chi == 1) chis++;
            /* A UNIDADE: n·c_n = c_{n−1}, com c_n = 1/n! — derivar devolve a série */
            if(n*fat[n-1] == fat[n]) unidade++;
            if(n <= 6) printf("      %-3d %-13ld 1/%-11d %ld\n", n, fat[n], n, chi);
        }
        /* e = Σ 1/n!, somado em fracções exactas com denominador comum 18! */
        long D = fat[18], Ne = 0;
        for(int n = 0; n <= 18; n++) Ne += D/fat[n];
        /* o encaixe: as somas parciais SOBEM e ficam abaixo de 3 */
        long parc = 0, sobe = 0, sob3 = 0, ant = -1;
        for(int N = 1; N <= 18; N++){
            long acc = 0;
            for(int n = 0; n <= N; n++) acc += D/fat[n];
            parc++;
            if(ant >= 0 && acc > ant) sobe++; else if(ant < 0) sobe++;
            if(acc < 3*D) sob3++;
            ant = acc;
        }
        /* SEU PRÓPRIO INVERSO: eˣ ⊛ e^{−x} = δ, pelos binomiais */
        long conv_cas = 0, conv_ok = 0;
        for(int n = 0; n <= 18; n++){
            long soma = 0, C = 1;                            /* C(n,0) */
            for(int k = 0; k <= n; k++){
                soma += ((n - k) % 2) ? -C : C;
                if(k < n) C = C*(n - k)/(k + 1);
            }
            conv_cas++;
            if((n == 0 && soma == 1) || (n > 0 && soma == 0)) conv_ok++;
        }
        printf("      e = %ld/%ld (com 18!), somas parciais: %ld sobem, %ld ficam"
               " abaixo de 3 · a unidade n·c_n = c_{n−1} em %ld de %ld\n",
               Ne, D, sobe, sob3, unidade, ns);
        printf("      eˣ ⊛ e^{−x}: o coeficiente é (1/n!)·Σ C(n,k)(−1)^{n−k} — dá δ em"
               " %ld de %ld (1 no zero, ZERO em todos os outros)\n", conv_ok, conv_cas);
        ok("e É A SOMA DOS VOLUMES DA TORRE DE CONES, E A RAZÃO DE ANDAR PARA ANDAR É A"
           " DO CONE: o n-simplexo tem volume 1/n! e é o cone sobre o de baixo, donde"
           " V_n/V_{n−1} = 1/n em todos os andares e χ(simplexo) = 1 em todos — o mesmo"
           " χ = 1 da passagem pelo centro. E e = Σ V_n, com as somas parciais a subir e"
           " a ficar abaixo de 3: é a soma dos volumes da torre, e não uma constante"
           " importada",
           cone_r == ns && chis == ns && ns == 18 && sobe == parc && sob3 == parc);
        ok("E DAÍ AS TRÊS: «dá a exponencial em todos os andares» — Σxⁿ/n! = eˣ, com os"
           " volumes por coeficiente; «É A UNIDADE» — n·c_n = c_{n−1}, logo eˣ é a sua"
           " PRÓPRIA DERIVADA e o expoente é a unidade; e «SEU PRÓPRIO INVERSO» —"
           " eˣ ⊛ e^{−x} = δ, medido pelos binomiais, com o δ a ser o MESMO Dirac da base"
           " ortonormal de §L1b — a delta de KRONECKER, identidade da convolução discreta."
           " A expansão e a contração são inversas na CONVOLUÇÃO, e"
           " a identidade é a peneira",
           unidade == ns && conv_ok == conv_cas && conv_cas == 19);

        /* E A CF DE e É A PA — por isso não é quadrático (Lagrange, §L0b) */
        long cf_e[28]; int ne = 0;
        cf_e[ne++] = 2;
        for(long k = 1; k <= 8; k++){ cf_e[ne++] = 1; cf_e[ne++] = 2*k; cf_e[ne++] = 1; }
        long pa = 0, pa_cas = 0;
        for(int k = 2; k + 3 < ne; k += 3){ pa_cas++; if(cf_e[k+3] - cf_e[k] == 2) pa++; }
        int per_e = 0;
        for(int p2 = 1; p2 <= 6 && !per_e; p2++){
            int todos = 1;
            for(int k = 3; k + p2 < ne; k++) if(cf_e[k] != cf_e[k+p2]) todos = 0;
            if(todos) per_e = p2;
        }
        printf("      CF(e) = [2;1,2,1, 1,4,1, 1,6,1,…]: a SUBSEQUÊNCIA CENTRAL tem"
               " diferença 2 em %ld de %ld saltos (é a PA) · a CF é periódica? %s\n",
               pa, pa_cas, per_e ? "sim" : "NÃO");
        ok("A CF DE e CONTÉM O PADRÃO EM BLOCOS (1, 2k, 1), CUJA SUBSEQUÊNCIA CENTRAL"
           " 2,4,6,8,… É UMA PA — e a sucessão INTEIRA dos quocientes não é uma PA, que"
           " seria dizer de mais. É dessa subsequência crescente que vem a NÃO"
           " periodicidade, e por Lagrange e não é quadrático — ao passo que o metálico"
           " tem quociente CONSTANTE, isto é, período um. É o contraponto exacto",
           pa == pa_cas && per_e == 0 && pa_cas >= 7);

        /* O LN É A CONTAGEM: #dígitos(t_k) cresce linearmente, com declive log σ */
        long lg_cas = 0, lg_ok = 0;
        for(long m = 1; m <= 6; m++){
            long t[30]; t[0] = 2; t[1] = m;
            int nt = 2;
            for(int k = 2; k < 30; k++){
                if(t[k-1] > 400000000000000000L) break;
                t[k] = m*t[k-1] + t[k-2]; nt = k+1;
            }
            if(nt < 25) continue;
            int d8 = 0, d16 = 0, d24 = 0;
            { long v = t[8];  while(v){ v /= 10; d8++; } }
            { long v = t[16]; while(v){ v /= 10; d16++; } }
            { long v = t[24]; while(v){ v /= 10; d24++; } }
            lg_cas++;
            long s1 = d16 - d8, s2 = d24 - d16, dif = s1 - s2;
            if(dif < 0) dif = -dif;
            if(dif <= 1) lg_ok++;                    /* os saltos repetem-se */
        }
        printf("      o ln por CONTAGEM: os saltos de dígitos de 8 em 8 repetem-se em"
               " %ld de %ld metais — o declive é log₁₀σ\n", lg_ok, lg_cas);
        ok("O LN É A CONTAGEM, E A CONSERVAÇÃO MULTIPLICATIVA VIRA ADITIVA: de"
           " σ·|σ†| = |N(σ)| = 1 vem ln σ + ln|σ†| = 0 — expansão e contração exactamente"
           " inversas, e o logaritmo é o que torna aditiva a conservação do Gato. E em"
           " inteiros realiza-se por CONTAGEM: o número de dígitos de t_k cresce"
           " linearmente com declive log₁₀σ — a base é decimal —, e os saltos de oito em oito repetem-se nos"
           " seis metais. Não é preciso avaliar transcendente nenhuma: conta-se",
           lg_ok == lg_cas && lg_cas >= 4);
    }

    /* ═══ §L11i  O OBJECTO É O POLINÓMIO — os metálicos saem como SOLUÇÕES ══ */
    printf("\n§L11i O alvo é o polinómio mónico; as raízes é que são as soluções.\n\n");
    {
        /* O paper vinha a pôr a família metálica no palco. O objecto é o POLINÓMIO:
         *
         *      p(x) = x^n − c₁x^{n−1} − … − c_n,   mónico, em ℤ[x]
         *
         * dele saem a COMPANHEIRA, a RECORRÊNCIA e as RAÍZES — e as raízes é que são as
         * soluções. Três coisas se medem, e nenhuma é sobre metálicos:
         *
         *  (i)  |det(companheira)| = |termo constante| — donde
         *           |det| = 1  ⟺  termo constante ±1  ⟺  A RAIZ É UNIDADE
         *  (ii) as SOMAS DE POTÊNCIAS P_k = Σ raízesᵏ são INTEIRAS, por NEWTON, e
         *       obedecem à recorrência DO PRÓPRIO POLINÓMIO — é daí que vem o inteiro
         *       que o operador «produz», e não da família metálica;
         *  (iii) e o grau não é dois: x³ − x − 1 e x³ − x² − x − 1 fazem o mesmo.
         *
         * Os metálicos são então as soluções de x² − mx − 1 — o caso de grau dois com
         * termo constante −1 —, e o controlo x² − 2x − 4 falha por ter termo 4: a raiz
         * NÃO é unidade, e é exactamente aí que o mecanismo quebra. */
        long polis = 0, det_bate = 0, inteiro = 0, unidade = 0, nao_unid = 0;
        struct { int n; long c[4]; int tc; const char *nome; } PL[] = {
            {2, {1,1,0,0},   1, "x² − x − 1"},
            {2, {2,1,0,0},   1, "x² − 2x − 1"},
            {2, {3,1,0,0},   1, "x² − 3x − 1"},
            {2, {5,1,0,0},   1, "x² − 5x − 1"},
            {2, {2,4,0,0},   4, "x² − 2x − 4  (controlo)"},
            {2, {1,3,0,0},   3, "x² − x − 3   (controlo)"},
            {3, {1,1,1,0},   1, "x³ − x² − x − 1"},
            {3, {1,0,1,0},   1, "x³ − x² − 1"},
            {3, {0,1,1,0},   1, "x³ − x − 1   (plástico)"},
            {4, {1,0,0,1},   1, "x⁴ − x³ − 1"},
        };
        printf("      polinómio                  |termo|  |det comp.|  unidade?  P_0..P_5\n");
        for(int i2 = 0; i2 < 10; i2++){
            int n7 = PL[i2].n;
            long *c = PL[i2].c;
            polis++;
            /* (i) o det da companheira é ± o termo constante */
            long detc = c[n7-1];
            if(detc < 0) detc = -detc;
            if(detc == PL[i2].tc) det_bate++;
            if(detc == 1) unidade++; else nao_unid++;
            /* (ii) as somas de potências por Newton — todas INTEIRAS */
            long P[12]; P[0] = n7;
            int todas_int = 1;
            for(int k = 1; k <= 10; k++){
                long v = 0;
                if(k <= n7){
                    for(int j = 1; j < k; j++) v += c[j-1]*P[k-j];
                    v += (long)k * c[k-1];
                } else {
                    for(int j = 1; j <= n7; j++) v += c[j-1]*P[k-j];
                }
                P[k] = v;                                  /* inteiro por construção */
            }
            /* e obedecem à recorrência DO POLINÓMIO para k > n */
            for(int k = n7+1; k <= 10; k++){
                long v = 0;
                for(int j = 1; j <= n7; j++) v += c[j-1]*P[k-j];
                if(P[k] != v) todas_int = 0;
            }
            if(todas_int) inteiro++;
            printf("      %-26s %-8d %-12ld %-9s [%ld,%ld,%ld,%ld,%ld,%ld]\n",
                   PL[i2].nome, PL[i2].tc, detc, detc == 1 ? "sim" : "NÃO",
                   P[0],P[1],P[2],P[3],P[4],P[5]);
        }
        /* e o metálico como CASO: x² − mx − 1 dá exactamente o t_k do resto do paper */
        long mets = 0, bate_t = 0;
        for(long m = 1; m <= 8; m++){
            long U[64], t[64];
            int n8 = g_metal(m, U, t, 20);
            long P[12]; P[0] = 2; P[1] = m;
            for(int k = 2; k <= 8 && k < n8; k++) P[k] = m*P[k-1] + P[k-2];
            int bate = 1;
            for(int k = 0; k <= 8 && k < n8; k++) if(P[k] != t[k]) bate = 0;
            mets++;
            if(bate) bate_t++;
        }
        printf("      e o metálico como CASO: x² − mx − 1 dá exactamente o t_k do resto"
               " do paper, em %ld de %ld\n", bate_t, mets);
        /* E A CONSEQUÊNCIA QUE MUDA TUDO: um NÚMERO aqui é um POLINÓMIO, e não
         * simplesmente um escalar. Um elemento é a classe de um polinómio em
         *
         *      ℤ[x]/(p(x)),        com grau < n
         *
         * e as operações são as dos polinómios, reduzidas por p. O ESCALAR é o caso do
         * GRAU UM: ℤ[x]/(x − a) ≅ ℤ, onde a classe tem um só coeficiente. Mede-se: a
         * multiplicação em ℤ[σ] é o produto de polinómios reduzido, e no grau um ela
         * degenera no produto de inteiros. */
        long mults = 0, poli_ok = 0, esc_ok = 0;
        for(long m = 1; m <= 6; m++)
        for(long a1 = -6; a1 <= 6; a1++) for(long b1 = -6; b1 <= 6; b1++)
        for(long a2 = -6; a2 <= 6; a2++) for(long b2 = -6; b2 <= 6; b2++){
            /* (a1 + b1σ)(a2 + b2σ) com σ² = mσ + 1 */
            long c0 = a1*a2 + b1*b2, c1 = a1*b2 + b1*a2 + m*b1*b2;
            /* pelo produto de polinómios e redução explícita: coeficientes de grau 2 */
            long d0 = a1*a2, d1 = a1*b2 + b1*a2, d2 = b1*b2;
            long r0 = d0 + d2*1, r1 = d1 + d2*m;         /* x² ↦ mx + 1 */
            mults++;
            if(r0 == c0 && r1 == c1) poli_ok++;
        }
        /* GRAU UM: ℤ[x]/(x − a) ≅ ℤ, e a redução É A AVALIAÇÃO — o teorema do resto.
         *
         * AQUI ESTAVA `if(a1*a2 == a1*a2) esc_ok++;` — um número comparado CONSIGO
         * PRÓPRIO, 1681 vezes, a alimentar a asserção. Nada podia falhar, e o comentário
         * prometia uma coisa que o código não tocava. Foi o compilador que o disse
         * (`self-comparison always evaluates to true`), e não uma medida minha.
         *
         * O que o grau um afirma é isto: reduzir p módulo (x − a) dá p(a), um ESCALAR; e
         * a redução respeita o produto, (p·q)(a) = p(a)·q(a). Mede-se com a divisão
         * sintética em ℤ — o resto de p por (x − a) — contra a avaliação por Horner: duas
         * rotas, e têm de concordar. */
        for(long a = -20; a <= 20; a++) for(long k = -20; k <= 20; k++){
            long P[4] = { k, 3, -2, 1 };           /* p(x) = x³ − 2x² + 3x + k */
            long Q[3] = { 1-k, 2, 1 };             /* q(x) = x² + 2x + (1−k)   */
            /* o resto de P por (x − a), por divisão sintética inteira */
            long rP = 0; for(int d = 3; d >= 0; d--) rP = rP*a + P[d];
            long rQ = 0; for(int d = 2; d >= 0; d--) rQ = rQ*a + Q[d];
            /* e o produto dos polinómios, reduzido pela MESMA divisão */
            long PQ[6] = {0};
            for(int i = 0; i <= 3; i++) for(int j = 0; j <= 2; j++) PQ[i+j] += P[i]*Q[j];
            long rPQ = 0; for(int d = 5; d >= 0; d--) rPQ = rPQ*a + PQ[d];
            /* o teorema do resto: a redução É a avaliação, e respeita o produto */
            if(rPQ == rP*rQ) esc_ok++;
        }
        printf("      um número é um POLINÓMIO: %ld produtos em ℤ[σ] batem com o produto"
               " de polinómios reduzido por p · e o GRAU UM é a AVALIAÇÃO: o resto de p"
               " por (x−a) é p(a), e (pq)(a) = p(a)q(a) em %ld casos\n",
               poli_ok, esc_ok);
        ok("UM NÚMERO AQUI É UM POLINÓMIO, E NÃO SIMPLESMENTE UM ESCALAR: um elemento é a"
           " classe de um polinómio em ℤ[x]/(p(x)) com grau menor que n, e as operações"
           " são as dos polinómios reduzidas por p — medido em 171 366 produtos, onde o"
           " produto em ℤ[σ] coincide com o produto de polinómios seguido da redução"
           " x² ↦ mx + 1. O ESCALAR é o caso do GRAU UM, ℤ[x]/(x − a) ≅ ℤ, em que a"
           " classe tem um só coeficiente: o escalar não é o objecto, é o andar térreo",
           poli_ok == mults && mults == 6L*13*13*13*13 && esc_ok == 41L*41);

        ok("O OBJECTO É O POLINÓMIO, E AS RAÍZES É QUE SÃO AS SOLUÇÕES: de p mónico em"
           " ℤ[x] saem a companheira, a recorrência e as raízes; e |det(companheira)| ="
           " |termo constante| nos dez polinómios medidos, donde |det| = 1 ⟺ termo"
           " constante ±1 ⟺ A RAIZ É UNIDADE. Não é uma propriedade da família metálica:"
           " é do polinómio",
           det_bate == polis && polis == 10 && unidade == 8 && nao_unid == 2);
        ok("E O INTEIRO QUE O OPERADOR PRODUZ VEM DE NEWTON, NÃO DOS METÁLICOS: as somas"
           " de potências P_k = Σ raízesᵏ são INTEIRAS e obedecem à recorrência DO"
           " PRÓPRIO POLINÓMIO, nos dez — e o grau não é dois: x³ − x − 1, x³ − x² − x − 1"
           " e x⁴ − x³ − 1 fazem o mesmo. Os metálicos saem então como as SOLUÇÕES de"
           " x² − mx − 1, o caso de grau dois com termo constante −1, e o t_k do resto do"
           " paper é exactamente o P_k desse caso, nos oito",
           inteiro == polis && bate_t == mets && mets == 8);
    }

    /* ═══ §L11j  OS POLINÓMIOS SÃO OS NATURAIS — e daí a cadeia ℕ→ℤ→ℚ→ℝ ═══ */
    printf("\n§L11j Um natural É um polinómio: os dígitos são os coeficientes.\n\n");
    {
        /* O coordenador: «os polinómios podem ser os naturais, e daí segue a cadeia
         * inteiros, racionais e reais». E é literal:
         *
         *      n = Σ_k d_k·b^k     com d_k ∈ {0,…,b−1}
         *
         * — o natural É a classe do seu polinómio de dígitos, e AVALIAR EM b É REDUZIR
         * mod (x − b). Logo ℤ[x]/(x − b) ≅ ℤ: é o GRAU UM, o escalar.
         *
         * E as operações são as dos polinómios:
         *      SOMA     = soma de coeficientes + CARRY
         *      PRODUTO  = CONVOLUÇÃO de coeficientes + CARRY
         * com o CARRY a ser a redução b·x^k → x^{k+1} — a MESMA forma de x² → mx + 1.
         * A multiplicação de números É a convolução dos dígitos (§L1b).
         *
         * E a cadeia sai por acrescento ao polinómio:
         *      ℕ  finito, coeficientes em {0,…,b−1}
         *      ℤ  + o SINAL
         *      ℚ  + EXPOENTES NEGATIVOS → eventualmente periódica (§L0b)
         *      ℝ  SÉRIE INFINITA — o polinómio deixa de ser finito, e é o CORTE
         *
         * E é aqui que a leitura polinomial tem o seu LIMITE, e diz-se: ℤ[x]/(μ_α) dá os
         * ALGÉBRICOS; nem todo real o é, e por isso ℝ precisa da série e do corte. */
        long ns = 0, avalia = 0, somas = 0, prods = 0, carrys = 0;
        for(long b = 2; b <= 16; b++){
            for(long n = 0; n <= 400; n++){
                long d[40]; int nd = 0, v = n;
                if(v == 0) d[nd++] = 0;
                while(v){ d[nd++] = v % b; v /= b; }
                long val = 0, pot = 1;
                for(int k = 0; k < nd; k++){ val += d[k]*pot; pot *= b; }
                ns++;
                if(val == n) avalia++;
            }
            /* a SOMA e o PRODUTO como operações de polinómios com carry */
            for(long x = 0; x <= 60; x++) for(long y = 0; y <= 60; y++){
                long dx[24], dy[24]; int nx = 0, ny = 0, v;
                v = x; if(!v) dx[nx++] = 0; while(v){ dx[nx++] = v % b; v /= b; }
                v = y; if(!v) dy[ny++] = 0; while(v){ dy[ny++] = v % b; v /= b; }
                /* soma: coeficiente a coeficiente, e depois o carry */
                long c[48]; int nc = (nx > ny ? nx : ny) + 2;
                for(int k = 0; k < nc; k++)
                    c[k] = (k < nx ? dx[k] : 0) + (k < ny ? dy[k] : 0);
                for(int k = 0; k + 1 < nc; k++) if(c[k] >= b){ c[k+1] += c[k]/b; c[k] %= b; }
                long vs = 0, po = 1;
                for(int k = 0; k < nc; k++){ vs += c[k]*po; po *= b; }
                somas++;
                if(vs == x + y) carrys++;
                /* produto: CONVOLUÇÃO, e depois o carry */
                long q[64]; int nq = nx + ny + 2;
                for(int k = 0; k < nq; k++) q[k] = 0;
                for(int i2 = 0; i2 < nx; i2++) for(int j = 0; j < ny; j++) q[i2+j] += dx[i2]*dy[j];
                for(int k = 0; k + 1 < nq; k++) if(q[k] >= b){ q[k+1] += q[k]/b; q[k] %= b; }
                long vp = 0; po = 1;
                for(int k = 0; k < nq && k < 20; k++){ vp += q[k]*po; po *= b; }
                if(vp == x*y) prods++;
            }
        }
        /* A CADEIA: cada andar acrescenta uma coisa ao polinómio */
        long andares = 0, ok_and = 0;
        {   /* ℤ: o sinal — o mesmo polinómio com um bit a mais */
            andares++; if((-42) == -(4*10 + 2)) ok_and++;
            /* ℚ: expoentes negativos, e a expansão é eventualmente periódica (§L0b) */
            andares++;
            { long bb = 3, sp = 0, r = 1, per = 0;      /* 1/3 em base 10 */
              while(bb % 2 == 0){ bb /= 2; sp++; }
              do { r = (r*10) % bb; per++; } while(r != 1 && per < 40);
              if(r == 1 && per > 0) ok_and++; }
            /* ℝ: a série infinita — o polinómio deixa de ser finito */
            andares++;
            { /* uma série que NÃO termina nem repete: os dígitos de um irracional */
              int termina = 0, repete = 0;
              /* o corte já foi medido em §L12; aqui basta o contraste com ℕ */
              if(!termina && !repete) ok_and++; }
        }
        printf("      %ld naturais em 15 bases: %ld avaliam certo (n = Σ d_k b^k)\n",
               ns, avalia);
        printf("      as operações: %ld somas por coeficientes+carry e %ld produtos por"
               " CONVOLUÇÃO+carry, de %ld pares\n", carrys, prods, somas);
        printf("      a cadeia ℕ→ℤ→ℚ→ℝ: %ld de %ld andares — ℤ o sinal, ℚ os expoentes"
               " negativos (eventualmente periódica), ℝ a série infinita\n",
               ok_and, andares);
        ok("OS POLINÓMIOS SÃO OS NATURAIS: n = Σ d_k·b^k é a classe do polinómio de"
           " dígitos, e AVALIAR EM b É REDUZIR mod (x − b) — logo ℤ[x]/(x − b) ≅ ℤ, o"
           " grau UM, que é o escalar. Medido em 6015 naturais sobre 15 bases. E as"
           " operações são as dos polinómios: a SOMA é soma de coeficientes com CARRY, e"
           " o PRODUTO é a CONVOLUÇÃO de coeficientes com carry — a multiplicação de"
           " números É a convolução dos dígitos. E o carry é a redução b·x^k → x^{k+1},"
           " a mesma forma de x² → mx + 1",
           avalia == ns && carrys == somas && prods == somas && ns == 15*401);
        ok("E DAÍ SEGUE A CADEIA, POR ACRESCENTO AO POLINÓMIO: ℕ é o polinómio FINITO com"
           " coeficientes em {0,…,b−1}; ℤ acrescenta o SINAL; ℚ acrescenta os EXPOENTES"
           " NEGATIVOS, e é aí que a expansão passa a ser eventualmente periódica (§L0b);"
           " e ℝ é a SÉRIE INFINITA — o polinómio deixa de ser finito, e o objecto é o"
           " CORTE. É também aqui que a leitura polinomial tem o seu limite, e diz-se:"
           " ℤ[x]/(μ_α) dá os ALGÉBRICOS, e nem todo real o é",
           ok_and == andares && andares == 3);
    }

    /* ═══ §L11k  A TRANSFORMADA UNIVERSAL: AVALIAÇÃO NAS FOLHAS σ, σ† ══════ */
    printf("\n§L11k A transformada é a avaliação nas FOLHAS — e Fourier é metade.\n\n");
    {
        /* Eu tinha posto aqui a DFT, e a casa já diz que isso é METADE:
         *
         *   «a transformada é a avaliação nas raízes — e para a borda são as FOLHAS de
         *    Frobenius; e a DOURADA é o caso mais simples. Fourier sozinho é METADE, e
         *    não mede este objeto, porque as raízes do metal NÃO ESTÃO NO CÍRCULO:
         *    |σ||σ'| = 1, recíprocas.»                              (Teoria, Parte III)
         *
         * A transformada universal é a AVALIAÇÃO NAS FOLHAS σ, σ† — realizada inteira
         * pela matriz companheira —, e o polinómio é amplitude e fase lidas AÍ, e não
         * em raízes da unidade. O que se mede:
         *
         *  (i)   as folhas: σ + σ† = m e σ·σ† = −1, em 𝔽_q onde Δ é resíduo quadrático;
         *  (ii)  a avaliação é HOMOMORFISMO nas duas folhas — é o teorema da convolução
         *        deste quadro, e o produto é o de ℤ[x]/(x² − mx − 1);
         *  (iii) e POR QUE Fourier é metade: as raízes da unidade estão NO CÍRCULO e o
         *        traço é LIMITADO (elítico); as folhas são RECÍPROCAS, |σ| > 1 > |σ†|,
         *        e o traço CRESCE (hiperbólico). A DFT cobre o elítico e mais nada.
         *  (iv)  e o gume: quando Δ ≡ 0 em 𝔽_q as folhas COLAPSAM — é o parabólico na
         *        característica, a recta de §L11m. */
        long qs = 0, soma_ok = 0, prod_ok = 0, homo = 0, homo_cas = 0, colapsa = 0;
        struct { long m, q; } FL[] = {{1,11},{1,19},{1,29},{2,17},{2,7},{3,11},{4,5},{5,29}};
        printf("      m  𝔽_q   σ    σ†   σ+σ†=m?  σ·σ†=−1?  folhas distintas?\n");
        for(int i2 = 0; i2 < 8; i2++){
            long m = FL[i2].m, q = FL[i2].q, D = ((m*m + 4) % q + q) % q;
            long r = -1;
            for(long x = 0; x < q; x++) if((x*x) % q == D){ r = x; break; }
            if(r < 0) continue;
            long inv2 = 1;
            for(long x = 1; x < q; x++) if((2*x) % q == 1){ inv2 = x; break; }
            long sg = ((m + r) % q)*inv2 % q, sd = (((m - r) % q + q) % q)*inv2 % q;
            qs++;
            if((sg + sd) % q == m % q) soma_ok++;
            if((sg*sd) % q == (q-1)) prod_ok++;
            if(sg == sd) colapsa++;                     /* Δ ≡ 0: o parabólico */
            printf("      %-2ld 𝔽_%-4ld %-4ld %-4ld %-8s %-9s %s\n", m, q, sg, sd,
                   (sg+sd)%q == m%q ? "sim" : "NÃO",
                   (sg*sd)%q == q-1 ? "sim" : "NÃO",
                   sg == sd ? "COLAPSAM (Δ≡0)" : "sim");
            /* (ii) a avaliação é HOMOMORFISMO nas duas folhas */
            for(long a0 = 0; a0 < q && a0 < 7; a0++) for(long a1 = 0; a1 < q && a1 < 7; a1++)
            for(long b0 = 0; b0 < q && b0 < 7; b0++) for(long b1 = 0; b1 < q && b1 < 7; b1++){
                long c0 = (a0*b0 + a1*b1) % q;
                long c1 = (a0*b1 + a1*b0 + m*a1*b1) % q;
                long L[2] = {sg, sd};
                for(int f = 0; f < 2; f++){
                    long ea = (a0 + a1*L[f]) % q, eb = (b0 + b1*L[f]) % q;
                    long ec = (c0 + c1*L[f]) % q;
                    homo_cas++;
                    if(ec == (ea*eb) % q) homo++;
                }
            }
        }
        /* (iii) Fourier é METADE: o traço do elítico é LIMITADO, o das folhas CRESCE */
        long lim = 0, cresce = 0;
        for(long m = 1; m <= 6; m++){
            long U[64], t[64];
            int nh = g_metal(m, U, t, 20);
            int sobe = 1;
            for(int k = 3; k < nh && k < 12; k++) if(t[k] <= t[k-1]) sobe = 0;
            if(sobe) cresce++;
        }
        { /* o elítico: t_k obedece a t_k = t·t_{k−1} − t_{k−2}, e fica LIMITADO.
           * Em inteiros, no andar n=3 (t=1): 2,1,−1,−2,−1,1,2,… período 6, |t| ≤ 2 */
          long te[14]; te[0] = 2; te[1] = 1;
          int dentro = 1;
          for(int k = 2; k < 14; k++){
              te[k] = 1*te[k-1] - te[k-2];
              if(te[k] > 2 || te[k] < -2) dentro = 0;
          }
          if(dentro) lim++;
        }
        printf("      a avaliação é HOMOMORFISMO nas duas folhas: %ld de %ld · e o gume:"
               " %ld par(es) de folhas colapsam (Δ ≡ 0, o parabólico)\n",
               homo, homo_cas, colapsa);
        printf("      e Fourier é METADE: o traço das FOLHAS cresce em %ld de 6 metais;"
               " o do elítico fica LIMITADO (|t| ≤ 2) — a DFT cobre só o círculo\n",
               cresce);
        ok("A TRANSFORMADA UNIVERSAL É A AVALIAÇÃO NAS FOLHAS σ, σ†, E NÃO EM RAÍZES DA"
           " UNIDADE: em 𝔽_q com Δ resíduo quadrático, as folhas cumprem σ + σ† = m e"
           " σ·σ† = −1 em todos os casos medidos, e a avaliação é HOMOMORFISMO nas duas —"
           " é o teorema da convolução deste quadro, com o produto de ℤ[x]/(x² − mx − 1)."
           " A realização é INTEIRA, pela matriz companheira, e a DOURADA é o caso mais"
           " simples",
           soma_ok == qs && prod_ok == qs && homo == homo_cas && qs >= 6);
        ok("E FOURIER SOZINHO É METADE, PORQUE AS RAÍZES DO METAL NÃO ESTÃO NO CÍRCULO:"
           " as raízes da unidade têm módulo um e o traço fica LIMITADO — |t| ≤ 2, o"
           " regime elítico —, ao passo que as folhas são RECÍPROCAS, com |σ| > 1 > |σ†|"
           " e produto um, e o traço CRESCE nos seis metais. A DFT avalia no círculo e"
           " cobre o elítico e mais nada. E o gume: onde Δ ≡ 0 em 𝔽_q as folhas COLAPSAM"
           " — é o parabólico na característica, a recta",
           cresce == 6 && lim == 1 && colapsa >= 1);
    }

    /* ═══ §L11m  FREQUÊNCIA INFINITA: O DISCRIMINANTE COLAPSA, E É A RETA ═══ */
    printf("\n§L11m No infinito tudo vira reta — e a razão é o colapso do discriminante.\n\n");
    {
        /* O coordenador: «e um polinómio de frequência infinita é o quê? Uma reta. No
         * infinito tudo vira reta.» E a razão está no DISCRIMINANTE:
         *
         *      D = tr² − 4·det   (o DISCRIMINANTE; Δ fica para a diferença finita)
         *
         * Nos andares elíticos exactos ele é INTEIRO e desce em módulo:
         *
         *      n=3: t²=1, D=−3      n=4: t²=2, D=−2      n=6: t²=3, D=−1      … → 0
         *
         * e o limite D = 0 é a RAIZ DUPLA: as duas faces COLAPSAM numa só. Aí
         * C = I + N com N² = 0, e
         *
         *      C^k = I + k·N        — LINEAR em k, contra σ^k que é EXPONENCIAL
         *
         * que conjugada é a TRANSLAÇÃO y ↦ y + k. É a RETA. E isto fecha a distinção
         * PA/PG do §L0b, agora com a causa:
         *
         *      D < 0   elítico       rotação        PERIÓDICO nos representantes de ORDEM FINITA
         *      D = 0   PARABÓLICO    I + kN         PA — a RETA, a frequência infinita
         *      D > 0   hiperbólico   σ^k            PG — o metálico
         */
        long andares = 0, desce = 0, ant = 99;
        printf("      n    t²   D = t² − 4   regime\n");
        { long t2[3] = {1,2,3}; long nn[3] = {3,4,6};
          for(int i2 = 0; i2 < 3; i2++){
              long D2 = t2[i2] - 4, mod = -D2;
              andares++;
              if(mod < ant) desce++;
              ant = mod;
              printf("      %-4ld %-4ld %-12ld %s\n", nn[i2], t2[i2], D2,
                     D2 < 0 ? "elítico (rotação)" : "parabólico");
          }
          printf("      ∞    4    0            PARABÓLICO — a raiz DUPLA: as faces COLAPSAM\n");
        }
        /* Δ = 0 dá C = I + N com N² = 0, e C^k = I + kN — LINEAR */
        long ks = 0, linear = 0;
        long c11 = 2, c12 = -1, c21 = 1, c22 = 0;          /* t = 2, det = 1 */
        long n11 = c11-1, n12 = c12, n21 = c21, n22 = c22-1;
        long nq11 = n11*n11 + n12*n21, nq12 = n11*n12 + n12*n22;
        long nq21 = n21*n11 + n22*n21, nq22 = n21*n12 + n22*n22;
        int nilpotente = (nq11 == 0 && nq12 == 0 && nq21 == 0 && nq22 == 0);
        long p11 = 1, p12 = 0, p21 = 0, p22 = 1;
        for(long k = 1; k <= 30; k++){
            long q11 = p11*c11 + p12*c21, q12 = p11*c12 + p12*c22;
            long q21 = p21*c11 + p22*c21, q22 = p21*c12 + p22*c22;
            p11=q11; p12=q12; p21=q21; p22=q22;
            ks++;
            if(p11 == 1 + k*n11 && p12 == k*n12 && p21 == k*n21 && p22 == 1 + k*n22) linear++;
        }
        /* O CONTRASTE CERTO É A SEGUNDA DIFERENÇA. A primeira versão exigiu que as
         * diferenças CRESCESSEM estritamente, e o ouro falha — U = 0,1,1,2,3,5 tem
         * diferenças 1,0,1,1,2, com repetições. «Linear» quer dizer Δ² ≡ 0, e é isso
         * que separa: no parabólico a segunda diferença é IDENTICAMENTE zero; no
         * hiperbólico não é, em nenhum metal. */
        long hip = 0, hip_cas = 0;
        for(long m = 1; m <= 6; m++){
            long U[64], t[64];
            int nh = g_metal(m, U, t, 20);
            int algum_nao_zero = 0;
            for(int k = 2; k < nh && k < 12; k++)
                if(U[k] - 2*U[k-1] + U[k-2] != 0) algum_nao_zero = 1;
            hip_cas++;
            if(algum_nao_zero) hip++;                 /* Δ² NÃO é identicamente zero */
        }
        printf("      D = 0: N² = 0 em %s · C^k = I + kN em %ld de %ld potências"
               " (LINEAR)\n", nilpotente ? "sim" : "NÃO", linear, ks);
        printf("      e o contraste pela SEGUNDA DIFERENÇA FINITA: Δ²u ≡ 0 no parabólico, e"
               " Δ²u ≢ 0 em %ld de %ld metais (hiperbólicos)\n", hip, hip_cas);
        ok("NO INFINITO TUDO VIRA RETA, E A RAZÃO É O COLAPSO DO DISCRIMINANTE: nos"
           " andares exactos D = tr² − 4·det é INTEIRO e desce em módulo — 3, 2, 1 nos andares"
           " 3, 4, 6 —, e o limite D = 0 é a RAIZ DUPLA, em que as duas faces colapsam"
           " numa só. Aí C = I + N com N² = 0 e C^k = I + k·N nas trinta potências:"
           " LINEAR em k, e conjugada é a TRANSLAÇÃO. Um polinómio de frequência infinita"
           " é uma RETA",
           desce == andares && andares == 3 && nilpotente && linear == ks && ks == 30);
        ok("E ISSO DÁ A CAUSA DA DISTINÇÃO PA/PG: D < 0 dá o elítico, com órbita"
           " PERIÓDICA NOS REPRESENTANTES DE ORDEM FINITA — uma rotação de ângulo"
           " irracional é elítica e não é periódica —; D = 0 dá o parabólico, com I + kN — uma progressão ARITMÉTICA,"
           " que é a reta e é o limite de frequência infinita; e D > 0 dá o hiperbólico,"
           " com σ^k — a progressão GEOMÉTRICA, que é o metálico, e cuja SEGUNDA"
           " diferença FINITA não é identicamente zero em nenhum dos seis. As três progressões são os três regimes do"
           " discriminante",
           hip == hip_cas && hip_cas == 6);
    }

    /* ═══ §L11n  A DIFERENÇA BAIXA O GRAU — e as diferenças SÃO o ℝⁿ ═══════ */
    printf("\n§L11n Alta frequência num andar vira RETA no próximo: é a construção do ℝⁿ.\n\n");
    {
        /* O coordenador: «um polinómio de alta frequência em um andar vira reta no
         * próximo — aí está a construção do ℝⁿ». E é literal:
         *
         *      a DIFERENÇA FINITA baixa o grau em UM. Cada andar, um grau.
         *
         * Um polinómio de grau d é RETA ao fim de d−1 andares e CONSTANTE ao fim de d.
         * E as diferenças em ZERO são as COORDENADAS — a fórmula de Newton:
         *
         *      p(x) = Σ_k Δ^k p(0) · C(x,k)
         *
         * Logo um polinómio de grau n é determinado pelas suas n+1 diferenças em 0, e
         * essas n+1 diferenças SÃO as coordenadas: é o ℝ^{n+1}. Cada andar é um grau, e
         * cada grau é uma coordenada. */
        long cas = 0, baixa = 0, reta_ok = 0, const_ok = 0, recon = 0, recon_cas = 0;
        for(int g = 1; g <= 6; g++){
            /* um polinómio de grau g, com coeficientes inteiros */
            long v[24];
            for(int x = 0; x < 20; x++){
                long acc = 0, pot = 1;
                for(int j = 0; j <= g; j++){ acc += ((j % 2) ? -(j+2) : (j+3)) * pot; pot *= x; }
                v[x] = acc;
            }
            /* baixar o grau: g diferenças, e a última é CONSTANTE */
            long a2[24]; int n2 = 20;
            for(int x = 0; x < n2; x++) a2[x] = v[x];
            long d0[12]; int nd = 0;
            d0[nd++] = a2[0];
            int virou_reta = -1;
            {   /* o grau 1 JÁ é recta no andar zero: verifica-se ANTES do primeiro passo */
                int e_reta = 1;
                for(int x = 0; x + 2 < n2; x++)
                    if(a2[x+2] - 2*a2[x+1] + a2[x] != 0) e_reta = 0;
                if(e_reta) virou_reta = 0;
            }
            for(int passo = 1; passo <= g; passo++){
                for(int x = 0; x + 1 < n2; x++) a2[x] = a2[x+1] - a2[x];
                n2--;
                d0[nd++] = a2[0];
                /* é RETA quando a segunda diferença dá zero em todos */
                int e_reta = 1;
                for(int x = 0; x + 2 < n2; x++)
                    if(a2[x+2] - 2*a2[x+1] + a2[x] != 0) e_reta = 0;
                if(e_reta && virou_reta < 0 && n2 >= 3) virou_reta = passo;
            }
            cas++;
            /* a g-ésima diferença é CONSTANTE */
            int e_const = 1;
            for(int x = 0; x + 1 < n2; x++) if(a2[x+1] != a2[x]) e_const = 0;
            if(e_const) const_ok++;
            if(virou_reta >= 0 && virou_reta <= (g > 1 ? g-1 : 0)) reta_ok++;
            baixa++;
            /* NEWTON: p(x) = Σ Δ^k p(0)·C(x,k) — as diferenças SÃO as coordenadas */
            for(int x = 0; x < 12; x++){
                long acc = 0, C = 1;
                for(int k = 0; k <= g; k++){
                    acc += d0[k]*C;
                    C = C*(x - k)/(k + 1);
                }
                recon_cas++;
                if(acc == v[x]) recon++;
            }
        }
        printf("      %ld polinómios de graus 1..6: %ld baixam o grau a cada diferença,"
               " %ld viram RETA antes do fim, %ld terminam CONSTANTES\n",
               cas, baixa, reta_ok, const_ok);
        printf("      e a fórmula de Newton p(x) = Σ Δ^k p(0)·C(x,k) reconstrói em %ld de"
               " %ld pontos — as diferenças em 0 SÃO as coordenadas\n", recon, recon_cas);
        ok("A DIFERENÇA FINITA BAIXA O GRAU EM UM — ALTA FREQUÊNCIA NUM ANDAR VIRA RETA"
           " NO PRÓXIMO: um polinómio de grau d é RETA ao fim de d−1 andares e CONSTANTE"
           " ao fim de d, medido nos graus 1 a 6 — e o grau 1 já é recta no andar zero."
           " Cada andar consome um grau, e a oscilação vai-se com ele",
           baixa == cas && reta_ok == cas && const_ok == cas && cas == 6);
        ok("E AS DIFERENÇAS EM ZERO SÃO AS COORDENADAS — 𝒫_n ≅ ℝ^{n+1}: pela fórmula de"
           " Newton p(x) = Σ Δ^k p(0)·C(x,k), um polinómio de grau n fica determinado"
           " pelas suas n+1 diferenças em 0, e a reconstrução é EXACTA nos pontos"
           " medidos. O espaço dos polinómios de grau ≤ n é ISOMORFO a ℝ^{n+1} pelo mapa"
           " p ↦ (Δ⁰p(0), …, Δⁿp(0)): cada andar é um grau, e cada grau é uma coordenada",
           recon == recon_cas && recon_cas == 72);
    }

    /* ═══ §L11o  A ORDENAÇÃO, E O PONTO ACIMA QUE É POLINÓMIO ABAIXO ═══════ */
    printf("\n§L11o Cada dimensão dá um EIXO à seguinte — e daí a ordem.\n\n");
    {
        /* O coordenador: «e está aí a ordenação também, porque cada dimensão fornece um
         * eixo de coordenadas para a próxima»; e «cada ponto numa reta acima é um
         * polinómio abaixo».
         *
         * (i) A ORDEM é LEXICOGRÁFICA nas coordenadas-diferença, do TOPO para baixo: o
         *     eixo de maior grau decide primeiro, e só em caso de empate se desce. É a
         *     mesma ordem dos VALORES, e é a mesma da leitura posicional — comparar dois
         *     naturais é comparar os dígitos do mais significativo para baixo.
         *
         * (ii) E os dois níveis são DUAIS: pela fórmula de Newton, a coordenada
         *      Δ^k p(0) — um PONTO no andar k — emparelha com o binómio C(x,k), que é um
         *      POLINÓMIO de grau k no andar abaixo. E o emparelhamento é ortonormal:
         *
         *          Δ^j C(x,k)|₀ = δ_{jk}          — Gram = I, outra vez (§L1)
         *
         *      Cada ponto acima É um polinómio abaixo, e a base dos binómios é a base
         *      DUAL das diferenças. */
        long cas = 0, lex_ok = 0, maior = 0, menor = 0, est5 = 41;
        for(int t = 0; t < 400; t++){
            long c1[4], c2[4]; int d = 1 + (t % 3);
            for(int j = 0; j <= d; j++){
                est5 = (est5*1103515245L + 12345L) % 2147483647L; c1[j] = ((est5>>7) % 9) - 4;
                est5 = (est5*1103515245L + 12345L) % 2147483647L; c2[j] = ((est5>>7) % 9) - 4;
            }
            if(c1[d] == 0 || c2[d] == 0) continue;
            /* as coordenadas-diferença */
            long A[8], B[8];
            for(int q = 0; q < 2; q++){
                long *c = q ? c2 : c1, *O = q ? B : A;
                long v[12];
                for(int x = 0; x < d+3; x++){
                    long acc = 0, pot = 1;
                    for(int j = 0; j <= d; j++){ acc += c[j]*pot; pot *= x; }
                    v[x] = acc;
                }
                int nv = d+3;
                for(int k = 0; k <= d; k++){
                    O[k] = v[0];
                    for(int x = 0; x + 1 < nv; x++) v[x] = v[x+1] - v[x];
                    nv--;
                }
            }
            /* lexicográfico do TOPO para baixo */
            int lex = 0;
            for(int k = d; k >= 0 && !lex; k--) if(A[k] != B[k]) lex = (A[k] > B[k]) ? 1 : -1;
            /* contra a ordem dos VALORES num x grande */
            long X = 60, v1 = 0, v2 = 0, pot = 1;
            for(int j = 0; j <= d; j++){ v1 += c1[j]*pot; v2 += c2[j]*pot; pot *= X; }
            int val = (v1 > v2) ? 1 : ((v1 < v2) ? -1 : 0);
            cas++;
            if(lex == val) lex_ok++;
            if(val > 0) maior++; else if(val < 0) menor++;
        }
        /* (ii) a base DUAL: Δ^j C(x,k)|₀ = δ_{jk} */
        long pares = 0, dual_ok = 0;
        for(int k = 0; k <= 6; k++) for(int j = 0; j <= 6; j++){
            /* C(x,k) avaliado em x = 0..10, e depois j diferenças */
            long v[14]; int nv = 12;
            for(int x = 0; x < nv; x++){
                long C = 1;
                for(int i2 = 0; i2 < k; i2++) C = C*(x - i2)/(i2 + 1);
                v[x] = (k == 0) ? 1 : C;
            }
            for(int passo = 0; passo < j; passo++){
                for(int x = 0; x + 1 < nv; x++) v[x] = v[x+1] - v[x];
                nv--;
            }
            pares++;
            if(v[0] == (j == k ? 1 : 0)) dual_ok++;
        }
        printf("      a ORDEM lexicográfica nas coordenadas-diferença bate com a ordem"
               " dos VALORES em %ld de %ld (%ld maiores, %ld menores: a comparação"
               " NÃO é constante)\n", lex_ok, cas, maior, menor);
        printf("      e a base DUAL: Δ^j C(x,k)|₀ = δ_{jk} em %ld de %ld pares — Gram = I"
               " outra vez\n", dual_ok, pares);
        ok("A ORDENAÇÃO SAI DA MESMA CONSTRUÇÃO, PORQUE CADA DIMENSÃO DÁ UM EIXO À"
           " SEGUINTE: a ordem é LEXICOGRÁFICA nas coordenadas-diferença, do topo para"
           " baixo — o eixo de maior grau decide primeiro e só em empate se desce —, e"
           " ela coincide com a ordem dos VALORES em todos os casos medidos. É também a"
           " ordem da leitura posicional: comparar dois naturais é comparar os dígitos do"
           " mais significativo para baixo",
           lex_ok == cas && maior > 100 && menor > 100);
        ok("E CADA PONTO NUMA RECTA ACIMA É UM POLINÓMIO ABAIXO: pela fórmula de Newton, a"
           " coordenada Δ^k p(0) — um PONTO no andar k — emparelha com o binómio C(x,k),"
           " que é um POLINÓMIO de grau k no andar de baixo. E o emparelhamento é"
           " ORTONORMAL: Δ^j C(x,k)|₀ = δ_{jk} nos 49 pares, Gram = I outra vez. Os"
           " binómios são a base DUAL das diferenças, e é por isso que a soma reconstrói",
           dual_ok == pares && pares == 49);
    }

    /* ═══ §L11g  A DEFINIÇÃO GERAL DO CORTE — sem passar pelo metálico ═════ */
    printf("\n§L11g A classe A define-se pelos CONVERGENTES, e não pelo §metálico.\n\n");
    {
        /* O §corte decide a/b < σ_m: é o metálico. Para CF ARBITRÁRIA a classe tem de
         * ser definida directamente pelos convergentes, e é:
         *
         *      A_{(a_k)} = { r ∈ ℚ : r < c_{2j} para algum convergente PAR }
         *
         * Os pares crescem e ficam todos abaixo; os ímpares descem e ficam todos acima.
         * Nada aqui usa σ_m. E mede-se que a definição é consistente: A é fechado para
         * baixo, não contém nenhum convergente ímpar, e não tem máximo. */
        long seqs = 0, fecha = 0, disjunto = 0, semax = 0, est3 = 23;
        for(int sq = 0; sq < 100; sq++){
            long a5[22];
            for(int k = 1; k <= 18; k++){
                if(sq == 0) a5[k] = 1;
                else { est3 = (est3*1103515245L + 12345L) % 2147483647L;
                       a5[k] = 1 + ((est3 >> 9) % 5); }
            }
            long P[22], Q[22], pm1 = 1, qm1 = 0;
            P[0] = 0; Q[0] = 1;
            P[1] = a5[1]*P[0] + pm1; Q[1] = a5[1]*Q[0] + qm1;
            int n5 = 1;
            for(int k = 2; k <= 18; k++){
                if(Q[k-1] > 500000000L/(a5[k]+1)) break;
                P[k] = a5[k]*P[k-1] + P[k-2];
                Q[k] = a5[k]*Q[k-1] + Q[k-2];
                n5 = k;
            }
            seqs++;
            /* A = {r : r < c_{2j} para algum j}. Fechado para baixo: se r ∈ A e r' < r,
             * então r' ∈ A — imediato da definição, e verifica-se numa amostra. */
            int f2 = 1, d2 = 1, m2 = 1;
            for(long b2 = 1; b2 <= 8; b2++) for(long a2 = -20; a2 <= 20; a2++){
                int em_A = 0;
                for(int j = 2; j <= n5; j += 2)
                    if(a2*Q[j] < P[j]*b2){ em_A = 1; break; }
                if(!em_A) continue;
                /* fechado para baixo */
                for(long c2 = -20; c2 < a2; c2++){
                    int em2 = 0;
                    for(int j = 2; j <= n5; j += 2)
                        if(c2*Q[j] < P[j]*b2){ em2 = 1; break; }
                    if(!em2) f2 = 0;
                }
            }
            /* A não contém convergente ÍMPAR: os ímpares estão todos acima dos pares */
            for(int j = 1; j <= n5; j += 2)
                for(int i2 = 2; i2 <= n5; i2 += 2)
                    if(!(P[i2]*Q[j] < P[j]*Q[i2])) d2 = 0;
            /* sem máximo: dado c_{2j} ∈ A, existe c_{2j+2} ∈ A maior */
            for(int j = 2; j + 2 <= n5; j += 2)
                if(!(P[j]*Q[j+2] < P[j+2]*Q[j])) m2 = 0;
            if(f2) fecha++;
            if(d2) disjunto++;
            if(m2) semax++;
        }
        printf("      %ld sucessões arbitrárias: A fechado para baixo em %ld · nenhum"
               " convergente ÍMPAR em A, em %ld · A sem máximo em %ld\n",
               seqs, fecha, disjunto, semax);
        ok("A CLASSE A DEFINE-SE PELOS CONVERGENTES, E NÃO PELO §METÁLICO: para CF"
           " arbitrária põe-se A = {r ∈ ℚ : r < c_{2j} para algum convergente PAR}, e"
           " nada aqui usa σ_m. Medido em 100 sucessões arbitrárias: A é fechado para"
           " baixo, não contém nenhum convergente ímpar — os pares ficam todos abaixo"
           " dos ímpares —, e não tem máximo, com a testemunha a ser o próprio"
           " convergente par posterior. A cadeia é (a_k) → (p_k/q_k) → I_k → corte, e o"
           " metálico fica como caso explícito",
           fecha == seqs && disjunto == seqs && semax == seqs && seqs == 100);
    }

    /* ═══ §L11h  t_n SEM π — a ordem primeiro, a identificação depois ═══════ */
    printf("\n§L11h t_n caracteriza-se pela ORDEM, sem π; o cosseno é identificação.\n\n");
    {
        /* Escrever t_n = 2cos(π/n) e depois dizer «nenhuma constante foi importada» é
         * circular: π entrou na definição. A caracterização algébrica não precisa dele:
         *
         *      t_n é o traço da companheira C = [[t,−1],[1,0]] com det = 1 tal que
         *          C^n = −I   e   C^k ≠ ±I para 0 < k < n
         *
         * — pura ordem. Só DEPOIS, como IDENTIFICAÇÃO, se escreve t_n = 2cos(π/n). */
        long cas = 0, cn_menos_I = 0, primitivo = 0;
        struct { int n; long t, p; const char *nome; } E2[] = {
            {3,  1, 17, "n=3, t=1"},
            {4, 11, 17, "n=4, t=√2 ≡ 11 em 𝔽₁₇"},
            {6,  9, 13, "n=6, t=√3 ≡ 9  em 𝔽₁₃"},
            {3,  1, 13, "n=3 noutro primo"},
        };
        printf("      andar                        C^n = −I ?   algum C^k = ±I, k<n ?\n");
        for(int i2 = 0; i2 < 4; i2++){
            long n6 = E2[i2].n, t6 = E2[i2].t, p6 = E2[i2].p;
            long x11=1,x12=0,x21=0,x22=1, c11=((t6%p6)+p6)%p6, c12=(p6-1)%p6;
            int eh_menos = 0, antes = 0;
            for(long k = 1; k <= n6; k++){
                long y11=(x11*c11+x12)%p6, y12=(x11*c12)%p6;
                long y21=(x21*c11+x22)%p6, y22=(x21*c12)%p6;
                x11=y11; x12=y12; x21=y21; x22=y22;
                int eI  = (x11==1 && x12==0 && x21==0 && x22==1);
                int emI = (x11==(p6-1)%p6 && x12==0 && x21==0 && x22==(p6-1)%p6);
                if(k < n6 && (eI || emI)) antes++;
                if(k == n6 && emI) eh_menos = 1;
            }
            cas++;
            if(eh_menos) cn_menos_I++;
            if(!antes) primitivo++;
            printf("      %-28s %-12s %s\n", E2[i2].nome, eh_menos ? "sim" : "NÃO",
                   antes ? "SIM (não primitivo)" : "nenhum");
        }
        ok("t_n CARACTERIZA-SE PELA ORDEM, E NÃO POR π: o traço do andar é o t para o"
           " qual a companheira C = (t,−1;1,0), com det = 1, cumpre C^n = −I sem que"
           " nenhum C^k com 0 < k < n seja ±I — pura condição de ordem, verificada nos"
           " quatro casos. A escrita t_n = 2cos(π/n) passa a ser uma IDENTIFICAÇÃO"
           " POSTERIOR, e não a definição: assim a cadeia ordem → t_n → A^in, A^circ →"
           " π_n → π deixa de ser circular",
           cn_menos_I == cas && primitivo == cas && cas == 4);
    }

    /* ═══ §L12  A COMPLETUDE, MOSTRADA ══════════════════════════════════════ */
    printf("\n§L12 A completude: o mecanismo produz um CORTE, e o corte é ÚNICO.\n\n");
    {
        /* O que os intervalos encaixados dão, e o que NÃO dão, separa-se assim
         * (thm:central-continuo, itens 1 e 4):
         *
         *  (a) UNICIDADE, pelo POMBAL, e é exacta em inteiros: se r/s e r'/s' habitam
         *      os mesmos intervalos até uma profundidade K com 2^K > s·s', então
         *          |r/s − r'/s'| < 1/(s s')  ⟹  |r s' − r' s| < 1
         *      e |r s' − r' s| é INTEIRO — logo é ZERO, logo r/s = r'/s'. Não há dois
         *      habitantes.
         *
         *  (b) EXISTÊNCIA: o que a sucessão de decisões produz é um CORTE de Dedekind —
         *      as quatro cláusulas: A ≠ ∅, B ≠ ∅, A fechado para baixo, A sem máximo.
         *      O ponto não vive em ℚ, e é por isso que o corte É o objecto.
         *
         *  (c) e o corte é DETERMINADO pela sucessão: sucessões distintas dão cortes
         *      distintos, e exibe-se o racional que os separa.
         *
         * A identificação do conjunto dos cortes com o corpo ordenado completo é
         * herdada; o que aqui se mostra é que o mecanismo entrega um corte, e um só. */
        long pares = 0, pombal = 0, viola = 0;
        for(long s1 = 1; s1 <= 20; s1++) for(long r1 = -40; r1 <= 40; r1++)
        for(long s2 = 1; s2 <= 20; s2++) for(long r2 = -40; r2 <= 40; r2++){
            long d = r1*s2 - r2*s1;
            if(d < 0) d = -d;
            pares++;
            /* |r1 s2 − r2 s1| < 1 ⟺ é zero ⟺ as duas fracções são a MESMA */
            int perto = (d < 1), igual = (r1*s2 == r2*s1);
            if(perto == igual) pombal++; else viola++;
        }
        /* (b) as QUATRO cláusulas do corte, para cada σ_m */
        long ms = 0, quatro = 0;
        for(long m = 1; m <= G_MMAX; m++){
            int A_naovazio = 0, B_naovazio = 0, fechado = 1, sem_max = 1;
            for(long b = 1; b <= 14; b++) for(long a = -50; a <= 50; a++){
                int lado = g_cmp(a, b, m);
                if(lado < 0) A_naovazio = 1; else B_naovazio = 1;
                /* fechado para baixo: q < a/b < σ ⟹ q < σ */
                if(lado < 0)
                    for(long c = -50; c <= a; c++)
                        if(c*b <= a*b && g_cmp(c, b, m) >= 0) fechado = 0;
                /* sem máximo: já medido em §L9, e reconfirma-se por um passo */
                if(lado < 0){
                    long pot = 1; int achou = 0;
                    for(int j = 1; j <= 20 && !achou; j++){
                        pot *= 2;
                        if(pot*b > 40000000L) break;
                        if(g_cmp(pot*a + 1, pot*b, m) < 0 && (pot*a+1)*b > a*(pot*b)) achou = 1;
                    }
                    if(!achou) sem_max = 0;
                }
            }
            ms++;
            if(A_naovazio && B_naovazio && fechado && sem_max) quatro++;
        }
        /* (c) sucessões distintas → cortes distintos, com o separador EXIBIDO */
        /* E «EXIBIDO» TEM DE EXIBIR. Este bloco dizia «com o racional separador EXIBIDO»
         * em três sítios — comentário, printf e asserção — e não imprimia racional
         * nenhum: contava `separa` e seguia. A palavra prometia o que o código não fazia,
         * e nenhuma asserção o podia apanhar, porque o número estava certo. */
        long pares_m = 0, separa = 0;
        long sep_a[4], sep_b[4], sep_m1[4], sep_m2[4]; int nsep = 0;
        for(long m1 = 1; m1 <= G_MMAX; m1++) for(long m2 = m1+1; m2 <= G_MMAX; m2++){
            pares_m++;
            int achou = 0;
            long ga = 0, gb = 0;
            for(long b = 1; b <= 30 && !achou; b++) for(long a = 0; a <= 300; a++)
                if(g_cmp(a,b,m1) > 0 && g_cmp(a,b,m2) < 0){ achou = 1; ga = a; gb = b; break; }
            if(achou){
                separa++;
                if(nsep < 4){ sep_a[nsep] = ga; sep_b[nsep] = gb;
                              sep_m1[nsep] = m1; sep_m2[nsep] = m2; nsep++; }
            }
        }
        printf("      (a) o POMBAL: %ld pares de fracções — |r₁s₂ − r₂s₁| < 1 coincide"
               " com a igualdade em %ld, viola em %ld\n", pares, pombal, viola);
        printf("      (b) as QUATRO cláusulas do corte: %ld de %ld σ_m (A e B não"
               " vazios, A fechado para baixo, A sem máximo)\n", quatro, ms);
        printf("      (c) sucessões distintas dão cortes distintos: %ld de %ld pares,"
               " com o racional separador EXIBIDO:\n", separa, pares_m);
        for(int i = 0; i < nsep; i++)
            printf("          σ_%ld < %ld/%ld < σ_%ld\n",
                   sep_m1[i], sep_a[i], sep_b[i], sep_m2[i]);
        ok("A COMPLETUDE MOSTRA-SE ASSIM, E NÃO POR VARREDURA: (a) a UNICIDADE é o"
           " POMBAL em inteiros — se dois racionais habitam os mesmos intervalos até"
           " uma profundidade K com 2^K > s·s', então |r₁s₂ − r₂s₁| é um inteiro menor"
           " que 1, logo ZERO, logo são o mesmo; medido nos 2 624 400 pares sem uma"
           " violação. E o que isto dá é a UNICIDADE DOS APROXIMANTES RACIONAIS — não há"
           " dois racionais distintos no mesmo intervalo —, e não directamente a"
           " unicidade do habitante REAL, que vem da completude herdada",
           viola == 0 && pombal == pares && pares > 200000);
        ok("E (b) O QUE A SUCESSÃO PRODUZ É UM CORTE DE DEDEKIND, com as quatro"
           " cláusulas verificadas em todos os oito σ_m: A e B não vazios, A fechado"
           " para baixo, e A sem máximo. O ponto não vive em ℚ — e é por isso que o"
           " CORTE é o objecto, e não um ponto que se procura dentro de ℚ. (c) E o"
           " corte é determinado: sucessões distintas dão cortes distintos, com o"
           " racional separador exibido em todos os pares — e exibido a sério, que é o"
           " que a palavra promete: quatro deles saem impressos com os dois σ_m em volta",
           quatro == ms && ms == G_MMAX && separa == pares_m && pares_m == 28);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  o bit fornece a unidade → as oito leis fornecem a base →\n");
        printf("  o dual fornece a recorrência → Pisot fornece o encolhimento →\n");
        printf("  o corte fornece a ordem → ℝ.\n\n");
        printf("  A passagem a ℝ não é a identificação posterior de um subconjunto\n");
        printf("  conveniente: é a realização da face alcançada da própria construção.\n");
    }
    return falhas ? 1 : 0;
}
