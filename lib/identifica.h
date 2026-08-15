/* identifica.h — O MESMO PONTO, POR QUATRO PORTAS, E A IDENTIFICAÇÃO MEDIDA.
 *
 * O `eval.txt` fecha o andar com o pedido exato:
 *
 *   «mostrar que esse fechamento NÃO DEPENDE DO MÉTODO ESCOLHIDO: corte, Cauchy,
 *    bisseção e FC têm de produzir O MESMO PONTO, com a VOLTA verificando a
 *    identificação.»
 *
 * E antes disso a regra que proíbe o atalho: o critério aₙ − bₙ → 0 «é justamente a
 * ponte que permite identificar os dois SEM SIMPLESMENTE DECLARAR QUE SÃO IGUAIS».
 *
 * Então não se declara. O real É o corte — é essa a definição do andar —, e portanto
 * dois métodos dão o mesmo ponto exatamente quando INDUZEM O MESMO CORTE: para cada
 * racional q, o mesmo lado. É isto que aqui se mede, e nos SEIS pares, não só contra o
 * corte (que seria eleger um árbitro e chamar-lhe acordo).
 *
 *   corte     DECISÃO                 q² < a — e decide toda a gente de uma vez
 *   Möbius    DINÂMICA                duas órbitas, de baixo e de cima: o par dual
 *   bisseção  ENCAIXOTAMENTO          as pontas da caixa, largura (b₀−a₀)/2ᵏ
 *   FC        REPRESENTAÇÃO DISCRETA  os convergentes, que alternam à volta do ponto
 *
 * A INDECISÃO DIZ-SE. Um método com esforço finito não decide os racionais que ainda
 * caem dentro da sua caixa — e isso é 0, não é um lado. Fingir que decidiu seria a
 * fraude que dá o acordo de graça; e é por isso que se conta quantos ficaram indecisos e
 * se mede que o número CAI quando o esforço sobe.
 *
 * Precisa de `racionais.h`, `reais.h`, `cauchy.h` e `cifra.h`. */
#ifndef IDENTIFICA_H
#define IDENTIFICA_H

#define ID_ABAIXO (-1)
#define ID_ACIMA  (+1)
#define ID_INDECISO 0

/* ── 1. o CORTE: a decisão, e é a definição ────────────────────────────────────── */
static int id_corte(long a, Qz q){
    if(q.p <= 0) return ID_ABAIXO;
    int bom, s = rz_cmp(q, 2, a, &bom);
    if(!bom) return ID_INDECISO;
    return s < 0 ? ID_ABAIXO : (s > 0 ? ID_ACIMA : ID_INDECISO);
}
/* ── 2. o MÖBIUS: a dinâmica, e são DUAS órbitas — o par dual ──────────────────
 * De ⌊√a⌋ sobe e fica sempre abaixo; de ⌊√a⌋+1 desce e fica sempre acima. Uma órbita
 * só decidia um lado, e um lado só é meia identificação. */
static int id_mobius(long a, Qz q, long passos){
    long b = rz_b(a);
    Qz x = qz_de_inteiro(raizi(a));          /* sobe, por baixo */
    Qz y = qz_de_inteiro(raizi(a) + 1);      /* desce, por cima */
    for(long k = 0; k <= passos; k++){
        if(qz_menor(q, x)) return ID_ABAIXO;
        if(qz_menor(y, q)) return ID_ACIMA;
        x = rz_passo(a, b, x);
        y = rz_passo(a, b, y);
    }
    return ID_INDECISO;                      /* ainda dentro do par: e diz-se */
}
/* ── 3. a BISSEÇÃO: o encaixotamento ───────────────────────────────────────────── */
static int id_bisec(long a, Qz q, int dobras){
    Corte c = { a, 2 };
    Qz lo, hi;
    if(!rz_caixa_inicial(c, &lo, &hi)) return ID_INDECISO;
    rz_encaixota(c, &lo, &hi, dobras);
    if(qz_menor(q, lo) || qz_igual(q, lo)) return ID_ABAIXO;
    if(qz_menor(hi, q) || qz_igual(q, hi)) return ID_ACIMA;
    return ID_INDECISO;                      /* dentro da caixa: ainda não decidiu */
}
/* ── 4. a FRAÇÃO CONTÍNUA: a representação discreta ────────────────────────────
 * Os convergentes ALTERNAM à volta do ponto — os de índice par por baixo, os ímpares por
 * cima —, e é essa alternância que os torna um crivo. */
static int id_fc(long a, Qz q, int termos){
    long t[48];
    size_t nt = lado(0, -a, t, 48);
    if(!nt) return ID_INDECISO;
    long pn = 1, qn = 0, pa = 0, qa = 1;
    for(int i = 0; i < termos; i++){
        long ai = t[(size_t)i < nt ? (size_t)i : (1 + (i - 1) % (int)(nt > 1 ? nt - 1 : 1))];
        long pp = ai*pn + pa, qq = ai*qn + qa;
        if(qq <= 0 || qq > (1L<<28)) break;
        pa = pn; qa = qn; pn = pp; qn = qq;
        Qz c = qz(pn, qn);
        if(i % 2 == 0){ if(qz_menor(q, c)) return ID_ABAIXO; }   /* par: está por baixo */
        else          { if(qz_menor(c, q)) return ID_ACIMA;  }   /* ímpar: por cima */
    }
    return ID_INDECISO;
}
/* ── O QUADRO: os quatro lados de um mesmo racional ────────────────────────────── */
typedef struct { int v[4]; } Quatro;         /* corte, möbius, bisseção, FC */
static Quatro id_quatro(long a, Qz q, int esforco){
    Quatro r;
    r.v[0] = id_corte(a, q);
    r.v[1] = id_mobius(a, q, esforco);
    r.v[2] = id_bisec(a, q, esforco);
    r.v[3] = id_fc(a, q, esforco);
    return r;
}
/* CONCORDAM? Dois métodos DISCORDAM só quando ambos decidem e decidem ao contrário.
 * Um indeciso não é um desacordo — é uma medida que ainda não terminou, e a diferença
 * entre as duas coisas é a diferença entre medir e fingir. */
static int id_choque(Quatro q, int i, int j){
    return q.v[i] != ID_INDECISO && q.v[j] != ID_INDECISO && q.v[i] != q.v[j];
}
static int id_choques(Quatro q){             /* nos SEIS pares, e não contra um árbitro */
    int n = 0;
    for(int i = 0; i < 4; i++) for(int j = i+1; j < 4; j++) if(id_choque(q, i, j)) n++;
    return n;
}
static int id_indecisos(Quatro q){
    int n = 0;
    for(int i = 0; i < 4; i++) if(q.v[i] == ID_INDECISO) n++;
    return n;
}
static const char *id_nome(int i){
    return i == 0 ? "corte" : i == 1 ? "Möbius" : i == 2 ? "bisseção" : "FC";
}
/* o RASTRO de cada porta — e os nomes concordam em género, porque a linha é para ler */
static const char *id_rastro(int i){
    return i == 0 ? "decisão" : i == 1 ? "dinâmica"
         : i == 2 ? "caixa que aperta" : "representação discreta";
}
#endif
