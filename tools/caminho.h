/* caminho.h — A DESCIDA. O endereço de um valor é o seu CAMINHO, e o formato é só a roupa.
 *
 * Nasceu do analisador do stratum, que eu tinha escrito a contar símbolos e que partiu duas vezes
 * — no array da merkle branch e numa aspa escapada. Contar símbolos onde há ESTRUTURA é o erro; a
 * estrutura endereça-se descendo, um termo por nível, que é o que a cifra faz.
 *
 * E o que muda de formato para formato NÃO é a descida: é como cada um MARCA o nível.
 *
 *     JSON       o nível é o parêntese          [ { } ]
 *     YAML       o nível é a INDENTAÇÃO         os espaços à esquerda
 *     Markdown   o nível é a contagem de #      # ## ###
 *
 * A lei é a mesma nos três, e é a do tesseracto: M_k = M_{k-1}·A_1, o nível k carrega o k-1.
 * Trocar de formato é trocar quem lê a marca — não a descida.
 */
#ifndef CAMINHO_H
#define CAMINHO_H
#include <string.h>
#include <stdlib.h>

/* A TRADUÇÃO É DO HIPERCORPO, E NÃO DE VARREDURA DE STRING.
 *
 * Eu escrevi um analisador que contava aspas e ele partiu duas vezes: primeiro com o array da
 * merkle branch (que desloca as posições conforme o número de ramos), depois com uma ASPA
 * ESCAPADA — JSON válido, e devolvia zeros. Remendar terceira vez seria esperar a quarta.
 *
 * JSON é recursão auto-similar: um nível contém cópias de si, que é a lei do tesseracto
 * (`M_k = M_{k-1}A_1`, o nível k carrega o k-1). E a posição de um campo NÃO é uma contagem de
 * símbolos: é o seu CAMINHO — params, depois o 5.o — e caminho é o que a cifra endereça. Logo
 * não se varre: DESCE-SE, um nível por termo, e o aninhamento fica certo por construção porque
 * o aninhamento É a descida.
 *
 * Uma string lê-se num sítio só, e é lá que o escape se respeita — não espalhado por um scanner. */
static const char *js_fim_string(const char *p){        /* p aponta ao " de abertura */
    p++;
    while(*p){
        if(*p == '\\' && p[1]) p += 2;                  /* o escape: dois símbolos, um valor */
        else if(*p == '"') return p;
        else p++;
    }
    return NULL;
}
static const char *js_fim_valor(const char *p){         /* o fim do valor que começa em p */
    if(*p == '"'){ const char *e = js_fim_string(p); return e ? e + 1 : NULL; }
    if(*p == '[' || *p == '{'){
        char ab = *p, fe = (ab == '[') ? ']' : '}';
        int prof = 0;
        while(*p){
            if(*p == '"'){ const char *e = js_fim_string(p); if(!e) return NULL; p = e + 1; continue; }
            if(*p == ab) prof++;
            else if(*p == fe){ prof--; if(!prof) return p + 1; }
            p++;
        }
        return NULL;
    }
    while(*p && *p != ',' && *p != ']' && *p != '}') p++;
    return p;
}
/* Desce um nível: devolve o k-ésimo elemento do container que começa em p. */
static const char *js_desce(const char *p, int k){
    if(*p != '[' && *p != '{') return NULL;
    p++;
    for(int n = 0; *p; n++){
        while(*p == ' ' || *p == '\t') p++;
        if(*p == ']' || *p == '}') return NULL;
        const char *fim = js_fim_valor(p);
        if(!fim) return NULL;
        if(n == k) return p;
        p = fim;
        while(*p == ' ' || *p == '\t') p++;
        if(*p == ',') p++;
    }
    return NULL;
}
/* O CAMINHO: um termo por nível, como a cifra. Devolve a string do fim, sem as aspas. */
static const char *js_caminho(const char *raiz, const int *cam, int n, size_t *len){
    const char *p = raiz;
    for(int i = 0; i < n; i++){
        p = js_desce(p, cam[i]);
        if(!p) return NULL;
    }
    if(*p != '"') return NULL;
    const char *e = js_fim_string(p);
    if(!e) return NULL;
    *len = (size_t)(e - p - 1);
    return p + 1;
}

/* ---- YAML: o nível é a indentação ---- */
static int ya_nivel(const char *l){ int n = 0; while(l[n] == ' ') n++; return n; }
/* o k-ésimo item do nível , a partir de p; devolve a linha */
static const char *ya_desce(const char *p, int ind, int k, size_t *len){
    int n = 0;
    while(*p){
        const char *fim = strchr(p, 10);
        size_t m = fim ? (size_t)(fim - p) : strlen(p);
        int v = ya_nivel(p);
        if(m && v == ind){
            if(n == k){ *len = m; return p; }
            n++;
        } else if(m && v < ind) return NULL;      /* saiu do bloco: acabou o nível */
        if(!fim) break;
        p = fim + 1;
    }
    return NULL;
}
/* ---- Markdown: o nível é a contagem de # ---- */
static int md_nivel(const char *l){ int n = 0; while(l[n] == '#') n++; return n; }
static const char *md_desce(const char *p, int nivel, int k, size_t *len){
    int n = 0;
    while(*p){
        const char *fim = strchr(p, 10);
        size_t m = fim ? (size_t)(fim - p) : strlen(p);
        int v = md_nivel(p);
        if(v == nivel){
            if(n == k){ *len = m; return p; }
            n++;
        } else if(v && v < nivel && n) return NULL;
        if(!fim) break;
        p = fim + 1;
    }
    return NULL;
}
#endif
