/* ═══════════════════════════════════════════════════════════════════════════
 * lib/serie.h — A EXPONENCIAL E O π, pela definição da aranha: em FATORIAL.
 *
 * O paper (§sec:serie) não importa constante nenhuma. Da única hipótese J²=−1
 * as potências de J ciclam com período quatro, a série da exponencial PARTE-SE
 * pela paridade do índice, e recolhendo cada metade:
 *
 *     exp(tJ) = c(t)·1 + s(t)·J,
 *     c(t) = Σ_k (−1)^k t^{2k}/(2k)!,      s(t) = Σ_k (−1)^k t^{2k+1}/(2k+1)!
 *
 * e daí, sem uma palavra de geometria:
 *
 *     π = min{ t > 0 : exp(tJ)·1 = −1 }   --- o tempo que o fluxo leva a
 *                                             realizar a Lei 1.
 *
 * AQUI NÃO HÁ VÍRGULA. Trabalha-se em escala inteira: um número x é guardado
 * como o inteiro round(x·S), e o `thm:serie` diz porque isto TERMINA --- «os
 * termos anulam-se ao fim de um número finito de parcelas, que é o que a
 * divisão inteira faz». O fatorial cresce mais depressa que qualquer potência,
 * e a partir de certo k o termo é ZERO na escala. Não é truncar: é parar.
 *
 * E o termo calcula-se por RECORRÊNCIA, nunca formando t^n nem n! --- os dois
 * estouram e o quociente deles não:
 *
 *     T_0 = S,   T_{k+1} = −T_k · t² / ((2k+1)(2k+2))     (para c)
 *
 * com t já em escala, pelo que cada passo divide também por S. O sinal
 * alternado é o (−1)^k, que é «o número de voltas lido módulo dois».
 *
 * Medido em `tests/serie.c` e `tests/pgwire.c` §W183.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef SERIE_H
#define SERIE_H

/* o resultado de uma série: o valor em escala, e o CUSTO --- quantos termos
 * até o termo ir a zero, que é o ℓ do thm:serie lido no campo */
typedef struct { long valor; int termos; int parou; } SfSerie;

/* ── c(t) = Σ (−1)^k t^{2k}/(2k)! ──────────────────────────────────────────
 * `t` e o resultado em escala `S`. Devolve também em quantos termos parou. */
static SfSerie sf_c(long t, long S){
    SfSerie r; r.valor = S; r.termos = 1; r.parou = 0;
    long T = S;                                   /* T_0 = 1, em escala */
    for(int k = 0; k < 64; k++){
        /* T_{k+1} = −T_k · t · t / ((2k+1)(2k+2) · S · S) */
        long d1 = (long)(2*k+1), d2 = (long)(2*k+2);
        long num = T / d1;                        /* divide cedo: não estoura */
        num = num * t / S;
        num = num / d2;
        num = num * t / S;
        T = -num;
        if(T == 0){ r.parou = 1; break; }
        r.valor += T;
        r.termos++;
    }
    return r;
}

/* ── s(t) = Σ (−1)^k t^{2k+1}/(2k+1)! ─────────────────────────────────────── */
static SfSerie sf_s(long t, long S){
    SfSerie r; r.valor = t; r.termos = 1; r.parou = 0;
    long T = t;                                   /* T_0 = t */
    for(int k = 0; k < 64; k++){
        long d1 = (long)(2*k+2), d2 = (long)(2*k+3);
        long num = T / d1;
        num = num * t / S;
        num = num / d2;
        num = num * t / S;
        T = -num;
        if(T == 0){ r.parou = 1; break; }
        r.valor += T;
        r.termos++;
    }
    return r;
}

/* ── A CONSERVAÇÃO, que sai por derivação e não se impõe:
 *      (c²+s²)' = 2cc' + 2ss' = −2cs + 2sc = 0,  e vale 1 em t=0.
 * Aqui verifica-se em escala: c²+s² tem de dar S², a menos do grão. */
static long sf_nivel(long t, long S){
    SfSerie c = sf_c(t, S), s = sf_s(t, S);
    return (c.valor/1000)*(c.valor/1000) + (s.valor/1000)*(s.valor/1000);
}

/* ── π = min{t>0 : exp(tJ)·1 = −1}.
 *
 * A condição é sobre o PAR: exp(πJ) = −1 quer dizer c(π) = −1 E s(π) = 0. E as
 * duas metades não são igualmente úteis para PROCURAR --- é preciso dizê-lo,
 * porque a primeira escrita disto usou a errada e deu 3,097 em vez de 3,1415:
 *
 *   · procurar c(t) = −1 é procurar um MÍNIMO, e perto do mínimo a função é
 *     PLANA: c(t) ≈ −1 + (t−π)²/2, pelo que um desvio de 10^-3 no valor dá
 *     4·10^-2 no t. A condição é mal posta e o erro é da CONDIÇÃO, não da série.
 *
 *   · procurar s(t) = 0 é procurar um zero SIMPLES: s'(π) = c(π) = −1 ≠ 0, e o
 *     desvio no t é da ordem do desvio no valor. Bem posta.
 *
 * São a mesma equação lida nas duas faces, e só uma delas se procura. Usa-se a
 * segunda, e CONFERE-SE a primeira no ponto achado --- que é o par a fechar. */
typedef struct { long pi; long c_no_ponto; int termos; int achou; } SfPi;

static SfPi sf_pi(long S, long grao){
    SfPi r; r.pi = 0; r.c_no_ponto = 0; r.termos = 0; r.achou = 0;
    long ant = 0; int primeiro = 1;
    for(long t = grao; t < 8*S; t += grao){
        SfSerie sv = sf_s(t, S);
        r.termos += sv.termos;
        if(!primeiro && ant > 0 && sv.valor <= 0){
            /* o zero está entre t−grão e t: fica-se com o mais próximo */
            long tz = (ant > -sv.valor) ? t : t - grao;
            r.pi = tz;
            r.c_no_ponto = sf_c(tz, S).valor;      /* tem de dar −S: a outra face */
            r.achou = 1;
            break;
        }
        ant = sv.valor; primeiro = 0;
    }
    return r;
}

/* ── E A SOLUÇÃO DA EDO É A EXPONENCIAL, que é esta série.
 * Para y' = λy com λ real, y(t) = e^{λt} = Σ (λt)^k/k! --- a mesma recorrência
 * sem o sinal alternado. É o que o `edo` resolve, avaliado. */
static SfSerie sf_exp(long x, long S){
    SfSerie r; r.valor = S; r.termos = 1; r.parou = 0;
    long T = S;
    for(int k = 1; k <= 64; k++){
        T = (T / k) * x / S;
        if(T == 0){ r.parou = 1; break; }
        r.valor += T;
        r.termos++;
    }
    return r;
}

/* ═══ A EDO RESOLVIDA PELA SÉRIE --- e a raiz NUNCA é precisa ══════════════
 *
 * Este é o ponto: para avaliar a solução de
 *
 *     y^{(n)} + c_{n-1} y^{(n-1)} + ... + c_0 y = 0
 *
 * não é preciso extrair raiz nenhuma. As derivadas em zero saem da PRÓPRIA
 * recorrência --- é ela que as dá ---
 *
 *     y^{(k+n)}(0) = −c_{n-1} y^{(k+n-1)}(0) − ... − c_0 y^{(k)}(0),
 *
 * e a série de Taylor é FATORIAL por construção:
 *
 *     y(t) = Σ_k y^{(k)}(0) · t^k / k!
 *
 * O termo calcula-se por recorrência, T_{k+1} = T_k · t/(k+1) com o T a
 * carregar já a derivada --- nunca se forma t^k nem k!. E os termos ANULAM-SE
 * (thm:serie), pelo que o processo TERMINA e o número de termos é o custo.
 *
 * Isto resolve o irracional sem o nomear: quando as raízes são (−B±√Δ)/2 com Δ
 * não quadrado, a solução avaliada sai na mesma, exacta na escala --- porque a
 * série não precisa de saber quais são as raízes, só dos coeficientes.
 * E quando elas são inteiras, a série tem de bater com a forma fechada: é o
 * controlo. */

#define SF_ORD 8

/* avalia y(t) para a EDO de ordem n com coeficientes `co` (do termo constante
 * para cima, mónico implícito) e condições iniciais `d0` = (y(0), y'(0), ...),
 * tudo em escala S. Devolve o valor e os termos gastos. */
static SfSerie sf_edo(const long *co, int n, const long *d0, long t, long S){
    SfSerie r; r.valor = 0; r.termos = 0; r.parou = 0;
    /* O TERMO CARREGA JÁ O FATORIAL, e é isso que evita a perda: escrevendo
     *
     *     w_k = y^{(k)}(0) · t^k / k!,
     *
     * a recorrência das derivadas transporta-se para os w directamente ---
     *
     *     w_{k+n} = − Σ_j c_j · w_{k+j} · t^{n-j} / ((k+j+1)···(k+n)),
     *
     * porque y^{(k+j)} = w_{k+j}·(k+j)!/t^{k+j}. Assim nunca se forma t^k nem
     * k!, nunca se divide um valor grande por outro, e a soma é dos próprios w.
     *
     * A primeira escrita disto guardava a derivada e o t^k/k! SEPARADOS e
     * multiplicava-os com duas divisões por mil --- e perdia: dava 0,16 onde a
     * forma fechada dá 0,307. O controlo apanhou-a, e é para isso que ele lá
     * está. */
    long w[SF_ORD];
    for(int k = 0; k < n && k < SF_ORD; k++){
        /* w_k = d0[k]·t^k/k!, com o t^k/k! por recorrência */
        long pot = S;
        for(int u = 0; u < k; u++) pot = (pot / (u+1)) * t / S;
        w[k] = (k == 0) ? d0[0] : (d0[k] / 1000) * (pot / 1000);
    }
    for(int k = 0; k < n; k++){ r.valor += w[k]; r.termos++; }
    for(int k = n; k < 64; k++){
        long nova = 0; int estourou = 0;
        for(int j = 0; j < n; j++){
            /* c_j · w_{k-n+j} · t^{n-j} / ((k-n+j+1)···k) */
            long termo = -co[j] * w[j];
            for(int u = 0; u < n - j; u++){
                if(termo > (1L<<50) || termo < -(1L<<50)){ estourou = 1; break; }
                termo = termo * t / S;
            }
            if(estourou) break;
            for(int u = k - n + j + 1; u <= k; u++) termo /= u;
            nova += termo;
        }
        if(estourou) break;
        for(int j = 0; j + 1 < n; j++) w[j] = w[j+1];
        w[n-1] = nova;
        if(nova == 0){ r.parou = 1; break; }
        r.valor += nova;
        r.termos++;
    }
    return r;
}

/* e o CONTROLO: a mesma solução pela forma fechada, quando as raízes são
 * inteiras --- y = Σ A_i e^{λ_i t}. Serve para confrontar, não para resolver. */
static long sf_fechada(const long *lam, const long *amp, int m, long t, long S){
    long v = 0;
    for(int i = 0; i < m; i++){
        SfSerie e = sf_exp(lam[i] * t / S, S);
        v += (amp[i] / 1000) * (e.valor / 1000);
    }
    return v;
}

#endif /* SERIE_H */
