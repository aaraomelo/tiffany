/* parâmetro linear t no termo cruzado: D² + t C², e Δ = 4 t C²
 * sobre o par 2D (E,B). Três classes = fis:thm:leidisc.
 * Rotor (s,p)=(0,1) e estrela (1,-1) = fis:thm:estrela(2).
 *   gcc -O2 -std=c99 tools/_probe_cruzado_t.c -o tools/_probe_cruzado_t
 */
#include <stdio.h>

static long det2(long a, long b, long c, long d){ return a*d - b*c; }
static long tr2(long a, long d){ return a + d; }

int main(void){
    long tlist[5] = { -2, -1, 0, 1, 2 };
    printf("cruzado  D=E·B  C=det(E,B)  A_t=[[D, t C],[C, D]]  Δ=4 t C²\n");
    printf("classes  Δ<0 elíptico (roda)  Δ=0 parabólico  Δ>0 hiperbólico\n\n");

    for(int k = 0; k < 5; k++){
        long t = tlist[k];
        long n = 0, eli = 0, par = 0, hip = 0, cone = 0, C0 = 0;
        for(long e1=-4;e1<=4;e1++) for(long e2=-4;e2<=4;e2++)
        for(long b1=-4;b1<=4;b1++) for(long b2=-4;b2<=4;b2++){
            long D = e1*b1 + e2*b2;
            long C = e1*b2 - e2*b1;
            if(D==0 && C==0) continue;
            n++;
            long Del = 4 * t * C * C;
            long det = D*D - t*C*C;
            if(C==0) C0++;
            if(Del<0) eli++;
            else if(Del==0) par++;
            else hip++;
            if(det==0) cone++;
        }
        printf("  t=%2ld  pares=%ld  eli=%ld  par=%ld  hip=%ld  C=0:%ld  detA=0 (cone de A_t):%ld\n",
               t, n, eli, par, hip, C0, cone);
    }

    /* R:(a,b)↦(-b,a)  ordem 4 — o rotor no plano, sempre */
    {
        long tot=0, o4=0;
        for(long a=-8;a<=8;a++) for(long b=-8;b<=8;b++){
            if(a==0 && b==0) continue;
            tot++;
            long a1=-b, b1=a;
            long a2=-b1, b2=a1;
            long a4=-a2, b4=-b2; /* R²=-id, R⁴=id */
            if(a2==-a && b2==-b && a4==a && b4==b) o4++;
        }
        printf("\n  R⁴=id no plano  %ld/%ld  (espiral: ordem 4, Δ do rotor = -4)\n", o4, tot);
    }

    /* (s,p) do paper: Δ=s²-4p. Rotor (0,1), estrela (1,-1), troca (0,-1). */
    {
        struct { long s, p; const char *nome; } U[4] = {
            {0, 1, "rotor  x²=-1"},
            {1,-1, "estrela x²=x+1"},
            {0,-1, "troca  x²=1"},
            {1, 1, "x²-x+1"},
        };
        printf("\n  cifra (s,p)     Δ=s²-4p     classe          ord da companheira [[0,-p],[1,s]]\n");
        for(int i=0;i<4;i++){
            long s=U[i].s, p=U[i].p;
            long Del = s*s - 4*p;
            const char *cls = Del<0 ? "elíptico" : Del==0 ? "parabólico" : "hiperbólico";
            /* A=[[0,-p],[1,s]], potência até 8 ou I */
            long a00=0, a01=-p, a10=1, a11=s;
            int ord=0;
            long b00=a00, b01=a01, b10=a10, b11=a11;
            for(int k=1;k<=12;k++){
                if(b00==1 && b01==0 && b10==0 && b11==1){ ord=k; break; }
                long n00=b00*a00+b01*a10, n01=b00*a01+b01*a11;
                long n10=b10*a00+b11*a10, n11=b10*a01+b11*a11;
                b00=n00; b01=n01; b10=n10; b11=n11;
            }
            printf("  (%ld,%ld) %-14s  Δ=%3ld  %-14s  ord=%d\n",
                   s, p, U[i].nome, Del, cls, ord);
        }
    }

    /* Fibonacci: C/D → estrela. Pares (F_n, F_{n+1}) como (D,C)? Cassini. */
    {
        long f[12]; f[0]=0; f[1]=1;
        for(int i=2;i<12;i++) f[i]=f[i-1]+f[i-2];
        printf("\n  Fibonacci (D,C)=(F_n,F_{n+1}):  D²+D C-C²  (estrela: x²=x+1 ⇒ 1+1-1=1, não 0)\n");
        printf("  a identidade de Cassini  F_{n-1}F_{n+1}-F_n² = (-1)^n\n");
        int cassini=0, tot=0;
        for(int n=1;n<=10;n++){
            long D=f[n], C=f[n+1];
            long cass = f[n-1]*f[n+1] - f[n]*f[n];
            long Q = D*D + D*C - C*C; /* x²+x-1 em (C/D)?  D²+DC-C² = -Cassini*sign */
            tot++;
            if(cass == ((n%2==0)?1:-1) || cass == ((n%2==0)?-1:1)){
                /* (-1)^n : n even +1, n odd -1 */
            }
            if(cass == (n%2==0 ? 1 : -1)) cassini++;
            printf("    n=%2d  (D,C)=(%ld,%ld)  Cassini=%ld  D²+DC-C²=%ld  Δ_estrela-family s=D+C p=-DC? skip\n",
                   n, D, C, cass, Q);
        }
        printf("  Cassini exacto em %d/%d\n", cassini, tot);
    }
    return 0;
}
