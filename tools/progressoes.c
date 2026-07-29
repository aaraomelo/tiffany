/* progressoes.c — PA E PG DE ORDEM k, E O POLINÔMIO GENERALIZADO (O PAR).
 *
 * A tríade do §1 é ⊕ (a cisão), ⊗ (o gato) e ∏ (exp/log). As progressões de ordem superior são
 * exatamente essa tríade posta em sequências, e o teorema clássico é a ponte:
 *
 *      PA de ordem k  --(exp)-->  PG de ordem k        e  --(log)-->  de volta,
 *
 * com a ORDEM k preservada na travessia. Isto é o ∏ da tríade, e é o que liga os dois lados do corpo.
 *
 * As duas torres, uma de cada lado:
 *      ⊕  PA de ordem k :  Δᵏa = const ≠ 0 ,  Δᵏ⁺¹a = 0     ⟺  aₙ é POLINÔMIO de grau k
 *         base natural: os binomiais C(n,i)  (Newton),  aₙ = Σᵢ Δⁱa(0)·C(n,i)
 *      ⊗  PG de ordem k :  Rᵏb = const ≠ 1 ,  Rᵏ⁺¹b = 1     (R = razão, b_{n+1}/b_n)
 *         base natural: as potências,  bₙ = ∏ᵢ cᵢ^{C(n,i)}
 *
 * O POLINÔMIO GENERALIZADO é o PAR: as duas escritas do mesmo objeto, ligadas de dois modos —
 *   · pela ponte ∏ (exp/log), que troca o lado mantendo a ordem;
 *   · pelo par Δ (diferença) e Σ (acumulação), que são o gato e o esquilo do lado aditivo:
 *     Δ∘Σ = id, e Σ sobe a ordem k→k+1 enquanto Δ desce k→k−1. É a MESMA torre dimensional do
 *     corpo (dim n pela n−1), agora no lado ⊕.
 * E a conversão entre as duas BASES (potências ↔ binomiais) é Stirling, com S·s = I: a ida e a volta.
 *
 * Aqui a ponte exp/log é EXATA, não aproximada: mede-se em ℤ_p com um gerador g, onde o logaritmo
 * discreto existe e é exato. Resíduo 0 ou falha.
 *
 * Nota de notação: k é a ORDEM da progressão; m continua sendo o metal do gato (σ²=mσ+1). São
 * parâmetros diferentes, e o §P5 mostra onde se encontram.
 *
 *   cc -O2 -std=c99 progressoes.c -lm -o progressoes && ./progressoes
 */
#include <stdio.h>

#define KMAX 8
#define NMAX 40
static int ok = 1;
static long p = 10007, g = 0;                       /* ℤ_p e um gerador — para o log exato          */

static long md(long x){ x%=p; return x<0?x+p:x; }
static long ipow(long b, long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=(r*b)%p; b=(b*b)%p; e>>=1; } return r; }
static long inv(long a){ return ipow(a,p-2); }
static int primo(long n){ if(n<2)return 0; for(long d=2;d*d<=n;d++) if(n%d==0) return 0; return 1; }
static long ordem_de(long a){ long k=1,c=a; while(c!=1){ c=(c*a)%p; k++; if(k>p) return -1; } return k; }
static long log_disc(long y){                        /* log_g y, por busca — exato                  */
    long c=1;
    for(long i=0;i<p-1;i++){ if(c==y) return i; c=(c*g)%p; }
    return -1;
}
/* Δ e Σ — o gato e o esquilo do lado aditivo */
static void delta(const long *a, long *d, int n){ for(int i=0;i+1<n;i++) d[i]=md(a[i+1]-a[i]); }
static void sigma_acum(const long *a, long *s, int n){ s[0]=0; for(int i=0;i<n;i++) s[i+1]=md(s[i]+a[i]); }
/* a razão R — o Δ do lado multiplicativo */
static void razao(const long *b, long *r, int n){ for(int i=0;i+1<n;i++) r[i]=md(b[i+1]*inv(b[i])); }

static long binom_int(int n, int k){
    if(k<0||k>n) return 0;
    long r=1;
    for(int i=1;i<=k;i++) r = r*(n-k+i)/i;
    return r;
}
/* Stirling: x^n = Σ S(n,k)·(x)_k  e  (x)_n = Σ s(n,k)·x^k */
static long S2[KMAX+1][KMAX+1], S1[KMAX+1][KMAX+1];
static void stirling(void){
    for(int n=0;n<=KMAX;n++) for(int k=0;k<=KMAX;k++){ S2[n][k]=0; S1[n][k]=0; }
    S2[0][0]=1; S1[0][0]=1;
    for(int n=1;n<=KMAX;n++) for(int k=1;k<=n;k++){
        S2[n][k] = S2[n-1][k-1] + (long)k*S2[n-1][k];             /* segundo tipo                  */
        S1[n][k] = S1[n-1][k-1] - (long)(n-1)*S1[n-1][k];         /* primeiro tipo (com sinal)     */
    }
}

int main(void){
    while(!primo(p)) p++;
    for(long cand=2; cand<p; cand++) if(ordem_de(cand)==p-1){ g=cand; break; }
    stirling();

    printf("PROGRESSOES — PA e PG de ordem k, e o polinômio generalizado (o par)\n");
    printf("ℤ_%ld com gerador g=%ld (o log discreto é exato: a ponte ∏ não aproxima)\n", p, g);
    printf("=================================================================\n");

    /* ---------- P1: PA de ordem k ⟺ polinômio de grau k ---------- */
    printf("§P1  ⊕  PA de ordem k ⟺ POLINÔMIO de grau k:  Δᵏa = k!·aₖ  e  Δᵏ⁺¹a = 0\n");
    {
        int erro=0;
        for(int k=1;k<=5;k++){
            long a[NMAX], buf[2][NMAX];
            /* um polinômio de grau k com coeficiente líder 3 */
            for(int n=0;n<NMAX;n++){
                long v=0, pot=1;
                for(int i=0;i<=k;i++){ long c = (i==k)?3:(i+1); v = md(v + c*pot); pot = md(pot*n); }
                a[n]=v;
            }
            /* Δᵏ e Δᵏ⁺¹ */
            int cur=0, len=NMAX;
            for(int i=0;i<NMAX;i++) buf[0][i]=a[i];
            for(int t=0;t<k;t++){ delta(buf[cur],buf[1-cur],len); cur=1-cur; len--; }
            long konst = buf[cur][0]; int cte=1;
            for(int i=1;i<len;i++) if(buf[cur][i]!=konst) cte=0;
            long fat=1; for(int i=2;i<=k;i++) fat*=i;
            long esperado = md(fat*3);
            delta(buf[cur],buf[1-cur],len); cur=1-cur; len--;
            int zero=1; for(int i=0;i<len;i++) if(buf[cur][i]!=0) zero=0;
            printf("       k=%d : Δᵏa constante = %-6ld (k!·3 = %-6ld) %s ; Δᵏ⁺¹a = 0 %s\n",
                   k, konst, esperado, (cte&&konst==esperado)?"✓":"✗", zero?"✓":"✗");
            if(!cte || konst!=esperado || !zero) erro=1;
        }
        printf("     %s\n", erro?"FALHA":"resíduo 0 — a PA de ordem k É o polinômio de grau k");
        if(erro) ok=0;
    }

    /* ---------- P2: a base de Newton (os binomiais) ---------- */
    printf("\n§P2  a base natural do ⊕ são os BINOMIAIS: aₙ = Σᵢ Δⁱa(0)·C(n,i)  (Newton)\n");
    {
        int erro=0;
        for(int k=1;k<=5;k++){
            long a[NMAX], buf[2][NMAX], coef[KMAX+1];
            for(int n=0;n<NMAX;n++){
                long v=0, pot=1;
                for(int i=0;i<=k;i++){ long c=(i==k)?3:(i+1); v=md(v+c*pot); pot=md(pot*n); }
                a[n]=v;
            }
            int cur=0, len=NMAX;
            for(int i=0;i<NMAX;i++) buf[0][i]=a[i];
            coef[0]=buf[0][0];
            for(int t=1;t<=k;t++){ delta(buf[cur],buf[1-cur],len); cur=1-cur; len--; coef[t]=buf[cur][0]; }
            int bom=1;
            for(int n=0;n<20;n++){
                long soma=0;
                for(int i=0;i<=k;i++) soma = md(soma + coef[i]*md(binom_int(n,i)));
                if(soma != a[n]) bom=0;
            }
            printf("       k=%d : reconstrução por C(n,i) nos 20 primeiros termos %s\n", k, bom?"✓":"✗");
            if(!bom) erro=1;
        }
        printf("     %s\n", erro?"FALHA":"resíduo 0 — os binomiais são a base do lado aditivo");
        if(erro) ok=0;
    }

    /* ---------- P3: a PONTE ∏ (exp/log) leva PA_k em PG_k, preservando a ORDEM ---------- */
    printf("\n§P3  a PONTE ∏: exp leva PA de ordem k em PG de ordem k (e log volta) — a ORDEM\n");
    printf("     se preserva na travessia. Em ℤ_p o log é exato, então isto é resíduo 0:\n");
    {
        int erro=0;
        for(int k=1;k<=4;k++){
            long a[NMAX], b[NMAX], buf[2][NMAX];
            for(int n=0;n<24;n++){
                long v=0, pot=1;
                for(int i=0;i<=k;i++){ long c=(i==k)?2:(i+1); v=(v + c*pot) % (p-1); pot=(pot*n)%(p-1); }
                a[n]=v;                                            /* PA de ordem k nos EXPOENTES  */
            }
            for(int n=0;n<24;n++) b[n]=ipow(g,a[n]);               /* exp: PG de ordem k           */
            /* a PG tem ordem k? Rᵏb constante e Rᵏ⁺¹b = 1 */
            int cur=0, len=24;
            for(int i=0;i<24;i++) buf[0][i]=b[i];
            for(int t=0;t<k;t++){ razao(buf[cur],buf[1-cur],len); cur=1-cur; len--; }
            long kk=buf[cur][0]; int cte=1;
            for(int i=1;i<len;i++) if(buf[cur][i]!=kk) cte=0;
            razao(buf[cur],buf[1-cur],len); cur=1-cur; len--;
            int um=1; for(int i=0;i<len;i++) if(buf[cur][i]!=1) um=0;
            /* e o log volta à PA original */
            int volta=1;
            for(int n=0;n<12;n++) if(log_disc(b[n]) != a[n]) volta=0;
            printf("       k=%d : Rᵏb constante %s ; Rᵏ⁺¹b = 1 %s ; log(exp(a)) = a %s\n",
                   k, cte?"✓":"✗", um?"✓":"✗", volta?"✓":"✗");
            if(!cte||!um||!volta) erro=1;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — exp e log trocam ⊕ por ⊗ SEM mudar a ordem k. É o ∏ da tríade, e é\n"
          "     ele que faz do par (PA,PG) um só objeto em duas escritas.");
        if(erro) ok=0;
    }

    /* ---------- P4: Δ e Σ são o gato e o esquilo do lado ⊕, e movem a ordem ---------- */
    printf("\n§P4  Δ e Σ (diferença e acumulação) são o GATO e o ESQUILO do lado ⊕:\n");
    {
        int erro=0;
        long a[NMAX], s[NMAX], d[NMAX];
        for(int n=0;n<24;n++) a[n]=md(7*n*n + 3*n + 5);            /* PA de ordem 2                 */
        sigma_acum(a,s,24);                                        /* Σ: sobe a ordem              */
        delta(s,d,25);                                             /* Δ: desce de volta            */
        int idvolta=1;
        for(int n=0;n<24;n++) if(d[n]!=a[n]) idvolta=0;
        printf("       Δ∘Σ = id : %s   (a ida e a volta do lado aditivo, exata)\n", idvolta?"✓":"✗");
        if(!idvolta) erro=1;
        /* a ordem sobe com Σ e desce com Δ */
        long buf[2][NMAX]; int cur=0, len=25;
        for(int i=0;i<25;i++) buf[0][i]=s[i];
        int ordem_s=0;
        for(int t=1;t<=6;t++){
            delta(buf[cur],buf[1-cur],len); cur=1-cur; len--;
            int cte=1; for(int i=1;i<len;i++) if(buf[cur][i]!=buf[cur][0]) cte=0;
            if(cte && buf[cur][0]!=0){ ordem_s=t; break; }
        }
        printf("       a PA de ordem 2 acumulada tem ordem %d : %s  (Σ sobe k→k+1, Δ desce)\n",
               ordem_s, ordem_s==3?"✓":"✗");
        if(ordem_s!=3) erro=1;
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — a torre da ORDEM (⊕) é a mesma escada da torre da DIMENSÃO (⊗, §3):\n"
          "     Σ sobe um degrau, Δ desce, e a volta é exata. Gato e esquilo, do outro lado.");
        if(erro) ok=0;
    }

    /* ---------- P5: as duas BASES do mesmo espaço, e Stirling é a conversão ---------- */
    printf("\n§P5  o PAR em duas escritas: base de POTÊNCIAS (⊗) e base BINOMIAL (⊕). A\n");
    printf("     conversão é STIRLING, e a ida e volta é exata: Σₖ s(n,k)·S(k,j) = δₙⱼ\n");
    {
        int erro=0;
        for(int n=1;n<=6;n++){
            for(int j=1;j<=6;j++){
                long soma=0;
                for(int k=0;k<=KMAX;k++) soma += S1[n][k]*S2[k][j];
                long esperado = (n==j)?1:0;
                if(soma != esperado) erro=1;
            }
        }
        printf("       S·s = I para n,j ≤ 6 : %s\n", erro?"✗":"✓ resíduo 0");
        /* e a identidade de base: x^n = Σ S(n,k)·(x)_k, avaliada em vários x */
        int idbase=1;
        for(int n=1;n<=6;n++) for(long x=0;x<12;x++){
            long esq = ipow(x,n), dir=0;
            for(int k=0;k<=n;k++){
                long ff=1;                                        /* (x)_k = x(x−1)…(x−k+1)        */
                for(int i=0;i<k;i++) ff = md(ff*(x-i));
                dir = md(dir + md(S2[n][k])*ff);
            }
            if(esq != dir) idbase=0;
        }
        printf("       x^n = Σₖ S(n,k)·(x)ₖ  (n≤6, x<12) : %s\n", idbase?"✓ resíduo 0":"✗");
        if(!idbase) erro=1;
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — o mesmo polinômio nas duas bases: potências (o lado do gato) e\n"
          "     binomiais (o lado da cisão). Stirling é o dicionário, e é involutivo.");
        if(erro) ok=0;
    }

    /* ---------- P6: onde a ordem k encontra o metal m — a peça ---------- */
    printf("\n§P6  onde a ORDEM k encontra o METAL m: a peça é uma recorrência, não uma PA nem\n");
    printf("     uma PG — é o que fica ENTRE as duas, e por isso gera o corpo:\n");
    {
        int erro=0;
        for(long m=1;m<=3;m++){
            /* a sequência de Lucas U: U₀=0, U₁=1, U_{n+1} = m·U_n + U_{n−1}  (Fibonacci em m=1) */
            long U[24]; U[0]=0; U[1]=1;
            for(int n=1;n<23;n++) U[n+1] = md(m*U[n] + U[n-1]);
            /* não é PA de ordem k ≤ 6 (Δᵏ nunca estaciona) */
            long buf[2][NMAX]; int cur=0, len=24, pa=0;
            for(int i=0;i<24;i++) buf[0][i]=U[i];
            for(int t=1;t<=6;t++){
                delta(buf[cur],buf[1-cur],len); cur=1-cur; len--;
                int cte=1; for(int i=1;i<len;i++) if(buf[cur][i]!=buf[cur][0]) cte=0;
                if(cte){ pa=t; break; }
            }
            /* nem PG de ordem k ≤ 6 (as razões nunca estacionam) */
            cur=0; len=23; int pg=0;
            for(int i=0;i<23;i++) buf[0][i]=U[i+1];
            for(int t=1;t<=6;t++){
                razao(buf[cur],buf[1-cur],len); cur=1-cur; len--;
                int cte=1; for(int i=1;i<len;i++) if(buf[cur][i]!=buf[cur][0]) cte=0;
                if(cte){ pg=t; break; }
            }
            printf("       m=%ld : U_n é PA de ordem %s ; é PG de ordem %s\n", m,
                   pa?"(sim!)":"NENHUMA ≤6", pg?"(sim!)":"NENHUMA ≤6");
            if(pa||pg) erro=1;
        }
        printf("     %s\n", erro?"REVER — estacionou onde não devia":
          "resíduo 0 — a peça não é PA nem PG de ordem finita: é a REALIMENTAÇÃO (σ=m+1/σ),\n"
          "     e é justamente por não estacionar de nenhum dos dois lados que ela gera um corpo\n"
          "     em toda dimensão. A PA e a PG são os seus dois limites; o metal é o que está\n"
          "     entre eles.");
        if(erro) ok=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — o POLINÔMIO GENERALIZADO é um PAR, e a tríade do §1 é a sua gramática:\n"
      "  ⊕  a PA de ordem k É o polinômio de grau k, na base dos binomiais (Newton);\n"
      "  ⊗  a PG de ordem k é a sua imagem por exp, na base das potências;\n"
      "  ∏  exp/log atravessa de um lado ao outro SEM mudar a ordem k — a ponte.\n"
      "E as duas ligações internas: Δ e Σ são gato e esquilo do lado ⊕ (Δ∘Σ=id, Σ sobe a\n"
      "ordem, Δ desce) — a mesma escada da torre dimensional do corpo —, e Stirling é o\n"
      "dicionário entre as duas bases, involutivo (S·s=I).\n"
      "\n"
      "E a peça não é nenhuma das duas: U_n(m) não é PA nem PG de ordem finita. Ela é a\n"
      "realimentação σ=m+1/σ, o que fica ENTRE a PA e a PG — e é por não estacionar de\n"
      "nenhum lado que gera corpo em toda dimensão."
      : "FALHOU — rever");
    return !ok;
}
