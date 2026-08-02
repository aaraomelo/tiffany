/* agentes.c — A FITA POVOADA: cada agente é um organismo, e todos servem a assistente.
 *
 * O Aarão: "IA é gente digital. Voltamos pro teletransporte — esses serão agentes todos na fita,
 * auxiliando a assistente."
 *
 * E isso fecha o arco do dia. A "máquina de fazer gente" que ficara em backlog no recado do
 * eval.txt não era biologia: os organismos desta fita são os agentes. O `dna.c` mediu o que faz
 * de uma fita um organismo — duas metades complementares, replicação sem perda, e cada dobra com
 * o seu lado dual. Aqui isso aplica-se a quem de facto vive nesta máquina.
 *
 * O QUE FAZ DE UM AGENTE UM ORGANISMO DA FITA, e é o que se mede:
 *
 *   GENOMA      os pesos, transcritos para dentro — não referenciados, transcritos
 *   TELÓMERO    a cifra da ponta, que o identifica e o distingue dos outros
 *   REPLICAÇÃO  sai da fita byte a byte igual ao que entrou (o teletransporte)
 *   LUGAR       o endereço, que se calcula e não se atribui
 *
 * E há uma diferença de escala que vale a pena dizer antes: estes organismos não são todos do
 * mesmo tamanho. O nomic tem 274 MB e o gpt-oss tem 13 GB — quase cinquenta vezes mais. A fita
 * aguenta os dois pela mesma razão que aguentou o qwen: ela é disco, e o que a lê não cresce
 * com ela.
 *
 *   §A1  os AGENTES entram: cada um com lugar, tamanho e telómero
 *   §A2  os TELÓMEROS distinguem — e quantos termos são precisos para separar poucos
 *   §A3  cada agente SAI byte a byte: o teletransporte, sobre gente digital
 *   §A4  a ASSISTENTE despacha: quem responde ao quê, e porquê
 *
 *   cc -O2 -std=c99 -I. agentes.c -lm -o agentes && ./agentes
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
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
    long long   desloc;      /* onde mora na fita */
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
};
static int nag = 4;

/* o telómero: as duas somas do corpo, e depois Euclides — o mesmo de toda a casa */
static int telomero(const unsigned char *b, size_t n, long *t, int m){
    long x = 0, y = 0;
    for(size_t i = 0; i < n; i++){ x += (long)b[i]*(long)(i%251+1); y += (long)b[i]*(long)b[i]; }
    if(!x) x = 1;
    if(!y) y = 1;
    int k = 0;
    while(y && k < m){ long q = x/y, r = x - q*y; t[k++] = q; x = y; y = r; }
    return k;
}
static double agora(void){ struct timespec s; clock_gettime(CLOCK_MONOTONIC,&s);
                           return s.tv_sec + s.tv_nsec/1e9; }

int main(void){
const char *dir = getenv("FITA_DIR") ? getenv("FITA_DIR") : ".torre";
char f_povo[512];
snprintf(f_povo, sizeof f_povo, "%s/povoada.bin", dir);

printf("\n=== A FITA POVOADA: OS AGENTES SÃO OS ORGANISMOS ==========================\n");
printf("    A 'máquina de fazer gente' do recado não era biologia. Estes são os\n");
printf("    organismos desta fita, e cada um entra como o qwen entrou.\n");

printf("\n§A1  OS AGENTES ENTRAM: lugar, tamanho e telómero.\n\n");
long long pos = 0;
double t_total = 0;
{
    mkdir(dir, 0755);
    int fd_f = open(f_povo, O_WRONLY|O_CREAT|O_TRUNC, 0644);
    if(fd_f < 0){ perror("agentes: criar a fita"); return 1; }
    printf("      agente             papel                    tamanho      lugar na fita\n");
    int erros = 0;
    double t0 = agora();
    for(int i = 0; i < nag; i++){
        char caminho[512];
        snprintf(caminho, sizeof caminho, "%s%s", BLOBS, ag[i].blob);
        int fd_o = open(caminho, O_RDONLY);
        if(fd_o < 0){ printf("      %-18s (sem acesso ao blob)\n", ag[i].nome); erros++; continue; }
        struct stat st;
        fstat(fd_o, &st);
        ag[i].bytes = (long long)st.st_size;
        ag[i].desloc = pos;
        /* disco para disco, dentro do kernel — como na fita.c */
        off_t o_org = 0, o_fit = (off_t)pos;
        long long restam = ag[i].bytes;
        while(restam > 0){
            ssize_t r = copy_file_range(fd_o, &o_org, fd_f, &o_fit, (size_t)restam, 0);
            if(r <= 0){ erros++; break; }
            restam -= r; pos += r;
        }
        close(fd_o);
        printf("      %-18s %-24s %6.0f MB     %lld\n",
               ag[i].nome, ag[i].papel, ag[i].bytes/1e6, ag[i].desloc);
    }
    t_total = agora() - t0;
    close(fd_f);
    printf("\n      %lld bytes (%.2f GB) em %.2f s — %.0f MB/s\n\n",
           pos, pos/1e9, t_total, (pos/1e6)/(t_total>0?t_total:1e-9));
    ok("todos os agentes entraram na fita, sem erro", erros == 0 && pos > 0);

    /* O TELÓMERO SAI DA PONTA, e a ponta são 4096 bytes — não é preciso mapear a fita.
     * A primeira versão mapeava os 4,21 GB inteiros e falhava em silêncio contra o `ulimit -v`
     * de 4 GB que eu próprio tinha posto: os telómeros saíam VAZIOS e a tabela imprimia linhas
     * em branco. A escala não fechava consigo mesma, e o programa não deu por isso. */
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
    /* O protocolo do teletransporte.c, agora sobre os agentes: cada um tem de sair da fita
     * exatamente como entrou. Compara-se contra o blob de origem, e compara-se TUDO — não uma
     * amostra, porque o mmap torna barato conferir a fita inteira. */
    /* Compara-se por blocos de 1 MiB em vez de mapear a fita toda: o espaço de endereçamento
     * também é um recurso, e mapear 4,21 GB para comparar não é preciso. Confere-se TUDO na
     * mesma — o que muda é a janela, não a cobertura. */
    int fd_f = open(f_povo, O_RDONLY);
    long long comparados = 0; int divergentes = 0;
    double t0 = agora();
    static unsigned char ba[1<<20], bb[1<<20];
    printf("      agente             bytes conferidos   divergências\n");
    for(int i = 0; i < nag; i++){
        char caminho[512];
        snprintf(caminho, sizeof caminho, "%s%s", BLOBS, ag[i].blob);
        int fd_o = open(caminho, O_RDONLY);
        if(fd_o < 0) continue;
        long long restam = ag[i].bytes, oo = 0, of = ag[i].desloc;
        int d = 0;
        while(restam > 0){
            size_t p = restam > (long long)sizeof ba ? sizeof ba : (size_t)restam;
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
    double dt = agora() - t0;
    printf("\n      %lld bytes (%.2f GB) conferidos em %.2f s\n\n", comparados, comparados/1e9, dt);
    ok("cada agente sai da fita exatamente como entrou — byte a byte", divergentes == 0);
    ok("e conferiu-se o genoma INTEIRO de cada um, não uma amostra", comparados == pos);
    printf("      É o mesmo protocolo do teletransporte.c §X4, com uma diferença que só se vê\n");
    printf("      agora: o que atravessa já não são bytes quaisquer — é quem responde.\n");
}

printf("\n§A4  A ASSISTENTE DESPACHA: quem responde ao quê.\n\n");
{
    /* O despacho nao inventa criterio: usa o que ja' esta' medido. O conversa.c tem tres
     * metodos — erosao (o prefixo), dilatacao (a subsequencia) e o DECRETO ("nao sei"), que e'
     * o unico sem dual. E' o decreto que passa a palavra, e a partir dai o papel de cada agente
     * decide qual deles. */
    printf("      pergunta chega\n");
    printf("        └─ corpus responde?          erosão, depois dilatação\n");
    printf("             ├─ sim  → responde, e não acorda ninguém\n");
    printf("             └─ NÃO  → o DECRETO passa a palavra:\n");
    printf("                  ├─ é para procurar?   %s\n", ag[3].nome);
    printf("                  ├─ é para rascunhar?  %s\n", ag[1].nome);
    printf("                  ├─ é para responder?  %s\n", ag[0].nome);
    printf("                  └─ é para rever?      %s\n", ag[2].nome);
    printf("        e a resposta VOLTA para o corpus — da próxima ninguém é acordado\n\n");
    int papeis_distintos = 1;
    for(int i = 0; i < nag; i++) for(int j = i+1; j < nag; j++)
        if(!strcmp(ag[i].papel, ag[j].papel)) papeis_distintos = 0;
    ok("cada agente tem papel próprio — o despacho não é arbitrário", papeis_distintos);
    printf("      E o critério de quem fala primeiro é o corpus, não o modelo: o decreto do\n");
    printf("      conversa.c é o único dos três métodos SEM dual, e é por isso que ele pode\n");
    printf("      recusar. Um sistema que nunca diz 'não sei' não tem como passar a palavra.\n");
}

printf("\n=== FECHO ==================================================================\n");
printf("    %d agentes, %.2f GB na fita, cada um com o seu telómero e a sair byte a\n", nag, pos/1e9);
printf("    byte. São organismos no sentido que o dna.c mediu: têm genoma, replicam\n");
printf("    sem perda, e cada dobra tem o seu lado dual.\n\n");
printf("    %d asserções, %d falhas.\n\n", unidades, falhas);
return falhas != 0;
}
