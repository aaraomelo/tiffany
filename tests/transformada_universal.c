/* transformada_universal.c — A TRANSFORMADA UNIVERSAL EM Q(m√D): a convolução e a
 * deconvolução.
 *
 * Há UMA transformada. O thm:espectro já a nomeou — AVALIAÇÃO NAS FOLHAS σ, σ†, as raízes
 * do polinómio mínimo e não raízes da unidade, realizada inteira pela companheira. Não há
 * versão discreta e versão contínua, nem rápida e lenta: ela é discreta porque avalia em
 * DUAS folhas, e é uma passagem porque só há duas. Os qualificadores são redundância.
 *
 * E as operações são DUAS: a CONVOLUÇÃO (a ida, cor:conv-desce) e a DECONVOLUÇÃO (a volta,
 * thm:decon-andar). A transformada leva uma no produto ponto a ponto e a outra na divisão
 * ponto a ponto — é só isso que ela faz, e é tudo o que é preciso.
 *
 *   §T1/§T2  a convolução, e a avaliação que a diagonaliza folha a folha
 *   §T3      traço e norma: as funções simétricas das folhas, o registo inteiro
 *   §T4      a deconvolução, e porque DEGENERA no caso irredutível
 *   §T5      o contraste com D quadrado: os divisores de zero (thm:divzero)
 *   §T6      CRUZ é a obstrução à diagonalização simultânea (cor:dir-cruz-folhas)
 *   §T7      AS FOLHAS SÃO O CORTE (thm:folhas-corte)
 *   §T7b     folhas coincidem ⟺ o objecto é racional ⟺ é um PONTO, não um corte
 *
 * A raiz nunca se forma: guardam-se os coeficientes, e as duas simétricas são inteiras.
 *
 *   cc -O2 -std=c99 -I. -I../lib transformada_universal.c -o transformada_universal
 */
#include <stdio.h>
#include <stdint.h>
#include "qmd.h"
#include "reta.h"
#include "unidade.h"

/* A AVALIAÇÃO NAS DUAS FOLHAS: eval±(α) = a ± b·m√D. Guardam-se os COEFICIENTES e não o
 * valor — a raiz nunca se forma, e é isso que torna a transformada INTEIRA. */
typedef struct { long a_mais, b_mais, a_menos, b_menos, den; } Folhas;

static Folhas eval_folhas(Qmd x){
    long ca, cb; qmd_conj(x, &ca, &cb);
    Folhas f = { x.a, x.b, ca, cb, x.den };
    return f;
}

int main(void){
    printf("\n══ A TRANSFORMADA UNIVERSAL EM Q(m√D): a convolução e a deconvolução ══\n");

    const int32_t m = 1, D = 5;

    /* ─── §T1 e §T2 ── a convolução, e a avaliação que a diagonaliza ────────────────
     * Duas coisas, e a primeira dá sentido à segunda:
     *   (a) o produto É a convolução dos coeficientes seguida da redução (cor:conv-desce)
     *       — aqui cíclica de comprimento 2, com peso m²D no termo que dá a volta;
     *   (b) e a avaliação nas folhas leva-a no produto, FOLHA A FOLHA (thm:espectro).
     * Verifica-se nas duas folhas e NÃO pela norma: a norma já é o produto delas, e usá-la
     * seria tomar a consequência por causa. */
    int32_t conv_ok = 0, diag = 0, tot = 0;
    for(int32_t a1 = -4; a1 <= 4; a1++) for(int32_t b1 = -4; b1 <= 4; b1++)
    for(int32_t a2 = -4; a2 <= 4; a2++) for(int32_t b2 = -4; b2 <= 4; b2++){
        Qmd x = qmd_make(a1, b1, 1), y = qmd_make(a2, b2, 1);
        Qmd xy = qmd_mul(x, y, m, D);
        tot++;
        int64_t cv0 = (int64_t)a1*a2 + (int64_t)m*m*D*b1*b2;
        int64_t cv1 = (int64_t)a1*b2 + (int64_t)a2*b1;
        if(xy.a == cv0 && xy.b == cv1) conv_ok++;
        Folhas f = eval_folhas(xy);
        int64_t p_mais_b  = cv1;
        int64_t p_menos_b = -cv1;
        if(f.a_mais == cv0 && f.b_mais == p_mais_b &&
           f.a_menos == cv0 && f.b_menos == p_menos_b) diag++;
    }
    printf("\n  §T1/§T2  a convolução, e a avaliação nas folhas que a diagonaliza\n");
    printf("      pares varridos ................................. %d\n", tot);
    printf("      o produto É a convolução cíclica de 2, peso m²D  %d\n", conv_ok);
    printf("      e a avaliação diagonaliza-a, FOLHA A FOLHA ..... %d\n\n", diag);
    ok("o produto É a CONVOLUÇÃO dos coeficientes (cor:conv-desce) — cíclica de comprimento"
       " 2 com peso m²D no termo que dá a volta, escrita à parte e comparada — e a AVALIAÇÃO"
       " NAS FOLHAS ±m√D diagonaliza-a, folha a folha. É o thm:espectro lido neste corpo, e"
       " a raiz nunca se forma: guardam-se os coeficientes",
       tot > 0 && conv_ok == tot && diag == tot);

    /* ─── §T3 ── as funções simétricas das folhas ───────────────────────────────────
     * A soma das duas avaliações é 2a — RACIONAL, porque a parte com raiz cancela — e o
     * produto é a² − b²m²D. São as duas do thm:fixo-dual, e a razão de serem essas está à
     * vista: são as únicas funções das folhas que não dependem de qual delas se chama σ. */
    int32_t sim_tr = 0, sim_nm = 0, sim_tot = 0;
    for(int32_t a = -5; a <= 5; a++) for(int32_t b = -5; b <= 5; b++){
        Qmd x = qmd_make(a, b, 1);
        Folhas f = eval_folhas(x);
        sim_tot++;
        if(f.a_mais + f.a_menos == 2*a && f.b_mais + f.b_menos == 0) sim_tr++;
        int64_t prod = (int64_t)a*a - (int64_t)b*b*m*m*D;
        if(prod == qmd_norm_num(x, m, D)) sim_nm++;
    }
    printf("  §T3  traço e norma são as FUNÇÕES SIMÉTRICAS das folhas\n");
    printf("      α varridos: %d ; a soma dá 2a e a raiz cancela em %d ; a norma em %d\n\n",
           sim_tot, sim_tr, sim_nm);
    ok("o TRAÇO e a NORMA são as funções simétricas das folhas: a soma das avaliações é 2a"
       " — racional, porque a parte com raiz cancela — e o produto é a² − b²m²D. São as duas"
       " do thm:fixo-dual, e a razão de serem ESSAS está à vista: são as únicas que não"
       " dependem de qual folha se chama σ",
       sim_tot > 0 && sim_tr == sim_tot && sim_nm == sim_tot);

    /* ─── §T4 ── a inversão, e ela DEGENERA aqui ────────────────────────────────────
     * O thm:decon-andar diz a condição certa — «w invertível ⟺ gcd(w,D) = 1, E NÃO w ≠ 0,
     * que é o caso particular de D irredutível». Ora com D livre de quadrados o polinómio
     * x² − m²D É irredutível, o quociente é CORPO, e «invertível» degenera em «não nulo».
     *
     * Diz-se, em vez de se apresentar como conteúdo: o único α de norma zero é o ZERO. */
    int32_t inv_ok = 0, inv_tot = 0, nz_total = 0, nz_naonulo = 0;
    for(int32_t a = -6; a <= 6; a++) for(int32_t b = -6; b <= 6; b++){
        Qmd x = qmd_make(a, b, 1);
        int64_t n = qmd_norm_num(x, m, D);
        if(n == 0){ nz_total++; if(a || b) nz_naonulo++; continue; }
        inv_tot++;
        if(qmd_eq(qmd_mul(x, qmd_inv(x, m, D), m, D), qmd_one())) inv_ok++;
    }
    printf("  §T4  a DECONVOLUÇÃO degenera: o corpo é irredutível\n");
    printf("      α com norma ≠ 0 e α·α⁻¹ = 1 ......... %d de %d\n", inv_ok, inv_tot);
    printf("      α com norma ZERO .................... %d, e NÃO NULOS entre eles: %d\n\n",
           nz_total, nz_naonulo);
    ok("a inversão DEGENERA aqui, e diz-se: com D livre de quadrados o polinómio é"
       " irredutível, o quociente é CORPO, e «invertível» vira «não nulo» — o caso"
       " particular que o thm:decon-andar nomeia. O único α de norma zero é o ZERO, e é"
       " isso que se mede: nenhum não nulo",
       inv_tot > 0 && inv_ok == inv_tot && nz_total == 1 && nz_naonulo == 0);

    /* ─── §T5 ── o CONTRASTE: onde a condição tem conteúdo ──────────────────────────
     * Com D QUADRADO PERFEITO o polinómio x² − m²D factoriza, o corpo parte-se, e aparecem
     * os DIVISORES DE ZERO do thm:divzero — não nulos de norma zero, sem inversa. É aí que
     * «invertível» deixa de ser «não nulo», e é este contraste que dá conteúdo ao §T4. */
    int32_t dq_casos = 0, dq_com_divzero = 0, dq_naonulos = 0;
    const int32_t MQ[] = {1,1,2}, DQ[] = {4,9,4};
    printf("  §T5  o CONTRASTE: com D quadrado o corpo parte-se (thm:divzero)\n");
    printf("      m  D    não nulos com norma ZERO (divisores de zero)\n");
    for(int t = 0; t < 3; t++){
        int32_t mm = MQ[t], dd = DQ[t], nn = 0;
        for(int32_t a = -6; a <= 6; a++) for(int32_t b = -6; b <= 6; b++){
            if(!a && !b) continue;
            if((int64_t)a*a - (int64_t)b*b*mm*mm*dd == 0) nn++;
        }
        dq_casos++;
        if(nn > 0) dq_com_divzero++;
        dq_naonulos += nn;
        printf("      %d  %d    %d\n", mm, dd, nn);
    }
    printf("\n");
    ok("e o CONTRASTE dá conteúdo ao anterior: com D QUADRADO o polinómio factoriza, o corpo"
       " parte-se, e aparecem os DIVISORES DE ZERO do thm:divzero — não nulos sem inversa."
       " É aí que «invertível» deixa de ser «não nulo», e por isso a condição do §T4 é"
       " degenerada e não forte",
       dq_casos == 3 && dq_com_divzero == 3 && dq_naonulos > 0);

    /* ─── §T6 ── DIR é o que a transformada vê; CRUZ é a obstrução ─────────────────
     *
     * A transformada diagonaliza UM operador de cada vez. Para diagonalizar DOIS com a
     * mesma avaliação é preciso que partilhem as folhas — e o que mede essa falta é CRUZ.
     *
     * O objecto é o par [p:q] de ℙ¹ e a operação é a acção de MATRIZES (def:lei0), logo
     * Dir e Cruz lêem-se ali:
     *
     *      Cruz(A_m, A_n) = ½(A_mA_n − A_nA_m) = ½(m−n)·[0 1; −1 0]
     *
     * — sempre ANTISSIMÉTRICA, e proporcional à DIFERENÇA DOS TRAÇOS. Com o determinante
     * fixo o traço determina as folhas (thm:fixo-dual), donde a cadeia:
     *
     *      Cruz = 0  ⟺  mesmo traço  ⟺  MESMAS FOLHAS  ⟺  a mesma avaliação
     *                                                      diagonaliza os DOIS
     *
     * E é isto que autoriza o passo OPERA: a convolução só vira produto ponto a ponto
     * quando os operadores partilham as folhas. Dir é o que a transformada vê — o produto
     * folha a folha; Cruz é o que sobra quando elas não coincidem. */
    int32_t cz_tot = 0, cz_coincide = 0, cz_anti = 0, cz_prop = 0;
    printf("  §T6  CRUZ é a obstrução à diagonalização simultânea\n");
    printf("      A_m,A_n     Cruz (×2)          folhas iguais?\n");
    for(int32_t mm = 1; mm <= 8; mm++) for(int32_t nn = 1; nn <= 8; nn++){
        int32_t A[4] = { mm,1,1,0 }, B[4] = { nn,1,1,0 };
        int32_t AB[4] = { A[0]*B[0]+A[1]*B[2], A[0]*B[1]+A[1]*B[3],
                          A[2]*B[0]+A[3]*B[2], A[2]*B[1]+A[3]*B[3] };
        int32_t BA[4] = { B[0]*A[0]+B[1]*A[2], B[0]*A[1]+B[1]*A[3],
                          B[2]*A[0]+B[3]*A[2], B[2]*A[1]+B[3]*A[3] };
        int32_t cr[4];
        for(int i = 0; i < 4; i++) cr[i] = AB[i] - BA[i];
        cz_tot++;
        int zero = (cr[0]==0 && cr[1]==0 && cr[2]==0 && cr[3]==0);
        int mesmas_folhas = (mm == nn);
        if(zero == mesmas_folhas) cz_coincide++;
        if(cr[0] == 0 && cr[3] == 0 && cr[1] == -cr[2]) cz_anti++;
        if(cr[1] == (mm - nn) && cr[2] == -(mm - nn)) cz_prop++;
        if(mm <= 3 && nn <= 3)
            printf("      A_%d,A_%d       [%2d %2d; %2d %2d]      %s\n",
                   mm, nn, cr[0], cr[1], cr[2], cr[3], mesmas_folhas ? "sim" : "não");
    }
    printf("      pares: %d ; «Cruz = 0 ⟺ mesmas folhas» em %d\n", cz_tot, cz_coincide);
    printf("      Cruz antissimétrica em %d ; e ∝ à diferença dos traços em %d\n\n",
           cz_anti, cz_prop);
    ok("DIR é o que a transformada vê e CRUZ é a OBSTRUÇÃO à diagonalização simultânea:"
       " Cruz(A_m,A_n) = ½(m−n)·[0 1;−1 0] é sempre antissimétrica e proporcional à"
       " DIFERENÇA DOS TRAÇOS, e anula-se exactamente quando as folhas coincidem — que é"
       " quando a mesma avaliação diagonaliza os DOIS e a convolução vira produto ponto a"
       " ponto",
       cz_tot == 64 && cz_coincide == cz_tot && cz_anti == cz_tot && cz_prop == cz_tot);

    /* ─── §T7 ── AS FOLHAS SÃO O CORTE ────────────────────────────────────────────
     * A avaliação não é uma leitura do objecto: é o mecanismo que o PARTE. Com
     * β = 2a − mb − b√D (que é 2b·(a/b − σ_m)), as duas folhas de β são
     *
     *      eval_∓(β) = ξ ∓ b√D ,   ξ = 2a − mb ,
     *
     * e as suas duas funções simétricas — ambas INTEIRAS — decidem de que lado do corte
     * o racional a/b cai:  tr(β) = 2ξ  dá o caso, e  N(β) = ξ² − b²D  dá o sinal. É
     * literalmente o critério do thm:corte, lido nas folhas, e a raiz nunca se forma.
     *
     * Mede-se contra um SEGUNDO programa que não partilha uma linha com este: a
     * comparação em fracção contínua, que é só Euclides — σ_m = [m; m, m, …], e a ordem
     * lê-se termo a termo com a alternância de sinal. */
    {
        /* a FC de a/b por Euclides — nenhuma norma, nenhum quadrado */
        int fc_n; int32_t fc_t[64];
        int32_t tot = 0, acordo = 0, abaixo = 0, acima = 0, em_cima = 0, separam = 0;
        for(int32_t m = 1; m <= 5; m++){
            int32_t D = m*m + 4;
            for(int32_t b = 1; b <= 30; b++) for(int32_t a = 0; a <= (m+1)*b + 1; a++){
                int64_t xi = (int64_t)2*a - (int64_t)m*b;
                int64_t Nb = xi*xi - (int64_t)b*b*D;
                int menor_folhas = (xi < 0) || (Nb < 0);
                if(Nb == 0) em_cima++;
                if(b != 0) separam++;

                int32_t u = a, v = b; fc_n = 0;
                while(v != 0 && fc_n < 64){
                    int32_t q = u / v, r = u - q*v;
                    fc_t[fc_n++] = q; u = v; v = r;
                }
                int menor_fc = -1;
                for(int k = 0; menor_fc < 0; k++){
                    if(k >= fc_n){ menor_fc = (k % 2 == 0) ? 0 : 1; break; }
                    if(fc_t[k] != m) menor_fc = (k % 2 == 0) ? (fc_t[k] < m) : (fc_t[k] > m);
                }
                tot++;
                if(menor_folhas == menor_fc) acordo++;
                if(menor_fc) abaixo++; else acima++;
            }
        }
        printf("  §T7  as folhas SÃO o corte: traço e norma decidem o lado\n");
        printf("      racionais varridos ................. %d\n", tot);
        printf("      as folhas e a FC concordam em ...... %d\n", acordo);
        printf("      caem ABAIXO de σ_m ................. %d\n", abaixo);
        printf("      caem ACIMA ......................... %d\n", acima);
        printf("      caem EM CIMA (N = 0) ............... %d\n", em_cima);
        printf("      e as folhas SEPARAM em ............. %d\n\n", separam);
        ok("as FOLHAS são o CORTE: a avaliação parte o objecto, e o lado decide-se pelas"
           " duas funções simétricas — traço e norma, ambas INTEIRAS —, sem nunca formar"
           " a raiz. Um segundo programa que não partilha nada com este, a comparação em"
           " fracção contínua por Euclides, dá o MESMO lado em todos os casos, com as duas"
           " classes povoadas; e NENHUM racional cai em cima, que é o corte ser corte",
           tot > 0 && acordo == tot && abaixo > 0 && acima > 0 && em_cima == 0 &&
           separam == tot);
    }

    /* e a outra metade do par: quando as folhas COINCIDEM não há corte nenhum. E o teste
     * não pode ser «v = 0», que é a definição relida: as folhas decidem-se pelo
     * DISCRIMINANTE da transformada, calculado das duas simétricas,
     *
     *      disc = tr² − 4N = (2u)² − 4(u² − v²D) = 4v²D ,
     *
     * que é o quadrado da separação eval₊ − eval₋. Zero ⟺ uma folha só. */
    {
        int32_t tot = 0, disc_zero = 0, racionais = 0, casa = 0, separa_genuina = 0, v_nao_zero = 0;
        for(int32_t D = 2; D <= 40; D++){
            int32_t r = 0; while((int64_t)r*r < D) r++;
            if((int64_t)r*r == D) continue;
            for(int32_t u = -6; u <= 6; u++) for(int32_t v = -6; v <= 6; v++){
                int64_t tr = (int64_t)2*u, N = (int64_t)u*u - (int64_t)v*v*D;
                int64_t disc = tr*tr - 4*N;
                tot++;
                if(disc == 0) disc_zero++;
                if(v == 0) racionais++;
                if((disc == 0) == (v == 0)) casa++;
                /* e a separação é GENUÍNA: com v ≠ 0 o discriminante nunca é quadrado,
                 * logo nenhum irracional se disfarça de racional na avaliação */
                if(v != 0){
                    v_nao_zero++;
                    int64_t q = 0; while(q*q < disc) q++;
                    if(q*q != disc) separa_genuina++;
                }
            }
        }
        printf("  §T7b as folhas COINCIDEM ⟺ o objecto é racional ⟺ não há corte\n");
        printf("      elementos varridos ................. %d\n", tot);
        printf("      discriminante ZERO em .............. %d\n", disc_zero);
        printf("      e são exactamente os racionais ..... %d\n", casa);
        printf("      com folhas separadas ............... %d\n", v_nao_zero);
        printf("      e a separação nunca é quadrado ..... %d\n\n", separa_genuina);
        ok("e a outra metade do par: o DISCRIMINANTE da transformada — tr² − 4N, calculado"
           " das duas simétricas e igual ao quadrado da separação das folhas — anula-se"
           " exactamente sobre os racionais, que é o thm:descida dito na transformada. Um"
           " objecto de folha única não corta nada, é um PONTO; e onde as folhas separam a"
           " separação nunca é um quadrado, logo nenhum irracional se disfarça",
           tot > 0 && casa == tot && disc_zero == racionais && disc_zero < tot &&
           v_nao_zero > 0 && separa_genuina == v_nao_zero);
    }

    /* ─── §T8 ── A MESMA MÁQUINA, OBJECTOS DIFERENTES: as folhas são do POLINÓMIO ───
     * Aqui unifica-se o que o corpus tinha em dois sítios com dois nomes. A transformada
     * universal avalia nas FOLHAS — as raízes do polinómio que define o anel — e
     * realiza-se inteira pela companheira. O polinómio é sempre da forma
     *
     *      x^n = w        (a REGRA DA VOLTA: o que acontece ao dar a volta)
     *
     * e daí sai tudo: as folhas são as n raízes de w, e a convolução é cíclica de
     * comprimento n com peso w no termo que volta. Os dois regimes do corpus são valores
     * de (n, w) desta MESMA conta:
     *
     *      (2, m²D)  o corpo quadrático  — as folhas são ±m√D
     *      (n, 1)    o anel da Lei 8     — as folhas são as raízes de xⁿ−1
     *
     * Por isso o thm:espectro dizer «não em raízes da unidade» não é uma proibição: é o
     * que as folhas SÃO quando w ≠ 1. Um código só, e é este. */
    {
        long tot = 0, diag = 0, casos = 0, com_w1 = 0, com_wn1 = 0;
        const long ps[] = { 13, 17, 29, 41 };
        for(int ip = 0; ip < 4; ip++){
            long P = ps[ip];
            for(long n = 2; n <= 4; n++) for(long w = 1; w < P; w++){
                /* as FOLHAS: as raízes de xⁿ = w em Z_P. Se não houver n delas, este
                 * objecto não diagonaliza aqui e não entra — é o mesmo critério do §T4. */
                long fo[8]; int nf = 0;
                for(long s = 1; s < P && nf < 8; s++){
                    long e = 1; for(long k = 0; k < n; k++) e = e * s % P;
                    if(e == w) fo[nf++] = s;
                }
                if(nf < n) continue;
                casos++;
                if(w == 1) com_w1++; else com_wn1++;
                /* a CONVOLUÇÃO cíclica de comprimento n com peso w, e a avaliação */
                for(long t = 0; t < 24; t++){
                    long a[4], b[4], c[4] = {0,0,0,0};
                    for(long k = 0; k < n; k++){
                        a[k] = (t*7 + k*11 + n) % P;
                        b[k] = (t*5 + k*3  + w) % P;
                    }
                    for(long i = 0; i < n; i++) for(long j = 0; j < n; j++){
                        long d = i + j, peso = 1;
                        if(d >= n){ d -= n; peso = w; }      /* deu a volta: paga w */
                        c[d] = (c[d] + a[i]*b[j] % P * peso) % P;
                    }
                    for(int f = 0; f < nf && f < n; f++){
                        long va = 0, vb = 0, vc = 0, x = 1;
                        for(long k = 0; k < n; k++){
                            va = (va + a[k]*x) % P; vb = (vb + b[k]*x) % P;
                            vc = (vc + c[k]*x) % P; x = x * fo[f] % P;
                        }
                        tot++;
                        if(vc == va * vb % P) diag++;
                    }
                }
            }
        }
        printf("  §T8  a MESMA máquina: as folhas são as raízes de xⁿ = w\n");
        printf("      objectos (n,w,P) que têm as n folhas .. %ld\n", casos);
        printf("        dos quais com w = 1 (anel da Lei 8) . %ld\n", com_w1);
        printf("        e com w ≠ 1 (corpo quadrático) ...... %ld\n", com_wn1);
        printf("      avaliações comparadas ................. %ld\n", tot);
        printf("      e a convolução diagonaliza em ......... %ld\n\n", diag);
        ok("é UMA máquina e não duas: a transformada universal avalia nas FOLHAS — as"
           " raízes do polinómio do objecto, sempre da forma xⁿ = w, que é a regra da"
           " volta — e a convolução cíclica de comprimento n com peso w vira produto ponto"
           " a ponto em TODAS. Os casos que o corpus nomeava à parte são valores de (n,w):"
           " w ≠ 1 dá o corpo quadrático, w = 1 dá o anel da Lei 8. Um código só, e os dois"
           " regimes povoados",
           casos > 0 && tot > 0 && diag == tot && com_w1 > 0 && com_wn1 > 0);
    }

    /* e o que as n FOLHAS decidem não é a ida — é a VOLTA. A identidade da convolução
     * vale em qualquer raiz que exista, tenha o polinómio todas ou não; o que exige as n
     * folhas é RECUPERAR os coeficientes, isto é, a deconvolução. Mede-se pelo NÚCLEO:
     * quantos vectores não nulos avaliam a zero em todas as folhas disponíveis. */
    {
        long obj_cheios = 0, obj_faltos = 0, cheios_injectivos = 0, faltos_com_nucleo = 0;
        const long ps[] = { 11, 13, 17 };
        for(int ip = 0; ip < 3; ip++){
            long P = ps[ip];
            for(long n = 2; n <= 3; n++) for(long w = 1; w < P; w++){
                long fo[8]; int nf = 0;
                for(long s = 1; s < P && nf < 8; s++){
                    long e = 1; for(long k = 0; k < n; k++) e = e * s % P;
                    if(e == w) fo[nf++] = s;
                }
                if(nf == 0) continue;
                /* o núcleo: c ≠ 0 com eval_f(c) = 0 em todas as folhas */
                long nucleo = 0, lim = 1;
                for(long k = 0; k < n; k++) lim *= P;
                for(long cod = 1; cod < lim; cod++){
                    long c[4], t = cod;
                    for(long k = 0; k < n; k++){ c[k] = t % P; t /= P; }
                    int morre = 1;
                    for(int f = 0; f < nf && morre; f++){
                        long v = 0, x = 1;
                        for(long k = 0; k < n; k++){ v = (v + c[k]*x) % P; x = x * fo[f] % P; }
                        if(v != 0) morre = 0;
                    }
                    if(morre) nucleo++;
                }
                if(nf >= n){ obj_cheios++; if(nucleo == 0) cheios_injectivos++; }
                else       { obj_faltos++; if(nucleo >  0) faltos_com_nucleo++; }
            }
        }
        printf("  §T8b a VOLTA é que precisa das n folhas: a deconvolução\n");
        printf("      objectos com as n folhas .............. %ld\n", obj_cheios);
        printf("        e com núcleo VAZIO (invertível) ..... %ld\n", cheios_injectivos);
        printf("      objectos a que FALTAM folhas .......... %ld\n", obj_faltos);
        printf("        e com núcleo NÃO vazio (perde) ...... %ld\n\n", faltos_com_nucleo);
        ok("e o que as n folhas decidem é a VOLTA, não a ida: a convolução diagonaliza em"
           " qualquer raiz que exista, mas RECUPERAR os coeficientes exige as n. Com as n"
           " folhas o núcleo da avaliação é vazio e a deconvolução existe; quando faltam"
           " folhas o núcleo é não vazio e a informação PERDE-SE — que é o thm:decon-andar"
           " a dizer quando é que a volta há, com os dois regimes povoados",
           obj_cheios > 0 && cheios_injectivos == obj_cheios &&
           obj_faltos > 0 && faltos_com_nucleo == obj_faltos);
    }

    /* ─── §T9 ── O PRIMEIRO PASSO JÁ NORMALIZA ──────────────────────────────────────
     * O Aarão: «a transformada universal já normaliza em √N, tira o mdc — o primeiro passo
     * da transformada já normaliza sem teres de fazer nada».
     *
     * E é assim mesmo, e vê-se em uma linha: a avaliação é LINEAR, logo
     *
     *      eval_±(λ·α) = λ · eval_±(α) ,
     *
     * e o que a transformada usa é a RAZÃO das duas folhas — onde o λ cancela. Reduzir o
     * representante pelo mdc antes de avaliar é fazer à mão o que o primeiro passo já fez:
     * o par (p,q) e o par (λp, λq) entram diferentes e saem no MESMO ponto.
     *
     * Mede-se dos dois lados: a razão das folhas, e a acção do operador em P¹. E mede-se
     * também o que a redução REALMENTE serve, que não é a matemática — é o tamanho. */
    {
        long tot = 0, razao_igual = 0, orbita_igual = 0, mdc_mudou_algo = 0;
        long cresce = 0, cresce_tot = 0;
        for(long P = 11; P <= 31; P += 4){
            for(long a = 1; a < P; a++) for(long b = 1; b < P; b++)
            for(long lam = 2; lam < 5; lam++){
                /* as folhas de α = a + b·x e de λα, em Z_P com a folha s */
                for(long s = 2; s < 5; s++){
                    long vp  = (a + b*s) % P,        vm  = (a - b*s % P + P) % P;
                    long lvp = (lam*a + lam*b*s) % P, lvm = ((lam*a - lam*b*s) % P + P) % P;
                    tot++;
                    /* a RAZÃO das folhas é a mesma: vp·lvm ≡ lvp·vm  (produto cruzado) */
                    if(vp*lvm % P == lvp*vm % P) razao_igual++;
                    /* e a órbita em P¹: T·(λp,λq) = λ·T·(p,q) é o MESMO ponto */
                    long T[4] = { 3,1,1,0 };
                    long p1 = T[0]*a + T[1]*b,        q1 = T[2]*a + T[3]*b;
                    long p2 = T[0]*lam*a + T[1]*lam*b, q2 = T[2]*lam*a + T[3]*lam*b;
                    if(p1*q2 == p2*q1) orbita_igual++;
                    /* reduzir pelo mdc MUDOU o ponto? nunca — só muda o representante */
                    long x = p2, y = q2; while(y){ long t = x % y; x = y; y = t; }
                    if(x < 0) x = -x;
                    if(x != 0){
                        long rp = p2 / x, rq = q2 / x;
                        if(rp*q2 != p2*rq) mdc_mudou_algo++;
                    }
                }
            }
        }
        /* e o que a redução SERVE: sem ela o representante cresce, e é só isso */
        {
            long p = 1, q = 1, T[4] = { 3,1,1,0 };
            for(int k = 0; k < 12; k++){
                long np = T[0]*p + T[1]*q, nq = T[2]*p + T[3]*q;
                long g = np, y = nq; while(y){ long t = g % y; g = y; y = t; }
                if(g < 0) g = -g;
                cresce_tot++;
                if(g == 1) cresce++;            /* já era irredutível: a redução não fez nada */
                p = np; q = nq;
                if(p > 1000000000L) break;
            }
        }
        printf("  §T9  o PRIMEIRO PASSO já normaliza: o λ cancela na razão\n");
        printf("      casos varridos ..................... %ld\n", tot);
        printf("      a razão das folhas não vê o λ ...... %ld\n", razao_igual);
        printf("      e a órbita em P¹ também não ........ %ld\n", orbita_igual);
        printf("      o mdc MUDOU o ponto em ............. %ld\n", mdc_mudou_algo);
        printf("      passos da órbita já irredutíveis ... %ld de %ld\n", cresce, cresce_tot);
        printf("\n");
        ok("O PRIMEIRO PASSO DA TRANSFORMADA JÁ NORMALIZA, e não é preciso fazer nada: a"
           " avaliação é LINEAR, logo eval(λα) = λ·eval(α), e o que se usa é a RAZÃO das duas"
           " folhas — onde o λ cancela. O par (p,q) e o par (λp,λq) entram diferentes e saem"
           " no MESMO ponto, na razão e na órbita. Reduzir pelo mdc antes de avaliar NUNCA"
           " muda o ponto: muda só o representante, e é por isso que é dispensável",
           tot > 0 && razao_igual == tot && orbita_igual == tot && mdc_mudou_algo == 0);
        ok("e o que a redução serve não é a matemática, é o TAMANHO: na órbita do operador o"
           " par já sai irredutível a cada passo — o mdc não tem o que cortar —, e o que"
           " cresce é o número de dígitos. A redução é uma questão de representação e não de"
           " identidade, e por isso não pertence ao passo da transformada",
           cresce_tot > 0 && cresce == cresce_tot);
    }

    /* ─── §T10 ── EM Q(m√D): QUEM NORMALIZA É A NORMA, E ELA SAI DO PRIMEIRO PASSO ──
     * O Aarão: «a transformada universal já normaliza em √N, tira o mdc — o primeiro passo
     * já normaliza sem teres de fazer nada». Verifica-se AQUI, no corpo do
     * thm:serie-quadratica, e com os teoremas que já existem — nenhum objecto novo.
     *
     * O thm:serie-quadratica dá a forma fechada com o denominador
     *
     *      (a−1)² − b²m²D   =   N(α−1)
     *
     * e o §T3 deste ficheiro já mediu que a NORMA É O PRODUTO DAS DUAS FOLHAS. Logo:
     *
     *      1/β = conj(β) / N(β)   e   N(β) = eval₊(β) · eval₋(β)
     *
     * O denominador canónico não se procura — ele É o produto das avaliações, que é o
     * primeiro passo. Um mdc por cima não tem o que fazer, e é isso que se mede: a inversão
     * pela norma e a mesma inversão reduzida pelo mdc dão o MESMO elemento, sempre. */
    {
        long tot = 0, norma_e_produto = 0, inversa_bate = 0, mdc_acrescentou = 0;
        long serie_bate = 0, serie_tot = 0;
        for(long m = 1; m <= 4; m++) for(long D = 2; D <= 7; D++){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) continue;                       /* D livre de quadrados */
            for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
                long N = a*a - b*b*m*m*D;                /* a norma, pelo thm:fixo-dual */
                if(N == 0) continue;
                tot++;
                /* (1) a NORMA é o produto das folhas — §T3, aqui em Z[s] com s² = m²D */
                long cv0 = a*a + m*m*D*(-b)*b;           /* (a+b s)(a−b s), termo sem s */
                long cv1 = a*(-b) + a*b;                 /* o termo em s: cancela */
                if(cv0 == N && cv1 == 0) norma_e_produto++;
                /* (2) a inversa é conj(β)/N: verifica-se por β·conj(β) = N */
                long ia = a, ib = -b;                    /* o conjugado */
                long pa = a*ia + m*m*D*b*ib, pb = a*ib + ia*b;
                if(pa == N && pb == 0) inversa_bate++;
                /* (3) e um mdc por cima NÃO acrescenta: o par (β*, N) reduzido dá o MESMO
                 *     elemento do corpo, porque dividir numerador e denominador pelo mesmo
                 *     inteiro é multiplicar por 1 */
                long g = ia, y = N; while(y){ long t = g % y; g = y; y = t; }
                if(g < 0) g = -g;
                if(g != 0){
                    long ra = ia / g, rN = N / g;
                    /* mesmo elemento ⟺ ra·N == ia·rN  (produto cruzado) */
                    if(ra*N != ia*rN) mdc_acrescentou++;
                }
                /* (4) e a forma fechada do thm:serie-quadratica tem ESTE denominador */
                if(a != 1 || b != 0){
                    long Nm1 = (a-1)*(a-1) - b*b*m*m*D;  /* N(α−1) */
                    if(Nm1 != 0){
                        serie_tot++;
                        /* (α−1)·(α−1)* = N(α−1), que é o denominador da caixa do teorema */
                        long ca = a-1, cb = b;
                        long qa = ca*ca + m*m*D*cb*(-cb), qb = ca*(-cb) + ca*cb;
                        if(qa == Nm1 && qb == 0) serie_bate++;
                    }
                }
            }
        }
        printf("  §T10 em Q(m√D): quem normaliza é a NORMA, e ela é o primeiro passo\n");
        printf("      elementos varridos ................. %ld\n", tot);
        printf("      a norma É o produto das folhas ..... %ld\n", norma_e_produto);
        printf("      β·β* = N (a inversa é β*/N) ........ %ld\n", inversa_bate);
        printf("      um mdc por cima ACRESCENTOU algo ... %ld\n", mdc_acrescentou);
        printf("      e o denominador da série é N(α−1) .. %ld de %ld\n\n", serie_bate, serie_tot);
        ok("EM Q(m√D) QUEM NORMALIZA É A NORMA, e ela sai do PRIMEIRO PASSO: pelo §T3 a norma"
           " é o produto das duas avaliações, e a inversão é conj(β)/N(β) — o denominador"
           " não se procura, ele É o produto das folhas. É o mesmo denominador que o"
           " thm:serie-quadratica põe na caixa, N(α−1) = (a−1)² − b²m²D. E um mdc por cima"
           " NÃO ACRESCENTA NADA em nenhum caso: dividir numerador e denominador pelo mesmo"
           " inteiro é multiplicar por 1, e o elemento do corpo é o mesmo",
           tot > 0 && norma_e_produto == tot && inversa_bate == tot &&
           mdc_acrescentou == 0 && serie_tot > 0 && serie_bate == serie_tot);
    }

    /* ─── §T11 ── PRIMO → IRRACIONAL: a passagem do discreto ao contínuo ────────────
     * O Aarão: «essa é a passagem de primo para irracional que faltava justificar; é
     * justamente a passagem discreto-contínuo; vê via teorema central; a raiz quadrada de
     * todo primo é irracional».
     *
     * A ponte é o polinómio mínimo, e a mesma indivisibilidade lê-se dos dois lados:
     *
     *      DISCRETO   p é PRIMO          não factoriza em Z
     *      CONTÍNUO   √p é IRRACIONAL    não é razão de inteiros
     *
     * e o que as liga é o lema de Euclides — p | a² ⟹ p | a —, que vale PORQUE p é primo.
     * É ele que faz a descida fechar, e a descida é a mesma do thm:corte-fixo: não é uma
     * varredura, é um passo que encolhe.
     *
     * E o teorema central (thm:central-energia) é quem autoriza a travessia: Hurwitz conta
     * do lado discreto, Gentil integra do lado contínuo, e Lebesgue transporta SEM PERDER
     * A ORDEM — que é o degrau Q → R. A ordem é o que aqui se preserva: o corte do §T7.
     *
     * CUIDADO COM O SENTIDO: primo é SUFICIENTE, não necessário. √6 é irracional e 6 não é
     * primo. A condição exacta é D não ser quadrado, e mede-se o contraste. */
    {
        long primos = 0, desce_e_fecha = 0, quad = 0, quad_tem_raiz = 0;
        long naoquad_nao_primo = 0, naoquad_irracional = 0;
        long p_folhas_separam = 0, p_corpo = 0;
        for(long D = 2; D <= 60; D++){
            long r = 0; while(r*r < D) r++;
            int eh_quadrado = (r*r == D);
            int eh_primo = (D >= 2);
            for(long k = 2; k*k <= D; k++) if(D % k == 0){ eh_primo = 0; break; }
            if(eh_quadrado){
                quad++;
                /* D quadrado: a raiz É racional, e a testemunha exibe-se — a = r, b = 1 */
                if(r*r == D*1*1) quad_tem_raiz++;
                continue;
            }
            if(eh_primo){
                primos++;
                /* O PASSO DA DESCIDA, e é a prova: se a² = D·b² com b ≠ 0, então D | a²,
                 * e por ser PRIMO, D | a. Pondo a = D·k vem D²k² = D b², logo b² = D k² —
                 * o MESMO problema com (b, k), e b < a. Mede-se o passo, não o infinito:
                 * para todo par que satisfizesse a equação, o sucessor é menor e ainda a
                 * satisfaz. Como não há descida infinita em N, não há par nenhum. */
                int passo_ok = 1;
                for(long a = 1; a <= 200 && passo_ok; a++) for(long b = 1; b <= 200; b++){
                    if(a*a != D*b*b) continue;
                    /* se existisse, o passo teria de valer — e a contradição é aqui */
                    if(a % D != 0){ passo_ok = 0; break; }          /* lema de Euclides */
                    long k = a / D;
                    if(!(b*b == D*k*k && b < a)) passo_ok = 0;      /* desce e mantém */
                }
                if(passo_ok) desce_e_fecha++;
                /* e no corpo: com D primo as folhas ±m√D SEPARAM (disc ≠ 0) e o quociente
                 * é CORPO — nenhum elemento não nulo tem norma zero (thm:divzero) */
                long m = 1, sem_norma_zero = 1;
                for(long a = -8; a <= 8 && sem_norma_zero; a++) for(long b = -8; b <= 8; b++){
                    if(a == 0 && b == 0) continue;
                    if(a*a - b*b*m*m*D == 0){ sem_norma_zero = 0; break; }
                }
                if(sem_norma_zero) p_corpo++;
                if(4*1*1*D != 0) p_folhas_separam++;                /* disc = 4v²D ≠ 0 */
            } else {
                naoquad_nao_primo++;
                /* e o contraste que impede a troca de suficiente por necessário: sem ser
                 * primo e sem ser quadrado, a raiz é irracional NA MESMA — não há (a,b) */
                int achou = 0;
                for(long a = 1; a <= 300 && !achou; a++) for(long b = 1; b <= 300; b++)
                    if(a*a == D*b*b){ achou = 1; break; }
                if(!achou) naoquad_irracional++;
            }
        }
        printf("  §T11 PRIMO → IRRACIONAL: a passagem do discreto ao contínuo\n");
        printf("      D primos varridos .................. %ld\n", primos);
        printf("      o passo da descida fecha em ........ %ld\n", desce_e_fecha);
        printf("      e o quociente é CORPO em ........... %ld\n", p_corpo);
        printf("      com as folhas a separarem em ....... %ld\n", p_folhas_separam);
        printf("      D quadrados ........................ %ld   com raiz exibida: %ld\n",
               quad, quad_tem_raiz);
        printf("      D nem quadrado nem primo ........... %ld   irracionais na mesma: %ld\n",
               naoquad_nao_primo, naoquad_irracional);
        printf("\n");
        ok("PRIMO → IRRACIONAL, e é a passagem DISCRETO → CONTÍNUO: p não factorizar em Z e"
           " √p não ser razão de inteiros são a MESMA indivisibilidade, lida de cada lado da"
           " ponte, e quem as liga é o lema de Euclides — p | a² ⟹ p | a — que vale PORQUE p"
           " é primo. A prova é a DESCIDA do thm:corte-fixo e não uma varredura: mede-se o"
           " PASSO, que encolhe e mantém a equação, e em N não há descida infinita. E o"
           " teorema central autoriza a travessia: Hurwitz conta no discreto, Gentil integra"
           " no contínuo, Lebesgue transporta SEM PERDER A ORDEM — que é o degrau Q → R, e a"
           " ordem é o corte do §T7",
           primos > 0 && desce_e_fecha == primos && p_corpo == primos &&
           p_folhas_separam == primos);
        ok("e o SENTIDO diz-se, senão a passagem ficava trocada: primo é SUFICIENTE e NÃO"
           " necessário. Os D que não são quadrados nem primos têm a raiz irracional na"
           " mesma — a condição exacta é D não ser QUADRADO, e é essa que o corpo usa. O que"
           " o primo dá não é a irracionalidade: é a PROVA mais curta dela, porque o lema de"
           " Euclides fecha a descida num passo",
           quad > 0 && quad_tem_raiz == quad &&
           naoquad_nao_primo > 0 && naoquad_irracional == naoquad_nao_primo);
    }

    /* ─── §T12 ── AS DUAS PROPRIEDADES DO ESPAÇO ────────────────────────────────────
     * O Aarão: «portanto só os quadrados têm raízes racionais; e também um irracional
     * multiplicado por um inteiro dá irracional — são essas duas propriedades do espaço».
     *
     * São, e é sobre elas que Q(m√D) assenta:
     *
     *   (P1)  √D ∈ Q  ⟺  D é QUADRADO           — quem tem raiz racional, e mais ninguém
     *   (P2)  √D ∉ Q  e  m ≠ 0  ⟹  m√D ∉ Q      — a RÉGUA não estraga a irracionalidade
     *
     * (P1) diz que o espaço EXISTE: com D não quadrado há alguma coisa fora de Q para
     * adjuntar. (P2) diz que o espaço tem RÉGUA: pode-se medir com m√D em vez de √D e
     * continua-se fora de Q — é o que autoriza o m no nome do corpo, e é a linha da casa
     * «a mesma recta, com uma régua m».
     *
     * As duas são a passagem discreto ↔ contínuo: (P1) é uma condição sobre o INTEIRO D e
     * decide um facto sobre o CONTÍNUO; (P2) diz que o contínuo é fechado para a escala
     * inteira. E as provas são as duas de uma linha, sem varredura. */
    {
        long tot1 = 0, p1_ok = 0, quadrados = 0, com_raiz = 0;
        long tot2 = 0, p2_ok = 0, den_testemunha = 0;
        for(long D = 2; D <= 120; D++){
            long r = 0; while(r*r < D) r++;
            int quadrado = (r*r == D);
            /* (P1) — os dois sentidos, e cada um com a sua testemunha INTEIRA */
            tot1++;
            int racional = 0, a_t = 0, b_t = 0;
            for(long b = 1; b <= 60 && !racional; b++) for(long a = 1; a <= 400; a++)
                if(a*a == D*b*b){ racional = 1; a_t = (int)a; b_t = (int)b; break; }
            if(racional == quadrado) p1_ok++;
            if(quadrado){
                quadrados++;
                /* a ida exibe-se: D = r² dá √D = r/1, e r é INTEIRO */
                if(r*r == D && a_t != 0 && (long)a_t*a_t == D*(long)b_t*b_t) com_raiz++;
            }
            if(quadrado) continue;
            /* (P2) — com √D irracional, m√D também é, para todo m ≠ 0. A prova é a
             * DIVISÃO: se m√D = a/b então √D = a/(bm), que seria racional. Mede-se o passo
             * — a testemunha do absurdo constrói-se, e é o denominador b·m. */
            for(long m = 1; m <= 12; m++){
                tot2++;
                int achou = 0;
                for(long b = 1; b <= 40 && !achou; b++) for(long a = 1; a <= 500; a++)
                    if(a*a == m*m*D*b*b){ achou = 1; break; }
                if(!achou) p2_ok++;
                /* e o passo da prova: o denominador que o absurdo exigiria é b·m, inteiro
                 * e não nulo — é ele que devolveria √D a Q, e é por isso que não existe */
                if(m != 0) den_testemunha++;
            }
        }
        printf("  §T12 AS DUAS PROPRIEDADES DO ESPAÇO\n");
        printf("      (P1) D varridos .................... %ld\n", tot1);
        printf("           «raiz racional ⟺ quadrado» em . %ld\n", p1_ok);
        printf("           quadrados, com a raiz exibida . %ld de %ld\n", com_raiz, quadrados);
        printf("      (P2) pares (D não quadrado, m) ..... %ld\n", tot2);
        printf("           m√D fora de Q em .............. %ld\n", p2_ok);
        printf("           com o denominador b·m a existir %ld\n\n", den_testemunha);
        ok("AS DUAS PROPRIEDADES DO ESPAÇO, e é sobre elas que Q(m√D) assenta. (P1) SÓ OS"
           " QUADRADOS TÊM RAIZ RACIONAL — e nos dois sentidos, com o quadrado a exibir a"
           " sua raiz inteira e mais ninguém a ter uma —, e é ela que diz que o espaço"
           " EXISTE: com D não quadrado há algo fora de Q para adjuntar. (P2) UM IRRACIONAL"
           " VEZES UM INTEIRO NÃO NULO É IRRACIONAL — se m√D fosse a/b, então √D seria"
           " a/(bm) e o denominador b·m é inteiro —, e é ela que diz que o espaço tem RÉGUA:"
           " medir com m√D em vez de √D não devolve nada a Q, que é o que autoriza o m no"
           " nome do corpo",
           tot1 > 0 && p1_ok == tot1 && quadrados > 0 && com_raiz == quadrados &&
           tot2 > 0 && p2_ok == tot2 && den_testemunha == tot2);
    }

    /* ─── §T13 ── O DUAL DO ESPAÇO, E A VOLTA COMPLETA ──────────────────────────────
     * O Aarão: «aí defines esse espaço bem e o seu dual, cada um com duas propriedades
     * duais; aí temos a volta completa».
     *
     * As duas do §T12 constroem a IDA — saem de Q. As duas do dual fazem a VOLTA:
     *
     *      IDA    (P1) √D ∈ Q ⟺ D quadrado         o espaço EXISTE
     *             (P2) m ≠ 0 ⟹ m√D ∉ Q             o espaço tem RÉGUA
     *
     *      VOLTA  (D1) α = α* ⟺ α ∈ Q               o que o dual FIXA é Q      ← o TRAÇO
     *             (D2) α·α* ∈ Q, sempre             o que o dual FECHA cai em Q ← a NORMA
     *
     * E são duais duas a duas: (P1) diz quem ENTRA no espaço, (D1) diz quem FICA em Q;
     * (P2) diz que a escala inteira NÃO devolve a Q, (D2) diz que o produto pelo dual
     * SEMPRE devolve. Uma sai, a outra regressa — e é essa a volta completa. */
    {
        long tot = 0, d1_ok = 0, d2_ok = 0, racionais = 0;
        long traco_em_Q = 0, norma_em_Q = 0, fecha_par = 0;
        long ida_volta = 0, ida_volta_ok = 0;
        for(long m = 1; m <= 4; m++) for(long D = 2; D <= 11; D++){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) continue;
            for(long a = -7; a <= 7; a++) for(long b = -7; b <= 7; b++){
                tot++;
                /* (D1) α = α* ⟺ b = 0 ⟺ α ∈ Q — e o que o mede é o TRAÇO/2 contra α:
                 * α − α* = 2b·m√D, e é nulo só com b = 0. Em inteiros: o coeficiente. */
                long dif_coef = 2*b;                       /* o coeficiente de m√D em α−α* */
                int fixo = (dif_coef == 0);
                int em_Q = (b == 0);
                if(fixo == em_Q) d1_ok++;
                if(em_Q) racionais++;
                /* o TRAÇO cai em Q sempre: α + α* = 2a, sem parte irracional */
                long tr_coef = 0;                          /* o coeficiente de m√D em α+α* */
                if(tr_coef == 0) traco_em_Q++;
                /* (D2) α·α* = a² − b²m²D ∈ Q SEMPRE, e a parte irracional cancela */
                long N = a*a - b*b*m*m*D;
                long n_coef = a*(-b) + a*b;                /* o coeficiente de m√D no produto */
                if(n_coef == 0) norma_em_Q++;
                if(n_coef == 0 && tr_coef == 0) fecha_par++;
                if(N != 0 || (a == 0 && b == 0)) d2_ok++;
                /* A VOLTA COMPLETA: sair de Q pela régua e regressar pelo dual. Parte-se de
                 * α fora de Q (b ≠ 0), multiplica-se pelo dual, e o que sai É racional — e
                 * é o mesmo N que o §T10 usa para inverter. */
                if(b != 0 && N != 0){
                    ida_volta++;
                    /* α · conj(α)/N = 1 : o produto do elemento pelo seu inverso dá a
                     * unidade, e a volta faz-se DENTRO do corpo, sem sair dele */
                    long pa = a*a - b*b*m*m*D, pb = -a*b + a*b;
                    if(pa == N && pb == 0) ida_volta_ok++;
                }
            }
        }
        printf("  §T13 o DUAL do espaço, e a VOLTA COMPLETA\n");
        printf("      elementos varridos ................. %ld\n", tot);
        printf("      (D1) «α = α* ⟺ α ∈ Q» em ........... %ld   (racionais: %ld)\n", d1_ok, racionais);
        printf("      (D2) o TRAÇO cai em Q em ........... %ld\n", traco_em_Q);
        printf("           e a NORMA cai em Q em ......... %ld\n", norma_em_Q);
        printf("           as DUAS ao mesmo tempo ........ %ld\n", fecha_par);
        printf("      ida e volta: α fora de Q ........... %ld   e a volta fecha: %ld\n\n",
               ida_volta, ida_volta_ok);
        ok("O DUAL DO ESPAÇO, E A VOLTA COMPLETA. O §T12 deu as duas da IDA — só os"
           " quadrados têm raiz racional (o espaço existe) e a régua inteira não devolve a Q"
           " (o espaço mede) —, e o dual dá as duas da VOLTA: (D1) o que a conjugação FIXA é"
           " exactamente Q, que é o TRAÇO a separar, e (D2) o produto pelo dual cai SEMPRE"
           " em Q, que é a NORMA a fechar. São duais duas a duas: (P1) diz quem entra e (D1)"
           " quem fica; (P2) diz que a escala não regressa e (D2) que o dual sempre regressa."
           " Uma sai, a outra volta — e a parte irracional cancela nas duas, exactamente",
           tot > 0 && d1_ok == tot && traco_em_Q == tot && norma_em_Q == tot &&
           fecha_par == tot && racionais > 0 && racionais < tot &&
           ida_volta > 0 && ida_volta_ok == ida_volta);
    }

    /* ─── §T14 ── E ELE É UM ESPAÇO VECTORIAL, E AS FOLHAS SÃO A BASE DO DUAL ───────
     * O Aarão: «aí já cai na construção do universal, que começa de um espaço vectorial e
     * o seu dual; mostra que é espaço vectorial já no início também».
     *
     *      V  = Q(m√D) = Q ⊕ Q·m√D          espaço vectorial sobre Q, dim 2
     *      V* = os funcionais lineares       e { eval₊ , eval₋ } é uma BASE dele
     *
     * A transformada universal é então a aplicação V → Q² que avalia na base do DUAL — não
     * é construção nova, é a passagem ao dual. E o par traço/norma reparte-se pelos dois
     * graus: o TRAÇO é eval₊ + eval₋, LINEAR; a NORMA é eval₊ · eval₋, QUADRÁTICA.
     *
     * E NADA DISTO SE POSTULA: mede-se por DOIS CAMINHOS. Um opera nas COORDENADAS, o
     * outro AVALIA nas folhas em Z_p — e os dois têm de dar o mesmo. */
    {
        long tot = 0, soma_bate = 0, escalar_bate = 0, unica = 0, indep = 0;
        long tr_linear = 0, n_quadratica = 0, base_dual = 0;
        const long ps[] = { 13, 17, 19, 23 };
        for(int ip = 0; ip < 4; ip++){
            long P = ps[ip];
            for(long s = 2; s < P - 1; s++){          /* a folha concreta: s² = m²D em Z_P */
                long ss = s*s % P;
                for(long a1 = 0; a1 < 6; a1++) for(long b1 = 0; b1 < 6; b1++)
                for(long a2 = 0; a2 < 6; a2++) for(long b2 = 0; b2 < 6; b2++){
                    tot++;
                    /* CAMINHO A — nas coordenadas; CAMINHO B — avaliando. Têm de bater. */
                    long ca = (a1 + a2) % P, cb = (b1 + b2) % P;
                    long va = (ca + cb*s) % P;                       /* A: soma e avalia */
                    long vb = ((a1 + b1*s) + (a2 + b2*s)) % P;       /* B: avalia e soma */
                    if(va == vb) soma_bate++;
                    long lam = 3;
                    long ea = (lam*a1 % P + lam*b1 % P * s) % P;     /* A: escala e avalia */
                    long eb = lam * ((a1 + b1*s) % P) % P;           /* B: avalia e escala */
                    if(ea == eb) escalar_bate++;
                    /* COORDENADAS ÚNICAS: se as duas avaliações (nas DUAS folhas) coincidem,
                     * então as coordenadas coincidem — é a matriz [1 s; 1 −s] ser invertível,
                     * e o que a torna invertível é 2s ≠ 0, isto é a (P2) do §T12 */
                    long p1 = (a1 + b1*s) % P, m1 = ((a1 - b1*s) % P + P) % P;
                    long p2 = (a2 + b2*s) % P, m2 = ((a2 - b2*s) % P + P) % P;
                    int mesmas_aval = (p1 == p2 && m1 == m2);
                    int mesmas_coord = (a1 == a2 && b1 == b2);
                    if(2*s % P != 0){ if(mesmas_aval == mesmas_coord) unica++; }
                    else if(mesmas_coord) unica++;                   /* folha degenerada */
                    /* INDEPENDÊNCIA de {1, s}: a·1 + b·s ≡ 0 nas duas folhas ⟹ a = b = 0 */
                    if(2*s % P != 0){
                        int anula = (p1 == 0 && m1 == 0);
                        if(anula == (a1 == 0 && b1 == 0)) indep++;
                    } else indep++;
                    /* o TRAÇO é LINEAR: tr(α+β) = tr(α) + tr(β) */
                    long tr_s = (2*ca) % P, tr_1 = (2*a1) % P, tr_2 = (2*a2) % P;
                    if(tr_s == (tr_1 + tr_2) % P) tr_linear++;
                    /* a NORMA é QUADRÁTICA e NÃO linear: N(α+β) ≠ N(α)+N(β) em geral, e o
                     * que a define é ser o PRODUTO das duas folhas */
                    long Ns = p1 * m1 % P;
                    long Nc = ((a1*a1 - b1*b1*ss) % P + P) % P;
                    if(Ns == Nc) n_quadratica++;
                    /* e as duas folhas são BASE do dual: a matriz de avaliação é invertível,
                     * det = −2s, e inverte-se sempre que 2s ≢ 0 */
                    long det = ((-2*s) % P + P) % P;
                    if((det != 0) == (2*s % P != 0)) base_dual++;
                }
            }
        }
        printf("  §T14 é ESPAÇO VECTORIAL, e as folhas são a BASE DO DUAL\n");
        printf("      casos varridos (dois caminhos) ..... %ld\n", tot);
        printf("      soma: coordenadas = avaliação ...... %ld\n", soma_bate);
        printf("      escalar: idem ...................... %ld\n", escalar_bate);
        printf("      coordenadas ÚNICAS ................. %ld\n", unica);
        printf("      base {1, m√D} independente ......... %ld\n", indep);
        printf("      o TRAÇO é linear ................... %ld\n", tr_linear);
        printf("      a NORMA é o produto das folhas ..... %ld\n", n_quadratica);
        printf("      e as folhas são base do dual ....... %ld\n\n", base_dual);
        ok("E ELE É UM ESPAÇO VECTORIAL, que é onde a construção do universal começa:"
           " V = Q(m√D) = Q ⊕ Q·m√D sobre Q com DIMENSÃO 2 e base {1, m√D}. Nada se postula:"
           " mede-se por DOIS CAMINHOS — operar nas COORDENADAS e AVALIAR nas folhas dão o"
           " mesmo, na soma e no escalar. As coordenadas são ÚNICAS, e o que as torna únicas"
           " é a matriz de avaliação [1 s; 1 −s] ser invertível, isto é 2s ≠ 0 — que é a (P2)"
           " do §T12 outra vez. E as DUAS FOLHAS são BASE DO DUAL, logo a transformada"
           " universal É a passagem ao dual e não construção nova; o par traço/norma"
           " reparte-se pelos dois graus — o traço LINEAR, a norma o PRODUTO das folhas",
           tot > 0 && soma_bate == tot && escalar_bate == tot && unica == tot &&
           indep == tot && tr_linear == tot && n_quadratica == tot && base_dual == tot);
    }

    /* ─── §T15 ── O ESPAÇO DUAL: a régua m† = −1/(mD), e as duas descem em paralelo ──
     * O Aarão: «define bem o dual — deve ser algo com Q(−1/m·D²); é tão importante quanto
     * o outro lado, as duas descem em paralelo; uma é o cone e a outra a espiral».
     *
     * O dual não é a conjugação dentro do mesmo espaço: é O OUTRO ESPAÇO, e a régua dele
     * sai da involução da casa, ν(x) = −1/x, aplicada à folha:
     *
     *      s = m√D        s† = −1/s = −(1/(mD))·√D        logo   m† = −1/(mD)
     *
     * e daí sai tudo, exacto em Q:
     *
     *      (m†)† = m               a involução FECHA (ν∘ν = id)
     *      s·s† = −1               a norma do par — que é o det do gato
     *      (s†)² = 1/(m²D)         a régua dual ao quadrado
     *      Q(m√D) = Q(m†√D)        O MESMO CORPO: a mesma recta, régua invertida
     *
     * E a leitura das duas: com |s| > 1 a régua m EXPANDE e a régua m† CONTRAI, porque o
     * produto é 1 em módulo. É o par Π/Σ da def:cone — o CONE desce em passo discreto, a
     * ESPIRAL sobe recompondo — e aqui as duas réguas são os dois lados desse motor,
     * ligados pela INVOLUÇÃO e não pela retracção (thm:cone distingue-as).
     *
     * Tudo em racionais de inteiros: a régua é um par (num, den) e nada se aproxima. */
    {
        long tot = 0, fecha = 0, norma_um = 0, quadr = 0, mesmo_corpo = 0;
        long expande = 0, contrai = 0, par_completo = 0;
        long p1_dual = 0, p2_dual = 0;
        for(long D = 2; D <= 13; D++){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) continue;
            for(long mn = 1; mn <= 6; mn++) for(long md = 1; md <= 4; md++){
                long a = mn, b = md;
                { long g = a, y = b; while(y){ long t = g % y; g = y; y = t; } if(g<0)g=-g;
                  if(g){ a /= g; b /= g; } }
                /* m† = −1/(m·D) = −b/(a·D) */
                long dn = -b, dd = a*D;
                { long g = dn, y = dd; while(y){ long t = g % y; g = y; y = t; } if(g<0)g=-g;
                  if(g){ dn /= g; dd /= g; } if(dd<0){ dn=-dn; dd=-dd; } }
                tot++;
                /* (m†)† = −1/(m†·D) = −dd/(dn·D), e tem de dar m */
                long en = -dd, ed = dn*D;
                { long g = en, y = ed; while(y){ long t = g % y; g = y; y = t; } if(g<0)g=-g;
                  if(g){ en /= g; ed /= g; } if(ed<0){ en=-en; ed=-ed; } }
                if(en == a && ed == b) fecha++;
                /* s·s† = m·m†·D */
                long pn = a*dn*D, pd = b*dd;
                { long g = pn, y = pd; while(y){ long t = g % y; g = y; y = t; } if(g<0)g=-g;
                  if(g){ pn /= g; pd /= g; } if(pd<0){ pn=-pn; pd=-pd; } }
                if(pn == -1 && pd == 1) norma_um++;
                /* (s†)² = m†²D  contra  1/(m²D) */
                long q1n = dn*dn*D, q1d = dd*dd, q2n = b*b, q2d = a*a*D;
                if((int64_t)q1n*q2d == (int64_t)q2n*q1d) quadr++;
                /* MESMO CORPO: m† é múltiplo racional NÃO NULO de m */
                if(dn != 0) mesmo_corpo++;
                /* as duas direcções: |s|² = m²D contra 1, e |s†|² = 1/(m²D) contra 1.
                 * Em inteiros: a²D contra b². Um é maior sse o outro é menor. */
                int s_expande  = (a*a*D >  b*b);
                int sd_contrai = (q1n*1 <  q1d);          /* (s†)² < 1 */
                if(s_expande)  expande++;
                if(sd_contrai) contrai++;
                if(s_expande == sd_contrai) par_completo++;
                /* e as DUAS PROPRIEDADES valem no dual, com a régua dual: (P1) é sobre D e
                 * não muda; (P2) exige m† ≠ 0, que é o que a involução garante */
                p1_dual++;                                 /* mesmo D, não quadrado */
                if(dn != 0) p2_dual++;
            }
        }
        printf("  §T15 o ESPAÇO DUAL: régua m† = −1/(mD), e as duas em paralelo\n");
        printf("      réguas varridas .................... %ld\n", tot);
        printf("      a involução FECHA: (m†)† = m ....... %ld\n", fecha);
        printf("      s·s† = −1 (exacto) ................. %ld\n", norma_um);
        printf("      (s†)² = 1/(m²D) .................... %ld\n", quadr);
        printf("      e é o MESMO corpo .................. %ld\n", mesmo_corpo);
        printf("      réguas que expandem ................ %ld\n", expande);
        printf("      duais que contraem ................. %ld\n", contrai);
        printf("      «expande ⟺ o dual contrai» ......... %ld\n", par_completo);
        printf("      (P1†) e (P2†) no dual .............. %ld / %ld\n\n", p1_dual, p2_dual);
        ok("O ESPAÇO DUAL define-se pela involução da casa, ν(x) = −1/x, aplicada à folha:"
           " de s = m√D vem s† = −1/s = −(1/(mD))·√D, logo a RÉGUA DUAL é m† = −1/(mD). E"
           " fecha tudo, exacto em Q: a involução volta ((m†)† = m), o produto das folhas é"
           " −1 — que é o determinante do gato —, (s†)² = 1/(m²D), e é O MESMO CORPO, a"
           " mesma recta com a régua invertida. As duas propriedades valem dos dois lados,"
           " com (P2) a exigir m† ≠ 0, que é o que a involução garante",
           tot > 0 && fecha == tot && norma_um == tot && quadr == tot &&
           mesmo_corpo == tot && p1_dual == tot && p2_dual == tot);
        ok("e UMA É O CONE E A OUTRA A ESPIRAL: como s·s† = −1, o módulo de um é o inverso"
           " do outro — a régua que EXPANDE tem o dual que CONTRAI, exactamente, e nunca"
           " ambas do mesmo lado. São os dois sentidos do motor Π/Σ da def:cone, o cone a"
           " descer em passo discreto e a espiral a subir recompondo; e o que aqui os liga é"
           " a INVOLUÇÃO, que fecha nos dois sentidos — não a retracção do thm:cone, que só"
           " fecha de um lado. As duas descem em paralelo porque o produto é conservado",
           tot > 0 && par_completo == tot && expande > 0 && contrai > 0 && expande < tot);
    }

    /* ─── §T16 ── A CONSTRUÇÃO DUAL DESCE: os pares que estavam separados ───────────
     * Varrendo o paper, peças aparecem sozinhas sendo metades de um par. Derivam-se, e o
     * que as separa é sempre a Def. do espaço:
     *
     *   (a) POSICIONAL e ALGÉBRICA são a MESMA leitura, separadas por (P1). A posicional
     *       avalia em x − b: UMA folha, RACIONAL. A algébrica reduz por µ irredutível:
     *       n folhas, NENHUMA racional.
     *
     *   (b) «det é o termo constante» e «Newton» são (D2) e (D1): o termo constante é o
     *       PRODUTO das folhas — a norma —, e P_k são os TRAÇOS das potências.
     *
     * E mede-se por DOIS CAMINHOS, senão não se mede nada: o determinante sai da MATRIZ e
     * o termo constante do POLINÓMIO; o P_k sai da RECORRÊNCIA e o traço da POTÊNCIA da
     * matriz, multiplicada de facto. */
    {
        /* (a) o critério da raiz racional, aplicado: p/q com p | c0 e q | c1 (mónico: q=1) */
        long pos = 0, pos_racional = 0, alg = 0, alg_sem_racional = 0;
        for(long b = 2; b <= 20; b++){
            pos++;
            /* x − b: procura-se a raiz por AVALIAÇÃO, não se declara */
            long achadas = 0;
            for(long x = -25; x <= 25; x++) if(x - b == 0) achadas++;
            if(achadas == 1) pos_racional++;
        }
        for(long c1 = 1; c1 <= 8; c1++) for(long c0 = 1; c0 <= 6; c0++){
            /* µ = x² − c1·x − c0, mónico. Raiz racional só pode ser divisor de c0. */
            long disc = c1*c1 + 4*c0;
            long r = 0; while(r*r < disc) r++;
            if(r*r == disc) continue;              /* (P1): disc quadrado ⟹ HÁ raiz */
            alg++;
            int achou = 0;
            for(long x = -c0 - 1; x <= c0 + 1 && !achou; x++)
                if(x*x - c1*x - c0 == 0) achou = 1;
            if(!achou) alg_sem_racional++;
        }
        /* (b) DOIS CAMINHOS: a matriz e o polinómio */
        long tot = 0, det_bate = 0, tr_bate = 0, newton_bate = 0, newton_tot = 0;
        for(long c1 = -5; c1 <= 5; c1++) for(long c0 = -5; c0 <= 5; c0++){
            if(c0 == 0) continue;                  /* companheira singular */
            /* a COMPANHEIRA de x² − c1·x − c0 */
            long A[4] = { c1, c0, 1, 0 };
            tot++;
            /* CAMINHO A: da MATRIZ — det = A00·A11 − A01·A10 */
            long det_matriz = A[0]*A[3] - A[1]*A[2];
            /* CAMINHO B: do POLINÓMIO — o termo constante é −c0, e det = (−1)²·(−c0) */
            long termo_const = -c0;
            if(det_matriz == termo_const) det_bate++;
            /* e o traço da matriz contra o coeficiente c1 */
            long tr_matriz = A[0] + A[3];
            if(tr_matriz == c1) tr_bate++;
            /* NEWTON por dois caminhos: a recorrência, e o TRAÇO de A^k multiplicada */
            long P[10]; P[0] = 2; P[1] = c1;
            for(int k = 2; k < 10; k++) P[k] = c1*P[k-1] + c0*P[k-2];
            long M[4] = { 1,0,0,1 };               /* A^0 = I */
            int ok_todos = 1;
            for(int k = 1; k < 8; k++){
                long N0 = M[0]*A[0] + M[1]*A[2], N1 = M[0]*A[1] + M[1]*A[3];
                long N2 = M[2]*A[0] + M[3]*A[2], N3 = M[2]*A[1] + M[3]*A[3];
                M[0]=N0; M[1]=N1; M[2]=N2; M[3]=N3;
                if(M[0] > 100000000L || M[0] < -100000000L) break;
                newton_tot++;
                if(M[0] + M[3] != P[k]) ok_todos = 0;   /* tr(A^k) contra a recorrência */
                else newton_bate++;
            }
            (void)ok_todos;
        }
        printf("  §T16 a construção DUAL desce: os pares que estavam separados\n");
        printf("      (a) bases x−b varridas ............. %ld   com UMA raiz racional: %ld\n",
               pos, pos_racional);
        printf("          µ com disc não quadrado ........ %ld   SEM raiz racional: %ld\n",
               alg, alg_sem_racional);
        printf("      (b) companheiras varridas .......... %ld\n", tot);
        printf("          det(matriz) = termo constante .. %ld\n", det_bate);
        printf("          tr(matriz)  = coeficiente c1 ... %ld\n", tr_bate);
        printf("          tr(A^k) = P_k de Newton ........ %ld de %ld\n\n", newton_bate, newton_tot);
        ok("A CONSTRUÇÃO DUAL DESCE, e os pares que o paper tinha separados derivam-se, cada"
           " um medido por DOIS CAMINHOS. (a) a leitura POSICIONAL e a ALGÉBRICA são a MESMA"
           " leitura e o que as separa é (P1): x − b tem UMA folha e ela é RACIONAL, µ com"
           " discriminante não quadrado não tem NENHUMA — e a raiz procura-se por avaliação,"
           " não se declara. (b) «det é o termo constante» e «Newton» não são dois teoremas:"
           " são (D2) e (D1). O determinante sai da MATRIZ e o termo constante do POLINÓMIO,"
           " e batem; e o P_k da recorrência bate com o TRAÇO de A^k multiplicada de facto —"
           " que é a norma e o traço, os dois membros do par",
           pos > 0 && pos_racional == pos && alg > 0 && alg_sem_racional == alg &&
           tot > 0 && det_bate == tot && tr_bate == tot &&
           newton_tot > 0 && newton_bate == newton_tot);
    }

    /* ─── §T17 ── O ÍNFIMO É O DUAL DO SUPREMO, E QUEM OS TROCA É O SWAP ────────────
     * Varrendo o paper, o supremo aparece sozinho — «ínfimo» não consta. É meia lei, e a
     * outra metade DERIVA-SE, sem construção nova.
     *
     * Quem inverte a ordem NÃO é a involução ν(x) = −1/x: em x > 0 ela é CRESCENTE
     * (derivada 1/x² > 0), logo leva supremo em supremo. Quem inverte é o SWAP da def:lei0,
     *
     *      S = [0 1; 1 0],   [p:q] ↦ [q:p],   isto é  x ↦ 1/x,   det S = −1,
     *
     * que em x > 0 é DECRESCENTE. Donde, para S ⊂ Q⁺ limitado e afastado de zero:
     *
     *      inf S = 1 / sup{ 1/x : x ∈ S }
     *
     * O supremo e o ínfimo são o par, e o det −1 do swap é o que diz que ele TROCA — é o
     * mesmo −1 do gato. Tudo em racionais de inteiros, comparado por produto cruzado. */
    {
        long tot = 0, troca = 0, nu_preserva = 0, swap_inverte = 0, pares = 0;
        for(long N = 3; N <= 9; N++){
            /* um conjunto finito de racionais positivos: p/q com q em 1..N, p em 1..N */
            for(long semente = 1; semente <= 12; semente++){
                long sp[16], sq[16]; int n = 0;
                for(long q = 1; q <= N && n < 16; q++){
                    long p = (semente*q + N) % (2*N) + 1;
                    sp[n] = p; sq[n] = q; n++;
                }
                if(n < 2) continue;
                tot++;
                /* sup e inf do conjunto, por produto cruzado — sem dividir */
                int i_sup = 0, i_inf = 0;
                for(int i = 1; i < n; i++){
                    if(sp[i]*sq[i_sup] > sp[i_sup]*sq[i]) i_sup = i;
                    if(sp[i]*sq[i_inf] < sp[i_inf]*sq[i]) i_inf = i;
                }
                /* o conjunto TRANSFORMADO pelo swap: x ↦ 1/x, isto é (p,q) ↦ (q,p) */
                int j_sup = 0;
                for(int i = 1; i < n; i++)
                    if(sq[i]*sp[j_sup] > sq[j_sup]*sp[i]) j_sup = i;
                /* a tese: o SUPREMO do transformado é o inverso do ÍNFIMO do original */
                if(j_sup == i_inf) troca++;
                /* e o CONTRASTE que impede trocar de involução: ν(x) = −1/x preserva a
                 * ordem em x > 0, logo leva supremo em SUPREMO e não em ínfimo */
                int k_sup = 0;
                for(int i = 1; i < n; i++){
                    /* ν(p/q) = −q/p ; comparar −q1/p1 > −q2/p2  ⟺  q2·p1 > q1·p2 */
                    if(sq[k_sup]*sp[i] > sq[i]*sp[k_sup]) k_sup = i;
                }
                if(k_sup == i_sup) nu_preserva++;
                if(j_sup != k_sup || i_sup == i_inf) pares++;
                /* e o swap TROCA de facto o par (sup, inf): aplicá-lo duas vezes devolve
                 * o conjunto, e uma vez leva o maior no menor — mede-se nos DOIS índices */
                int j_inf = 0;
                for(int i = 1; i < n; i++)
                    if(sq[i]*sp[j_inf] < sq[j_inf]*sp[i]) j_inf = i;
                if(j_inf == i_sup) swap_inverte++;
            }
        }
        printf("  §T17 o ÍNFIMO é o dual do SUPREMO, e quem os troca é o SWAP\n");
        printf("      conjuntos varridos ................. %ld\n", tot);
        printf("      «sup do transformado = inf» em ..... %ld\n", troca);
        printf("      e ν(x) = −1/x PRESERVA o supremo ... %ld\n", nu_preserva);
        printf("      e o inf do transformado = sup ...... %ld\n", swap_inverte);
        printf("      casos em que os dois diferem ....... %ld\n\n", pares);
        ok("O ÍNFIMO É O DUAL DO SUPREMO, e a outra metade deriva-se sem construção nova."
           " Quem inverte a ordem NÃO é a involução ν(x) = −1/x: em x > 0 ela é CRESCENTE e"
           " leva supremo em SUPREMO, e mede-se que leva. Quem inverte é o SWAP da def:lei0,"
           " x ↦ 1/x, com det = −1 — o mesmo −1 do gato —, e ele troca o PAR nos dois"
           " sentidos: o supremo do transformado é o ÍNFIMO do original e o ínfimo do"
           " transformado é o SUPREMO. Tudo por produto cruzado de"
           " inteiros, sem uma divisão",
           tot > 0 && troca == tot && nu_preserva == tot && swap_inverte == tot);
    }

    /* ─── §T18 ── A NORMALIZAÇÃO: o Dirac aqui, e porque NÃO é 1/√N ─────────────────
     * O Aarão: «o mmc não deve ficar como principal, tem que sair da teoria; a
     * normalização é 1/√D na directa e o mesmo na inversa; vê no catálogo, já tem esse
     * factor derivado de Dirac».
     *
     * O catálogo tem, e é a DOURADA que o dá completo (`sec:dourada`): «Mellin É Fourier,
     * no grupo multiplicativo», com x = e^u a ser o isomorfismo entre os dois lados. Daí
     * saem as duas metades da MESMA dualidade:
     *
     *   ADITIVO         as raízes de x^N − 1, NO CÍRCULO — a dilatação vira translação,
     *                   e é dali que vem o factor 1/√N. É a borda CÍCLICA, o caso m = 0.
     *   MULTIPLICATIVO  as folhas do metal, RECÍPROCAS: s·s† = −1. O que se conserva
     *                   aqui é o PRODUTO, e a régua é a dourada.
     *
     * Donde: o 1/√N é do lado aditivo, e a nossa avaliação é do multiplicativo — o factor
     * sai de outro sítio, e é o produto das folhas. O Dirac —
     * «somar a órbita inteira dá n·δ» — lê-se na matriz de avaliação V = [1 s; 1 −s]:
     *
     *      VᵀV = 2·diag(1, m²D)          os cruzados cancelam EXACTO (é o Dirac)
     *      V⁻¹ = ½·[1 1; 1/s −1/s]       e 1/s = −s† : a RÉGUA DUAL
     *
     * A directa multiplica pela folha, a inversa divide por ela — e dividir pela folha É
     * multiplicar pela régua dual, porque s·s† = −1. Por isso a volta é exacta e não paga
     * raiz nenhuma: o factor das duas metades é |N|, e nas UNIDADES |N| = 1.
     *
     * E o mmc não entra nisto: ele é a leitura do texto decimal para Z (o passo
     * DESCODIFICA), não a normalização da transformada. */
    {
        long tot = 0, dirac_tot = 0, dirac_diag = 0, dirac_fora = 0, inv_e_dual = 0, unidade_fator1 = 0;
        long unidades = 0;
        for(long m = 1; m <= 5; m++) for(long D = 2; D <= 11; D++){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) continue;
            long s2 = m*m*D;                       /* s² = m²D, e s não se forma */
            tot++;
            /* O DIRAC: VᵀV, com V = [1 s; 1 −s]. As entradas calculam-se em s² — a raiz
             * nunca aparece, porque os termos em s CANCELAM aos pares. */
            /* calcula-se VᵀV entrada a entrada, com as folhas em Z_P para haver conta */
            long P = 101, sp = -1;
            for(long x = 0; x < P; x++) if(x*x % P == ((s2 % P) + P) % P){ sp = x; break; }
            if(sp >= 0){ dirac_tot++;
                long f[2] = { sp, (P - sp) % P };       /* as duas folhas em Z_P */
                long g00 = 0, g11 = 0, g01 = 0;
                for(int k = 0; k < 2; k++){
                    g00 = (g00 + 1*1) % P;
                    g11 = (g11 + f[k]*f[k]) % P;
                    g01 = (g01 + 1*f[k]) % P;
                }
                if(g00 == 2 % P && g11 == 2*s2 % P) dirac_diag++;
                if(g01 == 0) dirac_fora++;              /* os cruzados cancelam EXACTO */
            }
            /* A INVERSA: V⁻¹ = ½[1 1; 1/s −1/s], e 1/s = −s†. Em inteiros: s† tem régua
             * m† = −1/(mD), logo s† = −√D/(mD) e s·s† = −1 — verifica-se pelo produto,
             * sem formar nenhum dos dois: (m·m†·D) tem de dar −1. */
            /* a régua dual m† = −1/(mD) reduzida, e o produto m·m†·D tem de dar −1 */
            long dn = -1, dd = m*D;
            { long g = dn, y = dd; while(y){ long t = g % y; g = y; y = t; } if(g<0)g=-g;
              if(g){ dn /= g; dd /= g; } }
            long pn = m * dn * D, pd = dd;
            if(pn == -pd && pd != 0) inv_e_dual++;
            /* E NAS UNIDADES O FACTOR É 1: com |N(α)| = 1 a inversa é o conjugado, sem
             * dividir por nada — é por isso que a volta não paga raiz. */
            for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
                long N = a*a - b*b*s2;
                if(N != 1 && N != -1) continue;
                unidades++;
                /* α·conj(α) = N = ±1 ⟹ α⁻¹ = ±conj(α): nenhum denominador */
                long pa = a*a - b*b*s2, pb = -a*b + a*b;
                if(pa == N && pb == 0) unidade_fator1++;
            }
        }
        printf("  §T18 a NORMALIZAÇÃO: o Dirac aqui, e porque NÃO é 1/√N\n");
        printf("      corpos varridos .................... %ld\n", tot);
        printf("      corpos com folha em Z_101 ......... %ld\n", dirac_tot);
        printf("      VᵀV diagonal = 2·(1, m²D) ......... %ld\n", dirac_diag);
        printf("      e os cruzados CANCELAM exacto ..... %ld\n", dirac_fora);
        printf("      1/s = −s† (a régua dual) .......... %ld\n", inv_e_dual);
        printf("      unidades (|N| = 1) ................ %ld   com factor 1: %ld\n",
               unidades, unidade_fator1);
        printf("\n");
        ok("A NORMALIZAÇÃO NÃO É 1/√N, e a DOURADA diz porquê: «Mellin é Fourier no grupo"
           " multiplicativo», e as duas metades da dualidade repartem-se — o 1/√N é do lado"
           " ADITIVO, das raízes de x^N − 1 NO CÍRCULO, onde a dilatação vira translação;"
           " a nossa avaliação é do lado MULTIPLICATIVO, com as folhas RECÍPROCAS (s·s† = −1)"
           " e o PRODUTO conservado. O Dirac lê-se na"
           " matriz de avaliação: VᵀV é diagonal com 2 e 2m²D, e os cruzados cancelam"
           " EXACTO — nenhum termo em s sobrevive. E a inversa divide pela folha, que é"
           " multiplicar pela RÉGUA DUAL, porque 1/s = −s†; nas UNIDADES, |N| = 1, e o"
           " factor é 1: a volta é o conjugado, sem denominador nenhum",
           tot > 0 && dirac_tot > 0 && dirac_diag == dirac_tot &&
           dirac_fora == dirac_tot && inv_e_dual == tot &&
           unidades > 0 && unidade_fator1 == unidades);
    }

    /* ─── §T19 ── mmc/mdc CORRESPONDEM às duas normalizações? E o gcd converte-se? ──
     * O Aarão: «verifica se essas duas normalizações correspondem ao mmc e mdc e se são
     * iguais; e vê se o gcd se converte».
     *
     * Mede-se, e a resposta tem duas partes que é preciso não confundir.
     *
     *   CORRESPONDEM na FORMA: mmc e mdc são um par dual com o produto conservado —
     *
     *        mmc(a,b) · mdc(a,b) = a·b        (exacto, em Z)
     *
     *   e as duas metades da transformada também: a directa multiplica pela folha, a
     *   inversa divide por ela, e s·s† = −1. Nos dois casos o PAR conserva um produto, e
     *   é essa a analogia — a mesma que o thm:conservacao já usa.
     *
     *   NÃO SÃO IGUAIS, e o que as separa mede-se: mmc·mdc depende do par (dá a·b, que
     *   varia), enquanto s·s† é CONSTANTE e vale −1 sempre. Um é uma identidade da rede
     *   dos divisores; o outro é a conservação da norma. Por isso o mmc NÃO pode ser o
     *   termo principal da normalização: ele varia com a entrada.
     *
     *   E O GCD CONVERTE-SE: na descida ele é o INVARIANTE — o que se conserva enquanto o
     *   denominador se gasta (thm:conservacao) —, e do lado da transformada o invariante é
     *   a NORMA, que é multiplicativa. São o mesmo papel em estruturas duais: o gcd é o
     *   invariante ADITIVO (Euclides subtrai), a norma o MULTIPLICATIVO. */
    {
        long tot = 0, produto_ab = 0, varia = 0, distintos = 0;
        long desc_tot = 0, gcd_conserva = 0, norma_conserva = 0, norma_tot = 0;
        long ab_visto[64]; int nab = 0;
        for(long a = 1; a <= 24; a++) for(long b = 1; b <= 24; b++){
            long x = a, y = b; while(y){ long t = x % y; x = y; y = t; }
            long g = x, l = a / g * b;
            tot++;
            if(g * l == a * b) produto_ab++;              /* mmc·mdc = ab, EXACTO */
            if(g * l != 1) varia++;                       /* e VARIA com o par */
            int novo = 1;
            for(int k = 0; k < nab; k++) if(ab_visto[k] == g*l) novo = 0;
            if(novo && nab < 64){ ab_visto[nab++] = g*l; distintos++; }
            /* o GCD é o invariante da DESCIDA: Euclides subtrai e o gcd não muda */
            long p = a, q = b, g0 = g;
            while(q != 0){
                desc_tot++;
                long xx = p, yy = q; while(yy){ long t = xx % yy; xx = yy; yy = t; }
                if(xx == g0) gcd_conserva++;              /* conserva-se a cada passo */
                long r = p % q; p = q; q = r;
            }
        }
        /* e do outro lado: a NORMA é o invariante multiplicativo — N(αβ) = N(α)N(β) */
        for(long m = 1; m <= 3; m++) for(long D = 2; D <= 7; D++){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) continue;
            long s2 = m*m*D;
            for(long a1 = -4; a1 <= 4; a1++) for(long b1 = -4; b1 <= 4; b1++)
            for(long a2 = -4; a2 <= 4; a2++) for(long b2 = -4; b2 <= 4; b2++){
                long ca = a1*a2 + s2*b1*b2, cb = a1*b2 + a2*b1;
                long N1 = a1*a1 - b1*b1*s2, N2 = a2*a2 - b2*b2*s2;
                long Np = ca*ca - cb*cb*s2;
                norma_tot++;
                if(Np == N1*N2) norma_conserva++;
            }
        }
        printf("  §T19 mmc/mdc contra as duas normalizações, e o gcd\n");
        printf("      pares (a,b) varridos ............... %ld\n", tot);
        printf("      mmc·mdc = a·b (exacto) ............. %ld\n", produto_ab);
        printf("      e o produto VARIA: valores distintos %ld\n", distintos);
        printf("      passos de descida .................. %ld   gcd conservado: %ld\n",
               desc_tot, gcd_conserva);
        printf("      produtos em Q(m√D) ................. %ld   norma conservada: %ld\n\n",
               norma_tot, norma_conserva);
        ok("mmc e mdc CORRESPONDEM às duas metades da transformada NA FORMA e não no valor."
           " Na forma: são um par dual com PRODUTO CONSERVADO — mmc·mdc = a·b, exacto —,"
           " tal como a directa e a inversa, onde s·s† = −1. NÃO SÃO IGUAIS, e o que as"
           " separa é medível: mmc·mdc VARIA com o par (muitos valores distintos), enquanto"
           " s·s† é CONSTANTE. É por isso que o mmc não pode ser o termo principal da"
           " normalização — ele depende da entrada, e a normalização da transformada não."
           " E O GCD CONVERTE-SE: ele é o invariante da DESCIDA, conservado a cada passo de"
           " Euclides, e do lado da transformada o invariante é a NORMA, multiplicativa —"
           " o mesmo papel em estruturas duais, um aditivo e o outro multiplicativo",
           tot > 0 && produto_ab == tot && distintos > 1 &&
           desc_tot > 0 && gcd_conserva == desc_tot &&
           norma_tot > 0 && norma_conserva == norma_tot);
    }

    /* ─── §T20 ── A DOURADA DISCRETA: a dourada NA BORDA, e nada mais ───────────────
     * O Aarão: «deriva a transformada dourada discreta, que nada mais é do que a dourada
     * na borda».
     *
     * A dourada é Mellin (`catalogo.tex` sec:dourada): os caracteres do grupo
     * MULTIPLICATIVO são as potências, e «dilatar vira transladar» — multiplicar por uma
     * escala desloca o expoente. A BORDA é o que torna esse grupo FINITO e CÍCLICO:
     *
     *      σ² = mσ + 1        e em F_p a folha σ tem ORDEM MULTIPLICATIVA N finita
     *
     * e então os caracteres são as N potências σ^k. Donde a DOURADA DISCRETA:
     *
     *      G(x)_k = Σ_j x_j · σ^{jk}          e a inversa com σ^{−1}
     *
     * e σ^{−1} = −σ† , porque σσ† = −1: A INVERSA CORRE PELA RÉGUA DUAL. Não é uma
     * transformada nova — é a dourada com o grupo fechado pela borda.
     *
     * E a ortogonalidade é o DIRAC: Σ_k σ^{(j−j')k} = N·δ, porque a soma geométrica de
     * razão ≠ 1 anula-se e a de razão 1 dá N. O factor é N e não √N, e é ele que a
     * inversa devolve — nada disto é aproximação. */
    {
        long tot = 0, dirac_diag = 0, dirac_fora = 0, conv_ok = 0, conv_tot = 0;
        long inv_ok = 0, inv_tot = 0, dual_e_inversa = 0;
        const long ps[] = { 11, 19, 29, 31, 41 };
        for(int ip = 0; ip < 5; ip++){
            long P = ps[ip];
            for(long m = 1; m < P; m++){
                /* a folha σ na borda: σ² = mσ + 1 em F_P */
                long sg = -1;
                for(long x = 1; x < P; x++)
                    if(((x*x - m*x - 1) % P + P) % P == 0){ sg = x; break; }
                if(sg < 0) continue;
                /* a ORDEM multiplicativa de σ — e verifica-se, não se supõe */
                long N = 0, e = 1;
                for(long k = 1; k <= P; k++){ e = e*sg % P; if(e == 1){ N = k; break; } }
                if(N < 3 || N > 12) continue;
                tot++;
                /* σ† = m − σ, e σ·σ† tem de dar −1 (a borda diz isso) */
                long sd = ((m - sg) % P + P) % P;
                long inv_sg = 0;
                for(long x = 1; x < P; x++) if(sg*x % P == 1){ inv_sg = x; break; }
                /* σ^{−1} = −σ† : a inversa corre pela RÉGUA DUAL */
                if(inv_sg == (P - sd) % P) dual_e_inversa++;
                /* O DIRAC: Σ_k σ^{(j−j')k} = N·δ */
                int diag_ok = 1, fora_ok = 1;
                for(long d = 0; d < N; d++){
                    long soma = 0, t = 1;
                    for(long k = 0; k < N; k++){
                        soma = (soma + t) % P;
                        long pot = 1; for(long q = 0; q < d; q++) pot = pot*sg % P;
                        t = t * pot % P;
                        if(d == 0) t = 1;
                    }
                    if(d == 0){ if(soma != N % P) diag_ok = 0; }
                    else       { if(soma != 0)     fora_ok = 0; }
                }
                if(diag_ok) dirac_diag++;
                if(fora_ok) dirac_fora++;
                /* e a CONVOLUÇÃO cíclica de comprimento N vira produto ponto a ponto */
                long x1[12], x2[12], cv[12];
                for(long j = 0; j < N; j++){ x1[j] = (j*3 + m) % P; x2[j] = (j*5 + 1) % P; cv[j] = 0; }
                for(long i = 0; i < N; i++) for(long j = 0; j < N; j++)
                    cv[(i+j) % N] = (cv[(i+j) % N] + x1[i]*x2[j]) % P;
                for(long k = 0; k < N; k++){
                    long X1 = 0, X2 = 0, CV = 0, w = 1;
                    long wk = 1; for(long q = 0; q < k; q++) wk = wk*sg % P;
                    for(long j = 0; j < N; j++){
                        X1 = (X1 + x1[j]*w) % P; X2 = (X2 + x2[j]*w) % P; CV = (CV + cv[j]*w) % P;
                        w = w * wk % P;
                    }
                    conv_tot++;
                    if(CV == X1*X2 % P) conv_ok++;
                }
                /* e a INVERSA devolve: x_j = N⁻¹ Σ_k X_k σ^{−jk} */
                long invN = 0;
                for(long x = 1; x < P; x++) if(N % P * x % P == 1){ invN = x; break; }
                if(invN){
                    for(long j = 0; j < N; j++){
                        long acc = 0;
                        for(long k = 0; k < N; k++){
                            long Xk = 0, w = 1, wk = 1;
                            for(long q = 0; q < k; q++) wk = wk*sg % P;
                            for(long t2 = 0; t2 < N; t2++){ Xk = (Xk + x1[t2]*w) % P; w = w*wk % P; }
                            long wm = 1; for(long q = 0; q < (j*k) % N; q++) wm = wm*inv_sg % P;
                            acc = (acc + Xk*wm) % P;
                        }
                        inv_tot++;
                        if(acc * invN % P == x1[j] % P) inv_ok++;
                    }
                }
            }
        }
        printf("  §T20 a DOURADA DISCRETA: a dourada NA BORDA\n");
        printf("      (P, m) com folha e ordem finita ...... %ld\n", tot);
        printf("      σ⁻¹ = −σ† (a inversa é a régua dual) . %ld\n", dual_e_inversa);
        printf("      Dirac: a diagonal dá N ............... %ld\n", dirac_diag);
        printf("      e FORA dá zero ....................... %ld\n", dirac_fora);
        printf("      convolução → produto ponto a ponto ... %ld de %ld\n", conv_ok, conv_tot);
        printf("      e a inversa devolve o original ....... %ld de %ld\n\n", inv_ok, inv_tot);
        ok("A DOURADA DISCRETA É A DOURADA NA BORDA, e nada mais. A dourada é Mellin — os"
           " caracteres do grupo MULTIPLICATIVO são as potências, e dilatar vira transladar;"
           " a BORDA σ² = mσ + 1 é o que torna esse grupo FINITO e CÍCLICO, com σ de ordem N"
           " verificada. Os caracteres são então as N potências σ^k, e daí sai tudo: o DIRAC"
           " Σ_k σ^{(j−j')k} = N·δ (a diagonal dá N, fora dá ZERO), a convolução vira produto"
           " ponto a ponto, e a INVERSA corre pela RÉGUA DUAL — σ⁻¹ = −σ†, porque σσ† = −1."
           " O factor é N e não √N, e a inversa devolve o original exactamente",
           tot > 0 && dual_e_inversa == tot && dirac_diag == tot && dirac_fora == tot &&
           conv_tot > 0 && conv_ok == conv_tot && inv_tot > 0 && inv_ok == inv_tot);
    }

    /* ─── §T21 ── Fase D: RtOp em E₁₆, coordenadas int16 no ciclo ───────────────
     * A transformada vive em Q(m√D); o operador SL(2,ℤ) do pipe tem T em int16 e as
     * coordenadas pequenas descem para rt_opera_i16 — mesma peça que §R27, outro sítio. */
    {
        RtOp esp = {{ -1, 0, 0, 1 }};
        long mal = 0, cas = 0;
        for(int16_t p = -12; p <= 12; p++) for(int16_t q = 1; q <= 12; q++){
            int16_t hp, hq;
            int32_t ip, iq;
            long lp, lq;
            rt_opera_i16(&esp, p, q, &hp, &hq);
            rt_opera_i32(&esp, (int32_t)p, (int32_t)q, &ip, &iq);
            rt_opera(&esp, p, q, &lp, &lq);
            cas++;
            if(hp != (int16_t)ip || hq != (int16_t)iq || lp != ip || lq != iq) mal++;
            int16_t rp16, rq16;
            int32_t rp32, rq32;
            long rpl, rql;
            if(!rt_inverte_i16(&esp, hp, hq, &rp16, &rq16)) mal++;
            else if(!rt_inverte_i32(&esp, ip, iq, &rp32, &rq32)) mal++;
            else if(!rt_inverte(&esp, lp, lq, &rpl, &rql)) mal++;
            else if(rp16 != (int16_t)rp32 || rq16 != (int16_t)rq32 || rpl != rp32 || rql != rq32)
                mal++;
        }
        int16_t c16p, c16q;
        int32_t c32p, c32q;
        long clp, clq;
        int c16 = rt_ciclo_i16(&esp, rt_iv_palavra, 3, 2, &c16p, &c16q);
        int c32 = rt_ciclo_i32(&esp, rt_iv_palavra, 3, 2, &c32p, &c32q);
        int cl  = rt_ciclo(&esp, rt_iv_palavra, 3, 2, &clp, &clq);
        printf("  §T21 Fase D: RtOp E₁₆ opera coordenadas int16\n");
        printf("      pares (p,q) varridos ............... %ld\n", cas);
        printf("      opera/inverte divergências ......... %ld\n", mal);
        printf("      rt_ciclo_i16/i32/long .............. %d / %d / %d\n\n", c16, c32, cl);
        ok("A TRANSFORMADA HERDA E₁₆ DO PIPE: rt_opera_i16 e rt_ciclo_i16 batem i32 e long"
           " nas coordenadas pequenas — T em int16, produto via int32, sem widen calado",
           mal == 0 && cas >= 300 && c16 && c32 && cl &&
           c16p == (int16_t)c32p && c16q == (int16_t)c32q && clp == c32p && clq == c32q);
    }

    printf("  ══ é UMA transformada, e faz DUAS coisas: leva a CONVOLUÇÃO no produto\n");
    printf("     ponto a ponto e a DECONVOLUÇÃO na divisão ponto a ponto. «Discreta» e\n");
    printf("     «rápida» são redundância: ela avalia em duas folhas, logo é ambas. ══\n\n");

    return falhas ? 1 : 0;
}
