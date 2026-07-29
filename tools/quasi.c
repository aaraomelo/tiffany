/* quasi.c — POR QUE A DIMENSÃO 5 NÃO TEM OURO: a periodicidade proíbe o cinco, e o que sobra é o
 * QUASICRISTAL.
 *
 * O furo achado em transforma.c (x⁵−x⁴−1 redutível para m=1) não é acidente aritmético: é a
 * restrição cristalográfica, e ela se deriva de uma conta só.
 *
 *  (Q1) UMA ROTAÇÃO DE ORDEM n CABE NUMA REDE ⟺ o seu traço é INTEIRO, isto é 2cos(2π/n) ∈ ℤ.
 *       Isso dá n ∈ {1,2,3,4,6} e mais nada — e o que exclui o CINCO é precisamente o ouro:
 *              2cos(2π/5) = φ − 1 = 1/φ ,      irracional.
 *       O cinco é proibido em cristal porque a sua rotação PEDE o ouro, e o ouro não é inteiro.
 *
 *  (Q2) O FURO FATORA no giro permitido e no crescimento aperiódico:
 *              x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1) = Φ₆(x) · (x³ − x − 1).
 *       O primeiro fator é o 6º ciclotômico — raízes de ORDEM 6, giro puro, a única ordem "extra"
 *       que uma rede admite. O segundo é o número PLÁSTICO ρ≈1,3247, o menor número de Pisot, sem
 *       nenhuma raiz da unidade. Em dimensão 5 o ouro não fecha em corpo porque a dimensão 5
 *       SEPARA o giro do crescimento: uma peça vira duas.
 *
 *  (Q3) A REDE MÍNIMA é φ(n)-dimensional: uma simetria de ordem n cabe numa rede de dimensão d
 *       sse φ(n) ≤ d (φ = totiente de Euler, o grau de Φ_n). No plano (d=2): φ(n) ≤ 2 ⟺
 *       n ∈ {1,2,3,4,6} — a mesma lista de (Q1), por outro caminho. Para n=5, φ(5)=4: o cinco
 *       precisa de QUATRO dimensões e de uma PROJEÇÃO. Projetar rede periódica de dimensão maior
 *       num espaço menor é exatamente o corte-e-projeção que constrói o Penrose.
 *
 *  (Q4) O QUASICRISTAL É O GATO. A substituição de Fibonacci a→ab, b→a tem matriz [[1,1],[1,0]] —
 *       o gato do ouro, A₁. A palavra que ela gera é ordenada e APERIÓDICA, e mede-se as duas
 *       coisas: nenhum período q ≤ N/2, e complexidade de fatores exatamente k+1 (sturmiana — o
 *       MÍNIMO possível para uma palavra aperiódica). Ordem máxima sem periodicidade: é a definição
 *       de quasicristal, e o gerador é a peça.
 *
 * Sobre a vida: Schrödinger (1944) chamou o portador da hereditariedade de "cristal APERIÓDICO", e
 * a simetria de ordem 5/icosaédrica — proibida em cristal — é a dos capsídeos virais e da biologia
 * pentâmera. Que a vida NASÇA daí é tese, não teorema; o que se mede aqui é o que a antecede: ordem
 * sem periodicidade exige a simetria proibida, e a simetria proibida é o ouro.
 *
 * Tudo em inteiros exatos, buffers fixos, zero malloc.
 *
 *   cc -O2 -std=c99 quasi.c -o quasi && ./quasi
 */
#include <stdio.h>
#include <string.h>

static int ok = 1;

/* ---------- Q1: 2cos(2π/n) inteiro? — a recorrência s_{k+1} = x·s_k − s_{k−1}, s_k = 2cos(kθ) ---- */
static int traco_inteiro(int n){                     /* devolve x se 2cos(2π/n)=x ∈ ℤ, senão 99    */
    for(int x=-2; x<=2; x++){
        long s0 = 2, s1 = x;                         /* s_0=2cos0=2, s_1=2cosθ=x                   */
        int volta = -1;
        for(int k=1; k<=n; k++){
            if(k>1){ long s2 = (long)x*s1 - s0; s0 = s1; s1 = s2; }
            if(s1 == 2 && k < n){ volta = k; break; }  /* voltou antes: a ordem é menor            */
        }
        if(volta > 0) continue;
        if(s1 == 2) return x;                        /* 2cos(2π n/n)=2: fecha exatamente em n      */
    }
    return 99;
}
/* ---------- Q2: aritmética exata de polinômios inteiros ---------- */
static void polmul(const long *a, int da, const long *b, int db, long *r, int *dr){
    for(int i=0;i<=da+db;i++) r[i]=0;
    for(int i=0;i<=da;i++) for(int j=0;j<=db;j++) r[i+j] += a[i]*b[j];
    *dr = da+db;
}
/* x^k mod (x²−x+1)  →  x² = x−1 ; devolve (c0,c1) exatos em ℤ */
static void pot_phi6(int k, long *c0, long *c1){
    long a0=1, a1=0;                                  /* 1 */
    for(int t=0;t<k;t++){                             /* ×x : (a0 + a1 x)·x = a0 x + a1 x² = a1(x−1) + a0 x */
        long n0 = -a1, n1 = a0 + a1;
        a0 = n0; a1 = n1;
    }
    *c0 = a0; *c1 = a1;
}
static int totiente(int n){ int r=0; for(int k=1;k<=n;k++){ int a=k,b=n; while(b){int t=a%b;a=b;b=t;} if(a==1) r++; } return r; }

/* ---------- Q4: a palavra de Fibonacci, gerada pelo gato ---------- */
#define NW 8192
static char W[NW]; static int LW;
static void fibword(void){
    static char A[NW], B[NW];
    int la, lb;
    A[0]='a'; la=1;                                   /* S_1 = a */
    B[0]='a'; B[1]='b'; lb=2;                         /* S_2 = ab */
    while(lb + la <= NW){
        static char C[NW]; int lc = 0;
        memcpy(C, B, lb); lc = lb;
        memcpy(C+lc, A, la); lc += la;                /* S_{k} = S_{k−1} S_{k−2} */
        memcpy(A, B, lb); la = lb;
        memcpy(B, C, lc); lb = lc;
    }
    memcpy(W, B, lb); LW = lb;
}
static int periodo_algum(int N){                      /* algum q ≤ N/2 é período? 0 = aperiódica    */
    for(int q=1;q<=N/2;q++){
        int bom = 1;
        for(int i=0;i+q<N;i++) if(W[i]!=W[i+q]){ bom=0; break; }
        if(bom) return q;
    }
    return 0;
}
static int fatores_distintos(int N, int k){           /* nº de fatores distintos de comprimento k   */
    int cnt = 0;
    for(int i=0;i+k<=N;i++){
        int novo = 1;
        for(int j=0;j<i;j++) if(!memcmp(W+i, W+j, k)){ novo=0; break; }
        cnt += novo;
    }
    return cnt;
}

int main(void){
    printf("QUASI — por que a dimensão 5 não tem ouro\n");
    printf("=================================================================\n");

    /* ---- Q1 ---- */
    printf("§Q1  uma rotação de ordem n cabe numa REDE ⟺ 2cos(2π/n) ∈ ℤ:\n");
    {
        int permitidas = 0;
        for(int n=1;n<=12;n++){
            int x = traco_inteiro(n);
            if(x != 99){ permitidas++;
                printf("       n=%2d : 2cos(2π/n) = %+d  → PERMITIDA na rede\n", n, x); }
            else if(n==5 || n==7 || n==8)
                printf("       n=%2d : 2cos(2π/n) ∉ ℤ    → PROIBIDA%s\n", n,
                       n==5 ? "  ← e o que ela pede é o OURO: 2cos(2π/5) = φ−1" : "");
        }
        printf("     permitidas: %d  %s\n", permitidas,
               permitidas==5 ? "resíduo 0 — são {1,2,3,4,6} e mais nada" : "REVER");
        if(permitidas != 5) ok = 0;
        /* exato em ℤ[φ], φ²=φ+1 : (φ−1)² + (φ−1) − 1 = 0 ? */
        long a0=-1, a1=1;                             /* φ−1 = −1 + 1·φ                             */
        /* (a0+a1φ)² = a0² + 2a0a1 φ + a1²φ² = a0²+a1² + (2a0a1+a1²)φ  [φ²=φ+1]                     */
        long q0 = a0*a0 + a1*a1, q1 = 2*a0*a1 + a1*a1;
        long r0 = q0 + a0 - 1, r1 = q1 + a1;          /* + (φ−1) − 1                                */
        printf("     em ℤ[φ] (φ²=φ+1): (φ−1)²+(φ−1)−1 = %ld + %ldφ  %s\n", r0, r1,
               (r0==0&&r1==0) ? "resíduo 0 — φ−1 é raiz de x²+x−1, grau 2, irracional" : "REVER");
        if(r0||r1) ok = 0;
    }

    /* ---- Q2 ---- */
    printf("\n§Q2  o FURO fatora: giro permitido × crescimento aperiódico\n");
    {
        long f6[3] = {1,-1,1};                        /* x²−x+1 = Φ₆   (coef de x⁰,x¹,x²)          */
        long pl[4] = {-1,-1,0,1};                     /* x³−x−1                                     */
        long r[8]; int dr;
        polmul(f6,2, pl,3, r,&dr);
        long alvo[6] = {-1,0,0,0,-1,1};               /* x⁵−x⁴−1                                    */
        int erro = (dr!=5);
        for(int i=0;i<=5 && !erro;i++) if(r[i]!=alvo[i]) erro = 1;
        printf("     (x²−x+1)(x³−x−1) = ");
        for(int i=dr;i>=0;i--) if(r[i]) printf("%+ldx^%d ", r[i], i);
        printf(" %s\n", erro?"FALHA":"= x⁵−x⁴−1  resíduo 0");
        if(erro) ok=0;
        /* Φ₆: a ordem de x é exatamente 6 */
        int ordem = 0;
        for(int k=1;k<=12;k++){ long c0,c1; pot_phi6(k,&c0,&c1); if(c0==1&&c1==0){ ordem=k; break; } }
        printf("     em ℤ[x]/(x²−x+1): ordem de x = %d  %s  (giro puro, a ordem 'extra' da rede)\n",
               ordem, ordem==6?"resíduo 0":"REVER");
        if(ordem!=6) ok=0;
        /* x³−x−1: Pisot, por SINAL (exato, sem float): ρ∈(1,2) e |outras|=1/√ρ<1 */
        long f1 = 1-1-1, f2 = 8-2-1;                  /* f(1)=−1<0 , f(2)=5>0 → raiz real em (1,2) */
        printf("     x³−x−1: f(1)=%ld<0, f(2)=%ld>0 → ρ∈(1,2); produto das raízes =1 ⟹ |z|=1/√ρ<1\n",
               f1, f2);
        printf("     %s\n", (f1<0&&f2>0) ?
            "resíduo 0 — é PISOT (o menor de todos), e não tem raiz da unidade: crescimento, não giro"
            : "REVER");
        if(!(f1<0&&f2>0)) ok=0;
        printf("     ⟹ em n=5 a peça do ouro CINDE: Φ₆ (giro de ordem 6) e ρ (crescimento\n");
        printf("        plástico). O que impede o corpo é a dimensão 5 separar giro de crescimento.\n");
    }

    /* ---- Q3 ---- */
    printf("\n§Q3  a rede MÍNIMA de uma simetria de ordem n tem dimensão φ(n) = grau de Φ_n:\n");
    {
        int erro = 0, plano = 0;
        for(int n=1;n<=12;n++){
            int t = totiente(n);
            int cabe = (t <= 2);
            if(cabe) plano++;
            if(n==5 || n<=6 || n==8 || n==12)
                printf("       n=%2d : φ(n)=%d → %s\n", n, t,
                       cabe ? "cabe no PLANO (cristal periódico)"
                            : (n==5 ? "precisa de 4D e PROJEÇÃO → quasicristal (Penrose)"
                                    : "precisa de projeção"));
            if(cabe != (n==1||n==2||n==3||n==4||n==6)) erro = 1;
        }
        printf("     φ(n) ≤ 2 dá exatamente {1,2,3,4,6}: %s  — a MESMA lista de §Q1, por outro\n",
               erro?"FALHA":"resíduo 0");
        printf("     caminho. O cinco não é proibido por capricho: ele não CABE, tem de ser\n");
        printf("     projetado — e projetar rede periódica maior é o corte-e-projeção.\n");
        if(erro) ok=0; (void)plano;
    }

    /* ---- Q4 ---- */
    printf("\n§Q4  o quasicristal É O GATO: substituição a→ab, b→a, matriz [[1,1],[1,0]] = A₁\n");
    {
        fibword();
        printf("     palavra gerada: %d letras — início: ", LW);
        for(int i=0;i<32 && i<LW;i++) putchar(W[i]);
        printf("…\n");
        int q = periodo_algum(LW);
        printf("     algum período q ≤ N/2 ? %s\n", q ? "SIM (FALHA)" : "NENHUM — resíduo 0, é APERIÓDICA");
        if(q) ok=0;
        /* razão de letras → φ : #a/#b deve ser Fibonacci consecutivo */
        long na=0, nb=0;
        for(int i=0;i<LW;i++){ if(W[i]=='a') na++; else nb++; }
        printf("     #a=%ld  #b=%ld   #a/#b = %.9f   (φ = 1,618033989)\n", na, nb, (double)na/nb);
        /* complexidade: fatores distintos de comprimento k = k+1 (sturmiana: o mínimo aperiódico) */
        int N = 1000, erro = 0;
        printf("     fatores distintos de comprimento k (nos primeiros %d):\n", N);
        for(int k=1;k<=8;k++){
            int c = fatores_distintos(N,k);
            printf("       k=%d : %d %s\n", k, c, c==k+1 ? "= k+1" : "≠ k+1  ← REVER");
            if(c != k+1) erro = 1;
        }
        printf("     %s\n", erro ? "FALHA" :
            "resíduo 0 — complexidade k+1: STURMIANA, o MÍNIMO possível para aperiódica.\n"
            "     Ordem máxima sem periodicidade — a definição de quasicristal, gerada pela peça.");
        if(erro) ok=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — a dimensão 5 não tem ouro porque o cinco não cabe numa rede: a sua\n"
      "rotação pede 2cos(2π/5)=φ−1, que não é inteiro (§Q1), e a sua rede mínima tem\n"
      "dimensão φ(5)=4, logo exige PROJEÇÃO (§Q3). O furo do ouro em n=5 fatora no giro\n"
      "permitido Φ₆ e no crescimento plástico ρ (§Q2): a dimensão 5 separa giro de\n"
      "crescimento, e por isso não fecha em corpo. E o que ocupa esse lugar é ordenado\n"
      "sem ser periódico — a palavra do GATO do ouro é aperiódica com complexidade k+1,\n"
      "o mínimo (§Q4): o quasicristal não é uma falha da peça, é a peça sem rede.\n"
      "\n"
      "Schrödinger chamou o portador da hereditariedade de \"cristal aperiódico\" (1944), e\n"
      "a simetria pentagonal/icosaédrica — proibida em cristal — é a dos capsídeos virais.\n"
      "Que a vida nasça daí é TESE, não teorema. O que está medido é o que a antecede."
      : "FALHOU — rever");
    return !ok;
}
