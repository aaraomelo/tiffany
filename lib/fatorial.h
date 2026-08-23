/* ═══════════════════════════════════════════════════════════════════════════
 * lib/fatorial.h — A BASE FATORIAL: a exponencial, o π e as raízes em dígitos.
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
 * NOTA: este ficheiro é NOVO. As séries formais em Q --- `sr_exp`, `sr_sin`,
 * `sr_cos`, `sr_log1p` --- vivem em `lib/serie.h` e são outra coisa: lá os
 * coeficientes são exactos em Q, aqui avalia-se em escala inteira e conta-se
 * o custo. Escrevi por cima daquele ficheiro uma vez, e o compilador do banco
 * é que o disse --- por isso o nome mudou.
 *
 * Medido em `tests/fatorial.c`.
 * ═══════════════════════════════════════════════════════════════════════════ */
#ifndef FATORIAL_H
#define FATORIAL_H

#include <stdio.h>       /* snprintf, para a apresentação dos dígitos */

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

/* ═══ A RAIZ EM BASE FATORIAL: exacta, em dígitos, sem uma vírgula ═════════
 *
 * A série da aranha escreve tudo sobre 1/k!. A representação que lhe corresponde
 * é a BASE FATORIAL --- um número escreve-se
 *
 *     x = d_0 + Σ_{k≥1} d_k / k!,      com  0 ≤ d_k ≤ k,
 *
 * e ela não é uma escolha de conveniência: é a base em que e = Σ1/k! tem
 * dígitos todos iguais a 1, e em que os termos c(t) e s(t) já estão escritos.
 * Cada nível k tem k+1 dígitos possíveis --- a aridade CRESCE ---, e descer por
 * eles é o «navegar = descer por prefixo onde o prefixo é a bola» do paper: o
 * prefixo de k dígitos fixa uma bola, e escolher d_{k+1} parte-a em k+2.
 *
 * Aqui não há aproximação nem arredondamento: cada dígito é um INTEIRO decidido
 * pelo sinal de P, que se calcula em inteiros --- com x = N/k!, o valor
 * P(N/k!)·(k!)^n é inteiro e tem o mesmo sinal. O processo PÁRA onde o produto
 * deixa de caber no anel, e o nível a que parou é o CUSTO, que se conta.
 *
 * Medido em `tests/serie.c` §S5. */

#define FAT_NIV 12

typedef struct {
    long d[FAT_NIV+1];   /* os dígitos: d[0] inteiro, depois 0 ≤ d[k] ≤ k */
    int  niveis;         /* até onde desceu */
    int  parou_por;      /* 0 = exacto (P deu zero), 1 = o anel encheu */
    long num, den;       /* o valor como fracção N/k!, exacta */
    int  achou;
} FatRaiz;

/* sinal de P(N/D) sem sair dos inteiros: Σ c_j N^j D^{n-j}, com guarda */
static int fat_sinal(const long *co, int n, long N, long D, int *coube){
    long acc = 0; *coube = 1;
    const long T = 1L << 58;
    for(int j = n; j >= 0; j--){
        long cj = (j == n) ? 1 : co[j];
        long termo = cj;
        for(int u = 0; u < n - j; u++){
            if(termo > T/D || termo < -(T/D)){ *coube = 0; return 0; }
            termo *= D;
        }
        if(acc > T/(N?N:1) || acc < -(T/(N?N:1))){ *coube = 0; return 0; }
        acc = acc * N + termo;
    }
    return acc > 0 ? 1 : (acc < 0 ? -1 : 0);
}

/* desce os dígitos factoriais da raiz que vive em [a, a+1] */
static FatRaiz fat_raiz(const long *co, int n, long a){
    FatRaiz r; r.niveis = 0; r.parou_por = 1; r.achou = 0;
    r.num = 0; r.den = 1;
    for(int k = 0; k <= FAT_NIV; k++) r.d[k] = 0;
    int coube;
    int sa = fat_sinal(co, n, a, 1, &coube);
    if(!coube) return r;
    int sb = fat_sinal(co, n, a+1, 1, &coube);
    /* se P(a) ou P(a+1) é zero, a raiz é INTEIRA e não pede base nenhuma ---
     * o caminho das racionais já a dá, e diz-se em vez de se calar */
    if(sa == 0){ r.achou = 1; r.parou_por = 0; r.d[0] = a; r.num = a; r.den = 1;
                 r.niveis = 0; return r; }
    if(sb == 0){ r.achou = 1; r.parou_por = 0; r.d[0] = a+1; r.num = a+1; r.den = 1;
                 r.niveis = 0; return r; }
    if(!coube || sa == sb) return r;                         /* sem raiz cercada */
    r.achou = 1;
    r.d[0] = a;
    long N = a, D = 1;                       /* o prefixo corrente é N/D */
    for(int k = 1; k <= FAT_NIV; k++){
        long Dn = D * k;
        if(Dn / k != D) break;               /* o factorial estourou */
        long Nn = N * k;
        /* procura o maior d ≤ k com P((Nn+d)/Dn) do mesmo sinal que sa */
        long escolhido = 0; int falhou = 0;
        for(long d = 0; d <= k; d++){
            int s2 = fat_sinal(co, n, Nn + d, Dn, &coube);
            if(!coube){ falhou = 1; break; }
            if(s2 == 0){ escolhido = d; r.parou_por = 0; r.d[k] = d;
                         r.num = Nn + d; r.den = Dn; r.niveis = k; return r; }
            if(s2 == sa) escolhido = d; else break;
        }
        if(falhou) break;
        r.d[k] = escolhido;
        N = Nn + escolhido; D = Dn;
        r.niveis = k;
        r.num = N; r.den = D;
    }
    return r;
}

/* ═══ AS TRÊS FACES SAEM DA MESMA SÉRIE: ω² = t, com t ∈ {−1,0,+1} ═══════════
 *
 * O paper faz a cisão por paridade com J² = −1. Com ω² = t as potências ciclam
 * do mesmo modo, mas o que elas devolvem depende de t:
 *
 *     ω^{2k} = t^k,        ω^{2k+1} = t^k·ω,
 *
 * pelo que a exponencial se parte exactamente na mesma:
 *
 *     exp(xω) = c_t(x)·1 + s_t(x)·ω,
 *     c_t(x) = Σ_k t^k x^{2k}/(2k)!,     s_t(x) = Σ_k t^k x^{2k+1}/(2k+1)!
 *
 * e as TRÊS CLASSES do thm:leidisc são os três valores de t --- não três
 * teorias, três leituras da mesma série:
 *
 *   t = −1  ELÍPTICO     c,s = cos,sen    ω⁴ = 1, período 4   RODA
 *   t =  0  PARABÓLICO   c,s = 1, x       ω² = 0, nilpotente  DESLIZA
 *   t = +1  HIPERBÓLICO  c,s = cosh,senh  ω² = 1, período 2   FOGE
 *
 * E A CONSERVAÇÃO É UMA SÓ, que é a NORMA da tríade:
 *
 *     c_t² − t·s_t² = 1     (= N(c + sω), com N(a+bω) = a² − t b²)
 *
 * --- em t=−1 é c²+s²=1, em t=0 é c²=1, em t=+1 é c²−s²=1. A exponencial
 * PRESERVA A NORMA nas três faces, e isso não se impõe: sai da derivação,
 * (c²−ts²)' = 2cc' − 2t s s' = 2c(ts) − 2t s c = 0.
 *
 * E o caso t=0 é o que se vê de imediato: a série TERMINA em dois termos, porque
 * ω²=0 mata tudo o resto. O parabólico é a face onde a série é FINITA. */

typedef struct { long c, s; int termos; int parou; } FtFace;

static FtFace ft_face(long x, int t, long S){
    FtFace r; r.c = S; r.s = x; r.termos = 1; r.parou = 0;
    if(t == 0){ r.parou = 1; r.termos = 2; return r; }   /* ω²=0: a série ACABA */
    long Tc = S, Ts = x;
    for(int k = 0; k < 64; k++){
        /* Tc_{k+1} = t·Tc_k·x²/((2k+1)(2k+2)) */
        long a2 = Tc / (2*k+1); a2 = a2 * x / S; a2 = a2 / (2*k+2); a2 = a2 * x / S;
        long b2 = Ts / (2*k+2); b2 = b2 * x / S; b2 = b2 / (2*k+3); b2 = b2 * x / S;
        Tc = t * a2; Ts = t * b2;
        if(Tc == 0 && Ts == 0){ r.parou = 1; break; }
        r.c += Tc; r.s += Ts; r.termos++;
    }
    return r;
}

/* a norma da tríade avaliada na face: tem de dar S², seja qual for t */
static long ft_norma(FtFace f, int t, long S){
    (void)S;
    return (f.c/1000)*(f.c/1000) - (long)t * (f.s/1000)*(f.s/1000);
}

/* ── A MULTIPLICIDADE DE ω É CONTAGEM: aplica-se ω até voltar, e CONTA-SE.
 *
 * O paper não tabela isto --- «o índice k é o que conta as VOLTAS do ciclo de
 * quatro», «π lê-se contando voltas», e o thm:periodo lê |{G>1}| = p no campo
 * sem seguir a trajectória. A ordem de ω é o mesmo: o menor k>0 com ω^k = 1, e
 * acha-se APLICANDO.
 *
 * Um elemento a+bω escreve-se como o par (a,b); multiplicar por ω dá
 *
 *     (a + bω)·ω = aω + bω² = b·t + a·ω   →   (a,b) ↦ (t·b, a),
 *
 * que é a companheira 2×2 outra vez. Parte-se de 1 = (1,0) e conta-se até
 * voltar. Devolve 0 quando não volta --- o nilpotente, onde a órbita CAI no
 * zero em vez de fechar, e isso também se vê contando. */
static int ft_ordem(int t){
    long a = 1, b = 0;                          /* o elemento 1 = 1 + 0·ω */
    for(int k = 1; k <= 12; k++){               /* o lem:cristal fecha em 6 */
        long na = (long)t * b, nb = a;          /* ×ω */
        a = na; b = nb;
        if(a == 1 && b == 0) return k;          /* voltou: a ordem é k */
        if(a == 0 && b == 0) return 0;          /* caiu no zero: nilpotente */
    }
    return -1;                                   /* não fechou no tecto da rede */
}

/* e a ÓRBITA escrita, que é o que a contagem percorre --- para o cliente ver
 * de onde a ordem veio, em vez de a receber por decreto */
static void ft_orbita(int t, char *out, long lim){
    long a = 1, b = 0, o = 0;
    o += snprintf(out + o, lim - o, "1");
    for(int k = 1; k <= 6 && o < lim - 12; k++){
        long na = (long)t * b, nb = a;
        a = na; b = nb;
        if(a == 0 && b == 0){ o += snprintf(out + o, lim - o, " → 0"); break; }
        o += snprintf(out + o, lim - o, " → ");
        if(a && b)      o += snprintf(out + o, lim - o, "%ld%+ldw", a, b);
        else if(a)      o += snprintf(out + o, lim - o, "%ld", a);
        else            o += snprintf(out + o, lim - o, "%sw", b == 1 ? "" : (b == -1 ? "-" : ""));
        if(a == 1 && b == 0) break;
    }
}

/* ── A APRESENTAÇÃO: os dígitos fatoriais escritos como o cliente os lê,
 * sem o N/k! gigante. `lim` é o tamanho do buffer. */
static void ft_escreve(const FatRaiz *r, char *out, long lim){
    long o = 0;
    o += snprintf(out + o, lim - o, "%ld", r->d[0]);
    if(r->niveis > 0){
        o += snprintf(out + o, lim - o, ";");
        for(int k = 1; k <= r->niveis && o < lim - 8; k++)
            o += snprintf(out + o, lim - o, "%ld%s", r->d[k], k < r->niveis ? "," : "");
    }
    /* e o valor em milionésimos, que é o que se lê de relance --- calculado
     * dos DÍGITOS e não guardado à parte */
    if(r->den){
        long v = (r->num * 1000000L) / r->den;
        snprintf(out + o, lim - o, "  (%ld,%06ld)", v/1000000, v%1000000);
    }
}

/* ═══ A SOLUÇÃO DA EDO EM NOTAÇÃO FATORIAL: só CONTAGEM ═════════════════════
 *
 * Aqui não há operação em vírgula nenhuma, e não é preciso: os coeficientes da
 * solução SÃO inteiros. Escrevendo
 *
 *     y(t) = Σ_k d_k · t^k / k!,     d_k = y^{(k)}(0),
 *
 * a recorrência da equação DÁ os d_k, um a um, em inteiros exactos:
 *
 *     d_{k+n} = −c_{n-1} d_{k+n-1} − ... − c_0 d_k .
 *
 * Nada se avalia, nada se divide, nada se aproxima --- CONTA-SE. E o que o
 * cliente recebe é a lista dos d_k, que é a solução escrita na base em que a
 * série já vive. Avaliar num t é outra pergunta, e faz-se depois se se quiser.
 *
 * É a mesma disciplina do resto: o valor é uma leitura tardia do objecto, e o
 * objecto é a contagem. */

#define FT_COEF 24

typedef struct {
    long d[FT_COEF];     /* os coeficientes fatoriais, inteiros e exactos */
    int  n;              /* quantos cabem antes de o anel encher */
    int  ordem;          /* a ordem da equação */
    int  estourou;       /* 1 se parou por o coeficiente não caber */
} FtSol;

static FtSol ft_solucao(const long *co, int ordem, const long *d0, int quantos){
    FtSol r; r.n = 0; r.ordem = ordem; r.estourou = 0;
    if(quantos > FT_COEF) quantos = FT_COEF;
    for(int k = 0; k < ordem && k < quantos; k++){ r.d[k] = d0[k]; r.n++; }
    const long T = 1L << 55;
    for(int k = ordem; k < quantos; k++){
        long v = 0; int cabe = 1;
        for(int j = 0; j < ordem; j++){
            long termo = -co[j] * r.d[k - ordem + j];
            if(termo > T || termo < -T){ cabe = 0; break; }
            v += termo;
            if(v > T || v < -T){ cabe = 0; break; }
        }
        if(!cabe){ r.estourou = 1; break; }
        r.d[k] = v; r.n++;
    }
    return r;
}

/* a escrita para o cliente: ⟨d0, d1, d2, ...⟩ com o «!» a dizer a base */
static void ft_sol_escreve(const FtSol *s, char *out, long lim){
    long o = 0;
    o += snprintf(out + o, lim - o, "<");
    for(int k = 0; k < s->n && o < lim - 16; k++)
        o += snprintf(out + o, lim - o, "%s%ld", k ? "," : "", s->d[k]);
    snprintf(out + o, lim - o, "%s>!", s->estourou ? ",..." : "");
}

/* ── E O GUME QUE ELA PEDE: os coeficientes têm de SATISFAZER a recorrência.
 * Reconfere-se cada um a partir dos n anteriores --- se algum não bater, a
 * lista não é solução de equação nenhuma. Devolve quantos batem. */
static int ft_sol_confere(const FtSol *s, const long *co){
    int batem = 0;
    for(int k = s->ordem; k < s->n; k++){
        long v = 0;
        for(int j = 0; j < s->ordem; j++) v -= co[j] * s->d[k - s->ordem + j];
        if(v == s->d[k]) batem++;
    }
    return batem;
}

#endif /* FATORIAL_H */
