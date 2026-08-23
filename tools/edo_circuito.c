/* edo_circuito.c — O CIRCUITO RLC RESOLVIDO NO BANCO, pela borda do corpo.
 *
 * Não há maquinaria nova. A `lib/edo.h` já diz que a equação característica
 *
 *     y'' + B y' + C y = 0     ⟷     λ² + Bλ + C = 0
 *
 * É A BORDA da álgebra, σ² = b₀ + b₁σ com B = −b₁ e C = −b₀; e que o
 * Δ = B²−4C que classifica as soluções é o MESMO Δ que classifica os corpos.
 *
 * A ponte para o motor é a matriz COMPANHEIRA. Pondo o estado x = (q, q'),
 *
 *     x' = A x,       A = [ 0    1 ]      tr A = −B,   det A = C,
 *                         [ −C  −B ]      Δ = tr² − 4det = B² − 4C
 *
 * e é isto que `SELECT regime(*)` lê --- ele já existia, e lê o Δ de um operador
 * 2×2. Logo RESOLVER A EDO NO BANCO é pôr a companheira numa tabela e perguntar.
 *
 * O CIRCUITO. Malha RLC em série, com q a carga:
 *
 *     L q'' + R q' + q/C = 0     ⟹     q'' + (R/L) q' + (1/LC) q = 0
 *
 * donde B = R/L e C_edo = 1/(LC) --- e os três regimes do chess são os três
 * comportamentos que o electrotécnico já conhece por outro nome:
 *
 *     Δ > 0   sobreamortecido    duas exponenciais      o GATO
 *     Δ = 0   crítico            raiz dupla, t·e^{λt}   a fronteira
 *     Δ < 0   subamortecido      oscila e decai         o ESQUILO
 *
 *   cc -O2 -std=c99 -Ilib -Ibanco -Itests -DSQL_NO_MAIN -DPGWIRE_NO_MAIN \
 *      tools/edo_circuito.c banco/sql.c banco/pgwire.c -lm -o /tmp/edoc
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include "edo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    SqlOut o, o2;
    char q[240];
    unlink("/tmp/ec.mem"); unlink("/tmp/ec.prog");
    if(!sql_abrir("/tmp/ec")) return 1;

    /* R, L e 1/(LC) inteiros --- a aritmética fica exacta, sem uma vírgula */
    struct { const char *nome; long R, L, invLC; const char *espera; } K[] = {
      {"sobreamortecido  R=5 L=1 1/LC=6", 5, 1, 6, "hiperból"},
      {"crítico          R=4 L=1 1/LC=4", 4, 1, 4, "parabóli"},
      {"subamortecido    R=2 L=1 1/LC=5", 2, 1, 5, "elíptico"},
      {"sem perdas       R=0 L=1 1/LC=4", 0, 1, 4, "elíptico"},
      {"só resistivo     R=3 L=1 1/LC=0", 3, 1, 0, "hiperból"},
    };
    int NK = (int)(sizeof K / sizeof K[0]);

    printf("\n  O CIRCUITO RLC RESOLVIDO NO BANCO --- a companheira é a borda do corpo\n\n");
    printf("  circuito                          a EDO                  Δ   regime     classe"
           "        raízes\n");
    printf("  ────────────────────────────────────────────────────────────────────────────"
           "──────────────────────\n");
    int certos = 0, batem = 0;
    for(int c = 0; c < NK; c++){
        long B = K[c].R / K[c].L, Ce = K[c].invLC;
        long D = B*B - 4*Ce;

        /* ── (1) A EDO, pela lib: dois caminhos para o mesmo Δ ───────────── */
        char eq[96];
        if(B && Ce)      snprintf(eq, sizeof eq, "y'' + %ldy' + %ldy = 0", B, Ce);
        else if(B)       snprintf(eq, sizeof eq, "y'' + %ldy' = 0", B);
        else             snprintf(eq, sizeof eq, "y'' + %ldy = 0", Ce);
        Edo E; int leu = edo_le(eq, &E);
        long Dlib = leu ? E.D : 0;

        /* ── (2) A COMPANHEIRA no banco, e o motor resolve ───────────────── */
        sql_executa("DROP TABLE IF EXISTS A", &o2);
        sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
        snprintf(q, sizeof q, "INSERT INTO A VALUES (0,1)");        sql_executa(q, &o2);
        snprintf(q, sizeof q, "INSERT INTO A VALUES (%ld,%ld)", -Ce, -B); sql_executa(q, &o2);
        sql_executa("SELECT regime(*) FROM A", &o);
        if(!o.ok){ printf("  %-33s RECUSADA: %s\n", K[c].nome, o.err); continue; }
        /* as colunas são (regime, classe, traco, disc) --- e `disc` JÁ É o Δ,
         * não o determinante: lê-lo como det dava um número torto na tabela. */
        const char *reg = o.cell[0][0], *cls = o.cell[0][1];
        long tr = atol(o.cell[0][2]), Dmotor = atol(o.cell[0][3]);
        (void)tr;

        /* ── (3) AS RAÍZES, exactas quando o Δ é quadrado perfeito ───────── */
        char raiz[64];
        if(D > 0){
            long r = 0; while(r*r < D) r++;
            if(r*r == D) snprintf(raiz, sizeof raiz, "%ld, %ld", (-B+r)/2, (-B-r)/2);
            else         snprintf(raiz, sizeof raiz, "(-%ld±√%ld)/2", B, D);
        } else if(D == 0) snprintf(raiz, sizeof raiz, "%ld (dupla)", -B/2);
        else {
            long r = 0; while(r*r < -D) r++;
            if(r*r == -D) snprintf(raiz, sizeof raiz, "%ld ± %ldi", -B/2, r/2);
            else          snprintf(raiz, sizeof raiz, "(-%ld±i√%ld)/2", B, -D);
        }
        printf("  %-33s %-22s %4ld  %-9s  %-14s %s\n",
               K[c].nome, eq, Dmotor, reg, cls, raiz);

        /* o Δ do motor tem de ser o Δ da lib --- dois caminhos, uma conta.
         * E a classe compara-se pelo PREFIXO: o motor devolve «hiperbólico (duas
         * raízes reais)», com acento e explicação; comparar por igualdade exacta
         * reprovava as cinco com o motor certo. */
        if(leu && Dlib == Dmotor) batem++;
        if(Dmotor == D) certos++;
        if(strncmp(cls, K[c].espera, strlen(K[c].espera)))
            printf("      ^^^ classe inesperada: %s\n", cls);
    }
    printf("  ────────────────────────────────────────────────────────────────────────────"
           "──────────────────────\n");
    printf("  %d de %d com o Δ do motor igual ao Δ à mão · %d de %d com o Δ da lib"
           " igual ao do motor --- TRÊS caminhos\n", certos, NK, batem, NK);

    /* ── E O REGIME É O SINAL DE Re(λ): quem dissipa, quem orbita, quem foge ── */
    printf("\n  e o REGIME lê o sinal de Re(λ) --- é o que decide se o circuito morre:\n");
    printf("    R > 0  →  traço = −R/L < 0  →  CRISTAL: colapsa no ponto fixo (dissipa)\n");
    printf("    R = 0  →  traço = 0         →  BORDA:   orbita e conserva (LC puro)\n");
    printf("  e não há terceiro caso num RLC passivo: R negativo seria fonte, não resistor.\n\n");
    sql_fechar();
    return 0;
}
