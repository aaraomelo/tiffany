/* geometria_real.c — A RETA CONSTRUÍDA, e não pressuposta.
 *
 * O Aarão, no eval: «o geometrico.tex não deve mais ser um paper paralelo explicando a
 * geometria; ele deve ser a realização geométrica da construção já fechada no Universal»,
 * e a espinha é
 *
 *      bit → 8 leis → 𝔽₂⁸ → dual → metálico/Pisot → corte → ℝ
 *
 * com a regra editorial: «nenhuma definição geométrica anterior à construção do corte».
 * E a tese: «não usamos a reta para construir os números; construímos a reta a partir do
 * processo que produz os números.»
 *
 * ── O QUE ESTE MEDIDOR ACRESCENTA, E O QUE JÁ ESTAVA MEDIDO ───────────────────
 * Já medido, e cita-se em vez de se repetir: σ^k = F_{k−1} + F_k·σ, o traço em ℤ pela
 * recorrência de Lucas, N(σ^k) = (−1)^k e «unidade e Pisot não são o mesmo critério»
 * (`metalica.c`); a base ortonormal das oito leis, G = I e b_i = ⟨b,e_i⟩ (`ortonormal.c`);
 * o corpo ordenado completo pelas cinco vias (`ordenado.c`); π exacto por andar
 * (`pi_familia.js`, `arquimedes_area.js`).
 *
 * O que falta, e é o coração do paper novo:
 *
 *   §G0  a GEOMETRIA DISCRETA existe ANTES de qualquer real — a distância no cubo 𝔽₂⁸
 *        é a soma das coordenadas-bit, e toma valores em {0,…,8}: inteiros
 *   §G1  o DUAL PRODUZ A RECORRÊNCIA — três caminhos independentes para σ² = mσ + 1
 *   §G2  PISOT: a aproximação não é escolhida, é PRODUZIDA — dist(σ^k,ℤ) = |σ†|^k, com o
 *        critério em inteiros puros E o caso em que ele FALHA, exibido
 *   §G3  o ENCAIXE: os convergentes alternam e a largura é 1/(F_k F_{k+1}) — exacto
 *   §G4  A ÁREA É A RAZÃO (o Gato): |det| = 1, logo comprimento × largura = 1; e o
 *        CONTROLO não-Pisot, onde a largura CRESCE e o encaixe não fecha
 *   §G5  o CORTE decidido em inteiros: nenhum racional cai em cima, e as duas classes
 *        não têm extremo — Dedekind sem avaliar uma raiz
 *   §G6  e o BIT LÊ O CORTE: a bisseção dá um bit por passo, e oito bits são um byte
 *
 * Nenhum double, nenhum limiar, nenhuma raiz avaliada. As comparações com σ fazem-se
 * elevando ao quadrado, que é a única coisa que uma raiz quadrada permite fazer em ℤ.
 *
 *   cc -O2 -std=c99 -I. -I../lib geometria_real.c -o geometria_real && ./geometria_real
 */
#include <stdio.h>
#include "unidade.h"

/* o tecto MEDE-SE, não se adivinha: com Δ ≤ 68 e F ≤ 1e8, tanto Δ·F² como (t+1)²
 * cabem em long (< 9,2e18). Quem parar por aqui é CONTADO, não silenciado. */
#define G_LIMF 100000000L
#define G_MMAX 8

/* ── a família, em inteiros: F é o Fibonacci metálico, t o Lucas metálico ──────*/
static int g_fib(long m, long *F, long *t, int max){
    int k;
    F[0] = 0; F[1] = 1; t[0] = 2; t[1] = m;
    for(k = 2; k < max; k++){
        if(F[k-1] > G_LIMF/(m+1)) break;          /* o tecto, antes de o passar */
        F[k] = m*F[k-1] + F[k-2];
        t[k] = m*t[k-1] + t[k-2];
    }
    return k;                                      /* quantos andares couberam */
}
/* a/b < σ_m ?  com σ_m = (m + √Δ)/2, Δ = m²+4.  Devolve −1, 0, +1 e nunca 0 para Δ
 * não quadrado — e é isso que se mede. b > 0 sempre. */
static int g_cmp(long a, long b, long m){
    long s = 2*a - m*b, D = m*m + 4;
    if(s < 0) return -1;                           /* a/b abaixo do eixo: já é menor */
    long e = s*s, d = b*b*D;
    return e < d ? -1 : (e > d ? 1 : 0);
}
static int g_peso(int x){ int p = 0; while(x){ p += x & 1; x >>= 1; } return p; }

int main(void){
    printf("\n=== A RETA CONSTRUÍDA: bit → 8 leis → dual → Pisot → corte → ℝ ===\n");

    /* ═══ §G0 A GEOMETRIA DISCRETA, ANTES DE QUALQUER REAL ═══════════════════ */
    printf("\n§G0 A distância existe no cubo das oito leis — e é INTEIRA.\n\n");
    {
        /* ⟨a,b⟩ = paridade(a ∧ b). A distância entre dois bytes é a soma, sobre as oito
         * posições, da diferença das coordenadas: d(a,b) = Σ_i |⟨a,e_i⟩ − ⟨b,e_i⟩|.
         * Mede-se que isso COINCIDE com o peso de a⊕b — dois caminhos —, que toma valores
         * em {0,…,8}, e que o triângulo fecha. Nenhum real entrou. */
        long pares = 0, div_coord = 0, fora = 0, tri = 0, trios = 0;
        int diam = 0;
        for(int a = 0; a < 256; a++) for(int b = 0; b < 256; b++){
            int soma = 0;
            for(int i = 0; i < 8; i++){
                int ai = (a >> i) & 1, bi = (b >> i) & 1;
                soma += ai > bi ? ai - bi : bi - ai;   /* |⟨a,e_i⟩ − ⟨b,e_i⟩| */
            }
            int peso = g_peso(a ^ b);
            pares++;
            if(soma != peso) div_coord++;
            if(peso < 0 || peso > 8) fora++;
            if(peso > diam) diam = peso;
            if(peso == 0 && a != b) fora++;           /* d = 0 só na diagonal */
        }
        for(int a = 0; a < 64; a++) for(int b = 0; b < 64; b++) for(int c = 0; c < 64; c++){
            trios++;
            if(g_peso(a^c) > g_peso(a^b) + g_peso(b^c)) tri++;
        }
        printf("      %ld pares: %ld divergências coordenada×peso, %ld valores fora de"
               " {0..8}, diâmetro %d\n", pares, div_coord, fora, diam);
        printf("      %ld trios: %ld violações do triângulo\n", trios, tri);
        ok("A GEOMETRIA EXISTE ANTES DO REAL: no cubo das oito leis a distância é a SOMA"
           " DAS COORDENADAS-BIT, d(a,b) = Σ|⟨a,e_i⟩−⟨b,e_i⟩|, coincide com o peso de a⊕b"
           " pelos dois caminhos, toma valores em {0,…,8} e fecha o triângulo — tudo em"
           " inteiros, e o diâmetro é 8 porque as leis são oito",
           div_coord == 0 && fora == 0 && tri == 0 && diam == 8 && pares == 65536);
    }

    /* ═══ §G1 O DUAL PRODUZ A RECORRÊNCIA ════════════════════════════════════ */
    printf("\n§G1 A recorrência não se postula: sai do dual, por três caminhos.\n\n");
    {
        /* (A) a matriz companheira A_m = [[m,1],[1,0]]: traço e determinante lidos dela.
         * (B) o dual ν(x) = −1/x dentro de ℤ[σ]: σ² = mσ+1 dá σ⁻¹ = σ−m, logo
         *     σ† = −σ⁻¹ = m−σ, e daí σ+σ† = m e σ·σ† = −1 SEM avaliar raiz.
         * (C) a potência da matriz: A_m^k = [[F_{k+1},F_k],[F_k,F_{k-1}]].
         * Se os três não derem a mesma coisa, a recorrência não vem do dual. */
        long ms = 0, batem = 0, pot = 0, potk = 0, gume = 0; int minand = 99;
        for(long m = 1; m <= 40; m++){
            long trA = m + 0, detA = m*0 - 1*1;             /* (A) da matriz  */
            /* (B) em ℤ[σ], o elemento a+bσ; σ† = m−σ é (m,−1) */
            long sa = 0, sb = 1, da = m, db = -1;           /* σ = (0,1), σ† = (m,−1) */
            long soma_a = sa+da, soma_b = sb+db;            /* σ+σ† */
            /* produto (a+bσ)(c+dσ) = ac+bd + (ad+bc+m·bd)σ, pois σ² = mσ+1 */
            long pa = sa*da + sb*db, pb = sa*db + sb*da + m*sb*db;
            ms++;
            int okB = (soma_b == 0 && soma_a == m) && (pb == 0 && pa == -1);
            if(okB && trA == m && detA == -1) batem++;
            /* (C) a potência da matriz É a recorrência */
            long F[64], t[64];
            int n = g_fib(m, F, t, 40);
            long a11 = 1, a12 = 0, a21 = 0, a22 = 1;        /* identidade */
            int cai = 0;
            for(int k = 1; k < n-1 && k <= 12; k++){
                long b11 = a11*m + a12*1, b12 = a11*1 + a12*0;
                long b21 = a21*m + a22*1, b22 = a21*1 + a22*0;
                a11=b11; a12=b12; a21=b21; a22=b22;
                if(a11 != F[k+1] || a12 != F[k] || a21 != F[k] || a22 != F[k-1]) cai++;
                potk++;
            }
            if(!cai) pot++;
            { int a = (n-2 < 12) ? n-2 : 12; if(a < minand) minand = a; }
            /* o GUME: mexer numa entrada da companheira e o dual deixa de fechar */
            long detG = m*1 - 1*1;                           /* [[m,1],[1,1]] */
            if(detG != -1) gume++;
        }
        printf("      m=1..40: %ld com os três caminhos a concordar; %ld com a potência"
               " da matriz = (F_{k+1},F_k;F_k,F_{k-1}) em %ld andares — e o m mais curto,"
               " o 40, que enche o tecto em cinco degraus, ainda dá %d\n",
               batem, pot, potk, minand);
        printf("      gume: mexida uma entrada da companheira, det ≠ −1 em %ld de %ld\n",
               gume, ms);
        ok("O DUAL PRODUZ A RECORRÊNCIA, e não o contrário: a matriz companheira, o dual"
           " ν(x) = −1/x lido dentro de ℤ[σ], e a potência A^k = (F_{k+1},F_k;F_k,F_{k−1})"
           " dão os MESMOS σ+σ† = m e σσ† = −1 — sem avaliar uma raiz. E mexer uma entrada"
           " da companheira parte o determinante em 40 de 40: a identidade tem conteúdo",
           batem == ms && pot == ms && ms == 40 && gume == ms && minand >= 3);
    }

    /* ═══ §G2 PISOT: A APROXIMAÇÃO É PRODUZIDA, NÃO ESCOLHIDA ════════════════ */
    printf("\n§G2 dist(σ^k, ℤ) = |σ†|^k — e o andar onde isso quebra, exibido.\n\n");
    {
        /* σ^k + (σ†)^k = t_k ∈ ℤ. Logo σ^k dista de t_k exactamente |σ†|^k, e t_k é o
         * inteiro MAIS PRÓXIMO precisamente quando |σ†|^k < 1/2. Em inteiros:
         *
         *      (σ†)^k = (t_k − F_k√Δ)/2,  logo  |σ†|^k < 1/2  ⟺  (t_k−1)² < ΔF_k² < (t_k+1)²
         *
         * e o SEGUNDO caminho, pela identidade t_k² − ΔF_k² = 4(−1)^k, reduz isso a uma
         * desigualdade só em t_k. Os dois têm de concordar. */
        long casos = 0, ident = 0, discorda = 0, vale = 0, falha = 0;
        int fm = 0, fk = 0, minand = 99;
        printf("      m    andares   t_k²−ΔF_k² = 4(−1)^k   |σ†|^k < 1/2 falha em\n");
        for(long m = 1; m <= G_MMAX; m++){
            long F[64], t[64], D = m*m + 4;
            int n = g_fib(m, F, t, 40);
            int prim = -1;
            for(int k = 1; k < n; k++){
                casos++;
                long alvo = (k % 2) ? -4 : 4;
                if(t[k]*t[k] - D*F[k]*F[k] == alvo) ident++;
                /* caminho 1: as duas desigualdades directas */
                int c1 = ((t[k]-1)*(t[k]-1) < D*F[k]*F[k]) && (D*F[k]*F[k] < (t[k]+1)*(t[k]+1));
                /* caminho 2: substituída a identidade, fica só t_k */
                int c2 = (k % 2) ? (t[k] >= 2) : (t[k] >= 3);
                if(c1 != c2) discorda++;
                if(c1){ vale++; } else { falha++; if(prim < 0){ prim = k; fm = (int)m; fk = k; } }
            }
            if(n-1 < minand) minand = n-1;
            printf("      %-4ld %-9d %-22s %s\n", m, n-1, "sim",
                   prim < 0 ? "nunca" : (prim == 1 ? "k=1" : "k>1"));
        }
        printf("      %ld casos: %ld cumprem a identidade, %ld discordâncias entre os dois"
               " caminhos, %ld com |σ†|^k < 1/2 e %ld sem (o m mais curto dá %d andares)\n",
               casos, ident, discorda, vale, falha, minand);
        printf("      e o que falha é EXACTAMENTE (m=%d, k=%d): t=1, ΔF²=5 e (t+1)²=4 —"
               " |φ†| = 0,618 > 1/2, logo o inteiro mais próximo de φ é 2 e não 1\n", fm, fk);
        ok("PISOT — A APROXIMAÇÃO NÃO É ESCOLHIDA, É PRODUZIDA PELA DUALIDADE:"
           " σ^k + (σ†)^k = t_k é INTEIRO, logo dist(σ^k, ℤ) = |σ†|^k, e o critério"
           " (t_k−1)² < ΔF_k² < (t_k+1)² decide-o em inteiros puros, concordando com a"
           " forma reduzida pela identidade t_k² − ΔF_k² = 4(−1)^k nos dois caminhos",
           ident == casos && discorda == 0 && minand >= 9);
        ok("E A HIPÓTESE NÃO É VAZIA: há EXACTAMENTE UM andar em toda a janela onde"
           " |σ†|^k < 1/2 falha — o ouro no primeiro degrau, |φ†| = 0,618 > 1/2 — e aí"
           " t_k já não é o inteiro mais próximo. A lei vale do segundo degrau em diante"
           " para m=1, e desde o primeiro para m ≥ 2: o escopo é parte do enunciado",
           falha == 1 && fm == 1 && fk == 1);
    }

    /* ═══ §G3 O ENCAIXE ══════════════════════════════════════════════════════ */
    printf("\n§G3 Os convergentes alternam, e a largura é 1/(F_k F_{k+1}).\n\n");
    {
        /* c_k = F_{k+1}/F_k. Cassini metálico: F_{k+1}² − F_{k+2}F_k = (−1)^k, logo
         * |c_{k+1} − c_k| = 1/(F_k F_{k+1}) — a largura, exacta e sem divisão. */
        long casos = 0, cass = 0, alterna = 0, encaixa = 0, aperta = 0, encaixa_pod = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long F[64], t[64];
            int n = g_fib(m, F, t, 40);
            for(int k = 1; k < n-2; k++){
                casos++;
                if(F[k+1]*F[k+1] - F[k+2]*F[k] == ((k % 2) ? -1 : 1)) cass++;
                /* alternância: c_k e c_{k+1} caem em lados OPOSTOS de σ */
                int s1 = g_cmp(F[k+1], F[k], m), s2 = g_cmp(F[k+2], F[k+1], m);
                if(s1 != 0 && s2 != 0 && s1 != s2) alterna++;
                /* encaixe: o intervalo seguinte está DENTRO do anterior */
                if(k >= 2){
                    /* [min(c_{k-1},c_k), max] ⊃ [min(c_k,c_{k+1}), max] */
                    long an = F[k], ad = F[k-1], bn = F[k+1], bd = F[k];
                    long cn = F[k+2], cd = F[k+1];
                    long lo_n, lo_d, hi_n, hi_d;
                    if(an*bd < bn*ad){ lo_n=an; lo_d=ad; hi_n=bn; hi_d=bd; }
                    else            { lo_n=bn; lo_d=bd; hi_n=an; hi_d=ad; }
                    int dentro = (lo_n*bd <= bn*lo_d) && (bn*hi_d <= hi_n*bd)
                              && (lo_n*cd <= cn*lo_d) && (cn*hi_d <= hi_n*cd);
                    if(dentro) encaixa++;
                }
                /* e aperta GEOMETRICAMENTE: F_{k+1} ≥ m·F_k, logo a largura cai
                 * por um factor ≥ m² a cada passo — e para m=1 por ≥ φ² > 2 em dois passos */
                if(F[k+2]*F[k+1] > F[k+1]*F[k]) aperta++;
                if(k >= 2) encaixa_pod++;
            }
        }
        printf("      %ld degraus: %ld com Cassini exacto, %ld a alternar de lado,"
               " %ld encaixados, %ld a apertar\n", casos, cass, alterna, encaixa, aperta);
        ok("O ENCAIXE SAI DA RECORRÊNCIA: os convergentes F_{k+1}/F_k caem alternadamente"
           " nos dois lados de σ, cada intervalo está DENTRO do anterior, e a largura é"
           " exactamente 1/(F_k F_{k+1}) por Cassini metálico F_{k+1}² − F_{k+2}F_k ="
           " (−1)^k — a régua é inteira e a divisão nunca se faz",
           cass == casos && alterna == casos && aperta == casos
           && encaixa == encaixa_pod && encaixa_pod > 100);
    }

    /* ═══ §G4 A ÁREA É A RAZÃO — O GATO, E O CONTROLO ════════════════════════ */
    printf("\n§G4 |det| = 1: a área conserva-se, o comprimento diverge, a largura vai a zero.\n\n");
    {
        /* det(A_m) = −1, logo det(A_m^k) = (−1)^k e |det| = 1 SEMPRE. Isso é o Teorema do
         * Gato: a área conserva-se. O rectângulo de lados σ^k (a direcção que estica) e
         * |σ†|^k (a que encolhe) tem área |σσ†|^k = 1. Como o comprimento diverge, a
         * LARGURA TEM DE IR A ZERO — e é isso, e não um axioma, que fecha o corte. */
        long casos = 0, area1 = 0, cresce_t = 0, cresce_F = 0; int plano_m = 0, plano_k = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long F[64], t[64];
            int n = g_fib(m, F, t, 40);
            for(int k = 2; k < n-1; k++){
                casos++;
                long det = F[k+1]*F[k-1] - F[k]*F[k];       /* det(A^k) */
                if(det == ((k % 2) ? -1 : 1)) area1++;
                if(t[k] > t[k-1]) cresce_t++;                 /* o comprimento diverge */
                if(F[k] > F[k-1]) cresce_F++;
                else if(!plano_m){ plano_m = (int)m; plano_k = k; }
            }
        }
        /* O CONTROLO: x² − 2x − 4, com σ = 1+√5 e σ† = 1−√5. |σ†| = 2,236 > 1: NÃO é
         * Pisot, e a norma é −4, logo |det| = 4 ≠ 1. A área NÃO se conserva, e o que
         * devia encolher CRESCE. Se o critério de §G2 passasse aqui, ele não media nada. */
        long u[32], w[32], Dc = 4 + 16;      /* traço 2, norma −4 ⟹ Δ = 4+16 = 20 */
        u[0]=0; u[1]=1; w[0]=2; w[1]=2;
        long ctrl = 0, ctrl_pisot = 0, ctrl_area = 0;
        for(int k = 2; k < 14; k++){ u[k] = 2*u[k-1] + 4*u[k-2]; w[k] = 2*w[k-1] + 4*w[k-2]; }
        for(int k = 2; k < 14; k++){
            ctrl++;
            if((w[k]-1)*(w[k]-1) < Dc*u[k]*u[k] && Dc*u[k]*u[k] < (w[k]+1)*(w[k]+1))
                ctrl_pisot++;                                  /* tem de ser ZERO */
            /* A = [[2,4],[1,0]]: A^k = [[u_{k+1},4u_k],[u_k,4u_{k-1}]], det = 4(u_{k+1}u_{k-1}−u_k²) */
            long det = 4*(u[k+1]*u[k-1] - u[k]*u[k]);
            if(det == 1 || det == -1) ctrl_area++;             /* tem de ser ZERO */
        }
        printf("      metálicos: %ld andares, %ld com |det| = 1, %ld com o traço a crescer,"
               " %ld com F a crescer\n", casos, area1, cresce_t, cresce_F);
        printf("      e o único degrau PLANO é (m=%d, k=%d): F_1 = F_2 = 1 — o ouro demora"
               " um andar a arrancar, e é o mesmo m do |σ†| > 1/2 de §G2\n", plano_m, plano_k);
        printf("      controlo x²−2x−4 (σ† = 1−√5, |σ†| > 1): %ld andares, %ld a cumprir"
               " o critério de Pisot, %ld com |det| = 1 (det = (−4)^k)\n",
               ctrl, ctrl_pisot, ctrl_area);
        ok("A COMPLETUDE É CONSEQUÊNCIA DA CONSERVAÇÃO DA ÁREA — o Teorema do Gato: |det|"
           " = |σσ†| = 1 em todos os andares, logo comprimento × largura = 1; e como o"
           " comprimento σ^k diverge, a largura |σ†|^k TEM de ir a zero. O corte não fecha"
           " por axioma acrescentado: fecha porque a área não pode aumentar nem diminuir",
           area1 == casos && cresce_t == casos && cresce_F == casos - 1
           && plano_m == 1 && plano_k == 2);
        ok("E O CONTROLO MOSTRA QUE É PISOT QUE FAZ O TRABALHO: com x² − 2x − 4, onde"
           " |σ†| = √5 > 1 e |det| = 4, o critério de §G2 falha em TODOS os 12 andares e"
           " |det| = 1 em NENHUM — o que devia encolher cresce, e o encaixe não fecha."
           " Uma recorrência inteira qualquer não dá a reta: tem de ser unidade e Pisot",
           ctrl_pisot == 0 && ctrl_area == 0 && ctrl == 12);
    }

    /* ═══ §G5 O CORTE, DECIDIDO EM INTEIROS ══════════════════════════════════ */
    printf("\n§G5 Cada racional cai de um lado — e nenhum cai em cima.\n\n");
    {
        /* a/b < σ_m ⟺ 2a−mb < 0 ou (2a−mb)² < b²Δ. Total, decidido em ℤ, e a igualdade
         * nunca acontece porque Δ = m²+4 não é quadrado para m ≥ 1 — o que quer dizer,
         * exactamente, que σ ∉ ℚ e que o corte é genuíno. */
        long casos = 0, iguais = 0, baixo = 0, cima = 0, sem_max = 0, sem_min = 0, testes = 0;
        for(long m = 1; m <= G_MMAX; m++)
        for(long b = 1; b <= 20; b++)
        for(long a = -60; a <= 60; a++){
            casos++;
            int s = g_cmp(a, b, m);
            if(s == 0) iguais++;
            else if(s < 0) baixo++; else cima++;
            /* SEM EXTREMO: dado a/b < σ, EXIBE-SE a'/b' com a/b < a'/b' < σ. A primeira
             * versão tentava (2a+1)/(2b) e (3a+1)/(3b) e falhava em 70 dos 19360 — os
             * racionais já demasiado perto de σ, onde um passo de 1/(2b) salta por cima.
             * O passo tem de ENCOLHER até caber no intervalo, e ele existe porque a/b < σ
             * é ESTRITO. Procura-se o primeiro j com a/b + 1/(2^j b) ainda abaixo:
             * a'/b' = (2^j a + 1)/(2^j b), e o tecto de j guarda-se contra o estouro. */
            if(s != 0){
                testes++;
                long pot = 1;
                for(int j = 1; j <= 20; j++){
                    pot *= 2;
                    long cn = pot*a + (s < 0 ? 1 : -1), cd = pot*b;
                    if(cd > 40000000L) break;                  /* o tecto, antes de o passar */
                    int sc = g_cmp(cn, cd, m);
                    if(s < 0){ if(sc < 0 && cn*b > a*cd){ sem_max++; break; } }
                    else      { if(sc > 0 && cn*b < a*cd){ sem_min++; break; } }
                }
            }
        }
        printf("      %ld racionais: %ld abaixo, %ld acima, %ld EM CIMA de σ\n",
               casos, baixo, cima, iguais);
        printf("      dos %ld com lado definido: %ld sem máximo à esquerda, %ld sem mínimo"
               " à direita (soma %ld)\n", testes, sem_max, sem_min, sem_max + sem_min);
        ok("O CORTE É UM CORTE DE DEDEKIND, E DECIDE-SE EM INTEIROS: cada racional cai de"
           " um lado por (2a−mb)² contra b²Δ, NENHUM dos 19 360 cai em cima — porque Δ ="
           " m²+4 não é quadrado, que é dizer σ ∉ ℚ — e nenhuma das duas classes tem"
           " extremo. A raiz nunca é avaliada: elevar ao quadrado é tudo o que é preciso",
           iguais == 0 && sem_max + sem_min == testes && baixo > 0 && cima > 0);
    }

    /* ═══ §G6 O BIT LÊ O CORTE ═══════════════════════════════════════════════ */
    printf("\n§G6 A bisseção dá UM BIT por passo — e oito bits são um byte.\n\n");
    {
        /* O corte de §G5 responde a uma pergunta de sim/não. Bissectar [m, m+1] e
         * perguntar de que lado está σ dá, a cada passo, exactamente UM BIT — e o bit
         * é a coordenada da posição, pela base ortonormal das oito leis. Oito passos
         * enchem um byte, e o intervalo final tem largura 1/256 e contém σ. */
        long ms = 0, contem = 0, larg = 0, bits_ok = 0;
        printf("      m    inteiro   os oito bits    intervalo final ⊃ σ ?\n");
        for(long m = 1; m <= G_MMAX; m++){
            long lo = m, hi = m + 1, den = 1;      /* [lo/den, hi/den], σ dentro */
            int byte = 0, bom = 1;
            if(!(g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0)) bom = 0;
            for(int j = 0; j < 8; j++){
                lo *= 2; hi *= 2; den *= 2;
                long meio = (lo + hi) / 2;
                int b;
                if(g_cmp(meio, den, m) < 0){ b = 1; lo = meio; }   /* σ está na metade de cima */
                else                       { b = 0; hi = meio; }
                byte |= b << (7 - j);
                if(!(g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0)) bom = 0;
                if(hi - lo != 1) bom = 0;                          /* a largura é 1/den */
            }
            ms++;
            if(bom) contem++;
            if(den == 256 && hi - lo == 1) larg++;
            /* o bit lido é a coordenada: ⟨byte, e_i⟩ = bit i, e reconstruir devolve o byte */
            int rec = 0;
            for(int i = 0; i < 8; i++) rec |= (g_peso(byte & (1 << i)) & 1) << i;
            if(rec == byte) bits_ok++;
            printf("      %-4ld %-9ld ", m, m);
            for(int j = 7; j >= 0; j--) printf("%d", (byte >> j) & 1);
            printf("        %s\n", bom ? "sim" : "NÃO");
        }
        printf("      %ld metais: %ld com σ dentro em todos os oito passos, %ld com"
               " largura final 1/256, %ld com a coordenada a devolver o bit\n",
               ms, contem, larg, bits_ok);
        ok("E O BIT LÊ O CORTE: cada bisseção é UMA pergunta de lado — a de §G5, em"
           " inteiros — e devolve exactamente UM BIT; oito passos enchem um byte, o"
           " intervalo fica com largura 1/256 e σ continua dentro nos oito. O bit lê-se"
           " pela coordenada ⟨b,e_i⟩ da base ortonormal, e a reta é o fluxo desses bits",
           contem == ms && larg == ms && bits_ok == ms && ms == G_MMAX);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  Não usámos a reta para construir os números.\n");
        printf("  Construímos a reta a partir do processo que os produz:\n");
        printf("  bit → oito leis → dual → recorrência → Pisot → encaixe → corte → ℝ.\n");
        printf("  E o corte fecha porque a ÁREA se conserva: |det| = |σσ†| = 1.\n");
    }
    return falhas ? 1 : 0;
}
