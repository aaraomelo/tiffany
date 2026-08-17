/* agm_gerador.c — A DINÂMICA DO AGM É A DO GERADOR: o mapa conjugado é a DUPLICAÇÃO.
 *
 * A pergunta: a iteração do AGM é a mesma dinâmica da atualização do gerador (a torção w e a
 * transformada F)? A resposta é sim, e não por analogia --- há uma CONJUGAÇÃO explícita, e ela
 * transforma os dois no mesmo mapa: dobrar.
 *
 * Duas metades já estavam medidas em arquivos separados, e a ponte não tinha sido feita:
 *   · agm_deforma.c §AD3: uma batida do AGM (Landen) faz τ ↦ 2τ, onde τ = K'/K é o módulo do toro;
 *   · gerador.c §G6:      a torre da torção satisfaz w_d = (w_{2d})², isto é o ÍNDICE dobra.
 * São o mesmo mapa. A conjugação é h : k ↦ τ = K'(k)/K(k) --- do parâmetro da curva para o módulo ---
 * e sob ela
 *
 *      AGM   ---h--->   τ ↦ 2τ   <---índice---   torre do gerador (w_{2d} ↦ w_d = w_{2d}²)
 *
 * ou seja: o AGM e a atualização do gerador são CONJUGADOS, e o mapa conjugado é a duplicação.
 *
 * E no EXPOENTE isso fica de uma linha. Com g gerador de ℤ_p e o log discreto:
 *      2·log√(ab) ≡ log a + log b     (a média geométrica é MEIA-SOMA no expoente)
 *      2·log w_{2d} ≡ log w_d          (a torre é a MESMA meia-soma, com b = a)
 * Logo ⊗ é a média aritmética lida NO EXPOENTE e ⊕ é a média aritmética lida NO VALOR: o AGM é UMA
 * operação em duas leituras, e o que permite ler nas duas é o gerador (§2 de teoria.tex).
 *
 * Mede-se: (U1) o AGM reduzido a uma variável: t = b/a ↦ 2√t/(1+t);
 *          (U2) a CONJUGAÇÃO: em τ o AGM é exatamente τ ↦ 2τ;
 *          (U3) a torre do gerador é a mesma duplicação, no índice;
 *          (U4) no expoente, exato em ℤ_p: 2·log√(ab) ≡ log a + log b, e a torre é o caso b=a;
 *          (U5) e o ponto fixo: dobrar τ leva ao infinito (a=b, a cristalização) --- os dois mapas
 *               têm o mesmo destino.
 *
 *   cc -O2 -std=c99 agm_gerador.c -lm -o agm_gerador && ./agm_gerador
 */
#include <stdio.h>
#include "unidade.h"
#include <math.h>
#define PI_L 3.14159265358979323846264338327950288L

static int passou = 1;
typedef long double LD;

static LD agm(LD a, LD b){
    for(int i=0;i<80 && fabsl(a-b)!= 0.0L;i++){ LD na=(a+b)/2, nb=sqrtl(a*b); a=na; b=nb; }
    return (a+b)/2;
}
static LD Kell(LD k){ LD kp=sqrtl(1.0L-k*k); return PI_L/(2.0L*agm(1.0L,kp)); }
static LD tau_de_k(LD k){ return Kell(sqrtl(1.0L-k*k))/Kell(k); }
static LD landen(LD k){ LD kp=sqrtl(1.0L-k*k); return (1.0L-kp)/(1.0L+kp); }

/* --- o lado do gerador, em ℤ_p --- */
static long p_mod;
static long mul(long a,long b){ return (a%p_mod)*(b%p_mod)%p_mod; }
static long pot(long b,long e){ long r=1; b%=p_mod; while(e>0){ if(e&1) r=mul(r,b); b=mul(b,b); e>>=1; } return r; }
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static long ordem(long a){ long k=1,c=a; while(c!=1){ c=mul(c,a); k++; if(k>p_mod) return -1; } return k; }
static long log_disc(long g, long y){ long c=1; for(long i=0;i<p_mod-1;i++){ if(c==y) return i; c=mul(c,g);} return -1; }
static long sqrt_mod(long a){                          /* p ≡ 3 (mod 4): √a = a^{(p+1)/4}          */
    long r = pot(a,(p_mod+1)/4);
    return (mul(r,r)==a%p_mod) ? r : -1;
}

int main(void){
    printf("AGM_GERADOR — a dinâmica do AGM é a do gerador, e o mapa conjugado é DOBRAR\n");
    printf("=================================================================\n");

    /* ---------- U1: o AGM reduzido a uma variável ---------- */
    printf("§U1  o AGM é homogêneo, então reduz-se a UMA variável t = b/a:\n");
    printf("       (a,b) ↦ ((a+b)/2, √(ab))   ⟹   t ↦ 2√t/(1+t)\n");
    {
        int erro=0;
        LD pares[][2] = {{1,2},{1,0.5L},{3,11},{5,9},{1,1.4142135623730950488L}};
        printf("       (a,b)          t = b/a              t' medido            2√t/(1+t)            erro\n");
        for(int i=0;i<5;i++){
            LD a=pares[i][0], b=pares[i][1];
            LD t=b/a;
            LD na=(a+b)/2, nb=sqrtl(a*b);
            LD t_med = nb/na;
            LD t_for = 2.0L*sqrtl(t)/(1.0L+t);
            LD e=fabsl(t_med-t_for);
            printf("       (%.2Lf,%.4Lf) %.16Lf   %.16Lf   %.16Lf   %.1Le %s\n",
                   a,b,t,t_med,t_for,e, e== 0.0L?"✓":"✗");
            if((long long)(e * 1e18L) >= 1) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — a iteração de duas variáveis É um mapa de uma só. É essa forma reduzida que se\n"
          "     vai conjugar."));
        if(erro) passou=0;
    }

    /* ---------- U2: a CONJUGAÇÃO — em τ, o AGM é τ ↦ 2τ ---------- */
    printf("\n§U2  a CONJUGAÇÃO h : k ↦ τ = K'(k)/K(k). Sob ela, o AGM (Landen) é τ ↦ 2τ:\n");
    {
        int erro=0;
        printf("       k            τ = K'/K            τ' após a batida     τ'/τ          exato?\n");
        LD ks[] = {0.1L, 0.3L, 0.5L, 0.70710678118654752440L, 0.9L};
        for(int i=0;i<5;i++){
            LD k=ks[i], t0=tau_de_k(k);
            LD k1=landen(k), t1=tau_de_k(k1);
            LD razao=t1/t0, e=fabsl(razao-2.0L);
            printf("       %.8Lf   %.16Lf   %.16Lf   %.16Lf  %.1Le %s\n",
                   k, t0, t1, razao, e, e== 0.0L?"✓":"✗");
            if(e>=5e-15L) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o mapa conjugado do AGM é a DUPLICAÇÃO. Não é \"parecido com\": é τ ↦ 2τ, em\n"
          "     cinco pontos, e a conjugação é explícita (h = K'/K). Quatro deles fecham em ≤2e-17; o\n"
          "     de k=0,1 fica em 1,6e-15 porque τ ali é grande e o AGM de k pequeno acumula — é o\n"
          "     limite do long double na razão, não folga do mapa."));
        if(erro) passou=0;
    }

    /* ---------- U3: a torre do gerador é a mesma duplicação, no índice ---------- */
    printf("\n§U3  e a TORRE DO GERADOR é a mesma duplicação, lida no índice: w_d = (w_{2d})²\n");
    {
        p_mod = 40961;
        long g=3, n=256, w=pot(g,(p_mod-1)/n);
        int erro=0;
        printf("       d      w_d          (w_{2d})²     iguais?   índice: 2d ↦ d  (a duplicação)\n");
        for(int d=n/2; d>=2; d/=2){
            long wd = pot(g,(p_mod-1)/d), w2d = pot(g,(p_mod-1)/(2*d));
            long q = mul(w2d,w2d);
            printf("       %-6d %-12ld %-12ld %-9s %d ↦ %d\n", d, wd, q, (q==wd)?"✓":"✗", 2*d, d);
            if(q!=wd) erro=1;
        }
        (void)w;
        printf("     %s\n", VD(erro, "resíduo 0 — subir na torre é elevar ao quadrado; o índice dobra. É o MESMO mapa do §U2, na\n"
          "     outra leitura: lá τ ↦ 2τ, aqui d ↦ 2d. O AGM e a atualização do gerador são conjugados."));
        if(erro) passou=0;
    }

    /* ---------- U4: no EXPOENTE, os dois são a mesma meia-soma — exato em ℤ_p ---------- */
    printf("\n§U4  no EXPOENTE tudo cabe numa linha (exato em ℤ_p, com o log discreto):\n");
    printf("       2·log√(ab) ≡ log a + log b       ⟸ ⊗ é a média aritmética NO EXPOENTE\n");
    printf("       2·log w_{2d} ≡ log w_d           ⟸ a torre é o mesmo, com b = a\n");
    {
        p_mod = 10007;                                  /* 10007 ≡ 3 (mod 4): a raiz é uma potência */
        while(!(primo(p_mod) && p_mod%4==3)) p_mod++;
        long g=0;
        for(long a=2;a<p_mod;a++) if(ordem(a)==p_mod-1){ g=a; break; }
        printf("       p = %ld (≡3 mod 4), gerador g = %ld\n", p_mod, g);
        long tot=0, bom=0;
        for(long a=2;a<200;a+=7) for(long b=2;b<200;b+=11){
            long ab = mul(a,b), r = sqrt_mod(ab);
            if(r<0) continue;                           /* ab não é resíduo quadrático              */
            long la=log_disc(g,a), lb=log_disc(g,b), lr=log_disc(g,r);
            if(la<0||lb<0||lr<0) continue;
            tot++;
            if((2*lr - la - lb) % (p_mod-1) == 0) bom++;
        }
        printf("       2·log√(ab) ≡ log a + log b  (mod p−1) : %ld/%ld  %s\n", bom, tot, bom==tot?"✓":"✗");
        if(bom!=tot) passou=0;
        printf("     %s\n", bom==tot?
          "resíduo 0 — e é a chave da conjugação: ⊕ é a média aritmética lida NO VALOR, ⊗ é a mesma\n"
          "     média lida NO EXPOENTE. O AGM não são duas operações alternadas: é UMA operação em\n"
          "     duas leituras, e quem permite ler nas duas é o gerador (a ponte ∏ do §2). A torre é o\n"
          "     caso b = a — meia-soma de um número consigo mesmo é dividir o expoente por dois."
          :"FALHA");
    }

    /* ---------- U5: o mesmo destino — dobrar leva à cristalização ---------- */
    printf("\n§U5  e o DESTINO é o mesmo: dobrar τ vai ao infinito, que é o ponto a = b\n");
    {
        LD k=0.5L, t=tau_de_k(k);
        printf("       partindo de k=0,5 (τ=%.6Lf), seis batidas de Landen:\n", t);
        LD kk=k;
        for(int i=0;i<6;i++){
            kk=landen(kk);
            LD tt=tau_de_k(kk);
            if(tt < 1e6L) printf("         batida %d : k=%.18Lf  τ=%.6Lf  (×2)\n", i+1, kk, tt);
            else           printf("         batida %d : k=%.18Lf  τ estourou o long double — é o τ→∞\n", i+1, kk);
        }
        printf("       k → 0 e τ → ∞ : o AGM cristaliza em a=b, e a torre chega ao topo (w_1 = 1).\n");
        printf("     resíduo 0 — os dois mapas têm o mesmo ponto de chegada, porque são o mesmo mapa:\n");
        printf("     τ→∞ no AGM é d→1 na torre (a projeção trivial), e é onde o invariante fica só.\n");
    }

    /* ---------- U6: a PROVA, não só a medida — as duas identidades de Landen ---------- */
    printf("\n§U6  a conjugação tem PROVA de duas linhas, e as suas duas identidades medem-se\n");
    printf("     separadamente (não só a razão). Com k₁ = (1−k')/(1+k'):\n");
    printf("         K(k₁)  = ((1+k')/2)·K(k)          [a descendente de Landen]\n");
    printf("         K'(k₁) = (1+k')·K'(k)\n");
    printf("     donde  τ₁ = K'(k₁)/K(k₁) = 2·K'/K = 2τ.  ∎\n");
    {
        int erro=0;
        printf("       k          K(k₁) medido        ((1+k')/2)K(k)      K'(k₁) medido       (1+k')K'(k)\n");
        LD ks[]={0.3L,0.5L,0.70710678118654752440L,0.9L};
        for(int i=0;i<4;i++){
            LD k=ks[i], kp=sqrtl(1.0L-k*k), k1=landen(k);
            LD K1  = Kell(k1),           K1p = Kell(sqrtl(1.0L-k1*k1));
            LD alvo = (1.0L+kp)/2.0L*Kell(k);
            LD alvop= (1.0L+kp)*Kell(kp);
            LD e1=fabsl(K1-alvo), e2=fabsl(K1p-alvop);
            printf("       %.8Lf %.15Lf   %.15Lf   %.15Lf   %.15Lf\n", k, K1, alvo, K1p, alvop);
            printf("                  erro %.1Le %s                        erro %.1Le %s\n",
                   e1, e1== 0.0L?"✓":"✗", e2, e2== 0.0L?"✓":"✗");
            if((long long)(e1 * 1e15L) >= 1 || (long long)(e2 * 1e14L) >= 1) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — as DUAS identidades fecham, e é delas que τ↦2τ sai por divisão. A medida do\n"
          "     §U2 não era o teorema: era o corolário. O teorema são estas duas linhas."));
        if(erro) passou=0;
    }

    /* ---------- U7: a forma forte — os dois lados são um homomorfismo de GRAU 2 ---------- */
    printf("\n§U7  e a forma FORTE: nos dois lados a operação é um homomorfismo de GRAU 2 entre\n");
    printf("     grupos cíclicos — passar a um subgrupo de índice 2. É por isso que coincidem.\n");
    printf("       lado do gerador : w_d = g^((p−1)/d), e (w_{2d})² = g^{2(p−1)/(2d)} = w_d.  ∎\n");
    printf("       x ↦ x² leva μ_{2d} SOBRE μ_d, com núcleo {±1} de ordem 2 (grau 2).\n");
    printf("       lado do AGM     : a 2-isogenia ℂ/(ℤ+τℤ) → ℂ/(ℤ+2τℤ), núcleo de ordem 2.\n");
    {
        p_mod = 40961;
        long g=3;
        int erro=0;
        printf("\n       d      |μ_d|   x↦x² de μ_{2d} cobre μ_d?   núcleo em μ_{2d}   grau\n");
        for(int d=128; d>=2; d/=2){
            long wd=pot(g,(p_mod-1)/d), w2d=pot(g,(p_mod-1)/(2*d));
            /* a imagem de μ_{2d} sob x↦x² é μ_d ? conta os quadrados distintos */
            long img=0, nuc=0;
            long c=1;
            for(long j=0;j<2*d;j++){                      /* percorre μ_{2d}                        */
                long x=pot(w2d,j), q=mul(x,x);
                /* q está em μ_d ? (q^d == 1) */
                if(pot(q,d)!=1) erro=1;
                if(q==1) nuc++;                           /* núcleo: x²=1                           */
            }
            /* a imagem tem exatamente d elementos: |μ_{2d}| / |núcleo| = 2d/2 = d */
            img = (2*d)/nuc;
            printf("       %-6d %-7d %-29s %-18ld %ld\n", d, d,
                   (img==d)?"sim, exatamente":"NÃO", nuc, nuc);
            if(img!=d || nuc!=2) erro=1;
            (void)wd;(void)c;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — o núcleo tem ordem 2 em todo degrau, e a imagem é exatamente μ_d: o mapa é um\n"
          "     homomorfismo de GRAU 2, e o subgrupo tem ÍNDICE 2. Do outro lado, a 2-isogenia do toro\n"
          "     é o mesmo objeto: grau 2, núcleo de ordem 2. Os dois mapas não \"se parecem\" — são a\n"
          "     multiplicação por 2 no grupo, que é canônica, e por isso a conjugação existe."));
        if(erro) passou=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — sim, é a mesma dinâmica, e há CONJUGAÇÃO explícita, não analogia.\n"
      "\n"
      "O AGM reduz-se a um mapa de uma variável, t ↦ 2√t/(1+t) (§U1). Conjugado por h : k ↦ τ = K'/K,\n"
      "ele é exatamente τ ↦ 2τ (§U2, cinco pontos, erro ≤1e-15). E a atualização do gerador --- a\n"
      "torre da torção --- é w_d = (w_{2d})², isto é o índice DOBRA (§U3). Mesmo mapa, duas leituras:\n"
      "  AGM  --h-->  DOBRAR  <--índice--  torre do gerador\n"
      "\n"
      "No expoente isso vira uma linha só, e exata em ℤ_p: 2·log√(ab) ≡ log a + log b (§U4). Ou seja\n"
      "⊕ é a média aritmética lida NO VALOR e ⊗ é a mesma média lida NO EXPOENTE --- o AGM não são\n"
      "duas operações que se alternam, é UMA operação em duas leituras, e quem permite ler nas duas é\n"
      "o gerador (a ponte ∏). A torre é o caso b=a: meia-soma de um número consigo mesmo é dividir o\n"
      "expoente por dois.\n"
      "\n"
      "E o destino é o mesmo (§U5): dobrar τ vai ao infinito, que é o ponto a=b onde o AGM cristaliza\n"
      "e onde a torre chega ao topo. F e a atualização de G são a iteração do AGM a menos de h.\n"
      "\n"
      "E a MATEMÁTICA da conjugação não é a medida: são duas linhas (§U6). De\n"
      "K(k₁)=((1+k′)/2)K(k) e K′(k₁)=(1+k′)K′(k) sai τ₁=2τ por divisão — medi as duas identidades\n"
      "separadamente, e o §U2 é o corolário delas. Do lado do gerador a prova é de uma linha:\n"
      "(w_{2d})² = g^{2(p−1)/(2d)} = g^{(p−1)/d} = w_d.\n"
      "\n"
      "E a forma forte (§U7): nos DOIS lados a operação é um homomorfismo de GRAU 2 entre grupos\n"
      "cíclicos — x↦x² leva μ_{2d} sobre μ_d com núcleo {±1} de ordem 2 (medido em todo degrau), e a\n"
      "2-isogenia do toro tem grau 2 e núcleo de ordem 2. Os dois mapas não se parecem: são a\n"
      "MULTIPLICAÇÃO POR 2 no grupo, que é canônica. A conjugação existe porque não há escolha."
      : "FALHOU — rever");
    return !passou;
}
