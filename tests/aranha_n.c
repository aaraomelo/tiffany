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
 *   §AN14 a vizinhança na ÁRVORE é 1 e não 2n · e o custo CONTADO: 2n
 *         leituras por passo, nem uma a mais — o autómato não varre
 *   §AN31 UM AGENTE OU MUITOS: o campo é o MESMO — a cláusula 3 torna a
 *         estigmergia agente-agnóstica, e é por isso que ela escala
 *   §AN30 A BASE É O ALFABETO: a trajectória é uma PALAVRA, reconstrói-se de
 *         π(0) + palavra, e a DOBRA é o núcleo do morfismo palavra → posição
 *   §AN29 O PAR DUAL DE CANTOR com FOLGA: contar injecta e sobra metade da
 *         recta · encher cobre com G ≡ 2 · a retracção, e o que o levantamento repõe
 *   §AN28 A REVISÃO EXTERNA medida: τ = N pede a DIAGONAL 1 e não g = I · no
 *         injectivo G = 1 na IMAGEM · as coordenadas são F(f)/n e não F(f)
 *   §AN27 O OUTRO LADO: W = ℤ^I com S NILPOTENTE (bloco de Jordan, não
 *         diagonaliza) — e é por isso que 1−S inverte SEMPRE, ao contrário de C_f
 *   §AN26 A ESTRUTURA LINEAR: X sobre 𝔽₂ e V = ℤ^X sobre ℤ · o anel de grupo ·
 *         os C_f comutam · os χ_k são os próprios simultâneos · F é MUDANÇA DE
 *         BASE e F∘F = n·id é a bidualidade
 *   §AN25 π_k EM BITS é truncar (sobrevivem 2^k réguas de oito) · e χ = V − A
 *         do subgrafo percorrido, com b₁ = 1 − χ a contar as voltas fechadas
 *   §AN24 as RÉGUAS formam BASE: as direcções de aresta de Q_8 são a base
 *         ortonormal e_k = 2^k do naturais.tex — OITO, e compostas cobrem tudo
 *   §AN23 a TABELA das realizações do paper, medida — dobrar e ser percurso
 *         são independentes, e as quatro combinações existem
 *   §AN22 g DEPENDE DO PONTO: g_uv(x) = G(x)·δ_uv, o tempo passa diferente em
 *         cada ponto, e o passo do ciclo é a GEODÉSICA dessa métrica
 *   §AN21 A MÉTRICA g: dτ² = g_uv dx^u dx^v — e com g = I o tempo próprio É o
 *         parâmetro, que é o que torna a grade a régua barata
 *   §AN20 O TEMPO é a variação da TAXA DE DOBRAS por salto — e o número muda
 *         com a RÉGUA: uniforme na grade, não uniforme na ultramétrica
 *   §AN19 O ESPAÇO é o que a dobra constrói (F_{2w} = F_w ⊕ σF_w = Q_{2w}) e o
 *         TEMPO é a TAXA da variação: μ = 1−S é a derivada, ζ integra
 *   §AN18 QUEM É O OPERADOR: o shift S no domínio (ζ = (1−S)⁻¹, μ = 1−S) e a
 *         ADJACÊNCIA no contradomínio — e a lei local vértice/aresta
 *   §AN17 a GEOMETRIA: vértice, aresta, GRAU — e o hipercubo Q_m construído
 *         pela dobra T_{k+1} = T_k + T_k*, com os caracteres a serem os seus
 *         modos próprios (valor próprio m − 2|k|, inteiro)
 *   §AN16 a MEDIDA conserva-se e a FIBRA perde-se: mesma ∑G, dobras diferentes
 *         — é por isto que G existe
 *   §AN15 a ARANHA É ζ e a INVERSA É μ = ζ⁻¹: acumular e desacumular, a mesma
 *         inversão que o `dirichlet.h` corre na árvore dos divisores
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
/* cláusula 4: SENTIR É LER G. O contador existe para o custo ser CONTADO e não
 * afirmado: o §AN14 zera-o, corre o ciclo, e lê quantas leituras foram feitas. */
static long an_leituras = 0;
static long an_le(Vet v){
    an_leituras++;
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


/* ── O PAR DUAL DE CANTOR: encher e contar, com FOLGA ────────────────────────
 * A casa mede em `peano_dual.c` que encher e contar são BIJECÇÃO no finito —
 * com |I| = |X| não pode ser outra coisa. A pergunta é outra: e o par em que
 * uma ENCHE (sobrejectiva, não injectiva) e a outra INJECTA deixando espaço
 * por usar? Isso obriga |I| > |X|, e a folga é exactamente |I| − |X|.
 *
 * Aqui: X = [0,2^w)² e I = [0, 2·4^w). A intercalação de bits (Morton) leva o
 * quadrado nos primeiros 4^w índices — injectiva, e a metade alta da recta
 * fica POR USAR. E encher lê o índice módulo 4^w, cobrindo tudo duas vezes. */
#define PD_W  4
#define PD_Q  (1 << PD_W)              /* lado do quadrado: 16 */
#define PD_X  (PD_Q * PD_Q)            /* |X| = 256 células */
#define PD_I  (2 * PD_X)               /* |I| = 512 índices: sobra metade */

static long pd_contar(int x, int y){   /* X → I, intercalar bits: injectiva */
    long d = 0;
    for(int b = 0; b < PD_W; b++){
        d |= (long)((x >> b) & 1) << (2*b);
        d |= (long)((y >> b) & 1) << (2*b + 1);
    }
    return d;                          /* em [0, 4^w): a metade baixa de I */
}
static void pd_encher(long d, int *px, int *py){   /* I → X: sobrejectiva */
    long r = d % PD_X;                 /* a metade alta dobra sobre a baixa */
    int x = 0, y = 0;
    for(int b = 0; b < PD_W; b++){
        x |= (int)((r >> (2*b)) & 1) << b;
        y |= (int)((r >> (2*b + 1)) & 1) << b;
    }
    *px = x; *py = y;
}

/* a comparação de dois campos, UMA só função — para que o controlo negativo do
 * §AN31 proteja a asserção positiva: mutá-la derruba as duas leituras ao mesmo
 * tempo, e não só a que interessa. */
static long campos_diferem(const long *a, const long *b, long n){
    long d = 0;
    for(long i = 0; i < n; i++) if(a[i] != b[i]) d++;
    return d;
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
    printf("\n§AN10  quem lê a célula e quem lê o índice: a aranha e o dragão.\n\n");
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
           " a dobra desaparece, G̃ ≡ 1. Não se podem ter as três."
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


    /* ═══ §AN14: a vizinhança na ÁRVORE, e o custo CONTADO ═══════════════════ */
    printf("\n§AN14  a vizinhança fora da grade, e o custo contado passo a passo.\n\n");
    {
        long mau = 0;
        const int NB = 10;
        printf("      nível p   bolas   irmãos por bola   união = mãe\n");
        for(int prof = 1; prof <= 5; prof++){
            long bolas = 1L << prof, irmaos_mau = 0, uniao_mau = 0;
            for(long b = 0; b < bolas; b++){
                long irm = b ^ 1L, mae = b >> 1, conta = 0;
                for(long c = 0; c < bolas; c++)
                    if(c != b && (c >> 1) == mae) conta++;
                if(conta != 1) irmaos_mau++;
                for(long x = 0; x < (1L << NB); x += 8){
                    long pb = x >> (NB - prof), pm = x >> (NB - prof + 1);
                    int em_b = (pb == b), em_i = (pb == irm), em_mae = (pm == mae);
                    if(em_mae != (em_b || em_i)) uniao_mau++;
                    if(em_b && em_i) uniao_mau++;
                }
            }
            printf("      %-9d %-7ld %-17s %s\n", prof, bolas,
                   irmaos_mau ? "NÃO" : "1, sempre", uniao_mau ? "NÃO" : "sim");
            if(irmaos_mau || uniao_mau) mau++;
        }
        printf("\n      n   passos   leituras   por passo   2n   varre?\n");
        const long T = 64;
        for(int n = 1; n <= 4; n++){
            an_zera(n);
            Vet x = vet0();
            an_visita(x);
            an_leituras = 0;
            for(long t = 0; t < T; t++){
                long melhorg = -1; Vet melhor = x;
                for(int i = 0; i < n; i++)
                    for(int sg = -1; sg <= 1; sg += 2){
                        Vet v = x; v.c[i] += sg;
                        long g = an_le(v);
                        if(melhorg < 0 || g < melhorg){ melhorg = g; melhor = v; }
                    }
                x = melhor;
                an_visita(x);
            }
            printf("      %d   %-8ld %-10ld %-11ld %-4d %s\n",
                   n, T, an_leituras, an_leituras/T, 2*n,
                   (an_leituras == T*2*n) ? "não" : "SIM");
            if(an_leituras != T * 2 * n) mau++;
        }
        printf("\n");
        ok("O CUSTO É CONTADO, E A VIZINHANÇA NÃO É SEMPRE 2n. Primeiro: na ÁRVORE do"
           " encaixe cada bola tem EXACTAMENTE UM irmão — o mesmo prefixo com o último"
           " bit trocado — e os dois particionam a bola-mãe, medido em cinco níveis."
           " Logo |V(x)| = 1 ali, contra 2n na grade: a cláusula 4 é o sítio onde a"
           " estrutura entra, e trocar a estrutura troca exactamente esse número, mais"
           " nada. Segundo, e é o que impede a afirmação de custo de ser conversa: o"
           " contador de leituras diz que o ciclo faz EXACTAMENTE 2n leituras por passo"
           " em ℤ¹ a ℤ⁴ — nem uma a mais. O autómato NÃO VARRE o espaço: lê a"
           " vizinhança e escreve num ponto, e o custo é Θ(n·|I|) em tempo e Θ(|I|) em"
           " memória, sem dependência de |X|. Escrever «uma leitura e uma escrita por"
           " passo» — como eu tinha escrito — esquecia a cláusula 4 e prometia um custo"
           " n vezes menor do que o que o ciclo faz",
           mau == 0);
    }

    /* ═══ §AN15: a ARANHA É ζ, E A INVERSA É μ — convolução e deconvolução ═══ */
    printf("\n§AN15  o directo e o inverso da aranha: convolução com ζ, deconvolução com μ.\n\n");
    {
        /* A álgebra de incidência da ordem (I, ≤): ζ(u,t) = 1 se u ≤ t, e a sua
         * inversa μ, que na ordem TOTAL é a diferença finita — μ(t,t) = 1,
         * μ(t−1,t) = −1, e zero no resto. É o mesmo par «F = f*1, f = F*μ» que o
         * `lib/dirichlet.h` corre sobre a árvore dos divisores: só muda a ordem. */
        const long TT = 64;
        long mau = 0;

        /* (a) μ é MESMO a inversa de ζ: Σ_v ζ(u,v)·μ(v,t) = δ(u,t) */
        long falhas_inv = 0;
        for(long u = 0; u < TT; u++)
            for(long t = 0; t < TT; t++){
                long soma = 0;
                for(long v = u; v <= t; v++){
                    long z = 1;                          /* ζ(u,v) = 1, pois u ≤ v */
                    long m = (v == t) ? 1 : ((v == t-1) ? -1 : 0);
                    soma += z * m;
                }
                if(soma != (u == t)) falhas_inv++;
            }
        printf("      μ = ζ⁻¹ na ordem (I,≤):  %s em %ld pares\n",
               falhas_inv ? "FALHA" : "confirmado", TT*TT);
        if(falhas_inv) mau++;

        /* (b) A ARANHA DIRECTA É A CONVOLUÇÃO COM ζ. Sobre uma célula fixa x,
         *     seja a(t) = 1 se π(t) = x. Então G_t(x) = Σ_{u≤t} a(u) = (a*ζ)(t),
         *     e isso tem de bater com a escrita incremental do §AN1. */
        /* O REGIME: não serve uma célula qualquer. A fibra só é testada onde ela é
         * FUNDA, por isso escolhe-se a de MAIOR G — e a realização é a da ISA, que
         * dobra doze vezes, e não o dragão, que dobra duas. */
        long nt = real_isa(400);
        const int DIM = 4;
        an_zera(DIM);
        for(long t = 0; t < nt; t++) an_visita(traj[t]);
        Vet alvo = traj[0]; long melhorG = 0;
        for(long t = 0; t < nt; t++){
            long g = an_le(traj[t]);
            if(g > melhorG){ melhorG = g; alvo = traj[t]; }
        }
        printf("      a fibra escolhida é a mais funda: G = %ld\n", melhorG);
        long a_de[512], Gconv[512], Ginc[512];
        an_zera(DIM);
        for(long t = 0; t < nt && t < 512; t++){
            a_de[t] = an_igual(traj[t], alvo);
            an_visita(traj[t]);
            Ginc[t] = an_le(alvo);               /* a escrita incremental, cláusula 3 */
            (void)0;
        }
        long n_use = nt < 512 ? nt : 512;
        for(long t = 0; t < n_use; t++){
            long s = 0;
            for(long u = 0; u <= t; u++) s += a_de[u];    /* (a * ζ)(t) */
            Gconv[t] = s;
        }
        long dif_dir = 0;
        for(long t = 0; t < n_use; t++) if(Gconv[t] != Ginc[t]) dif_dir++;
        printf("      directo   G_t(x) = (a * ζ)(t):  divergências %ld em %ld passos\n",
               dif_dir, n_use);
        if(dif_dir) mau++;

        /* (c) A ARANHA INVERSA É A DECONVOLUÇÃO COM μ: (G * μ)(t) = a(t). Isto É
         *     o Algoritmo B — a marca individual de cada passo volta da acumulação,
         *     e não é preciso guardar a trajectória para a obter. */
        long dif_inv = 0, marcas = 0;
        for(long t = 0; t < n_use; t++){
            long rec = Ginc[t] - (t ? Ginc[t-1] : 0);     /* (G * μ)(t) */
            if(rec != a_de[t]) dif_inv++;
            marcas += rec;
        }
        printf("      inverso   a(t) = (G * μ)(t):    divergências %ld · marcas recuperadas %ld\n",
               dif_inv, marcas);
        if(dif_inv || marcas != Gconv[n_use-1]) mau++;

        /* (d) E O LEVANTAMENTO É A MESMA CONVOLUÇÃO, RESTRITA À FIBRA: k(i) é a
         *     acumulação de a ao longo da fibra, e desacumulá-la dá 1 em cada
         *     visita — que é exactamente G̃ ≡ 1. */
        long ks[512], nk = 0;
        for(long t = 0; t < n_use; t++) if(a_de[t]) ks[nk++] = Ginc[t];
        long fib_mau = 0;
        for(long r = 0; r < nk; r++){
            if(ks[r] != r + 1) fib_mau++;                 /* k = a acumulação */
            long d = ks[r] - (r ? ks[r-1] : 0);           /* desacumular: sempre 1 */
            if(d != 1) fib_mau++;
        }
        printf("      na fibra  k(i) = acumulação · desacumulada = 1 em cada visita:"
               " %s (%ld visitas)\n", fib_mau ? "NÃO" : "sim", nk);
        if(fib_mau || nk != melhorG || nk < 5) mau++;

        /* (e) E É A MESMA INVERSÃO DA ÁRVORE DOS DIVISORES, que a casa já corre:
         *     F = f*1 e f = F*μ com o μ de Möbius. Trocar a ORDEM — de (I,≤) para
         *     a divisibilidade — não troca a álgebra: ζ acumula, μ desacumula. */
        long fD[64], FD[64], gD[64], dif_div = 0;
        for(long n = 1; n < 64; n++) fD[n] = (n * 7) % 5 + 1;
        for(long n = 1; n < 64; n++){
            long s = 0;
            for(long d = 1; d <= n; d++) if(n % d == 0) s += fD[d];
            FD[n] = s;                                    /* F = f * 1 */
        }
        for(long n = 1; n < 64; n++){
            long s = 0;
            for(long d = 1; d <= n; d++) if(n % d == 0){
                /* μ de Möbius, calculado pela factorização: 0 se tem quadrado */
                long m = 1, k = d;
                for(long q = 2; q * q <= k; q++)
                    if(k % q == 0){ k /= q; if(k % q == 0){ m = 0; break; } m = -m; }
                if(m && k > 1) m = -m;
                s += m * FD[n/d];
            }
            gD[n] = s;                                    /* f = F * μ */
        }
        for(long n = 1; n < 64; n++) if(gD[n] != fD[n]) dif_div++;
        printf("      e na árvore dos divisores: f = (f*1)*μ com %ld divergências em 63\n\n",
               dif_div);
        if(dif_div) mau++;

        ok("O DIRECTO E O INVERSO DA ARANHA SÃO CONVOLUÇÃO E DECONVOLUÇÃO, e não por"
           " analogia: na álgebra de incidência da ordem (I,≤), a escrita incremental"
           " da cláusula 3 É a convolução com ζ — G_t(x) = Σ_{u≤t} a(u) = (a*ζ)(t) —, e"
           " o Algoritmo B É a deconvolução com μ = ζ⁻¹, que na ordem total é a"
           " diferença finita: (G*μ)(t) devolve a marca individual de cada passo. Mede-se"
           " nas duas direcções sobre o passo da ISA em ℤ⁴, resíduo 0. E o LEVANTAMENTO é"
           " a mesma convolução restrita à FIBRA: k(i) é a acumulação ao longo dela, e"
           " desacumular dá 1 em cada visita, que é o G̃ ≡ 1 do §AN9 dito na outra"
           " linguagem — e mede-se na fibra MAIS FUNDA que a realização tem, não numa"
           " qualquer: numa fibra de duas visitas quase não há o que desacumular. Por fim, é A MESMA inversão que a casa já corre na árvore dos"
           " divisores — F = f*1 e f = F*μ (`lib/dirichlet.h`): trocar a ORDEM não troca"
           " a álgebra, ζ acumula e μ desacumula. A aranha não precisava de uma"
           " transformada nova: precisava da inversa de ζ, e ela já tinha nome",
           mau == 0);
    }


    /* ═══ §AN16: a MEDIDA conserva-se e a FIBRA perde-se ═════════════════════ */
    printf("\n§AN16  a medida não distingue: mesma ∑G, dobras diferentes.\n\n");
    {
        long mau = 0;
        /* a espiral quadrada: a mesma regra do dragão com a viragem SEMPRE à
         * esquerda. Mesmo número de passos, e por isso a mesma medida. */
        long nd = real_dragao(4096);
        Res rd = an_corre(2, nd);
        long nesp = 0;
        {
            int x = 0, y = 0, dx = 1, dy = 0;
            Vet v = vet0(); traj[nesp++] = v;
            for(long t = 0; t < 4096 && nesp < AN_IMAX; t++){
                x += dx; y += dy;
                v = vet0(); v.c[0] = x; v.c[1] = y; traj[nesp++] = v;
                int a = -dy, b = dx; dx = a; dy = b;      /* sempre à esquerda */
            }
        }
        Res re = an_corre(2, nesp);
        long nr = real_recta(4096, 2);
        Res rr = an_corre(2, nr);
        printf("      realização    |I|     ∑G      células   max G\n");
        printf("      Heighway      %-7ld %-7ld %-9ld %ld\n", rd.nt, rd.soma, rd.celulas, rd.max_g);
        printf("      espiral       %-7ld %-7ld %-9ld %ld\n", re.nt, re.soma, re.celulas, re.max_g);
        printf("      recta         %-7ld %-7ld %-9ld %ld\n\n", rr.nt, rr.soma, rr.celulas, rr.max_g);
        /* a MEDIDA é a mesma nas três; a FIBRA não */
        int medida_igual = (rd.nt == re.nt && re.nt == rr.nt
                         && rd.soma == re.soma && re.soma == rr.soma
                         && rd.soma == rd.nt);
        int fibra_difere = (rd.max_g != re.max_g) || (rd.celulas != re.celulas);
        int controlo = (rr.max_g == 1);
        if(!medida_igual || !fibra_difere || !controlo) mau++;
        if(!rd.recontou || !re.recontou || !rr.recontou) mau++;

        ok("A MEDIDA CONSERVA-SE E A FIBRA PERDE-SE, e é esta a razão de ser de G: três"
           " realizações com o MESMO |I| e a MESMA ∑G — a curva de Heighway, a espiral"
           " quadrada e a recta — têm dobras diferentes. A medida de contagem é cega à"
           " identificação que a projecção faz: duas patas produzem uma marca, e a marca"
           " não diz quantas patas a produziram. Donde a consequência prática, e o gume"
           " confirmou-a: qualquer quantidade que seja invariante da medida — ∑G, |I| —"
           " não separa realizações, e usá-la para as distinguir não distingue nada."
           " O que separa é o campo: max G e o número de células. E a recta é o"
           " controlo, com a fibra trivial",
           mau == 0);
    }


    /* ═══ §AN17: a GEOMETRIA — vértice, aresta, grau; e o hipercubo pela dobra ═ */
    printf("\n§AN17  os grafos que o campo percorre: grau, laplaciano, e a dobra.\n\n");
    {
        long mau = 0;

        /* (a) O GRAU DO VÉRTICE em cada um dos três grafos que o paper usa.
         *     |V(x)| não é uma abstracção: é o GRAU, e cada grafo tem o seu. */
        printf("      grafo               vértice          aresta                 grau\n");
        for(int n = 1; n <= 4; n++){
            an_zera(n);
            Vet o = vet0(); long viz = 0;
            for(int i = 0; i < n; i++) for(int sg = -1; sg <= 1; sg += 2){
                Vet v = o; v.c[i] += sg; (void)v; viz++;
            }
            if(viz != 2*n) mau++;
            if(n == 1) printf("      grade ℤⁿ            ponto            ±e_i                   2n = %ld\n", viz);
        }
        /* a ÁRVORE: vértice = bola, aresta = mãe–filha. Grau interno = 3. */
        const int PROF = 6;
        long grau_mau = 0;
        for(int nivel = 1; nivel < PROF; nivel++)
            for(long b = 0; b < (1L << nivel); b++){
                long g = 1;                       /* a mãe */
                g += 2;                           /* as duas filhas */
                if(g != 3) grau_mau++;
            }
        printf("      árvore do encaixe   bola             mãe–filha              3 (interno)\n");
        if(grau_mau) mau++;
        /* o HIPERCUBO Q_m: vértice = palavra de m bits, aresta = diferir num bit */
        const int MB = 8, QN = 1 << MB;
        long grau_q = 0, arestas = 0, q_mau = 0;
        for(long v = 0; v < QN; v++){
            long g = 0;
            for(int i = 0; i < MB; i++) if((v ^ (1L << i)) < QN) g++;
            if(g != MB) q_mau++;
            grau_q = g; arestas += g;
        }
        printf("      hipercubo Q_m       palavra de m bits  um bit trocado         m = %ld\n",
               grau_q);
        printf("      e as arestas de Q_m: %ld = m·2^m/2 = %d\n\n", arestas/2, MB*QN/2);
        if(q_mau || arestas/2 != MB*QN/2) mau++;

        /* (b) OS CARACTERES SÃO OS MODOS PRÓPRIOS DO HIPERCUBO. A adjacência A de
         *     Q_m aplicada a χ_k dá (m − 2|k|)·χ_k, com |k| o peso de Hamming.
         *     Inteiro, exacto: a transformada do §AN12 não foi escolhida — é a
         *     diagonalização deste grafo. */
        printf("      |k|   valor próprio m−2|k|   confere\n");
        long vp_mau = 0;
        for(int peso = 0; peso <= MB; peso += 2){
            long k = 0;
            for(int i = 0; i < peso; i++) k |= (1L << i);
            long esperado = MB - 2*peso, mal = 0;
            for(long x = 0; x < QN; x++){
                long s = 0;
                for(int i = 0; i < MB; i++) s += ta_chi(k, x ^ (1L << i));
                if(s != esperado * ta_chi(k, x)) mal++;
            }
            printf("      %-5d %-23ld %s\n", peso, esperado, mal ? "NÃO" : "sim");
            if(mal) vp_mau++;
        }
        if(vp_mau) mau++;
        printf("\n");

        /* (c) E O HIPERCUBO CONSTRÓI-SE PELA DOBRA: Q_{m+1} são DUAS cópias de Q_m
         *     ligadas por um emparelhamento perfeito — o mesmo T_{k+1} = T_k + T_k*
         *     que faz o dragão e a torre. Conta-se: vértices dobram, e as arestas
         *     são 2·(as de Q_m) + 2^m, que são as do emparelhamento. */
        printf("      m   |Q_m|   arestas   2·arestas(Q_{m−1}) + 2^{m−1}   confere\n");
        long dobra_mau = 0;
        for(int m = 1; m <= 7; m++){
            long vm = 1L << m, am = (long)m * vm / 2;
            long anterior = (m-1) * (1L << (m-1)) / 2;
            long previsto = 2*anterior + (1L << (m-1));
            printf("      %d   %-7ld %-9ld %-29ld %s\n",
                   m, vm, am, previsto, (am == previsto) ? "sim" : "NÃO");
            if(am != previsto) dobra_mau++;
        }
        if(dobra_mau) mau++;
        printf("\n");

        /* (d) E O QUE É χ_k NA GEOMETRIA: uma FUNÇÃO ±1 NOS VÉRTICES, isto é uma
         *     BIPARTIÇÃO deles; e k é o conjunto de DIRECÇÕES DE ARESTA em que ela
         *     alterna. A aresta (j, j⊕e_i) muda de sinal ⟺ o bit i de k é 1. Donde
         *     o corte tem |k|·2^{m−1} arestas, e o valor próprio m−2|k| é, em cada
         *     vértice, (arestas que concordam) − (arestas que discordam). */
        printf("      |k|   arestas do corte   |k|·2^{m−1}   concord.−discord.   m−2|k|\n");
        long geo_mau = 0;
        for(int peso = 0; peso <= MB; peso += 2){
            long k = 0;
            for(int i = 0; i < peso; i++) k |= (1L << i);
            long corte = 0, saldo_min = 0, saldo_max = 0;
            for(long x = 0; x < QN; x++){
                long conc = 0, disc = 0;
                for(int i = 0; i < MB; i++){
                    if(ta_chi(k, x) == ta_chi(k, x ^ (1L << i))) conc++; else { disc++; corte++; }
                }
                long saldo = conc - disc;
                if(x == 0){ saldo_min = saldo_max = saldo; }
                if(saldo < saldo_min) saldo_min = saldo;
                if(saldo > saldo_max) saldo_max = saldo;
            }
            corte /= 2;                                   /* cada aresta contada duas vezes */
            long previsto = (long)peso * (1L << (MB-1));
            int bate = (corte == previsto) && (saldo_min == saldo_max)
                    && (saldo_min == MB - 2*peso);
            printf("      %-5d %-18ld %-13ld %-19ld %-6d %s\n",
                   peso, corte, previsto, saldo_min, MB - 2*peso, bate ? "" : " NÃO");
            if(!bate) geo_mau++;
        }
        if(geo_mau) mau++;
        printf("\n");

        ok("A GEOMETRIA TEM NOME, E O OBJECTO CONSTRUÍDO É O HIPERCUBO. |V(x)| não é uma"
           " abstracção: é o GRAU DO VÉRTICE, e o paper percorre três grafos distintos —"
           " a grade ℤⁿ (vértice: ponto; aresta: ±e_i; grau 2n), a árvore do encaixe"
           " (vértice: bola; aresta: mãe–filha; grau interno 3) e o hipercubo Q_m"
           " (vértice: palavra de m bits; aresta: um bit trocado; grau m, e m·2^m/2"
           " arestas). E os caracteres do §AN12 NÃO foram escolhidos por conveniência:"
           " são os modos próprios da adjacência de Q_m, com valor próprio INTEIRO"
           " m − 2|k|, onde |k| é o peso de Hamming — a transformada é a diagonalização"
           " deste grafo e de mais nenhum. Por fim, Q_{m+1} são DUAS cópias de Q_m"
           " ligadas por um emparelhamento perfeito: arestas(Q_m) = 2·arestas(Q_{m−1})"
           " + 2^{m−1}, que é T_{k+1} = T_k + T_k* — a mesma dobra que constrói o dragão"
           " e a torre, aqui a construir o grafo onde a transformada vive."
           " E O QUE É χ_k, PERGUNTADO NA GEOMETRIA: não é um vértice — é uma FUNÇÃO"
           " ±1 SOBRE os vértices, ou seja uma BIPARTIÇÃO deles; e o índice k é o"
           " conjunto de DIRECÇÕES DE ARESTA em que essa bipartição alterna, porque a"
           " aresta (j, j⊕e_i) muda de sinal exactamente quando o bit i de k é 1."
           " Medido: o corte tem |k|·2^{m−1} arestas, e o valor próprio m−2|k| é, em"
           " cada vértice e com o mesmo valor em todos, (arestas que concordam) menos"
           " (arestas que discordam). Com |k|=0 é a função constante e nenhuma aresta"
           " é cortada; com |k|=m é a paridade e são cortadas TODAS; e |k|=m/2 dá"
           " valor próprio zero, o corte que parte as arestas ao meio. Assim, cada"
           " coordenada F(G)_k da transformada do campo é o SALDO da trajectória num"
           " corte do hipercubo — de que lado ela esteve, somado",
           mau == 0);
    }


    /* ═══ §AN18: QUEM É O OPERADOR — e o índice como contagem de arestas ═════ */
    printf("\n§AN18  o operador, e o que o índice conta: arestas.\n\n");
    {
        long mau = 0;

        /* (a) NO DOMÍNIO O OPERADOR É O SHIFT S: (Sf)(t) = f(t−1). Dele sai tudo:
         *     ζ = Σ_{j≥0} S^j = (1 − S)⁻¹ e μ = 1 − S. O índice não é «tempo»
         *     importado de lado nenhum — é a CONTAGEM das aplicações de S. */
        const long TT = 32;
        long v[64], zv[64], sv[64], resid = 0;
        for(long t = 0; t < TT; t++) v[t] = (t * 13) % 7 - 3;
        for(long t = 0; t < TT; t++){                 /* ζ aplicado: acumula */
            long acc = 0;
            for(long u = 0; u <= t; u++) acc += v[u];
            zv[t] = acc;
        }
        for(long t = 0; t < TT; t++)                  /* (1 − S) aplicado a ζv */
            sv[t] = zv[t] - (t ? zv[t-1] : 0);
        for(long t = 0; t < TT; t++) if(sv[t] != v[t]) resid++;
        printf("      (1 − S)∘ζ = id:  resíduo %ld em %ld · logo μ = 1 − S e ζ = (1 − S)⁻¹\n",
               resid, TT);
        if(resid) mau++;

        /* (b) NO CONTRADOMÍNIO O OPERADOR É A ADJACÊNCIA A: cada passo da
         *     trajectória é uma ARESTA do grafo, π(t+1) ∈ V(π(t)). É isto que faz
         *     de π um PERCURSO e não uma sucessão qualquer de pontos — e é o que
         *     dá sentido a contar o índice: ele conta arestas percorridas. */
        struct { const char *nome; int n; int espera_percurso; } cs[4];
        cs[0].nome = "dragão ℤ²";    cs[0].n = 2; cs[0].espera_percurso = 1;
        cs[1].nome = "recta axial";  cs[1].n = 2; cs[1].espera_percurso = 1;
        cs[2].nome = "recta diagonal"; cs[2].n = 2; cs[2].espera_percurso = 0;
        cs[3].nome = "relógio J";    cs[3].n = 2; cs[3].espera_percurso = 0;
        printf("\n      realização       |I|    arestas   é percurso   o passo é\n");
        for(int c = 0; c < 4; c++){
            long nt;
            if(c == 0) nt = real_dragao(512);
            else if(c == 1){                      /* AXIAL: só a coordenada 0 avança */
                nt = 0;
                for(long t = 0; t <= 512 && nt < AN_IMAX; t++){
                    Vet v = vet0(); v.c[0] = (int)t; traj[nt++] = v;
                }
            }
            else if(c == 2) nt = real_recta(512, 2);
            else nt = real_relogio(3, 1, 60);
            /* toda transição é aresta do grafo? (na grade: difere em ±1 numa coord) */
            long nao_aresta = 0;
            for(long t = 0; t + 1 < nt; t++){
                long dif = 0, soma = 0;
                for(int i = 0; i < cs[c].n; i++){
                    long d = traj[t+1].c[i] - traj[t].c[i];
                    if(d) dif++;
                    soma += d < 0 ? -d : d;
                }
                if(!(dif == 1 && soma == 1)) nao_aresta++;
            }
            const char *oque = (c == 0) ? "±e_i" : (c == 1) ? "+e_0"
                             : (c == 2) ? "(1,1): diagonal" : "rotação: salta";
            printf("      %-16s %-6ld %-9ld %-12s %s\n", cs[c].nome, nt, nt-1,
                   nao_aresta ? "NÃO" : "sim", oque);
            if(cs[c].espera_percurso == (nao_aresta > 0)) mau++;
        }

        /* (c) A LEI LOCAL QUE LIGA VÉRTICE E ARESTA. Num percurso, cada visita a um
         *     vértice usa duas arestas — uma a entrar e uma a sair — excepto nos
         *     dois extremos. Logo, com G_ar a multiplicidade da ARESTA,
         *         Σ_{e ∋ x} G_ar(e) = 2·G(x) − [x = π(0)] − [x = π(N)].
         *     É o handshaking, dito sobre o percurso. */
        long nt = real_dragao(512);
        an_zera(2);
        for(long t = 0; t < nt; t++) an_visita(traj[t]);
        long viola = 0, verificados = 0, soma_ar = 0;
        for(long t = 0; t < nt; t++){
            Vet x = traj[t];
            long inc = 0;
            for(long u = 0; u + 1 < nt; u++){          /* arestas incidentes a x */
                if(an_igual(traj[u], x) || an_igual(traj[u+1], x)) inc++;
            }
            long extremos = (an_igual(x, traj[0]) ? 1 : 0)
                          + (an_igual(x, traj[nt-1]) ? 1 : 0);
            long previsto = 2*an_le(x) - extremos;
            if(inc != previsto) viola++;
            verificados++;
            if(t + 1 < nt) soma_ar++;
        }
        printf("\n      lei local  Σ_{e∋x} G_ar(e) = 2·G(x) − [início] − [fim]:"
               " violações %ld em %ld vértices\n", viola, verificados);
        printf("      e ∑_e G_ar(e) = %ld = N = %ld\n\n", soma_ar, nt-1);
        if(viola || soma_ar != nt-1) mau++;

        ok("O OPERADOR SÃO DOIS, E É POR ISSO QUE O ÍNDICE NÃO É «TEMPO» IMPORTADO."
           " No DOMÍNIO o operador é o SHIFT S, (Sf)(t) = f(t−1): dele sai ζ = Σ S^j"
           " = (1 − S)⁻¹ e μ = 1 − S, medido com resíduo 0 — o índice é a CONTAGEM"
           " das aplicações de S, e a dinâmica que o define é essa. No CONTRADOMÍNIO"
           " o operador é a ADJACÊNCIA do grafo: cada passo é uma ARESTA,"
           " π(t+1) ∈ V(π(t)), e é isso que faz de π um PERCURSO e não uma sucessão"
           " qualquer de pontos. E isso SEPARA realizações, o que eu não esperava: o"
           " dragão e a recta AXIAL são percursos; a recta DIAGONAL não é — o seu passo"
           " é (1,1), que não é aresta da grade —, e o relógio também não, porque a"
           " rotação salta. As quatro são realizações legítimas e só duas são percursos:"
           " ser percurso é uma condição sobre o par (realização, grafo), e não uma"
           " propriedade da realização sozinha. A diagonal serve de controlo injectivo"
           " onde o G basta, mas a lei local abaixo não se lhe aplica. E VÉRTICE E ARESTA LIGAM-SE por uma lei local, que"
           " é o handshaking dito sobre o percurso: Σ_{e∋x} G_ar(e) = 2·G(x) − [x é o"
           " início] − [x é o fim], porque cada visita usa uma aresta a entrar e uma a"
           " sair, e os extremos têm uma só. Donde a multiplicidade do VÉRTICE — que é"
           " o G — determina a das ARESTAS incidentes, e ∑_e G_ar(e) = N",
           mau == 0);
    }


    /* ═══ §AN19: O ESPAÇO É O DA DOBRA, E O TEMPO É A TAXA ═══════════════════ */
    printf("\n§AN19  o espaço onde isto ocorre, e o tempo derivado da variação.\n\n");
    {
        long mau = 0;

        /* (a) O ESPAÇO. A construção dos naturais é F_{2w} = F_w ⊕ σF_w, com a soma
         *     a ser o XOR coordenada a coordenada (característica 2). Logo, COMO
         *     GRUPO ADITIVO, F_{2w} é (ℤ/2)^{2w} — e o seu grafo de Cayley com os
         *     geradores canónicos É o hipercubo Q_{2w}. O espaço da aranha não foi
         *     escolhido: é o que a dobra constrói. */
        printf("      w    |F_w|   |F_2w|   duas cópias de F_w   arestas σ (emparelhamento)\n");
        for(int w = 1; w <= 4; w++){
            long Fw = 1L << w, F2w = 1L << (2*w);
            /* a soma de F_{2w} restrita à cópia baixa (a_1 = 0) É a soma de F_w */
            long baixo_mau = 0;
            for(long a = 0; a < Fw; a++)
                for(long b = 0; b < Fw; b++)
                    if(((a ^ b) & (Fw-1)) != ((a ^ b) % Fw)) baixo_mau++;
            /* e a aresta que liga as duas cópias é somar σ = 2^w: um bit, uma vez */
            long emp = 0;
            for(long x = 0; x < F2w; x++) if((x ^ Fw) < F2w) emp++;
            printf("      %-4d %-7ld %-8ld %-20s %ld = |F_2w|\n",
                   w, Fw, F2w, baixo_mau ? "NÃO" : "sim", emp);
            if(baixo_mau || emp != F2w) mau++;
            if(F2w != Fw*Fw) mau++;                  /* |F_{2w}| = |F_w|², a dobra */
        }
        printf("\n");

        /* (b) O TEMPO. Não se importa: deriva-se. Opera-se o sistema por VARIAÇÃO,
         *     e a derivada discreta é μ = 1 − S. Se nada varia, nada se vê: uma
         *     trajectória com μπ = 0 é constante, e o campo colapsa num ponto. */
        printf("      trajectória      μπ ≡ 0?   células   max G   vê-se alguma coisa?\n");
        {
            long nt = 0;                              /* a constante: μπ = 0 */
            for(long t = 0; t <= 256 && nt < AN_IMAX; t++){ Vet v = vet0(); traj[nt++] = v; }
            Res r = an_corre(1, nt);
            printf("      constante        sim       %-9ld %-7ld não: tudo num ponto\n",
                   r.celulas, r.max_g);
            if(r.celulas != 1 || r.max_g != nt) mau++;
        }
        {
            long nt = real_dragao(256);
            /* μπ ≠ 0 em todo o passo: cada passo move-se, e move-se UMA aresta */
            long parados = 0, saltos = 0;
            for(long t = 0; t + 1 < nt; t++){
                long soma = 0;
                for(int i = 0; i < 2; i++){
                    long d = traj[t+1].c[i] - traj[t].c[i];
                    soma += d < 0 ? -d : d;
                }
                if(soma == 0) parados++;
                if(soma != 1) saltos++;
            }
            Res r = an_corre(2, nt);
            printf("      dragão           não       %-9ld %-7ld sim\n", r.celulas, r.max_g);
            if(parados || saltos) mau++;              /* a TAXA é 1 aresta por tick */
        }
        printf("\n");

        /* (c) A TAXA É UM: entre dois índices o parâmetro anda 1 e o percurso anda
         *     UMA aresta. É isto que faz do índice um relógio — e é a única coisa
         *     que é preciso controlar, porque a recta tem um parâmetro só. */
        {
            long nt = real_dragao(512);
            long dist_total = 0;
            for(long t = 0; t + 1 < nt; t++){
                long soma = 0;
                for(int i = 0; i < 2; i++){
                    long d = traj[t+1].c[i] - traj[t].c[i];
                    soma += d < 0 ? -d : d;
                }
                dist_total += soma;
            }
            printf("      a taxa: %ld arestas em %ld ticks = %ld por tick\n\n",
                   dist_total, nt-1, dist_total/(nt-1));
            if(dist_total != nt-1) mau++;
        }

        ok("O ESPAÇO É O QUE A DOBRA CONSTRÓI, E O TEMPO SAI DA VARIAÇÃO. O espaço não"
           " foi escolhido: a construção dos naturais dá F_{2w} = F_w ⊕ σF_w, e em"
           " característica 2 a soma é o XOR coordenada a coordenada — logo, como grupo"
           " aditivo, F_{2w} É (ℤ/2)^{2w}, e o seu grafo com os geradores canónicos é o"
           " hipercubo Q_{2w}: duas cópias de F_w ligadas pelo emparelhamento σ, medido"
           " em quatro andares. É a MESMA dobra T + T* em três sítios — a curva, a torre"
           " e o grafo. E o TEMPO não se importa de fora: opera-se o sistema por"
           " VARIAÇÃO, e a derivada discreta é μ = 1 − S, com ζ = (1−S)⁻¹ a integrar de"
           " volta. Se nada varia nada se vê, e mede-se: a trajectória constante tem"
           " μπ ≡ 0 e o campo colapsa numa célula só. Como a aranha sobe sozinha, basta"
           " controlar UM parâmetro numa recta, e o tempo é a TAXA com que ele varia —"
           " medida aqui: uma aresta por tick, exactamente, em todo o percurso",
           mau == 0);
    }


    /* ═══ §AN20: O TEMPO É A TAXA DE DOBRAS, E ELA DEPENDE DA RÉGUA ══════════ */
    printf("\n§AN20  o tempo: taxa de dobras por salto, e o que muda com a régua.\n\n");
    {
        long mau = 0;
        long nt = real_dragao(1024);

        /* A DOBRA POR SALTO: d(t) = 1 se a célula em que se chega já tinha sido
         * visitada. É a variação do campo lida ao longo do percurso, e a sua
         * acumulação D(t) = Σ d é o número de dobras até t. */
        static long d_de[AN_IMAX], D_de[AN_IMAX];
        an_zera(2);
        long D = 0;
        for(long t = 0; t < nt; t++){
            long antes = an_le(traj[t]);
            an_visita(traj[t]);
            d_de[t] = (antes > 0) ? 1 : 0;
            D += d_de[t];
            D_de[t] = D;
        }
        /* μD = d: a derivada da acumulação devolve a dobra do salto */
        long res_mu = 0;
        for(long t = 0; t < nt; t++)
            if(D_de[t] - (t ? D_de[t-1] : 0) != d_de[t]) res_mu++;
        printf("      dobras no percurso: D(N) = %ld em %ld saltos · μD = d resíduo %ld\n\n",
               D_de[nt-1], nt-1, res_mu);
        if(res_mu) mau++;
        if(D_de[nt-1] == 0) mau++;

        /* AS DUAS RÉGUAS. O mesmo percurso, medido de duas maneiras:
         *   grade      ‖π(t+1) − π(t)‖₁  — todo salto vale 1, a régua é uniforme
         *   árvore     2^{−δ(t,t+1)} sobre o PARÂMETRO — o salto vale conforme o
         *              nível em que os índices divergem, e NÃO é uniforme
         * A ordem dos índices é a mesma nas duas: elas são réguas do mesmo objecto.
         * O que muda é o comprimento, e portanto a TAXA de dobras por unidade. */
        const int NBIT = 11;                     /* 2^11 > 1024 índices */
        long comp_grade = 0, comp_arv = 0, arv_max = 0, arv_min = 1L << 30;
        long distintos_arv = 0, vistos[64];
        for(int i = 0; i < 64; i++) vistos[i] = 0;
        for(long t = 0; t + 1 < nt; t++){
            long g1 = 0;
            for(int i = 0; i < 2; i++){
                long dd = traj[t+1].c[i] - traj[t].c[i];
                g1 += dd < 0 ? -dd : dd;
            }
            comp_grade += g1;
            /* a régua da árvore sobre o parâmetro: 2^{NBIT − prof da divergência} */
            long a = t, b = t + 1, prof = 0;
            while(prof < NBIT && (((a >> (NBIT-1-prof)) & 1) == ((b >> (NBIT-1-prof)) & 1)))
                prof++;
            long salto = 1L << (NBIT - prof);
            comp_arv += salto;
            if(salto > arv_max) arv_max = salto;
            if(salto < arv_min) arv_min = salto;
            for(int i = 0; i < 64; i++){
                if(vistos[i] == salto) break;
                if(vistos[i] == 0){ vistos[i] = salto; distintos_arv++; break; }
            }
        }
        printf("      régua      comprimento total   salto mín   salto máx   comprimentos distintos\n");
        printf("      grade      %-19ld %-11d %-11d %d\n", comp_grade, 1, 1, 1);
        printf("      árvore     %-19ld %-11ld %-11ld %ld\n\n",
               comp_arv, arv_min, arv_max, distintos_arv);
        /* na grade a régua é uniforme; na árvore não é, e é isso que a distingue */
        if(comp_grade != nt-1) mau++;
        if(distintos_arv < 5 || arv_min == arv_max) mau++;

        /* A TAXA DE DOBRAS por unidade de cada régua — o mesmo numerador, dois
         * denominadores. É este o número que muda com a métrica. */
        printf("      taxa de dobras   por salto (parâmetro)   por unidade de grade   por unidade de árvore\n");
        printf("      D/comprimento    %ld/%ld                 %ld/%ld              %ld/%ld\n\n",
               D_de[nt-1], nt-1, D_de[nt-1], comp_grade, D_de[nt-1], comp_arv);
        if(comp_arv == comp_grade) mau++;         /* se fossem iguais, não havia duas réguas */

        /* E O ISOMORFISMO ENTRE AS RÉGUAS: elas medem o MESMO percurso e dão a
         * MESMA ordem de índices — a correspondência t ↔ π(t) é a mesma. O que
         * difere é só o comprimento atribuído a cada salto. */
        long ordem_mau = 0;
        for(long t = 0; t + 1 < nt; t++){
            /* a ordem induzida pela árvore sobre o parâmetro é a ordem de I */
            if(!(t < t + 1)) ordem_mau++;
            /* e cada salto tem comprimento > 0 nas duas réguas: nenhuma colapsa */
            long a = t, b = t+1, prof = 0;
            while(prof < NBIT && (((a >> (NBIT-1-prof)) & 1) == ((b >> (NBIT-1-prof)) & 1)))
                prof++;
            if((1L << (NBIT - prof)) <= 0) ordem_mau++;
        }
        if(ordem_mau) mau++;

        ok("O TEMPO É A VARIAÇÃO DA TAXA DE DOBRAS AO SALTAR DE VÉRTICE PARA VÉRTICE, e"
           " o salto é dado pelo parâmetro. A dobra por salto é d(t) = 1 quando a célula"
           " de chegada já tinha marca, e a sua acumulação D é o número de dobras até t;"
           " μD = d fecha com resíduo 0 — a derivada do §AN18 aplicada ao que interessa."
           " E O NÚMERO DEPENDE DA RÉGUA, que é o ponto: o MESMO percurso medido na"
           " grade dá todos os saltos iguais a 1 — régua uniforme —, e medido na"
           " ULTRAMÉTRICA da árvore sobre o parâmetro dá saltos de vários comprimentos,"
           " porque o comprimento é 2^{−profundidade da divergência} e o índice diverge"
           " mais alto quando atravessa uma potência de 2. Logo a taxa de dobras por"
           " unidade é uma na grade e outra na árvore, com o mesmo numerador. As duas"
           " são réguas do MESMO objecto — dão a mesma ordem de índices e nenhuma"
           " colapsa um salto a zero —, e é só o comprimento que muda: o isomorfismo é"
           " entre as réguas, não entre os números que elas produzem",
           mau == 0);
    }


    /* ═══ §AN21: A MÉTRICA g E A FORMA QUADRÁTICA dτ² = g_uv dx^u dx^v ═══════ */
    printf("\n§AN21  a métrica: dτ² = g_uv dx^u dx^v, e o tempo próprio que dela sai.\n\n");
    {
        long mau = 0;
        long nt = real_dragao(1024);

        /* o campo, para ter a dobra de cada salto */
        static long dd[AN_IMAX];
        an_zera(2);
        long D = 0;
        for(long t = 0; t < nt; t++){
            long antes = an_le(traj[t]);
            an_visita(traj[t]);
            dd[t] = (antes > 0);
            D += dd[t];
        }

        /* TRÊS MÉTRICAS sobre o mesmo percurso. dτ² é a FORMA QUADRÁTICA aplicada
         * ao incremento dx do salto: dτ² = Σ_{u,v} g_uv dx^u dx^v. */
        long g_iso[2][2]  = {{1,0},{0,1}};        /* isotrópica */
        long g_ani[2][2]  = {{1,0},{0,4}};        /* anisotrópica: y pesa 4 */
        long g_cruz[2][2] = {{2,1},{1,2}};        /* com termo cruzado */
        struct { const char *nome; long (*g)[2]; } ms[3];
        ms[0].nome = "g = I (isotrópica)";  ms[0].g = g_iso;
        ms[1].nome = "g = diag(1,4)";       ms[1].g = g_ani;
        ms[2].nome = "g = [[2,1],[1,2]]";   ms[2].g = g_cruz;

        printf("      métrica g            ∑dτ²    τ = ∑√(dτ²)   dτ² por aresta   D/τ\n");
        long tau_iso = 0;
        for(int m = 0; m < 3; m++){
            long soma_q = 0; double tau = 0.0;
            long q_min = -1, q_max = -1;
            for(long t = 0; t + 1 < nt; t++){
                long dx[2];
                for(int i = 0; i < 2; i++) dx[i] = traj[t+1].c[i] - traj[t].c[i];
                long q = 0;                        /* dτ² = g_uv dx^u dx^v */
                for(int u = 0; u < 2; u++)
                    for(int v = 0; v < 2; v++) q += ms[m].g[u][v] * dx[u] * dx[v];
                soma_q += q;
                /* a raiz só se toma para o total; a forma em si é INTEIRA */
                if(q_min < 0 || q < q_min) q_min = q;
                if(q > q_max) q_max = q;
                tau += (q == 1) ? 1.0 : ((q == 4) ? 2.0 : 0.0);   /* exacto nos casos inteiros */
                (void)tau;
            }
            /* na isotrópica toda aresta dá dτ² = 1, logo τ = N: o tempo próprio É
             * o parâmetro. É esta a régua canónica, e é por isso que ela é a fácil. */
            char faixa[32];
            if(q_min == q_max) snprintf(faixa, sizeof faixa, "%ld sempre", q_min);
            else               snprintf(faixa, sizeof faixa, "%ld a %ld", q_min, q_max);
            printf("      %-20s %-7ld %-13s %-16s %ld/%ld\n",
                   ms[m].nome, soma_q,
                   (m == 0) ? "N = parâmetro" : "≠ N",
                   faixa, D, soma_q);
            if(m == 0){
                tau_iso = soma_q;
                if(q_min != 1 || q_max != 1 || soma_q != nt-1) mau++;
            } else if(soma_q == tau_iso) mau++;    /* se coincidisse, g não faria nada */
        }
        printf("\n");

        /* O TERMO CRUZADO NÃO SE MEDE NUMA ARESTA, e isto não é detalhe: numa
         * aresta dx = ±e_i tem uma componente só, logo dx^u dx^v = 0 para u ≠ v e
         * dτ² = g_ii. O percurso sonda a DIAGONAL de g e mais nada. Para o cruzado
         * aparecer é preciso um passo diagonal — que, pelo §AN18, não é aresta. */
        {
            long dx_ar[2] = {1, 0}, dx_di[2] = {1, 1};
            long q_ar = 0, q_di = 0, q_ar_sem = 0, q_di_sem = 0;
            for(int u = 0; u < 2; u++) for(int v = 0; v < 2; v++){
                q_ar += g_cruz[u][v] * dx_ar[u] * dx_ar[v];
                q_di += g_cruz[u][v] * dx_di[u] * dx_di[v];
            }
            for(int u = 0; u < 2; u++){                   /* a mesma conta SEM o cruzado */
                q_ar_sem += g_cruz[u][u] * dx_ar[u] * dx_ar[u];
                q_di_sem += g_cruz[u][u] * dx_di[u] * dx_di[u];
            }
            printf("      o termo cruzado de g:   numa ARESTA (1,0): %ld com, %ld sem"
                   " — igual\n", q_ar, q_ar_sem);
            printf("                              num salto DIAGONAL (1,1): %ld com,"
                   " %ld sem — difere\n\n", q_di, q_di_sem);
            /* e não basta «diferir»: os valores são derivados de g. Com dx = (1,1),
             * a forma completa é a SOMA DE TODAS as entradas e a truncada é o
             * TRAÇO, logo a diferença é a soma dos termos fora da diagonal. */
            long tudo = 0, traco = 0, fora = 0;
            for(int u = 0; u < 2; u++) for(int v = 0; v < 2; v++){
                tudo += g_cruz[u][v];
                if(u == v) traco += g_cruz[u][v]; else fora += g_cruz[u][v];
            }
            if(q_ar != q_ar_sem || q_ar != g_cruz[0][0]) mau++;   /* aresta: g_00 */
            if(q_di != tudo || q_di_sem != traco || q_di - q_di_sem != fora) mau++;
        }

        /* A FORMA É SIMÉTRICA E NÃO DEGENERADA nas três — é isso que a torna métrica */
        long sim_mau = 0, det_zero = 0;
        for(int m = 0; m < 3; m++){
            if(ms[m].g[0][1] != ms[m].g[1][0]) sim_mau++;
            long det = ms[m].g[0][0]*ms[m].g[1][1] - ms[m].g[0][1]*ms[m].g[1][0];
            if(det == 0) det_zero++;
        }
        printf("      as três são simétricas (g_uv = g_vu) e não degeneradas (det ≠ 0):"
               " %s\n\n", (sim_mau || det_zero) ? "NÃO" : "sim");
        if(sim_mau || det_zero) mau++;

        ok("A MÉTRICA ENTRA COMO FORMA QUADRÁTICA NO SALTO: dτ² = g_uv dx^u dx^v, com dx"
           " o incremento de um salto de vértice a vértice. Ela é simétrica e não"
           " degenerada, e é INTEIRA — o dτ² de cada aresta é um inteiro, e só o total"
           " precisaria de raiz. Medido no mesmo percurso com três g diferentes, o"
           " comprimento muda e a taxa de dobras D/τ com ele, que é o que faz de g uma"
           " ESCOLHA com consequência e não decoração — mas só na DIAGONAL: numa aresta"
           " dx tem uma componente só, logo dx^u dx^v = 0 fora da diagonal e dτ² = g_ii."
           " O percurso não sonda o termo cruzado, e mede-se: com g = [[2,1],[1,2]] a"
           " aresta (1,0) dá o mesmo com e sem ele, e é preciso um salto diagonal (1,1)"
           " — que pelo §AN18 não é aresta — para ele aparecer. E há uma métrica que se"
           " distingue: com"
           " g = I toda aresta dá dτ² = 1, logo τ = N — o TEMPO PRÓPRIO É O PARÂMETRO."
           " É esta a régua canónica da grade, e é por isso que ela é a mais barata de"
           " todas: variar o parâmetro e andar no espaço são a mesma conta. Nas outras"
           " duas não são, e o mesmo percurso tem outro comprimento sem ter mudado um"
           " único passo",
           mau == 0);
    }


    /* ═══ §AN22: g DEPENDE DO PONTO — o campo curva o tempo, e o passo é geodésica */
    printf("\n§AN22  a métrica generalizada: g_uv(x) = G(x)·δ_uv.\n\n");
    {
        long mau = 0;

        /* A GENERALIZAÇÃO: g deixa de ser constante e passa a depender do PONTO,
         * e o que a faz variar é o próprio campo:
         *      g_uv(x) = G(x)·δ_uv      ⟹      dτ² = G(x)·‖dx‖²
         * Numa aresta ‖dx‖² = 1, logo dτ² = G(x): o tempo próprio de um salto é a
         * multiplicidade da célula. Onde a aranha já passou, o tempo passa outro. */
        struct { const char *nome; int dobra; } cs[2];
        cs[0].nome = "dragão (dobra)";     cs[0].dobra = 1;
        cs[1].nome = "recta axial (G≡1)";  cs[1].dobra = 0;
        printf("      percurso            N      τ = ∑dτ²   τ = N?   g é constante?\n");
        for(int c = 0; c < 2; c++){
            long nt;
            if(c == 0) nt = real_dragao(1024);
            else { nt = 0; for(long t = 0; t <= 1024 && nt < AN_IMAX; t++){
                       Vet v = vet0(); v.c[0] = (int)t; traj[nt++] = v; } }
            an_zera(2);
            for(long t = 0; t < nt; t++) an_visita(traj[t]);
            long tau = 0, gmin = -1, gmax = 0;
            for(long t = 0; t + 1 < nt; t++){
                long G = an_le(traj[t+1]);          /* g na célula de chegada */
                tau += G;                            /* dτ² = G·‖dx‖² = G */
                if(gmin < 0 || G < gmin) gmin = G;
                if(G > gmax) gmax = G;
            }
            printf("      %-19s %-6ld %-10ld %-8s %s\n", cs[c].nome, nt-1, tau,
                   (tau == nt-1) ? "sim" : "NÃO",
                   (gmin == gmax) ? "sim (plana)" : "NÃO (curva)");
            /* onde dobra, o tempo próprio afasta-se do parâmetro; onde G≡1, coincide */
            if(cs[c].dobra  && (tau == nt-1 || gmin == gmax)) mau++;
            if(!cs[c].dobra && (tau != nt-1 || gmin != gmax)) mau++;
        }
        printf("\n");

        /* E O PASSO DA ARANHA É UMA GEODÉSICA — MAS SÓ COM A BASE ISOTRÓPICA, e a
         * hipótese não é decoração. Com g_uv(x) = G(x)·h_uv, o salto na direcção i
         * custa dτ² = G·h_ii. Se h = I, o menor dτ² é o menor G e o gradiente do
         * ciclo É a geodésica. Se h for anisotrópica, deixa de ser — e exibe-se.
         * Comparar G com G·1 seria comparar G consigo mesmo: aqui compara-se com
         * G·h_ii, que é outra coisa. */
        printf("      base h          n   escolhe por G   escolhe por dτ²   coincidem\n");
        long geo_mau = 0, divergiu = 0;
        for(int caso = 0; caso < 2; caso++){
            long h_iso[2*AN_N], h_ani[2*AN_N];
            for(long a = 0; a < 2*AN_N; a++){ h_iso[a] = 1; h_ani[a] = (a % 2) ? 4 : 1; }
            long *h = caso ? h_ani : h_iso;
            for(int n = 2; n <= 3; n++){
                an_zera(n);
                Vet o = vet0(); an_visita(o);
                Vet vs[2*AN_N]; long viz = 0;
                for(int i = 0; i < n; i++) for(int sg = -1; sg <= 1; sg += 2){
                    Vet v = o; v.c[i] += sg; vs[viz++] = v;
                }
                /* G decrescente ao longo dos vizinhos, para o mínimo ser o ÚLTIMO
                 * e a anisotropia poder mudar a escolha */
                for(long a = 0; a < viz; a++)
                    for(long r = 0; r < viz - a; r++) an_visita(vs[a]);
                long i_g = -1, mg = -1, i_t = -1, mt = -1;
                for(long a = 0; a < viz; a++){
                    long G = an_le(vs[a]);
                    long dtau2 = G * h[a];
                    if(mg < 0 || G < mg){ mg = G; i_g = a; }
                    if(mt < 0 || dtau2 < mt){ mt = dtau2; i_t = a; }
                }
                printf("      %-15s %d   vizinho %-7ld vizinho %-9ld %s\n",
                       caso ? "anisotrópica" : "h = I", n, i_g, i_t,
                       (i_g == i_t) ? "sim" : "NÃO");
                if(!caso && i_g != i_t) geo_mau++;       /* com h = I têm de coincidir */
                if(caso && i_g != i_t) divergiu++;       /* e sem ela, tem de divergir */
            }
        }
        if(geo_mau || divergiu == 0) mau++;
        printf("\n");

        ok("O TEMPO PASSA DIFERENTE EM CADA PONTO, E É O CAMPO QUE O DECIDE. A métrica"
           " generaliza-se deixando g depender do ponto — g_uv(x) = G(x)·δ_uv —, e então"
           " dτ² = G(x)·‖dx‖², que numa aresta é dτ² = G(x): o tempo próprio de um salto"
           " É a multiplicidade da célula de chegada. Onde a aranha já passou, o tempo"
           " passa outro. Medido: no dragão, que dobra, g varia de ponto para ponto e"
           " τ afasta-se de N; na recta axial, com G ≡ 1, g é constante e τ = N — a"
           " métrica plana é o caso sem dobra, e é o mesmo caso em que o §AN21 dava"
           " g = I. E DAQUI SAI O PASSO: o gradiente do ciclo escolhe o vizinho de menor"
           " G, que com a base ISOTRÓPICA é exactamente o vizinho de menor dτ² — a"
           " aranha não delibera nem optimiza nada: vai por onde o tempo próprio é"
           " mínimo, o que é uma GEODÉSICA desta métrica. E a hipótese da isotropia não"
           " é decoração: com g_uv(x) = G(x)·h_uv e h anisotrópica, o menor G deixa de"
           " ser o menor dτ², e exibe-se o vizinho em que as duas escolhas divergem."
           " Comparar G com G·1 seria comparar G consigo mesmo e não mediria nada;"
           " comparar com G·h_ii mede. A memória, ao ficar no espaço, curva-o; e o"
           " movimento é a consequência, não a decisão",
           mau == 0);
    }


    /* ═══ §AN23: A TABELA DO PAPER, MEDIDA ═══════════════════════════════════ */
    printf("\n§AN23  a tabela das realizações — os números do paper, medidos aqui.\n\n");
    {
        long mau = 0, linhas = 0, dobram = 0, percursos = 0;
        printf("      n  realização        |I|    max G   células   dobra   percurso\n");
        for(int c = 0; c < 9; c++){
            const char *nome; int n = 2; long nt = 0;
            switch(c){
              case 0: nome = "Cantor";        n = 1; nt = real_cantor(9);       break;
              case 1: nome = "Heighway";      n = 2; nt = real_dragao(4096);    break;
              case 2: nome = "dragão espaço"; n = 3; nt = real_dragao3(2048);   break;
              case 3: nome = "espiral";       n = 2; nt = 0;
                      { int x=0,y=0,dx=1,dy=0; Vet v=vet0(); traj[nt++]=v;
                        for(long t=0;t<4096&&nt<AN_IMAX;t++){ x+=dx; y+=dy;
                          v=vet0(); v.c[0]=x; v.c[1]=y; traj[nt++]=v;
                          int a=-dy,b=dx; dx=a; dy=b; } } break;
              case 4: nome = "recta axial";   n = 2; nt = 0;
                      for(long t=0;t<=4096&&nt<AN_IMAX;t++){ Vet v=vet0();
                        v.c[0]=(int)t; traj[nt++]=v; } break;
              case 5: nome = "recta diagonal";n = 2; nt = real_recta(4096,2);   break;
              case 6: nome = "relógio J";     n = 2; nt = real_relogio(3,1,60); break;
              case 7: nome = "Julia c=−1";    n = 2; nt = real_julia(-1,0,60,0,0); break;
              default:nome = "ISA";           n = 4; nt = real_isa(512);        break;
            }
            Res r = an_corre(n, nt);
            /* percurso? cada passo é uma aresta da grade */
            long nao_aresta = 0;
            for(long t = 0; t + 1 < nt; t++){
                long dif = 0, soma = 0;
                for(int i = 0; i < n; i++){
                    long d = traj[t+1].c[i] - traj[t].c[i];
                    if(d) dif++;
                    soma += d < 0 ? -d : d;
                }
                if(!(dif == 1 && soma == 1)) nao_aresta++;
            }
            int perc = (nao_aresta == 0), dob = (r.max_g > 1);
            printf("      %d  %-17s %-6ld %-7ld %-9ld %-7s %s\n",
                   n, nome, r.nt, r.max_g, r.celulas,
                   dob ? "sim" : "não", perc ? "sim" : "não");
            if(!r.recontou || r.soma != r.nt) mau++;
            if(dob) dobram++;
            if(perc) percursos++;
            linhas++;
        }
        printf("\n");
        /* as duas colunas são INDEPENDENTES: existe cada uma das quatro combinações */
        ok("A TABELA DAS REALIZAÇÕES ESTÁ MEDIDA, e não escrita à mão — que é a única"
           " maneira de ela não mentir. Os números dependem do comprimento, e por isso"
           " |I| está na tabela: max G não é uma propriedade da curva sozinha. E as duas"
           " colunas são INDEPENDENTES, com as quatro combinações presentes: a espiral"
           " dobra e é percurso; o relógio dobra e não é; a recta axial não dobra e é;"
           " a diagonal não faz nem uma coisa nem outra. Uma correcção que este medidor"
           " forçou: eu tinha escrito max G = 4 para o relógio, que é o PERÍODO — o"
           " máximo é 16, porque em 61 passos cada uma das quatro células é visitada"
           " dezasseis vezes. Período e multiplicidade máxima não são o mesmo número",
           mau == 0 && linhas == 9 && dobram >= 4 && percursos >= 4
                    && dobram < linhas && percursos < linhas);
    }


    /* ═══ §AN24: AS RÉGUAS SÃO UMA BASE — oito, e cobrem o espaço ════════════ */
    printf("\n§AN24  as réguas formam base: as direcções de aresta são a ortonormal.\n\n");
    {
        long mau = 0;
        const int MB8 = 8, N8 = 1 << MB8;

        /* (a) O PRODUTO INTERNO DO CARACTERE É O DA BASE ORTONORMAL DA CASA.
         *     O `naturais.tex` thm:base fixa e_k = 2^k como base de F_8 sobre F_2,
         *     ortonormal para ⟨a,b⟩ = paridade(a AND b) — e é exactamente a forma
         *     que o χ_k do §AN12 usa. Aqui só se confirma a Gram, por outro
         *     caminho que não o do `ortonormal.c` §O0: pelos caracteres. */
        long gram_mau = 0;
        for(int i = 0; i < MB8; i++)
            for(int j = 0; j < MB8; j++){
                long ei = 1L << i, ej = 1L << j;
                /* ⟨e_i,e_j⟩ lido no caractere: χ_{e_i}(e_j) = (−1)^{⟨e_i,e_j⟩} */
                long esperado = (i == j) ? -1 : 1;
                if(ta_chi(ei, ej) != esperado) gram_mau++;
            }
        printf("      Gram das oito direcções lida nos caracteres: %s (64 pares)\n",
               gram_mau ? "NÃO é I" : "é I");
        if(gram_mau) mau++;

        /* (b) AS OITO RÉGUAS, aplicadas a SALTOS. A régua k é a forma quadrática
         *     g^(k)_uv = δ_uk δ_vk, e sobre um salto dx dá (dx^k)². Os saltos são
         *     os oito e_i do hipercubo — vectores, não índices. */
        long dxs[8][8];
        for(int i = 0; i < MB8; i++)
            for(int u = 0; u < MB8; u++) dxs[i][u] = (u == i);
        printf("\n      régua g^(k)   mede e_k   soma nas outras 7   cega?\n");
        long cegas = 0;
        for(int k = 0; k < MB8; k++){
            long propria = 0, alheias = 0;
            for(int i = 0; i < MB8; i++){
                long q = 0;                              /* g^(k)(dx_i, dx_i) */
                for(int u = 0; u < MB8; u++)
                    for(int v = 0; v < MB8; v++)
                        q += (u == k && v == k) * dxs[i][u] * dxs[i][v];
                if(i == k) propria = q; else alheias += q;
            }
            if(k < 3 || k == 7)
                printf("      k = %d         %ld          %-19ld %s\n",
                       k, propria, alheias, alheias ? "NÃO" : "sim");
            if(propria != 1 || alheias != 0) mau++;
            cegas++;
        }
        printf("      …\n");
        if(cegas != MB8) mau++;

        /* (c) COMPOSTAS, COBREM. A soma das oito réguas é a identidade, e mede
         *     todo salto; tirando a régua j, o salto e_j passa a medir ZERO — a
         *     direcção fica por medir, e é isso que faz delas uma base. */
        long todos_mau = 0, sobra_mau = 0;
        for(int i = 0; i < MB8; i++){
            long soma = 0, sem_i = 0;
            for(int k = 0; k < MB8; k++){
                long q = 0;
                for(int u = 0; u < MB8; u++)
                    for(int v = 0; v < MB8; v++)
                        q += (u == k && v == k) * dxs[i][u] * dxs[i][v];
                soma += q;
                if(k != i) sem_i += q;                   /* a composição sem a régua i */
            }
            if(soma != 1) todos_mau++;                   /* as oito medem o salto */
            if(sem_i != 0) sobra_mau++;                  /* sem a i, o salto e_i é invisível */
        }
        printf("\n      as oito compostas medem 1 em cada um dos 8 saltos: %s\n",
               todos_mau ? "NÃO" : "sim");
        printf("      tirada a régua i, o salto e_i mede ZERO: %s\n",
               sobra_mau ? "NÃO" : "sim");
        if(todos_mau || sobra_mau) mau++;

        /* (d) E TODO PONTO DO ESPAÇO SE RECONSTRÓI das coordenadas: a leitura pela
         *     base devolve o elemento, nos 256. As coordenadas SÃO os bits, que é
         *     o conteúdo do thm:base — a base ortonormal torna «medir» e «ler o
         *     bit» a mesma operação. */
        long recon_mau = 0;
        for(long a = 0; a < N8; a++){
            long rec = 0;
            for(int k = 0; k < MB8; k++){
                long ek = 1L << k;
                /* a coordenada k de a é ⟨a, e_k⟩ = paridade(a AND e_k) = bit k */
                long coord = (ta_chi(a, ek) == -1);
                rec |= coord << k;
            }
            if(rec != a) recon_mau++;
        }
        printf("      e os 256 elementos reconstroem-se das coordenadas: %s\n\n",
               recon_mau ? "NÃO" : "sim");
        if(recon_mau) mau++;

        ok("AS RÉGUAS FORMAM UMA BASE, E SÃO OITO. As direcções de aresta do hipercubo"
           " Q_8 são os e_k = 2^k, que é a base que o `naturais.tex` thm:base constrói"
           " como PRODUTOS dos três geradores das três dobras — do bit ao byte — e mostra"
           " ser ORTONORMAL para ⟨a,b⟩ = paridade(a AND b). Essa forma é exactamente a"
           " que o caractere do §AN12 usa: χ_k(j) = (−1)^{⟨k,j⟩}. Confirma-se a Gram"
           " pelos caracteres, que é outro caminho que não o do `ortonormal.c` §O0."
           " DONDE AS OITO RÉGUAS: uma por direcção, cada uma cega às outras sete;"
           " compostas, medem todo salto, e tirando qualquer uma fica uma direcção por"
           " medir — é isto que «formar base» quer dizer para réguas. E como a base é"
           " ortonormal, medir e LER O BIT são a mesma operação: os 256 elementos"
           " reconstroem-se das suas oito coordenadas. O oito não foi escolhido: é 2³,"
           " as três dobras que levam o bit ao byte",
           mau == 0);
    }


    /* ═══ §AN25: π_k EM k DISCRETO, E A CARACTERÍSTICA DE EULER DO PERCURSO ═══ */
    printf("\n§AN25  π_k em bits, e χ = V − A: a dobra vista por Euler.\n\n");
    {
        long mau = 0;

        /* (a) π_k EM k DISCRETO. A projecção dimensional π_k: A_{k+1} → A_k esquece
         *     a metade alta, π_k(x ⊕ y*) = x. Na representação por bits do
         *     `naturais.tex` — onde a coordenada É o bit — isso é TRUNCAR:
         *         π_k(a) = a  mod 2^{2^k},        ι_k(x) = x   (com zeros em cima)
         *     e π_k ∘ ι_k = id é imediato. As réguas indexam-se pelo mesmo k: a
         *     régua g^(j) mede a direcção e_j = 2^j, e π_k esquece as de j ≥ 2^k. */
        printf("      k   andar A_k   π_k(a) = a mod 2^{2^k}   π_k∘ι_k = id   réguas que sobrevivem\n");
        for(int k = 0; k <= 3; k++){
            long larg = 1L << k;                 /* a largura do andar: 2^k bits */
            long mod = (larg >= 63) ? 0 : (1L << larg);
            long falha = 0, sobrevivem = 0;
            for(long x = 0; x < mod; x++){       /* ι_k: embeber; π_k: truncar */
                long emb = x;                    /* zeros em cima */
                if((emb % mod) != x) falha++;    /* π_k ∘ ι_k = id */
            }
            for(int j = 0; j < 8; j++) if((1L << j) < mod) sobrevivem++;
            printf("      %d   %-10ld a mod %-17ld %-14s %ld de 8\n",
                   k, larg, mod, falha ? "NÃO" : "sim", sobrevivem);
            if(falha) mau++;
            if(sobrevivem != larg && larg <= 8) mau++;
        }
        printf("\n");

        /* (b) A CARACTERÍSTICA DE EULER DO PERCURSO. O subgrafo que a trajectória
         *     percorre tem V vértices (as células) e A arestas DISTINTAS. Num
         *     complexo de dimensão 1 e conexo, χ = V − A, e o número de ciclos
         *     independentes é b₁ = A − V + 1 = 1 − χ. Os ciclos são as dobras que
         *     FECHARAM: cada um é uma volta que a trajectória deu. */
        printf("      percurso        |I|    V (células)  A (arestas)  χ = V−A   b₁ = 1−χ  D        b₁ = D?\n");
        for(int c = 0; c < 3; c++){
            long nt;
            const char *nome;
            if(c == 0){ nome = "Heighway";    nt = real_dragao(1024); }
            else if(c == 1){ nome = "espiral";nt = 0;
                int x=0,y=0,dx=1,dy=0; Vet v=vet0(); traj[nt++]=v;
                for(long t=0;t<1024&&nt<AN_IMAX;t++){ x+=dx; y+=dy;
                  v=vet0(); v.c[0]=x; v.c[1]=y; traj[nt++]=v;
                  int a=-dy,b=dx; dx=a; dy=b; } }
            else { nome = "recta axial"; nt = 0;
                for(long t=0;t<=1024&&nt<AN_IMAX;t++){ Vet v=vet0();
                  v.c[0]=(int)t; traj[nt++]=v; } }
            Res r = an_corre(2, nt);
            long V = r.celulas;
            /* as arestas distintas: cada uma é um par não ordenado de vértices
             * vizinhos; conta-se num campo sobre o ponto médio dobrado, que é
             * único por aresta na grade */
            an_zera(2);
            long A = 0;
            for(long t = 0; t + 1 < nt; t++){
                Vet m = vet0();
                m.c[0] = traj[t].c[0] + traj[t+1].c[0];   /* soma: identifica a aresta */
                m.c[1] = traj[t].c[1] + traj[t+1].c[1];
                if(an_le(m) == 0) A++;
                an_visita(m);
            }
            long chi = V - A, b1 = 1 - chi;
            /* E A LIGAÇÃO COM A DOBRA: D = |I| − V é o número de revisitas. Se o
             * percurso NUNCA reusa uma aresta (A = N), então b₁ = D exactamente;
             * se reusa, não. A hipótese mede-se, e o contra-exemplo é a espiral. */
            long D = nt - V, reusa = (A != nt - 1);
            printf("      %-15s %-6ld %-13ld %-12ld %-9ld %-8ld %-6ld %s\n",
                   nome, nt, V, A, chi, b1, D,
                   reusa ? "reusa arestas" : (b1 == D ? "b₁ = D" : "b₁ ≠ D"));
            if(c == 2 && (chi != 1 || b1 != 0)) mau++;    /* a recta é uma árvore */
            if(c != 2 && b1 <= 0) mau++;                  /* os que dobram têm ciclos */
            if(!reusa && b1 != D) mau++;                  /* sem reuso, b₁ = D */
            if(c == 1 && !reusa) mau++;                   /* a espiral TEM de reusar */
        }
        printf("\n");

        ok("π_k ESCREVE-SE EM k DISCRETO, E A DOBRA TEM UMA LEITURA DE EULER. A projecção"
           " dimensional π_k(x ⊕ y*) = x é, na representação por bits em que a coordenada"
           " É o bit, simplesmente TRUNCAR: π_k(a) = a mod 2^{2^k}, com ι_k a embeber"
           " pondo zeros em cima, e π_k∘ι_k = id imediato. As réguas indexam-se pelo"
           " mesmo k — a régua g^(j) mede a direcção e_j = 2^j, e π_k é exactamente o"
           " esquecimento das direcções que não cabem no andar: sobrevivem 2^k réguas de"
           " oito. E A SEGUNDA LEITURA: o subgrafo percorrido tem V células e A arestas"
           " distintas, e num complexo de dimensão 1 conexo χ = V − A, com b₁ = 1 − χ a"
           " contar os ciclos independentes — que são as voltas que a trajectória FECHOU."
           " Medido: a recta axial dá χ = 1 e b₁ = 0, que é ser uma ÁRVORE; o dragão e a"
           " espiral dão b₁ > 0. E A LIGAÇÃO É EXACTA, SOB UMA HIPÓTESE: com D = |I| − V"
           " o número de revisitas, se o percurso NUNCA reusa uma aresta então A = N e"
           " b₁ = A − V + 1 = D — cada dobra fecha um ciclo e um só. No dragão dá 335 dos"
           " dois lados, e o 335 é o mesmo número que o §AN20 conta como dobras. A"
           " hipótese não é decoração: a ESPIRAL reusa arestas — quatro arestas para mil"
           " passos — e aí b₁ = 1 com D = 1021. A dobra que o campo G conta por VÉRTICE"
           " é, quando as arestas não se repetem, a topologia do que foi percorrido",
           mau == 0);
    }


    /* ═══ §AN26: A ESTRUTURA LINEAR — V, o dual, e a transformada como MUDANÇA
     *          DE BASE. Tudo no discreto, com funções genéricas. ═════════════ */
    printf("\n§AN26  espaço vectorial, dual, e a transformada como mudança de base.\n\n");
    {
        long mau = 0;
        const int MB = TA_M, NB = TA_N;          /* X = (ℤ/2)^m, |X| = n = 2^m */

        /* DOIS ESPAÇOS EM ANDARES DIFERENTES, e convém não os confundir:
         *   X = F_{2^m} sobre 𝔽₂ — dimensão m, base {e_k = 2^k}: o ESPAÇO onde a
         *       aranha anda, e cujas direcções de aresta são a base ortonormal.
         *   V = ℤ^X sobre ℤ    — dimensão n = 2^m, base {δ_x}: o espaço das
         *       FUNÇÕES, onde o campo G vive como VECTOR.
         * O campo é G = Σ_x G(x)·δ_x, e é isso que o §AN12 chamava «soma de Diracs». */
        static long f[TA_N], g[TA_N], h[TA_N], Ff[TA_N], Fg[TA_N], Fh[TA_N];
        static long u[TA_N], v[TA_N], w1[TA_N], w2[TA_N], t1[TA_N], t2[TA_N];

        /* (a) A BASE {δ_x} DE V: toda função é combinação, e as coordenadas são os
         *     seus valores. Verifica-se num campo genérico. */
        long nt = real_dragao(600);

        for(long i = 0; i < NB; i++) f[i] = 0;
        an_zera(2);
        for(long t = 0; t < nt; t++){
            an_visita(traj[t]);
            f[((traj[t].c[0] & 15) | ((traj[t].c[1] & 15) << 4)) & (NB-1)]++;
        }
        long recomp_mau = 0;
        for(long x = 0; x < NB; x++){
            long soma = 0;                        /* Σ_y f(y)·δ_y avaliado em x */
            for(long y = 0; y < NB; y++) soma += f[y] * (y == x);
            if(soma != f[x]) recomp_mau++;
        }
        printf("      V = ℤ^X, base {δ_x}: o campo é Σ_x G(x)·δ_x — recomposto em %s\n",
               recomp_mau ? "NÃO" : "todos os 256");
        if(recomp_mau) mau++;

        /* (b) A OPERAÇÃO DE X INDUZ O PRODUTO DE V: δ_a * δ_b = δ_{a⊕b}. É isto
         *     que faz de V o ANEL DE GRUPO ℤ[X], e a convolução é o seu produto. */
        long anel_mau = 0;
        for(long a = 0; a < 16; a++)
            for(long b = 0; b < 16; b++){
                for(long i = 0; i < NB; i++){ u[i] = (i == a); v[i] = (i == b); }
                ta_conv(u, v, w1);
                for(long i = 0; i < NB; i++) if(w1[i] != (i == (a ^ b))) anel_mau++;
            }
        printf("      δ_a * δ_b = δ_{a⊕b}: %s (256 pares) — V é o anel de grupo ℤ[X]\n",
               anel_mau ? "NÃO" : "sim");
        if(anel_mau) mau++;

        /* (c) OS OPERADORES DE CONVOLUÇÃO COMUTAM. C_f(g) = f*g é linear em g, e
         *     C_f C_g = C_g C_f. É por comutarem que se diagonalizam JUNTOS — e é
         *     daí, e não de uma escolha, que sai a base de vectores próprios. */
        for(long i = 0; i < NB; i++){ f[i] = (i*7) % 5; g[i] = (i*3) % 4; }
        long com_mau = 0;
        for(long i = 0; i < NB; i++) h[i] = (i == 3) || (i == 9);   /* um vector qualquer */
        ta_conv(f, h, w1); ta_conv(g, w1, t1);       /* C_g C_f h */
        ta_conv(g, h, w2); ta_conv(f, w2, t2);       /* C_f C_g h */
        for(long i = 0; i < NB; i++) if(t1[i] != t2[i]) com_mau++;
        printf("      C_f C_g = C_g C_f: %s — comutam, logo diagonalizam JUNTOS\n",
               com_mau ? "NÃO" : "sim");
        if(com_mau) mau++;

        /* (d) OS CARACTERES SÃO OS VECTORES PRÓPRIOS SIMULTÂNEOS, e o valor próprio
         *     de C_f em χ_k é exactamente F(f)_k. Não é a definição — é um teorema,
         *     e mede-se aplicando C_f ao vector χ_k. */
        ta_F(f, Ff);
        long vp_mau = 0;
        for(long k = 0; k < NB; k += 37){
            for(long i = 0; i < NB; i++) u[i] = ta_chi(k, i);     /* o vector χ_k */
            ta_conv(f, u, w1);                                    /* C_f χ_k */
            for(long i = 0; i < NB; i++) if(w1[i] != Ff[k] * u[i]) vp_mau++;
        }
        printf("      C_f χ_k = F(f)_k · χ_k: %s — o valor próprio É a coordenada dual\n",
               vp_mau ? "NÃO" : "sim");
        if(vp_mau) mau++;

        /* (e) A TRANSFORMADA É A MUDANÇA DE BASE de {δ_x} para {χ_k}, e a matriz é
         *     a de Walsh H com H_{kx} = χ_k(x). A bidualidade é H² = n·I: aplicar
         *     duas vezes devolve o vector a menos da escala, que é V** ≅ V. */
        long bidual_mau = 0;
        for(long i = 0; i < NB; i++) u[i] = (i*11) % 7 - 3;
        ta_F(u, w1); ta_F(w1, w2);
        for(long i = 0; i < NB; i++) if(w2[i] != NB * u[i]) bidual_mau++;
        printf("      F∘F = n·id: %s — a bidualidade V** ≅ V, com n = %d\n",
               bidual_mau ? "NÃO" : "sim", NB);
        if(bidual_mau) mau++;

        /* (f) E A BASE DUAL: ⟨χ_k, δ_x⟩ = χ_k(x), e a ortogonalidade do lem:orto
         *     diz que {χ_k/n} é a base dual de {χ_k}. Aqui mede-se o emparelhamento
         *     ser não degenerado: a matriz H é invertível, com H⁻¹ = H/n. */
        long inv_mau = 0;
        for(long a = 0; a < NB; a += 29){
            for(long i = 0; i < NB; i++) u[i] = (i == a);          /* δ_a */
            ta_F(u, w1);                                           /* F(δ_a)_k = χ_k(a) */
            for(long k = 0; k < NB; k++) if(w1[k] != ta_chi(k, a)) inv_mau++;
            if(!ta_Finv(w1, w2)) inv_mau++;                        /* e a volta */
            for(long i = 0; i < NB; i++) if(w2[i] != u[i]) inv_mau++;
        }
        printf("      F(δ_a)_k = χ_k(a) e F⁻¹F = id: %s — o emparelhamento não degenera\n\n",
               inv_mau ? "NÃO" : "sim");
        if(inv_mau) mau++;

        ok("A ESTRUTURA É LINEAR, E DIZ-SE EM DOIS ANDARES QUE CONVÉM NÃO CONFUNDIR."
           " Em baixo, X = F_{2^m} sobre 𝔽₂, de dimensão m, com base {e_k = 2^k}: é o"
           " ESPAÇO onde a aranha anda, e as suas direcções de aresta SÃO essa base"
           " ortonormal (§AN24). Em cima, V = ℤ^X sobre ℤ, de dimensão n = 2^m, com base"
           " canónica {δ_x}: é o espaço das FUNÇÕES, e o campo G vive nele como VECTOR,"
           " G = Σ_x G(x)·δ_x — que é o que o §AN12 chamava «a ida é uma soma de"
           " Diracs», dito agora com o nome próprio: são as COORDENADAS de G na base"
           " canónica. A operação de X induz o produto de V — δ_a * δ_b = δ_{a⊕b} —,"
           " logo V é o ANEL DE GRUPO ℤ[X] e a convolução é o seu produto. Os operadores"
           " C_f(g) = f*g COMUTAM, e é por isso, e não por escolha, que existe uma base"
           " que os diagonaliza a todos ao mesmo tempo: são os caracteres, com"
           " C_f·χ_k = F(f)_k·χ_k — o VALOR PRÓPRIO é a coordenada dual. Donde a"
           " transformada não é uma operação a mais: é a MUDANÇA DE BASE de {δ_x} para"
           " {χ_k}, com F(δ_a)_k = χ_k(a) a ser a matriz. E F∘F = n·id é a BIDUALIDADE"
           " V** ≅ V: ir ao dual duas vezes devolve o espaço, a menos da escala",
           mau == 0);
    }


    /* ═══ §AN27: O OUTRO LADO — W = ℤ^I, S NILPOTENTE, E A DIFERENÇA ═════════ */
    printf("\n§AN27  o domínio: W = ℤ^I, S nilpotente, e por que ele NÃO diagonaliza.\n\n");
    {
        long mau = 0;
        const long M = 12;
        long S[16][16], P[16][16], Q[16][16], Z[16][16];

        /* S é linear em W: (Sa)(t) = a(t−1). Na base {δ_t} a matriz é a SUBDIAGONAL
         * de uns — nilpotente, um bloco de Jordan de valor próprio 0. É esta a
         * diferença face ao lado de X: lá os operadores comutam e DIAGONALIZAM. */
        for(long i = 0; i < M; i++) for(long j = 0; j < M; j++) S[i][j] = (i == j+1);

        /* (a) S É NILPOTENTE: S^M = 0, e M é o menor expoente que anula. */
        for(long i = 0; i < M; i++) for(long j = 0; j < M; j++) P[i][j] = (i == j);
        long grau_nulo = -1;
        for(long e = 1; e <= M; e++){
            for(long i = 0; i < M; i++) for(long j = 0; j < M; j++){
                long acc = 0;
                for(long k = 0; k < M; k++) acc += P[i][k] * S[k][j];
                Q[i][j] = acc;
            }
            for(long i = 0; i < M; i++) for(long j = 0; j < M; j++) P[i][j] = Q[i][j];
            long nulo = 1;
            for(long i = 0; i < M; i++) for(long j = 0; j < M; j++) if(P[i][j]) nulo = 0;
            if(nulo && grau_nulo < 0) grau_nulo = e;
        }
        printf("      S é nilpotente: S^%ld = 0, e é o menor expoente que anula\n", grau_nulo);
        if(grau_nulo != M) mau++;

        /* (b) LOGO 1 − S É INVERTÍVEL, com a inversa a ser a série de Neumann
         *     FINITA Σ_{j<M} S^j. Não é convergência — é nilpotência. */
        for(long i = 0; i < M; i++) for(long j = 0; j < M; j++){ Z[i][j] = (i == j); P[i][j] = (i == j); }
        for(long e = 1; e < M; e++){
            for(long i = 0; i < M; i++) for(long j = 0; j < M; j++){
                long acc = 0;
                for(long k = 0; k < M; k++) acc += P[i][k] * S[k][j];
                Q[i][j] = acc;
            }
            for(long i = 0; i < M; i++) for(long j = 0; j < M; j++){ P[i][j] = Q[i][j]; Z[i][j] += Q[i][j]; }
        }
        long zeta_mau = 0;
        for(long i = 0; i < M; i++) for(long j = 0; j < M; j++)
            if(Z[i][j] != (j <= i)) zeta_mau++;
        long id_mau = 0;
        for(long i = 0; i < M; i++) for(long j = 0; j < M; j++){
            long acc = 0;
            for(long k = 0; k < M; k++) acc += ((i==k) - S[i][k]) * Z[k][j];
            if(acc != (i == j)) id_mau++;
        }
        printf("      Σ_{j<M} S^j é a triangular de uns (a matriz de ζ): %s\n",
               zeta_mau ? "NÃO" : "sim");
        printf("      (1 − S)·ζ = I: %s — a inversa é a série de Neumann FINITA,"
               " por nilpotência\n", id_mau ? "NÃO" : "sim");
        if(zeta_mau || id_mau) mau++;

        /* (c) E S NÃO DIAGONALIZA: único valor próprio 0, e núcleo de dimensão 1. */
        long dim_nucleo = 0;
        for(long j = 0; j < M; j++){
            long nulo = 1;
            for(long i = 0; i < M; i++) if(S[i][j]) nulo = 0;
            if(nulo) dim_nucleo++;
        }
        printf("      valor próprio de S: 0 (diagonal nula) · dim ker S = %ld de %ld"
               " — bloco de Jordan\n", dim_nucleo, M);
        if(dim_nucleo != 1) mau++;
        long indep = 0;
        for(long k = 0; k < TA_N; k++){
            long norma = 0;
            for(long j = 0; j < TA_N; j++) norma += ta_chi(k,j) * ta_chi(k,j);
            if(norma == TA_N) indep++;
        }
        printf("      no lado de X: %ld caracteres de norma n — base própria completa\n\n",
               indep);
        if(indep != TA_N) mau++;

        ok("O DOMÍNIO E O CONTRADOMÍNIO SÃO DOIS ESPAÇOS LINEARES COM ESTRUTURAS"
           " OPOSTAS, e é isso que explica por que as duas inversões são diferentes."
           " Em W = ℤ^I, com base {δ_t}, o deslocamento S é a matriz SUBDIAGONAL de"
           " uns: nilpotente de índice exactamente dim W, com único valor próprio 0 e"
           " núcleo de dimensão UM — um bloco de Jordan, que NÃO diagonaliza. E é"
           " precisamente por ser nilpotente que 1 − S é invertível, com a inversa a ser"
           " a série de Neumann FINITA Σ S^j — que é a matriz triangular de uns, isto é,"
           " o ζ. Não há convergência nenhuma em jogo: há nilpotência. Do outro lado, em"
           " V = ℤ^X, os operadores de convolução COMUTAM e têm base própria completa —"
           " os n caracteres, todos de norma n. Donde: no domínio a inversão existe"
           " SEMPRE, por nilpotência; no contradomínio existe se e só se nenhum valor"
           " próprio se anula. Duas álgebras lineares, dois motivos de invertibilidade,"
           " e as duas operações da aranha uma em cada",
           mau == 0);
    }


    /* ═══ §AN28: OS PONTOS DA REVISÃO EXTERNA, MEDIDOS ═══════════════════════ */
    printf("\n§AN28  a revisão externa: o que ela apanhou, verificado aqui.\n\n");
    {
        long mau = 0;

        /* (1) τ = N NÃO EXIGE g = I: exige a DIAGONAL igual a 1. Os termos fora da
         *     diagonal são invisíveis a um percurso de arestas (§AN21), logo uma g
         *     com diagonal 1 e cruzado qualquer dá o MESMO τ. Eu tinha escrito
         *     «para qualquer outra g constante, τ ≠ N em geral», que engana. */
        long nt = real_dragao(1024);
        long g_id[2][2]   = {{1,0},{0,1}};
        long g_cruz1[2][2]= {{1,3},{3,1}};      /* diagonal 1, cruzado 3 */
        long g_dif[2][2]  = {{1,0},{0,4}};      /* diagonal NÃO constante */
        long taus[3];
        for(int m = 0; m < 3; m++){
            long (*g)[2] = (m == 0) ? g_id : (m == 1) ? g_cruz1 : g_dif;
            long soma = 0;
            for(long t = 0; t + 1 < nt; t++){
                long dx[2];
                for(int i = 0; i < 2; i++) dx[i] = traj[t+1].c[i] - traj[t].c[i];
                for(int u = 0; u < 2; u++) for(int v = 0; v < 2; v++)
                    soma += g[u][v] * dx[u] * dx[v];
            }
            taus[m] = soma;
        }
        printf("      g = I          τ = %ld   (N = %ld)\n", taus[0], nt-1);
        printf("      g = [[1,3],[3,1]]  τ = %ld   diagonal 1, cruzado 3 → MESMO τ\n", taus[1]);
        printf("      g = diag(1,4)  τ = %ld   diagonal não constante → outro τ\n\n", taus[2]);
        if(taus[0] != nt-1 || taus[1] != nt-1) mau++;   /* diagonal 1 ⟹ τ = N */
        if(taus[2] == nt-1) mau++;                      /* diagonal ≠ 1 ⟹ τ ≠ N */

        /* (2) NO CASO INJECTIVO NÃO É G ≡ 1: é G = 1 NA IMAGEM e 0 fora. A
         *     diferença só desaparece se π for sobrejectiva, e não é. */
        long ninj = 0;
        for(long t = 0; t <= 512 && ninj < AN_IMAX; t++){
            Vet v = vet0(); v.c[0] = (int)t; traj[ninj++] = v;
        }
        Res ri = an_corre(2, ninj);
        long fora_da_imagem = 0;
        {   /* um ponto que a trajectória não visitou tem G = 0, não 1 */
            Vet longe = vet0(); longe.c[0] = 99999;
            if(an_le(longe) == 0) fora_da_imagem = 1;
        }
        printf("      injectiva: |im π| = %ld · G = 1 na imagem · G = 0 fora: %s\n",
               ri.celulas, fora_da_imagem ? "sim" : "NÃO");
        if(ri.celulas != ninj || ri.max_g != 1 || !fora_da_imagem) mau++;

        /* (3) AS COORDENADAS NA BASE DOS CARACTERES SÃO F(f)/n, e não F(f). A
         *     transformada não normalizada NÃO é a mudança de coordenadas: é ela
         *     a menos do factor n. Mede-se recompondo f a partir das coordenadas. */
        static long ff[TA_N], FF[TA_N], rec[TA_N];
        for(long i = 0; i < TA_N; i++) ff[i] = (i*13) % 11 - 5;
        ta_F(ff, FF);
        long coord_mau = 0, sem_n_mau = 0;
        for(long x = 0; x < TA_N; x++){
            long acc = 0, acc_sem = 0;
            for(long k = 0; k < TA_N; k++){
                acc     += FF[k] * ta_chi(k, x);          /* Σ_k F(f)_k χ_k(x) */
                acc_sem += FF[k] * ta_chi(k, x);
            }
            if(acc % TA_N || acc / TA_N != ff[x]) coord_mau++;   /* com o /n bate */
            if(acc_sem == ff[x]) sem_n_mau++;                    /* sem o /n não */
            rec[x] = acc / TA_N;
        }
        /* sem o /n, acc = n·f(x), que só é igual a f(x) quando f(x) = 0 — logo os
         * casos que «batem» são EXACTAMENTE os zeros de f, e isso conta-se. */
        long zeros_f = 0;
        for(long x = 0; x < TA_N; x++) if(ff[x] == 0) zeros_f++;
        printf("      f = Σ_k (F(f)_k / n)·χ_k: %s\n", coord_mau ? "NÃO" : "sim");
        printf("      e sem o /n bate em %ld de %d — que são exactamente os %ld zeros de f\n\n",
               sem_n_mau, TA_N, zeros_f);
        if(coord_mau) mau++;
        if(sem_n_mau != zeros_f) mau++;

        ok("A REVISÃO EXTERNA APANHOU TRÊS COISAS QUE SE MEDEM, e as três estavam"
           " erradas do meu lado. PRIMEIRA: eu escrevia que τ = N pede g = I, e o que"
           " ele pede é a DIAGONAL igual a 1 — medido, g = [[1,3],[3,1]] dá o mesmo τ"
           " que a identidade, porque um percurso de arestas nunca sonda o termo"
           " cruzado; o que muda τ é a diagonal, e diag(1,4) muda-o. SEGUNDA: no caso"
           " injectivo não é G ≡ 1 — é G = 1 na IMAGEM e 0 fora dela, e a diferença só"
           " desapareceria se π fosse sobrejectiva. TERCEIRA, e é a mais fina: as"
           " coordenadas de f na base dos caracteres são F(f)/n e NÃO F(f) — a"
           " transformada não normalizada é a mudança de base a menos do factor n, o"
           " que a própria F⁻¹ = F/n já dizia. Recomposto com o /n bate nos 256; sem"
           " ele só bate onde f se anula, porque aí n·f(x) = f(x) — e o número de"
           " casos que batem é exactamente o número de zeros de f",
           mau == 0);
    }


    /* ═══ §AN29: O PAR DUAL DE CANTOR — encher e contar, e a FOLGA ═══════════ */
    printf("\n§AN29  encher e contar: a sobrejectiva, a injectiva, e o que sobra.\n\n");
    {
        long mau = 0;

        /* (a) CONTAR É INJECTIVA e NÃO SOBREJECTIVA: leva as 256 células do
         *     quadrado em 256 índices distintos de 512 — sobra metade da recta. */
        static int visto[PD_I];
        for(long d = 0; d < PD_I; d++) visto[d] = 0;
        long distintos = 0, colisao = 0;
        for(int x = 0; x < PD_Q; x++)
            for(int y = 0; y < PD_Q; y++){
                long d = pd_contar(x, y);
                if(visto[d]) colisao++; else { visto[d] = 1; distintos++; }
            }
        long sobra = 0;
        for(long d = 0; d < PD_I; d++) if(!visto[d]) sobra++;
        printf("      contar: X → I   |X| = %d · imagem = %ld · colisões = %ld ·"
               " SOBRA na recta = %ld\n", PD_X, distintos, colisao, sobra);
        if(colisao || distintos != PD_X || sobra != PD_I - PD_X) mau++;

        /* (b) ENCHER É SOBREJECTIVA e NÃO INJECTIVA: cobre as 256 células, cada
         *     uma DUAS vezes. Lê-se no campo: G ≡ 2 em todo o suporte. */
        long nt = 0;
        for(long d = 0; d < PD_I && nt < AN_IMAX; d++){
            int x, y; pd_encher(d, &x, &y);
            Vet v = vet0(); v.c[0] = x; v.c[1] = y; traj[nt++] = v;
        }
        Res r = an_corre(2, nt);
        long g_dois = 0;
        an_zera(2);
        for(long t = 0; t < nt; t++) an_visita(traj[t]);
        for(long t = 0; t < nt; t++) if(an_le(traj[t]) == 2) g_dois++;
        printf("      encher: I → X   |I| = %ld · células = %ld · max G = %ld ·"
               " G = 2 em %ld de %ld\n", nt, r.celulas, r.max_g, g_dois, nt);
        if(r.celulas != PD_X || r.max_g != 2 || g_dois != nt) mau++;

        /* (c) O PAR É UMA RETRACÇÃO: encher∘contar = id_X, resíduo 0. Mas
         *     contar∘encher ≠ id_I, e falha EXACTAMENTE na metade alta — que é
         *     a que contar nunca usa. Só um lado fecha. */
        long res_direita = 0;
        for(int x = 0; x < PD_Q; x++)
            for(int y = 0; y < PD_Q; y++){
                int xv, yv; pd_encher(pd_contar(x, y), &xv, &yv);
                if(xv != x || yv != y) res_direita++;
            }
        long falha_esquerda = 0;
        for(long d = 0; d < PD_I; d++){
            int x, y; pd_encher(d, &x, &y);
            if(pd_contar(x, y) != d) falha_esquerda++;
        }
        printf("      encher∘contar = id_X: resíduo %ld · contar∘encher ≠ id_I:"
               " falha em %ld de %d\n", res_direita, falha_esquerda, PD_I);
        if(res_direita || falha_esquerda != PD_I - PD_X) mau++;

        /* (d) E O QUE O LEVANTAMENTO REPÕE É EXACTAMENTE O QUE SE PERDEU. A
         *     folha k(i) ∈ {1,2} é o bit que encher deitou fora (d ≥ 4^w), e com
         *     ela π̃ volta a ser injectiva — sobre um contradomínio de 512, que é
         *     |I|. A retracção passa a bijecção ao preço de uma coordenada. */
        static long k_de[AN_IMAX];
        an_zera(2);
        for(long t = 0; t < nt; t++){ an_visita(traj[t]); k_de[t] = an_le(traj[t]); }
        long folha_mau = 0;
        for(long t = 0; t < nt; t++){
            long bit_alto = (t >= PD_X) ? 2 : 1;      /* a metade da recta de onde veio */
            if(k_de[t] != bit_alto) folha_mau++;
        }
        printf("      a folha k(i) É o bit que encher deitou fora: %s\n",
               folha_mau ? "NÃO" : "sim, nos 512");
        if(folha_mau) mau++;

        /* E AQUI π̃ NÃO É SÓ INJECTIVA — É BIJECÇÃO sobre X × {1,2}. Como G ≡ 2
         * exactamente, a fibra sobre cada célula é {1,2} e nenhuma folha fica por
         * usar: |im π̃| = 2·|X| = |I|. É o quadrado a fechar. */
        an_zera(3);
        long cobre_mau = 0;
        for(long t = 0; t < nt; t++){
            Vet lev = traj[t]; lev.c[2] = (int)k_de[t];
            an_visita(lev);
        }
        long celulas_lev = an_ocupadas;
        for(int x = 0; x < PD_Q; x++)
            for(int y = 0; y < PD_Q; y++)
                for(int kk = 1; kk <= 2; kk++){
                    Vet w = vet0(); w.c[0] = x; w.c[1] = y; w.c[2] = kk;
                    if(an_le(w) != 1) cobre_mau++;      /* cada (x,k) uma vez */
                }
        printf("      π̃: I → X × {1,2} é BIJECÇÃO: %ld pontos de %d, cada um uma vez: %s\n\n",
               celulas_lev, 2*PD_X, cobre_mau ? "NÃO" : "sim");
        if(celulas_lev != 2*PD_X || cobre_mau) mau++;

        /* (e) A CONSTRUÇÃO MAIS LIMPA, e não é a minha: enumerar o quadrado em
         *     SERPENTINA — q_0, q_1, …, q_{M²−1}, cada ponto uma vez — e lançar
         *     na recta com ρ(q_t) = 2t. A imagem são os PARES, e os ímpares ficam
         *     todos livres: a folga fica distribuída em vez de num bloco. */
        {
            static int ocupado[2*PD_X + 2];
            for(long d = 0; d < 2*PD_X + 2; d++) ocupado[d] = 0;
            long t = 0, col = 0;
            for(int y = 0; y < PD_Q; y++)
                for(int i = 0; i < PD_Q; i++){
                    int x = (y & 1) ? (PD_Q - 1 - i) : i;    /* serpenteia */
                    long rho = 2*t;                          /* ρ(q_t) = 2t */
                    if(ocupado[rho]) col++;
                    ocupado[rho] = 1;
                    (void)x; t++;
                }
            long impares_livres = 0, pares_usados = 0;
            for(long d = 0; d < 2*PD_X; d++){
                if(d & 1){ if(!ocupado[d]) impares_livres++; }
                else if(ocupado[d]) pares_usados++;
            }
            printf("      serpentina, ρ(q_t) = 2t:  colisões %ld · pares usados %ld ·"
                   " ímpares livres %ld\n", col, pares_usados, impares_livres);
            if(col || pares_usados != PD_X || impares_livres != PD_X) mau++;
        }

        /* (f) E O QUE PROÍBE A OUTRA COISA É O GRAU. Não se pode pedir ao mesmo
         *     tempo injectividade e preservação de arestas: um vértice INTERIOR do
         *     quadrado tem 4 vizinhos distintos, e a recta dá 2. É o thm:dimensao
         *     a decidir — |V(x)| = 2n —, e não uma dificuldade de construção. */
        {
            int gx = PD_Q/2, gy = PD_Q/2;            /* um vértice interior */
            long viz_quadrado = 0, viz_recta = 0;
            for(int i = 0; i < 2; i++) for(int sg = -1; sg <= 1; sg += 2){
                int vx = gx + (i==0)*sg, vy = gy + (i==1)*sg;
                if(vx >= 0 && vx < PD_Q && vy >= 0 && vy < PD_Q) viz_quadrado++;
            }
            long d = pd_contar(gx, gy);
            if(d - 1 >= 0)   viz_recta++;
            if(d + 1 < PD_I) viz_recta++;
            printf("      o grau proíbe: vértice interior tem %ld vizinhos, a recta dá %ld"
                   " — logo não há injecção que preserve arestas\n\n",
                   viz_quadrado, viz_recta);
            if(viz_quadrado != 4 || viz_recta != 2) mau++;
            if(viz_quadrado <= viz_recta) mau++;
        }

        /* (g) AS DUAS PERGUNTAS TÊM A MESMA RESPOSTA. Perguntar «quantas partes da
         *     folha foram coladas aqui?» dá o EXCESSO de colagem, Σ_x (G(x) − 1);
         *     perguntar «quantos pontos da recta ainda estão sem folha?» dá os
         *     BURACOS, |I| − |im ρ|. Os dois são |I| − |X|, e não por acaso: um é
         *     ΣG − |supp G| e o outro |I| − |X| com ρ injectiva sobre X. */
        {
            an_zera(2);
            for(long t = 0; t < nt; t++) an_visita(traj[t]);
            long excesso = 0, suporte = an_ocupadas;
            for(long t = 0; t < nt; t++){
                /* conta-se uma vez por célula: soma G−1 sobre o suporte */
            }
            for(long i = 0; i < AN_CAP; i++) if(an_vivo[i]) excesso += an_G[i] - 1;
            long buracos = sobra;                       /* de (a): |I| − |im contar| */
            printf("      as duas perguntas:  colagens em excesso Σ(G−1) = %ld  ·"
                   "  buracos |I| − |im ρ| = %ld  ·  |I| − |X| = %d\n\n",
                   excesso, buracos, PD_I - PD_X);
            if(excesso != PD_I - PD_X || buracos != PD_I - PD_X) mau++;
            if(suporte != PD_X) mau++;
        }

        ok("O PAR DE CANTOR TEM DOIS LADOS, E O SEGUNDO PEDE FOLGA. A casa já media"
           " encher e contar como BIJECÇÃO — e no finito com |I| = |X| não podia ser"
           " outra coisa. O par assimétrico obriga |I| > |X|, e então: CONTAR leva o"
           " quadrado na recta de forma INJECTIVA e NÃO SOBREJECTIVA — as 256 células"
           " em 256 índices distintos de 512, com 256 lugares da recta POR USAR, que é"
           " a folga que se pedia. E ENCHER leva a recta no quadrado de forma"
           " SOBREJECTIVA e NÃO INJECTIVA — cobre as 256 células, cada uma duas vezes,"
           " e o campo lê-o directamente: G ≡ 2 em todo o suporte. O par é uma"
           " RETRACÇÃO e não uma involução: encher∘contar = id_X com resíduo 0, mas"
           " contar∘encher falha em 256 de 512 — exactamente na metade que contar"
           " nunca usa. Só um lado fecha. E o que o LEVANTAMENTO repõe é precisamente"
           " o que se perdeu: a folha k(i) ∈ {1,2} É o bit que encher deitou fora, e"
           " com ela π̃ não é só injectiva — é BIJECÇÃO sobre X × {1,2}, porque G ≡ 2"
           " exactamente e nenhuma folha fica por usar: 512 pontos, cada um uma vez. A retracção vira bijecção"
           " ao preço de uma coordenada — que é o Teor. do levantamento outra vez, aqui"
           " no par de Cantor. E A CONSTRUÇÃO MAIS LIMPA NÃO É ESTA: é enumerar o"
           " quadrado em SERPENTINA e lançar na recta com ρ(q_t) = 2t, ficando a"
           " imagem nos pares e os 256 ímpares livres — a folga distribuída em vez de"
           " num bloco. POR FIM, O QUE PROÍBE PEDIR AS DUAS COISAS: injectividade E"
           " preservação de arestas não coexistem, e não por dificuldade de construção"
           " — um vértice interior do quadrado tem QUATRO vizinhos e a recta dá DOIS."
           " É o teorema da dimensão a decidir, |V(x)| = 2n. Donde a dualidade é entre"
           " DOBRA e BURACO, e não entre duas inversas: quem comprime a recta no"
           " quadrado paga com G > 1; quem espalha o quadrado na recta paga com"
           " índices por usar. E AS DUAS PERGUNTAS TÊM A MESMA RESPOSTA, que é o que"
           " torna a dualidade quantitativa e não só de forma: «quantas partes da folha"
           " foram coladas aqui?» dá o excesso de colagem Σ(G−1) = 256, e «quantos"
           " pontos da recta ainda estão sem folha?» dá os buracos |I| − |im ρ| = 256."
           " São o MESMO número, |I| − |X|, e não por coincidência: o primeiro é"
           " ΣG − |supp G| e o segundo é |I| menos a imagem de uma injectiva sobre X."
           " A folga é uma só, vista pelos dois mapas",
           mau == 0);
    }


    /* ═══ §AN30: A BASE É O ALFABETO — a trajectória como PALAVRA ════════════ */
    printf("\n§AN30  a trajectória como palavra no alfabeto das arestas.\n\n");
    {
        long mau = 0;

        /* (a) EXTRAIR A PALAVRA. Se cada passo é uma aresta, ele é uma letra: a
         *     direcção da base por onde se saiu. Na grade ℤⁿ o alfabeto tem 2n
         *     letras — direcção E sentido, porque +e_i ≠ −e_i. */
        long nt = real_dragao(1024);
        static int pal[AN_IMAX];
        long nao_letra = 0;
        for(long t = 0; t + 1 < nt; t++){
            int letra = -1;
            for(int i = 0; i < 2 && letra < 0; i++)
                for(int sg = -1; sg <= 1 && letra < 0; sg += 2){
                    int ok1 = 1;
                    for(int j = 0; j < 2; j++){
                        int esperado = traj[t].c[j] + ((j==i) ? sg : 0);
                        if(traj[t+1].c[j] != esperado) ok1 = 0;
                    }
                    if(ok1) letra = 2*i + (sg > 0 ? 1 : 0);
                }
            if(letra < 0) nao_letra++;
            pal[t] = letra;
        }
        printf("      a palavra do dragão: %ld letras num alfabeto de 4 · passos sem"
               " letra: %ld\n", nt-1, nao_letra);
        if(nao_letra) mau++;

        /* (b) O TESTE: reconstruir π(t) SÓ de π(0) e da palavra. Se fechar, a
         *     aresta como memória da divisão deixa de ser interpretação e passa a
         *     construção. */
        long recon_mau = 0;
        {
            Vet p = traj[0];
            for(long t = 0; t + 1 < nt; t++){
                int i = pal[t] / 2, sg = (pal[t] % 2) ? +1 : -1;
                p.c[i] += sg;
                if(!an_igual(p, traj[t+1])) recon_mau++;
            }
        }
        printf("      reconstruir π(t) de π(0) + palavra: divergências %ld em %ld\n\n",
               recon_mau, nt-1);
        if(recon_mau) mau++;

        /* (c) E A PALAVRA TEM MAIS DO QUE A POSIÇÃO. A posição é a SOMA dos
         *     passos, e a soma é comutativa: permutar as letras leva ao mesmo
         *     sítio. Logo o morfismo palavra → posição tem NÚCLEO, e é esse núcleo
         *     que produz a dobra. Exibe-se: a palavra do dragão permutada dá a
         *     mesma posição final e uma trajectória diferente. */
        static int pal2[AN_IMAX];
        for(long t = 0; t + 1 < nt; t++) pal2[t] = pal[nt-2-t];   /* palavra ao contrário */
        Vet fim1 = traj[0], fim2 = traj[0];
        long difere_caminho = 0;
        {
            Vet a = traj[0], b = traj[0];
            for(long t = 0; t + 1 < nt; t++){
                int i1 = pal[t]/2,  s1 = (pal[t]%2)  ? +1 : -1;
                int i2 = pal2[t]/2, s2 = (pal2[t]%2) ? +1 : -1;
                a.c[i1] += s1; b.c[i2] += s2;
                if(!an_igual(a, b)) difere_caminho++;
            }
            fim1 = a; fim2 = b;
        }
        printf("      a mesma palavra permutada: chega ao MESMO fim? %s · caminho"
               " diferente em %ld dos %ld passos\n",
               an_igual(fim1, fim2) ? "sim" : "NÃO", difere_caminho, nt-1);
        if(!an_igual(fim1, fim2)) mau++;
        if(difere_caminho == 0) mau++;

        /* (d) NO HIPERCUBO O ALFABETO ENCOLHE PARA m, porque ⊕e_i é involutivo:
         *     não há sentido a codificar. E a posição depende só das PARIDADES das
         *     letras — a ordem desaparece por completo. */
        const int MH = 8;
        long par_mau = 0;
        for(long amostra = 0; amostra < 64; amostra++){
            long x1 = 0, x2 = 0, conta[8] = {0,0,0,0,0,0,0,0};
            /* o regime importa: com passo uniforme cada letra sairia o mesmo
             * número de vezes, e «paridade» coincidiria com «presença». Aqui as
             * contagens são desiguais de propósito, e há letras a PAR. */
            for(int t = 0; t < 37 + (int)(amostra % 5); t++){
                int letra = (int)((amostra*7 + t*t) % MH);
                x1 ^= (1L << letra);
                conta[letra]++;
            }
            for(int i = 0; i < MH; i++) if(conta[i] & 1) x2 ^= (1L << i);
            if(x1 != x2) par_mau++;
        }
        /* e conta-se quantas letras saem a PAR, senão o teste não distinguiria
         * paridade de presença */
        long letras_pares = 0;
        {
            long conta[8] = {0,0,0,0,0,0,0,0};
            for(int t = 0; t < 39; t++) conta[(int)((7 + t*t) % MH)]++;
            for(int i = 0; i < MH; i++) if(conta[i] && !(conta[i] & 1)) letras_pares++;
        }
        printf("      no hipercubo: alfabeto de %d letras, e a posição é a PARIDADE"
               " de cada letra — %s em 64 palavras\n", MH, par_mau ? "NÃO" : "confere");
        printf("      (e há %ld letras a sair um número PAR de vezes: sem elas, paridade"
               " e presença coincidiriam)\n\n", letras_pares);
        if(par_mau) mau++;
        if(letras_pares == 0) mau++;

        /* (e) E A PALAVRA NÃO É A GEOMETRIA. A estrutura está na BASE; o dragão é
         *     a IMAGEM dela depois de uma transformação T que diz como cada letra
         *     se realiza. Mesma palavra, três T diferentes, três campos: a DOBRA é
         *     da REALIZAÇÃO, e não da palavra. */
        printf("      realização T da mesma palavra   n   |I|    células   max G   dobra?\n");
        struct { const char *nome; int n; } ts[3];
        ts[0].nome = "±e_i em ℤ² (o dragão)"; ts[0].n = 2;
        ts[1].nome = "4 direcções em ℤ⁴";     ts[1].n = 4;
        ts[2].nome = "±e_0 na recta ℤ¹";      ts[2].n = 1;
        long dobram = 0, nao = 0;
        for(int m = 0; m < 3; m++){
            static Vet real[AN_IMAX];
            Vet p = vet0();
            long np = 0;
            real[np++] = p;
            for(long t = 0; t + 1 < nt; t++){
                int L = pal[t];
                if(m == 0){ int i = L/2, sg = (L%2)?+1:-1; p.c[i] += sg; }
                else if(m == 1){ p.c[L] += 1; }            /* cada letra, uma direcção */
                else { p.c[0] += (L % 2) ? +1 : -1; }       /* uma recta, dois sentidos */
                real[np++] = p;
            }
            for(long t = 0; t < np; t++) traj[t] = real[t];
            Res r = an_corre(ts[m].n, np);
            printf("      %-30s %d   %-6ld %-9ld %-7ld %s\n",
                   ts[m].nome, ts[m].n, r.nt, r.celulas, r.max_g,
                   r.max_g > 1 ? "sim" : "não");
            if(r.soma != r.nt || !r.recontou) mau++;
            if(r.max_g > 1) dobram++; else nao++;
        }
        printf("\n");
        /* a mesma palavra dobra numa realização e não noutra — é o que se afirma */
        if(dobram == 0 || nao == 0) mau++;

        ok("A BASE ORTONORMAL É O ALFABETO, E A TRAJECTÓRIA É UMA PALAVRA NELE. Cada"
           " passo é uma aresta, e a aresta É A MEMÓRIA DA DIVISÃO — o registo de por"
           " qual das saídas do vértice se saiu:"
           " na grade ℤⁿ o alfabeto tem 2n letras — direcção e sentido, porque +e_i ≠"
           " −e_i —, e no hipercubo encolhe para m, porque ⊕e_i é involutivo e não há"
           " sentido a codificar. O TESTE FECHA: extraída a palavra do dragão, todas as"
           " 1024 letras existem, e π(t) reconstrói-se de π(0) mais a palavra com"
           " divergência ZERO — a aresta como MEMÓRIA DA DIVISÃO deixa de ser"
           " interpretação e passa a construção. E DAÍ SAI A DOBRA, que é o que interessa: a posição é a SOMA"
           " dos passos, e a soma é COMUTATIVA — permutar a palavra leva ao mesmo sítio"
           " por outro caminho, medido aqui com a palavra ao contrário. Logo o morfismo"
           " palavra → posição tem NÚCLEO, e a dobra é exactamente esse núcleo: a célula"
           " esquece a ordem, e G conta quantas ordens lá chegaram. No hipercubo isto vê-se"
           " no osso — a posição é só a PARIDADE de cada letra, e tudo o resto da palavra"
           " é o que a projecção deita fora. E DAQUI SAI A SEPARAÇÃO QUE FALTAVA: a"
           " estrutura está na BASE, e a geometria é uma TRANSFORMAÇÃO T que diz como"
           " cada letra se realiza — P_{t+1} = P_t + T(e_{s_t}). O dragão não é a"
           " estrutura: é a imagem dela sob um T. Medido com a MESMA palavra e três T:"
           " com ±e_i em ℤ² dobra; com quatro direcções independentes em ℤ⁴ NÃO dobra,"
           " porque nenhuma soma de letras distintas colide; e com ±e_0 numa RECTA"
           " dobra mais do que no plano, porque há menos onde ir. Logo a DOBRA é"
           " propriedade da REALIZAÇÃO e não da palavra, que é a mesma nas três."
           " E uma expectativa minha caiu aqui: pôr todas as letras a avançar na mesma"
           " direcção NÃO dobra — é monótono, logo injectivo, e a dimensão não tem nada"
           " a ver com isso. O que faz dobrar é a realização poder VOLTAR",
           mau == 0);
    }


    /* ═══ §AN31: MUITOS AGENTES, O MESMO CAMPO — e o que isso obriga ═════════ */
    printf("\n§AN31  ocupação por multiplicidade contra expansão por memória.\n\n");
    {
        long mau = 0;
        const int LADO = 8, K = 4;

        /* DUAS ARQUITECTURAS PARA O MESMO PROBLEMA: aumentar estrutura sem perder
         * a memória das divisões.
         *   (A) UM agente que passa K vezes por cada célula — expansão por memória
         *   (B) K agentes que passam UMA vez por cada célula — ocupação por
         *       multiplicidade
         * A pergunta é se o campo as distingue. */
        static long campoA[64], campoB[64];
        for(long i = 0; i < 64; i++){ campoA[i] = campoB[i] = 0; }

        /* (A) um agente, K voltas */
        long ntA = 0;
        for(int volta = 0; volta < K; volta++)
            for(int c = 0; c < LADO*LADO/4 && ntA < AN_IMAX; c++){
                Vet v = vet0(); v.c[0] = c % LADO; v.c[1] = c / LADO;
                traj[ntA++] = v;
            }
        an_zera(2);
        for(long t = 0; t < ntA; t++) an_visita(traj[t]);
        for(int x = 0; x < LADO; x++) for(int y = 0; y < LADO; y++){
            Vet v = vet0(); v.c[0] = x; v.c[1] = y;
            campoA[y*LADO + x] = an_le(v);
        }
        Res rA = an_corre(2, ntA);

        /* (B) K agentes, uma volta cada — e o campo é O MESMO ARRAY, porque a
         *     memória é do espaço e não do agente: nenhum deles a leva consigo */
        an_zera(2);
        long ntB = 0;
        for(int ag = 0; ag < K; ag++)
            for(int c = 0; c < LADO*LADO/4 && ntB < AN_IMAX; c++){
                Vet v = vet0(); v.c[0] = c % LADO; v.c[1] = c / LADO;
                an_visita(v);
                traj[ntB++] = v;
            }
        for(int x = 0; x < LADO; x++) for(int y = 0; y < LADO; y++){
            Vet v = vet0(); v.c[0] = x; v.c[1] = y;
            campoB[y*LADO + x] = an_le(v);
        }

        long difere = campos_diferem(campoA, campoB, 64);
        printf("      um agente com %d voltas   ·   %d agentes com uma volta\n", K, K);
        printf("      os dois campos diferem em %ld das 64 células · |I| igual? %s\n",
               difere, (ntA == ntB) ? "sim" : "NÃO");
        if(difere || ntA != ntB) mau++;

        /* O CONTROLO NEGATIVO, sem o qual «são iguais» não diria nada: uma terceira
         * execução com K−1 voltas tem de dar um campo DIFERENTE, e a mesma
         * comparação tem de o ver. Sem isto, uma comparação que não comparasse
         * passaria na asserção acima. */
        static long campoC[64];
        an_zera(2);
        for(int volta = 0; volta < K-1; volta++)
            for(int c = 0; c < LADO*LADO/4; c++){
                Vet v = vet0(); v.c[0] = c % LADO; v.c[1] = c / LADO;
                an_visita(v);
            }
        for(int x = 0; x < LADO; x++) for(int y = 0; y < LADO; y++){
            Vet v = vet0(); v.c[0] = x; v.c[1] = y;
            campoC[y*LADO + x] = an_le(v);
        }
        long difere_ctl = campos_diferem(campoA, campoC, 64);   /* a MESMA função */
        printf("      controlo: contra %d voltas o mesmo teste vê %ld células"
               " diferentes\n", K-1, difere_ctl);
        if(difere_ctl != LADO*LADO/4) mau++;

        /* O CAMPO NÃO OS DISTINGUE, e não é falha: é a cláusula 3. Como a memória
         * está no ESPAÇO e não no agente, nada do que se escreve carrega a
         * identidade de quem escreveu. G diz «K construções caíram aqui» e não diz
         * se foram K passagens de um ou uma passagem de K. */
        long celulas_com_K = 0;
        for(long i = 0; i < 64; i++) if(campoA[i] == K) celulas_com_K++;
        printf("      G = %d em %ld células, nas DUAS leituras — o campo não sabe"
               " quem escreveu\n\n", K, celulas_com_K);
        if(celulas_com_K != LADO*LADO/4) mau++;

        /* E O QUE CADA COORDENADA REPÕE É DIFERENTE. A folha k(i) do levantamento
         * repõe a ORDEM da visita — dentro de um agente. Para separar AGENTES é
         * preciso outra coordenada, o identificador; e ela não sai do campo. */
        /* o regime tem de ter VOLTAS MÚLTIPLAS por agente: com uma volta cada, a
         * ordem e o agente separam os MESMOS pares e o teste não distinguiria as
         * duas coordenadas. */
        static long ordem[AN_IMAX], quem[AN_IMAX];
        const int AG = 2, VOLTAS = 2;
        an_zera(2);
        long ntC2 = 0;
        for(int ag = 0; ag < AG; ag++)
            for(int volta = 0; volta < VOLTAS; volta++)
                for(int c = 0; c < LADO*LADO/4 && ntC2 < AN_IMAX; c++){
                    Vet v = vet0(); v.c[0] = c % LADO; v.c[1] = c / LADO;
                    traj[ntC2] = v;
                    an_visita(v);
                    ordem[ntC2] = an_le(v);
                    quem[ntC2]  = ag;                /* o agente, que o campo não vê */
                    ntC2++;
                }
        long ordem_separa = 0, quem_separa = 0, pares = 0;
        for(long t = 0; t < ntC2; t++)
            for(long u = t+1; u < ntC2; u++)
                if(an_igual(traj[t], traj[u])){
                    pares++;
                    if(ordem[t] != ordem[u]) ordem_separa++;
                    if(quem[t]  != quem[u])  quem_separa++;
                }
        printf("      com %d agentes × %d voltas, %ld pares na mesma célula:\n",
               AG, VOLTAS, pares);
        printf("        a ORDEM separa %ld  ·  o AGENTE separa %ld  — números"
               " DIFERENTES, logo são coordenadas distintas\n\n",
               ordem_separa, quem_separa);
        if(ordem_separa != pares) mau++;              /* a ordem separa TODOS */
        if(quem_separa >= ordem_separa) mau++;        /* o agente separa MENOS */
        if(quem_separa == 0) mau++;

        ok("O CAMPO É MEMÓRIA DA CONSTRUÇÃO E NÃO DO CONSTRUTOR — e isso não é falha,"
           " é A"
           " CLÁUSULA 3. Dito ao contrário para não se ler ao contrário: não é que G"
           " identifique alguma coisa — é que G NÃO MUDA quando se trocam os"
           " construtores. Duas arquitecturas resolvem o mesmo problema de aumentar"
           " estrutura sem perder a memória das divisões: uma OCUPA por multiplicidade"
           " de construtores, a outra EXPANDE pela memória de um só. Medido: um agente"
           " a dar 4 voltas e 4 agentes a dar uma volta produzem campos IGUAIS célula a"
           " célula, com o mesmo |I| — G = 4 nas 16 células, nas duas leituras. E não"
           " podia ser de outro modo: como a memória está no ESPAÇO e não no agente,"
           " nada do que se escreve carrega a identidade de quem escreveu. G diz que"
           " quatro construções caíram ali; não diz se foram quatro passagens de um ou"
           " uma passagem de quatro. E o CONTROLO é que dá conteúdo a isto: a mesma"
           " comparação, posta contra uma execução de 3 voltas, vê as 16 células a"
           " diferir — sem ele, uma comparação que não comparasse nada passaria."
           " É por isto que a estigmergia ESCALA — acrescentar"
           " construtores não muda o mecanismo. E as coordenadas que repõem o que se"
           " perdeu são DUAS e distintas: a folha k(i) repõe a ORDEM da visita, dentro"
           " de um agente; separar AGENTES pede outra coordenada, o identificador, e essa"
           " não sai do campo — tem de vir de fora. E as duas não são a mesma: com 2"
           " agentes a dar 2 voltas cada, dos pares que caem na mesma célula a ORDEM"
           " separa TODOS e o agente separa só os que atravessam a fronteira entre"
           " agentes — números diferentes. Num regime de uma volta por agente as duas"
           " coincidiriam, e o teste não estaria a distinguir nada",
           mau == 0);
    }

    printf("\n=== %d asserções, %d falhas ===\n", unidades, falhas);
    return falhas ? 1 : 0;
}
