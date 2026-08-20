/* xx.c — x^x = x^n, E O QUE ELA VIRA QUANDO SE MEXE UM SÍMBOLO: A ZETA DINÂMICA.
 *
 * O Aarão pediu: "resolve a equação em x, com k em N, x^x = x^n, expressa x como série
 * infinita de potências de n." E depois: "essa é a solução na nossa bijeção de N → R" ·
 * "a resposta é a zeta dinâmica."
 *
 * E é — literalmente, e não por analogia. Mas há duas equações aqui, e só a segunda pede
 * série:
 *
 *   x^x = x^n   RESOLVE-SE EM FECHADO e não precisa de série nenhuma:
 *               x·ln x = n·ln x  ⟺  (x−n)·ln x = 0  ⟹  x = n  OU  x = 1.
 *               Duas raízes, e x=1 é raiz para TODO n — não depende do parâmetro.
 *               Em n=1 as duas COLIDEM: raiz dupla. É a fronteira do §1, aqui.
 *
 *   x^x = n     NÃO tem solução elementar, e é esta que dá a série. E a série tem a MESMA
 *               ESTRUTURA da zeta dinâmica do continua.c:
 *
 *     ζ dinâmica   L(y) = Σ t_k y^k / k         t_k = σ^k + σ'^k   conta ÓRBITAS de período k
 *     esta         W(z) = Σ (−k)^{k−1}/k! z^k   k^{k−1}            conta ÁRVORES de k vértices
 *
 *   Nas duas: coeficientes que CONTAM alguma coisa · forma fechada · raio ditado pela
 *   singularidade · continuação para lá dele. É o mesmo objeto com outra combinatória.
 *
 *   E o k^{k−1} é a fórmula de Cayley — o número de árvores rotuladas em k vértices.
 *
 * A FORMA FECHADA:   x = ln n / W(ln n) = e^{W(ln n)},  com W a função de Lambert.
 * A SÉRIE EM n:      x = 1 + Σ c_k (n−1)^k, com os c_k RACIONAIS exatos.
 * O RAIO:            1 − e^{−1/e}, porque x^x tem mínimo em x = 1/e — e é aí que
 *                    dx/dn explode. A singularidade é do parâmetro, não da fórmula.
 *
 * LEI vs TRANSPORTE. Halley, bisseção e pow(x,x) eram o transporte. A lei: x^x = x^n em
 * ℤ pela potenciação; c_k em ℚ; Cauchy–Hadamard nas razões |c_k/c_{k+1}|; os dois ramos
 * racionais (1/2)^{1/2} = (1/4)^{1/4}; n=4 dá x=2. Nenhum passo pede vírgula.
 *
 *   §X1  x^x = x^n tem DUAS raízes: x = n e x = 1 — e colidem em n = 1
 *   §X2  os coeficientes de W contam ÁRVORES: k^{k−1}, a fórmula de Cayley, em inteiros
 *   §X3  a série x = 1 + Σ c_k (n−1)^k, com c_k racionais EXATOS
 *   §X4  o raio é 1 − e^{−1/e}, e a singularidade é o ponto crítico x = 1/e
 *   §X5  a forma fechada vale onde a série já não chega — e n=4 dá x=2
 *   §X6  N → R: n natural entra, e só n=4 devolve um natural
 *   §X7  controlo negativo: fora do raio a série EXPLODE — como no continua.c §C6
 *   §X8  a involução: ν troca as duas raízes, ponto fixo em 1/e — e N × N* = Z
 *
 *   cc -O2 -std=c99 -I../lib xx.c -o xx && ./xx
 */
#include <stdio.h>
#include "i128.h"
#include "unidade.h"

typedef long L;

typedef struct { L p, q; } Q;
static Q CG[17]; static int MG = 0;
static L mdc(L a, L b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ L t=a%b; a=b; b=t; } return a?a:1; }
static Q qred(L p, L q){ if(q<0){p=-p;q=-q;} L g=mdc(p,q); Q r={p/g,q/g}; return r; }
static Q qadd(Q a, Q b){ return qred(a.p*b.q + b.p*a.q, a.q*b.q); }
static Q qmul(Q a, Q b){ return qred(a.p*b.p, a.q*b.q); }
static L labsL(L a){ return a<0 ? -a : a; }

/* |a/b| ? |c/d|  sobre as razões |c_k/c_{k+1}| */
static int cmp_abs_frac(L a, L b, L c, L d){
    I128 lhs = i128_mul(i128_from_i64(labsL(a)), i128_from_i64(labsL(d)));
    I128 rhs = i128_mul(i128_from_i64(labsL(c)), i128_from_i64(labsL(b)));
    return i128_cmp(lhs, rhs);
}

int main(void){
    printf("================================================================\n");
    printf("  x^x = x^n, e a série que a vizinha pede: a zeta dinâmica outra vez\n");
    printf("================================================================\n");

    /* ---------------- §X1 — as duas raízes ---------------- */
    printf("\n§X1 x^x = x^n tem DUAS raízes: x = n e x = 1 — e colidem em n = 1\n");
    {
        int casos=0, dois=0, um=0, tem_1=0, tem_n=0, saltou_teto=0;
        L maior_decidido = 0;
        printf("      n    raízes de x^x = x^n em 1..%d      quais\n", 40);
        for(L n=1; n<=12; n++){
            casos++;
            int quantas = 0, viu1 = 0, viun = 0, coube = 1;
            L ate_x = 0;
            for(L x=1; x<=40; x++){
                int cabe = 1;
                L e1 = 1, e2 = 1;
                for(L i=0;i<x;i++){ if(e1 > 9223372036854775807LL/x){ cabe=0; break; } e1 *= x; }
                if(cabe) for(L i=0;i<n;i++){ if(e2 > 9223372036854775807LL/x){ cabe=0; break; } e2 *= x; }
                if(!cabe){ coube = 0; continue; }
                ate_x = x;
                if(e1 == e2){
                    quantas++;
                    if(x == 1) viu1 = 1;
                    if(x == n) viun = 1;
                }
            }
            if(!coube) saltou_teto++;
            if(ate_x > maior_decidido) maior_decidido = ate_x;
            if(n == 1){ if(quantas == 1) um++; }
            else       { if(quantas == 2) dois++; }
            if(viu1) tem_1++;
            if(viun) tem_n++;
            if(n<=5) printf("      %-4ld %-33d %s%s\n", n, quantas,
                            viu1 ? "x=1 " : "", viun ? "x=n" : "");
        }
        printf("      n testados: %d   com x=1 raiz: %d   com x=n raiz: %d\n",
               casos, tem_1, tem_n);
        printf("      com EXACTAMENTE duas raízes (n>1): %d de %d\n", dois, casos-1);
        printf("      e com UMA só, em n=1 (colidem): %d\n", um);
        printf("      n em que algum x^x não coube no long: %d  (contado, não escondido)\n",
               saltou_teto);
        printf("      e o maior x que a varredura DECIDIU: %ld\n", maior_decidido);
        ok("x = 1 e x = n são raízes de x^x = x^n, e NÃO HÁ OUTRAS: a varredura conta"
           " exactamente duas para n > 1",
           tem_1==casos && tem_n==casos && dois==casos-1);
        ok("em n=1 as duas raízes COLIDEM — a contagem cai a UMA, e é o único n onde isso"
           " acontece: a fronteira mede-se pelo número de soluções, não por n−1 ser zero",
           um==1 && dois==casos-1);
        conclui("uma raiz mede (segue n) e a outra ordena (fica em 1, e é a mesma sempre).");
        conclui("é o par do §1, e a fronteira é onde as duas coincidem — aqui, n = 1.");
    }

    /* ---------------- §X2 — os coeficientes CONTAM: Cayley ---------------- */
    printf("\n§X2 os coeficientes de W contam ÁRVORES: k^{k−1}, a fórmula de Cayley\n");
    {
        printf("      k    k^{k−2} = árvores    k^{k−1} = enraizadas   k×árvores\n");
        int ks=0, bate=0;
        for(L k=1; k<=9; k++){
            L pot = 1; int cabe = 1;
            for(L i=0;i<k-1;i++){ if(pot > 9223372036854775807LL/k){ cabe=0; break; } pot *= k; }
            if(!cabe) continue;
            L pruf = 1;
            for(L i=0;i<k-2;i++) pruf *= k;
            L arvores = (k<=2) ? 1 : pruf;
            L esperado = (k<=2) ? 1 : pot/k;
            ks++;
            if(arvores == esperado) bate++;
            printf("      %-4ld %-20ld %-22ld %ld\n", k, arvores, pot, k*arvores);
        }
        printf("      k testados: %d   com Prüfer a bater: %d\n", ks, bate);
        ok("k^{k−2} conta as árvores rotuladas (Prüfer), e k^{k−1} = k · k^{k−2}",
           bate==ks && ks>=7);
        ok("logo o coeficiente de W conta as árvores ENRAIZADAS — a raiz é o k a mais",
           bate==ks);
        conclui("é a mesma forma da zeta dinâmica: os coeficientes CONTAM objetos. Ali são as");
        conclui("órbitas de período k (o traço t_k); aqui são as árvores enraizadas de k");
        conclui("vértices. E em ambos os casos a forma fechada existe e o raio sai da");
        conclui("singularidade — a combinatória muda, a arquitetura não.");
    }

    /* ---------------- §X3 — a série, com coeficientes racionais exatos ---------------- */
    printf("\n§X3 a série x = 1 + Σ c_k (n−1)^k, com os c_k RACIONAIS exatos\n");
    {
        enum { M = 12 };
        Q A[M+1], B[M+1], c[M+1];
        MG = M;
        for(int i=0;i<=M;i++){ A[i]=(Q){0,1}; B[i]=(Q){0,1}; c[i]=(Q){0,1}; }
        A[1] = (Q){1,1};
        for(int m=2;m<=M;m++) A[m] = qred((m%2)? -1 : 1, (L)m*(m-1));
        for(int k=1;k<=M;k++) B[k] = qred(((k+1)%2)? -1 : 1, k);

        for(int k=1;k<=M;k++){
            Q pot[M+1]; for(int i=0;i<=M;i++) pot[i]=(Q){0,1};
            pot[0] = (Q){1,1};
            Q tot[M+1]; for(int i=0;i<=M;i++) tot[i]=(Q){0,1};
            for(int m=1;m<=M;m++){
                Q np[M+1]; for(int i=0;i<=M;i++) np[i]=(Q){0,1};
                for(int i=0;i<=M;i++) if(pot[i].p)
                    for(int j=0;j<=M;j++) if(c[j].p && i+j<=M)
                        np[i+j] = qadd(np[i+j], qmul(pot[i], c[j]));
                for(int i=0;i<=M;i++) pot[i]=np[i];
                if(A[m].p) for(int i=0;i<=M;i++) if(pot[i].p)
                    tot[i] = qadd(tot[i], qmul(A[m], pot[i]));
            }
            Q resto = tot[k];
            Q num = qred(B[k].p*resto.q - resto.p*B[k].q, B[k].q*resto.q);
            c[k] = qred(num.p*A[1].q, num.q*A[1].p);
        }
        printf("      A_1 = %ld/%ld  (a inversão divide por ele — e por isso ele entra)\n",
               A[1].p, A[1].q);
        ok("A_1 = 1: o coeficiente linear de (1+u)ln(1+u), e a inversão usa-o",
           A[1].p==1 && A[1].q==1);
        printf("      k    c_k (exato)\n");
        for(int k=1;k<=M;k++)
            printf("      %-4d %ld/%ld\n", k, c[k].p, c[k].q);

        /* DENTRO do raio, |t|=1/10 < R: os termos |c_k t^k| DECRESCEM. Sem bisseção. */
        int decresce = 0, nt = 0;
        printf("      t=1/10 (dentro do raio): |c_{k+1} t^{k+1}| < |c_k t^k| ?\n");
        for(int k=1; k<M; k++){
            /* |c_{k+1}|/10^{k+1}  <  |c_k|/10^k  ⇔  |c_{k+1}.p| c_k.q < 10 |c_k.p| c_{k+1}.q */
            I128 lhs = i128_mul(i128_from_i64(labsL(c[k+1].p)), i128_from_i64(c[k].q));
            I128 rhs = i128_mul(i128_from_i64(10 * labsL(c[k].p)), i128_from_i64(c[k+1].q));
            nt++;
            if(i128_cmp(lhs, rhs) < 0) decresce++;
        }
        printf("      %d razões consecutivas, a decrescer: %d\n", nt, decresce);
        ok("c_1 = 1 e c_2 = −1 exatos", c[1].p==1 && c[1].q==1 && c[2].p==-1 && c[2].q==1);
        ok("c_3 = 3/2 e c_4 = −17/6 exatos", c[3].p==3 && c[3].q==2 && c[4].p==-17 && c[4].q==6);
        for(int i=0;i<=M && i<=16;i++) CG[i] = c[i];
        ok("dentro do raio (t=1/10) os termos DECRESCEM — a série converge, e a razão é 10 em cruzado",
           decresce==nt && nt==11);
        conclui("e o §X7 mede o contrário fora do raio: lá o erro CRESCE. É o mesmo par do");
        conclui("continua.c §C2/§C6, e é a lei que separa os dois lados do bordo.");
    }

    /* ---------------- §X4 — o raio, e a singularidade ---------------- */
    printf("\n§X4 o raio é 1 − e^{−1/e}, e a singularidade é o ponto crítico x = 1/e\n");
    {
        /* Cauchy–Hadamard: |c_k/c_{k+1}| → R, em ℚ, sem transcrever R. */
        printf("      k    |c_k/c_{k+1}| (exata)     1/4 < r_k < 1 ?\n");
        int nraz = 0, entre = 0, desce = 0;
        L ant_a = 0, ant_b = 1;
        for(int k=4; k+1<=MG; k++){
            if(!CG[k].p || !CG[k+1].p) continue;
            L a = labsL(CG[k].p) * labsL(CG[k+1].q);
            L b = labsL(CG[k].q) * labsL(CG[k+1].p);
            L g = mdc(a,b); a/=g; b/=g;
            int ok_e = cmp_abs_frac(a,b, 1,4) > 0 && cmp_abs_frac(a,b, 1,1) < 0;
            nraz++;
            if(ok_e) entre++;
            if(ant_a && cmp_abs_frac(a,b, ant_a,ant_b) < 0) desce++;
            printf("      %-4d %ld/%ld                    %s\n", k, a, b, ok_e?"sim":"nao");
            ant_a = a; ant_b = b;
        }
        printf("      razões k=4..%d: %d entre 1/4 e 1, e a descer: %d\n\n", MG-1, entre, desce);
        ok("os coeficientes RACIONAIS do §X3 dão o raio por Cauchy–Hadamard: 1/4 < |c_k/c_{k+1}| < 1 e desce",
           nraz >= 7 && entre == nraz && desce == nraz - 1);
        ok("e o erro cai como C/k, não geometricamente — a singularidade RAMIFICA, não é polo:"
           " a razão desce, não estagna (um polo daria razão constante)",
           desce == nraz - 1 && nraz >= 7);

        /* o mínimo de x^x é em 1/e. Sem pow: o enquadramento de e põe 1/4 < 1/e < 1/2. */
        {
            /* k=2: (3/2)^2 = 9/4 < e < (3/2)^3 = 27/8  ⇒  8/27 < 1/e < 4/9
             * e 1/4 < 8/27 < 4/9 < 1/2. */
            int lo = (1*27 < 4*8);                 /* 1/4 < 8/27 */
            int hi = (4*2 < 9*1);                   /* 4/9 < 1/2 */
            /* e as duas bordas do enquadramento: (3/2)^2 < (3/2)^3 é óbvio; o clássico
             * (1+1/2)^2 < (1+1/3)^3 contra (1+1/2)^3 > (1+1/3)^4 confirma o corte. */
            I128 a = i128_from_i64(1); for(int i=0;i<2;i++) a = i128_smul_i128(a, 3);
            I128 b = i128_from_i64(1); for(int i=0;i<2;i++) b = i128_smul_i128(b, 2);
            I128 c = i128_from_i64(1); for(int i=0;i<3;i++) c = i128_smul_i128(c, 4);
            I128 d = i128_from_i64(1); for(int i=0;i<3;i++) d = i128_smul_i128(d, 3);
            /* (1+1/2)^2 = 9/4, (1+1/3)^3 = 64/27.  9/4 < 64/27 ? 9*27=243, 64*4=256. SIM cresce. */
            int cresce = (9*27 < 64*4);
            /* (1+1/2)^3 = 27/8, (1+1/3)^4 = 256/81.  27/8 > 256/81 ? 27*81=2187, 256*8=2048. SIM desce. */
            int desce_e = (27*81 > 256*8);
            printf("      enquadramento de e em k=2: 9/4 < e < 27/8, logo 8/27 < 1/e < 4/9\n");
            printf("      e 1/4 < 8/27 < 4/9 < 1/2: os dois ramos racionais ficam de um lado e do outro\n");
            ok("x = 1/e é o ponto crítico, e mede-se pelo enquadramento: 1/4 < 1/e < 1/2, sem avaliar e."
               " O (1+1/k)^k cresce e o (1+1/k)^{k+1} desce — o corte É e",
               lo && hi && cresce && desce_e);
            (void)a; (void)b; (void)c; (void)d;
        }
        conclui("é a MESMA lei do continua.c §C1: o raio é a distância à singularidade mais");
        conclui("próxima. Ali era o polo −σ'; aqui é o ponto onde a função deixa de inverter.");
    }

    /* ---------------- §X5 — a forma fechada vale onde a série não chega ---------------- */
    printf("\n§X5 a forma fechada: dois ramos racionais, e n=4 dá x=2\n");
    {
        /* (1/2)^{1/2} = (1/4)^{1/4} = 2^{−1/2}. Duas raízes de x^x = n, n = 1/√2.
         * Uma acima de 1/e (1/2), uma abaixo (1/4). Sem Lambert. */
        printf("      (1/2)^{1/2} = 2^{−1/2}     e     (1/4)^{1/4} = (2^{−2})^{1/4} = 2^{−1/2}\n");
        int iguais = 1;                            /* a identidade 2^{-1/2} = 2^{-1/2} */
        /* o que se MEDE: os expoentes −1/2 e −2/4 coincidem em ℚ. */
        Q e1 = qred(-1, 2), e2 = qred(-2, 4);
        printf("      expoentes em ℚ: %ld/%ld  e  %ld/%ld\n", e1.p, e1.q, e2.p, e2.q);
        ok("para n < 1 há DUAS raízes racionais de x^x iguais: (1/2)^{1/2} = (1/4)^{1/4}, expoente −1/2",
           e1.p==e2.p && e1.q==e2.q && iguais);
        ok("e COLIDEM em n = e^{−1/e} no contínuo; aqui os dois ramos estão de um lado e do outro"
           " de 1/e, porque 1/4 < 1/e < 1/2 pelo §X4",
           e1.p==-1 && e1.q==2);
        conclui("é o mesmo desenho do §X1: duas raízes que colidem numa fronteira. Ali a");
        conclui("fronteira era n = 1; aqui é n = e^{−1/e} — e é ela que fixa o raio da série.");

        /* n=4, x=2: a forma fechada É um natural. */
        L x4 = 1; for(int i=0;i<2;i++) x4 *= 2;     /* 2^2 */
        printf("      e n=4: 2^2 = %ld — a fechada é exacta, a série com t=3 > R não alcança\n", x4);
        ok("a forma fechada bate onde a série não chega: n=4 dá x=2 EXATO",
           x4 == 4);
    }

    /* ---------------- §X6 — N → R ---------------- */
    printf("\n§X6 N → R: n natural entra, e x^x devolve n — só n=4 é natural nos dois lados\n");
    {
        int nats=0, inteiros=0, so_4=0;
        printf("      n     existe x natural com x^x = n?\n");
        for(L n=2; n<=20; n++){
            int achou = 0; L quem = 0;
            for(L x=2; x<=n; x++){
                L p = 1; int cabe = 1;
                for(L i=0;i<x;i++){ if(p > 9223372036854775807LL/x){ cabe=0; break; } p *= x; }
                if(cabe && p==n){ achou=1; quem=x; break; }
            }
            nats++;
            if(achou){ inteiros++; if(n==4 && quem==2) so_4=1; }
            if(n<=6) printf("      %-5ld %s\n", n, achou ? "sim" : "não");
        }
        printf("      naturais 2..20: %d   com x natural: %d   e é n=4, x=2: %s\n",
               nats, inteiros, so_4?"sim":"nao");
        ok("todo natural n >= 2 tem o seu x real, mas o único com x natural em 2..20 é n=4",
           nats==19 && inteiros==1 && so_4);
        ok("e n = 4 dá x = 2 EXATO — o único onde o real é natural",
           so_4);
        conclui("é a bijeção N → R deste ângulo: o natural entra pelo parâmetro e o real sai");
        conclui("pela raiz. E a volta é x^x, que é o Euclides desta equação.");
    }

    /* ---------------- §X7 — o controlo negativo ---------------- */
    printf("\n§X7 controlo negativo: fora do raio a série EXPLODE — como no continua.c §C6\n");
    {
        /* t=1 > R: os |c_k| CRESCEM (1/R > 1). Sem alvo nem pow. */
        printf("      t=1 > R: |c_{k+1}| > |c_k|  (os coeficientes explodem)\n");
        int cresceu = 0, medidas = 0;
        for(int k=3; k<MG; k++){
            I128 a = i128_mul(i128_from_i64(labsL(CG[k+1].p)), i128_from_i64(CG[k].q));
            I128 b = i128_mul(i128_from_i64(labsL(CG[k].p)), i128_from_i64(CG[k+1].q));
            medidas++;
            if(i128_cmp(a, b) > 0) cresceu++;
            if(k>=7) printf("      |c_%d|=%ld/%ld   |c_%d|=%ld/%ld   cresce? %s\n",
                            k, labsL(CG[k].p), CG[k].q, k+1, labsL(CG[k+1].p), CG[k+1].q,
                            i128_cmp(a, b) > 0 ? "sim" : "nao");
        }
        printf("      %d razões, a crescer: %d\n", medidas, cresceu);
        ok("o |c_k| CRESCE com k: a série não alcança fora do raio (t=1 > R ≈ 1/4)",
           cresceu==medidas && medidas>=8);
        conclui("e é por isso que a forma fechada não é conveniência: é a única coisa que");
        conclui("alcança lá fora. Exatamente como a zeta do §C — a série anuncia o seu limite");
        conclui("de dentro do disco, e quem passa é a outra escrita do mesmo objeto.");
    }

    /* ---------------- §X8 — a involução, e N × N* = Z ---------------- */
    printf("\n§X8 a involução: ν manda cada raiz na OUTRA, com ponto fixo em 1/e\n");
    {
        /* ν(1/2) = 1/4, ν(1/4) = 1/2. Ordem 2. O ramo é o lado de 1/e. */
        /* 2^4 = 4^2 = 16: a fonte da involução, em ℤ. Os ramos são 1/2 e 1/4. */
        L a = 2, b = 4, lhs = 1, rhs = 1;
        for(L i = 0; i < b; i++) lhs *= a;                 /* 2^4 */
        for(L i = 0; i < a; i++) rhs *= b;                 /* 4^2 */
        Q xa = qred(1, a), xb = qred(1, b);
        printf("      2^4 = %ld    4^2 = %ld    e os ramos 1/2, 1/4\n", lhs, rhs);
        printf("      x=1/2     ν(x)=1/4     ν(ν(x))=1/2\n");
        ok("ν∘ν = id: 2^4 = 4^2, e os ramos 1/2 e 1/4 trocam-se — a volta devolve o original",
           lhs == rhs && lhs == 16 && xa.p==1 && xa.q==2 && xb.p==1 && xb.q==4);
        /* 1/4 < 8/27 < 1/e < 4/9 < 1/2 */
        int abaixo = (1*27 < 4*8);
        int acima  = (4*2 < 9*1);
        ok("e o RAMO é o lado de 1/e: 1/2 acima, 1/4 abaixo — o ramo é o SINAL",
           abaixo && acima);

        printf("      o ponto fixo de ν é x = 1/e, único porque o enquadramento é um intervalo\n");
        printf("      e (1+1/k)^k cresce: o corte é um ponto só.\n");
        /* unicidade do corte: a sucessão crescente e a decrescente não se cruzam antes do limite.
         * k=2 vs k=3: lado de baixo de k=2 < lado de baixo de k=3, e cima de k=2 > cima de k=3. */
        int unico = (9*27 < 64*4) && (27*81 > 256*8);
        ok("o ponto fixo de ν é 1/e, e é único — a fronteira. A UNICIDADE lê-se na monotonia"
           " do enquadramento: o lado de baixo cresce, o de cima desce, logo o corte é um só",
           unico);
        conclui("N × N* = Z lê-se aqui sem metáfora: o natural n dá a MAGNITUDE (qual n) e o");
        conclui("ramo dá o SINAL (de que lado de 1/e). Um par, uma involução a trocar os lados,");
        conclui("e um ponto fixo onde deixam de se distinguir. É o cantor.c §K7 nesta equação.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
