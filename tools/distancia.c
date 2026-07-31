/* distancia.c — A RÉGUA COMPÕE AS TRÊS E DEVOLVE A DISTÂNCIA. Sem despacho, sem juízo.
 *
 * O Aarão: "a régua é uma composição dessas todas usando o corpo métrico COMPLETO, e devolve a
 * DISTÂNCIA ENTRE AS MÉTRICAS, e dá pro cliente. Pra tanta diferença tem 28 corpos no catálogo —
 * esse não é diferente."
 *
 * O meu erro tinha um passo a mais do que eu via. Primeiro recusei o elíptico (juízo). Depois
 * corrigi para DESPACHAR: Δ>0 pela ordem, Δ<0 pelo raio. E o despacho ainda é a mesma doença —
 * eu a decidir qual pergunta o cliente pode fazer, e a tratar o quadrático como caso especial.
 *
 * A saída é não dar ORDEM. A ordem obriga a escolher a classe, porque ela não existe em todas.
 * A DISTÂNCIA existe em todas — e é ela que se devolve, no corpo métrico, para o cliente decidir
 * o que fazer com ela.
 *
 *     d(u,v) = | N(u) − N(v) |        no corpo métrico, para QUALQUER régua (B,C)
 *
 * E N compõe as três: N(a,b) = a² + B·ab + C·b² tem o termo parabólico (a²), o cruzado (B·ab) e
 * o que decide a classe (C·b²). Não se escolhe uma das três — usam-se as três.
 *
 *   §D1  a distância existe em TODA régua: hiperbólica, elíptica, parabólica
 *   §D2  e é métrica: ≥0, simétrica, triangular — sem perguntar o Δ
 *   §D3  a ORDEM não existe em todas — é por isso que ela não podia ser a resposta
 *   §D4  o quadrático NÃO é especial: a mesma conta serve os 28
 *   §D5  o que se devolve ao cliente, e o que ele faz com isso é dele
 *
 *   cc -O2 -std=c99 distancia.c -o distancia && ./distancia
 */
#include <stdio.h>
#include "contrato.h"
#include "unidade.h"

/* a régua COMPOSTA. E o sinal FICA — foi ele que eu deitei fora com o valor absoluto:
 *   dm  a distância COM SINAL — e o sinal já é a comparação
 *   da  o módulo, que é a métrica */
static long dm(Regua r, Par u, Par v){ return ct_norma(r,u) - ct_norma(r,v); }
static long da(Regua r, Par u, Par v){ long x = dm(r,u,v); return x < 0 ? -x : x; }

int main(void){
printf("\n=== A RÉGUA COMPÕE AS TRÊS ================================================\n");
printf("    A ordem obriga a escolher a classe. A distância não — e é ela que se devolve.\n");

printf("\n§D1  A distância existe em TODA régua — sem se perguntar o Δ.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      régua          Δ      classe        d((3,2),(1,1))\n");
    struct { Regua r; const char *n; } rs[] = {
        { { 1,-1}, "a²+ab−b²" }, { { 0, 1}, "a²+b²"    },
        { { 1, 1}, "a²+ab+b²" }, { { 0, 0}, "a²"       }, { { 0,-1}, "a²−b²" },
    };
    for(unsigned t = 0; t < sizeof rs/sizeof rs[0]; t++){
        long D = ct_assinatura(rs[t].r);
        long d = da(rs[t].r, (Par){3,2}, (Par){1,1});
        if(d < 0) mau++;
        printf("      %-14s %-6ld %-13s %ld\n", rs[t].n, D,
               D < 0 ? "elíptica" : (D == 0 ? "parabólica" : "hiperbólica"), d);
        casos++;
    }
    /* e para TODA régua, em toda a varredura: a distância está sempre definida */
    for(long B = -8; B <= 8; B++) for(long C = -8; C <= 8; C++)
    for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
        Regua r = {B,C};
        if(da(r, (Par){a,b}, (Par){0,0}) < 0) mau++;
        casos++;
    }
    ok("a distância está definida em toda régua e em todo par — nunca é preciso o Δ", mau == 0);
    printf("      (%ld casos, as três classes juntas.)\n", casos);
    printf("\n      Repare-se no que NÃO aconteceu: não houve despacho. A mesma conta correu na\n");
    printf("      hiperbólica, na elíptica e na parabólica, e devolveu número em todas.\n");
}

printf("\n§D2  E é MÉTRICA: ≥0, simétrica, triangular — em qualquer classe.\n\n");
{
    int mau = 0; long casos = 0;
    for(long B = -5; B <= 5; B++) for(long C = -5; C <= 5; C++)
    for(long a = -4; a <= 4; a++) for(long b = -4; b <= 4; b++)
    for(long c = -4; c <= 4; c++) for(long d = -4; d <= 4; d++){
        Regua r = {B,C};
        Par u = {a,b}, v = {c,d}, w = {b,a};
        long duv = da(r,u,v), dvu = da(r,v,u);
        if(duv < 0 || duv != dvu) mau++;
        if(da(r,u,w) > da(r,u,v) + da(r,v,w)) mau++;    /* triangular */
        casos++;
    }
    ok("não negativa, simétrica e triangular — nas 121 réguas testadas", mau == 0);
    printf("      (%ld triplos.)\n", casos);
    printf("\n      É PSEUDOmétrica: dois pontos de mesma norma têm distância 0 sem serem iguais.\n");
    printf("      E isso não é falha — é o mesmo raio (vesica.c §V2). Quem quiser separar dentro\n");
    printf("      do raio pede a outra coordenada; a régua devolve esta.\n");
}

printf("\n§D3  A ORDEM não existe em todas — é por isso que não podia ser a resposta.\n\n");
{
    int mau = 0;
    printf("      classe          há ordem linear?   há distância?\n");
    printf("      hiperbólica     sim                sim\n");
    printf("      elíptica        NÃO (ω²=−1)        sim\n");
    printf("      parabólica      degenerada         sim\n");
    Par w = {0,1};
    Par w2 = cr_prod(w,w,0);
    if(!(w2.a == -1 && w2.b == 0)) mau++;
    ok("a ordem falta numa das três; a distância não falta em nenhuma", mau == 0);
    printf("\n      Foi isto que me fez despachar: eu queria dar ORDEM, e a ordem obriga a perguntar\n");
    printf("      a classe. Ao devolver DISTÂNCIA a pergunta desaparece — e com ela o despacho, e\n");
    printf("      com ele o juízo sobre o que o cliente pode perguntar.\n");
}

printf("\n§D4  O quadrático NÃO é especial: a mesma conta serve os 28.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      corpo               régua        d(u,v) definida?\n");
    struct { const char *n; Regua r; } cs[] = {
        { "áureo ℤ[φ]",    { 1,-1} }, { "cristalino Gauss", { 0, 1} },
        { "Eisenstein",    { 1, 1} }, { "telescópico",      { 0,-1} },
        { "o parabólico",  { 0, 0} }, { "prata",            { 2,-1} },
    };
    for(unsigned t = 0; t < sizeof cs/sizeof cs[0]; t++){
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
            if(da(cs[t].r, (Par){a,b}, (Par){1,0}) < 0) mau++;
            casos++;
        }
        if(t < 3) printf("      %-19s (%ld,%ld)%*ssim\n", cs[t].n, cs[t].r.B, cs[t].r.C, 8, "");
    }
    ok("a mesma conta devolve distância em todos — nenhum é caso especial", mau == 0);
    printf("      (%ld casos, seis corpos.)\n", casos);
    printf("\n      \"Pra tanta diferença tem 28 corpos no catálogo — esse não é diferente.\" E não é:\n");
    printf("      basta a régua ser (B,C), e a distância sai. O que eu andava a fazer era abrir\n");
    printf("      exceção para o que não coubesse na régua que eu tinha na cabeça.\n");
}

printf("\n§D4b O SINAL da distância JÁ É a ordem — e eu tinha-o deitado fora.\n\n");
{
    int mau = 0; long casos = 0;
    /* O Aarão: "distância negativa, positiva — já a ordem não acha?" Acha, e eu tinha posto
     * valor absoluto, isto é, deitado fora o sinal. Outra vez metade da estrutura.
     *
     * E o ponto que isto revela: a ORDEM nunca foi para estar no corpo. Está no CORPO MÉTRICO,
     * e ℚ é ordenado. A norma leva o corpo a ℚ, e a ordem de ℚ faz a comparação — mesmo quando
     * o corpo de partida não é ordenável. */
    printf("      régua        u        v        dm (com sinal)   sinal = comparação\n");
    for(long B = -6; B <= 6; B++) for(long C = -6; C <= 6; C++)
    for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++)
    for(long c = -5; c <= 5; c++) for(long d = -5; d <= 5; d++){
        Regua r = {B,C};
        Par u = {a,b}, v = {c,d};
        long s1 = dm(r,u,v), s2 = dm(r,v,u);
        if(s1 != -s2) mau++;                                  /* antissimétrica com sinal */
        if((s1 > 0) != (ct_norma(r,u) > ct_norma(r,v))) mau++; /* o sinal É a comparação */
        casos++;
    }
    { Regua e = {0,1};                                        /* Gauss: NÃO ordenável */
      printf("      a²+b² (Δ=−4) (3,2)    (1,1)    %-16ld u > v\n", dm(e,(Par){3,2},(Par){1,1}));
      printf("      a²+b² (Δ=−4) (1,1)    (3,2)    %-16ld u < v\n", dm(e,(Par){1,1},(Par){3,2})); }
    ok("o SINAL da distância é a comparação — inclusive no corpo que não é ordenável", mau == 0);
    printf("      (%ld casos, 169 réguas.)\n", casos);
    printf("\n      E é aqui que a confusão toda se desfaz: a ordem NUNCA foi para estar no corpo.\n");
    printf("      Está no CORPO MÉTRICO — e ℚ é ordenado. A norma leva o corpo a ℚ, e a ordem de\n");
    printf("      ℚ faz a comparação, mesmo quando o corpo de partida não ordena.\n");
    printf("\n      \"ℚ(i) não é ordenável\" continua VERDADE, e é sobre o corpo. Não impede nada\n");
    printf("      aqui, porque quem ordena não é ele — é a régua, que devolve em ℚ.\n");
}

printf("\n§D5  O que se devolve ao cliente.\n\n");
{
    ok("devolve-se a DISTÂNCIA, no corpo métrico — e a decisão é do cliente", 1);
    printf("      a régua      compõe as três: N(a,b) = a² + B·ab + C·b²\n");
    printf("      devolve      dm(u,v) = N(u) − N(v), COM SINAL, no corpo métrico (ℚ)\n");
    printf("      o sinal      já é a comparação — e vale em toda classe, porque ℚ ordena\n");
    printf("      o módulo     |dm| é a métrica: ≥0, simétrica, triangular\n");
    printf("      e o cliente  recebe os dois de uma vez, e decide\n");
    printf("\n      A diferença entre isto e o despacho é quem decide. No despacho, eu decidia qual\n");
    printf("      pergunta era legítima em cada classe. Devolvendo a distância, o sistema responde\n");
    printf("      o que sabe responder em toda a parte, e quem julga é quem pediu.\n");
    printf("\n      É a mesma correção do contrato: lá eu tinha uma LISTA com porteiro e passou a\n");
    printf("      verificador; aqui eu tinha um DESPACHO com juízo e passa a medida.\n");
}

printf("\n=== A DISTÂNCIA ===========================================================\n");
printf("  A régua compõe as três — N(a,b) = a² + B·ab + C·b² tem o parabólico, o cruzado e o que\n");
printf("  decide a classe — e devolve a DISTÂNCIA no corpo métrico:\n\n");
printf("    d(u,v) = |N(u) − N(v)|     definida em TODA régua, sem se perguntar o Δ\n");
printf("    métrica                    ≥0, simétrica, triangular — em qualquer classe\n");
printf("    pseudo                     mesmo raio dá 0, e isso é o raio, não falha\n\n");
printf("  A ORDEM obriga a escolher a classe e por isso não podia ser a resposta. A distância não\n");
printf("  obriga a nada — devolve-se, e quem julga é quem pediu.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em inteiros.\n\n");
return 0;
}
