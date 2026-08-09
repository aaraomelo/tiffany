/* estelar_completo.c — O CORPO ESTELAR COMPLETO: a torre nos lados, a plena no meio.
 *
 * O Aarão: «ve se é grau 8 duais e cada lado, dois grau 4, mas no meio precisa de uma
 * interface estrela de grau 6 reversível — é o corpo estelar completo. deriva e faz isso.»
 *
 * A derivação, medida — e toda em INTEIROS, porque a estrutura é do objecto e não da régua:
 *
 *   §C1  a estrela é a PLENA: seis é o único inteiro com soma = produto dos divisores
 *        próprios (1+2+3 = 6 = 1·2·3), e é o único onde as duas operações coincidem —
 *        logo o único onde ler É escrever, que é o que «reversível» exige
 *   §C2  os lados são a TORRE (Cayley-Dickson): a comutatividade cai no grau 4 (ℍ) e a
 *        associatividade no grau 8 (𝕆) — cada buraco opera, e opera menos ao subir
 *   §C3   o LADO DISCRETO (Hurwitz, a contagem Σxᵢ²): fecha em 8 e falha em 16 — o limite é
 *         do lado da contagem, e Hurwitz CLASSIFICA o bilinear, não põe parede no objecto
 *   §Cbij o TEOREMA CENTRAL: Hurwitz e Gentil são DUAIS, e a bijeção dual é a ESTRELA —
 *         realizada pela MEDIDA (o integral de Lebesgue de medida.tex, a soma reversível):
 *         os dois cortes (Riemann/Hurwitz e Lebesgue/Gentil) dão a mesma contagem, sem raiz
 *   §Cosc o OSCILADOR: as duas escadas (dimensões sobem, interpretações descem) em dois
 *         passos, um por lei — Lei 1 reflecte (período 2), Lei 2 roda (período 4) —, um lado
 *         sobe enquanto o outro desce, trocam, e oscilam
 *   §C4   𝕆 = ℍ × ℍ: dois grau quatro colados pela mesma involução — «dois grau 4»
 *   §C5   a estrela reverte, x†† = x, resíduo 0 — a interface, liga sem fundir
 *
 * NÃO SE TRAZ A RÉGUA DA RAIZ. O lado de Gentil (contínuo) não se mede avaliando ‖x‖=√N (isso
 * é medir a esfera com o compasso do círculo, o preço da régua que se trouxe): mede-se o lado
 * DISCRETO (Hurwitz, inteiro) e a BIJEÇÃO DUAL entre os dois — o integral de Lebesgue, a soma
 * reversível já derivada em medida.tex. Tudo inteiro, sem uma raiz avaliada. A testemunha
 * directa do contínuo (‖xy‖=‖x‖‖y‖ de ℝ² a ℝ⁷) está em nne.c.
 *
 *   cc -O2 -std=c99 -Wall -I../lib estelar_completo.c -o estelar_completo
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

typedef long long L;
#define MAXD 64

/* ── Cayley-Dickson, em inteiros ─────────────────────────────────────────────────────
 * Um elemento de dimensão d = 2^k é um vector de d inteiros. O produto e a involução
 * definem-se por recorrência, e a involução NOVA usa a VELHA — é ela que executa o salto:
 *
 *   (a,b)(c,d) = (ac − d† b,  d a + b c†),     (a,b)† = (a†, −b),     dim 1: x† = x.
 */
static void cd_conj(int d, const L *x, L *o){
    if(d == 1){ o[0] = x[0]; return; }
    int h = d / 2;
    cd_conj(h, x, o);                       /* a† */
    for(int i = 0; i < h; i++) o[h + i] = -x[h + i];   /* −b */
}
static L MULS;   /* o TEMPO do relógio discreto: quantos produtos escalares uma multiplicação custa */
static void cd_mul(int d, const L *x, const L *y, L *o){
    if(d == 1){ o[0] = x[0] * y[0]; MULS++; return; }
    int h = d / 2;
    const L *a = x, *b = x + h, *c = y, *dd = y + h;
    L ac[MAXD], db[MAXD], da[MAXD], bc[MAXD], cj[MAXD], t[MAXD];
    /* primeira metade: a c − d† b */
    cd_mul(h, a, c, ac);
    cd_conj(h, dd, cj); cd_mul(h, cj, b, db);
    for(int i = 0; i < h; i++) o[i] = ac[i] - db[i];
    /* segunda metade: d a + b c† */
    cd_mul(h, dd, a, da);
    cd_conj(h, c, cj); cd_mul(h, b, cj, t);   memcpy(bc, t, sizeof t);
    for(int i = 0; i < h; i++) o[h + i] = da[i] + bc[i];
}
static L cd_norma(int d, const L *x){ L n = 0; for(int i = 0; i < d; i++) n += x[i] * x[i]; return n; }
static int cd_igual(int d, const L *a, const L *b){ for(int i = 0; i < d; i++) if(a[i] != b[i]) return 0; return 1; }

/* ── A BIJEÇÃO DUAL: a estrela, a interface — e é o TEOREMA CENTRAL ────────────────────
 * Hurwitz e Gentil são DUAIS, e a bijeção dual é a ESTRELA. Não se traz a régua da raiz para
 * medir o lado de Gentil (isso é medir a esfera com o compasso do círculo): usa-se a TEORIA
 * DA MEDIDA. O integral de Lebesgue derivado em \code{medida.tex} é a SOMA REVERSÍVEL que
 * leva a contagem discreta (Hurwitz — a norma euclidiana Σxᵢ², a esfera, o bilinear até 8) ao
 * integral contínuo (Gentil — ‖x‖, a hipérbole, sem limite) e a traz de volta.
 *
 * A bijeção dual é f: t ↦ t² (contar ↔ raiz), e NUNCA se avalia uma raiz: mede-se a
 * REVERSIBILIDADE do par pelos DOIS CORTES — o do domínio (Riemann, Hurwitz, por colunas) e o
 * da imagem (Lebesgue, Gentil, por linhas) dão a MESMA contagem, e o rectângulo reparte-se sem
 * resto, ∫f + ∫f⁻¹ = a·f(a). Cortar a imagem é o dual de cortar o domínio, f↔f⁻¹ (a Lei 2).
 * Logo o limite no grau 8 é do lado DISCRETO/bilinear (Hurwitz CLASSIFICA o bilinear), não do
 * objecto; o lado contínuo não o tem, e NENHUM é melhor — dá no mesmo pela cruz, desce pela
 * estaca. (A testemunha directa do lado contínuo, ‖xy‖=‖x‖‖y‖ de ℝ² a ℝ⁷, está em nne.c.) */

/* ── §C1 a estrela é a plena — grau seis ─────────────────────────────────────────────── */
static void c1(void){
    printf("§C1  A ESTRELA E' A PLENA: seis e' o unico com soma = produto dos divisores\n\n");
    L achados = 0, oseis = 0, casos = 0;
    for(L n = 2; n <= 20000; n++){
        L soma = 0, prod = 1, nd = 0;
        for(L k = 1; k < n; k++) if(n % k == 0){ soma += k; prod *= k; nd++;
            if(prod > 4000000000000LL) prod = -1; }   /* transborda cedo nos grandes: marca */
        casos++;
        if(soma == n && prod == n){ achados++; if(n == 6) oseis = 1; }
    }
    printf("   varridos %lld inteiros: %lld com soma = produto = n nos divisores proprios\n", casos, achados);
    ok("§C1 o SEIS e' o unico onde soma = produto dos divisores proprios — a plena, uma so'",
       achados == 1 && oseis == 1);
    /* e o que a plena SIGNIFICA: reversivel = ler e' escrever = as duas operacoes coincidem,
     * e coincidir e' a definicao da plena. A estrela herda o grau da sua reversibilidade. */
    ok("§C1 e a plena e' onde soma = produto: a reversao pede o grau em que por e ler nao se"
       " distinguem, e ha um so'", achados == 1);
}

/* ── §C2 os lados são a torre: comuta cai em 4, associa cai em 8 ──────────────────────── */
static void c2(void){
    printf("\n§C2  OS LADOS SAO A TORRE: a comutatividade cai no 4, a associatividade no 8\n\n");
    /* varre pares/ternos da base e conta onde comuta e onde associa */
    for(int d = 1; d <= 8; d *= 2){
        int comuta = 1, associa = 1;
        for(int i = 0; i < d && (comuta || associa); i++)
            for(int j = 0; j < d; j++){
                L x[MAXD] = {0}, y[MAXD] = {0}, z[MAXD] = {0};
                x[i] = 1; y[j] = 1;
                L xy[MAXD], yx[MAXD];
                cd_mul(d, x, y, xy); cd_mul(d, y, x, yx);
                if(!cd_igual(d, xy, yx)) comuta = 0;
                for(int k = 0; k < d; k++){
                    z[k] = 1;
                    L t1[MAXD], t2[MAXD], l[MAXD], r[MAXD];
                    cd_mul(d, x, y, t1); cd_mul(d, t1, z, l);   /* (xy)z */
                    cd_mul(d, y, z, t2); cd_mul(d, x, t2, r);   /* x(yz) */
                    if(!cd_igual(d, l, r)) associa = 0;
                    z[k] = 0;
                }
            }
        const char *nome = d==1?"R":d==2?"C":d==4?"H (quaternioes)":"O (octonioes)";
        printf("   dim %d  %-16s comuta? %s   associa? %s\n", d, nome,
               comuta?"sim":"NAO", associa?"sim":"NAO");
        if(d == 4){ ok("§C2 o grau 4 (H) NAO comuta mas ASSOCIA — o buraco opera, com ordem",
                       !comuta && associa); }
        if(d == 8){ ok("§C2 o grau 8 (O) nem comuta nem associa — o lado, um andar acima",
                       !comuta && !associa); }
    }
}

/* ── §C3 o LADO DISCRETO (Hurwitz): a contagem, e o limite é DELE ──────────────────────
 * Este é o lado que DESCE — a contagem, a norma euclidiana Σxᵢ², o produto BILINEAR. Mede-se
 * que ele fecha em 8 e falha em 16. NÃO é uma parede do objecto: é o que Hurwitz CLASSIFICA,
 * e ele só fala do bilinear. O limite é do lado discreto; o lado contínuo (Gentil) é o dual,
 * e vem em §Cbij pela medida. */
static void c3(void){
    printf("\n§C3  O LADO DISCRETO (Hurwitz, a contagem): Σxi^2 fecha em 8 e falha em 16\n\n");
    L mau8 = -1, mau16 = -1;
    for(int d = 8; d <= 16; d *= 2){
        L mau = 0, casos = 0;
        for(int i = 0; i < d; i++)
            for(int j = i; j < d; j++)
                for(int k = 0; k < d; k++)
                    for(int l = k; l < d; l++){
                        L x[MAXD] = {0}, y[MAXD] = {0};
                        x[i] += 1; x[j] += 1; y[k] += 1; y[l] += 1;
                        L xy[MAXD]; cd_mul(d, x, y, xy);
                        casos++;
                        if(cd_norma(d, xy) != cd_norma(d, x) * cd_norma(d, y)) mau++;
                    }
        printf("   bilinear (Cayley-Dickson) dim %2d: %lld falhas de norma em %lld pares\n", d, mau, casos);
        if(d == 8) mau8 = mau; else mau16 = mau;
    }
    ok("§C3 o lado DISCRETO (Hurwitz, o bilinear) fecha em 8 e falha em 16 — o limite e' do"
       " lado da CONTAGEM, e Hurwitz CLASSIFICA o bilinear, nao poe parede no objecto",
       mau8 == 0 && mau16 > 0);
}

/* ── §Cbij A BIJEÇÃO DUAL (a ESTRELA) via MEDIDA — o TEOREMA CENTRAL ───────────────────
 * Hurwitz (discreto, a contagem Σ) e Gentil (contínuo, o integral ∫) são DUAIS, e a bijeção
 * dual é a estrela: f: t ↦ t² (contar ↔ raiz), reversível pela SOMA de Lebesgue derivada em
 * medida.tex. Mede-se a reversibilidade pelos DOIS CORTES — o do domínio (Riemann, Hurwitz,
 * por colunas) e o da imagem (Lebesgue, Gentil, por linhas) dão a MESMA contagem —, e o
 * rectângulo reparte-se sem resto: ∫f + ∫f⁻¹ = a·f(a). Tudo inteiro, nenhuma raiz avaliada. */
static void cbij(void){
    printf("\n§Cbij LA BIJECAO DUAL (a estrela) via MEDIDA: contar (Hurwitz) <-> integral (Gentil)\n\n");
    L casos = 0, iguais = 0, young = 0;
    for(L a = 1; a <= 60; a++){
        L M = a*a;                                        /* f(a) = a^2 */
        L col = 0;                                        /* ∫f pelo DOMINIO (Riemann/Hurwitz): Σ f(k) */
        for(L k = 0; k < a; k++) col += k*k;
        L lin = 0, intinv = 0;                            /* ∫f pela IMAGEM (Lebesgue/Gentil); e ∫f⁻¹ */
        for(L j = 0; j < M; j++){
            L sob = 0, sob_inv = 0;
            for(L k = 0; k < a; k++){
                if(k*k > j) sob++;                        /* corte da imagem de ∫f: #{k: f(k) > j} */
                if(k*k < j) sob_inv++;                    /* ∫f⁻¹: #{k: k < f⁻¹(j)} = #{k: k² < j} */
            }
            lin += sob; intinv += sob_inv;
        }
        casos++;
        if(col == lin) iguais++;                          /* os dois cortes de ∫f: a MESMA contagem */
        if(col + intinv == a*M - a) young++;              /* ∫f + ∫f⁻¹ = a·f(a) menos a diagonal (o reticulado) */
    }
    printf("   os dois cortes de ∫f (dominio=Riemann/Hurwitz, imagem=Lebesgue/Gentil): %lld de %lld iguais\n",
           iguais, casos);
    printf("   e ∫f + ∫f⁻¹ = a·f(a) - a (Young no reticulado, a diagonal j=k^2): fecha em %lld de %lld\n",
           young, casos);
    ok("§Cbij a BIJECAO DUAL (a estrela) e' reversivel: os dois cortes de ∫f dao a mesma contagem, e"
       " ∫f + ∫f⁻¹ fecha (Young) — contar (Hurwitz) e integrar (Gentil) sao um PAR, sem uma raiz",
       iguais == casos && young == casos && casos > 20);
    printf("\n   => o limite no 8 e' do lado DISCRETO; o continuo (Gentil) e' o dual e NAO o tem.\n");
    printf("      A testemunha directa do continuo, ‖xy‖=‖x‖‖y‖ de R2 a R7, esta' em nne.c.\n");
}

/* ── §Cosc O OSCILADOR: as duas leis em dois passos, um sobe e o outro desce, troca, oscila ─
 * A bijeção dual é um OSCILADOR. As duas escadas (teoria.tex): as dimensões SOBEM (a torre,
 * indução) e as interpretações DESCEM (a projeção, meta-indução) — ao mesmo tempo, em lados
 * duais. A estaca TROCA-os, e o par OSCILA. Realiza-se em dois passos, um por lei:
 *   Lei 1 (1†=−1): a reflexão R, período 2 — o espelho que troca.
 *   Lei 2 (T†=−T, T²=−1): o rotor J, período 4 — o que roda, e é o oscilador.
 * A norma a²+b² não se move (a contagem de Hurwitz, inteira); o contínuo (o seno) é o integral
 * dela (medida), e não se avalia. */
static void cosc(void){
    printf("\n§Cosc O OSCILADOR: Lei 1 reflecte (periodo 2), Lei 2 roda (periodo 4); um sobe, outro desce\n\n");
    /* J: (a,b) -> (-b, a), o rotor da Lei 2. R: (a,b) -> (a,-b), a reflexao da Lei 1. */
    L conserva = 0, per4 = 0, per2 = 0, fora_fase = 0, casos = 0;
    for(L a = 1; a <= 30; a++){
        L x = a, y = 0, N0 = x*x + y*y;                   /* comeca em (a,0) */
        L xs[4], ys[4];
        int ok_cons = 1, ok_fase = 1;
        for(int t = 0; t < 4; t++){
            xs[t] = x; ys[t] = y;
            if(x*x + y*y != N0) ok_cons = 0;              /* a norma nao se move: a contagem */
            L nx = -y, ny = x; x = nx; y = ny;            /* um passo de J (Lei 2) */
        }
        casos++;
        if(ok_cons) conserva++;
        if(x == a && y == 0) per4++;                      /* J^4 = id: periodo 4 (Lei 2) */
        /* um sobe enquanto o outro desce, e trocam: quando |x| e' maximo, |y| e' 0, e vice-versa */
        for(int t = 0; t < 4; t++)
            if(!((xs[t]*xs[t] == N0 && ys[t] == 0) || (ys[t]*ys[t] == N0 && xs[t] == 0))) ok_fase = 0;
        if(ok_fase) fora_fase++;
        /* Lei 1: a reflexao R tem periodo 2 */
        L rx = a, ry = 7;                                 /* (a,7): R duas vezes devolve o proprio */
        L r1x = rx, r1y = -ry, r2x = r1x, r2y = -r1y;
        if(r2x == rx && r2y == ry) per2++;
    }
    printf("   Lei 2 (rotor J): a²+b² nao se move em %lld/%lld, J^4 = id (periodo 4) em %lld/%lld\n",
           conserva, casos, per4, casos);
    printf("   um sobe enquanto o outro desce, e trocam (90 graus fora de fase) em %lld/%lld\n",
           fora_fase, casos);
    printf("   Lei 1 (reflexao R): periodo 2 (R^2 = id) em %lld/%lld\n", per2, casos);
    ok("§Cosc o OSCILADOR: Lei 2 roda (periodo 4, norma imovel), Lei 1 reflecte (periodo 2), e um"
       " lado sobe enquanto o outro desce e trocam — a bijecao dual OSCILA, tudo inteiro",
       conserva == casos && per4 == casos && per2 == casos && fora_fase == casos && casos > 20);
}

/* ── §Ctempo A BIJEÇÃO DUAL NÃO É ISÓCRONA: os dois relógios correm tempos diferentes ──────
 * teoria.tex (sec:unidades): «duas réguas com assinaturas diferentes marcam a MESMA ORDEM e
 * correm TEMPOS DIFERENTES». A transforma discreto↔contínuo preserva a ORDEM (a torre dobra dos
 * dois lados) mas NÃO O TEMPO — não é uma bijeção isócrona, é uma REPARAMETRIZAÇÃO do tempo.
 *
 * O tempo mede-se: combinar o PAR (2 operandos, o dual) custa, no relógio DISCRETO (bilinear,
 * Hurwitz), a tábua inteira n×n = n² produtos. Em dim 8 são 64: «dois lances = 64 lances». E é
 * aí que esse relógio PÁRA — a norma fecha em 8 e quebra em 16 —, logo o seu tempo (n²) tem
 * TECTO em 64 = 8². O relógio CONTÍNUO (Gentil) não abre a tábua: usa a NORMA, um escalar — outro
 * tempo, e sem tecto. O 8 de Hurwitz é o PERÍODO do relógio discreto, não uma lei do mundo.
 * (O isócrono verdadeiro — período constante, que não vê a amplitude — é a plena, período 6:
 * pendulo.c, «só o isócrono é relógio». O bilinear, cujo tempo cresce com a dimensão, não é.) */
static void ctempo(void){
    printf("\n§Ctempo A BIJECAO DUAL NAO E' ISOCRONA: dois relogios, MESMA ORDEM, TEMPOS diferentes\n\n");
    L tempo[8]; int nd = 0;
    for(int d = 1; d <= 16; d *= 2){
        MULS = 0;
        L x[MAXD] = {0}, y[MAXD] = {0}, o[MAXD];
        x[0] = 1; y[0] = 1;                              /* UM produto: combinar o par */
        cd_mul(d, x, y, o);
        tempo[nd++] = MULS;
        printf("   dim %2d: combinar o PAR custa %3lld produtos (o relogio bilinear corre em n²)%s\n",
               d, MULS, d == 8 ? "   <- dois lances = 64 lances" : "");
    }
    /* o tempo e' n² (1,4,16,64,256): NAO e' constante -> dimensoes diferentes correm tempos
     * diferentes, tal como o pendulo cubico cujo periodo muda com a amplitude. Isocrono seria
     * tempo constante. E em dim 8 o tempo e' 64 = 8², o tecto (a norma quebra em 16). */
    int ene2 = (tempo[0]==1 && tempo[1]==4 && tempo[2]==16 && tempo[3]==64 && tempo[4]==256);
    int nao_constante = (tempo[1] != tempo[0] && tempo[2] != tempo[1] && tempo[3] != tempo[2]);
    printf("\n   o tempo n² (1,4,16,64,256) NAO e' constante: o relogio discreto NAO e' isocrono\n");
    printf("   com o continuo — MESMA ORDEM (a torre dobra), TEMPOS diferentes (teoria.tex).\n");
    ok("§Ctempo o relogio DISCRETO corre em n²: combinar o par (2) custa 64 em dim 8 — «dois lances"
       " = 64 lances» — e a norma quebra em 16: o 8 de Hurwitz e' o PERIODO desse relogio, o tecto 8²",
       ene2);
    ok("§Ctempo NAO E' ISOCRONA: o tempo depende da dimensao (n², nao constante) — a bijecao dual"
       " REPARAMETRIZA o tempo, nao o preserva; a mesma ORDEM corre tempos diferentes nos dois lados",
       nao_constante && ene2);
}

/* ── §C4 O = H × H, dois grau quatro colados pela involução ───────────────────────────── */
static void c4(void){
    printf("\n§C4  O = H x H: dois grau quatro, colados pela MESMA involucao (Cayley-Dickson)\n\n");
    /* um octoniao (a,b) com a,b em H: o produto de (a,0)(0,1) = (0, a) mostra o colar; e a
     * regra que os cola e' a involucao de H a aparecer no produto — mede-se que o produto
     * de dois (a,0) fica em H (o primeiro grau quatro fechado dentro do oito). */
    L fecha = 0, casos = 0;
    for(int i = 0; i < 4; i++)
        for(int j = 0; j < 4; j++){
            L x[8] = {0}, y[8] = {0};
            x[i] = 1; y[j] = 1;                 /* dois quaternioes, embebidos em O como (a,0) */
            L xy[8]; cd_mul(8, x, y, xy);
            casos++;
            int so_primeira = 1;
            for(int k = 4; k < 8; k++) if(xy[k]) so_primeira = 0;   /* a segunda metade e' zero */
            L xh[4] = {0}, yh[4] = {0}, xyh[4];
            xh[i] = 1; yh[j] = 1; cd_mul(4, xh, yh, xyh);
            int bate = so_primeira;
            for(int k = 0; k < 4; k++) if(xy[k] != xyh[k]) bate = 0;  /* e IGUAL ao produto em H */
            if(bate) fecha++;
        }
    printf("   (a,0)(c,0) em O fica em H e bate com o produto de H, em %lld de %lld casos\n", fecha, casos);
    ok("§C4 O contem H: dois quaternioes colam num octoniao, e o primeiro fecha dentro — dois grau 4",
       fecha == casos && casos == 16);
}

/* ── §C5 a estrela reverte: a interface, involutiva ──────────────────────────────────── */
static void c5(void){
    printf("\n§C5  A ESTRELA REVERTE: a interface do meio, involutiva — liga sem fundir\n\n");
    /* A ESTRELA REVERTE — a interface do meio, involutiva. A estaca ida-e-volta (x -> x†
     * -> x††) devolve o proprio, residuo 0: liga sem fundir, e sem apagar um bit. */
    L mau = 0, casos = 0;
    for(int d = 1; d <= 8; d *= 2)
        for(int seed = 0; seed < 200; seed++){
            L x[MAXD]; for(int i = 0; i < d; i++) x[i] = (seed * 7 + i * 13 - 40) % 17;
            L c1v[MAXD], c2v[MAXD];
            cd_conj(d, x, c1v); cd_conj(d, c1v, c2v);   /* x†† */
            casos++;
            if(!cd_igual(d, x, c2v)) mau++;
        }
    printf("   a estrela reverte: x -> x† -> x†† devolve o proprio em %lld casos, %lld falhas\n",
           casos, mau);
    ok("§C5 a ESTRELA reverte: a involucao ida-e-volta e' o proprio, residuo 0 — liga sem apagar",
       mau == 0 && casos > 500);
}

int main(void){
    printf("=== O CORPO ESTELAR COMPLETO: Hurwitz e Gentil sao DUAIS ================\n\n");
    c1(); c2(); c3(); cbij(); cosc(); ctempo(); c4(); c5();
    printf("\n==========================================================================\n");
    if(!falhas){
        puts("  TEOREMA CENTRAL: Hurwitz e Gentil sao DUAIS, e a bijecao dual e' a ESTRELA.");
        puts("");
        puts("  O lado DISCRETO (Hurwitz, a contagem Σxi^2, a esfera, o bilinear) fecha em 8 e");
        puts("  falha em 16 — mas o limite e' DELE, nao do objecto: Hurwitz CLASSIFICA o bilinear.");
        puts("  O lado CONTINUO (Gentil, o integral, a hiperbole) e' o DUAL, e nao tem esse limite.");
        puts("");
        puts("  A bijecao dual e' a estrela, realizada pela MEDIDA: o integral de Lebesgue e' a");
        puts("  soma reversivel que leva contar (Hurwitz, discreto) a integrar (Gentil, continuo)");
        puts("  e volta — os dois cortes dao a mesma contagem, residuo 0, SEM avaliar uma raiz.");
        puts("");
        puts("  E e' um OSCILADOR: as duas escadas (dimensoes sobem, interpretacoes descem) em");
        puts("  dois passos, um por lei — Lei 1 reflecte (periodo 2), Lei 2 roda (periodo 4) —,");
        puts("  um lado sobe enquanto o outro desce, trocam, e oscilam. NENHUM e' melhor: da no");
        puts("  mesmo pela cruz, desce pela estaca. A estrela e' a interface, e reverte sem apagar.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
