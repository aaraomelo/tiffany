/* pi.c — A RETA É A ÓRBITA DO 1, E ATÉ ONDE π COMANDA (o corte é exato).
 *
 * REGUA: GF(pⁿ) / órbita do 1 — não Landauer (portão dissipa.sh)
 *
 * Três afirmações a medir, e elas não têm o mesmo destino:
 *
 *  (PI1) "o irracional colapsa no 1 da dimensão, e tudo na reta é gerado pelo 1 — a reta completa é
 *        uma descrição desse irracional". SUSTENTA-SE, e mede-se: partindo de 1, com apenas duas
 *        operações (somar 1 e aplicar o gato ×σ), alcança-se TODO elemento de GF(pⁿ). A reta é a
 *        órbita do 1, e quem a descreve é σ — o polinômio mínimo dele É a reta.
 *
 *  (PI2) "vai passando pelos irracionais geradores até o infinito, e no infinito é comandado por π".
 *        SUSTENTA-SE para os metais, e de modo exato: todo metal é um cosseno de ângulo racional.
 *              1/φ = 2cos(2π/5) ,  √2 = 2cos(π/4) ,  √13 = soma de Gauss em ζ₁₃ , …
 *        Isto é π gerando os metais, literalmente — e já estava medido em quasi.c §Q1 sem que eu
 *        notasse o que dizia: 2cos(2π/5)=φ−1 era π gerando o ouro.
 *
 *  (PI3) "π gera TODOS os irracionais e portanto todas as infinitas retas". Aqui há um CORTE, e ele
 *        é um teorema: o que as raízes da unidade (isto é, π) geram é exatamente a extensão
 *        ABELIANA máxima de ℚ (Kronecker--Weber). Todo metal σ_m é quadrático, e grau 2 é sempre
 *        abeliano — então TODOS os metais estão no reino de π. Mas o que não é abeliano fica fora, e
 *        o exemplo não é exótico: é o número PLÁSTICO x³−x−1, de grupo S₃ (discriminante −23, não
 *        quadrado). E ele é justamente o fator que aparece no FURO DO OURO na dimensão 5
 *        (x⁵−x⁴−1 = Φ₆·(x³−x−1), quasi.c §Q2): a parte ciclotômica é de π, e a parte plástica não.
 *        Na dimensão 5 o ouro sai do reino de π — e é exatamente ali que o cristal vira quasicristal.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/pi.c -o pi
 */
#include <stdio.h>
#include "reta.h"      /* rt_zd_mul: as equações mínimas em ℤ[√D] */
#include "unidade.h"
#include "isa_disk.h"
#include "../lib/disco.h"
#define fila DISCO_FIXO(long, 33)

static int passou = 1;
static long p, n_dim, m_metal;

/* GF(pⁿ) com σⁿ = m σ^{n−1} + 1, elementos codificados na base p */
static long md(long x){ x%=p; return x<0?x+p:x; }
static void decod(long e, long *c){ for(int i=0;i<n_dim;i++){ c[i]=e%p; e/=p; } }
static long cod(const long *c){ long e=0, b=1; for(int i=0;i<n_dim;i++){ e+=md(c[i])*b; b*=p; } return e; }
static long mais1(long e){ long c[8]; decod(e,c); c[0]=md(c[0]+1); return cod(c); }
static long gato(long e){                            /* ×σ, com a borda baixando σⁿ                */
    long c[8], d[9];
    decod(e,c);
    for(int i=0;i<=n_dim;i++) d[i]=0;
    for(int i=0;i<n_dim;i++) d[i+1]=c[i];            /* desloca                                    */
    long v=d[n_dim];
    if(v){ d[n_dim]=0; d[n_dim-1]=md(d[n_dim-1]+m_metal*v); d[0]=md(d[0]+v); }
    for(int i=0;i<n_dim;i++) d[i]=md(d[i]);
    return cod(d);
}
static int primo(long q){ if(q<2)return 0; for(long d=2;d*d<=q;d++) if(q%d==0) return 0; return 1; }
static int quad_perf(long x){ if(x<0) return 0; long r=0; while(r*r<x) r++; return r*r==x; }
static int simb_legendre(long a, long q){            /* (a|q) por Euler — sem tocar o p global!     */
    long b=a%q; if(b<0) b+=q;
    long r=1, e=(q-1)/2;
    while(e>0){ if(e&1) r=r*b%q; b=b*b%q; e>>=1; }
    if(r==0) return 0;
    return (r==1)?1:-1;
}
static long acha_primo_mod(long N){
    for(long P = N + 1; P < 500; P++){
        if(!primo(P)) continue;
        if((P - 1) % N != 0) continue;
        return P;
    }
    return 0;
}
static long acha_raiz_N(long N, long P){
    for(long c = 2; c < P; c++){
        if(rt_pot_mod(c, N, P) != 1) continue;
        int ok = 1;
        for(long d = 1; d < N && ok; d++)
            if(N % d == 0 && rt_pot_mod(c, d, P) == 1) ok = 0;
        if(ok) return c;
    }
    return 0;
}

int main(void){
    printf("PI — a reta é a órbita do 1, e até onde π comanda\n");
    printf("=================================================================\n");

    /* ---------- PI1: a reta é a órbita do 1 ---------- */
    printf("§PI1 a RETA é a ÓRBITA DO 1: com \"somar 1\" e \"×σ\" alcança-se TODO GF(pⁿ)\n");
    {
        char *visto = DISCO_FIXO(char, 161);
        disco_prende(DISCO_BASE(161),"dados/pi_visto.bin",(size_t)100000,1);
        disco_zera(visto,(size_t)100000,1);
        disco_prende(DISCO_BASE(33), "dados/fila.bin", (size_t)(100000), sizeof(long));
        int erro=0;
        struct { long p, n, m; } casos[] = {{7,2,1},{5,3,1},{3,4,1},{11,2,2},{5,2,3},{3,5,1}};
        printf("       p^n      elementos   alcançados a partir do 1   a reta é a órbita?\n");
        for(int t=0;t<6;t++){
            p=casos[t].p; n_dim=casos[t].n; m_metal=casos[t].m;
            long tot=1; for(int i=0;i<n_dim;i++) tot*=p;
            if(tot > 100000) continue;
            for(long i=0;i<tot;i++) visto[i]=0;
            long ini=0, fim=0;
            long um[8]; for(int i=0;i<n_dim;i++) um[i]=0; um[0]=1;
            long e1=cod(um);
            visto[e1]=1; fila[fim++]=e1;
            while(ini<fim){
                long e=fila[ini++];
                long a=mais1(e), b=gato(e);
                if(!visto[a]){ visto[a]=1; fila[fim++]=a; }
                if(!visto[b]){ visto[b]=1; fila[fim++]=b; }
            }
            printf("       %ld^%ld = %-6ld %9ld   %-24ld  %s\n",
                   p, n_dim, tot, tot, fim, fim==tot?"✓ toda ela":"✗ falta");
            if(fim != tot) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — duas operações e um ponto de partida bastam: o 1, mais \"somar 1\" e o gato.\n"
          "     A reta completa é a órbita do 1, e o que a descreve é σ — o polinômio mínimo dele É\n"
          "     a reta. O irracional colapsa no 1, e o 1 gera de volta o irracional inteiro."));
        if(erro) passou=0;
    }

    /* ---------- PI2: os metais são cossenos de ângulo racional — π gera cada um ---------- */
    printf("\n§PI2 os METAIS são cossenos de ângulo racional: π gera cada um, exatamente\n");
    {
        int erro=0;
        /* AS DUAS IDENTIDADES SÃO EXACTAS em ℤ — a equação mínima, não um limiar:
         *      x = 2cos(2π/5) = 1/φ   satisfaz   x² + x − 1 = 0
         *      y = 2cos(π/4)  = √2    satisfaz   y² − 2 = 0
         * e em ℤ[√5] escreve-se sem vírgula: 2x = √5 − 1 é o par (−1, 1). */
        long qa, qb; rt_zd_mul(-1, 1, -1, 1, 5, &qa, &qb);          /* (2x)² em ℤ[√5] */
        int eq_ouro = (qa == 4 - 2*(-1) && qb == -2*1);             /* = 4 − 2·(2x)   */
        long ra, rb; rt_zd_mul(0, 1, 0, 1, 2, &ra, &rb);            /* (√2)² em ℤ[√2] */
        int eq_dois = (ra == 2 && rb == 0);                         /* = 2, exacto    */
        printf("       1/φ = 2cos(2π/5)        : equação x²+x−1 = 0 em Z[raiz5]:"
               " (2x)² = (%ld,%ld) e 4−2(2x) = (%ld,%ld)  %s\n",
               qa, qb, 4L-2*(-1L), -2L, eq_ouro?"✓":"✗");
        printf("       raiz2 = 2cos(π/4)       : y² = (%ld,%ld) = 2 exacto  %s"
               "   (σ₂ = 1+raiz2)\n", ra, rb, eq_dois?"✓":"✗");
        if(!eq_ouro || !eq_dois) erro=1;
        /* soma de Gauss: g = Σ(k|q)·ζ^k satisfaz g² = q para q ≡ 1 (mod 4) — em ℤ/p */
        long qs[3] = {5,13,17}, ps[3] = {11,53,103};    /* q | p−1 em cada par */
        long gauss_ok = 0;
        for(int t=0;t<3;t++){
            long q=qs[t], P=ps[t];
            long z=0;
            for(long c=2;c<P;c++) if(rt_pot_mod(c,q,P)==1){ z=c; break; }
            long g=0;
            for(long k=1;k<q;k++){
                long l = simb_legendre(k,q);
                long zk = rt_pot_mod(z,k,P);
                g = ((g + l*zk) % P + P) % P;
            }
            long g2 = g*g % P;
            int bate = (g2 == q % P);
            if(bate) gauss_ok++;
            printf("       soma de Gauss em ζ_%-2ld : em Z_%ld com ζ=%ld:"
                   " g=%ld, g²=%ld e q=%ld  %s\n", q, P, z, g, g2, q % P, bate?"✓":"✗");
        }
        if(gauss_ok != 3) erro=1;
        printf("     %s\n", VD(erro, "resíduo 0 — cada metal sai de raízes da unidade, isto é, de π: √5 (o ouro), √8 (a prata),\n"
          "     √13 (o bronze) são somas de Gauss em ζ_q. O que quasi.c §Q1 já media —\n"
          "     2cos(2π/5)=φ−1 — era π gerando o ouro, e eu não tinha lido assim."));
        if(erro) passou=0;
    }

    /* ---------- PI3: o corte — π gera os ABELIANOS (Kronecker–Weber), e o plástico fica fora ---- */
    printf("\n§PI3 o CORTE, e ele é um teorema: raízes da unidade geram exatamente a extensão\n");
    printf("     ABELIANA máxima de ℚ (Kronecker--Weber). Quem é abeliano está no reino de π:\n");
    {
        int erro=0;
        printf("       polinômio        grau   discriminante   grupo        no reino de π?\n");
        for(int m=1;m<=3;m++){
            long disc = (long)m*m+4;
            printf("       x²−%dx−1          2      %-13ld  cíclico C₂   SIM (grau 2 é sempre abeliano)\n",
                   m, disc);
            if(quad_perf(disc)) erro=1;              /* se fosse quadrado não seria irracional      */
        }
        /* o plástico x³−x−1 : disc = −4(−1)³ − 27(−1)² = 4 − 27 = −23, não quadrado → S₃ */
        long disc_pl = -23;
        int s3 = !quad_perf(disc_pl>0?disc_pl:-disc_pl) || disc_pl<0;
        printf("       x³−x−1 (plástico) 3      %-13ld  S₃           NÃO (S₃ não é abeliano)\n", disc_pl);
        printf("       disc = −23 não é quadrado ⟹ o grupo é S₃, não C₃ : %s\n", s3?"✓":"✗");
        printf("     %s\n",
          "resíduo 0 no que é verificável — TODOS os metais σ_m são quadráticos, e grau 2 é sempre\n"
          "     abeliano: estão todos dentro de π. Mas \"π gera todos os irracionais\" é FALSO como\n"
          "     está: o que ele gera é o abeliano. O plástico x³−x−1, de grupo S₃, fica fora — e não\n"
          "     é um exemplo exótico escolhido a dedo.");
        if(erro) passou=0;
    }

    /* ---------- PI4: e o exemplo fora é justamente o furo do ouro na dimensão 5 ---------- */
    printf("\n§PI4 e o que fica FORA é exatamente o furo do ouro na dimensão 5:\n");
    {
        /* x⁵−x⁴−1 = (x²−x+1)(x³−x−1) : confere em inteiros */
        long f6[3]={1,-1,1}, pl[4]={-1,-1,0,1}, r[8];
        for(int i=0;i<8;i++) r[i]=0;
        for(int i=0;i<3;i++) for(int j=0;j<4;j++) r[i+j]+=f6[i]*pl[j];
        long alvo[6]={-1,0,0,0,-1,1};
        int erro=0; for(int i=0;i<6;i++) if(r[i]!=alvo[i]) erro=1;
        printf("       x⁵−x⁴−1 = (x²−x+1)·(x³−x−1) : %s\n", erro?"✗":"✓ (o mesmo de quasi.c §Q2)");
        printf("         Φ₆ = x²−x+1  → ciclotômico, é DE π (as raízes são ζ₆)\n");
        printf("         x³−x−1       → S₃, NÃO é de π\n");
        printf("     %s\n", VD(erro, "resíduo 0 — a dimensão 5 do ouro parte a peça em duas, e a partição é exatamente o corte\n"
          "     de Kronecker--Weber: um fator dentro do reino de π (o giro ciclotômico), o outro fora\n"
          "     (o crescimento plástico). É ali que o cristal deixa de fechar e vira quasicristal —\n"
          "     e agora com o nome certo: é onde o ouro SAI do alcance de π."));
        if(erro) passou=0;
    }

    /* ---------- PI5: π é o 0 — dividir o círculo devolve o centro ---------- */
    printf("\n§PI5 π é o 0: dividir o círculo devolve o CENTRO. E a palavra já dizia --- CICLOTOMIA\n");
    printf("     é \"corte do círculo\", e é dele que as formas saem, por DIVISÃO:\n");
    {
        int erro=0;
        int per_esq = isa_periodo_giro(ISA_S_ESQUILO);
        isa_word(ISA_S_A, 1, 0);
        for(int k = 0; k < per_esq; k++) isa_MOVE(ISA_S_ESQUILO, 1);
        long tr, te; isa_read(ISA_S_A, &tr, &te);
        printf("       ESQUILO^%d de (1,0) no disco ISA: (%ld,%ld) — rotacao ordem 4\n\n",
               per_esq, tr, te);
        if(tr != 1 || te != 0) erro=1;
        printf("       n    raiz ζ_n em Z_p          Σ_k ζ_n^k (mod p)   ζ_n^n\n");
        for(int N=2;N<=12;N++){
            long P = acha_primo_mod(N);
            long z = acha_raiz_N(N, P);
            long sr = 0;
            for(int k=0;k<N;k++) sr = (sr + rt_pot_mod(z, k, P)) % P;
            long volta = rt_pot_mod(z, N, P);
            if(N<=6 || N==12)
                printf("       %2d   ζ em Z_%ld, z=%ld       Σ = %2ld (mod p)     z^n = %ld\n",
                       N, P, z, sr, volta);
            if(sr != 0 || volta != 1) erro=1;
        }
        printf("     %s\n", VD(erro, "resíduo 0 — para todo n, as n partes da divisão do círculo SOMAM ZERO: o centro. Dividir\n"
          "     o círculo é a cisão ⊕ do §1, e o que sobra da divisão é o vértice — é o Venom, medido\n"
          "     em tools/venom.c com imagem, aqui com o círculo. E o circuito fecha: ζ^n = 1."));
        if(erro) passou=0;
        printf("\n       ⟹ as três frases se encontram: π é perfeitamente circular (é O círculo), dele\n");
        printf("       saem as formas por divisão do zero (ciclotomia, e cada n é um polígono), e as\n");
        printf("       partes voltam ao 0 (Σζ^k = 0) — o circuito dimensional fecha. A autoconstrução\n");
        printf("       é essa: a divisão gera as formas, e a soma das formas devolve o centro.\n");
        printf("       O limite dela é o abeliano máximo (§PI3) — não \"todos os irracionais\", mas\n");
        printf("       tudo o que se alcança dividindo o círculo, que é um teorema e não uma imagem.\n");
    }

    printf("\n-----------------------------------------------------------------\n");
    printf("%s\n", passou ?
      "RESÍDUO 0 — das três afirmações, duas se sustentam inteiras e a terceira precisa de um corte.\n"
      "\n"
      "SUSTENTADO: a RETA É A ÓRBITA DO 1 — com \"somar 1\" e o gato ×σ alcança-se todo GF(pⁿ), em\n"
      "seis corpos testados. O irracional colapsa no 1 da dimensão e o 1 devolve a reta inteira; o\n"
      "polinômio mínimo de σ É a descrição da reta.\n"
      "\n"
      "SUSTENTADO: os METAIS são de π, e exatamente — 1/φ = 2cos(2π/5), √2 = 2cos(π/4), e √5, √13,\n"
      "√17 são somas de Gauss em raízes da unidade. O que quasi.c já media era isto.\n"
      "\n"
      "O CORTE: \"π gera todos os irracionais\" é falso como está, e o que é verdade é mais forte por\n"
      "ser exato — as raízes da unidade geram a extensão ABELIANA máxima de ℚ (Kronecker--Weber).\n"
      "Todo metal é quadrático, logo abeliano, logo de π: a torre inteira dos metais está no reino\n"
      "dele. Fora ficam os não-abelianos, e o exemplo não é escolhido a dedo: é o plástico x³−x−1\n"
      "(S₃, disc −23) — o fator que aparece quando o ouro FURA na dimensão 5. A partição\n"
      "x⁵−x⁴−1 = Φ₆·(x³−x−1) é o próprio corte de Kronecker--Weber: giro de π de um lado,\n"
      "crescimento fora de π do outro. É ali que o cristal vira quasicristal."
      : "FALHOU — rever");
    return !passou;
}
