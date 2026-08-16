/* residuo_gerador.c — O RESÍDUO E O CORTE SÃO O MESMO PONTO FIXO, EM DOIS REGIMES.
 *
 * O Aarão: «vê referências do teorema do resíduo no corpo de Peano e vê se dá para
 * interpretar como o gerador; vê a ida e volta que já temos na recta geométrica via
 * polinómios, vê se se relaciona.»
 *
 * Dá, e a ligação estava escrita nos dois sítios sem se cruzarem.
 *
 * ── O QUE O UNIVERSAL JÁ DIZIA (thm:residuo) ────────────────────────────────────
 * «O RESÍDUO É O PONTO FIXO DA MONODROMIA: a volta age no espectro por
 *  ĉ_k ↦ ω^k·ĉ_k, e o único modo invariante é o que carrega o resíduo.»
 *
 * ── O QUE A RECTA GEOMÉTRICA JÁ DIZIA (thm:corte-ponto-fixo) ────────────────────
 * «O CORTE É O PONTO FIXO da Möbius x ↦ m + 1/x, visto do andar que não o contém.»
 *
 * São a MESMA palavra, e o que os separa é o regime — que é o discriminante:
 *
 *      ω    ponto fixo da MONODROMIA     |ω| = 1, ordem FINITA      D < 0, roda
 *      σ    ponto fixo da MÖBIUS         |σ| > 1, ordem INFINITA    D > 0, cresce
 *
 * ── E A FACTORIZAÇÃO SEPARA-OS, LITERALMENTE ────────────────────────────────────
 * O único polinómio da família que factoriza (thm:gerador-andar) parte-se exactamente
 * nos dois:
 *
 *      x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1) = Φ₆(x) · (x³ − x − 1)
 *                     └─ o ω, ordem 6 ─┘   └─ o σ, plástica ─┘
 *
 * O factor quadrático É o sexto CICLOTÓMICO: as suas raízes são as primitivas sextas da
 * unidade — o ω do teorema do resíduo. O cúbico é a razão plástica — o σ do corte. *O
 * andar que roda e o andar que cresce, separados por uma divisão exacta em ℤ.*
 *
 * ── E A IDA E VOLTA É A MESMA FRASE NOS DOIS ────────────────────────────────────
 *      no resíduo   a volta (soma de contorno) lê UMA coordenada: Σ f(z)z = M·c₋₁
 *      no corte     a volta (a Möbius inversa) chega ao ∞ EXACTO
 * — em ambos, a volta lê exactamente o que o gerador deixa fixo.
 *
 *   §R1  ω tem ordem 6 EXACTA em ℤ[x]/Φ₆ — o gerador que fecha
 *   §R2  σ nunca fecha: σ^k ≠ 1 para todo k, e o traço cresce — o gerador que abre
 *   §R3  a VOLTA lê um modo só: Σ_k ω^{jk} = 6·[6 | j], exacto em ℤ[ω]
 *   §R4  e a factorização SEPARA os dois, com a volta a reconstruir
 *
 * Nenhum double, nenhum limiar.
 *
 *   cc -O2 -std=c99 -I. -I../lib residuo_gerador.c -o residuo_gerador && ./residuo_gerador
 */
#include <stdio.h>
#include "unidade.h"

/* ── ℤ[ω] com ω² = ω − 1 (o sexto ciclotómico): z = a + b·ω ─────────────────────── */
typedef struct { long a, b; } Zw;
static Zw w_mul(Zw x, Zw y){
    /* (a+bω)(c+dω) = ac + (ad+bc)ω + bd·ω², e ω² = ω − 1 */
    Zw r; long bd = x.b*y.b;
    r.a = x.a*y.a - bd;
    r.b = x.a*y.b + x.b*y.a + bd;
    return r;
}
static int w_um(Zw x){ return x.a == 1 && x.b == 0; }

/* ── ℤ[σ] com σ³ = σ + 1 (a plástica): z = a + b·σ + c·σ² ───────────────────────── */
typedef struct { long a, b, c; } Zs;
static Zs s_mul(Zs x, Zs y){
    /* grau 4 reduzido por σ³ = σ+1, logo σ⁴ = σ² + σ */
    long p0 = x.a*y.a;
    long p1 = x.a*y.b + x.b*y.a;
    long p2 = x.a*y.c + x.b*y.b + x.c*y.a;
    long p3 = x.b*y.c + x.c*y.b;
    long p4 = x.c*y.c;
    Zs r;
    r.a = p0 + p3;                    /* σ³ → σ + 1 : o 1 */
    r.b = p1 + p3 + p4;               /* σ³ → σ ; σ⁴ → σ */
    r.c = p2 + p4;                    /* σ⁴ → σ² */
    return r;
}
static int s_um(Zs x){ return x.a == 1 && x.b == 0 && x.c == 0; }

int main(void){
    printf("\n=== O RESÍDUO E O CORTE SÃO O MESMO PONTO FIXO, EM DOIS REGIMES ===\n");

    /* ═══ §R1  ω TEM ORDEM 6 EXACTA — O GERADOR QUE FECHA ═══════════════════ */
    printf("\n§R1 ω tem ordem 6 exacta em ℤ[ω] — e é o ω do teorema do resíduo.\n\n");
    {
        /* A monodromia do thm:residuo é z ↦ ωz, e o ω é uma raiz M-ésima da unidade. Aqui
         * ele é concreto: a raiz de x² − x + 1 = Φ₆, e a aritmética corre em ℤ[ω] com
         * ω² = ω − 1 — inteira, sem avaliar raiz nenhuma. */
        Zw w = {0, 1}, p = {1, 0};
        long k, ordem = 0, antes = 0;
        printf("      k    ω^k = a + b·ω      é 1?\n");
        for(k = 1; k <= 12; k++){
            p = w_mul(p, w);
            if(w_um(p) && !ordem) ordem = k;
            if(w_um(p) && k < 6) antes++;
            if(k <= 6) printf("      %-4ld %+ld %+ld·ω%s          %s\n", k, p.a, p.b,
                              (p.a > -10 && p.a < 10) ? "  " : "", w_um(p) ? "SIM" : "não");
        }
        printf("      ordem de ω: %ld · e nenhum k < 6 dá 1: %s\n",
               ordem, antes == 0 ? "sim" : "NÃO");
        ok("ω TEM ORDEM 6 EXACTA, E É O GERADOR QUE FECHA: a monodromia do thm:residuo é"
           " z ↦ ωz com ω raiz da unidade, e aqui ele é concreto — a raiz de Φ₆ = x² − x +"
           " 1, com a aritmética a correr em ℤ[ω] por ω² = ω − 1, inteira e sem avaliar"
           " raiz nenhuma. ω⁶ = 1 e nenhuma potência menor o dá: a órbita FECHA, e é por"
           " isso que a soma de contorno tem um número finito de termos",
           ordem == 6 && antes == 0);
    }

    /* ═══ §R2  σ NUNCA FECHA — O GERADOR QUE ABRE ═══════════════════════════ */
    printf("\n§R2 σ nunca fecha: σ^k ≠ 1 para todo k, e cresce — o gerador do corte.\n\n");
    {
        /* O outro factor é x³ − x − 1, a razão PLÁSTICA — o menor Pisot. Em ℤ[σ] com
         * σ³ = σ + 1 as potências crescem e nunca voltam ao 1: a órbita ABRE.
         *
         * É essa a diferença de regime, e é ela que separa o resíduo do corte: um gerador
         * de ordem finita dá um contorno que fecha; um de ordem infinita dá um corte. */
        Zs s = {0, 1, 0}, p = {1, 0, 0};
        long k, fechou = 0, cresceu = 0, ant = 1;
        printf("      k    σ^k = a + b·σ + c·σ²      |coef| cresce?\n");
        for(k = 1; k <= 24; k++){
            p = s_mul(p, s);
            if(s_um(p)) fechou++;
            long mx = (p.a < 0 ? -p.a : p.a);
            if((p.b < 0 ? -p.b : p.b) > mx) mx = (p.b < 0 ? -p.b : p.b);
            if((p.c < 0 ? -p.c : p.c) > mx) mx = (p.c < 0 ? -p.c : p.c);
            if(mx >= ant) cresceu++;
            ant = mx;
            if(k <= 5 || k == 24)
                printf("      %-4ld %+ld %+ld·σ %+ld·σ²           %s\n", k, p.a, p.b, p.c,
                       mx >= 1 ? "sim" : "não");
        }
        /* E A ARITMÉTICA TEM DE SER MEDIDA, senão «cresce e nunca fecha» passa com a
         * redução errada — foi o que um gume mostrou. Duas leis que falham se σ⁴ ou σ³
         * estiverem mal reduzidos:
         *
         *   a DEFINIDORA   σ³ − σ − 1 = 0 em ℤ[σ]
         *   a CONSISTENTE  σ^i · σ^j = σ^{i+j}, para todo par
         *
         * A segunda é a que apanha o σ⁴: ela obriga a redução a ser a mesma vista de
         * qualquer decomposição do expoente. */
        Zs s3 = {0,0,1}; s3 = s_mul(s3, s);                  /* σ³ */
        int definidora = (s3.a == 1 && s3.b == 1 && s3.c == 0);   /* σ³ = 1 + σ */
        long pares = 0, consist = 0;
        Zs pw[16]; pw[0] = (Zs){1,0,0};
        for(int t = 1; t < 16; t++) pw[t] = s_mul(pw[t-1], s);
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            Zs e = s_mul(pw[i], pw[j]), f = pw[i+j];
            pares++;
            if(e.a == f.a && e.b == f.b && e.c == f.c) consist++;
        }
        printf("      σ^k = 1 em %ld dos %ld · e a magnitude não decresce em %ld\n",
               fechou, k-1, cresceu);
        printf("      e a aritmética mede-se: σ³ = 1 + σ (%s) · σ^i·σ^j = σ^{i+j} em %ld"
               " de %ld pares\n", definidora ? "sim" : "NÃO", consist, pares);
        ok("σ NUNCA FECHA, E É O GERADOR QUE ABRE: em ℤ[σ] com σ³ = σ + 1 — a razão"
           " PLÁSTICA, o menor Pisot — as potências crescem e σ^k não é 1 para nenhum k"
           " varrido. É esta a diferença de regime, e é ela que separa o resíduo do corte:"
           " um gerador de ordem FINITA dá um contorno que fecha e uma soma com um número"
           " finito de termos; um de ordem INFINITA dá uma órbita que não volta, e o corte"
           " é o ponto para onde ela vai. E a aritmética é medida, não suposta: a lei"
           " DEFINIDORA σ³ = 1 + σ, e a CONSISTÊNCIA σ^i·σ^j = σ^{i+j} em todos os pares —"
           " esta última obriga a redução a ser a mesma vista de qualquer decomposição do"
           " expoente, e sem ela «cresce e nunca fecha» passaria com σ⁴ mal reduzido",
           fechou == 0 && cresceu > 20 && definidora && consist == pares && pares == 64);
    }

    /* ═══ §R3  A VOLTA LÊ UM MODO SÓ ════════════════════════════════════════ */
    printf("\n§R3 A volta lê UMA coordenada: Σ_k ω^{jk} = 6·[6 | j], exacto em ℤ[ω].\n\n");
    {
        /* É a peça central do thm:residuo — «a volta só vê um modo» — e ela é inteira:
         *
         *      Σ_{k=0}^{5} ω^{jk} = 6  se 6 | j,  e  0  caso contrário
         *
         * É por isso que a soma de contorno selecciona c₋₁ e mais nada: os outros modos
         * cancelam-se EXACTAMENTE. E o mesmo cálculo diz qual é o ponto fixo da
         * monodromia — o modo j ≡ 0 (mod 6), o único que ω não move. */
        long js = 0, bate = 0, seis = 0, zeros = 0;
        printf("      j    Σ_{k=0..5} ω^{jk}        6 | j ?   esperado\n");
        for(long j = 0; j <= 18; j++){
            Zw soma = {0, 0}, pot = {1, 0}, wj = {1, 0}, w = {0, 1};
            for(long t = 0; t < j; t++) wj = w_mul(wj, w);     /* ω^j */
            for(long k = 0; k < 6; k++){
                soma.a += pot.a; soma.b += pot.b;
                pot = w_mul(pot, wj);
            }
            long esp = (j % 6 == 0) ? 6 : 0;
            js++;
            if(soma.a == esp && soma.b == 0) bate++;
            if(j % 6 == 0) seis++; else zeros++;
            if(j <= 7) printf("      %-4ld %+ld %+ld·ω                %-9s %ld\n",
                              j, soma.a, soma.b, (j%6==0) ? "sim" : "não", esp);
        }
        printf("      %ld valores de j: a identidade vale em %ld · %ld múltiplos de 6,"
               " %ld não\n", js, bate, seis, zeros);
        ok("A VOLTA LÊ UMA COORDENADA SÓ, E A CONTA É INTEIRA: Σ_{k=0..5} ω^{jk} dá 6"
           " quando 6 divide j e ZERO nos outros — exacto em ℤ[ω], sem uma raiz avaliada."
           " É por isso que a soma de contorno selecciona o resíduo e mais nada: os outros"
           " modos cancelam-se EXACTAMENTE, não aproximadamente. E o mesmo cálculo diz qual"
           " é o ponto fixo da monodromia — o modo j ≡ 0 (mod 6), o único que ω não move",
           bate == js && seis > 0 && zeros > 0);
    }

    /* ═══ §R4  E A FACTORIZAÇÃO SEPARA OS DOIS ══════════════════════════════ */
    printf("\n§R4 E a factorização separa os dois geradores, com a volta a fechar.\n\n");
    {
        /* x⁵ − x⁴ − 1 = (x² − x + 1)(x³ − x − 1) = Φ₆ · plástica.
         *
         * O quadrático é o sexto CICLOTÓMICO — divide x⁶ − 1 e não divide x³ − 1 nem
         * x² − 1, logo as suas raízes têm ordem EXACTAMENTE 6. O cúbico é a plástica.
         * A mesma divisão exacta que o thm:gerador-andar usa para dizer que o zero desce,
         * lida de outro lado: ela separa o gerador que RODA do gerador que CRESCE. */
        long P[6] = {-1, 0, 0, 0, -1, 1};        /* x⁵ − x⁴ − 1 */
        long F[3] = {1, -1, 1};                  /* Φ₆ = x² − x + 1 */
        long G[4] = {-1, -1, 0, 1};              /* x³ − x − 1 */
        long R[9] = {0};
        for(int i = 0; i <= 2; i++) for(int j = 0; j <= 3; j++) R[i+j] += F[i]*G[j];
        int volta = 1;
        for(int d = 0; d <= 5; d++) if(R[d] != P[d]) volta = 0;
        /* e Φ₆ divide x⁶ − 1 mas não x³ − 1 nem x² − 1: a ordem é EXACTAMENTE 6 */
        long S6[7] = {-1,0,0,0,0,0,1}, S3[4] = {-1,0,0,1}, S2[3] = {-1,0,1};
        int div6 = 1, div3 = 0, div2 = 0;
        {   /* divisão sintética por Φ₆, e o resto tem de ser zero só no primeiro */
            long A[7]; int n;
            n = 6; for(int d = 0; d <= n; d++) A[d] = S6[d];
            for(int d = n; d >= 2; d--){ long c = A[d]; A[d] -= c; A[d-1] -= c*F[1]; A[d-2] -= c*F[0]; }
            div6 = (A[0] == 0 && A[1] == 0);
            n = 3; for(int d = 0; d <= n; d++) A[d] = S3[d];
            for(int d = n; d >= 2; d--){ long c = A[d]; A[d] -= c; A[d-1] -= c*F[1]; A[d-2] -= c*F[0]; }
            div3 = (A[0] == 0 && A[1] == 0);
            n = 2; for(int d = 0; d <= n; d++) A[d] = S2[d];
            for(int d = n; d >= 2; d--){ long c = A[d]; A[d] -= c; A[d-1] -= c*F[1]; A[d-2] -= c*F[0]; }
            div2 = (A[0] == 0 && A[1] == 0);
        }
        printf("      (x² − x + 1)(x³ − x − 1) reconstrói x⁵ − x⁴ − 1: %s\n",
               volta ? "sim" : "NÃO");
        printf("      Φ₆ divide x⁶−1: %s · x³−1: %s · x²−1: %s  ⟹ ordem EXACTAMENTE 6\n",
               div6 ? "sim" : "NÃO", div3 ? "SIM (mau)" : "não", div2 ? "SIM (mau)" : "não");
        ok("E A FACTORIZAÇÃO SEPARA OS DOIS GERADORES: x⁵ − x⁴ − 1 = Φ₆·(x³ − x − 1), com"
           " a volta a reconstruir o original coeficiente a coeficiente. O factor"
           " quadrático É o sexto ciclotómico — divide x⁶ − 1 e NÃO divide x³ − 1 nem"
           " x² − 1, logo as raízes têm ordem exactamente 6 —, e é o ω do teorema do"
           " resíduo; o cúbico é a plástica, e é o σ do corte. A mesma divisão exacta que"
           " diz que o zero DESCE, lida de outro lado, separa o gerador que RODA do"
           " gerador que CRESCE",
           volta && div6 && !div3 && !div2);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  «O resíduo é o ponto fixo da monodromia» e «o corte é o ponto fixo\n");
        printf("  da Möbius» são a mesma frase em dois regimes, e o discriminante é\n");
        printf("  o que os separa: ω roda com ordem finita, σ cresce sem voltar.\n");
        printf("  E a volta lê, nos dois, exactamente o que o gerador deixa fixo —\n");
        printf("  lá uma coordenada do espectro, aqui o ∞ de onde a órbita partiu.\n");
    }
    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
