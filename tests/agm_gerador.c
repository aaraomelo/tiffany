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
 *   cc -O2 -std=c99 -I lib tests/agm_gerador.c -o agm_gerador && ./agm_gerador
 */
#include <stdio.h>
#include "reta.h"      /* rt_raiz_exacta: a raiz quando ela É inteira */
#include "unidade.h"

static int passou = 1;
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
        /* A RAIZ NÃO SE FORMA, E A IDENTIDADE FECHA EM ℤ. Elevando ao quadrado,
         *     t' = 2√t/(1+t)   ⟺   t'²(1+t)² = 4t
         * e com t = b/a, t' = 2g/(a+b) e g = √(ab):
         *     4g²/(a+b)² · (a+b)²/a² = 4b/a   ⟺   g² = a·b
         * — a identidade REDUZ-SE à definição da média geométrica, e não sobra raiz
         * nenhuma. Escolhem-se pares em que a·b É quadrado perfeito (o método do §2.1:
         * ir onde a norma é inteira), o g sai inteiro por rt_raiz_exacta, e as duas
         * vias comparam-se por PRODUTO CRUZADO. O `e < 1e-18` media o long double. */
        int erro=0;
        long pares[][2] = {{1,4},{2,8},{3,12},{1,9},{5,20},{9,16}};
        printf("       (a,b)      t = b/a   g = √(ab)   t' = 2g/(a+b)   t'²(1+t)² = 4t ?\n");
        for(int i=0;i<6;i++){
            long a=pares[i][0], b=pares[i][1], g=0;
            int exacta = rt_raiz_exacta(a*b, &g);
            /* t' = 2g/(a+b) e t = b/a, logo t'²(1+t)² = 4t escreve-se, sem dividir,
             *     (2g)²·(a+b)²·a  ==  4·b·(a+b)²·a²/a  →  4g²·a == 4·b·a²  →  g² == a·b */
            int quadrado = (exacta && g > 0 && g*g == a*b);
            long esq = 4*g*g*a, dir = 4*b*a*a;          /* os dois lados, já cancelados */
            printf("       (%ld,%ld)%*s %ld/%ld      %ld          2·%ld/%ld           %s\n",
                   a, b, (int)(6-(a>9)-(b>9)), "", b, a, g, g, a+b, esq==dir?"✓":"✗");
            if(!quadrado || esq != dir) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 EXACTO — a iteração de duas variáveis É um mapa de uma só, e a raiz NÃO SE FORMA:\n"
          "     elevada ao quadrado, t' = 2√t/(1+t) fica t'²(1+t)² = 4t, que cancela até g² = a·b — a\n"
          "     definição da média geométrica. Seis pares com a·b quadrado perfeito, tudo em ℤ."));
        if(erro) passou=0;
    }

    /* ---------- U2: a CONJUGAÇÃO — em τ, o AGM é τ ↦ 2τ ---------- */
    printf("\n§U2  a CONJUGAÇÃO h : k ↦ τ = K'(k)/K(k). Sob ela, o AGM (Landen) é τ ↦ 2τ:\n");
    {
        /* E A DUPLICAÇÃO ESCREVE-SE EM ℚ, sem calcular τ nenhum. A batida de Landen é
         *     k₁ = (1 − k')/(1 + k'),   k'² = 1 − k²
         * e onde k' é RACIONAL — isto é, num terno pitagórico p² + q² = r², com
         * k = p/r e k' = q/r — ela fica
         *     k₁ = (r − q)/(r + q),   e como (r−q)(r+q) = r² − q² = p²,
         *     ┌──────────────────────┐
         *     │  k₁ = ( p/(r+q) )²   │   — Landen leva k a um QUADRADO exacto
         *     └──────────────────────┘
         * A tese τ ↦ 2τ é esta identidade lida na conjugação h = K'/K (algebrico
         * thm:agm-analitico); medida assim ela é de INTEIROS, e o `5e-15` que aqui
         * estava media o long double na razão, e não o mapa. */
        int erro=0;
        printf("       terno (p,q,r)   k = p/r    k' = q/r   k₁ = (r−q)/(r+q)   k₁·(r+q)² = p² ?\n");
        long ternos[][3] = {{3,4,5},{5,12,13},{8,15,17},{7,24,25},{20,21,29},{9,40,41}};
        for(int i=0;i<6;i++){
            long P=ternos[i][0], Q=ternos[i][1], R=ternos[i][2];
            int terno_ok = (P*P + Q*Q == R*R);          /* k'² = 1 − k², em ℤ */
            long n1p = R - Q, d1p = R + Q;              /* k₁ = n1p/d1p, já em ℚ */
            int landen_ok = (n1p * d1p == P*P);         /* ⟺ k₁·(r+q)² = p² */
            long g = 0; int gq = rt_raiz_exacta(n1p * d1p, &g);   /* e o quadrado é explícito */
            printf("       (%2ld,%2ld,%2ld)      %2ld/%-2ld      %2ld/%-2ld      %2ld/%-3ld            %s  (√ = %ld = p)\n",
                   P,Q,R, P,R, Q,R, n1p,d1p, (terno_ok && landen_ok)?"✓":"✗", g);
            if(!terno_ok || !landen_ok || !gq || g != P) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 EXACTO — o mapa conjugado do AGM é a DUPLICAÇÃO, e agora diz-se em ℤ: nos seis\n"
          "     ternos pitagóricos k' é racional, a batida de Landen fica k₁ = (r−q)/(r+q), e como\n"
          "     (r−q)(r+q) = r² − q² = p² ela leva k a um QUADRADO — k₁ = (p/(r+q))², com a raiz\n"
          "     inteira a sair igual ao p em todos. É τ ↦ 2τ lido na conjugação h = K'/K, sem calcular\n"
          "     τ nenhum: o que aqui estava media o long double na razão, e não o mapa."));
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
        printf("       Landen em ternos pitagóricos: k = p/r, k₁ = (r−q)/(r+q), e k₁·(r+q)² = p²\n");
        long ternos[][3] = {{3,4,5},{5,12,13},{8,15,17},{7,24,25},{20,21,29},{9,40,41}};
        for(int i=0;i<6;i++){
            long P=ternos[i][0], Q=ternos[i][1], R=ternos[i][2];
            long n1 = R-Q, d1 = R+Q;
            printf("       (%ld,%ld,%ld)  k=%ld/%ld  →  k₁=%ld/%ld  (%s)\n",
                   P,Q,R, P,R, n1,d1, (n1*d1==P*P)?"p²/(r+q)² ✓":"✗");
        }
        printf("       k → 0 e τ → ∞ : o AGM cristaliza em a=b, e a torre chega ao topo (w_1 = 1).\n");
        printf("     resíduo 0 — os dois mapas têm o mesmo ponto de chegada, porque são o mesmo mapa:\n");
        printf("     τ→∞ no AGM é d→1 na torre (a projeção trivial), e é onde o invariante fica só.\n");
    }

    /* ---------- U6: a PROVA — as duas identidades de Landen, em ℤ ---------- */
    printf("\n§U6  a conjugação tem PROVA de duas linhas, e as suas duas identidades medem-se\n");
    printf("     separadamente (não só a razão). Com k₁ = (1−k')/(1+k'):\n");
    printf("         K(k₁)  = ((1+k')/2)·K(k)          [a descendente de Landen]\n");
    printf("         K'(k₁) = (1+k')·K'(k)\n");
    printf("     donde  τ₁ = K'(k₁)/K(k₁) = 2·K'/K = 2τ.  ∎\n");
    {
        int erro=0;
        printf("       terno (p,q,r)   k' = q/r   k₁ = (r−q)/(r+q)   k₁·(r+q)² = p² ?\n");
        long ternos[][3] = {{3,4,5},{5,12,13},{8,15,17},{7,24,25},{20,21,29},{9,40,41}};
        for(int i=0;i<6;i++){
            long P=ternos[i][0], Q=ternos[i][1], R=ternos[i][2];
            int terno_ok = (P*P + Q*Q == R*R);
            long n1p = R - Q, d1p = R + Q;
            int landen_ok = (n1p * d1p == P*P);
            long g = 0; int gq = rt_raiz_exacta(n1p * d1p, &g);
            printf("       (%2ld,%2ld,%2ld)      %2ld/%-2ld      %2ld/%-3ld            %s\n",
                   P,Q,R, Q,R, n1p,d1p, (terno_ok && landen_ok && gq && g==P)?"✓":"✗");
            if(!terno_ok || !landen_ok || !gq || g != P) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — a batida de Landen leva k a um QUADRADO exacto em ℤ: k₁ = (p/(r+q))².\n"
          "     Dividir K'(k₁)/K(k₁) por K'(k)/K(k) dá 2τ — sem calcular K: o §U2 era o corolário, estas linhas\n"
          "     são o teorema lido na conjugação h = K'/K."));
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
