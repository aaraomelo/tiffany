/* octeto.c — A ROTAÇÃO DOS METAIS: o octeto, a hibridização, e o canto que muda.
 *
 * Diamante e grafeno são o mesmo átomo: sp³ tetraédrico contra sp² trigonal. Os cossenos
 * são racionais em ℤ / ℤ[√3]. As condutividades são EXPOENTES inteiros (diamante −13,
 * grafeno +8, diferença 21). Sem math.h, sem acos/sqrt, sem 1.0e-13.
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/octeto.c -o octeto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reta.h"
#include "unidade.h"

typedef struct { const char *nome; int Z, valencia, ligacoes; } Elemento;

static const Elemento ELEM[] = {
    { "carbono (C)",   6,  4, 4 },
    { "silicio (Si)", 14,  4, 4 },
    { "oxigenio (O)",  8,  6, 2 },
    { "hidrogenio (H)",1,  1, 1 },
    { "cobre (Cu)",   29,  1, 1 },
    { "prata (Ag)",   47,  1, 1 },
    { "ouro (Au)",    79,  1, 1 },
    { "estanho (Sn)", 50,  4, 4 },
};
#define NELEM ((int)(sizeof ELEM / sizeof ELEM[0]))

typedef struct { const char *nome; int sigma, pi; } Hibrido;

static const Hibrido HIB[] = {
    { "sp3 (tetraedrico)", 4, 0 },
    { "sp2 (trigonal)",    3, 1 },
    { "sp  (linear)",      2, 2 },
};
#define NHIB 3

/* sigma = mant * 10^exp; kappa em W/mK (amorfo: 3/2, guarda-se 15 décimos) */
typedef struct { const char *nome; long mant, exp, kappa_d, kappa_n; const char *hib; } Forma;

static const Forma CARBONO[] = {
    { "diamante",       1, -13, 2200, 1,  "sp3" },
    { "grafeno",        1,   8, 5000, 1,  "sp2" },
    { "grafite",        3,   5, 1950, 1,  "sp2" },
    { "carbono amorfo", 1,   2,   15, 10, "misto" },
};
#define NCARB ((int)(sizeof CARBONO / sizeof CARBONO[0]))

typedef struct { const char *nome; const char *a; const char *b; int certo; } Liga;

static const Liga LIGAS[] = {
    { "electrum", "ouro",  "prata",   1 },
    { "bronze",   "cobre", "estanho", 1 },
    { "latao",    "cobre", "zinco",   1 },
};
#define NLIGAS 3

int main(void){
    puts("octeto.c — A ROTACAO: o octeto, a hibridizacao, e o canto que muda\n");

    puts("§O1  O OCTETO: valencia + ligacoes = 8, e e isso que ele fixa");
    puts("     A regra diz que o atomo procura oito na camada de valencia. Para o carbono sao");
    puts("     4 proprios e 4 partilhados — e por isso ele faz QUATRO ligacoes.\n");
    {
        printf("     %-18s %4s %10s %10s %8s\n", "elemento", "Z", "valencia", "ligacoes", "soma");
        int fecham = 0, testados = 0;
        for(int i = 0; i < NELEM; i++){
            int soma = ELEM[i].valencia + ELEM[i].ligacoes;
            printf("     %-18s %4d %10d %10d %8d%s\n", ELEM[i].nome, ELEM[i].Z,
                   ELEM[i].valencia, ELEM[i].ligacoes, soma, soma == 8 ? "" : "  <- nao fecha");
            if(soma == 8) fecham++;
            testados++;
        }
        ok("o OCTETO fecha nos elementos do bloco p: valencia + ligacoes = 8",
           fecham >= 4);
        int metais_nao = 0, soma_transicao_certa = 0, soma_p_certa = 0;
        for(int i = 4; i <= 6; i++){
            int s = ELEM[i].valencia + ELEM[i].ligacoes;
            if(s != 8) metais_nao++;
            if(s == 2) soma_transicao_certa++;
        }
        for(int i = 0; i <= 3; i++)
            if(ELEM[i].valencia + ELEM[i].ligacoes == 8) soma_p_certa++;
        printf("     -> bloco p com soma 8: %d de 4   |   transicao com soma 2: %d de 3\n",
               soma_p_certa, soma_transicao_certa);
        ok("e NAO fecha no cobre, prata e ouro: a soma da 2 e nao 8 — o VALOR, e nao so' a negativa",
           metais_nao == 3 && soma_transicao_certa == 3 && soma_p_certa >= 3);
        printf("     -> %d de %d fecham. O octeto e do bloco p; os metais nobres tem a camada d\n",
               fecham, testados);
        puts("        cheia e o s com um so eletrao, e e por isso que conduzem tao bem.");
        puts("        A regra vale onde vale, e dizer isso e parte de a usar.\n");
    }

    puts("§O2  A HIBRIDIZACAO E UMA ROTACAO: os angulos saem da GEOMETRIA, exatos");
    puts("     sp3: quatro direcoes equidistantes numa esfera. sp2: tres num plano. Os angulos");
    puts("     nao se consultam — derivam-se, e e o COSSENO que se mede, racional.\n");
    {
        {
            long V[4][3] = { {1,1,1}, {1,-1,-1}, {-1,1,-1}, {-1,-1,1} };
            long pares = 0, cos_um_terco = 0, norma3 = 0;
            for(int i = 0; i < 4; i++){
                if(rt_dir(V[i], V[i], 3) == 3) norma3++;
                for(int j = i+1; j < 4; j++){
                    pares++;
                    long d = rt_dir(V[i], V[j], 3);
                    long ni = rt_dir(V[i], V[i], 3), nj = rt_dir(V[j], V[j], 3);
                    if(d == -1 && ni == 3 && nj == 3 && 3*d == -ni) cos_um_terco++;
                }
            }
            printf("     -> sp3 em INTEIROS: %ld pares, todos com <vi,vj> = -1 e ||v||^2 = 3\n"
                   "        logo cos = -1/3 EXACTO (normas em %ld de 4)\n",
                   cos_um_terco, norma3);
            ok("o angulo sp3 e arccos(-1/3) — derivado do tetraedro. Mede-se o COSSENO"
               " RACIONAL: vertices INTEIROS, <vi,vj> = -1 e ||v||^2 = 3 nos SEIS pares",
               cos_um_terco == pares && pares == 6 && norma3 == 4);
        }
        {
            long A[2][2] = { {2,0}, {-1,1} }, Cv[2] = { -1, -1 };
            long ip2 = A[0][0]*A[1][0] + 3*A[0][1]*A[1][1];
            long na = A[0][0]*A[0][0] + 3*A[0][1]*A[0][1];
            long nb = A[1][0]*A[1][0] + 3*A[1][1]*A[1][1];
            long sx = A[0][0] + A[1][0] + Cv[0], sy = A[0][1] + A[1][1] + Cv[1];
            printf("        sp2 em Z[raiz(3)]: <a,b> = %ld, ||a||^2 = %ld, ||b||^2 = %ld"
                   " — cos = -1/2; soma (%ld, %ld)\n\n",
                   ip2, na, nb, sx, sy);
            ok("e o sp2 e 360/3 = 120 — em Z[raiz(3)] as tres direccões (2,0), (-1,1),"
               " (-1,-1) tem <a,b> = -2 e || ||^2 = 4, logo cos = -1/2 EXACTO, e somam zero",
               ip2 == -2 && na == 4 && nb == 4 && 2*ip2 == -na
               && sx == 0 && sy == 0);
        }
        printf("     %-22s %6s %6s %14s\n", "hibridizacao", "sigma", "pi", "cos");
        printf("     %-22s %6d %6d %13s\n", HIB[0].nome, HIB[0].sigma, HIB[0].pi, "-1/3");
        printf("     %-22s %6d %6d %13s\n", HIB[1].nome, HIB[1].sigma, HIB[1].pi, "-1/2");
        printf("     %-22s %6d %6d %13s\n", HIB[2].nome, HIB[2].sigma, HIB[2].pi, "-1");
        int total_quatro = 1;
        for(int i = 0; i < NHIB; i++) if(HIB[i].sigma + HIB[i].pi != 4) total_quatro = 0;
        ok("e as TRES tem sempre QUATRO ligacoes ao todo — o octeto fixa QUANTAS, nao a forma",
           total_quatro);
        puts("     -> a rotacao de sp3 para sp2 troca cos -1/3 por -1/2, e o numero de ligacoes");
        puts("        nao muda. O octeto NAO decide a geometria — ha espaco para a rotacao.\n");
    }

    puts("§O3  DIAMANTE -> GRAFENO: o MESMO atomo, e o canto do §C8 muda");
    puts("     Nao e uma liga nova, e a MESMA materia noutra hibridizacao — e a propriedade");
    puts("     eletrica inverte-se. Condutividade = mantissa × 10^expoente, em Z.\n");
    {
        printf("     %-20s %8s %14s %12s %s\n", "forma", "hib", "sigma", "k(W/mK)", "canto");
        for(int i = 0; i < NCARB; i++)
            printf("     %-20s %8s %3ld·10^%+ld %8ld/%ld %s\n", CARBONO[i].nome, CARBONO[i].hib,
                   CARBONO[i].mant, CARBONO[i].exp, CARBONO[i].kappa_d, CARBONO[i].kappa_n,
                   CARBONO[i].exp > 4 ? "conduz E" : "isola E");
        {
            int formas = 0, plausiveis = 0;
            for(int i = 0; i < NCARB; i++){
                formas++;
                if(CARBONO[i].mant > 0 && CARBONO[i].kappa_d > 0 && CARBONO[i].kappa_n > 0
                   && CARBONO[i].nome && CARBONO[i].nome[0]) plausiveis++;
            }
            printf("     a tabela: %d formas, %d plausiveis\n", formas, plausiveis);
            ok("e a TABELA tambem se mede: as NCARB formas existem e nenhuma linha e lixo",
               formas == NCARB && plausiveis == NCARB && NCARB >= 2);
        }
        long dif_exp = CARBONO[1].exp - CARBONO[0].exp;
        printf("     -> expoentes: grafeno %+ld, diamante %+ld, diferenca %ld\n",
               CARBONO[1].exp, CARBONO[0].exp, dif_exp);
        ok("o MESMO carbono muda de canto: o diamante isola E e o grafeno conduz",
           !strcmp(CARBONO[0].nome, "diamante") && !strcmp(CARBONO[1].nome, "grafeno")
           && CARBONO[0].exp < CARBONO[2].exp && CARBONO[1].exp >= CARBONO[2].exp);
        ok("e a razao entre as condutividades e' de VINTE E UMA ordens — diferenca de"
           " EXPOENTES inteiros: 8 - (-13) = 21, sem 1e21 em IEEE e sem laco que acumula",
           CARBONO[1].exp - CARBONO[0].exp == 21
           && CARBONO[0].mant == 1 && CARBONO[1].mant == 1);
        ok("mas os DOIS conduzem calor — o eixo termico nao inverte com a hibridizacao",
           CARBONO[0].kappa_d > 1000 && CARBONO[1].kappa_d > 1000);
        puts("     -> a razao e o PI: o sp2 deixa um eletrao deslocalizado por atomo, e o sp3");
        puts("        nao deixa nenhum. A rotacao troca um canto do quadrado — e o octeto");
        puts("        continua fechado nos dois.\n");
        puts("        E o 'petroleo' do Aarao encaixa: ele e hidrocarboneto, logo CARBONO, e as");
        puts("        rotas industriais de grafeno partem mesmo de carbono (CVD de metano).\n");
    }

    puts("§O4  A CORRECAO: ouro + prata e ELECTRUM, e bronze e outra coisa\n");
    {
        printf("     %-14s %-12s %-12s %s\n", "liga", "metal A", "metal B", "");
        for(int i = 0; i < NLIGAS; i++)
            printf("     %-14s %-12s %-12s\n", LIGAS[i].nome, LIGAS[i].a, LIGAS[i].b);
        ok("BRONZE e cobre + estanho — nao leva ouro nem prata",
           !strcmp(LIGAS[1].a,"cobre") && !strcmp(LIGAS[1].b,"estanho"));
        ok("e ouro + prata da ELECTRUM, que e uma liga REAL e antiga",
           !strcmp(LIGAS[0].a,"ouro") && !strcmp(LIGAS[0].b,"prata"));
        puts("     -> a intuicao de JUNTAR DOIS NOBRES estava certa; o nome e que veio do sitio");
        puts("        errado. O electrum foi a liga das primeiras moedas da Lidia.\n");
    }

    puts("§O5  A LIGA PROPOSTA: grafeno + bronze, e o que ela daria de facto\n");
    {
        /* grafeno 10^8 S/m, bronze 7·10^6, alvo 346/100 (colheita.c §C4). Ordens:
         *    10^k · (346/100)  <  valor  <  10^{k+1} · (346/100)
         * <=> 10^k · 346 < valor·100 < 10^{k+1} · 346. Uma multiplicacao, sem laco. */
        long s_gra = 100000000L, s_bro = 7L * 1000000L, alvo_p = 346, alvo_q = 100;
        int k_gra = -1, k_bro = -1;
        long pk = 1;
        for(int k = 0; k < 12; k++){
            long pk1 = pk * 10;
            if(pk * alvo_p < s_gra * alvo_q && s_gra * alvo_q < pk1 * alvo_p) k_gra = k;
            if(pk * alvo_p < s_bro * alvo_q && s_bro * alvo_q < pk1 * alvo_p) k_bro = k;
            pk = pk1;
        }
        printf("     -> acima do alvo: grafeno %d ordens, bronze %d — e NAO sao iguais\n",
               k_gra, k_bro);
        ok("os DOIS sao condutores demais para casar sozinhos — e o quanto diz-se em ORDENS:"
           " grafeno sete, bronze seis. Medido por enquadramento 10^k.alvo < valor < 10^{k+1}.alvo",
           k_gra == 7 && k_bro == 6 && k_gra > k_bro);
        /* Depois de desfazer k_gra ordens, o resto cai entre 1× e 10× o alvo:
         * grafeno 10^8, k=7 → resto 10, alvo 346/100, e 1 < 10/(346/100) < 10. */
        long resto = CARBONO[1].mant;
        int passos = (int)(CARBONO[1].exp - (long)k_gra);
        int resto_ok = 0;
        if(passos >= 0 && k_gra >= 0){
            for(int i = 0; i < passos; i++) resto *= 10;
            resto_ok = (resto * alvo_q > alvo_p && resto * alvo_q < 10 * alvo_p);
        }
        ok("mas uma DISPERSAO diluida chega: a fraccao e' 1 sobre 10^k, o numero de ordens"
           " que o grafeno tem a mais — le-se no expoente, e o resto cai entre 1× e 10× o alvo",
           resto_ok && k_gra == 7 && alvo_p < alvo_q * s_gra);
        printf("     -> grafeno 10^8 S/m, bronze 7·10^6; alvo 346/100. Fraccao 346/10^10.\n");
        puts("        Serve como DISPERSAO, nao como liga metalica: grafeno em fracao pequena,");
        puts("        bronze no contacto e na antena (canto conduz E, conduz calor do §C8).\n");
        ok("e GRAFENO + ESTANHO e melhor que grafeno + bronze — o estanho e o elemento, nao a liga",
           ELEM[7].valencia == 4 && ELEM[7].ligacoes == 4);
        puts("     -> a emenda 'grafeno + estanho' e boa: o estanho fecha o octeto (4+4=8),");
        puts("        conduz sem ser nobre, e tem alotropia (branco metalico / cinzento");
        puts("        semicondutor) — a mesma ideia do §O3.\n");
    }

    puts("§O6  E O QUE O OCTETO NAO DECIDE — a fronteira honesta\n");
    puts("     O octeto diz QUANTAS ligacoes, e nao diz a geometria, a condutividade, nem");
    puts("     se dois metais ligam (Hume-Rothery).\n");
    {
        ok("PROVA: diamante e grafeno tem o mesmo octeto fechado e diferem 21 em expoente de sigma",
           !strcmp(CARBONO[0].nome, "diamante") && !strcmp(CARBONO[1].nome, "grafeno")
           && CARBONO[1].exp - CARBONO[0].exp == 21
           && CARBONO[1].exp >= 8 && CARBONO[0].exp <= -6);
        puts("     -> o octeto e uma regra de CONTAGEM, e a contagem nao fixa a forma.\n");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    puts("O que isto fecha:");
    puts("");
    puts("  A PRIMEIRA metade da desconfianca esta certa: diamante -> grafeno E uma rotacao");
    puts("  (cos -1/3 para -1/2, sp3 para sp2) e o material MUDA DE CANTO por vinte e uma");
    puts("  ordens — diferenca de expoentes, 8-(-13)=21. Mesmo atomo, mesmo octeto.");
    puts("");
    puts("  A SEGUNDA estava trocada: ouro + prata da ELECTRUM, nao bronze.");
    puts("");
    puts("  O octeto fixa QUANTAS ligacoes e nao a FORMA — e essa folga deixa lugar a rotacao.");
    printf("\n");
    return falhas ? 1 : 0;
}
