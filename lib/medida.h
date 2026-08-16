/* medida.h — A CONSERVAÇÃO MÉTRICA POR DUALIDADE, e a meta-indução que a prova.
 *
 * O `eval.txt` abre com a correção que este ficheiro existe para honrar:
 *
 *   «estourar o tipo NÃO é resultado matemático. É falha de representação. O teorema tem
 *    de sobreviver à troca de representação.»
 *
 * E manda formalizar, por esta ordem: primeiro a DESCIDA — indução e meta-indução —, e só
 * depois a conservação métrica que ela prova.
 *
 * ── PRIMEIRO: A DESCIDA, E POR QUE ELA É O DUAL DA INDUÇÃO ─────────────────────
 * A indução sobe: base em 0, e P(n) ⟹ P(n+1). A descida desce: de um contra-exemplo em n
 * fabrica-se um em n−1, e como não há descida infinita em ℕ, contra-exemplo nenhum existe.
 *
 * As duas são o MESMO facto lido nos dois sentidos — ℕ é bem ordenado —, e são duais no
 * sentido preciso desta casa: a involução é inverter a ordem, e ν∘ν = id. A indução
 * PROJECTA (constrói o andar seguinte); a descida LÊ (recusa o andar anterior). É o par
 * λ⁺/λ⁻ da `def:inducao` do Corpo Universal, agora com as duas metades nomeadas.
 *
 * E a META-INDUÇÃO é o degrau acima: o que se mede não é uma tabela de andares, é O PASSO.
 * Um passo cujo corpo não menciona n prova P(n) para todo n — e a ausência de tecto é
 * consequência da FORMA da construção, T_{n+1} = T_n ⊕ T_n*, e não de uma varredura.
 * Foi exactamente o defeito que esta casa apanhou hoje duas vezes: contar «5 de 8» quando
 * as três em falta eram o `long` a estourar, e chamar «sem tecto» a um array de 64.
 *
 * ── DEPOIS: A CONSERVAÇÃO ─────────────────────────────────────────────────────
 * O determinante é o factor de volume: A(v₁)∧…∧A(vₙ) = det(A)·v₁∧…∧vₙ. Logo |det A| = 1
 * é a conservação da medida, μ(AE) = μ(E) — e no lado contínuo é o Jacobiano da mudança
 * de variável, μ(T(E)) = ∫_E |det DT|.
 *
 * ── E O QUE AQUI NÃO SE FAZ ───────────────────────────────────────────────────
 * O eval define σ_n(A) := |det A|. Com essa definição, «det|·| = σ_n» é VERDADE POR
 * DEFINIÇÃO, e uma asserção que não pode falhar não é medição — é cerimónia. O conteúdo
 * está noutro sítio, e é ele que se mede: esse MESMO número é, ao mesmo tempo,
 *
 *   (a) o determinante, por eliminação — álgebra;
 *   (b) o factor da ação em Λⁿ, pela cunha das colunas — geometria;
 *   (c) o produto dos valores próprios σσ′ — espectro;
 *   (d) a razão entre pontos do reticulado na imagem e na fonte — CONTAGEM, e esta não
 *       toca em determinante nenhum.
 *
 * Quatro contas independentes e um só número. E σσ′ = −1, que a casa já tinha escrito no
 * Corpo de Peano e no Universal, É |det A_m| = 1 — a hipótese estrutural do eval e a
 * conservação da área são a mesma frase.
 *
 * Precisa de `racionais.h`, `linear.h`, `exterior.h`, `corpos.h`. */
#ifndef MEDIDA_H
#define MEDIDA_H

static long md_estouros = 0;
static long md_saturou  = 0;    /* quantas vezes uma REPRESENTAÇÃO não coube — e isto
                                 * conta-se à parte dos defeitos, porque não é um */

/* ═══ PARTE I — A DESCIDA: INDUÇÃO E META-INDUÇÃO ══════════════════════════════
 *
 * Uma proposição sobre andares representa-se pelo que se pode medir dela: o valor do
 * invariante no andar. Aqui o invariante é |det| do operador do andar, e a proposição é
 * P(n): «o operador do andar n conserva a medida». */

/* ── A BASE: o andar zero ──────────────────────────────────────────────────────
 * det(I) = 1, e é a única coisa que a base afirma. */
static Qz mi_base(void){ return qz(1,1); }

/* ── O PASSO: e repare-se que ele NÃO RECEBE n ─────────────────────────────────
 * A extensão dual T ↦ T ⊕ T* leva det(T) para det(T)·det(T*). Para o dual métrico
 * T* = T⁻ᵀ tem-se det(T*) = 1/det(T), e o produto é 1 — SEJA QUAL FOR o andar, porque a
 * conta não menciona a dimensão. É este o conteúdo da meta-indução: o passo é um só, e o
 * «para todo n» sai da forma dele e não de o correr muitas vezes.
 *
 * Recebe det(T) e devolve det(T ⊕ T*). Uma função sem n. */
static int mi_passo(Qz det_T, Qz *det_extensao){
    if(det_T.p == 0) return 0;                 /* T sem inversa: a extensão dual não existe */
    Qz det_dual;
    if(!qz_divide(qz(1,1), det_T, &det_dual)) return 0;   /* det(T*) = 1/det(T) */
    *det_extensao = qz_mult(det_T, det_dual);
    return 1;
}
/* ── A DESCIDA, que é o DUAL do passo — e a forma CERTA dela ───────────────────
 * Escrevi primeiro uma descida que recebia um número e o empurrava para baixo, e ela era
 * oca: dei-lhe racionais que nunca foram determinantes da torre, ela «desceu» a copiá-los,
 * e sessenta em sessenta «sobreviveram à base». O defeito não era o resultado — era eu
 * ter dado à descida um objecto que não é do domínio dela.
 *
 * A descida certa é a do CONTRA-EXEMPLO MÍNIMO. A tese é «a conservação vale em todo
 * andar»; a negação dela é «há um primeiro andar onde falha». Procura-se esse primeiro
 * andar. Se ele existisse em N, o passo aplicado em N−1 tê-lo-ia produzido — e o passo
 * produz 1, sempre. Logo não existe, e a procura tem de voltar VAZIA.
 *
 * Uma procura que volta vazia só vale alguma coisa se puder achar. Por isso o passo entra
 * por PARÂMETRO: com o verdadeiro ela volta vazia, e com um sabotado ela acha — e é o
 * segundo controlo que faz do primeiro uma medição. */
typedef int (*MiPasso)(Qz, Qz *);

/* o passo SABOTADO: duplica a cada andar, e portanto quebra em N = 1 */
static int mi_passo_sabotado(Qz det_T, Qz *det_extensao){
    if(det_T.p == 0) return 0;
    *det_extensao = qz_mult(det_T, qz(2,1));
    return 1;
}
/* a procura do PRIMEIRO andar onde a conservação falha. Devolve 1 se achou. */
static int mi_procura_minimo(MiPasso passo, int ate, int *nivel){
    Qz d = mi_base();
    for(int n = 1; n <= ate; n++){
        Qz prox;
        if(!passo(d, &prox)){ if(nivel) *nivel = n; return 1; }   /* o passo falhou */
        if(!qz_igual(prox, qz(1,1)) && !qz_igual(prox, qz(-1,1))){
            if(nivel) *nivel = n;
            return 1;                                             /* achou o mínimo */
        }
        d = prox;
    }
    return 0;                                                     /* voltou vazia */
}
/* e a leitura DUAL do mesmo facto: se não há andar mínimo onde falhe, não há andar
 * nenhum onde falhe — porque ℕ é bem ordenado, e é só isso que a descida usa. */
static int mi_sem_contraexemplo(MiPasso passo, int ate){
    int n = 0;
    return !mi_procura_minimo(passo, ate, &n);
}
/* ── E A META-INDUÇÃO MEDIDA: o passo aplicado a entradas ARBITRÁRIAS ──────────
 * A tese «para todo n» não se varre. O que se mede é que o passo devolve 1 para QUALQUER
 * det de entrada invertível — e se ele o faz sem nunca ver n, então vale em todo andar.
 * A varredura aqui não é sobre andares: é sobre ENTRADAS do passo, que é outra coisa. */
static int mi_passo_vale(Qz det_T){
    Qz e;
    if(!mi_passo(det_T, &e)) return 0;
    return qz_igual(e, qz(1,1));
}

/* ═══ PARTE II — O DETERMINANTE É O FACTOR DE VOLUME ═══════════════════════════ */

/* ── (b) A AÇÃO EM Λⁿ: a cunha das colunas ────────────────────────────────────
 * Para n = 2 a cunha das duas colunas é o menor 2×2; para n = 3 é o produto misto. É a
 * MESMA quantidade que o determinante, e chega lá por outro caminho: aqui não há
 * eliminação nenhuma, há alternância. */
static Vec md_coluna(Mat A, int j){
    Vec v = vec0(A.m);
    for(int i = 0; i < A.m; i++) v.c[i] = A.a[i][j];
    return v;
}
static Qz md_volume_wedge(Mat A){
    if(A.n == 2){
        Vec u = md_coluna(A,0), v = md_coluna(A,1);
        return qz_soma(qz_mult(u.c[0],v.c[1]), qz_oposto(qz_mult(u.c[1],v.c[0])));
    }
    if(A.n == 3) return ex_misto(md_coluna(A,0), md_coluna(A,1), md_coluna(A,2));
    md_estouros++;
    return qz(0,1);
}
/* ── (c) O ESPECTRO: σ e σ′ do gato, e o produto deles ─────────────────────────
 * A_m = [[m,1],[1,0]] tem polinómio característico λ² − mλ − 1, logo σ + σ′ = m e
 * σσ′ = −1. O produto dos valores próprios É o determinante, e não se avalia raiz
 * nenhuma para o dizer: sai dos COEFICIENTES, que são inteiros. */
static long md_traco_gato(long m){ return m; }
static long md_produto_proprios(long m){ (void)m; return -1; }   /* σσ′ = −1, sempre */
static Mat md_gato(long m){
    Mat A = mat0(2,2);
    A.a[0][0] = qz_de_inteiro(m); A.a[0][1] = qz(1,1);
    A.a[1][0] = qz(1,1);          A.a[1][1] = qz(0,1);
    return A;
}
/* e a ESTACA, que é o dual: σ† = m − σ, e σσ† = −1 verifica-se nos coeficientes */
static int md_estaca_fecha(long m){
    /* σ² = mσ + 1  ⟹  σ(m − σ) = mσ − σ² = mσ − mσ − 1 = −1, para todo m */
    return md_produto_proprios(m) == -1 && md_traco_gato(m) == m;
}
/* ── (d) A CONTAGEM: pontos do reticulado, e NÃO toca em determinante ──────────
 * Se A tem entradas inteiras e |det A| = 1, então A leva ℤ² sobre ℤ² — é uma bijecção do
 * reticulado. Mede-se contando: quantos pontos de uma caixa têm pré-imagem inteira. Com
 * |det| = 1 são todos; com |det| = k são um em cada k.
 *
 * Esta é a realização de Lebesgue no discreto, e é a que sobrevive à troca de
 * representação: são somas de inteiros pequenos, e não há nada para estourar. */
static int md_preimagem_inteira(Mat A, long x, long y, long *u, long *v){
    Qz d = mat_det(A);
    if(d.p == 0) return 0;
    /* a inversa de [[a,b],[c,d]] é (1/det)·[[d,−b],[−c,a]] */
    long a = A.a[0][0].p, b = A.a[0][1].p, c = A.a[1][0].p, dd = A.a[1][1].p;
    long det = a*dd - b*c;
    long nu = dd*x - b*y, nv = -c*x + a*y;
    if(nu % det || nv % det) return 0;
    *u = nu/det; *v = nv/det;
    return 1;
}
static long md_conta_imagem(Mat A, long R){
    long n = 0;
    for(long x = -R; x <= R; x++) for(long y = -R; y <= R; y++){
        long u, v;
        if(md_preimagem_inteira(A, x, y, &u, &v)) n++;
    }
    return n;
}
/* ── E A MESMA CONTAGEM PELO OUTRO LADO: GERANDO ───────────────────────────────
 * A primeira conta desce — para cada ponto da caixa, pergunta se tem pré-imagem inteira.
 * Esta sobe — percorre o reticulado de partida, aplica A, e conta os que CAEM na caixa.
 * São dois caminhos sem código em comum, e têm de dar o mesmo número.
 *
 * Escrevi primeiro a razão «um em cada |det|» como referência à mão, e ela estava errada:
 * numa caixa simétrica [−12,12] os múltiplos de 3 são 9 e não 25/3. A referência escrita
 * à mão é o defeito desta casa, e a maneira de não o cometer é não ter referência: pôr
 * duas contas a concordar. */
static long md_conta_gerando(Mat A, long R, long alcance){
    long n = 0;
    long a = A.a[0][0].p, b = A.a[0][1].p, c = A.a[1][0].p, d = A.a[1][1].p;
    for(long u = -alcance; u <= alcance; u++) for(long v = -alcance; v <= alcance; v++){
        long x = a*u + b*v, y = c*u + d*v;
        if(x >= -R && x <= R && y >= -R && y <= R) n++;
    }
    return n;
}
/* ═══ PARTE III — O LADO CONTÍNUO: O JACOBIANO ════════════════════════════════
 * μ(T(E)) = ∫_E |det DT|. Para T polinomial, DT é exacto em ℚ e o determinante avalia-se
 * ponto a ponto. O caso conservativo é |det DT| ≡ 1 — e o cisalhamento (x,y) ↦ (x + y², y)
 * é-o em TODO ponto, apesar de não ser linear: é a testemunha de que a lei é local. */
typedef struct { long a, b, c; } Cis;   /* T(x,y) = (x + a·y² + b·y + c, y) */

static Mat md_jacobiano_cisalha(Cis t, Qz y){
    Mat J = mat0(2,2);
    J.a[0][0] = qz(1,1);
    J.a[0][1] = qz_soma(qz_mult(qz_de_inteiro(2*t.a), y), qz_de_inteiro(t.b));  /* ∂/∂y */
    J.a[1][0] = qz(0,1);
    J.a[1][1] = qz(1,1);
    return J;
}
/* e o controlo: uma DILATAÇÃO, cujo Jacobiano é constante k e NÃO é 1 */
static Mat md_jacobiano_dilata(long k){
    Mat J = mat0(2,2);
    J.a[0][0] = qz_de_inteiro(k); J.a[1][1] = qz(1,1);
    return J;
}
/* ═══ PARTE IV — A SEGUNDA REALIZAÇÃO, quando a primeira não cabe ═════════════
 * O eval: «a conservação deve ser verificada por uma SEGUNDA REALIZAÇÃO INDEPENDENTE
 * sempre que a primeira atingir o seu limite representacional».
 *
 * A primeira realização é o determinante em Qz, e ela satura: as entradas de Aᵏ crescem
 * como σᵏ e ao fim de poucas potências o produto p·q não cabe num `long`. A segunda é o
 * determinante calculado MODULARMENTE — det(Aᵏ) mod P para primos P distintos —, que não
 * cresce nunca, porque os resíduos vivem numa caixa fixa.
 *
 * E o ponto é o que se conclui quando a primeira falha: NADA sobre a lei. A saturação
 * conta-se em `md_saturou`, que é outro sítio que não os defeitos. */
static long md_det_mod(Mat A, long P){
    long a = ((A.a[0][0].p % P) + P) % P, b = ((A.a[0][1].p % P) + P) % P;
    long c = ((A.a[1][0].p % P) + P) % P, d = ((A.a[1][1].p % P) + P) % P;
    return (((a*d - b*c) % P) + P) % P;
}
static Mat md_mat_mod(Mat A, Mat B, long P){
    Mat C = mat0(2,2);
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++){
        long s = 0;
        for(int k = 0; k < 2; k++) s = (s + A.a[i][k].p * B.a[k][j].p) % P;
        C.a[i][j] = qz_de_inteiro(((s % P) + P) % P);
    }
    return C;
}
/* a potência k-ésima do gato, em resíduos: nunca cresce, e responde a todo k */
static long md_det_potencia_mod(long m, long k, long P){
    Mat A = md_gato(m);
    for(int i = 0; i < 2; i++) for(int j = 0; j < 2; j++)
        A.a[i][j] = qz_de_inteiro(((A.a[i][j].p % P) + P) % P);
    Mat R = mat_id(2);
    for(long i = 0; i < k; i++) R = md_mat_mod(R, A, P);
    return md_det_mod(R, P);
}
/* e a primeira realização, que SATURA — e conta-se onde deve */
static int md_det_potencia_exacto(long m, long k, Qz *det){
    Mat A = md_gato(m), R = mat_id(2);
    for(long i = 0; i < k; i++){
        Mat C = mat0(2,2);
        for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
            __int128 s = 0;
            for(int c = 0; c < 2; c++)
                s += (__int128)R.a[a][c].p * A.a[c][b].p;
            if(s > 3000000000LL || s < -3000000000LL){ md_saturou++; return 0; }
            C.a[a][b] = qz_de_inteiro((long)s);
        }
        R = C;
    }
    *det = mat_det(R);
    return 1;
}
/* ═══ PARTE V — OS DOIS MOVIMENTOS: SOBE EM ESPIRAL, DESCE DISCRETO ═══════════
 * O gato sobe pela extensão dual (contínuo na representação) e desce pela projecção
 * métrica (discreto na leitura). A casa já tinha o par medido em `tests/cone_espiral.c`:
 *
 *      Π : ℝ → ℕ^ℕ     o CONE, extracção discreta       (a descida)
 *      Σ : ℕ^ℕ → ℝ     a ESPIRAL, reconstrução contínua (a subida)
 *      Σ∘Π = Id_ℝ,  mas  Π∘Σ ≠ Id  — é uma RETRAÇÃO, não uma involução
 *
 * Lá o Π corre em vírgula flutuante e o próprio ficheiro conta o que isso lhe custou.
 * Aqui corre em INTEIROS, porque é Euclides e sai exacto — e o que se mede é a metade
 * MÉTRICA, que é nova: cada passo do cone é a matriz [[a,1],[1,0]], com det = −1. Logo
 *
 *      |det| = 1 nos DOIS compostos, e só UM deles é a identidade.
 *
 * A conservação da medida é estritamente mais fraca que o fecho do laço, e a diferença
 * conta-se: as duas expansões de um racional dão matrizes DIFERENTES com o MESMO |det|.
 * O gato desce ao mesmo tronco e não às mesmas marcas de unha. */
#define MD_CF 40

/* Π — o cone, em inteiros: Euclides, e escreve os quocientes */
static int md_cone(long p, long q, long *a, int max){
    int k = 0;
    if(q == 0) return 0;
    while(q != 0 && k < max){
        long d = p / q, r = p % q;
        if(r < 0){ d--; r += (q > 0 ? q : -q); }     /* o quociente para BAIXO */
        a[k++] = d; p = q; q = r;
    }
    return k;
}
/* Σ — a espiral, em inteiros: recompõe de trás para a frente, exacto */
static int md_espiral(const long *a, int n, long *p, long *q){
    if(n <= 0) return 0;
    long P = a[n-1], Q = 1;
    for(int i = n - 2; i >= 0; i--){
        long nP = a[i] * P + Q, nQ = P;
        if(P > 1000000000L || Q > 1000000000L){ md_saturou++; return 0; }
        P = nP; Q = nQ;
    }
    *p = P; *q = Q;
    return 1;
}
/* a MATRIZ do percurso: o produto dos [[a,1],[1,0]] — e o det dela é (−1)^n */
static long md_det_percurso(const long *a, int n){
    long d = 1;
    for(int i = 0; i < n; i++) d = -d;             /* cada passo tem det = −1 */
    return d;
}
/* a OUTRA expansão: [a₀,…,aₙ] ↔ [a₀,…,aₙ−1,1]. São exactamente duas, e só duas. */
static int md_outra_expansao(const long *a, int n, long *b, int max){
    if(n <= 0 || n + 1 > max) return 0;
    for(int i = 0; i < n; i++) b[i] = a[i];
    if(b[n-1] <= 1) return 0;                      /* já é a que termina em 1 */
    b[n-1] -= 1; b[n] = 1;
    return n + 1;
}
/* ═══ PARTE VI — A BIDUALIDADE: o gato é o PONTO FIXO, o passarinho volta ═════
 * O dual métrico inverte o factor de medida: det(T*) = 1/det(T). Logo
 *
 *      PRIMEIRA dualidade   d ↦ 1/d        e ela NÃO é a identidade
 *      SEGUNDA  (bidual)    d ↦ 1/d ↦ d    e ela É — a Lei 1, †∘† = id
 *
 * E daí sai o que faz do gato um gato: ele é o PONTO FIXO da primeira,
 *
 *      d = 1/d  ⟺  d² = 1  ⟺  |det| = 1,
 *
 * que é exactamente a conservação da medida. Ou seja, conservar a medida É ser o seu
 * próprio dual métrico. O passarinho — um corpo qualquer, uma instância — não é ponto
 * fixo: precisa das DUAS aplicações para voltar. O gato volta à primeira.
 *
 * Esta é a lei desta casa em dois níveis: toda representação tem dual, e exigir que o
 * PASSO seja o dual dá a forma. Aqui o passo é a medida, e a forma é |det| = 1. */
static int md_dual_medida(Qz d, Qz *dual){
    if(d.p == 0) return 0;                       /* sem dual: a fibra é vazia */
    return qz_divide(qz(1,1), d, dual);
}
static int md_bidual(Qz d, Qz *volta){
    Qz u;
    if(!md_dual_medida(d, &u)) return 0;
    return md_dual_medida(u, volta);
}
/* é ponto fixo da PRIMEIRA dualidade? — e isto é |det| = 1, dito de outra maneira */
static int md_ponto_fixo(Qz d){
    Qz u;
    if(!md_dual_medida(d, &u)) return 0;
    return qz_igual(d, u);
}
#endif
