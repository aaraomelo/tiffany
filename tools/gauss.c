/* gauss.c — Z[i], A ORDEM DA TORÇÃO, E A MÖBIUS DUAL QUE RODA EM VEZ DE REFLETIR.
 *
 * O Aarão: "vamos justificar isso com o grupo Z[i] de Gauss na lemniscata discreta, daí
 * podemos mapear as órbitas da torção precisamente" · "eles têm a mesma cardinalidade, é
 * facto" · "precisa ver a ordem, a reversão — aí entra Möbius dual".
 *
 * E a coisa fecha num par que já estava escrito no projeto, mas nunca medido junto:
 *
 *     ANEL      UNIDADES        ORDEM   O QUE A REVERSÃO FAZ
 *     Z         {+1, −1}          2     ESPELHA — involução, ν∘ν = id
 *     Z[i]      {1, i, −1, −i}    4     RODA    — quatro passos para voltar
 *
 * A Möbius de GL2(Z) tem det = ±1 e reverte em DOIS. A Möbius dual — GL2(Z[i]), o grupo
 * de Picard — tem det numa unidade de Gauss e reverte em QUATRO. Mesma construção, mesma
 * exatidão, ordem diferente. É a terceira consequência do §1 medida na álgebra: subir de
 * dimensão não acrescenta tamanho, acrescenta FASE — e aqui a fase é literalmente o i.
 *
 * E a cardinalidade não muda: Z[i] é numerável como Z, e Z[i]^N tem exatamente a mesma
 * cardinalidade que N^N. O contínuo continua a ser a POTÊNCIA, e a torção continua a ser
 * o produto de Möbius — só que agora com quatro direções em vez de duas.
 *
 * A LEMNISCATA DISCRETA é onde isto tem nome próprio: assim como as raízes da unidade
 * dividem o CÍRCULO e são governadas por Z, os pontos de divisão da LEMNISCATA são
 * governados por Z[i] (é a multiplicação complexa de y² = x³ − x). O círculo tem duas
 * simetrias reais; a lemniscata tem quatro. Este medidor não constrói a lemniscata —
 * mede o GRUPO que a governa, que é o que a torção usa.
 *
 *   §G1  as unidades de Z[i] são EXATAMENTE quatro, e formam grupo cíclico de ordem 4
 *   §G2  Z[i] é euclidiano: a divisão com resto FUNCIONA, e Euclides termina
 *   §G3  a fracção contínua de Gauss: dígitos em Z[i], e a reversão é EXATA
 *   §G4  GL2(Z[i]): det é unidade ⟹ inversa inteira ⟹ a cifra estende-se
 *   §G5  A ORDEM: em Z a reversão fecha em 2; em Z[i] há torções que fecham em 4
 *   §G6  a cardinalidade não muda: Z[i] enumera-se sem repetição, como Z
 *   §G7  controlo negativo: em Z[i√5] a divisão com resto FALHA — não é o anel, é ESTE anel
 *
 *   cc -O2 -std=c99 -Wall gauss.c -o gauss && ./gauss
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

typedef long long L;
typedef struct { L a, b; } G;                 /* a + b i */

static G g_add(G x, G y){ return (G){x.a+y.a, x.b+y.b}; }
static G g_sub(G x, G y){ return (G){x.a-y.a, x.b-y.b}; }
static G g_mul(G x, G y){ return (G){x.a*y.a - x.b*y.b, x.a*y.b + x.b*y.a}; }
static G g_con(G x){ return (G){x.a, -x.b}; }
static L g_nrm(G x){ return x.a*x.a + x.b*x.b; }
static int g_eq(G x, G y){ return x.a==y.a && x.b==y.b; }

/* divisão com resto em Z[i]: q = arredondamento de x/y, r = x − q y, com N(r) < N(y).
 * x/y = x·conj(y)/N(y), e arredonda-se cada componente ao inteiro mais próximo. */
static int g_div(G x, G y, G *q, G *r){
    L n = g_nrm(y);
    if(n == 0) return 0;
    G t = g_mul(x, g_con(y));                 /* numerador */
    /* arredondar t.a/n e t.b/n ao inteiro mais próximo, com sinal */
    L qa = (2*t.a + (t.a>=0 ? n : -n)) / (2*n);
    L qb = (2*t.b + (t.b>=0 ? n : -n)) / (2*n);
    *q = (G){qa, qb};
    *r = g_sub(x, g_mul(*q, y));
    return 1;
}

#define KMAX 40

int main(void){
    printf("================================================================\n");
    printf("  Z[i]: a ordem da torção, e a Möbius que roda\n");
    printf("================================================================\n");

    /* ---------------- §G1 — as unidades são quatro, e formam Z/4 ---------------- */
    printf("\n§G1 as unidades de Z[i] são EXATAMENTE quatro, e o grupo é cíclico de ordem 4\n");
    {
        G un[8]; int n=0;
        for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++)
            if(g_nrm((G){a,b}) == 1 && n<8) un[n++] = (G){a,b};
        printf("      unidades (N = 1): ");
        for(int i=0;i<n;i++) printf("%+lld%+lldi  ", un[i].a, un[i].b);
        printf("\n");
        ok("são EXATAMENTE 4 — contra as 2 de Z", n==4);

        /* i gera: i, i², i³, i⁴ = 1, e nenhuma potência anterior é 1 */
        G i1 = {0,1}, p = {1,0}; int ordem = 0;
        printf("      potências de i: ");
        for(int k=1;k<=6;k++){
            p = g_mul(p, i1);
            printf("i^%d = %+lld%+lldi   ", k, p.a, p.b);
            if(g_eq(p,(G){1,0}) && !ordem) ordem = k;
        }
        printf("\n      ordem de i: %d\n", ordem);
        ok("i tem ordem 4: i,−1,−i,1 — e nenhuma potência antes fecha", ordem==4);

        /* e em Z, a unidade não-trivial tem ordem 2 */
        printf("      em Z: (−1)² = 1, ordem 2\n");
        ok("em Z a unidade não-trivial tem ordem 2 — ESPELHA, não roda", 1*1==1 && (-1)*(-1)==1);
        conclui("duas unidades espelham; quatro unidades rodam. É o mesmo mecanismo com um");
        conclui("quarto de volta em vez de meia — e é aqui que a fase entra na álgebra.");
    }

    /* ---------------- §G2 — Z[i] é euclidiano ---------------- */
    printf("\n§G2 Z[i] é euclidiano: a divisão com resto funciona, e N(r) < N(y)\n");
    {
        int casos=0, bons=0; L pior=0;
        for(L xa=-12;xa<=12;xa++) for(L xb=-12;xb<=12;xb++)
        for(L ya=-6;ya<=6;ya++) for(L yb=-6;yb<=6;yb++){
            G x={xa,xb}, y={ya,yb};
            if(g_nrm(y)==0) continue;
            G q,r;
            if(!g_div(x,y,&q,&r)) continue;
            casos++;
            /* a identidade tem de fechar EXATA e o resto tem de encolher */
            G volta = g_add(g_mul(q,y), r);
            if(g_eq(volta,x) && g_nrm(r) < g_nrm(y)) bons++;
            else if(g_nrm(r) >= g_nrm(y) && g_nrm(r) > pior) pior = g_nrm(r);
        }
        printf("      divisões testadas: %d   com x = qy + r E N(r) < N(y): %d\n", casos, bons);
        ok("a divisão com resto fecha EXATA e o resto encolhe sempre", bons==casos && casos>10000);
        conclui("é isto que faz Euclides terminar em Z[i] — e é por isso que a construção toda");
        conclui("se transporta: o que ela pede do anel é só isto.");
    }

    /* ---------------- §G3 — a fracção contínua de Gauss ---------------- */
    printf("\n§G3 a fracção contínua em Z[i]: dígitos gaussianos, e a reversão é EXATA\n");
    {
        int casos=0, exatos=0, maxk=0, no_tecto=0;
        for(L xa=-9;xa<=9;xa++) for(L xb=-9;xb<=9;xb++)
        for(L ya=1;ya<=6;ya++) for(L yb=-6;yb<=6;yb++){
            G x={xa,xb}, y={ya,yb};
            if(g_nrm(y)==0) continue;
            /* Euclides: dígitos a_k = quociente, e o resto vira o novo denominador */
            G a[KMAX]; int k=0; G p=x, q=y;
            while(g_nrm(q)!=0 && k<KMAX){
                G qu,r; if(!g_div(p,q,&qu,&r)) break;
                a[k++]=qu; p=q; q=r;
            }
            /* mesmo defeito do palavra.c §P1, e a mesma correção: os descartados contam-se,
             * e a asserção passa a ser sobre eles em vez de sobre os que sobreviveram. */
            if(k>=KMAX || g_nrm(q)!=0){ no_tecto++; continue; }
            casos++; if(k>maxk) maxk=k;
            /* reconstruir por Möbius sobre Z[i]: M = A_{a0}···A_{a_{k−1}} */
            G m00={1,0}, m01={0,0}, m10={0,0}, m11={1,0};
            for(int j=0;j<k;j++){
                G n00 = g_add(g_mul(m00,a[j]), m01), n01 = m00;
                G n10 = g_add(g_mul(m10,a[j]), m11), n11 = m10;
                m00=n00; m01=n01; m10=n10; m11=n11;
            }
            /* a 1ª coluna é (p', q') com p'/q' = x/y: verifica-se por produto cruzado */
            if(g_eq(g_mul(m00,y), g_mul(m10,x))) exatos++;
        }
        printf("      pares em Z[i] testados: %d   palavra mais longa: %d dígitos\n", casos, maxk);
        printf("      reconstruídos EXATOS (produto cruzado em Z[i], sem divisão): %d\n", exatos);
        printf("      que bateram no tecto de %d dígitos: %d\n", KMAX, no_tecto);
        ok("Euclides TERMINA em Z[i]: ZERO bateram no tecto", no_tecto==0 && casos>3000);
        ok("e a Möbius gaussiana devolve x/y EXATO — resíduo 0", exatos==casos);
    }

    /* ---------------- §G4 — GL2(Z[i]) e a cifra ---------------- */
    printf("\n§G4 GL2(Z[i]): det numa UNIDADE ⟹ inversa inteira ⟹ a cifra estende-se\n");
    {
        int chaves=0, casos=0, revertidos=0;
        int dets[4] = {0,0,0,0};                  /* quantas chaves com det 1, i, −1, −i */
        for(L k0a=-2;k0a<=2;k0a++) for(L k0b=-2;k0b<=2;k0b++)
        for(L k1a=-2;k1a<=2;k1a++) for(L k1b=-2;k1b<=2;k1b++){
            G ch[2] = { {k0a,k0b}, {k1a,k1b} };
            G K00={1,0}, K01={0,0}, K10={0,0}, K11={1,0};
            for(int j=0;j<2;j++){
                G n00 = g_add(g_mul(K00,ch[j]), K01), n01 = K00;
                G n10 = g_add(g_mul(K10,ch[j]), K11), n11 = K10;
                K00=n00; K01=n01; K10=n10; K11=n11;
            }
            G det = g_sub(g_mul(K00,K11), g_mul(K01,K10));
            if(g_nrm(det) != 1) continue;         /* tem de ser unidade */
            chaves++;
            if(g_eq(det,(G){1,0})) dets[0]++;
            else if(g_eq(det,(G){0,1})) dets[1]++;
            else if(g_eq(det,(G){-1,0})) dets[2]++;
            else dets[3]++;
            /* cifrar e decifrar: adj/det, e det é unidade ⟹ dividir é multiplicar pelo inverso */
            G invdet = g_con(det);                /* para |det|=1, det⁻¹ = conj(det) */
            for(L pa=-3;pa<=3;pa++) for(L pb=-3;pb<=3;pb++)
            for(L qa=-3;qa<=3;qa++) for(L qb=-3;qb<=3;qb++){
                G P={pa,pb}, Q={qa,qb};
                if(g_nrm(P)==0 && g_nrm(Q)==0) continue;
                casos++;
                G c1 = g_add(g_mul(K00,P), g_mul(K01,Q));
                G c2 = g_add(g_mul(K10,P), g_mul(K11,Q));
                G d1 = g_mul(invdet, g_sub(g_mul(K11,c1), g_mul(K01,c2)));
                G d2 = g_mul(invdet, g_add(g_mul((G){-K10.a,-K10.b},c1), g_mul(K00,c2)));
                if(g_eq(d1,P) && g_eq(d2,Q)) revertidos++;
            }
        }
        printf("      chaves de 2 dígitos com det unidade: %d   (det=1: %d, i: %d, −1: %d, −i: %d)\n",
               chaves, dets[0],dets[1],dets[2],dets[3]);
        printf("      pares cifrados: %d   decifrados EXATOS: %d\n", casos, revertidos);
        ok("toda chave de GL2(Z[i]) reverte exato — sem uma divisão", revertidos==casos && casos>10000);
        /* E aqui está um facto que a primeira versão deste medidor previu mal: as palavras
         * A_{a0}···A_{a_{k−1}} dão SEMPRE det = (−1)^k, mesmo com dígitos gaussianos —
         * porque cada A_a tem det −1, e isso não depende do anel. Logo os A_a sozinhos NÃO
         * GERAM GL2(Z[i]): alcançam só as duas unidades que Z já tinha.
         *
         * Para chegar às outras duas é preciso um gerador que Z não tem: a rotação
         * R = [[i,0],[0,1]], com det = i. É o §G5, e é o conteúdo de "a torção roda". */
        ok("as palavras de A_a dão det = (−1)^k SÓ — não dependem do anel",
           dets[0]==chaves && dets[1]==0 && dets[2]==0 && dets[3]==0);
        {
            /* com a rotação R à frente, as quatro unidades aparecem */
            int alcancadas[4]={0,0,0,0};
            G R00={0,1}, R01={0,0}, R10={0,0}, R11={1,0};
            G r00={1,0}, r01={0,0}, r10={0,0}, r11={1,0};
            for(int e=0;e<4;e++){
                G det = g_sub(g_mul(r00,r11), g_mul(r01,r10));
                if(g_eq(det,(G){1,0})) alcancadas[0]=1;
                else if(g_eq(det,(G){0,1})) alcancadas[1]=1;
                else if(g_eq(det,(G){-1,0})) alcancadas[2]=1;
                else if(g_eq(det,(G){0,-1})) alcancadas[3]=1;
                G n00=g_add(g_mul(r00,R00),g_mul(r01,R10)), n01=g_add(g_mul(r00,R01),g_mul(r01,R11));
                G n10=g_add(g_mul(r10,R00),g_mul(r11,R10)), n11=g_add(g_mul(r10,R01),g_mul(r11,R11));
                r00=n00;r01=n01;r10=n10;r11=n11;
            }
            printf("      com a rotação R = [[i,0],[0,1]], os dets alcançados: %d,%d,%d,%d (1,i,−1,−i)\n",
                   alcancadas[0],alcancadas[1],alcancadas[2],alcancadas[3]);
            ok("R gera as QUATRO unidades — é o gerador que Z não tem",
               alcancadas[0]&&alcancadas[1]&&alcancadas[2]&&alcancadas[3]);
        }
        conclui("é a mesma cifra do palavra.c §P9, com o anel trocado. O que ela pede não é Z:");
        conclui("é um anel euclidiano com unidades — e Z[i] tem quatro em vez de duas.");
    }

    /* ---------------- §G5 — A ORDEM: 2 em Z, 4 em Z[i] ---------------- */
    printf("\n§G5 A ORDEM da torção: em Z fecha em 2; em Z[i] há torções que fecham em 4\n");
    {
        /* A_0 = [[0,1],[1,0]] em Z: ordem 2 — ESPELHA.
         * R = [[i,0],[0,1]] em Z[i]: det = i, e R⁴ = I — RODA. */
        G A00={0,0}, A01={1,0}, A10={1,0}, A11={0,0};
        G m00=A00,m01=A01,m10=A10,m11=A11;
        int ordem_A = 0;
        for(int k=2;k<=8;k++){
            G n00 = g_add(g_mul(m00,A00), g_mul(m01,A10));
            G n01 = g_add(g_mul(m00,A01), g_mul(m01,A11));
            G n10 = g_add(g_mul(m10,A00), g_mul(m11,A10));
            G n11 = g_add(g_mul(m10,A01), g_mul(m11,A11));
            m00=n00;m01=n01;m10=n10;m11=n11;
            if(!ordem_A && g_eq(m00,(G){1,0}) && g_eq(m01,(G){0,0})
                        && g_eq(m10,(G){0,0}) && g_eq(m11,(G){1,0})) ordem_A = k;
        }
        printf("      A_0 = [[0,1],[1,0]] em Z:      ordem %d — ESPELHA\n", ordem_A);

        G R00={0,1}, R01={0,0}, R10={0,0}, R11={1,0};      /* [[i,0],[0,1]], det = i */
        G r00=R00,r01=R01,r10=R10,r11=R11;
        int ordem_R = 0;
        for(int k=2;k<=8;k++){
            G n00 = g_add(g_mul(r00,R00), g_mul(r01,R10));
            G n01 = g_add(g_mul(r00,R01), g_mul(r01,R11));
            G n10 = g_add(g_mul(r10,R00), g_mul(r11,R10));
            G n11 = g_add(g_mul(r10,R01), g_mul(r11,R11));
            r00=n00;r01=n01;r10=n10;r11=n11;
            if(!ordem_R && g_eq(r00,(G){1,0}) && g_eq(r01,(G){0,0})
                        && g_eq(r10,(G){0,0}) && g_eq(r11,(G){1,0})) ordem_R = k;
        }
        G detR = g_sub(g_mul(R00,R11), g_mul(R01,R10));
        printf("      R   = [[i,0],[0,1]] em Z[i]:   ordem %d — RODA   (det = %+lld%+lldi)\n",
               ordem_R, detR.a, detR.b);
        ok("em Z a torção fecha em DOIS: A_0² = I, e é involução", ordem_A==2);
        ok("em Z[i] há torção que fecha em QUATRO, e não antes", ordem_R==4);
        ok("e o det dela é i — a unidade que Z não tem", g_eq(detR,(G){0,1}));
        conclui("a reversão existe nos dois, e é exata nos dois. O que muda é EM QUANTOS PASSOS:");
        conclui("dois é o espelho, quatro é o quarto de volta. Mesma cardinalidade, outra fase —");
        conclui("que é exatamente a terceira consequência do enunciado, medida na álgebra.");
    }

    /* ---------------- §G6 — a cardinalidade não muda ---------------- */
    printf("\n§G6 a cardinalidade não muda: Z[i] enumera-se sem repetição, como Z\n");
    {
        /* enumeração em quadrados concêntricos: cada elemento aparece uma vez */
        enum { RAIO = 60 };
        static char visto[2*RAIO+1][2*RAIO+1];
        long n=0, repetidos=0;
        for(int r=0; r<=RAIO; r++)
            for(int a=-r; a<=r; a++) for(int b=-r; b<=r; b++){
                if(a!=-r && a!=r && b!=-r && b!=r) continue;   /* só a casca */
                if(visto[a+RAIO][b+RAIO]) { repetidos++; continue; }
                visto[a+RAIO][b+RAIO] = 1; n++;
            }
        long total = (2L*RAIO+1)*(2L*RAIO+1);
        printf("      elementos alcançados: %ld de %ld   repetições: %ld\n", n, total, repetidos);
        ok("Z[i] enumera-se com ZERO repetições — é numerável como Z", n==total && repetidos==0);
        conclui("logo Z[i]^N e N^N têm a MESMA cardinalidade, e o contínuo continua a ser a");
        conclui("potência. Trocar Z por Z[i] não faz o conjunto crescer — faz a torção rodar.");
    }

    /* ---------------- §G7 — o controlo negativo ---------------- */
    printf("\n§G7 controlo negativo: em Z[i√5] a divisão com resto FALHA\n");
    {
        /* Z[√−5] tem as mesmas duas unidades de Z mas NÃO é euclidiano: 6 = 2·3 = (1+√−5)(1−√−5).
         * Mede-se o que a construção realmente precisa — e não é "ser um anel". */
        int casos=0, falhou=0;
        for(L xa=-8;xa<=8;xa++) for(L xb=-8;xb<=8;xb++)
        for(L ya=-4;ya<=4;ya++) for(L yb=-4;yb<=4;yb++){
            L ny = ya*ya + 5*yb*yb;
            if(ny == 0) continue;
            casos++;
            /* x/y = x·conj(y)/N(y), com conj(a+b√−5) = a−b√−5 */
            L ta = xa*ya + 5*xb*yb, tb = xb*ya - xa*yb;
            L qa = (2*ta + (ta>=0?ny:-ny))/(2*ny), qb = (2*tb + (tb>=0?ny:-ny))/(2*ny);
            L ra = xa - (qa*ya - 5*qb*yb), rb = xb - (qa*yb + qb*ya);
            L nr = ra*ra + 5*rb*rb;
            if(nr >= ny) falhou++;                 /* o resto NÃO encolhe */
        }
        printf("      divisões em Z[√−5]: %d   em que o resto NÃO encolhe: %d  (%.1f%%)\n",
               casos, falhou, 100.0*falhou/casos);
        ok("em Z[√−5] o resto NÃO encolhe sempre — o anel não serve", falhou>0);
        conclui("a construção não pede 'um anel': pede EUCLIDIANO. Z e Z[i] são; Z[√−5] não é,");
        conclui("e neste medidor vê-se a percentagem exata em que a divisão desiste. É o que");
        conclui("impede o texto de dizer que a torção se transporta para qualquer anel.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
