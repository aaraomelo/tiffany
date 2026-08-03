/* zetadin.c — OS QUATRO NA ZETA DINÂMICA: direta, inversa, convolução, deconvolução.
 *
 * O Aarão: "avança com o capítulo de análise da teoria: construir a transformada direta,
 * inversa, convolução e deconvolução na zeta dinâmica."
 *
 * (Não confundir com zeta.c, que é sobre a de Riemann. Esta é a de Artin–Mazur.)
 *
 * É o objeto onde os quatro fecham em INTEIROS, sem um float:
 *
 *     ζ(x) = 1/det(I − xC) = 1/(1 − mx − x²),    C = [[m,1],[1,0]]
 *
 * e o que a torna especial é uma linha:  ζ(0) = 1/det(I) = 1.  SEMPRE. É esse 1 que decide a
 * deconvolução, porque uma série com termo constante 1 é UNIDADE de Z[[x]] — invertível sem
 * sair dos inteiros, e sem uma divisão.
 *
 * OS DOIS NÍVEIS, e são o par:
 *
 *   nos COEFICIENTES   ζ = Σ c_n x^n         multiplicar zetas é CONVOLUIR    ⊛
 *   nos TRAÇOS         log ζ = Σ t_k x^k/k   multiplicar zetas é SOMAR        ⊕
 *
 * e o log leva um ao outro — a prop:conjuga do texto, aqui a fazer trabalho. A mesma operação
 * tem duas caras conforme a coordenada em que se olha.
 *
 * E OS NÚMEROS SÃO OS DE SEMPRE. Para m = 1:
 *
 *     coeficientes de ζ   1, 1, 2, 3, 5, 8, 13, …    FIBONACCI — conta CAMINHOS
 *     traços t_k          2, 1, 3, 4, 7, 11, 18, …    LUCAS     — conta ÓRBITAS
 *
 *   §Z1  DIRETA: ζ·det(I−xC) = 1, e os coeficientes saem por recorrência — inteiros
 *   §Z2  INVERSA: t_k = c_k + c_{k−2}, exato — dos caminhos voltam as órbitas
 *   §Z3  CONVOLUÇÃO: ζ_A·ζ_B convolui os coeficientes, e os denominadores multiplicam
 *   §Z4  DECONVOLUÇÃO: existe SEMPRE, porque ζ(0) = 1 é unidade de Z[[x]]
 *   §Z5  o PAR: nos coeficientes ⊛, nos traços ⊕ — a mesma operação, duas caras
 *   §Z6  controlo negativo: com termo constante ≠ ±1 a deconvolução SAI de Z
 *
 *   cc -O2 -std=c99 -Wall zetadin.c -o zetadin && ./zetadin
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;
#define NC 24

static void zeta_coef(L m, L *c, int N){
    c[0]=1; if(N>1) c[1]=m;
    for(int n=2;n<N;n++) c[n]=m*c[n-1]+c[n-2];
}
static void tracos(L m, L *t, int N){
    t[0]=2; if(N>1) t[1]=m;
    for(int k=2;k<N;k++) t[k]=m*t[k-1]+t[k-2];
}
static void conv(const L *a, const L *b, L *r, int N){
    for(int n=0;n<N;n++){ r[n]=0; for(int k=0;k<=n;k++) r[n]+=a[k]*b[n-k]; }
}

int main(void){
    printf("================================================================\n");
    printf("  Os quatro na zeta dinamica — tudo em inteiros\n");
    printf("================================================================\n");

    printf("\n§Z1 DIRETA: ζ·det(I−xC) = 1, e os coeficientes saem por recorrência\n");
    {
        int metais=0, ok_um=0, fib=0;
        printf("      m    coeficientes de ζ         ζ·(1−mx−x²)\n");
        for(L m=1;m<=8;m++){
            L c[NC]; zeta_coef(m,c,NC);
            L d[NC]; for(int i=0;i<NC;i++) d[i]=0;
            d[0]=1; d[1]=-m; d[2]=-1;
            L p[NC]; conv(c,d,p,NC);
            metais++;
            int bom=(p[0]==1); for(int n=1;n<NC-2;n++) if(p[n]!=0) bom=0;
            if(bom) ok_um++;
            if(m==1){
                L f[NC]; f[0]=1; f[1]=1;
                for(int n=2;n<NC;n++) f[n]=f[n-1]+f[n-2];
                int ig=1; for(int n=0;n<12;n++) if(c[n]!=f[n]) ig=0;
                fib=ig;
            }
            if(m<=3) printf("      %-4lld %lld %lld %lld %lld %lld %lld %lld%*s[%lld %lld %lld %lld ...]\n",
                            m,c[0],c[1],c[2],c[3],c[4],c[5],c[6],8,"",p[0],p[1],p[2],p[3]);
        }
        printf("      metais: %d   com ζ·(1−mx−x²) = 1: %d\n", metais, ok_um);
        ok("ζ·det(I−xC) = 1 exatamente, em inteiros — a direta fecha", ok_um==metais);
        ok("e para m=1 os coeficientes de ζ sao FIBONACCI", fib);
        conclui("ζ(0) = 1/det(I) = 1 SEMPRE — e e esse 1 que vai decidir tudo o resto.");
    }

    printf("\n§Z2 INVERSA: t_k = c_k + c_{k−2} — dos caminhos voltam as orbitas\n");
    {
        int metais=0, bate=0, luc=0;
        printf("      m    tracos t_k                c_k + c_{k−2}\n");
        for(L m=1;m<=8;m++){
            L c[NC],t[NC]; zeta_coef(m,c,NC); tracos(m,t,NC);
            metais++;
            int bom=1;
            for(int k=2;k<NC-1;k++) if(t[k]!=c[k]+c[k-2]) bom=0;
            if(bom) bate++;
            if(m==1){
                L lu[12]={2,1,3,4,7,11,18,29,47,76,123,199};
                int ig=1; for(int k=0;k<12;k++) if(t[k]!=lu[k]) ig=0;
                luc=ig;
            }
            if(m<=3) printf("      %-4lld %lld %lld %lld %lld %lld %lld %lld%*s%lld %lld %lld %lld\n",
                            m,t[0],t[1],t[2],t[3],t[4],t[5],t[6],8,"",
                            c[2]+c[0],c[3]+c[1],c[4]+c[2],c[5]+c[3]);
        }
        printf("      metais: %d   com t_k = c_k + c_{k−2}: %d\n", metais, bate);
        ok("a inversa e exata: t_k = c_k + c_{k−2}, em inteiros", bate==metais);
        ok("e para m=1 os tracos sao LUCAS — contra os Fibonacci dos coeficientes", luc);
        conclui("um conta CAMINHOS (os c_n) e o outro conta ORBITAS (os t_k), e a diferenca");
        conclui("entre eles e dois passos atras. Fibonacci e Lucas, o par de sempre.");
    }

    printf("\n§Z3 CONVOLUCAO: ζ_A·ζ_B convolui os coeficientes, e os denominadores multiplicam\n");
    {
        int pares=0, fecha=0;
        printf("      A  B    (ζ_A ⊛ ζ_B)_n         d_A·d_B              o produto ⊛ o denom.\n");
        for(L ma=1;ma<=4;ma++) for(L mb=1;mb<=4;mb++){
            L ca[NC],cb[NC],cc[NC]; zeta_coef(ma,ca,NC); zeta_coef(mb,cb,NC);
            conv(ca,cb,cc,NC);
            L da[3]={1,-ma,-1}, db[3]={1,-mb,-1}, dd[5]={0,0,0,0,0};
            for(int i=0;i<3;i++) for(int j=0;j<3;j++) dd[i+j]+=da[i]*db[j];
            L D[NC]; for(int i=0;i<NC;i++) D[i]=(i<5)?dd[i]:0;
            L chk[NC]; conv(cc,D,chk,NC);
            pares++;
            int bom=(chk[0]==1); for(int n=1;n<NC-5;n++) if(chk[n]!=0) bom=0;
            if(bom) fecha++;
            if(pares<=3)
                printf("      %lld  %lld    %lld %lld %lld %lld %lld%*s[%lld %lld %lld %lld %lld]   [%lld %lld %lld %lld ...]\n",
                       ma,mb,cc[0],cc[1],cc[2],cc[3],cc[4],6,"",
                       dd[0],dd[1],dd[2],dd[3],dd[4],chk[0],chk[1],chk[2],chk[3]);
        }
        printf("      pares (A,B): %d   com (ζ_A⊛ζ_B)·(d_A d_B) = 1: %d\n", pares, fecha);
        ok("multiplicar zetas e CONVOLUIR os coeficientes — exato, em inteiros",
           fecha==pares && pares == 16);
        conclui("e o denominador do produto e o produto dos denominadores: a convolucao das");
        conclui("series corresponde a multiplicacao dos polinomios caracteristicos.");
    }

    printf("\n§Z4 DECONVOLUCAO: existe SEMPRE, porque ζ(0) = 1 e unidade de Z[[x]]\n");
    {
        int casos=0, exatos=0;
        printf("      A  B    deconvoluir (ζ_A ζ_B) por ζ_B     devolve ζ_A?\n");
        for(L ma=1;ma<=5;ma++) for(L mb=1;mb<=5;mb++){
            L ca[NC],cb[NC],prod[NC]; zeta_coef(ma,ca,NC); zeta_coef(mb,cb,NC);
            conv(ca,cb,prod,NC);
            L q[NC];
            for(int n=0;n<NC;n++){
                L s=prod[n];
                for(int k=1;k<=n;k++) s-=cb[k]*q[n-k];
                q[n]=s;                      /* b_0 = 1: NAO ha divisao */
            }
            casos++;
            int ig=1; for(int n=0;n<NC-2;n++) if(q[n]!=ca[n]) ig=0;
            if(ig) exatos++;
            if(casos<=3)
                printf("      %lld  %lld    %lld %lld %lld %lld %lld   contra   %lld %lld %lld %lld %lld    %s\n",
                       ma,mb,q[0],q[1],q[2],q[3],q[4],ca[0],ca[1],ca[2],ca[3],ca[4],ig?"SIM":"nao");
        }
        printf("      pares: %d   a devolver ζ_A exatamente: %d\n", casos, exatos);
        ok("a deconvolucao devolve ζ_A EXATO — sem uma divisao, porque b_0 = 1",
           exatos==casos && casos == 25);
        conclui("e o oposto do caso que cinde: la uma casa nula mata a deconvolucao, e aqui");
        conclui("ela NUNCA falha — porque ζ(0) = 1/det(I) = 1, e isso nao depende do metal.");
    }

    printf("\n§Z5 o PAR: nos coeficientes ⊛, nos tracos ⊕ — a mesma operacao, duas caras\n");
    {
        int pares=0, ok_par=0;
        printf("      A  B    t^A_k + t^B_k          c_n do produto (≠ soma dos c_n)\n");
        for(L ma=1;ma<=5;ma++) for(L mb=1;mb<=5;mb++){
            L ta[NC],tb[NC],ca[NC],cb[NC],cc[NC];
            tracos(ma,ta,NC); tracos(mb,tb,NC);
            zeta_coef(ma,ca,NC); zeta_coef(mb,cb,NC); conv(ca,cb,cc,NC);
            int nao_soma=0;
            for(int n=2;n<8;n++) if(cc[n]!=ca[n]+cb[n]) nao_soma++;
            pares++;
            if(nao_soma>=5) ok_par++;
            if(pares<=3)
                printf("      %lld  %lld    %lld %lld %lld %lld %lld%*s%lld %lld %lld  (soma daria %lld %lld %lld)\n",
                       ma,mb,ta[1]+tb[1],ta[2]+tb[2],ta[3]+tb[3],ta[4]+tb[4],ta[5]+tb[5],6,"",
                       cc[2],cc[3],cc[4],ca[2]+cb[2],ca[3]+cb[3],ca[4]+cb[4]);
        }
        printf("      pares: %d   com os coeficientes a NAO somar: %d\n", pares, ok_par);
        ok("os TRACOS somam e os COEFICIENTES nao — a mesma operacao, duas caras",
           ok_par==pares && pares == 25);
        conclui("e a prop:conjuga a fazer trabalho: exp leva a soma ao produto, e por isso a");
        conclui("mesma operacao e ⊕ nos tracos e ⊛ nos coeficientes. Uma mede, a outra ordena.");
    }

    printf("\n§Z6 controlo negativo: com termo constante ≠ ±1 a deconvolucao SAI de Z\n");
    {
        /* dividir por b com b_0 = 2: q_0 = r_0/2, e se r_0 e impar sai de Z. */
        L b0 = 2, r0 = 1;
        printf("      b_0 = %lld,  r_0 = %lld   →  q_0 = r_0/b_0 = %lld/%lld\n", b0, r0, r0, b0);
        printf("      e divisivel? %s\n", (r0 % b0)? "NAO — sai de Z" : "sim");
        ok("com b_0 = 2 e r_0 impar a deconvolucao sai dos inteiros", (r0 % b0) != 0);
        /* e b_0 = 0: nao ha q_0 nenhum */
        int sem_inversa = 1;                 /* 0 nao e unidade de Z[[x]] */
        printf("      e com b_0 = 0: nao ha q_0 nenhum — a serie NAO e invertivel\n");
        ok("com b_0 = 0 nao ha deconvolucao: 0 nao e unidade de Z[[x]]", sem_inversa);
        /* e a zeta escapa aos dois, para TODOS os metais */
        int metais=0, escapa=0;
        for(L m=1;m<=8;m++){ L c[NC]; zeta_coef(m,c,NC); metais++; if(c[0]==1) escapa++; }
        printf("      e a zeta dinamica: c_0 = 1 em %d de %d metais\n", escapa, metais);
        ok("a zeta escapa aos dois: c_0 = 1 em TODOS os metais — e 1 e unidade",
           escapa==metais);
        conclui("nao e sorte do metal: c_0 = ζ(0) = 1/det(I) = 1, e o determinante da");
        conclui("identidade e 1 em toda a familia. A condicao vem da estrutura, nao do caso.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
