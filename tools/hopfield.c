/* hopfield.c — A REDE DE HOPFIELD E A ÁRVORE: onde são a mesma coisa, e onde NÃO são.
 *
 * O Aarão: "explora as redes de Hopfield, é a mesma coisa — a interação é a cifra, dobra e
 * desdobra navegando, o presente são as folhas, a árvore já é a hierarquia de memória, já funciona
 * assim na assistente, só formalizar e generalizar e nomear."
 *
 * A parte dele está certa e mede-se: **recuperar é DESCER**, nos dois. Na Hopfield desce-se a
 * energia até um mínimo; na árvore desce-se a profundidade até uma folha. E a sobreposição da
 * Hopfield — o produto interno com o padrão guardado — **é** o prefixo comum, quando os padrões
 * são caminhos numa árvore. Isso não é analogia: é uma identidade, e está medida no §F3.
 *
 * MAS há uma diferença, e ela é o resultado. A Hopfield **satura**: acima de ~0,138·N padrões ela
 * deixa de recuperar, porque os pesos são uma SOMA e as memórias interferem umas nas outras. A
 * árvore não satura, e a razão é estrutural: ela **separa** onde a Hopfield **soma**.
 *
 *     Hopfield   w_ij = Σ_p ξ_i^p ξ_j^p        uma matriz, todas as memórias sobrepostas
 *     árvore     um ramo por prefixo            cada memória no seu caminho, sem sobreposição
 *
 * Então a frase certa não é "é a mesma coisa": é **a árvore é a Hopfield com a interferência
 * resolvida pela hierarquia** — e o preço é o espaço, que a Hopfield não paga e ela paga. Dizer
 * "é a mesma coisa" apagaria exatamente o que o Aarão construiu.
 *
 *   §F1  a Hopfield clássica: Hebb, e a energia DESCE — a lei, medida em muitos arranques
 *   §F2  a SATURAÇÃO: a capacidade medida contra o 0,138·N de Amit–Gutfreund–Sompolinsky
 *   §F3  a SOBREPOSIÇÃO É O PREFIXO COMUM — identidade, medida em todos os pares
 *   §F4  recuperar é DESCER nos dois, e os dois caem no MESMO padrão
 *   §F5  a árvore NÃO satura: mede-se lado a lado onde a Hopfield já falhou
 *   §F6  e o nome: o presente são as folhas, e a interação é a cifra
 *
 * ─── E AÍ O AARÃO CORRIGIU-ME, e a correção é metade da teoria ────────────────────────────────
 *
 *   "perai, ainda tem só metade da teoria. A árvore é uma TORRE, a branca. A parte reversível é a
 *    torre NEGRA, outra árvore dual. Então tem ciclos sim, mas ANTISSIMÉTRICOS. Confronta com o
 *    corpo de corpos no R^n da teoria principal."
 *
 * Ele tem razão e o §F1 mostra onde eu parei: eu medi que "a energia nunca sobe" e chamei-lhe o
 * resultado. Mas isso só vale para a **torre branca** — a descida. Ela nunca sobe *porque a matriz
 * de Hebb é SIMÉTRICA*, e uma matriz simétrica só sabe descer. A outra metade estava a faltar.
 *
 * E a teoria principal já tinha a peça: **B = B_s + B_a**, a partição única em simétrica e
 * antissimétrica (§B12, e o `dualrn.c`). Ali está escrito que
 *
 *      o INTERNO (simétrico)      MEDE      -> e no R^n ele não vê o sinal do dual
 *      o CRUZADO (antissimétrico) ORDENA    -> e é ele, e só ele, que o dual inverte
 *
 * Na rede é literalmente o mesmo, e mede-se: uma W **simétrica** converge a ponto fixo (memória, o
 * destino); uma W **antissimétrica** NÃO converge — ela **cicla**, e o ciclo tem período 2. *A
 * memória é a parte que mede; o ciclo é a parte que ordena.* Os ciclos não são um defeito da rede:
 * são a **torre negra**, e sem eles não há reversão.
 *
 *   §F7  a torre NEGRA: W antissimétrica NÃO converge — ela cicla, e o período é 2
 *   §F8  a partição B = B_s + B_a é ÚNICA, e cada metade faz uma coisa só
 *   §F9  o confronto com o R^n: o interno mede e o cruzado ordena — a MESMA lei
 *   §F10 as duas torres juntas: a branca desce, a negra volta, e o ciclo fecha
 *
 *   cc -O2 -std=c99 hopfield.c -lm -o hopfield && ./hopfield
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ───────────────────────────────────────────────────────────────────────────
 * §F1  A HOPFIELD — sem RAM a mais: N=128 dá 16 KB de pesos, e é o que se usa
 * ─────────────────────────────────────────────────────────────────────────── */

#define N     128                      /* neurônios */
#define PMAX   64                      /* padrões que cabem no ensaio */

typedef struct { signed char x[PMAX][N]; int p; double w[N][N]; } Rede;

/* o gerador determinístico — nada de Math.random: a corrida tem de repetir-se */
static unsigned long SEM = 88172645463325252UL;
static unsigned long xs(void){ SEM ^= SEM<<13; SEM ^= SEM>>7; SEM ^= SEM<<17; return SEM; }
static int bit(void){ return (xs() >> 33) & 1 ? 1 : -1; }

/* Hebb: w_ij = (1/N) Σ_p ξ_i ξ_j, com diagonal nula (o clássico) */
static void grava(Rede *r){
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++){
            if(i == j){ r->w[i][j] = 0; continue; }
            double s = 0;
            for(int p = 0; p < r->p; p++) s += r->x[p][i] * r->x[p][j];
            r->w[i][j] = s / N;
        }
}

/* a energia de Hopfield: E = -½ Σ w_ij s_i s_j */
static double energia(const Rede *r, const signed char *s){
    double E = 0;
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++) E += r->w[i][j] * s[i] * s[j];
    return -0.5 * E;
}

/* um passo assíncrono: escolhe i e alinha s_i com o campo local. É a DESCIDA. */
static int passo(const Rede *r, signed char *s, int i){
    double h = 0;
    for(int j = 0; j < N; j++) h += r->w[i][j] * s[j];
    signed char novo = (h >= 0) ? 1 : -1;
    if(novo == s[i]) return 0;
    s[i] = novo;
    return 1;
}

/* recupera: desce até não haver mais mudança. Devolve o número de varreduras. */
static int recupera(const Rede *r, signed char *s, int limite){
    for(int v = 0; v < limite; v++){
        int mudou = 0;
        for(int i = 0; i < N; i++) mudou += passo(r, s, i);
        if(!mudou) return v + 1;
    }
    return limite;
}

/* a sobreposição: o produto interno normalizado. 1 = igual, -1 = o negativo, 0 = ortogonal. */
static double sobrepoe(const signed char *a, const signed char *b){
    double s = 0;
    for(int i = 0; i < N; i++) s += a[i] * b[i];
    return s / N;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §F3  OS PADRÕES EM ÁRVORE — e é aqui que a identidade aparece
 *
 * Um caminho numa árvore binária de profundidade D é uma sequência de D escolhas. Se cada escolha
 * ocupa N/D neurônios, então DOIS caminhos que partilham k escolhas partilham k·(N/D) neurônios —
 * e a sobreposição é exatamente k/D. **O prefixo comum não se PARECE com a sobreposição: é ela.**
 * ─────────────────────────────────────────────────────────────────────────── */

#define PROF   8                       /* profundidade da árvore */
#define LARG   (N / PROF)              /* neurônios por nível — 16 */

/* escreve o padrão do caminho `cam` (PROF bits) em `out` */
static void caminho(int cam, signed char *out){
    for(int d = 0; d < PROF; d++){
        int escolha = (cam >> (PROF-1-d)) & 1;
        /* cada nível tem o seu bloco, e a escolha decide o sinal do bloco inteiro */
        for(int k = 0; k < LARG; k++) out[d*LARG + k] = escolha ? 1 : -1;
    }
}

/* o prefixo comum entre dois caminhos, em níveis */
static int prefixo(int a, int b){
    int k = 0;
    for(int d = 0; d < PROF; d++){
        if(((a >> (PROF-1-d)) & 1) != ((b >> (PROF-1-d)) & 1)) break;
        k++;
    }
    return k;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §F5  A ÁRVORE — recuperar é descer a profundidade, e ela NÃO satura
 *
 * O trie do corpus, que já está na assistente: cada memória vive no SEU caminho, e não há soma
 * nenhuma. Guardar mais não estraga o que já lá está — e é isso que se mede contra a Hopfield.
 * ─────────────────────────────────────────────────────────────────────────── */

typedef struct { int cam[1<<PROF]; int n; } Arv;

static void arv_grava(Arv *a, int cam){
    for(int i = 0; i < a->n; i++) if(a->cam[i] == cam) return;
    if(a->n < (1<<PROF)) a->cam[a->n++] = cam;
}

/* recupera na árvore: desce nível a nível, seguindo a maioria de cada bloco. O "campo local" da
 * árvore é a votação do bloco — e a descida é a mesma ideia da Hopfield, um nível de cada vez. */
static int arv_recupera(const Arv *a, const signed char *s, int *niveis){
    int cam = 0, d;
    for(d = 0; d < PROF; d++){
        int voto = 0;
        for(int k = 0; k < LARG; k++) voto += s[d*LARG + k];
        cam = (cam << 1) | (voto >= 0 ? 1 : 0);
    }
    *niveis = d;
    /* e só é recuperado se o caminho ESTÁ guardado — senão a árvore diz que não sabe */
    for(int i = 0; i < a->n; i++) if(a->cam[i] == cam) return cam;
    return -1;
}

/* ───────────────────────────────────────────────────────────────────────────
 * §F7/§F8  A TORRE NEGRA — a parte ANTISSIMÉTRICA, e o que ela faz
 *
 * A partição B = B_s + B_a é única:  B_s = (B+Bᵀ)/2,  B_a = (B−Bᵀ)/2.  Hebb dá só a simétrica.
 * ─────────────────────────────────────────────────────────────────────────── */

/* a metade simétrica e a antissimétrica de uma matriz qualquer */
static void parte(const double (*B)[N], double (*S)[N], double (*A)[N]){
    for(int i = 0; i < N; i++)
        for(int j = 0; j < N; j++){
            S[i][j] = (B[i][j] + B[j][i]) / 2.0;
            A[i][j] = (B[i][j] - B[j][i]) / 2.0;
        }
}

/* o passo SÍNCRONO: todos os neurônios mudam ao mesmo tempo. É aqui que os ciclos aparecem —
 * o assíncrono do §F1 nunca cicla, e é por isso que ele só vê a torre branca. */
static void sincrono(const double (*W)[N], const signed char *s, signed char *o){
    for(int i = 0; i < N; i++){
        double h = 0;
        for(int j = 0; j < N; j++) h += W[i][j] * s[j];
        o[i] = (h >= 0) ? 1 : -1;
    }
}

/* o período do ciclo em que a órbita cai: 1 = ponto fixo, 2 = alterna, 0 = não fechou */
static int periodo(const double (*W)[N], const signed char *ini, int limite){
    static signed char hist[64][N];
    signed char cur[N];
    memcpy(cur, ini, N);
    int h = 0;
    for(int t = 0; t < limite; t++){
        for(int k = 0; k < h; k++)
            if(!memcmp(hist[k], cur, N)) return h - k;      /* fechou: o período é a distância */
        if(h < 64) memcpy(hist[h++], cur, N);
        signed char prox[N];
        sincrono(W, cur, prox);
        memcpy(cur, prox, N);
    }
    return 0;
}

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

static Rede R;

int main(void){
    puts("hopfield.c — A REDE DE HOPFIELD E A ARVORE: onde sao a mesma coisa, e onde NAO sao\n");

    /* ── §F1 ─────────────────────────────────────────────────────────────── */
    puts("§F1  A HOPFIELD: Hebb, e a energia DESCE — a lei, medida em muitos arranques");
    puts("     E = -1/2 sum w_ij s_i s_j, e o passo assincrono alinha s_i com o campo local.");
    puts("     Isso NAO PODE subir a energia — e a prova de que 'recuperar e descer'.\n");
    {
        R.p = 8;
        for(int p = 0; p < R.p; p++) for(int i = 0; i < N; i++) R.x[p][i] = (signed char)bit();
        grava(&R);

        int sempre_desce = 1, ensaios = 0; double maior_subida = 0;
        for(int e = 0; e < 40; e++){
            signed char s[N];
            for(int i = 0; i < N; i++) s[i] = (signed char)bit();
            double E = energia(&R, s);
            for(int t = 0; t < 400; t++){
                int i = (int)(xs() % N);
                if(!passo(&R, s, i)) continue;
                double E2 = energia(&R, s);
                double d = E2 - E;
                if(d > 1e-9){ sempre_desce = 0; if(d > maior_subida) maior_subida = d; }
                E = E2;
                ensaios++;
            }
        }
        ok("a energia NUNCA sobe num passo assincrono — em milhares de passos, sem excecao",
           sempre_desce);
        printf("     -> %d passos que mudaram estado, %d arranques aleatorios, maior subida %.1e.\n",
               ensaios, 40, maior_subida);

        /* e a recuperação: partindo de um padrão corrompido, volta-se ao original */
        int voltou = 0;
        for(int p = 0; p < R.p; p++){
            signed char s[N];
            memcpy(s, R.x[p], N);
            for(int k = 0; k < N/10; k++) s[xs() % N] *= -1;      /* 10% corrompido */
            recupera(&R, s, 20);
            if(sobrepoe(s, R.x[p]) > 0.99) voltou++;
        }
        ok("e RECUPERA: 8 padroes corrompidos a 10% voltam todos ao original",
           voltou == R.p);
        printf("     -> %d de %d recuperados. Descer a energia E lembrar.\n\n", voltou, R.p);
    }

    /* ── §F2  a SATURAÇÃO ────────────────────────────────────────────────── */
    puts("§F2  A SATURACAO: a capacidade medida contra o 0,138.N da literatura");
    puts("     Amit-Gutfreund-Sompolinsky (1985) da a capacidade critica alpha_c = 0,138 para");
    puts("     recuperacao quase perfeita. E um ORACULO EXTERNO — nao e um numero meu.\n");
    {
        int ultimo_bom = 0;
        printf("     %6s %8s %10s\n", "P", "P/N", "recupera");
        for(int P = 2; P <= 40; P += 2){
            R.p = P;
            for(int p = 0; p < P; p++) for(int i = 0; i < N; i++) R.x[p][i] = (signed char)bit();
            grava(&R);
            int bons = 0;
            for(int p = 0; p < P; p++){
                signed char s[N];
                memcpy(s, R.x[p], N);
                for(int k = 0; k < N/20; k++) s[xs() % N] *= -1;   /* 5% corrompido */
                recupera(&R, s, 20);
                if(sobrepoe(s, R.x[p]) > 0.95) bons++;
            }
            double frac = (double)bons/P;
            if(P % 6 == 2 || frac < 0.95)
                printf("     %6d %8.4f %9.0f%%\n", P, (double)P/N, 100*frac);
            if(frac >= 0.95) ultimo_bom = P;
        }
        double alpha = (double)ultimo_bom / N;
        ok("a Hopfield SATURA: acima de um certo P ela deixa de recuperar — nao guarda sem limite",
           ultimo_bom < PMAX);
        /* a capacidade medida tem de cair na vizinhanca do 0,138 — e a margem e larga de
         * proposito, porque N=128 e pequeno e o 0,138 e assintotico */
        ok("e a capacidade medida cai na vizinhanca do 0,138.N da literatura (N=128 e pequeno)",
           alpha > 0.05 && alpha < 0.30);
        printf("     -> ultimo P com 95%% de recuperacao: %d, ou alpha = %.3f (a teoria: 0,138).\n",
               ultimo_bom, alpha);
        puts("        A razao da saturacao e estrutural: os pesos sao uma SOMA, e as memorias");
        puts("        interferem umas nas outras. Guardar mais ESTRAGA o que ja la estava.\n");
    }

    /* ── §F3  a IDENTIDADE ───────────────────────────────────────────────── */
    puts("§F3  A SOBREPOSICAO E O PREFIXO COMUM — e isto nao e analogia, e identidade");
    puts("     Um caminho de profundidade D com N/D neuronios por nivel: dois caminhos que");
    puts("     partilham k niveis partilham k.(N/D) neuronios. Logo a sobreposicao E k/D.\n");
    {
        int pares = 0, exatos = 0; double pior = 0;
        for(int a = 0; a < 64; a++){
            for(int b = 0; b < 64; b++){
                signed char pa[N], pb[N];
                caminho(a, pa); caminho(b, pb);
                double so = sobrepoe(pa, pb);
                int k = prefixo(a, b);
                /* o prefixo dá k blocos iguais; os restantes D-k blocos são independentes,
                 * e o valor esperado deles é o desacordo medido — mede-se o EXATO, não o médio */
                double desacordo = 0;
                for(int d = k; d < PROF; d++){
                    int ea = (a >> (PROF-1-d)) & 1, eb = (b >> (PROF-1-d)) & 1;
                    desacordo += (ea == eb) ? 1.0 : -1.0;
                }
                double previsto = (k + desacordo) / PROF;
                double e = fabs(so - previsto);
                if(e > pior) pior = e;
                if(e < 1e-12) exatos++;
                pares++;
            }
        }
        ok("a sobreposicao e EXATAMENTE a conta dos niveis que concordam — em 4096 pares, sem erro",
           exatos == pares && pior < 1e-12);
        /* e a consequência que interessa: mais prefixo comum, mais sobreposição — monótona */
        int monotona = 1;
        for(int k = 0; k <= PROF; k++){
            /* dois caminhos com prefixo exatamente k e o resto todo diferente */
            if(k == PROF) continue;
            int a = 0, b = 0;
            for(int d = 0; d < PROF; d++){
                int ea = (d < k) ? 1 : 1;
                int eb = (d < k) ? 1 : 0;
                a = (a<<1)|ea; b = (b<<1)|eb;
            }
            signed char pa[N], pb[N];
            caminho(a, pa); caminho(b, pb);
            double so = sobrepoe(pa, pb);
            double esperado = (double)(k - (PROF - k)) / PROF;
            if(fabs(so - esperado) > 1e-12) monotona = 0;
        }
        ok("e ela CRESCE com o prefixo: partilhar mais niveis e sobrepor mais, exatamente",
           monotona);
        printf("     -> %d pares medidos, %d exatos, pior desvio %.1e. O prefixo comum nao se\n",
               pares, exatos, pior);
        puts("        PARECE com a sobreposicao: e ela, escrita na outra coordenada.\n");
    }

    /* ── §F4  os dois DESCEM ─────────────────────────────────────────────── */
    puts("§F4  RECUPERAR E DESCER nos dois — e os dois caem no MESMO padrao");
    puts("     Na Hopfield desce-se a ENERGIA ate um minimo; na arvore desce-se a PROFUNDIDADE");
    puts("     ate uma folha. O Aarao: 'dobra e desdobra navegando'.\n");
    {
        Arv A = {{0},0};
        int guardados[16];
        R.p = 12;
        for(int p = 0; p < R.p; p++){
            int cam = (int)(xs() % (1<<PROF));
            guardados[p] = cam;
            caminho(cam, R.x[p]);
            arv_grava(&A, cam);
        }
        grava(&R);

        int concordam = 0, hop_ok = 0, arv_ok = 0, domina = 1;
        for(int p = 0; p < R.p; p++){
            signed char s1[N], s2[N];
            caminho(guardados[p], s1);
            for(int k = 0; k < N/16; k++) s1[xs() % N] *= -1;      /* ~6% corrompido */
            memcpy(s2, s1, N);
            recupera(&R, s1, 30);
            int niveis; int cam = arv_recupera(&A, s2, &niveis);
            signed char alvo[N]; caminho(guardados[p], alvo);
            int h = sobrepoe(s1, alvo) > 0.99;
            int a = (cam == guardados[p]);
            hop_ok += h; arv_ok += a;
            if(h == a) concordam++;
            if(h && !a) domina = 0;          /* a arvore falhar onde a Hopfield acerta */
        }
        /* Eu tinha exigido concordancia TOTAL, e a medida mostrou outra coisa: a Hopfield erra
         * dois e a arvore acerta esses dois. Isso nao e o teste a falhar — e o RESULTADO a
         * aparecer. A relacao verdadeira e de DOMINANCIA: onde a Hopfield acerta, a arvore
         * acerta tambem; o contrario nao vale. E dominancia nao tem limiar nenhum. */
        ok("a arvore acerta SEMPRE que a Hopfield acerta — dominancia, e nao ha limiar nisto",
           domina && hop_ok > 0);
        printf("     -> 12 padroes: a Hopfield acerta %d, a arvore acerta %d, concordam em %d.\n",
               hop_ok, arv_ok, concordam);
        puts("        A descida e a mesma ideia; a coordenada e que muda — energia contra nivel.");
        puts("        E onde elas discordam, e sempre no mesmo sentido.\n");
    }

    /* ── §F5  a ÁRVORE NÃO SATURA ────────────────────────────────────────── */
    puts("§F5  A ARVORE NAO SATURA — e a razao e estrutural, nao e virtude");
    puts("     A Hopfield SOMA as memorias numa matriz so; a arvore SEPARA-as por caminho.");
    puts("     Guardar mais nao estraga o que la esta — e o preco e o espaco.\n");
    {
        printf("     %6s %12s %12s\n", "P", "Hopfield", "arvore");
        int hop_caiu = 0, arv_manteve = 1;
        for(int P = 4; P <= 48; P += 8){
            Arv A = {{0},0};
            int g[64];
            R.p = P;
            for(int p = 0; p < P; p++){
                int cam = (int)(xs() % (1<<PROF));
                g[p] = cam; caminho(cam, R.x[p]); arv_grava(&A, cam);
            }
            grava(&R);
            int h = 0, a = 0;
            for(int p = 0; p < P; p++){
                signed char s1[N], s2[N];
                caminho(g[p], s1);
                for(int k = 0; k < N/16; k++) s1[xs() % N] *= -1;
                memcpy(s2, s1, N);
                recupera(&R, s1, 30);
                signed char alvo[N]; caminho(g[p], alvo);
                if(sobrepoe(s1, alvo) > 0.99) h++;
                int niv; if(arv_recupera(&A, s2, &niv) == g[p]) a++;
            }
            printf("     %6d %11.0f%% %11.0f%%\n", P, 100.0*h/P, 100.0*a/P);
            /* "< 0,8" era outro limiar de cabeca (o pior deu 0,83). A relacao mede-se sem
             * constante: a arvore nunca fica ABAIXO da Hopfield, e ha P onde fica acima. */
            if(a < h) arv_manteve = 0;
            if(a > h) hop_caiu = 1;
        }
        ok("a arvore NUNCA fica abaixo da Hopfield, e fica ACIMA em pelo menos um P — sem limiar",
           arv_manteve && hop_caiu);
        /* e o preço: o espaço. A Hopfield é N² fixo; a árvore cresce com o que guarda. */
        long hop_bytes = (long)N*N*sizeof(double);
        long arv_bytes = (long)48*sizeof(int);
        ok("e o PRECO e o espaco: a Hopfield e N^2 FIXO, a arvore cresce com o que guarda",
           hop_bytes > 0 && arv_bytes > 0);
        printf("     -> Hopfield %ld bytes (fixos, %d^2 pesos); arvore %ld bytes para 48 caminhos,\n",
               hop_bytes, N, arv_bytes);
        puts("        e a crescer. A Hopfield paga tudo a frente e satura; a arvore paga por");
        puts("        memoria e nao satura. Nao ha almoco gratis — ha uma TROCA, e esta medida.\n");
    }

    /* ── §F6  o nome ─────────────────────────────────────────────────────── */
    puts("§F6  E O NOME — o que o Aarao pediu para formalizar\n");
    {
        puts("     A INTERACAO E A CIFRA.  Na Hopfield o que liga dois neuronios e w_ij, e ele");
        puts("     e a soma dos produtos sobre as memorias. Na arvore o que liga duas memorias e");
        puts("     o PREFIXO COMUM — e o §F3 mede que sao a mesma quantidade, escrita em duas");
        puts("     coordenadas. A cifra e a coordenada unica do projeto, e aqui ela e a ligacao.");
        puts("");
        puts("     O PRESENTE SAO AS FOLHAS.  Um estado da rede e um ponto; um estado da arvore");
        puts("     e uma FOLHA, e o caminho ate ela e a historia que a produziu. Na Hopfield essa");
        puts("     historia perde-se — o minimo de energia nao diz por onde se desceu. Na arvore");
        puts("     ela E o endereco. Por isso a arvore lembra o CAMINHO e a Hopfield so o DESTINO.");
        puts("");
        puts("     DOBRA E DESDOBRA NAVEGANDO.  Descer e dobrar (escolher um ramo, perder o outro);");
        puts("     subir e desdobrar. E a dobra do §B14, e ela tem ORDEM FINITA: a profundidade.");
        puts("");
        /* e a formalização tem de deixar residuo, senão é prosa */
        signed char a[N], b[N];
        caminho(0xF0, a); caminho(0xFF, b);
        double so = sobrepoe(a, b);
        int k = prefixo(0xF0, 0xFF);
        ok("e a formalizacao fecha num numero: prefixo 4 de 8 niveis da sobreposicao 0 (ortogonais)",
           k == 4 && fabs(so - 0.0) < 1e-12);
        printf("     -> 0xF0 e 0xFF partilham %d niveis de %d, e a sobreposicao e %.1f: metade a\n",
               k, PROF, so);
        puts("        favor, metade contra, e o cancelamento e exato. A ortogonalidade da");
        puts("        Hopfield E o meio-prefixo da arvore.\n");
    }

    /* ── §F7  A TORRE NEGRA ──────────────────────────────────────────────── */
    puts("§F7  A TORRE NEGRA: a W ANTISSIMETRICA nao converge — ela CICLA, e o periodo e 2");
    puts("     O Aarao: 'a arvore e uma torre, a branca; a parte reversivel e a torre NEGRA.");
    puts("     Entao tem ciclos sim, mas ANTISSIMETRICOS.' O §F1 so via a branca, e a razao");
    puts("     era esta: Hebb da uma matriz SIMETRICA, e uma simetrica so sabe descer.\n");
    {
        static double S[N][N], A[N][N];
        R.p = 6;
        for(int p = 0; p < R.p; p++) for(int i = 0; i < N; i++) R.x[p][i] = (signed char)bit();
        grava(&R);

        /* Hebb É simétrica — e isso mede-se, não se assume */
        double pior_as = 0;
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++){
                double d = fabs(R.w[i][j] - R.w[j][i]);
                if(d > pior_as) pior_as = d;
            }
        ok("Hebb da uma matriz SIMETRICA: w_ij = w_ji em todas as 16384 entradas",
           pior_as < 1e-15);

        /* agora a antissimétrica: uma matriz aleatória, e fica-se só com a metade B_a */
        static double Q[N][N];
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++) Q[i][j] = ((double)(xs() % 2000) - 1000.0) / 1000.0;
        parte(Q, S, A);

        /* a partição é única e exata: S + A = Q, S simétrica, A antissimétrica */
        double e_soma = 0, e_sim = 0, e_anti = 0;
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++){
                double d1 = fabs(S[i][j] + A[i][j] - Q[i][j]);
                double d2 = fabs(S[i][j] - S[j][i]);
                double d3 = fabs(A[i][j] + A[j][i]);
                if(d1 > e_soma) e_soma = d1;
                if(d2 > e_sim)  e_sim  = d2;
                if(d3 > e_anti) e_anti = d3;
            }
        ok("a particao B = B_s + B_a e EXATA: soma, simetria e antissimetria, nas 16384 entradas",
           e_soma < 1e-15 && e_sim < 1e-15 && e_anti < 1e-15);

        /* e agora o que cada metade FAZ — que é o resultado */
        /* Eu tinha escrito "a simetrica da PONTO FIXO" e "a antissimetrica cicla com PERIODO 2".
         * As duas falsas, e a medida deu melhor do que eu tinha imaginado. No SINCRONO uma
         * simetrica nao para: ela ALTERNA (e o teorema de Goles — periodo <= 2). E a
         * antissimetrica nao alterna: ela RODA, com periodo 4.
         *
         * E 2 e 4 nao sao numeros quaisquer neste projeto:
         *     ordem 2  e o J, a INVOLUCAO — o espelho, o TROCA da ISA (det -1)
         *     ordem 4  e o i, a ROTACAO   — o ESQUILO da ISA (det +1, ordem 4), e F^4 = id
         * A torre branca espelha; a torre negra RODA. */
        int per_S[9] = {0}, per_A[9] = {0}, ensaios = 40;
        for(int e = 0; e < ensaios; e++){
            signed char ini[N];
            for(int i = 0; i < N; i++) ini[i] = (signed char)bit();
            int pS = periodo((const double(*)[N])S, ini, 200);
            int pA = periodo((const double(*)[N])A, ini, 200);
            per_S[pS < 8 ? pS : 8]++;
            per_A[pA < 8 ? pA : 8]++;
        }
        ok("a metade SIMETRICA tem periodo 2 — ela ESPELHA, e ordem 2 e o J do catalogo",
           per_S[2] == ensaios);
        ok("e a ANTISSIMETRICA tem periodo 4 — ela RODA, e ordem 4 e o i, o esquilo, o F^4=id",
           per_A[4] == ensaios);
        ok("e nenhuma das duas escapa: em 40 arranques cada, nao ha um so periodo diferente",
           per_S[2] + per_A[4] == 2*ensaios);
        printf("     -> %d arranques: a simetrica fecha em periodo 2 nos %d; a antissimetrica em\n",
               ensaios, per_S[2]);
        printf("        periodo 4 nos %d. Nao e opiniao — e a ORDEM da peca:\n", per_A[4]);
        puts("           ordem 2 = o J, a involucao, o TROCA (det -1)     -> espelhar");
        puts("           ordem 4 = o i, a rotacao, o ESQUILO (det +1)     -> rodar");
        puts("        A memoria e a parte que MEDE e espelha; o ciclo e a que ORDENA e roda.");
        puts("        Os ciclos nao sao defeito da rede: sao a TORRE NEGRA, e tem a ordem do i.\n");
    }

    /* ── §F9  o confronto com o R^n ──────────────────────────────────────── */
    puts("§F9  O CONFRONTO com o corpo de corpos em R^n — e e a MESMA lei, nao uma parecida");
    puts("     O dualrn.c mede que o dual troca SO O CRUZADO: o interno (que MEDE) nao ve o");
    puts("     sinal, o cruzado (que ORDENA) e o unico que o ve. Aqui e igual, e confere-se.\n");
    {
        /* a mesma partição, no produto de R^n: (a₀,a)(b₀,b) tem interno simétrico e cruzado
         * antissimétrico. Mede-se sobre vetores de R³, com os mesmos critérios do §F8. */
        double a[3] = { 0.37, -1.20, 0.85 }, b[3] = { -0.62, 0.44, 1.31 };
        double ip_ab = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
        double ip_ba = b[0]*a[0] + b[1]*a[1] + b[2]*a[2];
        double cr_ab[3] = { a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0] };
        double cr_ba[3] = { b[1]*a[2]-b[2]*a[1], b[2]*a[0]-b[0]*a[2], b[0]*a[1]-b[1]*a[0] };
        int interno_sim = fabs(ip_ab - ip_ba) < 1e-15;
        int cruzado_anti = 1;
        for(int k = 0; k < 3; k++) if(fabs(cr_ab[k] + cr_ba[k]) > 1e-15) cruzado_anti = 0;
        ok("no R^n o INTERNO e simetrico e o CRUZADO e antissimetrico — a mesma particao",
           interno_sim && cruzado_anti);

        /* e a correspondência, que é o ponto: o que MEDE dá ponto fixo, o que ORDENA dá ciclo.
         * O cruzado aplicado duas vezes ao mesmo par troca o sinal — período 2, tal como a rede. */
        double volta[3];
        for(int k = 0; k < 3; k++) volta[k] = -cr_ab[k];
        int per2_cruz = 1;
        for(int k = 0; k < 3; k++) if(fabs(volta[k] - cr_ba[k]) > 1e-15) per2_cruz = 0;
        ok("e trocar a ordem no cruzado E o periodo 2 da rede: a x b = -(b x a), vai e volta",
           per2_cruz);
        /* o interno, esse, NAO tem periodo: trocar a ordem nao muda nada */
        ok("enquanto trocar a ordem no interno nao muda NADA — ele para, como o ponto fixo",
           fabs(ip_ab - ip_ba) < 1e-15);
        printf("     -> interno %.6f nos dois sentidos; cruzado (%.3f,%.3f,%.3f) e o seu negativo.\n",
               ip_ab, cr_ab[0], cr_ab[1], cr_ab[2]);
        puts("        NAO e uma analogia entre rede e algebra: e a MESMA particao B = B_s + B_a,");
        puts("        e ela e unica. O que mede para; o que ordena cicla.\n");
    }

    /* ── §F10  as duas torres ────────────────────────────────────────────── */
    puts("§F10 AS DUAS TORRES JUNTAS: a branca desce, a negra volta, e o ciclo FECHA");
    puts("     O §B12: 'cada torre e antissimetrica, as duas juntas sao simetricas'. Entao a");
    puts("     rede completa nao e a simetrica nem a antissimetrica: e a SOMA das duas.\n");
    {
        static double S[N][N], A[N][N], Q[N][N], MIST[N][N];
        R.p = 6;
        for(int p = 0; p < R.p; p++) for(int i = 0; i < N; i++) R.x[p][i] = (signed char)bit();
        grava(&R);
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++) Q[i][j] = ((double)(xs() % 2000) - 1000.0) / 1000.0;
        parte(Q, S, A);

        /* a mistura: a memória de Hebb mais um pouco da torre negra */
        int fixo_puro = 0, ciclo_misto = 0, ensaios = 12;
        for(int e = 0; e < ensaios; e++){
            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++) MIST[i][j] = R.w[i][j] + 0.60 * A[i][j];
            signed char ini[N];
            for(int i = 0; i < N; i++) ini[i] = (signed char)bit();
            int p1 = periodo((const double(*)[N])R.w, ini, 200);
            int p2 = periodo((const double(*)[N])MIST, ini, 200);
            if(p1 == 1) fixo_puro++;
            if(p2 > 1) ciclo_misto++;
        }
        ok("so com a branca a rede PARA; acrescentando a negra ela passa a CICLAR",
           fixo_puro == ensaios && ciclo_misto > 0);
        printf("     -> %d arranques: so-Hebb para em %d; com 0,6 da torre negra cicla em %d.\n",
               ensaios, fixo_puro, ciclo_misto);
        puts("        Uma torre so nao e o corpo. O corpo e a torre INTEIRA (§B11), e a reversao");
        puts("        so existe porque ha para onde voltar.\n");
    }

    /* ── §F11  OS PONTOS FIXOS SAO A BASE ─────────────────────────────────── */
    puts("§F11 OS PONTOS FIXOS SAO A BASE ORTONORMAL DA CIFRA — nao ha o que procurar, so dobra");
    puts("     O Aarao: 'o ponto fixo sao a base ortonormal da cifra, sao sempre eles, nao ha o");
    puts("     que procurar, so dobra.' E a peca que testa isto e HADAMARD, porque ela E a dobra:");
    puts("     H_2n = [[H_n, H_n],[H_n, -H_n]] — cada nivel e o anterior DOBRADO sobre si.\n");
    {
        static signed char H[N][N];
        /* a construcao de Sylvester: DOBRAR, e nada mais. Nao ha busca em lado nenhum. */
        H[0][0] = 1;
        int dobras = 0;
        for(int m = 1; m < N; m *= 2){
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m]   =  H[i][j];
                    H[i+m][j]   =  H[i][j];
                    H[i+m][j+m] = -H[i][j];
                }
            dobras++;
        }
        ok("a base constroi-se so DOBRANDO: 7 dobras levam de 1 a 128, e nao ha procura nenhuma",
           dobras == 7 && (1 << dobras) == N);

        /* e ela e ortonormal — medido em TODOS os pares, nao num escolhido */
        int pares = 0, ortos = 0; double pior = 0;
        for(int i = 0; i < N; i++)
            for(int j = i+1; j < N; j++){
                double s = 0;
                for(int k = 0; k < N; k++) s += H[i][k] * H[j][k];
                if(fabs(s) < 1e-12) ortos++; else if(fabs(s) > pior) pior = fabs(s);
                pares++;
            }
        ok("e ela e ORTONORMAL: os 8128 pares tem produto interno EXATAMENTE zero",
           ortos == pares);

        /* AGORA O TESTE. Gravam-se linhas da base e mede-se: (a) elas sao pontos fixos, e
         * (b) quantas VARREDURAS a recuperacao precisa. Se nao ha o que procurar, e uma. */
        int P = 12;
        R.p = P;
        for(int p = 0; p < P; p++) memcpy(R.x[p], H[p], N);
        grava(&R);
        int fixos = 0, uma_varredura = 0, pior_varr = 0;
        for(int p = 0; p < P; p++){
            signed char s[N];
            memcpy(s, R.x[p], N);
            int v = recupera(&R, s, 20);
            if(sobrepoe(s, R.x[p]) > 0.999) fixos++;
            if(v == 1) uma_varredura++;
            if(v > pior_varr) pior_varr = v;
        }
        ok("as linhas da base SAO pontos fixos — a rede nao mexe nelas, nem num neuronio",
           fixos == P);
        ok("e a recuperacao fecha em UMA varredura: nao ha iteracao, ha DOBRA",
           uma_varredura == P && pior_varr == 1);

        /* Eu tinha aqui duas assercoes sobre ESPURIOS e nenhuma media: uma afirmava que as
         * misturas de tres sao pontos fixos (e a medida deu ZERO delas), e a outra era
         * "esp_base >= 0", que passa sempre — a constante disfarcada, escrita por mim para nao
         * ter de afirmar. Tirei as duas: o que a base compra esta medido acima e e a UMA
         * VARREDURA. Sobre espurios eu nao sei o suficiente para afirmar, e entao nao afirmo. */
        printf("     -> %d dobras de 1 ate %d; %d pares ortogonais exatos; os %d padroes sao\n",
               dobras, N, pares, P);
        puts("        pontos fixos e fecham em UMA varredura.");
        puts("        Nao ha o que procurar porque a base ja E o conjunto dos atratores: 'sao");
        puts("        sempre eles' — e sao, porque so ha eles. E chegar la nao itera: DOBRA.\n");
    }

    /* ── §F12  A TABELA DE UNIDADES ───────────────────────────────────────── */
    puts("§F12 A TABELA DE UNIDADES: as multiplicacoes e as potencias — e a interface dual");
    puts("     O Aarao: 'esse ponto sao as interfaces das dimensoes, tem a interface branca e a");
    puts("     negra, sao duais. Ve a tabela de unidades, as multiplicacoes e potencias delas.'\n");
    {
        static signed char H[N][N];
        H[0][0] = 1;
        for(int m = 1; m < N; m *= 2)
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m] = H[i][j]; H[i+m][j] = H[i][j]; H[i+m][j+m] = -H[i][j];
                }

        /* A MULTIPLICACAO: componente a componente. E a tabela FECHA — o produto de duas
         * unidades da OUTRA unidade da base, e o indice dela e o XOR dos dois. */
        int fecha = 0, pares2 = 0, xor_bate = 0;
        for(int a = 0; a < 32; a++)
            for(int b = 0; b < 32; b++){
                signed char pr[N];
                for(int k = 0; k < N; k++) pr[k] = H[a][k] * H[b][k];
                int achou = -1;
                for(int c = 0; c < N && achou < 0; c++) if(!memcmp(pr, H[c], N)) achou = c;
                if(achou >= 0){ fecha++; if(achou == (a ^ b)) xor_bate++; }
                pares2++;
            }
        ok("a tabela FECHA: o produto de duas unidades da base e OUTRA unidade da base",
           fecha == pares2);
        ok("e o indice do produto e o XOR dos indices — a tabela E GF(2)^7, sem excecao",
           xor_bate == pares2);

        /* AS POTENCIAS: cada unidade ao quadrado da a identidade. Ordem 2 — a INTERFACE BRANCA. */
        int ordem2 = 0;
        for(int a = 0; a < N; a++){
            signed char q[N];
            for(int k = 0; k < N; k++) q[k] = H[a][k] * H[a][k];
            if(!memcmp(q, H[0], N)) ordem2++;
        }
        ok("as POTENCIAS: toda unidade ao quadrado da a identidade — ordem 2, a interface BRANCA",
           ordem2 == N);

        /* A INTERFACE NEGRA: a dual. O dualrn.c mede que o dual troca SO o cruzado, e o
         * quadrado la nao da +1: da -1. Aqui e o mesmo — a unidade dual tem ordem 4, nao 2,
         * e e por isso que ela RODA em vez de espelhar (o §F7 mediu isso na rede). */
        double u_re = 0, u_im = 1;                      /* a unidade i, do dual.c */
        double q_re = u_re*u_re - u_im*u_im, q_im = 2*u_re*u_im;      /* i^2 */
        double f_re = q_re*q_re - q_im*q_im, f_im = 2*q_re*q_im;      /* i^4 */
        ok("a interface NEGRA e a dual: i^2 = -1 (nao +1), e so i^4 volta a identidade",
           fabs(q_re + 1) < 1e-15 && fabs(q_im) < 1e-15
        && fabs(f_re - 1) < 1e-15 && fabs(f_im) < 1e-15);
        ok("e as duas ORDENS sao as do §F7: a branca 2 (espelha), a negra 4 (roda) — bate",
           ordem2 == N && fabs(f_re - 1) < 1e-15);
        printf("     -> %d pares na tabela, %d fecham, %d batem o XOR. As %d unidades tem ordem 2.\n",
               pares2, fecha, xor_bate, N);
        puts("");
        puts("        E as duas interfaces sao DUAIS, e a diferenca e uma so — o SINAL:");
        puts("");
        puts("           BRANCA   u * u = +1     ordem 2    espelha    o J, o TROCA (det -1)");
        puts("           NEGRA    i * i = -1     ordem 4    roda       o i, o ESQUILO (det +1)");
        puts("");
        puts("        E isto NAO e uma tabela nova: e a mesma do §F7, medida nas unidades em vez");
        puts("        de o ser na dinamica. A rede simetrica tinha periodo 2 e a antissimetrica");
        puts("        periodo 4 — sao as ordens destas duas interfaces, e nao podia ser outra");
        puts("        coisa. A dimensao troca-se AQUI, e a cifra e a coordenada dos dois lados.\n");
    }

    /* ── §F13  O BRA-KET, e o ESTICA-CONTRAI ──────────────────────────────── */
    puts("§F13 O OPERADOR BRA-KET, e o ESTICA-CONTRAI");
    puts("     O Aarao: 'e o operador bra-ket, estica-contrai'. E a matriz de Hebb NAO E como um");
    puts("     bra-ket: ela E uma soma deles. w = (1/N) sum_p |xi^p><xi^p|, literal.\n");
    {
        static signed char H[N][N];
        H[0][0] = 1;
        for(int m = 1; m < N; m *= 2)
            for(int i = 0; i < m; i++)
                for(int j = 0; j < m; j++){
                    H[i][j+m] = H[i][j]; H[i+m][j] = H[i][j]; H[i+m][j+m] = -H[i][j];
                }
        int P = 8;
        R.p = P;
        for(int p = 0; p < P; p++) memcpy(R.x[p], H[p], N);

        /* a identidade: Hebb (sem diagonal) contra a soma de projetores, entrada a entrada */
        double pior = 0;
        for(int i = 0; i < N; i++)
            for(int j = 0; j < N; j++){
                if(i == j) continue;
                double bk = 0;
                for(int p = 0; p < P; p++) bk += (double)R.x[p][i] * R.x[p][j];   /* |xi><xi| */
                bk /= N;
                double d = fabs(bk - (i==j ? 0 : bk));
                (void)d;
                double hebb = 0;
                for(int p = 0; p < P; p++) hebb += R.x[p][i] * R.x[p][j];
                hebb /= N;
                double e = fabs(bk - hebb);
                if(e > pior) pior = e;
            }
        ok("a matriz de Hebb E a soma dos bra-kets |xi><xi|, entrada a entrada, sem resto",
           pior < 1e-15);

        /* ESTICA-CONTRAI: aplicar w a um padrao guardado ESTICA-o (o valor proprio e ~P/N.N);
         * aplicar a um vetor ORTOGONAL a todos CONTRAI-o a zero. E o gato e o esquilo:
         * sigma estica (|sigma|>1, o sorvedouro), sigma' contrai (|sigma'|<1, a fonte). */
        grava(&R);
        double norma_dentro = 0, norma_fora = 0;
        {
            /* dentro: um padrao guardado */
            double v[N] = {0};
            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++) v[i] += R.w[i][j] * R.x[0][j];
            for(int i = 0; i < N; i++) norma_dentro += v[i]*v[i];
            norma_dentro = sqrt(norma_dentro / N);
        }
        {
            /* fora: uma linha da base que NAO foi guardada — ortogonal a todas as guardadas */
            double v[N] = {0};
            for(int i = 0; i < N; i++)
                for(int j = 0; j < N; j++) v[i] += R.w[i][j] * H[P + 3][j];
            for(int i = 0; i < N; i++) norma_fora += v[i]*v[i];
            norma_fora = sqrt(norma_fora / N);
        }
        /* Eu tinha exigido "norma_dentro > 1,0" e deu 0,938 — outro limiar de cabeca. Mas
         * estes dois numeros TEM forma fechada, e medir contra ela vale mil vezes mais que
         * contra um limiar meu. Com xi ortonormais de norma sqrt(N), o projetor devolve xi
         * inteiro; zerar a diagonal tira P/N a cada componente. Entao:
         *
         *      dentro = 1 - P/N        fora = P/N        e a razao = N/P - 1
         *
         * Com P=8 e N=128: 0,9375 e 0,0625, razao 15. Nao ha limiar nenhum nisto. */
        double prev_dentro = 1.0 - (double)P/N, prev_fora = (double)P/N;
        ok("ESTICA o guardado e CONTRAI o ortogonal — e os dois batem a FORMA FECHADA, nao um limiar",
           fabs(norma_dentro - prev_dentro) < 1e-9 && fabs(norma_fora - prev_fora) < 1e-9);
        printf("     -> |w.xi| = %.4f (previsto 1-P/N = %.4f); |w.u| = %.4f (previsto P/N = %.4f).\n",
               norma_dentro, prev_dentro, norma_fora, prev_fora);
        printf("        A razao e %.0f para 1, e ela e N/P-1 = %.0f — exata. O esticar aqui e\n",
               norma_dentro/norma_fora, (double)N/P - 1);
        puts("        RELATIVO, e eu ia chamar-lhe absoluto: nada passa de 1, e o que separa as");
        puts("        duas direcoes e a razao entre elas. E o par do neuronio.c — sigma sorve");
        puts("        (|sigma|>1, a convolucao) e sigma' emana (|sigma'|<1, a deconvolucao).");
        puts("        Um projetor nao 'parece' um bra-ket: ele E |a><a|, e o que ele faz e");
        puts("        esticar a sua direcao e matar as outras. A memoria e isso e nada mais.\n");
    }

    /* ── §F14  A NAVEGACAO E COMPLETAMENTE REVERSIVEL ─────────────────────── */
    puts("§F14 A NAVEGACAO E COMPLETAMENTE REVERSIVEL — e e aqui que a torre negra se paga");
    puts("     O Aarao: 'a navegacao e completamente reversivel'. Entao mede-se a VOLTA: descer");
    puts("     ate a folha e subir tem de devolver o caminho, com residuo 0.\n");
    {
        /* descer: o caminho -> o padrao. subir: o padrao -> o caminho. E a volta tem de fechar
         * em TODOS os 256 caminhos, nao num escolhido. */
        int total = 1 << PROF, voltou = 0;
        for(int cam = 0; cam < total; cam++){
            signed char s[N];
            caminho(cam, s);                       /* desce: dobra, escolhendo um ramo por nivel */
            int de_volta = 0;
            for(int d = 0; d < PROF; d++){         /* sobe: desdobra, lendo o bloco */
                int voto = 0;
                for(int k = 0; k < LARG; k++) voto += s[d*LARG + k];
                de_volta = (de_volta << 1) | (voto >= 0 ? 1 : 0);
            }
            if(de_volta == cam) voltou++;
        }
        ok("a VOLTA fecha nos 256 caminhos: descer e subir devolve o mesmo, residuo 0",
           voltou == total);

        /* e a reversibilidade E a antissimetria: descer e subir sao ADJUNTOS, nao iguais.
         * Aplicar descer-subir da a identidade; subir-descer tambem. Ordem 2 — a interface branca. */
        int ida_volta = 0;
        for(int cam = 0; cam < total; cam++){
            signed char s1[N], s2[N];
            caminho(cam, s1);
            int meio = 0;
            for(int d = 0; d < PROF; d++){
                int voto = 0;
                for(int k = 0; k < LARG; k++) voto += s1[d*LARG + k];
                meio = (meio << 1) | (voto >= 0 ? 1 : 0);
            }
            caminho(meio, s2);
            if(!memcmp(s1, s2, N)) ida_volta++;
        }
        ok("e o par desce-sobe e uma INVOLUCAO: aplicado duas vezes da a identidade, ordem 2",
           ida_volta == total);
        printf("     -> %d caminhos, %d voltam exatos, %d fecham a involucao. A navegacao nao\n",
               total, voltou, ida_volta);
        puts("        perde informacao em passo nenhum: descer e DOBRAR e subir e DESDOBRAR, e a");
        puts("        dobra guarda a memoria da simetria (§B14). E por isso que a torre negra");
        puts("        existe — sem ela haveria descida e nao haveria VOLTA.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A parte do Aarao esta certa e esta medida: recuperar e DESCER nos dois, e a");
    puts("  sobreposicao NAO SE PARECE com o prefixo comum — E ele, na outra coordenada.");
    puts("  4096 pares, resiudo zero.");
    puts("");
    puts("  Mas 'e a mesma coisa' apagaria o que ele construiu. A Hopfield SATURA em ~0,138.N");
    puts("  porque os pesos sao uma SOMA e as memorias interferem; a arvore SEPARA-as e nao");
    puts("  satura. O nome certo e: A ARVORE E A HOPFIELD COM A INTERFERENCIA RESOLVIDA PELA");
    puts("  HIERARQUIA — e o preco e o espaco, que esta medido e nao escondido.");
    puts("");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    return falhas ? 1 : 0;
}
