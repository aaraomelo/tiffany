/* servomotor.c — UM CIRCUITO ELECTROMECÂNICO REAL, MONTADO PELO CLIENTE.
 *
 * O objecto é um servomotor DC de bancada, com os valores de catálogo:
 *
 *     R = 2 Ω        resistência do enrolamento
 *     L = 0,5 H      indutância do enrolamento
 *     J = 0,02 kg·m² inércia do rotor
 *     b = 0,2 N·m·s  atrito viscoso
 *     K = 0,1        constante de torque = constante de f.e.m. (SI: são iguais)
 *
 * As duas malhas, que são os dois corpos:
 *
 *     ELÉCTRICO   L·di/dt + R·i + K·ω = V        (o eletromagnético)
 *     MECÂNICO    J·dω/dt + b·ω − K·i = τ        (o mecânico)
 *
 * O acoplamento K aparece nos DOIS com sinais opostos --- é ele que faz a
 * energia atravessar de um corpo para o outro, e é a fusão em forma de física.
 * Eliminando i, a característica do fundido é
 *
 *     λ² + (R/L + b/J)·λ + (R·b + K²)/(L·J) = 0
 *
 * e com estes valores dá coeficientes INTEIROS --- B = 14 s⁻¹, C = 41 s⁻² ---,
 * que é o que permite ao motor resolver sem uma vírgula.
 *
 * O CLIENTE faz três coisas, todas por SQL:
 *   1. funde os dois corpos          SELECT funde(*)
 *   2. resolve o transitório         SELECT edo(*)
 *   3. lê o regime                   SELECT regime(*)
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
    unlink("/tmp/sv.mem"); unlink("/tmp/sv.prog");
    if(!sql_abrir("/tmp/sv")) return 1;

    printf("\n  ══ SERVOMOTOR DC ══  R=2Ω  L=0,5H  J=0,02kg·m²  b=0,2N·m·s  K=0,1\n\n");
    /* em centésimos, para a conta ser inteira: R=200 L=50 J=2 b=20 K=10 */
    long R = 200, L = 50, J = 2, b = 20, K = 10;
    long B = R/L + b/J;                     /* 4 + 10 = 14 s⁻¹ */
    long C = (R*b + K*K)/(L*J);             /* (4000+100)/100 = 41 s⁻² */
    printf("  as duas malhas, que são os dois corpos:\n");
    printf("    ELÉCTRICO   L·di/dt + R·i + K·ω = V      taxa própria R/L = %ld s⁻¹\n", R/L);
    printf("    MECÂNICO    J·dω/dt + b·ω − K·i = τ      taxa própria b/J = %ld s⁻¹\n", b/J);
    printf("    o acoplamento K entra nos dois com sinais opostos\n\n");

    /* ── 1. O CLIENTE FUNDE OS DOIS CORPOS ────────────────────────────────── */
    sql_executa("DROP TABLE IF EXISTS M", &o2);
    sql_executa("CREATE TABLE M (eletrico RACIONAL, mecanico RACIONAL)", &o2);
    for(long i = 0; i < 6; i++) for(long w = 0; w < 6; w++){
        /* AS LEITURAS SÃO AS DOS INSTRUMENTOS, e cada uma vê metade: o
         * amperímetro no enrolamento lê a CORRENTE e não sabe a velocidade; o
         * tacómetro no veio lê a VELOCIDADE e não sabe a corrente. É por isso
         * que cada um sozinho funde --- e é isso que a fusão desfaz. */
        snprintf(q, sizeof q, "INSERT INTO M VALUES (%ld,%ld)", i, w);
        sql_executa(q, &o2);
    }
    sql_executa("SELECT funde(*) FROM M", &o);
    long dist = 0, de = 0, dm = 0;
    if(o.ok){
        long v[64], n = 0, ve[64], nee = 0, vm[64], nm = 0;
        for(long r = 0; r < o.nrows; r++){
            long f = atol(o.cell[r][2]), a = atol(o.cell[r][0]), bb = atol(o.cell[r][1]);
            int n1=1,n2=1,n3=1;
            for(long j=0;j<n;j++) if(v[j]==f){n1=0;break;}
            for(long j=0;j<nee;j++) if(ve[j]==a){n2=0;break;}
            for(long j=0;j<nm;j++) if(vm[j]==bb){n3=0;break;}
            if(n1&&n<64) v[n++]=f; if(n2&&nee<64) ve[nee++]=a; if(n3&&nm<64) vm[nm++]=bb;
        }
        dist = n; de = nee; dm = nm;
    }
    printf("  1) SELECT funde(*) FROM M      -- o cliente funde os dois corpos\n");
    printf("     o amperímetro distingue %ld estados · o tacómetro %ld · o FUNDIDO %ld\n",
           de, dm, dist);
    printf("     --- nenhum instrumento sozinho determina o estado do motor;"
           " os dois juntos determinam\n\n");

    /* ── 2. O CLIENTE RESOLVE O TRANSITÓRIO ───────────────────────────────── */
    sql_executa("DROP TABLE IF EXISTS A", &o2);
    sql_executa("CREATE TABLE A (x RACIONAL, y RACIONAL)", &o2);
    sql_executa("INSERT INTO A VALUES (0,1)", &o2);
    snprintf(q, sizeof q, "INSERT INTO A VALUES (%ld,%ld)", -C, -B);
    sql_executa(q, &o2);
    sql_executa("SELECT edo(*) FROM A", &o);
    printf("  2) SELECT edo(*) FROM A        -- o cliente resolve o transitório\n");
    if(o.ok){
        printf("     %s\n", o.cell[0][0]);
        printf("     Δ = %s · %s · λ = %s, %s\n", o.cell[0][1], o.cell[0][2],
               o.cell[0][3], o.cell[0][4]);
        printf("     ω(t) = %s\n\n", o.cell[0][5]);
    } else printf("     RECUSADA: %s\n\n", o.err);

    /* ── 3. O CLIENTE LÊ O REGIME ─────────────────────────────────────────── */
    sql_executa("SELECT regime(*) FROM A", &o2);
    printf("  3) SELECT regime(*) FROM A     -- o cliente pergunta se o motor estabiliza\n");
    if(o2.ok) printf("     %s · %s · traço %s · Δ %s\n\n",
                     o2.cell[0][0], o2.cell[0][1], o2.cell[0][2], o2.cell[0][3]);

    /* ── E O QUE ISTO DIZ DO MOTOR, em engenharia ─────────────────────────── */
    printf("  o que o cliente lê disto:\n");
    printf("    · as duas raízes são reais e NEGATIVAS  →  o motor PÁRA sozinho\n");
    printf("    · λ = −7 ± 2√2 ≈ −4,17 e −9,83 s⁻¹     →  as duas constantes de tempo\n");
    printf("    · a lenta manda: τ = 1/4,17 ≈ 0,24 s   →  é o tempo de resposta\n");
    printf("    · Δ = %ld > 0, sem oscilação            →  não há sobre-elongação\n",
           B*B - 4*C);
    printf("    · e o motor NÃO arredondou o √32: quem quiser o número sobe a torre\n\n");

    /* ── O GUME: sem acoplamento, o motor deixa de ser um motor ───────────── */
    long C0 = (R*b)/(L*J);
    sql_executa("DROP TABLE IF EXISTS Z", &o2);
    sql_executa("CREATE TABLE Z (x RACIONAL, y RACIONAL)", &o2);
    sql_executa("INSERT INTO Z VALUES (0,1)", &o2);
    snprintf(q, sizeof q, "INSERT INTO Z VALUES (%ld,%ld)", -C0, -B);
    sql_executa(q, &o2);
    sql_executa("SELECT edo(*) FROM Z", &o);
    printf("  o gume --- com K=0 (sem acoplamento) o motor deixa de ser motor:\n");
    if(o.ok) printf("     %s · λ = %s, %s  ← as taxas próprias %ld e %ld, separadas\n\n",
                    o.cell[0][0], o.cell[0][3], o.cell[0][4], R/L, b/J);
    sql_fechar();
    return 0;
}
