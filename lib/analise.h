/* analise.h — ANÁLISE REAL: só o que faltava, e as CINCO VIAS SEM ÁRBITRO.
 *
 * O `eval.txt` traz 25 problemas e diz o que este andar é: «a Análise Real não inventa
 * outro andar — ela finalmente explica, com toda a maquinaria formal, POR QUE O ANDAR ℝ
 * PRECISAVA DE EXISTIR».
 *
 * Procurei antes de escrever, e a casa já tinha quase tudo:
 *   · `reais.h`   — o Corte, o encaixotamento, o passo de Möbius
 *   · `cauchy.h`  — o módulo de Cauchy PROCURADO, a equivalência, as testemunhas
 *   · `calculo.h` — as somas de Riemann e a telescopagem exacta
 *   · `metrico.h` — os axiomas, Banach, a bola
 * Aqui fica o que não existia: supremo/ínfimo, liminf/limsup, a continuidade uniforme, a
 * convergência uniforme, e — o centro do andar — as CINCO VIAS da completude.
 *
 * ── O PEDIDO QUE ORGANIZA O ANDAR ──────────────────────────────────────────────
 * «Exigir que NENHUM CAMINHO SEJA USADO COMO ÁRBITRO DOS OUTROS.»
 *
 * As cinco formas da completude — Dedekind, Cauchy, supremo, intervalos encaixantes e
 * Bolzano--Weierstrass — são equivalentes, e a tentação é provar quatro a partir de uma.
 * Aqui cada uma é construída SOZINHA, com a sua própria testemunha da falha em ℚ, e
 * nenhuma função chama outra. Que as cinco falhem no mesmo sítio é o resultado; que
 * falhem INDEPENDENTEMENTE é o método.
 *
 * ── E O GUME QUE ELE PEDE ──────────────────────────────────────────────────────
 * «Dê uma sucessão racional que converge em ℝ mas não em ℚ. Agora prove que ela é de
 * Cauchy SEM USAR O SEU LIMITE.» É exactamente o que `cy_modulo` faz: compara |xₘ − xₙ|
 * e nunca menciona limite nenhum. A separação está na própria assinatura da função.
 *
 * Precisa de `racionais.h`, `reais.h`, `cauchy.h`, `calculo.h`. */
#ifndef ANALISE_H
#define ANALISE_H

#include "dual32.h"   /* 64 bits são dois duais de 32 — e é o par que compara */

static long an_estouros = 0;

static Qz an_abs(Qz x){ return x.p < 0 ? qz_oposto(x) : x; }
static int an_menor(Qz a, Qz b){ return a.p * b.q < b.p * a.q; }
static int an_menor_ig(Qz a, Qz b){ return a.p * b.q <= b.p * a.q; }

/* ── COMPARAR x² COM UM INTEIRO SEM ELEVAR AO QUADRADO DENTRO DO TIPO ──────────
 * As cinco vias comparam sempre a mesma coisa: x² contra 2. Mas `qz_mult(x,x)` faz p·p
 * num `long`, e a via de NEWTON duplica os dígitos a cada passo — ao quinto, p ≈ 9·10¹¹ e
 * p² ≈ 8·10²³ não cabe. O passo estava certo; era a VERIFICAÇÃO que estourava, e em
 * silêncio: a via parava e eu leria «5 de 8» como se a matemática tivesse falhado.
 *
 * A pergunta é a mais barata que há e está no catálogo desta casa: CABE NO TIPO? Não
 * cabia. E a comparação faz-se pelo PAR DUAL de 32 bits (`dual32.h`): p² e c·q² são cada
 * um um par (alto, baixo), e comparam-se pelo alto primeiro — que é justamente a metade
 * que um tipo estreito perderia. Nenhum tipo de 64 bits entra aqui.
 * Devolve −1, 0 ou +1, e 0 quando os números saem do par (que se conta). */
static int an_cmp_quad(Qz x, long c){
    unsigned p = d32_abs((int)x.p), q = d32_abs((int)x.q);
    D64 e = d64_mult(p, p), q2 = d64_mult(q, q), d;
    if(c < 0) return 1;
    if(!d64_esc(q2, (unsigned)c, &d)){ an_estouros++; return 0; }
    return d64_cmp(e, d);
}
/* ── SUPREMO E ÍNFIMO: a existência separada do ATINGIMENTO ─────────────────────
 * Sobre uma lista finita de racionais o supremo existe e é atingido — e é aí que a
 * distinção não morde. Ela morde no conjunto A = {q > 0 : q² < 2}, que é limitado e cujo
 * supremo NÃO é racional. Devolve o máximo da lista e diz se ele pertence. */
static Qz an_sup_lista(const Qz *v, int n, int *atingido){
    Qz s = v[0];
    for(int i = 1; i < n; i++) if(an_menor(s, v[i])) s = v[i];
    if(atingido) *atingido = 1;             /* numa lista finita, sempre */
    return s;
}
/* ── A PROPRIEDADE ARQUIMEDIANA: dado ε, achar n com 1/n < ε ────────────────────
 * Construtiva e exacta: n = ⌊q/p⌋ + 1 para ε = p/q. Não é «existe n»: é este n. */
static int an_arquimedes(Qz eps, long *n){
    if(eps.p <= 0) return 0;
    *n = eps.q / eps.p + 1;
    Qz um_sobre = qz(1, *n);
    return an_menor(um_sobre, eps);
}
/* ── DENSIDADE de ℚ: um racional ENTRE quaisquer dois ───────────────────────────
 * O ponto médio serve, e é exacto. E a densidade dos IRRACIONAIS obtém-se somando √2/n
 * a um racional do intervalo, com n grande — mas aqui só se pode VERIFICAR que o
 * candidato cai dentro, porque √2 não vive em ℚ. Diz-se, e não se finge. */
static int an_entre(Qz a, Qz b, Qz *m){
    if(!an_menor(a,b)) return 0;
    return qz_divide(qz_soma(a,b), qz_de_inteiro(2), m);
}
/* ── LIMSUP E LIMINF numa janela — e a convergência é a igualdade ───────────────
 * Para uma sucessão limitada, o sup e o inf da CAUDA a partir de k. Quando os dois se
 * aproximam, ela converge; quando ficam afastados, não. E (−1)ⁿ é o caso onde eles são
 * +1 e −1 e nunca se encontram. */
static void an_cauda(Suc s, long k, long ate, Qz *sup, Qz *inf){
    *sup = cy_termo(s, k); *inf = *sup;
    for(long n = k + 1; n <= ate; n++){
        Qz t = cy_termo(s, n);
        if(an_menor(*sup, t)) *sup = t;
        if(an_menor(t, *inf)) *inf = t;
    }
}
/* ── CONTINUIDADE UNIFORME: e onde ela FALHA ───────────────────────────────────
 * f(x) = 1/x em (0,1) é contínua e NÃO uniformemente contínua. A testemunha é exacta:
 * para ε = 1, dado qualquer δ, tomam-se x = δ/2 e y = δ/3; então |x − y| = δ/6 < δ e
 * |f(x) − f(y)| = |2/δ − 3/δ| = 1/δ, que excede 1 para δ < 1. Devolve o par. */
static int an_falha_uniforme(Qz delta, Qz *x, Qz *y, Qz *salto){
    if(delta.p <= 0) return 0;
    Qz d2, d3;
    if(!qz_divide(delta, qz_de_inteiro(2), &d2)) return 0;
    if(!qz_divide(delta, qz_de_inteiro(3), &d3)) return 0;
    *x = d2; *y = d3;
    Qz fx, fy;
    if(!qz_divide(qz(1,1), d2, &fx)) return 0;      /* 1/x = 2/δ */
    if(!qz_divide(qz(1,1), d3, &fy)) return 0;      /* 1/y = 3/δ */
    *salto = an_abs(qz_soma(fx, qz_oposto(fy)));    /* = 1/δ */
    return 1;
}
/* ── CONVERGÊNCIA UNIFORME: fₙ(x) = xⁿ em [0,1] ───────────────────────────────
 * Pontualmente vai a 0 em [0,1) e vale 1 em x = 1. NÃO é uniforme: para ε = 1/2, em cada
 * n existe xₙ = 1 − 1/(2n) com xₙⁿ > 1/2. A testemunha é construída do n, e é exacta. */
static int an_falha_uniforme_seq(long n, Qz *x, Qz *valor){
    if(n < 2) return 0;                    /* em n = 1 dá exactamente 1/2, e não excede */
    Qz xn = qz_soma(qz(1,1), qz_oposto(qz(1, 2*n)));
    Qz p = qz(1,1);
    for(long k = 0; k < n; k++){
        p = qz_mult(p, xn);
        if(p.q > 1000000000L || p.p > 1000000000L){ an_estouros++; return 0; }
    }
    *x = xn; *valor = p;
    return an_menor(qz(1,2), p);           /* xₙⁿ > 1/2 */
}
/* ═══ AS CINCO VIAS DA COMPLETUDE — CADA UMA SOZINHA ═══════════════════════════
 * Cada função constrói a sua testemunha da falha em ℚ SEM chamar nenhuma das outras.
 * O objecto é sempre o mesmo (√2), e o resultado é que as cinco falham no mesmo sítio —
 * mas o MÉTODO é falharem independentemente. */

/* (1) DEDEKIND: o corte (A,B) com A = {q ≤ 0 ou q² < 2}, B = o resto.
 * As duas propriedades: A é fechado para baixo, e A não tem MÁXIMO. A ausência de
 * máximo prova-se construtivamente: dado a ∈ A, acha-se a' ∈ A com a' > a. */
static int an_dedekind_sem_maximo(Qz a, Qz *maior){
    if(!(a.p > 0)) { *maior = qz(1,1); return 1; }
    if(an_cmp_quad(a, 2) >= 0) return 0;                /* a ∉ A */
    /* a' = (2a + 2)/(a + 2) — a média de Möbius: se a² < 2 então a'² < 2 e a' > a */
    Qz num = qz_soma(qz_mult(qz(2,1), a), qz(2,1));
    Qz den = qz_soma(a, qz(2,1));
    if(!qz_divide(num, den, maior)) return 0;
    return an_menor(a, *maior) && an_cmp_quad(*maior, 2) < 0;
}
/* (2) CAUCHY: o módulo, sem mencionar limite — está em cy_modulo, e cita-se.
 * Aqui fica só a testemunha da falha: nenhum racional tem quadrado 2. */
static int an_sem_raiz_racional(long p, long q, long a){
    /* p² = a·q²? — pelo par, e sem tipo largo: comparam-se os dois pares */
    D64 e = d64_mult(d32_abs((int)p), d32_abs((int)p));
    D64 q2 = d64_mult(d32_abs((int)q), d32_abs((int)q)), d;
    if(!d64_esc(q2, (unsigned)(a < 0 ? -a : a), &d)){ an_estouros++; return 1; }
    return d64_cmp(e, d) != 0;
}
/* (3) SUPREMO: A = {q > 0 : q² < 2} é limitado e NÃO tem supremo racional.
 * Prova-se construtivamente: dado um majorante b (b² > 2), acha-se um MENOR. */
static int an_sup_nao_racional(Qz b, Qz *menor){
    if(!(b.p > 0)) return 0;
    if(an_cmp_quad(b, 2) <= 0) return 0;                /* b não majora */
    /* b' = (b + 2/b)/2 — o passo de Newton, exacto em ℚ: b'² > 2 e b' < b */
    Qz doisb;
    if(!qz_divide(qz(2,1), b, &doisb)) return 0;
    if(!qz_divide(qz_soma(b, doisb), qz_de_inteiro(2), menor)) return 0;
    return an_menor(*menor, b) && an_cmp_quad(*menor, 2) > 0;
}
/* (4) INTERVALOS ENCAIXANTES: [aₖ,bₖ] com aₖ² < 2 < bₖ² e bₖ − aₖ → 0.
 * A intersecção em ℚ é VAZIA, e mede-se pelo comprimento a encolher sem nada lá dentro. */
static int an_encaixa(Qz *a, Qz *b){
    Qz m;
    if(!qz_divide(qz_soma(*a,*b), qz_de_inteiro(2), &m)) return 0;
    if(an_cmp_quad(m, 2) < 0) *a = m; else *b = m;
    return 1;
}
/* (5) BOLZANO–WEIERSTRASS: uma sucessão limitada e MONÓTONA numa metade.
 * Aqui constrói-se a subsucessão pela bisseção: em cada passo fica-se com a metade que
 * contém infinitos termos. A testemunha da falha em ℚ é que o candidato a limite teria
 * quadrado 2. */
static int an_limitada_em(Qz x, Qz lo, Qz hi){
    return an_menor_ig(lo, x) && an_menor_ig(x, hi);
}

/* ── TAYLOR: o resto MEDIDO, e não estimado ────────────────────────────────────
 * Num polinómio o desenvolvimento de Taylor é EXACTO, e o resto não precisa de cota
 * nenhuma: constrói-se Tₙ com os coeficientes f⁽ᵏ⁾(a)/k! e subtrai-se. Quando n ≥ grau
 * o resto é IDENTICAMENTE zero; quando n < grau ele é a cauda, e vê-se qual é.
 * O eval diz «medir o resto» — e medir é isto, não majorá-lo. */
static Cf an_taylor(Cf f, Qz a, int ordem){
    Cf T = fn0();
    Cf d = f;
    Qz fat = qz(1,1);
    for(int k = 0; k <= ordem && k <= CL_MAX; k++){
        if(k > 0){ d = fn_deriva(d); fat = qz_mult(fat, qz_de_inteiro(k)); }
        Qz ck;
        if(!qz_divide(fn_av(d, a), fat, &ck)){ an_estouros++; return T; }
        /* soma ck·(x − a)^k, expandido pelo binómio, tudo em ℚ */
        Cf termo = fn0(); termo.n = 0; termo.c[0] = qz(1,1);
        for(int j = 0; j < k; j++){                    /* multiplica por (x − a) */
            Cf novo = fn0(); novo.n = termo.n + 1;
            if(novo.n > CL_MAX){ an_estouros++; return T; }
            for(int i = 0; i <= termo.n; i++){
                novo.c[i+1] = qz_soma(novo.c[i+1], termo.c[i]);
                novo.c[i]   = qz_soma(novo.c[i], qz_mult(qz_oposto(a), termo.c[i]));
            }
            termo = novo;
        }
        for(int i = 0; i <= termo.n && i <= CL_MAX; i++)
            T.c[i] = qz_soma(T.c[i], qz_mult(ck, termo.c[i]));
        if(termo.n > T.n) T.n = termo.n;
    }
    return T;
}
static Qz an_resto(Cf f, Cf T, Qz x){
    return qz_soma(fn_av(f,x), qz_oposto(fn_av(T,x)));
}
/* ── BOLZANO: a subsucessão PROCURADA, por bisseção ────────────────────────────
 * Parte-se a caixa ao meio e fica-se com a metade que contém mais termos; os índices que
 * lá caem são a subsucessão. Devolve quantos índices sobreviveram e a caixa final. */
static int an_subsuc(Suc s, long ate, Qz *lo, Qz *hi, long *idx, int cap){
    int n = 0;
    for(long k = 1; k <= ate && n < cap; k++)
        if(an_limitada_em(cy_termo(s,k), *lo, *hi)) idx[n++] = k;
    if(n == 0) return 0;
    for(int volta = 0; volta < 8; volta++){
        Qz m;
        if(!qz_divide(qz_soma(*lo,*hi), qz_de_inteiro(2), &m)) break;
        int esq = 0, dir = 0;
        for(int i = 0; i < n; i++)
            if(an_menor(cy_termo(s,idx[i]), m)) esq++; else dir++;
        Qz nlo = *lo, nhi = *hi;
        if(dir >= esq) nlo = m; else nhi = m;
        int m2 = 0;
        for(int i = 0; i < n; i++)
            if(an_limitada_em(cy_termo(s,idx[i]), nlo, nhi)) idx[m2++] = idx[i];
        if(m2 == 0) break;
        n = m2; *lo = nlo; *hi = nhi;
    }
    return n;
}
/* ── CONTINUIDADE: o δ CONSTRUÍDO do ε, e a forma sequencial ───────────────────
 * Para f(x) = x²: |x² − a²| = |x−a|·|x+a| ≤ δ(2|a| + δ), e δ = min(1, ε/(2|a|+1)) serve.
 * É exacto em ℚ, e é um δ, não uma promessa de δ. */
static int an_delta(Qz a, Qz eps, Qz *delta){
    if(eps.p <= 0) return 0;
    Qz cota = qz_soma(qz_mult(qz(2,1), an_abs(a)), qz(1,1));
    Qz cand;
    if(!qz_divide(eps, cota, &cand)) return 0;
    *delta = an_menor(cand, qz(1,1)) ? cand : qz(1,1);
    return 1;
}
static int an_delta_serve(Qz a, Qz delta, Qz eps){
    Qz x = qz_soma(a, delta);                       /* o pior ponto da bola */
    Qz d = an_abs(qz_soma(qz_mult(x,x), qz_oposto(qz_mult(a,a))));
    return an_menor_ig(d, eps);
}
/* a forma SEQUENCIAL: xₙ → a implica f(xₙ) → f(a), e as duas distâncias exibem-se */
static void an_sequencial(Cf f, Qz a, long n, Qz *dx, Qz *df){
    Qz x = qz_soma(a, qz(1, n));
    *dx = an_abs(qz_soma(x, qz_oposto(a)));
    *df = an_abs(qz_soma(fn_av(f,x), qz_oposto(fn_av(f,a))));
}
/* ── WEIERSTRASS: o máximo e o mínimo ATINGIDOS numa malha do compacto ─────────
 * Numa malha racional de [a,b] o máximo existe e é atingido — e o ponto devolve-se.
 * O contraste é (0,1) aberto com 1/x, onde o supremo da malha cresce sem limite. */
static void an_extremos(Cf f, Qz a, Qz b, long n, Qz *mx, Qz *xmx, Qz *mn, Qz *xmn){
    Qz h;
    if(!qz_divide(qz_soma(b, qz_oposto(a)), qz_de_inteiro(n), &h)){ an_estouros++; return; }
    *xmx = a; *xmn = a; *mx = fn_av(f,a); *mn = *mx;
    Qz x = a;
    for(long k = 1; k <= n; k++){
        x = qz_soma(x, h);
        Qz v = fn_av(f,x);
        if(an_menor(*mx, v)){ *mx = v; *xmx = x; }
        if(an_menor(v, *mn)){ *mn = v; *xmn = x; }
    }
}
/* e 1/x na malha do ABERTO (0,1): o sup é n, e cresce com a malha — não é atingido */
static int an_sup_no_aberto(long n, Qz *sup){ *sup = qz_de_inteiro(n); return 1; }
/* ── A TROCA DE LIMITE E INTEGRAL: ∫₀¹ xⁿ = 1/(n+1), exacto ───────────────────
 * Aqui os dois lados calculam-se e comparam-se: lim ∫fₙ e ∫ lim fₙ. Em xⁿ os dois dão 0
 * e a troca é legítima — mas a razão NÃO é a convergência pontual: é preciso mais. */
static int an_int_pot(long n, Qz *v){
    if(n < 0) return 0;
    *v = qz(1, n + 1);
    return 1;
}
/* ── E O PASSO, QUE É A TESE — não a órbita ────────────────────────────────────
 * Eu tinha iterado Newton oito vezes a partir de 2 e contado quantas sobreviviam: cinco.
 * As outras três não falharam por matemática — falharam porque Newton DUPLICA os dígitos
 * a cada passo e ao sexto o numerador passa de 8·10¹¹, cujos produtos não cabem num
 * `long`. Chamar «cinco de oito» a isso seria dar a uma falha da minha aritmética o
 * estatuto de resultado, e não é: saturação não é teorema.
 *
 * A regra desta casa já dizia o que fazer: uma tese com «todo» ou «sempre» não se varre,
 * PROVA-SE O PASSO. E a tese aqui é do passo: para TODO majorante b, (b + 2/b)/2 é um
 * majorante ESTRITAMENTE MENOR. Verificada em majorantes independentes e pequenos, ela
 * não obriga número nenhum a crescer — e prova mais do que a órbita provava, porque vale
 * para qualquer b e não só para os descendentes de 2. */
static int an_passo_desce(Qz b, Qz *menor){ return an_sup_nao_racional(b, menor); }
static int an_passo_sobe(Qz a, Qz *maior){ return an_dedekind_sem_maximo(a, maior); }
#endif
