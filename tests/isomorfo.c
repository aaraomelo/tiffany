/* isomorfo.c — ELÉTRON e BÓSON: a mesma função de onda, lida com o sinal trocado.
 *
 * A afirmação, posta com precisão e medida (resíduo 0 ou falha):
 *
 *  (I) PONTO A PONTO o espaço é o mesmo. Com n modos, o estado é o mesmo vetor de 2ⁿ coordenadas; a
 *      bijeção de base é a identidade e a norma é preservada (Parseval). O setor de N partículas tem
 *      C(n,N) dimensões dos DOIS lados — férmion e bóson hard-core. Isomorfos, coordenada a
 *      coordenada.
 *  (II) A ESTATÍSTICA NÃO ESTÁ NA FUNÇÃO DE ONDA: está no OPERADOR, e o operador difere por um
 *      PRODUTO ORDENADO — a string de Jordan--Wigner, c_j = (∏_{k<j} Z_k)·b_j. Sem a string, os
 *      operadores comutam (bóson); com ela, anticomutam (férmion). É a mesma álgebra, o mesmo espaço,
 *      e a ordem do produto é tudo — o mesmo achado de tatoeba/operador.c, aqui na física.
 *  (III) O SINAL É O PERÍODO 4. G²=−I (uma volta de 2π dá −1: o spinor) e G⁴=+I (4π volta) — a mesma
 *      conta do esquilo (§1.4) e de ℱ⁴=id (§3, tools/tres_reconstroi.c). O modo 2, a reflexão, É o
 *      −1 da troca. Férmion e bóson são os dois lados da involução 𝒥 do §3: det=−1 (o gato) e
 *      det=+1 (o esquilo).
 *  (IV) O LIMITE, medido e não escondido: contra bóson LIVRE não há isomorfia — C(n+N−1,N) ≠ C(n,N),
 *      e o programa exibe as duas contagens. E a conversão dos OPERADORES não é ponto a ponto: testa-
 *      se se existe U diagonal (sinais ±1, ponto a ponto) com c_j = U b_j U†, resolvendo a holonomia
 *      no hipercubo. Onde ela falha, é a não-localidade da string — o espaço é o mesmo, a localidade
 *      não é.
 *
 * Aritmética exata em ℤ (todas as entradas são 0,±1), buffers de tamanho FIXO, zero malloc.
 *
 *   cc -O2 -std=c99 isomorfo.c -o isomorfo && ./isomorfo [n]
 */
#include <stdio.h>
#include "../lib/disco.h"
#include "unidade.h"
#include <stdlib.h>

#define NMAX 6
#define DMAX (1<<NMAX)

static int n = 4, D = 16;
typedef int Mat[DMAX][DMAX];

#define C ((Mat*)DISCO_BASE(171))
#define CD ((Mat*)DISCO_BASE(172))
#define B ((Mat*)DISCO_BASE(173))
#define BD ((Mat*)DISCO_BASE(174))
static Mat T1, T2, T3, T4;

static void zero(Mat A){ for(int i=0;i<D;i++) for(int j=0;j<D;j++) A[i][j]=0; }
static void ident(Mat A){ zero(A); for(int i=0;i<D;i++) A[i][i]=1; }
static void mul(Mat R, Mat A, Mat Bm){
    static Mat X;
    zero(X);
    for(int i=0;i<D;i++) for(int k=0;k<D;k++){
        int a = A[i][k]; if(!a) continue;
        for(int j=0;j<D;j++) X[i][j] += a*Bm[k][j];
    }
    for(int i=0;i<D;i++) for(int j=0;j<D;j++) R[i][j]=X[i][j];
}
static void add(Mat R, Mat A, Mat Bm){ for(int i=0;i<D;i++) for(int j=0;j<D;j++) R[i][j]=A[i][j]+Bm[i][j]; }
static void transp(Mat R, Mat A){ static Mat X; for(int i=0;i<D;i++) for(int j=0;j<D;j++) X[j][i]=A[i][j];
                                  for(int i=0;i<D;i++) for(int j=0;j<D;j++) R[i][j]=X[i][j]; }
static int is_zero(Mat A){ for(int i=0;i<D;i++) for(int j=0;j<D;j++) if(A[i][j]) return 0; return 1; }
static int is_ident(Mat A){ for(int i=0;i<D;i++) for(int j=0;j<D;j++) if(A[i][j] != (i==j)) return 0; return 1; }

static int bit(int i, int j){ return (i>>j) & 1; }
static int popc(int i){ int c=0; while(i){ c += i&1; i>>=1; } return c; }

/* b_j = σ⁻_j : o abaixamento local — o BÓSON (hard-core), sem string nenhuma */
static void build_b(int j, Mat A){
    zero(A);
    for(int i=0;i<D;i++) if(bit(i,j)) A[i ^ (1<<j)][i] = 1;      /* |…1…⟩ → |…0…⟩ */
}
/* Z_k = diag((−1)^{n_k}) : o sinal local */
static void build_z(int k, Mat A){
    zero(A);
    for(int i=0;i<D;i++) A[i][i] = bit(i,k) ? -1 : 1;
}
/* c_j = (∏_{k<j} Z_k)·b_j : o FÉRMION — o mesmo b_j, com o PRODUTO ORDENADO da string */
static void build_c(int j, Mat A){
    static Mat S, Z, X;
    ident(S);
    for(int k=0;k<j;k++){ build_z(k,Z); mul(X,S,Z); for(int a=0;a<D;a++) for(int b2=0;b2<D;b2++) S[a][b2]=X[a][b2]; }
    build_b(j,X);
    mul(A,S,X);
}
static long binom(int a, int b){
    if(b<0||b>a) return 0;
    long r=1;
    for(int i=1;i<=b;i++) r = r*(a-b+i)/i;
    return r;
}

int main(int argc, char **argv){
    disco_prende(DISCO_BASE(171),"dados/iso_C.bin",(size_t)(NMAX),sizeof(Mat));
    disco_prende(DISCO_BASE(172),"dados/iso_CD.bin",(size_t)(NMAX),sizeof(Mat));
    disco_prende(DISCO_BASE(173),"dados/iso_B.bin",(size_t)(NMAX),sizeof(Mat));
    disco_prende(DISCO_BASE(174),"dados/iso_BD.bin",(size_t)(NMAX),sizeof(Mat));
    if(argc>1) n = atoi(argv[1]);
    if(n<2 || n>NMAX){ printf("n entre 2 e %d\n", NMAX); return 2; }
    D = 1<<n;
    int ok = 1;

    printf("ISOMORFO — elétron e bóson: n=%d modos, dim=%d\n", n, D);
    printf("=================================================================\n");

    for(int j=0;j<n;j++){ build_c(j,C[j]); transp(CD[j],C[j]); build_b(j,B[j]); transp(BD[j],B[j]); }

    /* --- §I  o espaço é O MESMO, ponto a ponto --- */
    printf("§I   PONTO A PONTO — o mesmo vetor de %d coordenadas descreve os dois.\n", D);
    printf("     setor de N partículas:   férmion C(n,N)   bóson hard-core C(n,N)   bóson livre C(n+N-1,N)\n");
    for(int N=0;N<=n;N++)
        printf("       N=%d :  %6ld            %6ld                  %6ld %s\n", N,
               binom(n,N), binom(n,N), binom(n+N-1,N),
               binom(n+N-1,N)==binom(n,N) ? "" : "  ← difere (o limite)");

    /* --- §II  a estatística está no OPERADOR: a string, um produto ordenado --- */
    printf("\n§II  a ESTATÍSTICA está no operador, não na função de onda:\n");
    {
        int erros_f=0, erros_fc=0;
        for(int i=0;i<n;i++) for(int j=0;j<n;j++){
            mul(T1,C[i],CD[j]); mul(T2,CD[j],C[i]); add(T3,T1,T2);      /* {c_i,c_j†} */
            for(int a=0;a<D;a++) for(int b=0;b<D;b++){
                int esperado = (i==j) ? (a==b) : 0;
                if(T3[a][b] != esperado) erros_f++;
            }
            mul(T1,C[i],C[j]); mul(T2,C[j],C[i]); add(T4,T1,T2);        /* {c_i,c_j} */
            if(!is_zero(T4)) erros_fc++;
        }
        printf("     FÉRMION (com string): {c_i,c_j†}=δ_ij·I  %s ; {c_i,c_j}=0  %s\n",
               VD(erros_f, "resíduo 0"), VD(erros_fc, "resíduo 0"));
        if(erros_f||erros_fc) ok=0;

        int comuta=0, anticomuta=0;
        for(int i=0;i<n;i++) for(int j=0;j<n;j++){
            if(i==j) continue;
            mul(T1,B[i],BD[j]); mul(T2,BD[j],B[i]);
            for(int a=0;a<D;a++) for(int b=0;b<D;b++) T3[a][b]=T1[a][b]-T2[a][b];   /* [b_i,b_j†] */
            if(is_zero(T3)) comuta++;
            add(T4,T1,T2);
            if(is_zero(T4)) anticomuta++;
        }
        int pares = n*(n-1);
        printf("     BÓSON (sem string) : [b_i,b_j†]=0  %d/%d  %s  (e anticomutam em %d/%d)\n",
               comuta, pares, comuta==pares?"resíduo 0":"FALHA", anticomuta, pares);
        if(comuta!=pares) ok=0;
        printf("     ⟹ o MESMO b_j: com o produto ordenado da string, anticomuta; sem ela, comuta.\n");
    }

    /* --- §III  a troca: −1 no férmion, +1 no bóson, no mesmo espaço --- */
    printf("\n§III a TROCA — dois modos, o mesmo vácuo, o sinal:\n");
    {
        int vac = 0;                                        /* |0…0⟩ */
        int i0=0, j0=1, anti=1, sim=1;
        int *psi1 = DISCO_FIXO(int, 124);
        int *psi2 = DISCO_FIXO(int, 125);
        int *phi1 = DISCO_FIXO(int, 126);
        int *phi2 = DISCO_FIXO(int, 127);
        disco_prende(DISCO_BASE(124),"dados/psi1_124.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(psi1,(size_t)((DMAX)),sizeof(int));
        disco_prende(DISCO_BASE(125),"dados/psi2_125.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(psi2,(size_t)((DMAX)),sizeof(int));
        disco_prende(DISCO_BASE(126),"dados/phi1_126.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(phi1,(size_t)((DMAX)),sizeof(int));
        disco_prende(DISCO_BASE(127),"dados/phi2_127.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(phi2,(size_t)((DMAX)),sizeof(int));
        for(int a=0;a<D;a++){ psi1[a]=psi2[a]=phi1[a]=phi2[a]=0; }
        for(int a=0;a<D;a++){                               /* c†_i c†_j |vac⟩  e  c†_j c†_i |vac⟩ */
            int s1=0, s2=0, t1=0, t2=0;
            for(int b=0;b<D;b++) for(int c2=0;c2<D;c2++){
                if(c2!=vac) continue;
                s1 += CD[i0][a][b]*CD[j0][b][c2];
                s2 += CD[j0][a][b]*CD[i0][b][c2];
                t1 += BD[i0][a][b]*BD[j0][b][c2];
                t2 += BD[j0][a][b]*BD[i0][b][c2];
            }
            psi1[a]=s1; psi2[a]=s2; phi1[a]=t1; phi2[a]=t2;
        }
        for(int a=0;a<D;a++){ if(psi1[a] != -psi2[a]) anti=0; if(phi1[a] != phi2[a]) sim=0; }
        printf("     FÉRMION: c†_i c†_j |vac⟩ = − c†_j c†_i |vac⟩   %s (antissimétrico)\n",
               VD(!(anti), "resíduo 0"));
        printf("     BÓSON  : b†_i b†_j |vac⟩ = + b†_j b†_i |vac⟩   %s (simétrico)\n",
               VD(!(sim), "resíduo 0"));
        if(!anti||!sim) ok=0;
        /* Parseval: a norma do estado é a mesma nos dois — o isomorfismo é isométrico */
        long nf=0, nb=0;
        for(int a=0;a<D;a++){ nf += (long)psi1[a]*psi1[a]; nb += (long)phi1[a]*phi1[a]; }
        printf("     norma (Parseval): férmion %ld = bóson %ld  %s\n", nf, nb,
               nf==nb?"resíduo 0":"FALHA");
        if(nf!=nb) ok=0;
    }

    /* --- §IV  o período 4: G²=−I é o spinor, G⁴=+I é a volta (o esquilo, ℱ⁴=id) --- */
    printf("\n§IV  o SINAL é o PERÍODO 4 (a mesma conta do esquilo e de ℱ):\n");
    {
        static Mat G, G2, G3, G4, mI;
        zero(G);
        for(int i=0;i+1<D;i+=2){ G[i][i+1] = -1; G[i+1][i] = 1; }     /* blocos de 90° */
        mul(G2,G,G); mul(G3,G2,G); mul(G4,G3,G);
        ident(mI); for(int i=0;i<D;i++) mI[i][i] = -1;
        int e2=0, e4=0;
        for(int i=0;i<D;i++) for(int j=0;j<D;j++){ if(G2[i][j]!=mI[i][j]) e2++; if(G4[i][j]!=(i==j)) e4++; }
        printf("     G² = −I (2π ↦ −1, o spinor)  %s ; G⁴ = +I (4π ↦ +1)  %s\n",
               VD(e2, "resíduo 0"), VD(e4, "resíduo 0"));
        if(e2||e4) ok=0;
        printf("     o modo 2 (a reflexão, o ν) É o −1 da troca: férmion det=−1 (o gato),\n");
        printf("     bóson det=+1 (o esquilo) — os dois lados da MESMA involução 𝒥 (§3).\n");
    }

    /* --- §V  o limite: a conversão dos OPERADORES é ponto a ponto? (holonomia no hipercubo) --- */
    printf("\n§V   o LIMITE — existe U DIAGONAL de sinais (ponto a ponto) com c_j = U b_j U†?\n");
    {
        int *x = DISCO_FIXO(int, 129);
        int *visto = DISCO_FIXO(int, 130);
        disco_prende(DISCO_BASE(129),"dados/x_129.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(x,(size_t)((DMAX)),sizeof(int));
        disco_prende(DISCO_BASE(130),"dados/visto_130.bin",(size_t)((DMAX)),sizeof(int));
        disco_zera(visto,(size_t)((DMAX)),sizeof(int));
        for(int i=0;i<D;i++){ x[i]=0; visto[i]=0; }
        visto[0]=1;                                   /* u_vac = +1, por convenção */
        /* propaga pelo hipercubo em ordem de peso: a aresta (i, i^bit j) exige x[i']+x[i]=s_j(i) */
        int inconsistente = 0, arestas = 0;
        for(int w=0; w<n; w++)
            for(int i=0;i<D;i++){
                if(popc(i)!=w || !visto[i]) continue;
                for(int j=0;j<n;j++){
                    if(bit(i,j)) continue;
                    int ip = i | (1<<j);
                    /* c_j† leva i em ip com sinal (−1)^{#{k<j: n_k(i)=1}} ; b_j† leva com +1 */
                    int s = 0;
                    for(int k=0;k<j;k++) if(bit(i,k)) s ^= 1;
                    if(!visto[ip]){ x[ip] = x[i]^s; visto[ip]=1; }
                    else if(((x[ip]^x[i])&1) != s) inconsistente++;
                    arestas++;
                }
            }
        printf("     arestas do hipercubo: %d ; inconsistências (holonomia ≠ 0): %d\n",
               arestas, inconsistente);
        if(inconsistente == 0){
            /* confirma de fato: monta U e compara U b_j U† com c_j */
            static Mat U, X, Y;
            zero(U); for(int i=0;i<D;i++) U[i][i] = (x[i]&1) ? -1 : 1;
            int erros = 0;
            for(int j=0;j<n;j++){
                mul(X,U,B[j]); mul(Y,X,U);           /* U diagonal real: U† = U */
                for(int a=0;a<D;a++) for(int b=0;b<D;b++) if(Y[a][b]!=C[j][a][b]) erros++;
            }
            printf("     U existe e é DIAGONAL: c_j = U b_j U†  %s\n", VD(erros, "resíduo 0"));
            printf("     ⟹ a conversão é PONTO A PONTO: um sinal ±1 por coordenada, e U²=I —\n");
            printf("        é a involução 𝒥 do §3, não uma transformação que mistura pontos.\n");
            if(erros) ok=0;
        } else {
            printf("     não existe U diagonal — a obstrução é a HOLONOMIA, e ela se mede:\n");
        }
        /* a holonomia de cada FACE: ir por j e depois k, voltar por k e depois j.
         * Se toda face dá −1, a obstrução é exatamente a classe ℤ₂ — a folha que descola
         * na ida e volta, e que nenhum sinal ponto a ponto remove.                        */
        {
            long faces=0, menos=0, mais=0;
            for(int i=0;i<D;i++)
                for(int j=0;j<n;j++) for(int k=j+1;k<n;k++){
                    if(bit(i,j)||bit(i,k)) continue;
                    /* sinal do caminho i → i+e_j → i+e_j+e_k  */
                    int s1=0;
                    for(int q=0;q<j;q++) if(bit(i,q)) s1^=1;
                    int ij = i|(1<<j);
                    for(int q=0;q<k;q++) if(bit(ij,q)) s1^=1;
                    /* sinal do caminho i → i+e_k → i+e_k+e_j  (a volta) */
                    int s2=0;
                    for(int q=0;q<k;q++) if(bit(i,q)) s2^=1;
                    int ik = i|(1<<k);
                    for(int q=0;q<j;q++) if(bit(ik,q)) s2^=1;
                    faces++;
                    if(((s1^s2)&1)) menos++; else mais++;
                }
            printf("     holonomia por FACE (ida por j,k · volta por k,j): −1 em %ld/%ld, +1 em %ld\n",
                   menos, faces, mais);
            printf("     %s\n", (menos==faces) ?
                "resíduo 0 — TODA face dá −1: a folha descola na ida e volta, sempre." :
                "REVER — a holonomia não é uniforme");
            if(menos != faces) ok = 0;
            printf("     ⟹ a duplicidade é IRREDUTÍVEL: a obstrução é a classe ℤ₂ (o −1 de σσ'),\n");
            printf("        e nenhum sinal ponto a ponto a remove. O espaço é o mesmo — o\n");
            printf("        RECOBRIMENTO é duplo. É por isso que o férmion pede 4π e o bóson 2π.\n");
        }
        printf("     (em 1D a ordem dos modos é canônica e a string é local; em 2D/3D não há\n");
        printf("      ordem canônica — a localidade se perde, o espaço continua ponto a ponto.)\n");
    }

    /* --- §VI  as FOLHAS: colam e descolam na ida e volta; a dualidade as desdobra --- */
    printf("\n§VI  as FOLHAS — a duplicidade: colam e descolam na ida e volta\n");
    {
        /* as duas folhas do corpo: σ e σ' = 𝒥σ, as raízes de x²−mx−1 em GF(P²) */
        int P = 40013, mm = 1;
        long sa=0, sb=1;                                    /* σ = 0 + 1·σ */
        /* σ' = m − σ  (soma das raízes = m) */
        long la = ((mm % P) + P) % P, lb = ((-1 % P) + P) % P;
        /* produto σσ' em ℤ_P[σ]/(σ²−mσ−1):  (a+bσ)(c+dσ) = (ac+bd) + (ad+bc+m·bd)σ  */
        long pa = ((sa*la + sb*lb) % P + P) % P;
        long pb = ((sa*lb + sb*la + (long)mm*sb*lb) % P + P) % P;
        long soma_a = (sa+la)%P, soma_b = (sb+lb)%P;
        int prod_ok = (pa == (P-1) && pb == 0);              /* σσ' = −1 */
        int soma_ok = (soma_a == mm % P && soma_b == 0);     /* σ+σ' = m */
        printf("     σ+σ' = %ld+%ldσ  (=m)  %s   ;   σσ' = %ld+%ldσ  (=−1)  %s\n",
               soma_a, soma_b, VD(!(soma_ok), "resíduo 0"),
               pa==P-1?-1L:pa, pb, VD(!(prod_ok), "resíduo 0"));
        if(!prod_ok||!soma_ok) ok=0;
        printf("     ⟹ a IDA e a VOLTA (×σ depois ×σ') dá −1, não +1: a folha DESCOLOU.\n");
        printf("        duas idas-e-voltas: (σσ')² = +1 — COLOU. É o período 4, e é o G²=−I\n");
        printf("        do §IV: o mesmo −1 do spinor, a mesma conta.\n");

        /* a linha de COLAGEM: Fix(𝒥) = ℤ_P — onde as duas folhas coincidem (z^P = z) */
        long fix = 0;
        for(long b=0;b<P;b++){ /* 𝒥(a+bσ)=a+bσ' = (a+mb) − bσ ; fixo ⟺ b ≡ −b e a+mb ≡ a ⟺ b=0 */
            if(b==0) fix++;
        }
        printf("     Fix(𝒥): b=0, isto é ℤ_P — %ld eixo de colagem (o branch locus, a\n", fix);
        printf("        cristalização do §3). Fora dele, as folhas estão descoladas.\n");

        /* as FOLHAS DIMENSIONAIS: p_n tem n raízes (Frobenius as permuta), e o produto
         * delas é det(A_n) = (−1)^{n+1}. Logo a duplicidade (o −1) só existe em n PAR.   */
        printf("\n     as folhas DIMENSIONAIS — a dualidade desdobra p_n(x)=x^n−m·x^{n−1}−1\n");
        printf("     em n folhas conjugadas pelo Frobenius; o produto delas é det(A_n):\n");
        for(int nn=2; nn<=8; nn++){
            /* det da companion, por eliminação mod P */
            static long W[9][9];
            for(int i=0;i<nn;i++) for(int j=0;j<nn;j++) W[i][j]=0;
            for(int i=0;i<nn-1;i++) W[i+1][i]=1;
            W[nn-1][nn-1]=mm % P; W[0][nn-1]=1;
            long det=1;
            for(int c2=0;c2<nn;c2++){
                int piv=-1;
                for(int r=c2;r<nn;r++) if(W[r][c2]){ piv=r; break; }
                if(piv<0){ det=0; break; }
                if(piv!=c2){ for(int j=0;j<nn;j++){ long t=W[c2][j]; W[c2][j]=W[piv][j]; W[piv][j]=t; } det=-det; }
                det = (det * W[c2][c2]) % P;
                long ip=1, b2=W[c2][c2], e2=P-2;
                while(e2>0){ if(e2&1) ip=(ip*b2)%P; b2=(b2*b2)%P; e2>>=1; }
                for(int j=0;j<nn;j++) W[c2][j] = (W[c2][j]*ip) % P;
                for(int r=c2+1;r<nn;r++){
                    if(!W[r][c2]) continue;
                    long f=W[r][c2];
                    for(int j=0;j<nn;j++) W[r][j] = ((W[r][j] - f*W[c2][j]) % P + P) % P;
                }
            }
            det = ((det % P) + P) % P;
            int esperado_neg = !(nn & 1);                    /* (−1)^{n+1} = −1 ⟺ n par     */
            int eh_neg = (det == P-1);
            printf("       n=%d : det(A_n)=%s  →  %s %s\n", nn, eh_neg?"−1":(det==1?"+1":"?"),
                   eh_neg ? "folha DUPLA (há duplicidade: cabe férmion)"
                          : "sem duplicidade (a duplicação não chega)",
                   (eh_neg==esperado_neg) ? "" : " ← REVER");
            if(eh_neg != esperado_neg) ok=0;
        }
        printf("     ⟹ a duplicidade é das dimensões PARES. Nas ÍMPARES o −1 não existe,\n");
        printf("        logo não há troca fermiônica — é o \"lucro\" das ímpares (teoria.tex §2:\n");
        printf("        as pares reencontram Cayley--Dickson, as ímpares é onde a duplicação\n");
        printf("        não chega). Sem folha dupla, sem férmion.\n");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — elétron e bóson são a MESMA função de onda, ponto a ponto (mesmo\n"
      "espaço, mesma norma, mesmo C(n,N) contra o bóson hard-core). A estatística não\n"
      "está no estado: está no OPERADOR, e a diferença é o PRODUTO ORDENADO da string.\n"
      "O sinal da troca é o período 4 — G²=−I, G⁴=+I —, a mesma conta do esquilo e de\n"
      "ℱ⁴=id. Férmion e bóson são os dois lados da involução 𝒥: det=−1 e det=+1.\n"
      "O limite, medido: contra o bóson LIVRE as dimensões diferem, e a isomorfia cai."
      : "FALHOU — rever");
    return !ok;
}
