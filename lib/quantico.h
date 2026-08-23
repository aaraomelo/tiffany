/* ═══════════════════════════════════════════════════════════════════════════
 * lib/quantico.h — o estado, a fase global, e porque a leitura é ρ e não ψ
 *
 * Neste corpo a igualdade NÃO é a dos parâmetros:
 *
 *     |ψ⟩ ~ u|ψ⟩   para u unidade      --- é o MESMO estado
 *
 * Escrito com as amplitudes em Z[i], a fase é exacta e finita: as unidades são
 * quatro, {1, i, −1, −i}. E aí o critério das duas metades decide sozinho:
 *
 *   pelas AMPLITUDES   separa mas NÃO é bem definida --- QUEBRA o estado
 *   por ρ = |ψ⟩⟨ψ|     bem definida E separadora     --- SERVE
 *
 * Não é escolha de gosto: das duas leituras do mesmo corpo, só uma endereça o
 * objecto. A fase entra por u e sai por ū, e ρ não a vê.
 *
 * Tudo em inteiros. Medido em `tests/quantico.c` e `tests/pgwire.c` §W136.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef QUANTICO_H
#define QUANTICO_H

/* |ψ⟩ = (a+bi, c+di), com as quatro amplitudes inteiras */
typedef struct { long a, b, c, d; } Psi;

static Psi psi(long a, long b, long c, long d){ Psi p = {a,b,c,d}; return p; }
static int psi_nulo(Psi p){ return p.a == 0 && p.b == 0 && p.c == 0 && p.d == 0; }

/* multiplicar pela unidade i: (x+yi)·i = −y + xi, nas duas componentes */
static Psi psi_roda(Psi p){ Psi r = {-p.b, p.a, -p.d, p.c}; return r; }

/* ── A IGUALDADE DO CORPO É A PROJECTIVA, e não só as quatro unidades:
 *
 *     |ψ⟩ ~ λ|ψ⟩   para todo λ ≠ 0
 *
 * O espaço de estados é P¹, e é essa a igualdade que a física usa. A que só
 * admite as unidades de Z[i] é PEQUENA DEMAIS --- foi o que a medição mostrou:
 * a leitura pela fase «fundia» centenas de pares que, à luz da igualdade certa,
 * são o MESMO estado. O defeito não era da leitura: era da igualdade.
 *
 * `psi_mesma_fase` é a igualdade do corpo; `psi_mesmo_estado` fica como a
 * relação estreita (só as unidades), que é a que o degrau Z[i] realiza. */
static int psi_mesma_fase(Psi x, Psi y){
    /* λ existe sse os «produtos cruzados» se anulam: x1·y2 = x2·y1 em C */
    long re = x.a*y.c - x.b*y.d - (x.c*y.a - x.d*y.b);
    long im = x.a*y.d + x.b*y.c - (x.c*y.b + x.d*y.a);
    return re == 0 && im == 0;
}

/* ── E A RELAÇÃO ESTREITA: ψ ~ u·ψ para alguma das quatro unidades. É a que o
 * degrau Z[i] realiza, e é menor que a do corpo. */
static int psi_mesmo_estado(Psi x, Psi y){
    Psi t = x;
    for(int k = 0; k < 4; k++){
        if(t.a == y.a && t.b == y.b && t.c == y.c && t.d == y.d) return 1;
        t = psi_roda(t);
    }
    return 0;
}

/* ── A LEITURA QUE QUEBRA: as amplitudes. Separa, e é por isso que falha ---
 * distingue ψ de iψ, que a física diz ser o mesmo. */
static long psi_end_amplitudes(Psi p, long base){
    return ((p.a + base)*2*base + (p.b + base))*2*base*2*base
         + (p.c + base)*2*base + (p.d + base);
}

/* ── A LEITURA QUE SERVE: ρ = |ψ⟩⟨ψ|, invariante à fase.
 *   ρ11 = |ψ1|²,  ρ22 = |ψ2|²,  ρ12 = ψ1·conj(ψ2)
 * A fase entra por u e sai por ū, com uū = 1 --- ρ não a vê. */
typedef struct { long r11, r22, r12re, r12im; } Rho;

static Rho psi_rho(Psi p){
    Rho r;
    r.r11 = p.a*p.a + p.b*p.b;
    r.r22 = p.c*p.c + p.d*p.d;
    r.r12re = p.a*p.c + p.b*p.d;
    r.r12im = p.b*p.c - p.a*p.d;
    return r;
}
static int rho_igual(Rho x, Rho y){
    return x.r11 == y.r11 && x.r22 == y.r22
        && x.r12re == y.r12re && x.r12im == y.r12im;
}

/* ── A PUREZA, exacta: det ρ = ρ11·ρ22 − |ρ12|² = 0 para todo estado ─────── */
static long rho_det(Rho r){
    return r.r11*r.r22 - (r.r12re*r.r12re + r.r12im*r.r12im);
}
static int rho_puro(Rho r){ return rho_det(r) == 0; }

/* ── A FASE QUE ρ MATA É MAIOR QUE A DO CORPO ────────────────────────────
 * ρ determina ψ a menos de fase de norma 1 em ℚ(i) --- e Z[i] só tem QUATRO
 * unidades. Logo há estados com o mesmo ρ que NÃO diferem por unidade: eles
 * diferem por uma fase racional gaussiana de norma 1, como (3−4i)/5.
 *
 * Verifica-se sem sair dos inteiros: ψ' = λ·ψ com λ = p/q de norma 1 sse
 *   q·ψ'_k = p·ψ_k  para as duas componentes, com N(p) = N(q).
 * Devolve 1 quando os dois estados diferem por uma dessas fases. */
static int psi_fase_racional(Psi x, Psi y){
    /* λ = y1/x1 = (y1 · conj(x1)) / N(x1); e o mesmo tem de valer na 2ª */
    long nx1 = x.a*x.a + x.b*x.b, nx2 = x.c*x.c + x.d*x.d;
    long ny1 = y.a*y.a + y.b*y.b, ny2 = y.c*y.c + y.d*y.d;
    if(nx1 != ny1 || nx2 != ny2) return 0;      /* a norma tem de bater */
    /* λ pela primeira componente: (p_re + p_im i) / nx1 */
    long p_re, p_im;
    if(nx1 != 0){ p_re = y.a*x.a + y.b*x.b; p_im = y.b*x.a - y.a*x.b; }
    else        { p_re = y.c*x.c + y.d*x.d; p_im = y.d*x.c - y.c*x.d;
                  nx1 = nx2; }
    if(nx1 == 0) return 0;
    /* e a segunda componente tem de dar o MESMO λ: nx1·y2 = λ·nx1·x2 */
    long q_re = p_re*x.c - p_im*x.d, q_im = p_re*x.d + p_im*x.c;
    return q_re == nx1*y.c && q_im == nx1*y.d;
}

/* ── A REPRESENTAÇÃO É A FASE, E O ENDEREÇO É A CLASSE PROJECTIVA ────────
 * «O módulo não carrega nada, e a fase carrega tudo» (aranha §transf). Logo o
 * endereço de um estado não se lê da amplitude: lê-se da FASE, e a fase de
 * (z,w) a menos de factor comum é o ponto de P¹.
 *
 * Reduz-se pelo mdc gaussiano e normaliza-se pela unidade: o resultado é o
 * representante da classe, e é ELE o endereço. E daí a régua da casa desce
 * sobre ele --- a ultramétrica da def:arvore, como em qualquer outro corpo.
 *
 * O ZERO NÃO TEM FASE, e por isso não tem endereço: (0,0) não é ponto de P¹.
 * Devolve 0 e não toca em `out` --- a excepção fica dita, não escondida. */
static long q_norma(long a, long b){ return a*a + b*b; }

static int psi_endereco(Psi p, long base, long *out){
    if(psi_nulo(p)) return 0;                  /* o zero não tem fase */
    long a = p.a, b = p.b, c = p.c, d = p.d;
    /* o mdc gaussiano de (a+bi) e (c+di), por divisões sucessivas na norma */
    long ga = a, gb = b, ha = c, hb = d;
    while(q_norma(ha, hb) != 0){
        long N = q_norma(ha, hb);
        long pr = ga*ha + gb*hb, pi = gb*ha - ga*hb;
        long qr = (pr >= 0 ? (2*pr + N) : (2*pr - N)) / (2*N);
        long qi = (pi >= 0 ? (2*pi + N) : (2*pi - N)) / (2*N);
        long rr = ga - (qr*ha - qi*hb), ri = gb - (qr*hb + qi*ha);
        ga = ha; gb = hb; ha = rr; hb = ri;
    }
    /* divide-se o par pelo mdc: (a+bi)/(g) */
    long N = q_norma(ga, gb);
    if(N == 0) return 0;
    long ra = (a*ga + b*gb) / N, rb = (b*ga - a*gb) / N;
    long rc = (c*ga + d*gb) / N, rd = (d*ga - c*gb) / N;
    /* normaliza pela unidade: roda até a primeira componente não nula ficar
     * no quadrante canónico (parte real > 0, ou real = 0 e imaginária > 0) */
    for(int k = 0; k < 4; k++){
        long pa = ra, pb = rb;
        if(pa == 0 && pb == 0){ pa = rc; pb = rd; }
        if(pa > 0 || (pa == 0 && pb > 0)) break;
        long na = -rb, nb = ra, nc = -rd, nd = rc;
        ra = na; rb = nb; rc = nc; rd = nd;
    }
    *out = (((ra + base)*2*base + (rb + base))*2*base + (rc + base))*2*base + (rd + base);
    return 1;
}

/* ── E O TAMANHO DA ÓRBITA: quatro nos pontos livres, menos nos fixos ────── */
static int psi_orbita(Psi p){
    if(psi_nulo(p)) return 0;
    Psi t = psi_roda(p);
    int n = 1;
    while(!(t.a == p.a && t.b == p.b && t.c == p.c && t.d == p.d) && n < 4){
        t = psi_roda(t); n++;
    }
    return n;
}

#endif /* QUANTICO_H */
