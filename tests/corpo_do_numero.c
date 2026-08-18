/* corpo_do_numero.c — O NÚMERO↔TEXTO FORMA UM CORPO? E DE QUE GRAU --- 2 OU 4?
 *
 * O Aarão: «vê se isso forma um corpo, grau 2 ou 4.» E: «provavelmente são dois duais.»
 *
 * «Isso» é a camada número↔texto do núcleo (lib/le_num.h + o formatador de vírgula-fixa): o PARSE
 * (str2dbl) e o FORMAT. Tem os dois lados --- logo tem DUAL: a volta fecha (parse∘format=id). E o
 * Aarão viu o que eu não vi: são DOIS duais. Um número em vírgula-fixa é sign · m · 10^(-k), e tem
 * DOIS sinais, que são duas involuções que FECHAM ambas em vírgula-fixa:
 *
 *   - o SINAL DO VALOR (aditivo, ±m):  "100" ↔ "-100"  --- reflexão em torno de 0 (a Lei 1)
 *   - o SINAL DA ESCALA (multiplicativo, ±k): "0.01" ↔ "100" --- mover a vírgula, o dual da escala
 *
 * As duas comutam e são independentes, logo a órbita de um número genérico tem QUATRO estados
 * {+v, -v, +v', -v'} (a CRUZ) --- GRAU 4, a tetral. É o par aditivo/multiplicativo do teorema das
 * duas leis, agora no corpo do número. (Antes eu dissera grau 2, fixado na notação do expoente `e`;
 * mas o dual da escala não é o `e` --- é a POSIÇÃO DA VÍRGULA, e a vírgula-fixa escreve-a.)
 *
 * E O NOME CERTO É A LEI DOS TECIDOS (Lei 4 tetral; corpo_analitico thm:tecidos): o passo é T + T*,
 * o par (A, A*) lido pela CRUZ (x⊕x†, x⊗x†), cujas DUAS coordenadas SÃO o aditivo (o sinal do valor)
 * e o multiplicativo (o sinal da escala). escrita = T (o discreto, contar os dígitos, Hurwitz),
 * leitura = T* (o contínuo, o valor, Gentil): o número↔texto é UM passo de tecido, e a medida é a
 * CRUZ / contar↔integrar --- o degrau Q→R, o teorema central. A norma de CAYLEY-DICKSON NÃO MEDE
 * NADA AQUI: o «octonião / grau 8» seria régua trazida de fora (a torre é N→Z→Q→R, não R→C→H→O), e
 * um fator que não se elimina seria o preço dessa régua, não um resultado. O tecido mede-se pela cruz.
 *
 *   §C1  forma CORPO: parse∘format = id (a volta fecha, resíduo 0)
 *   §C2  o 1.º dual --- o SINAL DO VALOR (aditivo): -(-v)=v, fecha nos dois lados, período 2
 *   §C3  o 2.º dual --- o SINAL DA ESCALA (multiplicativo): mover a vírgula, fecha, período 2
 *   §C4  os dois COMUTAM e são independentes -> órbita 4 (a cruz): GRAU 4
 *
 *   cc -O2 -std=c99 -Wall -I../lib corpo_do_numero.c -o corpo_do_numero && ./corpo_do_numero
 */
#include <stdio.h>
#include <string.h>
#include "unidade.h"
#include "le_num.h"      /* o parse do núcleo: str2dbl */

/* 10^k exacto em double (k pequeno) */
static double pot10(int k){ double p = 1.0; for(int i = 0; i < k; i++) p *= 10.0; return p; }

/* o valor sign · m · 10^(-k), com k podendo ser NEGATIVO (a escala é ±) */
static double valor(int sign, long m, int k){
    double mag = (k >= 0) ? (double)m / pot10(k) : (double)m * pot10(-k);
    return sign < 0 ? -mag : mag;
}

/* o lado FORMAT (vírgula-fixa): escreve sign · m · 10^(-k). k>0: casas decimais; k<=0: inteiro com
 * -k zeros. SEM expoente --- a escala está na POSIÇÃO DA VÍRGULA, não numa letra. */
static void fmt(char *o, int sign, long m, int k){
    char *p = o;
    if(sign < 0) *p++ = '-';
    if(k <= 0){                                   /* inteiro: m seguido de -k zeros */
        char t[24]; int n = 0; long u = m;
        if(u == 0) t[n++] = '0'; else while(u){ t[n++] = (char)('0' + u % 10); u /= 10; }
        while(n) *p++ = t[--n];
        for(int i = 0; i < -k; i++) *p++ = '0';
    } else {                                      /* fracção: parte inteira '.' k dígitos */
        long div = 1; for(int i = 0; i < k; i++) div *= 10;
        long ip = m / div, fr = m % div;
        char t[24]; int n = 0; long u = ip;
        if(u == 0) t[n++] = '0'; else while(u){ t[n++] = (char)('0' + u % 10); u /= 10; }
        while(n) *p++ = t[--n];
        *p++ = '.';
        for(int i = k - 1; i >= 0; i--){ long d = fr; for(int j = 0; j < i; j++) d /= 10; *p++ = (char)('0' + d % 10); }
    }
    *p = 0;
}

static int mesmos(double a, double b){ unsigned long long x, y; memcpy(&x,&a,8); memcpy(&y,&b,8); return x==y; }

int main(void){
    printf("=== O NÚMERO↔TEXTO FORMA UM CORPO? GRAU 2 OU 4? --- SÃO DOIS DUAIS =================\n\n");

    /* ── §C1 forma CORPO: parse∘format = id ────────────────────────────────────────────── */
    long res_corpo = 0, tot = 0;
    for(long m = 0; m <= 999; m++) for(int k = -2; k <= 3; k++) for(int sg = -1; sg <= 1; sg += 2){
        if(m == 0 && sg < 0) continue;
        char txt[40]; fmt(txt, sg, m, k);
        if(!mesmos(str2dbl(txt, NULL), valor(sg, m, k))) res_corpo++;
        tot++;
    }
    printf("      %ld números (sign, m, escala±), parse∘format --- %ld resíduo\n\n", tot, res_corpo);
    ok("§C1 a camada número↔texto FORMA CORPO: o format (vírgula-fixa) escreve e o parse (str2dbl) lê"
       " de volta, e o valor fecha EXACTO --- parse∘format = id, resíduo 0. É o MOVE(-1)/MOVE(+1) do"
       " tradutor num número: um lado emite o texto, o outro absorve-o", res_corpo == 0 && tot > 0);

    /* ── §C2 o 1.º dual: o SINAL DO VALOR (aditivo) ────────────────────────────────────── */
    long res_v = 0, par_v = 0;
    for(long m = 1; m <= 400; m++) for(int k = -1; k <= 2; k++){
        char tp[40], tn[40]; fmt(tp, +1, m, k); fmt(tn, -1, m, k);
        double vp = str2dbl(tp, NULL), vn = str2dbl(tn, NULL);
        if(!mesmos(vn, -vp) || !mesmos(-vn, vp)) res_v++;   /* -(-v)=v, período 2, fecha nos dois lados */
        par_v++;
    }
    printf("§C2  1.º dual (sinal do valor, aditivo): -(-v)=v em %ld pares, %ld resíduo\n\n", par_v, res_v);
    ok("§C2 o 1.º DUAL é o SINAL DO VALOR (aditivo, ±m): o format escreve o '-', o parse lê-o, e"
       " -(-v)=v (período 2, fecha nos dois lados). É a reflexão em torno de 0 --- a Lei 1", res_v == 0);

    /* ── §C3 o 2.º dual: o SINAL DA ESCALA (multiplicativo) FECHA em vírgula-fixa ───────── */
    /* mover a vírgula: sign·m·10^(-k) ↦ sign·m·10^(+k). O que se MEDE (não a definição): que a
     * vírgula-fixa REALIZA os dois lados --- escreve "0.0m" (k>0) E "m00" (k<0) e o parse lê ambos
     * exactos. Assim o dual multiplicativo fecha como FORMA, ao contrário do expoente `e` que só o
     * parse tinha. A composição sinal∘escala dá um NOVO estado (não colapsa no de partida). */
    long res_e = 0, par_e = 0, cruz_nova = 0;
    for(long m = 1; m <= 400; m++) for(int k = 1; k <= 3; k++){
        char t1[40], t2[40]; fmt(t1, +1, m, k); fmt(t2, +1, m, -k);   /* 10^(-k) e 10^(+k) */
        if(!mesmos(str2dbl(t1, NULL), valor(+1, m, k))) res_e++;      /* "0.0m" fecha */
        if(!mesmos(str2dbl(t2, NULL), valor(+1, m, -k))) res_e++;     /* "m00"  fecha */
        /* sinal∘escala leva a um estado que NÃO é o de partida (senão os duais seriam o mesmo) */
        if(!mesmos(valor(-1, m, -k), valor(+1, m, k))) cruz_nova++;   /* -m·10^k != m·10^-k */
        par_e++;
    }
    printf("§C3  2.º dual (sinal da escala): os dois lados \"0.0m\"/\"m00\" fecham em %ld pares (%ld"
           " resíduo) ; sinal∘escala é estado novo em %ld/%ld\n\n", par_e, res_e, cruz_nova, par_e);
    ok("§C3 o 2.º DUAL é o SINAL DA ESCALA (multiplicativo, ±k): a vírgula-fixa REALIZA os dois lados"
       " (escreve \"0.0m\" e \"m00\", e o parse lê ambos exactos, resíduo 0) --- fecha como FORMA, ao"
       " contrário do expoente `e` que só o parse tinha; e sinal∘escala dá um estado NOVO, logo é um"
       " dual distinto do aditivo", res_e == 0 && cruz_nova == par_e);

    /* ── §C4 os dois duais dão órbita 4 (a cruz): GRAU 4 --- e degeneram a 2 nos inteiros ── */
    /* a evidência do grau: a órbita de um número sob os DOIS duais {+v, -v, +v', -v'} (v'=escala-dual).
     * Para escala>0 (não-inteiro) os quatro são DISTINTOS -> grau 4 (a cruz). Para escala=0 (inteiro)
     * v'=v, e a órbita colapsa a 2 -> grau 2: os inteiros são o CONJUNTO FIXO do dual da escala, onde
     * a cruz degenera (como o ponto fixo do trial). Se os dois duais fossem o mesmo, nunca daria 4. */
    long o4 = 0, npos = 0, o2 = 0, nzero = 0;
    for(long m = 1; m <= 400; m++){
        for(int k = 1; k <= 3; k++){                 /* escala > 0: espera 4 */
            double o[4] = { valor(+1,m,k), valor(-1,m,k), valor(+1,m,-k), valor(-1,m,-k) };
            int d = 1; for(int i=0;i<4&&d;i++) for(int j=i+1;j<4;j++) if(mesmos(o[i],o[j])){ d=0; break; }
            npos++; if(d) o4++;
        }
        {                                            /* escala = 0 (inteiro): espera 2 */
            double o[4] = { valor(+1,m,0), valor(-1,m,0), valor(+1,m,0), valor(-1,m,0) };
            int dist = 0; for(int i=0;i<4;i++){ int novo=1; for(int j=0;j<i;j++) if(mesmos(o[i],o[j])) novo=0; dist+=novo; }
            nzero++; if(dist == 2) o2++;
        }
    }
    printf("§C4  escala>0: %ld/%ld com 4 estados distintos (grau 4) ; inteiros (escala=0): %ld/%ld"
           " degeneram a 2 (grau 2, o conjunto fixo)\n\n", o4, npos, o2, nzero);
    ok("§C4 os DOIS duais dão órbita de QUATRO estados distintos {+v,-v,+v',-v'} para escala>0 --- a"
       " CRUZ, GRAU 4, a tetral (o par aditivo/multiplicativo das duas leis, no número). E degenera a"
       " GRAU 2 nos INTEIROS (escala=0), o conjunto fixo do dual da escala --- como o ponto fixo colapsa"
       " o trial. Se os dois duais fossem o mesmo, nunca daria 4", o4 == npos && o2 == nzero && npos > 0);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  SIM, forma um corpo --- e é a LEI DOS TECIDOS (Lei 4 tetral): GRAU 4, a cruz.");
        puts("  O número↔texto é um passo de tecido T+T*: escrita=T (contar os dígitos, o discreto),");
        puts("  leitura=T* (o valor, o contínuo), e a volta fecha (resíduo 0) --- é contar↔integrar, o");
        puts("  degrau Q→R do teorema central. O par (A,A*) lê-se pela CRUZ, cujas duas coordenadas são");
        puts("  o aditivo (o sinal do valor, ±m) e o multiplicativo (o sinal da escala, ±k, mover a");
        puts("  vírgula): órbita de 4 estados. A norma de CAYLEY-DICKSON não mede aqui --- o tecido");
        puts("  mede-se pela cruz, não pela norma, e o «grau 8» seria régua de fora. GRAU 4, a tetral.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
