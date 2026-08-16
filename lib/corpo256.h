/* corpo256.h — O ESPAÇO: as oito leis são a BASE, e o byte é um CORPO de 256 elementos.
 *
 * O Aarão: «isso é a base de um espaço — definir o operador e os duais, soma e
 * multiplicação nesse espaço, mostrar que é completo e ordenado, aí temos a operação
 * completa.»
 *
 * ── O ESPAÇO, E A BASE SÃO AS OITO LEIS ───────────────────────────────────────
 * O byte é um vector de 𝔽₂⁸ com base
 *
 *      e₀ = 1,  e₁ = x,  e₂ = x²,  …,  e₇ = x⁷
 *
 * e a posição k é a Lei k, pela declaração da arquitectura. Toda a estrutura que se segue
 * é sobre ESSA base: não é uma escolha nova, é a que já estava.
 *
 * ── A SOMA E O SEU DUAL ───────────────────────────────────────────────────────
 *      a ⊕ b = XOR, coordenada a coordenada.
 *      o dual da soma é ELA PRÓPRIA: −a = a, e o neutro é 0.
 * O grupo aditivo é (ℤ/2)⁸ — todo elemento é a sua própria volta, que é a Lei 1 lida no
 * espaço inteiro.
 *
 * ── O PRODUTO E O SEU DUAL ────────────────────────────────────────────────────
 * O produto é o dos POLINÓMIOS, reduzido módulo um irredutível de grau oito:
 *
 *      p(x) = x⁸ + x⁴ + x³ + x + 1        (0x11B)
 *
 * e o dual do produto é a INVERSA, que existe para todo a ≠ 0 por Fermat: a⁻¹ = a²⁵⁴.
 * O grau oito não é escolha: é o número de posições, logo o número de leis.
 *
 * ── O OPERADOR ────────────────────────────────────────────────────────────────
 * O operador do espaço é a MULTIPLICAÇÃO POR x — que em bits é rodar uma posição e
 * realimentar com p(x) quando o bit de topo sai:
 *
 *      x·a  =  (a << 1)  ⊕  (0x1B se o bit 7 estava aceso)
 *
 * É o gerador do ciclo das oito leis com a realimentação que o fecha. E as potências de x
 * percorrem a base: x⁰, x¹, …, x⁷ são exactamente e₀ … e₇.
 *
 * ── COMPLETO, SIM. ORDENADO, NÃO — E ISSO É UM TEOREMA ────────────────────────
 * COMPLETO no sentido algébrico: todo a ≠ 0 tem inverso, e a operação nunca sai do espaço.
 *
 * ORDENADO não, e não é uma lacuna: é impossível. Um corpo ordenado tem característica 0,
 * porque 1 > 0 obriga 1 + 1 > 0, e assim por diante — nenhuma soma de uns pode dar zero.
 * Aqui 1 + 1 = 0. Logo NÃO EXISTE ordem compatível com as operações, e dizer o contrário
 * seria inventar.
 *
 * O que EXISTE, e é o que se mede: o grupo multiplicativo é CÍCLICO de ordem 255, logo
 * há um gerador g e todo a ≠ 0 é g^k para um k único — o LOGARITMO DISCRETO. Isso dá uma
 * enumeração canónica de 1 a 255, que é uma ordem no sentido de «percurso», e não no
 * sentido de «compatível com +». São coisas diferentes e nomeiam-se à parte.
 *
 * Precisa de `stdint.h`. */
#ifndef CORPO256_H
#define CORPO256_H

#include <stdint.h>

#define C6_POLI 0x1Bu                  /* x⁸+x⁴+x³+x+1, sem o bit 8 */
#define C6_N    256

typedef uint8_t E;                     /* um elemento do corpo — e um byte */

/* ── A SOMA: o XOR, e o dual dela é ela própria ───────────────────────────────*/
static E c6_som(E a, E b){ return (E)(a ^ b); }
static E c6_opo(E a){ return a; }              /* −a = a */
static E c6_zero(void){ return 0; }
static E c6_um(void){ return 1; }

/* ── O OPERADOR: ×x, que é rodar e realimentar ────────────────────────────────*/
static E c6_x(E a){
    unsigned t = (unsigned)a << 1;
    unsigned alto = (a >> 7) & 1u;
    return (E)((t ^ (alto * C6_POLI)) & 0xFFu);   /* sem ramo: o bit multiplica */
}
/* ── O PRODUTO: o russo camponês em 𝔽₂ — somar os deslocamentos ───────────────*/
static E c6_mul(E a, E b){
    E r = 0;
    for(int i = 0; i < 8; i++){
        E m = (E)(0u - (E)((b >> i) & 1u));       /* máscara: 0x00 ou 0xFF, sem ramo */
        r = (E)(r ^ (a & m));
        a = c6_x(a);
    }
    return r;
}
/* ── O DUAL DO PRODUTO: a inversa por Fermat, a⁻¹ = a²⁵⁴ ─────────────────────
 * A cadeia é fixa e escrita em linha recta, tal como no `sem_ramo.h`: sem laço com teste.
 * 254 = 11111110₂. E o zero devolve zero — não por excepção, mas porque é o que a
 * potência dá; o ∞ dele vive em ℙ¹, e é o Corolário 0 ↔ ∞ outra vez. */
static E c6_inv(E a){
    E a2   = c6_mul(a, a);
    E a3   = c6_mul(a2, a);
    E a6   = c6_mul(a3, a3);
    E a7   = c6_mul(a6, a);
    E a14  = c6_mul(a7, a7);
    E a15  = c6_mul(a14, a);
    E a30  = c6_mul(a15, a15);
    E a31  = c6_mul(a30, a);
    E a62  = c6_mul(a31, a31);
    E a63  = c6_mul(a62, a);
    E a126 = c6_mul(a63, a63);
    E a127 = c6_mul(a126, a);
    return c6_mul(a127, a127);                    /* a²⁵⁴ */
}
/* ── A BASE: as potências de x são e₀ … e₇, e a posição k é a Lei k ───────────*/
static E c6_base(int k){
    E r = 1;
    for(int i = 0; i < k; i++) r = c6_x(r);
    return r;
}
/* ── A ORDEM QUE EXISTE: o logaritmo discreto num gerador ────────────────────
 * O grupo multiplicativo é cíclico de ordem 255. Devolve a ordem de g (255 se for
 * gerador) e, se `log` não for nulo, preenche a tabela do logaritmo. */
static int c6_ordem(E g, int *log256){
    E p = g;
    for(int k = 1; k <= 255; k++){
        if(log256) log256[p] = k % 255;
        if(p == 1) return k;
        p = c6_mul(p, g);
    }
    return 0;
}
/* ── E A IMPOSSIBILIDADE DA ORDEM DE CORPO, exibida ───────────────────────────
 * Num corpo ordenado, 1 > 0 força 1 + 1 > 0. Aqui 1 + 1 = 0. Devolve a soma de n uns —
 * e ela é 0 para n par, o que é o contra-exemplo. */
static E c6_soma_uns(int n){
    E s = 0;
    for(int i = 0; i < n; i++) s = c6_som(s, 1);
    return s;
}
#endif
