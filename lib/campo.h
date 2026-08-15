/* campo.h — CÁLCULO III: campos, fluxo, circulação, e a BORDA.
 *
 * O `eval.txt` fecha a sequência do andar com uma frase só:
 *
 *        número → vetor → operador → campo → BORDA
 *
 * e com a observação que decide o andar: «Green, Stokes e Gauss como A MESMA ESTRUTURA
 * em dimensões diferentes — integral da borda = integral do operador no interior».
 *
 * ── O QUE ISSO OBRIGA ──────────────────────────────────────────────────────────
 * Se são a mesma estrutura, então NÃO se medem como três teoremas. Mede-se a estrutura:
 * um operador de derivação D, uma região R, e a igualdade
 *
 *        ∫_{∂R} ω  =  ∫_R Dω
 *
 * com D a mudar de nome em cada dimensão — rot em 2D (Green), rot em 3D (Stokes),
 * div em 3D (Gauss). O que se mede é que os DOIS LADOS, calculados por caminhos sem
 * nada em comum, dão o mesmo número. E o gume é o mesmo nos três: se o operador não é
 * o certo, a igualdade parte-se.
 *
 * ── E A CASA JÁ TEM A ÁLGEBRA DA UNIFICAÇÃO ────────────────────────────────────
 * Ele deixaria as FORMAS DIFERENCIAIS para depois, «porque aí Stokes deixa de ser quatro
 * teoremas separados e começa a aparecer como uma única lei». A álgebra dessa unificação
 * já cá está, do andar exterior: Λᵏ, a cunha, o Hodge ⋆, e a contração ι_v. O que falta
 * é o d, e o d é a derivada exterior — o passo seguinte, não este.
 *
 * ── A TRADUÇÃO PARA O UNIVERSAL ────────────────────────────────────────────────
 *   campo conservativo ↔ a FIBRA: F = ∇φ é «dado F, achar φ», e ela tem solução
 *                        exactamente quando rot F = 0 (numa região simples)
 *   independência do caminho ↔ a VOLTA: o trabalho não depende do percurso ⟺ a volta
 *                        fecha com resíduo 0
 *   rot e div          ↔ o par DIRECTO/CRUZADO outra vez: div é a parte simétrica
 *                        (o fluxo que atravessa) e rot é a antissimétrica (o que roda) —
 *                        corpo-estelar §640, «o torque É o produto cruzado»
 *   interior ↔ borda   ↔ o par adjunto δ ⊣ ε da morfologia (thm:morf-par) e o
 *                        thm:borda-dirac: o operador de borda tem adjunto
 *
 * Tudo em Qz. Precisa de `racionais.h`, `linear.h`, `calculo2.h`. */
#ifndef CAMPO_H
#define CAMPO_H

#define P3_MAX 4                  /* grau máximo por variável */
typedef struct { Qz c[P3_MAX+1][P3_MAX+1][P3_MAX+1]; } P3;   /* Σ c[i][j][k] xⁱyʲzᵏ */
typedef struct { P3 f[3]; } Cmp;                              /* o campo (F₁,F₂,F₃) */

static long cp_estouros = 0;

static P3 p3_0(void){
    P3 p;
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++) p.c[i][j][k] = qz(0,1);
    return p;
}
static Cmp cmp0(void){ Cmp F; for(int t = 0; t < 3; t++) F.f[t] = p3_0(); return F; }

static Qz p3_av(P3 p, Qz x, Qz y, Qz z){
    Qz s = qz(0,1), px = qz(1,1);
    for(int i = 0; i <= P3_MAX; i++){
        Qz py = qz(1,1);
        for(int j = 0; j <= P3_MAX; j++){
            Qz pz = qz(1,1);
            for(int k = 0; k <= P3_MAX; k++){
                if(p.c[i][j][k].p)
                    s = qz_soma(s, qz_mult(p.c[i][j][k], qz_mult(px, qz_mult(py, pz))));
                pz = qz_mult(pz, z);
            }
            py = qz_mult(py, y);
        }
        px = qz_mult(px, x);
    }
    return s;
}
/* as três parciais, formais e exactas */
static P3 p3_d(P3 p, int var){
    P3 r = p3_0();
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!p.c[i][j][k].p) continue;
        if(var == 0 && i > 0) r.c[i-1][j][k] = qz_soma(r.c[i-1][j][k],
                                    qz_mult(qz_de_inteiro(i), p.c[i][j][k]));
        if(var == 1 && j > 0) r.c[i][j-1][k] = qz_soma(r.c[i][j-1][k],
                                    qz_mult(qz_de_inteiro(j), p.c[i][j][k]));
        if(var == 2 && k > 0) r.c[i][j][k-1] = qz_soma(r.c[i][j][k-1],
                                    qz_mult(qz_de_inteiro(k), p.c[i][j][k]));
    }
    return r;
}
static P3 p3_soma(P3 a, P3 b){
    P3 r = p3_0();
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++) r.c[i][j][k] = qz_soma(a.c[i][j][k], b.c[i][j][k]);
    return r;
}
static P3 p3_neg(P3 a){
    P3 r = p3_0();
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++) r.c[i][j][k] = qz_oposto(a.c[i][j][k]);
    return r;
}
static int p3_nulo(P3 a){
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++) if(a.c[i][j][k].p) return 0;
    return 1;
}
/* ── DIVERGÊNCIA E ROTACIONAL — o directo e o cruzado do campo ──────────────────
 * div F = ∂₁F₁ + ∂₂F₂ + ∂₃F₃      é a parte que ATRAVESSA (o fluxo)
 * rot F = (∂₂F₃−∂₃F₂, ∂₃F₁−∂₁F₃, ∂₁F₂−∂₂F₁)   é a parte que RODA (a circulação)
 * E não é analogia: é literalmente o produto interno e o produto cruzado de ∇ com F —
 * o mesmo par directo/cruzado que fecha em Lagrange. */
static P3 cp_div(Cmp F){
    return p3_soma(p3_soma(p3_d(F.f[0],0), p3_d(F.f[1],1)), p3_d(F.f[2],2));
}
static Cmp cp_rot(Cmp F){
    Cmp R = cmp0();
    R.f[0] = p3_soma(p3_d(F.f[2],1), p3_neg(p3_d(F.f[1],2)));
    R.f[1] = p3_soma(p3_d(F.f[0],2), p3_neg(p3_d(F.f[2],0)));
    R.f[2] = p3_soma(p3_d(F.f[1],0), p3_neg(p3_d(F.f[0],1)));
    return R;
}
/* as duas identidades que NÃO se postulam: rot(∇φ) = 0 e div(rot F) = 0 */
static Cmp cp_grad(P3 phi){
    Cmp G = cmp0();
    for(int t = 0; t < 3; t++) G.f[t] = p3_d(phi, t);
    return G;
}
/* ── A INTEGRAL, exacta: ∫x^k de a a b ──────────────────────────────────────── */
static Qz cp_pot_int(Qz a, Qz b, int k){
    Qz pb = qz(1,1), pa = qz(1,1), r;
    for(int t = 0; t <= k; t++){ pb = qz_mult(pb, b); pa = qz_mult(pa, a); }
    if(!qz_divide(qz_soma(pb, qz_oposto(pa)), qz_de_inteiro(k+1), &r)) cp_estouros++;
    return r;
}
/* a integral tripla de um P3 numa CAIXA [x0,x1]×[y0,y1]×[z0,z1] — separável e exacta */
static Qz p3_int_caixa(P3 p, Qz x0, Qz x1, Qz y0, Qz y1, Qz z0, Qz z1){
    Qz s = qz(0,1);
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!p.c[i][j][k].p) continue;
        Qz t = qz_mult(p.c[i][j][k], cp_pot_int(x0,x1,i));
        t = qz_mult(t, cp_pot_int(y0,y1,j));
        t = qz_mult(t, cp_pot_int(z0,z1,k));
        s = qz_soma(s, t);
    }
    return s;
}
/* a integral dupla numa FACE, com uma variável CONGELADA */
static Qz p3_int_face(P3 p, int fixo, Qz vfixo,
                      Qz a0, Qz a1, Qz b0, Qz b1){
    /* fixo = 0 congela x (integra em y,z); 1 congela y (x,z); 2 congela z (x,y) */
    Qz s = qz(0,1);
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!p.c[i][j][k].p) continue;
        Qz pot = qz(1,1);
        int e1, e2;
        if(fixo == 0){ for(int t = 0; t < i; t++) pot = qz_mult(pot, vfixo); e1 = j; e2 = k; }
        else if(fixo == 1){ for(int t = 0; t < j; t++) pot = qz_mult(pot, vfixo); e1 = i; e2 = k; }
        else { for(int t = 0; t < k; t++) pot = qz_mult(pot, vfixo); e1 = i; e2 = j; }
        Qz t = qz_mult(p.c[i][j][k], pot);
        t = qz_mult(t, cp_pot_int(a0,a1,e1));
        t = qz_mult(t, cp_pot_int(b0,b1,e2));
        s = qz_soma(s, t);
    }
    return s;
}
/* ── GAUSS: o FLUXO pela borda contra a DIVERGÊNCIA no interior ─────────────────
 * A caixa tem seis faces; em cada uma a normal exterior é ±eᵢ, e o fluxo é a integral
 * da componente correspondente com o sinal certo. Nada disto partilha código com a
 * integral tripla — são dois cálculos independentes, e é isso que faz a comparação
 * ser o teorema. */
static Qz cp_fluxo_caixa(Cmp F, Qz x0, Qz x1, Qz y0, Qz y1, Qz z0, Qz z1){
    Qz s = qz(0,1);
    /* x = x1 com n = +e₁ ; x = x0 com n = −e₁ */
    s = qz_soma(s, p3_int_face(F.f[0], 0, x1, y0,y1, z0,z1));
    s = qz_soma(s, qz_oposto(p3_int_face(F.f[0], 0, x0, y0,y1, z0,z1)));
    /* y = y1 com n = +e₂ ; y = y0 com n = −e₂ */
    s = qz_soma(s, p3_int_face(F.f[1], 1, y1, x0,x1, z0,z1));
    s = qz_soma(s, qz_oposto(p3_int_face(F.f[1], 1, y0, x0,x1, z0,z1)));
    /* z = z1 com n = +e₃ ; z = z0 com n = −e₃ */
    s = qz_soma(s, p3_int_face(F.f[2], 2, z1, x0,x1, y0,y1));
    s = qz_soma(s, qz_oposto(p3_int_face(F.f[2], 2, z0, x0,x1, y0,y1)));
    return s;
}
/* ── A INTEGRAL DE LINHA num segmento paralelo a um eixo ────────────────────────
 * ∫ Fᵢ dxᵢ com as outras duas congeladas. Os quatro lados de um rectângulo montam-se
 * disto, e o sentido positivo dá os sinais. */
static Qz cp_linha_eixo(P3 comp, int eixo, Qz t0, Qz t1, Qz u, Qz v){
    /* eixo = 0: integra em x com y = u, z = v; 1: em y com x = u, z = v; 2: em z com x = u, y = v */
    Qz s = qz(0,1);
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!comp.c[i][j][k].p) continue;
        Qz pu = qz(1,1), pv = qz(1,1);
        int e;
        if(eixo == 0){ for(int t=0;t<j;t++) pu = qz_mult(pu,u);
                       for(int t=0;t<k;t++) pv = qz_mult(pv,v); e = i; }
        else if(eixo == 1){ for(int t=0;t<i;t++) pu = qz_mult(pu,u);
                            for(int t=0;t<k;t++) pv = qz_mult(pv,v); e = j; }
        else { for(int t=0;t<i;t++) pu = qz_mult(pu,u);
               for(int t=0;t<j;t++) pv = qz_mult(pv,v); e = k; }
        Qz t = qz_mult(comp.c[i][j][k], qz_mult(pu,pv));
        s = qz_soma(s, qz_mult(t, cp_pot_int(t0,t1,e)));
    }
    return s;
}
/* ── STOKES: a CIRCULAÇÃO na borda contra o ROTACIONAL na superfície ────────────
 * A superfície é o rectângulo z = zc, [x0,x1]×[y0,y1], com normal +e₃. A borda são os
 * quatro lados no sentido positivo. */
static Qz cp_circulacao(Cmp F, Qz x0, Qz x1, Qz y0, Qz y1, Qz zc){
    Qz s = qz(0,1);
    s = qz_soma(s, cp_linha_eixo(F.f[0], 0, x0, x1, y0, zc));   /* baixo:   dx, y=y0 */
    s = qz_soma(s, cp_linha_eixo(F.f[1], 1, y0, y1, x1, zc));   /* direita: dy, x=x1 */
    s = qz_soma(s, cp_linha_eixo(F.f[0], 0, x1, x0, y1, zc));   /* topo:    dx, y=y1 */
    s = qz_soma(s, cp_linha_eixo(F.f[1], 1, y1, y0, x0, zc));   /* esquerda:dy, x=x0 */
    return s;
}
static Qz cp_rot_face(Cmp F, Qz x0, Qz x1, Qz y0, Qz y1, Qz zc){
    Cmp R = cp_rot(F);
    return p3_int_face(R.f[2], 2, zc, x0,x1, y0,y1);            /* (rot F)·e₃ */
}
/* ── O CAMPO CONSERVATIVO, e a FIBRA que o define ──────────────────────────────
 * F = ∇φ é «dado F, achar φ» — uma fibra. Constrói-se φ integrando F₁ em x, e depois
 * corrigindo com o que falta em y e z. Se a construção fechar, F é conservativo; se não
 * fechar, não é — e a recusa é o resultado, não uma falha. */
static int cp_potencial(Cmp F, P3 *phi){
    if(!p3_nulo(cp_rot(F).f[0]) || !p3_nulo(cp_rot(F).f[1])
       || !p3_nulo(cp_rot(F).f[2])) return 0;         /* rot ≠ 0: a fibra é VAZIA */
    P3 p = p3_0();
    /* ∫F₁ dx: sobe o índice de x e divide */
    for(int i = 0; i <= P3_MAX-1; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!F.f[0].c[i][j][k].p) continue;
        if(!qz_divide(F.f[0].c[i][j][k], qz_de_inteiro(i+1), &p.c[i+1][j][k]))
            { cp_estouros++; return 0; }
    }
    /* o que falta em y: ∫(F₂ − ∂φ/∂y) dy, com o termo já sem x */
    P3 falta = p3_soma(F.f[1], p3_neg(p3_d(p, 1)));
    for(int j = 0; j <= P3_MAX-1; j++) for(int k = 0; k <= P3_MAX; k++){
        if(!falta.c[0][j][k].p) continue;
        Qz q;
        if(!qz_divide(falta.c[0][j][k], qz_de_inteiro(j+1), &q)){ cp_estouros++; return 0; }
        p.c[0][j+1][k] = qz_soma(p.c[0][j+1][k], q);
    }
    /* e o que falta em z */
    P3 f2 = p3_soma(F.f[2], p3_neg(p3_d(p, 2)));
    for(int k = 0; k <= P3_MAX-1; k++){
        if(!f2.c[0][0][k].p) continue;
        Qz q;
        if(!qz_divide(f2.c[0][0][k], qz_de_inteiro(k+1), &q)){ cp_estouros++; return 0; }
        p.c[0][0][k+1] = qz_soma(p.c[0][0][k+1], q);
    }
    *phi = p;
    return 1;
}
/* a VOLTA: ∇φ tem de devolver F exactamente. Sem esta verificação, «achei um potencial»
 * não vale nada — é a construção a ser conferida contra o que devia produzir. */
static int cp_confere_potencial(Cmp F, P3 phi){
    Cmp G = cp_grad(phi);
    for(int t = 0; t < 3; t++)
        if(!p3_nulo(p3_soma(G.f[t], p3_neg(F.f[t])))) return 0;
    return 1;
}
#endif
