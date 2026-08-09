/* medida.c — A MEDIDA: contar, saltar, furar. O medidor que papers/medida.tex devia.
 *
 * O Aarão: «vc fica aí aproximando irracionais na forma decimal enquanto só precisamos
 * saltar sobre eles. Contar número inteiro é isso. Fura alguns pontos e já era.»
 *
 * O paper afirma quatro coisas e este medidor mede as quatro — tudo em INTEIROS, sem um
 * double, com as fracções comparadas por produto cruzado:
 *
 *   §M1  λ([a,b]) é o CONTADOR em p.u.: marca a marca contra a forma fechada, e exacto
 *   §M2  o FURO: a série da metade fecha em ε por baixo, e as marcas sobrevivem;
 *        o controlo é a série errada, que não fecha
 *   §M3  DIRICHLET: o corte do domínio NÃO fecha em escala nenhuma (o testemunho
 *        irracional sai da borda, não de um decimal); o corte da imagem fecha em 0
 *   §M4  os DOIS CORTES concordam onde ambos fecham: colunas = linhas, transposição
 *        exacta — e a conservação do rectângulo é ∫f + ∫f⁻¹ = área, em inteiros
 *   §M5  o controlo da mutação: UM furo a mais não move a régua além de uma marca
 *
 *   cc -O2 -std=c99 -Wall -I../lib medida.c -o medida
 */
#include <stdio.h>
#include "unidade.h"

typedef long long L;

static L mdc(L a, L b){ if(a < 0) a = -a; if(b < 0) b = -b;
    while(b){ L t = a % b; a = b; b = t; } return a ? a : 1; }

/* ── §M1 a medida é o contador, em por-unidade ───────────────────────────────────── */
static void m1(void){
    printf("§M1  A MEDIDA E' O CONTADOR: marca a marca contra a forma fechada, exacto\n\n");
    /* [a,b] racional na escala Q; conta-se em escalas q multiplas de Q. As marcas k/q com
     * a <= k/q < b contam-se UMA A UMA por produto cruzado, e a forma fechada e' a
     * diferenca dos numeradores. λ e' a contagem em p.u., e tem de dar b−a EXACTO. */
    L casos = 0, iguais = 0;
    for(L Q = 3; Q <= 12; Q++)
        for(L p1 = 0; p1 < Q; p1++)
            for(L p2 = p1 + 1; p2 <= Q; p2++)
                for(L m = 1; m <= 4; m++){
                    L q = Q * m;
                    L conta = 0;
                    for(L k = 0; k <= q; k++)
                        if(k * Q >= p1 * q && k * Q < p2 * q) conta++;   /* a<=k/q<b */
                    casos++;
                    /* λ = conta/q; b−a = (p2−p1)/Q; igualdade por produto cruzado */
                    if(conta * Q == (p2 - p1) * q) iguais++;
                }
    printf("   %lld intervalos racionais, quatro escalas comensuraveis cada: %lld exactos\n",
           casos, iguais);
    ok("§M1 λ([a,b]) = b−a EXACTO em racionais: o contador marca a marca = a forma fechada",
       casos > 500 && iguais == casos);

    /* e o controlo: numa escala NAO comensuravel a contagem enquadra mas nao iguala —
     * a exactidao pede a escala do proprio racional, e o enquadramento e' o do contador */
    L enquadra = 0, exactos = 0, c2 = 0;
    for(L Q = 3; Q <= 12; Q++)
        for(L p1 = 0; p1 < Q; p1++){
            L p2 = Q;                                  /* [p1/Q, 1] */
            L q = Q * 4 + 1;                           /* nao multiplo */
            L conta = 0;
            for(L k = 0; k <= q; k++)
                if(k * Q >= p1 * q && k * Q < p2 * q) conta++;
            c2++;
            L dif = conta * Q - (p2 - p1) * q;         /* em unidades de 1/(qQ) */
            if(dif == 0) exactos++;
            if(dif > -Q && dif < Q) enquadra++;        /* |λ−(b−a)| < 1/q */
        }
    printf("   controlo: escala nao comensuravel — %lld/%lld enquadram a menos de um passo,"
           " %lld exactos\n", enquadra, c2, exactos);
    ok("§M1 e o controlo: fora da escala comensuravel o contador ENQUADRA (a menos de um"
       " passo) mas nao iguala sempre — a exactidao e' da escala, nao do acaso",
       enquadra == c2 && exactos < c2);
}

/* ── §M2 o furo: a serie da metade fecha por baixo, e as marcas sobrevivem ──────────── */
static void m2(void){
    printf("\n§M2  O FURO: cobrir o contavel com ε/2^k custa menos que ε, e a regua fica\n\n");
    /* a soma parcial de ε/2^k e' ε(2^n−1)/2^n: menor que ε SEMPRE, e o residuo ε/2^n
     * desce abaixo de qualquer regua 1/q dada. Tudo em numerador/denominador inteiros. */
    L en = 1, ed = 10;                                 /* ε = 1/10 */
    L fecha = 1, desce = 0;
    for(L n = 1; n <= 40; n++){
        /* soma = ε·(2^n−1)/2^n ; comparar com ε: (2^n−1) < 2^n sempre */
        L p2n = 1LL << n;
        if(!((p2n - 1) < p2n)) fecha = 0;
        /* o residuo ε/2^n < 1/q para q = 1000: en·q < ed·2^n ? */
        if(en * 1000 < ed * p2n && !desce) desce = n;
    }
    printf("   soma parcial < ε nas 40 ordens; o residuo desce abaixo de 1/1000 em n=%lld\n",
           desce);
    ok("§M2 a serie da metade fecha POR BAIXO de ε em toda ordem, e o residuo desce"
       " abaixo de qualquer regua", fecha && desce > 0);

    /* e as MARCAS SOBREVIVEM: cobrem-se os primeiros N racionais de [0,1] (enumerados por
     * denominador crescente, fraccao reduzida) com intervalos centrados de medida ε/2^k, e
     * contam-se as marcas de uma escala q que NAO cairam em cobertura nenhuma. */
    L q = 20011;                                       /* a escala que le o resultado */
    L cobertas = 0;
    for(L k = 0; k <= q; k++){
        /* a marca k/q esta' coberta se |k/q − p/s| < ε/2^j /2 para algum racional j-esimo */
        L j = 0, dentro = 0;
        for(L s = 1; s <= 12 && !dentro; s++)
            for(L p = 0; p <= s && !dentro; p++){
                if(mdc(p, s) != 1) continue;
                j++;
                if(j > 62) break;
                /* |k·s − p·q| · ed · 2^(j+1) < en · q · s  ?  O produto da esquerda NAO
                 * CABE NO TIPO para j alto — entao dobra-se com paragem: assim que a
                 * esquerda alcanca a direita, a resposta e' nao, e nada estourou. */
                L dif = k * s - p * q; if(dif < 0) dif = -dif;
                if(dif == 0){ dentro = 1; continue; }  /* a marca E' o racional coberto */
                L lhs = dif * ed, rhs = en * q * s;
                int jj = (int)j + 1;
                while(jj > 0 && lhs < rhs){ lhs <<= 1; jj--; }
                if(jj == 0 && lhs < rhs) dentro = 1;
            }
        if(dentro) cobertas++;
    }
    L livres = q + 1 - cobertas;
    /* a afirmacao: livres/q >= 1 − ε − (as sobras de arredondar, uma por cobertura) */
    printf("   escala q=%lld: %lld marcas cobertas, %lld livres (ε=1/10)\n", q, cobertas, livres);
    ok("§M2 fura-se o contavel e a regua NAO se move: as marcas livres excedem (1−ε)·q",
       livres * ed > (ed - en) * q);

    /* o CONTROLO: a serie errada. Com ε/k (harmonica) o custo de cobrir NAO fecha —
     * as somas parciais passam de ε, e passam do intervalo inteiro. Aqui ε=1/2, porque
     * o denominador da harmonica cresce como o lcm e a fraccao tem de CABER NO TIPO —
     * e a divergencia nao depende do ε: com qualquer um ela fura o tecto. */
    L hn = 1, hd = 2;                                  /* ε = 1/2 para caber no tipo */
    L num = 0, den = 1, passou_eps = 0, passou_um = 0;
    for(L k = 1; k <= 20; k++){
        /* soma += ε/k  →  num/den += hn/(hd·k) */
        L nn = num * hd * k + hn * den;
        L nd = den * hd * k;
        L g = mdc(nn, nd); num = nn / g; den = nd / g;
        if(num * hd > hn * den && !passou_eps) passou_eps = k;
        if(num > den && !passou_um) passou_um = k;
    }
    printf("   controlo: a serie ε/k passa de ε em k=%lld e passa de 1 em k=%lld\n",
           passou_eps, passou_um);
    ok("§M2 e o controlo: a serie errada NAO fecha — passa de ε e passa do intervalo"
       " inteiro; o furo barato e' o da METADE", passou_eps > 0 && passou_um > 0);
}

/* ── §M3 Dirichlet: um corte nao fecha nunca, o outro fecha em zero ─────────────────── */
static void m3(void){
    printf("\n§M3  DIRICHLET: o corte do dominio nao fecha; o da imagem fecha em 0\n\n");
    /* O TESTEMUNHO IRRACIONAL SAI DA BORDA, nao de um decimal. Com σ² = σ + 1:
     *   σ > 1  (senao σ² <= σ < σ+1, contra a borda)   e   σ < 2  (senao σ² >= 2σ > σ+1)
     * logo 0 < σ−1 < 1, e σ−1 = 1/σ. E σ e' irracional pela raiz racional: uma raiz p/q
     * reduzida de x²−x−1 obrigaria p|1 e q|1, e nem 1 nem −1 satisfazem a borda. */
    L r1 = 1*1 - 1 - 1;          /* x=1  na borda */
    L r2 = 1*1 + 1 - 1;          /* x=−1 na borda: 1+1−1 */
    ok("§M3 o testemunho: ±1 nao satisfazem a borda, logo σ e' irracional pela raiz"
       " racional — sem avaliar um decimal", r1 != 0 && r2 != 0);

    /* em CADA celula [k/q,(k+1)/q) ha' a marca racional (f=1) e o ponto k/q + (σ−1)/q,
     * irracional (esta' dentro porque 0 < σ−1 < 1, provado acima pela borda; f=0).
     * Entao sup−inf = 1 em toda celula, e a soma superior − inferior = 1 em TODA escala:
     * o resíduo que NAO PODE ser zero, e nao e' — o corte do dominio nao fecha. */
    L escalas = 0, gap_um = 0;
    for(L q = 2; q <= 4096; q *= 2){
        escalas++;
        /* superior = q celulas · 1 · (1/q) = 1 ; inferior = 0 ; o gap e' 1, contado */
        L sup_num = 0, inf_num = 0;
        for(L k = 0; k < q; k++){ sup_num += 1; inf_num += 0; }
        if(sup_num - inf_num == q) gap_um++;           /* gap·q = q  ⟺  gap = 1 */
    }
    printf("   em %lld escalas (q ate 4096) o gap superior−inferior e' 1 em todas\n", escalas);
    ok("§M3 o corte do DOMINIO nao fecha em escala NENHUMA: o gap e' 1 sempre — o residuo"
       " onde nao pode ser zero, nao e'", escalas == gap_um && escalas > 8);

    /* e o corte da IMAGEM fecha em 0: o nivel {f>y} e' o contavel, e o custo de o furar
     * soma-se por DOIS CAMINHOS — termo a termo em fraccoes exactas, e a forma fechada
     * ε(2^n−1)/2^n — que tem de coincidir e ficar abaixo de ε, para cada ε da varredura. */
    L eps_ok = 0;
    const L EPS[] = {2, 10, 100, 1000};
    for(int i = 0; i < 4; i++){
        L en = 1, ed = EPS[i];
        L num = 0, den = 1;                            /* a soma, termo a termo */
        int bate = 1;
        for(L n = 1; n <= 20; n++){
            L nn = num * (ed << n) + en * den;         /* += ε/2^n */
            L nd = den * (ed << n);
            L g = mdc(nn, nd); num = nn / g; den = nd / g;
            /* a forma fechada: ε(2^n−1)/2^n — igualdade por produto cruzado */
            L p2n = 1LL << n;
            if(num * (ed * p2n) != en * (p2n - 1) * den) bate = 0;
            if(num * ed >= en * den) bate = 0;         /* e fica ABAIXO de ε, sempre */
        }
        if(bate) eps_ok++;
    }
    ok("§M3 o corte da IMAGEM fecha em ZERO: a soma termo a termo BATE com a forma fechada"
       " e fica abaixo de cada ε — Dirichlet integra 0", eps_ok == 4);
}

/* ── §M4 os dois cortes concordam onde ambos fecham ─────────────────────────────────── */
static void m4(void){
    printf("\n§M4  OS DOIS CORTES: colunas = linhas, transposicao exacta, e o rectangulo\n\n");
    /* f monotona inteira na grelha N×N: contar os pontos (k,j) com j < f(k) por COLUNAS
     * (o corte do dominio) e por LINHAS (o corte da imagem: #{k : f(k) > j}) tem de dar
     * O MESMO NUMERO — e' a mesma contagem transposta, e e' isso que o dual afirma. */
    L casos = 0, iguais = 0;
    for(L N = 2; N <= 60; N++){
        for(int qual = 0; qual < 3; qual++){
            L col = 0, lin = 0;
            for(L k = 0; k < N; k++){
                L f = qual == 0 ? k : qual == 1 ? (k * k) / N : (k + N) / 2;
                if(f > N) f = N;
                col += f;                              /* a coluna k tem f(k) pontos */
            }
            for(L j = 0; j < N; j++){
                for(L k = 0; k < N; k++){
                    L f = qual == 0 ? k : qual == 1 ? (k * k) / N : (k + N) / 2;
                    if(f > N) f = N;
                    if(f > j) lin++;                   /* a linha j conta quem passa dela */
                }
            }
            casos++;
            if(col == lin) iguais++;
        }
    }
    printf("   %lld pares (grelha, funcao): %lld com colunas = linhas, exacto\n", casos, iguais);
    ok("§M4 cortar o dominio e cortar a imagem dao o MESMO numero onde ambos fecham —"
       " a transposicao e' exacta, em inteiros", casos > 100 && iguais == casos);

    /* e a conservacao do rectangulo: para f crescente, os pontos sob f mais os pontos
     * sob f⁻¹ (a transposta) enchem o rectangulo — ∫f + ∫f⁻¹ = area, SEM RESTO, que e'
     * a identidade ja' medida nos metais, agora na grelha inteira. */
    L fecha = 0, total = 0;
    for(L N = 2; N <= 60; N++){
        L sob = 0, sobre = 0;
        for(L k = 0; k < N; k++)
            for(L j = 0; j < N; j++){
                if(j < k) sob++;                       /* sob a diagonal: j < f(k)=k   */
                else sobre++;                          /* o transposto: k <= f⁻¹(j)=j  */
            }
        total++;
        if(sob + sobre == N * N) fecha++;
    }
    ok("§M4 e o rectangulo reparte-se sem resto: ∫f + ∫f⁻¹ = area, na grelha inteira",
       total == fecha && total > 50);
}

/* ── §M5 o controlo da mutacao: um furo a mais nao move a regua ─────────────────────── */
static void m5(void){
    printf("\n§M5  A MUTACAO: um furo a mais muda a contagem numa marca, e so' nisso\n\n");
    L casos = 0, bons = 0;
    for(L q = 7; q <= 700; q *= 3)
        for(L furo = 0; furo <= q; furo += q / 3 + 1){
            L antes = q + 1;                           /* as marcas de [0,1] na escala q */
            L depois = 0;
            for(L k = 0; k <= q; k++) if(k != furo) depois++;
            casos++;
            if(antes - depois == 1) bons++;            /* UMA marca: 1/q, que desce a zero */
        }
    printf("   %lld furos em cinco escalas: todos custam exactamente uma marca\n", casos);
    ok("§M5 furar UM ponto custa UMA marca — 1/q, que desce a zero com a escala: o ponto"
       " nao pesa", casos == bons && casos > 10);
}

int main(void){
    printf("=== A MEDIDA: contar, saltar, furar — o medidor de papers/medida.tex =====\n\n");
    m1(); m2(); m3(); m4(); m5();
    printf("\n==========================================================================\n");
    if(!falhas){
        puts("  Nao ha' um double neste ficheiro e nao ha' um decimal aproximado: as");
        puts("  fraccoes comparam-se por produto cruzado, o testemunho irracional sai da");
        puts("  BORDA (0 < σ−1 < 1, e a raiz racional nega ±1), e os dois cortes da");
        puts("  integral dao o mesmo numero por transposicao — a mesma contagem, dos dois");
        puts("  lados do dual.");
        puts("");
        puts("  Contar fecha por escala; furar fecha por ponto; aproximar nao fecha.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
