/* isa.c — A ISA. UMA FUNÇÃO, UM LAÇO, E NADA ESCRITO DUAS VEZES.
 *
 * O Aarão, três vezes, e eu falhei as três por ordem:
 *
 *   «a única função que deve ter no ISA é o MOVE»   — eu tinha escrito dezanove
 *   «o ruído duplicado agora em todo lugar»          — apaguei-as e colei os corpos
 *   «tira todas essas funções e substitui tudo pelo MOVE»
 *
 * Inlinar não é apagar: a soma tinha passado a estar escrita três vezes. E deixar auxiliares
 * `static` também não é apagar — continuam a ser funções, só não saem pela porta.
 *
 * O que resolve as três de uma vez é fazer a conta UMA VEZ. O somador completo, bit a bit,
 * com carry de entrada, num laço só — e esse laço serve TUDO o que aqui soma: a soma, a
 * subtracção (que é somar o complemento com carry 1), o gato, a volta do gato e o simétrico.
 * Não há `soma`, não há `nand`, não há `menos`. Há destinos, e há um laço.
 *
 *     MOVE(destino, sentido)      +1 absorve · −1 emite
 *
 * ── O ESTADO NÃO É VARIÁVEL: É SLOT ──────────────────────────────────────────────────
 *
 * Não há `Atot`, `PC`, `FLAGS`, `arranca`, `reg`, `end_mem`. «Não há RAM; o estado vive no
 * disco, endereçado por MOVE» — e quem hospeda escreve nos slots pela vista da memória e
 * chama a função, que é o que o `app/src/motor_wasm.js` faz há meses com os módulos do
 * chessc: «escreve nos slots, chama, lê do slot; lida direto dos slots, sem transformação».
 *
 * ── O MAPA, QUE É O CONTRATO ─────────────────────────────────────────────────────────
 *
 *   slot 0 … 1023   a fita — cada slot são dois inteiros {total, e}
 *   1024 A · 1025 B · 1026 R · 1027 pc · 1028 bandeiras · 1029 tamanho do programa
 *   1030 …          o programa, em bytes, a partir do byte 1030·16
 *
 *   ler 1040 dá A+B · 1041 A−B · 1042 A&B · 1043 A|B · 1044 A^B
 *   ler 1045 dá o gato · 1046 a volta dele · 1047 ×i (ordem 4) · 1048 J (ordem 2)
 *   ler 1049 compara A e B e deixa as bandeiras
 *   ler 1050 BATE O RELÓGIO: corre um passo e devolve 1 enquanto houver programa
 *
 * O disco começa no byte 8 — o zero não é slot de ninguém, que é o ponteiro nulo do C. Quem
 * hospeda soma 8 e conta em slots de 16 bytes. Não há função que lho diga: é o contrato.
 *
 *   ../tools/bin/traduz isa.c -o isa.wasm
 */

static long M[8192];            /* o disco. `static`: o disco NÃO é porta — o MOVE corre nele. */

#define S_A       1024
#define S_B       1025
#define S_R       1026
#define S_PC      1027
#define S_FL      1028
#define S_N       1029
#define S_PROG    1030
#define S_SOMA    1040
#define S_SUB     1041
#define S_E       1042
#define S_OU      1043
#define S_XOU     1044
#define S_GOLD    1045
#define S_NEGRO   1046
#define S_ESQUILO 1047
#define S_TROCA   1048
#define S_CMP     1049
#define S_PASSO   1050

long MOVE(long destino, long sentido){
    if(sentido < 0){                       /* emite: o que está em R vai para o destino */
        M[2*destino] = M[2*S_R];
        M[2*destino + 1] = M[2*S_R + 1];
        return M[2*S_R];
    }

    if(destino == S_PASSO){                /* o relógio é um DESTINO: lê-se dele e ele bate */
        long pc = M[2*S_PC];
        if(pc >= M[2*S_N]) return 0;
        char *p = (char*)M;
        long base = S_PROG * 16;
        int op = p[base + pc] & 255;
        M[2*S_PC] = pc + 1;
        if(op == 0) return 0;

        /* o opcode não é uma operação: é um destino e um sinal, e esta é a ISA inteira */
        long d = -2;
        long s = 1;
        if(op == 1 || op == 14) d = -1;
        else if(op == 2){ d = -1; s = -1; }
        else if(op == 3) d = S_SOMA;
        else if(op == 4) d = S_SUB;
        else if(op == 5) d = S_E;
        else if(op == 6) d = S_OU;
        else if(op == 7) d = S_XOU;
        else if(op == 8) d = S_GOLD;
        else if(op == 15) d = S_NEGRO;
        else if(op == 16) d = S_ESQUILO;
        else if(op == 17) d = S_TROCA;
        else if(op == 9) d = S_CMP;
        else if(op == 10 || op == 11 || op == 12){ d = S_PC; s = -1; }
        if(d == -2) return 0;

        if(d < 0){                                 /* o slot vem escrito no programa */
            long q = M[2*S_PC];
            long sl = (p[base + q] & 255) + ((p[base + q + 1] & 255) << 8);
            M[2*S_PC] = q + 2;
            MOVE(sl, s);
            return 1;
        }
        if(d == S_PC){                             /* o salto: a condição decide SE se move */
            long rel = p[base + M[2*S_PC]];
            long cond = 1;
            if(op == 11) cond = M[2*S_FL] & 1;
            if(op == 12) cond = !(M[2*S_FL] & 1);
            long gt = M[2*S_R];
            long ge = M[2*S_R + 1];
            if(cond) M[2*S_R] = M[2*S_PC] + 1 + rel;
            else M[2*S_R] = M[2*S_PC] + 1;
            MOVE(S_PC, -1);
            M[2*S_R] = gt;
            M[2*S_R + 1] = ge;
            return 1;
        }
        MOVE(d, 1);
        return 1;
    }

    /* ── absorve. A SEMÂNTICA É A DO sql.c, QUE É O GABARITO — o medidor das duas
     * realizações (tests/isa_dupla.js) apanhou esta transcrição a divergir do erg na ULA,
     * e a régua é quem as duas transcrevem: a fita desloca A para B e NÃO toca em R; a
     * ULA escreve SÓ R; o giro (GOLD, NEGRO, ESQUILO, TROCA) move A e R; CMP só flags. */
    long t = 0;
    long e = 0;

    if(destino < S_SOMA){
        M[2*S_B] = M[2*S_A]; M[2*S_B + 1] = M[2*S_A + 1];
        M[2*S_A] = M[2*destino]; M[2*S_A + 1] = M[2*destino + 1];
        return M[2*S_A];
    }
    {
        long at = M[2*S_A];
        long ae = M[2*S_A + 1];
        long bt = M[2*S_B];
        long be = M[2*S_B + 1];

        /* o que soma diz COM QUE PARCELAS e com que carry; a conta é a mesma para todos */
        long x[2];
        long y[2];
        long cin = 0;
        int conta = 1;

        if(destino == S_SOMA){ x[0] = at; y[0] = bt; x[1] = ae; y[1] = be; }
        else if(destino == S_SUB){                 /* A − B = A + ~B + 1 */
            x[0] = at; y[0] = ~bt; x[1] = ae; y[1] = ~be; cin = 1;
        }
        else if(destino == S_GOLD){                /* (a,b) -> (a+b, a): o gato */
            x[0] = at; y[0] = ae; x[1] = 0; y[1] = at;
        }
        else if(destino == S_NEGRO){               /* (a,b) -> (b, a−b): a volta, INTEIRA */
            x[0] = 0; y[0] = ae; x[1] = at; y[1] = ~ae; cin = 1;
        }
        else if(destino == S_ESQUILO){             /* (a,b) -> (−b, a): ×i, ordem 4 */
            x[0] = 0; y[0] = ~ae; x[1] = 0; y[1] = at; cin = 1;
        }
        else conta = 0;

        if(conta){
            /* O SOMADOR, UMA VEZ. Bit a bit, com carry de entrada — e o carry de entrada é o
             * que faz o complemento virar subtracção sem uma segunda passagem. O de dentro
             * corre os 64 bits, o de fora corre as duas coordenadas da Word. */
            for(int k = 0; k < 2; k++){
                long r = 0;
                long c = cin;
                for(int i = 0; i < 64; i++){
                    long a1 = (x[k] >> i) & 1;
                    long b1 = (y[k] >> i) & 1;
                    long sm = a1 ^ b1 ^ c;
                    c = (a1 & b1) | (c & (a1 ^ b1));
                    r = r | (sm << i);
                }
                if(k == 0) t = r;
                else e = r;
            }
            /* o ESQUILO tem o segundo componente sem carry: é `a`, não `a+1` */
            if(destino == S_ESQUILO) e = at;
            if(destino == S_GOLD) e = at;
            if(destino == S_NEGRO) t = ae;
        }
        else if(destino == S_E){ t = ~(~(at & bt) & ~(at & bt));
                                 e = ~(~(ae & be) & ~(ae & be)); }
        else if(destino == S_OU){ t = ~(~(at & at) & ~(bt & bt));
                                  e = ~(~(ae & ae) & ~(be & be)); }
        else if(destino == S_XOU){ t = ~(~(at & ~(at & bt)) & ~(bt & ~(at & bt)));
                                   e = ~(~(ae & ~(ae & be)) & ~(be & ~(ae & be))); }
        else if(destino == S_TROCA){ t = ae; e = at; }        /* J: det −1, ordem 2 */
        else if(destino == S_CMP){
            long f = 0;
            if(at == 0 && ae == 0 && bt == 0 && be == 0) f = f + 1;
            if(at == bt && ae == be) f = f + 2;
            else if(at < bt) f = f + 4;               /* FL_LT: faltava, e o sql.c tem-no */
            M[2*S_FL] = f;
            return f;                                  /* CMP só escreve as flags */
        }
    }
    if(destino == S_GOLD || destino == S_NEGRO || destino == S_ESQUILO || destino == S_TROCA){
        M[2*S_A] = t; M[2*S_A + 1] = e;                /* o giro MOVE o estado: A = f(A) */
    }
    M[2*S_R] = t; M[2*S_R + 1] = e;                    /* e a conta sai sempre por R */
    return t;
}
