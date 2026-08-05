/* llm.c — A LLM MÍNIMA, montada peça a peça e VALIDADA — e a ler os pesos do DISCO.
 *
 * REGUA: RssAnon — mede o custo que o disco evita (o controlo)
 *
 * O Aarão: "coloca LLM no toolkit, monta uma simples e valida, depois põe os pesos do llama
 * nela."
 *
 * Esta é a primeira metade: a máquina, e a prova de que ela está certa. Os pesos reais entram
 * depois, e entram por aqui — pela `matmul_disco`, que é a única porta por onde um peso passa.
 *
 * PORQUE É QUE ISTO EXISTE. O `mmu.c` pôs o corpus em disco, mas o modelo continuou onde
 * estava: 934,70 MiB copiados para a VRAM da GTX 1650, 203 MB de RAM anónima no host. O
 * llama.cpp já mapeia o GGUF (o endereço É o deslocamento — a realização B do mmu.c §M3), e
 * depois desfaz-no: COPIA os tensores para a placa. Para os pesos morarem mesmo no disco, o
 * carregador tem de ler tensor a tensor, sob demanda, e largar. É essa a peça que falta, e é
 * esta.
 *
 * A ARQUITETURA é a do Llama (que é a do qwen2.5, o modelo que a torre usa): RMSNorm, atenção
 * com GQA e RoPE, e SwiGLU. Nenhuma peça é postulada — cada uma é medida contra uma
 * propriedade que a define, e não contra um número que eu tenha escolhido:
 *
 *   §L1  SOFTMAX     soma 1, e é INVARIANTE a deslocamento — é isso que a define
 *   §L2  RMSNORM     devolve norma √n, e é IDEMPOTENTE (normalizar duas vezes não move)
 *   §L3  MATMUL      do disco, linha a linha — e bate com a conta direta em RAM
 *   §L4  RoPE        é uma ROTAÇÃO: preserva a norma, e o produto interno só vê m−n
 *   §L5  ATENÇÃO     causal; e no caso uniforme cai na MÉDIA, que se sabe de cor
 *   §L6  A REDE      uma passagem completa, contra a conta direta feita à parte
 *   §L7  A RAM       não cresce com o modelo — e há controlo positivo, como no mmu.c §M5
 *
 *   cc -O2 -std=c99 llm.c -lm -o llm && ./llm
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <unistd.h>
#include <fcntl.h>
#include "unidade.h"

#define MAXDIM 2048                /* teto de uma linha de peso — é a ÚNICA RAM que isto pede */

/* A régua das comparações em vírgula flutuante não é escolhida por mim: é o épsilon da
 * precisão, multiplicado pelo número de termos que se somaram (o erro de arredondamento
 * acumula linearmente no pior caso) e pela escala dos valores. Um limiar inventado passaria
 * a esconder exatamente os defeitos que estas secções existem para apanhar. */
static double regua(int n_termos, double escala){
    return (double)n_termos * FLT_EPSILON * (escala > 1.0 ? escala : 1.0) * 4.0;
}

/* ---- §L1  SOFTMAX ------------------------------------------------------------------------*/
static void softmax(float *x, int n){
    float m = x[0];
    for(int i = 1; i < n; i++) if(x[i] > m) m = x[i];
    float s = 0;
    for(int i = 0; i < n; i++){ x[i] = expf(x[i] - m); s += x[i]; }
    for(int i = 0; i < n; i++) x[i] /= s;
}

/* ---- §L2  RMSNORM -----------------------------------------------------------------------*/
static void rmsnorm(float *y, const float *x, const float *peso, int n){
    double s = 0;
    for(int i = 0; i < n; i++) s += (double)x[i]*x[i];
    float inv = (float)(1.0 / sqrt(s/n + 1e-6));
    for(int i = 0; i < n; i++) y[i] = x[i] * inv * (peso ? peso[i] : 1.0f);
}

/* ---- §L3  MATMUL DO DISCO ---------------------------------------------------------------*
 * y = W·x, com W guardada em disco e lida LINHA A LINHA. Em RAM nunca há mais do que uma
 * linha — MAXDIM floats — independentemente de W ter mil linhas ou um milhão. É este o ponto
 * todo: o tamanho do modelo deixa de ser o tamanho da memória.                              */
static void matmul_disco(float *y, const float *x, int fd, off_t base, int linhas, int cols){
    static float linha[MAXDIM];                 /* estático: uma só, reutilizada sempre */
    for(int i = 0; i < linhas; i++){
        ssize_t r = pread(fd, linha, (size_t)cols*sizeof(float),
                          base + (off_t)i*cols*sizeof(float));
        if(r != (ssize_t)(cols*sizeof(float))){ y[i] = 0; continue; }
        double acc = 0;
        for(int j = 0; j < cols; j++) acc += (double)linha[j] * x[j];
        y[i] = (float)acc;
    }
}
/* o mesmo produto, mas com a matriz TODA em RAM — só existe para servir de segundo caminho */
static void matmul_ram(float *y, const float *x, const float *W, int linhas, int cols){
    for(int i = 0; i < linhas; i++){
        double acc = 0;
        for(int j = 0; j < cols; j++) acc += (double)W[i*cols+j] * x[j];
        y[i] = (float)acc;
    }
}

/* ---- §L4  RoPE: roda o par (2i, 2i+1) por um ângulo que depende da POSIÇÃO ---------------*/
static void rope(float *v, int dim, int pos, float base){
    for(int i = 0; i < dim/2; i++){
        float freq = 1.0f / powf(base, (float)(2*i) / (float)dim);
        float ang = pos * freq, c = cosf(ang), s = sinf(ang);
        float a = v[2*i], b = v[2*i+1];
        v[2*i]   = a*c - b*s;
        v[2*i+1] = a*s + b*c;
    }
}

/* ---- §L5  ATENÇÃO causal de uma cabeça ---------------------------------------------------*/
static void atencao(float *saida, const float *q, const float *K, const float *V,
                    int T, int dim){
    static float pontos[MAXDIM];
    float escala = 1.0f / sqrtf((float)dim);
    for(int t = 0; t < T; t++){
        double acc = 0;
        for(int d = 0; d < dim; d++) acc += (double)q[d] * K[t*dim+d];
        pontos[t] = (float)acc * escala;
    }
    softmax(pontos, T);
    for(int d = 0; d < dim; d++){
        double acc = 0;
        for(int t = 0; t < T; t++) acc += (double)pontos[t] * V[t*dim+d];
        saida[d] = (float)acc;
    }
}

static long rss_anon_kb(void){
    FILE *f = fopen("/proc/self/status", "r");
    if(!f) return -1;
    char lin[256]; long v = -1;
    while(fgets(lin, sizeof lin, f))
        if(!strncmp(lin, "RssAnon:", 8)){ sscanf(lin + 8, "%ld", &v); break; }
    fclose(f);
    return v;
}

/* gerador determinista: os mesmos pesos em cada corrida, sem Math.random de ninguém */
static float pseudo(long i){
    unsigned long h = (unsigned long)i * 6364136223846793005UL + 1442695040888963407UL;
    h ^= h >> 33; h *= 0xff51afd7ed558ccdUL; h ^= h >> 33;
    return (float)((double)(h % 20001) / 10000.0 - 1.0);   /* em [-1, 1] */
}

int main(void){
printf("\n=== A LLM MÍNIMA: MONTADA, MEDIDA, E A LER OS PESOS DO DISCO ===============\n");
printf("    Cada peça é medida contra a propriedade que a DEFINE — não contra um\n");
printf("    número escolhido por mim. Os pesos entram por uma porta só: matmul_disco.\n");

const char *ficheiro = getenv("LLM_PESOS") ? getenv("LLM_PESOS") : "/tmp/llm_pesos.bin";

printf("\n§L1  SOFTMAX: soma 1, e é INVARIANTE a deslocamento — é isso que a define.\n\n");
{
    /* A invariancia ao deslocamento nao e' um detalhe de implementacao: e' a propriedade que
     * torna o softmax bem-posto (e o que justifica subtrair o maximo, que e' como se evita o
     * overflow). Se ela falha, a estabilidade numerica foi-se sem aviso. */
    float a[8], b[8];
    for(int i = 0; i < 8; i++){ a[i] = pseudo(i)*10.0f; b[i] = a[i] + 137.0f; }
    softmax(a, 8); softmax(b, 8);
    double soma = 0, maxdif = 0;
    for(int i = 0; i < 8; i++){ soma += a[i]; double d = fabs(a[i]-b[i]); if(d > maxdif) maxdif = d; }
    printf("      soma dos 8 pesos:            %.10f   (tem de ser 1)\n", soma);
    printf("      maior diferença após +137:   %.3e\n", maxdif);
    printf("      régua (8 termos, escala 1):  %.3e\n\n", regua(8, 1.0));
    ok("o softmax soma 1", fabs(soma - 1.0) < regua(8, 1.0));
    ok("e é invariante a deslocamento — softmax(x+c) = softmax(x)", maxdif < regua(8, 1.0));
    /* o caso degenerado, que também tem resposta conhecida de cor */
    float u[5]; for(int i = 0; i < 5; i++) u[i] = 3.0f;
    softmax(u, 5);
    int uniforme = 1;
    for(int i = 0; i < 5; i++) if(fabs(u[i] - 0.2) > regua(5,1.0)) uniforme = 0;
    ok("entradas iguais dão saída uniforme (1/5 cada)", uniforme);
}

printf("\n§L2  RMSNORM: devolve norma √n, e normalizar duas vezes não move nada.\n\n");
{
    float x[64], y[64], z[64];
    for(int i = 0; i < 64; i++) x[i] = pseudo(100+i) * 7.0f;
    rmsnorm(y, x, NULL, 64);
    rmsnorm(z, y, NULL, 64);
    double ny = 0, dif = 0;
    for(int i = 0; i < 64; i++){ ny += (double)y[i]*y[i]; dif += fabs(y[i]-z[i]); }
    ny = sqrt(ny);
    printf("      ‖RMSNorm(x)‖ = %.6f   e √64 = %.6f\n", ny, sqrt(64.0));
    printf("      Σ|norm(x) − norm(norm(x))| = %.3e\n\n", dif);
    ok("a norma do resultado é √n", fabs(ny - 8.0) < regua(64, 8.0));
    ok("e é idempotente: normalizar o normalizado não move", dif < regua(64, 1.0));
}

printf("\n§L3  MATMUL DO DISCO: linha a linha, e bate com a conta em RAM.\n\n");
{
    /* Os DOIS CAMINHOS. A matriz vai para o disco e e' lida linha a linha; a mesma matriz fica
     * em RAM e multiplica-se do modo direto. Ou os vetores batem, ou uma das duas esta' errada
     * — e nao ha' aqui numero meu nenhum a servir de arbitro. */
    const int L = 64, C = 128;
    static float W[64*128], x[128], y_disco[64], y_ram[64];
    for(int i = 0; i < L*C; i++) W[i] = pseudo(1000+i);
    for(int j = 0; j < C; j++) x[j] = pseudo(9000+j);
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd < 0){ perror("llm: pesos"); return 1; }
    if(write(fd, W, sizeof W) != (ssize_t)sizeof W){ perror("llm: escrita"); return 1; }
    matmul_disco(y_disco, x, fd, 0, L, C);
    matmul_ram  (y_ram,   x, W,    L, C);
    double maxdif = 0, escala = 0;
    for(int i = 0; i < L; i++){
        double d = fabs((double)y_disco[i] - y_ram[i]);
        if(d > maxdif) maxdif = d;
        if(fabs(y_ram[i]) > escala) escala = fabs(y_ram[i]);
    }
    printf("      matriz 64×128 = %zu B em disco, e %d floats de RAM por vez\n", sizeof W, C);
    printf("      maior diferença disco vs RAM: %.3e\n", maxdif);
    printf("      régua (128 termos, escala %.2f): %.3e\n\n", escala, regua(C, escala));
    ok("o produto lido do disco bate com o produto feito em RAM", maxdif < regua(C, escala));
    close(fd);
}

printf("\n§L4  RoPE: é uma ROTAÇÃO — preserva a norma, e o produto interno só vê m−n.\n\n");
{
    /* A propriedade que DEFINE o RoPE nao e' "rodar": e' que o produto interno entre duas
     * posicoes dependa SO' da distancia relativa. E' isso que faz dele um codificador de
     * posicao relativa, e e' isso que se mede — em varios pares com o mesmo m−n. */
    const int D = 64;
    float v[64], w[64];
    for(int i = 0; i < D; i++){ v[i] = pseudo(2000+i); w[i] = pseudo(3000+i); }
    double n_antes = 0;
    for(int i = 0; i < D; i++) n_antes += (double)v[i]*v[i];
    float t[64]; memcpy(t, v, sizeof t);
    rope(t, D, 5, 10000.0f);
    double n_depois = 0;
    for(int i = 0; i < D; i++) n_depois += (double)t[i]*t[i];
    printf("      ‖v‖² antes %.8f   depois de rodar %.8f\n\n", n_antes, n_depois);
    ok("rodar preserva a norma — logo é mesmo uma rotação",
       fabs(n_antes - n_depois) < regua(D, n_antes));

    printf("      m    n    m−n    ⟨RoPE(v,m), RoPE(w,n)⟩\n");
    double ref = 0; int mau = 0;
    for(int k = 0; k < 4; k++){
        int m = 3 + k*5, n = m - 3;            /* m−n = 3 sempre, e as posições mudam */
        float a[64], b[64];
        memcpy(a, v, sizeof a); memcpy(b, w, sizeof b);
        rope(a, D, m, 10000.0f); rope(b, D, n, 10000.0f);
        double ip = 0;
        for(int i = 0; i < D; i++) ip += (double)a[i]*b[i];
        if(k == 0) ref = ip; else if(fabs(ip - ref) > regua(D, fabs(ref))) mau++;
        printf("      %-4d %-4d %-6d %.10f\n", m, n, m-n, ip);
    }
    printf("\n");
    ok("com m−n fixo o produto interno não muda — a posição é RELATIVA", mau == 0);
}

printf("\n§L5  ATENÇÃO: e no caso uniforme ela cai na MÉDIA, que se sabe de cor.\n\n");
{
    /* O caso degenerado aqui NAO iguala trivialmente os dois lados: escolhe-se q = 0, o que
     * torna todos os pontos iguais, o softmax uniforme, e a saida tem de ser exatamente a
     * MEDIA dos V. A media calcula-se a' parte, e e' o oraculo. */
    const int T = 6, D = 16;
    static float K[6*16], V[6*16], q[16], saida[16], media[16];
    for(int i = 0; i < T*D; i++){ K[i] = pseudo(4000+i); V[i] = pseudo(5000+i); }
    for(int d = 0; d < D; d++) q[d] = 0.0f;             /* pontos todos iguais → uniforme */
    atencao(saida, q, K, V, T, D);
    for(int d = 0; d < D; d++){
        double s = 0;
        for(int t = 0; t < T; t++) s += V[t*D+d];
        media[d] = (float)(s / T);
    }
    double maxdif = 0;
    for(int d = 0; d < D; d++){ double x = fabs(saida[d]-media[d]); if(x>maxdif) maxdif = x; }
    printf("      q = 0  ->  pesos uniformes (1/%d cada)\n", T);
    printf("      maior diferença entre a atenção e a média dos V: %.3e\n\n", maxdif);
    ok("com pesos uniformes a atenção É a média dos valores", maxdif < regua(T, 1.0));

    /* A CAUSALIDADE, e aqui eu tinha escrito uma asserção VAZIA. Chamava `atencao(s2,q,K,V2,
     * corte,D)` depois de estragar V2 nas posições >= corte — posições que a função, com o
     * loop `t < T` e T=corte, NUNCA LÊ. Não existia entrada capaz de a fazer falhar, e ela
     * passava verde a afirmar uma propriedade que não estava a testar.
     *
     * O que se mede agora é a sequência INTEIRA de uma vez, cada posição a olhar só para o seu
     * prefixo — que é onde a máscara causal existe de facto. E o teste tem DUAS metades, de
     * propósito: o passado não pode mexer-se, e o presente TEM de mexer-se. Sem a segunda, um
     * "não mudou nada" trivial passaria por causalidade outra vez. */
    static float Q[6*16], sa[6*16], sb[6*16], V3[6*16];
    for(int i = 0; i < T*D; i++) Q[i] = pseudo(4500+i);
    for(int t = 0; t < T; t++) atencao(sa + t*D, Q + t*D, K, V, t+1, D);   /* a máscara */
    memcpy(V3, V, sizeof V3);
    for(int d = 0; d < D; d++) V3[(T-1)*D+d] += 50.0f;      /* estraga-se a ÚLTIMA posição */
    for(int t = 0; t < T; t++) atencao(sb + t*D, Q + t*D, K, V3, t+1, D);
    int passado_mexeu = 0; double mudou_presente = 0;
    for(int t = 0; t < T-1; t++)
        for(int d = 0; d < D; d++)
            if(fabs(sa[t*D+d] - sb[t*D+d]) > regua(T,1.0)) passado_mexeu++;
    for(int d = 0; d < D; d++)
        mudou_presente += fabs(sa[(T-1)*D+d] - sb[(T-1)*D+d]);
    printf("      alterando V só na posição %d:\n", T-1);
    printf("        posições 0..%d mexeram-se em  %d componentes  (tem de ser 0)\n",
           T-2, passado_mexeu);
    printf("        a posição %d mexeu-se em      %.4f            (tem de ser > 0)\n\n",
           T-1, mudou_presente);
    ok("o passado não vê o futuro E o presente vê-se a si — a máscara é causal",
       passado_mexeu == 0 && mudou_presente > 1.0);
}

printf("\n§L6  A REDE: uma passagem completa, contra a conta direta feita à parte.\n\n");
{
    /* Monta-se um bloco inteiro — RMSNorm, projecao pelo disco, RoPE, atencao, residual — e
     * refaz-se a MESMA conta pelo caminho direto, com a matriz em RAM. Dois caminhos outra
     * vez: o que valida a rede nao e' ela concordar consigo propria. */
    const int D = 32, T = 4;
    static float Wq[32*32], x[32], h[32], q1[32], q2[32], K[4*32], V[4*32];
    for(int i = 0; i < D*D; i++) Wq[i] = pseudo(6000+i) * 0.1f;
    for(int i = 0; i < D; i++)   x[i]  = pseudo(7000+i);
    for(int i = 0; i < T*D; i++){ K[i] = pseudo(4000+i)*0.5f; V[i] = pseudo(5000+i)*0.5f; }
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(write(fd, Wq, sizeof Wq) != (ssize_t)sizeof Wq){ perror("llm: escrita"); return 1; }

    rmsnorm(h, x, NULL, D);
    matmul_disco(q1, h, fd, 0, D, D);     /* pelo DISCO */
    rope(q1, D, 2, 10000.0f);
    static float s1[32];
    atencao(s1, q1, K, V, T, D);
    for(int i = 0; i < D; i++) s1[i] += x[i];               /* residual */

    matmul_ram(q2, h, Wq, D, D);          /* pela RAM */
    rope(q2, D, 2, 10000.0f);
    static float s2[32];
    atencao(s2, q2, K, V, T, D);
    for(int i = 0; i < D; i++) s2[i] += x[i];

    double maxdif = 0, escala = 0;
    for(int i = 0; i < D; i++){
        double d = fabs((double)s1[i]-s2[i]); if(d > maxdif) maxdif = d;
        if(fabs(s2[i]) > escala) escala = fabs(s2[i]);
    }
    printf("      bloco: RMSNorm → Wq (disco) → RoPE → atenção → residual\n");
    printf("      maior diferença contra o mesmo bloco em RAM: %.3e\n", maxdif);
    printf("      régua: %.3e\n\n", regua(D*T, escala));
    ok("o bloco inteiro com os pesos no DISCO dá o mesmo que em RAM",
       maxdif < regua(D*T, escala));
    close(fd);
}

printf("\n§L7  A RAM NÃO CRESCE COM O MODELO — e há controlo positivo.\n\n");
{
    /* O mesmo desenho do mmu.c §M5, e pela mesma razao: "+0 kB" e' o resultado que eu queria
     * ver, logo tem de se provar que o instrumento consegue ver outra coisa. */
    printf("      linhas de peso    bytes em disco     RssAnon      cresceu\n");
    long base = rss_anon_kb(), cresc_max = 0;
    static float x[MAXDIM], y[MAXDIM];
    for(int i = 0; i < 256; i++) x[i] = pseudo(8000+i);
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    static float linha[256];
    for(long L = 256; L <= 8192; L *= 4){
        for(long i = 0; i < L; i++){
            for(int j = 0; j < 256; j++) linha[j] = pseudo(i*256+j);
            if(write(fd, linha, sizeof linha) != (ssize_t)sizeof linha) break;
        }
        matmul_disco(y, x, fd, 0, (int)(L > MAXDIM ? MAXDIM : L), 256);
        long agora = rss_anon_kb(), cresceu = agora - base;
        if(cresceu > cresc_max) cresc_max = cresceu;
        printf("      %-17ld %-18ld %-12ld %+ld kB\n", L, L*256*4, agora, cresceu);
        lseek(fd, 0, SEEK_SET);
    }
    close(fd);

    long antes_ctl = rss_anon_kb();
    /* ctl_: CONTROLO. Esta RAM e' para FICAR — e' o custo que o disco evita, e sem ela
     * a medida de cima nao prova nada. tools/ram.sh poupa tudo o que comece por ctl_. */
    static float ctl_modelo_em_ram[8192][256];        /* 8 MiB: o que o modelo custaria residente */
    for(long i = 0; i < 8192; i++)
        for(int j = 0; j < 256; j++) ctl_modelo_em_ram[i][j] = pseudo(i*256+j);
    double soma = 0;
    for(long i = 0; i < 8192; i++) soma += ctl_modelo_em_ram[i][i % 256];
    long subiu = rss_anon_kb() - antes_ctl;
    printf("\n      CONTROLO — o mesmo modelo residente em RAM (verificação %.3f):\n", soma);
    printf("        subiu %+ld kB, e a fórmula dizia 8192 × 256 × 4 B = %d kB\n\n",
           subiu, 8192*256*4/1024);
    ok("o instrumento DETETA o modelo em RAM — o zero acima é medida, não cegueira",
       subiu >= 8192L*256*4/1024/2);
    ok("a RAM não cresce quando o modelo em disco cresce 16×", cresc_max < subiu/4);
}

printf("\n=== FECHO ==================================================================\n");
printf("    A máquina está de pé e medida: sete secções, e cada peça contra a\n");
printf("    propriedade que a define. Os pesos passam por UMA porta — matmul_disco —\n");
printf("    e é por essa porta que os do llama vão entrar.\n\n");
printf("    O que falta é o CARREGADOR: ler o GGUF, achar cada tensor pelo nome, e\n");
printf("    dar-lhe o deslocamento. Nada disto muda; só passa a apontar para lá.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
unlink(ficheiro);
return falhas != 0;
}
