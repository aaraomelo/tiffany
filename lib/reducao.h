/* reducao.h — A PONTE ENTRE AS DUAS FACES: ℚ → ℙ¹(𝔽₁₂₇), e ela REFUTA.
 *
 * O Teorema do Gato diz que a lei é universal e a face é da instância. 𝔽₁₂₇ é uma face
 * onde tudo é exaustível e nada tem ramos; ℚ é a face onde a matemática desta casa vive.
 * Este ficheiro é a ponte, e ela serve para uma coisa precisa:
 *
 *      𝔽₁₂₇ NÃO PROVA NADA SOBRE ℚ.   MAS REFUTA.
 *
 * Porque a redução é um HOMOMORFISMO: se uma identidade vale em ℚ, a sua redução vale em
 * 𝔽₁₂₇. Logo se FALHA em 𝔽₁₂₇, era falsa em ℚ — e falha-se lá em milissegundos, sobre
 * TODOS os casos, sem um ramo e sem nada crescer.
 *
 * É o refutador exaustivo que esta casa não tinha: barato, completo, e honesto sobre o
 * que pode dizer. Provar continua a ser trabalho de ℚ; DESMENTIR passa a ser trabalho de
 * 𝔽₁₂₇, e é onde o desmentido é mais barato.
 *
 * ── E A REDUÇÃO É TOTAL, o que só é possível por causa do ∞ ────────────────────
 *      [p : q]  ↦  [p mod 127 : q mod 127]
 *
 * Quando 127 | q o denominador anula-se, e em ℚ isso seria «não se pode reduzir». Aqui
 * não é: dá [x : 0] = ∞, que é o ponto certo — é o POLO. A redução é total porque o
 * contradomínio tem o infinito, e é o Corolário 0 ↔ ∞ a trabalhar num sítio novo.
 *
 * O único par que ela recusa é [0:0], e esse não é ponto de partida nem de chegada.
 *
 * Precisa de `racionais.h` e `sem_ramo.h`. */
#ifndef REDUCAO_H
#define REDUCAO_H

#include "sem_ramo.h"

static long rd_recusas = 0;         /* os [0:0] — e mede-se que nunca vêm de um racional */

/* ── A REDUÇÃO, e não tem um `if` sobre o valor: só o domínio ──────────────────*/
static int rd_de_qz(Qz x, Pr *r){
    int p = x.p % (int)SR_P, q = x.q % (int)SR_P;
    p = (p + (int)SR_P) % (int)SR_P;
    q = (q + (int)SR_P) % (int)SR_P;
    if(p == 0 && q == 0){ rd_recusas++; return 0; }   /* [0:0] não é ponto */
    *r = sr_pt((Fp)p, (Fp)q);
    return 1;
}
/* e o teste que interessa: 127 divide o denominador? aí a redução dá o POLO */
static int rd_e_polo(Qz x){ return (x.q % (int)SR_P) == 0; }

/* ── O QUE A PONTE PERMITE: refutar uma identidade sem a provar ────────────────
 * Duas quantidades de ℚ são iguais? Se as reduções diferirem, elas diferem. Se
 * coincidirem, não se sabe — e é preciso dizê-lo, senão a ponte vira uma mentira.
 * Devolve: 1 REFUTOU (são diferentes de certeza), 0 não refutou, −1 não aplicável. */
static int rd_refuta(Qz a, Qz b){
    Pr ra, rb;
    if(!rd_de_qz(a, &ra) || !rd_de_qz(b, &rb)) return -1;
    return sr_igual(ra, rb) ? 0 : 1;
}
/* e a mesma coisa para uma família: refuta se ALGUM par diverge */
static int rd_refuta_lista(const Qz *a, const Qz *b, int n, int *onde){
    for(int i = 0; i < n; i++){
        int r = rd_refuta(a[i], b[i]);
        if(r == 1){ if(onde) *onde = i; return 1; }
    }
    return 0;
}
#endif
