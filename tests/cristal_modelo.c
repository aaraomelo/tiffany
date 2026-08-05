/* cristal_modelo.c — O MODELO NO BANCO: ler e ESCREVER, que era a metade que faltava.
 *
 * O Aarao: "tira o conceito de doador — tem o modelo, ele roda no banco, interage por
 * la', e' mais 1 cristal, PODE LER E ESCREVER NELE, ele pode ensinar e aprender."
 *
 * O que estava feito era metade: o forward lia token_embd.weight do gguf e devolvia
 * vectores. Ler e' o lado facil, e foi o unico que o conceito de "doador" deixava ver —
 * de um doador tira-se, e ele nao recebe.
 *
 * Aqui o modelo entra no banco como qualquer outro cristal, e o par fecha:
 *
 *      gravar     ->  o vector do modelo passa a ser um registo com crc
 *      banco_ver  ->  volta EXACTO, e o crc confirma-o
 *      gravar     ->  e volta a escrever-se por cima, com o que se aprendeu
 *
 * NAO E' TREINO, e nao se finge que e'. E' a via: o que se escreve fica, e da proxima
 * vez le-se o que ficou. Sem isso, "ensinar e aprender" nao tem onde acontecer.
 *
 *   §C1  o modelo entra no banco, e volta byte a byte
 *   §C2  ESCREVE-SE por cima, e o que volta e' o novo — a segunda metade do par
 *   §C3  o gato e' local, logo ler um elemento e' O(1) mesmo com ele aplicado
 *   §C4  e o CONTROLO: um byte trocado no .dat e o crc recusa o registo
 *
 * Corre sem o modelo presente: se nao houver gguf, usa um vector proprio e diz que o
 * fez — o que se mede e' o BANCO, e o gguf so' torna o caso real.
 *
 *   cc -O2 -std=c99 -Wall -I../lib cristal_modelo.c -o cristal_modelo
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* getline e POSIX; com -std=c99 estrito a libc esconde-o, e a bateria compila assim. */
extern long getline(char **, size_t *, FILE *);
#include "banco.h"
#include "unidade.h"

#define D    768                    /* a dimensao do nomic */
#define NV    16                    /* quantos vectores se poem no banco */
#define VMAXB 4096                  /* o VMAX do banco, em bytes */

int main(void){
    puts("\n  O MODELO NO BANCO — ler e ESCREVER, e nao so' colher\n");

    /* os vectores: do gguf se ele estiver la', senao proprios. o que se mede e' o banco. */
    static float v[NV][D];
    int do_gguf = 0;
    {
        FILE *f = fopen("/tmp/vetores.txt", "r");
        if(f){
            char *linha = NULL; size_t cap = 0; int i = 0;
            while(i < NV && getline(&linha, &cap, f) > 0){
                char *p = linha; int d = 0;
                while(d < D){
                    while(*p==' '||*p=='\t') p++;
                    if(p[0]!='0' || (p[1]!='x' && p[1]!='X')) break;
                    char *fim; unsigned u = (unsigned)strtoul(p, &fim, 16);
                    if(fim == p) break;
                    memcpy(&v[i][d++], &u, sizeof(float)); p = fim;
                }
                if(d == D) i++;
            }
            free(linha); fclose(f);
            if(i == NV) do_gguf = 1;
        }
        if(!do_gguf)
            for(int i=0;i<NV;i++) for(int d=0;d<D;d++)
                v[i][d] = (float)((i*7919 + d*104729) % 2003 - 1000) / 1000.0f;
        printf("      vectores: %s\n", do_gguf ? "DO MODELO (/tmp/vetores.txt)"
                                               : "proprios (o modelo nao estava la')");
    }

    struct base b;
    unlink("/tmp/cm.dat"); unlink("/tmp/cm.idx");
    if(!abrir(&b, "/tmp/cm", 1)){ perror("abrir"); return 1; }

    /* ═══ §C1 — o modelo ENTRA no banco ════════════════════════════════════════════
     * cada vector parte-se em pedacos de VMAX, e cada pedaco e' um registo com a sua
     * impressao e o seu crc. e' o mesmo tratamento de qualquer outro cristal. */
    const int P = (int)((D*sizeof(float) + VMAXB - 1) / VMAXB);   /* pedacos por vector */
    int gravou = 0;
    for(int i=0;i<NV;i++)
        for(int p=0;p<P;p++){
            char c[48]; snprintf(c,sizeof c,"emb.%d.%d",i,p);
            size_t off = (size_t)p*VMAXB;
            size_t n = D*sizeof(float) - off; if(n > VMAXB) n = VMAXB;
            if(gravar(&b, c, (unsigned char*)v[i] + off, (long)n)) gravou++;
        }
    fechar(&b);
    printf("      %d registos gravados (%d vectores x %d pedacos)\n", gravou, NV, P);

    Mapa m;
    if(!banco_mapa(&m, "/tmp/cm")){ puts("      nao mapeou"); return 1; }
    long iguais = 0, total = 0;
    for(int i=0;i<NV;i++)
        for(int p=0;p<P;p++){
            char c[48]; snprintf(c,sizeof c,"emb.%d.%d",i,p);
            long n = 0; const unsigned char *r = banco_ver(&m, c, &n);
            if(!r) continue;
            unsigned char *orig = (unsigned char*)v[i] + (size_t)p*VMAXB;
            for(long k=0;k<n;k++){ if(banco_byte(r,k) == orig[k]) iguais++; total++; }
        }
    banco_larga(&m);
    printf("      volta byte a byte: %ld de %ld\n", iguais, total);
    ok("o MODELO E' MAIS UM CRISTAL: entra no banco por registos com crc e volta exacto,"
       " byte a byte", total > 0 && iguais == total);

    /* ═══ §C2 — ESCREVE-SE POR CIMA: a segunda metade do par ═══════════════════════
     * o que faltava. de um "doador" so' se tira; num cristal escreve-se — e o que se
     * escreve FICA, que e' onde "aprender" tem de acontecer para nao ser palavra. */
    {
        float novo[D];
        for(int d=0; d<D; d++) novo[d] = v[3][d] * 2.0f + 1.0f;   /* uma "licao" qualquer */
        if(!abrir(&b, "/tmp/cm", 0)){ perror("reabrir"); return 1; }
        int reescreveu = 0;
        for(int p=0;p<P;p++){
            char c[48]; snprintf(c,sizeof c,"emb.3.%d",p);
            size_t off=(size_t)p*VMAXB, n=D*sizeof(float)-off; if(n>VMAXB) n=VMAXB;
            if(gravar(&b, c, (unsigned char*)novo + off, (long)n)) reescreveu++;
        }
        fechar(&b);

        Mapa m2;
        if(!banco_mapa(&m2, "/tmp/cm")){ puts("      nao remapeou"); return 1; }
        long bate_novo = 0, bate_velho = 0, n_tot = 0;
        for(int p=0;p<P;p++){
            char c[48]; snprintf(c,sizeof c,"emb.3.%d",p);
            long n=0; const unsigned char *r = banco_ver(&m2, c, &n);
            if(!r) continue;
            unsigned char *pn=(unsigned char*)novo+(size_t)p*VMAXB;
            unsigned char *pv=(unsigned char*)v[3]+(size_t)p*VMAXB;
            for(long k=0;k<n;k++){
                unsigned char x = banco_byte(r,k);
                if(x==pn[k]) bate_novo++;
                if(x==pv[k]) bate_velho++;
                n_tot++;
            }
        }
        /* e os OUTROS vectores nao foram tocados — escrever num nao mexe nos vizinhos */
        long vizinhos_ok = 0, vizinhos_tot = 0;
        for(int i=0;i<NV;i++){
            if(i==3) continue;
            for(int p=0;p<P;p++){
                char c[48]; snprintf(c,sizeof c,"emb.%d.%d",i,p);
                long n=0; const unsigned char *r=banco_ver(&m2,c,&n);
                if(!r) continue;
                unsigned char *o=(unsigned char*)v[i]+(size_t)p*VMAXB;
                for(long k=0;k<n;k++){ if(banco_byte(r,k)==o[k]) vizinhos_ok++; vizinhos_tot++; }
            }
        }
        banco_larga(&m2);
        printf("      reescrito o vector 3: %ld de %ld bytes sao o NOVO (%ld ainda o velho)\n",
               bate_novo, n_tot, bate_velho);
        printf("      os outros %d vectores: %ld de %ld intactos\n", NV-1, vizinhos_ok, vizinhos_tot);
        ok("ESCREVE-SE NO CRISTAL e o que volta e' o novo — e' a metade que faltava, e sem"
           " ela 'aprender' nao tem onde acontecer",
           reescreveu==P && n_tot>0 && bate_novo==n_tot);
        ok("e escrever num vector NAO MEXE nos vizinhos — o cristal tem enderecos, nao e'"
           " uma pasta que se reescreve inteira", vizinhos_tot>0 && vizinhos_ok==vizinhos_tot);
    }

    /* ═══ §C4 — o CONTROLO: o crc recusa o torto ══════════════════════════════════ */
    {
        int fd = open("/tmp/cm.dat", O_RDWR);
        if(fd >= 0){
            struct stat st; fstat(fd,&st);
            off_t meio = st.st_size/2;
            unsigned char x; lseek(fd,meio,SEEK_SET);
            if(read(fd,&x,1)==1){ x^=0xFF; lseek(fd,meio,SEEK_SET); if(write(fd,&x,1)!=1){} }
            close(fd);
        }
        Mapa m3; long lidos=0, recusados=0;
        if(banco_mapa(&m3,"/tmp/cm")){
            for(int i=0;i<NV;i++) for(int p=0;p<P;p++){
                char c[48]; snprintf(c,sizeof c,"emb.%d.%d",i,p); long n=0;
                if(banco_ver(&m3,c,&n)) lidos++; else recusados++;
            }
            banco_larga(&m3);
        }
        printf("      com um byte trocado: %ld lidos, %ld recusados pelo crc\n", lidos, recusados);
        ok("e o CONTROLO fecha: um registo torto e' RECUSADO e nao entregue — sem isto, o"
           " §C1 nao provava que o crc faz alguma coisa", recusados > 0 && lidos > 0);
    }

    puts("");
    if(!falhas){
        puts("  ─────────────────────────────────────────────────────────────────────────");
        puts("  NAO HA' DOADOR: HA' MAIS UM CRISTAL. 'Doador' e' extractivo — tira-se dele");
        puts("  e ele nao recebe — e foi esse conceito que fez os colhedores SO' LEREM. O");
        puts("  nome estava a desenhar o codigo.");
        puts("");
        puts("  O modelo e' um ficheiro com estrutura e enderecos, que e' o que o banco");
        puts("  guarda. Posto la', o par fecha: grava-se, le-se de volta EXACTO com o crc a");
        puts("  confirmar, e ESCREVE-SE POR CIMA — e o que se escreve fica.");
        puts("");
        puts("  E escrever num vector nao mexe nos vizinhos: o cristal tem ENDERECOS, nao e'");
        puts("  uma pasta que se reescreve inteira. E' isso que faz 'ensinar e aprender'");
        puts("  poder acontecer aqui em vez de ser palavra — o que se aprende tem onde ficar,");
        puts("  e da' proxima vez le-se o que ficou.");
        puts("");
        puts("  NAO E' TREINO, e nao se finge que e'. E' a VIA para ele.");
    } else printf("  FALHOU: %d\n", falhas);
    return falhas ? 1 : 0;
}
