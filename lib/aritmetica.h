/* aritmetica.h — A ARITMÉTICA NATURAL, e a fibra que cada andar torna total.
 *
 * O corolário anterior mostrou que «0⁻¹ não existe» era da CARTA e não do objecto: em ℙ¹
 * a inversão é total, e o preço é a soma deixar de o ser. Este ficheiro leva a mesma
 * pergunta ao andar de baixo, e a resposta é a mesma frase lida ao contrário:
 *
 *      em ℕ    a SOMA é total          e a SUBTRACÇÃO é parcial (a + x = b só se b ≥ a)
 *      em ℤ    a subtracção fica total  e paga-se com o sinal
 *      em ℚ    a divisão fica total     excepto no zero
 *      em ℙ¹   a inversão fica total    e a soma deixa de o ser
 *
 * ── E ENTÃO A ESCADA TEM UMA LEI, E NÃO É UMA LISTA ───────────────────────────
 * Cada andar torna UMA fibra total, e paga noutra. Não há andar onde tudo seja total, e
 * o «0⁻¹ não existe» nunca foi a excepção da escada: era a fibra que estava a ser paga
 * NAQUELE andar. Dizer isto muda o estatuto de todas as excepções que esta casa escreveu
 * — deixam de ser defeitos e passam a ser o preço, que é uma quantidade e se conta.
 *
 * ── A ARITMÉTICA NATURAL PROPRIAMENTE ─────────────────────────────────────────
 * Aqui não há sinal e não há fracção: um racional positivo é uma SEQUÊNCIA DE NATURAIS —
 * a fracção contínua [a₀; a₁, …, aₙ] —, e as operações que interessam fazem-se sobre os
 * naturais sem nunca formar o quociente:
 *
 *   · comparar: pela sequência, alternando o sentido (é a ordem de Stern--Brocot)
 *   · aproximar: os convergentes, pela recorrência pₙ = aₙpₙ₋₁ + pₙ₋₂
 *   · e a DISTÂNCIA entre convergentes consecutivos tem FORMA FECHADA:
 *
 *          |pₙ/qₙ − pₙ₊₁/qₙ₊₁| = 1/(qₙ · qₙ₊₁)
 *
 * ── É ESTA FORMA FECHADA QUE DESBLOQUEIA A MIGRAÇÃO ───────────────────────────
 * O módulo de Cauchy desta casa procurava o N iterando a órbita e SUBTRAINDO racionais —
 * e é isso que faz os denominadores crescerem e a representação saturar. Com a forma
 * fechada, achar N para ε = a/b é achar o primeiro n com
 *
 *          qₙ · qₙ₊₁  >  b/a,
 *
 * uma comparação de NATURAIS pequenos. Nenhuma diferença de racionais é formada, nenhum
 * denominador é multiplicado por outro, e o N sai do PASSO em vez de sair da órbita —
 * que é a regra do Teorema do Gato aplicada ao sítio onde ela ainda faltava.
 *
 * Precisa de nada. Tudo em `unsigned long` sobre ℕ, e os produtos vigiados. */
#ifndef ARITMETICA_H
#define ARITMETICA_H

static long nt_saturou = 0;
#define NT_MAX 40                 /* comprimento máximo de uma fracção contínua */
#define NT_TECTO 3000000000UL     /* acima disto o produto de dois já não é seguro */

/* ── A FIBRA DA SOMA EM ℕ: a + x = b, e ela é PARCIAL ──────────────────────────
 * Devolve 1 e escreve x quando existe; 0 quando não — e o «não» aqui não é um defeito,
 * é o preço deste andar. É a mesma forma do 0⁻¹, e por isso mede-se do mesmo modo. */
static int nt_fibra_soma(unsigned long a, unsigned long b, unsigned long *x){
    if(b < a) return 0;                       /* sem fibra: em ℕ não há b − a */
    *x = b - a;
    return 1;
}
/* e a da MULTIPLICAÇÃO: a·x = b, parcial pela divisibilidade */
static int nt_fibra_mult(unsigned long a, unsigned long b, unsigned long *x){
    if(a == 0) return b == 0 ? 0 : 0;         /* 0·x = b: nenhuma ou todas — sem fibra */
    if(b % a) return 0;
    *x = b / a;
    return 1;
}
/* ── A FRACÇÃO CONTÍNUA: um racional positivo É uma sequência de naturais ──────
 * Euclides, e sai exacto. Devolve o comprimento. */
static int nt_fc(unsigned long p, unsigned long q, unsigned long *a, int max){
    int n = 0;
    while(q != 0 && n < max){
        a[n++] = p / q;
        unsigned long r = p % q;
        p = q; q = r;
    }
    return n;
}
/* ── OS CONVERGENTES pela recorrência — e nunca se forma o quociente ───────────
 * pₙ = aₙ·pₙ₋₁ + pₙ₋₂,  qₙ = aₙ·qₙ₋₁ + qₙ₋₂. Devolve quantos coube. */
static int nt_convergentes(const unsigned long *a, int n, unsigned long *p, unsigned long *q){
    unsigned long pm1 = 1, pm2 = 0, qm1 = 0, qm2 = 1;
    int k = 0;
    for(; k < n; k++){
        if(a[k] != 0 && (pm1 > NT_TECTO / (a[k] ? a[k] : 1)
                      || qm1 > NT_TECTO / (a[k] ? a[k] : 1))){ nt_saturou++; break; }
        unsigned long pk = a[k]*pm1 + pm2, qk = a[k]*qm1 + qm2;
        p[k] = pk; q[k] = qk;
        pm2 = pm1; pm1 = pk;
        qm2 = qm1; qm1 = qk;
    }
    return k;
}
/* ── A COMPARAÇÃO DE DOIS RACIONAIS, sem os dividir e sem os subtrair ──────────
 * p/q contra r/s é p·s contra r·q — e aqui o produto vigia-se em vez de se assumir.
 * Devolve −1, 0, +1, e −2 quando não cabe (e conta-se). */
static int nt_cmp(unsigned long p, unsigned long q, unsigned long r, unsigned long s){
    if(q == 0 || s == 0) return -2;
    if(p > NT_TECTO || s > NT_TECTO || r > NT_TECTO || q > NT_TECTO){ nt_saturou++; return -2; }
    unsigned long x = p*s, y = r*q;
    return x < y ? -1 : (x > y ? 1 : 0);
}
/* ── A DISTÂNCIA ENTRE CONVERGENTES CONSECUTIVOS: FORMA FECHADA ────────────────
 * |pₙ/qₙ − pₙ₊₁/qₙ₊₁| = 1/(qₙ qₙ₊₁), e a identidade que a sustenta é
 * pₙ qₙ₊₁ − pₙ₊₁ qₙ = ±1. Devolve o denominador qₙ·qₙ₊₁, ou 0 se não couber.
 * Repare-se: NENHUMA subtracção de racionais acontece. */
static unsigned long nt_dist_denom(unsigned long qn, unsigned long qn1){
    if(qn == 0 || qn1 == 0) return 0;
    if(qn > NT_TECTO / qn1){ nt_saturou++; return 0; }
    return qn * qn1;
}
/* e a identidade que a justifica, verificável em naturais: |pₙqₙ₊₁ − pₙ₊₁qₙ| = 1 */
static int nt_identidade(unsigned long pn, unsigned long qn,
                         unsigned long pn1, unsigned long qn1, unsigned long *d){
    if(pn > NT_TECTO || qn1 > NT_TECTO || pn1 > NT_TECTO || qn > NT_TECTO){
        nt_saturou++; return 0;
    }
    unsigned long a = pn*qn1, b = pn1*qn;
    *d = a > b ? a - b : b - a;               /* a subtracção com a fibra JÁ escolhida */
    return 1;
}
/* ── O MÓDULO DE CAUCHY PELO PASSO, e não pela órbita ──────────────────────────
 * Dado ε = ea/eb, acha o primeiro n com 1/(qₙ qₙ₊₁) < ea/eb, isto é
 *
 *      qₙ · qₙ₊₁ · ea  >  eb.
 *
 * É uma comparação de naturais pequenos, e o N sai sem se formar diferença nenhuma.
 * Devolve 1 e escreve N; 0 se não achou dentro dos convergentes disponíveis. */
static int nt_modulo(const unsigned long *q, int n, unsigned long ea, unsigned long eb,
                     int *N){
    if(ea == 0) return 0;
    for(int k = 0; k + 1 < n; k++){
        unsigned long d = nt_dist_denom(q[k], q[k+1]);
        if(d == 0) return 0;
        if(d > NT_TECTO / (ea ? ea : 1)){ nt_saturou++; return 0; }
        if(d * ea > eb){ *N = k; return 1; }
    }
    return 0;
}
/* ── O MEDIANTE: a operação de Stern--Brocot, e é PURAMENTE natural ────────────
 * (p+r)/(q+s) fica estritamente entre p/q e r/s. Sem divisão, sem subtracção. */
static void nt_mediante(unsigned long p, unsigned long q, unsigned long r, unsigned long s,
                        unsigned long *mp, unsigned long *mq){
    *mp = p + r; *mq = q + s;
}
#endif
