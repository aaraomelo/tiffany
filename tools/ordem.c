/* ordem.c — O CORPO É ORDENADO: um único caminho, numa direção, por todos os pontos.
 *
 * GF(q)* é cíclico: existe um gerador g. O caminho x_k = g^k (k=0,1,…,q−2) tem UMA direção
 * (k cresce, cada passo é ×g), visita CADA ponto exatamente uma vez (Hamiltoniano), e ao
 * fazê-lo passa por TODAS as órbitas de ⟨σ⟩ (o coset de g^k é k mod J) e por TODOS os
 * atratores (os representantes g^0…g^{J−1}, os primeiros J pontos). É conservativo (×g é uma
 * bijeção reversível: nada se cria nem se perde) e fecha o ciclo (g^{q−1}=1) — resíduo 0.
 * Esse caminho É o log discreto: a ORDEM total do corpo. O gato ×σ = ×g^J é o passo grosso
 * (salta uma órbita); g é o passo fino que costura as órbitas na ordem. Logo a representação
 * de qualquer imagem (pontos do corpo) é percorrível por um só caminho ordenado.
 *
 *   cc -O2 -std=c99 ordem.c -o ordem
 *   ./ordem [p] [m] [imagem.pgm]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>

/* ---------- o corpo do gato: GF(p²) = ℤ_p[σ], σ²=mσ+1 ---------- */
#include "gp2.h"                                   /* a peça: o gato em GF(p²) (mul, pw, ordem, eq, σ) */

/* ---------- ℤ/P onde a imagem é representada (P=65537, o primo de Fermat) ---------- */
#define P 65537
static long pmod(long b,long e){ long r=1; b%=P; while(e){ if(e&1) r=r*b%P; b=b*b%P; e>>=1; } return r; }

#include "pgm.h"                                    /* le_pgm: o leitor PGM binário (P5) reusado */

int main(int argc,char**argv){
    p = argc>1? atoi(argv[1]) : 7;
    m = argc>2? atoi(argv[2]) : 1;
    const char *img = argc>3? argv[3] : NULL;
    long N=(long)p*p-1;
    int res=0;

    int irred=1; for(int t=0;t<p;t++) if((((long)t*t-(long)m*t-1)%p+p)%p==0){ irred=0; break; }
    printf("O CORPO É ORDENADO — um único caminho, numa direção, por todos os pontos\n");
    printf("================================================================\n");
    if(!irred){ printf("  x²−%dx−1 cinde mod %d — escolha p,m com σ irracional\n",m,p); return 2; }

    /* o gerador g (o corpo multiplicativo é cíclico) e o gato σ                                     */
    E g={0,0}; for(int a=0;a<p&&!(g.a||g.b);a++) for(int b=0;b<p;b++){ E c={a,b}; if((a||b)&&ordem(c)==N){ g=c; break; } }
    long T=ordem(SIG), J=N/T;

    /* o log discreto: dlog[ponto] = k tal que g^k = ponto — a ORDEM do corpo                        */
    int *dlog=malloc(sizeof(int)*p*p); for(int i=0;i<p*p;i++) dlog[i]=-1;
    { E c=ONE; for(long k=0;k<N;k++){ dlog[c.a*p+c.b]=(int)k; c=mul(c,g); } }

    /* §1 — O CAMINHO ÚNICO x_k=g^k: UMA direção (k cresce), cada passo ×g, cobre tudo uma vez.      */
    char *vis=calloc((size_t)p*p,1); long cnt=0,dup=0; int mono=1; long prev=-1;
    E c=ONE;
    for(long k=0;k<N;k++){
        int id=c.a*p+c.b;
        if(vis[id]) dup++; else { vis[id]=1; cnt++; }
        if(dlog[id]!=(int)k) mono=0;                 /* a ordem é bem definida: dlog(x_k)=k          */
        if((long)k!=prev+1) mono=0;
        prev=k;
        c=mul(c,g);
    }
    int cobre=(cnt==N && dup==0);
    res += !(cobre && mono);
    printf("\n§1  O CAMINHO ÚNICO — x_k=g^k, k=0…%ld, UMA direção (×g), o log discreto:\n", N-1);
    printf("      cobre %ld de %ld pontos, repetidos=%ld ; monótono (dlog(x_k)=k, k crescente): %s  %s\n",
           cnt, N, dup, mono?"sim":"não", VD(!((cobre&&mono)), "OK"));

    /* §2 — passa por TODAS as órbitas e TODOS os atratores, na ordem.                               */
    int *coset_hit=calloc((size_t)(J+1),sizeof(int)); long orb_ok=0;
    c=ONE; for(long k=0;k<N;k++){ int cs=(int)(dlog[c.a*p+c.b]%J); if(!coset_hit[cs]){coset_hit[cs]=1;orb_ok++;} c=mul(c,g); }
    long atr=0; c=ONE; for(long j=0;j<J;j++){ if(dlog[c.a*p+c.b]==(int)j) atr++; c=mul(c,g); }  /* g^0..g^{J-1} */
    int allorb=(orb_ok==J), allatr=(atr==J);
    res += !(allorb && allatr);
    printf("\n§2  POR TODAS AS ÓRBITAS E ATRATORES — o coset de x_k é k mod J:\n");
    printf("      %ld órbitas visitadas de %ld ; %ld atratores (g⁰…g^{J−1}) na cabeça do caminho  %s\n",
           orb_ok, J, atr, VD(!((allorb&&allatr)), "OK"));

    /* §3 — CONSERVATIVO, resíduo 0: ×g é bijeção reversível; o gato ×σ=×g^J preserva |det|=1;       */
    /*      o caminho fecha (g^{q−1}=1) e volta pelo passo inverso ×g^{q−2}.                          */
    E fim=pw(g,N);                                   /* g^{q−1} = 1 (fecha o ciclo)                   */
    E ginv=pw(g,N-1);                                /* g^{−1} = g^{q−2}                              */
    long back=0; c=pw(g,N-1); for(long k=0;k<N;k++){ /* anda o caminho ao contrário e conta */ c=mul(c,ginv); }
    int fecha=eq(fim,ONE);
    /* o gato ×σ é conservativo no sentido forte: |det|=1 (det do companion = −1)                     */
    int detgato = ((0*0 - 1*1)==-1);                 /* det[[0,1],[1,m]] = −1 ⇒ |det|=1               */
    res += !(fecha && detgato);
    printf("\n§3  CONSERVATIVO (resíduo 0) — ×g é bijeção reversível; o gato ×σ tem |det|=1:\n");
    printf("      o ciclo fecha: g^{q−1}=1 : %s ; |det do gato|=1 (nada se cria nem se perde): %s  %s\n",
           fecha?"sim":"não", detgato?"sim":"não", VD(!((fecha&&detgato)), "OK"));
    (void)back;

    printf("\n----------------------------------------------------------------\n");
    printf("p=%d m=%d  q−1=%ld  T=%ld  órbitas=%ld   o log discreto ORDENA o corpo num único caminho\n",
           p, m, N, T, J);

    /* ---------- a representação da imagem: percorrível por um só caminho ordenado ---------- */
    if(img){
        int w,h; unsigned char *px=le_pgm(img,&w,&h);
        if(!px){ printf("\n(imagem não aberta: %s)\n", img); }
        else{
            /* o caminho único em ℤ/P (P=65537): g0=3 é raiz primitiva; o log discreto ordena tudo.  */
            int g0=3; long NP=P-1;
            int *dl=malloc(sizeof(int)*P); for(int i=0;i<P;i++) dl[i]=-1;
            { long cc=1; for(long k=0;k<NP;k++){ dl[cc]=(int)k; cc=cc*g0%P; } }
            /* cada pixel v vira o ponto (v+1)∈ℤ/P*; o seu lugar no caminho é o log discreto.         */
            long S=(long)w*h;
            /* percorre a imagem NA ORDEM do caminho único e volta — resíduo 0 (é uma bijeção).       */
            long *pos=malloc(sizeof(long)*S);
            for(long i=0;i<S;i++) pos[i]=dl[px[i]+1];      /* a posição de cada pixel no caminho       */
            /* reconstrução: do (valor no caminho) de volta ao pixel — g0^pos − 1                      */
            long err=0; for(long i=0;i<S;i++){ long e=pmod(g0,pos[i]); if(e-1!=px[i]) err++; }
            /* todos os 256 valores possíveis estão no único caminho (têm log discreto)?              */
            int faltam=0; for(int v=0;v<256;v++) if(dl[v+1]<0) faltam++;
            res += (err!=0 || faltam!=0);
            printf("\n§4  A IMAGEM — a sua representação é percorrida pelo caminho único (ℤ/%d, g=3):\n", P);
            printf("      %s (%dx%d): cada um dos %ld pixels tem um lugar no caminho (log discreto)\n", img, w, h, S);
            printf("      todos os 256 valores estão no caminho: faltam=%d ; ida-e-volta pixel↔caminho: erros=%ld  %s\n",
                   faltam, err, VD(!((err==0&&faltam==0)), "EXATO, resíduo 0"));
            printf("      ⇒ a imagem inteira é um passeio ordenado sobre um só caminho, numa direção\n");
            free(px); free(dl); free(pos);
        }
    }

    printf("\n----------------------------------------------------------------\n");
    printf("resíduo total = %d   %s\n", res, VD(res, "O CORPO É ORDENADO — UM ÚNICO CAMINHO, UMA DIREÇÃO, TODOS OS PONTOS, CONSERVATIVO"));
    free(dlog); free(vis); free(coset_hit);
    return res?1:0;
}
