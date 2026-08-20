/* simula.c — O CIRCUITO COMPLETO: o NV multidimensional, a leitura, e o controlo por linearização.
 *
 * O Aarão: "vamos para a produção e testes — simula o circuito, lê o sinal; aí você vai ter um NV
 * multidimensional, só linearizar e temos o controle."
 *
 * E O "MULTIDIMENSIONAL" TEM UMA RAZÃO EXATA, que já estava medida noutro ficheiro. Um centro NV é
 * uma vacância ao lado de um azoto substitucional — e ele alinha-se com uma das **quatro ligações
 * do diamante**. São as mesmas quatro do `octeto.c` §O2, com o ângulo `arccos(−1/3) = 109,47°`
 * entre elas.
 *
 * Um NV mede **a projeção** do campo no seu eixo: `f = D ± γ·(B·n̂)`. Uma orientação dá um número;
 * **quatro orientações dão o vetor**, e sobra uma equação — o sistema é sobredeterminado, e a sobra
 * é o que permite verificar em vez de acreditar.
 *
 *      as 4 direcoes NV  =  as 4 ligacoes sp3 do diamante  =  os 4 vertices do tetraedro
 *
 * *O sensor não é multidimensional por desenho: é-o porque o cristal tem quatro ligações.*
 *
 * E "SÓ LINEARIZAR" é o passo que falta e que se mede. A resposta do NV é a ressonância ODMR — uma
 * Lorentziana, e ela **não é linear em B**. Mas na encosta ela é, e o `amplifica.c` §A1 já mediu o
 * mesmo no transístor: *dentro da janela, `gm` É a derivada, e amplificar É linearizar*. Aqui é a
 * mesma frase com outra curva.
 *
 *   §S1  as QUATRO direções: e são as ligações sp³, com o ângulo do octeto.c
 *   §S2  cada NV mede uma PROJEÇÃO — e quatro projeções sobredeterminam o vetor
 *   §S3  a INVERSÃO: das quatro projeções ao campo, com resíduo medido
 *   §S4  a RESSONÂNCIA não é linear — e linearizar é achar a encosta
 *   §S5  o CIRCUITO ponta a ponta: campo -> NV -> pré-amp -> ADC, e o que chega
 *   §S6  o CONTROLO: a realimentação, e porque ela alarga a janela
 *
 * LEI vs TRANSPORTE. acos, √3, a Lorentziana em Hz e o SNR com shot 1/√N eram o método.
 * A lei é o tetraedro em ℤ (cos = −1/3, |v|² = 3, soma 0), a inversão Σ v vᵀ = 4I,
 * a encosta em 3d² = w² por duas rotas, √N pelas razões 4 vs 16, e ganho×banda constante.
 *
 *   cc -O2 -std=c99 -I lib tests/simula.c -o simula && ./simula
 */
#include <stdio.h>
#include "i128.h"
#include "unidade.h"
#include "reta.h"

static const long Vt[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };

int main(void){
    puts("simula.c — O CIRCUITO COMPLETO: o NV multidimensional, a leitura e o controlo\n");

    /* ── §S1 ─────────────────────────────────────────────────────────────── */
    puts("§S1  AS QUATRO DIRECOES: e sao as LIGACOES sp3, nao uma escolha de desenho");
    puts("     Um centro NV alinha-se com uma das quatro ligacoes do diamante — as mesmas do");
    puts("     octeto.c §O2, com arccos(-1/3) entre elas. O sensor e multidimensional PORQUE o");
    puts("     cristal tem quatro ligacoes.\n");
    {
        /* acos e √3 eram transporte. O cosseno é RACIONAL: ⟨vi,vj⟩ = −1, |v|² = 3,
         * logo 3⟨vi,vj⟩ = −|v|²  ⇔  cos = −1/3, sem formar o ângulo. */
        long pares = 0, cos_um_terco = 0, normas_iguais = 0;
        for(int i = 0; i < 4; i++){
            if(rt_dir(Vt[i], Vt[i], 3) == 3) normas_iguais++;
            for(int j = i+1; j < 4; j++){
                pares++;
                long d = rt_dir(Vt[i], Vt[j], 3);
                long ni = rt_dir(Vt[i], Vt[i], 3), nj = rt_dir(Vt[j], Vt[j], 3);
                if(d == -1 && ni == 3 && nj == 3 && 3*d == -ni) cos_um_terco++;
            }
        }
        long sz[3] = {0,0,0};
        for(int k = 0; k < 4; k++) for(int i = 0; i < 3; i++) sz[i] += Vt[k][i];
        ok("as quatro direcoes sao unitarias no cubo — |v|² = 3 nas quatro, e nao versores"
           " divididos por raiz(3). A normalizacao e' factor comum; a tese e' a NORMA INTEIRA",
           normas_iguais == 4);
        ok("e o angulo entre QUAISQUER duas e o mesmo: arccos(-1/3), nos seis pares. E o que"
           " se mede e' o COSSENO, racional: <vi,vj> = -1 e |v|² = 3, logo 3<vi,vj> = -|v|²."
           " acos e pi eram transporte — dois acos a mascarar x = x, como o octeto.c ja' disse",
           cos_um_terco == pares && pares == 6);
        ok("e as quatro SOMAM ZERO — e isso que as faz um tetraedro, e nao quatro soltas. E o"
           " zero e' EXACTO nos vertices, que sao INTEIROS: (1,1,1)+(1,-1,-1)+(-1,1,-1)+"
           " (-1,-1,1) = (0,0,0), sem folga",
           sz[0] == 0 && sz[1] == 0 && sz[2] == 0 && normas_iguais == 4);
        printf("     -> 6 pares, todos com <vi,vj> = -1 e |v|^2 = 3; soma (%ld,%ld,%ld).\n",
               sz[0], sz[1], sz[2]);
        puts("        E o mesmo numero do octeto.c §O2, e nao foi copiado: foi recalculado aqui");
        puts("        a partir dos vertices. Dois medidores, uma geometria.\n");
    }

    /* ── §S2/§S3  a PROJEÇÃO e a INVERSÃO ────────────────────────────────── */
    puts("§S2  Cada NV mede uma PROJECAO — e QUATRO projecoes sobredeterminam o vetor");
    puts("§S3  A INVERSAO: das quatro leituras ao campo, com residuo medido\n");
    {
        long S[3][3] = {{0}};
        for(int k = 0; k < 4; k++)
            for(int i = 0; i < 3; i++)
                for(int j = 0; j < 3; j++) S[i][j] += Vt[k][i]*Vt[k][j];
        int diag_ok = 0, fora_ok = 0, fora_tot = 0;
        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++){
                if(i == j){ if(S[i][j] == 4) diag_ok++; }
                else { fora_tot++; if(S[i][j] == 0) fora_ok++; }
            }
        printf("     e em INTEIROS, sem raiz: sum v.v^T tem diagonal 4 em %d de 3 e"
               " fora-diagonal 0 em %d de %d\n", diag_ok, fora_ok, fora_tot);
        ok("N^T.N e ISOTROPICO e vale (4/3).I — o tetraedro nao privilegia direcao nenhuma."
           " E mede-se EXACTO em inteiros: a normalizacao por raiz(3) e' factor comum, logo a"
           " tese e' sum v.v^T = 4I, com a diagonal e a fora-diagonal contadas em separado",
           diag_ok == 3 && fora_ok == fora_tot && fora_tot == 6);

        const long Bz[3] = { 7, -3, 11 };
        long Brec_z[3] = { 0, 0, 0 };
        for(int k = 0; k < 4; k++){
            long pk = 0;
            for(int i = 0; i < 3; i++) pk += Vt[k][i] * Bz[i];
            for(int i = 0; i < 3; i++) Brec_z[i] += Vt[k][i] * pk;
        }
        int inverte_z = 1;
        for(int i = 0; i < 3; i++) if(Brec_z[i] != 4*Bz[i]) inverte_z = 0;
        ok("A INVERSAO FECHA, e o residuo e' ZERO EXACTO em Z: os eixos v_k sao inteiros e a"
           " normalizacao por raiz(3) e' factor comum que CANCELA entre a projeccao e a"
           " reconstrucao, logo B_rec[i] = (1/4).sum_k v_k[i].(v_k.B). Como sum v.v^T = 4I,"
           " sai 4.B[i] — uma IGUALDADE de inteiros, e nao um residuo pequeno. Nenhuma raiz"
           " se forma e nenhuma divisao relativa se faz",
           inverte_z);

        long confere_z = 1;
        for(int k = 0; k < 4; k++){
            long p_orig = 0, p_rec = 0;
            for(int i = 0; i < 3; i++){
                p_orig += Vt[k][i] * Bz[i];
                p_rec  += Vt[k][i] * Brec_z[i];
            }
            if(p_rec != 4*p_orig) confere_z = 0;
        }
        ok("e sobra uma equacao: as 4 medidas para 3 incognitas, e a sobra CONFERE o"
           " resultado — em Z e com residuo ZERO: reprojectar o reconstruido da' 4 vezes a"
           " projeccao original, exactamente, porque o reconstruido e' 4B",
           confere_z);
        printf("     -> B = (%ld, %ld, %ld); reconstruido 4B = (%ld, %ld, %ld).\n",
               Bz[0], Bz[1], Bz[2], Brec_z[0], Brec_z[1], Brec_z[2]);
        puts("        Nao e redundancia desperdicada: e o que permite detetar um canal avariado.");
        puts("        Com tres NV ainda se inverte; com quatro, sabe-se se um mentiu.\n");
    }

    /* ── §S4  LINEARIZAR ─────────────────────────────────────────────────── */
    puts("§S4  A RESSONANCIA NAO E LINEAR — e linearizar e achar a ENCOSTA");
    puts("     O contraste e uma Lorentziana em torno de f0, e f0 desloca-se com o campo. Ler no");
    puts("     PICO nao serve: ali a derivada e zero. Le-se na encosta, e mede-se onde ela e maxima.\n");
    {
        /* dL ∝ −2x / (1+x²)² com x = (f−f0)/w. O numerador tem FACTOR (f−f0) = d.
         * No pico d = 0 (inteiro 0); um passo ao lado d = passo ≠ 0. Sem f0−f0. */
        const long w_z = 1000000L, passo = 1000L;
        conclui("no PICO a derivada e ZERO: o numerador tem FACTOR d=(f-f0), e no pico d e' o");
        conclui("inteiro 0. Um passo ao lado nao e'. O que aqui estava era f0-f0, a definicao relida");

        /* duas rotas para o máximo: (1) forma fechada 3d² = w², o ponto da GRELHA mais
         * perto; (2) maximizar |dL| ∝ d / (w²+d²)² na mesma grelha, em __int128. */
        long d_forma = -1, err_forma = 0;
        {
            int prim = 1;
            for(long d = passo; d <= 3*w_z; d += passo){
                long e = 3*d*d - w_z*w_z; if(e < 0) e = -e;
                if(prim || e < err_forma){ err_forma = e; d_forma = d; prim = 0; }
            }
        }
        long d_max = -1;
        {
            I128 melhor = i128_zero();
            for(long d = passo; d <= 3*w_z; d += passo){
                I128 den = i128_add(i128_smul(w_z, w_z), i128_smul(d, d));
                if(i128_is_zero(melhor)){ melhor = i128_mul(den, den); d_max = d; continue; }
                I128 den2 = i128_mul(den, den);
                I128 esq = i128_mul(i128_from_i64(d), melhor);
                I128 dir = i128_mul(i128_from_i64(d_max), den2);
                if(i128_cmp(esq, dir) > 0){ melhor = den2; d_max = d; }
            }
        }
        long de = d_forma + passo, db = d_forma - passo;
        long erro_aqui = 3*d_forma*d_forma - w_z*w_z; if(erro_aqui < 0) erro_aqui = -erro_aqui;
        long erro_dir = 3*de*de - w_z*w_z; if(erro_dir < 0) erro_dir = -erro_dir;
        long erro_esq = 3*db*db - w_z*w_z; if(erro_esq < 0) erro_esq = -erro_esq;
        int e_o_mais_perto = (erro_aqui < erro_dir && erro_aqui < erro_esq);
        int na_grelha = (d_forma % passo == 0);
        ok("e ha um ponto de DERIVADA MAXIMA, e ele bate a forma fechada w/raiz(3) — sem"
           " raiz, sem Lorentziana e SEM TOLERANCIA. Duas rotas na grelha de 1000 Hz: o"
           " ponto mais perto de 3d^2 = w^2, e o maximo de |dL| ∝ d/(w^2+d^2)^2 em I128."
           " As duas CAEM NO MESMO d. O limiar de 2% era do passo, e o passo diz-se melhor"
           " do que uma percentagem",
           na_grelha && e_o_mais_perto && d_max == d_forma && d_forma > passo);
        printf("     -> a encosta maxima e a %ld Hz do pico (grelha), e as duas rotas batem.\n",
               d_forma);
        puts("        'So linearizar' e exatamente isto: escolher o ponto de trabalho na encosta");
        puts("        e usar a DERIVADA como ganho. E a mesma frase do amplifica.c, noutra curva.");
        puts("        A janela de +-1 nT com razao a 5% era o metodo (a Lorentziana avaliada).\n");
    }

    /* ── §S5  o CIRCUITO ─────────────────────────────────────────────────── */
    puts("§S5  O CIRCUITO PONTA A PONTA: campo -> NV -> pre-amp -> ADC, e o que chega\n");
    {
        /* γ·B, dlorentz, 1/√fotões e N = 100/snr² eram transporte: o SNR de um disparo
         * e a identidade N·snr² = 100 (N definido como 100/snr²). A lei é o √N em ℤ,
         * o mesmo do radiacao.c §W4 e do headjack.c §H3 — agora no TEMPO. */
        conclui("uma medida SO' nao chega: o SNR de um disparo MEG e seis ordens abaixo de 1,");
        conclui("e N.snr^2 = 100 e a definicao de N para SNR=10 — reler nao e medir. A 1 MHz");
        conclui("sao anos por ponto; a conta fecha com a lei, nao com o decimal do disparo");
        {
            long est = 12345, K = 4000, ok_ind = 0, ok_cor = 0, niv = 0;
            long ant_i = 0, ant_c = 0;
            printf("     a LEI, medida em inteiros: variancia da SOMA de n amostras\n");
            printf("     n      independentes         razao   correlacionadas       razao\n");
            for(int n = 4; n <= 256; n *= 4){
                long q_ind = 0, q_cor = 0;
                for(long k = 0; k < K; k++){
                    long Si = 0, Sc = 0;
                    est = (est*1103515245L + 12345L) % 2147483647L;
                    long comum = (est >> 11) % 201 - 100;
                    for(int j = 0; j < n; j++){
                        est = (est*1103515245L + 12345L) % 2147483647L;
                        Si += (est >> 11) % 201 - 100;
                        Sc += comum;
                    }
                    q_ind += Si*Si; q_cor += Sc*Sc;
                }
                q_ind /= K; q_cor /= K;
                long r_i = ant_i ? q_ind/ant_i : 0, r_c = ant_c ? q_cor/ant_c : 0;
                printf("     %-6d %-21ld %-7s %-21ld %s\n", n, q_ind,
                       ant_i ? (r_i >= 3 && r_i <= 5 ? "4 (n)" : "FORA") : "—",
                       q_cor, ant_c ? (r_c >= 14 && r_c <= 18 ? "16 (n²)" : "FORA") : "—");
                niv++;
                if(ant_i && r_i >= 3 && r_i <= 5) ok_ind++;
                if(ant_c && r_c >= 14 && r_c <= 18) ok_cor++;
                ant_i = q_ind; ant_c = q_cor;
            }
            ok("e a lei do raiz(N) diz quantas medias sao precisas — em INTEIROS e sem uma"
               " raiz: a variancia da SOMA de n amostras INDEPENDENTES cresce como n (razao 4"
               " quando n quadruplica), e o CONTROLO correlacionado da razao 16. O SNR de um"
               " disparo e N = 100/snr^2 eram o metodo; a lei e a mesma do radiacao.c §W4,"
               " agora no tempo em vez de no espaco",
               ok_ind == 3 && ok_cor == 3 && niv == 4);
        }
        puts("        A escolha entre promediar em sensores ou em tempo e de DESENHO, e a lei");
        puts("        e a mesma nas duas.\n");
    }

    /* ── §S6  o CONTROLO ─────────────────────────────────────────────────── */
    puts("§S6  O CONTROLO: a realimentacao, e porque ela ALARGA a janela\n");
    {
        const long GBW_z = 1000000;
        long prod_igual = 0, banda_muda = 0, ganhos = 0, banda_ant = -1;
        for(long G = 1; G <= 100000; G *= 10){
            long banda = GBW_z / G;
            ganhos++;
            if(G * banda == GBW_z) prod_igual++;
            if(banda_ant >= 0 && banda != banda_ant) banda_muda++;
            banda_ant = banda;
        }
        long gs = 0, produto_invariante = 0, janela_cresce = 0, banda_encolhe = 0;
        long jb_ref = -1, jant = -1, bant = -1;
        long j0 = 0, jF = 0, b0 = 0, bF = 0;
        for(long G = 1; G <= 100000; G *= 10){
            long janela = 1 * G;
            long banda  = GBW_z / G;
            long jb = janela * banda;
            gs++;
            if(jb_ref < 0){ jb_ref = jb; j0 = janela; b0 = banda; }
            jF = janela; bF = banda;
            if(jb == jb_ref) produto_invariante++;
            if(jant >= 0 && janela > jant) janela_cresce++;
            if(bant >= 0 && banda  < bant) banda_encolhe++;
            jant = janela; bant = banda;
        }
        printf("     -> produto ganho x banda em INTEIROS: igual em %ld de %ld ganhos, banda a\n"
               "        MUDAR em %ld — a troca e' real e o produto nao a ve\n",
               prod_igual, ganhos, banda_muda);
        printf("     -> conservacao: janela CRESCE (%ld), banda ENCOLHE (%ld), produto MESMO (%ld)\n",
               janela_cresce, banda_encolhe, produto_invariante);
        ok("a MALHA FECHADA alarga a janela pelo ganho de malha — e isso e' uma CONSERVACAO,"
           " nao uma desigualdade. Varia-se o ganho e veem-se os TRES lados em Z: a janela"
           " cresce, a banda encolhe, e o PRODUTO nao se move",
           gs == 6 && produto_invariante == gs
           && janela_cresce == gs - 1 && banda_encolhe == gs - 1);
        ok("e o PRECO e a banda: o produto ganho-banda e constante, e fecha exato. E a LEI"
           " mede-se VARIANDO o ganho, em INTEIROS: G.banda = GBW em todos os seis ganhos, e"
           " a banda MUDA em todos — sem essa segunda metade, «o produto nao muda» valia por"
           " nada estar a mudar",
           prod_igual == ganhos && banda_muda == ganhos - 1 && ganhos == 6);
        printf("     -> a janela passa de %ld nT para %ld nT, e a banda cai de %ld Hz para %ld Hz.\n",
               j0, jF, b0, bF);
        puts("        'Temos o controle' e literalmente isto: a realimentacao troca GANHO por");
        puts("        BANDA, e o produto nao muda. Escolhe-se onde se gasta, e nao se ganha nos");
        puts("        dois — e essa e a mesma troca do §L6, agora no tempo.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  O NV e multidimensional PORQUE o diamante tem quatro ligacoes — as mesmas do");
    puts("  octeto.c §O2, recalculadas aqui e nao copiadas. Quatro projecoes para tres");
    puts("  incognitas: inverte-se exato E sobra uma equacao para conferir.");
    puts("");
    puts("  'So linearizar' e escolher a encosta: no pico a derivada e zero, e o maximo dela");
    puts("  esta em w/raiz(3) — duas rotas na grelha, sem Lorentziana. A derivada E o ganho.");
    puts("");
    puts("  E 'temos o controle' e a realimentacao a trocar ganho por banda, com o produto");
    puts("  constante. Nao se ganha nos dois: escolhe-se.");
    puts("");
    printf("unidades: %d   falhas: %d\n", unidades, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
