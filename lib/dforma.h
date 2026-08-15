/* dforma.h — A DERIVADA EXTERIOR: Λ⁰ →d Λ¹ →d Λ² →d Λ³, com d² = 0.
 *
 * O `eval.txt` aponta o andar pelo nome, e diz porquê: «aí vocês fecham o buraco que
 * ficou aberto no Cálculo III e, pela primeira vez, o directo/cruzado, exterior,
 * estrela, contração e borda podem entrar TODOS NA MESMA LINGUAGEM».
 *
 * ── O QUE O d FAZ, E É TODO O ANDAR ────────────────────────────────────────────
 * Em ℝ³, dim Λᵏ = 1, 3, 3, 1 — e por isso uma forma cabe sempre em três coeficientes.
 * A derivada exterior é UMA operação, e em cada grau ela já tinha outro nome:
 *
 *        d em Λ⁰  =  GRAD      (f  ↦  ∂f/∂x, ∂f/∂y, ∂f/∂z)
 *        d em Λ¹  =  ROT       (as combinações que dão o rotacional)
 *        d em Λ²  =  DIV       (a soma das três parciais)
 *        d em Λ³  =  0         (não há grau 4 em ℝ³)
 *
 * Não são três operadores parecidos: é O MESMO, e o que muda é o grau da forma. Isso
 * não se afirma aqui — CONSTRÓI-SE o d uma vez só e mede-se que ele COINCIDE com o
 * grad, o rot e o div já construídos no `campo.h`, coeficiente a coeficiente.
 *
 * E daí as duas identidades do Cálculo III passam a ser UMA:
 *
 *        d² = 0        que em Λ⁰ é rot(∇φ) = 0  e em Λ¹ é div(rot F) = 0.
 *
 * ── E OS TRÊS TEOREMAS PASSAM A SER UM ─────────────────────────────────────────
 *        ∫_{∂R} ω  =  ∫_R dω
 * com UMA função para a borda e UMA para o interior. Green, Stokes e Gauss obtêm-se
 * chamando o MESMO par com grau 1 e grau 2 — e é isso que se mede: não que os números
 * batam (já se sabia), mas que o CÓDIGO seja o mesmo.
 *
 * ── A MESMA LINGUAGEM PARA TUDO O QUE JÁ CÁ ESTAVA ─────────────────────────────
 *   directo/cruzado ↔ Λ¹∧Λ¹ é o cruzado (grau 2) e ⋆(α∧⋆β) é o directo (grau 0)
 *   exterior        ↔ o ∧, do andar de Λ V
 *   estrela         ↔ o ⋆, que troca Λᵏ com Λ³⁻ᵏ, e ⋆⋆ = id
 *   contração       ↔ ι_v, a antiderivação que baixa o grau
 *   borda           ↔ o ∂ de ∫_∂R ω = ∫_R dω — e ∂ é ADJUNTO de d
 *
 * Precisa de `racionais.h`, `linear.h`, `campo.h`. Tudo em Qz. */
#ifndef DFORMA_H
#define DFORMA_H

/* Uma forma de grau k em ℝ³, com coeficientes polinomiais. As bases são:
 *   grau 0:  1
 *   grau 1:  dx, dy, dz
 *   grau 2:  dy∧dz, dz∧dx, dx∧dy
 *   grau 3:  dx∧dy∧dz
 * e por isso três lugares chegam sempre. */
typedef struct { int grau; P3 c[3]; } Frm;

static long df_estouros = 0;

static Frm frm0(int grau){
    Frm w; w.grau = grau;
    for(int i = 0; i < 3; i++) w.c[i] = p3_0();
    return w;
}
static int frm_nula(Frm w){
    int n = (w.grau == 0 || w.grau == 3) ? 1 : 3;
    for(int i = 0; i < n; i++) if(!p3_nulo(w.c[i])) return 0;
    return 1;
}
static int frm_igual(Frm a, Frm b){
    if(a.grau != b.grau) return 0;
    int n = (a.grau == 0 || a.grau == 3) ? 1 : 3;
    for(int i = 0; i < n; i++)
        if(!p3_nulo(p3_soma(a.c[i], p3_neg(b.c[i])))) return 0;
    return 1;
}
static Frm frm_soma(Frm a, Frm b){
    Frm r = frm0(a.grau);
    for(int i = 0; i < 3; i++) r.c[i] = p3_soma(a.c[i], b.c[i]);
    return r;
}
static Frm frm_neg(Frm a){
    Frm r = frm0(a.grau);
    for(int i = 0; i < 3; i++) r.c[i] = p3_neg(a.c[i]);
    return r;
}
/* ── A DERIVADA EXTERIOR — UMA operação, quatro graus ───────────────────────────
 * Escrita de uma vez, e não em três casos parecidos: em cada grau ela é a combinação
 * antissimétrica das parciais que o grau permite. Que dê grad, rot e div MEDE-SE. */
static Frm frm_d(Frm w){
    if(w.grau >= 3) return frm0(3);               /* não há Λ⁴ em ℝ³ */
    Frm r = frm0(w.grau + 1);
    if(w.grau == 0){                               /* df = (∂₁f, ∂₂f, ∂₃f) */
        for(int t = 0; t < 3; t++) r.c[t] = p3_d(w.c[0], t);
    } else if(w.grau == 1){                        /* dω: as três combinações cruzadas */
        r.c[0] = p3_soma(p3_d(w.c[2],1), p3_neg(p3_d(w.c[1],2)));   /* ∂₂R − ∂₃Q */
        r.c[1] = p3_soma(p3_d(w.c[0],2), p3_neg(p3_d(w.c[2],0)));   /* ∂₃P − ∂₁R */
        r.c[2] = p3_soma(p3_d(w.c[1],0), p3_neg(p3_d(w.c[0],1)));   /* ∂₁Q − ∂₂P */
    } else {                                       /* dω = (∂₁A + ∂₂B + ∂₃C) */
        r.c[0] = p3_soma(p3_soma(p3_d(w.c[0],0), p3_d(w.c[1],1)), p3_d(w.c[2],2));
    }
    return r;
}
/* as pontes para o `campo.h`, que é o que torna a coincidência MEDÍVEL em vez de dita */
static Frm frm_de_escalar(P3 f){ Frm w = frm0(0); w.c[0] = f; return w; }
static Frm frm_de_campo(Cmp F, int grau){
    Frm w = frm0(grau);
    for(int t = 0; t < 3; t++) w.c[t] = F.f[t];
    return w;
}
static Cmp cmp_de_frm(Frm w){
    Cmp F = cmp0();
    for(int t = 0; t < 3; t++) F.f[t] = w.c[t];
    return F;
}
/* ── O PRODUTO EXTERIOR entre formas ────────────────────────────────────────────
 * Λ⁰ × Λᵏ → Λᵏ  (escalar);   Λ¹ × Λ¹ → Λ²  (o CRUZADO);   Λ¹ × Λ² → Λ³  (o DIRECTO).
 * É exactamente o mesmo ∧ do andar de Λ V, agora com coeficientes que são funções. */
static P3 p3_mult(P3 a, P3 b){
    P3 r = p3_0();
    for(int i = 0; i <= P3_MAX; i++) for(int j = 0; j <= P3_MAX; j++)
    for(int k = 0; k <= P3_MAX; k++){
        if(!a.c[i][j][k].p) continue;
        for(int u = 0; u + i <= P3_MAX; u++) for(int v = 0; v + j <= P3_MAX; v++)
        for(int t = 0; t + k <= P3_MAX; t++){
            if(!b.c[u][v][t].p) continue;
            r.c[i+u][j+v][k+t] = qz_soma(r.c[i+u][j+v][k+t],
                                         qz_mult(a.c[i][j][k], b.c[u][v][t]));
        }
    }
    return r;
}
static Frm frm_cunha(Frm a, Frm b){
    int g = a.grau + b.grau;
    if(g > 3){ df_estouros++; return frm0(3); }
    Frm r = frm0(g);
    if(a.grau == 0){ for(int t = 0; t < 3; t++) r.c[t] = p3_mult(a.c[0], b.c[t]); return r; }
    if(b.grau == 0){ for(int t = 0; t < 3; t++) r.c[t] = p3_mult(a.c[t], b.c[0]); return r; }
    if(a.grau == 1 && b.grau == 1){          /* o CRUZADO dos coeficientes */
        r.c[0] = p3_soma(p3_mult(a.c[1],b.c[2]), p3_neg(p3_mult(a.c[2],b.c[1])));
        r.c[1] = p3_soma(p3_mult(a.c[2],b.c[0]), p3_neg(p3_mult(a.c[0],b.c[2])));
        r.c[2] = p3_soma(p3_mult(a.c[0],b.c[1]), p3_neg(p3_mult(a.c[1],b.c[0])));
        return r;
    }
    if(a.grau + b.grau == 3){                /* o DIRECTO dos coeficientes */
        r.c[0] = p3_soma(p3_soma(p3_mult(a.c[0],b.c[0]), p3_mult(a.c[1],b.c[1])),
                         p3_mult(a.c[2],b.c[2]));
        return r;
    }
    df_estouros++;
    return frm0(g);
}
/* ── A ESTRELA ⋆: Λᵏ → Λ³⁻ᵏ, com os MESMOS coeficientes ─────────────────────────
 * Em ℝ³ com a métrica usual e esta escolha de bases, o ⋆ é a identidade nos
 * coeficientes: ⋆(f) = f dx∧dy∧dz, ⋆(P dx + …) = P dy∧dz + …, e ⋆⋆ = id EXACTO. É a
 * mesma estrela do andar exterior, e a mesma involução ν∘ν = id da casa. */
static Frm frm_estrela(Frm w){
    Frm r = frm0(3 - w.grau);
    for(int t = 0; t < 3; t++) r.c[t] = w.c[t];
    return r;
}
/* ── A CONTRAÇÃO ι_v: Λᵏ → Λᵏ⁻¹, a antiderivação ───────────────────────────────
 * ι_v(P dx + Q dy + R dz) = vP + vQ + vR (grau 0), e em Λ² dá o cruzado com v. */
static Frm frm_contrai(Vec v, Frm w){
    if(w.grau == 0) return frm0(0);
    Frm r = frm0(w.grau - 1);
    if(w.grau == 1){
        P3 s = p3_0();
        for(int t = 0; t < 3; t++){
            P3 c = p3_0(); c.c[0][0][0] = v.c[t];
            s = p3_soma(s, p3_mult(c, w.c[t]));
        }
        r.c[0] = s;
        return r;
    }
    if(w.grau == 2){                          /* ι_v em Λ² é o cruzado com v */
        P3 vx = p3_0(), vy = p3_0(), vz = p3_0();
        vx.c[0][0][0] = v.c[0]; vy.c[0][0][0] = v.c[1]; vz.c[0][0][0] = v.c[2];
        r.c[0] = p3_soma(p3_mult(vy, w.c[2]), p3_neg(p3_mult(vz, w.c[1])));
        r.c[1] = p3_soma(p3_mult(vz, w.c[0]), p3_neg(p3_mult(vx, w.c[2])));
        r.c[2] = p3_soma(p3_mult(vx, w.c[1]), p3_neg(p3_mult(vy, w.c[0])));
        return r;
    }
    /* grau 3 → grau 2 */
    P3 vx = p3_0(), vy = p3_0(), vz = p3_0();
    vx.c[0][0][0] = v.c[0]; vy.c[0][0][0] = v.c[1]; vz.c[0][0][0] = v.c[2];
    r.c[0] = p3_mult(vx, w.c[0]);
    r.c[1] = p3_mult(vy, w.c[0]);
    r.c[2] = p3_mult(vz, w.c[0]);
    return r;
}
/* ── O TEOREMA DE STOKES, UMA VEZ SÓ ────────────────────────────────────────────
 *        ∫_{∂R} ω  =  ∫_R dω
 * UMA função para a borda e UMA para o interior. A região é a caixa [x0,x1]×[y0,y1]×
 * [z0,z1]; quando ω tem grau 1 usa-se a FACE z = z0 e a sua borda (quatro segmentos) —
 * isso é Green/Stokes; quando tem grau 2 usa-se a caixa inteira e as seis faces — isso
 * é Gauss. Mesmo par de funções, dois graus. */
static Qz frm_int_borda(Frm w, Qz x0, Qz x1, Qz y0, Qz y1, Qz z0, Qz z1){
    if(w.grau == 1){                          /* a borda de uma FACE: quatro segmentos */
        Cmp F = cmp_de_frm(w);
        return cp_circulacao(F, x0, x1, y0, y1, z0);
    }
    if(w.grau == 2){                          /* a borda de uma CAIXA: seis faces */
        Cmp F = cmp_de_frm(w);
        return cp_fluxo_caixa(F, x0, x1, y0, y1, z0, z1);
    }
    df_estouros++;
    return qz(0,1);
}
static Qz frm_int_interior(Frm dw, Qz x0, Qz x1, Qz y0, Qz y1, Qz z0, Qz z1){
    if(dw.grau == 2)                          /* a face: só a componente normal */
        return p3_int_face(dw.c[2], 2, z0, x0,x1, y0,y1);
    if(dw.grau == 3)                          /* a caixa inteira */
        return p3_int_caixa(dw.c[0], x0,x1, y0,y1, z0,z1);
    df_estouros++;
    return qz(0,1);
}
/* e o teorema, escrito UMA VEZ: devolve 1 quando os dois lados coincidem */
static int frm_stokes(Frm w, Qz x0, Qz x1, Qz y0, Qz y1, Qz z0, Qz z1, Qz *bord, Qz *inte){
    *bord = frm_int_borda(w, x0,x1, y0,y1, z0,z1);
    *inte = frm_int_interior(frm_d(w), x0,x1, y0,y1, z0,z1);
    return qz_igual(*bord, *inte);
}
#endif
