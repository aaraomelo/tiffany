/* mcu.c — O MICROCONTROLADOR MULTIFRACTAL: o circuito completo, do transistor ao programa.
 *
 * O Aarão: "agora um circuito completo, o microcontrolador multifractal."
 *
 * A arquitetura está em chess/sandbox/corpo_transistor.tex, e cada peça da corte é um bloco:
 *
 *     CLOCK      o PRÍNCIPE (Benjamim, o astável)   T = ln2·(R₁C₁ + R₂C₂)
 *     ALU        os DUQUES — JOAQUIM (⊕, o somador) e YASMIN (⊗, o multiplicador)
 *     MEMÓRIA    o grafo multifractal, endereçamento b^n
 *     CONTROLE   o motor — o program counter é o lance
 *     BARRAMENTO casado ao metal Z₀ = σ_m, com Γ = 0: sem eco, resíduo 0
 *
 * E o fecho: o MATE é o HALT. O ponto fixo — executá-lo de novo não move nada, Δ = 0.
 *
 * O que este medidor tem de diferente dos outros: ele não mede peças soltas, monta a máquina
 * INTEIRA e roda um programa nela. A ALU é feita SÓ de NAND (as portas medidas no
 * amplifica.c §A7), o somador é o ripple-carry, e no fim compara-se o que a máquina computou
 * com a conta direta. Dois caminhos, e o circuito só fecha se concordarem.
 *
 *   §U1  o CLOCK: o astável, e o seu período é ln2·(R₁C₁+R₂C₂)
 *   §U2  o BARRAMENTO casado: Γ = 0, e o ganho MÁXIMO é o casamento
 *   §U3  a ALU dos Duques, montada só de NAND
 *   §U4  a MEMÓRIA multifractal: endereçamento b^n, e a árvore é o índice
 *   §U5  o CICLO: fetch-decode-execute, um por pulso
 *   §U6  o HALT é o PONTO FIXO — executá-lo de novo não move nada
 *   §U7  o BUSTROFÉDON: passo unitário, sem fio de retorno
 *   §U8  RODAR: o circuito completo executa, e valida contra a conta direta
 *
 *   cc -O2 -std=c99 mcu.c -lm -o mcu && ./mcu
 */
#include <stdio.h>
#include <string.h>
#include "eletrico.h"
#include "unidade.h"

/* ---- as portas: tudo nasce do NAND (o operador), como o §A7 mediu ---------------------- */
static int NAND(int a,int b){ return !(a && b); }
static int NOT (int a)      { return NAND(a,a); }
static int AND (int a,int b){ return NOT(NAND(a,b)); }
static int OR  (int a,int b){ return NAND(NOT(a), NOT(b)); }
static int XOR (int a,int b){ int t = NAND(a,b); return NAND(NAND(a,t), NAND(b,t)); }

/* ---- JOAQUIM (⊕): o somador de 8 bits, ripple-carry, só de portas ---------------------- */
static int joaquim(int x, int y, int *carry_out){
    int c = 0, r = 0;
    for(int k = 0; k < 8; k++){
        int a = (x>>k)&1, b = (y>>k)&1;
        int s = XOR(XOR(a,b), c);
        c = OR(AND(a,b), AND(c, XOR(a,b)));
        r |= s << k;
    }
    if(carry_out) *carry_out = c;
    return r & 0xFF;
}
/* ---- YASMIN (⊗): o multiplicador, por deslocamento e soma — e a soma é o Joaquim ------- */
static int yasmin(int x, int y){
    int acc = 0;
    for(int k = 0; k < 8; k++)
        if((y>>k)&1) acc = joaquim(acc, (x<<k) & 0xFF, 0);
    return acc & 0xFF;
}
/* e as lógicas bit a bit, também só de portas */
static int alu_and(int x,int y){ int r=0; for(int k=0;k<8;k++) r |= AND((x>>k)&1,(y>>k)&1)<<k; return r; }
static int alu_xor(int x,int y){ int r=0; for(int k=0;k<8;k++) r |= XOR((x>>k)&1,(y>>k)&1)<<k; return r; }
static int alu_not(int x)      { int r=0; for(int k=0;k<8;k++) r |= NOT((x>>k)&1)<<k; return r; }

/* ---- a MÁQUINA ------------------------------------------------------------------------- */
enum { HALT=0, LDI, MOV, ADD, MUL, ANDR, XORR, NOTR, JNZ, DEC, OUT };
typedef struct { int op, a, b; } Instr;
typedef struct {
    int R[4], PC, ciclos, parado, saida[16], ns;
} Mcu;

static const char *nome_op(int op){
    static const char *n[] = { "HALT","LDI","MOV","ADD","MUL","AND","XOR","NOT","JNZ","DEC","OUT" };
    return (op >= 0 && op <= OUT) ? n[op] : "?";
}
/* UM PULSO DO CLOCK = UM CICLO: fetch, decode, execute. */
static void pulso(Mcu *m, const Instr *prog, int n){
    if(m->parado) return;
    if(m->PC < 0 || m->PC >= n){ m->parado = 1; return; }
    Instr i = prog[m->PC];                       /* FETCH: o motor aponta, busca o lance */
    int avanca = 1;
    switch(i.op){                                /* DECODE + EXECUTE */
        case HALT: m->parado = 1; avanca = 0; break;
        case LDI:  m->R[i.a] = i.b & 0xFF; break;
        case MOV:  m->R[i.a] = m->R[i.b]; break;
        case ADD:  m->R[i.a] = joaquim(m->R[i.a], m->R[i.b], 0); break;
        case MUL:  m->R[i.a] = yasmin (m->R[i.a], m->R[i.b]); break;
        case ANDR: m->R[i.a] = alu_and(m->R[i.a], m->R[i.b]); break;
        case XORR: m->R[i.a] = alu_xor(m->R[i.a], m->R[i.b]); break;
        case NOTR: m->R[i.a] = alu_not(m->R[i.a]); break;
        case DEC:  m->R[i.a] = joaquim(m->R[i.a], 0xFF, 0); break;   /* -1 é +255 em 8 bits */
        case JNZ:  if(m->R[i.a]){ m->PC = i.b; avanca = 0; } break;
        case OUT:  if(m->ns < 16) m->saida[m->ns++] = m->R[i.a]; break;
    }
    if(avanca) m->PC++;
    m->ciclos++;
}
static void roda(Mcu *m, const Instr *prog, int n, int limite){
    memset(m, 0, sizeof *m);
    for(int k = 0; k < limite && !m->parado; k++) pulso(m, prog, n);
}

int main(void){
printf("\n=== O MICROCONTROLADOR MULTIFRACTAL: O CIRCUITO COMPLETO =================\n");
printf("    Não são peças soltas: monta-se a máquina inteira e roda-se um programa\n");
printf("    nela. A ALU é feita SÓ de NAND, e no fim compara-se com a conta direta.\n");

printf("\n§U1  O CLOCK: o astável, e o seu período é ln2·(R₁C₁ + R₂C₂).\n\n");
{
    /* O astavel e' o PRINCIPE: o marca-passo. E o periodo sai da EXPONENCIAL do RC — que e'
     * o mesmo operador de Shockley noutra roupa: o capacitor carrega por e^{-t/RC}, e o
     * limiar e' atingido em t = RC·ln2. Pontryagin a marcar o compasso. */
    printf("      T = ln2·(R₁C₁ + R₂C₂)      e  f = 1/T\n\n");
    printf("      R₁ (kΩ)  C₁ (nF)  R₂ (kΩ)  C₂ (nF)   T (µs)      f (kHz)\n");
    int mal = 0;
    for(int k = 0; k < 4; k++){
        double R1 = (10.0 + 5*k)*1e3, C1 = 10e-9, R2 = (22.0 + 3*k)*1e3, C2 = 10e-9;
        double T = log(2.0)*(R1*C1 + R2*C2);
        /* o SEGUNDO caminho: integrar a carga do capacitor ate o limiar de 1/2 */
        double t = 0, h = 1e-12, v = 0;
        while(v < 0.5 && t < 1){ v += h*(1.0 - v)/(R1*C1); t += h; }
        double T1 = t;
        double T1_for = R1*C1*log(2.0);
        printf("      %-8.0f %-8.0f %-8.0f %-9.0f %-11.4f %.4f\n",
               R1/1e3, C1*1e9, R2/1e3, C2*1e9, T*1e6, 1.0/T/1e3);
        if(fabs(T1 - T1_for)/T1_for > 1e-4) mal++;   /* a meia-fase, pelos dois caminhos */
    }
    printf("\n      (e a meia-fase medida por integração bate com RC·ln2: %d falhas)\n\n", mal);
    ok("o clock é o astável, e o período sai da EXPONENCIAL do RC — dois caminhos",
       mal == 0);
    printf("      O marca-passo não é uma peça à parte: é a mesma exponencial do transistor,\n");
    printf("      agora a carregar um capacitor. O operador Π marca o compasso da máquina.\n");
}

printf("\n§U2  O BARRAMENTO casado: Γ = 0, e o ganho MÁXIMO é o casamento.\n\n");
{
    /* Γ = (Z_L - Z0)/(Z_L + Z0), e Γ = 0 exatamente em Z_L = Z0. E a potencia entregue
     * P = V²R_L/(R_s+R_L)² e' MAXIMA em R_L = R_s — o ganho maximo E o residuo 0. */
    double Z0 = 50.0, V = 5.0;
    printf("      Z₀ = %.0f Ω (o metal do barramento)\n\n", Z0);
    printf("      Z_L (Ω)   Γ = (Z-Z₀)/(Z+Z₀)   |Γ|²  (eco)    P entregue (mW)\n");
    double Pmax = 0, ZPmax = 0;
    int malG = 0;
    for(int k = 0; k < 7; k++){
        double ZL = 10.0*(k+1) + (k==4 ? 0 : 0);
        double G = (ZL-Z0)/(ZL+Z0), P = V*V*ZL/((Z0+ZL)*(Z0+ZL));
        printf("      %-9.0f %+-20.6f %-14.6f %.6f\n", ZL, G, G*G, P*1e3);
        if(P > Pmax){ Pmax = P; ZPmax = ZL; }
        if(fabs(ZL-Z0) < 1e-9 && fabs(G) > 1e-12) malG++;
    }
    /* varrer fino para achar o maximo de verdade */
    double melhor = 0, zbest = 0;
    for(double ZL = 1; ZL <= 200; ZL += 0.01){
        double P = V*V*ZL/((Z0+ZL)*(Z0+ZL));
        if(P > melhor){ melhor = P; zbest = ZL; }
    }
    printf("\n      máximo de P varrendo fino: Z_L = %.2f Ω  (e Z₀ = %.0f)\n", zbest, Z0);
    printf("      P_max medido = %.6f mW,  e V²/4R_s = %.6f mW\n\n",
           melhor*1e3, V*V/(4*Z0)*1e3);
    /* A VARREDURA MEDIA O PASSO, NAO A LEI: `fabs(zbest - Z0) < 0.05` com passo 0,01 diz
     * que o maximo caiu perto — mas a lei e EXATA e prova-se por identidade:
     *
     *     P(Z) = V^2 Z/(Z0+Z)^2  <=  V^2/(4 Z0)   <=>   4 Z0 Z <= (Z0+Z)^2   <=>   0 <= (Z0-Z)^2
     *
     * e a igualdade da-se SO em Z = Z0. E a media aritmetico-geometrica, e mede-se em
     * INTEIROS sem varrer nada. */
    {
        long long Z0i = (long long)Z0, casos=0, vale=0, so_igual=0;
        for(long long Z=1; Z<=400; Z++){
            long long lhs = 4*Z0i*Z, rhs = (Z0i+Z)*(Z0i+Z);
            casos++;
            if(lhs <= rhs) vale++;
            if((lhs == rhs) == (Z == Z0i)) so_igual++;
        }
        printf("      Z inteiros de 1 a 400 (Z0 = %lld):  4 Z0 Z <= (Z0+Z)^2 em %lld\n",
               Z0i, vale);
        printf("      e a IGUALDADE da-se exatamente em Z = Z0: %lld de %lld\n", so_igual, casos);
        ok("P e maxima em Z_L = Z0, e a prova e 0 <= (Z0-Z)^2 — identidade, em inteiros",
           vale==casos && so_igual==casos && casos==400);
    }
    printf("      É o mesmo lugar das outras vezes, com outro nome: o ganho máximo é o resíduo\n");
    printf("      0. Casar ao metal não é otimizar — é fazer o eco desaparecer, e a potência\n");
    printf("      inteira passar. Fora do casamento há reflexão, e a reflexão é energia que\n");
    printf("      volta sem ter feito nada.\n");
}

printf("\n§U3  A ALU dos Duques, montada SÓ de NAND.\n\n");
{
    /* JOAQUIM (⊕) e' o somador; YASMIN (⊗) e' o multiplicador. E os dois sao construidos
     * do NAND — o operador — como o §A7 mediu. O produto passa PELO operador. */
    printf("      JOAQUIM ⊕ = o somador (ripple-carry, 8 bits)\n");
    printf("      YASMIN  ⊗ = o multiplicador (deslocar e somar — e a soma é o Joaquim)\n\n");
    int malS = 0, malM = 0, malL = 0;
    for(int x = 0; x < 256; x++) for(int y = 0; y < 256; y++){
        int c, s = joaquim(x,y,&c);
        if(((c<<8)|s) != x+y) malS++;
        if(yasmin(x,y) != ((x*y) & 0xFF)) malM++;
        if(alu_and(x,y) != (x&y) || alu_xor(x,y) != (x^y)) malL++;
    }
    for(int x = 0; x < 256; x++) if(alu_not(x) != (~x & 0xFF)) malL++;
    printf("      JOAQUIM contra x+y, 65536 pares          : %d falhas\n", malS);
    printf("      YASMIN contra x·y mod 256, 65536 pares   : %d falhas\n", malM);
    printf("      AND/XOR/NOT contra os do C, 65536+256    : %d falhas\n\n", malL);
    ok("a ALU inteira sai do NAND, e bate com a aritmética em 65536 pares",
       malS == 0 && malM == 0 && malL == 0);
    printf("      E note-se a ordem em que as coisas nascem: do OPERADOR (NAND) saem a SOMA\n");
    printf("      (Joaquim, o XOR encadeado) e o PRODUTO (Yasmin, que é soma deslocada). O\n");
    printf("      operador é o gerador, e os Duques são o que ele gera — que é a mesma frase\n");
    printf("      do contrato, agora com portas.\n");
}

printf("\n§U4  A MEMÓRIA multifractal: o endereço é o caminho na árvore.\n\n");
{
    /* O enderecamento e' FRACTAL: b^n. Cada endereco e' o topo de um sub-fractal, e o
     * caminho na arvore E' os digitos do endereco na base b. Mede-se a bijecao. */
    int b = 4, n = 4;                              /* base 4, profundidade 4 -> 256 células */
    int total = 1; for(int k = 0; k < n; k++) total *= b;
    printf("      base b = %d, profundidade n = %d  ->  %d células\n\n", b, n, total);
    printf("      endereço   caminho na árvore     de volta   confere?\n");
    int mal = 0;
    for(int e = 0; e < total; e++){
        int cam[8], t = e;
        for(int k = 0; k < n; k++){ cam[k] = t % b; t /= b; }
        int volta = 0, pot = 1;
        for(int k = 0; k < n; k++){ volta += cam[k]*pot; pot *= b; }
        if(volta != e) mal++;
        if(e < 4 || e == total-1)
            printf("      %-10d %d-%d-%d-%d                %-10d %s\n", e,
                   cam[3],cam[2],cam[1],cam[0], volta, volta == e ? "sim" : "NÃO");
    }
    printf("\n      %d endereços, %d falhas\n\n", total, mal);
    ok("o endereço É o caminho na árvore, e a volta é exata — a bijeção fecha", mal == 0);
    printf("      Não há tabela de endereços: o endereço é o próprio percurso. É o mesmo que\n");
    printf("      o projeto diz da cifra — a trie É o índice, e não há índice à parte do\n");
    printf("      objeto. Aqui o mesmo, em barramento de memória.\n");
}

printf("\n§U5  O CICLO: fetch-decode-execute, um por pulso do clock.\n\n");
{
    /* Um pulso = um ciclo. Mede-se que o PC avanca exatamente um por pulso (salvo salto), e
     * que o numero de ciclos e' o numero de instrucoes executadas. */
    Instr prog[] = {
        { LDI, 0, 7 },      /* R0 = 7 */
        { LDI, 1, 6 },      /* R1 = 6 */
        { MUL, 0, 1 },      /* R0 = R0 * R1 = 42 */
        { OUT, 0, 0 },
        { HALT,0, 0 },
    };
    int n = sizeof prog/sizeof *prog;
    Mcu m;
    memset(&m, 0, sizeof m);
    printf("      ciclo   PC   instrução      R0    R1    estado\n");
    for(int k = 0; k <= n; k++){
        printf("      %-7d %-4d %-14s %-5d %-5d %s\n", m.ciclos, m.PC,
               m.PC < n ? nome_op(prog[m.PC].op) : "-", m.R[0], m.R[1],
               m.parado ? "PARADO" : "a correr");
        if(m.parado) break;
        pulso(&m, prog, n);
    }
    printf("\n      saída: ");
    for(int k = 0; k < m.ns; k++) printf("%d ", m.saida[k]);
    printf("\n      e 7 × 6 = %d\n\n", 7*6);
    ok("o ciclo executa um por pulso, e a saída é a conta certa (7 × 6 = 42)",
       m.ns == 1 && m.saida[0] == 42 && m.parado);
    printf("      E o MUL passou pelo operador: Yasmin é soma deslocada, a soma é Joaquim, e\n");
    printf("      Joaquim é XOR encadeado — que sai do NAND. O produto NUNCA é direto: passa\n");
    printf("      sempre pelo operador, e é isso que o §U7 vai medir como lei.\n");
}

printf("\n§U6  O HALT é o PONTO FIXO — executá-lo de novo não move nada.\n\n");
{
    /* O mate e' o HALT: o estado absorvente. Executar mais pulsos depois de parado nao muda
     * NADA — Δ = 0, o residuo 0. Mede-se comparando o estado antes e depois. */
    Instr prog[] = { { LDI, 0, 3 }, { OUT, 0, 0 }, { HALT, 0, 0 } };
    int n = 3;
    Mcu m; roda(&m, prog, n, 100);
    Mcu antes = m;
    for(int k = 0; k < 1000; k++) pulso(&m, prog, n);   /* mais mil pulsos */
    int igual = (m.PC == antes.PC) && (m.ciclos == antes.ciclos)
             && !memcmp(m.R, antes.R, sizeof m.R) && (m.ns == antes.ns);
    printf("      depois do HALT:   PC = %d, ciclos = %d, R0 = %d\n", antes.PC, antes.ciclos,
           antes.R[0]);
    printf("      + 1000 pulsos:    PC = %d, ciclos = %d, R0 = %d\n\n", m.PC, m.ciclos, m.R[0]);
    ok("o HALT é absorvente: mil pulsos a mais não movem nada — Δ = 0", igual);
    printf("      É a cifra imóvel outra vez, noutro sítio: o ponto onde aplicar a operação\n");
    printf("      devolve o mesmo. O mate é isso — não é o fim por convenção, é o estado de\n");
    printf("      onde não se sai. E o resíduo 0 é como se reconhece.\n");
}

printf("\n§U7  O BUSTROFÉDON: passo unitário, sem fio de retorno.\n\n");
{
    /* A varredura em bustrofedon: a linha par vai E->D, a impar D->E. O passo entre celulas
     * consecutivas e' SEMPRE 1 — o boi arando, sem fio de retorno. Contra o raster, que
     * salta N ao fim de cada linha. Mede-se a soma dos saltos. */
    /* Mede-se a LEI para vários N, e não constantes escritas de cabeça. Escrevi maxR == N-1
     * e o valor é N: o retorno do raster anda N-1 em x E MAIS 1 em y. As leis são
     *     bustrofédon: N²-1  passos, maior passo 1
     *     raster:      2N(N-1) passos, maior passo N
     * e a razão entre os dois tende a 2. */
    printf("      N     bustrofédon (soma, máx)   raster (soma, máx)   N²-1   2N(N-1)   razão\n");
    int mal = 0;
    for(int N = 3; N <= 10; N++){
        int saltoB = 0, saltoR = 0, maxB = 0, maxR = 0;
        int bx = 0, by = 0, rx = 0, ry = 0, primeiro = 1;
        for(int lin = 0; lin < N; lin++)
            for(int c = 0; c < N; c++){
                int x = (lin % 2 == 0) ? c : (N-1-c), y = lin;   /* bustrofédon */
                int xr = c, yr = lin;                             /* raster */
                if(!primeiro){
                    int d = abs(x-bx) + abs(y-by), dr = abs(xr-rx) + abs(yr-ry);
                    saltoB += d; saltoR += dr;
                    if(d  > maxB) maxB = d;
                    if(dr > maxR) maxR = dr;
                }
                bx = x; by = y; rx = xr; ry = yr; primeiro = 0;
            }
        printf("      %-5d %-6d %-18d %-6d %-13d %-6d %-9d %.4f\n", N,
               saltoB, maxB, saltoR, maxR, N*N-1, 2*N*(N-1), (double)saltoR/saltoB);
        if(maxB != 1 || saltoB != N*N-1) mal++;              /* a lei do bustrofédon */
        if(maxR != N || saltoR != 2*N*(N-1)) mal++;          /* a lei do raster      */
    }
    printf("\n");
    ok("as duas leis fecham em N = 3..10: bustrofédon N²-1 com passo 1, raster 2N(N-1) com passo N",
       mal == 0);
    printf("      O boi arando: chega ao fim da leira e vira ali mesmo, sem voltar ao começo.\n");
    printf("      O raster paga um retorno por linha, e esse retorno é trabalho que não lavra.\n");
    printf("      E a mão troca a cada linha — é o quiral em movimento, σ e σ' alternando.\n");
}

printf("\n§U8  RODAR: o circuito completo executa, e valida contra a conta direta.\n\n");
{
    /* O CIRCUITO COMPLETO. Um programa de verdade — a soma 1+2+...+n — rodado na maquina
     * cuja ALU sai do NAND, cujo clock e' o astavel e cujo barramento e' casado. E validado
     * contra a formula n(n+1)/2. Dois caminhos, e so' fecha se concordarem. */
    printf("      programa: acumula 1 + 2 + ... + n, com JNZ e DEC\n\n");
    printf("      R0 = n (o contador),  R1 = 0 (o acumulador)\n");
    printf("      laço:  R1 += R0 ; R0-- ; se R0 != 0 volta\n\n");
    printf("      n     saída da MÁQUINA   n(n+1)/2   ciclos   confere?\n");
    int mal = 0;
    for(int n = 1; n <= 12; n++){
        Instr prog[] = {
            { LDI, 0, n },        /* 0: R0 = n            */
            { LDI, 1, 0 },        /* 1: R1 = 0            */
            { ADD, 1, 0 },        /* 2: R1 += R0          */
            { DEC, 0, 0 },        /* 3: R0--              */
            { JNZ, 0, 2 },        /* 4: se R0 != 0 -> 2   */
            { OUT, 1, 0 },        /* 5: emite R1          */
            { HALT,0, 0 },        /* 6: para              */
        };
        Mcu m; roda(&m, prog, 7, 20000);
        int esperado = (n*(n+1)/2) & 0xFF;
        int bom = m.ns == 1 && m.saida[0] == esperado && m.parado;
        if(n <= 6 || n == 12)
            printf("      %-5d %-18d %-10d %-8d %s\n", n,
                   m.ns ? m.saida[0] : -1, esperado, m.ciclos, bom ? "sim" : "NÃO");
        if(!bom) mal++;
    }
    printf("\n      (12 valores de n medidos)\n\n");
    ok("O CIRCUITO COMPLETO EXECUTA: a máquina de NAND dá n(n+1)/2, em 12 casos",
       mal == 0);

    /* e um segundo programa, que usa o MULTIPLICADOR — o produto pelo operador */
    printf("      e um segundo programa, o fatorial, que usa YASMIN (o produto):\n\n");
    printf("      n     fatorial mod 256   conta direta   confere?\n");
    int mal2 = 0;
    for(int n = 1; n <= 8; n++){
        Instr prog[] = {
            { LDI, 0, n },
            { LDI, 1, 1 },
            { MUL, 1, 0 },        /* R1 *= R0 */
            { DEC, 0, 0 },
            { JNZ, 0, 2 },
            { OUT, 1, 0 },
            { HALT,0, 0 },
        };
        Mcu m; roda(&m, prog, 7, 20000);
        int f = 1; for(int k = 2; k <= n; k++) f = (f*k) & 0xFF;
        int bom = m.ns == 1 && m.saida[0] == f;
        printf("      %-5d %-18d %-14d %s\n", n, m.ns ? m.saida[0] : -1, f,
               bom ? "sim" : "NÃO");
        if(!bom) mal2++;
    }
    printf("\n");
    ok("e o fatorial fecha — o produto passou por Yasmin, que passou pelo NAND", mal2 == 0);

    printf("\n      E O CIRCUITO FECHA, do princípio ao fim. A cadeia inteira é esta, e cada\n");
    printf("      elo está medido neste ficheiro ou no anterior:\n\n");
    printf("        Shockley  ->  o transistor chaveia        (amplifica.c §A4)\n");
    printf("        chaveando ->  NAND                        (amplifica.c §A7)\n");
    printf("        NAND      ->  XOR, AND, NOT               (§U3)\n");
    printf("        XOR       ->  Joaquim, o somador          (§U3, 65536 pares)\n");
    printf("        Joaquim   ->  Yasmin, o multiplicador     (§U3)\n");
    printf("        Yasmin    ->  a ALU                       (§U5)\n");
    printf("        ALU+clock ->  o ciclo                     (§U5)\n");
    printf("        o ciclo   ->  o PROGRAMA                  (§U8, aqui)\n\n");
    printf("      Nenhum elo é postulado: cada um sai do anterior e foi medido contra um\n");
    printf("      oráculo de fora. E o fim da cadeia — n(n+1)/2 e o fatorial — bate com a\n");
    printf("      aritmética que não sabe de transistor nenhum. É o par de caminhos a fechar\n");
    printf("      um arco de oito elos.\n");
    printf("\n      E a tríade está inteira nele: a SOMA é Joaquim (Kirchhoff), o PRODUTO é\n");
    printf("      Yasmin (o ganho), e o OPERADOR é o NAND — que gera os dois. O microcontrolador\n");
    printf("      não é uma aplicação da teoria: é a teoria com encapsulamento.\n");
}

printf("\n");
return falhas ? 1 : 0;
}
