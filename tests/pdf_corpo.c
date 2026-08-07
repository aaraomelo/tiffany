/* pdf_corpo.c — O PDF É UM CORPO, E O GERADOR E O LEITOR SÃO OS SEUS DOIS SENTIDOS.
 *
 * O Aarão: «põe a assinatura do gerador/leitor de PDF no banco, foca nele — depois o LaTeX
 * via conversão por partes, não misture tudo.»
 *
 * E a separação é o ponto, não uma arrumação. Eu tinha o \LaTeX{} e o PDF no mesmo saco
 * porque o tradutor faz as duas coisas no mesmo programa — e por isso a assinatura do PDF
 * nunca tinha sido escrita. Ela é diferente, e a diferença diz o que cada um é:
 *
 *      \LaTeX      COMPOR    expande (+1), quebra (−1), o texto passa (0)      (1,1,1)
 *      PDF         O PAR     gerar (+1) e ler (−1), e nada degenerado          (1,1,0)
 *
 * (1,1,0) é a assinatura da PRÓPRIA TRÍADE, e não por coincidência: o gerador é o lado que
 * SÓ EMITE — o buraco branco — e o leitor é o lado que SÓ ABSORVE — o buraco negro. Nenhum
 * dos dois é reversível sozinho. É o PAR que é a estrela, e é por isso que o teste do
 * formato é a volta e não a validade: um PDF válido que não se lê de volta é meia operação.
 *
 *   §P1  as três assinaturas no banco: gerador, leitor, e o par           — e saem de lá
 *   §P2  o gerador SÓ emite e o leitor SÓ absorve — e o par tem os dois   — é o trial
 *   §P3  a VOLTA: o que se escreveu no PDF lê-se de volta, resíduo 0
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

printf("\n=== O PDF É UM CORPO: O GERADOR E O LEITOR SÃO OS DOIS SENTIDOS ==============\n");

printf("\n§P1  As três assinaturas entram no banco — e saem de lá.\n\n");
    {
        struct { const char *chave; const char *faz; long p, q, r; } as[] = {
            { "corpo/pdf/gerador", "emitir",  1, 0, 0 },
            { "corpo/pdf/leitor",  "absorver",0, 1, 0 },
            { "corpo/pdf",         "o par",   1, 1, 0 },
        };
        long postas = 0, resid = 0;
        unsigned char out[VMAX];
        printf("      chave                 assinatura   o que faz\n");
        for(int i = 0; i < 3; i++){
            unsigned char v[96];
            long m = (long)snprintf((char*)v, sizeof v, "%ld,%ld,%ld|%s",
                                    as[i].p, as[i].q, as[i].r, as[i].faz);
            if(gravar(&b, as[i].chave, v, m)) postas++;
            long k = ler(&b, as[i].chave, out, sizeof out);
            if(k != m || memcmp(out, v, (size_t)m) != 0) resid++;
            printf("      %-21s (%ld,%ld,%ld)      %s\n", as[i].chave, as[i].p, as[i].q, as[i].r, as[i].faz);
        }
        ok("as TRÊS assinaturas do PDF entram no banco pela mesma porta que tudo o resto e saem"
           " sem resíduo. E são três e não uma porque o gerador e o leitor NÃO são o mesmo"
           " corpo: um só emite, o outro só absorve, e é o PAR que tem os dois sentidos",
           postas == 3 && resid == 0);
    }

printf("\n§P2  O gerador SÓ emite, o leitor SÓ absorve — e o par tem os dois. É o trial.\n\n");
    {
        /* lê-se do BANCO, não da tabela acima: se a leitura não bater, o §P1 mediu a escrita
         * e não o registo. E o regime sai da assinatura pela mesma regra do §B7. */
        struct { const char *chave; const char *regime; } esp[] = {
            { "corpo/pdf/gerador", "branco" },   /* p sem q: só cresce */
            { "corpo/pdf/leitor",  "negro"  },   /* q sem p: só recolhe */
            { "corpo/pdf",         "estrela"},   /* os dois: a interface */
        };
        long mau = 0;
        printf("      corpo                 lido do banco   regime\n");
        for(int i = 0; i < 3; i++){
            unsigned char v[96];
            long k = ler(&b, esp[i].chave, v, sizeof v - 1);
            if(k <= 0){ mau++; continue; }
            v[k] = 0;
            long p = 0, q = 0, r = 0;
            if(sscanf((char*)v, "%ld,%ld,%ld", &p, &q, &r) != 3){ mau++; continue; }
            const char *reg = (p > 0 && q > 0) ? "estrela" : (p > 0 ? "branco" : (q > 0 ? "negro" : "-"));
            printf("      %-21s (%ld,%ld,%ld)          %s\n", esp[i].chave, p, q, r, reg);
            if(strcmp(reg, esp[i].regime) != 0) mau++;
        }
        ok("o REGIME lê-se da assinatura e bate nos três: o gerador é o BRANCO (só emite), o"
           " leitor é o NEGRO (só absorve) e o par é a ESTRELA (tem os dois, e é o único"
           " reversível). Não é uma metáfora posta por cima: é a mesma regra do §B7 aplicada a"
           " uma assinatura que veio do banco", mau == 0);
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
printf("  O gerador e o leitor não são o mesmo corpo, e a assinatura di-lo: (1,0,0) só emite,\n");
printf("  (0,1,0) só absorve. Nenhum é reversível sozinho — é o PAR, (1,1,0), que é a estrela,\n");
printf("  e (1,1,0) é a assinatura da própria tríade.\n\n");
printf("  E o teste do formato é a VOLTA, não a validade: um PDF válido que não se lê de volta\n");
printf("  é meia operação. Escrever é MOVE(−1), ler é MOVE(+1), e só o par fecha.\n\n");
printf("  Isto é o corpo PDF, sozinho. O \\LaTeX{} é outro — (1,1,1), a única com um de cada —\n");
printf("  e a conversão entre os dois faz-se por partes, que é a próxima peça e não esta.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
printf("\n  RESÍDUO 0 — o par fecha.\n\n");
    return 0;
}
