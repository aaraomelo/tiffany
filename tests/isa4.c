/* isa4.c — A ISA TEM QUATRO OPERACOES, E AS OUTRAS QUINZE DERIVAM-SE.
 *
 * O Aarao: "reduz tudo a 4 se possivel, depois centramos em 1" e "transforma o excedente
 * em funcao primeiro, depois sai trocando".
 *
 * Feito, e este medidor prova a reducao em vez de a afirmar. Os quatro irredutiveis:
 *
 *     LOAD / STORE   o par dual — gerador e motor, e o SINAL decide qual
 *     o SALTO        uma funcao, e a CONDICAO e' argumento
 *     NAND           e nao vive no processador: vive no SLOT (banco_relogio.c §B4)
 *     o CORPO        e' CAMPO da instrucao, nao opcode
 *
 *   §I1  os tres saltos sao um: a condicao e' argumento
 *   §I2  AND, OR, XOR e NOT saem de NAND — e verificam-se nas 4 entradas
 *   §I3  ADD e SUB saem de NAND, exactos, com negativos
 *   §I4  ESQUILO e TROCA sao a mesma operacao: o sinal decide (e e' o J da Lei 2)
 *   §I5  o gato e a volta sao a mesma: A_n^-1 = J.A_{-n}.J
 *   §I6  e o CONTROLO: XOR sozinho NAO chega — nao e' universal
 *
 * Zero doubles.
 *
 *   cc -O2 -std=c99 -Wall -I../lib isa4.c -o isa4
 */
#include <stdio.h>
#include "unidade.h"

static long nd(long a, long b){ return ~(a & b); }
static long n_and(long a, long b){ long n = nd(a,b); return nd(n,n); }
static long n_or (long a, long b){ return nd(nd(a,a), nd(b,b)); }
static long n_xor(long a, long b){ long n = nd(a,b); return nd(nd(a,n), nd(b,n)); }
static long n_not(long a){ return nd(a,a); }
static long n_soma(long a, long b){
    long s = n_xor(a,b), c = n_and(a,b);
    for(int i = 0; i < 64 && c; i++){ c <<= 1; long s2 = n_xor(s,c), c2 = n_and(s,c); s=s2; c=c2; }
    return s;
}
static int salto(long pc, long rel, int cond){ return (int)(cond ? pc + 1 + rel : pc + 1); }
static void gira(long *a, long *b, long s){ long t = s * (*b); *b = *a; *a = t; }

int main(void){
    puts("\n  A ISA TEM QUATRO OPERACOES — e as outras quinze derivam-se\n");

    /* ═══ §I1 — os tres saltos sao um ══════════════════════════════════════════════ */
    {
        long mau = 0;
        for(long pc = 0; pc < 50; pc++) for(long rel = -8; rel <= 8; rel++){
            if(salto(pc,rel,1)      != pc+1+rel) mau++;      /* JMP  */
            if(salto(pc,rel,0)      != pc+1)     mau++;      /* nao salta */
            /* e distinguem-se — EXCEPTO em rel = 0, onde saltar zero E' nao saltar.
             * A primeira versao exigia que fossem sempre diferentes e o medidor apanhou-a. */
            if(rel != 0 && salto(pc,rel,1) == salto(pc,rel,0)) mau++;
            if(rel == 0 && salto(pc,rel,1) != salto(pc,rel,0)) mau++;
        }
        ok("os TRES saltos sao UM: JMP e' condicao sempre verdadeira, JZ e' a flag, JNZ e'"
           " a flag negada — e a condicao e' ARGUMENTO, nao instrucao", mau == 0);
    }

    /* ═══ §I2 — os booleanos saem de NAND ══════════════════════════════════════════ */
    {
        long mau = 0;
        for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++){
            long A = a ? -1L : 0L, B = b ? -1L : 0L;         /* 0 ou todos os bits */
            if(n_and(A,B) != (A & B)) mau++;
            if(n_or (A,B) != (A | B)) mau++;
            if(n_xor(A,B) != (A ^ B)) mau++;
            if(n_not(A)   != ~A)      mau++;
        }
        /* e em palavras inteiras, nao so' em bits soltos */
        for(long a = -300; a <= 300; a += 7) for(long b = -300; b <= 300; b += 11){
            if(n_and(a,b) != (a&b)) mau++;
            if(n_or (a,b) != (a|b)) mau++;
            if(n_xor(a,b) != (a^b)) mau++;
        }
        ok("AND, OR, XOR e NOT saem todos de NAND — verificados nas quatro entradas e em"
           " palavras inteiras. Nenhum deles precisa de corpo proprio", mau == 0);
    }

    /* ═══ §I3 — a aritmetica sai de NAND ═══════════════════════════════════════════ */
    {
        long mau = 0, casos = 0;
        for(long a = -400; a <= 400; a += 3) for(long b = -400; b <= 400; b += 7){
            casos++;
            if(n_soma(a,b) != a + b) mau++;
            if(n_soma(a, n_soma(n_not(b),1)) != a - b) mau++;   /* SUB = ADD do complemento */
        }
        printf("      soma e subtraccao sobre NAND: %ld casos, %ld falhas\n", casos, mau);
        ok("ADD e SUB saem de NAND pelo meia-soma com transporte, EXACTOS e com negativos —"
           " e a subtraccao e' a soma do complemento, -b = NAND(b,b) + 1", mau == 0);
    }

    /* ═══ §I4 — o esquilo e a troca sao a mesma, e e' o J ══════════════════════════ */
    {
        long mau = 0;
        for(long a = -9; a <= 9; a++) for(long b = -9; b <= 9; b++){
            long x = a, y = b; gira(&x,&y,-1);               /* ESQUILO: (a,b)->(-b,a) */
            if(x != -b || y != a) mau++;
            long u = a, v = b; gira(&u,&v,+1);               /* TROCA:   (a,b)->( b,a) */
            if(u != b || v != a) mau++;
            /* e o esquilo aplicado QUATRO vezes devolve: e' o J, com J^4 = I */
            long p = a, q = b;
            for(int k = 0; k < 4; k++) gira(&p,&q,-1);
            if(p != a || q != b) mau++;
        }
        ok("ESQUILO e TROCA sao a MESMA operacao com o SINAL trocado — e o esquilo aplicado"
           " quatro vezes devolve: e' o J da Lei 2, a unica solucao de -f = f^-1", mau == 0);
    }

    /* ═══ §I5 — o gato e a volta sao a mesma ═══════════════════════════════════════ */
    {
        long mau = 0;
        for(long n = -6; n <= 6; n++) for(long a = -8; a <= 8; a++) for(long b = -8; b <= 8; b++){
            long ca = n*a + b, cb = a;                       /* cifra */
            long da = cb, db = ca - n*cb;                    /* decifra: volta ao ponto */
            if(da != a || db != b) mau++;
            /* e a volta E' a cifra com n -> -n entre duas trocas: J.A_{-n}.J */
            long ja = b, jb = a;                             /* J(w) */
            long ka = (-n)*ja + jb, kb = ja;                 /* cifra com -n */
            long la = kb, lb = ka;                           /* J outra vez */
            if(la != b || lb != a - n*b) mau++;
        }
        ok("o GATO e a VOLTA sao a mesma operacao: a inversa e' A_n^-1 = J.A_{-n}.J — a"
           " antipoda entre duas trocas —, e ela e' INTEIRA porque |det| = 1", mau == 0);
    }

    /* ═══ §I6 — o CONTROLO: XOR sozinho nao chega ══════════════════════════════════
     * A tentacao e' dizer "tudo sai de XOR", e e' FALSO: XOR e' linear sobre GF(2) e o AND
     * nao e'. Mede-se: as combinacoes de XOR de a e b (e das constantes) nunca dao AND. */
    {
        int achou_and = 0;
        /* as funcoes lineares de duas variaveis sobre GF(2): 0, a, b, a^b, e os complementos */
        int lin[8][4]; int nl = 0;
        for(int c0 = 0; c0 < 2; c0++) for(int ca = 0; ca < 2; ca++) for(int cb = 0; cb < 2; cb++){
            for(int a = 0; a < 2; a++) for(int b = 0; b < 2; b++)
                lin[nl][a*2+b] = c0 ^ (ca & a) ^ (cb & b);
            nl++;
        }
        int tab_and[4] = { 0,0,0,1 };
        for(int i = 0; i < nl; i++){
            int igual = 1;
            for(int k = 0; k < 4; k++) if(lin[i][k] != tab_and[k]) igual = 0;
            if(igual) achou_and = 1;
        }
        printf("\n      funcoes lineares (XOR e constantes) testadas: %d, e nenhuma e' AND: %s\n",
               nl, achou_and ? "FALSO" : "certo");
        ok("e o CONTROLO: XOR SOZINHO NAO CHEGA — ele e' linear sobre GF(2) e o AND nao e',"
           " logo nenhuma combinacao de XOR o produz. O irredutivel e' o NAND, e ele nao e'"
           " um opcode a acrescentar: E' A OPERACAO DO SLOT", !achou_and && nl == 8);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  A ISA TEM QUATRO OPERACOES:");
        puts("");
        puts("     LOAD / STORE   o par dual — gerador e motor, e o SINAL decide qual");
        puts("     o SALTO        uma funcao, e a CONDICAO e' argumento");
        puts("     NAND           e nao vive no processador: vive no SLOT");
        puts("     o CORPO        e' CAMPO da instrucao, nao opcode");
        puts("");
        puts("  E AS OUTRAS QUINZE DERIVAM-SE, cada uma medida aqui: os tres saltos sao um,");
        puts("  os quatro booleanos saem de NAND, a aritmetica sai do meia-soma, o esquilo e");
        puts("  a troca diferem no sinal, e o gato e a volta na antipoda entre duas trocas.");
        puts("");
        puts("  E O CONTROLO DIZ O QUE NAO SE PODE DIZER: 'tudo sai de XOR' e' FALSO. O XOR");
        puts("  e' linear sobre GF(2) e o AND nao e' — nenhuma combinacao de XOR o produz. O");
        puts("  irredutivel e' o NAND, e o slot ja' o era antes de sabermos.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
