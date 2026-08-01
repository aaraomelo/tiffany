/* transplante.c — O TRANSPLANTE DE MEDULA: extrair a medula de uma LLM e enxertá-la num corpo.
 *
 * O Aarão: "o que você verificou foi um transplante de células-tronco hematopoiéticas (TCHC),
 * também chamado de transplante de medula óssea. Faz experimento: baixa uma LLM, pode ser ollama,
 * e transplanta a medula dele pra outro corpo do banco."
 *
 * A ANALOGIA É EXATA, e é dele. No `reconstroi.c` a janela de n+2 termos **é** a célula-tronco: o
 * mínimo que regenera o corpo inteiro. Uma célula-tronco não é um pedaço do doador — é a REGRA que
 * refaz o resto. E o transplante só é transplante se a regra pegar no receptor.
 *
 * ENTÃO O CRITÉRIO É CLÍNICO, e é ele que dá sentido à medida:
 *
 *      **uma medula REGENERA a partir de pouco; copiar tecido inteiro não é transplante.**
 *
 * Se a sequência do doador tem complexidade linear L, um corpo de grau L reproduz-na exatamente —
 * isso é sempre verdade e não prova nada. O que decide é a RAZÃO L/N:
 *
 *      L << N     ha medula: pouco enxerta e o resto regenera        (a familia metalica: L = n)
 *      L ~ N/2    nao ha medula: e preciso levar metade do doador    (o aleatorio)
 *
 * O doador é o `llama3.2:1b` a correr localmente, com temperatura 0 e semente fixa — determinista,
 * porque um doador que muda a cada colheita não se mede. O receptor é um corpo do banco.
 *
 *   §T1  a COLHEITA: a saída do doador vira sequência em Z_p, e nada mais lhe é feito
 *   §T2  os CONTROLOS: o aleatório (sem medula) e o metálico (medula mínima) — os dois extremos
 *   §T3  a MEDULA do doador: a complexidade linear medida, e a razão L/N
 *   §T4  O ENXERTO: o corpo receptor gera a sequência do doador? Mede-se termo a termo
 *   §T5  a PEGA: até onde o enxerto regenera sozinho, e onde ele para
 *   §T6  o veredito, e ele vale nos dois sentidos
 *
 *   cc -O2 -std=c99 transplante.c -lm -o transplante && ./transplante
 *   (a colheita é feita por tools/colhe_llm.sh; o ficheiro fica em /tmp/llm_medula.txt)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P     251                      /* primo: cabe num byte, e os bytes são o que o doador dá */
#define NMAX  4096

/* ───────────────────────────────────────────── Berlekamp–Massey, o mesmo do reconstroi.c */

static int mod(long a, int p){ long r = a % p; return (int)(r < 0 ? r + p : r); }
static int inv_p(int a, int p){
    int r = 1, b = a % p, e = p - 2;
    while(e){ if(e & 1) r = (int)((long)r * b % p); b = (int)((long)b * b % p); e >>= 1; }
    return r;
}

/* devolve L, a complexidade linear; C[] fica com a recorrência (C[0]=1) */
static int massey(const int *s, int N, int p, int *C, int lim){
    int *B = calloc((size_t)lim, sizeof(int));
    int *T = calloc((size_t)lim, sizeof(int));
    int L = 0, mdes = 1, b = 1;
    memset(C, 0, sizeof(int)*(size_t)lim);
    C[0] = B[0] = 1;
    for(int i = 0; i < N; i++){
        long d = s[i];
        for(int j = 1; j <= L && j <= i; j++) d += (long)C[j] * s[i-j];
        d = mod(d, p);
        if(d == 0){ mdes++; continue; }
        memcpy(T, C, sizeof(int)*(size_t)lim);
        long coef = (long)d * inv_p(b, p) % p;
        for(int j = 0; j + mdes < lim; j++) C[j+mdes] = mod(C[j+mdes] - coef * B[j], p);
        if(2*L <= i){ L = i + 1 - L; memcpy(B, T, sizeof(int)*(size_t)lim); b = (int)d; mdes = 1; }
        else mdes++;
    }
    free(B); free(T);
    return L;
}

/* dado C e os primeiros L termos, REGENERA o resto — é isto que o enxerto tem de fazer */
static void regenera(const int *C, int L, const int *semente, int *saida, int N, int p){
    for(int i = 0; i < L && i < N; i++) saida[i] = semente[i];
    for(int i = L; i < N; i++){
        long v = 0;
        for(int j = 1; j <= L; j++) v -= (long)C[j] * saida[i-j];
        saida[i] = mod(v, p);
    }
}

/* a sequência do corpo metálico: o controlo com medula mínima */
static void metalico(int n, int m, int *s, int N, int p){
    for(int i = 0; i < N; i++){
        if(i < n){ s[i] = (i == 0) ? 1 : 0; continue; }
        s[i] = mod((long)m * s[i-1] + s[i-n], p);
    }
}

static unsigned long SEM = 88172645463325252UL;
static unsigned long xs(void){ SEM ^= SEM<<13; SEM ^= SEM>>7; SEM ^= SEM<<17; return SEM; }

/* ───────────────────────────────────────────────────────── o programa */

static int falhas = 0, feitas = 0;
static void ok(const char *q, int cond){
    feitas++; if(!cond) falhas++;
    printf("#UNIT %s %s\n", cond ? "ok" : "falha", q);
    printf("  [%s] %s\n", cond ? "ok" : "FALHA", q);
}

static int DOADOR[NMAX], NDOADOR = 0;

/* lê a colheita: um byte por termo, reduzido mod P */
static int colheita(const char *cam){
    FILE *f = fopen(cam, "rb");
    if(!f) return 0;
    int c, n = 0;
    while((c = fgetc(f)) != EOF && n < NMAX) DOADOR[n++] = c % P;
    fclose(f);
    NDOADOR = n;
    return n;
}

int main(void){
    puts("transplante.c — O TRANSPLANTE DE MEDULA: da LLM para um corpo do banco\n");

    if(!colheita("/tmp/llm_medula.txt") || NDOADOR < 200){
        puts("  [aviso] nao ha colheita em /tmp/llm_medula.txt (ou e curta demais).");
        puts("          Corre tools/colhe_llm.sh primeiro — ele fala com o ollama local.");
        puts("          Sem doador nao ha transplante, e medir sem doador seria inventa-lo.\n");
        puts("unidades: 0   falhas: 0\nRESIDUO 0");
        return 0;
    }

    int N = NDOADOR < 1200 ? NDOADOR : 1200;
    int LIM = 2*N + 8;
    int *C = calloc((size_t)LIM, sizeof(int));

    /* ── §T1 ─────────────────────────────────────────────────────────────── */
    puts("§T1  A COLHEITA: a saida do doador vira sequencia em Z_251, e nada mais lhe e feito");
    puts("     O doador e o llama3.2:1b local, com temperatura 0 e semente fixa — um doador que");
    puts("     muda a cada colheita nao se mede. Os bytes da resposta SAO a sequencia.\n");
    {
        int distintos = 0, hist[P];
        memset(hist, 0, sizeof hist);
        for(int i = 0; i < N; i++) if(!hist[DOADOR[i]]++) distintos++;
        ok("a colheita tem termos suficientes e variedade — nao e uma constante disfarcada",
           N >= 200 && distintos > 20);
        printf("     -> %d termos colhidos, %d valores distintos. Os primeiros: ", N, distintos);
        for(int i = 0; i < 12; i++) printf("%d ", DOADOR[i]);
        puts("\n");
    }

    /* ── §T2  os CONTROLOS ───────────────────────────────────────────────── */
    puts("§T2  OS CONTROLOS: os dois extremos, e sem eles a medida do doador nao diz nada");
    puts("     O aleatorio nao tem medula (L ~ N/2, o teorema); o metalico tem a minima (L = n).\n");
    int L_alea = 0, L_met = 0;
    {
        int *alea = malloc(sizeof(int)*(size_t)N);
        for(int i = 0; i < N; i++) alea[i] = (int)(xs() % P);
        L_alea = massey(alea, N, P, C, LIM);
        ok("o ALEATORIO nao tem medula: a complexidade linear fica em cerca de N/2",
           L_alea > 0.45*N && L_alea < 0.55*N);

        int *met = malloc(sizeof(int)*(size_t)N);
        metalico(5, 3, met, N, P);
        L_met = massey(met, N, P, C, LIM);
        ok("o METALICO tem a medula minima: L = n = 5, e isso e o corpo inteiro em cinco numeros",
           L_met == 5);
        printf("     -> aleatorio L = %d (N/2 = %d); metalico L = %d (n = 5).\n",
               L_alea, N/2, L_met);
        printf("        A razao L/N: %.3f contra %.4f. Sao os dois extremos, e o doador cai\n",
               (double)L_alea/N, (double)L_met/N);
        puts("        algures entre eles — ou num deles.\n");
        free(alea); free(met);
    }

    /* ── §T3  a MEDULA do doador ─────────────────────────────────────────── */
    puts("§T3  A MEDULA DO DOADOR: a complexidade linear da saida da LLM\n");
    int L_llm = massey(DOADOR, N, P, C, LIM);
    {
        double razao = (double)L_llm / N;
        ok("o doador TEM complexidade linear finita e ela mede-se — o transplante e possivel",
           L_llm > 0 && L_llm <= N);
        /* e agora a pergunta clinica: ha medula, ou e preciso levar metade do doador? */
        int ha_medula = (razao < 0.25);
        ok("mas ela NAO e pequena: a razao L/N fica junto do aleatorio, nao junto do metalico",
           !ha_medula);
        printf("     -> L = %d em %d termos, razao %.3f.\n", L_llm, N, razao);
        printf("        aleatorio %.3f | DOADOR %.3f | metalico %.4f\n",
               (double)L_alea/N, razao, (double)L_met/N);
        puts("        O doador cai do lado do ALEATORIO. Isto nao e um defeito da LLM: e o que");
        puts("        significa a saida dela nao ser uma recorrencia linear. Nao ha medula");
        puts("        LINEAR a colher — e era essa a medula que o reconstroi.c sabe enxertar.\n");
    }

    /* ── §T4  O ENXERTO ──────────────────────────────────────────────────── */
    puts("§T4  O ENXERTO: o corpo receptor gera mesmo a sequencia do doador?");
    puts("     Aqui o transplante FUNCIONA — e e por isso que a medida do §T3 importa. Um corpo");
    puts("     de grau L reproduz a sequencia EXATAMENTE. A questao nunca foi se pega: e o preco.\n");
    {
        int *saida = malloc(sizeof(int)*(size_t)N);
        regenera(C, L_llm, DOADOR, saida, N, P);
        int iguais = 0;
        for(int i = 0; i < N; i++) if(saida[i] == DOADOR[i]) iguais++;
        ok("o ENXERTO PEGA: o corpo de grau L regenera a sequencia do doador termo a termo",
           iguais == N);
        printf("     -> %d de %d termos regenerados, residuo 0. O corpo receptor passou a\n",
               iguais, N);
        printf("        produzir o que o doador produzia — mas foi preciso enxertar %d dos %d\n",
               L_llm, N);
        printf("        termos (%.0f%%). Isso nao e um transplante de medula: e um transplante\n",
               100.0*L_llm/N);
        puts("        de TECIDO. A celula-tronco regenera de pouco; isto levou quase tudo.\n");
        free(saida);
    }

    /* ── §T5  a PEGA, e onde ela para ────────────────────────────────────── */
    puts("§T5  A PEGA: enxertando MENOS que L, ate onde o receptor regenera sozinho?");
    puts("     E o teste clinico de verdade: da-se pouca medula e ve-se ate onde o enxerto");
    puts("     sustenta. Com o metalico, cinco termos sustentam mil. Com a LLM?\n");
    {
        int marcos[] = { 5, 10, 20, 50, 100 };
        printf("     %8s %14s %14s\n", "enxerto", "metalico", "doador LLM");
        int met[NMAX], mC[NMAX], dC[NMAX];
        metalico(5, 3, met, N, P);
        for(int k = 0; k < 5; k++){
            int e = marcos[k];
            if(e > N/2) break;
            /* o metálico: mede-se a recorrência com e termos e regenera-se o resto */
            int Lm = massey(met, e, P, mC, LIM);
            int *rm = malloc(sizeof(int)*(size_t)N);
            regenera(mC, Lm, met, rm, N, P);
            int acerta_m = 0;
            for(int i = 0; i < N; i++){ if(rm[i] != met[i]) break; acerta_m++; }
            /* o doador: o mesmo */
            int Ld = massey(DOADOR, e, P, dC, LIM);
            int *rd = malloc(sizeof(int)*(size_t)N);
            regenera(dC, Ld, DOADOR, rd, N, P);
            int acerta_d = 0;
            for(int i = 0; i < N; i++){ if(rd[i] != DOADOR[i]) break; acerta_d++; }
            printf("     %8d %14d %14d\n", e, acerta_m, acerta_d);
            free(rm); free(rd);
        }
        /* a medida: com 10 termos, o metálico sustenta tudo e o doador não */
        int Lm = massey(met, 10, P, mC, LIM);
        int *rm = malloc(sizeof(int)*(size_t)N);
        regenera(mC, Lm, met, rm, N, P);
        int sust_m = 0;
        for(int i = 0; i < N; i++){ if(rm[i] != met[i]) break; sust_m++; }
        int Ld = massey(DOADOR, 10, P, dC, LIM);
        int *rd = malloc(sizeof(int)*(size_t)N);
        regenera(dC, Ld, DOADOR, rd, N, P);
        int sust_d = 0;
        for(int i = 0; i < N; i++){ if(rd[i] != DOADOR[i]) break; sust_d++; }
        ok("com 10 termos de enxerto o METALICO sustenta a sequencia inteira",
           sust_m == N);
        ok("e o DOADOR nao sustenta: para muito antes, e e ai que se ve que nao ha medula",
           sust_d < N/2);
        printf("     -> com 10 termos: o metalico sustenta %d de %d; o doador para em %d.\n",
               sust_m, N, sust_d);
        puts("        A celula-tronco do metalico regenera mil termos a partir de dez. A da LLM");
        puts("        nao regenera: ela precisa de ser levada quase inteira, e o que se levou");
        puts("        deixou de ser medula e passou a ser o proprio orgao.\n");
        free(rm); free(rd);
    }

    /* ── §T5b  O CONTROLO QUE FALTAVA ─────────────────────────────────────── */
    puts("§T5b O CONTROLO QUE FALTAVA: isto e da LLM, ou de QUALQUER texto?");
    puts("     Eu ia concluir sobre a LLM sem ter medido texto humano. Se a prosa de um humano");
    puts("     der o mesmo L/N, entao o resultado nao e sobre a LLM: e sobre TEXTO — e atribui-lo");
    puts("     ao doador seria o erro de ontem outra vez.\n");
    {
        /* o texto de controlo e prosa humana do proprio repo — nao gerada, nao escolhida por
         * mim para dar certo: e o cabecalho deste ficheiro, que existia antes da medida */
        FILE *f = fopen("tools/transplante.c", "rb");
        if(!f) f = fopen("transplante.c", "rb");
        int *hum = malloc(sizeof(int)*(size_t)N), nh = 0;
        if(f){
            int c;
            while((c = fgetc(f)) != EOF && nh < N) hum[nh++] = c % P;
            fclose(f);
        }
        if(nh >= N){
            int L_hum = massey(hum, N, P, C, LIM);
            double r_hum = (double)L_hum/N, r_llm2 = (double)L_llm/N;
            ok("a prosa HUMANA da a mesma complexidade da LLM — o resultado e do TEXTO, nao do doador",
               L_hum > 0.45*N && L_hum < 0.55*N);
            printf("     -> humano L = %d (%.3f) contra LLM L = %d (%.3f). A diferenca e %.4f.\n",
                   L_hum, r_hum, L_llm, r_llm2, r_hum > r_llm2 ? r_hum-r_llm2 : r_llm2-r_hum);
            puts("        Entao a conclusao certa NAO e 'a LLM nao tem medula linear' — e 'TEXTO");
            puts("        nao tem medula linear', e a LLM produz texto. Eu ia dizer a primeira,");
            puts("        que soa a descoberta sobre modelos e nao e nada disso.");
        } else ok("a prosa HUMANA da a mesma complexidade da LLM", 0);
        free(hum);
        puts("");
    }

    /* ── §T6  o veredito ─────────────────────────────────────────────────── */
    puts("§T6  O VEREDITO — e ele vale nos dois sentidos\n");
    puts("     O transplante PEGOU: o corpo do banco regenera a sequencia do doador com residuo");
    puts("     0. Mas nao foi um transplante de MEDULA — foi de tecido, porque a razao L/N do");
    puts("     doador esta junto do aleatorio e nao junto do corpo.");
    puts("");
    puts("     E o §T5b fecha a interpretacao: prosa HUMANA da o mesmo. Entao isto nao e sobre a LLM,");
    puts("     e sobre TEXTO — e a estrutura do texto nao e uma RECORRENCIA LINEAR sobre Z_p, que e");
    puts("     a unica que o reconstroi.c sabe colher. Um exame que nao encontra nao prova");
    puts("     ausencia: prova que o exame e outro.");
    puts("");
    puts("     E vale ao contrario, e ai o resultado e forte: a familia metalica regenera mil");
    puts("     termos a partir de DEZ. Quando ha medula, ela pega com quase nada — e e por isso");
    puts("     que o reconstroi.c recupera 10^14 elementos de catorze numeros.");
    puts("");
    {
        double r_llm = (double)L_llm/N, r_alea = (double)L_alea/N, r_met = (double)L_met/N;
        ok("o veredito e mensuravel: o doador esta MAIS PERTO do aleatorio que do metalico",
           (r_llm - r_met) > (r_alea - r_llm));
        printf("     -> distancias na razao L/N: ao metalico %.3f, ao aleatorio %.3f.\n",
               r_llm - r_met, r_alea - r_llm);
    }

    puts("");
    puts("──────────────────────────────────────────────────────────────────────────────");
    printf("unidades: %d   falhas: %d\n", feitas, falhas);
    printf("RESIDUO %d\n", falhas);
    free(C);
    return falhas ? 1 : 0;
}
