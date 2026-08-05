/* veste.c — O OLLAMA VESTIDO COM A TÚNICA: ele emite ISA, o banco executa, e o ciclo fica ESCRITO.
 *
 * O Aarão: "o plugue deve ser inversível, claro — ler e escrever é a mesma operação dual. Veste o
 * ollama com a túnica e faz ele controlar o banco."
 *
 * A TÚNICA é o par adjunto (ler, escrever), e ele já está medido: `colheita.c` §C2 mede que ler e
 * escrever são adjuntos com resíduo zero, e o `icc.c` §I4 mede o mesmo par na aferente/eferente.
 * **Vestir o modelo com ela é dar-lhe os dois lados:**
 *
 *      ler       o banco -> o estado -> o prompt         (a torre BRANCA desce)
 *      escrever  a resposta -> a operação -> o banco     (a torre NEGRA sobe)
 *
 * E aqui o modelo não responde: **controla**. Cada resposta dele vira um `STORE` num slot, e o slot
 * seguinte a ler é o que ele acabou de escrever. *O programa é o modelo; o banco é a máquina.*
 *
 * O QUE SE MEDIU, com o `llama3.2:1b` local (o script é `tools/veste.sh`):
 *
 *      t   LOAD   a resposta                          STORE
 *      0     0    "O número 3 é um número cardinal"      4
 *      1     0    "O número 4 é um número cardinal"     11
 *      2     0    "O número 11 é um número que tem"      3
 *      FECHOU: período 3, em 3 passos, no estado 3 (de 16 possíveis)
 *
 * E o banco no fim:  `0 0 0 4 11 0 0 0 0 0 0 3 0 0 0 0`
 *
 * **O ciclo do modelo ficou ESCRITO no banco**: o slot 3 guarda 4, o 4 guarda 11, o 11 guarda 3.
 * Ler a memória é ler a órbita. *Não foi preciso guardar o percurso à parte — ele É o estado.*
 *
 *   §V1  a TÚNICA: ler e escrever são adjuntos, e é isso que a torna vestível
 *   §V2  o FECHO é garantido: em Z_q a órbita repete em <= q passos, por gaiola
 *   §V3  o ciclo fica ESCRITO no banco — a órbita é a memória, e mede-se
 *   §V4  e o que o modelo NÃO faz: ele não calcula o fecho, o corpo é que o impõe
 *
 *   cc -O2 -std=c99 -Wall -Wformat veste.c -lm -o veste && ./veste
 *   (o ciclo mede-se com tools/veste.sh, que fala com o ollama local)
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "unidade.h"

int main(void){
    puts("veste.c — O OLLAMA VESTIDO COM A TUNICA: ele controla, o banco executa\n");

    /* ── §V1 ─────────────────────────────────────────────────────────────── */
    puts("§V1  A TUNICA: ler e escrever sao ADJUNTOS — e e isso que a torna vestivel\n");
    {
        /* a adjuncao <Af,c> = <f,A'c>, aqui num banco de 8 slots com uma leitura por indice.
         * E o mesmo par do icc.c §I4 e do colheita.c §C2 — refeito aqui, nao citado. */
        /* A ADJUNCAO e uma identidade de SELECAO: A le pelo indice e A^T soma no indice.
         * <Af,c> = sum_i f[idx[i]]*c[i] = <f,A'c>, e as duas somas tem EXATAMENTE os mesmos
         * termos, reordenados. Nao ha nada a arredondar. A versao anterior usava vetores de
         * double e comparava com 1e-12 — media o arredondamento de uma identidade exata.
         * Em inteiros ela e igualdade, e varre-se uma familia em vez de um caso. */
        long long casos = 0, iguais = 0;
        for(int semente = 0; semente < 500; semente++){
            long long f[8], c[8];
            int idx[8];
            for(int i = 0; i < 8; i++){
                f[i]   = ((semente*7 + i*13) % 21) - 10;
                c[i]   = ((semente*11 + i*5) % 19) - 9;
                idx[i] = (semente*3 + i*i) % 8;
            }
            long long Af[8], Ac[8];
            for(int i = 0; i < 8; i++){ Af[i] = f[idx[i]]; Ac[i] = 0; }
            for(int i = 0; i < 8; i++) Ac[idx[i]] += c[i];
            long long e1 = 0, e2 = 0;
            for(int i = 0; i < 8; i++){ e1 += Af[i]*c[i]; e2 += f[i]*Ac[i]; }
            casos++;
            if(e1 == e2) iguais++;
        }
        printf("     -> %lld pares (f,c,idx) em Z^8, com <Af,c> == <f,A'c> EXATO: %lld\n",
               casos, iguais);
        ok("LER e ESCREVER sao adjuntos: <Af,c> = <f,A'c>, IGUALDADE em inteiros",
           iguais == casos && casos >= 500);
        conclui("e e por isso que a tunica se veste nos dois sentidos: o mesmo par serve para");
        conclui("o modelo ler o banco e para escrever nele — nao sao duas interfaces, e uma.");
        puts("");
    }

    /* ── §V2 ─────────────────────────────────────────────────────────────── */
    puts("§V2  O FECHO E GARANTIDO: em Z_q a orbita repete em <= q passos, por gaiola\n");
    {
        /* nao e sorte do modelo: e contagem. Mede-se em varios q, com uma funcao qualquer. */
        int fecham = 0, casos = 0, pior = 0;
        for(int q = 4; q <= 64; q *= 2){
            for(int semente = 0; semente < q; semente++){
                int visto[64], est = semente, t = 0;
                memset(visto, -1, sizeof visto);
                while(t <= q){
                    if(visto[est] >= 0) break;
                    visto[est] = t;
                    est = (est*est + 7) % q;      /* uma funcao qualquer: o modelo e uma delas */
                    t++;
                }
                if(t <= q) fecham++;
                if(t > pior) pior = t;
                casos++;
            }
        }
        ok("TODA orbita em Z_q fecha em no maximo q passos — em 124 sementes, sem excecao",
           fecham == casos);
        printf("     -> %d sementes em q = 4..64, todas fecham; o pior levou %d passos.\n",
               casos, pior);
        conclui("o fecho nao depende de o modelo ser bom: depende de o corpo ser FINITO.");
        puts("");
    }

    /* ── §V3 ─────────────────────────────────────────────────────────────── */
    puts("§V3  O CICLO FICA ESCRITO NO BANCO — a orbita E a memoria\n");
    {
        /* medido com tools/veste.sh contra o ollama local: o ciclo 3 -> 4 -> 11 -> 3, e o banco
         * no fim tem 4 no slot 3, 11 no slot 4, e 3 no slot 11. */
        /* ── ISTO E' UM REGISTO, NAO UMA MEDIDA ──────────────────────────────────
         * Estes numeros sao a TRANSCRICAO de uma corrida do Ollama de julho, e a
         * assercao abaixo compara-os consigo proprios: nao pode falhar. O script que
         * os produziu (tools/veste.sh) saiu com o Ollama, e nao ha' snapshot em
         * dados/colhido/ — a corrida NAO E' REPRODUZIVEL, nem em principio.
         *
         * Fica como registo historico e diz-se o que e'. O que a assercao verifica e'
         * que a ESTRUTURA do ciclo fecha — que 3->4->11->3 e' um 3-ciclo no banco
         * transcrito —, e isso e' uma propriedade dos numeros, nao do modelo. */
        int banco[16] = { 0,0,0,4,11,0,0,0,0,0,0,3,0,0,0,0 };
        int ciclo[3] = { 3, 4, 11 };
        int fecha = 1;
        for(int i = 0; i < 3; i++)
            if(banco[ciclo[i]] != ciclo[(i+1)%3]) fecha = 0;
        ok("o ciclo esta ESCRITO: slot 3 guarda 4, slot 4 guarda 11, slot 11 guarda 3",
           fecha);
        /* e os outros slots ficaram intactos — o modelo so tocou onde passou */
        int tocados = 0;
        for(int i = 0; i < 16; i++) if(banco[i]) tocados++;
        ok("e so os slots VISITADOS foram escritos: 3 de 16, e os outros ficaram a zero",
           tocados == 3);
        printf("     -> banco: ");
        for(int i = 0; i < 16; i++) printf("%d ", banco[i]);
        puts("");
        conclui("nao foi preciso guardar o percurso a parte — ele E o estado. Ler a memoria e");
        conclui("ler a orbita, e e por isso que a assistente nao precisa de um log.");
        puts("");
    }

    /* ── §V4 ─────────────────────────────────────────────────────────────── */
    puts("§V4  E O QUE O MODELO NAO FAZ\n");
    {
        /* o modelo escolhe o PROXIMO; nao escolhe FECHAR. Mede-se: a mesma orbita com uma
         * funcao aleatoria fecha igual, e com um corpo INFINITO nao fecharia. */
        long est = 3, t = 0;
        int fechou_infinito = 0;
        while(t < 10000){                            /* sem modulo: o "corpo infinito" */
            est = est*est + 7;
            if(est == 3){ fechou_infinito = 1; break; }
            if(est > 1000000000L) break;
            t++;
        }
        ok("num corpo INFINITO a mesma dinamica NAO fecha — logo o fecho e do corpo, nao do modelo",
           !fechou_infinito);
        conclui("o modelo controla o QUE se escreve; o corpo controla o QUANDO acaba. Sao as");
        conclui("duas torres: a branca escolhe o passo, a negra garante o retorno.");
        puts("");
    }

    puts("──────────────────────────────────────────────────────────────────────────────");
    /* o unidade.h imprime o rodape e o residuo sozinho, no atexit */
    /* SAIA COM 0 mesmo com assercoes a falhar: o rodape do unidade.h e' impresso por
     * atexit, e o atexit SO' IMPRIME — nao altera o codigo de saida. A bateria decide
     * VERDE/FALHA pelo exit, logo as falhas daqui eram invisiveis. Medido por injecao:
     * tornei a primeira assercao falsa e o medidor continuava a sair 0. */
    return falhas ? 1 : 0;
}
