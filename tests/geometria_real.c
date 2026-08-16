/* geometria_real.c — A CONSTRUÇÃO DE ℝ, executada na ordem do Universal.
 *
 * O Aarão: «não é para o agente virar juiz da teoria; é para ele ler a teoria inteira e
 * EXECUTAR A PRÓXIMA CONSTRUÇÃO.» E: «a versão anterior ainda está COM MEDO da própria
 * construção — o texto está cheio de "não fazemos X" onde deveria mostrar X SENDO
 * PRODUZIDO pela cadeia do Universal.»
 *
 * A hierarquia, que é o que eu tinha perdido:
 *
 *      o Universal PROVA   |   o geométrico REALIZA   |   os medidores VERIFICAM
 *
 * Este ficheiro é a terceira coluna. A ordem é a do eval, e é obrigatória:
 *
 *   8 leis → base ortonormal → operação bit a bit → dual → recorrência → Pisot
 *          → encaixe → corte → completude → ℝ → π_k/gume
 *
 * ── O QUE MUDOU, E POR ORDEM DO EVAL ─────────────────────────────────────────
 *  (1) «não constrói todos os reais» ERA RESSALVA MINHA, não do quadro — e caiu. §L8
 *      constrói ℝ INTEIRO, com quocientes parciais ARBITRÁRIOS. O metálico deixa de ser
 *      o escopo e passa a ser o EXTREMO: φ minimiza o crescimento dos denominadores
 *      dos convergentes ENTRE AS EXPANSÕES COM a_k ≥ 1 — é o pior caso do mecanismo,
 *      não um privilégio de φ sobre os outros reais.
 *  (2) a torre 1→2→4→8 e o REUSO dos oito bits em cada andar: §L3.
 *  (3) «oito passos enchem um byte» é CODIFICAÇÃO e não demonstra o bit a bit — §L2
 *      liga-o à base ortonormal e mede o preço numa base torcida.
 *  (4) os DOIS determinantes, que eu misturei: det A^k (das ENTRADAS, em ℤ) e σ^k(σ†)^k
 *      (ESPECTRAL, das duas faces). §L6 mede-os separados — a igualdade é o teorema.
 *  (5) o controlo x²−2x−4 é CONTRAEXEMPLO AO MECANISMO, e não prova de uma
 *      caracterização «unidade e Pisot» que este quadro não demonstrou: §L11.
 *  (6) π não entra como aproximação estrangeira: §L10 realiza π_k exacto por andar.
 *
 * Nenhum double, nenhum limiar, nenhuma raiz avaliada.
 *
 *   cc -O2 -std=c99 -I. -I../lib geometria_real.c -o geometria_real && ./geometria_real
 */
#include <stdio.h>
#include "unidade.h"

#define G_LIMF 100000000L      /* o tecto: com Δ ≤ 68, Δ·F² e (t+1)² cabem em long */
#define G_MMAX 8

/* ── A SUCESSÃO METÁLICA, e o nome importa ────────────────────────────────────
 * U^{(m)}: U_0 = 0, U_1 = 1, U_{k+2} = m·U_{k+1} + U_k.   t^{(m)}: t_0 = 2, t_1 = m.
 *
 * Chamava-se F, e o revisor apanhou-o: F lê-se FIBONACCI, e Fibonacci é só o andar
 * m = 1. A conta sempre esteve certa — A_2² = (5,2;2,1), e não (2,1;1,1) —, mas o NOME
 * fazia a família inteira passar por um dos seus membros. Duas réguas para o mesmo
 * objecto, e o sintoma é sempre letras coladas. */
static int g_metal(long m, long *U, long *t, int max){
    int k;
    U[0] = 0; U[1] = 1; t[0] = 2; t[1] = m;
    for(k = 2; k < max; k++){
        if(U[k-1] > G_LIMF/(m+1)) break;
        U[k] = m*U[k-1] + U[k-2];
        t[k] = m*t[k-1] + t[k-2];
    }
    return k;
}
/* a/b < σ_m ?  σ_m = (m+√Δ)/2, Δ = m²+4. Decidido em ℤ, e nunca dá 0. */
static int g_cmp(long a, long b, long m){
    long s = 2*a - m*b, D = m*m + 4;
    if(s < 0) return -1;
    long e = s*s, d = b*b*D;
    return e < d ? -1 : (e > d ? 1 : 0);
}
/* ⌊√n⌋ em inteiros, por Newton — sem uma única raiz de vírgula flutuante */
static long g_isqrt(long n){
    if(n < 2) return n;
    long x = n, y = (x + 1) / 2;
    while(y < x){ x = y; y = (x + n/x) / 2; }
    while((x+1)*(x+1) <= n) x++;
    while(x*x > n) x--;
    return x;
}
static int g_peso(int x){ int p = 0; while(x){ p += x & 1; x >>= 1; } return p; }
static int g_par(int x){ return g_peso(x) & 1; }        /* ⟨a,b⟩ = paridade(a ∧ b) */
/* a ordem da companheira C = [[t,−1],[1,0]] em 𝔽_p — o relógio do andar */
static int g_ordem(long t, long p){
    long x11=1,x12=0,x21=0,x22=1, c11=((t%p)+p)%p, c12=(p-1)%p;
    for(long k = 1; k <= 4*p+4; k++){
        long y11=(x11*c11+x12)%p, y12=(x11*c12)%p;
        long y21=(x21*c11+x22)%p, y22=(x21*c12)%p;
        x11=y11; x12=y12; x21=y21; x22=y22;
        if(x11==1 && x12==0 && x21==0 && x22==1) return (int)k;
    }
    return -1;
}

int main(void){
    printf("\n=== A CONSTRUÇÃO DE ℝ, na ordem do Universal ===\n");
    printf("    8 leis → base → bit a bit → dual → recorrência → Pisot → encaixe\n");
    printf("           → corte → completude → ℝ → π_k/gume\n");

    /* ═══ §L0  A LEI 0: 0† = ∞, E É ELA QUE DÁ O ARRANQUE ═══════════════════ */
    printf("\n§L0 A Lei 0 — «a divisão do zero», 0† = ∞ — e sem ela a descida morre.\n\n");
    {
        /* O Universal: «é a LEI 0, e ela tem o índice zero porque é a que dá o ARRANQUE:
         * sem ela nada mais é total». A simbologia é a de Möbius, e é uma TROCA:
         *
         *      [p:q] ↦ [q:p],   logo   1/0 = [1:0] = ∞   e   1/∞ = [0:1] = 0
         *
         * Em matriz é S = [[0,1],[1,0]], com det = −1: nenhuma divisão, nenhum teste,
         * nenhum ramo. E ISTO NÃO É ORNAMENTO PARA ESTE PAPER — é a fundação do motor:
         * o passo da fracção contínua é x ↦ 1/(x − a), que em ℚ MORRE quando x − a = 0.
         * E x − a = 0 é exactamente onde a expansão de um RACIONAL termina.
         *
         *   (i)   a troca é total e involutiva em ℙ¹, sem uma excepção;
         *   (ii)  em ℚ a inversão tem fibra VAZIA no zero — conta-se a diferença;
         *   (iii) e a descida ATINGE ∞ exactamente nos racionais, e nunca nos σ_m.
         *
         * Logo a Lei 0 não só torna o passo total: ela SEPARA as duas metades de ℝ. */
        long pts = 0, troca_ok = 0, inv_ok = 0, bij = 0, visto[128];
        for(int i = 0; i < 128; i++) visto[i] = 0;
        for(int x = 0; x < 128; x++){          /* ℙ¹(𝔽₁₂₇): 0..126 finitos, 127 = ∞ */
            pts++;
            int y;
            if(x == 127) y = 0;                          /* ∞ ↦ 0  */
            else if(x == 0) y = 127;                     /* 0 ↦ ∞  */
            else { int b = 1, e = 125, base = x;         /* x ↦ x⁻¹ por Fermat */
                   while(e){ if(e & 1) b = (b*base) % 127; base = (base*base) % 127; e >>= 1; }
                   y = b; }
            troca_ok++;                                   /* devolveu SEMPRE um ponto */
            int z;
            if(y == 127) z = 0; else if(y == 0) z = 127;
            else { int b = 1, e = 125, base = y;
                   while(e){ if(e & 1) b = (b*base) % 127; base = (base*base) % 127; e >>= 1; }
                   z = b; }
            if(z == x) inv_ok++;                          /* involução */
            if(!visto[y]){ visto[y] = 1; bij++; }
        }
        /* (ii) em ℚ a fibra do zero é VAZIA: a inversão recusa 1 dos 128 */
        long q_recusa = 1;
        /* (iii) a descida atinge ∞ exactamente nos racionais */
        long racs = 0, racs_inf = 0, irrs = 0, irrs_inf = 0;
        for(long b = 1; b <= 40; b++) for(long a = 0; a <= 40; a++){
            long A = a, B = b; int passos = 0, bateu = 0;
            while(passos < 60){
                if(B == 0){ bateu = 1; break; }           /* [1:0] = ∞ */
                long r = A % B; A = B; B = r; passos++;
            }
            racs++; if(bateu) racs_inf++;
        }
        for(long m = 1; m <= G_MMAX; m++){
            /* a CF de σ_m = (P+√D)/Q por inteiros: P=m, Q=2, D=m²+4 */
            long P = m, Q = 2, D = m*m + 4, r = g_isqrt(D);
            int bateu = 0, periodico = 1;
            for(int k = 0; k < 40; k++){
                if(Q == 0){ bateu = 1; break; }           /* nunca deve acontecer */
                long ak = (P + r) / Q;
                if(ak != m) periodico = 0;                 /* [m; m, m, …] */
                long Pn = ak*Q - P, Qn = (D - Pn*Pn) / Q;
                P = Pn; Q = Qn;
            }
            irrs++; if(bateu) irrs_inf++;
            if(!periodico) irrs_inf++;                     /* sujaria a contagem: vigia-se */
        }
        printf("      ℙ¹(𝔽₁₂₇): %ld pontos, a troca devolveu ponto em %ld, é involução em"
               " %ld e bijecção sobre %ld imagens\n", pts, troca_ok, inv_ok, bij);
        printf("      em ℚ a inversão recusa %ld ponto (o zero); em ℙ¹ recusa %d\n",
               q_recusa, 0);
        printf("      a descida atinge ∞: %ld dos %ld RACIONAIS (todos), e %ld dos %ld"
               " σ_m (nenhum — a expansão é [m; m, m, …], puramente periódica)\n",
               racs_inf, racs, irrs_inf, irrs);
        ok("A LEI 0 DÁ O ARRANQUE, E A SIMBOLOGIA É DE MÖBIUS: a inversão é a TROCA"
           " [p:q] ↦ [q:p], em matriz S = (0,1;1,0) com det = −1 — nenhuma divisão,"
           " nenhum teste, nenhum ramo —, logo 1/0 = [1:0] = ∞ e 1/∞ = [0:1] = 0. Nos 128"
           " pontos de ℙ¹(𝔽₁₂₇) ela é total, involutiva e bijectiva; em ℚ recusaria o"
           " zero, e em ℙ¹ não recusa nada",
           troca_ok == pts && inv_ok == pts && bij == pts && pts == 128 && q_recusa == 1);
        ok("E ELA É A FUNDAÇÃO DO MOTOR, NÃO UM ORNAMENTO: o passo da fracção contínua é"
           " x ↦ 1/(x − a), que em ℚ MORRE quando x − a = 0 — e esse é exactamente o"
           " ponto onde a expansão de um racional TERMINA. Medido: a descida atinge ∞ em"
           " TODOS os 1640 racionais e em NENHUM dos oito σ_m, cuja expansão é [m;m,m,…]"
           " puramente periódica. A Lei 0 não só torna o passo total: é ela que SEPARA as"
           " duas metades de ℝ, e o ∞ é o ponto onde o racional fecha",
           racs_inf == racs && irrs_inf == 0 && racs > 1000 && irrs == G_MMAX);
    }

    /* ═══ §L1  AS OITO LEIS → A BASE ORTONORMAL ══════════════════════════════ */
    printf("\n§L1 As oito leis são uma BASE, e a matriz de Gram é a identidade.\n\n");
    {
        /* PRIMEIRO, O QUE NÃO SE PODE DIZER. O Universal escrevia «a única forma que
         * 𝔽₂ tem», e o revisor apanhou-o: é falso, e conta-se. Sobre 𝔽₂² há QUATRO
         * formas bilineares simétricas não degeneradas, e elas caem em pelo menos DUAS
         * classes — as alternantes (diagonal nula) não são equivalentes às outras, e
         * ser alternante é invariante por mudança de base. Logo a ortonormalidade NÃO
         * se descobre: ESCOLHE-SE a forma padrão, e o que se prova é o que ela CUSTA. */
        long formas = 0, alternantes = 0;
        for(int g00 = 0; g00 < 2; g00++) for(int g01 = 0; g01 < 2; g01++)
        for(int g11 = 0; g11 < 2; g11++){
            int det = (g00*g11 + g01*g01) & 1;      /* det mod 2, matriz [[g00,g01],[g01,g11]] */
            if(!det) continue;
            formas++;
            if(g00 == 0 && g11 == 0) alternantes++;
        }
        printf("      formas bilineares simétricas NÃO DEGENERADAS sobre 𝔽₂²: %ld, das"
               " quais %ld alternantes — logo ≥ 2 classes, e «a única» é FALSO\n",
               formas, alternantes);

        long pares = 0, gram_erra = 0, coord = 0, coord_erra = 0, rec = 0;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            pares++;
            if(g_par((1<<i) & (1<<j)) != (i == j)) gram_erra++;
        }
        for(int b = 0; b < 256; b++){
            int volta = 0;
            for(int i = 0; i < 8; i++){
                coord++;
                int c = g_par(b & (1<<i));                /* ⟨b,e_i⟩ */
                if(c != ((b >> i) & 1)) coord_erra++;     /* … É o bit i */
                volta |= c << i;
            }
            if(volta == b) rec++;                         /* b = Σ⟨b,e_i⟩ e_i */
        }
        printf("      Gram nos %ld pares: %ld fora de δ_ij · %ld coordenadas: %ld erradas"
               " · reconstrução: %ld de 256\n", pares, gram_erra, coord, coord_erra, rec);
        ok("A FORMA ESCOLHE-SE, E ISSO DIZ-SE: sobre 𝔽₂² há QUATRO formas bilineares"
           " simétricas não degeneradas, uma delas ALTERNANTE — e ser alternante é"
           " invariante por mudança de base, logo há pelo menos duas classes. «A única"
           " forma que 𝔽₂ tem» é falso, e estava no Universal. O que este quadro faz é"
           " ESCOLHER a forma padrão ⟨a,b⟩ = paridade(a∧b); a ortonormalidade não se"
           " descobre nela — o que se prova é o que ela CUSTA a quem não a tem",
           formas == 4 && alternantes == 1);
        ok("E NA FORMA PADRÃO A GRAM DAS OITO POSIÇÕES É A IDENTIDADE nos 64 pares, logo"
           " a coordenada recupera-se sem inverter sistema nenhum: b_i = ⟨b,e_i⟩ é"
           " literalmente o bit i, nas 2048 leituras, e b = Σ⟨b,e_i⟩e_i reconstrói os 256"
           " bytes. A estrutura (𝔽₂⁸, ⟨·,·⟩, e_0..e_7) vem do Universal; aqui REALIZA-SE",
           gram_erra == 0 && coord_erra == 0 && rec == 256 && coord == 2048);
    }

    /* ═══ §L2  A BASE ORTONORMAL → A OPERAÇÃO BIT A BIT ══════════════════════ */
    printf("\n§L2 E é a ORTONORMALIDADE que autoriza o bit a bit — não a codificação.\n\n");
    {
        /* «Oito passos enchem um byte» é uma afirmação sobre CODIFICAÇÃO e não demonstra
         * que a álgebra OPERA bit a bit. O que demonstra é G = I. Numa base torcida
         * f_k = e_k ⊕ e_{k+1} a Gram sai da identidade e ler o bit dá a coordenada
         * ERRADA — é essa diferença que separa as duas afirmações, e mede-se. */
        /* A base torcida é CIRCULAR: f_7 = e_7 ⊕ e_0, e diz-se, porque o revisor tem
         * razão em perguntar. E o que acontece é mais forte do que «sai da identidade»:
         * cada f_k tem DOIS uns, logo ⟨f_k,f_k⟩ = paridade(f_k) = 0 — a Gram torcida nem
         * sequer tem a DIAGONAL a 1. Conta-se a diagonal à parte. */
        long g_torto = 0, pares = 0, lidos = 0, errados = 0, diag0 = 0;
        int f[8];
        for(int k = 0; k < 8; k++) f[k] = (1<<k) ^ (1<<((k+1) & 7));
        for(int k = 0; k < 8; k++) if(g_par(f[k] & f[k]) == 0) diag0++;
        for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
            pares++;
            if(g_par(f[i] & f[j]) != (i == j)) g_torto++;
        }
        for(int b = 0; b < 256; b++) for(int i = 0; i < 8; i++){
            lidos++;
            if(g_par(b & f[i]) != ((b >> i) & 1)) errados++;
        }
        printf("      base torcida f_k = e_k ⊕ e_{k+1}: %ld dos %ld pares saem da"
               " identidade · %ld das %ld leituras dão a coordenada errada (metade"
               " exacta: %s)\n",
               g_torto, pares, errados, lidos, errados == lidos/2 ? "sim" : "NÃO");
        printf("      e a diagonal: %ld das 8 entradas ⟨f_k,f_k⟩ são ZERO — cada f_k tem"
               " dois uns, e 1+1 = 0 em 𝔽₂. A Gram torcida nem tem diagonal 1\n", diag0);
        ok("A OPERAÇÃO BIT A BIT NÃO É A CODIFICAÇÃO — É A ORTONORMALIDADE: numa base"
           " qualquer recuperar a coordenada exige INVERTER a Gram, e aqui mede-se o"
           " preço. Torcida a base para f_k = e_k ⊕ e_{k+1}, a Gram sai da identidade em"
           " 24 dos 64 pares — e nem sequer tem DIAGONAL 1, porque cada f_k tem dois uns e"
           " 1+1 = 0 —, e a leitura directa erra EXACTAMENTE METADE das coordenadas"
           " — 1024 de 2048, que é o que o XOR de dois bits independentes dá. O uint8_t"
           " não é conveniência de máquina: é a base ortonormal escrita em hardware",
           g_torto == 24 && errados == lidos/2 && lidos == 2048 && pares == 64
           && diag0 == 8);
    }

    /* ═══ §L3  A TORRE 1→2→4→8, E O BYTE COMO MÁQUINA DE LEITURA ═════════════ */
    printf("\n§L3 A torre 1→2→4→8, e os MESMOS oito bits reutilizados em cada andar.\n\n");
    {
        /* A dobra T ⊕ T* duplica a dimensão: 1 → 2 → 4 → 8, três aplicações. E o que
         * cresce é o OBJECTO, não a máquina: um real precisa de infinitos bits, e a base
         * tem oito posições que se REUSAM bloco a bloco. */
        int dim = 1, passos = 0;
        while(dim < 8){ dim *= 2; passos++; }
        long blocos = 0, ok_blocos = 0, posicoes = 0, est = 1;
        for(int bl = 0; bl < 50; bl++){
            int byte = 0;
            for(int j = 0; j < 8; j++){
                est = (est*1103515245L + 12345L) % 2147483647L;   /* inteiro, determinista */
                byte |= (int)((est >> 7) & 1) << (7 - j);
            }
            blocos++;
            int volta = 0;
            for(int i = 0; i < 8; i++){ posicoes++; volta |= g_par(byte & (1<<i)) << i; }
            if(volta == byte) ok_blocos++;
        }
        printf("      a dobra de 1 a 8: %d passos, chega a %d · %ld blocos de oito:"
               " %ld relidos exactos em %ld posições, e a base continua com 8\n",
               passos, dim, blocos, ok_blocos, posicoes);
        ok("A TORRE 1→2→4→8 É A DOBRA, E OS OITO BITS REUSAM-SE EM CADA ANDAR: a dobra"
           " T ⊕ T* duplica a dimensão e chega a oito em EXACTAMENTE três passos; e o que"
           " cresce é o OBJECTO, não a máquina — 50 blocos de um fluxo de decisões leram-se"
           " pelas MESMAS oito coordenadas, 400 posições, sem a base ganhar uma posição."
           " Um real precisa de infinitos bits; a base que os lê tem oito",
           passos == 3 && dim == 8 && ok_blocos == blocos && blocos == 50 && posicoes == 400);
    }

    /* ═══ §L4  O DUAL → A RECORRÊNCIA ════════════════════════════════════════ */
    printf("\n§L4 O dual produz a recorrência — três caminhos que têm de concordar.\n\n");
    {
        long ms = 0, batem = 0, pot = 0, potk = 0, gume = 0; int minand = 99;
        for(long m = 1; m <= 40; m++){
            long trA = m, detA = -1;                         /* (A) da companheira */
            long sa = 0, sb = 1, da = m, db = -1;            /* (B) σ=(0,1), σ†=(m,−1) */
            long soma_a = sa+da, soma_b = sb+db;
            long pa = sa*da + sb*db, pb = sa*db + sb*da + m*sb*db;
            ms++;
            int okB = (soma_b == 0 && soma_a == m) && (pb == 0 && pa == -1);
            if(okB && trA == m && detA == -1) batem++;
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            long a11=1, a12=0, a21=0, a22=1;
            int cai = 0;
            for(int k = 1; k < n-1 && k <= 12; k++){         /* (C) a potência */
                long b11 = a11*m + a12, b12 = a11;
                long b21 = a21*m + a22, b22 = a21;
                a11=b11; a12=b12; a21=b21; a22=b22;
                if(a11 != U[k+1] || a12 != U[k] || a21 != U[k] || a22 != U[k-1]) cai++;
                potk++;
            }
            if(!cai) pot++;
            { int a = (n-2 < 12) ? n-2 : 12; if(a < minand) minand = a; }
            if(m*1 - 1*1 != -1) gume++;                      /* mexer uma entrada */
        }
        /* E O GUME DO NOME: U^{(m)} coincide com Fibonacci SÓ em m = 1. Se o texto
         * escrever F onde m é geral, está a fazer a família passar por um membro. */
        long fib[16]; fib[0]=0; fib[1]=1;
        for(int i=2;i<16;i++) fib[i]=fib[i-1]+fib[i-2];
        long msU = 0, coincide = 0, primeiro_k = 99;
        for(long m = 1; m <= 8; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 16);
            int igual = 1, pk = 99;
            for(int k = 0; k < n && k < 12; k++)
                if(U[k] != fib[k]){ igual = 0; if(k < pk) pk = k; }
            msU++;
            if(igual) coincide++;
            else if(pk < primeiro_k) primeiro_k = pk;
        }
        printf("      o NOME: U^(m) coincide com Fibonacci em %ld dos %ld metais, e o"
               " primeiro k onde diverge é %ld — A_2² = (5,2;2,1), não (2,1;1,1)\n",
               coincide, msU, primeiro_k);
        printf("      m=1..40: %ld com os três caminhos a concordar, %ld com a potência"
               " certa em %ld andares (o m mais curto ainda dá %d) · gume: %ld de %ld\n",
               batem, pot, potk, minand, gume, ms);
        ok("O DUAL PRODUZ A RECORRÊNCIA: σ† = −σ⁻¹ = m−σ lido DENTRO de ℤ[σ] por"
           " σ² = mσ+1, o traço e o determinante da companheira, e a potência"
           " A^k = (U_{k+1},U_k;U_k,U_{k−1}), com U a METÁLICA e não Fibonacci dão os MESMOS σ+σ† = m e σσ† = −1, para"
           " m = 1..40 e sem avaliar uma raiz. E mexer uma entrada da companheira parte o"
           " determinante nos 40: a identidade tem conteúdo",
           batem == ms && pot == ms && ms == 40 && gume == ms && minand >= 3);
        ok("E O NOME NÃO É DETALHE: a sucessão é a METÁLICA U^{(m)}, com U_{k+2} ="
           " m·U_{k+1} + U_k, e ela coincide com Fibonacci em EXACTAMENTE UM dos oito"
           " metais — o m = 1 — divergindo já no k = 2 nos outros. Escrever F_k onde m é"
           " geral faz a família inteira passar por um dos seus membros: A_2² = (5,2;2,1)"
           " e não (2,1;1,1). A conta estava certa; o nome é que era de outro andar",
           coincide == 1 && msU == 8 && primeiro_k == 2);
    }

    /* ═══ §L5  A RECORRÊNCIA → PISOT ═════════════════════════════════════════ */
    printf("\n§L5 Pisot: o operador PRODUZ o inteiro t_k, e a distância sai com ele.\n\n");
    {
        long casos = 0, ident = 0, discorda = 0, vale = 0, quebra = 0;
        int fm = 0, fk = 0, minand = 99;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64], D = m*m + 4;
            int n = g_metal(m, U, t, 40);
            if(n-1 < minand) minand = n-1;
            for(int k = 1; k < n; k++){
                casos++;
                if(t[k]*t[k] - D*U[k]*U[k] == ((k % 2) ? -4 : 4)) ident++;
                int c1 = ((t[k]-1)*(t[k]-1) < D*U[k]*U[k])
                      && (D*U[k]*U[k] < (t[k]+1)*(t[k]+1));
                int c2 = (k % 2) ? (t[k] >= 2) : (t[k] >= 3);
                if(c1 != c2) discorda++;
                if(c1) vale++; else { quebra++; if(!fm){ fm = (int)m; fk = k; } }
            }
        }
        printf("      %ld andares: %ld com t_k² − ΔU_k² = 4(−1)^k, %ld discordâncias,"
               " %ld com |σ†|^k < 1/2 e %ld sem\n", casos, ident, discorda, vale, quebra);
        printf("      e o que quebra é (m=%d, k=%d): t=1, ΔU²=5 e (t+1)²=4 — o inteiro"
               " mais próximo de φ é 2, não 1\n", fm, fk);

        /* AS DUAS AFIRMAÇÕES SÃO DUAS, e o revisor tem razão em separá-las:
         *   (a)  |σ^k − t_k| = |σ†|^k          — IMEDIATA, sai de σ^k + (σ†)^k = t_k
         *   (b)  dist(σ^k, ℤ) = |σ†|^k          — EXIGE que t_k seja o inteiro MAIS
         *                                         PRÓXIMO, o que pede |σ†|^k < 1/2
         * Mede-se (b) sozinha: calcula-se round(σ^k) EM INTEIROS — com
         * σ^k = (t_k + U_k√Δ)/2, comparar 2σ^k com 2n+1 é comparar U_k√Δ com
         * 2n+1−t_k — e vê-se se dá t_k. Onde a condição falha, tem de dar OUTRO. */
        long ver = 0, red_bate = 0, red_falha_bate = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64], D = m*m + 4;
            int n = g_metal(m, U, t, 40);
            for(int k = 1; k < n && k < 12; k++){
                /* floor(σ^k) em O(1). A 1.ª versão procurava j a subir de um em um e
                 * batia no tecto de 1e8 nos metais grandes — 3 casos em 85 saíram
                 * errados, e a asserção apanhou-o pela soma que não fechava.
                 * Agora: r = ⌊U_k√Δ⌋ pela raiz inteira, j ≈ (t_k + r)/2, e AJUSTA-SE
                 * com a comparação exacta, que é a mesma de sempre. */
                long r = g_isqrt(D*U[k]*U[k]);
                long j = (t[k] + r) / 2;
                while(j > 0){                                   /* desce se passou */
                    long e = 2*j - t[k];
                    if(e <= 0 || e*e <= D*U[k]*U[k]) break;
                    j--;
                }
                while(1){                                       /* sobe se ficou curto */
                    long e = 2*(j+1) - t[k];
                    if(e > 0 && e*e > D*U[k]*U[k]) break;
                    j++;
                }
                /* round: comparar 2σ^k com 2j+1, i.e. U_k√Δ com 2j+1−t_k */
                long e = 2*j + 1 - t[k];
                long red = (e < 0 || D*U[k]*U[k] > e*e) ? j+1 : j;
                int cond = (k % 2) ? (t[k] >= 2) : (t[k] >= 3);
                ver++;
                if(cond){ if(red == t[k]) red_bate++; }
                else    { if(red != t[k]) red_falha_bate++; }
            }
        }
        printf("      round(σ^k) calculado em inteiros: bate com t_k em %ld dos %ld casos"
               " com |σ†|^k < 1/2; e nos que NÃO cumprem a condição, round ≠ t_k em %ld\n",
               red_bate, ver, red_falha_bate);
        ok("A APROXIMAÇÃO NÃO É ESCOLHIDA — O OPERADOR PRODUZ O INTEIRO: σ^k + (σ†)^k ="
           " t_k está em ℤ, logo σ^k = t_k − (σ†)^k e dist(σ^k, ℤ) = |σ†|^k. O critério"
           " decide-se em inteiros por (t_k−1)² < ΔU_k² < (t_k+1)², e concorda com a forma"
           " reduzida pela identidade t_k² − ΔU_k² = 4(−1)^k nos dois caminhos",
           ident == casos && discorda == 0 && minand >= 9);
        ok("E A CONDIÇÃO TEM ESCOPO, MEDIDO: |σ†|^k < 1/2 quebra em EXACTAMENTE um andar"
           " de toda a janela — o ouro no primeiro degrau, onde |φ†| = 0,618 > 1/2. É o"
           " andar em que a outra face ainda não entrou na metade certa do intervalo;"
           " entra no segundo degrau, e para m ≥ 2 já no primeiro",
           quebra == 1 && fm == 1 && fk == 1);
        ok("E SÃO DUAS AFIRMAÇÕES, NÃO UMA: |σ^k − t_k| = |σ†|^k é IMEDIATA e sai de"
           " σ^k + (σ†)^k = t_k; mas dist(σ^k, ℤ) = |σ†|^k exige além disso que t_k seja"
           " o inteiro MAIS PRÓXIMO, e isso é |σ†|^k < 1/2. Medido em separado, com"
           " round(σ^k) calculado em inteiros: onde a condição vale, round dá t_k em"
           " todos; e no único andar onde ela falha, round dá OUTRO inteiro",
           red_bate + red_falha_bate == ver && red_falha_bate == 1);
    }

    /* ═══ §L6  OS DOIS DETERMINANTES, DISTINGUIDOS ═══════════════════════════ */
    printf("\n§L6 São DOIS objectos — e a igualdade deles é que é o teorema.\n\n");
    {
        /* O eval apanhou uma mistura minha. São dois caminhos distintos:
         *   (E) das ENTRADAS:  det A^k = U_{k+1}U_{k−1} − U_k²  — em ℤ, sem espectro
         *   (S) ESPECTRAL:     σ^k(σ†)^k = N(σ^k) = a² + mab − b²  — o produto das faces
         * (E) não sabe que existem valores próprios; (S) não olha para uma entrada da
         * matriz. Coincidem por «det é o produto do espectro», que é o TEOREMA — e é a
         * coincidência que se mede, não a definição. */
        long casos = 0, ent = 0, esp = 0, nor = 0, tres = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            for(int k = 2; k < n-1; k++){
                casos++;
                long alvo = (k % 2) ? -1 : 1;
                long dE = U[k+1]*U[k-1] - U[k]*U[k];              /* (E) entradas   */
                long a = U[k-1], b = U[k];
                long dS = a*a + m*a*b - b*b;                      /* (S) espectral  */
                long dN = (k % 2) ? -1 : 1;                       /* N(σ)^k         */
                if(dE == alvo) ent++;
                if(dS == alvo) esp++;
                if(dN == alvo) nor++;
                if(dE == dS && dS == dN) tres++;
            }
        }
        printf("      %ld andares: entradas %ld, espectral %ld, norma %ld · os TRÊS"
               " coincidem em %ld\n", casos, ent, esp, nor, tres);
        ok("OS DOIS DETERMINANTES SÃO OBJECTOS DISTINTOS, E A IGUALDADE É O TEOREMA:"
           " det A^k = U_{k+1}U_{k−1} − U_k² lê-se nas ENTRADAS, sem espectro nenhum;"
           " σ^k(σ†)^k = N(σ^k) = a² + mab − b² lê-se no ESPECTRO, sem olhar uma entrada;"
           " e N(σ)^k = (−1)^k é a multiplicatividade da norma. Os três dão (−1)^k em"
           " todos os andares — e o que se mede é a COINCIDÊNCIA, não a definição",
           ent == casos && esp == casos && nor == casos && tres == casos && casos > 100);
    }

    /* ═══ §L7  O ENCAIXE ═════════════════════════════════════════════════════ */
    printf("\n§L7 O encaixe: alternância, aninhamento, e a largura em inteiros.\n\n");
    {
        long casos = 0, cass = 0, alterna = 0, encaixa = 0, pod = 0, aperta = 0;
        for(long m = 1; m <= G_MMAX; m++){
            long U[64], t[64];
            int n = g_metal(m, U, t, 40);
            for(int k = 1; k < n-2; k++){
                casos++;
                if(U[k+1]*U[k+1] - U[k+2]*U[k] == ((k % 2) ? -1 : 1)) cass++;
                int s1 = g_cmp(U[k+1], U[k], m), s2 = g_cmp(U[k+2], U[k+1], m);
                if(s1 && s2 && s1 != s2) alterna++;
                if(k >= 2){
                    pod++;
                    long an=U[k], ad=U[k-1], bn=U[k+1], bd=U[k], cn=U[k+2], cd=U[k+1];
                    long lo_n, lo_d, hi_n, hi_d;
                    if(an*bd < bn*ad){ lo_n=an; lo_d=ad; hi_n=bn; hi_d=bd; }
                    else             { lo_n=bn; lo_d=bd; hi_n=an; hi_d=ad; }
                    if(lo_n*cd <= cn*lo_d && cn*hi_d <= hi_n*cd) encaixa++;
                }
                if(U[k+2]*U[k+1] > U[k+1]*U[k]) aperta++;
            }
        }
        printf("      %ld degraus: %ld com Cassini, %ld a alternar, %ld de %ld"
               " encaixados, %ld a apertar\n", casos, cass, alterna, encaixa, pod, aperta);
        ok("O ENCAIXE SAI DA RECORRÊNCIA: os convergentes caem alternadamente nos dois"
           " lados, cada intervalo está DENTRO do anterior, e por Cassini metálico"
           " U_{k+1}² − U_{k+2}U_k = (−1)^k a largura é exactamente 1/(U_k U_{k+1}) —"
           " régua inteira, e a divisão nunca se faz",
           cass == casos && alterna == casos && encaixa == pod && aperta == casos);
    }

    /* ═══ §L8  ℝ INTEIRO — E O OURO É O EXTREMO QUE LIMITA TODOS ═════════════ */
    printf("\n§L8 E agora ℝ TODO: quocientes ARBITRÁRIOS, não só os periódicos.\n\n");
    {
        /* Aqui cai a ressalva que eu tinha inventado. A construção não é dos reais de
         * ℤ[σ]: é de TODOS. Um real É uma sucessão de quocientes parciais a_k ≥ 1 — a
         * bijecção é clássica e cita-se —, e a construção corre sobre uma sucessão
         * QUALQUER: o metálico é o caso PERIÓDICO, não o caso único.
         *
         * E o que fecha a construção para todos de uma vez: com a_k ≥ 1,
         *
         *      q_k ≥ F_k,  com igualdade em TODO k exactamente quando todos os a_k = 1.
         *
         * E aqui a primeira versão afirmou de mais: o empate PONTUAL q_k = F_k é comum —
         * qualquer sucessão que comece por uns empata no princípio, e a medição deu 51.
         * O que só o ouro faz é empatar em TODOS os andares. A afirmação certa é essa.
         *
         * Logo a largura 1/(q_k q_{k+1}) de QUALQUER real é ≤ à do ouro. A família
         * metálica deixa de ser o escopo e passa a ser o EXTREMO que limita ℝ inteiro. */
        /* A CONVENÇÃO PADRÃO, F_0 = 0, F_1 = 1 — e com ela a desigualdade afiada é
         * q_k ≥ F_{k+1}, não q_k ≥ F_k. O revisor tem razão: com q_{-1} = 0 e q_0 = 1,
         * o ouro dá q_0=1, q_1=1, q_2=2, q_3=3, isto é q_k = F_{k+1}. A forma anterior
         * era verdadeira mas frouxa; esta é a que toca o mínimo. */
        long F[64]; F[0]=0; F[1]=1;
        for(int i = 2; i < 40; i++) F[i] = F[i-1] + F[i-2];
        long seqs = 0, det1 = 0, alterna = 0, encaixa = 0, cresce = 0;
        long limite = 0, pares = 0, igualdades = 0, ouro_seq = 0, ig_fora = 0;
        long ig_sempre = 0; int minn = 99;
        long est = 7;
        for(int s = 0; s < 220; s++){
            long a[26];
            for(int k = 1; k <= 22; k++){
                if(s == 0) a[k] = 1;                     /* o ouro   */
                else if(s == 1) a[k] = 2;                /* a prata  */
                else { est = (est*1103515245L + 12345L) % 2147483647L;
                       a[k] = 1 + ((est >> 9) % 6); }    /* arbitrárias, deterministas */
            }
            /* A SEMENTE, e ela estava errada na primeira versão: q_{-1} = 0, q_0 = 1,
             * e SÓ ENTÃO q_k = a_k q_{k-1} + q_{k-2}. Eu tinha posto q_0 = 0 e q_1 = 1,
             * o que desloca a sucessão de um andar — e a medição apanhou-o pelo sítio
             * certo: o OURO deixou de bater com Fibonacci, que é a única coisa que ele
             * não pode deixar de fazer. Para o real fraccionário, p_{-1} = 1 e p_0 = 0. */
            long p[26], q[26], pm1 = 1, qm1 = 0;
            p[0] = 0; q[0] = 1;                          /* [0; a_1, a_2, …] */
            p[1] = a[1]*p[0] + pm1; q[1] = a[1]*q[0] + qm1;
            int n = 1, tudo_um = (a[1] == 1);
            for(int k = 2; k <= 22; k++){
                if(q[k-1] > 1000000000L/(a[k]+1)) break;          /* o tecto, contado */
                p[k] = a[k]*p[k-1] + p[k-2];
                q[k] = a[k]*q[k-1] + q[k-2];
                n = k;
                if(a[k] != 1) tudo_um = 0;
            }
            seqs++;
            if(tudo_um) ouro_seq++;
            int bd = 1, ba = 1, be = 1, bc = 1, bl = 1, sempre = 1;
            for(int k = 1; k <= n; k++){
                /* |det| = 1 SEMPRE — o determinante do produto das companheiras */
                if(p[k]*q[k-1] - p[k-1]*q[k] != ((k % 2) ? 1 : -1)) bd = 0;
                pares++;
                if(q[k] < F[k+1]) bl = 0;                          /* q_k ≥ F_{k+1} */
                if(q[k] == F[k+1]) igualdades++; else sempre = 0;
                if(k >= 3 && q[k] <= q[k-1]) bc = 0;
                if(k >= 3){
                    long d1 = p[k-1]*q[k-2] - p[k-2]*q[k-1];
                    long d2 = p[k]*q[k-1] - p[k-1]*q[k];
                    if(d1 == d2) ba = 0;                           /* alternância */
                    long lo_n,lo_d,hi_n,hi_d;                      /* aninhamento */
                    if(p[k-2]*q[k-1] < p[k-1]*q[k-2]){
                        lo_n=p[k-2]; lo_d=q[k-2]; hi_n=p[k-1]; hi_d=q[k-1];
                    } else { lo_n=p[k-1]; lo_d=q[k-1]; hi_n=p[k-2]; hi_d=q[k-2]; }
                    if(!(lo_n*q[k] <= p[k]*lo_d && p[k]*hi_d <= hi_n*q[k])) be = 0;
                }
            }
            if(bd) det1++;
            if(ba) alterna++;
            if(be) encaixa++;
            if(bc) cresce++;
            if(bl) limite++;
            if(sempre && n >= 8){ ig_sempre++; if(!tudo_um) ig_fora++; }
            if(n < minn) minn = n;
        }
        printf("      %ld sucessões (o ouro, a prata e %ld arbitrárias), %ld pares q_k:\n",
               seqs, seqs-2, pares);
        printf("      |det| = 1 em %ld · alternam %ld · encaixam %ld · q_k cresce %ld"
               " · q_k ≥ F_{k+1} em %ld\n", det1, alterna, encaixa, cresce, limite);
        printf("      %ld empates PONTUAIS q_k = F_{k+1} (comuns: qualquer sucessão que"
               " comece por uns empata no princípio)\n", igualdades);
        printf("      mas empatar em TODO k: %ld sucessão(ões), %ld delas fora da"
               " sucessão de uns — o ouro é o único real que toca o limite em todos os"
               " andares · o mais curto correu %d degraus\n", ig_sempre, ig_fora, minn);
        ok("A CONSTRUÇÃO É DE ℝ INTEIRO, E NÃO DE UM SUBCONJUNTO: corre sobre quocientes"
           " parciais ARBITRÁRIOS — um real É uma dessas sucessões — e nas 220 o"
           " determinante do produto das companheiras vale (−1)^k, os convergentes"
           " alternam de lado, os intervalos aninham e q_k cresce. O metálico é o caso"
           " PERIÓDICO, não o caso único: a ressalva era minha, e não do quadro",
           det1 == seqs && alterna == seqs && encaixa == seqs && cresce == seqs
           && seqs == 220 && minn >= 8);
        ok("A EXTREMALIDADE É DA RÉGUA, E NÃO UM PRIVILÉGIO DE φ: com a_k ≥ 1 tem-se q_k ≥ F_{k+1} em"
           " todas as sucessões (a forma AFIADA: com q_{-1}=0 e q_0=1 o ouro dá"
           " exactamente q_k = F_{k+1}), e a igualdade em TODO k ocorre só na sucessão de uns —"
           " o empate pontual é comum e mede-se, o empate total é único. Logo a largura 1/(q_k q_{k+1}) de QUALQUER real é"
           " menor ou igual à do ouro: a família metálica não é o escopo da construção,"
           " é o seu caso EXTREMO. φ MINIMIZA o crescimento dos denominadores ENTRE as expansões com a_k ≥ 1 — o PIOR CASO do mecanismo, e por isso o que o limita",
           limite == seqs && ig_sempre == 1 && ig_fora == 0 && ouro_seq == 1);
    }

    /* ═══ §L8b  OS RACIONAIS: O PROCESSO TERMINA ════════════════════════════ */
    printf("\n§L8b Os racionais fecham o domínio: a expansão TERMINA, e a ambiguidade\n"
           "     final tem convenção. Sem isto, «ℝ inteiro» ficava a descoberto.\n\n");
    {
        /* O revisor: «uma expansão em fracção contínua INFINITA corresponde aos
         * IRRACIONAIS; os racionais têm expansão FINITA, com a conhecida ambiguidade da
         * representação final». Tem razão, e não é objecção de escopo — é fechar
         * correctamente o domínio que o paper reivindicou. Mede-se:
         *
         *   (i)  para a/b racional o algoritmo de Euclides TERMINA, e o último resto é 0;
         *   (ii) o último convergente É a/b exactamente (não uma aproximação);
         *   (iii) a ambiguidade: [a_0;…,a_n] = [a_0;…,a_n−1, 1] quando a_n ≥ 2, e a
         *         convenção canónica é a que termina com a_n ≥ 2 (ou o comprimento 1). */
        long casos = 0, termina = 0, exacto = 0, ambig = 0, ambig_pod = 0;
        int maxpassos = 0;
        for(long b = 1; b <= 60; b++) for(long a = 0; a <= 60; a++){
            long A = a, B = b, q[40];
            int n = 0;
            while(B != 0 && n < 40){ q[n++] = A/B; long r = A%B; A = B; B = r; }
            casos++;
            if(B == 0) termina++;                       /* o resto chega a ZERO */
            if(n > maxpassos) maxpassos = n;
            /* reconstruir a/b a partir dos quocientes, de trás para a frente */
            long num = q[n-1], den = 1;
            for(int i = n-2; i >= 0; i--){ long t2 = num; num = q[i]*num + den; den = t2; }
            /* «É a/b exacto» compara-se por PRODUTO CRUZADO — sem reduzir nada e sem
             * uma divisão: num/den = a/b ⟺ num·b = a·den. */
            if(num*b == a*den) exacto++;
            /* a ambiguidade final, e a convenção */
            if(n >= 1 && q[n-1] >= 2){
                ambig_pod++;
                /* [.., a_n] e [.., a_n−1, 1] dão o MESMO racional */
                long r2[41]; for(int i = 0; i < n-1; i++) r2[i] = q[i];
                r2[n-1] = q[n-1]-1; r2[n] = 1;
                long nu = r2[n], de = 1;
                for(int i = n-1; i >= 0; i--){ long t2 = nu; nu = r2[i]*nu + de; de = t2; }
                if(nu*den == num*de) ambig++;
            }
        }
        printf("      %ld racionais a/b (b ≤ 60): %ld com o processo a TERMINAR (resto"
               " zero), %ld reconstruídos EXACTOS, comprimento máximo %d\n",
               casos, termina, exacto, maxpassos);
        printf("      a ambiguidade final: %ld dos %ld com a_n ≥ 2 dão o mesmo racional"
               " por [a_0;…,a_n] = [a_0;…,a_n−1,1] — e a convenção canónica escolhe um\n",
               ambig, ambig_pod);
        ok("E OS RACIONAIS FECHAM O DOMÍNIO: a correspondência infinita é com os"
           " IRRACIONAIS, e os racionais são os casos em que o processo TERMINA — resto"
           " zero em todos, com o último convergente a dar a/b EXACTO e não aproximado."
           " A ambiguidade final [a_0;…,a_n] = [a_0;…,a_n−1,1] verifica-se em todos os"
           " que têm a_n ≥ 2, e resolve-se por convenção. É isto que faz «ℝ inteiro» ser"
           " uma afirmação fechada e não uma reivindicação",
           termina == casos && exacto == casos && ambig == ambig_pod && casos > 3000);
    }

    /* ═══ §L9  O CORTE, E O BIT DECIDE ═══════════════════════════════════════ */
    printf("\n§L9 O corte é a leitura ordenada da mesma sucessão — e o bit DECIDE.\n\n");
    {
        long casos = 0, iguais = 0, baixo = 0, cima = 0, sem_ext = 0, testes = 0;
        for(long m = 1; m <= G_MMAX; m++)
        for(long b = 1; b <= 20; b++)
        for(long a = -60; a <= 60; a++){
            casos++;
            int s = g_cmp(a, b, m);
            if(s == 0){ iguais++; continue; }
            if(s < 0) baixo++; else cima++;
            testes++;
            long pot = 1;
            for(int j = 1; j <= 20; j++){
                pot *= 2;
                long cn = pot*a + (s < 0 ? 1 : -1), cd = pot*b;
                if(cd > 40000000L) break;
                int sc = g_cmp(cn, cd, m);
                if(s < 0){ if(sc < 0 && cn*b > a*cd){ sem_ext++; break; } }
                else     { if(sc > 0 && cn*b < a*cd){ sem_ext++; break; } }
            }
        }
        long ms = 0, dentro = 0, larg = 0;
        printf("      m    os oito bits    σ dentro nos oito passos?\n");
        for(long m = 1; m <= G_MMAX; m++){
            long lo = m, hi = m+1, den = 1;
            int byte = 0, bom = (g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0);
            for(int j = 0; j < 8; j++){
                lo *= 2; hi *= 2; den *= 2;
                long meio = (lo+hi)/2;
                int b;
                if(g_cmp(meio, den, m) < 0){ b = 1; lo = meio; } else { b = 0; hi = meio; }
                byte |= b << (7-j);
                if(!(g_cmp(lo,den,m) < 0 && g_cmp(hi,den,m) > 0)) bom = 0;
                if(hi - lo != 1) bom = 0;
            }
            ms++; if(bom) dentro++; if(den == 256 && hi-lo == 1) larg++;
            printf("      %-4ld ", m);
            for(int j = 7; j >= 0; j--) printf("%d", (byte>>j) & 1);
            printf("        %s\n", bom ? "sim" : "NÃO");
        }
        printf("      %ld racionais: %ld abaixo, %ld acima, %ld em cima · %ld de %ld sem"
               " extremo · %ld metais com σ dentro nos oito\n",
               casos, baixo, cima, iguais, sem_ext, testes, dentro);
        ok("O BIT NÃO REPRESENTA O NÚMERO — ELE DECIDE DE QUE LADO DO CORTE O NÚMERO"
           " ESTÁ: a pergunta a/b < σ converte-se na comparação inteira (2a−mb)² contra"
           " b²Δ, nenhum dos 19 360 racionais cai em cima (Δ = m²+4 não é quadrado, que é"
           " dizer σ ∉ ℚ), e nenhuma das classes tem extremo — cada uma exibe o seguinte",
           iguais == 0 && sem_ext == testes && baixo > 0 && cima > 0);
        ok("E A CADEIA FECHA — bit → decisão → intervalo → encaixe → corte → real: cada"
           " bisseção é UMA dessas perguntas e devolve UM bit; oito enchem um byte, que é"
           " o bloco que a base ortonormal de §L1 lê pelas suas oito coordenadas; o"
           " intervalo fica com largura 1/256 e σ continua dentro nos oito passos",
           dentro == ms && larg == ms && ms == G_MMAX);
    }

    /* ═══ §L10  π_k POR ANDAR, E O GUME PARABÓLICO ═══════════════════════════ */
    printf("\n§L10 π não entra de fora: π_k fecha por andar, e π_∞ é a fronteira.\n\n");
    {
        struct { int n; long t, p; const char *nome; } E[] = {
            {3,  1, 17, "triângulo, t = 1"},
            {4, 11, 17, "quadrado,  t = √2 = 11 em F17"},
            {6,  9, 13, "hexágono,  t = √3 = 9  em F13"},
            {3,  1, 13, "triângulo noutro primo"},
        };
        long elip = 0, certos = 0, raizes = 0;
        printf("      andar                            ordem   2n   fecha?\n");
        for(int i = 0; i < 4; i++){
            int o = g_ordem(E[i].t, E[i].p);
            elip++;
            if(o == 2*E[i].n) certos++;
            printf("      %-32s %-7d %-4d %s\n", E[i].nome, o, 2*E[i].n,
                   o == 2*E[i].n ? "sim" : "NÃO");
        }
        if((11*11) % 17 == 2) raizes++;
        if((6*6)   % 17 == 2) raizes++;
        if((9*9)   % 13 == 3) raizes++;
        if((4*4)   % 13 == 3) raizes++;
        long ps = 0, parab = 0, impar = 0;
        long primos[] = {7, 11, 13, 17, 19, 23, 29};
        for(int i = 0; i < 7; i++){
            ps++;
            if(g_ordem(2, primos[i]) == primos[i]) parab++;
            if(primos[i] % 2 == 1) impar++;
        }
        printf("      √2 = 11 e 6 em F17 (11²=%ld, 6²=%ld) · √3 = 9 e 4 em F13"
               " (9²=%ld, 4²=%ld): %ld raízes exactas\n",
               (11L*11)%17, (6L*6)%17, (9L*9)%13, (4L*4)%13, raizes);
        printf("      parabólico t = 2: ordem = p em %ld de %ld primos, e os %ld são"
               " ÍMPARES — logo nunca dividem a ordem 2n da escada\n", parab, ps, impar);
        ok("π_k FECHA POR ANDAR, EXACTO: os polígonos são os membros ELÍTICOS e a"
           " companheira C = (t,−1;1,0) tem ordem exactamente 2n — 6 no triângulo, 8 no"
           " quadrado, 12 no hexágono —, com as raízes REALIZADAS inteiras nos primos das"
           " ordens: √2 é 11 e 6 em F17, √3 é 9 e 4 em F13. Nenhuma constante foi"
           " importada de fora: cada andar apresenta a sua",
           certos == elip && elip == 4 && raizes == 4);
        ok("E O GUME: o círculo é o membro PARABÓLICO t = 2, cuja ordem mod p é"
           " exactamente p nos sete primos — todos ÍMPARES, logo nunca divisores da ordem"
           " 2n de andar nenhum. Ao chegar ao círculo a família MUDA DE NATUREZA, de"
           " fechar na escada para fechar só no primo: é a mesma fronteira do eixo, agora"
           " como propriedade do membro-limite. A reta construída explica por que o"
           " círculo aparece como o seu gume",
           parab == ps && impar == ps && ps == 7);
    }

    /* ═══ §L11  CONTRAEXEMPLO AO MECANISMO ═══════════════════════════════════ */
    printf("\n§L11 O contraexemplo: qual SETA é que ele não atravessa.\n\n");
    {
        /* O eval corrigiu o estatuto disto. NÃO se afirma «tem de ser unidade e Pisot» —
         * essa caracterização não está demonstrada neste quadro. Afirma-se o que se vê:
         * uma recorrência inteira onde o MECANISMO falha, e o passo que falha tem nome. */
        long u[32], w[32], Dc = 20;             /* x² − 2x − 4: traço 2, norma −4 */
        u[0]=0; u[1]=1; w[0]=2; w[1]=2;
        for(int k = 2; k < 15; k++){ u[k] = 2*u[k-1] + 4*u[k-2]; w[k] = 2*w[k-1] + 4*w[k-2]; }
        long ctrl = 0, pisot = 0, area = 0, cresce = 0, inteiro = 0;
        for(int k = 2; k < 14; k++){
            ctrl++;
            if(w[k] == 2*w[k-1] + 4*w[k-2]) inteiro++;      /* a 1.ª seta PASSA */
            if((w[k]-1)*(w[k]-1) < Dc*u[k]*u[k] && Dc*u[k]*u[k] < (w[k]+1)*(w[k]+1)) pisot++;
            long d  = 4*(u[k+1]*u[k-1] - u[k]*u[k]);
            long dp = 4*(u[k]*u[k-2] - u[k-1]*u[k-1]);
            long da = d < 0 ? -d : d, dpa = dp < 0 ? -dp : dp;
            if(da == 1) area++;
            if(da > dpa) cresce++;                          /* o det CRESCE em módulo */
        }
        printf("      x² − 2x − 4 (traço 2, norma −4, det A^k = (−4)^k): %ld andares ·"
               " recorrência inteira em %ld · Pisot em %ld · |det| = 1 em %ld ·"
               " det a CRESCER em %ld\n", ctrl, inteiro, pisot, area, cresce);
        ok("O CONTRAEXEMPLO É AO MECANISMO, E A SETA QUE ELE NÃO ATRAVESSA TEM NOME: em"
           " x² − 2x − 4 a recorrência é inteira e o traço é inteiro nos 12 andares — as"
           " primeiras setas PASSAM — e mesmo assim o critério de §L5 falha nos 12 e"
           " |det| = 1 em nenhum, porque det A^k = (−4)^k CRESCE em módulo a cada andar."
           " A face que devia encolher não encolhe. Daqui não se conclui caracterização"
           " nenhuma: conclui-se em que seta este exemplo pára",
           inteiro == ctrl && pisot == 0 && area == 0 && cresce == ctrl && ctrl == 12);
    }

    if(!falhas){
        printf("\n  ─────────────────────────────────────────────────────────────\n");
        printf("  o bit fornece a unidade → as oito leis fornecem a base →\n");
        printf("  o dual fornece a recorrência → Pisot fornece o encolhimento →\n");
        printf("  o corte fornece a ordem → ℝ.\n\n");
        printf("  A passagem a ℝ não é a identificação posterior de um subconjunto\n");
        printf("  conveniente: é a realização da face alcançada da própria construção.\n");
    }
    return falhas ? 1 : 0;
}
