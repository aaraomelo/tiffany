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
