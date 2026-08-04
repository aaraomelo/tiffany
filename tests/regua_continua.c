/* regua_continua.c — A RÉGUA É GRADUADA E CONTÍNUA. Em ℤ ela não opera; em ℚ opera.
 *
 * O Aarão: "você está usando a régua dentro do corpo métrico — a régua é graduada contínua.
 * Senão você não consegue fazer operações com réguas. Precisa usar o corpo de fato contínuo."
 *
 * Ele tem razão, e a correção derruba uma afirmação minha: eu escrevi em topologia.c §P6 que "o
 * espaço dos corpos é ℤ pelas assinaturas". É falso, e o preço aparece em duas coisas que eu já
 * tinha visto sem entender:
 *
 *   1. o transporte só existia quando B₁ ≡ B₂ mod 2, porque t = (B₂−B₁)/2 tinha de ser inteiro.
 *      Eu tratei essa paridade como propriedade do mecanismo. É artefato de eu ter posto a
 *      régua em ℤ.
 *   2. com B, C inteiros, Δ = B²−4C ≡ B² ≡ 0 ou 1 (mod 4). Logo Δ = 2, 3, 6, 7, … NÃO EXISTEM.
 *      O "espaço dos corpos" que eu desenhei tinha BURACOS, e eu chamei-lhe reta.
 *
 * A régua é graduada — os inteiros são as MARCAS — e contínua entre as marcas. E a continuidade
 * não é enfeite: é o que permite OPERAR com réguas. Somar duas, escalar uma, tomar o ponto médio
 * entre dois corpos — nada disso fecha em ℤ, e tudo fecha em ℚ.
 *
 * E ℚ está aqui, exato, desde o racional_pg.c: sem um único float.
 *
 *   §Y1  com régua INTEIRA, Δ só toma valores ≡ 0 ou 1 mod 4 — o espaço tem buracos
 *   §Y2  e o transporte só existe com B₁ ≡ B₂ mod 2 — a paridade era minha, não do mecanismo
 *   §Y3  em ℚ todo Δ é alcançado, e o transporte existe SEMPRE
 *   §Y4  as OPERAÇÕES com réguas: somar, escalar, ponto médio — fecham em ℚ, nenhuma em ℤ
 *   §Y5  entre duas marcas há corpo: o ponto médio entre Δ=5 e Δ=8 é Δ=13/2, e existe
 *   §Y6  a correção, dita: não é ℤ, é ℚ — e ℤ é a graduação
 *
 *   cc -O2 -std=c99 regua_continua.c -o regua_continua && ./regua_continua
 */
#include <stdio.h>
#include "corpos.h"
#include "unidade.h"

/* a régua no corpo CONTÍNUO: B e C são racionais, exatos, sem float */
typedef struct { Par B, C; } RegQ;
static Par q(long n, long d){ return ra_classe((Par){n,d}); }
static Par q_sub(Par x, Par y){ return ra_soma(x, (Par){-y.a, y.b}); }
static Par delta_q(RegQ r){                      /* Δ = B² − 4C */
    return q_sub(ra_prod(r.B, r.B), ra_prod(q(4,1), r.C)); }
static int  q_eq(Par x, Par y){ x = ra_classe(x); y = ra_classe(y); return x.a==y.a && x.b==y.b; }
static Par  q_meio(Par x, Par y){ return ra_prod(ra_soma(x,y), q(1,2)); }

int main(void){
printf("\n=== A RÉGUA É GRADUADA E CONTÍNUA =========================================\n");
printf("    Em ℤ ela não opera. Os inteiros são as MARCAS, não o espaço.\n");

printf("\n§Y1  Com régua INTEIRA, Δ só toma valores ≡ 0 ou 1 mod 4 — há BURACOS.\n\n");
{
    int mau = 0; long alcanca[8] = {0};
    printf("      Δ     alcançável com (B,C) inteiros?\n");
    for(long D = 0; D <= 12; D++){
        int achou = 0;
        for(long B = -40; B <= 40 && !achou; B++)
            for(long C = -400; C <= 400; C++)
                if(B*B - 4*C == D){ achou = 1; break; }
        long r = ((D % 4) + 4) % 4;
        if(achou != (r == 0 || r == 1)) mau++;
        if(achou) alcanca[r]++;
        if(D <= 7) printf("      %-5ld %s\n", D, achou ? "sim" : "NÃO — o buraco");
    }
    ok("Δ inteiro é alcançável EXATAMENTE quando Δ ≡ 0 ou 1 (mod 4)", mau == 0);
    printf("\n      Metade dos inteiros não é assinatura de régua nenhuma. Eu desenhei uma reta e\n");
    printf("      contei vizinhos a distância 1 — e os vizinhos de Δ=5 são Δ=4 e Δ=6, mas Δ=6 NÃO\n");
    printf("      EXISTE em ℤ. A minha reta tinha buracos e eu chamei-lhe reta.\n");
}

printf("\n§Y2  E o transporte só existia com B₁ ≡ B₂ mod 2 — a paridade era MINHA.\n\n");
{
    int mau = 0; long liga = 0, nao = 0;
    for(long B1 = -8; B1 <= 8; B1++) for(long B2 = -8; B2 <= 8; B2++){
        int inteiro = ((B2 - B1) % 2) == 0;
        if(inteiro) liga++; else nao++;
    }
    if(nao == 0) mau++;
    ok("em ℤ, metade dos pares de bases NÃO tem transporte — e não é do mecanismo", mau == 0);
    printf("      (%ld pares com transporte inteiro, %ld sem.)\n", liga, nao);
    printf("\n      Eu tratei essa paridade como propriedade da matemática. É consequência de eu ter\n");
    printf("      posto a régua em ℤ: t = (B₂−B₁)/2 pede uma DIVISÃO, e ℤ não divide.\n");
}

printf("\n§Y3  Em ℚ todo Δ é alcançado, e o transporte existe SEMPRE.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      Δ pedido    régua (B,C) que o dá        confere?\n");
    for(long n = -12; n <= 12; n++) for(long d = 1; d <= 6; d++){
        Par D = q(n,d);
        /* a secção: B = 0, C = −Δ/4 — existe para TODO racional Δ */
        RegQ r = { q(0,1), ra_prod(D, q(-1,4)) };
        if(!q_eq(delta_q(r), D)) mau++;
        if(n == 13 || (n == 6 && d == 1) || (n == 13 && d == 2)) ;
        casos++;
    }
    { RegQ a = { q(0,1), q(-6,4) }; printf("      6           (0, -3/2)                   %s\n",
        q_eq(delta_q(a), q(6,1)) ? "sim ✓" : "NÃO"); }
    { RegQ a = { q(0,1), q(-13,8) }; printf("      13/2        (0, -13/8)                  %s\n",
        q_eq(delta_q(a), q(13,2)) ? "sim ✓" : "NÃO"); }
    /* e o transporte: t = (B₂−B₁)/2 é racional, logo existe sempre */
    int mau2 = 0; long c2 = 0;
    for(long b1 = -6; b1 <= 6; b1++) for(long b2 = -6; b2 <= 6; b2++){
        Par B1 = q(b1,1), B2 = q(b2,1);
        Par t = ra_prod(q_sub(B2, B1), q(1,2));
        /* transportar B₁ por t tem de dar B₂: B' = B + 2t */
        Par Bl = ra_soma(B1, ra_prod(q(2,1), t));
        if(!q_eq(Bl, B2)) mau2++;
        c2++;
    }
    ok("todo Δ racional tem régua, e o transporte t = (B₂−B₁)/2 existe sempre em ℚ",
       mau == 0 && mau2 == 0);
    printf("      (%ld assinaturas pedidas, %ld pares de bases — nenhum sem transporte.)\n",
           casos, c2);
    printf("\n      A paridade some. Não porque eu a tenha contornado: porque ela nunca foi do\n");
    printf("      mecanismo — era a régua estar no corpo errado.\n");
}

printf("\n§Y4  As OPERAÇÕES com réguas: somar, escalar, ponto médio.\n\n");
{
    int mau = 0; long casos = 0;
    printf("      operação           em ℤ            em ℚ\n");
    printf("      somar réguas       fecha           fecha\n");
    printf("      escalar por 1/2    NÃO fecha       fecha\n");
    printf("      ponto médio        NÃO fecha       fecha\n");
    for(long b1 = -6; b1 <= 6; b1++) for(long c1 = -6; c1 <= 6; c1++)
    for(long b2 = -6; b2 <= 6; b2++) for(long c2 = -6; c2 <= 6; c2++){
        RegQ r1 = { q(b1,1), q(c1,1) }, r2 = { q(b2,1), q(c2,1) };
        /* somar */
        RegQ so = { ra_soma(r1.B, r2.B), ra_soma(r1.C, r2.C) };
        if(so.B.b != 1 || so.C.b != 1) mau++;               /* a soma fica inteira */
        /* o PONTO MÉDIO — e é aqui que ℤ falha */
        RegQ me = { q_meio(r1.B, r2.B), q_meio(r1.C, r2.C) };
        /* em ℚ existe sempre; em ℤ só quando as paridades batem */
        int cabe_em_Z = (((b1+b2) % 2) == 0) && (((c1+c2) % 2) == 0);
        if(cabe_em_Z != (me.B.b == 1 && me.C.b == 1)) mau++;
        casos++;
    }
    ok("somar fecha nos dois; o ponto médio só fecha em ℚ — e é aí que ℤ quebra", mau == 0);
    printf("      (%ld pares de réguas.)\n", casos);
    printf("\n      É esta a frase dele: SENÃO NÃO SE CONSEGUE FAZER OPERAÇÕES COM RÉGUAS. Uma régua\n");
    printf("      que não se pode dividir ao meio não é régua graduada — é uma lista de marcas.\n");
}

printf("\n§Y5  Entre duas marcas há corpo: o meio de Δ=5 e Δ=8 é Δ=13/2, e existe.\n\n");
{
    int mau = 0;
    Par D1 = q(5,1), D2 = q(8,1);
    Par Dm = q_meio(D1, D2);
    RegQ rm = { q(0,1), ra_prod(Dm, q(-1,4)) };
    if(!q_eq(delta_q(rm), Dm)) mau++;
    if(!q_eq(Dm, q(13,2))) mau++;
    printf("      Δ₁ = 5 (áureo)   Δ₂ = 8 (prata)   meio = %ld/%ld   régua (0, %ld/%ld)   existe ✓\n",
           Dm.a, Dm.b, rm.C.a, rm.C.b);
    /* e o meio é hiperbólico como os dois extremos — a classe é preservada no segmento */
    if(ra_cmp(Dm, q(0,1)) <= 0) mau++;
    /* mas de Δ>0 a Δ<0 o segmento CRUZA o zero, e o cruzamento é exato */
    Par Da = q(5,1), Db = q(-4,1);
    /* Δ(s) = Da + s(Db−Da); zera em s = Da/(Da−Db) = 5/9 */
    Par s = ra_prod(Da, (Par){ q_sub(Da,Db).b, q_sub(Da,Db).a });
    Par Dz = ra_soma(Da, ra_prod(s, q_sub(Db, Da)));
    if(!q_eq(Dz, q(0,1))) mau++;
    if(!q_eq(s, q(5,9))) mau++;
    printf("      de Δ=5 a Δ=−4 o segmento cruza ZERO em s = %ld/%ld — exato, e é a FRONTEIRA\n",
           s.a, s.b);
    ok("entre duas marcas há corpo, e a travessia de classe tem ponto de cruzamento EXATO",
       mau == 0);
    printf("\n      O parabólico deixa de ser \"um ponto isolado da reta\" e passa a ser o que é: a\n");
    printf("      FRONTEIRA que o caminho contínuo atravessa. Em ℤ isso não se podia dizer, porque\n");
    printf("      não havia caminho — havia saltos.\n");
}

printf("\n§Y6  A correção, dita: não é ℤ, é ℚ — e ℤ é a GRADUAÇÃO.\n\n");
{
    conclui("a régua vive no corpo contínuo; os inteiros são as marcas dele");
    printf("      eu escrevi        \"o espaço dos corpos é ℤ pelas assinaturas\"  (topologia.c §P6)\n");
    printf("      o certo é         o espaço é ℚ (denso), e ℤ são as GRADUAÇÕES da régua\n");
    printf("\n      E os dois sintomas que eu já tinha visto sem entender:\n");
    printf("        a paridade B₁≡B₂ mod 2 no transporte — não era do mecanismo, era de ℤ\n");
    printf("        Δ ≡ 0 ou 1 mod 4 — metade dos inteiros não era assinatura de régua nenhuma,\n");
    printf("        e eu contei \"vizinhos a distância 1\" sobre buracos\n");
    printf("\n      Nada disto pede float: ℚ está exato aqui desde o racional_pg.c. O corpo contínuo\n");
    printf("      que faltava já estava no toolkit — eu é que estava a medir a régua com a régua\n");
    printf("      errada.\n");
}

printf("\n=== A RÉGUA CONTÍNUA ======================================================\n");
printf("  A régua é graduada E contínua. Em ℤ ela não opera:\n\n");
printf("    Δ inteiro       só existe quando Δ ≡ 0 ou 1 (mod 4) — metade são buracos\n");
printf("    o transporte    só existia com B₁ ≡ B₂ mod 2 — artefato de ℤ, não do mecanismo\n");
printf("    o ponto médio   não fecha em ℤ; e sem ele não há operação com réguas\n\n");
printf("  Em ℚ tudo fecha, exato e sem float: todo Δ tem régua, todo par tem transporte, e entre\n");
printf("  duas marcas há corpo — o meio de Δ=5 e Δ=8 é Δ=13/2. E a travessia de classe passa a\n");
printf("  ter ponto de cruzamento EXATO: de Δ=5 a Δ=−4 o zero está em s = 5/9.\n\n");
printf("  Correção: \"o espaço dos corpos é ℤ\" estava errado. É ℚ, e ℤ é a graduação.\n");
if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — exato, em racionais, sem um único float.\n\n");
return 0;
}
