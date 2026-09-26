/* tests/buraco_racional.c — MEDIDOR MÍNIMO DO BURACO Mat2Q ↔ Qz ↔ banco
 *
 * Pergunta: um objecto A_q representado em Mat2Q (q = 15/4, racional não
 * inteiro) pode ser transportado para Mat, sofrer uma operação da casa,
 * e voltar à representação Mat2Q preservando o objecto e as identidades,
 * mesmo quando esp_disc recusa o racional não inteiro?
 *
 * NÃO testa: equivalência Mat2Q ≡ Qz, substituição do banco, ou correcção
 * de esp_disc. Testa apenas a travessia para UM racional específico.
 *
 * Quatro resultados, cada um respondendo uma pergunta diferente:
 *
 *  1. Mat2Q: A_q, W_q, split funcionam em q=15/4
 *  2. Ponte → casa: esp_disc(A_q) == −1 (recusa documentada de forma.h)
 *  3. Ponte → casa: mat_mult(A,A) funciona na representação Mat
 *  4. Ponte ← casa: reconstrução Mat2Q é identidade
 *
 * Uso:
 *   cc -O2 -std=c99 -I../lib buraco_racional.c -lm -o buraco_racional
 *   ./buraco_racional
 */
#include "matriz2q.h"
#include "matriz2q_ponte.h"
#include "cifra.h"
#include "forma.h"
#include "linear.h"
#include "unidade.h"

int main(void){
    printf("=== BURACO Mat2Q <-> Qz <-> banco: q = 15/4 ===\n\n");

    Qz q = qz(15, 4);
    int falhas = 0;

    /* 1. Mat2Q: A_q, W_q, split funcionam */
    printf("--- 1. Mat2Q: invariantes de A_q em q=15/4 ---\n");
    {
        Mat2Q A = m2q_A(q), W = m2q_W(q), I = m2q_id();
        int okk = 1;
        if(!qz_igual(A.a, qz(15,4))) okk = 0;
        if(!qz_igual(A.b, qz(1,1))) okk = 0;
        if(!qz_igual(A.c, qz(1,1))) okk = 0;
        if(!qz_igual(A.d, qz(0,1))) okk = 0;
        if(!qz_igual(m2q_tr(A), q)) okk = 0;
        if(!qz_igual(m2q_det(A), qz(-1,1))) okk = 0;
        if(!m2q_igual(m2q_mult(A, A), m2q_soma(m2q_esc(q, A), I))) okk = 0;
        Mat2Q W_exp = m2q(qz(15,4), qz(2,1), qz(2,1), qz_oposto(q));
        if(!m2q_igual(W, W_exp)) okk = 0;
        if(!m2q_igual(m2q_mult(W, W), m2q_esc(m2q_Delta(q), I))) okk = 0;
        int split = m2q_split(q, NULL);
        printf("      split? %s (Delta = 289/16 = (17/4)^2)\n", split ? "sim" : "nao");
        ok("§1 Mat2Q: A_q, W_q, split, tr, det, identidades em q=15/4", okk);
        falhas += okk ? 0 : 1;
    }

    /* 2. Ponte -> casa: esp_disc recusa (contrato forma.h:143) */
    printf("\n--- 2. Ponte -> casa: esp_disc recusa racional nao inteiro ---\n");
    {
        Mat A = m2q_ponte_Mat(m2q_A(q));
        long D = esp_disc(A);
        printf("      esp_disc(A_q) = %ld (esperado -1)\n", D);
        int okk = (D == -1);
        ok("§2 esp_disc(A_q) == -1 (recusa documentada de forma.h)", okk);
        falhas += okk ? 0 : 1;
    }

    /* 3. Ponte -> casa: mat_mult(A,A) funciona */
    printf("\n--- 3. Ponte -> casa: operacao matricial da casa ---\n");
    {
        Mat A = m2q_ponte_Mat(m2q_A(q));
        Mat I = mat_id(2);
        Mat AA = mat_mult(A, A);
        Mat qA_plus_I = mat_soma(mat_esc(q, A), I);
        int okk = mat_igual(AA, qA_plus_I);
        printf("      A_q^2 = q A_q + I via mat_mult da casa: %s\n", okk ? "SIM" : "NAO");
        ok("§3 mat_mult(A,A) = q A_q + I na representacao Mat", okk);
        falhas += okk ? 0 : 1;
    }

    /* 4. Ponte <- casa: reconstrucao Mat2Q e round-trip */
    printf("\n--- 4. Ponte <- casa: reconstrucao e round-trip ---\n");
    {
        Mat2Q original = m2q_A(q);
        Mat A = m2q_ponte_Mat(original);
        Mat2Q reconstruido;
        int ok_ida = m2q_ponte_Mat2Q(A, &reconstruido);
        int okk = ok_ida && m2q_igual(reconstruido, original);
        printf("      Mat2Q -> Mat -> Mat2Q: %s\n", okk ? "IDENTIDADE" : "FALHA");
        ok("§4 round-trip Mat2Q -> Mat -> Mat2Q e identidade", okk);
        falhas += okk ? 0 : 1;
    }

    /* 5. Comparação de invariantes antes/depois */
    printf("\n--- 5. Invariantes antes vs depois ---\n");
    {
        Mat2Q original = m2q_A(q);
        Mat A = m2q_ponte_Mat(original);
        Mat2Q reconstruido;
        m2q_ponte_Mat2Q(A, &reconstruido);
        int okk = 1;
        if(!qz_igual(m2q_tr(original), m2q_tr(reconstruido))) okk = 0;
        if(!qz_igual(m2q_det(original), m2q_det(reconstruido))) okk = 0;
        if(!m2q_igual(original, reconstruido)) okk = 0;
        printf("      tr original = tr reconstruido: %s\n",
               qz_igual(m2q_tr(original), m2q_tr(reconstruido)) ? "SIM" : "NAO");
        printf("      det original = det reconstruido: %s\n",
               qz_igual(m2q_det(original), m2q_det(reconstruido)) ? "SIM" : "NAO");
        printf("      Mat2Q identical: %s\n", m2q_igual(original, reconstruido) ? "SIM" : "NAO");
        ok("§5 invariantes (tr, det, igualdade) preservados no round-trip", okk);
        falhas += okk ? 0 : 1;
    }

    printf("\n=== RESULTADO: %d falhas ===\n", falhas);
    printf("Cada falha responde a uma pergunta diferente.\n");
    printf("Nenhum resultado implica equivalencia geral nem substituicao do banco.\n");
    return falhas ? 1 : 0;
}