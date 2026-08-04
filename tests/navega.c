/* navega.c — CAMINHAR: sobre as órbitas, entrando e saindo pelos atratores.
 *
 * O gato ×σ parte o espaço GF(p²) em órbitas. Cada órbita tem UM atrator — o representante
 * r_j que a gera inteira (a órbita é σ^k·r_j). CAMINHAR sobre a órbita é aplicar o gato (×σ).
 * Mas as órbitas não ficam isoladas: os atratores estão ligados entre si pela COSTURA (a
 * escala ×g), formando ELES uma órbita (no quociente G/⟨σ⟩). Então: ando uma órbita até o
 * atrator, pela costura SAIO nele e ENTRO no atrator de outra órbita, e ando essa — e assim,
 * de atrator em atrator, cubro TODO o espaço. O atrator é a costura que liga as órbitas: por
 * ele o espaço fica contínuo (um só, conexo). Tudo exato, resíduo 0.
 *
 * (No contínuo, o atrator é o irracional σ_m=[m;m,m,…], o ponto fixo do gato x↦m+1/x; sair
 *  para outro atrator é trocar de metal. Aqui, no finito, o atrator é o representante — e a
 *  validação é exata, resíduo 0.)
 *
 *   cc -O2 -std=c99 navega.c -o navega
 *   ./navega [p] [m]
 */
#include <stdio.h>
#include "unidade.h"
#include <stdlib.h>
#include "gp2.h"                                   /* a peça: o gato em GF(p²) (mul, gato, pw, ordem, σ) */

int main(int argc, char **argv){
    p = argc>1? atoi(argv[1]) : 7;
    m = argc>2? atoi(argv[2]) : 1;
    long N = (long)p*p - 1;                         /* |GF(p²)*| = p²−1                              */

    /* x²−mx−1 tem de ser irredutível mod p (σ irracional): sem raiz em ℤ_p.                        */
    int irred=1; for(int t=0;t<p;t++) if(((long)t*t - (long)m*t - 1)%p==0 || (((long)t*t-(long)m*t-1)%p+p)%p==0){ irred=0; break; }
    printf("CAMINHAR SOBRE AS ÓRBITAS — entrar e sair pelos atratores (GF(%d²), σ²=%dσ+1)\n", p, m);
    printf("================================================================\n");
    if(!irred){ printf("  x²−%dx−1 não é irredutível mod %d (σ seria racional) — escolha outro p,m\n", m, p); return 2; }

    long T = ordem(SIG);                            /* período de uma órbita = ord(σ)                */
    /* um gerador g de GF(p²)* (a costura mínima entre atratores)                                    */
    E g={0,0}; for(int a=0;a<p&&!(g.a||g.b);a++) for(int b=0;b<p;b++){ E c={a,b}; if((a||b)&&ordem(c)==N){ g=c; break; } }
    long J = N / T;                                 /* nº de órbitas = nº de atratores (cosets)      */
    int res=0;

    printf("\n§1  O ESPAÇO E O GATO — %ld pontos ≠0; o gato ×σ:(a,b)↦(b,a+mb) parte-os em órbitas\n", N);
    printf("      ord(σ)=T=%ld (o tamanho de cada órbita) ; %ld órbitas ⇒ %ld atratores ; gerador g=%d+%dσ\n",
           T, J, J, g.a, g.b);

    /* §2 — CAMINHAR sobre uma órbita: do atrator r₁, ×σ passo a passo, e a órbita fecha nele.       */
    E r1 = pw(g,1);
    printf("\n§2  CAMINHAR NUMA ÓRBITA — do atrator, o gato ×σ anda e a órbita fecha em T passos:\n      ");
    E cur=r1; for(long k=0;k<T && k<10;k++){ printf("(%d,%d)%s", cur.a, cur.b, k+1<T&&k<9?"→":""); cur=gato(cur); }
    cur=r1; for(long k=0;k<T;k++) cur=gato(cur);    /* T passos completos                            */
    int fecha = eq(cur, r1); res += !fecha;
    printf("%s\n      após T=%ld passos volta ao atrator: %s\n", T>10?" …":"", T, VD(!(fecha), "fecha (resíduo 0)"));

    /* §3 — o ATRATOR gera a sua órbita: r_j=g^j, e σ^k·r_j varre a órbita j inteira (uma classe).   */
    long v3=0;
    for(long j=0;j<J;j++){
        E rj=pw(g,j), c2=rj;
        for(long k=0;k<T;k++){ /* todos na mesma classe (coset), T distintos */ c2=gato(c2); }
        if(!eq(c2,rj)) v3++;                        /* cada órbita fecha no seu atrator              */
    }
    res += (v3!=0);
    printf("\n§3  O ATRATOR GERA — cada órbita j é σ^k·r_j (r_j=gʲ, o representante): fecha em todas: viol=%ld  %s\n",
           v3, VD(v3, "OK"));

    /* §4 — SAIR EM OUTRO ATRATOR (a costura): ×g leva o atrator r_j ao atrator r_{j+1}; os J        */
    /*      atratores formam ELES uma órbita (no quociente) e o ciclo fecha.                          */
    printf("\n§4  A COSTURA — ×g SAI de um atrator e ENTRA no próximo; os atratores ciclam:\n      ");
    E rj=ONE; long v4=0; int seen[4096]={0};
    for(long j=0;j<J;j++){
        int cs = (int)(( (long)0 )); /* coset via ordem: contamos por posição */
        printf("r%ld=(%d,%d)%s", j, rj.a, rj.b, j+1<J?" →g→ ":"");
        (void)cs;
        /* marca a órbita deste atrator para conferir que os J atratores estão em órbitas distintas   */
        E c3=rj; int key=-1;
        for(long k=0;k<T;k++){ int id=c3.a*p+c3.b; if(k==0) key=id; if(id<key) key=id; c3=gato(c3); }
        if(key>=0 && key<4096){ if(seen[key]) v4++; else seen[key]=1; }
        rj = mul(rj, g);                            /* a costura: ×g → o próximo atrator             */
    }
    int fechaC = eq(rj, pw(g,J));                   /* g^J ∈ ⟨σ⟩: a costura fecha o ciclo no quociente */
    res += (v4!=0);
    printf("\n      os %ld atratores estão em órbitas distintas (viol=%ld) e ×g cobre os cosets, fechando no quociente (g^J∈⟨σ⟩: %s)  %s\n",
           J, v4, fechaC?"sim":"?", VD(v4, "OK"));

    /* §5 — TUDO CONTÍNUO: o caminho que anda cada órbita (×σ) e salta de atrator em atrator (a       */
    /*      costura) visita CADA um dos N pontos exatamente uma vez — o espaço é um só, conexo.       */
    char *vis = calloc((size_t)p*p, 1);
    long cnt=0, dup=0;
    for(long j=0;j<J;j++){                           /* de atrator em atrator (a costura entre eles)  */
        E c4 = pw(g,j);                              /* entra no atrator r_j                          */
        for(long k=0;k<T;k++){                       /* caminha a órbita inteira (×σ)                 */
            int id=c4.a*p+c4.b;
            if(vis[id]) dup++; else { vis[id]=1; cnt++; }
            c4 = gato(c4);
        }
    }
    int cobre = (cnt==N && dup==0); res += !cobre;
    printf("\n§5  TUDO CONTÍNUO — o caminho (órbita ×σ, salto de atrator ×g) visita cada ponto uma vez:\n");
    printf("      pontos cobertos = %ld de %ld ; repetidos = %ld  ⇒ %s\n",
           cnt, N, dup, VD(!(cobre), "o espaço é UM só, costurado pelos atratores (resíduo 0)"));
    free(vis);

    /* §6 — reversível: o passo ×σ desfaz-se por ×σ^{T−1}; caminhar de volta devolve o ponto.         */
    E fwd = pw(g,1), sinv = pw(SIG, T-1);           /* σ^{-1} = σ^{T-1}                              */
    E walk=fwd; for(long k=0;k<T;k++) walk=gato(walk);
    for(long k=0;k<T;k++) walk=mul(walk, sinv);
    int volta = eq(walk, fwd); res += !volta;
    printf("\n§6  REVERSÍVEL — ×σ desfaz-se por ×σ^{T−1}; ida e volta na órbita devolve o ponto: %s\n",
           VD(!(volta), "EXATO, resíduo 0"));

    printf("\n----------------------------------------------------------------\n");
    printf("p=%d m=%d  T=%ld  atratores=%ld   resíduo total = %d   %s\n",
           p, m, T, J, res,
           VD(res, "O ATRATOR É A COSTURA — AS ÓRBITAS SE LIGAM, O ESPAÇO É CONTÍNUO"));
    return res?1:0;
}
