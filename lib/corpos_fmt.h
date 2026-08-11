/* corpos_fmt.h — AS ASSINATURAS DOS CORPOS DE FORMATO, NUMA FONTE SÓ.
 *
 * (Nasceu como lib/corpos.h em 07/08 — e esse nome já era do TOOLKIT, incluído por 39
 *  medidores que deixaram de compilar em silêncio: a assinatura da bateria cobre o .c e
 *  não os headers, e os atestados ficaram verdes sobre fontes que não compilavam. O
 *  toolkit voltou ao nome corpos.h; este ficou com o nome que diz o que ele é.)
 *
 * Nasceu de um defeito de ORDEM, e o defeito é instrutivo: o `converte.c` lê do banco a
 * assinatura de `corpo/latex` e `corpo/pdf` para mostrar que a composição fecha porque as
 * duas voltas já fechavam. Só que quem PÕE essas assinaturas no banco são o `latex_corpo.c`
 * e o `pdf_corpo.c` — e a bateria corre por ordem alfabética, onde `converte` vem antes.
 * Resultado: leu (0,0,0) e a asserção acusou, com razão.
 *
 * A saída errada seria o `converte.c` escrever as assinaturas ele próprio: passava a haver
 * DUAS FONTES para o mesmo facto, e duas fontes divergem — é o que o §F2 da fonte no banco
 * já diz. A saída errada nº2 seria escrever os números à mão na asserção, que é a referência
 * escrita à mão: mudava-se a assinatura num sítio e o medidor continuava verde.
 *
 * Fica UMA fonte, aqui, e três leitores. Quem grava, grava daqui; quem confere, confere
 * daqui. Mudar uma assinatura é mudar uma linha, e os três medidores acompanham sozinhos.
 *
 *   corpo/pdf     (1,1,0)   MOVE(pdf, sentido)     a volta é byte a byte: não sobra invariante
 *   corpo/latex   (1,1,1)   MOVE(latex, sentido)   o texto atravessa (o 0); a forma muda
 *
 * E o SENTIDO é argumento nos dois, pela Lei 1: 1† = −1, logo gerar† = ler e compor† =
 * descompor. Não são duas operações — é uma com o sinal trocado.
 */
#ifndef CORPOS_FMT_H
#define CORPOS_FMT_H
#include <stdio.h>
#include <string.h>
#include "banco.h"

struct corpo_fmt { const char *chave; const char *faz; long p, q, r; };

static const struct corpo_fmt CORPOS_FMT[] = {
    { "corpo/pdf",   "MOVE(pdf, sentido)",   1, 1, 0 },
    { "corpo/latex", "MOVE(latex, sentido)", 1, 1, 1 },
};
#define N_CORPOS_FMT ((long)(sizeof CORPOS_FMT / sizeof CORPOS_FMT[0]))

/* grava os corpos no banco a partir DESTA tabela; devolve quantos ficaram sem resíduo */
static long corpos_gravar(struct base *b)
{
    long ok_n = 0;
    unsigned char v[128], out[VMAX];
    for(long i = 0; i < N_CORPOS_FMT; i++){
        const struct corpo_fmt *c = &CORPOS_FMT[i];
        long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s", c->p, c->q, c->r, c->faz);
        if(!gravar(b, c->chave, v, m)) continue;
        long k = ler(b, c->chave, out, sizeof out);
        if(k == m && memcmp(out, v, (size_t)m) == 0) ok_n++;
    }
    return ok_n;
}

/* lê a assinatura de um corpo do banco; 1 se leu, 0 se não está lá */
static int corpos_ler(struct base *b, const char *chave, long *p, long *q, long *r)
{
    unsigned char v[128];
    long k = ler(b, chave, v, sizeof v - 1);
    if(k <= 0) return 0;
    v[k] = 0;
    return sscanf((char*)v, "%ld,%ld,%ld", p, q, r) == 3;
}

/* a assinatura que ESTA tabela declara — para conferir a leitura contra a fonte, e não
 * contra números escritos na asserção */
static const struct corpo_fmt *corpos_tabela(const char *chave)
{
    for(long i = 0; i < N_CORPOS_FMT; i++)
        if(strcmp(CORPOS_FMT[i].chave, chave) == 0) return &CORPOS_FMT[i];
    return NULL;
}
#endif
