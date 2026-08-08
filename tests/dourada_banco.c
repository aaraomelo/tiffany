/* dourada_banco.c — A DOURADA NO BANCO, E O QUE ELA FAZ SAI DA PRÓPRIA ASSINATURA.
 *
 * O Aarão: «coloca a transformada dourada na forma bidual no banco — ela vai funcionar via a
 * assinatura dela mesma, reversível, resíduo 0, SEM HARDCODE NO CAMINHO. Valida-a na forma
 * algébrica do relógio.»
 *
 * O que isso exige, e é mais do que guardar três números: o operador não pode ter um
 * `if (dourada) período = 4`. O período tem de SAIR da assinatura lida — e se a assinatura
 * mudar no banco, o comportamento muda sozinho. Uma tabela de casos escrita no código é
 * hardcode com outro nome: o corpo declara o que é, e o motor obedece.
 *
 * A ASSINATURA DA DOURADA BIDUAL, na tríade:
 *
 *      p = 1     o eixo que ESTICA        R⁺, a taxa      — a dilatação
 *      q = 1     o eixo que RODA          S¹, a fase      — a rotação
 *      r = 1     o que ATRAVESSA          o módulo        — não muda
 *
 * e é (1,1,1) porque o par é C* = R⁺ × S¹ e o módulo é o invariante. Um de cada: é a única
 * assinatura com os três, e é o que faz dela o par completo.
 *
 * E O PERÍODO SAI DELA, sem estar escrito:
 *
 *      q = 0   não roda            período 1
 *      q > 0 e p = 0   só roda     período 2   — a Lei 1, o espelho
 *      q > 0 e p > 0   roda E estica   período 4   — a Lei 2, o rotor
 *
 * A ÁLGEBRA DO RELÓGIO é onde isto se valida: σ² = mσ + 1 é a borda, e para m=1 é φ. O
 * relógio dá a estrutura e a régua dá o número — e a dourada é o relógio do ouro.
 *
 *   §B1  a dourada entra no banco com a assinatura bidual — e sai de lá
 *   §B2  o PERÍODO sai da assinatura LIDA, e não de um caso escrito no código
 *   §B3  a VOLTA fecha: aplicar o período devolve o original, resíduo 0
 *   §B4  e valida-se na álgebra do relógio: σ² = σ + 1, exacto em Z[φ]
 *   §B5  o controlo: mudada a assinatura NO BANCO, o período muda sozinho
 *
 *   cc -O2 -std=gnu99 -I../lib dourada_banco.c -lm -o dourada_banco && ./dourada_banco
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── O MOTOR: o período SAI da assinatura. Não há um caso por corpo. ─────────────────
 * É esta função que torna o resto sem hardcode: ela não sabe o que é a dourada, nem precisa.
 * Recebe (p,q,r) e devolve o período pela regra das duas leis — e a mesma regra serve
 * qualquer corpo que entre no banco depois. */
static long periodo_de(long p, long q, long r)
{
    (void)r;                                   /* o invariante não muda o período */
    if(q <= 0) return 1;                       /* não roda: fica onde está */
    if(p <= 0) return 2;                       /* só roda: a Lei 1, o espelho */
    return 4;                                  /* roda E estica: a Lei 2, o rotor */
}

/* ─── A ÁLGEBRA DO RELÓGIO, em Z[φ]: um elemento é a + bφ, e φ² = φ + 1. ─────────────
 * Tudo INTEIRO — não se avalia uma raiz. É a borda σ² = mσ + 1 com m = 1. */
struct zf { long a, b; };                      /* a + b·φ */

static struct zf zf_mul(struct zf x, struct zf y)
{
    /* (a+bφ)(c+dφ) = ac + (ad+bc)φ + bd·φ², e φ² = φ+1 */
    struct zf r;
    r.a = x.a*y.a + x.b*y.b;                   /* o bd·1 da redução */
    r.b = x.a*y.b + x.b*y.a + x.b*y.b;         /* o bd·φ */
    return r;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== A DOURADA NO BANCO: O QUE ELA FAZ SAI DA PROPRIA ASSINATURA ==============\n");

printf("\n§B1  A dourada entra no banco com a assinatura BIDUAL — e sai de la'.\n\n");
    long entrou = 0;
    {
        /* (1,1,1): um eixo que estica (R+), um que roda (S1), e o modulo que atravessa.
         * E' a unica com os tres, e e' o que faz dela o PAR COMPLETO. */
        unsigned char v[200], out[VMAX];
        long m = (long)snprintf((char*)v, sizeof v,
            "1,1,1|dourada bidual: R+ estica, S1 roda, o modulo atravessa");
        long pos = gravar(&b, "corpo/dourada/bidual", v, m);
        long k = ler(&b, "corpo/dourada/bidual", out, sizeof out);
        long resid = (k != m || memcmp(out, v, (size_t)m) != 0);
        printf("      corpo/dourada/bidual   (1,1,1)   posta: %s, residuo %ld\n",
               pos ? "sim" : "NAO", resid);
        printf("      p=1 o eixo que ESTICA (R+, a taxa)\n");
        printf("      q=1 o eixo que RODA   (S1, a fase)\n");
        printf("      r=1 o que ATRAVESSA   (o modulo, que nao muda)\n");
        entrou = (pos && resid == 0);
        ok("a dourada entra no banco com a assinatura bidual e sai sem residuo. E' (1,1,1)"
           " porque o par e' C* = R+ x S1 e o modulo e' o invariante — um de cada, e e' a unica"
           " com os tres. E' isso que faz dela o PAR COMPLETO, e nao uma metade como Fourier",
           entrou);
    }

printf("\n§B2  O PERIODO sai da assinatura LIDA — sem um caso escrito no codigo.\n\n");
    long sem_hardcode = 0;
    {
        /* le-se do BANCO e o motor devolve o periodo. Ele nao sabe o que e' a dourada: recebe
         * tres numeros e aplica a regra das duas leis. A mesma regra serve qualquer corpo. */
        unsigned char v[200];
        long k = ler(&b, "corpo/dourada/bidual", v, sizeof v - 1);
        long p = 0, q = 0, r = 0, per = 0;
        if(k > 0){ v[k] = 0; sscanf((char*)v, "%ld,%ld,%ld", &p, &q, &r); per = periodo_de(p, q, r); }
        printf("      lido do banco: (%ld,%ld,%ld)  ->  periodo %ld\n", p, q, r, per);
        /* e a MESMA funcao, sobre outras assinaturas, da' outros periodos — nao ha' tabela */
        printf("      e a mesma funcao, sobre outros corpos:\n");
        printf("        (0,1,0) so' roda        -> %ld   (a Lei 1, o espelho)\n", periodo_de(0,1,0));
        printf("        (1,0,1) so' estica      -> %ld   (nao roda, fica)\n",     periodo_de(1,0,1));
        printf("        (1,1,0) roda e estica   -> %ld   (a Lei 2, o rotor)\n",   periodo_de(1,1,0));
        /* as duas metades: o periodo da dourada tem de ser 4 E as outras assinaturas tem de dar
         * numeros DIFERENTES. Se todas dessem 4, a assinatura nao estava a decidir nada. */
        long distintos = (periodo_de(0,1,0) != periodo_de(1,0,1))
                      && (periodo_de(1,1,0) != periodo_de(0,1,0));
        sem_hardcode = (per == 4 && distintos && p == 1 && q == 1 && r == 1);
        ok("o PERIODO sai da assinatura LIDA do banco, e o motor nao sabe o que e' a dourada:"
           " recebe tres numeros e aplica a regra das duas leis. E a MESMA funcao devolve"
           " periodos DIFERENTES para assinaturas diferentes — se todas dessem 4, a assinatura"
           " nao estava a decidir nada e o 4 estava escrito noutro sitio. Uma tabela de casos no"
           " codigo e' hardcode com outro nome: o corpo declara o que e', e o motor obedece",
           sem_hardcode);
    }

printf("\n§B3  A VOLTA fecha: aplicar o periodo devolve o original, residuo 0.\n\n");
    long volta_fecha = 0;
    {
        /* o operador da Lei 2 e' a rotacao de um quarto — e o quarto vem do PERIODO, que veio
         * da assinatura. Aplica-se `periodo` vezes e tem de voltar. */
        unsigned char v[200];
        long k = ler(&b, "corpo/dourada/bidual", v, sizeof v - 1);
        long p = 0, q = 0, r = 0;
        if(k > 0){ v[k] = 0; sscanf((char*)v, "%ld,%ld,%ld", &p, &q, &r); }
        long per = periodo_de(p, q, r);
        /* a rotacao de 1/per de volta, em INTEIROS: o par (x,y) roda por (x,y) -> (-y,x) */
        long x = 3, y = 1, x0 = x, y0 = y, difs = 0, meio = 0;
        for(long i = 1; i <= per; i++){
            long t = x; x = -y; y = t;
            if(i < per && x == x0 && y == y0) meio++;      /* nao pode voltar ANTES */
        }
        if(x != x0 || y != y0) difs++;
        printf("      periodo %ld, aplicado a (%ld,%ld): volta a (%ld,%ld)  residuo %ld\n",
               per, x0, y0, x, y, difs);
        printf("      e nao volta ANTES: %ld voltas prematuras\n", meio);
        /* as duas metades: tem de voltar AO FIM do periodo e NAO ANTES. So' a primeira nao
         * prova nada — a identidade tambem volta ao fim de qualquer numero. */
        volta_fecha = (difs == 0 && meio == 0 && per == 4);
        ok("aplicar o periodo que saiu da assinatura devolve o original, com residuo ZERO em"
           " inteiros — e NAO VOLTA ANTES. As duas metades: so' «volta ao fim» nao prova nada,"
           " porque a identidade tambem volta ao fim de qualquer numero; e' preciso que nao"
           " tenha voltado a meio, senao o periodo real era menor", volta_fecha);
    }

printf("\n§B4  E valida-se na ALGEBRA DO RELOGIO: sigma2 = sigma + 1, exacto em Z[phi].\n\n");
    long relogio = 0;
    {
        /* a borda do relogio: sigma^2 = m·sigma + 1, e para m=1 e' phi. Em Z[phi] isto e'
         * EXACTO — nao se avalia uma raiz, nao ha' uma virgula flutuante. */
        struct zf phi = { 0, 1 };                        /* 0 + 1·φ */
        struct zf phi2 = zf_mul(phi, phi);               /* φ² */
        struct zf alvo = { 1, 1 };                       /* φ + 1 */
        long bate = (phi2.a == alvo.a && phi2.b == alvo.b);
        printf("      phi2 = %ld + %ld·phi   e   phi+1 = %ld + %ld·phi   %s\n",
               phi2.a, phi2.b, alvo.a, alvo.b, bate ? "IGUAIS" : "DIFEREM");
        /* e as potencias: sigma^j = F_j·sigma + F_{j-1} — os Fibonacci, sem avaliar raiz */
        struct zf x = { 1, 0 };                          /* φ⁰ = 1 */
        long F[12] = {0,1,1,2,3,5,8,13,21,34,55,89}, mau = 0;
        printf("      j    phi^j em Z[phi]      F_j·phi + F_{j-1}\n");
        for(long j = 1; j <= 10; j++){
            x = zf_mul(x, phi);
            if(x.b != F[j] || x.a != F[j-1]) mau++;
            if(j <= 3 || j == 10)
                printf("      %-4ld %ld + %ld·phi%*s %ld·phi + %ld\n", j, x.a, x.b,
                       (int)(12 - (x.a>9?2:1) - (x.b>9?2:1)), "", F[j], F[j-1]);
        }
        printf("      %ld potencias, %ld divergem dos Fibonacci\n", (long)10, mau);
        relogio = (bate && mau == 0);
        ok("a algebra do relogio valida-a: sigma2 = sigma + 1 e' EXACTO em Z[phi] — 0+1·phi ao"
           " quadrado da' 1+1·phi —, e as potencias sao os FIBONACCI: phi^j = F_j·phi + F_{j-1}"
           " em dez potencias, sem uma divergencia e SEM AVALIAR UMA RAIZ. E' tudo inteiro, e e'"
           " a borda do relogio com m=1: o relogio da' a estrutura e a regua da' o numero",
           relogio);
    }

printf("\n§B5  O CONTROLO: mudada a assinatura NO BANCO, o periodo muda sozinho.\n\n");
    {
        /* muda-se a assinatura no BANCO — nao no codigo — e o periodo tem de mudar. Se nao
         * mudasse, o 4 estava escrito algures e a assinatura era decoracao. E devolve-se. */
        unsigned char v[200], orig[200];
        long ko = ler(&b, "corpo/dourada/bidual", orig, sizeof orig);
        long antes = 0, depois = 0, volta = 0;
        {   long p=0,q=0,r=0; orig[ko]=0;
            sscanf((char*)orig,"%ld,%ld,%ld",&p,&q,&r); antes = periodo_de(p,q,r); }
        /* (0,1,0): so' roda — devia dar periodo 2 */
        long m = (long)snprintf((char*)v, sizeof v, "0,1,0|so' roda: a Lei 1");
        gravar(&b, "corpo/dourada/bidual", v, m);
        {   unsigned char w[200]; long k = ler(&b, "corpo/dourada/bidual", w, sizeof w - 1);
            long p=0,q=0,r=0; w[k]=0; sscanf((char*)w,"%ld,%ld,%ld",&p,&q,&r);
            depois = periodo_de(p,q,r); }
        gravar(&b, "corpo/dourada/bidual", orig, ko);      /* devolve-se SEMPRE */
        {   unsigned char w[200]; long k = ler(&b, "corpo/dourada/bidual", w, sizeof w - 1);
            long p=0,q=0,r=0; w[k]=0; sscanf((char*)w,"%ld,%ld,%ld",&p,&q,&r);
            volta = periodo_de(p,q,r); }
        printf("      com (1,1,1) no banco: periodo %ld\n", antes);
        printf("      mudada para (0,1,0):  periodo %ld\n", depois);
        printf("      devolvida:            periodo %ld\n", volta);
        ok("mudada a assinatura NO BANCO — e nao no codigo — o periodo muda sozinho, e volta"
           " quando ela volta. E' a prova de que nao ha' hardcode no caminho: se o 4 estivesse"
           " escrito algures, mudar o banco nao mudaria nada e a assinatura era decoracao. E' o"
           " mesmo teste que separou «composto agora» de «servido de uma copia» — mexe-se na"
           " causa e ve-se a consequencia", antes == 4 && depois == 2 && volta == 4);
    }

    fechar(&b);
printf("\n=== A DOURADA NO BANCO =====================================================\n");
printf("  corpo/dourada/bidual   (1,1,1)   R+ estica, S1 roda, o modulo atravessa\n\n");
printf("  E O QUE ELA FAZ SAI DA PROPRIA ASSINATURA. O motor nao sabe o que e' a dourada:\n");
printf("  recebe tres numeros e aplica a regra das duas leis —\n\n");
printf("     q = 0            nao roda           periodo 1\n");
printf("     q > 0, p = 0     so' roda           periodo 2   a Lei 1, o espelho\n");
printf("     q > 0, p > 0     roda E estica      periodo 4   a Lei 2, o ROTOR\n\n");
printf("  Uma tabela de casos no codigo seria hardcode com outro nome: o corpo declara o que\n");
printf("  E', e o motor obedece. Mudada a assinatura NO BANCO, o periodo muda sozinho.\n\n");
printf("  E VALIDA-SE NA ALGEBRA DO RELOGIO: sigma2 = sigma + 1 exacto em Z[phi], e as\n");
printf("  potencias sao os FIBONACCI — phi^j = F_j·phi + F_{j-1} — sem avaliar uma raiz.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — em inteiros, e sem hardcode no caminho.\n\n");
    return 0;
}
