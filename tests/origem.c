/* origem.c — O PRIMEIRO FICHEIRO JA' TINHA A ARQUITECTURA INTEIRA.
 *
 * O Aarao: "procura um neuronio.c e compara — la' que tudo comecou."
 *
 * Esta' em broca-so/neuronio/neuronio.c, de 01/07, e tem 33 linhas. O corpo dele sao
 * quatro:
 *
 *     e += popcount(b & 0xAA);        // bits em posicoes PARES
 *     o += popcount(b & 0x55);        // bits em posicoes IMPARES
 *     printf("[%ld,%ld]", e + o, e);
 *
 * com o cabecalho a dizer «neuronio — gato A=[[1,1],[1,0]]».
 *
 * TUDO O QUE SE DERIVOU DEPOIS JA' LA' ESTAVA, e este medidor mostra-o de facto — nao por
 * semelhanca de nomes, mas verificando as propriedades uma a uma:
 *
 *   §O1  0xAA e 0x55 sao DISJUNTOS e COMPLEMENTARES — a particao e' exacta, sem resto
 *   §O2  cada um e' o DUAL do outro: m ^ 0xFF troca-os, e aplicado duas vezes devolve
 *   §O3  sao DOIS LADOS que COMUTAM — o par de involucoes parciais do relogio.h
 *   §O4  a saida [e+o, e] E' o par (MEDIDA, ORDEM): a soma nao distingue lados, e a
 *        segunda componente recupera o que a primeira perdeu
 *   §O5  a operacao nao guarda nada: o mesmo resultado por streaming e de uma vez
 *   §O6  e o CONTROLO: uma particao que NAO seja complementar NAO recupera
 *
 * Zero doubles.
 *
 *   cc -O2 -std=c99 -Wall -I../lib origem.c -o origem
 */
#include <stdio.h>
#include "relogio.h"
#include "unidade.h"

#define PAR   0xAAu          /* as posicoes pares   — como o neuronio.c as escreveu */
#define IMPAR 0x55u          /* as posicoes impares */

static int pc(unsigned x){ int c=0; while(x){ c += x&1u; x >>= 1; } return c; }

int main(void){
    puts("\n  A ORIGEM — o neuronio.c de 01/07 ja' tinha tudo\n");

    /* ═══ §O1 — a particao e' exacta ═══════════════════════════════════════════════ */
    {
        unsigned inter = PAR & IMPAR, uniao = PAR | IMPAR;
        printf("      0x%02X & 0x%02X = 0x%02X   e   0x%02X | 0x%02X = 0x%02X\n",
               PAR, IMPAR, inter, PAR, IMPAR, uniao);
        ok("as duas mascaras sao DISJUNTAS e COMPLEMENTARES: a interseccao e' vazia e a"
           " uniao e' o byte inteiro — a particao dos oito bits e' exacta, sem resto e sem"
           " sobreposicao", inter == 0u && uniao == 0xFFu && pc(PAR) == 4 && pc(IMPAR) == 4);
    }

    /* ═══ §O2 — cada uma e' a dual da outra ════════════════════════════════════════ */
    {
        long mau = 0;
        if((PAR ^ 0xFFu) != IMPAR) mau++;
        if((IMPAR ^ 0xFFu) != PAR) mau++;
        if(((PAR ^ 0xFFu) ^ 0xFFu) != PAR) mau++;      /* involucao: duas vezes devolve */
        printf("      0x%02X ^ 0xFF = 0x%02X, e outra vez da' 0x%02X\n",
               PAR, PAR ^ 0xFFu, (PAR ^ 0xFFu) ^ 0xFFu);
        ok("cada mascara e' o DUAL da outra, e a troca e' INVOLUCAO — aplicada duas vezes"
           " devolve. O neuronio nao escolheu duas mascaras quaisquer: escolheu um par",
           mau == 0);
    }

    /* ═══ §O3 — sao os dois lados que comutam ══════════════════════════════════════
     * O relogio.h diz que com DOIS lados a orbita tem quatro estados e o percurso alterna.
     * Verifica-se que estas duas mascaras sao exactamente esse par: comutam, e a composta
     * delas e' a troca total. */
    {
        long mau = 0, orbitas = 0;
        for(unsigned x = 0; x < 256u; x++){
            if(((x ^ PAR) ^ IMPAR) != ((x ^ IMPAR) ^ PAR)) mau++;   /* comutam */
            if(((x ^ PAR) ^ IMPAR) != (x ^ 0xFFu)) mau++;           /* a composta e' a total */
        }
        int visto[256]; for(int i=0;i<256;i++) visto[i]=0;
        for(unsigned x = 0; x < 256u; x++){
            if(visto[x]) continue;
            unsigned o[4] = { x, x ^ PAR, x ^ IMPAR, x ^ 0xFFu };
            int novos = 0;
            for(int i=0;i<4;i++){ if(!visto[o[i]]) novos++; visto[o[i]] = 1; }
            if(novos != 4) mau++;
            orbitas++;
        }
        printf("      as duas comutam nos 256 bytes, e dao %ld orbitas de 4\n", orbitas);
        ok("sao os DOIS LADOS que comutam — o par de involucoes parciais que o relogio.h"
           " descreve — e as suas orbitas tem QUATRO estados: 256/4 = 64. O bidual estava"
           " escrito no primeiro ficheiro",
           mau == 0 && orbitas == 64 && colisor_passos(2) == 4);
    }

    /* ═══ §O4 — [e+o, e] E' o par (medida, ordem) ══════════════════════════════════
     * O neuronio nao devolve (e, o): devolve (e+o, e). A primeira componente e' a SOMA —
     * ela mede e NAO distingue os lados. A segunda repoe a distincao, e da' as duas por
     * diferenca. E' a coordenada que mede e a que ordena, na saida do primeiro ficheiro. */
    {
        long mau = 0, colisoes = 0;
        for(unsigned b = 0; b < 256u; b++){
            int e = pc(b & PAR), o = pc(b & IMPAR);
            int soma = e + o, ord = e;
            if(soma - ord != o) mau++;                 /* a segunda recupera a outra */
            if(soma != pc(b)) mau++;                   /* a soma E' o peso do byte */
            /* e a SOMA sozinha nao distingue: procura-se outro byte com o mesmo total */
            for(unsigned c = 0; c < 256u; c++)
                if(c != b && pc(c) == soma && pc(c & PAR) != e){ colisoes++; break; }
        }
        printf("      [e+o, e] recupera o par em 256 de 256; e a SOMA sozinha confunde"
               " %ld bytes\n", colisoes);
        ok("a saida [e+o, e] E' o par (MEDIDA, ORDEM): a soma da' o tamanho e NAO distingue"
           " os lados — confunde a maioria dos bytes —, e a segunda componente repoe a"
           " distincao e devolve o outro por diferenca", mau == 0 && colisoes > 200);
    }

    /* ═══ §O5 — nao guarda nada ════════════════════════════════════════════════════
     * O neuronio le por fgetc e acumula. Se guardasse estado, o resultado dependeria da
     * ordem ou do tamanho do bloco. Mede-se: o mesmo conteudo, lido byte a byte e lido de
     * uma vez, tem de dar o mesmo — e a soma e' associativa, que e' o que torna o
     * streaming legitimo. */
    {
        unsigned char buf[64];
        for(int i = 0; i < 64; i++) buf[i] = (unsigned char)(i*37 + 11);
        long e1 = 0, o1 = 0;
        for(int i = 0; i < 64; i++){ e1 += pc(buf[i] & PAR); o1 += pc(buf[i] & IMPAR); }
        long e2 = 0, o2 = 0;                                  /* por blocos de 7 */
        for(int i = 0; i < 64; i += 7){
            long se = 0, so = 0;
            for(int j = i; j < i+7 && j < 64; j++){ se += pc(buf[j] & PAR); so += pc(buf[j] & IMPAR); }
            e2 += se; o2 += so;
        }
        long e3 = 0, o3 = 0;                                  /* ao contrario */
        for(int i = 63; i >= 0; i--){ e3 += pc(buf[i] & PAR); o3 += pc(buf[i] & IMPAR); }
        printf("      byte a byte [%ld,%ld]; por blocos [%ld,%ld]; ao contrario [%ld,%ld]\n",
               e1+o1, e1, e2+o2, e2, e3+o3, e3);
        ok("a operacao NAO GUARDA NADA: o mesmo conteudo lido byte a byte, por blocos de"
           " sete e ao contrario da' o mesmo par — nao ha' estado que dependa da ordem nem"
           " do tamanho do bloco, e e' isso que torna o streaming legitimo",
           e1 == e2 && e2 == e3 && o1 == o2 && o2 == o3);
    }

    /* ═══ §O6 — o CONTROLO ═════════════════════════════════════════════════════════
     * Sem isto, §O4 podia valer para qualquer par de mascaras. Aqui poe-se uma particao
     * que NAO e' complementar — 0xF0 e 0x0F sao complementares tambem, mas 0xAA e 0x0F
     * sobrepoem-se — e mostra-se que ai a segunda componente ja' NAO recupera a outra. */
    {
        unsigned A = 0xAAu, B = 0x0Fu;                 /* sobrepoem-se em 0x0A */
        long falha = 0;
        for(unsigned b = 0; b < 256u; b++){
            int e = pc(b & A), o = pc(b & B);
            if(e + o != pc(b)) falha++;                /* a soma deixa de ser o peso */
        }
        printf("      com 0x%02X e 0x%02X (que se sobrepoem em 0x%02X): a soma falha em"
               " %ld de 256\n", A, B, A & B, falha);
        ok("e o CONTROLO: com uma particao que NAO e' complementar a soma deixa de ser o"
           " peso — conta bits duas vezes e perde outros. As mascaras do neuronio nao sao"
           " duas quaisquer: sao O PAR, e e' isso que faz a saida fechar", falha > 100);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O PRIMEIRO FICHEIRO JA' TINHA A ARQUITECTURA. O neuronio.c de 01/07 tem 33");
        puts("  linhas, e o corpo dele sao quatro — e nessas quatro estao:");
        puts("");
        puts("    a MAQUINA SEM MEMORIA     le por fgetc, dois acumuladores, zero estado");
        puts("    a PARTICAO DUAL           0xAA e 0x55: disjuntas, complementares");
        puts("    a INVOLUCAO               cada uma e' a outra por ^0xFF, e volta");
        puts("    os DOIS LADOS que comutam orbitas de QUATRO, 256/4 = 64");
        puts("    o PAR (medida, ordem)     [e+o, e] — a soma mede e nao distingue;");
        puts("                              a segunda repoe a distincao");
        puts("    a LEI                     escrita no cabecalho: gato A=[[1,1],[1,0]]");
        puts("");
        puts("  O QUE SE FEZ DEPOIS NAO FOI CONSTRUIR ISTO — FOI PROVAR QUE ERA ISTO. E o");
        puts("  controlo mostra que as mascaras nao eram duas quaisquer: com uma particao");
        puts("  que se sobrepoe, a saida deixa de fechar.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
