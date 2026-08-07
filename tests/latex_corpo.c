/* latex_corpo.c — O \LaTeX É UM CORPO SÓ, E O SENTIDO É UM ARGUMENTO.
 *
 * O Aarão: «sim, agora o mesmo pro latex unificado — depois a conversão via dualsort sai na
 * hora.»
 *
 * O mesmo que o PDF, e pela mesma razão: a Lei 1 é `1† = −1`, logo COMPOR† = DESCOMPOR. Não
 * são duas operações, é UMA com o sinal trocado — e por isso há UMA chave e o sentido é
 * argumento, exactamente como no `MOVE`.
 *
 *      MOVE(latex, −1)   COMPOR      o fonte vira estrutura   — emite
 *      MOVE(latex, +1)   DESCOMPOR   a estrutura vira fonte   — absorve
 *
 * MAS A ASSINATURA É OUTRA, e a diferença é o resultado deste medidor. O PDF é (1,1,0): o
 * par fecha e não sobra nada — a volta é byte a byte, e nada fica de fora. O \LaTeX tem um
 * terceiro estado, e ele NÃO é decorativo: o TEXTO atravessa sem mudar enquanto a FORMA
 * muda. Esse invariante é o `0` do trial, e é o que faz a assinatura ser (1,1,1) — a única
 * com um de cada entre as nove linguagens.
 *
 * E aqui isso não se afirma: MEDE-SE. Compõe-se e descompõe-se, e conta-se o que ficou igual
 * (o texto) e o que mudou (a forma). Se o texto mudasse, o `0` não existia; se a forma não
 * mudasse, compor não fazia nada.
 *
 *   §L1  UM corpo no banco, com o sentido como argumento — não dois
 *   §L2  a LEI 1: compor† = descompor, e a involução fecha — período conta MOVE
 *   §L3  o TERCEIRO ESTADO existe: o texto atravessa (0) enquanto a forma muda
 *   §L4  o controlo: sem a Lei 1 a volta NÃO fecha — e é a lei, não o cuidado
 *
 *   cc -O2 -std=c99 -I../lib latex_corpo.c -o latex_corpo && ./latex_corpo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── COMPOR — MOVE(latex, −1): o fonte vira estrutura. Emite. ────────────────────────
 * A estrutura aqui é a linha composta: o comando vira marca, o texto fica. Não é o tex.c
 * inteiro — é o núcleo dele, o bastante para a volta poder fechar e ser medida. */
static long compor(const char *fonte, char *saida, long cap)
{
    long m = 0;
    for(const char *p = fonte; *p && m < cap - 8; ){
        if(*p == '\\' && isalpha((unsigned char)p[1])){
            const char *ini = ++p;
            while(isalpha((unsigned char)*p)) p++;
            long n = p - ini;
            /* a marca guarda o comando POR INTEIRO — é ela que deixa a volta ser exacta.
             * Guardar só «houve aqui um comando» seria apagar, e apagar não se desfaz. */
            m += snprintf(saida + m, (size_t)(cap - m), "<%.*s>", (int)n, ini);
            if(*p == '{'){ saida[m++] = '{'; p++; }
            continue;
        }
        saida[m++] = *p++;
    }
    saida[m] = 0;
    return m;
}

/* ─── DESCOMPOR — MOVE(latex, +1): a estrutura vira fonte. Absorve. ───────────────────
 * É o inverso EXACTO do de cima, e tem de o ser: se não for, a volta não fecha e a Lei 1
 * não se cumpre. Não é «desfazer com cuidado» — é a mesma passagem lida ao contrário. */
static long descompor(const char *estr, char *saida, long cap)
{
    long m = 0;
    for(const char *p = estr; *p && m < cap - 4; ){
        if(*p == '<'){
            const char *ini = ++p;
            while(*p && *p != '>') p++;
            long n = p - ini;
            if(*p == '>') p++;
            m += snprintf(saida + m, (size_t)(cap - m), "\\%.*s", (int)n, ini);
            continue;
        }
        saida[m++] = *p++;
    }
    saida[m] = 0;
    return m;
}

/* o texto puro: o que sobra tirando os comandos. É o que TEM de atravessar. */
static long texto_puro(const char *s, char *out, long cap)
{
    long m = 0;
    for(const char *p = s; *p && m < cap - 2; ){
        if(*p == '\\' && isalpha((unsigned char)p[1])){
            p++; while(isalpha((unsigned char)*p)) p++;
            continue;
        }
        if(*p == '<'){ while(*p && *p != '>') p++; if(*p) p++; continue; }
        if(*p == '{' || *p == '}'){ p++; continue; }
        out[m++] = *p++;
    }
    out[m] = 0;
    return m;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    const char *casos[] = {
        "\\textbf{a unidade e dual} e o resto e derivacao",
        "a estaca move, e a \\emph{cruz} mede o que nao se moveu",
        "\\section{O corpo estelar} a trindade: branco, estrela, negro",
        "MOVE(slot, sentido): \\code{+1} le e \\code{-1} escreve",
    };
    const long NC = (long)(sizeof casos / sizeof casos[0]);

printf("\n=== O LATEX E UM CORPO SO, E O SENTIDO E UM ARGUMENTO ========================\n");

printf("\n§L1  UM corpo no banco, e o SENTIDO e um argumento — nao dois.\n\n");
    {
        long postas = 0, resid = 0, partidas = 0;
        unsigned char v[96], out[VMAX];
        long m = (long)snprintf((char*)v, sizeof v, "1,1,1|MOVE(latex, sentido)");
        if(gravar(&b, "corpo/latex", v, m)) postas++;
        long k = ler(&b, "corpo/latex", out, sizeof out);
        if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        if(ler(&b, "corpo/latex/compositor", out, sizeof out) > 0) partidas++;
        if(ler(&b, "corpo/latex/leitor",     out, sizeof out) > 0) partidas++;
        printf("      corpo/latex   (1,1,1)   MOVE(latex, sentido)\n");
        printf("      chaves partidas: %ld (tem de ser 0)\n", partidas);
        ok("ha UM corpo e o sentido e ARGUMENTO, como no PDF e pela mesma razao: a Lei 1 diz"
           " 1+ = -1, logo compor+ = descompor — uma operacao com o sinal trocado, e nao duas."
           " Partir em dois seria repetir o erro que o corpo do PDF ja' custou: dois codigos"
           " onde o MOVE diz que e' um codigo e um argumento",
           postas == 1 && resid == 0 && partidas == 0);
    }

printf("\n§L2  A LEI 1: compor+ = descompor, e a involucao FECHA.\n\n");
    long periodo = 0;
    {
        char e[4096], f[4096];
        long difs = 0;
        printf("      fonte                                                    volta?\n");
        for(long i = 0; i < NC; i++){
            compor(casos[i], e, sizeof e);
            descompor(e, f, sizeof f);
            int igual = strcmp(f, casos[i]) == 0;
            if(!igual) difs++;
            printf("      %-56.56s %s\n", casos[i], igual ? "fecha" : "NAO FECHA");
        }
        /* o PERIODO conta MOVE e nao pares — a licao que o corpo do PDF deu: cada par sao
         * dois MOVE, logo o par volta em 1 e a operacao tem periodo 2. */
        char a[4096], bb[4096];
        strcpy(a, casos[0]);
        long pares = 0;
        for(long i = 1; i <= 8; i++){
            compor(a, e, sizeof e);
            descompor(e, bb, sizeof bb);
            strcpy(a, bb);
            if(strcmp(a, casos[0]) == 0 && !pares) pares = i;
        }
        periodo = 2 * pares;
        printf("      volta ao original ao fim de %ld par(es) = %ld MOVE — periodo %ld\n",
               pares, periodo, periodo);
        ok("a LEI 1 fecha nos quatro: compor e descompor sao inversos exactos, e a volta"
           " devolve o fonte. E o periodo conta MOVE e nao pares — cada par sao dois MOVE,"
           " logo o par volta em UM e a operacao tem periodo DOIS, que e' a involucao. Nao ha'"
           " «desfazer com cuidado»: e' a mesma passagem lida ao contrario, e por isso a marca"
           " guarda o comando POR INTEIRO — guardar so' «houve aqui um comando» seria apagar,"
           " e apagar nao se desfaz", difs == 0 && periodo == 2);
    }

printf("\n§L3  O TERCEIRO ESTADO: o TEXTO atravessa (0) enquanto a FORMA muda.\n\n");
    {
        /* E' AQUI QUE A ASSINATURA SE SEPARA DA DO PDF, e nao por decreto. O PDF e' (1,1,0):
         * o par fecha byte a byte e nao sobra invariante nenhum. O LaTeX tem um TERCEIRO
         * estado — o texto que atravessa sem mudar enquanto a forma muda. Mede-se pelas duas
         * metades: o que TEM de ficar igual (o texto) e o que NAO PODE ficar igual (a forma).
         * Se o texto mudasse, o 0 nao existia; se a forma nao mudasse, compor nao fazia nada. */
        char e[4096], t1[4096], t2[4096];
        long texto_igual = 0, forma_mudou = 0;
        printf("      caso   texto atravessa?   forma mudou?\n");
        for(long i = 0; i < NC; i++){
            compor(casos[i], e, sizeof e);
            texto_puro(casos[i], t1, sizeof t1);
            texto_puro(e,       t2, sizeof t2);
            int mesmo = strcmp(t1, t2) == 0;
            int mudou = strcmp(casos[i], e) != 0;
            texto_igual += mesmo; forma_mudou += mudou;
            printf("      %-6ld %-18s %s\n", i + 1, mesmo ? "sim" : "NAO", mudou ? "sim" : "NAO");
        }
        printf("      logo a assinatura tem um terceiro estado: (1,1,1), e nao (1,1,0)\n");
        ok("o TEXTO atravessa nos quatro e a FORMA muda nos quatro — e sao as duas metades da"
           " mesma medida: o residuo que TEM de ser zero (o texto) e o que NAO PODE ser zero"
           " (a forma). E' isto que separa a assinatura do LaTeX da do PDF, e nao um decreto:"
           " o PDF e' (1,1,0) porque a volta e' byte a byte e nao sobra invariante; o LaTeX e'"
           " (1,1,1) porque o texto e' o invariante e a forma e' o que se move. Se o texto"
           " mudasse o 0 nao existia; se a forma nao mudasse, compor nao fazia nada",
           texto_igual == NC && forma_mudou == NC);
    }

printf("\n§L4  O CONTROLO: sem a Lei 1 a volta NAO fecha — e' a lei, nao o cuidado.\n\n");
    {
        /* muda-se o compositor para guardar so' UMA marca em vez do comando inteiro — que e'
         * a tentacao natural e e' APAGAR. A volta tem de partir-se: sem o dual guardado, o
         * passo deixa de se desfazer. */
        long quebrou = 0;
        char e[4096], f[4096];
        for(long i = 0; i < NC; i++){
            /* compor a apagar: o comando vira uma marca sem nome */
            long m = 0;
            for(const char *p = casos[i]; *p && m < (long)sizeof e - 4; ){
                if(*p == '\\' && isalpha((unsigned char)p[1])){
                    p++; while(isalpha((unsigned char)*p)) p++;
                    e[m++] = '<'; e[m++] = '>';           /* o nome PERDEU-SE */
                    if(*p == '{'){ e[m++] = '{'; p++; }
                    continue;
                }
                e[m++] = *p++;
            }
            e[m] = 0;
            descompor(e, f, sizeof f);
            if(strcmp(f, casos[i]) != 0) quebrou++;
        }
        printf("      com o nome do comando apagado, a volta parte-se em %ld de %ld\n", quebrou, NC);
        ok("apagando o NOME do comando — guardar «houve aqui um comando» em vez do comando — a"
           " volta parte-se nos quatro. E' a segunda metade da medida, e diz que o §L2 nao"
           " passa por cuidado meu: passa porque a Lei 1 esta' cumprida. Um passo que apaga"
           " nao tem inverso, e nenhum cuidado o devolve", quebrou == NC);
    }

    fechar(&b);
printf("\n=== O CORPO LATEX ===========================================================\n");
printf("  E' UM corpo, e o sentido e' ARGUMENTO: MOVE(latex, sentido). -1 compoe, +1 descompoe.\n");
printf("  A Lei 1 di-lo antes de qualquer medicao: 1+ = -1, logo compor+ = descompor.\n\n");
printf("  E a ASSINATURA e' outra que a do PDF, por MEDIDA e nao por decreto:\n\n");
printf("    PDF     (1,1,0)   a volta e' byte a byte, e nao sobra invariante nenhum\n");
printf("    LaTeX   (1,1,1)   o TEXTO atravessa (o 0) enquanto a FORMA muda\n\n");
printf("  O terceiro estado nao e' decorativo: e' o invariante. E mede-se pelas duas metades —\n");
printf("  o residuo que TEM de ser zero (o texto) e o que NAO PODE ser zero (a forma).\n\n");
printf("  E o CONTROLO diz que a Lei 1 e' que sustenta a volta: apagado o nome do comando, ela\n");
printf("  parte-se nos quatro. Um passo que apaga nao tem inverso, e nenhum cuidado o devolve.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a volta fecha, e o texto atravessa.\n\n");
    return 0;
}
