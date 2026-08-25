/* DEPENDE-DE: catalogo.tex enredo.tex teoria.tex papers/arquitetura.tex papers/corpo_analitico.tex
 * O que este medidor LÊ entra na assinatura da bateria — sem isto, mudar um
 * destes ficheiros não reabre a semente, e o verde é sobre um estado que já
 * não existe. Mesma razão dos headers, um andar acima. */
/* fonte_banco.c — A FONTE DO DOCUMENTO ENTRA NO BANCO, E SAI INTEIRA.
 *
 * O Aarao: «o pdf entra no banco e e' renderizado na hora» · «quando clica renderiza em
 * tempo real, NADA PRE-GRAVADO».
 *
 * E' o mesmo que ja' se disse dos GIFs, no sitio onde eu ainda nao o tinha visto: um PDF
 * pre-compilado e' uma GRAVACAO. Esta' ao lado da fonte como o GIF estava ao lado do kernel,
 * e envelhece do mesmo modo — foi exactamente isso que aconteceu esta semana, com dois papers
 * a serem servidos de uma publicacao anterior enquanto o link devolvia 200.
 *
 * Para compor na hora e' preciso que a FONTE seja lida do banco e nao do disco. Este medidor
 * mede a primeira metade disso, que e' a que se pode medir hoje:
 *
 *   §F1  a fonte entra e SAI byte a byte           — MOVE(-1) e MOVE(+1), residuo 0
 *   §F2  e e' UMA fonte, nao duas                  — o banco e o disco nao divergem
 *   §F3  o controlo: um byte trocado ACUSA         — senao a volta fecharia na mesma
 *   §F4  e a fonte e' a do LATEX, que e' a nona linguagem — nao o meio
 *
 * A fonte nao cabe num valor so': o catalogo tem 1,6 MB. Parte-se em BLOCOS, e a chave leva o
 * indice — que e' a mesma arvore por onde tudo o resto desce. Partir nao e' um truque de
 * armazenamento: e' a operacao, e o que a valida e' a volta fechar.
 *
 *   cc -O2 -std=c99 -I../lib fonte_banco.c -o fonte_banco && ./fonte_banco
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE  "/tmp/fonte_banco"
#define BLOCO 2048                      /* cabe com folga no valor, e a chave leva o indice */

/* poe um ficheiro no banco em blocos; devolve quantos blocos escreveu, ou -1 */
static long subir(struct base *b, const char *nome, const char *caminho)
{
    FILE *f = fopen(caminho, "rb");
    if(!f) return -1;
    unsigned char buf[BLOCO];
    long i = 0, n;
    while((n = (long)fread(buf, 1, BLOCO, f)) > 0){
        char chave[160];
        snprintf(chave, sizeof chave, "fonte/%s/%06ld", nome, i);
        if(!gravar(b, chave, buf, n)){ fclose(f); return -1; }
        i++;
    }
    fclose(f);
    /* e o numero de blocos tambem vai para o banco: sem ele a descida nao sabe onde parar,
     * e adivinhar o fim pela leitura falhar e' confundir «acabou» com «partiu-se». */
    char ck[160]; snprintf(ck, sizeof ck, "fonte/%s/blocos", nome);
    char cv[32]; long m = (long)snprintf(cv, sizeof cv, "%ld", i);
    if(!gravar(b, ck, (unsigned char*)cv, m)) return -1;
    return i;
}

/* tira do banco e compara com o disco, byte a byte; devolve o numero de bytes que diferem */
static long descer_e_comparar(struct base *b, const char *nome, const char *caminho,
                              long *lidos_out)
{
    char ck[160]; snprintf(ck, sizeof ck, "fonte/%s/blocos", nome);
    unsigned char v[32]; long k = ler(b, ck, v, sizeof v - 1);
    if(k <= 0) return -1;
    v[k] = 0; long nb = atol((char*)v);

    FILE *f = fopen(caminho, "rb");
    if(!f) return -1;
    long difs = 0, lidos = 0;
    unsigned char doBanco[BLOCO], doDisco[BLOCO];
    for(long i = 0; i < nb; i++){
        char chave[160];
        snprintf(chave, sizeof chave, "fonte/%s/%06ld", nome, i);
        long a = ler(b, chave, doBanco, sizeof doBanco);
        long d = (long)fread(doDisco, 1, BLOCO, f);
        if(a != d){ difs += (a > d ? a - d : d - a); if(a < 0) a = 0; }
        long menor = a < d ? a : d;
        for(long j = 0; j < menor; j++) if(doBanco[j] != doDisco[j]) difs++;
        lidos += a > 0 ? a : 0;
    }
    /* e o disco nao pode ter sobrado nada: se sobrou, o banco tem MENOS do que a fonte */
    unsigned char sobra[1];
    if(fread(sobra, 1, 1, f) == 1) difs++;
    fclose(f);
    if(lidos_out) *lidos_out = lidos;
    return difs;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

    struct { const char *nome; const char *caminho; } docs[] = {
        { "teoria",        "../teoria.tex" },
        { "catalogo",      "../catalogo.tex" },
        { "enredo",        "../enredo.tex" },
        { "corpo_analitico", "../papers/corpo_analitico.tex" },
        { "computacional", "../papers/arquitetura.tex" },
    };
    const int ND = (int)(sizeof docs / sizeof docs[0]);

printf("\n=== A FONTE ENTRA NO BANCO, E SAI INTEIRA ==================================\n");

printf("\n§F1  A fonte SOBE por MOVE(-1) e DESCE por MOVE(+1) — e a volta fecha.\n\n");
    long total_blocos = 0, total_bytes = 0, total_difs = 0, subidos = 0;
    printf("      documento        blocos     bytes na volta   difere em\n");
    for(int i = 0; i < ND; i++){
        long nb = subir(&b, docs[i].nome, docs[i].caminho);
        if(nb < 0){ printf("      %-16s NAO ABRIU\n", docs[i].nome); continue; }
        subidos++;
        long lidos = 0;
        long difs = descer_e_comparar(&b, docs[i].nome, docs[i].caminho, &lidos);
        total_blocos += nb; total_bytes += lidos; total_difs += difs < 0 ? 1 : difs;
        printf("      %-16s %-10ld %-16ld %ld\n", docs[i].nome, nb, lidos, difs);
    }
    ok("os CINCO documentos entram no banco e saem byte a byte, com residuo ZERO — e' o par a"
       " fechar: sobe por MOVE(-1), desce por MOVE(+1). Enquanto a fonte mora so' no disco, um"
       " PDF pre-compilado e' uma GRAVACAO ao lado dela — foi assim que dois papers ficaram a"
       " ser servidos de uma publicacao anterior com o link a devolver 200. Com a fonte no"
       " banco, compor deixa de precisar de uma copia guardada",
       subidos == ND && total_difs == 0 && total_bytes > 0);
    printf("      (%ld blocos, %ld bytes, %ld diferencas.)\n", total_blocos, total_bytes, total_difs);

printf("\n§F2  E e' UMA fonte, nao duas: o tamanho no banco e' o tamanho no disco.\n\n");
    {
        long mau = 0;
        printf("      documento        no disco      no banco\n");
        for(int i = 0; i < ND; i++){
            FILE *f = fopen(docs[i].caminho, "rb");
            if(!f){ mau++; continue; }
            fseek(f, 0, SEEK_END); long noDisco = ftell(f); fclose(f);
            long noBanco = 0;
            (void)descer_e_comparar(&b, docs[i].nome, docs[i].caminho, &noBanco);
            printf("      %-16s %-13ld %ld\n", docs[i].nome, noDisco, noBanco);
            if(noDisco != noBanco) mau++;
        }
        ok("o tamanho e' o mesmo nos dois lados — duas FONTES divergem, uma fonte e a sua"
           " projeccao nao podem. E' a mesma razao por que o manifesto passou a sair do banco:"
           " o que se le' em dois sitios acaba por discordar num deles, e o silencio nao diz"
           " qual", mau == 0);
    }

printf("\n§F3  O CONTROLO: um byte trocado no banco ACUSA — senao a volta fechava na mesma.\n\n");
    {
        /* estraga-se UM byte de UM bloco e mede-se se a comparacao o apanha. Sem isto, o §F1
         * so' diria que a leitura devolve alguma coisa — nao que devolve a coisa certa. */
        const char *alvo = "computacional";
        char chave[160]; snprintf(chave, sizeof chave, "fonte/%s/000000", alvo);
        unsigned char v[BLOCO];
        long n = ler(&b, chave, v, sizeof v);
        long apanhou = 0, devolvido = 0;
        if(n > 10){
            unsigned char guardado = v[7];
            v[7] = (unsigned char)(guardado ^ 0xFF);        /* um byte, um so' */
            gravar(&b, chave, v, n);
            long difs = descer_e_comparar(&b, alvo, "../papers/arquitetura.tex", NULL);
            apanhou = (difs > 0);
            printf("      um byte trocado no bloco 0: a comparacao acusa %ld diferenca(s)\n", difs);
            v[7] = guardado;                                 /* devolve-se SEMPRE */
            gravar(&b, chave, v, n);
            devolvido = (descer_e_comparar(&b, alvo, "../papers/arquitetura.tex", NULL) == 0);
        }
        ok("um unico byte trocado ACUSA, e o ficheiro volta ao original depois — e' a segunda"
           " metade da medida: a primeira diz que o que esta' certo passa, esta diz que o que"
           " esta' errado NAO. Sem ela, o §F1 dizia apenas que a leitura devolve alguma coisa,"
           " e nao que devolve a coisa certa", apanhou && devolvido);
    }

printf("\n§F4  E a fonte e' de uma LINGUAGEM, que nao e' o meio.\n\n");
    {
        /* o LaTeX esta' no banco como as outras oito, e a sua assinatura le-se de la'. Se ele
         * fosse o suporte e nao uma realizacao, nao teria assinatura nenhuma — teria um lugar
         * a' parte, e e' isso que privilegio quer dizer. */
        struct base c;
        long tem = 0, p = 0, q = 0, r = 0;
        /* auditoria 14/08: /tmp/cards_banco é partilhado por 21 medidores e
         * abrir(...,1) TRUNCA — depender da sobra do cards era fragilidade de
         * ordem. A âncora entre-programas mantém-se: a assinatura lê-se DA
         * FONTE do cards.c (muda lá, muda aqui); e a ida-e-volta prova-se no
         * banco DESTE medidor, isolado. */
        (void)c;
        {
            FILE *fc = fopen("cards.c", "rb");
            if(fc){
                char lin[512];
                while(fgets(lin, sizeof lin, fc)){
                    char faz[64];
                    if(sscanf(lin, " { \"latex\", \"%63[^\"]\", %ld, %ld, %ld },", faz, &p, &q, &r) == 4){ tem = 1; break; }
                }
                fclose(fc);
            }
        }
        if(tem){
            unsigned char v[96]; unsigned char out[96];
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld", p, q, r);
            if(!gravar(&b, "linguagem/latex", v, m)) tem = 0;
            else {
                long k = ler(&b, "linguagem/latex", out, sizeof out - 1);
                if(k != m || memcmp(out, v, (size_t)m) != 0) tem = 0;
            }
        }
        if(tem) printf("      latex no banco: (%ld,%ld,%ld) — expande, quebra, e o texto passa\n", p, q, r);
        else    printf("      latex no banco: AUSENTE (corra tests/cards.c primeiro)\n");
        ok("o LATEX esta' no banco com assinatura, como as outras oito — e (1,1,1) e' a unica"
           " com um de cada, porque ele faz as tres: expande macros (+1), quebra o fluxo em"
           " paginas (-1) e o conteudo atravessa sem mudar (0). Se ele fosse o SUPORTE e nao"
           " uma realizacao, nao teria assinatura nenhuma — teria um lugar a' parte, e e' isso"
           " que privilegio quer dizer. Um documento e' uma projeccao, e o predicado e' o"
           " texto: ele nao mora no compositor",
           tem && p == 1 && q == 1 && r == 1);
    }

    fechar(&b);
printf("\n=== A FONTE =================================================================\n");
printf("  Um PDF pre-compilado e' uma GRAVACAO — a mesma coisa que o GIF era ao lado do\n");
printf("  kernel, e envelhece do mesmo modo: esta semana dois papers foram servidos de uma\n");
printf("  publicacao anterior enquanto o link devolvia 200.\n\n");
printf("  Com a FONTE no banco, compor deixa de precisar de uma copia guardada. Isto mede a\n");
printf("  metade que hoje se pode medir: a fonte sobe, desce byte a byte, o tamanho bate dos\n");
printf("  dois lados, e um unico byte trocado acusa. A outra metade — compor a pedido — pede\n");
printf("  o compositor do lado do servidor, e nao se mede com o que ha' aqui.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESIDUO 0 — a fonte entra e sai inteira.\n\n");
    return 0;
}
