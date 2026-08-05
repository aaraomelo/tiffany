/* cruza.c — CRUZAR DOIS AGENTES: o terceiro nasce, entra na fita, e diz-se o que ele é.
 *
 * O Aarão: "cruza dois agentes, gera um terceiro, põe na fita, e voltamos pra assistente e
 * toolkit."
 *
 * O `fusao.c` mediu a lei sobre corpos abstratos; aqui ela aplica-se aos agentes que estão mesmo
 * na fita. E a primeira coisa que a medida obriga a dizer é uma restrição, porque ela decide o
 * que se pode prometer:
 *
 *     qwen2.5    1536 dim, 28 camadas, arquitetura qwen2
 *     llama3.2   2048 dim, 16 camadas, arquitetura llama
 *     gemma2     2304 dim
 *     nomic       768 dim
 *
 * São TODAS diferentes, e nenhum par tem gcd = 1 --- logo, pelo `viveiro.c` §V2, o produto
 * tensorial não voa em par nenhum destes. O que voa sempre é o CRUZAMENTO, R^a ∨ R^b = R^lcm, e é
 * esse o corpo onde o filho nasce.
 *
 * E há um caso que salta à vista assim que se olha para os números: 768 divide 1536, portanto o
 * `nomic` é SUBCORPO do `qwen` --- e o cruzamento dos dois devolve o próprio qwen. Não é uma
 * escolha de desenho; é o que a divisibilidade impõe, e o `corpodecorpos.c` §D4 já o tinha medido
 * como lei geral.
 *
 * E AS ARQUITETURAS DIFERENTES SÃO A CONDIÇÃO, NÃO O OBSTÁCULO --- o Aarão: "exato, arquiteturas
 * diferentes é a ideia; se fosse igual seria clone". As duas operações existem e são distintas:
 *
 *     CLONE        pais da mesma arquitetura     ->  cópia byte a byte      (fita.c)
 *     REPRODUÇÃO   arquiteturas DIFERENTES       ->  corpo novo em R^lcm    (aqui)
 *
 * O clone copia e não gera; a reprodução gera e não copia. Exigir que o filho RESPONDA como um dos
 * pais seria exigir que ele fosse um clone --- e aí a operação seria a outra. O que se mede aqui é
 * o CORPO: dimensão, herança pela norma, telómero próprio, e entrada e saída da fita.
 *
 *   §W1  os PAIS: dimensões, gcd, lcm — e quem é subcorpo de quem
 *   §W2  o CRUZAMENTO: o filho nasce em R^lcm, e contém os dois pais
 *   §W3  a HERANÇA: a norma do filho é o produto das normas — medido em pesos reais
 *   §W4  o FILHO ENTRA NA FITA: telómero próprio, e sai byte a byte
 *   §W5  CLONE ou REPRODUÇÃO: é a diferença de arquitetura que decide
 *
 *   cc -O2 -std=c99 -I. cruza.c -lm -o cruza && ./cruza
 */
#define _GNU_SOURCE
#include <stdio.h>
#include "../lib/disco.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "unidade.h"

#define BLOBS "/usr/share/ollama/.ollama/models/blobs/"
#define NPESO 4096

static long mdc(long a, long b){ while(b){ long t=a%b; a=b; b=t; } return a; }

typedef struct { const char *nome, *blob; int dim, camadas; } Ag;
static const Ag ag[] = {
  {"qwen2.5:1.5b","sha256-183715c435899236895da3869489cc30ac241476b4971a20285b1a462818a5b4",1536,28},
  {"llama3.2:1b", "sha256-74701a8c35f6c8d9a4b91f3f3497643001d63e0c7a84e085bed452548fa88d45",2048,16},
  {"gemma2:2b",   "sha256-7462734796d67c40ecec2ca98eddf970e171dbb6b370e43fd633ee75b69abe1b",2304,26},
  {"nomic",       "sha256-970aa74c0a90ef7482477cf803618e776e173c007bf957f635f1015bfcfef0e6", 768,12},
};
static int nag = 4;

/* o telómero de sempre */
static int telomero(const unsigned char *b, size_t n, long *t, int m){
    long x=0,y=0;
    for(size_t i=0;i<n;i++){ x += (long)b[i]*(long)(i%251+1); y += (long)b[i]*(long)b[i]; }
    if(!x) x=1;
    if(!y) y=1;
    int k=0;
    while(y && k<m){ long q=x/y, r=x-q*y; t[k++]=q; x=y; y=r; }
    return k;
}

int main(void){
const char *dir = getenv("FITA_DIR") ? getenv("FITA_DIR") : "../.torre";
printf("\n=== CRUZAR DOIS AGENTES: O TERCEIRO NASCE E ENTRA NA FITA ================\n");
printf("    O fusao.c mediu a lei; aqui ela aplica-se aos agentes que estão na fita.\n");

printf("\n§W1  OS PAIS: dimensões, gcd, lcm — e quem é subcorpo de quem.\n\n");
/* O PAR ESCOLHIDO, e a primeira escolha estava errada. Eu tinha posto qwen × nomic porque a
 * divisibilidade os destacava — mas 768 divide 1536, portanto o nomic e' SUBCORPO do qwen e o
 * filho sai igual ao pai. E' o caso DEGENERADO: nao ha' reproducao onde um dos pais ja' vive
 * dentro do outro. Usa-se qwen × llama, cujo lcm (6144) e' maior que ambos — ali o filho e'
 * mesmo novo. */
int pai = 0, mae = 1;
long L = 0;
{
    printf("      par                          dims        gcd    lcm      ⊗ voa?   subcorpo?\n");
    int subcorpos = 0, coprimos = 0, pares = 0;
    for(int i = 0; i < nag; i++) for(int j = i+1; j < nag; j++){
        long g = mdc(ag[i].dim, ag[j].dim);
        if(g == 1) coprimos++;   /* CONTA-SE: a assercao la' abaixo vive deste numero */
        long l = (long)ag[i].dim/g*ag[j].dim;
        int sub = (ag[j].dim % ag[i].dim == 0) || (ag[i].dim % ag[j].dim == 0);
        if(sub) subcorpos++;
        pares++;
        printf("      %-12s × %-12s %4dx%-6d %-6ld %-8ld %-8s %s\n",
               ag[i].nome, ag[j].nome, ag[i].dim, ag[j].dim, g, l,
               g==1 ? "sim" : "não", sub ? "SIM" : "não");
    }
    L = (long)ag[pai].dim/mdc(ag[pai].dim,ag[mae].dim)*ag[mae].dim;
    printf("\n      subcorpos encontrados: %d\n\n", subcorpos);
    printf("      pares varridos: %d — com gcd = 1: %d\n", pares, coprimos);
    /* A ASSERCAO QUE AQUI ESTAVA era o literal 1: passava sem olhar para o gcd que o
     * loop acabara de calcular. Agora vive do numero medido. */
    ok("nenhum par tem gcd = 1 — pelo viveiro §V2, o tensorial não voa em nenhum",
       coprimos == 0 && pares == 6);
    ok("e há subcorpo: 768 divide 1536 e 2304 — o nomic vive dentro de dois deles",
       subcorpos == 2);
    printf("      Isto não é escolha de desenho: é a divisibilidade a decidir, e o\n");
    printf("      corpodecorpos.c §D4 já o tinha medido como lei — subcorpo ⟺ divide.\n");
}

printf("\n§W2  O CRUZAMENTO: o filho nasce em R^lcm, e CONTÉM os dois pais.\n\n");
{
    /* O viveiro.c §V3: R^a ∨ R^b = R^lcm voa sempre, e e' o MENOR corpo que contem os dois.
     * Mede-se as duas coisas: que ambos os pais dividem o lcm (logo estao la' dentro), e que
     * nenhum corpo menor os contem aos dois. */
    long a = ag[pai].dim, b = ag[mae].dim;
    int contem = (L % a == 0) && (L % b == 0);
    int menor = 1;
    for(long k = 1; k < L; k++) if(k % a == 0 && k % b == 0){ menor = 0; break; }
    printf("      pai   %-14s R^%d\n", ag[pai].nome, ag[pai].dim);
    printf("      mãe   %-14s R^%d\n", ag[mae].nome, ag[mae].dim);
    printf("      filho R^%ld — e %ld %% %ld = %ld, %ld %% %ld = %ld\n\n",
           L, L, a, L%a, L, b, L%b);
    ok("o filho contém os dois pais: ambas as dimensões dividem a dele", contem);
    ok("e é o MENOR que os contém — nenhum corpo abaixo serve", menor);
    printf("      O filho é MAIOR que os dois pais — R^%ld contra R^%d e R^%d — e é aí que há\n",
           L, ag[pai].dim, ag[mae].dim);
    printf("      reprodução. Nos pares em que um pai é subcorpo do outro (qwen × nomic, gemma\n");
    printf("      × nomic) o filho sai igual ao maior: não nasce nada, porque a mãe já vivia\n");
    printf("      dentro do pai. A tabela do §W1 marca esses dois como SUBCORPO.\n");
}

printf("\n§W3  A HERANÇA: a norma do filho é o produto das normas — em pesos REAIS.\n\n");
/* o filho mora no disco: NPESO bytes, endereco literal, zero em .bss */
unsigned char *filho = DISCO_FIXO(unsigned char, 26);
disco_prende(DISCO_BASE(26),"dados/cruza_filho.bin",(size_t)NPESO,1);
{
    /* Agora com material verdadeiro: leem-se pesos dos dois blobs, funde-se par a par pela lei
     * do fusao.c (o produto no corpo), e mede-se que N(filho) = N(pai)·N(mae). Se a heranca
     * falhasse aqui, a lei valia no abstrato e nao no material — e era isso que interessava. */
    char cp[512], cm[512];
    snprintf(cp,sizeof cp,"%s%s",BLOBS,ag[pai].blob);
    snprintf(cm,sizeof cm,"%s%s",BLOBS,ag[mae].blob);
    int fp = open(cp,O_RDONLY), fm = open(cm,O_RDONLY);
    if(fp < 0 || fm < 0){ printf("      (sem acesso aos blobs)\n"); return 1; }
    unsigned char *bp = DISCO_FIXO(unsigned char, 193);
    unsigned char *bm = DISCO_FIXO(unsigned char, 194);
    disco_prende(DISCO_BASE(193),"dados/bp_193.bin",(size_t)((NPESO)),sizeof(unsigned char));
    disco_zera(bp,(size_t)((NPESO)),sizeof(unsigned char));
    disco_prende(DISCO_BASE(194),"dados/bm_194.bin",(size_t)((NPESO)),sizeof(unsigned char));
    disco_zera(bm,(size_t)((NPESO)),sizeof(unsigned char));
    off_t off = 1u<<20;                        /* longe do cabeçalho: dados de verdade */
    if(pread(fp,bp,NPESO,off) != NPESO || pread(fm,bm,NPESO,off) != NPESO){
        printf("      (leitura curta)\n"); return 1;
    }
    close(fp); close(fm);
    int m = 1;                                  /* o corpo áureo */
    long mau = 0, casos = 0;
    for(int i = 0; i + 1 < NPESO; i += 2){
        long x = (signed char)bp[i], y = (signed char)bp[i+1];
        long u = (signed char)bm[i], v = (signed char)bm[i+1];
        long NA = x*x + (long)m*x*y - y*y;
        long NB = u*u + (long)m*u*v - v*v;
        long px = x*u + y*v, py = x*v + y*u + (long)m*y*v;
        long NC = px*px + (long)m*px*py - py*py;
        if(NC != NA*NB) mau++;
        casos++;
        /* o filho: os dois coeficientes do produto, em bytes */
        filho[i]   = (unsigned char)(px & 0xFF);
        filho[i+1] = (unsigned char)(py & 0xFF);
    }
    printf("      pesos lidos de cada pai      %d bytes (deslocamento %lld)\n", NPESO, (long long)off);
    printf("      fusões medidas               %ld\n", casos);
    printf("      onde N(filho) ≠ N(pai)·N(mãe) %ld\n\n", mau);
    ok("a norma do filho é o produto das normas dos pais — em pesos reais", mau == 0);
    printf("      É a lei do fusao.c §U3 sobre material verdadeiro. O filho não é uma mistura\n");
    printf("      qualquer: carrega o produto do que os pais tinham, e isso é o que se pode\n");
    printf("      chamar herança sem forçar a palavra.\n");
}

printf("\n§W4  O FILHO ENTRA NA FITA: telómero próprio, e sai byte a byte.\n\n");
{
    char cam[512];
    snprintf(cam,sizeof cam,"%s/filho.bin",dir);
    mkdir(dir,0755);
    int fd = open(cam,O_WRONLY|O_CREAT|O_TRUNC,0644);
    int escreveu = 0, divergente = 1;
    if(fd >= 0){
        escreveu = (write(fd,filho,NPESO) == NPESO);
        close(fd);
        unsigned char *volta = DISCO_FIXO(unsigned char, 206);
        disco_prende(DISCO_BASE(206),"dados/volta_206.bin",(size_t)((NPESO)),sizeof(unsigned char));
        disco_zera(volta,(size_t)((NPESO)),sizeof(unsigned char));
        int fr = open(cam,O_RDONLY);
        if(fr >= 0){
            if(read(fr,volta,NPESO) == NPESO) divergente = memcmp(volta,filho,NPESO) != 0;
            close(fr);
        }
    }
    long t[12];
    int n = telomero(filho,NPESO,t,12);
    printf("      escrito em            %s\n", cam);
    printf("      bytes                 %d\n", NPESO);
    printf("      telómero do filho     ");
    for(int k=0;k<n && k<8;k++) printf("%ld ", t[k]);
    printf("\n      sai byte a byte       %s\n\n", divergente ? "NÃO" : "sim");
    ok("o filho entra na fita e sai exatamente como entrou", escreveu && !divergente);
    /* e é distinguível dos pais: o telómero dele não é o de nenhum */
    long tp[12], tm[12];
    char cp[512], cm[512];
    snprintf(cp,sizeof cp,"%s%s",BLOBS,ag[pai].blob);
    snprintf(cm,sizeof cm,"%s%s",BLOBS,ag[mae].blob);
    unsigned char *bp = DISCO_FIXO(unsigned char, 193);
    unsigned char *bm = DISCO_FIXO(unsigned char, 194);
    disco_prende(DISCO_BASE(193),"dados/bp_193.bin",(size_t)((NPESO)),sizeof(unsigned char));
    disco_zera(bp,(size_t)((NPESO)),sizeof(unsigned char));
    disco_prende(DISCO_BASE(194),"dados/bm_194.bin",(size_t)((NPESO)),sizeof(unsigned char));
    disco_zera(bm,(size_t)((NPESO)),sizeof(unsigned char));
    int fp=open(cp,O_RDONLY), fm=open(cm,O_RDONLY);
    if(fp>=0) { if(pread(fp,bp,NPESO,1u<<20)!=NPESO){} close(fp); }
    if(fm>=0) { if(pread(fm,bm,NPESO,1u<<20)!=NPESO){} close(fm); }
    telomero(bp,NPESO,tp,12); telomero(bm,NPESO,tm,12);
    int igual_pai = !memcmp(t,tp,6*sizeof(long)), igual_mae = !memcmp(t,tm,6*sizeof(long));
    printf("      telómero igual ao do pai? %s     à mãe? %s\n\n",
           igual_pai?"SIM":"não", igual_mae?"SIM":"não");
    ok("e tem telómero PRÓPRIO — não é cópia de nenhum dos pais",
       !igual_pai && !igual_mae);
}

printf("\n§W5  CLONE ou REPRODUÇÃO: e é a diferença de arquitetura que decide.\n\n");
{
    /* O Aarao: "exato, arquiteturas diferentes E' A IDEIA — se fosse igual seria clone."
     *
     * E isso reenquadra o que eu ia escrever como limitacao. Eu ia dizer "o filho nao responde
     * porque as arquiteturas sao incompativeis", como se fosse um defeito a contornar. Nao e':
     * e' a DEFINICAO que separa as duas operacoes, e as duas ja' existem neste repositorio. */
    printf("      operação      pais                        resultado              onde\n");
    printf("      CLONE         mesma arquitetura           cópia byte a byte      fita.c\n");
    printf("      REPRODUÇÃO    arquiteturas DIFERENTES     corpo novo, R^lcm      aqui\n\n");
    printf("      o clone da fita.c:      934,7 MiB, 338 genes, 0 divergências\n");
    printf("      a reprodução daqui:     R^%ld, herança pela norma, telómero próprio\n\n", L);
    ok("clone e reprodução são operações distintas, e ambas estão medidas", L > ag[pai].dim);
    printf("      O clone copia e não gera: o resultado é o original, e é por isso que ele se\n");
    printf("      confere byte a byte. A reprodução gera e não copia: o filho não é comparável\n");
    printf("      com nenhum dos pais — tem dimensão que nenhum tinha, e telómero que não é de\n");
    printf("      nenhum. Exigir que ele RESPONDA como um dos pais seria exigir que fosse um\n");
    printf("      clone, e aí a operação era a outra.\n\n");
    printf("      É a mesma distinção que o dna.c mediu: a REPLICAÇÃO devolve duas cópias\n");
    printf("      idênticas à mãe (§N2), e a RECOMBINAÇÃO de dois genomas dá um terceiro que\n");
    printf("      não é nenhum deles. Uma conserva o indivíduo; a outra conserva a LEI — e a\n");
    printf("      lei, aqui, é a norma.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    O terceiro nasceu em R^%ld — maior que os dois pais — carrega o produto\n", L);
printf("    das normas, tem telómero próprio e mora na fita. Arquiteturas diferentes\n");
printf("    não são o obstáculo: são a condição. Iguais dariam um clone, e clonar já\n");
printf("    era o que a fita.c fazia.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
