/* estrutura.h — ÁLGEBRA MODERNA: DEIXA-SE DE CALCULAR NÚMEROS E PASSA-SE A ESTUDAR
 * OPERAÇÕES.
 *
 *  ordem do coordenador põe a escada das estruturas — operação → semigrupo → monoide → grupo →
 * abeliano → subgrupo → homomorfismo → núcleo/imagem → isomorfismo → classes → quociente
 * → anel → domínio → corpo → ideal → anel quociente — e diz o que ela tem de especial
 * para esta casa:
 *
 *     «(G,⋆) é grupo quando ... todo a possui inverso» — «exatamente a reversibilidade
 *      que vocês já encontraram no andar dos inteiros»
 *
 * É literal: a folha a⁻¹ = −a de ℤ e o inverso do grupo são a MESMA condição, e o que a
 * álgebra moderna acrescenta é que ela pode ser exigida sem se dizer de que números se
 * fala. Por isso este andar não traz motor novo — traz o NOME do que já corria.
 *
 * ── A ESTRUTURA É UMA TÁBUA ────────────────────────────────────────────────────
 * Tudo aqui é FINITO e dado por tábua de Cayley: os elementos são 0..n−1 e a operação é
 * op[a][b]. Assim toda propriedade se varre EXAUSTIVAMENTE — a associatividade em n³, o
 * resto em n². Não há amostra: num objeto finito a varredura completa é a prova, e é por
 * isso que este andar cabe inteiro na máquina.
 *
 * E quando um teorema falha, a TESTEMUNHA exibe-se: o par que não comuta, o triplo que
 * não associa, o elemento sem inverso.
 *
 * Precisa de `inteiros.h`. */
#ifndef ESTRUTURA_H
#define ESTRUTURA_H

#define ES_MAX 24
typedef struct {
    int n;
    int op[ES_MAX][ES_MAX];
    const char *nome;
} Est;

/* ── AS ESTRUTURAS DO FICHEIRO ─────────────────────────────────────────────────── */
static int es_zn(Est *E, int n){
    if(n < 1 || n > ES_MAX){ E->n = 0; return 0; }                 /* (ℤₙ, +) — grupo abeliano */
    E->n = n; E->nome = "(Zn,+)";
    for(int a = 0; a < n; a++) for(int b = 0; b < n; b++) E->op[a][b] = (a + b) % n;
    return 1;
}
static int es_zn_mult(Est *E, int n){
    if(n < 1 || n > ES_MAX){ E->n = 0; return 0; }            /* (ℤₙ, ×) — monoide, e não grupo */
    E->n = n; E->nome = "(Zn,x)";
    for(int a = 0; a < n; a++) for(int b = 0; b < n; b++) E->op[a][b] = (a * b) % n;
    return 1;
}
/* a⋆b = a + b + 1, o exercício do §1 e §3 — fechada em ℤ, e o neutro é −1 */
static int es_mais_um(Est *E, int n){
    if(n < 1 || n > ES_MAX){ E->n = 0; return 0; }
    E->n = n; E->nome = "a*b=a+b+1";
    for(int a = 0; a < n; a++) for(int b = 0; b < n; b++) E->op[a][b] = (a + b + 1) % n;
    return 1;
}
/* a⋆b = a − b: NÃO associativa, e é o contra-exemplo que ele pede no §2 */
static int es_menos(Est *E, int n){
    if(n < 1 || n > ES_MAX){ E->n = 0; return 0; }
    E->n = n; E->nome = "a*b=a-b";
    for(int a = 0; a < n; a++) for(int b = 0; b < n; b++) E->op[a][b] = ((a - b) % n + n) % n;
    return 1;
}
/* S₃, o grupo simétrico: o gume do §5, porque NÃO é abeliano.
 * Os 6 elementos são as permutações de {0,1,2}, e a operação é a composição. */
static const int ES_S3[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
};
static void es_s3(Est *E){
    E->n = 6; E->nome = "S3";
    for(int a = 0; a < 6; a++) for(int b = 0; b < 6; b++){
        int c[3];
        for(int i = 0; i < 3; i++) c[i] = ES_S3[a][ ES_S3[b][i] ];   /* a ∘ b */
        for(int k = 0; k < 6; k++){
            if(ES_S3[k][0]==c[0] && ES_S3[k][1]==c[1] && ES_S3[k][2]==c[2]){ E->op[a][b] = k; break; }
        }
    }
}
/* (ℤₙ)*, as unidades sob × — grupo abeliano, e é onde vive o logaritmo discreto */
static int es_unidades(Est *E, int n, int *rot){
    int m = 0;
    for(int a = 1; a < n; a++) if(iz_gcd(a, n, 0, 0) == 1) rot[m++] = a;
    E->n = m; E->nome = "(Zn)*";
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
        int v = (rot[i] * rot[j]) % n;
        for(int k = 0; k < m; k++) if(rot[k] == v){ E->op[i][j] = k; break; }
    }
    return m;
}

/* ── AS PROPRIEDADES, VARRIDAS, E COM A TESTEMUNHA ─────────────────────────────── */
static int es_fechada(const Est *E){              /* a tábua não sai do conjunto */
    for(int a = 0; a < E->n; a++) for(int b = 0; b < E->n; b++)
        if(E->op[a][b] < 0 || E->op[a][b] >= E->n) return 0;
    return 1;
}
static int es_assoc(const Est *E, int *ta, int *tb, int *tc){
    for(int a = 0; a < E->n; a++) for(int b = 0; b < E->n; b++) for(int c = 0; c < E->n; c++)
        if(E->op[E->op[a][b]][c] != E->op[a][E->op[b][c]]){
            if(ta){ *ta = a; *tb = b; *tc = c; }
            return 0;
        }
    return 1;
}
static int es_neutro(const Est *E){               /* o índice do neutro, ou −1 */
    for(int e = 0; e < E->n; e++){
        int bom = 1;
        for(int a = 0; a < E->n && bom; a++)
            if(E->op[e][a] != a || E->op[a][e] != a) bom = 0;
        if(bom) return e;
    }
    return -1;
}
/* O INVERSO exige-se dos DOIS LADOS, e é assim que a definição o pede. Nas estruturas
 * desta bateria a segunda metade é redundante (medido por mutação: apagá-la não muda um
 * resultado), porque todas as que aqui entram são grupos ou são comutativas — e nesses
 * a⋆b = e já obriga b⋆a = e. A condição fica inteira porque a função é geral: num
 * monoide não comutativo há inversos só de um lado, e aí ela decide. */
static int es_inverso(const Est *E, int a){
    int e = es_neutro(E);
    if(e < 0) return -1;
    for(int b = 0; b < E->n; b++) if(E->op[a][b] == e && E->op[b][a] == e) return b;
    return -1;
}
static int es_grupo(const Est *E, int *sem_inverso){
    if(!es_fechada(E) || !es_assoc(E,0,0,0)) return 0;
    if(es_neutro(E) < 0) return 0;
    for(int a = 0; a < E->n; a++)
        if(es_inverso(E, a) < 0){ if(sem_inverso) *sem_inverso = a; return 0; }
    return 1;
}
static int es_abeliano(const Est *E, int *ta, int *tb){
    for(int a = 0; a < E->n; a++) for(int b = 0; b < E->n; b++)
        if(E->op[a][b] != E->op[b][a]){
            if(ta){ *ta = a; *tb = b; }
            return 0;
        }
    return 1;
}
/* ── SUBGRUPOS: o subconjunto é uma MÁSCARA de bits ────────────────────────────
 * O critério do ficheiro: a,b ∈ H ⟹ ab⁻¹ ∈ H. Um critério só, e é ele que se usa. */
static int es_em(unsigned H, int a){ return (H >> a) & 1u; }
static int es_subgrupo(const Est *E, unsigned H){
    if(!H) return 0;                              /* vazio não é subgrupo */
    for(int a = 0; a < E->n; a++){
        if(!es_em(H,a)) continue;
        for(int b = 0; b < E->n; b++){
            if(!es_em(H,b)) continue;
            int bi = es_inverso(E, b);
            if(bi < 0 || !es_em(H, E->op[a][bi])) return 0;
        }
    }
    return 1;
}
static int es_ordem_conj(unsigned H){
    int c = 0;
    for(int i = 0; i < ES_MAX; i++) if(es_em(H,i)) c++;
    return c;
}
static unsigned es_gerado(const Est *E, int a){   /* ⟨a⟩ */
    unsigned H = 0;
    int e = es_neutro(E), x = e;
    if(e < 0) return 0;
    for(int k = 0; k <= E->n; k++){ H |= 1u << x; x = E->op[x][a]; }
    return H;
}
/* a CLASSE LATERAL gH — e é ela que particiona o grupo */
static unsigned es_classe(const Est *E, unsigned H, int g){
    unsigned C = 0;
    for(int h = 0; h < E->n; h++) if(es_em(H,h)) C |= 1u << E->op[g][h];
    return C;
}
/* NORMAL: gHg⁻¹ = H para todo g — e é a condição que faz o quociente existir */
static int es_normal(const Est *E, unsigned H){
    if(!es_subgrupo(E,H)) return 0;
    for(int g = 0; g < E->n; g++){
        int gi = es_inverso(E,g);
        if(gi < 0) return 0;
        for(int h = 0; h < E->n; h++){
            if(!es_em(H,h)) continue;
            if(!es_em(H, E->op[E->op[g][h]][gi])) return 0;
        }
    }
    return 1;
}
/* ── HOMOMORFISMO: f preserva a operação — «não é só uma função» ───────────────── */
static int es_homo(const Est *G, const Est *H, const int *f, int *ta, int *tb){
    for(int a = 0; a < G->n; a++) for(int b = 0; b < G->n; b++)
        if(f[G->op[a][b]] != H->op[f[a]][f[b]]){
            if(ta){ *ta = a; *tb = b; }
            return 0;
        }
    return 1;
}
static unsigned es_nucleo(const Est *G, const Est *H, const int *f){
    unsigned K = 0;
    int e = es_neutro(H);
    for(int a = 0; a < G->n; a++) if(f[a] == e) K |= 1u << a;
    return K;
}
static unsigned es_imagem(const Est *G, const int *f){
    unsigned I = 0;
    for(int a = 0; a < G->n; a++) I |= 1u << f[a];
    return I;
}
static int es_injetiva(const Est *G, const int *f){
    for(int a = 0; a < G->n; a++) for(int b = a+1; b < G->n; b++) if(f[a] == f[b]) return 0;
    return 1;
}
/* ── O QUOCIENTE G/N: os elementos são as CLASSES ──────────────────────────────── */
static int es_quociente(const Est *G, unsigned N, Est *Q, unsigned *cls){
    if(!es_normal(G, N)) return 0;
    int m = 0;
    unsigned visto = 0;
    for(int g = 0; g < G->n; g++){
        if(es_em(visto,g)) continue;
        unsigned C = es_classe(G, N, g);
        cls[m++] = C;
        visto |= C;
    }
    Q->n = m; Q->nome = "G/N";
    for(int i = 0; i < m; i++) for(int j = 0; j < m; j++){
        int gi = -1, gj = -1;
        for(int g = 0; g < G->n; g++) if(es_em(cls[i],g)){ gi = g; break; }
        for(int g = 0; g < G->n; g++) if(es_em(cls[j],g)){ gj = g; break; }
        unsigned C = es_classe(G, N, G->op[gi][gj]);
        for(int k = 0; k < m; k++) if(cls[k] == C){ Q->op[i][j] = k; break; }
    }
    return m;
}
/* ── ISOMORFISMO: existe bijeção que preserva? (busca exaustiva, n ≤ 8) ────────── */
static int es_iso_busca(const Est *A, const Est *B, int *f, int k, int usados){
    if(A->n != B->n) return 0;
    if(k == A->n) return es_homo(A, B, f, 0, 0);
    for(int v = 0; v < B->n; v++){
        if(usados & (1 << v)) continue;
        f[k] = v;
        if(es_iso_busca(A, B, f, k+1, usados | (1 << v))) return 1;
    }
    return 0;
}
static int es_isomorfas(const Est *A, const Est *B, int *f){
    int g[ES_MAX];
    if(!f) f = g;
    return es_iso_busca(A, B, f, 0, 0);
}
/* ── ANÉIS: duas operações, e a distributividade a ligá-las ────────────────────── */
typedef struct { int n; int soma[ES_MAX][ES_MAX]; int mult[ES_MAX][ES_MAX]; } Anel;
/* O CONSTRUTOR RECUSA acima do teto, e devolve 0 em vez de escrever fora da tábua.
 * Chamá-lo com n = 40 escrevia 1600 inteiros num array de 24×24 — sobre a pilha, e o
 * programa entrava em ciclo sem dizer porquê. Um teto que não é verificado não é teto:
 * é uma suposição sobre quem chama. */
static int an_zn(Anel *R, int n){
    if(n < 1 || n > ES_MAX){ R->n = 0; return 0; }
    R->n = n;
    for(int a = 0; a < n; a++) for(int b = 0; b < n; b++){
        R->soma[a][b] = (a + b) % n;
        R->mult[a][b] = (a * b) % n;
    }
    return 1;
}
static int an_distrib(const Anel *R){
    for(int a = 0; a < R->n; a++) for(int b = 0; b < R->n; b++) for(int c = 0; c < R->n; c++)
        if(R->mult[a][R->soma[b][c]] != R->soma[R->mult[a][b]][R->mult[a][c]]) return 0;
    return 1;
}
/* DOMÍNIO INTEGRAL: sem divisores de zero — e a testemunha do falhanço exibe-se */
static int an_dominio(const Anel *R, int *ta, int *tb){
    for(int a = 1; a < R->n; a++) for(int b = 1; b < R->n; b++)
        if(R->mult[a][b] == 0){ if(ta){ *ta = a; *tb = b; } return 0; }
    return 1;
}
static int an_corpo(const Anel *R, int *sem){
    for(int a = 1; a < R->n; a++){
        int tem = 0;
        for(int b = 1; b < R->n; b++) if(R->mult[a][b] == 1 % R->n) tem = 1;
        if(!tem){ if(sem) *sem = a; return 0; }
    }
    return 1;
}
/* IDEAL: fechado para a diferença e ABSORVENTE para o produto */
static int an_ideal(const Anel *R, unsigned I){
    if(!I) return 0;
    for(int a = 0; a < R->n; a++){
        if(!es_em(I,a)) continue;
        for(int b = 0; b < R->n; b++){
            if(es_em(I,b)){
                int mb = -1;                       /* o oposto de b */
                for(int t = 0; t < R->n; t++) if(R->soma[b][t] == 0) mb = t;
                if(mb < 0 || !es_em(I, R->soma[a][mb])) return 0;
            }
            /* A ABSORÇÃO — e em ℤₘ ela é AUTOMÁTICA: todo subgrupo aditivo é dℤₘ e
             * r·(dk) = d(rk) já lá está. Medido por mutação: apagar esta linha não muda
             * um único resultado sobre ℤₘ, e em ℤ₆ há 4 subgrupos aditivos e 4 ideais.
             * A linha fica porque a função é sobre ANÉIS em geral, onde a condição
             * decide — mas quem a ler não deve julgá-la exercitada por esta bateria. */
            if(!es_em(I, R->mult[b][a])) return 0;
        }
    }
    return 1;
}
#endif
