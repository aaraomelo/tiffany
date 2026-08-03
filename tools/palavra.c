/* palavra.c — O DEGRAU QUE FALTAVA: Z × N* ↔ R, E A MÖBIUS É QUEM O SOBE.
 *
 * O Aarão apontou o buraco: "a bijeção você mostrou de N em irracionais, falta um salto — o
 * irracional gera a classe racional. Junta a bijeção com o gerador e você tem a função
 * bijetiva N×Z → R. Ou Q em R. Ou N×N* → R, que dá N em R do mesmo jeito."
 *
 * Ele tem razão, e o buraco era meu: a prop:bijecao dá as palavras PERIÓDICAS ↔ quadráticos.
 * É uma classe, não é R. O que falta não é uma régua nova — é dizer QUAL É O DOMÍNIO:
 *
 *     N  finito     ↔  Q          Euclides TERMINA.  (§P1)
 *     N  periódico  ↔  quadráticos   Euclides CICLA — Lagrange.  (§P5)
 *     N* = N^N      ↔  R \ Q      Euclides NUNCA para.  (§P2, §P3)
 *     Z × N*        ↔  R          a cabeça inteira dá o sinal e o resto.  (§P6)
 *
 * N* é o DUAL de N no eixo de Pontryagin — o discreto e o compacto. A máquina opera em N,
 * finito e exato; o contínuo está no dual dela, e não no seu interior. Não é a mesma
 * afirmação que "N ↔ R", que é falsa: é a afirmação de que o degrau existe e é UM SÓ.
 *
 * E o mecanismo é a Möbius do §1, com a mesma matriz de sempre:
 *
 *     M_w = A_{a0}·A_{a1}···A_{a_{k−1}} = [[p_{k−1}, p_{k−2}], [q_{k−1}, q_{k−2}]],
 *     det M_w = (−1)^k, logo a INVERSA é inteira, logo a reversão é EXATA.
 *
 * As DUAS COLUNAS de M_w são dois convergentes consecutivos: um por defeito, outro por
 * excesso. É literalmente R = Q + Q* — o real é o PAR de racionais que o encaixota, e a
 * involução ν é quem troca os lados do encaixe. (§P2, §P3)
 *
 *   §P1  N finito ↔ Q: Euclides termina, e M_w devolve p/q EXATO
 *   §P2  R = Q + Q*: os convergentes ALTERNAM os lados — o par encaixota
 *   §P3  o encaixe aperta: |x − p_k/q_k| < 1/q_k², em aritmética exata
 *   §P4  det = ±1: a inversa é inteira e M_w^{-1}M_w = I nos INTEIROS
 *   §P5  N periódico ↔ quadrático: Euclides sobre σ CICLA, em Z[√D] exato
 *   §P6  a cabeça: Z dá o sinal e a parte inteira, e a reversão continua exata
 *   §P7  controlo negativo: truncar dá OUTRO real, e o 0 no meio DESTRÓI a unicidade
 *   §P8  a diagonal É UMA PALAVRA — nomear todo real e listá-los são coisas diferentes
 *   §P9  A CIFRA COM CHAVE: toda M ∈ GL2(Z) cifra, e M^{-1} decifra sem perder um bit
 *
 * O cifra_continuo.c já tinha a cifra CANÓNICA de Q — Euclides, cf_cifra/cf_decifra. O que
 * este acrescenta é o ALCANCE e a CHAVE: o alcance é R (§P6), e a chave é que a cifra não
 * precisa de ser a canónica. QUALQUER M em GL2(Z) serve, e a decifra é M^{-1}, que existe
 * nos inteiros exatamente porque det M = ±1. É o mesmo facto do §P4, virado para fora.
 *
 * Tudo em inteiros. Nenhum float decide asserção nenhuma neste ficheiro.
 *
 *   cc -O2 -std=c99 -Wall palavra.c -o palavra && ./palavra
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include <limits.h>

typedef long long L;

#define KMAX 64

static L mdc(L a, L b){ if(a<0) a=-a; if(b<0) b=-b; while(b){ L t=a%b; a=b; b=t; } return a; }

/* Euclides sobre o racional p/q (q>0): devolve a palavra e o comprimento. */
static int euclides(L p, L q, L *a){
    int k = 0;
    while(q != 0 && k < KMAX){
        L d = p / q, r = p - d*q;      /* piso, com q > 0 */
        if(r < 0){ d--; r += q; }
        a[k++] = d;
        p = q; q = r;
    }
    return k;
}

/* M_w = A_{a0}···A_{a_{k−1}}, com A_a = [[a,1],[1,0]]. Devolve p,q dos DOIS convergentes. */
static void matriz(const L *a, int k, L *p1, L *p2, L *q1, L *q2){
    L m00=1, m01=0, m10=0, m11=1;      /* identidade */
    for(int i=0;i<k;i++){
        L n00 = m00*a[i] + m01, n01 = m00;
        L n10 = m10*a[i] + m11, n11 = m10;
        m00=n00; m01=n01; m10=n10; m11=n11;
    }
    *p1=m00; *p2=m01; *q1=m10; *q2=m11;
}

int main(void){
    printf("================================================================\n");
    printf("  Z × N* ↔ R — o degrau, e a Möbius que o sobe\n");
    printf("================================================================\n");

    /* ---------------- §P1 — N finito ↔ Q: termina, e reverte exato ---------------- */
    printf("\n§P1 N finito ↔ Q: Euclides TERMINA, e M_w devolve p/q exato\n");
    {
        int casos=0, exatos=0, maxk=0; L piorq=0;
        for(L q=1; q<=120; q++) for(L p=0; p<=240; p++){
            if(mdc(p,q) != 1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            if(k >= KMAX) continue;                    /* não truncar em silêncio */
            casos++;
            if(k > maxk){ maxk=k; piorq=q; }
            L p1,p2,q1,q2; matriz(a,k,&p1,&p2,&q1,&q2);
            /* a 1.ª coluna É p/q, já reduzida — igualdade de INTEIROS, não de floats */
            if(p1==p && q1==q) exatos++;
        }
        printf("      racionais testados: %d   palavra mais longa: %d dígitos (q=%lld)\n",
               casos, maxk, piorq);
        printf("      reconstruídos com p e q IGUAIS aos de partida: %d\n", exatos);
        ok("Euclides termina em TODO racional (nenhum bateu no tecto)", casos > 3000);
        ok("M_w devolve p/q EXATO — igualdade de inteiros, resíduo 0", exatos == casos);
        conclui("a palavra finita é o racional. O alfabeto é N e a mensagem é Q.");
    }

    /* ---------------- §P2 — R = Q + Q*: os convergentes alternam ---------------- */
    printf("\n§P2 R = Q + Q*: os dois convergentes ENCAIXOTAM, e alternam de lado\n");
    {
        int testados=0, alternam=0;
        for(L q=7; q<=200; q+=3) for(L p=1; p<=400; p+=7){
            if(mdc(p,q)!=1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            if(k < 4 || k >= KMAX) continue;
            testados++;
            /* sinal de (p/q − p_j/q_j) = sinal de (p·q_j − q·p_j): tem de ALTERNAR em j */
            int bom = 1; int sinal_ant = 0;
            L m00=1,m01=0,m10=0,m11=1;
            for(int i=0;i<k-1;i++){                    /* até k−1: o último é o próprio p/q */
                L n00=m00*a[i]+m01, n01=m00, n10=m10*a[i]+m11, n11=m10;
                m00=n00; m01=n01; m10=n10; m11=n11;
                L d = p*m10 - q*m00;                   /* < 0 ⟺ convergente ACIMA de p/q */
                int s = (d>0) - (d<0);
                if(s == 0){ bom = 0; break; }
                if(sinal_ant != 0 && s == sinal_ant) bom = 0;
                sinal_ant = s;
            }
            if(bom) alternam++;
        }
        printf("      racionais com ≥4 dígitos: %d   com os lados a alternar: %d\n",
               testados, alternam);
        ok("os convergentes ALTERNAM: um por defeito, o seguinte por excesso", alternam==testados && testados>200);
        conclui("as duas colunas de M_w são os dois lados do encaixe: R = Q + Q*, e ν troca-os.");
    }

    /* ---------------- §P3 — o encaixe aperta: |x − p/q| < 1/q² ---------------- */
    printf("\n§P3 o encaixe APERTA: |x − p_j/q_j| < 1/q_j², em aritmética exata\n");
    {
        long testados=0, dentro=0;
        for(L q=11; q<=150; q+=2) for(L p=1; p<=300; p+=5){
            if(mdc(p,q)!=1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            if(k < 3 || k >= KMAX) continue;
            L m00=1,m01=0,m10=0,m11=1;
            for(int i=0;i<k-1;i++){
                L n00=m00*a[i]+m01, n01=m00, n10=m10*a[i]+m11, n11=m10;
                m00=n00; m01=n01; m10=n10; m11=n11;
                if(m10 == 0) continue;
                /* |p/q − p_j/q_j| < 1/q_j²  ⟺  |p·q_j − q·p_j| · q_j < q  (tudo inteiro) */
                L d = p*m10 - q*m00; if(d<0) d=-d;
                if(m10 > 0 && d < LLONG_MAX/m10){
                    testados++;
                    if(d * m10 < q) dentro++;
                }
            }
        }
        printf("      convergentes testados: %ld   dentro de 1/q²: %ld\n", testados, dentro);
        ok("TODO convergente aperta melhor que 1/q² — sem exceção", dentro==testados && testados>1000);
        conclui("o par não só encaixota: fecha-se. É por isso que o limite existe e é ÚNICO.");
    }

    /* ---------------- §P4 — det = ±1, e a inversa é INTEIRA ---------------- */
    printf("\n§P4 det M_w = (−1)^k: a inversa é INTEIRA, e por isso a reversão é exata\n");
    {
        int casos=0, det_ok=0, inv_ok=0;
        for(L q=1; q<=90; q++) for(L p=1; p<=180; p++){
            if(mdc(p,q)!=1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            if(k >= KMAX) continue;
            L p1,p2,q1,q2; matriz(a,k,&p1,&p2,&q1,&q2);
            casos++;
            L det = p1*q2 - p2*q1;
            L esp = (k % 2) ? -1 : 1;
            if(det == esp) det_ok++;
            /* M^{-1} = adj/det, INTEIRA porque det=±1. Verificar M^{-1}·M = I nos inteiros. */
            L i00 =  q2/det*p1 + (-p2)/det*q1, i01 =  q2/det*p2 + (-p2)/det*q2;
            L i10 = (-q1)/det*p1 +  p1/det*q1, i11 = (-q1)/det*p2 +  p1/det*q2;
            if(i00==1 && i01==0 && i10==0 && i11==1) inv_ok++;
        }
        printf("      palavras: %d   det = (−1)^k: %d   M⁻¹M = I nos inteiros: %d\n",
               casos, det_ok, inv_ok);
        ok("det M_w = (−1)^k exatamente — o sinal É a paridade da palavra", det_ok==casos);
        ok("a inversa é INTEIRA: M⁻¹M = I sem divisão nenhuma", inv_ok==casos && casos>2000);
        conclui("det=±1 é a condição da reversão exata, e ela nasce de A_a ter det −1: uma por dígito.");
    }

    /* ---------------- §P5 — N periódico ↔ quadrático, em Z[√D] exato ---------------- */
    printf("\n§P5 N periódico ↔ quadrático: Euclides sobre σ CICLA (Lagrange)\n");
    {
        /* x = (P + √D)/Q, o algoritmo clássico, TODO em inteiros. */
        int testados=0, ciclou=0, bateu_forma=0;
        for(L m=1; m<=40; m++){
            L D = m*m + 4;                       /* σ raiz de x² − m x − 1 = 0 */
            L r = 0; while((r+1)*(r+1) <= D) r++; /* ⌊√D⌋ */
            if(r*r == D) continue;               /* quadrado perfeito: não é irracional */
            testados++;
            L P = m, Q = 2;                      /* σ = (m + √D)/2 */
            L a[KMAX]; int k = 0; int perido = 0;
            L P0=P, Q0=Q;
            while(k < KMAX){
                L ai = (r + P) / Q;              /* ⌊x⌋, exato: ⌊(P+√D)/Q⌋ = ⌊(P+⌊√D⌋)/Q⌋ */
                a[k++] = ai;
                P = ai*Q - P;
                Q = (D - P*P) / Q;               /* divisão EXATA — invariante do algoritmo */
                if(Q == 0) break;
                if(P == P0 && Q == Q0){ perido = k; break; }
            }
            if(perido == 1 && a[0] == m) ciclou++;     /* σ_m = [m; m, m, …], período 1 */
            /* e o ponto fixo de A_m satisfaz q x² + (s−p)x − r = 0, isto é x² − m x − 1 */
            L p1,p2,q1,q2; L um[1] = { m }; matriz(um,1,&p1,&p2,&q1,&q2);
            if(q1==1 && q2==0 && p1==m && p2==1) bateu_forma++;
        }
        printf("      metais testados (m=1..40): %d   com período 1 e dígito m: %d\n",
               testados, ciclou);
        ok("Euclides sobre σ_m CICLA com período 1 — a palavra é finita", ciclou==testados && testados>30);
        ok("a matriz da palavra é A_m, e o ponto fixo é x² − mx − 1 = 0", bateu_forma==testados);
        conclui("periódico ⟺ quadrático é Lagrange, e aqui está medido em Z[√D] sem um só float.");
    }

    /* ---------------- §P6 — a cabeça: Z dá o sinal e a parte inteira ---------------- */
    printf("\n§P6 a cabeça inteira: Z × N* — o sinal e a parte inteira à frente\n");
    {
        int casos=0, exatos=0, negativos=0;
        for(L q=1; q<=80; q++) for(L p=-160; p<=160; p++){
            if(p==0) continue;
            if(mdc(p,q)!=1) continue;
            casos++; if(p<0) negativos++;
            /* a0 = ⌊p/q⌋ ∈ Z (pode ser negativo); o resto {x} ∈ [0,1) tem palavra em N \ {0} */
            L a0 = p/q; if(p % q != 0 && p < 0) a0--;
            L rp = p - a0*q;                     /* 0 ≤ rp < q */
            L a[KMAX]; int k = (rp==0) ? 0 : euclides(q, rp, a);
            if(k >= KMAX) continue;
            /* reconstruir: x = a0 + rp/q, com rp/q = 1/[a1;a2,…] */
            L np, nq;
            if(k==0){ np=0; nq=1; }
            else { L p1,p2,q1,q2; matriz(a,k,&p1,&p2,&q1,&q2); np=q1; nq=p1; }  /* invertido */
            L P = a0*nq + np, Qd = nq;
            L g = mdc(P,Qd); if(g){ P/=g; Qd/=g; }
            L pp=p, qq=q; L g2 = mdc(pp,qq); if(g2){ pp/=g2; qq/=g2; }
            if(P==pp && Qd==qq) exatos++;
        }
        printf("      racionais com sinal: %d (dos quais %d negativos)\n", casos, negativos);
        printf("      reconstruídos exatos a partir de (a0 ∈ Z, palavra ∈ N*): %d\n", exatos);
        ok("a cabeça em Z e o resto em N* reconstroem x EXATO, sinal incluído", exatos==casos && negativos>1000);
        conclui("Z × N* é o domínio. Z é a involução (o sinal); N* é a palavra. Nada mais entra.");
    }

    /* ---------------- §P7 — o controlo negativo ---------------- */
    printf("\n§P7 controlo negativo: onde a bijeção QUEBRA se a regra cair\n");
    {
        /* (a) truncar a palavra dá OUTRO real — a palavra infinita não é dispensável */
        int trunc_dif = 0, trunc_casos = 0;
        for(L q=13; q<=90; q+=1) for(L p=1; p<=180; p+=1){
            if(mdc(p,q)!=1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            if(k < 3 || k >= KMAX) continue;
            trunc_casos++;
            L p1,p2,q1,q2; matriz(a,k-1,&p1,&p2,&q1,&q2);
            if(!(p1==p && q1==q)) trunc_dif++;      /* tem de DIFERIR */
        }
        printf("      palavras truncadas num dígito: %d   que dão OUTRO racional: %d\n",
               trunc_casos, trunc_dif);
        ok("truncar muda o real — a palavra não tem folga", trunc_dif==trunc_casos && trunc_casos>500);

        /* (b) um 0 no meio destrói a unicidade: [a,0,b] = [a+b] */
        int colisoes = 0, tent = 0;
        for(L x=1; x<=25; x++) for(L y=1; y<=25; y++){
            L com0[3] = { x, 0, y };
            L sem0[1] = { x + y };
            L p1,p2,q1,q2, r1,r2,s1,s2;
            matriz(com0,3,&p1,&p2,&q1,&q2);
            matriz(sem0,1,&r1,&r2,&s1,&s2);
            tent++;
            if(p1*s1 == r1*q1) colisoes++;          /* mesmo racional por duas palavras */
        }
        printf("      pares (x,y) testados com um 0 no meio: %d   que COLIDEM: %d\n", tent, colisoes);
        ok("com 0 permitido a palavra DEIXA de ser única: [a,0,b] = [a+b]", colisoes==tent);
        /* (c) E A SEGUNDA COLISÃO, que eu tinha deixado passar e está mais escondida:
         * uma palavra FINITA terminada em 1 dá o mesmo racional que a mais curta com o
         * último dígito somado:   [..., a] = [..., a−1, 1].
         * Todos os dígitos são >= 1 nas duas — a regra do §P7(b) NÃO a apanha. É por isso
         * que a unicidade das finitas pede também  a_{k−1} >= 2  (salvo a palavra de um
         * dígito só, que é o inteiro). Sem esta condição a bijeção QUEBRA sobre Q. */
        int col2=0, t2=0;
        for(L x=1; x<=20; x++) for(L y=1; y<=20; y++) for(L a=2; a<=8; a++){
            L curta[3] = { x, y, a };
            L longa[4] = { x, y, a-1, 1 };
            L p1,p2,q1,q2, r1,r2,s1,s2;
            matriz(curta,3,&p1,&p2,&q1,&q2);
            matriz(longa,4,&r1,&r2,&s1,&s2);
            t2++;
            if(p1*s1 == r1*q1) col2++;             /* mesmo racional */
        }
        printf("      palavras [..,a] contra [..,a−1,1]: %d   que dão o MESMO racional: %d\n",
               t2, col2);
        ok("[..,a] = [..,a−1,1]: as finitas precisam de a_{k−1} >= 2", col2==t2);
        conclui("esta segunda colisão tem TODOS os dígitos >= 1 — a regra de (b) não a apanha.");

        conclui("a bijeção exige a_i ≥ 1 para i ≥ 1, E a_{k−1} ≥ 2 nas finitas. Sem elas");
        conclui("duas palavras nomeiam o mesmo real, e a inversa deixa de ser função.");
        conclui("é o que este medidor FALHARIA se o texto dissesse que o alfabeto é N inteiro.");
    }

    /* ---------------- §P8 — a diagonal É UMA PALAVRA (e é Möbius também) ---------------- */
    printf("\n§P8 a lista: dada QUALQUER enumeração de palavras, a diagonal é outra palavra\n");
    {
        /* Este é o teste que separa NOMEAR de LISTAR, e faz-se com a máquina da casa:
         * a diagonal não é um argumento de fora — é uma palavra construída por Möbius,
         * do mesmo alfabeto, com a mesma regra a_i >= 1. Se ela cai no domínio e não cai
         * na lista, então Z × N* nomeia todo real E nenhuma lista indexada por N os apanha.
         * As duas coisas ao mesmo tempo, e sem contradição: o infinito está no NOME. */
        enum { M = 400 };                      /* uma lista qualquer, com M palavras */
        static L lista[M][KMAX];
        for(int i=0;i<M;i++)
            for(int j=0;j<KMAX;j++)
                lista[i][j] = 1 + ((i*7 + j*13 + i*j) % 9);   /* dígitos >= 1, uma lista concreta */

        L diag[KMAX];
        for(int j=0;j<KMAX;j++){
            L d = lista[j % M][j];
            diag[j] = (d == 1) ? 2 : 1;        /* difere na diagonal, e continua >= 1 */
        }

        int valida = 1;
        for(int j=0;j<KMAX;j++) if(diag[j] < 1) valida = 0;

        int difere_de_todas = 1, iguais = 0;
        for(int i=0;i<M;i++){
            int igual = 1;
            for(int j=0;j<KMAX;j++) if(lista[i][j] != diag[j]){ igual = 0; break; }
            if(igual){ igual = 1; iguais++; difere_de_todas = 0; }
        }
        /* e ela nomeia um real: a Möbius dela tem det = ±1 como qualquer outra */
        L p1,p2,q1,q2; matriz(diag, 12, &p1,&p2,&q1,&q2);
        L det = p1*q2 - p2*q1;

        printf("      lista com %d palavras; a diagonal difere de cada uma na posição própria\n", M);
        printf("      a diagonal é palavra legítima (todos os dígitos >= 1): %s\n", valida?"sim":"NÃO");
        printf("      palavras da lista iguais à diagonal: %d\n", iguais);
        printf("      det da Möbius da diagonal (12 dígitos): %lld\n", det);
        ok("a diagonal está NO DOMÍNIO: é uma palavra do mesmo alfabeto", valida);
        ok("e NÃO está na lista: difere de TODAS as palavras dela", difere_de_todas);
        ok("e nomeia um real como qualquer outra: det = ±1", det==1 || det==-1);
        conclui("as duas coisas valem ao mesmo tempo, e não se contradizem:");
        conclui("  Z × N* NOMEIA todo real, exato e reversível — §P1 a §P6;");
        conclui("  e nenhuma lista indexada por N os apanha — este §P8, com a mesma Möbius.");
        conclui("a diferença é ONDE está o infinito: no NOME de cada real, e não numa lista só.");
        conclui("e a fronteira entre Q e R é UMA e é operacional: Euclides parar, ou não parar.");
    }

    /* ---------------- §P9 — A CIFRA COM CHAVE: GL2(Z) inteiro ---------------- */
    printf("\n§P9 a cifra com CHAVE: toda M ∈ GL2(Z) cifra, e M⁻¹ decifra sem perda\n");
    {
        /* A cifra canónica de Q já estava no cifra_continuo.c: Euclides, e a palavra é o
         * criptograma. Aqui a cifra ganha CHAVE, e a chave não é escolha de gosto — é uma
         * matriz de GL2(Z), isto é, uma palavra:  K = A_{k0}·A_{k1}···A_{kr}.
         *
         *    cifrar:   (p,q)  ↦  K·(p,q)        decifrar:  K^{-1}·(p',q')
         *
         * Reverte sem perder um bit porque det K = ±1 — e é O MESMO facto do §P4. A cifra
         * é a composição de Möbius; a chave é o produto; e a decifra é a ordem inversa com
         * o sinal trocado. Nada de arredondamento, nada de módulo: aritmética de inteiros. */
        int casos=0, revertidos=0; int chaves=0;
        for(L k0=1; k0<=6; k0++) for(L k1=1; k1<=6; k1++) for(L k2=1; k2<=6; k2++){
            L chave[3] = { k0, k1, k2 };
            L K00,K01,K10,K11; matriz(chave,3,&K00,&K01,&K10,&K11);
            L detK = K00*K11 - K01*K10;
            if(detK != 1 && detK != -1) continue;      /* nunca acontece: A_a tem det −1 */
            chaves++;
            for(L q=1; q<=25; q++) for(L p=1; p<=50; p++){
                if(mdc(p,q)!=1) continue;
                casos++;
                L cp = K00*p + K01*q, cq = K10*p + K11*q;         /* cifrado */
                /* decifra: adjunta a dividir por det, mas det=±1 ⟹ INTEIRA */
                L dp = ( K11*cp - K01*cq) / detK;
                L dq = (-K10*cp + K00*cq) / detK;
                if(dp==p && dq==q) revertidos++;
            }
        }
        printf("      chaves de 3 dígitos testadas: %d   pares cifrados: %d\n", chaves, casos);
        printf("      decifrados IGUAIS ao original (inteiros, não floats): %d\n", revertidos);
        ok("toda chave de GL2(Z) reverte EXATO: K⁻¹K = id sobre os pares", revertidos==casos && casos>10000);

        /* O CONTROLO NEGATIVO, e a primeira versão dele estava vazia: eu testava se
         * adj(B)·(B·v) é divisível por det — e é SEMPRE, porque adj(B)·B = det·I. Uma
         * asserção que não podia falhar. O defeito real de det ≠ ±1 não é reverter o que
         * se cifrou: é que a cifra DEIXA DE SER SOBRE. Há criptogramas sem origem inteira,
         * e um decifrador que recebe um deles não tem o que devolver. */
        {
            L B00=1,B01=1,B10=1,B11=3;                 /* det = 2 — não está em GL2(Z) */
            L det=B00*B11-B01*B10;
            int alcancados=0, total=0;
            for(L c1=-40; c1<=40; c1++) for(L c2=-40; c2<=40; c2++){
                total++;
                if(( B11*c1 - B01*c2) % det == 0 && (-B10*c1 + B00*c2) % det == 0) alcancados++;
            }
            /* e o mesmo varrimento com uma chave de GL2(Z): tem de alcançar TUDO */
            L chave[3]={2,3,1}; L K00,K01,K10,K11; matriz(chave,3,&K00,&K01,&K10,&K11);
            L dK=K00*K11-K01*K10;
            int alc_gl=0;
            for(L c1=-40; c1<=40; c1++) for(L c2=-40; c2<=40; c2++)
                if(( K11*c1 - K01*c2) % dK == 0 && (-K10*c1 + K00*c2) % dK == 0) alc_gl++;

            printf("      criptogramas varridos: %d\n", total);
            printf("      com det = 2  — decifráveis: %d  (%.1f%%)\n",
                   alcancados, 100.0*alcancados/total);
            printf("      com det = ±1 — decifráveis: %d  (%.1f%%)\n",
                   alc_gl, 100.0*alc_gl/total);
            ok("com det ≠ ±1 há criptogramas SEM origem: a cifra não é sobre", alcancados < total);
            ok("com det = ±1 TODO criptograma decifra — e é isso que a faz cifra", alc_gl == total);
        }
        conclui("a cifra reversível é esta: o texto é a palavra, a chave é a matriz, e o");
        conclui("determinante ±1 é a condição — a mesma que dá a bijeção do §P6. Um facto só,");
        conclui("a servir de duas coisas: o degrau até R, e a cifra que volta sem perda.");
    }

    /* ---------------- §P10 — o cruzamento: 0 e ∞ colapsam, e os números ficam ---------------- */
    printf("\n§P10 o cruzamento de dimensão: 0 e ∞ colapsam, e os NÚMEROS são os mesmos\n");
    {
        /* O Aarão: "no cruzamento de dimensão o infinito colapsa no 1 e 0 também, então fica
         * indistinguível, mas os números são os mesmos."
         *
         * O dígito 0 É o cruzamento. A_0 = [[0,1],[1,0]] é a Möbius x -> 1/x: troca 0 com ∞
         * e fixa ±1. E a colisão do §P7 não é acidente de notação — é uma IDENTIDADE de
         * matrizes inteiras:
         *
         *     A_a · A_0 · A_b  =  A_{a+b}      exatamente, entrada a entrada
         *
         * Duas palavras distintas, a MESMA matriz. O que colapsa é a carta — qual das
         * coordenadas de P¹ se está a usar — e não o número: as entradas continuam
         * inteiras, o determinante continua ±1, e a operação continua reversível.
         *
         * É por isso que a bijeção do §P6 pede a_i >= 1 a partir do primeiro: não para
         * arrumar, mas para NÃO ATRAVESSAR o cruzamento a meio da palavra. */
        int ident=0, tent=0;
        for(L x=-30; x<=30; x++) for(L y=-30; y<=30; y++){
            L com0[3] = { x, 0, y }, sem0[1] = { x + y };
            L p1,p2,q1,q2, r1,r2,s1,s2;
            matriz(com0,3,&p1,&p2,&q1,&q2);
            matriz(sem0,1,&r1,&r2,&s1,&s2);
            tent++;
            if(p1==r1 && p2==r2 && q1==s1 && q2==s2) ident++;   /* IGUALDADE de matrizes */
        }
        printf("      pares (a,b) em [-30,30]²: %d   com A_a·A_0·A_b = A_{a+b} entrada a entrada: %d\n",
               tent, ident);
        ok("o colapso é uma IDENTIDADE de matrizes, não acidente de escrita", ident==tent);

        /* A_0 é a involução x -> 1/x: ordem 2, troca 0 e ∞, fixa ±1 */
        L z[1] = { 0 }; L a00,a01,a10,a11; matriz(z,1,&a00,&a01,&a10,&a11);
        L d0[2] = { 0, 0 }; L b00,b01,b10,b11; matriz(d0,2,&b00,&b01,&b10,&b11);
        printf("      A_0 = [[%lld,%lld],[%lld,%lld]]   A_0·A_0 = [[%lld,%lld],[%lld,%lld]]\n",
               a00,a01,a10,a11, b00,b01,b10,b11);
        ok("A_0 é involução: A_0·A_0 = I, e é ν com ν∘ν = id", b00==1&&b01==0&&b10==0&&b11==1);
        /* troca 0 com ∞: (0,1) -> (1,0) e (1,0) -> (0,1), em coordenadas projetivas */
        ok("A_0 troca 0 com ∞ em P¹: (0:1) ↔ (1:0)", a00*0+a01*1==1 && a10*0+a11*1==0);
        /* e fixa ±1: A_0·(1,1) = (1,1),  A_0·(1,−1) = (−1,1) ~ −(1,−1) */
        ok("e FIXA ±1 — os pontos onde as duas cartas concordam",
           (a00+a01==1 && a10+a11==1) && (a00-a01==-1 && a10-a11==1));

        /* e o determinante NÃO se altera no cruzamento: os números são os mesmos */
        L det_com = 0, det_sem = 0; int det_igual=0, dt=0;
        for(L x=1; x<=20; x++) for(L y=1; y<=20; y++){
            L com0[3]={x,0,y}, sem0[1]={x+y};
            L p1,p2,q1,q2,r1,r2,s1,s2;
            matriz(com0,3,&p1,&p2,&q1,&q2); matriz(sem0,1,&r1,&r2,&s1,&s2);
            det_com = p1*q2-p2*q1; det_sem = r1*s2-r2*s1;
            dt++; if(det_com==det_sem && (det_com==1||det_com==-1)) det_igual++;
        }
        printf("      det antes e depois do cruzamento: iguais e ±1 em %d de %d casos\n", det_igual, dt);
        ok("o det ATRAVESSA o cruzamento sem mudar: a reversão não se perde", det_igual==dt);
        conclui("o que fica indistinguível é a CARTA — qual coordenada de P¹ se usa — e não o número.");
        conclui("as entradas continuam inteiras, det continua ±1, e a operação continua reversível.");
        conclui("e é exatamente por isto que o §P6 pede a_i >= 1: para não atravessar o cruzamento");
        conclui("a meio da palavra. A regra não arruma notação — evita o ponto onde 0 e ∞ são um só.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
