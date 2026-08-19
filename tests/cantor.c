/* cantor.c — O GERADOR É CANTOR², E ELE TEM OS DOIS LADOS.
 *
 * O Aarão: "o gerador é Cantor²" · "Cantor dos dois lados" · "forma algébrica e polar" ·
 * "direto e cruzado" · "dual".
 *
 * E tem mesmo dois lados, que são o par de sempre. Ambos são bijeções N² → N, ambos exatos,
 * ambos reversíveis — e nenhum é o outro:
 *
 *     DIRETO / ALGÉBRICO (o que MEDE)      π(a,b) = (a+b)(a+b+1)/2 + b
 *                                          a diagonal. O invariante da fibra é a SOMA a+b,
 *                                          e a fibra é finita: um triângulo por soma.
 *                                          É ADITIVO.
 *
 *     CRUZADO / POLAR (o que ORDENA)       ρ(a,b) = 2^a (2b+1) − 1
 *                                          a fatoração. O invariante da fibra é o EXPOENTE
 *                                          de 2, e a fibra é infinita: uma progressão por
 *                                          expoente. É MULTIPLICATIVO.
 *
 * Um parte N em triângulos, o outro parte-o em progressões. As duas partições são de tamanhos
 * completamente diferentes — e é isso que os torna DUAIS e não variantes: a informação que um
 * põe na coordenada, o outro põe na fibra.
 *
 * E "Cantor²" é literal: o gerador é o par, e iterá-lo dá N^k → N para todo k FINITO. O que ele
 * NÃO dá é N^N — e é aí que a Möbius entra, porque é ela que faz a potência infinita
 * (palavra.c). Os dois geradores repartem o trabalho, e a fronteira entre eles é exatamente a
 * fronteira entre Q e R.
 *
 *   §K1  o direto é bijeção N² → N: sem repetição, sem buraco, e a inversa é exata
 *   §K2  o polar é bijeção N² → N: o mesmo, por outro caminho
 *   §K3  são DUAIS: fibra finita contra fibra infinita — aditivo contra multiplicativo
 *   §K4  Cantor² itera: N^k → N para todo k finito, e a reversão sobrevive à composição
 *   §K5  a involução: trocar (a,b) por (b,a) — o que cada um faz com o espelho
 *   §K6  controlo negativo: iterar Cantor NÃO alcança N^N — é aí que a Möbius entra\n *   §K7  a involução fecha os QUADRANTES: Z² → N, e os quatro têm o mesmo tamanho
 *
 * Tudo em inteiros. Nenhum float decide asserção nenhuma.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/cantor.c -o cantor && ./cantor
 */
#include <stdio.h>
#include "disco.h"

#include "unidade.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef long long L;

static L tri_inv(L n){
    L lo = 0, hi = 1;
    while(hi <= 3037000499LL && hi*(hi+1)/2 <= n){
        lo = hi;
        if(hi > (1LL<<62)/2) break;
        hi *= 2;
    }
    while(lo < hi){
        L mid = lo + (hi - lo + 1)/2;
        if(mid*(mid+1)/2 <= n) lo = mid; else hi = mid - 1;
    }
    return lo;
}

/* ---- DIRETO / ALGÉBRICO: a diagonal ---- */
static L dir_par(L a, L b){ L s = a+b; return s*(s+1)/2 + b; }
static void dir_imp(L n, L *a, L *b){
    L s = tri_inv(n);
    *b = n - s*(s+1)/2;
    *a = s - *b;
}

/* ---- CRUZADO / POLAR: a fatoração ---- */
static L pol_par(L a, L b){
    L p = 1; for(L i=0;i<a;i++) p *= 2;
    return p*(2*b+1) - 1;
}
static void pol_imp(L n, L *a, L *b){
    L m = n + 1, e = 0;
    while(m % 2 == 0){ m /= 2; e++; }
    *a = e; *b = (m-1)/2;
}

int main(void){
    printf("================================================================\n");
    printf("  Cantor², dos dois lados: o direto mede, o cruzado ordena\n");
    printf("================================================================\n");

    enum { N = 4000 };
    char *visto = DISCO_FIXO(char, 210);
    disco_prende(DISCO_BASE(210),"dados/visto_210.bin",(size_t)((N+64)),sizeof(char));
    disco_zera(visto,(size_t)((N+64)),sizeof(char));
    /* ---------------- §K1 — o direto ---------------- */
    printf("\n§K1 DIRETO (algébrico): π(a,b) = (a+b)(a+b+1)/2 + b — a diagonal\n");
    {
        memset(visto,0,((size_t)((N+64))*sizeof(char)));
        int pares=0, repetidos=0, voltas=0;
        L maior = 0;
        for(L a=0;a<=90;a++) for(L b=0;b<=90;b++){
            L n = dir_par(a,b);
            if(n >= N) continue;
            pares++;
            if(visto[n]) repetidos++; else visto[n]=1;
            if(n > maior) maior = n;
            L x,y; dir_imp(n,&x,&y);
            if(x==a && y==b) voltas++;
        }
        int buracos=0; for(L n=0;n<=maior;n++) if(!visto[n]) buracos++;
        printf("      pares mapeados: %d   repetições: %d   buracos em [0,%lld]: %d\n",
               pares, repetidos, maior, buracos);
        printf("      inversas exatas (a,b) → n → (a,b): %d\n", voltas);
        ok("o direto é INJETIVO: zero repetições", repetidos==0);
        ok("e SOBREJETIVO: zero buracos até ao maior alcançado", buracos==0);
        ok("e a inversa devolve (a,b) EXATO — resíduo 0", voltas==pares);
        printf("      π(0,0)=%lld  π(1,0)=%lld  π(0,1)=%lld  π(2,3)=%lld\n",
               dir_par(0,0), dir_par(1,0), dir_par(0,1), dir_par(2,3));
    }

    /* ---------------- §K2 — o polar ---------------- */
    printf("\n§K2 CRUZADO (polar): ρ(a,b) = 2^a(2b+1) − 1 — a fatoração\n");
    {
        memset(visto,0,((size_t)((N+64))*sizeof(char)));
        int pares=0, repetidos=0, voltas=0;
        L maior = 0;
        for(L a=0;a<=12;a++) for(L b=0;b<=N;b++){
            L n = pol_par(a,b);
            if(n >= N || n < 0) continue;
            pares++;
            if(visto[n]) repetidos++; else visto[n]=1;
            if(n > maior) maior = n;
            L x,y; pol_imp(n,&x,&y);
            if(x==a && y==b) voltas++;
        }
        int buracos=0; for(L n=0;n<=maior;n++) if(!visto[n]) buracos++;
        printf("      pares mapeados: %d   repetições: %d   buracos em [0,%lld]: %d\n",
               pares, repetidos, maior, buracos);
        printf("      inversas exatas: %d\n", voltas);
        ok("o polar é INJETIVO: zero repetições", repetidos==0);
        ok("e SOBREJETIVO: zero buracos", buracos==0);
        ok("e a inversa devolve (a,b) EXATO", voltas==pares);
        printf("      ρ(0,0)=%lld  ρ(1,0)=%lld  ρ(0,1)=%lld  ρ(2,3)=%lld\n",
               pol_par(0,0), pol_par(1,0), pol_par(0,1), pol_par(2,3));
    }

    /* ---------------- §K3 — são DUAIS ---------------- */
    printf("\n§K3 são DUAIS: fibra finita contra fibra infinita, aditivo contra multiplicativo\n");
    {
        /* fibra do direto: {(a,b) : a+b = s} tem s+1 elementos — FINITA e cresce.
         * fibra do polar:  {(a,b) : a = e}   é infinita — uma P.A. de razão 2^{e+1}. */
        printf("      s   |fibra direta a+b=s|   e   |fibra polar a=e| em [0,%d)   passo\n", N);
        int dir_ok=1, pol_ok=1;
        for(L s=0;s<=5;s++){
            int c=0; for(L a=0;a<=s;a++){ L b=s-a; if(dir_par(a,b) < N) c++; }
            if(c != s+1) dir_ok=0;
            int d=0; L primeiro=-1, segundo=-1;
            for(L b=0;b<N;b++){ L n=pol_par(s,b); if(n<N && n>=0){ d++;
                if(primeiro<0) primeiro=n; else if(segundo<0) segundo=n; } }
            L passo = (segundo>=0) ? segundo-primeiro : -1;
            L esperado = 1; for(L i=0;i<=s;i++) esperado *= 2;
            if(passo != esperado) pol_ok=0;
            printf("      %lld          %d              %lld         %6d            %lld  (=2^%lld)\n",
                   s, c, s, d, passo, s+1);
        }
        ok("a fibra do DIRETO tem s+1 elementos — finita, e cresce com s", dir_ok);
        ok("a do POLAR é uma P.A. de passo 2^{e+1} — infinita, e rarefaz", pol_ok);

        /* e os dois pares NÃO coincidem: são bijeções diferentes */
        int diferem=0, tot=0;
        for(L a=0;a<=40;a++) for(L b=0;b<=40;b++){
            L u=dir_par(a,b), v=pol_par(a,b);
            if(v>=0 && v<1000000){ tot++; if(u!=v) diferem++; }
        }
        printf("      pares em que os dois dão valores DIFERENTES: %d de %d\n", diferem, tot);
        ok("não são a mesma bijeção com outra roupa: diferem em quase todo lado",
           diferem > tot*9/10);
        conclui("um parte N em TRIÂNGULOS (soma constante), o outro em PROGRESSÕES (expoente");
        conclui("constante). A informação que um põe na coordenada, o outro põe na fibra —");
        conclui("é o que os torna duais, e não variantes um do outro.");
    }

    /* ---------------- §K4 — Cantor² itera ---------------- */
    printf("\n§K4 Cantor² ITERA: N^k → N para todo k finito, e a reversão sobrevive\n");
    {
        int ks=0, exatos=0;
        printf("      k   tuplos testados   reversões exatas   descartados (>64 bits)\n");
        for(int k=2;k<=6;k++){
            int tot=0, bons=0, descartados=0;
            for(int rep=0; rep<3000; rep++){
                L v[8];
                for(int i=0;i<k;i++) v[i] = (rep*7 + i*13 + i*rep) % 9;   /* determinístico */
                /* dobrar à esquerda: n = π(…π(π(v0,v1),v2)…, v_{k−1}).
                 * O guard tem de estar no RESULTADO e não na entrada: a 1.ª versão só via
                 * n < 0 no fim, e 1000 dos 2000 tuplos de k=6 estouravam em silêncio a meio
                 * — a asserção acusava a composição de Cantor em vez de me acusar a mim. */
                L n = v[0]; int coube = 1;
                for(int i=1;i<k;i++){
                    L nn = dir_par(n, v[i]);
                    /* O guard por sinal NÃO chega: um overflow de s*(s+1) pode enrolar para
                     * um positivo MAIOR e passar despercebido — foi o que aconteceu em k=6,
                     * e a asserção acusava a composição de Cantor em vez de me acusar a mim.
                     * A verificação robusta é AUTO-VERIFICAR a dobra: se desdobrar não
                     * devolve o par que entrou, então os 64 bits acabaram ali. */
                    L cx, cy; dir_imp(nn, &cx, &cy);
                    if(nn < 0 || cx != n || cy != v[i]){ coube = 0; break; }
                    n = nn;
                }
                if(!coube){ descartados++; continue; }
                /* desdobrar */
                L w[8]; L cur = n;
                for(int i=k-1;i>=1;i--){ L x,y; dir_imp(cur,&x,&y); w[i]=y; cur=x; }
                w[0]=cur;
                tot++;
                int igual=1; for(int i=0;i<k;i++) if(w[i]!=v[i]) igual=0;
                if(igual) bons++;
            }
            ks++; if(bons==tot) exatos++;
            printf("      %d       %6d            %6d          %6d\n", k, tot, bons, descartados);
        }
        ok("a composição de Cantor reverte EXATO em N^k, k = 2..6", exatos==ks);
        conclui("os descartados NÃO são falhas: são tuplos cujo código passa dos 64 bits, e");
        conclui("vão contados para que a linha não pareça cobertura que não é.");
        conclui("é por isto que 'Cantor²' é o gerador: aplicar duas vezes dá N³, k vezes dá");
        conclui("N^{k+1}. A dimensão finita sai toda de UM par.");
    }

    /* ---------------- §K5 — a involução ---------------- */
    printf("\n§K5 o espelho (a,b) ↦ (b,a): o que cada lado faz com a involução\n");
    {
        /* no direto, a diagonal é simétrica: a soma não muda, só o lugar dentro dela.
         * no polar, trocar expoente com ímpar muda TUDO — não há simetria. */
        int dir_mesma_fibra=0, dir_tot=0, pol_mesma_fibra=0, pol_tot=0;
        for(L a=0;a<=40;a++) for(L b=0;b<=40;b++){
            if(a==b) continue;
            L u1=dir_par(a,b), u2=dir_par(b,a);
            L x1,y1,x2,y2; dir_imp(u1,&x1,&y1); dir_imp(u2,&x2,&y2);
            dir_tot++; if(x1+y1 == x2+y2) dir_mesma_fibra++;    /* mesma diagonal */
            L v1=pol_par(a,b), v2=pol_par(b,a);
            if(v1>0 && v2>0 && v1<100000000 && v2<100000000){
                L p1,q1,p2,q2; pol_imp(v1,&p1,&q1); pol_imp(v2,&p2,&q2);
                pol_tot++; if(p1==p2) pol_mesma_fibra++;         /* mesmo expoente */
            }
        }
        printf("      DIRETO: o espelho fica na MESMA diagonal em %d de %d\n", dir_mesma_fibra, dir_tot);
        printf("      POLAR:  o espelho fica no mesmo expoente em %d de %d\n", pol_mesma_fibra, pol_tot);
        /* Esta era a COMUTATIVIDADE DA ADIÇÃO: dir_imp(dir_par(a,b))=(a,b), logo a condição
         * x1+y1 == x2+y2 é a+b == b+a. Um revisor apanhou-a — e eu tinha corrigido a asserção
         * SEGUINTE deste bloco, deixando esta. A magnitude, medida logo abaixo, é que separa
         * os dois lados; esta linha fica como resumo. */
        conclui("no DIRETO o espelho fica na mesma diagonal — mas isso é a+b = b+a, e não mede");
        (void)dir_mesma_fibra; (void)dir_tot;
        /* A 2.ª asserção deste bloco era VAZIA e apanhei-a a testá-la: o `if(a==b) continue`
         * três linhas acima garante a≠b, e o expoente do polar É a primeira coordenada —
         * logo o espelho troca-o SEMPRE. Contar 0 de 508 não mede nada: é o filtro a
         * montante a decidir o resultado. É o padrão (c) da minha própria lista.
         *
         * O que tem conteúdo é a MAGNITUDE do desvio, que é onde os dois diferem de facto:
         * no direto o espelho move o código por |a−b|, que é limitado pela caixa; no polar
         * move-o EXPONENCIALMENTE. Isso pode falhar — basta a lei ser outra. */
        {
            L pior_dir = 0, pior_pol = 0;
            for(L a=0;a<=20;a++) for(L b=0;b<=20;b++){
                if(a==b) continue;
                L d1 = dir_par(a,b) - dir_par(b,a); if(d1<0) d1=-d1;
                if(d1 > pior_dir) pior_dir = d1;
                L v1 = pol_par(a,b), v2 = pol_par(b,a);
                if(v1>0 && v2>0 && v1<(1LL<<50) && v2<(1LL<<50)){
                    L d2 = v1 - v2; if(d2<0) d2=-d2;
                    if(d2 > pior_pol) pior_pol = d2;
                }
            }
            printf("      maior desvio do espelho em [0,20]²:  DIRETO %lld   POLAR %lld\n",
                   pior_dir, pior_pol);
            printf("      (no direto o desvio é |a−b| <= 20; no polar cresce como 2^max)\n");
            ok("o desvio do DIRETO é limitado por |a−b| — o aditivo quase não sente o espelho",
               pior_dir <= 20);
            ok("e o do POLAR é exponencial: mais de 1000x maior na mesma caixa",
               pior_pol > 1000*pior_dir);
        }
        conclui("o aditivo é simétrico sob o espelho e o multiplicativo não é: é a assinatura");
        conclui("do par mede/ordena, e aqui vê-se sem interpretação nenhuma pelo meio.");
    }

    /* ---------------- §K6 — o controlo negativo ---------------- */
    printf("\n§K6 controlo negativo: iterar Cantor NÃO alcança N^N\n");
    {
        /* Cada aplicação sobe UMA dimensão. Para chegar a N^N seria preciso um número
         * infinito de aplicações — e aí o resultado deixa de ser um natural.
         * Mede-se o que acontece de facto: o valor CRESCE sem limite com k, e o tipo do
         * objeto (um natural) não sobrevive ao limite. */
        /* O `ok_cab = (n > ant && n > 0)` MISTURAVA DUAS COISAS — crescer e não estourar —
         * e o `estourou_em` marcava as duas com a mesma palavra. Daí que a asserção
         * `estourou_em > 0 || n > 1e9` passasse por MOTIVOS OPOSTOS: substituindo
         * `n = dir_par(n,1)` por `n = ant`, o valor fica parado em 1, o `n > ant` é falso,
         * `estourou_em` vira 1, e a asserção que afirma «cresce sem limite» PASSA — a
         * imprimir «NÃO — estourou» num valor que não se mexeu. Medido, não suposto.
         *
         * Separam-se: CRESCER é n > ant com ambos positivos; ESTOURAR é n dar a volta
         * (n < ant ou n <= 0) DEPOIS de ter crescido, que é o sinal do overflow em
         * s(s+1)/2. A tese é que ele cresce em TODOS os passos até ao tecto da máquina —
         * e o tecto é da máquina, não do objecto, o que já vai dito na conclusão. */
        printf("      k    π aplicado k vezes a (1,1,1,…)      cresceu?   cabe em long long?\n");
        int estourou_em = 0, cresceu = 0;
        L n = 1;
        for(int k=1;k<=12;k++){
            L ant = n;
            n = dir_par(n, 1);
            int subiu  = (n > ant && ant > 0 && n > 0);
            int deu_volta = (n < ant || n <= 0);
            if(subiu && !estourou_em) cresceu++;
            if(deu_volta && !estourou_em) estourou_em = k;
            printf("      %2d   %22lld           %-10s %s\n", k, n,
                   subiu ? "sim" : "nao", deu_volta ? "NAO — estourou" : "sim");
        }
        printf("\n      cresceu em %d passos seguidos, e estourou no %d\n\n", cresceu, estourou_em);
        ok("o valor cresce sem limite: cada dimensão nova custa, e o custo compõe-se — cresce"
           " ESTRITAMENTE em todos os passos até dar a volta no tipo, e o passo do estouro é"
           " o seguinte ao último que cresceu. A condição anterior media `estourou_em > 0`,"
           " que uma função PARADA também satisfazia",
           cresceu >= 6 && estourou_em == cresceu + 1);
        conclui("Cantor dá N^k para todo k FINITO, e mais nada. N^N — a potência infinita —");
        conclui("não é o limite disto: é outro objeto, e quem o dá é a Möbius do palavra.c,");
        conclui("porque ali a palavra NÃO tem de terminar. Os dois geradores repartem o");
        conclui("trabalho, e a fronteira entre eles é a fronteira entre Q e R.");
    }

    /* ---------------- §K7 — a involução fecha os QUADRANTES ---------------- */
    printf("\n§K7 a involução fecha os quadrantes: Z² → N, e são os QUATRO\n");
    {
        /* O Aarão: "e aplica a involução para fechar os quadrantes."
         *
         * Cantor dá N² → N, e N² é UM quadrante. Os outros três chegam pela involução do
         * sinal — a mesma que faz N ↔ Z (0,1,−1,2,−2,…), aplicada em CADA coordenada:
         *
         *     n2z(n) = (n par) ? n/2 : −(n+1)/2          z2n(z) = (z>=0) ? 2z : −2z−1
         *
         * Compondo:  Z² --(z2n em cada coordenada)--> N² --(Cantor)--> N,  bijeção.
         * E os quatro quadrantes fecham porque a involução tem ponto fixo em 0 e troca os
         * lados: é ν∘ν = id no eixo, e o eixo é a fronteira entre quadrantes. */
        int quad[4] = {0,0,0,0};
        long tot=0, voltas=0, repetidos=0;
        enum { MZ = 60 };
        /* VARIAVEL LOCAL e nao macro: `v2` tambem e' o nome de uma local noutra funcao
         * (L v1=..., v2=...), e um #define apanhava-a. o escopo do bloco resolve, e o
         * ponteiro vive na PILHA — nao conta para .bss. */
        char *v2 = DISCO_FIXO(char, 90);
        disco_prende(DISCO_BASE(90),"dados/v2.bin",(size_t)400000,1);
        memset(v2,0,((size_t)400000));
        for(L x=-MZ; x<=MZ; x++) for(L y=-MZ; y<=MZ; y++){
            L nx = (x>=0) ? 2*x : -2*x-1;                 /* z2n */
            L ny = (y>=0) ? 2*y : -2*y-1;
            L n = dir_par(nx, ny);
            if(n < 0 || n >= (L)((size_t)400000)) continue;
            tot++;
            if(v2[n]) repetidos++; else v2[n]=1;
            /* e a volta: n → (nx,ny) → (x,y) */
            L a2,b2; dir_imp(n,&a2,&b2);
            L rx = (a2%2==0) ? a2/2 : -(a2+1)/2;          /* n2z */
            L ry = (b2%2==0) ? b2/2 : -(b2+1)/2;
            if(rx==x && ry==y) voltas++;
            /* contar por quadrante (o eixo conta como fronteira, e vai à parte) */
            if(x>0 && y>0) quad[0]++;
            else if(x<0 && y>0) quad[1]++;
            else if(x<0 && y<0) quad[2]++;
            else if(x>0 && y<0) quad[3]++;
        }
        printf("      pontos de Z² mapeados: %ld   repetições: %ld\n", tot, repetidos);
        printf("      voltas exatas Z² → N → Z²: %ld\n", voltas);
        printf("      por quadrante: I=%d  II=%d  III=%d  IV=%d\n",
               quad[0],quad[1],quad[2],quad[3]);
        ok("Z² → N é INJETIVO: zero repetições, os quatro quadrantes cabem", repetidos==0);
        ok("e reverte EXATO: a involução do sinal desfaz-se com ν∘ν = id", voltas==tot);
        /* A asserção que aqui estava — "os quatro quadrantes têm o mesmo tamanho" — era
         * VAZIA: varrer [−60,60]² e excluir os eixos dá 60×60 por quadrante SEMPRE. Era a
         * simetria da CAIXA a decidir, não a da involução. Padrão (d) da lista.
         *
         * O que tem conteúdo é que o QUADRANTE FICA ESCRITO no código, e lê-se sem
         * descodificar: z2n manda os >=0 em PARES e os <0 em ÍMPARES, logo os dois bits de
         * baixo de (nx,ny) dizem o quadrante. Isso pode falhar — basta a involução ser
         * outra — e é a afirmação que interessa: a involução não só cabe, como deixa marca. */
        int marca_ok = 0, marca_tot = 0;
        for(L x=-40; x<=40; x++) for(L y=-40; y<=40; y++){
            if(x==0 || y==0) continue;
            L nx = (x>=0) ? 2*x : -2*x-1;
            L ny = (y>=0) ? 2*y : -2*y-1;
            int q_esperado = (x>0) ? ((y>0) ? 0 : 3) : ((y>0) ? 1 : 2);
            int q_lido = ((nx&1) ? ((ny&1) ? 2 : 1) : ((ny&1) ? 3 : 0));
            marca_tot++;
            if(q_lido == q_esperado) marca_ok++;
        }
        printf("      o quadrante lido nos DOIS BITS DE BAIXO de (nx,ny): %d de %d\n",
               marca_ok, marca_tot);
        ok("o quadrante fica ESCRITO no código — lê-se em 2 bits, sem descodificar",
           marca_ok==marca_tot && marca_tot>5000);
        conclui("os 3600 por quadrante são a simetria da CAIXA e não medem nada; o que a");
        conclui("involução faz é deixar marca — e a marca é a paridade.");
        conclui("Cantor sozinho dá UM quadrante; a involução dá os outros três. E não é");
        conclui("acrescentar régua: é a mesma ν do §1, com ponto fixo em 0 — e o ponto fixo");
        conclui("é exatamente o EIXO, que é a fronteira entre os quadrantes.");
    }

    printf("\n================================================================\n");
    printf("  %d unidade(s), %d falha(s)%s\n", unidades, falhas, falhas ? "" : " — RESÍDUO 0");
    return falhas ? 1 : 0;
}
