/* dente.c — POR QUE o sistema Σg(w_en)=λ·z_pt é inconsistente (ancora.c, §A2).
 *
 * A colheita provou que não existe g nenhum. Aqui se separa a CAUSA em três, porque elas pedem
 * consertos diferentes:
 *
 *  (1) a PARÁFRASE — duas frases PT distintas com a MESMA frase EN. O lado esquerdo é o mesmo
 *      número e o alvo λz_pt é outro: contradição imediata, e a culpa é do hash arbitrário do PT,
 *      que dá pontos diferentes a coisas que significam o mesmo.
 *  (2) a ORDEM — duas frases EN com as mesmas palavras em ordem diferente (anagramas). A soma é
 *      comutativa: para ela, as duas são a mesma frase. Se os PT diferem, contradição — e a culpa
 *      é do Σ, que apaga o caminho.
 *  (3) a CONTAGEM — mesmo sem (1) e (2), há E equações para V incógnitas. Se E>V, um sistema com
 *      alvos arbitrários é inconsistente por falta de grau de liberdade, e nenhum conserto de
 *      léxico salva: o que falta é liberdade, não sorte.
 *
 * O lado esquerdo de uma equação depende SÓ do multiconjunto de palavras EN (a soma é comutativa).
 * A sua assinatura é então sig = Σ marca(w) — comutativa como ele. Duas equações com a mesma sig
 * têm o mesmo lado esquerdo; se os alvos diferem, elas se contradizem. E dentro do grupo, olhando
 * a frase EN literal (ordem preservada), (1) se separa de (2).
 *
 * SEM MEMÓRIA: duas etapas em streaming, e o agrupamento é feito pelo `sort` externo (disco), não
 * por tabela. Estado em RAM O(1). As palavras distintas contam-se em arquivos de marcas (o mesmo
 * princípio do léxico de ancora.c: o nó é fiação, não RAM).
 *
 *   cc -O2 -std=c99 -I../tools dente.c -o dente
 *   ./dente emite pares.tsv > sig.txt && sort -S 64M sig.txt | ./dente agrupa
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "gp2.h"

#define NSLOT (1L<<20)                                 /* marcas de palavras distintas, em disco   */

static uint64_t marca(const char *s, size_t n){
    uint64_t h = 1469598103934665603UL;
    for(size_t i=0;i<n;i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
static E hpt(const char *s, size_t n){
    uint64_t h1 = marca(s,n), h2 = (h1 ^ 0x9e3779b97f4a7c15UL) * 1099511628211UL;
    E z = { (int)(h1 % (uint64_t)p), (int)(h2 % (uint64_t)p) };
    if(!z.a && !z.b) z.a = 1;
    return z;
}
/* conta palavras distintas: marca em arquivo (open addressing), devolve 1 se era nova */
static int nova_marca(int fd, uint64_t fp){
    long i = (long)(fp % (uint64_t)NSLOT);
    for(long t=0;t<NSLOT;t++){
        uint64_t s;
        if(pread(fd,&s,sizeof s,i*8) != 8) return 0;
        if(s == 0){ pwrite(fd,&fp,sizeof fp,i*8); return 1; }
        if(s == fp) return 0;
        i = (i+1) % NSLOT;
    }
    return 0;
}
static int abre_marcas(const char *nome){
    unlink(nome);
    int fd = open(nome, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd>=0 && ftruncate(fd, NSLOT*8) != 0) return -1;
    return fd;
}
static int primo(long n){ if(n<2)return 0; for(long d=2;d*d<=n;d++) if(n%d==0) return 0; return 1; }

static char line[8192];

/* ---------- etapa 1: cada par vira a assinatura do seu lado esquerdo + o seu alvo ---------- */
static int emite(const char *path){
    FILE *f = fopen(path,"r");
    if(!f){ fprintf(stderr,"sem %s\n", path); return 2; }
    int fden = abre_marcas("marcas_en.bin"), fdpt = abre_marcas("marcas_pt.bin");
    if(fden<0||fdpt<0){ fprintf(stderr,"marcas falharam\n"); return 2; }
    long E_eq=0, V_en=0, V_pt=0;

    while(fgets(line,sizeof line,f)){
        char *tab = strchr(line,'\t'); if(!tab) continue;
        *tab = 0;
        char *en = line, *pt = tab+1;
        char *nl = strchr(pt,'\n'); if(nl) *nl = 0;
        if(!*en || !*pt) continue;

        uint64_t sig = 0, lit = 1469598103934665603UL;   /* sig: comutativa; lit: a frase literal  */
        for(char *w=en; *w; ){
            while(*w==' ') w++;
            char *e=w; while(*e && *e!=' ') e++;
            if(e>w){
                uint64_t fp = marca(w,e-w);
                sig += fp;                               /* soma — comutativa como o lado esquerdo */
                lit = (lit ^ (fp & 0xff)) * 1099511628211UL;
                lit ^= fp + 0x9e3779b97f4a7c15UL + (lit<<6) + (lit>>2);   /* sensível à ORDEM      */
                V_en += nova_marca(fden, fp);
            }
            w = e;
        }
        E z = {0,0};
        for(char *w=pt; *w; ){
            while(*w==' ') w++;
            char *e=w; while(*e && *e!=' ') e++;
            if(e>w){ z = add(z, hpt(w,e-w)); V_pt += nova_marca(fdpt, marca(w,e-w)); }
            w = e;
        }
        E_eq++;
        printf("%016llx\t%05d\t%05d\t%016llx\n",
               (unsigned long long)sig, z.a, z.b, (unsigned long long)lit);
    }
    fclose(f); close(fden); close(fdpt);
    fprintf(stderr, "equações E=%ld ; incógnitas EN V=%ld ; palavras PT=%ld ; E/V=%.2f\n",
            E_eq, V_en, V_pt, V_en?(double)E_eq/V_en:0);
    return 0;
}

/* ---------- etapa 2: as linhas chegam ordenadas — agrupa por assinatura e conta ---------- */
static int agrupa(void){
    long E_eq=0, grupos=0;
    long col_grupos=0, col_eq=0;              /* grupos com ≥2 alvos: a contradição imediata       */
    long par_grupos=0, par_eq=0;              /* (1) paráfrase: frase EN literal idêntica          */
    long ana_grupos=0, ana_eq=0;              /* (2) ordem: mesmas palavras, frase EN diferente    */
    /* estado do grupo corrente — O(1) */
    unsigned long long sig=0, sig0=0, lit=0, lit0=0;
    int a=0,b=0,a0=-1,b0=-1; long ntot=0, nalvos=0; int mesmo_lit=1, aberto=0;

    while(fgets(line,sizeof line,stdin)){
        if(sscanf(line,"%llx\t%d\t%d\t%llx",&sig,&a,&b,&lit)!=4) continue;
        E_eq++;
        if(!aberto || sig!=sig0){
            if(aberto){                                    /* fecha o grupo anterior              */
                grupos++;
                if(nalvos>=2){
                    col_grupos++; col_eq += ntot;
                    if(mesmo_lit){ par_grupos++; par_eq += ntot; }
                    else         { ana_grupos++; ana_eq += ntot; }
                }
            }
            sig0=sig; lit0=lit; a0=a; b0=b; ntot=0; nalvos=0; mesmo_lit=1; aberto=1;
        }
        if(lit != lit0) mesmo_lit = 0;                      /* alguma frase EN em outra ordem      */
        if(a!=a0 || b!=b0){ nalvos++; a0=a; b0=b; }         /* alvos vêm ordenados: distintos       */
        if(ntot==0) nalvos = 1;
        ntot++;
    }
    if(aberto){
        grupos++;
        if(nalvos>=2){
            col_grupos++; col_eq += ntot;
            if(mesmo_lit){ par_grupos++; par_eq += ntot; }
            else         { ana_grupos++; ana_eq += ntot; }
        }
    }

    printf("DENTE — a causa da inconsistência de Σg(w_en)=λ·z_pt\n");
    printf("=================================================================\n");
    printf("equações                                  : %ld\n", E_eq);
    printf("lados esquerdos distintos (multiconjuntos): %ld\n", grupos);
    printf("\ncontradição IMEDIATA (mesmo lado esquerdo, alvos λz_pt distintos):\n");
    printf("  grupos                                  : %ld\n", col_grupos);
    printf("  equações envolvidas                     : %ld/%ld  (%.1f%%)\n",
           col_eq, E_eq, E_eq?100.0*col_eq/E_eq:0);
    printf("    (1) PARÁFRASE  frase EN idêntica      : %ld grupos, %ld eq  (%.1f%%)\n",
           par_grupos, par_eq, E_eq?100.0*par_eq/E_eq:0);
    printf("    (2) ORDEM      mesmas palavras, outra : %ld grupos, %ld eq  (%.1f%%)\n",
           ana_grupos, ana_eq, E_eq?100.0*ana_eq/E_eq:0);
    printf("\n(3) CONTAGEM: as %ld equações restantes têm lado esquerdo único — não se contradizem\n",
           E_eq - col_eq);
    printf("    entre si, mas dividem as mesmas incógnitas. Ver E/V impresso na etapa 1: com E>V\n");
    printf("    o sistema é inconsistente por falta de grau de liberdade, e nenhum conserto de\n");
    printf("    léxico o salva — o que falta é liberdade, não sorte.\n");
    return 0;
}

int main(int argc, char **argv){
    m = 1;
    for(p = 40009; ; p++) if(primo(p) && irred_gp2()) break;
    if(argc>1 && !strcmp(argv[1],"emite")) return emite(argc>2?argv[2]:"pares.tsv");
    if(argc>1 && !strcmp(argv[1],"agrupa")) return agrupa();
    fprintf(stderr,"uso: dente emite pares.tsv > sig.txt ; sort -S 64M sig.txt | dente agrupa\n");
    return 2;
}
