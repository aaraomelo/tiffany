/* cone_espiral.c — O CONE E A ESPIRAL: Σ∘Π = Id, mas Π∘Σ NÃO — e é aí que está a informação.
 *
 * O Aarão pôs no `eval.txt` a secção formal do par, e ela define
 *
 *      Π : ℝ → ℕ^ℕ        Π(x) = (a₀, a₁, a₂, …)      o CONE, extração discreta
 *      Σ : ℕ^ℕ → ℝ        Σ(a) = a₀ + 1/(a₁ + 1/(a₂ + …))   a ESPIRAL, reconstrução contínua
 *      Σ∘Π = Id_ℝ         "sempre que a sequência representa uma expansão válida"
 *
 * Este ficheiro não transcreve isso --- mede-o, e mede sobretudo a frase entre aspas, que é onde
 * está o conteúdo. Porque **Σ∘Π = Id e Π∘Σ ≠ Id**, e a assimetria não é um defeito da construção:
 * é o que faz do cone um cone.
 *
 * O QUE SE APURA, e nenhuma das três coisas estava dita:
 *
 *   1. Σ∘Π = Id vale, e vale ao nível do epsilon --- a espiral recompõe o real inteiro.
 *   2. Π∘Σ ≠ Id: há sequências que a espiral aceita e que o cone nunca produz. Duas famílias,
 *      e as duas mediram-se: as que têm aᵢ = 0 no meio, e as que TERMINAM EM 1.
 *   3. E a segunda é exata e conta-se: todo racional tem **exatamente duas** representações,
 *      [a₀; …, aₙ] e [a₀; …, aₙ−1, 1]. Logo Σ não é injetiva, e o par é uma RETRAÇÃO e não uma
 *      bijeção. O cone escolhe uma das duas; a espiral aceita ambas.
 *
 * E ISTO SEPARA DOIS TIPOS DE DUAL, que o projeto vinha a tratar como um só:
 *
 *      INVOLUÇÃO   ν∘ν = id      o mesmo espaço, ida e volta simétricas   (operador_dual.c)
 *      RETRAÇÃO    Σ∘Π = Id      espaços DIFERENTES, e só um lado fecha   (aqui)
 *
 * O ν do `operador_dual.c` é do primeiro tipo; o cone/espiral é do segundo. Confundi-los faria
 * esperar que Π∘Σ fechasse --- e não fecha, por uma razão que se conta em números.
 *
 *   §E1  Σ∘Π = Id: a espiral recompõe o real, e o resíduo é epsilon
 *   §E2  Π∘Σ ≠ Id: as sequências que a espiral aceita e o cone nunca produz
 *   §E3  a ambiguidade dos racionais: EXATAMENTE duas representações, contadas
 *   §E4  logo o par é uma RETRAÇÃO, e não uma involução — os dois tipos de dual
 *   §E5  e o que o cone perde por passo é o que a régua ganha: a informação não some
 *
 * E UM DEFEITO MEU QUE A MEDIÇÃO APANHOU, e que é uma lição do próprio projeto: a primeira versão
 * fazia Π em vírgula flutuante, e ele devolveu [1;1,1,1,1] para 8/5 quando o certo é [1;1,1,2] ---
 * porque 1/0.5000000000000002 dá 1.9999999999999991 e o `floor` disso é 1. *O cone é instável
 * perto dos racionais em double.* Em inteiros não há instabilidade nenhuma: é o algoritmo de
 * Euclides, e sai exato. "Ele sai inteiro, usa a régua infinita" não era um modo de dizer.
 *
 *   cc -O2 -std=c99 -I. cone_espiral.c -lm -o cone_espiral && ./cone_espiral
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

/* Π: o CONE — extrai a sequência. Cada passo remove a parte inteira e inverte o resto. */
static int Pi(double x, int *a, int n){
    int k = 0;
    for(; k < n; k++){
        double f = floor(x);
        /* o guard é 2e9 e não 1e15: o cast é para int, e int estoura em 2,1e9. Com 1e15 um
         * racional cujo resto vira ruído de vírgula flutuante gera f enorme e o cast dá
         * −2147483648 — que foi exatamente o que apareceu em 355/113 na primeira versão. */
        if(f > 2e9) break;
        a[k] = (int)f; x -= f;
        if(x == 0.0){ k++; break; }
        x = 1.0/x;
    }
    return k;
}
/* Π EXATO, em inteiros: para um racional p/q não há vírgula flutuante nenhuma, e por isso não
 * há instabilidade. A primeira versão deste ficheiro usava só o Π em double, e ele devolveu
 * [1;1,1,1,1] para 8/5 quando o certo é [1;1,1,2] — porque 1/0.5000000000000002 dá
 * 1.9999999999999991 e o floor disso é 1. É o próprio projeto a dizê-lo: a régua sai INTEIRA. */
static int Pi_exato(long p, long q, int *a, int n){
    int k = 0;
    while(q != 0 && k < n){
        long f = p / q;                 /* divisão inteira, com p,q > 0 */
        a[k++] = (int)f;
        long r = p - f*q;
        p = q; q = r;
    }
    return k;
}
/* Σ EXATO: devolve o racional p/q reconstruído, sem vírgula flutuante */
static void Sigma_exato(const int *a, int n, long *p, long *q){
    long P = 1, Q = 0;                  /* de trás para a frente: [aₙ] = aₙ/1 */
    for(int k = n-1; k >= 0; k--){ long np = a[k]*P + Q; Q = P; P = np; }
    *p = P; *q = Q;
}

/* Σ: a ESPIRAL — recompõe o real, de trás para a frente. */
static double Sigma(const int *a, int n){
    if(n <= 0) return 0.0;
    double v = a[n-1];
    for(int k = n-2; k >= 0; k--) v = a[k] + 1.0/v;
    return v;
}
/* o cone é canónico: aᵢ ≥ 1 para i ≥ 1, e o último ≠ 1 (quando há mais de um) */
static int canonica(const int *a, int n){
    for(int i = 1; i < n; i++) if(a[i] < 1) return 0;
    if(n > 1 && a[n-1] == 1) return 0;
    return 1;
}

int main(void){
printf("\n=== O CONE E A ESPIRAL: Σ∘Π = Id, mas Π∘Σ NÃO ============================\n");
printf("    O eval.txt define o par e diz \"sempre que a sequência é válida\". É essa\n");
printf("    frase que este ficheiro mede — porque é aí que está o conteúdo.\n");

printf("\n§E1  Σ∘Π = Id: a espiral recompõe o real, e o resíduo é epsilon.\n\n");
{
    printf("      x                     Π(x) = (a₀,a₁,…)              Σ∘Π(x)          resíduo\n");
    double xs[] = { (1+sqrt(5.0))/2, sqrt(2.0), 3.14159265358979323846, exp(1.0), 355.0/113.0, 0.618033988749895 };
    const char *nm[] = { "φ = (1+√5)/2", "√2", "π", "e", "355/113", "1/φ" };
    double pior = 0;
    for(int i = 0; i < 6; i++){
        int a[24], n = Pi(xs[i], a, 24);
        double v = Sigma(a, n), r = fabs(v - xs[i])/fabs(xs[i]);
        if(r > pior) pior = r;
        printf("      %-21s [", nm[i]);
        for(int k = 0; k < n && k < 6; k++) printf("%d%s", a[k], k < n-1 && k < 5 ? "; " : "");
        printf("%s]", n > 6 ? "…" : "");
        for(int s = 0; s < (int)(30 - 6*2); s++) putchar(' ');
        printf("%-15.10f %.2e\n", v, r);
    }
    printf("\n      pior resíduo relativo: %.3e\n\n", pior);
    ok("Σ∘Π = Id — a espiral recompõe o real que o cone extraiu", pior < 1e-9);
    printf("      Uma projeção sozinha PERDE (fica só um inteiro); a composição de todas não\n");
    printf("      perde nada. É o que o eval.txt diz, e é o que faz da régua uma coordenada.\n\n");
    printf("      E repare-se QUEM tem o pior resíduo: φ e 1/φ, com 1e-10 contra o zero exato\n");
    printf("      dos outros. Não é defeito — é o teorema de Hurwitz a aparecer. φ = [1;1,1,…]\n");
    printf("      tem todos os quocientes no mínimo possível, logo os denominadores crescem o\n");
    printf("      mais devagar que podem: é o número MAIS IRRACIONAL, o que a régua comprime\n");
    printf("      pior. O limiar aqui é 1e-9 por causa dele, e não por folga.\n");
}

printf("\n§E2  Π∘Σ ≠ Id: as sequências que a espiral aceita e o cone NUNCA produz.\n\n");
{
    /* A frase "sempre que a sequencia e' valida" tem conteudo: ha' sequencias que Sigma aceita
     * e que Pi nao devolve nunca. Mede-se dando-as a Sigma, voltando com Pi, e comparando. */
    printf("      sequência dada          Σ(a)        Π(Σ(a))          canónica?  voltou?\n");
    struct { int a[6], n; const char *porque; } casos[] = {
        {{3,7,15,1},        4, "canónica"},
        {{3,7,15,0,1},      5, "tem um ZERO no meio"},
        {{3,7,16},          3, "canónica"},
        {{3,7,15,1,1},      5, "TERMINA EM 1"},
        {{1,1,1,1,1},       5, "canónica (φ truncado)"},
        {{2,0,3},           3, "tem um ZERO no meio"},
    };
    int naoVolta = 0, naoCanon = 0;
    for(int i = 0; i < 6; i++){
        long P, Q; Sigma_exato(casos[i].a, casos[i].n, &P, &Q);
        double v = (double)P/Q;
        int b[24], m = Pi_exato(P, Q, b, 24);   /* EXATO: sem vírgula flutuante */
        int igual = (m == casos[i].n);
        if(igual) for(int k = 0; k < m; k++) if(b[k] != casos[i].a[k]) igual = 0;
        int can = canonica(casos[i].a, casos[i].n);
        if(!can) naoCanon++;
        if(!igual) naoVolta++;
        printf("      [");
        for(int k = 0; k < casos[i].n; k++) printf("%d%s", casos[i].a[k], k<casos[i].n-1?";":"");
        printf("]");
        for(int s = 0; s < 22 - 2*casos[i].n - 2; s++) putchar(' ');
        printf("%-11.6f [", v);
        for(int k = 0; k < m && k < 5; k++) printf("%d%s", b[k], k<m-1&&k<4?";":"");
        printf("]");
        for(int s = 0; s < 17 - 2*(m<5?m:5) - 2; s++) putchar(' ');
        printf("%-10s %s\n", can ? "sim" : "NÃO", igual ? "sim" : "NÃO");
    }
    printf("\n      %d de 6 não voltaram; %d não eram canónicas\n\n", naoVolta, naoCanon);
    ok("há sequências que Σ aceita e Π nunca produz — Π∘Σ NÃO é a identidade", naoVolta > 0);
    ok("e são exatamente as não-canónicas: zero no meio, ou terminar em 1", naoVolta == naoCanon);
    printf("      Logo o domínio da espiral é MAIOR que a imagem do cone. O cone escolhe uma\n");
    printf("      representação; a espiral aceita as que ele recusa.\n");
}

printf("\n§E3  A AMBIGUIDADE dos racionais: exatamente DUAS representações, contadas.\n\n");
{
    /* O caso interessante do §E2: [a0;…,an] e [a0;…,an−1,1] dao O MESMO numero. Nao e' um
     * acidente de dois exemplos — mede-se por varredura sobre todos os racionais p/q com q
     * pequeno, contando quantos tem a segunda forma e verificando que ela da' o mesmo valor. */
    int comDupla = 0, total = 0, mau = 0;
    printf("      p/q      Π(p/q)            a forma alternativa      dão o mesmo?\n");
    for(int q = 2; q <= 40; q++)
    for(int p = 1; p < q; p++){
        if(p % 2 == 0 && q % 2 == 0) continue;              /* evita repetir a mesma fração */
        double x = (double)p/q;
        int a[24], n = Pi_exato(p, q, a, 24);   /* EXATO */
        if(n < 2) continue;
        total++;
        /* a forma alternativa: baixar o último em 1 e acrescentar um 1 */
        int b[25]; memcpy(b, a, n*sizeof(int));
        b[n-1] -= 1; b[n] = 1;
        int m = n + 1;
        if(b[n-1] < 1 && n > 1) continue;                    /* não existe alternativa aqui */
        comDupla++;
        long P2, Q2; Sigma_exato(b, m, &P2, &Q2);
        /* a comparação também é exata: p/q == P2/Q2  <=>  p·Q2 == P2·q */
        if((long)p*Q2 != P2*(long)q) mau++;
        double v = (double)P2/Q2; (void)v;
        if(total <= 3){
            printf("      %d/%-6d [", p, q);
            for(int k = 0; k < n && k < 4; k++) printf("%d%s", a[k], k<n-1&&k<3?";":"");
            printf("]");
            for(int s = 0; s < 18 - 2*(n<4?n:4) - 2; s++) putchar(' ');
            printf("[");
            for(int k = 0; k < m && k < 5; k++) printf("%d%s", b[k], k<m-1&&k<4?";":"");
            printf("]");
            for(int s = 0; s < 25 - 2*(m<5?m:5) - 2; s++) putchar(' ');
            printf("%s\n", fabs(v-x) == 0.0 ? "sim" : "NÃO");
        }
    }
    printf("      …\n\n      %d racionais varridos, %d com forma dupla, %d discordâncias\n\n",
           total, comDupla, mau);
    ok("a forma alternativa dá EXATAMENTE o mesmo número — Σ não é injetiva", mau == 0 && comDupla > 100);
    printf("      É por isto que Π∘Σ não pode ser a identidade: duas sequências distintas caem\n");
    printf("      no mesmo real, e o cone só devolve uma delas. A perda não está na precisão —\n");
    printf("      está na ESCOLHA, e a escolha é o que torna a representação canónica.\n");
}

printf("\n§E4  Logo o par é uma RETRAÇÃO, e não uma involução — os dois tipos de dual.\n\n");
{
    /* A distincao que este ficheiro existe para fixar. O projeto tratava "dual" como uma coisa
     * so'. Sao duas, e confundi-las faz esperar que Pi∘Sigma feche. */
    printf("      tipo         a lei             espaços      exemplo neste projeto\n");
    printf("      INVOLUÇÃO    ν∘ν = id          o MESMO      ν(a,b) = (a+Bb, −b)\n");
    printf("      RETRAÇÃO     Σ∘Π = Id          DIFERENTES   o cone e a espiral\n\n");
    /* mede-se a diferenca: aplicar duas vezes o mesmo operador so' faz sentido no primeiro */
    int mau = 0;
    {   /* involução: ν duas vezes devolve */
        for(long a = -5; a <= 5; a++) for(long b = -5; b <= 5; b++){
            long a2 = a + 2*b, b2 = -b;          /* ν com B = 2 */
            long a3 = a2 + 2*b2, b3 = -b2;
            if(a3 != a || b3 != b) mau++;
        }
    }
    ok("na involução, o MESMO operador duas vezes devolve", mau == 0);
    {   /* retração: Π e Σ são operadores DIFERENTES, e só uma ordem fecha */
        double x = sqrt(3.0);
        int a[20], n = Pi(x, a, 20);
        double volta = Sigma(a, n);
        int b[20]; int m = Pi(volta, b, 20);
        int ida = (fabs(volta - x) < 1e-9);
        /* e a outra ordem, com uma sequência não-canónica, não fecha (§E2) */
        int c[4] = {1,7,0,2};
        double v2 = Sigma(c, 4); int d[20], k2 = Pi(v2, d, 20);
        int outra = (k2 == 4 && d[0]==c[0] && d[1]==c[1] && d[2]==c[2] && d[3]==c[3]);
        printf("      Σ∘Π em √3:        volta? %s\n", ida ? "sim" : "não");
        printf("      Π∘Σ em [1;7,0,2]: volta? %s\n\n", outra ? "sim" : "NÃO");
        ok("Σ∘Π fecha e Π∘Σ não — é retração, e a assimetria é o cone", ida && !outra);
        (void)m;
    }
    printf("      Confundir os dois faria esperar simetria onde não há. O ν do operador_dual.c\n");
    printf("      é do primeiro tipo; este par é do segundo — e é por ser do segundo que a\n");
    printf("      régua COMPRIME: se fosse simétrico, não comprimia nada.\n");
}

printf("\n§E5  E o que o cone perde por passo é o que a régua ganha.\n\n");
{
    /* O fecho, e o que liga isto ao resto do projeto. Uma projecao isolada deita fora tudo
     * menos um inteiro. Mede-se quanto: o erro de truncar em k termos cai como 1/q_k², onde
     * q_k e' o denominador do convergente — logo cada termo novo paga-se sozinho. */
    printf("      termos k   convergente     erro |φ − p/q|    1/q²          erro·q²\n");
    double x = (1+sqrt(5.0))/2;
    int a[20], n = Pi(x, a, 20);
    long p0=1,q0=0,p1=a[0],q1=1;
    int mau = 0, casos = 0;
    for(int k = 1; k < n && k <= 8; k++){
        long pn = a[k]*p1+p0, qn = a[k]*q1+q0;
        p0=p1;q0=q1;p1=pn;q1=qn;
        double e = fabs(x - (double)pn/qn), b = 1.0/((double)qn*qn);
        double r = e*(double)qn*qn;
        if(!(r < 1.0)) mau++;                 /* o teorema: |x − p/q| < 1/q² */
        casos++;
        printf("      %-10d %ld/%-13ld %-17.3e %-13.3e %.4f\n", k+1, pn, qn, e, b, r);
    }
    printf("\n");
    ok("cada termo do cone paga-se: o erro cai abaixo de 1/q², sempre", mau == 0 && casos == 8);
    printf("      Portanto a informação não some entre as projeções: passa do contínuo para o\n");
    printf("      inteiro, e o inteiro é exato. É o que o telomero.c já dizia por outro lado —\n");
    printf("      o resto encurta, tem fim, e o que fica identifica.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    Σ∘Π = Id e Π∘Σ ≠ Id, e a assimetria é o conteúdo: o domínio da espiral é\n");
printf("    maior que a imagem do cone, porque todo racional tem DUAS representações e o\n");
printf("    cone escolhe uma. Logo o par é uma RETRAÇÃO e não uma involução — e é por\n");
printf("    isso que ele comprime. Um dual simétrico não comprimiria nada.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
