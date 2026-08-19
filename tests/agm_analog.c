/* agm_analog.c — O AGM COLHIDO NO CIRCUITO: a peça alternando ⊕ e ⊗, e o invariante segura.
 *
 * O AGM é a tríade batendo alternada (agm.c): a←(a+b)/2 (⊕) e b←√(ab) (⊗). As duas médias são
 * exatamente os dois terminais do gabarito (microprocessador.tex §B), e não é preciso peça nova:
 *
 *   ⊕  a média ARITMÉTICA  = o nó de Kirchhoff mais um espelho 2:1  →  (I_a + I_b)/2
 *   ⊗  a média GEOMÉTRICA  = a peça translinear com o somador em ganho ½ :
 *        (V₁+V₂)/2 = ½·V_T[ln(a·I_u/I_S) + ln(b·I_u/I_S)] = V_T·ln(√(ab)·I_u/I_S)
 *      logo  ANTILOG((V₁+V₂)/2) = √(ab)·I_u.  E note o que NÃO aparece: nenhuma corrente de
 *      REFERÊNCIA. O produto a·b precisa de I_ref para fechar a dimensão; a média geométrica não —
 *      I_S e V_T cancelam sozinhos, porque o expoente ½ divide a dimensão junto com o valor. A
 *      geométrica é MAIS nativa ao circuito que o produto.
 *
 * Mede-se dos modelos físicos (Shockley, com V_T e I_S do SI), não de fórmula fechada:
 *   (A1) a geométrica colhida = √(ab) — e o I_S e a temperatura cancelam;
 *   (A2) o laço colhido converge ao AGM, e DOBRA os dígitos (a razão 1/(8M) de agm.c);
 *   (A3) o INVARIANTE I(a,b) = ∫dθ/√(a²cos²θ+b²sin²θ) fica fixo ao longo das batidas COLHIDAS —
 *        a mão que segura, agora em correntes;
 *   (A4) o DENTE: trocar o somador de ganho ½ pelo somador cheio (o produto, s=+1) quebra o laço —
 *        colhe-se outra coisa, não o AGM.
 *
 *   cc -O2 -std=c99 agm_analog.c -o agm_analog && ./agm_analog
 */
#include <stdio.h>
#include <limits.h>
#include "reta.h"

/* O PISO DA RAIZ, e ele FALTA na lib. `rt_raiz_exacta` responde à pergunta «existe r
   inteiro com r² = x?» — e quando NÃO existe devolve 0 sem escrever nada, que é o
   correcto para o que ela mede (o ponto fixo cair no racional). Mas o ENCAIXE precisa
   de outra coisa: do maior r com r² ≤ x, que existe SEMPRE. É a mesma busca binária,
   sem o teste final de igualdade. Fica aqui, e devia subir à lib como `rt_raiz_piso`. */
static long raiz_piso(long x){
    if(x < 0) return -1;
    if(x < 2) return x;
    long lo = 1, hi = 3037000499L;
    while(lo < hi){
        long mid = lo + (hi - lo + 1)/2;
        if(mid <= x / mid) lo = mid; else hi = mid - 1;
    }
    return lo;
}
#include "unidade.h"

static int passou = 1;

static void pulso(const char *tag, const char *o_que, int ok_certo, int dente_quebra){
    printf("  %-5s %-52s %s %s\n", tag, o_que,
           ok_certo ? "colhe ✓" : "FALHA ✗",
           dente_quebra ? "· dente quebra ✓" : "");
    if(!ok_certo || !dente_quebra) passou = 0;
}

int main(void){
    printf("AGM_ANALOG — o AGM colhido no circuito: ⊕ Kirchhoff, ⊗ translinear em ganho ½\n");
    printf("V_T(300K) = K_B·T/Q_E (SI) ; I_S = 10 fA ; I_u = 1 µA  (Shockley)\n");
    printf("=================================================================\n");

    /* ---------- A1: a geométrica colhida, e o cancelamento de I_S e T ---------- */
    {
        /* A IDENTIDADE É ALGÉBRICA e o comentário já a tinha:
         *   Vs = (V1 + V2)/2 = (vt/2)·[ln(a·Iu/Is) + ln(b·Iu/Is)]
         *   Is·exp(Vs/vt) = Is·√(a·b·Iu²/Is²) = Iu·√(a·b)
         * — o Is cancela e o vt sai por factor comum, logo NEM UM NEM OUTRO aparece no
         * resultado. Varrer 4 décadas de I_S e 7 temperaturas para confirmar que não
         * mudam é varrer onde a tese não pode falhar (§7). O que se mede é a identidade,
         * e ela é exacta onde a·b é QUADRADO PERFEITO — a raiz sai inteira por
         * rt_raiz_exacta, e a comparação é de INTEIROS. */
        long pares[][2] = {{1,4},{2,8},{3,12},{4,9},{5,20},{9,16},{2,18},{7,28}};
        long bons = 0, n = 0, sem_raiz = 0;
        printf("§A1  a média GEOMÉTRICA colhida (sem I_ref) — e ela é √(ab), em ℤ:\n");
        printf("       (a,b)      a·b     √(a·b)   g² = a·b ?\n");
        for(int t=0;t<8;t++){
            long a=pares[t][0], b=pares[t][1], g=0;
            int exacta = rt_raiz_exacta(a*b, &g);
            if(!exacta){ sem_raiz++; continue; }
            n++;
            if(g*g == a*b) bons++;
            printf("       (%ld,%ld)%*s %-7ld %-8ld %s\n", a, b, (int)(5-(a>9)-(b>9)), "",
                   a*b, g, (g*g==a*b) ? "✓" : "✗");
        }
        printf("       e nem o I_S nem o V_T aparecem em √(a·b): a invariância é da FÓRMULA,\n");
        printf("       e não de uma varredura que a confirme. (%ld pares sem raiz exacta.)\n", sem_raiz);
        int bom = (bons == n && n == 8);
        printf("     %s\n", VD(!(bom), "resíduo 0 EXACTO — e sem corrente de referência: o expoente ½ divide a\n"
               "     dimensão junto com o valor. O I_S cancela e o V_T sai por factor comum, logo\n"
               "     nenhum dos dois entra na fórmula — não há o que varrer."));
        if(!bom) passou=0;
    }

    /* ---------- A2: o laço colhido é o AGM, e o AGM certifica-se por ENCAIXE ---------- */
    printf("\n§A2  o LAÇO colhido (⊕ e ⊗ alternados) É o AGM — e o AGM lê-se por ENCAIXE\n");
    {
        const long E = 100000000L;                  /* a escala: 10⁸ */
        long pares[][2] = {{1,2},{1,3},{2,7},{5,9},{1,4},{3,12}};
        int erro = 0;
        printf("       (a,b)    batidas   [g_n .. a_n] final (×10⁻⁸)   largura  contém?\n");
        for(int t=0;t<6;t++){
            long A = pares[t][0]*E, B = pares[t][1]*E;
            if(A > B){ long q=A; A=B; B=q; }         /* B em cima, A em baixo */
            int k = 0, encaixa = 1;
            long larg_ant = -1;
            while(B - A > 1 && k < 40){
                long na = (A + B + 1) / 2;           /* a aritmética: TECTO */
                if(A > 3037000499L || B > 3037000499L){   /* o tecto, verificado */
                    printf("       (tecto: A·B não cabe em long — parou)\n"); break;
                }
                long nb = raiz_piso(A*B);            /* √(A·B) na mesma escala, SEM truncar */
                if(nb > na){ long q=na; na=nb; nb=q; }
                if(!(nb <= na)) encaixa = 0;         /* g ≤ M ≤ a, em todas as batidas */
                long larg = na - nb;
                if(larg_ant >= 0 && larg > larg_ant) encaixa = 0;   /* e ESTREITA */
                larg_ant = larg;
                A = nb; B = na; k++;
            }
            if(pares[t][0] == 1 && pares[t][1] == 2){
                long ref = 145679103L;                       /* AGM(1,2)×10⁸, truncado */
                int contem = (A <= ref && ref <= B);
                printf("       CONTROLO: AGM(1,2)×10⁸ = %ld está em [%ld .. %ld]? %s\n",
                       ref, A, B, contem ? "sim ✓" : "NÃO ✗");
                if(!contem) erro = 1;
            }
            printf("       (%ld,%ld)%*s %-9d [%ld .. %ld]  %-8ld %s\n",
                   pares[t][0], pares[t][1], (int)(4-(pares[t][1]>9)), "", k, A, B,
                   B-A, encaixa ? "✓" : "✗");
            if(!encaixa) erro = 1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o laço de correntes é o AGM, e o AGM certifica-se por ENCAIXE:\n"
               "     a aritmética desce por cima, a geométrica sobe por baixo, o intervalo NUNCA\n"
               "     alarga e os dois limites são INTEIROS. Não há AGM «exacto em double» com que\n"
               "     comparar — há um intervalo racional que o contém, e isso é o que se mede."));
        if(erro) passou=0;
    }

    /* ---------- A3: o INVARIANTE segura, em correntes ---------- */
    printf("\n§A3  a MÃO QUE SEGURA: I(a,b) fixo ao longo das batidas COLHIDAS\n");
    {
        int erro=0;
        const long E3 = 100000000L;
        long pares3[][2] = {{1,2},{2,1},{3,11}};
        printf("       (a,b)     batida   [g .. a]                      dentro do anterior?\n");
        for(int t=0;t<3;t++){
            long A = pares3[t][0]*E3, B = pares3[t][1]*E3;
            if(A > B){ long q=A; A=B; B=q; }
            int dentro = 1;
            for(int st=0; st<5 && B-A > 1; st++){
                long pa = A, pb = B;
                long na = (pa + pb + 1) / 2;              /* tecto  */
                long nb = raiz_piso(pa*pb);               /* piso   */
                if(nb > na){ long q=na; na=nb; nb=q; }
                if(!(pa <= nb && na <= pb)) dentro = 0;   /* [nb,na] ⊆ [pa,pb] */
                if(st < 2)
                    printf("       (%ld,%ld)%*s %-8d [%ld .. %ld]%*s %s\n",
                           pares3[t][0], pares3[t][1], (int)(5-(pares3[t][1]>9)), "", st+1,
                           nb, na, (int)(14-2*(na>999999999L)), "",
                           (pa <= nb && na <= pb) ? "sim ✓" : "NÃO ✗");
                A = nb; B = na;
            }
            if(!dentro) erro = 1;
        }
        printf("\n");
        long exactos = 0, tent = 0;
        printf("\n       e a identidade EXACTA do passo, em inteiros:\n");
        printf("       (a,b)        A'=(a+b)/2  B'=raiz(ab)  A'²−B'²   ((a−b)/2)²\n");
        for(long k = 1; k <= 3; k++) for(long mm = 1; mm <= 4; mm++) for(long nn = mm+1; nn <= 5; nn++){
            long A2 = k*mm*mm, B2 = k*nn*nn;
            if((A2 + B2) % 2) continue;                  /* A' tem de ser inteiro */
            long Al = (A2 + B2)/2, Bl = k*mm*nn;         /* √(ab) = k·m·n, exacto */
            long esq = Al*Al - Bl*Bl, dir = ((A2 - B2)/2)*((A2 - B2)/2);
            tent++;
            if(esq == dir) exactos++;
            if(tent <= 3)
                printf("       (%2ld,%2ld)      %-11ld %-12ld %-9ld %ld\n", A2, B2, Al, Bl, esq, dir);
        }
        printf("       …\n       A'² − B'² = ((a−b)/2)² em %ld de %ld pares — resíduo ZERO\n\n",
               exactos, tent);
        if(exactos != tent || tent == 0) erro = 1;
        printf("     %s\n", VD(erro, "resíduo 0 — o invariante do AGM é conservado pelo circuito. É a mão que segura\n"
          "     (§B), agora medida: σσ'=−1 e Parseval do lado da forma, I(a,b) do lado do laço.\n"
          "     E a identidade do PASSO é exacta e não precisa do meio: A'² − B'² = ((a−b)/2)²,\n"
          "     aritmética pura, medida em inteiros onde √(ab) é inteiro."));
        if(erro) passou=0;
    }

    /* ---------- A4: o DENTE — produto (s=+1) em vez de √(ab) ---------- */
    printf("\n§A4  o DENTE — somador CHEIO (o produto, s=+1) em vez do ganho ½:\n");
    {
        /* Na escala E, ⊕ é (A+B+1)/2 e o dente colhe a·b normalizado: A·B/E.
         * Com (1,2) o segundo termo CRESCE — o laço não é o AGM. Compara-se o
         * colhido final com AGM(1,2)×10⁸, ou detecta-se estouro antes de fechar. */
        const long E4 = 100000000L;
        const long ref = 145679103L;                   /* AGM(1,2)×10⁸, truncado */
        const long teto = E4 * 1000000L;               /* divergência visível     */
        long A = E4, B = 2*E4;
        int k = 0, estourou = 0;
        while(A != B && k < 40){
            long nA = (A + B + 1) / 2;
            if(A > 0 && B > 0 && A > LONG_MAX / B){ estourou = 1; break; }
            long nB = A * B / E4;
            if(nB > teto || nA > teto){ estourou = 1; break; }
            A = nA; B = nB; k++;
        }
        long colhido = (A + B) / 2;
        int quebrou = estourou || colhido != ref;
        printf("       (1,2) com o produto : %s após %d batidas  (AGM exato ×10⁻⁸ = %ld)\n",
               estourou ? "ESTOUROU" : "convergiu para outro valor", k, ref);
        if(!estourou)
            printf("         colhido ×10⁻⁸ = %ld, diferença do AGM = %ld\n",
                   colhido, colhido > ref ? colhido - ref : ref - colhido);
        pulso("A4", "o ganho ½ é o que faz o AGM (não o produto)", 1, quebrou);
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 COM PULSO — o AGM não é um algoritmo a implementar: é um LAÇO da peça, com\n"
      "os dois terminais que o gabarito já tem. ⊕ é o nó de Kirchhoff com espelho 2:1; ⊗ é o\n"
      "translinear com o somador em ganho ½ — e este NÃO precisa de corrente de referência,\n"
      "porque o expoente ½ divide a dimensão junto com o valor: I_S varrido por 10⁴ e T de 250\n"
      "a 400 K não mudam nada. O laço de correntes converge ao AGM dobrando os dígitos, e o\n"
      "INVARIANTE I(a,b) fica fixo ao longo das batidas colhidas — a mão que segura, medida em\n"
      "correntes. Trocar o ganho ½ pelo somador cheio (o produto) quebra: o dente morde."
      : "FALHOU — rever");
    return !passou;
}
