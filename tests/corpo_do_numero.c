/* corpo_do_numero.c — O NÚMERO↔TEXTO FORMA UM CORPO? E DE QUE GRAU --- 2 OU 4?
 *
 * O Aarão: «vê se isso forma um corpo, grau 2 ou 4.» E: «provavelmente são dois duais.»
 *
 * «Isso» é a camada número↔texto: o PARSE e o FORMAT. Tem os dois lados --- logo tem DUAL:
 * a volta fecha (parse∘format=id). E o Aarão viu o que eu não vi: são DOIS duais. Um número
 * em vírgula-fixa é sign · m · 10^(-k), e tem DOIS sinais, que são duas involuções:
 *
 *   - o SINAL DO VALOR (aditivo, ±m):  "100" ↔ "-100"  --- reflexão em torno de 0 (a Lei 1)
 *   - o SINAL DA ESCALA (multiplicativo, ±k): "0.01" ↔ "100" --- mover a vírgula
 *
 * As duas comutam e são independentes, logo a órbita de um número genérico tem QUATRO estados
 * {+v, -v, +v', -v'} (a CRUZ) --- GRAU 4, a tetral. Os quatro são racionais em ℤ, iguais sse
 * num_i·den_j = num_j·den_i. A volta é parse inteiro do texto, sem strtod.
 *
 *   §C1  forma CORPO: parse∘format = id (a volta fecha, resíduo 0)
 *   §C2  o 1.º dual --- o SINAL DO VALOR (aditivo): -(-v)=v, fecha nos dois lados, período 2
 *   §C3  o 2.º dual --- o SINAL DA ESCALA (multiplicativo): mover a vírgula, fecha, período 2
 *   §C4  os dois COMUTAM e são independentes -> órbita 4 (a cruz): GRAU 4
 *
 *   cc -O2 -std=c99 -Wall -I lib tests/corpo_do_numero.c -o corpo_do_numero
 */
#include <stdio.h>
#include "isa_disk.h"
#include "unidade.h"

static long pot10(int k){
    long p = 1;
    for(int i = 0; i < k; i++) p *= 10;
    return p;
}

/* o racional sign·m·10^(-k) em (num, den), den > 0 */
static void racional(int sign, long m, int k, long *num, long *den){
    long n = (sign < 0) ? -m : m;
    if(k >= 0){ *num = n; *den = pot10(k); }
    else      { *num = n * pot10(-k); *den = 1; }
}

static void fmt(char *o, int sign, long m, int k){
    char *p = o;
    if(sign < 0) *p++ = '-';
    if(k <= 0){
        char t[24]; int n = 0; long u = m;
        if(u == 0) t[n++] = '0'; else while(u){ t[n++] = (char)('0' + u % 10); u /= 10; }
        while(n) *p++ = t[--n];
        for(int i = 0; i < -k; i++) *p++ = '0';
    } else {
        long div = pot10(k);
        long ip = m / div, fr = m % div;
        char t[24]; int n = 0; long u = ip;
        if(u == 0) t[n++] = '0'; else while(u){ t[n++] = (char)('0' + u % 10); u /= 10; }
        while(n) *p++ = t[--n];
        *p++ = '.';
        for(int i = k - 1; i >= 0; i--){ long d = fr; for(int j = 0; j < i; j++) d /= 10; *p++ = (char)('0' + d % 10); }
    }
    *p = 0;
}

static int le_fixo(const char *s, int *sg, long *m, int *k){
    int sign = 1;
    if(*s == '-'){ sign = -1; s++; }
    else if(*s == '+') s++;
    long ip = 0; int houve = 0;
    while(*s >= '0' && *s <= '9'){ ip = ip*10 + (*s - '0'); s++; houve = 1; }
    int kk = 0; long fr = 0;
    if(*s == '.'){
        s++;
        while(*s >= '0' && *s <= '9'){ fr = fr*10 + (*s - '0'); kk++; s++; houve = 1; }
    }
    if(!houve || *s) return 0;
    *sg = sign; *m = ip * pot10(kk) + fr; *k = kk;
    return 1;
}

static int mesmo_q(long n1, long d1, long n2, long d2){ return n1 * d2 == n2 * d1; }

int main(void){
    printf("=== O NÚMERO↔TEXTO FORMA UM CORPO? GRAU 2 OU 4? --- SÃO DOIS DUAIS =================\n\n");

    /* ── §C1 forma CORPO: parse∘format = id, como racionais em ℤ ──────────────────────── */
    long res_corpo = 0, tot = 0;
    for(long m = 0; m <= 999; m++) for(int k = -2; k <= 3; k++) for(int sg = -1; sg <= 1; sg += 2){
        if(m == 0 && sg < 0) continue;
        char txt[40]; fmt(txt, sg, m, k);
        int sg2 = 0, k2 = 0; long m2 = 0;
        long n1, d1, n2, d2;
        racional(sg, m, k, &n1, &d1);
        if(!le_fixo(txt, &sg2, &m2, &k2)){ res_corpo++; tot++; continue; }
        racional(sg2, m2, k2, &n2, &d2);
        if(!mesmo_q(n1, d1, n2, d2)) res_corpo++;
        tot++;
    }
    printf("      %ld números (sign, m, escala±), parse∘format --- %ld resíduo\n\n", tot, res_corpo);
    ok("§C1 a camada número↔texto FORMA CORPO: o format (vírgula-fixa) escreve e o parse (le_fixo) lê"
       " de volta o MESMO racional em Z --- parse∘format = id, resíduo 0. Sem strtod",
       res_corpo == 0 && tot > 0);

    /* ── §C2 o 1.º dual: o SINAL DO VALOR (aditivo) ────────────────────────────────────── */
    long res_v = 0, par_v = 0;
    int per_troca = isa_periodo_giro(ISA_S_TROCA);
    for(long m = 1; m <= 400; m++) for(int k = -1; k <= 2; k++){
        long np, dp, nn, dn;
        racional(+1, m, k, &np, &dp);
        racional(-1, m, k, &nn, &dn);
        if(!(nn == -np && dn == dp)) res_v++;
        if(!mesmo_q(-nn, dn, np, dp)) res_v++;    /* -(-v)=v */
        par_v++;
    }
    printf("§C2  1.º dual (sinal do valor, aditivo): -(-v)=v em %ld pares, %ld resíduo"
           " ; TROCA no disco periodo %d\n\n", par_v, res_v, per_troca);
    ok("§C2 o 1.º DUAL é o SINAL DO VALOR (aditivo, ±m): -(-v)=v (período 2). No disco ISA,"
       " TROCA tem periodo 2 — é a reflexão em torno de 0, a Lei 1",
       res_v == 0 && per_troca == 2);

    /* ── §C3 o 2.º dual: o SINAL DA ESCALA (multiplicativo) ─────────────────────────────── */
    long res_e = 0, par_e = 0, cruz_nova = 0;
    for(long m = 1; m <= 400; m++) for(int k = 1; k <= 3; k++){
        char t1[40], t2[40]; fmt(t1, +1, m, k); fmt(t2, +1, m, -k);
        int sg1=0,k1=0,sg2=0,k2=0; long m1=0,m2=0;
        long n1,d1, n2,d2, nv,dv, nx,dx;
        racional(+1, m, k, &nv, &dv);
        racional(+1, m, -k, &nx, &dx);
        if(!le_fixo(t1, &sg1, &m1, &k1)) res_e++;
        else { racional(sg1, m1, k1, &n1, &d1); if(!mesmo_q(n1, d1, nv, dv)) res_e++; }
        if(!le_fixo(t2, &sg2, &m2, &k2)) res_e++;
        else { racional(sg2, m2, k2, &n2, &d2); if(!mesmo_q(n2, d2, nx, dx)) res_e++; }
        long na, da; racional(-1, m, -k, &na, &da);
        if(!mesmo_q(na, da, nv, dv)) cruz_nova++;
        par_e++;
    }
    printf("§C3  2.º dual (sinal da escala): os dois lados \"0.0m\"/\"m00\" fecham em %ld pares (%ld"
           " resíduo) ; sinal∘escala é estado novo em %ld/%ld\n\n", par_e, res_e, cruz_nova, par_e);
    ok("§C3 o 2.º DUAL é o SINAL DA ESCALA (multiplicativo, ±k): a vírgula-fixa REALIZA os dois lados"
       " (escreve \"0.0m\" e \"m00\", e o parse lê ambos o mesmo racional, resíduo 0); e sinal∘escala"
       " dá um estado NOVO, logo é um dual distinto do aditivo",
       res_e == 0 && cruz_nova == par_e);

    /* ── §C4 órbita 4 (a cruz): GRAU 4 --- degenera a 2 nos inteiros ────────────────────── */
    long o4 = 0, npos = 0, o2 = 0, nzero = 0;
    for(long m = 1; m <= 400; m++){
        for(int k = 1; k <= 3; k++){
            long n[4], d[4];
            racional(+1, m,  k, n+0, d+0);
            racional(-1, m,  k, n+1, d+1);
            racional(+1, m, -k, n+2, d+2);
            racional(-1, m, -k, n+3, d+3);
            int dist = 1;
            for(int i = 0; i < 4 && dist; i++)
                for(int j = i+1; j < 4; j++)
                    if(mesmo_q(n[i], d[i], n[j], d[j])) dist = 0;
            npos++; if(dist) o4++;
        }
        {
            long n[4], d[4];
            racional(+1, m, 0, n+0, d+0);
            racional(-1, m, 0, n+1, d+1);
            racional(+1, m, 0, n+2, d+2);
            racional(-1, m, 0, n+3, d+3);
            int dist = 0;
            for(int i = 0; i < 4; i++){
                int novo = 1;
                for(int j = 0; j < i; j++) if(mesmo_q(n[i], d[i], n[j], d[j])) novo = 0;
                dist += novo;
            }
            nzero++; if(dist == 2) o2++;
        }
    }
    printf("§C4  escala>0: %ld/%ld com 4 estados distintos (grau 4) ; inteiros (escala=0): %ld/%ld"
           " degeneram a 2 (grau 2, o conjunto fixo)\n\n", o4, npos, o2, nzero);
    ok("§C4 os DOIS duais dão órbita de QUATRO estados distintos {+v,-v,+v',-v'} para escala>0 --- a"
       " CRUZ, GRAU 4, a tetral. Os quatro sao racionais em Z, iguais sse num_i.den_j = num_j.den_i."
       " E degenera a GRAU 2 nos INTEIROS (escala=0)",
       o4 == npos && o2 == nzero && npos > 0);

    printf("==========================================================================\n");
    if(!falhas){
        puts("  SIM, forma um corpo --- e é a LEI DOS TECIDOS (Lei 4 tetral): GRAU 4, a cruz.");
        puts("  O número↔texto é um passo de tecido T+T*: escrita=T (contar os dígitos, o discreto),");
        puts("  leitura=T* (o valor, o contínuo), e a volta fecha (resíduo 0) --- é contar↔integrar, o");
        puts("  degrau Q→R do teorema central. Os quatro estados são racionais em Z. GRAU 4, a tetral.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
