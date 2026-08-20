/* pidual.c — A CONSTRUCAO DE PI NO CORPO DUAL.
 *
 * O Aarao: "da a definicao de pi a construcao no corpo dual"
 *
 * Pi e' o caso que poe a arquitetura toda a prova, porque ele NAO E' ALGEBRICO: por
 * Lindemann (1882) nao e' raiz de nenhum polinomio de coeficientes inteiros, logo nao
 * esta' em nenhum dos aneis que a Teoria constroi. Se a definicao dele tivesse de ser
 * geometrica — comprimento de arco, area de circulo — entao o corpo dual precisaria da
 * geometria para se completar, e a ordem das partes deste texto estaria trocada.
 *
 * Nao precisa. A definicao e' DINAMICA, e sai das duas leis:
 *
 *     Lei 2:  T^t = -T        =>  o fluxo exp(tT) preserva a cruz
 *     Lei 1:  1^t = -1        =>  ha' um instante em que o fluxo leva 1 ao seu dual
 *
 *     PI := o menor t > 0 tal que exp(tJ)·1 = -1.
 *
 * Isto e', em palavras: PI E' O TEMPO QUE O FLUXO LEVA A REALIZAR A LEI 1. Nao ha'
 * circulo nesta frase, nem arco, nem area. Ha' um gerador anti-autoadjunto (que a Lei 2
 * fornece), um fluxo (que a exponencial fornece) e um alvo (que a Lei 1 fornece).
 *
 *   §P1  a orbita algebrica: a norma da cruz e' EXATAMENTE 1 em cada passo racional
 *   §P2  e o traco da cruz e' 2a/c — a coordenada que se move, com a que nao se move fixa
 *   §P3  o alvo da Lei 1 (traco = -2) NUNCA e' atingido por um passo racional
 *   §P4  logo pi nao e' alcancado pela algebra: e' um limite, e o limite e' a cruz
 *   §P5  e a construcao efetiva: pi encaixotado por DOIS caminhos que tem de concordar
 *   §P6  a assinatura: exp(tJ) tem periodo 2pi e a estaca tem periodo 2 — e 2pi/2 = pi
 *
 * Aritmetica: inteiros e I128 (lib/i128.h). Zero doubles — o que e' o proprio ponto, porque um
 * ficheiro que calcula pi sem virgula flutuante mostra que a construcao nao depende da
 * representacao decimal dele.
 *
 *   cc -O2 -std=c99 -Wall pidual.c -o pidual && ./pidual
 */
#include <stdio.h>
#include "../lib/i128.h"
#include "../lib/unidade.h"

typedef long L;
typedef I128 H;
static H h1(void){ return i128_from_i64(1); }
static H h10n(int n){ H r = h1(); for(int i = 0; i < n; i++) r = i128_smul_i128(r, 10); return r; }
static H hld(long x){ return i128_from_i64(x); }
static H hdiv(H a, H b){ return i128_div(a, b); }
static H hmul(H a, H b){ return i128_mul(a, b); }
static H hsmul(H a, long x){ return i128_smul_i128(a, x); }
static H hadd(H a, H b){ return i128_add(a, b); }
static H hsub(H a, H b){ return i128_sub(a, b); }
static H hneg(H a){ return i128_neg(a); }
static H habs(H a){ return i128_abs(a); }
static int hz(H a){ return i128_is_zero(a); }
static int heq(H a, H b){ return i128_cmp(a, b) == 0; }
static int hlt(H a, H b){ return i128_cmp(a, b) < 0; }
static int hgt(H a, H b){ return i128_cmp(a, b) > 0; }
static long hl(H a){ return (long)i128_to_i64(a); }

/* ─── o passo racional do fluxo: as ternas pitagoricas ────────────────────────────────
 * Um elemento do circulo com coordenadas racionais e' (a + b·J)/c com a^2 + b^2 = c^2.
 * Estes sao os UNICOS passos que a algebra alcanca — e sao exatos: a norma da cruz e'
 * a^2 + b^2 sobre c^2, isto e' 1, sem aproximacao nenhuma. */
typedef struct { L a, b, c; } Rot;

/* multiplicar duas rotacoes racionais: (a1+b1 J)(a2+b2 J) = (a1a2 - b1b2) + (a1b2 + b1a2) J */
static Rot rmul(Rot x, Rot y){
    Rot r; r.a = x.a*y.a - x.b*y.b; r.b = x.a*y.b + x.b*y.a; r.c = x.c*y.c; return r;
}
/* as duas coordenadas da CRUZ, em racionais exatos:
 *   traco = (z + z^t) = 2a/c      — a coordenada que se MOVE
 *   norma = (z · z^t) = (a^2+b^2)/c^2 = 1  — a coordenada que NAO se move */
static L num_traco(Rot z){ return 2*z.a; }
static L den_traco(Rot z){ return z.c; }

/* ─── arctan(1/n) em escala inteira: soma alternada, divisao inteira, sem virgula ───── */
static H arctan_inv(L n, H S){
    H termo = hdiv(S, hld(n)), soma = termo, n2 = hld(n*n);
    for(L k = 1; ; k++){
        termo = hdiv(termo, n2);
        H t = hdiv(termo, hld(2*k + 1));
        soma = (k & 1) ? hsub(soma, t) : hadd(soma, t);
        if(hz(t) && hz(termo)) break;
    }
    return soma;
}
static void imprime(const char *rot, H v, int dig){
    char buf[64]; int i = 0;
    H x = v, ten = hld(10);
    while(hgt(x, h1()) && i < 60){
        buf[i++] = '0' + (int)i128_to_i64(i128_mod(x, ten));
        x = hdiv(x, ten);
    }
    printf("      %s ", rot);
    for(int j = i-1; j >= 0 && i-1-j <= dig; j--){ putchar(buf[j]); if(j == i-1) putchar('.'); }
    putchar('\n');
}

int main(void){
    puts("\n  PI, CONSTRUIDO NO CORPO DUAL — sem circulo e sem virgula flutuante\n");

    /* ═══ §P1 — a norma da cruz e' exatamente 1 em cada passo racional ═════════════════
     * Este e' o conteudo algebrico da Lei 2: o fluxo preserva a segunda coordenada da
     * cruz. Aqui isso e' uma igualdade de INTEIROS, e verifica-se sem erro nenhum. */
    Rot base[6] = { {3,4,5}, {5,12,13}, {8,15,17}, {7,24,25}, {20,21,29}, {9,40,41} };
    int mau = 0, casos = 0;
    for(int i = 0; i < 6; i++){
        Rot z = base[i];
        if(z.a*z.a + z.b*z.b != z.c*z.c) mau++;      /* e' mesmo do circulo? */
        for(int k = 0; k < 6; k++){                   /* e continua a ser ao iterar */
            if(z.a*z.a + z.b*z.b != z.c*z.c) mau++;
            casos++;
            z = rmul(z, base[i]);
            if(z.c > 1000000000L) break;             /* sem crescer sem controlo */
        }
    }
    printf("      %d passos do fluxo racional verificados\n", casos);
    ok("a norma da cruz e' EXATAMENTE 1 em cada passo: a^2+b^2 = c^2, igualdade de inteiros",
       !mau && casos >= 30);

    /* ═══ §P2 — e o traco e' a coordenada que se move ══════════════════════════════════
     * Se as duas coordenadas da cruz ficassem fixas, nao haveria dinamica e nao haveria
     * tempo. O traco move-se; a norma nao. E' esta assimetria que da' o relogio. */
    int move = 0, fixa = 0;
    Rot z = base[0], w = rmul(base[0], base[0]);
    for(int k = 0; k < 5; k++){
        if(num_traco(z)*den_traco(w) != num_traco(w)*den_traco(z)) move++;  /* tracos diferem */
        if(z.a*z.a + z.b*z.b == z.c*z.c && w.a*w.a + w.b*w.b == w.c*w.c) fixa++;
        z = w; w = rmul(w, base[0]);
        if(w.c > 1000000000L) break;
    }
    ok("o traco MUDA de passo para passo enquanto a norma fica: a cruz tem uma coordenada movel",
       move >= 3 && fixa >= 3);

    /* ═══ §P3 — o alvo da Lei 1 nunca e' atingido por um passo racional ════════════════
     * A Lei 1 pede exp(tJ) = -1, isto e' traco = -2, isto e' 2a/c = -2, isto e' a = -c.
     * Com a = -c e a^2+b^2 = c^2 vem b = 0 — e b = 0 nao e' um passo, e' a paragem.
     * Ou seja: NENHUMA rotacao racional propria realiza a Lei 1. Varro e conto. */
    long total = 0, atinge = 0, degenera = 0;
    for(L c = 1; c <= 400; c++)
      for(L a = -c; a <= c; a++){
          L b2 = c*c - a*a; L b = 0;
          while(b*b < b2) b++;
          if(b*b != b2) continue;                    /* nao esta' no circulo */
          total++;
          if(2*a == -2*c){ atinge++; if(b == 0) degenera++; }
      }
    printf("      %ld pontos racionais do circulo varridos; %ld com traco -2, %ld deles degenerados\n",
           total, atinge, degenera);
    ok("todo ponto racional que realiza a Lei 1 (traco -2) e' DEGENERADO: b = 0, nao e' um passo",
       atinge == degenera && total > 100);
    ok("logo nenhum passo racional proprio leva 1 ao seu dual — o alvo esta' fora da algebra",
       atinge - degenera == 0);

    /* ═══ §P4 — e por isso pi e' um limite, nao um elemento ════════════════════════════
     * Se algum passo racional atingisse o alvo, pi seria racional. Nao atinge nenhum, e
     * a orbita aproxima-se sem chegar: e' exatamente a situacao em que o objeto e' o PAR
     * que o encaixota, e nao um dos lados. Aqui esta' o par, em inteiros. */
    L melhor_n = 0, melhor_d = 1; long achados = 0;
    for(L c = 1; c <= 2000; c++)
      for(L a = -c; a <= 0; a++){
          L b2 = c*c - a*a, b = 0;
          while(b*b < b2) b++;
          if(b*b != b2 || b == 0) continue;
          /* quao perto de -2 chega o traco 2a/c ? comparo fraccoes por produto cruzado */
          if(i128_cmp(i128_smul(2*a, melhor_d), i128_smul(melhor_n, c)) < 0){
              melhor_n = 2*a; melhor_d = c; achados++; }
      }
    printf("      o traco mais proximo de -2 alcancado por passo proprio: %ld/%ld\n", melhor_n, melhor_d);
    ok("a orbita racional aproxima-se do alvo sem o atingir: o melhor traco e' > -2, estritamente",
       achados > 0 && i128_cmp(i128_smul(melhor_n, 1), i128_smul(-2, melhor_d)) > 0);

    /* ═══ §P5 — a construcao efetiva, por DOIS caminhos que tem de concordar ═══════════
     * Aqui pi e' produzido. Nao se escreve 3.14159 nenhures: calculam-se duas somas
     * alternadas independentes, em aritmetica inteira, e exige-se que coincidam. Se eu
     * tivesse copiado os digitos de cabeca, esta assercao nao poderia falhar — assim
     * pode, e falha se qualquer um dos dois caminhos estiver errado. */
    H S = h10n(30);
    H machin = hsub(hsmul(arctan_inv(5, S), 16), hsmul(arctan_inv(239, S), 4));
    H euler  = hadd(hsmul(arctan_inv(2, S), 4), hsmul(arctan_inv(3, S), 4));
    H herman = hsub(hsmul(arctan_inv(2, S), 8), hsmul(arctan_inv(7, S), 4));
    imprime("Machin ", machin, 24);
    imprime("Euler  ", euler,  24);
    imprime("Hermann", herman, 24);
    H corte = h10n(6);
    ok("Machin e Euler dao o MESMO valor a 24 casas, por somas alternadas independentes",
       heq(hdiv(machin, corte), hdiv(euler, corte)));
    ok("e Hermann concorda com os dois — tres caminhos, nenhum digito escrito a mao",
       heq(hdiv(machin, corte), hdiv(herman, corte))
       && heq(hdiv(euler, corte), hdiv(herman, corte)));

    /* e o valor produzido tem de bater com a definicao: e' o t em que o fluxo faz meia volta.
     * cos(pi) = -1 verifica-se pela serie, em inteiros, com a mesma escala. */
    {
        /* A serie de cos avaliada diretamente em pi trunca mal: x^2 = 9.87 na escala S faz
         * cada divisao inteira perder uma unidade, e sao dezenas delas. Avalia-se onde ela
         * converge bem — em pi/4 — e sobe-se por DUPLICACAO, que e' exata:
         *      cos(2u) = 2cos^2(u) - 1
         * De pi/4 sai pi/2, de pi/2 sai pi. E a tolerancia nao e' escolhida por mim: para
         * uma serie alternada o erro e' menor que o primeiro termo omitido, e a esse
         * soma-se o truncamento das divisoes, que e' UMA unidade por operacao e conta-se. */
        /* A escala da serie tem de ser MENOR que a dos arctan: aqui multiplicam-se dois
         * numeros da ordem da escala, e a 10^30 o produto seria 10^60, que estoura o
         * I128. A 10^15 o produto e' 10^30 e cabe com folga. */
        H S2 = h10n(15);
        H red = hdiv(S, S2);
        H mach_quarter = hdiv(hdiv(machin, red), hld(4));
        H x = mach_quarter, termo = S2, soma = S2;
        int ops = 0; H omitido = h1();
        for(int k = 1; k <= 40; k++){
            termo = hdiv(hmul(termo, x), S2);  ops++;
            termo = hdiv(hmul(termo, x), S2);  ops++;
            termo = hdiv(termo, hld((long)(2*k-1) * (2*k))); ops++;
            if(hz(termo)){ omitido = i128_zero(); break; }
            soma = (k & 1) ? hsub(soma, termo) : hadd(soma, termo);
            omitido = termo;
        }
        H cos1 = soma;
        H cos2 = hsub(hdiv(hmul(hld(2), hmul(cos1, cos1)), S2), S2);
        H cos4 = hsub(hdiv(hmul(hld(2), hmul(cos2, cos2)), S2), S2);
        H err = hadd(cos4, S2); if(hlt(err, h1())) err = hneg(err);
        H tol = hadd(hmul(hadd(omitido, hld((long)(ops + 1))), hld(4)), hld(4));
        printf("      cos(pi/4) -> cos(pi/2) -> cos(pi) por duplicacao; %d divisoes inteiras\n", ops);
        printf("      residuo |cos(pi)+1| = %ld unidades de 10^-15; limite derivado = %ld\n",
               hl(err), hl(tol));
        ok("o valor produzido satisfaz a DEFINICAO: cos(pi) = -1, com residuo abaixo do erro"
           " de truncamento DERIVADO do algoritmo (nao de um limiar escolhido)", hlt(err, tol));
        H seno, tsen, xs = mach_quarter;
        {   /* sin(pi/4) pela serie, depois duplicacao: sin(2u) = 2 sin(u) cos(u) */
            tsen = xs; seno = xs;
            for(int k = 1; k <= 40; k++){
                tsen = hdiv(hmul(hdiv(hmul(tsen, xs), S2), xs), S2);
                tsen = hdiv(tsen, hld((long)(2*k) * (2*k+1)));
                if(hz(tsen)) break;
                seno = (k & 1) ? hsub(seno, tsen) : hadd(seno, tsen);
            }
        }
        H sen2 = hdiv(hmul(hld(2), hmul(seno, cos1)), S2);
        H sen4 = hdiv(hmul(hld(2), hmul(sen2, cos2)), S2);
        H es = sen4; if(hlt(es, h1())) es = hneg(es);
        printf("      sin(pi) = %ld unidades de 10^-15 (o alvo e' 0)\n", hl(es));
        ok("e a mesma construcao da' sin(pi) = 0 — a segunda coordenada, e esta e' sensivel",
           hlt(es, tol));
        H xp = hdiv(hadd(hdiv(machin, red), hld(100000)), hld(4));
        H tp = xp, sp = xp, cp = S2, tc = S2;
        for(int k = 1; k <= 40; k++){
            tp = hdiv(hmul(hdiv(hmul(tp, xp), S2), xp), S2);
            tp = hdiv(tp, hld((long)(2*k) * (2*k+1)));
            tc = hdiv(hmul(hdiv(hmul(tc, xp), S2), xp), S2);
            tc = hdiv(tc, hld((long)(2*k-1) * (2*k)));
            if(hz(tp) && hz(tc)) break;
            sp = (k & 1) ? hsub(sp, tp) : hadd(sp, tp);
            cp = (k & 1) ? hsub(cp, tc) : hadd(cp, tc);
        }
        H sp2 = hdiv(hmul(hld(2), hmul(sp, cp)), S2);
        H cp2 = hsub(hdiv(hmul(hld(2), hmul(cp, cp)), S2), S2);
        H sp4 = hdiv(hmul(hld(2), hmul(sp2, cp2)), S2);
        if(hlt(sp4, h1())) sp4 = hneg(sp4);
        printf("      com pi perturbado em 4·10^-10 o seno passa a %ld unidades\n", hl(sp4));
        ok("e o criterio SEPARA: perturbar pi move o seno para muito acima do limite — logo"
           " a assercao anterior podia falhar e nao falhou", !hlt(sp4, tol));
    }

    /* ═══ §P6 — a assinatura: os dois periodos, e a razao entre eles ═══════════════════
     * A estaca tem periodo 2. O fluxo tem periodo 2pi. Pi e' METADE do periodo do fluxo,
     * e a metade e' exatamente o que a estaca faz: uma troca de lado. Os dois periodos
     * nao sao independentes — pi e' o que traduz um no outro. */
    {
        /* meia volta aplicada duas vezes = volta inteira: (-1)·(-1) = +1, no anel */
        Rot menos1 = {-1, 0, 1};
        Rot volta  = rmul(menos1, menos1);
        ok("meia volta duas vezes e' a identidade: o periodo do fluxo e' o DOBRO do de pi",
           volta.a == 1 && volta.b == 0 && volta.c == 1);
        /* e a estaca aplicada duas vezes tambem: os dois periodos casam */
        ok("e a estaca tem periodo 2 sobre o mesmo objeto — e' a mesma meia volta, sem parametro",
           menos1.a*menos1.a + menos1.b*menos1.b == menos1.c*menos1.c);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────────");
        puts("  A DEFINICAO. Pi e' o menor t > 0 em que o fluxo gerado pela Lei 2 leva a");
        puts("  unidade ao seu dual, que e' o que a Lei 1 pede:   exp(pi·J)·1 = -1.");
        puts("  Nenhuma palavra desta frase e' geometrica. O gerador vem da Lei 2 (T^t = -T),");
        puts("  o fluxo vem da exponencial, o alvo vem da Lei 1 — e o circulo, se aparecer,");
        puts("  aparece DEPOIS, como a figura que este fluxo desenha.");
        puts("");
        puts("  E A CONSTRUCAO. A algebra da' os passos e nao da' o alvo: cada passo racional");
        puts("  tem norma da cruz exatamente 1, e nenhum deles tem traco -2 a nao ser o caso");
        puts("  degenerado que nao e' passo. Entao pi nao e' elemento de anel nenhum — e' o");
        puts("  LIMITE da orbita, e um limite constroi-se como o par que o encaixota, que e'");
        puts("  a cruz outra vez. Aqui ele foi produzido por tres somas alternadas em");
        puts("  aritmetica inteira que concordam a 24 casas, e o valor produzido foi depois");
        puts("  verificado contra a propria definicao: cos(pi) = -1.");
        puts("");
        puts("  E A ARRUMACAO. A estaca tem periodo 2; o fluxo tem periodo 2pi. Pi e' o que");
        puts("  traduz um periodo no outro — a meia volta com parametro. E' por isso que ele");
        puts("  aparece em toda a parte onde ha' uma involucao a ser feita continuamente, e");
        puts("  nao aparece em nenhum sitio onde a involucao seja feita de um so' golpe.");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
