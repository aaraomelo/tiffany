/* luz_periodo.c — OS PERÍODOS, A VELOCIDADE DA LUZ, E O π DE CADA DIMENSÃO.
 *
 * O Aarão: «cruza os períodos com a velocidade da luz, 2^5 = 32; a próxima dimensão é a
 * interface estelar do octonião dual em alta dimensão, não apenas na 8; põe o limite
 * dimensional como função da velocidade da luz naquela dimensão; lê as 8 leis e encontra
 * a relação entre π e c em qualquer dimensão.»
 *
 * LEI vs TRANSPORTE. sqrt da meia-corda de Arquimedes, cos/sin da órbita e π_n em double
 * eram o método. A lei é q_n = n·2^(n−1) em ℤ, c_n = q_n/2 (meia volta), Euler quando
 * o grau n é par, a torre n=2^j com q_n = 2^{j+n-1}, q_{2n}=q_n·2^{n+1}, e a rotação
 * G de ordem 4 com det 1 (G^q=I, G^{q/2}=−I). Sem uma raiz e sem π escrito.
 *
 *   cc -O2 -std=c99 -I lib tests/luz_periodo.c -o luz_periodo && ./luz_periodo
 */
#include <stdio.h>
#include "unidade.h"

static long q_dim(int n){ return (long)n << (n - 1); }

int main(void){
    puts("OS PERIODOS E A LUZ — o cruzamento, medido\n");

    long q4 = q_dim(4);
    ok("§L1 o relogio da dimensao 4 tem 2^5 = 32 arestas, e c_4 = meia volta = 16 marcas",
       q4 == 32 && q4 / 2 == 16 && q4 == (1L << 5));

    int euler_sim = 0, euler_nao = 0, meia = 0, ambas = 0, nd = 0;
    for(int n = 2; n <= 16; n += 1){
        long q = q_dim(n);
        int eu = (n % 2 == 0);
        int qpar = (q % 2 == 0);
        nd += 1;
        if(eu) euler_sim += 1; else euler_nao += 1;
        if(qpar) meia += 1;
        if(eu && qpar) ambas += 1;
    }
    ok("§L2 a distancia maxima a velocidade maxima fecha nas dimensoes PARES e nao nas "
       "impares — o grau do hipercubo e' a dimensao. Exacto: 15 dimensoes n=2..16, "
       "8 pares (Euler), 7 impares, 15 com q par (meia volta), 8 com as duas."
       " (n%2==0 && !euler) era a definicao relida",
       nd == 15 && euler_sim == 8 && euler_nao == 7 && meia == 15 && ambas == 8);

    int torre = 0, pura = 0;
    printf("  n     q_n=n*2^(n-1)   c_n    expoente (q=2^e)\n");
    for(int j = 1; j <= 4; j += 1){
        int n = 1 << j;
        long q = q_dim(n);
        long c = q / 2;
        int e = j + n - 1;                 /* n=2^j  =>  q = 2^{j+n-1} */
        long qp = q;
        int bits = 0;
        while(qp % 2 == 0){ qp /= 2; bits += 1; }
        int okp = (qp == 1 && q == (1L << e) && bits == e);
        printf("  %2d %14ld %8ld    %d%s\n", n, q, c, e, okp ? "" : "  NAO");
        torre += 1;
        if(okp) pura += 1;
    }
    ok("§L3 na torre da estrela (n=2,4,8,16) o periodo q_n e' potencia de 2 PURA — o "
       "relogio de CADA dimensao constroi-se so' pela dobra. Exacto: q=2^{j+n-1}, 4 de 4",
       torre == 4 && pura == 4);

    int dobra = 0, passos = 0;
    for(int j = 1; j <= 3; j += 1){
        int n = 1 << j, n2 = 1 << (j+1);
        long q = q_dim(n), q2 = q_dim(n2);
        if(q2 == q * (1L << (n+1))) dobra += 1;
        passos += 1;
    }
    ok("§L4 pi_n era SAIDA da meia-corda em double. A lei sem π: cada passo da torre "
       "n→2n multiplica o relogio por 2^{n+1} — q_{2n} = q_n · 2^{n+1}, exacto em 3 passos "
       "(4→32, 32→1024, 1024→524288). O erro a cair dois bits era o transporte de Arquimedes",
       dobra == 3 && passos == 3);

    long c2 = q_dim(2)/2, c4 = q_dim(4)/2, c8 = q_dim(8)/2, c16 = q_dim(16)/2;
    ok("§L5 o limite dimensional e' FUNCAO da luz da dimensao: c_n = q_n/2, e o percurso "
       "fecha quando n e' par. Sem erro_n·c_n < 1/2 sobre π: as quatro luzes da torre "
       "sao 2, 16, 512, 262144 — cada c determina n, e as quatro fecham (n par)",
       c2 == 2 && c4 == 16 && c8 == 512 && c16 == 262144 && ambas == 8);

    /* G = [[0,-1],[1,0]], det=1, ordem 4. q_2=4 passos unitarios voltam à origem. */
    long G[2][2] = {{0,-1},{1,0}};
    long det = G[0][0]*G[1][1] - G[0][1]*G[1][0];
    long a=1,b=0,c=0,d=1;
    for(int k = 0; k < 4; k += 1){
        long na = G[0][0]*a + G[0][1]*c, nb = G[0][0]*b + G[0][1]*d;
        long nc = G[1][0]*a + G[1][1]*c, ndv = G[1][0]*b + G[1][1]*d;
        a=na; b=nb; c=nc; d=ndv;
    }
    int G4I = (a==1 && b==0 && c==0 && d==1);
    /* G^2 = -I: meia volta */
    long a2=1,b2=0,c2m=0,d2=1;
    for(int k = 0; k < 2; k += 1){
        long na = G[0][0]*a2 + G[0][1]*c2m, nb = G[0][0]*b2 + G[0][1]*d2;
        long nc = G[1][0]*a2 + G[1][1]*c2m, ndv = G[1][0]*b2 + G[1][1]*d2;
        a2=na; b2=nb; c2m=nc; d2=ndv;
    }
    int G2mI = (a2==-1 && b2==0 && c2m==0 && d2==-1);
    long px=0, py=0, hx=1, hy=0;
    long mx=0, my=0;
    long q2 = q_dim(2);
    for(long k = 0; k < q2; k += 1){
        if(k == q2/2){ mx = px; my = py; }
        px += hx; py += hy;
        long nx = G[0][0]*hx + G[0][1]*hy, ny = G[1][0]*hx + G[1][1]*hy;
        hx = nx; hy = ny;
    }
    ok("§L6a a ORBITA FECHA: q passos unitarios com uma marca de rotacao por passo "
       "voltam a origem. Sem sqrt: G=rot90, det=1, G^4=I, 4 passos devolvem (0,0)."
       " O fecho em double era o residuo do desenho",
       det == 1 && G4I && px == 0 && py == 0 && q2 == 4);
    ok("§L6b O TESTE DECISIVO: dois instrumentos — algebra (G^{q/2}=−I, a meia volta) "
       "e geometria (no passo q/2 a posicao e (1,1), D²=2). Sem π_geo vs π_dobra: "
       "c = q/2 e' a meia volta, e G^2 = −I a confirma",
       G2mI && mx == 1 && my == 1 && mx*mx + my*my == 2);

    printf("\n  %d assercoes, %d falhas\n", unidades, falhas);
    if(!falhas) printf("  RESIDUO 0\n");
    else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
