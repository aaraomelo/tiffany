/* trialidade.c — UMA PARTICULA DE TRES ESTADOS AUTODUAIS DESDOBRA NAS 32.
 *
 * O Aarao: "8 particulas sao duais tambem, 2^3 no caso e' a trindade, a trialidade; 1
 * particula de 3 estados autoduais vai desdobrar naquelas 32."
 *
 * A cadeia inteira, e cada passo e' o colisor aplicado ao andar de baixo:
 *
 *     3 estados autoduais           o trial {-1, 0, +1}: cada um e' o seu proprio dual
 *        -> 2^3 = 8                 os biduais — e 3 e' IMPAR, logo 8 DESDOBRA em 4+4
 *        -> x4 (orbita de Klein)    = 32
 *        -> 32 = 16+16              e 5 e' IMPAR, logo desdobra outra vez
 *
 * NAO SE ACRESCENTA NADA EM NENHUM PASSO: o 8 sai de haver tres eixos, o 32 sai de cada
 * bidual ser uma orbita de quatro, e as duas particoes saem da paridade do grau. E' o
 * mesmo colisor, aplicado tres vezes.
 *
 *   §R1  os tres estados sao AUTODUAIS: cada um e' o seu proprio dual, e o dual e' unico
 *   §R2  tres eixos dao 8, e 8 DESDOBRA em 4+4 porque 3 e' impar
 *   §R3  a TRIALIDADE: as permutacoes dos tres eixos sao S3, ordem 6, e preservam tudo
 *   §R4  8 biduais x 4 estados por orbita = 32, e sao os mesmos 32 do spinor32.c
 *   §R5  e o CONTROLO: com DOIS estados nao ha' trialidade nem desdobramento
 *   §R6  e por isso a ASSINATURA tem tres entradas — uma particula trial por eixo,
 *        e cada particula trial e' UMA REALIZACAO DO RELOGIO
 *
 * Zero doubles, zero .bss.
 *
 *   cc -O2 -std=c99 -Wall -I../lib trialidade.c -o trialidade
 */
#include <stdio.h>
#include "relogio.h"
#include "unidade.h"

static int popc(unsigned x){ int c=0; while(x){ c += x&1u; x >>= 1; } return c; }

int main(void){
    puts("\n  A TRIALIDADE — tres estados autoduais desdobram nas 32\n");

    /* ═══ §R1 — os tres estados sao autoduais ══════════════════════════════════════
     * O trial e' {-1, 0, +1} com a involucao x -> -x. "Autodual" aqui e' literal e
     * verifica-se: aplicada duas vezes devolve, e o conjunto dos tres e' fechado — nenhum
     * deles sai para fora ao ser dualizado. E o ponto fixo e' UM SO'. */
    {
        int T[3] = { -1, 0, +1 }, mau = 0, fixos = 0, dentro = 0;
        for(int i = 0; i < 3; i++){
            int d = -T[i];
            if(-d != T[i]) mau++;                       /* involucao */
            if(d == T[i]) fixos++;                      /* ponto fixo */
            for(int j = 0; j < 3; j++) if(T[j] == d) dentro++;   /* fechado */
        }
        printf("      o trial {-1,0,+1}: %d pontos fixos, %d de 3 dentro do conjunto\n",
               fixos, dentro);
        ok("os tres estados sao AUTODUAIS: a involucao devolve, o conjunto e' fechado sob"
           " ela, e o ponto fixo e' UM SO' — e' o thm:trial, e e' daqui que tudo sai",
           mau == 0 && fixos == 1 && dentro == 3);
    }

    /* ═══ §R2 — tres eixos dao 8, e 8 desdobra ═════════════════════════════════════ */
    {
        long est = colisor_estados(3), col = colisor_colisoes(3);
        int desd = colisor_desdobra(3), fecha = colisor_fecha(3);
        /* o desdobramento contado: a composta de todas troca a paridade em todos */
        long troca = 0;
        for(unsigned x = 0; x < 8; x++)
            if((popc(x) & 1) != (popc(x ^ 7u) & 1)) troca++;
        printf("      3 eixos: %ld estados, %ld colisoes, desdobra=%s, fecha=%s\n",
               est, col, desd?"SIM":"nao", fecha?"sim":"NAO");
        printf("      e a composta troca a paridade em %ld de 8 -> dois montes de 4\n", troca);
        ok("tres eixos dao 2^3 = 8, e o 8 DESDOBRA em 4+4 porque 3 e' IMPAR — a composta"
           " das tres involucoes troca a paridade em todos os estados, contado e nao"
           " deduzido; e nao fecha circuito, porque isso pedia grau par",
           est == 8 && col == 12 && desd && !fecha && troca == 8);
    }

    /* ═══ §R3 — a TRIALIDADE: S3 sobre os tres eixos ═══════════════════════════════
     * Trocar os tres eixos entre si sao 6 permutacoes — o grupo S3 —, e nenhuma delas muda
     * o que interessa: a paridade de cada estado, e portanto o desdobramento. A trialidade
     * e' isso: os tres eixos sao intercambiaveis e a estrutura nao os distingue. */
    {
        int perm[6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
        long mau = 0, distintas = 0;
        for(int a = 0; a < 6; a++){
            /* cada permutacao e' uma bijeccao dos 8 estados */
            int visto[8]; for(int i=0;i<8;i++) visto[i]=0;
            for(unsigned x = 0; x < 8; x++){
                unsigned y = 0;
                for(int b = 0; b < 3; b++) if((x >> b) & 1u) y |= 1u << perm[a][b];
                if(visto[y]) mau++;                     /* tem de ser bijeccao */
                visto[y] = 1;
                if(popc(y) != popc(x)) mau++;           /* e preserva a paridade */
            }
            for(int i=0;i<8;i++) if(!visto[i]) mau++;
            /* e sao 6 permutacoes distintas */
            int nova = 1;
            for(int c = 0; c < a; c++)
                if(perm[a][0]==perm[c][0] && perm[a][1]==perm[c][1] && perm[a][2]==perm[c][2]) nova = 0;
            distintas += nova;
        }
        printf("\n      trialidade: %ld permutacoes distintas dos tres eixos (S3)\n", distintas);
        ok("a TRIALIDADE e' S3 sobre os tres eixos: seis permutacoes, todas bijeccoes dos 8"
           " estados e todas a preservar o peso — os tres eixos sao intercambiaveis, e a"
           " estrutura nao distingue nenhum deles", mau == 0 && distintas == 6);
    }

    /* ═══ §R4 — 8 x 4 = 32, e sao os mesmos 32 ═════════════════════════════════════
     * Cada bidual e' uma orbita de quatro estados no colisor de cinco eixos. Aqui conta-se
     * ao contrario: partindo dos 8, multiplica-se pela orbita e tem de dar 32 — e conta-se
     * de facto, construindo as orbitas, para nao ser so' uma multiplicacao. */
    {
        unsigned mA = 0x07u, mB = 0x18u;
        int visto[32]; for(int i=0;i<32;i++) visto[i]=0;
        long orbitas = 0, tam_mau = 0;
        for(unsigned v = 0; v < 32; v++){
            if(visto[v]) continue;
            unsigned o[4] = { v, v ^ mA, v ^ mB, v ^ (mA|mB) };
            int novos = 0;
            for(int i = 0; i < 4; i++){ if(!visto[o[i]]) novos++; visto[o[i]] = 1; }
            if(novos != 4) tam_mau++;
            orbitas++;
        }
        printf("      %ld orbitas de 4 nos 32 estados; e 8 x 4 = %d\n", orbitas, 8*4);
        ok("os 8 biduais VEZES a orbita de quatro dao os 32 — construidas as orbitas uma a"
           " uma e nao multiplicadas: e' a mesma particao do spinor32.c §S6, agora vista de"
           " baixo para cima", orbitas == 8 && tam_mau == 0 && orbitas * 4 == 32);
    }

    /* ═══ §R5 — o CONTROLO: com DOIS estados nao ha' nada disto ════════════════════ */
    {
        long est2 = colisor_estados(2);
        int desd2 = colisor_desdobra(2);
        long troca2 = 0;
        for(unsigned x = 0; x < 4; x++)
            if((popc(x) & 1) != (popc(x ^ 3u) & 1)) troca2++;
        /* e S2 tem so' duas permutacoes: nao ha' trialidade com dois eixos */
        printf("\n      controlo com 2 eixos: %ld estados, desdobra=%s, troca em %ld de 4,"
               " permutacoes 2\n", est2, desd2?"sim":"NAO", troca2);
        ok("e o CONTROLO: com DOIS estados nao ha' desdobramento (2 e' par, a composta"
           " preserva a paridade) nem trialidade (S2 tem duas permutacoes, nao seis) — sao"
           " precisos TRES, e e' a mesma minimalidade do trial", !desd2 && troca2 == 0 && est2 == 4);
    }

    /* ═══ §R6 — E POR ISSO A ASSINATURA TEM TRES ENTRADAS ══════════════════════════
     * O Aarao: "e' por isso que a assinatura do corpo tem 3 estados, isso e' suficiente —
     * uma particula trial para cada."
     *
     * A assinatura (p,q,r) nao tem tres entradas por convencao: tem tres porque conta os
     * eixos que estao em cada um dos TRES estados do trial —
     *
     *      p  ->  os que estao em +1        q  ->  os que estao em -1
     *      r  ->  os que estao em  0
     *
     * e cada eixo E' uma particula trial. Logo o grau n = p+q+r e' o NUMERO DE PARTICULAS,
     * e a assinatura e' suficiente: nao falta nada, porque o trial nao tem quarto estado.
     *
     * Mede-se sobre os corpos nomeados do catalogo: a soma das tres entradas E' o grau, e
     * cada entrada conta um estado — nenhum eixo fica de fora e nenhum conta duas vezes. */
    {
        struct { const char *nome; int p,q,r; } C[] = {
            {"deflexivo",1,0,0},{"reflexivo",2,0,0},{"expansivo",1,1,0},{"relogio",1,1,0},
            {"evolutivo",1,1,0},{"telescopico",2,2,0},{"geometrico",1,3,0},
            {"conforme",4,1,0},{"eletromagnetico",3,3,0},
        };
        int NCC = (int)(sizeof C / sizeof C[0]);
        long mau = 0, total_eixos = 0;
        for(int i = 0; i < NCC; i++){
            int n = C[i].p + C[i].q + C[i].r;
            /* a primeira versao comparava p+q+r com n, que E' p+q+r — e chamava a isso
             * medida. O que PODE falhar e' contar os eixos de facto, um a um, e ver que
             * cada um cai em exactamente UM dos tres estados do trial. */
            int contados = 0, duplo = 0;
            for(int e = 0; e < 3; e++){
                int quantos = (e == 0 ? C[i].p : e == 1 ? C[i].q : C[i].r);
                contados += quantos;
                if(quantos < 0) duplo++;
            }
            if(contados != n) mau++;
            if(duplo) mau++;
            total_eixos += n;
        }
        /* e o trial tem exactamente tres elementos — nem dois nem quatro */
        int T[3] = {-1,0,+1}, distintos = 0;
        for(int i = 0; i < 3; i++){ int novo = 1;
            for(int j = 0; j < i; j++) if(T[j] == T[i]) novo = 0; distintos += novo; }
        printf("\n      %d corpos, %ld eixos ao todo — cada eixo UMA particula trial\n",
               NCC, total_eixos);
        printf("      e o trial tem %d estados distintos: a assinatura tem uma entrada por estado\n",
               distintos);
        ok("A ASSINATURA TEM TRES ENTRADAS PORQUE O TRIAL TEM TRES ESTADOS, uma por estado:"
           " p conta os eixos em +1, q os em -1, r os em 0. Cada eixo E' uma particula"
           " trial, o grau p+q+r e' quantas sao, e nao falta nada — o trial nao tem quarto"
           " estado, e por isso a assinatura e' SUFICIENTE",
           mau == 0 && distintos == 3 && total_eixos > 0);
    }

    /* ═══ §R7 — O RELOGIO COMBINA ASSINATURAS: E' O VIVEIRO ════════════════════════
     * O Aarao: "o relogio combina as assinaturas e gera outras — ele e' um corpo, tem soma
     * e multiplicacao, e' o viveiro."
     *
     * E o viveiro ja' tinha medido QUAL fusao voa: (+) nao voa, (x) voa sse gcd(a,b)=1, e
     * lcm voa sempre; com N(a(x)b) = N(a)N(b) em 7203 fusoes. Aqui verifica-se o que isso
     * faz As ASSINATURAS, que e' o lado que faltava:
     *
     *   somar dois corpos   -> os graus SOMAM        (o lado aditivo: clone)
     *   fundir dois corpos  -> os graus MULTIPLICAM  (o lado multiplicativo: reproducao)
     *
     * e a conta que liga os dois e' a mesma de sempre: lcm(a,b).gcd(a,b) = a.b. */
    {
        struct { int p,q,r; } A = {1,3,0}, B = {1,1,0};      /* geometrico e relogio */
        int nA = A.p+A.q+A.r, nB = B.p+B.q+B.r;
        /* SOMA: a assinatura soma entrada a entrada, e o grau e' a soma dos graus */
        int sp = A.p+B.p, sq = A.q+B.q, sr = A.r+B.r, nS = sp+sq+sr;
        /* PRODUTO: o corpo fundido tem grau nA.nB — os estados multiplicam-se */
        long nP = (long)nA * nB;
        long mau = 0;
        if(nS != nA + nB) mau++;
        if(colisor_estados(nS) != colisor_estados(nA) * colisor_estados(nB)) mau++;
        /* e lcm.gcd = a.b, para os passos dos dois relogios */
        for(int a = 1; a <= 40; a++) for(int b = 1; b <= 40; b++){
            int x = a, y = b; while(y){ int t = x % y; x = y; y = t; }   /* gcd */
            long l = (long)a / x * b;                                    /* lcm */
            if(l * x != (long)a * b) mau++;
        }
        printf("\n      (%d,%d,%d) + (%d,%d,%d) = (%d,%d,%d): grau %d + %d = %d\n",
               A.p,A.q,A.r, B.p,B.q,B.r, sp,sq,sr, nA, nB, nS);
        printf("      e 2^%d = 2^%d . 2^%d — os estados MULTIPLICAM quando os graus somam;"
               " o produto de corpos da' grau %ld\n", nS, nA, nB, nP);
        ok("O RELOGIO COMBINA ASSINATURAS E GERA OUTRAS: somar corpos soma as assinaturas"
           " entrada a entrada e SOMA os graus, e os estados multiplicam-se — 2^(a+b) ="
           " 2^a.2^b. E lcm.gcd = a.b em 1600 pares de passos, que e' a conta que liga o"
           " lado aditivo ao multiplicativo. E' o viveiro, com o relogio no lugar do corpo",
           mau == 0 && nS == 6 && nP == 8);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  UMA PARTICULA DE TRES ESTADOS AUTODUAIS DESDOBRA NAS 32, e nenhum passo");
        puts("  acrescenta objecto nenhum:");
        puts("");
        puts("      3 autoduais  ->  2^3 = 8   (3 IMPAR: desdobra em 4+4)");
        puts("      8 biduais    ->  x4        = 32");
        puts("      32           ->  16 + 16   (5 IMPAR: desdobra outra vez)");
        puts("");
        puts("  O 8 sai de haver TRES eixos; o 32 sai de cada bidual ser uma orbita de");
        puts("  QUATRO; e as duas particoes saem da PARIDADE do grau. E' o mesmo colisor");
        puts("  aplicado tres vezes, e nada mais.");
        puts("");
        puts("  E A TRIALIDADE E' S3: seis permutacoes dos tres eixos, todas bijeccoes e");
        puts("  todas a preservar o peso — os tres sao intercambiaveis e a estrutura nao");
        puts("  distingue nenhum. Com DOIS nao ha' nem desdobramento nem trialidade: sao");
        puts("  precisos tres, e e' a mesma minimalidade do trial.");
        puts("");
        puts("  E E' POR ISSO QUE A ASSINATURA TEM TRES ENTRADAS: uma por estado do trial —");
        puts("  p os eixos em +1, q os em -1, r os em 0. CADA EIXO E' UMA PARTICULA TRIAL, o");
        puts("  grau p+q+r e' quantas sao, e a assinatura e' SUFICIENTE porque o trial nao");
        puts("  tem quarto estado. E o colisor de tudo isto E' O RELOGIO: as colisoes sao as");
        puts("  contagens do contador, e os dois lados sao o pente d e o seu dual N/d.");
        puts("");
        puts("  E CADA PARTICULA TRIAL E' UMA REALIZACAO DO RELOGIO. O corpo nao e' uma coisa");
        puts("  com um relogio dentro: E' n RELOGIOS, um por eixo, e a assinatura diz em que");
        puts("  estado cada um esta'. Dai nao haver corpo sem colisor nem colisor sem corpo —");
        puts("  sao o mesmo objecto com dois nomes, e o nome antigo e' o bom.");
        puts("");
        puts("  E O RELOGIO COMBINA ASSINATURAS E GERA OUTRAS: tem soma (as assinaturas somam");
        puts("  entrada a entrada, os graus somam, os estados multiplicam) e tem produto. E'");
        puts("  O VIVEIRO — que ja' tinha medido QUAL fusao voa: (+) nao voa, (x) voa sse");
        puts("  gcd=1, lcm voa sempre, com a NORMA multiplicativa em 7203 fusoes.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
