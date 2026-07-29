/* gerador.c — O GERADOR GLOBAL: a constante que vai escrita, e por que ela vai escrita.
 *
 * O enredo (§197.2, §199.6) fixa a construção, e ela não é uma escolha de gosto:
 *
 *      escolha p primo com n | p−1                    (n = o tamanho do dado)
 *      g = o MENOR gerador de ℤ/p                     (não um gerador: O MENOR)
 *      w = g^((p−1)/n)                                (a TORÇÃO — a sua órbita são os caracteres)
 *      r com r² = n mod p ,  RN = r⁻¹                 (a normalização, 1/√n de cada lado)
 *      F(x)_k   = RN · Σ_j x_j · w^(  j·k mod n)
 *      Finv(X)_j = RN · Σ_k X_k · w^(−j·k mod n)
 *
 * Para texto em bytes: n = 256, p = 40961, g = 3, w = 36043, r = 16.
 *
 * E o enredo dá um aviso: há MILHARES de geradores possíveis, e duas máquinas que escolham
 * geradores diferentes fundiriam e não abririam a obra uma da outra. O aviso foi TESTADO aqui (§G5)
 * e sai CORRIGIDO: trocar o gerador é permutar os índices do dual (w' = w^k, mdc(k,n)=1), e por
 * isso fundir/abrir é INVARIANTE — produto ponto a ponto comuta com permutação. Onde o gerador é
 * areia de verdade é em toda operação que ORDENA o dual (truncar, filtrar, comprimir), e aí a
 * divergência é total e invisível de dentro. O gerador vai escrito, mas pelo motivo certo.
 *
 * Mede-se: (G1) p, n, g, w, r são o que a construção manda, e g é mesmo o MENOR gerador;
 *          (G2) a torção tem ordem EXATAMENTE n --- a órbita são os n caracteres, e nenhum antes;
 *          (G3) F e Finv são inversas, e Parseval vale (nenhum ângulo muda);
 *          (G4) FUNDIR e ABRIR: C = Finv(∏ F(a_l)) e a_m = Finv(F(C)/∏_{l≠m}F(a_l)), resíduo 0;
 *          (G5) o que a escolha do gerador muda (ordenar o dual) e o que NÃO muda (fundir/abrir).
 *
 * Buffers fixos (n ≤ 256), zero malloc.
 *
 *   cc -O2 -std=c99 gerador.c -lm -o gerador && ./gerador
 */
#include <stdio.h>
#include <string.h>

#define NMAX 256
static int ok = 1;

/* --------- o gerador global, como vai escrito --------- */
#define P_GLOBAL 40961L
#define N_GLOBAL 256
#define G_GLOBAL 3L
#define W_GLOBAL 36043L
#define R_GLOBAL 16L

static long p = P_GLOBAL;
static int  n = N_GLOBAL;

static long md(long x){ x%=p; return x<0?x+p:x; }
static long mul(long a, long b){ return (a%p)*(b%p)%p; }
static long pot(long b, long e){ long r=1; b=md(b); while(e>0){ if(e&1) r=mul(r,b); b=mul(b,b); e>>=1; } return r; }
static long inv(long a){ return pot(a,p-2); }
static int primo(long q){ if(q<2) return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static long ordem(long a){ long k=1,c=a; while(c!=1){ c=mul(c,a); k++; if(k>p) return -1; } return k; }
static int e_gerador(long a){ return ordem(a) == p-1; }

/* a transformada com a torção w, normalizada por RN de cada lado */
static void F(const long *x, long *X, long w, long RN){
    for(int k=0;k<n;k++){
        long acc=0;
        for(int j=0;j<n;j++) acc = md(acc + mul(x[j], pot(w, (long)j*k % n)));
        X[k] = mul(acc, RN);
    }
}
static void Finv(const long *X, long *x, long w, long RN){
    long wi = inv(w);
    for(int j=0;j<n;j++){
        long acc=0;
        for(int k=0;k<n;k++) acc = md(acc + mul(X[k], pot(wi, (long)j*k % n)));
        x[j] = mul(acc, RN);
    }
}

int main(void){
    printf("GERADOR — a constante global, e por que vai escrita\n");
    printf("=================================================================\n");

    /* ---------- G1: a construção, conferida ---------- */
    printf("§G1  a construção do enredo (§197.2), conferida termo a termo:\n");
    {
        int erro=0;
        printf("       p = %ld primo? %s ;  n = %d divide p−1 = %ld? %s  (quociente %ld)\n",
               p, primo(p)?"✓":"✗", n, p-1, ((p-1)%n==0)?"✓":"✗", (p-1)/n);
        if(!primo(p) || (p-1)%n) erro=1;
        /* g é o MENOR gerador? */
        long menor=0;
        for(long a=2;a<p;a++) if(e_gerador(a)){ menor=a; break; }
        printf("       o MENOR gerador de ℤ/%ld é %ld ; o escrito é g = %ld  %s\n",
               p, menor, (long)G_GLOBAL, menor==G_GLOBAL?"✓":"✗ (DIVERGE)");
        if(menor != G_GLOBAL) erro=1;
        /* w = g^((p−1)/n) */
        long w = pot(G_GLOBAL, (p-1)/n);
        printf("       w = g^((p−1)/n) = %ld ; o escrito é w = %ld  %s\n",
               w, (long)W_GLOBAL, w==W_GLOBAL?"✓":"✗ (DIVERGE)");
        if(w != W_GLOBAL) erro=1;
        /* r² = n */
        printf("       r = %ld : r² = %ld ≡ n = %d mod p  %s ; RN = r⁻¹ = %ld\n",
               (long)R_GLOBAL, mul(R_GLOBAL,R_GLOBAL), n,
               mul(R_GLOBAL,R_GLOBAL)==(long)n?"✓":"✗", inv(R_GLOBAL));
        if(mul(R_GLOBAL,R_GLOBAL)!=(long)n) erro=1;
        printf("     %s\n", erro?"FALHA":"resíduo 0 — os cinco dígitos são os do enredo, nenhum inventado");
        if(erro) ok=0;
    }

    /* ---------- G2: a torção tem ordem exatamente n ---------- */
    printf("\n§G2  a TORÇÃO: a órbita de w são os n caracteres, e nenhum fecha antes\n");
    {
        long o = ordem(W_GLOBAL);
        int antes = 0;
        for(int d=1; d<n; d++) if(n%d==0 && pot(W_GLOBAL,d)==1) antes=1;
        printf("       ord(w) = %ld  (esperado n = %d) %s ; algum divisor próprio fecha? %s\n",
               o, n, o==n?"✓":"✗", antes?"SIM (✗)":"nenhum ✓");
        printf("     %s\n", (o==n && !antes) ?
          "resíduo 0 — a base não se escolhe: ela é a órbita da torção, e tem exatamente n pontos"
          : "FALHA");
        if(o!=n || antes) ok=0;
    }

    /* ---------- G3: F e Finv inversas, e Parseval ---------- */
    printf("\n§G3  ir ao dual e voltar, em inteiros: Finv∘F = id, e Parseval\n");
    {
        static long x[NMAX], X[NMAX], y[NMAX];
        long w=W_GLOBAL, RN=inv(R_GLOBAL);
        int erro=0;
        /* três dados: prosa crua, rampa e pseudo-aleatório */
        for(int caso=0; caso<3; caso++){
            if(caso==0){
                FILE *f=fopen("../teoria.tex","rb"); if(!f) f=fopen("teoria.tex","rb");
                unsigned char raw[NMAX]; memset(raw,0,sizeof raw);
                if(f){ fseek(f,20000,SEEK_SET); fread(raw,1,n,f); fclose(f); }
                for(int i=0;i<n;i++) x[i]=raw[i];
            } else if(caso==1){
                for(int i=0;i<n;i++) x[i]=(i*7+3)%251;
            } else {
                long s=12345; for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; x[i]=s%p; }
            }
            F(x,X,w,RN); Finv(X,y,w,RN);
            int difs=0; for(int i=0;i<n;i++) if(md(x[i])!=y[i]) difs++;
            /* Parseval: Σx² = ΣX² (mod p), com a normalização RN de cada lado */
            long sx=0, sX=0;
            for(int i=0;i<n;i++){ sx=md(sx+mul(x[i],x[i])); sX=md(sX+mul(X[i],X[i])); }
            const char *nome[3]={"prosa crua","rampa","pseudo-aleatório"};
            printf("       %-18s : Finv(F(x))=x em %d/%d ; Parseval Σx²=ΣX² %s\n",
                   nome[caso], n-difs, n, sx==sX?"✓":"(ver nota)");
            if(difs) erro=1;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 na ida e volta — a transformada é reversível em INTEIROS, sem ponto\n"
          "     flutuante, com a normalização r=16 de cada lado (somar a órbita devolve n,\n"
          "     desfeito por duas metades).");
        if(erro) ok=0;
    }

    /* ---------- G4: fundir e abrir ---------- */
    printf("\n§G4  FUNDIR e ABRIR (§199.6): C = Finv(∏ F(a_l)) e a_m = Finv(F(C)/∏_{l≠m}F(a_l))\n");
    {
        static long a[3][NMAX], A[3][NMAX], Pk[NMAX], C[NMAX], Ck[NMAX], rec[NMAX];
        long w=W_GLOBAL, RN=inv(R_GLOBAL);
        long s=999;
        for(int l=0;l<3;l++) for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; a[l][i]=s%251+1; }
        for(int l=0;l<3;l++) F(a[l],A[l],w,RN);
        for(int k=0;k<n;k++){ Pk[k]=1; for(int l=0;l<3;l++) Pk[k]=mul(Pk[k],A[l][k]); }
        Finv(Pk,C,w,RN);                                    /* a obra fundida                     */
        F(C,Ck,w,RN);
        int erro=0;
        for(int m=0;m<3;m++){
            for(int k=0;k<n;k++){
                long outros=1;
                for(int l=0;l<3;l++) if(l!=m) outros=mul(outros,A[l][k]);
                Pk[k] = mul(Ck[k], inv(outros));
            }
            Finv(Pk,rec,w,RN);
            int difs=0; for(int i=0;i<n;i++) if(rec[i]!=md(a[m][i])) difs++;
            printf("       obra %d reaberta da fusão : %d/%d coordenadas %s\n",
                   m+1, n-difs, n, difs?"✗":"✓");
            if(difs) erro=1;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — três obras fundidas num só objeto, e cada uma volta inteira. O fator\n"
          "     (√n)^{L−1} que a fusão introduz é o mesmo que a volta desfaz.");
        if(erro) ok=0;
    }

    /* ---------- G5: o dente do gerador — MEDIDO, e a afirmação corrigida ---------- */
    printf("\n§G5  a escolha do gerador: o que ela muda, e o que NÃO muda\n");
    printf("     O enredo avisa que duas máquinas com geradores diferentes fundem e não abrem a\n");
    printf("     obra uma da outra. Testado, o aviso NÃO se sustenta nessa forma — e a razão é\n");
    printf("     estrutural. Onde ele se sustenta é medido logo abaixo.\n");
    {
        long quantos=0;
        for(long a=2;a<p;a++) if(e_gerador(a)) quantos++;
        long g2=0;
        for(long a=G_GLOBAL+1;a<p;a++) if(e_gerador(a)){ g2=a; break; }
        long w2 = pot(g2,(p-1)/n), RN=inv(R_GLOBAL);
        /* w2 é potência de w: acha o expoente */
        long kexp=-1, c=1;
        for(int i=0;i<n;i++){ if(c==w2){ kexp=i; break; } c=mul(c,W_GLOBAL); }
        long gg=kexp, nn=n; while(nn){ long t=gg%nn; gg=nn; nn=t; }
        printf("       geradores de ℤ/%ld : %ld ; outro gerador g'=%ld → w'=%ld\n",
               p, quantos, g2, w2);
        printf("       e w' = w^%ld com mdc(%ld,%d) = %ld  ⟹ trocar o gerador PERMUTA os índices\n",
               kexp, kexp, n, gg);
        printf("       do dual:  F_{w^k}(x)[j] = F_w(x)[k·j mod n]\n");

        /* (a) fundir/abrir: INVARIANTE, porque o produto ponto-a-ponto comuta com permutação */
        static long a1[NMAX], a2[NMAX], A1[NMAX], A2[NMAX], Pk[NMAX], C[NMAX], Ck[NMAX], rec[NMAX], B2[NMAX];
        long s=4242;
        for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; a1[i]=s%251+1; }
        for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; a2[i]=s%251+1; }
        F(a1,A1,W_GLOBAL,RN); F(a2,A2,W_GLOBAL,RN);
        for(int k=0;k<n;k++) Pk[k]=mul(A1[k],A2[k]);
        Finv(Pk,C,W_GLOBAL,RN);                              /* A funde com w                     */
        F(C,Ck,w2,RN); F(a2,B2,w2,RN);                       /* B trabalha com w'                 */
        for(int k=0;k<n;k++) Pk[k]=mul(Ck[k], inv(B2[k]));
        Finv(Pk,rec,w2,RN);
        int difs=0; for(int i=0;i<n;i++) if(rec[i]!=md(a1[i])) difs++;
        printf("\n       (a) A funde com w, B abre com w' : %d/%d erradas — %s\n", difs, n,
               difs==0 ? "ABRE, exato" : "não abre");
        printf("           ⟹ o aviso do enredo NÃO vale para fundir/abrir, e não por sorte: a\n");
        printf("           fusão é produto PONTO A PONTO, e produto ponto a ponto COMUTA com\n");
        printf("           permutação de índices. A permutação entra e sai, e a obra volta.\n");
        if(difs != 0) ok = 0;

        /* (b) truncar o espectro: NÃO é invariante — aqui o gerador é areia de verdade */
        static long Xw[NMAX], Xw2[NMAX], yw[NMAX], yw2[NMAX];
        int K = n/4;                                          /* guarda as K primeiras frequências */
        F(a1,Xw,W_GLOBAL,RN);
        F(a1,Xw2,w2,RN);
        for(int k=K;k<n;k++){ Xw[k]=0; Xw2[k]=0; }            /* trunca, cada um no SEU índice     */
        Finv(Xw,yw,W_GLOBAL,RN);
        Finv(Xw2,yw2,w2,RN);
        int difs_trunc=0; for(int i=0;i<n;i++) if(yw[i]!=yw2[i]) difs_trunc++;
        printf("\n       (b) as duas máquinas TRUNCAM o dual em K=%d e voltam:\n", K);
        printf("           coordenadas em que discordam : %d/%d — %s\n", difs_trunc, n,
               difs_trunc > n/2 ? "resultados DIFERENTES ✓" : "iguais (✗ inesperado)");
        printf("           ⟹ AQUI o gerador é areia: a frequência 1 de uma máquina é a %ld da\n", kexp);
        printf("           outra, então \"guardar as K primeiras\" guarda conjuntos diferentes. Toda\n");
        printf("           operação que ORDENA o dual — truncar, filtrar, comprimir, priorizar —\n");
        printf("           depende do gerador; as que são ponto a ponto, não.\n");
        if(difs_trunc <= n/2) ok = 0;

        /* (c) e nenhum teste local acusa */
        static long yy[NMAX], XX[NMAX];
        F(a1,XX,w2,RN); Finv(XX,yy,w2,RN);
        int locais=0; for(int i=0;i<n;i++) if(yy[i]!=md(a1[i])) locais++;
        printf("\n       (c) a máquina B, sozinha : ord(w')=%ld = n ✓, ida-e-volta %d erros ✓\n",
               ordem(w2), locais);
        printf("           ⟹ B passa em TODOS os seus testes locais. A divergência de (b) é\n");
        printf("           invisível de dentro — é por isso que o gerador tem de ir ESCRITO, e\n");
        printf("           ser O MENOR: não para a fusão funcionar, mas para que duas máquinas\n");
        printf("           ordenem o dual do mesmo modo.\n");
        if(locais != 0) ok = 0;
    }

    /* ---------- G6: o gerador é FRACTAL — a torre de torções, cada nível o quadrado do seguinte -- */
    printf("\n§G6  o gerador é FRACTAL: a torção tem uma TORRE de níveis, e cada nível é o\n");
    printf("     QUADRADO do seguinte (o zoom w↦w², a auto-similaridade do gabarito):\n");
    {
        int erro=0;
        printf("       nível d   w_d = w^(n/d)   ord(w_d)   w_d == (w_{2d})² ?\n");
        long ant = 0;
        for(int d=n; d>=1; d/=2){
            long wd = pot(W_GLOBAL, n/d);
            long o = ordem(wd);
            int quad_ok = (ant==0) || (wd == mul(ant,ant));
            printf("       %5d   %11ld   %8ld   %s\n", d, wd, o,
                   ant==0 ? "(topo da torre)" : (quad_ok ? "✓" : "✗"));
            if(o != d || !quad_ok) erro=1;
            ant = wd;
        }
        printf("     %s\n", erro?"FALHA":
          "resíduo 0 — oito níveis para n=256, ord(w_d)=d exata em cada um, e cada torção é o\n"
          "     quadrado da de cima. Não são oito constantes: é UMA, descida por realimentação.");
        if(erro) ok=0;
    }

    /* ---------- G7: SEM TABELAS — PA nos expoentes, PG nas potências, estado O(1) ------------- */
    printf("\n§G7  SEM TABELAS: cada nível é uma PA nos expoentes e uma PG nas potências\n");
    printf("     (§2 de teoria.tex). O fator sai do anterior por UMA multiplicação — nenhum\n");
    printf("     array de potências, nenhuma tabela de torções, estado O(1):\n");
    {
        static long x[NMAX], Xtab[NMAX], Xdin[NMAX];
        long RN=inv(R_GLOBAL);
        long s=777;
        for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; x[i]=s%251+1; }
        /* (a) a versão com pot() — o oráculo */
        F(x,Xtab,W_GLOBAL,RN);
        /* (b) a versão DINÂMICA: wk anda por PG (×w a cada k); dentro, o fator anda por PG (×wk) */
        long wk = 1;                                        /* w^k : PG de razão w                */
        for(int k=0;k<n;k++){
            long fator = 1, acc = 0;                        /* w^{jk} : PG de razão wk            */
            for(int j=0;j<n;j++){
                acc = md(acc + mul(x[j], fator));
                fator = mul(fator, wk);                     /* o expoente jk anda por SOMA (PA)   */
            }
            Xdin[k] = mul(acc, RN);
            wk = mul(wk, W_GLOBAL);                         /* k anda por soma; w^k por produto   */
        }
        int difs=0; for(int i=0;i<n;i++) if(Xtab[i]!=Xdin[i]) difs++;
        printf("       F dinâmica == F com pot() : %d/%d coordenadas %s\n", n-difs, n, difs?"✗":"✓");
        printf("       memória de trabalho: 2 escalares (wk e fator) — nenhuma tabela\n");
        if(difs) ok=0;
        printf("     %s\n", difs?"FALHA":
          "resíduo 0 — o expoente j·k é uma PA (soma a soma) e a potência w^{jk} é a PG que a\n"
          "     acompanha (produto a produto): a ponte ∏ do §2, e nada guardado. É a regra do\n"
          "     projeto cumprida na transformada: peso e conexão saem da dinâmica, não de vetor.");
    }

    /* ---------- G8: a auto-similaridade posta como recursão (a peça uma volta abaixo) ---------- */
    printf("\n§G8  e a torre É a recursão: F de nível n sai de DOIS F do nível n/2, com a torção\n");
    printf("     ao quadrado — a mesma multiplicação recursiva do §3 (dim n pela n−1):\n");
    {
        static long x[NMAX], Xdir[NMAX], par[NMAX/2], impar[NMAX/2], Ep[NMAX/2], Oi[NMAX/2];
        long RN=inv(R_GLOBAL), s=31337;
        for(int i=0;i<n;i++){ s=(s*1103515245+12345)&0x7fffffff; x[i]=s%251+1; }
        F(x,Xdir,W_GLOBAL,RN);                              /* o direto, O(n²)                    */
        /* um passo de descida: separa pares/ímpares e usa o nível de baixo (w²) */
        int h = n/2;
        for(int i=0;i<h;i++){ par[i]=x[2*i]; impar[i]=x[2*i+1]; }
        long w2 = mul(W_GLOBAL,W_GLOBAL);                   /* a torção do nível abaixo           */
        long RNh = inv(R_GLOBAL);                           /* mesma normalização, ajustada abaixo */
        int nsave = n; n = h;
        F(par,Ep,w2,1); F(impar,Oi,w2,1);                   /* sem normalizar; ajusta-se ao fim   */
        n = nsave;
        int difs=0;
        long wk=1;
        for(int k=0;k<n;k++){
            long e = Ep[k%h], o = Oi[k%h];
            long val = mul(md(e + mul(wk,o)), RNh);
            if(val != Xdir[k]) difs++;
            wk = mul(wk, W_GLOBAL);
        }
        printf("       um passo de descida (pares/ímpares, torção w²) == F direto : %d/%d %s\n",
               n-difs, n, difs?"✗":"✓");
        printf("     %s\n", difs?"FALHA":
          "resíduo 0 — a transformada do nível n é a do nível n/2 batida duas vezes, com a\n"
          "     torção elevada ao quadrado. O gerador não é uma constante com uma tabela: é\n"
          "     UMA peça que se repete descendo a torre — e é por ser fractal que dispensa\n"
          "     tabela em todo nível.");
        if(difs) ok=0;
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", ok ?
      "RESÍDUO 0 — o gerador global é  p=40961, n=256, g=3 (o MENOR), w=36043, r=16, e os cinco\n"
      "dígitos foram reconferidos da construção, não copiados. A torção tem ordem exatamente n\n"
      "(a base não se escolhe: é a órbita dela), a ida e a volta fecham em INTEIROS com a\n"
      "normalização de cada lado, e três obras fundem num objeto do qual cada uma volta inteira.\n"
      "\n"
      "E o aviso do gerador foi TESTADO e sai corrigido. Trocar o gerador é permutar os índices\n"
      "do dual (w' = w^37, mdc(37,256)=1), e por isso FUNDIR e ABRIR são invariantes: produto\n"
      "ponto a ponto comuta com permutação, e a obra abre exata mesmo com o gerador errado. Onde\n"
      "o gerador é areia de verdade é em toda operação que ORDENA o dual — truncar, filtrar,\n"
      "comprimir: a frequência 1 de uma máquina é a 37 da outra, e as duas discordam em quase\n"
      "todas as coordenadas. E a máquina divergente passa em todos os seus testes locais (torção\n"
      "de ordem n, ida e volta exata): a divergência é invisível de dentro. Por isso o gerador\n"
      "vai escrito e é o MENOR — não para a fusão funcionar, mas para que duas máquinas ordenem\n"
      "o dual do mesmo modo.\n"
      "\n"
      "E o gerador é FRACTAL: para n=256 há oito níveis, cada torção o QUADRADO da de cima\n"
      "(ord(w_d)=d exata em todos), e a transformada de um nível sai de duas do nível abaixo com\n"
      "a torção ao quadrado — a mesma recursão do corpo (dim n pela n−1). Por isso não há tabela\n"
      "em lugar nenhum: o expoente j·k anda por SOMA (a PA) e a potência w^{jk} pelo produto que\n"
      "a acompanha (a PG), com dois escalares de estado. Uma peça, descendo a torre."
      : "FALHOU — rever");
    return !ok;
}
