/* calculo.h — CÁLCULO I EXACTO, SEM UM ÚNICO DOUBLE.
 *
 * O `eval.txt` traz o Cálculo I inteiro — funções, limites ε-δ, continuidade, derivada,
 * Fermat/Rolle/Valor Médio, integral de Riemann e o Teorema Fundamental — com vinte
 * demonstrações e «um gume explícito em cada teorema: retirar uma hipótese e procurar
 * automaticamente um contraexemplo».
 *
 * O problema aparente é que Cálculo I é a matéria dos limites, e esta casa não usa
 * doubles. Mas o problema desaparece assim que se olha para o objecto certo:
 *
 * ── O ACHADO: A DERIVADA DE UM POLINÓMIO NÃO PRECISA DE LIMITE ──────────────────
 * Para f polinomial, o quociente de diferenças
 *
 *        q(h) = (f(a+h) − f(a)) / h
 *
 * É ELE PRÓPRIO UM POLINÓMIO EM h — porque f(a+h) − f(a) não tem termo constante, logo
 * é divisível por h EXACTAMENTE. E então f'(a) = q(0), que é uma AVALIAÇÃO, não um
 * limite. Não há processo infinito nenhum: há uma divisão exata.
 *
 * E a divisão exata é a FIBRA (`thm:divisao-fibra` do corpo universal): «a divisão não
 * se postula como aritmética: é a fibra inversa da fusão». Aqui a fibra é «dado
 * f(a+h) − f(a) e o factor h, achar o outro factor». Ela existe porque o dividendo se
 * anula em h = 0, e é exactamente aí que a derivada vive.
 *
 * ── A TRADUÇÃO PARA OS TEOREMAS DO UNIVERSAL ────────────────────────────────────
 *   limite            ↔ o CORTE e os intervalos encaixantes  (`thm:real-caminho`)
 *   ε-δ               ↔ a escada de observadores: cada ε é um degrau (`thm:escada`)
 *   derivada          ↔ a fibra da divisão por h (`thm:divisao-fibra`); e no anel a
 *                       derivada discreta é c_k ↦ (ω^k − 1)c_k — «o ik do contínuo é
 *                       ω^k − 1» (`thm:metronomo-fourier`)
 *   integral          ↔ a soma reversível de Gentil/Lebesgue (`obs:triade-central`)
 *   TFC               ↔ ∫f + ∫f⁻¹ = b·f(b) − a·f(a): a VOLTA, ν∘ν = id (`thm:central`)
 *   aproximação linear↔ o segmento afim que sela o contínuo, erro ≤ 2⁻ᵏ
 *                       (`thm:batuta-continuo`)
 *   TVI               ↔ a bisseção no corte, que é o corte de Dedekind a decidir
 *   continuidade      ↔ a membrana: o corpo em trânsito pela fatoração (`def:membrana`)
 *
 * Tudo em Qz. Precisa de `racionais.h`. */
#ifndef CALCULO_H
#define CALCULO_H

#define CL_MAX 8                  /* grau máximo; o tecto verifica-se em cl_estouros */
static long cl_estouros = 0;

typedef struct { Qz c[CL_MAX+1]; int n; } Cf;    /* c[0] + c[1]x + … + c[n]xⁿ */

static Cf fn0(void){ Cf f; f.n = 0; for(int i = 0; i <= CL_MAX; i++) f.c[i] = qz(0,1); return f; }
static Cf fn_const(Qz a){ Cf f = fn0(); f.c[0] = a; return f; }
static void fn_ajusta(Cf *f){ while(f->n > 0 && f->c[f->n].p == 0) f->n--; }
static int fn_igual(Cf f, Cf g){
    int n = f.n > g.n ? f.n : g.n;
    for(int i = 0; i <= n; i++){
        Qz a = i <= f.n ? f.c[i] : qz(0,1), b = i <= g.n ? g.c[i] : qz(0,1);
        if(!qz_igual(a,b)) return 0;
    }
    return 1;
}
/* avaliação por HORNER — n multiplicações, e exacta */
static Qz fn_av(Cf f, Qz x){
    Qz s = f.c[f.n];
    for(int i = f.n - 1; i >= 0; i--) s = qz_soma(qz_mult(s, x), f.c[i]);
    return s;
}
static Cf fn_soma(Cf f, Cf g){
    Cf r = fn0(); r.n = f.n > g.n ? f.n : g.n;
    if(r.n > CL_MAX){ cl_estouros++; r.n = CL_MAX; }
    for(int i = 0; i <= r.n; i++)
        r.c[i] = qz_soma(i <= f.n ? f.c[i] : qz(0,1), i <= g.n ? g.c[i] : qz(0,1));
    fn_ajusta(&r);
    return r;
}
static Cf fn_esc(Qz l, Cf f){
    Cf r = f;
    for(int i = 0; i <= f.n; i++) r.c[i] = qz_mult(l, f.c[i]);
    fn_ajusta(&r);
    return r;
}
static Cf fn_mult(Cf f, Cf g){
    Cf r = fn0();
    if(f.n + g.n > CL_MAX){ cl_estouros++; return r; }
    r.n = f.n + g.n;
    for(int i = 0; i <= f.n; i++) for(int j = 0; j <= g.n; j++)
        r.c[i+j] = qz_soma(r.c[i+j], qz_mult(f.c[i], g.c[j]));
    fn_ajusta(&r);
    return r;
}
/* ── A DERIVADA FORMAL, e a que sai da DEFINIÇÃO ────────────────────────────────
 * A formal é a regra de sempre. A da definição constrói-se pelo quociente de
 * diferenças, e as duas TÊM DE CONCORDAR — dois caminhos independentes. */
static Cf fn_deriva(Cf f){
    Cf r = fn0();
    if(f.n == 0) return r;
    r.n = f.n - 1;
    for(int i = 1; i <= f.n; i++) r.c[i-1] = qz_mult(qz_de_inteiro(i), f.c[i]);
    fn_ajusta(&r);
    return r;
}
/* f(a+h) como polinómio em h — a translação (o desenvolvimento de Taylor, exacto) */
static Cf fn_desloca(Cf f, Qz a){
    Cf r = fn0(), pot = fn_const(qz(1,1));       /* pot = (x+a)^i, em h */
    Cf xa = fn0(); xa.n = 1; xa.c[0] = a; xa.c[1] = qz(1,1);   /* h + a */
    for(int i = 0; i <= f.n; i++){
        r = fn_soma(r, fn_esc(f.c[i], pot));
        if(i < f.n) pot = fn_mult(pot, xa);
    }
    return r;
}
/* ── O QUOCIENTE DE DIFERENÇAS, e a FIBRA que o resolve ─────────────────────────
 * q(h) = (f(a+h) − f(a))/h. O dividendo anula-se em h = 0, portanto é divisível por h
 * EXACTAMENTE — e dividir por h é só baixar os índices. Devolve 0 se o termo constante
 * não for nulo, que é o caso em que a fibra seria vazia (e não acontece nunca aqui:
 * mede-se para o mostrar). */
static int fn_quociente(Cf f, Qz a, Cf *q){
    Cf d = fn_desloca(f, a);
    d.c[0] = qz_soma(d.c[0], qz_oposto(fn_av(f, a)));      /* f(a+h) − f(a) */
    fn_ajusta(&d);
    if(d.c[0].p != 0) return 0;                            /* a fibra seria vazia */
    Cf r = fn0();
    r.n = d.n > 0 ? d.n - 1 : 0;
    for(int i = 1; i <= d.n; i++) r.c[i-1] = d.c[i];
    fn_ajusta(&r);
    *q = r;
    return 1;
}
/* a derivada PELA DEFINIÇÃO: f'(a) = q(0), uma AVALIAÇÃO e não um limite */
static int fn_deriva_def(Cf f, Qz a, Qz *saida){
    Cf q;
    if(!fn_quociente(f, a, &q)) return 0;
    *saida = fn_av(q, qz(0,1));
    return 1;
}
/* ── O TERCEIRO CAMINHO: O NÚMERO DUAL, que a casa JÁ CORRIA ────────────────────
 * Procurei antes de escrever, e o `resolve_calculo` de `conversa.c` já derivava exacto
 * há muito, por outra via: «é a parte ε do dual, f(a+bε) = f(a) + f'(a)·b·ε, com ε² = 0
 * — a derivada é exata, sem passo h e sem limite».
 *
 * É a MESMA conclusão por outro caminho, e o ε com ε² = 0 é o dual desta casa outra vez.
 * Então não se refaz: mede-se a CONCORDÂNCIA. Ficam TRÊS caminhos independentes para a
 * derivada — a regra formal, o quociente de diferenças, e a parte ε do dual — e eles têm
 * de dar o mesmo em todo ponto. Um deles errado, e a comparação denuncia-o. */
typedef struct { Qz p, e; } Dual;                 /* p + e·ε, com ε² = 0 */
static Dual dl_soma(Dual a, Dual b){ Dual r; r.p = qz_soma(a.p,b.p); r.e = qz_soma(a.e,b.e); return r; }
static Dual dl_mult(Dual a, Dual b){              /* o ε² cai fora sozinho */
    Dual r; r.p = qz_mult(a.p, b.p);
    r.e = qz_soma(qz_mult(a.p, b.e), qz_mult(a.e, b.p));
    return r;
}
/* avaliar f em a + 1·ε devolve (f(a), f'(a)) — a derivada é a segunda coordenada */
static Dual fn_av_dual(Cf f, Qz a){
    Dual x; x.p = a; x.e = qz(1,1);
    Dual s; s.p = f.c[f.n]; s.e = qz(0,1);
    for(int i = f.n - 1; i >= 0; i--){
        Dual c; c.p = f.c[i]; c.e = qz(0,1);
        s = dl_soma(dl_mult(s, x), c);
    }
    return s;
}
/* ── ε-δ: PROCURADO, não afirmado ───────────────────────────────────────────────
 * Dado ε racional, procura δ = 1/2^k tal que 0 < |x−a| < δ ⟹ |f(x) − L| < ε, e
 * VERIFICA a implicação numa malha do intervalo. É o que o eval pede: «procurar
 * automaticamente um δ válido e depois verificar a implicação inteira». */
static int cl_menor(Qz x, Qz y){                  /* |x| < |y|, exacto, sem raiz */
    Qz a = x.p < 0 ? qz_oposto(x) : x, b = y.p < 0 ? qz_oposto(y) : y;
    return a.p * b.q < b.p * a.q;
}
static int fn_acha_delta(Cf f, Qz a, Qz L, Qz eps, int kmax, Qz *delta, long malha){
    for(int k = 0; k <= kmax; k++){
        Qz d = qz(1, 1L << k);
        int bom = 1;
        for(long i = -malha; i <= malha && bom; i++){
            if(i == 0) continue;                  /* 0 < |x−a| exclui o próprio a */
            Qz x = qz_soma(a, qz_mult(d, qz(i, malha)));
            if(!cl_menor(qz_soma(x, qz_oposto(a)), d)) continue;
            if(!cl_menor(qz_soma(fn_av(f,x), qz_oposto(L)), eps)) bom = 0;
        }
        if(bom){ *delta = d; return 1; }
    }
    return 0;
}
/* ── O VALOR INTERMÉDIO por BISSEÇÃO — o corte de Dedekind a decidir ────────────
 * Não devolve «a raiz»: devolve o INTERVALO ENCAIXANTE que a contém, com o sinal
 * trocado nos extremos. É a régua desta casa — o real é o corte, nunca um decimal. */
static long cl_saturou = 0;    /* passos que a REPRESENTAÇÃO não deu, contados à parte */

/* o maior denominador que sobrevive a ser elevado ao grau — a pergunta do Gato feita ao
 * grau e não só ao número: «cabe no tipo?» depende de quantas vezes ele se multiplica */
static long cl_tecto_grau(int grau){
    /* q^d ≤ 10¹⁸, e não 9·10¹⁸ — a folga é para as SOMAS que vêm a seguir. Encostar o
     * tecto ao limite do tipo é a mesma miopia de não ter tecto nenhum: o que estoura é
     * sempre a operação seguinte àquela que se vigiou. */
    if(grau <= 1) return 1000000000L;
    if(grau == 2) return 1000000000L;   /* q² = 10¹⁸ */
    if(grau == 3) return 1000000L;      /* q³ = 10¹⁸ */
    if(grau == 4) return 31600L;
    if(grau == 5) return 3980L;
    return 1000L;
}

static int fn_bissec(Cf f, Qz a, Qz b, int passos, Qz *lo, Qz *hi){
    Qz fa = fn_av(f,a), fb = fn_av(f,b);
    if(fa.p == 0){ *lo = *hi = a; return 1; }
    if(fb.p == 0){ *lo = *hi = b; return 1; }
    if((fa.p > 0) == (fb.p > 0)) return 0;         /* sem troca de sinal: sem garantia */
    Qz meio;
    for(int k = 0; k < passos; k++){
        /* O TECTO, E ELE DEPENDE DO GRAU — que foi a segunda vez que me enganei aqui.
         *
         * À primeira não havia guarda nenhuma: pedia-se profundidade 40, o denominador
         * DUPLICA a cada bisseção, e ao passo 32 a soma qz_soma(a,b) — que faz a.q·b.q —
         * enrolava em silêncio. Os últimos nove passos corriam sobre inteiros já
         * enrolados e a função devolvia 1 na mesma: uma SATURAÇÃO a sair como veredicto.
         *
         * Pus então um tecto no denominador do MEIO, e continuou a estourar — porque o
         * que estoura não é o meio, é `fn_av` a AVALIAR nele. Por Horner, um polinómio de
         * grau d avaliado em p/q carrega q^d, e com d = 3 e q ≈ 10⁹ isso é 10²⁷. O tecto
         * não é do número: é do NÚMERO ELEVADO AO GRAU, e eu não tinha perguntado o grau.
         *
         * Logo q^d ≤ 9·10¹⁸, e o tecto sai daí — 9·10¹⁸ para grau 1, 3·10⁹ para grau 2,
         * 2·10⁶ para grau 3, 5,5·10⁴ para grau 4. A profundidade honesta da bisseção é a
         * que o grau permite, e não a que eu escrevi na chamada. Conta-se em
         * `cl_saturou`, à parte dos defeitos, e devolve-se o intervalo que se TEM — mais
         * largo e verdadeiro em vez de estreito e falso. */
        qz_divide(qz_soma(a,b), qz_de_inteiro(2), &meio);
        /* e a guarda vai no MEIO, que é quem vai ser avaliado — pô-la em a e b ainda me
         * deixou estourar, porque o denominador do meio é MAIOR que os dois. O número a
         * vigiar é aquele que entra na conta seguinte, não os que lá estavam. */
        if(meio.q > cl_tecto_grau(f.n)){ cl_saturou++; break; }
        Qz fm = fn_av(f, meio);
        if(fm.p == 0){ *lo = *hi = meio; return 1; }
        if((fa.p > 0) != (fm.p > 0)){ b = meio; fb = fm; }
        else { a = meio; fa = fm; }
    }
    *lo = a; *hi = b;
    return 1;
}
/* ── ROLLE E O VALOR MÉDIO: o c PROCURADO numa malha racional ───────────────────
 * Procura c em (a,b) da forma a + j(b−a)/M com f'(c) = 0 (Rolle) ou com
 * f'(c) = (f(b)−f(a))/(b−a) (Valor Médio). Quando a malha não o apanha, diz-se — e o
 * intervalo encaixante entra pela bisseção sobre f' − a inclinação. */
static int fn_acha_c(Cf f, Qz a, Qz b, Qz alvo, long M, Qz *c){
    Cf d = fn_deriva(f);
    Cf g = fn_soma(d, fn_const(qz_oposto(alvo)));         /* f' − alvo */
    for(long j = 1; j < M; j++){
        Qz x = qz_soma(a, qz_mult(qz_soma(b, qz_oposto(a)), qz(j, M)));
        if(fn_av(g, x).p == 0){ *c = x; return 1; }
    }
    /* não caiu na malha: entrega o intervalo por bisseção sobre f' − alvo */
    Qz lo, hi;
    if(fn_bissec(g, a, b, 40, &lo, &hi)){ *c = lo; return 2; }
    return 0;
}
/* ── A INTEGRAL: soma de Riemann EXACTA, e a primitiva EXACTA ───────────────────
 * A soma de Riemann com n subintervalos e o ponto DIREITO, tudo em ℚ. E a primitiva
 * obtém-se baixando o índice e dividindo por k+1 — uma fibra outra vez. */
static Qz fn_riemann(Cf f, Qz a, Qz b, long n, int direito){
    Qz h, s = qz(0,1);
    qz_divide(qz_soma(b, qz_oposto(a)), qz_de_inteiro(n), &h);
    for(long k = 0; k < n; k++){
        Qz x = qz_soma(a, qz_mult(h, qz_de_inteiro(direito ? k+1 : k)));
        s = qz_soma(s, qz_mult(fn_av(f, x), h));
    }
    return s;
}
static int fn_primitiva(Cf f, Cf *F){
    Cf r = fn0();
    if(f.n + 1 > CL_MAX){ cl_estouros++; return 0; }
    r.n = f.n + 1;
    for(int i = 0; i <= f.n; i++)
        if(!qz_divide(f.c[i], qz_de_inteiro(i+1), &r.c[i+1])) return 0;
    fn_ajusta(&r);
    *F = r;
    return 1;
}
/* ── O GUME AUTOMÁTICO: retirar a hipótese e PROCURAR o contra-exemplo ──────────
 * A varredura de polinómios de grau ≤ 2 com coeficientes pequenos, à procura de um f
 * que satisfaça a hipótese enfraquecida e VIOLE a tese. Devolve 1 e o contra-exemplo
 * quando acha. Precisa de DOIS controlos: um regime onde tem de achar e outro onde tem
 * de voltar vazio — senão «não achou» não distingue a lei do buscador partido. */
static long fn_gume(long lim, int (*hip)(Cf), int (*tese)(Cf), Cf *contra){
    long achados = 0;
    for(long a = -lim; a <= lim; a++) for(long b = -lim; b <= lim; b++)
    for(long c = -lim; c <= lim; c++){
        Cf f = fn0(); f.n = 2;
        f.c[0] = qz_de_inteiro(c); f.c[1] = qz_de_inteiro(b); f.c[2] = qz_de_inteiro(a);
        fn_ajusta(&f);
        if(!hip(f)) continue;
        if(!tese(f)){ if(!achados) *contra = f; achados++; }
    }
    return achados;
}
#endif
