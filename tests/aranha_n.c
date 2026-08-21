/* aranha_n.c — A ARANHA EM ℤⁿ: o teorema não usa o 2.
 *
 * O `thm:multiplicidade` (arquitetura.tex §sec:aranha) está escrito para
 *
 *     π : I ⟶ ℤ²,        G(x) = |π⁻¹(x)|
 *
 * e o `aranha_g.c` realiza-o num array `G[2048][2048]` — o plano metido no
 * TIPO. Em ℤ³ o mesmo array pediria 34 GB, e a torre vai a oito.
 *
 * MAS A PROVA NÃO USA O DOIS. Lida cláusula a cláusula:
 *   (1) duplicidade é dobra    `i ∼ j ⟺ π(i) = π(j)` — é sobre a FIBRA
 *   (2) G regista a dobra      `G(x) > 1` — conta a fibra
 *   (3) a memória é do espaço  `G_{t+1} = G_t + 1_{π(t)}` — escrita local
 *   (4) sentir é ler G         a vizinhança V(x) — e é SÓ aqui que o n entra,
 *                              porque V(x) tem 2n vizinhos
 * Nenhuma delas menciona a dimensão. O ℤ² era do exemplo, não do teorema.
 *
 * Então o algoritmo escreve-se UMA vez, com n por parâmetro, e as realizações
 * entram por cima:
 *
 *   n=1  CANTOR      o terço do meio: o endereço ternário na recta
 *   n=2  DRAGÃO      Heighway — e tem de dar os MESMOS números do aranha_g.c
 *   n=3  DRAGÃO NO ESPAÇO   a mesma dobra com um terceiro eixo
 *   n=2  JULIA       a órbita z ↦ z² + c no reticulado de Gauss
 *   n=4  ISA         o passo da máquina ⟼ (registo, slot, banco, andar)
 *   n=k  ULTRAMÉTRICA  o endereço na árvore do encaixe — e aí a distância que
 *                      a fibra respeita é a ULTRAMÉTRICA, não a euclidiana
 *
 * O campo G é ESPARSO e mora num arena fixo: a memória estigmérgica vive onde a
 * trajectória passou, não sobre a grade toda. É a cláusula 3 lida à letra — e é
 * também a regra da casa sobre não pedir memória em execução.
 *
 *   §AN1  o campo incremental bate a RECONTAGEM, em n = 1,2,3,4
 *   §AN2  ∑ G = |I| em todas as dimensões (cláusula 3: nada se perde)
 *   §AN3  o DRAGÃO pelos BITS == o DRAGÃO pela DOBRA D_{k+1} = D_k + D_k*
 *   §AN4  a DOBRA existe em ℤ² e em ℤ³ e NÃO na recta nem no Cantor (controlos)
 *   §AN5  o LEVANTAMENTO π̃(i) = (π(i), k(i)) é injectivo: G̃ ≡ 1, em todo n
 *   §AN6  a ULTRAMÉTRICA: as BOLAS particionam e todo ponto é centro — a
 *         euclidiana falha as duas, e a célula de π É a bola
 *   §AN7  a vizinhança é o único sítio onde o n entra: |V(x)| = 2n
 *   §AN8  o JULIA: |{x : G(x) > 1}| = o PERÍODO da órbita, e G = 1 é a cauda
 *   §AN9  a ARANHA INVERSA em ℤⁿ: π̃ sobe UM andar, e a VOLTA devolve G
 *   §AN10 quem lê o ESPAÇO e quem lê o ÍNDICE: a aranha e o dragão são duais
 *   §AN11 a SÉRIE DE π: o campo lê o custo, e o valor sai por três caminhos
 *   §AN13 o CICLO: escrita · leitura · decisão · fecho, em ℤⁿ — cada passo
 *         É uma cláusula, e o gradiente não delibera: lê o mínimo do chão
 *   §AN12 a TRANSFORMADA ALGÉBRICA: a ida é Dirac, a dobra é a NORMA, e a
 *         convolução/deconvolução compõem e decompõem campos (e onde degenera)
 *
 *   cc -O2 -std=c99 -w -I../lib -o aranha_n aranha_n.c && ./aranha_n
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

#define AN_N     8          /* a torre vai a oito */
#define AN_CAP   16384      /* células distintas que o arena segura */
#define AN_IMAX  8192       /* comprimento máximo da trajectória */

typedef struct { int c[AN_N]; } Vet;

/* ── O CAMPO G, ESPARSO ───────────────────────────────────────────────────────
 * Endereçamento aberto num arena fixo: sem malloc, e a memória vive onde a
 * trajectória passou. A chave é o VECTOR — o n não aparece na estrutura, só no
 * comprimento que se compara. */
static Vet an_chave[AN_CAP];
static long an_G[AN_CAP];
static int  an_vivo[AN_CAP];
static long an_ocupadas;
static int  an_dim;

static unsigned an_hash(Vet v){
    unsigned h = 2166136261u;
    for(int i = 0; i < an_dim; i++){
        unsigned c = (unsigned)(v.c[i] + 32768);
        h = (h ^ (c & 255u)) * 16777619u;
        h = (h ^ ((c >> 8) & 255u)) * 16777619u;
    }
    return h;
}
static int an_igual(Vet a, Vet b){
    for(int i = 0; i < an_dim; i++) if(a.c[i] != b.c[i]) return 0;
    return 1;
}
static void an_zera(int n){
    an_dim = n; an_ocupadas = 0;
    for(long i = 0; i < AN_CAP; i++){ an_vivo[i] = 0; an_G[i] = 0; }
}
static long an_slot(Vet v){
    unsigned h = an_hash(v) % AN_CAP;
    for(long p = 0; p < AN_CAP; p++){
        long i = (long)((h + (unsigned)p) % AN_CAP);
        if(!an_vivo[i] || an_igual(an_chave[i], v)) return i;
    }
    return -1;                                    /* arena cheio: diz-se */
}
/* cláusula 3: a ESCRITA incremental, G_{t+1}(x) = G_t(x) + 1 */
static int an_visita(Vet v){
    long i = an_slot(v);
    if(i < 0) return 0;
    if(!an_vivo[i]){ an_vivo[i] = 1; an_chave[i] = v; an_ocupadas++; }
    an_G[i]++;
    return 1;
}
/* cláusula 4: SENTIR É LER G */
static long an_le(Vet v){
    long i = an_slot(v);
    return (i < 0 || !an_vivo[i]) ? 0 : an_G[i];
}

/* ── AS REALIZAÇÕES — cada uma devolve a trajectória π(0..N) ─────────────────*/
static Vet traj[AN_IMAX];

static Vet vet0(void){ Vet v; for(int i = 0; i < AN_N; i++) v.c[i] = 0; return v; }

/* n=2 — HEIGHWAY, exactamente como o aranha_g.c o gera */
static int drag_esq(long k){ return (((k & -k) << 1) & k) != 0; }
static long real_dragao(long n){
    int x = 0, y = 0, dx = 1, dy = 0;
    long t = 0;
    Vet v = vet0(); v.c[0] = 0; v.c[1] = 0; traj[t++] = v;
    for(long s = 0; s < n && t < AN_IMAX; s++){
        x += dx; y += dy;
        v = vet0(); v.c[0] = x; v.c[1] = y; traj[t++] = v;
        if(drag_esq(s + 1)){ int a = -dy, b = dx; dx = a; dy = b; }
        else               { int a =  dy, b = -dx; dx = a; dy = b; }
    }
    return t;
}

/* O DRAGÃO PELA DOBRA — o segundo caminho, e é o passo da TORRE:
 *
 *     D_{k+1} = D_k + D_k*        (torre_alg.h: T_{k+1} = T_k + T_k*)
 *
 * onde o dual `*` é a curva percorrida AO CONTRÁRIO e rodada de 90° em torno do
 * ponto final. É a dobra do papel: dobrar a tira ao meio é colar a ela a sua
 * própria imagem invertida. Este gerador não sabe o que é `drag_esq` — não há
 * uma linha em comum com a régua de bits, e é isso que faz dele um caminho.
 * O sinal da rotação NÃO se escolhe aqui: o medidor corre os dois. */
static int dg_x[2][AN_IMAX], dg_y[2][AN_IMAX];
static long dragao_dobra(int k, int sentido, int *ox, int *oy){
    int cur = 0;
    long n = 1;
    dg_x[0][0] = 0; dg_y[0][0] = 0; dg_x[0][1] = 1; dg_y[0][1] = 0;
    for(int it = 0; it < k && 2*n < AN_IMAX; it++){
        int nx = 1 - cur;
        long N = n;
        for(long i = 0; i <= N; i++){ dg_x[nx][i] = dg_x[cur][i]; dg_y[nx][i] = dg_y[cur][i]; }
        int fx = dg_x[cur][N], fy = dg_y[cur][N];
        for(long j = 1; j <= N; j++){                    /* o DUAL: reverso + rodado */
            int vx = fx - dg_x[cur][N-j], vy = fy - dg_y[cur][N-j], rx, ry;
            if(sentido > 0){ rx = -vy; ry =  vx; } else { rx =  vy; ry = -vx; }
            dg_x[nx][N+j] = fx + rx; dg_y[nx][N+j] = fy + ry;
        }
        n = 2*N; cur = nx;
    }
    for(long i = 0; i <= n; i++){ ox[i] = dg_x[cur][i]; oy[i] = dg_y[cur][i]; }
    return n;
}

/* n=3 — O DRAGÃO NO ESPAÇO. A regra de viragem é a de Heighway, mas o PLANO da
 * viragem roda de eixo a cada passo. É preciso dizer o que isto é e o que NÃO é:
 * é UMA realização em ℤ³ da regra de dobra, e não «o mesmo dragão com mais espaço».
 * Medido, dobra MAIS que o do plano (max G = 3 contra 2) — eu tinha escrito o
 * contrário antes de medir, e a diferença é da CURVA, não da dimensão. */
static long real_dragao3(long n){
    int p[3] = {0,0,0}, d[3] = {1,0,0};
    long t = 0;
    Vet v = vet0(); traj[t++] = v;
    for(long s = 0; s < n && t < AN_IMAX; s++){
        for(int i = 0; i < 3; i++) p[i] += d[i];
        v = vet0(); v.c[0] = p[0]; v.c[1] = p[1]; v.c[2] = p[2]; traj[t++] = v;
        int eixo = (int)(s % 3);                   /* o plano da viragem roda */
        int a = (eixo + 1) % 3, b = (eixo + 2) % 3;
        int na, nb;
        if(drag_esq(s + 1)){ na = -d[b]; nb =  d[a]; }
        else               { na =  d[b]; nb = -d[a]; }
        d[a] = na; d[b] = nb;
    }
    return t;
}

/* n=1 — CANTOR: o endereço do terço. Em cada passo desce-se um nível e escolhe-se
 * o terço da esquerda ou da direita; a célula é a posição em unidades de 3^-k
 * levada a inteiro. O terço do MEIO nunca se visita, e é isso que faz o pó. */
static long real_cantor(int niveis){
    long t = 0;
    long total = 1L << niveis;                     /* 2^niveis folhas */
    for(long f = 0; f < total && t < AN_IMAX; f++){
        long pos = 0, esc = 1;
        for(int k = niveis - 1; k >= 0; k--){
            esc *= 3;
            if((f >> k) & 1) pos = pos*3 + 2; else pos = pos*3;
        }
        (void)esc;
        Vet v = vet0(); v.c[0] = (int)(pos % 20000);
        traj[t++] = v;
    }
    return t;
}

/* n=2 — JULIA no reticulado de Gauss: z ↦ z² + c, tudo em inteiros. A órbita que
 * escapa sai do arena; a que não escapa DOBRA (entra em ciclo), e é aí que o G
 * conta a fibra. */
static long real_julia(int cx, int cy, int passos, int x0, int y0){
    long t = 0;
    int x = x0, y = y0;
    Vet v = vet0(); v.c[0] = x; v.c[1] = y; traj[t++] = v;
    for(int s = 0; s < passos && t < AN_IMAX; s++){
        int nx = x*x - y*y + cx, ny = 2*x*y + cy;
        if(nx > 3000 || nx < -3000 || ny > 3000 || ny < -3000) break;   /* escapou */
        x = nx; y = ny;
        v = vet0(); v.c[0] = x; v.c[1] = y; traj[t++] = v;
    }
    return t;
}

/* n=4 — A ISA: o passo da máquina realiza-se em (registo, slot, banco, andar).
 * A aranha não sabe que é uma máquina: lê e escreve o mesmo G. */
static long real_isa(long passos){
    long t = 0;
    int reg = 0, slot = 0, banco = 0, andar = 0;
    for(long s = 0; s < passos && t < AN_IMAX; s++){
        reg   = (int)((s * 3) % 4);
        slot  = (int)((s * 5) % 8);
        banco = (int)((s / 8) % 2);
        andar = (int)((s / 32) % 3);
        Vet v = vet0();
        v.c[0] = reg; v.c[1] = slot; v.c[2] = banco; v.c[3] = andar;
        traj[t++] = v;
    }
    return t;
}

/* a RECTA — o controlo injectivo: G ≡ 1 por construção */
static long real_recta(long n, int dim){
    long t = 0;
    for(long s = 0; s <= n && t < AN_IMAX; s++){
        Vet v = vet0();
        v.c[0] = (int)s;
        if(dim > 1) v.c[1] = (int)s;               /* diagonal: continua injectiva */
        traj[t++] = v;
    }
    return t;
}

/* ── AS CLÁUSULAS, medidas em qualquer n ─────────────────────────────────────*/

/* (1)(2) o campo incremental bate a RECONTAGEM da fibra, sobre um percurso dado */
static int an_bate_recontagem_em(const Vet *p, long nt, long *max_g){
    long mg = 0;
    for(long i = 0; i < AN_CAP; i++){
        if(!an_vivo[i]) continue;
        long c = 0;
        for(long t = 0; t < nt; t++) if(an_igual(p[t], an_chave[i])) c++;
        if(c != an_G[i]) return 0;
        if(an_G[i] > mg) mg = an_G[i];
    }
    if(max_g) *max_g = mg;
    return 1;
}
/* (1)(2) o campo incremental bate a RECONTAGEM da fibra */
static int an_bate_recontagem(long nt, long *max_g){
    long mg = 0;
    for(long i = 0; i < AN_CAP; i++){
        if(!an_vivo[i]) continue;
        long c = 0;
        for(long t = 0; t < nt; t++) if(an_igual(traj[t], an_chave[i])) c++;
        if(c != an_G[i]) return 0;
        if(an_G[i] > mg) mg = an_G[i];
    }
    if(max_g) *max_g = mg;
    return 1;
}
/* (3) ∑ G = |I| */
static long an_soma(void){
    long s = 0;
    for(long i = 0; i < AN_CAP; i++) if(an_vivo[i]) s += an_G[i];
    return s;
}
/* quantas células têm G > 1 — as que a trajectória visitou mais de uma vez */
static long an_dobradas(void){
    long d = 0;
    for(long i = 0; i < AN_CAP; i++) if(an_vivo[i] && an_G[i] > 1) d++;
    return d;
}
/* O SEGUNDO CAMINHO para o período: busca directa do primeiro par repetido na
 * trajectória. Não olha para o campo G — é a outra régua, e é ela que dá
 * conteúdo à leitura estigmérgica do §AN8. Devolve 0 se a órbita não voltar. */
static long periodo_busca(long nt, long *cauda){
    for(long i = 0; i < nt; i++)
        for(long j = i + 1; j < nt; j++)
            if(an_igual(traj[i], traj[j])){ if(cauda) *cauda = i; return j - i; }
    if(cauda) *cauda = nt;
    return 0;
}

/* o LEVANTAMENTO: k(i) = o número da visita, e π̃ = (π(i), k(i)) é injectivo */
static int an_levanta_injectivo(long nt){
    static long k_de[AN_IMAX];
    an_zera(an_dim);
    for(long t = 0; t < nt; t++){
        if(!an_visita(traj[t])) return 0;
        k_de[t] = an_le(traj[t]);                  /* o número da visita ATÉ aqui */
    }
    for(long i = 0; i < nt; i++)
        for(long j = i + 1; j < nt; j++)
            if(k_de[i] == k_de[j] && an_igual(traj[i], traj[j])) return 0;
    return 1;
}


/* ── A SÉRIE DE PI, em inteiros: arctan(1/n) por soma alternada ──────────────
 * Nenhum dígito escrito à mão. A escala é 10^8 para o valor caber num átomo do
 * vector (int), e o critério de paragem é o termo ir a ZERO na divisão inteira
 * — que é o que "convergir" quer dizer quando não há vírgula. */
static long arctan_inv(long n, long S, long *termos){
    long termo = S/n, soma = termo, n2 = n*n, p = 1;
    for(long k = 1; k < 200; k++){
        termo /= n2;
        long t = termo/(2*k + 1);
        soma = (k & 1) ? soma - t : soma + t;
        p++;
        if(t == 0 && termo == 0) break;
    }
    if(termos) *termos = p;
    return soma;
}
/* a TRAJECTÓRIA das somas parciais: π(t) = S_t. O passo continua depois de o
 * termo ir a zero, e é aí que a trajectória entra no seu PONTO FIXO. */
static long real_serie(long n, long S, long passos){
    long termo = S/n, soma = termo, n2 = n*n, t = 0;
    Vet v = vet0(); v.c[0] = (int)soma; traj[t++] = v;
    for(long k = 1; k <= passos && t < AN_IMAX; k++){
        termo /= n2;
        long x = termo/(2*k + 1);
        soma = (k & 1) ? soma - x : soma + x;
        v = vet0(); v.c[0] = (int)soma; traj[t++] = v;
    }
    return t;
}

/* ── O RELÓGIO: a meia-volta involutiva M² = id, e o rotor de período 4 ──────
 * A rotação J: (a,b) ↦ (−b,a). É determinista por construção — o passo lê o
 * ESTADO e mais nada — e é o relógio que a casa já corria. */
static long real_relogio(long a0, long b0, long passos){
    long t = 0; int a = (int)a0, b = (int)b0;
    Vet v = vet0(); v.c[0] = a; v.c[1] = b; traj[t++] = v;
    for(long s = 0; s < passos && t < AN_IMAX; s++){
        int na = -b, nb = a;  a = na; b = nb;
        v = vet0(); v.c[0] = a; v.c[1] = b; traj[t++] = v;
    }
    return t;
}

/* O PASSO É FUNÇÃO DO ESTADO? Procura na trajectória DUAS ocorrências da mesma
 * célula com sucessores DIFERENTES. Devolve 1 se achar (logo NÃO determinista)
 * e escreve a testemunha. Não é uma opinião sobre a construção: é uma busca. */
static int nao_determinista(long nt, long *oi, long *oj){
    for(long i = 0; i + 1 < nt; i++)
        for(long j = i + 1; j + 1 < nt; j++)
            if(an_igual(traj[i], traj[j]) && !an_igual(traj[i+1], traj[j+1])){
                if(oi) *oi = i; if(oj) *oj = j; return 1;
            }
    return 0;
}


/* ── A TRANSFORMADA ALGÉBRICA: a mesma do `transformada.c` §U1--§U5 ──────────
 * O grupo é (ℤ/2)^m e o caractere é χ_k(j) = (−1)^(bits comuns) — valores ±1,
 * logo a transformada é INTEIRA e não há um único float. Sem normalizar, como
 * a casa a usa: ‖Fx‖² = n‖x‖² (§U3), e a volta divide por n.
 *
 * É este o grupo em que a aranha já corria sem saber: o endereço de um índice
 * na árvore do encaixe (§AN6) É um elemento de (ℤ/2)^m, e a convolução do
 * grupo é o XOR. */
#define TA_M   8
#define TA_N   (1 << TA_M)

static int ta_chi(long k, long j){
    long b = k & j, p = 0;
    while(b){ p ^= (b & 1); b >>= 1; }
    return p ? -1 : 1;
}
static void ta_F(const long *x, long *X){
    for(long k = 0; k < TA_N; k++){
        long s = 0;
        for(long j = 0; j < TA_N; j++) s += x[j] * ta_chi(k, j);
        X[k] = s;
    }
}
/* a VOLTA: x_j = (1/n) Σ_k X_k χ_k(j). Devolve 0 se alguma divisão não for exacta
 * — a transformada é inteira, e uma divisão com resto seria a saída do anel. */
static int ta_Finv(const long *X, long *x){
    for(long j = 0; j < TA_N; j++){
        long s = 0;
        for(long k = 0; k < TA_N; k++) s += X[k] * ta_chi(k, j);
        if(s % TA_N) return 0;
        x[j] = s / TA_N;
    }
    return 1;
}
/* a CONVOLUÇÃO do grupo: (f*g)(y) = Σ_x f(x)·g(x⊕y). Custo n², e é o que a
 * transformada evita. */
static void ta_conv(const long *f, const long *g, long *h){
    for(long y = 0; y < TA_N; y++){
        long s = 0;
        for(long x = 0; x < TA_N; x++) s += f[x] * g[x ^ y];
        h[y] = s;
    }
}
/* o campo G de um percurso, como vector sobre o grupo */
static void ta_campo(const long *p, long nt, long *g){
    for(long i = 0; i < TA_N; i++) g[i] = 0;
    for(long t = 0; t < nt; t++) g[p[t] & (TA_N - 1)]++;
}

/* corre uma realização e devolve o essencial */
typedef struct { long nt, celulas, soma, max_g, dobradas; int recontou; } Res;
static Res an_corre(int n, long nt){
    Res r;
    an_zera(n);
    for(long t = 0; t < nt; t++) an_visita(traj[t]);
    r.nt = nt; r.celulas = an_ocupadas; r.soma = an_soma();
    r.recontou = an_bate_recontagem(nt, &r.max_g);
    r.dobradas = an_dobradas();
    return r;
}

int main(void){
    printf("\n=== A ARANHA EM ℤⁿ — o teorema não usa o 2 ==========================\n");

    /* ═══ §AN1 e §AN2: o campo bate a recontagem, e ∑G = |I|, em quatro dimensões */
    printf("\n§AN1/§AN2  o campo incremental bate a recontagem, e ∑G = |I|.\n\n");
    {
        long mau = 0, dims = 0;
        printf("      realização        n   |I|    células   ∑G     max G  recontou\n");
        struct { const char *nome; int n; long nt; } casos[6];
        int nc = 0;
        casos[nc].nome = "Cantor";        casos[nc].n = 1; casos[nc].nt = real_cantor(9);      nc++;
        Res r0 = an_corre(1, casos[0].nt);
        casos[nc].nome = "dragão (ℤ²)";   casos[nc].n = 2; casos[nc].nt = real_dragao(2048);   nc++;
        Res r1 = an_corre(2, casos[1].nt);
        casos[nc].nome = "dragão espaço"; casos[nc].n = 3; casos[nc].nt = real_dragao3(2048);  nc++;
        Res r2 = an_corre(3, casos[2].nt);
        casos[nc].nome = "ISA (ℤ⁴)";      casos[nc].n = 4; casos[nc].nt = real_isa(512);       nc++;
        Res r3 = an_corre(4, casos[3].nt);
        Res R[4]; R[0] = r0; R[1] = r1; R[2] = r2; R[3] = r3;
        for(int k = 0; k < 4; k++){
            printf("      %-17s %d   %-6ld %-9ld %-6ld %-6ld %s\n",
                   casos[k].nome, casos[k].n, R[k].nt, R[k].celulas, R[k].soma,
                   R[k].max_g, R[k].recontou ? "sim" : "NÃO");
            if(!R[k].recontou) mau++;
            if(R[k].soma != R[k].nt) mau++;        /* ∑G = |I| */
            dims++;
        }
        printf("\n");
        ok("O CAMPO É O MESMO EM QUALQUER DIMENSÃO, e é isso que se afirma: o mesmo"
           " código corre em ℤ¹, ℤ², ℤ³ e ℤ⁴ — Cantor, o dragão, o dragão no espaço e"
           " a ISA — e nas quatro o campo incremental G_{t+1} = G_t + 1 bate a"
           " RECONTAGEM da fibra |π⁻¹(x)| e ∑G = |I|. O `thm:multiplicidade` está"
           " escrito para ℤ² e nenhuma das suas quatro cláusulas menciona a dimensão:"
           " duplicidade é a fibra, G conta a fibra, a escrita é local, e sentir é ler."
           " O 2 era do exemplo. E o campo é ESPARSO: a memória vive onde a trajectória"
           " passou, que é a cláusula 3 lida à letra — em ℤ³ o array do aranha_g.c"
           " pediria 34 GB",
           mau == 0 && dims == 4);
    }

    /* ═══ §AN3: o dragão pelos BITS contra o dragão pela DOBRA ══════════════ */
    printf("\n§AN3  o dragão por dois caminhos: a régua de bits e a DOBRA.\n\n");
    {
        static int bx[AN_IMAX], by[AN_IMAX], dx_[AN_IMAX], dy_[AN_IMAX];
        long mau = 0, batem_pos = 0, batem_neg = 0, ordens = 0;
        printf("      k    passos   dobra(+90°)   dobra(−90°)\n");
        for(int k = 1; k <= 12; k++){
            long n = 1L << k;
            long nt = real_dragao(n);                 /* caminho 1: a régua de bits */
            for(long i = 0; i < nt; i++){ bx[i] = traj[i].c[0]; by[i] = traj[i].c[1]; }
            int igual[2];
            for(int sg = 0; sg < 2; sg++){            /* caminho 2: D_k + D_k* */
                long m = dragao_dobra(k, sg ? 1 : -1, dx_, dy_);
                igual[sg] = (m == n);
                if(igual[sg])
                    for(long i = 0; i <= m; i++)
                        if(dx_[i] != bx[i] || dy_[i] != by[i]){ igual[sg] = 0; break; }
            }
            printf("      %-4d %-8ld %-13s %s\n", k, n,
                   igual[1] ? "igual" : "difere", igual[0] ? "igual" : "difere");
            if(igual[1]) batem_pos++;
            if(igual[0]) batem_neg++;
            if(igual[0] == igual[1]) mau++;           /* uma e só uma tem de bater */
            ordens++;
        }
        /* e o campo G sobre a curva, para amarrar ao aranha_g.c */
        long nt = real_dragao(4096);
        Res r = an_corre(2, nt);
        printf("\n      campo G em 4096 passos: |I| = %ld, células = %ld, ∑G = %ld, max G = %ld\n\n",
               r.nt, r.celulas, r.soma, r.max_g);
        if(r.soma != r.nt || r.max_g < 2 || !r.recontou) mau++;
        ok("O DRAGÃO SAI POR DOIS CAMINHOS SEM UMA LINHA EM COMUM: a régua de bits"
           " `drag_esq` — a mesma do `aranha_g.c` — e a DOBRA D_{k+1} = D_k + D_k*, com"
           " o dual a ser a curva ao contrário rodada de 90° em torno do ponto final."
           " Coincidem ponto a ponto em k = 1..12, e o sinal da rotação não foi escolhido"
           " por mim: o medidor corre os dois e exige que EXACTAMENTE UM bata, o MESMO"
           " em todas as ordens — se batessem os dois, ou nenhum, a coincidência não"
           " dizia nada. Isto é o passo da torre `T_{k+1} = T_k + T_k*` a aparecer numa"
           " curva: dobrar a tira ao meio é colar-lhe a sua própria imagem invertida."
           " O que a versão anterior desta asserção comparava — |I| e ∑G — passava com"
           " QUALQUER trajectória do mesmo comprimento, e o gume mostrou-o: uma espiral"
           " sobrevivia-lhe",
           mau == 0 && ordens == 12 && (batem_pos == 12 || batem_neg == 12)
                    && batem_pos + batem_neg == 12);
    }

    /* ═══ §AN4: a dobra existe em ℤ² e ℤ³, e NÃO na recta ════════════════════ */
    printf("\n§AN4  a DOBRA e o seu controlo: onde G > 1 e onde G ≡ 1.\n\n");
    {
        long nt2 = real_dragao(2048);   Res d2 = an_corre(2, nt2);
        long nt3 = real_dragao3(2048);  Res d3 = an_corre(3, nt3);
        long ntr = real_recta(2048, 2); Res rr = an_corre(2, ntr);
        long ntc = real_cantor(9);      Res cc = an_corre(1, ntc);
        printf("      dragão ℤ²      max G = %ld   (dobra)\n", d2.max_g);
        printf("      dragão ℤ³      max G = %ld   (dobra — e MAIS, não menos)\n", d3.max_g);
        printf("      Cantor ℤ¹      max G = %ld   (o pó: endereços distintos)\n", cc.max_g);
        printf("      recta          max G = %ld   (controlo injectivo)\n\n", rr.max_g);
        ok("A DOBRA MEDE-SE E O CONTROLO TAMBÉM: o dragão dobra em ℤ² e em ℤ³ (max G > 1)"
           " e a recta NÃO (max G = 1) — sem o controlo, «G > 1» passaria com um campo que"
           " contasse mal. E o CANTOR também não dobra (max G = 1): o pó são endereços"
           " distintos, e é por isso que ele tem medida nula sem se auto-intersectar."
           " O QUE NÃO SE AFIRMA, e eu tinha escrito ANTES de medir: que dobrasse MENOS"
           " em ℤ³ «porque há mais onde virar». Medido, dobra MAIS — 3 contra 2 —, e a"
           " razão não é a dimensão: a curva em ℤ³ é OUTRA, com o plano da viragem a"
           " rodar de eixo a cada passo, e o max G compara as duas CURVAS e não os dois"
           " espaços. Uma propriedade da construção que eu inventei não é uma lei do andar",
           d2.max_g > 1 && d3.max_g > 1 && rr.max_g == 1 && cc.max_g == 1);
    }

    /* ═══ §AN5: o levantamento é injectivo em todo n ═════════════════════════ */
    printf("\n§AN5  o LEVANTAMENTO π̃(i) = (π(i), k(i)): G̃ ≡ 1 em qualquer n.\n\n");
    {
        long mau = 0;
        struct { const char *nome; int n; long nt; } cs[4];
        cs[0].nome = "Cantor ℤ¹";     cs[0].n = 1; cs[0].nt = real_cantor(8);
        int i0 = an_levanta_injectivo(cs[0].nt); an_dim = 1;
        an_zera(1); cs[0].nt = real_cantor(8); an_dim = 1; i0 = an_levanta_injectivo(cs[0].nt);
        an_dim = 2; cs[1].nome = "dragão ℤ²";  cs[1].n = 2; cs[1].nt = real_dragao(1024);
        int i1 = an_levanta_injectivo(cs[1].nt);
        an_dim = 3; cs[2].nome = "dragão ℤ³";  cs[2].n = 3; cs[2].nt = real_dragao3(1024);
        int i2 = an_levanta_injectivo(cs[2].nt);
        an_dim = 4; cs[3].nome = "ISA ℤ⁴";     cs[3].n = 4; cs[3].nt = real_isa(512);
        int i3 = an_levanta_injectivo(cs[3].nt);
        printf("      %-14s injectivo: %s\n", cs[0].nome, i0 ? "sim" : "NÃO");
        printf("      %-14s injectivo: %s\n", cs[1].nome, i1 ? "sim" : "NÃO");
        printf("      %-14s injectivo: %s\n", cs[2].nome, i2 ? "sim" : "NÃO");
        printf("      %-14s injectivo: %s\n\n", cs[3].nome, i3 ? "sim" : "NÃO");
        if(!(i0 && i1 && i2 && i3)) mau++;
        ok("O LEVANTAMENTO DO `thm:aranha-inversa` TAMBÉM NÃO USA A DIMENSÃO:"
           " π̃(i) = (π(i), k(i)) com k = o número da visita é injectivo em ℤ¹, ℤ², ℤ³ e"
           " ℤ⁴ — logo G̃ ≡ 1 em todos, e a fibra que a projecção perdia volta como"
           " coordenada de folha. É o mesmo desfazer da identificação i∼j que o dragão"
           " fazia na grade, agora em qualquer grade",
           mau == 0);
    }

    /* ═══ §AN6: a ULTRAMÉTRICA — as BOLAS são as células ═════════════════════ */
    printf("\n§AN6  a ULTRAMÉTRICA: as bolas PARTICIONAM, e é isso que o campo G lê.\n\n");
    {
        /* O endereço de um índice é o seu caminho de bits, e a distância é
         * 2^{-profundidade da primeira divergência} — a métrica que o CORTE induz
         * nos reais (reais.tex): dois cortes estão perto quando CONCORDAM até
         * fundo. A propriedade que a separa da euclidiana não é a desigualdade
         * escrita: é que as bolas PARTICIONAM. Duas bolas do mesmo raio ou
         * coincidem ou são disjuntas, e todo o ponto de uma bola é seu centro.
         * E é por isso que a aranha corre aqui sem mudar uma linha: a célula de
         * π É a bola, e G conta quem lá caiu. */
        const int NB = 10, PROF = 4;             /* 1024 índices, bolas de raio 2^-4 */
        const long N = 1L << NB, RAIO = 1L << (NB - PROF);
        long mau = 0;

        /* a aranha sobre a ÁRVORE: π(i) = o prefixo de PROF bits = a bola de i */
        long nt = 0;
        for(long i = 0; i < N && nt < AN_IMAX; i++){
            Vet v = vet0(); v.c[0] = (int)(i >> (NB - PROF)); traj[nt++] = v;
        }
        Res ra = an_corre(1, nt);
        long bolas = 1L << PROF, dentro = N / bolas;
        printf("      a aranha na árvore: |I| = %ld, células = %ld, ∑G = %ld, cada G = %ld\n",
               ra.nt, ra.celulas, ra.soma, ra.max_g);
        if(ra.celulas != bolas || ra.soma != ra.nt || ra.max_g != dentro) mau++;
        if(!ra.recontou) mau++;

        /* AS BOLAS PARTICIONAM — ou iguais ou disjuntas, e nunca a cruzarem-se.
         * O controlo é a MESMA varredura com |a−b| ≤ r na recta, mesmo raio. */
        long ultra_cruza = 0, eucl_cruza = 0, ultra_pares = 0;
        for(long a = 0; a < N; a += 8)
            for(long b = 0; b < N; b += 8){
                /* bola ultramétrica de a = os que concordam nos PROF bits altos */
                int mesma = ((a >> (NB - PROF)) == (b >> (NB - PROF)));
                /* cruzam-se parcialmente? existe x em ambas, e alguém só numa */
                int ambos = 0, so_a = 0, so_b = 0;
                for(long x = 0; x < N; x += 8){
                    int ea = ((x >> (NB - PROF)) == (a >> (NB - PROF)));
                    int eb = ((x >> (NB - PROF)) == (b >> (NB - PROF)));
                    if(ea && eb) ambos = 1; else if(ea) so_a = 1; else if(eb) so_b = 1;
                }
                if(ambos && so_a && so_b) ultra_cruza++;
                (void)mesma; ultra_pares++;
                /* e a euclidiana, com o mesmo raio */
                int eambos = 0, eso_a = 0, eso_b = 0;
                for(long x = 0; x < N; x += 8){
                    long da = x > a ? x - a : a - x, db = x > b ? x - b : b - x;
                    int ea = (da <= RAIO/2), eb = (db <= RAIO/2);
                    if(ea && eb) eambos = 1; else if(ea) eso_a = 1; else if(eb) eso_b = 1;
                }
                if(eambos && eso_a && eso_b) eucl_cruza++;
            }
        printf("      bolas que se CRUZAM sem conter (%ld pares):  árvore %ld · recta %ld\n",
               ultra_pares, ultra_cruza, eucl_cruza);
        if(ultra_cruza != 0 || eucl_cruza == 0) mau++;

        /* TODO PONTO É CENTRO: b ∈ B(a,r) ⟹ B(b,r) = B(a,r). Na recta, falso. */
        long centro_falha = 0, ecentro_falha = 0;
        for(long a = 0; a < N; a += 8)
            for(long b = 0; b < N; b += 8){
                if((a >> (NB - PROF)) == (b >> (NB - PROF)))
                    for(long x = 0; x < N; x += 8)
                        if((((x >> (NB-PROF)) == (a >> (NB-PROF)))) !=
                           (((x >> (NB-PROF)) == (b >> (NB-PROF))))) centro_falha++;
                long dab = a > b ? a - b : b - a;
                if(dab <= RAIO/2)
                    for(long x = 0; x < N; x += 8){
                        long da = x > a ? x - a : a - x, db = x > b ? x - b : b - x;
                        if((da <= RAIO/2) != (db <= RAIO/2)) ecentro_falha++;
                    }
            }
        printf("      «todo ponto é centro» falha em:              árvore %ld · recta %ld\n\n",
               centro_falha, ecentro_falha);
        if(centro_falha != 0 || ecentro_falha == 0) mau++;

        ok("A ULTRAMÉTRICA NÃO É UMA DESIGUALDADE MAIS APERTADA: é as BOLAS"
           " PARTICIONAREM, e é por isso que a aranha corre nela sem mudar uma linha."
           " O endereço de um índice é o seu caminho de bits e a distância é"
           " 2^{-profundidade da primeira divergência} — a métrica que o CORTE induz nos"
           " reais, onde dois estão perto por CONCORDAREM até fundo. Medido: as bolas da"
           " árvore nunca se cruzam parcialmente e todo o ponto de uma é seu CENTRO; as"
           " da recta, com o mesmo raio, falham as duas coisas. E o campo G lê exactamente"
           " isso — a célula de π É a bola, ∑G = |I| sem sobreposição porque não há"
           " sobreposição a haver, e cada G é o tamanho da bola. Não se varre a"
           " desigualdade d ≤ max: ela sai em três casos do primeiro bit divergente, e"
           " varrê-la seria substituir a prova pelo número",
           mau == 0);
    }


    /* ═══ §AN7: o n entra num sítio só — a vizinhança ════════════════════════ */
    printf("\n§AN7  onde o n entra: |V(x)| = 2n, e em mais lado nenhum.\n\n");
    {
        long mau = 0;
        printf("      n   |V(x)|  vizinhos distintos  G lido em cada\n");
        for(int n = 1; n <= 4; n++){
            an_zera(n);
            Vet o = vet0();
            an_visita(o);
            long viz = 0, distintos = 0, lidos = 0;
            Vet vs[2*AN_N];
            for(int i = 0; i < n; i++)
                for(int s = -1; s <= 1; s += 2){
                    Vet v = o; v.c[i] += s;
                    vs[viz++] = v;
                    an_visita(v);
                }
            for(long a = 0; a < viz; a++){
                int rep = 0;
                for(long b = 0; b < a; b++) if(an_igual(vs[a], vs[b])) rep = 1;
                if(!rep) distintos++;
                if(an_le(vs[a]) == 1) lidos++;
            }
            printf("      %d   %-7ld %-19ld %ld\n", n, viz, distintos, lidos);
            if(viz != 2*n || distintos != 2*n || lidos != 2*n) mau++;
        }
        printf("\n");
        ok("O n ENTRA NUM SÍTIO SÓ, e é a cláusula 4: a vizinhança V(x) tem 2n vizinhos,"
           " todos distintos, e o G lê-se em cada um. As outras três cláusulas —"
           " duplicidade é a fibra, G conta a fibra, a escrita é local — não sabem em"
           " que dimensão estão, e é por isso que o mesmo código correu ℤ¹ a ℤ⁴ sem uma"
           " linha diferente. Dizer «o teorema generaliza» sem exibir ONDE a dimensão"
           " entra era afirmar sem medir",
           mau == 0);
    }


    /* ═══ §AN8: o JULIA — o campo G lê o PERÍODO da órbita ═══════════════════ */
    printf("\n§AN8  o JULIA em ℤ[i]: a estigmergia lê o período, e separa a cauda.\n\n");
    {
        struct { const char *nome; int cx, cy, x0, y0, passos; } orb[4];
        orb[0].nome = "c=0,  z0=0"; orb[0].cx =  0; orb[0].cy = 0; orb[0].x0 = 0; orb[0].y0 = 0; orb[0].passos = 60;
        orb[1].nome = "c=-1, z0=0"; orb[1].cx = -1; orb[1].cy = 0; orb[1].x0 = 0; orb[1].y0 = 0; orb[1].passos = 60;
        orb[2].nome = "c=i,  z0=0"; orb[2].cx =  0; orb[2].cy = 1; orb[2].x0 = 0; orb[2].y0 = 0; orb[2].passos = 60;
        orb[3].nome = "c=1,  z0=1"; orb[3].cx =  1; orb[3].cy = 0; orb[3].x0 = 1; orb[3].y0 = 0; orb[3].passos = 60;
        long mau = 0, presos = 0, escapou = 0;
        printf("      órbita       |I|   células  max G  G>1  período  cauda  G>1 = período\n");
        for(int k = 0; k < 4; k++){
            long nt = real_julia(orb[k].cx, orb[k].cy, orb[k].passos, orb[k].x0, orb[k].y0);
            Res r = an_corre(2, nt);
            long cauda = 0, per = periodo_busca(nt, &cauda);
            int bate = (r.dobradas == per);
            /* a cauda pela ESTIGMERGIA: as células com G == 1 */
            long cauda_g = r.celulas - r.dobradas;
            int bate_cauda = (per == 0) ? (cauda_g == r.celulas) : (cauda_g == cauda);
            printf("      %-12s %-5ld %-8ld %-6ld %-4ld %-8ld %-6ld %s\n",
                   orb[k].nome, r.nt, r.celulas, r.max_g, r.dobradas, per, cauda,
                   (bate && bate_cauda) ? "sim" : "NÃO");
            if(!bate || !bate_cauda || !r.recontou || r.soma != r.nt) mau++;
            if(per > 0) presos++; else escapou++;
        }
        /* A HIPÓTESE É NECESSÁRIA, e mede-se retirando-a: se a trajectória parar
         * depois de entrar no ciclo mas antes de o fechar duas vezes, as células
         * do ciclo ainda visitadas uma só vez contam para a cauda. Exige-se
         * N ≥ μ + 2p − 1; aqui μ = 0 e p = 2, logo N ≥ 3. */
        long curto_dobradas = -1, curto_per = -1, longo_dobradas = -1, longo_per = -1;
        {
            long nt = real_julia(-1, 0, 2, 0, 0);        /* N = 2: falta UM passo */
            Res r = an_corre(2, nt);
            curto_dobradas = r.dobradas; curto_per = periodo_busca(nt, 0);
        }
        {
            long nt = real_julia(-1, 0, 3, 0, 0);        /* N = 3: a hipótese cumpre-se */
            Res r = an_corre(2, nt);
            longo_dobradas = r.dobradas; longo_per = periodo_busca(nt, 0);
        }
        printf("      a hipótese N ≥ μ+2p−1 (μ=0, p=2, logo N≥3):\n");
        printf("        N=2  período %ld  mas |{G>1}| = %ld   ← a lei FALHA sem ela\n",
               curto_per, curto_dobradas);
        printf("        N=3  período %ld  e   |{G>1}| = %ld   ← e volta a valer\n",
               longo_per, longo_dobradas);
        if(!(curto_per == 2 && curto_dobradas != curto_per)) mau++;
        if(!(longo_per == 2 && longo_dobradas == longo_per)) mau++;
        printf("\n");
        ok("O CAMPO G LÊ O PERÍODO, e é a cláusula 3 a pagar: numa órbita DETERMINISTA"
           " — z ↦ z² + c no reticulado de Gauss, tudo inteiro — a célula É o estado,"
           " logo voltar a uma célula é voltar ao mesmo estado e a órbita fecha. O número"
           " de células com G > 1 é EXACTAMENTE o período, e as de G = 1 são exactamente"
           " a cauda: o ρ de Floyd lido no chão. E não é a mesma conta duas vezes — o"
           " período do lado direito vem de uma BUSCA do primeiro par repetido na"
           " trajectória, que nunca olha para G. O controlo é a órbita que ESCAPA"
           " (c=1, z0=1): período 0, nenhuma célula com G > 1, e a cauda é tudo."
           " Isto é o `thm:multiplicidade` a dar de graça o que o algoritmo clássico"
           " resolve com DUAS patas, lebre e tartaruga: a aranha tem uma só, porque a"
           " memória não é dela — é do espaço. E A HIPÓTESE N ≥ μ+2p−1 NÃO É"
           " DECORAÇÃO: retirada, a lei cai — a mesma órbita c=−1 truncada em N=2"
           " (um passo abaixo do limiar) tem período 2 e UMA só célula com G > 1,"
           " porque parou antes de fechar a segunda volta. Com N=3 volta a valer."
           " O contra-exemplo não pode ser N=1: aí não há repetição nenhuma e a"
           " hipótese do próprio teorema falha, logo não seria contra-exemplo. Uma hipótese que não se exibe"
           " a falhar quando é retirada não estava a fazer trabalho nenhum",
           mau == 0 && presos == 3 && escapou == 1);
    }


    /* ═══ §AN9: a ARANHA INVERSA em ℤⁿ — o levantamento sobe UM andar ════════ */
    printf("\n§AN9  a aranha INVERSA em ℤⁿ: π̃ = (π,k) e a VOLTA.\n\n");
    {
        static Vet lev[AN_IMAX];
        static long k_i[AN_IMAX], g_i[AN_IMAX];
        struct { const char *nome; int n; long nt; } cs[5];
        int nc = 0;
        cs[nc].nome = "Cantor ℤ¹";  cs[nc].n = 1; cs[nc].nt = real_cantor(9);      nc++;
        cs[nc].nome = "dragão ℤ²";  cs[nc].n = 2; cs[nc].nt = real_dragao(2048);   nc++;
        cs[nc].nome = "dragão ℤ³";  cs[nc].n = 3; cs[nc].nt = real_dragao3(2048);  nc++;
        cs[nc].nome = "ISA ℤ⁴";     cs[nc].n = 4; cs[nc].nt = real_isa(512);       nc++;
        cs[nc].nome = "recta (ctl)";cs[nc].n = 2; cs[nc].nt = real_recta(2048, 2); nc++;
        long mau = 0, casos = 0, com_dobra = 0, degenerados = 0;
        printf("      realização   n  n+1  max G base  max G̃  células  pr₁  fibra  volta\n");
        for(int c = 0; c < nc; c++){
            int n = cs[c].n; long nt = cs[c].nt;
            /* NOTA: cada caso regenera a sua trajectória — real_* escreve em traj[] */
            if(c == 0) nt = real_cantor(9);
            else if(c == 1) nt = real_dragao(2048);
            else if(c == 2) nt = real_dragao3(2048);
            else if(c == 3) nt = real_isa(512);
            else nt = real_recta(2048, 2);

            /* 1. o campo na BASE, e o k de cada índice pela escrita incremental */
            an_zera(n);
            for(long t = 0; t < nt; t++){ an_visita(traj[t]); k_i[t] = an_le(traj[t]); }
            for(long t = 0; t < nt; t++) g_i[t] = an_le(traj[t]);   /* G(π(i)), já total */
            long max_base = 0; int recontou = an_bate_recontagem(nt, &max_base);
            long soma_base = an_soma();
            if(!recontou || soma_base != nt) mau++;

            /* 2. o LEVANTAMENTO: π̃(i) = (π(i), k(i)) — uma coordenada a mais, e UMA só */
            for(long t = 0; t < nt; t++){
                lev[t] = traj[t];
                lev[t].c[n] = (int)k_i[t];
            }

            /* 3. G̃ ≡ 1: a MESMA aranha, agora em dimensão n+1 */
            an_zera(n + 1);
            for(long t = 0; t < nt; t++) an_visita(lev[t]);
            long max_lev = 0; int rec_lev = an_bate_recontagem_em(lev, nt, &max_lev);
            long cel_lev = an_ocupadas, soma_lev = an_soma();
            if(!rec_lev || max_lev != 1 || cel_lev != nt || soma_lev != nt) mau++;

            /* 4. pr₁ ∘ π̃ = π: apagar a folha devolve a base, resíduo 0 */
            long residuo = 0;
            for(long t = 0; t < nt; t++)
                for(int j = 0; j < n; j++)
                    if(lev[t].c[j] != traj[t].c[j]) residuo++;
            if(residuo) mau++;

            /* 5. a FIBRA VERTICAL é {1,…,G(x)}: os k estão nos limites, são distintos
             *    (é a injectividade do ponto 3) e são G(x) deles — logo são todos.
             *    Aqui mede-se o que NÃO sai da injectividade: os limites. */
            long fora = 0;
            for(long t = 0; t < nt; t++)
                if(k_i[t] < 1 || k_i[t] > g_i[t]) fora++;
            if(fora) mau++;

            /* 6. A VOLTA: reconstruir G a partir SÓ do levantamento, sem o campo da
             *    base. E pr₁ NÃO É um passo que se escreva: é o campo a correr em
             *    dimensão n sobre os mesmos vectores — a folha está lá e o campo não
             *    a lê, que é literalmente esquecer a última coordenada. (Escrevi
             *    `v.c[n] = 0` antes de perceber isso; o gume mostrou que a linha não
             *    fazia nada, porque an_igual só compara an_dim coordenadas.) */
            an_zera(n);
            for(long t = 0; t < nt; t++) an_visita(lev[t]);
            long volta_mau = 0;
            for(long t = 0; t < nt; t++) if(an_le(lev[t]) != g_i[t]) volta_mau++;
            if(volta_mau) mau++;

            printf("      %-11s  %d  %d    %-11ld %-7ld %-8ld %-4ld %-6s %s\n",
                   cs[c].nome, n, n + 1, max_base, max_lev, cel_lev, residuo,
                   fora ? "NÃO" : "sim", volta_mau ? "NÃO" : "res. 0");
            if(max_base > 1) com_dobra++; else degenerados++;
            casos++;
        }
        printf("\n");
        ok("A ARANHA INVERSA GENERALIZA, E SOBE UM ANDAR SÓ: o levantamento"
           " π̃(i) = (π(i), k(i)) leva ℤⁿ em ℤⁿ⁺¹ para todo n — a folha é UMA coordenada,"
           " não n —, e as quatro cláusulas do `thm:aranha-inversa` medem-se em ℤ¹, ℤ²,"
           " ℤ³ e ℤ⁴ com o MESMO código: G̃ ≡ 1 (e não por comparação de pares: é a"
           " própria aranha a correr em dimensão n+1, o que faz dois caminhos),"
           " pr₁∘π̃ = π com resíduo 0, e a fibra vertical em cada x é {1,…,G(x)}."
           " E A VOLTA, que é o que faz dele um inverso e não um levantamento: sem o"
           " campo da base, projectar o levantamento e recontar devolve o G original"
           " célula a célula. E os CONTROLOS são dois, não um: a recta E o Cantor têm"
           " G ≡ 1 já na base, logo não há fibra a desfazer neles e «G̃ ≡ 1» passaria"
           " sozinho — três das cinco realizações dobram (dragão ℤ², dragão ℤ³ e a ISA)"
           " e duas não. Contei o Cantor entre as que dobram antes de olhar para o"
           " max G da sua própria linha, que a §AN4 já tinha medido a 1",
           mau == 0 && casos == 5 && com_dobra == 3 && degenerados == 2);
    }


    /* ═══ §AN10: o passo lê o ESTADO ou lê o ÍNDICE — e onde está o outro ════ */
    printf("\n§AN10  quem lê o espaço e quem lê o tempo: a aranha e o dragão.\n\n");
    {
        long mau = 0, det = 0, ndet = 0;
        struct { const char *nome; int n; int espera_det; } cs[4];
        cs[0].nome = "dragão ℤ²"; cs[0].n = 2; cs[0].espera_det = 0;
        cs[1].nome = "Julia ℤ[i]"; cs[1].n = 2; cs[1].espera_det = 1;
        cs[2].nome = "relógio J";  cs[2].n = 2; cs[2].espera_det = 1;
        cs[3].nome = "recta";      cs[3].n = 2; cs[3].espera_det = 1;
        printf("      realização    |I|    células  o passo lê   testemunha\n");
        for(int c = 0; c < 4; c++){
            long nt;
            if(c == 0)      nt = real_dragao(512);
            else if(c == 1) nt = real_julia(-1, 0, 60, 0, 0);
            else if(c == 2) nt = real_relogio(3, 1, 60);
            else            nt = real_recta(512, 2);
            Res r = an_corre(cs[c].n, nt);
            long i = -1, j = -1;
            int nd = nao_determinista(nt, &i, &j);
            char test[96];
            if(nd) snprintf(test, sizeof test, "π(%ld)=π(%ld) e π(%ld)≠π(%ld)", i, j, i+1, j+1);
            else   snprintf(test, sizeof test, "—");
            printf("      %-13s %-6ld %-8ld %-12s %s\n",
                   cs[c].nome, r.nt, r.celulas, nd ? "o ÍNDICE" : "o ESTADO", test);
            if(nd == cs[c].espera_det) mau++;
            if(nd) ndet++; else det++;
            if(!r.recontou || r.soma != r.nt) mau++;
        }
        /* E O RELÓGIO LÊ-SE NO CAMPO: período 4, cauda 0. */
        long ntr = real_relogio(3, 1, 60);
        Res rr = an_corre(2, ntr);
        long cauda_r = 0, per_r = periodo_busca(ntr, &cauda_r);
        printf("\n      o relógio no campo: período %ld (busca) · |{G>1}| = %ld · cauda %ld\n\n",
               per_r, rr.dobradas, cauda_r);
        if(per_r != 4 || rr.dobradas != per_r || cauda_r != 0) mau++;

        ok("NÃO HÁ NADA FORA: a hipótese do §AN8 não separa o matemático do resto —"
           " separa DUAS DEPENDÊNCIAS, e as duas vivem aqui dentro. Ou o passo é função"
           " da CÉLULA (o Julia, o relógio J, a recta) ou é função do ÍNDICE (o dragão)."
           " E não é opinião sobre a construção: procura-se na trajectória a mesma célula"
           " com sucessores DIFERENTES, e no dragão ela existe e exibe-se. É por isso que"
           " ele dobra sem ter período — a viragem `drag_esq(s)` lê s, não lê onde está."
           " OS DOIS SÃO DUAIS: a ARANHA põe a memória no ESPAÇO e o agente não guarda a"
           " história; o DRAGÃO põe-na no ÍNDICE e o espaço não a tem. O levantamento"
           " π̃ = (π,k) é a ponte, e cobra o preço no §AN9: junta os dois num estado só e"
           " a dobra desaparece, G̃ ≡ 1. Não se pode ter as duas coisas ao mesmo tempo."
           " E o relógio da casa — a meia-volta involutiva, J:(a,b)↦(−b,a) — é"
           " determinista, logo o §AN8 aplica-se-lhe e lê o seu período: 4",
           mau == 0 && det == 3 && ndet == 1);
    }

    /* ═══ §AN11: a SÉRIE DE π — o campo lê o custo da convergência ═══════════ */
    printf("\n§AN11  a série de π: três caminhos, e o G a ler o custo.\n\n");
    {
        const long S = 100000000L;                 /* escala 10^8, tudo inteiro */
        long t5, t239, t2, t3, t7;
        long a5 = arctan_inv(5, S, &t5), a239 = arctan_inv(239, S, &t239);
        long a2 = arctan_inv(2, S, &t2), a3 = arctan_inv(3, S, &t3);
        long a7 = arctan_inv(7, S, &t7);
        long machin = 16*a5 - 4*a239, euler = 4*a2 + 4*a3, herman = 8*a2 - 4*a7;
        const long CORTE = 100;                    /* 6 casas: o que a escala 10^8 sustenta */
        printf("      Machin  16·atan(1/5) − 4·atan(1/239) = %ld\n", machin);
        printf("      Euler    4·atan(1/2) + 4·atan(1/3)   = %ld\n", euler);
        printf("      Hermann  8·atan(1/2) − 4·atan(1/7)   = %ld\n", herman);
        printf("      a 6 casas: %ld · %ld · %ld\n\n", machin/CORTE, euler/CORTE, herman/CORTE);
        int concordam = (machin/CORTE == euler/CORTE) && (euler/CORTE == herman/CORTE);

        /* a trajectória das somas parciais, e o campo sobre ela */
        long mau = 0;
        printf("      arctan(1/n)   termos até o termo ir a 0   |I|   células  |{G>1}|  período  cauda\n");
        struct { long n, termos; } sr[4];
        sr[0].n = 5; sr[0].termos = t5;  sr[1].n = 239; sr[1].termos = t239;
        sr[2].n = 2; sr[2].termos = t2;  sr[3].n = 3;   sr[3].termos = t3;
        for(int c = 0; c < 4; c++){
            long PASSOS = 40;
            long nt = real_serie(sr[c].n, S, PASSOS);
            Res r = an_corre(1, nt);
            long cauda = 0, per = periodo_busca(nt, &cauda);
            printf("      1/%-11ld %-27ld %-5ld %-8ld %-8ld %-8ld %ld\n",
                   sr[c].n, sr[c].termos, r.nt, r.celulas, r.dobradas, per, cauda);
            /* o ponto fixo é UM: período 1. E a cauda que o campo lê é o número de
             * somas parciais DISTINTAS antes de estabilizar — o custo da série. */
            if(per != 1) mau++;
            if(r.dobradas != 1) mau++;
            if(cauda != r.celulas - 1) mau++;
            if(cauda + 1 > sr[c].termos) mau++;   /* nunca mais distintas que termos */
            if(!r.recontou || r.soma != r.nt) mau++;
        }
        printf("\n");
        ok("A SÉRIE DE π É UMA ÓRBITA, E O CAMPO LÊ O SEU CUSTO: as somas parciais de"
           " arctan(1/n) em aritmética inteira formam uma trajectória que entra num PONTO"
           " FIXO quando o termo vai a zero — e convergir, sem vírgula, é exactamente"
           " isso. O §AN8 lê-o: período 1, uma única célula com G > 1 (o ponto fixo), e a"
           " cauda é o número de somas parciais DISTINTAS, que é o custo da série. E o"
           " valor não é escrito à mão: Machin, Euler e Hermann são três somas alternadas"
           " independentes, em inteiros, e exige-se que os três coincidam a 6 casas — se"
           " eu tivesse copiado os dígitos, esta asserção não poderia falhar. π está"
           " DENTRO: é construído, não citado",
           mau == 0 && concordam && machin > 0);
    }


    /* ═══ §AN12: a TRANSFORMADA ALGÉBRICA — a ida é Dirac, a dobra é a norma ══ */
    printf("\n§AN12  a transformada algébrica sobre o campo: convolução e deconvolução.\n\n");
    {
        static long perA[4096], perB[4096], gA[TA_N], gB[TA_N], FA[TA_N], FB[TA_N];
        static long h[TA_N], Fh[TA_N], Fq[TA_N], q[TA_N], gr[TA_N], Fr[TA_N];
        long mau = 0;

        /* Dois percursos, e a escolha deles é parte da medida — custou duas
         * tentativas. O factor tem de ter espectro SEM ZEROS (senão não há
         * deconvolução) e NÃO pode ser δ₀ mais uma constante: com esses, TODAS as
         * convoluções coincidem, seja qual for o grupo, e a medida deixaria de
         * falar deste grupo. O primeiro que escrevi, (t·37 mod 97), tinha 26 zeros
         * em 256; o segundo, «volta sempre à origem», era δ₀ + constante e o gume
         * mostrou-o: trocar o XOR por soma cíclica não derrubava nada.
         * Este serve: A = 3·δ₀ + δ₁ tem espectro {4,2}, nunca zero, e não é
         * invariante à escolha do grupo. */
        long nA = 0;
        perA[nA++] = 0; perA[nA++] = 0; perA[nA++] = 0; perA[nA++] = 1;
        long nB = 0;
        perB[nB++] = 0; perB[nB++] = 3; perB[nB++] = 5; perB[nB++] = 6;  /* injectivo */
        ta_campo(perA, nA, gA);
        ta_campo(perB, nB, gB);
        ta_F(gA, FA); ta_F(gB, FB);

        /* (a) A IDA É UMA SOMA DE DIRACS: F(G)_k = Σ_t χ_k(π(t)). Dois caminhos —
         *     transformar o campo, ou somar os caracteres ao longo da trajectória. */
        long dif_dirac = 0;
        for(long k = 0; k < TA_N; k++){
            long s = 0;
            for(long t = 0; t < nA; t++) s += ta_chi(k, perA[t] & (TA_N-1));
            if(s != FA[k]) dif_dirac++;
        }
        printf("      F(G)_k = Σ_t χ_k(π(t)):  divergências em %ld de %d\n", dif_dirac, TA_N);
        if(dif_dirac) mau++;

        /* (b) A DOBRA LÊ-SE NA NORMA. Três caminhos para a mesma quantidade:
         *     contar os pares i<j com π(i)=π(j); somar G²; e ‖FG‖²/n. */
        long pares = 0;
        for(long i = 0; i < nA; i++)
            for(long j = i+1; j < nA; j++)
                if((perA[i] & (TA_N-1)) == (perA[j] & (TA_N-1))) pares++;
        long somaq = 0; for(long i = 0; i < TA_N; i++) somaq += gA[i]*gA[i];
        long normaF = 0; for(long k = 0; k < TA_N; k++) normaF += FA[k]*FA[k];
        printf("      dobra:  pares = %ld · ΣG² = %ld · ‖FG‖²/n = %ld · |I| + 2·pares = %ld\n",
               pares, somaq, normaF/TA_N, nA + 2*pares);
        if(normaF % TA_N) mau++;
        if(!(somaq == normaF/TA_N && somaq == nA + 2*pares)) mau++;

        /* e o CONTROLO: no percurso injectivo o excesso é ZERO */
        long paresB = 0;
        for(long i = 0; i < nB; i++)
            for(long j = i+1; j < nB; j++)
                if((perB[i] & (TA_N-1)) == (perB[j] & (TA_N-1))) paresB++;
        long somaqB = 0; for(long i = 0; i < TA_N; i++) somaqB += gB[i]*gB[i];
        long normaFB = 0; for(long k = 0; k < TA_N; k++) normaFB += FB[k]*FB[k];
        printf("      controlo injectivo:  pares = %ld · ΣG² = %ld = |I| = %ld · ‖FG‖²/n = %ld\n\n",
               paresB, somaqB, nB, normaFB/TA_N);
        if(!(paresB == 0 && somaqB == nB && normaFB/TA_N == nB)) mau++;

        /* (c) A CONVOLUÇÃO, e a transformada a diagonalizá-la: F(f*g) = F(f)·F(g) */
        ta_conv(gA, gB, h);
        ta_F(h, Fh);
        long dif_conv = 0;
        for(long k = 0; k < TA_N; k++) if(Fh[k] != FA[k]*FB[k]) dif_conv++;
        printf("      F(G_A * G_B) = F(G_A)·F(G_B):  divergências em %ld de %d  (custo n² → n)\n",
               dif_conv, TA_N);
        if(dif_conv) mau++;

        /* (d) A DECONVOLUÇÃO: divide casa a casa e devolve o factor exacto */
        long zeros = 0, resto = 0;
        for(long k = 0; k < TA_N; k++){
            if(FA[k] == 0){ zeros++; Fq[k] = 0; continue; }
            if(Fh[k] % FA[k]) resto++;
            Fq[k] = Fh[k] / FA[k];
        }
        int volta = zeros ? 0 : ta_Finv(Fq, q);
        long dif_dec = 0;
        if(volta) for(long i = 0; i < TA_N; i++) if(q[i] != gB[i]) dif_dec++;
        printf("      deconvolução F(h)/F(G_A):  zeros no espectro %ld · restos %ld · resíduo %ld\n",
               zeros, resto, volta ? dif_dec : -1);
        if(zeros || resto || !volta || dif_dec) mau++;

        /* (e) E ONDE ELA DEGENERA, exibido: um factor com zero no espectro. O campo
         *     de um percurso que visita 0 e 1 uma vez cada tem F_k = 1 + χ_k(1),
         *     que é ZERO em metade dos k — e aí a divisão não existe. */
        for(long i = 0; i < TA_N; i++) gr[i] = 0;
        gr[0] = 1; gr[1] = 1;
        ta_F(gr, Fr);
        long zeros_r = 0;
        for(long k = 0; k < TA_N; k++) if(Fr[k] == 0) zeros_r++;
        printf("      degenerescência: o campo {0,1} tem %ld zeros no espectro de %d"
               " — metade, e aí a volta NÃO existe\n\n", zeros_r, TA_N);
        if(zeros_r != TA_N/2) mau++;

        ok("A TRANSFORMADA ALGÉBRICA APLICA-SE AO CAMPO, E O GRUPO JÁ ERA O DA CASA:"
           " (ℤ/2)^m, caracteres ±1, tudo inteiro e sem um float — o mesmo do"
           " `transformada.c` §U1--§U5, e o mesmo em que o endereço da árvore do §AN6 já"
           " vivia. TRÊS COISAS. A IDA da aranha é uma soma de Diracs, e a transformada"
           " di-lo: F(G)_k = Σ_t χ_k(π(t)), o delta a espalhar-se em tudo (§U5)."
           " A DOBRA lê-se na NORMA: por Parseval (§U3) ‖FG‖² = n·ΣG², e ΣG² = |I| +"
           " 2·(pares dobrados) — três caminhos para o mesmo número, e o percurso"
           " injectivo dá excesso ZERO, que é o controlo. Logo π é injectiva ⟺"
           " ‖FG‖² = n·|I|: a dobra mede-se sem visitar a trajectória. E a CONVOLUÇÃO"
           " compõe campos com a transformada a diagonalizá-la casa a casa, custo n² → n,"
           " enquanto a DECONVOLUÇÃO os decompõe, com resíduo 0 e divisão inteira exacta."
           " MAS SÓ CONTRA UM FACTOR INVERTÍVEL NO ESPECTRO, e essa não é uma condição"
           " técnica — é A condição. E A ESCOLHA DO FACTOR É PARTE DA MEDIDA, o que me"
           " custou duas tentativas: (t·37 mod 97) tinha 26 zeros em 256 e a volta não"
           " existia; e «voltar sempre à origem» era δ₀ mais uma constante — para esses,"
           " TODAS as convoluções coincidem seja qual for o grupo, e o gume mostrou-o"
           " (trocar o XOR por soma cíclica não derrubava nada). O que serve é"
           " A = 3·δ₀ + δ₁, com espectro {4,2}. A degenerescência exibe-se no extremo:"
           " o campo {0,1} tem F_k = 1 + χ_k(1), zero em METADE dos k. É a mesma do"
           " `transformada_universal.c` §T4, neste grupo",
           mau == 0);
    }


    /* ═══ §AN13: o CICLO da aranha em ℤⁿ — escrita, leitura, decisão, fecho ══ */
    printf("\n§AN13  o ciclo: escrita · leitura · decisão · fecho, em ℤⁿ.\n\n");
    {
        long mau = 0, dims = 0;
        printf("      n   |V|  lê G=0 vs G>0  gradiente escolhe min  fronteira=∞  planos R⁴=id\n");
        for(int n = 1; n <= 4; n++){
            an_zera(n);
            Vet o = vet0();
            /* PASSO 1 — ESCRITA: a marca fica no espaço, não no agente */
            an_visita(o);

            /* monta a vizinhança e dá a cada vizinho um G diferente, para que o
             * mínimo seja único e a decisão não possa acertar por acaso */
            Vet vs[2*AN_N]; long viz = 0;
            for(int i = 0; i < n; i++)
                for(int sg = -1; sg <= 1; sg += 2){
                    Vet v = o; v.c[i] += sg; vs[viz++] = v;
                }
            /* o vizinho 0 fica com G = 0 (nunca visitado); os outros com 2,3,4,… */
            for(long a = 1; a < viz; a++)
                for(long r = 0; r <= a; r++) an_visita(vs[a]);

            /* PASSO 2 — LEITURA: P_t(x) sobre V(x) distingue G=0 de G>0 */
            long zeros = 0, positivos = 0;
            for(long a = 0; a < viz; a++){
                if(an_le(vs[a]) == 0) zeros++; else positivos++;
            }
            int distingue = (zeros == 1 && positivos == viz - 1);

            /* PASSO 3 — DECISÃO: gradiente, o vizinho de MENOR G. A fronteira lê-se
             * como obstáculo (+∞) e por isso nunca é escolhida. Aqui a fronteira é
             * o último vizinho, marcado com o valor de bloqueio. */
            const long INF = 1L << 40;
            long melhor = -1, melhorg = INF;
            for(long a = 0; a < viz; a++){
                long g = (a == viz - 1) ? INF : an_le(vs[a]);   /* fronteira */
                if(g < melhorg){ melhorg = g; melhor = a; }
            }
            int escolheu_min = (melhor == 0 && melhorg == 0);
            /* e o controlo da fronteira: sem o obstáculo ela seria a escolhida,
             * o que mostra que o +∞ está a fazer trabalho */
            long melhor2 = -1, melhorg2 = INF;
            for(long a = 0; a < viz; a++){
                long g = (a == viz - 1) ? 0 : an_le(vs[a]) + 1;
                if(g < melhorg2){ melhorg2 = g; melhor2 = a; }
            }
            int fronteira_conta = (melhor2 == viz - 1);

            /* PASSO 4 — FECHO: R⁴ = id. Em ℤⁿ a rotação de 90° vive num PLANO
             * coordenado (e_i,e_j), e há n(n−1)/2 deles; em cada um, quatro
             * aplicações devolvem o ponto. É o mesmo J⁴ = id da bidualidade. */
            long planos = 0, planos_ok = 0;
            for(int i = 0; i < n; i++)
                for(int j = i+1; j < n; j++){
                    Vet v = vet0();
                    for(int c = 0; c < n; c++) v.c[c] = c + 1;
                    Vet u = v;
                    for(int r = 0; r < 4; r++){                  /* R: (a,b) ↦ (−b,a) */
                        int a = u.c[i], b = u.c[j];
                        u.c[i] = -b; u.c[j] = a;
                    }
                    planos++;
                    if(an_igual(u, v)) planos_ok++;
                }
            int fecho_ok = (planos == (long)n*(n-1)/2) && (planos_ok == planos);

            printf("      %d   %-4ld %-15s %-21s %-12s %ld de %ld\n",
                   n, viz, distingue ? "sim" : "NÃO",
                   escolheu_min ? "sim" : "NÃO",
                   fronteira_conta ? "sim" : "NÃO", planos_ok, planos);
            if(!distingue || !escolheu_min || !fronteira_conta || !fecho_ok) mau++;
            if(viz != 2*n) mau++;
            dims++;
        }
        printf("\n");
        ok("O CICLO INTEIRO CORRE EM ℤⁿ, e cada passo dele É uma cláusula e não um"
           " extra. ESCRITA: a marca fica no espaço. LEITURA: P_t(x) sobre V(x)"
           " distingue G = 0 de G > 0 — exactamente um vizinho por visitar em cada"
           " dimensão, e os outros 2n−1 já marcados. DECISÃO: o gradiente vai ao vizinho"
           " de MENOR G, e não é escolha entre planos — é o mínimo do que está escrito"
           " no chão; a fronteira lê-se como obstáculo (+∞) e por isso nunca é escolhida,"
           " e o controlo mostra que o +∞ trabalha: tirado, é ela a escolhida. FECHO:"
           " R⁴ = id, o mesmo J⁴ = id da bidualidade — e este é o único passo em que a"
           " dimensão faz mais do que contar vizinhos, porque a rotação vive num PLANO"
           " e em ℤⁿ há n(n−1)/2 deles: nenhum em ℤ¹, um em ℤ², três em ℤ³, seis em ℤ⁴,"
           " e em cada um o rotor fecha",
           mau == 0 && dims == 4);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
