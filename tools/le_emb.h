/* le_emb.h — LER UM EMBEDDING EM QUALQUER DOS DOIS FORMATOS.
 *
 * O Aarão perguntou onde é que o float é inevitável, e a resposta é: em lado nenhum. Todo
 * float32 é um racional exato — m·2^e — e cabe nos 32 bits que já são um inteiro. Guardado
 * assim, volta BIT A BIT: não há round-trip decimal, nem casas a discutir.
 *
 * A migração faz-se pelo LEITOR primeiro, para não partir o que já está escrito:
 *
 *     0x3DB851EC     ← os bits, em hexadecimal com prefixo. EXATO.
 *     0.09000000     ← decimal. Lê-se com strtod, como sempre.
 *
 * O prefixo 0x é o critério, e é inequívoco. A alternativa óbvia — "sem ponto nem 'e' é
 * bits" — PARTE-SE: %.17g escreve o valor 1,0 como "1", sem ponto, e isso lido como bits
 * daria 1,4e-45. O prefixo não tem esse buraco.
 *
 * Uso: substituir  strtod(p, &fim)  por  emb_le(p, &fim).
 */
#ifndef LE_EMB_H
#define LE_EMB_H
#include <stdlib.h>
#include <string.h>

static double emb_le(const char *p, char **fim){
    while(*p==' '||*p=='\t') p++;
    int neg = 0;
    const char *q = p;
    if(*q=='-'||*q=='+'){ neg = (*q=='-'); q++; }
    if(q[0]=='0' && (q[1]=='x'||q[1]=='X')){
        /* os BITS de um float32, em hex — reinterpretam-se sem perder um bit */
        unsigned long b = strtoul(q, fim, 16);
        unsigned int u = (unsigned int)b;
        float f;
        memcpy(&f, &u, sizeof f);
        return neg ? -(double)f : (double)f;
    }
    return strtod(p, fim);
}
#endif
