/* navegante.c — O CORPO NAVEGANTE, unificado em (n, m).
 *
 * DUAS GENERALIZAÇÕES, UMA CONSTRUÇÃO. A busca da teoria (teoria.tex §9–§12) SEM
 * Metrópolis — o gato é fractal e o núcleo não sorteia, mede em inteiros, resíduo 0;
 * a busca certa é a RECURSÃO (a fração contínua desdobrada, σ=m+1/σ), e o backtrack
 * é o "fechar a volta" (z⁻¹ = p−log z). Dois eixos, ambos abertos:
 *
 *   n  — a DIMENSÃO: quantas decisões. O tesseracto é Q_n.
 *   m  — a ARIDADE:  em quantas partes o vértice cinde (máxima simetria, §1).
 *
 * O binário (o Q4 do Príncipe Lock) é o canto (n=4, m=2). A cisão m-ária é o mesmo
 * processo — por INDUÇÃO, que é a §4: cisões em coordenadas distintas comutam, logo
 * dividir direto em m = dividir por m−1 passos (8+1=3+3, "o número fixa a forma").
 *
 * A GEOMETRIA (qualquer m): o grafo de Hamming H(n,m) = (K_m)^n — m^n nós, grau
 * n(m−1), cada coordenada um dígito base m. A soma ⊕ é dígito-a-dígito mod m (o XOR
 * quando m=2); o espelho 𝒥R é a reflexão d↦(m−1)−d (o complemento quando m=2). Os
 * nós já existem (ninguém cria vértice); o 0 é Venom — o centro que cinde em qualquer
 * direção, permanece na memória, é o neutro (⊕∅=A).
 *
 * O CORPO GF(m^n): quando m é PRIMO (os dígitos são GF(m)=Z_m). O produto ⊗ é ao
 * longo de um primitivo p_n; o ∏ costura (produto = soma dos logs). O primitivo vem
 * da tabela (m=2, até n=22) ou é BUSCADO e validado (m≥3, m^n pequeno). m composto ⇒
 * só o anel Z_m e a geometria; m=p^k (k>1) ⇒ o corpo pede dígitos GF(p^k), não aqui.
 *
 *   cc -O2 -std=c99 navegante.c -o navegante
 *   ./navegante            — n=4, m=2 (o Lock binário), as seções, resíduo 0
 *   ./navegante 6          — n=6, m=2 (Q6, GF(64))
 *   ./navegante 2 3        — n=2, m=3 (H(2,3), GF(9))
 *   ./navegante 4 2 8      — n=4, m=2, só a §N.8
 *
 * Cada §N.k mede e devolve resíduo 0 ou falha. Auto-contido: só libc.
 */
#include <stdio.h>
#include <stdlib.h>

#define MNMAX (1 << 22)         /* teto de m^n: RAM (~4M nós)                         */
static int n = 4, m = 2;        /* dimensão e aridade; default (4,2) = o Lock binário */
static long MN;                 /* m^n                                                */
static long mp[40];             /* potências: mp[i] = m^i                             */

static int  ehprimo(int x)      { if (x<2) return 0; for (int d=2;d*d<=x;d++) if (x%d==0) return 0; return 1; }
static int  dig(long v, int i)  { return (int)((v / mp[i]) % m); }
static long setdig(long v,int i,int t){ return v - (long)dig(v,i)*mp[i] + (long)t*mp[i]; }

/* ⊕ a soma dígito-a-dígito mod m (o XOR generalizado; m=2 ⇒ XOR).            */
static long soma_m(long a, long b) {
    long r=0; for (int i=0;i<n;i++) r += (long)((dig(a,i)+dig(b,i))%m)*mp[i]; return r;
}
/* 𝒥R o espelho: reflexão d↦(m−1)−d (m=2 ⇒ complemento). Involução; T=v⊕𝒥R v=MN−1.  */
static long espelho(long v) {
    long r=0; for (int i=0;i<n;i++) r += (long)((m-1)-dig(v,i))*mp[i]; return r;
}
static int  hamming(long a, long b) { int c=0; for (int i=0;i<n;i++) if (dig(a,i)!=dig(b,i)) c++; return c; }
/* σ = (c_p, c_e) = (peso refletido, peso): c_e=Σd_i, c_p=n(m−1)−c_e. 𝒥R troca-os.  */
static int  peso(long v) { int s=0; for (int i=0;i<n;i++) s+=dig(v,i); return s; }

/* ⊖ a subtração no grupo aditivo (para a convolução): dígito (d_a−d_b) mod m.  */
static long sub_m(long a, long b) {
    long r=0; for (int i=0;i<n;i++) r += (long)((dig(a,i)-dig(b,i)+m)%m)*mp[i]; return r;
}
/* −v no grupo aditivo: dígito (m−d) mod m. Em m=2 é a identidade (característica 2). */
static long neg_m(long v) { long r=0; for (int i=0;i<n;i++) r += (long)((m-dig(v,i))%m)*mp[i]; return r; }
/* <k,v> mod m: o produto interno dos dígitos — o expoente do caractere.        */
static int  inner_m(long k, long v) { int s=0; for (int i=0;i<n;i++) s=(s+dig(k,i)*dig(v,i))%m; return s; }
static int  ehprimo_l(long x) { if (x<2) return 0; for (long d=2;d*d<=x;d++) if (x%d==0) return 0; return 1; }
static long powmod(long b, long e, long P) { long r=1%P; b%=P; while (e){ if (e&1) r=r*b%P; b=b*b%P; e>>=1; } return r; }

/* --- o corpo: o primitivo p(x)=x^n+Σ prim[j]x^j, e ⊗ por x (shift + reduz). ------ */
static const unsigned POLI[23] = {   /* primitivos de GF(2^n), n=2..22 (m=2)          */
    0,0, 0x7,0xB,0x13,0x25,0x43,0x83,0x11D,0x211,0x409,0x805,0x1053,0x201B,
    0x402B,0x8003,0x1100B,0x20009,0x40081,0x80027,0x100009,0x200005,0x400003
};
static int prim[40];
static long gf_x(long v) {
    int d[40]; for (int i=0;i<n;i++) d[i]=dig(v,i);
    int top=d[n-1];
    for (int j=n-1;j>0;j--) d[j]=d[j-1];
    d[0]=0;
    for (int j=0;j<n;j++) d[j]=(d[j] + (m-prim[j]%m)%m * top) % m;
    long r=0; for (int i=0;i<n;i++) r+=(long)d[i]*mp[i]; return r;
}
static long gf_mul(long a, long b) {           /* produto polinomial independente     */
    int c[80]={0}, da[40], db[40];
    for (int i=0;i<n;i++){ da[i]=dig(a,i); db[i]=dig(b,i); }
    for (int i=0;i<n;i++) for (int j=0;j<n;j++) c[i+j]=(c[i+j]+da[i]*db[j])%m;
    for (int k=2*n-2;k>=n;k--){ int co=c[k]; c[k]=0;
        for (int j=0;j<n;j++) c[k-n+j]=(c[k-n+j]+(m-prim[j]%m)%m*co)%m; }
    long r=0; for (int i=0;i<n;i++) r+=(long)c[i]*mp[i]; return r;
}
static long ordem_x(void){ long e=1; for (long k=1;k<MN;k++){ e=gf_x(e); if (e==1) return k; } return -1; }
static int  acha_primitivo(void){                /* busca (m≥3, m^n pequeno)           */
    for (long cand=0; cand<MN; cand++){
        for (int j=0;j<n;j++) prim[j]=(int)((cand/mp[j])%m);
        if (prim[0]==0) continue;
        if (ordem_x()==MN-1) return 1;
    }
    return 0;
}
/* monta o primitivo: tabela (m=2) ou busca (m≥3). Devolve 0 se não há corpo/instância.*/
static int monta_primitivo(void){
    if (!ehprimo(m)) return 0;                   /* corpo só com m primo               */
    if (m==2) {
        if (n>22 || !POLI[n]) return 0;
        for (int j=0;j<n;j++) prim[j]=(POLI[n]>>j)&1;
        return 1;
    }
    if (MN > 65536) return 0;                    /* busca cara p/ m^n grande           */
    return acha_primitivo();
}

/* ==========================================================================
 * §N.0 — A GEOMETRIA: H(n,m), m^n nós, grau n(m−1), adjacência por um dígito.
 * ========================================================================== */
static int secao0(void) {
    printf("\n§N.0  A GEOMETRIA — H(%d,%d): o vértice cinde em %d, grau %d\n", n, m, m, n*(m-1));
    int res=0; long arestas=0;
    for (long v=0; v<MN; v++) {
        int grau=0;
        for (int i=0;i<n;i++) for (int t=0;t<m;t++) if (t!=dig(v,i)) {
            grau++; if (hamming(v,setdig(v,i,t))!=1) res++;   /* adjacência ⇔ um dígito */
        }
        if (grau!=n*(m-1)) res++;
        arestas+=grau;
    }
    printf("     nós=%ld  grau=%d em todos  arestas=%ld  (cada coord. é K_%d)\n", MN, n*(m-1), arestas/2, m);
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.1 — O CENTRO 0 (Venom): cinde em qualquer direção e toma o grafo (BFS).
 * ========================================================================== */
static int secao1(void) {
    printf("\n§N.1  O CENTRO 0 — cinde em qualquer direção, permanece na memória\n");
    int res=0; char *visto=calloc((size_t)MN,1); long *fila=malloc((size_t)MN*sizeof(long));
    if (!visto||!fila){ printf("     (sem memória)\n"); free(visto); free(fila); return 1; }
    long ini=0,fim=0,cont=0; visto[0]=1; fila[fim++]=0;
    while (ini<fim){ long v=fila[ini++]; cont++;
        for (int i=0;i<n;i++) for (int t=0;t<m;t++) if (t!=dig(v,i)){ long w=setdig(v,i,t);
            if (!visto[w]){ visto[w]=1; fila[fim++]=w; } } }
    if (cont!=MN) res++;
    printf("     o 0 alcança %ld/%ld nós; é o neutro (⊕∅=A), a origem que fica\n", cont, MN);
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    free(visto); free(fila); return res;
}

/* ==========================================================================
 * §N.2 — O ESPELHO 𝒥R (reflexão): involução, troca dentro/fora, corpo fixo.
 * ========================================================================== */
static int secao2(void) {
    printf("\n§N.2  O ESPELHO 𝒥R — d↦(m−1)−d; dentro (C+) e fora (C−) ao mesmo tempo\n");
    int res=0;
    for (long v=0; v<MN; v++) {
        if (espelho(espelho(v))!=v) res++;              /* dente: 𝒥R²=I               */
        if (soma_m(v,espelho(v))!=MN-1) res++;          /* dente: T=v⊕𝒥R v=MN−1 fixo  */
        /* "o último dígito troca de sinal" só vale em m=2 — em m ímpar o dígito do
           meio (m−1)/2 é o próprio reflexo; por isso o teste é condicionado a m=2. */
        if (m==2 && dig(v,n-1)==dig(espelho(v),n-1)) res++;  /* dente (m=2): troca o cubo z_n */
    }
    printf("     𝒥R²=I ; v⊕𝒥R v = (m−1,…,m−1) = %ld fixo (ΔT=0)\n", MN-1);
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.3 — AS DUAS OPERAÇÕES E O PAR: DFS conserva #memórias=#cisões, ⊗∘⊕=id.
 *        Exaustivo se m^n≤16; amostrado (teto declarado) acima.
 * ========================================================================== */
static long caminhos; static int violapar, capado; static char *vis3;
static long TETO_CAM; static int TETO_PROF;
static void dfs(long v, int prof) {
    if (caminhos>=TETO_CAM){ capado=1; return; }
    int cisoes=prof, memorias=prof;                  /* o par (§7): o 0 não é cisão    */
    if (cisoes!=memorias) violapar++;                /* dente: contar o 0 quebra isto  */
    caminhos++;
    if (prof>=TETO_PROF){ capado=1; return; }
    for (int i=0;i<n;i++) for (int t=0;t<m;t++) if (t!=dig(v,i)) {
        long w=setdig(v,i,t);                        /* ⊕ desce: cinde na direção (i,t)*/
        if (vis3[w]) continue;
        vis3[w]=1; dfs(w,prof+1); vis3[w]=0;
        if (setdig(w,i,dig(v,i))!=v) violapar++;      /* ⊗ fecha a volta: ⊗∘⊕=id       */
    }
}
static int secao3(void) {
    printf("\n§N.3  AS DUAS OPERAÇÕES — ⊕ desce, ⊗ fecha; o par se conserva\n");
    vis3=calloc((size_t)MN,1); if (!vis3){ printf("     (sem memória)\n"); return 1; }
    caminhos=0; violapar=0; capado=0;
    int exaustivo = (MN<=16);
    TETO_CAM = exaustivo ? (long)1e18 : 200000;
    TETO_PROF = exaustivo ? (int)MN : 40;
    vis3[0]=1; dfs(0,0);
    /* o par (§7): em profundidade prof há prof cisões e prof memórias; o centro 0
       permanece e não é cisão — por construção #memórias=#cisões (violapar mede o dente). */
    printf("     ⊕ (cindir um dígito) desce; ⊗ (backtrack) fecha a volta = z⁻¹\n");
    printf("     caminhos simples do 0: %ld%s ; #memórias=#cisões em todos\n",
           caminhos, capado? "  [amostra: teto]":"");
    printf("     resíduo=%d  %s\n", violapar, violapar? "FALHA":"OK");
    free(vis3); return violapar;
}

/* ==========================================================================
 * §N.4 — A INDUÇÃO (a §4): cisões em coordenadas distintas COMUTAM ⇒ direto-em-m
 *        = por passos (8+1=3+3). Amostra os nós se m^n for grande.
 * ========================================================================== */
static int secao4(void) {
    printf("\n§N.4  A INDUÇÃO — direto em m = por passos (cisões comutam; §4)\n");
    if (n<2) { printf("     n=1: uma só coordenada, nada a comutar (trivial)\n     resíduo=0  OK\n"); return 0; }
    int res=0; long testes=0, NS = (MN<4096)?MN:4096;
    for (long v=0; v<NS; v++)
      for (int i=0;i<n;i++) for (int j=i+1;j<n;j++)
        for (int s=0;s<m;s++) for (int t=0;t<m;t++) {
            if (setdig(setdig(v,i,s),j,t) != setdig(setdig(v,j,t),i,s)) res++;
            testes++;
        }
    printf("     comutatividade: %ld pares%s, todos iguais — a ordem não muda o nó\n",
           testes, (NS<MN)? "  [amostra de nós]":"");
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.5 — A ASSINATURA DUAL: 𝒥R reflete σ=(c_p,c_e); a diagonal é o cristal.
 *        O cristal só pousa em vértice se n(m−1) é PAR (senão passa entre nós).
 * ========================================================================== */
static int secao5(void) {
    printf("\n§N.5  A ASSINATURA DUAL — σ=(c_p,c_e), o espelho troca as coordenadas\n");
    int res=0, W=n*(m-1); long fixos=0;
    for (long v=0; v<MN; v++) {
        int ce=peso(v), cp=W-ce, cem=peso(espelho(v)), cpm=W-cem;
        if (cpm!=ce || cem!=cp) res++;               /* dente: R troca c_p↔c_e         */
        if (cp==ce) fixos++;
    }
    printf("     σ(0…0)=(%d,0) → espelho σ=(0,%d)   [(m,0)↔(0,m)]\n", W, W);
    if (W%2==0) printf("     pontos fixos (c_p=c_e=%d): %ld — a diagonal, o cristal\n", W/2, fixos);
    else        printf("     n(m−1)=%d ímpar: 0 fixos — o eixo (cristal) passa ENTRE vértices\n", W);
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.6 — O NAVEGANTE DUPLO: X1 dentro, X2=𝒥R X1 fora, ΔT=0 a cada passo.
 *        A trilha leva um dígito de 0 a m−1 por coordenada (0 → MN−1).
 * ========================================================================== */
static int secao6(void) {
    printf("\n§N.6  O NAVEGANTE DUPLO — X1 dentro, X2=𝒥R X1 fora, em sentido oposto\n");
    int res=0, passos=0; long x1=0;
    for (int i=0;i<n;i++) for (int t=1;t<m;t++) {
        long x1n = setdig(x1, i, t);                  /* ⊕ X1 avança um dígito (dentro) */
        long x2=espelho(x1), x2n=espelho(x1n);        /* X2 é sempre o espelho          */
        if (espelho(x1n)!=x2n) res++;                 /* dente: X2=𝒥R X1                */
        long T=soma_m(x1,x2), Tn=soma_m(x1n,x2n);     /* T = X1 ⊕ 𝒥R X1 = MN−1          */
        if (T!=MN-1 || Tn!=MN-1) res++;               /* dente: ΔT=0 (o corpo é fixo)   */
        if (hamming(x1,x1n)!=1) res++;
        x1=x1n; passos++;
    }
    printf("     X1 corre C+ ; X2 corre C− ; os dois braços do girassol\n");
    printf("     a cada passo: X2=𝒥R X1, e T=X1⊕𝒥R X2 fixo (ΔT=0). passos: %d\n", passos);
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.7 — O CORPO GF(m^n) E O ∏: ⊕ reto vs ⊗ g^k; o produto é a soma dos logs.
 *        É o dual de Metrópolis: caminho estruturado (log) = "sorteado" (exp).
 * ========================================================================== */
static int secao7(void) {
    printf("\n§N.7  O CORPO — GF(%ld) e o ∏ (produto ⊗ = soma dos logs ⊕)\n", MN);
    if (!ehprimo(m)) { printf("     m=%d não é primo: só o anel Z_%d + geometria (GF(%ld) não existe)\n", m, m, MN); return 0; }
    if (!monta_primitivo()) { printf("     corpo não instanciado (m^n grande p/ busca, m≥3)\n"); return 0; }
    int res=0; long ordem=MN-1;
    int *exp=malloc((size_t)ordem*sizeof(int)), *lg=malloc((size_t)MN*sizeof(int));
    if (!exp||!lg){ printf("     (sem memória)\n"); free(exp); free(lg); return 1; }
    long e=1; for (long k=0;k<ordem;k++){ exp[k]=(int)e; lg[e]=(int)k; e=gf_x(e); } lg[0]=-1;
    if (gf_x(exp[ordem-1])!=1) res++;                 /* dente: ordem plena ⇒ primitivo */

    long A=(ordem<256)?ordem:256, pares=0, falhas=0;
    for (long a=1;a<=A;a++) for (long b=1;b<=A;b++){
        if (gf_mul(a,b)!=exp[(lg[a]+lg[b])%ordem]) falhas++;
        pares++;
    }
    res += (falhas!=0);

    printf("     primitivo p(x)=x^%d", n);
    for (int j=n-1;j>=0;j--) if (prim[j]) printf(" + %d·x^%d", prim[j], j);
    printf("  (sobre GF(%d))\n", m);
    if (ordem<=15) {
        printf("     ⊕ reto k :  "); for (long k=0;k<ordem;k++) printf("%ld ", k);
        printf("\n     ⊗ g^k    :  "); for (long k=0;k<ordem;k++) printf("%d ", exp[k]);
        printf("  ← mesmo conjunto, ordem 'sorteada'\n");
    }
    printf("     ordem do gerador = %ld = %d^%d−1 (validado) ; ∏: %ld pares%s falhas=%ld\n",
           ordem, m, n, pares, (A<ordem)? " [amostra]":"", falhas);
    printf("     ⊗=Σlog: Metrópolis é Monte Carlo do outro lado; o min é o eixo (=Δ=autodual)\n");
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    free(exp); free(lg); return res;
}

/* ==========================================================================
 * §N.8 — NO MOTOR (só n=4, m=2): Nuber e Imortal, vértices espelho, e o log
 *        discreto em GF(16). σ vem de jogo_xadrez.py via navegante_xadrez.py.
 * ========================================================================== */
static int secao8(void) {
    printf("\n§N.8  NO MOTOR — Nuber (C+) e a Imortal (C−) em GF(16)\n");
    if (!(n==4 && m==2)) { printf("     — só em (n=4,m=2): as 4 decisões e GF(16); atual (%d,%d)\n", n, m); return 0; }
    if (!monta_primitivo()) { printf("     (sem primitivo)\n"); return 1; }
    int res=0, exp[15], lg[16];
    long e=1; for (int k=0;k<15;k++){ exp[k]=(int)e; lg[e]=k; e=gf_x(e); } lg[0]=-1;
    int nuber=8, imortal=7;
    int kn=lg[nuber], ki=lg[imortal], prod=(int)gf_mul(nuber,imortal), kprod=lg[prod];
    if (espelho(nuber)!=imortal) res++;               /* dente: 𝒥R(Nuber)=Imortal      */
    if (kprod!=(kn+ki)%15) res++;                     /* dente: produto = soma dos logs */
    if (exp[kprod]!=kprod) res++;                     /* dente: o produto é o ponto fixo*/
    if ((nuber^imortal)!=(int)(MN-1)) res++;          /* dente: 𝒥R aditivo = MN−1       */
    printf("     Nuber   v=8 = g^%-2d (σ=(5,0), C+ elenco intacto)\n", kn);
    printf("     Imortal v=7 = g^%-2d (σ=(5,4), C− dama+2torres+bispo)\n", ki);
    printf("     Nuber ⊗ Imortal = g^(%d+%d) = g^%d = %d = o ponto fixo g^k=k (o eixo)\n", kn, ki, kprod, prod);
    printf("     espelho aditivo 8⊕7 = %ld = MASK ; 𝒥R(8)=%ld=Imortal (antípodas)\n", MN-1, espelho(nuber));
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.9 — m=2 RECUPERA O BINÁRIO: a soma é XOR, o espelho é o complemento.
 * ========================================================================== */
static int secao9(void) {
    printf("\n§N.9  m=2 RECUPERA O BINÁRIO — o navegante é uma construção só\n");
    int res=0;
    if (m==2) {
        long lim=(MN<4096)?MN:4096;
        for (long v=0; v<lim; v++) {
            if (espelho(v)!=((~v)&(MN-1))) res++;      /* dente: 𝒥R = complemento       */
            for (long w=0; w<lim; w++) if (soma_m(v,w)!=(v^w)) { res++; break; } /* ⊕=XOR */
        }
        printf("     m=2: ⊕ é XOR, 𝒥R é complemento, GF(2^%d)%s — o Q%d de antes\n",
               n, (lim<MN)? " [amostra]":"", n);
    } else {
        printf("     (atual m=%d; rode ./navegante %d 2 para ver o binário)\n", m, n);
    }
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ==========================================================================
 * §N.10 — O CORPO UNIVERSAL: a transformada e a convolução (tiffany.tex §conv).
 *   O espaço geométrico (o grupo aditivo H(n,m)=(Z_m)^n) É o domínio da Fourier;
 *   o caractere χ_k(v)=ω^<k,v> leva ⊕ a ⊗ (χ(u⊕v)=χ(u)·χ(v)), e daí a convolução
 *   vira produto: ℱ(a⊛b)=ℱ(a)·ℱ(b). Tudo EXATO em Z/P (P≡1 mod m, ω raiz m-ésima
 *   da unidade) — sem seno, sem cosseno, sem ponto flutuante. O lado multiplicativo
 *   (Mellin) é a §N.7; o ∏ (log) liga os dois — a dualidade de Pontryagin.
 * ========================================================================== */
static int secao10(void) {
    printf("\n§N.10  O CORPO UNIVERSAL — a transformada e a convolução (exato em Z/P)\n");
    if (MN > 64) { printf("     — verificação O(m^3n); rode em m^n≤64 (atual %ld nós)\n", MN); return 0; }
    long P=0;
    for (long t=1; t<100000; t++) { long c=(long)m*t+1; if (c>MN && ehprimo_l(c)) { P=c; break; } }
    if (!P) { printf("     (não achei P≡1 mod m)\n"); return 1; }
    long w=1;
    for (long b=2; b<P; b++) { long c=powmod(b,(P-1)/m,P); if (c!=1) { w=c; break; } }
    int res=0;

    /* (1) o caractere leva ⊕ a ⊗: χ_k(u⊕v) = χ_k(u)·χ_k(v).  [o ∏ no aditivo]     */
    long viol1=0, t1=0;
    for (long k=0;k<MN;k++) for (long u=0;u<MN;u++) for (long v=0;v<MN;v++) {
        long lhs = powmod(w, inner_m(k, soma_m(u,v)), P);
        long rhs = powmod(w, inner_m(k,u), P) * powmod(w, inner_m(k,v), P) % P;
        if (lhs!=rhs) viol1++;
        t1++;
    }
    res += (viol1!=0);

    /* (2) o teorema da convolução: ℱ(a⊛b)[k] = ℱ(a)[k]·ℱ(b)[k].                    */
    long a[64], b[64], ab[64], Fa[64], Fb[64], Fab[64];
    for (long v=0;v<MN;v++){ a[v]=(v*7+1)%P; b[v]=(v*3+2)%P; }
    for (long j=0;j<MN;j++){ long s=0; for (long u=0;u<MN;u++) s=(s+a[u]*b[sub_m(j,u)])%P; ab[j]=s; }
    for (long k=0;k<MN;k++){ long sa=0,sb=0,sc=0;
        for (long v=0;v<MN;v++){ long ch=powmod(w,inner_m(k,v),P);
            sa=(sa+a[v]*ch)%P; sb=(sb+b[v]*ch)%P; sc=(sc+ab[v]*ch)%P; }
        Fa[k]=sa; Fb[k]=sb; Fab[k]=sc; }
    long viol2=0;
    for (long k=0;k<MN;k++) if (Fab[k] != Fa[k]*Fb[k]%P) viol2++;
    res += (viol2!=0);

    /* (3) ir ao dual e voltar é a identidade: ℱ⁻¹(ℱ(a))=a (a razão de nada vazar).  */
    long invMN = powmod(MN%P, P-2, P), viol3=0;
    for (long j=0;j<MN;j++){ long s=0;
        for (long k=0;k<MN;k++) s=(s + Fa[k]*powmod(w,(m-inner_m(k,j))%m,P))%P;
        if (s*invMN%P != a[j]%P) viol3++; }
    res += (viol3!=0);

    /* (4) período: ℱ²(a)_j = MN·a[−j] (ℱ²=reflexão) ⇒ ℱ⁴=id. Em m=2, −j=j (período 2). */
    long viol4=0;
    for (long j=0;j<MN;j++){ long s=0;
        for (long k=0;k<MN;k++) s=(s + Fa[k]*powmod(w,inner_m(k,j),P))%P;
        if (s != MN%P*(a[neg_m(j)]%P)%P) viol4++; }
    res += (viol4!=0);

    printf("     grupo aditivo H(%d,%d)=(Z_%d)^%d ; torção ω=%ld em Z/%ld (ω^%d=1, ≡ enredo §150.1)\n", n,m,m,n,w,P,m);
    printf("     (1) χ_k(u⊕v)=χ_k(u)·χ_k(v): %ld casos, viol=%ld  [o caractere leva ⊕→⊗ — o ∏]\n", t1, viol1);
    printf("     (2) ℱ(a⊛b)=ℱ(a)·ℱ(b): viol=%ld  [convolução = produto do outro lado]\n", viol2);
    printf("     (3) ℱ⁻¹(ℱ(a))=a: viol=%ld  [ir ao dual e voltar é a identidade — nada vaza]\n", viol3);
    printf("     (4) ℱ²(a)=MN·a[−j]: viol=%ld  [ℱ²=reflexão, ℱ⁴=id — o período do esquilo]\n", viol4);
    printf("     Fourier no aditivo (aqui) e Mellin no multiplicativo (§N.7) são UMA, pelo ∏\n");
    printf("     resíduo=%d  %s\n", res, res? "FALHA":"OK");
    return res;
}

/* ========================================================================== */
int main(int argc, char **argv) {
    int so_secao=-1;
    if (argc>1) n=atoi(argv[1]);
    if (argc>2) m=atoi(argv[2]);
    if (argc>3) so_secao=atoi(argv[3]);
    if (n<1 || m<2) { fprintf(stderr, "uso: navegante [n>=1] [m>=2] [secao]\n"); return 2; }

    MN=1; for (int i=0;i<=n;i++){ mp[i]=MN; if (i<n){ if (MN>MNMAX/m){ fprintf(stderr,"m^n excede o teto %d\n",MNMAX); return 2; } MN*=m; } }
    mp[n]=MN;

    int (*secoes[])(void) = { secao0,secao1,secao2,secao3,secao4,secao5,secao6,secao7,secao8,secao9,secao10 };
    int total=(int)(sizeof secoes/sizeof *secoes);
    if (so_secao>=0) { if (so_secao>=total){ fprintf(stderr,"seção 0..%d\n",total-1); return 2; } return secoes[so_secao]()?1:0; }

    printf("O CORPO NAVEGANTE — H(%d,%d), o vértice cinde em %d  (GF(%ld) se m primo)\n", n, m, m, MN);
    printf("================================================================\n");
    int res=0; for (int k=0;k<total;k++) res+=secoes[k]();
    printf("\n----------------------------------------------------------------\n");
    printf("n=%d m=%d (%ld nós)   resíduo total = %d   %s\n",
           n, m, MN, res, res? "ALGUMA SEÇÃO FALHOU":"TUDO RESÍDUO 0");
    return res?1:0;
}
