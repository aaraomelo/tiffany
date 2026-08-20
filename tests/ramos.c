/* tests/ramos.c — OS DOIS RAMOS de |det| = 1: a leitura métrica do par de |det| = 1, e são UMA UNIDADE.
 *
 * O Aarão: «o esquilo não é remendo, é DUAL. Um lado é o corpo universal, do outro a
 * realização — Peano, por exemplo. Nenhum é melhor que o outro, os dois são uma unidade.»
 *
 * Um remendo mede-se perguntando se salva; um DUAL mede-se perguntando três coisas, e são
 * essas que este ficheiro faz:
 *
 *   §S0  ESGOTAM: todo |det| = 1 é gato, esquilo ou a borda — nada fica de fora
 *   §S1  NENHUM É MELHOR: cada um tem exactamente o que falta ao outro, e conta-se
 *   §S2  A UNIDADE é a conservação: os dois fazem-na, e é o único sítio onde coincidem
 *   §S3  A LEI 8: no anel o gato ganha período — logo o RAMO é da realização, a lei não
 *   §S4  O GUME: |det| ≠ 1 não é nem um nem outro, e o buscador tem de o ver
 *   §S5  E a restrição inteira: rodar E ser inteiro só permite traço −1, 0, 1
 */
#include <stdio.h>
#include "ramos.h"
#include "unidade.h"

int main(void){
    printf("\n=== O TEOREMA DO ESQUILO: o outro ramo, e a unidade ===\n");

    /* ═══ §S0 OS DOIS RAMOS ESGOTAM |det| = 1 ═════════════════════════════════ */
    printf("\n§S0 Todo |det| = 1 é gato, esquilo ou a borda — e nada fica de fora.\n\n");
    {
        long gatos = 0, esquilos = 0, bordas = 0, fora = 0, nem = 0, casos = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++)
        for(long c = -6; c <= 6; c++) for(long d = -6; d <= 6; d++){
            Sq2 x = { a, b, c, d };
            long det = sq_det(x);
            casos++;
            int r = sq_ramo(sq_traco(x), det);
            if(r == SQ_NEM){ nem++; continue; }
            if(r == SQ_GATO) gatos++;
            else if(r == SQ_ESQUILO) esquilos++;
            else if(r == SQ_BORDA) bordas++;
            else fora++;
        }
        printf("      em %ld matrizes: gato %ld · esquilo %ld · borda %ld · fora dos"
               " ramos %ld\n", casos, gatos, esquilos, bordas, fora);
        printf("      e as que nem conservam a medida (|det| ≠ 1): %ld\n", nem);
        ok("OS DOIS RAMOS ESGOTAM A LEI: toda matriz inteira com |det| = 1 é gato"
           " (hiperbólica), esquilo (elíptica) ou está na borda entre eles — nenhuma fica"
           " de fora. É isto que faz deles um PAR e não uma coisa e o seu remendo: juntos"
           " cobrem exactamente a conservação da medida, e o discriminante t² − 4d decide"
           " qual, sem avaliar raiz nenhuma. E as duas famílias são ambas não vazias, o que"
           " é preciso dizer: um par com um lado vazio não é um par",
           fora == 0 && gatos > 0 && esquilos > 0 && casos == 28561);
    }

    /* ═══ §S1 NENHUM É MELHOR: cada um tem o que falta ao outro ═══════════════ */
    printf("\n§S1 O gato mede e cresce; o esquilo roda e não mede.\n\n");
    {
        printf("        objecto        traço  det   ramo        período   satura no passo\n");
        long g_sem_periodo = 0, g_satura = 0, gn = 0;
        for(long m = 1; m <= 5; m++){
            Sq2 x = sq_gato(m);
            long per = sq_periodo(x, 500);
            long sat = sq_passo_que_satura(x, 1000000000L);
            gn++;
            if(per == 0) g_sem_periodo++;
            if(sat > 0) g_satura++;
            printf("        gato A_%-8ld %-6ld %-5ld hiperbólico %-9s %ld\n",
                   m, sq_traco(x), sq_det(x), per ? "tem" : "NENHUM", sat);
        }
        long e_com_periodo = 0, e_satura = 0, en = 0;
        Sq2 rot[3];
        rot[0] = sq_rotor();                       /* traço 0 → ordem 4 */
        { Sq2 t = { 0, -1, 1, -1 }; rot[1] = t; }  /* traço −1 → ordem 3 */
        { Sq2 t = { 1, -1, 1,  0 }; rot[2] = t; }  /* traço 1 → ordem 6 */
        for(int i = 0; i < 3; i++){
            long per = sq_periodo(rot[i], 500);
            long sat = sq_passo_que_satura(rot[i], 1000000000L);
            en++;
            if(per > 0) e_com_periodo++;
            if(sat > 0) e_satura++;
            printf("        esquilo t=%-5ld %-6ld %-5ld elíptico    %-9ld %s\n",
                   sq_traco(rot[i]), sq_traco(rot[i]), sq_det(rot[i]), per,
                   sat ? "satura" : "NUNCA");
        }
        printf("      → o gato: %ld de %ld sem período nenhum, e %ld de %ld saturam\n",
               g_sem_periodo, gn, g_satura, gn);
        printf("      → o esquilo: %ld de %ld com período, e %ld de %ld saturam\n",
               e_com_periodo, en, e_satura, en);
        ok("NENHUM É MELHOR, E É ISSO QUE SE MEDE: o gato tem direcções próprias REAIS —"
           " estica por σ numa e encolhe por 1/σ na outra —, e é essa assimetria que lhe dá"
           " uma RÉGUA: ele conserva MEDINDO. O preço está na coluna da direita: a órbita"
           " cresce como σᵏ e satura em toda representação finita. O esquilo não tem eixo"
           " real nenhum: só roda, tem período 3, 4 ou 6, e NUNCA satura. O preço é o"
           " simétrico — sem eixo não há régua, e ele conserva SEM MEDIR. Cada um tem"
           " exactamente o que falta ao outro, e por isso nenhum sozinho é o teorema",
           g_sem_periodo == gn && g_satura == gn && e_com_periodo == en && e_satura == 0);
    }

    /* ═══ §S2 A UNIDADE: a conservação é o que ambos fazem ════════════════════ */
    printf("\n§S2 A unidade é |det| = 1 — o único sítio onde os dois coincidem.\n\n");
    {
        long mal = 0, casos = 0;
        for(long m = 1; m <= 40; m++){
            Sq2 x = sq_gato(m);
            casos++;
            if(sq_det(x) != -1) mal++;             /* conserva a medida, invertendo */
        }
        Sq2 r = sq_rotor();
        Sq2 p = r;
        long rot_mal = 0;
        for(int k = 1; k <= 8; k++){
            casos++;
            if(sq_det(p) != 1) rot_mal++;          /* conserva a medida, sem inverter */
            p = sq_mult(p, r);
        }
        printf("      o gato em 40 metais: |det| = 1 sempre (%ld falhas), com det = −1 —"
               " inverte a orientação\n", mal);
        printf("      o esquilo em 8 potências: |det| = 1 sempre (%ld falhas), com"
               " det = +1 — não inverte\n", rot_mal);
        ok("A UNIDADE É A CONSERVAÇÃO, E É O ÚNICO SÍTIO ONDE OS DOIS COINCIDEM. Ambos têm"
           " |det| = 1 — ambos conservam a medida —, e diferem no SINAL: o gato inverte a"
           " orientação e o esquilo não. A medida não vê o sinal, e é por isso que a lei é"
           " a mesma para os dois; a orientação vê, e é por isso que as faces são"
           " diferentes. A lei é o que eles têm em comum; medir e rodar é como cada um a"
           " realiza",
           mal == 0 && rot_mal == 0 && casos == 48);
    }

    /* ═══ §S3 A LEI 8: no anel o gato ganha período ═══════════════════════════ */
    printf("\n§S3 A Lei 8: no anel o gato ganha período — logo o RAMO é da realização.\n\n");
    {
        printf("        metal   período em ℤ    período em ℤ_65537\n");
        long sem_z = 0, com_anel = 0, n = 0;
        for(long m = 1; m <= 5; m++){
            Sq2 x = sq_gato(m);
            long pz = sq_periodo(x, 400);
            long pa = sq_periodo_anel(x, SQ_LEI8, 300000);
            n++;
            if(pz == 0) sem_z++;
            if(pa > 0) com_anel++;
            printf("        A_%-6ld %-15s %ld\n", m, pz ? "tem" : "NENHUM", pa);
        }
        printf("      → sobre ℤ: %ld de %ld sem período.  No anel da Lei 8: %ld de %ld"
               " COM período\n", sem_z, n, com_anel, n);
        ok("E A LEI 8 É ONDE A UNIDADE SE VÊ: no anel ℤ_65537 — o primo de Fermat 2¹⁶+1 —"
           " o grupo é finito, logo toda órbita fecha, INCLUSIVE a do gato. Ele, que sobre"
           " ℤ cresce sem parar e não tem período nenhum, no anel passa a ter um. A leitura"
           " NÃO é «o anel salva o gato»: é que ser hiperbólico ou elíptico é propriedade"
           " da REALIZAÇÃO — de se estar sobre ℤ ou sobre ℤ_p —, enquanto |det| = 1 é a"
           " mesma em todas. A lei é universal; a face é da instância. É a doutrina desta"
           " casa dita em matrizes, e agora medida",
           sem_z == n && com_anel == n && n == 5);
    }

    /* ═══ §S4 O GUME: |det| ≠ 1 não é nem um nem outro ════════════════════════ */
    printf("\n§S4 O gume: sem |det| = 1 não há ramo nenhum.\n\n");
    {
        long achou = 0, casos = 0, falso = 0, ctrl = 0;
        for(long k = 2; k <= 9; k++){
            Sq2 x = { k, 0, 0, 1 };                /* det = k ≠ ±1 */
            casos++;
            if(sq_ramo(sq_traco(x), sq_det(x)) == SQ_NEM) achou++;
        }
        for(long m = 1; m <= 20; m++){
            Sq2 x = sq_gato(m);
            ctrl++;
            if(sq_ramo(sq_traco(x), sq_det(x)) == SQ_NEM) falso++;
        }
        printf("      dilatações com |det| = k ≠ 1: %ld de %ld reconhecidas como fora da"
               " lei\n", achou, casos);
        printf("      e o controlo, nos 20 metais: %ld falsos positivos\n", falso);
        ok("E O GUME TEM OS DOIS CONTROLOS: um regime onde a classificação tem de dizer"
           " «nem um nem outro» — as dilatações, que escalam a área e não conservam nada —"
           " e um onde tem de reconhecer o ramo, que são os metais. Sem o segundo, uma"
           " classificação que recusasse tudo passaria no primeiro. E o que isto diz é que"
           " o par gato/esquilo não é uma partição de todas as matrizes: é a partição"
           " EXACTA das que cumprem a lei",
           achou == casos && falso == 0 && ctrl == 20);
    }

    /* ═══ §S5 RODAR E SER INTEIRO: a restrição que aparece sozinha ════════════ */
    printf("\n§S5 Rodar E ser inteiro só permite traço −1, 0, 1.\n\n");
    {
        long tracos[8]; int nt = 0;
        long mal = 0, casos = 0;
        for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++)
        for(long c = -8; c <= 8; c++) for(long d = -8; d <= 8; d++){
            Sq2 x = { a, b, c, d };
            if(sq_det(x) != 1) continue;
            if(sq_ramo(sq_traco(x), 1) != SQ_ESQUILO) continue;
            casos++;
            long t = sq_traco(x);
            if(t < -1 || t > 1) mal++;
            int visto = 0;
            for(int i = 0; i < nt; i++) if(tracos[i] == t) visto = 1;
            if(!visto && nt < 8) tracos[nt++] = t;
        }
        printf("      %ld esquilos inteiros, e os traços que aparecem: ", casos);
        for(int i = 0; i < nt; i++) printf("%ld ", tracos[i]);
        printf("  (%d valores)\n", nt);
        long ordens[4]; int no = 0, om = 0;
        Sq2 amostra[3];
        amostra[0] = sq_rotor();
        { Sq2 t = { 0,-1,1,-1 }; amostra[1] = t; }
        { Sq2 t = { 1,-1,1, 0 }; amostra[2] = t; }
        for(int i = 0; i < 3; i++){
            long per = sq_periodo(amostra[i], 100);
            if(per != 4 && per != 3 && per != 6) om++;
            if(no < 4) ordens[no++] = per;
        }
        printf("      e as ordens: %ld, %ld, %ld — para traço 0, −1 e 1\n",
               ordens[0], ordens[1], ordens[2]);
        ok("RODAR E SER INTEIRO AO MESMO TEMPO SÓ PERMITE TRÊS TRAÇOS: −1, 0 e 1, com"
           " ordens 3, 4 e 6. Isto não foi procurado — apareceu ao varrer, e é a restrição"
           " cristalográfica: as únicas rotações que um reticulado admite. O esquilo é"
           " inteiro e por isso é RARO, enquanto há um gato para cada metal m — e essa"
           " assimetria de cardinalidade é mais uma coisa que os distingue sem tornar"
           " nenhum melhor",
           mal == 0 && nt == 3 && om == 0 && casos > 0);
    }

    printf("\n=== %d asserções, %d falhas, %ld estouros ===\n",
           unidades, falhas, sq_estouros);
    return falhas ? 1 : 0;
}
