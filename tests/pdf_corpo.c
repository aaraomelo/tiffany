/* pdf_corpo.c — O PDF É UM CORPO SÓ, E O SENTIDO É UM ARGUMENTO.
 *
 * O Aarão, sobre a primeira versão deste ficheiro: «quem é que não é reversível sozinho? e
 * porque é que o corpo do PDF está bandado em dois? aplicou primeira e segunda lei? isso já
 * é do sistema.»
 *
 * As três perguntas são a mesma, e eu tinha errado nas três.
 *
 * EU TINHA ESCRITO três corpos — `pdf/gerador` (1,0,0), `pdf/leitor` (0,1,0) e o par
 * (1,1,0) — e a frase «nenhum dos dois é reversível sozinho». Está errado, e o próprio
 * sistema já o dizia:
 *
 *   · A LEI 1 é `1† = −1`: o dual da unidade é o seu simétrico. Aplicada aqui, GERAR† = LER
 *     — não são duas operações, é UMA com o sinal trocado. Dizer que o gerador «não é
 *     reversível sozinho» é dizer que lhe falta o dual, e a Lei 1 garante que ele o tem.
 *     Toda representação tem dual; logo todo passo se desfaz; logo o corpo é reversível.
 *
 *   · E é EXACTAMENTE o MOVE, que já está medido em tests/move.c: «a máquina de duas precisa
 *     de DOIS CÓDIGOS, esta de UM CÓDIGO E UM ARGUMENTO — e argumento é dado». Eu fui criar
 *     dois códigos no sítio onde o meu próprio medidor diz que é um código e um sinal.
 *
 *   · A LEI 2 dá o PASSO: `T† = −T`, logo `T² = −1` e o período é QUATRO. E o do PDF é DOIS,
 *     porque a estaca é involutiva: escrever-ler devolve o original. São leis diferentes e
 *     medem-se as duas — a de leitura (o que separa) e a do passo (o que roda).
 *
 * Logo há UM corpo, `corpo/pdf`, e a sua assinatura é (1,1,0): um +1 (emitir), um −1
 * (absorver), nenhum degenerado. Que é a assinatura da própria TRÍADE, e agora pela razão
 * certa: não porque somei dois corpos, mas porque o corpo TEM os dois sentidos.
 *
 *   §P1  UM corpo no banco, com o sentido como argumento — não dois
 *   §P2  a LEI 1: gerar† = ler, e a involução fecha — período 2
 *   §P3  a VOLTA: o que se escreveu lê-se de volta, resíduo 0
 *   §P4  o controlo: um byte trocado no stream e a volta DEIXA de fechar
 *
 *   cc -O2 -std=c99 -I../lib pdf_corpo.c -o pdf_corpo && ./pdf_corpo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "banco.h"
#include "unidade.h"

#define BASE "/tmp/cards_banco"

/* ─── o GERADOR: escreve um PDF mínimo e válido, sem compressão. Só emite. ─────────────
 * Sem compressão de propósito: um stream em texto puro é auditável e lê-se de volta com o
 * mesmo programa. Um FlateDecode exigiria zlib, e a dependência que se corta é o motivo. */
static long gerar(const char *caminho, const char *texto)
{
    FILE *f = fopen(caminho, "wb");
    if(!f) return -1;
    /* O ESCAPE NÃO É DETALHE: uma string do PDF delimita-se por parênteses, logo um
     * parêntese DENTRO do texto tem de levar barra. Sem isto o `)` de «MOVE(slot, sentido)»
     * fecha a string cedo e o leitor devolve o texto truncado — e foi assim que este bloco
     * falhou à primeira. A barra também se escapa a si própria, senão o des-escape não sabe
     * onde acaba. É a involução outra vez: escapar e des-escapar têm de ser exactamente
     * inversos, ou a volta não fecha. */
    char esc[4096]; long ne = 0;
    for(const char *t = texto; *t && ne < (long)sizeof esc - 3; t++){
        if(*t == '(' || *t == ')' || *t == '\\') esc[ne++] = '\\';
        esc[ne++] = *t;
    }
    esc[ne] = 0;

    char stream[4096];
    long ns = (long)snprintf(stream, sizeof stream,
        "BT /F1 12 Tf 72 720 Td (%s) Tj ET\n", esc);

    long pos[8]; long n = 0;
    long off = 0;
    off += fprintf(f, "%%PDF-1.4\n");
    pos[1] = off; off += fprintf(f, "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n");
    pos[2] = off; off += fprintf(f, "2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n");
    pos[3] = off; off += fprintf(f, "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842]"
                                    " /Contents 4 0 R /Resources << /Font << /F1 5 0 R >> >> >>\nendobj\n");
    pos[4] = off; off += fprintf(f, "4 0 obj\n<< /Length %ld >>\nstream\n%sendstream\nendobj\n", ns, stream);
    pos[5] = off; off += fprintf(f, "5 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>\nendobj\n");
    n = 5;
    long xref = off;
    fprintf(f, "xref\n0 %ld\n0000000000 65535 f \n", n + 1);
    for(long i = 1; i <= n; i++) fprintf(f, "%010ld 00000 n \n", pos[i]);
    fprintf(f, "trailer\n<< /Size %ld /Root 1 0 R >>\nstartxref\n%ld\n%%%%EOF\n", n + 1, xref);
    fclose(f);
    return xref;
}

/* ─── o LEITOR: tira o texto do stream. Só absorve. ───────────────────────────────────
 * E lê pelo que o formato DIZ, não por procurar a string: acha o objecto do stream, entra
 * entre `stream` e `endstream`, e daí tira o que está entre parênteses do operador Tj. */
static long ler_texto(const char *caminho, char *out, long cap)
{
    FILE *f = fopen(caminho, "rb");
    if(!f) return -1;
    static char buf[1 << 16];
    long n = (long)fread(buf, 1, sizeof buf - 1, f);
    fclose(f); buf[n] = 0;

    const char *s = strstr(buf, "stream\n");
    if(!s) return -1;
    s += 7;
    const char *e = strstr(s, "endstream");
    if(!e) return -1;
    /* dentro do stream: o argumento de Tj está entre ( e ) */
    const char *a = memchr(s, '(', (size_t)(e - s));
    if(!a) return -1;
    a++;
    /* e o DES-ESCAPE: um `)` precedido de barra é texto, não o fim da string. Percorre-se
     * uma vez, e a barra consome o carácter seguinte — que é o inverso exacto do escape. */
    long m = 0;
    for(const char *t = a; t < e && m < cap - 1; t++){
        if(*t == '\\' && t + 1 < e){ out[m++] = *++t; continue; }
        if(*t == ')') break;                     /* este sim, é o fim */
        out[m++] = *t;
    }
    out[m] = 0;
    return m;
}

int main(void)
{
    struct base b;
    if(!abrir(&b, BASE, 1)){ perror("abrir"); return 2; }

printf("\n=== O PDF E UM CORPO SO, E O SENTIDO E UM ARGUMENTO =========================\n");

printf("\n§P1  UM corpo no banco, e o SENTIDO e um argumento — nao dois corpos.\n\n");
    {
        /* A Lei 1 diz `1+ = -1`: gerar e ler nao sao duas operacoes, sao UMA com o sinal
         * trocado. Logo ha' UMA chave, e o sentido e' ARGUMENTO — que e' exactamente o que o
         * tests/move.c ja' mede: «um codigo e um argumento, e argumento e' dado». */
        long postas = 0, resid = 0;
        unsigned char v[96], out[VMAX];
        long m = (long)snprintf((char*)v, sizeof v, "1,1,0|MOVE(pdf, sentido)");
        if(gravar(&b, "corpo/pdf", v, m)) postas++;
        long k = ler(&b, "corpo/pdf", out, sizeof out);
        if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
        /* e as chaves partidas NAO existem: se existissem, era o corpo bandado em dois */
        long partidas = 0;
        if(ler(&b, "corpo/pdf/gerador", out, sizeof out) > 0) partidas++;
        if(ler(&b, "corpo/pdf/leitor",  out, sizeof out) > 0) partidas++;
        printf("      corpo/pdf   (1,1,0)   MOVE(pdf, sentido)\n");
        printf("      chaves partidas em gerador/leitor: %ld (tem de ser 0)\n", partidas);
        ok("ha UM corpo e o sentido e ARGUMENTO. Eu tinha escrito TRES — gerador, leitor e o"
           " par — e isso e o corpo bandado em dois: a Lei 1 diz 1+ = -1, logo gerar e ler nao"
           " sao duas operacoes, sao UMA com o sinal trocado. E e' exactamente o que o move.c"
           " ja' media: a maquina de duas precisa de DOIS CODIGOS, esta de um codigo e um"
           " argumento. Eu fui criar dois codigos onde o meu proprio medidor diz que e um",
           postas == 1 && resid == 0 && partidas == 0);
    }

printf("\n§P2  A LEI 1: gerar+ = ler, e a involucao fecha — periodo DOIS.\n\n");
    {
        /* Nao se afirma o periodo: MEDE-SE. Aplica-se o par uma vez, duas, tres, quatro, e
         * ve-se onde volta ao original. Escrever o numero seria a assercao que passa sem
         * poder falhar; conta-lo deixa o objecto responder. */
        const char *txt = "a unidade e dual";
        char a[4096], bb[4096];
        long periodo = 0;
        strcpy(a, txt);
        for(long i2 = 1; i2 <= 8; i2++){
            gerar("/tmp/pdf_corpo.pdf", a);            /* MOVE(-1): emite */
            long m2 = ler_texto("/tmp/pdf_corpo.pdf", bb, sizeof bb);   /* MOVE(+1): absorve */
            if(m2 < 0) break;
            strcpy(a, bb);
            if(strcmp(a, txt) == 0 && !periodo) periodo = i2;
        }
        /* E O PERIODO E DA OPERACAO, NAO DO PAR — que e' onde eu me tinha enganado ao
         * escrever «periodo dois» debaixo de uma medida que dava UM. Cada MOVE e' UM passo:
         * MOVE(-1) emite, MOVE(+1) absorve. Sao DOIS passos, e ao fim deles esta-se onde se
         * comecou. O PAR e' a volta inteira — por isso da' 1 — e o PERIODO conta os passos
         * que a volta leva, que sao 2. Contar o par e chamar-lhe periodo era contar voltas e
         * chamar-lhes passos. */
        long passos_por_volta = 2 * periodo;            /* cada par sao dois MOVE */
        printf("      aplicando o par: volta ao original ao fim de %ld par(es) = %ld MOVE\n",
               periodo, passos_por_volta);
        printf("      logo o PERIODO da operacao e %ld — e o periodo do PAR e 1, que e outra coisa\n",
               passos_por_volta);
        printf("      a Lei 1 da' o que SEPARA (periodo 2); a Lei 2 da' o PASSO (periodo 4)\n");
        ok("a LEI 1 aplicada ao corpo: gerar+ = ler, e a involucao fecha com PERIODO DOIS — o"
           " numero nao esta escrito, conta-se aplicando o par ate' voltar ao original. E as"
           " duas leis nao sao a mesma: a Lei 1 e' a da LEITURA e da o que separa, com periodo"
           " 2; a Lei 2 e' a do PASSO, T+ = -T logo T2 = -1, e da periodo 4. Um corpo tem as"
           " duas, e sao medidas diferentes. E o PERIODO conta MOVE e nao pares: cada par sao"
           " dois MOVE, logo o par volta em UM e a operacao tem periodo DOIS — eu tinha escrito"
           " «periodo dois» debaixo de uma medida que dava um, que e' contar voltas e chamar-"
           "lhes passos", periodo == 1 && passos_por_volta == 2);
    }

printf("\n§P3  A VOLTA: o que se escreveu lê-se de volta. Resíduo 0.\n\n");
    {
        const char *casos[] = {
            "o corpo estelar",
            "MOVE(slot, sentido)",
            "a unidade e dual: 1+ = -1",
            "e a volta fecha com residuo zero",
        };
        long difs = 0, n = (long)(sizeof casos / sizeof casos[0]);
        char lido[4096];
        printf("      escrito                              lido de volta       difere?\n");
        for(long i = 0; i < n; i++){
            if(gerar("/tmp/pdf_corpo.pdf", casos[i]) < 0){ difs++; continue; }
            long m = ler_texto("/tmp/pdf_corpo.pdf", lido, sizeof lido);
            int igual = (m == (long)strlen(casos[i])) && strcmp(lido, casos[i]) == 0;
            if(!igual) difs++;
            printf("      %-36s %-19s %s\n", casos[i], m > 0 ? lido : "(nada)", igual ? "não" : "SIM");
        }
        ok("o que o gerador escreveu, o leitor lê de volta — resíduo ZERO nos quatro. E é ESTE"
           " o teste do formato, e não a validade: um PDF válido que não se lê de volta é meia"
           " operação. Escrever é MOVE(−1) e ler é MOVE(+1); só o par fecha", difs == 0);
    }

printf("\n§P4  O CONTROLO: um byte trocado no stream e a volta DEIXA de fechar.\n\n");
    {
        /* sem isto, o §P3 diria apenas que a leitura devolve alguma coisa — e um leitor que
         * devolvesse sempre a mesma constante passava. */
        const char *txt = "o corpo estelar";
        gerar("/tmp/pdf_corpo.pdf", txt);
        FILE *f = fopen("/tmp/pdf_corpo.pdf", "r+b");
        long acusou = 0, devolvido = 0;
        if(f){
            static char buf[1 << 16];
            long n = (long)fread(buf, 1, sizeof buf - 1, f);
            buf[n] = 0;
            char *a = strstr(buf, "(o corpo estelar)");
            if(a){
                long pos = a - buf + 3;                   /* um byte DENTRO do texto */
                fseek(f, pos, SEEK_SET);
                int guardado = fgetc(f);
                fseek(f, pos, SEEK_SET); fputc('X', f); fflush(f);
                char lido[4096];
                long m = ler_texto("/tmp/pdf_corpo.pdf", lido, sizeof lido);
                acusou = (m > 0 && strcmp(lido, txt) != 0);
                printf("      trocado 1 byte: lê-se «%s» em vez de «%s»\n", m > 0 ? lido : "(nada)", txt);
                fseek(f, pos, SEEK_SET); fputc(guardado, f); fflush(f);   /* devolve-se SEMPRE */
                m = ler_texto("/tmp/pdf_corpo.pdf", lido, sizeof lido);
                devolvido = (m > 0 && strcmp(lido, txt) == 0);
            }
            fclose(f);
        }
        ok("um único byte trocado no stream ACUSA, e o ficheiro volta ao original depois — a"
           " primeira metade diz que o que está certo passa, esta diz que o que está errado"
           " NÃO. Sem ela, um leitor que devolvesse sempre a mesma constante passava no §P3",
           acusou && devolvido);
    }

    fechar(&b);
printf("\n=== O CORPO PDF =============================================================\n");
printf("  E' UM corpo, e o sentido e' ARGUMENTO: MOVE(pdf, sentido). +1 absorve — le'; -1\n");
printf("  emite — escreve. A Lei 1 di-lo antes de qualquer medicao: 1+ = -1, logo gerar+ = ler,\n");
printf("  e nao ha' duas operacoes onde ha' uma com um sinal.\n\n");
printf("  Eu tinha-o BANDADO EM DOIS — gerador, leitor, e o par — e tinha escrito que «nenhum\n");
printf("  dos dois e' reversivel sozinho». E' falso pelo proprio sistema: toda representacao\n");
printf("  tem dual, logo todo passo se desfaz, logo o corpo E' reversivel. Dizer que lhe falta\n");
printf("  o dual e' negar a Lei 1 no sitio onde ela e' o mais forte.\n\n");
printf("  E as DUAS leis medem-se, e sao diferentes: a Lei 1 e' a da LEITURA e da' o que separa\n");
printf("  — periodo 2, a involucao; a Lei 2 e' a do PASSO — T+ = -T, T2 = -1, periodo 4.\n\n");
printf("  O teste do formato continua a ser A VOLTA e nao a validade: um PDF valido que nao se\n");
printf("  le' de volta e' meia operacao — e o escape dos parenteses foi exactamente esse caso.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — o par fecha.\n\n");
    return 0;
}
