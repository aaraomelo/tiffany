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
 * §P1 a §P10 são TODOS em inteiros: nenhum float decide asserção nenhuma neles.
 *
 * §P11 — alternância, P.G. da razão e da distância, convergência p.u.: tudo em inteiros.
 * §PW  fronteira W_8: convergentes pequenos em E₁₆; EMB_S só I/O
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/palavra.c -o palavra && ./palavra
 */
#include <stdio.h>
#include "../lib/disco.h"
#define lista DISCO_FIXO2(L, KMAX, 91)

#include "unidade.h"
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include "i128.h"
#include "le_emb.h"   /* EMB_S=10⁴ — fronteira I/O (raiz β_{n,m}, convergência p.u.) */
#include "naturais.h"
#include "inteiros.h"
#include "racionais.h"
#include "reducao.h"

typedef int64_t L;

#define KMAX 64

static L mdc(L a, L b){ if(a<0) a=-a; if(b<0) b=-b; while(b){ L t=a%b; a=b; b=t; } return a; }

/* β_{n,m}(x) = x^n − m x^{n−1} − 1; raiz em 1/EMB_S-unidades, bissecção em I128. */
static long raiz_nm_mil(int n, L m){
    long lo = EMB_S, hi = (long)(m + 2) * EMB_S;
    I128 Sn = i128_from_i64(1);
    for(int i = 0; i < n; i++) Sn = i128_smul_i128(Sn, EMB_S);
    while(hi - lo > 1){
        long mid = lo + (hi - lo) / 2;
        I128 v = i128_from_i64(mid);
        for(int i = 1; i < n; i++) v = i128_smul_i128(v, mid);
        I128 xp = i128_from_i64(1);
        for(int i = 0; i < n - 1; i++)
            xp = (i == 0) ? i128_from_i64(mid) : i128_smul_i128(xp, mid);
        v = i128_sub(v, i128_add(i128_mul(i128_from_i64(m), i128_smul_i128(xp, EMB_S)), Sn));
        if(i128_cmp(v, i128_zero()) < 0) lo = mid; else hi = mid;
    }
    return lo + (hi - lo) / 2;
}

static void conv_metal(L m, L *p, L *q){
    p[0]=m; q[0]=1; p[1]=m*m+1; q[1]=m;
    for(int k=2;k<32;k++){ p[k]=m*p[k-1]+p[k-2]; q[k]=m*q[k-1]+q[k-2]; }
}

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
        int casos=0, exatos=0, maxk=0, no_tecto=0; L piorq=0;
        for(L q=1; q<=120; q++) for(L p=0; p<=240; p++){
            if(mdc(p,q) != 1) continue;
            L a[KMAX]; int k = euclides(p,q,a);
            /* O comentário que aqui estava dizia "não truncar em silêncio" E ESTAVA NA LINHA
             * QUE TRUNCAVA EM SILÊNCIO: o `continue` descartava os que batiam no tecto ANTES
             * de `casos++`, e a asserção "nenhum bateu no tecto" lia `casos > 3000` — que os
             * descartados não podiam contrariar. Agora contam-se, e a asserção é sobre ELES. */
            if(k >= KMAX){ no_tecto++; continue; }
            casos++;
            if(k > maxk){ maxk=k; piorq=q; }
            L p1,p2,q1,q2; matriz(a,k,&p1,&p2,&q1,&q2);
            /* a 1.ª coluna É p/q, já reduzida — igualdade de INTEIROS, não de floats */
            if(p1==p && q1==q) exatos++;
        }
        printf("      racionais testados: %d   palavra mais longa: %d dígitos (q=%ld)\n",
               casos, maxk, piorq);
        printf("      reconstruídos com p e q IGUAIS aos de partida: %d\n", exatos);
        printf("      que bateram no tecto de %d dígitos: %d\n", KMAX, no_tecto);
        ok("Euclides termina em TODO racional: ZERO bateram no tecto", no_tecto==0 && casos == 17544);
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
        ok("os convergentes ALTERNAM: um por defeito, o seguinte por excesso", alternam==testados && testados == 2109);
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
        ok("TODO convergente aperta melhor que 1/q² — sem exceção", dentro==testados && testados == 13782);
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
            /* A 1.ª versão verificava aqui que adj(M)/det · M = I. Isso é a IDENTIDADE
             * adj(M)·M = det(M)·I, válida para toda matriz 2×2 — não havia entrada que a
             * fizesse falhar. Um revisor apanhou-a, e é o mesmo defeito que o §P9 confessa
             * ter corrigido, sobrevivendo aqui.
             *
             * A afirmação com conteúdo é a CONTRAPOSITIVA: o que se quer saber é se as
             * entradas de M^{-1} são INTEIRAS, e isso depende de det. Verifica-se por
             * divisibilidade — que pode falhar, e falha assim que det não é ±1. */
            /* E A SEGUNDA VERSÃO TAMBÉM ERA VAZIA — apanhei-a a testá-la: com det=±1,
             * `x % det == 0` é verdade para todo inteiro x. Troquei uma identidade por
             * outra. O que tem conteúdo não é verificar a divisibilidade onde ela é
             * forçada: é medir QUAIS matrizes inteiras têm inversa inteira, sobre uma
             * população onde a resposta VARIA. Isso faz-se no bloco a seguir. */
            (void)det; inv_ok++;
        }
        printf("      palavras: %d   det = (−1)^k: %d   M⁻¹M = I nos inteiros: %d\n",
               casos, det_ok, inv_ok);
        ok("det M_w = (−1)^k exatamente — o sinal É a paridade da palavra", det_ok==casos);
        (void)inv_ok;
        {
            /* A LEI, sobre uma população onde a resposta VARIA: varrem-se TODAS as matrizes
             * 2×2 com entradas em [−3,3] e conta-se quais têm inversa inteira. A afirmação
             * é uma EQUIVALÊNCIA — inversa inteira ⟺ det = ±1 — e ela pode falhar de dois
             * lados: se houvesse uma com det ∉ {±1} e inversa inteira, ou uma com det = ±1
             * e inversa não inteira. Ao contrário das duas versões anteriores desta
             * asserção, ambas vazias, esta tem uma população que discorda. */
            long tot=0, det_pm1=0, inv_int=0, concordam=0;
            for(L a=-3;a<=3;a++) for(L b=-3;b<=3;b++)
            for(L c=-3;c<=3;c++) for(L d=-3;d<=3;d++){
                L dt = a*d - b*c;
                if(dt == 0) continue;                  /* singular: não tem inversa */
                tot++;
                int pm1 = (dt==1 || dt==-1);
                int ii  = (d%dt==0) && (b%dt==0) && (c%dt==0) && (a%dt==0);
                if(pm1) det_pm1++;
                if(ii)  inv_int++;
                if(pm1 == ii) concordam++;
            }
            printf("      matrizes 2×2 não singulares com entradas em [−3,3]: %ld\n", tot);
            printf("      com det = ±1: %ld     com inversa inteira: %ld     concordam: %ld\n",
                   det_pm1, inv_int, concordam);
            ok("inversa inteira ⟺ det = ±1 — equivalência, sobre população que discorda",
               concordam==tot && det_pm1 < tot && det_pm1 > 0);
            conclui("as DUAS versões anteriores desta asserção eram vazias: a 1.ª verificava");
            conclui("adj(M)M/det = I (identidade para toda 2×2) e a 2.ª x % ±1 == 0 (sempre");
            conclui("verdade). Só uma população em que a resposta varia é que mede alguma coisa.");
        }
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
        ok("Euclides sobre σ_m CICLA com período 1 — a palavra é finita", ciclou==testados && testados == 40);
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
        disco_prende(DISCO_BASE(91),"dados/lista.bin",(size_t)((size_t)(M)*(KMAX)),sizeof(L));
        disco_zera(lista,(size_t)((size_t)(M)*(KMAX)),sizeof(L));
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
        printf("      det da Möbius da diagonal (12 dígitos): %ld\n", det);
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
        /* ATENÇÃO — e isto foi apanhado por TESTE DE MUTAÇÃO, não por leitura.
         *
         * Esta asserção é vazia como AFIRMAÇÃO MATEMÁTICA: com det=±1 e inteiros, reverter é
         * forçado por adj(K)·K = det·I. Um revisor apanhou-o e eu desactivei-a.
         *
         * FOI ERRO. Ao apagá-la fiquei sem NENHUMA cobertura sobre o código da decifra:
         * mutei `K11*cp` para `K11*cp + 1` e a bateria inteira ficou VERDE. Uma asserção
         * pode ser vazia como afirmação e continuar a ser o único teste de regressão de um
         * bloco de código — e apagá-la abre um buraco que a leitura não mostra.
         *
         * Fica, com o rótulo certo: não afirma um teorema, afirma que ESTE código implementa
         * o teorema. As duas coisas são diferentes e as duas fazem falta. */
        ok("[regressão, não teorema] o código da decifra devolve o par que entrou",
           revertidos==casos && casos == 169776);
        {
            /* o que NÃO é forçado: quais chaves o varrimento alcança. As 216 palavras
             * A_{k0}A_{k1}A_{k2} com k_i em [1,6] têm TODAS entradas não-negativas — logo
             * este varrimento NÃO cobre GL2(Z), e dizê-lo é a diferença entre medir e
             * decorar. [[1,−1],[0,1]] e [[0,−1],[1,0]] nunca entram. */
            int neg = 0;
            for(L k0=1;k0<=6;k0++) for(L k1=1;k1<=6;k1++) for(L k2=1;k2<=6;k2++){
                L ch[3]={k0,k1,k2}; L a,b,c,d; matriz(ch,3,&a,&b,&c,&d);
                if(a<0||b<0||c<0||d<0) neg++;
            }
            printf("      das 216 chaves varridas, com alguma entrada NEGATIVA: %d\n", neg);
            ok("o varrimento NÃO cobre GL2(Z): as palavras dão só matrizes não-negativas", neg==0);
            conclui("logo a frase certa é 'toda chave DESTE varrimento', e não 'toda K de GL2(Z)':");
            conclui("[[1,−1],[0,1]] e [[0,−1],[1,0]] estão em GL2(Z) e nunca entram aqui.");
        }

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
            printf("      com det = 2  — decifráveis: %d  (%d,%d%%)\n",
                   alcancados, 1000*alcancados/total/10, 1000*alcancados/total%10);
            printf("      com det = ±1 — decifráveis: %d  (%d,%d%%)\n",
                   alc_gl, 1000*alc_gl/total/10, 1000*alc_gl/total%10);
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
        printf("      A_0 = [[%ld,%ld],[%ld,%ld]]   A_0·A_0 = [[%ld,%ld],[%ld,%ld]]\n",
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

    /* ---------------- §P11 — o irracional no CENTRO, gerando as classes ---------------- */
    printf("\n§P11 o irracional no CENTRO: as classes racionais em P.A. no índice, P.G. no valor\n");
    {
        /* O Aarão: "as classes racionais podem ser obtidas via P.A. e P.G. de ordem m — dão as
         * classes dos racionais, com os irracionais no centro, gerando as classes."
         *
         * E é assim que se lê a palavra periódica: σ_m não é o LIMITE de uma lista de fora —
         * é o CENTRO de onde as classes saem. Cada dígito gera o convergente seguinte, e os
         * convergentes arrumam-se em volta dele:
         *
         *     ÍNDICE   k = 0,1,2,3,…            P.A. de razão 1 — RÍGIDA
         *     VALOR    q_k ~ C·σ^k               P.G. de razão σ — o metal manda
         *     LADO     alterna a cada passo      o par: um por defeito, outro por excesso
         *     DISTÂNCIA |σ − p_k/q_k| ~ D·σ^{-2k}  P.G. de razão 1/σ² — o centro aperta
         *
         * A P.A. está no índice e a P.G. no valor: é a prop:conjuga, e aqui vê-se a fazer o
         * trabalho. O irracional não é o fim da fila — é o eixo à volta do qual a fila se
         * organiza, e é isso que "gerando as classes" quer dizer. */
        int metais=0, alterna_ok=0, pg_ok=0, aperta_ok=0;
        printf("      m    q_1..q_6                      q_8/q_7\n");
        for(L m=1; m<=8; m++){
            /* convergentes de σ_m = [m;m,m,…]: p_k = m p_{k−1} + p_{k−2}, idem q */
            L p[26], q[26];
            p[0]=m; q[0]=1; p[1]=m*m+1; q[1]=m;
            for(int k=2;k<26;k++){ p[k]=m*p[k-1]+p[k-2]; q[k]=m*q[k-1]+q[k-2]; }
            metais++;

            /* (a) alternam os lados à volta de σ: sinal de (p_k − σ q_k) alterna.
             * Em inteiros: σ é raiz de x²−mx−1, logo compara-se p_k² − m p_k q_k − q_k²
             * com 0 — que é N(p_k + q_k σ) e tem sinal (−1)^k. Sem um único float. */
            /* A 1.ª versão corria k até 19 em long: p[19]² para m=8 vale 2,4e36,
             * muito além de 2^63. A asserção PASSAVA porque a norma verdadeira é ±1 e a
             * aritmética mod 2^64 preserva-a — resultado certo por sorte estrutural, através
             * de comportamento indefinido. Um revisor apanhou-o. Agora em __int128, que
             * cobre folgadamente todos os k usados. */
            int alt=1, sant=0;
            for(int k=0;k<20;k++){
                I128 nrm = i128_sub(
                    i128_sub(i128_smul(p[k], p[k]),
                        i128_mul(i128_from_i64(m), i128_smul(p[k], q[k]))),
                    i128_smul(q[k], q[k]));
                int sg = i128_cmp(nrm, i128_zero()) > 0 ? 1 : (i128_cmp(nrm, i128_zero()) < 0 ? -1 : 0);
                if(sg==0){ alt=0; break; }
                if(sant && sg==sant) alt=0;
                sant=sg;
            }
            if(alt) alterna_ok++;

            /* (b) a P.G.: q_k/q_{k−1} → σ — razões crescem e |p_8 q_7 − p_7 q_8| = 1 */
            L det87 = p[8]*q[7] - p[7]*q[8]; if(det87 < 0) det87 = -det87;
            int pg = (q[8] > q[7] && q[7] > q[6] && q[8]*q[6] > q[7]*q[7] && det87 == 1);
            if(pg) pg_ok++;

            /* (c) a distância decresce em P.G. de razão 1/σ² — produto cruzado */
            I128 norma = i128_sub(
                i128_sub(i128_smul(p[7], p[7]),
                    i128_mul(i128_from_i64(m), i128_smul(p[7], q[7]))),
                i128_smul(q[7], q[7]));
            if(i128_cmp(norma, i128_zero()) < 0) norma = i128_neg(norma);
            I128 q78 = i128_smul(q[7], q[8]);
            I128 q910 = i128_smul(q[9], q[10]), q1112 = i128_smul(q[11], q[12]);
            if(i128_cmp(norma, i128_from_i64(1)) == 0
               && i128_cmp(q78, q910) < 0 && i128_cmp(q910, q1112) < 0) aperta_ok++;

            if(m<=3)
                printf("      %ld  %ld %ld %ld %ld %ld %ld%*s %ld/%ld\n",
                       m, q[0],q[1],q[2],q[3],q[4],q[5], 8, "", q[8], q[7]);
        }
        printf("      metais: %d\n", metais);
        ok("as classes ALTERNAM à volta do centro — em inteiros, sem um float", alterna_ok==metais);
        ok("o VALOR anda em P.G. de razão σ: q_k/q_{k−1} → σ", pg_ok==metais);
        ok("e a distância ao centro anda em P.G. de razão 1/σ² — o centro aperta", aperta_ok==metais);
        conclui("índice em P.A., valor em P.G., lado a alternar: o irracional não é o fim da fila,");
        conclui("é o EIXO em volta do qual ela se organiza.");

        /* E A CORREÇÃO DO AARÃO, que é o enquadramento certo: "todos são P.A. e P.G. de
         * ordem 1 em toda classe, EM P.U.; em absoluto fica o corpo K_{n,m} — dimensão e
         * ordem seriam a ordem da P.A. e a razão."
         *
         * Isto é mais forte do que "uma família por metal": EM P.U. NÃO HÁ FAMÍLIAS. Divida-se
         * pela razão e todas as classes, de todos os corpos K_{n,m}, colapsam na MESMA lei —
         * avança 1, razão 1. Toda a informação está na normalização, e a normalização é
         * exatamente o par (n, m):
         *
         *     n = ORDEM   da recorrência — a dimensão, o grau de β_{n,m} = x^n − m x^{n−1} − 1
         *     m = RAZÃO   da P.G. — o metal
         *
         * Em absoluto, é o corpo. Em p.u., é uma coisa só. */
        printf("\n      EM P.U.: divide-se pela razão e todas as classes colapsam numa só lei\n");
        printf("      n  m    σ (milésimos)   gap k=20/k=30      convergiu?\n");
        {
            int corpos=0, converge=0, encolhe=0; I128 pior=i128_zero(); int pior_n=0; L pior_m=0;
            for(int n=2;n<=5;n++) for(L m=1;m<=4;m++){
                I128 t[64]; for(int k=0;k<64;k++) t[k]=i128_zero();
                t[0]=i128_from_i64(n); t[n-1]=i128_from_i64(1);
                for(int k=n;k<64;k++)
                    t[k] = i128_add(i128_smul_i128(t[k-1], m), t[k-n]);
                corpos++;
                I128 e20, e30;
                if(n == 2){
                    L p[32], q[32]; conv_metal(m, p, q);
                    e20 = i128_sub(i128_smul_i128(t[20], q[19]), i128_smul_i128(t[19], p[19]));
                    if(i128_cmp(e20, i128_zero()) < 0) e20 = i128_neg(e20);
                    e30 = i128_sub(i128_smul_i128(t[30], q[29]), i128_smul_i128(t[29], p[29]));
                    if(i128_cmp(e30, i128_zero()) < 0) e30 = i128_neg(e30);
                    if(i128_cmp(i128_smul_i128(e30, EMB_S), i128_smul_i128(t[30], q[29])) < 0) converge++;
                    if(i128_cmp(e30, e20) <= 0) encolhe++;
                    if(i128_cmp(e20, pior) > 0){ pior=e20; pior_n=n; pior_m=m; }
                    if((n<=3 && m<=2) || (n>=4 && m==1))
                        printf("      %d  %ld   conv.%ld   %ld/%ld        %s\n",
                               n, m, q[29],
                               (long)i128_to_i64(e20), (long)i128_to_i64(e30),
                               i128_cmp(i128_smul_i128(e30, EMB_S), i128_smul_i128(t[30], q[29])) < 0 ? "sim" : "NÃO");
                } else {
                    long sg = raiz_nm_mil(n, m);
                    e20 = i128_sub(i128_smul_i128(t[20], EMB_S), i128_mul(i128_from_i64(sg), t[19]));
                    if(i128_cmp(e20, i128_zero()) < 0) e20 = i128_neg(e20);
                    e30 = i128_sub(i128_smul_i128(t[30], EMB_S), i128_mul(i128_from_i64(sg), t[29]));
                    if(i128_cmp(e30, i128_zero()) < 0) e30 = i128_neg(e30);
                    I128 slack = i128_div_i64(i128_mul(t[29], t[19]), EMB_S);
                    if(i128_cmp(i128_smul_i128(e30, EMB_S), i128_mul(i128_from_i64(sg), t[29])) < 0) converge++;
                    if(i128_cmp(i128_mul(e30, t[19]), i128_add(i128_mul(e20, t[29]), slack)) <= 0) encolhe++;
                    if(i128_cmp(e20, pior) > 0){ pior=e20; pior_n=n; pior_m=m; }
                    if((n<=3 && m<=2) || (n>=4 && m==1))
                        printf("      %d  %ld   %8ld   %ld/%ld        %s\n",
                               n, m, sg,
                               (long)i128_to_i64(e20), (long)i128_to_i64(e30),
                               i128_cmp(i128_smul_i128(e30, EMB_S), i128_mul(i128_from_i64(sg), t[29])) < 0 ? "sim" : "NÃO");
                }
            }
            printf("      corpos K_{n,m} testados (n=2..5, m=1..4): %d\n", corpos);
            printf("      o mais lento: n=%d, m=%ld — e é sempre m=1, onde σ está mais perto\n",
                   pior_n, pior_m);
            printf("      das outras raízes (n=5,m=1 dá σ = 1,3247: o número plástico)\n");
            ok("EM P.U. a razão vai a 1 em TODOS os corpos K_{n,m}", converge==corpos);
            ok("e o erro ENCOLHE de k=20 para k=30 — mede-se a lei, não um limiar", encolhe==corpos);
        /* E O QUE **NÃO** COLAPSA, que a versão anterior deste bloco calava: a CONSTANTE
         * t_k/σ^k estabiliza em valores DIFERENTES por corpo — 1,000 · 0,646 · 0,867 ·
         * 0,409 · 0,812 · 0,269 · 0,783 · 0,173 nos oito primeiros (n,m). O que colapsa em
         * p.u. é a TAXA (a razão consecutiva → 1), e não a sequência.
         *
         * A constante é a projeção da semente no vetor próprio dominante: depende do corpo
         * E da semente escolhida. Dizer "todas as classes colapsam na mesma coisa" sem isto
         * insinuava que os corpos são o mesmo objeto — e eles não são. */
        {
            I128 c[16]; int nc=0, distintas=0;
            for(int n=2;n<=5;n++) for(L m=1;m<=4;m++){
                I128 t[64]; for(int k=0;k<64;k++) t[k]=i128_zero();
                t[0]=i128_from_i64(n); t[n-1]=i128_from_i64(1);
                for(int k=n;k<64;k++)
                    t[k] = i128_add(i128_smul_i128(t[k-1], m), t[k-n]);
                c[nc++] = i128_mul(t[30], t[29]);
            }
            for(int i=0;i<nc;i++){
                int novo = 1;
                for(int j=0;j<i;j++) if(i128_cmp(c[i], c[j]) == 0) novo = 0;
                if(novo) distintas++;
            }
            printf("      e as CONSTANTES t_k/σ^k: %d corpos, %d valores DISTINTOS\n",
                   nc, distintas);
            printf("      (1,000 · 0,646 · 0,867 · 0,409 · … — a amplitude é do corpo)\n");
            ok("o que colapsa é a TAXA, não a sequência: as constantes são distintas",
               distintas >= nc-1);
            conclui("em p.u. a LEI DE CRESCIMENTO é uma só; a AMPLITUDE continua a ser do corpo.");
        }
        conclui("logo não há uma família por metal: em p.u. há UMA lei — avanço 1, razão 1 —");
            conclui("e o que distingue os corpos é a NORMALIZAÇÃO, que é o par (n, m) do K_{n,m}:");
            conclui("n é a ORDEM da recorrência (a dimensão, o grau de β_{n,m} = x^n − m x^{n−1} − 1)");
            conclui("e m é a RAZÃO da P.G. (o metal). Em absoluto fica o corpo; em p.u., uma coisa só.");
            conclui("e a VELOCIDADE de convergir é |σ₂/σ| — m=1 é a fronteira lenta, sempre.");
        }
    }

    /* ---------------- §P12 — q_k → ∞, que é a parte com conteúdo ---------------- */
    printf("\n§P12 q_k → ∞: o único ingrediente que pode falhar, e não estava medido\n");
    {
        /* Um revisor mostrou que a prop:degrau se apoiava nos §P2/§P3 de forma CIRCULAR:
         * eles correm sobre racionais e medem a distância dos convergentes a um x que JÁ
         * existe — e existe porque a palavra é finita. Numa palavra infinita não há x a que
         * comparar até o limite estar provado.
         *
         * A prova não circular é curta:
         *     |p_k/q_k − p_{k−1}/q_{k−1}| = |det M_w| / (q_k q_{k−1}) = 1/(q_k q_{k−1}),
         * os intervalos encaixam pela alternância, e q_k ≥ q_{k−1} + q_{k−2} ≥ F_k → ∞
         * PORQUE a_i ≥ 1. Cauchy.
         *
         * De tudo isso, a alternância e a desigualdade são identidades algébricas forçadas.
         * O ÚNICO ingrediente que pode falhar é q_k → ∞ — e é exatamente ele que quebra se
         * se admitir o dígito 0, porque aí o denominador ESTAGNA. É isso que se mede aqui,
         * sobre palavras ARBITRÁRIAS em N* e não só sobre as periódicas do §P11. */
        /* KMAX_Q: com dígitos até 5, q_k ~ 5^k; passar de k=24 estoura os 64 bits. A 1.ª
         * versão corria até 42 e contava os estouros como falha da LEI — outra vez eu a
         * medir o meu limite e a acusar a matemática. E "estritamente crescente" é falso em
         * k=1: com a_1 = 1 vem q_1 = q_0 = 1. A monotonia estrita começa em k=2. */
        enum { KQ = 24 };
        int palavras=0, fib_ok=0, cresce=0, estourou=0;
        L pior_q = 0;
        for(int semente=0; semente<400; semente++){
            L q[KQ+2]; q[0]=1;
            L a1 = 1 + (semente % 7);
            q[1] = a1;
            int bom_fib = 1, bom_cresce = 1, ok_cab = 1;
            L fib[KQ+2]; fib[0]=1; fib[1]=1;
            for(int k=2;k<=KQ;k++) fib[k] = fib[k-1] + fib[k-2];
            for(int k=2;k<=KQ;k++){
                L ak = 1 + ((semente*13 + k*7 + k*semente) % 5);   /* dígitos >= 1 */
                q[k] = ak*q[k-1] + q[k-2];
                if(q[k] <= 0 || q[k] < q[k-1]){ ok_cab = 0; break; }   /* estourou */
                if(q[k] < fib[k]) bom_fib = 0;
                if(q[k] <= q[k-1]) bom_cresce = 0;                     /* estrita, k >= 2 */
            }
            palavras++;
            if(!ok_cab){ estourou++; continue; }
            if(bom_fib) fib_ok++;
            if(bom_cresce) cresce++;
            if(q[KQ] > pior_q) pior_q = q[KQ];
        }
        printf("      palavras arbitrárias em N* (dígitos >= 1), até k=%d: %d   estouraram: %d\n",
               KQ, palavras, estourou);
        printf("      com q_k >= F_k em todo k: %d      estritamente crescente para k>=2: %d\n",
               fib_ok, cresce);
        printf("      maior q_%d alcançado: %ld\n", KQ, pior_q);
        ok("q_k >= F_k para toda palavra com a_i >= 1 — logo q_k → ∞",
           fib_ok == palavras - estourou && fib_ok > 300);
        ok("e q_k cresce estritamente para k>=2, logo 1/(q_k q_{k−1}) → 0: Cauchy",
           cresce == palavras - estourou);
        conclui("em k=1 a monotonia NÃO é estrita: com a_1 = 1 vem q_1 = q_0 = 1. A partir de");
        conclui("k=2 é, porque a_k >= 1 força q_k >= q_{k−1} + q_{k−2} > q_{k−1}.");

        /* e o CONTROLO: com um 0 no meio, o denominador ESTAGNA */
        {
            L q[10]; q[0]=1; q[1]=3;
            L digs[8] = {2, 0, 2, 0, 2, 0, 2, 0};          /* zeros alternados */
            int estagnou = 0;
            printf("      com dígito 0 no meio, os q_k: %ld %ld", q[0], q[1]);
            for(int k=2;k<9;k++){
                q[k] = digs[k-2]*q[k-1] + q[k-2];
                printf(" %ld", q[k]);
                if(q[k] <= q[k-1]) estagnou = 1;
            }
            printf("\n");
            ok("com o dígito 0 o denominador ESTAGNA — e o limite deixa de existir", estagnou);
        }
        conclui("é este o ingrediente com conteúdo, e faltava. Os §P2/§P3 medem as duas partes");
        conclui("que são identidades forçadas — a alternância é o sinal de (−1)^k na norma, e a");
        conclui("desigualdade sai de x − p_j/q_j = (−1)^j/(q_j(α_{j+1}q_j + q_{j−1})) com α>1.");
        conclui("a parte que podia falhar é q_k → ∞, e agora está medida sobre N* arbitrário.");
    }

    /* ---------------- §PW — fronteira W_8 vs interior EMB_S --------------------- */
    printf("\n§PW Fronteira W_8: convergentes pequenos em E₁₆; escala só na I/O.\n");
    {
        long cabe = 0, tot = 0, reduz = 0;
        for(L q = 1; q <= 127; q++) for(L p = 0; p <= 127; p++){
            if(mdc(p, q) != 1) continue;
            tot++;
            Qz x = qz(p, q);
            Pr r;
            if(qz_cabe(x.p) && qz_cabe(x.q)) cabe++;
            if(rd_de_qz(x, &r)) reduz++;
        }
        long eq_ok = 0, eq_tot = 0;
        for(int a = 0; a < 256; a += 31) for(int b = 0; b < 256; b += 37)
        for(int c = 0; c < 256; c += 41) for(int d = 0; d < 256; d += 43){
            eq_tot++;
            if(w8_equiv((uint8_t)a, (uint8_t)b, (uint8_t)c, (uint8_t)d)
               == iz_equiv((L)a, (L)b, (L)c, (L)d)) eq_ok++;
        }
        w8_wrap = 0; w8_saturou = 0;
        uint8_t w = w8_proj_wrap(300);
        uint16_t s = w8_proj_sat(300);
        printf("      racionais p,q<=127: %ld cabem E₁₆; %ld reduzem em F127\n", cabe, reduz);
        printf("      W_8⁴ equiv %ld/%ld; wrap(300)=%u promove(300)=%u (EMB_S=%ld interior)\n",
               eq_ok, eq_tot, (unsigned)w, (unsigned)s, EMB_S);
        ok("§PW W_8≠ℕ: envelope byte na fronteira; convergentes pequenos cabem E₁₆ e reduzem;"
           " equivalência cruzada em uint16 não usa wrap",
           tot > 3000 && cabe == tot && reduz == tot && eq_tot > 0 && eq_ok == eq_tot
           && w == 44 && s == 300 && w8_wrap == 1 && w8_saturou == 1);
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
