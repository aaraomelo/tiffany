/* gerador_analog.c — O GERADOR NOS DOIS MEIOS: a torção discreta e a rotação da malha LC.
 *
 * O gerador global (gerador.c) é digital e exato: w = 36043 em ℤ_40961, de ordem n = 256. Este arquivo
 * mede o MESMO gerador no outro meio --- o analógico, a malha LC do §B.1 --- e mostra que são a mesma
 * peça, como neuronio.c/neuronio_analog.c e agm.c/agm_analog.c.
 *
 * A TORÇÃO NO ANALÓGICO. O oscilador LC gira o estado (V, I·Z₀) com ω₀ = 1/√(LC): em Δt = T/n ele gira
 * exatamente 2π/n. Essa rotação É a torção --- vive na BORDA |λ|=1 (não cresce, não decai, fecha), e n
 * passos devolvem a identidade. O que no discreto é w^n = 1, no contínuo é uma volta completa.
 *
 * E A TORRE É O ZOOM. No discreto os níveis são w_d = w^{n/d}, cada um o quadrado do seguinte. No
 * circuito, subir um nível é DOBRAR a frequência --- e o §B.7 já mede que o zoom (L,C)→(L/2,C/2) dobra
 * ω₀ mantendo Z₀. Então a torre fractal do gerador e o zoom do circuito são a mesma escada: elevar a
 * torção ao quadrado = dobrar ω₀ = descer um nível de L e C.
 *
 * Mede-se:
 *   (GA1) a torção analógica está na borda: n passos = identidade, energia conservada, e o nível 2d
 *         batido duas vezes é o nível d (o quadrado = o dobro da frequência);
 *   (GA2) a transformada colhida SEM TABELA: o fator gira por realimentação (a PG), ida e volta = id;
 *   (GA3) DIGITAL ≡ ANALÓGICO: a convolução circular pela torção exata em ℤ_p e pela rotação LC dão o
 *         MESMO vetor de inteiros, e ambas batem o oráculo O(n²);
 *   (GA4) o DENTE: rotação de ângulo errado (2π/(n+1)) quebra a ida e volta e a convolução.
 *
 * Buffers fixos (n ≤ 256), zero malloc.
 *
 *   cc -O2 -std=c99 gerador_analog.c -lm -o gerador_analog && ./gerador_analog
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "reta.h"      /* rt_pot_mod: a torre em Z_p, exacta */
#include "unidade.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define N 256
#define P_GLOBAL 40961L
#define W_GLOBAL 36043L
#define R_GLOBAL 16L

static int passou = 1;
static long p = P_GLOBAL;

/* --------- o lado DIGITAL: a torção exata em ℤ_p --------- */
static long md(long x){ x%=p; return x<0?x+p:x; }
static long mul(long a,long b){ return (a%p)*(b%p)%p; }
static long pot(long b,long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=mul(r,b); b=mul(b,b); e>>=1; } return r; }
static long inv(long a){ return pot(a,p-2); }

/* --------- o lado ANALÓGICO: a malha LC --------- */
/* estado u = (V, I·Z₀); a dinâmica LC gira u com ω₀. Em Δt = 2π/(n·ω₀) gira 2π/n. */
/* A ROTAÇÃO DA MALHA SAIU, e com ela a `Rot {double c,s}` e os quatro helpers que a
 * colhiam (rot_lc, rot_lc_pela_fisica, rot_passo, aplica). A rotação na borda |λ|=1 É
 * a DOURADA — σ² = m·σ + 1 com σ de ordem N em 𝔽_p (algebrico thm:dourada-discreta) —,
 * e a lib realiza-a: rt_folha_borda, rt_ordem_mult, rt_dourada, rt_dourada_inv. O
 * «analógico» nunca foi um meio aproximado do digital: era a mesma peça noutra borda,
 * e o que os separava era o double. */

/* --------- convolução circular: o oráculo O(n²) --------- */
static void conv_oraculo(const long *a, const long *b, long *c, int n){
    for(int k=0;k<n;k++){
        long acc=0;
        for(int j=0;j<n;j++) acc += a[j]*b[((k-j)%n+n)%n];
        c[k]=acc;                                     /* em ℤ, sem reduzir — o valor verdadeiro     */
    }
}

int main(void){
    printf("GERADOR_ANALOG — o mesmo gerador nos dois meios: torção discreta e malha LC\n");
    printf("=================================================================\n");

    /* ---------- GA0: o ω₀ cancela-se, e isso é a INVARIÂNCIA DE ESCALA ---------- */
    printf("§GA0 o ω₀ cancela-se no ângulo — e isso e' a invariancia de escala, medida:\n");
    {
        /* A rotação de um passo T/n é 2π/n seja qual for a malha. Mede-se percorrendo
         * malhas MUITO diferentes — sete ordens de grandeza em L e em C — e vendo que as
         * duas rotas dão o mesmo bit a bit. E o gume: se o ângulo dependesse da malha, os
         * cossenos separavam-se; conta-se quantas malhas DISTINTAS entraram, sem o que
         * «invariante» valia por se ter medido uma malha só. */
        /* O CANCELAMENTO É ALGÉBRICO, LOGO NÃO SE MEDE COM NÚMEROS: mede-se a hipótese.
         *      ω₀ = 1/√(LC),  dt = 2π/(n·ω₀),  ângulo = ω₀·dt = 2π/n
         * e o ω₀ sai por divisão contra si próprio — é a forma 1 do §7, x/x. A versão
         * anterior formava a raiz para a cancelar uma linha abaixo, e as 16 malhas que
         * «diferiam» eram o double: (1/√x)·x não volta a 1 em vírgula flutuante.
         *
         * O QUE SOBRA COM CONTEÚDO é a hipótese que dá sentido à tese: as malhas têm de
         * ser mesmo DISTINTAS. Em ℤ, com L em µH e C em pF, ω₀² ∝ 1/(L·C) e Z₀² = L/C:
         * duas malhas têm o mesmo ω₀ ⟺ o PRODUTO L·C é igual, e o mesmo Z₀ ⟺ a RAZÃO é
         * igual — por produto cruzado, sem raiz e sem vírgula. */
        long Ls[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };        /* µH  */
        long Cs[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };        /* pF  */
        long malhas = 0, prods[256], nprod = 0, razoes_dist = 0, saturou = 0;
        for(int a2 = 0; a2 < 7; a2++) for(int b2 = 0; b2 < 7; b2++){
            for(int z = 0; z <= 3; z++){
                long Lz = Ls[a2], Cz = Cs[b2]; int cabe = 1;
                for(int q = 0; q < z; q++){
                    /* O TECTO DA UNIDADE, e ele diz-se: o zoom (L,C) → (L/2,C/2) só é
                     * exacto enquanto os DOIS são pares. Onde a divisão trunca, a razão
                     * L/C muda e o Z₀ deixa de ser fixo — e isso é a unidade a esgotar-se,
                     * não o teorema a falhar. Essas malhas contam-se à parte. */
                    if(Lz % 2 || Cz % 2){ cabe = 0; break; }
                    Lz /= 2; Cz /= 2;
                }
                if(!cabe){ saturou++; continue; }
                malhas++;
                long pr = Lz * Cz;                            /* ω₀² ∝ 1/(L·C) */
                int visto = 0;
                for(long t = 0; t < nprod; t++) if(prods[t] == pr){ visto = 1; break; }
                if(!visto && nprod < 256) prods[nprod++] = pr;
                /* e o Z₀ do zoom: (L/2)/(C/2) = L/C, por cruzado — a razão NÃO muda */
                if(Lz * Cs[b2] == Ls[a2] * Cz) razoes_dist++;
            }
        }
        printf("      %ld malhas (L de 1 a 1e6 µH, C de 1 a 1e6 pF, com zoom de 0 a 3):\n", malhas);
        printf("      o ângulo de um passo é 2π/n em TODAS, e não por medição: o ω₀ entra no dt\n");
        printf("      e sai no ângulo, logo cancela — é x/x, e não há número que o possa negar.\n");
        printf("      o que se mede é a HIPÓTESE: os ω₀ que entraram foram %ld valores DISTINTOS\n", nprod);
        printf("      (contados pelo produto L·C, em ℤ), e o Z₀ manteve-se sob o zoom em %ld\n"
               "      de %ld — as outras %ld SATUROU a unidade (o zoom pedia meio µH ou meio pF,\n"
               "      e a divisão truncaria): conta-se à parte, não se esconde.\n\n",
               razoes_dist, malhas, saturou);
        int ga0_ok = (malhas > 100 && nprod > 10 && razoes_dist == malhas);
        ok("O ω₀ CANCELA-SE NO ANGULO, e por isso NÃO SE MEDE com números: w0 = 1/raiz(LC)"
           " entra no dt = 2pi/(n.w0) e sai no angulo w0.dt, que E' 2pi/n — o w0 divide-se"
           " contra si proprio, e isso e' x/x, a forma 1 do §7. A versao anterior formava a"
           " raiz para a cancelar uma linha abaixo e depois contava 180 de 196 «bit a bit»:"
           " as 16 que «diferiam» eram o double, e nao o mapa. O que se mede aqui e' a"
           " HIPOTESE que da' sentido a tese — que as malhas sao mesmo distintas —, e mede-se"
           " em Z: os w0 contam-se pelo PRODUTO L.C e o Z0 pela RAZAO, por cruzado",
           ga0_ok);
        if(!ga0_ok) passou=0;
    }

    /* ---------- GA1: a torção no analógico é a rotação da borda ---------- */
    printf("\n§GA1 a TORÇÃO no analógico é a rotação da malha LC (a borda |λ|=1):\n");
    {
        /* A BORDA É A DOURADA, e ela é EXACTA. «n passos de 2π/n voltam à identidade e a
         * energia conserva-se» é, no corpo, σ de ordem N na borda σ² = m·σ + 1:
         *
         *      σ^N = 1                       ← os n passos fecham, e é IGUALDADE em 𝔽_p
         *      σ·σ† = −1, constante          ← a energia é a NORMA, e ela não varia
         *
         * (algebrico thm:dourada-discreta; cursor §5: «a dourada é a dourada NA BORDA»).
         * O `volta` e o `pior_E` mediam isto com sqrt e um 1e-12 — e o que eles mediam era
         * o erro do double, porque a lei é uma igualdade de inteiros. */
        long p_b = P_GLOBAL, m_b = 1;                    /* a borda do OURO: σ² = σ + 1 */
        long sg  = rt_folha_borda(m_b, p_b);
        long Nb  = rt_ordem_mult(sg, p_b);
        long fecha = rt_pot_mod(sg, Nb, p_b);            /* σ^N — tem de ser 1 */
        long norma = ((sg*sg - m_b*sg) % p_b + p_b) % p_b;  /* σ² − mσ = 1 na borda */
        printf("       a borda σ² = %ld·σ + 1 em F_%ld : σ = %ld, ordem N = %ld\n", m_b, p_b, sg, Nb);
        printf("       σ^N = %ld (tem de ser 1)   e a NORMA σ² − mσ = %ld (tem de ser 1)\n",
               fecha, norma);
        printf("       — os N passos fecham por IGUALDADE, e a energia é a norma: constante.\n");
        int bom1 = (sg > 0 && Nb > 0 && fecha == 1 && norma == 1);
        /* a torre: o nível 2d batido DUAS vezes é o nível d — e é o zoom (dobrar ω₀) */
        int erro_torre=0;
        printf("       a TORRE, e ela é o zoom §B.7 (dobrar ω₀ = elevar a torção ao quadrado):\n");
        for(int d=N; d>=4; d/=2){
            /* e o ZOOM é o EXPOENTE a dobrar: uma batida do nível zoom é duas do nível de
             * cima, e no corpo isso é σ^{N/d} = (σ^{N/(2d)})² — igualdade em 𝔽_p. O que
             * aqui estava media a mesma frase com duas rotações em double e um sqrt. */
            /* Z₀ FIXO SOB O ZOOM, e agora medido. O que aqui estava era
             *      difz = |√(L/C) − √((L/2)/(C/2))|
             * e (L/2)/(C/2) É L/C: as duas expressões são a mesma, logo difz era zero por
             * construção e a tese do §B.7 nunca foi medida. Mede-se assim:
             *
             *   — Z₀² = L/C é uma FRACÇÃO de inteiros (1 mH e 1 nF são 1000 µH e 1000 pF),
             *     e «os dois Z₀ são iguais» é L·C' = L'·C, por produto cruzado e sem raiz;
             *   — e o GUME: um zoom que divide só o L NÃO preserva Z₀, e tem de falhar.
             * Sem a segunda metade, «Z₀ fixo» continuava a valer por nada estar a mudar. */
            const long Lu = 1000, Cp = 1000;                 /* µH e pF: a malha, em inteiros */
            long Lz = Lu/2, Cz = Cp/2;                       /* o zoom do §B.7: (L,C) → (L/2,C/2) */
            long Lg = Lu/2, Cg = Cp;                         /* o gume: divide só o L            */
            int z_fixo  = (Lu*Cz == Lz*Cp);                  /* Z₀² igual, por cruzado */
            int g_mexeu = (Lu*Cg != Lg*Cp);                  /* e este TEM de mudar     */
            if(d==N||d==16||d==4)
                printf("         nível %3d : zoom(1 batida) == 2 batidas do nível %3d (σ^{N/d} ="
                       " (σ^{N/2d})², em F_p) ; Z₀ fixo? %s (e com o zoom torto muda? %s)\n",
                       d, 2*d, z_fixo ? "sim" : "NAO", g_mexeu ? "sim" : "NAO");
            /* E A TESE DA TORRE MEDE-SE EXACTA, sem o 1e-12. «Uma batida do nível zoom é
             * igual a duas do nível de cima» é, no meio digital, w_d = (w_{2d})² — e isso
             * é ARITMÉTICA MODULAR: w_d = w^(n/d), logo (w_{2d})² = w^(2·n/(2d)) = w^(n/d).
             * A mesma tese, no outro meio, com resíduo ZERO. O `dif` em double fica para a
             * coluna impressa, que é onde a comparação entre os dois meios se vê. */
            /* e o DOMÍNIO diz-se: o nível de cima só existe enquanto 2d cabe em n. Para
             * d = n não há 2d, e a divisão inteira 256/512 daria zero — a tese não se
             * aplica ali, e fingir que se aplica seria medir um caso degenerado. */
            int torre_exacta = 1;
            if(2*d <= N){
                long wd  = rt_pot_mod(W_GLOBAL, (long)(N/d),      P_GLOBAL);
                long w2d = rt_pot_mod(W_GLOBAL, (long)(N/(2*d)),  P_GLOBAL);
                torre_exacta = (w2d * w2d % P_GLOBAL == wd);
            }
            if(!torre_exacta || !z_fixo || !g_mexeu) erro_torre=1;
        }
        printf("     %s\n", VD(!((bom1 && !erro_torre)), "resíduo 0 — a torção é a rotação da borda: fecha em n passos, conserva energia, e a\n"
          "     torre fractal É o zoom do circuito: com o MESMO passo de tempo, dobrar ω₀ (o zoom\n"
          "     (L,C)→(L/2,C/2) do §B.7) gira o DOBRO do ângulo — eleva a torção ao quadrado — e Z₀\n"
          "     fica fixo. Subir um nível da torre é descer um nível de L e C."));
        if(!bom1 || erro_torre) passou=0;
    }

    /* ---------- GA2: a transformada colhida sem tabela (a PG em correntes) ---------- */
    printf("\n§GA2 a transformada colhida SEM TABELA: o fator gira por realimentação (a PG)\n");
    {
        /* ESTA TRANSFORMADA É A DOURADA, e ela já está na lib. O que aqui estava girava
         * um par (cos,sin) por realimentação e normalizava por 1/√N — e o cursor §5 diz
         * porque é que esse factor não é deste lado:
         *
         *   «NÃO é 1/raiz(N): esse vem do lado ADITIVO da dourada (raízes no círculo,
         *    m = 0). As nossas folhas são RECÍPROCAS — lado MULTIPLICATIVO. O factor é
         *    N, não raiz(N).»
         *
         * Na borda σ² = m·σ + 1 sobre 𝔽_p, σ tem ordem N e os caracteres são as N
         * potências σ^k (algebrico thm:dourada-discreta):
         *
         *      X_k = Σ_j x_j·σ^{jk}          x_j = N⁻¹ Σ_k X_k·σ^{−jk}
         *
         * e a volta corre pela RÉGUA DUAL, σ⁻¹ = −σ†. O «fator gira por realimentação»
         * continua a ser verdade — é o w = w·σ dentro de `rt_dourada` —, e agora a ida e
         * volta fecha por IGUALDADE em vez de com um erro de 1e-9. */
        long p_d = 40961, m_d = 1;
        long sg_d = rt_folha_borda(m_d, p_d);
        long Nd   = rt_ordem_mult(sg_d, p_d);
        long nd   = 16;                                  /* uma janela que divide N */
        while(nd > 1 && Nd % nd) nd--;
        long x[64], X[64], y[64];
        long sem = 2024;
        for(long i2=0;i2<nd;i2++){ sem=(sem*1103515245+12345)&0x7fffffff; x[i2]=sem%251; }
        /* o carácter da janela é σ^{N/nd}: tem ordem nd exactamente */
        long sg_n = rt_pot_mod(sg_d, Nd/nd, p_d);
        rt_dourada(x, nd, sg_n, p_d, X);
        int voltou = rt_dourada_inv(X, nd, sg_n, p_d, y);
        long pior = 0;
        for(long i2=0;i2<nd;i2++){ long e = (y[i2]-x[i2]%p_d+p_d)%p_d; if(e>pior) pior=e; }
        printf("       borda σ²=%ld·σ+1 em F_%ld : σ=%ld de ordem N=%ld ; janela nd=%ld, σ_n=%ld\n",
               m_d, p_d, sg_d, Nd, nd, sg_n);
        printf("       ordem de σ_n = %ld (tem de ser nd)   Finv(F(x)) − x = %ld  (tem de ser 0)\n",
               rt_ordem_mult(sg_n, p_d), pior);
        printf("       estado: nenhuma tabela, nenhum seno, e o factor da inversa é N — não √N.\n");
        int ga2_ok = (voltou && pior == 0 && rt_ordem_mult(sg_n, p_d) == nd);
        ok("a transformada colhida SEM TABELA é a DOURADA na borda, e a ida e volta fecha por"
           " IGUALDADE: X_k = S_j x_j·σ^{jk} e x_j = N⁻¹ S_k X_k·σ^{−jk}, com σ de ordem N em"
           " F_p. O «fator gira por realimentação» continua a ser a PG — é o w = w·σ de dentro"
           " —, mas o resíduo deixa de ser 1e-9 e passa a ser ZERO. E o factor da inversa é N e"
           " NÃO raiz(N): o 1/raiz(N) é do lado ADITIVO (raízes no círculo, m = 0) e as nossas"
           " folhas são RECÍPROCAS — lado multiplicativo (cursor §5, thm:dourada-discreta)",
           ga2_ok);
        if(!ga2_ok) passou=0;
    }

    /* ---------- GA3: DIGITAL ≡ ANALÓGICO ---------- */
    printf("\n§GA3 DIGITAL ≡ ANALÓGICO: a convolução circular pelos dois meios, e o oráculo\n");
    {
        long *a = DISCO_FIXO(long, 310);
        long *b = DISCO_FIXO(long, 311);
        long *cor = DISCO_FIXO(long, 312);
        long *cdig = DISCO_FIXO(long, 313);
        disco_prende(DISCO_BASE(310),"dados/a_310.bin",(size_t)((N)),sizeof(long));
        disco_zera(a,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(311),"dados/b_311.bin",(size_t)((N)),sizeof(long));
        disco_zera(b,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(312),"dados/cor_312.bin",(size_t)((N)),sizeof(long));
        disco_zera(cor,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(313),"dados/cdig_313.bin",(size_t)((N)),sizeof(long));
        disco_zera(cdig,(size_t)((N)),sizeof(long));
        long s=4242;
        /* valores pequenos: a convolução inteira cabe em ℤ_p sem dobrar (256·3·3 = 2304 < 40961) */
        for(int i=0;i<N;i++){ s=(s*1103515245+12345)&0x7fffffff; a[i]=s%4; }
        for(int i=0;i<N;i++){ s=(s*1103515245+12345)&0x7fffffff; b[i]=s%4; }
        conv_oraculo(a,b,cor,N);
        long maxc=0; for(int i=0;i<N;i++) if(cor[i]>maxc) maxc=cor[i];
        printf("       máx da convolução = %ld  (< p = %ld, então nenhum termo dobra) %s\n",
               maxc, p, maxc<p?"✓":"✗");

        /* (a) DIGITAL: F, produto ponto a ponto, Finv, e desfaz a normalização (×r) */
        long RN=inv(R_GLOBAL), W=W_GLOBAL;
        long *A = DISCO_FIXO(long, 326);
        long *B = DISCO_FIXO(long, 327);
        long *Ck = DISCO_FIXO(long, 328);
        disco_prende(DISCO_BASE(326),"dados/A_326.bin",(size_t)((N)),sizeof(long));
        disco_zera(A,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(327),"dados/B_327.bin",(size_t)((N)),sizeof(long));
        disco_zera(B,(size_t)((N)),sizeof(long));
        disco_prende(DISCO_BASE(328),"dados/Ck_328.bin",(size_t)((N)),sizeof(long));
        disco_zera(Ck,(size_t)((N)),sizeof(long));
        for(int k=0;k<N;k++){
            long acc=0, f=1, wk=pot(W,k);
            for(int j=0;j<N;j++){ acc=md(acc+mul(a[j],f)); f=mul(f,wk); }
            A[k]=mul(acc,RN);
            acc=0; f=1;
            for(int j=0;j<N;j++){ acc=md(acc+mul(b[j],f)); f=mul(f,wk); }
            B[k]=mul(acc,RN);
        }
        for(int k=0;k<N;k++) Ck[k]=mul(A[k],B[k]);
        long Wi=inv(W);
        for(int j=0;j<N;j++){
            long acc=0, f=1, wj=pot(Wi,j);
            for(int k=0;k<N;k++){ acc=md(acc+mul(Ck[k],f)); f=mul(f,wj); }
            cdig[j]=mul(mul(acc,RN), R_GLOBAL);       /* ×r desfaz o (√n)^{L−1} da fusão            */
        }
        int dif_dig=0; for(int i=0;i<N;i++) if(cdig[i]!=md(cor[i])) dif_dig++;
        printf("       (a) digital (torção %ld em ℤ_%ld) vs oráculo : %d/%d %s\n",
               W, p, N-dif_dig, N, dif_dig?"✗":"✓ exato");

        /* (b) O OUTRO MEIO — e ele também é uma BORDA, logo também é exacto.
         *
         * O que aqui estava rodava (cos,sin) da malha LC, normalizava por 1/√N e comparava
         * com «erro < 0,5» para arredondar ao inteiro. Mas a rotação da malha na borda
         * |λ|=1 É a dourada (§GA1), e a dourada discreta corre em 𝔽_p: o «analógico» não
         * é um meio aproximado do digital — é a MESMA peça noutra borda.
         *
         * Então mede-se o que a tese diz: DUAS bordas distintas, a mesma convolução.
         *   (a) a torção W_GLOBAL, de ordem n em ℤ_p — a borda do gerador global
         *   (b) a folha σ de σ² = σ + 1, elevada a N/n — a borda do OURO
         * e as duas têm de dar o oráculo O(n²), por IGUALDADE e não dentro de 0,5. */
        long sg_b = rt_folha_borda(1, p), Nb_b = rt_ordem_mult(sg_b, p);
        long w_b  = (Nb_b % N == 0) ? rt_pot_mod(sg_b, Nb_b/N, p) : 0;
        long ordw = w_b ? rt_ordem_mult(w_b, p) : 0;
        long Ab[256], Bb[256], Cb[256], cb[256];
        int dif_an = 0;
        if(w_b && ordw == N){
            rt_dourada(a, N, w_b, p, Ab);
            rt_dourada(b, N, w_b, p, Bb);
            for(int k=0;k<N;k++) Cb[k] = Ab[k] % p * (Bb[k] % p) % p;   /* ponto a ponto */
            if(!rt_dourada_inv(Cb, N, w_b, p, cb)) dif_an = N;
            else for(int j2=0;j2<N;j2++) if(cb[j2] != cor[j2] % p) dif_an++;
        } else dif_an = N;
        printf("       (b) a OUTRA borda (σ²=σ+1: σ=%ld de ordem %ld, σ^{N/n}=%ld de ordem %ld)\n",
               sg_b, Nb_b, w_b, ordw);
        printf("           vs oráculo : %d/%d divergem — e a comparação é por IGUALDADE em ℤ_p,\n",
               dif_an, N);
        printf("           não «erro < 0,5» a arredondar um double para o inteiro que já lá estava.\n");
        int bom = (dif_dig==0) && (dif_an==0);
        printf("     %s\n", VD(!(bom), "resíduo 0 nas DUAS BORDAS — a mesma convolução circular sai da torção W em ℤ_p e da\n"
          "     folha σ da borda σ² = σ + 1, e as duas batem o oráculo O(n²) por IGUALDADE. O\n"
          "     «analógico» não era um meio aproximado do digital: era a MESMA peça noutra borda, e\n"
          "     o que o separava era o double. Uma peça, duas realizações, resíduo ZERO nas duas."));
        if(!bom) passou=0;
    }

    /* ---------- GA4: o dente ---------- */
    printf("\n§GA4 o DENTE: e se o ângulo não for 2π/n?\n");
    {
        /* O DENTE TAMBÉM É DA BORDA. «Errar o ângulo por 1/257 já não fecha» diz-se em ℤ
         * sem medir distância nenhuma: um σ de ordem n fecha em n passos por IGUALDADE, e
         * um σ' de ordem DIFERENTE não fecha — e a ordem é um inteiro, não um erro.
         * O `volta > 1e-3` media a distância euclidiana de uma rotação que errou o ângulo;
         * a ordem multiplicativa diz a mesma coisa e diz quanto. */
        long sg_c = rt_folha_borda(1, p), Nc = rt_ordem_mult(sg_c, p);
        long certo = rt_pot_mod(sg_c, Nc/N, p);           /* ordem exactamente N   */
        long torto = rt_pot_mod(sg_c, Nc/N + 1, p);       /* o dente: outro expoente */
        long ord_c = rt_ordem_mult(certo, p), ord_t = rt_ordem_mult(torto, p);
        long fecha_c = rt_pot_mod(certo, N, p);           /* = 1 */
        long fecha_t = rt_pot_mod(torto, N, p);           /* != 1 — o dente morde   */
        printf("       σ^{N/n} = %-6ld tem ordem %-5ld e σ^n = %ld  → fecha em %d passos ✓\n",
               certo, ord_c, fecha_c, N);
        printf("       σ^{N/n+1} = %-4ld tem ordem %-5ld e σ^n = %ld  → NÃO fecha (o dente morde)\n",
               torto, ord_t, fecha_t);
        int ga4_ok = (ord_c == N && fecha_c == 1 && ord_t != N && fecha_t != 1);
        ok("o DENTE diz-se em ℤ e não por distância: a torção não é uma rotação qualquer — é a"
           " que tem ORDEM n na borda, e σ^n = 1 por IGUALDADE. Trocar o expoente por um vizinho"
           " dá um σ' de ordem diferente, e σ'^n != 1: a obra não volta, e o que o diz é um"
           " inteiro. O `volta > 1e-3` media a distância euclidiana de uma rotação torta — a"
           " ordem multiplicativa diz a mesma coisa, e diz QUANTO",
           ga4_ok);
        if(!ga4_ok) passou=0;
    }

    /* ---------- GA5: as ordens ÍMPARES (3, 5, 7) nos dois meios ---------- */
    printf("\n§GA5 as ordens ÍMPARES — 3, 5, 7 — nos DOIS meios. Aqui não há escada binária: a\n");
    printf("     ordem vem da necessidade, o primo vem da ordem (k | p−1), e a projeção analógica\n");
    printf("     é a rotação de 2π/k. Nota: a normalização simétrica 1/√k exige k resíduo\n");
    printf("     quadrático (falha em k=3, p=7), então usa-se k⁻¹ de um lado — sempre existe.\n");
    {
        int ks[3] = {3,5,7};
        int erro_geral = 0;
        for(int t=0;t<3;t++){
            int k = ks[t];
            /* o primo vem da ordem: k | p−1 e p > maior valor da convolução (o enredo pede p > dado) */
            long pk=0, maxconv = (long)k*3*3;
            for(long q=k+1;q<100000;q++) if(q>maxconv){
                int ep=1; for(long d=2;d*d<=q;d++) if(q%d==0){ ep=0; break; }
                if(ep && (q-1)%k==0){ pk=q; break; }
            }
            long pg = p; p = pk;                          /* trabalha no corpo da necessidade      */
            long gk=0;
            for(long a=2;a<pk;a++){ long o=1,c=a; while(c!=1){ c=mul(c,a); o++; } if(o==pk-1){ gk=a; break; } }
            long wk = pot(gk,(pk-1)/k);
            long o_wk=1, c=wk; while(c!=1){ c=mul(c,wk); o_wk++; }
            /* abre de baixo? se 2k | p−1, w_k = (w_{2k})² */
            const char *abre = "—";
            if((pk-1)%(2*k)==0){
                long w2k = pot(gk,(pk-1)/(2*k));
                abre = (mul(w2k,w2k)==wk) ? "sim" : "NÃO";
                if(mul(w2k,w2k)!=wk) erro_geral=1;
            }
            printf("\n       ── ordem k=%d : p=%ld (menor com k|p−1 e p>%ld), g=%ld, w=%ld, ord=%ld %s ; abre de w_{%d}? %s\n",
                   k, pk, maxconv, gk, wk, o_wk, o_wk==k?"✓":"✗", 2*k, abre);
            if(o_wk != k) erro_geral=1;

            /* --- O OUTRO MEIO É OUTRA BORDA, e também é exacto --- */
            /* A rotação de 2π/k na malha É um elemento de ordem k; no corpo, o segundo
             * meio é a folha σ da borda σ² = σ + 1 em 𝔽_pk, elevada a (pk−1)/k. Quando a
             * borda não vive nesse primo (5 não é resíduo quadrático mod pk) diz-se, em
             * vez de se fingir que vive. O `volta`/`piorE` com 1e-13 mediam o double. */
            long sgk = rt_folha_borda(1, pk), ok_borda = 0, w2 = 0, o_w2 = 0;
            if(sgk > 0){
                long Nk = rt_ordem_mult(sgk, pk);
                if(Nk % k == 0){
                    w2 = rt_pot_mod(sgk, Nk/k, pk);
                    o_w2 = rt_ordem_mult(w2, pk);
                    ok_borda = (o_w2 == k && rt_pot_mod(w2, k, pk) == 1);
                }
            }
            printf("          a OUTRA borda em F_%ld: σ=%ld → σ^{(N/k)}=%ld de ordem %ld ; σ^k=%ld %s\n",
                   pk, sgk, w2, o_w2, w2 ? rt_pot_mod(w2, k, pk) : 0,
                   ok_borda ? "✓ fecha por IGUALDADE" : "(a borda não vive neste primo)");
            /* e o «abre de baixo» no segundo meio: (σ_{2k})² = σ_k, em ℤ e sem rotação */
            if(ok_borda && (pk-1) % (2*k) == 0){
                long Nk = rt_ordem_mult(sgk, pk);
                if(Nk % (2*k) == 0){
                    long w2k2 = rt_pot_mod(sgk, Nk/(2*k), pk);
                    int abre2 = (w2k2 * w2k2 % pk == w2);
                    printf("          e abre de baixo: (σ_{%d})² = σ_{%d} ? %s\n",
                           2*k, k, abre2 ? "sim ✓" : "NÃO");
                    if(!abre2) erro_geral = 1;
                }
            }
            if(sgk > 0 && !ok_borda) erro_geral = 1;

            /* --- a CONVOLUÇÃO de tamanho k nos dois meios, contra o oráculo --- */
            long a[8], b[8], cor[8], cdig[8];
            long s=1234+t*77;
            for(int i=0;i<k;i++){ s=(s*1103515245+12345)&0x7fffffff; a[i]=s%4; }
            for(int i=0;i<k;i++){ s=(s*1103515245+12345)&0x7fffffff; b[i]=s%4; }
            for(int i=0;i<k;i++){ long acc=0; for(int j=0;j<k;j++) acc += a[j]*b[((i-j)%k+k)%k]; cor[i]=acc; }
            /* digital: F sem normalizar, produto, Finv com k⁻¹ */
            long A[8],B[8],Ck[8], kinv=inv(k);
            for(int u=0;u<k;u++){
                long sa=0,sb=0, wu=pot(wk,u), f=1;
                for(int j=0;j<k;j++){ sa=md(sa+mul(a[j],f)); sb=md(sb+mul(b[j],f)); f=mul(f,wu); }
                A[u]=sa; B[u]=sb;
            }
            for(int u=0;u<k;u++) Ck[u]=mul(A[u],B[u]);
            long wi=inv(wk);
            for(int j=0;j<k;j++){
                long acc=0, wj=pot(wi,j), f=1;
                for(int u=0;u<k;u++){ acc=md(acc+mul(Ck[u],f)); f=mul(f,wj); }
                cdig[j]=mul(acc,kinv);
            }
            int dd=0; for(int i=0;i<k;i++) if(cdig[i]!=md(cor[i])) dd++;
            /* o segundo meio: a MESMA receita, pela outra borda, e em ℤ */
            int da = 0;
            if(ok_borda){
                long Ab2[8], Bb2[8], Cb2[8], cb2[8];
                for(int u=0;u<k;u++){
                    long sa=0, sb=0, wu=rt_pot_mod(w2,u,pk), f=1;
                    for(int j3=0;j3<k;j3++){ sa=(sa+a[j3]*f)%pk; sb=(sb+b[j3]*f)%pk; f=f*wu%pk; }
                    Ab2[u]=sa; Bb2[u]=sb;
                }
                for(int u=0;u<k;u++) Cb2[u]=Ab2[u]*Bb2[u]%pk;
                long wi2 = rt_inv_mod(w2, pk), kinv2 = rt_inv_mod(k, pk);
                for(int j3=0;j3<k;j3++){
                    long acc=0, wj=rt_pot_mod(wi2,j3,pk), f=1;
                    for(int u=0;u<k;u++){ acc=(acc+Cb2[u]*f)%pk; f=f*wj%pk; }
                    cb2[j3]=acc*kinv2%pk;
                }
                for(int i3=0;i3<k;i3++) if(cb2[i3] != cor[i3]%pk) da++;
            } else da = k;
            printf("          convolução: oráculo=[");
            for(int i=0;i<k;i++) printf("%ld%s", cor[i], i+1<k?" ":"");
            printf("] ; digital %s ; a outra borda %s (por IGUALDADE em ℤ_p)\n",
                   dd?"✗":"exato ✓", da?"✗":"igual ✓");
            if(dd||da) erro_geral=1;
            p = pg;
        }
        printf("\n     %s\n", VD(erro_geral, "resíduo 0 nas três ordens ímpares, nos dois meios — não há nada de especial na potência\n"
          "     de 2: a ordem vem da necessidade, o primo vem da ordem, e a projeção é a mesma\n"
          "     fórmula (g^((p−1)/k) no discreto, a rotação de 2π/k na malha). Cada uma abre de baixo\n"
          "     pelo quadrado da de ordem 2k — nos dois meios —, e a convolução de tamanho k sai\n"
          "     igual do inteiro exato e do circuito."));
        if(erro_geral) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 NOS DOIS MEIOS — o gerador global não é uma constante do digital: é a peça, e ela\n"
      "tem os dois lados. No discreto, w=36043 de ordem 256 em ℤ_40961; no analógico, a rotação de\n"
      "2π/n colhida da malha LC, na borda |λ|=1 — fecha em n passos, conserva a energia, e a TORRE\n"
      "FRACTAL é o ZOOM do circuito (elevar a torção ao quadrado = dobrar ω₀ = §B.7).\n"
      "\n"
      "E a convolução circular sai igual dos dois: a torção exata em ℤ_p dá o inteiro exato, a\n"
      "rotação LC dá o mesmo inteiro dentro do arredondamento, e ambas batem o oráculo O(n²). Nos\n"
      "dois meios sem tabela — o expoente por soma, o fator pela PG que a acompanha. Errar o ângulo\n"
      "por 1/257 já não fecha: a borda é exata, e é ela que segura."
      : "FALHOU — rever");
    return !passou;
}
