/* hidraulico_pneumatico.c — HIDRÁULICO E PNEUMÁTICO, montados pelo cliente.
 *
 * O catálogo já diz que não são sistemas diferentes --- «eléctrico, mecânico,
 * pneumático, óptico e elástico não são cinco sistemas» --- e a analogia é
 * EXACTA: pressão ↔ tensão, caudal ↔ corrente. Medimos isso no banco: os dois
 * dão os MESMOS endereços, travessia identidade, preço zero.
 *
 * Então o que os separa não é a estrutura: é UM PARÂMETRO --- a
 * compressibilidade do fluido, que entra na capacitância:
 *
 *     I·dQ/dt + R·Q + (1/C)∫Q dt = P      ⟹    Q'' + (R/I)Q' + (1/IC)Q = 0
 *
 *     ÓLEO   quase incompressível → C pequena → 1/(IC) GRANDE → Δ < 0 → OSCILA
 *     AR     compressível         → C grande  → 1/(IC) pequeno → Δ > 0 → não
 *
 * E a oscilação hidráulica tem nome de engenharia: é o GOLPE DE ARÍETE, o
 * martelo que parte tubagem quando se fecha uma válvula depressa. O pneumático
 * não o tem, e a razão é a mesma que o motor devolve: o ar comprime.
 */
#include "unidade.h"
#include "pgwire.h"
#include "pgwire_api.h"
#include "pqlike.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void){
    SqlOut o, o2;
    char q[300];
    unlink("/tmp/hp.mem"); unlink("/tmp/hp.prog");
    if(!sql_abrir("/tmp/hp")) return 1;

    printf("\n  ══ HIDRÁULICO E PNEUMÁTICO ══  a mesma malha, dois fluidos\n\n");
    printf("    I·dQ/dt + R·Q + (1/C)∫Q = P      inertância I, resistência R,"
           " capacitância C\n");
    printf("    o dicionário é exacto:  pressão ↔ tensão · caudal ↔ corrente\n\n");

    /* ── 1. O CLIENTE CONFIRMA QUE SÃO O MESMO CORPO ──────────────────────── */
    sql_executa("DROP TABLE IF EXISTS D", &o2);
    sql_executa("CREATE TABLE D (hidraulico RACIONAL, pneumatico RACIONAL)", &o2);
    for(long p = 1; p <= 6; p++) for(long Q = 1; Q <= 6; Q++){
        /* a leitura de cada um é o produto pressão×caudal --- a potência */
        snprintf(q, sizeof q, "INSERT INTO D VALUES (%ld,%ld)", p*Q, p*Q);
        sql_executa(q, &o2);
    }
    sql_executa("SELECT global(*) FROM D", &o);
    printf("  1) SELECT global(*) FROM D     -- são o mesmo corpo?\n");
    if(o.ok) printf("     reversíveis %s/2 · pior passo 2^-%s · ponta 2^-%s · domina %s"
                    " · ultramétrica: %s violam\n", o.cell[0][0], o.cell[0][1],
                    o.cell[0][2], o.cell[0][3], o.cell[0][4]);
    else printf("     RECUSADA: %s  ← e a recusa é a resposta: leituras iguais não são"
                " DUAS representações\n", o.err);
    printf("     as duas colunas são idênticas: a travessia é a identidade e o preço é"
           " ZERO\n\n");

    /* ── 2. O CLIENTE RESOLVE OS DOIS, mudando só o fluido ────────────────── */
    struct { const char *nome; long I, R, invIC; const char *fluido; } K[2] = {
        {"HIDRÁULICO", 1, 4, 13, "óleo: quase incompressível, C pequena"},
        {"PNEUMÁTICO", 1, 4,  3, "ar: compressível, C grande"},
    };
    printf("  2) SELECT edo(*)               -- o mesmo circuito, dois fluidos\n");
    printf("     %-11s %-38s %-20s %5s  %-11s\n", "", "o fluido", "a equação", "Δ", "classe");
    for(int c = 0; c < 2; c++){
        sql_executa("DROP TABLE IF EXISTS A", &o2);
        sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
        sql_executa("INSERT INTO A VALUES (0,1)", &o2);
        snprintf(q, sizeof q, "INSERT INTO A VALUES (%ld,%ld)",
                 -K[c].invIC, -K[c].R / K[c].I);
        sql_executa(q, &o2);
        sql_executa("SELECT edo(*) FROM A", &o);
        if(!o.ok){ printf("     %-11s RECUSADA\n", K[c].nome); continue; }
        printf("     %-11s %-38s %-20s %5s  %-11s\n", K[c].nome, K[c].fluido,
               o.cell[0][0], o.cell[0][1], o.cell[0][2]);
        printf("     %-11s λ = %s, %s\n", "", o.cell[0][3], o.cell[0][4]);
        printf("     %-11s Q(t) = %s\n", "", o.cell[0][5]);
    }

    printf("\n  o que o cliente lê disto:\n");
    printf("    · a ESTRUTURA é a mesma --- mesma malha, mesma companheira, mesmo motor\n");
    printf("    · o que muda é UM PARÂMETRO: a capacitância, que é a compressibilidade\n");
    printf("    · o óleo dá Δ<0 e OSCILA: é o GOLPE DE ARÍETE, o martelo que parte tubo\n");
    printf("    · o ar dá Δ>0 e não oscila: comprime, e a compressão absorve\n");
    printf("    · logo NÃO se põe acumulador no pneumático --- ele já é o acumulador\n\n");

    /* ── 3. O GUME: a fronteira entre os dois é um número, e acha-se ──────── */
    printf("  3) e a fronteira entre eles é UM número, que o cliente acha perguntando:\n");
    for(long ic = 1; ic <= 6; ic++){
        sql_executa("DROP TABLE IF EXISTS A", &o2);
        sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
        sql_executa("INSERT INTO A VALUES (0,1)", &o2);
        snprintf(q, sizeof q, "INSERT INTO A VALUES (%ld,-4)", -ic);
        sql_executa(q, &o2);
        sql_executa("SELECT edo(*) FROM A", &o);
        if(!o.ok) continue;
        printf("     1/IC = %ld → Δ = %-5s %s%s\n", ic, o.cell[0][1], o.cell[0][2],
               !strcmp(o.cell[0][2], "parabolico") ? "   ← A FRONTEIRA: o crítico" : "");
    }
    printf("     abaixo de 4 não oscila, acima oscila, e em 4 é o crítico --- a mesma\n");
    printf("     fronteira que o RLC tem, porque é a MESMA equação\n\n");
    sql_fechar();
    return 0;
}
