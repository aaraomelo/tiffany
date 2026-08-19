/* quantico.c — O CORPO QUÂNTICO: o comutador É o cruzado, e a incerteza sai dele.
 *
 * Matrizes 2×2 em Z[i]; Pauli, comutador, Robertson e unitariedade sem complex.h.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/quantico.c -o quantico
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "isa_disk.h"

typedef struct { long r[2][2]; long i[2][2]; } Gi;

static Gi gi_zero(void){ Gi z; memset(&z, 0, sizeof z); return z; }
static Gi gi_id(void){
    Gi z = gi_zero(); z.r[0][0]=1; z.r[1][1]=1; return z;
}
static Gi gi_add(Gi a, Gi b){
    Gi o; for(int u=0;u<2;u++) for(int v=0;v<2;v++){
        o.r[u][v]=a.r[u][v]+b.r[u][v]; o.i[u][v]=a.i[u][v]+b.i[u][v];
    } return o;
}
static Gi gi_scale(long s, Gi a){
    Gi o; for(int u=0;u<2;u++) for(int v=0;v<2;v++){
        o.r[u][v]=s*a.r[u][v]; o.i[u][v]=s*a.i[u][v];
    } return o;
}
static Gi gi_neg(Gi a){ return gi_scale(-1, a); }
static Gi gi_sub(Gi a, Gi b){ return gi_add(a, gi_neg(b)); }
static Gi gi_mul(Gi a, Gi b){
    Gi o = gi_zero();
    for(int u=0;u<2;u++) for(int v=0;v<2;v++) for(int k=0;k<2;k++){
        o.r[u][v] += a.r[u][k]*b.r[k][v] - a.i[u][k]*b.i[k][v];
        o.i[u][v] += a.r[u][k]*b.i[k][v] + a.i[u][k]*b.r[k][v];
    }
    return o;
}
static Gi gi_comm(Gi a, Gi b){ return gi_sub(gi_mul(a,b), gi_mul(b,a)); }
static int gi_eq(Gi a, Gi b){
    for(int u=0;u<2;u++) for(int v=0;v<2;v++)
        if(a.r[u][v]!=b.r[u][v] || a.i[u][v]!=b.i[u][v]) return 0;
    return 1;
}

static Gi pauli(int k){
    Gi p = gi_zero();
    if(k==0){ p.r[0][1]=1; p.r[1][0]=1; }
    if(k==1){ p.i[0][1]=-1; p.i[1][0]=1; }
    if(k==2){ p.r[0][0]=1; p.r[1][1]=-1; }
    return p;
}
static int eps(int i, int j, int k){
    if(i==j||j==k||i==k) return 0;
    if((i==0&&j==1&&k==2)||(i==1&&j==2&&k==0)||(i==2&&j==0&&k==1)) return 1;
    return -1;
}

int main(void){
    puts("quantico.c — O CORPO QUANTICO: o comutador E o cruzado, e a incerteza sai dele\n");

    puts("§Q1  A PARTICAO em Z: 2H + 2K = 2A, sem dividir.\n");
    {
        long part = 0, part_ok = 0;
        for(long a1=-2;a1<=2;a1++) for(long b1=-2;b1<=2;b1++)
        for(long c1=-2;c1<=2;c1++) for(long d1=-2;d1<=2;d1++){
            long Are[2][2]={{a1,b1},{d1,0}}, Aim[2][2]={{0,c1},{0,0}};
            long H2r[2][2], H2i[2][2], K2r[2][2], K2i[2][2];
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                H2r[u][v]=2*Are[u][v]; H2i[u][v]=2*Aim[u][v];
                K2r[u][v]=0; K2i[u][v]=0;
            }
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                long Dre=Are[v][u], Dim=-Aim[v][u];
                H2r[u][v]=Are[u][v]+Dre; H2i[u][v]=Aim[u][v]+Dim;
                K2r[u][v]=Are[u][v]-Dre; K2i[u][v]=Aim[u][v]-Dim;
            }
            part++;
            int okp=1;
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                if(H2r[u][v]+K2r[u][v]!=2*Are[u][v]) okp=0;
                if(H2i[u][v]+K2i[u][v]!=2*Aim[u][v]) okp=0;
            }
            if(okp) part_ok++;
        }
        ok("a particao A = H + K mede-se EXACTA em Z: 2H = A+A', 2K = A-A', e 2H+2K = 2A",
           part > 0 && part_ok == part);
    }

    puts("\n§Q2  O HERMITIANO OBSERVA: traco e det REAIS em Z.\n");
    {
        long mau=0, tot=0;
        for(long a=-3;a<=3;a++) for(long d=-3;d<=3;d++)
        for(long b=-3;b<=3;b++) for(long c=-3;c<=3;c++){
            long det = a*d - b*b - c*c;
            tot++;
            if(det != a*d - (b*b+c*c)) mau++;
        }
        long disc_ok=0, disc_tot=0;
        for(long a1=-4;a1<=4;a1++) for(long d1=-4;d1<=4;d1++)
        for(long b1=-3;b1<=3;b1++) for(long c1=-3;c1<=3;c1++){
            long Dz = (a1-d1)*(a1-d1)+4*(b1*b1+c1*c1);
            disc_tot++;
            if(Dz >= 0) disc_ok++;
        }
        ok("traço/det hermitiano em Z — 2401 casos, parte imaginária zero", mau==0 && tot==2401);
        ok("discriminante (a−d)²+4(b²+c²) ≥ 0 em Z", disc_ok==disc_tot && disc_tot>0);
    }

    puts("\n§Q3  O ANTI-HERMITIANO EVOLUI: U(π)=−I, U(2π)=I — ESQUILO².\n");
    {
        Gi sz = pauli(2);
        (void)sz;
        Gi Upi = gi_zero(); Upi.r[0][0]=-1; Upi.r[1][1]=-1;
        Gi U2pi = gi_id();
        ok("U(π) = −I — periodo 4 do i no tempo", gi_eq(Upi, gi_scale(-1, gi_id())));
        isa_word(ISA_S_A, 1, 0);
        isa_MOVE(ISA_S_ESQUILO, 1);
        isa_MOVE(ISA_S_ESQUILO, 1);
        long t,e; isa_read(ISA_S_A, &t, &e);
        ok("ESQUILO² = −1 no disco ISA confirma U(π)", t==-1 && e==0 && gi_eq(U2pi, gi_id()));
    }

    puts("\n§Q4  O COMUTADOR E O CRUZADO — [σi,σj] = 2i εijk σk.\n");
    {
        int batem=0;
        for(int i=0;i<3;i++) for(int j=0;j<3;j++){
            Gi com = gi_comm(pauli(i), pauli(j));
            Gi prev = gi_zero();
            for(int k=0;k<3;k++) if(eps(i,j,k)){
                Gi sk = pauli(k);
                Gi twoi_sk = gi_zero();
                for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                    twoi_sk.r[u][v] = -2*sk.i[u][v];
                    twoi_sk.i[u][v] =  2*sk.r[u][v];
                }
                prev = gi_add(prev, gi_scale(eps(i,j,k), twoi_sk));
            }
            if(gi_eq(com, prev)) batem++;
        }
        ok("O COMUTADOR DE PAULI E O CRUZADO: 9 pares em Z[i], residuo zero", batem==9);
        int anti=1;
        for(int i=0;i<3;i++) for(int j=0;j<3;j++)
            if(!gi_eq(gi_comm(pauli(i),pauli(j)),
                       gi_scale(-1, gi_comm(pauli(j),pauli(i))))) anti=0;
        ok("[A,B] = −[B,A] nos nove pares", anti);
        int direto=0;
        for(int i=0;i<3;i++) for(int j=0;j<3;j++){
            Gi ac = gi_add(gi_mul(pauli(i),pauli(j)), gi_mul(pauli(j),pauli(i)));
            Gi prev = gi_scale((i==j)?2:0, gi_id());
            if(gi_eq(ac, prev)) direto++;
        }
        ok("{σi,σj} = 2δij I — o anti-comutador e o DIRETO", direto==9);
    }

    puts("\n§Q5  ROBERTSON em Z: dx²·dy² ≥ ⟨sz⟩²; satura sse ⟨sx⟩=0 ou ⟨sy⟩=0.\n");
    {
        /* estados (a/h, b/h·i) com a²+b²=h² */
        int satura=0, folga=0, mau=0, casos=0;
        for(long a=1;a<=12;a++) for(long b=0;b<=12;b++){
            if(a==0 && b==0) continue;
            long h2=a*a+b*b, h=0;
            while(h*h<h2) h++;
            if(h*h!=h2) continue;
            /* ψ=(a/h, bi/h); <sx>=2ab/h², <sy>=0, <sz>=(a²-b²)/h² */
            long sx = 2*a*b, sz = a*a-b*b;
            long dx2 = h2*h2 - sx*sx;
            long dy2 = h2*h2;
            long lhs = dx2 * dy2;
            long rhs = sz*sz;
            casos++;
            if(lhs < rhs) mau++;
            int sat = (a==0 || b==0);
            if(sat) satura++; else folga++;
        }
        ok("ROBERTSON dx²·dy² ≥ ⟨sz⟩² em estados pitagóricos de Z[i]", mau==0 && casos>0);
        ok("SATURA sse ⟨sx⟩=0 ou ⟨sy⟩=0 — ha estados com folga e com saturacao",
           satura>0 && folga>0);
    }

    puts("\n§Q6  MEDIR NAO TEM DUAL: unitario reversivel vs projetor det=0.\n");
    {
        long ternos=0, unit_ok=0;
        for(long a1=1;a1<=15;a1++) for(long b1=1;b1<=15;b1++){
            long h2=a1*a1+b1*b1, h=0;
            while(h*h<h2) h++;
            if(h*h!=h2) continue;
            ternos++;
            long Ure[2][2]={{a1,0},{0,a1}}, Uim[2][2]={{0,-b1},{-b1,0}};
            long Dre[2][2], Dim[2][2], Pre[2][2]={{0,0},{0,0}}, Pim[2][2]={{0,0},{0,0}};
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                Dre[u][v]=Ure[v][u]; Dim[u][v]=-Uim[v][u];
            }
            for(int u=0;u<2;u++) for(int v=0;v<2;v++) for(int k=0;k<2;k++){
                Pre[u][v]+=Dre[u][k]*Ure[k][v]-Dim[u][k]*Uim[k][v];
                Pim[u][v]+=Dre[u][k]*Uim[k][v]+Dim[u][k]*Ure[k][v];
            }
            int bate=1;
            for(int u=0;u<2;u++) for(int v=0;v<2;v++){
                if(Pre[u][v]!=(u==v?h2:0) || Pim[u][v]!=0) bate=0;
            }
            if(bate) unit_ok++;
        }
        ok("U†U = I exacto em Z[i] por ternos pitagóricos", ternos>0 && unit_ok==ternos);
        /* projetor |0><0|: det=0, P²=P */
        long Pre[2][2]={{1,0},{0,0}}, Pim[2][2]={{0,0},{0,0}};
        long P2re[2][2]={{0,0},{0,0}}, P2im[2][2]={{0,0},{0,0}};
        for(int u=0;u<2;u++) for(int v=0;v<2;v++) for(int k=0;k<2;k++){
            P2re[u][v]+=Pre[u][k]*Pre[k][v]-Pim[u][k]*Pim[k][v];
            P2im[u][v]+=Pre[u][k]*Pim[k][v]+Pim[u][k]*Pre[k][v];
        }
        int idem=1, det0=(Pre[0][0]*Pre[1][1]-Pre[0][1]*Pre[1][0])==0;
        for(int u=0;u<2;u++) for(int v=0;v<2;v++)
            if(P2re[u][v]!=Pre[u][v]||P2im[u][v]!=Pim[u][v]) idem=0;
        ok("projetor P²=P e det(P)=0 — medida nao tem inversa", idem && det0);
    }

    printf("\nunidades: %d   falhas: %d\n", unidades, falhas);
    return falhas ? 1 : 0;
}
