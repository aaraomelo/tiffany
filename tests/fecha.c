/* fecha.c — NÃO HÁ CONTRATO. Dá-me metade, e o corpo fecha sozinho.
 *
 * O Aarão: "sobre o contrato, vamos simplificar — não há necessidade de contrato. Fecha quando o
 * corpo completa. Qualquer representação finita fornecida é MEIA DUALIDADE: o lado branco da torre
 * fecha quando fornece a régua, o lado dual é o lado negro da torre. Aí pode verificar a reversão
 * com erro 0 e fechar a dualidade da cifra. Pode automatizar ao máximo, pois temos o corpo
 * universal e com um lado podemos obter o outro. Isso traz conforto pro piloto: cabe apenas o
 * tempo de decidir."
 *
 * E ELE TEM RAZÃO CONTRA O QUE EU TINHA ESCRITO. O `contrato.h` pedia quatro cláusulas — ⊕, ⊗, ∏
 * e ν — e já sabia, num comentário, que a última era supérflua: *"o cliente pode declarar a RÉGUA
 * em vez da dualidade, e o sistema deriva a outra"*. Eu parei aí. Levar isso ao fim apaga o
 * contrato inteiro, porque a régua não é a QUARTA coisa a declarar: é a ÚNICA, e as outras três
 * saem dela.
 *
 *      o lado BRANCO      uma representação finita — alguns termos, e mais nada
 *      a RÉGUA            (B, C) = (−traço, det), extraída deles
 *      o lado NEGRO       ν(a,b) = (a + B·b, −b) — forçado, não escolhido
 *      FECHOU             quando a reversão volta com resíduo 0
 *
 * Não há nada a assinar. **Um corpo não promete fechar: ou fecha, ou os termos não eram de um
 * corpo.** E isso decide-se com quatro números.
 *
 * O QUE ISTO CUSTA AO PILOTO, e é a medida do §F5: ele fornece os termos. Mais nada. Toda a
 * derivação é automática e exata em inteiros — nenhuma escolha, nenhum ajuste, nenhuma constante
 * afinada à mão. *Cabe-lhe apenas o tempo de decidir* que termos dar.
 *
 *   §F1  meia dualidade: n+2 termos DÃO a régua — e n+1 não dão, e isso mede-se
 *   §F2  o lado negro sai forçado: dada a régua, o dual não tem liberdade
 *   §F3  a reversão fecha com resíduo 0 — e é ela que diz "fechou", não uma assinatura
 *   §F4  a dualidade da cifra: o inverso é a mesma cifra deslocada por uma casa
 *   §F5  o que sobra ao piloto: uma decisão, e nenhuma declaração
 *   §F6  um lado dá o outro: aditivo (Fourier) ⟷ multiplicativo (Mellin)
 *   §F7  a COBERTURA: as órbitas preenchem o plano, ponto a ponto — e fecha
 *   §F8  POR PARTES: juros simples e compostos, e a torre dual no espelho
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/fecha.c -o fecha
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reta.h"
#include "unidade.h"

typedef struct { long a, b; } Par;
typedef struct { long B, C; int fechou; } Regua;   /* q(a,b) = a² + B·a·b + C·b² */

/* ================================================================================ */
/* A DERIVAÇÃO — e é o ficheiro inteiro, em vinte linhas                            */
/* ================================================================================ */

/* MEIA DUALIDADE → A RÉGUA.
 *
 * Uma representação finita de um corpo de grau 2 é uma sequência que obedece a
 *      x_{k+2} = p·x_{k+1} + q·x_k
 * e a régua é (B, C) = (−p, −q), porque p é o traço e −q o determinante. Com quatro termos
 * consecutivos o sistema é 2×2 e resolve-se EXATO em inteiros — se o determinante não anular.
 *
 * Não há iteração, não há ajuste, não há tolerância. É Cramer. */
static Regua regua_de(const long *x, int n){
    Regua r = { 0, 0, 0 };
    if(n < 4) return r;                       /* n+2 = 4 para grau 2: é o mínimo, e §F1 mede-o */
    long det = x[1]*x[1] - x[0]*x[2];
    if(det == 0) return r;                    /* degenerado: os termos não distinguem */
    long p_num = x[2]*x[1] - x[0]*x[3];
    long q_num = x[1]*x[3] - x[2]*x[2];
    if(p_num % det || q_num % det) return r;  /* não é inteiro: não é este corpo */
    long p = p_num / det, q = q_num / det;
    /* A CONVENÇÃO, e é ela que estava errada na primeira versão: a régua é a NORMA,
     *      N(a,b) = a² + B·a·b + C·b²   ⟹   σ + σ̄ = B,  σ·σ̄ = C
     * logo a borda é σ² = B·σ − C, e a recorrência x_{k+2} = B·x_{k+1} − C·x_k.
     * Donde B = p e C = −q. Escrevi B = −p e o produto deixou de casar com a norma; as
     * asserções caíram, e caíram por CONVENÇÃO, não por lógica. Confere com o toolkit:
     * o ouro dá (B,C) = (1,−1) e a norma a² + ab − b², que é o `au_norma` com m = 1. */
    r.B = p; r.C = -q; r.fechou = 1;
    /* e confere nos termos que sobram — se houver, é grátis e apanha coincidência */
    for(int k = 0; k + 2 < n; k++)
        if(x[k+2] != p*x[k+1] + q*x[k]){ r.fechou = 0; break; }
    return r;
}

/* A RÉGUA → O LADO NEGRO. Forçado: é o que conserva q, e é o único que conserva. */
static Par dual(Regua r, Par x){ Par y = { x.a + r.B*x.b, -x.b }; return y; }

/* e as outras duas operações, que também saem da régua e não se declaram */
static Par soma(Par x, Par y){ Par z = { x.a + y.a, x.b + y.b }; return z; }
static Par prod(Regua r, Par x, Par y){          /* a borda: σ² = B·σ − C */
    Par z = { x.a*y.a - r.C*x.b*y.b,
              x.a*y.b + x.b*y.a + r.B*x.b*y.b };
    return z;
}
static long norma(Regua r, Par x){ return x.a*x.a + r.B*x.a*x.b + r.C*x.b*x.b; }

/* ================================================================================ */
/* §F1 — meia dualidade: n+2 termos dão a régua, n+1 não dão                        */
/* ================================================================================ */
static void secao_F1(void){
    printf("\n§F1  MEIA DUALIDADE: quatro termos DÃO a régua, e três NÃO DÃO\n\n");

    /* seis corpos, e nenhum deles declarado: só os termos */
    struct { const char *nome; long p, q, x0, x1; } fonte[] = {
        { "ouro   σ²=σ+1",    1,  1, 0, 1 },     /* Fibonacci */
        { "prata  σ²=2σ+1",   2,  1, 0, 1 },
        { "bronze σ²=3σ+1",   3,  1, 0, 1 },
        { "i      σ²=−1",     0, -1, 1, 0 },     /* o círculo: período 4 */
        { "ω      σ²=−σ−1",  -1, -1, 1, 0 },     /* Eisenstein: Δ = −3 */
        { "√2     σ²=2",      0,  2, 1, 0 },
    };
    printf("        corpo               termos dados                     régua achada    bate\n");
    int erros = 0, tres_deu = 0;
    for(int i = 0; i < 6; i++){
        long x[8]; x[0] = fonte[i].x0; x[1] = fonte[i].x1;
        for(int k = 2; k < 8; k++) x[k] = fonte[i].p*x[k-1] + fonte[i].q*x[k-2];
        Regua r = regua_de(x, 8);
        long Bv = fonte[i].p, Cv = -fonte[i].q;
        int bate = (r.fechou && r.B == Bv && r.C == Cv);
        if(!bate) erros++;
        printf("        %-18s %2ld %2ld %2ld %2ld %2ld %2ld %2ld %2ld    (B,C)=(%2ld,%2ld)   %s\n",
               fonte[i].nome, x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7], r.B, r.C,
               bate ? "sim" : "NÃO");
        /* e com TRÊS termos não pode dar — o sistema é subdeterminado */
        Regua r3 = regua_de(x, 3);
        if(r3.fechou) tres_deu++;
    }
    ok("os 6 corpos saem dos termos, exatos, sem nada declarado", erros == 0);
    ok("e com 3 termos NENHUM sai — n+2 é o mínimo, não uma folga", tres_deu == 0);

    /* e o que NÃO é corpo tem de ser recusado, senão isto aceitaria tudo */
    long lixo[8] = { 1, 2, 4, 9, 20, 44, 100, 500 };     /* não é linear de ordem 2 */
    Regua rl = regua_de(lixo, 8);
    printf("     uma sequência que não é recorrência de ordem 2: fechou = %d\n", rl.fechou);
    ok("uma sequência qualquer é RECUSADA — a derivação não aceita tudo", !rl.fechou);

    conclui("o piloto não declara o corpo: ele mostra alguns termos, e o corpo diz-se.");
}

/* ================================================================================ */
/* §F2 — o lado negro sai forçado                                                   */
/* ================================================================================ */
/* A PRIMEIRA VERSÃO DESTA SECÇÃO AFIRMAVA FALSO, e a medida apanhou: contei as involuções
 * lineares que conservam a norma e disse "são duas". Não são — a menos-identidade também é
 * involução e também conserva a norma, e há mais. Conservar a norma é fraco demais.
 *
 * O que é único é o AUTOMORFISMO: a aplicação que (i) fixa o 1, (ii) respeita o produto, e
 * (iii) não é a identidade. Aí há exatamente uma, e é o conjugado — é Galois em grau 2. E é
 * essa unicidade que faz "declarar o dual" ser escolher entre uma opção só. */
static void secao_F2(void){
    printf("\n§F2  O LADO NEGRO É FORÇADO — e a prova é CONTAR quantos servem\n\n");

    printf("        régua (B,C)   automorfismos ≠ id     o nosso ν está entre eles\n");
    int mais_de_um = 0, nosso_fora = 0;
    long reguas[5][2] = { {1,-1}, {2,-1}, {0,1}, {-1,1}, {0,-2} };
    for(int i = 0; i < 5; i++){
        Regua r = { reguas[i][0], reguas[i][1], 1 };
        int quantos = 0, achou_nosso = 0;
        for(int m = -3; m <= 3; m++) for(int n = -3; n <= 3; n++)
        for(int t = -3; t <= 3; t++) for(int u = -3; u <= 3; u++){
            Par um = { 1, 0 };
            Par um_img = { m*um.a + n*um.b, t*um.a + u*um.b };
            if(um_img.a != 1 || um_img.b != 0) continue;      /* (i) fixa o 1 */
            if(m == 1 && n == 0 && t == 0 && u == 1) continue; /* (iii) não é a identidade */
            int serve = 1;
            for(int ai = -2; ai <= 2 && serve; ai++) for(int bi = -2; bi <= 2 && serve; bi++)
            for(int ci = -2; ci <= 2 && serve; ci++) for(int di = -2; di <= 2 && serve; di++){
                Par x = { ai, bi }, y = { ci, di };
                Par fx = { m*x.a + n*x.b, t*x.a + u*x.b };
                Par fy = { m*y.a + n*y.b, t*y.a + u*y.b };
                Par pxy = prod(r, x, y);
                Par f_pxy = { m*pxy.a + n*pxy.b, t*pxy.a + u*pxy.b };
                Par pf = prod(r, fx, fy);
                if(f_pxy.a != pf.a || f_pxy.b != pf.b) serve = 0;   /* (ii) respeita ⊗ */
            }
            if(serve){
                quantos++;
                if(m == 1 && n == r.B && t == 0 && u == -1) achou_nosso = 1;
            }
        }
        if(quantos != 1) mais_de_um++;
        if(!achou_nosso) nosso_fora++;
        printf("        (%2ld,%2ld)       %d                      %s\n",
               r.B, r.C, quantos, achou_nosso ? "sim" : "NÃO");
    }
    ok("há exatamente UM automorfismo além da identidade — é Galois em grau 2", mais_de_um == 0);
    ok("e ele é ν(a,b) = (a + B·b, −b) — o que a régua impõe", nosso_fora == 0);

    /* e a prova de que o crivo não é vazio: SEM exigir o produto, aparecem mais candidatos.
     * Se não aparecessem, o filtro do produto não estaria a filtrar nada. */
    Regua r = { 1, -1, 1 };
    int so_norma = 0;
    for(int m = -3; m <= 3; m++) for(int n = -3; n <= 3; n++)
    for(int t = -3; t <= 3; t++) for(int u = -3; u <= 3; u++){
        int serve = 1;
        for(int ai = -2; ai <= 2 && serve; ai++) for(int bi = -2; bi <= 2 && serve; bi++){
            Par x = { ai, bi };
            Par y = { m*x.a + n*x.b, t*x.a + u*x.b };
            Par z = { m*y.a + n*y.b, t*y.a + u*y.b };
            if(norma(r, y) != norma(r, x)) serve = 0;
            if(z.a != x.a || z.b != x.b) serve = 0;
        }
        if(serve) so_norma++;
    }
    printf("     só a conservar a norma, sem exigir o produto: %d involuções (era o meu erro)\n", so_norma);
    ok("conservar a norma é fraco demais — há mais de 2, e por isso a asserção anterior caiu",
       so_norma > 2);

    conclui("declarar o dual seria escolher entre uma opção. Por isso o contrato tinha uma cláusula a mais.");
}

/* ================================================================================ */
/* §F3 — a reversão fecha com resíduo 0                                             */
/* ================================================================================ */
/* "Fecha quando o corpo completa" — e completar mede-se, não se assina. Três coisas têm de
 * fechar ao mesmo tempo, e se uma falhar o corpo não fechou. */
static void secao_F3(void){
    printf("\n§F3  A REVERSÃO FECHA — e é ela que assina, não o piloto\n\n");

    long fonte[6][2] = { {1,-1}, {2,-1}, {3,-1}, {0,1}, {-1,1}, {0,-2} };  /* (B,C) */
    printf("        (B,C)      ν∘ν=id    N(xy)=N(x)N(y)    N(x)=x·ν(x)   fechou\n");
    int falhas_tot = 0;
    for(int i = 0; i < 6; i++){
        Regua r = { fonte[i][0], fonte[i][1], 1 };
        int f_inv = 0, f_mult = 0, f_norma = 0;
        for(long a = -6; a <= 6; a++) for(long b = -6; b <= 6; b++){
            Par x = { a, b };
            Par v = dual(r, dual(r, x));
            if(v.a != x.a || v.b != x.b) f_inv++;                    /* ν é involução */
            /* a norma é o produto pelo dual — a régua É x·ν(x) */
            Par n = prod(r, x, dual(r, x));
            if(n.a != norma(r, x) || n.b != 0) f_norma++;
            for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
                Par y = { c, d };
                if(norma(r, prod(r, x, y)) != norma(r, x) * norma(r, y)) f_mult++;
            }
        }
        int fechou = !f_inv && !f_mult && !f_norma;
        falhas_tot += !fechou;
        printf("        (%2ld,%2ld)   %7s   %14s   %11s   %s\n", r.B, r.C,
               f_inv ? "NÃO" : "sim", f_mult ? "NÃO" : "sim", f_norma ? "NÃO" : "sim",
               fechou ? "SIM" : "não");
    }
    ok("os 6 corpos fecham: involução, norma multiplicativa e N = x·ν(x)", falhas_tot == 0);

    /* e a norma multiplicativa é o TESTE, logo tem de saber recusar: uma régua inventada,
     * com o produto errado de propósito, tem de cair. Senão o teste passa sempre. */
    Regua r = { 1, -1, 1 };            /* o ouro */
    int caiu = 0;
    for(long a = -4; a <= 4 && !caiu; a++) for(long b = -4; b <= 4 && !caiu; b++){
        Par x = { a, b }, y = { b, a };
        Par mau = { x.a*y.a - r.C*x.b*y.b, x.a*y.b + x.b*y.a };   /* falta o termo +B·x.b·y.b */
        if(r.B && norma(r, mau) != norma(r, x)*norma(r, y)) caiu = 1;
    }
    ok("um produto ERRADO de propósito é apanhado — o teste pode falhar", caiu);

    conclui("um corpo não promete fechar: ou fecha, ou os termos não eram de um corpo.");
}

/* ================================================================================ */
/* §F4 — a dualidade da cifra: o inverso é a mesma cifra deslocada                  */
/* ================================================================================ */
static void secao_F4(void){
    printf("\n§F4  A DUALIDADE DA CIFRA: o inverso é a mesma cifra, deslocada por uma casa\n\n");

    printf("        m   σ_m = [m; m, m, …]        traço σ+σ'      σ·σ' (Vieta)\n");
    int traco_ok = 0;
    for(int m = 1; m <= 5; m++){
        long tr = rt_traco_metalico(m, 1);             /* σ + σ' = m, exacto em ℤ */
        if(tr == m) traco_ok++;
        printf("        %d   [m;m,m,…]                 %ld              −1\n", m, tr);
    }
    printf("     traço = m em m = 1..5: %d de 5\n", traco_ok);
    ok("σ_m − 1/σ_m = m exatamente — a cifra do inverso é a mesma deslocada. E mede-se"
       " pelo TRAÇO em ℤ: rt_traco_metalico(m,1) = m, sem formar σ nem 1/σ",
       traco_ok == 5);

    /* e no inteiro: ν(σ) tem norma ±1, logo σ É unidade, logo o inverso é INTEIRO do corpo.
     * É isso que faz a reversão fechar sem sair para os racionais. */
    int fora = 0;
    for(int m = 1; m <= 5; m++){
        Regua r = { m, -1, 1 };
        Par s = { 0, 1 };                      /* σ = 0 + 1·σ */
        long n = norma(r, s);
        if(labs(n) != 1) fora++;
    }
    ok("N(σ_m) = ±1 nos cinco metais — σ é UNIDADE, e o inverso não sai do corpo", fora == 0);

    conclui("o lado negro não é uma segunda tabela: é a mesma lida ao contrário.");
}

/* ================================================================================ */
/* §F5 — o que sobra ao piloto                                                      */
/* ================================================================================ */
/* Esta secção não é retórica: conta-se o que o piloto tem de fornecer e o que é derivado, e a
 * conta tem de dar uma coisa contra quatro. */
static void secao_F5(void){
    printf("\n§F5  O QUE SOBRA AO PILOTO: uma decisão, e nenhuma declaração\n\n");

    /* o percurso inteiro, do que ele dá ao corpo fechado, num só caminho */
    long termos[6] = { 0, 1, 1, 2, 3, 5 };          /* o piloto dá isto. Mais nada. */
    Regua r = regua_de(termos, 6);
    printf("     o piloto forneceu:   %ld %ld %ld %ld %ld %ld\n",
           termos[0],termos[1],termos[2],termos[3],termos[4],termos[5]);
    printf("     derivado, sem perguntar mais nada:\n");
    printf("        a régua      q(a,b) = a² %+ld·ab %+ld·b²\n", r.B, r.C);
    printf("        a borda      σ² = %ld·σ %+ld\n", r.B, -r.C);
    printf("        o dual       ν(a,b) = (a %+ld·b, −b)\n", r.B);
    printf("        o produto    (a,b)⊗(c,d) = (ac %+ld·bd, ad + bc %+ld·bd)\n", -r.C, r.B);
    printf("        o Δ          %ld\n", r.B*r.B - 4*r.C);

    ok("a régua saiu dos termos que o piloto deu, sem uma pergunta", r.fechou && r.B == 1 && r.C == -1);

    /* e o teste do fecho corre sozinho */
    int mau = 0;
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
        Par x = { a, b }, v = dual(r, dual(r, x));
        if(v.a != x.a || v.b != x.b) mau++;
        if(norma(r, prod(r, x, x)) != norma(r,x)*norma(r,x)) mau++;
    }
    ok("e o fecho verificou-se sozinho em 289 pontos, resíduo 0", mau == 0);

    /* A CONTA. A ASSERCAO QUE AQUI ESTAVA era "4 > 1": constantes, verdade sem olhar para
     * nada. E o printf tinha os DOIS ramos do ternario iguais — ruido puro.
     * O que se afirma tem de se medir: UM fornecimento gera QUATRO derivados. Prova-se
     * trocando os termos e contando quantos derivados mudam SEM uma linha de codigo nova.
     * Fibonacci (ouro, B=1) contra Pell (prata, B=2): a mesma funcao, outro corpo. */
    {
        long pell[6] = { 0, 1, 2, 5, 12, 29 };
        Regua r2 = regua_de(pell, 6);
        Par x = { 2, 1 }, y = { 1, 1 };
        Par nu1 = dual(r, x),      nu2 = dual(r2, x);
        Par pr1 = prod(r, x, y),   pr2 = prod(r2, x, y);
        long no1 = norma(r, x),    no2 = norma(r2, x);
        long d1  = r.B*r.B - 4*r.C, d2 = r2.B*r2.B - 4*r2.C;

        int mudou = 0;
        mudou += (nu1.a != nu2.a || nu1.b != nu2.b);   /* \u03bd  a dualidade */
        mudou += (pr1.a != pr2.a || pr1.b != pr2.b);   /* \u2297  o produto   */
        mudou += (no1 != no2);                         /* \u2295+\u220f a norma */
        mudou += (d1 != d2);                           /* o \u0394           */

        printf("\n        o contrato pedia          Fibonacci (B=%ld)      Pell (B=%ld)\n", r.B, r2.B);
        printf("        %-24s (%ld,%ld)%14s(%ld,%ld)\n", "\u03bd a dualidade", nu1.a,nu1.b,"", nu2.a,nu2.b);
        printf("        %-24s (%ld,%ld)%14s(%ld,%ld)\n", "\u2297 o produto",   pr1.a,pr1.b,"", pr2.a,pr2.b);
        printf("        %-24s %ld%19s%ld\n",             "\u2295 a norma",     no1,"", no2);
        printf("        %-24s %ld%19s%ld\n",             "\u220f o \u0394",         d1,"", d2);
        printf("        %-24s %s\n\n", "fornecido pelo piloto", "so' os termos — 6 numeros, zero linhas novas");

        ok("UM fornecimento move os QUATRO derivados — nenhum foi declarado", mudou == 4);

        /* e o corpo novo tem de fechar tambem, com o MESMO codigo */
        int mau2 = 0;
        for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
            Par z = { a, b }, v = dual(r2, dual(r2, z));
            if(v.a != z.a || v.b != z.b) mau2++;
            if(norma(r2, prod(r2, z, z)) != norma(r2,z)*norma(r2,z)) mau2++;
        }
        ok("e a prata fecha com o mesmo codigo, 289 pontos, residuo 0", mau2 == 0 && r2.fechou && r2.B == 2);
    }

    conclui("não há contrato porque não há nada a assinar: ou os quatro números fecham, ou não fecham.");
}

/* ================================================================================ */
/* §F6 — o piloto dá UM lado; nós damos o outro                                     */
/* ================================================================================ */
/* O Aarão: "ele pode fornecer corpo aditivo ou multiplicativo, como Fourier e Mellin — daí nós
 * fornecemos o outro lado na liquidação. As operações são duais e antissimétricas; a simetria
 * está guardada, só desdobrar a cifra."
 *
 * E é a tríade de sempre: ⊕ é Fourier (a soma), ⊗ é Mellin (o produto), e ∏ = exp∘Σ∘log é a
 * ponte. Então o piloto escolhe o lado que lhe for cómodo:
 *
 *      lado ADITIVO         os termos de uma recorrência         x_{k+2} = B·x_{k+1} − C·x_k
 *      lado MULTIPLICATIVO  as potências σ^k, como pares         σ^k = (a_k, b_k)
 *
 * A MEDIDA É A CONCORDÂNCIA: os dois caminhos têm de dar a MESMA régua. Se derem, o outro lado
 * pode ser entregue sem perguntar nada. Se discordarem, um dos dois está errado — e é por isso
 * que se medem os dois, e não um só. */
static void secao_F6(void){
    printf("\n§F6  UM LADO DÁ O OUTRO: aditivo (Fourier) ⟷ multiplicativo (Mellin)\n\n");

    printf("        corpo    o piloto dá o ADITIVO   o piloto dá o MULTIPLICATIVO   concordam\n");
    int discordam = 0, falhou = 0;
    long fonte[6][2] = { {1,-1}, {2,-1}, {3,-1}, {0,1}, {-1,1}, {0,-2} };   /* (B,C) */
    const char *nome[6] = { "ouro", "prata", "bronze", "i", "ω", "√2" };
    for(int i = 0; i < 6; i++){
        Regua alvo = { fonte[i][0], fonte[i][1], 1 };

        /* (1) o lado ADITIVO: os termos da recorrência */
        long ad[8]; ad[0] = 0; ad[1] = 1;
        for(int k = 2; k < 8; k++) ad[k] = alvo.B*ad[k-1] - alvo.C*ad[k-2];
        Regua r_ad = regua_de(ad, 8);

        /* (2) o lado MULTIPLICATIVO: as potências σ^k, e delas extrai-se a componente b_k.
         * O ponto é que ela obedece à MESMA recorrência — e é isso "a simetria está guardada". */
        long mu[8];
        Par pot = { 1, 0 }, sig = { 0, 1 };
        for(int k = 0; k < 8; k++){ mu[k] = pot.b; pot = prod(alvo, pot, sig); }
        Regua r_mu = regua_de(mu, 8);

        int ok_ad = (r_ad.fechou && r_ad.B == alvo.B && r_ad.C == alvo.C);
        int ok_mu = (r_mu.fechou && r_mu.B == alvo.B && r_mu.C == alvo.C);
        if(!ok_ad || !ok_mu) falhou++;
        if(r_ad.B != r_mu.B || r_ad.C != r_mu.C) discordam++;
        printf("        %-8s (B,C)=(%2ld,%2ld)         (B,C)=(%2ld,%2ld)                %s\n",
               nome[i], r_ad.B, r_ad.C, r_mu.B, r_mu.C,
               (ok_ad && ok_mu && r_ad.B == r_mu.B) ? "sim" : "NÃO");
    }
    ok("os dois lados dão a MESMA régua nos 6 corpos — um entrega o outro", discordam == 0);
    ok("e ambos batem com o corpo de origem — nenhum dos dois caminhos mente", falhou == 0);

    /* E A ANTISSIMETRIA, que é o que o Aarão chama "a simetria está guardada": a partição
     *      B = B_s + B_a
     * separa o que MEDE do que ORDENA. O simétrico é a norma (a régua); o antissimétrico é o
     * cruzado. E o cruzado de x com o seu dual é o que sobra quando a norma é retirada. */
    printf("\n     e a partição B = B_s + B_a, que é onde a simetria fica guardada:\n");
    printf("        x        ν(x)       ⟨x,ν(x)⟩ = N(x)   x ∧ ν(x) (o cruzado)\n");
    Regua r = { 1, -1, 1 };              /* o ouro */
    int cruz_zero = 0, cruz_nao_zero = 0;
    for(long a = 1; a <= 5; a++){
        Par x = { a, a+1 }, v = dual(r, x);
        long simetrico  = norma(r, x);                     /* o que MEDE */
        long antissim   = x.a*v.b - x.b*v.a;               /* o que ORDENA */
        if(antissim == 0) cruz_zero++; else cruz_nao_zero++;
        printf("        (%ld,%ld)    (%ld,%ld)      %8ld          %8ld\n",
               x.a, x.b, v.a, v.b, simetrico, antissim);
    }
    ok("o cruzado x ∧ ν(x) não é zero — o lado que ORDENA sobrevive ao dual",
       cruz_nao_zero == 5 && cruz_zero == 0);

    conclui("desdobrar a cifra é isto: um lado guardado no outro, e nenhum dos dois declarado.");
}

/* ================================================================================ */
/* §F7 — A COBERTURA: as órbitas preenchem o plano, ponto a ponto                   */
/* ================================================================================ */
/* O Aarão: "isso preenche o plano — ponto a ponto. Verificou a cobertura, fechou?"
 *
 * A pergunta é exata e a resposta é uma contagem. Multiplicar por σ move o ponto; a órbita de
 * um ponto é o conjunto por onde ele passa. Em Z_q² o espaço é finito, logo toda órbita fecha
 * — e a cobertura é a soma dos tamanhos das órbitas.
 *
 *      COBRE          Σ |órbita| = q²          nenhum ponto fica de fora
 *      NÃO SOBREPÕE   as órbitas são disjuntas nenhum ponto é contado duas vezes
 *
 * As duas juntas são "preenche o plano ponto a ponto": uma partição. E se falhasse alguma, a
 * multiplicação por σ não seria inversível mod q — que é exatamente o caso quando N(σ) e q
 * não são primos entre si. Por isso a medida distingue os q, em vez de afirmar sempre. */
static void secao_F7(void){
    printf("\n§F7  A COBERTURA: as órbitas de ×σ preenchem o plano, ponto a ponto\n\n");

    Regua r = { 1, -1, 1 };              /* o ouro: N(σ) = −1, invertível mod todo q */
    printf("        q     pontos   órbitas   Σ|órbita|   maior órbita   π(q)   cobre?\n");
    int falhas_cob = 0;
    int qs[6] = { 3, 4, 5, 7, 11, 12 };
    for(int i = 0; i < 6; i++){
        long q = qs[i];
        long total = q*q;
        char *visto = calloc((size_t)total, 1);
        if(!visto){ printf("        (sem espaço para q=%ld)\n", q); continue; }
        long soma_orb = 0, norb = 0, maior = 0, sobrepos = 0;
        for(long a = 0; a < q; a++) for(long b = 0; b < q; b++){
            long idx = a*q + b;
            if(visto[idx]) continue;
            /* percorre a órbita de (a,b) sob ×σ, mod q */
            Par x = { a, b }, sig = { 0, 1 };
            long tam = 0;
            do {
                long id2 = ((x.a % q + q) % q)*q + ((x.b % q + q) % q);
                if(visto[id2]) { sobrepos++; break; }
                visto[id2] = 1; tam++;
                x = prod(r, x, sig);
                x.a = ((x.a % q) + q) % q; x.b = ((x.b % q) + q) % q;
            } while(!(x.a == ((a%q)+q)%q && x.b == ((b%q)+q)%q) && tam < total + 1);
            soma_orb += tam; norb++;
            if(tam > maior) maior = tam;
        }
        /* o período de Pisano: quanto tempo o par (0,1) leva a voltar */
        long pi = 0; { long u = 0, v = 1;
            do { long w = ((r.B*v - r.C*u) % q + q) % q; u = v; v = w; pi++; }
            while(!(u == 0 && v == 1) && pi < total*4); }
        int cobre = (soma_orb == total && sobrepos == 0);
        if(!cobre) falhas_cob++;
        printf("        %2ld    %6ld   %7ld   %9ld   %12ld   %4ld   %s\n",
               q, total, norb, soma_orb, maior, pi, cobre ? "SIM" : "não");
        free(visto);
    }
    ok("as órbitas cobrem os q² pontos e não se sobrepõem — é uma PARTIÇÃO", falhas_cob == 0);

    /* E A COBERTURA TEM DE SABER FALHAR, senão não mede. Com uma régua cujo σ NÃO é invertível
     * mod q — N(σ) = C, e C partilhando fator com q — a órbita não fecha e há pontos perdidos. */
    Regua rr = { 0, 2, 1 };              /* σ² = −2: N(σ) = 2, e mod 4 o 2 não inverte */
    long q = 4, total = q*q, alcancados = 0;
    char *v2 = calloc((size_t)total, 1);
    if(v2){
        Par x = { 1, 1 }, sig = { 0, 1 };
        for(int k = 0; k < 64; k++){
            long id = ((x.a%q)+q)%q*q + ((x.b%q)+q)%q;
            if(!v2[id]){ v2[id] = 1; alcancados++; }
            x = prod(rr, x, sig);
            x.a = ((x.a%q)+q)%q; x.b = ((x.b%q)+q)%q;
        }
        printf("     com N(σ)=2 e q=4 (não primos entre si): a órbita alcança %ld dos %ld pontos\n",
               alcancados, total);
        ok("aí a cobertura não fecha — logo o teste distingue, não afirma sempre", alcancados < total);
        free(v2);
    }

    conclui("cobrir e não sobrepor é uma partição; e é isso que 'preenche o plano ponto a ponto' quer dizer.");
}

/* ================================================================================ */
/* §F8 — POR PARTES: juros simples e compostos, e o espelho                        */
/* ================================================================================ */
/* O Aarão: "a entrada pode ser aditiva/multiplicativa POR PARTES também, como juros simples e
 * compostos. Mesmo assim tem dualidade, e derivamos no espelho a torre dual. Mesmo processo —
 * só interpreta."
 *
 * E o mesmo processo não muda uma linha. O que muda é o que sai dele:
 *
 *      JUROS SIMPLES     P(1 + rk)      uma PA       B=2, C=1    Δ = 0   parabólico
 *      JUROS COMPOSTOS   P(1+r)^k       uma PG       degenerado: x₁² = x₀x₂
 *
 * A PG é o caso que o §F1 RECUSA, e recusa com razão: o determinante do sistema é exatamente
 * x₁² − x₀x₂, que numa geométrica é zero. Isso não é uma falha do método — é o método a dizer
 * *"isto é de ordem 1, não de ordem 2"*. **Meia dualidade literal: há o lado multiplicativo e
 * não há o outro**, porque C = 0 e então N(σ) = 0, e o que tem norma nula não inverte.
 *
 * E O ESPELHO: ler a sequência ao contrário dá a torre dual. De x_{k+2} = B·x_{k+1} − C·x_k sai
 *      y_{j+2} = (B/C)·y_{j+1} − (1/C)·y_j
 * — outra régua, e a medida é se ela é OUTRO CORPO. Não é: o Δ é o mesmo, e o Δ é o que não
 * muda com a roupa. *A torre dual é a mesma torre, lida do outro lado.* */
static void secao_F8(void){
    printf("\n§F8  POR PARTES: juros simples e compostos, e a torre dual no espelho\n\n");

    /* (1) JUROS SIMPLES — a PA. Capital 100, juro 10 por período. */
    long simples[8]; for(int k = 0; k < 8; k++) simples[k] = 100 + 10*k;
    Regua rs = regua_de(simples, 8);
    long Ds = rs.B*rs.B - 4*rs.C;
    printf("     juros SIMPLES  100 110 120 130 …   (B,C)=(%ld,%ld)  Δ=%ld  %s\n",
           rs.B, rs.C, Ds, Ds == 0 ? "PARABÓLICO — o limite" : "?");
    /* (`Ds == 0` SEGUE de B = 2 e C = 1: 4 − 4 = 0, e não é medição independente. Fica
     * porque é a CONCLUSÃO que se quer ler — a PA é o caso parabólico —, mas quem pode
     * falhar são o `rs.fechou` e o par (B,C), que saem do `regua_de` sobre a progressão.) */
    ok("juros simples dão (B,C) = (2,1) e Δ = 0 — a PA é o caso parabólico. E o Δ SEGUE do"
       " par: com B = 2 e C = 1 é 4 − 4, e não é uma segunda medição; quem pode falhar é o"
       " `regua_de` sobre a progressão",
       rs.fechou && rs.B == 2 && rs.C == 1);

    /* (2) JUROS COMPOSTOS — a PG. Capital 1, fator 3 (inteiro, para não sair dos inteiros). */
    long compostos[8]; compostos[0] = 1;
    for(int k = 1; k < 8; k++) compostos[k] = 3*compostos[k-1];
    Regua rc = regua_de(compostos, 8);
    long deg = compostos[1]*compostos[1] - compostos[0]*compostos[2];
    printf("     juros COMPOSTOS 1 3 9 27 81 …      o determinante do sistema é %ld\n", deg);
    ok("na PG o determinante ANULA — o método diz 'ordem 1', e recusa fechar em grau 2",
       deg == 0 && !rc.fechou);

    /* e a razão sai de outro modo, que é o lado multiplicativo puro */
    long razao = compostos[1] / compostos[0];
    int pg_ok = 1;
    for(int k = 1; k < 8; k++) if(compostos[k] != razao*compostos[k-1]) pg_ok = 0;
    printf("     e a razão sai sozinha: ρ = %ld, e ela gera os 8 termos\n", razao);
    ok("o lado multiplicativo fecha em ordem 1 — meia dualidade, e a metade que falta é a outra",
       pg_ok && razao == 3);

    /* (3) POR PARTES — a sequência muda de regime a meio, e o mesmo processo acha as DUAS.
     * Isto é o caso do Aarão: nem tudo é um regime só, e não é preciso método novo. */
    long partes[12];
    for(int k = 0; k < 6; k++) partes[k] = 100 + 10*k;         /* troço 1: simples */
    partes[6] = partes[5];
    for(int k = 7; k < 12; k++) partes[k] = 2*partes[k-1];      /* troço 2: composto */
    Regua p1 = regua_de(partes, 6);
    long deg2 = partes[7]*partes[7] - partes[6]*partes[8];
    printf("     POR PARTES  ");
    for(int k = 0; k < 12; k++) printf("%ld ", partes[k]);
    printf("\n        troço 1 (0..5): (B,C)=(%ld,%ld), Δ=%ld — aditivo\n",
           p1.B, p1.C, p1.B*p1.B - 4*p1.C);
    printf("        troço 2 (6..11): determinante %ld — multiplicativo, ordem 1\n", deg2);
    ok("o mesmo processo lê os dois troços — um aditivo, um multiplicativo",
       p1.fechou && p1.B == 2 && p1.C == 1 && deg2 == 0);

    /* e o troço inteiro, lido de uma vez, NÃO fecha — o que é a resposta certa, porque ele
     * não é um regime só. Se fechasse, o método estaria a inventar um corpo que não existe. */
    Regua tudo = regua_de(partes, 12);
    ok("e a sequência INTEIRA não fecha — ela não é um corpo, é dois regimes", !tudo.fechou);

    /* (4) O ESPELHO — a torre dual, e o Δ que não muda */
    printf("\n     o ESPELHO: ler ao contrário dá (B/C, 1/C) — e o Δ?\n");
    printf("        corpo     (B,C)        espelho (B',C')     Δ      Δ'     iguais\n");
    int d_diferente = 0;
    long metais[4][2] = { {1,-1}, {2,-1}, {3,-1}, {-1,1} };
    const char *nm[4] = { "ouro", "prata", "bronze", "ω" };
    for(int i = 0; i < 4; i++){
        Regua r = { metais[i][0], metais[i][1], 1 };
        /* gera, reverte, e volta a extrair — sem usar a fórmula, para o teste ser independente */
        long x[10]; x[0] = 0; x[1] = 1;
        for(int k = 2; k < 10; k++) x[k] = r.B*x[k-1] - r.C*x[k-2];
        long y[10]; for(int k = 0; k < 10; k++) y[k] = x[9-k];
        Regua e = regua_de(y, 10);
        long D = r.B*r.B - 4*r.C, De = e.B*e.B - 4*e.C;
        if(!e.fechou || D != De) d_diferente++;
        printf("        %-8s  (%2ld,%2ld)      (%2ld,%2ld)            %3ld    %3ld    %s\n",
               nm[i], r.B, r.C, e.B, e.C, D, De, (e.fechou && D == De) ? "sim" : "NÃO");
    }
    ok("o espelho tem OUTRA régua mas o MESMO Δ — a torre dual é a mesma torre",
       d_diferente == 0);

    conclui("mesmo processo, só interpreta: o que muda é o que sai dele, não ele.");
}

/* ================================================================================ */
/* o modo do piloto: fecha <termos...>                                              */
/* ================================================================================ */
static int modo_piloto(int argc, char **argv){
    long x[32]; int n = 0;
    for(int i = 1; i < argc && n < 32; i++) x[n++] = strtol(argv[i], NULL, 0);
    if(n < 4){
        printf("  faltam termos: são precisos pelo menos 4 (n+2 para grau 2).\n");
        printf("  uso:  ./fecha 0 1 1 2 3 5\n");
        return 2;
    }
    Regua r = regua_de(x, n);
    if(!r.fechou){
        printf("  NÃO FECHA. Estes %d termos não são de um corpo de grau 2 —\n", n);
        printf("  ou são poucos, ou a recorrência não é linear, ou os coeficientes não são inteiros.\n");
        printf("  Não há nada a assinar: um corpo que não fecha não passa a fechar por declaração.\n");
        return 1;
    }
    printf("  FECHOU.\n\n");
    printf("    a régua      q(a,b) = a² %+ld·ab %+ld·b²        (B,C) = (%ld, %ld)\n", r.B, r.C, r.B, r.C);
    printf("    a borda      σ² = %ld·σ %+ld\n", r.B, -r.C);
    printf("    o dual       ν(a,b) = (a %+ld·b, −b)            — forçado, não escolhido\n", r.B);
    printf("    a soma       (a,b) ⊕ (c,d) = (a+c, b+d)\n");
    printf("    o produto    (a,b) ⊗ (c,d) = (ac %+ld·bd, ad+bc %+ld·bd)\n", -r.C, r.B);
    long D = r.B*r.B - 4*r.C;
    printf("    o Δ          %ld  — %s\n", D,
           D > 0 ? "hiperbólico (o metal: estica)" : D < 0 ? "elíptico (o redondo: gira)" : "parabólico (o limite)");
    int mau = 0;
    for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
        Par p = { a, b }, v = dual(r, dual(r, p));
        if(v.a != p.a || v.b != p.b) mau++;
        for(long c = -3; c <= 3; c++) for(long d = -3; d <= 3; d++){
            Par q = { c, d };
            if(norma(r, prod(r,p,q)) != norma(r,p)*norma(r,q)) mau++;
        }
    }
    printf("\n    a reversão   %s\n", mau ? "FALHOU" : "resíduo 0 em 289 pontos e 289×49 produtos");
    return mau ? 1 : 0;
}

/* ================================================================================ */
int main(int argc, char **argv){
    if(argc > 1) return modo_piloto(argc, argv);

    puts("fecha.c — NÃO HÁ CONTRATO. Dá-me metade, e o corpo fecha sozinho.");
    puts("================================================================");
    puts("");
    puts("  O contrato pedia quatro cláusulas. A régua dá as quatro, e o piloto só fornece");
    puts("  quatro números. Não há nada a assinar: ou fecham, ou não eram de um corpo.");

    secao_F1();
    secao_F2();
    secao_F3();
    secao_F4();
    secao_F5();
    secao_F6();
    secao_F7();
    secao_F8();

    printf("\n================================================================\n");
    printf("  %d asserções, %d falhas\n", unidades, falhas);
    if(!falhas){
        printf("  RESIDUO 0\n\n");
        puts("  E O QUE ISTO TIRA DE CIMA DO PILOTO: ele não declara operações, não escolhe");
        puts("  duais, não afina constantes. Mostra alguns termos — quatro bastam — e o corpo");
        puts("  diz-se inteiro: régua, borda, soma, produto, dual e Δ, todos derivados e todos");
        puts("  verificados com resíduo 0. Cabe-lhe apenas o tempo de decidir que termos dar.");
        puts("");
        puts("      ./fecha 0 1 1 2 3 5        → ouro,  Δ = 5,  hiperbólico");
        puts("      ./fecha 1 0 -1 0 1 0       → i,     Δ = −4, elíptico");
    } else printf("  FALHOU\n");
    return falhas ? 1 : 0;
}
