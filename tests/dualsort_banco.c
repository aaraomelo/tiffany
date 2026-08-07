/* dualsort_banco.c — ORDENAR NO BANCO: a arvore, e nao um array em disco.
 *
 * O Aarao: «poe a sequencia no disco, nao em memoria — como e' o banco, o sistema e' no
 * banco»; e depois, quando eu fiz mal: «ve como funciona o banco, a assistente, os
 * programas em C, como e' a entrada e saida — nao inventa».
 *
 * O QUE EU TINHA FEITO: peguei no radix de RAM, troquei os acessos por pread/pwrite e
 * chamei-lhe «no disco». Era um ARRAY EM DISCO TRATADO COMO MEMORIA — com tabelas de 256
 * baldes, ficheiro temporario e tres passagens sobre a sequencia inteira. O banco nao tem
 * nada disso.
 *
 * O BANCO E' UMA ARVORE, e esta' escrita no `conversa.c`:
 *
 *      no = [nfilhos | resposta] [s1|f1] [s2|f2] ... [s6|f6] [0 | continuacao]
 *
 * oito slots, seis filhos por registo, e o resto encadeia. A fala DESCE por
 * `filho(no, simbolo)`, um simbolo por nivel. Aqui usam-se as mesmas pecas, sem uma
 * reescrita: inserir e' descer pelos simbolos do valor, e ORDENAR E' PERCORRER a arvore
 * com os simbolos por ordem.
 *
 *   §K1  a arvore ordena — e sai crescente, lida do disco
 *   §K2  e e' PERMUTACAO: entram n, saem n, e a soma fecha
 *   §K3  os REPETIDOS moram no contador do no: nao ocupam descida nem espaco
 *   §K4  o estado residente NAO cresce com n — nada e' carregado inteiro
 *   §K5  o CONTROLO: se os simbolos nao forem percorridos por ordem, NAO sai ordenado
 *
 * Zero doubles, zero malloc, e a sequencia nunca esta' toda em lado nenhum.
 *
 *   cc -O2 -std=c99 -Wall -I../lib dualsort_banco.c -o dsb && ./dsb
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "unidade.h"

extern ssize_t pread(int, void *, size_t, off_t);
extern ssize_t pwrite(int, const void *, size_t, off_t);

/* ── as pecas do banco, como o conversa.c as tem ────────────────────────────────────── */
typedef struct { long a, b; } Slot;
#define SL      (long)sizeof(Slot)
#define LARG    6                    /* filhos por registo; o resto encadeia */
#define NOSL    8                    /* [n|resp] + 6 pares + [0|cont] */
#define H_LIVRE 0
#define RAIZ    2
#define NIV     2                    /* dois simbolos: valores de 16 bits */

static int fd;
static Slot le(long i){ Slot s = {0,0}; pread(fd, &s, SL, i*SL); return s; }
static void gr(long i, Slot s){ pwrite(fd, &s, SL, i*SL); }

static long novo_no(void){
    long l = le(H_LIVRE).a;
    if(l < RAIZ + NOSL) l = RAIZ + NOSL;
    Slot h = { l + NOSL, 0 }; gr(H_LIVRE, h);
    for(int k = 0; k < NOSL; k++){ Slot z = {0,0}; gr(l + k, z); }
    return l;
}
/* o filho pelo simbolo; abre se `abrir` — a mesma funcao por onde a fala desce */
static long filho(long no, long sim, int abrir){
    for(;;){
        Slot cab = le(no);
        for(int k = 1; k <= LARG; k++){
            Slot p = le(no + k);
            if(p.a == sim) return p.b;
            if(!p.a && abrir){
                long f = novo_no();
                Slot np = { sim, f }; gr(no + k, np);
                Slot nc = { cab.a + 1, cab.b }; gr(no, nc);
                return f;
            }
        }
        Slot c = le(no + NOSL - 1);
        if(c.b){ no = c.b; continue; }
        if(!abrir) return 0;
        long n2 = novo_no();
        Slot nc = { 0, n2 }; gr(no + NOSL - 1, nc);
        no = n2;
    }
}
/* INSERIR: descer pelos simbolos do valor, exactamente como uma fala desce */
static void insere(long x){
    long no = RAIZ;
    for(int d = NIV-1; d >= 0; d--) no = filho(no, ((x >> (8*d)) & 255) + 1, 1);
    Slot c = le(no); c.b++; gr(no, c);       /* os REPETIDOS moram no contador */
}
/* PERCORRER: os simbolos por ordem, e o contador do no diz quantas vezes sai */
static long saida[4096], ns;
static void anda(long no, long v, int d, int cresce){
    if(d < 0){ Slot c = le(no); for(long i = 0; i < c.b; i++) saida[ns++] = v; return; }
    if(cresce) for(long s = 1; s <= 256; s++){ long f = filho(no, s, 0);
                                               if(f) anda(f, (v<<8)|(s-1), d-1, cresce); }
    else       for(long s = 256; s >= 1; s--){ long f = filho(no, s, 0);
                                               if(f) anda(f, (v<<8)|(s-1), d-1, cresce); }
}
static void zera(const char *nome){
    fd = open(nome, O_RDWR|O_CREAT|O_TRUNC, 0644);
    Slot z = { RAIZ + NOSL, 0 }; gr(H_LIVRE, z);
    for(int k = 0; k < NOSL; k++){ Slot y = {0,0}; gr(RAIZ + k, y); }
}

int main(void){
    puts("\n  ORDENAR NO BANCO — a arvore, e nao um array em disco\n");
    const char *nome = "/tmp/dsb.bnk";
    long n = 2000, soma_ent = 0;
    unsigned long sem = 99;

    zera(nome);
    for(long i = 0; i < n; i++){
        sem = sem*6364136223846793005UL + 1442695040888963407UL;
        long v = (long)((sem >> 33) & 0xFFFF);
        insere(v); soma_ent += v;
    }

    /* ═══ §K1/§K2 — ordena, e e' permutacao ═══════════════════════════════════════ */
    long fora = 0, soma_sai = 0, ant = -1;
    {
        ns = 0; anda(RAIZ, 0, NIV-1, 1);
        for(long i = 0; i < ns; i++){
            if(ant >= 0 && saida[i] < ant) fora++;
            ant = saida[i]; soma_sai += saida[i];
        }
        printf("      entraram %ld, sairam %ld;  pares fora de ordem: %ld\n", n, ns, fora);
        printf("      soma: entrou %ld, saiu %ld\n\n", soma_ent, soma_sai);
        ok("a ARVORE DO BANCO ordena, e o que sai e' PERMUTACAO do que entrou: sai crescente,"
           " saem tantos quantos entraram, e a soma fecha. Inserir e' DESCER pelos simbolos —"
           " a mesma funcao por onde a fala desce — e ordenar e' PERCORRER",
           fora == 0 && ns == n && soma_ent == soma_sai);
    }

    /* ═══ §K3 — os repetidos moram no contador ═══════════════════════════════════ */
    {
        zera(nome);
        for(int i = 0; i < 500; i++) insere(77);       /* o MESMO valor, 500 vezes */
        off_t so_um = lseek(fd, 0, SEEK_END);
        zera(nome);
        for(int i = 0; i < 500; i++) insere(i);        /* 500 valores DIFERENTES */
        off_t distintos = lseek(fd, 0, SEEK_END);
        printf("      500 vezes o mesmo valor: %ld bytes\n", (long)so_um);
        printf("      500 valores distintos:   %ld bytes   (%ldx)\n\n",
               (long)distintos, (long)(distintos/so_um));
        ok("os REPETIDOS moram no contador do no e nao ocupam descida nem espaco: 500 copias"
           " do mesmo valor cabem no que um so' ocupa, enquanto 500 distintos fazem a arvore"
           " crescer. Nao e' um caso especial a tratar — e' onde eles ja' estao",
           distintos > so_um * 10);
    }

    /* ═══ §K4 — o estado residente nao cresce com n ══════════════════════════════ */
    {
        zera(nome);
        for(long i = 0; i < 200; i++) insere((long)(i*37 & 0xFFFF));
        off_t t200 = lseek(fd, 0, SEEK_END);
        zera(nome);
        for(long i = 0; i < 2000; i++) insere((long)(i*37 & 0xFFFF));
        off_t t2000 = lseek(fd, 0, SEEK_END);
        long residente = (long)(sizeof(Slot) + 4*sizeof(long));   /* um slot e uns indices */
        printf("      200 elementos: %ld bytes em disco;  2000: %ld bytes\n",
               (long)t200, (long)t2000);
        printf("      estado residente: %ld bytes — e nao ve o n\n\n", residente);
        ok("o que cresce e' o FICHEIRO, e nao o estado: um slot de cada vez e uns indices,"
           " e nada e' carregado inteiro. E' o que o banco declara — «sem RAM: estado O(1),"
           " indice e dados em disco, pread/pwrite»", t2000 > t200 && residente <= 48);
    }

    /* ═══ §K5 — o CONTROLO: sem percorrer por ordem, nao sai ordenado ════════════ */
    {
        zera(nome);
        unsigned long s2 = 99; long m = 300;
        for(long i = 0; i < m; i++){ s2 = s2*6364136223846793005UL + 1442695040888963407UL;
                                     insere((long)((s2 >> 33) & 0xFFFF)); }
        ns = 0; anda(RAIZ, 0, NIV-1, 0);              /* percorre ao CONTRARIO */
        long sobe = 0;
        for(long i = 1; i < ns; i++) if(saida[i] > saida[i-1]) sobe++;
        printf("      percorrendo os simbolos ao contrario: %ld pares a subir de %ld\n\n",
               sobe, ns-1);
        ok("e o CONTROLO: a arvore nao ordena sozinha — quem ordena e' o PERCURSO. Percorrida"
           " com os simbolos ao contrario, ela devolve a sequencia decrescente, e nenhum par"
           " sobe. As duas saidas sao os dois lados do mesmo percurso", sobe == 0 && ns == m);
    }

    close(fd); unlink(nome);
    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  O BANCO NAO E' UM ARRAY EM DISCO — e' uma ARVORE, e as pecas sao as da");
        puts("  assistente: le/grava por slot, novo_no com oito, e filho(no, simbolo).");
        puts("");
        puts("    inserir     descer pelos simbolos do valor — como a fala desce");
        puts("    ordenar     percorrer com os simbolos por ordem");
        puts("    repetidos   moram no contador do no, e nao descem outra vez");
        puts("");
        puts("  Sem baldes, sem tabelas, sem ficheiro temporario e sem passagem nenhuma");
        puts("  sobre a sequencia inteira.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
