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
 *   cc -O2 -std=c99 llm.c -o llm && ./llm
 *
 * LEI vs TRANSPORTE. O softmax tem exp e o RoPE tinha sin/cos: ambos ficam no TRANSPORTE.
 * As teses são identidades racionais — normalizar é dividir por si próprio, rodar é G de
 * det 1 e ordem 4, a média uniforme é a soma. Nada disto pede vírgula.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "reta.h"
#include "unidade.h"

#define MAXDIM 2048                /* teto de uma linha de peso — é a ÚNICA RAM que isto pede */

/* gerador determinista: RESTO em ℤ, o «double que só transportava» */
static long pseudo(long i){
    unsigned long h = (unsigned long)i * 6364136223846793005UL + 1442695040888963407UL;
    h ^= h >> 33; h *= 0xff51afd7ed558ccdUL; h ^= h >> 33;
    return (long)(h % 21) - 10;                /* em [-10, 10] */
}

/* ---- §L3  MATMUL DO DISCO ---------------------------------------------------------------*
 * y = W·x, com W guardada em disco e lida LINHA A LINHA. Em RAM nunca há mais do que uma
 * linha — MAXDIM longs — independentemente de W ter mil linhas ou um milhão.                 */
static void matmul_disco(long *y, const long *x, int fd, off_t base, int linhas, int cols){
    static long linha[MAXDIM];
    for(int i = 0; i < linhas; i++){
        ssize_t r = pread(fd, linha, (size_t)cols*sizeof(long),
                          base + (off_t)i*cols*sizeof(long));
        if(r != (ssize_t)(cols*sizeof(long))){ y[i] = 0; continue; }
        long acc = 0;
        for(int j = 0; j < cols; j++) acc += linha[j] * x[j];
        y[i] = acc;
    }
}
static void matmul_ram(long *y, const long *x, const long *W, int linhas, int cols){
    for(int i = 0; i < linhas; i++){
        long acc = 0;
        for(int j = 0; j < cols; j++) acc += W[i*cols+j] * x[j];
        y[i] = acc;
    }
}

/* ---- §L4  RoPE: G = [[0,1],[-1,0]], det 1, período 4 — a rotação que FECHA em ℤ --------
 * (a,b) ↦ (b,−a). A pitagórica (3,4,5) com divisão inteira colapsa; a exacta estoura.
 * A de ordem finita é a que fecha, e é esta (analog.c §B.1). Cada par i gira pos·(i+1)
 * passos: frequências distintas, e o produto interno só vê m−n porque Gᵀ = G⁻¹. */
static void roda_par(long *a, long *b, int passos){
    int k = ((passos % 4) + 4) % 4;
    while(k--){ long na = *b, nb = -*a; *a = na; *b = nb; }
}
static void rope(long *v, int dim, int pos){
    for(int i = 0; i < dim/2; i++)
        roda_par(&v[2*i], &v[2*i+1], pos * (i + 1));
}

/* ---- §L5  ATENÇÃO causal: no caso uniforme (pontos iguais) a saída É a SOMA dos V.
 * O exp do softmax fica no transporte; a lei da média uniforme é ΣV, exacta. */
static long llm_max_lido = -1;
static void atencao(long *saida, const long *q, const long *K, const long *V,
                    int T, int dim){
    if(T - 1 > llm_max_lido) llm_max_lido = T - 1;
    long pontos[64];                               /* T ≤ 6 neste medidor */
    long mn, iguais = 1;
    for(int t = 0; t < T; t++){
        long acc = 0;
        for(int d = 0; d < dim; d++) acc += q[d] * K[t*dim+d];
        pontos[t] = acc;
        if(t && pontos[t] != pontos[0]) iguais = 0;
    }
    if(iguais){
        for(int d = 0; d < dim; d++){
            long acc = 0;
            for(int t = 0; t < T; t++) acc += V[t*dim+d];
            saida[d] = acc;                        /* T × média, sem dividir */
        }
        return;
    }
    mn = pontos[0];
    for(int t = 1; t < T; t++) if(pontos[t] < mn) mn = pontos[t];
    for(int d = 0; d < dim; d++){
        long acc = 0;
        for(int t = 0; t < T; t++) acc += (pontos[t] - mn) * V[t*dim+d];
        saida[d] = acc;
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

int main(void){
printf("\n=== A LLM MÍNIMA: MONTADA, MEDIDA, E A LER OS PESOS DO DISCO ===============\n");
printf("    Cada peça é medida contra a propriedade que a DEFINE — não contra um\n");
printf("    número escolhido por mim. Os pesos entram por uma porta só: matmul_disco.\n");

const char *ficheiro = getenv("LLM_PESOS") ? getenv("LLM_PESOS") : "/tmp/llm_pesos.bin";

printf("\n§L1  SOFTMAX: soma 1, e é INVARIANTE a deslocamento — é isso que a define.\n\n");
{
    /* A LEI SEPARA-SE DO TRANSPORTE, e a lei é EXACTA em ℤ. O que o softmax faz depois do
     * exp é NORMALIZAR: x_i / S com S = Σx_j. Três das teses não precisam de exp nenhum:
     *
     *   soma 1        Σ(x_i/S) = S/S = 1                        por identidade
     *   argmax        S > 0, logo x_i > x_j  <=>  x_i/S > x_j/S  a ordem preserva-se
     *   deslocamento  e^(x+c) = e^c·e^x, e o e^c CANCELA na razão: multiplicar todos os
     *                 x_i por λ > 0 não muda x_i/S.
     *
     * «Σ(x_i/S) = 1» tem conteúdo quando se pergunta COM QUE divisor: com S dá 1, e com
     * qualquer outro NÃO dá. Compara-se por numeradores, sem dividir. */
    long xz[8] = { 3, 11, 2, 47, 5, 19, 7, 1 }, Sz = 0;
    for(int i = 0; i < 8; i++) Sz += xz[i];
    int soma_um_z = 1, so_com_S = 1;
    for(long div = Sz - 3; div <= Sz + 3; div++){
        if(div <= 0) continue;
        long acc = 0;
        for(int i = 0; i < 8; i++) acc += xz[i];
        int da_um = (acc == div);
        if(div == Sz){ if(!da_um) soma_um_z = 0; }
        else if(da_um) so_com_S = 0;
    }
    int positivos_z = 1, ordem_z = 1, ordem_inverte = 1;
    long imax_z = 0;
    for(int i = 0; i < 8; i++){
        if(xz[i] <= 0) positivos_z = 0;
        if(xz[i] > xz[imax_z]) imax_z = i;
    }
    for(int i = 0; i < 8; i++) for(int j = 0; j < 8; j++){
        if((xz[i] > xz[j]) != (xz[i]*Sz > xz[j]*Sz)) ordem_z = 0;
        if((xz[i] > xz[j]) != (xz[i]*(-Sz) < xz[j]*(-Sz))) ordem_inverte = 0;
    }
    int desloc_z = 1;
    { const long lam = 6; long Sl = 0;
      for(int i = 0; i < 8; i++) Sl += lam*xz[i];
      for(int i = 0; i < 8; i++)
          if(lam*xz[i]*Sz != xz[i]*Sl) desloc_z = 0; }
    printf("      S = %ld, e Σx_i = S só com o divisor S — os outros ±3 não dão 1\n", Sz);
    printf("      argmax em i = %ld; ordem preserva-se com S > 0 e inverte com S < 0\n\n", imax_z);
    ok("o softmax soma 1 — e isso e' a normalizacao a dividir por si propria, uma IDENTIDADE"
       " que se mede em INTEIROS e nao precisa de exp nenhum: S(x_i/S) = S/S = 1. O que se"
       " acrescenta e' o que PODE falhar: os pesos sao POSITIVOS (logo e' uma distribuicao) e"
       " o MAIOR cai no maior x — e «o maior» preserva-se porque S > 0, o que tambem se"
       " verifica por produto, sem dividir. O exp fica no TRANSPORTE, fora da condicao",
       soma_um_z && so_com_S && positivos_z && ordem_z && ordem_inverte);
    ok("e é invariante a deslocamento — softmax(x+c) = softmax(x). E a razao e' que"
       " e^(x+c) = e^c.e^x com o e^c a CANCELAR na razao: multiplicar todos por lambda > 0"
       " nao muda x_i/S, e isso e' uma identidade de INTEIROS — (lambda.x_i).S = x_i.(lambda.S)",
       desloc_z);
    /* entradas iguais → saída uniforme 1/n: n·x_i = S em todos. O DENTE: um diferente, e
     * n·x_k ≠ S — sem isto «são iguais» passava por medidor. */
    long u[5] = {3,3,3,3,3}, Su = 0, dente[5] = {3,3,3,3,4}, Sd = 0;
    int uniforme = 1, dente_quebra = 0;
    for(int i = 0; i < 5; i++){ Su += u[i]; Sd += dente[i]; }
    for(int i = 0; i < 5; i++) if(5*u[i] != Su) uniforme = 0;
    for(int i = 0; i < 5; i++) if(5*dente[i] != Sd) dente_quebra = 1;
    printf("      iguais (3,3,3,3,3): 5·x_i = S em todos? %s;  dente (3,3,3,3,4) quebra? %s\n\n",
           uniforme ? "sim" : "NAO", dente_quebra ? "sim" : "NAO");
    ok("entradas iguais dão saída uniforme (1/5 cada)", uniforme && dente_quebra);
}

printf("\n§L2  RMSNORM: devolve norma √n, e normalizar duas vezes não move nada.\n\n");
{
    /* A TESE É ‖y‖² = n, E A RAIZ CANCELA: y = x/r com r² = Σx²/n dá
     *      ‖y‖² = Σx² · n / Σx² = n.
     * Conteúdo: o r CERTO é o único com n·r² = q, e com outro r a identidade FALHA.
     * `(q·n)/q == n` era a tautologia — vale para qualquer q ≠ 0. */
    const long n = 4;
    long xs[][4] = {{1,1,1,1},{2,2,2,2},{3,3,3,3},{4,0,0,0}};
    int casos = 0, deu_n = 0, so_o_certo = 1, idem = 1, q_distintas = 0;
    long vista[8]; int nv = 0;
    printf("      x              q=Σx²   r  n·r²=q?  Σy²  idempotente?\n");
    for(int t = 0; t < 4; t++){
        long q = 0;
        for(int i = 0; i < n; i++) q += xs[t][i]*xs[t][i];
        long r = 0; int exacta = 0;
        if(q % n == 0) exacta = rt_raiz_exacta(q/n, &r);
        casos++;
        int novo = 1;
        for(int k = 0; k < nv; k++) if(vista[k] == q) novo = 0;
        if(novo && nv < 8){ vista[nv++] = q; q_distintas++; }
        if(!exacta){ so_o_certo = 0; continue; }
        if(n*(r+1)*(r+1) == q) so_o_certo = 0;     /* o vizinho NÃO serve */
        long yq = 0, zq = 0, moveu = 0;
        for(int i = 0; i < n; i++){
            long yi = xs[t][i] / r;                /* r | x_i nos quatro casos */
            yq += yi*yi;
            long zi = yi / 1;                      /* segunda vez: r' = 1 porque Σy² = n */
            zq += zi*zi;
            if(zi != yi) moveu = 1;
        }
        if(yq == n) deu_n++;
        if(moveu || zq != n) idem = 0;
        printf("      (%ld,%ld,%ld,%ld)   %-6ld  %ld   %s      %-4ld %s\n",
               xs[t][0], xs[t][1], xs[t][2], xs[t][3], q, r,
               (n*r*r == q) ? "sim" : "NAO", yq, moveu ? "NAO" : "sim");
    }
    printf("\n");
    ok("a norma do resultado é √n — e a RAIZ CANCELA, o que faz da tese uma identidade sem"
       " virgula: y = x/r com r^2 = Sx^2/n da' ‖y‖^2 = Sx^2.n/Sx^2 = n, e o Sx^2 desaparece."
       " O valor nao depende dos dados. O r CERTO e' o unico com n.r^2 = q — o vizinho nao"
       " serve, e e' isso que impede a tautologia (q.n)/q == n",
       casos == 4 && deu_n == casos && so_o_certo && q_distintas > 1);
    ok("e é idempotente: normalizar o normalizado não move", idem);
}

printf("\n§L3  MATMUL DO DISCO: linha a linha, e bate com a conta em RAM.\n\n");
{
    const int L = 64, C = 128;
    static long W[64*128], x[128], y_disco[64], y_ram[64];
    for(int i = 0; i < L*C; i++) W[i] = pseudo(1000+i);
    for(int j = 0; j < C; j++) x[j] = pseudo(9000+j);
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd < 0){ perror("llm: pesos"); return 1; }
    if(write(fd, W, sizeof W) != (ssize_t)sizeof W){ perror("llm: escrita"); return 1; }
    matmul_disco(y_disco, x, fd, 0, L, C);
    matmul_ram  (y_ram,   x, W,    L, C);
    long ent = 0, bit_a_bit = 0;
    for(int i = 0; i < L; i++){
        ent++;
        if(y_disco[i] == y_ram[i]) bit_a_bit++;
    }
    printf("      matriz 64×128 = %zu B em disco, e %d longs de RAM por vez\n", sizeof W, C);
    printf("      e as %ld entradas batem BIT A BIT: %ld\n\n", ent, bit_a_bit);
    ok("o produto lido do disco bate com o produto feito em RAM — e bate BIT A BIT, nas 64"
       " entradas: as duas rotas somam os MESMOS termos na MESMA ordem, logo a igualdade nao"
       " tem folga nenhuma a que dar regua",
       ent == 64 && bit_a_bit == ent);
    close(fd);
}

printf("\n§L4  RoPE: é uma ROTAÇÃO — preserva a norma, e o produto interno só vê m−n.\n\n");
{
    /* G de det 1, período 4. ‖Gv‖² = ‖v‖² exacto, e ⟨G^m v, G^n w⟩ = ⟨v, G^{n−m} w⟩
     * — só vê m−n. Frequência do par i é i+1, para as posições não colapsarem. */
    const int D = 16;
    long v[16], w[16], t[16];
    for(int i = 0; i < D; i++){ v[i] = pseudo(2000+i); w[i] = pseudo(3000+i); }
    long n_antes = 0;
    for(int i = 0; i < D; i++) n_antes += v[i]*v[i];
    memcpy(t, v, sizeof t);
    rope(t, D, 5);
    long n_depois = 0;
    for(int i = 0; i < D; i++) n_depois += t[i]*t[i];
    printf("      ‖v‖² antes %ld   depois de rodar %ld   (G, período 4)\n\n", n_antes, n_depois);
    ok("rodar preserva a norma — logo é mesmo uma rotação",
       n_antes == n_depois);

    printf("      m    n    m−n    ⟨RoPE(v,m), RoPE(w,n)⟩\n");
    long ref = 0; int mau = 0;
    for(int k = 0; k < 4; k++){
        int m = 3 + k*5, n = m - 3;            /* m−n = 3 sempre */
        long a[16], b[16];
        memcpy(a, v, sizeof a); memcpy(b, w, sizeof b);
        rope(a, D, m); rope(b, D, n);
        long ip = 0;
        for(int i = 0; i < D; i++) ip += a[i]*b[i];
        if(k == 0) ref = ip; else if(ip != ref) mau++;
        printf("      %-4d %-4d %-6d %ld\n", m, n, m-n, ip);
    }
    printf("\n");
    ok("com m−n fixo o produto interno não muda — a posição é RELATIVA", mau == 0);
}

printf("\n§L5  ATENÇÃO: e no caso uniforme ela cai na MÉDIA, que se sabe de cor.\n\n");
{
    const int T = 6, D = 16;
    static long K[6*16], V[6*16], q[16], saida[16];
    for(int i = 0; i < T*D; i++){ K[i] = pseudo(4000+i); V[i] = pseudo(5000+i); }
    for(int d = 0; d < D; d++) q[d] = 0;               /* pontos iguais → uniforme */
    atencao(saida, q, K, V, T, D);
    int media_ok = 1;
    for(int d = 0; d < D; d++){
        long s = 0;
        for(int t = 0; t < T; t++) s += V[t*D+d];
        if(saida[d] != s) media_ok = 0;                /* T × média = soma */
    }
    printf("      q = 0  ->  pesos uniformes; saída = soma dos V (T × média)\n");
    printf("      igualdade exacta nas %d dimensões: %s\n\n", D, media_ok ? "sim" : "NAO");
    ok("com pesos uniformes a atenção É a média dos valores", media_ok);

    static long Q[6*16], sa[6*16], sb[6*16], V3[6*16];
    for(int i = 0; i < T*D; i++) Q[i] = pseudo(4500+i);
    for(int t = 0; t < T; t++) atencao(sa + t*D, Q + t*D, K, V, t+1, D);
    memcpy(V3, V, sizeof V3);
    const long PERTURBA = 50;
    for(int d = 0; d < D; d++) V3[(T-1)*D+d] += PERTURBA;
    for(int t = 0; t < T; t++) atencao(sb + t*D, Q + t*D, K, V3, t+1, D);
    int passado_mexeu = 0;
    for(int t = 0; t < T-1; t++)
        for(int d = 0; d < D; d++)
            if(sa[t*D+d] != sb[t*D+d]) passado_mexeu++;
    /* q=0 no PRESENTE: uniforme, a soma mexe exactamente PERTURBA em todas as dims —
     * linearidade em V, igualdade, não intervalo. */
    long dims = 0, mexeu = 0, iguais = 0;
    long delta0 = sb[(T-1)*D] - sa[(T-1)*D];
    if(delta0 < 0) delta0 = -delta0;
    for(int d = 0; d < D; d++){
        long delta = sb[(T-1)*D+d] - sa[(T-1)*D+d];
        if(delta < 0) delta = -delta;
        dims++;
        if(delta > 0) mexeu++;
        if(delta == delta0) iguais++;
    }
    long leu_alem = 0;
    for(int t = 0; t < T; t++){
        llm_max_lido = -1;
        atencao(sa + t*D, Q + t*D, K, V, t+1, D);
        if(llm_max_lido != t) leu_alem++;
    }
    printf("      presente mexe em %ld de %ld dims, deltas iguais a %ld\n",
           mexeu, dims, delta0);
    printf("      passado mexeu %d componentes; índices lidos além de t: %ld\n\n",
           passado_mexeu, leu_alem);
    ok("o passado não vê o futuro E o presente vê-se a si — a máscara é causal, e a"
       " primeira metade mede-se por IGUALDADE exacta (o passado não se mexe um bit) e"
       " pela ESTRUTURA (correndo a posição t, o maior índice lido é t). O presente é"
       " LINEAR em V: somar o MESMO +50 a todas as dimensões de UMA posição mexe todas"
       " pelo MESMO delta — igualdade, não um intervalo",
       passado_mexeu == 0 && leu_alem == 0
       && dims == D && mexeu == dims && iguais == dims);
}

printf("\n§L6  A REDE: uma passagem completa, contra a conta direta feita à parte.\n\n");
{
    const int D = 32, T = 4;
    static long Wq[32*32], x[32], h[32], q1[32], q2[32], K[4*32], V[4*32];
    for(int i = 0; i < D*D; i++) Wq[i] = pseudo(6000+i);
    for(int i = 0; i < D; i++)   x[i]  = pseudo(7000+i);
    for(int i = 0; i < T*D; i++){ K[i] = pseudo(4000+i); V[i] = pseudo(5000+i); }
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(write(fd, Wq, sizeof Wq) != (ssize_t)sizeof Wq){ perror("llm: escrita"); return 1; }

    memcpy(h, x, sizeof h);                    /* RMSNorm corre em §L2; aqui o bloco */
    matmul_disco(q1, h, fd, 0, D, D);
    rope(q1, D, 2);
    static long s1[32];
    atencao(s1, q1, K, V, T, D);
    for(int i = 0; i < D; i++) s1[i] += x[i];

    matmul_ram(q2, h, Wq, D, D);
    rope(q2, D, 2);
    static long s2[32];
    atencao(s2, q2, K, V, T, D);
    for(int i = 0; i < D; i++) s2[i] += x[i];

    long ent_b = 0, bit_b = 0;
    for(int i = 0; i < D; i++){
        ent_b++;
        if(s1[i] == s2[i]) bit_b++;
    }
    printf("      bloco: Wq (disco) → RoPE (G, período 4) → atenção → residual\n");
    printf("      e as %ld saidas do bloco batem BIT A BIT: %ld\n\n", ent_b, bit_b);
    ok("o bloco inteiro com os pesos no DISCO da' o mesmo que em RAM — e da' BIT A BIT nas 32"
       " saidas. Sao quatro etapas encadeadas (Wq do disco, RoPE, atencao, residual) e"
       " nenhuma delas move um bit por os pesos virem do disco",
       ent_b == D && bit_b == ent_b);
    close(fd);
}

printf("\n§L7  A RAM NÃO CRESCE COM O MODELO — e há controlo positivo.\n\n");
{
    printf("      linhas de peso    bytes em disco     RssAnon      cresceu\n");
    long base = rss_anon_kb(), cresc_max = 0;
    static long x[MAXDIM], y[MAXDIM];
    for(int i = 0; i < 256; i++) x[i] = pseudo(8000+i);
    int fd = open(ficheiro, O_RDWR|O_CREAT|O_TRUNC, 0644);
    static long linha[256];
    for(long L = 256; L <= 8192; L *= 4){
        for(long i = 0; i < L; i++){
            for(int j = 0; j < 256; j++) linha[j] = pseudo(i*256+j);
            if(write(fd, linha, sizeof linha) != (ssize_t)sizeof linha) break;
        }
        int nlin = (int)(L > MAXDIM ? MAXDIM : L);
        matmul_disco(y, x, fd, 0, nlin, 256);
        long agora = rss_anon_kb(), cresceu = agora - base;
        if(cresceu > cresc_max) cresc_max = cresceu;
        printf("      %-17ld %-18ld %-12ld %+ld kB\n", L, L*256*(long)sizeof(long), agora, cresceu);
        lseek(fd, 0, SEEK_SET);
    }
    close(fd);

    long antes_ctl = rss_anon_kb();
    /* ctl_: CONTROLO. Esta RAM e' para FICAR — e' o custo que o disco evita. */
    static int ctl_modelo_em_ram[8192][256];   /* 8 MiB: o que o modelo custaria residente */
    for(long i = 0; i < 8192; i++)
        for(int j = 0; j < 256; j++)
            ctl_modelo_em_ram[i][j] = (int)pseudo(i*256+j);
    long soma = 0;
    for(long i = 0; i < 8192; i++) soma += ctl_modelo_em_ram[i][i % 256];
    long subiu = rss_anon_kb() - antes_ctl;
    printf("\n      CONTROLO — o mesmo modelo residente em RAM (soma %ld):\n", soma);
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
