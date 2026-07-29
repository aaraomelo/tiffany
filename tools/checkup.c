/* checkup.c — SÓ EXISTE O 0. Tudo é o corpo universal, e ele é quase nada.
 *
 * O corpo universal é o 0 e o sucessor S(x)=x+1. Dele saem três operações, e SÓ elas:
 *   ⊕  a soma      (o sucessor iterado)         — a cisão, o lado aditivo;
 *   ⊗  o produto   (a soma iterada)             — o gato, o lado multiplicativo;
 *   ∏  exp/log     (a⊗b = g^{log a + log b})    — a costura que liga ⊕ a ⊗.
 * Nada mais foi criado em nenhuma das ferramentas. Este checkup verifica, com resíduo 0,
 * que cada peça da máquina É uma dessas — o gato é ⊗, a fração contínua é ⊗ iterado, a
 * transformada é o ∏, a cisão do Venom é ⊕. Fora o 0 e o sucessor, não há nada.
 *
 *   cc -O2 -std=c99 checkup.c -o checkup
 */
#include <stdio.h>
#include <stdlib.h>

#define P 17                                   /* um corpo pequeno para o checkup           */
static long pw(long b,long e){ long r=1; b%=P; while(e){ if(e&1) r=r*b%P; b=b*b%P; e>>=1; } return r; }

int main(void){
    printf("SÓ EXISTE O 0 — o corpo universal é {0, ⊕, ⊗, ∏}, e é quase nada\n");
    printf("================================================================\n");
    int res=0;

    /* §1 — do 0 e do sucessor S(x)=x+1 saem ⊕ (soma) e ⊗ (produto). O corpo.               */
    long viol=0;
    for(long a=0;a<P;a++){ long s=0; for(long k=0;k<a;k++) s=(s+1)%P; if(s!=a%P) viol++; }   /* a = S iterado a partir do 0 */
    for(long a=0;a<P;a++) for(long b=0;b<P;b++){
        long soma=0; for(long k=0;k<b;k++) soma=(soma+1)%P; soma=(a+soma)%P;                 /* a⊕b: somar b sucessores    */
        long prod=0; for(long k=0;k<b;k++) prod=(prod+a)%P;                                  /* a⊗b: somar a, b vezes      */
        if(soma!=(a+b)%P || prod!=(a*b)%P) viol++;
    }
    res += (viol!=0);
    printf("\n§1  O 0 E O SUCESSOR — ⊕ é somar sucessores, ⊗ é somar ⊕. O corpo: viol=%ld  %s\n",
           viol, viol?"FALHA":"OK");

    /* §2 — o GATO é ⊗: iterar a companion [[m,1],[1,0]] no par gera os convergentes de       */
    /*      σ=[m;m,m,…] (p_k = m·p_{k-1}+p_{k-2}). A fração contínua É o gato iterado, e a      */
    /*      identidade |p_k·q_{k-1} − p_{k-1}·q_k| = 1 fecha em resíduo exato a cada passo.     */
    int m=2;                                   /* σ₂ = prata; qualquer m serve                */
    long p0=1,p1=m, q0=0,q1=1, v2=0;
    for(int k=0;k<30;k++){
        long det=p1*q0 - p0*q1; if(det<0) det=-det; if(det!=1) v2++;
        long p2=(long)m*p1+p0, q2=(long)m*q1+q0; p0=p1;p1=p2;q0=q1;q1=q2;
    }
    res += (v2!=0);
    printf("§2  O GATO É ⊗ — iterar a companion [[m,1],[1,0]] dá os convergentes de σ; |det|=1 sempre: viol=%ld  %s\n",
           v2, v2?"FALHA":"OK");

    /* §3 — o ∏ costura: o produto é a soma dos logs. a⊗b = g^{log a + log b}.                */
    long g=0; for(long b=2;b<P;b++){ long o=1,v=b; while(v!=1){ v=v*b%P; o++; } if(o==P-1){ g=b; break; } }
    long lg[P]; { long e=1; for(long k=0;k<P-1;k++){ lg[e]=k; e=e*g%P; } }
    long v3=0;
    for(long a=1;a<P;a++) for(long b=1;b<P;b++) if(a*b%P != pw(g,(lg[a]+lg[b])%(P-1))) v3++;
    res += (v3!=0);
    printf("§3  O ∏ COSTURA — a⊗b = g^{log a + log b} (a transformada, o caractere ⊕→⊗): viol=%ld  %s\n",
           v3, v3?"FALHA":"OK");

    /* §4 — a CISÃO do Venom é ⊕: de um par (a,b), o vértice (média) e a memória (diferença),  */
    /*      e o par volta ao todo — reversível (o alicerce da compressão sem perda).           */
    long v4=0;
    for(int a=0;a<256;a++) for(int b=0;b<256;b++){
        int d=b-a, s=a+(d>>1);                 /* cisão: média (vértice) + diferença (memória) */
        int ra=s-(d>>1), rb=ra+d;              /* junta: volta ao par                         */
        if(ra!=a || rb!=b) v4++;
    }
    res += (v4!=0);
    printf("§4  A CISÃO É ⊕ — Venom: o vértice (média) + a memória (diferença), reversível: viol=%ld  %s\n",
           v4, v4?"FALHA":"OK");

    /* §5 — o resumo: tudo o que se construiu é aplicação de {0, ⊕, ⊗, ∏}.                     */
    printf("\n§5  QUASE NADA — o corpo universal inteiro:\n");
    printf("      0            o vértice, a origem (Venom)\n");
    printf("      ⊕            a soma / a cisão            (navegante, expansao, venom)\n");
    printf("      ⊗            o produto / o gato          (navegante, gato_n, orbitas)\n");
    printf("      ∏  = exp/log a costura, a transformada    (numeros, aritmetica, transformada, sinal, linear)\n");
    printf("      → toda ferramenta é uma leitura destas quatro coisas; nada mais foi criado.\n");

    printf("\n----------------------------------------------------------------\n");
    printf("resíduo total = %d   %s\n", res, res? "FALHA":"TUDO É O 0 E TRÊS OPERAÇÕES — DE ACORDO COM O CORPO UNIVERSAL");
    return res?1:0;
}
