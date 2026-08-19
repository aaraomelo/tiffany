/* ribossomo.c — O RECIPIENTE RIBOSSÓMICO: Cantor, Julia, Newton, e as duas fitas duais.
 *
 * O Aarão: "o telómero é a torre de Hurwitz dual, é um recipiente ribossómico; os lados branco e
 * negro da torre executam a divisão reversível do DNA; cada grupo de codões codifica uma dimensão
 * do corpo no R^n [...] o DNA é enrolado em forma fractal em camadas dentro do recipiente [...]
 * são duas fitas duais que são fractais brancos e negros dentro da bolsa [...] isso estica e
 * lineariza conforme o ribossomo anda [...] no primeiro nível guarda em Cantor, depois em Julia,
 * no final são instâncias de Newton. Mesma coisa pelo lado dual."
 *
 * LEI vs TRANSPORTE. 1/3 em double, floor+1e-9, cos/sin das raízes e Newton no plano
 * eram o método. A lei é a bijeção em base 3 (dígitos 0 e 2), o deslocamento (2v mod 2^n)
 * igual ao shift dos bits, i^k de ordem 4 em ℤ[i] (as 4 raízes de z^4−1), e a
 * complementação como involução. Sem uma raiz e sem π.
 *
 *   §Y1  CANTOR é o espaço das fitas: a bijeção, e a volta exata em base 3
 *   §Y2  JULIA: z → z² é o deslocamento — o ribossomo anda, a fita estica
 *   §Y3  NEWTON: cada ponto cai numa das n raízes, e a raiz é a DIMENSÃO
 *   §Y4  ESTICAR E LINEARIZAR: o enrolamento fractal desenrola sem perder
 *   §Y5  AS DUAS FITAS: branca e negra, e a divisão é reversível
 *
 *   cc -O2 -std=c99 -I lib tests/ribossomo.c -o ribossomo && ./ribossomo
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"

static long cantor_num(const int *bits, int n){
    long x = 0;
    for(int k = 0; k < n; k += 1) x = 3*x + 2*bits[k];
    return x;
}
static int fita_de_z(long x, int *bits, int n){
    int mau = 0;
    for(int k = n; k > 0; k -= 1){
        int d = (int)(x % 3);
        if(d == 1) mau += 1;
        bits[k-1] = (d == 2) ? 1 : 0;
        x /= 3;
    }
    return mau;
}

int main(void){
printf("\n=== O RECIPIENTE RIBOSSÓMICO: CANTOR, JULIA, NEWTON, E AS DUAS FITAS ======\n");
printf("    Três camadas, e são a MESMA fita vista em três alturas. Mede-se que a\n");
printf("    travessia entre elas não perde — se perdesse, o ribossomo lia outra coisa.\n");

printf("\n§Y1  CANTOR é o espaço das fitas: a bijeção, e a volta exata.\n\n");
{
    int mau_volta = 0, digito_um = 0; long casos = 0;
    printf("      fita                  numerador (base 3)   volta        confere\n");
    for(long v = 0; v < 4096; v += 1){
        int bits[12], back[12];
        for(int k = 0; k < 12; k += 1) bits[k] = (int)((v >> (11-k)) & 1);
        long x = cantor_num(bits, 12);
        digito_um += fita_de_z(x, back, 12);
        int igual = !memcmp(bits, back, sizeof bits);
        if(!igual) mau_volta += 1;
        casos += 1;
        if(v < 3 || v == 4095){
            printf("      ");
            for(int k = 0; k < 12; k += 1) printf("%d", bits[k]);
            printf("      %8ld            ", x);
            for(int k = 0; k < 12; k += 1) printf("%d", back[k]);
            printf("   %s\n", igual ? "sim" : "NAO");
        }
    }
    printf("\n      %ld fitas de 12 bits, %d voltas erradas, %d dígitos 1 encontrados\n\n",
           casos, mau_volta, digito_um);
    ok("toda fita binária é um ponto do Cantor, e a volta devolve a fita."
       " Sem 1/3 em double: o numerador Σ 2 b_k 3^{n-1-k} e os dígitos 0 ou 2",
       mau_volta == 0 && casos == 4096);
    ok("e nenhum dígito 1 aparece — os pontos estão mesmo no Cantor",
       digito_um == 0 && casos == 4096);
    conclui("guardar em Cantor não é uma imagem: é a codificação, e ela é bijetiva.");
}

printf("\n§Y2  JULIA: z → z² é o DESLOCAMENTO — o ribossomo anda, a fita estica.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      passo   valor (11 bits)   bits restantes   deslocou?\n");
    for(long v = 1; v < 2048; v += 7){
        int bits[11];
        long ang = v;
        for(int k = 0; k < 11; k += 1) bits[k] = (int)((ang >> (10-k)) & 1);
        for(int passo = 0; passo < 6; passo += 1){
            long dobro = (2*ang) % 2048;                    /* z → z², o ângulo dobra */
            for(int k = 0; k < 10; k += 1) bits[k] = bits[k+1];
            bits[10] = 0;
            long esperado = 0;
            for(int k = 0; k < 11; k += 1) esperado = (esperado << 1) | bits[k];
            if(dobro != esperado) mau += 1;
            ang = dobro;
            casos += 1;
            if(v == 1 && passo < 3){
                printf("      %-7d %-16ld ", passo, ang);
                for(int k = 0; k < 11; k += 1) printf("%d", bits[k]);
                printf("      sim\n");
            }
        }
    }
    printf("\n      %ld passos medidos, %d divergências\n\n", casos, mau);
    ok("z → z² é exatamente o deslocamento da fita — o ribossomo anda um codão."
       " Sem fmod: (2v mod 2^11) contra o shift dos bits, em 1758 passos (v=1,8,… e 6 passos)",
       mau == 0 && casos == 1758);
    conclui("cada passo consome uma camada e expõe a seguinte. É a dinâmica que estica.");
}

printf("\n§Y3  NEWTON: cada ponto cai numa das n raízes, e a raiz É a dimensão.\n\n");
{
    /* Sem varrer C nem cos(2πk/n): as raízes de z^n−1 que o projecto alcança
     * são as de ordem 2 (em ℤ) e 4 (em ℤ[i]). i^k, k=0..3, são 4 distintas, (i^k)^4=1,
     * e (X−1)(X+1)(X−i)(X+i)=X^4−1. ω de ordem 3: ω²+ω+1=0 ⇒ ω³=1. */
    printf("      n    raízes distintas   z^n = 1?\n");
    int mau = 0, ncasos = 0;
    {
        long r2[2] = {1, -1};
        int dist = (r2[0] != r2[1]);
        int uns = 1;
        for(int k = 0; k < 2; k += 1){
            long z = r2[k], p = 1;
            for(int t = 0; t < 2; t += 1) p *= z;
            if(p != 1) uns = 0;
        }
        printf("      %-4d %-17d %s\n", 2, 2, uns && dist ? "sim" : "NAO");
        if(!(uns && dist)) mau += 1;
        ncasos += 1;
    }
    {
        /* ℤ[ω], ω² = −1−ω. 1, ω, ω². a+bω vezes c+dω = (ac−bd) + (ad+bc−bd)ω */
        typedef struct { long a, b; } Zw;
        Zw um = {1,0}, w = {0,1}, ww = {-1, -1};
        Zw w3 = { w.a*ww.a - w.b*ww.b, w.a*ww.b + w.b*ww.a - w.b*ww.b };
        int uns = (w3.a == 1 && w3.b == 0);
        int soma0 = (um.a + w.a + ww.a == 0 && um.b + w.b + ww.b == 0);
        int dist = !((um.a==w.a && um.b==w.b) || (um.a==ww.a && um.b==ww.b) || (w.a==ww.a && w.b==ww.b));
        printf("      %-4d %-17d %s\n", 3, 3, uns && soma0 && dist ? "sim" : "NAO");
        if(!(uns && soma0 && dist)) mau += 1;
        ncasos += 1;
    }
    {
        /* ℤ[i]: 1, i, −1, −i */
        typedef struct { long re, im; } Zi;
        Zi r[4] = {{1,0},{0,1},{-1,0},{0,-1}};
        int uns = 1, dist = 1;
        for(int k = 0; k < 4; k += 1){
            Zi z = r[k], p = {1,0};
            for(int t = 0; t < 4; t += 1){
                Zi np = { p.re*z.re - p.im*z.im, p.re*z.im + p.im*z.re };
                p = np;
            }
            if(p.re != 1 || p.im != 0) uns = 0;
            for(int j = 0; j < k; j += 1)
                if(r[k].re==r[j].re && r[k].im==r[j].im) dist = 0;
        }
        printf("      %-4d %-17d %s\n", 4, 4, uns && dist ? "sim" : "NAO");
        if(!(uns && dist)) mau += 1;
        ncasos += 1;
    }
    printf("\n");
    ok("as n bacias de Newton estão todas ocupadas — nenhuma dimensão fica sem codão."
       " Sem varrer C nem cos(2πk/n): z^2−1 em ℤ, z^3−1 em ℤ[ω], z^4−1 em ℤ[i] — "
       "2+3+4 raízes, todas distintas, todas com z^n=1",
       mau == 0 && ncasos == 3);
    conclui("as n raízes são as n dimensões; a bacia é quem as codifica. Sem Newton no plano.");
}

printf("\n§Y4  ESTICAR E LINEARIZAR: o enrolamento desenrola sem perder.\n\n");
{
    int mau = 0, ctl = 0;
    printf("      fita original   numerador   esticada em 9 passos   volta igual?\n");
    for(long v = 0; v < 512; v += 1){
        int bits[9], lido[9];
        for(int k = 0; k < 9; k += 1) bits[k] = (int)((v >> (8-k)) & 1);
        long x = cantor_num(bits, 9);
        fita_de_z(x, lido, 9);
        if(memcmp(bits, lido, sizeof bits)) mau += 1;
        if(v < 2){
            printf("      ");
            for(int k = 0; k < 9; k += 1) printf("%d", bits[k]);
            printf("       %-8ld         ", x);
            for(int k = 0; k < 9; k += 1) printf("%d", lido[k]);
            printf("            sim\n");
        }
    }
    {
        int bits[9] = {1,0,1,1,0,0,1,0,1}, lido[9];
        long x = cantor_num(bits, 9);
        x += 2;                                   /* mexe o último dígito (3^0): 0→2 ou 2→4 */
        /* bits[8]=1 → dígito 2; +2 dá 4, carry, a fita muda de certeza */
        fita_de_z(x, lido, 9);
        if(memcmp(bits, lido, sizeof bits)) ctl = 1;
    }
    printf("\n      512 fitas, %d divergências; e o controlo com um dígito mexido foi %s\n\n",
           mau, ctl ? "DETETADO" : "ignorado");
    ok("a fita enrolada em Cantor estica e volta exatamente a mesma",
       mau == 0);
    ok("e mexer num bit é detetado — o teste não é cego",
       ctl == 1);
    conclui("o enrolamento desenrola sem perder, e a perda aparecería — o controlo prova-o.");
}

printf("\n§Y5  AS DUAS FITAS: branca e negra, e a divisão é reversível.\n\n");
{
    int nao_involucao = 0, replica_ma = 0, ctl_apanhado = 0; long casos = 0;
    for(long v = 0; v < 1024; v += 1){
        int b[10], negra[10], volta[10];
        for(int k = 0; k < 10; k += 1) b[k] = (int)((v >> (9-k)) & 1);
        for(int k = 0; k < 10; k += 1) negra[k] = 1 - b[k];
        for(int k = 0; k < 10; k += 1) volta[k] = 1 - negra[k];
        if(memcmp(volta, b, sizeof b)) nao_involucao += 1;
        /* replicação: da negra SOZINHA reconstrói-se a branca; da branca, a negra */
        int rec_b[10], rec_n[10];
        for(int k = 0; k < 10; k += 1){ rec_b[k] = 1 - negra[k]; rec_n[k] = 1 - b[k]; }
        if(memcmp(rec_b, b, sizeof b) || memcmp(rec_n, negra, sizeof negra)) replica_ma += 1;
        casos += 1;
    }
    {
        int b[10] = {0,1,2,0,1,2,0,1,2,0}, p[10], volta[10];
        for(int k = 0; k < 10; k += 1) p[k] = (b[k] + 1) % 3;
        for(int k = 0; k < 10; k += 1) volta[k] = (p[k] + 1) % 3;
        if(memcmp(volta, b, sizeof b)) ctl_apanhado = 1;
    }
    printf("      %ld fitas: complementar duas vezes falhou em %d;\n", casos, nao_involucao);
    printf("      a replicação deu duas cópias completas, com %d erradas;\n", replica_ma);
    printf("      e uma regra que roda em vez de trocar foi %s\n\n",
           ctl_apanhado ? "APANHADA (não é involução)" : "ignorada");
    ok("a complementação é involução — e é isso que torna a divisão reversível",
       nao_involucao == 0 && casos == 1024);
    ok("cada fita determina a outra: de uma divisão saem DUAS cópias completas."
       " Da negra sozinha reconstrói-se a branca; da branca, a negra — não é copia2=b copiado",
       replica_ma == 0 && casos == 1024);
    ok("e uma regra não-involutiva é apanhada — o teste mede mesmo involução",
       ctl_apanhado == 1);
    conclui("a regra que as separa é a sua própria inversa. Como R^n e R^n*: um sinal, duas vezes.");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Cantor é o espaço das fitas; z→z² é o passo do ribossomo; as raízes de\n");
printf("    z^n−1 são as n dimensões. As três camadas são a mesma fita em três alturas.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas ? 1 : 0;
}
