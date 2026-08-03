/* ortogonal.c — GRAM É O DISCRIMINANTE, E O DIRAC É A BASE QUE JÁ VEM PRONTA.
 *
 * O Aarão: "agora funções de ortogonalização e transformada via Dirac."
 *
 * O transformada.c JÁ mede duas metades disto, e não se repetem aqui:
 *   §U1  Σ_k χ_k(j)χ_k(j') = n·δ   — os caracteres são ORTOGONAIS
 *   §U5  o delta espalha-se em tudo, e a volta concentra no ponto
 *
 * O que falta é a CONSTRUÇÃO — o que se faz quando a base não vem ortogonal — e a
 * quantidade que a mede. E ela sai no objeto do projeto:
 *
 *     G_ij = ⟨σ^i, σ^j⟩ = Tr(σ^{i+j}) = t_{i+j}     ← a matriz de Gram é feita de TRAÇOS
 *     det G(1, σ) = 2·t_2 − t_1² = m² + 4 = Δ       ← e o determinante É o DISCRIMINANTE
 *
 * O mesmo Δ que dá σ = (m+√Δ)/2. A independência da base de potências, medida pelo Gram,
 * e o discriminante do corpo são O MESMO NÚMERO — e é inteiro.
 *
 * E DAÍ O PAR, que é o assunto:
 *
 *   A BASE DE POTÊNCIAS {1, σ, σ², …}   NÃO é ortogonal. Gram-Schmidt CUSTA, e o que
 *                                       sobra do custo é Δ.
 *   A BASE DE CARACTERES {χ_k}          JÁ é ortogonal. Gram = n·I, e não há nada a fazer.
 *
 * Uma mede (as coordenadas do corpo, onde se opera) e a outra ordena (as frequências, onde
 * se separa). E a TRANSFORMADA é exatamente a mudança de uma para a outra: não constrói
 * ortogonalidade nenhuma — leva à base que já a tem.
 *
 * O DIRAC é o que torna isso visível: δ é o mais concentrado que há, e a sua transformada é
 * a mais espalhada. Na base de potências não há elemento com essa propriedade.
 *
 * E O GRAM É O WRONSKIANO NOUTRA ROUPA: os dois são determinantes que medem independência —
 * um de VETORES (⟨v_i,v_j⟩), outro de FUNÇÕES (f_i^{(j)}) — e os dois degeneram exatamente
 * onde a independência se perde. O lambert.c §Y8 mediu o segundo; este mede o primeiro.
 *
 *   §O1  a matriz de Gram das potências é feita de TRAÇOS: G_ij = t_{i+j}
 *   §O2  det G(1,σ) = Δ = m²+4 — o Gram É o discriminante, em inteiros
 *   §O3  Gram-Schmidt sobre {1,σ}, exato em Q — e o que ele custa
 *   §O4  os caracteres são ortogonais DE GRAÇA: Gram = n·I, sem construção
 *   §O5  Gram ↔ Wronskiano: dois determinantes, a mesma pergunta
 *   §O6  controlo negativo: com a base dependente, det G = 0 EXATO
 *
 *   cc -O2 -std=c99 -Wall ortogonal.c -lm -o ortogonal && ./ortogonal
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>

typedef long long L;

/* racional exato */
typedef struct { L p, q; } Q;
static L mdc(L a, L b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ L t=a%b; a=b; b=t; } return a?a:1; }
static Q qr(L p, L q){ if(q<0){p=-p;q=-q;} L g=mdc(p,q); Q r={p/g,q/g}; return r; }
static Q qsub(Q a, Q b){ return qr(a.p*b.q - b.p*a.q, a.q*b.q); }
static Q qmul(Q a, Q b){ return qr(a.p*b.p, a.q*b.q); }
static Q qdiv(Q a, Q b){ return qr(a.p*b.q, a.q*b.p); }
static double qv(Q a){ return (double)a.p/(double)a.q; }

int main(void){
    printf("================================================================\n");
    printf("  Gram é o discriminante — e o Dirac é a base que já vem pronta\n");
    printf("================================================================\n");

    /* ---------------- §O1 — o Gram é feito de traços ---------------- */
    printf("\n§O1 a matriz de Gram das potências é feita de TRAÇOS: G_ij = Tr(σ^{i+j}) = t_{i+j}\n");
    {
        /* ⟨x,y⟩ = Tr(x·y) para x,y no corpo. Com a base {1, σ}:
         *   G_00 = Tr(1)  = t_0 = 2
         *   G_01 = Tr(σ)  = t_1 = m
         *   G_11 = Tr(σ²) = t_2 = m² + 2                                     */
        int metais=0, bate=0;
        printf("      m    t_0  t_1  t_2      G = [[t_0,t_1],[t_1,t_2]]\n");
        for(L m=1; m<=8; m++){
            L t[6]; t[0]=2; t[1]=m;
            for(int k=2;k<6;k++) t[k] = m*t[k-1] + t[k-2];
            metais++;
            /* t_2 tem de ser m²+2, e é o traço de σ² */
            if(t[2] == m*m + 2) bate++;
            if(m<=4) printf("      %-4lld %-4lld %-4lld %-8lld [[%lld,%lld],[%lld,%lld]]\n",
                            m, t[0], t[1], t[2], t[0],t[1],t[1],t[2]);
        }
        printf("      metais: %d   com t_2 = m²+2: %d\n", metais, bate);
        ok("as entradas do Gram são os traços t_k — inteiros do corpo", bate==metais);
        conclui("a matriz que mede a independência da base é feita dos mesmos traços que dão");
        conclui("a zeta dinâmica. Não é coincidência de notação: é o mesmo t_k.");
    }

    /* ---------------- §O2 — det Gram = o discriminante ---------------- */
    printf("\n§O2 det G(1,σ) = Δ = m²+4 — o Gram É o discriminante, em inteiros\n");
    {
        int metais=0, igual=0;
        printf("      m    det G = 2·t_2 − t_1²    Δ = m²+4    σ = (m+√Δ)/2\n");
        for(L m=1; m<=8; m++){
            L t[6]; t[0]=2; t[1]=m;
            for(int k=2;k<6;k++) t[k] = m*t[k-1] + t[k-2];
            L det = t[0]*t[2] - t[1]*t[1];
            L D = m*m + 4;
            metais++;
            if(det == D) igual++;
            if(m<=4) printf("      %-4lld %-22lld %-11lld %.12f\n",
                            m, det, D, (m+sqrt((double)D))/2.0);
        }
        printf("      metais: %d   com det G = Δ: %d\n", metais, igual);
        ok("det Gram(1,σ) = Δ = m²+4 — EXATO, e é o mesmo Δ de σ = (m+√Δ)/2",
           igual==metais);
        conclui("a independência da base e o discriminante do corpo são O MESMO NÚMERO. Quem");
        conclui("mede quanto a base é 'aberta' é quem mede quanto o corpo é ramificado.");
    }

    /* ---------------- §O3 — Gram-Schmidt, e o que ele custa ---------------- */
    printf("\n§O3 Gram-Schmidt sobre {1, σ}: exato em Q, e o que ele custa\n");
    {
        /* v_0 = 1;  v_1 = σ − (⟨σ,1⟩/⟨1,1⟩)·1 = σ − (t_1/t_0)·1 = σ − m/2.
         * E ⟨v_1,v_1⟩ = t_2 − t_1²/t_0 = (t_0 t_2 − t_1²)/t_0 = Δ/2.
         * Logo o produto das normas é t_0 · Δ/2 = Δ — o det do Gram. */
        int metais=0, coef_ok=0, norma_ok=0, prod_ok=0;
        printf("      m    coef = t_1/t_0    ‖v_1‖² = Δ/t_0    ‖v_0‖²·‖v_1‖²    det G\n");
        for(L m=1; m<=6; m++){
            L t[6]; t[0]=2; t[1]=m;
            for(int k=2;k<6;k++) t[k] = m*t[k-1] + t[k-2];
            L D = m*m + 4;
            Q coef = qr(t[1], t[0]);                    /* m/2 */
            Q n0   = qr(t[0], 1);                       /* ‖v_0‖² = 2 */
            /* ‖v_1‖² = t_2 − coef²·t_0 ... = Δ/t_0 */
            Q c2 = qmul(coef, coef);
            Q n1 = qsub(qr(t[2],1), qmul(c2, qr(t[0],1)));
            Q alvo = qr(D, t[0]);
            Q prod = qmul(n0, n1);
            metais++;
            /* A 1.ª versão testava coef.p==m && coef.q==2 — isto é a FORMA, não o valor.
             * Para m par, qr(m,2) reduz a (m/2)/1 e a asserção falhava sem que nada
             * estivesse errado. Compara-se o VALOR, por produto cruzado em inteiros. */
            if(coef.p * 2 == m * coef.q) coef_ok++;
            if(n1.p == alvo.p && n1.q == alvo.q) norma_ok++;
            if(prod.p == D && prod.q == 1) prod_ok++;
            if(m<=4) printf("      %-4lld %lld/%-14lld %lld/%-15lld %lld/%-14lld %lld\n",
                            m, coef.p, coef.q, n1.p, n1.q, prod.p, prod.q, D);
        }
        printf("      metais: %d   coef = m/2: %d   ‖v_1‖² = Δ/2: %d   produto = Δ: %d\n",
               metais, coef_ok, norma_ok, prod_ok);
        ok("o coeficiente de Gram-Schmidt é t_1/t_0 = m/2 — exato em Q", coef_ok==metais);
        ok("e ‖v_1‖² = Δ/t_0 = Δ/2, também exato", norma_ok==metais);
        ok("e o PRODUTO das normas é Δ = det G — o custo da ortogonalização É o discriminante",
           prod_ok==metais);
        conclui("Gram-Schmidt tira o que a base tinha de oblíquo, e o que sobra multiplicado");
        conclui("é exatamente o determinante de Gram. O custo é o Δ, e não se pode reduzir.");
    }

    /* ---------------- §O4 — os caracteres vêm prontos ---------------- */
    printf("\n§O4 os caracteres são ortogonais DE GRAÇA: Gram = n·I, sem construção\n");
    {
        /* Base de caracteres de Z/n: χ_k(j) = ω^{jk} com ω = e^{2πi/n}.
         * ⟨χ_k, χ_l⟩ = Σ_j ω^{j(k−l)} = n·δ_{kl}. Aqui em REAIS, com n par, usando
         * as raízes reais ±1 do subgrupo — ou simplesmente medindo a soma de Σ cos.  */
        int ns=0, diag_ok=0, fora_ok=0;
        printf("      n    ⟨χ_k,χ_k⟩ = n    max |⟨χ_k,χ_l⟩| com k≠l\n");
        for(int n=2; n<=12; n+=2){
            double pior_fora = 0; int diag_bom = 1;
            for(int k=0;k<n;k++) for(int l=0;l<n;l++){
                double re=0, im=0;
                for(int j=0;j<n;j++){
                    double th = 2.0*3.14159265358979323846*j*(k-l)/n;
                    re += cos(th); im += sin(th);
                }
                double mod = sqrt(re*re+im*im);
                if(k==l){ if(fabs(mod - n) > 1e-9) diag_bom = 0; }
                else if(mod > pior_fora) pior_fora = mod;
            }
            ns++;
            if(diag_bom) diag_ok++;
            if(pior_fora < 1e-9) fora_ok++;
            printf("      %-4d %-16d %.2e\n", n, n, pior_fora);
        }
        printf("      n testados: %d   diagonal = n: %d   fora da diagonal = 0: %d\n",
               ns, diag_ok, fora_ok);
        ok("a diagonal do Gram dos caracteres é n — sem construção nenhuma", diag_ok==ns);
        ok("e fora da diagonal é ZERO: a base já vem ortogonal", fora_ok==ns);
        conclui("é o par: a base de POTÊNCIAS é oblíqua e custa Δ a endireitar; a de");
        conclui("CARACTERES já vem pronta. Uma mede (as coordenadas do corpo), a outra ordena");
        conclui("(as frequências) — e a TRANSFORMADA é a mudança de uma para a outra.");
        conclui("ela não constrói ortogonalidade: leva à base que já a tem.");
    }

    /* ---------------- §O5 — Gram e Wronskiano ---------------- */
    printf("\n§O5 Gram ↔ Wronskiano: dois determinantes, a mesma pergunta\n");
    {
        /* Gram: det(⟨v_i,v_j⟩) — independência de VETORES.
         * Wronskiano: det(f_i^{(j)}) — independência de FUNÇÕES.
         * Ambos degeneram exatamente onde a independência se perde. Mede-se o Gram com
         * vetores que se aproximam da dependência, e vê-se o determinante a ir a zero. */
        printf("      ângulo   v_1 = (1,0)   v_2 = (cos θ, sin θ)   det G = sin²θ\n");
        int ts=0, lei_ok=0;
        for(double th=1.2; th>0.001; th/=4.0){
            double g11=1.0, g12=cos(th), g22=1.0;
            double det = g11*g22 - g12*g12;
            ts++;
            if(fabs(det - sin(th)*sin(th)) < 1e-12) lei_ok++;
            printf("      %.5f  (1,0)         (%.5f, %.5f)      %.12e\n",
                   th, cos(th), sin(th), det);
        }
        printf("      ângulos: %d   com det G = sin²θ: %d\n", ts, lei_ok);
        ok("det Gram = sin²θ: vai a ZERO quando os vetores se alinham", lei_ok==ts);
        conclui("o Wronskiano do lambert.c §Y8 faz o mesmo para FUNÇÕES, e lá EXPLODE em vez");
        conclui("de anular — porque as derivadas divergem mais depressa do que as funções se");
        conclui("aproximam. Mesma pergunta, respostas opostas: é isso que os torna o par.");
    }

    /* ---------------- §O6 — o controlo negativo ---------------- */
    printf("\n§O6 controlo negativo: com a base DEPENDENTE, det G = 0 EXATO\n");
    {
        /* Se v_2 = λ v_1, o Gram é singular e o determinante é zero — em inteiros, sem
         * arredondamento. É isto que impede o texto de dizer que o Gram é sempre positivo. */
        int casos=0, zeros=0;
        printf("      v_1        v_2 = λ·v_1     det G\n");
        for(L lam=1; lam<=5; lam++){
            L a=3, b=7;                        /* v_1 = (3,7) */
            L c=lam*a, d=lam*b;                /* v_2 = λ v_1 */
            L g11=a*a+b*b, g12=a*c+b*d, g22=c*c+d*d;
            L det = g11*g22 - g12*g12;
            casos++;
            if(det == 0) zeros++;
            if(lam<=3) printf("      (%lld,%lld)      (%lld,%lld)          %lld\n", a,b,c,d,det);
        }
        printf("      casos: %d   com det G = 0 exato: %d\n", casos, zeros);
        ok("com a base dependente o Gram anula-se — em INTEIROS, sem arredondamento",
           zeros==casos);
        /* e o contraste: com a base do corpo, nunca é zero, porque Δ = m²+4 > 0 */
        int nunca_zero=0;
        for(L m=1;m<=8;m++) if(m*m+4 > 0) nunca_zero++;
        printf("      e com a base do corpo: Δ = m²+4 > 0 em %d de 8 metais\n", nunca_zero);
        ok("a base de potências NUNCA degenera: Δ = m²+4 >= 5 sempre", nunca_zero==8);
        conclui("o Gram distingue as duas situações com um número: zero na dependência, Δ na");
        conclui("base do corpo. E Δ >= 5 diz que ela nunca chega perto de degenerar.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
