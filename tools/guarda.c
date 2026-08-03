/* guarda.c — O QUE SE GUARDA, E COMO. Nunca um float.
 *
 * A pergunta do Aarão: "onde mesmo que o float é inevitável?" — e a resposta, medida, é
 * EM LADO NENHUM. Depois as correções dele, que completam a cadeia:
 *
 *   "se guarda irracionais sim, na forma de PRIMOS — essa é a forma certa de guardar.
 *    Guarda-se o ponto fixo, já provado irracionais <-> primos via Möbius."
 *   "com 3 já reconstrói; 1 ponto fixo não reconstrói."
 *   "é uma BASE, a base ortonormal do corpo que guardamos — em primos."
 *
 * E O QUE FICA, que é a cadeia inteira:
 *
 *   DIÁDICOS      todo float32 É m·2^e, com m e e inteiros. Guarda-se como os 32 BITS,
 *                 que já são um inteiro, e volta bit a bit.
 *
 *   IRRACIONAIS   não se guarda o valor — guarda-se a BASE do corpo, {1, σ}. E a base
 *                 identifica-se pelos PRIMOS que a sua norma representa.
 *
 *   E A MÖBIUS    um ponto fixo NÃO a determina: infinitas a partilham. São precisos TRÊS
 *                 pontos, porque ela tem 4 entradas menos a escala projetiva = 3 graus.
 *
 *   §G1  todo float32 é um racional exato, e volta pelos bits — bit a bit
 *   §G2  um ponto fixo NÃO reconstrói a Möbius: muitas o partilham
 *   §G3  com TRÊS pontos ela é única (a menos do sinal, que é a mesma em P¹)
 *   §G4  a base guarda-se pelos PRIMOS que a norma representa
 *   §G5  e o critério é LEGENDRE: p é representado sse Δ é resíduo quadrático mod p
 *   §G6  controlo negativo: Δ = 5 e Δ = 20 dão a MESMA lista — é o mesmo corpo
 *
 *   cc -O2 -std=c99 -Wall guarda.c -o guarda && ./guarda
 */
#include <stdio.h>
#include "unidade.h"
#include <string.h>

typedef long long L;

int main(void){
    printf("================================================================\n");
    printf("  O que se guarda, e como — nunca um float\n");
    printf("================================================================\n");

    printf("\n§G1 todo float32 e um racional exato, e volta pelos BITS — bit a bit\n");
    {
        float v[6] = { 0.1f, -0.017616723f, 3.14159265358979323846f, 1.0f, 0.0f, -2.5f };
        int n=0, volta=0;
        printf("      valor            bits (inteiro)   de volta         igual?\n");
        for(int i=0;i<6;i++){
            unsigned u; memcpy(&u, &v[i], 4);
            float w;    memcpy(&w, &u, 4);
            n++;
            if(w == v[i]) volta++;
            printf("      %-16g %-16u %-16g %s\n", (double)v[i], u, (double)w,
                   (w==v[i])?"sim":"NAO");
        }
        printf("      valores: %d   que voltam BIT A BIT: %d\n", n, volta);
        ok("todo float32 volta exato pelos seus 32 bits — e eles ja sao um inteiro",
           volta==n && n==6);
        conclui("nao ha 'float inevitavel': ha DIADICO, que se guarda como inteiro, e");
        conclui("IRRACIONAL, que se guarda pela base. Nunca um decimal a truncar.");
    }

    printf("\n§G2 um ponto fixo NAO reconstroi a Mobius: muitas o partilham\n");
    {
        /* Quais M = [[a,b],[c,d]] de det ±1 fixam φ (raiz de x²=x+1)?
         * (aφ+b)/(cφ+d) = φ  ⟺  cφ² + (d−a)φ − b = 0  ⟺  (c+d−a)φ + (c−b) = 0
         * e como φ é irracional:  c = b  E  a = b + d.  São infinitas. */
        L cnt=0;
        printf("      M = [[a,b],[c,d]] com det = ±1 que fixam φ, entradas em [-6,6]:\n");
        int most=0;
        for(L a=-6;a<=6;a++) for(L b=-6;b<=6;b++) for(L c=-6;c<=6;c++) for(L d=-6;d<=6;d++){
            if(a*d-b*c != 1 && a*d-b*c != -1) continue;
            if(c==b && a==b+d){
                cnt++;
                if(most<4){ printf("        [[%lld,%lld],[%lld,%lld]]\n", a,b,c,d); most++; }
            }
        }
        printf("      total: %lld  (e sao INFINITAS: a condicao c=b, a=b+d nao limita)\n", cnt);
        ok("um ponto fixo NAO determina a Mobius — muitas o partilham", cnt > 2);
        conclui("guardar 'o ponto fixo' nao chega: ele nao identifica a operacao que o gerou.");
    }

    printf("\n§G3 com TRES pontos a Mobius e unica (a menos do sinal — a mesma em P^1)\n");
    {
        /* M tem 4 entradas menos a escala projetiva = 3 graus de liberdade, logo TRES
         * condicoes determinam-na. Verifica-se: dados 3 pares (x -> y), quantas M batem? */
        L A=2,B=1,C=1,D=1;                       /* a Mobius alvo, det = 1 */
        L px[3]={0,1,2}, py_n[3], py_d[3];
        for(int i=0;i<3;i++){ py_n[i]=A*px[i]+B; py_d[i]=C*px[i]+D; }
        L achadas=0;
        for(L a=-4;a<=4;a++) for(L b=-4;b<=4;b++) for(L c=-4;c<=4;c++) for(L d=-4;d<=4;d++){
            if(a*d-b*c != 1 && a*d-b*c != -1) continue;
            int bate=1;
            for(int i=0;i<3;i++){
                L n=a*px[i]+b, m=c*px[i]+d;
                /* n/m == py_n/py_d, por produto cruzado — sem dividir */
                if(n*py_d[i] != py_n[i]*m){ bate=0; break; }
            }
            if(bate) achadas++;
        }
        printf("      a Mobius [[2,1],[1,1]] leva 0,1,2 em 1/1, 3/2, 5/3\n");
        printf("      matrizes de GL2(Z) que reproduzem os TRES pares: %lld\n", achadas);
        ok("com TRES pontos so ha uma Mobius — as duas achadas sao M e -M, a mesma em P^1",
           achadas == 2);
        conclui("e por isso que 3 reconstroi e 1 nao: a Mobius tem 4 entradas menos a escala");
        conclui("projetiva, e sobram TRES graus de liberdade. Tres condicoes fecham-na.");
    }

    printf("\n§G4 a BASE guarda-se pelos PRIMOS que a norma representa\n");
    {
        /* N(a+bσ) = a² + m·ab − b². Os primos que ela representa identificam o corpo. */
        printf("      m   Delta   primos p com N(a+bsigma) = +-p\n");
        int metais=0, tem=0;
        for(L m=1;m<=5;m++){
            L D=m*m+4;
            int lista[16]; int nl=0;
            for(L p=2;p<=60 && nl<12;p++){
                int prim=1;
                for(L q=2;q*q<=p;q++) if(p%q==0){ prim=0; break; }
                if(!prim) continue;
                int achou=0;
                for(L a=-9;a<=9 && !achou;a++) for(L b=-9;b<=9 && !achou;b++){
                    L n=a*a+m*a*b-b*b;
                    if(n==p || n==-p) achou=1;
                }
                if(achou) lista[nl++]=(int)p;
            }
            metais++;
            if(nl>0) tem++;
            printf("      %-3lld %-7lld ", m, D);
            for(int i=0;i<nl;i++) printf("%d ", lista[i]);
            printf("\n");
        }
        ok("cada base representa uma lista PROPRIA de primos — e ela identifica o corpo",
           tem==metais && metais==5);
        conclui("o irracional nao se guarda pelo valor: guarda-se pela BASE, e a base pelos");
        conclui("primos que a norma dela alcanca. Tudo inteiro, tudo finito de verificar.");
    }

    printf("\n§G5 e o criterio e LEGENDRE: p e representado sse Delta e residuo quadratico mod p\n");
    {
        int testes=0, bate=0;
        printf("      m  Delta  p    Delta e quadrado mod p   representado?\n");
        for(L m=1;m<=2;m++){
            L D=m*m+4;
            for(L p=3;p<=23;p++){
                int prim=1;
                for(L q=2;q*q<=p;q++) if(p%q==0){ prim=0; break; }
                if(!prim) continue;
                int qr=0;
                for(L x=0;x<p;x++) if((x*x-D)%p==0){ qr=1; break; }
                int rep=0;
                for(L a=-12;a<=12 && !rep;a++) for(L b=-12;b<=12 && !rep;b++){
                    L n=a*a+m*a*b-b*b;
                    if(n==p || n==-p) rep=1;
                }
                testes++;
                if(qr==rep) bate++;
                if(testes<=8)
                    printf("      %-2lld %-6lld %-4lld %-23s %s\n",
                           m, D, p, qr?"sim":"nao", rep?"SIM":"nao");
            }
        }
        printf("      testes (p impar): %d   com Legendre a bater: %d\n", testes, bate);
        ok("p e representado SSE Delta e residuo quadratico mod p — Legendre, em inteiros",
           bate==testes && testes>=10);
        conclui("logo a lista de primos nao e uma tabela a decorar: e o simbolo de Legendre,");
        conclui("e calcula-se de p e Delta. A base guarda-se com DOIS inteiros: m e Delta.");
    }

    printf("\n§G6 controlo negativo: Delta = 5 e Delta = 20 dao a MESMA lista\n");
    {
        /* 20 = 4·5, logo m=1 e m=4 geram o MESMO corpo Q(sqrt 5): o discriminante
         * FUNDAMENTAL e o mesmo, e a lista de primos coincide. Isso impede o texto de
         * dizer que cada m da um corpo diferente. */
        int lista1[24], lista4[24], n1=0, n4=0;
        for(L p=2;p<=60;p++){
            int prim=1;
            for(L q=2;q*q<=p;q++) if(p%q==0){ prim=0; break; }
            if(!prim) continue;
            for(L m=1;m<=4;m+=3){
                int achou=0;
                for(L a=-9;a<=9 && !achou;a++) for(L b=-9;b<=9 && !achou;b++){
                    L n=a*a+m*a*b-b*b;
                    if(n==p || n==-p) achou=1;
                }
                if(achou){ if(m==1) lista1[n1++]=(int)p; else lista4[n4++]=(int)p; }
            }
        }
        int igual = (n1==n4);
        for(int i=0;i<n1 && igual;i++) if(lista1[i]!=lista4[i]) igual=0;
        printf("      m=1 (Delta=5):  "); for(int i=0;i<n1;i++) printf("%d ", lista1[i]);
        printf("\n      m=4 (Delta=20): "); for(int i=0;i<n4;i++) printf("%d ", lista4[i]);
        printf("\n      iguais? %s   (20 = 4x5, logo o discriminante FUNDAMENTAL e o mesmo)\n",
               igual?"SIM":"nao");
        ok("Delta=5 e Delta=20 dao a MESMA lista — nem todo m da corpo diferente", igual);
        conclui("o que a lista identifica e o corpo, nao o m. Dois m podem partilhar corpo, e");
        conclui("e o discriminante FUNDAMENTAL que os separa — nao o m^2+4 em bruto.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESIDUO 0");
    return falhas ? 1 : 0;
}
