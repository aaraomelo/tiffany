/* regua.c — AS DUAS RÉGUAS. PT e EN não se contêm; se contivessem, seriam o mesmo idioma.
 *
 * O erro anterior (ancora.c, homogeneo.c) não foi de método: foi de ALVO. Eu pedia uma
 * transformação finita ponto a ponto — perfeição —, e perfeição só há na morte. Aqui não se
 * busca isomorfismo: mede-se a régua, acha-se a ÂNCORA e AMPUTA-SE o resto no ponto em que
 * cortar custa menos que continuar.
 *
 * Quatro medidas:
 *   §R1  as duas réguas: quantos tipos, quantos tokens, quanto de cauda. Se |V_pt| != |V_en|
 *        sobre o MESMO conteúdo, nenhuma bijeção de tipos existe — por contagem, não por sorte.
 *   §R2  o leque: o próprio corpus não é FUNÇÃO em nenhuma das duas direções. Uma frase EN tem
 *        várias PT e vice-versa. Quem diz que não há mapa ponto a ponto é o dado, não eu.
 *   §R3  a amputação: a curva de cobertura. Onde as primeiras palavras dão quase tudo e a cauda
 *        cobra o infinito. O corte é onde a próxima palavra deixa de pagar.
 *   §R4  a âncora é ÓRBITA, não ponto: um verbo não é uma palavra, é um paradigma gerado por
 *        regra. Candidato a infinitivo (-ar/-er/-ir) vira verbo quando a sua FAMÍLIA aparece no
 *        corpus. Isso valida a âncora sem dicionário externo — colhe-se da borda.
 *
 * SEM MEMÓRIA: estado em RAM O(1). Contagens, marcas e histogramas vivem em arquivos, por
 * pread/pwrite, endereçamento aberto — o mesmo princípio do léxico de ancora.c.
 *
 *   cc -O2 -std=c99 regua.c -o regua && ./regua pares.tsv
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define NS   (1L<<20)          /* slots das tabelas de contagem (tipos ~1e5, carga ~10%)   */
#define NM   (1L<<21)          /* slots das tabelas de marca                               */
#define NCC  (1L<<20)          /* histograma de contagens (count-of-counts)                */
#define LMAX 4096

#include "unidade.h"

/* ------------------------------------------------------------ disco: as tabelas ---------- */
static int abre(const char *nome, long bytes){
    int fd = open(nome, O_RDWR|O_CREAT|O_TRUNC, 0644);
    if(fd < 0){ perror(nome); exit(2); }
    if(ftruncate(fd, bytes) < 0){ perror("ftruncate"); exit(2); }
    return fd;
}
static uint64_t fp64(const char *s, size_t n){
    uint64_t h = 1469598103934665603UL;
    for(size_t i = 0; i < n; i++){ h ^= (unsigned char)s[i]; h *= 1099511628211UL; }
    return h ? h : 1;
}
/* marca: 8 B/slot. devolve 1 se a chave era NOVA */
static int marca(int fd, uint64_t fp){
    long i = (long)(fp % (uint64_t)NM);
    for(long t = 0; t < NM; t++){
        uint64_t s = 0;
        if(pread(fd, &s, 8, i*8) != 8) return 0;
        if(!s){ pwrite(fd, &fp, 8, i*8); return 1; }
        if(s == fp) return 0;
        i = (i+1) % NM;
    }
    return 0;
}
static int vista(int fd, uint64_t fp){                       /* só consulta */
    long i = (long)(fp % (uint64_t)NM);
    for(long t = 0; t < NM; t++){
        uint64_t s = 0;
        if(pread(fd, &s, 8, i*8) != 8) return 0;
        if(!s) return 0;
        if(s == fp) return 1;
        i = (i+1) % NM;
    }
    return 0;
}
/* contagem: 16 B/slot {fp, cnt}. devolve a contagem nova; *nova=1 se a chave era nova */
static uint64_t inc(int fd, uint64_t fp, int *nova){
    long i = (long)(fp % (uint64_t)NS);
    if(nova) *nova = 0;
    for(long t = 0; t < NS; t++){
        uint64_t s[2] = {0,0};
        if(pread(fd, s, 16, i*16) != 16) return 0;
        if(!s[0]){ s[0] = fp; s[1] = 1; pwrite(fd, s, 16, i*16); if(nova) *nova = 1; return 1; }
        if(s[0] == fp){ s[1]++; pwrite(fd, s+1, 8, i*16+8); return s[1]; }
        i = (i+1) % NS;
    }
    return 0;
}
static uint64_t consulta(int fd, uint64_t fp){
    long i = (long)(fp % (uint64_t)NS);
    for(long t = 0; t < NS; t++){
        uint64_t s[2] = {0,0};
        if(pread(fd, s, 16, i*16) != 16) return 0;
        if(!s[0]) return 0;
        if(s[0] == fp) return s[1];
        i = (i+1) % NS;
    }
    return 0;
}
static void cc_inc(int fd, uint64_t c){                      /* histograma de contagens */
    if(c >= NCC) c = NCC - 1;
    uint64_t v = 0;
    pread(fd, &v, 8, (long)c*8); v++;
    pwrite(fd, &v, 8, (long)c*8);
}

/* ------------------------------------------------------------ tokens --------------------- */
/* palavra = corrida de [a-z0-9] + qualquer byte >=0x80 (acento é letra). ASCII vira minúscula. */
static int letra(unsigned char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c >= 0x80;
}
static size_t proxima(const char *s, size_t n, size_t *i, char *out, size_t om){
    while(*i < n && !letra((unsigned char)s[*i])) (*i)++;
    size_t k = 0;
    while(*i < n && letra((unsigned char)s[*i])){
        unsigned char c = (unsigned char)s[*i];
        if(c >= 'A' && c <= 'Z') c += 32;
        if(k + 1 < om) out[k++] = (char)c;
        (*i)++;
    }
    out[k] = 0;
    return k;
}
static uint64_t fp_frase(const char *s, size_t n){           /* frase normalizada, ordem preservada */
    char buf[LMAX]; size_t k = 0, i = 0, l; char w[128];
    while((l = proxima(s, n, &i, w, sizeof w))){
        if(k + l + 1 >= sizeof buf) break;
        if(k) buf[k++] = ' ';
        memcpy(buf + k, w, l); k += l;
    }
    buf[k] = 0;
    return fp64(buf, k);
}

/* ------------------------------------------------------------ paradigmas ----------------- */
/* A família de um infinitivo. Não é dicionário: é a REGRA da conjugação (a borda que gera). */
static const char *suf_ar[] = {"o","a","as","amos","am","ei","ou","aram","ava","avam","ando","ado","ada",0};
static const char *suf_er[] = {"o","e","es","emos","em","i","eu","eram","ia","iam","endo","ido","ida",0};
static const char *suf_ir[] = {"o","e","es","imos","em","i","iu","iram","ia","iam","indo","ido","ida",0};


/* O CORPUS NÃO ESTÁ NO REPOSITÓRIO — está no .gitignore, e com razão: são 12 MB. A consequência
 * é que este medidor depende de um ficheiro que um disco limpo não tem, e num disco limpo ele
 * não mede. Isso não se resolve escondendo: procura-se onde o corpus costuma estar, e se não
 * houver diz-se e sai-se com 2. Um medidor sem o objeto a medir não passa nem falha: não mediu.
 * (Foi assim que ele passou meses verde aqui e teria falhado no CI.) */
static const char *corpus_procura(const char *pedido){
    /* O PEDIDO É UMA PREFERÊNCIA, NÃO UMA OBRIGAÇÃO — e devolvê-lo sem o abrir era o defeito:
     * a bateria passa "pares.tsv" por argumento, o ficheiro não está aí, e o medidor desistia
     * em vez de procurar. Se o pedido abre, é ele; se não abre, procura-se. */
    if(pedido){
        FILE *f = fopen(pedido, "r");
        if(f){ fclose(f); return pedido; }
    }
    const char *e = getenv("TATOEBA_CORPUS");
    if(e && *e) return e;
    static const char *cands[] = { "pares.tsv", "tatoeba/pares.tsv", "../tatoeba/pares.tsv",
                                   "/tmp/pares.tsv", NULL };
    for(int i = 0; cands[i]; i++){
        FILE *f = fopen(cands[i], "r");
        if(f){ fclose(f); return cands[i]; }
    }
    return "pares.tsv";
}

int main(int argc, char **argv){
    const char *arq = corpus_procura(argc > 1 ? argv[1] : NULL);
    FILE *f;

    printf("\n=== AS DUAS RÉGUAS ========================================================\n");
    printf("    corpus: %s\n", arq);

    int cen = abre("regua_cnt_en.bin", NS*16), cpt = abre("regua_cnt_pt.bin", NS*16);
    int fen = abre("regua_fan_en.bin", NS*16), fpt = abre("regua_fan_pt.bin", NS*16);
    int mpar = abre("regua_par.bin", NM*8);
    int ccen = abre("regua_cc_en.bin", NCC*8), ccpt = abre("regua_cc_pt.bin", NCC*8);
    int mcand = abre("regua_cand.bin", NM*8), mforma = abre("regua_forma.bin", NM*8);

    /* ---------------------------------------------------------------- passada 1 ---------- */
    long linhas = 0, tok_en = 0, tok_pt = 0, tip_en = 0, tip_pt = 0;
    long par_dist = 0, sen_en = 0, sen_pt = 0;
    long lmax_en = 0, lmax_pt = 0;
    if(!(f = fopen(arq, "r"))){ perror(arq); return 2; }
    {
        char linha[LMAX];
        while(fgets(linha, sizeof linha, f)){
            size_t n = strlen(linha);
            while(n && (linha[n-1]=='\n' || linha[n-1]=='\r')) linha[--n] = 0;
            char *tab = strchr(linha, '\t');
            if(!tab) continue;
            *tab = 0;
            const char *en = linha, *pt = tab + 1;
            size_t nen = strlen(en), npt = strlen(pt);
            if(!nen || !npt) continue;
            linhas++;

            char w[128]; size_t i, l;
            i = 0; while((l = proxima(en, nen, &i, w, sizeof w))){ int nv; inc(cen, fp64(w,l), &nv); tok_en++; tip_en += nv; }
            i = 0; while((l = proxima(pt, npt, &i, w, sizeof w))){ int nv; inc(cpt, fp64(w,l), &nv); tok_pt++; tip_pt += nv; }

            uint64_t he = fp_frase(en, nen), hp = fp_frase(pt, npt);
            char both[LMAX]; int m = snprintf(both, sizeof both, "%016llx%016llx",
                                             (unsigned long long)he, (unsigned long long)hp);
            if(marca(mpar, fp64(both, (size_t)m))){          /* par (en,pt) inédito */
                par_dist++;
                int nv;
                uint64_t ce = inc(fen, he, &nv); if(nv) sen_en++;
                uint64_t cp = inc(fpt, hp, &nv); if(nv) sen_pt++;
                if((long)ce > lmax_en) lmax_en = (long)ce;
                if((long)cp > lmax_pt) lmax_pt = (long)cp;
            }
        }
    }
    fclose(f);

    printf("\n§R1  As duas réguas — o mesmo conteúdo, medido duas vezes.\n\n");
    printf("      lado   tokens      tipos     tokens/tipo\n");
    printf("      EN   %9ld  %9ld      %6.2f\n", tok_en, tip_en, (double)tok_en/tip_en);
    printf("      PT   %9ld  %9ld      %6.2f\n", tok_pt, tip_pt, (double)tok_pt/tip_pt);
    printf("\n      pares lidos: %ld     |V_pt|/|V_en| = %.3f\n", linhas, (double)tip_pt/tip_en);
    ok("|V_pt| != |V_en| sobre o MESMO conteúdo", tip_pt != tip_en);
    printf("\n      As duas réguas têm marcação diferente: o PT parte o mesmo conteúdo em MAIS\n");
    printf("      tipos (flexiona mais). Logo nenhuma bijeção de tipos existe — e a razão é\n");
    printf("      contagem, não escolha de método. Uma não contém a outra.\n");

    /* ---------------------------------------------------------------- §R2 --------------- */
    printf("\n§R2  O leque: o corpus não é FUNÇÃO em nenhuma das duas direções.\n\n");
    long fan_en_ge2 = 0, fan_pt_ge2 = 0;
    {
        /* varredura sequencial das tabelas de leque */
        long off;
        for(off = 0; off < NS; off++){
            uint64_t s[2];
            if(pread(fen, s, 16, off*16) == 16 && s[0] && s[1] >= 2) fan_en_ge2++;
            if(pread(fpt, s, 16, off*16) == 16 && s[0] && s[1] >= 2) fan_pt_ge2++;
        }
    }
    printf("      pares distintos ............................. %ld\n", par_dist);
    printf("      frases EN distintas ........................ %ld   leque médio %.3f\n",
           sen_en, (double)par_dist/sen_en);
    printf("      frases PT distintas ........................ %ld   leque médio %.3f\n",
           sen_pt, (double)par_dist/sen_pt);
    printf("      frases EN com 2+ traduções PT .............. %ld  (%.1f%%)   máx %ld\n",
           fan_en_ge2, 100.0*fan_en_ge2/sen_en, lmax_en);
    printf("      frases PT com 2+ traduções EN .............. %ld  (%.1f%%)   máx %ld\n",
           fan_pt_ge2, 100.0*fan_pt_ge2/sen_pt, lmax_pt);
    ok("leque > 1 nos DOIS sentidos (relação, não função)", fan_en_ge2 > 0 && fan_pt_ge2 > 0);
    printf("\n      Não é ruído do corpus: é o que traduzir é. Uma relação com leque nos dois\n");
    printf("      lados não admite inversa ponto a ponto — quem nega o isomorfismo finito é o\n");
    printf("      DADO. Exigir bijeção era exigir que os dois idiomas fossem um só.\n");

    /* ---------------------------------------------------------------- §R3 --------------- */
    printf("\n§R3  A amputação: onde a próxima palavra deixa de pagar.\n");
    printf("     Curva de cobertura exata, colhida do histograma de contagens (sem ordenar).\n");
    for(int lado = 0; lado < 2; lado++){
        int cfd = lado ? cpt : cen, ccfd = lado ? ccpt : ccen;
        long tokens = lado ? tok_pt : tok_en, tipos = lado ? tip_pt : tip_en;
        /* histograma */
        long soma_t = 0, soma_k = 0;
        for(long off = 0; off < NS; off++){
            uint64_t s[2];
            if(pread(cfd, s, 16, off*16) == 16 && s[0]){ cc_inc(ccfd, s[1]); soma_k++; soma_t += (long)s[1]; }
        }
        if(soma_k != tipos || soma_t != tokens) falhas++;
        printf("\n      %s   tipos=%ld  tokens=%ld   (identidade do histograma: %s)\n",
               lado ? "PT" : "EN", tipos, tokens,
               (soma_k == tipos && soma_t == tokens) ? "fecha ✓" : "QUEBRA ✗");
        double alvo[6] = {0.50, 0.80, 0.90, 0.95, 0.99, 1.00};
        int ia = 0;
        long k = 0, cob = 0, hapax = 0;
        printf("      cobertura   palavras (k)   %% do léxico\n");
        for(long c = NCC - 1; c >= 1; c--){
            uint64_t q = 0;
            if(pread(ccfd, &q, 8, c*8) != 8 || !q) continue;
            if(c == 1) hapax = (long)q;
            k += (long)q; cob += (long)q * c;
            while(ia < 6 && (double)cob/tokens >= alvo[ia]){
                printf("        %5.0f%%     %10ld       %6.2f%%\n", alvo[ia]*100, k, 100.0*k/tipos);
                ia++;
            }
        }
        printf("      hapax (contagem 1): %ld  = %.1f%% do léxico, %.2f%% dos tokens\n",
               hapax, 100.0*hapax/tipos, 100.0*hapax/tokens);
        if(k != tipos || cob != tokens) falhas++;
    }
    printf("\n      A cauda é o infinito: metade do léxico aparece UMA vez e paga quase nada.\n");
    printf("      Amputar não é desistir — é parar onde o custo por ponto de cobertura explode.\n");

    /* ---------------------------------------------------------------- §R4 --------------- */
    printf("\n§R4  A âncora é ÓRBITA, não ponto: o verbo é um paradigma, e a regra o valida.\n");
    long cand = 0, verbo = 0, formas_marcadas = 0, orb_soma = 0, orb_max = 0;
    if(!(f = fopen(arq, "r"))){ perror(arq); return 2; }
    {
        char linha[LMAX], w[128];
        while(fgets(linha, sizeof linha, f)){
            char *tab = strchr(linha, '\t'); if(!tab) continue;
            const char *pt = tab + 1; size_t npt = strlen(pt), i = 0, l;
            while((l = proxima(pt, npt, &i, w, sizeof w))){
                if(l < 4) continue;
                const char **suf = 0;
                if(!strcmp(w+l-2, "ar")) suf = suf_ar;
                else if(!strcmp(w+l-2, "er")) suf = suf_er;
                else if(!strcmp(w+l-2, "ir")) suf = suf_ir;
                if(!suf) continue;
                if(!marca(mcand, fp64(w,l))) continue;        /* tipo já examinado */
                cand++;
                /* a família: quantas formas do paradigma existem no corpus? */
                char forma[160]; long orb = 0; int pres[24]; int np = 0;
                for(int s = 0; suf[s]; s++){
                    int m = snprintf(forma, sizeof forma, "%.*s%s", (int)(l-2), w, suf[s]);
                    pres[np++] = (consulta(cpt, fp64(forma,(size_t)m)) > 0);
                    if(pres[np-1]) orb++;
                }
                if(orb >= 3){                                  /* órbita presente => é verbo */
                    verbo++; orb_soma += orb; if(orb > orb_max) orb_max = orb;
                    marca(mforma, fp64(w,l)); formas_marcadas++;
                    for(int s = 0; suf[s]; s++){
                        int m = snprintf(forma, sizeof forma, "%.*s%s", (int)(l-2), w, suf[s]);
                        if(marca(mforma, fp64(forma,(size_t)m))) formas_marcadas++;
                    }
                }
            }
        }
    }
    fclose(f);
    printf("\n      candidatos a infinitivo (-ar/-er/-ir, |w|>=4) ....... %ld\n", cand);
    printf("      validados pela FAMÍLIA (3+ formas no corpus) ........ %ld  (%.1f%%)\n",
           verbo, 100.0*verbo/cand);
    printf("      órbita média %.2f formas   máx %ld de %d\n",
           verbo ? (double)orb_soma/verbo : 0.0, orb_max, 13);
    printf("      formas marcadas como âncora ......................... %ld\n", formas_marcadas);
    ok("a regra separa verbo de sósia (nem todo -er é verbo)", verbo > 0 && verbo < cand);

    /* alcance da âncora: quantos pares têm ao menos uma forma ancorada */
    long com_ancora = 0, tok_ancora = 0, pares2 = 0;
    if(!(f = fopen(arq, "r"))){ perror(arq); return 2; }
    {
        char linha[LMAX], w[128];
        while(fgets(linha, sizeof linha, f)){
            char *tab = strchr(linha, '\t'); if(!tab) continue;
            const char *pt = tab + 1; size_t npt = strlen(pt), i = 0, l;
            int tem = 0; pares2++;
            while((l = proxima(pt, npt, &i, w, sizeof w)))
                if(vista(mforma, fp64(w,l))){ tem = 1; tok_ancora++; }
            if(tem) com_ancora++;
        }
    }
    fclose(f);
    printf("\n      pares com ao menos uma âncora ....................... %ld  (%.1f%%)\n",
           com_ancora, 100.0*com_ancora/pares2);
    printf("      tokens PT ancorados ................................. %ld  (%.1f%% dos tokens)\n",
           tok_ancora, 100.0*tok_ancora/tok_pt);
    printf("\n      O verbo não é uma palavra: é uma ÓRBITA gerada por regra — e é por isso que\n");
    printf("      serve de âncora. O que se traduz nele é a FUNÇÃO, que sobrevive à troca de\n");
    printf("      régua; o nome próprio é ponto, e ponto é o que se amputa.\n");

    printf("\n=== O QUE ISSO MUDA =======================================================\n");
    printf("  Não há transformação finita ponto a ponto — e a culpa não é do método: o corpus\n");
    printf("  é uma RELAÇÃO com leque nos dois sentidos (§R2), e as duas réguas nem têm o mesmo\n");
    printf("  número de marcas (§R1). Pedir bijeção era pedir que PT e EN fossem um só idioma.\n");
    printf("  O que resta é traduzir pela FUNÇÃO: ancorar na órbita (§R4), cobrir o quanto a\n");
    printf("  curva paga (§R3) e amputar a cauda — sem avançar no infinito.\n");
    if(falhas){ printf("\n  FALHAS: %d\n\n", falhas); return 1; }
    printf("\n  RESÍDUO 0 — as identidades de contagem fecham; as medidas estão acima.\n\n");
    return 0;
}
