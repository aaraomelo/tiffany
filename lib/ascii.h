/* ascii.h — A REALIZAÇÃO ASCII: 128 códigos são os 128 pontos de ℙ¹(𝔽₁₂₇).
 *
 * O Aarão: «vamos tabelar ASCII e formalizar a realização ASCII da álgebra universal».
 *
 * ── O ENCAIXE, E ELE NÃO FOI PROCURADO ────────────────────────────────────────
 * O ASCII tem 128 códigos, de 0 a 127. E |ℙ¹(𝔽₁₂₇)| = 127 + 1 = 128. São o mesmo número,
 * e pela mesma razão: 127 é primo, é o topo do `int8_t`, e o ASCII foi desenhado para
 * caber em sete bits.
 *
 *      o código c ∈ {0, …, 126}  ↦  o ponto [c : 1]     os finitos
 *      o código 127 (DEL)        ↦  o ponto [1 : 0]     o ∞
 *
 * O DEL a ser o infinito não é uma escolha bonita: é o único código que sobra depois de
 * os 127 finitos estarem atribuídos, e é o último. A tabela fecha sozinha.
 *
 * ── E O ASCII TEM DUAS ÁLGEBRAS AO MESMO TEMPO ────────────────────────────────
 * O mesmo conjunto de 128 objectos carrega duas estruturas diferentes:
 *
 *      como ℙ¹(𝔽₁₂₇)   o byte é UM PONTO, e o gato é uma PERMUTAÇÃO dos caracteres
 *      como 𝔽₂⁷        o byte é um VECTOR de sete bits, e a soma é o XOR
 *
 * E elas NÃO são compatíveis: o gato permuta os pontos e não respeita o XOR. Isso não é
 * um defeito — é o conteúdo. Duas álgebras sobre o mesmo suporte, e cada operação só faz
 * sentido numa. Confundi-las é o erro que esta tabela existe para tornar impossível.
 *
 * ── O QUE ISTO DÁ ─────────────────────────────────────────────────────────────
 * O gato A_m lido em ASCII é uma CIFRA: uma bijecção das 128 letras, sem tabela e sem
 * chave — só a lei. E o esquilo desfá-la, porque é a acção à direita.
 *
 * Precisa de `sem_ramo.h`. */
#ifndef ASCII_H
#define ASCII_H

#include "sem_ramo.h"

#define AS_N 128                          /* os códigos do ASCII, e os pontos de ℙ¹ */
#define AS_INF 127                        /* o DEL — o ponto no infinito */

/* o código ASCII como ponto de ℙ¹(𝔽₁₂₇), sem normalizar */
static Pr as_ponto(int c){
    /* c = 127 dá [1:0] = ∞; os outros dão [c:1]. Sem ramo sobre o valor: a própria
     * aritmética o faz, porque 127 ≡ 0 e o denominador anula-se. */
    return sr_pt((Fp)(c == AS_INF ? 1 : c), (Fp)(c == AS_INF ? 0 : 1));
}
/* e a volta: o ponto ao código, que é a NORMALIZAÇÃO — logo tem o ramo, e é para
 * IMPRIMIR, tal como o `sem_ramo.h` já dizia. */
static int as_codigo(Pr x){
    if(sr_e_inf(x)) return AS_INF;
    return (int)sr_indice(x);
}
/* o gato como CIFRA sobre o ASCII: uma permutação das 128 letras */
static int as_gato(int m, int c){ return as_codigo(sr_gato((Fp)m, as_ponto(c))); }
/* a involução ν(x) = −1/x sobre o ASCII */
static int as_nu(int c){ return as_codigo(sr_nu(as_ponto(c))); }
/* e a inversão pura: a troca, que leva o 0 no DEL e o DEL no 0 */
static int as_inverte(int c){ return as_codigo(sr_inverte(as_ponto(c))); }

/* ── A OUTRA ÁLGEBRA: o byte como vector de 𝔽₂⁷, e a soma é o XOR ─────────────*/
static int as_xor(int a, int b){ return (a ^ b) & 0x7F; }
static int as_peso(int c){
    int n = 0;
    for(int i = 0; i < 7; i++) n += (c >> i) & 1;
    return n;
}
/* o nome legível de um código, para a tabela sair lida por gente */
static const char *as_nome(int c, char *buf){
    static const char *ctrl[33] = {
        "NUL","SOH","STX","ETX","EOT","ENQ","ACK","BEL","BS","HT","LF","VT","FF","CR",
        "SO","SI","DLE","DC1","DC2","DC3","DC4","NAK","SYN","ETB","CAN","EM","SUB","ESC",
        "FS","GS","RS","US","SP" };
    if(c >= 0 && c <= 32) return ctrl[c];
    if(c == 127) return "DEL";
    buf[0] = (char)c; buf[1] = 0;
    return buf;
}
#endif
