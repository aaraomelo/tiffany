/* agentes.c — A FITA POVOADA: cada agente é um organismo, e todos servem a assistente.
 *
 * (comentário teórico inalterado — ver git)
 *
 *   cc -O2 -std=c99 -I. agentes.c -o agentes && ./agentes
 */
#define _GNU_SOURCE
#include <stdio.h>
#include "../lib/disco.h"
#define ba DISCO_FIXO(unsigned char, 34)
#define bb DISCO_FIXO(unsigned char, 35)

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "unidade.h"

#define MAXAG 8
#define BLOBS "/usr/share/ollama/.ollama/models/blobs/"

typedef struct {
    const char *nome;
    const char *blob;
    const char *papel;
    long long   bytes;
    long long   desloc;
    long        telo[16];
    int         n_telo;
} Agente;

static Agente ag[MAXAG] = {
  {"qwen2.5:1.5b",    "sha256-183715c435899236895da3869489cc30ac241476b4971a20285b1a462818a5b4",
   "escreve e responde",      0,0,{0},0},
  {"llama3.2:1b",     "sha256-74701a8c35f6c8d9a4b91f3f3497643001d63e0c7a84e085bed452548fa88d45",
   "o mais leve: rascunha",   0,0,{0},0},
  {"gemma2:2b",       "sha256-7462734796d67c40ecec2ca98eddf970e171dbb6b370e43fd633ee75b69abe1b",
   "revê o que o outro diz",  0,0,{0},0},
  {"nomic-embed-text","sha256-970aa74c0a90ef7482477cf803618e776e173c007bf957f635f1015bfcfef0e6",
   "cifra em vetor: procura", 0,0,{0},0},
  {"gpt-oss:20b",   "sha256-e7b273f9636059a689e3ddcab3716e4f65abe0143ac978e46673ad0e52d09efb",
   "MoE 32x4: o pesado",      0,0,{0},0},
};
static int nag = 5;

static int telomero(const unsigned char *b, size_t n, long *t, int m){
    long x = 0, y = 0;
    for(size_t i = 0; i < n; i++){ x += (long)b[i]*(long)(i%251+1); y += (long)b[i]*(long)b[i]; }
    if(!x) x = 1;
    if(!y) y = 1;
    int k = 0;
    while(y && k < m){ long q = x/y, r = x - q*y; t[k++] = q; x = y; y = r; }
    return k;
}

static long long agora_ns(void){
    struct timespec s;
    clock_gettime(CLOCK_MONOTONIC, &s);
    return (long long)s.tv_sec * 1000000000LL + s.tv_nsec;
}

int main(void){
    disco_prende(DISCO_BASE(34),"dados/ba.bin",(size_t)(1<<20),1);
    disco_zera(ba,(size_t)(1<<20),1);
    disco_prende(DISCO_BASE(35),"dados/bb.bin",(size_t)(1<<20),1);
    disco_zera(bb,(size_t)(1<<20),1);
const char *dir = getenv("FITA_DIR") ? getenv("FITA_DIR") : ".torre";
char f_povo[512];
snprintf(f_povo, sizeof f_povo, "%s/povoada.bin", dir);

printf("\n=== A FITA POVOADA: OS AGENTES SÃO OS ORGANISMOS ==========================\n");
printf("    A 'máquina de fazer gente' do recado não era biologia. Estes são os\n");
printf("    organismos desta fita, e cada um entra como o qwen entrou.\n");

printf("\n§A1  OS AGENTES ENTRAM: lugar, tamanho e telómero.\n\n");
long long pos = 0;
long long t_total_ns = 0;
{
    mkdir(dir, 0755);
    int fd_f = open(f_povo, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fd_f < 0){ perror("agentes: criar a fita"); return 1; }
    printf("      agente             papel                    tamanho      lugar na fita\n");
    int erros = 0;
    long long t0 = agora_ns();
    for(int i = 0; i < nag; i++){
        char caminho[512];
        snprintf(caminho, sizeof caminho, "%s%s", BLOBS, ag[i].blob);
        int fd_o = open(caminho, O_RDONLY);
        if(fd_o < 0){ printf("      %-18s (sem acesso ao blob)\n", ag[i].nome); erros++; continue; }
        struct stat st;
        fstat(fd_o, &st);
        ag[i].bytes = (long long)st.st_size;
        ag[i].desloc = pos;
        off_t o_org = 0, o_fit = (off_t)pos;
        long long restam = ag[i].bytes;
        while(restam > 0){
            ssize_t r = copy_file_range(fd_o, &o_org, fd_f, &o_fit, (size_t)restam, 0);
            if(r <= 0){ erros++; break; }
            restam -= r; pos += r;
        }
        close(fd_o);
        printf("      %-18s %-24s %6lld MB     %lld\n",
               ag[i].nome, ag[i].papel, ag[i].bytes / 1000000LL, ag[i].desloc);
    }
    t_total_ns = agora_ns() - t0;
    close(fd_f);
    printf("\n      %lld bytes (%lld.%03lld GB) em %lld.%03lld s — %lld MB/s\n\n",
           pos, pos / 1000000000LL, (pos / 1000000LL) % 1000,
           t_total_ns / 1000000000LL, (t_total_ns / 1000000LL) % 1000,
           t_total_ns > 0 ? pos * 1000LL / t_total_ns : 0);
    ok("todos os agentes entraram na fita, sem erro", erros == 0 && pos > 0);

    int fd_m = open(f_povo, O_RDONLY);
    for(int i = 0; i < nag; i++){
        unsigned char ponta[4096];
        if(pread(fd_m, ponta, sizeof ponta, (off_t)ag[i].desloc) == (ssize_t)sizeof ponta)
            ag[i].n_telo = telomero(ponta, sizeof ponta, ag[i].telo, 16);
    }
    close(fd_m);
}

printf("\n§A2  OS TELÓMEROS distinguem — e quantos termos são precisos.\n\n");
{
    printf("      agente             telómero (8 primeiros)\n");
    for(int i = 0; i < nag; i++){
        printf("      %-18s ", ag[i].nome);
        for(int k = 0; k < ag[i].n_telo && k < 8; k++) printf("%ld ", ag[i].telo[k]);
        printf("\n");
    }
    printf("\n      termos   pares colididos\n");
    int primeiro_bom = -1;
    for(int k = 1; k <= 8; k++){
        int col = 0;
        for(int i = 0; i < nag; i++) for(int j = i+1; j < nag; j++)
            if(!memcmp(ag[i].telo, ag[j].telo, (size_t)k*sizeof(long))) col++;
        printf("      %-8d %d\n", k, col);
        if(col == 0 && primeiro_bom < 0) primeiro_bom = k;
    }
    printf("\n      bastam %d termo(s) para separar %d agentes\n\n", primeiro_bom, nag);
    ok("os telómeros distinguem os agentes uns dos outros", primeiro_bom > 0);
    printf("      Na fita.c foram precisos 10 termos para separar 338 genes; aqui basta muito\n");
    printf("      menos, e a razão é a mesma lei do aniversário: o custo cresce com N² sobre o\n");
    printf("      espaço, e quatro é muito menos que trezentos e trinta e oito.\n");
}

printf("\n§A3  CADA AGENTE SAI BYTE A BYTE: o teletransporte, sobre gente digital.\n\n");
{
    int fd_f = open(f_povo, O_RDONLY);
    long long comparados = 0; int divergentes = 0;
    long long t0 = agora_ns();

    printf("      agente             bytes conferidos   divergências\n");
    for(int i = 0; i < nag; i++){
        char caminho[512];
        snprintf(caminho, sizeof caminho, "%s%s", BLOBS, ag[i].blob);
        int fd_o = open(caminho, O_RDONLY);
        if(fd_o < 0) continue;
        long long restam = ag[i].bytes, oo = 0, of = ag[i].desloc;
        int d = 0;
        while(restam > 0){
            size_t p = restam > (long long)((size_t)((1<<20))*sizeof(unsigned char)) ? ((size_t)((1<<20))*sizeof(unsigned char)) : (size_t)restam;
            if(pread(fd_o, ba, p, oo) != (ssize_t)p){ d = 1; break; }
            if(pread(fd_f, bb, p, of) != (ssize_t)p){ d = 1; break; }
            if(memcmp(ba, bb, p)){ d = 1; break; }
            oo += p; of += p; restam -= p; comparados += p;
        }
        close(fd_o);
        divergentes += d;
        printf("      %-18s %10lld         %d\n", ag[i].nome, ag[i].bytes, d);
    }
    close(fd_f);
    long long dt = agora_ns() - t0;
    printf("\n      %lld bytes (%lld.%03lld GB) conferidos em %lld.%03lld s\n\n",
           comparados, comparados / 1000000000LL, (comparados / 1000000LL) % 1000,
           dt / 1000000000LL, (dt / 1000000LL) % 1000);
    ok("cada agente sai da fita exatamente como entrou — byte a byte", divergentes == 0);
    ok("e conferiu-se o genoma INTEIRO de cada um, não uma amostra", comparados == pos);
    printf("      É o mesmo protocolo do teletransporte.c §X4, com uma diferença que só se vê\n");
    printf("      agora: o que atravessa já não são bytes quaisquer — é quem responde.\n");
}

printf("\n§A4  A ASSISTENTE DESPACHA: quem responde ao quê.\n\n");
{
    printf("      pergunta chega\n");
    printf("        └─ corpus responde?          erosão, depois dilatação\n");
    printf("             ├─ sim  → responde, e não acorda ninguém\n");
    printf("             └─ NÃO  → o DECRETO passa a palavra:\n");
    printf("                  ├─ é para procurar?   %s\n", ag[3].nome);
    printf("                  ├─ é para rascunhar?  %s\n", ag[1].nome);
    printf("                  ├─ é para responder?  %s\n", ag[0].nome);
    printf("                  ├─ é para rever?      %s\n", ag[2].nome);
    printf("                  └─ é o difícil?       %s\n", ag[4].nome);
    printf("        e a resposta VOLTA para o corpus — da próxima ninguém é acordado\n\n");
    int papeis_distintos = 1;
    for(int i = 0; i < nag; i++) for(int j = i+1; j < nag; j++)
        if(!strcmp(ag[i].papel, ag[j].papel)) papeis_distintos = 0;
    ok("cada agente tem papel próprio — o despacho não é arbitrário", papeis_distintos);
    printf("      E o gpt-oss entra por último no despacho pela mesma razão por que entrou por\n");
    printf("      último na fita: é cinquenta vezes o nomic, e acordá-lo custa. A ordem não é\n");
    printf("      de qualidade — é de preço, e quem decide é o corpus ao dizer que não sabe.\n\n");
    printf("      E o critério de quem fala primeiro é o corpus, não o modelo: o decreto do\n");
    printf("      conversa.c é o único dos três métodos SEM dual, e é por isso que ele pode\n");
    printf("      recusar. Um sistema que nunca diz 'não sei' não tem como passar a palavra.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    %d agentes, %lld.%03lld GB na fita, cada um com o seu telómero e a sair byte a\n",
       nag, pos / 1000000000LL, (pos / 1000000LL) % 1000);
printf("    byte. São organismos no sentido que o dna.c mediu: têm genoma, replicam\n");
printf("    sem perda, e cada dobra tem o seu lado dual.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
