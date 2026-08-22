/* calculo2.h — CÁLCULO II EXACTO: séries, várias variáveis, e a borda.
 *
 *  ordem do coordenador sobe para o Cálculo II e acrescenta uma coisa nova à espinha:
 *
 *        LOCAL  →  GLOBAL
 *
 * «porque é justamente isso que começa a aparecer quando passamos de derivadas locais
 * para integrais, séries, campos e teoremas de borda». Tem razão, e é o eixo do andar.
 *
 * ── O QUE MUDA EM RELAÇÃO AO CÁLCULO I ──────────────────────────────────────────
 * No Cálculo I a derivada de um polinómio era exacta porque o quociente de diferenças É
 * um polinómio. Aqui aparecem objectos que NÃO são finitos — séries — e a régua da casa
 * obriga a dizer o que se faz com eles:
 *
 *   · uma SÉRIE não se avalia: manipula-se como SÉRIE FORMAL, coeficiente a
 *     coeficiente, exactamente como o `dirichlet.h` já faz para as de Dirichlet.
 *     A soma existe como objecto algébrico antes de qualquer limite.
 *   · a CONVERGÊNCIA não se estima: prova-se por COMPARAÇÃO com uma série cuja soma
 *     parcial se sabe em forma fechada, e as duas comparam-se em ℚ, exactamente.
 *   · e onde a soma tem FORMA FECHADA (geométrica, telescópica), usa-se a forma
 *     fechada — que é a regra que esta casa já tinha: a série custa e dá lixo, a forma
 *     fechada custa nada e dá o valor exacto.
 *
 * ── A TRADUÇÃO PARA O UNIVERSAL ────────────────────────────────────────────────
 *   série formal      ↔ a convolução na árvore (`dirichlet.h`): coeficientes, nunca s
 *   somas parciais    ↔ a ESCADA: cada N é um degrau, e a filtração é o refinamento
 *   Taylor            ↔ os MOMENTOS Φ_m do observador — a função lida pelos seus
 *                       coeficientes, e o grau do observador é a ordem do resto
 *   gradiente         ↔ o DUAL: ∇f é o funcional df lido como vector pela métrica
 *   Jacobiano         ↔ Λⁿ T = det T, o factor de VOLUME (`thm:geometria`, a carta W)
 *   Hessiana          ↔ a FORMA QUADRÁTICA e a sua assinatura — caçam-se testemunhas
 *   Green/Gauss/Stokes↔ interior ↔ BORDA, que é o `thm:borda-dirac` e o par dual
 *                       δ ⊣ ε da morfologia: o operador de borda e o seu adjunto
 *
 * Tudo em Qz. Precisa de `racionais.h`, `linear.h` e `calculo.h`. */
#ifndef CALCULO2_H
#define CALCULO2_H

#include <stdint.h>
/* AS SÉRIES SAÍRAM PARA `serie.h`, e são as mesmas: quem incluía este ficheiro
 * continua a tê-las todas. A razão da mudança está lá escrita — o banco precisa
 * das séries e não das matrizes, e as matrizes deste andar chocam com as de
 * `corpos.h`. Extraiu-se em vez de se copiar. */
#include "serie.h"
/* ── VÁRIAS VARIÁVEIS: o gradiente, a jacobiana e a hessiana, EXACTOS ───────────
 * Um polinómio em duas variáveis guarda-se como matriz de coeficientes c[i][j] para
 * xⁱyʲ. As parciais são formais e exactas, e a simetria f_xy = f_yx VERIFICA-SE em vez
 * de se assumir — é o teorema de Schwarz, e aqui ele é uma identidade de coeficientes. */
#define P2_MAX 5
typedef struct { Qz c[P2_MAX+1][P2_MAX+1]; } P2;

static P2 p2_0(void){
    P2 p;
    for(int i = 0; i <= P2_MAX; i++) for(int j = 0; j <= P2_MAX; j++) p.c[i][j] = qz(0,1);
    return p;
}
static Qz p2_av(P2 p, Qz x, Qz y){
    Qz s = qz(0,1), px = qz(1,1);
    for(int i = 0; i <= P2_MAX; i++){
        Qz py = qz(1,1);
        for(int j = 0; j <= P2_MAX; j++){
            s = qz_soma(s, qz_mult(p.c[i][j], qz_mult(px, py)));
            py = qz_mult(py, y);
        }
        px = qz_mult(px, x);
    }
    return s;
}
static P2 p2_dx(P2 p){
    P2 r = p2_0();
    for(int i = 1; i <= P2_MAX; i++) for(int j = 0; j <= P2_MAX; j++)
        r.c[i-1][j] = qz_mult(qz_de_inteiro(i), p.c[i][j]);
    return r;
}
static P2 p2_dy(P2 p){
    P2 r = p2_0();
    for(int i = 0; i <= P2_MAX; i++) for(int j = 1; j <= P2_MAX; j++)
        r.c[i][j-1] = qz_mult(qz_de_inteiro(j), p.c[i][j]);
    return r;
}
static int p2_igual(P2 a, P2 b){
    for(int i = 0; i <= P2_MAX; i++) for(int j = 0; j <= P2_MAX; j++)
        if(!qz_igual(a.c[i][j], b.c[i][j])) return 0;
    return 1;
}
/* a HESSIANA num ponto — e ela é uma Mat, para o andar do espectro a poder ler */
static Mat p2_hessiana(P2 p, Qz x, Qz y){
    Mat H = mat0(2,2);
    H.a[0][0] = p2_av(p2_dx(p2_dx(p)), x, y);
    H.a[0][1] = p2_av(p2_dy(p2_dx(p)), x, y);
    H.a[1][0] = p2_av(p2_dx(p2_dy(p)), x, y);
    H.a[1][1] = p2_av(p2_dy(p2_dy(p)), x, y);
    return H;
}
static Vec p2_gradiente(P2 p, Qz x, Qz y){
    Vec g = vec0(2);
    g.c[0] = p2_av(p2_dx(p), x, y);
    g.c[1] = p2_av(p2_dy(p), x, y);
    return g;
}
/* ── A CLASSIFICAÇÃO PELO ESPECTRO: mínimo, máximo ou SELA ──────────────────────
 * Para a hessiana 2×2 simétrica, det e traço decidem — e o det < 0 é a SELA, que é o
 * caso indefinido. Não se lê a matriz: usa-se o critério, e as testemunhas caçam-se
 * como no andar das formas quadráticas. Devolve +1 mínimo, −1 máximo, 0 sela, 2 indeciso. */
static int p2_classifica(Mat H){
    Qz d = mat_det(H);
    if(d.p < 0) return 0;                            /* indefinida: SELA */
    if(d.p == 0) return 2;                           /* degenerada: o critério NÃO decide */
    return H.a[0][0].p > 0 ? 1 : -1;
}
/* ── O JACOBIANO: o factor de VOLUME, exacto ────────────────────────────────────
 * Para a mudança (u,v) ↦ (x(u,v), y(u,v)), o factor é |det J|. E é o mesmo determinante
 * do andar exterior — Λⁿ T = det T, «a acção do operador no volume». */
static Mat p2_jacobiana(P2 X, P2 Y, Qz u, Qz v){
    Mat J = mat0(2,2);
    J.a[0][0] = p2_av(p2_dx(X), u, v);  J.a[0][1] = p2_av(p2_dy(X), u, v);
    J.a[1][0] = p2_av(p2_dx(Y), u, v);  J.a[1][1] = p2_av(p2_dy(Y), u, v);
    return J;
}
/* ── FUBINI e GREEN, exactos em polinómios ──────────────────────────────────────
 * A integral dupla de um polinómio num rectângulo faz-se pelas primitivas em cada
 * variável, e as duas ordens têm de dar o MESMO — é Fubini medido, não citado.
 * E Green liga o interior à borda: ∮(P dx + Q dy) = ∬(Q_x − P_y) dA. Aqui o rectângulo
 * é a região, a borda são os quatro lados, e os dois lados calculam-se à parte. */
/* a primitiva de x^k entre a e b, exacta */
static Qz p2_pot_int(Qz a, Qz b, int k){
    Qz pb = qz(1,1), pa = qz(1,1), r;
    for(int t = 0; t <= k; t++){ pb = qz_mult(pb, b); pa = qz_mult(pa, a); }
    if(!qz_divide(qz_soma(pb, qz_oposto(pa)), qz_de_inteiro(k+1), &r)) c2_estouros++;
    return r;
}
/* ∫dy e DEPOIS ∫dx: o interior colapsa y e deixa um polinómio em x, que se integra */
static Qz p2_int_dy_dx(P2 p, Qz x0, Qz x1, Qz y0, Qz y1){
    Qz interno[P2_MAX+1];                      /* o coeficiente de xⁱ depois de ∫dy */
    for(int i = 0; i <= P2_MAX; i++){
        interno[i] = qz(0,1);
        for(int j = 0; j <= P2_MAX; j++)
            interno[i] = qz_soma(interno[i], qz_mult(p.c[i][j], p2_pot_int(y0,y1,j)));
    }
    Qz s = qz(0,1);
    for(int i = 0; i <= P2_MAX; i++)
        s = qz_soma(s, qz_mult(interno[i], p2_pot_int(x0,x1,i)));
    return s;
}
/* ∫dx e DEPOIS ∫dy: o interior colapsa x e deixa um polinómio em y. Objecto intermédio
 * DIFERENTE do de cima — e é por isso que a comparação dos dois É Fubini, e não uma
 * tautologia com um parâmetro ignorado. */
static Qz p2_int_dx_dy(P2 p, Qz x0, Qz x1, Qz y0, Qz y1){
    Qz interno[P2_MAX+1];                      /* o coeficiente de yʲ depois de ∫dx */
    for(int j = 0; j <= P2_MAX; j++){
        interno[j] = qz(0,1);
        for(int i = 0; i <= P2_MAX; i++)
            interno[j] = qz_soma(interno[j], qz_mult(p.c[i][j], p2_pot_int(x0,x1,i)));
    }
    Qz s = qz(0,1);
    for(int j = 0; j <= P2_MAX; j++)
        s = qz_soma(s, qz_mult(interno[j], p2_pot_int(y0,y1,j)));
    return s;
}
/* ── GREEN: o interior contra a BORDA ───────────────────────────────────────────
 * ∮_{∂R}(P dx + Q dy) = ∬_R (Q_x − P_y) dA. No rectângulo a borda são quatro segmentos,
 * percorridos no sentido positivo, e cada um é uma integral de UMA variável com a outra
 * congelada. Os dois lados constroem-se por caminhos completamente distintos: um soma
 * quatro linhas, o outro integra uma área. Terem de bater é o teorema. */
static Qz p2_linha_x(P2 P, Qz x0, Qz x1, Qz yfix){    /* ∫ P(x,yfix) dx */
    Qz s = qz(0,1);
    for(int i = 0; i <= P2_MAX; i++){
        Qz coef = qz(0,1), py = qz(1,1);
        for(int j = 0; j <= P2_MAX; j++){
            coef = qz_soma(coef, qz_mult(P.c[i][j], py));
            py = qz_mult(py, yfix);
        }
        s = qz_soma(s, qz_mult(coef, p2_pot_int(x0,x1,i)));
    }
    return s;
}
static Qz p2_linha_y(P2 Q, Qz y0, Qz y1, Qz xfix){    /* ∫ Q(xfix,y) dy */
    Qz s = qz(0,1);
    for(int j = 0; j <= P2_MAX; j++){
        Qz coef = qz(0,1), px = qz(1,1);
        for(int i = 0; i <= P2_MAX; i++){
            coef = qz_soma(coef, qz_mult(Q.c[i][j], px));
            px = qz_mult(px, xfix);
        }
        s = qz_soma(s, qz_mult(coef, p2_pot_int(y0,y1,j)));
    }
    return s;
}
/* a borda do rectângulo no sentido positivo: baixo →, direita ↑, topo ←, esquerda ↓ */
static Qz p2_green_borda(P2 P, P2 Q, Qz x0, Qz x1, Qz y0, Qz y1){
    Qz s = qz(0,1);
    s = qz_soma(s, p2_linha_x(P, x0, x1, y0));        /* baixo, dx de x0 a x1 */
    s = qz_soma(s, p2_linha_y(Q, y0, y1, x1));        /* direita, dy de y0 a y1 */
    s = qz_soma(s, p2_linha_x(P, x1, x0, y1));        /* topo, dx de x1 a x0 */
    s = qz_soma(s, p2_linha_y(Q, y1, y0, x0));        /* esquerda, dy de y1 a y0 */
    return s;
}
static Qz p2_green_area(P2 P, P2 Q, Qz x0, Qz x1, Qz y0, Qz y1){
    P2 rot = p2_0();                                   /* Q_x − P_y */
    P2 qx = p2_dx(Q), py = p2_dy(P);
    for(int i = 0; i <= P2_MAX; i++) for(int j = 0; j <= P2_MAX; j++)
        rot.c[i][j] = qz_soma(qx.c[i][j], qz_oposto(py.c[i][j]));
    return p2_int_dy_dx(rot, x0, x1, y0, y1);
}
#endif
