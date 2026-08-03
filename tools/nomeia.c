/* nomeia.c — A FUNÇÃO BIJETIVA, COMO FERRAMENTA. Z × N* ↔ R, na linha de comando.
 *
 * O palavra.c MEDE que a bijeção existe. Este EXECUTA-A: é o algoritmo reversível, usável,
 * a operar em inteiros do princípio ao fim. Nenhum float entra nesta ferramenta.
 *
 *   ./nomeia   nome    p q          →  a palavra de p/q       (Euclides, e PARA)
 *   ./nomeia   real    a0 a1 ...    →  o racional p/q         (Möbius, e é EXATO)
 *   ./nomeia   metal   m            →  a palavra periódica de σ_m, e a quadrática
 *   ./nomeia   cifra   K... -- p q  →  o criptograma (c1,c2)  (chave = palavra)
 *   ./nomeia   decifra K... -- c1 c2→  volta a (p,q) sem perder um bit
 *
 * A chave é uma PALAVRA: K = A_{k0}·A_{k1}···A_{kr}, e det K = ±1 sempre, porque cada
 * A_a = [[a,1],[1,0]] tem det −1. É isso — e só isso — que faz a decifra ser inteira.
 * Com det ≠ ±1 a cifra deixa de ser sobrejetiva e metade dos criptogramas não decifra
 * (medido: 50,0% contra 100,0%, palavra.c §P9).
 *
 * Exemplo, e a volta fecha:
 *   $ ./nomeia nome 13 8            → [1;1,1,1,2]
 *   $ ./nomeia real 1 1 1 1 2       → 13/8
 *   $ ./nomeia cifra 3 1 4 -- 13 8  → (c1,c2)
 *   $ ./nomeia decifra 3 1 4 -- c1 c2 → 13/8
 *
 *   cc -O2 -std=c99 -Wall nomeia.c -o nomeia
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <string.h>

typedef long long L;
#define KMAX 96

static L mdc(L a, L b){ if(a<0)a=-a; if(b<0)b=-b; while(b){ L t=a%b; a=b; b=t; } return a; }

/* Euclides sobre p/q com q>0. Devolve o comprimento da palavra. */
static int nome(L p, L q, L *a){
    int k = 0;
    while(q != 0 && k < KMAX){
        L d = p/q, r = p - d*q;
        if(r < 0){ d--; r += q; }          /* piso, também para p negativo */
        a[k++] = d; p = q; q = r;
    }
    return k;
}

/* M_w = A_{a0}···A_{a_{k−1}}, com A_a = [[a,1],[1,0]]. */
static void mob(const L *a, int k, L *m00, L *m01, L *m10, L *m11){
    L x00=1,x01=0,x10=0,x11=1;
    for(int i=0;i<k;i++){
        L n00 = x00*a[i]+x01, n01 = x00;
        L n10 = x10*a[i]+x11, n11 = x10;
        x00=n00; x01=n01; x10=n10; x11=n11;
    }
    *m00=x00; *m01=x01; *m10=x10; *m11=x11;
}

static void uso(void){
    fprintf(stderr,
      "uso:\n"
      "  nomeia nome    <p> <q>              a palavra de p/q  (Euclides — e PARA)\n"
      "  nomeia real    <a0> <a1> ...        o racional        (Möbius — e é EXATO)\n"
      "  nomeia metal   <m>                  a palavra de sigma_m, e a quadratica\n"
      "  nomeia cifra   <k...> -- <p> <q>    cifrar com a chave (palavra)\n"
      "  nomeia decifra <k...> -- <c1> <c2>  decifrar — sem perder um bit\n");
}

/* lê a chave até "--"; devolve o comprimento, e põe em *resto o índice do 1.º argumento a seguir */
static int le_chave(int argc, char **argv, int i0, L *k, int *resto){
    int n = 0;
    int i = i0;
    for(; i < argc; i++){
        if(!strcmp(argv[i], "--")){ i++; break; }
        if(n >= KMAX) return -1;
        k[n] = atoll(argv[i]);
        if(k[n] < 1){ fprintf(stderr, "nomeia: dígito de chave < 1 em '%s'\n", argv[i]); return -1; }
        n++;
    }
    *resto = i;
    return n;
}

/* Sem argumentos, a ferramenta DEMONSTRA-SE: ida e volta em inteiros, e a cifra a fechar.
 * É o mínimo para ela entrar na bateria como o que é — uma ferramenta que se verifica. */
static int autoteste(void){
    printf("nomeia — a função bijetiva, executável. Autoteste:\n\n");
    int casos=0, voltas=0;
    for(L q=1; q<=60; q++) for(L p=-120; p<=120; p++){
        if(q==0 || mdc(p<0?-p:p,q)!=1) continue;
        L a[KMAX]; int k = nome(p,q,a);
        if(k >= KMAX) continue;
        casos++;
        L m00,m01,m10,m11; mob(a,k,&m00,&m01,&m10,&m11);
        L rp=m00, rq=m10;
        if(rq < 0){ rp=-rp; rq=-rq; }
        L g = mdc(rp,rq); if(g>1){ rp/=g; rq/=g; }
        L pp=p, qq=q; L g2 = mdc(pp<0?-pp:pp,qq); if(g2>1){ pp/=g2; qq/=g2; }
        if(rp==pp && rq==qq) voltas++;
    }
    printf("      racionais (com sinal) nomeados e devolvidos: %d de %d\n", voltas, casos);
    ok("nome → real devolve p/q EXATO, sinal incluído", voltas==casos && casos>3000);

    /* a cifra: cifrar e decifrar com chaves de GL2(Z) */
    int cif=0, dec=0;
    for(L k0=1;k0<=5;k0++) for(L k1=1;k1<=5;k1++){
        L ch[2]={k0,k1}; L K00,K01,K10,K11; mob(ch,2,&K00,&K01,&K10,&K11);
        L det=K00*K11-K01*K10;
        for(L q=1;q<=20;q++) for(L p=1;p<=40;p++){
            if(mdc(p,q)!=1) continue;
            cif++;
            L c1=K00*p+K01*q, c2=K10*p+K11*q;
            L dp=( K11*c1 - K01*c2)/det, dq=(-K10*c1 + K00*c2)/det;
            if(dp==p && dq==q) dec++;
        }
    }
    printf("      pares cifrados e decifrados por 25 chaves: %d de %d\n", dec, cif);
    ok("cifra → decifra volta EXATO, porque det = ±1", dec==cif && cif>5000);

    /* a recusa do 0 no meio, que é a condição da bijeção */
    {
        L com0[3]={3,0,5}, sem0[1]={8};
        L p1,p2,q1,q2, r1,r2,s1,s2;
        mob(com0,3,&p1,&p2,&q1,&q2); mob(sem0,1,&r1,&r2,&s1,&s2);
        printf("      [3,0,5] e [8] dão a mesma matriz? %s\n",
               (p1==r1&&p2==r2&&q1==s1&&q2==s2) ? "sim — por isso o 0 é recusado" : "não");
        ok("[a,0,b] = [a+b]: é por isso que a ferramenta RECUSA o 0 no meio",
           p1==r1 && p2==r2 && q1==s1 && q2==s2);
    }
    printf("\n");
    uso();
    printf("\n  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas?"":" — RESÍDUO 0");
    return falhas ? 1 : 0;
}

int main(int argc, char **argv){
    if(argc == 1) return autoteste();
    if(argc < 3){ uso(); return 2; }
    const char *cmd = argv[1];

    if(!strcmp(cmd, "nome")){
        if(argc != 4){ uso(); return 2; }
        L p = atoll(argv[2]), q = atoll(argv[3]);
        if(q == 0){ fprintf(stderr, "nomeia: denominador 0\n"); return 2; }
        if(q < 0){ p = -p; q = -q; }
        L a[KMAX]; int k = nome(p,q,a);
        if(k >= KMAX){ fprintf(stderr, "nomeia: palavra maior que %d\n", KMAX); return 3; }
        printf("[%lld", a[0]);
        for(int i=1;i<k;i++) printf("%s%lld", i==1?";":",", a[i]);
        printf("]\n");
        /* a cabeça é Z, o resto é N* — e é a única letra que pode ser <= 0 */
        if(k > 1) printf("  cabeça em Z: %lld    palavra em N*: %d dígitos, todos >= 1\n", a[0], k-1);
        return 0;
    }

    if(!strcmp(cmd, "real")){
        int k = argc - 2;
        if(k < 1 || k > KMAX){ uso(); return 2; }
        L a[KMAX];
        for(int i=0;i<k;i++){
            a[i] = atoll(argv[i+2]);
            if(i > 0 && a[i] < 1){
                fprintf(stderr, "nomeia: a_%d = %lld — os dígitos a partir do 1.o têm de ser >= 1,\n"
                                "        senão a palavra deixa de ser única: [a,0,b] = [a+b].\n", i, a[i]);
                return 2;
            }
        }
        L m00,m01,m10,m11; mob(a,k,&m00,&m01,&m10,&m11);
        L p = m00, q = m10;
        if(q == 0){ printf("infinito (denominador 0)\n"); return 0; }
        if(q < 0){ p = -p; q = -q; }
        L g = mdc(p,q); if(g > 1){ p/=g; q/=g; }
        printf("%lld/%lld\n", p, q);
        printf("  M_w = [[%lld, %lld], [%lld, %lld]]   det = %lld   (a inversa é INTEIRA)\n",
               m00,m01,m10,m11, m00*m11-m01*m10);
        printf("  os dois convergentes: %lld/%lld e %lld/%lld — um por defeito, outro por excesso\n",
               m00,m10, m01,m11);
        return 0;
    }

    if(!strcmp(cmd, "metal")){
        if(argc != 3){ uso(); return 2; }
        L m = atoll(argv[2]);
        if(m < 1){ fprintf(stderr, "nomeia: metal >= 1\n"); return 2; }
        printf("sigma_%lld = [%lld; %lld, %lld, ...]   período 1\n", m, m, m, m);
        printf("  quadrática: x^2 - %lld x - 1 = 0     D = %lld\n", m, m*m+4);
        printf("  A_%lld = [[%lld, 1], [1, 0]]   det = -1\n", m, m);
        printf("  e o ponto fixo atrativo de x -> %lld + 1/x É sigma_%lld\n", m, m);
        return 0;
    }

    int cifrar = !strcmp(cmd, "cifra"), decifrar = !strcmp(cmd, "decifra");
    if(cifrar || decifrar){
        L k[KMAX]; int resto;
        int n = le_chave(argc, argv, 2, k, &resto);
        if(n < 1){ uso(); return 2; }
        if(resto + 1 >= argc){ uso(); return 2; }
        L x = atoll(argv[resto]), y = atoll(argv[resto+1]);
        L K00,K01,K10,K11; mob(k,n,&K00,&K01,&K10,&K11);
        L det = K00*K11 - K01*K10;
        if(det != 1 && det != -1){                    /* não pode acontecer, mas é dito */
            fprintf(stderr, "nomeia: det = %lld — a chave saiu de GL2(Z)\n", det);
            return 3;
        }
        if(cifrar){
            printf("%lld %lld\n", K00*x + K01*y, K10*x + K11*y);
        } else {
            printf("%lld %lld\n", ( K11*x - K01*y)/det, (-K10*x + K00*y)/det);
        }
        fprintf(stderr, "  chave: %d dígitos   K = [[%lld, %lld], [%lld, %lld]]   det = %lld\n",
                n, K00,K01,K10,K11, det);
        return 0;
    }

    uso();
    return 2;
}
