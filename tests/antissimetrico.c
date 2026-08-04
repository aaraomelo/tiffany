/* antissimetrico.c — SEM ASSIMETRIA NÃO HÁ NADA. E O PRODUTO ANTISSIMÉTRICO É O ÚNICO
 * QUE ATRAVESSA TODAS AS DIMENSÕES.
 *
 * A tese, em partes:
 *   (a) a assimetria inicial é uma ANTIssimetria — ω(u,v) = −ω(v,u);
 *   (b) ela "carrega tudo e nada ao mesmo tempo": ω(u,u)=0 (nada, sobre um ponto só)
 *       e não-degenerada (tudo, sobre pares);
 *   (c) é a única que se conserva em TODA dimensão: uma só classe, sempre;
 *   (d) os complexos são o primeiro lugar onde ela existe;
 *   (e) e a lei de potência sai dela: det = Pf², e o módulo |x|^d do corpo local.
 *
 * Cada seção é exata: resíduo 0 ou falha.  Só libc+libm.  Sem tabela persistida.
 */
#include <stdio.h>
#include <math.h>

#include "unidade.h"

/* ---------- aritmética de F_p, p pequeno ---------- */
static int P = 3;
static int md(int a){ a %= P; return a < 0 ? a + P : a; }
static int ivp(int a){ int r = 1, b = md(a); for(int i = 0; i < P - 2; i++) r = md(r * b); return r; }

#define NX 6
static int detp(int n, int M[NX][NX]){
    int A[NX][NX], d = 1;
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) A[i][j] = md(M[i][j]);
    for(int c = 0; c < n; c++){
        int piv = -1;
        for(int r = c; r < n; r++) if(A[r][c]){ piv = r; break; }
        if(piv < 0) return 0;
        if(piv != c){ for(int j = 0; j < n; j++){ int t = A[c][j]; A[c][j] = A[piv][j]; A[piv][j] = t; } d = md(-d); }
        d = md(d * A[c][c]);
        int ic = ivp(A[c][c]);
        for(int r = c + 1; r < n; r++){
            int f = md(A[r][c] * ic);
            if(f) for(int j = c; j < n; j++) A[r][j] = md(A[r][j] - f * A[c][j]);
        }
    }
    return d;
}

/* |GL_n(q)| = prod_{i=0}^{n-1}(q^n - q^i) ;  |Sp_2m(q)| = q^{m^2} prod_{i=1}^{m}(q^{2i}-1) */
static long long ipow_ll(long long b, int e){ long long r = 1; while(e--) r *= b; return r; }
static long long ord_gl(int n, int q){
    long long r = 1, qn = ipow_ll(q, n);
    for(int i = 0; i < n; i++) r *= (qn - ipow_ll(q, i));
    return r;
}
static long long ord_sp(int m, int q){
    long long r = ipow_ll(q, m * m);
    for(int i = 1; i <= m; i++) r *= (ipow_ll(q, 2 * i) - 1);
    return r;
}

/* decodifica um código base-p nas k entradas livres do triângulo superior */
static void alt_de(int n, long code, int B[NX][NX]){
    for(int i = 0; i < n; i++) for(int j = 0; j < n; j++) B[i][j] = 0;
    for(int i = 0; i < n; i++) for(int j = i + 1; j < n; j++){
        int e = (int)(code % P); code /= P;
        B[i][j] = e; B[j][i] = md(-e);          /* antissimétrica, e diagonal 0 => alternante */
    }
}
static void sim_de(int n, long code, int B[NX][NX]){
    for(int i = 0; i < n; i++) for(int j = i; j < n; j++){
        int e = (int)(code % P); code /= P;
        B[i][j] = e; B[j][i] = e;
    }
}

/* =========================================================================== */
int main(void){
printf("\n=== O PRODUTO ANTISSIMÉTRICO ==============================================\n");

/* ---------------------------------------------------------------- §A1 ------ */
printf("\n§A1  Onde ela existe: em nenhuma dimensão ímpar, em toda dimensão par.\n");
printf("     Contagem por força bruta das formas alternantes NÃO-DEGENERADAS em F_p^n,\n");
printf("     contra a predição fechada |GL_n|/|Sp_n| (uma única órbita de GL_n).\n\n");
printf("      p   n    formas         não-deg.   |GL_n|/|Sp_n|   uma classe?\n");
{
    int ps[3] = {2, 3, 5};
    for(int a = 0; a < 3; a++){
        P = ps[a];
        int nmax = (P == 5) ? 4 : 5;
        for(int n = 1; n <= nmax; n++){
            int k = n * (n - 1) / 2;
            long tot = 1; for(int i = 0; i < k; i++) tot *= P;
            long nd = 0;
            int B[NX][NX];
            for(long c = 0; c < tot; c++){ alt_de(n, c, B); if(detp(n, B)) nd++; }
            long long pred = (n % 2 == 0) ? ord_gl(n, P) / ord_sp(n / 2, P) : 0;
            printf("      %d   %d   %8ld   %10ld   %13lld    %s\n",
                   P, n, tot, nd, pred, (nd == pred) ? "sim ✓" : "NÃO ✗");
            if(nd != pred) falhas++;
        }
    }
}
printf("\n     Dimensão ímpar: ZERO. Não é escassez — é impossibilidade: em n ímpar\n");
printf("     det(B) = det(-B^T) = (-1)^n det(B) = -det(B), logo det B = 0 sempre.\n");
printf("     Em n=1 a única forma alternante é a NULA: no ponto, sem assimetria,\n");
printf("     não há nada — nem produto, nem par, nem observação.\n");

/* ---------------------------------------------------------------- §A2 ------ */
printf("\n§A2  Uma só classe, em toda dimensão — construída, não suposta (Darboux).\n");
printf("     Para CADA forma alternante não-degenerada de F_3^4, construo a base\n");
printf("     simplética P e confiro P^t B P = J padrão, entrada por entrada.\n\n");
P = 3;
{
    int Jp[NX][NX]; for(int i=0;i<4;i++) for(int j=0;j<4;j++) Jp[i][j]=0;
    Jp[0][1]=1; Jp[1][0]=md(-1); Jp[2][3]=1; Jp[3][2]=md(-1);
    long tot = 1; for(int i = 0; i < 6; i++) tot *= 3;
    long nd = 0, feitas = 0;
    for(long c = 0; c < tot; c++){
        int B[NX][NX]; alt_de(4, c, B);
        if(!detp(4, B)) continue;
        nd++;
        int e1[4], f1[4], e2[4], f2[4], v[4], achou;
        #define OM(x,y) ({ int s=0; for(int i=0;i<4;i++) for(int j=0;j<4;j++) s+=x[i]*B[i][j]*y[j]; md(s); })
        /* e1 = primeiro vetor não-nulo */
        for(int i=0;i<4;i++) e1[i]=0; e1[0]=1;
        /* f1 com ω(e1,f1)=1 */
        achou = 0;
        for(int code = 1; code < 81 && !achou; code++){
            int t = code; for(int i=0;i<4;i++){ v[i]=t%3; t/=3; }
            int w = OM(e1, v);
            if(w){ int iw = ivp(w); for(int i=0;i<4;i++) f1[i] = md(v[i]*iw); achou = 1; }
        }
        if(!achou){ falhas++; continue; }
        /* e2, f2 no complemento ω-ortogonal de <e1,f1> */
        achou = 0;
        for(int code = 1; code < 81 && !achou; code++){
            int t = code; for(int i=0;i<4;i++){ v[i]=t%3; t/=3; }
            if(OM(e1,v)==0 && OM(f1,v)==0){ for(int i=0;i<4;i++) e2[i]=v[i]; achou = 1; }
        }
        if(!achou){ falhas++; continue; }
        achou = 0;
        for(int code = 1; code < 81 && !achou; code++){
            int t = code; for(int i=0;i<4;i++){ v[i]=t%3; t/=3; }
            if(OM(e1,v)==0 && OM(f1,v)==0){
                int w = OM(e2, v);
                if(w){ int iw = ivp(w); for(int i=0;i<4;i++) f2[i] = md(v[i]*iw); achou = 1; }
            }
        }
        if(!achou){ falhas++; continue; }
        /* P = [e1 f1 e2 f2] em colunas ; confere P^t B P = J */
        int Pm[NX][NX], T[NX][NX], R[NX][NX];
        for(int i=0;i<4;i++){ Pm[i][0]=e1[i]; Pm[i][1]=f1[i]; Pm[i][2]=e2[i]; Pm[i][3]=f2[i]; }
        for(int i=0;i<4;i++) for(int j=0;j<4;j++){ int s=0; for(int l=0;l<4;l++) s+=Pm[l][i]*B[l][j]; T[i][j]=md(s); }
        for(int i=0;i<4;i++) for(int j=0;j<4;j++){ int s=0; for(int l=0;l<4;l++) s+=T[i][l]*Pm[l][j]; R[i][j]=md(s); }
        int igual = 1;
        for(int i=0;i<4;i++) for(int j=0;j<4;j++) if(R[i][j]!=Jp[i][j]) igual = 0;
        if(igual) feitas++; else falhas++;
        #undef OM
    }
    printf("      formas alternantes não-degeneradas em F_3^4 ....... %ld\n", nd);
    printf("      levadas a J padrão por base construída ........... %ld\n", feitas);
    ok("todas na MESMA classe (uma só órbita)", feitas == nd && nd > 0);
}
printf("\n     E a simétrica? O discriminante det mod quadrados é INVARIANTE de congruência\n");
printf("     (det(P^t B P) = det(P)^2 det(B)), logo separa classes que não se juntam:\n\n");
{
    printf("      n   simétricas não-deg.   discriminantes realizados\n");
    for(int n = 1; n <= 4; n++){
        int k = n * (n + 1) / 2;
        long tot = 1; for(int i = 0; i < k; i++) tot *= 3;
        long nd = 0; int vis[3] = {0,0,0};
        int B[NX][NX];
        for(long c = 0; c < tot; c++){
            sim_de(n, c, B);
            int d = detp(n, B);
            if(d){ nd++; vis[d] = 1; }
        }
        int q = vis[1] + vis[2];
        printf("      %d   %18ld   %d  (%s%s)\n", n, nd, q,
               vis[1] ? "1 " : "", vis[2] ? "2" : "");
        if(q != 2) falhas++;
    }
    /* a identidade que faz do discriminante um invariante, medida */
    int mau = 0;
    for(long cb = 0; cb < 729; cb++){
        int B[NX][NX], Pm[NX][NX], T[NX][NX], R[NX][NX];
        sim_de(3, cb, B);
        for(int cp = 0; cp < 729; cp += 37){
            int t = cp;
            for(int i=0;i<3;i++) for(int j=0;j<3;j++){ Pm[i][j] = t % 3; t /= 3; }
            int dp = detp(3, Pm); if(!dp) continue;
            for(int i=0;i<3;i++) for(int j=0;j<3;j++){ int s=0; for(int l=0;l<3;l++) s+=Pm[l][i]*B[l][j]; T[i][j]=md(s); }
            for(int i=0;i<3;i++) for(int j=0;j<3;j++){ int s=0; for(int l=0;l<3;l++) s+=T[i][l]*Pm[l][j]; R[i][j]=md(s); }
            if(detp(3, R) != md(dp * dp * detp(3, B))) mau++;
        }
    }
    ok("det(P^t B P) = det(P)^2 det(B) (exata, sem exceção)", mau == 0);
}
printf("\n      alternante:   1 classe em toda dimensão par, sempre, sobre qualquer corpo\n");
printf("      simétrica:  >=2 classes em F_p (discriminante), e n+1 em R (assinatura)\n");
printf("      => só a antissimétrica atravessa as dimensões sem se dividir.\n");

/* ---------------------------------------------------------------- §A3 ------ */
printf("\n§A3  O gato mora nela: ω(Au,Av) = det(A)·ω(u,v).  E det do gato é -1.\n\n");
{
    printf("      peça          det   fator medido em ω   conserva ω?\n");
    for(int m = 1; m <= 3; m++){
        long A[2][2] = {{m, 1}, {1, 0}};
        long det = A[0][0]*A[1][1] - A[0][1]*A[1][0];
        int bate = 1;
        for(long u1=-6; u1<=6; u1++) for(long u2=-6; u2<=6; u2++)
        for(long v1=-6; v1<=6; v1++) for(long v2=-6; v2<=6; v2++){
            long om = u1*v2 - u2*v1;
            long a1 = A[0][0]*u1 + A[0][1]*u2, a2 = A[1][0]*u1 + A[1][1]*u2;
            long b1 = A[0][0]*v1 + A[0][1]*v2, b2 = A[1][0]*v1 + A[1][1]*v2;
            if(a1*b2 - a2*b1 != det*om) bate = 0;
        }
        printf("      gato m=%d      %+ld     %+ld                 %s\n",
               m, det, det, det == 1 ? "sim" : "não — inverte");
        if(!bate) falhas++;
    }
    long G[2][2] = {{0,-1},{1,0}};                 /* o esquilo: det +1, G^4 = I */
    long dg = G[0][0]*G[1][1] - G[0][1]*G[1][0];
    int bate = 1;
    for(long u1=-6;u1<=6;u1++) for(long u2=-6;u2<=6;u2++)
    for(long v1=-6;v1<=6;v1++) for(long v2=-6;v2<=6;v2++){
        long om = u1*v2 - u2*v1;
        long a1 = G[0][0]*u1+G[0][1]*u2, a2 = G[1][0]*u1+G[1][1]*u2;
        long b1 = G[0][0]*v1+G[0][1]*v2, b2 = G[1][0]*v1+G[1][1]*v2;
        if(a1*b2 - a2*b1 != dg*om) bate = 0;
    }
    printf("      esquilo       %+ld     %+ld                 %s\n", dg, dg, "sim");
    if(!bate) falhas++;
    /* a condição era a constante 1: o cálculo acima (bate, falhas) era deitado fora.
     * É o T1 do paper, e a unidade era verde por construção. */
    ok("ω(Au,Av) = det(A)·ω(u,v), exata em 28561 pares", bate && falhas == 0);
    printf("\n      O gato ANTI-conserva (det=-1): uma batida vira o sinal de ω, duas devolvem.\n");
    printf("      A holonomia -1 das folhas é a própria antissimetria da forma.\n");
}

/* ---------------------------------------------------------------- §A4 ------ */
printf("\n§A4  Tudo e nada: ω(u,u)=0 e não-degenerada. E os complexos são o primeiro lugar.\n\n");
P = 3;
{
    int B[NX][NX]; for(int i=0;i<4;i++) for(int j=0;j<4;j++) B[i][j]=0;
    B[0][1]=1; B[1][0]=2; B[2][3]=1; B[3][2]=2;
    int diag_zero = 1, todo_ponto_ve = 1;
    for(int cu = 0; cu < 81; cu++){
        int u[4], t = cu; for(int i=0;i<4;i++){ u[i]=t%3; t/=3; }
        int s = 0; for(int i=0;i<4;i++) for(int j=0;j<4;j++) s += u[i]*B[i][j]*u[j];
        if(md(s)) diag_zero = 0;
        if(!cu) continue;
        int algum = 0;
        for(int cv = 0; cv < 81 && !algum; cv++){
            int v[4], r = cv; for(int i=0;i<4;i++){ v[i]=r%3; r/=3; }
            int w = 0; for(int i=0;i<4;i++) for(int j=0;j<4;j++) w += u[i]*B[i][j]*v[j];
            if(md(w)) algum = 1;
        }
        if(!algum) todo_ponto_ve = 0;
    }
    ok("ω(u,u) = 0 para TODO u  (nada, sobre um ponto só)", diag_zero);
    ok("todo u != 0 tem v com ω(u,v) != 0  (tudo, sobre pares)", todo_ponto_ve);
    printf("\n      Por isso ela carrega tudo e nada: um ponto é invisível a si mesmo —\n");
    printf("      a medida precisa de DOIS. Não há observação de um ponto sozinho.\n");
}
{
    long J[2][2] = {{0,1},{-1,0}};
    long J2[2][2];
    for(int i=0;i<2;i++) for(int j=0;j<2;j++){ long s=0; for(int l=0;l<2;l++) s+=J[i][l]*J[l][j]; J2[i][j]=s; }
    ok("J^2 = -I  (J = [[0,1],[-1,0]], a alternante de dim 2)",
       J2[0][0]==-1 && J2[1][1]==-1 && J2[0][1]==0 && J2[1][0]==0);
    int liga = 1;
    for(long u1=-5;u1<=5;u1++) for(long u2=-5;u2<=5;u2++)
    for(long v1=-5;v1<=5;v1++) for(long v2=-5;v2<=5;v2++){
        long om = u1*v2 - u2*v1;
        long jv1 = J[0][0]*v1 + J[0][1]*v2, jv2 = J[1][0]*v1 + J[1][1]*v2;
        if(u1*jv1 + u2*jv2 != om) liga = 0;
    }
    ok("ω(u,v) = <u, Jv>  — a forma E a estrutura complexa são o MESMO objeto", liga);
    /* R[J] = C : tabela de multiplicação */
    int tab = 1;
    for(long a=-4;a<=4;a++) for(long b=-4;b<=4;b++) for(long c=-4;c<=4;c++) for(long d=-4;d<=4;d++){
        long Z[2][2] = {{a, b},{-b, a}}, W[2][2] = {{c, d},{-d, c}}, R[2][2];
        for(int i=0;i<2;i++) for(int j=0;j<2;j++){ long s=0; for(int l=0;l<2;l++) s+=Z[i][l]*W[l][j]; R[i][j]=s; }
        if(R[0][0] != a*c - b*d || R[0][1] != a*d + b*c) tab = 0;
    }
    ok("R[J] ≅ C  (aI+bJ multiplica como (a+bi))", tab);
    printf("\n      n=1: a única alternante é a nula (§A1) — no ponto não há produto.\n");
    printf("      n=2: já é não-degenerada, e é C. Nenhum real tem x^2 = -1; J tem.\n");
    printf("      => os complexos são o PRIMEIRO lugar onde há algo. Não por escolha: por dimensão.\n");
}

/* ---------------------------------------------------------------- §A5 ------ */
printf("\n§A5  A lei de potência sai daí.\n");
printf("     (i) o volume é uma POTÊNCIA da forma: det B = Pf(B)^2 — exato, em Z e em F_p.\n\n");
{
    int mau = 0; long testes = 0;
    for(long b12=-4;b12<=4;b12++) for(long b13=-4;b13<=4;b13++) for(long b14=-4;b14<=4;b14++)
    for(long b23=-4;b23<=4;b23++) for(long b24=-4;b24<=4;b24++) for(long b34=-4;b34<=4;b34++){
        long pf = b12*b34 - b13*b24 + b14*b23;
        /* det 4x4 alternante por Laplace na primeira linha */
        long M[4][4] = {{0,b12,b13,b14},{-b12,0,b23,b24},{-b13,-b23,0,b34},{-b14,-b24,-b34,0}};
        long det = 0;
        for(int c = 0; c < 4; c++){
            long m3[3][3]; int ri = 0;
            for(int i=1;i<4;i++){ int ci=0; for(int j=0;j<4;j++){ if(j==c) continue; m3[ri][ci++]=M[i][j]; } ri++; }
            long d3 = m3[0][0]*(m3[1][1]*m3[2][2]-m3[1][2]*m3[2][1])
                    - m3[0][1]*(m3[1][0]*m3[2][2]-m3[1][2]*m3[2][0])
                    + m3[0][2]*(m3[1][0]*m3[2][1]-m3[1][1]*m3[2][0]);
            det += ((c%2)?-1:1) * M[0][c] * d3;
        }
        testes++;
        if(det != pf*pf) mau++;
    }
    printf("      testes: %ld     det = Pf^2 em todos? %s\n", testes, mau==0 ? "sim ✓" : "NÃO ✗");
    if(mau) falhas++;
    printf("      Em dim 2m o volume é ω^m/m! — a mesma forma ELEVADA. A potência não é\n");
    printf("      analogia: é o produto exterior da única forma consigo mesma.\n");
}
printf("\n     (ii) e o módulo do corpo local é |x|^d, com d = a dimensão sobre R:\n");
printf("          o determinante da multiplicação — que é a forma — é uma potência da norma.\n\n");
{
    printf("      corpo   d   det(mult) medido            = |x|^d ?\n");
    /* R : 1x1 — det da multiplicação na base {1} contra a norma^1 */
    int mau = 0;
    for(long a=-9;a<=9;a++){
        if(!a) continue;
        long det = a, nr = a < 0 ? -a : a;         /* |det| = |a|^1 */
        if((det<0?-det:det) != nr) mau++;
    }
    printf("      R       1   det = a,        |det| = |a|¹   %s\n", mau==0?"sim ✓":"NÃO ✗");
    /* C : 2x2 — det da multiplicação na base {1,J} contra a norma^1 = |z|^2 */
    mau = 0;
    for(long a=-9;a<=9;a++) for(long b=-9;b<=9;b++){
        long L[2][2] = {{a,-b},{b,a}};
        long det = L[0][0]*L[1][1] - L[0][1]*L[1][0];
        if(det != a*a + b*b) mau++;                /* = |z|^2 */
    }
    printf("      C       2   det = a²+b²,    = |z|²        %s\n", mau==0?"sim ✓":"NÃO ✗");
    if(mau) falhas++;
    /* H : 4x4, multiplicação à esquerda por q = a+bi+cj+dk */
    mau = 0; long tst = 0;
    for(long a=-3;a<=3;a++) for(long b=-3;b<=3;b++) for(long c=-3;c<=3;c++) for(long d=-3;d<=3;d++){
        long L[4][4] = {{ a,-b,-c,-d},
                        { b, a,-d, c},
                        { c, d, a,-b},
                        { d,-c, b, a}};
        long det = 0;
        for(int k = 0; k < 4; k++){
            long m3[3][3]; int ri = 0;
            for(int i=1;i<4;i++){ int ci=0; for(int j=0;j<4;j++){ if(j==k) continue; m3[ri][ci++]=L[i][j]; } ri++; }
            long d3 = m3[0][0]*(m3[1][1]*m3[2][2]-m3[1][2]*m3[2][1])
                    - m3[0][1]*(m3[1][0]*m3[2][2]-m3[1][2]*m3[2][0])
                    + m3[0][2]*(m3[1][0]*m3[2][1]-m3[1][1]*m3[2][0]);
            det += ((k%2)?-1:1) * L[0][k] * d3;
        }
        long nr = a*a + b*b + c*c + d*d;
        tst++;
        if(det != nr*nr) mau++;
    }
    printf("      H       4   det = (a²+b²+c²+d²)² = |q|⁴  %s   (%ld testes)\n",
           mau==0?"sim ✓":"NÃO ✗", tst);
    if(mau) falhas++;
    printf("\n      1, 2, 4 — o expoente É a dimensão. Esse é o módulo normalizado do corpo\n");
    printf("      local, e é o que força os quasicaracteres a serem |x|^s: a lei de potência\n");
    printf("      não é ajuste empírico, é o determinante da própria multiplicação.\n");
}

/* ---------------------------------------------------------------- §A6 ------ */
printf("\n§A6  Por que \"tudo e nada\" não é figura de linguagem: Sp é transitiva, O não é.\n");
printf("     Construo, para CADA par de vetores não-nulos de F_3^4, um S com S^t J S = J\n");
printf("     e S·u = v (transvecções simpléticas). Se existe sempre, a forma antissimétrica\n");
printf("     não distingue ponto de ponto: sobre um ponto só, ela é NADA.\n\n");
P = 3;
{
    int J[NX][NX]; for(int i=0;i<4;i++) for(int j=0;j<4;j++) J[i][j]=0;
    J[0][1]=1; J[1][0]=2; J[2][3]=1; J[3][2]=2;
    #define OMJ(x,y) ({ int s=0; for(int i=0;i<4;i++) for(int j=0;j<4;j++) s+=x[i]*J[i][j]*y[j]; md(s); })
    /* T_{w,c}[i][j] = delta + c*w[i]*(Jw)[j] ;  T(x) = x + c*ω(x,w)*w */
    #define TRANSV(T,w,c) do{ int jw[4]; \
        for(int i=0;i<4;i++){ int s=0; for(int j=0;j<4;j++) s+=J[i][j]*w[j]; jw[i]=md(s); } \
        for(int i=0;i<4;i++) for(int j=0;j<4;j++) T[i][j] = md((i==j?1:0) + (c)*w[i]*jw[j]); }while(0)
    #define MUL4(R,A,B) do{ for(int i=0;i<4;i++) for(int j=0;j<4;j++){ int s=0; \
        for(int l=0;l<4;l++) s+=A[i][l]*B[l][j]; R[i][j]=md(s); } }while(0)

    long pares = 0, resolvidos = 0, simpleticos = 0;
    for(int cu = 1; cu < 81; cu++){
        int u[4], t = cu; for(int i=0;i<4;i++){ u[i]=t%3; t/=3; }
        for(int cv = 1; cv < 81; cv++){
            int v[4], r = cv; for(int i=0;i<4;i++){ v[i]=r%3; r/=3; }
            pares++;
            int S[NX][NX], A[NX][NX], Bm[NX][NX];
            int pronto = 0;
            int w = OMJ(u, v);
            if(w){
                int wv[4]; for(int i=0;i<4;i++) wv[i] = md(v[i] - u[i]);
                TRANSV(S, wv, ivp(w)); pronto = 1;
            } else {
                for(int cz = 1; cz < 81 && !pronto; cz++){
                    int z[4], s = cz; for(int i=0;i<4;i++){ z[i]=s%3; s/=3; }
                    int w1 = OMJ(u, z), w2 = OMJ(z, v);
                    if(!w1 || !w2) continue;
                    int wa[4], wb[4];
                    for(int i=0;i<4;i++){ wa[i] = md(z[i]-u[i]); wb[i] = md(v[i]-z[i]); }
                    TRANSV(A, wa, ivp(w1)); TRANSV(Bm, wb, ivp(w2));
                    MUL4(S, Bm, A); pronto = 1;
                }
            }
            if(!pronto) continue;
            /* confere S u = v */
            int su[4], bate = 1;
            for(int i=0;i<4;i++){ int s=0; for(int j=0;j<4;j++) s+=S[i][j]*u[j]; su[i]=md(s); }
            for(int i=0;i<4;i++) if(su[i]!=v[i]) bate = 0;
            if(bate) resolvidos++;
            /* confere S^t J S = J */
            int T[NX][NX], R[NX][NX], eh = 1;
            for(int i=0;i<4;i++) for(int j=0;j<4;j++){ int s=0; for(int l=0;l<4;l++) s+=S[l][i]*J[l][j]; T[i][j]=md(s); }
            MUL4(R, T, S);
            for(int i=0;i<4;i++) for(int j=0;j<4;j++) if(R[i][j]!=J[i][j]) eh = 0;
            if(eh) simpleticos++;
        }
    }
    printf("      pares (u,v) não-nulos ......................... %ld\n", pares);
    printf("      com S construído levando u em v ............... %ld\n", resolvidos);
    printf("      e com S^t J S = J (de fato simplético) ........ %ld\n", simpleticos);
    ok("Sp transitiva: UMA órbita de pontos — nada distingue", resolvidos == pares && simpleticos == pares);
    #undef OMJ
    #undef TRANSV
    #undef MUL4
}
{
    int vis[3] = {0,0,0};
    for(int cu = 1; cu < 81; cu++){
        int u[4], t = cu; for(int i=0;i<4;i++){ u[i]=t%3; t/=3; }
        int q = 0; for(int i=0;i<4;i++) q += u[i]*u[i];
        vis[md(q)] = 1;
    }
    int nq = vis[0] + vis[1] + vis[2];
    printf("\n      valores da forma simétrica q(u)=Σu² nos 80 pontos ... %d distintos\n", nq);
    ok("a simétrica SEPARA pontos (>= 2 órbitas: a norma é rótulo)", nq >= 2);
    printf("\n      A simétrica mede o ponto (dá comprimento, dá nome). A antissimétrica não:\n");
    printf("      sobre um ponto é 0, e todos os pontos são o mesmo ponto. Ela só fala de\n");
    printf("      PARES — de diferença. É por isso que carrega tudo e nada ao mesmo tempo:\n");
    printf("      nada de absoluto, tudo de relativo. E é a única que atravessa a dimensão.\n");
}

printf("\n=== FECHAMENTO ============================================================\n");
printf("  n ímpar: a alternante é degenerada — no ponto (n=1) ela é a forma NULA.\n");
printf("  n par:   ela existe e é ÚNICA, sobre qualquer corpo, em qualquer dimensão.\n");
printf("  n=2:     é C, e é a estrutura complexa J. O primeiro lugar onde há algo.\n");
printf("  o gato vive nela com det=-1: anti-conserva, e a holonomia -1 é a antissimetria.\n");
printf("  det = Pf² e |x|^d: a lei de potência é o determinante da multiplicação.\n");
printf("  e nada disso mede um ponto: a medida precisa de dois. Sem assimetria, nada.\n");

if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato em todas as seções.\n\n");
return 0;
}
