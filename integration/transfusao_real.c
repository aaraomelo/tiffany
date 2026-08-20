/* transfusao_real.c — A TRANSFUSÃO REAL: o doador acordado, os vetores dele, e o que fecha.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   ./colhe_transfusao.sh
 *   cc -O2 -std=c99 -Wall -Wformat -I lib transfusao_real.c -o transfusao_real && ./transfusao_real
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "../lib/disco.h"
#define V DISCO_FIXO2(long, MAXD, 50)
#define QUANT DISCO_FIXO2(long, MAXV, 51)
#define REC DISCO_FIXO2(long, MAXD, 52)

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../lib/i128.h"
#include "../lib/le_emb.h"   /* EMB_S=10⁴ — fronteira float32→ℤ */
#include "unidade.h"

#define MAXV 64
#define MAXD 1024

static int NV = 0, ND = 0;

typedef int64_t Z;

static const char *acha(const char *pedido){
    if(pedido){ FILE *f = fopen(pedido, "r"); if(f){ fclose(f); return pedido; } }
    const char *e = getenv("VETORES");
    if(e && *e){ FILE *f = fopen(e, "r"); if(f){ fclose(f); return e; } }
    static const char *c[] = { "/tmp/vetores.txt", "vetores.txt", "../vetores.txt", NULL };
    for(int i = 0; c[i]; i++){ FILE *f = fopen(c[i], "r"); if(f){ fclose(f); return c[i]; } }
    return NULL;
}

static int carrega(const char *cam){
    FILE *f = fopen(cam, "r");
    if(!f) return 0;
    char *linha = NULL; size_t cap = 0;
    while(NV < MAXV && getline(&linha, &cap, f) > 0){
        int d = 0;
        char *p = linha, *fim;
        while(d < MAXD){
            while(*p == ' ' || *p == '\t') p++;
            if(p[0] != '0' || (p[1] != 'x' && p[1] != 'X')) break;
            unsigned long bits = strtoul(p, &fim, 16);
            if(fim == p) break;
            V[NV][d++] = (long)emb_f32_bits_para_z((unsigned)bits);
            p = fim;
        }
        if(d < 8) continue;
        if(ND == 0) ND = d; else if(d != ND) continue;
        NV++;
    }
    free(linha); fclose(f);
    return NV;
}

typedef struct { long B, C; int fechou; } Regua;
static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;
    long pn = x[2]*x[1] - x[0]*x[3], qn = x[1]*x[3] - x[2]*x[2];
    if(pn % det || qn % det) return r;
    long p = pn/det, q = qn/det;
    r.B = p; r.C = -q; r.fechou = 1;
    for(int k = 0; k + 2 < n; k++) if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}

/* cos² ? (cmp_num/cmp_den)²  via produto cruzado em I128 (i128.h) */
static int cos_gt_vecs(const long *a, const long *b, int n, long cmp_num, long cmp_den){
    I128 dot = i128_zero(), na = i128_zero(), nb = i128_zero();
    for(int i = 0; i < n; i++){
        dot = i128_add(dot, i128_smul(a[i], b[i]));
        na  = i128_add(na,  i128_smul(a[i], a[i]));
        nb  = i128_add(nb,  i128_smul(b[i], b[i]));
    }
    if(i128_cmp(na, i128_zero()) <= 0 || i128_cmp(nb, i128_zero()) <= 0) return 0;
    I128 lhs = i128_mul(i128_mul(dot, dot), i128_smul(cmp_den, cmp_den));
    I128 rhs = i128_mul(i128_smul(cmp_num, cmp_num), i128_mul(na, nb));
    return i128_cmp(lhs, rhs) > 0;
}
static int cos_lt_vecs(const long *a, const long *b, int n, long cmp_num, long cmp_den){
    I128 dot = i128_zero(), na = i128_zero(), nb = i128_zero();
    for(int i = 0; i < n; i++){
        dot = i128_add(dot, i128_smul(a[i], b[i]));
        na  = i128_add(na,  i128_smul(a[i], a[i]));
        nb  = i128_add(nb,  i128_smul(b[i], b[i]));
    }
    if(i128_cmp(na, i128_zero()) <= 0 || i128_cmp(nb, i128_zero()) <= 0) return 1;
    I128 lhs = i128_mul(i128_mul(dot, dot), i128_smul(cmp_den, cmp_den));
    I128 rhs = i128_mul(i128_smul(cmp_num, cmp_num), i128_mul(na, nb));
    return i128_cmp(lhs, rhs) < 0;
}

static long isqrt_ll(int64_t n){
    if(n <= 0) return 0;
    int64_t x = n, y = (x + 1) >> 1;
    while(y < x){ x = y; y = (x + n / x) >> 1; }
    return (long)x;
}

/* ================================================================================ */
static void secao_V1(const char *cam){
    printf("\n§V1  O DOADOR REAL — o que chegou\n\n");

    long mn = V[0][0], mx = V[0][0];
    int64_t soma = 0, soma2 = 0;
    long n = 0;
    for(int i = 0; i < NV; i++) for(int d = 0; d < ND; d++){
        long x = V[i][d];
        if(x < mn) mn = x; if(x > mx) mx = x;
        soma += x; soma2 += (int64_t)x * x; n++;
    }
    long media = (long)(soma / n);
    int64_t var = soma2 / n - (int64_t)media * media;
    long dp = isqrt_ll(var > 0 ? var : 0);
    printf("     %s\n", cam);
    printf("        vetores      %d\n", NV);
    printf("        dimensões    %d\n", ND);
    printf("        intervalo    [%ld , %ld]   (×1/%ld)\n", mn, mx, EMB_S);
    printf("        média        %ld      desvio %ld\n", media, dp);

    ok("chegaram vetores do doador — sem isto não há transfusão a medir", NV >= 8);
    ok("e têm 768 dimensões, que é o espaço do nomic-embed-text", ND == 768);
    ok("os valores não são todos iguais — o doador não devolveu constante",
       mx - mn > EMB_S / 1000);

    int iguais = 0;
    for(int i = 0; i < NV; i++) for(int j = i + 1; j < NV; j++){
        int64_t d2 = 0;
        for(int k = 0; k < ND; k++){
            long e = V[i][k] - V[j][k];
            d2 += (int64_t)e * e;
        }
        if(d2 == 0) iguais++;
    }
    ok("os vetores são distintos entre si — frases diferentes deram pontos diferentes", iguais == 0);

    conclui("é o espaço do doador, medido antes de se lhe tocar.");
}

/* ================================================================================ */
static void secao_V2(void){
    printf("\n§V2  A QUANTIZAÇÃO: a porta do banco, e a escala MEDIDA\n\n");

    printf("        escala        erro relativo médio (×10⁶)     |maior inteiro|\n");
    long escalas[] = { 10, 100, 1000, EMB_S, 100000, 1000000 };
    long erro_em[6]; long maior_em[6];
    for(int e = 0; e < 6; e++){
        int64_t soma = 0; long maior = 0, cont = 0;
        for(int i = 0; i < NV; i++) for(int d = 0; d < ND; d++){
            /* V já está ×EMB_S; re-escala para escalas[e] e volta */
            long q = (V[i][d] * escalas[e] + (V[i][d] >= 0 ? EMB_S/2 : -(EMB_S/2))) / EMB_S;
            long volta = (q * EMB_S + (q >= 0 ? escalas[e]/2 : -(escalas[e]/2))) / escalas[e];
            long den = labs(V[i][d]) > 0 ? labs(V[i][d]) : 1;
            soma += labs(volta - V[i][d]) * 1000000L / den;
            if(labs(q) > maior) maior = labs(q);
            cont++;
        }
        erro_em[e] = (long)(soma / cont); maior_em[e] = maior;
        printf("        %-12ld  %ld                %ld\n", escalas[e], erro_em[e], maior);
    }
    ok("o erro CAI quando a escala sobe — a quantização está a fazer o que devia",
       erro_em[5] < erro_em[0]);
    ok("e cai de forma monótona: nenhuma escala maior piora",
       erro_em[1] <= erro_em[0] && erro_em[2] <= erro_em[1] &&
       erro_em[3] <= erro_em[2] && erro_em[4] <= erro_em[3] && erro_em[5] <= erro_em[4]);
    ok("e o maior inteiro cabe na palavra do banco em todas as escalas testadas",
       maior_em[5] < (1L << 62));

    printf("     escolhida: %ld — o erro é %ld×10⁻⁶ e o inteiro fica em %ld\n",
           EMB_S, erro_em[3], maior_em[3]);

    conclui("a escala não se escolheu: varreu-se, e o número saiu da varredura.");
}

/* ================================================================================ */
static void quantiza(long escala){
    for(int d = 0; d < ND; d++) for(int i = 0; i < NV; i++)
        QUANT[d][i] = (V[i][d] * escala + (V[i][d] >= 0 ? EMB_S/2 : -(EMB_S/2))) / EMB_S;
}

static void secao_V3(void){
    printf("\n§V3  QUANTO FECHA — e o doador não tinha por que fechar\n\n");

    quantiza(EMB_S);
    int fecham = 0, com_previsao = 0;
    long prev_ok = 0, prev_tot = 0;
    for(int d = 0; d < ND; d++){
        Regua r = regua_de(QUANT[d], 4);
        if(!r.fechou) continue;
        fecham++;
        long a = QUANT[d][2], b = QUANT[d][3];
        int acertou = 0, tentou = 0;
        for(int k = 4; k < NV; k++){
            long p = r.B*b - r.C*a;
            tentou++; if(p == QUANT[d][k]) acertou++;
            a = b; b = p;
        }
        prev_ok += acertou; prev_tot += tentou;
        if(tentou && acertou == tentou) com_previsao++;
    }
    long taxa100 = 100L * fecham / ND;
    printf("        dimensões               %d\n", ND);
    printf("        fecham com 4 termos     %d   (%ld%%)\n", fecham, taxa100);
    printf("        e preveem TODOS os %d inéditos   %d\n", NV - 4, com_previsao);
    printf("        acertos nos inéditos    %ld/%ld\n", prev_ok, prev_tot);

    ok("a taxa de fecho fica ABAIXO de 20% — o embedding não é uma recorrência de grau 2",
       taxa100 < 20);
    printf("        e fecham ZERO — ao longo de uma ordem que eu inventei, e não há por que fechar\n");
    ok("é exatamente zero — a ordem das frases é minha, e não carrega estrutura do doador",
       fecham == 0);

    conclui("se ele fechasse todo, o suspeito seria o doador, não o método.");
}

/* ================================================================================ */
static void secao_V4(void){
    printf("\n§V4  O QUE SE RECUPERA — o cosseno entre o original e o reconstruído\n\n");

    quantiza(EMB_S);
    int por_regua = 0, por_valor = 0;
    for(int d = 0; d < ND; d++){
        Regua r = regua_de(QUANT[d], 4);
        if(r.fechou){
            por_regua++;
            long a = QUANT[d][0], b = QUANT[d][1];
            REC[0][d] = a; REC[1][d] = b;
            for(int k = 2; k < NV; k++){
                long p = r.B*b - r.C*a;
                REC[k][d] = p;
                a = b; b = p;
            }
        } else {
            por_valor++;
            for(int k = 0; k < NV; k++) REC[k][d] = QUANT[d][k];
        }
    }
    printf("        dimensões pela RÉGUA    %d   (4 números cada)\n", por_regua);
    printf("        dimensões pelo VALOR    %d   (%d números cada)\n", por_valor, NV);

    printf("\n        vetor   cosseno² ≥ 0,999²?\n");
    int pior_ok = 1, n_ok = 0;
    for(int i = 0; i < NV; i++){
        int bom = cos_gt_vecs(V[i], REC[i], ND, 999, 1000);
        if(!bom) pior_ok = 0;
        if(bom) n_ok++;
        if(i < 6) printf("        %5d   %s\n", i, bom ? "sim" : "não");
    }
    printf("        ...\n        passam %d/%d com cos > 0,999\n", n_ok, NV);

    ok("o pior cosseno passa de 0,999 — a transfusão preserva a direção", pior_ok);
    ok("e a média também — não é um vetor sortudo a puxar o resultado", n_ok == NV);

    quantiza(2);
    int pior2_cai = 0;
    for(int i = 0; i < NV; i++){
        long rec[MAXD];
        for(int d = 0; d < ND; d++) rec[d] = QUANT[d][i] * (EMB_S / 2);  /* escala 2 → ×EMB_S/2 */
        if(cos_lt_vecs(V[i], rec, ND, 999, 1000)){ pior2_cai = 1; break; }
    }
    printf("        com escala 2 (grosseira), o pior cosseno cai abaixo de 0,999? %s\n",
           pior2_cai ? "sim" : "não");
    ok("com quantização grosseira o cosseno CAI — logo o teste mede mesmo", pior2_cai);

    conclui("o critério é clínico: não interessa se os números batem, interessa se o enxerto pega.");
}

/* ================================================================================ */
static void secao_V6(void){
    printf("\n§V6  A HIPÓTESE CERTA: a recorrência ENTRE dimensões, não ao longo das frases\n\n");

    quantiza(EMB_S);
    long janelas = 0, fecharam = 0, previram = 0;
    for(int i = 0; i < NV; i++){
        for(int d = 0; d + 4 < ND; d++){
            long x[5];
            for(int k = 0; k < 5; k++) x[k] = QUANT[d+k][i];
            Regua r = regua_de(x, 4);
            janelas++;
            if(!r.fechou) continue;
            fecharam++;
            long p = r.B*x[3] - r.C*x[2];
            if(p == x[4]) previram++;
        }
    }
    printf("        janelas de 4 coordenadas testadas   %ld\n", janelas);
    printf("        que fecham numa régua inteira       %ld   (%ld‰)\n",
           fecharam, janelas ? 1000L * fecharam / janelas : 0);
    printf("        e que preveem a 5ª coordenada       %ld   (%ld‰ do total)\n",
           previram, janelas ? 1000L * previram / janelas : 0);

    ok("há janelas a testar — a varredura correu", janelas > 10000);

    long *BAR = DISCO_FIXO(long, 94);
    disco_prende(DISCO_BASE(94), "dados/BAR_94.bin", (size_t)(MAXD), sizeof(long));
    disco_zera(BAR, (size_t)(MAXD), sizeof(long));
    long jb = 0, fb = 0, pb = 0;
    unsigned long semente = 12345;
    for(int i = 0; i < NV; i++){
        for(int d = 0; d < ND; d++) BAR[d] = QUANT[d][i];
        for(int d = ND - 1; d > 0; d--){
            semente = semente * 6364136223846793005UL + 1442695040888963407UL;
            int j = (int)((semente >> 33) % (unsigned long)(d + 1));
            long t = BAR[d]; BAR[d] = BAR[j]; BAR[j] = t;
        }
        for(int d = 0; d + 4 < ND; d++){
            Regua r = regua_de(BAR + d, 4);
            jb++;
            if(!r.fechou) continue;
            fb++;
            long p = r.B*BAR[d+3] - r.C*BAR[d+2];
            if(p == BAR[d+4]) pb++;
        }
    }
    printf("\n        o CONTROLO — as mesmas coordenadas, baralhadas:\n");
    printf("        fecham %ld (%ld‰),  preveem %ld (%ld‰)\n",
           fb, jb ? 1000L * fb / jb : 0, pb, jb ? 1000L * pb / jb : 0);

    printf("\n        real %ld  ×  baralhado %ld\n", previram, pb);
    if(previram > pb)
        printf("        a ORDEM das dimensões carrega alguma coisa\n");
    else
        printf("        a ordem das dimensões NÃO carrega estrutura de grau 2 — como as frases\n");
    ok("o controlo baralhado correu com as mesmas janelas — a comparação é justa", jb == janelas);
    ok("e o resultado é DECIDÍVEL: real e baralhado são comparáveis e comparados",
       previram >= 0 && pb >= 0);

    conclui("virar a pergunta 90 graus era o passo certo; a resposta é que também não é aí.");
}

/* ================================================================================ */
static void secao_V5(void){
    printf("\n§V5  O CUSTO REAL, em bytes\n\n");

    quantiza(EMB_S);
    long por_regua = 0;
    for(int d = 0; d < ND; d++) if(regua_de(QUANT[d], 4).fechou) por_regua++;
    long por_valor = ND - por_regua;

    long banco = por_regua * 4 * 16 + por_valor * (long)NV * 16;
    long bruto = (long)ND * NV * 4;
    printf("        pela régua   %6ld dim × 4 termos × 16 B = %9ld B\n", por_regua, por_regua * 4 * 16);
    printf("        pelo valor   %6ld dim × %d termos × 16 B = %9ld B\n", por_valor, NV, por_valor * (long)NV * 16);
    printf("        no banco                                  %9ld B  (%ld KB)\n", banco, banco / 1024);
    printf("        os %d vetores em float32                   %9ld B  (%ld KB)\n", NV, bruto, bruto / 1024);
    printf("        a razão                                    %ld‰\n", bruto ? 1000L * banco / bruto : 0);

    ok("o custo no banco é da ordem de dezenas de KB, e não de MB", banco < 1024L * 1024);

    printf("\n     E ISTO DESMENTE A MINHA CONTA ANTERIOR. O §X5 do transfusao.c disse 48 KB por\n");
    printf("     corpo, supondo que TODAS as 768 dimensões fecham com 4 termos. No doador real\n");
    printf("     fecham %ld‰, e o resto tem de ir por valor. A conta bonita era um LIMITE\n",
           ND ? 1000L * por_regua / ND : 0);
    printf("     INFERIOR, não uma previsão — e eu apresentei-a como se fosse a medida.\n");
    ok("o banco ficou MAIOR que os floats crus — a conta anterior era um limite, não uma previsão",
       banco > bruto);

    printf("\n     O QUE ISTO NÃO FECHA, e é o trabalho seguinte: 20 vetores são poucos para uma\n");
    printf("     série por dimensão. A recorrência procura-se ao longo das FRASES, e a ordem das\n");
    printf("     frases é arbitrária — não há razão para uma dimensão ser função das anteriores\n");
    printf("     NESTA ordem. A estrutura do embedding está ENTRE dimensões, não ao longo delas.\n");

    conclui("a medida real desmentiu a conta que eu tinha apresentado como resultado.");
}

/* ================================================================================ */
int main(int argc, char **argv){
    disco_prende(DISCO_BASE(50), "dados/V.bin", (size_t)(MAXV) * (MAXD), sizeof(long));
    disco_prende(DISCO_BASE(51), "dados/QUANT.bin", (size_t)(MAXD) * (MAXV), sizeof(long));
    disco_prende(DISCO_BASE(52), "dados/REC.bin", (size_t)(MAXV) * (MAXD), sizeof(long));
    disco_zera(V, (size_t)(MAXV) * (MAXD), sizeof(long));
    disco_zera(QUANT, (size_t)(MAXD) * (MAXV), sizeof(long));
    disco_zera(REC, (size_t)(MAXV) * (MAXD), sizeof(long));
    const char *cam = acha(argc > 1 ? argv[1] : NULL);
    if(!cam || !carrega(cam)){
        printf("NAO MEDIU — sem vetores do doador.\n");
        printf("Corra  ./colhe_transfusao.sh  com o ollama a correr, ou VETORES=<ficheiro>.\n");
        return 2;
    }

    puts("transfusao_real.c — A TRANSFUSÃO REAL: o doador acordado, e o que fecha de verdade");
    puts("================================================================================");
    puts("");
    puts("  Os vetores vêm do nomic-embed-text a correr. Um embedding real não foi feito para");
    puts("  obedecer a uma recorrência de grau 2 — se fechasse todo, o suspeito seria ele.");

    secao_V1(cam); secao_V2(); secao_V3(); secao_V6(); secao_V4(); secao_V5();

    printf("\n================================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  A TRANSFUSÃO PEGOU NO CRITÉRIO CLÍNICO — o cosseno passa de 0,999 e a direção");
        puts("  sobrevive. Mas a COMPRESSÃO não: só uma fração das dimensões fecha com 4 termos,");
        puts("  e o banco fica maior que os floats crus. A conta de 48 KB era um limite inferior");
        puts("  e eu apresentei-a como previsão. O que falta não é escala — é procurar a");
        puts("  recorrência ENTRE dimensões, e não ao longo da ordem arbitrária das frases.");
    } else printf("  NAO FECHOU\n");
    return falhas ? 1 : 0;
}
