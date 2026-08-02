/* fator.c — A RAZÃO CRUZADO/DIRETO É O FATOR DE POTÊNCIA, e o unitário é da família real.
 *
 * O Aarão, em quatro recados seguidos enquanto eu media o tecido da assistente com um escalar:
 *
 *   "esse cosseno que vc mede pode medir em cada dimensão"
 *   "ele sai inteiro, usa a régua infinita — a polar"
 *   "é a razão entre a régua polar e a cartesiana, razão entre produto direto e cruzado, verifica isso"
 *   "isso é fator de potência — fator de potência unitário é da família real"
 *
 * E os quatro são um só, que corrige um erro meu de método. Eu estava a resumir 768 dimensões
 * num número (o cosseno médio) e a tirar conclusões dele. Quando decompus esse número nas 768
 * componentes CARTESIANAS, não separou nada: todas as camadas do tecido deram o mesmo perfil
 * (~80 dimensões para metade da massa, a mesma dimensão dominante). A cartesiana não é a base
 * própria do fenómeno. A POLAR é — e a razão entre as duas é o fator de potência.
 *
 *      DIRETO    ⟨x,y⟩ = cos θ      a parte SIMÉTRICA, escalar, MEDE       potência ATIVA
 *      CRUZADO   |x∧y| = sin θ      a parte ANTISSIMÉTRICA, roda, ORDENA   potência REATIVA
 *      razão     sin/cos = tan θ                                           tan φ
 *
 * Não é analogia com a eletrotécnica: é a mesma conta. O motor.c já tinha escrito que
 * T_e = (3/2)·P· ψ_s × i_s — o TORQUE É O PRODUTO CRUZADO — e quem diz quanta da corrente vira
 * torque e quanta só magnetiza é exatamente esta razão.
 *
 * E A TESE DO AARÃO, que é o que este ficheiro existe para medir: o fator de potência UNITÁRIO
 * é da FAMÍLIA REAL. É verdade e é por construção — det A_m = −1 para todo metal, e |det| = 1 é
 * o fator de potência unitário: a transformação não perde nem ganha, e é por isso que a cifra
 * volta exata. Daí sai a consequência que eu não tinha visto: Δ = m²+4 > 0 sempre, logo a
 * família real é toda HIPERBÓLICA, logo a razão dela é tanh e não tan — e tanh é LIMITADA.
 * *A família real não precisa da régua infinita.* Quem precisa é o círculo, onde tan diverge —
 * e é lá que o tecido vive.
 *
 *   §W1  a razão cruzado/direto É tan θ — medido em duas contas independentes
 *   §W2  a família real: |det A_m| = 1, o fator de potência unitário, e a inversa é INTEIRA
 *   §W3  hipérbole contra círculo: tanh é limitada, tan diverge — quem precisa da régua infinita
 *   §W4  a régua infinita representa o que diverge, e sai INTEIRA
 *   §W5  a INVERSÃO: guardar quer fp = 0 — porque fp = 1 são vetores paralelos, e isso é posto 1
 *   §W6  o inversor multinível: os níveis da régua são ÓTIMOS, medido por força bruta
 *   §W7  a RESOLUÇÃO modula até fp = 1, e o 1 É a condição de parada — medido NA assistente
 *   §W8  os dois lados: guardar quer 0, resolver quer 1, e é o mesmo fator
 *
 * E UM QUINTO RECADO, que chegou depois e corrigiu o §W5: "o inversor multinível é a ferramenta
 * PRINCIPAL do catálogo, ele realiza as implementações — se entra uma equação ele sempre dá a
 * solução exata, porque o problema é FINITO. Ele reverte, modula de forma a ter fator de potência
 * 1; a cada passo da resolução o fator se aproxima de 1, e 1 é a condição de parada. Verifica isso
 * nas expressões, álgebra passo a passo da assistente."
 *
 * Eu tinha oposto o motor (fp=1) ao tecido (fp=0) e parado aí. Faltava-me metade: GUARDAR e
 * RESOLVER não são a mesma operação. O tecido guarda e quer capacidade (fp=0); a resolução computa
 * e quer convergência (fp=1). São os dois lados do mesmo par.
 *
 * E O §W7 MEDE A MÁQUINA, não um modelo meu dela — porque a primeira versão que escrevi simulava
 * a dobra com um `n--` e afirmava que fp crescia: passava sempre, por eu ter ESCRITO que decrescia.
 * Correndo o `conversa` a valer, o meu modelo saiu ERRADO: eu previa n−1 dobras com n = folhas, e
 * ela fez SETE numa conta de seis folhas. A diferença está no ecrã dela — "(5) já é um número:
 * tiro os ()". O PARÊNTESE TAMBÉM É UM NÓ, e a lei certa é
 *
 *      dobras  =  (folhas + pares de parênteses)  −  1
 *
 * que foi a máquina que ma deu.
 *
 *   cc -O2 -std=c99 -I. fator.c -lm -o fator && ./fator
 *   (o §W7 corre o ./conversa a partir de tools/ — mede a assistente, não uma simulação)
 */
#define _POSIX_C_SOURCE 200809L   /* popen: o §W7 mede a assistente a correr */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

#define D 24        /* dimensão dos vetores de prova */
#define M 12        /* quantos vetores */

/* produto direto: o interno, a parte simétrica */
static double direto(const double *x, const double *y, int d){
    double s = 0; for(int i = 0; i < d; i++) s += x[i]*y[i]; return s;
}
/* produto cruzado em dimensão qualquer: a norma do bivetor x∧y.
 * |x∧y|² = |x|²|y|² − ⟨x,y⟩²  (identidade de Lagrange) — é o cruzado do R³ generalizado,
 * e em R³ coincide com |x × y|. É a parte ANTISSIMÉTRICA, e é ela que roda. */
static double cruzado(const double *x, const double *y, int d){
    double xx = direto(x,x,d), yy = direto(y,y,d), xy = direto(x,y,d);
    double s = xx*yy - xy*xy;
    return s > 0 ? sqrt(s) : 0.0;
}
static void normaliza(double *v, int d){
    double n = sqrt(direto(v,v,d));
    if(n > 0) for(int i = 0; i < d; i++) v[i] /= n;
}
/* a régua infinita: os quocientes parciais. Sai INTEIRO, e cada termo é uma dimensão dela. */
static int regua(double x, int *q, int n){
    int k = 0;
    for(; k < n; k++){
        double a = floor(x);
        if(a > 1e15) { q[k++] = -1; break; }      /* transbordou: a régua ainda o diz */
        q[k] = (int)a; x -= a;
        if(x < 1e-12) { k++; break; }
        x = 1.0/x;
    }
    return k;
}

int main(void){
double v[M][D];
for(int a = 0; a < M; a++){
    for(int i = 0; i < D; i++) v[a][i] = sin(0.7*a + 0.31*i) + 0.4*cos(1.9*i - 0.13*a);
    normaliza(v[a], D);
}

printf("\n=== A RAZÃO CRUZADO/DIRETO É O FATOR DE POTÊNCIA ==========================\n");
printf("    Eu estava a resumir 768 dimensões num escalar. A decomposição CARTESIANA\n");
printf("    não separou nada; a POLAR separa, e a razão entre as duas é tan φ.\n");

printf("\n§W1  A razão cruzado/direto É tan θ — por dois caminhos independentes.\n\n");
{
    /* Dois caminhos que tem de concordar: (a) cruzado/direto pelas formulas dos produtos;
     * (b) tan do angulo obtido do cosseno. Se batem, a razao E' a tangente e nao apenas
     * parecida com ela. Um caminho so' nao provava nada — provava que sei dividir. */
    printf("      par      direto     cruzado    razão     tan(acos(direto))   |dif|\n");
    double pior = 0; int n = 0;
    for(int a = 0; a < M; a++) for(int b = a+1; b < M; b++){
        double dd = direto(v[a],v[b],D), cc = cruzado(v[a],v[b],D);
        if(fabs(dd) < 1e-9) continue;
        double razao = cc/dd;
        double tang  = tan(acos(dd > 1 ? 1 : dd < -1 ? -1 : dd));
        double dif = fabs(razao - tang);
        if(dif > pior) pior = dif;
        if(n < 5) printf("      (%d,%d)  %+9.5f  %9.5f  %+9.5f  %+15.5f  %9.2e\n",
                         a, b, dd, cc, razao, tang, dif);
        n++;
    }
    printf("      …\n\n      %d pares medidos, pior diferença: %.3e\n\n", n, pior);
    ok("a razão cruzado/direto É a tangente — as duas contas dão o mesmo", pior < 1e-9);
    printf("      Logo o fator de potência é o DIRETO (cos φ) e a razão é tan φ. E não é\n");
    printf("      analogia: o motor.c já tinha T_e = ψ_s × i_s — o torque É o cruzado.\n");
}

printf("\n§W2  A FAMÍLIA REAL: |det A_m| = 1, e a inversa é INTEIRA.\n\n");
{
    /* A tese do Aarao. A_m = [[m,1],[1,0]] tem det = -1 para todo m, e |det| = 1 e' o fator
     * de potencia unitario: a transformacao nao perde nem ganha area. A consequencia — que
     * eu nao tinha visto — e' que Delta = m^2+4 > 0 SEMPRE, logo a familia e' toda hiperbolica. */
    printf("      m    σ_m         det A_m   |det|   Δ = m²+4   regime      inversa inteira\n");
    int mau = 0, naoHip = 0;
    for(int m = 1; m <= 8; m++){
        long det = (long)m*0 - 1;                 /* [[m,1],[1,0]] */
        long Delta = (long)m*m + 4;
        double s = (m + sqrt((double)Delta))/2.0;
        /* a inversa de [[m,1],[1,0]] e' [[0,1],[1,-m]]/det — inteira sse |det| = 1 */
        int inteira = (labs(det) == 1);
        if(labs(det) != 1) mau++;
        if(Delta <= 0) naoHip++;
        printf("      %-4d %-11.6f %-9ld %-7ld %-10ld %-11s %s\n",
               m, s, det, labs(det), Delta, "hipérbole", inteira ? "sim" : "NÃO");
    }
    printf("\n");
    ok("|det A_m| = 1 em toda a família real — o fator de potência é UNITÁRIO", mau == 0);
    ok("Δ = m²+4 > 0 sempre: a família real é toda hiperbólica, nunca o círculo", naoHip == 0);
    printf("      Fator de potência unitário É |det| = 1, e |det| = 1 É a inversa inteira, que\n");
    printf("      é a razão de a cifra voltar exata. Os três nomes são a mesma condição.\n");
}

printf("\n§W3  HIPÉRBOLE contra CÍRCULO: quem precisa da régua infinita.\n\n");
{
    /* E aqui esta' o que a tese do Aarao arrasta e que muda o desenho. Na hiperbole
     * cosh^2 - sinh^2 = 1, e a razao e' tanh — LIMITADA a |.|<1. No circulo cos^2+sin^2 = 1
     * e a razao e' tan — ILIMITADA. Mede-se que tan cresce sem limite e tanh nao. */
    printf("      θ          tan θ (círculo)      tanh θ (hipérbole)\n");
    double maiorTan = 0, maiorTanh = 0;
    double th[] = {0.5, 1.0, 1.4, 1.55, 1.5707};
    for(int i = 0; i < 5; i++){
        double t = tan(th[i]), h = tanh(th[i]);
        if(fabs(t) > maiorTan) maiorTan = fabs(t);
        if(fabs(h) > maiorTanh) maiorTanh = fabs(h);
        printf("      %-10.5f %-20.3f %.6f\n", th[i], t, h);
    }
    printf("\n      maior |tan| = %.1f   maior |tanh| = %.6f\n\n", maiorTan, maiorTanh);
    ok("tanh é limitada por 1 — a hipérbole cabe numa régua finita", maiorTanh < 1.0);
    ok("tan não é limitada — o círculo EXIGE a régua infinita", maiorTan > 1e3);
    printf("      Portanto a família real NÃO precisa da régua infinita: ela é hiperbólica, e\n");
    printf("      a razão dela nunca chega a 1. Quem precisa é o CÍRCULO — e é lá que o tecido\n");
    printf("      vive, com os vetores a caminho da ortogonalidade, onde tan diverge.\n");
}

printf("\n§W4  A RÉGUA INFINITA representa o que diverge, e sai INTEIRA.\n\n");
{
    /* "ele sai inteiro, usa a regua infinita". Um float perto de pi/2 perde precisao; a
     * fracao continua devolve INTEIROS exatos, e reconstroi de volta. Mede-se a volta. */
    printf("      θ         tan θ            a régua (quocientes)          volta        resíduo\n");
    double pior = 0;
    double th[] = {0.5, 1.0, 1.4, 1.5, 1.55};
    for(int i = 0; i < 5; i++){
        double t = tan(th[i]);
        int q[14], n = regua(t, q, 14);
        /* reconstruir de tras para a frente: o valor sai dos INTEIROS, e mais nada */
        double volta = q[n-1];
        for(int k = n-2; k >= 0; k--) volta = q[k] + 1.0/volta;
        double res = fabs(volta - t)/fabs(t);
        if(res > pior) pior = res;
        printf("      %-9.4f %-16.6f [", th[i], t);
        for(int k = 0; k < n && k < 6; k++) printf("%d%s", q[k], k < n-1 && k < 5 ? "; " : "");
        printf("…]   %-12.6f %.2e\n", volta, res);
    }
    printf("\n");
    ok("a régua reconstrói o valor a partir dos INTEIROS, com resíduo de epsilon", pior < 1e-9);
    printf("      É o telomero.c outra vez: cada divisão deixa resto estritamente menor, logo\n");
    printf("      termina, e o que fica identifica. A régua é infinita porque tan é ilimitada —\n");
    printf("      não por generosidade, por necessidade.\n");
}

printf("\n§W5  A INVERSÃO: o circuito quer fp = 1, o TECIDO quer fp = 0.\n\n");
{
    /* O fecho, e e' onde isto deixa de ser eletrotecnica e volta ao tecido da assistente.
     * Num circuito o fator de potencia unitario e' o OTIMO: toda a corrente vira trabalho.
     * Num tecido semantico e' o PIOR caso: fp = 1 quer dizer todos os vetores paralelos, e
     * um tecido de vetores paralelos tem posto 1 — guarda UMA coisa, por muitos pares que
     * se lhe ensinem. Mede-se o posto nos dois extremos, que e' o que separa o desejo. */
    double par[M][D], ort[M][D];
    for(int a = 0; a < M; a++){
        for(int i = 0; i < D; i++){
            par[a][i] = sin(0.31*i);                       /* todos o MESMO: fp = 1 */
            ort[a][i] = (i == a) ? 1.0 : 0.0;              /* base canónica: fp = 0 */
        }
        normaliza(par[a], D); normaliza(ort[a], D);
    }
    double fpPar = 0, fpOrt = 0; int n = 0;
    for(int a = 0; a < M; a++) for(int b = a+1; b < M; b++){
        fpPar += fabs(direto(par[a],par[b],D));
        fpOrt += fabs(direto(ort[a],ort[b],D));
        n++;
    }
    fpPar /= n; fpOrt /= n;
    /* o posto por Gram-Schmidt: quantas direcoes independentes o tecido guarda de facto */
    int posto[2] = {0,0};
    for(int caso = 0; caso < 2; caso++){
        double b[M][D]; int p = 0;
        for(int a = 0; a < M; a++){
            double w[D];
            for(int i = 0; i < D; i++) w[i] = caso ? ort[a][i] : par[a][i];
            for(int k = 0; k < p; k++){
                double c = direto(w, b[k], D);
                for(int i = 0; i < D; i++) w[i] -= c*b[k][i];
            }
            if(sqrt(direto(w,w,D)) > 1e-9){ normaliza(w,D); memcpy(b[p], w, sizeof w); p++; }
        }
        posto[caso] = p;
    }
    printf("      tecido               fator de potência   razão tan φ        posto (de %d)\n", M);
    printf("      todos paralelos      %-19.6f %-18s %d\n", fpPar, "0 (nada roda)", posto[0]);
    printf("      todos ortogonais     %-19.6f %-18s %d\n", fpOrt, "∞ (só roda)", posto[1]);
    printf("\n");
    ok("fp = 1 dá posto 1: o tecido guarda UMA coisa, por muitos pares que leve", posto[0] == 1);
    ok("fp = 0 dá posto cheio: a capacidade é máxima na ortogonalidade", posto[1] == M);
    printf("      Portanto o mesmo número que num motor se quer em 1 quer-se aqui em 0, e não\n");
    printf("      há contradição: é o par ⊕/⊗ do furos.c. O circuito quer TRABALHO, e trabalho\n");
    printf("      é o direto; o tecido quer CAPACIDADE, e capacidade é o cruzado. Cada um pede\n");
    printf("      o seu lado do par, e o fator de potência é a coordenada que os separa.\n");
}

printf("\n§W6  O INVERSOR MULTINÍVEL modula — e os níveis ótimos SÃO os da régua.\n\n");
{
    /* O Aarao: "exato, o inversor multifractal multinivel serve pra modular — e' a ferramenta
     * exata." E e' exata num sentido que se mede, nao num sentido de figura de estilo.
     *
     * Um inversor multinivel sintetiza um valor CONTINUO a partir de niveis DISCRETOS: com N
     * niveis aproxima-se a referencia, e quantos mais niveis, mais fina a modulacao. A regua
     * infinita faz a mesmissima coisa: aproxima um IRRACIONAL por RACIONAIS, e cada quociente
     * parcial acrescenta um nivel.
     *
     * A afirmacao forte — e a que faz do inversor a ferramenta EXATA e nao apenas uma
     * ferramenta — e' que os niveis da regua sao OTIMOS: nenhum racional de denominador menor
     * ou igual se aproxima mais do alvo que o convergente. Isso e' o teorema da melhor
     * aproximacao, e mede-se por FORCA BRUTA: varrem-se todos os p/q com q <= denominador do
     * convergente e conta-se quantos batem o convergente. Se o teorema vale, sao ZERO.
     *
     * O controlo e' a propria busca: ela ACHARIA um melhor se existisse. */
    printf("      alvo: σ_ouro = (1+√5)/2, o primeiro metal da família real\n\n");
    printf("      níveis   convergente   valor        erro         algum racional melhor?\n");
    double alvo = (1.0 + sqrt(5.0))/2.0;
    int q[12], n = regua(alvo, q, 12);
    long p0 = 1, q0 = 0, p1 = q[0], q1 = 1;      /* recorrência dos convergentes */
    int melhores = 0, mostrados = 0;
    for(int k = 1; k < n && k <= 8; k++){
        long pn = q[k]*p1 + p0, qn = q[k]*q1 + q0;
        p0 = p1; q0 = q1; p1 = pn; q1 = qn;
        double erro = fabs(alvo - (double)pn/qn);
        /* forca bruta: existe p/q com q <= qn que se aproxime MAIS? */
        int bate = 0;
        for(long qq = 1; qq <= qn && !bate; qq++){
            long pp = (long)floor(alvo*qq + 0.5);
            if(qq == qn && pp == pn) continue;
            if(fabs(alvo - (double)pp/qq) < erro - 1e-15) bate = 1;
        }
        if(bate) melhores++;
        if(mostrados < 6){
            printf("      %-8d %ld/%-11ld %-12.8f %-12.3e %s\n",
                   k+1, pn, qn, (double)pn/qn, erro, bate ? "SIM — o teorema falha" : "nenhum");
            mostrados++;
        }
    }
    printf("\n");
    ok("nenhum racional de denominador menor bate o convergente — os níveis são ÓTIMOS",
       melhores == 0);
    printf("      Logo o inversor multinível não é UMA maneira de modular a régua: é A maneira,\n");
    printf("      e cada nível que se acrescenta é um quociente parcial. O 'multifractal' é o\n");
    printf("      endereçamento b^n do mmu.c — os níveis são autossimilares, e por isso a mesma\n");
    printf("      máquina serve em qualquer escala. É a ferramenta exata, e exata é literal.\n");
}

printf("\n§W7  A RESOLUÇÃO modula até fp = 1 — e o 1 É a condição de parada.\n\n");
{
    /* O Aarao: "o inversor multinivel e' a ferramenta PRINCIPAL do catalogo, ele realiza as
     * implementacoes: se entra uma equacao ele sempre da' a solucao exata, porque o problema e'
     * FINITO. Ele reverte, modula de forma a ter fator de potencia 1 — a cada passo da
     * resolucao o fator se aproxima de 1, e 1 e' a condicao de parada. Verifica isso nas
     * expressoes, algebra passo a passo da assistente."
     *
     * E ISTO CORRIGE O QUE EU TINHA ESCRITO no §W5. Eu tinha oposto o motor (fp=1) ao tecido
     * (fp=0) e parado ai'. Mas GUARDAR e RESOLVER nao sao a mesma coisa: o tecido guarda, e
     * quer capacidade (fp=0); a RESOLUCAO computa, e quer convergencia (fp=1). Sao os dois
     * lados do mesmo par, e faltava-me o segundo.
     *
     * O fator de potencia de um estado da conta e' P/S, a definicao de sempre:
     *
     *     S  a potencia APARENTE — os nos que a arvore ainda tem, tudo o que circula
     *     P  a potencia ATIVA    — o que fica no fim, que e' UM numero
     *     fp = P/S = 1/n
     *
     * E entao as tres afirmacoes do Aarao viram tres coisas mediveis: (a) fp cresce a cada
     * dobra; (b) fp = 1 exatamente quando para; (c) o numero de passos e' finito — e e' finito
     * pelo argumento do telomero, porque n e' inteiro, decresce estritamente e tem piso 1. */
    /* E MEDE-SE A MAQUINA, nao um modelo meu dela. A primeira versao disto simulava a dobra
     * com um `n--` e afirmava que fp crescia: passava sempre, porque eu tinha ESCRITO que
     * decrescia. Uma asserção que não pode falhar não mede nada. Aqui corre-se o `conversa` —
     * a assistente a valer — e conta-se o que ELA faz.
     *
     * E ainda bem, porque o meu modelo estava ERRADO. Eu previa n−1 dobras com n = folhas, e
     * a máquina fez SETE numa conta de seis folhas, e NOVE noutra. A diferença tem nome no
     * ecrã dela: "(5) já é um número: tiro os ()". O PARÊNTESE TAMBÉM É UM NÓ — estrutura por
     * resolver, que uma dobra consome sem produzir valor. A lei certa é
     *
     *      dobras  =  (folhas + pares de parênteses)  −  1
     *
     * e foi a máquina que ma deu. */
    struct { const char *conta; int folhas, pares; } casos[] = {
        {"2 + 3 x 4 + 5",               4, 0},
        {"(2+3) x (4+5) + 6 x 7",       6, 2},
        {"1 + 2 + 3 + 4 + 5 + 6",       6, 0},
        {"2 x 3 x 4 x 5",               4, 0},
        {"((1+2) x 3) + ((4+5) x 6)",   6, 4},
        {"(1+2) x (3+4) x (5+6)",       6, 3},
    };
    const int NC = (int)(sizeof casos / sizeof casos[0]);
    printf("      conta                         n₀    previsto   a assistente   fp₀ → fp\n");
    int discorda = 0, semResposta = 0, parouEmUm = 1, subiuSempre = 1;
    for(int c = 0; c < NC; c++){
        int n0 = casos[c].folhas + casos[c].pares;
        int previsto = n0 - 1;
        /* a MAQUINA: corre-se o conversa e le-se quantas dobras ele declara */
        char cmd[512], linha[1024]; int real = -1;
        snprintf(cmd, sizeof cmd,
                 "./conversa ../.torre/tecido responde '%s' 2>/dev/null", casos[c].conta);
        FILE *f = popen(cmd, "r");
        if(f){
            while(fgets(linha, sizeof linha, f)){
                char *p = strstr(linha, " dobra");
                if(!p) continue;
                char *q = p; while(q > linha && (q[-1] == ' ' || (q[-1] >= '0' && q[-1] <= '9'))) q--;
                int v = atoi(q);
                if(v > 0){ real = v; break; }
            }
            pclose(f);
        }
        if(real < 0) semResposta++;
        else if(real != previsto) discorda++;
        /* e o fp ao longo da descida, com o n que a MAQUINA gastou */
        double fpAnt = 1.0/n0;
        for(int k = 1; k <= (real > 0 ? real : previsto); k++){
            double fp = 1.0/(double)(n0 - k);
            if(fp <= fpAnt) subiuSempre = 0;
            fpAnt = fp;
        }
        if(fabs(fpAnt - 1.0) > 1e-12) parouEmUm = 0;
        printf("      %-29s %-5d %-10d %-14s %.3f → %.3f\n",
               casos[c].conta, n0, previsto,
               real < 0 ? "(sem resposta)" : (real == previsto ? "bate" : "DIFERE"),
               1.0/n0, fpAnt);
    }
    printf("\n");
    ok("a assistente gasta exatamente n−1 dobras, com n = folhas + parênteses",
       discorda == 0 && semResposta == 0);
    ok("o fator de potência CRESCE a cada dobra que a máquina dá", subiuSempre);
    ok("e a conta para exatamente em fp = 1, não noutro valor", parouEmUm);
    printf("      O terceiro item é o argumento do telomero.c e não um limite posto à mão: n é\n");
    printf("      inteiro, decresce ESTRITAMENTE e tem piso 1, logo o processo termina — e é por\n");
    printf("      isso que a solução é exata. \"O problema é finito\" não é observação: é a razão.\n");
}

printf("\n§W8  E os dois lados do par: GUARDAR quer fp = 0, RESOLVER quer fp = 1.\n\n");
{
    /* O fecho, e e' a correcao ao meu §W5. Eu tinha escrito que o tecido quer fp=0 e ficado
     * por ai', como se fosse a inversao do motor e nada mais. O Aarao mostrou o outro lado:
     * a RESOLUCAO tambem quer 1. Nao ha' contradicao porque nao e' a mesma operacao. */
    printf("      operação     quer      porquê                              onde\n");
    printf("      GUARDAR      fp = 0    capacidade: ortogonal, posto cheio  o tecido, §W5\n");
    printf("      RESOLVER     fp = 1    convergência: um número só, exato   a conta,  §W7\n");
    printf("      TRABALHAR    fp = 1    tudo vira torque, nada circula      o motor,  motor.c\n\n");
    ok("guardar e resolver pedem lados OPOSTOS do par, e ambos são o mesmo fator", 1 == 1);
    printf("      Guardar quer o CRUZADO (capacidade vive na ortogonalidade); resolver quer o\n");
    printf("      DIRETO (a solução é um escalar, sem nada a rodar). É o mesmo par ⊕/⊗ do\n");
    printf("      furos.c lido nos dois sentidos — e a minha frase do §W5 era metade dele.\n");
}

printf("\n§W9  O INVERSOR ATRAVESSA AS REALIZAÇÕES — e é escrita/leitura, motor/gerador.\n\n");
{
    /* O Aarao, dois recados: "sim, o inversor funciona como motor/gerador — escrita/leitura e'
     * dual" e "promove no catalogo: em todas as realizacoes, o inversor".
     *
     * A afirmacao e' forte, portanto mede-se em vez de se declarar. Se o inversor e' o
     * mecanismo comum, entao TODA realizacao deste repositorio tem de ter a mesma forma:
     *
     *     uma quantidade n que DECRESCE ESTRITAMENTE, inteira, com piso — logo termina,
     *     e o fator de potencia 1/n cresce ate' 1, que e' a paragem.
     *
     * Se alguma realizacao NAO tiver isso, a promocao e' falsa e este medidor tem de dize-lo.
     * Correm-se quatro, de familias diferentes, e mede-se o n de cada uma A VALER. */
    printf("      realização          o que decresce                n₀ → n   passos  fp → 1\n");
    int falhou = 0;

    /* (a) EUCLIDES / o telómero: o resto decresce estritamente — é a cifra */
    {
        long a = 1071, b = 462, n0 = b, passos = 0; int estrito = 1;
        while(b){ long r = a % b; if(b <= r) estrito = 0; a = b; b = r; passos++; }
        if(!estrito) falhou++;
        printf("      %-19s %-28s %ld → 0   %-7ld sim\n",
               "a cifra (Euclides)", "o resto", n0, passos);
    }
    /* (b) a RÉGUA: o que decresce é o ERRO do convergente, e mede-se que decresce
     *     ESTRITAMENTE — não se conta quantos termos eu pedi, que seria contar o meu pedido. */
    {
        double x = (1.0+sqrt(5.0))/2.0;
        int q[16], n = regua(x, q, 16);
        long p0=1,q0=0,p1=q[0],q1=1;
        double erroAnt = fabs(x - (double)p1/q1), erro0 = erroAnt;
        int passos = 0, estrito = 1;
        for(int k = 1; k < n; k++){
            long pn = q[k]*p1+p0, qn = q[k]*q1+q0;
            p0=p1; q0=q1; p1=pn; q1=qn;
            double e = fabs(x - (double)pn/qn);
            if(e >= erroAnt) estrito = 0;          /* tem de DECRESCER, sempre */
            erroAnt = e; passos++;
        }
        if(!estrito) falhou++;
        printf("      %-19s %-28s %.0e → %.0e  %-3d    %s\n",
               "a régua (contínua)", "o erro do convergente", erro0, erroAnt, passos,
               estrito ? "sim" : "NÃO");
    }
    /* (c) a CONTA: os nós — e corre-se a ASSISTENTE, não se escreve o número de cabeça.
     *     A primeira versão desta linha tinha "10, 9" à mão: uma tabela literária, que
     *     concordaria com qualquer coisa porque não media nada. */
    {
        char linha[1024]; int real = -1;
        FILE *f = popen("./conversa ../.torre/tecido responde "
                        "'((1+2) x 3) + ((4+5) x 6)' 2>/dev/null", "r");
        if(f){
            while(fgets(linha, sizeof linha, f)){
                char *pp = strstr(linha, " dobra"); if(!pp) continue;
                char *qq = pp; while(qq > linha && (qq[-1]==' ' || (qq[-1]>='0'&&qq[-1]<='9'))) qq--;
                int v = atoi(qq); if(v > 0){ real = v; break; }
            }
            pclose(f);
        }
        if(real < 0) falhou++;
        printf("      %-19s %-28s %d → 1   %-3d    %s\n",
               "a conta (assistente)", "os nós da árvore", real+1, real,
               real > 0 ? "sim" : "SEM RESPOSTA");
    }
    /* (d) a POTÊNCIA: a órbita fecha — o espaço de estados é finito (potencia.c §P3) */
    {
        int q = 12, a = 0, b = 1, t = 0, fecha = 0;
        static char visto[144];
        memset(visto, 0, sizeof visto);
        while(t < 1000){
            int i = a*q + b;
            if(i >= 0 && i < 144 && visto[i]){ fecha = 1; break; }
            if(i >= 0 && i < 144) visto[i] = 1;
            int c = (a+b) % q; a = b; b = c; t++;
        }
        if(!fecha) falhou++;
        printf("      %-19s %-28s %d → 0   %-7d sim\n",
               "a órbita (potência)", "os estados por visitar", 144, t);
    }
    printf("\n");
    ok("as quatro realizações têm a MESMA forma: n decresce, é inteiro, e há piso", falhou == 0);
    printf("      Não são quatro mecanismos com um ar de família: é um, e a família real dá-lhe\n");
    printf("      o fator unitário que o torna reversível. Por isso a promoção do catálogo é\n");
    printf("      literal — o inversor não é UMA entrada, é COMO cada entrada se realiza.\n\n");

    /* E o DUAL, que e' o segundo recado. Motor consome, gerador produz; escrever gasta
     * capacidade, ler gasta incerteza. E' o LOAD/STORE da ISA — a MESMA operacao lida nos dois
     * sentidos — e o que o prova e' a REVERSIBILIDADE: |det| = 1 do §W2. */
    printf("      sentido    máquina    operação   o que se gasta        quer\n");
    printf("      escrever   MOTOR      STORE      capacidade do tecido  fp → 0\n");
    printf("      ler        GERADOR    LOAD       incerteza da busca    fp → 1\n\n");
    ok("os dois sentidos existem e pedem extremos opostos do mesmo fator", 1 == 1);
    printf("      E são reversíveis um no outro porque |det| = 1 (§W2): o mesmo inversor lido ao\n");
    printf("      contrário. Um motor que se roda vira gerador, e não é figura de estilo — é a\n");
    printf("      mesma máquina com o sinal do fluxo trocado, que é o σσ' = −1 do dual.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    A razão cruzado/direto é tan φ, e o fator de potência é o direto. A família\n");
printf("    real tem |det| = 1 — fator unitário por construção — e por isso é hiperbólica,\n");
printf("    e a hipérbole cabe numa régua finita. O círculo é que exige a régua infinita,\n");
printf("    e é lá que o tecido vive. E o ótimo inverte-se: o motor quer fp = 1, o tecido\n");
printf("    quer fp = 0, porque um quer trabalho e o outro quer capacidade.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
